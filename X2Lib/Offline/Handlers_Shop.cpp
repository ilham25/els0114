#include "stdafx.h"
#include "X2OfflineServer.h"

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-04
// Description: Offline mode - phase 7. The cash shop.
//
//              The flow the client's own packets describe, and the one
//              implemented here:
//
//                open      EGS_BILL_PRODUCT_INFO_REQ, page after page until
//                          m_bFinal, then EGS_GET_WISH_LIST_REQ
//                buy       EGS_BUY_CASH_ITEM_REQ - charge the wallet and put a
//                          line in the deposit; the item does NOT reach the bag
//                claim     EGS_BILL_GET_PURCHASED_CASH_ITEM_REQ - take the line
//                          out of the deposit and put the item in the bag
//                balance   EGS_CHECK_BALANCE_REQ, which the client re-sends
//                          itself after every purchase (X2CashShop.cpp:3580)
//
//              The deposit is a real table (cash_order) rather than a shortcut
//              straight into the inventory, because the client has a whole
//              window for it and asks for it by name; skipping it would leave
//              that window permanently empty and the "get item" button inert.
//
//              What the catalog is and where it comes from: see
//              X2OfflineCashShop.h. What a purchase deliberately refuses, and
//              why: see NOT_MODELLED below.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineCashShop.h"
#include "X2OfflineClassChangeSeed.h"
#include "X2OfflineSkill.h"
#include "X2OfflineStatTable.h"
#include "X2OfflineQuest.h"
#include "X2OfflineInventory.h"

namespace
{
	/// Cash items whose whole point is a side effect the offline server does
	/// not model. Buying one would take the money and hand back an item that
	/// does nothing, so the purchase is refused instead and the log says why.
	///
	/// Every ID here is a named constant in the client's own X2Define.h, or is
	/// named in the client's own purchase handler - none is guessed. The
	/// INVENTORY_SLOT_ADD_ITEM* cards used to sit in this list too, before
	/// offline phase 9 made them purchasable for real - see the claim handler
	/// below.
	///
	///   127030   the resurrection stone. It is not an inventory item at all -
	///            it is a counter on the character (CX2Unit::
	///            GetResurrectionStoneNum), reported as 0 since phase 4 and
	///            not persisted anywhere. X2CashShop.cpp:3508 names the ID.
	const int NOT_MODELLED[] =
	{
		127030,
	};

	bool IsNotModelled( int iItemID )
	{
		for( int i = 0; i < (int)( sizeof( NOT_MODELLED ) / sizeof( NOT_MODELLED[0] ) ); ++i )
		{
			if( NOT_MODELLED[i] == iItemID )
				return true;
		}

		return false;
	}

	/// Every card grants this many slots per category - CXSLInventory::
	/// SLOT_COUNT_ONE_LINE (KncWX2Server/Common/X2Data/XSLInventory.h:54).
	/// Read for structure, same as ClassChangeTargetOf's table below: it is a
	/// compile-time constant in the shared header, not data behind a stored
	/// procedure, so citing it here is honest and does not need the user's DB.
	const int EXPAND_SLOT_INCREMENT = 8;

	/// The six categories INVENTORY_SLOT_ADD_ITEM (200750, the bundle card)
	/// expands at once - GSUserFunction.cpp's CI_EXPAND_INVENTORY case.
	const int EXPAND_ALL_CATEGORIES[] =
	{
		CX2Inventory::ST_EQUIP,
		CX2Inventory::ST_ACCESSORY,
		CX2Inventory::ST_MATERIAL,
		CX2Inventory::ST_SPECIAL,
		CX2Inventory::ST_QUEST,
		CX2Inventory::ST_QUICK_SLOT,
	};

	/// The one category a per-category card (200751..200756, or their _EVENT
	/// twins) expands, or ST_NONE for the bundle card or anything else -
	/// X2CashShop.cpp:3242's switch, and GSUserFunction.cpp's per-category
	/// CI_EXPAND_INVENTORY_* cases.
	int ExpandedCategoryOf( int iItemID )
	{
		switch( iItemID )
		{
		case INVENTORY_SLOT_ADD_ITEM_EQUIP:
		case INVENTORY_SLOT_ADD_ITEM_EQUIP_EVENT:
			return CX2Inventory::ST_EQUIP;
		case INVENTORY_SLOT_ADD_ITEM_ACCESSORY:
		case INVENTORY_SLOT_ADD_ITEM_ACCESSORY_EVENT:
			return CX2Inventory::ST_ACCESSORY;
		case INVENTORY_SLOT_ADD_ITEM_QUICK_SLOT:
		case INVENTORY_SLOT_ADD_ITEM_QUICK_SLOT_EVENT:
			return CX2Inventory::ST_QUICK_SLOT;
		case INVENTORY_SLOT_ADD_ITEM_MATERIAL:
		case INVENTORY_SLOT_ADD_ITEM_MATERIAL_EVENT:
			return CX2Inventory::ST_MATERIAL;
		case INVENTORY_SLOT_ADD_ITEM_QUEST:
		case INVENTORY_SLOT_ADD_ITEM_QUEST_EVENT:
			return CX2Inventory::ST_QUEST;
		case INVENTORY_SLOT_ADD_ITEM_SPECIAL:
		case INVENTORY_SLOT_ADD_ITEM_SPECIAL_EVENT:
			return CX2Inventory::ST_SPECIAL;
		default:
			return CX2Inventory::ST_NONE;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// the catalog

bool CX2OfflineServer::Handler_EGS_BILL_PRODUCT_INFO_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_BILL_PRODUCT_INFO_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineCashShop* pShop = CX2OfflineCashShop::Instance();
	pShop->EnsureLoaded();

	KEGS_BILL_PRODUCT_INFO_ACK kAck;
	pShop->GetPage( kReq.m_iPage, kAck.m_vecProductInfo, kAck.m_bFinal );

#ifdef SERV_KEEP_ITEM_SHOW_CASHSHOP
	// "you must already own item X for product Y to be visible". The list lives
	// in the billing DB; empty means every product is visible, which is the
	// permissive answer and the one that cannot hide a product by accident.
	kAck.m_mapKeepShowItem.clear();
	kAck.m_DisCountInfoMap.clear();
#endif SERV_KEEP_ITEM_SHOW_CASHSHOP

	CX2OfflineLog::Server( L"CASH     catalog page %d: %u product(s)%s",
		kReq.m_iPage, (unsigned int)kAck.m_vecProductInfo.size(),
		true == kAck.m_bFinal ? L" (last)" : L"" );

	return Reply( kSes, EGS_BILL_PRODUCT_INFO_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// the wish list

bool CX2OfflineServer::Handler_EGS_GET_WISH_LIST_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	// Sent with SendID - no body, so nothing to deserialize.
	std::vector< int > vecItemID;
	CX2OfflineDB::Instance()->LoadWishList( kSes.m_nUserUID, vecItemID );

	// The map is <itemID, itemID>. CX2CashShop::Handler_EGS_GET_WISH_LIST_ACK
	// walks it and only ever reads the key; the server filled both halves the
	// same way.
	KEGS_GET_WISH_LIST_ACK kAck;
	for( size_t i = 0; i < vecItemID.size(); ++i )
		kAck.m_mapWishList[ vecItemID[i] ] = vecItemID[i];

	return Reply( kSes, EGS_GET_WISH_LIST_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_MODIFY_WISH_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_MODIFY_WISH_LIST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineDB* pDB = CX2OfflineDB::Instance();

	for( size_t i = 0; i < kReq.m_vecWishListItemID.size(); ++i )
		pDB->SetWishListItem( kSes.m_nUserUID, kReq.m_vecWishListItemID[i], false == kReq.m_bIsDelete );

	KEGS_MODIFY_WISH_LIST_ACK kAck;
	kAck.m_iOK					= NetError::NET_OK;
	kAck.m_vecWishListItemID	= kReq.m_vecWishListItemID;
	kAck.m_bIsDelete			= kReq.m_bIsDelete;

	return Reply( kSes, EGS_MODIFY_WISH_LIST_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// the deposit

bool CX2OfflineServer::Handler_EGS_BILL_INVENTORY_INQUIRY_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_BILL_INVENTORY_INQUIRY_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	std::vector< KOfflineCashOrderRow > vecRow;
	CX2OfflineDB::Instance()->LoadCashOrders( kSes.m_nUserUID, vecRow );

	KEGS_BILL_INVENTORY_INQUIRY_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	// The request names a page and a page size, and the ACK has nowhere to say
	// which page it is answering - the client tracks that itself in
	// m_iReqDepositPageIndex. So the page is honoured here rather than ignored,
	// or a second page of deposit lines would draw over the first.
	const int iPerPage	= kReq.m_nItemPerPage > 0 ? kReq.m_nItemPerPage : (int)vecRow.size();
	const int iPage		= kReq.m_iCurrentPage > 0 ? kReq.m_iCurrentPage : 1;
	const int iFirst	= ( iPage - 1 ) * iPerPage;

	for( int i = iFirst; i < iFirst + iPerPage && i < (int)vecRow.size(); ++i )
	{
		KBillOrderInfo kOrder;
		CX2OfflineCashShop::MakeOrderInfo( vecRow[i], kOrder );
		kAck.m_vecOrderInfo.push_back( kOrder );
	}

#ifdef SERV_GLOBAL_CASH_PACKAGE
	// Nothing offline creates a package purchase, so all three package vectors
	// stay empty - which is what the client reads as "no packages in the
	// deposit" rather than as an error.
	kAck.m_vecPackageInfo.clear();
	kAck.m_vecSubPackageTrans.clear();
	kAck.m_vecSubPackageInfo.clear();
#endif SERV_GLOBAL_CASH_PACKAGE

	CX2OfflineLog::Server( L"CASH     deposit page %d: %u of %u line(s)",
		iPage, (unsigned int)kAck.m_vecOrderInfo.size(), (unsigned int)vecRow.size() );

	return Reply( kSes, EGS_BILL_INVENTORY_INQUIRY_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// buying

bool CX2OfflineServer::Handler_EGS_BUY_CASH_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_BUY_CASH_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineCashShop*	pShop	= CX2OfflineCashShop::Instance();
	CX2OfflineDB*		pDB		= CX2OfflineDB::Instance();

	pShop->EnsureLoaded();

	KEGS_BUY_CASH_ITEM_ACK kAck;
	kAck.m_iOK			= NetError::ERR_BUY_CASH_ITEM_00;
	kAck.m_bIsWishList	= kReq.m_bIsWishList;

	if( true == kReq.m_vecPurchaseReqInfo.empty() )
		return Reply( kSes, EGS_BUY_CASH_ITEM_ACK, kAck );

#ifdef SERV_SUPPORT_SEVERAL_CASH_TYPES
	// The offline wallet is one number, and the US client's own default is
	// GCT_PUBLISHER_CASH (X2CashShop.cpp:11170) - so that is the only type
	// there is. Anything else is a client from another region's build path and
	// is refused rather than quietly charged to the one wallet.
	if( KGlobalCashInfo::GCT_PUBLISHER_CASH != kReq.m_iUseCashType )
	{
		CX2OfflineLog::Server( L"CASH     refused: cash type %d, and offline only has"
			L" GCT_PUBLISHER_CASH", kReq.m_iUseCashType );

		kAck.m_iOK = NetError::ERR_BUY_CASH_ITEM_01;
		return Reply( kSes, EGS_BUY_CASH_ITEM_ACK, kAck );
	}
#endif SERV_SUPPORT_SEVERAL_CASH_TYPES

	const int iBalance = pDB->GetWallet();

	// Price the whole cart before charging any of it, so a cart with one bad
	// line does not half-buy. This is KUserCashItemManager's shape too: it
	// validates every KNXBTPurchaseReqInfo before it sends a single order.
	int iTotal = 0;

	// The resolved cart: one entry per ordered unit, built here and inserted
	// below without a second look-up.
	//
	// It used to be two parallel vectors of item IDs and counts, with the
	// insert loop re-finding each product by item ID - which worked only while
	// product numbers WERE item IDs. Once the catalog started carrying real
	// product numbers (1..2360) that Find() returned NULL for every line, the
	// insert loop skipped every row, and the purchase reported success with an
	// empty deposit. Carrying the resolved row forward removes the class of bug
	// rather than the instance.
	std::vector< KOfflineCashOrderRow > vecOrder;

	for( size_t i = 0; i < kReq.m_vecPurchaseReqInfo.size(); ++i )
	{
		const KNXBTPurchaseReqInfo& kInfo = kReq.m_vecPurchaseReqInfo[i];

		const CX2OfflineCashShop::KProduct* pProduct = pShop->Find( (int)kInfo.m_ulProductNo );
		if( NULL == pProduct )
		{
			CX2OfflineLog::Server( L"CASH     refused: no product %lu in the catalog",
				kInfo.m_ulProductNo );
			return Reply( kSes, EGS_BUY_CASH_ITEM_ACK, kAck );
		}

		if( true == IsNotModelled( pProduct->m_iItemID ) )
		{
			CX2OfflineLog::Server( L"CASH     refused item %d - its effect (an inventory expansion"
				L" or the resurrection-stone counter) is not modelled offline, so the purchase"
				L" would take the cash and do nothing", pProduct->m_iItemID );

			kAck.m_iOK = NetError::ERR_BUY_CASH_ITEM_01;
			return Reply( kSes, EGS_BUY_CASH_ITEM_ACK, kAck );
		}

		// A quantity of 0 reaches here from a cart row the UI built but never
		// filled in; the real server treats it as one.
		const int iOrder = kInfo.m_usOrderQuantity > 0 ? (int)kInfo.m_usOrderQuantity : 1;

		iTotal += pProduct->m_iPrice * iOrder;

		// One deposit line per ordered unit, not one line carrying a count.
		// KBillOrderInfo has no quantity of its own - the count it shows comes
		// from its embedded product's m_cQuantity - so two of something has to
		// be two lines or the second one is lost.
		for( int n = 0; n < iOrder; ++n )
		{
			KOfflineCashOrderRow kRow;
			kRow.m_iProductNo	= pProduct->m_iProductNo;
			kRow.m_iItemID		= pProduct->m_iItemID;
			kRow.m_iQuantity	= pProduct->m_iQuantity;
			kRow.m_iPeriod		= pProduct->m_iPeriod;
			kRow.m_iPrice		= pProduct->m_iPrice;

			vecOrder.push_back( kRow );
		}
	}

	// The wallet is checked but never spent - this is a single-player game and
	// the cash shop is a wardrobe, not a transaction. The check is still here
	// because it is the one thing that makes the knob meaningful: setting
	// `settings.cash_start` to 0 turns the shop into a look-but-don't-touch
	// window, and lowering it below an item's price makes that item refuse.
	if( iTotal < 0 || iBalance < iTotal )
	{
		CX2OfflineLog::Server( L"CASH     refused: %d cash needed, wallet is %d", iTotal, iBalance );

		kAck.m_iOK = NetError::ERR_BUY_CASH_ITEM_16;		///< insufficient balance
		return Reply( kSes, EGS_BUY_CASH_ITEM_ACK, kAck );
	}

	// Straight to the deposit lines, with nothing deducted. No room check
	// either: a purchase lands in the deposit, not the bag, and the deposit has
	// no size - the bag is checked when the line is claimed.
	int iWritten = 0;
	for( size_t i = 0; i < vecOrder.size(); ++i )
	{
		if( true == pDB->InsertCashOrder( kSes.m_nUserUID, vecOrder[i] ) )
			++iWritten;
	}

	// A purchase that wrote nothing is a failed purchase, and it must not
	// report success - the client would say "purchased" over an empty deposit,
	// which is exactly how the product-number regression stayed invisible.
	if( 0 == iWritten )
	{
		CX2OfflineLog::Server( L"CASH     ERROR purchase wrote no deposit lines for %u cart"
			L" entrie(s) - nothing was bought", (unsigned int)vecOrder.size() );

		kAck.m_iOK = NetError::ERR_BUY_CASH_ITEM_01;
		return Reply( kSes, EGS_BUY_CASH_ITEM_ACK, kAck );
	}

	CX2OfflineLog::Server( L"CASH     bought %d deposit line(s) listed at %d cash;"
		L" wallet unchanged at %d", iWritten, iTotal, iBalance );

	if( iWritten < (int)vecOrder.size() )
	{
		CX2OfflineLog::Server( L"CASH     WARNING only %d of %u cart line(s) reached the deposit",
			iWritten, (unsigned int)vecOrder.size() );
	}

	kAck.m_iOK					= NetError::NET_OK;
	kAck.m_vecPurchaseReqInfo	= kReq.m_vecPurchaseReqInfo;

	return Reply( kSes, EGS_BUY_CASH_ITEM_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// claiming out of the deposit

namespace
{
	/// The UNIT_CLASS a "class change" cash item advances to, or 0.
	///
	/// These 42 items are the one thing the cash shop sells that must never
	/// reach the bag: on live the claim is intercepted before the insert
	/// (GSUserCashShop.cpp:2887) and turned into a class change, so the item is
	/// consumed by the act of claiming it and no row is ever written. Offline it
	/// was landing in the inventory as an ordinary item that did nothing, ever -
	/// which is exactly what "the class change item isn't working" looks like.
	int ClassChangeTargetOf( int iItemID )
	{
		for( int i = 0; i < X2OfflineClassChangeSeed::ROW_COUNT; ++i )
		{
			if( X2OfflineClassChangeSeed::ROWS[i].m_iItemID == iItemID )
				return X2OfflineClassChangeSeed::ROWS[i].m_iUnitClass;
		}

		return 0;
	}
}

bool CX2OfflineServer::Handler_EGS_BILL_GET_PURCHASED_CASH_ITEM_REQ( KOfflineSession& kSes,
																	 const KEvent& kEvent )
{
	KEGS_BILL_GET_PURCHASED_CASH_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineDB*			pDB		= CX2OfflineDB::Instance();
	CX2OfflineInventory*	pInven	= CX2OfflineInventory::Instance();

	KEGS_BILL_GET_PURCHASED_CASH_ITEM_ACK kAck;
	kAck.m_iOK						= NetError::ERR_BUY_CASH_ITEM_00;
	kAck.m_iNumResurrectionStone	= 0;
#ifdef SERV_GUILD_CHANGE_NAME
	kAck.m_iExceptionProcessItemID	= 0;
#endif SERV_GUILD_CHANGE_NAME

	// Empty unless an INVENTORY_SLOT_ADD_ITEM* card is claimed below. The
	// client adds each entry
	// onto its current slot count, so a non-empty map here that the save file
	// did not also grow would desync the two.
	kAck.m_mapExpandedCategorySlot.clear();

	// Find the line first, without deleting it: the bag might be full, and a
	// line consumed into a full bag is a purchase the player paid for and can
	// never get back.
	std::vector< KOfflineCashOrderRow > vecRow;
	pDB->LoadCashOrders( kSes.m_nUserUID, vecRow );

	const KOfflineCashOrderRow* pRow = NULL;
	for( size_t i = 0; i < vecRow.size(); ++i )
	{
		if( (__int64)kReq.m_iTransNo == vecRow[i].m_nTransNo )
		{
			pRow = &vecRow[i];
			break;
		}
	}

	if( NULL == pRow )
	{
		CX2OfflineLog::Server( L"CASH     claim refused: no deposit line %I64d",
			(__int64)kReq.m_iTransNo );

		return Reply( kSes, EGS_BILL_GET_PURCHASED_CASH_ITEM_ACK, kAck );
	}

	const int iItemID	= pRow->m_iItemID;
	const int iQuantity	= pRow->m_iQuantity > 0 ? pRow->m_iQuantity : 1;

	//////////////////////////////////////////////////////////////////////////
	// Class change: claimed, not carried.
	//
	// This runs before the room check on purpose. The item never enters the
	// inventory, so a full bag is no reason to refuse it - and refusing on a
	// full bag is what the room check below would have done.
	const int iNewClass = ClassChangeTargetOf( iItemID );

	if( 0 != iNewClass )
	{
		KOfflineUnitRow kUnit;
		if( false == pDB->LoadUnit( kSes.m_nSelectedUnitUID, kUnit ) )
			return Reply( kSes, EGS_BILL_GET_PURCHASED_CASH_ITEM_ACK, kAck );

		// The two guards the client itself applies before it will even offer the
		// purchase (X2CashShop.cpp:6096-6109), re-checked here because the client
		// is not the authority on what the save file may contain.
		if( iNewClass == kUnit.m_iUnitClass )
		{
			CX2OfflineLog::Server( L"CASH     class change refused: already class %d"
				L" (the deposit line is left in place)", iNewClass );

			kAck.m_iOK = NetError::ERR_BUY_CASH_ITEM_00;
			return Reply( kSes, EGS_BILL_GET_PURCHASED_CASH_ITEM_ACK, kAck );
		}

		if( CX2OfflineStatTable::UnitTypeOf( iNewClass ) !=
			CX2OfflineStatTable::UnitTypeOf( kUnit.m_iUnitClass ) )
		{
			CX2OfflineLog::Server( L"CASH     class change refused: item %d advances to class %d,"
				L" which is a different character to this one's class %d (the deposit line is"
				L" left in place)", iItemID, iNewClass, kUnit.m_iUnitClass );

			kAck.m_iOK = NetError::ERR_BUY_CASH_ITEM_00;
			return Reply( kSes, EGS_BILL_GET_PURCHASED_CASH_ITEM_ACK, kAck );
		}

		// Take the line out first - same ordering rule the item path below
		// follows. If it was not there after all, nothing should have happened.
		if( false == pDB->DeleteCashOrder( kSes.m_nUserUID, pRow->m_nTransNo ) )
		{
			CX2OfflineLog::Server( L"CASH     class change refused: deposit line %I64d could not"
				L" be removed", pRow->m_nTransNo );

			return Reply( kSes, EGS_BILL_GET_PURCHASED_CASH_ITEM_ACK, kAck );
		}

		const int iOldClass = kUnit.m_iUnitClass;

		// What DBE_BUY_UNIT_CLASS_CHANGE_ACK does, minus the parts that need
		// tables this build has no offline copy of. Kept: the class itself, and
		// the full skill-tree reset that goes with it - the live path resets the
		// tree, hands back every skill point, and re-grants the new class's
		// defaults, and InitSkillTree is that same operation.
		//
		// NOT kept, and worth being explicit about: the memo remap, the item
		// remap, and the quest remap the live packet also carries
		// (m_mapChangeMemo / m_vecChangeItem / m_vecChangeCompleteQuest). Those
		// come out of ClassChangeMemo.lua, ClassChangeItem.lua and
		// ClassChangeQuest.lua, none of which is packed. The consequence is
		// narrow and visible rather than silent: class-specific skill notes and
		// completed job quests keep their old class's IDs. The class change
		// itself, which is what the item is for, is complete.
		pDB->SaveUnitClass( kUnit.m_nUnitUID, iNewClass );

		int iSPTotal = 0;
		CX2OfflineSkill::Instance()->Load( kUnit.m_nUnitUID );
		CX2OfflineSkill::Instance()->InitSkillTree( kUnit.m_iLevel, iNewClass, iSPTotal );
		pDB->SaveSkillPoint( kUnit.m_nUnitUID, iSPTotal, 0 );

		kAck.m_iOK = NetError::NET_OK;
		Reply( kSes, EGS_BILL_GET_PURCHASED_CASH_ITEM_ACK, kAck );

		// The ACK first, then the notification - the client is holding an
		// AddServerPacket wait on the ACK, and EGS_BUY_UNIT_CLASS_CHANGE_NOT
		// rebuilds the character sheet and the skill window against a unit it has
		// not been told about yet if the two arrive the other way round. Same
		// ordering rule AfterQuestComplete follows for the job-advancement quest.
		KOfflineUnitRow kAfter;
		if( false == pDB->LoadUnit( kUnit.m_nUnitUID, kAfter ) )
			kAfter = kUnit;

		KEGS_BUY_UNIT_CLASS_CHANGE_NOT kNot;
		kNot.m_iUnitUID		= (UidType)kAfter.m_nUnitUID;
		kNot.m_iNewUnitClass	= iNewClass;
		MakeUnitInfoFromRow( kNot.m_kUnitInfo, kAfter );

		// The quest lists are NOT optional and NOT "the ones that changed",
		// whatever the field names suggest. CX2State's handler feeds them
		// straight into CX2QuestManager::SetUnitQuest, which opens with
		// ClearUnitQuest() and rebuilds from what it was handed
		// (X2QuestManager.cpp) - so sending the empty vectors this packet is
		// born with would blank the player's entire quest log the moment they
		// changed class, until the next relog put it back.
		//
		// On live the two lists are genuinely the remapped ones, because a
		// class change rewrites class-specific quest IDs out of
		// ClassChangeQuest.lua. That file is not packed, so no remap happens
		// here and the honest thing to send is the lists exactly as they stand.
		CX2OfflineQuest::Instance()->Load( kAfter.m_nUnitUID );
		CX2OfflineQuest::Instance()->GetQuestInstances( kNot.m_vecChangeInProgressQuest );
		CX2OfflineQuest::Instance()->GetCompleteQuests( kNot.m_vecChangeCompleteQuest );

		Reply( kSes, EGS_BUY_UNIT_CLASS_CHANGE_NOT, kNot );

		CX2OfflineLog::Server( L"CASH     class change: item %d took unitUID=%I64d from class %d"
			L" to %d; skill tree reset, %d SP returned",
			iItemID, (__int64)kAfter.m_nUnitUID, iOldClass, iNewClass, iSPTotal );

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Inventory expansion: claimed, not carried - same shape as the class
	// change above and for the same reason. The card's effect is the DB row,
	// not a bag item, so it runs before the room check rather than being
	// subject to it.
	{
		std::vector< int > vecCategory;

		const int iSingleCategory = ExpandedCategoryOf( iItemID );
		if( CX2Inventory::ST_NONE != iSingleCategory )
		{
			vecCategory.push_back( iSingleCategory );
		}
		else if( INVENTORY_SLOT_ADD_ITEM == iItemID )
		{
			vecCategory.assign( EXPAND_ALL_CATEGORIES,
				EXPAND_ALL_CATEGORIES + ( sizeof( EXPAND_ALL_CATEGORIES ) / sizeof( EXPAND_ALL_CATEGORIES[0] ) ) );
		}

		if( false == vecCategory.empty() )
		{
			// Take the line out first - same ordering rule the item path below
			// follows. If it was not there after all, nothing should have
			// happened.
			if( false == pDB->DeleteCashOrder( kSes.m_nUserUID, pRow->m_nTransNo ) )
			{
				CX2OfflineLog::Server( L"CASH     expand refused: deposit line %I64d could not be"
					L" removed", pRow->m_nTransNo );

				return Reply( kSes, EGS_BILL_GET_PURCHASED_CASH_ITEM_ACK, kAck );
			}

			for( size_t i = 0; i < vecCategory.size(); ++i )
			{
				int iGranted = 0;
				pInven->ExpandCategorySlot( vecCategory[i], EXPAND_SLOT_INCREMENT * iQuantity, iGranted );

				if( iGranted > 0 )
					kAck.m_mapExpandedCategorySlot[ vecCategory[i] ] = iGranted;
			}

			CX2OfflineLog::Server( L"CASH     claimed line %I64d: item %d expanded %u categor%s",
				pRow->m_nTransNo, iItemID, (unsigned int)kAck.m_mapExpandedCategorySlot.size(),
				1 == kAck.m_mapExpandedCategorySlot.size() ? L"y" : L"ies" );

			kAck.m_iOK = NetError::NET_OK;
			return Reply( kSes, EGS_BILL_GET_PURCHASED_CASH_ITEM_ACK, kAck );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// The cash skill note: claimed, not carried - the third of these, and the
	// same shape as the class change and the slot expansion above.
	//
	// Phase 22 first implemented this on the *bag* path, which was wrong for
	// this id. The live server handles CXSLItem::CI_CASH_SKILL_NOTE_ITEM inside
	// EGS_GET_PURCHASED_CASH_ITEM_REQ (GSUserCashShop.cpp:1585), not inside the
	// use-item switch: picking it out of the cash deposit is what expands the
	// pages, and the item never becomes an inventory item at all. Claiming it
	// into the bag - which is what this handler used to do - produced exactly
	// what ISSUES.md #12 describes: an item that sits there and cannot be used,
	// because CX2UIInventory::OnRClickedItem has no path for it either.
	//
	// The plain SKILL_NOTE_ITEM_ID (99600) is deliberately NOT handled here.
	// That one really is a bag item on live - CXSLItem::SI_SKILL_NOTE_ITEM in
	// the use-item switch, GSUserInventory.cpp:4610 - and the bag path added in
	// phase 22 covers it. The offline shop does not sell it, so this branch
	// would never see it anyway.
	if( CASH_SKILL_NOTE_ITEM_ID == iItemID )
	{
		KOfflineUnitRow kUnit;
		if( false == pDB->LoadUnit( kSes.m_nSelectedUnitUID, kUnit ) )
			return Reply( kSes, EGS_BILL_GET_PURCHASED_CASH_ITEM_ACK, kAck );

		const char cNewPage = CX2OfflineSkill::ExpandSkillNotePageForLevel( kUnit.m_iLevel );

		if( 0 == cNewPage )
		{
			CX2OfflineLog::Server( L"CASH     skill note refused: it needs level 20-69, this"
				L" character is level %d (the deposit line is left in place)", kUnit.m_iLevel );

			kAck.m_iOK = NetError::ERR_SKILL_NOTE_07;
			return Reply( kSes, EGS_BILL_GET_PURCHASED_CASH_ITEM_ACK, kAck );
		}

		CX2OfflineSkill* pSkill = CX2OfflineSkill::Instance();
		pSkill->Load( kSes.m_nSelectedUnitUID );

		if( cNewPage <= pSkill->GetSkillNoteMaxPage() )
		{
			CX2OfflineLog::Server( L"CASH     skill note refused: it would give %d page(s) and this"
				L" character already has %d (the deposit line is left in place)",
				(int)cNewPage, (int)pSkill->GetSkillNoteMaxPage() );

			kAck.m_iOK = NetError::ERR_SKILL_NOTE_06;
			return Reply( kSes, EGS_BILL_GET_PURCHASED_CASH_ITEM_ACK, kAck );
		}

		// Line out first, same ordering rule as the two branches above.
		if( false == pDB->DeleteCashOrder( kSes.m_nUserUID, pRow->m_nTransNo ) )
		{
			CX2OfflineLog::Server( L"CASH     skill note refused: deposit line %I64d could not be"
				L" removed", pRow->m_nTransNo );

			return Reply( kSes, EGS_BILL_GET_PURCHASED_CASH_ITEM_ACK, kAck );
		}

		pSkill->SetSkillNoteMaxPage( cNewPage );

		kAck.m_iOK = NetError::NET_OK;
		Reply( kSes, EGS_BILL_GET_PURCHASED_CASH_ITEM_ACK, kAck );

		// The ACK carries no page count, so the _NOT is what tells the client.
		// CX2State::Handler_EGS_EXPAND_SKILL_NOTE_PAGE_NUM_NOT is the only
		// caller of SetSkillNoteMaxPage *and* of HideSkillNote( false ), which
		// is what makes the skill-note button appear in the inventory
		// (X2State.cpp:9087-9110).
		KEGS_EXPAND_SKILL_NOTE_PAGE_NUM_NOT kNot;
		kNot.m_cExpandedPageNum = cNewPage;

		Reply( kSes, EGS_EXPAND_SKILL_NOTE_PAGE_NUM_NOT, kNot );

		CX2OfflineLog::Server( L"CASH     claimed line %I64d: item %d gave the skill note %d page(s)",
			pRow->m_nTransNo, iItemID, (int)cNewPage );

		return true;
	}

	if( false == pInven->HasRoomFor( iItemID, iQuantity ) )
	{
		CX2OfflineLog::Server( L"CASH     claim refused: no room for %d x item %d - the deposit"
			L" line is left in place", iQuantity, iItemID );

		kAck.m_iOK = NetError::ERR_BUY_CASH_ITEM_19;
		return Reply( kSes, EGS_BILL_GET_PURCHASED_CASH_ITEM_ACK, kAck );
	}

	// The socket ("attribute") the player picked in the popup, keyed by item ID.
	// The combo box it came from was built from the group MakeOrderInfo put in
	// m_vecSocketOption, so the pick is checked back against that same group -
	// the client decides combat outcomes offline, but a socket outside the
	// item's own group would be a permanent stat the game never offered.
	std::vector< int > vecSocket;

	std::map< int, int >::const_iterator mitSocket = kReq.m_mapSocketForCash.find( iItemID );
	if( kReq.m_mapSocketForCash.end() != mitSocket && 0 != mitSocket->second )
	{
		if( true == CX2OfflineCashShop::IsSocketAllowed( iItemID, mitSocket->second ) )
		{
			vecSocket.push_back( mitSocket->second );
		}
		else
		{
			// Refused rather than dropped: claiming without the socket would
			// hand back an item the player thinks they chose an attribute for,
			// and the deposit line is already gone by then. Bail before the
			// delete, so the line stays claimable.
			CX2OfflineLog::Server( L"CASH     claim refused: socket %d is not in item %d's"
				L" cash-avatar socket group", mitSocket->second, iItemID );

			kAck.m_iOK = NetError::ERR_ITEM_00;
			return Reply( kSes, EGS_BILL_GET_PURCHASED_CASH_ITEM_ACK, kAck );
		}
	}

	// Take the line out before inserting. If DeleteCashOrder comes back false
	// the line was not there after all, and inserting anyway would mint an item
	// out of nothing.
	if( false == pDB->DeleteCashOrder( kSes.m_nUserUID, pRow->m_nTransNo ) )
	{
		CX2OfflineLog::Server( L"CASH     claim refused: deposit line %I64d could not be removed",
			pRow->m_nTransNo );

		return Reply( kSes, EGS_BILL_GET_PURCHASED_CASH_ITEM_ACK, kAck );
	}

	int iInserted = 0;
	pInven->InsertItem( iItemID, iQuantity, 0, kAck.m_vecInventorySlotInfo, iInserted,
						true == vecSocket.empty() ? NULL : &vecSocket );

	CX2OfflineLog::Server( L"CASH     claimed line %I64d: %d x item %d into the bag%s",
		pRow->m_nTransNo, iInserted, iItemID,
		true == vecSocket.empty() ? L"" : L" (with the chosen socket)" );

	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_BILL_GET_PURCHASED_CASH_ITEM_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_GET_PURCHASED_PACKAGE_CASH_ITEM_REQ( KOfflineSession& kSes,
																		const KEvent& kEvent )
{
	KEGS_GET_PURCHASED_PACKAGE_CASH_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// A package is several products sold as one deposit line, and nothing
	// offline makes one: the catalog has a single product per item and
	// MakeOrderInfo always writes m_byteProductKind 0. So this can only be
	// reached by a client that has a package line from somewhere else, and the
	// honest answer is a refusal rather than an item.
	//
	// It is answered rather than dropped because the client arms an
	// AddServerPacket wait on it.
	CX2OfflineLog::Server( L"CASH     package claim refused (order %lu, product %lu) - the offline"
		L" catalog has no packages", kReq.m_ulOrderNo, kReq.m_ulPackageProductNo );

	KEGS_GET_PURCHASED_PACKAGE_CASH_ITEM_ACK kAck;
	kAck.m_iOK						= NetError::ERR_BUY_CASH_ITEM_00;
	kAck.m_iNumResurrectionStone	= 0;
#ifdef SERV_GUILD_CHANGE_NAME
	kAck.m_iExceptionProcessItemID	= 0;
#endif SERV_GUILD_CHANGE_NAME

	return Reply( kSes, EGS_GET_PURCHASED_PACKAGE_CASH_ITEM_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// the rest of the shop's packets

bool CX2OfflineServer::Handler_EGS_PRESENT_CASH_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_PRESENT_CASH_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// There is exactly one account and the gift button is off for every product
	// (MakeProductInfo sets m_bEnableGift false), so this should be
	// unreachable. Answered anyway, because the client waits on the ACK and a
	// silent drop is a five-second stall and a timeout box.
	//
	// ERR_BUY_CASH_ITEM_22 is "no such nickname", which is exactly true: the
	// only character that exists is the sender's own.
	CX2OfflineLog::Server( L"CASH     present refused - '%s' does not exist offline",
		kReq.m_wstrReceiverNickName.c_str() );

	KEGS_PRESENT_CASH_ITEM_ACK kAck;
	kAck.m_iOK			= NetError::ERR_BUY_CASH_ITEM_22;
	kAck.m_bIsWishList	= false;

	return Reply( kSes, EGS_PRESENT_CASH_ITEM_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_VISIT_CASH_SHOP_NOT( KOfflineSession& /*kSes*/, const KEvent& /*kEvent*/ )
{
	// Telemetry: the studio counted shop visits. No reply, nothing waits on it.
	// Listed here rather than left to the UNHANDLED catch-all so the packet log
	// stays signal-only - it fires on every open and every close.
	return true;
}

bool CX2OfflineServer::Handler_EGS_CHECK_PRESENT_CASH_INVENTORY_NOT( KOfflineSession& /*kSes*/,
																	 const KEvent& /*kEvent*/ )
{
	// "does the recipient have room for this gift?" - a push with no reply the
	// client waits on. There are no gifts offline (see
	// Handler_EGS_PRESENT_CASH_ITEM_REQ), so there is nothing to check.
	return true;
}

bool CX2OfflineServer::Handler_EGS_APPLY_COUPON_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_APPLY_COUPON_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// A coupon serial is validated against the publisher's billing service.
	// There is no such service offline and no table of issued serials to check
	// against, so every serial is invalid - which is what an unissued serial
	// would get on live too.
	CX2OfflineLog::Server( L"CASH     coupon '%s' refused - there is no coupon service offline",
		kReq.m_wstrCouponSerial.c_str() );

	KEGS_APPLY_COUPON_ACK kAck;
	kAck.m_iOK = NetError::ERR_BUY_CASH_ITEM_00;

	return Reply( kSes, EGS_APPLY_COUPON_ACK, kAck );
}

#endif SERV_IRUHADEV_OFFLINE
