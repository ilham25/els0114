#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-04
// Description: Offline mode - what comes out of a cube (phase 7b).
//
//              Phase 7 refused EGS_OPEN_RANDOM_ITEM_REQ outright, on the
//              grounds that the cube contents table is server data with no
//              client copy and rolling an invented one would be exactly the
//              band-aid this project's rules name. That was the right call
//              then and the wrong conclusion: the table is not missing, it is
//              just not packed. It is two of the studio's own files, and this
//              class reads them the same way X2OfflineDropTable reads
//              DropTable.lua.
//
//                RandomItemData.lua    1825 AddAttractionItemInfo, 106
//                  (220 KB)            AddNotifyResultItemInfo, 67
//                                      AddSealRandomItemInfo, 76 AddCharmItem.
//                                      Presentation, not contents: which
//                                      results are announced, which are handed
//                                      over sealed, and which cubes pay a bonus
//                                      charm alongside the draw.
//
//                RandomItemTable.lua   4360 AddRandomItemTemplet blocks (one
//                  (8.8 MB)            per cube) and 90770 AddRandomItemGroup
//                                      rows (the odds). THIS is the contents
//                                      table. It is eleven times the size of
//                                      DropTable.lua, which is why the load is
//                                      lazy: nothing touches it until the
//                                      player opens their first cube.
//
//              Both belong XOR-encrypted in data036.kom alongside every other
//              client script. A loose copy resolves through the same call and
//              should be deleted once packed - it hides a failed repack.
//
//              The draw is CXSLRandomItemManager::GetResultItem's, verbatim
//              (KncWX2Server/Common/X2Data/XSLRandomItemManager.cpp:756):
//
//                * pick the item group by m_UseCondition - UC_ANYONE takes
//                  entry [0]; UC_ONE_UNIT collapses the character's class to
//                  its base class first; UC_ONE_CLASS matches the class exactly
//                * m_bGiveAll false: one KLottery draw over the group
//                * m_bGiveAll true : every row in the group
//                * plus the charm bonus item, if the cube has one
//
//              What is deliberately NOT carried over, and why:
//
//                * SERV_CUBE_IN_ITEM_MAPPING's timed group swap. The mechanism
//                  is on in this build but RandomItemMapping.lua is 100%
//                  comments in the US branch, so the live US server swaps
//                  nothing either. The two Lua entry points are still bound, as
//                  no-ops that log - an unbound call would abort the file.
//                * SERV_ENCHANTED_ITEM_CUBE_REWARD's enchant level on weapons
//                  and armour. SERV_NEW_ITEM_SYSTEM_2013_05 is ON, and it
//                  #else's that block out of the single-draw path. (The
//                  give-all path is not guarded the same way - an asymmetry in
//                  the studio's own source, left alone rather than
//                  "corrected".)
//                * The resurrection and stamina cubes' own counters. They are
//                  carried on the packet and passed through, but nothing
//                  offline consumes them.
//
//              ED: SERV_CUBE_OPEN_ED_CONDITION is ON, which means two things.
//              m_iRequiredED gates opening, and - the part that matters -
//              the client OVERWRITES the character's ED with the ACK's m_iED
//              (X2UIInventory.cpp:9121). Sending the default 0 would zero the
//              player's money on every cube. The handler always fills it.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

class CX2OfflineRandomItem
{
public:
	/// One cube, keyed by (its item ID, the key item it wants). A cube with no
	/// key requirement has m_iKeyItemID 0.
	struct KCube
	{
		int		m_iItemID;
		bool	m_bGiveAll;
		int		m_iUseCondition;		///< CXSLItem::USE_CONDITION
		int		m_iKeyItemID;
		int		m_iRequiredKeyCount;
		int		m_iRessurectionCount;
		int		m_iRestoreSpirit;
		int		m_iRequiredED;

		/// unit class -> item group. For UC_ANYONE there is exactly one entry,
		/// with class UC_NONE, and the class is not consulted.
		std::vector< std::pair< int, int > >	m_vecUnitGroup;

		KCube()
			: m_iItemID( 0 )
			, m_bGiveAll( false )
			, m_iUseCondition( 0 )
			, m_iKeyItemID( 0 )
			, m_iRequiredKeyCount( 0 )
			, m_iRessurectionCount( 0 )
			, m_iRestoreSpirit( 0 )
			, m_iRequiredED( 0 )
		{
		}
	};

	/// One thing the cube handed over.
	struct KResult
	{
		int	m_iItemID;
		int	m_iQuantity;
		int	m_iPeriod;		///< days; 0 is permanent

		KResult() : m_iItemID( 0 ), m_iQuantity( 0 ), m_iPeriod( 0 ) {}
	};

public:
	static CX2OfflineRandomItem*	Instance();
	static void						Release();

	/// The cube this item is, or NULL when the item is not a cube in the table.
	/// iKeyItemID picks between the several rows a cube can have - one per key
	/// it accepts - and 0 is the no-key row.
	const KCube*	GetCube( int iItemID, int iKeyItemID );

	/// Every key this cube accepts, as keyItemID -> required count. A single
	/// entry of ( 0, 0 ) means it needs no key. NULL when the item is not a
	/// cube. Mirrors CXSLRandomItemManager::GetRandomItemKeyList, which
	/// KGSInventory walks to pick the first key the player actually holds.
	const std::map< int, int >*	GetKeyList( int iItemID );

	/// The draw. Returns false when the cube resolves to no item group for this
	/// character, or the group is empty, or the odds did not sum high enough to
	/// hit anything - all of which are the live server's failure cases too, and
	/// all of which must leave the cube in the bag.
	bool	GetResult( int iUnitClass, const KCube& kCube, OUT std::vector< KResult >& vecOut );

	/// RandomItemData.lua's presentation lists. A sealed result is handed over
	/// sealed; an announced one is broadcast. Neither changes what is drawn, and
	/// the offline server has nobody to broadcast to - they are read only to
	/// keep the log honest about what the live client would have shown.
	bool	IsSealedCube( int iItemID );
	bool	IsAnnouncedResult( int iItemID );

	/// The bonus item this cube pays alongside the draw, or 0. 76 cubes have one.
	int		GetCharmBonusItem( int iItemID );

	bool	IsLoaded();

	//////////////////////////////////////////////////////////////////////////
	// Bound into the Lua state as g_pRandomItemManager:*. Public because
	// lua_tinker needs their addresses; every entry point the two files reach
	// has to be here or the chunk dies at the first unbound call.

	/// RandomItemTable.lua: g_pRandomItemManager:AddRandomItemTemplet{ ... }.
	/// The block is the only argument and is read off the Lua stack, exactly as
	/// CXSLRandomItemManager::AddRandomItemTemplet_LUA reads it.
	void	AddRandomItemTemplet_LUA();
	void	AddRandomItemGroup_LUA( int iGroupID, int iItemID, float fRate,
									int iPeriod, int iQuantity );

	void	SetNotifyMsg_LUA( bool bVal );
	void	AddAttractionItemInfo_LUA( int iItemID, bool bNotify );
	void	AddSealRandomItemInfo_LUA( int iItemID );
	void	AddNotifyResultItemInfo_LUA( int iItemID );
	void	AddCharmItem_LUA( int iItemID, int iBonusItemID );

	/// SERV_CUBE_IN_ITEM_MAPPING's timed group swap - bound, not implemented.
	/// See the header comment.
	void	AddRandomItemMappingData_LUA( int iIndex, int iCubeID, int iKeyItemID,
										  int iBeforeGroupID, int iAfterGroupID );
	void	AddRandomItemMappingTime_LUA( int iIndex, const char* szStart, const char* szEnd );

private:
	CX2OfflineRandomItem();
	~CX2OfflineRandomItem();

	void	EnsureLoaded();
	bool	RunScript( const wchar_t* szName, int& iRowCounter );

	/// Which item group this cube gives to this character, or 0.
	int		ResolveGroup( int iUnitClass, const KCube& kCube ) const;

private:
	/// One row of an item group: what can come out, how likely, for how long,
	/// and how many. Kept as a vector rather than KLottery's map because the
	/// map's only job there is collapsing a repeated case ID, and
	/// RandomItemTable.lua does repeat item IDs inside a group with different
	/// periods - collapsing them would throw rows away.
	struct KGroupCase
	{
		int		m_iItemID;
		float	m_fRate;		///< percentage
		int		m_iPeriod;
		int		m_iQuantity;

		KGroupCase()
			: m_iItemID( 0 ), m_fRate( 0.0f ), m_iPeriod( 0 ), m_iQuantity( 0 )
		{
		}
	};

	static CX2OfflineRandomItem*	ms_pInstance;

	/// (cube item ID, key item ID) -> cube
	std::map< std::pair< int, int >, KCube >	m_mapCube;

	/// cube item ID -> ( key item ID -> required count )
	std::map< int, std::map< int, int > >		m_mapCubeKey;

	/// group ID -> its cases, in file order
	std::map< int, std::vector< KGroupCase > >	m_mapGroup;

	std::set< int >		m_setSealed;
	std::set< int >		m_setAnnounced;
	std::map< int, int >	m_mapCharmBonus;

	bool	m_bNotifyMsg;
	bool	m_bLoadAttempted;
	int		m_iCubeRows;
	int		m_iGroupCaseRows;
	int		m_iPresentationRows;

	/// Templet blocks the file offered and this build dropped - almost all of
	/// them for naming an item ID with no client templet. Logged as a count
	/// rather than 4000 lines.
	int		m_iCubeDropped;
};

#endif SERV_IRUHADEV_OFFLINE
