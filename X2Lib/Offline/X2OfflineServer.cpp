#include "stdafx.h"
#include "X2OfflineServer.h"

#ifdef SERV_IRUHADEV_OFFLINE

CX2OfflineServer* CX2OfflineServer::ms_pInstance = NULL;

//////////////////////////////////////////////////////////////////////////

CX2OfflineServer::CX2OfflineServer()
: m_nUserUID( 0 )
, m_iUnitSlots( CX2OfflineDB::DEFAULT_UNIT_SLOTS )
, m_wstrLoginID( L"" )
{
}

CX2OfflineServer::~CX2OfflineServer()
{
}

CX2OfflineServer* CX2OfflineServer::Instance()
{
	if( NULL == ms_pInstance )
	{
		CX2OfflineLog::Open();

		ms_pInstance = new CX2OfflineServer;
		g_pX2OfflineHook = ms_pInstance;

		CX2OfflineLog::Server( L"---- offline server up (SERV_IRUHADEV_OFFLINE, phase 2) ----" );

		// els_db.sql sits next to the two logs, in the process working
		// directory - which is the game data\ folder (X2Main mounts the .kom
		// archives through a "./" prefix, so it can be nothing else).
		CX2OfflineDB::Instance()->Open( L"els_db.sql" );
	}

	return ms_pInstance;
}

void CX2OfflineServer::Release()
{
	if( NULL == ms_pInstance )
		return;

	CX2OfflineLog::Server( L"---- offline server down ----" );

	g_pX2OfflineHook = NULL;
	delete ms_pInstance;
	ms_pInstance = NULL;

	CX2OfflineDB::Release();
	CX2OfflineLog::Close();
}

/*static*/ const wchar_t* CX2OfflineServer::KindStr( PROXY_KIND eKind )
{
	switch( eKind )
	{
	case PK_GAME:		return L"GS";
	case PK_RELAY:		return L"TR";
	case PK_CHANNEL:	return L"CH";
	case PK_COLLECT:	return L"CS";
	default:			return L"??";
	}
}

/*static*/ CX2OfflineServer::PROXY_KIND CX2OfflineServer::KindFromEventID( unsigned short usEventID )
{
	// KActorProxy::GetClassID() looks like the obvious way to tell the channel
	// proxy from the game proxy, and it is NOT usable from X2Lib:
	//
	//   X2ServerProtocol/StdAfx.h defines ADD_COLLECT_CLIENT_INFO_PROTOCOL
	//   before it includes X2ServerProtocolLib.h, and that macro adds a member
	//   (m_pSADatabase) to KSession. X2Lib reaches the same header through
	//   X2Main.h, where Session.h is included BEFORE X2ServerProtocol.h defines
	//   the macro - so X2Lib's KSession is 4 bytes smaller, and every member of
	//   the derived KActorProxy/KUserProxy reads 4 bytes low from X2Lib.
	//   Observed directly: GetClassID() returned -1, which is the value of
	//   m_nAckOK, the int declared immediately before m_iClassID.
	//
	// Classify from the event ID's own namespace instead - data both projects
	// agree on, since the IDs come from the shared EventID_Client.h.
	const wchar_t* szName = CX2OfflineLog::EventName( usEventID );
	if( NULL == szName )
		return PK_UNKNOWN;

	if( 0 == wcsncmp( szName, L"ECH_", 4 ) )	return PK_CHANNEL;
	if( 0 == wcsncmp( szName, L"ETR_", 4 ) )	return PK_RELAY;
	if( 0 == wcsncmp( szName, L"ECS_", 4 ) )	return PK_COLLECT;

	return PK_GAME;		///< EGS_*, ENX_*, and the E_* system events
}

/*static*/ std::wstring CX2OfflineServer::NowString()
{
	SYSTEMTIME st;
	::GetLocalTime( &st );

	wchar_t szBuff[32];
	_snwprintf( szBuff, 32, L"%04d-%02d-%02d %02d:%02d:%02d",
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond );
	szBuff[31] = L'\0';

	return std::wstring( szBuff );
}

//////////////////////////////////////////////////////////////////////////
// IX2OfflineHook

void CX2OfflineServer::OnSessionConnect( KSession* pSession, const char* szIP, unsigned short usPort )
{
	if( NULL == pSession )
		return;

	// The kind is left PK_UNKNOWN here and pinned by the session's first packet
	// (see OnClientSend / KindFromEventID) - the proxy object itself cannot be
	// asked from X2Lib.
	{
		KLocker lock( m_cs );

		KOfflineSession kSes;
		kSes.m_pSession	= pSession;
		kSes.m_eKind	= PK_UNKNOWN;
		kSes.m_eState	= S_INIT;

		m_mapSession[ pSession ] = kSes;
	}

	std::wstring wstrIP;
	ConvertCharToWCHAR( wstrIP, ( NULL != szIP ) ? szIP : "" );

	CX2OfflineLog::Server( L"CONNECT  session=0x%08X  (would have been %s:%u)",
		(unsigned int)pSession, wstrIP.c_str(), (unsigned int)usPort );
}

void CX2OfflineServer::OnSessionClose( KSession* pSession )
{
	if( NULL == pSession )
		return;

	PROXY_KIND eKind = PK_UNKNOWN;
	bool bFound = false;

	{
		KLocker lock( m_cs );

		std::map< KSession*, KOfflineSession >::iterator mit = m_mapSession.find( pSession );
		if( mit != m_mapSession.end() )
		{
			eKind  = mit->second.m_eKind;
			bFound = true;
			m_mapSession.erase( mit );
		}
	}

	if( true == bFound )
	{
		CX2OfflineLog::Server( L"CLOSE    %s  session=0x%08X",
			KindStr( eKind ), (unsigned int)pSession );
	}
}

bool CX2OfflineServer::OnClientSend( KSession* pSession, const KEvent& kEvent )
{
	if( NULL == pSession )
		return true;

	// A session the offline server never saw connect - nothing sensible to do
	// with the packet, but it must not fall through to the dead socket path.
	KOfflineSession kSes;
	{
		KLocker lock( m_cs );

		std::map< KSession*, KOfflineSession >::iterator mit = m_mapSession.find( pSession );
		if( mit == m_mapSession.end() )
		{
			CX2OfflineLog::Packet( true, L"??", kEvent.m_usEventID,
				kEvent.m_kbuff.GetLength(), L"*** UNKNOWN SESSION - DROPPED ***" );
			return true;
		}

		kSes = mit->second;
	}

	// system-level chatter the emulator has no use for; consume it silently
	switch( kEvent.m_usEventID )
	{
	case E_HEART_BEAT:
	case E_CHECK_SEQUENCE_COUNT_NOT:
		return true;
	default:
		break;
	}

	// Pin which server this session is standing in for, from its first packet.
	if( PK_UNKNOWN == kSes.m_eKind )
	{
		kSes.m_eKind = KindFromEventID( kEvent.m_usEventID );

		CX2OfflineLog::Server( L"SESSION  0x%08X is the %s proxy",
			(unsigned int)pSession, KindStr( kSes.m_eKind ) );
	}

	// The handler's replies are logged from inside Reply(); buffer them so this
	// request's line lands above them in the file (see CX2OfflineLog::DeferEnd).
	CX2OfflineLog::DeferBegin();

	bool bHandled = false;
	try
	{
		bHandled = Dispatch( kSes, kEvent );
	}
	catch( ... )
	{
		CX2OfflineLog::Server( L"EXCEPTION in handler for %s (id=%u) - consumed",
			CX2OfflineLog::EventName( kEvent.m_usEventID ), (unsigned int)kEvent.m_usEventID );
		bHandled = true;
	}

	CX2OfflineLog::DeferEnd( true, KindStr( kSes.m_eKind ), kEvent.m_usEventID,
		kEvent.m_kbuff.GetLength(),
		bHandled ? L"HANDLED" : L"*** UNHANDLED ***" );

	// Write the (possibly advanced) session state back.
	{
		KLocker lock( m_cs );

		std::map< KSession*, KOfflineSession >::iterator mit = m_mapSession.find( pSession );
		if( mit != m_mapSession.end() )
			mit->second = kSes;
	}

	// Always consume: an unhandled packet must never reach the dead socket path.
	return true;
}

bool CX2OfflineServer::ReplyID( KOfflineSession& kSes, unsigned short usEventID )
{
	if( NULL == kSes.m_pSession )
		return false;

	UidType anTrace[2] = { -1, -1 };

	KEventPtr spEvent( new KEvent );
	spEvent->SetData( PI_GS_PROXY, anTrace, usEventID );

	CX2OfflineLog::Packet( false, KindStr( kSes.m_eKind ), usEventID, 0, L"" );

	kSes.m_pSession->QueueingEvent( spEvent );
	return true;
}

//////////////////////////////////////////////////////////////////////////

/*static*/ void CX2OfflineServer::MakeDefaultUnitInfo( KUnitInfo& kOut,
													   UidType nOwnerUserUID,
													   UidType nUnitUID,
													   char cUnitClass,
													   const std::wstring& wstrNickName,
													   int iLevel )
{
	// KUnitInfo::Init() zeroes most of the ~70 fields already; only the ones it
	// leaves alone, plus the ones that actually mean something, are set here.
	kOut.Init();

	kOut.m_iOwnerUserUID	= nOwnerUserUID;
	kOut.m_nUnitUID			= nUnitUID;
	kOut.m_cUnitClass		= cUnitClass;
	kOut.m_wstrNickName		= wstrNickName;
	kOut.m_ucLevel			= (UCHAR)iLevel;
	kOut.m_cAuthLevel		= (char)CX2User::XUAL_NORMAL_USER;

	// not covered by Init()
	kOut.m_iOfficialMatchCnt	= 0;
	kOut.m_iMaxRating			= 0;
	kOut.m_bIsWinBeforeMatch	= false;
	kOut.m_iPastSeasonWin		= 0;

	// Reuse the client's own EXP table rather than duplicating it.
	if( NULL != g_pData && NULL != g_pData->GetEXPTable() )
	{
		kOut.m_iEXP					= g_pData->GetEXPTable()->GetEXPData( iLevel ).m_nTotalExp;
		kOut.m_nNowBaseLevelEXP		= g_pData->GetEXPTable()->GetEXPData( iLevel ).m_nTotalExp;
		kOut.m_nNextBaseLevelEXP	= g_pData->GetEXPTable()->GetEXPData( iLevel + 1 ).m_nTotalExp;
	}

	kOut.m_iED			= 0;
	kOut.m_iSpiritMax	= 0;
	kOut.m_iSpirit		= 0;

	kOut.m_bDeleted		= false;
	kOut.m_wstrLastDate	= NowString();
}


//////////////////////////////////////////////////////////////////////////
// Dispatch lives here rather than in a Handlers_*.cpp so that each of those
// files stays a flat list of handlers as the phases add more of them.

bool CX2OfflineServer::Dispatch( KOfflineSession& kSes, const KEvent& kEvent )
{
	switch( kEvent.m_usEventID )
	{
	//////////////////////////////////////////////////////////////////////////
	// channel server (PI_CHANNEL_USER) - Handlers_Login.cpp
	case ECH_VERIFY_ACCOUNT_REQ:			return Handler_ECH_VERIFY_ACCOUNT_REQ( kSes, kEvent );
	case ECH_GET_SERVERGROUP_LIST_REQ:		return Handler_ECH_GET_SERVERGROUP_LIST_REQ( kSes, kEvent );
	case ECH_GET_CHANNEL_LIST_REQ:			return Handler_ECH_GET_CHANNEL_LIST_REQ( kSes, kEvent );
	case ECH_DISCONNECT_REQ:				return Handler_ECH_DISCONNECT_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// game server (PI_GS_USER), login chain - Handlers_Login.cpp
	case EGS_CONNECT_REQ:					return Handler_EGS_CONNECT_REQ( kSes, kEvent );
	case EGS_VERIFY_ACCOUNT_REQ:			return Handler_EGS_VERIFY_ACCOUNT_REQ( kSes, kEvent );
	case EGS_CHECK_MACHINE_ID_REQ:			return Handler_EGS_CHECK_MACHINE_ID_REQ( kSes, kEvent );
	case EGS_STATE_CHANGE_SERVER_SELECT_REQ:return Handler_EGS_STATE_CHANGE_SERVER_SELECT_REQ( kSes, kEvent );
	case EGS_CURRENT_TIME_REQ:				return Handler_EGS_CURRENT_TIME_REQ( kSes, kEvent );
	case EGS_SELECT_SERVER_SET_REQ:			return Handler_EGS_SELECT_SERVER_SET_REQ( kSes, kEvent );
	case EGS_CHECK_BALANCE_REQ:				return Handler_EGS_CHECK_BALANCE_REQ( kSes, kEvent );
	case EGS_KEYBOARD_MAPPING_INFO_WRITE_REQ:
											return Handler_EGS_KEYBOARD_MAPPING_INFO_WRITE_REQ( kSes, kEvent );
	case EGS_DISCONNECT_FOR_SERVER_SELECT_REQ:
											return Handler_EGS_DISCONNECT_FOR_SERVER_SELECT_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// character CRUD - Handlers_Unit.cpp
	case EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ:
											return Handler_EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ( kSes, kEvent );
	case EGS_CREATE_UNIT_REQ:				return Handler_EGS_CREATE_UNIT_REQ( kSes, kEvent );
	case EGS_DELETE_UNIT_REQ:				return Handler_EGS_DELETE_UNIT_REQ( kSes, kEvent );
	case EGS_FINAL_DELETE_UNIT_REQ:			return Handler_EGS_FINAL_DELETE_UNIT_REQ( kSes, kEvent );
	case EGS_RESTORE_UNIT_REQ:				return Handler_EGS_RESTORE_UNIT_REQ( kSes, kEvent );
	case EGS_SELECT_UNIT_REQ:				return Handler_EGS_SELECT_UNIT_REQ( kSes, kEvent );
	case EGS_GET_MY_INVENTORY_REQ:			return Handler_EGS_GET_MY_INVENTORY_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// answered only because the client blocks on them - Handlers_Stub.cpp
	case EGS_GET_PET_LIST_REQ:				return Handler_EGS_GET_PET_LIST_REQ( kSes, kEvent );
	case EGS_GET_RIDING_PET_LIST_REQ:		return Handler_EGS_GET_RIDING_PET_LIST_REQ( kSes, kEvent );

	default:
		break;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::EnsureAccount( KOfflineSession& kSes, const std::wstring& wstrLoginID )
{
	std::wstring wstrID = wstrLoginID;
	if( true == wstrID.empty() )
		wstrID = m_wstrLoginID;
	if( true == wstrID.empty() )
		wstrID = L"offline";

	// Already resolved on this server object, and the ID has not changed:
	// nothing to look up. The client rebuilds its proxies several times per
	// session, so this runs far more often than an account is created.
	if( 0 != m_nUserUID && wstrID == m_wstrLoginID )
	{
		kSes.m_nUserUID		= m_nUserUID;
		kSes.m_wstrLoginID	= m_wstrLoginID;
		return true;
	}

	UidType nUserUID	= 0;
	int		iUnitSlots	= CX2OfflineDB::DEFAULT_UNIT_SLOTS;

	if( false == CX2OfflineDB::Instance()->GetOrCreateAccount( wstrID, nUserUID, iUnitSlots ) )
	{
		CX2OfflineLog::Server( L"ERROR    could not resolve account '%s' - the save file is unusable",
			wstrID.c_str() );
		return false;
	}

	m_nUserUID		= nUserUID;
	m_iUnitSlots	= iUnitSlots;
	m_wstrLoginID	= wstrID;

	kSes.m_nUserUID		= nUserUID;
	kSes.m_wstrLoginID	= wstrID;

	return true;
}

/*static*/ void CX2OfflineServer::MakeUnitInfoFromRow( KUnitInfo& kOut, const KOfflineUnitRow& kRow )
{
	MakeDefaultUnitInfo( kOut, kRow.m_nUserUID, kRow.m_nUnitUID,
		(char)kRow.m_iUnitClass, kRow.m_wstrNickName, kRow.m_iLevel );

	kOut.m_iEXP					= kRow.m_iEXP;
	kOut.m_iED					= kRow.m_iED;
	kOut.m_iSPoint				= kRow.m_iSP;
	kOut.m_iSpirit				= kRow.m_iSpirit;
	kOut.m_kLastPos.m_iMapID	= kRow.m_iLastPos;
	kOut.m_wstrLastDate			= CX2OfflineDB::FormatDate( kRow.m_tLastDate );

	// The character-select screen draws its whole delete/restore UI from these
	// three: m_bDeleted picks the deleted slot layout, m_trDelAbleDate is both
	// the date printed on it and the gate on the final-delete button
	// (CreateUnitButton compares it against GetServerCurrentTime64), and
	// m_trRestoreAbleDate would gate restore - offline it never does.
	kOut.m_bDeleted				= kRow.IsDeleted();
	kOut.m_trDelAbleDate		= kRow.IsDeleted()
									? CX2OfflineDB::DelAbleDate( kRow.m_tDelDate )
									: 0LL;
	kOut.m_trRestoreAbleDate	= 0LL;
}

#endif SERV_IRUHADEV_OFFLINE
