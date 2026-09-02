#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-01
// Description: Offline mode - dungeon EXP and ED rewards (phase 4).
//
//              The second piece of game data the client does not have, and for
//              the same reason as the stat table (see X2OfflineStatTable.h):
//              the numbers lived on the server, not in any .kom.
//
//              What the plan assumed, and why it is wrong: phase 4 says
//              "accept the client's reported EXP, ED, drops and clear rank".
//              The client reports no such thing. KEGS_NPC_UNIT_DIE_REQ carries
//              the dead NPC's UID, who killed it and how, and
//              KEGS_MY_USER_UNIT_INFO_TO_SERVER_REQ carries combat *scores*.
//              EXP and ED were computed server-side, per kill, out of two
//              tables, and pushed back down in KEGS_NPC_UNIT_DIE_NOT's
//              m_EXPList - which is what CX2Game::ProcessExpListByNpcUnitDie
//              adds to the client's own unit. So the offline server has to own
//              these tables too.
//
//              Both files are the studio's own, loaded exactly the way
//              X2OfflineStatTable loads StatTable.lua - bound into the client's
//              Lua state as the global `DropTable` and run through the
//              mass-file loader. Like StatTable.lua they ship XOR-encrypted
//              inside data036.kom, which is where every other client script
//              lives; a loose copy still resolves through the same code path
//              but masks a failed repack, so it should not be left behind:
//
//                NpcExpTable.lua   DropTable:AddNpcExpTable( npcLevel, exp )
//                                  EXP for killing an NPC of that level.
//
//                DropTable.lua     DropTable:AddMonsterDropInfo{ DungeonID,
//                                  MonsterID, Exp, ED, EDProperty, ... }
//                                  Only three fields matter here: Exp is a
//                                  *gate*, not an amount (any value above zero
//                                  means "this monster gives EXP"), ED is a
//                                  per-level multiplier, and EDProperty is the
//                                  chance ED drops at all.
//
//              The arithmetic is KDropTable::NormalNpcDropItem's, verbatim
//              (KncWX2Server/CenterServer/KDropTable.cpp:1523-1535):
//
//                exp = GetNpcExp( npcLevel ) / partyCount     if Exp > 0
//                ed  = ( 10 + ED * npcLevel ) / partyCount    if ED  > 0
//
//              partyCount is always 1 offline. The item lottery in the same
//              function is deliberately NOT implemented - see the note on
//              GetNpcReward.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

class CX2OfflineDropTable
{
public:
	enum
	{
		/// NpcExpTable.lua runs to level 80, like StatTable.lua.
		MAX_NPC_LEVEL = 80,
	};

public:
	static CX2OfflineDropTable*	Instance();
	static void					Release();

	/// EXP and ED for one killed NPC. Both come back 0 when the monster has no
	/// row, or has a row with the reward gated off - which is correct and
	/// common: plenty of rows are set to Exp = 0 (props, allies, summons).
	///
	/// iKey is the dungeon ID plus its difficulty, or the battlefield ID, and
	/// bBattleField says which - the two are separate tables in the file
	/// (AddMonsterDropInfo vs AddBattleFieldNpcDropInfo) and have to stay
	/// separate here. Real IDs never collide (dungeons are 30000+, battlefields
	/// 40000+), but *both* use 0 as their wildcard, and six monsters appear
	/// under both wildcards with different rewards. Merging the two maps let
	/// the battlefield row - Exp = 0 for all six - silently overwrite the
	/// dungeon row and cost those monsters their EXP in a dungeon.
	///
	/// Item drops are not returned. They are phase 5 work, not an oversight:
	/// an item has to land in an inventory, and the inventory does not
	/// round-trip through SQLite until then (EGS_SELECT_UNIT_1_NOT still sends
	/// an empty m_mapItem). Dropping items a relog would silently eat is worse
	/// than dropping none.
	void	GetNpcReward( int iKey, bool bBattleField, int iNpcID, int iNpcLevel,
						  OUT int& iEXP, OUT int& iED );

	bool	IsLoaded() const					{ return false == m_mapNpcExp.empty(); }

	//////////////////////////////////////////////////////////////////////////
	// Bound into the Lua state as DropTable:*. Public because lua_tinker needs
	// to take their addresses; nothing else should call them.

	/// NpcExpTable.lua: DropTable:AddNpcExpTable( npcLevel, exp )
	void	AddNpcExpTable_LUA( int iNpcLevel, int iEXP );

	/// DropTable.lua: DropTable:AddMonsterDropInfo{ ... }. The table is the
	/// only argument and is read off the Lua stack, which is exactly how
	/// KDropTable::AddMonsterDropInfo_LUA does it.
	void	AddMonsterDropInfo_LUA();

	/// The other three block kinds in DropTable.lua. They have to be bound or
	/// the chunk errors out on the first one it reaches; only the battlefield
	/// one carries rewards this phase can reach, and it is keyed by
	/// BattleFieldID rather than DungeonID.
	void	AddBattleFieldNpcDropInfo_LUA();
	void	AddHenirMonsterDropInfo_LUA();
	void	AddExtraStageMonsterDropInfo_LUA();

	/// DropTable:AddToGroup( groupID, itemID, probability ) - the item-group
	/// lottery. Ignored: no items drop this phase.
	void	AddToGroup_LUA( int iGroupID, int iItemID, float fProbability );

private:
	CX2OfflineDropTable();
	~CX2OfflineDropTable();

	/// Loads on first use, for the same reason X2OfflineStatTable does: the
	/// offline server is constructed before the .kom archives are mounted.
	void	EnsureLoaded();
	bool	RunScript( const wchar_t* szName );

	int		GetNpcExp( int iNpcLevel ) const;

	/// Shared body of the AddXxxDropInfo_LUA readers: pulls the key field,
	/// MonsterID, Exp, ED and EDProperty off the table on the Lua stack and
	/// files the row in the map it belongs to.
	void	ReadDropBlock( const char* szKeyField, bool bBattleField );

private:
	struct KNpcReward
	{
		int		m_iExpGate;			///< > 0 means this monster grants EXP
		int		m_iEDPerLevel;		///< > 0 means it grants ED, at this rate
		float	m_fEDProperty;		///< percentage chance the ED drops at all

		KNpcReward()
			: m_iExpGate( 0 )
			, m_iEDPerLevel( 0 )
			, m_fEDProperty( 0.0f )
		{
		}
	};

	static CX2OfflineDropTable*	ms_pInstance;

	/// npc level -> exp
	std::map< int, int >								m_mapNpcExp;

	/// (dungeonID, monsterID) -> reward. DungeonID 0 is the wildcard row the
	/// real table uses for monsters that appear in more than one dungeon, and
	/// KDropTable::NormalNpcDropItem falls back to it - so this map is looked up
	/// twice, exactly as the server does.
	std::map< std::pair< int, int >, KNpcReward >		m_mapMonsterDrop;

	/// (battleFieldID, monsterID) -> reward. A separate map, not a separate key
	/// range: see GetNpcReward for the six monsters that made that necessary.
	std::map< std::pair< int, int >, KNpcReward >		m_mapBattleFieldDrop;

	bool	m_bLoadAttempted;
	int		m_iNpcExpRows;
	int		m_iMonsterRows;
};

#endif SERV_IRUHADEV_OFFLINE
