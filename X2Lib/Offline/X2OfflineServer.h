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
//              Phase 2: SQLite behind it (X2OfflineDB) and real character CRUD.
//              Phase 3: real base stats (X2OfflineStatTable) and field entry.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

#include "OfflineHook.h"
#include "X2OfflineLog.h"
#include "X2OfflineDB.h"
#include "X2OfflineStatTable.h"

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

		/// Last gauge values written to SQLite, so the three-second
		/// EGS_UPDATE_PLAY_STATUS_NOT only touches the file when something
		/// actually moved. -1 means "nothing written yet on this session".
		int				m_iSavedHP;
		int				m_iSavedMP;
		int				m_iSavedHyper;
		int				m_iSavedAbil;
		std::wstring	m_wstrLoginID;
		std::wstring	m_wstrPassport;

		KOfflineSession()
			: m_pSession( NULL )
			, m_eKind( PK_UNKNOWN )
			, m_eState( S_INIT )
			, m_nUserUID( 0 )
			, m_nSelectedUnitUID( 0 )
			, m_iSavedHP( -1 )
			, m_iSavedMP( -1 )
			, m_iSavedHyper( -1 )
			, m_iSavedAbil( -1 )
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

	/// MakeDefaultUnitInfo plus everything the `unit` row persists, including
	/// the soft-delete state the character-select screen renders its restore
	/// and final-delete UI from.
	static void MakeUnitInfoFromRow( KUnitInfo& kOut, const KOfflineUnitRow& kRow );

	/// The live HP/MP the client's HUD is seeded from. Sent in
	/// EGS_SELECT_UNIT_4_NOT; without it CX2GageManager never receives a max HP
	/// and the health bar renders empty.
	static void MakeGamePlayStatus( const KOfflineUnitRow& kRow, OUT KGamePlayStatus& kOut );

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

	/// Resolve (or create) the single offline account this login ID maps to,
	/// caching it on the server and on the session.
	bool EnsureAccount( KOfflineSession& kSes, const std::wstring& wstrLoginID );

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
	bool Handler_EGS_KEYBOARD_MAPPING_INFO_WRITE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DISCONNECT_FOR_SERVER_SELECT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CHECK_BALANCE_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	//////////////////////////////////////////////////////////////////////////
	// Handlers_Unit.cpp
	bool Handler_EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CREATE_UNIT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DELETE_UNIT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_FINAL_DELETE_UNIT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_RESTORE_UNIT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_SELECT_UNIT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_MY_INVENTORY_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	/// The five SERV_SELECT_UNIT_PACKET_DIVISION notifications, pushed in order
	/// before EGS_SELECT_UNIT_ACK.
	void PushSelectUnitNotifications( KOfflineSession& kSes, const KOfflineUnitRow& kRow );

	//////////////////////////////////////////////////////////////////////////
	// Handlers_Field.cpp
	bool Handler_EGS_STATE_CHANGE_FIELD_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_FIELD_LOADING_COMPLETE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_OPTION_UPDATE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_UPDATE_PLAY_STATUS_NOT( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_FIELD_UNIT_SYNC_DATA_NOT( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_JOIN_BATTLE_FIELD_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	//////////////////////////////////////////////////////////////////////////
	// Handlers_Stub.cpp - answered because the client blocks on them, nothing more
	bool Handler_EGS_GET_PET_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_RIDING_PET_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_MODULE_INFO_UPDATE_NOT( KOfflineSession& kSes, const KEvent& kEvent );

private:
	static CX2OfflineServer*					ms_pInstance;

	KncCriticalSection							m_cs;
	std::map< KSession*, KOfflineSession >		m_mapSession;

	UidType										m_nUserUID;			///< the single offline account
	int											m_iUnitSlots;
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
