#include "stdafx.h"
#include "X2OfflineRandomItem.h"

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineLog.h"
#include "X2OfflineLuaEnum.h"
#include "X2OfflineStatTable.h"

CX2OfflineRandomItem* CX2OfflineRandomItem::ms_pInstance = NULL;

namespace
{
	const wchar_t* const SCRIPT_RANDOM_DATA		= L"RandomItemData.lua";
	const wchar_t* const SCRIPT_RANDOM_TABLE	= L"RandomItemTable.lua";

	/// CXSLItem::USE_CONDITION. The client has no copy of this enum - see
	/// X2OfflineLuaEnumSeed.h - so the three values the table actually uses are
	/// named here rather than left as bare numbers in the branch below.
	enum
	{
		UC_NONE			= 0,
		UC_ANYONE		= 1,
		UC_ONE_UNIT		= 2,
		UC_ONE_CLASS	= 3,
	};
}

//////////////////////////////////////////////////////////////////////////

CX2OfflineRandomItem::CX2OfflineRandomItem()
: m_bNotifyMsg( false )
, m_bLoadAttempted( false )
, m_iCubeRows( 0 )
, m_iGroupCaseRows( 0 )
, m_iPresentationRows( 0 )
, m_iCubeDropped( 0 )
{
}

CX2OfflineRandomItem::~CX2OfflineRandomItem()
{
}

CX2OfflineRandomItem* CX2OfflineRandomItem::Instance()
{
	if( NULL == ms_pInstance )
		ms_pInstance = new CX2OfflineRandomItem;

	return ms_pInstance;
}

void CX2OfflineRandomItem::Release()
{
	if( NULL == ms_pInstance )
		return;

	delete ms_pInstance;
	ms_pInstance = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Loading

bool CX2OfflineRandomItem::RunScript( const wchar_t* szName, int& iRowCounter )
{
	KGCMassFileManager::CMassFile::MASSFILE_MEMBERFILEINFO_POINTER kInfo;
	kInfo = g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LoadDataFile( szName );

	if( NULL == kInfo )
	{
		CX2OfflineLog::Server(
			L"CUBE     ERROR '%s' not found in any .kom or on disk.", szName );
		CX2OfflineLog::Server(
			L"CUBE     XOR-encrypt KncWX2Server/ServerResource/US/%s and pack it into data036.kom.",
			szName );
		return false;
	}

	const int iBefore = iRowCounter;

	bool bRan	= ( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemory( kInfo->pRealData, kInfo->size ) );
	bool bAdded	= ( iRowCounter > iBefore );

	if( false == bRan || false == bAdded )
	{
		bRan	= ( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemoryNotEncript( kInfo->pRealData, kInfo->size ) );
		bAdded	= ( iRowCounter > iBefore );

		if( true == bRan && true == bAdded )
		{
			CX2OfflineLog::Server(
				L"CUBE     NOTE '%s' is NOT encrypted - loaded as plaintext. Fine for testing;"
				L" encrypt it to match every other packed script.", szName );
		}
	}

	if( false == bRan || false == bAdded )
	{
		CX2OfflineLog::Server(
			L"CUBE     ERROR '%s' produced no rows, encrypted or plaintext.", szName );
		return false;
	}

	return true;
}

void CX2OfflineRandomItem::EnsureLoaded()
{
	if( true == m_bLoadAttempted )
		return;

	m_bLoadAttempted = true;

	if( NULL == g_pKTDXApp ||
		NULL == g_pKTDXApp->GetLuaBinder() ||
		NULL == g_pKTDXApp->GetDeviceManager() ||
		NULL == g_pKTDXApp->GetDeviceManager()->GetMassFileManager() )
	{
		CX2OfflineLog::Server( L"CUBE     ERROR engine not ready - cubes stay closed." );
		return;
	}

	lua_State* pLuaState = g_pKTDXApp->GetLuaBinder()->GetLuaState();
	if( NULL == pLuaState )
	{
		CX2OfflineLog::Server( L"CUBE     ERROR no lua state - cubes stay closed." );
		return;
	}

	// RandomItemTable.lua writes USE_CONDITION["UC_ANYONE"] and
	// UNIT_CLASS["UC_NONE"] in every one of its 4360 blocks, and `True` for
	// m_bGiveAll. Without those globals every block would load with condition 0
	// and give-all false - a table that parses cleanly and is wrong in a way
	// nothing would catch. So this is a hard prerequisite, not a nicety.
	if( false == CX2OfflineLuaEnum::Publish() )
	{
		CX2OfflineLog::Server( L"CUBE     ERROR enum tables could not be published - not loading"
			L" RandomItemTable.lua, because every cube would load with a null use-condition." );
		return;
	}

	lua_tinker::class_add< CX2OfflineRandomItem >( pLuaState, "CX2OfflineRandomItem" );

	lua_tinker::class_def< CX2OfflineRandomItem >( pLuaState, "AddRandomItemTemplet",
		&CX2OfflineRandomItem::AddRandomItemTemplet_LUA );
	lua_tinker::class_def< CX2OfflineRandomItem >( pLuaState, "AddRandomItemGroup",
		&CX2OfflineRandomItem::AddRandomItemGroup_LUA );
	lua_tinker::class_def< CX2OfflineRandomItem >( pLuaState, "SetNotifyMsg",
		&CX2OfflineRandomItem::SetNotifyMsg_LUA );
	lua_tinker::class_def< CX2OfflineRandomItem >( pLuaState, "AddAttractionItemInfo",
		&CX2OfflineRandomItem::AddAttractionItemInfo_LUA );
	lua_tinker::class_def< CX2OfflineRandomItem >( pLuaState, "AddSealRandomItemInfo",
		&CX2OfflineRandomItem::AddSealRandomItemInfo_LUA );
	lua_tinker::class_def< CX2OfflineRandomItem >( pLuaState, "AddNotifyResultItemInfo",
		&CX2OfflineRandomItem::AddNotifyResultItemInfo_LUA );
	lua_tinker::class_def< CX2OfflineRandomItem >( pLuaState, "AddCharmItem",
		&CX2OfflineRandomItem::AddCharmItem_LUA );
	lua_tinker::class_def< CX2OfflineRandomItem >( pLuaState, "AddRandomItemMappingData",
		&CX2OfflineRandomItem::AddRandomItemMappingData_LUA );
	lua_tinker::class_def< CX2OfflineRandomItem >( pLuaState, "AddRandomItemMappingTime",
		&CX2OfflineRandomItem::AddRandomItemMappingTime_LUA );

	lua_tinker::decl( pLuaState, "g_pRandomItemManager", this );

	// Order matters here, unlike the drop table's two files: the group rows
	// have to exist before a templet block can name one, and RandomItemTable.lua
	// is written that way internally. RandomItemData.lua is independent and goes
	// first only so its counters are separate in the log.
	RunScript( SCRIPT_RANDOM_DATA,	m_iPresentationRows );
	RunScript( SCRIPT_RANDOM_TABLE,	m_iGroupCaseRows );

	CX2OfflineLog::Server( L"CUBE     loaded: %d cube(s) in %u key set(s), %d case(s) across"
		L" %u group(s); %d presentation row(s)",
		m_iCubeRows, (unsigned int)m_mapCubeKey.size(), m_iGroupCaseRows,
		(unsigned int)m_mapGroup.size(), m_iPresentationRows );

	if( m_iCubeDropped > 0 )
	{
		CX2OfflineLog::Server( L"CUBE     NOTE %d cube templet(s) dropped - no item templet for the"
			L" cube itself in this client's .kom. Normal: RandomItemTable.lua covers every region.",
			m_iCubeDropped );
	}

	if( 0 == m_iCubeRows )
	{
		CX2OfflineLog::Server( L"CUBE     WARNING no cubes loaded. Opening a cube stays refused"
			L" until RandomItemTable.lua is packed into data036.kom." );
	}
}

//////////////////////////////////////////////////////////////////////////
// Bound into Lua as g_pRandomItemManager:*

void CX2OfflineRandomItem::AddRandomItemTemplet_LUA()
{
	KLuaManager luaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState() );

	KCube kCube;

	int iGiveAll = 0;

	LUA_GET_VALUE( luaManager, "m_ItemID",				kCube.m_iItemID,			0 );
	LUA_GET_VALUE( luaManager, "m_KeyItemID",			kCube.m_iKeyItemID,			0 );
	LUA_GET_VALUE( luaManager, "m_RequiredKeyCount",	kCube.m_iRequiredKeyCount,	0 );
	LUA_GET_VALUE( luaManager, "m_iRessurectionCount",	kCube.m_iRessurectionCount,	0 );
	LUA_GET_VALUE( luaManager, "m_iRestoreSpirit",		kCube.m_iRestoreSpirit,		0 );
	LUA_GET_VALUE( luaManager, "m_iRequiredED",			kCube.m_iRequiredED,		0 );
	LUA_GET_VALUE( luaManager, "m_UseCondition",		kCube.m_iUseCondition,		0 );
	LUA_GET_VALUE( luaManager, "m_bGiveAll",			iGiveAll,					0 );

	kCube.m_bGiveAll = ( 0 != iGiveAll );

	if( kCube.m_iItemID <= 0 )
		return;

	// The same GetItemTemplet gate CXSLRandomItemManager applies under
	// SERV_SERVER_ERROR_LOG_BUG_FIX, for the same reason: RandomItemTable.lua
	// carries every region's cubes and this .kom carries one region's items.
	// Counted, not logged per row - there are thousands.
	if( NULL == g_pData || NULL == g_pData->GetItemManager() ||
		NULL == g_pData->GetItemManager()->GetItemTemplet( kCube.m_iItemID ) )
	{
		++m_iCubeDropped;
		return;
	}

	// The resurrection and stamina cubes carry no unit/group rows at all - the
	// real loader synthesises a single ( 0, 0 ) entry for them and skips the
	// table walk entirely (SERV_MODIFY_RANDOMBOX_LOAD, on in this build).
	if( kCube.m_iRessurectionCount > 0 || kCube.m_iRestoreSpirit > 0 )
	{
		kCube.m_vecUnitGroup.push_back( std::make_pair( (int)UC_NONE, 0 ) );
	}
	else
	{
		int iIndex = 1;
		while( true == luaManager.BeginTable( iIndex ) )
		{
			int iUnitClass	= 0;
			int iGroupID	= 0;

			LUA_GET_VALUE( luaManager, "m_cUnitClass",		iUnitClass,	0 );
			LUA_GET_VALUE( luaManager, "m_iItemGroupID",	iGroupID,	0 );

			kCube.m_vecUnitGroup.push_back( std::make_pair( iUnitClass, iGroupID ) );

			++iIndex;
			luaManager.EndTable();
		}
	}

	if( true == kCube.m_vecUnitGroup.empty() )
	{
		++m_iCubeDropped;
		return;
	}

	// Keyed by ( cube, key ). A cube that accepts three different keys is three
	// rows here and one entry in the key table, which is what lets the handler
	// pick the key the player actually holds.
	const std::pair< int, int > kKey( kCube.m_iItemID, kCube.m_iKeyItemID );

	if( m_mapCube.end() != m_mapCube.find( kKey ) )
	{
		// A duplicate ( cube, key ) is a data error the real loader also
		// refuses. First one wins, same as the server's insert().
		++m_iCubeDropped;
		return;
	}

	m_mapCube.insert( std::make_pair( kKey, kCube ) );
	m_mapCubeKey[ kCube.m_iItemID ].insert(
		std::make_pair( kCube.m_iKeyItemID, kCube.m_iRequiredKeyCount ) );

	++m_iCubeRows;
}

void CX2OfflineRandomItem::AddRandomItemGroup_LUA( int iGroupID, int iItemID, float fRate,
												   int iPeriod, int iQuantity )
{
	// Exactly the guard CXSLRandomItemManager::AddRandomItemGroup_LUA uses,
	// including rejecting a zero rate: a 0% case can never be drawn and only
	// costs the lottery a slot.
	if( iGroupID <= 0 || iItemID <= 0 || fRate <= 0.0f || iPeriod < 0 || iQuantity < 0 )
		return;

	KGroupCase kCase;
	kCase.m_iItemID		= iItemID;
	kCase.m_fRate		= fRate;
	kCase.m_iPeriod		= iPeriod;
	kCase.m_iQuantity	= iQuantity;

	m_mapGroup[ iGroupID ].push_back( kCase );
	++m_iGroupCaseRows;
}

void CX2OfflineRandomItem::SetNotifyMsg_LUA( bool bVal )
{
	m_bNotifyMsg = bVal;
}

void CX2OfflineRandomItem::AddAttractionItemInfo_LUA( int iItemID, bool bNotify )
{
	if( iItemID <= 0 )
		return;

	// The attraction list is the "rare enough to announce" list. 1825 rows, and
	// offline there is nobody to announce to - only the bNotify ones are kept,
	// and only so the log can say a rare pull happened.
	if( true == bNotify )
		m_setAnnounced.insert( iItemID );

	++m_iPresentationRows;
}

void CX2OfflineRandomItem::AddSealRandomItemInfo_LUA( int iItemID )
{
	if( iItemID <= 0 )
		return;

	m_setSealed.insert( iItemID );
	++m_iPresentationRows;
}

void CX2OfflineRandomItem::AddNotifyResultItemInfo_LUA( int iItemID )
{
	if( iItemID <= 0 )
		return;

	m_setAnnounced.insert( iItemID );
	++m_iPresentationRows;
}

void CX2OfflineRandomItem::AddCharmItem_LUA( int iItemID, int iBonusItemID )
{
	if( iItemID <= 0 || iBonusItemID <= 0 )
		return;

	m_mapCharmBonus[ iItemID ] = iBonusItemID;
	++m_iPresentationRows;
}

void CX2OfflineRandomItem::AddRandomItemMappingData_LUA( int iIndex, int iCubeID, int /*iKeyItemID*/,
														 int /*iBeforeGroupID*/, int /*iAfterGroupID*/ )
{
	// Bound so the call cannot abort the file; not implemented, because the US
	// RandomItemMapping.lua has every one of its calls commented out and the
	// live US server therefore swaps nothing either. If that ever changes, this
	// line is the thing that says so.
	CX2OfflineLog::Server( L"CUBE     NOTE timed group mapping %d for cube %d is not applied"
		L" offline (SERV_CUBE_IN_ITEM_MAPPING is not implemented here).", iIndex, iCubeID );
}

void CX2OfflineRandomItem::AddRandomItemMappingTime_LUA( int /*iIndex*/, const char* /*szStart*/,
														 const char* /*szEnd*/ )
{
}

//////////////////////////////////////////////////////////////////////////
// Reads

const CX2OfflineRandomItem::KCube* CX2OfflineRandomItem::GetCube( int iItemID, int iKeyItemID )
{
	EnsureLoaded();

	std::map< std::pair< int, int >, KCube >::const_iterator mit =
		m_mapCube.find( std::make_pair( iItemID, iKeyItemID ) );

	if( m_mapCube.end() == mit )
		return NULL;

	return &( mit->second );
}

const std::map< int, int >* CX2OfflineRandomItem::GetKeyList( int iItemID )
{
	EnsureLoaded();

	std::map< int, std::map< int, int > >::const_iterator mit = m_mapCubeKey.find( iItemID );
	if( m_mapCubeKey.end() == mit )
		return NULL;

	return &( mit->second );
}

int CX2OfflineRandomItem::ResolveGroup( int iUnitClass, const KCube& kCube ) const
{
	if( UC_ANYONE == kCube.m_iUseCondition )
	{
		// Not class-dependent: entry [0] is the only one.
		return kCube.m_vecUnitGroup.empty() ? 0 : kCube.m_vecUnitGroup[0].second;
	}

	int iMatch = iUnitClass;

	if( UC_ONE_UNIT == kCube.m_iUseCondition )
	{
		// CXSLUnit::GetUnitClassBaseGrade: collapse a promoted class to the
		// base class of its character. The client has no copy of that function,
		// but it does not need one - the eight base classes are numbered 1..8
		// and CX2Unit::UNIT_TYPE is numbered UT_ELSWORD=1..UT_ELESIS=8 to match,
		// so the unit type IS the base-grade class value. UnitTypeOf already
		// reads it off the client's own unit templet.
		iMatch = CX2OfflineStatTable::UnitTypeOf( iUnitClass );
	}

	for( size_t i = 0; i < kCube.m_vecUnitGroup.size(); ++i )
	{
		if( kCube.m_vecUnitGroup[i].first == iMatch )
			return kCube.m_vecUnitGroup[i].second;
	}

	return 0;
}

bool CX2OfflineRandomItem::GetResult( int iUnitClass, const KCube& kCube,
									  OUT std::vector< KResult >& vecOut )
{
	EnsureLoaded();

	vecOut.clear();

	const int iGroupID = ResolveGroup( iUnitClass, kCube );

	// A resurrection or stamina cube legitimately resolves to no group: its
	// whole payload is the counter on the packet. The live GetResultItem
	// returns true with an empty map for exactly these two.
	if( 0 == iGroupID )
	{
		if( kCube.m_iRessurectionCount > 0 || kCube.m_iRestoreSpirit > 0 )
			return true;

		return false;
	}

	std::map< int, std::vector< KGroupCase > >::const_iterator mit = m_mapGroup.find( iGroupID );
	if( m_mapGroup.end() == mit )
		return false;

	const std::vector< KGroupCase >& vecCase = mit->second;
	if( true == vecCase.empty() )
		return false;

	if( true == kCube.m_bGiveAll )
	{
		for( size_t i = 0; i < vecCase.size(); ++i )
		{
			KResult kResult;
			kResult.m_iItemID	= vecCase[i].m_iItemID;
			kResult.m_iQuantity	= vecCase[i].m_iQuantity;
			kResult.m_iPeriod	= vecCase[i].m_iPeriod;

			vecOut.push_back( kResult );
		}
	}
	else
	{
		// KLottery::Decision: one roll in [0,100), accumulate the cases in file
		// order, take the first whose running total reaches the roll. Drawing
		// nothing is a legal outcome for a group whose rates sum below 100, and
		// it is the reason the live server has a CASE_BLANK at all - so it is
		// reported as a failed open rather than quietly rounded to the last row.
		//
		// rand() % 10000 rather than rand() % 100: rates in this table go to two
		// decimal places (0.01% cases are common in the rare groups) and integer
		// percent would round every one of them out of existence.
		const float fRoll = ( (float)( rand() % 10000 ) ) / 100.0f;

		float fRunning = 0.0f;
		for( size_t i = 0; i < vecCase.size(); ++i )
		{
			fRunning += vecCase[i].m_fRate;

			if( fRoll < fRunning )
			{
				KResult kResult;
				kResult.m_iItemID	= vecCase[i].m_iItemID;
				kResult.m_iQuantity	= vecCase[i].m_iQuantity;
				kResult.m_iPeriod	= vecCase[i].m_iPeriod;

				vecOut.push_back( kResult );
				break;
			}
		}

		if( true == vecOut.empty() )
			return false;
	}

	// The charm bonus, if this cube pays one. It rides alongside the draw in
	// both the give-all and single-draw paths on the live server
	// (SERV_CHARM_ITEM_SCRIPT, on in this build), always one unit, never timed.
	const int iBonusItemID = GetCharmBonusItem( kCube.m_iItemID );
	if( iBonusItemID > 0 )
	{
		KResult kBonus;
		kBonus.m_iItemID	= iBonusItemID;
		kBonus.m_iQuantity	= 1;
		kBonus.m_iPeriod	= 0;

		vecOut.push_back( kBonus );
	}

	return ( false == vecOut.empty() );
}

bool CX2OfflineRandomItem::IsSealedCube( int iItemID )
{
	EnsureLoaded();
	return ( m_setSealed.end() != m_setSealed.find( iItemID ) );
}

bool CX2OfflineRandomItem::IsAnnouncedResult( int iItemID )
{
	EnsureLoaded();
	return ( m_setAnnounced.end() != m_setAnnounced.find( iItemID ) );
}

int CX2OfflineRandomItem::GetCharmBonusItem( int iItemID )
{
	EnsureLoaded();

	std::map< int, int >::const_iterator mit = m_mapCharmBonus.find( iItemID );
	if( m_mapCharmBonus.end() == mit )
		return 0;

	return mit->second;
}

bool CX2OfflineRandomItem::IsLoaded()
{
	EnsureLoaded();
	return ( m_iCubeRows > 0 );
}

#endif SERV_IRUHADEV_OFFLINE
