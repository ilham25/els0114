#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-08-31
// Description: Offline mode - the abstract hook that the in-process server
//              emulator (CX2OfflineServer, implemented in X2Lib) registers
//              itself through. It is declared here, in X2ServerProtocol,
//              because the socket layer must not depend on X2Lib.
//
//              This header is deliberately NOT included from
//              X2ServerProtocolLib.h: X2Lib pulls X2ServerProtocolLib.h in
//              from X2Main.h BEFORE KTDX.h/Always.h, so SERV_IRUHADEV_OFFLINE
//              is not yet defined at that point. Any flag-conditional
//              declaration on that include path would give X2Lib and
//              X2ServerProtocol different class layouts. Include this header
//              explicitly, after the flag is visible.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

class KEvent;
class KSession;

class IX2OfflineHook
{
public:
	virtual ~IX2OfflineHook() {}

	/// return true if the offline server consumed the packet (it must never reach a socket)
	virtual bool OnClientSend( KSession* pSession, const KEvent& kEvent ) = 0;
	virtual void OnSessionConnect( KSession* pSession, const char* szIP, unsigned short usPort ) = 0;
	virtual void OnSessionClose( KSession* pSession ) = 0;
};

extern IX2OfflineHook* g_pX2OfflineHook;    ///< NULL unless the offline server registered itself

#endif SERV_IRUHADEV_OFFLINE
