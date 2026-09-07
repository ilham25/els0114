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
//              EGS_SOCKET_ITEM_REQ is answered with a refusal rather than left
//              unhandled, because the client waits on it and a silent drop
//              turns into a five-second timeout and a modal error box.
//              Socketing needs the socket option tables, which are server data
//              this build has no copy of, so it is off and says so.
//              EGS_ENCHANT_ITEM_REQ was in that same sentence until phase 23
//              found EnchantTable.lua and implemented it for real.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineInventory.h"
#include "X2OfflineSkill.h"

//{{ Iruha : 2026-09-08 // phase 36 - the level-up scrolls
// The server's own names and values, CXSLItem::EI_CHAR_LEVEL_UP_ITEM and
// CXSLItem::EL_CHAR_LEVEL_UP_ITEM2 (XSLItem.h:713, :773). Both flags that gate
// them - SERV_CHAR_LEVEL_UP_ITEM and SERV_CHAR_LEVEL_UP_ITEM_EVENT_RENA - are
// on in this build, so both ids reach the live validation switch; only the
// first has a live effect case. Kept local to this file rather than added to
// X2Define.h, which is a studio header inside every project's PCH.
static const int CHAR_LEVEL_UP_ITEM_ID		= 160267;	///< "Philosopher's Scroll"
static const int CHAR_LEVEL_UP_ITEM_ID_2	= 60004276;	///< the Rena-event scroll; dead on live, see below
//}}

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

	//{{ Iruha : 2026-09-08 // phase 36 - the ED, and the Philosopher's Scroll
	// ISSUES_2.md #6: this field was left at 0 on every reply including the
	// success path, and CX2UIInventory::Handler_EGS_USE_ITEM_IN_INVENTORY_ACK
	// ASSIGNS it over the character's ED (X2UIInventory.cpp:8919) rather than
	// treating it as a delta - so using anything from the bag emptied the
	// wallet. The real server sources it from GetED() (GSUserInventory.cpp:4544).
	//
	// Filled here, on the initial error value, the way the cube handler does
	// (Handlers_Social.cpp:2637): that is what makes it impossible for one of
	// the early `return Reply(...)` refusals below to ship a zero.
	FillAckED( kSes, kAck.m_iED );
	//}}

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

	CX2OfflineSkill* pSkill = CX2OfflineSkill::Instance();
	pSkill->Load( kSes.m_nSelectedUnitUID );

	// Phase 21: "Camilla's Secret Manual" (270970-270973) and every other
	// unseal item. Checked before anything is consumed - the live server refuses
	// with ERR_USE_ITEM_IN_INVENTORY_00 and keeps the item when the character's
	// class does not match (GSUserInventory.cpp:5596-5608), and a manual eaten
	// for a skill this character cannot learn is unrecoverable.
	const int iUnsealSkillID = CX2OfflineSkill::SkillForUnsealItem( kRow.m_iItemID );
	const bool bIsUnsealItem =
		( NULL != g_pData && NULL != g_pData->GetSkillTree() &&
		  true == g_pData->GetSkillTree()->IsUnsealSkillItemID( kRow.m_iItemID ) );

	if( true == bIsUnsealItem )
	{
		if( 0 == iUnsealSkillID )
		{
			kAck.m_iOK = NetError::ERR_USE_ITEM_IN_INVENTORY_00;

			CX2OfflineLog::Server( L"SKILL    refused - item %d unseals no skill this class has"
				L" (the manual is left in the bag)", kRow.m_iItemID );

			return Reply( kSes, EGS_USE_ITEM_IN_INVENTORY_ACK, kAck );
		}

		if( true == pSkill->IsSkillUnsealed( iUnsealSkillID ) )
		{
			kAck.m_iOK = NetError::ERR_USE_ITEM_IN_INVENTORY_00;

			CX2OfflineLog::Server( L"SKILL    refused - item %d unseals skill %d, which is already"
				L" unsealed (the manual is left in the bag)",
				kRow.m_iItemID, iUnsealSkillID );

			return Reply( kSes, EGS_USE_ITEM_IN_INVENTORY_ACK, kAck );
		}
	}

	// Phase 22: the skill note. Both ids do the same thing on the live server -
	// CXSLItem::SI_SKILL_NOTE_ITEM (GSUserInventory.cpp:4610) and
	// CXSLItem::CI_CASH_SKILL_NOTE_ITEM (GSUserCashShop.cpp:1585) both call
	// GetExpandSkillNotePage and UpdateSkillNoteMaxPageNum. The cash one is the
	// only one the offline shop sells, so both are accepted from the bag.
	const bool bIsSkillNoteItem =
		( SKILL_NOTE_ITEM_ID == kRow.m_iItemID || CASH_SKILL_NOTE_ITEM_ID == kRow.m_iItemID );

	char cNewSkillNotePage = 0;

	if( true == bIsSkillNoteItem )
	{
		KOfflineUnitRow kUnitRow;
		CX2OfflineDB::Instance()->LoadUnit( kSes.m_nSelectedUnitUID, kUnitRow );

		cNewSkillNotePage = CX2OfflineSkill::ExpandSkillNotePageForLevel( kUnitRow.m_iLevel );

		if( 0 == cNewSkillNotePage )
		{
			// The server logs this as "should never happen" because its shop
			// refuses the purchase below level 20 first. Offline the shop does
			// not, so it is a real case and the note stays in the bag.
			kAck.m_iOK = NetError::ERR_SKILL_NOTE_07;

			CX2OfflineLog::Server( L"SKILL    refused - the skill note needs level 20-69, this"
				L" character is level %d (the note is left in the bag)", kUnitRow.m_iLevel );

			return Reply( kSes, EGS_USE_ITEM_IN_INVENTORY_ACK, kAck );
		}

		if( cNewSkillNotePage <= pSkill->GetSkillNoteMaxPage() )
		{
			kAck.m_iOK = NetError::ERR_SKILL_NOTE_06;

			CX2OfflineLog::Server( L"SKILL    refused - the skill note would give %d page(s) and"
				L" this character already has %d (the note is left in the bag)",
				(int)cNewSkillNotePage, (int)pSkill->GetSkillNoteMaxPage() );

			return Reply( kSes, EGS_USE_ITEM_IN_INVENTORY_ACK, kAck );
		}
	}

	//{{ Iruha : 2026-09-08 // phase 36 - the Philosopher's Scroll (160267)
	// Route A (OFFLINE_MODE_PHASE36_PLAN.md 0.1): 160267 is not in the client's
	// RandomItem.lua registry and has no case of its own in CX2UIInventory's
	// switch, so the right-click falls through to `default:`
	// (X2UIInventory.cpp:6894) and arrives here rather than at the cube handler.
	//
	// The live server splits it across two switch arms - validation at
	// GSUserInventory.cpp:4162 and the effect at :4655. (The second copy of both
	// at :4940 / :5931 is inside an #else AND commented out; anything above 4938
	// is dead code.) Everything below the consume transcribes :4655.
	const bool bIsCharLevelUpItem = ( CHAR_LEVEL_UP_ITEM_ID == kRow.m_iItemID );

	int iLevelUpAddEXP = 0;

	// 60004276 is refused rather than transcribed. EL_CHAR_LEVEL_UP_ITEM2 has a
	// case in the live *validation* switch (GSUserInventory.cpp:4164) but none
	// in the live *effect* switch at :4655 - its only effect case is at :6066,
	// inside the commented-out #else. So on the live US server this item is
	// validated, consumed, and does nothing. Reproducing that faithfully would
	// eat a player's item for nothing, and an item consumed for nothing is
	// unrecoverable; offline diverges deliberately and says so here rather than
	// leaving the divergence to be rediscovered.
	if( CHAR_LEVEL_UP_ITEM_ID_2 == kRow.m_iItemID )
	{
		kAck.m_iOK = NetError::ERR_UNKNOWN;

		CX2OfflineLog::Server( L"ITEM     refused item %d - the live server consumes it and does"
			L" nothing (GSUserInventory.cpp:4655 has no case for it; the one at :6066 is inside"
			L" the commented-out #else), so offline leaves it in the bag instead",
			kRow.m_iItemID );

		return Reply( kSes, EGS_USE_ITEM_IN_INVENTORY_ACK, kAck );
	}

	if( true == bIsCharLevelUpItem )
	{
		KOfflineUnitRow kUnit;
		if( false == CX2OfflineDB::Instance()->LoadUnit( kSes.m_nSelectedUnitUID, kUnit ) )
			return Reply( kSes, EGS_USE_ITEM_IN_INVENTORY_ACK, kAck );

		// The live refusal, GSUserInventory.cpp:4166-4178: at the cap the scroll
		// is refused with the item intact. SERV_LIMIT_LEVEL_NOT_USE_ITEM_MSG is
		// JP-only, so the US code is ERR_UNKNOWN and not ERR_NOT_USE_01. The
		// server reads its cap from SiKGameSysVal()->GetLimitsLevel(); offline
		// stands in g_iMaxLevel, whose own comment beside the #define says it and
		// GameSysValTable.lua's MAXLevel must be changed together
		// (Always_US.h:75), and which ApplyDungeonReward already caps against.
		//
		// The live test is `==`; `>=` here because a character somehow above the
		// cap must be refused too rather than fall through to the effect.
		if( kUnit.m_iLevel >= (int)_CONST_X2GAME_::g_iMaxLevel )
		{
			kAck.m_iOK = NetError::ERR_UNKNOWN;

			CX2OfflineLog::Server( L"ITEM     refused item %d - the character is level %d and the"
				L" cap is %d (the scroll is left in the bag)",
				kRow.m_iItemID, kUnit.m_iLevel, (int)_CONST_X2GAME_::g_iMaxLevel );

			return Reply( kSes, EGS_USE_ITEM_IN_INVENTORY_ACK, kAck );
		}

		// The scroll does not grant a flat amount of EXP: :4655 tops the
		// character up to EXACTLY the total the next level needs, so a character
		// 40% through level 30 ends at 0% of level 31. Adding a "need" figure
		// instead would over-pay a mid-level character and could grant two.
		if( NULL == g_pData || NULL == g_pData->GetEXPTable() )
			return Reply( kSes, EGS_USE_ITEM_IN_INVENTORY_ACK, kAck );

		const int iNextLevelTotalEXP =
			g_pData->GetEXPTable()->GetEXPData( kUnit.m_iLevel + 1 ).m_nTotalExp;

		// CX2EXPTable::GetEXPData returns a zeroed EXPData for a level it has no
		// row for (X2EXPTable.cpp:43-54), so a missing row reads as a total of 0
		// and would compute a NEGATIVE top-up - which ApplyDungeonReward would
		// clamp to zero EXP, wiping the character's progress for a consumed
		// scroll. Refused instead, with the item intact.
		if( iNextLevelTotalEXP <= kUnit.m_iEXP )
		{
			kAck.m_iOK = NetError::ERR_UNKNOWN;

			CX2OfflineLog::Server( L"ITEM     refused item %d - the EXP table has no usable row for"
				L" level %d (it reports total EXP %d against the character's %d), so the top-up"
				L" would be negative (the scroll is left in the bag)",
				kRow.m_iItemID, kUnit.m_iLevel + 1, iNextLevelTotalEXP, kUnit.m_iEXP );

			return Reply( kSes, EGS_USE_ITEM_IN_INVENTORY_ACK, kAck );
		}

		iLevelUpAddEXP = iNextLevelTotalEXP - kUnit.m_iEXP;
	}
	//}}

	KInventoryItemInfo kSlotInfo;
	if( false == pInven->ConsumeOne( kReq.m_iItemUID, kSlotInfo ) )
		return Reply( kSes, EGS_USE_ITEM_IN_INVENTORY_ACK, kAck );

	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_iUsedItemID	= kRow.m_iItemID;
	kAck.m_vecKInventorySlotInfo.push_back( kSlotInfo );

	CX2OfflineLog::Server( L"ITEM     used item %d from the bag", kRow.m_iItemID );

	Reply( kSes, EGS_USE_ITEM_IN_INVENTORY_ACK, kAck );

	//{{ Iruha : 2026-09-08 // phase 36 - the level-up itself
	// GSUserInventory.cpp:4655-4671, transcribed: add the difference up to the
	// next level's total, then CheckCharLevelUp(). ApplyDungeonReward is the
	// offline equivalent of that pair - it is not dungeon-specific, it is the
	// only exp/level/SP path there is, and it already levels off the client's own
	// EXP table, grants the skill points the curve owes and persists all of it.
	// A second level-up path beside it is how the client and the save start
	// disagreeing about what level the character is.
	if( true == bIsCharLevelUpItem && 0 < iLevelUpAddEXP )
	{
		int iOldLevel = 0;
		const int iNewLevel =
			ApplyDungeonReward( kSes.m_nSelectedUnitUID, iLevelUpAddEXP, 0, &iOldLevel );

		// Only if the level actually rose, the same guard the dungeon clear bonus
		// uses (Handlers_Room.cpp:2880). EGS_CHAR_LEVEL_UP_NOT is what sets
		// CX2Unit::SetIsLevelUp and plays the effect; QuestOnLevelUp runs the
		// level-up quest and title steps.
		if( iNewLevel > iOldLevel )
		{
			PushLevelUp( kSes, kSes.m_nSelectedUnitUID );
			QuestOnLevelUp( kSes );
		}

		CX2OfflineLog::Server( L"ITEM     item %d topped the character up by %d exp: level %d -> %d",
			kRow.m_iItemID, iLevelUpAddEXP, iOldLevel, iNewLevel );
	}
	//}}

	// Phase 19: the bag path needs the same _NOT the quick-slot path needs, and
	// for the same reason - CX2UIInventory::Handler_EGS_USE_ITEM_IN_INVENTORY_ACK
	// only updates the bag, the ED and the sort order (X2UIInventory.cpp:8908),
	// it never touches the unit. The one client handler that calls
	// UseItemSpecialAbility / ApplyBuffFactorToGUUser is
	// CX2UIQuickSlot::Handler_EGS_USE_QUICK_SLOT_NOT (X2UIQuickSlot.cpp:1032-1036),
	// and it works off the item ID alone - it does not care that the item came
	// from a quick slot. Without it "Giant Potion (Elixir)" (78894) and its seven
	// siblings are eaten by the bag and do nothing at all, which is ISSUES.md #2.
	//
	// The live server does this differently: it owns the buff (KGSUser::
	// ActivateItemBuff, GSUserFunction.cpp:15978) and pushes the whole world-buff
	// list back as EGS_UPDATE_BUFF_INFO_IN_ROOM_NOT. That needs a buff-factor ID
	// the client cannot look up from a buff templet ID, plus a persisted buff
	// table; the item's own BUFF_FACTOR entry in the client's Item.lua carries
	// the same effect (78894's row names BFI_BUFF_ELIXIR_GIANT_POTION), so the
	// _NOT applies the real factor. The difference is that the buff lives for
	// the run rather than for a wall-clock duration across rooms.
	//
	// Only inside a room, and only for an item that actually has an in-game
	// effect: Handler_EGS_USE_QUICK_SLOT_NOT's default branch is an
	// ASSERT( !L"Can not use this item in this State" ), and an item with no
	// ability and no buff factor would only add a "you used X" chat line.
	if( S_ROOM == kSes.m_eState && NULL != pTemplet &&
		( 0 < pTemplet->GetNumSpecialAbility() || 0 < pTemplet->GetNumBuffFactorPtr() ) )
	{
		KEGS_USE_QUICK_SLOT_NOT kNot;
		kNot.m_UnitUID				= kSes.m_nSelectedUnitUID;
		kNot.m_ItemID				= kRow.m_iItemID;
		kNot.m_bUseItemNeedPayment	= false;

		Reply( kSes, EGS_USE_QUICK_SLOT_NOT, kNot );

		CX2OfflineLog::Server( L"ITEM     item %d has %u ability / %u buff factor(s) -"
			L" sent USE_QUICK_SLOT_NOT so the effect applies",
			kRow.m_iItemID, pTemplet->GetNumSpecialAbility(), pTemplet->GetNumBuffFactorPtr() );
	}

	// Phase 21: the unseal, which is a _NOT of its own. The live server sends it
	// out of DBE_UNSEAL_SKILL_ACK (GSUserGameCommon.cpp:6716-6730); the client's
	// CX2SkillTree::Handler_EGS_UNSEAL_SKILL_NOT is what actually calls
	// AddSkillUnsealed, redraws the tree and opens the "skill unsealed" dialog.
	// The ACK above carries nothing about it.
	if( true == bIsUnsealItem && 0 != iUnsealSkillID )
	{
		pSkill->UnsealSkill( iUnsealSkillID );

		KEGS_UNSEAL_SKILL_NOT kNot;
		kNot.m_iSkillID = iUnsealSkillID;

		Reply( kSes, EGS_UNSEAL_SKILL_NOT, kNot );

		CX2OfflineLog::Server( L"SKILL    item %d unsealed skill %d", kRow.m_iItemID, iUnsealSkillID );
	}

	// Phase 22: the page count, likewise a _NOT. Without it the client leaves
	// GetMaxSkillNoteSlot() at zero, keeps the skill-note button hidden
	// (HideSkillNote( true )) and refuses every memo with STR_ID_4988 before it
	// puts a byte on the wire - which is what ISSUES.md #12 was.
	if( true == bIsSkillNoteItem && 0 != cNewSkillNotePage )
	{
		pSkill->SetSkillNoteMaxPage( cNewSkillNotePage );

		KEGS_EXPAND_SKILL_NOTE_PAGE_NUM_NOT kNot;
		kNot.m_cExpandedPageNum = cNewSkillNotePage;

		Reply( kSes, EGS_EXPAND_SKILL_NOTE_PAGE_NUM_NOT, kNot );

		CX2OfflineLog::Server( L"SKILL    item %d gave the skill note %d page(s)",
			kRow.m_iItemID, (int)cNewSkillNotePage );
	}

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
// Phase 23: EGS_ENCHANT_ITEM_REQ used to be a hardcoded refusal that did not
// even read its request, on the grounds that the enchant tables were server
// data with no client copy. EnchantTable.lua is now packed into data036.kom,
// so this ports the real thing: KInventory::EnchantItem (Inventory.cpp:12098)
// plus the KGSUser handler around it (GSUserInventory.cpp:3042) and the ACK
// that DBE_ENCHANT_ITEM_ACK sends back (:3421). CX2OfflineEnchantTable is the
// data half; see its header for what is and is not client-side.
//
// The client half of the feature was never broken and is not touched.
// CX2UIShop::EnchantItem (X2UIShop.cpp:2642) refuses to even open the dialog
// unless the character holds a stone in the item's own level band, which is
// why no EGS_ENCHANT_ITEM_REQ appears in the play-test census: that is the
// client working, not a gate to remove.

namespace
{
	/// The five NetError::ERR_ENCHANT_RESULT_* values in English, for the log.
	/// NetError::GetErrStr would give the shipped Korean text in CP949, which
	/// offline_server.log is not.
	const wchar_t* EnchantResultStr( int iEnchantResult )
	{
		switch( iEnchantResult )
		{
		case NetError::ERR_ENCHANT_RESULT_00:	return L"success";
		case NetError::ERR_ENCHANT_RESULT_01:	return L"no change";
		case NetError::ERR_ENCHANT_RESULT_02:	return L"down one";
		case NetError::ERR_ENCHANT_RESULT_03:	return L"reset to +0";
		case NetError::ERR_ENCHANT_RESULT_04:	return L"BROKEN";
		}

		return L"?";
	}
}

bool CX2OfflineServer::Handler_EGS_ENCHANT_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_ENCHANT_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineInventory*	pInven	= CX2OfflineInventory::Instance();
	CX2OfflineDB*			pDB		= CX2OfflineDB::Instance();
	CX2OfflineEnchantTable*	pTable	= CX2OfflineEnchantTable::Instance();

	// The real server's failure reply is a default-constructed ACK with only
	// m_iOK set: the client tests it with IsValidPacket and puts up the message
	// for that error code without reading anything else.
	KEGS_ENCHANT_ITEM_ACK kAck;
	kAck.m_iOK				= NetError::ERR_ENCHANT_ITEM_00;
	kAck.m_iED				= 0;
	kAck.m_iEnchantResult	= NetError::ERR_ENCHANT_RESULT_01;
	kAck.m_iEnchantedItemUID = 0;
	kAck.m_iEnchantedItemID	= 0;

	//////////////////////////////////////////////////////////////////////////
	// The data has to be there before anything else is checked, because
	// everything below turns on the probability rows.

	if( false == pTable->IsLoaded() )
	{
		kAck.m_iOK = NetError::ERR_ENCHANT_ITEM_06;		///< "cannot proceed with enhancement"

		CX2OfflineLog::Server( L"ITEM     refused an enhancement - EnchantTable.lua is not loaded."
			L" XOR-encrypt KncWX2Server/ServerResource/US/EnchantTable.lua and pack it into"
			L" data036.kom; the ENCHANT lines above say what went wrong." );

		return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );
	}

	//////////////////////////////////////////////////////////////////////////
	// KInventory::EnchantItem's gates, in its order.

	KOfflineItemRow kRow;
	if( false == pInven->GetItemRow( kReq.m_iItemUID, kRow ) )
		return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );	///< ERR_ENCHANT_ITEM_00

	if( CX2Inventory::ST_BANK == kRow.m_iCategory ||
		CX2Inventory::ST_SHARE_BANK == kRow.m_iCategory )
	{
		kAck.m_iOK = NetError::ERR_BANK_02;
		return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );
	}

	// KInventoryItem::IsSealed (InventoryItem.cpp:501).
	if( kRow.m_iSealData >= 100 )
	{
		kAck.m_iOK = NetError::ERR_ENCHANT_ITEM_14;
		return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );
	}

	const CX2Item::ItemTemplet* pTemplet = CX2OfflineInventory::Templet( kRow.m_iItemID );
	if( NULL == pTemplet )
	{
		kAck.m_iOK = NetError::ERR_ENCHANT_ITEM_01;
		return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );
	}

	if( false == pTemplet->GetCanEnchant() )
	{
		kAck.m_iOK = NetError::ERR_ENCHANT_ITEM_03;
		return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );
	}

	// KInventoryItem::IsBroken (InventoryItem.cpp:479): an enchantable item
	// with a negative level. The client checks this too (CX2Item::IsDisabled),
	// so reaching it means the two disagree, which is worth a log line.
	if( kRow.m_iEnchantLevel < 0 )
	{
		CX2OfflineLog::Server( L"ITEM     enhancement refused: item %d is broken (+%d)",
			kRow.m_iItemID, kRow.m_iEnchantLevel );

		kAck.m_iOK = NetError::ERR_ENCHANT_ITEM_12;
		return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );
	}

	const int iEnchantLevel = kRow.m_iEnchantLevel;

	// SERV_SUPPORT_MATERIAL_LIMIT is on in this build. The event limit arm
	// (SERV_SUPPORT_MATERIAL_ENCHANT_EVENT) is compiled in on the real server
	// but only taken while KGameSysVal says an enchant event is running;
	// nothing offline runs one, so this is the plain arm.
	if( true == kReq.m_bIsSupportMaterial )
	{
		const int iLimit = pTable->GetSupportMaterialEnchantLimitLevel();

		if( iLimit > 0 && iEnchantLevel >= iLimit )
		{
			// The real server switches on the limit VALUE to pick the message,
			// so that the dialog names the right level; 10 and 11 are the only
			// two it knows, and anything else falls back to the +10 wording.
			kAck.m_iOK = ( 11 == iLimit ) ? NetError::ERR_ENCHANT_ITEM_15
										  : NetError::ERR_ENCHANT_ITEM_11;

			return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );
		}
	}

	if( iEnchantLevel >= CX2OfflineEnchantTable::MAX_ENCHANT_LEVEL )
	{
		kAck.m_iOK = NetError::ERR_ENCHANT_ITEM_04;		///< cannot be enhanced further
		return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );
	}

	//////////////////////////////////////////////////////////////////////////
	// Cost. Identical to the figure the client already printed in the dialog
	// (CX2UIShop::OpenEnchantDLG, X2UIShop.cpp:2797) - both are
	// price * 0.1 * 0.5 for a weapon and price * 0.1 * 0.1 for armour, in that
	// order and with that float truncation.

	int iED = 0;

	if( CX2Item::IT_WEAPON == pTemplet->GetItemType() )
	{
		iED = (int)( ( (float)pTemplet->GetPrice() * 0.1f ) * 0.5f );
	}
	else if( CX2Item::IT_DEFENCE == pTemplet->GetItemType() )
	{
		iED = (int)( ( (float)pTemplet->GetPrice() * 0.1f ) * 0.1f );
	}
	else
	{
		kAck.m_iOK = NetError::ERR_ENCHANT_ITEM_02;		///< weapons and armour only
		return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );
	}

	KOfflineUnitRow kUnit;
	if( false == pDB->LoadUnit( kSes.m_nSelectedUnitUID, kUnit ) )
		return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );

	if( kUnit.m_iED < iED )
	{
		kAck.m_iOK = NetError::ERR_ENCHANT_ITEM_07;
		return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );
	}

	//////////////////////////////////////////////////////////////////////////
	// Which stone. m_bIsNewEnchant is the client's "special enhancement"
	// button: true means the level-banded stones from AddEnchantStoneInfo,
	// false means the two flat pre-2008 ones. m_bIsRareEnchantStone is dead -
	// X2UIShop.cpp:3753 sets it false and calls it obsolete - but it is still
	// what selects the rare probability table below, so it is read, not
	// assumed.

	int iEnchantStoneID = 0;

	if( true == kReq.m_bIsNewEnchant )
	{
		// SERV_EVENT_ENCHANT_STONE_MULTI: any event stone the character
		// actually holds wins over the standard one.
		std::vector< int > vecEventStoneID;
		pTable->GetEventEnchantStoneItemID( pTemplet->GetUseLevel(),
											(int)pTemplet->GetItemType(), vecEventStoneID );

		for( size_t i = 0; i < vecEventStoneID.size(); ++i )
		{
			if( vecEventStoneID[i] > 0 &&
				pInven->CountItemByID( vecEventStoneID[i], true ) > 0 )
			{
				iEnchantStoneID = vecEventStoneID[i];
				break;
			}
		}

		if( 0 == iEnchantStoneID )
		{
			iEnchantStoneID = pTable->GetEnchantStoneItemID( pTemplet->GetUseLevel(),
															 (int)pTemplet->GetItemType() );
		}

		if( iEnchantStoneID <= 0 )
		{
			CX2OfflineLog::Server( L"ITEM     enhancement refused: no stone registered for"
				L" use level %d, item type %d - EnchantTable.lua is incomplete?",
				pTemplet->GetUseLevel(), (int)pTemplet->GetItemType() );

			kAck.m_iOK = NetError::ERR_ENCHANT_ITEM_06;
			return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );
		}
	}
	else
	{
		// SERV_BLESSED_RURIEL_ENCHANT_STONE_EVENT is defined for US
		// (ServerDefine_US.h:607), and it rewrites this branch: the rare stone
		// is tried first whatever the request said, then Ruriel's event stone,
		// then the plain one. The client's own count in
		// CX2UIShop::EnchantItem adds the same Ruriel IDs under the same flag.
		const bool bWeapon = ( CX2Item::IT_WEAPON == pTemplet->GetItemType() );

		iEnchantStoneID = bWeapon ? pTable->GetRareWeaponEnchantStoneID()
								  : pTable->GetRareArmorEnchantStoneID();

		if( pInven->CountItemByID( iEnchantStoneID, true ) <= 0 )
		{
			iEnchantStoneID = bWeapon
				? _CONST_BLESSED_RURIEL_ENCHANT_STONE_EVENT::iRuriel_Enchant_Stone_Weapon_Item
				: _CONST_BLESSED_RURIEL_ENCHANT_STONE_EVENT::iRuriel_Enchant_Stone_Defence_Item;

			if( pInven->CountItemByID( iEnchantStoneID, true ) <= 0 )
			{
				iEnchantStoneID = bWeapon ? pTable->GetWeaponEnchantStoneID()
										  : pTable->GetArmorEnchantStoneID();
			}
		}
	}

	if( pInven->CountItemByID( iEnchantStoneID, true ) <= 0 )
	{
		CX2OfflineLog::Server( L"ITEM     enhancement refused: no enchant stone %d in the bag",
			iEnchantStoneID );

		kAck.m_iOK = NetError::ERR_ENCHANT_ITEM_05;
		return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );
	}

	// Worn gear cannot be enhanced - the item has to be in the bag first.
	if( CX2Inventory::ST_E_EQUIP == kRow.m_iCategory )
	{
		kAck.m_iOK = NetError::ERR_ENCHANT_ITEM_08;
		return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );
	}

	//////////////////////////////////////////////////////////////////////////
	// The fluor stone, if one was asked for. Found and RESERVED here but not
	// spent until the roll is done, because the real server's order is: find
	// it, spend it, then let it rewrite the result. Refusing after the roll
	// would be indistinguishable, but refusing before it means a request that
	// cannot be served has not consumed the character's luck for the attempt.

	int iSupportMaterialID = 0;

	if( true == kReq.m_bIsSupportMaterial )
	{
		// SERV_COBO_SUPPORT_MATERIAL_EVENT + SERV_EVENT_SUPPORT_MATERIAL_MULTI:
		// an event fluor stone in the bag wins over the standard one.
		std::vector< int > vecEventSupportID;
		pTable->GetEventEnchantSupportMaterialItemID( pTemplet->GetUseLevel(), vecEventSupportID );

		for( size_t i = 0; i < vecEventSupportID.size(); ++i )
		{
			if( vecEventSupportID[i] > 0 &&
				pInven->CountItemByID( vecEventSupportID[i], true ) > 0 )
			{
				iSupportMaterialID = vecEventSupportID[i];
				break;
			}
		}

		if( 0 == iSupportMaterialID )
			iSupportMaterialID = pTable->GetEnchantSupportMaterialItemID( pTemplet->GetUseLevel() );

		if( iSupportMaterialID <= 0 )
		{
			kAck.m_iOK = NetError::ERR_ENCHANT_ITEM_10;		///< no fluor stone for this level
			return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );
		}

		if( pInven->CountItemByID( iSupportMaterialID, true ) <= 0 )
		{
			kAck.m_iOK = NetError::ERR_ENCHANT_ITEM_09;		///< none in the bag
			return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// The roll. Keyed by the level being ATTEMPTED, hence +1.

	int iEnchantResult = NetError::ERR_ENCHANT_RESULT_01;

	if( false == pTable->GetEnchantResult( kReq.m_bIsRareEnchantStone,
										   iEnchantLevel + 1, iEnchantResult ) )
	{
		CX2OfflineLog::Server( L"ITEM     enhancement refused: no probability row for +%d%s",
			iEnchantLevel + 1, kReq.m_bIsRareEnchantStone ? L" (rare)" : L"" );

		kAck.m_iOK = NetError::ERR_ENCHANT_ITEM_06;
		return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );
	}

	// The GM cheat checkbox. The client only sets m_bDebug for an account with
	// CX2User::XUAL_DEV, and the real server re-checks the auth level before
	// honouring it; offline the account is whatever the login handler said it
	// was, so this is honoured as sent rather than re-derived.
	if( true == kReq.m_bDebug )
	{
		iEnchantResult = NetError::ERR_ENCHANT_RESULT_00;

		CX2OfflineLog::Server( L"ITEM     enhancement forced to succeed by the dev cheat box" );
	}

	//////////////////////////////////////////////////////////////////////////
	// Spend the fluor stone and let it rewrite the result: it turns a
	// down-one or a reset into "no change". It does NOT stop a break, which is
	// what the separate destroy-guard item was for (TW/HK only).

	if( iSupportMaterialID > 0 )
	{
		pInven->ConsumeByID( iSupportMaterialID, 1, kAck.m_vecInventorySlotInfo );

		if( NetError::ERR_ENCHANT_RESULT_02 == iEnchantResult ||
			NetError::ERR_ENCHANT_RESULT_03 == iEnchantResult )
		{
			iEnchantResult = NetError::ERR_ENCHANT_RESULT_01;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Apply. KInventory::EnchantItem's result switch (Inventory.cpp:12952) as
	// absolute levels rather than as its Increase/Decrease deltas - a break is
	// a decrease of abs(level)*2, i.e. the level with its sign flipped.

	int iLevelAfterEnchant = iEnchantLevel;

	switch( iEnchantResult )
	{
	case NetError::ERR_ENCHANT_RESULT_00:	iLevelAfterEnchant = iEnchantLevel + 1;	break;
	case NetError::ERR_ENCHANT_RESULT_01:	/* unchanged */							break;
	case NetError::ERR_ENCHANT_RESULT_02:	iLevelAfterEnchant = iEnchantLevel - 1;	break;
	case NetError::ERR_ENCHANT_RESULT_03:	iLevelAfterEnchant = 0;					break;
	case NetError::ERR_ENCHANT_RESULT_04:	iLevelAfterEnchant = -iEnchantLevel;	break;

	default:
		CX2OfflineLog::Server( L"ITEM     enhancement produced an undefined result %d",
			iEnchantResult );

		kAck.m_iOK = NetError::ERR_ENCHANT_ITEM_06;
		return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );
	}

	if( iLevelAfterEnchant != iEnchantLevel )
	{
		KInventoryItemInfo kEnchanted;

		if( false == pInven->SetEnchantLevel( kReq.m_iItemUID, iLevelAfterEnchant, kEnchanted ) )
		{
			kAck.m_iOK = NetError::ERR_ENCHANT_ITEM_13;		///< the DB update failed
			kAck.m_vecInventorySlotInfo.clear();
			return Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );
		}

		kAck.m_vecInventorySlotInfo.push_back( kEnchanted );
	}

	// The stone goes whatever the result was.
	pInven->ConsumeByID( iEnchantStoneID, 1, kAck.m_vecInventorySlotInfo );

	kUnit.m_iED -= iED;

	pDB->SaveProgress( kUnit.m_nUnitUID, kUnit.m_iLevel, kUnit.m_iEXP, kUnit.m_iED );

	kAck.m_iOK					= NetError::NET_OK;
	kAck.m_iED					= kUnit.m_iED;
	kAck.m_iEnchantResult		= iEnchantResult;
	kAck.m_iEnchantedItemUID	= kReq.m_iItemUID;
	kAck.m_iEnchantedItemID		= kRow.m_iItemID;

	CX2OfflineLog::Server( L"ITEM     enhanced item %d: +%d -> +%d (%s), stone %d%s, %d ED, %d ED left",
		kRow.m_iItemID, iEnchantLevel, iLevelAfterEnchant,
		EnchantResultStr( iEnchantResult ), iEnchantStoneID,
		( iSupportMaterialID > 0 ) ? L" + fluor stone" : L"",
		iED, kUnit.m_iED );

	Reply( kSes, EGS_ENCHANT_ITEM_ACK, kAck );

	//////////////////////////////////////////////////////////////////////////
	// The quest hook, which is the reason two title sub-quests existed and
	// could never be finished. KUserQuestManager::Handler_OnEnchantItem is
	// called from DBE_ENCHANT_ITEM_ACK (GSUserInventory.cpp:3354) - i.e. on
	// every result EXCEPT "no change", which returns before the DB round trip.
	// The level it passes is the level AFTER the attempt, so a step that asks
	// for +5 is satisfied by reaching +5 and by nothing else.

	if( NetError::ERR_ENCHANT_RESULT_01 != iEnchantResult )
	{
		KOfflineUnitRow kQuestUnit;
		if( true == LoadQuestState( kSes, kQuestUnit ) )
		{
			std::vector< KQuestInstance > vecChanged;

			CX2OfflineQuest::Instance()->OnEnchantItem( kRow.m_iItemID, iLevelAfterEnchant,
														kQuestUnit, vecChanged );

			PushQuestUpdate( kSes, vecChanged );
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Phase 25: EGS_ENCHANT_ATTACH_ITEM_REQ (ISSUES.md #5, the "magic amulet")
// used to sit on X2OfflineIgnore.cpp's list, which sends no reply at all and
// leaves the client's dialog hanging on its AddServerPacket wait - strictly
// worse for the player than a refusal, which at least closes the dialog.
//
// It needs no new data. The amulet is DETERMINISTIC: EnchantTable.lua's
// AddAttachItemInfo says what enchant level each ticket grants, and phase 23's
// CX2OfflineEnchantTable has been parsing that table since it was packed. This
// is KInventory::EnchantAttachItem (Inventory.cpp:16066) plus the KGSUser
// handler around it (GSUserInventory.cpp:6844) and the ACK that
// DBE_ENCHANT_ATTACH_ITEM_ACK sends back (:6942).

bool CX2OfflineServer::Handler_EGS_ENCHANT_ATTACH_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_ENCHANT_ATTACH_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineInventory*	pInven	= CX2OfflineInventory::Instance();
	CX2OfflineEnchantTable*	pTable	= CX2OfflineEnchantTable::Instance();

	KEGS_ENCHANT_ATTACH_ITEM_ACK kAck;
	kAck.m_iOK = NetError::ERR_ATTACH_ITEM_00;

	if( false == pTable->IsLoaded() )
	{
		CX2OfflineLog::Server( L"ITEM     refused an enchant amulet - EnchantTable.lua is not loaded."
			L" XOR-encrypt KncWX2Server/ServerResource/US/EnchantTable.lua and pack it into data036.kom." );

		kAck.m_iOK = NetError::ERR_ATTACH_ITEM_01;
		return Reply( kSes, EGS_ENCHANT_ATTACH_ITEM_ACK, kAck );
	}

	//////////////////////////////////////////////////////////////////////////
	// KInventory::EnchantAttachItem's gates, in its order.

	KOfflineItemRow kDest;
	KOfflineItemRow kAttach;

	if( false == pInven->GetItemRow( kReq.m_iDestItemUID, kDest ) ||
		false == pInven->GetItemRow( kReq.m_iAttachItemUID, kAttach ) )
	{
		return Reply( kSes, EGS_ENCHANT_ATTACH_ITEM_ACK, kAck );		///< ERR_ATTACH_ITEM_00
	}

	// 2. Is the ticket a ticket, and what does it grant?
	int iAttachEnchantLevel	= 0;
	int iAttachItemType		= 0;

	if( false == pTable->GetAttachItemEnchantLevel( kAttach.m_iItemID,
													iAttachEnchantLevel, iAttachItemType ) )
	{
		CX2OfflineLog::Server( L"ITEM     amulet refused: item %d is not an enchant-attach ticket",
			kAttach.m_iItemID );

		kAck.m_iOK = NetError::ERR_ATTACH_ITEM_01;
		return Reply( kSes, EGS_ENCHANT_ATTACH_ITEM_ACK, kAck );
	}

	// 3. Both templets. SERV_ENCHANT_ATTACH_MODIFY is on in this build, which
	// is what makes the ticket's own templet load-bearing: it decides at the
	// end whether the ticket is a stack to decrement or an item to delete.
	const CX2Item::ItemTemplet* pDestTemplet	= CX2OfflineInventory::Templet( kDest.m_iItemID );
	const CX2Item::ItemTemplet* pAttachTemplet	= CX2OfflineInventory::Templet( kAttach.m_iItemID );

	if( NULL == pDestTemplet || NULL == pAttachTemplet )
	{
		kAck.m_iOK = NetError::ERR_ITEM_04;
		return Reply( kSes, EGS_ENCHANT_ATTACH_ITEM_ACK, kAck );
	}

	if( false == pDestTemplet->GetCanEnchant() )
	{
		kAck.m_iOK = NetError::ERR_ATTACH_ITEM_02;
		return Reply( kSes, EGS_ENCHANT_ATTACH_ITEM_ACK, kAck );
	}

	// The ticket's own item-type restriction: IT_NONE takes any weapon or
	// piece of armour, IT_DEFENCE takes armour only, and anything else is a
	// broken table row rather than a player mistake.
	if( CX2Item::IT_NONE == iAttachItemType )
	{
		if( CX2Item::IT_WEAPON != pDestTemplet->GetItemType() &&
			CX2Item::IT_DEFENCE != pDestTemplet->GetItemType() )
		{
			kAck.m_iOK = NetError::ERR_ATTACH_ITEM_02;
			return Reply( kSes, EGS_ENCHANT_ATTACH_ITEM_ACK, kAck );
		}
	}
	else if( CX2Item::IT_DEFENCE == iAttachItemType )
	{
		if( CX2Item::IT_DEFENCE != pDestTemplet->GetItemType() )
		{
			kAck.m_iOK = NetError::ERR_ATTACH_ITEM_02;
			return Reply( kSes, EGS_ENCHANT_ATTACH_ITEM_ACK, kAck );
		}
	}
	else
	{
		CX2OfflineLog::Server( L"ITEM     amulet refused: ticket %d has item type %d,"
			L" which EnchantTable.lua should never produce",
			kAttach.m_iItemID, iAttachItemType );

		kAck.m_iOK = NetError::ERR_ATTACH_ITEM_02;
		return Reply( kSes, EGS_ENCHANT_ATTACH_ITEM_ACK, kAck );
	}

	// SERV_ADD_ITEM_LEVEL_CONDITION_FOR_ENCHANT_ATTACH_ITEM, on in this build.
	if( false == pTable->CheckAttachItemEnableLevel( kAttach.m_iItemID, pDestTemplet->GetUseLevel() ) )
	{
		kAck.m_iOK = NetError::ERR_ATTACH_ITEM_04;		///< wrong equipment level for this amulet
		return Reply( kSes, EGS_ENCHANT_ATTACH_ITEM_ACK, kAck );
	}

	// 4. Broken item.
	if( kDest.m_iEnchantLevel < 0 )
	{
		kAck.m_iOK = NetError::ERR_ATTACH_ITEM_02;
		return Reply( kSes, EGS_ENCHANT_ATTACH_ITEM_ACK, kAck );
	}

	// 6. The amulet only ever raises: a target already at or above the level
	// the ticket grants is refused, not silently downgraded.
	if( kDest.m_iEnchantLevel >= iAttachEnchantLevel )
	{
		CX2OfflineLog::Server( L"ITEM     amulet refused: item %d is already +%d, ticket grants +%d",
			kDest.m_iItemID, kDest.m_iEnchantLevel, iAttachEnchantLevel );

		kAck.m_iOK = NetError::ERR_ATTACH_ITEM_02;
		return Reply( kSes, EGS_ENCHANT_ATTACH_ITEM_ACK, kAck );
	}

	// 7. Range.
	if( kDest.m_iEnchantLevel > CX2OfflineEnchantTable::MAX_ENCHANT_LEVEL )
	{
		kAck.m_iOK = NetError::ERR_ATTACH_ITEM_02;
		return Reply( kSes, EGS_ENCHANT_ATTACH_ITEM_ACK, kAck );
	}

	// 8. Worn gear cannot be attached to.
	if( CX2Inventory::ST_E_EQUIP == kDest.m_iCategory )
	{
		kAck.m_iOK = NetError::ERR_ATTACH_ITEM_02;
		return Reply( kSes, EGS_ENCHANT_ATTACH_ITEM_ACK, kAck );
	}

	//////////////////////////////////////////////////////////////////////////
	// 9. Apply. No roll, no ED, no chance of loss - that is the whole point of
	// the amulet.

	KInventoryItemInfo kChanged;

	if( false == pInven->SetEnchantLevel( kReq.m_iDestItemUID, iAttachEnchantLevel, kChanged ) )
	{
		kAck.m_iOK = NetError::ERR_ATTACH_ITEM_00;
		return Reply( kSes, EGS_ENCHANT_ATTACH_ITEM_ACK, kAck );
	}

	kAck.m_vecKInventorySlotInfo.push_back( kChanged );

	// 11. Spend the ticket. SERV_ENCHANT_ATTACH_MODIFY: a stackable ticket
	// loses one, a non-stackable one is deleted whole. DeleteItem does both -
	// a quantity of 1 against a single-item row removes the row.
	KInventoryItemInfo kSpent;

	if( true == pInven->DeleteItem( kReq.m_iAttachItemUID, 1, kSpent ) )
		kAck.m_vecKInventorySlotInfo.push_back( kSpent );

	kAck.m_iOK = NetError::NET_OK;

	CX2OfflineLog::Server( L"ITEM     amulet %d set item %d to +%d (was +%d)",
		kAttach.m_iItemID, kDest.m_iItemID, iAttachEnchantLevel, kDest.m_iEnchantLevel );

	Reply( kSes, EGS_ENCHANT_ATTACH_ITEM_ACK, kAck );

	// The amulet reaches a level the same way an enhancement does, so it feeds
	// the same quest hook. The real server does not do this - its
	// DBE_ENCHANT_ATTACH_ITEM_ACK has no Handler_OnEnchantItem call, only the
	// enhancement path does - so it is NOT done here either; a step that asks
	// the player to enhance an item is not satisfied by buying the level.

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Phase 26: item attributes (ISSUES.md #10). Two packets, both previously on
// the ignore list and therefore both hanging their dialog:
//
//   EGS_ATTRIB_ENCHANT_ITEM_REQ  - the El shard NPC. Put one attribute into
//                                  one slot, or take one out. Costs ED and
//                                  shards; an El shard [Unknown] rolls.
//   EGS_ATTRIB_ATTACH_ITEM_REQ   - the attribute amulet. Sets all three slots
//                                  at once, deterministically, for free.
//
// Most of this is client-side already and is used rather than re-ported:
// CX2EnchantItem::GetAttribEnchantRequireMagicStoneCount and
// ...RequireED are AttribEnchantRequire.lua, which the client loads itself;
// CanEnchantAttribute is CXSLAttribEnchantItem::IsPossibleToPush; GetItemID is
// GetRequireItemID. Only the random tables are server-only, and those are
// CX2OfflineAttribTable's job.

namespace
{
	/// CXSLAttribEnchantItem::GetAttribCountType (XSLAttribEnchantItem.cpp:304),
	/// which counts NON-EMPTY slots rather than reading them in order - an item
	/// with slot 0 empty and slot 1 filled is ACT_SINGLE, not ACT_NONE.
	int AttribCountType( int iAttrib0, int iAttrib1, int iAttrib2 )
	{
		int iCount = 0;

		if( 0 != iAttrib0 )	++iCount;
		if( 0 != iAttrib1 )	++iCount;
		if( 0 != iAttrib2 )	++iCount;

		switch( iCount )
		{
		case 0:		return CX2EnchantItem::ACT_NONE;
		case 1:		return CX2EnchantItem::ACT_SINGLE;
		case 2:		return CX2EnchantItem::ACT_DUAL;
		}

		return CX2EnchantItem::ACT_TRIPLE;
	}

	/// CXSLAttribEnchantItem::GetRequireItemID - which El shard a given
	/// attribute is bought with. The client's CX2EnchantItem::GetItemID is the
	/// same table but has no ET_NONE case, and ET_NONE (removing an attribute)
	/// costs no shard at all, so this keeps the server's shape: 0 for ET_NONE,
	/// -1 for a value that is not an attribute.
	int AttribRequireItemID( char cAttribEnchantType )
	{
		switch( cAttribEnchantType )
		{
		case CX2EnchantItem::ET_NONE:	return 0;
		case CX2EnchantItem::ET_BLAZE:	return CX2EnchantItem::ATI_RED;
		case CX2EnchantItem::ET_WATER:	return CX2EnchantItem::ATI_BLUE;
		case CX2EnchantItem::ET_NATURE:	return CX2EnchantItem::ATI_GREEN;
		case CX2EnchantItem::ET_WIND:	return CX2EnchantItem::ATI_WIND;
		case CX2EnchantItem::ET_LIGHT:	return CX2EnchantItem::ATI_LIGHT;
		case CX2EnchantItem::ET_DARK:	return CX2EnchantItem::ATI_DARK;
		case CX2EnchantItem::ET_RANDOM:	return CX2EnchantItem::ATI_UNKNOWN;
		}

		return -1;
	}

	/// CXSLAttribEnchantItem::VerifyAttribEnchantType.
	bool IsAttribEnchantType( char cAttribEnchantType )
	{
		switch( cAttribEnchantType )
		{
		case CX2EnchantItem::ET_NONE:
		case CX2EnchantItem::ET_BLAZE:
		case CX2EnchantItem::ET_WATER:
		case CX2EnchantItem::ET_NATURE:
		case CX2EnchantItem::ET_WIND:
		case CX2EnchantItem::ET_LIGHT:
		case CX2EnchantItem::ET_DARK:
		case CX2EnchantItem::ET_RANDOM:
			return true;
		}

		return false;
	}

	/// CXSLAttribEnchantItem::IsPossibleToPush - the two exclusive triads.
	/// CX2EnchantItem::CanEnchantAttribute is the client's own copy of the same
	/// rule but takes a whole ItemEnchantedAttribute; this takes the pair the
	/// server's version does, which is what the caller has.
	bool IsPossibleToPush( char cExistingType, char cTypeToAdd )
	{
		if( CX2EnchantItem::ET_NONE == cExistingType || CX2EnchantItem::ET_NONE == cTypeToAdd )
			return false;

		switch( cExistingType )
		{
		case CX2EnchantItem::ET_BLAZE:
			return ( CX2EnchantItem::ET_WATER != cTypeToAdd &&
					 CX2EnchantItem::ET_NATURE != cTypeToAdd );

		case CX2EnchantItem::ET_WATER:
			return ( CX2EnchantItem::ET_BLAZE != cTypeToAdd &&
					 CX2EnchantItem::ET_NATURE != cTypeToAdd );

		case CX2EnchantItem::ET_NATURE:
			return ( CX2EnchantItem::ET_BLAZE != cTypeToAdd &&
					 CX2EnchantItem::ET_WATER != cTypeToAdd );

		case CX2EnchantItem::ET_WIND:
			return ( CX2EnchantItem::ET_LIGHT != cTypeToAdd &&
					 CX2EnchantItem::ET_DARK != cTypeToAdd );

		case CX2EnchantItem::ET_LIGHT:
			return ( CX2EnchantItem::ET_WIND != cTypeToAdd &&
					 CX2EnchantItem::ET_DARK != cTypeToAdd );

		case CX2EnchantItem::ET_DARK:
			return ( CX2EnchantItem::ET_WIND != cTypeToAdd &&
					 CX2EnchantItem::ET_LIGHT != cTypeToAdd );
		}

		return true;
	}
}

bool CX2OfflineServer::Handler_EGS_ATTRIB_ENCHANT_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_ATTRIB_ENCHANT_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineInventory*	pInven	= CX2OfflineInventory::Instance();
	CX2OfflineDB*			pDB		= CX2OfflineDB::Instance();
	CX2OfflineAttribTable*	pTable	= CX2OfflineAttribTable::Instance();

	KEGS_ATTRIB_ENCHANT_ITEM_ACK kAck;
	kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_00;

	if( NULL == g_pData || NULL == g_pData->GetEnchantItem() )
		return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );

	CX2EnchantItem* pClientEnchant = g_pData->GetEnchantItem();

	char cAttribEnchantType = kReq.m_cAttribEnchantID;

	// A random request needs the packed lottery; a specific one does not, so
	// the data gate is only applied where it actually bites.
	if( CX2EnchantItem::ET_RANDOM == cAttribEnchantType && false == pTable->IsLoaded() )
	{
		CX2OfflineLog::Server( L"ITEM     refused a random attribute - AttribEnchantTable.lua is not"
			L" loaded. XOR-encrypt KncWX2Server/ServerResource/US/AttribEnchantTable.lua and pack"
			L" it into data036.kom." );

		kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_01;
		return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
	}

	//////////////////////////////////////////////////////////////////////////
	// KInventory::AttribEnchantItem's gates, in its order.

	KOfflineItemRow kRow;
	if( false == pInven->GetItemRow( kReq.m_iItemUID, kRow ) )
		return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );

	if( CX2Inventory::ST_BANK == kRow.m_iCategory ||
		CX2Inventory::ST_SHARE_BANK == kRow.m_iCategory )
	{
		kAck.m_iOK = NetError::ERR_BANK_02;
		return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
	}

	if( kRow.m_iEnchantLevel < 0 )
	{
		kAck.m_iOK = NetError::ERR_RESTORE_ITEM_06;		///< broken item
		return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
	}

	if( false == IsAttribEnchantType( cAttribEnchantType ) )
	{
		kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_01;
		return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
	}

	const int iRequireItemID = AttribRequireItemID( cAttribEnchantType );
	if( iRequireItemID < 0 )
	{
		kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_01;
		return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
	}

	//////////////////////////////////////////////////////////////////////////
	// CXSLAttribEnchantItem::IsPossibleAttribEnchant, which is also where
	// ET_RANDOM stops being random: it is resolved into a real attribute here,
	// against the slots that are already filled, before anything is charged.

	const int eCountType = AttribCountType( kRow.m_iAttrib0, kRow.m_iAttrib1, kRow.m_iAttrib2 );

	if( CX2EnchantItem::ET_NONE != cAttribEnchantType )
	{
		// Which slot, and what is in the other two.
		char cFirstAttribType	= 0;
		char cSecondAttribType	= 0;

		switch( kReq.m_cAttribEnchantSlotNo )
		{
		case CX2EnchantItem::ESI_SLOT_1:
			if( 0 != kRow.m_iAttrib0 )
			{
				kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_02;		///< slot already taken
				return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
			}
			cFirstAttribType	= (char)kRow.m_iAttrib1;
			cSecondAttribType	= (char)kRow.m_iAttrib2;
			break;

		case CX2EnchantItem::ESI_SLOT_2:
			if( 0 != kRow.m_iAttrib1 )
			{
				kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_02;
				return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
			}
			cFirstAttribType	= (char)kRow.m_iAttrib0;
			cSecondAttribType	= (char)kRow.m_iAttrib2;
			break;

		case CX2EnchantItem::ESI_SLOT_3:
			if( 0 != kRow.m_iAttrib2 )
			{
				kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_02;
				return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
			}
			cFirstAttribType	= (char)kRow.m_iAttrib0;
			cSecondAttribType	= (char)kRow.m_iAttrib1;
			break;

		default:
			kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_01;
			return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
		}

		switch( eCountType )
		{
		case CX2EnchantItem::ACT_NONE:
			if( CX2EnchantItem::ET_RANDOM == cAttribEnchantType )
			{
				cAttribEnchantType = pTable->GetRandomAttribResult( eCountType, 0, 0 );
			}
			break;

		case CX2EnchantItem::ACT_SINGLE:
			{
				// Exactly one of the other two slots holds something; the real
				// server reads whichever it is, in that order.
				const char cExisting = ( 0 != cFirstAttribType ) ? cFirstAttribType
																 : cSecondAttribType;

				if( 0 == cExisting )
				{
					kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_01;
					return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
				}

				if( CX2EnchantItem::ET_RANDOM == cAttribEnchantType )
					cAttribEnchantType = pTable->GetRandomAttribResult( eCountType, cExisting, 0 );

				if( false == IsPossibleToPush( cExisting, cAttribEnchantType ) )
				{
					kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_01;
					return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
				}
			}
			break;

		case CX2EnchantItem::ACT_DUAL:
			{
				if( CX2EnchantItem::ET_RANDOM == cAttribEnchantType )
				{
					cAttribEnchantType = pTable->GetRandomAttribResult(
						eCountType, cFirstAttribType, cSecondAttribType );
				}

				// The triple lottery IS the legality table: an attribute that
				// has no case registered for this pair cannot be added, random
				// or chosen. This needs the packed file even when the request
				// was not random, so the gate is applied here rather than up
				// top.
				if( false == pTable->IsLoaded() )
				{
					CX2OfflineLog::Server( L"ITEM     refused a third attribute -"
						L" AttribEnchantTable.lua is not loaded." );

					kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_01;
					return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
				}

				if( false == pTable->IsExistTripleCase( cFirstAttribType, cSecondAttribType,
														cAttribEnchantType ) )
				{
					kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_01;
					return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
				}
			}
			break;

		default:
			// Already triple - nothing more fits.
			kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_01;
			return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
		}

		// A roll that found no row comes back ET_NONE, which would silently
		// turn "add an attribute" into "clear the slot".
		if( CX2EnchantItem::ET_NONE == cAttribEnchantType )
		{
			kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_01;
			return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
		}
	}
	else
	{
		// Removing: the named slot has to hold something.
		int iExisting = 0;

		switch( kReq.m_cAttribEnchantSlotNo )
		{
		case CX2EnchantItem::ESI_SLOT_1:	iExisting = kRow.m_iAttrib0;	break;
		case CX2EnchantItem::ESI_SLOT_2:	iExisting = kRow.m_iAttrib1;	break;
		case CX2EnchantItem::ESI_SLOT_3:	iExisting = kRow.m_iAttrib2;	break;

		default:
			kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_01;
			return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
		}

		if( 0 == iExisting )
		{
			kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_03;		///< nothing to remove
			return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Weapon or armour, and the cost.

	const CX2Item::ItemTemplet* pTemplet = CX2OfflineInventory::Templet( kRow.m_iItemID );
	if( NULL == pTemplet )
	{
		kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_06;
		return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
	}

	bool bWeapon = false;

	if( CX2Item::IT_WEAPON == pTemplet->GetItemType() )
	{
		bWeapon = true;
	}
	else if( CX2Item::IT_DEFENCE != pTemplet->GetItemType() )
	{
		kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_07;
		return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
	}

	KOfflineUnitRow kUnit;
	if( false == pDB->LoadUnit( kSes.m_nSelectedUnitUID, kUnit ) )
		return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );

	int iRequireQuantity	= 0;
	int iCost				= 0;

	// The shard count and the ED both come from AttribEnchantRequire.lua, which
	// the CLIENT loads - CX2EnchantItem parses it at start-up, so these two
	// calls are the same table the real server reads, not a re-derivation.
	// Both take the count of attributes ALREADY on the item, which is
	// eCountType, and CX2EnchantItem's ACT_* values are the server's.
	if( CX2EnchantItem::ET_NONE != cAttribEnchantType )
	{
		if( false == pClientEnchant->GetAttribEnchantRequireMagicStoneCount(
				bWeapon, eCountType, pTemplet->GetUseLevel(),
				pTemplet->GetItemGrade(), iRequireQuantity ) )
		{
			kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_01;
			return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
		}

		if( iRequireItemID > 0 &&
			pInven->CountItemByID( iRequireItemID, true ) < iRequireQuantity )
		{
			kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_05;		///< not enough El shards
			return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
		}
	}

	if( false == pClientEnchant->GetAttribEnchantRequireED(
			bWeapon, eCountType, pTemplet->GetUseLevel(), pTemplet->GetItemGrade(), iCost ) )
	{
		kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_01;
		return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
	}

	if( kUnit.m_iED < iCost )
	{
		// The insert and remove paths report a short wallet with different
		// codes, so the dialog can name which operation could not be paid for.
		kAck.m_iOK = ( CX2EnchantItem::ET_NONE != cAttribEnchantType )
					 ? NetError::ERR_ATTRIB_ENCHANT_04
					 : NetError::ERR_ATTRIB_ENCHANT_08;

		return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
	}

	//////////////////////////////////////////////////////////////////////////
	// Apply.

	KInventoryItemInfo kChanged;

	if( false == pInven->SetAttribEnchant( kReq.m_iItemUID, (int)kReq.m_cAttribEnchantSlotNo,
										   cAttribEnchantType, kChanged ) )
	{
		kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_00;
		return Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );
	}

	if( CX2EnchantItem::ET_NONE != cAttribEnchantType &&
		iRequireItemID > 0 && iRequireQuantity > 0 )
	{
		pInven->ConsumeByID( iRequireItemID, iRequireQuantity, kAck.m_vecInventorySlotInfo );
	}

	kAck.m_vecInventorySlotInfo.push_back( kChanged );

	kUnit.m_iED -= iCost;
	pDB->SaveProgress( kUnit.m_nUnitUID, kUnit.m_iLevel, kUnit.m_iEXP, kUnit.m_iED );

	kAck.m_iOK = NetError::NET_OK;

	CX2OfflineLog::Server( L"ITEM     attribute slot %d of item %d set to %d"
		L" (%d shard(s) of %d, %d ED, %d ED left)",
		(int)kReq.m_cAttribEnchantSlotNo, kRow.m_iItemID, (int)cAttribEnchantType,
		iRequireQuantity, iRequireItemID, iCost, kUnit.m_iED );

	Reply( kSes, EGS_ATTRIB_ENCHANT_ITEM_ACK, kAck );

	// SQT_ITEM_ATTRIB is not driven from here. The offline quest census already
	// says why and it is still true: the clear type is parsed but this build
	// has no reachable sub-quest that uses it, and the real server's own hook
	// sits behind a flag combination this configuration does not compile.

	return true;
}

bool CX2OfflineServer::Handler_EGS_ATTRIB_ATTACH_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_ATTRIB_ATTACH_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineInventory*	pInven	= CX2OfflineInventory::Instance();
	CX2OfflineAttribTable*	pTable	= CX2OfflineAttribTable::Instance();

	KEGS_ATTRIB_ATTACH_ITEM_ACK kAck;
	kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_00;

	if( false == pTable->IsLoaded() )
	{
		CX2OfflineLog::Server( L"ITEM     refused an attribute amulet - the attribute tables are"
			L" not loaded. XOR-encrypt KncWX2Server/ServerResource/US/AttribEnchantTable.lua and"
			L" AttribAttachTable.lua and pack them into data036.kom." );

		kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_01;
		return Reply( kSes, EGS_ATTRIB_ATTACH_ITEM_ACK, kAck );
	}

	KOfflineItemRow kDest;
	KOfflineItemRow kAttach;

	if( false == pInven->GetItemRow( kReq.m_iDestItemUID, kDest ) ||
		false == pInven->GetItemRow( kReq.m_iAttachItemUID, kAttach ) )
	{
		return Reply( kSes, EGS_ATTRIB_ATTACH_ITEM_ACK, kAck );
	}

	if( CX2Inventory::ST_BANK == kDest.m_iCategory ||
		CX2Inventory::ST_SHARE_BANK == kDest.m_iCategory ||
		CX2Inventory::ST_BANK == kAttach.m_iCategory ||
		CX2Inventory::ST_SHARE_BANK == kAttach.m_iCategory )
	{
		kAck.m_iOK = NetError::ERR_BANK_02;
		return Reply( kSes, EGS_ATTRIB_ATTACH_ITEM_ACK, kAck );
	}

	if( kDest.m_iEnchantLevel < 0 )
	{
		kAck.m_iOK = NetError::ERR_RESTORE_ITEM_06;
		return Reply( kSes, EGS_ATTRIB_ATTACH_ITEM_ACK, kAck );
	}

	const CX2Item::ItemTemplet* pTemplet = CX2OfflineInventory::Templet( kDest.m_iItemID );
	if( NULL == pTemplet )
	{
		kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_06;
		return Reply( kSes, EGS_ATTRIB_ATTACH_ITEM_ACK, kAck );
	}

	if( CX2Item::IT_WEAPON != pTemplet->GetItemType() &&
		CX2Item::IT_DEFENCE != pTemplet->GetItemType() )
	{
		kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_07;
		return Reply( kSes, EGS_ATTRIB_ATTACH_ITEM_ACK, kAck );
	}

	char cAttrib0 = 0;
	char cAttrib1 = 0;
	char cAttrib2 = 0;

	if( false == pTable->GetAttribAttachInfo( kAttach.m_iItemID, cAttrib0, cAttrib1, cAttrib2 ) )
	{
		CX2OfflineLog::Server( L"ITEM     amulet refused: item %d is not an attribute amulet",
			kAttach.m_iItemID );

		kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_01;
		return Reply( kSes, EGS_ATTRIB_ATTACH_ITEM_ACK, kAck );
	}

	// The item has to have room for as many attributes as the amulet carries.
	int iAmuletCount = 0;
	if( 0 != cAttrib0 )	++iAmuletCount;
	if( 0 != cAttrib1 )	++iAmuletCount;
	if( 0 != cAttrib2 )	++iAmuletCount;

	if( iAmuletCount > pTemplet->GetMaxAttribEnchantCount() )
	{
		kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_01;
		return Reply( kSes, EGS_ATTRIB_ATTACH_ITEM_ACK, kAck );
	}

	//////////////////////////////////////////////////////////////////////////
	// Apply all three slots. The amulet OVERWRITES - it writes every slot,
	// including the empty ones, so an item with attributes already on it comes
	// out carrying exactly what the amulet says and nothing else. That is the
	// real server's loop, which walks ESI_SLOT_1..ESI_SLOT_MAX unconditionally.

	KInventoryItemInfo kChanged;
	bool bWrote = false;

	for( int iSlotID = CX2EnchantItem::ESI_SLOT_1; iSlotID <= CX2EnchantItem::ESI_SLOT_3; ++iSlotID )
	{
		char cType = 0;

		switch( iSlotID )
		{
		case CX2EnchantItem::ESI_SLOT_1:	cType = cAttrib0;	break;
		case CX2EnchantItem::ESI_SLOT_2:	cType = cAttrib1;	break;
		case CX2EnchantItem::ESI_SLOT_3:	cType = cAttrib2;	break;
		}

		if( true == pInven->SetAttribEnchant( kReq.m_iDestItemUID, iSlotID, cType, kChanged ) )
			bWrote = true;
	}

	if( false == bWrote )
	{
		kAck.m_iOK = NetError::ERR_ATTRIB_ENCHANT_00;
		return Reply( kSes, EGS_ATTRIB_ATTACH_ITEM_ACK, kAck );
	}

	// kChanged is the last write's slot info, which carries all three
	// attributes - each SetAttribEnchant rebuilds it from the whole row.
	kAck.m_vecInventorySlotInfo.push_back( kChanged );

	KInventoryItemInfo kSpent;
	if( true == pInven->DeleteItem( kReq.m_iAttachItemUID, 1, kSpent ) )
		kAck.m_vecInventorySlotInfo.push_back( kSpent );

	kAck.m_iOK = NetError::NET_OK;

	CX2OfflineLog::Server( L"ITEM     attribute amulet %d set item %d to %d/%d/%d",
		kAttach.m_iItemID, kDest.m_iItemID, (int)cAttrib0, (int)cAttrib1, (int)cAttrib2 );

	return Reply( kSes, EGS_ATTRIB_ATTACH_ITEM_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// Phase 24: EGS_SOCKET_ITEM_REQ (ISSUES.md #3) was the twin of the phase-23
// enhancement refusal, and had the same shape of answer: the option tables ARE
// server data, and they are in the tree - SocketItemTable.lua. This is
// KInventory::SocketItem (Inventory.cpp:13486) plus its KGSUser wrapper
// (GSUserInventory.cpp:3437) and the ACK that DBE_SOCKET_ITEM_ACK sends (:3571).
//
// One request can fill several slots at once - m_mapSocketInfo is
// slot index -> magic stone item UID - and the real server prices, validates
// and then applies all of them, so a request that cannot be paid for changes
// nothing at all. That two-pass shape is kept.

bool CX2OfflineServer::Handler_EGS_SOCKET_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_SOCKET_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineInventory*	pInven	= CX2OfflineInventory::Instance();
	CX2OfflineDB*			pDB		= CX2OfflineDB::Instance();
	CX2OfflineSocketTable*	pTable	= CX2OfflineSocketTable::Instance();

	KEGS_SOCKET_ITEM_ACK kAck;
	kAck.m_iOK		= NetError::ERR_SOCKET_ITEM_00;
	kAck.m_iED		= 0;
	kAck.m_iItemUID	= 0;

	if( false == pTable->IsLoaded() )
	{
		CX2OfflineLog::Server( L"ITEM     refused a socketing - SocketItemTable.lua is not loaded."
			L" XOR-encrypt KncWX2Server/ServerResource/US/SocketItemTable.lua and pack it into"
			L" data036.kom; the SOCKET lines above say what went wrong." );

		kAck.m_iOK = NetError::ERR_SOCKET_ITEM_01;
		return Reply( kSes, EGS_SOCKET_ITEM_ACK, kAck );
	}

	// The GM socket-option cheat is not honoured offline. On the real server it
	// writes chosen option IDs straight into the item and is gated on
	// UAL_GM; there is no operator here, and the request carries the option IDs
	// themselves, so honouring it would let a modified client write arbitrary
	// socket options. Refused, and it says so rather than silently rolling.
	if( true == kReq.m_bCheat )
	{
		CX2OfflineLog::Server( L"ITEM     refused a socket CHEAT request - offline has no GM" );

		kAck.m_iOK = NetError::ERR_VERIFY_12;
		return Reply( kSes, EGS_SOCKET_ITEM_ACK, kAck );
	}

	//////////////////////////////////////////////////////////////////////////
	// KInventory::SocketItem's gates on the target, in its order.

	KOfflineItemRow kRow;
	if( false == pInven->GetItemRow( kReq.m_iItemUID, kRow ) )
		return Reply( kSes, EGS_SOCKET_ITEM_ACK, kAck );

	if( CX2Inventory::ST_BANK == kRow.m_iCategory ||
		CX2Inventory::ST_SHARE_BANK == kRow.m_iCategory )
	{
		kAck.m_iOK = NetError::ERR_BANK_01;
		return Reply( kSes, EGS_SOCKET_ITEM_ACK, kAck );
	}

	if( kRow.m_iEnchantLevel < 0 )
	{
		kAck.m_iOK = NetError::ERR_RESTORE_ITEM_06;		///< broken item
		return Reply( kSes, EGS_SOCKET_ITEM_ACK, kAck );
	}

	const CX2Item::ItemTemplet* pTemplet = CX2OfflineInventory::Templet( kRow.m_iItemID );
	if( NULL == pTemplet )
	{
		kAck.m_iOK = NetError::ERR_SOCKET_ITEM_01;
		return Reply( kSes, EGS_SOCKET_ITEM_ACK, kAck );
	}

	if( true == pTemplet->GetNoEquip() )
	{
		kAck.m_iOK = NetError::ERR_SOCKET_ITEM_06;
		return Reply( kSes, EGS_SOCKET_ITEM_ACK, kAck );
	}

	if( true == kReq.m_mapSocketInfo.empty() )
	{
		kAck.m_iOK = NetError::ERR_SOCKET_ITEM_03;
		return Reply( kSes, EGS_SOCKET_ITEM_ACK, kAck );
	}

	KOfflineUnitRow kUnit;
	if( false == pDB->LoadUnit( kSes.m_nSelectedUnitUID, kUnit ) )
		return Reply( kSes, EGS_SOCKET_ITEM_ACK, kAck );

	//////////////////////////////////////////////////////////////////////////
	// First pass: price every insert and count how many of each stone the
	// request needs, without touching anything.

	const int iAssignedSocketCount = pInven->GetCountAssignedItemSocket( kReq.m_iItemUID );

	int iTotalCost = 0;

	std::map< UidType, int > mapRequireStone;		///< stone item UID -> how many

	std::map< int, UidType >::const_iterator mitSI;
	for( mitSI = kReq.m_mapSocketInfo.begin(); mitSI != kReq.m_mapSocketInfo.end(); ++mitSI )
	{
		const int		iSocketSlotID	= mitSI->first;
		const UidType	nMagicStoneUID	= mitSI->second;

		++mapRequireStone[ nMagicStoneUID ];

		const int iMagicStoneItemID = pInven->GetItemID( nMagicStoneUID );

		if( false == pTable->IsMagicStoneItemID( iMagicStoneItemID ) )
		{
			CX2OfflineLog::Server( L"ITEM     socketing refused: item %d is not a magic stone",
				iMagicStoneItemID );

			kAck.m_iOK = NetError::ERR_SOCKET_ITEM_03;
			return Reply( kSes, EGS_SOCKET_ITEM_ACK, kAck );
		}

		int iSocketOptionID = 0;
		if( false == pInven->GetItemSocketOption( kReq.m_iItemUID, iSocketSlotID, iSocketOptionID ) )
		{
			kAck.m_iOK = NetError::ERR_SOCKET_ITEM_00;
			return Reply( kSes, EGS_SOCKET_ITEM_ACK, kAck );
		}

		const bool bIsEmptySlot = ( 0 == iSocketOptionID );

		int iCost = 0;
		if( false == pTable->CalcInsertSocketCost( kRow.m_iItemID, iMagicStoneItemID,
												   iAssignedSocketCount, bIsEmptySlot, iCost ) )
		{
			kAck.m_iOK = NetError::ERR_SOCKET_ITEM_03;
			return Reply( kSes, EGS_SOCKET_ITEM_ACK, kAck );
		}

		iTotalCost += iCost;
	}

	if( kUnit.m_iED < iTotalCost )
	{
		kAck.m_iOK = NetError::ERR_SOCKET_ITEM_02;
		return Reply( kSes, EGS_SOCKET_ITEM_ACK, kAck );
	}

	// Every stone the request names has to actually be there, in the quantity
	// the request uses it - checked before the first one is spent.
	std::map< UidType, int >::const_iterator mitRS;
	for( mitRS = mapRequireStone.begin(); mitRS != mapRequireStone.end(); ++mitRS )
	{
		KOfflineItemRow kStone;

		if( false == pInven->GetItemRow( mitRS->first, kStone ) ||
			kStone.m_iQuantity < mitRS->second )
		{
			CX2OfflineLog::Server( L"ITEM     socketing refused: not enough of magic stone %I64d",
				(__int64)mitRS->first );

			kAck.m_iOK = NetError::ERR_SOCKET_ITEM_03;
			return Reply( kSes, EGS_SOCKET_ITEM_ACK, kAck );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Second pass: roll and apply.

	KInventoryItemInfo kChanged;
	bool bAnyWritten = false;

	for( mitSI = kReq.m_mapSocketInfo.begin(); mitSI != kReq.m_mapSocketInfo.end(); ++mitSI )
	{
		const int		iSocketSlotID	= mitSI->first;
		const int		iMagicStoneItemID = pInven->GetItemID( mitSI->second );

		int iSocketResult = 0;

		if( false == pTable->GetResultItem( iMagicStoneItemID, (int)pTemplet->GetItemType(),
											pTemplet->GetUseLevel(), pTemplet->GetFashion(),
											kRow.m_iEnchantLevel, iSocketResult ) )
		{
			// Mid-request failure. The real server returns here too, leaving
			// whatever it already wrote in place; the ED has not been charged
			// yet either way, so the honest thing is to stop and say so.
			CX2OfflineLog::Server( L"ITEM     socketing stopped at slot %d: no option rolled"
				L" for stone %d", iSocketSlotID, iMagicStoneItemID );

			kAck.m_iOK = NetError::ERR_SOCKET_ITEM_01;
			kAck.m_vecInventorySlotInfo.clear();
			return Reply( kSes, EGS_SOCKET_ITEM_ACK, kAck );
		}

		if( false == pInven->SetItemSocketOption( kReq.m_iItemUID, iSocketSlotID,
												  iSocketResult, kChanged ) )
		{
			kAck.m_iOK = NetError::ERR_SOCKET_ITEM_06;
			kAck.m_vecInventorySlotInfo.clear();
			return Reply( kSes, EGS_SOCKET_ITEM_ACK, kAck );
		}

		bAnyWritten = true;

		CX2OfflineLog::Server( L"ITEM     socket slot %d of item %d <- option %d (stone %d)",
			iSocketSlotID, kRow.m_iItemID, iSocketResult, iMagicStoneItemID );
	}

	if( false == bAnyWritten )
	{
		kAck.m_iOK = NetError::ERR_SOCKET_ITEM_06;
		return Reply( kSes, EGS_SOCKET_ITEM_ACK, kAck );
	}

	// kChanged is the last write's slot info and carries the whole socket list,
	// because SetItemSocketOption rebuilds it from the row every time.
	kAck.m_vecInventorySlotInfo.push_back( kChanged );

	// Spend the stones.
	for( mitRS = mapRequireStone.begin(); mitRS != mapRequireStone.end(); ++mitRS )
	{
		KInventoryItemInfo kSpent;

		if( true == pInven->DeleteItem( mitRS->first, mitRS->second, kSpent ) )
			kAck.m_vecInventorySlotInfo.push_back( kSpent );
	}

	kUnit.m_iED -= iTotalCost;
	pDB->SaveProgress( kUnit.m_nUnitUID, kUnit.m_iLevel, kUnit.m_iEXP, kUnit.m_iED );

	kAck.m_iOK		= NetError::NET_OK;
	kAck.m_iED		= kUnit.m_iED;
	kAck.m_iItemUID	= kReq.m_iItemUID;

	CX2OfflineLog::Server( L"ITEM     socketed item %d: %u slot(s) for %d ED, %d ED left",
		kRow.m_iItemID, (unsigned int)kReq.m_mapSocketInfo.size(), iTotalCost, kUnit.m_iED );

	Reply( kSes, EGS_SOCKET_ITEM_ACK, kAck );

	//////////////////////////////////////////////////////////////////////////
	// The quest hook, the socket twin of phase 23's. The real server calls it
	// from DBE_SOCKET_ITEM_ACK (GSUserInventory.cpp:3566) with the number of
	// slots this one request filled, not the item's running total.

	KOfflineUnitRow kQuestUnit;
	if( true == LoadQuestState( kSes, kQuestUnit ) )
	{
		std::vector< KQuestInstance > vecChanged;

		CX2OfflineQuest::Instance()->OnSocketItem( kRow.m_iItemID,
			(int)kReq.m_mapSocketInfo.size(), kQuestUnit, vecChanged );

		PushQuestUpdate( kSes, vecChanged );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Phase 12: EGS_RESOLVE_ITEM_REQ (dismantle) used to sit on
// X2OfflineIgnore.cpp's ignore list, which sends no reply and leaves the
// dialog hanging. ResolveTable.lua is packed into data036.kom, so this ports
// the real yield algorithm - KInventory::ResolveItem (Inventory.cpp:11799)
// plus CXSLResolveItemManager::GetResultItem - rather than refusing; see
// X2OfflineResolveTable.h for exactly which pieces live where.

bool CX2OfflineServer::Handler_EGS_RESOLVE_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_RESOLVE_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineInventory*	 pInven = CX2OfflineInventory::Instance();
	CX2OfflineResolveTable* pTable = CX2OfflineResolveTable::Instance();

	KEGS_RESOLVE_ITEM_ACK kAck;
	kAck.m_iOK		= NetError::ERR_RESOLVE_ITEM_00;	///< no such item - KInventory::ResolveItem's own default
	kAck.m_bJackpot	= false;
#ifdef SERV_MULTI_RESOLVE
	kAck.m_iED = 0;
#endif SERV_MULTI_RESOLVE

	KOfflineItemRow kRow;
	if( false == pInven->GetItemRow( kReq.m_iItemUID, kRow ) )
		return Reply( kSes, EGS_RESOLVE_ITEM_ACK, kAck );

	const CX2Item::ItemTemplet* pTemplet = CX2OfflineInventory::Templet( kRow.m_iItemID );
	if( NULL == pTemplet )
	{
		kAck.m_iOK = NetError::ERR_RESOLVE_ITEM_01;
		return Reply( kSes, EGS_RESOLVE_ITEM_ACK, kAck );
	}

	// KInventory::ResolveItem's type gate (Inventory.cpp:11884-11894).
	if( CX2Item::IT_WEAPON != pTemplet->GetItemType() &&
		CX2Item::IT_DEFENCE != pTemplet->GetItemType() &&
		CX2Item::IT_ACCESSORY != pTemplet->GetItemType() )
	{
		kAck.m_iOK = NetError::ERR_RESOLVE_ITEM_03;
		return Reply( kSes, EGS_RESOLVE_ITEM_ACK, kAck );
	}

	// Fashion (avatar/costume) dismantle is a different path on the real
	// server - it prices the item from CXSLCashItemManager's cash-item price
	// table (GetCashItemPriceFromScript, Inventory.cpp:11939) and pays out a
	// fixed "unknown attribute stone" instead of anything from
	// ResolveTable.lua. That table is not this phase's; refuse honestly rather
	// than guess a price. Almost every real CX2Item::IT_ACCESSORY is fashion,
	// so this is also where a non-dismantleable accessory actually lands.
	if( true == pTemplet->GetFashion() )
	{
		kAck.m_iOK = NetError::ERR_RESOLVE_ITEM_04;
		CX2OfflineLog::Server( L"ITEM     dismantle refused for item %d - it is a fashion/avatar"
			L" piece, priced from the cash-item table (CXSLCashItemManager), not"
			L" ResolveTable.lua; not implemented offline", kRow.m_iItemID );
		return Reply( kSes, EGS_RESOLVE_ITEM_ACK, kAck );
	}

	// KInventory::GetSellPrice, SERV_SELL_ED_ITEM_PRICE_FIX arm - byte for
	// byte the same formula Handler_EGS_SELL_ED_ITEM_REQ already ports above,
	// because dismantle prices its yield the same "1/5 of shop price,
	// prorated by remaining endurance" way the shop-sell handler does.
	int iSellPrice = 0;

	switch( pTemplet->GetPeriodType() )
	{
	case CX2Item::PT_INFINITY:
		iSellPrice = (int)floorf( 0.2f * (float)pTemplet->GetPrice() + 0.01f );
		break;

	case CX2Item::PT_ENDURANCE:
		if( pTemplet->GetEndurance() <= 0 || kRow.m_iEndurance < 0 )
		{
			kAck.m_iOK = NetError::ERR_RESOLVE_ITEM_01;
			return Reply( kSes, EGS_RESOLVE_ITEM_ACK, kAck );
		}
		{
			const float fRate = (float)kRow.m_iEndurance / (float)pTemplet->GetEndurance();
			iSellPrice = (int)floorf( (float)pTemplet->GetPrice() * 0.2f * fRate + 0.01f );
		}
		break;

	case CX2Item::PT_QUANTITY:
		if( pTemplet->GetQuantity() <= 0 )
		{
			kAck.m_iOK = NetError::ERR_RESOLVE_ITEM_01;
			return Reply( kSes, EGS_RESOLVE_ITEM_ACK, kAck );
		}
		iSellPrice = (int)floorf( 0.2f * (float)pTemplet->GetPrice() * (float)kRow.m_iQuantity + 0.01f );
		break;

	default:
		kAck.m_iOK = NetError::ERR_RESOLVE_ITEM_01;
		return Reply( kSes, EGS_RESOLVE_ITEM_ACK, kAck );
	}

	if( iSellPrice < 0 )
		iSellPrice = 0;			///< the server's own correction for a negative result

	// The per-session jackpot gate - GSUserInventory.cpp:2894-2944. See
	// KOfflineSession::m_tNextJackpotAt for why the very first dismantle after
	// this session starts is always an attempt.
	const __int64 tNow = (__int64)::_time64( NULL );
	bool bJackpot = ( tNow >= kSes.m_tNextJackpotAt );

	std::map< int, int > mapResult;
	const int iError = pTable->GetResultItem( (int)pTemplet->GetItemGrade(), pTemplet->GetUseLevel(),
											   (int)pTemplet->GetItemType(), iSellPrice, bJackpot, mapResult );

	if( NetError::NET_OK != iError )
	{
		kAck.m_iOK = iError;
		return Reply( kSes, EGS_RESOLVE_ITEM_ACK, kAck );
	}

	// Space check before touching anything - KInventory::IsEnoughSpaceExist
	// (Inventory.cpp:12043). Checked as independent single-item calls rather
	// than as one combined reservation, so two different new material types
	// each claiming the last free slot in the same category is the one case
	// this can get wrong - a corner rare enough for a bag of a handful of
	// material stacks that it is not worth a bigger check.
	for( std::map< int, int >::const_iterator mit = mapResult.begin(); mit != mapResult.end(); ++mit )
	{
		if( false == pInven->HasRoomFor( mit->first, mit->second ) )
		{
			kAck.m_iOK = NetError::ERR_RESOLVE_ITEM_06;
			return Reply( kSes, EGS_RESOLVE_ITEM_ACK, kAck );
		}
	}

	KInventoryItemInfo kDeletedSlot;
	if( false == pInven->DeleteItem( kReq.m_iItemUID, 0, kDeletedSlot ) )
	{
		kAck.m_iOK = NetError::ERR_RESOLVE_ITEM_04;
		return Reply( kSes, EGS_RESOLVE_ITEM_ACK, kAck );
	}

	kAck.m_vecKInventorySlotInfo.push_back( kDeletedSlot );

	for( std::map< int, int >::const_iterator mit = mapResult.begin(); mit != mapResult.end(); ++mit )
	{
		std::vector< KInventoryItemInfo > vecChanged;
		int iInserted = 0;

		pInven->InsertItem( mit->first, mit->second, 0, vecChanged, iInserted );

		for( size_t i = 0; i < vecChanged.size(); ++i )
			kAck.m_vecKInventorySlotInfo.push_back( vecChanged[i] );

		if( iInserted > 0 )
			kAck.m_mapInsertedItem[ mit->first ] = iInserted;
	}

	if( true == bJackpot )
		kSes.m_tNextJackpotAt = tNow + pTable->GetJackpotTime();

	kAck.m_iOK		= NetError::NET_OK;
	kAck.m_bJackpot	= bJackpot;

	CX2OfflineLog::Server( L"ITEM     dismantled item %d (grade=%d level=%d sellPrice=%d)%s ->"
		L" %u material kind(s)",
		kRow.m_iItemID, (int)pTemplet->GetItemGrade(), pTemplet->GetUseLevel(), iSellPrice,
		( true == bJackpot ) ? L" JACKPOT" : L"", (unsigned int)kAck.m_mapInsertedItem.size() );

	return Reply( kSes, EGS_RESOLVE_ITEM_ACK, kAck );
}

#endif SERV_IRUHADEV_OFFLINE
