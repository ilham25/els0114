#include "stdafx.h"
#include "X2OfflineServer.h"

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-02
// Description: Offline mode - phase 5. The inventory packets: moving and
//              swapping items, equipping and unequipping, discarding, sorting,
//              using a consumable, the quick-slot bar, and the ED shop.
//
//              The rules all live in CX2OfflineInventory; this file is the
//              packet layer over it. The reference for each reply is the real
//              handler in KncWX2Server/GameServer/GSUserInventory.cpp plus the
//              client's own ACK handler - and for the inventory the client's
//              side is the one that matters, because every one of these ACKs
//              feeds CX2UIInventory::UpdateInventorySlotList, whose contract is
//              "one entry per touched slot, m_iItemUID 0 meaning the slot is
//              now empty" (X2UIInventory.cpp:3891-3947).
//
//              Two packets are answered with a refusal rather than left
//              unhandled, because the client waits on them and a silent drop
//              turns into a five-second timeout and a modal error box:
//              EGS_ENCHANT_ITEM_REQ and EGS_SOCKET_ITEM_REQ. Enhancement and
//              socketing need the enchant tables, which are server data with no
//              client copy, so they are off and say so.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineInventory.h"

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_CHANGE_INVENTORY_SLOT_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CHANGE_INVENTORY_SLOT_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineInventory* pInven = CX2OfflineInventory::Instance();

	KEGS_CHANGE_INVENTORY_SLOT_ITEM_ACK kAck;

	// The server's failure reply is the default-constructed ACK: m_iOK is 0,
	// which is NET_OK, with an empty slot list. That reads as "nothing changed"
	// rather than as an error, and the client redraws from its own state - which
	// is what makes an illegal drag snap back instead of popping a dialog.
	kAck.m_iOK = NetError::NET_OK;

	const bool bOK = pInven->MoveItem( (int)kReq.m_cFromSlotType, kReq.m_iFromSlotID,
									   (int)kReq.m_cToSlotType,   kReq.m_iToSlotID,
									   kAck.m_vecInventorySlotInfo );

	if( false == bOK )
	{
		CX2OfflineLog::Server( L"ITEM     move refused: %d/%d -> %d/%d",
			(int)kReq.m_cFromSlotType, kReq.m_iFromSlotID,
			(int)kReq.m_cToSlotType, kReq.m_iToSlotID );

		kAck.m_vecInventorySlotInfo.clear();
		return Reply( kSes, EGS_CHANGE_INVENTORY_SLOT_ITEM_ACK, kAck );
	}

	CX2OfflineLog::Server( L"ITEM     move %d/%d -> %d/%d, %u slot(s) changed",
		(int)kReq.m_cFromSlotType, kReq.m_iFromSlotID,
		(int)kReq.m_cToSlotType, kReq.m_iToSlotID,
		(unsigned int)kAck.m_vecInventorySlotInfo.size() );

	Reply( kSes, EGS_CHANGE_INVENTORY_SLOT_ITEM_ACK, kAck );

	// Equipment changed, so the character model has to change with it. On the
	// real server this is SendUpdateChangeEquippedItem broadcasting to whoever
	// can see the player; offline there is nobody else, but the player's own
	// viewer is redrawn from the same push - the field one in a village, the
	// room one inside a dungeon. Sending the wrong one for the state is
	// harmless (the handler finds no unit and returns), sending neither leaves
	// the old gear on screen until the next state change.
	if( CX2Inventory::ST_E_EQUIP == (int)kReq.m_cFromSlotType ||
		CX2Inventory::ST_E_EQUIP == (int)kReq.m_cToSlotType )
	{
		PushEquipChanged( kSes, kAck.m_vecInventorySlotInfo );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

void CX2OfflineServer::PushEquipChanged( KOfflineSession& kSes,
										 const std::vector< KInventoryItemInfo >& vecChanged )
{
	if( 0 == kSes.m_nSelectedUnitUID )
		return;

	if( S_ROOM == kSes.m_eState )
	{
		KEGS_CHANGE_EQUIPPED_ITEM_IN_ROOM_NOT kNot;
		kNot.m_cChangeEquippedPacketType	= KEGS_CHANGE_EQUIPPED_ITEM_IN_ROOM_NOT::CEPT_CHANGE_EQUIP;
		kNot.m_UnitUID						= kSes.m_nSelectedUnitUID;
		kNot.m_vecInventorySlotInfo			= vecChanged;
		kNot.m_iEquippedTitleID				= 0;

		// m_kGameStat is not decoration here, for the same reason phase 3 found
		// in the village and phase 4 found in the room slot list: the client
		// assigns it straight over the unit's game stat, so a zeroed one would
		// give the character max HP 0 the moment it changed a hat.
		KOfflineUnitRow kRow;
		if( true == CX2OfflineDB::Instance()->LoadUnit( kSes.m_nSelectedUnitUID, kRow ) )
			MakeGameStat( kRow, kNot.m_kGameStat );

		Reply( kSes, EGS_CHANGE_EQUIPPED_ITEM_IN_ROOM_NOT, kNot );
		return;
	}

	KEGS_CHANGE_EQUIPPED_ITEM_IN_FIELD_NOT kNot;
	kNot.m_iUnitUID				= kSes.m_nSelectedUnitUID;
	kNot.m_vecInventorySlotInfo	= vecChanged;
	kNot.m_iEquippedTitleID		= 0;

	Reply( kSes, EGS_CHANGE_EQUIPPED_ITEM_IN_FIELD_NOT, kNot );
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_DELETE_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_DELETE_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineInventory* pInven = CX2OfflineInventory::Instance();

	KEGS_DELETE_ITEM_ACK kAck;
	kAck.m_iOK = NetError::ERR_ITEM_04;			///< no such item

	KOfflineItemRow kRow;
	if( false == pInven->GetItemRow( kReq.m_iItemUID, kRow ) )
		return Reply( kSes, EGS_DELETE_ITEM_ACK, kAck );

	// An equipped item cannot be thrown away while it is worn. The client's own
	// UI blocks it, so this is the belt-and-braces half.
	if( CX2Inventory::ST_E_EQUIP == kRow.m_iCategory )
	{
		kAck.m_iOK = NetError::ERR_ITEM_05;
		return Reply( kSes, EGS_DELETE_ITEM_ACK, kAck );
	}

	if( false == pInven->DeleteItem( kReq.m_iItemUID, kReq.m_iQuantity, kAck.m_kInventoryItemInfo ) )
		return Reply( kSes, EGS_DELETE_ITEM_ACK, kAck );

	kAck.m_iOK = NetError::NET_OK;

	CX2OfflineLog::Server( L"ITEM     discarded %d x item %d (itemUID=%I64d)",
		( kReq.m_iQuantity > 0 ) ? kReq.m_iQuantity : kRow.m_iQuantity,
		kRow.m_iItemID, (__int64)kReq.m_iItemUID );

	return Reply( kSes, EGS_DELETE_ITEM_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_SORT_CATEGORY_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_SORT_CATEGORY_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_SORT_CATEGORY_ITEM_ACK kAck;
	kAck.m_iCategoryType	= kReq.m_iCategoryType;
	kAck.m_iOK				= NetError::NET_OK;

	if( false == CX2OfflineInventory::Instance()->SortCategory( kReq.m_iCategoryType,
			kAck.m_vecUpdatedInventorySlot ) )
	{
		kAck.m_iOK = NetError::ERR_ITEM_04;
		kAck.m_vecUpdatedInventorySlot.clear();
	}
	else
	{
		CX2OfflineLog::Server( L"ITEM     sorted category %d", kReq.m_iCategoryType );
	}

	return Reply( kSes, EGS_SORT_CATEGORY_ITEM_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_USE_ITEM_IN_INVENTORY_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_USE_ITEM_IN_INVENTORY_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineInventory* pInven = CX2OfflineInventory::Instance();

	KEGS_USE_ITEM_IN_INVENTORY_ACK kAck;
	kAck.m_iOK				= NetError::ERR_ITEM_04;
	kAck.m_iUsedItemID		= 0;
	kAck.m_iWarpPointMapID	= 0;
	kAck.m_iED				= 0;
	kAck.m_iTempCode		= kReq.m_iTempCode;

	KOfflineItemRow kRow;
	if( false == pInven->GetItemRow( kReq.m_iItemUID, kRow ) )
		return Reply( kSes, EGS_USE_ITEM_IN_INVENTORY_ACK, kAck );

	const CX2Item::ItemTemplet* pTemplet = CX2OfflineInventory::Templet( kRow.m_iItemID );

	// Only the items the client itself marks as usable from the bag. Anything
	// else - a warp ticket, a cube, a package - has a server-side effect this
	// phase does not implement, and consuming it to do nothing would destroy
	// the item. Refused with the item intact.
	if( NULL == pTemplet || false == pTemplet->GetCanUseInventory() )
	{
		kAck.m_iOK = NetError::ERR_ITEM_11;

		CX2OfflineLog::Server( L"ITEM     refused use of item %d from the bag - its effect is not"
			L" implemented offline, so the item is left alone", kRow.m_iItemID );

		return Reply( kSes, EGS_USE_ITEM_IN_INVENTORY_ACK, kAck );
	}

	KInventoryItemInfo kSlotInfo;
	if( false == pInven->ConsumeOne( kReq.m_iItemUID, kSlotInfo ) )
		return Reply( kSes, EGS_USE_ITEM_IN_INVENTORY_ACK, kAck );

	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_iUsedItemID	= kRow.m_iItemID;
	kAck.m_vecKInventorySlotInfo.push_back( kSlotInfo );

	CX2OfflineLog::Server( L"ITEM     used item %d from the bag", kRow.m_iItemID );

	Reply( kSes, EGS_USE_ITEM_IN_INVENTORY_ACK, kAck );

	// Phase 6: TMCT_USE_ITEM title missions, and the collection steps the item
	// leaving the bag may have just broken or completed.
	{
		KOfflineUnitRow kUnitRow;
		if( true == LoadQuestState( kSes, kUnitRow ) )
		{
			std::vector< KMissionInstance >	vecNewMission;
			std::vector< KMissionInstance >	vecMissionChanged;
			std::vector< KTitleInfo >		vecNewTitle;

			CX2OfflineTitle::Instance()->OnUseItem( kRow.m_iItemID, kUnitRow,
													vecMissionChanged, vecNewTitle );

			PushMissionUpdate( kSes, vecNewMission, vecMissionChanged, vecNewTitle );
		}
	}

	// And the SQT_ITEM_USE quest steps - a different system from the title
	// missions above, listening to the same moment.
	QuestOnUseItem( kSes, kRow.m_iItemID );

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_USE_QUICK_SLOT_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_USE_QUICK_SLOT_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineInventory* pInven = CX2OfflineInventory::Instance();

	KEGS_USE_QUICK_SLOT_ACK kAck;
	kAck.m_iOK		= NetError::ERR_ITEM_04;
	kAck.m_ItemID	= -1;

	const UidType nItemUID = pInven->GetItemUID( CX2Inventory::ST_E_QUICK_SLOT, (int)kReq.m_sSlotID );

	KOfflineItemRow kRow;
	if( 0 == nItemUID || false == pInven->GetItemRow( nItemUID, kRow ) )
		return Reply( kSes, EGS_USE_QUICK_SLOT_ACK, kAck );

	if( false == pInven->ConsumeOne( nItemUID, kAck.m_InventorySlotInfo ) )
		return Reply( kSes, EGS_USE_QUICK_SLOT_ACK, kAck );

	kAck.m_iOK		= NetError::NET_OK;
	kAck.m_ItemID	= kRow.m_iItemID;

	Reply( kSes, EGS_USE_QUICK_SLOT_ACK, kAck );

	CX2OfflineLog::Server( L"ITEM     quick slot %d used item %d",
		(int)kReq.m_sSlotID, kRow.m_iItemID );

	// The server counts a quick-slot use towards SQT_ITEM_USE exactly as it
	// counts one from the bag - two call sites, one handler
	// (GSUserInventory.cpp:1308 and :4558).
	QuestOnUseItem( kSes, kRow.m_iItemID );

	// The _NOT is what actually applies the item - the ACK only updates the bag
	// and restarts the cooldown. CX2UIQuickSlot::Handler_EGS_USE_QUICK_SLOT_NOT
	// is the sole caller of UseItemSpecialAbility and ApplyBuffFactorToGUUser
	// (X2UIQuickSlot.cpp:1032-1036), so without it a potion is consumed and
	// heals nothing. Phase 4's correction 6, a fourth time.
	//
	// Only inside a room, though: that handler's default branch is an
	// ASSERT( !L"Can not use this item in this State" ), so sending it in a
	// village would trip a debug assert for no gain - there is no CX2Game to
	// apply the effect to there anyway.
	if( S_ROOM == kSes.m_eState )
	{
		KEGS_USE_QUICK_SLOT_NOT kNot;
		kNot.m_UnitUID				= kSes.m_nSelectedUnitUID;
		kNot.m_ItemID				= kRow.m_iItemID;
		kNot.m_bUseItemNeedPayment	= false;

		Reply( kSes, EGS_USE_QUICK_SLOT_NOT, kNot );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_BUY_ED_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_BUY_ED_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineInventory* pInven	= CX2OfflineInventory::Instance();
	CX2OfflineDB*		 pDB	= CX2OfflineDB::Instance();

	KEGS_BUY_ED_ITEM_ACK kAck;
	kAck.m_iOK		= NetError::ERR_BUY_ED_ITEM_05;
	kAck.m_iED		= 0;
	kAck.m_iAPoint	= 0;

	KOfflineUnitRow kUnit;
	if( false == pDB->LoadUnit( kSes.m_nSelectedUnitUID, kUnit ) )
		return Reply( kSes, EGS_BUY_ED_ITEM_ACK, kAck );

	kAck.m_iED = kUnit.m_iED;

	// KInventory::PrepareBuy (Inventory.cpp:5618), minus the PvP-point half -
	// there is no PvP offline, so an item priced in arena points is simply not
	// buyable and its ED price is what gets checked.
	int iTotalED = 0;

	std::map< int, int >::const_iterator mit;
	for( mit = kReq.m_mapItem.begin(); mit != kReq.m_mapItem.end(); ++mit )
	{
		const int iItemID	= mit->first;
		const int iQuantity	= mit->second;

		if( iItemID <= 0 || iQuantity <= 0 )
			return Reply( kSes, EGS_BUY_ED_ITEM_ACK, kAck );

		const CX2Item::ItemTemplet* pTemplet = CX2OfflineInventory::Templet( iItemID );
		if( NULL == pTemplet )
			return Reply( kSes, EGS_BUY_ED_ITEM_ACK, kAck );

		// The server checks IsNpcSellEDItem, its own list of what the shops
		// stock. The client has the same list - CX2ItemManager loads it from
		// AddShopItemList_LUA - but no accessor that answers "is this item for
		// sale anywhere"; what it has is per-map shop lists, and the request
		// does not say which shop. So the check here is the price instead: an
		// item with no ED price is not something a shop sells for ED.
		if( pTemplet->GetPrice() <= 0 )
		{
			CX2OfflineLog::Server( L"SHOP     refused item %d - it has no ED price", iItemID );
			return Reply( kSes, EGS_BUY_ED_ITEM_ACK, kAck );
		}

		if( pTemplet->GetPricePvPPoint() > 0 )
		{
			kAck.m_iOK = NetError::ERR_BUY_ED_ITEM_07;

			CX2OfflineLog::Server( L"SHOP     refused item %d - it costs arena points, and there is"
				L" no PvP offline", iItemID );

			return Reply( kSes, EGS_BUY_ED_ITEM_ACK, kAck );
		}

		iTotalED += pTemplet->GetPrice() * iQuantity;

		if( false == pInven->HasRoomFor( iItemID, iQuantity ) )
		{
			CX2OfflineLog::Server( L"SHOP     refused: no room for %d x item %d", iQuantity, iItemID );
			return Reply( kSes, EGS_BUY_ED_ITEM_ACK, kAck );
		}
	}

	if( iTotalED < 0 || kUnit.m_iED < iTotalED )
	{
		CX2OfflineLog::Server( L"SHOP     refused: %d ED needed, %d held", iTotalED, kUnit.m_iED );
		return Reply( kSes, EGS_BUY_ED_ITEM_ACK, kAck );
	}

	// Charge first, then insert. HasRoomFor was checked per item above, so the
	// inserts cannot come up short - and if one somehow did, the log line in
	// InsertItem says so rather than the ED disappearing silently.
	for( mit = kReq.m_mapItem.begin(); mit != kReq.m_mapItem.end(); ++mit )
	{
		std::vector< KInventoryItemInfo > vecChanged;
		int iInserted = 0;

		pInven->InsertItem( mit->first, mit->second, 0, vecChanged, iInserted );

		for( size_t i = 0; i < vecChanged.size(); ++i )
			kAck.m_vecInventorySlotInfo.push_back( vecChanged[i] );
	}

	kUnit.m_iED -= iTotalED;

	pDB->SaveProgress( kUnit.m_nUnitUID, kUnit.m_iLevel, kUnit.m_iEXP, kUnit.m_iED );

	kAck.m_iOK	= NetError::NET_OK;
	kAck.m_iED	= kUnit.m_iED;

	CX2OfflineLog::Server( L"SHOP     bought %u kind(s) of item for %d ED, %d ED left",
		(unsigned int)kReq.m_mapItem.size(), iTotalED, kUnit.m_iED );

	Reply( kSes, EGS_BUY_ED_ITEM_ACK, kAck );

	// Phase 6: a collection quest can be satisfied by buying the item, and a
	// title mission with a collection step has no other moment at which it is
	// noticed. The quest half needs nothing - both the client and the completion
	// check count the bag directly.
	QuestOnInventoryChanged( kSes );

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_SELL_ED_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_SELL_ED_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineInventory* pInven	= CX2OfflineInventory::Instance();
	CX2OfflineDB*		 pDB	= CX2OfflineDB::Instance();

	KEGS_SELL_ED_ITEM_ACK kAck;
	kAck.m_iOK	= NetError::ERR_SELL_ED_ITEM_05;		///< cannot be sold back
	kAck.m_iED	= 0;

	KOfflineUnitRow kUnit;
	if( false == pDB->LoadUnit( kSes.m_nSelectedUnitUID, kUnit ) )
		return Reply( kSes, EGS_SELL_ED_ITEM_ACK, kAck );

	kAck.m_iED = kUnit.m_iED;

	KOfflineItemRow kRow;
	if( false == pInven->GetItemRow( kReq.m_iItemUID, kRow ) )
		return Reply( kSes, EGS_SELL_ED_ITEM_ACK, kAck );

	if( CX2Inventory::ST_E_EQUIP == kRow.m_iCategory )
		return Reply( kSes, EGS_SELL_ED_ITEM_ACK, kAck );

	const CX2Item::ItemTemplet* pTemplet = CX2OfflineInventory::Templet( kRow.m_iItemID );
	if( NULL == pTemplet )
		return Reply( kSes, EGS_SELL_ED_ITEM_ACK, kAck );

	// A bound item cannot be sold - CX2Item's own m_bVested. The client greys
	// the sell button out for it, so this is the second half of that.
	if( true == pTemplet->GetVested() )
	{
		CX2OfflineLog::Server( L"SHOP     refused sale of item %d - it is bound to the character",
			kRow.m_iItemID );

		return Reply( kSes, EGS_SELL_ED_ITEM_ACK, kAck );
	}

	int iQuantity = kReq.m_iQuantity;
	if( iQuantity <= 0 || iQuantity > kRow.m_iQuantity )
		iQuantity = kRow.m_iQuantity;

	// KInventory::GetSellPrice (Inventory.cpp:14932), all three arms. The
	// fifth of the shop price is the studio's own constant, and the endurance
	// arm is the SERV_SELL_ED_ITEM_PRICE_FIX one - that flag is defined in this
	// build, checked rather than assumed, so a worn item sells for its
	// remaining fraction rather than having a repair bill deducted.
	int iPrice = 0;

	switch( pTemplet->GetPeriodType() )
	{
	case CX2Item::PT_INFINITY:
		iPrice = (int)floorf( 0.2f * (float)pTemplet->GetPrice() + 0.01f );
		break;

	case CX2Item::PT_ENDURANCE:
		{
			if( pTemplet->GetEndurance() <= 0 || kRow.m_iEndurance < 0 )
				return Reply( kSes, EGS_SELL_ED_ITEM_ACK, kAck );

			const float fRate = (float)kRow.m_iEndurance / (float)pTemplet->GetEndurance();

			iPrice = (int)floorf( (float)pTemplet->GetPrice() * 0.2f * fRate + 0.01f );
		}
		break;

	case CX2Item::PT_QUANTITY:
		if( pTemplet->GetQuantity() <= 0 )
			return Reply( kSes, EGS_SELL_ED_ITEM_ACK, kAck );

		iPrice = (int)floorf( 0.2f * (float)pTemplet->GetPrice() * (float)iQuantity + 0.01f );
		break;

	default:
		return Reply( kSes, EGS_SELL_ED_ITEM_ACK, kAck );
	}

	if( iPrice < 0 )
		iPrice = 0;			///< the server's own correction for a negative result

	if( false == pInven->DeleteItem( kReq.m_iItemUID, iQuantity, kAck.m_kInventorySlotInfo ) )
		return Reply( kSes, EGS_SELL_ED_ITEM_ACK, kAck );

	kUnit.m_iED += iPrice;

	pDB->SaveProgress( kUnit.m_nUnitUID, kUnit.m_iLevel, kUnit.m_iEXP, kUnit.m_iED );

	kAck.m_iOK	= NetError::NET_OK;
	kAck.m_iED	= kUnit.m_iED;

	CX2OfflineLog::Server( L"SHOP     sold %d x item %d for %d ED, %d ED total",
		iQuantity, kRow.m_iItemID, iPrice, kUnit.m_iED );

	return Reply( kSes, EGS_SELL_ED_ITEM_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_REPAIR_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_REPAIR_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineInventory* pInven	= CX2OfflineInventory::Instance();
	CX2OfflineDB*		 pDB	= CX2OfflineDB::Instance();

	KEGS_REPAIR_ITEM_ACK kAck;
	kAck.m_iOK		= NetError::ERR_ITEM_07;		///< cannot be repaired
	kAck.m_iED		= 0;
	kAck.m_iAPoint	= 0;

	KOfflineUnitRow kUnit;
	if( false == pDB->LoadUnit( kSes.m_nSelectedUnitUID, kUnit ) )
		return Reply( kSes, EGS_REPAIR_ITEM_ACK, kAck );

	kAck.m_iED = kUnit.m_iED;

	// KInventory::RepairItem (Inventory.cpp:8203): cost is the templet's ED per
	// point of endurance, times the endurance missing, summed over every item
	// in the request. The discount factor is 0 offline - it comes from server
	// buffs and title attributes, neither of which exists here - and the arena
	// point half is 0 because endurance repair has cost no VP since 2008,
	// which is a comment in the original.
	//
	// In practice this always finds nothing to do, because nothing offline
	// wears gear down. The real server decrements endurance per stage from the
	// drop table's bDecreaseEndurance flag; that is not implemented and is
	// noted rather than half-done, since wearing gear out without the repair
	// path working would be strictly worse than neither. The handler exists so
	// that clicking Repair at the blacksmith answers instead of timing out
	// into a network-error box five seconds later.
	int iTotalED = 0;

	// (item UID, endurance to restore it to). Collected in a first pass and
	// applied in a second, so a request the character cannot afford changes
	// nothing at all rather than repairing the first few items.
	std::vector< std::pair< UidType, int > > vecRepair;

	for( size_t i = 0; i < kReq.m_vecItemUID.size(); ++i )
	{
		KOfflineItemRow kRow;
		if( false == pInven->GetItemRow( kReq.m_vecItemUID[i], kRow ) )
			return Reply( kSes, EGS_REPAIR_ITEM_ACK, kAck );

		const CX2Item::ItemTemplet* pTemplet = CX2OfflineInventory::Templet( kRow.m_iItemID );
		if( NULL == pTemplet )
			return Reply( kSes, EGS_REPAIR_ITEM_ACK, kAck );

		if( CX2Item::PT_ENDURANCE != pTemplet->GetPeriodType() )
			return Reply( kSes, EGS_REPAIR_ITEM_ACK, kAck );

		const int iMissing = pTemplet->GetEndurance() - kRow.m_iEndurance;
		if( iMissing <= 0 )
			continue;						///< already full; costs nothing

		iTotalED += (int)( pTemplet->GetRepairED() * (float)iMissing );

		vecRepair.push_back( std::make_pair( kRow.m_nItemUID, pTemplet->GetEndurance() ) );
	}

	if( iTotalED < 0 )
		iTotalED = 0;

	if( kUnit.m_iED < iTotalED )
	{
		CX2OfflineLog::Server( L"ITEM     repair refused: %d ED needed, %d held", iTotalED, kUnit.m_iED );
		return Reply( kSes, EGS_REPAIR_ITEM_ACK, kAck );
	}

	for( size_t i = 0; i < vecRepair.size(); ++i )
	{
		KInventoryItemInfo kInfo;

		if( true == pInven->SetEndurance( vecRepair[i].first, vecRepair[i].second, kInfo ) )
			kAck.m_vecInventorySlotInfo.push_back( kInfo );
	}

	kUnit.m_iED -= iTotalED;

	pDB->SaveProgress( kUnit.m_nUnitUID, kUnit.m_iLevel, kUnit.m_iEXP, kUnit.m_iED );

	kAck.m_iOK	= NetError::NET_OK;
	kAck.m_iED	= kUnit.m_iED;

	CX2OfflineLog::Server( L"ITEM     repaired %u item(s) for %d ED, %d ED left",
		(unsigned int)vecRepair.size(), iTotalED, kUnit.m_iED );

	return Reply( kSes, EGS_REPAIR_ITEM_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// Refused, not unhandled. Both arm an AddServerPacket wait, so dropping them
// would time out into a modal network error five seconds later.

bool CX2OfflineServer::Handler_EGS_ENCHANT_ITEM_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	// Enhancement needs the per-level success rates and stat multipliers out of
	// the server's enchant tables. There is no client copy - the client draws
	// the stat preview from data the server sent it - so there is nothing to
	// compute a result from, and inventing a success curve would silently
	// destroy items. Off, and it says so.
	KEGS_ENCHANT_ITEM_ACK kAck;
	kAck.m_iOK = NetError::ERR_ENCHANT_ITEM_00;

	CX2OfflineLog::Server( L"ITEM     refused an enhancement - the enchant tables are server data"
		L" with no client copy, so enhancement is not implemented" );

	return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_SOCKET_ITEM_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	KEGS_SOCKET_ITEM_ACK kAck;
	kAck.m_iOK = NetError::ERR_SOCKET_ITEM_00;

	CX2OfflineLog::Server( L"ITEM     refused a socketing - the socket option tables are server"
		L" data with no client copy, so socketing is not implemented" );

	return Reply( kSes, EGS_SOCKET_ITEM_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// Phase 12: EGS_RESOLVE_ITEM_REQ (dismantle) used to sit on
// X2OfflineIgnore.cpp's ignore list, which sends no reply and leaves the
// dialog hanging. Same shape as the two refusals just above: the yield table
// (ResolveTable.lua -> CXSLResolveItemManager::m_mapResolveData /
// m_vecBrokenPieceResolve) is server-only data with no client copy.

#ifdef SERV_IRUHADEV_OFFLINE_ITEM_RESOLVE
bool CX2OfflineServer::Handler_EGS_RESOLVE_ITEM_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	KEGS_RESOLVE_ITEM_ACK kAck;
	kAck.m_iOK		= NetError::ERR_RESOLVE_ITEM_04;	///< "분해를 할 수 없습니다." (cannot dismantle)
	kAck.m_bJackpot	= false;
#ifdef SERV_MULTI_RESOLVE
	kAck.m_iED = 0;
#endif SERV_MULTI_RESOLVE

	CX2OfflineLog::Server( L"ITEM     refused a dismantle - the resolve-yield table"
		L" (ResolveTable.lua -> CXSLResolveItemManager) is server data with no client copy,"
		L" so the item workshop is not implemented offline" );

	return Reply( kSes, EGS_RESOLVE_ITEM_ACK, kAck );
}
#endif SERV_IRUHADEV_OFFLINE_ITEM_RESOLVE

#endif SERV_IRUHADEV_OFFLINE
