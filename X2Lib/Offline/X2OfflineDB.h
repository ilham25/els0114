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
		SCHEMA_VERSION			= 4,

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
	};

public:
	static CX2OfflineDB*	Instance();
	static void				Release();

	bool	Open( const wchar_t* szPath );
	void	Close();
	bool	IsOpen() const					{ return NULL != m_pDB; }

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

	/// Skill points. Kept separate from SaveProgress because they move on their
	/// own - a skill learned or reset changes SP without touching level or EXP.
	bool	SaveSkillPoint( UidType nUnitUID, int iSP, int iCSP );

	//////////////////////////////////////////////////////////////////////////
	// inventory
	bool	LoadInventorySizes( UidType nUnitUID, OUT std::map< int, int >& mapOut );
	bool	SeedInventorySizes( UidType nUnitUID );

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
};

#endif SERV_IRUHADEV_OFFLINE
