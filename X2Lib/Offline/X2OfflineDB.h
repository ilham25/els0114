#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-01
// Description: Offline mode - the SQLite persistence layer (phase 2).
//
//              A thin sqlite3 wrapper plus the handful of queries the offline
//              server needs. None of KODBC and none of the 785 stored
//              procedures are ported; the offline server talks to SQLite
//              directly.
//
//              Schema is modelled on the real T-SQL in
//              DataBase/Database2/Database2/Schema Objects/Tables/, reduced to
//              the columns that are actually read back.
//
//              Two conventions, picked once and held:
//
//              * TEXT is bound and read through the UTF-16 API
//                (sqlite3_bind_text16 / sqlite3_column_text16), so a
//                std::wstring round-trips with no conversion of ours. SQLite
//                still stores UTF-8 in the file, so `grep` and any editor can
//                read it.
//              * Dates are INTEGER seconds-since-epoch, not the TEXT the
//                original T-SQL used. The client wants __int64 time_t values
//                (KUnitInfo::m_trDelAbleDate, CTime, GetServerCurrentTime64),
//                so epoch is the native currency here and TEXT would mean
//                parsing on every read. The soft-delete trick from dbo.GUnit
//                survives the change unaltered: del_date == reg_date means
//                "not deleted", and restoring is del_date = reg_date.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

struct sqlite3;
struct sqlite3_stmt;

//////////////////////////////////////////////////////////////////////////
/// One row of `item` (phase 5), which is one KInventoryItemInfo split into
/// the columns that have to be filtered on plus the scalars of KItemInfo.
///
/// The socket vectors are stored as comma-separated TEXT rather than as the
/// `blob_data` KSerBuffer the v1 schema anticipated, for one reason: a save
/// file that can be read with `sqlite3 els_db.sql "select * from item"` is
/// debuggable and a blob is not. Nothing sorts or filters on them, so the cost
/// of parsing on load is a handful of strtol calls per item.
struct KOfflineItemRow
{
	UidType			m_nItemUID;
	UidType			m_nUnitUID;
	int				m_iCategory;		///< CX2Inventory::SORT_TYPE
	int				m_iSlotID;
	int				m_iItemID;
	int				m_iQuantity;
	int				m_iEndurance;
	int				m_iEnchantLevel;
	int				m_iSealData;
	int				m_iUsageType;		///< CX2Item::PERIOD_TYPE, mirrored into KItemInfo::m_cUsageType
	int				m_iItemState;		///< KItemInfo::ITEM_STATE
	int				m_iPeriod;
	int				m_iAttrib0;
	int				m_iAttrib1;
	int				m_iAttrib2;
	std::vector<int>	m_vecSocket;
	std::vector<int>	m_vecRandomSocket;

	KOfflineItemRow()
		: m_nItemUID( 0 )
		, m_nUnitUID( 0 )
		, m_iCategory( 0 )
		, m_iSlotID( 0 )
		, m_iItemID( 0 )
		, m_iQuantity( 1 )
		, m_iEndurance( 0 )
		, m_iEnchantLevel( 0 )
		, m_iSealData( 0 )
		, m_iUsageType( 0 )
		, m_iItemState( 0 )
		, m_iPeriod( 0 )
		, m_iAttrib0( 0 )
		, m_iAttrib1( 0 )
		, m_iAttrib2( 0 )
	{
	}
};

//////////////////////////////////////////////////////////////////////////
/// One row of `unit_skill` (phase 5). `m_iSlot` is the equipped skill slot,
/// 0-3 for slot A and 4-7 for slot B, or -1 for a learned skill that is not
/// equipped - the same numbering KUserSkillTree::m_aiSkillSlot uses.
struct KOfflineSkillRow
{
	int				m_iSkillID;
	int				m_iLevel;
	int				m_iCSPoint;
	int				m_iSlot;

	KOfflineSkillRow()
		: m_iSkillID( 0 )
		, m_iLevel( 0 )
		, m_iCSPoint( 0 )
		, m_iSlot( -1 )
	{
	}
};

//////////////////////////////////////////////////////////////////////////
/// One sub-quest's progress (phase 6). `m_iClearData` is the byte counter the
/// wire calls KSubQuestInstance::m_ucClearData - a kill count, a collect count,
/// or 0/1 for the "did it happen" types - and `m_bSuccess` is the same struct's
/// flag. The pair is stored rather than recomputed because for a hunt quest
/// there is nothing to recompute it from: the monsters are already dead.
///
/// Missions (the title system) use the identical shape, so the same row type
/// serves both tables. KSubMissionInstance's counter is a short rather than a
/// byte; an int holds either.
struct KOfflineSubQuestRow
{
	int				m_iSubID;
	int				m_iClearData;
	bool			m_bSuccess;

	KOfflineSubQuestRow()
		: m_iSubID( 0 )
		, m_iClearData( 0 )
		, m_bSuccess( false )
	{
	}
};

/// One in-progress quest and its sub-quests. The sub-quest *order* is not
/// stored: it is rebuilt from the templet's own m_vecSubQuest on load, because
/// the wire format is positional - KQuestInstance::m_vecSubQuestInstance[i]
/// must line up with QuestTemplet::m_vecSubQuest[i], which is how every server
/// handler indexes it (UserQuestManager.cpp:1446 and everywhere near it).
struct KOfflineQuestRow
{
	int				m_iQuestID;
	std::vector< KOfflineSubQuestRow >	m_vecSub;

	KOfflineQuestRow()
		: m_iQuestID( 0 )
	{
	}
};

/// One finished quest - dbo.GUnitQuestComplete, and KCompleteQuestInfo on the
/// wire. The count matters because a repeatable quest can be finished more than
/// once, and the date because a daily one may not be re-taken before 6am.
struct KOfflineCompleteQuestRow
{
	int				m_iQuestID;
	int				m_iCount;
	__int64			m_tDate;

	KOfflineCompleteQuestRow()
		: m_iQuestID( 0 )
		, m_iCount( 0 )
		, m_tDate( 0 )
	{
	}
};

/// One owned title. m_tEndDate 0 means permanent, which is what KTitleInfo
/// calls m_bInfinity; nothing offline issues a rented title, but the column is
/// there so one can be without a migration.
struct KOfflineTitleRow
{
	int				m_iTitleID;
	__int64			m_tEndDate;

	KOfflineTitleRow()
		: m_iTitleID( 0 )
		, m_tEndDate( 0 )
	{
	}
};

/// One row of `cash_product` - one purchasable line of the cash shop.
///
/// Seeded from X2OfflineCashSeed.h, which is a transcription of dbo.EB_Product
/// out of the ES_BILLING database. Everything about it that was constant across
/// all 2360 source rows lives in code rather than in a column; see that header.
struct KOfflineCashProductRow
{
	int				m_iProductNo;
	int				m_iItemID;
	int				m_iCategoryNo;		///< the billing category, 11..63
	int				m_iQuantity;
	int				m_iPrice;
	bool			m_bEvent;

	KOfflineCashProductRow()
		: m_iProductNo( 0 )
		, m_iItemID( 0 )
		, m_iCategoryNo( 0 )
		, m_iQuantity( 1 )
		, m_iPrice( 0 )
		, m_bEvent( false )
	{
	}
};

/// One line of the cash deposit - a cash-shop product bought and not yet
/// claimed into the bag. The live equivalent is the billing DB's cash
/// inventory, which KBillOrderInfo is the wire shape of; `trans_no` is the row
/// id, which is what m_iTransNo carries and what the claim request names.
struct KOfflineCashOrderRow
{
	__int64			m_nTransNo;
	int				m_iProductNo;
	int				m_iItemID;
	int				m_iQuantity;
	int				m_iPeriod;		///< rental days; 0 is permanent
	int				m_iPrice;
	__int64			m_tBuyDate;

	KOfflineCashOrderRow()
		: m_nTransNo( 0 )
		, m_iProductNo( 0 )
		, m_iItemID( 0 )
		, m_iQuantity( 1 )
		, m_iPeriod( 0 )
		, m_iPrice( 0 )
		, m_tBuyDate( 0 )
	{
	}
};

/// One owned pet - dbo.GPet, and KPetInfo on the wire.
struct KOfflinePetRow
{
	__int64			m_nPetUID;
	int				m_iPetID;
	std::wstring	m_wstrName;
	int				m_iEvolutionStep;
	int				m_iSatiety;
	int				m_iIntimacy;
	int				m_iExtroversion;
	int				m_iEmotion;
	bool			m_bAutoFeed;
	bool			m_bAutoLooting;
	__int64			m_tLastFeedDate;
	__int64			m_tLastSummonDate;
	__int64			m_tRegDate;

	KOfflinePetRow()
		: m_nPetUID( 0 )
		, m_iPetID( 0 )
		, m_iEvolutionStep( 0 )
		, m_iSatiety( 0 )
		, m_iIntimacy( 0 )
		, m_iExtroversion( 0 )
		, m_iEmotion( 0 )
		, m_bAutoFeed( false )
		, m_bAutoLooting( false )
		, m_tLastFeedDate( 0 )
		, m_tLastSummonDate( 0 )
		, m_tRegDate( 0 )
	{
	}
};

/// One owned riding pet - KRidingPetInfo on the wire. `m_tDestroyDate` 0 means
/// permanent, the same convention KOfflineTitleRow uses for a title.
struct KOfflineRidingPetRow
{
	__int64			m_nRidingPetUID;
	int				m_iRidingPetID;
	float			m_fStamina;
	__int64			m_tDestroyDate;
	__int64			m_tLastUnSummonDate;

	KOfflineRidingPetRow()
		: m_nRidingPetUID( 0 )
		, m_iRidingPetID( 0 )
		, m_fStamina( 0.0f )
		, m_tDestroyDate( 0 )
		, m_tLastUnSummonDate( 0 )
	{
	}
};

/// One in-progress title mission. Same shape as KOfflineQuestRow, and for the
/// same positional reason.
struct KOfflineMissionRow
{
	int				m_iMissionID;
	std::vector< KOfflineSubQuestRow >	m_vecSub;

	KOfflineMissionRow()
		: m_iMissionID( 0 )
	{
	}
};

//////////////////////////////////////////////////////////////////////////
/// One cleared dungeon - dbo.GUnitDungeon, and KDungeonClearInfo on the wire.
///
/// `m_iDungeonID` is the dungeon ID with its difficulty already added, which is
/// how the client keys it: CX2StateDungeonGame computes
/// `m_iDungeonID + m_cDifficulty` and calls that the real dungeon ID
/// ([X2StateDungeonGame.cpp:3006](X2Lib/X2StateDungeonGame.cpp#L3006)), and
/// CX2DungeonManager::IsActiveDungeon looks a prerequisite up by the same
/// number. Storing the base ID alone would unlock the wrong thing the moment a
/// dungeon is played on anything but the lowest difficulty.
struct KOfflineDungeonClearRow
{
	int				m_iDungeonID;
	int				m_iClearCount;
	int				m_iBestRank;		///< lowest number seen; 1 is S, 0 is "none"
	int				m_iMaxScore;
	__int64			m_tClearDate;

	KOfflineDungeonClearRow()
		: m_iDungeonID( 0 )
		, m_iClearCount( 0 )
		, m_iBestRank( 0 )
		, m_iMaxScore( 0 )
		, m_tClearDate( 0 )
	{
	}
};

//////////////////////////////////////////////////////////////////////////
/// One row of `unit`, in the shape the packet handlers want it.
struct KOfflineUnitRow
{
	UidType			m_nUnitUID;
	UidType			m_nUserUID;
	int				m_iUnitClass;
	std::wstring	m_wstrNickName;
	int				m_iLevel;
	int				m_iEXP;
	int				m_iED;
	int				m_iSP;
	int				m_iCSP;				///< cash skill points; always 0 offline, see SaveSkillPoint
	int				m_iSpirit;
	int				m_iLastPos;
	int				m_iLastLineIndex;
	int				m_iLastPosValue;
	int				m_iCurHP;			///< 0 means "never stored" - see MakeGamePlayStatus
	int				m_iCurMP;
	int				m_iHyperGage;
	int				m_iAbilCount;		///< WSP / cannonballs / force, per unit type
	int				m_iTitleID;			///< the worn title, KUnitInfo::m_iTitleID; 0 = none
	__int64			m_tRegDate;
	__int64			m_tDelDate;
	__int64			m_tLastDate;

	/// mirrors dbo.GUnit's computed column:
	///   Deleted AS (case RegDate when DelDate then 0 else 1 end)
	bool IsDeleted() const		{ return m_tDelDate != m_tRegDate; }

	KOfflineUnitRow()
		: m_nUnitUID( 0 )
		, m_nUserUID( 0 )
		, m_iUnitClass( 0 )
		, m_iLevel( 1 )
		, m_iEXP( 0 )
		, m_iED( 0 )
		, m_iSP( 0 )
		, m_iCSP( 0 )
		, m_iSpirit( 0 )
		, m_iLastPos( 0 )
		, m_iLastLineIndex( 0 )
		, m_iLastPosValue( 0 )
		, m_iCurHP( 0 )
		, m_iCurMP( 0 )
		, m_iHyperGage( 0 )
		, m_iAbilCount( 0 )
		, m_iTitleID( 0 )
		, m_tRegDate( 0 )
		, m_tDelDate( 0 )
		, m_tLastDate( 0 )
	{
	}
};

//////////////////////////////////////////////////////////////////////////

class CX2OfflineDB
{
public:
	enum
	{
		/// Schema revision. Bump it and add a rung to Migrate() when a later
		/// phase needs a new table, so existing saves are not wiped.
		SCHEMA_VERSION			= 9,

		/// How long after a soft delete the final delete becomes possible.
		/// Zero: a solo save has nobody to protect a character from, so the
		/// final-delete button is live the moment the character is deleted.
		/// (dbo.gup_delete_unit used one minute; the live global service used
		/// days, which is what the STR_ID_16102 confirmation text still says -
		/// that string is baked into the .kom string table and gates nothing.
		/// The only real gate is KUnitInfo::m_trDelAbleDate - see DelAbleDate.)
		DELETE_WAIT_SECONDS		= 0,

		/// How far into the past a zero-wait able-date is placed. See
		/// DelAbleDate for why it cannot simply be "now".
		DELETE_CLOCK_SLACK_SECONDS = 60,

		/// Character slots on a fresh account. dbo.GUser.USSize.
		DEFAULT_UNIT_SLOTS		= 3,

		/// dbo.gup_create_unit inserts LastPosition = 20000 on a new character.
		DEFAULT_LAST_POSITION	= 20000,

		/// The cash wallet. Cosmetic: a purchase does NOT deduct it (see
		/// Handler_EGS_BUY_CASH_ITEM_REQ), so this is the number the shop shows
		/// for the whole life of the save rather than a starting balance.
		///
		/// It is still checked before a purchase, so setting it to 0 in the
		/// save file turns the shop into a look-but-don't-touch window - which
		/// is the only reason the affordability check is still there.
		///
		/// Lives in `settings` under the key `cash_start`, so it can be changed
		/// with a SQL editor and no rebuild.
		DEFAULT_CASH_BALANCE	= 999999,

		/// The price every seeded cash_product row gets. dbo.EB_Product carries
		/// 1 for all 2360 of its rows - a deliberate choice by the owner of this
		/// save, since the wallet is cosmetic and nothing is ever deducted.
		///
		/// Using the real KOG prices instead is not an option: CashItemPrice.lua
		/// covers only 789 of the 2342 items this shop sells, so two thirds of
		/// the catalog would display 0.
		SEED_CASH_PRICE		= 1,

		/// What DEFAULT_CASH_BALANCE used to be. The v8 migration only rewrites
		/// a `cash_start` row still sitting on this value, so a hand-edited one
		/// survives.
		LEGACY_CASH_BALANCE	= 100000,
	};

public:
	static CX2OfflineDB*	Instance();
	static void				Release();

	bool	Open( const wchar_t* szPath );
	void	Close();
	bool	IsOpen() const					{ return NULL != m_pDB; }

	//////////////////////////////////////////////////////////////////////////
	// save integrity (phase 8)
	//
	// One packet is one unit of work. CX2OfflineServer::OnClientSend opens a
	// transaction before dispatch and commits it only if the handler came back
	// without faulting, so a handler that dies half way through a multi-row
	// change leaves the save file as it was rather than as it briefly was.
	//
	// SAVEPOINT rather than BEGIN, because two operations already had a
	// transaction of their own - CreateUnit and FinalDeleteUnit - and SQLite
	// answers a nested BEGIN with "cannot start a transaction within a
	// transaction". Savepoints nest by name and the outermost one behaves
	// exactly like a transaction.
	//
	// Not guarded by m_cs for their whole span: the lock is taken per
	// statement, and what actually makes a transaction safe here is that all
	// DB access happens on the dispatch path, which CX2OfflineServer
	// serializes on m_csDispatch. Nothing on the client's own threads touches
	// this class.

	bool	Begin();
	bool	Commit();
	bool	Rollback();
	int		TransactionDepth() const		{ return m_iTxnDepth; }

	/// PRAGMA wal_checkpoint(TRUNCATE) - folds els_db.sql-wal back into
	/// els_db.sql and empties it. A no-op inside a transaction, so callers run
	/// it from outside one.
	bool	Checkpoint();

	/// Copy the whole save to szPath through sqlite3_backup_*, replacing what
	/// is there. The online-backup API and not CopyFile: the connection is
	/// never closed while the game runs, so a byte copy of els_db.sql alone
	/// misses everything still sitting in the -wal file.
	bool	Backup( const wchar_t* szPath );

	//////////////////////////////////////////////////////////////////////////
	// account
	bool	GetOrCreateAccount( const std::wstring& wstrLoginID, OUT UidType& nUserUID, OUT int& iUnitSlots );

	//////////////////////////////////////////////////////////////////////////
	// units
	bool	LoadUnits( UidType nUserUID, OUT std::vector< KOfflineUnitRow >& vecOut );
	bool	LoadUnit( UidType nUnitUID, OUT KOfflineUnitRow& kOut );
	bool	IsNickNameTaken( const std::wstring& wstrNickName );
	int		CountLiveUnits( UidType nUserUID );

	/// Inserts with del_date == reg_date, seeds the inventory sizes, and hands
	/// back the row it just wrote.
	bool	CreateUnit( UidType nUserUID, int iUnitClass, const std::wstring& wstrNickName,
						OUT KOfflineUnitRow& kOut );

	bool	SoftDeleteUnit( UidType nUnitUID, __int64 tDelDate );	///< del_date = tDelDate
	bool	RestoreUnit( UidType nUnitUID );						///< del_date = reg_date
	bool	FinalDeleteUnit( UidType nUnitUID );					///< the only real DELETE FROM
	bool	TouchLastDate( UidType nUnitUID );

	/// Which village the character is standing in - dbo.GUnit.LastPosition.
	/// Written when the client asks to enter a field, so the next login starts
	/// where this one left off.
	bool	SaveLastPosition( UidType nUnitUID, int iMapID );

	/// Where on that village's line map. REMEMBER_LOGOUT_POSITION_TEST is on
	/// (KTDXLIB/Always.h:1943), so the client sends these in
	/// EGS_FIELD_LOADING_COMPLETE_REQ and reads them back out of
	/// KUnitInfo::m_kLastPos on the next login.
	bool	SaveLastFieldPos( UidType nUnitUID, int iLineIndex, int iPosValue );

	/// The live gauges, as the client reports them in EGS_UPDATE_PLAY_STATUS_NOT
	/// roughly every three seconds. This is the write half of what
	/// CX2OfflineServer::MakeGamePlayStatus reads back on the next login; the
	/// real server keeps the same values in its GUnitPlayInfo table.
	bool	SaveGamePlayStatus( UidType nUnitUID, int iCurHP, int iCurMP,
								int iHyperGage, int iAbilCount );

	/// Level, EXP and ED after a dungeon run (phase 4). One statement rather
	/// than three, because the three move together and a half-written level is
	/// the one state the client cannot recover from - its EXP bar would sit past
	/// the end of its own level.
	bool	SaveProgress( UidType nUnitUID, int iLevel, int iEXP, int iED );

	/// Bump dbo.GUnitDungeon's clear count for a dungeon, remembering the best
	/// rank seen. Quest progression (phase 6) reads this back; the client's own
	/// dungeon UI draws its "cleared" marks from it too.
	bool	AddDungeonClear( UidType nUnitUID, int iDungeonID, int iRank );

	/// Every dungeon this character has cleared.
	///
	/// Not a convenience: without it `KUnitInfo::m_mapDungeonClear` goes out
	/// empty, `CX2Unit::IsClearDungeon` answers false for everything, and
	/// `CX2DungeonManager::IsActiveDungeon` locks every dungeon that names a
	/// prerequisite - permanently, across every login.
	bool	LoadDungeonClears( UidType nUnitUID,
							   OUT std::vector< KOfflineDungeonClearRow >& vecOut );

	/// Skill points. Kept separate from SaveProgress because they move on their
	/// own - a skill learned or reset changes SP without touching level or EXP.
	bool	SaveSkillPoint( UidType nUnitUID, int iSP, int iCSP );

	//////////////////////////////////////////////////////////////////////////
	// inventory
	bool	LoadInventorySizes( UidType nUnitUID, OUT std::map< int, int >& mapOut );
	bool	SeedInventorySizes( UidType nUnitUID );

#ifdef SERV_IRUHADEV_OFFLINE_INVENTORY_EXPAND
	/// Bump one category's persisted size by up to iRequestedIncrement, capped
	/// so the result never exceeds iMaxSize. iGranted comes back as the amount
	/// actually applied (0 if the category was already at the cap); the
	/// return value is only about whether the write itself succeeded.
	bool	ExpandInventorySize( UidType nUnitUID, int iCategory, int iRequestedIncrement,
								 int iMaxSize, OUT int& iGranted );
#endif SERV_IRUHADEV_OFFLINE_INVENTORY_EXPAND

	//////////////////////////////////////////////////////////////////////////
	// items (phase 5)

	bool	LoadItems( UidType nUnitUID, OUT std::vector< KOfflineItemRow >& vecOut );

	/// Inserts and fills in kInOut.m_nItemUID with the row id SQLite assigned -
	/// which becomes the item's UID on the wire. Item UIDs therefore survive a
	/// relog, which the client relies on: it keys its own inventory by them and
	/// EGS_DELETE_ITEM_REQ / EGS_GET_ITEM_REQ address items that way.
	bool	InsertItem( IN OUT KOfflineItemRow& kInOut );

	bool	MoveItemRow( UidType nItemUID, int iCategory, int iSlotID );
	bool	SetItemQuantity( UidType nItemUID, int iQuantity );

	/// Everything about an item except where it sits: endurance, enchant, seal,
	/// sockets. Used by a pickup that stacks onto an existing row and by any
	/// later phase that changes an item in place.
	bool	UpdateItem( const KOfflineItemRow& kRow );

	bool	DeleteItemRow( UidType nItemUID );

	//////////////////////////////////////////////////////////////////////////
	// skills (phase 5)

	bool	LoadSkills( UidType nUnitUID, OUT std::vector< KOfflineSkillRow >& vecOut );

	/// Upsert. Level 0 rows are kept rather than deleted: a reset skill that is
	/// a class default or has a learned follower goes back to level 1 and one
	/// that does not goes to level 0, and the client draws that difference
	/// (CX2SkillTree::Handler_EGS_RESET_SKILL_ACK), so the row has to survive.
	bool	SaveSkill( UidType nUnitUID, const KOfflineSkillRow& kRow );

	/// Every skill back to "not learned" and every slot back to empty, for
	/// EGS_INIT_SKILL_TREE_REQ. The rows go away entirely here - the whole tree
	/// is being rebuilt from the class defaults.
	bool	ClearSkills( UidType nUnitUID );

	/// Writes just the slot column, for EGS_CHANGE_SKILL_SLOT_REQ. Takes the
	/// slot rather than the skill so an emptied slot can be cleared without
	/// knowing what used to be in it.
	bool	SetSkillSlot( UidType nUnitUID, int iSlot, int iSkillID );

	//////////////////////////////////////////////////////////////////////////
	// quests (phase 6)

	bool	LoadQuests( UidType nUnitUID, OUT std::vector< KOfflineQuestRow >& vecOut );

	/// Upsert of one quest and every sub-quest it carries. Whole-quest rather
	/// than per-sub-quest because an event can advance two sub-quests of the
	/// same quest at once and the pair has to land together - a counter written
	/// without its success flag is a quest that can never be handed in.
	bool	SaveQuest( UidType nUnitUID, const KOfflineQuestRow& kRow );

	/// Give up, or finish. Removes the header and every sub-quest row.
	bool	DeleteQuest( UidType nUnitUID, int iQuestID );

	bool	LoadCompleteQuests( UidType nUnitUID, OUT std::vector< KOfflineCompleteQuestRow >& vecOut );
	bool	SaveCompleteQuest( UidType nUnitUID, const KOfflineCompleteQuestRow& kRow );

	/// Job advancement. The only thing that ever changes a character's class
	/// after creation is a QT_CHANGE_JOB quest's reward.
	bool	SaveUnitClass( UidType nUnitUID, int iUnitClass );

	//////////////////////////////////////////////////////////////////////////
	// titles and their missions (phase 6)

	bool	LoadMissions( UidType nUnitUID, OUT std::vector< KOfflineMissionRow >& vecOut );
	bool	SaveMission( UidType nUnitUID, const KOfflineMissionRow& kRow );
	bool	DeleteMission( UidType nUnitUID, int iMissionID );

	bool	LoadTitles( UidType nUnitUID, OUT std::vector< KOfflineTitleRow >& vecOut );
	bool	SaveTitle( UidType nUnitUID, const KOfflineTitleRow& kRow );
	bool	SaveEquippedTitle( UidType nUnitUID, int iTitleID );

	//////////////////////////////////////////////////////////////////////////
	// the cash wallet, the cash deposit and the wish list (phase 7)

	/// The wallet the shop displays, out of `settings.cash_start`.
	///
	/// NOT account.cash_balance. That column has been in the schema since v1
	/// and briefly held the wallet in phase 7; it is left alone now because the
	/// wallet is cosmetic and never spent, and a per-account copy of a constant
	/// is just a second value to disagree with the first. Reading the setting
	/// directly also means changing the knob takes effect on the next login
	/// with no migration.
	int		GetWallet();

	bool	GetCashBalance( UidType nUserUID, OUT int& iOut );
	bool	SetCashBalance( UidType nUserUID, int iBalance );

	/// The whole cash-shop catalog, in product-number order.
	///
	/// Keyed by product number, not item ID: eighteen items are sold as two
	/// products each in the source table, and the client expects exactly that -
	/// CX2ItemManager::AddCashItem keeps a vector of KBillProductInfo per item.
	bool	LoadCashProducts( OUT std::vector< KOfflineCashProductRow >& vecOut );

	/// Everything bought and not yet claimed, oldest first.
	bool	LoadCashOrders( UidType nUserUID, OUT std::vector< KOfflineCashOrderRow >& vecOut );

	/// Inserts and fills in kInOut.m_nTransNo with the row id, which becomes
	/// KBillOrderInfo::m_iTransNo on the wire and is what the claim request
	/// names back. Deposit lines therefore survive a relog, the same way item
	/// UIDs do.
	bool	InsertCashOrder( UidType nUserUID, IN OUT KOfflineCashOrderRow& kInOut );

	bool	DeleteCashOrder( UidType nUserUID, __int64 nTransNo );

	/// The wish list is per account, not per character, which is how the client
	/// treats it: CX2CashShop asks for it once when the shop opens and never
	/// mentions a unit.
	bool	LoadWishList( UidType nUserUID, OUT std::vector< int >& vecOut );
	bool	SetWishListItem( UidType nUserUID, int iItemID, bool bAdd );

	/// A free-form key/value table, so a knob can be changed by editing the save
	/// file rather than by rebuilding. Reads return false when the key is absent
	/// rather than inventing a value.
	bool	GetSetting( const char* szKey, OUT int& iOut );
	bool	SetSetting( const char* szKey, int iValue );

	//////////////////////////////////////////////////////////////////////////
	// pets and riding pets (phase 7)

	bool	LoadPets( UidType nUnitUID, OUT std::vector< KOfflinePetRow >& vecOut );
	bool	InsertPet( UidType nUnitUID, IN OUT KOfflinePetRow& kInOut );
	bool	SavePet( UidType nUnitUID, const KOfflinePetRow& kRow );

	bool	LoadRidingPets( UidType nUnitUID, OUT std::vector< KOfflineRidingPetRow >& vecOut );
	bool	InsertRidingPet( UidType nUnitUID, IN OUT KOfflineRidingPetRow& kInOut );
	bool	SaveRidingPet( UidType nUnitUID, const KOfflineRidingPetRow& kRow );
	bool	DeleteRidingPet( UidType nUnitUID, __int64 nRidingPetUID );

	/// Base slot count per CX2Inventory::SORT_TYPE category. Mirrors
	/// KInventory::GetBaseSlotSize (KncWX2Server/GameServer/Inventory.cpp),
	/// including the SERV_IRUHADEV_QUICK_SLOT_FULL_FREE override.
	static int	BaseSlotSize( int iCategory );

	/// "YYYY-MM-DD HH:MM:SS" for an epoch value, for the wstring date fields
	/// the client only ever displays.
	static std::wstring	FormatDate( __int64 tEpoch );

	/// When the final delete becomes possible for a unit soft-deleted at
	/// tDelDate - i.e. what goes in KUnitInfo::m_trDelAbleDate.
	///
	/// This cannot just be tDelDate + DELETE_WAIT_SECONDS. CreateUnitButton()
	/// enables the final-delete button on a *strict*
	///     m_trDelAbleDate < g_pData->GetServerCurrentTime64()
	/// and evaluates it exactly once, when the slots are rebuilt. With a zero
	/// wait that comparison is "now < now" at the one moment it is made, so the
	/// button comes up greyed and stays greyed until something else rebuilds
	/// the buttons - which is why it only lit up after a trip out to channel
	/// selection and back. GetServerCurrentTime64() makes it worse: it is the
	/// login-time snapshot plus whole seconds of GetTickCount, so it can sit a
	/// second or two behind real time.
	///
	/// So a zero wait puts the date safely in the past instead of at "now".
	static __int64	DelAbleDate( __int64 tDelDate );

private:
	CX2OfflineDB();
	~CX2OfflineDB();

	bool	Exec( const char* szSQL );
	bool	Migrate();
	int		ReadSchemaVersion();
	void	LogError( const wchar_t* szWhat );

	/// prepare + bind helpers; every one of them logs and returns NULL/false
	sqlite3_stmt*	Prepare( const char* szSQL );
	static void		BindText( sqlite3_stmt* pStmt, int iIndex, const std::wstring& wstr );
	static std::wstring	ColumnText( sqlite3_stmt* pStmt, int iCol );
	static void		ReadUnitRow( sqlite3_stmt* pStmt, OUT KOfflineUnitRow& kOut );
	static void		ReadItemRow( sqlite3_stmt* pStmt, OUT KOfflineItemRow& kOut );

	/// The socket vectors, as a comma-separated list. Empty in, empty out.
	static std::wstring	FormatIntList( const std::vector<int>& vec );
	static void			ParseIntList( const std::wstring& wstr, OUT std::vector<int>& vecOut );

	/// the column list every unit SELECT uses, in ReadUnitRow's order
	static const char*	UNIT_COLUMNS;

	/// likewise for `item`, in ReadItemRow's order
	static const char*	ITEM_COLUMNS;

private:
	static CX2OfflineDB*	ms_pInstance;

	sqlite3*				m_pDB;
	KncCriticalSection		m_cs;

	/// how many savepoints are open; 0 means "not in a transaction"
	int						m_iTxnDepth;
};

//////////////////////////////////////////////////////////////////////////
/// Scoped transaction, for the one caller that needs it (OnClientSend).
///
/// Commits only when Commit() is called explicitly. Every other way out -
/// an early return, a handler that declined the packet, a fault caught by
/// the __except in DispatchProtected - runs the destructor, which rolls back.
class KOfflineDBTxn
{
public:
	KOfflineDBTxn()
		: m_bOpen( false )
	{
		m_bOpen = CX2OfflineDB::Instance()->Begin();
	}

	~KOfflineDBTxn()
	{
		if( true == m_bOpen )
			CX2OfflineDB::Instance()->Rollback();
	}

	bool	IsOpen() const	{ return m_bOpen; }

	void	Commit()
	{
		if( true == m_bOpen )
		{
			CX2OfflineDB::Instance()->Commit();
			m_bOpen = false;
		}
	}

private:
	// no copies: two guards over one savepoint would release it twice
	KOfflineDBTxn( const KOfflineDBTxn& );
	KOfflineDBTxn& operator = ( const KOfflineDBTxn& );

	bool	m_bOpen;
};

#endif SERV_IRUHADEV_OFFLINE
