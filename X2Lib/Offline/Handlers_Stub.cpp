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

#endif SERV_IRUHADEV_OFFLINE
