#include "stdafx.h"
#include "X2OfflineServer.h"

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-01
// Description: Offline mode - phase 3. Entering a village.
//
//              Two request/reply pairs, in this order:
//
//                EGS_STATE_CHANGE_FIELD_REQ / _ACK
//                  Sent from CX2StateServerSelect::Handler_EGS_STATE_CHANGE_
//                  FIELD_REQ (X2StateServerSelect.cpp:5934). Its ACK
//                  (:6022) is the only thing that sends
//                  XGM_STATE_CHANGE -> XS_VILLAGE_MAP, i.e. it is what
//                  actually leaves character select for the village. There is
//                  no separate EGS_STATE_CHANGE_VILLAGE_MAP_* event.
//
//                EGS_FIELD_LOADING_COMPLETE_REQ / _ACK
//                  Sent once the village world has loaded
//                  (X2StateField.cpp:1407). Carries the position the client
//                  spawned at; the real server stashes it in KGSUser::
//                  m_kLastPos (GSUserField.cpp:410) for the next login.
//
//              Both are waited on with AddServerPacket, so both must be
//              answered. Neither reply carries any state the client does not
//              already have - the village, its NPCs and its line map all come
//              out of the client's own .kom data, and the field user list is a
//              server *push* that simply never arrives offline, which is
//              correct: there is nobody else in the village.
//
//              Then four more, all found by running the village rather than by
//              reading the code:
//
//                EGS_OPTION_UPDATE_REQ / _ACK
//                  Sent from the CX2StateField constructor
//                  (X2StateField.cpp:131), i.e. once per village entry, and
//                  waited on.
//
//                EGS_UPDATE_PLAY_STATUS_NOT
//                  The client's live HP/MP/hyper, pushed every ~3s by
//                  CX2State::CheckAndSendingPlayStatus. This is the write half
//                  of what MakeGamePlayStatus reads back at character select.
//
//                EGS_FIELD_UNIT_SYNC_DATA_NOT
//                  Position sync, pushed every ~2s while the player moves. The
//                  real server broadcasts it to the rest of the field; offline
//                  there is nobody to broadcast to, but it is the only place
//                  the client reports where it is standing, so it is what makes
//                  "log back in where you left off" actually work.
//
//              EGS_JOIN_BATTLE_FIELD_REQ - the village portal out to a field -
//              was refused here in phase 3 and now lives in Handlers_Room.cpp:
//              in this build the fields outside a village are rooms, so
//              reaching one is room emulation rather than field entry.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_STATE_CHANGE_FIELD_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_STATE_CHANGE_FIELD_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_STATE_CHANGE_FIELD_ACK kAck;
	kAck.m_iOK				= NetError::NET_OK;
	kAck.m_iLimitLevel		= 0;
	kAck.m_iRequireDungeonID= 0;

	// This ACK must never carry an error. Every failure path in
	// CX2StateServerSelect::Handler_EGS_STATE_CHANGE_FIELD_ACK
	// (X2StateServerSelect.cpp:6074) ends in
	// Handler_EGS_STATE_CHANGE_FIELD_REQ( true ), i.e. it immediately re-sends
	// the request - so refusing here is not "the client shows an error", it is
	// an unbounded request/reply loop. Anything that looks wrong is logged and
	// allowed through instead.
	//
	// Nor is there a CheckEnterTheVillage(). The real server gates villages on
	// level and on having cleared a dungeon (ERR_FIELD_05 / ERR_FIELD_06);
	// offline the client is trusted, and it only asks for a village its own UI
	// already lets the player reach. Its own
	// Handler_EGS_STATE_CHANGE_FIELD_REQ has additionally rewritten the map to
	// VMI_RUBEN for a character with EXP 0 or one whose stored map has no
	// village templet, so an unusable ID does not get this far.
	kSes.m_eState = S_FIELD_MAP;

	if( 0 != kSes.m_nSelectedUnitUID )
	{
		CX2OfflineDB::Instance()->SaveLastPosition( kSes.m_nSelectedUnitUID, kReq.m_iMapID );

		CX2OfflineLog::Server( L"FIELD    unitUID=%I64d entering mapID=%d%s",
			(__int64)kSes.m_nSelectedUnitUID, kReq.m_iMapID,
			( true == kReq.m_bUseWarpItem ) ? L" (warp item)" : L"" );
	}
	else
	{
		CX2OfflineLog::Server( L"FIELD    WARNING mapID=%d with no character selected on this session"
			L" - allowed through, nothing persisted", kReq.m_iMapID );
	}

	return Reply( kSes, EGS_STATE_CHANGE_FIELD_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_FIELD_LOADING_COMPLETE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_FIELD_LOADING_COMPLETE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// SERV_EVENT_RETURN_USER_MARK and SERV_EVENT_GUIDELINE_POPUP are both off in
	// this build, so this ACK really is just KPacketOK - no returning-user mark,
	// no first-visit guide popup to decide about.
	KEGS_FIELD_LOADING_COMPLETE_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	if( 0 != kSes.m_nSelectedUnitUID )
	{
		CX2OfflineDB::Instance()->SaveLastFieldPos( kSes.m_nSelectedUnitUID,
			(int)kReq.m_ucLastTouchLineIndex, (int)kReq.m_usLastPosValue );

		CX2OfflineLog::Server( L"FIELD    unitUID=%I64d loaded, line=%d pos=%d",
			(__int64)kSes.m_nSelectedUnitUID,
			(int)kReq.m_ucLastTouchLineIndex, (int)kReq.m_usLastPosValue );
	}

	Reply( kSes, EGS_FIELD_LOADING_COMPLETE_ACK, kAck );

	//////////////////////////////////////////////////////////////////////////
	// Author: Iruha
	// Date: 2026-09-05
	// Description: Phase 28. This handler runs on every village entry within a
	// session, not only the first one after login (a dungeon/battlefield uses
	// a separate completion packet - EGS_BATTLE_FIELD_NPC_LOAD_COMPLETE_REQ,
	// Handlers_Room.cpp). SendPendingPetRestore is a no-op after its first
	// call - kSes.m_bPetRestorePending only gets set once, by Handler_EGS_
	// SELECT_UNIT_REQ - so calling it unconditionally here does not re-spawn
	// the pet on ordinary village-to-village travel. See the phase 28 section
	// of OFFLINE_MODE_PHASE9_PLAN.md's Trap.
	SendPendingPetRestore( kSes );
	//////////////////////////////////////////////////////////////////////////

	// Phase 6: standing in a village finishes a SQT_VISIT_VILLAGE step, and it
	// is also where a character that levelled elsewhere is told which title
	// missions it has grown into. Done here rather than at
	// EGS_STATE_CHANGE_FIELD_REQ because the client is only ready to draw a
	// quest update once the world is loaded, and the village is only *entered*
	// at that point - the state-change request is issued from character select,
	// before the map even exists.
	//
	// kReq carries no map ID, so the village is read back off the row that
	// EGS_STATE_CHANGE_FIELD_REQ just wrote a moment ago.
	{
		KOfflineUnitRow kRow;
		if( true == CX2OfflineDB::Instance()->LoadUnit( kSes.m_nSelectedUnitUID, kRow ) )
			QuestOnEnterVillage( kSes, kRow.m_iLastPos );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_OPTION_UPDATE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_OPTION_UPDATE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// m_kFieldOption is the client's graphics detail level, which the real
	// server uses to decide how many other players to put in a field sector.
	// There is nobody else in the field, so there is nothing to do with it - but
	// CX2State::Handler_EGS_OPTION_UPDATE_REQ arms AddServerPacket, so it has to
	// be answered.
	KEGS_OPTION_UPDATE_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_OPTION_UPDATE_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_UPDATE_PLAY_STATUS_NOT( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_UPDATE_PLAY_STATUS_NOT kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// No reply: this is a push, and the real server does not answer it either.
	if( 0 == kSes.m_nSelectedUnitUID )
		return true;

	// Placed before the early-outs below on purpose: this has to run on every
	// tick, not only the ones where a gauge moved. This packet's fixed
	// three-second cadence is the only regular beat a dungeon run has, and
	// PushRemainingPlayTime rides it to keep CX2Game::m_AllowFrameCount topped up
	// - without which the client stops reading the keyboard about seven seconds
	// into every dungeon. See PushRemainingPlayTime for the full story.
	PushRemainingPlayTime( kSes );

	// ...and the auto-party queue's, for the same reason. This packet is the
	// only regular beat the village has - CX2StateMenu::OnFrameMove pushes it
	// every three seconds through m_TimerForSendingPlayStatus - so the fake
	// matchmaking wait is counted out on it rather than on a timer the
	// emulator does not have. See AUTO_PARTY_QUEUE_MS for what that costs.
	TickAutoPartyMatch( kSes );

	// Note this one carries a bare KGamePlayStatus, not the
	// KGamePlayStatusContainer that EGS_SELECT_UNIT_4_NOT uses.
	const KGamePlayStatus& kStatus = kReq.m_kGamePlayStatus;

	// A status the client could not build at all comes through as max HP 0
	// (CX2GageManager::GetMyPlayStatusToPacket gates the send on returning true,
	// but a gage set that exists with nothing in it still reports zeroes).
	// Writing that would overwrite a good saved HP with 0.
	if( kStatus.m_iMaxHP <= 0 )
		return true;

	// Fires every three seconds for the whole session, so only write when a
	// value actually moved. The cache lives on the session, which means one
	// redundant write after a reconnect - cheaper than the alternative of
	// tracking it globally and getting it wrong when the character changes.
	if( kStatus.m_iCurHP		== kSes.m_iSavedHP &&
		kStatus.m_iCurMP		== kSes.m_iSavedMP &&
		kStatus.m_iCurHyperGage	== kSes.m_iSavedHyper &&
		kStatus.m_iCharAbilCount== kSes.m_iSavedAbil )
	{
		return true;
	}

	CX2OfflineDB::Instance()->SaveGamePlayStatus( kSes.m_nSelectedUnitUID,
		kStatus.m_iCurHP, kStatus.m_iCurMP, kStatus.m_iCurHyperGage, kStatus.m_iCharAbilCount );

	kSes.m_iSavedHP		= kStatus.m_iCurHP;
	kSes.m_iSavedMP		= kStatus.m_iCurMP;
	kSes.m_iSavedHyper	= kStatus.m_iCurHyperGage;
	kSes.m_iSavedAbil	= kStatus.m_iCharAbilCount;

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_FIELD_UNIT_SYNC_DATA_NOT( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_FIELD_UNIT_SYNC_DATA_NOT kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// No reply. The real server turns this into EGS_FIELD_UNIT_SYNC_DATA_BROAD_NOT
	// for everyone else in the sector; offline there is nobody, so the broadcast
	// half is correctly absent.
	//
	// What is worth keeping is the position. Without it, last_line_index and
	// last_pos_value are only ever written once, by
	// EGS_FIELD_LOADING_COMPLETE_REQ - which reports where the character
	// *spawned*, so the stored position never actually changes and "log back in
	// where you left off" silently does nothing. This is the only packet that
	// reports where the character is standing.
	if( 0 == kSes.m_nSelectedUnitUID )
		return true;

	CX2OfflineDB::Instance()->SaveLastFieldPos( kSes.m_nSelectedUnitUID,
		(int)kReq.m_ucLastTouchLineIndex, (int)kReq.m_usLastPosValue );

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_UPDATE_BATTLE_FIELD_USER_POS_NOT( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_UPDATE_BATTLE_FIELD_USER_POS_NOT kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// This was an explicit-ignore stub, on the reasoning that the real server
	// only rebroadcast it and there is nobody to rebroadcast to. Half right: it
	// is also the ONLY packet that reports where the character is standing in a
	// battlefield, and the login path needs that.
	//
	// The client's own re-entry code is what makes it matter
	// (X2StateServerSelect.cpp:4444-4450). On login it looks at the map it last
	// saw and, when that is a field rather than a village, re-joins the field
	// with the stored coordinates:
	//
	//     if ( VMI_BATTLE_FIELD_RUBEN_FIELD_01 <= pUnitData->m_nMapID &&
	//          VMI_BATTLE_FIELD_END            >  pUnitData->m_nMapID )
	//         Handler_EGS_JOIN_BATTLE_FIELD_REQ( pUnitData->m_nMapID,
	//                                            pUnitData->m_ucLastTouchLineIndex,
	//                                            pUnitData->m_usLastPosValue );
	//
	// All three come out of KUnitInfo::m_kLastPos, i.e. out of unit.last_pos,
	// last_line_index and last_pos_value. The battlefield join already stores the
	// field's ID in last_pos, so that branch was being taken correctly - but the
	// line and position still held whatever the *village* last wrote, because
	// EGS_FIELD_UNIT_SYNC_DATA_NOT is sent by CX2TFieldGame (the village) and
	// never by a field. Re-entering therefore placed the character at a village
	// line index inside a field, which is precisely the out-of-bounds spawn this
	// is meant to fix.
	//
	// One shared pair of columns for both map kinds is the studio's design, not a
	// shortcut: the server keeps a single KGSUser::m_kLastPos and overwrites it
	// from whichever place the player is in. The two encodings of m_usLastPosValue
	// differ - a village writes floatToHalf( ratio along the line )
	// (X2StateField.cpp:1400) and a field writes sqrtf( distance ) * 100
	// (X2BattleFieldGame.cpp:1031) - and that is safe because last_pos says which
	// map it belongs to, so the reader always knows which one it is looking at.
	if( 0 == kSes.m_nSelectedUnitUID )
		return true;

	// And this is the trigger for the field's monsters, which is the whole reason
	// it lives on this packet rather than on the join ACK. The client builds this
	// push out of GetMyUnit()->GetLastTouchLineIndex()
	// (X2BattleFieldGame.cpp:1014), so its arrival is proof that
	// CX2StateBattleField exists and the player's unit is in the world - which is
	// exactly the condition Handler_EGS_BATTLE_FIELD_NPC_LOAD_NOT branches on
	// (X2StateBattleField.cpp:1141). Sending the monsters from the join ACK
	// instead would race the XGM_STATE_CHANGE that creates that state.
	PushFieldNpcs( kSes );

	// ...and the field's heartbeat. This push is the closest thing a field has to
	// a clock - it arrives as the player moves - so respawns and the middle boss
	// ride it. A player standing perfectly still stops receiving it and the field
	// stops refilling, which is a real limitation but a harmless one: nothing is
	// being killed either.
	TickField( kSes );

	// KGSUser's own guard (GSUserBattleField.cpp:1032): a position for a map the
	// player is not on is stale, and writing it would move them somewhere they
	// have never been. Offline the current field is the room's.
	if( false == m_kRoom.m_bActive ||
		kReq.m_kLastPos.m_iMapID != m_kRoom.m_kInfo.m_iBattleFieldID )
	{
		return true;
	}

	CX2OfflineDB::Instance()->SaveLastFieldPos( kSes.m_nSelectedUnitUID,
		(int)kReq.m_kLastPos.m_ucLastTouchLineIndex,
		(int)kReq.m_kLastPos.m_usLastPosValue );

	return true;
}

#endif SERV_IRUHADEV_OFFLINE
