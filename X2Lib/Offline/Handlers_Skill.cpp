#include "stdafx.h"
#include "X2OfflineServer.h"

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-02
// Description: Offline mode - phase 5. The skill packets: learning and
//              upgrading, resetting one skill, resetting the whole tree, and
//              assigning skills to the eight equipped slots.
//
//              The rules live in CX2OfflineSkill; this is the packet layer.
//              References are KncWX2Server/GameServer/GSUserGameCommon.cpp
//              (:4466 learn, :4682 reset, :4950 init, :2747 slot) and the
//              client's own X2Lib/X2SkillTree.cpp handlers.
//
//              One decision worth naming, because it is a gameplay change and
//              not an emulation detail: EGS_RESET_SKILL_REQ consumes a skill
//              reset scroll if the character has one and goes ahead without one
//              if it does not. The real server refuses outright
//              (ERR_RESET_SKILL_01). Offline there is currently no way to
//              obtain that scroll - it is a cash-shop item, and the cash shop
//              is phase 7 - so requiring it would make a feature this phase is
//              supposed to deliver impossible to use or test. The log says
//              which of the two happened every time.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineInventory.h"
#include "X2OfflineSkill.h"

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_GET_SKILL_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GET_SKILL_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineDB*		pDB		= CX2OfflineDB::Instance();
	CX2OfflineSkill*	pSkill	= CX2OfflineSkill::Instance();

	KEGS_GET_SKILL_ACK kAck;
	kAck.m_iOK			= NetError::ERR_UNKNOWN;
	kAck.m_iRemainSP	= 0;
	kAck.m_iRemainCSP	= 0;

	KOfflineUnitRow kRow;
	if( false == pDB->LoadUnit( kSes.m_nSelectedUnitUID, kRow ) )
		return Reply( kSes, EGS_GET_SKILL_ACK, kAck );

	kAck.m_iRemainSP = kRow.m_iSP;

	pSkill->Load( kRow.m_nUnitUID );

	// The request is the client's desired end state, skill by skill. It comes
	// back echoed with the before-levels filled in, which is what the client's
	// ACK handler writes into its own tree.
	kAck.m_mapSkillList = kReq.m_mapSkillList;

	int iSPLeft = kRow.m_iSP;

	kAck.m_iOK = pSkill->LearnSkills( kRow.m_iLevel, kRow.m_iUnitClass,
									  kAck.m_mapSkillList, kRow.m_iSP, iSPLeft );

	if( NetError::NET_OK != kAck.m_iOK )
	{
		kAck.m_mapSkillList.clear();
		return Reply( kSes, EGS_GET_SKILL_ACK, kAck );
	}

	pDB->SaveSkillPoint( kRow.m_nUnitUID, iSPLeft, 0 );

	kAck.m_iRemainSP	= iSPLeft;
	kAck.m_iRemainCSP	= 0;

	return Reply( kSes, EGS_GET_SKILL_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_RESET_SKILL_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_RESET_SKILL_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineDB*			pDB		= CX2OfflineDB::Instance();
	CX2OfflineSkill*		pSkill	= CX2OfflineSkill::Instance();
	CX2OfflineInventory*	pInven	= CX2OfflineInventory::Instance();

	KEGS_RESET_SKILL_ACK kAck;
	kAck.m_iOK			= NetError::ERR_RESET_SKILL_00;
	kAck.m_iDelSkillID	= kReq.m_iSkillID;
	kAck.m_iSPoint		= 0;
	kAck.m_iCSPoint		= 0;

	KOfflineUnitRow kRow;
	if( false == pDB->LoadUnit( kSes.m_nSelectedUnitUID, kRow ) )
		return Reply( kSes, EGS_RESET_SKILL_ACK, kAck );

	kAck.m_iSPoint = kRow.m_iSP;

	pSkill->Load( kRow.m_nUnitUID );

	int iRefund = 0;

	kAck.m_iOK = pSkill->ResetSkill( kReq.m_iSkillID, kRow.m_iUnitClass, iRefund );

	if( NetError::NET_OK != kAck.m_iOK )
		return Reply( kSes, EGS_RESET_SKILL_ACK, kAck );

	// The scroll, if there is one. See the file header for why its absence does
	// not stop the reset.
	const UidType nScrollUID = FindItemByID( CX2OfflineSkill::RI_RESET_ONE_SKILL );

	if( 0 != nScrollUID )
	{
		KInventoryItemInfo kSlotInfo;
		if( true == pInven->ConsumeOne( nScrollUID, kSlotInfo ) )
			kAck.m_vecInventorySlotInfo.push_back( kSlotInfo );
	}
	else
	{
		CX2OfflineLog::Server( L"SKILL    no skill reset scroll (item %d) in the bag - the reset is"
			L" allowed anyway offline", (int)CX2OfflineSkill::RI_RESET_ONE_SKILL );
	}

	kRow.m_iSP += iRefund;

	pDB->SaveSkillPoint( kRow.m_nUnitUID, kRow.m_iSP, 0 );

	kAck.m_iSPoint	= kRow.m_iSP;
	kAck.m_iCSPoint	= 0;

	return Reply( kSes, EGS_RESET_SKILL_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_INIT_SKILL_TREE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_INIT_SKILL_TREE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineDB*			pDB		= CX2OfflineDB::Instance();
	CX2OfflineSkill*		pSkill	= CX2OfflineSkill::Instance();
	CX2OfflineInventory*	pInven	= CX2OfflineInventory::Instance();

	KEGS_INIT_SKILL_TREE_ACK kAck;
	kAck.m_iOK		= NetError::ERR_ITEM_04;
	kAck.m_iSPoint	= 0;
	kAck.m_iCSPoint	= 0;

	KOfflineUnitRow kRow;
	if( false == pDB->LoadUnit( kSes.m_nSelectedUnitUID, kRow ) )
		return Reply( kSes, EGS_INIT_SKILL_TREE_ACK, kAck );

	kAck.m_iSPoint = kRow.m_iSP;

	pSkill->Load( kRow.m_nUnitUID );

	// Unlike the single-skill reset, this request *names* the item it is being
	// paid for: the client only sends it when the player uses a whole-tree
	// reset scroll out of the inventory. So the item is required here, exactly
	// as GSUserGameCommon.cpp:4980 requires it - there is nothing to relax.
	KOfflineItemRow kItem;
	if( false == pInven->GetItemRow( kReq.m_iItemUID, kItem ) )
	{
		CX2OfflineLog::Server( L"SKILL    refused a full tree reset - itemUID=%I64d is not in the"
			L" inventory", (__int64)kReq.m_iItemUID );

		return Reply( kSes, EGS_INIT_SKILL_TREE_ACK, kAck );
	}

	int iSPTotal = 0;

	kAck.m_iOK = pSkill->InitSkillTree( kRow.m_iLevel, kRow.m_iUnitClass, iSPTotal );

	if( NetError::NET_OK != kAck.m_iOK )
		return Reply( kSes, EGS_INIT_SKILL_TREE_ACK, kAck );

	KInventoryItemInfo kSlotInfo;
	if( true == pInven->ConsumeOne( kReq.m_iItemUID, kSlotInfo ) )
		kAck.m_vecInventorySlotInfo.push_back( kSlotInfo );

	pDB->SaveSkillPoint( kRow.m_nUnitUID, iSPTotal, 0 );

	kAck.m_iSPoint	= iSPTotal;
	kAck.m_iCSPoint	= 0;

	return Reply( kSes, EGS_INIT_SKILL_TREE_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_CHANGE_SKILL_SLOT_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CHANGE_SKILL_SLOT_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineSkill* pSkill = CX2OfflineSkill::Instance();

	KEGS_CHANGE_SKILL_SLOT_ACK kAck;
	kAck.m_iOK			= NetError::ERR_SKILL_09;
	kAck.m_iSlotID		= kReq.m_iSlotID;
	kAck.m_iSkillID		= kReq.m_iSkillID;
	kAck.m_iSlotID2		= -1;
	kAck.m_iSkillID2	= 0;

	if( 0 == kSes.m_nSelectedUnitUID )
		return Reply( kSes, EGS_CHANGE_SKILL_SLOT_ACK, kAck );

	pSkill->Load( kSes.m_nSelectedUnitUID );

	int iSlotID2	= -1;
	int iSkillID2	= 0;

	kAck.m_iOK = pSkill->ChangeSkillSlot( kReq.m_iSlotID, kReq.m_iSkillID, iSlotID2, iSkillID2 );

	if( NetError::NET_OK != kAck.m_iOK )
		return Reply( kSes, EGS_CHANGE_SKILL_SLOT_ACK, kAck );

	// The server overwrites slot 1 from the request and reports slot 2 from the
	// derived pair, which is the shape the client's ACK handler reads: it calls
	// SetSkillSlotInfo for slot 1 unconditionally and for slot 2 only when
	// m_iSlotID2 is not -1 (X2SkillTree.cpp:1669-1677).
	kAck.m_iSlotID		= kReq.m_iSlotID;
	kAck.m_iSkillID		= kReq.m_iSkillID;
	kAck.m_iSlotID2		= iSlotID2;
	kAck.m_iSkillID2	= iSkillID2;

	// No EGS_CHANGE_SKILL_SLOT_NOT. Its client handler only does anything in
	// XS_PVP_ROOM and only for another player's slot data
	// (X2SkillTree.cpp:1732-1743), so offline it has nothing to update.

	return Reply( kSes, EGS_CHANGE_SKILL_SLOT_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////

UidType CX2OfflineServer::FindItemByID( int iItemID )
{
	CX2OfflineInventory* pInven = CX2OfflineInventory::Instance();

	std::map< UidType, KInventoryItemInfo > mapItem;
	pInven->GetAllItems( mapItem );

	std::map< UidType, KInventoryItemInfo >::const_iterator mit;
	for( mit = mapItem.begin(); mit != mapItem.end(); ++mit )
	{
		if( mit->second.m_kItemInfo.m_iItemID != iItemID )
			continue;

		// Not one that is being worn - a consumable never is, but the guard
		// costs nothing and stops a future caller from eating equipment.
		if( CX2Inventory::ST_E_EQUIP == (int)mit->second.m_cSlotCategory )
			continue;

		return mit->first;
	}

	return 0;
}

#endif SERV_IRUHADEV_OFFLINE
