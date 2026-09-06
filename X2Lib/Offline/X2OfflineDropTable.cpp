#include "stdafx.h"
#include "X2OfflineDropTable.h"

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineLog.h"
#include "X2OfflineLuaEnum.h"

CX2OfflineDropTable* CX2OfflineDropTable::ms_pInstance = NULL;

namespace
{
	const wchar_t* const SCRIPT_NPC_EXP		= L"NpcExpTable.lua";
	const wchar_t* const SCRIPT_DROP_TABLE	= L"DropTable.lua";

	/// Phase 27. The third file of the set, and the one the offline server had
	/// never loaded: every ordinary consumable drop is in here rather than in
	/// DropTable.lua. Not fatal if it is missing - the static drop simply
	/// switches off and EnsureLoaded says which file to pack.
	const wchar_t* const SCRIPT_STATIC_DROP	= L"StaticDropTable.lua";

	/// NpcExpTable.lua's own level-1 row, used only if the file cannot be
	/// loaded at all. The real curve is roughly quadratic; this keeps a
	/// tutorial run from awarding literally nothing while the log shouts.
	const int SYNTHETIC_EXP_L1 = 9;
}

//////////////////////////////////////////////////////////////////////////

CX2OfflineDropTable::CX2OfflineDropTable()
: m_bLoadAttempted( false )
, m_iNpcExpRows( 0 )
, m_iMonsterRows( 0 )
, m_iItemCaseRows( 0 )
, m_iStaticRows( 0 )
, m_iStaticCaseRows( 0 )
{
}

CX2OfflineDropTable::~CX2OfflineDropTable()
{
}

CX2OfflineDropTable* CX2OfflineDropTable::Instance()
{
	if( NULL == ms_pInstance )
		ms_pInstance = new CX2OfflineDropTable;

	return ms_pInstance;
}

void CX2OfflineDropTable::Release()
{
	if( NULL == ms_pInstance )
		return;

	delete ms_pInstance;
	ms_pInstance = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Loading

bool CX2OfflineDropTable::RunScript( const wchar_t* szName )
{
	// Same two-layer dance StatTable.lua needs, for the same two reasons: the
	// .kom container comes off in LoadDataFile, the XOR encryption comes off in
	// DoMemory. See X2OfflineStatTable::EnsureLoaded for the full reasoning -
	// this is deliberately the identical shape.
	//
	// The shipped home for both files is data036.kom, XOR-encrypted, exactly
	// like StatTable.lua and every other client script. MASS_FILE_FIRST means
	// LoadDataFile still falls back to a loose file on disk, which is a useful
	// testing escape hatch but also one that masks a failed repack - so a loose
	// copy should be deleted once the archive carries the file.
	KGCMassFileManager::CMassFile::MASSFILE_MEMBERFILEINFO_POINTER kInfo;
	kInfo = g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LoadDataFile( szName );

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
			L"DROP     ERROR '%s' not found in any .kom or on disk.", szName );
		CX2OfflineLog::Server(
			L"DROP     XOR-encrypt KncWX2Server/ServerResource/US/%s and pack it into data036.kom.",
			szName );
		return false;
	}

	// Every counter, not just the npc-exp and monster ones. The old form named
	// those two explicitly, which was correct while there were two files and
	// silently wrong the moment a third arrived: StaticDropTable.lua touches
	// neither counter, so a perfectly good load would have been reported as
	// "produced no rows" and then re-run as plaintext for nothing.
	const int iRowsBefore = TotalRows();

	bool bRan = ( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemory( kInfo->pRealData, kInfo->size ) );
	bool bAdded = ( TotalRows() > iRowsBefore );

	if( false == bRan || false == bAdded )
	{
		bRan = ( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemoryNotEncript( kInfo->pRealData, kInfo->size ) );
		bAdded = ( TotalRows() > iRowsBefore );

		if( true == bRan && true == bAdded )
		{
			CX2OfflineLog::Server(
				L"DROP     NOTE '%s' is NOT encrypted - loaded as plaintext. Fine for testing;"
				L" encrypt it (XOREncrypt / FileEncrypt) to match every other packed script.",
				szName );
		}
	}

	if( false == bRan || false == bAdded )
	{
		CX2OfflineLog::Server(
			L"DROP     ERROR '%s' produced no rows, encrypted or plaintext.", szName );
		return false;
	}

	return true;
}

void CX2OfflineDropTable::EnsureLoaded()
{
	if( true == m_bLoadAttempted )
		return;

	m_bLoadAttempted = true;

	if( NULL == g_pKTDXApp ||
		NULL == g_pKTDXApp->GetLuaBinder() ||
		NULL == g_pKTDXApp->GetDeviceManager() ||
		NULL == g_pKTDXApp->GetDeviceManager()->GetMassFileManager() )
	{
		CX2OfflineLog::Server( L"DROP     ERROR engine not ready - dungeon EXP and ED will be approximate." );
		return;
	}

	lua_State* pLuaState = g_pKTDXApp->GetLuaBinder()->GetLuaState();
	if( NULL == pLuaState )
	{
		CX2OfflineLog::Server( L"DROP     ERROR no lua state - dungeon EXP and ED will be approximate." );
		return;
	}

	// Bind as the global both files call into, mirroring
	// KDropTable::RegisterLuaBind. Every method the two files reach has to be
	// bound - a missing one aborts the chunk at the first call, and DropTable.lua
	// reaches five of them.
	lua_tinker::class_add< CX2OfflineDropTable >( pLuaState, "CX2OfflineDropTable" );
	lua_tinker::class_def< CX2OfflineDropTable >( pLuaState, "AddNpcExpTable",
		&CX2OfflineDropTable::AddNpcExpTable_LUA );
	lua_tinker::class_def< CX2OfflineDropTable >( pLuaState, "AddMonsterDropInfo",
		&CX2OfflineDropTable::AddMonsterDropInfo_LUA );
	lua_tinker::class_def< CX2OfflineDropTable >( pLuaState, "AddBattleFieldNpcDropInfo",
		&CX2OfflineDropTable::AddBattleFieldNpcDropInfo_LUA );
	lua_tinker::class_def< CX2OfflineDropTable >( pLuaState, "AddHenirMonsterDropInfo",
		&CX2OfflineDropTable::AddHenirMonsterDropInfo_LUA );
	lua_tinker::class_def< CX2OfflineDropTable >( pLuaState, "AddExtraStageMonsterDropInfo",
		&CX2OfflineDropTable::AddExtraStageMonsterDropInfo_LUA );
	lua_tinker::class_def< CX2OfflineDropTable >( pLuaState, "AddToGroup",
		&CX2OfflineDropTable::AddToGroup_LUA );
	lua_tinker::class_def< CX2OfflineDropTable >( pLuaState, "AddStaticDropInfo",
		&CX2OfflineDropTable::AddStaticDropInfo_LUA );
	lua_tinker::class_def< CX2OfflineDropTable >( pLuaState, "AddBattleFieldStaticDropInfo",
		&CX2OfflineDropTable::AddBattleFieldStaticDropInfo_LUA );
	lua_tinker::decl( pLuaState, "DropTable", this );

	// Order matters only for the log: the three files are independent.
	RunScript( SCRIPT_NPC_EXP );
	RunScript( SCRIPT_DROP_TABLE );

	// StaticDropTable.lua writes `Enable = True` on all 170 of its blocks - a
	// bare capitalised global that exists in a GameServer's Lua state and
	// nowhere in the client's. Unpublished it reads as nil, LUA_GET_VALUE falls
	// back to its init value, and because that init value is `true` the file
	// would have loaded perfectly and nobody would ever have found out. It is
	// published as the NUMBER 1 for the reason X2OfflineLuaEnum.cpp explains at
	// length: KLuaManager gates every read on lua_isnumber, which is false for
	// a Lua boolean.
	//
	// The other two files need none of this, which is why the call is here and
	// not at the top of EnsureLoaded - a Publish() failure must not cost them
	// their load.
	if( false == CX2OfflineLuaEnum::Publish() )
	{
		CX2OfflineLog::Server(
			L"DROP     ERROR could not publish True/False - skipping '%s'."
			L" Ordinary consumable drops will be off.", SCRIPT_STATIC_DROP );
	}
	else
	{
		RunScript( SCRIPT_STATIC_DROP );
	}

	CX2OfflineLog::Server( L"DROP     loaded: %d npc-exp row(s), %d monster row(s),"
		L" %d item case(s) in %u group(s), %d static row(s) with %d case(s)",
		m_iNpcExpRows, m_iMonsterRows, m_iItemCaseRows,
		(unsigned int)m_mapDropGroup.size(), m_iStaticRows, m_iStaticCaseRows );

	if( 0 == m_iNpcExpRows )
	{
		CX2OfflineLog::Server(
			L"DROP     WARNING no NPC exp table - a dungeon run will award almost no EXP." );
	}

	// Degrade visibly. This is the one that answers ISSUES.md #17: "Aqua"
	// (item 99811) is in a static row for nearly every dungeon in the game and
	// appears in exactly four AddToGroup lines in the whole of DropTable.lua,
	// only one of which any monster row references - so with this file absent
	// it, and every other ordinary consumable, simply never drops.
	if( 0 == m_iStaticRows )
	{
		CX2OfflineLog::Server(
			L"DROP     WARNING no static drop table - ordinary consumable drops (Aqua,"
			L" Ruve Herb, Whole Grain Flour, ...) are OFF. Pack"
			L" KncWX2Server/ServerResource/US/StaticDropTable.lua into data036.kom." );
	}
}

//////////////////////////////////////////////////////////////////////////
// Bound into Lua as DropTable:*

void CX2OfflineDropTable::AddNpcExpTable_LUA( int iNpcLevel, int iEXP )
{
	if( iNpcLevel <= 0 || iNpcLevel > (int)MAX_NPC_LEVEL || iEXP < 0 )
		return;

	m_mapNpcExp[ iNpcLevel ] = iEXP;
	++m_iNpcExpRows;
}

void CX2OfflineDropTable::ReadDropBlock( const char* szKeyField, bool bBattleField )
{
	// The block is the call's only argument and is still on the Lua stack, so
	// KLuaManager reads its fields directly - the same convention
	// KDropTable::AddMonsterDropInfo_LUA and CX2OfflineStatTable::SetUnitStat_LUA
	// both use.
	KLuaManager luaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState() );

	int		iKey			= 0;
	int		iMonsterID		= -1;
	int		iExp			= 0;
	int		iED				= 0;
	float	fEDProperty		= 0.0f;

	LUA_GET_VALUE( luaManager, szKeyField,	iKey,			0 );
	LUA_GET_VALUE( luaManager, "MonsterID",	iMonsterID,		-1 );
	LUA_GET_VALUE( luaManager, "Exp",		iExp,			0 );
	LUA_GET_VALUE( luaManager, "ED",		iED,			0 );
	LUA_GET_VALUE( luaManager, "EDProperty",fEDProperty,	0.0f );

	if( iMonsterID <= 0 )
		return;

	KNpcReward kReward;
	kReward.m_iExpGate		= iExp;
	kReward.m_iEDPerLevel	= iED;
	kReward.m_fEDProperty	= fEDProperty;

	// DropItemList: { itemID, probability } pairs, read by position exactly as
	// KDropTable::AddMonsterDropInfo_LUA reads them (Inventory-side sibling at
	// KDropTable.cpp:515-551). An item the client cannot resolve is dropped from
	// the list rather than from the monster - the real loader does the same
	// GetItemTemplet check and skips the case.
	if( true == luaManager.BeginTable( "DropItemList" ) )
	{
		int iIndex = 1;
		while( true == luaManager.BeginTable( iIndex ) )
		{
			int		iItemID	= -1;
			float	fProb	= 0.0f;

			LUA_GET_VALUE( luaManager, 1, iItemID,	-1 );
			LUA_GET_VALUE( luaManager, 2, fProb,	0.0f );

			if( iItemID > 0 && fProb > 0.0f )
			{
				KDropCase kCase;
				kCase.m_iID		= iItemID;
				kCase.m_fProb	= fProb;
				kCase.m_bGroup	= false;

				kReward.m_vecItemCase.push_back( kCase );
				++m_iItemCaseRows;
			}

			++iIndex;
			luaManager.EndTable();
		}

		luaManager.EndTable();
	}

	// DropGroupList: { groupID, probability }. The group itself was built by
	// AddToGroup_LUA earlier in the file; a group ID that has not been seen is
	// skipped, which is the check the real loader makes too.
	if( true == luaManager.BeginTable( "DropGroupList" ) )
	{
		int iIndex = 1;
		while( true == luaManager.BeginTable( iIndex ) )
		{
			int		iGroupID	= -1;
			float	fProb		= 0.0f;

			LUA_GET_VALUE( luaManager, 1, iGroupID,	-1 );
			LUA_GET_VALUE( luaManager, 2, fProb,	0.0f );

			if( iGroupID > 0 && fProb > 0.0f &&
				m_mapDropGroup.find( iGroupID ) != m_mapDropGroup.end() )
			{
				KDropCase kCase;
				kCase.m_iID		= iGroupID;
				kCase.m_fProb	= fProb;
				kCase.m_bGroup	= true;

				kReward.m_vecItemCase.push_back( kCase );
				++m_iItemCaseRows;
			}

			++iIndex;
			luaManager.EndTable();
		}

		luaManager.EndTable();
	}

	// A row with no reward of any kind is not worth a map entry: GetNpcReward
	// and GetNpcItemDrop both return nothing for a missing key anyway, and
	// DropTable.lua has thousands of them. The item test is part of the
	// condition now - phase 4 could skip on EXP and ED alone because it did not
	// read items, and keeping that would have thrown away every drop-only row.
	if( iExp <= 0 && iED <= 0 && true == kReward.m_vecItemCase.empty() )
		return;

	if( true == bBattleField )
		m_mapBattleFieldDrop[ std::make_pair( iKey, iMonsterID ) ] = kReward;
	else
		m_mapMonsterDrop[ std::make_pair( iKey, iMonsterID ) ] = kReward;

	++m_iMonsterRows;
}

void CX2OfflineDropTable::AddMonsterDropInfo_LUA()
{
	ReadDropBlock( "DungeonID", false );
}

void CX2OfflineDropTable::AddBattleFieldNpcDropInfo_LUA()
{
	// Battlefield rows are keyed by BattleFieldID - the live values are
	// SEnum::VMI_BATTLE_FIELD_* (40000 and up), the same numbers GetNpcReward
	// is handed for a battlefield room - and go in their own map. They cannot
	// share the dungeon map: both tables use 0 as their wildcard, and six
	// monsters (393, 662, 665, 1115, 3003, 3005) appear under both with
	// different rewards, so one would quietly overwrite the other.
	ReadDropBlock( "BattleFieldID", true );
}

void CX2OfflineDropTable::AddHenirMonsterDropInfo_LUA()
{
	// Henir is keyed by dungeon *mode*, not dungeon ID, so its rows would
	// collide with real dungeon IDs 1, 2, 3. Read and discard: the Henir
	// dungeon is not reachable this phase.
	KLuaManager luaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState() );

	int iMonsterID = -1;
	LUA_GET_VALUE( luaManager, "MonsterID", iMonsterID, -1 );
}

void CX2OfflineDropTable::AddExtraStageMonsterDropInfo_LUA()
{
	// Secret/extra stages: no dungeon key at all, so there is nowhere to put
	// these without shadowing a real row. Read and discard.
	KLuaManager luaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState() );

	int iMonsterID = -1;
	LUA_GET_VALUE( luaManager, "MonsterID", iMonsterID, -1 );
}

void CX2OfflineDropTable::ReadStaticBlock( const char* szKeyField, bool bBattleField )
{
	// KDropTable::AddStaticDropInfo_LUA (KDropTable.cpp:828), which is a much
	// smaller reader than AddMonsterDropInfo_LUA: a key, an Enable flag and one
	// flat DropItemList. No groups, no EXP, no ED - a static row only ever puts
	// an item on the floor.
	KLuaManager luaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState() );

	int		iKey		= -1;
	bool	bEnable		= true;

	LUA_GET_VALUE( luaManager, szKeyField,	iKey,		-1 );
	LUA_GET_VALUE( luaManager, "Enable",	bEnable,	true );

	// -1 rather than 0 is the real loader's invalid marker, and it has to be:
	// 0 is a legitimate key here (the event-drop row). Defaulting to 0 the way
	// ReadDropBlock does would silently file a malformed block as the event
	// table.
	if( -1 == iKey )
		return;

	if( false == bEnable )
		return;

	std::vector<KDropCase> vecCase;

	if( true == luaManager.BeginTable( "DropItemList" ) )
	{
		int iIndex = 1;
		while( true == luaManager.BeginTable( iIndex ) )
		{
			int		iItemID	= -1;
			float	fProb	= 0.0f;

			LUA_GET_VALUE( luaManager, 1, iItemID,	-1 );
			LUA_GET_VALUE( luaManager, 2, fProb,	0.0f );

			if( iItemID > 0 && fProb > 0.0f )
			{
				KDropCase kCase;
				kCase.m_iID		= iItemID;
				kCase.m_fProb	= fProb;
				kCase.m_bGroup	= false;		///< a static row never names a group

				vecCase.push_back( kCase );
				++m_iStaticCaseRows;
			}

			++iIndex;
			luaManager.EndTable();
		}

		luaManager.EndTable();
	}

	if( true == vecCase.empty() )
		return;

	// insert, not operator[] - the real loader refuses a duplicate key and
	// keeps the row it already has, and says so. Same here, minus the shouting:
	// StaticDropTable.lua has no duplicates, so this is a guard, not a case.
	std::map< int, std::vector<KDropCase> >& mapTarget =
		( true == bBattleField ) ? m_mapBattleFieldStaticDrop : m_mapStaticDrop;

	if( mapTarget.find( iKey ) != mapTarget.end() )
		return;

	mapTarget[ iKey ] = vecCase;

	++m_iStaticRows;
}

void CX2OfflineDropTable::AddStaticDropInfo_LUA()
{
	ReadStaticBlock( "DungeonID", false );
}

void CX2OfflineDropTable::AddBattleFieldStaticDropInfo_LUA()
{
	ReadStaticBlock( "BattleFieldID", true );
}

void CX2OfflineDropTable::AddToGroup_LUA( int iGroupID, int iItemID, float fProbability )
{
	if( iGroupID <= 0 || iItemID <= 0 || fProbability <= 0.0f )
		return;

	KDropCase kCase;
	kCase.m_iID		= iItemID;
	kCase.m_fProb	= fProbability;
	kCase.m_bGroup	= false;			///< a group never contains another group

	m_mapDropGroup[ iGroupID ].push_back( kCase );
}

//////////////////////////////////////////////////////////////////////////

/*static*/ int CX2OfflineDropTable::Decide( const std::vector<KDropCase>& vecCase,
											OUT bool& bGroup )
{
	bGroup = false;

	if( true == vecCase.empty() )
		return -1;

	// KLottery::Decision (KncWX2Server/Common/Lottery.cpp:115). The roll is
	// uniform over [0,100) and the cases are accumulated in order; the first
	// whose running total reaches the roll wins, and falling off the end is
	// CASE_BLANK. Two hundredths of a percent of resolution, which is finer
	// than any probability the table actually uses.
	const float fRoll = (float)( rand() % 10000 ) * 0.01f;		///< 0.00 .. 99.99

	float fAccumulate = 0.0f;

	for( size_t i = 0; i < vecCase.size(); ++i )
	{
		fAccumulate += vecCase[i].m_fProb;

		if( fRoll <= fAccumulate )
		{
			bGroup = vecCase[i].m_bGroup;
			return vecCase[i].m_iID;
		}
	}

	return -1;
}

void CX2OfflineDropTable::GetNpcItemDrop( int iKey, bool bBattleField, int iNpcID,
										  OUT std::vector<int>& vecItemID )
{
	vecItemID.clear();

	EnsureLoaded();

	if( iNpcID <= 0 )
		return;

	const std::map< std::pair< int, int >, KNpcReward >& mapDrop =
		( true == bBattleField ) ? m_mapBattleFieldDrop : m_mapMonsterDrop;

	// Specific row first, then the wildcard, the same two-step GetNpcReward does.
	std::map< std::pair< int, int >, KNpcReward >::const_iterator mit;

	mit = mapDrop.find( std::make_pair( iKey, iNpcID ) );
	if( mit == mapDrop.end() )
		mit = mapDrop.find( std::make_pair( 0, iNpcID ) );

	if( mit == mapDrop.end() )
		return;

	//{{ Iruha : 2026-09-06 // offline QoL: repeat the draw for a 3x drop rate
	// One draw is the shipped behaviour, so iDraws == 1 restores it exactly and
	// there is no #else branch to keep in sync. See the flag comment in
	// KTDXLIB/Always.h for why the draw is repeated rather than the probability
	// scaled: Decide is a single weighted pick over one accumulated list, so
	// tripling m_fProb saturates a row that already sums near 100 percent and
	// silently makes every case listed after that point unreachable. Repeating
	// the draw is what the studio's own drop rate event does
	// (KncWX2Server/CenterServer/KDropTable.cpp:1284) and it multiplies the
	// expected item count exactly, leaving every rarity ratio alone.
	int iDraws = 1;

#ifdef SERV_IRUHADEV_OFFLINE_DROP_BOOST
	iDraws = SERV_IRUHADEV_OFFLINE_DROP_DRAWS;
#endif SERV_IRUHADEV_OFFLINE_DROP_BOOST

	for( int iDraw = 0; iDraw < iDraws; ++iDraw )
	{
		bool bGroup = false;

		const int iCase = Decide( mit->second.m_vecItemCase, bGroup );
		if( iCase <= 0 )
			continue;					///< CASE_BLANK - nothing dropped

		if( false == bGroup )
		{
			vecItemID.push_back( iCase );
			continue;
		}

		// The case was a group, so draw once more inside it. A group draw that
		// comes up blank drops nothing, which is how the real one behaves.
		//
		// ONE inner draw however many outer draws there are: a group is a
		// near-uniform selector, not a rarity gate - group 1 is eight rows of
		// 12.5 - so repeating it would saturate the group and pin every group
		// win to its first item. All of the rarity lives in the outer draw's
		// DropGroupList probability, which is what iDraws already multiplies.
		std::map< int, std::vector<KDropCase> >::const_iterator git = m_mapDropGroup.find( iCase );
		if( git == m_mapDropGroup.end() )
			continue;

		bool bInner = false;

		const int iItemID = Decide( git->second, bInner );
		if( iItemID > 0 )
			vecItemID.push_back( iItemID );
	}
	//}} Iruha : 2026-09-06
}

//////////////////////////////////////////////////////////////////////////

void CX2OfflineDropTable::GetStaticDrop( int iKey, bool bBattleField,
										 OUT std::vector<int>& vecItemID )
{
	vecItemID.clear();

	EnsureLoaded();

	const std::map< int, std::vector<KDropCase> >& mapStatic =
		( true == bBattleField ) ? m_mapBattleFieldStaticDrop : m_mapStaticDrop;

	// Exact key only. KDropTable::StaticDropItem does not fall back to key 0 -
	// see the note on GetStaticDrop in the header - and a dungeon with no static
	// row simply has no place-drop, which the real server logs as a clog line
	// and otherwise ignores.
	std::map< int, std::vector<KDropCase> >::const_iterator mit = mapStatic.find( iKey );
	if( mit == mapStatic.end() )
		return;

	// AddMultiProbRate( fUserContribution ) is skipped rather than mirrored:
	// contribution is 1.0 for a solo player who did all the damage, and
	// multiplying every case by 1.0 is a no-op. The same simplification phases
	// 4 and 5 already made for EXP, ED and the monster lottery.
	//{{ Iruha : 2026-09-06 // offline QoL: repeat the draw for a 3x drop rate
	// Its own count, because this row is a different kind of loot: the monster
	// row is where gear comes from, this one is where the ordinary consumables
	// come from. Covers GetEventDrop too, which is StaticDropItem( 0 ) verbatim.
	int iDraws = 1;

#ifdef SERV_IRUHADEV_OFFLINE_DROP_BOOST
	iDraws = SERV_IRUHADEV_OFFLINE_STATIC_DROP_DRAWS;
#endif SERV_IRUHADEV_OFFLINE_DROP_BOOST

	for( int iDraw = 0; iDraw < iDraws; ++iDraw )
	{
		bool bGroup = false;

		const int iItemID = Decide( mit->second, bGroup );
		if( iItemID > 0 )
			vecItemID.push_back( iItemID );
	}
	//}} Iruha : 2026-09-06
}

void CX2OfflineDropTable::GetEventDrop( OUT std::vector<int>& vecItemID )
{
	// KDropTable::EventDropItem is StaticDropItem( 0 ) verbatim
	// (KDropTable.cpp:1894). Key 0 is a real row in StaticDropTable.lua, not a
	// wildcard: Magic Ice Powder, Alchemy Essence and the Unidentified Ancient
	// Fossil, at 8 / 2 / 2 percent.
	GetStaticDrop( 0, false, vecItemID );
}

//////////////////////////////////////////////////////////////////////////

int CX2OfflineDropTable::GetNpcExp( int iNpcLevel ) const
{
	std::map< int, int >::const_iterator mit = m_mapNpcExp.find( iNpcLevel );
	if( mit != m_mapNpcExp.end() )
		return mit->second;

	if( true == m_mapNpcExp.empty() )
	{
		// No table at all. Better a small wrong number than zero, and the load
		// path has already shouted about it.
		return SYNTHETIC_EXP_L1 * ( ( iNpcLevel > 0 ) ? iNpcLevel : 1 );
	}

	// Past the end of the table: clamp to the last row, which is what the
	// server's own std::map lookup effectively does for a level it has seen.
	std::map< int, int >::const_reverse_iterator rit = m_mapNpcExp.rbegin();
	return rit->second;
}

void CX2OfflineDropTable::GetNpcReward( int iKey, bool bBattleField, int iNpcID, int iNpcLevel,
										OUT int& iEXP, OUT int& iED )
{
	iEXP = 0;
	iED  = 0;

	EnsureLoaded();

	if( iNpcID <= 0 )
		return;

	const std::map< std::pair< int, int >, KNpcReward >& mapDrop =
		( true == bBattleField ) ? m_mapBattleFieldDrop : m_mapMonsterDrop;

	// The real lookup, in the real order: the specific row first, then the
	// wildcard row (KDropTable::NormalNpcDropItem:1504-1519). A monster with no
	// row at all is not an error - it simply grants nothing.
	std::map< std::pair< int, int >, KNpcReward >::const_iterator mit;

	mit = mapDrop.find( std::make_pair( iKey, iNpcID ) );
	if( mit == mapDrop.end() )
		mit = mapDrop.find( std::make_pair( 0, iNpcID ) );

	if( mit == mapDrop.end() )
		return;

	const KNpcReward& kReward = mit->second;

	// A level the client never reported comes through as 0 - treat it as 1
	// rather than handing GetNpcExp a level it has no row for.
	const int iLevel = ( iNpcLevel > 0 ) ? iNpcLevel : 1;

	// Exp in the table is a gate, not an amount: NormalNpcDropItem overwrites
	// it with GetNpcExp( npcLevel ) whenever it is above zero. partyCount is 1.
	if( kReward.m_iExpGate > 0 )
		iEXP = GetNpcExp( iLevel );

	if( kReward.m_iEDPerLevel > 0 )
	{
		// EDProperty is the percentage chance the ED drops at all - the real
		// lottery lives in NpcDropItem, which rolls it per kill. Rolling it here
		// keeps the average right instead of paying out on every single monster.
		bool bPaysOut = true;

		if( kReward.m_fEDProperty < 100.0f )
		{
			const float fRoll = (float)( rand() % 10000 ) * 0.01f;		///< 0.00 .. 99.99
			bPaysOut = ( fRoll < kReward.m_fEDProperty );
		}

		if( true == bPaysOut )
			iED = ( 10 + ( kReward.m_iEDPerLevel * iLevel ) );
	}
}

#endif SERV_IRUHADEV_OFFLINE
