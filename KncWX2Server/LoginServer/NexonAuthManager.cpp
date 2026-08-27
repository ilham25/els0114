#include <WinSock2.h>
#include "NexonAuthManager.h"

#include "Socket/NetCommon.h"
#include "KncSend.h"
#include "NexonAuth.h"
#include "LoginServer.h"
#include "LoginSimLayer.h"

//{{ 2012. 01. 27  ��μ�	�α��� �������� ���� ���� �ڵ� sms ����
#ifdef SERV_NEXON_AUTH_SERVER_DISCONNECT_SMS
	#include "Mornitoring/MornitoringManager.h"
#endif SERV_NEXON_AUTH_SERVER_DISCONNECT_SMS
//}}


//{{ 2010. 06. 04  ������	�ؽ�PC�� ���� ���� ����
#ifdef SERV_PCBANG_AUTH_NEW


DWORD KNexonAuthManager::ms_dwHeartBeatGap( 1 * 60 * 1000 );		// 1�� ���� ��Ʈ��Ʈ
DWORD KNexonAuthManager::ms_dwConnectionCheckGap( 1 * 30 * 1000 );	// 30�� ���� ���� üũ

ImplToStringW( KNexonAuthManager )
{
	return stm_;
}

ImplPfID( KNexonAuthManager, PI_LOGIN_NX_AUTH );

ImplementSingleton( KNexonAuthManager );

KNexonAuthManager::KNexonAuthManager()
{
	m_dwLastHeartBeatTick = ::GetTickCount();
	m_dwLastConnectionCheckTick = ::GetTickCount();
    m_iRecvCP = 0;
}

KNexonAuthManager::~KNexonAuthManager()
{
}

#include <KncLua.h>
#include <lua_tinker.h>
using namespace lua_tinker;

void KNexonAuthManager::RegToLua()
{
	lua_tinker::class_add<KNexonAuthManager>( g_pLua, "KNexonAuthManager" );
	lua_tinker::class_def<KNexonAuthManager>( g_pLua, "dump",				 &KNexonAuthManager::Dump );
	lua_tinker::class_def<KNexonAuthManager>( g_pLua, "InitNexonAuthInfo",	 &KNexonAuthManager::InitNexonAuthInfo_LUA );
	lua_tinker::class_def<KNexonAuthManager>( g_pLua, "SetNumThread",		 &KNexonAuthManager::Init );
	lua_tinker::class_def<KNexonAuthManager>( g_pLua, "Disconnect",			 &KNexonAuthManager::DisconnectAuthServer_LUA );

	lua_tinker::decl( g_pLua, "NXAuthManager", this );
}

void KNexonAuthManager::InitNexonAuthInfo_LUA( const char* szNexonAuthIP, unsigned short usPort, int iDomain, const char* szDomainName, int iSynchronizeType, int iSynchronizeCount )
{
	m_kNexonAuthInfo.m_strIP			 = szNexonAuthIP;
	m_kNexonAuthInfo.m_usPort			 = usPort;
	m_kNexonAuthInfo.m_iDomain			 = iDomain;
	m_kNexonAuthInfo.m_wstrDomainName	 = KncUtil::toWideString_UTF8( std::string( szDomainName ) );
	m_kNexonAuthInfo.m_iSynchronizeType	 = iSynchronizeType;
	m_kNexonAuthInfo.m_iSynchronizeCount = iSynchronizeCount;
}

void KNexonAuthManager::DisconnectAuthServer_LUA()
{
	START_LOG( cout, L"�׽�Ʈ�� ���� ���� ���� ���� ����!" );

    CLOSE_SOCKET( m_sock );
}

void KNexonAuthManager::Init( int nThreadNum )
{
    m_iRecvCP = 0;

    //////////////////////////////////////////////////////////////////////////
    // thread setting : recv�� recvfrom() �Լ����� �˾Ƽ� block�ȴ�.
    //{{ Iruha : 2026-08-27 // VS2010 port: bare Class::Method as a member-function pointer
    // was a VC7.1 extension; VC10 requires the explicit &.
    m_spThreadRecv = boost::shared_ptr< KTThread< KNexonAuthManager > >
        ( new KTThread< KNexonAuthManager >( *this, &KNexonAuthManager::Recv, 50 ) );

    m_spThreadSend = boost::shared_ptr< KTThread< KNexonAuthManager > >
        ( new KTThread< KNexonAuthManager >( *this, &KNexonAuthManager::Send, 100 ) );
    //}}

	KThreadManager::Init( nThreadNum );
}

KThread* KNexonAuthManager::CreateThread()
{
	return new KNexonAuth;
}

void KNexonAuthManager::BeginThread()
{
    if( m_spThreadRecv )
    {
	    m_spThreadRecv->Begin();
    }

    if( m_spThreadSend )
    {
	    m_spThreadSend->Begin();
    }

	KThreadManager::BeginThread();

    if( !Connect() )
    {
        START_LOG( cerr, L"�ؽ� PC�� ���� ���� ���� ����." )
            << END_LOG;
    }
}

void KNexonAuthManager::EndThread()
{
	KThreadManager::EndThread();

    if( m_spThreadSend )
    {
	    m_spThreadSend->End( 10000 );
    }
    
    if( m_spThreadRecv )
    {
	    m_spThreadRecv->End( 3000 );
    }
}

void KNexonAuthManager::Tick()
{
    CheckConnection();

    KeepConnection();
}

void KNexonAuthManager::QueueingSendPacket( const KNexonAuthPacketPtr& spPacket )
{
    JIF( spPacket );

    KLocker lock( m_csSendQueue );

    m_kSendQueue.push( spPacket );
}

void KNexonAuthManager::Recv()
{
	if( !IsConnected() )
	{
        return;
	}

    int ret;
    ret = ::recv( m_sock,
        m_cRecvBuffer + m_iRecvCP,
        MAX_PACKET_SIZE_OF_NEXON_AUTH - m_iRecvCP,
        0 );

    START_LOG( clog, L"��Ŷ ����." )
        << BUILD_LOG( ret );

    if( ret == SOCKET_ERROR )
    {
        START_LOG( cerr, GET_WSA_MSG );
        CLOSE_SOCKET( m_sock );

		//{{ 2010. 10. 11	������	������ ���� ���� �α�
#ifdef SERV_SERVER_DISCONNECT_LOG
		CTime kRegDate = CTime::GetCurrentTime();
		KE_LOCAL_LOG_SERVER_DISCONNECT_NOT kNot;
		kNot.m_cLogType				= KE_LOCAL_LOG_SERVER_DISCONNECT_NOT::SDLT_DISCONNECT;
		kNot.m_wstrSourceName		= boost::str( boost::wformat( L"%d" ) % KBaseServer::GetKObj()->GetUID() );
		kNot.m_wstrDestinationName	= L"Nexon Auth";
		kNot.m_wstrDestinationIP	= KncUtil::toWideString( m_kNexonAuthInfo.m_strIP );
		kNot.m_wstrReason			= L"��ȿ ���� ���� ����";
		kNot.m_wstrRegDate			= (const wchar_t*)kRegDate.Format(_T("%Y-%m-%d %H:%M:%S"));
		KSIManager.QueueingEvent( E_LOCAL_LOG_SERVER_DISCONNECT_NOT, kNot );
#endif SERV_SERVER_DISCONNECT_LOG
		//}}

		//{{ 2012. 01. 27  ��μ�	�α��� �������� ���� ���� �ڵ� sms ����
#ifdef SERV_NEXON_AUTH_SERVER_DISCONNECT_SMS
		{
			KE_DISCONNECT_SERVER_REPORT_NOT kNot;
			kNot.m_cType = KMornitoringManager::ET_DISCONNECT_LOGIN_AUTH;
			KEventPtr spEvent( new KEvent );
			spEvent->SetData( 0, NULL, E_DISCONNECT_SERVER_REPORT_NOT, kNot );
			KBaseServer::GetKObj()->QueueingEvent( spEvent );
		}
#endif SERV_NEXON_AUTH_SERVER_DISCONNECT_SMS
		//}}

        return;
    }

    if( ret == 0 )
    {
        START_LOG( cerr, L"���� ������ ������." )
            << END_LOG;

        CLOSE_SOCKET( m_sock );

		//{{ 2010. 10. 11	������	������ ���� ���� �α�
#ifdef SERV_SERVER_DISCONNECT_LOG
		CTime kRegDate = CTime::GetCurrentTime();
		KE_LOCAL_LOG_SERVER_DISCONNECT_NOT kNot;
		kNot.m_cLogType				= KE_LOCAL_LOG_SERVER_DISCONNECT_NOT::SDLT_DISCONNECT;
		kNot.m_wstrSourceName		= boost::str( boost::wformat( L"%d" ) % KBaseServer::GetKObj()->GetUID() );
		kNot.m_wstrDestinationName	= L"Nexon Auth";
		kNot.m_wstrDestinationIP	= KncUtil::toWideString( m_kNexonAuthInfo.m_strIP );
		kNot.m_wstrReason			= L"���������� ���� ����";
		kNot.m_wstrRegDate			= (const wchar_t*)kRegDate.Format(_T("%Y-%m-%d %H:%M:%S"));
		KSIManager.QueueingEvent( E_LOCAL_LOG_SERVER_DISCONNECT_NOT, kNot );
#endif SERV_SERVER_DISCONNECT_LOG
		//}}
        return;
    }

    m_iRecvCP += ret;

    while( m_iRecvCP >= 4 )
    {
        // ���(1) + ������(2) + Ÿ��(1) = 4
        // ��� ��Ŷ�� 4����Ʈ �̻���

        unsigned short usLength;
        ::memcpy( &usLength, m_cRecvBuffer + 1, sizeof( usLength ) );

        usLength = ::ntohs( usLength );
        int iTotalPacketSize = usLength + 3;
        //if( iTotalPacketSize > MAX_PACKET_SIZE_OF_NEXON_AUTH ) ��μ�
		if( iTotalPacketSize >= MAX_PACKET_SIZE_OF_NEXON_AUTH )
        {
            START_LOG( cerr, L"������ ��Ŷ ������ �̻�." )
                << BUILD_LOG( iTotalPacketSize )
                << BUILD_LOG( MAX_PACKET_SIZE_OF_NEXON_AUTH )
                << END_LOG;

            CLOSE_SOCKET( m_sock );

			//{{ 2010. 10. 11	������	������ ���� ���� �α�
#ifdef SERV_SERVER_DISCONNECT_LOG
			CTime kRegDate = CTime::GetCurrentTime();
			KE_LOCAL_LOG_SERVER_DISCONNECT_NOT kNot;
			kNot.m_cLogType				= KE_LOCAL_LOG_SERVER_DISCONNECT_NOT::SDLT_DISCONNECT;
			kNot.m_wstrSourceName		= boost::str( boost::wformat( L"%d" ) % KBaseServer::GetKObj()->GetUID() );
			kNot.m_wstrDestinationName	= L"Nexon Auth";
			kNot.m_wstrDestinationIP	= KncUtil::toWideString( m_kNexonAuthInfo.m_strIP );
			kNot.m_wstrReason			= boost::str( boost::wformat( L"�ִ� ��Ŷ ������ �ʰ�! MaxPacketSize : %d bytes" ) % iTotalPacketSize );
			kNot.m_wstrRegDate			= (const wchar_t*)kRegDate.Format(_T("%Y-%m-%d %H:%M:%S"));
			KSIManager.QueueingEvent( E_LOCAL_LOG_SERVER_DISCONNECT_NOT, kNot );
#endif SERV_SERVER_DISCONNECT_LOG
			//}}
            return;
        }

        if( iTotalPacketSize > m_iRecvCP )
        {
            return;
        }

        MakeEventFromReceivedPacket();

        if( m_iRecvCP > iTotalPacketSize )
        {
            ::memmove( m_cRecvBuffer, m_cRecvBuffer + iTotalPacketSize, m_iRecvCP - iTotalPacketSize );
        }
        m_iRecvCP -= iTotalPacketSize;
    }
}

void KNexonAuthManager::Send()
{
	if( !IsConnected() )
	{
        return;
	}

	int ret;
	char buf[MAX_PACKET_SIZE_OF_NEXON_AUTH];
    KNexonAuthPacketPtr spPacket;
	while( GetSendPacket( spPacket ) )
	{
		if( !spPacket )
		{
			START_LOG( cerr, L"������ �̻�." )
				<< BUILD_LOG( m_kSendQueue.size() )
				<< END_LOG;

			continue;
		}

		spPacket->WriteToBuffer( ( BYTE* )buf );
        DumpBuffer( ( BYTE* )buf, false );

        char* cCurBufPos = buf;
		int iRemainedSendSize = spPacket->GetTotalLength();
		while( iRemainedSendSize > 0 )
		{
			ret = ::send( m_sock,
				cCurBufPos,
				iRemainedSendSize,
				0 );

			if( ret == SOCKET_ERROR )
			{
				START_LOG( cwarn, GET_WSA_MSG );
				CLOSE_SOCKET( m_sock );
				return;
			}

			iRemainedSendSize -= ret;
            cCurBufPos += ret;
		}
	}
}

bool KNexonAuthManager::Connect()
{
    m_iRecvCP = 0;
	m_sock = ::socket( AF_INET, SOCK_STREAM, 0 );    // ���� ����

	if( INVALID_SOCKET == m_sock )
	{
		START_LOG( cerr, GET_WSA_MSG )
			<< BUILD_LOG( WSAGetLastError() )
			<< END_LOG;

		return false;
	}

	{
		KLocker lock( m_csSendQueue );

		// ���� �����ϴ� �Ŷ�� sendqueue�� ������ �����! 
		// ���� ���� �� ���� ó�� ���� ��Ŷ�� �ʱ�ȭ ��Ŷ�̾�� �ϱ� ����!
		while( !m_kSendQueue.empty() )
		{
			m_kSendQueue.pop();
		}
	}

	SOCKADDR_IN addr;
	addr.sin_family			= AF_INET;
	addr.sin_addr.s_addr	= inet_addr( m_kNexonAuthInfo.m_strIP.c_str() );
	addr.sin_port			= htons( m_kNexonAuthInfo.m_usPort );

	int ret = ::connect( m_sock, ( struct sockaddr* )&addr, sizeof( addr ) );

	if( SOCKET_ERROR == ret )
	{
		START_LOG( cerr, GET_WSA_MSG )
			<< BUILD_LOG( WSAGetLastError() )
			<< BUILD_LOG( m_kNexonAuthInfo.m_strIP )
			<< BUILD_LOG( m_kNexonAuthInfo.m_usPort )
			<< END_LOG;

		CLOSE_SOCKET( m_sock );
		return false;
	}

	// ���� ���� �ο� Ȯ��
	const BYTE byteSynchronizeType = ( GetKLoginSimLayer()->GetConcurrentUserCount() == 0 ? 0 : m_kNexonAuthInfo.m_iSynchronizeType );

	// ���� �õ�
    KENX_AUTH_INITIALIZE_REQ kPacketInit;
    kPacketInit.m_bytePacketType	  = 41;
	kPacketInit.m_byteInitializeType  = 1;
    kPacketInit.m_byteGameSN		  = 29;									  // ���ҵ� ���� ��ȣ
    kPacketInit.m_byteDomainSN		  = m_kNexonAuthInfo.m_iDomain;
    kPacketInit.m_wstrDomainName	  = m_kNexonAuthInfo.m_wstrDomainName;
	kPacketInit.m_byteSynchronizeType = byteSynchronizeType;				  // [����] 0:����ȭ ����, 1:���ǹ�ȣ ����ȭ, 2:���ǹ�ȣ,������ ���� ���̵� ����ȭ
	kPacketInit.m_ulSynchronizeCount  = m_kNexonAuthInfo.m_iSynchronizeCount; // ��Ŷ�� ����ȭ�� ��� ���� ��

    boost::shared_ptr< KNexonAuthPacket > spPacket( new KNexonAuthPacket );
    spPacket->Write( kPacketInit );
    QueueingSendPacket( spPacket );

    START_LOG( cout, L"�ؽ� PC�� ���� ���� ���� ����! �ʱ�ȭ ��Ŷ ����!" )
		<< BUILD_LOG( kPacketInit.m_byteGameSN )
		<< BUILD_LOG( kPacketInit.m_byteDomainSN )
		<< BUILD_LOG( kPacketInit.m_wstrDomainName )
		<< BUILD_LOG( byteSynchronizeType )
		<< BUILD_LOG( m_kNexonAuthInfo.m_iSynchronizeCount );

	// ���Ӽ��� ���� 1�� �ڿ� ��Ʈ��Ʈ�� ������ ���ؼ� ���⼭ TickCount�����صд�.
	m_dwLastHeartBeatTick = ::GetTickCount();
	return true;
}

bool KNexonAuthManager::IsConnected()
{
	return ( m_sock != INVALID_SOCKET );
}

void KNexonAuthManager::KeepConnection()
{
	if( ::GetTickCount() - m_dwLastHeartBeatTick < ms_dwHeartBeatGap )
	{
		return;
	}
	
	if( IsConnected() == false )
	{
		// ������ ���°� �ƴ϶�� ������ �ʴ´�.
		return;
	}

    m_dwLastHeartBeatTick = ::GetTickCount();

	// ��Ʈ�� ������
    KENX_AUTH_ALIVE_NOT kPacketNot;
    kPacketNot.m_bytePacketType = 100;
    boost::shared_ptr< KNexonAuthPacket > spPacket( new KNexonAuthPacket );
    spPacket->Write( kPacketNot );
    QueueingSendPacket( spPacket );
}

void KNexonAuthManager::CheckConnection()
{
	if( ::GetTickCount() - m_dwLastConnectionCheckTick < ms_dwConnectionCheckGap )
	{
		return;
	}

    m_dwLastConnectionCheckTick = ::GetTickCount();

	if( !IsConnected() )
	{
		Connect();
	}
}

bool KNexonAuthManager::GetSendPacket( KNexonAuthPacketPtr& spPacket )
{
    KLocker lock( m_csSendQueue );

    if( m_kSendQueue.empty() )
    {
        return false;
    }

    spPacket = m_kSendQueue.front();
    m_kSendQueue.pop();

    return true;
}

void KNexonAuthManager::MakeEventFromReceivedPacket()
{
    DumpBuffer( ( BYTE* )m_cRecvBuffer, false );
	KNexonAuthPacket kReceivedPacket;
	if( !kReceivedPacket.ReadFromBuffer( ( BYTE* )m_cRecvBuffer ) )
    {
        START_LOG( cerr, L"���ۿ��� �б� ����." )
            << END_LOG;

        DumpBuffer( ( BYTE* )m_cRecvBuffer, true );
        return;
    }

	switch( kReceivedPacket.GetPacketType() )
	{
	case KNexonAuthPacket::NA_PT_INITIALIZE:
		{
			KENX_AUTH_INITIALIZE_ACK kPacket;
			kReceivedPacket.Read( kPacket );

			// ���� ������ ��� ó���ұ�?
			if( kPacket.m_byteResult == 0 )
			{
				START_LOG( cout, L"�ؽ� PC�� �������� �ʱ�ȭ ����!" )
					<< BUILD_LOG( kPacket.m_byteInitializeType )
					<< BUILD_LOG( kPacket.m_byteResult )
					<< BUILD_LOG( kPacket.m_byteDomainSN )
					<< BUILD_LOG( kPacket.m_wstrMessage );
			}
			else
			{
				START_LOG( cerr, L"�ؽ� PC�� �������� �ʱ�ȭ ����!" )
					<< BUILD_LOG( kPacket.m_byteInitializeType )
					<< BUILD_LOG( kPacket.m_byteResult )
					<< BUILD_LOG( kPacket.m_byteDomainSN )
					<< BUILD_LOG( kPacket.m_wstrMessage );

				// ���� ���������Ƿ� ���� ������ �����Ѵ�.
				CLOSE_SOCKET( m_sock );
			}
		}
		break;

	case KNexonAuthPacket::NA_PT_LOGIN:
		{
			KENX_AUTH_LOGIN_ACK kPacket;
			kReceivedPacket.Read( kPacket );

			//{{ 2011. 07. 27    ��μ�    ���Ϸ��� ä�θ�
#ifdef SERV_TOONILAND_CHANNELING
			std::wstring wstrResult = KncUtil::EraseFrontWord( kPacket.m_wstrUserID, L';' );
			kPacket.m_wstrUserID = wstrResult;
#endif SERV_TOONILAND_CHANNELING
			//}}

			KEventPtr spEvent( new KEvent );
			spEvent->SetData( PI_LOGIN_SERVER, NULL, ENX_AUTH_LOGIN_ACK, kPacket );
			KBaseServer::GetKObj()->QueueingEvent( spEvent );
		}
		break;

	case KNexonAuthPacket::NA_PT_TERMINATE:
		{
			KENX_AUTH_TERMINATE_NOT kPacket;
			kReceivedPacket.Read( kPacket );

			//{{ 2011. 07. 27    ��μ�    ���Ϸ��� ä�θ�
#ifdef SERV_TOONILAND_CHANNELING
			std::wstring wstrResult = KncUtil::EraseFrontWord( kPacket.m_wstrUserID, L';' );
			kPacket.m_wstrUserID = wstrResult;
#endif SERV_TOONILAND_CHANNELING
			//}}
            
			KEventPtr spEvent( new KEvent );
			spEvent->SetData( PI_LOGIN_SERVER, NULL, ENX_AUTH_TERMINATE_NOT, kPacket );
			KBaseServer::GetKObj()->QueueingEvent( spEvent );
		}
		break;

	case KNexonAuthPacket::NA_PT_MESSAGE:
		{
			KENX_AUTH_MESSAGE_NOT kPacket;
			kReceivedPacket.Read( kPacket );

			//{{ 2011. 07. 27    ��μ�    ���Ϸ��� ä�θ�
#ifdef SERV_TOONILAND_CHANNELING
			std::wstring wstrResult = KncUtil::EraseFrontWord( kPacket.m_wstrUserID, L';' );
			kPacket.m_wstrUserID = wstrResult;
#endif SERV_TOONILAND_CHANNELING
			//}}

			KEventPtr spEvent( new KEvent );
			spEvent->SetData( PI_LOGIN_SERVER, NULL, ENX_AUTH_MESSAGE_NOT, kPacket );
			KBaseServer::GetKObj()->QueueingEvent( spEvent );
		}
		break;

	case KNexonAuthPacket::NA_PT_SYNCHRONIZE:
		{
			KENX_AUTH_SYNCHRONIZE_REQ kPacket;
			kReceivedPacket.Read( kPacket );
			
			KEventPtr spEvent( new KEvent );
			spEvent->SetData( PI_LOGIN_SERVER, NULL, ENX_AUTH_SYNCHRONIZE_REQ, kPacket );
			KBaseServer::GetKObj()->QueueingEvent( spEvent );
		}
		break;

	default:
		{
			START_LOG( cerr, L"��Ŷ Ÿ�� �̻�." )
				<< BUILD_LOG( kReceivedPacket.GetPacketType() )
				<< END_LOG;

			DumpBuffer( ( BYTE* )m_cRecvBuffer, true );
			return;
		}
		break;
	}
}

void KNexonAuthManager::DumpBuffer( BYTE* buffer, bool bError )
{
    JIF( buffer );
    JIF( sizeof( buffer ) >= 4 );

    unsigned short usLength;
    ::memcpy( &usLength, buffer + 1, sizeof( usLength ) );

    usLength = ::ntohs( usLength );
    int iTotalPacketSize = usLength + 3;

    LIF( iTotalPacketSize <= MAX_PACKET_SIZE_OF_NEXON_AUTH );
    iTotalPacketSize =  std::min< int >( iTotalPacketSize, MAX_PACKET_SIZE_OF_NEXON_AUTH );

    char szBuffer[MAX_PACKET_SIZE_OF_NEXON_AUTH * 3 + 1];
    char szByte[4];

    for( int i = 0; i < iTotalPacketSize; i++ )
    {
        if( ( int )buffer[i] >= 16 )
        {
            ::itoa( ( int )buffer[i], szByte, 16 );
            szBuffer[i * 3] = szByte[0];
            szBuffer[i * 3 + 1] = szByte[1];
            szBuffer[i * 3 + 2] = ' ';
        }
        else
        {
            ::itoa( ( int )buffer[i], szByte, 16 );
            szBuffer[i * 3] = '0';
            szBuffer[i * 3 + 1] = szByte[0];
            szBuffer[i * 3 + 2] = ' ';
        }
    }
    szBuffer[iTotalPacketSize * 3] = '\0';

    std::string strBuffer = szBuffer;

    if( bError )
    {
        START_LOG( cerr2, L"BUFFER : ")
            << BUILD_LOG( strBuffer );
    }
    else
    {
        START_LOG( clog2, L"BUFFER : ")
            << BUILD_LOG( strBuffer );
    }
}


#endif SERV_PCBANG_AUTH_NEW
//}}


