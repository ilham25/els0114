#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-04
// Description: Offline mode - the cash shop catalog (phase 7).
//
//              The catalog is `cash_product` in els_db.sql, seeded by the
//              schema v9 migration from X2OfflineCashSeed.h - a transcription
//              of dbo.EB_Product out of the ES_BILLING database. That is where
//              this client's shop was really populated from: the billing DB,
//              reached through the LoginServer, which is the whole reason
//              EGS_BILL_PRODUCT_INFO_ACK exists as a packet.
//
//              **This replaces the CashItemPrice.lua catalog phase 7 shipped
//              first, which was wrong.** That file is the GameServer's
//              item-resolve price lookup, not a sale list: it prices 9,947
//              items where the shop sold 2,342, and it has a price for only
//              789 of the ones actually on sale. Built from it, the shop showed
//              about 3,000 items that were never purchasable and put every one
//              of them in a tab derived from the item templet.
//
//              Two things that were guessed before now come from the table:
//
//              * m_cCategoryNo is the BILLING category (11..63), not the client
//                CSSC_* enum. CX2CashShop::GetItemByCategory compares it
//                against the second element of each CASH_SHOP_REAL_SUB_CATEGORY
//                _ID pair in CashShopCategory.lua - a different number space
//                entirely from the enum, which is what CategoryFor used to
//                return. Anything above 127 could never have matched either:
//                the field is a signed char.
//              * m_cQuantity is the real per-purchase count (1, 10, 20 or 100).
//
//              Still not from the table, because it is constant across all
//              2360 source rows: the period (0, permanent), the level limit,
//              the show/sale flags, and the gift flag (off). See
//              X2OfflineCashSeed.h.
//
//              m_cPeriod stays 0. With no rental period anywhere in the source
//              data there is nothing to model, and the client renders the
//              quantity instead of a day count when it is zero - so the shop
//              says "1" or "100", which is true.
#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineDB.h"

class CX2OfflineCashShop
{
public:
	enum
	{
		/// The socket ("attribute") group a cash weapon and a cash armour piece
		/// offer in the send-to-inventory popup.
		///
		/// These are the client's own numbers, not ours: CX2SocketItem::
		/// GetSocketIdListForCashAvatar falls back to exactly these two when it
		/// is handed a group ID that is not in its table
		/// ([X2SocketItem.cpp:2214-2231](X2Lib/X2SocketItem.cpp#L2214), the
		/// SERV_GLOBAL_BILLING values - the non-global build negates them).
		/// Live gets a per-product group out of the billing DB; offline there is
		/// no such row, so the fallback group is what every cash item offers.
		CASH_SOCKET_GROUP_DEFENCE	= 1000,
		CASH_SOCKET_GROUP_WEAPON	= 2000,

		/// How many products go in one EGS_BILL_PRODUCT_INFO_ACK. The client
		/// asks page by page until m_bFinal, so this is purely a packet-size
		/// choice: 2360 products at ~50 bytes each in one event would be 120KB.
		PRODUCTS_PER_PAGE	= 400,
	};

	/// One catalog line - the offline half of KBillProductInfo.
	struct KProduct
	{
		int		m_iProductNo;		///< == m_iItemID; see the file comment
		int		m_iItemID;
		int		m_iPrice;
		int		m_iCategoryNo;		///< CX2CashShop::CASH_SHOP_SUB_CATEGORY
		int		m_iQuantity;
		int		m_iPeriod;			///< rental days; 0 permanent
		bool	m_bEvent;			///< DI_ISEVENT - drives the shop's event badge

		KProduct()
			: m_iProductNo( 0 )
			, m_iItemID( 0 )
			, m_iPrice( 0 )
			, m_iCategoryNo( 0 )
			, m_iQuantity( 1 )
			, m_iPeriod( 0 )
			, m_bEvent( false )
		{
		}
	};

public:
	static CX2OfflineCashShop*	Instance();
	static void					Release();

	/// Reads `cash_product` on first call and never again.
	///
	/// Deferred rather than done at startup because a product whose item the
	/// client cannot resolve is dropped, and the item templets are not loaded
	/// when the offline server is constructed. Unlike the Lua loader it
	/// replaced, this needs no .kom and no XOR - the catalog is in the save
	/// file, and can be edited there.
	void	EnsureLoaded();

	bool	IsLoaded() const					{ return m_bLoadAttempted && false == m_vecProduct.empty(); }
	int		Count() const						{ return (int)m_vecProduct.size(); }

	/// How many pages GetPage will produce. At least 1, so an empty catalog
	/// still answers one final page rather than none - the client loops until
	/// m_bFinal and would ask forever otherwise.
	int		PageCount() const;

	/// One page of the catalog, in the shape EGS_BILL_PRODUCT_INFO_ACK wants.
	/// iPage is 1-based, matching KEGS_BILL_PRODUCT_INFO_REQ::m_iPage.
	void	GetPage( int iPage, OUT std::vector< KBillProductInfo >& vecOut, OUT bool& bFinal ) const;

	const KProduct*	Find( int iProductNo ) const;

	/// Fill a KBillProductInfo from a product, for the deposit list as well as
	/// for the catalog - KBillOrderInfo embeds one.
	static void		MakeProductInfo( const KProduct& kProduct, OUT KBillProductInfo& kOut );

	/// A deposit row as the client wants to see it.
	static void		MakeOrderInfo( const KOfflineCashOrderRow& kRow, OUT KBillOrderInfo& kOut );

	/// Is iSocketID one of the sockets iItemID is actually allowed to be given?
	///
	/// The chosen socket arrives from the client in
	/// EGS_BILL_GET_PURCHASED_CASH_ITEM_REQ::m_mapSocketForCash, and the client
	/// is trusted about combat but not about this - a socket outside the item's
	/// group would be a permanent stat the game never offered. The check runs
	/// through the same CX2SocketItem::GetSocketIdListForCashAvatar the combo
	/// box was built from, so the two cannot disagree.
	///
	/// An item that cannot hold a socket at all answers false for any non-zero
	/// socket, and true for 0 ("none").
	static bool	IsSocketAllowed( int iItemID, int iSocketID );

	/// Which cash-shop tab an item belongs under, from its templet alone.
	///
	/// The live value comes out of the billing DB. This is a mapping over the
	/// client's own item data instead: fashion pieces by equip position into
	/// the CSSC_FASHION_* range, accessories into CSSC_ACCESSORY_*, consumables
	/// into CSSC_CONSUME_*, and everything else into CSSC_INSTALL_ETC. It only
	/// decides which tab shows the item; nothing about price or contents
	/// depends on it.
	static int		CategoryFor( const CX2Item::ItemTemplet* pTemplet );


private:
	CX2OfflineCashShop();
	~CX2OfflineCashShop();


private:
	static CX2OfflineCashShop*	ms_pInstance;

	bool						m_bLoadAttempted;

	/// Every row `cash_product` offered, and the ones dropped for naming an item
	/// this build has no templet for - so the log can say how many of each.
	int							m_iRows;
	int							m_iDroppedRows;

	/// The catalog, in item-ID order, and an index into it by product number.
	std::vector< KProduct >		m_vecProduct;
	std::map< int, int >		m_mapProductIndex;
};

#endif SERV_IRUHADEV_OFFLINE
