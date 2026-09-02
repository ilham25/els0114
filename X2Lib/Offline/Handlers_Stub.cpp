#include "stdafx.h"
#include "X2OfflineServer.h"

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-01
// Description: Offline mode - packets that must be answered but have nothing
//              to say yet.
//
//              These are not the same thing as the UNHANDLED catch-all in
//              OnClientSend. A packet lands here because dropping it leaves
//              the client stuck: it is either waiting on the ACK
//              (AddServerPacket) or holding an in-flight latch that only the
//              ACK clears. Everything else can safely be logged and dropped.
//
//              As later phases give these subsystems real state, handlers move
//              out of this file into the phase's own Handlers_*.cpp.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

//////////////////////////////////////////////////////////////////////////
// pets - the whole subsystem is out of scope until later; both of these fire
// unprompted right after EGS_SELECT_UNIT_ACK.

bool CX2OfflineServer::Handler_EGS_GET_PET_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GET_PET_LIST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// CX2UIPetInfo::Handler_EGS_GET_PET_LIST_REQ sets m_bProcessPetList and
	// refuses to ask again until the ACK clears it, so an unanswered request
	// wedges the pet window shut for the rest of the run - no timeout, no
	// error, just a UI that never refreshes again.
	KEGS_GET_PET_LIST_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_nTotalPage	= 1;					///< one empty page, not zero pages
	kAck.m_nViewPage	= kReq.m_nViewPage;
	kAck.m_vecPetList.clear();

	return Reply( kSes, EGS_GET_PET_LIST_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_GET_RIDING_PET_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GET_RIDING_PET_LIST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// This one is a hard 60s AddServerPacket wait.
	KEGS_GET_RIDING_PET_LIST_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;
	kAck.m_vecRidingPetList.clear();

	return Reply( kSes, EGS_GET_RIDING_PET_LIST_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// anti-cheat bookkeeping - deliberately ignored, and listed here rather than
// left to the UNHANDLED catch-all so the packet log stays signal-only.

bool CX2OfflineServer::Handler_EGS_MODULE_INFO_UPDATE_NOT( KOfflineSession& /*kSes*/, const KEvent& /*kEvent*/ )
{
	// ~3.4KB of loaded-module names, pushed once per session from
	// X2State.cpp:9070 so the operator can spot injected DLLs. No reply, and
	// nothing offline would ever look at it. Not even deserialized: the payload
	// is a list of strings this build has no use for, and parsing it would only
	// create a way for the handler to fail.
	return true;
}

//////////////////////////////////////////////////////////////////////////
// In-match telemetry the real server recorded and nothing offline consumes.
// Listed here rather than left to the catch-all for the same reason as the
// module list: every line left in the UNHANDLED bucket costs attention on the
// next run, and none of these hangs anything - they carry no ACK the client
// waits on.

bool CX2OfflineServer::Handler_EGS_DUNGEON_PLAY_INFO_TO_SERVER_NOT( KOfflineSession& /*kSes*/, const KEvent& /*kEvent*/ )
{
	// Pushed once as a dungeon run ends (X2Game.cpp). The studio fed it to its
	// statistics DB; the numbers that matter for play - scores and ranks - arrive
	// separately in EGS_MY_USER_UNIT_INFO_TO_SERVER_REQ, which is handled.
	return true;
}

bool CX2OfflineServer::Handler_EGS_FRAME_AVERAGE_REQ( KOfflineSession& /*kSes*/, const KEvent& /*kEvent*/ )
{
	// Average framerate, for the studio's performance dashboards. Its ACK exists
	// but the client arms no wait on it, so silence is correct.
	return true;
}

bool CX2OfflineServer::Handler_EGS_REQUEST_GET_AUTO_PARTY_BONUS_INFO_NOT( KOfflineSession& /*kSes*/, const KEvent& /*kEvent*/ )
{
	// Pushed every time the local map opens (X2LocalMapUI.cpp:519) to ask which
	// dungeons currently carry an auto-party recruitment bonus. The reply,
	// EGS_UPDATE_AUTO_PARTY_BONUS_INFO_NOT, only decorates the hero-recruit
	// button; no wait is armed on it, and there is no auto-party offline. Left
	// unanswered on purpose - an empty bonus map is exactly what the client
	// already assumes.
	return true;
}

bool CX2OfflineServer::Handler_EGS_CLIENT_QUIT_REQ( KOfflineSession& /*kSes*/, const KEvent& /*kEvent*/ )
{
	// Sent as the client shuts down. Its ACK would let the server close the
	// session cleanly; offline the process is going away regardless, and
	// ~KSession already tells the offline server through OnSessionClose.
	return true;
}

#endif SERV_IRUHADEV_OFFLINE
