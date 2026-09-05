#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-05
// Description: Offline mode - phase 23 (OFFLINE_MODE_PHASE9_PLAN.md), real
//              equipment enhancement instead of a permanent refusal.
//
//              The success-rate tables live only on the server:
//              KncWX2Server/ServerResource/US/EnchantTable.lua, loaded through
//              CXSLEnchantItemManager (KncWX2Server/Common/X2Data/
//              XSLEnchantItemManager.{h,cpp}). This class is that manager's
//              client-side twin - same maps, same lottery math, loaded the
//              same way X2OfflineResolveTable/X2OfflineDropTable load theirs:
//              XOR-decrypted out of data036.kom via the client's own
//              mass-file loader and Lua state.
//
//              **The refusal this replaces was half right, and the half that
//              was wrong is the interesting half.** It said "the enchant
//              tables are server data with no client copy". The *rates* have
//              a client copy and always did - ENCHANT_STAT_SCALE
//              ([X2Define.h:212](X2Lib/X2Define.h#L212)) is
//              SetEnchantRate's twenty-one values, byte for byte, which is
//              why a +7 weapon dropped by a monster already showed the right
//              stat. So do the stone IDs: NORMAL_*_ENCHANT_STONE_ITEM_ID and
//              NEW_*_ENCHANT_STONE_ITEM_ID[] are AddEnchantStoneInfo's, and
//              CX2UIShop::GetEnchantStoneLevel is its level banding. What has
//              no client copy is exactly one thing: SetEnchantProbability -
//              the up/nochange/down/reset/break split per level. That is what
//              this file is for, and it is why nothing here is guessable: the
//              other two thirds can be checked against the client and agree.
//
//              Everything in the packed file is parsed, not just the parts
//              this phase reads, for two reasons. A method the file calls and
//              nothing binds raises a Lua error that aborts the *rest* of the
//              chunk, so an unbound AddAttachItemInfo would silently cost the
//              probability rows that come after it. And the rows that phase 23
//              does not use are the rows phases 25-27 will: AddAttachItemInfo
//              is the "magic amulet" (enchant-attach ticket) table ISSUES.md
//              #5 needs, AddRestoreItemInfo is the repair-scroll table, and
//              Add{Drop,Cube}RandomEnchantInfo is what puts a random enchant
//              level on a dropped item. They are stored and counted so the
//              load line proves they arrived; no accessor is exposed for them
//              until the phase that reads them exists.
//
//              The rows for flags this build does not compile - EnchantPlus
//              and DestroyGuard, both TW/HK-only (ServerDefine_TWHK.h:80-85) -
//              are parsed and ignored the same way. They sit inside a Lua
//              block comment in the shipped file anyway.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

class CX2OfflineEnchantTable
{
public:
	enum
	{
		/// CXSLEnchantItemManager::MAX_ENCHANT_LEVEL.
		MAX_ENCHANT_LEVEL	= 20,
	};

public:
	static CX2OfflineEnchantTable*	Instance();
	static void						Release();

	/// True once EnchantTable.lua has been found, run, and produced rows.
	/// Loads on first call; every accessor below does the same, so a caller
	/// can ask this one question and then trust the rest.
	bool	IsLoaded();

	//////////////////////////////////////////////////////////////////////////
	// Enchant stones. CXSLEnchantItemManager's four getters plus the two
	// level-banded lookups; iItemType is CX2Item::ITEM_TYPE cast to int.

	int		GetWeaponEnchantStoneID();
	int		GetRareWeaponEnchantStoneID();
	int		GetArmorEnchantStoneID();
	int		GetRareArmorEnchantStoneID();

	/// AddEnchantStoneInfo's map: the stone for this equip level and item
	/// type, or 0. The client's own NEW_*_ENCHANT_STONE_ITEM_ID[] arrays are
	/// the same data indexed by CX2UIShop::GetEnchantStoneLevel's banding.
	int		GetEnchantStoneItemID( int iEquipLv, int iItemType );

	/// SERV_EVENT_ENCHANT_STONE_MULTI's multimap, in insertion order. The
	/// server tries each of these before falling back to the line above.
	void	GetEventEnchantStoneItemID( int iEquipLv, int iItemType,
										OUT std::vector< int >& vecOut );

	//////////////////////////////////////////////////////////////////////////
	// Fluor stone ("support material") - the optional item that turns a
	// down-one or a reset into a no-change.

	int		GetEnchantSupportMaterialItemID( int iEquipLv );

	/// SERV_EVENT_SUPPORT_MATERIAL_MULTI's multimap, in insertion order.
	void	GetEventEnchantSupportMaterialItemID( int iEquipLv,
												  OUT std::vector< int >& vecOut );

	/// SetSupportMaterialLimit - the enchant level at which a fluor stone
	/// stops being usable (10 in the packed file). 0 when the file never set
	/// it, which the caller must read as "no limit known", not as "limit 0".
	int		GetSupportMaterialEnchantLimitLevel();

	/// SetEventSupportMaterialLimit - the same during an enchant event (11).
	/// Nothing offline runs an enchant event, so this is loaded for
	/// completeness and for the log line; see the handler.
	int		GetEventSupportMaterialEnchantLimitLevel();

	//////////////////////////////////////////////////////////////////////////

	/// CXSLEnchantItemManager::GetEnchantResult. iEnchantLevel is the level
	/// being attempted - the CURRENT level plus one, which is how
	/// KInventory::EnchantItem calls it (Inventory.cpp:12654) and how
	/// SetEnchantProbability's rows are keyed. Fills iResult with one of
	/// NetError::ERR_ENCHANT_RESULT_00..04 and returns true, or returns false
	/// when there is no row for the level.
	bool	GetEnchantResult( bool bIsRare, int iEnchantLevel, OUT int& iResult );

	/// SetEnchantRate - the stat multiplier for an enchant level. The client
	/// applies its own ENCHANT_STAT_SCALE and never asks for this; it is here
	/// so the loader can cross-check the two and say so in the log, which is
	/// the cheapest possible proof that the packed file is the right file.
	bool	GetEnchantRate( int iEnchantLevel, OUT float& fPercent );

	//////////////////////////////////////////////////////////////////////////
	// AddAttachItemInfo - the "enchant attach" tickets, ISSUES.md #5's magic
	// amulet (phase 25). Parsed since phase 23; these two are the accessors.

	/// CXSLEnchantItemManager::GetAttachItemEnchantLevel. False when the item
	/// is not an attach ticket at all, which is the ERR_ATTACH_ITEM_01 case.
	/// iItemType is CX2Item::ITEM_TYPE: IT_NONE means "any enchantable item",
	/// IT_DEFENCE means armour only.
	bool	GetAttachItemEnchantLevel( int iAttachItemID, OUT int& iEnchantLevel,
									   OUT int& iItemType );

	/// CXSLEnchantItemManager::CheckAttachItemEnableLevel - the
	/// SERV_ADD_ITEM_LEVEL_CONDITION_FOR_ENCHANT_ATTACH_ITEM gate, on in this
	/// build. A ticket with both bounds 0 has no restriction; otherwise the
	/// target's use level must fall inside them. False for an unknown ticket.
	bool	CheckAttachItemEnableLevel( int iAttachItemID, int iUseLevel );

	//////////////////////////////////////////////////////////////////////////
	// Bound into Lua as EnchantItemManager:* - public for lua_tinker.
	//
	// Every method EnchantTable.lua names is bound, including the ones inside
	// its Lua block comments and the ones whose SERV_ flag is off in this
	// build: an unbound method is a runtime error that would abandon the rest
	// of the file. See the header comment.

	void	SetWeaponEnchantStone_LUA( int iItemID );
	void	SetRareWeaponEnchantStone_LUA( int iItemID );
	void	SetArmorEnchantStone_LUA( int iItemID );
	void	SetRareArmorEnchantStone_LUA( int iItemID );

	void	SetSupportMaterialLimit_LUA( int iEnchantLimitLevel );
	void	SetEventSupportMaterialLimit_LUA( int iEnchantLimitLevel );
	void	SetEnchantLimit_LUA( int iEnchantLimitLevel );

	void	AddEnchantStoneInfo_LUA( int iItemID, int iItemType, int iEquipLvMin, int iEquipLvMax );
	void	AddEventEnchantStoneInfo_LUA( int iItemID, int iItemType, int iEquipLvMin, int iEquipLvMax );
	void	AddSupportMaterialInfo_LUA( int iItemID, int iEquipLvMin, int iEquipLvMax );
	void	AddEventSupportMaterialInfo_LUA( int iItemID, int iEquipLvMin, int iEquipLvMax );
	void	AddRestoreItemInfo_LUA( int iItemID, int iItemType, int iEquipLvMin, int iEquipLvMax );
	void	AddEventRestoreItemInfo_LUA( int iItemID, int iItemType, int iEquipLvMin, int iEquipLvMax );
	void	AddEnchantPlusInfo_LUA( int iItemID, int iEquipLvMin, int iEquipLvMax );
	void	AddDestroyGuardInfo_LUA( int iItemID, int iEquipLvMin, int iEquipLvMax );

	/// Takes the row as a second, table argument still on the Lua stack -
	/// same shape as CX2OfflineResolveTable::AddResolveDataBrokenPiece_LUA.
	/// SERV_ADD_ITEM_LEVEL_CONDITION_FOR_ENCHANT_ATTACH_ITEM is defined in
	/// this build, so this is the one-argument form; the packed file passes
	/// iItemType / iEnchantLevel / iMinItemLevel / iMaxItemLevel in the table.
	void	AddAttachItemInfo_LUA( int iItemID );

	void	SetEnchantRate_LUA( int iEnchantLevel, float fPercent );

	/// The four probability setters, all with the row in a table argument.
	void	SetEnchantProbability_LUA( int iEnchantLevel );
	void	SetEnchantProbability_Event_LUA( int iEnchantLevel );
	void	SetEnchantRareProbability_LUA( int iEnchantLevel );
	void	SetEnchantPlusProbability_LUA( int iEnchantLevel );

	void	AddDropRandomEnchantInfo_LUA( int iEnchantLevel, float fRate );
	void	AddCubeRandomEnchantInfo_LUA( int iEnchantLevel, float fRate );

	/// EnchantTable.lua never calls this, but CXSLEnchantItemManager binds it
	/// and a later revision of the file might; binding a no-op costs nothing
	/// and an unbound name costs the rest of the chunk.
	void	Dump_LUA();

private:
	/// One registered lottery: (case value, weight in percent), in the order
	/// KLottery's std::map would iterate them. The enchant result cases are
	/// NetError::ERR_ENCHANT_RESULT_00..04, which are consecutive and
	/// ascending, so insertion order and key order are the same and this can
	/// be a vector.
	typedef std::vector< std::pair< int, float > >	KProbRow;

	/// AddAttachItemInfo's row - CXSLEnchantItemManager::KAttachInfo. Parsed
	/// since phase 23; read by phase 25 through the two accessors above.
	struct KAttachInfo
	{
		int	m_iItemType;
		int	m_iEnchantLevel;
		int	m_iMinItemLevel;
		int	m_iMaxItemLevel;

		KAttachInfo()
			: m_iItemType( 0 ), m_iEnchantLevel( 0 )
			, m_iMinItemLevel( 0 ), m_iMaxItemLevel( 0 )
		{
		}
	};

	/// (equip level, item type), the key shape of
	/// CXSLEnchantItemManager::KEStoneInfoKey.
	typedef std::pair< int, int >	KStoneKey;

	/// (item type, equip level) - CXSLEnchantItemManager::KRestoreInfoKey is
	/// the other way round from KEStoneInfoKey. Transcribed, not tidied.
	typedef std::pair< int, int >	KRestoreKey;

private:
	CX2OfflineEnchantTable();
	~CX2OfflineEnchantTable();

	/// Loads on first use, for the same reason every other offline Lua table
	/// does: the offline server is constructed before the .kom archives are
	/// mounted.
	void	EnsureLoaded();

	/// Reads the five weights out of the table argument on top of the Lua
	/// stack, in SetEnchantProbability_LUA's own order, and returns them as a
	/// KProbRow. Shared by all four probability setters, which differ only in
	/// which map they store the result in.
	static void	ReadProbabilityRow( OUT KProbRow& kOut );

	static void	StoreProbabilityRow( int iEnchantLevel, const KProbRow& kRow,
									 OUT std::map< int, KProbRow >& mapOut );

	/// KLottery::Decision for several registered cases: one roll, cases
	/// accumulated in registration order, the first whose running total
	/// reaches the roll wins. Returns KLottery::CASE_BLANK (-1) if the roll
	/// falls past every case, which only happens when the weights do not sum
	/// to 100. Identical to CX2OfflineResolveTable::LotteryDecideMulti.
	static int	LotteryDecideMulti( const KProbRow& kRow );

	/// Log a line naming every enchant level where the packed SetEnchantRate
	/// value and the client's own ENCHANT_STAT_SCALE disagree. They should
	/// never disagree - see the header comment - so a line here means the
	/// packed file is from a different region or a different build.
	void	CrossCheckRatesAgainstClient() const;

private:
	static CX2OfflineEnchantTable*	ms_pInstance;

	bool	m_bLoadAttempted;
	bool	m_bLoaded;

	int		m_iWeaponEnchantStone;
	int		m_iRareWeaponEnchantStone;
	int		m_iArmorEnchantStone;
	int		m_iRareArmorEnchantStone;

	int		m_iSupportMaterialLimit;
	int		m_iEventSupportMaterialLimit;
	int		m_iEnchantLimit;

	std::map< KStoneKey, int >			m_mapEnchantStoneInfo;
	std::multimap< KStoneKey, int >		m_mmapEventEnchantStoneInfo;
	std::map< int, int >				m_mapSupportMaterialInfo;
	std::multimap< int, int >			m_mmapEventSupportMaterialInfo;

	std::map< int, KProbRow >			m_mapEnchantProb;
	std::map< int, KProbRow >			m_mapEnchantRareProb;
	std::map< int, KProbRow >			m_mapEnchantEventProb;
	std::map< int, KProbRow >			m_mapEnchantPlusProb;

	std::map< int, float >				m_mapEnchantRate;

	//////////////////////////////////////////////////////////////////////////
	// Parsed, stored, and deliberately not exposed - see the header comment.
	// Phase 25 reads m_mapEnchantAttach; the restore maps are the repair
	// scrolls; the two random-enchant lotteries belong to phase 27.

	std::map< KRestoreKey, int >		m_mapRestoreItemInfo;
	std::multimap< KRestoreKey, int >	m_mmapEventRestoreItemInfo;
	std::map< int, KAttachInfo >		m_mapEnchantAttach;
	std::map< int, int >				m_mapEnchantPlusInfo;
	std::map< int, int >				m_mapDestroyGuardInfo;
	KProbRow							m_vecDropRandomEnchant;
	KProbRow							m_vecCubeRandomEnchant;

	//////////////////////////////////////////////////////////////////////////
	// Row counts, for the one load line. m_iProbRows is the one that decides
	// whether the load counts as successful: it is the only thing in the file
	// with no client copy, so a file that produced everything else and none of
	// these is not the file this phase needs.

	int		m_iProbRows;
	int		m_iStoneRows;
	int		m_iSupportRows;
	int		m_iRateRows;
	int		m_iOtherRows;
};

#endif SERV_IRUHADEV_OFFLINE
