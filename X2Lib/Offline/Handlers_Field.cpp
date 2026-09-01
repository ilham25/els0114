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
//                EGS_JOIN_BATTLE_FIELD_REQ / _ACK
//                  The village portal out to a field. Phase 4 owns this; for
//                  now it is refused cleanly instead of hanging - see the
//                  handler.
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

	return Reply( kSes, EGS_FIELD_LOADING_COMPLETE_ACK, kAck );
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

bool CX2OfflineServer::Handler_EGS_JOIN_BATTLE_FIELD_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_JOIN_BATTLE_FIELD_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Walking into a village portal sends this. It is NOT phase 3 work: a real
	// ACK has to carry a KRoomInfo and a KRoomSlotInfo list, and its handler
	// (X2State.cpp:10739) then calls ConnectRelayServer() and switches to
	// XS_BATTLE_FIELD - that is phase 4's room emulation, in full.
	//
	// Left unanswered the client just hangs on the portal: the request arms
	// AddServerPacket( EGS_JOIN_BATTLE_FIELD_ACK ) and nothing else clears it.
	// So refuse it properly. Unlike EGS_STATE_CHANGE_FIELD_ACK, the failure path
	// here does not re-send - Handler_EGS_JOIN_BATTLE_FIELD_ACK just returns
	// false - so an error is safe.
	//
	// ERR_BATTLEFIELD_00 is "not in a state that can move to that area", which
	// is both true and the honest message. It is not one of the two codes the
	// handler special-cases (ERR_BATTLEFIELD_13 / _14 print a required level or
	// dungeon, which would be a lie), so it falls through to
	// CX2Main::IsValidPacket's default branch: one OK dialog with that text, and
	// the player stays in the village.
	KEGS_JOIN_BATTLE_FIELD_ACK kAck;
	kAck.Initialize();
	kAck.m_iOK = NetError::ERR_BATTLEFIELD_00;

	CX2OfflineLog::Server( L"FIELD    refused battlefield ID=%d - phase 4 owns room emulation",
		kReq.m_iBattleFieldID );

	return Reply( kSes, EGS_JOIN_BATTLE_FIELD_ACK, kAck );
}

#endif SERV_IRUHADEV_OFFLINE
