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
#include "X2OfflineQuest.h"

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

	Reply( kSes, EGS_GET_SKILL_ACK, kAck );

	//////////////////////////////////////////////////////////////////////////
	// Phase 6, round 6: the quest steps that ask the player to open the skill
	// tree and spend a point.
	//
	// This was the whole of the reported bug. Learning a skill worked, the SP
	// came off, the tree persisted - and the quest step never moved, because
	// nothing told the quest engine. On live it is ticked from
	// DBE_INSERT_SKILL_ACK, the *database reply* handler
	// (GSUserGameCommon.cpp:4639), which is a long way from anything
	// quest-shaped and easy to miss when porting the packet rather than the
	// feature.
	//
	// Two separate types come off one request, counted differently:
	//
	//  * USE_SKILL_POINT counts POINTS. One skill taken from level 1 to 3 is
	//    two points, and the server calls its handler once per level in a loop.
	//  * LEARN_NEW_SKILL counts SKILLS, and only ones that were not known
	//    before. The client works that list out itself and sends it in the
	//    request as m_vecNowLearnSkill (X2SkillTree.cpp:3879), so it is used
	//    verbatim rather than recomputed here - same as the server does
	//    (GSUserGameCommon.cpp:4569).
	{
		std::map< int, KGetSkillInfo >::const_iterator mitS;
		for( mitS = kAck.m_mapSkillList.begin(); mitS != kAck.m_mapSkillList.end(); ++mitS )
		{
			const int iGained = mitS->second.m_iSkillLevel - mitS->second.m_iBeforeSkillLevel;

			if( iGained > 0 )
				QuestOnUseSkillPoint( kSes, mitS->first, iGained );
		}

		QuestOnLearnNewSkill( kSes, kReq.m_vecNowLearnSkill );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

/// The dungeons a skill-use quest step is allowed to count in.
///
/// The server excludes four categories - tutorial, Henir, Ruben and the
/// training camp - via CXSLDungeon predicates its own handler calls
/// (GSUserGameCommon.cpp:8846). Transcribed here against the CLIENT's DI_*
/// enum, which is the right target and is not the same list: most of the
/// entries CXSLDungeon names are commented out of X2Lib/X2Dungeon.h, so those
/// dungeons cannot be entered from this build at all and need no case.
///
/// Every name below was checked to be a live enumerator, not merely present in
/// the file - the first attempt at this used grep and matched six commented-out
/// lines, which the compiler then rejected one by one.
static bool IsSkillUseCountedDungeon( int iDungeonID )
{
	switch( iDungeonID )
	{
	// CXSLDungeon::IsTutorialDungeon. Only these two of its fourteen entries
	// survive in this client. The room's own m_bTutorial flag is checked by the
	// caller as well and is the more reliable of the two.
	case CX2Dungeon::DI_TUTORIAL_ELSWORD:
	case CX2Dungeon::DI_BATTLE_FIELD_TUTORIAL_ELSWORD:

	// CXSLDungeon::IsHenirDungeon. The other five Henir spaces are commented
	// out here exactly as they are in the client's own IsHenirDungeon.
	case CX2Dungeon::DI_ELDER_HENIR_SPACE:

	// CXSLDungeon::IsRubenDungeon, less DI_RUBEN_SECRET_HELL, which this client
	// has no enumerator for. The two entries the server itself keeps commented
	// out are left out for the same reason it leaves them out.
	case CX2Dungeon::DI_EL_FOREST_WEST_NORMAL:
	case CX2Dungeon::DI_EL_FOREST_NORTH_NORMAL:
	case CX2Dungeon::DI_EL_FOREST_GATE_NORMAL:
	case CX2Dungeon::DI_EL_FOREST_HELL_NORMAL:
	case CX2Dungeon::DI_EVENT_KIDDAY_RUBEN:
	case CX2Dungeon::DI_MONSTER_TEST_NORMAL:
	case CX2Dungeon::DI_RUBEN_SECRET_COMMON:
	case CX2Dungeon::DI_RUBEN_EL_TREE_NORMAL:
	case CX2Dungeon::DI_RUBEN_RUIN_OF_ELF_NORMAL:
	case CX2Dungeon::DI_RUBEN_SWAMP_NORMAL:
		return false;

	default:
		break;
	}

	// CXSLDungeon::IsTCDungeon - the training camp, one contiguous range from
	// DI_TRAINING_FREE (39000) to DI_TRAINING_RAVEN_6. This client stops its
	// enum at DI_TRAINING_RAVEN_0 (39400), so the upper bound is spelled as
	// that plus six rather than as a bare 39406: the server's enum runs
	// RAVEN_0..RAVEN_6 with no explicit values (XSLDungeon.h), and writing the
	// arithmetic out keeps the derivation visible.
	if( iDungeonID >= (int)CX2Dungeon::DI_TRAINING_FREE &&
		iDungeonID <= (int)CX2Dungeon::DI_TRAINING_RAVEN_0 + 6 )
	{
		return false;
	}

	return true;
}

bool CX2OfflineServer::Handler_EGS_SKILL_USE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_SKILL_USE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// No ACK exists for this one and the client arms no wait on it - it is a
	// fire-and-forget notification that only SQT_SKILL_USE listens to. Handled
	// rather than dropped so the log stops calling it an unknown packet, which
	// matters more than it sounds: it arrives on every skill cast, so an
	// unhandled line for it drowns out everything else in a dungeon run.
	if( -1 == kReq.m_iSkillID )
		return true;

	// Dungeons only, and not the ones the feature is switched off in.
	if( false == m_kRoom.m_bActive || 0 == m_kRoom.m_kInfo.m_iDungeonID )
		return true;

	if( true == m_kRoom.m_bTutorial )
		return true;

	if( false == IsSkillUseCountedDungeon( m_kRoom.m_kInfo.m_iDungeonID ) )
		return true;

	KOfflineUnitRow kRow;
	if( false == LoadQuestState( kSes, kRow ) )
		return true;

	std::vector< KQuestInstance > vecChanged;
	CX2OfflineQuest::Instance()->OnUseSkill( kReq.m_iSkillID, kRow, vecChanged );
	PushQuestUpdate( kSes, vecChanged );

	return true;
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
