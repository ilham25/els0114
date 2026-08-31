#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-08-31
// Description: Offline mode - the in-process server emulator.
//
//              It implements IX2OfflineHook, so KSession hands it every
//              outbound KEvent instead of serializing and sending it, and it
//              answers by calling KSession::QueueingEvent() on the same
//              session - the exact point the real receive path queues into.
//              Everything above the socket layer runs unmodified.
//
//              Phase 1: no SQLite. Login chain plus one hardcoded character.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

#include "OfflineHook.h"
#include "X2OfflineLog.h"

class CX2OfflineServer : public IX2OfflineHook
{
public:
	enum PROXY_KIND
	{
		PK_UNKNOWN = 0,
		PK_GAME,			///< CX2ServerProtocol::GSPROXY_ID
		PK_RELAY,			///< CX2ServerProtocol::RTPROXY_ID
		PK_CHANNEL,			///< CX2ServerProtocol::CHPROXY_ID
		PK_COLLECT,			///< CX2ServerProtocol::CSPROXY_ID
	};

	/// as much of KncWX2Server/Common/GSFSMState_def.h as offline mode needs
	enum FSM_STATE
	{
		S_INIT = 0,
		S_UID_UPDATED,
		S_LOGINED,
		S_SERVER_SELECT,
		S_FIELD_MAP,
		S_ROOM,
	};

	struct KOfflineSession
	{
		KSession*		m_pSession;
		PROXY_KIND		m_eKind;
		FSM_STATE		m_eState;
		UidType			m_nUserUID;
		UidType			m_nSelectedUnitUID;
		std::wstring	m_wstrLoginID;
		std::wstring	m_wstrPassport;

		KOfflineSession()
			: m_pSession( NULL )
			, m_eKind( PK_UNKNOWN )
			, m_eState( S_INIT )
			, m_nUserUID( 0 )
			, m_nSelectedUnitUID( 0 )
		{
		}
	};

public:
	static CX2OfflineServer*	Instance();
	static void					Release();

	//////////////////////////////////////////////////////////////////////////
	// IX2OfflineHook
	virtual bool OnClientSend( KSession* pSession, const KEvent& kEvent );
	virtual void OnSessionConnect( KSession* pSession, const char* szIP, unsigned short usPort );
	virtual void OnSessionClose( KSession* pSession );

	//////////////////////////////////////////////////////////////////////////
	// helpers every handler uses
	template< typename T > bool ReadReq( const KEvent& kEvent, T& kOut );
	template< typename T > bool Reply( KOfflineSession& kSes, unsigned short usEventID, const T& kData );
	bool ReplyID( KOfflineSession& kSes, unsigned short usEventID );

	/// "YYYY-MM-DD HH:MM:SS" - the only date format KncUtil::ConvertStringToCTime parses
	static std::wstring NowString();

	/// The one place a KUnitInfo gets built. Every later phase reuses it.
	static void MakeDefaultUnitInfo( KUnitInfo& kOut,
									 UidType nOwnerUserUID,
									 UidType nUnitUID,
									 char cUnitClass,
									 const std::wstring& wstrNickName,
									 int iLevel );

	static const wchar_t* KindStr( PROXY_KIND eKind );

	/// Which server a session stands in for, derived from the event ID's own
	/// namespace. See the comment on the definition for why KActorProxy::
	/// GetClassID() must not be read from X2Lib.
	static PROXY_KIND KindFromEventID( unsigned short usEventID );

private:
	CX2OfflineServer();
	virtual ~CX2OfflineServer();

	/// returns true if a handler consumed the packet, false -> log it UNHANDLED
	bool Dispatch( KOfflineSession& kSes, const KEvent& kEvent );

	//////////////////////////////////////////////////////////////////////////
	// Handlers_Login.cpp
	bool Handler_ECH_VERIFY_ACCOUNT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_ECH_GET_SERVERGROUP_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_ECH_GET_CHANNEL_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_ECH_DISCONNECT_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	bool Handler_EGS_CONNECT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_VERIFY_ACCOUNT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CHECK_MACHINE_ID_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_STATE_CHANGE_SERVER_SELECT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CURRENT_TIME_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_SELECT_SERVER_SET_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_KEYBOARD_MAPPING_INFO_WRITE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DISCONNECT_FOR_SERVER_SELECT_REQ( KOfflineSession& kSes, const KEvent& kEvent );

private:
	static CX2OfflineServer*					ms_pInstance;

	KncCriticalSection							m_cs;
	std::map< KSession*, KOfflineSession >		m_mapSession;

	UidType										m_nUserUID;			///< the single offline account
	std::wstring								m_wstrLoginID;
};

//////////////////////////////////////////////////////////////////////////
// The offline server never touches the wire format of the KEvent envelope,
// only the inner payload - and for that it uses the very same KSerializer the
// real server does.

template< typename T >
bool CX2OfflineServer::ReadReq( const KEvent& kEvent, T& kOut )
{
	KSerBuffer* pBuff = const_cast< KSerBuffer* >( &kEvent.m_kbuff );

	// the client sends with bCompress = false by default, but the real receive
	// path uncompresses unconditionally, so do the same
	pBuff->UnCompress();

	KSerializer ks;
	ks.BeginReading( pBuff );
	bool bOK = ks.Get( kOut );
	ks.EndReading();

	if( false == bOK )
	{
		CX2OfflineLog::Server( L"ERROR  deserialize failed for %s (id=%u)",
			CX2OfflineLog::EventName( kEvent.m_usEventID ), (unsigned int)kEvent.m_usEventID );
	}

	return bOK;
}

template< typename T >
bool CX2OfflineServer::Reply( KOfflineSession& kSes, unsigned short usEventID, const T& kData )
{
	if( NULL == kSes.m_pSession )
		return false;

	UidType anTrace[2] = { -1, -1 };

	KEventPtr spEvent( new KEvent );
	spEvent->SetData( PI_GS_PROXY, anTrace, usEventID, kData );

	CX2OfflineLog::Packet( false, KindStr( kSes.m_eKind ), usEventID,
		spEvent->m_kbuff.GetLength(), L"" );

	kSes.m_pSession->QueueingEvent( spEvent );
	return true;
}

#endif SERV_IRUHADEV_OFFLINE
