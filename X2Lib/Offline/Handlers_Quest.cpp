#include "stdafx.h"
#include "X2OfflineServer.h"

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-03
// Description: Offline mode - phase 6. The quest packets, the title packet,
//              and the game-event bridges the other handler files call into.
//
//              The rules live in CX2OfflineQuest and CX2OfflineTitle; this is
//              the packet layer, the reward payout and the job advancement.
//              References are KncWX2Server/GameServer/GSUserGameCommon.cpp
//              (:1482 accept, :1689 give up, :1856 complete, :6461 title) and
//              UserQuestManager.cpp's DBE_QUEST_COMPLETE_ACK (:4948), which is
//              where the real server actually pays a quest out.
//
//              One structural note. On live, accepting or completing a quest is
//              a round trip through the game DB: the REQ validates, sends a
//              DBE_*, and the reward is paid in the DBE_*_ACK handler. There is
//              no DB actor offline and no reason to invent one, so the two
//              halves are joined - the handler validates, pays and answers in
//              one pass. What that costs is the ordering the DB round trip used
//              to impose, and the one place it mattered is called out below:
//              EGS_NEW_QUEST_ACK must reach the client before EGS_NEW_QUEST_NOT.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineInventory.h"
#include "X2OfflineSkill.h"
#include "X2OfflineQuest.h"
#include "X2OfflineTitle.h"

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::LoadQuestState( KOfflineSession& kSes, OUT KOfflineUnitRow& kRow )
{
	if( 0 == kSes.m_nSelectedUnitUID )
		return false;

	if( false == CX2OfflineDB::Instance()->LoadUnit( kSes.m_nSelectedUnitUID, kRow ) )
		return false;

	// The inventory has to be the same character's too: every collection
	// sub-quest is answered by counting it, and phase 5's near-miss (a helper
	// that reads "the current character" being wrong inside a loop) is exactly
	// this shape.
	CX2OfflineInventory::Instance()->Load( kRow.m_nUnitUID );

	CX2OfflineQuest::Instance()->Load( kRow.m_nUnitUID );
	CX2OfflineTitle::Instance()->Load( kRow.m_nUnitUID );

	return true;
}

void CX2OfflineServer::PushQuestUpdate( KOfflineSession& kSes,
										const std::vector< KQuestInstance >& vecChanged )
{
	if( true == vecChanged.empty() )
		return;

	KEGS_UPDATE_QUEST_NOT kNot;
	kNot.m_vecQuestInst = vecChanged;

	Reply( kSes, EGS_UPDATE_QUEST_NOT, kNot );
}

void CX2OfflineServer::PushNewQuest( KOfflineSession& kSes,
									 const std::vector< KQuestInstance >& vecOpened )
{
	for( size_t i = 0; i < vecOpened.size(); ++i )
	{
		KEGS_NEW_QUEST_NOT kNot;
		kNot.m_QuestInst = vecOpened[i];
		kNot.m_vecUpdatedInventorySlot.clear();

		Reply( kSes, EGS_NEW_QUEST_NOT, kNot );
	}
}

void CX2OfflineServer::QuestAutoOpen( KOfflineSession& kSes, int ePlace, int iPlaceID,
									  const KOfflineUnitRow& kRow )
{
	CX2OfflineQuest* pQuest = CX2OfflineQuest::Instance();

	std::vector< KQuestInstance > vecOpened;

	pQuest->CheckAutoOpen( (CX2OfflineQuest::AUTO_OPEN_PLACE)ePlace, iPlaceID, kRow, vecOpened );

	if( true == vecOpened.empty() )
		return;

	PushNewQuest( kSes, vecOpened );

	// A quest opened by walking into a village usually starts with "go to that
	// village", and one opened by a level-up may start with "reach that level".
	// Both are already true the moment the quest exists, so they are checked
	// immediately - which is what the server does off the back of the accept it
	// just made (GSUserGameCommon.cpp:1608). Without this the first step of
	// every chain link sits unticked until the player walks out and back in.
	//
	// Checked against where the character actually IS, not against the place
	// that triggered this sweep. Those are the same thing for the three place
	// sweeps and different for the two that carry the story: a quest opened by
	// AOP_AFTER_QUEST opens wherever the player happened to hand the last one
	// in, and used to get no visit check at all.
	std::vector< KQuestInstance > vecChanged;

	QuestCheckHereAndNow( kSes, kRow, vecChanged );

	PushQuestUpdate( kSes, vecChanged );
}

void CX2OfflineServer::QuestCheckHereAndNow( KOfflineSession& kSes, const KOfflineUnitRow& kRow,
											 OUT std::vector< KQuestInstance >& vecChanged )
{
	CX2OfflineQuest* pQuest = CX2OfflineQuest::Instance();

	// Repeated, not run once, because finishing one step can UNSTAGE another.
	// A quest's sub-quests are grouped, and CheckBeforeGroup refuses a step
	// while any earlier group is outstanding - so "talk to Lowe, then go to
	// Elder" leaves the Elder step refused on the first pass and eligible on
	// the second, with nothing left to trigger it if the player is already in
	// Elder. Three passes settles a three-deep chain; no quest in the script
	// has more groups than that, and a bounded loop cannot hang the way a
	// while-something-changed one could. Same shape as
	// CX2OfflineTitle::ReCheckPassive.
	//
	// Every one of these is idempotent - a step already flagged done is skipped
	// and MarkChanged never lists a quest twice - so the extra passes cost a
	// walk of at most MAX_ING_QUEST_NUM quests and change nothing else.
	for( int iPass = 0; iPass < 3; ++iPass )
	{
		// m_kRoom is the only record of being inside something; outside one,
		// the character's saved map ID is the village it is standing in. Same
		// order the dungeon/field/village question is asked everywhere else in
		// this file.
		if( true == m_kRoom.m_bActive && 0 != m_kRoom.m_kInfo.m_iBattleFieldID )
		{
			pQuest->OnVisitField( m_kRoom.m_kInfo.m_iBattleFieldID, kRow, vecChanged );
		}
		else if( true == m_kRoom.m_bActive && 0 != m_kRoom.m_kInfo.m_iDungeonID )
		{
			pQuest->OnVisitDungeon( m_kRoom.m_kInfo.m_iDungeonID,
									m_kRoom.m_kInfo.m_DifficultyLevel, kRow, vecChanged );
		}
		else
		{
			pQuest->OnVisitVillage( kRow.m_iLastPos, kRow, vecChanged );
		}

		pQuest->OnLevelUp( kRow, vecChanged );
	}
}

void CX2OfflineServer::PushMissionUpdate( KOfflineSession& kSes,
										  const std::vector< KMissionInstance >& vecNew,
										  const std::vector< KMissionInstance >& vecChanged,
										  const std::vector< KTitleInfo >& vecNewTitle )
{
	if( false == vecNew.empty() )
	{
		KEGS_NEW_MISSION_NOT kNot;
		kNot.m_bFirst			= false;
		kNot.m_vecNewMission	= vecNew;

		Reply( kSes, EGS_NEW_MISSION_NOT, kNot );
	}

	if( false == vecChanged.empty() )
	{
		KEGS_UPDATE_MISSION_NOT kNot;
		kNot.m_vecMissionInst = vecChanged;

		Reply( kSes, EGS_UPDATE_MISSION_NOT, kNot );
	}

	// One packet per title: EGS_REWARD_TITLE_NOT carries a single KTitleInfo,
	// and it is what plays the "new title" banner.
	for( size_t i = 0; i < vecNewTitle.size(); ++i )
	{
		KEGS_REWARD_TITLE_NOT kNot;
		kNot.m_kTitle = vecNewTitle[i];

		Reply( kSes, EGS_REWARD_TITLE_NOT, kNot );
	}
}

//////////////////////////////////////////////////////////////////////////
// accept

bool CX2OfflineServer::Handler_EGS_NEW_QUEST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_NEW_QUEST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KOfflineUnitRow kRow;
	if( false == LoadQuestState( kSes, kRow ) )
	{
		KEGS_NEW_QUEST_ACK kAck;
		kAck.m_iOK = NetError::ERR_QUEST_03;
		return Reply( kSes, EGS_NEW_QUEST_ACK, kAck );
	}

	KQuestInstance kInst;

	const int iResult = CX2OfflineQuest::Instance()->Accept(
		kReq.m_iQuestID, kReq.m_iTalkNPCID, kRow, false, kInst );

	// The ACK first, then the _NOT. This is not cosmetic. The client arms a
	// wait on EGS_NEW_QUEST_ACK and its _NOT handler runs the quest into the
	// UI; on live the ACK is sent from the DB reply handler one line before the
	// _NOT (GSUserGameCommon.cpp:1532 then :1600), and doing it the other way
	// round here left the wait armed while the UI had already moved on.
	//
	// The exception is an epic quest, which the client does NOT wait on:
	// CX2UIQuestNew::Handler_EGS_NEW_QUEST_REQ returns before AddServerPacket
	// for QT_EPIC (X2UIQuestNew.cpp:1746). Sending the ACK anyway is harmless -
	// DeleteServerPacket simply finds nothing - so it goes out unconditionally
	// rather than being conditioned on a templet lookup that could disagree
	// with the client's.
	{
		KEGS_NEW_QUEST_ACK kAck;
		kAck.m_iOK = iResult;

		Reply( kSes, EGS_NEW_QUEST_ACK, kAck );
	}

	if( NetError::NET_OK != iResult )
		return true;

	KEGS_NEW_QUEST_NOT kNot;
	kNot.m_QuestInst = kInst;

	// Quests that pay an item just for accepting them (m_mapAcceptRewardItem on
	// the server's templet) are not implemented: the client's own QuestTemplet
	// has no such field, so there is nothing offline to read the list from.
	// The vector goes out empty, which the client treats as "no items changed".
	kNot.m_vecUpdatedInventorySlot.clear();

	Reply( kSes, EGS_NEW_QUEST_NOT, kNot );

	// Accepting can complete a step immediately - a "go to Ruben" quest taken
	// while standing in Ruben, or a "reach level 10" one taken at level 12.
	// GSUserGameCommon.cpp:1608 does exactly this off the back of the same
	// packet.
	std::vector< KQuestInstance > vecChanged;

	QuestCheckHereAndNow( kSes, kRow, vecChanged );

	PushQuestUpdate( kSes, vecChanged );

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_UPDATE_QUEST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	// The request carries a default-constructed KQuestInstance - the client
	// sends it empty (CX2State::Handler_EGS_UPDATE_QUEST_REQ, X2State.cpp:4236)
	// and it means "resend my quests", not "here is a quest". It still has to be
	// read off the wire so the buffer is consumed the same way every other
	// handler consumes it.
	KEGS_UPDATE_QUEST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_UPDATE_QUEST_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	Reply( kSes, EGS_UPDATE_QUEST_ACK, kAck );

	KOfflineUnitRow kRow;
	if( false == LoadQuestState( kSes, kRow ) )
		return true;

	std::vector< KQuestInstance > vecAll;
	CX2OfflineQuest::Instance()->GetQuestInstances( vecAll );

	PushQuestUpdate( kSes, vecAll );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// complete

int CX2OfflineServer::CompleteOneQuest( KOfflineSession& kSes,
										const KEGS_QUEST_COMPLETE_REQ& kReq,
										OUT KEGS_QUEST_COMPLETE_ACK& kAck )
{
	kAck.m_kCompleteQuestInfo.m_iQuestID = kReq.m_iQuestID;

	// Cleared up front, not just on the success path: every `return` below
	// leaves it as it found it, and a stale one from an earlier completion
	// would be replayed against the wrong quest.
	m_kQuestAfter.m_bValid = false;

	KOfflineUnitRow kRow;
	if( false == LoadQuestState( kSes, kRow ) )
		return NetError::ERR_QUEST_03;

	CX2OfflineQuest*		pQuest	= CX2OfflineQuest::Instance();
	CX2OfflineInventory*	pInven	= CX2OfflineInventory::Instance();

	const int iCheck = pQuest->CheckCompletable( kReq.m_iQuestID, kRow );
	if( NetError::NET_OK != iCheck )
		return iCheck;

	const CX2QuestManager::QuestTemplet* pTemplet = CX2OfflineQuest::Templet( kReq.m_iQuestID );
	if( NULL == pTemplet )
		return NetError::ERR_QUEST_08;

	//////////////////////////////////////////////////////////////////////////
	// The reward items, gathered before anything is spent, so a full bag
	// refuses the hand-in rather than eating the collected items first.
	std::map< int, int > mapReward;			///< item ID -> quantity

	for( size_t i = 0; i < pTemplet->m_Reward.m_vecItem.size(); ++i )
	{
		const CX2QuestManager::ITEM_DATA& kItem = pTemplet->m_Reward.m_vecItem[i];

		if( 0 == kItem.m_iItemID || 0 == kItem.m_iQuantity )
			continue;

		mapReward[ kItem.m_iItemID ] += kItem.m_iQuantity;
	}

	// The chosen half. The client names which of the offered items it wants;
	// anything it names that is not on the offer list is ignored, and naming
	// more than the quest allows is refused outright - both are the real
	// server's rules (UserQuestManager.cpp:4076).
	if( false == kReq.m_mapSelectItemID.empty() )
	{
		if( (int)kReq.m_mapSelectItemID.size() > pTemplet->m_SelectReward.m_iSelectionCount )
			return NetError::ERR_QUEST_14;

		bool bAnyMatched = false;

		for( size_t i = 0; i < pTemplet->m_SelectReward.m_vecSelectItem.size(); ++i )
		{
			const CX2QuestManager::ITEM_DATA& kItem = pTemplet->m_SelectReward.m_vecSelectItem[i];

			if( kReq.m_mapSelectItemID.end() == kReq.m_mapSelectItemID.find( kItem.m_iItemID ) )
				continue;

			mapReward[ kItem.m_iItemID ] += kItem.m_iQuantity;
			bAnyMatched = true;
		}

		if( false == bAnyMatched )
			return NetError::ERR_QUEST_15;
	}

	//////////////////////////////////////////////////////////////////////////
	// Hand the collected items in FIRST, then check there is room for the
	// reward, then give it.
	//
	// The order is the point. Checking for room before the hand-in refuses a
	// perfectly valid completion whenever the bag is full - which is exactly
	// when a quest that takes ten items and gives one is most likely to be
	// handed in, and it is the common case, not a corner. The real server has
	// no such problem because KInventory::DeleteAndInsert does both halves in
	// one call (UserQuestManager.cpp:4234).
	//
	// If the reward still does not fit after the hand-in, the collected items go
	// straight back and the completion is refused. Putting them back cannot
	// itself fail: the slots they came out of are still free.
	std::map< int, int > mapCondition;
	CX2OfflineQuest::GetConditionItems( kReq.m_iQuestID, mapCondition );

	std::vector< KInventoryItemInfo > vecChangedSlot;
	std::map< int, int > mapTaken;			///< what was actually handed in, for the undo

	{
		std::map< int, int >::const_iterator mit;
		for( mit = mapCondition.begin(); mit != mapCondition.end(); ++mit )
		{
			const int iTook = pInven->ConsumeByID( mit->first, mit->second, vecChangedSlot );

			if( iTook > 0 )
				mapTaken[ mit->first ] = iTook;

			CX2OfflineLog::Server( L"QUEST    complete %d took %d/%d x item %d",
				kReq.m_iQuestID, iTook, mit->second, mit->first );
		}
	}

	// Room for the reward? HasRoomFor answers per item, which is exact for the
	// one-item case and optimistic for several - two rewards that would each
	// fit the same last free slot both pass. The insert below reports what it
	// actually managed and says so.
	{
		bool bNoRoom = false;

		std::map< int, int >::const_iterator mit;
		for( mit = mapReward.begin(); mit != mapReward.end() && false == bNoRoom; ++mit )
		{
			if( false == pInven->HasRoomFor( mit->first, mit->second ) )
			{
				CX2OfflineLog::Server( L"QUEST    complete %d refused: no room for %d x item %d",
					kReq.m_iQuestID, mit->second, mit->first );
				bNoRoom = true;
			}
		}

		if( true == bNoRoom )
		{
			std::vector< KInventoryItemInfo > vecUndo;

			for( mit = mapTaken.begin(); mit != mapTaken.end(); ++mit )
			{
				int iBack = 0;
				pInven->InsertItem( mit->first, mit->second, 0, vecUndo, iBack );

				CX2OfflineLog::Server( L"QUEST    complete %d rolled back %d/%d x item %d",
					kReq.m_iQuestID, iBack, mit->second, mit->first );
			}

			return NetError::ERR_QUEST_13;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Past the last refusal. From here everything succeeds.

	{
		std::map< int, int >::const_iterator mit;
		for( mit = mapReward.begin(); mit != mapReward.end(); ++mit )
		{
			int iInserted = 0;
			pInven->InsertItem( mit->first, mit->second, 0, vecChangedSlot, iInserted );

			kAck.m_kUpdateUnitInfo.m_mapItemObtained[ mit->first ] = iInserted;

			if( iInserted < mit->second )
			{
				CX2OfflineLog::Server(
					L"QUEST    complete %d WARNING only %d of %d x item %d fitted - the rest is"
					L" lost (there is no temp inventory offline)",
					kReq.m_iQuestID, iInserted, mit->second, mit->first );
			}
			else
			{
				CX2OfflineLog::Server( L"QUEST    complete %d gave %d x item %d",
					kReq.m_iQuestID, iInserted, mit->first );
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// EXP, ED and SP.
	//
	// ApplyDungeonReward is the one place that adds EXP, levels the character
	// off the client's EXP table and grants the skill points those levels are
	// worth. Reusing it here rather than writing a second payout is what keeps
	// a quest level-up identical to a dungeon one.
	int iOldLevel = kRow.m_iLevel;

	const int iNewLevel = ApplyDungeonReward( kRow.m_nUnitUID,
											  pTemplet->m_Reward.m_iEXP,
											  pTemplet->m_Reward.m_iED,
											  &iOldLevel );

	// m_Reward.m_iSP is a *skill* point grant on top of the level-up ones. The
	// live server routes it to AddAPoint under SERV_PVP_NEW_SYSTEM, which is an
	// arena point rather than a skill point; that system does not exist offline
	// and PvP is out of scope, so it is paid as SP - the meaning the field has
	// in the client's own Reward struct and in the pre-PvP server code.
	if( pTemplet->m_Reward.m_iSP > 0 )
	{
		KOfflineUnitRow kNow;
		if( true == CX2OfflineDB::Instance()->LoadUnit( kRow.m_nUnitUID, kNow ) )
		{
			CX2OfflineDB::Instance()->SaveSkillPoint( kRow.m_nUnitUID,
				kNow.m_iSP + pTemplet->m_Reward.m_iSP, kNow.m_iCSP );

			CX2OfflineLog::Server( L"QUEST    complete %d gave +%d SP",
				kReq.m_iQuestID, pTemplet->m_Reward.m_iSP );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Job advancement.
	//
	// This is the whole of a class change. XS_CLASS_CHANGE is dead code - every
	// call site that would enter it is commented out (X2UIQuestNew.cpp:1503) -
	// and the client instead reads the new class straight off the KUnitInfo in
	// this ACK, resets the skill tree UI and shows GetClassChangePopup().
	const int iNewClass = CX2OfflineQuest::GetChangeUnitClass( kReq.m_iQuestID );

	bool bChangedJob = false;

	if( (int)CX2Unit::UC_NONE != iNewClass && iNewClass != kRow.m_iUnitClass )
	{
		CX2OfflineDB::Instance()->SaveUnitClass( kRow.m_nUnitUID, iNewClass );

		// The new class's starting skills. SERV_UPGRADE_SKILL_SYSTEM_2013 is on
		// (ServerDefine.h:3749), and the live path grants the two defaults the
		// advancement unlocks. SeedDefaultSkills grants the whole default list
		// for the class, which is the same set: an already-known skill is
		// re-upserted at level 1 and is a no-op for anything already learned
		// higher, because the base skills are shared down the class tree.
		CX2OfflineSkill::Instance()->Load( kRow.m_nUnitUID );
		CX2OfflineSkill::Instance()->SeedDefaultSkills( kRow.m_nUnitUID, iNewClass );

		bChangedJob = true;

		CX2OfflineLog::Server( L"QUEST    complete %d CLASS CHANGE unitUID=%I64d %d -> %d",
			kReq.m_iQuestID, (__int64)kRow.m_nUnitUID, kRow.m_iUnitClass, iNewClass );
	}

	//////////////////////////////////////////////////////////////////////////
	// Move the quest to the completed list.
	pQuest->Finish( kReq.m_iQuestID, kAck.m_kCompleteQuestInfo );

	CX2OfflineLog::Server( L"QUEST    completed %d '%s': +%d exp, +%d ED, lv %d -> %d",
		kReq.m_iQuestID, pTemplet->m_wstrTitle.c_str(),
		pTemplet->m_Reward.m_iEXP, pTemplet->m_Reward.m_iED, iOldLevel, iNewLevel );

	//////////////////////////////////////////////////////////////////////////
	// The character, as it now stands. Read back rather than patched up: the
	// class, the level, the EXP and the SP were all written by different calls
	// above and the row is the only place they are all correct at once.
	KOfflineUnitRow kAfter;
	if( true == CX2OfflineDB::Instance()->LoadUnit( kRow.m_nUnitUID, kAfter ) )
	{
		MakeUnitInfoFromRow( kAck.m_kUpdateUnitInfo.m_kUnitInfo, kAfter );
		kRow = kAfter;
	}

	kAck.m_kUpdateUnitInfo.m_vecKInventorySlotInfo	= vecChangedSlot;
	kAck.m_kUpdateUnitInfo.m_bGameEnd				= false;

	//////////////////////////////////////////////////////////////////////////
	// What the completion sets off is deliberately NOT done here.
	//
	// Everything above builds the ACK; the packets that follow from the
	// completion - the level-up effect, the quest updates, the title work - have
	// to reach the client *after* it, because the client is holding an
	// AddServerPacket wait on EGS_QUEST_COMPLETE_ACK and its _NOT handlers run
	// against a character it has not been told about yet. The caller sends the
	// ACK and then calls AfterQuestComplete, which is also what the live pair of
	// servers ends up doing (UserQuestManager.cpp:5140 sends the ACK, then the
	// chained-quest and title work runs).
	m_kQuestAfter.m_bValid		= true;
	m_kQuestAfter.m_iQuestID	= kReq.m_iQuestID;
	m_kQuestAfter.m_iOldLevel	= iOldLevel;
	m_kQuestAfter.m_iNewLevel	= iNewLevel;
	m_kQuestAfter.m_bChangedJob	= bChangedJob;

	return NetError::NET_OK;
}

void CX2OfflineServer::AfterQuestComplete( KOfflineSession& kSes )
{
	if( false == m_kQuestAfter.m_bValid )
		return;

	const int  iQuestID		= m_kQuestAfter.m_iQuestID;
	const int  iOldLevel	= m_kQuestAfter.m_iOldLevel;
	const int  iNewLevel	= m_kQuestAfter.m_iNewLevel;
	const bool bChangedJob	= m_kQuestAfter.m_bChangedJob;

	m_kQuestAfter.m_bValid = false;

	KOfflineUnitRow kRow;
	if( false == LoadQuestState( kSes, kRow ) )
		return;

	std::vector< KQuestInstance > vecQuestChanged;

	// A level gained from the quest reward can finish a "reach level N" step.
	if( iNewLevel > iOldLevel )
	{
		PushLevelUp( kSes, kRow.m_nUnitUID );
		CX2OfflineQuest::Instance()->OnLevelUp( kRow, vecQuestChanged );
	}

	// The items just handed in and just received both move collection steps of
	// *other* quests, but the client recounts its own bag to draw them, so
	// nothing has to be pushed for that - only what actually changed here.
	PushQuestUpdate( kSes, vecQuestChanged );

	// THE CHAIN. Finishing a quest is what makes the next one available, and
	// the player never asks for it: the server re-runs the prerequisite sweep
	// after every completion (UserQuestManager.cpp:5175). The place sweep runs
	// too, because a quest that names this village as its opening place could
	// not open while its prerequisite was outstanding and can now.
	CX2OfflineLog::Server(
		L"QUEST    chain check after %d: %d quest(s) in the script list it as a prerequisite",
		iQuestID, CX2OfflineQuest::Instance()->CountWaitingOn( iQuestID ) );

	// The after-quest list FIRST, because it is the one that actually carries
	// the story: the finished quest names its own successors, and most epic
	// quests have no other way in. 11005 '[Field] Thief Pursuit' is the proof -
	// no prerequisite, no opening village, no opening dungeon.
	QuestAutoOpen( kSes, (int)CX2OfflineQuest::AOP_AFTER_QUEST, iQuestID, kRow );

	QuestAutoOpen( kSes, (int)CX2OfflineQuest::AOP_BEFORE_QUEST, 0, kRow );

	if( false == m_kRoom.m_bActive )
		QuestAutoOpen( kSes, (int)CX2OfflineQuest::AOP_VILLAGE, kRow.m_iLastPos, kRow );

	// The sweeps above only re-check where the player is standing when they
	// actually open something. A quest already in the list can also have been
	// waiting on this completion - a "hand in at Lowe, then visit Elder" pair
	// where the second quest was granted earlier - so the here-and-now check
	// runs unconditionally too.
	{
		std::vector< KQuestInstance > vecSettled;
		QuestCheckHereAndNow( kSes, kRow, vecSettled );
		PushQuestUpdate( kSes, vecSettled );
	}

	std::vector< KMissionInstance >	vecNewMission;
	std::vector< KMissionInstance >	vecMissionChanged;
	std::vector< KTitleInfo >		vecNewTitle;

	CX2OfflineTitle* pTitle = CX2OfflineTitle::Instance();

	pTitle->OnQuestComplete( iQuestID, kRow, vecMissionChanged, vecNewTitle );
	pTitle->ReCheckPassive( kRow, vecMissionChanged, vecNewTitle );

	// A level or a class change opens missions that were out of reach.
	if( iNewLevel > iOldLevel || true == bChangedJob )
		pTitle->CheckNewMission( kRow, vecNewMission );

	PushMissionUpdate( kSes, vecNewMission, vecMissionChanged, vecNewTitle );
}

bool CX2OfflineServer::Handler_EGS_QUEST_COMPLETE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_QUEST_COMPLETE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_QUEST_COMPLETE_ACK kAck;
	kAck.m_iOK = CompleteOneQuest( kSes, kReq, kAck );

	Reply( kSes, EGS_QUEST_COMPLETE_ACK, kAck );

	AfterQuestComplete( kSes );

	return true;
}

bool CX2OfflineServer::Handler_EGS_ALL_COMPLETED_QUEST_COMPLETE_REQ( KOfflineSession& kSes,
																	 const KEvent& kEvent )
{
	KEGS_ALL_COMPLETED_QUEST_COMPLETE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// The "hand in everything that is ready" button. Each entry is an ordinary
	// completion request; the batch answer carries one ACK per quest, and the
	// client walks them (CX2UIQuestNew::Handler_EGS_ALL_COMPLETED_QUEST_
	// COMPLETE_ACK). A quest that cannot be handed in gets its own error in its
	// own entry rather than failing the batch, because the player pressed one
	// button for several quests and one bad quest should not lose the others.
	KEGS_ALL_COMPLETED_QUEST_COMPLETE_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	// The follow-up work for each quest is collected as the batch runs and only
	// released after the one ACK goes out, for the same reason the single case
	// defers it: the client is waiting on that ACK.
	std::vector< KQuestAfter > vecAfter;

	for( size_t i = 0; i < kReq.m_vecQuestCompleteReq.size(); ++i )
	{
		KEGS_QUEST_COMPLETE_ACK kOne;
		kOne.m_iOK = CompleteOneQuest( kSes, kReq.m_vecQuestCompleteReq[i], kOne );

		kAck.m_vecQuestCompleteAck.push_back( kOne );

		if( true == m_kQuestAfter.m_bValid )
		{
			vecAfter.push_back( m_kQuestAfter );
			m_kQuestAfter.m_bValid = false;
		}
	}

	CX2OfflineLog::Server( L"QUEST    batch complete: %u quest(s)",
		(unsigned int)kReq.m_vecQuestCompleteReq.size() );

	Reply( kSes, EGS_ALL_COMPLETED_QUEST_COMPLETE_ACK, kAck );

	for( size_t i = 0; i < vecAfter.size(); ++i )
	{
		m_kQuestAfter = vecAfter[i];
		AfterQuestComplete( kSes );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// give up

bool CX2OfflineServer::Handler_EGS_GIVE_UP_QUEST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GIVE_UP_QUEST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KOfflineUnitRow kRow;

	KEGS_GIVE_UP_QUEST_ACK kAck;
	kAck.m_iQuestID	= kReq.m_iQuestID;
	kAck.m_iOK		= NetError::ERR_QUEST_02;

	if( true == LoadQuestState( kSes, kRow ) &&
		true == CX2OfflineQuest::Instance()->GiveUp( kReq.m_iQuestID ) )
	{
		kAck.m_iOK = NetError::NET_OK;
	}

	// The client does NOT arm a wait on this ACK - both AddServerPacket calls
	// are commented out (X2UIQuestNew.cpp:1641) - but its handler is what
	// actually removes the quest from the UI, so the reply is still required.
	return Reply( kSes, EGS_GIVE_UP_QUEST_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_GATHER_GIVE_UP_QUEST_REQ( KOfflineSession& kSes,
															 const KEvent& kEvent )
{
	KEGS_GATHER_GIVE_UP_QUEST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KOfflineUnitRow kRow;
	const bool bLoaded = LoadQuestState( kSes, kRow );

	// One ACK per quest, which is what the server does too - it loops the IDs
	// into single give-ups and answers each (GSUserGameCommon.cpp:1723).
	for( size_t i = 0; i < kReq.m_vecGatherQuestID.size(); ++i )
	{
		KEGS_GIVE_UP_QUEST_ACK kAck;
		kAck.m_iQuestID	= kReq.m_vecGatherQuestID[i];
		kAck.m_iOK		= NetError::ERR_QUEST_02;

		if( true == bLoaded &&
			true == CX2OfflineQuest::Instance()->GiveUp( kReq.m_vecGatherQuestID[i] ) )
		{
			kAck.m_iOK = NetError::NET_OK;
		}

		Reply( kSes, EGS_GIVE_UP_QUEST_ACK, kAck );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// titles

bool CX2OfflineServer::Handler_EGS_EQUIP_TITLE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_EQUIP_TITLE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_EQUIP_TITLE_ACK kAck;
	kAck.m_iTitleID	= kReq.m_iTitleID;
	kAck.m_iOK		= NetError::ERR_TITLE_00;

	KOfflineUnitRow kRow;
	if( false == LoadQuestState( kSes, kRow ) )
		return Reply( kSes, EGS_EQUIP_TITLE_ACK, kAck );

	kAck.m_iOK = CX2OfflineTitle::Instance()->CheckEquip( kReq.m_iTitleID, kRow.m_iTitleID );

	if( NetError::NET_OK == kAck.m_iOK )
	{
		CX2OfflineDB::Instance()->SaveEquippedTitle( kRow.m_nUnitUID, kReq.m_iTitleID );

		CX2OfflineLog::Server( L"TITLE    unitUID=%I64d wears title %d",
			(__int64)kRow.m_nUnitUID, kReq.m_iTitleID );
	}

	Reply( kSes, EGS_EQUIP_TITLE_ACK, kAck );

	if( NetError::NET_OK != kAck.m_iOK )
		return true;

	// The broadcast that redraws the character. Phase 4 correction 6 again: the
	// ACK updates the player's own data, the _NOT is what makes the model show
	// it, and it is a different packet in a room than in a village.
	if( S_ROOM == kSes.m_eState )
	{
		KEGS_CHANGE_EQUIPPED_ITEM_IN_ROOM_NOT kNot;
		kNot.m_cChangeEquippedPacketType	= KEGS_CHANGE_EQUIPPED_ITEM_IN_ROOM_NOT::CEPT_CHANGE_TITLE;
		kNot.m_UnitUID						= kRow.m_nUnitUID;
		kNot.m_iEquippedTitleID				= kReq.m_iTitleID;

		// The same trap PushEquipChanged documents: the client assigns
		// m_kGameStat straight over the unit's own, so a zeroed one would give
		// the in-dungeon character max HP 0 for the sake of a title change.
		MakeGameStat( kRow, kNot.m_kGameStat );

		Reply( kSes, EGS_CHANGE_EQUIPPED_ITEM_IN_ROOM_NOT, kNot );
	}
	else
	{
		KEGS_CHANGE_EQUIPPED_ITEM_IN_FIELD_NOT kNot;
		kNot.m_iUnitUID			= kRow.m_nUnitUID;
		kNot.m_iEquippedTitleID	= kReq.m_iTitleID;

		Reply( kSes, EGS_CHANGE_EQUIPPED_ITEM_IN_FIELD_NOT, kNot );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// the event bridges

void CX2OfflineServer::QuestOnTalkNPC( KOfflineSession& kSes, int iNPCID )
{
	KOfflineUnitRow kRow;
	if( false == LoadQuestState( kSes, kRow ) )
		return;

	// What this NPC has to offer, asked of the client's own quest manager - the
	// same call its quest window makes to build the list it draws.
	//
	// This is a diagnostic, and it is here because the other half of "no quest
	// appeared" is an NPC that offers nothing, which produces no packet at all
	// and so leaves no trace whatsoever. A quest an NPC starts is accepted
	// entirely on the player's initiative; if the list is empty the offline
	// server never hears about it and cannot know anything went wrong.
	if( NULL != g_pData && NULL != g_pData->GetQuestManager() )
	{
		CX2QuestManager* pQM = g_pData->GetQuestManager();

		std::vector< int > vecStart, vecAvailable, vecEnd;

		pQM->GetStartQuestList( (CX2UnitManager::NPC_UNIT_ID)iNPCID, vecStart );
		pQM->GetAvailableQuest( (CX2UnitManager::NPC_UNIT_ID)iNPCID, vecAvailable );
		pQM->GetEndQuestList( (CX2UnitManager::NPC_UNIT_ID)iNPCID, vecEnd );

		CX2OfflineLog::Server(
			L"QUEST    NPC %d offers %u quest(s), %u available now, %u completable here",
			iNPCID, (unsigned int)vecStart.size(), (unsigned int)vecAvailable.size(),
			(unsigned int)vecEnd.size() );
	}

	std::vector< KQuestInstance > vecChanged;
	CX2OfflineQuest::Instance()->OnTalkNPC( iNPCID, kRow.m_iLastPos, kRow, vecChanged );

	// "Talk to Lowe, then go to Elder" is one quest with two staged groups, and
	// the player is standing in Elder while talking to Lowe. Finishing the talk
	// step is what makes the visit step eligible, and by then the visit has
	// already happened - so it is re-checked here rather than waiting for the
	// player to leave the village and come back.
	QuestCheckHereAndNow( kSes, kRow, vecChanged );

	PushQuestUpdate( kSes, vecChanged );

	std::vector< KMissionInstance >	vecNewMission;
	std::vector< KMissionInstance >	vecMissionChanged;
	std::vector< KTitleInfo >		vecNewTitle;

	CX2OfflineTitle::Instance()->OnTalkNPC( iNPCID, kRow, vecMissionChanged, vecNewTitle );
	PushMissionUpdate( kSes, vecNewMission, vecMissionChanged, vecNewTitle );
}

void CX2OfflineServer::QuestOnNpcDie( KOfflineSession& kSes, int iNpcID )
{
	if( iNpcID <= 0 )
		return;

	KOfflineUnitRow kRow;
	if( false == LoadQuestState( kSes, kRow ) )
		return;

	std::vector< KQuestInstance > vecChanged;

	CX2OfflineQuest::Instance()->OnNpcDie( m_kRoom.m_kInfo.m_iDungeonID,
										   m_kRoom.m_kInfo.m_DifficultyLevel,
										   m_kRoom.m_kInfo.m_iBattleFieldID,
										   iNpcID, kRow, vecChanged );

	PushQuestUpdate( kSes, vecChanged );

	std::vector< KMissionInstance >	vecNewMission;
	std::vector< KMissionInstance >	vecMissionChanged;
	std::vector< KTitleInfo >		vecNewTitle;

	CX2OfflineTitle::Instance()->OnNpcDie( m_kRoom.m_kInfo.m_iDungeonID,
										   m_kRoom.m_kInfo.m_DifficultyLevel,
										   iNpcID, kRow, vecMissionChanged, vecNewTitle );

	PushMissionUpdate( kSes, vecNewMission, vecMissionChanged, vecNewTitle );
}

void CX2OfflineServer::QuestOnDungeonClear( KOfflineSession& kSes, char cRank,
											int iPlayTime, int iDamage )
{
	KOfflineUnitRow kRow;
	if( false == LoadQuestState( kSes, kRow ) )
		return;

	std::vector< KQuestInstance > vecChanged;

	CX2OfflineQuest::Instance()->OnDungeonClear( m_kRoom.m_kInfo.m_iDungeonID,
												 m_kRoom.m_kInfo.m_DifficultyLevel,
												 cRank, iPlayTime, iDamage, kRow, vecChanged );

	PushQuestUpdate( kSes, vecChanged );

	std::vector< KMissionInstance >	vecNewMission;
	std::vector< KMissionInstance >	vecMissionChanged;
	std::vector< KTitleInfo >		vecNewTitle;

	CX2OfflineTitle::Instance()->OnDungeonClear( m_kRoom.m_kInfo.m_iDungeonID,
												 m_kRoom.m_kInfo.m_DifficultyLevel,
												 cRank, iPlayTime, iDamage, kRow,
												 vecMissionChanged, vecNewTitle );

	PushMissionUpdate( kSes, vecNewMission, vecMissionChanged, vecNewTitle );
}

void CX2OfflineServer::QuestOnEnterVillage( KOfflineSession& kSes, int iMapID )
{
	KOfflineUnitRow kRow;
	if( false == LoadQuestState( kSes, kRow ) )
		return;

	// The auto-open FIRST, so a quest this village hands over gets its own
	// "you are here" step ticked by the same visit; then the visit check for
	// everything already in the list.
	QuestAutoOpen( kSes, (int)CX2OfflineQuest::AOP_VILLAGE, iMapID, kRow );

	std::vector< KQuestInstance > vecChanged;
	CX2OfflineQuest::Instance()->OnVisitVillage( iMapID, kRow, vecChanged );
	PushQuestUpdate( kSes, vecChanged );

	// Entering a village is also the moment a newly levelled character is idle
	// enough to be told about missions it has grown into. The real server does
	// this on every state entry (KUserTitleManager::CheckNewMission).
	std::vector< KMissionInstance >	vecNewMission;
	std::vector< KMissionInstance >	vecMissionChanged;
	std::vector< KTitleInfo >		vecNewTitle;

	CX2OfflineTitle::Instance()->CheckNewMission( kRow, vecNewMission );
	CX2OfflineTitle::Instance()->ReCheckPassive( kRow, vecMissionChanged, vecNewTitle );

	PushMissionUpdate( kSes, vecNewMission, vecMissionChanged, vecNewTitle );
}

void CX2OfflineServer::QuestOnEnterField( KOfflineSession& kSes, int iBattleFieldID )
{
	KOfflineUnitRow kRow;
	if( false == LoadQuestState( kSes, kRow ) )
		return;

	QuestAutoOpen( kSes, (int)CX2OfflineQuest::AOP_BATTLE_FIELD, iBattleFieldID, kRow );

	std::vector< KQuestInstance > vecChanged;
	CX2OfflineQuest::Instance()->OnVisitField( iBattleFieldID, kRow, vecChanged );
	PushQuestUpdate( kSes, vecChanged );
}

void CX2OfflineServer::QuestOnEnterDungeon( KOfflineSession& kSes )
{
	if( false == m_kRoom.m_bActive || 0 == m_kRoom.m_kInfo.m_iDungeonID )
		return;

	KOfflineUnitRow kRow;
	if( false == LoadQuestState( kSes, kRow ) )
		return;

	QuestAutoOpen( kSes, (int)CX2OfflineQuest::AOP_DUNGEON,
				   m_kRoom.m_kInfo.m_iDungeonID, kRow );

	std::vector< KQuestInstance > vecChanged;

	CX2OfflineQuest::Instance()->OnVisitDungeon( m_kRoom.m_kInfo.m_iDungeonID,
												 m_kRoom.m_kInfo.m_DifficultyLevel,
												 kRow, vecChanged );

	PushQuestUpdate( kSes, vecChanged );
}

void CX2OfflineServer::QuestOnFindNPC( KOfflineSession& kSes, const std::vector< int >& vecNpcID )
{
	if( true == vecNpcID.empty() )
		return;

	KOfflineUnitRow kRow;
	if( false == LoadQuestState( kSes, kRow ) )
		return;

	const bool bDungeon = ( 0 == m_kRoom.m_kInfo.m_iBattleFieldID );

	std::vector< KQuestInstance > vecChanged;

	CX2OfflineQuest::Instance()->OnFindNPC( vecNpcID, bDungeon,
											m_kRoom.m_kInfo.m_iDungeonID,
											m_kRoom.m_kInfo.m_DifficultyLevel,
											m_kRoom.m_kInfo.m_iBattleFieldID,
											kRow, vecChanged );

	PushQuestUpdate( kSes, vecChanged );
}

void CX2OfflineServer::QuestOnLevelUp( KOfflineSession& kSes )
{
	KOfflineUnitRow kRow;
	if( false == LoadQuestState( kSes, kRow ) )
		return;

	std::vector< KQuestInstance > vecChanged;
	CX2OfflineQuest::Instance()->OnLevelUp( kRow, vecChanged );
	PushQuestUpdate( kSes, vecChanged );

	std::vector< KMissionInstance >	vecNewMission;
	std::vector< KMissionInstance >	vecMissionChanged;
	std::vector< KTitleInfo >		vecNewTitle;

	CX2OfflineTitle::Instance()->CheckNewMission( kRow, vecNewMission );
	PushMissionUpdate( kSes, vecNewMission, vecMissionChanged, vecNewTitle );
}

void CX2OfflineServer::QuestOnUseSkillPoint( KOfflineSession& kSes, int iSkillID, int iCount )
{
	if( iCount <= 0 )
		return;

	KOfflineUnitRow kRow;
	if( false == LoadQuestState( kSes, kRow ) )
		return;

	std::vector< KQuestInstance > vecChanged;

	// Once per point, not once per request. A skill taken from level 1 to
	// level 3 in one click spends two points and counts as two towards a
	// "spend 5 SP" step - the server loops the same way
	// (GSUserGameCommon.cpp:4637), which is easy to miss because the loop is
	// three levels of nesting away from the handler it lives in.
	for( int i = 0; i < iCount; ++i )
		CX2OfflineQuest::Instance()->OnUseSkillPoint( iSkillID, kRow, vecChanged );

	PushQuestUpdate( kSes, vecChanged );
}

void CX2OfflineServer::QuestOnLearnNewSkill( KOfflineSession& kSes,
											 const std::vector< int >& vecSkillID )
{
	if( true == vecSkillID.empty() )
		return;

	KOfflineUnitRow kRow;
	if( false == LoadQuestState( kSes, kRow ) )
		return;

	std::vector< KQuestInstance > vecChanged;
	CX2OfflineQuest::Instance()->OnLearnNewSkill( vecSkillID, kRow, vecChanged );
	PushQuestUpdate( kSes, vecChanged );
}

void CX2OfflineServer::QuestOnUseItem( KOfflineSession& kSes, int iItemID )
{
	KOfflineUnitRow kRow;
	if( false == LoadQuestState( kSes, kRow ) )
		return;

	const int  iDungeonID	= ( true == m_kRoom.m_bActive ) ? m_kRoom.m_kInfo.m_iDungeonID : 0;
	const char cDifficulty	= ( true == m_kRoom.m_bActive )
								? (char)m_kRoom.m_kInfo.m_DifficultyLevel : (char)0;
	const int  iBattleField	= ( true == m_kRoom.m_bActive ) ? m_kRoom.m_kInfo.m_iBattleFieldID : 0;

	std::vector< KQuestInstance > vecChanged;

	CX2OfflineQuest::Instance()->OnUseItem( iItemID, iDungeonID, cDifficulty,
											iBattleField, kRow, vecChanged );

	PushQuestUpdate( kSes, vecChanged );
}

void CX2OfflineServer::QuestOnInventoryChanged( KOfflineSession& kSes )
{
	KOfflineUnitRow kRow;
	if( false == LoadQuestState( kSes, kRow ) )
		return;

	std::vector< KMissionInstance >	vecNewMission;
	std::vector< KMissionInstance >	vecMissionChanged;
	std::vector< KTitleInfo >		vecNewTitle;

	CX2OfflineTitle::Instance()->ReCheckPassive( kRow, vecMissionChanged, vecNewTitle );
	PushMissionUpdate( kSes, vecNewMission, vecMissionChanged, vecNewTitle );
}

#endif SERV_IRUHADEV_OFFLINE
