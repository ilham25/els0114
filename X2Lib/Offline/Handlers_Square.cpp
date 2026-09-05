#include "stdafx.h"
#include "X2OfflineServer.h"

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-05
// Description: Offline mode - phase 13. Cobo Express, the trade square.
//
//              EGS_SQUARE_LIST_REQ was the whole bug: two UNHANDLED entries in
//              the play-test log, 400ms apart. The client's own market-entry
//              flow (CX2TFieldGame::Handler_EGS_SQUARE_LIST_REQ fires
//              ST_TRADE unconditionally - X2TFieldGame.cpp:2729) got no reply,
//              retried once, then gave up into "You cannot enter the village".
//              Nothing about villages is actually involved; the dialog is just
//              what a timed-out market-entry flow happens to show.
//
//              One square exists: the trade market, population 1 (this
//              character), page 1 of 1. An empty list reads the same as a
//              missing village to the client's own UI, so the honest answer
//              here is one occupied slot, not zero.
//
//              Three packets follow the list, all of them small:
//
//                EGS_JOIN_SQUARE_REQ            ACK echoes the same
//                  KSquareInfo back; m_vecUserInfo is empty because there is
//                  nobody else to seed the market with -
//                  CX2TFieldGame::Handler_EGS_JOIN_SQUARE_ACK only calls
//                  JoinSquareUnit() for entries in that vector
//                  (X2TFieldGame.cpp:4130-4134) and never adds "my own" unit,
//                  so an empty vector here is correct, not a stub.
//                EGS_SQUARE_UNIT_SYNC_DATA_REQ  fire-and-forget position
//                  update - same shape as the village's
//                  EGS_FIELD_UNIT_SYNC_DATA_NOT. The real server only
//                  rebroadcasts it to everyone else in the square, and offline
//                  there is nobody else. Unlike a field position, a market
//                  visit is not somewhere the game re-enters the player on
//                  login, so there is nothing to persist either.
//                EGS_LEAVE_SQUARE_REQ           KEGS_LEAVE_SQUARE_ACK
//                  (a bare KPacketOK - ClientPacket.h:517). CX2StateSquareGame
//                  ::Handler_EGS_LEAVE_SQUARE_ACK is what drives the client
//                  back out (Handler_EGS_STATE_CHANGE_FIELD_REQ, already
//                  handled - Handlers_Field.cpp), so nothing else has to
//                  happen here.
//
//              Square is not a KOfflineRoom: it never survives a relog and
//              carries no dungeon/battlefield semantics, so kSes.m_eState is
//              left exactly where EGS_STATE_CHANGE_FIELD_REQ already put it
//              (S_FIELD_MAP) rather than growing a new FSM state for it.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

namespace
{
	/// The one trade square. Never created or destroyed - it just always
	/// exists, the way a real GameServer's market squares do.
	const UidType SQUARE_UID = 1;

	/// Comfortably above the real m_SquareMaxUserNum default, so the client's
	/// own clamp (X2TFieldGame.cpp:4970, g_pMain->GetSquareMaxUserNum()) is
	/// what actually caps the number shown - matching what a live square would
	/// send rather than hardcoding that cap a second time here.
	const char SQUARE_MAX_SLOT = 40;
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_SQUARE_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_SQUARE_LIST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KSquareInfo kInfo;
	kInfo.m_cSquareType	= kReq.m_cSquareType;
	kInfo.m_iSquareUID	= SQUARE_UID;
	kInfo.m_wstrRoomName= L"Cobo Express";
	kInfo.m_MaxSlot		= SQUARE_MAX_SLOT;
	kInfo.m_JoinSlot	= 0;			///< nobody has joined it yet on this list
	kInfo.m_Port		= 0;

	KEGS_SQUARE_LIST_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_nTotalPage	= 1;
	kAck.m_nViewPage	= 1;
	kAck.m_vecSquareInfo.clear();
	kAck.m_vecSquareInfo.push_back( kInfo );

	CX2OfflineLog::Server( L"SQUARE   list requested (type=%d), offering square %I64d",
		(int)kReq.m_cSquareType, (__int64)SQUARE_UID );

	return Reply( kSes, EGS_SQUARE_LIST_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_JOIN_SQUARE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_JOIN_SQUARE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_JOIN_SQUARE_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	kAck.m_kSquareInfo.m_cSquareType	= (char)CX2SquareGame::ST_TRADE;
	kAck.m_kSquareInfo.m_iSquareUID		= kReq.m_iSquareUID;
	kAck.m_kSquareInfo.m_wstrRoomName	= L"Cobo Express";
	kAck.m_kSquareInfo.m_MaxSlot		= SQUARE_MAX_SLOT;
	kAck.m_kSquareInfo.m_JoinSlot		= 1;		///< this character, joining now
	kAck.m_kSquareInfo.m_Port			= 0;

	// Nobody else to seed the market with - see the file header.
	kAck.m_vecUserInfo.clear();

	CX2OfflineLog::Server( L"SQUARE   joining square %I64d", (__int64)kReq.m_iSquareUID );

	return Reply( kSes, EGS_JOIN_SQUARE_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_LEAVE_SQUARE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_LEAVE_SQUARE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_LEAVE_SQUARE_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	CX2OfflineLog::Server( L"SQUARE   leaving square (reason=%d)", (int)kReq.m_iReason );

	return Reply( kSes, EGS_LEAVE_SQUARE_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_SQUARE_UNIT_SYNC_DATA_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_SQUARE_UNIT_SYNC_DATA_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// No reply, nothing to persist - see the file header.
	return true;
}

#endif SERV_IRUHADEV_OFFLINE
