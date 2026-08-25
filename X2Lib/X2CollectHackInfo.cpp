
#include "StdAfx.h"

#ifdef ADD_COLLECT_CLIENT_INFO

#include "X2CollectHackInfo.h"

CX2CollectHackInfo::CX2CollectHackInfo(void)
{
	m_pCollectServer = NULL;
	m_bSendPacket = false;
	m_pHackImgBuf = NULL;
	m_lTotalSize = 0;
	m_iImageOffset = 0;
	m_bRunThread = false;
	::InitializeCriticalSection( &m_csEventQueue );
}


CX2CollectHackInfo::~CX2CollectHackInfo(void)
{
	m_bRunThread = false;
	ClearQueue();

	EndThread(5000);

	::DeleteCriticalSection( &m_csEventQueue );

	SAFE_DELETE_ARRAY( m_pHackImgBuf );
	m_lTotalSize = 0;
	m_iImageOffset = 0;
}

bool CX2CollectHackInfo::BeginThread()
{
	m_bRunThread = true;
	return KJohnThread::BeginThread();
}//BeginThread()

void CX2CollectHackInfo::EndThread(DWORD dwTimeout_)
{				
	KJohnThread::EndThread(dwTimeout_);
}//EndThread()

DWORD CX2CollectHackInfo::RunThread()
{
#if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_START
#endif

	while(1)
	{	
		THEMIDA_ENCODE_START

		if( m_bRunThread == false )
			break;

		DoProcessEvent();
		Sleep(100);

		THEMIDA_ENCODE_END
	}

#if defined( _SERVICE_ ) 
	ELSWORD_VIRTUALIZER_END
#endif
	
	return 0;
}//RunThread()

void CX2CollectHackInfo::ClearQueue()
{
	::EnterCriticalSection( &m_csEventQueue );  // 050630. 이벤트를 처리하는 동안 unlock 되도록 수정.

	// traverse all event
	COLLECT_SERVER_PACKET spEvent;

	while( !m_queEvent.empty() )	
	{
		// 050912. florist. 050911에 넷마블 테섭이 이곳에서 crash를 일으킴. 별도 예외처리.
		try 
		{ 
			spEvent = m_queEvent.front(); 
		} 
		catch( ... ) 
		{ 
			m_queEvent.pop(); 			
			continue;
		}

		m_queEvent.pop();

		::LeaveCriticalSection( &m_csEventQueue );

		SAFE_DELETE( spEvent.m_pSerBuff )
			::EnterCriticalSection( &m_csEventQueue );
	}

	::LeaveCriticalSection( &m_csEventQueue );
}
void CX2CollectHackInfo::EnQueue( unsigned short iEventID, KSerBuffer *pBuff, bool bSend, unsigned short usType, std::string strHackInfo, bool bExit )
{
	CSLock lock( m_csEventQueue );

	COLLECT_SERVER_PACKET csPacket;
	csPacket.m_iEventID = iEventID;
	csPacket.m_pSerBuff = pBuff;
	csPacket.m_bSend = bSend;
	csPacket.m_usType = usType;
	csPacket.m_strHackInfo = strHackInfo;
	csPacket.m_bExit = bExit;
	m_queEvent.push( csPacket );

}

bool CX2CollectHackInfo::ConnectCollectServer()
{
	bool bIsSERVICE = false;

#ifdef _SERVICE_
	bIsSERVICE = true;
#endif
	if( m_pCollectServer == NULL )
		m_pCollectServer = new CX2ServerProtocol( KTDXSendGameMessage, g_pKTDXApp->GetHWND(), XGM_RECEIVE_FROM_COLLECTSERVER, CX2ServerEvent::COLLECT_SERVER_EVENT_ID_STR, COLLECT_EVENTID_SENTINEL, bIsSERVICE );


	if( m_pCollectServer != NULL )
	{
		if( m_pCollectServer->IsCSConnected() == true )
			return true;

		bool bCSConnected = m_pCollectServer->ConnectedToCollectServer( L"14.45.79.56", 13050 );
		//bool bCSConnected = m_pCollectServer->ConnectedToCollectServer( L"192.168.66.28", 13050 );

		if( bCSConnected == false )
			return false;

		return true;
	}	

	return false;
}

void CX2CollectHackInfo::LoadHackImg( std::string &strFileName )
{
	FILE* hFile = NULL;

	g_pMain->MakeHackScreenShot( strFileName.c_str() );

	hFile = fopen(strFileName.c_str(), "rb");
	if(hFile == NULL)
	{
		m_bSendPacket = false;
		SAFE_DELETE_ARRAY( m_pHackImgBuf );
		m_lTotalSize = 0;
		m_iImageOffset = 0;
		return;
	}

	fseek(hFile, 0, SEEK_END);
	m_lTotalSize = ftell(hFile);	

	if ( 0 <= m_lTotalSize )
	{
		m_pHackImgBuf = new char[m_lTotalSize+1];
		memset( m_pHackImgBuf, 0, sizeof(char) * m_lTotalSize+1 );

		m_iImageOffset = 0;

		fseek (hFile,0,SEEK_SET);
		fread(m_pHackImgBuf, sizeof(char), m_lTotalSize, hFile);
	}

	fclose(hFile);
	DeleteFileA( strFileName.c_str() );
}

void CX2CollectHackInfo::SendCSImg( int iCount, std::string &strFileName, bool bExit )
{
	if( m_pHackImgBuf == NULL || m_lTotalSize <= 0 )
	{
#if defined( _SERVICE_ )
		ELSWORD_VIRTUALIZER_START
#endif
		m_iImageOffset = 0;
		m_lTotalSize = 0;
		m_bSendPacket = false;
		if( g_pKTDXApp != NULL && bExit == true )
		{
			g_pKTDXApp->SetFindHacking(true);
		}
#if defined( _SERVICE_ )
		ELSWORD_VIRTUALIZER_END
#endif
		return;
	}

#if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_START
#endif
	long lStartOffset = 0l;
	long lEndOffset = 0l;
	bool bLast = false;

	lStartOffset = iCount * CRASH_IMAGE_BUFF_MAX;
	lEndOffset = lStartOffset + CRASH_IMAGE_BUFF_MAX - 1;
	if( lEndOffset >= m_lTotalSize - 1 )
	{
		bLast = true;
		lEndOffset = m_lTotalSize - 1;
	}		

	KECL_CRASH_IMAGE_INFO_REQ kEvent;
	kEvent.m_strFileName = strFileName;
	kEvent.m_sNo = iCount;
	kEvent.m_bLast = bLast;
	kEvent.m_bExit = bExit;
	kEvent.m_vecImageBuff = vector<char>( &m_pHackImgBuf[lStartOffset], &m_pHackImgBuf[lEndOffset+1] );

	kEvent.m_dwBuffSize	= lEndOffset - lStartOffset + 1;

	bool bConnectedCollectServer = true;
	if( m_pCollectServer == NULL || m_pCollectServer->IsCSConnected() == false )
	{
		bConnectedCollectServer = ConnectCollectServer();
	}
	if( bConnectedCollectServer == false )
	{
		ErrorLogMsg( XEM_ERROR1, "Connect Fail (CollectServer)" );

		g_pMain->SendHackMail_HackUserNot("Not Connect Server");
				
		m_bSendPacket = false;
		m_iImageOffset = 0;
		m_lTotalSize = 0;
		SAFE_DELETE_ARRAY( m_pHackImgBuf );
		

		if( g_pKTDXApp != NULL && bExit == true )
		{
			g_pKTDXApp->SetFindHacking(true);
		}

		return;
	}

	m_pCollectServer->SendCSPacket( ECL_CRASH_IMAGE_INFO_REQ, kEvent );

	++m_iImageOffset;
	//CRASH_IMAGE_BUFF_MAX

// 이미지 파일들을 모두 보낸경우
	if( bLast == true )
	{
		m_iImageOffset = 0;
		m_lTotalSize = 0;
		SAFE_DELETE_ARRAY( m_pHackImgBuf );
						
		m_bSendPacket = false;
		
		if( g_pKTDXApp != NULL && bExit == true )
		{
			g_pKTDXApp->SetFindHacking(true);
		}
	}	

#if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_END
#endif
}

void CX2CollectHackInfo::SendCSEvent( unsigned short iEventId, unsigned short usType, std::string &strHackInfo, bool bExit )
{

#if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_START
#endif

	bool bConnectedCollectServer = true;
	if( m_pCollectServer == NULL || m_pCollectServer->IsCSConnected() == false )
	{
		bConnectedCollectServer = ConnectCollectServer();
		//g_pMain->EnQueue( iEventId, usType );
		//return;
	}

#if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_END
#endif

	if( bConnectedCollectServer == false )
	{
#if defined( _SERVICE_ )
		ELSWORD_VIRTUALIZER_START
#endif
		ErrorLogMsg( XEM_ERROR1, "Connect Fail (CollectServer)" );

		g_pMain->SendHackMail_HackUserNot("Not Connect Server");

		m_bSendPacket = false;
		m_iImageOffset = 0;
		m_lTotalSize = 0;

#if defined( _SERVICE_ )
		ELSWORD_VIRTUALIZER_END
#endif
		return;
	}

	if( iEventId == ECL_CRASH_INFO_REQ && m_bSendPacket == true )
	{
#if defined( _SERVICE_ )
		ELSWORD_VIRTUALIZER_START
#endif
		EnQueue( iEventId, NULL, true, usType, strHackInfo, bExit );
#if defined( _SERVICE_ )
		ELSWORD_VIRTUALIZER_END
#endif
		return;
	}

#if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_START
#endif
#ifdef MORE_INFO_ABOUT_HACK
	stringstream strstmInfo;
	// 시스템 정보
	g_pMain->AddSystemInfo(strstmInfo);

	// DLL 정보
	g_pMain->AddDLLInfo(strstmInfo);

	// 프로세스 & 스레드 정보
	g_pMain->AddProcessInfo(strstmInfo);
#endif // MORE_INFO_ABOUT_HACK

#ifdef CHECK_ALL_WINDOW
	HWND hWnd = ::GetTopWindow(NULL);
	while(hWnd)
	{
		string strWndText = " ";
		string strWndClass = " ";
		WCHAR wTextWindow[_MAX_PATH] = L"";
		WCHAR wTextClass[_MAX_PATH] = L"";
		int hr1 = GetWindowText(hWnd, wTextWindow, _MAX_PATH);
		int hr2 = GetClassName(hWnd,wTextClass,_MAX_PATH);

		THREAD_WND_INFO threadWndInfo;

		if( hr1 > 0 )
			ConvertWCHARToChar(strWndText, wTextWindow);

		if( hr2 > 0 )
			ConvertWCHARToChar(strWndClass, wTextClass);

		if( hr1 > 0 )
		{
			if( strWndText.compare( "MSCTFIME UI") != 0 && strWndText.compare( "Default IME") != 0 )
			{
				strstmInfo << strWndText.c_str() << "\t" << strWndClass.c_str() << "\n";
			}
		}

		hWnd = ::GetNextWindow(hWnd, GW_HWNDNEXT);
	}
#endif


#if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_END
#endif

	switch( iEventId )
	{
	case ECL_CRASH_INFO_REQ:
		{
#if defined( _SERVICE_ )
			ELSWORD_VIRTUALIZER_START
#endif
			string strId = "";
			if( g_pMain != NULL &&
				g_pMain->GetUserIdToFindHack().size() > 0 )
			{
				ConvertWCHARToChar( strId, g_pMain->GetUserIdToFindHack() );				
			}

			KECL_CRASH_INFO_REQ kEvent;
			kEvent.m_kCrashInfo.m_sType = usType;
			kEvent.m_kCrashInfo.m_strHackingInfo = strHackInfo;
			kEvent.m_kCrashInfo.m_strUserID = strId;
#ifdef MACHINE_ID
			kEvent.m_kCrashInfo.m_strMachineID = g_pMain->GetMachineId();
#endif
			kEvent.m_kCrashInfo.m_strText = strstmInfo.str();
			kEvent.m_kCrashInfo.m_iServerSN = 0;

#ifdef SERV_SERIAL_NUMBER_AVAILABILITY_CHECK	// 빌드 오류로 해외팀 추가
			memcpy(kEvent.m_kCrashInfo.m_charServerSN, g_pInstanceData->GetSN(), sizeof(unsigned char) * SERVER_SN);
#endif // SERV_SERIAL_NUMBER_AVAILABILITY_CHECK
			kEvent.m_bExit = bExit;

			m_pCollectServer->SendCSPacket( ECL_CRASH_INFO_REQ, kEvent );	

			m_bSendPacket = true;	
#if defined( _SERVICE_ )
			ELSWORD_VIRTUALIZER_END
#endif
		}
		break;
	default:
		break;
	}
}

void CX2CollectHackInfo::ReceiveCSEvent( COLLECT_SERVER_PACKET &spEvent )
{
	//MessageBox(NULL, CX2ServerEvent::COLLECT_SERVER_EVENT_ID_STR[spEvent.m_iEventID], L"CSEvent!", MB_OK);

	switch( spEvent.m_iEventID )
	{
	
	case ECL_CRASH_INFO_ACK:
		{
			KSerBuffer* pBuff = (KSerBuffer*)spEvent.m_pSerBuff;
			KECL_CRASH_INFO_ACK kEvent;
			DeSerialize( pBuff, &kEvent );

			if( g_pMain != NULL && g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
			{
				m_iImageOffset = 0;
				m_lTotalSize = 0;
				SAFE_DELETE_ARRAY(m_pHackImgBuf);

				if( m_iImageOffset == 0 )
					LoadHackImg( kEvent.m_strFileName );

				SendCSImg( m_iImageOffset, kEvent.m_strFileName, kEvent.m_bExit );
				//EnQueue( ECL_CRASH_IMAGE_INFO_REQ, NULL, true, 0,  kEvent.m_strFileName, kEvent.m_bExit );				
			}
		}
		break;
	case ECL_CRASH_IMAGE_INFO_ACK:
		{
			KSerBuffer* pBuff = (KSerBuffer*)spEvent.m_pSerBuff;
			KECL_CRASH_IMAGE_INFO_ACK kEvent;
			DeSerialize( pBuff, &kEvent );

			if( g_pMain != NULL && g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
			{
				//EnQueue( ECL_CRASH_IMAGE_INFO_REQ, NULL, true, 0,  kEvent.m_strFileName, kEvent.m_bExit );
				if( kEvent.m_dwRecvedTotalBuffSize < (DWORD)m_lTotalSize )
					SendCSImg( m_iImageOffset, kEvent.m_strFileName, kEvent.m_bExit );
			}			
		}
		break;
	default:
		break;
	}

	SAFE_DELETE( spEvent.m_pSerBuff );
}

void CX2CollectHackInfo::DoProcessEvent()
{	
	if( m_bRunThread == false )
		return;

	::EnterCriticalSection( &m_csEventQueue );  // 050630. 이벤트를 처리하는 동안 unlock 되도록 수정.

	// traverse all event
	COLLECT_SERVER_PACKET spEvent;

	//while( !g_pMain->m_queEvent.empty() )
	if( m_queEvent.empty() == false )	
	{
		// 050912. florist. 050911에 넷마블 테섭이 이곳에서 crash를 일으킴. 별도 예외처리.
		try 
		{ 
			spEvent = m_queEvent.front(); 
		} 
		catch( ... ) 
		{ 
			m_queEvent.pop(); 
			::LeaveCriticalSection( &m_csEventQueue );
			return;
		}

		m_queEvent.pop();

		::LeaveCriticalSection( &m_csEventQueue );


		// 2. call event handler
		if( spEvent.m_bSend == true )
		{
			SendCSEvent( spEvent.m_iEventID, spEvent.m_usType, spEvent.m_strHackInfo, spEvent.m_bExit );
		}
		else
		{
			ReceiveCSEvent( spEvent );		
		}

		::EnterCriticalSection( &m_csEventQueue );
	}

	::LeaveCriticalSection( &m_csEventQueue );
}

bool CX2CollectHackInfo::UIServerEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;

	switch( wParam )
	{
	case ECL_CONNECTION_LOST_NOT:
	case ECL_RESERVE_DESTROY:
		{
			SAFE_DELETE( pBuff );
			ClearQueue();
			m_bSendPacket = false;
			m_iImageOffset = 0;
			m_lTotalSize = 0;
			SAFE_DELETE_ARRAY( m_pHackImgBuf );

			if( m_pCollectServer != NULL )
				m_pCollectServer->DisconnectFromCollectServer();
			//SAFE_DELETE( m_pCollectServer );		
		}
		break;	
	case COLLECT_EVENTID_SENTINEL:
		{
			SAFE_DELETE( pBuff );
			//m_bSendPacket = false;
		}
		break;
	default:
		{
			EnQueue( (unsigned short)wParam, pBuff, false, 0, "", false );
		}
		break;
	}

	return true;
}

#endif //ADD_COLLECT_CLIENT_INFO