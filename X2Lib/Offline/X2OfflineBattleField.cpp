#include "stdafx.h"
#include "X2OfflineBattleField.h"

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineLog.h"

CX2OfflineBattleField* CX2OfflineBattleField::ms_pInstance = NULL;

namespace
{
	const wchar_t* const SCRIPT_BATTLE_FIELD_DATA = L"BattleFieldData.lua";
	const wchar_t* const SCRIPT_SERVER_DATA			= L"BattleFieldServerData.lua";
}

//////////////////////////////////////////////////////////////////////////

CX2OfflineBattleField::CX2OfflineBattleField()
: m_iLoadingFieldID( 0 )
, m_bIndexLoaded( false )
, m_iGroupRows( 0 )
, m_iFieldRows( 0 )
, m_bServerDataLoaded( false )
, m_bServerDataAttempted( false )
, m_iRespawnTimeMin( 15 )
, m_iRespawnTimeMax( 25 )
, m_iDangerValueMax( 4800 )
, m_iDangerValueWarning( 5 )
, m_iDangerEventRate( 4 )
, m_iBossCheckUserCount( 1 )
, m_iEliteMonsterDropValue( 15 )
, m_fEXPFactor( 1.5f )
, m_fEDFactor( 0.8f )
{
}

CX2OfflineBattleField::~CX2OfflineBattleField()
{
}

CX2OfflineBattleField* CX2OfflineBattleField::Instance()
{
	if( NULL == ms_pInstance )
		ms_pInstance = new CX2OfflineBattleField;

	return ms_pInstance;
}

void CX2OfflineBattleField::Release()
{
	if( NULL != ms_pInstance )
	{
		delete ms_pInstance;
		ms_pInstance = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// Loading

void CX2OfflineBattleField::BindGlobal()
{
	lua_State* pLuaState = g_pKTDXApp->GetLuaBinder()->GetLuaState();

	lua_tinker::class_add< CX2OfflineBattleField >( pLuaState, "CX2OfflineBattleField" );
	lua_tinker::class_def< CX2OfflineBattleField >( pLuaState, "AddBattleFieldData",
		&CX2OfflineBattleField::AddBattleFieldData_LUA );
	lua_tinker::class_def< CX2OfflineBattleField >( pLuaState, "AddSpawnMonsterGroup",
		&CX2OfflineBattleField::AddSpawnMonsterGroup_LUA );

	// BattleFieldServerData.lua calls all eleven of these. Only six are read,
	// but every one has to exist or the chunk dies at the first unbound call and
	// the constants after it are silently lost.
	lua_tinker::class_def< CX2OfflineBattleField >( pLuaState, "SetMonsterRespawnFactorByUserCount",
		&CX2OfflineBattleField::SetMonsterRespawnFactorByUserCount_LUA );
	lua_tinker::class_def< CX2OfflineBattleField >( pLuaState, "SetMonsterRespawnTimeMinMax",
		&CX2OfflineBattleField::SetMonsterRespawnTimeMinMax_LUA );
	lua_tinker::class_def< CX2OfflineBattleField >( pLuaState, "SetBattleFieldFactor",
		&CX2OfflineBattleField::SetBattleFieldFactor_LUA );
	lua_tinker::class_def< CX2OfflineBattleField >( pLuaState, "SetBattleFieldEDFactor",
		&CX2OfflineBattleField::SetBattleFieldEDFactor_LUA );
	lua_tinker::class_def< CX2OfflineBattleField >( pLuaState, "SetBattleFieldEXPFactor",
		&CX2OfflineBattleField::SetBattleFieldEXPFactor_LUA );
	lua_tinker::class_def< CX2OfflineBattleField >( pLuaState, "SetDangerousValueMax",
		&CX2OfflineBattleField::SetDangerousValueMax_LUA );
	lua_tinker::class_def< CX2OfflineBattleField >( pLuaState, "SetDangerousValueWarning",
		&CX2OfflineBattleField::SetDangerousValueWarning_LUA );
	lua_tinker::class_def< CX2OfflineBattleField >( pLuaState, "SetBossCheckUserCount",
		&CX2OfflineBattleField::SetBossCheckUserCount_LUA );
	lua_tinker::class_def< CX2OfflineBattleField >( pLuaState, "SetEliteMonsterDropValue",
		&CX2OfflineBattleField::SetEliteMonsterDropValue_LUA );
	lua_tinker::class_def< CX2OfflineBattleField >( pLuaState, "SetDangerousValueEventRate",
		&CX2OfflineBattleField::SetDangerousValueEventRate_LUA );
	lua_tinker::class_def< CX2OfflineBattleField >( pLuaState, "AddEliteMonsterDropInfo",
		&CX2OfflineBattleField::AddEliteMonsterDropInfo_LUA );

	lua_tinker::decl( pLuaState, "g_pBattleFieldManager", this );
}

void CX2OfflineBattleField::UnbindGlobal()
{
	// Hand the name back. CX2BattleFieldManager::OpenScriptFile re-declares it
	// every time it runs, so leaving it pointed here would probably survive - but
	// "probably" is not good enough for a global the shipped scripts call methods
	// on, and a mismatch would surface as a Lua error deep inside content code.
	if( NULL != g_pData )
	{
		lua_tinker::decl( g_pKTDXApp->GetLuaBinder()->GetLuaState(),
			"g_pBattleFieldManager", &g_pData->GetBattleFieldManager() );
	}
}

bool CX2OfflineBattleField::RunScript( const wchar_t* szName, int& iRowsBefore, int& iRowsNow )
{
	KGCMassFileManager::CMassFile::MASSFILE_MEMBERFILEINFO_POINTER kInfo;
	kInfo = g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LoadDataFile( szName );

	if( NULL == kInfo )
	{
		CX2OfflineLog::Server( L"FIELDNPC ERROR '%s' not found in any .kom or on disk.", szName );
		return false;
	}

	iRowsBefore = iRowsNow;

	bool bRan	= ( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemory( kInfo->pRealData, kInfo->size ) );
	bool bAdded	= ( iRowsNow > iRowsBefore );

	if( false == bRan || false == bAdded )
	{
		bRan	= ( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemoryNotEncript( kInfo->pRealData, kInfo->size ) );
		bAdded	= ( iRowsNow > iRowsBefore );
	}

	if( false == bRan || false == bAdded )
	{
		CX2OfflineLog::Server( L"FIELDNPC ERROR '%s' loaded but added nothing.", szName );
		return false;
	}

	return true;
}

void CX2OfflineBattleField::EnsureIndexLoaded()
{
	if( true == m_bIndexLoaded )
		return;

	m_bIndexLoaded = true;

	if( NULL == g_pKTDXApp ||
		NULL == g_pKTDXApp->GetLuaBinder() ||
		NULL == g_pKTDXApp->GetDeviceManager() ||
		NULL == g_pKTDXApp->GetDeviceManager()->GetMassFileManager() )
	{
		CX2OfflineLog::Server( L"FIELDNPC ERROR engine not ready - fields will be empty." );
		return;
	}

	if( NULL == g_pKTDXApp->GetLuaBinder()->GetLuaState() )
	{
		CX2OfflineLog::Server( L"FIELDNPC ERROR no lua state - fields will be empty." );
		return;
	}

	BindGlobal();

	int iBefore = 0;
	RunScript( SCRIPT_BATTLE_FIELD_DATA, iBefore, m_iFieldRows );

	UnbindGlobal();

	CX2OfflineLog::Server( L"FIELDNPC index: %d field(s) from '%s'",
		m_iFieldRows, SCRIPT_BATTLE_FIELD_DATA );
}

bool CX2OfflineBattleField::EnsureFieldLoaded( int iBattleFieldID )
{
	EnsureIndexLoaded();

	std::map< int, KField >::iterator mit = m_mapField.find( iBattleFieldID );
	if( mit == m_mapField.end() )
	{
		CX2OfflineLog::Server( L"FIELDNPC field %d is not in %s",
			iBattleFieldID, SCRIPT_BATTLE_FIELD_DATA );
		return false;
	}

	KField& kField = mit->second;

	if( true == kField.m_bTableLoaded )
		return false == kField.m_vecGroup.empty();

	kField.m_bTableLoaded = true;

	if( true == kField.m_wstrNpcTableFileName.empty() )
	{
		// Legitimate: the rest areas have no monsters at all, and the server logs
		// the same thing rather than treating it as an error
		// (XSLBattleField.cpp:123).
		CX2OfflineLog::Server( L"FIELDNPC field %d declares no NPC_TABLE_FILE_NAME - no monsters",
			iBattleFieldID );
		return false;
	}

	BindGlobal();

	m_iLoadingFieldID = iBattleFieldID;

	int iBefore = 0;
	RunScript( kField.m_wstrNpcTableFileName.c_str(), iBefore, m_iGroupRows );

	m_iLoadingFieldID = 0;

	UnbindGlobal();

	CX2OfflineLog::Server( L"FIELDNPC field %d '%s': %u group(s), level %d, max %d monster(s)",
		iBattleFieldID, kField.m_wstrNpcTableFileName.c_str(),
		(unsigned int)kField.m_vecGroup.size(),
		kField.m_iStandardMonsterLevel, kField.m_iMaxNumberMonster );

	return false == kField.m_vecGroup.empty();
}

void CX2OfflineBattleField::EnsureServerDataLoaded()
{
	if( true == m_bServerDataAttempted )
		return;

	m_bServerDataAttempted = true;

	if( NULL == g_pKTDXApp || NULL == g_pKTDXApp->GetLuaBinder() ||
		NULL == g_pKTDXApp->GetDeviceManager() ||
		NULL == g_pKTDXApp->GetDeviceManager()->GetMassFileManager() ||
		NULL == g_pKTDXApp->GetLuaBinder()->GetLuaState() )
	{
		return;
	}

	// Unlike every other script this subsystem touches, this one is not in the
	// client's .kom - it is the server's own tuning file and was never shipped to
	// players. LoadDataFile will therefore fail unless it has been XOR-encrypted
	// and packed in, so the miss is reported as guidance rather than as an error.
	KGCMassFileManager::CMassFile::MASSFILE_MEMBERFILEINFO_POINTER kInfo;
	kInfo = g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LoadDataFile( SCRIPT_SERVER_DATA );

	if( NULL == kInfo )
	{
		CX2OfflineLog::Server(
			L"FIELDNPC '%s' not packed - using the live US constants as fallbacks"
			L" (respawn %d-%ds, danger rate %d). The middle boss needs this file's"
			L" GET_MIDDLE_BOSS_MONSTER_DROP_RATE and stays disabled without it.",
			SCRIPT_SERVER_DATA, m_iRespawnTimeMin, m_iRespawnTimeMax, m_iDangerEventRate );
		CX2OfflineLog::Server(
			L"FIELDNPC XOR-encrypt KncWX2Server/ServerResource/US/%s and pack it into data036.kom.",
			SCRIPT_SERVER_DATA );
		return;
	}

	BindGlobal();

	// No row counter to check against here: this file only assigns constants, so
	// "did it add anything" cannot be measured. Both decrypt paths are tried and
	// the values are sanity-checked instead.
	const int iRateBefore = m_iDangerEventRate;
	m_iDangerEventRate = -1;

	if( E_FAIL == g_pKTDXApp->GetLuaBinder()->DoMemory( kInfo->pRealData, kInfo->size ) ||
		-1 == m_iDangerEventRate )
	{
		g_pKTDXApp->GetLuaBinder()->DoMemoryNotEncript( kInfo->pRealData, kInfo->size );
	}

	UnbindGlobal();

	if( -1 == m_iDangerEventRate )
	{
		m_iDangerEventRate = iRateBefore;
		CX2OfflineLog::Server( L"FIELDNPC ERROR '%s' loaded but set nothing - fallbacks kept.",
			SCRIPT_SERVER_DATA );
		return;
	}

	m_bServerDataLoaded = true;

	CX2OfflineLog::Server(
		L"FIELDNPC server data: respawn %d-%ds, danger rate %d (max %d, warn %d),"
		L" boss users %d, EXP x%.2f, ED x%.2f",
		m_iRespawnTimeMin, m_iRespawnTimeMax, m_iDangerEventRate,
		m_iDangerValueMax, m_iDangerValueWarning, m_iBossCheckUserCount,
		m_fEXPFactor, m_fEDFactor );
}

bool CX2OfflineBattleField::HasServerData()
{
	EnsureServerDataLoaded();
	return m_bServerDataLoaded;
}

int CX2OfflineBattleField::GetRespawnTimeSec()
{
	EnsureServerDataLoaded();

	const int iSpread = m_iRespawnTimeMax - m_iRespawnTimeMin;
	if( iSpread <= 0 )
		return m_iRespawnTimeMax;

	return m_iRespawnTimeMin + ( rand() % ( iSpread + 1 ) );
}

int CX2OfflineBattleField::GetDangerEventRate()
{
	EnsureServerDataLoaded();
	return ( m_iDangerEventRate > 0 ) ? m_iDangerEventRate : 1;
}

int CX2OfflineBattleField::GetDangerValueMax()
{
	EnsureServerDataLoaded();
	return m_iDangerValueMax;
}

int CX2OfflineBattleField::GetDangerValueWarning()
{
	EnsureServerDataLoaded();
	return m_iDangerValueWarning;
}

int CX2OfflineBattleField::GetBossCheckUserCount()
{
	EnsureServerDataLoaded();
	return m_iBossCheckUserCount;
}

float CX2OfflineBattleField::GetEXPFactor()
{
	EnsureServerDataLoaded();
	return ( m_fEXPFactor > 0.0f ) ? m_fEXPFactor : 1.0f;
}

float CX2OfflineBattleField::GetEDFactor()
{
	EnsureServerDataLoaded();
	return ( m_fEDFactor > 0.0f ) ? m_fEDFactor : 1.0f;
}

float CX2OfflineBattleField::GetMiddleBossDropRate( int iCurDanger, int iOldDanger )
{
	// The escalation curve is a Lua function, not a constant - it returns 100 the
	// moment the danger value crosses a multiple of its internal threshold. There
	// is no way to approximate it honestly, so without the file there is no
	// middle boss.
	if( false == HasServerData() )
		return 0.0f;

	lua_State* pLuaState = g_pKTDXApp->GetLuaBinder()->GetLuaState();
	if( NULL == pLuaState )
		return 0.0f;

	float fRate = 0.0f;
	try
	{
		fRate = lua_tinker::call< float >( pLuaState, "GET_MIDDLE_BOSS_MONSTER_DROP_RATE",
			iCurDanger, iOldDanger );
	}
	catch( ... )
	{
		return 0.0f;
	}

	return fRate;
}

//////////////////////////////////////////////////////////////////////////
// Bound into Lua

void CX2OfflineBattleField::SetMonsterRespawnFactorByUserCount_LUA( int /*iFactor*/ )
{
	// Scales respawn speed by how many players are in the field. Always one
	// player offline, so there is nothing to scale.
}

void CX2OfflineBattleField::SetMonsterRespawnTimeMinMax_LUA( int iMin, int iMax )
{
	if( iMin > 0 && iMax >= iMin )
	{
		m_iRespawnTimeMin = iMin;
		m_iRespawnTimeMax = iMax;
	}
}

void CX2OfflineBattleField::SetBattleFieldFactor_LUA( float /*fFactor*/ )
{
	// Superseded by the separate ED and EXP factors below; the call is commented
	// out in the shipped file but the binding has to exist regardless.
}

void CX2OfflineBattleField::SetBattleFieldEDFactor_LUA( float fFactor )
{
	if( fFactor > 0.0f )
		m_fEDFactor = fFactor;
}

void CX2OfflineBattleField::SetBattleFieldEXPFactor_LUA( float fFactor )
{
	if( fFactor > 0.0f )
		m_fEXPFactor = fFactor;
}

void CX2OfflineBattleField::SetDangerousValueMax_LUA( int iValue )
{
	if( iValue > 0 )
		m_iDangerValueMax = iValue;
}

void CX2OfflineBattleField::SetDangerousValueWarning_LUA( int iValue )
{
	if( iValue >= 0 )
		m_iDangerValueWarning = iValue;
}

void CX2OfflineBattleField::SetBossCheckUserCount_LUA( int iCount )
{
	if( iCount >= 0 )
		m_iBossCheckUserCount = iCount;
}

void CX2OfflineBattleField::SetEliteMonsterDropValue_LUA( int iValue )
{
	if( iValue > 0 )
		m_iEliteMonsterDropValue = iValue;
}

void CX2OfflineBattleField::SetDangerousValueEventRate_LUA( int iRate )
{
	if( iRate > 0 )
		m_iDangerEventRate = iRate;
}

void CX2OfflineBattleField::AddEliteMonsterDropInfo_LUA( int /*iMin*/, int /*iMax*/,
														 int /*iDifficulty*/, int /*iValue*/ )
{
	// Which difficulty of attribute-enchanted monster the danger value buys.
	// Ignored: the stat rolls come from AttribNpcTable.lua, which is server-only
	// too, so no elite monster can actually be built.
}

void CX2OfflineBattleField::AddBattleFieldData_LUA()
{
	KLuaManager kLua( g_pKTDXApp->GetLuaBinder()->GetLuaState() );

	int iBattleFieldID = 0;
	LUA_GET_VALUE( kLua, "BATTLE_FIELD_ID", iBattleFieldID, 0 );

	if( 0 == iBattleFieldID )
		return;

	KField kField;
	LUA_GET_VALUE( kLua, "NPC_TABLE_FILE_NAME", kField.m_wstrNpcTableFileName, L"" );
	LUA_GET_VALUE( kLua, "STANDARD_MONSTER_LEVEL", kField.m_iStandardMonsterLevel, 1 );
	LUA_GET_VALUE( kLua, "MAX_NUMBER_OF_MONSTER_IN_THIS_BATTLE_FIELD", kField.m_iMaxNumberMonster, 0 );

	// BATTLE_FIELD_MIDDLE_BOSS_INFO. This one IS in the client's copy of the
	// file - the full-boss table (BATTLE_FIELD_RISK_INFO) is not, matching the
	// fact that the full boss is compiled out of this build.
	if( true == kLua.BeginTable( "BATTLE_FIELD_MIDDLE_BOSS_INFO" ) )
	{
		int iIndex = 1;
		while( true == kLua.BeginTable( iIndex ) )
		{
			KMiddleBoss kBoss;
			LUA_GET_VALUE( kLua, "SPAWN_ID",				kBoss.m_iSpawnID,			0 );
			LUA_GET_VALUE( kLua, "SPAWN_RATE",				kBoss.m_iSpawnRate,			0 );
			LUA_GET_VALUE( kLua, "RISK_VALUE",				kBoss.m_iRiskValue,			0 );
			LUA_GET_VALUE( kLua, "SHOW_BOSS_NAME",			kBoss.m_bShowBossName,		false );
			LUA_GET_VALUE( kLua, "BOSS_GAUGE_HP_LINES",		kBoss.m_bBossGaugeHpLines,	false );

			if( true == kLua.BeginTable( "SET_BOSS_GROUP_NAME" ) )
			{
				int i = 1;
				int iBuf = -1;
				while( true == kLua.GetValue( i, iBuf ) )
				{
					if( iBuf > 0 )
						kBoss.m_vecSpawnGroupID.push_back( iBuf );
					++i;
				}
				kLua.EndTable();
			}

			if( 0 != kBoss.m_iSpawnID && false == kBoss.m_vecSpawnGroupID.empty() )
				kField.m_vecMiddleBoss.push_back( kBoss );

			++iIndex;
			kLua.EndTable();
		}
		kLua.EndTable();
	}

	m_mapField[ iBattleFieldID ] = kField;
	++m_iFieldRows;
}

void CX2OfflineBattleField::AddSpawnMonsterGroup_LUA()
{
	// NOTE the comparisons below are `true ==`, not `S_OK ==`. There are two
	// KLuaManager classes in this tree with the same method names and different
	// return types: the server's (KncWX2Server/Common/Lua/KLuaManager.h) returns
	// HRESULT, the client's (luaLib/KLuaManager.h) returns bool. Since S_OK is 0,
	// porting the server's `== S_OK` idiom into client code inverts every test -
	// and an inverted BeginTable in a while loop does not fail visibly, it spins
	// forever. That is exactly what it did: the field's BGM kept playing while
	// the game stopped responding, because the hang is on the main thread inside
	// this parser.
	KLuaManager kLua( g_pKTDXApp->GetLuaBinder()->GetLuaState() );

	std::map< int, KField >::iterator mitField = m_mapField.find( m_iLoadingFieldID );
	if( mitField == m_mapField.end() )
		return;

	KSpawnGroup kGroup;
	LUA_GET_VALUE( kLua, "SPAWN_GROUP_ID", kGroup.m_iGroupID, 0 );
	LUA_GET_VALUE( kLua, "IS_BOSS_MONSTER_GROUP", kGroup.m_bIsBossMonsterGroup, false );
	LUA_GET_VALUE( kLua, "MIN_NUMBER_OF_MONSTER_IN_THIS_GROUP", kGroup.m_iMinNumberOfMonsterInThisGroup, 0 );
	LUA_GET_VALUE( kLua, "MAX_NUMBER_OF_MONSTER_IN_THIS_GROUP", kGroup.m_iMaxNumberOfMonsterInThisGroup, 0 );

	if( true == kLua.BeginTable( "SIEGE_MONSTER" ) )
	{
		int iIndex = 1;
		int iBuf = -1;
		while( true == kLua.GetValue( iIndex, iBuf ) )
		{
			if( iBuf > 0 )
				kGroup.m_setSiegeMonster.insert( iBuf );
			++iIndex;
		}
		kLua.EndTable();
	}

	// The monsters themselves are the group table's numbered entries.
	int iIndex = 1;
	while( true == kLua.BeginTable( iIndex ) )
	{
		KSpawnNpc kNpc;
		LUA_GET_VALUE( kLua, "MONSTER_ID",					kNpc.m_iNpcID,					0 );
		LUA_GET_VALUE( kLua, "RATE",						kNpc.m_fRate,					0.0f );
		LUA_GET_VALUE( kLua, "MAX_NUMBER_OF_THIS_MONSTER",	kNpc.m_iMaxNumberOfThisMonster,	0 );
		LUA_GET_VALUE( kLua, "ADD_LEVEL",					kNpc.m_iAddLevel,				0 );
		LUA_GET_VALUE( kLua, "DROP",						kNpc.m_bDrop,					false );
		LUA_GET_VALUE( kLua, "AGRESSIVE",					kNpc.m_bAggressive,				true );
		LUA_GET_VALUE( kLua, "ADD_POS_Y",					kNpc.m_fAddPosY,				0.0f );

		if( true == kLua.BeginTable( "START_POSITION" ) )
		{
			int i = 1;
			int iBuf = -1;
			while( true == kLua.GetValue( i, iBuf ) )
			{
				if( iBuf > 0 )
					kNpc.m_vecStartPosition.push_back( iBuf );
				++i;
			}
			kLua.EndTable();
		}

		if( true == kLua.BeginTable( "PETROL_LINE_INDEX" ) )
		{
			int i = 1;
			int iBuf = -1;
			while( true == kLua.GetValue( i, iBuf ) )
			{
				if( iBuf > 0 )
					kNpc.m_vecPetrolLineIndex.push_back( iBuf );
				++i;
			}
			kLua.EndTable();
		}

		if( true == kLua.BeginTable( "PLAY_LINE_INDEX" ) )
		{
			int i = 1;
			int iBuf = -1;
			while( true == kLua.GetValue( i, iBuf ) )
			{
				if( iBuf > 0 )
					kNpc.m_vecPlayLineIndex.push_back( iBuf );
				++i;
			}
			kLua.EndTable();
		}

		if( 0 != kNpc.m_iNpcID )
			kGroup.m_mapSpawnNpc[ kNpc.m_iNpcID ] = kNpc;

		++iIndex;
		kLua.EndTable();
	}

	// Every group is kept by ID, because the middle boss names its groups by ID
	// and those are precisely the ones flagged IS_BOSS_MONSTER_GROUP. Only the
	// non-boss ones go in the vector the opening fill and respawn iterate, which
	// is the same split the server keeps (m_mapNpcGroupList vs
	// m_vecRespawnNpcGroupList).
	mitField->second.m_mapAllGroup[ kGroup.m_iGroupID ] = kGroup;

	if( false == kGroup.m_bIsBossMonsterGroup )
		mitField->second.m_vecGroup.push_back( kGroup );

	++m_iGroupRows;
}

//////////////////////////////////////////////////////////////////////////
// Building the field's population

bool CX2OfflineBattleField::PickFromGroup( const KSpawnGroup& kGroup,
										   const std::map< int, int >& mapAliveByGroup,
										   const std::map< std::pair< int, int >, int >& mapAliveByNpc,
										   int iStandardLevel,
										   OUT KNPCUnitReq& kOut ) const
{
	// 1. Candidates: everything in the group not already at its own cap, and
	//    nothing at all once the group is at its cap.
	std::map< int, int >::const_iterator mitG = mapAliveByGroup.find( kGroup.m_iGroupID );
	const int iAliveInGroup = ( mitG != mapAliveByGroup.end() ) ? mitG->second : 0;

	if( iAliveInGroup >= kGroup.m_iMaxNumberOfMonsterInThisGroup )
		return false;

	std::vector< const KSpawnNpc* > vecCase;
	float fTotalProb = 0.0f;

	std::map< int, KSpawnNpc >::const_iterator mit;
	for( mit = kGroup.m_mapSpawnNpc.begin(); mit != kGroup.m_mapSpawnNpc.end(); ++mit )
	{
		const KSpawnNpc& kNpc = mit->second;

		std::map< std::pair< int, int >, int >::const_iterator mitN =
			mapAliveByNpc.find( std::make_pair( kGroup.m_iGroupID, kNpc.m_iNpcID ) );
		const int iAliveOfThis = ( mitN != mapAliveByNpc.end() ) ? mitN->second : 0;

		if( iAliveOfThis >= kNpc.m_iMaxNumberOfThisMonster )
			continue;

		vecCase.push_back( &kNpc );
		fTotalProb += kNpc.m_fRate;
	}

	if( true == vecCase.empty() )
		return false;

	// 2. The server's own normalisation: whatever is left of 100% is shared out
	//    evenly across the candidates, so a table whose rates sum to less than
	//    100 still always produces a monster (XSLBattleField.cpp:508-515).
	const float fAddProb = ( 100.0f - fTotalProb ) / (float)vecCase.size();

	float fSum = 0.0f;
	size_t i = 0;
	for( i = 0; i < vecCase.size(); ++i )
	{
		float f = vecCase[i]->m_fRate;
		if( fAddProb > 0.0f )
			f += fAddProb;
		fSum += f;
	}

	if( fSum <= 0.0f )
		return false;

	const float fRoll = ( (float)( rand() % 10000 ) / 10000.0f ) * fSum;

	const KSpawnNpc* pChosen = vecCase.back();
	float fWalk = 0.0f;
	for( i = 0; i < vecCase.size(); ++i )
	{
		float f = vecCase[i]->m_fRate;
		if( fAddProb > 0.0f )
			f += fAddProb;

		fWalk += f;
		if( fRoll < fWalk )
		{
			pChosen = vecCase[i];
			break;
		}
	}

	// 3. MakeMonsterInfo (XSLBattleField.cpp:618). No NPC templet check: that is
	//    the server validating its own table, and an ID the client cannot build
	//    simply fails to appear rather than corrupting anything.
	kOut.Init();
	kOut.m_iGroupID		= kGroup.m_iGroupID;
	kOut.m_NPCID		= pChosen->m_iNpcID;
	kOut.m_Level		= (char)( iStandardLevel + pChosen->m_iAddLevel );
	kOut.m_bAggressive	= pChosen->m_bAggressive;

	if( true == pChosen->m_vecStartPosition.empty() )
	{
		// The fallback the server uses when a monster declares no START_POSITION:
		// drop it on one of the lines it patrols.
		if( false == pChosen->m_vecPetrolLineIndex.empty() )
		{
			kOut.m_nStartPos =
				pChosen->m_vecPetrolLineIndex[ rand() % pChosen->m_vecPetrolLineIndex.size() ];
		}
		else
		{
			kOut.m_nStartPos = -1;
		}
	}
	else
	{
		kOut.m_nStartPos =
			pChosen->m_vecStartPosition[ rand() % pChosen->m_vecStartPosition.size() ];
	}

	ApplyStartPos( kOut );

	kOut.m_vecPetrolLineIndex	= pChosen->m_vecPetrolLineIndex;
	kOut.m_vecPlayLineIndex		= pChosen->m_vecPlayLineIndex;
	kOut.m_bActive				= true;
	kOut.m_bShowGage			= true;
	kOut.m_AddPos.y				= pChosen->m_fAddPosY;
	kOut.m_bSiegeMode			= kGroup.IsSiegeMonster( pChosen->m_iNpcID );
	kOut.m_bNoDrop				= !( pChosen->m_bDrop );
	kOut.m_fUnitScale			= 1.0f;
	kOut.m_cMonsterGrade		= 0;						///< MG_NORMAL_NPC
	kOut.m_cTeamNum				= (char)CX2Room::TN_MONSTER;
	kOut.m_iAllyUID				= -1;

	return true;
}

void CX2OfflineBattleField::GetCreateMonsterList( int iBattleFieldID, int /*iPlayerCount*/,
												  OUT std::vector< KNPCUnitReq >& vecOut )
{
	vecOut.clear();

	if( false == EnsureFieldLoaded( iBattleFieldID ) )
		return;

	const KField& kField = m_mapField[ iBattleFieldID ];

	std::map< int, int >						mapAliveByGroup;
	std::map< std::pair< int, int >, int >		mapAliveByNpc;

	int iRemaining = kField.m_iMaxNumberMonster;

	// Round-robin across the groups, one monster per group per pass, until the
	// field is full or no group can contribute - verbatim
	// CXSLBattleField::GetCreateMonsterList (XSLBattleField.cpp:570-612).
	while( iRemaining > 0 )
	{
		bool bCreatedAny = false;

		for( size_t g = 0; g < kField.m_vecGroup.size(); ++g )
		{
			if( iRemaining <= 0 )
				break;

			const KSpawnGroup& kGroup = kField.m_vecGroup[g];

			KNPCUnitReq kReq;
			if( false == PickFromGroup( kGroup, mapAliveByGroup, mapAliveByNpc,
										kField.m_iStandardMonsterLevel, kReq ) )
			{
				continue;
			}

			vecOut.push_back( kReq );

			++mapAliveByGroup[ kGroup.m_iGroupID ];
			++mapAliveByNpc[ std::make_pair( kGroup.m_iGroupID, kReq.m_NPCID ) ];

			--iRemaining;
			bCreatedAny = true;
		}

		if( false == bCreatedAny )
			break;
	}
}

void CX2OfflineBattleField::ApplyStartPos( OUT KNPCUnitReq& kOut ) const
{
	// GetMonsterStartPosInfo's client-side equal: the location manager resolves a
	// start-position index to a world position and a facing, and it is the same
	// table CX2StateBattleField uses to place the player (X2StateBattleField.cpp:884).
	if( kOut.m_nStartPos < 0 || NULL == g_pData || NULL == g_pData->GetLocationManager() )
		return;

	const D3DXVECTOR3 vPos = g_pData->GetLocationManager()->GetStartPosLoc( kOut.m_nStartPos );
	kOut.m_vPos.x	= vPos.x;
	kOut.m_vPos.y	= vPos.y;
	kOut.m_vPos.z	= vPos.z;
	kOut.m_bIsRight	= g_pData->GetLocationManager()->GetIsRight( kOut.m_nStartPos );
}

void CX2OfflineBattleField::GetRespawnMonsterList(
		int iBattleFieldID,
		const std::map< int, int >& mapAliveByGroup,
		const std::map< std::pair< int, int >, int >& mapAliveByNpc,
		const std::map< int, int >& mapRespawnReadyByGroup,
		OUT std::vector< KNPCUnitReq >& vecOut )
{
	vecOut.clear();

	if( false == EnsureFieldLoaded( iBattleFieldID ) )
		return;

	const KField& kField = m_mapField[ iBattleFieldID ];

	// Running copies: each monster placed here counts against the caps for the
	// ones after it, exactly as the server threads kTotalMonsterCountInfo through
	// the loop (XSLBattleField.cpp:630).
	std::map< int, int >					mapGroup = mapAliveByGroup;
	std::map< std::pair< int, int >, int >	mapNpc   = mapAliveByNpc;

	std::map< int, int >::const_iterator mit;
	for( mit = mapRespawnReadyByGroup.begin(); mit != mapRespawnReadyByGroup.end(); ++mit )
	{
		const int iGroupID		= mit->first;
		const int iRespawnCount	= mit->second;

		std::map< int, KSpawnGroup >::const_iterator mitG = kField.m_mapAllGroup.find( iGroupID );
		if( mitG == kField.m_mapAllGroup.end() )
			continue;

		// A boss group never respawns on its own - it only ever arrives through
		// the middle-boss path, and refilling it here would spawn a boss for free.
		if( true == mitG->second.m_bIsBossMonsterGroup )
			continue;

		int i = 0;
		for( i = 0; i < iRespawnCount; ++i )
		{
			KNPCUnitReq kReq;
			if( false == PickFromGroup( mitG->second, mapGroup, mapNpc,
										kField.m_iStandardMonsterLevel, kReq ) )
			{
				// The group is at its cap. The server gives up on the rest of this
				// group's queue too rather than spinning.
				break;
			}

			vecOut.push_back( kReq );

			++mapGroup[ iGroupID ];
			++mapNpc[ std::make_pair( iGroupID, kReq.m_NPCID ) ];
		}
	}
}

void CX2OfflineBattleField::MakeMiddleBoss( const KSpawnGroup& kGroup, const KSpawnNpc& kNpc,
											const KMiddleBoss& kBoss, int iStandardLevel,
											OUT KNPCUnitReq& kOut ) const
{
	kOut.Init();
	kOut.m_NPCID		= kNpc.m_iNpcID;
	kOut.m_iGroupID		= 0;					///< the server zeroes this for a boss
	kOut.m_iBossGroupID	= kBoss.m_iSpawnID;
	kOut.m_Level		= (char)iStandardLevel;	///< no ADD_LEVEL for a boss
	kOut.m_bAggressive	= kNpc.m_bAggressive;

	if( true == kNpc.m_vecStartPosition.empty() )
	{
		if( false == kNpc.m_vecPetrolLineIndex.empty() )
			kOut.m_nStartPos = kNpc.m_vecPetrolLineIndex[ rand() % kNpc.m_vecPetrolLineIndex.size() ];
		else
			kOut.m_nStartPos = -1;
	}
	else
	{
		kOut.m_nStartPos = kNpc.m_vecStartPosition[ rand() % kNpc.m_vecStartPosition.size() ];
	}

	ApplyStartPos( kOut );

	kOut.m_vecPetrolLineIndex	= kNpc.m_vecPetrolLineIndex;
	kOut.m_vecPlayLineIndex		= kNpc.m_vecPlayLineIndex;
	kOut.m_bActive				= true;
	kOut.m_bShowGage			= true;
	kOut.m_AddPos.y				= kNpc.m_fAddPosY;
	kOut.m_bShowBossName		= kBoss.m_bShowBossName;
	kOut.m_usBossGaugeHPLines	= (u_short)( ( true == kBoss.m_bBossGaugeHpLines ) ? 1 : 0 );
	kOut.m_bFocusCamera			= true;
	kOut.m_bSiegeMode			= kGroup.IsSiegeMonster( kNpc.m_iNpcID );
	kOut.m_bNoDrop				= !( kNpc.m_bDrop );
	kOut.m_fUnitScale			= 1.0f;
	kOut.m_cMonsterGrade		= 2;					///< MG_MIDDLE_BOSS_NPC
	kOut.m_cTeamNum				= (char)CX2Room::TN_MONSTER;
	kOut.m_iAllyUID				= -1;
}

void CX2OfflineBattleField::GetMiddleBossMonsterInfo( int iBattleFieldID, int iDangerValue,
													  OUT std::vector< KNPCUnitReq >& vecOut )
{
	vecOut.clear();

	if( false == EnsureFieldLoaded( iBattleFieldID ) )
		return;

	const KField& kField = m_mapField[ iBattleFieldID ];

	if( true == kField.m_vecMiddleBoss.empty() )
		return;

	// One weighted pick across the SPAWN_RATEs, then only that entry spawns -
	// and only if the danger value has reached its RISK_VALUE. Note the server
	// picks first and filters second, so a roll that lands on an entry the danger
	// value has not unlocked yields no boss at all rather than falling through to
	// a cheaper one. Kept, because changing it would make bosses appear sooner
	// than the live game.
	int iTotalRate = 0;
	size_t i = 0;
	for( i = 0; i < kField.m_vecMiddleBoss.size(); ++i )
		iTotalRate += kField.m_vecMiddleBoss[i].m_iSpawnRate;

	if( iTotalRate <= 0 )
		return;

	const int iRoll = rand() % iTotalRate;

	const KMiddleBoss* pBoss = NULL;
	int iWalk = 0;
	for( i = 0; i < kField.m_vecMiddleBoss.size(); ++i )
	{
		iWalk += kField.m_vecMiddleBoss[i].m_iSpawnRate;
		if( iRoll < iWalk )
		{
			pBoss = &kField.m_vecMiddleBoss[i];
			break;
		}
	}

	if( NULL == pBoss )
		return;

	if( pBoss->m_iRiskValue > iDangerValue )
		return;

	// Everything in every group the entry names, no lottery: a middle boss is a
	// set piece, not a random draw (XSLBattleField.cpp:770-802).
	for( i = 0; i < pBoss->m_vecSpawnGroupID.size(); ++i )
	{
		std::map< int, KSpawnGroup >::const_iterator mitG =
			kField.m_mapAllGroup.find( pBoss->m_vecSpawnGroupID[i] );

		if( mitG == kField.m_mapAllGroup.end() )
		{
			CX2OfflineLog::Server( L"FIELDNPC field %d middle boss %d names missing group %d",
				iBattleFieldID, pBoss->m_iSpawnID, pBoss->m_vecSpawnGroupID[i] );
			continue;
		}

		std::map< int, KSpawnNpc >::const_iterator mitN;
		for( mitN = mitG->second.m_mapSpawnNpc.begin();
			 mitN != mitG->second.m_mapSpawnNpc.end(); ++mitN )
		{
			KNPCUnitReq kReq;
			MakeMiddleBoss( mitG->second, mitN->second, *pBoss,
				kField.m_iStandardMonsterLevel, kReq );

			vecOut.push_back( kReq );
		}
	}
}

#endif SERV_IRUHADEV_OFFLINE
