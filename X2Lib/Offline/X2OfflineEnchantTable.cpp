#include "stdafx.h"
#include "X2OfflineEnchantTable.h"

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineLog.h"
#include "X2OfflineLuaEnum.h"

CX2OfflineEnchantTable* CX2OfflineEnchantTable::ms_pInstance = NULL;

namespace
{
	const wchar_t* const SCRIPT_ENCHANT_TABLE = L"EnchantTable.lua";

	/// How many entries ENCHANT_STAT_SCALE actually has. The client's own two
	/// readers of it (CX2Item::GetEnchantStat, X2Item.cpp:596 and :605) test
	/// the index against sizeof(ENCHANT_STAT_SCALE), which is the size in
	/// BYTES - 84, not 21. That bug is harmless there because the enchant
	/// level can never reach 84, so it is left alone rather than "fixed" in
	/// a file this phase has no business touching; this constant is the count
	/// the cross-check below needs and is deliberately not that expression.
	const int ENCHANT_STAT_SCALE_COUNT =
		(int)( sizeof( ENCHANT_STAT_SCALE ) / sizeof( ENCHANT_STAT_SCALE[0] ) );
}

//////////////////////////////////////////////////////////////////////////

CX2OfflineEnchantTable::CX2OfflineEnchantTable()
: m_bLoadAttempted( false )
, m_bLoaded( false )
, m_iWeaponEnchantStone( 0 )
, m_iRareWeaponEnchantStone( 0 )
, m_iArmorEnchantStone( 0 )
, m_iRareArmorEnchantStone( 0 )
, m_iSupportMaterialLimit( 0 )
, m_iEventSupportMaterialLimit( 0 )
, m_iEnchantLimit( 0 )
, m_iProbRows( 0 )
, m_iStoneRows( 0 )
, m_iSupportRows( 0 )
, m_iRateRows( 0 )
, m_iOtherRows( 0 )
{
}

CX2OfflineEnchantTable::~CX2OfflineEnchantTable()
{
}

CX2OfflineEnchantTable* CX2OfflineEnchantTable::Instance()
{
	if( NULL == ms_pInstance )
		ms_pInstance = new CX2OfflineEnchantTable;

	return ms_pInstance;
}

void CX2OfflineEnchantTable::Release()
{
	if( NULL == ms_pInstance )
		return;

	delete ms_pInstance;
	ms_pInstance = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Loading

void CX2OfflineEnchantTable::EnsureLoaded()
{
	if( true == m_bLoadAttempted )
		return;

	m_bLoadAttempted = true;

	if( NULL == g_pKTDXApp ||
		NULL == g_pKTDXApp->GetLuaBinder() ||
		NULL == g_pKTDXApp->GetDeviceManager() ||
		NULL == g_pKTDXApp->GetDeviceManager()->GetMassFileManager() )
	{
		CX2OfflineLog::Server( L"ENCHANT  ERROR engine not ready - enhancement will refuse." );
		return;
	}

	lua_State* pLuaState = g_pKTDXApp->GetLuaBinder()->GetLuaState();
	if( NULL == pLuaState )
	{
		CX2OfflineLog::Server( L"ENCHANT  ERROR no lua state - enhancement will refuse." );
		return;
	}

	// EnchantTable.lua subscripts ITEM_TYPE["IT_WEAPON"] / ["IT_DEFENCE"] /
	// ["IT_NONE"] - globals that exist on the real server and nowhere in the
	// client's Lua state. See X2OfflineLuaEnum.h.
	if( false == CX2OfflineLuaEnum::Publish() )
	{
		CX2OfflineLog::Server( L"ENCHANT  ERROR could not publish ITEM_TYPE - enhancement will refuse." );
		return;
	}

	// Bind as the global the file calls into, mirroring
	// CXSLEnchantItemManager::RegisterLuaBind. Every name the packed file
	// mentions is bound, including the ones inside its block comments and the
	// ones whose SERV_ flag is off in this build: lua_tinker turns a call to an
	// unbound method into an error that abandons the rest of the chunk, and the
	// probability rows this phase needs are at the very end of the file.
	lua_tinker::class_add< CX2OfflineEnchantTable >( pLuaState, "CX2OfflineEnchantTable" );

	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "SetWeaponEnchantStone",
		&CX2OfflineEnchantTable::SetWeaponEnchantStone_LUA );
	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "SetRareWeaponEnchantStone",
		&CX2OfflineEnchantTable::SetRareWeaponEnchantStone_LUA );
	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "SetArmorEnchantStone",
		&CX2OfflineEnchantTable::SetArmorEnchantStone_LUA );
	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "SetRareArmorEnchantStone",
		&CX2OfflineEnchantTable::SetRareArmorEnchantStone_LUA );

	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "SetSupportMaterialLimit",
		&CX2OfflineEnchantTable::SetSupportMaterialLimit_LUA );
	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "SetEventSupportMaterialLimit",
		&CX2OfflineEnchantTable::SetEventSupportMaterialLimit_LUA );
	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "SetEnchantLimit",
		&CX2OfflineEnchantTable::SetEnchantLimit_LUA );

	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "AddEnchantStoneInfo",
		&CX2OfflineEnchantTable::AddEnchantStoneInfo_LUA );
	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "AddEventEnchantStoneInfo",
		&CX2OfflineEnchantTable::AddEventEnchantStoneInfo_LUA );
	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "AddSupportMaterialInfo",
		&CX2OfflineEnchantTable::AddSupportMaterialInfo_LUA );
	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "AddEventSupportMaterialInfo",
		&CX2OfflineEnchantTable::AddEventSupportMaterialInfo_LUA );
	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "AddRestoreItemInfo",
		&CX2OfflineEnchantTable::AddRestoreItemInfo_LUA );
	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "AddEventRestoreItemInfo",
		&CX2OfflineEnchantTable::AddEventRestoreItemInfo_LUA );
	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "AddEnchantPlusInfo",
		&CX2OfflineEnchantTable::AddEnchantPlusInfo_LUA );
	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "AddDestroyGuardInfo",
		&CX2OfflineEnchantTable::AddDestroyGuardInfo_LUA );
	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "AddAttachItemInfo",
		&CX2OfflineEnchantTable::AddAttachItemInfo_LUA );

	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "SetEnchantRate",
		&CX2OfflineEnchantTable::SetEnchantRate_LUA );
	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "SetEnchantProbability",
		&CX2OfflineEnchantTable::SetEnchantProbability_LUA );
	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "SetEnchantProbability_Event",
		&CX2OfflineEnchantTable::SetEnchantProbability_Event_LUA );
	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "SetEnchantRareProbability",
		&CX2OfflineEnchantTable::SetEnchantRareProbability_LUA );
	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "SetEnchantPlusProbability",
		&CX2OfflineEnchantTable::SetEnchantPlusProbability_LUA );

	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "AddDropRandomEnchantInfo",
		&CX2OfflineEnchantTable::AddDropRandomEnchantInfo_LUA );
	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "AddCubeRandomEnchantInfo",
		&CX2OfflineEnchantTable::AddCubeRandomEnchantInfo_LUA );

	lua_tinker::class_def< CX2OfflineEnchantTable >( pLuaState, "dump",
		&CX2OfflineEnchantTable::Dump_LUA );

	lua_tinker::decl( pLuaState, "EnchantItemManager", this );

	// Same two-layer dance every other packed table needs: the .kom container
	// comes off in LoadDataFile, the XOR encryption comes off in DoMemory.
	KGCMassFileManager::CMassFile::MASSFILE_MEMBERFILEINFO_POINTER kInfo;
	kInfo = g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LoadDataFile( SCRIPT_ENCHANT_TABLE );

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
			L"ENCHANT  ERROR '%s' not found in any .kom or on disk - enhancement is OFF.",
			SCRIPT_ENCHANT_TABLE );
		CX2OfflineLog::Server(
			L"ENCHANT  XOR-encrypt KncWX2Server/ServerResource/US/%s and pack it into data036.kom.",
			SCRIPT_ENCHANT_TABLE );
		return;
	}

	bool bRan	= ( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemory( kInfo->pRealData, kInfo->size ) );
	bool bAdded	= ( m_iProbRows > 0 );

	if( false == bRan || false == bAdded )
	{
		bRan	= ( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemoryNotEncript( kInfo->pRealData, kInfo->size ) );
		bAdded	= ( m_iProbRows > 0 );

		if( true == bRan && true == bAdded )
		{
			CX2OfflineLog::Server(
				L"ENCHANT  NOTE '%s' is NOT encrypted - loaded as plaintext. Fine for testing;"
				L" encrypt it (XOREncrypt / FileEncrypt) to match every other packed script.",
				SCRIPT_ENCHANT_TABLE );
		}
	}

	if( false == bRan || false == bAdded )
	{
		CX2OfflineLog::Server(
			L"ENCHANT  ERROR '%s' produced no probability rows, encrypted or plaintext -"
			L" enhancement is OFF.", SCRIPT_ENCHANT_TABLE );
		return;
	}

	m_bLoaded = true;

	CX2OfflineLog::Server( L"ENCHANT  loaded: %d probability row(s), %d stone row(s),"
		L" %d fluor row(s), %d stat-rate row(s), %d other row(s)",
		m_iProbRows, m_iStoneRows, m_iSupportRows, m_iRateRows, m_iOtherRows );

	CX2OfflineLog::Server( L"ENCHANT  stones: weapon %d / rare %d, armor %d / rare %d;"
		L" fluor usable below +%d (+%d during an event)",
		m_iWeaponEnchantStone, m_iRareWeaponEnchantStone,
		m_iArmorEnchantStone, m_iRareArmorEnchantStone,
		m_iSupportMaterialLimit, m_iEventSupportMaterialLimit );

	CrossCheckRatesAgainstClient();
}

void CX2OfflineEnchantTable::CrossCheckRatesAgainstClient() const
{
	int iChecked	= 0;
	int iMismatch	= 0;

	std::map< int, float >::const_iterator mit;
	for( mit = m_mapEnchantRate.begin(); mit != m_mapEnchantRate.end(); ++mit )
	{
		if( mit->first < 0 || mit->first >= ENCHANT_STAT_SCALE_COUNT )
			continue;

		++iChecked;

		// The two are written to two decimal places in both files, so an exact
		// float compare would be fragile for no reason; a thousandth is far
		// tighter than any real disagreement would be.
		const float fDelta = mit->second - ENCHANT_STAT_SCALE[ mit->first ];

		if( fDelta > 0.001f || fDelta < -0.001f )
		{
			++iMismatch;

			CX2OfflineLog::Server(
				L"ENCHANT  WARNING +%d stat rate disagrees: packed %.3f, client ENCHANT_STAT_SCALE %.3f",
				mit->first, mit->second, ENCHANT_STAT_SCALE[ mit->first ] );
		}
	}

	if( 0 == iMismatch )
	{
		CX2OfflineLog::Server(
			L"ENCHANT  %d stat rate(s) match the client's own ENCHANT_STAT_SCALE - right file.",
			iChecked );
		return;
	}

	CX2OfflineLog::Server(
		L"ENCHANT  WARNING %d of %d stat rate(s) disagree with the client. The packed"
		L" EnchantTable.lua is probably from another region or another build; the enchant"
		L" levels this rolls will not match the stats the client draws.",
		iMismatch, iChecked );
}

//////////////////////////////////////////////////////////////////////////
// reads

bool CX2OfflineEnchantTable::IsLoaded()
{
	EnsureLoaded();
	return m_bLoaded;
}

int CX2OfflineEnchantTable::GetWeaponEnchantStoneID()
{
	EnsureLoaded();
	return m_iWeaponEnchantStone;
}

int CX2OfflineEnchantTable::GetRareWeaponEnchantStoneID()
{
	EnsureLoaded();
	return m_iRareWeaponEnchantStone;
}

int CX2OfflineEnchantTable::GetArmorEnchantStoneID()
{
	EnsureLoaded();
	return m_iArmorEnchantStone;
}

int CX2OfflineEnchantTable::GetRareArmorEnchantStoneID()
{
	EnsureLoaded();
	return m_iRareArmorEnchantStone;
}

int CX2OfflineEnchantTable::GetEnchantStoneItemID( int iEquipLv, int iItemType )
{
	EnsureLoaded();

	std::map< KStoneKey, int >::const_iterator mit =
		m_mapEnchantStoneInfo.find( KStoneKey( iEquipLv, iItemType ) );

	if( m_mapEnchantStoneInfo.end() == mit )
		return 0;

	return mit->second;
}

void CX2OfflineEnchantTable::GetEventEnchantStoneItemID( int iEquipLv, int iItemType,
														 OUT std::vector< int >& vecOut )
{
	EnsureLoaded();

	vecOut.clear();

	const KStoneKey kKey( iEquipLv, iItemType );

	std::multimap< KStoneKey, int >::const_iterator mit;
	for( mit = m_mmapEventEnchantStoneInfo.lower_bound( kKey );
		 mit != m_mmapEventEnchantStoneInfo.upper_bound( kKey ); ++mit )
	{
		vecOut.push_back( mit->second );
	}
}

int CX2OfflineEnchantTable::GetEnchantSupportMaterialItemID( int iEquipLv )
{
	EnsureLoaded();

	std::map< int, int >::const_iterator mit = m_mapSupportMaterialInfo.find( iEquipLv );

	if( m_mapSupportMaterialInfo.end() == mit )
		return 0;

	return mit->second;
}

void CX2OfflineEnchantTable::GetEventEnchantSupportMaterialItemID( int iEquipLv,
																   OUT std::vector< int >& vecOut )
{
	EnsureLoaded();

	vecOut.clear();

	std::multimap< int, int >::const_iterator mit;
	for( mit = m_mmapEventSupportMaterialInfo.lower_bound( iEquipLv );
		 mit != m_mmapEventSupportMaterialInfo.upper_bound( iEquipLv ); ++mit )
	{
		vecOut.push_back( mit->second );
	}
}

int CX2OfflineEnchantTable::GetSupportMaterialEnchantLimitLevel()
{
	EnsureLoaded();
	return m_iSupportMaterialLimit;
}

int CX2OfflineEnchantTable::GetEventSupportMaterialEnchantLimitLevel()
{
	EnsureLoaded();
	return m_iEventSupportMaterialLimit;
}

bool CX2OfflineEnchantTable::GetEnchantResult( bool bIsRare, int iEnchantLevel, OUT int& iResult )
{
	EnsureLoaded();

	const std::map< int, KProbRow >& mapProb =
		( true == bIsRare ) ? m_mapEnchantRareProb : m_mapEnchantProb;

	std::map< int, KProbRow >::const_iterator mit = mapProb.find( iEnchantLevel );
	if( mapProb.end() == mit )
		return false;

	iResult = LotteryDecideMulti( mit->second );

	// CXSLEnchantItemManager::GetEnchantResult's own guard, verbatim: anything
	// outside the five known results - including the -1 KLottery returns when
	// the weights do not sum to 100 - is treated as "no change" rather than
	// failing the enchant, so a rounding gap in the table can never destroy an
	// item.
	if( iResult < NetError::ERR_ENCHANT_RESULT_00 ||
		iResult > NetError::ERR_ENCHANT_RESULT_04 )
	{
		CX2OfflineLog::Server( L"ENCHANT  roll for +%d fell outside the table (%d) - no change.",
			iEnchantLevel, iResult );

		iResult = NetError::ERR_ENCHANT_RESULT_01;
	}

	return true;
}

bool CX2OfflineEnchantTable::GetEnchantRate( int iEnchantLevel, OUT float& fPercent )
{
	EnsureLoaded();

	std::map< int, float >::const_iterator mit = m_mapEnchantRate.find( iEnchantLevel );
	if( m_mapEnchantRate.end() == mit )
		return false;

	fPercent = mit->second;
	return true;
}

bool CX2OfflineEnchantTable::GetAttachItemEnchantLevel( int iAttachItemID, OUT int& iEnchantLevel,
														OUT int& iItemType )
{
	EnsureLoaded();

	std::map< int, KAttachInfo >::const_iterator mit = m_mapEnchantAttach.find( iAttachItemID );
	if( m_mapEnchantAttach.end() == mit )
		return false;

	iEnchantLevel	= mit->second.m_iEnchantLevel;
	iItemType		= mit->second.m_iItemType;
	return true;
}

bool CX2OfflineEnchantTable::CheckAttachItemEnableLevel( int iAttachItemID, int iUseLevel )
{
	EnsureLoaded();

	std::map< int, KAttachInfo >::const_iterator mit = m_mapEnchantAttach.find( iAttachItemID );
	if( m_mapEnchantAttach.end() == mit )
		return false;

	// Both bounds zero means the ticket has no level restriction at all, which
	// is most of them; only the four Cobo/Ruriel beginner amulets carry a band.
	if( 0 == mit->second.m_iMinItemLevel && 0 == mit->second.m_iMaxItemLevel )
		return true;

	if( iUseLevel < mit->second.m_iMinItemLevel )
		return false;

	if( mit->second.m_iMaxItemLevel < iUseLevel )
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Bound into Lua as EnchantItemManager:*

void CX2OfflineEnchantTable::SetWeaponEnchantStone_LUA( int iItemID )
{
	m_iWeaponEnchantStone = iItemID;
}

void CX2OfflineEnchantTable::SetRareWeaponEnchantStone_LUA( int iItemID )
{
	m_iRareWeaponEnchantStone = iItemID;
}

void CX2OfflineEnchantTable::SetArmorEnchantStone_LUA( int iItemID )
{
	m_iArmorEnchantStone = iItemID;
}

void CX2OfflineEnchantTable::SetRareArmorEnchantStone_LUA( int iItemID )
{
	m_iRareArmorEnchantStone = iItemID;
}

void CX2OfflineEnchantTable::SetSupportMaterialLimit_LUA( int iEnchantLimitLevel )
{
	m_iSupportMaterialLimit = iEnchantLimitLevel;
}

void CX2OfflineEnchantTable::SetEventSupportMaterialLimit_LUA( int iEnchantLimitLevel )
{
	m_iEventSupportMaterialLimit = iEnchantLimitLevel;
}

void CX2OfflineEnchantTable::SetEnchantLimit_LUA( int iEnchantLimitLevel )
{
	// SERV_ENCHANT_LIMIT is off in this build (ServerDefine_Global.h:534
	// comments the define out) and the packed file's own call is commented out
	// with it. Stored rather than dropped so that a file that does set it says
	// so in the log instead of silently doing nothing.
	m_iEnchantLimit = iEnchantLimitLevel;
}

void CX2OfflineEnchantTable::AddEnchantStoneInfo_LUA( int iItemID, int iItemType,
													  int iEquipLvMin, int iEquipLvMax )
{
	// CXSLEnchantItemManager::AddEnchantStoneInfo_LUA's two guards, then its
	// one-entry-per-level expansion. std::map::insert keeps the FIRST value
	// for a duplicated key, which is the behaviour the real manager has and
	// which the overlapping US/event blocks in the packed file rely on.
	if( iItemID <= 0 || iItemType <= 0 )
		return;

	if( iEquipLvMin < 0 || iEquipLvMax > 100 )
		return;

	for( int iLv = iEquipLvMin; iLv <= iEquipLvMax; ++iLv )
		m_mapEnchantStoneInfo.insert( std::make_pair( KStoneKey( iLv, iItemType ), iItemID ) );

	++m_iStoneRows;
}

void CX2OfflineEnchantTable::AddEventEnchantStoneInfo_LUA( int iItemID, int iItemType,
														   int iEquipLvMin, int iEquipLvMax )
{
	if( iItemID <= 0 || iItemType <= 0 )
		return;

	if( iEquipLvMin < 0 || iEquipLvMax > 100 )
		return;

	for( int iLv = iEquipLvMin; iLv <= iEquipLvMax; ++iLv )
		m_mmapEventEnchantStoneInfo.insert( std::make_pair( KStoneKey( iLv, iItemType ), iItemID ) );

	++m_iStoneRows;
}

void CX2OfflineEnchantTable::AddSupportMaterialInfo_LUA( int iItemID, int iEquipLvMin, int iEquipLvMax )
{
	if( iItemID <= 0 )
		return;

	if( iEquipLvMin < 0 || iEquipLvMax > 100 )
		return;

	for( int iLv = iEquipLvMin; iLv <= iEquipLvMax; ++iLv )
		m_mapSupportMaterialInfo.insert( std::make_pair( iLv, iItemID ) );

	++m_iSupportRows;
}

void CX2OfflineEnchantTable::AddEventSupportMaterialInfo_LUA( int iItemID, int iEquipLvMin, int iEquipLvMax )
{
	if( iItemID <= 0 )
		return;

	if( iEquipLvMin < 0 || iEquipLvMax > 100 )
		return;

	for( int iLv = iEquipLvMin; iLv <= iEquipLvMax; ++iLv )
		m_mmapEventSupportMaterialInfo.insert( std::make_pair( iLv, iItemID ) );

	++m_iSupportRows;
}

void CX2OfflineEnchantTable::AddRestoreItemInfo_LUA( int iItemID, int iItemType,
													 int iEquipLvMin, int iEquipLvMax )
{
	if( iItemID <= 0 )
		return;

	if( iEquipLvMin < 0 || iEquipLvMax > 100 )
		return;

	for( int iLv = iEquipLvMin; iLv <= iEquipLvMax; ++iLv )
		m_mapRestoreItemInfo.insert( std::make_pair( KRestoreKey( iItemType, iLv ), iItemID ) );

	++m_iOtherRows;
}

void CX2OfflineEnchantTable::AddEventRestoreItemInfo_LUA( int iItemID, int iItemType,
														  int iEquipLvMin, int iEquipLvMax )
{
	if( iItemID <= 0 )
		return;

	if( iEquipLvMin < 0 || iEquipLvMax > 100 )
		return;

	for( int iLv = iEquipLvMin; iLv <= iEquipLvMax; ++iLv )
		m_mmapEventRestoreItemInfo.insert( std::make_pair( KRestoreKey( iItemType, iLv ), iItemID ) );

	++m_iOtherRows;
}

void CX2OfflineEnchantTable::AddEnchantPlusInfo_LUA( int iItemID, int iEquipLvMin, int iEquipLvMax )
{
	if( iItemID <= 0 )
		return;

	for( int iLv = iEquipLvMin; iLv <= iEquipLvMax; ++iLv )
		m_mapEnchantPlusInfo.insert( std::make_pair( iLv, iItemID ) );

	++m_iOtherRows;
}

void CX2OfflineEnchantTable::AddDestroyGuardInfo_LUA( int iItemID, int iEquipLvMin, int iEquipLvMax )
{
	if( iItemID <= 0 )
		return;

	for( int iLv = iEquipLvMin; iLv <= iEquipLvMax; ++iLv )
		m_mapDestroyGuardInfo.insert( std::make_pair( iLv, iItemID ) );

	++m_iOtherRows;
}

void CX2OfflineEnchantTable::AddAttachItemInfo_LUA( int iItemID )
{
	if( iItemID <= 0 )
		return;

	if( NULL == g_pKTDXApp || NULL == g_pKTDXApp->GetLuaBinder() )
		return;

	KLuaManager luaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState() );

	KAttachInfo kRow;

	LUA_GET_VALUE( luaManager, "iItemType",		kRow.m_iItemType,		0 );
	LUA_GET_VALUE( luaManager, "iEnchantLevel",	kRow.m_iEnchantLevel,	0 );
	LUA_GET_VALUE( luaManager, "iMinItemLevel",	kRow.m_iMinItemLevel,	0 );
	LUA_GET_VALUE( luaManager, "iMaxItemLevel",	kRow.m_iMaxItemLevel,	0 );

	// CXSLEnchantItemManager::AddAttachItemInfo_LUA's four _JIF guards, in its
	// order. A row that fails any of them is dropped rather than stored,
	// because a ticket with a level of 0 or a nonsense item type would later
	// look like a valid ticket that grants nothing.
	if( kRow.m_iItemType < CX2Item::IT_NONE || kRow.m_iItemType > CX2Item::IT_ETC )
		return;

	if( kRow.m_iEnchantLevel <= 0 || kRow.m_iEnchantLevel > MAX_ENCHANT_LEVEL )
		return;

	if( kRow.m_iMinItemLevel < 0 || kRow.m_iMaxItemLevel < 0 )
		return;

	// insert, not operator[]: the real manager keeps the FIRST row for a
	// duplicated ticket ID, and the packed file duplicates several - 60007250
	// appears twice, once commented for Japan and once live for the overseas
	// team's Ruriel IDs, both at enchant level 5.
	m_mapEnchantAttach.insert( std::make_pair( iItemID, kRow ) );

	++m_iOtherRows;
}

void CX2OfflineEnchantTable::SetEnchantRate_LUA( int iEnchantLevel, float fPercent )
{
	if( iEnchantLevel < 0 || iEnchantLevel > MAX_ENCHANT_LEVEL )
		return;

	if( fPercent < 0.0f )
		return;

	m_mapEnchantRate[ iEnchantLevel ] = fPercent;

	++m_iRateRows;
}

void CX2OfflineEnchantTable::SetEnchantProbability_LUA( int iEnchantLevel )
{
	KProbRow kRow;
	ReadProbabilityRow( kRow );
	StoreProbabilityRow( iEnchantLevel, kRow, m_mapEnchantProb );

	if( false == kRow.empty() && iEnchantLevel > 0 && iEnchantLevel <= MAX_ENCHANT_LEVEL )
		++m_iProbRows;
}

void CX2OfflineEnchantTable::SetEnchantProbability_Event_LUA( int iEnchantLevel )
{
	// SERV_SUPPORT_MATERIAL_ENCHANT_EVENT is defined in this build, so the
	// real server picks this table instead whenever GameSysVal says an enchant
	// event is running. Nothing offline runs one - there is no GameSysVal.lua
	// clock and no event scheduler - so these rows are loaded and never rolled.
	KProbRow kRow;
	ReadProbabilityRow( kRow );
	StoreProbabilityRow( iEnchantLevel, kRow, m_mapEnchantEventProb );

	++m_iOtherRows;
}

void CX2OfflineEnchantTable::SetEnchantRareProbability_LUA( int iEnchantLevel )
{
	KProbRow kRow;
	ReadProbabilityRow( kRow );
	StoreProbabilityRow( iEnchantLevel, kRow, m_mapEnchantRareProb );

	if( false == kRow.empty() && iEnchantLevel > 0 && iEnchantLevel <= MAX_ENCHANT_LEVEL )
		++m_iProbRows;
}

void CX2OfflineEnchantTable::SetEnchantPlusProbability_LUA( int iEnchantLevel )
{
	// SERV_ENCHANT_PLUS_ITEM is TW/HK-only (ServerDefine_TWHK.h:85) and the
	// packed file's own calls sit inside a Lua block comment. Parsed anyway.
	KProbRow kRow;
	ReadProbabilityRow( kRow );
	StoreProbabilityRow( iEnchantLevel, kRow, m_mapEnchantPlusProb );

	++m_iOtherRows;
}

void CX2OfflineEnchantTable::AddDropRandomEnchantInfo_LUA( int iEnchantLevel, float fRate )
{
	m_vecDropRandomEnchant.push_back( std::make_pair( iEnchantLevel, fRate ) );
	++m_iOtherRows;
}

void CX2OfflineEnchantTable::AddCubeRandomEnchantInfo_LUA( int iEnchantLevel, float fRate )
{
	m_vecCubeRandomEnchant.push_back( std::make_pair( iEnchantLevel, fRate ) );
	++m_iOtherRows;
}

void CX2OfflineEnchantTable::Dump_LUA()
{
}

//////////////////////////////////////////////////////////////////////////

/*static*/ void CX2OfflineEnchantTable::ReadProbabilityRow( OUT KProbRow& kOut )
{
	kOut.clear();

	if( NULL == g_pKTDXApp || NULL == g_pKTDXApp->GetLuaBinder() )
		return;

	KLuaManager luaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState() );

	// CXSLEnchantItemManager::SetEnchantProbability_LUA's five reads, in its
	// order and with its keys. The order is load-bearing twice over: it is the
	// order the weights accumulate in during a roll, and it happens to match
	// the ascending ERR_ENCHANT_RESULT_* values that KLottery's std::map would
	// have sorted them into anyway.
	static const struct
	{
		const char*	m_szKey;
		int			m_iResult;
	}
	CASES[] =
	{
		{ "Up1",		NetError::ERR_ENCHANT_RESULT_00 },	///< success, +1
		{ "NoChange",	NetError::ERR_ENCHANT_RESULT_01 },
		{ "Down1",		NetError::ERR_ENCHANT_RESULT_02 },
		{ "DownTo0",	NetError::ERR_ENCHANT_RESULT_03 },
		{ "Break",		NetError::ERR_ENCHANT_RESULT_04 },	///< item becomes unusable
	};

	for( int i = 0; i < (int)( sizeof( CASES ) / sizeof( CASES[0] ) ); ++i )
	{
		float fProb = 0.0f;

		LUA_GET_VALUE( luaManager, CASES[i].m_szKey, fProb, 0.0f );

		if( fProb < 0.0f || fProb > 100.0f )
			return;							///< the real setter's _JIF: drop the whole row

		kOut.push_back( std::make_pair( CASES[i].m_iResult, fProb ) );
	}
}

/*static*/ void CX2OfflineEnchantTable::StoreProbabilityRow( int iEnchantLevel, const KProbRow& kRow,
															 OUT std::map< int, KProbRow >& mapOut )
{
	// CXSLEnchantItemManager's own bound: level 0 has no row, because a roll is
	// always keyed by the level being ATTEMPTED, which is at least 1.
	if( iEnchantLevel <= 0 || iEnchantLevel > MAX_ENCHANT_LEVEL )
		return;

	if( true == kRow.empty() )
		return;

	mapOut[ iEnchantLevel ] = kRow;			///< "겹치면 덮어쓴다" - last wins
}

/*static*/ int CX2OfflineEnchantTable::LotteryDecideMulti( const KProbRow& kRow )
{
	if( true == kRow.empty() )
		return -1;							///< KLottery::CASE_BLANK

	const float fRoll = (float)( rand() % 10000 ) * 0.01f;		///< 0.00 .. 99.99

	float fAccumulate = 0.0f;

	for( size_t i = 0; i < kRow.size(); ++i )
	{
		fAccumulate += kRow[i].second;

		if( fRoll <= fAccumulate )
			return kRow[i].first;
	}

	return -1;
}

#endif SERV_IRUHADEV_OFFLINE
