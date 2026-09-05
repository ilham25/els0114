#include "stdafx.h"
#include "X2OfflineResolveTable.h"

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineLog.h"
#include "X2OfflineLuaEnum.h"

CX2OfflineResolveTable* CX2OfflineResolveTable::ms_pInstance = NULL;

namespace
{
	const wchar_t* const SCRIPT_RESOLVE_TABLE = L"ResolveTable.lua";

	/// CXSLResolveItemManager::GetJackpotTime's own fallback for a range that
	/// was never set (XSLResolveItemManager.cpp:296).
	const int FALLBACK_JACKPOT_SECONDS = 10800;
}

//////////////////////////////////////////////////////////////////////////

CX2OfflineResolveTable::CX2OfflineResolveTable()
: m_bLoadAttempted( false )
, m_bLoaded( false )
, m_iMinJackpotTime( 0 )
, m_iMaxJackpotTime( 0 )
, m_fJackpotProb( 0.0f )
, m_iResolveDataRows( 0 )
, m_iBrokenPieceRows( 0 )
{
}

CX2OfflineResolveTable::~CX2OfflineResolveTable()
{
}

CX2OfflineResolveTable* CX2OfflineResolveTable::Instance()
{
	if( NULL == ms_pInstance )
		ms_pInstance = new CX2OfflineResolveTable;

	return ms_pInstance;
}

void CX2OfflineResolveTable::Release()
{
	if( NULL == ms_pInstance )
		return;

	delete ms_pInstance;
	ms_pInstance = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Loading

void CX2OfflineResolveTable::EnsureLoaded()
{
	if( true == m_bLoadAttempted )
		return;

	m_bLoadAttempted = true;

	if( NULL == g_pKTDXApp ||
		NULL == g_pKTDXApp->GetLuaBinder() ||
		NULL == g_pKTDXApp->GetDeviceManager() ||
		NULL == g_pKTDXApp->GetDeviceManager()->GetMassFileManager() )
	{
		CX2OfflineLog::Server( L"RESOLVE  ERROR engine not ready - dismantling will refuse." );
		return;
	}

	lua_State* pLuaState = g_pKTDXApp->GetLuaBinder()->GetLuaState();
	if( NULL == pLuaState )
	{
		CX2OfflineLog::Server( L"RESOLVE  ERROR no lua state - dismantling will refuse." );
		return;
	}

	// ResolveTable.lua subscripts ITEM_TYPE["IT_WEAPON"] and
	// ITEM_GRADE["IG_UNIQUE"] etc. - globals that exist on the real server and
	// nowhere in the client's Lua state. See X2OfflineLuaEnum.h.
	if( false == CX2OfflineLuaEnum::Publish() )
	{
		CX2OfflineLog::Server( L"RESOLVE  ERROR could not publish ITEM_TYPE/ITEM_GRADE - dismantling will refuse." );
		return;
	}

	// Bind as the global the file calls into, mirroring
	// CXSLResolveItemManager::RegisterLuaBind (SERV_ITEM_RESOLVE_NEW arm).
	// AddResolveDataAvatar and dump are not bound: nothing in the packed file
	// calls them live (the avatar block is inside a Lua block comment), and an
	// unbound method only matters if something actually calls it.
	lua_tinker::class_add< CX2OfflineResolveTable >( pLuaState, "CX2OfflineResolveTable" );
	lua_tinker::class_def< CX2OfflineResolveTable >( pLuaState, "SetMinMaxOfJackpotTime",
		&CX2OfflineResolveTable::SetMinMaxOfJackpotTime_LUA );
	lua_tinker::class_def< CX2OfflineResolveTable >( pLuaState, "SetJackpotProb",
		&CX2OfflineResolveTable::SetJackpotProb_LUA );
	lua_tinker::class_def< CX2OfflineResolveTable >( pLuaState, "AddWeaponResolveBrokenPieceRate",
		&CX2OfflineResolveTable::AddWeaponResolveBrokenPieceRate_LUA );
	lua_tinker::class_def< CX2OfflineResolveTable >( pLuaState, "AddArmorResolveBrokenPieceRate",
		&CX2OfflineResolveTable::AddArmorResolveBrokenPieceRate_LUA );
	lua_tinker::class_def< CX2OfflineResolveTable >( pLuaState, "AddResolveDataBrokenPiece",
		&CX2OfflineResolveTable::AddResolveDataBrokenPiece_LUA );
	lua_tinker::class_def< CX2OfflineResolveTable >( pLuaState, "AddResolveData",
		&CX2OfflineResolveTable::AddResolveData_LUA );
	lua_tinker::decl( pLuaState, "g_pResolveItemManager", this );

	// Same two-layer dance every other packed table needs: the .kom container
	// comes off in LoadDataFile, the XOR encryption comes off in DoMemory.
	KGCMassFileManager::CMassFile::MASSFILE_MEMBERFILEINFO_POINTER kInfo;
	kInfo = g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LoadDataFile( SCRIPT_RESOLVE_TABLE );

	// NOT `NULL == kInfo`. MASSFILE_MEMBERFILEINFO_POINTER is a STRUCT BY
	// VALUE with an `operator const MASSFILE_MEMBERFILEINFO*() const` that
	// returns `this` (KGCMassFileManager.h), so comparing it against NULL
	// takes the address of a local and is never true. A missing file then
	// falls through to the DoMemory calls and is reported as "failed to run"
	// instead of "not packed" - which is the one diagnosis this branch
	// exists to give. The client's own loader tests the payload
	// (X2ItemManager.cpp:231); so does this.
	if( NULL == kInfo->pRealData || kInfo->size <= 0 )
	{
		CX2OfflineLog::Server(
			L"RESOLVE  ERROR '%s' not found in any .kom or on disk.", SCRIPT_RESOLVE_TABLE );
		CX2OfflineLog::Server(
			L"RESOLVE  XOR-encrypt KncWX2Server/ServerResource/US/%s and pack it into data036.kom.",
			SCRIPT_RESOLVE_TABLE );
		return;
	}

	bool bRan = ( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemory( kInfo->pRealData, kInfo->size ) );
	bool bAdded = ( m_iResolveDataRows > 0 || m_iBrokenPieceRows > 0 );

	if( false == bRan || false == bAdded )
	{
		bRan = ( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemoryNotEncript( kInfo->pRealData, kInfo->size ) );
		bAdded = ( m_iResolveDataRows > 0 || m_iBrokenPieceRows > 0 );

		if( true == bRan && true == bAdded )
		{
			CX2OfflineLog::Server(
				L"RESOLVE  NOTE '%s' is NOT encrypted - loaded as plaintext. Fine for testing;"
				L" encrypt it (XOREncrypt / FileEncrypt) to match every other packed script.",
				SCRIPT_RESOLVE_TABLE );
		}
	}

	if( false == bRan || false == bAdded )
	{
		CX2OfflineLog::Server(
			L"RESOLVE  ERROR '%s' produced no rows, encrypted or plaintext.", SCRIPT_RESOLVE_TABLE );
		return;
	}

	m_bLoaded = true;

	CX2OfflineLog::Server( L"RESOLVE  loaded: %d resolve-data row(s), %d broken-piece row(s),"
		L" jackpot %.0f%% chance every %d-%d s",
		m_iResolveDataRows, m_iBrokenPieceRows, m_fJackpotProb,
		m_iMinJackpotTime, m_iMaxJackpotTime );
}

//////////////////////////////////////////////////////////////////////////
// Bound into Lua as g_pResolveItemManager:*

void CX2OfflineResolveTable::SetMinMaxOfJackpotTime_LUA( int iMin, int iMax )
{
	m_iMinJackpotTime = iMin;
	m_iMaxJackpotTime = iMax;
}

void CX2OfflineResolveTable::SetJackpotProb_LUA( float fRate )
{
	m_fJackpotProb = fRate;
}

void CX2OfflineResolveTable::AddWeaponResolveBrokenPieceRate_LUA( int iItemType, float fRate )
{
	m_vecWeaponBrokenRate.push_back( std::make_pair( iItemType, fRate ) );
}

void CX2OfflineResolveTable::AddArmorResolveBrokenPieceRate_LUA( int iItemType, float fRate )
{
	m_vecArmorBrokenRate.push_back( std::make_pair( iItemType, fRate ) );
}

void CX2OfflineResolveTable::AddResolveDataBrokenPiece_LUA( int iResolveResultType, int iItemID )
{
	if( iItemID <= 0 || iResolveResultType < (int)RRT_EL_FLOUR || iResolveResultType >= (int)RRT_MAX )
		return;

	KLuaManager luaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState() );

	KBrokenPieceResolve kRow;
	kRow.m_iItemID = iItemID;

	LUA_GET_VALUE( luaManager, "iUnitCost",	kRow.m_iUnitCost,	0 );
	LUA_GET_VALUE( luaManager, "iBeginLv",		kRow.m_iBeginLv,	0 );
	LUA_GET_VALUE( luaManager, "iEndLv",		kRow.m_iEndLv,		0 );

	m_vecBrokenPiece[ iResolveResultType ].push_back( kRow );
	++m_iBrokenPieceRows;
}

void CX2OfflineResolveTable::AddResolveData_LUA( int iItemGrade, int iItemID )
{
	if( iItemGrade <= 0 || iItemID <= 0 )
		return;

	KLuaManager luaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState() );

	KResolveData kData;
	kData.m_iResultItemID = iItemID;

	LUA_GET_VALUE( luaManager, "fSuccRate",	kData.m_fSuccRate,		0.0f );
	LUA_GET_VALUE( luaManager, "iRandomCnt",	kData.m_iRandomCount,	0 );
	LUA_GET_VALUE( luaManager, "iResultCnt",	kData.m_iResultCount,	0 );
	LUA_GET_VALUE( luaManager, "iBeginLv",		kData.m_iBeginLv,		0 );
	LUA_GET_VALUE( luaManager, "iEndLv",		kData.m_iEndLv,			0 );

	m_mapResolveData[ iItemGrade ].push_back( kData );
	++m_iResolveDataRows;
}

//////////////////////////////////////////////////////////////////////////
// Lottery, ported from KLottery::Decision (KncWX2Server/Common/Lottery.cpp) -
// same shape CX2OfflineDropTable::Decide already uses: one roll with 0.01
// resolution, cases accumulated in registration order.

/*static*/ int CX2OfflineResolveTable::LotteryDecideOne( int iCaseValue, float fWeight )
{
	if( fWeight <= 0.0f )
		return -1;

	const float fRoll = (float)( rand() % 10000 ) * 0.01f;		///< 0.00 .. 99.99

	return ( fRoll <= fWeight ) ? iCaseValue : -1;
}

/*static*/ int CX2OfflineResolveTable::LotteryDecideMulti( const std::vector< std::pair< int, float > >& vecCase )
{
	if( true == vecCase.empty() )
		return -1;

	const float fRoll = (float)( rand() % 10000 ) * 0.01f;		///< 0.00 .. 99.99

	float fAccumulate = 0.0f;

	for( size_t i = 0; i < vecCase.size(); ++i )
	{
		fAccumulate += vecCase[i].second;

		if( fRoll <= fAccumulate )
			return vecCase[i].first;
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////////

void CX2OfflineResolveTable::ResolveResultEnchantStone( const KResolveData& kData, bool bJackpot,
														 OUT std::map< int, int >& mapResult ) const
{
	for( int i = 0; i < kData.m_iRandomCount; ++i )
	{
		int iResultItemID = 0;

		if( true == bJackpot )
			iResultItemID = kData.m_iResultItemID;		///< jackpot: guaranteed hit
		else
			iResultItemID = LotteryDecideOne( kData.m_iResultItemID, kData.m_fSuccRate );

		if( iResultItemID <= 0 )
			continue;		///< CASE_BLANK - this draw missed

		std::map< int, int >::iterator mit = mapResult.find( iResultItemID );
		if( mit == mapResult.end() )
		{
			mapResult[ iResultItemID ] = ( true == bJackpot ) ? kData.m_iResultCount * 2 : kData.m_iResultCount;
		}
		else
		{
			// CXSLResolveItemManager::ResolveResult_EnchantStone
			// (XSLResolveItemManager.cpp:653-656) adds m_iResultCount on this
			// branch regardless of bJackpot - only a brand-new entry above
			// doubles. Transcribed as written, not "corrected": with
			// iRandomCnt <= 4 in the packed data this only ever fires on a
			// second-or-later successful draw of the very same stone within
			// one dismantle, which is rare enough that nobody would have
			// noticed the asymmetry either way.
			mit->second += kData.m_iResultCount;
		}
	}
}

void CX2OfflineResolveTable::ResolveResultBrokenPiece( int eType, int iEqipLevel, int iSellPrice,
													   OUT std::map< int, int >& mapResult ) const
{
	if( eType < (int)RRT_EL_FLOUR || eType >= (int)RRT_MAX )
		return;

	const std::vector< KBrokenPieceResolve >& vecRow = m_vecBrokenPiece[ eType ];

	for( size_t i = 0; i < vecRow.size(); ++i )
	{
		const KBrokenPieceResolve& kRow = vecRow[i];

		if( iEqipLevel < kRow.m_iBeginLv || iEqipLevel > kRow.m_iEndLv )
			continue;

		if( kRow.m_iUnitCost <= 0 )
			continue;

		const int   iQuantityInteger = (int)( (float)iSellPrice / (float)kRow.m_iUnitCost );
		const float fQuantityFloat   = ( ( (float)iSellPrice / (float)kRow.m_iUnitCost ) - (float)iQuantityInteger ) * 100.f;

		int iResultQuantity = iQuantityInteger;

		// The remainder is itself a lottery ticket for one more unit - the
		// case value (0) is meaningless, only hit/miss matters.
		if( -1 != LotteryDecideOne( 0, fQuantityFloat ) )
			++iResultQuantity;

		if( iResultQuantity <= 0 )
			continue;

		std::map< int, int >::iterator mit = mapResult.find( kRow.m_iItemID );
		if( mit == mapResult.end() )
			mapResult[ kRow.m_iItemID ] = iResultQuantity;
		else
			mit->second += iResultQuantity;
	}
}

int CX2OfflineResolveTable::GetJackpotTime() const
{
	const int iRange = m_iMaxJackpotTime - m_iMinJackpotTime;
	if( iRange <= 0 )
		return FALLBACK_JACKPOT_SECONDS;

	return ( rand() % iRange ) + m_iMinJackpotTime;
}

int CX2OfflineResolveTable::GetResultItem( int iItemGrade, int iUseLevel, int iItemType, int iSellPrice,
										   IN OUT bool& bJackpot, OUT std::map< int, int >& mapResult )
{
	mapResult.clear();

	EnsureLoaded();

	if( false == m_bLoaded )
	{
		bJackpot = false;
		return NetError::ERR_RESOLVE_ITEM_04;
	}

	// The manager's own confirmation roll (SetJackpotProb). The packed data
	// configures this at 100%, so a session-eligible attempt is always
	// confirmed today; the roll stays here because it is what the real
	// manager does, not because this data needs it.
	if( true == bJackpot && -1 == LotteryDecideOne( 1, m_fJackpotProb ) )
		bJackpot = false;

	// 1. Enchant stone, gated by item grade and level -
	// CXSLResolveItemManager::GetResultItem step 1 (XSLResolveItemManager.cpp:330-343).
	std::map< int, std::vector< KResolveData > >::const_iterator mitRD = m_mapResolveData.find( iItemGrade );
	if( mitRD != m_mapResolveData.end() )
	{
		for( size_t i = 0; i < mitRD->second.size(); ++i )
		{
			const KResolveData& kData = mitRD->second[i];

			if( iUseLevel < kData.m_iBeginLv || iUseLevel > kData.m_iEndLv )
				continue;

			ResolveResultEnchantStone( kData, bJackpot, mapResult );
		}
	}

	// 2. Elemental flour, always attempted - step 2 (:346-349).
	ResolveResultBrokenPiece( (int)RRT_EL_FLOUR, iUseLevel, iSellPrice, mapResult );

	// 3. Weapon or armor broken piece - step 3 (:350-425).
	if( CX2Item::IT_WEAPON == iItemType )
	{
		const int iResolveItemType = LotteryDecideMulti( m_vecWeaponBrokenRate );

		if( (int)CX2Item::IT_WEAPON == iResolveItemType )
			ResolveResultBrokenPiece( (int)RRT_WEAPON_BROKEN_PIECE, iUseLevel, iSellPrice, mapResult );
		else if( (int)CX2Item::IT_DEFENCE == iResolveItemType )
			ResolveResultBrokenPiece( (int)RRT_ARMOR_BROKEN_PIECE, iUseLevel, iSellPrice, mapResult );
		// else CASE_BLANK - unreachable with the packed data (single case at
		// 100%), and the real code only logs it, so there is nothing more to do.
	}
	else if( CX2Item::IT_DEFENCE == iItemType )
	{
		// The weapon-piece table's unit cost at this level, used as a floor
		// below which the roll is skipped entirely - XSLResolveItemManager.cpp
		// :376-397.
		int iWeaponMinUnitCost = 0;
		const std::vector< KBrokenPieceResolve >& vecWeapon = m_vecBrokenPiece[ RRT_WEAPON_BROKEN_PIECE ];
		for( size_t i = 0; i < vecWeapon.size(); ++i )
		{
			if( iUseLevel < vecWeapon[i].m_iBeginLv || iUseLevel > vecWeapon[i].m_iEndLv )
				continue;

			iWeaponMinUnitCost = vecWeapon[i].m_iUnitCost;
			break;
		}

		int iResolveItemType = (int)CX2Item::IT_NONE;
		if( iWeaponMinUnitCost <= iSellPrice )
			iResolveItemType = LotteryDecideMulti( m_vecArmorBrokenRate );
		else
			iResolveItemType = (int)CX2Item::IT_DEFENCE;		///< guaranteed floor

		if( (int)CX2Item::IT_WEAPON == iResolveItemType )
			ResolveResultBrokenPiece( (int)RRT_WEAPON_BROKEN_PIECE, iUseLevel, iSellPrice, mapResult );
		else if( (int)CX2Item::IT_DEFENCE == iResolveItemType )
			ResolveResultBrokenPiece( (int)RRT_ARMOR_BROKEN_PIECE, iUseLevel, iSellPrice, mapResult );
	}
	else
	{
		// Anything else - a non-fashion accessory, mainly, since almost every
		// real CX2Item::IT_ACCESSORY carries GetFashion() true and never
		// reaches this class at all - has no case in the real switch either
		// (XSLResolveItemManager.cpp:417-424) and fails the same way.
		return NetError::ERR_RESOLVE_ITEM_02;
	}

	return NetError::NET_OK;
}

#endif SERV_IRUHADEV_OFFLINE
