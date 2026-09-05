#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-05
// Description: Offline mode - phase 12 (OFFLINE_MODE_PHASE9_PLAN.md), real
//              dismantle materials instead of a permanent refusal.
//
//              The output-materials table lives only on the server:
//              KncWX2Server/ServerResource/US/ResolveTable.lua, loaded through
//              CXSLResolveItemManager (KncWX2Server/Common/X2Data/
//              XSLResolveItemManager.{h,cpp}). This class is that manager's
//              client-side twin - same maps, same lottery math, loaded the
//              same way X2OfflineDropTable/X2OfflineStatTable load their
//              files: XOR-decrypted out of data036.kom via the client's own
//              mass-file loader and Lua state.
//
//              Only the SERV_ITEM_RESOLVE_NEW branch of
//              CXSLResolveItemManager is ported - ResolveTable.lua's own
//              calls (AddResolveDataBrokenPiece with a 3-int-field table,
//              g_pResolveItemManager:AddWeaponResolveBrokenPieceRate, etc.)
//              are that branch's shape, so the file itself confirms which
//              branch actually shipped. The pre-2011 EQIP_POSITION branch is
//              not implemented; nothing in the packed file calls it.
//
//              GetResultItem is KInventory::ResolveItem's "not fashion" half
//              (Inventory.cpp:11974-12029) plus
//              CXSLResolveItemManager::GetResultItem itself
//              (XSLResolveItemManager.cpp:306-439), collapsed into one call:
//              the caller (Handler_EGS_RESOLVE_ITEM_REQ) computes the sell
//              price and passes it in, exactly the way
//              Handler_EGS_SELL_ED_ITEM_REQ already does for the shop - both
//              are KInventory::GetSellPrice, byte for byte, and the price
//              belongs to CX2OfflineInventory's territory rather than this
//              class's.
//
//              Fashion (avatar/costume) items are NOT handled here. Their
//              material comes from CXSLCashItemManager's cash-item price
//              table (GetCashItemPriceFromScript, Inventory.cpp:11939), a
//              different server-only file this phase does not have; the
//              handler refuses those before ever reaching this class.
//
//              The jackpot gate (bJackpot) is genuinely per-session state
//              (KGSUser::m_kTimer[TM_RESOLVE_JACKPOT] / m_dResolveJackpotTime,
//              GSUserInventory.cpp:2894-2944) - not this manager's - so it
//              lives on KOfflineSession, not here. This class only owns the
//              stateless half: the jackpot-confirmation roll and the random
//              cooldown draw (GetJackpotTime), exactly as
//              CXSLResolveItemManager itself does.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

class CX2OfflineResolveTable
{
public:
	static CX2OfflineResolveTable*	Instance();
	static void						Release();

	/// KInventory::ResolveItem's non-fashion arm + CXSLResolveItemManager::
	/// GetResultItem, both SERV_ITEM_RESOLVE_NEW. iItemGrade/iItemType are
	/// CX2Item::ITEM_GRADE/ITEM_TYPE cast to int; iSellPrice is the caller's
	/// own KInventory::GetSellPrice port. bJackpot is IN OUT: in, whether the
	/// session's timer says this attempt is eligible; out, whether the
	/// manager's own confirmation roll (SetJackpotProb) actually granted it -
	/// same two-stage gate the real server has.
	///
	/// Returns NetError::NET_OK with mapResult filled on success, or the
	/// NetError the real call chain would have set: ERR_RESOLVE_ITEM_02 for an
	/// item type with no case in the switch (a non-fashion accessory, mainly)
	/// or ERR_RESOLVE_ITEM_04 if the table itself never loaded.
	int		GetResultItem( int iItemGrade, int iUseLevel, int iItemType, int iSellPrice,
						   IN OUT bool& bJackpot, OUT std::map< int, int >& mapResult );

	/// CXSLResolveItemManager::GetJackpotTime - a fresh random cooldown in
	/// seconds, drawn from SetMinMaxOfJackpotTime's range (10800-21600 in the
	/// packed file, i.e. 3-6 hours), or 10800 if the range was never set.
	int		GetJackpotTime() const;

	bool	IsLoaded() const	{ return m_bLoaded; }

	//////////////////////////////////////////////////////////////////////////
	// Bound into Lua as g_pResolveItemManager:* - public for lua_tinker.

	void	SetMinMaxOfJackpotTime_LUA( int iMin, int iMax );
	void	SetJackpotProb_LUA( float fRate );
	void	AddWeaponResolveBrokenPieceRate_LUA( int iItemType, float fRate );
	void	AddArmorResolveBrokenPieceRate_LUA( int iItemType, float fRate );

	/// ResolveTable.lua's only two data calls. Both take the row as a third,
	/// table argument that is still on the Lua stack when this runs -
	/// KLuaManager reads it directly, exactly the way
	/// CXSLResolveItemManager::AddResolveDataBrokenPiece_LUA /
	/// AddResolveData_LUA and CX2OfflineDropTable::ReadDropBlock all do.
	void	AddResolveDataBrokenPiece_LUA( int iResolveResultType, int iItemID );
	void	AddResolveData_LUA( int iItemGrade, int iItemID );

private:
	enum
	{
		RRT_EL_FLOUR			= 0,
		RRT_WEAPON_BROKEN_PIECE	= 1,
		RRT_ARMOR_BROKEN_PIECE	= 2,
		RRT_MAX					= 3,
	};

	struct KBrokenPieceResolve
	{
		int	m_iItemID;
		int	m_iUnitCost;
		int	m_iBeginLv;
		int	m_iEndLv;

		KBrokenPieceResolve() : m_iItemID( 0 ), m_iUnitCost( 0 ), m_iBeginLv( 0 ), m_iEndLv( 0 ) {}
	};

	struct KResolveData
	{
		int		m_iResultItemID;
		float	m_fSuccRate;		///< percent, 0-100
		int		m_iRandomCount;
		int		m_iResultCount;
		int		m_iBeginLv;
		int		m_iEndLv;

		KResolveData()
			: m_iResultItemID( 0 ), m_fSuccRate( 0.0f ), m_iRandomCount( 0 )
			, m_iResultCount( 0 ), m_iBeginLv( 0 ), m_iEndLv( 0 )
		{
		}
	};

private:
	CX2OfflineResolveTable();
	~CX2OfflineResolveTable();

	/// Loads on first use, for the same reason every other offline Lua table
	/// does: the offline server is constructed before the .kom archives are
	/// mounted.
	void	EnsureLoaded();

	/// CXSLResolveItemManager::ResolveResult_EnchantStone, verbatim including
	/// its one asymmetry: a brand-new map entry doubles under jackpot, a merge
	/// into an existing one does not. See the .cpp for why that is transcribed
	/// rather than fixed.
	void	ResolveResultEnchantStone( const KResolveData& kData, bool bJackpot,
									   OUT std::map< int, int >& mapResult ) const;

	/// CXSLResolveItemManager::ResolveResult_BrokenPiece, verbatim.
	void	ResolveResultBrokenPiece( int eType, int iEqipLevel, int iSellPrice,
									  OUT std::map< int, int >& mapResult ) const;

	/// KLottery::Decision for a single registered case: one roll in
	/// [0.00, 100.00), the case wins if the roll falls at or under its weight.
	/// Returns iCaseValue on a hit, -1 (KLottery::CASE_BLANK) otherwise.
	static int	LotteryDecideOne( int iCaseValue, float fWeight );

	/// KLottery::Decision for several registered cases: one roll, cases
	/// accumulated in registration order, the first whose running total
	/// reaches the roll wins. Returns -1 if the roll falls past every case
	/// (only possible when the weights do not sum to 100).
	static int	LotteryDecideMulti( const std::vector< std::pair< int, float > >& vecCase );

private:
	static CX2OfflineResolveTable*	ms_pInstance;

	bool	m_bLoadAttempted;
	bool	m_bLoaded;

	int		m_iMinJackpotTime;
	int		m_iMaxJackpotTime;
	float	m_fJackpotProb;

	/// item type -> rate. One vector each because AddWeaponResolveBrokenPieceRate
	/// and AddArmorResolveBrokenPieceRate are two separate Lua-bound methods on
	/// the real manager, not one keyed by a "which lottery" argument.
	std::vector< std::pair< int, float > >	m_vecWeaponBrokenRate;
	std::vector< std::pair< int, float > >	m_vecArmorBrokenRate;

	std::vector< KBrokenPieceResolve >				m_vecBrokenPiece[ RRT_MAX ];
	std::map< int, std::vector< KResolveData > >	m_mapResolveData;	///< item grade -> rows

	int		m_iResolveDataRows;
	int		m_iBrokenPieceRows;
};

#endif SERV_IRUHADEV_OFFLINE
