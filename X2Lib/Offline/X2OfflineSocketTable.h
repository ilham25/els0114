#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-05
// Description: Offline mode - phase 24 (OFFLINE_MODE_PHASE9_PLAN.md), real
//              socketing instead of a permanent refusal.
//
//              CXSLSocketItem reduced to the four things KInventory::SocketItem
//              actually calls: is this item a magic stone, what socket pool
//              does it draw from, what does one insert cost, and what option
//              comes out. Everything else in that 2000-line class - the option
//              stat maths, the set-item effects, the cash-avatar groups - is
//              already in the client's own CX2SocketItem, which loads
//              OptionItemData.lua and SocketOptionForm.lua at start-up
//              (X2StateStartUp.cpp:189). This class does not touch any of it:
//              the option ID this rolls goes into the item's socket list and
//              the client reads it back through its own tables.
//
//              The one server-only file is SocketItemTable.lua, and it is the
//              biggest of the four this batch needed: 216 AddSocketRandomData
//              rows (socket data type -> option group, weighted) and 3586
//              AddSocketGroup rows (group + enchant-level band -> option ID,
//              weighted). The client loads neither and binds neither method.
//
//              **This loader borrows a global and gives it back**, the same
//              way X2OfflineAttribTable does and for the same reason: the file
//              is written against `g_pCX2SocketItem`, which is the name the
//              client's own CX2SocketItem publishes for itself
//              (X2SocketItem.cpp:69), and the client binds only
//              AddSocketData_LUA / AddSocketDataTrans_LUA /
//              AddSocektGroupDataForCashAvatar_LUA on it - not the two methods
//              this file calls.
//
//              The magic-stone table below was extracted mechanically from
//              CXSLSocketItem::GetSocketDataType with this build's flag set
//              applied, and it checks itself: the 58 stones it maps are
//              exactly the 58 CXSLSocketItem::IsMagicStoneItemID accepts. The
//              client's UI additionally offers 16 Luriel stones
//              (85003840.., 152000699..) that are NOT here, because
//              SERV_LURIEL_MAGIC_STONE is not defined anywhere in
//              KncWX2Server/Common - the live server would have refused them
//              too, and this refuses them with a log line rather than silently
//              rolling the wrong pool.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

class CX2OfflineSocketTable
{
public:
	static CX2OfflineSocketTable*	Instance();
	static void						Release();

	/// True once SocketItemTable.lua has been found, run, and produced rows.
	bool	IsLoaded();

	/// CXSLSocketItem::IsMagicStoneItemID - is this item ID a magic stone this
	/// build accepts? Answered from the same table GetSocketDataType uses, so
	/// the two can never disagree.
	bool	IsMagicStoneItemID( int iItemID );

	/// CXSLSocketItem::GetResultItem. iItemType is CX2Item::ITEM_TYPE,
	/// iEnchantLevel the target's current enchant level; a fashion item rolls
	/// as if it were +20, and a non-fashion item with use level 0 as if it
	/// were +8 (SERV_2012_PVP_SEASON2_1's PvP-gear rule, on in this build).
	/// False when the stone has no pool or the pool has no band for the level.
	bool	GetResultItem( int iMagicStoneItemID, int iItemType, int iUseLevel,
						   bool bFashion, int iEnchantLevel, OUT int& iResultSocketOption );

	/// CXSLSocketItem::CalcInsertSocketCost. bEmptySlot false means the slot
	/// already holds an option, which triples the price.
	bool	CalcInsertSocketCost( int iItemID, int iMagicStoneItemID,
								  int iAssignedSocketCount, bool bEmptySlot, OUT int& iCost );

	/// CXSLSocketItem::GetSocketCount - how many socket slots an item of this
	/// grade and type has at all. The values are the client's own
	/// CX2SocketItem::SOCKET_COUNT enum, which the socket window draws from,
	/// so the number of slots the player sees and the number the server will
	/// accept an index into cannot drift apart. 0 for a grade with no sockets.
	static int	GetSocketCount( int iItemGrade, int iItemType );

	//////////////////////////////////////////////////////////////////////////
	// Bound into Lua as g_pCX2SocketItem:* while SocketItemTable.lua runs.

	void	AddSocketRandomData_LUA( int iSocketDataType, int iGroupID, float fRate );
	void	AddSocketGroup_LUA( int iGroupID, int iBeginEnchantLv, int iEndEnchantLv,
								int iOptionID, float fRate );

private:
	typedef std::vector< std::pair< int, float > >	KProbRow;

	/// One band of a socket group: the enchant-level range it covers and the
	/// option lottery for it. A vector rather than a map because the real
	/// KSocketGroup is walked in order and the FIRST band containing the level
	/// wins - which is not the same as a lookup when bands overlap.
	struct KSocketBand
	{
		int			m_iBeginEnchantLv;
		int			m_iEndEnchantLv;
		KProbRow	m_vecOption;

		KSocketBand() : m_iBeginEnchantLv( 0 ), m_iEndEnchantLv( 0 ) {}
	};

private:
	CX2OfflineSocketTable();
	~CX2OfflineSocketTable();

	void	EnsureLoaded();

	/// The (magic stone -> socket data type) half of
	/// CXSLSocketItem::GetSocketDataType. Returns SDT_NONE (0) for a stone
	/// this build does not accept.
	static int	SocketDataType( int iMagicStoneItemID, int iItemType );

	static int	LotteryDecideMulti( const KProbRow& kRow );

private:
	static CX2OfflineSocketTable*	ms_pInstance;

	bool	m_bLoadAttempted;
	bool	m_bLoaded;

	std::map< int, KProbRow >					m_mapSocketRandom;	///< SDT -> group lottery
	std::map< int, std::vector< KSocketBand > >	m_mapSocketGroup;	///< group -> bands

	int		m_iRandomRows;
	int		m_iGroupRows;
};

#endif SERV_IRUHADEV_OFFLINE
