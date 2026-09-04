#include "stdafx.h"
#include "X2OfflineCashShop.h"

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineLog.h"
#include "X2OfflineInventory.h"
#include "X2CashShop.h"
#include "X2SocketItem.h"

CX2OfflineCashShop* CX2OfflineCashShop::ms_pInstance = NULL;

namespace
{
}

//////////////////////////////////////////////////////////////////////////

CX2OfflineCashShop::CX2OfflineCashShop()
: m_bLoadAttempted( false )
, m_iRows( 0 )
, m_iDroppedRows( 0 )
{
}

CX2OfflineCashShop::~CX2OfflineCashShop()
{
}

CX2OfflineCashShop* CX2OfflineCashShop::Instance()
{
	if( NULL == ms_pInstance )
		ms_pInstance = new CX2OfflineCashShop;

	return ms_pInstance;
}

void CX2OfflineCashShop::Release()
{
	if( NULL == ms_pInstance )
		return;

	delete ms_pInstance;
	ms_pInstance = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Loading

void CX2OfflineCashShop::EnsureLoaded()
{
	if( true == m_bLoadAttempted )
		return;

	m_bLoadAttempted = true;

	std::vector< KOfflineCashProductRow > vecRow;
	if( false == CX2OfflineDB::Instance()->LoadCashProducts( vecRow ) )
	{
		CX2OfflineLog::Server( L"CASH     ERROR could not read cash_product - the shop will be empty." );
		return;
	}

	m_iRows = (int)vecRow.size();

	if( NULL == g_pData || NULL == g_pData->GetItemManager() )
	{
		CX2OfflineLog::Server( L"CASH     ERROR item manager not ready - the shop will be empty." );
		return;
	}

	for( size_t i = 0; i < vecRow.size(); ++i )
	{
		const KOfflineCashProductRow& kRow = vecRow[i];

		// A product naming an item this build has no templet for is dropped.
		// The client filters the shop through GetItemTemplet anyway
		// (CX2CashShop::GetItemByCategory), so carrying it would only bloat the
		// catalog packet - and a .kom a few items behind the billing table is
		// the normal case, not an error. Only the count is logged.
		//
		// NOT through CX2OfflineInventory::Templet: that one logs a warning per
		// miss, which is right for a save file naming a missing item and wrong
		// for a bulk load.
		if( NULL == g_pData->GetItemManager()->GetItemTemplet( kRow.m_iItemID ) )
		{
			++m_iDroppedRows;
			continue;
		}

		KProduct kProduct;
		kProduct.m_iProductNo	= kRow.m_iProductNo;
		kProduct.m_iItemID		= kRow.m_iItemID;
		kProduct.m_iPrice		= kRow.m_iPrice;
		kProduct.m_iCategoryNo	= kRow.m_iCategoryNo;
		kProduct.m_iQuantity	= kRow.m_iQuantity > 0 ? kRow.m_iQuantity : 1;
		kProduct.m_iPeriod		= 0;
		kProduct.m_bEvent		= kRow.m_bEvent;

		// Keyed by product number, never by item ID. Eighteen items are sold as
		// two products each, and the client expects exactly that - it keeps a
		// vector of KBillProductInfo per item and picks [0] as the headline
		// option. Deduplicating here would silently drop the second.
		m_mapProductIndex[ kRow.m_iProductNo ] = (int)m_vecProduct.size();
		m_vecProduct.push_back( kProduct );
	}

	CX2OfflineLog::Server( L"CASH     catalog: %d product(s) from %d cash_product row(s);"
		L" %d dropped for having no item templet",
		(int)m_vecProduct.size(), m_iRows, m_iDroppedRows );

	if( true == m_vecProduct.empty() )
	{
		CX2OfflineLog::Server(
			L"CASH     WARNING the cash shop has nothing in it. cash_product is seeded by the"
			L" schema v9 migration - check offline_server.log for a DB line about it." );
	}
}

//////////////////////////////////////////////////////////////////////////
// Reads

int CX2OfflineCashShop::PageCount() const
{
	const int iCount = (int)m_vecProduct.size();
	if( 0 >= iCount )
		return 1;

	return ( iCount + (int)PRODUCTS_PER_PAGE - 1 ) / (int)PRODUCTS_PER_PAGE;
}

void CX2OfflineCashShop::GetPage( int iPage, OUT std::vector< KBillProductInfo >& vecOut,
								  OUT bool& bFinal ) const
{
	vecOut.clear();

	const int iPages = PageCount();

	if( iPage < 1 )
		iPage = 1;

	bFinal = ( iPage >= iPages );

	const int iFirst = ( iPage - 1 ) * (int)PRODUCTS_PER_PAGE;

	for( int i = iFirst; i < iFirst + (int)PRODUCTS_PER_PAGE && i < (int)m_vecProduct.size(); ++i )
	{
		KBillProductInfo kInfo;
		MakeProductInfo( m_vecProduct[i], kInfo );
		vecOut.push_back( kInfo );
	}
}

const CX2OfflineCashShop::KProduct* CX2OfflineCashShop::Find( int iProductNo ) const
{
	std::map< int, int >::const_iterator mit = m_mapProductIndex.find( iProductNo );
	if( m_mapProductIndex.end() == mit )
		return NULL;

	if( mit->second < 0 || mit->second >= (int)m_vecProduct.size() )
		return NULL;

	return &m_vecProduct[ mit->second ];
}

/*static*/ void CX2OfflineCashShop::MakeProductInfo( const KProduct& kProduct,
													 OUT KBillProductInfo& kOut )
{
	kOut.m_iProductNo		= kProduct.m_iProductNo;
	kOut.m_iProductID		= kProduct.m_iItemID;
	kOut.m_bSale			= false;
	kOut.m_bEvent			= kProduct.m_bEvent;	///< DI_ISEVENT; 60 of the 2360 rows
	kOut.m_bShow			= true;

	// "skip the deposit and go straight into the bag". Offline every purchase
	// lands in the deposit, because that is the flow the client's two packets
	// describe and the deposit is the only thing EGS_BILL_INVENTORY_INQUIRY_ACK
	// has to say.
	kOut.m_bIsCashInvenSkip	= false;

	kOut.m_wstrProductName	= L"";		///< only read for packages and coupons
	kOut.m_cCategoryNo		= (char)kProduct.m_iCategoryNo;
	kOut.m_iSalePrice		= kProduct.m_iPrice;
	kOut.m_iRealPrice		= kProduct.m_iPrice;
	kOut.m_cPeriod			= (char)kProduct.m_iPeriod;
	kOut.m_cQuantity		= (char)kProduct.m_iQuantity;

	// There is nobody to give a present to, so the gift button stays off rather
	// than opening a nickname box that can only ever fail. The refusal in
	// Handler_EGS_PRESENT_CASH_ITEM_REQ is the backstop for a client that gets
	// there another way.
	kOut.m_bEnableGift		= false;

	kOut.m_cLimitLevel		= 0;
	kOut.m_wstrEndDate		= L"";
}

/*static*/ void CX2OfflineCashShop::MakeOrderInfo( const KOfflineCashOrderRow& kRow,
												   OUT KBillOrderInfo& kOut )
{
	const CX2Item::ItemTemplet* pTemplet = CX2OfflineInventory::Templet( kRow.m_iItemID );

	kOut.m_iTransNo		= (UidType)kRow.m_nTransNo;
	kOut.m_wstrOrderID	= L"";
	kOut.m_byteType		= (byte)KBillOrderInfo::BCIT_BUY_ITEM;

	kOut.m_wstrSenderNickName		= L"";
	kOut.m_wstrSenderPresentMessage	= L"";
	kOut.m_iFromUnitUID				= 0;

#ifdef SERV_GLOBAL_CASH_PACKAGE
	kOut.m_byteProductKind = 0;		///< 1 would mean "package"; nothing offline makes one
#endif SERV_GLOBAL_CASH_PACKAGE

	// The socket ("attribute") the player picks in the send-to-inventory popup.
	//
	// This is NOT a list of socket IDs - it is a list of socket *group* IDs, and
	// both halves of the client agree on that: CashItemToInvenPopupSlot::
	// AddSocketIdListToComboBox feeds each entry to
	// CX2SocketItem::GetSocketIdListForCashAvatar to build the combo box, and
	// GetSelectedOptionList feeds m_vecSocketOption[0] to the same function to
	// turn the combo index back into a socket ID. Everything the group expands
	// into is client-side data.
	//
	// Leaving it empty - which is what phase 7 shipped first - draws no combo
	// box at all, and then the claim is refused with "select the item socket
	// option": CashItemToInvenPopup::GetSelectedOptionListAndGroupID returns
	// false for any item GetIsPossibleSocketItemByOnlyItemType says can hold a
	// socket, unless a selection was made ([X2CashShop.cpp:10626](X2Lib/X2CashShop.cpp#L10626)).
	// So a costume could be bought and never claimed.
	//
	// On live the group comes from the billing DB, per product. There is no such
	// row here, so this uses the group the client itself falls back to when it
	// is handed a group ID it does not know: 2000 for a weapon, 1000 for
	// defence ([X2SocketItem.cpp:2214-2231](X2Lib/X2SocketItem.cpp#L2214),
	// under SERV_GLOBAL_BILLING). That is the client's own documented behaviour
	// for an unknown group, not a number invented here - and because it is also
	// the fallback, passing it produces the same option list whether or not the
	// group happens to be in the loaded table.
	//
	// Anything that cannot hold a socket keeps an empty list, which hides the
	// combo box and makes the claim path skip the check entirely.
	kOut.m_vecSocketOption.clear();

	if( NULL != pTemplet )
	{
		switch( pTemplet->GetItemType() )
		{
		case CX2Item::IT_WEAPON:	kOut.m_vecSocketOption.push_back( CASH_SOCKET_GROUP_WEAPON );	break;
		case CX2Item::IT_DEFENCE:	kOut.m_vecSocketOption.push_back( CASH_SOCKET_GROUP_DEFENCE );	break;
		default:					break;
		}
	}

	// The deposit line carries its own copy of the product it was bought as,
	// rather than a look-up into the live catalog. That is what KBillOrderInfo
	// does on live too, and it is what lets a line bought before a price change
	// still describe itself - the claim path reads m_kBillProductInfo, never
	// the catalog.
	KProduct kProduct;
	kProduct.m_iProductNo	= kRow.m_iProductNo;
	kProduct.m_iItemID		= kRow.m_iItemID;
	kProduct.m_iPrice		= kRow.m_iPrice;
	kProduct.m_iQuantity	= kRow.m_iQuantity;
	kProduct.m_iPeriod		= kRow.m_iPeriod;
	kProduct.m_iCategoryNo	= CategoryFor( pTemplet );

	MakeProductInfo( kProduct, kOut.m_kBillProductInfo );
}

/*static*/ bool CX2OfflineCashShop::IsSocketAllowed( int iItemID, int iSocketID )
{
	if( 0 == iSocketID )
		return true;			///< "none" is always a legal pick

	if( NULL == g_pData || NULL == g_pData->GetSocketItem() )
		return false;

	const CX2Item::ItemTemplet* pTemplet = CX2OfflineInventory::Templet( iItemID );
	if( NULL == pTemplet )
		return false;

	int iGroupID = 0;
	switch( pTemplet->GetItemType() )
	{
	case CX2Item::IT_WEAPON:	iGroupID = CASH_SOCKET_GROUP_WEAPON;	break;
	case CX2Item::IT_DEFENCE:	iGroupID = CASH_SOCKET_GROUP_DEFENCE;	break;
	default:					return false;		///< cannot hold one at all
	}

	std::vector< int > vecSocketID;
	g_pData->GetSocketItem()->GetSocketIdListForCashAvatar(
		iGroupID, vecSocketID, (int)pTemplet->GetItemType() );

	for( size_t i = 0; i < vecSocketID.size(); ++i )
	{
		if( vecSocketID[i] == iSocketID )
			return true;
	}

	return false;
}

/*static*/ int CX2OfflineCashShop::CategoryFor( const CX2Item::ItemTemplet* pTemplet )
{
	// FALLBACK ONLY. The catalog carries the real billing category, and this is
	// reached solely by MakeOrderInfo for a deposit line whose product has since
	// gone from cash_product - a save edited between sessions, essentially.
	//
	// It returns a CSSC_* enum value, which is NOT the number space the wire
	// wants (see the file header), so an item routed through here lands in no
	// tab. That is the honest outcome for a product that no longer exists: the
	// deposit line still claims, it just cannot be browsed to.
	if( NULL == pTemplet )
		return (int)CX2CashShop::CSSC_CONSUME_ETC;

	const CX2Unit::EQIP_POSITION eSlot = pTemplet->GetEqipPosition();

	// A fashion piece is any item flagged m_bFashion, whatever its item type -
	// a fashion weapon is still IT_WEAPON. That flag is the one the client
	// itself uses to decide an avatar from real gear.
	if( true == pTemplet->GetFashion() )
	{
		switch( eSlot )
		{
		case CX2Unit::EP_WEAPON_HAND:	return (int)CX2CashShop::CSSC_FASHION_WEAPON;
		case CX2Unit::EP_DEFENCE_HAIR:	return (int)CX2CashShop::CSSC_FASHION_HAIR;
		case CX2Unit::EP_DEFENCE_BODY:	return (int)CX2CashShop::CSSC_FASHION_BODY;
		case CX2Unit::EP_DEFENCE_LEG:	return (int)CX2CashShop::CSSC_FASHION_LEG;
		case CX2Unit::EP_DEFENCE_HAND:	return (int)CX2CashShop::CSSC_FASHION_HAND;
		case CX2Unit::EP_DEFENCE_FOOT:	return (int)CX2CashShop::CSSC_FASHION_FOOT;
		default:						return (int)CX2CashShop::CSSC_FASHION_BODY;
		}
	}

	switch( pTemplet->GetItemType() )
	{
	case CX2Item::IT_ACCESSORY:
		switch( eSlot )
		{
		case CX2Unit::EP_AC_HAIR:
		case CX2Unit::EP_AC_FACE1:
		case CX2Unit::EP_AC_FACE2:
		case CX2Unit::EP_AC_FACE3:		return (int)CX2CashShop::CSSC_ACCESSORY_FACE;
		case CX2Unit::EP_AC_BODY:		return (int)CX2CashShop::CSSC_ACCESSORY_BODY;
		case CX2Unit::EP_AC_LEG:		return (int)CX2CashShop::CSSC_ACCESSORY_LEG;
		case CX2Unit::EP_AC_ARM:		return (int)CX2CashShop::CSSC_ACCESSORY_ARM;
		case CX2Unit::EP_AC_RING:		return (int)CX2CashShop::CSSC_ACCESSORY_RING;
		case CX2Unit::EP_AC_NECKLESS:	return (int)CX2CashShop::CSSC_ACCESSORY_NECKLACE;
		case CX2Unit::EP_AC_WEAPON:		return (int)CX2CashShop::CSSC_ACCESSORY_WEAPON;
		default:						return (int)CX2CashShop::CSSC_ACCESSORY_BODY;
		}

	case CX2Item::IT_QICK_SLOT:
		// The quick-slot consumables: potions and the like. CSSC_CONSUME_RESTORE
		// is the tab the shop calls them.
		return (int)CX2CashShop::CSSC_CONSUME_RESTORE;

	case CX2Item::IT_OUTLAY:
	case CX2Item::IT_MATERIAL:
	case CX2Item::IT_ETC:
	case CX2Item::IT_SKILL_MEMO:
		return (int)CX2CashShop::CSSC_CONSUME_ETC;

	case CX2Item::IT_WEAPON:
	case CX2Item::IT_DEFENCE:
		// Real gear sold for cash. There is no gear tab in the shop's own
		// category list, and the closest honest home is the etc tab rather than
		// the fashion one - putting a real weapon under "fashion weapon" would
		// be the mapping lying about what the item is.
		return (int)CX2CashShop::CSSC_CONSUME_ETC;

	case CX2Item::IT_SPECIAL:
	default:
		// Slot expansions, packages, name changes, class-change tickets - the
		// "install" tab is where the shop puts everything that changes the
		// account rather than the character's gear.
		return (int)CX2CashShop::CSSC_INSTALL_ETC;
	}
}

#endif SERV_IRUHADEV_OFFLINE
