#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-02
// Description: Offline mode - the monsters that stand in a field (phase 4).
//
//              A field ("battlefield") fills itself from a per-field monster
//              table the client never reads. The plan, and my own earlier note
//              in the phase 4 write-up, said this needed data that does not
//              ship with the client and would have to be recreated by hand.
//              That was wrong, and provably so: every one of those tables is
//              already inside data036.kom.
//
//              How that was established, since it decides the whole design:
//              the .kom carries a plaintext XML manifest, and 24 of its
//              entries are per-field scripts (Ruben_Field_00.lua,
//              Elder_Field_00.lua, ...). Decompressing each member and running
//              it through the same XOR the other scripts use turns up
//              AddSpawnMonsterGroup in 24 of them, SPAWN_GROUP_ID in 25 and
//              MONSTER_ID in 24 - while AddLine appears in none, so these are
//              the monster tables and not the line maps. Ruben_Field_00.lua
//              decompiles to exactly the keys CXSLBattleField parses, down to
//              the misspelled AGRESSIVE. Nothing has to be repacked.
//
//              So this class is the server's own two steps, no more:
//
//                BattleFieldData.lua   already parsed by the client, but only
//                                      for the fields the client cares about.
//                                      NPC_TABLE_FILE_NAME is the one field it
//                                      skips, and it is the one that matters -
//                                      it names the per-field monster table.
//
//                <Field>.lua           g_pBattleFieldManager:AddSpawnMonsterGroup{}
//                                      one call per spawn group: which monsters
//                                      may appear, at what weight, how many of
//                                      each, and on which lines they walk.
//
//              GetCreateMonsterList then reproduces
//              CXSLBattleField::GetCreateMonsterList / CreateMonsterFromSpawnNpcGroup
//              / MakeMonsterInfo (XSLBattleField.cpp:542, 470, 618).
//
//              Respawn and the middle boss are here too, driven by the offline
//              server's own tick (the field's position push) rather than by a
//              real timer:
//
//                GetRespawnMonsterList        XSLBattleField.cpp:623
//                GetMiddleBossMonsterInfo     XSLBattleField.cpp:726
//
//              Both need constants and one Lua function that live in
//              BattleFieldServerData.lua - the only file in this subsystem that
//              is NOT in the client's .kom, because it was never meant to reach
//              a client. It has to be XOR-encrypted and packed alongside
//              StatTable.lua. Without it the field still populates and still
//              respawns, on the documented fallbacks below, and the middle boss
//              never appears; the log says so plainly rather than pretending.
//
//              What is still NOT here, and why:
//
//                The full boss (BATTLE_FIELD_RISK_INFO / MG_BOSS_NPC) - not a
//                gap. CXSLBattleField::GetBattieFieldBossMonsterInfo is entirely
//                #else'd out under SERV_BATTLEFIELD_MIDDLE_BOSS, which this
//                build defines, so the shipping game replaced the full boss with
//                the middle boss. The client's BattleFieldData.lua carries no
//                BATTLE_FIELD_RISK_INFO at all, consistent with that.
//
//                Elite / attribute-enchanted monsters - the escalation reserves
//                them, but their stat rolls come from AttribNpcTable.lua, which
//                is also server-only. m_mapAttribNpcInfo goes out empty, which
//                the client reads as "ordinary monster".
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

class CX2OfflineBattleField
{
public:
	static CX2OfflineBattleField*	Instance();
	static void						Release();

	/// The field's opening monster population, in the shape
	/// EGS_BATTLE_FIELD_NPC_LOAD_NOT carries it. Empty means the field has no
	/// table, which is legitimate - the rest areas (VMI_BATTLE_FIELD_*_REST_00)
	/// have none.
	///
	/// iPlayerCount is accepted and unused, exactly as on the server: the count
	/// scaling there is commented out in favour of the field's flat
	/// MAX_NUMBER_OF_MONSTER_IN_THIS_BATTLE_FIELD (XSLBattleField.cpp:556-568).
	/// Kept in the signature because it is the server's signature.
	void	GetCreateMonsterList( int iBattleFieldID, int iPlayerCount,
								  OUT std::vector< KNPCUnitReq >& vecOut );

	/// One monster per entry in mapRespawnReadyByGroup, chosen the same way the
	/// opening fill chooses - so a group that has since hit its cap declines and
	/// the slot is simply not refilled. Mirrors GetRespawnMonsterList.
	void	GetRespawnMonsterList( int iBattleFieldID,
								   const std::map< int, int >& mapAliveByGroup,
								   const std::map< std::pair< int, int >, int >& mapAliveByNpc,
								   const std::map< int, int >& mapRespawnReadyByGroup,
								   OUT std::vector< KNPCUnitReq >& vecOut );

	/// The middle boss for a field at a given danger value: one weighted pick
	/// across BATTLE_FIELD_MIDDLE_BOSS_INFO, then every monster of every spawn
	/// group that entry names. Empty when the field declares no middle boss or
	/// the danger value has not reached the chosen entry's RISK_VALUE.
	void	GetMiddleBossMonsterInfo( int iBattleFieldID, int iDangerValue,
									  OUT std::vector< KNPCUnitReq >& vecOut );

	/// True once BattleFieldServerData.lua has been read. False means every
	/// getter below is returning its fallback.
	bool	HasServerData();

	/// Seconds until a killed monster comes back, rolled per monster out of
	/// SetMonsterRespawnTimeMinMax.
	int		GetRespawnTimeSec();

	/// Danger added per kill is the monster's type factor times this
	/// (KBattleFieldGameManager::IncreaseDangerousValue).
	int		GetDangerEventRate();
	int		GetDangerValueMax();
	int		GetDangerValueWarning();
	int		GetBossCheckUserCount();

	/// GET_MIDDLE_BOSS_MONSTER_DROP_RATE, called in the client's own Lua state.
	/// Returns 0 when the function is not loaded, i.e. no middle boss.
	float	GetMiddleBossDropRate( int iCurDanger, int iOldDanger );

	/// SetBattleFieldEXPFactor / SetBattleFieldEDFactor. A field's rewards are
	/// scaled by these; 1.0 when unknown.
	float	GetEXPFactor();
	float	GetEDFactor();

	//////////////////////////////////////////////////////////////////////////
	// Bound into Lua as g_pBattleFieldManager:*. Public only because lua_tinker
	// needs their addresses.

	/// BattleFieldData.lua. The client binds a method of this name on its own
	/// manager and reads a different subset of the same block; this one exists
	/// to pick up NPC_TABLE_FILE_NAME.
	void	AddBattleFieldData_LUA();

	/// <Field>.lua
	void	AddSpawnMonsterGroup_LUA();

	/// BattleFieldServerData.lua. All eleven have to be bound even though only
	/// six are read - an unbound method aborts the chunk at the first call, and
	/// the file makes all eleven.
	void	SetMonsterRespawnFactorByUserCount_LUA( int iFactor );
	void	SetMonsterRespawnTimeMinMax_LUA( int iMin, int iMax );
	void	SetBattleFieldFactor_LUA( float fFactor );
	void	SetBattleFieldEDFactor_LUA( float fFactor );
	void	SetBattleFieldEXPFactor_LUA( float fFactor );
	void	SetDangerousValueMax_LUA( int iValue );
	void	SetDangerousValueWarning_LUA( int iValue );
	void	SetBossCheckUserCount_LUA( int iCount );
	void	SetEliteMonsterDropValue_LUA( int iValue );
	void	SetDangerousValueEventRate_LUA( int iRate );
	void	AddEliteMonsterDropInfo_LUA( int iMin, int iMax, int iDifficulty, int iValue );

private:
	CX2OfflineBattleField();
	~CX2OfflineBattleField();

	/// Reads BattleFieldData.lua once, for the field -> table-name mapping.
	void	EnsureIndexLoaded();

	/// Reads BattleFieldServerData.lua once, for the escalation constants.
	void	EnsureServerDataLoaded();

	/// Reads one field's monster table, once. Returns false when the field has
	/// no table or it could not be run.
	bool	EnsureFieldLoaded( int iBattleFieldID );

	/// LoadDataFile + DoMemory, with the plaintext fallback every other offline
	/// script loader uses. bAdded reports whether the chunk actually called
	/// anything, which is the only reliable way to tell "ran" from "ran and did
	/// nothing because it was still encrypted".
	bool	RunScript( const wchar_t* szName, int& iRowsBefore, int& iRowsNow );

	/// Swaps the global the shipped scripts call into over to this object, and
	/// puts the client's own manager back afterwards. Both files address
	/// g_pBattleFieldManager, and CX2BattleFieldManager owns that name during
	/// normal play.
	void	BindGlobal();
	void	UnbindGlobal();

private:
	struct KSpawnNpc
	{
		int					m_iNpcID;
		float				m_fRate;				///< weight within the group
		int					m_iMaxNumberOfThisMonster;
		int					m_iAddLevel;			///< added to STANDARD_MONSTER_LEVEL
		bool				m_bDrop;
		bool				m_bAggressive;
		float				m_fAddPosY;

		/// START_POSITION picked uniformly; when empty the server falls back to
		/// a random PETROL_LINE_INDEX instead.
		std::vector< int >	m_vecStartPosition;
		std::vector< int >	m_vecPetrolLineIndex;
		std::vector< int >	m_vecPlayLineIndex;

		KSpawnNpc()
			: m_iNpcID( 0 )
			, m_fRate( 0.0f )
			, m_iMaxNumberOfThisMonster( 0 )
			, m_iAddLevel( 0 )
			, m_bDrop( false )
			, m_bAggressive( true )
			, m_fAddPosY( 0.0f )
		{
		}
	};

	struct KSpawnGroup
	{
		int							m_iGroupID;
		bool						m_bIsBossMonsterGroup;
		int							m_iMinNumberOfMonsterInThisGroup;
		int							m_iMaxNumberOfMonsterInThisGroup;
		std::set< int >				m_setSiegeMonster;
		std::map< int, KSpawnNpc >	m_mapSpawnNpc;		///< npcID -> row

		KSpawnGroup()
			: m_iGroupID( 0 )
			, m_bIsBossMonsterGroup( false )
			, m_iMinNumberOfMonsterInThisGroup( 0 )
			, m_iMaxNumberOfMonsterInThisGroup( 0 )
		{
		}

		bool IsSiegeMonster( int iNpcID ) const
		{
			return m_setSiegeMonster.find( iNpcID ) != m_setSiegeMonster.end();
		}
	};

	/// One BATTLE_FIELD_MIDDLE_BOSS_INFO row. The server calls this SRiskInfo and
	/// shares the type with the (dead) full-boss path, which is why the field
	/// names read oddly for a boss entry.
	struct KMiddleBoss
	{
		int					m_iSpawnID;			///< the lottery case
		int					m_iSpawnRate;		///< its weight
		int					m_iRiskValue;		///< danger needed before it may spawn
		bool				m_bShowBossName;
		bool				m_bBossGaugeHpLines;
		std::vector< int >	m_vecSpawnGroupID;	///< every group it brings, all of it

		KMiddleBoss()
			: m_iSpawnID( 0 )
			, m_iSpawnRate( 0 )
			, m_iRiskValue( 0 )
			, m_bShowBossName( false )
			, m_bBossGaugeHpLines( false )
		{
		}
	};

	struct KField
	{
		std::wstring				m_wstrNpcTableFileName;
		int							m_iStandardMonsterLevel;
		int							m_iMaxNumberMonster;
		bool						m_bTableLoaded;

		/// Non-boss groups in file order - what the opening fill and respawn
		/// round-robin over. Mirrors m_vecRespawnNpcGroupList.
		std::vector< KSpawnGroup >	m_vecGroup;

		/// Every group including the boss ones, by ID. The middle boss names its
		/// groups by ID and those are exactly the ones missing from the vector
		/// above, so both containers are needed - the server keeps the same two
		/// (m_mapNpcGroupList / m_vecRespawnNpcGroupList).
		std::map< int, KSpawnGroup >	m_mapAllGroup;

		std::vector< KMiddleBoss >	m_vecMiddleBoss;

		KField()
			: m_iStandardMonsterLevel( 1 )
			, m_iMaxNumberMonster( 0 )
			, m_bTableLoaded( false )
		{
		}
	};

	/// One monster's worth of the decision, split out so the loop below reads
	/// like the server's. Returns false when the group has nothing left to give.
	bool	PickFromGroup( const KSpawnGroup& kGroup,
						   const std::map< int, int >& mapAliveByGroup,
						   const std::map< std::pair< int, int >, int >& mapAliveByNpc,
						   int iStandardLevel,
						   OUT KNPCUnitReq& kOut ) const;

	/// MakeMiddleBossMonsterInfo (XSLBattleField.cpp:816). Not PickFromGroup:
	/// a middle boss is not a lottery within its group, every monster of every
	/// named group spawns, and the result carries boss flags.
	void	MakeMiddleBoss( const KSpawnGroup& kGroup, const KSpawnNpc& kNpc,
							const KMiddleBoss& kBoss, int iStandardLevel,
							OUT KNPCUnitReq& kOut ) const;

	/// Resolve a start-position index to a world position and facing, the way
	/// GetMonsterStartPosInfo does server-side.
	void	ApplyStartPos( OUT KNPCUnitReq& kOut ) const;

private:
	static CX2OfflineBattleField*	ms_pInstance;

	std::map< int, KField >			m_mapField;		///< battlefield ID -> field

	/// Which field AddSpawnMonsterGroup_LUA is currently filling. The shipped
	/// table does not name its own field, so the loader has to say.
	int								m_iLoadingFieldID;

	bool							m_bIndexLoaded;
	int								m_iGroupRows;
	int								m_iFieldRows;

	//////////////////////////////////////////////////////////////////////////
	// BattleFieldServerData.lua. The fallbacks are that file's own US values,
	// read out of KncWX2Server/ServerResource/US/BattleFieldServerData.lua, so
	// an unpacked install behaves like the live server rather than like nothing -
	// except for the middle boss, which needs the Lua function and so genuinely
	// cannot run without the file.
	bool							m_bServerDataLoaded;
	bool							m_bServerDataAttempted;

	int								m_iRespawnTimeMin;		///< 15
	int								m_iRespawnTimeMax;		///< 25
	int								m_iDangerValueMax;		///< 4800
	int								m_iDangerValueWarning;	///< 5
	int								m_iDangerEventRate;		///< 4
	int								m_iBossCheckUserCount;	///< 1
	int								m_iEliteMonsterDropValue;	///< 15
	float							m_fEXPFactor;			///< 1.5
	float							m_fEDFactor;			///< 0.8
};

#endif SERV_IRUHADEV_OFFLINE
