#include "stdafx.h"
#include ".\x2main.h"

#include <ShlObj.h>

#ifdef MACHINE_ID
#include "MachineIdLib.h"
#endif //MACHINE_ID

#if defined(BUILD_BY_HUDSON)
#define BUG_TRAP
#include "../hudsondef.h"
#endif

#include <boost/bind.hpp>
using boost::bind;

#ifdef IN_HOUSE_PLAY_LOG_TEST
	#include <fstream>
	#include "direct.h"
#endif IN_HOUSE_PLAY_LOG_TEST

#ifdef	CHECK_KOM_FILE_ON_LOAD
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "sha1.h"
#endif	CHECK_KOM_FILE_ON_LOAD

#ifdef DLL_MANAGER
#include "X2DLLManager.h"
#endif // DLL_MANAGER

#if defined(DEBUG) || defined(_DEBUG)
#pragma comment( lib, "X2ServerProtocolD.lib" )
#ifdef CLIENT_COUNTRY_KR
#pragma comment( lib, "NexonModuleD.lib" )
#endif // CLIENT_COUNTRY_KR
#else // DEBUG
#pragma comment( lib, "X2ServerProtocol.lib" )
#ifdef CLIENT_COUNTRY_KR
#pragma comment( lib, "NexonModule.lib" )
#endif // CLIENT_COUNTRY_KR
#endif // DEBUG

#ifndef NO_GAMEGUARD
	//#pragma comment( lib, "NPGameLib.lib" )

//[NOTE] �߱��� ���̺귯���� ������ϴ�. �������� ��Ȯ������ �����ϻ��� ������ �ٸ� ���̺귯���� ���� �������� �����ϱ�
#if defined CLIENT_COUNTRY_CN
#pragma comment( lib, "NPGameLib_98_MD_DE.lib" )
#elif defined SERV_COUNTRY_PH
#pragma comment( lib, "NPGameLib.lib" )
#else CLIENT_COUNTRY_CN
#pragma comment( lib, "NPGameLib_96_MD.lib" )
#endif CLIENT_COUNTRY_CN

#endif //NO_GAMEGUARD

#ifdef BUG_TRAP
    #include "BugTrap.h"

    // Link with one of BugTrap libraries.
    #ifdef _UNICODE
    #pragma comment(lib, "BugTrapU.lib")
    #else
    #pragma comment(lib, "BugTrap.lib")
    #endif
#endif

//{{ kimhc // 2010.3.3 // ���� ���⵵ ����
#ifdef	IGA_TEST
	#if defined(DEBUG) || defined(_DEBUG)
		#pragma comment( lib, "IGALibD.lib" )
	#else
		#pragma comment( lib, "IGALib.lib" )
	#endif // defined(DEBUG) || defined(_DEBUG)
#endif	IGA_TEST
//}} kimhc // 2010.3.3 // ���� ���⵵ ����







#ifdef THEMIDA_BY_TOOL_TEAM

#	ifdef APPLY_THEMIDA
		THEMIDA_BY_TOOL_TEAM �� APPLY_THEMIDA �� ���� ���Ǹ� �ȵ˴ϴ�. 	
#	endif APPLY_THEMIDA

#	ifdef VIRTUALIZER_CODE
		THEMIDA_BY_TOOL_TEAM �� VIRTUALIZER_CODE �� ���� ���Ǹ� �ȵ˴ϴ�. 	
#	endif VIRTUALIZER_CODE

#	ifdef DAMAGE_HISTORY
		DAMAGE_HISTORY ����� ������� �ʽ��ϴ�. ��ſ� NO_ALL_KILL_HACKING_TEST�� ����մϴ�. 
#	endif DAMAGE_HISTORY

#else THEMIDA_BY_TOOL_TEAM

	// �Ʒ��� �ڵ尡 �ƹ� ���۵� ���� �ʾƾ� �����Դϴ�.
	THEMIDA_VM_START				
	THEMIDA_CODEREPLACE_START	
	THEMIDA_ENCODE_START			

#endif THEMIDA_BY_TOOL_TEAM



#ifdef SERV_CLIENT_DIRECT_CONNECT_AUTH_PCBANG
#include "../X2Lib/OnlyGlobal/ID/NetCafe/PCbangModuleID.cpp"
#endif //CLIENT_DIRECT_CONNECT_AUTH_PCBANG


CX2Main*			g_pMain			= NULL;
CX2Data*			g_pData			= NULL;
CX2InstanceData*	g_pInstanceData = NULL;
CX2Game*			g_pX2Game		= NULL;
CX2Room*			g_pX2Room		= NULL;
CX2SquareGame*		g_pSquareGame	= NULL;

#ifdef ELSWORD_NEW_BEGINNING
CX2BeginningGame*	g_pBeginningGame = NULL;
#endif ELSWORD_NEW_BEGINNING
CX2TFieldGame*		g_pTFieldGame   = NULL;
#ifdef ADDED_RELATIONSHIP_SYSTEM
CX2WeddingGame*		g_pWeddingGame	= NULL;
#endif //ADDED_RELATIONSHIP_SYSTEM

CX2ChatBox*			g_pChatBox		= NULL;

FILE*				g_fpStream		= NULL;
FILE*				g_fpStreamError = NULL;

HINTERNET			g_hInternet		= NULL;


#ifdef _NEXON_KR_
wstring				g_pNexonPassport;	// ü�� ���̵�� �ؽ� �޽��� �����ϱ� ������ passport ���� ����
#endif
extern		CX2SMTPMail*			g_pX2SMTPMail;

#ifdef CHECK_THREAD_WND


BOOL CALLBACK EnumCheckThreadWndProc(HWND hwnd, LPARAM lParam)
{
	ELSWORD_VIRTUALIZER_START

	if( hwnd )
	{
		vector<THREAD_WND_INFO>* vecThreadWndInfo = (vector<THREAD_WND_INFO>*)lParam;

		WCHAR wTextWindow[_MAX_PATH] = L"";
		WCHAR wTextClass[_MAX_PATH] = L"";
		int hr1 = GetWindowText(hwnd, wTextWindow, _MAX_PATH);
		int hr2 = GetClassName(hwnd,wTextClass,_MAX_PATH);
		
		THREAD_WND_INFO threadWndInfo;

		if( hr1 > 0 )
			threadWndInfo.m_wstrName = wTextWindow;
		else
			threadWndInfo.m_wstrName = L"NameEmpty";

		if( hr2 > 0 )
			threadWndInfo.m_wstrClass = wTextClass;
		else
			threadWndInfo.m_wstrClass = L"ClassEmpty";
				
		if( hr1 > 0 )
		{
			if( threadWndInfo.m_wstrName.compare( L"MSCTFIME UI") != 0 &&
				threadWndInfo.m_wstrName.compare( L"Default IME") != 0 )
				//StrStrW( threadWndInfo.m_wstrClass.c_str(), L"HwndlessEditorFakeHwndWrapper") &&
				//threadWndInfo.m_wstrClass.compare L"tooltips_class32") != 0 && 
				//threadWndInfo.m_wstrClass.compare L"tooltips_class32") != 0 &&
				//threadWndInfo.m_wstrClass.compare L"WorkerW") != 0 )
			{
				vecThreadWndInfo->push_back(threadWndInfo);
			}
		}
	}

	ELSWORD_VIRTUALIZER_END

	return TRUE;	
}

vector<THREAD_WND_INFO> GetThreadWnd( DWORD threadId )
{
	ELSWORD_VIRTUALIZER_START

	vector<THREAD_WND_INFO>	vecThreadWndInfo;
	EnumThreadWindows(threadId, EnumCheckThreadWndProc, (LPARAM)&vecThreadWndInfo);

#ifdef _IN_HOUSE_
	FILE *fp = fopen("threadInfo.info", "w");
	for(UINT i=0; i<vecThreadWndInfo.size(); ++i)
	{
		wstring threadinfoWindow = vecThreadWndInfo[i].m_wstrName;
		wstring threadinfoClass = vecThreadWndInfo[i].m_wstrClass;
		fwprintf(fp, L"%s [%s]\n", threadinfoWindow.c_str(), threadinfoClass.c_str());
	}
	fclose(fp);
#endif

	ELSWORD_VIRTUALIZER_END

	return vecThreadWndInfo;
}
#endif // CHECK_THREAD_WND

#if defined( SERV_HACKING_TOOL_LIST ) 
BOOL CALLBACK EnumWindowsProc(HWND hwnd , LPARAM lp)
{


#if defined( _SERVICE_ ) 
	// checked 
	ELSWORD_VIRTUALIZER_START
#endif



	WCHAR textWindow[_MAX_PATH] = L"";
	WCHAR textClass[_MAX_PATH] = L"";

	if(GetParent(hwnd)) 
		return TRUE;

	//THEMIDA_VM_START


	int hr1 = GetWindowText(hwnd,textWindow,_MAX_PATH);
	int hr2 = GetClassName(hwnd,textClass,_MAX_PATH);

	MakeUpperCase(textWindow);
	MakeUpperCase(textClass);
	
	if( g_pMain != NULL && g_pMain->GetCheckWindowInfo() != NULL )
	{
		KHackingToolInfo windowInfo;
		if( hr1 > 0 )
			windowInfo.m_wstrProcessName = textWindow;
		else
			windowInfo.m_wstrProcessName = L"EMPTY";
		if( hr2 > 0 )
			windowInfo.m_wstrWindowClassName = textClass;
		else
			windowInfo.m_wstrWindowClassName = L"EMPTY";

		g_pMain->GetCheckWindowInfo()->PushWindowInfo(windowInfo);
	}

	//THEMIDA_VM_END


#if defined( _SERVICE_ ) 
	ELSWORD_VIRTUALIZER_END
#endif

	return TRUE;

	
}

BOOL Pesudo_IsDebuggerPresent()
{

	// TODO : ���⿡ VM_START �Ǵ� ENCODE_START ����ϴϱ� ������ ���� ������ ����
	// Error: There is at least a nested VM/CodeReplace macros. VM/CodeReplace macros cannot be nested.
	//THEMIDA_VM_START
	//THEMIDA_ENCODE_START

#if defined( _SERVICE_ ) 
	// checked 
	ELSWORD_VIRTUALIZER_START
#endif

	BOOL Retval = 0;
	__asm
	{
		push eax
		mov eax,dword ptr fs:[0x18]
		mov eax,dword ptr ds:[eax+0x30]
		movzx eax,byte ptr ds:[eax+0x2]
		mov Retval,eax
		pop eax
	}

#if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_END
#endif

	//THEMIDA_ENCODE_END
	//THEMIDA_VM_END

#ifdef _IN_HOUSE_
	return false;
#else
	return Retval;
#endif

}


// NOTE : ���� �� �Լ��� ����ϰ� ���� �ʴ�. 
int CheckDebugger()
{
#if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_START
#endif

	_asm
	{
		mov eax, fs:[30h]
		mov eax, [eax+68h]
		and eax, 0x70
		test eax, eax
	}

#if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_END
#endif
}




#endif

#ifdef BUG_TRAP
void MakeScreenShot(std::string strfilename)
{
	try
	{

		HBITMAP hBitmap;
		RECT rect;
		HDC hMemDC;
		HDC hScrDC;
		HBITMAP hOldBitmap;

		//DC�� ��´�.

		HWND pDesktop = GetDesktopWindow();

		// ��Ŀ���� ���� �������� ��ǥ�� �����Ѵ�.      
		GetClientRect( pDesktop, &rect );


		// ������ ȭ�� DC�� �޸� DC�� ����� �޸� ��Ʈ���� �����Ѵ�.
		hScrDC = GetDC( pDesktop ); CreateDC( L"DISPLAY", NULL, NULL, NULL );
		hMemDC = CreateCompatibleDC( hScrDC );
		hBitmap = CreateCompatibleBitmap( hScrDC, rect.right - rect.left, rect.bottom - rect.top );
		hOldBitmap = (HBITMAP)SelectObject( hMemDC, hBitmap );

		// ȭ���� �޸� ��Ʈ������ �����Ѵ�.
		BitBlt( hMemDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
			hScrDC, rect.left, rect.top, SRCCOPY );
		SelectObject( hMemDC, hOldBitmap );

		CxImage cimage( CXIMAGE_FORMAT_JPG );

		if( cimage.CreateFromHBITMAP( hBitmap ) )
		{
			cimage.SetJpegQuality( 75 );
			cimage.Save( strfilename.c_str(), CXIMAGE_FORMAT_JPG );
		}

		DeleteDC( hMemDC );
		DeleteDC( hScrDC );
		DeleteObject( hBitmap );
		DeleteObject( hOldBitmap );
	}
	catch (...)
	{
	}
}

void CALLBACK CrashPreCallBack(INT_PTR nErrHandlerParam)
{
    BT_SetTerminate();

	try
	{
#ifndef REMOVE_CRASH_SCREENSHOT
		MakeScreenShot("Crash_ScreenShot.jpg");
#endif REMOVE_CRASH_SCREENSHOT
		stringstream wstrstm;
		if( g_pMain != NULL )
		{	
#ifdef	ADD_CRASH_INFO
			g_pMain->AddMemoryInfo( wstrstm );
#endif	ADD_CRASH_INFO			
			g_pMain->AddSystemInfo( wstrstm );
			// DLL ����
			g_pMain->AddDLLInfo( wstrstm );
			// ���μ��� ����
			g_pMain->AddProcessInfo(wstrstm, false);
		}
		g_CKTDXLog.StateLogToFile( "ErrorLog.txt" );
		g_CKTDXLog.ErrorLogToFile( "ErrorLog.txt", wstrstm.str().c_str() );
		
		//FileEncrypt( "ErrorLog.txt" );
		//FileEncrypt( "State.log" );

		if ( g_pMain != NULL )
		{	
			g_pMain->SetQuitType( NetError::ERR_CLIENT_QUIT_02 );
			g_pMain->SendQuitMsgToServer();
		}

		ShowWindow(g_pKTDXApp->GetHWND(), SW_HIDE); 

		SiCX2DLLManager()->ReportEvent();
	}
	catch( ... )
	//catch(std::exception *e)
	{
	}
	
}

void CALLBACK CrashPostCallBack(INT_PTR nErrHandlerParam)
{
	DeleteFileA( "ErrorLog.txt");
	DeleteFileA( "State.txt");
#ifndef REMOVE_CRASH_SCREENSHOT
	DeleteFileA( "Crash_ScreenShot.jpg");
#endif REMOVE_CRASH_SCREENSHOT
    //HWND hDlg;   
    
}
#endif

// crashreport���� ���̴� �ݹ��Լ�
inline BOOL WINAPI CrashReportCallback(LPVOID lpvState)
{
	SIZE_T currMemSize=0, peakMemSize=0;
	GetUsingMemorySize( L"X2.exe", currMemSize, peakMemSize );
	WCHAR wszText[64] = L"";
	swprintf( wszText, sizeof(wszText)/sizeof(WCHAR)-1, L"mem: curr-%d, peak-%d", currMemSize, peakMemSize );
	StateLog( wszText );



#ifdef _SERVICE_
	g_CKTDXLog.ErrorLogToFile( "ErrorLog.txt" );
	g_CKTDXLog.StateLogToFile( "State.log" );
	FileEncrypt( "ErrorLog.txt" );
	FileEncrypt( "State.log" );
#endif

#ifndef REMOVE_CRASH_SCREENSHOT
	IDirect3DSurface9* pBackBuffer = 0;
	if( NULL != g_pKTDXApp &&
		NULL != g_pKTDXApp->GetDevice() )
	{
		g_pKTDXApp->GetDevice()->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
		if( NULL != pBackBuffer )
		{
			D3DXSaveSurfaceToFile( L"Crash_ScreenShot.jpg", D3DXIFF_JPG, pBackBuffer, NULL, NULL );
			pBackBuffer->Release();
		}
	}
	AddFile(lpvState, (LPCTSTR)"Crash_ScreenShot.jpg",		(LPCTSTR)"screen shot"  );
#endif REMOVE_CRASH_SCREENSHOT

#ifndef EFFECT_TOOL
	AddFile(lpvState, (LPCTSTR)"log.htm",		(LPCTSTR)"log file"  );
	AddFile(lpvState, (LPCTSTR)"ErrorLog.txt",	(LPCTSTR)"ErrorLog File" );
	AddFile(lpvState, (LPCTSTR)"DialogLog.txt",	(LPCTSTR)"DialogLog File" );
	AddFile(lpvState, (LPCTSTR)"P2PLog.txt",	(LPCTSTR)"P2PLog File" );
	AddFile(lpvState, (LPCTSTR)"State.log",		(LPCTSTR)"StateLog File" );
#endif //EFFECT_TOOL

	if ( g_pMain != NULL )
	{	
		g_pMain->SetQuitType( NetError::ERR_CLIENT_QUIT_02 );
		g_pMain->SendQuitMsgToServer();
	}

	return TRUE;
}



//#ifndef _SERVICE_
//	extern CX2SMTPMail* g_pX2SMTPMail; // x2.cpp�� ���ǵǾ�����
//	LRESULT CALLBACK SendDebugMailDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
//{
//	switch (message)
//	{
//	case WM_INITDIALOG:
//		{
//			g_pX2SMTPMail->SetReceiverEMail( "carl79@kogstudios.com" ); 
//
//			g_pX2SMTPMail->SetSenderName( "�系���ҵ�����" );
//			g_pX2SMTPMail->SetSubject( "[�系 ���� �÷��� ������]" );
//			g_pX2SMTPMail->SetDesc( "^^" );
//
//			// ���� ÷��
//			g_pX2SMTPMail->ClearFile();			
//			std::vector< std::string > vecFiles;
//
//			char* buffer = NULL;
//			if( (buffer = _getcwd( NULL, 255 )) == NULL )
//			{
//				// error
//				return TRUE;
//			}
//
//			//
//
//
//			//stringstream strmFolder;
//			//strmFolder << buffer << "\\GameGuard";
//			//free(buffer);
//
//			//// npgl.erl�� �ῡ ÷�ΰ� �ȵǴ� ��찡 �־ ������ �����ؼ� �������� �Ѵ�
//			//stringstream strmSrcNPGL;
//			//strmSrcNPGL << strmFolder.str() << "\\npgl.erl";
//			//stringstream strmDestNPGL;
//			//strmDestNPGL << strmFolder.str() << "\\npgl_copy.erl";
//			//CopyFileA( strmSrcNPGL.str().c_str(), strmDestNPGL.str().c_str(), FALSE );
//
//
//
//			//g_pX2SMTPMail->FindFilesByExt( vecFiles, strmFolder.str().c_str(), ".erl" ); 
//			////g_pX2SMTPMail->FindFilesByExt( vecFiles, "C:\\Nexon\\ElSword\\Data\\GameGuard", ".erl" );
//
//			for( UINT i=0; i<vecFiles.size(); i++ )
//			{
//				g_pX2SMTPMail->AddFile( vecFiles[i].c_str());
//			}
//
//			bool bResult = g_pX2SMTPMail->SendMail( hDlg );
//
//			return TRUE;
//		} break;
//
//	case WM_COMMAND:
//		{
//			return TRUE;
//		} break;
//	}
//	return FALSE;
//}
//
//#endif // _SERVICE_






#ifdef IN_HOUSE_PLAY_LOG_TEST


	LRESULT CALLBACK InHousePlayLogMailDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_INITDIALOG:
			{
				g_pX2SMTPMail->SendMail( hDlg );

				return TRUE;
			} break;

		case WM_COMMAND:
			{
				return TRUE;
			} break;
		}
		return FALSE;
	}

#endif IN_HOUSE_PLAY_LOG_TEST


CX2Main::CX2Main() :
m_bEnabledDeveloperCheatCode( true ), 
m_pData( NULL ),
m_pStringFilter( NULL ),
m_pGameEdit( NULL ),
m_pTextManager( NULL ),
m_pNoticeManager( NULL ),
m_pCheckSumManager( NULL ),
m_pLVUpEventMgr( NULL ),
m_pTutorSystem( NULL ),
#ifndef COUPON_SYSTEM // ���� UI ����
m_pCouponBox( NULL ),
#endif // COUPON_SYSTEM

m_pX2PVPEmblem( NULL ),
m_pPostBox( NULL ),
m_pSecurityPad( NULL ),
m_pInformerManager( NULL ),
m_pMemoryHolder( NULL )
#ifdef ON_CHAT_REMAIN_FOCUS
, m_bIsPopUpOnChat( false )
#endif ON_CHAT_REMAIN_FOCUS
#ifdef SYS_INFO
,m_pSystemInfo( NULL )
#endif //SYS_INFO
#ifdef	CHECK_KOM_FILE_ON_LOAD
, m_pSession( NULL )
#endif	CHECK_KOM_FILE_ON_LOAD
#ifdef SERVER_GROUP_UI_ADVANCED
, m_DefaultChannelServerInfo()
#endif SERVER_GROUP_UI_ADVANCED
#ifdef REFORM_UI_KEYPAD
, m_pKeyPad( NULL )
#endif
#ifdef CLIENT_USE_NATION_FLAG
, m_dwNationFlag( NF_NONE )
#endif CLIENT_USE_NATION_FLAG
#ifdef BANDICAM_RECORDING
, m_iOldCaptureFileSize(-1)
, m_fFileSizeCheckTime(2.f)
#endif //BANDICAM_RECORDING
#ifdef CHECK_PLAY_TIME_INFORMATION
, m_iGameHour( 0 )							//Ŭ���̾�Ʈ ���� �� ���� �ð� ��ȯ
, m_pDlgPlayTimeInformation( NULL )			//�÷��� �ð� �ȳ��� Dialog
, m_pDlgShutDownInformation( NULL )			//�÷��� �ð� �ȳ��� Dialog

, m_fPlayTimeInfoShowTime( 0.f )			//�ȳ�â ���� �ð�
, m_fShutDownInfoShowTime( 0.f )			//�ȳ�â ���� �ð�

, m_bProcessPlayTimeInformation( false )	//�ȳ�â Fade ��� ���� ����
, m_bProcessShutDownInformation( false )	//�ȳ�â Fade ��� ���� ����

, m_wstrShutDownString( L"" )				//�˴ٿ� �ȳ�â�� ����
#endif CHECK_PLAY_TIME_INFORMATION
, m_bSurveyUser( false )
//{{ �ֹ�ö [2013/1/4]  ���ӳ� ���� ��Ʈ���� �������Ϸ� ���
#ifdef PRINT_INGAMEINFO_TO_EXCEL
, m_bInGameInfoToExcel( false )
#endif PRINT_INGAMEINFO_TO_EXCEL
#ifdef FESTIVAL_UI
, m_SeqFestivalUI( INVALID_PARTICLE_HANDLE )
#endif //FESTIVAL_UI
#ifdef ADDED_EVENT_JUMPING_CHARACTER	// ������, �������� �̺�Ʈ ���� ĳ����
, m_bIsJumpingCharacter ( false )		// ���� ĳ���� ���� ĳ�����ΰ�?
#endif // ADDED_EVENT_JUMPING_CHARACTER	// ������, �������� �̺�Ʈ ���� ĳ����
#ifdef ALWAYS_INTERNAL_NPGE_PACKING
, ClientArg("")
#endif ALWAYS_INTERNAL_NPGE_PACKING
#ifdef SERV_VALIDITY_CHECK_CEHCKKOM_SCRIPT
, m_bIsValideCheckKomScript ( true )
#endif // SERV_VALIDITY_CHECK_CEHCKKOM_SCRIPT
{

#ifdef  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
	m_bUdpPacketOverlap = true;

    for( int i = 0; i < CX2Game::GT_NUMS; i++ )
    {
        m_aeUDPMode[ i ] = CKTDNUDP::FORCE_CONNECT_MODE_DEFAULT;
    }
#endif  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK

#ifdef IN_HOUSE_PLAY_LOG_TEST
	DeleteFileA( "InHouse.log" );
#endif IN_HOUSE_PLAY_LOG_TEST

#ifdef CHANGE_MAILSTRING
	m_strMailAddress = "";
	ConvertWCHARToChar( m_strMailAddress, GET_STRING( STR_ID_18009 ) );
	if ( NULL != g_pX2SMTPMail )
		g_pX2SMTPMail->SetSMTPAddress( m_strMailAddress.c_str() );
#endif

#ifdef	SERV_SERVER_TIME_GET
	m_bMappingResult = false;
	m_SumCount = 0;
	m_GetTimeCount = 0.0f;
	//��������� �ʾ ��Ŷ�� ���� ���ϴ� ��츦 ����Ͽ� ó�� ��ġ�ð��� �־��ش�.�ѹ��� �̺�Ʈ ó���� �ǰ� 
	m_TempTime = L"2013-04-26 00:00:00";
	m_bOneChange = false; //�ѹ��� ó���ϱ� ���ؼ�
#endif  SERV_SERVER_TIME_GET

#ifdef BUG_TRAP    
	wstring clientVersion;
	wstring szAppName;

#if defined(JOB_NAME)
	szAppName = JOB_NAME;
#endif

#ifdef EFFECT_TOOL
	szAppName = L"EFFECT_TOOL";
#endif // EFFECT_TOOL

#if defined(BUILD_ID)
	clientVersion = BUILD_ID;
#endif

	BT_InstallSehFilter();

	BT_SetAppName((LPCTSTR)szAppName.c_str());
	BT_SetAppVersion((LPCTSTR)clientVersion.c_str());

 	BT_SetFlags( BTF_DETAILEDMODE | BTF_ATTACHREPORT ); // BTF_DETAILEDMODE || BTF_SCREENCAPTURE	

	BT_SetDumpType( MiniDumpNormal ); //( MiniDumpWithDataSegs | MiniDumpWithIndirectlyReferencedMemory );
	BT_SetReportFormat(BTRF_XML);

	wchar_t szFilePath[1024] = _T("");
	wchar_t strStateLogBuffer[1024], strErrorLogBuffer[1024];
	GetCurrentDirectory( _MAX_PATH, szFilePath );

#ifdef EFFECT_TOOL
	BT_SetReportFilePath(szFilePath);
	BT_SetActivityType( BTA_SAVEREPORT );
#else
	BT_SetSupportServer( L"Crash-getter.kog.co.kr", 9999 );
	//BT_SetSupportServer( L"116.120.238.16", 9999 );
	BT_SetActivityType( BTA_SENDREPORT );
#endif //EFFECT_TOOL

	StringCchPrintf(strStateLogBuffer, 1024,  _T("%s\\State.log"), szFilePath);
	//wsprintf(strStateLogBuffer, _T("%s\\State.log"), szFilePath);
	BT_AddLogFile((LPCTSTR)strStateLogBuffer);

	StringCchPrintf(strErrorLogBuffer, 1024,  _T("%s\\ErrorLog.txt"), szFilePath);
	//wsprintf(strErrorLogBuffer, _T("%s\\ErrorLog.txt"), szFilePath);
	BT_AddLogFile((LPCTSTR)strErrorLogBuffer);    

#ifndef REMOVE_CRASH_SCREENSHOT
	StringCchPrintf(strErrorLogBuffer, 1024, _T("%s\\Crash_ScreenShot.jpg"), szFilePath);
	//wsprintf(strErrorLogBuffer, _T("%s\\Crash_ScreenShot.jpg"), szFilePath);
	BT_AddLogFile((LPCTSTR)strErrorLogBuffer);
#endif REMOVE_CRASH_SCREENSHOT

	BT_SetPreErrHandler(CrashPreCallBack, -1);
	BT_SetPostErrHandler(CrashPostCallBack, -1);
#endif
	

	RegisterHotKey( NULL, NULL, MOD_ALT, VK_F4 );


	g_pMain					= this;



#ifdef _SERVICE_
#ifdef _NEXON_KR_
	//OpenGlobalURL( L"http://Elsword.nexon.com/Elsword/etc/gameinstall.aspx?Section=chceck2" );
	OpenGlobalURL( L"http://elsword.nexon.com/wiselog/gameinstall.aspx?section=check2" );
#endif
#endif









#ifndef WITHOUT_MASS_FILE_TEST
	WCHAR wszKomFileName[256] = L"";

	const int KOM_FILE_COUNT = 145;
	for( int i=1; i<=KOM_FILE_COUNT; i++ )
	{
		if( i < 10 )
		{
			StringCchPrintfW( wszKomFileName, ARRAY_SIZE(wszKomFileName), L"data00%d.kom", i );
		}
		else if( i < 100 )
		{
			StringCchPrintfW( wszKomFileName, ARRAY_SIZE(wszKomFileName), L"data0%d.kom", i );
		}
		else
		{
			StringCchPrintfW( wszKomFileName, ARRAY_SIZE(wszKomFileName), L"data%d.kom", i );
		}

		m_KOMList.push_back( wszKomFileName );
	}
#endif WITHOUT_MASS_FILE_TEST
	
	

#ifdef	CONVERSION_VS
	m_DLLList.push_back( L"d3d9.dll" );
	m_DLLList.push_back( L"d3dx9_43.dll" );
 	m_DLLList.push_back( L"d3dcsx_43.dll" );
	m_DLLList.push_back( L"D3DCompiler_43.dll" );
	m_DLLList.push_back( L"xactengine3_7.dll" );
	m_DLLList.push_back( L"XAPOFX1_5.dll" );
	m_DLLList.push_back( L"XAudio2_7.dll" );
#if defined (_DEBUG) || defined(_IN_HOUSE_)
	m_DLLList.push_back( L"d3dx9d_43.dll");
#endif //defined (_DEBUG) || defined(_IN_HOUSE_)
#else	CONVERSION_VS
	m_DLLList.push_back( L"d3dx9_30.dll" );
#endif // CONVERSION_VS
	m_DLLList.push_back( L"dbghelp.dll" );
	m_DLLList.push_back( L"FindH.dll" );
	m_DLLList.push_back( L"KncCrashRpt.dll" );
	m_DLLList.push_back( L"KncFirewall.dll" );
	//m_DLLList.push_back( L"libhoard.dll" );
	m_DLLList.push_back( L"MailMIME.dll" );
	m_DLLList.push_back( L"MailSMTP.dll" );
#ifdef	CONVERSION_VS
	m_DLLList.push_back( L"msvcp100.dll" );
	m_DLLList.push_back( L"msvcr100.dll" );
#if defined (_DEBUG) || defined(_IN_HOUSE_)
	m_DLLList.push_back( L"msvcp100d.dll" );
	m_DLLList.push_back( L"msvcr100d.dll" );
#endif //defined (_DEBUG) || defined(_IN_HOUSE_)
#else	CONVERSION_VS
 	m_DLLList.push_back( L"msvcp71.dll" );
 	m_DLLList.push_back( L"msvcr71.dll" );
#endif // CONVERSION_VS
	//m_DLLList.push_back( L"nmcogame.dll" );		// �ؿ��� ���ʿ� dll ����
	m_DLLList.push_back( L"usp10.dll" );
	//m_DLLList.push_back( L"EndingFlash.DLL" );	// �ؿ��� ���ʿ� dll ����
	//m_DLLList.push_back( L"winhoard.dll" );

	//{{ 081001.hoons.�������� ���� ������ ĸ�ı�� ������ �ʿ���.
	m_DLLList.push_back( L"bdcap32.dll" );
	m_DLLList.push_back( L"bdcore32.dll" );
	m_DLLList.push_back( L"vcomp90.dll" );
	//}}

    m_DLLList.push_back( L"BugTrapU.dll" );
	m_DLLList.push_back( L"BugTrap.dll" );
	m_DLLList.push_back( L"fmodex.dll" );

	//{{ ����� : [2009/9/14] //	�ٽ��� dll �߰�
	m_DLLList.push_back( L"asc_com.dll" );
	m_DLLList.push_back( L"asc_dh.dll" );
	m_DLLList.push_back( L"asc_fse.dll" );
	m_DLLList.push_back( L"asc_intg.dll" );
	m_DLLList.push_back( L"asc_mmgr.dll" );
	m_DLLList.push_back( L"asc_unp.dll" );
	m_DLLList.push_back( L"fse_base.dll" );
	m_DLLList.push_back( L"fse_fact.dll" );
	m_DLLList.push_back( L"fse_pe.dll" );
	m_DLLList.push_back( L"gfs_base.dll" );
	m_DLLList.push_back( L"gfs_fact.dll" );
	m_DLLList.push_back( L"gfs_file.dll" );
	m_DLLList.push_back( L"gfs_mem.dll" );
	m_DLLList.push_back( L"gfs_os.dll" );
	m_DLLList.push_back( L"gfs_proc.dll" );
	m_DLLList.push_back( L"gfs_util.dll" );
	m_DLLList.push_back( L"ehsvc.dll" );
	m_DLLList.push_back( L"psapi.dll" );
	m_DLLList.push_back( L"v3pro32s.dll" );

	m_DLLList.push_back( L"AhnUpCtl.dll" );
	m_DLLList.push_back( L"AhnUpGS.dll" );
	m_DLLList.push_back( L"HSInst.dll" );
	m_DLLList.push_back( L"V3Hunt.dll" );
	m_DLLList.push_back( L"V3InetGS.dll" );		

		//}} ����� : [2009/9/14] //	�ٽ��� dll �߰�	

#ifdef SERV_EPAY_SYSTEM
	m_DLLList.push_back( L"msvcr80.dll" );
	m_DLLList.push_back( L"msvcp80.dll" );
	m_DLLList.push_back( L"msvcm80.dll" );
	m_DLLList.push_back( L"netsign.dll" );
	m_DLLList.push_back( L"safeinput4jh.dll" );
	m_DLLList.push_back( L"bjrcbedit.dll" );
	m_DLLList.push_back( L"pecin.dll" );
	m_DLLList.push_back( L"keyboardprotection.dll" );
	m_DLLList.push_back( L"processprotection.dll" );
	m_DLLList.push_back( L"pecin.dll" );
	m_DLLList.push_back( L"cmbedit.dll" );
	m_DLLList.push_back( L"libpemp12.dll" );
	m_DLLList.push_back( L"cgbedit.dll" );
	m_DLLList.push_back( L"inputcontrol.dll" );
	m_DLLList.push_back( L"msvcp60.dll" );
	m_DLLList.push_back( L"submitcontrol.dll" );
	m_DLLList.push_back( L"pecsp.dll" );
	m_DLLList.push_back( L"paicenc.dll" );
	m_DLLList.push_back( L"pecsp.dll" );
	m_DLLList.push_back( L"pecsp.dll" );
	m_DLLList.push_back( L"PECSP.dll" );
	m_DLLList.push_back( L"alidcp.dll" );
	m_DLLList.push_back( L"aliedit.dll" );
	m_DLLList.push_back( L"pta.dll" );
	m_DLLList.push_back( L"rsa.dll" );
#endif //SERV_EPAY_SYSTEM

	// �ӱԼ� �Ϻ� �߰� 09.11.26 ���� ���� dll ���� ����
#ifdef CLIENT_PURPLE_MODULE
	m_DLLList.push_back( L"HanAuthForClient.dll" );
	m_DLLList.push_back( L"HanGameInfoForClient.dll" );
	m_DLLList.push_back( L"HanReportForClient.dll" );
	m_DLLList.push_back( L"HGPSHELP.DLL" );
	m_DLLList.push_back( L"PurpleForClient.dll" );
	m_DLLList.push_back( L"HanReportForClientD.dll" );
	m_DLLList.push_back( L"PurpleForClientD.dll" );
	m_DLLList.push_back( L"HanGameInfoForClientD.dll" );
	m_DLLList.push_back( L"HanAuthForClientD.dll" );
#endif // CLIENT_PURPLE_MODULE

#ifdef HACKSHIELD_AUTO_UPDATE
	//{{ 2010/10/27 ��ȿ��	�ٽ��� �ڵ� ������Ʈ ��� �߰��Ǹ鼭 �߰��� dll ����
	m_DLLList.push_back( L"AspINet.dll" );
	m_DLLList.push_back( L"Bz32Ex.dll" );

	// hshield/update ���� �� 
	m_DLLList.push_back( L"ahni2.dll" );
	m_DLLList.push_back( L"v3bz32.dll" );
	//}}
#endif HACKSHIELD_AUTO_UPDATE

	//{{ ���� : XTRAP - DLL �߰�
#ifdef CLIENT_USE_XTRAP
	m_DLLList.push_back( L"psapi.dll" );
	m_DLLList.push_back( L"XTrapExt.dll" );
	m_DLLList.push_back( L"XTrapVa.dll" );
	m_DLLList.push_back( L"XPva03.dll" );
#endif CLIENT_USE_XTRAP

#ifdef MASSFILE_MAPPING_FUNCTION
	m_DLLList.push_back( L"mfc100u.dll" );
#if defined (_DEBUG) || defined(_IN_HOUSE_)
	m_DLLList.push_back( L"mfc100ud.dll" );
#endif //defined (_DEBUG) || defined(_IN_HOUSE_)
#endif

#ifdef CLIENT_COUNTRY_ID
	m_DLLList.push_back( L"NMClientAuthDLL.dll" );
	m_DLLList.push_back( L"NMRunParamDLL.dll" );
	m_DLLList.push_back( L"SSOWebDLL32.dll" );
#endif //CLIENT_COUNTRY_ID

#ifdef SERV_STEAM
	m_DLLList.push_back( L"steam.dll" );
	m_DLLList.push_back( L"steam_api.dll" );
	m_DLLList.push_back( L"steamclient.dll" );
	m_DLLList.push_back( L"sdkencryptedappticket.dll" );
#endif //SERV_STEAM

	//{{ ����� : [2009/10/12] //	���� ���� ���ҽ� �����ڵ�
#ifdef _SERVICE_
	DeleteFile( L"1.docx" );
	DeleteFile( L"unomap01.tga" );
	DeleteFile( L"unomap02.tga" );
	DeleteFile( L"unomap03.tga" );
	DeleteFile( L"nui_kenage_hair.tga" );
	DeleteFile( L"nui_kenage_cloth.tga" );
	DeleteFile( L"nui_kenage_body.tga" );
	DeleteFile( L"motion_unohound.x" );
	DeleteFile( L"motion_elemental_kenaz.x" );
#endif
	//}} ����� : [2009/10/12] //	���� ���� ���ҽ� �����ڵ�

#ifdef BANDICAM_CAPTURE
	DeleteFile( L"bdcap64.dll" );
#endif BANDICAM_CAPTURE

#if defined(X2OPTIMIZE_ITEM_TEMPLET_PREPROCESSING) || defined(_IN_HOUSE_)
	DeleteFile( L"item.kim" );
#endif	// defined(X2OPTIMIZE_ITEM_TEMPLET_PREPROCESSING) || define(_IN_HOUSE_)

#ifndef WITHOUT_MASS_FILE_TEST
#ifndef WORLD_TOOL
	DeleteOtherFile( L"KOM", m_KOMList );
	DeleteOtherFile( L"KOM_", m_KOMList );
	DeleteOtherFile( L"DLL", m_DLLList );
#endif //WORLD_TOOL
#endif WITHOUT_MASS_FILE_TEST

	m_QuitType				= NetError::ERR_CLIENT_QUIT_00;

#ifdef LOADING_ANIMATION_TEST
	m_pLoadingAnimation		= NULL;
#endif LOADING_ANIMATION_TEST

	m_pCrashReportState		= NULL;

#if defined _NEXON_KR_
	m_X2Publisher			= XP_NEXON_KOREA;
#elif defined CLIENT_COUNTRY_CN
	m_X2Publisher			= XP_GIANT_CN;
#elif defined CLIENT_COUNTRY_TW
	m_X2Publisher			= XP_GAMANIA_TW;	
#elif defined CLIENT_COUNTRY_HK
	m_X2Publisher			= XP_GAMANIA_HK;
#elif defined CLIENT_COUNTRY_JP
	m_X2Publisher			= XP_NHN_JP;	
#elif defined CLIENT_COUNTRY_ID
	m_X2Publisher			= XP_CJ_ID;	
#elif defined CLIENT_COUNTRY_TH
	m_X2Publisher			= XP_ASIASOFT_TH;		
#elif defined CLIENT_COUNTRY_EU
	m_X2Publisher			= XP_GAMEFORGE_EU;		
#elif defined CLIENT_COUNTRY_US
	m_X2Publisher			= XP_K3RCOMBO_US;
#elif defined CLIENT_COUNTRY_BR
	m_X2Publisher			= XP_LEVELUP_BR;
#elif defined CLIENT_COUNTRY_PH
	m_X2Publisher			= XP_GARENA_PH;
#else
	NO PUBLISHER NO COMPILE
#endif

	m_bManualLogin			= true;
	m_fServerDataTime		= 0.0f;
	m_bCenterServerDie		= false;

	m_iServerUserCount			= 0;
	m_iCurPartyCount			= 0;
	m_iPlayGamePartyCount		= 0;

	m_bAutoQuit			= false;
	m_fAutoQuitTime		= 0.0f;
	m_fAutoQuitCoolTime = (float)RandomNumber( 2, 5 );
	m_fTimeToUpdateProcessList = 0;

	m_pPartyUI = NULL;

#ifdef SERV_SKILL_NOTE
	m_fInfoBoxLifeTime = -1.f;
	m_pDlgInfoBox = NULL;
#endif

#ifdef SHOOTING_TEST
	m_iShootingLevel = 0;
#endif SHOOTING_TEST


#ifdef CHECK_RECEIVED_EGS_KNPROTECT_AUTH_CHECK_NOT
	#ifdef NO_GAMEGUARD
		m_bReceived_EGS_KNPROTECT_AUTH_CHECK_NOT = true;
	#else
		m_bReceived_EGS_KNPROTECT_AUTH_CHECK_NOT = false;
	#endif NO_GAMEGUARD
#endif CHECK_RECEIVED_EGS_KNPROTECT_AUTH_CHECK_NOT


	m_hInternet = NULL;
	// HTTP Open
	if ( NULL == ( m_hInternet = ::InternetOpen( L"HTTP_WRAPPER_KOG",
		INTERNET_OPEN_TYPE_PRECONFIG,
		NULL,
		NULL,
		0 ) ) )
	{
		ErrorLog( XEM_ERROR106 );
	}

	m_pCheckSumManager		= new CX2CheckSumManager();

#ifdef _SERVICE_
	dbg::logfile::SetLogLevel(-1);
	
	#ifndef _SERVICE_MANUAL_LOGIN_
		m_bManualLogin = false;
	#endif // _SERVICE_MANUAL_LOGIN_

#endif
#ifdef _NEXON_KR_
	if( m_bManualLogin == true )
		m_bNexonLogin		= false;
	else
		m_bNexonLogin		= true;

	m_bNexonVirtualLogin	= false;
	m_bNexonLoginMessenger	= false;
#endif _NEXON_KR_

#ifndef X2TOOL
	switch( GetPublisher() )
	{
#ifdef _NEXON_KR_
		case XP_NEXON_KOREA:
			{
				ConvertCharToWCHAR( g_pNexonPassport, __argv[1] );	// ü�� ���̵� ���ؼ�

				char commandString[2000] = {0,};
				char* tempArgv = __argv[1];
				wstring domainStr;
				if( tempArgv != NULL )
				{
					strncpy( commandString, tempArgv , 1999 );
					//strcpy( commandString, tempArgv );
					if( m_bManualLogin == false )
						ConvertCharToWCHAR(domainStr, commandString);
				}


				BOOL bNexonRetVal = TRUE;
#ifndef NEW_MESSENGER	// �ؽ� �޽��� ����� ���� �ʴ´�.
				StateLog( L"CNMManager::GetInstance().Init()" );
				CNMManager::GetInstance().Init();
#endif //#ifndef NEW_MESSENGER
				StateLog( L"CNMCOClientObject::GetInstance().SetLocale( kLocaleID_KR )" );
				bNexonRetVal = CNMCOClientObject::GetInstance().SetLocale( kLocaleID_KR );
#ifndef _USE_NEXON_MSG_INHOUSE	// �系���� �ؽ�����
#ifndef _SERVICE_
				//�޽��� �׽�Ʈ ���� �ڵ� ���߿� ���������� �ֳ�? 34�� �����ؾ��ϴϱ�
				CNMConnConfig _config;
				_config.bAutoPatch = false;
				wcscpy( _config.szLoginServerIp, L"218.145.45.33" );
				wcscpy( _config.szStatServerIp, L"218.145.45.33" );
				_config.uLoginServerPort = 46567;
				_config.uStatServerPort  = 45456;

				StateLog( L"CNMCOClientObject::GetInstance().SetConnConfig( _config )" );
				CNMCOClientObject::GetInstance().SetConnConfig( _config );
#endif
#endif
				StateLog( L"CNMCOClientObject::GetInstance().Initialize( NEXON_KOREA_ELSWORD_GAMECODE )" );
				bNexonRetVal = CNMCOClientObject::GetInstance().Initialize( NEXON_KOREA_ELSWORD_GAMECODE );

				if( m_bManualLogin == false )
				{
					StateLog( L"CNMCOClientObject::GetInstance().AttachAuth( domainStr.c_str()" );
					if( CNMCOClientObject::GetInstance().AttachAuth( domainStr.c_str() ) == kLoginAuth_OK )
					{
						StateLog( L"CNMCOClientObject::GetInstance().GetMyInfo()" );
						CNMCOClientObject::GetInstance().GetMyInfo();
					}
					else
					{
						g_pKTDXApp->NoticeQuitType( CKTDXApp::KQT_NEXON_AUTH_FAILED );
					}
				}

				StateLog( L"CNMCOClientObject::GetInstance().RegisterCallbackMessage( DXUTGetHWND(), XGM_NEXON_CALLBACK )" );
				bNexonRetVal = CNMCOClientObject::GetInstance().RegisterCallbackMessage( DXUTGetHWND(), XGM_NEXON_CALLBACK );


			}
			break;
#endif _NEXON_KR_
#ifdef CLIENT_COUNTRY_TWHK
		case XP_GAMANIA_TW:
		case XP_GAMANIA_HK:
			{
				// ���� ���� ��
				// __argv[1]; <- ����� ID
				// __argv[2]; <- ����� Pass ���´�
				

				char* tempArgv = __argv[1];
				char* tempArgv2 = __argv[2];
				if( tempArgv != NULL && tempArgv2 != NULL )
				{
#ifdef ARGUMENT_LOGIN
					m_bManualLogin = false;
#endif
					char IDString[2000] = {0,};
					char PassString[2000] = {0,};

					m_wstrID.clear();
					strncpy( IDString, tempArgv , 1999 );
					//strcpy( IDString, tempArgv );
					ConvertCharToWCHAR(m_wstrID, IDString, strlen(IDString));

					m_wstrPass.clear();
					strncpy( PassString, tempArgv2 , 1999 );
					//strcpy( PassString, tempArgv2 );
					ConvertCharToWCHAR(m_wstrPass, PassString, strlen(PassString));
				}
				else
				{
#ifdef ARGUMENT_LOGIN
#ifdef BEANFUN_UPGRADE_HOMEPAGE
#ifdef CLIENT_COUNTRY_TW
					ShellExecute( NULL, NULL, L"http://www.elsword.com.tw/event/bf_0602/index.html", NULL, L"", SW_SHOW );
#else //CLIENT_COUNTRY_HK
					ShellExecute( NULL, NULL, L"http://www.elsonline.com.hk/news/guide_130425.asp", NULL, L"", SW_SHOW );
#endif
					exit(0);
#endif BEANFUN_UPGRADE_HOMEPAGE
					m_bManualLogin = true;
#endif ARGUMENT_LOGIN
				}

			} break;
#endif CLIENT_COUNTRY_TWHK
#ifdef _NHN_JP_	// �ӱԼ� �Ϻ� �߰�
		case XP_NHN_JP:
			{		
				m_bManualLogin = false;

			} break;
#endif
#ifdef SERV_GLOBAL_AUTH
		case XP_CJ_ID:
		case XP_GIANT_CN:
		case XP_GAMEFORGE_EU:
		case XP_ASIASOFT_TH:
		case XP_LEVELUP_BR:
			{
				m_bManualLogin = true;
			}
			break;
#ifdef SERV_COUNTRY_PH
		case XP_GARENA_PH:
			{
#ifdef _SERVICE_
				m_bManualLogin = false;
#else //_SERVICE_
				m_bManualLogin = true;
#endif //_SERVICE_
			}
			break;
#endif //SERV_COUNTRY_PH
#ifdef SERV_STEAM
		case XP_K3RCOMBO_US:
			{
				
				if( CX2Steam::GetSteamUserIDAndPass(m_wstrID, m_wstrPass) )
					m_bManualLogin = false;
#ifdef SERV_CHANNELING_AERIA // �Ƹ��� ������ ������ ���⼭ �ϰ� ����
				else if( __argc == 3 )
				{
					m_bManualLogin = false;
					// ����ID -> ����ID�� ����
					char* tempArgv = __argv[1];
					wstring wstrID;
					wstrID.clear();
					char IDString[2000] = {0,};
					strncpy( IDString, tempArgv , 1999 );
					ConvertCharToWCHAR(m_wstrID, IDString, strlen(IDString));
					// �Ƹ��ƿ� ���� ��� -> ���� PW�� ����
					m_wstrPass = L"81621XZJ2380DE";
				}
#endif //SERV_CHANNELING_AERIA
				else
					m_bManualLogin = true;
			}
			break;
#else //SERV_STEAM
		case XP_K3RCOMBO_US:
			{
				m_bManualLogin = true;
			}
			break;
#endif //SERV_STEAM
#endif SERV_GLOBAL_AUTH
	
		default:
			break;
	}
#endif //X2TOOL

//{{ Iruha : 2026-08-24 // static auto login : overrides whatever the publisher switch decided
#ifdef STATIC_AUTO_LOGIN
	m_bManualLogin	= false;
	m_wstrID		= STATIC_AUTO_LOGIN_ID;
	m_wstrPass		= STATIC_AUTO_LOGIN_PW;
#endif STATIC_AUTO_LOGIN
//}}

	m_iChannelServerPort	= 9400;
	m_iPickedChannelServerIPIndex = -1;
	m_iDefaultChannelServerIPIndex = 0;


	m_ConnectedGameServerIP 	= L"";
	m_iConnectedGameServerPort	= 0;
	m_ServerUDPPort				= 0;
	m_GameP2PPort				= rand()%1000 + 8000;
	m_fServerTimeOut		= 5.0f;
#ifdef OPTIMIZED_P2P
    m_FrameBufferNum        = 10;
#else
	m_FrameBufferNum		= 7;
#endif // OPTIMIZED_P2P

	CX2PacketLog::SetEnable( false );

	m_pData					= new CX2Data();
	g_pData					= m_pData;
	g_pInstanceData			= new CX2InstanceData;

#if defined(ARGUMENT_LOGIN) || defined(SERV_STEAM) || defined(STATIC_AUTO_LOGIN)
	if(m_bManualLogin == false)
	{
		g_pInstanceData->SetUserID( m_wstrID );
		g_pInstanceData->SetUserPassword( m_wstrPass );
		
#ifdef SERV_CHANNELING_AERIA // �Ƹ��� ������ ������ ���⼭ �ϰ� ����
		if( __argc == 3 )
			// ���� ID�� ��� ����ؾ� �ϴ� ������ �Ӵϴ�.
			g_pInstanceData->SetUserSessionID(m_wstrID);
#endif //SERV_CHANNELING_AERIA
	}
#endif	

#ifdef CLIENT_PURPLE_MODULE_IN_HOUSE_AUTH
	char* tempArgv = __argv[1];
	char* tempArgv2 = __argv[2];
	if( tempArgv != NULL && tempArgv2 != NULL )
	{
		wstring wstrID;
		wstring wstrPass;
		wstrID.clear();
		wstrPass.clear();
		char IDString[2000] = {0,};
		char PassString[2000] = {0,};

		strncpy( IDString, tempArgv , 1999 );
		ConvertCharToWCHAR(wstrID, IDString, strlen(IDString));
		strncpy( PassString, tempArgv2 , 1999 );
		ConvertCharToWCHAR(wstrPass, PassString, strlen(PassString));
		
		g_pInstanceData->SetUserID( wstrID );
		g_pInstanceData->SetUserPassword( wstrPass );
		g_pInstanceData->SetInHouseAccount(true);
	}
#endif // CLIENT_PURPLE_MODULE_IN_HOUSE_AUTH

	m_pNowState				= NULL;
	m_NowStateID			= XS_INVALID;

	m_SquareMaxUserNum	= 0;

	g_pKTDXApp->GetDeviceManager()->AddHighTexName( L"DLG_" );
	g_pKTDXApp->GetDeviceManager()->AddHighTexName( L"HQ_" );


#ifndef WITHOUT_MASS_FILE_TEST
	for( int i = 0; i < (int)m_KOMList.size(); i++ )
	{
		wstring komFileName = L"./";
		komFileName += m_KOMList[i];
#ifdef _SERVICE_
		if( NULL == g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->AddMassFile( komFileName.c_str() ) )
		{
#ifdef _OPEN_TEST_
			MessageBox( g_pKTDXApp->GetHWND(), komFileName.c_str(), L"Error", MB_OK );
#endif	//_ OPEN_TEST_
			// kom ������ �о���µ� �����ߴ�. Ŭ���̾�Ʈ ��������.
			int *x = NULL;
			*x = 1;
		}
#else
		g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->AddMassFile( komFileName.c_str() );

#endif _SERVICE_
	}

#ifdef	X2OPTIMIZE_REMOVE_MASS_FILE_MANAGER_LOCK
	// �� ���Ŀ��� AddMassFile �Ұ�!
	g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LockMassFileMap();
#endif	X2OPTIMIZE_REMOVE_MASS_FILE_MANAGER_LOCK

#ifndef CHINA_PATH_SERVER_CONNECT
#if defined(CHECK_KOM_EU)
	//�� �Ʒ��� �Ľ� �ϰ� �մϴ�. 
#else
#ifdef	CHECK_KOM_FILE_ON_LOAD
	ProcessSession();
#endif	CHECK_KOM_FILE_ON_LOAD
#endif
#endif CHINA_PATH_SERVER_CONNECT

#endif WITHOUT_MASS_FILE_TEST

	//{{ �ڱ��� : [2010/05/05]	//	�ٹ����ڵ� �ʱ�ȭ
#ifdef DLL_MANAGER
	SiCX2DLLManager()->Init();
#endif DLL_MANAGER
	//}}

	m_bAutoStart = false;
	m_AutoLoginUnitIndex = 0;


	m_bIsPlayFieldDungeon	= false;
	m_bIsExitFieldDungeon   = false;
	m_bIsPlayingTutorial 	= false;
	m_bIsExitingTutorial 	= false;
	m_StateIDBeforeTutorial = XS_INVALID;
	m_iRandLoadingBG		= 0;

	m_iTrainingRewardED = 0;
	m_iTrainingRewardEXP = 0;
	m_bTrainingGameSuccess = false;

	m_bLastStageWasTrainingGame = false;
	m_bIsPlayingFreeTraining = false;

	m_bAutoStartNextTraining = false;
	m_iNowTrainingID = -1;

	m_pDLGStateChange = NULL;



	// dmlee: 2008-11-3, ���� ktdxapp�� �ִ� �ڵ带 ����� �Ű���ϴ�
	//{{ robobeg : 2008-10-28
// 	HRESULT hr = g_pKTDXApp->LoadKLuaManagerTemplate();
// 	ASSERT( SUCCEEDED( hr ) );
	//}} robobeg : 2008-10-28


	RegisterLuabind( g_pKTDXApp->GetLuaBinder() );
	LoadLuaEnum( g_pKTDXApp->GetLuaBinder() );

//#ifndef SERV_MASSFILE_MAPPING_FUNCTION
#ifdef MASSFILE_MAPPING_FUNCTION
	g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->MassFileMapping();
#endif MASSFILE_MAPPING_FUNCTION
//#endif SERV_MASSFILE_MAPPING_FUNCTION

	//{{ megagame / �ڱ��� / 2010.05.12 / �ý��� ����
#ifdef SYS_INFO
	m_pSystemInfo = new CX2SystemInfo();
#endif //SYS_INFO
	//}} megagame / �ڱ��� / 2010.05.12 / �ý��� ����

#ifdef USE_FREE_TYPE
	InstallFont( FT_DEFAULT_FONTNAME, "gulim.ttc" );
#endif //USE_FREE_TYPE

	//{{ 2013.02.27 ��ȿ��	�ؿ� �⺻ ���� �۾� (SERV_GLOBAL_BASE)
	std::wstring wstrBasicConfigFile = GetWstrBasicConfigFile();
	OpenScriptFile(wstrBasicConfigFile.c_str());
	//}}
#if defined(CHECK_KOM_EU)
//�׼ų� �÷��� �Ľ� �� �Ŀ� üũ�� �ε��ϰ� �մϴ�. 
#ifdef	CHECK_KOM_FILE_ON_LOAD
	ProcessSession();
#endif	CHECK_KOM_FILE_ON_LOAD
#endif


#ifdef LAUNCHER_COMMAND_ARGUMENT
	if(__argc >= 3)
	{
		if(__argv[1] != NULL)
		{
#ifdef SERVER_GROUP_UI_ADVANCED
			ConvertUtf8ToWCHAR( m_DefaultChannelServerInfo.m_kServerIP, __argv[1] );
#else
			m_vecChannelServerIP.clear();
			AddChannelServerIP_LUA( __argv[1] );
#endif SERVER_GROUP_UI_ADVANCED
		}

		if(__argv[2] != NULL)
		{
#ifdef SERVER_GROUP_UI_ADVANCED
			m_DefaultChannelServerInfo.m_usMasterPort = atoi(__argv[2]);
#else
			SetChannelServerPort( atoi(__argv[2]) );
#endif SERVER_GROUP_UI_ADVANCED
		}

		SetDefaultChannelServerIPIndex( 0 );
	}
#endif // LAUNCHER_COMMAND_ARGUMENT

#ifdef CHINA_PATH_SERVER_CONNECT
	ProcessSession();
#endif CHINA_PATH_SERVER_CONNECT


//#ifndef OPEN_TEST_1_NO_MESSENGER_CASHSHOP
#if !defined(WORLD_TOOL) && !defined(X2TOOL)
	g_pData->ResetMessenger();
#endif
//#endif OPEN_TEST_1_NO_MESSENGER_CASHSHOP


#ifdef SERV_COUNTRY_PH
#ifdef _SERVICE_
	g_pInstanceData->SetAuthToken((std::string) __argv[1]);
#endif //_SERVICE_
#endif //SERV_COUNTRY_PH


//{{����ȯ �����ð� �޾ƿ���
#ifdef	SERV_SERVER_TIME_GET
	if(Handler_ECH_GET_SERVER_TIME_REQ()==false)
	{
		 KTDGUIMsgBox( D3DXVECTOR2(250,300), L"�����ð� �ޱ� ����", g_pMain->GetNowState() );
	}
#endif  SERV_SERVER_TIME_GET



#ifndef BUG_TRAP
	string clientVersion = "";

#ifdef _SERVICE_
	#ifdef _OPEN_TEST_
		#ifdef NEW_VILLAGE
			clientVersion = "[CRASH_REPORT_OPEN_TEST_NEW_VILLAGE] ";
		#else
			clientVersion = "[CRASH_REPORT_OPEN_TEST] ";
		#endif NEW_VILLAGE
	#else
		clientVersion = "[CRASH_REPORT_SERVICE] ";
	#endif _OPEN_TEST_
#else
	clientVersion = "[CRASH_REPORT_TEST] ";
#endif _SERVICE_


	string clientMainVersion = "";
	ConvertWCHARToChar( clientMainVersion, GetClientVersion() );
	clientVersion += "_(";	
	clientVersion += clientMainVersion;
	string subClientVersion = "";
	ConvertWCHARToChar( subClientVersion, GetSubClientVersion_LUA() );
	if( subClientVersion.length() > 0 )
	{
		clientVersion += "_";
		clientVersion += subClientVersion;	
	}
	clientVersion += ")_";

#ifndef _DEBUG
	m_pCrashReportState = Install( CrashReportCallback, (LPCTSTR)"wonpok@kogstudios.com", (LPCTSTR)clientVersion.c_str() );
#endif _DEBUG

#ifdef EFFECT_TOOL
	m_pCrashReportState = Install( CrashReportCallback, (LPCTSTR)"hboh88@kog.co.kr", (LPCTSTR)clientVersion.c_str() );
#endif //EFFECT_TOOL
#endif BUG_TRAP

	m_pStringFilter			= new CX2StringFilter();
#ifndef WORLD_TOOL	// 090519 �¿� �ӽü����ڵ�
	m_pGameEdit				= new CX2GameEdit();
#endif WORLD_TOOL
	m_pTextManager			= new CX2TextManager();
	m_pNoticeManager		= new CX2NoticeManager( 150 );

#ifndef COUPON_SYSTEM // ���� UI ����
	m_pCouponBox			= new CX2CouponBox( NULL );
#endif // COUPON_SYSTEM

	m_pTutorSystem			= new CX2TutorSystem( NULL );

	m_pX2PVPEmblem			= new CX2PVPEmblem();
	m_pInformerManager		= new CX2InformerManager();
	m_pMemoryHolder			= new CX2MemoryHolder();



#ifndef WORLD_TOOL	// 090519 �¿� �ӽü����ڵ�
	
	m_pLVUpEventMgr			= new CX2LVUpEventMgr();

#ifdef POSTBOX
    m_pPostBox              = new CX2PostBox(NULL);
#endif // POSTBOX

#ifdef SERV_SECOND_SECURITY
	m_pSecurityPad			= new CX2SecurityPad();
#endif SERV_SECOND_SECURITY

#ifdef REFORM_UI_KEYPAD
	m_pKeyPad		= new CX2KeyPad();
#endif

#endif // WORLD_TOOL


	m_pFontForDebug = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetUKFont( XUF_DODUM_15_BOLD );



	//m_vecTimedMessagePopUp.reserve( 64 );


#ifdef LOADING_ANIMATION_TEST
	m_pLoadingAnimation		= new CX2LoadingAnimation;

#endif LOADING_ANIMATION_TEST
	

	
#ifdef _SERVICE_
	SetWindowText( g_pKTDXApp->GetHWND(), L"Elsword" );
	//wstringstream wstrmWindowTitle;
	//wstrmWindowTitle << L"Elsword - " << m_ClientVersion.c_str();
	//SetWindowText( g_pKTDXApp->GetHWND(), wstrmWindowTitle.str().c_str() );
#else
	
	
	wstringstream wstrmWindowTitle;
	wstrmWindowTitle << L"X2_NEW_VILLAGE_" << m_ClientVersion.c_str() << L"; compiled at " << __DATE__ << L", " << __TIME__;

#ifdef THEMIDA_BY_TOOL_TEAM
	UNPROTECTED_START
	
	wstrmWindowTitle << ", No Themida";

	UNPROTECTED_END
#endif THEMIDA_BY_TOOL_TEAM

	SetWindowText( g_pKTDXApp->GetHWND(), wstrmWindowTitle.str().c_str() );

#endif _SERVICE_
	

	m_BrowserWrapper.Init();

	g_pKTDXApp->GetDIManager()->SetEnable( true );

	FreeConsole();
	if ( g_fpStream != NULL )
		fclose( g_fpStream );
	if ( g_fpStreamError != NULL )
		fclose( g_fpStreamError );

	g_pKTDXApp->InsertStage( m_pData );	
    g_pKTDXApp->SendGameMessage( XGM_STATE_CHANGE, XS_START_UP, NULL, false );

	m_bHackingUser = false;

#ifdef SERV_CLIENT_PUBLIC_IP
	m_wstrPublicIp = L"";
#endif

#ifdef ADD_CHKOPT_ABNORMAL_FUNCTION_CALL_V2	
	m_fCheckStatusTime = 0.f;
#endif
#ifdef SERV_HACKING_TOOL_LIST
	m_fCheckHackToolTime = 0.f;
#endif
	
#if defined( SERV_HACKING_TOOL_LIST )
#	if defined( _SERVICE_ ) 
	// checked
	ELSWORD_VIRTUALIZER_START
#	endif

#	ifdef REFACTORING_BY_TOOL_TEAM
#	else REFACTORING_BY_TOOL_TEAM
	m_fCheckDebugeTimer = 0.f;
#	endif REFACTORING_BY_TOOL_TEAM


	m_threadCheckWindowInfo = NULL;

#	ifdef REFACTORING_BY_TOOL_TEAM

	#ifndef	CHECK_KOM_FILE_ON_LOAD
		CreateCheckWindowInfoThreadIfNotExist();
	#endif	CHECK_KOM_FILE_ON_LOAD

#	else REFACTORING_BY_TOOL_TEAM

	#if !defined(CHECK_KOM_FILE_ON_LOAD) && !defined(X2TOOL)
		m_threadCheckWindowInfo = new CheckWindowInfo();
		m_threadCheckWindowInfo->m_pKTDXApp			= g_pKTDXApp;
		m_threadCheckWindowInfo->m_pInstanceData	= g_pInstanceData;
		m_threadCheckWindowInfo->BeginThread();
	#endif	CHECK_KOM_FILE_ON_LOAD
	
#	endif REFACTORING_BY_TOOL_TEAM

#ifdef ADD_COLLECT_CLIENT_INFO
		m_pCollectHackInfo = NULL;
		m_iSendHackInfo = 0;
#endif

#	if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_END
#	endif
#endif // SERV_HACKING_TOOL_LIST

g_pKTDXApp->GetDeviceManager()->ResumeThread();
 #ifdef MACHINE_ID
 	CSystemInfo* pSystemInfo = new CSystemInfo();
 	pSystemInfo->Init();
 	const unsigned char* pMachineId = pSystemInfo->GetMachineId();
 	std::string enMachineId = base64_encode(pMachineId, 16);
	delete pSystemInfo;

#ifdef _IN_HOUSE_
 	FILE *fp = NULL;
	fp = fopen("MachineId.info", "w");
	if( fp != NULL )
	{
		fprintf(fp, "%s", enMachineId.c_str() );
		fclose(fp);			
	}
#endif
	
	ErrorLogMsg( XEM_ERROR1, enMachineId.c_str() );
 #endif

#ifdef LOG_LOAD_DLG_SCRIPT
	DeleteFileA("OpenScriptFile.log");
#endif //LOG_LOAD_DLG_SCRIPT

#if ( defined( _SERVICE_ ) && defined( _OPEN_TEST_ ) ) || defined( _IN_HOUSE_ )
#ifdef NEXON_QA_CHEAT_REQ
	m_bNoCoolTime = false;
#endif //NEXON_QA_CHEAT_REQ
#endif

#ifdef ACTIVE_KOG_GAME_PERFORMANCE_CHECK
	_InitKOGGamePerformanceCheck();
#endif//ACTIVE_KOG_GAME_PERFORMANCE_CHECK
}




CX2Main::~CX2Main(void)
{

#ifdef APPLY_THEMIDA
	CODEREPLACE_START
#endif

#ifdef DLL_MANAGER
	DllManagerReport();	
#endif

#ifdef APPLY_THEMIDA
	CODEREPLACE_END
#endif



#if defined( SERV_HACKING_TOOL_LIST )
	SAFE_DELETE(m_threadCheckWindowInfo);
#endif

#ifdef ADD_COLLECT_CLIENT_INFO
	SAFE_DELETE( m_pCollectHackInfo );
#endif

#ifdef LOADING_ANIMATION_TEST
	SAFE_DELETE( m_pLoadingAnimation );
#endif LOADING_ANIMATION_TEST



#ifndef WORLD_TOOL
#ifdef IN_HOUSE_PLAY_LOG_TEST
	//SendInHousePlayLogMail( "carl79@kog.co.kr" );
#endif IN_HOUSE_PLAY_LOG_TEST
#endif WORLD_TOOL

	//{{ 2011.9.16	����ȣ  �ݵ�ķ ������ ĸ�� ����
#ifdef BANDICAM_RECORDING
	m_BandiCaptureLibrary.Destroy();
#endif BANDICAM_RECORDING
	//}}

#ifdef CHECK_PLAY_TIME_INFORMATION
	DeletePlayTimeInformation();		//�÷��� �ð� �ȳ�â ����
	DeleteShutDownInformation();		//�˴ٿ� �ȳ�â ����
#endif CHECK_PLAY_TIME_INFORMATION



	m_BrowserWrapper.UnInit();
	SendQuitMsgToServer();

	for( int i = 0; i < (int)m_WaitServerPacketList.size(); i++ )
	{
		WaitServerPacket* pWaitServerPacket = m_WaitServerPacketList[i];
		SAFE_DELETE( pWaitServerPacket );
	}
	m_WaitServerPacketList.clear();
	m_mapWaitServerPacketList.clear();


	SAFE_DELETE( g_pInstanceData );


	SAFE_DELETE( m_pStringFilter );
	SAFE_DELETE( m_pGameEdit );
	SAFE_DELETE( m_pTextManager );
	SAFE_DELETE( m_pNoticeManager );
	SAFE_DELETE( g_pChatBox );
	SAFE_DELETE( m_pCheckSumManager );

	SAFE_DELETE( m_pTutorSystem );
	
#ifndef COUPON_SYSTEM // ���� UI ����
	SAFE_DELETE( m_pCouponBox );
#endif // COUPON_SYSTEM
	
	SAFE_DELETE( m_pX2PVPEmblem );
	SAFE_DELETE( m_pInformerManager );
	SAFE_DELETE( m_pMemoryHolder );
	SAFE_DELETE( m_pLVUpEventMgr );

	SAFE_DELETE( m_pPartyUI );

#ifdef POSTBOX
    SAFE_DELETE( m_pPostBox );
#endif

#ifdef SERV_SECOND_SECURITY
	SAFE_DELETE( m_pSecurityPad	);
#endif SERV_SECOND_SECURITY

#ifdef REFORM_UI_KEYPAD
	SAFE_DELETE( m_pKeyPad );
#endif

	g_pData->DeleteMessenger();

	switch( GetPublisher() )
	{
#ifdef _NEXON_KR_
		case XP_NEXON_KOREA:
			{
				BOOL bNexonRetVal = TRUE;

#ifndef OPEN_TEST_1_NO_MESSENGER_CASHSHOP
#ifndef NEW_MESSENGER
				if ( GetNexonVirtualLogin() == true )
				{
					StateLog( L"CNMCOClientObject::GetInstance().LogoutVirtual()" );
					CNMCOClientObject::GetInstance().LogoutVirtual();
					SetNexonVirtualLogin( false );
				}
#endif //#ifndef NEW_MESSENGER
#endif OPEN_TEST_1_NO_MESSENGER_CASHSHOP

				StateLog( L"CNMCOClientObject::GetInstance().GetMyStatus()" );
				if ( CNMCOClientObject::GetInstance().GetMyStatus() == kStatus_Online )
				{
#ifndef OPEN_TEST_1_NO_MESSENGER_CASHSHOP
#ifndef NEW_MESSENGER
					if ( g_pMain->GetNexonLoginMessenger() == true )
					{
						StateLog( L"CNMCOClientObject::GetInstance().LogoutMessenger()" );
						bNexonRetVal = CNMCOClientObject::GetInstance().LogoutMessenger();

						if ( bNexonRetVal == FALSE )
						{
							MessageBox( g_pKTDXApp->GetHWND(), L"LogoutMessenger Failed!", L"Error", MB_OK );
						}
					}	
#endif //#ifndef NEW_MESSENGER
#endif OPEN_TEST_1_NO_MESSENGER_CASHSHOP
	

					//
					//	������������ �α׾ƿ�
					//
					if ( m_bManualLogin == true )
					{
						StateLog( L"CNMCOClientObject::GetInstance().LogoutAuth()" );
						bNexonRetVal = CNMCOClientObject::GetInstance().LogoutAuth();
						if ( bNexonRetVal == FALSE )
						{
							MessageBox( g_pKTDXApp->GetHWND(), L"LogoutAuth Failed!", L"Error", MB_OK );
						}
					}
					else
					{
						StateLog( L"CNMCOClientObject::GetInstance().DetachAuth()" );
						bNexonRetVal = CNMCOClientObject::GetInstance().DetachAuth();
						if ( bNexonRetVal == FALSE )
						{
							MessageBox( g_pKTDXApp->GetHWND(), L"DetachAuth Failed!", L"Error", MB_OK );
						}
					}
			
				}



				//
				//	�޽��� �̺�Ʈ �ڵ鷯 ��� ����
				//
				StateLog( L"CNMCOClientObject::GetInstance().ResetCallbackMessage( g_pKTDXApp->GetHWND() )" );
				bNexonRetVal = CNMCOClientObject::GetInstance().ResetCallbackMessage( g_pKTDXApp->GetHWND() );

				if ( bNexonRetVal == FALSE )
				{
					MessageBox( g_pKTDXApp->GetHWND(), L"ResetCallbackMessage Failed!", L"Error", MB_OK );
				}

#ifndef NEW_MESSENGER
				//
				//	�޽��� �Ŵ��� ����
				//
				
	
				StateLog( L"CNMManager::GetInstance().Finalize()" );
				CNMManager::GetInstance().Finalize();

				// �޽��� ĳ�� ����Ÿ ���� .. Ư�� ������ �̻� ����� ���� ���� ��..?
				//���� ���� ��� .. C:\Documents and Settings\All Users\Application Data\Nexon\Common\NMDATA
				//���� ���� ��� .. C:\Documents and Settings\All Users\Application Data\Nexon\Common\nmservice.ini

				if ( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_SPECIAL_USER )
				{
					DeleteNMCachData();
				}
#endif //#ifndef NEW_MESSENGER

			}
			break;
#endif _NEXON_KR_
#ifdef CLIENT_COUNTRY_TWHK
		case XP_GAMANIA_TW:
		case XP_GAMANIA_HK:
			{
				// ���� ���� ��

			} break;
#endif CLIENT_COUNTRY_TWHK
		default:
			break;
	}

	if( m_hInternet != NULL )
		InternetCloseHandle( m_hInternet );


	for ( int i = 0; i < (int)m_UseFontNameList.size(); i++ )
	{
		wstring fontName = m_UseFontNameList[i];
		//RemoveFontResourceEx( fontName.c_str(), FR_PRIVATE, NULL );
		if( RemoveFontResource( fontName.c_str() ) == false )
		{
			int i = 0;
		}
	}
/*
	InvalidateRect( g_pKTDXApp->GetHWND(), NULL, TRUE );
	SendMessage( HWND_BROADCAST, WM_FONTCHANGE, 0, 0 );
*/
	m_UseFontNameList.clear();

#ifdef _SERVICE_
#ifdef NX_ENDING_FLASH

#ifdef SERV_TOONILAND_CHANNELING
	if( NULL != g_pData && NULL != g_pData->GetMyUser() )
	{
		switch ( g_pData->GetMyUser()->GetUserData()->m_uChannelCode )
		{
		case 0:		// NEXON User
			{
				CNMCOClientObject::GetInstance().OpenGameSummary();
				//WinExec( "EndingFlash.DLL", SW_NORMAL );
			} break;
		case 3:		// TooniLand user
		default:
			break;
		}
	}
#else
	CNMCOClientObject::GetInstance().OpenGameSummary();
	//WinExec( "EndingFlash.DLL", SW_NORMAL );
#endif

#endif //NX_ENDING_FLASH
#endif //_SERVICE_

	//{{ �ڱ��� : [2010/03/24]	//	�ٹ����ڵ� ����
#ifdef DLL_MANAGER
	SiCX2DLLManager()->ReleaseInstance();
#endif
	//}}

//{{ megagame / �ڱ��� / 2010.05.12 / �ý��� ����
#ifdef SYS_INFO
	SAFE_DELETE( m_pSystemInfo );
#endif //SYS_INFO
//}} megagame / �ڱ��� / 2010.05.12 / �ý��� ����

#ifdef ACTIVE_KOG_GAME_PERFORMANCE_CHECK
	KOGGamePerformanceCheck_CallbackFuncTable::DeleteInstance();
	KOGGamePerformanceCheck::DeleteInstance();
#endif//ACTIVE_KOG_GAME_PERFORMANCE_CHECK

#ifdef NEW_EMBLEM_MANAGER
	CX2EmblemManager::DestroyInstance();
#endif // NEW_EMBLEM_MANAGER

	g_pMain = NULL;
	
#ifdef CJ_ID_WEB_BILLING
	UnInitSSOWebBrowser();
#endif //CJ_ID_WEB_BILLING
}


//{{ 2013.02.27 ��ȿ��	�ؿ� �⺻ ���� �۾� (SERV_GLOBAL_BASE)
std::wstring CX2Main::GetWstrBasicConfigFile()
{
	std::wstring wstrBasicConfigFile = L"Config_INTERNAL.lua";

#ifndef CLIENT_INT_INTERNAL // ���� �ҽ� �����Ͽ����� �Ѵ� Define
#ifdef _SERVICE_
#ifdef _OPEN_TEST_
	//{{ _OPEN_TEST_
#ifdef CLIENT_COUNTRY_TW
	wstrBasicConfigFile =  L"Config_TW_TEST.lua";
#elif CLIENT_COUNTRY_HK
	wstrBasicConfigFile =  L"Config_HK_TEST.lua";
#elif CLIENT_COUNTRY_JP
	wstrBasicConfigFile =  L"Config_JP_OPENTEST.lua";
#elif CLIENT_COUNTRY_CN
	wstrBasicConfigFile =  L"Config_CN_TEST.lua";
#elif CLIENT_COUNTRY_EU
	wstrBasicConfigFile =  L"Config_EU_TEST.lua";
#elif CLIENT_COUNTRY_TH
	wstrBasicConfigFile =  L"Config_TH_TEST.lua";
#elif CLIENT_COUNTRY_US
	wstrBasicConfigFile =  L"Config_US_TEST.lua";
#elif CLIENT_COUNTRY_ID
	wstrBasicConfigFile =  L"Config_ID_OPEN_TEST.lua";
#elif CLIENT_COUNTRY_BR
	wstrBasicConfigFile =  L"Config_BR_OPEN_TEST.lua";
#elif CLIENT_COUNTRY_PH
	wstrBasicConfigFile =  L"Config_PH_OPEN_TEST.lua";
#else
	wstrBasicConfigFile =  L"Config_TEST.lua";
#endif
	//}} _OPEN_TEST_
#else _OPEN_TEST_
#ifdef CLIENT_COUNTRY_TW
	wstrBasicConfigFile =  L"Config_TW_SERVICE.lua";
#elif CLIENT_COUNTRY_HK
	wstrBasicConfigFile =  L"Config_HK_SERVICE.lua";
#elif CLIENT_COUNTRY_JP
	wstrBasicConfigFile =  L"Config_JP_OPENTEST.lua";
#elif CLIENT_COUNTRY_CN
	wstrBasicConfigFile =  L"Config_CN_SERVICE.lua";
#elif CLIENT_COUNTRY_EU
	wstrBasicConfigFile =  L"Config_EU.lua";	
#elif CLIENT_COUNTRY_US
	wstrBasicConfigFile =  L"Config_US_SERVICE.lua";
#elif CLIENT_COUNTRY_ID
	wstrBasicConfigFile =  L"Config_ID_SERVICE.lua";
#elif CLIENT_COUNTRY_TH
	wstrBasicConfigFile =  L"Config_TH_SERVICE.lua";
#elif CLIENT_COUNTRY_BR
	wstrBasicConfigFile =  L"Config_BR_SERVICE.lua";
#elif CLIENT_COUNTRY_PH
	wstrBasicConfigFile =  L"Config_PH_SERVICE.lua";
#else
	wstrBasicConfigFile =  L"Config_SERVICE.lua";
#endif
#endif _OPEN_TEST_
#else _SERVICE_	// INTERNAL
#ifdef CLIENT_COUNTRY_TW
	wstrBasicConfigFile =  L"Config_TW_INTERNAL.lua";
#elif CLIENT_COUNTRY_HK
	wstrBasicConfigFile =  L"Config_HK_INTERNAL.lua";
#elif CLIENT_COUNTRY_JP
	wstrBasicConfigFile =  L"Config_JP_TEST.lua";
#elif CLIENT_COUNTRY_CN
	wstrBasicConfigFile =  L"Config_CN_INTERNAL.lua";
#elif CLIENT_COUNTRY_EU
	wstrBasicConfigFile =  L"Config_EU_INTERNAL.lua";	
#elif CLIENT_COUNTRY_US
	wstrBasicConfigFile =  L"Config_US_INTERNAL.lua";
#elif CLIENT_COUNTRY_ID
	wstrBasicConfigFile =  L"Config_ID_INTERNAL.lua";
#elif CLIENT_COUNTRY_TH
	wstrBasicConfigFile =  L"Config_TH_INTERNAL.lua";
#elif CLIENT_COUNTRY_BR
	wstrBasicConfigFile =  L"Config_BR_INTERNAL.lua";
#elif CLIENT_COUNTRY_PH
	wstrBasicConfigFile =  L"Config_PH_INTERNAL.lua";
#else
	wstrBasicConfigFile =  L"Config_INTERNAL.lua";
#endif
#endif _SERVICE_
#endif //CLIENT_INT_INTERNAL	

	return wstrBasicConfigFile;
}

HRESULT CX2Main::OnFrameMove( double fTime, float fElapsedTime )
{
#ifndef X2TOOL

#if defined( SERV_HACKING_TOOL_LIST )
#if defined( _SERVICE_ )
	// checked 
	ELSWORD_VIRTUALIZER_START
#endif
#ifdef  SERV_SERVER_TIME_GET
	if(m_bOneChange == false)
	{
		StateChangeTimeSleep( fElapsedTime );
	}
#endif  SERV_SERVER_TIME_GET

#ifdef ADD_CHKOPT_ABNORMAL_FUNCTION_CALL_V2	
		m_fCheckStatusTime += fElapsedTime;
		if( m_fCheckStatusTime > 60.f * 5.f )
		{
#ifdef ADD_COLLECT_CLIENT_INFO

#ifdef _IN_HOUSE_
			//	�޽��� ȣ��
			MessageBoxW( g_pKTDXApp->GetHWND(), L"�ٽǵ尡 ������ ���� �ʽ��ϴ�.", GET_STRING( STR_ID_4427 ), MB_OK );
#endif

			g_pMain->SendHackInfo3( ANTIHACKING_ID::ANTIHACKING_GAME_13, "", false, true );
#else //ADD_COLLECT_CLIENT_INFO
			static bool bSend = false;

			if( bSend == false )
			{
#ifdef _IN_HOUSE_
				//	�޽��� ȣ��
				MessageBoxW( g_pKTDXApp->GetHWND(), L"�ٽǵ尡 ������ ���� �ʽ��ϴ�.", GET_STRING( STR_ID_4427 ), MB_OK );
#endif

				// �ٸ��� ����
				g_pMain->SendHackMail_DamageHistory( "-- HS Status --" );
				bSend = true;
			}
#endif //ADD_COLLECT_CLIENT_INFO
		}
#endif //ADD_CHKOPT_ABNORMAL_FUNCTION_CALL_V2

#if defined(SERV_HACKING_TOOL_LIST)
		switch( GetNowStateID() )
		{
		case XS_DUNGEON_GAME:
		case XS_VILLAGE_MAP:
		case XS_SQUARE_GAME:
#ifdef ADDED_RELATIONSHIP_SYSTEM
		case XS_WEDDING_GAME:
#endif // ADDED_RELATIONSHIP_SYSTEM
			{
				if( (g_pData != NULL && ( g_pData->GetCashShop() == NULL || (g_pData->GetCashShop() != NULL && g_pData->GetCashShop()->GetOpen() == false) ) ) )
				{
					m_fCheckHackToolTime += fElapsedTime;
					if(	m_fCheckHackToolTime > 60.f )
					{
#ifdef ADD_COLLECT_CLIENT_INFO // ���� ������ �ؿ��� �߰�
						g_pMain->SendHackInfo4( ANTIHACKING_ID::ANTIHACKING_GAME_49, "", false, true );
#endif // ADD_COLLECT_CLIENT_INFO
						m_fCheckHackToolTime = 0.f;
					}
				}
				else
				{
					m_fCheckHackToolTime = 0.f;
				}
			} break;
		default:
			break;
		}
#endif

#if defined(_SERVICE_)

#	ifdef REFACTORING_BY_TOOL_TEAM
	
	m_DebuggerChecker.OnFrameMove( fElapsedTime );

#	else REFACTORING_BY_TOOL_TEAM
	m_fCheckDebugeTimer += fElapsedTime;
	if( m_fCheckDebugeTimer > 20.f )
	{
		if( Pesudo_IsDebuggerPresent() ) //|| CheckDebugger() )
		{
			if( g_pData != NULL && g_pData->GetServerProtocol() != NULL &&
				g_pKTDXApp->GetFindHacking() == false )
			{
				g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );

				if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL )
					g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
			}

#ifdef ADD_COLLECT_CLIENT_INFO			
			g_pMain->SendHackInfo5( ANTIHACKING_ID::ANTIHACKING_GAME_14, "", true, true );
#ifdef BROKEN_CODE
			BrokenCode();
#endif //BROKEN_CODE
#else //ADD_COLLECT_CLIENT_INFO
			g_pMain->SendHackMail_DamageHistory( ANTI_HACK_STRING_AntiHacking_IsDebuggerPresent );

#ifdef	BROKEN_CODE
			BrokenCode();
#endif	BROKEN_CODE
			

			g_pKTDXApp->SetFindHacking( true );
#endif //ADD_COLLECT_CLIENT_INFO
		}

		m_fCheckDebugeTimer = 0.f;
	}
#	endif REFACTORING_BY_TOOL_TEAM

#endif // _SERVICE_




#ifdef REFACTORING_BY_TOOL_TEAM

	CreateCheckWindowInfoThreadIfNotExist(); 
	
#else REFACTORING_BY_TOOL_TEAM
	//if( IsSamef(m_fCheckDebugeTimer) == true )
	{
		// 20�ʿ� �ѹ��� ���͸������� ��ȿ�� �˻�
		if( m_threadCheckWindowInfo == NULL )
		{
			m_threadCheckWindowInfo = new CheckWindowInfo();
			m_threadCheckWindowInfo->m_pKTDXApp			= g_pKTDXApp;
			m_threadCheckWindowInfo->m_pInstanceData	= g_pInstanceData;
#ifdef	CHECK_KOM_FILE_ON_LOAD
			m_threadCheckWindowInfo->m_pMain			= this;
#endif	CHECK_KOM_FILE_ON_LOAD
		}

		bool bIsvalidCheckThread = true;	
		if( m_threadCheckWindowInfo != NULL )
		{ 
#ifdef CHECKFILTER_IN_MAIN
 			if( m_threadCheckWindowInfo->GetThreadHandle() != NULL )
 			{
 				DWORD dwExitCode = 0;
 				::GetExitCodeThread( m_threadCheckWindowInfo->GetThreadHandle(), &dwExitCode );
 				if( dwExitCode != STILL_ACTIVE )
 					bIsvalidCheckThread = false;
 			}
#endif
			if( m_threadCheckWindowInfo->GetThreadHandle() == NULL || m_threadCheckWindowInfo->GetIsLoop() == false )
			{
				bIsvalidCheckThread = false;
			}		
		}

		if( m_threadCheckWindowInfo != NULL && bIsvalidCheckThread == false )
		{
			m_threadCheckWindowInfo->BeginThread();

#ifndef	CHECK_KOM_FILE_ON_LOAD
			if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
				g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
			{
				g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );		
			}
#ifdef ADD_COLLECT_CLIENT_INFO			
			g_pMain->SendHackInfo1( ANTIHACKING_ID::ANTIHACKING_GAME_15, "", false, false );
#else //ADD_COLLECT_CLIENT_INFO
			g_pMain->SendHackMail_DamageHistory( ANTI_HACK_STRING_UserID_AntiHacking_CheckWindowInfo_Fail );
#endif //ADD_COLLECT_CLIENT_INFO

#endif	CHECK_KOM_FILE_ON_LOAD
		}
	}
#endif REFACTORING_BY_TOOL_TEAM





#if defined( _SERVICE_ ) 
	ELSWORD_VIRTUALIZER_END
#endif
#endif // SERV_HACKING_TOOL_LIST

#endif //X2TOOL


	for( int i = 0; i < (int)m_WaitServerPacketList.size(); i++ )
	{
		WaitServerPacket* pWaitServerPacket = m_WaitServerPacketList[i];
		pWaitServerPacket->fReamainTimeOut -= fElapsedTime;
		if( pWaitServerPacket->fReamainTimeOut <= 0.0f )
		{
#ifndef _SERVICE_
			std::map<int, int>::iterator it;
			it = m_mapWaitServerPacketList.find( (int) pWaitServerPacket->waitACK );
			if( it != m_mapWaitServerPacketList.end() )
			{
				it->second -= 1;

				if( it->second <= 0 )
				{
					m_mapWaitServerPacketList.erase( it );
				}
			}
#endif // _SERVICE_
			
			g_pKTDXApp->SendGameMessage( XGM_SERVER_PACKET_TIMEOUT, pWaitServerPacket->waitACK, NULL, false );
			SAFE_DELETE( pWaitServerPacket );
			m_WaitServerPacketList.erase( m_WaitServerPacketList.begin() + i );
			i--;
		}
	}


	if( g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_OPERATOR )
	{
		m_fServerDataTime += fElapsedTime;
		if( m_fServerDataTime > 60.0f )
		{
			m_fServerDataTime = 0.0f;

			if( g_pData->GetServerProtocol() != NULL )
			{
				g_pData->GetServerProtocol()->SendID( EGS_ADMIN_SERVER_STATUS_REQ );
			}
		}
	}




	if ( NULL != m_pNoticeManager )
		m_pNoticeManager->OnFrameMove( fTime, fElapsedTime );


#ifdef NEW_CHECK_OPENED_DEVICE

	if( g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_H) == TRUE )
	{	
		g_pKTDXApp->GetDeviceManager()->SaveOpenedDeviceListToFile();
	}

#endif NEW_CHECK_OPENED_DEVICE


	if( g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_SYSRQ) == TRUE )
	{
		if( CX2Main::XS_PVP_GAME == g_pMain->GetNowStateID() && 
			KPVPChannelInfo::PCC_TOURNAMENT == g_pMain->GetPVPChannelClass( g_pMain->GetConnectedChannelID() ) )
		{
			if ( g_pChatBox != NULL )
				g_pChatBox->AddChatLog( GET_STRING( STR_ID_346 ), KEGS_CHAT_REQ::CPT_SYSTEM, D3DXCOLOR(1,1,0,1), L"#CFFFF00" );
		}
		else
		{
#ifdef ACTIVE_KOG_GAME_PERFORMANCE_CHECK
			KOGGamePerformanceCheck::GetInstance()->Pause();
#endif//ACTIVE_KOG_GAME_PERFORMANCE_CHECK

			bool bSuccessCapture = false;
			//{{ 2011.9.16	����ȣ  �ݵ�ķ ��ũ���� ĸ�� ����
#ifdef BANDICAM_CAPTURE
			bSuccessCapture = g_pMain->Bandi_ScreenCapture();
#else BANDICAM_CAPTURE
			bSuccessCapture = g_pKTDXApp->GetDGManager()->ScreenCapture();
#endif BANDICAM_CAPTURE
			//}}

			if ( bSuccessCapture )
			{
				if ( g_pChatBox != NULL )
					g_pChatBox->AddChatLog( GET_STRING( STR_ID_347 ), KEGS_CHAT_REQ::CPT_SYSTEM, D3DXCOLOR(1,1,0,1), L"#CFFFF00" );
			}
			else
			{
				if ( g_pChatBox != NULL )
					g_pChatBox->AddChatLog( GET_STRING( STR_ID_348 ), KEGS_CHAT_REQ::CPT_SYSTEM, D3DXCOLOR(1,1,0,1), L"#CFFFF00" );
			}

#ifdef ACTIVE_KOG_GAME_PERFORMANCE_CHECK
			KOGGamePerformanceCheck::GetInstance()->Resume();
#endif//ACTIVE_KOG_GAME_PERFORMANCE_CHECK
		}
	}




#ifdef REFACTORING_BY_TOOL_TEAM

	ExitClientIfHackingFound( fElapsedTime ); 

#else REFACTORING_BY_TOOL_TEAM

#if defined( _SERVICE_ ) 
	ELSWORD_VIRTUALIZER_START
#endif


	if ( g_pKTDXApp->GetFindHacking() == true )
	{
		static bool staticExitCheck = false;
		if ( staticExitCheck == false )
		{
			staticExitCheck = true;
			/*
			char buff[256] = {0};
			string cmdString;
			ConvertWCHARToChar( cmdString, g_pKTDXApp->GetFindHackingDetailMsg() );
			sprintf( buff, "FindH.dll %s", cmdString.c_str() );
			WinExec( buff, SW_HIDE );
			*/

			//g_pKTDXApp->NoticeQuitType( CKTDXApp::KQT_FIND_HACKING );

			m_bAutoQuit = true;
		}

	}

#ifdef GAMEGUARD_DETECTED_HACK_POPUP
	if ( g_pKTDXApp->GetGameGuardFindHacking() == true )
	{
		static bool staticGameGuardExitCheck = false;
		if ( staticGameGuardExitCheck == false )
		{
			staticGameGuardExitCheck = true;

			g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_27366 ), NULL );

			//g_pKTDXApp->NoticeQuitType( CKTDXApp::KQT_FIND_HACKING );

			m_bAutoQuit = true;
		}

	}
#endif //GAMEGUARD_DETECTED_HACK_POPUP


	if ( m_bAutoQuit == true )
	{
		m_fAutoQuitTime += fElapsedTime;
#ifdef ADD_COLLECT_CLIENT_INFO
		if ( m_fAutoQuitTime >= m_fAutoQuitCoolTime && GetSendHackPacket() == false )		
#else
		if ( m_fAutoQuitTime >= m_fAutoQuitCoolTime )
#endif
		{
			g_pMain->SetQuitType( NetError::ERR_CLIENT_QUIT_01 );
			PostQuitMessage(0);
		}
	}


#if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_END
#endif

#endif REFACTORING_BY_TOOL_TEAM

#ifdef USER_REGULATION_NOTICE
		//��������

		// 2012.06.14 lygan_������ // ������ ���ִ� ���� ��Ȯ�ϰ� 1�ð� ���� ������ �����°� �ƴ϶� ���� �۾�
		if( m_TimerUserRegulationNotice.elapsed() > 3600.0f ) // lyagan_������ // �ѽð� ���� ������ ��������.
		{
			if( g_pChatBox != NULL )
			{
				wstring wstrColor = L"#CFFFF00";				
				D3DXCOLOR coTextColor(1.f, 1.f, 0.f, 1.f);	
				g_pChatBox->AddChatLog( GET_STRING( STR_ID_350 ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );

			}

			m_TimerUserRegulationNotice.restart();
		}

		// ��ħ�� �ź�����
#ifdef IDENTITY_CONFIRM

		if( m_TimerIdentityConfirmChat.elapsed() > 3600.0f ) // lyagan_������ // �ѽð� ���� ������ ��������.
		{
			if( g_pChatBox != NULL )
			{
				wstring wstrColor = L"#CFFFF00";				
				D3DXCOLOR coTextColor(1.f, 1.f, 0.f, 1.f);	
				g_pChatBox->AddChatLog( GET_REPLACED_STRING( ( STR_ID_11305, "S", GET_STRING( STR_ID_14497 ) ) ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );
			}
			m_TimerIdentityConfirmChat.restart();
		}
#endif IDENTITY_CONFIRM

#endif //USER_REGULATION_NOTICE

#ifdef SERV_LOCAL_RANKING_SYSTEM
		if( NULL != g_pData->GetProfileManager() && true == g_pData->GetProfileManager()->GetShowRankup() )
		{
			switch( g_pMain->GetNowStateID() )
			{
			case CX2Main::XS_DUNGEON_GAME:
				{
					if( NULL != g_pMain->GetGameOption() && false == g_pMain->GetGameOption()->GetShowRankUpInDungeon() )
					{
						if( 5.0f > g_pData->GetProfileManager()->GetRankupTime() )
						{
							g_pData->GetProfileManager()->CloseRankup();
						}
						break;
					}
				}
				
			case CX2Main::XS_VILLAGE_MAP: //���� �Ǵ� �ʵ忡 �ֱ� ������ rankup ui ����� �̷��.
			case CX2Main::XS_BATTLE_FIELD:
				{
					g_pData->GetProfileManager()->ShowRankUpFade( fElapsedTime );

					break;
				}		

			default:
				break;
			}

			if( 0.0f > g_pData->GetProfileManager()->GetRankupTime() )
			{
				g_pData->GetProfileManager()->CloseRankup();
			}
		}
#endif //SERV_LOCAL_RANKING_SYSTEM

#ifndef DISABLE_PLAY_TIME_INFORMATION
#ifdef CHECK_PLAY_TIME_INFORMATION		//�÷��� �ð��� ���� �ȳ� ���� �ϰ� ����
	//��������
	if( static_cast<int>( g_pKTDXApp->GetTime() ) % 3600 == 0 )
	{
		int iGameHour = static_cast<int>( g_pKTDXApp->GetTime() ) / 3600;

		if( NULL != g_pChatBox && iGameHour != m_iGameHour )
		{
			m_iGameHour = iGameHour;					//�ش� �ð��� �ߺ� ���� ����( �ð����� Double�̶�, int�� ��ȯ�� �� 0���� ������ ���� ������ ���� ������. )
			m_bProcessPlayTimeInformation = true;		//���� �˾�â ���� ������ Satet���� �˻�

			wstring wstrColor = L"#CFFFF00";				
			D3DXCOLOR coTextColor(1.f, 1.f, 0.f, 1.f);
			// ���� �̿� @1�ð��� ����߽��ϴ�.\n������ ���� �̿��� �������� �ϻ� ��Ȱ�� ������ �� �� �ֽ��ϴ�.\n�޽� �� �ٽ� �̿����ֽø� �����ϰڽ��ϴ�.
			g_pChatBox->AddChatLog( GET_REPLACED_STRING( ( STR_ID_17720, "i", m_iGameHour ) ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );
			// ������ ������� Ÿ���� �ֹε�Ϲ�ȣ�� �̿��Ͻ� ��� �������� ���� �� �ֽ��ϴ�.
			g_pChatBox->AddChatLog( GET_STRING( STR_ID_350 ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );
			// ������ �������� ������ ���Ͽ� ���������� �ؽ� ���̵� �� ���� ������ ��й�ȣ�� �����Ͽ� �ּ���.
			g_pChatBox->AddChatLog( GET_STRING( STR_ID_351 ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );
			// �� ������ ��ü �̿밡 �Դϴ�. ������ǥ�ÿ� ���õ� �ڼ��� ������ �޴����� Ȯ���� �� �ֽ��ϴ�.
			g_pChatBox->AddChatLog( GET_STRING( STR_ID_352 ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );
			// ��ڴ� ������ �ؽ�ID, ��й�ȣ �� ���������� �䱸���� ������ ���Ǹ� ��Ź�帳�ϴ�.
			g_pChatBox->AddChatLog( GET_STRING( STR_ID_3700 ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );
			// ���ҵ忡���� ȸ�� �����е鲲 �ڵ��� ����, E-mail, �ؽ����� ���� ���ؼ� U-OTP ������ȣ Ȥ�� U-OTP ������ ���� �䱸���� �ʽ��ϴ�.
			g_pChatBox->AddChatLog( GET_STRING( STR_ID_9710 ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );

		}
	}
#else  CHECK_PLAY_TIME_INFORMATION
	//��������
	if( g_pKTDXApp->GetFrameMoveCount() % 300000 == 0 )
	{
		int gameHour = g_pKTDXApp->GetFrameMoveCount() / 300000;
		if( gameHour == 1 )
		{
			if( g_pChatBox != NULL )
			{
				wstring wstrColor = L"#CFFFF00";				
				D3DXCOLOR coTextColor(1.f, 1.f, 0.f, 1.f);	
				g_pChatBox->AddChatLog( GET_STRING( STR_ID_349 ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );
				g_pChatBox->AddChatLog( GET_STRING( STR_ID_350 ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );
				g_pChatBox->AddChatLog( GET_STRING( STR_ID_351 ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );
				g_pChatBox->AddChatLog( GET_STRING( STR_ID_352 ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );
				g_pChatBox->AddChatLog( GET_STRING( STR_ID_3700 ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );
				g_pChatBox->AddChatLog( GET_STRING( STR_ID_9710 ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );
			}
		}
		else if( gameHour >= 3 )
		{
			if( g_pChatBox != NULL )
			{
				wstring wstrColor = L"#CFFFF00";				
				D3DXCOLOR coTextColor(1.f, 1.f, 0.f, 1.f);
				g_pChatBox->AddChatLog( GET_REPLACED_STRING( ( STR_ID_353, "i", gameHour ) ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );
				g_pChatBox->AddChatLog( GET_STRING( STR_ID_350 ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );
				g_pChatBox->AddChatLog( GET_STRING( STR_ID_351 ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );
				g_pChatBox->AddChatLog( GET_STRING( STR_ID_352 ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );
				g_pChatBox->AddChatLog( GET_STRING( STR_ID_3700 ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );
				g_pChatBox->AddChatLog( GET_STRING( STR_ID_9710 ), KEGS_CHAT_REQ::CPT_SYSTEM, coTextColor, wstrColor );

			}
		}
		//- ������ ������� Ÿ���� �ֹε�Ϲ�ȣ�� �̿��Ͻ� ��� �������� ���� �� �ֽ��ϴ�.
		//- ������ �������� ������ ���Ͽ� ���������� �ؽ� ���̵��� ��й�ȣ�� ������ �ֽñ� �ٶ��ϴ�.
	}
#endif CHECK_PLAY_TIME_INFORMATION

#ifdef CHECK_PLAY_TIME_INFORMATION
	if( true == m_bProcessPlayTimeInformation || true == m_bProcessShutDownInformation )
	{
		switch(g_pMain->GetNowStateID())	//�ش� ������ �ֱ� ������ �ȳ�â ������ �̷��.
		{
			//���� �� �ŷ� ����
		case XS_VILLAGE_MAP:
		case XS_SQUARE_GAME:
#ifdef ADDED_RELATIONSHIP_SYSTEM
		case XS_WEDDING_GAME:
#endif // ADDED_RELATIONSHIP_SYSTEM
			{
				if( 0.f >= m_fShutDownInfoShowTime && true == m_bProcessPlayTimeInformation )			//�˴ٿ� �ȳ�â�� �����Ǿ� ���� ��, �÷��� �ð� �ȳ�â ������ �̷��.
					CreatePlayTimeInformation();
				else if( 0.f >= m_fPlayTimeInfoShowTime && true == m_bProcessShutDownInformation )		//�÷��� �ð� �ȳ�â�� �����Ǿ� ���� ��, �˴ٿ� �ȳ�â ������ �̷��.
					CreateShutDownInformation();
			}break;
			//X2Game ����Ǵ� ����
		case XS_PVP_GAME:
		case XS_DUNGEON_GAME:
		case XS_TRAINING_GAME:
			{
				if( NULL != g_pX2Game && CX2Game::GS_PLAY == g_pX2Game->GetGameState() )				//�ε��߿� �ȳ�â ���� �̷��.
				{
					if( 0.f >= m_fShutDownInfoShowTime && true == m_bProcessPlayTimeInformation )		//�˴ٿ� �ȳ�â�� �����Ǿ� ���� ��, �÷��� �ð� �ȳ�â ������ �̷��.
						CreatePlayTimeInformation();
					else if( 0.f >= m_fPlayTimeInfoShowTime && true == m_bProcessShutDownInformation )	//�÷��� �ð� �ȳ�â�� �����Ǿ� ���� ��, �˴ٿ� �ȳ�â ������ �̷��.
						CreateShutDownInformation();
				}

			} break;
		}
	}

	if( 0.f < m_fPlayTimeInfoShowTime )
	{
		FadePlayTimeInformation( m_fPlayTimeInfoShowTime );	//Fade ȿ��

		m_fPlayTimeInfoShowTime -= fElapsedTime;

		if( 0.f >= m_fPlayTimeInfoShowTime )
		{
			DeletePlayTimeInformation();		//���� �ð� ����� ����

			m_fPlayTimeInfoShowTime = 0.f;
		}
	}

	if( 0.f < m_fShutDownInfoShowTime )
	{
		FadeShutDownInformation( m_fShutDownInfoShowTime );	//Fade ȿ��

		m_fShutDownInfoShowTime -= fElapsedTime;

		if( 0.f >= m_fShutDownInfoShowTime )
		{
			DeleteShutDownInformation();		//���� �ð� ����� ����

			m_fShutDownInfoShowTime = 0.f;
		}
	}
#endif CHECK_PLAY_TIME_INFORMATION

#endif DISABLE_PLAY_TIME_INFORMATION

	//nProtect �� ����Ǿ� ���� �ʴٸ� ��������
	if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
		KNP_GetFindGameMon() == false )
	{
		g_pMain->SetQuitType( NetError::ERR_CLIENT_QUIT_01 );
		g_pKTDXApp->NoticeQuitType( CKTDXApp::KQT_FIND_HACKING );
		PostQuitMessage(0);
	}

#ifndef DISABLE_DISAGREE_HACK_USER
	if ( g_pData != NULL && g_pData->GetMyUser() != NULL )
	{
		if ( g_pData->GetMyUser()->GetUserData()->hackingUserType == CX2User::HUT_AGREE_HACK_USER )
		{
			m_fTimeToUpdateProcessList += fElapsedTime;
			if ( m_fTimeToUpdateProcessList >= 120.0f )
			{
				m_fTimeToUpdateProcessList = 0.0f;
#ifndef PROCESSLIST
				UpdateProcessList();
#endif
			}
		}
	}
#endif DISABLE_DISAGREE_HACK_USER

	if( NULL != GetPartyUI() )
	{
		GetPartyUI()->OnFrameMove( fTime, fElapsedTime );
	}

	//{{ kimhc // 2010.01.22 // list�� ����
	list<TimedMessagePopUp>::iterator ItrTimedPopUp = m_listTimedMessagePopUp.begin();

	while ( ItrTimedPopUp != m_listTimedMessagePopUp.end() )
	{
		if ( ItrTimedPopUp->m_fTimeLeft <= 0.f )
		{
			if ( -1 != ItrTimedPopUp->m_iTimeOutMsg )
			{
				g_pKTDXApp->SendGameMessage( CKTDXApp::KM_UI_CONTROL_CUSTOM_EVENT, ItrTimedPopUp->m_iTimeOutMsg, NULL, false );
			}

			m_listTimedMessagePopUp.erase( ItrTimedPopUp++ );
			continue;
		}

		ItrTimedPopUp->OnFrameMove( fTime, fElapsedTime );

		ItrTimedPopUp++;
	}
// 	for( int i=0; i< (int)m_listTimedMessagePopUp.size(); i++ )
// 	{
// 		TimedMessagePopUp& timedPopUp = m_vecTimedMessagePopUp[i];
// 
// 		if( timedPopUp.m_fTimeLeft <= 0.f )
// 		{
// 			if( -1 != timedPopUp.m_iTimeOutMsg )
// 			{
// 				g_pKTDXApp->SendGameMessage( CKTDXApp::KM_UI_CONTROL_CUSTOM_EVENT, timedPopUp.m_iTimeOutMsg, NULL, false );
// 			}
// 
// 			m_vecTimedMessagePopUp.erase( m_vecTimedMessagePopUp.begin() + i );
// 			i--;
// 			continue;
// 		}
// 
// 		timedPopUp.OnFrameMove( fTime, fElapsedTime );
// 	}
	//}} kimhc // 2010.01.22 // list�� ����

#ifdef SERV_SKILL_NOTE
	if( m_pDlgInfoBox != NULL && m_fInfoBoxLifeTime > 0.f )
	{
		m_fInfoBoxLifeTime -= fElapsedTime;
		if( m_fInfoBoxLifeTime <= 0.f && m_fInfoBoxLifeTime > -1.f )
		{
			g_pKTDXApp->GetDGManager()->GetDialogManager()->DeleteDlg(m_pDlgInfoBox);
			m_fInfoBoxLifeTime = -1.f;
		}

		if( m_fInfoBoxLifeTime <= -1.f )
			m_fInfoBoxLifeTime = -1.f;
	}	
#endif

#ifdef BANDICAM_CAPTURE
	m_fCaptureDelay -= fElapsedTime;
	if( m_fCaptureDelay < 0.0f )
		m_fCaptureDelay = 0.0f;

	Bandi_EnoughDiskSpace(fElapsedTime);
#endif BANDICAM_CAPTURE

	return S_OK;
}



bool CX2Main::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
		case XGM_STATE_CHANGE:
			{
				X2_STATE state = (X2_STATE)wParam;
				int iDetailStateID = (int)lParam;
	
				return StateChange( state, iDetailStateID );
			}
			break;

		case XGM_DELETE_DIALOG:
			{
#ifdef DIALOG_HANDLE_TEST
                //CKTDGUIDialogType pDialog( CKTDGUIDialog::MakeValidHandle( wParam ) );
				// added by wonpok. 20090820.
				CKTDGUIDialogType pDialog( CKTDGUIDialog::MakeValidHandle( wParam, lParam ) );
#else
				CKTDGUIDialogType pDialog = (CKTDGUIDialogType)wParam;
#endif // DIALOG_HANDLE_TEST

				if ( pDialog == NULL || g_pKTDXApp->GetDGManager()->GetDialogManager()->CheckDlg( pDialog ) == false )
				{
					//g_pKTDXApp->GetDGManager()->GetDialogManager()->DeleteDlg( pDialog );
					return true;
				}

				wstring MsgBoxName = pDialog->GetName();

				if ( MsgBoxName == L"OkAndCancelMsgBox" )
				{
					g_pKTDXApp->GetDIManager()->SetEnable(true);
				}

				D3DXCOLOR tempColor = pDialog->GetColor();
				tempColor.a = 0.0f;
				pDialog->Move(pDialog->GetPos(), tempColor, 0.15f, true, true);
				return true;
			}
			break;

		case XGM_REQUEST_FOCUS_CONTROL:
			{
				CKTDGUIControl*	pControl = (CKTDGUIControl*)wParam;
				pControl->RequestFocus();
				return true;
			}
			break;
#ifdef _NEXON_KR_
		case XGM_NEXON_CALLBACK:
			{
				m_CX2NexonMsgHandler.HandleNMEvent( wParam, lParam );
				return true;
			}
			break;
#endif _NEXON_KR_

		case XGM_QUIT_GAME:
			{
			//{{ kimhc // 2010.3.10 //	�������⵵ ���̺귯�� ����
		#ifdef	IGA_TEST
				CX2IGA::GetInstance()->Clear();
				CX2IGA::GetInstance()->ReleaseInstance();
		#endif	IGA_TEST
			//}} kimhc // 2010.3.10 //	�������⵵ ���̺귯�� ����

#ifdef CLIENT_PORT_CHANGE_REQUEST
				if ( g_pInstanceData != NULL)
				{
					if ( g_pData != NULL && g_pData->GetGameUDP() != NULL && g_pData->GetGameUDP()->GetMyPort() != 0 )
					{

						if ( g_pData->GetServerProtocol() != NULL )
						{
							KEGS_CLIENT_POPRT_CHANGE_REQUEST_INFO_NOT kPacket;
							kPacket.m_iUDPPortSuccessType = g_pInstanceData->GetUDPPortSuccessType();
							kPacket.m_iUseUdpPort = g_pData->GetGameUDP()->GetMyPort();
							g_pData->GetServerProtocol()->SendPacket( EGS_CLIENT_POPRT_CHANGE_REQUEST_INFO_NOT, kPacket );
						}
					}

				}

#endif //CLIENT_PORT_CHANGE_REQUEST

				PostQuitMessage(0);
				return true;
			}
			break;
			//{{ ������ // 2012-03-20 // UI�� ���� ���̾�α׸� UI ���� �� ���� �ϵ��� ����� ���� �߰�			
#ifdef UI_RESOURCE_REALTIME_LOAD
		case XGM_REAL_DELETE_DIALOG:
			{
#ifdef DIALOG_HANDLE_TEST
				//CKTDGUIDialogType pDialog( CKTDGUIDialog::MakeValidHandle( wParam ) );
				// added by wonpok. 20090820.
				CKTDGUIDialogType pDialog( CKTDGUIDialog::MakeValidHandle( wParam, lParam ) );
#else
				CKTDGUIDialogType pDialog = (CKTDGUIDialogType)wParam;
#endif // DIALOG_HANDLE_TEST

				if ( pDialog == NULL || g_pKTDXApp->GetDGManager()->GetDialogManager()->CheckDlg( pDialog ) == false )
				{
					return true;
				}

				SAFE_DELETE_DIALOG(pDialog);

				return true;
			}
			break;
#endif //UI_RESOURCE_REALTIME_LOAD
			//}} ������ // 2012-03-20 // UI�� ���� ���̾�α׸� UI ���� �� ���� �ϵ��� ����� ���� �߰�
		case WM_SYSCOMMAND:
			if( wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER )
			{
				return true;
			}
			break;			

		case CKTDXApp::KM_WEB_BROWSER_KEY_EVENT:
			{
				if( NULL != g_pMain->GetBrowserWrapper() && false == g_pMain->GetBrowserWrapper()->IsClosed() )
				{
					g_pMain->GetBrowserWrapper()->OnKeyboardEvents( (LPMSG) lParam );
				}
				return true;
			}
			break;
	}

	switch(wParam)
	{
		//{{����ȯ �����ð� �޾ƿ���
#ifdef	SERV_SERVER_TIME_GET
		case ECH_GET_SERVER_TIME_ACK:
			{
				return Handler_ECH_GET_SERVER_TIME_ACK( hWnd, uMsg, wParam, lParam );
			}
			break;
#endif SERV_SERVER_TIME_GET
#ifdef CHECK_KOM_FILE_ON_LOAD
#ifdef SERV_KOM_FILE_CHECK_ADVANCED
		case EGS_KOM_FILE_CHECK_LOG_ACK:
			{
				return Handler_EGS_KOM_FILE_CHECK_LOG_ACK( hWnd, uMsg, wParam, lParam );
			}
			break;
#endif SERV_KOM_FILE_CHECK_ADVANCED			
#endif CHECK_KOM_FILE_ON_LOAD
	}

#ifdef SERV_PVP_NEW_SYSTEM
	if( g_pMain->GetPartyUI() != NULL && g_pMain->GetPartyUI()->GetShow() == true &&
		GetPartyUI()->MsgProc(hWnd, uMsg, wParam, lParam) )
	{
		return true;
	}
#endif

#ifdef EVENT_SCENE
// oasis907 : ����� [2010.6.17] //  �׽�Ʈ�� �ӽ� ��ġ ����ڽ��ϴ�.
	if(g_pData->GetEventScene() != NULL)
	{
		return g_pData->GetEventScene()->MsgProc(hWnd, uMsg, wParam, lParam );
	}
#endif EVENT_SCENE

	return false;
}



void CX2Main::RegisterLuabind( KLuabinder* pKLuabinder )
{

	lua_State* L= pKLuabinder->GetLuaState();



	lua_tinker::decl( pKLuabinder->GetLuaState(),  "g_pMain", this );

	lua_tinker::class_add<CX2Room>( L, "CX2Room" );
	lua_tinker::class_def<CX2Room>( L, "GetSlotData",							&CX2Room::GetSlotData );
	lua_tinker::class_def<CX2Room>( L, "GetSlotNum",							&CX2Room::GetSlotNum );
	lua_tinker::class_def<CX2Room>( L, "GetRoomPublic",							&CX2Room::GetRoomPublic );
	lua_tinker::class_def<CX2Room>( L, "GetRoomState",							&CX2Room::GetRoomState_LUA );
	lua_tinker::class_def<CX2Room>( L, "GetMySlot",								&CX2Room::GetMySlot );
	


#ifdef EVENT_SCENE
	lua_tinker::class_add<CX2EventScene>( L, "CX2EventScene" );
	lua_tinker::class_def<CX2EventScene>( L, "AddEventSceneObject_LUA",			&CX2EventScene::AddEventSceneObject_LUA );
	lua_tinker::class_def<CX2EventScene>( L, "AddEventScene_LUA",				&CX2EventScene::AddEventScene_LUA );
#endif EVENT_SCENE



	lua_tinker::class_add<CX2SlideShot>( L, "CX2SlideShot" );
	lua_tinker::class_def<CX2SlideShot>( L, "ScenStart_LUA",					&CX2SlideShot::ScenStart_LUA);
	lua_tinker::class_def<CX2SlideShot>( L, "ScenWait",							&CX2SlideShot::ScenWait);
	lua_tinker::class_def<CX2SlideShot>( L, "ScenEnd",							&CX2SlideShot::ScenEnd);
	lua_tinker::class_def<CX2SlideShot>( L, "GoNextScen",						&CX2SlideShot::GoNextScen);
	lua_tinker::class_def<CX2SlideShot>( L, "GetNowTime",						&CX2SlideShot::GetNowTime);
	lua_tinker::class_def<CX2SlideShot>( L, "EventTimer",						&CX2SlideShot::EventTimer);
	lua_tinker::class_def<CX2SlideShot>( L, "CannotInput",						&CX2SlideShot::CannotInput );
	lua_tinker::class_def<CX2SlideShot>( L, "AddText_LUA",						&CX2SlideShot::AddText_LUA );
	lua_tinker::class_def<CX2SlideShot>( L, "AddText2_LUA",						&CX2SlideShot::AddText2_LUA);
	lua_tinker::class_def<CX2SlideShot>( L, "AddSeqMap_LUA",					&CX2SlideShot::AddSeqMap_LUA);
	lua_tinker::class_def<CX2SlideShot>( L, "CrashSeq_LUA",						&CX2SlideShot::CrashSeq_LUA);
	lua_tinker::class_def<CX2SlideShot>( L, "ChangeColor_LUA",					&CX2SlideShot::ChangeColor_LUA);
	lua_tinker::class_def<CX2SlideShot>( L, "ChangeTex_LUA",					&CX2SlideShot::ChangeTex_LUA);
	lua_tinker::class_def<CX2SlideShot>( L, "ChangeSize_LUA",					&CX2SlideShot::ChangeSize_LUA);
	lua_tinker::class_def<CX2SlideShot>( L, "DeleteSeq_LUA",					&CX2SlideShot::DeleteSeq_LUA);
	lua_tinker::class_def<CX2SlideShot>( L, "SetTextBoxShow",					&CX2SlideShot::SetTextBoxShow);
	lua_tinker::class_def<CX2SlideShot>( L, "BlackLayOutMove",					&CX2SlideShot::BlackLayOutMove );
	lua_tinker::class_def<CX2SlideShot>( L, "SetTextSpread",					&CX2SlideShot::SetTextSpread);
	lua_tinker::class_def<CX2SlideShot>( L, "IsTextSpreading",					&CX2SlideShot::IsTextSpreading );
	lua_tinker::class_def<CX2SlideShot>( L, "GetElapsedTimeAfterTextSpread",	&CX2SlideShot::GetElapsedTimeAfterTextSpread );
	lua_tinker::class_def<CX2SlideShot>( L, "IsPresentNow",						&CX2SlideShot::IsPresentNow);
	//{{ 2010.04.28 / �ڱ��� / ������ ��� ����
	lua_tinker::class_def<CX2SlideShot>( L, "SetNameTabShow",					&CX2SlideShot::SetNameTabShow);
	lua_tinker::class_def<CX2SlideShot>( L, "SetSpeechBallonShow",				&CX2SlideShot::SetSpeechBallonShow);
	lua_tinker::class_def<CX2SlideShot>( L, "SetEnterShow",						&CX2SlideShot::SetEnterShow);
	//}}
#ifdef REFORM_TUTORIAL
	lua_tinker::class_def<CX2SlideShot>( L, "PlaySound2D_LUA",					&CX2SlideShot::PlaySound2D_LUA);
	lua_tinker::class_def<CX2SlideShot>( L, "StopAllSound_LUA",					&CX2SlideShot::StopAllSound_LUA);
	lua_tinker::class_def<CX2SlideShot>( L, "SetSlideBGM_LUA",					&CX2SlideShot::SetSlideBGM_LUA);
	lua_tinker::class_def<CX2SlideShot>( L, "ResetBGM",							&CX2SlideShot::ResetBGM);
#endif //REFORM_TUTORIAL



	lua_tinker::class_add<CX2TrainingGameUI>( L, "CX2TrainingGameUI" );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "ScenStart_LUA",						&CX2TrainingGameUI::ScenStart_LUA);
	lua_tinker::class_def<CX2TrainingGameUI>( L, "ScenWait",							&CX2TrainingGameUI::ScenWait);
	lua_tinker::class_def<CX2TrainingGameUI>( L, "ScenEnd",								&CX2TrainingGameUI::ScenEnd);
	lua_tinker::class_def<CX2TrainingGameUI>( L, "GoNextScen",							&CX2TrainingGameUI::GoNextScen);
	lua_tinker::class_def<CX2TrainingGameUI>( L, "GetNowTime",							&CX2TrainingGameUI::GetNowTime);
	lua_tinker::class_def<CX2TrainingGameUI>( L, "EventTimer",							&CX2TrainingGameUI::EventTimer);
	lua_tinker::class_def<CX2TrainingGameUI>( L, "AddText_LUA",							&CX2TrainingGameUI::AddText_LUA );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "IsKeyPressed",						&CX2TrainingGameUI::IsKeyPressed);

	lua_tinker::class_def<CX2TrainingGameUI>( L, "ClearWaitingKeyPress",				&CX2TrainingGameUI::ClearWaitingKeyPress );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "AddWaitingKeyPress",					&CX2TrainingGameUI::AddWaitingKeyPress );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "AddWaitingKeyPressScaled",			&CX2TrainingGameUI::AddWaitingKeyPressScaled );

	lua_tinker::class_def<CX2TrainingGameUI>( L, "SetTextSpread",						&CX2TrainingGameUI::SetTextSpread);
	lua_tinker::class_def<CX2TrainingGameUI>( L, "IsTextSpreading",						&CX2TrainingGameUI::IsTextSpreading );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "GetElapsedTimeAfterTextSpread",		&CX2TrainingGameUI::GetElapsedTimeAfterTextSpread );

	lua_tinker::class_def<CX2TrainingGameUI>( L, "SetEnableWaitingUserState",			&CX2TrainingGameUI::SetEnableWaitingUserState );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "AddWaitingUserState_LUA",				&CX2TrainingGameUI::AddWaitingUserState_LUA );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "IsWaitingUserState",					&CX2TrainingGameUI::IsWaitingUserState );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "ClearWaitingUserState",				&CX2TrainingGameUI::ClearWaitingUserState );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "AddSimpleWaitingUserState",			&CX2TrainingGameUI::AddSimpleWaitingUserState );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "IsWaitingSimpleUserState",			&CX2TrainingGameUI::IsWaitingSimpleUserState );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "ClearSimpleWaitingUserState",			&CX2TrainingGameUI::ClearSimpleWaitingUserState );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "GetElapsedTimeAfterCompletingSimpleUserState",		&CX2TrainingGameUI::GetElapsedTimeAfterCompletingSimpleUserState );

	lua_tinker::class_def<CX2TrainingGameUI>( L, "ClearGatherItem",						&CX2TrainingGameUI::ClearGatherItem );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "AddGatherItem_LUA",					&CX2TrainingGameUI::AddGatherItem_LUA );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "IsWaitingGatherItem",					&CX2TrainingGameUI::IsWaitingGatherItem );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "ClearWaitingSpecialAttack",			&CX2TrainingGameUI::ClearWaitingSpecialAttack );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "AddWaitingSpecialAttack_LUA",			&CX2TrainingGameUI::AddWaitingSpecialAttack_LUA );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "IsWaitingSpecialAttack",				&CX2TrainingGameUI::IsWaitingSpecialAttack );

	lua_tinker::class_def<CX2TrainingGameUI>( L, "AddWaitingSeqUserState_LUA",			&CX2TrainingGameUI::AddWaitingSeqUserState_LUA );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "SetWaitingSeqUserState",				&CX2TrainingGameUI::SetWaitingSeqUserState );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "ClearWaitingSeqUserState",			&CX2TrainingGameUI::ClearWaitingSeqUserState );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "IsWaitingSeqUserState",				&CX2TrainingGameUI::IsWaitingSeqUserState );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "ClearKeySequence",					&CX2TrainingGameUI::ClearKeySequence );

	lua_tinker::class_def<CX2TrainingGameUI>( L, "ClearTrainingObjective",				&CX2TrainingGameUI::ClearTrainingObjective );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "AddTrainingObjective",				&CX2TrainingGameUI::AddTrainingObjective );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "AddTrainingObjectivePicture",			&CX2TrainingGameUI::AddTrainingObjectivePicture );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "AddTrainingObjectivePictureScaled",	&CX2TrainingGameUI::AddTrainingObjectivePictureScaled );

	lua_tinker::class_def<CX2TrainingGameUI>( L, "SetShowEnterMark",					&CX2TrainingGameUI::SetShowEnterMark );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "SetTwinkleEnterMark",					&CX2TrainingGameUI::SetTwinkleEnterMark );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "SetShowSuccessCountEffect",			&CX2TrainingGameUI::SetShowSuccessCountEffect );

	lua_tinker::class_def<CX2TrainingGameUI>( L, "GetFTNPCID",							&CX2TrainingGameUI::GetFTNPCID );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "GetFTNPCLevel",						&CX2TrainingGameUI::GetFTNPCLevel );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "GetFTNPCCount",						&CX2TrainingGameUI::GetFTNPCCount );
	lua_tinker::class_def<CX2TrainingGameUI>( L, "GetFTEnableNPCAttack",				&CX2TrainingGameUI::GetFTEnableNPCAttack );
	



	lua_tinker::class_add<CX2Room::SlotData>( L, "CX2Room::SlotData" );
	//lua_tinker::class_def<TEST_CLASS>( L, lua_tinker::constructor<>( L ); );
	//.def_readwrite("m_bHost", &CX2Room::SlotData::m_bHost );
	lua_tinker::class_def<CX2Room::SlotData>( L, "GetIsHost",				&CX2Room::SlotData::GetIsHost );
	lua_tinker::class_def<CX2Room::SlotData>( L, "GetIsReady",				&CX2Room::SlotData::GetIsReady );
	lua_tinker::class_def<CX2Room::SlotData>( L, "GetIsPitIn",				&CX2Room::SlotData::GetIsPitIn );
	lua_tinker::class_def<CX2Room::SlotData>( L, "GetSlotState",			&CX2Room::SlotData::GetSlotState );
	lua_tinker::class_def<CX2Room::SlotData>( L, "GetTeamNum",				&CX2Room::SlotData::GetTeamNum );
	lua_tinker::class_def<CX2Room::SlotData>( L, "IsMyUnit",				&CX2Room::SlotData::IsMyUnit );
	

	lua_tinker::class_add<CX2User>( L, "CX2User" );
	lua_tinker::class_def<CX2User>( L, "GetUnitNum",		&CX2User::GetUnitNum );
	lua_tinker::class_def<CX2User>( L, "GetUnitByIndex",	&CX2User::GetUnitByIndex );
	


	lua_tinker::class_add<CX2Unit>( L, "CX2Unit" );
	lua_tinker::class_def<CX2Unit>( L, "GetClass",			&CX2Unit::GetClass );
	lua_tinker::class_def<CX2Unit>( L, "GetNickName",		&CX2Unit::GetNickName );
	

	


	lua_tinker::class_add<CX2PVPRoom>( L, "CX2PVPRoom" );
	lua_tinker::class_inh< CX2PVPRoom, CX2Room>( L );
	lua_tinker::class_def<CX2PVPRoom>( L, "GetPVPGameType",				&CX2PVPRoom::GetPVPGameType_LUA );
	lua_tinker::class_def<CX2PVPRoom>( L, "GetWinKillNum",				&CX2PVPRoom::GetWinKillNum );
	lua_tinker::class_def<CX2PVPRoom>( L, "GetPlayTime",				&CX2PVPRoom::GetPlayTimeLimit );
	lua_tinker::class_def<CX2PVPRoom>( L, "GetCanIntrude",				&CX2PVPRoom::GetCanIntrude );
	lua_tinker::class_def<CX2PVPRoom>( L, "GetWorldID",					&CX2PVPRoom::GetWorldID );
	lua_tinker::class_def<CX2PVPRoom>( L, "SetPVPGameType",				&CX2PVPRoom::SetPVPGameType );
	lua_tinker::class_def<CX2PVPRoom>( L, "SetWinKillNum",				&CX2PVPRoom::SetWinKillNum );
	lua_tinker::class_def<CX2PVPRoom>( L, "SetPlayTime",				&CX2PVPRoom::SetPlayTime );
	lua_tinker::class_def<CX2PVPRoom>( L, "SetCanIntrude",				&CX2PVPRoom::SetCanIntrude );
	lua_tinker::class_def<CX2PVPRoom>( L, "SetWorldID",					&CX2PVPRoom::SetWorldID );
	

	lua_tinker::class_add<CX2DungeonRoom>( L, "CX2DungeonRoom" );
	lua_tinker::class_inh< CX2DungeonRoom, CX2Room>( L );
	lua_tinker::class_def<CX2DungeonRoom>( L, "GetDifficulty",			&CX2DungeonRoom::GetDifficulty );
	lua_tinker::class_def<CX2DungeonRoom>( L, "GetDungeonID",			&CX2DungeonRoom::GetDungeonID );
	lua_tinker::class_def<CX2DungeonRoom>( L, "SetDifficulty",			&CX2DungeonRoom::SetDifficulty );
	lua_tinker::class_def<CX2DungeonRoom>( L, "SetDungeonID",			&CX2DungeonRoom::SetDungeonID );
	

	lua_tinker::class_add<CX2Data>( L, "CX2Data" );
	lua_tinker::class_def<CX2Data>( L, "GetPVPRoom",					&CX2Data::GetPVPRoom );
	lua_tinker::class_def<CX2Data>( L, "GetDungeonRoom",				&CX2Data::GetDungeonRoom );
	lua_tinker::class_def<CX2Data>( L, "GetMyUser",						&CX2Data::GetMyUser );
	lua_tinker::class_def<CX2Data>( L, "GetUIMajorParticle",			&CX2Data::GetUIMajorParticle );
	lua_tinker::class_def<CX2Data>( L, "GetUIMajorXMeshPlayer",			&CX2Data::GetUIMajorXMeshPlayer );
	lua_tinker::class_def<CX2Data>( L, "GetUIMinorParticle",			&CX2Data::GetUIMinorParticle );
	lua_tinker::class_def<CX2Data>( L, "GetUIMinorXMeshPlayer",			&CX2Data::GetUIMinorXMeshPlayer );
	lua_tinker::class_def<CX2Data>( L, "GetUIUnitManager",				&CX2Data::GetUIUnitManager );
	lua_tinker::class_def<CX2Data>( L, "LoadPicCharAll_LUA",			&CX2Data::LoadPicCharAll_LUA );
#ifdef SERV_LOCAL_RANKING_SYSTEM
	lua_tinker::class_def<CX2Data>( L, "AddArea_LUA",					&CX2Data::AddArea_LUA );
#endif //SERV_LOCAL_RANKING_SYSTEM


	lua_tinker::class_add<CX2State>( L, "CX2State" );
	lua_tinker::class_inh<CX2State, CKTDXStage>( L );
	lua_tinker::class_def<CX2State>( L, "Handler_EGS_NOTIFY_MSG_REQ",					&CX2State::Handler_EGS_NOTIFY_MSG_REQ_LUA );
	lua_tinker::class_def<CX2State>( L, "Handler_EGS_ADMIN_MODIFY_UNIT_LEVEL_REQ",		&CX2State::Handler_EGS_ADMIN_MODIFY_UNIT_LEVEL_REQ_LUA );
	lua_tinker::class_def<CX2State>( L, "Handler_EGS_ADMIN_CHANGE_ED_REQ",				&CX2State::Handler_EGS_ADMIN_CHANGE_ED_REQ_LUA );
	lua_tinker::class_def<CX2State>( L, "Handler_EGS_ADMIN_KICK_USER_REQ",				&CX2State::Handler_EGS_ADMIN_KICK_USER_REQ_LUA );
	lua_tinker::class_def<CX2State>( L, "Handler_EGS_ADMIN_SET_PC_BANG_REQ",			&CX2State::Handler_EGS_ADMIN_SET_PC_BANG_REQ );

#ifdef SERV_HERO_PVP_MANAGE_LIST
#ifdef ADD_HERO_MATCH_NOTICE		/// ���� ���� ��� ���
//	lua_tinker::class_def<CX2State>( L, "Handler_EGS_ADMIN_REMOVE_HERO_PVP_USER_REQ",		&CX2State::Handler_EGS_ADMIN_REMOVE_HERO_PVP_USER_REQ );
	lua_tinker::class_def<CX2State>( L, "Handler_EGS_ADMIN_NOTIFY_HERO_PVP_USER_REQ_LUA",	&CX2State::Handler_EGS_ADMIN_NOTIFY_HERO_PVP_USER_REQ_LUA );
#endif ADD_HERO_MATCH_NOTICE


	//{{ 2012. 12. 5	������		������ ���� ���� ���
#ifdef SERV_OBSERVER_PVP_INTRUDE_MODE
	lua_tinker::class_def<CX2State>( L, "Handler_EGS_ADMIN_OBSERVER_MODE_REQ",			&CX2State::Handler_EGS_ADMIN_OBSERVER_MODE_REQ );
#endif SERV_OBSERVER_PVP_INTRUDE_MODE
	//}}
#endif //SERV_HERO_PVP_MANAGE_LIST

	lua_tinker::class_add<CX2StatePVPRoom>( L, "CX2StatePVPRoom" );
	lua_tinker::class_inh<CX2StatePVPRoom, CX2StateMenu>( L );
	lua_tinker::class_def<CX2StatePVPRoom>( L, "Handler_EGS_CHANGE_SLOT_OPEN_REQ",		&CX2StatePVPRoom::Handler_EGS_CHANGE_SLOT_OPEN_REQ );
	



	lua_tinker::class_add<CX2Main>( L, "CX2Main" );
	lua_tinker::class_def<CX2Main>( L, "SetChannelServerPort",			&CX2Main::SetChannelServerPort );
	lua_tinker::class_def<CX2Main>( L, "AddChannelServerIP_LUA",		&CX2Main::AddChannelServerIP_LUA );
	lua_tinker::class_def<CX2Main>( L, "SetDefaultChannelServerIPIndex",&CX2Main::SetDefaultChannelServerIPIndex );
	lua_tinker::class_def<CX2Main>( L, "SetServerTimeOut",				&CX2Main::SetServerTimeOut_LUA );
	lua_tinker::class_def<CX2Main>( L, "SetAutoLoginUnitIndex",			&CX2Main::SetAutoLoginUnitIndex );
	lua_tinker::class_def<CX2Main>( L, "SetSafetyKey",					&CX2Main::SetSafetyKey_LUA );
	lua_tinker::class_def<CX2Main>( L, "ResetFullScreenResolution",		&CX2Main::ResetFullScreenResolution );
	lua_tinker::class_def<CX2Main>( L, "KTDGUIOKMsgBox",				&CX2Main::KTDGUIOKMsgBox_LUA );
	lua_tinker::class_def<CX2Main>( L, "KTDGUIOkAndCancelMsgBox",		&CX2Main::KTDGUIOkAndCancelMsgBox_LUA );
	lua_tinker::class_def<CX2Main>( L, "InstallFont",					&CX2Main::InstallFont );
	lua_tinker::class_def<CX2Main>( L, "SetClientVersion_LUA",			&CX2Main::SetClientVersion_LUA );
	lua_tinker::class_def<CX2Main>( L, "DeleteFile",					&CX2Main::DeleteFile_LUA );
#ifdef CLIENT_USE_NATION_FLAG
	lua_tinker::class_def<CX2Main>( L, "SetNationFlag",					&CX2Main::SetNationFlag_LUA );

	lua_tinker::decl( L, "NF_NONE",										CX2Main::NF_NONE );
	lua_tinker::decl( L, "NF_DE",										CX2Main::NF_DE );
	lua_tinker::decl( L, "NF_FR",										CX2Main::NF_FR );
	lua_tinker::decl( L, "NF_IT",										CX2Main::NF_IT );
	lua_tinker::decl( L, "NF_PL",										CX2Main::NF_PL );
	lua_tinker::decl( L, "NF_ES",										CX2Main::NF_ES );
	lua_tinker::decl( L, "NF_TR",										CX2Main::NF_TR );
	lua_tinker::decl( L, "NF_EN",										CX2Main::NF_EN );

	lua_tinker::decl( L, "NF_KR",										CX2Main::NF_KR );
	lua_tinker::decl( L, "NF_JP",										CX2Main::NF_JP );
	lua_tinker::decl( L, "NF_TW",										CX2Main::NF_TW );
	lua_tinker::decl( L, "NF_HK",										CX2Main::NF_HK );
	lua_tinker::decl( L, "NF_US",										CX2Main::NF_US );
	lua_tinker::decl( L, "NF_CN",										CX2Main::NF_CN );
	lua_tinker::decl( L, "NF_VN",										CX2Main::NF_VN );
	lua_tinker::decl( L, "NF_TH",										CX2Main::NF_TH );
	lua_tinker::decl( L, "NF_ID",										CX2Main::NF_ID );
	lua_tinker::decl( L, "NF_BR",										CX2Main::NF_BR );
	lua_tinker::decl( L, "NF_PH",										CX2Main::NF_PH );
#endif CLIENT_USE_NATION_FLAG
	lua_tinker::class_def<CX2Main>( L, "BuyItemSuper_LUA",				&CX2Main::BuyItemSuper_LUA );
	lua_tinker::class_def<CX2Main>( L, "BuySuperEqipLevel_LUA",			&CX2Main::BuySuperEqipLevel_LUA );
	lua_tinker::class_def<CX2Main>( L, "SetAuthLevel_LUA",				&CX2Main::SetAuthLevel_LUA );
	lua_tinker::class_def<CX2Main>( L, "GetAuthUserList_LUA",			&CX2Main::GetAuthUserList_LUA );
	lua_tinker::class_def<CX2Main>( L, "SetSubClientVersion_LUA",		&CX2Main::SetSubClientVersion_LUA );
	lua_tinker::class_def<CX2Main>( L, "InitMyUnitSkillTree",			&CX2Main::InitMyUnitSkillTree );
	lua_tinker::class_def<CX2Main>( L, "SetMyUnitSP",					&CX2Main::SetMyUnitSP );
	lua_tinker::class_def<CX2Main>( L, "SendStatisticsLocLog",			&CX2Main::SendStatisticsLocLog );
	lua_tinker::class_def<CX2Main>( L, "SendStatisticsDBLog",			&CX2Main::SendStatisticsDBLog );
	lua_tinker::class_def<CX2Main>( L, "IntentionalCrash",				&CX2Main::IntentionalCrash );
	lua_tinker::class_def<CX2Main>( L, "SetFreeCamera",					&CX2Main::SetFreeCamera );
#ifdef KEYFRAME_CAMERA
	lua_tinker::class_def<CX2Main>( L, "SetWorldCameraEdit",			&CX2Main::SetWorldCameraEdit );
	lua_tinker::class_def<CX2Main>( L, "PlayWorldCamera",				&CX2Main::PlayWorldCamera );
	lua_tinker::class_def<CX2Main>( L, "EndWorldCamera",				&CX2Main::EndWorldCamera );
	lua_tinker::class_def<CX2Main>( L, "SetProjectionFov",				&CX2Main::SetProjectionFov );
#endif KEYFRAME_CAMERA

	//{{ 2008. 1. 2  ������  �ټ��� ġƮ
	lua_tinker::class_def<CX2Main>( L, "ChangeSpirit",					&CX2Main::ChangeSpirit );
	lua_tinker::class_def<CX2Main>( L, "ChangeVSPoint",					&CX2Main::ChangeVSPoint );
	//}}
	lua_tinker::class_def<CX2Main>( L, "SetShowUnitViewerUI",			&CX2Main::SetShowUnitViewerUI );
	//{{ 2008. 2. 18  ������  ���� Ŭ���� ġƮ
	lua_tinker::class_def<CX2Main>( L, "ChangeUnitClass",				&CX2Main::ChangeUnitClass );
	lua_tinker::class_def<CX2Main>( L, "ModifyChangeUnitClass",			&CX2Main::ModifyChangeUnitClass );
	//}}

	// kimhc // 2011-05-30 // �׳�ý� �Ⱓ ���� ġƮ
	lua_tinker::class_def<CX2Main>( L, "ChangeCashSkillPointDate",		&CX2Main::ChangeCashSkillPointDate );

	lua_tinker::class_def<CX2Main>( L, "SetSquareMaxUserNum",			&CX2Main::SetSquareMaxUserNum );
	//{{ 2008. 10. 24  ������	Ÿ��Ʋ ġƮ
	lua_tinker::class_def<CX2Main>( L, "GetTitleCheat",					&CX2Main::GetTitleCheat );
	//}}
	//{{ 09.07.03 �¿� : �系���� ���̵� ���� �ٽ� �����ϰ�
#if !defined(_SERVICE_) || defined (SERV_JOIN_IN_CLIENT_FOR_TW_TEST_SERVER)
	lua_tinker::class_def<CX2Main>( L, "SetGameServerPortForCreateID",			&CX2Main::SetGameServerPortForCreateID );
	lua_tinker::class_def<CX2Main>( L, "AddGameServerIPForCreateID_LUA",		&CX2Main::AddGameServerIPForCreateID_LUA );
#endif //!defined(_SERVICE_) || defined (SERV_JOIN_IN_CLIENT_FOR_TW_TEST_SERVER)
	lua_tinker::class_def<CX2Main>( L, "SetEnabledDeveloperCheatCode",			&CX2Main::SetEnabledDeveloperCheatCode );

	//{{ 2009. 7. 26  ������	party fever cheat
	lua_tinker::class_def<CX2Main>( L, "IncreasePartyFeverPoint",				&CX2Main::IncreasePartyFeverPoint );
	//}}
	//{{ 2009. 8. 5  ������		ü��IDġƮ
	lua_tinker::class_def<CX2Main>( L, "SetGuestUserMode",						&CX2Main::SetGuestUserMode );
	//}}

	//{{ ����� : [2010/3/12/] //	���� �̵� ���� ġƮ
	lua_tinker::class_def<CX2Main>( L, "WarpByCommand",							&CX2Main::WarpByCommand );
	//}} ����� : [2010/3/12/] //	
	
#ifdef CASH_CHARGE_URL_JP
	lua_tinker::class_def<CX2Main>( L, "MSCashChargeURL_LUA",		&CX2Main::MSCashChargeURL_LUA );
	lua_tinker::class_def<CX2Main>( L, "NHNCashChargeURL_LUA",		&CX2Main::NHNCashChargeURL_LUA );
	lua_tinker::class_def<CX2Main>( L, "NICOCashChargeURL_LUA",		&CX2Main::NICOCashChargeURL_LUA );
	lua_tinker::class_def<CX2Main>( L, "NEXONCashChargeURL_LUA",		&CX2Main::NEXONCashChargeURL_LUA );
#endif 

#ifdef SERV_ADMIN_CHEAT_GET_ALL_SKILL 
	lua_tinker::class_def<CX2Main>( L, "FullMyUnitSkillTree",					&CX2Main::FullMyUnitSkillTree );
#endif //SERV_ADMIN_CHEAT_GET_ALL_SKILL 

#if defined(PRINT_INGAMEINFO_TO_EXCEL) && defined( _IN_HOUSE_ )
	lua_tinker::class_def<CX2Main>( L, "SetInGameInfoToExcel",					&CX2Main::SetInGameInfoToExcel );
#endif

#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
	lua_tinker::class_def<CX2Main>( L, "FullMyUnitSkillTreeByLevel",			&CX2Main::FullMyUnitSkillTreeByLevel );
	lua_tinker::class_def<CX2Main>( L, "SetMyUnitSkillLevel",					&CX2Main::SetMyUnitSkillLevel );
#endif // UPGRADE_SKILL_SYSTEM_2013

#ifdef BATTLE_FIELD_WARP_EDIT
	lua_tinker::class_def<CX2Main>( L, "FieldWarpByCommand",					&CX2Main::FieldWarpByCommand );
#endif //BATTLE_FIELD_WARP_EDIT


#ifdef NEXON_QA_CHEAT_REQ
	lua_tinker::class_def<CX2Main>( L, "SetNoCoolTime",							&CX2Main::SetNoCoolTime );
	lua_tinker::class_def<CX2Main>( L, "InventoryClear",						&CX2Main::InventoryClear );
#endif //NEXON_QA_CHEAT_REQ


#ifdef ACTIVE_KOG_GAME_PERFORMANCE_CHECK
	lua_tinker::class_def<CX2Main>( L, "OccurLag_LUA",					&CX2Main::OccurLag_LUA );
#endif//ACTIVE_KOG_GAME_PERFORMANCE_CHECK

#ifdef ACTIVE_KOG_GAME_PERFORMANCE_CHECK
	lua_tinker::class_def<CX2Main>( L, "ForceHost_LUA",				&CX2Main::ForceHost_LUA );
	lua_tinker::class_def<CX2Main>( L, "SwitchConnect_LUA",		&CX2Main::SwitchConnect_LUA );
#endif//ACTIVE_KOG_GAME_PERFORMANCE_CHECK

#ifdef UDP_PACKET_ANALYSIS_LOG
	lua_tinker::class_def<CX2Main>( L, "UdpPacketAnalysisLog_LUA",		&CX2Main::UdpPacketAnalysisLog_LUA );
#endif//UDP_PACKET_ANALYSIS_LOG

#ifdef X2OPTIMIZE_UDP_RELAY_OVERHEAD_TEST
	lua_tinker::class_def<CX2Main>( L, "UdpRelayOverHeadTest_LUA",		&CX2Main::UdpRelayOverHeadTest_LUA );
#endif//X2OPTIMIZE_UDP_RELAY_OVERHEAD_TEST

#ifdef SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
	lua_tinker::class_def<CX2Main>( L, "UdpPacketOverlapTest_LUA",		&CX2Main::UdpPacketOverlapTest_LUA );
#endif//SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK

#ifdef X2OPTIMIZE_ONE_SIMUL_ONE_RENDER_TEST
	lua_tinker::class_def<CX2Main>( L, "OneSimulOneRenderTest_LUA",		&CX2Main::	OneSimulOneRenderTest_LUA );
#endif//X2OPTIMIZE_ONE_SIMUL_ONE_RENDER_TEST

//#ifdef  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
    lua_tinker::class_def<CX2Main>( L, "SetUDPMode",       &CX2Main::SetUDPMode_LUA );
//#endif  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK


	lua_tinker::class_add<CX2QuestManager>( L, "CX2QuestManager" );
	lua_tinker::class_def<CX2QuestManager>( L, "AddQuestTemplet_LUA",			&CX2QuestManager::AddQuestTemplet_LUA );
	lua_tinker::class_def<CX2QuestManager>( L, "AddSubQuestTemplet_LUA",		&CX2QuestManager::AddSubQuestTemplet_LUA );
#ifdef SERV_ACCOUNT_MISSION_SYSTEM
	//������//2012-12-17// ���� ���� ����Ʈ�� �Ϲ� ����Ʈ�� �и� �ϰ� ���� �߰��� �������̽�
						// ����� ������ ���� ������ AddQuestTemplet_LUA �׳� ������, ���� ���� ���� ����Ʈ���� ���� ������ �߰��Ǹ� ���� �ʿ��մϴ�.
	lua_tinker::class_def<CX2QuestManager>( L, "AddAccountQuestTemplet_LUA",			&CX2QuestManager::AddQuestTemplet_LUA );
#endif //SERV_ACCOUNT_MISSION_SYSTEM

#ifdef LUA_TRANS_DEVIDE
	lua_tinker::class_def<CX2QuestManager>( L, "AddQuestTempletTrans_LUA",			&CX2QuestManager::AddQuestTempletTrans_LUA );
	lua_tinker::class_def<CX2QuestManager>( L, "AddSubQuestTempletTrans_LUA",		&CX2QuestManager::AddSubQuestTempletTrans_LUA );
#endif LUA_TRANS_DEVIDE

#ifdef DAY_OF_THE_WEEK_QUEST
	lua_tinker::class_def<CX2QuestManager>( L, "AddAccountQuestDate",					&CX2QuestManager::AddAccountQuestDate_LUA );
#endif //DAY_OF_THE_WEEK_QUEST

#ifdef	X2OPTIMIZE_ITEM_TEMPLET_PREPROCESSING
	lua_tinker::class_add<CX2ItemManager::KProxy>( L, "CX2ItemManagerProxy" );
	lua_tinker::class_def<CX2ItemManager::KProxy>( L, "AddItemTemplet",				&CX2ItemManager::KProxy::AddItemTemplet_LUA );
	lua_tinker::class_def<CX2ItemManager::KProxy>( L, "AddSetItemData_LUA",				&CX2ItemManager::KProxy::AddSetItemData_LUA );
	lua_tinker::class_add<CX2ItemManager::KProxy2>( L, "CX2ItemManagerProxy2" );
	lua_tinker::class_def<CX2ItemManager::KProxy2>( L, "AddItemTemplet",			&CX2ItemManager::KProxy2::AddItemTemplet_LUA );
	lua_tinker::class_def<CX2ItemManager::KProxy2>( L, "AddSetItemData_LUA",		&CX2ItemManager::KProxy2::AddSetItemData_LUA );

#endif	//X2OPTIMIZE_ITEM_TEMPLET_PREPROCESSING

	lua_tinker::class_add<CX2ItemManager>( L, "CX2ItemManager" );
#ifndef	X2OPTIMIZE_ITEM_TEMPLET_PREPROCESSING
	lua_tinker::class_def<CX2ItemManager>( L, "AddItemTemplet",				&CX2ItemManager::AddItemTemplet_LUA );
#endif	//X2OPTIMIZE_ITEM_TEMPLET_PREPROCESSING
	lua_tinker::class_def<CX2ItemManager>( L, "AddShopItemList_LUA",		&CX2ItemManager::AddShopItemList_LUA );
	lua_tinker::class_def<CX2ItemManager>( L, "AddShopItemGroup_LUA",		&CX2ItemManager::AddShopItemGroup_LUA );
	lua_tinker::class_def<CX2ItemManager>( L, "AddRandomItemTemplet",		&CX2ItemManager::AddRandomItemTemplet_LUA );
	lua_tinker::class_def<CX2ItemManager>( L, "AddManufactureTemplet",		&CX2ItemManager::AddManufactureTemplet_LUA );
	lua_tinker::class_def<CX2ItemManager>( L, "AddManufactureVillageData",	&CX2ItemManager::AddManufactureVillageData_LUA );
	lua_tinker::class_def<CX2ItemManager>( L, "AddManufactureResultGroup",	&CX2ItemManager::AddManufactureResultGroup );

#ifdef PET_PREVIEW
	lua_tinker::class_def<CX2ItemManager>( L, "AddPetPreViewData",			&CX2ItemManager::AddPetPreViewData_LUA);
#endif // PET_PREVIEW

//{{ 2013.01.26 �ֹ�ö ť����Ű�������� �̸�������
#ifdef PACKAGE_IN_QUBE_PREVIEW
	lua_tinker::class_def<CX2ItemManager>(  L, "AddCubePackageData",			&CX2ItemManager::AddCube_PackageData_LUA );
#endif PACKAGE_IN_QUBE_PREVIEW
//}}
#ifdef SERV_ITEM_LUA_TRANS_DEVIDE
	lua_tinker::class_def<CX2ItemManager>( L, "AddItemTempletTrans",		&CX2ItemManager::AddItemTempletTrans_LUA );
#endif SERV_ITEM_LUA_TRANS_DEVIDE
#ifdef LUA_TRANS_DEVIDE
	lua_tinker::class_def<CX2ItemManager>( L, "AddSetItemDataTrans_LUA",	&CX2ItemManager::AddSetItemDataTrans_LUA);
#endif LUA_TRANS_DEVIDE
#ifdef IN_ITEM_KIM_USE_HIDE_SET_DESC
	lua_tinker::class_def<CX2ItemManager>( L, "AddHideSetDesc",				&CX2ItemManager::AddHideSetDesc_LUA);
#endif //IN_ITEM_KIM_USE_HIDE_SET_DESC

	//{{ 2008. 4. 24  ������  
	lua_tinker::class_def<CX2ItemManager>( L, "AddPackageItemData",			&CX2ItemManager::AddPackageItemData_LUA);
	//}}		
#ifndef	X2OPTIMIZE_ITEM_TEMPLET_PREPROCESSING
	lua_tinker::class_def<CX2ItemManager>( L, "AddSetItemData_LUA",			&CX2ItemManager::AddSetItemData_LUA);
#endif  //X2OPTIMIZE_ITEM_TEMPLET_PREPROCESSING

	//{{ 2008. 11. 18  ������
	lua_tinker::class_def<CX2ItemManager>( L, "AddItemExchangeGroup",		&CX2ItemManager::AddItemExchangeGroup_LUA);	
	//}}
#ifdef NEW_ITEM_EXCHANGE_TEST
	lua_tinker::class_def<CX2ItemManager>( L, "AddItemExchangeData",		&CX2ItemManager::AddItemExchangeData );	
#endif NEW_ITEM_EXCHANGE_TEST

#ifdef COOLTIME_SHARE_GROUP_ITEM
	lua_tinker::class_def<CX2ItemManager>( L, "AddCoolTimeGroupItem",		&CX2ItemManager::AddCoolTimeGroupItem_LUA );	
#endif COOLTIME_SHARE_GROUP_ITEM

	//{{ 2011. 10. 18	������	��Ʋ�ʵ� ������ ���� �ý���
#ifdef SERV_BATTLE_FIELD_ITEM_LEVEL
	lua_tinker::class_def<CX2ItemManager>( L, "SetItemLevelEnchantData",	&CX2ItemManager::SetItemLevelEnchantData_LUA );
#endif SERV_BATTLE_FIELD_ITEM_LEVEL
	//}}
#ifdef ICE_HEATER_PRE_VIEW
	lua_tinker::class_def<CX2ItemManager>( L, "AddIceHeaterItemData",	&CX2ItemManager::AddIceHeaterItemData_LUA);
#endif //ICE_HEATER_PRE_VIEW
	

#ifdef SERV_NEW_ITEM_SYSTEM_2013_05	
	lua_tinker::class_def<CX2ItemManager>( L, "AddItemConvertGroup", &CX2ItemManager::AddItemConvertGroup_LUA);
	lua_tinker::class_def<CX2ItemManager>( L, "AddItemConvertInfo",	&CX2ItemManager::AddItemConvertInfo_LUA);
#endif // SERV_NEW_ITEM_SYSTEM_2013_05


	//{{ 2009. 6. 24  ������	�������̺�
	lua_tinker::class_add<CX2RewardTable>( L, "CX2RewardTable" );
	lua_tinker::class_def<CX2RewardTable>( L, "AddRewardData",			&CX2RewardTable::AddRewardData_LUA );
	//}}
	//{{ 2011. 06. 22    ��μ�    ��Ż ���� �� - NEXON ���� �ý��� ����
#ifdef SERV_NEXON_SESSION_PROTOCOL
	lua_tinker::class_def<CX2RewardTable>( L, "SetOutUserRetainingRewardItemInfo",	&CX2RewardTable::SetOutUserRetainingRewardItemInfo_LUA );
#endif SERV_NEXON_SESSION_PROTOCOL
	//}} 
	//{{ 2011. 10.26    ��μ�	ĳ���� ������ �̺�Ʈ
#ifdef SERV_CHAR_LEVEL_UP_EVENT
	lua_tinker::class_def<CX2RewardTable>( L, "AddLevelUpRewardData",	&CX2RewardTable::AddLevelUpRewardData_LUA );
#endif SERV_CHAR_LEVEL_UP_EVENT
	//}}

	lua_tinker::class_add<CX2UnitManager>( L, "CX2UnitManager" );
#ifdef USE_DIFFERENT_SOUND_WHEN_IN_SPECIAL
	lua_tinker::class_def<CX2UnitManager>( L, "AddCharSoundChange",		&CX2UnitManager::AddCharSoundChange_LUA );
#endif //USE_DIFFERENT_SOUND_WHEN_IN_SPECIAL
	lua_tinker::class_def<CX2UnitManager>( L, "AddUnitTemplet",			&CX2UnitManager::AddUnitTemplet_LUA );
	lua_tinker::class_def<CX2UnitManager>( L, "AddNPCTemplet",			&CX2UnitManager::AddNPCTemplet_LUA );
	lua_tinker::class_def<CX2UnitManager>( L, "AddNPCStat",				&CX2UnitManager::AddNPCStat_LUA );
	lua_tinker::class_def<CX2UnitManager>( L, "AddNPCExtraAbility_LUA", &CX2UnitManager::AddNPCExtraAbility_LUA );


	lua_tinker::class_add<CX2UIUnitManager>( L, "CX2UIUnitManager" );
	lua_tinker::class_def<CX2UIUnitManager>( L, "ClearAllUIUnit",		&CX2UIUnitManager::ClearAllUIUnit );


	lua_tinker::class_add<CX2WorldManager>( L, "CX2WorldManager" );
	lua_tinker::class_def<CX2WorldManager>( L, "AddWorldData",			&CX2WorldManager::AddWorldData_LUA );
	//{{ seojt // 2008-10-21, 15:20
	lua_tinker::class_def<CX2WorldManager>( L, "SetPreprocessingData",  &CX2WorldManager::SetPreprocessingData_LUA );
	//}} seojt // 2008-10-21, 15:20


	lua_tinker::class_add<CX2World>( L, "CX2World" );
	lua_tinker::class_def<CX2World>( L, "SetOriginColor_LUA",			&CX2World::SetOriginColor_LUA );
	lua_tinker::class_def<CX2World>( L, "SetLightColor_LUA",			&CX2World::SetLightColor_LUA );
	lua_tinker::class_def<CX2World>( L, "SetLightPos",					&CX2World::SetLightPos );
	lua_tinker::class_def<CX2World>( L, "SetLineMap",					&CX2World::SetLineMap_LUA );
	lua_tinker::class_def<CX2World>( L, "SetBGM",						&CX2World::SetBGM_LUA );
	lua_tinker::class_def<CX2World>( L, "PlayBGM",						&CX2World::PlayBGM );
	//{{08.12.15.hoons.
	lua_tinker::class_def<CX2World>( L, "StopBGM",						&CX2World::StopBGM );
	//}}
	lua_tinker::class_def<CX2World>( L, "AddEffectBGM_LUA",				&CX2World::AddEffectBGM_LUA );
	lua_tinker::class_def<CX2World>( L, "Add3DEffectBGM_LUA",			&CX2World::Add3DEffectBGM_LUA );

	lua_tinker::class_def<CX2World>( L, "CreateSkyDome",				&CX2World::CreateSkyDome );
	lua_tinker::class_def<CX2World>( L, "CreateObjectMesh",				&CX2World::CreateObjectMesh );
	lua_tinker::class_def<CX2World>( L, "CreateLensFlare",				&CX2World::CreateLensFlare );
	lua_tinker::class_def<CX2World>( L, "CreateObjectSkinMesh",			&CX2World::CreateObjectSkinMesh );
	lua_tinker::class_def<CX2World>( L, "CreateObjectLightFlow",		&CX2World::CreateObjectLightFlow );
	lua_tinker::class_def<CX2World>( L, "CreateObjectParticle",			&CX2World::CreateObjectParticle );
	lua_tinker::class_def<CX2World>( L, "CreateObjectParticleDelay",	&CX2World::CreateObjectParticleDelay );	
    lua_tinker::class_def<CX2World>( L, "AddSyncObject",			    &CX2World::AddSyncObject );
    
#ifdef WORLD_LAYER
    lua_tinker::class_def<CX2World>( L, "CreateWorldLayer",				&CX2World::CreateWorldLayer );    
#endif
	lua_tinker::class_def<CX2World>( L, "SetWorldColor",				&CX2World::SetWorldColor );
	lua_tinker::class_def<CX2World>( L, "GetRenderLineMap",				&CX2World::GetRenderLineMap );
	lua_tinker::class_def<CX2World>( L, "SetRenderLineMap",				&CX2World::SetRenderLineMap );
#ifdef FOG_WORLD
    lua_tinker::class_def<CX2World>( L, "SetFogWorld",				    &CX2World::SetFogWorld );
#endif	
#ifdef BUBBLE_BOBBLE_TEST
	lua_tinker::class_def<CX2World>( L, "SetLookAt",				    &CX2World::SetLookAt );
	lua_tinker::class_def<CX2World>( L, "SetEye",						&CX2World::SetEye );
#endif BUBBLE_BOBBLE_TEST
	lua_tinker::class_def<CX2World>( L, "SetSquareCameraSetting",		&CX2World::SetSquareCameraSetting );
	lua_tinker::class_def<CX2World>( L, "SetSquareCameraType",			&CX2World::SetSquareCameraType );
#ifdef DEEP_WATER_DUNGEON_TEST
	lua_tinker::class_def<CX2World>( L, "SetGravityAndSpeedRate",		&CX2World::SetGravityAndSpeedRate );
	lua_tinker::class_def<CX2World>( L, "SetUnitAnimSpeedRate",			&CX2World::SetUnitAnimSpeedRate );
#endif DEEP_WATER_DUNGEON_TEST
//#ifdef MARIO_LIKE_BLOCK_TEST
	lua_tinker::class_def<CX2World>( L, "AddWorldMonster_LUA",			&CX2World::AddWorldMonster_LUA );
//#endif MARIO_LIKE_BLOCK_TEST

	// oasis907 : ����� [2010.12.13] // ���� �Ľ� ���� ������ ���� ����
	lua_tinker::class_def<CX2World>( L, "SetWorldCamera",				&CX2World::SetWorldCamera_LUA );
#ifdef MODIFY_FRUSTUM
	lua_tinker::class_def<CX2World>( L, "SetProjection",				&CX2World::SetProjection );	
#endif
	lua_tinker::class_def<CX2World>( L, "SetTriggerCamera",				&CX2World::SetTriggerCamera );	

	lua_tinker::class_add<CX2WorldObject>( L, "CX2WorldObject" );
	lua_tinker::class_inh<CX2WorldObject, CKTDGObject>( L );
	lua_tinker::class_def<CX2WorldObject>( L, "SetName",					&CX2WorldObject::SetName );
	lua_tinker::class_def<CX2WorldObject>( L, "SetPos",						&CX2WorldObject::SetPos );
	lua_tinker::class_def<CX2WorldObject>( L, "SetAngleDegree",				&CX2WorldObject::SetAngleDegree );
	lua_tinker::class_def<CX2WorldObject>( L, "SetColorFactor",				&CX2WorldObject::SetColorFactor );
	lua_tinker::class_def<CX2WorldObject>( L, "SetScale",					&CX2WorldObject::SetScale );		
	lua_tinker::class_def<CX2WorldObject>( L, "SetUseWorldColor",			&CX2WorldObject::SetUseWorldColor );	
	lua_tinker::class_def<CX2WorldObject>( L, "SetHideDistance",			&CX2WorldObject::SetHideDistance );
	lua_tinker::class_def<CX2WorldObject>( L, "SetHideNearDistance",		&CX2WorldObject::SetHideNearDistance );	
	lua_tinker::class_def<CX2WorldObject>( L, "SetForceShow",				&CX2WorldObject::SetForceShow );

#ifdef COLORFACTOR_FOR_EVERY_WORLDMESH
	lua_tinker::class_def<CX2WorldObject>( L, "SetOriginalColorFactor",		&CX2WorldObject::SetOriginalColorFactor );
#endif



#ifdef WORLD_LAYER
    lua_tinker::class_add<CX2WorldLayer>( L, "CX2WorldLayer" );
    lua_tinker::class_inh<CX2WorldLayer, CX2WorldObject>(L);
    lua_tinker::class_def<CX2WorldLayer>( L, "SetTex",						&CX2WorldLayer::SetTex );
    lua_tinker::class_def<CX2WorldLayer>( L, "SetColor",					&CX2WorldLayer::SetColor );
    lua_tinker::class_def<CX2WorldLayer>( L, "SetPos",						&CX2WorldLayer::SetPos );
    lua_tinker::class_def<CX2WorldLayer>( L, "SetBlendType",				&CX2WorldLayer::SetBlendType );  
    lua_tinker::class_def<CX2WorldLayer>( L, "SetTexUV",    				&CX2WorldLayer::SetTexUV );   
    lua_tinker::class_def<CX2WorldLayer>( L, "SetFeedBackUV",    			&CX2WorldLayer::SetFeedBackUV ); 
	lua_tinker::class_def<CX2WorldLayer>( L, "SetLayerId",    				&CX2WorldLayer::SetLayerId );
	lua_tinker::class_def<CX2WorldLayer>( L, "SetInitShow",    				&CX2WorldLayer::SetInitShow );
#endif

	lua_tinker::class_add<CX2WorldObjectMesh>( L, "CX2WorldObjectMesh" );
	lua_tinker::class_inh<CX2WorldObjectMesh, CX2WorldObject>( L );
	lua_tinker::class_def<CX2WorldObjectMesh>( L, "SetXMesh",				&CX2WorldObjectMesh::SetXMesh_LUA );
	lua_tinker::class_def<CX2WorldObjectMesh>( L, "SetXMeshLOD",			&CX2WorldObjectMesh::SetXMeshLOD_LUA );
	lua_tinker::class_def<CX2WorldObjectMesh>( L, "SetMultiTexXET",			&CX2WorldObjectMesh::SetMultiTexXET_LUA );
	lua_tinker::class_def<CX2WorldObjectMesh>( L, "SetChangeTexXET",		&CX2WorldObjectMesh::SetChangeTexXET_LUA );
	lua_tinker::class_def<CX2WorldObjectMesh>( L, "SetCanHide",				&CX2WorldObjectMesh::SetCanHide );
	lua_tinker::class_def<CX2WorldObjectMesh>( L, "SetNotRenderWhenHidden",	&CX2WorldObjectMesh::SetNotRenderWhenHidden );
	lua_tinker::class_def<CX2WorldObjectMesh>( L, "SetBillboardType",		&CX2WorldObjectMesh::SetBillboardType );
	lua_tinker::class_def<CX2WorldObjectMesh>( L, "GetRenderParam",			&CX2WorldObjectMesh::GetRenderParam );
	lua_tinker::class_def<CX2WorldObjectMesh>( L, "SetTex0UVSpeed",			&CX2WorldObjectMesh::SetTex0UVSpeed );
	lua_tinker::class_def<CX2WorldObjectMesh>( L, "SetTex1UVSpeed",			&CX2WorldObjectMesh::SetTex1UVSpeed );
	lua_tinker::class_def<CX2WorldObjectMesh>( L, "SetTex2UVSpeed",			&CX2WorldObjectMesh::SetTex2UVSpeed );
#ifdef FOG_WORLD
    lua_tinker::class_def<CX2WorldObjectMesh>( L, "SetFog",			        &CX2WorldObjectMesh::SetFog );
#endif    
	

	lua_tinker::class_add<CX2WorldObjectSkinMesh>( L, "CX2WorldObjectSkinMesh" );
	lua_tinker::class_inh<CX2WorldObjectSkinMesh, CX2WorldObject>( L );
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "PlayAnim",				&CX2WorldObjectSkinMesh::PlayAnim_LUA );
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "AddAnim",				&CX2WorldObjectSkinMesh::AddAnim_LUA );
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "SetAnimSpeed",			&CX2WorldObjectSkinMesh::SetAnimSpeed );
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "SetAnimAniXET",			&CX2WorldObjectSkinMesh::SetAnimAniXET_LUA );
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "SetXSkinMotion",			&CX2WorldObjectSkinMesh::SetXSkinMotion_LUA );
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "SetMeshAniXET",			&CX2WorldObjectSkinMesh::SetMeshAniXET_LUA );
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "SetMultiTexXET",			&CX2WorldObjectSkinMesh::SetMultiTexXET_LUA );
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "SetChangeTexXET",		&CX2WorldObjectSkinMesh::SetChangeTexXET_LUA );
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "AddXSkinModel",			&CX2WorldObjectSkinMesh::AddXSkinModel_LUA );
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "SetCanHide",				&CX2WorldObjectSkinMesh::SetCanHide );
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "SetBillboardType",		&CX2WorldObjectSkinMesh::SetBillboardType );
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "GetRenderParam",			&CX2WorldObjectSkinMesh::GetRenderParam );
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "SetOccasionalEventTime",	&CX2WorldObjectSkinMesh::SetOccasionalEventTime );
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "SetOccasionalPlayRate",	&CX2WorldObjectSkinMesh::SetOccasionalPlayRate );
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "SetTex0UVSpeed",			&CX2WorldObjectSkinMesh::SetTex0UVSpeed );
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "SetTex1UVSpeed",			&CX2WorldObjectSkinMesh::SetTex1UVSpeed );
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "SetTex2UVSpeed",			&CX2WorldObjectSkinMesh::SetTex2UVSpeed );
#ifdef FOG_WORLD
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "SetFog",			        &CX2WorldObjectSkinMesh::SetFog );
#endif
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "SetCartoonRender",		&CX2WorldObjectSkinMesh::SetCartoonRender );
	lua_tinker::class_def<CX2WorldObjectSkinMesh>( L, "PlayLocalInterval",		&CX2WorldObjectSkinMesh::SetAnimPlayLocalInterval );

	lua_tinker::class_add<CX2WorldObjectLightFlow>( L, "CX2WorldObjectLightFlow" );
	lua_tinker::class_inh<CX2WorldObjectLightFlow, CX2WorldObject>( L );
	lua_tinker::class_def<CX2WorldObjectLightFlow>( L, "SetXMesh",				&CX2WorldObjectLightFlow::SetXMesh_LUA );
	lua_tinker::class_def<CX2WorldObjectLightFlow>( L, "SetFlowMin",			&CX2WorldObjectLightFlow::SetFlowMin );
	lua_tinker::class_def<CX2WorldObjectLightFlow>( L, "SetFlowMax",			&CX2WorldObjectLightFlow::SetFlowMax );
	lua_tinker::class_def<CX2WorldObjectLightFlow>( L, "SetFlowSpeed",			&CX2WorldObjectLightFlow::SetFlowSpeed );
	lua_tinker::class_def<CX2WorldObjectLightFlow>( L, "SetWide",				&CX2WorldObjectLightFlow::SetWide );
	lua_tinker::class_def<CX2WorldObjectLightFlow>( L, "SetNowPoint",			&CX2WorldObjectLightFlow::SetNowPoint );
	lua_tinker::class_def<CX2WorldObjectLightFlow>( L, "SetRenderType",			&CX2WorldObjectLightFlow::SetRenderType );
	lua_tinker::class_def<CX2WorldObjectLightFlow>( L, "GetRenderParam",		&CX2WorldObjectLightFlow::GetRenderParam );
	

	lua_tinker::class_add<CX2WorldObjectParticle>( L, "CX2WorldObjectParticle" );
	lua_tinker::class_inh<CX2WorldObjectParticle, CX2WorldObject>( L );
	lua_tinker::class_def<CX2WorldObjectParticle>( L, "SetCanHide",				&CX2WorldObjectParticle::SetCanHide );
	lua_tinker::class_def<CX2WorldObjectParticle>( L, "SetParticlePos",			&CX2WorldObjectParticle::SetParticlePos );
	lua_tinker::class_def<CX2WorldObjectParticle>( L, "SetEmitMinMax",			&CX2WorldObjectParticle::SetEmitMinMax );
	lua_tinker::class_def<CX2WorldObjectParticle>( L, "SetTriggerCount",		&CX2WorldObjectParticle::SetTriggerCount );
	lua_tinker::class_def<CX2WorldObjectParticle>( L, "SetTriggerTime",			&CX2WorldObjectParticle::SetTriggerTime );
	lua_tinker::class_def<CX2WorldObjectParticle>( L, "SetDrawCount",			&CX2WorldObjectParticle::SetDrawCount );
	lua_tinker::class_def<CX2WorldObjectParticle>( L, "SetCullCheck",			&CX2WorldObjectParticle::SetCullCheck );
	lua_tinker::class_def<CX2WorldObjectParticle>( L, "SetParticleType",		&CX2WorldObjectParticle::SetParticleType );
	

	lua_tinker::class_add<CX2Dungeon>( L, "CX2Dungeon" );
	lua_tinker::class_def<CX2Dungeon>( L, "GetNowStage",						&CX2Dungeon::GetNowStage );
	

	lua_tinker::class_add<CX2DungeonStage>( L, "CX2DungeonStage" );
	lua_tinker::class_def<CX2DungeonStage>( L, "GetNowSubStage",				&CX2DungeonStage::GetNowSubStage );
	lua_tinker::class_def<CX2DungeonStage>( L, "StartGoEffect",					&CX2DungeonStage::StartGoEffect );
	lua_tinker::class_def<CX2DungeonStage>( L, "StopGoEffect",					&CX2DungeonStage::StopGoEffect );
	lua_tinker::class_def<CX2DungeonStage>( L, "IsGoEffectOn",					&CX2DungeonStage::IsGoEffectOn );
	

	lua_tinker::class_add<CX2DungeonSubStage>( L, "CX2DungeonSubStage" );
	lua_tinker::class_def<CX2DungeonSubStage>( L, "ClearSubStage",				&CX2DungeonSubStage::ClearSubStage );
	lua_tinker::class_def<CX2DungeonSubStage>( L, "GetWinWhenTimeOut",			&CX2DungeonSubStage::GetWinWhenTimeOut );
	lua_tinker::class_def<CX2DungeonSubStage>( L, "SetWinWhenTimeOut",			&CX2DungeonSubStage::SetWinWhenTimeOut );
	lua_tinker::class_def<CX2DungeonSubStage>( L, "GetStarted",					&CX2DungeonSubStage::GetStarted );
	lua_tinker::class_def<CX2DungeonSubStage>( L, "GetCheckedClearSubStage",	&CX2DungeonSubStage::GetCheckedClearSubStage );
	lua_tinker::class_def<CX2DungeonSubStage>( L, "GetDoneClearSubStage",		&CX2DungeonSubStage::GetDoneClearSubStage );



	lua_tinker::class_add<CX2DungeonManager>( L, "CX2DungeonManager" );
	lua_tinker::class_def<CX2DungeonManager>( L, "AddDungeonData",				&CX2DungeonManager::AddDungeonData_LUA );
	lua_tinker::class_def<CX2DungeonManager>( L, "AddDefaultRoomTitle_LUA",		&CX2DungeonManager::AddDefaultRoomTitle_LUA );



	

	lua_tinker::class_add<CX2Camera>( L, "CX2Camera" );
	lua_tinker::class_def<CX2Camera>( L, "NomalTrackingCamera_LUA",				&CX2Camera::NomalTrackingCamera_LUA );
	lua_tinker::class_def<CX2Camera>( L, "PartsLookTrackingCamera_LUA",			&CX2Camera::PartsLookTrackingCamera_LUA );
	lua_tinker::class_def<CX2Camera>( L, "NomalDirectCamera_LUA",				&CX2Camera::NomalDirectCamera_LUA );
	lua_tinker::class_def<CX2Camera>( L, "PartsLookDirectCamera_LUA",			&CX2Camera::PartsLookDirectCamera_LUA );
#ifdef NPC_TRACKINGCAMERA
	lua_tinker::class_def<CX2Camera>( L, "PartsLookTrackingCameraByNpc_LUA",	&CX2Camera::PartsLookTrackingCameraByNpc_LUA );
#endif
	lua_tinker::class_def<CX2Camera>( L, "GetCamera",							&CX2Camera::GetCamera );
	
#ifdef KEYFRAME_CAMERA
	lua_tinker::class_def<CX2Camera>( L, "AddWorldCameraData_LUA",				&CX2Camera::AddWorldCameraData_LUA );
#endif KEYFRAME_CAMERA

	lua_tinker::class_add<CX2Game>( L, "CX2Game" );
#ifdef ADDITIONAL_MEMO
	lua_tinker::class_def<CX2Game>( L, "GetGameType",							&CX2Game::GetGameType );
#endif
	lua_tinker::class_def<CX2Game>( L, "IsHost",								&CX2Game::IsHost );
	lua_tinker::class_def<CX2Game>( L, "GetMajorXMeshPlayer",					&CX2Game::GetMajorXMeshPlayer );
	lua_tinker::class_def<CX2Game>( L, "GetMinorXMeshPlayer",					&CX2Game::GetMinorXMeshPlayer );
	lua_tinker::class_def<CX2Game>( L, "GetMajorParticle",						&CX2Game::GetMajorParticle );
	lua_tinker::class_def<CX2Game>( L, "GetMinorParticle",						&CX2Game::GetMinorParticle );
	lua_tinker::class_def<CX2Game>( L, "GetDamageEffect",						&CX2Game::GetDamageEffect );
	lua_tinker::class_def<CX2Game>( L, "GetEffectSet",							&CX2Game::GetEffectSet );
	lua_tinker::class_def<CX2Game>( L, "GetX2Camera",							&CX2Game::GetX2Camera );
	lua_tinker::class_def<CX2Game>( L, "GetFocusUnitPos_LUA",					&CX2Game::GetFocusUnitPos_LUA );
	lua_tinker::class_def<CX2Game>( L, "LiveUserUnitNum",						&CX2Game::LiveUserUnitNum );
	lua_tinker::class_def<CX2Game>( L, "LiveActiveNPCNum",						&CX2Game::LiveActiveNPCNum );
	lua_tinker::class_def<CX2Game>( L, "LiveNPCNumType_LUA",					&CX2Game::LiveNPCNumType_LUA );
	lua_tinker::class_def<CX2Game>( L, "IsKillNPCKeyCode_LUA",					&CX2Game::IsKillNPCKeyCode_LUA );
	lua_tinker::class_def<CX2Game>( L, "LiveKeyCodeNPCNum",						&CX2Game::LiveKeyCodeNPCNum );
	lua_tinker::class_def<CX2Game>( L, "GetKeyCodeNPC",							&CX2Game::GetKeyCodeNPC );
	lua_tinker::class_def<CX2Game>( L, "SetWorldColor_LUA",						&CX2Game::SetWorldColor_LUA );
	lua_tinker::class_def<CX2Game>( L, "GetLineMap",							&CX2Game::GetLineMap );
	lua_tinker::class_def<CX2Game>( L, "CreateNPCReq_LUA",						&CX2Game::CreateNPCReq_LUA );
	lua_tinker::class_def<CX2Game>( L, "CreateNPCReq_LUA2",						&CX2Game::CreateNPCReq_LUA2 );
#ifdef SERV_INSERT_GLOBAL_SERVER
	lua_tinker::class_def<CX2Game>( L, "CreateNPCReqElite",						&CX2Game::CreateNPCReqElite );
#endif SERV_INSERT_GLOBAL_SERVER
#ifdef SEASON3_MONSTER_2011_7
	lua_tinker::class_def<CX2Game>( L, "CreateNPCReqAlly",						&CX2Game::CreateNPCReqAlly );
#endif SEASON3_MONSTER_2011_7
	lua_tinker::class_def<CX2Game>( L, "PushCreateNPCReq_Lua",					&CX2Game::PushCreateNPCReq_Lua );
	lua_tinker::class_def<CX2Game>( L, "FlushCreateNPCReq",						&CX2Game::FlushCreateNPCReq );
#ifdef CREATE_NPC_REQ_FULL_ARGUMENTS
	lua_tinker::class_def<CX2Game>( L, "CreateNPCReq_LUA3",						&CX2Game::CreateNPCReq_LUA3 );
	lua_tinker::class_def<CX2Game>( L, "PushCreateNPCReq_LUA3",					&CX2Game::PushCreateNPCReq_LUA3 );
#endif CREATE_NPC_REQ_FULL_ARGUMENTS
	lua_tinker::class_def<CX2Game>( L, "GetMyUnit",								&CX2Game::GetMyUnit );
	lua_tinker::class_def<CX2Game>( L, "IsKeyPressed",							&CX2Game::IsKeyPressed );
	lua_tinker::class_def<CX2Game>( L, "IsSkipKeyPressed",						&CX2Game::IsSkipKeyPressed );
	lua_tinker::class_def<CX2Game>( L, "GetGameTime",							&CX2Game::GetGameTime );
	lua_tinker::class_def<CX2Game>( L, "GetElapsedTimeAfterLastKeyboardInput",	&CX2Game::GetElapsedTimeAfterLastKeyboardInput );
	lua_tinker::class_def<CX2Game>( L, "PauseNPCAI",							&CX2Game::PauseNPCAI );
	lua_tinker::class_def<CX2Game>( L, "KillAllNPC_LUA",						&CX2Game::KillAllNPC_LUA );
	lua_tinker::class_def<CX2Game>( L, "KillNPC",								&CX2Game::KillNPC );
	lua_tinker::class_def<CX2Game>( L, "EnableAllNPCAI",						&CX2Game::EnableAllNPCAI );
	lua_tinker::class_def<CX2Game>( L, "SetFreeCamera",							&CX2Game::SetFreeCamera);
	lua_tinker::class_def<CX2Game>( L, "GoStage",								&CX2Game::GoStage);
	lua_tinker::class_def<CX2Game>( L, "GetWorld",								&CX2Game::GetWorld);
	lua_tinker::class_def<CX2Game>( L, "SetShowAllUnitGageBar",					&CX2Game::SetShowAllUnitGageBar);
	lua_tinker::class_def<CX2Game>( L, "GetUnitNum",							&CX2Game::GetUnitNum);
	lua_tinker::class_def<CX2Game>( L, "GetUnit",								&CX2Game::GetUnit);
	lua_tinker::class_def<CX2Game>( L, "GetUserUnitListSize",					&CX2Game::GetUserUnitListSize);
	lua_tinker::class_def<CX2Game>( L, "GetUserUnit",							&CX2Game::GetUserUnit);
	lua_tinker::class_def<CX2Game>( L, "GetUserUnitByUID",						&CX2Game::GetUserUnitByUID);    
	lua_tinker::class_def<CX2Game>( L, "GetNPCUnitListSize",					&CX2Game::GetNPCUnitListSize);
	lua_tinker::class_def<CX2Game>( L, "GetNPCUnit",							&CX2Game::GetNPCUnit);
    lua_tinker::class_def<CX2Game>( L, "GetNPCUnitByUID",						&CX2Game::GetNPCUnitByUID);   
	lua_tinker::class_def<CX2Game>( L, "GetNPCUnitByKeyCode",					&CX2Game::GetNPCUnitByKeyCode);
	lua_tinker::class_def<CX2Game>( L, "GetNPCUnitByType",						&CX2Game::GetNPCUnitByType);	
	lua_tinker::class_def<CX2Game>( L, "GetGameLevel",							&CX2Game::GetGameLevel);
	lua_tinker::class_def<CX2Game>( L, "GetDungeonType",						&CX2Game::GetDungeonType );	
	lua_tinker::class_def<CX2Game>( L, "GetSlideShot",							&CX2Game::GetSlideShot);
	lua_tinker::class_def<CX2Game>( L, "SetEnableAllNPCConditionTable",			&CX2Game::SetEnableAllNPCConditionTable);
	lua_tinker::class_def<CX2Game>( L, "EnableAllNPCEventProcess",				&CX2Game::EnableAllNPCEventProcess);
	lua_tinker::class_def<CX2Game>( L, "SetEnableAllKeyProcess",				&CX2Game::SetEnableAllKeyProcess);
	lua_tinker::class_def<CX2Game>( L, "SetEnableCommandKeyProcess",			&CX2Game::SetEnableCommandKeyProcess);
	lua_tinker::class_def<CX2Game>( L, "GetDropItemManager",					&CX2Game::GetDropItemManager);
	lua_tinker::class_def<CX2Game>( L, "GetNPCStartPos",						&CX2Game::GetNPCStartPos);
	lua_tinker::class_def<CX2Game>( L, "AnyUserUnitInRange",					&CX2Game::AnyUserUnitInRange);
	lua_tinker::class_def<CX2Game>( L, "GetNearNPCUnitUIDInStart_LUA",			&CX2Game::GetNearNPCUnitUIDInStart_LUA);
	lua_tinker::class_def<CX2Game>( L, "GetNearNPCUnitInStart_LUA",				&CX2Game::GetNearNPCUnitInStart );
	lua_tinker::class_def<CX2Game>( L, "GetNearNPCUnitUID_LUA",	        		&CX2Game::GetNearNPCUnitUID_LUA);
	lua_tinker::class_def<CX2Game>( L, "GetNearestNpcInSpecificRangeByNpcId_LUA",	&CX2Game::GetNearestNpcInSpecificRangeByNpcId);

	lua_tinker::class_def<CX2Game>( L, "GetNearestUserUnit_LUA",	       		&CX2Game::GetNearestUserUnit );

//{{oasis:�����////2009-10-7////
	lua_tinker::class_def<CX2Game>( L, "IsNearNPCUnitUID_LUA",					&CX2Game::IsNearNPCUnitUID_LUA);
	lua_tinker::class_def<CX2Game>( L, "GetNearestUserUnitUID_LUA",				&CX2Game::GetNearestUserUnitUID_LUA );
	lua_tinker::class_def<CX2Game>( L, "GetFixedDistancePosition_LUA",			&CX2Game::GetFixedDistancePosition_LUA );
	lua_tinker::class_def<CX2Game>( L, "GetDist_LUA",							&CX2Game::GetDist_LUA );
	lua_tinker::class_def<CX2Game>( L, "GetNearestUserUnitPos_LUA",				&CX2Game::GetNearestUserUnitPos_LUA );
//}}oasis:�����////2009-10-7////
	lua_tinker::class_def<CX2Game>( L, "GetNearestNPCUnitPos_LUA",				&CX2Game::GetNearestNPCUnitPos_LUA );
#ifdef VELDER_BOSS_5	
	//{{ oasis907 : ����� [2010.7.30] // 
	lua_tinker::class_def<CX2Game>( L, "GetFarthestUserUnitPos_LUA",			&CX2Game::GetFarthestUserUnitPos_LUA );
	lua_tinker::class_def<CX2Game>( L, "GetUserUnitNum_LUA",					&CX2Game::GetUserUnitNum );
	//}}
#endif VELDER_BOSS_5
	lua_tinker::class_def<CX2Game>( L, "ChangeRebirthStateNPCUnit_LUA",			&CX2Game::ChangeRebirthStateNPCUnit_LUA);
	lua_tinker::class_def<CX2Game>( L, "GetEmptyStartPos",						&CX2Game::GetEmptyStartPos);
	lua_tinker::class_def<CX2Game>( L, "SetAllNPCMPFull",						&CX2Game::SetAllNPCMPFull);
	lua_tinker::class_def<CX2Game>( L, "SetLastKillCheck",						&CX2Game::SetLastKillCheck);
	lua_tinker::class_def<CX2Game>( L, "SetLastkillShotTime",					&CX2Game::SetLastkillShotTime);
	lua_tinker::class_def<CX2Game>( L, "BuffAllNPC",							&CX2Game::BuffAllNPC);
	lua_tinker::class_def<CX2Game>( L, "GetEnableMonsterFly",					&CX2Game::GetEnableMonsterFly);

#ifdef SERV_CREATE_ELITE_FOR_ADMIN
	lua_tinker::class_def<CX2Game>( L, "Send_EGS_ADMIN_NPC_UNIT_CREATE_REQ_LUA",	&CX2Game::Send_EGS_ADMIN_NPC_UNIT_CREATE_REQ_LUA );
#endif SERV_CREATE_ELITE_FOR_ADMIN

#ifdef IN_GAME_MANUAL_CAMERA_POSITION_TEST
	lua_tinker::class_def<CX2Game>( L, "EnableManualCameraPosition",			&CX2Game::EnableManualCameraPosition);
#endif IN_GAME_MANUAL_CAMERA_POSITION_TEST

	// oasis907 : ����� [2010.12.13] // ���� �Ľ� ���� ������ ���� ����
	lua_tinker::class_def<CX2Game>( L, "PlayWorldCamera",						&CX2Game::PlayWorldCamera );

#ifdef FIX_LINK_ATTACK
	lua_tinker::class_def<CX2Game>( L, "GetNearestUserUnitInSpecificRange_LUA",		&CX2Game::GetNearestUserUnitInSpecificRange );
#endif
	//{{ ����� : [2011/3/4/] //	���� �̼�
#ifdef SERV_INSERT_GLOBAL_SERVER
	lua_tinker::class_def<CX2Game>( L, "GetNPCUnitNum",							&CX2Game::GetNPCUnitNum );
	lua_tinker::class_def<CX2Game>( L, "GetNPCUnitNumByNPCID",					&CX2Game::GetNPCUnitNumByNPCID );
	lua_tinker::class_def<CX2Game>( L, "rand",									&CX2Game::GetRandomValue );

	lua_tinker::class_def<CX2Game>( L, "GetDungeonProgress",					&CX2Game::GetDungeonProgress );
#endif SERV_INSERT_GLOBAL_SERVER
	//}} ����� : [2011/3/4/] //	���� �̼�

#ifdef SERV_INSERT_GLOBAL_SERVER	// oasis907 : ����� [2011.4.19]
	lua_tinker::class_def<CX2Game>( L, "GetEliteKeyCodeNPCUnitNum",				&CX2Game::GetEliteKeyCodeNPCUnitNum );
#endif SERV_INSERT_GLOBAL_SERVER

#ifdef SEASON3_MONSTER_2011_7
	lua_tinker::class_def<CX2Game>( L, "GetFarthestUserUnitUID_LUA",			&CX2Game::GetFarthestUserUnitUID_LUA );
	lua_tinker::class_def<CX2Game>( L, "AddReverseGravityRegion",				&CX2Game::AddReverseGravityRegion );
#endif SEASON3_MONSTER_2011_7

#ifdef NEW_HENIR_TEST
	lua_tinker::class_def<CX2Game>( L, "GetStartSecretStageEnteringEvent",		&CX2Game::GetStartSecretStageEnteringEvent );
	lua_tinker::class_def<CX2Game>( L, "SecretDungeonStageLoadReq",				&CX2Game::SecretDungeonStageLoadReq );
#endif NEW_HENIR_TEST

#ifdef SECRET_VELDER
	lua_tinker::class_def<CX2Game>( L, "GetNowStageIndex",	&CX2Game::GetNowStageIndex);
#endif
#ifdef DUNGEON_ALARM_SYSTEM
	lua_tinker::class_def<CX2Game>( L, "ShowDangerAlarm_LUA",	&CX2Game::ShowDangerAlarm_LUA);	
#endif
#ifdef ADDITIONAL_MEMO
	lua_tinker::class_def<CX2Game>( L, "GetNearestUnit_LUA",	&CX2Game::GetNearestUnit);	
#endif

#ifdef SERV_DUNGEON_FORCED_EXIT_SYSTEM
	lua_tinker::class_def<CX2Game>( L, "Send_EGS_DEFENCE_DUNGEON_WAVE_ID_NOT",	&CX2Game::Send_EGS_DEFENCE_DUNGEON_WAVE_ID_NOT);	
#endif //SERV_DUNGEON_FORCED_EXIT_SYSTEM

#ifdef SERV_EVENT_VALENTINE_DUNGEON
	lua_tinker::class_def<CX2Game>( L, "NotfiyCreateValentimeCupCake",			&CX2Game::NotfiyCreateValentimeCupCake);	
#endif //SERV_EVENT_VALENTINE_DUNGEON

#ifdef SERV_CODE_EVENT
	lua_tinker::class_def<CX2Game>( L, "IsEnableCodeEvent_LUA",		&CX2Game::IsEnableCodeEvent_LUA );
#endif SERV_CODE_EVENT

#ifdef GET_DUNGEON_ID_IN_LUA
	lua_tinker::class_def<CX2Game>( L, "GetDungeonID",		&CX2Game::GetDungeonID );
#endif GET_DUNGEON_ID_IN_LUA

#ifdef NEXON_QA_CHEAT_REQ
	lua_tinker::class_def<CX2Game>( L, "AddHyperModeCount",			&CX2Game::AddHyperModeCount);
	lua_tinker::class_def<CX2Game>( L, "AddAraGage",				&CX2Game::AddAraGage);
	lua_tinker::class_def<CX2Game>( L, "AddChungCannonBall",		&CX2Game::AddChungCannonBall);
	lua_tinker::class_def<CX2Game>( L, "AddChungExtraCannonBall",	&CX2Game::AddChungExtraCannonBall);
#endif //NEXON_QA_CHEAT_REQ


#ifdef SERV_NEW_DEFENCE_DUNGEON // ���볯¥: 2013-04-02

	/// ���� ���� ���� �ش� Ű�ڵ带 ���� NPC �˻�
	lua_tinker::class_def<CX2Game>( L, "LiveKeyCodeNPCNumFreeTeam",	&CX2Game::LiveKeyCodeNPCNumFreeTeam );

#endif //SERV_NEW_DEFENCE_DUNGEON

#ifdef NEAREST_NPC_ON_SAME_DIRECTION
	lua_tinker::class_def<CX2Game>( L, "GetNearestNPCUnitPosOnSameDirection_LUA",	&CX2Game::GetNearestNPCUnitPosOnSameDirection_LUA );
	lua_tinker::class_def<CX2Game>( L, "GetNearestNPCUnitOnSameDirection_LUA",		&CX2Game::GetNearestNPCUnitOnSameDirection_LUA );
#endif //NEAREST_NPC_ON_SAME_DIRECTION


	lua_tinker::class_add<CX2DungeonGame>( L, "CX2DungeonGame" );
	lua_tinker::class_inh<CX2DungeonGame, CX2Game>( L );
	lua_tinker::class_def<CX2DungeonGame>( L, "TextOutTutorial",					&CX2DungeonGame::TextOutTutorial_LUA );
	lua_tinker::class_def<CX2DungeonGame>( L, "GetIsThereTutorialMessage",			&CX2DungeonGame::GetIsThereTutorialMessage );
	lua_tinker::class_def<CX2DungeonGame>( L, "IsTutorialMessage",					&CX2DungeonGame::IsTutorialMessage_LUA );
	lua_tinker::class_def<CX2DungeonGame>( L, "GetDungeon",							&CX2DungeonGame::GetDungeon );
	lua_tinker::class_def<CX2DungeonGame>( L, "GetGameLevel",						&CX2DungeonGame::GetGameLevel );
	lua_tinker::class_def<CX2DungeonGame>( L, "GetDungeonType",						&CX2DungeonGame::GetDungeonType );	
	lua_tinker::class_def<CX2DungeonGame>( L, "ClearSubStage",						&CX2DungeonGame::ClearSubStage );
	lua_tinker::class_def<CX2DungeonGame>( L, "GetStartRealTraining",				&CX2DungeonGame::GetStartRealTraining );
	lua_tinker::class_def<CX2DungeonGame>( L, "SetStartRealTraining",				&CX2DungeonGame::SetStartRealTraining );
	lua_tinker::class_def<CX2DungeonGame>( L, "GetRemainPlayTime",					&CX2DungeonGame::GetRemainPlayTime );
	lua_tinker::class_def<CX2DungeonGame>( L, "GetTrainingGameUI",					&CX2DungeonGame::GetTrainingGameUI );
	lua_tinker::class_def<CX2DungeonGame>( L, "SetRemainPlayTime",					&CX2DungeonGame::SetRemainPlayTime );
	lua_tinker::class_def<CX2DungeonGame>( L, "ShowBossName_LUA",					&CX2DungeonGame::ShowBossName_LUA );
	lua_tinker::class_def<CX2DungeonGame>( L, "ResetUnitPositionAtTrainingGame",	&CX2DungeonGame::ResetUnitPositionAtTrainingGame );
	lua_tinker::class_def<CX2DungeonGame>( L, "GetTimerManyPurpose1",				&CX2DungeonGame::GetTimerManyPurpose1 );
	lua_tinker::class_def<CX2DungeonGame>( L, "GetTimerManyPurpose2",				&CX2DungeonGame::GetTimerManyPurpose2 );
	lua_tinker::class_def<CX2DungeonGame>( L, "SetClearEffect",						&CX2DungeonGame::SetClearEffect );

	
	
	lua_tinker::class_add<CX2SquareGame>( L, "CX2SquareGame" );
	lua_tinker::class_def<CX2SquareGame>( L, "SetFreeCamera",						&CX2SquareGame::SetFreeCamera );
	

	lua_tinker::class_add<CX2GameUnit>( L, "CX2GameUnit" );
	lua_tinker::class_def<CX2GameUnit>( L, "PlaySound_LUA",				&CX2GameUnit::PlaySound_LUA );
	lua_tinker::class_def<CX2GameUnit>( L, "StopSound_LUA",				&CX2GameUnit::StopSound_LUA );
	lua_tinker::class_def<CX2GameUnit>( L, "GetPos",					&CX2GameUnit::GetPos );
	lua_tinker::class_def<CX2GameUnit>( L, "ClearHitUnitList_LUA",		&CX2GameUnit::ClearHitUnitList_LUA );
	lua_tinker::class_def<CX2GameUnit>( L, "SetSpeedX",					&CX2GameUnit::SetSpeedX );
	lua_tinker::class_def<CX2GameUnit>( L, "SetSpeedY",					&CX2GameUnit::SetSpeedY );
	lua_tinker::class_def<CX2GameUnit>( L, "SetDamageData_LUA",			&CX2GameUnit::SetDamageData_LUA );
	lua_tinker::class_def<CX2GameUnit>( L, "GetNowHP",					&CX2GameUnit::GetNowHp );
	lua_tinker::class_def<CX2GameUnit>( L, "GetMaxHP",					&CX2GameUnit::GetMaxHp );
	lua_tinker::class_def<CX2GameUnit>( L, "SetNowHP",					&CX2GameUnit::SetNowHp );
	lua_tinker::class_def<CX2GameUnit>( L, "GetNowMP",					&CX2GameUnit::GetNowMp );
	lua_tinker::class_def<CX2GameUnit>( L, "SetNowMP",					&CX2GameUnit::SetNowMp );
	lua_tinker::class_def<CX2GameUnit>( L, "GetMaxMP",					&CX2GameUnit::GetMaxMp );
	lua_tinker::class_def<CX2GameUnit>( L, "SetColor",					&CX2GameUnit::SetColor );
	lua_tinker::class_def<CX2GameUnit>( L, "GetColor",					&CX2GameUnit::GetColor );
	lua_tinker::class_def<CX2GameUnit>( L, "GetImpactPoint_LUA",		&CX2GameUnit::GetImpactPoint_LUA );
	lua_tinker::class_def<CX2GameUnit>( L, "GetAttackerUnit_LUA",		&CX2GameUnit::GetAttackerUnit_LUA );
	lua_tinker::class_def<CX2GameUnit>( L, "GetDistanceFrom",			&CX2GameUnit::GetDistanceFrom );
	lua_tinker::class_def<CX2GameUnit>( L, "GetXDistanceFrom",			&CX2GameUnit::GetXDistanceFrom );
	lua_tinker::class_def<CX2GameUnit>( L, "GetWalkSpeed",				&CX2GameUnit::GetWalkSpeed );
	lua_tinker::class_def<CX2GameUnit>( L, "SetShowGage",				&CX2GameUnit::SetShowGage );
	lua_tinker::class_def<CX2GameUnit>( L, "SetCanNotInputTime_LUA",	&CX2GameUnit::SetCanNotInputTime_LUA );
	lua_tinker::class_def<CX2GameUnit>( L, "SetUseWorldColor",			&CX2GameUnit::SetUseWorldColor );		
	lua_tinker::class_def<CX2GameUnit>( L, "GetDirVector",				&CX2GameUnit::GetDirVector );
	lua_tinker::class_def<CX2GameUnit>( L, "GetTeam",					&CX2GameUnit::GetTeam );
	lua_tinker::class_def<CX2GameUnit>( L, "MultiplyStat",				&CX2GameUnit::MultiplyStat );
	lua_tinker::class_def<CX2GameUnit>( L, "SetDamageReducePercent",	&CX2GameUnit::SetDamageReducePercent );
	//lua_tinker::class_def<CX2GameUnit>( L, "TwinkleMPGageBar",			&CX2GameUnit::TwinkleMPGageBar );
	lua_tinker::class_def<CX2GameUnit>( L, "SetMajorMeshPlayer_LUA",	&CX2GameUnit::SetMajorMeshPlayer_LUA );
	lua_tinker::class_def<CX2GameUnit>( L, "ClearMajorMeshPlayer_LUA",	&CX2GameUnit::ClearMajorMeshPlayer_LUA );
	lua_tinker::class_def<CX2GameUnit>( L, "GetMajorMeshPlayer_LUA",	&CX2GameUnit::GetMajorMeshPlayer_LUA );
	lua_tinker::class_def<CX2GameUnit>( L, "GetMajorMeshPlayerHandle",	&CX2GameUnit::GetMajorMeshPlayerHandle );
	lua_tinker::class_def<CX2GameUnit>( L, "SetMinorMeshPlayer_LUA",	&CX2GameUnit::SetMinorMeshPlayer_LUA );
	lua_tinker::class_def<CX2GameUnit>( L, "ClearMinorMeshPlayer_LUA",	&CX2GameUnit::ClearMinorMeshPlayer_LUA );
	lua_tinker::class_def<CX2GameUnit>( L, "GetMinorMeshPlayer_LUA",	&CX2GameUnit::GetMinorMeshPlayer_LUA );
	lua_tinker::class_def<CX2GameUnit>( L, "GetMinorMeshPlayerHandle",	&CX2GameUnit::GetMinorMeshPlayerHandle );
	lua_tinker::class_def<CX2GameUnit>( L, "AddOnStatReq",				&CX2GameUnit::AddOnStatReq );
	lua_tinker::class_def<CX2GameUnit>( L, "GetWeaponBonePos_LUA",		&CX2GameUnit::GetWeaponBonePos_LUA);
	lua_tinker::class_def<CX2GameUnit>( L, "SetWeaponEnable",			&CX2GameUnit::SetWeaponEnable);
	lua_tinker::class_def<CX2GameUnit>( L, "AttachWeaponParticle_LUA",	&CX2GameUnit::AttachWeaponParticle_LUA);
	lua_tinker::class_def<CX2GameUnit>( L, "GetExtraDamagePack",		&CX2GameUnit::GetExtraDamagePack);
	lua_tinker::class_def<CX2GameUnit>( L, "SetExtraDamagePack",		&CX2GameUnit::SetExtraDamagePack);
	lua_tinker::class_def<CX2GameUnit>( L, "GetGameUnitType",			&CX2GameUnit::GetGameUnitType);
	lua_tinker::class_def<CX2GameUnit>( L, "SetExtraDamageDataTime",	&CX2GameUnit::SetExtraDamageDataTime);
	lua_tinker::class_def<CX2GameUnit>( L, "DeleteEffectSetOnDie",		&CX2GameUnit::DeleteEffectSetOnDie);
	lua_tinker::class_def<CX2GameUnit>( L, "SetLockOnNearTarget_LUA",	&CX2GameUnit::SetLockOnNearTarget_LUA);
	//{{ JHKang / ������ / 2010/12/07 / �밢�� ���� ����Ʈ ũ�� ���� �� ����� ���� ����
#ifdef NEW_SKILL_2010_11
#ifdef UPGRADE_SKILL_SYSTEM_2013 //JHKang
	lua_tinker::class_def<CX2GameUnit>( L, "GetScaleByUnit",			&CX2GameUnit::GetVec3ScaleByUnit);
#else //UPGRADE_SKILL_SYSTEM_2013
#ifdef INT_SKILL_BUG_FIX
	lua_tinker::class_def<CX2GameUnit>( L, "GetScaleByUnit",			&CX2GameUnit::GetVec3ScaleByUnit);
#else
	lua_tinker::class_def<CX2GameUnit>( L, "GetScaleByUnit",			&CX2GameUnit::GetScaleByUnit);
#endif INT_SKILL_BUG_FIX
#endif //UPGRADE_SKILL_SYSTEM_2013
	lua_tinker::class_def<CX2GameUnit>( L, "SetShieldDamageReduce",		&CX2GameUnit::SetShieldDamageReduce);
#endif NEW_SKILL_2010_11
	//}} JHKang / ������ / 2010/12/07 / �밢�� ���� ����Ʈ ũ�� ���� �� ����� ���� ����

#ifdef NEW_CHARACTER_CHUNG
	lua_tinker::class_def<CX2GameUnit>( L, "SetRemainHyperModeTime",	&CX2GameUnit::SetRemainHyperModeTime);
#endif NEW_CHARACTER_CHUNG
#ifdef VELDER_SECRET_DUNGEON
	lua_tinker::class_def<CX2GameUnit>( L, "GetTimeToReturnBasicScale_LUA",		&CX2GameUnit::GetTimeToReturnBasicScale);	
	
#endif

#ifdef SECRET_VELDER
	lua_tinker::class_def<CX2GameUnit>( L, "PulseWeaponRenderEffect",	&CX2GameUnit::PulseWeaponRenderEffect);
	lua_tinker::class_def<CX2GameUnit>( L, "SetEnableWeaponRenderEffect",	&CX2GameUnit::SetEnableWeaponRenderEffect);
#endif

#ifdef	SERV_TRAPPING_RANGER_TEST
	lua_tinker::class_def<CX2GameUnit>( L, "GetAllyTeam",				&CX2GameUnit::GetAllyTeam );
	lua_tinker::class_def<CX2GameUnit>( L, "GetUnitUID",				&CX2GameUnit::GetUnitUID );
#endif	SERV_TRAPPING_RANGER_TEST

	lua_tinker::class_def<CX2GameUnit>( L, "SetBuffFactorToGameUnitByBuffFactorID_LUA",	
		&CX2GameUnit::SetBuffFactorToGameUnitByBuffFactorID );

#ifdef ADD_LIVE_CREATOR_FINALIZER
	lua_tinker::class_def<CX2GameUnit>( L, "SetBuffFactorToGameUnitByBuffFactorIDAndCreatorID_LUA",	
		&CX2GameUnit::SetBuffFactorToGameUnitByBuffFactorIDAndCreatorID );
#endif ADD_LIVE_CREATOR_FINALIZER

	lua_tinker::class_def<CX2GameUnit>( L, "EraseBuffTempletFromGameUnit_LUA",	
		&CX2GameUnit::EraseBuffTempletFromGameUnit );

	lua_tinker::class_def<CX2GameUnit>( L, "SetBuffFactorToDamageData",	
		&CX2GameUnit::SetBuffFactorToDamageData );

	lua_tinker::class_def<CX2GameUnit>( L, "TransformScale_LUA",			&CX2GameUnit::TransformScale_LUA );	
	lua_tinker::class_def<CX2GameUnit>( L, "IsChangedGameUnitScaleByBuff",	&CX2GameUnit::IsChangedGameUnitScaleByBuff );

#ifdef POSSIBLE_ADD_CHANGE_SCALE_BUFF_VALUE
	lua_tinker::class_def<CX2GameUnit>( L, "GetChangeUnitScaleByBuff",	&CX2GameUnit::GetChangeUnitScaleByBuff );
#endif POSSIBLE_ADD_CHANGE_SCALE_BUFF_VALUE

#ifdef SERV_NEW_DEFENCE_DUNGEON // ���볯¥: 2013-04-12
	lua_tinker::class_def<CX2GameUnit>( L, "WriteTraceLog_LUA",			&CX2GameUnit::WriteTraceLog_LUA );

	lua_tinker::class_def<CX2GameUnit>( L, "ShowIncreaseValue",			&CX2GameUnit::ShowIncreaseValue );
#endif // SERV_NEW_DEFENCE_DUNGEON

#ifdef SERV_BATTLEFIELD_MIDDLE_BOSS
	lua_tinker::class_def<CX2GameUnit>( L, "HaveThisBuff",				&CX2GameUnit::HaveThisBuff );
#endif // SERV_BATTLEFIELD_MIDDLE_BOSS

#ifdef USER_LEVEL_IN_LUA
	lua_tinker::class_def<CX2GameUnit>( L, "GetUnitLevel",					&CX2GameUnit::GetUnitLevel );
#endif USER_LEVEL_IN_LUA

	//{{ GUNPC Start
	lua_tinker::class_add<CX2GUNPC>( L, "CX2GUNPC" );
	lua_tinker::class_inh<CX2GUNPC, CX2GameUnit>( L );
	lua_tinker::class_def<CX2GUNPC>( L, "AnimEventTimer_LUA",			&CX2GUNPC::AnimEventTimer_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "AnimTimer_LUA",				&CX2GUNPC::AnimTimer_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "GetNowAnimTime_LUA",			&CX2GUNPC::GetNowAnimTime_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "EventTimer",					&CX2GUNPC::EventTimer);
	lua_tinker::class_def<CX2GUNPC>( L, "GetLandPosition_LUA",			&CX2GUNPC::GetLandPosition_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "SetPosition",					&CX2GUNPC::SetPosition);
	lua_tinker::class_def<CX2GUNPC>( L, "GetPos",						&CX2GUNPC::GetPos);
	lua_tinker::class_def<CX2GUNPC>( L, "GetRotateDegree",				&CX2GUNPC::GetRotateDegree);
	lua_tinker::class_def<CX2GUNPC>( L, "GetXRotateDegree",				&CX2GUNPC::GetXRotateDegree);
	lua_tinker::class_def<CX2GUNPC>( L, "GetYRotateDegree",				&CX2GUNPC::GetYRotateDegree);
	lua_tinker::class_def<CX2GUNPC>( L, "GetZRotateDegree",				&CX2GUNPC::GetZRotateDegree);
#ifdef  X2OPTIMIZE_ROBUST_NPC_ROTATION
    lua_tinker::class_def<CX2GUNPC>( L, "SetIsRight",					&CX2GUNPC::SetIsRightNow);        
#else   X2OPTIMIZE_ROBUST_NPC_ROTATION
	lua_tinker::class_def<CX2GUNPC>( L, "SetIsRight",					&CX2GUNPC::SetIsRight);        
#endif  X2OPTIMIZE_ROBUST_NPC_ROTATION
	lua_tinker::class_def<CX2GUNPC>( L, "GetIsRight",					&CX2GUNPC::GetIsRight);
	lua_tinker::class_def<CX2GUNPC>( L, "GetDirPos_LUA",				&CX2GUNPC::GetDirPos_LUA);        
	lua_tinker::class_def<CX2GUNPC>( L, "SetAddRotateDegree_LUA",		&CX2GUNPC::SetAddRotateDegree_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "GetXSkinAnim",					&CX2GUNPC::GetXSkinAnim_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "GetBonePos_LUA",				&CX2GUNPC::GetBonePos_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "GetUnitCondition_bHit",		&CX2GUNPC::GetUnitCondition_bHit);
	lua_tinker::class_def<CX2GUNPC>( L, "SetKeyCode",					&CX2GUNPC::SetKeyCode);
	lua_tinker::class_def<CX2GUNPC>( L, "GetKeyCode",					&CX2GUNPC::GetKeyCode);
	lua_tinker::class_def<CX2GUNPC>( L, "SetFlag_LUA",					&CX2GUNPC::SetFlag_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "GetFlag_LUA",					&CX2GUNPC::GetFlag_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "SetInt_LUA",					&CX2GUNPC::SetInt_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "GetInt_LUA",					&CX2GUNPC::GetInt_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "SetVector_LUA",				&CX2GUNPC::SetVector_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "GetVector_LUA",				&CX2GUNPC::GetVector_LUA);
//#ifdef	X2OPTIMIZE_NPC_LUASPACE_SHARING
	lua_tinker::class_def<CX2GUNPC>( L, "SetNumber_LUA",				&CX2GUNPC::SetNumber_LUA );
	lua_tinker::class_def<CX2GUNPC>( L, "GetNumber_LUA",				&CX2GUNPC::GetNumber_LUA );
//#endif	X2OPTIMIZE_NPC_LUASPACE_SHARING
	lua_tinker::class_def<CX2GUNPC>( L, "SetAIEnable",					&CX2GUNPC::SetAIEnable);
	lua_tinker::class_def<CX2GUNPC>( L, "SetTimePauseNPCAI",			&CX2GUNPC::SetTimePauseNPCAI);	
	lua_tinker::class_def<CX2GUNPC>( L, "SetMajorParticle_LUA",			&CX2GUNPC::SetMajorParticle_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "SetMinorParticle_LUA",			&CX2GUNPC::SetMinorParticle_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "GetMajorParticle_LUA",			&CX2GUNPC::GetMajorParticle_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "GetMinorParticle_LUA",			&CX2GUNPC::GetMinorParticle_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "ClearMinorParticle_LUA",		&CX2GUNPC::ClearMinorParticle_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "ClearMajorParticle_LUA",		&CX2GUNPC::ClearMajorParticle_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "GetDamageEffect_LUA",			&CX2GUNPC::GetDamageEffect_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "SetDamageEffect_LUA",			&CX2GUNPC::SetDamageEffect_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "ClearDamageEffect_LUA",		&CX2GUNPC::ClearDamageEffect_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "SetInvincibleTime",			&CX2GUNPC::SetInvincibleTime);
	lua_tinker::class_def<CX2GUNPC>( L, "SetSuperArmor",				&CX2GUNPC::SetSuperArmorStateParam);
	lua_tinker::class_def<CX2GUNPC>( L, "CallOtherNPC",					&CX2GUNPC::CallOtherNPC);
	lua_tinker::class_def<CX2GUNPC>( L, "GetNowAnimationTime",			&CX2GUNPC::GetNowAnimationTime);
	lua_tinker::class_def<CX2GUNPC>( L, "SetPassiveSpeed",				&CX2GUNPC::SetPassiveSpeed);
	lua_tinker::class_def<CX2GUNPC>( L, "SetSpeedX",					&CX2GUNPC::SetSpeedX);
	lua_tinker::class_def<CX2GUNPC>( L, "SetSpeedY",					&CX2GUNPC::SetSpeedY);
	lua_tinker::class_def<CX2GUNPC>( L, "GetSpeedY",					&CX2GUNPC::GetSpeedY);
#ifdef SEASON3_MONSTER_2011_7
	lua_tinker::class_def<CX2GUNPC>( L, "GetSpeedX",					&CX2GUNPC::GetSpeedX);
	lua_tinker::class_def<CX2GUNPC>( L, "SetGAccel",					&CX2GUNPC::SetGAccel);
	lua_tinker::class_def<CX2GUNPC>( L, "SetReloadAccel",				&CX2GUNPC::SetReloadAccel);
#endif SEASON3_MONSTER_2011_7
	lua_tinker::class_def<CX2GUNPC>( L, "GetDamagedThisFrame",			&CX2GUNPC::GetDamagedThisFrame);
	lua_tinker::class_def<CX2GUNPC>( L, "HasTarget",					&CX2GUNPC::HasTarget);
	lua_tinker::class_def<CX2GUNPC>( L, "SetDieSeq",					&CX2GUNPC::SetDieSeq);
	lua_tinker::class_def<CX2GUNPC>( L, "GetHardLevel",					&CX2GUNPC::GetHardLevel);		
	lua_tinker::class_def<CX2GUNPC>( L, "GetBoneRotateDegree_LUA",		&CX2GUNPC::GetBoneRotateDegree_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "SetBoneScale_LUA",				&CX2GUNPC::SetBoneScale_LUA);	
	lua_tinker::class_def<CX2GUNPC>( L, "GetHyperModeCount",			&CX2GUNPC::GetHyperModeCount);
	lua_tinker::class_def<CX2GUNPC>( L, "ChangeHyperMode_LUA",			&CX2GUNPC::ChangeHyperMode_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "SetLockOnTarget",				&CX2GUNPC::SetLockOnTarget);
	lua_tinker::class_def<CX2GUNPC>( L, "SetLockOnNearTarget",			&CX2GUNPC::SetLockOnNearTarget);	
	lua_tinker::class_def<CX2GUNPC>( L, "SetLockOn",					&CX2GUNPC::SetLockOn);
	lua_tinker::class_def<CX2GUNPC>( L, "SetLockOnPos",					&CX2GUNPC::SetLockOnPos);
	lua_tinker::class_def<CX2GUNPC>( L, "SetSiegeMode",					&CX2GUNPC::SetSiegeMode);
	lua_tinker::class_def<CX2GUNPC>( L, "ReleaseSiegeMode",				&CX2GUNPC::ReleaseSiegeMode);
	lua_tinker::class_def<CX2GUNPC>( L, "GetUID",						&CX2GUNPC::GetUID);
	lua_tinker::class_def<CX2GUNPC>( L, "ClearEventCheck",				&CX2GUNPC::ClearEventCheck);
	lua_tinker::class_def<CX2GUNPC>( L, "GetFollowHitter",				&CX2GUNPC::GetFollowHitter);
	lua_tinker::class_def<CX2GUNPC>( L, "GetFollowHitterLost",			&CX2GUNPC::GetFollowHitterLost);
	lua_tinker::class_def<CX2GUNPC>( L, "GetLineGroupStartPos",			&CX2GUNPC::GetLineGroupStartPos);
	lua_tinker::class_def<CX2GUNPC>( L, "GetLineGroupEndPos",			&CX2GUNPC::GetLineGroupEndPos);
	lua_tinker::class_def<CX2GUNPC>( L, "GetStateTime",					&CX2GUNPC::GetStateTime);
	lua_tinker::class_def<CX2GUNPC>( L, "GetZVector",					&CX2GUNPC::GetZVector);
	lua_tinker::class_def<CX2GUNPC>( L, "GetNowHP_LUA",					&CX2GUNPC::GetNowHP_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "SetNowHP_LUA",					&CX2GUNPC::SetNowHP_LUA);
//{{ robobeg : 2008-10-21
	//lua_tinker::class_def<CX2GUNPC>( L, "SetShow",						&CX2GUNPC::SetShow);
    lua_tinker::class_def<CX2GUNPC>( L, "SetShow",						&CX2GUNPC::SetShowObject);
//}} robobeg : 2008-10-21
	lua_tinker::class_def<CX2GUNPC>( L, "GetAITargetPos",				&CX2GUNPC::GetAITargetPos);
	lua_tinker::class_def<CX2GUNPC>( L, "SetEnableLuaTargetingFunc",	&CX2GUNPC::SetEnableLuaTargetingFunc);
	lua_tinker::class_def<CX2GUNPC>( L, "SetLuaTargetingFunc",			&CX2GUNPC::SetLuaTargetingFunc);	
	lua_tinker::class_def<CX2GUNPC>( L, "SetTargetUnit",				&CX2GUNPC::SetTargetUnit);
	lua_tinker::class_def<CX2GUNPC>( L, "SetTargetUnitUID",				&CX2GUNPC::SetTargetUnitUID);
	lua_tinker::class_def<CX2GUNPC>( L, "SetTargetUnitType",			&CX2GUNPC::SetTargetUnitType);
	lua_tinker::class_def<CX2GUNPC>( L, "GetTargetUnitUID",				&CX2GUNPC::GetTargetUnitUID);	
	lua_tinker::class_def<CX2GUNPC>( L, "GetTargetUser",				&CX2GUNPC::GetTargetUser);
	lua_tinker::class_def<CX2GUNPC>( L, "GetNearestUserPos",			&CX2GUNPC::GetNearestUserPos);
	lua_tinker::class_def<CX2GUNPC>( L, "GetNearestNPCPos",				&CX2GUNPC::GetNearestNPCPos);
	lua_tinker::class_def<CX2GUNPC>( L, "IsUnitID",			        	&CX2GUNPC::IsUnitID);        

	lua_tinker::class_def<CX2GUNPC>( L, "GetStartState",				&CX2GUNPC::GetStartState);
	lua_tinker::class_def<CX2GUNPC>( L, "GetDisabledStateID",			&CX2GUNPC::GetDisabledStateID);
	lua_tinker::class_def<CX2GUNPC>( L, "StateChangeForce",				&CX2GUNPC::StateChangeForce);
	lua_tinker::class_def<CX2GUNPC>( L, "SetRageForce",					&CX2GUNPC::SetRageForce);
	lua_tinker::class_def<CX2GUNPC>( L, "GetNowStateID",				&CX2GUNPC::GetNowStateID);
#ifdef MONSTER_STATE_CHANGE_CHEAT
	lua_tinker::class_def<CX2GUNPC>( L, "GetStateIDByKeyword",		&CX2GUNPC::GetStateIDByKeyword);
#endif MONSTER_STATE_CHANGE_CHEAT
	lua_tinker::class_def<CX2GUNPC>( L, "StateChange_LUA",				&CX2GUNPC::StateChange_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "SetTimerRestart",				&CX2GUNPC::SetTimerRestart);
	lua_tinker::class_def<CX2GUNPC>( L, "GetTimerElapsedTime",			&CX2GUNPC::GetTimerElapsedTime);
	lua_tinker::class_def<CX2GUNPC>( L, "IsSelfDestructing",			&CX2GUNPC::IsSelfDestructing);
	lua_tinker::class_def<CX2GUNPC>( L, "StartSelfDestruction",			&CX2GUNPC::StartSelfDestruction);
	lua_tinker::class_def<CX2GUNPC>( L, "StopSelfDestruction",			&CX2GUNPC::StopSelfDestruction);
	lua_tinker::class_def<CX2GUNPC>( L, "AttachParticle_LUA",			&CX2GUNPC::AttachParticle_LUA);
    lua_tinker::class_def<CX2GUNPC>( L, "GetRandVal",			        &CX2GUNPC::GetRandomInt);
    lua_tinker::class_def<CX2GUNPC>( L, "GetAttackFront",			    &CX2GUNPC::GetAttackFront);
    lua_tinker::class_def<CX2GUNPC>( L, "SetShowBossGage",			    &CX2GUNPC::SetShowBossGage);
	lua_tinker::class_def<CX2GUNPC>( L, "GetShowBossName",			    &CX2GUNPC::GetShowBossName);	
    lua_tinker::class_def<CX2GUNPC>( L, "SetCanPassUnit",			    &CX2GUNPC::SetCanPassUnit);
    lua_tinker::class_def<CX2GUNPC>( L, "SetFocusCamera",			    &CX2GUNPC::SetFocusCamera);
	//lua_tinker::class_def<CX2GUNPC>( L, "ChangeNPC",					&CX2GUNPC::ChangeNPC);	
	lua_tinker::class_def<CX2GUNPC>( L, "SetUnitFadeStart",				&CX2GUNPC::SetUnitFadeStart);
	lua_tinker::class_def<CX2GUNPC>( L, "SetUnitFadeMove",				&CX2GUNPC::SetUnitFadeMove);
	lua_tinker::class_def<CX2GUNPC>( L, "SetUnitFadeScale",				&CX2GUNPC::SetUnitFadeScale);	
	lua_tinker::class_def<CX2GUNPC>( L, "SetUnitColor",					&CX2GUNPC::SetUnitColor);
	lua_tinker::class_def<CX2GUNPC>( L, "CreateArmagedonBlade",			&CX2GUNPC::CreateArmagedonBlade);
#ifdef PVP_BOT
	lua_tinker::class_def<CX2GUNPC>( L, "CreateArmagedonBladeByBot",	&CX2GUNPC::CreateArmagedonBladeByBot);
	lua_tinker::class_def<CX2GUNPC>( L, "ActiveSkillShow_LUA",			&CX2GUNPC::ActiveSkillShow_LUA);	
#endif
	lua_tinker::class_def<CX2GUNPC>( L, "AddConsultNpc",				&CX2GUNPC::AddConsultNpc);	
	lua_tinker::class_def<CX2GUNPC>( L, "GetConsultNpc",				&CX2GUNPC::GetConsultNpc);	
	lua_tinker::class_def<CX2GUNPC>( L, "GetEntangleTrapAttackDuration",	&CX2GUNPC::GetEntangleTrapAttackDuration);	
	lua_tinker::class_def<CX2GUNPC>( L, "GetClassType",					&CX2GUNPC::GetClassType);	
	
	lua_tinker::class_def<CX2GUNPC>( L, "GetRealatedSkillLevel",		&CX2GUNPC::GetRealatedSkillLevel );
	lua_tinker::class_def<CX2GUNPC>( L, "SetRealatedSkillLevel",		&CX2GUNPC::SetRealatedSkillLevel );

	//{{ kimhc // 2010.8.7 // ������ NPC���� ī�޶� ������ �ϴ� ���
#ifdef	FOCUS_CAMERA_NPC_FORCE
	lua_tinker::class_def<CX2GUNPC>( L, "GetFocusCameraForce",			&CX2GUNPC::GetFocusCameraForce);	
	lua_tinker::class_def<CX2GUNPC>( L, "SetFocusCameraForce",			&CX2GUNPC::SetFocusCameraForce);
#endif	FOCUS_CAMERA_NPC_FORCE
	//}} kimhc // 2010.8.7 // ������ NPC���� ī�޶� ������ �ϴ� ���

//{{oasis:�����////2009-10-7////
	lua_tinker::class_def<CX2GUNPC>( L, "GetTargetUnitUID_LUA",						&CX2GUNPC::GetTargetUnitUID_LUA);	

	lua_tinker::class_def<CX2GUNPC>( L, "SetGroupAICommand_LUA",							&CX2GUNPC::SetGroupAICommand_LUA);	
	lua_tinker::class_def<CX2GUNPC>( L, "GetGroupAICommand_LUA",							&CX2GUNPC::GetGroupAICommand_LUA);	
	lua_tinker::class_def<CX2GUNPC>( L, "COMMAND_HOLD_LUA",									&CX2GUNPC::COMMAND_HOLD_LUA);	
	lua_tinker::class_def<CX2GUNPC>( L, "COMMAND_ATTACK_MOVE_LUA",							&CX2GUNPC::COMMAND_ATTACK_MOVE_LUA);	
	lua_tinker::class_def<CX2GUNPC>( L, "COMMAND_FORCED_MOVE_TARGET_LUA",					&CX2GUNPC::COMMAND_FORCED_MOVE_TARGET_LUA);	
	lua_tinker::class_def<CX2GUNPC>( L, "COMMAND_FORCED_MOVE_GROUND_LUA",					&CX2GUNPC::COMMAND_FORCED_MOVE_GROUND_LUA);	
	lua_tinker::class_def<CX2GUNPC>( L, "COMMAND_SKILL_LUA",								&CX2GUNPC::COMMAND_SKILL_LUA);	

	lua_tinker::class_def<CX2GUNPC>( L, "GetOnlyConsultNPC_LUA",							&CX2GUNPC::GetOnlyConsultNPC_LUA);	
	lua_tinker::class_def<CX2GUNPC>( L, "GetOnlyConsultedNPC_LUA",							&CX2GUNPC::GetOnlyConsultedNPC_LUA);	
	lua_tinker::class_def<CX2GUNPC>( L, "GetConsultNearNPC_LUA",							&CX2GUNPC::GetConsultNearNPC_LUA);	
	lua_tinker::class_def<CX2GUNPC>( L, "IsRightTarget_LUA",								&CX2GUNPC::IsRightTarget_LUA);	


	lua_tinker::class_def<CX2GUNPC>( L, "SetAllFlag_LUA",									&CX2GUNPC::SetAllFlag_LUA);	

	lua_tinker::class_def<CX2GUNPC>( L, "SetGroupAISelfCommandTransit_LUA",					&CX2GUNPC::SetGroupAISelfCommandTransit_LUA);	

	lua_tinker::class_def<CX2GUNPC>( L, "SetPrevAIMessage_LUA",									&CX2GUNPC::SetPrevAIMessage_LUA);	
	
	lua_tinker::class_def<CX2GUNPC>( L, "SetAItoMainAIData_LUA",							&CX2GUNPC::SetAItoMainAIData_LUA);	
	lua_tinker::class_def<CX2GUNPC>( L, "SetAItoSubAIData_LUA",								&CX2GUNPC::SetAItoSubAIData_LUA);	
//}}oasis:�����////2009-10-7////

#ifdef MONSTER_REFLECT_DAMAGE_DATA
//{{ oasis907 : ����� [2010.4.19] // 
	lua_tinker::class_def<CX2GUNPC>( L, "SetReflectDamageOnPhysicalDamage_LUA",				&CX2GUNPC::SetReflectDamageOnPhysicalDamage);	
	lua_tinker::class_def<CX2GUNPC>( L, "SetReflectDamageOnMagicalDamage_LUA",				&CX2GUNPC::SetReflectDamageOnMagicalDamage);	
//}}
#endif MONSTER_REFLECT_DAMAGE_DATA

#ifdef RIDING_MONSTER
	lua_tinker::class_def<CX2GUNPC>( L, "GetRideUnit_LUA",									&CX2GUNPC::GetRideUnit);	
	lua_tinker::class_def<CX2GUNPC>( L, "RideOff_LUA",										&CX2GUNPC::RideOff);	
	lua_tinker::class_def<CX2GUNPC>( L, "IsMoveableState_LUA",								&CX2GUNPC::IsMoveableState);	
	lua_tinker::class_def<CX2GUNPC>( L, "JumpDownFromLinemap_LUA",							&CX2GUNPC::JumpDownFromLinemap);	
#endif RIDING_MONSTER

#ifdef SERV_SECRET_HELL
	//{{ megagame / �ڱ��� / [2010-04-06] ��Ʈ ī��Ʈ ��� �߰�
	lua_tinker::class_def<CX2GUNPC>( L, "GetHitCount_LUA",									&CX2GUNPC::GetHitCount_LUA);	
	lua_tinker::class_def<CX2GUNPC>( L, "SetHitCount_LUA",									&CX2GUNPC::SetHitCount_LUA);	

	lua_tinker::class_def<CX2GUNPC>( L, "GetStateID_LUA",									&CX2GUNPC::GetStateID_LUA);	
	lua_tinker::class_def<CX2GUNPC>( L, "GetNextStateID_LUA",								&CX2GUNPC::GetNextStateID_LUA);	
	//}} megagame / �ڱ��� / [2010-04-06]

	//{{ megagame / �ڱ��� / [2010-04-29] ������ �ڵ鸵 �߰�
	lua_tinker::class_def<CX2GUNPC>( L, "GetSiegeMode",										&CX2GUNPC::GetSiegeMode);	
	lua_tinker::class_def<CX2GUNPC>( L, "SetSiegeMode",										&CX2GUNPC::SetSiegeMode);	
	//}} megagame / �ڱ��� / [2010-04-29] ������ �ڵ鸵 �߰�

	//{{ kimhc // 2010-04-16 // ���ݷ�, ���� �� �ڵ鸵 �� �� �ֵ��� ��
	lua_tinker::class_def<CX2GUNPC>( L, "SetAtkPhysic_LUA",									&CX2GUNPC::SetAtkPhysic_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "SetAtkMagic_LUA",									&CX2GUNPC::SetAtkMagic_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "SetDefPhysic_LUA",									&CX2GUNPC::SetDefPhysic_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "SetDefMagic_LUA",									&CX2GUNPC::SetDefMagic_LUA);

	lua_tinker::class_def<CX2GUNPC>( L, "GetAtkPhysic_LUA",									&CX2GUNPC::GetAtkPhysic_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "GetAtkMagic_LUA",									&CX2GUNPC::GetAtkMagic_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "GetDefPhysic_LUA",									&CX2GUNPC::GetDefPhysic_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "GetDefMagic_LUA",									&CX2GUNPC::GetDefMagic_LUA);

	lua_tinker::class_def<CX2GUNPC>( L, "ReInitAtkAndDef_LUA",								&CX2GUNPC::ReInitAtkAndDef_LUA);

	lua_tinker::class_def<CX2GUNPC>( L, "SetEffectSet_LUA",									&CX2GUNPC::SetEffectSet_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "GetEffectSet_LUA",									&CX2GUNPC::GetEffectSet_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "ClearEffectSet_LUA",								&CX2GUNPC::ClearEffectSet_LUA);

	lua_tinker::class_def<CX2GUNPC>( L, "SetDefaultAnimSpeed_LUA",							&CX2GUNPC::SetDefaultAnimSpeed_LUA);
	lua_tinker::class_def<CX2GUNPC>( L, "GetDefaultAnimSpeed_LUA",							&CX2GUNPC::GetDefaultAnimSpeed_LUA);

	//}} kimhc // 2010-04-16 // ���ݷ�, ���� �� �ڵ鸵 �� �� �ֵ��� ��
#endif SERV_SECRET_HELL

	//{{ kimhc // 2010.6.29 // ������ ������Ʈ�� �߿��� Ư�� ������ ������Ʈ�� �ٸ� ������Ʈ�� ����
	lua_tinker::class_def<CX2GUNPC>( L, "ChangeDamageStateID_LUA",							&CX2GUNPC::ChangeDamageStateID_LUA );
	lua_tinker::class_def<CX2GUNPC>( L, "ChangeDyingStateID_LUA",							&CX2GUNPC::ChangeDyingStateID_LUA );
	//}} kimhc // 2010.6.29 // small_front, big_back ���� ���� ������Ʈ �� �߿��� A��� stateID�� �������� B��� stateID�� ����

//{{ kimhc // 2010.7.7 // ���Ͱ� ������ ���θ�
#ifdef	LINE_MAP_CREATED_BY_MONSTER
	lua_tinker::class_def<CX2GUNPC>( L, "InsertMonsterLineMapIndex_LUA",					&CX2GUNPC::InsertMonsterLineMapIndex_LUA );
	lua_tinker::class_def<CX2GUNPC>( L, "GetNumOfMonsterLineMap_LUA",						&CX2GUNPC::GetNumOfMonsterLineMap_LUA );
	lua_tinker::class_def<CX2GUNPC>( L, "GetMonsterLineMapIndex_LUA",						&CX2GUNPC::GetMonsterLineMapIndex_LUA );

#endif	LINE_MAP_CREATED_BY_MONSTER
//}} kimhc // 2010.7.7 // ���Ͱ� ������ ���θ�
	//{{ JHKang / ������ / 2010/12/20 / �� ���� ����Ʈ(SetDie ����)
#ifdef SEASON3_MONSTER_2010_12
	lua_tinker::class_def<CX2GUNPC>( L,	"SetDrainHPSeq",									&CX2GUNPC::SetDrainHPSeq );
	lua_tinker::class_def<CX2GUNPC>( L,	"GetReflectMagic_LUA",								&CX2GUNPC::GetReflectMagic );	
	lua_tinker::class_def<CX2GUNPC>( L,	"GetDamageTypeThisFrame_LUA",						&CX2GUNPC::GetDamageTypeThisFrame );
	lua_tinker::class_def<CX2GUNPC>( L,	"InitDamageTypeThisFrame_LUA",						&CX2GUNPC::InitDamageTypeThisFrame );
#endif SEASON3_MONSTER_2010_12
	//}} JHKang / ������ / 2010/12/20 / �� ���� ����Ʈ(SetDie ����)

#ifdef SECRET_VELDER
	lua_tinker::class_def<CX2GUNPC>( L,	"SetCriticalRate",									&CX2GUNPC::SetCriticalRate );
	lua_tinker::class_def<CX2GUNPC>( L,	"SetMPChangeRate",									&CX2GUNPC::SetCriticalRate );
	lua_tinker::class_def<CX2GUNPC>( L, "GetNearestUserLandPos",							&CX2GUNPC::GetNearestUserLandPos);
#endif

#ifdef USE_UNIT_FOG_RENDER_PARAM //JHKang
	lua_tinker::class_def<CX2GUNPC>( L, "SetFogNPC_LUA",									&CX2GUNPC::SetFogNPC);
#endif //USE_UNIT_FOG_RENDER_PARAM

	//}} GUNPC End


///////////////////////////////////////////////
	
#ifdef SKIN_LIGHTFLOW    
    lua_tinker::class_def<CX2GUNPC>( L, "SetLightFlowType",			    &CX2GUNPC::SetLightFlowType);
    lua_tinker::class_def<CX2GUNPC>( L, "SetLightFlowMin",			    &CX2GUNPC::SetLightFlowMin);
    lua_tinker::class_def<CX2GUNPC>( L, "SetLightFlowMax",			    &CX2GUNPC::SetLightFlowMax);
    lua_tinker::class_def<CX2GUNPC>( L, "SetLightFlowSpeed",			&CX2GUNPC::SetLightFlowSpeed);
    lua_tinker::class_def<CX2GUNPC>( L, "SetLightFlowWide",			    &CX2GUNPC::SetLightFlowWide);
    lua_tinker::class_def<CX2GUNPC>( L, "SetLightFlowColor",			&CX2GUNPC::SetLightFlowColor);
    lua_tinker::class_def<CX2GUNPC>( L, "SetLightFlowPoint",			&CX2GUNPC::SetLightFlowPoint);
#endif
#ifdef BONUS_DROP
	lua_tinker::class_def<CX2GUNPC>( L, "SetBonusDrop",					&CX2GUNPC::SetBonusDrop);
#endif
#ifdef USER_HOLD	
	lua_tinker::class_def<CX2GUNPC>( L, "SetUserHoldReset",				&CX2GUNPC::SetUserHoldReset);
	lua_tinker::class_def<CX2GUNPC>( L, "SetUserHold",					&CX2GUNPC::SetUserHold);
	lua_tinker::class_def<CX2GUNPC>( L, "GetUserHold",					&CX2GUNPC::GetUserHold);
#endif


#ifdef TEST_GROUP_GRAP
	lua_tinker::class_def<CX2GUNPC>( L, "IsLostGrap",				&CX2GUNPC::IsLostGrap);
	lua_tinker::class_def<CX2GUNPC>( L, "SetUserGrapReset",				&CX2GUNPC::SetUserGrapReset);
	lua_tinker::class_def<CX2GUNPC>( L, "SetUserGrap",					&CX2GUNPC::SetUserGrap);
	lua_tinker::class_def<CX2GUNPC>( L, "GetUserGrap",					&CX2GUNPC::GetUserGrap);
#endif TEST_GROUP_GRAP

//{{ kimhc // 2010.7.27 // �������� ������ HOLD �� �� �ֵ��� ��
#ifdef	USER_HOLD_EX
	lua_tinker::class_def<CX2GUNPC>( L, "SetUnHoldWhenRevenged",		&CX2GUNPC::SetUnHoldWhenRevenged);
#endif	USER_HOLD_EX
//}} kimhc // 2010.7.27 // �������� ������ HOLD �� �� �ֵ��� ��
	lua_tinker::class_def<CX2GUNPC>( L, "GetLastAttackGameUnit",		&CX2GUNPC::GetLastAttackGameUnit);	
	lua_tinker::class_def<CX2GUNPC>( L, "GetLastAttackUnit",			&CX2GUNPC::GetLastAttackUnit);
	lua_tinker::class_def<CX2GUNPC>( L, "GetHitter",					&CX2GUNPC::GetHitter);   

#ifdef VELDER_BOSS_5
	//{{ oasis907 : ����� [2010.7.28] // 
	lua_tinker::class_def<CX2GUNPC>( L, "SetRideToUnit_LUA",			&CX2GUNPC::SetRideToUnit );
	lua_tinker::class_def<CX2GUNPC>( L, "StopRideToUnit_LUA",			&CX2GUNPC::StopRideToUnit );
	lua_tinker::class_def<CX2GUNPC>( L, "SetPositionForce",				&CX2GUNPC::SetPositionForce);

	lua_tinker::class_def<CX2GUNPC>( L, "SetCloaking",					&CX2GUNPC::SetCloaking);
	lua_tinker::class_def<CX2GUNPC>( L, "SetUnCloaking",				&CX2GUNPC::SetUnCloaking);

	//}} oasis907 : ����� [2010.7.28] // 	
#endif VELDER_BOSS_5

#ifdef GUNPC_TOGGLE_APPLY_MOTION_OFFSET
	lua_tinker::class_def<CX2GUNPC>( L, "SetApplyMotionOffset",				&CX2GUNPC::SetApplyMotionOffset);
#endif GUNPC_TOGGLE_APPLY_MOTION_OFFSET

#ifdef FIX_LINK_ATTACK
	lua_tinker::class_def<CX2GUNPC>( L, "GetNearestGUUser",		&CX2GUNPC::GetNearestGUUser );
	lua_tinker::class_def<CX2GUNPC>( L, "SetNearestGUUser",	&CX2GUNPC::SetNearestGUUser );
#endif

#ifdef SEASON3_MONSTER_2011_7
	lua_tinker::class_def<CX2GUNPC>( L,	"SetRageCountMax",				&CX2GUNPC::SetRageCountMax );
	lua_tinker::class_def<CX2GUNPC>( L,	"SetEnableAfterImageAlways",	&CX2GUNPC::SetEnableAfterImageAlways );
	lua_tinker::class_def<CX2GUNPC>( L,	"ChangeAfterImageColor",		&CX2GUNPC::ChangeAfterImageColor );
#endif SEASON3_MONSTER_2011_7

#ifdef CHUNG_SECOND_CLASS_CHANGE
	lua_tinker::class_def<CX2GUNPC>( L, "GetOwnerUserUnitUID",		&CX2GUNPC::	GetOwnerGameUnitUID );
#endif

#ifdef VELDER_SECRET_DUNGEON
	lua_tinker::class_def<CX2GUNPC>( L, "IsOnSomething_LUA",		&CX2GUNPC::IsOnSomething);
#endif

#ifdef CREATE_ALLY_NPC_BY_MONSTER
	lua_tinker::class_def<CX2GUNPC>( L, "CreateAllyNpcByMonster_LUA",		&CX2GUNPC::CreateAllyNpcByMonster_LUA);	
	lua_tinker::class_def<CX2GUNPC>( L, "SetCoolTimeSummonCard_LUA",		&CX2GUNPC::SetCoolTimeSummonCard_LUA);	
#endif //CREATE_ALLY_NPC_BY_MONSTER

#ifdef SERV_CHUNG_TACTICAL_TROOPER
	lua_tinker::class_def<CX2GUNPC>( L, "GetNearestUnitPosByOwnerUnit", &CX2GUNPC::GetNearestUnitPosByOwnerUnit );		/// NPC�� ��ȯ�� ������ ���� ������ �ִ� �� ������ ��ȯ
	lua_tinker::class_def<CX2GUNPC>( L, "GetNearestActiveNPCPos",		&CX2GUNPC::GetNearestActiveNPCPos );			/// ���� Active ���� NPC�� ��ġ�� ��ȯ
#endif SERV_CHUNG_TACTICAL_TROOPER

#ifdef ADD_CHANGE_BACKGROUND_MUSIC
	//lua_tinker::class_def<CX2GUNPC>( L, "ChangeBGM_LUA",				&CX2GUNPC::ChangeBGM_LUA	);
#endif // ADD_CHANGE_BACKGROUND_MUSIC


	lua_tinker::class_add<CX2GUUser>( L, "CX2GUUser" );
	lua_tinker::class_inh<CX2GUUser, CX2GameUnit>( L );
	lua_tinker::class_def<CX2GUUser>( L, "GetHyperModeUsed",			&CX2GUUser::GetHyperModeUsed );
	lua_tinker::class_def<CX2GUUser>( L, "SetDevAddDamage",				&CX2GUUser::SetDevAddDamage );
	lua_tinker::class_def<CX2GUUser>( L, "ForceChangeHyperMode",		&CX2GUUser::ForceChangeHyperMode );
	lua_tinker::class_def<CX2GUUser>( L, "GetNowSpecialAttack",			&CX2GUUser::GetNowSpecialAttack );
	lua_tinker::class_def<CX2GUUser>( L, "GetHoldSpecialAttack",		&CX2GUUser::GetHoldSpecialAttack );
	lua_tinker::class_def<CX2GUUser>( L, "GetHoldingSpecialAttack",		&CX2GUUser::GetHoldingSpecialAttack );
	lua_tinker::class_def<CX2GUUser>( L, "GetAttackSuccess",			&CX2GUUser::GetAttackSuccess );
	lua_tinker::class_def<CX2GUUser>( L, "GetRotateDegree",				&CX2GUUser::GetRotateDegree );
#ifdef USER_HOLD
	lua_tinker::class_def<CX2GUUser>( L, "SetPos",						&CX2GUUser::SetPos );	
	lua_tinker::class_def<CX2GUUser>( L, "SetHold",						&CX2GUUser::SetHold );	
	lua_tinker::class_def<CX2GUUser>( L, "GetHold",						&CX2GUUser::GetHold );	
#endif
	
#ifdef TRANSFORMER_TEST
	lua_tinker::class_def<CX2GUUser>( L, "TransformIntoMonster",		&CX2GUUser::TransformIntoMonster );	
	lua_tinker::class_def<CX2GUUser>( L, "TransformIntoUser",			&CX2GUUser::TransformIntoUser );	
#endif TRANSFORMER_TEST

#ifdef VELDER_BOSS_5
	//{{ oasis907 : ����� [2010.8.2] // 
	lua_tinker::class_def<CX2GUUser>( L, "GetLandPosition_LUA",			&CX2GUUser::GetLandPos );	
	//}}
#endif VELDER_BOSS_5

	//{{ kimhc // 2010.8.10 // Ư�� ������ ��ġ ���� ��Ȱ ��ġ�� ���
#ifdef	USE_MONSTER_POS_FOR_REBIRTH
	lua_tinker::class_def<CX2GUUser>( L, "InitPosByMonsterPos",			&CX2GUUser::InitPosByMonsterPos );
#endif	USE_MONSTER_POS_FOR_REBIRTH
	//}} kimhc // 2010.8.10 // Ư�� ������ ��ġ ���� ��Ȱ ��ġ�� ���
	//{{ JHKang / ������ / 2010/12/16 / ���� �ǰ� �����ΰ�?
#ifdef SEASON3_MONSTER_2010_12
	lua_tinker::class_def<CX2GUUser>( L, "GetNowHit",					&CX2GUUser::GetNowHit );
	lua_tinker::class_def<CX2GUUser>( L, "SetNowHit",					&CX2GUUser::SetNowHit );
#endif SEASON3_MONSTER_2010_12 // ���� �ǰ� �����ΰ�?
	//}} JHKang / ������ / 2010/12/16

	//{{ JHKang / ������ / 2011/01/17 / ���� ������ ���� ������ ��� �Լ�
#ifdef SEASON3_MONSTER_2010_12
	lua_tinker::class_def<CX2GUUser>( L, "GetRealDamage",		&CX2GUUser::GetRealDamage );
	lua_tinker::class_def<CX2GUUser>( L, "SetAnimSpeed_LUA",	&CX2GUUser::SetAnimSpeed_LUA );
#endif SEASON3_MONSTER_2010_12
	//}} JHKang / ������ / 2011/01/17 / ���� ������ ���� ������ ��� �Լ�

#ifdef NEW_CHARACTER_CHUNG
	// oasis907 : ����� [2010.12.21] // û Ʃ�丮��
	lua_tinker::class_def<CX2GUUser>( L, "SetHyperModeCount",			&CX2GUUser::SetHyperModeCount );	
#endif NEW_CHARACTER_CHUNG

#ifdef SEASON3_MONSTER_2011_7
	lua_tinker::class_def<CX2GUUser>( L, "GetIsRight",					&CX2GUUser::GetIsRight);
#endif SEASON3_MONSTER_2011_7	
	
#ifdef NEW_HENIR_TEST
	lua_tinker::class_def<CX2GUUser>( L, "StartHenirBuffFirst",			&CX2GUUser::StartHenirBuffFirst);
#ifndef ADD_HENIR_BUFF		/// ������ ����Ǹ鼭 ������ �ʰ� �� ����
	lua_tinker::class_def<CX2GUUser>( L, "StartHenirBuffFire",			&CX2GUUser::StartHenirBuffFire);
	lua_tinker::class_def<CX2GUUser>( L, "StartHenirBuffWater",			&CX2GUUser::StartHenirBuffWater);
	lua_tinker::class_def<CX2GUUser>( L, "StartHenirBuffNature",		&CX2GUUser::StartHenirBuffNature);
	lua_tinker::class_def<CX2GUUser>( L, "StartHenirBuffWind",			&CX2GUUser::StartHenirBuffWind);
	lua_tinker::class_def<CX2GUUser>( L, "StartHenirBuffLight",			&CX2GUUser::StartHenirBuffLight);
#endif ADD_HENIR_BUFF
	lua_tinker::class_def<CX2GUUser>( L, "StartHenirBuffDark",			&CX2GUUser::StartHenirBuffDark);
	lua_tinker::class_def<CX2GUUser>( L, "EndHenirBuff",				&CX2GUUser::EndHenirBuff);
	
	lua_tinker::class_def<CX2GUUser>( L, "CreateTeleportEffect",		&CX2GUUser::CreateTeleportEffect);

	lua_tinker::class_def<CX2GUUser>( L, "SetPositionOnLine",			&CX2GUUser::SetPositionOnLine);
	lua_tinker::class_def<CX2GUUser>( L, "SetShow",						&CX2GUUser::SetShowObject);
#endif NEW_HENIR_TEST


#ifdef NEXON_QA_CHEAT_REQ
	lua_tinker::class_def<CX2GUUser>( L, "SetInvincibleAndNoMpConsume_Cheat",				&CX2GUUser::SetInvincibleAndNoMpConsume_Cheat);
#endif //NEXON_QA_CHEAT_REQ

#ifdef USER_LEVEL_IN_LUA
	lua_tinker::class_def<CX2GUUser>( L, "GetUnitLevel",					&CX2GUUser::GetUnitLevel );
#endif USER_LEVEL_IN_LUA



	lua_tinker::class_add<CX2DamageEffect>( L, "CX2DamageEffect" );
	lua_tinker::class_def<CX2DamageEffect>( L, "CreateInstance_LUA",		&CX2DamageEffect::CreateInstance_LUA );
	lua_tinker::class_def<CX2DamageEffect>( L, "CreateInstance_LUA2",		&CX2DamageEffect::CreateInstance_LUA2 );	
#ifdef SERV_PET_SYSTEM
	lua_tinker::class_def<CX2DamageEffect>( L, "CreateInstanceByPet_LUA",		&CX2DamageEffect::CreateInstanceByPet_LUA );
#endif
	lua_tinker::class_def<CX2DamageEffect>( L, "CreateInstanceParabolic_LUA",	&CX2DamageEffect::CreateInstanceParabolic_LUA );
	lua_tinker::class_def<CX2DamageEffect>( L, "IsLiveInstance",			&CX2DamageEffect::IsLiveInstance );
	lua_tinker::class_def<CX2DamageEffect>( L, "DestroyInstance",			&CX2DamageEffect::DestroyInstance );

	lua_tinker::class_add<CX2EffectSet>( L, "CX2EffectSet" );
	lua_tinker::class_def<CX2EffectSet>( L, "AddEffectSetTemplet_LUA",		&CX2EffectSet::AddEffectSetTemplet_LUA );
#ifdef EFFECT_TOOL
	lua_tinker::class_def<CX2EffectSet>( L, "AddEffectSet_Description",		&CX2EffectSet::AddEffectSet_Description_LUA );
	lua_tinker::class_def<CX2EffectSet>( L, "SetEffectSetVersion",			&CX2EffectSet::SetEffectSetVersion_LUA );
#endif //EFFECT_TOOL
	//{{ kimhc // 2010.4.19 // ��д��� �۾�
#ifdef SERV_SECRET_HELL
	lua_tinker::class_def<CX2EffectSet>( L, "PlayEffectSet_LUA",				&CX2EffectSet::PlayEffectSet_LUA );
	lua_tinker::class_def<CX2EffectSet>( L, "PlayEffectSetWithCustomPos_LUA",	&CX2EffectSet::PlayEffectSetWithCustomPos_LUA );
	lua_tinker::class_def<CX2EffectSet>( L, "StopEffectSet_LUA",				&CX2EffectSet::StopEffectSet_LUA );
#endif SERV_SECRET_HELL
	//}} kimhc // 2010.4.19 // ��д��� �۾�
	
	//{{ JHKang / ������ / 2011/01/19
#ifdef SEASON3_MONSTER_2010_12
	lua_tinker::class_def<CX2EffectSet>( L, "PlayEffectSetTraceTarget_LUA",	&CX2EffectSet::PlayEffectSetTraceTarget_LUA );
#endif SEASON3_MONSTER_2010_12
	//}} JHKang / ������ / 2011/01/19

#ifdef SERV_PET_SYSTEM
	lua_tinker::class_def<CX2EffectSet>( L, "PlayEffectSetByPet_LUA",			&CX2EffectSet::PlayEffectSetByPet_LUA );	
#endif
#ifdef ADD_PET_UNICORN
	lua_tinker::class_def<CX2EffectSet>( L, "SetEffectSetInstPos",			&CX2EffectSet::SetEffectSetInstPos );
#endif
	lua_tinker::class_def<CX2EffectSet>( L, "SetEffectScale_LUA",			&CX2EffectSet::SetEffectScale_LUA );

#ifdef GET_EFFECTSET_POSITION_IN_LUA
	lua_tinker::class_def<CX2EffectSet>( L, "GetEffectPosition_LUA",		&CX2EffectSet::GetEffectPosition_LUA );
#endif GET_EFFECTSET_POSITION_IN_LUA
	
//#ifdef SERV_HERO_PVP_MANAGE_LIST
//	lua_tinker::class_def<CX2UIHeroMatch>( L, "AddHelperText_Hero",		&CX2UIHeroMatch::AddHelperText_Hero );
//#endif //SERV_HERO_PVP_MANAGE_LIST

	lua_tinker::class_add<CX2GameEdit>( L, "CX2GameEdit" );
	lua_tinker::class_def<CX2GameEdit>( L, "AddCommandChange",			&CX2GameEdit::AddCommandChange );
	lua_tinker::class_def<CX2GameEdit>( L, "AddString",					&CX2GameEdit::AddString_LUA );
#ifdef MODIFY_EDIT_HELP
	lua_tinker::class_def<CX2GameEdit>( L, "AddHelperText",				&CX2GameEdit::AddHelperText );
#else
	lua_tinker::class_def<CX2GameEdit>( L, "SetCommandListHelperText",	&CX2GameEdit::SetCommandListHelperText );
#endif //MODIFY_EDIT_HELP
	lua_tinker::class_def<CX2GameEdit>( L, "ToggleCommandList",			&CX2GameEdit::ToggleCommandList );
	lua_tinker::class_def<CX2GameEdit>( L, "SaveFileLoadedDevices",		&CX2GameEdit::SaveFileLoadedDevices  );
	lua_tinker::class_def<CX2GameEdit>( L, "ChangeSpirit",				&CX2GameEdit::SaveFileLoadedDevices  );
#ifdef MONSTER_STATE_CHANGE_CHEAT
	lua_tinker::class_def<CX2GameEdit>( L, "GetLastCreatedMonster",		&CX2GameEdit::GetLastCreatedMonster  );
	lua_tinker::class_def<CX2GameEdit>( L, "SetLastCreatedMonster",		&CX2GameEdit::SetLastCreatedMonster  );
	lua_tinker::class_def<CX2GameEdit>( L, "MonsterStateChange",		&CX2GameEdit::MonsterStateChange  );
#endif MONSTER_STATE_CHANGE_CHEAT
	//{{ 2010. 8. 10	������	�� �ý���
#ifdef SERV_PET_SYSTEM
	lua_tinker::class_def<CX2GameEdit>( L, "PetCheat",					&CX2GameEdit::PetCheat_LUA );
	lua_tinker::class_def<CX2GameEdit>( L, "PetActionCheat",					&CX2GameEdit::PetActionCheat_LUA );	
#endif SERV_PET_SYSTEM
	//}}
	//{{ 2010. 9. 29	������	ED ����͸� ġƮ
#ifdef SERV_ADMIN_SHOW_ED
	lua_tinker::class_def<CX2GameEdit>( L, "UserEdCheat",				&CX2GameEdit::UserEdCheat_LUA );
#endif SERV_ADMIN_SHOW_ED
	//}}
	//{{ 2011. 04. 16	������	�븮 ����
//#ifdef SERV_PSHOP_AGENCY
	lua_tinker::class_def<CX2GameEdit>( L, "ShopCheat",					&CX2GameEdit::ShopCheat_LUA );
//#endif SERV_PSHOP_AGENCY
	//}}
	//{{ 2011. 08. 11	������	���� ���� ä�θ� 
#ifdef SERV_TOONILAND_CHANNELING // SERV_JAPAN_CHANNELING
	lua_tinker::class_def<CX2GameEdit>( L, "ChannelingCodeCheat",		&CX2GameEdit::ChannelingCodeCheat_LUA );
#endif SERV_TOONILAND_CHANNELING // SERV_JAPAN_CHANNELING
	//}}
	//{{ 2011. 08. 12   ��μ�      ��ϸ� ���� 
#ifdef SERV_NEW_HENIR_TEST
	lua_tinker::class_def<CX2GameEdit>( L, "HenirRewardCountCheat",		&CX2GameEdit::HenirRewardCountCheat_LUA );
#endif SERV_NEW_HENIR_TEST
		//}}
#ifdef NEW_HENIR_TEST
	lua_tinker::class_def<CX2GameEdit>( L, "HenirBuffCheat",			&CX2GameEdit::HenirBuffCheat_LUA );
#endif NEW_HENIR_TEST

#ifdef ELSWORD_WAY_OF_SWORD
	lua_tinker::class_def<CX2GameEdit>( L, "ElswordWSPCheat",			&CX2GameEdit::ElswordWSPCheat_LUA );
#endif ELSWORD_WAY_OF_SWORD

#ifdef SERV_RANDOM_DAY_QUEST
	lua_tinker::class_def<CX2GameEdit>( L, "RefreshDailyQuest",			&CX2GameEdit::RefreshDailyQuest );
#endif SERV_RANDOM_DAY_QUEST

	//{{ 2011. 09. 20  ��μ�	���ӳ� ED ���� ����͸�
#ifdef SERV_ED_MONITORING_IN_GAME
	lua_tinker::class_def<CX2GameEdit>( L, "EDMonitoringModeCheat",		&CX2GameEdit::EDMonitoringModeCheat_LUA );
#endif SERV_ED_MONITORING_IN_GAME
	//}}

#ifdef GUILD_SKILL
	//{{ 2009-11-30  ������
	lua_tinker::class_def<CX2GameEdit>( L, "AdminGetGuildSPoint",		&CX2GameEdit::AdminGetGuildSPoint_LUA );
	//}}
#endif GUILD_SKILL

	//{{ lucidash : 2010.11.24 
#ifdef ITEM_VIEW_CHEAT
	//lua_tinker::class_def<CX2GameEdit>( L, "ItemViewCheat",		&CX2GameEdit::ItemViewCheat );
#endif ITEM_VIEW_CHEAT
	//}} lucidash : 2010.11.24

#ifdef SERV_BATTLE_FIELD_DANGEROUS_CHEAT
	lua_tinker::class_def<CX2GameEdit>( L, "RequireGetDangerousValue",		&CX2GameEdit::RequireGetDangerousValue_LUA );
#endif //SERV_BATTLE_FIELD_DANGEROUS_CHEAT

#ifdef ADDED_CHEAT_BATTLEFIELD_DANGEROUS_VALUE
	lua_tinker::class_def<CX2GameEdit>( L, "IncreaseFieldDangerousValue",		&CX2GameEdit::IncreaseFieldDangerousValue_LUA );

#endif // ADDED_CHEAT_BATTLEFIELD_DANGEROUS_VALUE

#ifdef UDP_CAN_NOT_SEND_USER_KICK // 2012.06.19 lygan_������ // UDP ��Ŷ ���ų� IME ���� �̿������� �������� ��ũ ��Ŷ �Ⱥ����� ���� ű �ϴ� �ڵ� ��� ���� ����
		lua_tinker::class_def<CX2GameEdit>( L, "PvpKickSet",		&CX2GameEdit::PvpKickSet_LUA );
#endif //UDP_CAN_NOT_SEND_USER_KICK

#ifdef SERV_DELETE_GUILD_ADD_CHEAT
		lua_tinker::class_def<CX2GameEdit>( L, "DeleteGUildAdd",	&CX2GameEdit::DeleteGUildAdd_LUA );
#endif // SERV_DELETE_GUILD_ADD_CHEAT
		//{{ 2011.03.04 �ӱԼ� ��ϸ� ��ŷ ���� ġƮ ( ���,������ ���� )
#ifdef SERV_DELETE_HENIR_RANKING
		lua_tinker::class_def<CX2GameEdit>( L, "DeleteHenirRankingRank",	&CX2GameEdit::DeleteHenirRankingRank_LUA );
		lua_tinker::class_def<CX2GameEdit>( L, "DeleteHenirRankingNickName",	&CX2GameEdit::DeleteHenirRankingNickName_LUA );
		lua_tinker::class_def<CX2GameEdit>( L, "DeleteHenirRankingUnitUID",	&CX2GameEdit::DeleteHenirRankingUnitUID_LUA );
#endif SERV_DELETE_HENIR_RANKING
		//}}
#ifdef SERV_INVISIBLE_GM
		lua_tinker::class_def<CX2GameEdit>( L, "ToggleInvisible",			&CX2GameEdit::ToggleInvisible_LUA );
#endif SERV_INVISIBLE_GM

		//{{ kimhc // 2012-10-16 // ���� ���� ������ ȣ��Ʈ�� �����ϴ� �ڵ�
#ifdef	SERV_CHOOSE_FASTEST_HOST
		lua_tinker::class_def<CX2GameEdit>( L, "AveragePingTime",		&CX2GameEdit::AveragePingTime_LUA );	/// Host�� ������ �ΰ��� �׽�Ʈ������ ����
#endif	SERV_CHOOSE_FASTEST_HOST
		//}} kimhc // 2012-10-16 // ���� ���� ������ ȣ��Ʈ�� �����ϴ� �ڵ�

#ifdef SERV_DEVELOPER_RANDOM_OPEN_ITEM_LOG
		lua_tinker::class_def<CX2GameEdit>( L, "RandomCubeOpen",	&CX2GameEdit::RandomCubeOpen );
#endif//SERV_DEVELOPER_RANDOM_OPEN_ITEM_LOG

#ifdef ADDED_RELATIONSHIP_SYSTEM
	lua_tinker::class_def<CX2GameEdit>( L, "IncreaseLoverDate_LUA",	&CX2GameEdit::IncreaseLoverDate_LUA );
#endif // ADDED_RELATIONSHIP_SYSTEM

#ifdef RIDING_SYSTEM
	lua_tinker::class_def<CX2GameEdit>( L, "FullChargeRidingStamina",	&CX2GameEdit::FullChargeRidingStamina );
#endif // RIDING_SYSTEM
#ifdef EFFECT_USE_LOG
	lua_tinker::class_def<CX2GameEdit>( L, "SetEffectLogLevel",	&CX2GameEdit::SetEffectLogLevel );
#endif //EFFECT_USE_LOG

	lua_tinker::class_add<CX2TextManager::CX2Text>( L, "CX2Text" );
	lua_tinker::class_def<CX2TextManager::CX2Text>( L, "Move",				&CX2TextManager::CX2Text::Move_LUA );
	

	lua_tinker::class_add<CX2TextManager>( L, "CX2TextManager" );
	lua_tinker::class_def<CX2TextManager>( L, "AddText",					&CX2TextManager::AddText_LUA );
	

	lua_tinker::class_add<CX2SlotManager>( L, "CX2SlotManager" );
	lua_tinker::class_def<CX2SlotManager>( L, "AddSlot",					&CX2SlotManager::AddSlot );
	lua_tinker::class_def<CX2SlotManager>( L, "CreateItemSlot",				&CX2SlotManager::CreateItemSlot );
	lua_tinker::class_def<CX2SlotManager>( L, "GetDummyValue",				&CX2SlotManager::GetDummyValue_LUA );
	

#ifndef COUPON_SYSTEM // ���� UI ����
	lua_tinker::class_add<CX2CouponBox>( L, "CX2CouponBox" );
	lua_tinker::class_inh< CX2CouponBox, CX2SlotManager>( L );
#endif // COUPON_SYSTEM
	

	
#ifdef POSTBOX
    lua_tinker::class_add<CX2PostBox>( L, "CX2PostBox" );
    lua_tinker::class_inh< CX2PostBox, CX2SlotManager>( L );
#endif


	lua_tinker::class_add<CX2SlotManager::CX2Slot>( L, "CX2Slot" );
	lua_tinker::class_def<CX2SlotManager::CX2Slot>( L, "SetSlotData",		&CX2SlotManager::CX2Slot::SetSlotData_LUA );
	lua_tinker::class_def<CX2SlotManager::CX2Slot>( L, "SetClickable",		&CX2SlotManager::CX2Slot::SetClickable );
	

	lua_tinker::class_add<CX2SlotItem>( L, "CX2SlotItem" );
	lua_tinker::class_inh< CX2SlotItem, CX2SlotManager::CX2Slot>( L );
	


	lua_tinker::class_add<CX2SquareUnit>( L, "CX2SquareUnit" );
	lua_tinker::class_def<CX2SquareUnit>( L, "StateChange_LUA",				&CX2SquareUnit::StateChange_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "SetRight_LUA",				&CX2SquareUnit::SetRight_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetRight_LUA",				&CX2SquareUnit::GetRight_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetPureLeft_LUA",				&CX2SquareUnit::GetPureLeft_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetOneLeft_LUA",				&CX2SquareUnit::GetOneLeft_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetPureDoubleLeft_LUA",		&CX2SquareUnit::GetPureDoubleLeft_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetOneDoubleLeft_LUA",		&CX2SquareUnit::GetOneDoubleLeft_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetPureRight_LUA",			&CX2SquareUnit::GetPureRight_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetOneRight_LUA",				&CX2SquareUnit::GetOneRight_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetPureDoubleRight_LUA",		&CX2SquareUnit::GetPureDoubleRight_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetOneDoubleRight_LUA",		&CX2SquareUnit::GetOneDoubleRight_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetPureUp_LUA",				&CX2SquareUnit::GetPureUp_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetOneUp_LUA",				&CX2SquareUnit::GetOneUp_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetPureDoubleUp_LUA",			&CX2SquareUnit::GetPureDoubleUp_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetOneDoubleUp_LUA",			&CX2SquareUnit::GetOneDoubleUp_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetPureDown_LUA",				&CX2SquareUnit::GetPureDown_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetOneDown_LUA",				&CX2SquareUnit::GetOneDown_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetPureDoubleDown_LUA",		&CX2SquareUnit::GetPureDoubleDown_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetOneDoubleDown_LUA",		&CX2SquareUnit::GetOneDoubleDown_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetFootOnLine_LUA",			&CX2SquareUnit::GetFootOnLine_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "CanDown_LUA",					&CX2SquareUnit::CanDown_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "AddPosY_LUA",					&CX2SquareUnit::AddPosY_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetYSpeed_LUA",				&CX2SquareUnit::GetYSpeed_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "SetXSpeed_LUA",				&CX2SquareUnit::SetXSpeed_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "SetYSpeed_LUA",				&CX2SquareUnit::SetYSpeed_LUA );
#ifdef RIDING_SYSTEM
	lua_tinker::class_def<CX2SquareUnit>( L, "GetRiding_LUA",						&CX2SquareUnit::GetRidingOn );
	lua_tinker::class_def<CX2SquareUnit>( L, "GetCheckWaitHabitElapsedTime_LUA",	&CX2SquareUnit::GetCheckWaitHabitElapsedTime );
	lua_tinker::class_def<CX2SquareUnit>( L, "ResetWaitHabitElapsedTime_LUA",		&CX2SquareUnit::ResetWaitHabitElapsedTime );
	lua_tinker::class_add<CX2RidingPetManager>( L, "CX2RidingPetManager" );
	lua_tinker::class_def<CX2RidingPetManager>( L, "AddRidingPetTemplet",	&CX2RidingPetManager::AddRidingPetTemplet_LUA );
	lua_tinker::class_def<CX2RidingPetManager>( L, "SetEnableStamina", &CX2RidingPetManager::SetEnableStamina_LUA );
#else
	lua_tinker::class_def<CX2SquareUnit>( L, "SetActId_LUA",				&CX2SquareUnit::SetActId_LUA );
#endif //RIDING_SYSTEM
	lua_tinker::class_def<CX2SquareUnit>( L, "GetFieldUnit_LUA",			&CX2SquareUnit::GetFieldUnit_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "IsMyUnit",					&CX2SquareUnit::IsMyUnit );
	lua_tinker::class_def<CX2SquareUnit>( L, "PlaySound_LUA",				&CX2SquareUnit::PlaySound_LUA );
	lua_tinker::class_def<CX2SquareUnit>( L, "IsAnimationEnd_LUA",			&CX2SquareUnit::IsAnimationEnd_LUA );

	

	lua_tinker::class_add<CX2SkillTree>( L, "CX2SkillTree" );
	lua_tinker::class_def<CX2SkillTree>( L, "AddSkillTemplet_LUA",			&CX2SkillTree::AddSkillTemplet_LUA );
	lua_tinker::class_def<CX2SkillTree>( L, "AddSkillTreeTemplet_LUA",		&CX2SkillTree::AddSkillTreeTemplet_LUA );
#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
	lua_tinker::class_def<CX2SkillTree>( L, "AddSkillPowerRate_LUA",		&CX2SkillTree::AddSkillPowerRate_LUA );
	lua_tinker::class_def<CX2SkillTree>( L, "AddDefaultSkill_LUA",			&CX2SkillTree::AddDefaultSkill_LUA );

	lua_tinker::class_def<CX2SkillTree>( L, "IsMyClassSkillID_LUA",			&CX2SkillTree::IsMyClassSkillID );
	lua_tinker::class_def<CX2SkillTree>( L, "GetMaxLimitedSkillLevel_LUA",	&CX2SkillTree::GetMaxLimitedSkillLevel );
#endif // UPGRADE_SKILL_SYSTEM_2013
	//{{ 2009. 8. 5  ������		���ν�ų
	lua_tinker::class_def<CX2SkillTree>( L, "AddSealSkillInfo",				&CX2SkillTree::AddSealSkillInfo_LUA );
	//}}
#ifdef LUA_TRANS_DEVIDE
	lua_tinker::class_def<CX2SkillTree>( L, "AddSkillTempletTrans_LUA",		&CX2SkillTree::AddSkillTempletTrans_LUA );
#endif LUA_TRANS_DEVIDE

// oasis907 : ����� [2009.12.1] // ���� �Ľ� ���� ������ ����
//#ifdef GUILD_SKILL
	//{{ oasis907 : ����� //// 2009-11-12 //// 
	lua_tinker::class_def<CX2SkillTree>( L, "AddGuildSkillTreeTemplet_LUA",		&CX2SkillTree::AddGuildSkillTreeTemplet_LUA );
	//}} oasis907 : ����� //// 2009-11-12 //// 
//#endif GUILD_SKILL


	lua_tinker::class_add<CX2EXPTable>( L, "CX2EXPTable" );
	lua_tinker::class_def<CX2EXPTable>( L, "AddExpTable_LUA",				&CX2EXPTable::AddExpTable_LUA );



	lua_tinker::class_add<CX2DamageEffect::CEffect>( L, "CEffect" );
	lua_tinker::class_def<CX2DamageEffect::CEffect>( L, "SetLockOnNPCUID",					&CX2DamageEffect::CEffect::SetLockOnNPCUID );
	lua_tinker::class_def<CX2DamageEffect::CEffect>( L, "GetLockOnUnitUID_LUA",				&CX2DamageEffect::CEffect::GetLockOnUnitUID );
	lua_tinker::class_def<CX2DamageEffect::CEffect>( L, "GetMainEffect",					&CX2DamageEffect::CEffect::GetMainEffect );


#ifdef SEASON3_MONSTER_2010_12
	lua_tinker::class_def<CX2DamageEffect::CEffect>( L, "SetPos",			&CX2DamageEffect::CEffect::SetPos );
#endif SEASON3_MONSTER_2010_12    

#ifdef ADD_HALLOWEEN_PET_SYSTEM
	lua_tinker::class_def<CX2DamageEffect::CEffect>( L, "GetDamageData",					&CX2DamageEffect::CEffect::GetDamageData );
#endif ADD_HALLOWEEN_PET_SYSTEM
	


	lua_tinker::class_add<CX2TrainingCenterTable>( L, "KTrainingCenterTable" );
	lua_tinker::class_def<CX2TrainingCenterTable>( L, "AddTCTemplet",						&CX2TrainingCenterTable::AddTCTemplet_LUA );
	lua_tinker::class_def<CX2TrainingCenterTable>( L, "SetTrainingListByUnitClass_LUA",		&CX2TrainingCenterTable::SetTrainingListByUnitClass_LUA );
	

	lua_tinker::class_add<CX2DropItemManager>( L, "CX2DropItemManager" );
	lua_tinker::class_def<CX2DropItemManager>( L, "AddDropItem_LUA",						&CX2DropItemManager::AddDropItem_LUA );
	lua_tinker::class_def<CX2DropItemManager>( L, "CountDropItemByID",						&CX2DropItemManager::CountDropItemByID );
	lua_tinker::class_def<CX2DropItemManager>( L, "DeleteAllItem",							&CX2DropItemManager::DeleteAllItem );
	lua_tinker::class_def<CX2DropItemManager>( L, "GetDropItemPosition",					&CX2DropItemManager::GetDropItemPosition );

	

	lua_tinker::class_add<CX2PVPEmblem>( L, "CX2PVPEmblem" );
	lua_tinker::class_def<CX2PVPEmblem>( L, "AddEmblemData_LUA",							&CX2PVPEmblem::AddEmblemData_LUA );
	


	lua_tinker::class_add<CX2SocketItem>( L, "CX2SocketItem" );
	lua_tinker::class_def<CX2SocketItem>( L, "AddSocketData_LUA",							&CX2SocketItem::AddSocketData_LUA );

#ifdef LUA_TRANS_DEVIDE
	lua_tinker::class_def<CX2SocketItem>( L, "AddSocketDataTrans_LUA",						&CX2SocketItem::AddSocketDataTrans_LUA );
#endif LUA_TRANS_DEVIDE
	
#ifdef SERV_CASH_ITEM_SOCKET_OPTION
	lua_tinker::class_def<CX2SocketItem>( L, "AddSocektGroupDataForCashAvatar_LUA",			&CX2SocketItem::AddSocketGroupDataForCashAvatar_LUA );
#endif // SERV_CASH_ITEM_SOCKET_OPTION

#ifdef SERV_NEW_ITEM_SYSTEM_2013_05 // ������
	lua_tinker::class_def<CX2SocketItem>( L, "AddSkillLevelUpSocketGroup",					&CX2SocketItem::AddSkillLevelUpSocketGroup_LUA );
#endif // SERV_NEW_ITEM_SYSTEM_2013_05

	lua_tinker::class_add<CX2StringFilter>( L, "CXSLStringFilter" );
	lua_tinker::class_def<CX2StringFilter>( L, "AddNickNameFilter",				&CX2StringFilter::AddNickNameFilter_LUA );
	lua_tinker::class_def<CX2StringFilter>( L, "AddNickNameWordFilter",			&CX2StringFilter::AddNickNameWordFilter_LUA );
	lua_tinker::class_def<CX2StringFilter>( L, "AddWordFilter",					&CX2StringFilter::AddWordFilter_LUA );
#ifdef SERV_POST_BAN_WORD_FILTER
	lua_tinker::class_def<CX2StringFilter>( L, "AddPostWordFilter",				&CX2StringFilter::AddPostWordFilter_LUA );
#endif //SERV_POST_BAN_WORD_FILTER

	lua_tinker::class_add<CX2LVUpEventMgr>( L, "CX2LVUpEventMgr" );
	lua_tinker::class_def<CX2LVUpEventMgr>( L, "AddLevelUpEvent_LUA",			&CX2LVUpEventMgr::AddLevelUpEvent_LUA);

	lua_tinker::class_add<CX2StatePVPRoom::CX2MapSelectWindow>( L, "CX2MapSelectWindow" );
	lua_tinker::class_def<CX2StatePVPRoom::CX2MapSelectWindow>( L, "AddMapInfo_LUA",	&CX2StatePVPRoom::CX2MapSelectWindow::AddMapInfo_LUA );

	lua_tinker::class_add<CX2EnchantItem>( L, "CX2EnchantItem" );
	lua_tinker::class_def<CX2EnchantItem>( L, "AddEnchantData_LUA",			&CX2EnchantItem::AddEnchantData_LUA);
	lua_tinker::class_def<CX2EnchantItem>( L, "AddEnchantRequire_LUA",		&CX2EnchantItem::AddEnchantRequire_LUA);

	//{{ 2008. 10. 1  ������	
#ifdef TITLE_SYSTEM
	lua_tinker::class_add<CX2TitleManager>( L, "CX2TitleManager" );
	lua_tinker::class_def<CX2TitleManager>( L, "AddTitleInfo_LUA",			&CX2TitleManager::AddTitleInfo_LUA );	
    lua_tinker::class_def<CX2TitleManager>( L, "AddTitleMissionInfo_LUA",	&CX2TitleManager::AddTitleMissionInfo_LUA );
    lua_tinker::class_def<CX2TitleManager>( L, "AddSubTitleMissionInfo_LUA",	&CX2TitleManager::AddSubTitleMissionInfo_LUA );    
#ifdef LUA_TRANS_DEVIDE
	lua_tinker::class_def<CX2TitleManager>( L, "AddTitleInfoTrans_LUA",			&CX2TitleManager::AddTitleInfoTrans_LUA );	
	lua_tinker::class_def<CX2TitleManager>( L, "AddTitleMissionInfoTrans_LUA",	&CX2TitleManager::AddTitleMissionInfoTrans_LUA );
	lua_tinker::class_def<CX2TitleManager>( L, "AddSubTitleMissionInfoTrans_LUA",	&CX2TitleManager::AddSubTitleMissionInfoTrans_LUA );
#endif LUA_TRANS_DEVIDE
#endif
	//}}

	//{{ kimhc // 2009-10-31 // ����ö
#ifdef	GUILD_MANAGEMENT
	lua_tinker::class_add< CX2GuildManager >( L, "CX2GuildManager" );
	lua_tinker::class_def< CX2GuildManager >( L, "SetLimitGuildLevel",			&CX2GuildManager::SetLimitGuildLevel_LUA );
	lua_tinker::class_def< CX2GuildManager >( L, "AddGuildEXP",					&CX2GuildManager::AddGuildEXP_LUA );
	lua_tinker::class_def< CX2GuildManager >( L, "SetCharLevelFactor",			&CX2GuildManager::EmptyFuncUsedParsing_LUA );
	lua_tinker::class_def< CX2GuildManager >( L, "SetGuildMemberGradeFactor",	&CX2GuildManager::EmptyFuncUsedParsing_LUA );
	lua_tinker::class_def< CX2GuildManager >( L, "SetPcBangFactor",				&CX2GuildManager::EmptyFuncUsedParsing_LUA );
	lua_tinker::class_def< CX2GuildManager >( L, "SetPvpMemberFactor",			&CX2GuildManager::EmptyFuncUsedParsing_LUA );
#endif	GUILD_MANAGEMENT
	//}} kimhc // 2009-10-31 // ����ö

#ifdef SERV_PET_SYSTEM
	lua_tinker::class_add<CX2PET>( L, "CX2PET" );
	lua_tinker::class_def<CX2PET>( L, "AnimEventTimer",			&CX2PET::AnimEventTimer_LUA);	
	lua_tinker::class_def<CX2PET>( L, "GetNowAnimTime",			&CX2PET::GetNowAnimTime_LUA);
	lua_tinker::class_def<CX2PET>( L, "GetLandPosition_LUA",	&CX2PET::GetLandPosition_LUA);	
	lua_tinker::class_def<CX2PET>( L, "GetPos",					&CX2PET::GetPos);
	lua_tinker::class_def<CX2PET>( L, "GetBonePos_LUA",			&CX2PET::GetBonePos_LUA);	
	lua_tinker::class_def<CX2PET>( L, "GetRotateDegree",		&CX2PET::GetRotateDegree);
	lua_tinker::class_def<CX2PET>( L, "IsAttacking",			&CX2PET::IsAttacking);
	lua_tinker::class_def<CX2PET>( L, "GetDistTarget",			&CX2PET::GetDistTarget);
	lua_tinker::class_def<CX2PET>( L, "IsTargetFront",			&CX2PET::IsTargetFront);
	lua_tinker::class_def<CX2PET>( L, "InitOtherTarget",		&CX2PET::InitOtherTarget);
	lua_tinker::class_def<CX2PET>( L, "GetMaster",				&CX2PET::GetMaster);
	lua_tinker::class_def<CX2PET>( L, "GetUIMajorParticle",		&CX2PET::GetUIMajorParticle);
	lua_tinker::class_def<CX2PET>( L, "GetUIEffectSet",			&CX2PET::GetUIEffectSet);	
	lua_tinker::class_def<CX2PET>( L, "SetIsRight",				&CX2PET::SetIsRight);     
	lua_tinker::class_def<CX2PET>( L, "SetStateEffect",			&CX2PET::SetStateEffect);     	
	lua_tinker::class_def<CX2PET>( L, "GetNowStateTimer",		&CX2PET::GetNowStateTimer);  
#ifdef ADD_PET_UNICORN
	lua_tinker::class_def<CX2PET>( L, "GetTargetUnit",			&CX2PET::GetTargetUnit);  
	
#endif
#ifdef ADD_PET_NINE_TAIL_FOX
	lua_tinker::class_def<CX2PET>( L, "SetLockOnDamageEffect",	&CX2PET::SetLockOnDamageEffect);	/// ���� ���� ��ǥ�� ���� ������ ������ ����Ʈ�� �Ͽ� ��Ų��.
	lua_tinker::class_def<CX2PET>( L, "GetRandomInt",			&CX2PET::GetRandomInt);				/// ���� ��ȯ �Լ�
	lua_tinker::class_def<CX2PET>( L, "GetDirVector",			&CX2PET::GetDirVector);				/// �� ���� ���� ��ȯ �Լ�
	lua_tinker::class_def<CX2PET>( L, "GetIsRight",				&CX2PET::GetIsRight);				/// �� �¿��� ���� ��ȯ �Լ�
#endif ADD_PET_NINE_TAIL_FOX

#ifdef ADD_HALLOWEEN_PET_SYSTEM
	lua_tinker::class_def<CX2PET>( L, "GetHitUnitListSize",		&CX2PET::GetHitUnitListSize);		/// �ǰ� ���ּ� ��ȯ
	lua_tinker::class_def<CX2PET>( L, "GetDamageData_LUA",		&CX2PET::GetDamageData_LUA);		/// ������ ������ ��ȯ
	lua_tinker::class_def<CX2PET>( L, "SetDamageData_LUA",		&CX2PET::SetDamageData_LUA);		/// ������ ������ ����
	lua_tinker::class_def<CX2PET>( L, "ShowIncreaseValue",		&CX2PET::ShowIncreaseValue);		/// HP or MP ȸ�� ��ġ ǥ��
#endif ADD_HALLOWEEN_PET_SYSTEM

#ifdef SET_IN_PET_LUA
	lua_tinker::class_def<CX2PET>( L, "SetFlag_LUA",			&CX2PET::SetFlag_LUA);
	lua_tinker::class_def<CX2PET>( L, "GetFlag_LUA",			&CX2PET::GetFlag_LUA);
	lua_tinker::class_def<CX2PET>( L, "SetInt_LUA",				&CX2PET::SetInt_LUA);
	lua_tinker::class_def<CX2PET>( L, "GetInt_LUA",				&CX2PET::GetInt_LUA);
	lua_tinker::class_def<CX2PET>( L, "SetVector_LUA",			&CX2PET::SetVector_LUA);
	lua_tinker::class_def<CX2PET>( L, "GetVector_LUA",			&CX2PET::GetVector_LUA);
	lua_tinker::class_def<CX2PET>( L, "SetNumber_LUA",			&CX2PET::SetNumber_LUA );
	lua_tinker::class_def<CX2PET>( L, "GetNumber_LUA",			&CX2PET::GetNumber_LUA );
#endif SET_IN_PET_LUA


	lua_tinker::class_add<CX2PetManager>( L, "CX2PetManager" );
	lua_tinker::class_def<CX2PetManager>( L, "AddPetTemplet",			&CX2PetManager::AddPetTemplet_LUA);	
#endif

	lua_tinker::class_add<CX2BattleFieldManager>( L, "CX2BattleFieldManager" );
	lua_tinker::class_def<CX2BattleFieldManager>( L, "AddBattleFieldData",	&CX2BattleFieldManager::AddBattleFieldData_LUA );	
	lua_tinker::class_def<CX2BattleFieldManager>( L, "SetMonsterRespawnFactorByUserCount",	&CX2BattleFieldManager::SetMonsterRespawnFactorByUserCount_LUA );	
	lua_tinker::class_def<CX2BattleFieldManager>( L, "SetMonsterRespawnTimeMinMax",			&CX2BattleFieldManager::SetMonsterRespawnTimeMinMax_LUA );	
	lua_tinker::class_def<CX2BattleFieldManager>( L, "SetBattleFieldFactor",				&CX2BattleFieldManager::SetBattleFieldFactor_LUA );	

#ifdef BUFF_TEMPLET_SYSTEM	
	lua_tinker::class_add<CX2PremiumBuffManager>( L, "CX2PremiumBuffManager" );
	lua_tinker::class_def<CX2PremiumBuffManager>( L, "AddBuffInfo",			&CX2PremiumBuffManager::AddBuffInfo_LUA );
#endif BUFF_TEMPLET_SYSTEM

	lua_tinker::class_add<CX2BuffTempletManager>( L, "CX2BuffTempletManager" );
	lua_tinker::class_def<CX2BuffTempletManager>( L, "AddBuffTemplet_LUA",			&CX2BuffTempletManager::AddBuffTemplet_LUA );
	lua_tinker::class_def<CX2BuffTempletManager>( L, "AddBuffFactor_LUA",			&CX2BuffTempletManager::AddBuffFactor_LUA );

#ifdef SERV_QUEST_POSSESSION_BUFF
	lua_tinker::class_def<CX2PremiumBuffManager>( L, "AddEventQuestBuffInfo", &CX2PremiumBuffManager::AddEventQuestBuffInfo_LUA );
#endif //SERV_QUEST_POSSESSION_BUFF

#ifdef CUBE_OPEN_IMAGE_MANAGER
	lua_tinker::class_add<CX2CubeOpenImageManager>( L, "CX2CubeOpenImageManager" );
	lua_tinker::class_def<CX2CubeOpenImageManager>( L, "AddImageInfo_LUA",			&CX2CubeOpenImageManager::AddImageInfo_LUA );
#endif // CUBE_OPEN_IMAGE_MANAGER


#ifdef ADDED_RELATIONSHIP_SYSTEM
	lua_tinker::class_add<CX2WeddingManager>( L, "CX2WeddingManager" );
	lua_tinker::class_def<CX2WeddingManager>( L, "AddOfficiantTemplet",			&CX2WeddingManager::AddOfficiantTemplet_LUA );
	lua_tinker::class_def<CX2WeddingManager>( L, "AddWeddingHallTemplet",		&CX2WeddingManager::AddWeddingHallTemplet_LUA );
#endif // ADDED_RELATIONSHIP_SYSTEM

#ifdef FIX_ICE_HEATER_EVENT
	lua_tinker::class_add<CX2UIInventory>( L, "CX2UIInventory" );
	lua_tinker::class_def<CX2UIInventory>( L, "AddIceHeaterEventInfo",			&CX2UIInventory::AddIceHeaterEventInfo_LUA );
#endif // FIX_ICE_HEATER_EVENT
}

bool CX2Main::OpenScriptFile( const WCHAR* pFileName, KLuabinder* pKLuabinder )
{
	if ( pKLuabinder == NULL )
		pKLuabinder = g_pKTDXApp->GetLuaBinder();

	lua_tinker::decl( pKLuabinder->GetLuaState(),  "g_pMain", this );

	KGCMassFileManager::CMassFile::MASSFILE_MEMBERFILEINFO_POINTER Info;
	Info = g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LoadDataFile( pFileName );
	if( Info == NULL )
	{
		string strFileName;
		ConvertWCHARToChar( strFileName, pFileName );
		ErrorLogMsg( XEM_ERROR13, strFileName.c_str() );
		return false;
	}

	if( pKLuabinder->DoMemory( Info->pRealData, Info->size ) == E_FAIL )
	{
		string strFileName;
		ConvertWCHARToChar( strFileName, pFileName );
		ErrorLogMsg( XEM_ERROR14, strFileName.c_str() );

		return false;
	}

	return true;
}

bool CX2Main::StateChange( int stateID, int iDetailStateID )
{
	

	if ( stateID > XS_INVALID && stateID < XS_END )
	{


#ifdef EVENT_SCENE
		if(g_pData->GetEventScene() != NULL &&
			g_pData->GetEventScene()->GetIsPlayingEventScene() == true)
		{
			g_pData->GetEventScene()->EndEventScene(true);
		}
#endif EVENT_SCENE
		g_pKTDXApp->RemoveStage( m_pNowState, true );
		m_pNowState = NULL; 

#ifdef	X2OPTIMIZE_MASS_FILE_BUFFER_MANAGER
		if ( g_pKTDXApp->GetDeviceManager() != NULL )
			g_pKTDXApp->GetDeviceManager()->ReleaseAllMemoryBuffers();
#endif	X2OPTIMIZE_MASS_FILE_BUFFER_MANAGER

		m_NowStateID = (X2_STATE)stateID;
		StateInLog();
		AllDeleteServerPacket();
#ifdef DOWN_FRAME_TEST
		g_pKTDXApp->SetFrameRate( NUM_TEST_FRAME );
#else  DOWN_FRAME_TEST
		g_pKTDXApp->SetFrameRate( 75.f );
#endif DOWN_FRAME_TEST

		//{{ ����� : [2009/7/21] //	���ҵ� �ް��� â ��Ȱ��ȭ
#ifdef NASOD_SCOPE
		if(g_pChatBox != NULL)
		{
			g_pChatBox->ShowNasodMessageDlg( false );
		}
#endif
		//}} ����� : [2009/7/21] //	���ҵ� �ް��� â ��Ȱ��ȭ


#ifdef ADDED_RELATIONSHIP_SYSTEM
		if ( NULL != g_pData->GetRelationshipManager() )
		{
			g_pData->GetRelationshipManager()->ResetRelationPartnerInVillage();
			g_pData->GetRelationshipManager()->ResetRelationPartnerInGame();
		}
#endif // ADDED_RELATIONSHIP_SYSTEM

#ifdef NEW_EMBLEM_MANAGER
		CX2EmblemManager::GetInstance()->InitEmbelmPlayInfo();
#endif //NEW_EMBLEM_MANAGER

		switch( stateID )
		{
			case XS_START_UP:
				{
					m_pNowState = new CX2StateStartUp();
				}
				break;
			case XS_LOGIN:
				{
					m_pNowState = new CX2StateLogin();
				}
				break;

			case XS_JOIN:
				{
					m_pNowState = new CX2StateJoin();
				}
				break;

			case XS_LOADING:
				{
 					m_pNowState = new CX2StateLoading();
				}
				break;

#ifdef ELSWORD_NEW_BEGINNING
			case XS_BEGINNING:
				{
					m_pNowState = new CX2StateBeginning;					
				} 
				break;
#else
			case XS_SERVER_SELECT:
				{
#ifdef HEAP_BROKEN_BY_ROOM
					g_pData->DeleteAllRooms();
#endif // HEAP_BROKEN_BY_ROOM
					m_pNowState = new CX2StateServerSelect;					
				} 
				break;
#endif ELSWORD_NEW_BEGINNING

#ifndef ELSWORD_NEW_BEGINNING
			case XS_CREATE_UNIT:
				{
					m_pNowState = new CX2StateCreateUnit();
				}
				break;
#endif ELSWORD_NEW_BEGINNING

			case XS_PVP_LOBBY:
				{
#ifdef HEAP_BROKEN_BY_ROOM
					g_pData->DeleteAllRooms();
#endif // HEAP_BROKEN_BY_ROOM

					SAFE_DELETE(g_pTFieldGame);
					m_pNowState = new CX2StatePvPLobby();
				}
				break;

			case XS_PVP_ROOM:
				{
					m_pNowState = new CX2StatePVPRoom();
				}
				break;

			case XS_PVP_GAME:
				{
#ifdef OPTIMIZED_P2P
#ifdef DOWN_FRAME_TEST
					g_pKTDXApp->SetFrameRate( NUM_TEST_FRAME );
#else  DOWN_FRAME_TEST
                    g_pKTDXApp->SetFrameRate( 85.f );
#endif DOWN_FRAME_TEST
#endif OPTIMIZED_P2P
					
#ifdef HEAP_BROKEN_BY_ROOM
					g_pData->DeleteBattleFieldRoom();
					g_pData->DeleteDungeonRoom();	/// �̰� ��� �ʿ� ���� �� ������...

					if ( NULL == g_pData->GetPVPRoom() )
						g_pData->ResetPVPRoom();
					
					if ( NULL != g_pX2Room && CX2Room::IsNewDataSet() )		/// ���ο� �����Ͱ� ���� �Ǿ�����
						g_pX2Room->ApplyRoomPacketData();
#endif // HEAP_BROKEN_BY_ROOM

					m_pNowState = new CX2StatePVPGame();
				}
				break;

			case XS_PVP_RESULT:
				{
					m_pNowState = new CX2StatePVPResult();
				}
				break;

			case XS_DUNGEON_RESULT:
				{
					m_pNowState = new CX2StateDungeonResult();
				}
				break;

			case XS_VILLAGE_MAP:
				{

#ifdef ELSWORD_NEW_BEGINNING
					SAFE_DELETE(g_pBeginningGame);
#endif ELSWORD_NEW_BEGINNING

					//g_pData->GetEventScene()->BlackLayOutMove( false, 1.0f, false, true, 3.0f, 1.0f );

					g_pKTDXApp->SetFrameRate( 40.f );
					
#ifdef HEAP_BROKEN_BY_ROOM
					g_pData->DeleteAllRooms();
#endif // HEAP_BROKEN_BY_ROOM
					
					m_pNowState = new CX2StateField();

					int iMapId = g_pData->GetLocationManager()->GetCurrentVillageID();
					CX2World::WORLD_ID	eWorldId;
					SEnum::VILLAGE_MAP_ID villageId;
					D3DXVECTOR3 vPos;	

// 					if(g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetSelectUnit() != NULL)
// 						iMapId = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_nMapID;
					
					if(g_pData->GetLocationManager()->GetVillageMapTemplet((SEnum::VILLAGE_MAP_ID)iMapId) == NULL)
					{
						// �߸��� ������ �̵��� �ʱ�ȭ
						KEGS_STATE_CHANGE_FIELD_REQ kPacket;					

						iMapId = SEnum::VMI_RUBEN;
						kPacket.m_iMapID =  (int)iMapId;
						

						D3DXVECTOR3 vStartPos = g_pData->GetLocationManager()->GetStartPosLoc( 1 );	// �纥������ ������ ���� ��ġ
						g_pData->GetLocationManager()->SetLastPos( vStartPos );


						g_pData->GetLocationManager()->SetVillage( (SEnum::VILLAGE_MAP_ID)kPacket.m_iMapID, vStartPos );

						g_pData->GetServerProtocol()->SendPacket( EGS_STATE_CHANGE_FIELD_REQ, kPacket );
						g_pMain->AddServerPacket( EGS_STATE_CHANGE_FIELD_ACK );

						//SAFE_DELETE_DIALOG( m_pDLGMsgBox );
						//m_pDLGMsgBox = g_pMain->KTDGUIMsgBox( D3DXVECTOR2(250,300), L"������ �̵��ϰ� �ֽ��ϴ�.", this );	
					}	
					else
					{					
						g_pData->GetLocationManager()->GetVillage(villageId, vPos);
						if( true == g_pData->GetLocationManager()->IsDungeonLounge( villageId ) )
						{
							eWorldId = g_pInstanceData->GetDungeonLoungeWorldID();
						}
						else
						{
							eWorldId = (CX2World::WORLD_ID)g_pData->GetLocationManager()->GetWorldID(villageId);	
						}


						SAFE_DELETE(g_pTFieldGame);
						g_pTFieldGame = new CX2TFieldGame(eWorldId);

						if(g_pTFieldGame != NULL)
						{
							g_pTFieldGame->AddUnit( g_pData->GetMyUser()->GetSelectUnit(), true, true );
							g_pTFieldGame->SetAddedMyUnit(true);					
						}					
					}	

					if( NULL != g_pData && NULL != g_pData->GetUIManager() )
						g_pData->GetUIManager()->SetShowUI(true, true);				

				}
				break;

	

			case XS_DUNGEON_ROOM:
				{
					m_pNowState = new CX2StateDungeonRoom();
				} 
				break;

			case XS_DUNGEON_GAME:
				{
#ifdef DOWN_FRAME_TEST
					g_pKTDXApp->SetFrameRate( NUM_TEST_FRAME );
#else  DOWN_FRAME_TEST
					g_pKTDXApp->SetFrameRate( 70.f );
#endif DOWN_FRAME_TEST

#ifdef HEAP_BROKEN_BY_ROOM
					g_pData->DeleteBattleFieldRoom();
					g_pData->DeletePVPRoom();	/// �̰� ��� �ʿ� ���� �� ������...

					if ( NULL == g_pData->GetDungeonRoom() )
						g_pData->ResetDungeonRoom();

					if ( NULL != g_pX2Room && CX2Room::IsNewDataSet() )		/// ���ο� �����Ͱ� ���� �Ǿ�����
						g_pX2Room->ApplyRoomPacketData();
#endif // HEAP_BROKEN_BY_ROOM

					m_pNowState = new CX2StateDungeonGame();
				}
				break;


			case XS_SQUARE_GAME:
				{
					g_pKTDXApp->SetFrameRate( 40.f );

					SAFE_DELETE(g_pTFieldGame);
					m_pNowState = new CX2StateSquareGame();
				}
				break;

			case XS_TRAINING_SCHOOL:
				{
					m_pNowState = new CX2StateTrainingSchool();					
				}
				break;
			case XS_TRAINING_GAME:
				{
					m_pNowState = new CX2StateTrainingGame();
				}
				break;
				
			case XS_TRAINING_RESULT:
				{
					m_pNowState = new CX2StateTrainingResult();
				}
				break;				
			case XS_BATTLE_FIELD:
				{
					g_pKTDXApp->SetFrameRate( 72.f );
					
#ifdef HEAP_BROKEN_BY_ROOM
					g_pData->DeleteAllRooms();
					g_pData->ResetBattleFieldRoom();
					if ( NULL != g_pX2Room && CX2Room::IsNewDataSet() )		/// ���ο� �����Ͱ� ���� �Ǿ�����
						g_pX2Room->ApplyRoomPacketData();
#endif // HEAP_BROKEN_BY_ROOM

					if( NULL != g_pData && NULL != g_pData->GetUIManager() )
						g_pData->GetUIManager()->SetShowUI(true, true);		

					m_pNowState = new CX2StateBattleField();
				} break;

#ifdef ADDED_RELATIONSHIP_SYSTEM // ���� ��ü�� �����������..Ȯ���ʿ�.
			case XS_WEDDING_GAME:
				{
					g_pKTDXApp->SetFrameRate( 40.f );
					//SAFE_DELETE(g_pTFieldGame);
					m_pNowState = new CX2StateWeddingGame();

					if( NULL != g_pData && NULL != g_pData->GetUIManager() )
						g_pData->GetUIManager()->SetShowUI(false, true);

				} break;
#endif // ADDED_RELATIONSHIP_SYSTEM
			default:
				{
					StateLog( "This state does not exist" );
					ASSERT( !"This state does not exist" );
				} break;
		}

		g_pKTDXApp->GetDGManager()->GetDialogManager()->SetStage( m_pNowState );
		g_pKTDXApp->InsertStage( m_pNowState );

		CX2GageManager::StateChange( stateID );
        //{{ seojt // 2008-10-21, 18:08
        // ���� ��ȭ�� ���� �� g_pKTDXApp->PostChangeState()�� ȣ���Ѵ�.
        //g_pKTDXApp->PostChangeState( m_pNowState );
        //}} seojt // 2008-10-21, 18:08

		PostStateChange();

#ifdef	X2OPTIMIZE_MASS_FILE_BUFFER_MANAGER
		if ( g_pKTDXApp->GetDeviceManager() != NULL )
			g_pKTDXApp->GetDeviceManager()->ReleaseAllMemoryBuffers();
#endif	X2OPTIMIZE_MASS_FILE_BUFFER_MANAGER

#ifdef SEND_NEXON_WISE_LOG_BY_URL // ����ȯ
		if( false == m_BrowserWrapper.IsClosed() )	/// �������� ����������, ����
			m_BrowserWrapper.CloseBrowser();
#endif // SEND_NEXON_WISE_LOG_BY_URL

		return true;
	}
	else
	{
		return false;
	}
}

void CX2Main::PostStateChange()
{
#ifdef NEW_VILLAGE_UI

	// 08.12.14 ���¿� : State Change�ÿ� ���� �ִ� ��� UI ����
	if ( NULL != g_pData->GetUIManager() )
	{
		CX2UIManager* pUIManager = g_pData->GetUIManager();
		pUIManager->OnStateChange();

		//{{ kimhc	// �ǽð� ���ҵ� �� �ǽð� ������ ȹ�� ���� �ӽ� �κ��丮
#ifdef	REAL_TIME_ELSWORD
		if ( pUIManager->GetUITempInventory() != NULL &&
			pUIManager->GetUITempInventory()->GetNumberOfItem() > 0 )
			pUIManager->ToggleUI( CX2UIManager::UI_TEMP_INVENTORY, true );
#endif	REAL_TIME_ELSWORD
		//}} kimhc	// �ǽð� ���ҵ� �� �ǽð� ������ ȹ�� ���� �ӽ� �κ��丮
	}
#endif


#ifdef ITEM_CHEAT_POPUP_TEST
	g_pInstanceData->GetItemCheatPopup().SetStage( (CKTDXStage*) GetNowState() );
	if( true == g_pInstanceData->GetItemCheatPopup().IsOpened() )
	{
		g_pInstanceData->GetItemCheatPopup().OpenItemList( false );
	}
#endif ITEM_CHEAT_POPUP_TEST


	ReservedServerEventProc();

	// ��Ƽ UI ���� ����Ÿ ����
	if( NULL != g_pMain->GetPartyUI() )
	{
		g_pMain->GetPartyUI()->OnChangeState();
	}

#ifndef NEW_SKILL_TREE
	if( NULL != g_pInstanceData->GetSkillTreeUI() )
	{
		g_pInstanceData->GetSkillTreeUI()->SetStage( (CKTDXStage*) GetNowState() );
	}
#endif

	if( NULL != g_pInstanceData->GetMiniMapUI() )
	{
		g_pInstanceData->GetMiniMapUI()->SetStage( (CKTDXStage*) GetNowState() );

		switch ( GetNowStateID() )
		{
		case CX2Main::XS_DUNGEON_GAME:
		case CX2Main::XS_SQUARE_GAME:
		case CX2Main::XS_VILLAGE_MAP:
		case CX2Main::XS_BATTLE_FIELD:
			{
			} break;
		default:
			{
				g_pInstanceData->GetMiniMapUI()->SetShowMiniMap( CX2MiniMapUI::MMT_DUNGEON, false );
			}
			break;
		}
	}



	if( true == g_pData->GetClassChangePopup().GetShow() )
	{
		g_pData->GetClassChangePopup().SetShow( false );
	}

#ifdef XMAS_UI
	switch( GetNowStateID() )
	{
	case CX2Main::XS_VILLAGE_MAP:
	case CX2Main::XS_SQUARE_GAME:
		{
			g_pMain->GetMemoryHolder()->GetXMasDLG((CKTDXStage*) GetNowState())->SetShow(true);
		} break;
	default:
		{
			g_pMain->GetMemoryHolder()->GetXMasDLG((CKTDXStage*) GetNowState())->SetShow(false);
		} break;
	}
#endif

#ifdef SERV_CHINA_SPIRIT_EVENT
	switch( GetNowStateID() )
	{
	case CX2Main::XS_DUNGEON_GAME:
	case CX2Main::XS_SQUARE_GAME:
	case CX2Main::XS_VILLAGE_MAP:
	case CX2Main::XS_BATTLE_FIELD:
		{
			g_pMain->GetMemoryHolder()->GetUseSpiritEventDLG((CKTDXStage*) GetNowState(), true);
		}break;
	default:
		{
			g_pMain->GetMemoryHolder()->GetUseSpiritEventDLG((CKTDXStage*) GetNowState(), false);
		}break;
	}
#endif SERV_CHINA_SPIRIT_EVENT

#ifdef SERV_NEW_YEAR_EVENT_2014
	switch( GetNowStateID() )
	{
	case CX2Main::XS_VILLAGE_MAP:
	case CX2Main::XS_BATTLE_FIELD:
		{
			g_pMain->GetMemoryHolder()->GetNewYear2014EventDLG( (CKTDXStage*)GetNowState() );
			g_pMain->GetMemoryHolder()->SetShowNewYear2014Event( true );
			g_pMain->GetMemoryHolder()->SetShowNewYear2014EventBoard( false );
		}break;
	default:
		{
			g_pMain->GetMemoryHolder()->GetNewYear2014EventDLG( (CKTDXStage*)GetNowState() );
			g_pMain->GetMemoryHolder()->SetShowNewYear2014Event( false );
		}break;
	}
#endif SERV_NEW_YEAR_EVENT_2014
}



void CX2Main::ReservedServerEventProc()
{
	for( int i=0; i<(int)m_vecReservedServerPacket.size(); i++ )
	{
		ReservedServerPacket& reservedServerPacket = m_vecReservedServerPacket[i];
		switch( reservedServerPacket.m_eServerEventID )
		{
		case EGS_EXPIRE_CASH_SKILL_POINT_NOT:
			{
				switch( GetNowStateID() )
				{
				case CX2Main::XS_DUNGEON_GAME:
				case CX2Main::XS_DUNGEON_RESULT:
				case CX2Main::XS_PVP_GAME:
				case CX2Main::XS_PVP_RESULT:
					{
						continue;	// continue for loop
					} break;
				}


				KEGS_EXPIRE_CASH_SKILL_POINT_NOT* pEvent = (KEGS_EXPIRE_CASH_SKILL_POINT_NOT*) reservedServerPacket.m_pEvent;
				if( NULL != g_pData->GetSkillTree() )
				{
					g_pData->GetSkillTree()->On_EGS_EXPIRE_CASH_SKILL_POINT_NOT( *pEvent );
				}

				SAFE_DELETE( pEvent );
				m_vecReservedServerPacket.erase( m_vecReservedServerPacket.begin() + i );
				i--;
			} break;
//{{ oasis907 : ����� [2009.12.17] //
		case EGS_INVITE_GUILD_NOT:
			{
				switch( GetNowStateID() )
				{
				case CX2Main::XS_DUNGEON_GAME:
				case CX2Main::XS_DUNGEON_RESULT:
				case CX2Main::XS_PVP_GAME:
				case CX2Main::XS_PVP_RESULT:
					{
						continue;	// continue for loop
					} break;
				}



				KEGS_INVITE_GUILD_NOT* pEvent = (KEGS_INVITE_GUILD_NOT*) reservedServerPacket.m_pEvent;
				g_pData->GetGuildManager()->Handler_RESERVED_EGS_INVITE_GUILD_NOT( *pEvent );
				SAFE_DELETE( pEvent );
				m_vecReservedServerPacket.erase( m_vecReservedServerPacket.begin() + i );
				i--;
			} break;
//}} oasis907 : ����� [2009.12.17] //
		}
	}
}

bool CX2Main::DeleteFile( const WCHAR* pFileName )
{
	WCHAR wszFileName[MAX_PATH] = {0};

	wcscpy( wszFileName, pFileName );

	g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->GetPullPath( wszFileName );
	if ( wszFileName != NULL )
	{
		::DeleteFile( wszFileName );
		return true;
	}

	return false;
}



#ifdef ADDED_MESSAGEBOX_USING_CUSTOM_LUA
CKTDGUIDialogType CX2Main::KTDGUIOKMsgBox( D3DXVECTOR2 pos, const WCHAR* pText, CKTDXStage* pNowState, int iOKMsg /*= -1*/, float fTimeLeft /*= -1.f*/, wstring wstrFileName /* = L""  */ )
#else // ADDED_MESSAGEBOX_USING_CUSTOM_LUA
CKTDGUIDialogType CX2Main::KTDGUIOKMsgBox( D3DXVECTOR2 pos, const WCHAR* pText, CKTDXStage* pNowState, int iOKMsg /*= -1*/, float fTimeLeft /*= -1.f*/ )
#endif // ADDED_MESSAGEBOX_USING_CUSTOM_LUA
{
	if( pNowState == NULL )
		pNowState = m_pNowState;

#ifdef DIALOG_SHOW_TOGGLE
	if( g_pKTDXApp->GetDGManager()->GetDialogManager()->GetHideDialog() == true )
	{
		g_pKTDXApp->GetDGManager()->GetDialogManager()->SetShow( true );

		bool bHide = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetHideDialog();
		if( g_pInstanceData->GetMiniMapUI() != NULL )
		{				
			g_pInstanceData->GetMiniMapUI()->SetShowMiniMap( g_pInstanceData->GetMiniMapUI()->GetCurrentMiniMapType(), !bHide );
		}
		if( g_pInstanceData->GetMiniMapUI() != NULL && g_pInstanceData->GetMiniMapUI()->GetWorldMapUI() != NULL )
		{
			g_pInstanceData->GetMiniMapUI()->GetWorldMapUI()->OpenWorldMap(false);
			g_pInstanceData->GetMiniMapUI()->GetWorldMapUI()->OpenFieldMap(false);				
		}
	}
#endif

	DialogLog( pText );


#ifdef ADDED_MESSAGEBOX_USING_CUSTOM_LUA
	CKTDGUIDialogType pDialog = NULL;
	if ( wstrFileName != L"")
	{
		pDialog = new CKTDGUIDialog( pNowState, wstrFileName.c_str(), 0.07f, XDL_POP_UP );
		if ( pDialog == NULL )
		{
			ASSERT ( !L"pDialog Is Nothing" );
			pDialog = new CKTDGUIDialog( pNowState,  L"DLG_UI_OKMsgBox.lua", 0.07f, XDL_POP_UP );
		}
	}
	else
	{
		pDialog = new CKTDGUIDialog( pNowState,  L"DLG_UI_OKMsgBox.lua", 0.07f, XDL_POP_UP );
	}
#else // ADDED_MESSAGEBOX_USING_CUSTOM_LUA
	//g_pKTDXApp->GetDIManager()->SetEnable(false);
#ifdef NEW_VILLAGE_UI
	CKTDGUIDialogType pDialog = new CKTDGUIDialog( pNowState, L"DLG_UI_OKMsgBox.lua", 0.07f, XDL_POP_UP );
#else
	CKTDGUIDialogType pDialog = new CKTDGUIDialog( pNowState, L"DLG_OKMsgBox.lua", 0.07f, XDL_POP_UP );
#endif

#endif // ADDED_MESSAGEBOX_USING_CUSTOM_LUA
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( pDialog );

	if ( pos.x == -999 && pos.y == -999 )
	{
		pos = GetWindowPos( D3DXVECTOR2( 430, 148 ), D3DXVECTOR2( 216, 77 ) );
	}

	//debugPrintf( "%f %f", pos.x, pos.y );
	pos.y -= 50.f;
	pDialog->SetPos( pos );
	CKTDGUIStatic* pStatic = (CKTDGUIStatic*)pDialog->GetControl( L"StaticOKMsgBoxBG" );
	if( NULL == pStatic )
	{
#ifdef ADDED_MESSAGEBOX_USING_CUSTOM_LUA
		ASSERT( !L"pStatic Is Nothing");
#endif // ADDED_MESSAGEBOX_USING_CUSTOM_LUA

		SAFE_DELETE_DIALOG( pDialog );
		return NULL;
	}

	wstring tempText = pText;

	int lineNum = 1;

	const int constTextMaxLen = (int)(g_pKTDXApp->GetResolutionScaleX() * 366);
	//bool checkEnterChar = false;
	int nowTextLen = 0;

	CKTDGFontManager::CUKFont* pFont = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetUKFont( pStatic->GetString(0)->fontIndex );
	if( pFont == NULL )
	{
		SAFE_DELETE_DIALOG( pDialog );
		return NULL;
	}
#ifdef CLIENT_GLOBAL_LINEBREAK
	lineNum = CWordLineHandler::LineBreakInX2MainMsgBox( tempText, pFont, constTextMaxLen );
#else //CLIENT_GLOBAL_LINEBREAK
//	CKTDGUIUniBuffer uniBuffer;
//	uniBuffer.SetFontNode( pFont );

	for ( int i = 0; i < (int)tempText.size(); i++ )
	{
		WCHAR tempWchar = tempText[i];
		if ( nowTextLen < constTextMaxLen && tempWchar == '\n' )
		{
			if ( i != (int)tempText.size() - 1 )
			{
				lineNum++;
			}

			nowTextLen = 0;
			continue;
		}
		int iUniCharSize = 0;
//		uniBuffer.Clear();
//		uniBuffer.InsertChar( 0, tempWchar );
//		uniBuffer.CPtoX(uniBuffer.GetTextSize(), FALSE, &iUniCharSize);

		iUniCharSize = pFont->GetWidth( tempWchar );
		nowTextLen += iUniCharSize;

		if ( nowTextLen >= constTextMaxLen )
		{
			if ( i != (int)tempText.size() - 1 )
			{
				lineNum++;
			}

			nowTextLen = 0;
			wstring enterChar = L"\n";
			tempText.insert( tempText.begin() + i, enterChar.begin(), enterChar.end() );
			i++;
		}

		if ( i >= (int)tempText.size() )
		{
			break;
		}
	}
#endif //CLIENT_GLOBAL_LINEBREAK

	pStatic->GetString(0)->msg = tempText;

	if ( lineNum == 1 )
	{
		pStatic->GetString(0)->pos.y += pFont->GetHeight();
	}
	else if ( lineNum == 2 )
	{
		pStatic->GetString(0)->pos.y += pFont->GetHeight();	
	}
	else if ( lineNum == 3 )
	{
		pStatic->GetString(0)->pos.y += pFont->GetHeight()/2.0f;
	}

	CKTDGUIButton* pButtonOK = (CKTDGUIButton*) pDialog->GetControl( L"ButtonMsgBoxOK" );
	if( NULL != pButtonOK )
	{
		if( -1 != iOKMsg )
		{
			pButtonOK->SetCustomMsgMouseUp( iOKMsg );
		}
		pButtonOK->RequestFocus();
	}
#ifdef ADDED_MESSAGEBOX_USING_CUSTOM_LUA
	else
	{
		if ( NULL != pButtonOK )	
			ASSERT( !L"ButtonMsgBoxOK Is Nothing" );
	}
#endif // ADDED_MESSAGEBOX_USING_CUSTOM_LUA
	
	
	if( fTimeLeft > 0.f )
	{
		pDialog->Move( pDialog->GetPos(), D3DXCOLOR( 1, 1, 1, 1 ), fTimeLeft, false, true );
	}

	return pDialog;
}


CKTDGUIDialogType CX2Main::KTDGUIOKMsgBoxPlus( D3DXVECTOR2 pos, const WCHAR* pText, CKTDXStage* pNowState, int iOKMsg /*= -1*/, float fTimeLeft /*= -1.f*/ )
{
	if( pNowState == NULL )
		pNowState = m_pNowState;

#ifdef DIALOG_SHOW_TOGGLE
	if( g_pKTDXApp->GetDGManager()->GetDialogManager()->GetHideDialog() == true )
	{
		g_pKTDXApp->GetDGManager()->GetDialogManager()->SetShow( true );

		bool bHide = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetHideDialog();
		if( g_pInstanceData->GetMiniMapUI() != NULL )
		{				
			g_pInstanceData->GetMiniMapUI()->SetShowMiniMap( g_pInstanceData->GetMiniMapUI()->GetCurrentMiniMapType(), !bHide );
		}
		if( g_pInstanceData->GetMiniMapUI() != NULL && g_pInstanceData->GetMiniMapUI()->GetWorldMapUI() != NULL )
		{
			g_pInstanceData->GetMiniMapUI()->GetWorldMapUI()->OpenWorldMap(false);
			g_pInstanceData->GetMiniMapUI()->GetWorldMapUI()->OpenFieldMap(false);				
		}
	}
#endif

	DialogLog( pText );

	//g_pKTDXApp->GetDIManager()->SetEnable(false);
#ifdef NEW_VILLAGE_UI
	CKTDGUIDialogType pDialog = new CKTDGUIDialog( pNowState, L"DLG_UI_OKMsgBoxPlus.lua", 0.07f, XDL_POP_UP );
#else
	CKTDGUIDialogType pDialog = new CKTDGUIDialog( pNowState, L"DLG_OKMsgBox.lua", 0.07f, XDL_POP_UP );
#endif
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( pDialog );


	if ( pos.x == -999 && pos.y == -999 )
	{
		pos = GetWindowPos( D3DXVECTOR2( 430, 148 ), D3DXVECTOR2( 216, 77 ) );
	}

	//debugPrintf( "%f %f", pos.x, pos.y );
	pos.y -= 50.f;
	pDialog->SetPos( pos );
	CKTDGUIStatic* pStatic = (CKTDGUIStatic*)pDialog->GetControl( L"StaticOKMsgBoxBG" );
	if( NULL == pStatic )
	{
		SAFE_DELETE_DIALOG( pDialog );
		return NULL;
	}

	wstring tempText = pText;

	int lineNum = 1;

	const int constTextMaxLen = (int)(g_pKTDXApp->GetResolutionScaleX() * 366);
	//bool checkEnterChar = false;
	int nowTextLen = 0;

	CKTDGFontManager::CUKFont* pFont = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetUKFont( pStatic->GetString(0)->fontIndex );
	if( pFont == NULL )
	{
		SAFE_DELETE_DIALOG( pDialog );
		return NULL;
	}
	//	CKTDGUIUniBuffer uniBuffer;
	//	uniBuffer.SetFontNode( pFont );

	//{{ 2011/1/12	������	WordWrap(�ٹٲ�) �߰� - ���� �ҽ� ���� 
#ifdef CLIENT_GLOBAL_LINEBREAK
	lineNum = CWordLineHandler::LineBreakInX2MainMsgBox( tempText, pFont, constTextMaxLen );
#else
	for ( int i = 0; i < (int)tempText.size(); i++ )
	{
		WCHAR tempWchar = tempText[i];
		if ( nowTextLen < constTextMaxLen && tempWchar == '\n' )
		{
			if ( i != (int)tempText.size() - 1 )
			{
				lineNum++;
			}

			nowTextLen = 0;
			continue;
		}
		int iUniCharSize = 0;
		//		uniBuffer.Clear();
		//		uniBuffer.InsertChar( 0, tempWchar );
		//		uniBuffer.CPtoX(uniBuffer.GetTextSize(), FALSE, &iUniCharSize);

		iUniCharSize = pFont->GetWidth( tempWchar );
		nowTextLen += iUniCharSize;

		if ( nowTextLen >= constTextMaxLen )
		{
			if ( i != (int)tempText.size() - 1 )
			{
				lineNum++;
			}

			nowTextLen = 0;
			wstring enterChar = L"\n";
			tempText.insert( tempText.begin() + i, enterChar.begin(), enterChar.end() );
			i++;
		}

		if ( i >= (int)tempText.size() )
		{
			break;
		}
	}
#endif CLIENT_GLOBAL_LINEBREAK

	pStatic->GetString(0)->msg = tempText;

	if ( lineNum == 1 )
	{
		pStatic->GetString(0)->pos.y += pFont->GetHeight();
	}
	else if ( lineNum == 2 )
	{
		pStatic->GetString(0)->pos.y += pFont->GetHeight();	
	}
	else if ( lineNum == 3 )
	{
		pStatic->GetString(0)->pos.y += pFont->GetHeight()/2.0f;
	}

	CKTDGUIButton* pButtonOK = (CKTDGUIButton*) pDialog->GetControl( L"ButtonMsgBoxOK" );
	if( NULL != pButtonOK )
	{
		if( -1 != iOKMsg )
		{
			pButtonOK->SetCustomMsgMouseUp( iOKMsg );
		}
		pButtonOK->RequestFocus();
	}


	if( fTimeLeft > 0.f )
	{
		pDialog->Move( pDialog->GetPos(), D3DXCOLOR( 1, 1, 1, 1 ), fTimeLeft, false, true );
	}

	return pDialog;
}

#ifdef ADDED_MESSAGEBOX_USING_CUSTOM_LUA
CKTDGUIDialogType CX2Main::KTDGUIMsgBox( D3DXVECTOR2 pos, const WCHAR* pText, CKTDXStage* pNowState, wstring wstrFileName /* = L"" */ )
#else // ADDED_MESSAGEBOX_USING_CUSTOM_LUA
CKTDGUIDialogType CX2Main::KTDGUIMsgBox( D3DXVECTOR2 pos, const WCHAR* pText, CKTDXStage* pNowState )
#endif // ADDED_MESSAGEBOX_USING_CUSTOM_LUA
{
	if( pNowState == NULL )
		pNowState = m_pNowState;

#ifdef DIALOG_SHOW_TOGGLE
	if( g_pKTDXApp->GetDGManager()->GetDialogManager()->GetHideDialog() == true )
	{
		g_pKTDXApp->GetDGManager()->GetDialogManager()->SetShow( true );

		bool bHide = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetHideDialog();
		if( g_pInstanceData->GetMiniMapUI() != NULL )
		{				
			g_pInstanceData->GetMiniMapUI()->SetShowMiniMap( g_pInstanceData->GetMiniMapUI()->GetCurrentMiniMapType(), !bHide );
		}
		if( g_pInstanceData->GetMiniMapUI() != NULL && g_pInstanceData->GetMiniMapUI()->GetWorldMapUI() != NULL )
		{
			g_pInstanceData->GetMiniMapUI()->GetWorldMapUI()->OpenWorldMap(false);
			g_pInstanceData->GetMiniMapUI()->GetWorldMapUI()->OpenFieldMap(false);				
		}
	}
#endif

	DialogLog( pText );

#ifdef ADDED_MESSAGEBOX_USING_CUSTOM_LUA
	CKTDGUIDialogType pDialog = NULL;

	if ( wstrFileName != L"")
	{
		pDialog = new CKTDGUIDialog( pNowState, wstrFileName.c_str(), 0.07f, XDL_POP_UP );
		ASSERT(!L"pDialog Is Nothing");
		pDialog = new CKTDGUIDialog( pNowState,  L"DLG_UI_MsgBox.lua", 0.07f, XDL_POP_UP );
	}
	else
	{
		pDialog = new CKTDGUIDialog( pNowState,  L"DLG_UI_MsgBox.lua", 0.07f, XDL_POP_UP );
	}
#else // ADDED_MESSAGEBOX_USING_CUSTOM_LUA
#ifdef NEW_VILLAGE_UI
	CKTDGUIDialogType pDialog = new CKTDGUIDialog( pNowState, L"DLG_UI_MsgBox.lua", 0.07f, XDL_POP_UP );
#else
	CKTDGUIDialogType pDialog = new CKTDGUIDialog( pNowState, L"DLG_MsgBox.lua", 0.07f, XDL_POP_UP );
#endif

#endif // ADDED_MESSAGEBOX_USING_CUSTOM_LUA
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( pDialog );
	pDialog->SetColor( D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f) );
	pos.y -= 50.f;
	pDialog->SetPos( pos );
	CKTDGUIStatic* pStatic = (CKTDGUIStatic*)pDialog->GetControl( L"StaticMsgBoxBG" );

#ifdef ADDED_MESSAGEBOX_USING_CUSTOM_LUA

	if ( NULL == pStatic )
		ASSERT( !L"StaticMsgBoxBG Is Nothing" );

#endif // ADDED_MESSAGEBOX_USING_CUSTOM_LUA

	wstring tempText = pText;

	const int constTextMaxLen = (int)(g_pKTDXApp->GetResolutionScaleX() * 366);


	//bool checkEnterChar = false;
	int nowTextLen = 0;

//	CKTDGFontManager::CKTDGFont* pFont = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetFont( pStatic->GetString(0)->fontIndex );
	//CKTDGUIUniBuffer uniBuffer;
	//uniBuffer.SetFontNode( pFont );

#ifdef KTDGDEVICEFONT_SIMULATE_DIRECTX_FONT
	CKTDGFontManager::CUKFont* pFont = NULL;
	if(pStatic != NULL && pStatic->GetString(0) != NULL) //2011.11.23 lygan_������ //ũ���� �������� ���۵Ǵ� �� ����ó��
	{
		pFont = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetUKFont( pStatic->GetString(0)->fontIndex );
	}
	else
	{
		return NULL;
	}
#else
	CKTDGFontManager::CUKFont* pFont = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetUKFont( pStatic->GetString(0)->fontIndex );
#endif
#ifdef CLIENT_GLOBAL_LINEBREAK
	int lineNum = CWordLineHandler::LineBreakInX2MainMsgBox( tempText, pFont, constTextMaxLen );
#else //CLIENT_GLOBAL_LINEBREAK

	int lineNum = 1;

	for ( int i = 0; i < (int)tempText.size(); i++ )
	{
		WCHAR tempWchar = tempText[i];
		if ( nowTextLen < constTextMaxLen && tempWchar == '\n' )
		{
			if ( i != (int)tempText.size() - 1 )
			{
				lineNum++;
			}

			nowTextLen = 0;
			continue;
		}
		int iUniCharSize = 0;
		//uniBuffer.Clear();
		//uniBuffer.InsertChar( 0, tempWchar );
		//uniBuffer.CPtoX(uniBuffer.GetTextSize(), FALSE, &iUniCharSize);

		iUniCharSize = pFont->GetWidth( tempWchar );

		nowTextLen += iUniCharSize;

		if ( nowTextLen >= constTextMaxLen )
		{
			if ( i != (int)tempText.size() - 1 )
			{
				lineNum++;
			}

			nowTextLen = 0;
			wstring enterChar = L"\n";
			tempText.insert( tempText.begin() + i, enterChar.begin(), enterChar.end() );
			i++;

			
		}

		if ( i >= (int)tempText.size() )
		{
			break;
		}
	}
#endif //CLIENT_GLOBAL_LINEBREAK

	if ( lineNum == 1 )
	{
		pStatic->GetString(0)->pos.y += pFont->GetHeight();
	}
	else if ( lineNum == 2 )
	{
		pStatic->GetString(0)->pos.y += pFont->GetHeight();	
	}
	else if ( lineNum == 3 )
	{
		pStatic->GetString(0)->pos.y += pFont->GetHeight()/2.0f;
	}

	pStatic->GetString(0)->msg = tempText;


	return pDialog;
}
#ifdef ADDED_MESSAGEBOX_USING_CUSTOM_LUA
CKTDGUIDialogType CX2Main::KTDGUIOkAndCancelMsgBox( D3DXVECTOR2 pos, const WCHAR* pText, int okMsg, CKTDXStage* pNowState, int cancelMsg /*= -1*/, wstring wstrFileName /* = L"" */  )
#else // ADDED_MESSAGEBOX_USING_CUSTOM_LUA
CKTDGUIDialogType CX2Main::KTDGUIOkAndCancelMsgBox( D3DXVECTOR2 pos, const WCHAR* pText, int okMsg, CKTDXStage* pNowState, int cancelMsg /*= -1*/  )
#endif // ADDED_MESSAGEBOX_USING_CUSTOM_LUA
{
	if ( pNowState == NULL )
		pNowState = m_pNowState;

#ifdef DIALOG_SHOW_TOGGLE
	if( g_pKTDXApp->GetDGManager()->GetDialogManager()->GetHideDialog() == true )
	{
		g_pKTDXApp->GetDGManager()->GetDialogManager()->SetShow( true );

		bool bHide = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetHideDialog();
		if( g_pInstanceData->GetMiniMapUI() != NULL )
		{				
			g_pInstanceData->GetMiniMapUI()->SetShowMiniMap( g_pInstanceData->GetMiniMapUI()->GetCurrentMiniMapType(), !bHide );
		}
		if( g_pInstanceData->GetMiniMapUI() != NULL && g_pInstanceData->GetMiniMapUI()->GetWorldMapUI() != NULL )
		{
			g_pInstanceData->GetMiniMapUI()->GetWorldMapUI()->OpenWorldMap(false);
			g_pInstanceData->GetMiniMapUI()->GetWorldMapUI()->OpenFieldMap(false);				
		}
	}
#endif

	DialogLog( pText );

	//g_pKTDXApp->GetDIManager()->SetEnable(false);

#ifdef ADDED_MESSAGEBOX_USING_CUSTOM_LUA
	CKTDGUIDialogType pDialog = NULL;
	if ( wstrFileName != L"")
	{
		pDialog = new CKTDGUIDialog( pNowState, wstrFileName.c_str(), 0.07f, XDL_POP_UP );
		if ( pDialog == NULL )
		{
			ASSERT ( !L"pDialog Is Nothing" );
			pDialog = new CKTDGUIDialog( pNowState,  L"DLG_UI_OkAndCancelMsgBox.lua", 0.07f, XDL_POP_UP );
		}
	}
	else
	{
		pDialog = new CKTDGUIDialog( pNowState,  L"DLG_UI_OkAndCancelMsgBox.lua", 0.07f, XDL_POP_UP );
	}

#else // ADDED_MESSAGEBOX_USING_CUSTOM_LUA
#ifdef NEW_VILLAGE_UI
	CKTDGUIDialogType pDialog = new CKTDGUIDialog( pNowState, L"DLG_UI_OkAndCancelMsgBox.lua",0.07f ,XDL_POP_UP );
#else
	CKTDGUIDialogType pDialog = new CKTDGUIDialog( pNowState, L"DLG_OkAndCancelMsgBox.lua",0.07f ,XDL_POP_UP );
#endif
#endif // ADDED_MESSAGEBOX_USING_CUSTOM_LUA

	pDialog->SetOKAndCancelMsgBox( true );
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( pDialog );

	if ( pos.x == -999 && pos.y == -999 )
	{
		pos = GetWindowPos( D3DXVECTOR2( 430, 148 ), D3DXVECTOR2( 217, 71 ) );
	}

	pos.y -= 50.f;
	pDialog->SetPos( pos );
	CKTDGUIStatic* pStatic = (CKTDGUIStatic*)pDialog->GetControl( L"StaticMsgBoxBG" );
	
#ifdef ADDED_MESSAGEBOX_USING_CUSTOM_LUA
	if ( NULL == pStatic )
	{
		ASSERT ( !L"StaticMsgBoxBG Is Nothing" );
	}
#endif // ADDED_MESSAGEBOX_USING_CUSTOM_LUA
	
	wstring tempText = pText;
	//wstring tempText = L"���ع��� ��λ���";

	const int constTextMaxLen = (int)(g_pKTDXApp->GetResolutionScaleX() * 366);
	//bool checkEnterChar = false;
	int nowTextLen = 0;

	//CKTDGFontManager::CKTDGFont* pFont = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetFont( pStatic->GetString(0)->fontIndex );
	//CKTDGUIUniBuffer uniBuffer;
	//uniBuffer.SetFontNode( pFont );
#ifdef ADDED_MESSAGEBOX_USING_CUSTOM_LUA
	if ( NULL != pStatic->GetString(0) )
#endif // ADDED_MESSAGEBOX_USING_CUSTOM_LUA

	{
		CKTDGFontManager::CUKFont* pFont = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetUKFont( pStatic->GetString(0)->fontIndex );
#ifdef CLIENT_GLOBAL_LINEBREAK
	int lineNum = CWordLineHandler::LineBreakInX2MainMsgBox( tempText, pFont, constTextMaxLen );
#else //CLIENT_GLOBAL_LINEBREAK


		int lineNum = 1;

		for ( int i = 0; i < (int)tempText.size(); i++ )
		{
			WCHAR tempWchar = tempText[i];
			if ( nowTextLen < constTextMaxLen && tempWchar == '\n' )
			{
				if ( i != (int)tempText.size() - 1 )
				{
					lineNum++;
				}
				nowTextLen = 0;
				continue;
			}
			int iUniCharSize = 0;
			//uniBuffer.Clear();
			//uniBuffer.InsertChar(0, tempWchar );
			//uniBuffer.CPtoX(uniBuffer.GetTextSize(), FALSE, &iUniCharSize);

			iUniCharSize = pFont->GetWidth( tempWchar );

			nowTextLen += iUniCharSize;

			if ( nowTextLen >= constTextMaxLen )
			{
				if ( i != (int)tempText.size() - 1 )
				{
					lineNum++;
				}

				nowTextLen = 0;
				wstring enterChar = L"\n";
				tempText.insert( tempText.begin() + i, enterChar.begin(), enterChar.end() );
				i++;
			}

			if ( i >= (int)tempText.size() )
			{
				break;
			}
		}
#endif //CLIENT_GLOBAL_LINEBREAK
		pStatic->GetString(0)->msg = tempText;

		if ( lineNum == 1 )
		{
			pStatic->GetString(0)->pos.y += pFont->GetHeight();
		}
		else if ( lineNum == 2 )
		{
			pStatic->GetString(0)->pos.y += pFont->GetHeight();	
		}
		else if ( lineNum == 3 )
		{
			pStatic->GetString(0)->pos.y += pFont->GetHeight()/2.0f;
		}
	}

	CKTDGUIButton* pOkButton = (CKTDGUIButton*)pDialog->GetControl( L"MsgBoxOkAndCancelOkButton" );
#ifdef ADDED_MESSAGEBOX_USING_CUSTOM_LUA
	if ( pOkButton == NULL )
	{
		ASSERT( !L"pOkButton Is Nothing");
	}	
#endif // ADDED_MESSAGEBOX_USING_CUSTOM_LUA
	pOkButton->SetCustomMsgMouseUp( okMsg );
	pOkButton->RequestFocus();

	if( -1 != cancelMsg )
	{
		CKTDGUIButton* pCancelButton = (CKTDGUIButton*)pDialog->GetControl( L"MsgBoxOkAndCancelCancelButton" );
#ifdef ADDED_MESSAGEBOX_USING_CUSTOM_LUA
		if ( pCancelButton == NULL )
		{
			ASSERT( !L"pCancelButton Is Nothing");
		}	
#endif // ADDED_MESSAGEBOX_USING_CUSTOM_LUA
		pCancelButton->SetCustomMsgMouseUp( cancelMsg );
	}

	//{{ JHKang // ������ // 2010.10.04
#ifdef ON_CHAT_REMAIN_FOCUS
	if ( NULL != g_pChatBox && true == g_pChatBox->GetFocusChatEditBox() )
	{
		pDialog->SetModal( true );
		g_pChatBox->OpenChatWindow();

		m_bIsPopUpOnChat = true;
	}
#endif ON_CHAT_REMAIN_FOCUS
	//}} JHKang // ������ // 2010.10.04

	return pDialog;
}

CKTDGUIDialogType	CX2Main::KTDGUIOkAndCancelEditBox( D3DXVECTOR2 vPos, const WCHAR* pText, int iOkMsg, CKTDXStage* pNowState, int iLimitLength, bool bIsByte, int iCancelMsg )
{
	if ( pNowState == NULL )
		pNowState = m_pNowState;

#ifdef DIALOG_SHOW_TOGGLE
	if( g_pKTDXApp->GetDGManager()->GetDialogManager()->GetHideDialog() == true )
	{
		g_pKTDXApp->GetDGManager()->GetDialogManager()->SetShow( true );

		bool bHide = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetHideDialog();
		if( g_pInstanceData->GetMiniMapUI() != NULL )
		{				
			g_pInstanceData->GetMiniMapUI()->SetShowMiniMap( g_pInstanceData->GetMiniMapUI()->GetCurrentMiniMapType(), !bHide );
		}
		if( g_pInstanceData->GetMiniMapUI() != NULL && g_pInstanceData->GetMiniMapUI()->GetWorldMapUI() != NULL )
		{
			g_pInstanceData->GetMiniMapUI()->GetWorldMapUI()->OpenWorldMap(false);
			g_pInstanceData->GetMiniMapUI()->GetWorldMapUI()->OpenFieldMap(false);				
		}
	}
#endif

	DialogLog( pText );

	CKTDGUIDialogType pDialog = new CKTDGUIDialog( pNowState, L"DLG_UI_OkAndCancelEditBox.lua", 0.07f ,XDL_POP_UP );

	if( pDialog == NULL )
	{
		return NULL;
	}

	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( pDialog );

	if ( vPos.x == -999 && vPos.y == -999 )
	{
		vPos = GetWindowPos( D3DXVECTOR2( 430, 148 ), D3DXVECTOR2( 217, 71 ) );
	}

	vPos.y -= 50.f;
	pDialog->SetPos( vPos );

	CKTDGUIStatic* pStatic = (CKTDGUIStatic*)pDialog->GetControl( L"Static_Notice" );

	if( pStatic == NULL )
	{
		return NULL;
	}

	pStatic->SetString(0, pText);

	CKTDGUIIMEEditBox* pEditBox = (CKTDGUIIMEEditBox*) pDialog->GetControl( L"IMEEditBoxMessage" );

	if( pEditBox == NULL )
	{
		return NULL;
	}
	
	if( true == bIsByte )	//	����Ʈ ����
	{
		pEditBox->SetByteLimit_LUA( iLimitLength );
	}
	else	//	��������
	{
		pEditBox->SetLengthLimit_LUA( iLimitLength );
	}

	pEditBox->SetCustomMsgEnter( iOkMsg );

	//	OK, Cancel �̺�Ʈ ����
	CKTDGUIButton* pOkButton = (CKTDGUIButton*) pDialog->GetControl( L"Button_Check" );
	if( pOkButton == NULL )
	{
		return NULL;
	}
	pOkButton->SetCustomMsgMouseUp( iOkMsg );

	CKTDGUIButton* pCancelButton = (CKTDGUIButton*) pDialog->GetControl( L"Cancel" );
	if( pCancelButton == NULL )
	{
		return NULL;
	}

	pCancelButton->SetCustomMsgMouseUp( iCancelMsg );

	pDialog->SetCloseCustomUIEventID( iCancelMsg );

	return pDialog;
}

#ifdef SERV_LOCAL_RANKING_SYSTEM
CKTDGUIDialogType	CX2Main::KTDGUIOkAndCancelEditBox_Hide( D3DXVECTOR2 vPos, const WCHAR* pText, int iOkMsg, CKTDXStage* pNowState, int iLimitLength, bool bIsByte, int iCancelMsg )
{
	if ( pNowState == NULL )
		pNowState = m_pNowState;

#ifdef DIALOG_SHOW_TOGGLE
	if( g_pKTDXApp->GetDGManager()->GetDialogManager()->GetHideDialog() == true )
	{
		g_pKTDXApp->GetDGManager()->GetDialogManager()->SetShow( true );

		bool bHide = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetHideDialog();
		if( g_pInstanceData->GetMiniMapUI() != NULL )
		{				
			g_pInstanceData->GetMiniMapUI()->SetShowMiniMap( g_pInstanceData->GetMiniMapUI()->GetCurrentMiniMapType(), !bHide );
		}
		if( g_pInstanceData->GetMiniMapUI() != NULL && g_pInstanceData->GetMiniMapUI()->GetWorldMapUI() != NULL )
		{
			g_pInstanceData->GetMiniMapUI()->GetWorldMapUI()->OpenWorldMap(false);
			g_pInstanceData->GetMiniMapUI()->GetWorldMapUI()->OpenFieldMap(false);				
		}
	}
#endif

	DialogLog( pText );

	CKTDGUIDialogType pDialog = new CKTDGUIDialog( pNowState, L"DLG_UI_OkAndCancelEditBox_Hide.lua", 0.07f ,XDL_POP_UP );

	if( pDialog == NULL )
	{
		return NULL;
	}

	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( pDialog );

	if ( vPos.x == -999 && vPos.y == -999 )
	{
		vPos = GetWindowPos( D3DXVECTOR2( 430, 148 ), D3DXVECTOR2( 217, 71 ) );
	}

	vPos.y -= 50.f;
	pDialog->SetPos( vPos );

	CKTDGUIStatic* pStatic = (CKTDGUIStatic*)pDialog->GetControl( L"Static_Notice" );

	if( pStatic == NULL )
	{
		return NULL;
	}

	pStatic->SetString(0, pText);

	CKTDGUIEditBox* pEditBox = (CKTDGUIEditBox*) pDialog->GetControl( L"EditBoxMessage" );

	if( pEditBox == NULL )
	{
		return NULL;
	}

	if( true == bIsByte )	//	����Ʈ ����
	{
//		pEditBox->SetByteLimit_LUA( iLimitLength );
	}
	else	//	��������
	{
//		pEditBox->SetLengthLimit_LUA( iLimitLength );
	}

	pEditBox->SetCustomMsgEnter( iOkMsg );

	//	OK, Cancel �̺�Ʈ ����
	CKTDGUIButton* pOkButton = (CKTDGUIButton*) pDialog->GetControl( L"Button_Check" );
	if( pOkButton == NULL )
	{
		return NULL;
	}
	pOkButton->SetCustomMsgMouseUp( iOkMsg );

	CKTDGUIButton* pCancelButton = (CKTDGUIButton*) pDialog->GetControl( L"Cancel" );
	if( pCancelButton == NULL )
	{
		return NULL;
	}

	pCancelButton->SetCustomMsgMouseUp( iCancelMsg );

	pDialog->SetCloseCustomUIEventID( iCancelMsg );

	return pDialog;
}
#endif //SERV_LOCAL_RANKING_SYSTEM



CKTDGUIDialogType CX2Main::KTDGUIOkAndCancelEditBox2( D3DXVECTOR2 vPos, const WCHAR* pText, int iOkMsg, CKTDXStage* pNowState, int iLimitLength, bool bIsByte, int iCancelMsg )
{
	CKTDGUIDialogType pDialog = new CKTDGUIDialog( pNowState, L"DLG_UI_OkAndCancelEditBox2.lua", 0.07f ,XDL_POP_UP );

	if( pDialog == NULL )
	{
		return NULL;
	}

	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( pDialog );

	if ( vPos.x == -999 && vPos.y == -999 )
	{
		vPos = g_pMain->GetWindowPos( D3DXVECTOR2( 430, 148 ), D3DXVECTOR2( 217, 71 ) );
	}

	vPos.y -= 50.f;
	pDialog->SetPos( vPos );

	CKTDGUIStatic* pStatic = (CKTDGUIStatic*)pDialog->GetControl( L"Static_Notice" );

	if( pStatic == NULL )
	{
		return NULL;
	}

	pStatic->SetString(0, pText);

	CKTDGUIIMEEditBox* pEditBox = (CKTDGUIIMEEditBox*) pDialog->GetControl( L"IMEEditBoxMessage" );

	if( pEditBox == NULL )
	{
		return NULL;
	}

	if( true == bIsByte )	//	����Ʈ ����
	{
		pEditBox->SetByteLimit_LUA( iLimitLength );
	}
	else	//	��������
	{
		pEditBox->SetLengthLimit_LUA( iLimitLength );
	}

	pEditBox->SetCustomMsgEnter( iOkMsg );

	//	OK, Cancel �̺�Ʈ ����
	CKTDGUIButton* pOkButton = (CKTDGUIButton*) pDialog->GetControl( L"Button_Check" );
	if( pOkButton == NULL )
	{
		return NULL;
	}
	pOkButton->SetCustomMsgMouseUp( iOkMsg );

	CKTDGUIButton* pCancelButton = (CKTDGUIButton*) pDialog->GetControl( L"Cancel" );
	if( pCancelButton == NULL )
	{
		return NULL;
	}

	pCancelButton->SetCustomMsgMouseUp( iCancelMsg );

	pDialog->SetCloseCustomUIEventID( iCancelMsg );

	return pDialog;
}

CKTDGUIDialog* CX2Main::KTDGUIOKMsgBox_LUA( D3DXVECTOR2 pos, const char* pText, CKTDXStage* pNowState )
{
	wstring text;
	ConvertUtf8ToWCHAR( text, pText );
	CKTDGUIDialogType pDlg = KTDGUIOKMsgBox( pos, text.c_str(), pNowState, -1 );
#ifdef DIALOG_HANDLE_TEST // 2009-8-19 by jintaeks
    return CKTDGUIDialog::GetObjectByUID( pDlg.GetHandle() );
#else
    return pDlg;
#endif // DIALOG_HANDLE_TEST // 2009-8-19
}

CKTDGUIDialog* CX2Main::KTDGUIOkAndCancelMsgBox_LUA( D3DXVECTOR2 pos, const char* pText, int okMsg, CKTDXStage* pNowState )
{
	wstring text;
	ConvertUtf8ToWCHAR( text, pText );
	CKTDGUIDialogType pDlg = KTDGUIOkAndCancelMsgBox( pos, text.c_str(), okMsg, pNowState );
#ifdef DIALOG_HANDLE_TEST // 2009-8-19 by jintaeks
    return CKTDGUIDialog::GetObjectByUID( pDlg.GetHandle() );
#else
    return pDlg;
#endif // DIALOG_HANDLE_TEST // 2009-8-19
}

#ifdef DISABLE_REDUDANT_PACKET_TEST
	bool CX2Main::IsWaitingServerPacket( const SERVER_EVENT_ID waitACK )
	{
		for( UINT i=0; i<m_WaitServerPacketList.size(); i++ )
		{
			WaitServerPacket* pPacket = m_WaitServerPacketList[i];
			if( NULL != pPacket && 
				waitACK == pPacket->waitACK )
			{
				return true;
			}
		}

		return false;
	}
#endif DISABLE_REDUDANT_PACKET_TEST

void CX2Main::AddServerPacket( SERVER_EVENT_ID waitACK, float fReamainTimeOut /* = 0 */ )
{
#ifdef X2TOOL
	return;
#endif

#ifdef _DEBUG
	fReamainTimeOut += 99999.0f;
#endif 

	if ( fReamainTimeOut == 0 )
	{
		fReamainTimeOut = GetServerTimeOut();
	}

	WaitServerPacket* pWaitServerPacket	= new WaitServerPacket;
	pWaitServerPacket->waitACK			= waitACK;
	pWaitServerPacket->fReamainTimeOut	= fReamainTimeOut;

	m_WaitServerPacketList.push_back( pWaitServerPacket );



#ifndef _SERVICE_

	map<int, int>::iterator it;
	it = m_mapWaitServerPacketList.find( (int) waitACK );
	if( it != m_mapWaitServerPacketList.end() )
	{
		if( it->second > 0 )
		{
			WCHAR wszMsg[512] = L"";
			swprintf( wszMsg, sizeof(wszMsg)/sizeof(WCHAR)-1, L"�ߺ� %d.%d", it->first, it->second );
			StateLog( wszMsg );
		}

		it->second += 1;
	}
	else
	{
		m_mapWaitServerPacketList[ (int) waitACK ] = 1;
	}

#endif // _SERVICE_

}

bool CX2Main::DeleteServerPacket( SERVER_EVENT_ID waitACK )
{
	for( int i = 0; i < (int)m_WaitServerPacketList.size(); i++ )
	{
		WaitServerPacket* pWaitServerPacket = m_WaitServerPacketList[i];
		if( pWaitServerPacket->waitACK == waitACK )
		{

#ifndef _SERVICE_

			map<int, int>::iterator it;
			it = m_mapWaitServerPacketList.find( (int) waitACK );
			if( it != m_mapWaitServerPacketList.end() )
			{
				it->second -= 1;
			}

#endif // _SERVICE_

			SAFE_DELETE( pWaitServerPacket );
			m_WaitServerPacketList.erase( m_WaitServerPacketList.begin() + i );
			return true;
		}		
	}
	return false;
}

void CX2Main::AllDeleteServerPacket()
{
	for( int i = 0; i < (int)m_WaitServerPacketList.size(); i++ )
	{
		WaitServerPacket* pWaitServerPacket = m_WaitServerPacketList[i];
		SAFE_DELETE( pWaitServerPacket );
	}
	m_WaitServerPacketList.clear();
}

#ifdef SERV_GLOBAL_AUTH
void CX2Main::AddChannelServerIP( const WCHAR* pServerIP )
{
	if( NULL == pServerIP )
		return;

	m_vecChannelServerIP.push_back( pServerIP );
};		

void CX2Main::AddChannelServerIP_LUA( const char* pServerIP, int iServerPort /*= 9400*/ )
{
	if( NULL == pServerIP )
		return;

#ifdef DOMAIN_TO_IP
	struct hostent* host = NULL;
	wstring wstrChannelServerIP = L"";
	host = gethostbyname(pServerIP);

	if( host != NULL )
	{
		ConvertUtf8ToWCHAR( wstrChannelServerIP, inet_ntoa(*(struct in_addr*)host->h_addr_list[0]) );
	}
	else
	{
		ConvertUtf8ToWCHAR( wstrChannelServerIP, pServerIP );
	}			
#else
	wstring wstrChannelServerIP = L"";
	ConvertUtf8ToWCHAR( wstrChannelServerIP, pServerIP );
#endif DOMAIN_TO_IP

#ifdef	USE_CHANNEL_SERVER_IP_AT_SCRIPT
	m_DefaultChannelServerInfo.m_kServerIP = wstrChannelServerIP;
#else	USE_CHANNEL_SERVER_IP_AT_SCRIPT
	m_vecChannelServerIP.push_back( wstrChannelServerIP );
#endif	USE_CHANNEL_SERVER_IP_AT_SCRIPT


	// Config Lua �������� �������� ���� ä�� ������
#ifndef SERV_COUNTRY_CN
	// �� ���ڵ�� �������� 0���� ������� �����Ѵٴ� ������ 
	// AddChannelServerIP_LUA �Լ��� ������ ������� ȣ��Ǿ�� �Ѵٴ� ������ �־�� ��
	// ������ �̾߱��ϸ� �븸�� ���� �ڵ�
	static int iServerSetID = 0;
	
	KServerGroupInfo kTmpServerGroupInfo;
	kTmpServerGroupInfo.m_iOrder = iServerSetID;
	kTmpServerGroupInfo.m_bEnable = true;
	kTmpServerGroupInfo.m_iServerGroupUID = iServerSetID;
	kTmpServerGroupInfo.m_kServerIP = wstrChannelServerIP;
	kTmpServerGroupInfo.m_usMasterPort = iServerPort;

	g_pInstanceData->m_mapServerGroupInfo.insert(std::make_pair(iServerSetID, kTmpServerGroupInfo));
	
	iServerSetID++;
#endif //ndef SERV_COUNTRY_CN

}
#endif //SERV_GLOBAL_AUTH

void CX2Main::SetSafetyKey_LUA( const char* pKey )
{
	ConvertUtf8ToWCHAR( m_SafetyKey, pKey );
}

void CX2Main::SetClientVersion_LUA( char* pVersion )
{
	ConvertUtf8ToWCHAR( m_ClientVersion, pVersion );
}

void CX2Main::SetSubClientVersion_LUA( char* pVersion )
{
	ConvertUtf8ToWCHAR( m_SubClientVersion, pVersion );
}

void CX2Main::ChangeKoreanAndEnglishKey( bool bKorean )
{
	HIMC haneng; 
	haneng = ImmGetContext(g_pKTDXApp->GetHWND());
	ImmSetConversionStatus(haneng,bKorean,0);
}

//{{ kimhc // 2010.3.12 //	ä��â ����
#ifdef	CHAT_WINDOW_IMPROV
bool CX2Main::GetNativeAndEnglisKeyState() //���� �ѿ� ��� �˾Ƴ���	// TRUE �� �ѱ�
{
	bool bRet = false;  
	HIMC hIMC = NULL;  
	if ( hIMC = ImmGetContext( g_pKTDXApp->GetHWND() ) )   
	{  
		DWORD fdwConversion, fdwSentence;  
		
		BOOL bRet2 = ImmGetConversionStatus(hIMC, &fdwConversion, &fdwSentence);  
		
		if( bRet2 ) 
		{  
			bRet = ( fdwConversion & IME_CMODE_NATIVE ? true : false );  
		}
		
		ImmReleaseContext( g_pKTDXApp->GetHWND(), hIMC );  
	}  

	return bRet;  
}
#endif	CHAT_WINDOW_IMPROV	
//}} kimhc // 2010.3.12 //	ä��â ����

//{{ kimhc // 2010.9.14 // Ȩ������ �������� �ƹ�Ÿ �̹��� ǥ������ ���� ĳ���� �̹��� ĸ�� ��� �׽�Ʈ
#ifdef	AVATAR_IMAGE_TEST
bool	CX2Main::ScreenCaptureAndSendToWeb( IN double fTime, IN float fElapsedTime )
{
	wstring wstrFileName;

	if ( true == ScreenCaptureForWebAvatar( fTime, fElapsedTime, wstrFileName ) )
	{
		if ( true == SendFileCapturedToWeb( wstrFileName ) )
			return true;
	}

	return false;
	
}

bool	CX2Main::ScreenCaptureForWebAvatar( IN double fTime, IN float fElapsedTime, OUT wstring& wstrFileName )
{
	if ( g_pData == NULL ||
		 g_pData->GetMyUser() == NULL ||
		 g_pData->GetMyUser()->GetSelectUnit() == NULL )
		 return false;

	D3DXVECTOR2 vPos( -370.0f, 120.0f );
	//D3DXVECTOR2 vPos( 0.0f, 0.0f );

	CX2UnitViewerUI* pUnitViewerUI	= CX2UnitViewerUI::CreateUnitViewerUI();
	pUnitViewerUI->SetUnit( g_pData->GetMyUser()->GetSelectUnit(), CX2UnitViewerUI::UVS_CHARINFO );
	pUnitViewerUI->SetFixed( true );

	pUnitViewerUI->SetPositionOnScr( vPos.x, vPos.y );//, 200.0f, 1.15f );
	pUnitViewerUI->SetLightPos( -250000, 50000, -500000 );	// ĳ���ͺ�� ����Ʈ ��ġ ����
	pUnitViewerUI->GetMatrix().Rotate(D3DXToRadian(0),D3DXToRadian(0),D3DXToRadian(0));
	pUnitViewerUI->GetXSkinAnim()->SetApplyMotionOffset( false );
	pUnitViewerUI->PlayByMotionType( CX2UnitViewerUI::UVUMT_WAIT );
	pUnitViewerUI->SetShowObject( true );

	pUnitViewerUI->OnFrameMove( fTime, fElapsedTime );

	RECT rect;
	HWND pDesktop = g_pKTDXApp->GetHWND(); // ���ڽ��� �������� ��ǥ�� �����Ѵ�. 
	GetClientRect( pDesktop, &rect ); // ������ ȭ�� DC�� �޸� DC�� ����� �޸� ��Ʈ���� �����Ѵ�. 

	CKTDXDeviceRenderTargetTexture* pRenderTargetTextureAvatar 
		= g_pKTDXApp->GetDeviceManager()->OpenRenderTargetTexture( L"Avatar", rect.right, rect.bottom, D3DFMT_A8R8G8B8  );

	if ( NULL == pRenderTargetTextureAvatar )
	{
		ASSERT( !L"m_pRenderTargetTextureAvatar is wrong!" );
		return false;
	}

	pRenderTargetTextureAvatar->BeginRender( true );
	pRenderTargetTextureAvatar->Clear( D3DCOLOR_ARGB( 0, 0, 0, 0 ) );
	D3DXMATRIX matWorld, matView, matProj;

	bool bOriginalProjcetionMode = g_pKTDXApp->GetDGManager()->GetPerspective();
	D3DXMATRIX matIden;
	D3DXMatrixIdentity( &matIden );

	g_pKTDXApp->GetDevice()->GetTransform( D3DTS_WORLD, &matWorld );
	g_pKTDXApp->GetDevice()->GetTransform( D3DTS_VIEW, &matView );
	g_pKTDXApp->GetDevice()->GetTransform( D3DTS_PROJECTION, &matProj );


	// World Matrix�� Identify�� �����ϰ�
	g_pKTDXApp->SetWorldTransform( &matIden );

	// View Matrix�� 0,0,-1300���� 0,0,0�� �ٶ󺸴� ��ǥ (PVPRoom, StateServerSelect ��� ���̴� ��ǥ)�� ����
	D3DXMATRIX ViewMatrix;
	D3DXMatrixLookAtLH( &ViewMatrix, &D3DXVECTOR3(0,0,-100), &D3DXVECTOR3(0,0,0), &D3DXVECTOR3(0,1.f,0) );
	g_pKTDXApp->SetViewTransform( &ViewMatrix );

	// Projection ����
	g_pKTDXApp->GetDGManager()->SetProjection( g_pKTDXApp->GetDGManager()->GetNear(), g_pKTDXApp->GetDGManager()->GetFar(), false );	// Ortho

	CKTDGStateManager::PushStates( pUnitViewerUI->GetRenderStateID() );
	pUnitViewerUI->OnFrameRender();

	g_pKTDXApp->GetDGManager()->SetProjection( g_pKTDXApp->GetDGManager()->GetNear(), g_pKTDXApp->GetDGManager()->GetFar(), bOriginalProjcetionMode );
	g_pKTDXApp->SetProjectionTransform( &matProj );
	g_pKTDXApp->SetWorldTransform( &matWorld );
	g_pKTDXApp->SetViewTransform( &matView );

	pRenderTargetTextureAvatar->EndRender();
	
	bool bReturn = g_pKTDXApp->GetDGManager()->ScreenCaptureForAvatar( wstrFileName, pRenderTargetTextureAvatar );
	SAFE_CLOSE( pRenderTargetTextureAvatar );
	
	CX2UnitViewerUI::DeleteKTDGObject( pUnitViewerUI );

	return bReturn;
}

bool	CX2Main::SendFileCapturedToWeb( IN const wstring& wstrFileName )
{
	const int RESPONSE_LENGTH = 1024;
	WCHAR szBuffer[MAX_PATH] = { 0, };
	
	GetCurrentDirectoryW( MAX_PATH, szBuffer );

	wstring wstrFilePathAndName = szBuffer;
	wstrFilePathAndName += L"\\";
	wstrFilePathAndName += wstrFileName;
	
	CX2Unit::UnitData* pUnitData = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData();

	wstringstream wstrUrlStream;
	wstrUrlStream << L"http://avatar.elsword.nexon.com/upload.aspx?";

	// �ؽ� ����� Ű
	wstrUrlStream << "n4NexonSN=";
	wstrUrlStream << g_pData->GetMyUser()->GetNexonSN();

	// ĳ����Ű
	wstrUrlStream << "&n8CharacterSN=";
	wstrUrlStream << pUnitData->m_iNMKSerialNum;

	// ĳ���͸�(����)
	wstrUrlStream << "&strCharacterName=";
	wstrUrlStream << pUnitData->m_NickName;

	WCHAR szReponse[RESPONSE_LENGTH] = {0, };

	if ( false == NMCOHelpers::UploadFile( wstrUrlStream.str().c_str(), wstrFilePathAndName.c_str(), NULL, szReponse ) )
	{
// 		ErrorLogMsg( XEM_ERROR140, szReponse );
// 		CheckUploadFileLog( szReponse );
// 		CheckUploadFileLog( wstrUrlStream.str().c_str() );
		ASSERT( !L"NMCOHelpers::UploadFile is failed" );
		return false;
	}
	else
	{
		DeleteFile( wstrFileName.c_str() );
	}

// 	CheckUploadFileLog( szReponse );
// 	CheckUploadFileLog( wstrUrlStream.str().c_str() );

	return true;
}
#endif	AVATAR_IMAGE_TEST
//}} kimhc // 2010.9.14 // Ȩ������ �������� �ƹ�Ÿ �̹��� ǥ������ ���� ĳ���� �̹��� ĸ�� ��� �׽�Ʈ

void CX2Main::ResetCrashReport( const WCHAR* wszAddMsg )
{
	
	string clientVersion = "";

#ifdef _SERVICE_
	#ifdef _OPEN_TEST_
		clientVersion = "[CRASH_REPORT_OPEN_TEST] ";
	#else
		clientVersion = "[CRASH_REPORT_SERVICE] ";
	#endif
#else
	clientVersion = "[CRASH_REPORT_TEST] ";
#endif
	
	string clientMainVersion = "";
	ConvertWCHARToChar( clientMainVersion, GetClientVersion() );
	clientVersion += "_(";
	clientVersion += clientMainVersion;
	string subClientVersion = "";
	ConvertWCHARToChar( subClientVersion, GetSubClientVersion_LUA() );
	if( subClientVersion.length() > 0 )
	{
		clientVersion += "_";
		clientVersion += subClientVersion;	
	}
	clientVersion += ")_";

	string userID = "";
	if( NULL != g_pData && NULL != g_pData->GetMyUser() && 
		NULL != g_pData->GetMyUser()->GetUserData() )
	{
		ConvertWCHARToChar( userID, g_pData->GetMyUser()->GetUserData()->userID.c_str() );
	}

	string unitName = "";
	if( NULL != g_pData && NULL != g_pData->GetMyUser() && 
		NULL != g_pData->GetMyUser()->GetSelectUnit() && 
		NULL != g_pData->GetMyUser()->GetSelectUnit()->GetNickName() )
	{
		ConvertWCHARToChar( unitName, g_pData->GetMyUser()->GetSelectUnit()->GetNickName() );
	}
	
	clientVersion += "_";
	clientVersion += userID;
	clientVersion += "_";
	clientVersion += unitName;
	clientVersion += "_";

	if( NULL != wszAddMsg )
	{
		string szAddMsg;
		ConvertWCHARToChar( szAddMsg, wszAddMsg );
		clientVersion += szAddMsg;
	}

	if( NULL != m_pCrashReportState )
		SetEMailSubjectPrefix( m_pCrashReportState, (LPCTSTR)clientVersion.c_str() );	
}

bool CX2Main::SendQuitMsgToServer()
{
	char szErrorLogFileName[256] = "ErrorLog.txt";
	KEGS_CLIENT_QUIT_REQ kPacket;

#if 0 // ������ ��û�� ���� ����
	if ( g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->GetPullPath( szErrorLogFileName ) != NULL )
	{
		

		FILE *f;

		char buf[1024];

		f=fopen(szErrorLogFileName,"rt");

		if (f!=NULL) {

			for (;;) {

				if (fgets(buf,1024,f)==NULL) {

					break;

				}

				std::string errorMsg = buf;
				std::basic_string <char>::size_type indexString;
				std::basic_string <char>::size_type indexEnumBegin;
				std::basic_string <char>::size_type indexEnumEnd;

				indexEnumEnd = 0;

				while(1)
				{
					indexString		= errorMsg.find( "ErrorEnum", indexEnumEnd );
					if ( (int)indexString == -1 )
						break;

					indexEnumBegin	= errorMsg.find( "(", indexString ) + 1;

					if ( (int)indexEnumBegin == -1 )
						break;

					indexEnumEnd	= errorMsg.find( ")", indexEnumBegin );

					if ( (int)indexEnumEnd == -1 )
						break;

					string strErrorEnumID;
					strErrorEnumID.assign( errorMsg.begin() + indexEnumBegin, errorMsg.begin() + indexEnumEnd );

					int errorEnumID = atoi( strErrorEnumID.c_str() );

					indexEnumBegin -= 1;
					//int nextErrorEnum = (int)errorMsg.find( "ErrorEnum", indexEnumEnd );
					int nextErrorMsg = (int)errorMsg.find( "ErrorMsg", indexEnumEnd );
					if ( nextErrorMsg == -1 )
					{
						KErrorMsgInfo kErrorMsgInfo;
						kErrorMsgInfo.m_iErrorMsgID = errorEnumID;
						kErrorMsgInfo.m_wstrErrorMsg = L"";

						kPacket.m_ErrorMsgInfoList.push_back( kErrorMsgInfo );
					}
					else
					{
						indexString = errorMsg.find( "ErrorMsg", indexEnumEnd );

						indexEnumBegin	= errorMsg.find( "(", indexString ) + 1;

						if ( (int)indexEnumBegin == -1 )
							break;

						indexEnumEnd	= errorMsg.find( ")", indexEnumBegin );

						if ( (int)indexEnumEnd == -1 )
							break;

						string strErrorEnumID;
						strErrorEnumID.assign( errorMsg.begin() + indexEnumBegin, errorMsg.begin() + indexEnumEnd );
						wstring wstrErrorEnumMsg;
						ConvertCharToWCHAR( wstrErrorEnumMsg, strErrorEnumID.c_str() );

						KErrorMsgInfo kErrorMsgInfo;
						kErrorMsgInfo.m_iErrorMsgID = errorEnumID;
						wstring tempWstrErrorEnumMsg = wstrErrorEnumMsg;

						while( tempWstrErrorEnumMsg.find( L"'" ) != -1 )
						{
							int i = tempWstrErrorEnumMsg.find( L"'" );
							tempWstrErrorEnumMsg.erase( tempWstrErrorEnumMsg.begin() + i );
						}

						kErrorMsgInfo.m_wstrErrorMsg = tempWstrErrorEnumMsg;

						kPacket.m_ErrorMsgInfoList.push_back( kErrorMsgInfo );

					}


				}
			}

			fclose(f);

		}

		
	}
#endif

	if ( g_pData != NULL && g_pData->GetServerProtocol() != NULL )
	{
		kPacket.m_iReason = m_QuitType;
		g_pData->GetServerProtocol()->SendPacket( EGS_CLIENT_QUIT_REQ, kPacket );
	}

	return true;
}

bool CX2Main::DeleteFile_LUA( const char* pFileName )
{
	wstring fileName;
	ConvertUtf8ToWCHAR( fileName, pFileName );

	return DeleteFile( fileName.c_str() );
}


void CX2Main::ResetFullScreenResolution(int iWidth, int iHeight)
{
	// DXUTSetFullscreenClientRectAtModeChange()�� ���� DXUT�� ���� �Լ���. ���Ƿ� �߰�����.
	// fix! �켱 �Ǽ��� �̷��� �ص���. �Ƹ� createdevice() �Ǵ� changedevice()�� ����ؾ� �� �� ����.
	// ������ ������ �� fullscreen���� �����ϸ� �� �Լ��� �ƹ� �ҿ��� ����. winmain���� createdevice�� �� �ػ󵵸� �����ؾ�.

	if( DXUTIsWindowed() )
	{	
		DXUTSetFullscreenClientRectAtModeChange(iWidth, iHeight);
	}
	else
	{
		//DXUTSetWindowClientRectAtModeChange(iWidth,iHeight);	
		//DXUTDeviceSettings deviceSettings = DXUTGetDeviceSettings();
		//deviceSettings.pp.Windowed = !deviceSettings.pp.Windowed;
		//DXUTSetFullscreenClientRectAtModeChange(iWidth, iHeight);
	}		
}

bool CX2Main::IsValidPacket( int enumID )
{
	//////////////////////////////////////////////////////////////////////////
	/// �޼��� �˾� ���� ó���ϴ� �ݿ��� 
	//////////////////////////////////////////////////////////////////////////
	switch( enumID )
	{
	//// true �����ϴ� ���.
	case NetError::NET_OK:
	case NetError::ERR_SLOT_07:
	case NetError::NOT_LEAVE_FIELD_REASON_00: //�ٸ� �ʵ�� �̵��ϱ� ���� ��Ż�Դϴ�.
	case NetError::NOT_LEAVE_ROOM_REASON_27: //��Ʋ�ʵ� ������ ���� �� ��Ż �Դϴ�.
		return true;
		break;

	//// false �����ϴ� ���
	case NetError::ERR_REFRESH_01:
	case NetError::ERR_PARTY_03:		// ��Ƽ������ �߸��Ǿ����ϴ�.
	case NetError::ERR_ROOM_04:			// ��й�ȣ Ʋ���� �� ���� ����������
	case NetError::ERR_ROOM_21:			// �̹� ���� NPC �˾�â ����� �ʴ´�.
	case NetError::ERR_KNM_06:			// ģ���ʴ� �źδ��������� �˾� �ȶ��
	case NetError::ERR_RESURRECT_01:	// ������ '�̹� ��Ȱ�ϼ̽��ϴ�' �޽��� �ȶߵ����߰�
	case NetError::ERR_WRONG_STATE_00:	// ���� ���� ó�� : GSFSM ���°� ����
	case NetError::ERR_ITEM_12:			// �ӽ� �κ��丮 �������� �κ��丮�� ���� ������ �����մϴ�.
	case NetError::ERR_BUY_CASH_ITEM_42:
	case NetError::ERR_TITLE_01:
	case NetError::ERR_PERSONAL_SHOP_37:
	case NetError::ERR_BUY_CASH_ITEM_67:
	case NetError::ERR_FIELD_05:		//�ʵ� ���� ���� �����˾��� ���� ���
	case NetError::ERR_FIELD_06:
	case NetError::ERR_BATTLEFIELD_13:
	case NetError::ERR_BATTLEFIELD_14:
	case NetError::ERR_RELATIONSHIP_25:
#ifdef SERV_UNIT_WAIT_DELETE
	case NetError::ERR_DELETE_UNIT_02:
	case NetError::ERR_RESTORE_UNIT_02:
#endif //SERV_UNIT_WAIT_DELETE	
		return false;

	case NetError::ERR_ROOM_27:			// �����뿡�� ������ ���� ����â �ȶ��� �ϱ�
		{
			if( NULL != g_pData &&
				NULL != g_pData->GetMyUser() &&
				NULL != g_pData->GetMyUser()->GetSelectUnit() &&
				true == g_pData->GetMyUser()->GetSelectUnit()->GetMyInfoInDungeonRoom())
			{
				return false;
			}

			// �������� �ƴ϶�� ���������� ���� �˾� ����ϵ��� ����.
		}
		break;
	case NetError::ERR_SORT_CATEGORY_01:
		{
			g_pChatBox->AddChatLog( NetError::GetErrStrF( NetError::ERR_SORT_CATEGORY_01 ), KEGS_CHAT_REQ::CPT_SYSTEM, D3DXCOLOR(1,1,0,1), L"#CFFFF00" );
			return true;
		} break;
	// �˾� ó���ϴ� �޼����� ���� �Ʒ��� ó������� �Ǳ� ������ default���� return �ϸ� �ȵ�.
	default:
		break;
	}

	//////////////////////////////////////////////////////////////////////////
	/// �޼��� �˾� ó���ϴ� �ݿ��� 
	//////////////////////////////////////////////////////////////////////////
	std::wstringstream wstrStream;
	wstrStream << L"IsValidPacket" << L"_" << NetError::GetErrStrF(enumID);

#ifndef _SERVICE_
	if( enumID >= NetError::ERR_SENTINEL || enumID < 0 ) 
		wstrStream << L"(" << enumID << L")";
#endif //_SERVICE_

	string strErr;
	ConvertWCHARToChar( strErr, wstrStream.str().c_str() );

	switch( enumID )
	{
		//// true �����ϴ� ���.
	case NetError::ERR_SHUT_DOWN_00:
	case NetError::ERR_SHUT_DOWN_01:
	case NetError::ERR_SHUT_DOWN_02:
	case NetError::ERR_SHUT_DOWN_03:
	case NetError::ERR_SHUT_DOWN_04:
	case NetError::ERR_SHUT_DOWN_05:
		{
			CKTDGUIDialogType pDLGOK = 
				KTDGUIOKMsgBox( D3DXVECTOR2(250,300), NetError::GetErrStrF(enumID), 
								m_pNowState, CX2StateCreateUnit::SCUUCM_SHUT_DOWN_OK );
			return true;
		} break;
	case NetError::ERR_NX_AUTH_00:
		{
			CKTDGUIDialogType pDLGOK = 
				KTDGUIOKMsgBox( D3DXVECTOR2(250,300), NetError::GetErrStrF(enumID), 
								m_pNowState, CX2CashShop::CSUCM_CHECK_AUTHORITY_OK );
			return true;
		} break;

#ifdef SERV_RELATIONSHIP_SYSTEM_LAUNCHING_EVENT
	case NetError::ERR_REQUEST_OF_LOVE_01:
		{
			KTDGUIOKMsgBox( D3DXVECTOR2(250,300), NetError::GetErrStrF(enumID), m_pNowState );
			return true;
		} break;
#endif //SERV_RELATIONSHIP_SYSTEM_LAUNCHING_EVENT

		//// false �����ϴ� ���.
	case NetError::ERR_ROOM_09:
		{
			ErrorLogMsg( XEM_ERROR114, strErr.c_str() );
			return false;
		}
		break;
	case NetError::ERR_USE_ITEM_IN_INVENTORY_06:
		{
			if( NULL != g_pData->GetUIManager() &&
				NULL != g_pData->GetUIManager()->GetUIInventory() )
			{
				CKTDGUIDialogType pDLGOK = 
					KTDGUIOkAndCancelMsgBox( D3DXVECTOR2(-999,-999), GET_STRING( STR_ID_16527 ), 
					CX2UIInventory::UIM_ITEM_USE_DEFENCE_DUNGEON_POTION, 
					g_pMain->GetNowState() );
			}
			return false;
		} break;
#ifdef SERV_EVENT_ARA_NEW_CHAR_THE_ESSENCE_OF_HERETIC
	case NetError::ERR_ITEM_INT_01:
		{
			KTDGUIOKMsgBox( D3DXVECTOR2(250,300), NetError::GetErrStrF(enumID), m_pNowState );
			return false;
		} break;
#endif SERV_EVENT_ARA_NEW_CHAR_THE_ESSENCE_OF_HERETIC
	case NetError::ERR_UDP_01:
		{
			KTDGUIOKMsgBoxPlus( D3DXVECTOR2(250,300), GET_STRING(STR_ID_24041), m_pNowState );
			return false;
		} break;

#ifdef ADDED_RELATIONSHIP_SYSTEM
	case NetError::ERR_RELATIONSHIP_05 :
		{
			CX2Main::TimedMessagePopUp::TimedPopupUserData userData;
			userData.iMyUID			= g_pData->GetMyUser()->GetSelectUnit()->GetUID();
			userData.iOpponentUID	= g_pData->GetMyUser()->GetSelectUnit()->GetUID();

			g_pMain->RemoveTimedMessagePopup( CX2Main::TimedMessagePopUp::MT_WAIT_CHOICE_COUPLE, userData );
			KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING ( STR_ID_24603 ), m_pNowState );
		}
		break;

	case NetError::ERR_RELATIONSHIP_40 :
		{
			return true;
		}
		break;

	case NetError::ERR_RELATIONSHIP_06 :
		{
			CX2Main::TimedMessagePopUp::TimedPopupUserData userData;
			userData.iMyUID			= g_pData->GetMyUser()->GetSelectUnit()->GetUID();
			userData.iOpponentUID	= g_pData->GetMyUser()->GetSelectUnit()->GetUID();

			g_pMain->RemoveTimedMessagePopup( CX2Main::TimedMessagePopUp::MT_WAIT_CHOICE_COUPLE, userData );
			KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING ( STR_ID_24455 ), m_pNowState );

		}
		break;
	case NetError::ERR_RELATIONSHIP_03 :
		{	
			CX2Main::TimedMessagePopUp::TimedPopupUserData userData;
			userData.iMyUID			= g_pData->GetMyUser()->GetSelectUnit()->GetUID();
			userData.iOpponentUID	= g_pData->GetMyUser()->GetSelectUnit()->GetUID();

			g_pMain->RemoveTimedMessagePopup( CX2Main::TimedMessagePopUp::MT_WAIT_CHOICE_COUPLE, userData );
			KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING ( STR_ID_24456 ), m_pNowState );
		}
		break;
	case NetError::ERR_RELATIONSHIP_00 :
		{
			CX2Main::TimedMessagePopUp::TimedPopupUserData userData;
			userData.iMyUID			= g_pData->GetMyUser()->GetSelectUnit()->GetUID();
			userData.iOpponentUID	= g_pData->GetMyUser()->GetSelectUnit()->GetUID();

			g_pMain->RemoveTimedMessagePopup( CX2Main::TimedMessagePopUp::MT_WAIT_CHOICE_COUPLE, userData );
			KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING ( STR_ID_24458 ), m_pNowState );
		}
		break;
	case NetError::ERR_RELATIONSHIP_24 :
		{			
			return true;
		}
		break;
	case NetError::ERR_RELATIONSHIP_08 :
	case NetError::ERR_RELATIONSHIP_02 :
	case NetError::ERR_RELATIONSHIP_04 :
	case NetError::ERR_RELATIONSHIP_01 :
	case NetError::ERR_RELATIONSHIP_07 :				
		{
			CX2Main::TimedMessagePopUp::TimedPopupUserData userData;
			userData.iMyUID			= g_pData->GetMyUser()->GetSelectUnit()->GetUID();
			userData.iOpponentUID	= g_pData->GetMyUser()->GetSelectUnit()->GetUID();
			
			g_pMain->RemoveTimedMessagePopup( CX2Main::TimedMessagePopUp::MT_WAIT_CHOICE_COUPLE, userData );
			KTDGUIOKMsgBox( D3DXVECTOR2(250,300), NetError::GetErrStrF(enumID), m_pNowState );
			return false;
		} break;

	case NetError::ERR_RELATIONSHIP_47 :
		{
			CX2Main::TimedMessagePopUp::TimedPopupUserData userData;
			userData.iMyUID			= g_pData->GetMyUser()->GetSelectUnit()->GetUID();
			userData.iOpponentUID	= g_pData->GetMyUser()->GetSelectUnit()->GetUID();

			g_pMain->RemoveTimedMessagePopup( CX2Main::TimedMessagePopUp::MT_WAIT_CHOICE_COUPLE, userData );
			KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING ( STR_ID_26240 ), m_pNowState );
		} break;
#endif // ADDED_RELATIONSHIP_SYSTEM




	default:
		{
#ifdef NETERROR_KOREAN_HELP
			if ( g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_DEV )
			{
				std::wstring wstrErrorMsg = boost::str( boost::wformat( L"NetError(%1%)\n%2%\n%3%" ) % enumID % NetError::GetErrStrF(enumID) % NetError::GetErrStr(enumID) );
				KTDGUIOKMsgBox( D3DXVECTOR2(250,300), wstrErrorMsg.c_str(), m_pNowState );
			}
			else
			{
				KTDGUIOKMsgBox( D3DXVECTOR2(250,300), NetError::GetErrStrF(enumID), m_pNowState );
			}
#else NETERROR_KOREAN_HELP
			KTDGUIOKMsgBox( D3DXVECTOR2(250,300), NetError::GetErrStrF(enumID), m_pNowState );
#endif NETERROR_KOREAN_HELP
			ErrorLogMsg( XEM_ERROR77, strErr.c_str() );
			return false;
		} break;
	}

	return false;
}


bool CX2Main::InstallFont( const char* pValueName, const char* pFontFileName )
{
#ifdef USE_FREE_TYPE
	g_pKTDXApp->GetDGManager()->GetFontManager()->InstallFont( pValueName, pFontFileName );
#endif //USE_FREE_TYPE

	WCHAR FontPath[MAX_PATH] = {0,};
	WCHAR tempFontFileName[MAX_PATH] = {0,};
	WCHAR FontFileName[MAX_PATH] = {0,};
	WCHAR FilePath[MAX_PATH] = {0,};

	wstring fontFileName;
	wstring realFontFileName;
	ConvertUtf8ToWCHAR( fontFileName, pFontFileName );
	ConvertUtf8ToWCHAR( realFontFileName, pFontFileName );

	StringCchPrintf( FontFileName, MAX_PATH , L"%s", fontFileName.c_str() );
	StringCchPrintf( tempFontFileName, MAX_PATH , L"%s", fontFileName.c_str() );
	/*wsprintf( FontFileName, L"%s", fontFileName.c_str() );
	wsprintf( tempFontFileName, L"%s", fontFileName.c_str() );*/

	if ( g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->GetPullPath( tempFontFileName ) == NULL )
		return false;

	StringCchPrintf( FilePath, MAX_PATH , g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->GetPullPath( FontFileName ) );
	/*wsprintf( FilePath, g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->GetPullPath( FontFileName ) );*/
	/*

	GetWindowsDirectory( FontPath, MAX_PATH );
	wcscat( FontPath, L"\\Fonts\\" );
	wcscat( FontPath, realFontFileName.c_str() );

	string strFontPath;
	ConvertWCHARToChar( strFontPath, FontPath );

	if ( _access( strFontPath.c_str(), 0 ) != 0 )
	{
		if ( CopyFile( FilePath, FontPath, FALSE ) == FALSE )
		{
			//InvalidateRect( g_pKTDXApp->GetHWND(), NULL, TRUE );
			return false;
		}
	}
	*/
	
	if ( AddFontResource( FilePath ) == FALSE )
	{
		//KTDGUIOKMsgBox( D3DXVECTOR2(250,300), L"Font Install Fail", m_pNowState );
		ErrorLog(XEM_ERROR16);
		return false;
		
	}

//	SendMessage( HWND_BROADCAST, WM_FONTCHANGE, 0, 0 );

	m_UseFontNameList.push_back( FilePath );


/*
	
	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	WCHAR szKey[255] = {0};
	GetVersionEx( &vi );

	debugWPrintf( L"InstallFont8" );

	if ( vi.dwPlatformId == VER_PLATFORM_WIN32_NT )
	{
		wcscpy( szKey, L"SoftWare\\Microsoft\\Windows NT\\CurrentVersion\\Fonts" );
	}
	else
	{
		wcscpy( szKey, L"SoftWare\\Microsoft\\Windows\\CurrentVersion\\Fonts" );
	}

	debugWPrintf( L"InstallFont9" );

	//SHRegWriteString()
	//SHRegSetUSValue( szKey, , REG_SZ, , , HKEY_LOCAL_MACHINE );

	HKEY key;

	DWORD dwDisp;
	WCHAR lpData[256];
	wsprintf( lpData, L"%s", fontFileName.c_str() );

	wstring valueName;
	ConvertCharToWCHAR( valueName, pValueName );

	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKey,0,NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE,NULL,&key,&dwDisp)
		!=ERROR_SUCCESS)
	{
		KTDGUIOKMsgBox( D3DXVECTOR2(250,300), L"Font Install Fail", m_pNowState );
		ErrorLog(XEM_ERROR17);
		return false;
	}



	if(RegSetValueEx(key, valueName.c_str(), 0, REG_SZ, (LPBYTE)lpData,  sizeof(WCHAR) * (int)wcslen(lpData) )
		!=ERROR_SUCCESS)
	{
		ErrorLog(XEM_ERROR18);
		KTDGUIOKMsgBox( D3DXVECTOR2(250,300), L"Font Install Fail", m_pNowState );
		return false;
	}

	RegCloseKey(key);
*/
	return true;

}

void CX2Main::BuyItemSuper_LUA( int itemID, int iQuantity /*= 1*/, short sPeriod /*= 0*/ )
{
	if( g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_OPERATOR )
	{
#ifdef LIGHT_OPERATOR_ACCOUNT
		if(g_pData->GetMyUser()->GetAuthLevel() == CX2User::XUAL_LIGHT_OPERATOR )
		{
			return;
		}
#endif LIGHT_OPERATOR_ACCOUNT
		
		switch( GetNowStateID() )
		{
		case CX2Main::XS_VILLAGE_MAP: // �������
		case CX2Main::XS_BATTLE_FIELD:
			{
				if( iQuantity <= 0 )
					iQuantity = 1;

				KEGS_BUY_ED_ITEM_REQ kEGS_BUY_GP_ITEM_REQ;
				kEGS_BUY_GP_ITEM_REQ.m_bDebug = true;
				kEGS_BUY_GP_ITEM_REQ.m_sPeriod = sPeriod;
				kEGS_BUY_GP_ITEM_REQ.m_mapItem.insert( std::make_pair( itemID, iQuantity ) );

				// 09.07.03 �¿� : ACK ó���ϴ� Class�� UIShop�̹Ƿ� Ȥ�� ������ �������ش�.
				if(g_pData->GetUIManager()->GetUIShop() == NULL)
				{
					g_pData->GetUIManager()->CreateUIShop();
				}

				g_pData->GetServerProtocol()->SendPacket( EGS_BUY_ED_ITEM_REQ, kEGS_BUY_GP_ITEM_REQ );
				g_pMain->AddServerPacket( EGS_BUY_ED_ITEM_ACK );

			} break;
		}
	}
}


void CX2Main::SetAuthLevel_LUA( const CHAR* userID, CX2User::X2_USER_AUTH_LEVEL level )
{
#ifndef _SERVICE_
	if( g_pData->GetMyUser()->GetAuthLevel() < CX2User::XUAL_OPERATOR )
		return;

	wstring wstrUserID = L"";
	ConvertUtf8ToWCHAR( wstrUserID, userID );
	CX2State* pNowState = (CX2State*) g_pMain->GetNowState();
	pNowState->Handler_EGS_ADMIN_CHANGE_AUTH_LEVEL_REQ( wstrUserID.c_str(), level );
#endif
}

void CX2Main::GetAuthUserList_LUA( CX2User::X2_USER_AUTH_LEVEL level )
{
#ifdef LIGHT_OPERATOR_ACCOUNT
	if( g_pData->GetMyUser()->GetAuthLevel() == CX2User::XUAL_LIGHT_OPERATOR )
	{
		return ;
	}
#endif LIGHT_OPERATOR_ACCOUNT

	CX2State* pNowState = (CX2State*) g_pMain->GetNowState();
	pNowState->Handler_EGS_ADMIN_GET_AUTH_LEVEL_LIST_REQ( level );

}

void CX2Main::AddDungeonRewardItem( std::vector<KItemInfo>& vecDungeonRewardItem )
{
	m_vecDungeonRewardItem.insert( m_vecDungeonRewardItem.end(),
		vecDungeonRewardItem.begin(), vecDungeonRewardItem.end() );
}

void CX2Main::Handler_EGS_CHECK_SUM_REQ( bool bFileCheck /* = false */ )
{
	if ( m_pCheckSumManager != NULL )
		m_pCheckSumManager->Handler_EGS_CHECK_SUM_REQ( bFileCheck );
}
//{{����ȯ �����ð� �޾ƿ���
#ifdef	SERV_SERVER_TIME_GET
bool CX2Main::Handler_ECH_GET_SERVER_TIME_REQ()
{
	if(g_pData->GetServerProtocol() == NULL)
	{
		g_pData->ResetServerProtocol();
	}
	if( g_pData->GetServerProtocol()->ConnectedToChannelServer( g_pMain->GetPickedChannelServer().m_kServerIP.c_str(), g_pMain->GetPickedChannelServer().m_usMasterPort ) == true )
	{
		if(g_pData->GetServerProtocol()->IsChConnected())
		{
			g_pData->GetServerProtocol()->SendChID(ECH_GET_SERVER_TIME_REQ);
			return true;
		}
	}
	return false;
}
bool CX2Main::Handler_ECH_GET_SERVER_TIME_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	SAFE_DELETE_DIALOG( m_pDLGStateChange );
	KSerBuffer* pBuff = (KSerBuffer*)lParam;	
	KECH_GET_SERVER_TIME_ACK kEvent;
	DeSerialize( pBuff, &kEvent );
	m_TempTime = kEvent.m_wstrCurrentTime;
	m_bMappingResult = true;
	StateChangeTimeSleep(0.0f);
	return true;
}
void CX2Main::StateChangeTimeSleep(float nSleepTime)
{
	m_GetTimeCount = m_GetTimeCount + nSleepTime;
	if(m_GetTimeCount > 0.5f)
	{
		m_SumCount++;
		m_GetTimeCount = 0.0f;
		if(m_SumCount == 2)
		{
			m_bMappingResult = true;
			if(m_bMappingResult == true)
			{
				if(m_bOneChange == false)
				{
					CTime TmepTime = CTime::GetCurrentTime();
					m_TempTime = TmepTime.Format(_T("%Y-%m-%d %H:%M:%S"));
					g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->SetServerCurrentTime( m_TempTime );
					g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->MassFileMapping();
					g_pKTDXApp->SendGameMessage( XGM_STATE_CHANGE, CX2Main::XS_LOGIN, NULL, false );
					m_bMappingResult = false;
					m_bOneChange = true;
					StateLog(L"�����ð� �ޱ� ����..�𵩾���??");
				}
			}
		}
		else if(m_SumCount > 100)
		{
			m_SumCount = 3;
		}
	}
	if(m_bMappingResult == true)
	{
		if(m_bOneChange == false)
		{
			g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->SetServerCurrentTime( m_TempTime );
			g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->MassFileMapping();
			g_pKTDXApp->SendGameMessage( XGM_STATE_CHANGE, CX2Main::XS_LOGIN, NULL, false );
			m_bMappingResult = false;
			m_bOneChange = true;
			StateLog(L"�����ð� �޾Ƽ� ���μ���!!");
		}
	}
}
#endif  SERV_SERVER_TIME_GET

wstring CX2Main::GetEDString( int edToChange )
{
	WCHAR buff[256] = {0};
	_itow( edToChange, buff, 10 );

	return GetEDString( buff );
}
wstring CX2Main::GetEDString( const WCHAR* pEDToChange )
{
	wstringstream edString;
	wstring edToChange = pEDToChange;

	int commaCount = 0;
	for( int i = edToChange.size()-1; i >=0; i-- )
	{
		if( commaCount == 3 )
		{
			edString << L",";
			commaCount = 0;
		}
		edString << edToChange.at(i);
		commaCount++;		
	}
	edToChange = edString.str().c_str();


	std::reverse( edToChange.begin(), edToChange.end() );

	return edToChange;
}


// ED ���ڿ��� ���ڷ� �ٲ�, �߰��� ','�� ����
int CX2Main::GetEDFromString( const WCHAR* wszED )
{
	if( NULL == wszED )
		return 0;

	wstring wstrED = wszED;
	for( UINT i=0; i<wstrED.size(); ++i )
	{
		if( wstrED[i] == ',' )
		{
			wstrED.erase( wstrED.begin() + i );
			i--;
		}
	}

	return (int) wcstol( wstrED.c_str(), NULL, 10 );
}

//{{ ����� : [2009/11/5] //	���� Į��
#ifdef PRICE_COLOR
D3DXCOLOR	CX2Main::GetPriceColor( __int64 iPrice )
{
	D3DXCOLOR color;

	if( iPrice < 10000 )	//	��
	{
		color = D3DXCOLOR( 0.9f, 1.0f, 0.1f, 1.0f );
	}
	else if ( iPrice < 100000 )	//	��
	{
		color = D3DXCOLOR( 0.5f, 1.0f, 0.0f, 1.0f );
	}
	else if ( iPrice < 1000000 )	//	��Ȳ
	{
		color = D3DXCOLOR( 1.0f, 0.5f, 0.0f, 1.0f );
	}
	else if ( iPrice < 10000000 )	// ��
	{
		color = D3DXCOLOR( 0.0f, 1.0f, 1.0f, 1.0f );
	}
	else if ( iPrice < 100000000 )	// ����
	{
		color = D3DXCOLOR( 0.55f, 0.55f, 0.96f, 1.0f );
	}
	else	//	����
	{
		color = D3DXCOLOR( 1.0f, 0.0f, 0.0f, 1.0f );
	}

	return color;
}

#endif	//	PRICE_COLOR
//}} ����� : [2009/11/5] //	���� Į�� �Լ�




void CX2Main::InitMyUnitSkillTree()
{
	if( NULL == g_pData ||
		NULL == g_pData->GetMyUser() ||
		NULL == g_pData->GetMyUser()->GetSelectUnit() )
	{
		return;
	}

	if( g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_DEV )
	{
		CX2State* pState = (CX2State*) g_pMain->GetNowState();
		pState->Handler_EGS_ADMIN_INIT_SKILL_TREE_REQ();
	}
}


void CX2Main::SetMyUnitSP( int iSP )
{
	if( NULL == g_pData ||
		NULL == g_pData->GetMyUser() ||
		NULL == g_pData->GetMyUser()->GetSelectUnit() )
	{
		return;
	}

#ifdef LIGHT_OPERATOR_ACCOUNT
	if( g_pData->GetMyUser()->GetAuthLevel() == CX2User::XUAL_LIGHT_OPERATOR )
	{
		return;
	}
#endif LIGHT_OPERATOR_ACCOUNT
	
	if( g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_OPERATOR )
	{
		CX2State* pState = (CX2State*) g_pMain->GetNowState();
		pState->Handler_EGS_ADMIN_CHANGE_SKILL_POINT_REQ( iSP );
	}
}

#ifdef SERV_ADMIN_CHEAT_GET_ALL_SKILL 

void CX2Main::FullMyUnitSkillTree()
{
	if( NULL == g_pData ||
		NULL == g_pData->GetMyUser() ||
		NULL == g_pData->GetMyUser()->GetSelectUnit() )
	{
		return;
	}

	if( g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_DEV )
	{
		CX2State* pState = (CX2State*) g_pMain->GetNowState();
		pState->Handler_EGS_ADMIN_CHEAT_GET_ALL_SKILL_REQ();
	}
}
#endif //SERV_ADMIN_CHEAT_GET_ALL_SKILL 

#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
/** @function	: FullMyUnitSkillTree
	@brief		: ���� ������ �ø� �� �ִ� ��ų Ȱ��ȭ
*/
void CX2Main::FullMyUnitSkillTreeByLevel()
{
	if( NULL == g_pData ||
		NULL == g_pData->GetMyUser() ||
		NULL == g_pData->GetMyUser()->GetSelectUnit() )
	{
		return;
	}

	if( g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_DEV )
	{
		CX2State* pState = (CX2State*) g_pMain->GetNowState();
		pState->Handler_EGS_ADMIN_AUTO_GET_ALL_SKILL_REQ();
	}
}

/** @function	: SetMyUnitSkillLevel
	@brief		: Ư�� ��ų�� ������ ����
*/
void CX2Main::SetMyUnitSkillLevel( const int iSkillID, const int iSkillLevel )
{
	if( NULL == g_pData ||
		NULL == g_pData->GetMyUser() ||
		NULL == g_pData->GetMyUser()->GetSelectUnit() )
	{
		return;
	}

	if( g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_DEV )
	{
		CX2State* pState = (CX2State*) g_pMain->GetNowState();
		pState->Handler_EGS_ADMIN_GET_SKILL_REQ( iSkillID, iSkillLevel );
	}
}
#endif // UPGRADE_SKILL_SYSTEM_2013

void CX2Main::SendStatisticsLocLog()
{
	if ( g_pData == NULL )
		return;

	if ( g_pData->GetMyUser() == NULL )
		return;

#ifdef LIGHT_OPERATOR_ACCOUNT
	if( g_pData->GetMyUser()->GetAuthLevel() == CX2User::XUAL_LIGHT_OPERATOR )
	{
		return;
	}
#endif LIGHT_OPERATOR_ACCOUNT

	if ( g_pData->GetMyUser()->GetAuthLevel() < CX2User::XUAL_DEV )
		return;
	
	g_pData->GetServerProtocol()->SendID( EGS_SEND_STATISTICS_LOCLOG_REQ );
}

void CX2Main::SendStatisticsDBLog()
{
	if ( g_pData == NULL )
		return;

	if ( g_pData->GetMyUser() == NULL )
		return;

#ifdef LIGHT_OPERATOR_ACCOUNT
	if( g_pData->GetMyUser()->GetAuthLevel() == CX2User::XUAL_LIGHT_OPERATOR )
	{
		return;
	}
#endif LIGHT_OPERATOR_ACCOUNT

	if ( g_pData->GetMyUser()->GetAuthLevel() < CX2User::XUAL_DEV )
		return;

	g_pData->GetServerProtocol()->SendID( EGS_SEND_STATISTICS_DBLOG_REQ );
}

void CX2Main::BuySuperEqipLevel_LUA( int level )
{
	if ( g_pData == NULL )
		return;

	if ( g_pData->GetMyUser() == NULL )
		return;

#ifdef LIGHT_OPERATOR_ACCOUNT
	if( g_pData->GetMyUser()->GetAuthLevel() == CX2User::XUAL_LIGHT_OPERATOR )
	{
		return;
	}
#endif LIGHT_OPERATOR_ACCOUNT

	if ( g_pData->GetMyUser()->GetAuthLevel() < CX2User::XUAL_OPERATOR )
		return;

	CX2Unit* pCX2Unit = g_pData->GetMyUser()->GetSelectUnit();
	if( pCX2Unit != NULL )
	{
#ifdef  X2OPTIMIZE_ITEM_TEMPLET_PREPROCESSING
        CX2Unit::UNIT_CLASS eUnitClass = pCX2Unit->GetClass();
        unsigned uNumItemTemplets = g_pData->GetItemManager()->GetItemNum();
        for( unsigned int i = 0; i != uNumItemTemplets; ++i )
        {
            const CX2Item::ItemTemplet* pTemplet = g_pData->GetItemManager()->GetItemTempletFromIndex( i );
            if ( pTemplet == NULL || pTemplet->GetUnitClass() != eUnitClass || pTemplet->GetUseLevel() != level )
                continue;
			BuyItemSuper_LUA( pTemplet->GetItemID() );
        }
#else   //X2OPTIMIZE_ITEM_TEMPLET_PREPROCESSING
		vector<const CX2Item::ItemTemplet*> itemList;
		int num = g_pData->GetItemManager()->SerchByUnit( itemList, pCX2Unit->GetClass() );
		for( int i = 0; i < (int)itemList.size(); i++ )
		{
			const CX2Item::ItemTemplet* pItemTemplet = itemList[i];
			if( pItemTemplet != NULL )
			{
				if( pItemTemplet->GetUseLevel() == level )
				{
					BuyItemSuper_LUA( pItemTemplet->GetItemID() );
				}
			}
		}
#endif  //X2OPTIMIZE_ITEM_TEMPLET_PREPROCESSING
	}
}


void CX2Main::StateInLog()
{
	if( NULL == g_pMain )
		return;

	switch( m_NowStateID )
	{
	default:
	case XS_INVALID:
		StateLog( L"XS_INVALID");
		break;

	case XS_START_UP:
		StateLog( L"XS_START_UP");
		break;
		
	case XS_LOGIN:
		StateLog( L"XS_LOGIN");
		break;

	case XS_JOIN:
		StateLog( L"XS_JOIN");
		break;

	case XS_LOADING:
		StateLog( L"XS_LOADING");
		break;

#ifdef ELSWORD_NEW_BEGINNING
	case XS_BEGINNING:
		StateLog( L"XS_SERVER_SELECT");
		break;

#else
	case XS_SERVER_SELECT:
		StateLog( L"XS_SERVER_SELECT");
		break;
#endif ELSWORD_NEW_BEGINNING

	case XS_CREATE_UNIT:
		StateLog( L"XS_CREATE_UNIT");
		break;

	case XS_PVP_LOBBY:
		StateLog( L"XS_PVP_LOBBY");
		break;

	case XS_PVP_ROOM:
		StateLog( L"XS_PVP_ROOM");
		break;

	case XS_PVP_GAME:
		StateLog( L"XS_PVP_GAME");
		break;

	case XS_DUNGEON_ROOM:
		StateLog( L"XS_DUNGEON_ROOM");
		break;

	case XS_DUNGEON_GAME:
		StateLog( L"XS_DUNGEON_GAME");
		break;

	case XS_PVP_RESULT:
		StateLog( L"XS_PVP_RESULT");
		break;

	case XS_DUNGEON_RESULT:
		StateLog( L"XS_DUNGEON_RESULT");
		break;

	case XS_VILLAGE_MAP:
		StateLog( L"XS_VILLAGE_MAP");
		break;

	case XS_SQUARE_GAME:
		StateLog( L"XS_SQUARE_GAME");
		break;

	case XS_TRAINING_SCHOOL:
		StateLog( L"XS_TRAINING_SCHOOL");
		break;

	case XS_TRAINING_GAME:
		StateLog( L"XS_TRAINING_GAME");
		break;

	case XS_TRAINING_RESULT:
		StateLog( L"XS_TRAINING_RESULT");
		break;

	case XS_BATTLE_FIELD:
		StateLog( L"XS_BATTLE_FIELD");
		break;

#ifdef ADDED_RELATIONSHIP_SYSTEM
	case XS_WEDDING_GAME:
		StateLog( L"XS_WEDDING_GAME");
		break;
#endif // ADDED_RELATIONSHIP_SYSTEM
	}
}




void CX2Main::IntentionalCrash()
{
	if( NULL != g_pData && 
		NULL != g_pData->GetMyUser() &&
		g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_DEV )
	{
#ifdef LIGHT_OPERATOR_ACCOUNT
		if( g_pData->GetMyUser()->GetAuthLevel() == CX2User::XUAL_LIGHT_OPERATOR )
		{
			return;
		}
#endif LIGHT_OPERATOR_ACCOUNT
        int* x = NULL;	
		*x = 1;
	}
}

void CX2Main::SetFreeCamera( bool bEnable )
{
#ifdef LIGHT_OPERATOR_ACCOUNT
	if( g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetAuthLevel() == CX2User::XUAL_LIGHT_OPERATOR )
	{
		return ;
	}
#endif LIGHT_OPERATOR_ACCOUNT

	switch( g_pMain->GetNowStateID() )
	{
	case XS_VILLAGE_MAP:
		{
			if( g_pTFieldGame != NULL )
			{
				g_pTFieldGame->SetFreeCamera( bEnable );
			}

		} break;

	case XS_SQUARE_GAME:
		{
			if( g_pSquareGame != NULL )
			{
				g_pSquareGame->SetFreeCamera( bEnable );
			}
		} break;
#ifdef ADDED_RELATIONSHIP_SYSTEM
	case XS_WEDDING_GAME:
		{
			if( g_pWeddingGame != NULL )
			{
				g_pWeddingGame->SetFreeCamera( bEnable );
			}
		} break;
#endif // ADDED_RELATIONSHIP_SYSTEM

	case XS_TRAINING_GAME:
	case XS_PVP_GAME:
	case XS_DUNGEON_GAME:
	case XS_BATTLE_FIELD:
		{
			if( g_pX2Game != NULL )
			{
				g_pX2Game->SetFreeCamera( bEnable );
			}

		} break;
	}
}
bool CX2Main::GetFreeCamera() const
{
	switch( g_pMain->GetNowStateID() )
	{
	case XS_VILLAGE_MAP:
		{
			if( g_pTFieldGame != NULL )
			{
				return g_pTFieldGame->GetFreeCamera();
			}

		} break;

	case XS_SQUARE_GAME:
		{
			if( g_pSquareGame != NULL )
			{
				return g_pSquareGame->GetFreeCamera();
			}
		} break;
#ifdef ADDED_RELATIONSHIP_SYSTEM
	case XS_WEDDING_GAME:
		{
			if( g_pWeddingGame != NULL )
			{
				return g_pWeddingGame->GetFreeCamera();
			}
		} break;
#endif // ADDED_RELATIONSHIP_SYSTEM

	case XS_TRAINING_GAME:
	case XS_PVP_GAME:
	case XS_DUNGEON_GAME:
	case XS_BATTLE_FIELD:
		{
			if( g_pX2Game != NULL )
			{
				return g_pX2Game->GetFreeCamera();
			}
		} break;
	}

	return false;
}

#ifdef KEYFRAME_CAMERA

void CX2Main::SetWorldCameraEdit( bool bEnable)
{
	switch( g_pMain->GetNowStateID() )
	{
	case XS_VILLAGE_MAP:
		{
			if( g_pTFieldGame != NULL )
			{
				//g_pTFieldGame->SetFreeCamera( bEnable );
			}

		} break;

	case XS_SQUARE_GAME:
		{
			if( g_pSquareGame != NULL )
			{
				//g_pSquareGame->SetFreeCamera( bEnable );
			}

		} break;

	case XS_TRAINING_GAME:
	case XS_PVP_GAME:
	case XS_DUNGEON_GAME:
		{
			if( g_pX2Game != NULL )
			{
				g_pX2Game->SetWorldCameraEdit( bEnable );
			}
		} break;
	}


	return;
}



void CX2Main::PlayWorldCamera( int iWorldCameraID )
{
	switch( g_pMain->GetNowStateID() )
	{
	case XS_VILLAGE_MAP:
		{
			if( g_pTFieldGame != NULL )
			{
				//g_pTFieldGame->SetFreeCamera( bEnable );
			}

		} break;

	case XS_SQUARE_GAME:
		{
			if( g_pSquareGame != NULL )
			{
				//g_pSquareGame->SetFreeCamera( bEnable );
			}

		} break;

	case XS_TRAINING_GAME:
	case XS_PVP_GAME:
	case XS_DUNGEON_GAME:
		{
			if( g_pX2Game != NULL )
			{
				g_pX2Game->PlayWorldCamera(iWorldCameraID );
			}
		} break;
	}


}
void CX2Main::EndWorldCamera()
{
	if( g_pX2Game != NULL && g_pX2Game->GetX2Camera() != NULL)
	{
		if(g_pX2Game->GetX2Camera()->GetEnableWorldCamera() == true)
		{
			g_pX2Game->GetX2Camera()->EndWorldCamera();
		}
	}
	return;
}

void CX2Main::SetProjectionFov( float fFov )
{
	g_pKTDXApp->GetDGManager()->SetProjectionFov( g_pKTDXApp->GetDGManager()->GetNear(),
		g_pKTDXApp->GetDGManager()->GetFar(), fFov );
	return;
}

#endif KEYFRAME_CAMERA

D3DXVECTOR2 CX2Main::GetWindowPos( D3DXVECTOR2 windowSize, D3DXVECTOR2 wantMousePoint )
{
	/*
	D3DXVECTOR2 resultPos = D3DXVECTOR2( 0, 0 );

	POINT point;
	GetCursorPos( &point );
	ScreenToClient( g_pKTDXApp->GetHWND(), &point );


	windowSize = g_pKTDXApp->ConvertByResolution( windowSize );
	wantMousePoint = g_pKTDXApp->ConvertByResolution( wantMousePoint );

	resultPos.x = (float)point.x;
	resultPos.y = (float)point.y;

	resultPos = g_pKTDXApp->MouseConvertByResolution( resultPos );

	resultPos -= wantMousePoint;




	if ( resultPos.x < 0 )
	{
		resultPos.x = 0;
	}
	else if ( resultPos.x + windowSize.x > 1024 )
	{
		resultPos.x = 1024 - windowSize.x;
	}
	
	if ( resultPos.y < 0 )
	{
		resultPos.y = 0;
	}
	else if ( resultPos.y + windowSize.y > 768 )
	{
		resultPos.y = 768 - windowSize.y;
	}

	return resultPos;
	*/


	D3DXVECTOR2 resultPos = D3DXVECTOR2( 0, 0 );

	POINT point;
	GetCursorPos( &point );
	ScreenToClient( g_pKTDXApp->GetHWND(), &point );

	resultPos.x = (float)point.x;
	resultPos.y = (float)point.y;

	resultPos = g_pKTDXApp->MouseConvertByResolution( resultPos );

	resultPos -= wantMousePoint;




	if ( resultPos.x < 0 )
	{
		resultPos.x = 0;
	}
	else if ( resultPos.x + windowSize.x > 1024 )
	{
		resultPos.x = 1024 - windowSize.x;
	}

	if ( resultPos.y < 0 )
	{
		resultPos.y = 0;
	}
	else if ( resultPos.y + windowSize.y > 768 )
	{
		resultPos.y = 768 - windowSize.y;
	}

	return resultPos;
}

void CX2Main::OpenURL( WCHAR* pURL )
{
	// ������ �ִ� URL�� ������´�.
	HINTERNET hURLFile;
	if ( NULL == ( hURLFile = ::InternetOpenUrl( m_hInternet,
		pURL,
		NULL,
		0,
		INTERNET_FLAG_RELOAD |
		INTERNET_FLAG_NO_CACHE_WRITE,
		0 ) ) )
	{
		ErrorLog( XEM_ERROR107 );
		return;
	}

	::InternetCloseHandle( hURLFile );
}

void CX2Main::CreateStateChangeDLG( const WCHAR* szExplanation )
{
	SAFE_DELETE_DIALOG( m_pDLGStateChange );
	m_pDLGStateChange = KTDGUIMsgBox( D3DXVECTOR2(250,300), szExplanation, g_pMain->GetNowState() );
	m_pDLGStateChange->MoveStop();
	m_pDLGStateChange->SetColor( D3DXCOLOR( 1,1,1,1 ) );

	if ( m_pNowState != NULL )
	{
#ifdef DXUT_KTDXLIB_SAFE_ON_RESET
		if ( DXUTGetDeviceLost() == false && DXUTGetDeviceObjectsReset() == true )
		{
			g_pKTDXApp->GetDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x32323232, 1.0f, 0);
			g_pKTDXApp->GetDevice()->BeginScene();
			m_pNowState->OnFrameRender();
			g_pKTDXApp->GetDevice()->EndScene();
			HRESULT hr = g_pKTDXApp->GetDevice()->Present( NULL, NULL, NULL, NULL );

			if ( FAILED(hr) )
			{
				if ( D3DERR_DEVICELOST == hr )
				{
					DXUTSetDeviceLost( true );
				}
				else if ( D3DERR_DRIVERINTERNALERROR == hr )
				{
					// When D3DERR_DRIVERINTERNALERROR is returned from Present(),
					// the application can do one of the following:
					// 
					// - End, with the pop-up window saying that the application cannot continue 
					//   because of problems in the display adapter and that the user should 
					//   contact the adapter manufacturer.
					//
					// - Attempt to restart by calling IDirect3DDevice9::Reset, which is essentially the same 
					//   path as recovering from a lost device. If IDirect3DDevice9::Reset fails with 
					//   D3DERR_DRIVERINTERNALERROR, the application should end immediately with the message 
					//   that the user should contact the adapter manufacturer.
					// 
					// The framework attempts the path of resetting the device
					// 
					DXUTSetDeviceLost( true );
				}
			}
		}
#else
		g_pKTDXApp->GetDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x32323232, 1.0f, 0);
		g_pKTDXApp->GetDevice()->BeginScene();
		m_pNowState->OnFrameRender();
		g_pKTDXApp->GetDevice()->EndScene();
		g_pKTDXApp->GetDevice()->Present( NULL, NULL, NULL, NULL );
#endif
	}
	
}

void CX2Main::DeleteStateChangeDLG()
{
	if ( m_pDLGStateChange != NULL )
		g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGStateChange, NULL, false );

	m_pDLGStateChange = NULL;
}



void CX2Main::DeleteOtherFile( WCHAR* pExtention, vector<wstring>& fileList )
{
//#ifdef _SERVICE_

	WCHAR buff[MAX_PATH] = {0};
	GetCurrentDirectory( MAX_PATH, buff );	
	DeleteOtherFile( pExtention, fileList, buff );	

//#endif
}


void CX2Main::DeleteOtherFile( WCHAR* pExtention, vector<wstring>& fileList, WCHAR* wstrFolder )
{
//#ifdef _SERVICE_

	HANDLE				hSearch;
	WIN32_FIND_DATA		fd;
	WCHAR				wstrSearchPath[256];


	wcscpy( wstrSearchPath, wstrFolder);
	wcscat( wstrSearchPath, L"\\*.*");

	hSearch = FindFirstFile( wstrSearchPath, &fd );

	if(hSearch == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if( wcscmp(fd.cFileName, L".") && wcscmp(fd.cFileName, L"..") )
		{
			if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				WCHAR	wstrNewSearchDir[256];

				wcscpy( wstrNewSearchDir, wstrFolder );
				wcscat( wstrNewSearchDir, L"\\");
				wcscat( wstrNewSearchDir, fd.cFileName );

				DeleteOtherFile( pExtention, fileList, wstrNewSearchDir );
			}
			else
			{
				WCHAR			wstrNewSearchFile[256];
				std::wstring	wstrFile;
				std::wstring	wstrFileName;

				wcscpy( wstrNewSearchFile, wstrFolder );
				wcscat( wstrNewSearchFile, L"\\");
				wcscat( wstrNewSearchFile, fd.cFileName );

				wstrFile = wstrNewSearchFile;
				wstrFileName = fd.cFileName;

				bool bCheck = false;

				wstring nowFileName = wstrFileName;
				MakeUpperCase( nowFileName );

				for ( int i = 0; i < (int)fileList.size(); i++ )	//���������� �������� �˻�
				{
					wstring tempFileName = fileList[i];
					MakeUpperCase( tempFileName );

					if ( tempFileName == nowFileName )
					{
						bCheck = true;
						break;
					}
				}

				if ( bCheck == false )	//�������� ���̶��
				{
					//�Ⱦ��� kom���� ���ִ°�(������ �̸� �ٲٴ°�) ���� �κ���
					int strIndex = wstrFileName.rfind( L"." );

					if ( strIndex != -1 )
					{
						wstring extName = wstrFileName.substr( strIndex + 1 );


						for( std::wstring::iterator i = extName.begin(); i != extName.end(); i++ ) 
							*i = (WCHAR)towupper(*i);

						wstring tempExtName = pExtention;
						MakeUpperCase( tempExtName );

						if ( extName == tempExtName )
						{
							//wstring tempNewFile = wstrNewSearchFile;
							::DeleteFile( wstrNewSearchFile );
						}
					}
				}
			}
		}

	}while( FindNextFile( hSearch, &fd ) );

	FindClose(hSearch);

//#endif
}

wstring CX2Main::GetStrByLienBreak( const WCHAR* pOrgStr, int width, int fontIndex )
{
	if ( pOrgStr == NULL )
		return L"";

	wstring tempText = pOrgStr;
	const int constTextMaxLen = (int)(g_pKTDXApp->GetResolutionScaleX() * width);

	int nowTextLen = 0;

	CKTDGFontManager::CUKFont* pFont = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetUKFont( fontIndex );
	if( pFont != NULL )
	{
		for ( int i = 0; i < (int)tempText.size(); i++ )
		{
			WCHAR tempWchar = tempText[i];
			if ( nowTextLen < constTextMaxLen && tempWchar == L'\n' )
			{
				nowTextLen = 0;
				continue;
			}

			if ( tempText[i] == L'#' && i + 1 < (int)tempText.size() && ( tempText[i+1] == L'C' || tempText[i+1] == L'c' ) )
			{
				if ( i + 2 < (int)tempText.size() )
				{
					if( tempText[i+2] == L'x' || tempText[i+2] == L'X' )
					{
						i += 2;
					}
					else
					{
						i += 7;
					}
					continue;
				}
			}

			int iUniCharSize = 0;

			iUniCharSize = pFont->GetWidth( tempWchar );
			nowTextLen += iUniCharSize;

			if ( nowTextLen >= constTextMaxLen )
			{
				nowTextLen = 0;
				wstring enterChar = L"\n";
				if ( i + 1 < (int)tempText.size() ) 
					tempText.insert( tempText.begin() + i + 1, enterChar.begin(), enterChar.end() );
				else
					tempText.insert( tempText.begin() + i, enterChar.begin(), enterChar.end() );
				i++;
#ifdef REMOVE_FIRST_SPACE_AFTER_LINE_BREAK
				if( tempText[i + 1] == L' ' )
					tempText.erase( tempText.begin() + i + 1 );
#endif REMOVE_FIRST_SPACE_AFTER_LINE_BREAK
			}

			if ( i >= (int)tempText.size() )
			{
				break;
			}
		}


	}

	return tempText;
}

#ifdef FIX_TOOLTIP
wstring CX2Main::GetStrByLineBreakColor( const WCHAR* pOrgStr, int width, int fontIndex )
{
	if ( pOrgStr == NULL )
		return L"";

	wstring tempText = pOrgStr;
	const int constTextMaxLen = (int)(g_pKTDXApp->GetResolutionScaleX() * width);

	int nowTextLen = 0;

	CKTDGFontManager::CUKFont* pFont = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetUKFont( fontIndex );
	if( pFont != NULL )
	{
		wstring wstrPrevColorkey = L"";
		for ( int i = 0; i < (int)tempText.size(); i++ )
		{
			WCHAR tempWchar = tempText[i];
			if ( nowTextLen <= constTextMaxLen && tempWchar == L'\n' )
			{
				nowTextLen = 0;
				tempText.insert( tempText.begin() + i + 1, wstrPrevColorkey.begin(), wstrPrevColorkey.end() );
				wstrPrevColorkey = L"";
				continue;
			}

			if ( tempText[i] == L'#' && i + 1 < (int)tempText.size() && ( tempText[i+1] == L'C' || tempText[i+1] == L'c' ) )
			{
				if ( i + 2 < (int)tempText.size() )
				{
					if( tempText[i+2] == L'x' || tempText[i+2] == L'X' )
					{
						wstrPrevColorkey = L"";
						i += 2;
					}
					else
					{
						wstrPrevColorkey = tempText.substr(i, 8);
						i += 7;
					}
					continue;
				}
			}

			int iUniCharSize = 0;

			iUniCharSize = pFont->GetWidth( tempWchar );
			nowTextLen += iUniCharSize;

			if ( nowTextLen >= constTextMaxLen )
			{
				wstring enterChar = L"\n";
				nowTextLen = 0;

				if ( i + 1 <= (int)tempText.size() )
				{	
					if ( tempText[i + 1] != L'\n' )
					{
						tempText.insert( tempText.begin() + i + 1, enterChar.begin(), enterChar.end() );

						// ���� ���� ù ���ڰ� ������ ��� ����
						if ( i + 2 < (int)tempText.size() && tempText[i + 2] == L' ' )
						{
							tempText.erase( i + 2, 1 );
						}
					}

					if(wstrPrevColorkey != L"")
						{
							tempText.insert( tempText.begin() + i + 2, wstrPrevColorkey.begin(), wstrPrevColorkey.end() );
							wstrPrevColorkey = L"";
						}
				}
				else
				{
					if ( tempText[i] != L'\n' && tempText[i] != L')' && tempText[i] != L'.' )
					{
						tempText.insert( tempText.begin() + i, enterChar.begin(), enterChar.end() );

						if(wstrPrevColorkey != L"")
						{
							tempText.insert( tempText.begin() + i + 1, wstrPrevColorkey.begin(), wstrPrevColorkey.end() );
							wstrPrevColorkey = L"";
						}
					}
					nowTextLen = 0;
				}
				i++;
			}

			if ( i >= (int)tempText.size() )
			{
				break;
			}
		}
	}

	return tempText;
}
#endif FIX_TOOLTIP


void CX2Main::ResetLuaBinder( KLuabinder* pKLuabinder )
{
	if ( pKLuabinder == NULL )
		return;

	g_pKTDXApp->RegisterLuabind( pKLuabinder );
	RegisterLuabind( pKLuabinder );
	LoadLuaEnum( pKLuabinder );
	
}


void CX2Main::UpdateProcessList()
{
#ifdef PROCESSLIST
	wstring wstrTemp;
	DWORD aProcesses[1024], cbNeeded;

	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
		return;

#ifndef _SERVICE_
	FILE *fp = NULL;
	fp = fopen("ProcessList.Log", "w");
#endif

	DWORD cProcesses = cbNeeded / sizeof(DWORD);
	for ( DWORD PidFor = 0x0c; PidFor < 0xFFFF; PidFor +=4 )
	{
		if( PidFor == GetCurrentProcessId() )
			continue;

		HANDLE hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION |
			PROCESS_VM_READ,
			FALSE, PidFor );

		if (hProcess)
		{
			BOOL sHide = false;
			unsigned int i;
			WCHAR szName[MAX_PATH]= L"<Unknown>";
			HMODULE hModule;

			for ( i = 1; i <= cProcesses; i++ )
			{
				if (PidFor == aProcesses[i])
				{
					sHide = true;
					break;
				}

			}

			if(EnumProcessModules(hProcess,&hModule,sizeof(hModule),&cbNeeded))
			{
				GetModuleFileNameEx(hProcess,hModule,szName,ARRAY_SIZE(szName));
				wstrTemp = szName;
				MakeUpperCase( wstrTemp );
				
				if(!sHide)
					wstrTemp = wstrTemp + L" [Hidden] ";

				m_setProcessNameToFindHack.insert( wstrTemp );				

#ifndef _SERVICE_
				fwprintf(fp, L"%-5d - %16s  %s\n", PidFor, szName, (sHide) ? L"" : L"--[Hidden]--");
#endif
			}
			else
			{				
				GetProcessImageFileName(hProcess,szName,ARRAY_SIZE(szName));
				wstrTemp = szName;
				MakeUpperCase( wstrTemp );				

				wstrTemp = L"[Zombie] " + wstrTemp + L" \r\n ";
				m_setProcessNameToFindHack.insert( wstrTemp );						

#ifndef _SERVICE_
				fwprintf(fp, L"%-5d - %16s  %s\n", PidFor, szName, L"--[Zombie]--");
#endif
			}
		}

		CloseHandle( hProcess );
	}

#ifndef _SERVICE_
	fclose(fp);
#endif

#else
	HANDLE hSnapShot;
	PROCESSENTRY32W pEntry;

	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL,NULL);

	pEntry.dwSize =sizeof(pEntry);

	wstring wstrTemp;

	// �������� ���μ������� ù���� ������ �����´�. 
	BOOL hRes = Process32First( hSnapShot, &pEntry );

	while(hRes)
	{
		wstrTemp = pEntry.szExeFile;
		MakeUpperCase( wstrTemp );

		if ( m_setSendedProcessList.find( wstrTemp ) == m_setSendedProcessList.end() )
			m_setProcessNameToFindHack.insert( wstrTemp );		

		//������ ���μ����� ������ �����´�.
		hRes = Process32Next( hSnapShot, &pEntry );
	}
#endif

}




#ifdef IN_HOUSE_PLAY_LOG_TEST
	
	void CX2Main::SendInHousePlayLogMail( char* pReceiverAddress )
	{
		if( NULL == pReceiverAddress )
			return;

		g_pX2SMTPMail->SetSenderEMail( pReceiverAddress );
		g_pX2SMTPMail->SetReceiverEMail( pReceiverAddress );

		string senderName = "unknown";
		if( NULL != g_pInstanceData )
		{
			ConvertWCHARToChar( senderName, g_pInstanceData->GetUserID().c_str() );
		}

		g_pX2SMTPMail->SetSenderName( (char*)senderName.c_str() );
		g_pX2SMTPMail->SetSubject( "in-house game play log" );
		

		g_pX2SMTPMail->ClearFile();			
		std::vector< std::string > vecFiles;

		char* cwd = NULL;
		if( (cwd = _getcwd( NULL, 255 )) != NULL )
		{
			std::string strMailDesc = "";
			std::string filename = "";



			filename = cwd;
			filename += "\\InHouse.log";
			g_pX2SMTPMail->AddFile( filename );
			{
				char szText[4096] = "";
				std::ifstream ifs;
				ifs.open( filename.c_str() );
				if( true == ifs.is_open() )
				{
					ifs.read( szText, sizeof(szText) );
					ifs.close();
				}
				strMailDesc += "------------------------------------------\n";
				strMailDesc += szText;
			}




			filename = cwd;
			filename += "\\ErrorLog.txt";
			g_pX2SMTPMail->AddFile( filename );
			{
				char szText[4096] = "";
				std::ifstream ifs;
				ifs.open( filename.c_str() );
				if( true == ifs.is_open() )
				{
					ifs.read( szText, sizeof(szText) );
					ifs.close();
				}
				strMailDesc += "------------------------------------------\n";
				strMailDesc += szText;
			}



			filename = cwd;
			filename += "\\State.log";
			g_pX2SMTPMail->AddFile( filename );
			{
				char szText[4096] = "";
				std::ifstream ifs;
				ifs.open( filename.c_str() );
				if( true == ifs.is_open() )
				{
					ifs.read( szText, sizeof(szText) );
					ifs.close();
				}
				strMailDesc += "------------------------------------------\n";
				strMailDesc += szText;
			}




			free(cwd);
			g_pX2SMTPMail->SetDesc( (char*)strMailDesc.c_str() );

		}

		
		DialogBox( g_pKTDXApp->GetHInstance(), (LPCTSTR)IDD_SEND_MAIL_DIALOG, NULL, (DLGPROC)InHousePlayLogMailDlgProc );			
	}

#endif IN_HOUSE_PLAY_LOG_TEST


void CX2Main::SendHackMail()
{
//	ELSWORD_VIRTUALIZER_START
//	THEMIDA_VM_START
//
//
//	char szMail[2][MAX_PATH];
//	strncpy( szMail[0], "demiahun@gmail.com" , MAX_PATH - 1  );		// ����
//	strncpy( szMail[1], "elswordmkt@gmail.com" , MAX_PATH - 1 );	// �̵���
//	//strcpy( szMail[0], "demiahun@gmail.com" );		// ����
//	//strcpy( szMail[1], "elswordmkt@gmail.com" );	// �̵���
//
//	char szID[MAX_PATH] = "";
//	if( g_pInstanceData != NULL &&
//		g_pInstanceData->GetUserID().size() > 0 )
//	{
//		WCHAR wszID[MAX_PATH] = L"";
//		wcscpy( wszID, g_pInstanceData->GetUserID().c_str() );
//		::MultiByteToWideChar( CP_ACP, 0, szID, -1, wszID, MAX_PATH );
//	}
//
//#ifdef PROCESSLIST
//	m_setProcessNameToFindHack.clear();
//#endif
//	if ( m_setProcessNameToFindHack.empty() == false )
//	{
//		for( int i = 0; i < 2; ++i )
//		{
//			//g_pX2SMTPMail->SetSenderEMail( "demiahun@kogstudios.com" );
//			//g_pX2SMTPMail->SetReceiverEMail( "demiahun@kogstudios.com" );
//			g_pX2SMTPMail->SetSenderEMail( szMail[i] );
//			g_pX2SMTPMail->SetReceiverEMail( szMail[i] );
//
//			if( strlen( szID ) > 0 )
//				g_pX2SMTPMail->SetSenderName( szID );
//			else
//				g_pX2SMTPMail->SetSenderName( "���ҵ� ��ŷ ���� ������" ); // TODO : �� string always.h �� �ű��
//
//			string mailName;
//			ConvertWCHARToChar( mailName, m_MailNameToFindHack.c_str() );
//			g_pX2SMTPMail->SetSubject( (char*)mailName.c_str() );
//			stringstream wstrstm;
//
//			set<wstring>::iterator sit;
//			sit = m_setProcessNameToFindHack.begin();
//			for ( ; sit != m_setProcessNameToFindHack.end(); sit++ )
//			{
//				m_setSendedProcessList.insert( *sit );
//				string processName;
//				ConvertWCHARToChar( processName, (*sit).c_str() );
//				wstrstm << processName.c_str() << "\n";
//			}
//			g_pX2SMTPMail->SetDesc( (char*)wstrstm.str().c_str() );
//			g_pX2SMTPMail->SendMail( g_pKTDXApp->GetHWND() );
//		}
//
//
////{{ oasis907 : ����� [2009.12.21] // �� CSmtp
//		if(g_pX2SMTPMail->SMTPGetLastError() != CX2SMTPMail::CSMTP_NO_ERROR)
//		{
//			// Unable to initialize winsock2
//			return;
//		}
//
//#ifdef CHANGE_MAILSTRING
//		g_pX2SMTPMail->SMTPSetServer( m_strMailAddress.c_str(), 25 );
//#else
//		g_pX2SMTPMail->SMTPSetServer( "mail.kog.co.kr", 25 );
//#endif
//		g_pX2SMTPMail->SMTPSetLogin("escrash");            // ���� ����
//		g_pX2SMTPMail->SMTPSetPassword("@Els.123");
//
//		if( strlen( szID ) > 0 )
//			g_pX2SMTPMail->SMTPSetSenderName( szID );
//		else
//			g_pX2SMTPMail->SMTPSetSenderName( "���ҵ� ��ŷ ���� ������" ); // TODO : �� string always.h �� �ű��
//
//
//		g_pX2SMTPMail->SMTPSetSenderMail("escrash@kog.co.kr");
//		g_pX2SMTPMail->SMTPSetReplyTo("escrash@kog.co.kr"); // ȸ�� �ּ�
//
//		string mailName;
//		ConvertWCHARToChar( mailName, m_MailNameToFindHack.c_str() );
//		g_pX2SMTPMail->SMTPSetSubject( (char*)mailName.c_str() );
//		stringstream wstrstm;
//
//		// ���⿡ �޴� ����� ����
//		//g_pX2SMTPMail->SMTPAddRecipient("demiahun@kogstudios.com");
//		g_pX2SMTPMail->SMTPAddRecipient("eshack@kogstudios.com");	
//		//g_pX2SMTPMail->SMTPAddRecipient("oasis907@naver.com");
//		//g_pX2SMTPMail->SMTPAddRecipient("megagame@kog.co.kr");
//		//g_pX2SMTPMail->SMTPAddRecipient("wonpok@kog.co.kr");
//
//		g_pX2SMTPMail->SMTPSetXPriority(CX2SMTPMail::XPRIORITY_NORMAL);
//		g_pX2SMTPMail->SMTPSetXMailer("The Bat! (v3.02) Professional");
//
//#ifdef MORE_INFO_ABOUT_HACK
//		// �ý��� ����
//		AddSystemInfo(wstrstm);
//
//		// DLL ����
//		AddDLLInfo(wstrstm);
//
//		// ���μ��� ����
//		AddProcessInfo(wstrstm);
//#endif // MORE_INFO_ABOUT_HACK
//
//		set<wstring>::iterator sit;
//		sit = m_setProcessNameToFindHack.begin();
//		for ( ; sit != m_setProcessNameToFindHack.end(); sit++ )
//		{
//			m_setSendedProcessList.insert( *sit );
//			string processName;
//			ConvertWCHARToChar( processName, (*sit).c_str() );
//			wstrstm << processName.c_str() << "\n";
//		}
//
//		g_pX2SMTPMail->SMTPSetMessageBody((char*)wstrstm.str().c_str());
//
//		string strUserId = ANTI_HACK_STRING_UserID_Hack_ScreenShot;
//		wstring wstrUserId = ANTI_HACK_WSTRING_UserID_Hack_ScreenShot;
//
//		if( m_strUserIdToFindHack.empty() == false )
//		{
//			ConvertWCHARToChar( strUserId, m_strUserIdToFindHack.c_str() );
//			wstrUserId = m_strUserIdToFindHack;
//		}
//		strUserId = strUserId + ".jpg";
//		wstrUserId = wstrUserId + L".jpg";
//
//		MakeHackScreenShot( strUserId.c_str() );
//		// ���⿡ ÷�� ���� ����
//		//g_pX2SMTPMail->SMTPAddAttachment("c:\\mailtest2.exe");
//		//g_pX2SMTPMail->SMTPAddAttachment("c:\\test2.jpg");	
//		wchar_t szFilePath[1024] = _T("");
//		wchar_t strHackImageBuffer[1024];
//		GetCurrentDirectory( _MAX_PATH, szFilePath );
//		StringCchPrintf(strHackImageBuffer, 1024,  _T("%s\\%s"), szFilePath, wstrUserId.c_str());
//		//wsprintf(strHackImageBuffer, _T("%s\\%s"), szFilePath, wstrUserId.c_str());
//		string strNickName = "";
//		ConvertWCHARToChar( strNickName, strHackImageBuffer );
//
//		g_pX2SMTPMail->SMTPAddAttachment(strNickName.c_str());
//
//		if( g_pX2SMTPMail->SMTPSend() )
//		{
//			//The mail was send successfully
//		}
//		else
//		{
//			//Unable to send the mail
//		}
//		// �޴»�� �ּ�, ÷������ �ʱ�ȭ
//		g_pX2SMTPMail->SMTPClearRecipient();
//
//		
//		DeleteFile( wstrUserId.c_str() );
//
////}} oasis907 : ����� [2009.12.21] // �� CSmtp
//	}
//
//	m_setProcessNameToFindHack.clear();
//
//	THEMIDA_VM_END
//	ELSWORD_VIRTUALIZER_END
}

void CX2Main::SendHackMail_DamageHistory(const char *strComm)
{
////#ifdef _SERVICE_
//	ELSWORD_VIRTUALIZER_START
//	THEMIDA_VM_START
//
//	char szMail[2][MAX_PATH];
//	strncpy( szMail[0], "demiahun@gmail.com" , MAX_PATH - 1 );		// ����
//	strncpy( szMail[1], "elswordmkt@gmail.com" , MAX_PATH - 1 );	// �̵���
//	//strcpy( szMail[0], "demiahun@gmail.com" );		// ����
//	//strcpy( szMail[1], "elswordmkt@gmail.com" );	// �̵���
//
//	char szID[MAX_PATH] = "";
//	if( g_pInstanceData != NULL &&
//		g_pInstanceData->GetUserID().size() > 0 )
//	{
//		WCHAR wszID[MAX_PATH] = L"";
//		wcscpy( wszID, g_pInstanceData->GetUserID().c_str() );
//		::MultiByteToWideChar( CP_ACP, 0, szID, -1, wszID, MAX_PATH );
//	}
///*
//#ifdef PROCESSLIST
//	m_setProcessNameToFindHack.clear();
//#endif
//	if( m_setProcessNameToFindHack.empty() == true )
//		UpdateProcessList();
//
//
//	for( int i = 0; i < 2; ++i )
//	{
//		//g_pX2SMTPMail->SetSenderEMail( "demiahun@kogstudios.com" );
//		//g_pX2SMTPMail->SetReceiverEMail( "demiahun@kogstudios.com" );
//		g_pX2SMTPMail->SetSenderEMail( szMail[i] );
//		g_pX2SMTPMail->SetReceiverEMail( szMail[i] );
//
//		if( strlen( szID ) > 0 )
//			g_pX2SMTPMail->SetSenderName( szID );
//		else
//			g_pX2SMTPMail->SetSenderName( "���ҵ� ��ŷ ���� ������" );
//
//		string mailName;
//		ConvertWCHARToChar( mailName, m_MailNameToFindHack.c_str() );
//		g_pX2SMTPMail->SetSubject( (char*)mailName.c_str() );
//		stringstream wstrstm;
//
//		set<wstring>::iterator sit;
//		sit = m_setProcessNameToFindHack.begin();
//		for ( ; sit != m_setProcessNameToFindHack.end(); sit++ )
//		{
//			m_setSendedProcessList.insert( *sit );
//			string processName;
//			ConvertWCHARToChar( processName, (*sit).c_str() );
//			wstrstm << processName.c_str() << "\n";
//		}
//		g_pX2SMTPMail->SetDesc( (char*)wstrstm.str().c_str() );
//		g_pX2SMTPMail->SendMail( g_pKTDXApp->GetHWND() );
//	}*/
//
//
//	if( g_pX2SMTPMail == NULL )
//	{
//		g_pX2SMTPMail = new CX2SMTPMail;
//	}
//
//	//{{ oasis907 : ����� [2009.12.21] // �� CSmtp
//	if(g_pX2SMTPMail->SMTPGetLastError() != CX2SMTPMail::CSMTP_NO_ERROR)
//	{
//		// Unable to initialize winsock2
//		return;
//	}
//
//#ifdef CHANGE_MAILSTRING
//	g_pX2SMTPMail->SMTPSetServer( m_strMailAddress.c_str(), 25 );
//#else
//	g_pX2SMTPMail->SMTPSetServer( "mail.kog.co.kr", 25 );
//#endif
//	g_pX2SMTPMail->SMTPSetLogin("escrash");            // ���� ����
//	g_pX2SMTPMail->SMTPSetPassword("@Els.123");
//
//	if( strlen( szID ) > 0 )
//		g_pX2SMTPMail->SMTPSetSenderName( szID );
//	else
//		g_pX2SMTPMail->SMTPSetSenderName( "���ҵ� ��ŷ ���� ������" );
//
//
//	g_pX2SMTPMail->SMTPSetSenderMail("escrash@kog.co.kr");
//	g_pX2SMTPMail->SMTPSetReplyTo("escrash@kog.co.kr"); // ȸ�� �ּ�
//
//	string mailName;
//	ConvertWCHARToChar( mailName, m_MailNameToFindHack.c_str() );
//	g_pX2SMTPMail->SMTPSetSubject( (char*)mailName.c_str() );
//	stringstream wstrstm;
//
//	// ���⿡ �޴� ����� ����
//	//g_pX2SMTPMail->SMTPAddRecipient("demiahun@kogstudios.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("eshack@kogstudios.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("carl79@kog.co.kr");	
//	//g_pX2SMTPMail->SMTPAddRecipient("wonpok@kog.co.kr");
//	//g_pX2SMTPMail->SMTPAddRecipient("oasis907@naver.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("megagame@kog.co.kr");
//	g_pX2SMTPMail->SMTPAddRecipient("itexpertkim@kog.co.kr");
//	g_pX2SMTPMail->SMTPAddRecipient("JHKang@kog.co.kr");
//
//	g_pX2SMTPMail->SMTPSetXPriority(CX2SMTPMail::XPRIORITY_NORMAL);
//	g_pX2SMTPMail->SMTPSetXMailer("The Bat! (v3.02) Professional");
//
//	set<wstring>::iterator sit;
//
//#ifdef _IN_HOUSE_
//	wstrstm << "[IN HOUSE] " << strComm <<"\n\n";
//#else
//	wstrstm << strComm <<"\n\n";
//#endif
//	
//
//#ifdef MORE_INFO_ABOUT_HACK
//	// �ý��� ����
//	AddSystemInfo(wstrstm);
//
//	wstrstm << "- Loaded Dll -";
//	const char *dlllist = SiCX2DLLManager()->GetPreLoadedDllList();
//	wstrstm << dlllist;
//
//	// DLL ����
//	AddDLLInfo(wstrstm);
//
//	// ���μ��� ����
//	AddProcessInfo(wstrstm);
//	
//#endif // MORE_INFO_ABOUT_HACK
//
//	g_pX2SMTPMail->SMTPSetMessageBody((char*)wstrstm.str().c_str());
//
//	string strUserId = ANTI_HACK_STRING_UserID_Hack_ScreenShot;
//	wstring wstrUserId = ANTI_HACK_WSTRING_UserID_Hack_ScreenShot;
//
//	if( m_strUserIdToFindHack.empty() == false )
//	{
//		ConvertWCHARToChar( strUserId, m_strUserIdToFindHack.c_str() );
//		wstrUserId = m_strUserIdToFindHack;
//	}
//	strUserId = strUserId + ".jpg";
//	wstrUserId = wstrUserId + L".jpg";
//
//	MakeHackScreenShot( strUserId.c_str() );
//	// ���⿡ ÷�� ���� ����
//	//g_pX2SMTPMail->SMTPAddAttachment("c:\\mailtest2.exe");
//	//g_pX2SMTPMail->SMTPAddAttachment("c:\\test2.jpg");	
//	wchar_t szFilePath[1024] = _T("");
//	wchar_t strHackImageBuffer[1024];
//	GetCurrentDirectory( _MAX_PATH, szFilePath );
//	StringCchPrintf(strHackImageBuffer, 1024,  _T("%s\\%s"), szFilePath, wstrUserId.c_str());
//    //wsprintf(strHackImageBuffer, _T("%s\\%s"), szFilePath, wstrUserId.c_str());
//	string strNickName = "";
//	ConvertWCHARToChar( strNickName, strHackImageBuffer );
//
//	g_pX2SMTPMail->SMTPAddAttachment(strNickName.c_str());
//
//	if( g_pX2SMTPMail->SMTPSend() )
//	{
//		//The mail was send successfully
//	}
//	else
//	{
//		//Unable to send the mail
//	}
//	// �޴»�� �ּ�, ÷������ �ʱ�ȭ
//	g_pX2SMTPMail->SMTPClearRecipient();
//
//	DeleteFile( wstrUserId.c_str() );
//
//	//}} oasis907 : ����� [2009.12.21] // �� CSmtp
//
//	THEMIDA_VM_END
//	ELSWORD_VIRTUALIZER_END
////#endif
}

#ifdef SERV_PROTECT_AUTO_HACK
void CX2Main::SendHackMail_HackUserNot(const char *strComm)
{
//	ELSWORD_VIRTUALIZER_START
//	THEMIDA_VM_START
//
//	char szMail[2][MAX_PATH];
//	strncpy( szMail[0], "demiahun@gmail.com" , MAX_PATH - 1 );		// ����
//	strncpy( szMail[1], "elswordmkt@gmail.com" , MAX_PATH - 1 );	// �̵���
//	//strcpy( szMail[0], "demiahun@gmail.com" );		// ����
//	//strcpy( szMail[1], "elswordmkt@gmail.com" );	// �̵���
//
//	char szID[MAX_PATH] = "";
//	if( g_pInstanceData != NULL &&
//		g_pInstanceData->GetUserID().size() > 0 )
//	{
//		WCHAR wszID[MAX_PATH] = L"";
//		wcscpy( wszID, g_pInstanceData->GetUserID().c_str() );
//		::MultiByteToWideChar( CP_ACP, 0, szID, -1, wszID, MAX_PATH );
//	}
//
//	if( g_pX2SMTPMail == NULL )
//	{
//		g_pX2SMTPMail = new CX2SMTPMail;
//	}
//
//	//{{ oasis907 : ����� [2009.12.21] // �� CSmtp
//	if(g_pX2SMTPMail->SMTPGetLastError() != CX2SMTPMail::CSMTP_NO_ERROR)
//	{
//		// Unable to initialize winsock2
//		return;
//	}
//
//#ifdef CHANGE_MAILSTRING
//	g_pX2SMTPMail->SMTPSetServer( m_strMailAddress.c_str(), 25 );
//#else
//	g_pX2SMTPMail->SMTPSetServer( "mail.kog.co.kr", 25 );
//#endif
//	g_pX2SMTPMail->SMTPSetLogin("escrash");            // ���� ����
//	g_pX2SMTPMail->SMTPSetPassword("@Els.123");
//
//	if( strlen( szID ) > 0 )
//		g_pX2SMTPMail->SMTPSetSenderName( szID );
//	else
//		g_pX2SMTPMail->SMTPSetSenderName( "���ҵ� ��ŷ ���� ������" );
//
//
//	g_pX2SMTPMail->SMTPSetSenderMail("escrash@kog.co.kr");
//	g_pX2SMTPMail->SMTPSetReplyTo("escrash@kog.co.kr"); // ȸ�� �ּ�
//
//	string mailName;
//	ConvertWCHARToChar( mailName, m_MailNameToFindHack.c_str() );
//	g_pX2SMTPMail->SMTPSetSubject( (char*)mailName.c_str() );
//	stringstream wstrstm;
//
//	// ���⿡ �޴� ����� ����
//	//g_pX2SMTPMail->SMTPAddRecipient("demiahun@kogstudios.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("eshack@kogstudios.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("carl79@kog.co.kr");
//	//g_pX2SMTPMail->SMTPAddRecipient("wonpok@kog.co.kr");
//	g_pX2SMTPMail->SMTPAddRecipient("k4interq@kog.co.kr");
//	g_pX2SMTPMail->SMTPAddRecipient("itexpertkim@kog.co.kr");
//	g_pX2SMTPMail->SMTPAddRecipient("JHKang@kog.co.kr");
//		
//
//	g_pX2SMTPMail->SMTPSetXPriority(CX2SMTPMail::XPRIORITY_NORMAL);
//	g_pX2SMTPMail->SMTPSetXMailer("The Bat! (v3.02) Professional");
//
//	set<wstring>::iterator sit;
//
//	wstrstm << strComm <<"\n\n";
//
//#ifdef MORE_INFO_ABOUT_HACK
//	// �ý��� ����
//	AddSystemInfo(wstrstm);
//
//	wstrstm << "- Loaded Dll -";
//	const char *dlllist = SiCX2DLLManager()->GetPreLoadedDllList();
//	wstrstm << dlllist;
//
//	// DLL ����
//	wstring wstrDllInfo = SiCX2DLLManager()->GetReportEvent();
//	string strDllInfo = "";
//	ConvertWCHARToChar( strDllInfo, wstrDllInfo );
//	wstrstm << strDllInfo << "\n\n";
//
//	AddDLLInfo(wstrstm);
//
//	// ���μ��� ����
//	AddProcessInfo(wstrstm);
//#endif // MORE_INFO_ABOUT_HACK
//
//	g_pX2SMTPMail->SMTPSetMessageBody((char*)wstrstm.str().c_str());
//
//	string strUserId = ANTI_HACK_STRING_UserID_Hack_ScreenShot;
//	wstring wstrUserId = ANTI_HACK_WSTRING_UserID_Hack_ScreenShot;
//
//	if( m_strUserIdToFindHack.empty() == false )
//	{
//		ConvertWCHARToChar( strUserId, m_strUserIdToFindHack.c_str() );
//		wstrUserId = m_strUserIdToFindHack;
//	}
//	strUserId = strUserId + ".jpg";
//	wstrUserId = wstrUserId + L".jpg";
//
//	MakeHackScreenShot( strUserId.c_str() );
//	// ���⿡ ÷�� ���� ����
//	//g_pX2SMTPMail->SMTPAddAttachment("c:\\mailtest2.exe");
//	//g_pX2SMTPMail->SMTPAddAttachment("c:\\test2.jpg");	
//	wchar_t szFilePath[1024] = _T("");
//	wchar_t strHackImageBuffer[1024];
//	GetCurrentDirectory( _MAX_PATH, szFilePath );
//	StringCchPrintf(strHackImageBuffer, 1024, _T("%s\\%s"), szFilePath, wstrUserId.c_str());
//	//wsprintf(strHackImageBuffer, _T("%s\\%s"), szFilePath, wstrUserId.c_str());
//	string strNickName = "";
//	ConvertWCHARToChar( strNickName, strHackImageBuffer );
//
//	g_pX2SMTPMail->SMTPAddAttachment(strNickName.c_str());
//
//	if( g_pX2SMTPMail->SMTPSend() )
//	{
//		//The mail was send successfully
//	}
//	else
//	{
//		DisplayErrorMessage( L"mail error" );
//		//Unable to send the mail
//	}
//	// �޴»�� �ּ�, ÷������ �ʱ�ȭ
//	g_pX2SMTPMail->SMTPClearRecipient();
//
//	DeleteFile( wstrUserId.c_str() );
//
//	//}} oasis907 : ����� [2009.12.21] // �� CSmtp
//
//
//	THEMIDA_VM_END
//	ELSWORD_VIRTUALIZER_END

}
#endif // SERV_PROTECT_AUTO_HACK

#ifdef HACK_USER_ATTACH_FILE

void CX2Main::SendHackMail_AttachFile(const char *strComm, wstring userId)
{
//	ELSWORD_VIRTUALIZER_START
//	THEMIDA_VM_START
//
//
//	static bool bSend = false;
//
//	if( bSend == true )
//		return;
//
//
//	if( userId != L"" && userId.compare( m_strUserIdToFindHack ) != 0 )
//		return;
//
//	char szMail[2][MAX_PATH];
//	strncpy( szMail[0], "demiahun@gmail.com",  MAX_PATH - 1 );		// ����
//	strncpy( szMail[1], "elswordmkt@gmail.com" ,  MAX_PATH - 1 );	// �̵���
//	//strcpy( szMail[0], "demiahun@gmail.com" );		// ����
//	//strcpy( szMail[1], "elswordmkt@gmail.com" );	// �̵���
//
//	char szID[MAX_PATH] = "";
//	if( g_pInstanceData != NULL &&
//		g_pInstanceData->GetUserID().size() > 0 )
//	{
//		WCHAR wszID[MAX_PATH] = L"";
//		wcscpy( wszID, g_pInstanceData->GetUserID().c_str() );
//		::MultiByteToWideChar( CP_ACP, 0, szID, -1, wszID, MAX_PATH );
//	}
//
//	if( g_pX2SMTPMail == NULL )
//	{
//		g_pX2SMTPMail = new CX2SMTPMail;
//	}
//
//	//{{ oasis907 : ����� [2009.12.21] // �� CSmtp
//	if(g_pX2SMTPMail->SMTPGetLastError() != CX2SMTPMail::CSMTP_NO_ERROR)
//	{
//		// Unable to initialize winsock2
//		return;
//	}
//
//#ifdef CHANGE_MAILSTRING
//	g_pX2SMTPMail->SMTPSetServer( m_strMailAddress.c_str(), 25 );
//#else
//	g_pX2SMTPMail->SMTPSetServer( "mail.kog.co.kr", 25 );
//#endif
//	g_pX2SMTPMail->SMTPSetLogin("escrash");            // ���� ����
//	g_pX2SMTPMail->SMTPSetPassword("@Els.123");
//
//	if( strlen( szID ) > 0 )
//		g_pX2SMTPMail->SMTPSetSenderName( szID );
//	else
//		g_pX2SMTPMail->SMTPSetSenderName( "���ҵ� ��ŷ ���� ������" );
//
//
//	g_pX2SMTPMail->SMTPSetSenderMail("escrash@kog.co.kr");
//	g_pX2SMTPMail->SMTPSetReplyTo("escrash@kog.co.kr"); // ȸ�� �ּ�
//
//	string mailName;
//	ConvertWCHARToChar( mailName, m_MailNameToFindHack.c_str() );
//	g_pX2SMTPMail->SMTPSetSubject( (char*)mailName.c_str() );
//	stringstream wstrstm;
//
//	// ���⿡ �޴� ����� ����
//	//g_pX2SMTPMail->SMTPAddRecipient("demiahun@kogstudios.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("eshack@kogstudios.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("carl79@kog.co.kr");
//	//g_pX2SMTPMail->SMTPAddRecipient("wonpok@kog.co.kr");
//	g_pX2SMTPMail->SMTPAddRecipient("itexpertkim@kog.co.kr");
//	g_pX2SMTPMail->SMTPAddRecipient("JHKang@kog.co.kr");
//
//
//	g_pX2SMTPMail->SMTPSetXPriority(CX2SMTPMail::XPRIORITY_NORMAL);
//	g_pX2SMTPMail->SMTPSetXMailer("The Bat! (v3.02) Professional");
//
//	set<wstring>::iterator sit;
//
//	wstrstm << "Attach File" <<"\n\n";
//
//#ifdef MORE_INFO_ABOUT_HACK
//	// �ý��� ����
//	AddSystemInfo(wstrstm);
//
//	// DLL ����
//	wstring wstrDllInfo = SiCX2DLLManager()->GetReportEvent();
//	string strDllInfo = "";
//	ConvertWCHARToChar( strDllInfo, wstrDllInfo );
//	wstrstm << strDllInfo << "\n\n";
//
//	AddDLLInfo(wstrstm);
//
//	// ���μ��� ����
//	AddProcessInfo(wstrstm);
//#endif // MORE_INFO_ABOUT_HACK
//
//	g_pX2SMTPMail->SMTPSetMessageBody((char*)wstrstm.str().c_str());
//
//#if 0
//	string strUserId = ANTI_HACK_STRING_UserID_Hack_ScreenShot;
//	wstring wstrUserId = ANTI_HACK_WSTRING_UserID_Hack_ScreenShot;
//
//	if( m_strUserIdToFindHack.empty() == false )
//	{
//		ConvertWCHARToChar( strUserId, m_strUserIdToFindHack.c_str() );
//		wstrUserId = m_strUserIdToFindHack;
//	}
//	strUserId = strUserId + ".jpg";
//	wstrUserId = wstrUserId + L".jpg";
//
//	MakeHackScreenShot( strUserId.c_str() );
//	// ���⿡ ÷�� ���� ����
//	//g_pX2SMTPMail->SMTPAddAttachment("c:\\mailtest2.exe");
//	//g_pX2SMTPMail->SMTPAddAttachment("c:\\test2.jpg");	
//	wchar_t szFilePath[1024] = _T("");
//	wchar_t strHackImageBuffer[1024];
//	GetCurrentDirectory( _MAX_PATH, szFilePath );
//	StringCchPrintf(strHackImageBuffer, 1024, _T("%s\\%s"), szFilePath, wstrUserId.c_str());
//	/*wsprintf(strHackImageBuffer, _T("%s\\%s"), szFilePath, wstrUserId.c_str());*/
//	string strNickName = "";
//	ConvertWCHARToChar( strNickName, strHackImageBuffer );
//#endif
//
//	g_pX2SMTPMail->SMTPAddAttachment( strComm );
//
//	if( g_pX2SMTPMail->SMTPSend() )
//	{
//		//The mail was send successfully
//	}
//	else
//	{
//		//Unable to send the mail
//	}
//	// �޴»�� �ּ�, ÷������ �ʱ�ȭ
//	g_pX2SMTPMail->SMTPClearRecipient();
//
//	//DeleteFile( wstrUserId.c_str() );
//
//	//}} oasis907 : ����� [2009.12.21] // �� CSmtp
//
//	bSend = true;
//
//	THEMIDA_VM_END
//	ELSWORD_VIRTUALIZER_END

}
#endif


#ifdef SERACH_FOLDER_FILE

void CX2Main::SendHackMail_FileList(const char *strComm, wstring userId)
{
//	ELSWORD_VIRTUALIZER_START
//	THEMIDA_VM_START
//
//
//	static bool bSend = false;
//
//	if( bSend == true )
//		return;
//
//
//	if( userId != L"" && userId.compare( m_strUserIdToFindHack ) != 0 )
//		return;
//
//	char szMail[2][MAX_PATH];
//	strncpy( szMail[0], "demiahun@gmail.com" , MAX_PATH - 1 );		// ����
//	strncpy( szMail[1], "elswordmkt@gmail.com" , MAX_PATH - 1 );	// �̵���
//	//strcpy( szMail[0], "demiahun@gmail.com" );		// ����
//	//strcpy( szMail[1], "elswordmkt@gmail.com" );	// �̵���
//
//	char szID[MAX_PATH] = "";
//	if( g_pInstanceData != NULL &&
//		g_pInstanceData->GetUserID().size() > 0 )
//	{
//		WCHAR wszID[MAX_PATH] = L"";
//		wcscpy( wszID, g_pInstanceData->GetUserID().c_str() );
//		::MultiByteToWideChar( CP_ACP, 0, szID, -1, wszID, MAX_PATH );
//	}
//
//	if( g_pX2SMTPMail == NULL )
//	{
//		g_pX2SMTPMail = new CX2SMTPMail;
//	}
//
//	//{{ oasis907 : ����� [2009.12.21] // �� CSmtp
//	if(g_pX2SMTPMail->SMTPGetLastError() != CX2SMTPMail::CSMTP_NO_ERROR)
//	{
//		// Unable to initialize winsock2
//		return;
//	}
//
//#ifdef CHANGE_MAILSTRING
//	g_pX2SMTPMail->SMTPSetServer( m_strMailAddress.c_str(), 25 );
//#else
//	g_pX2SMTPMail->SMTPSetServer( "mail.kog.co.kr", 25 );
//#endif
//	g_pX2SMTPMail->SMTPSetLogin("escrash");            // ���� ����
//	g_pX2SMTPMail->SMTPSetPassword("@Els.123");
//
//	if( strlen( szID ) > 0 )
//		g_pX2SMTPMail->SMTPSetSenderName( szID );
//	else
//		g_pX2SMTPMail->SMTPSetSenderName( "���ҵ� ��ŷ ���� ������" );
//
//
//	g_pX2SMTPMail->SMTPSetSenderMail("escrash@kog.co.kr");
//	g_pX2SMTPMail->SMTPSetReplyTo("escrash@kog.co.kr"); // ȸ�� �ּ�
//
//	string mailName;
//	ConvertWCHARToChar( mailName, m_MailNameToFindHack.c_str() );
//	g_pX2SMTPMail->SMTPSetSubject( (char*)mailName.c_str() );
//	stringstream wstrstm;
//
//	// ���⿡ �޴� ����� ����
//	//g_pX2SMTPMail->SMTPAddRecipient("demiahun@kogstudios.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("eshack@kogstudios.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("carl79@kog.co.kr");
//	//g_pX2SMTPMail->SMTPAddRecipient("wonpok@kog.co.kr");
//	g_pX2SMTPMail->SMTPAddRecipient("itexpertkim@kog.co.kr");
//	g_pX2SMTPMail->SMTPAddRecipient("JHKang@kog.co.kr");
//
//
//	g_pX2SMTPMail->SMTPSetXPriority(CX2SMTPMail::XPRIORITY_NORMAL);
//	g_pX2SMTPMail->SMTPSetXMailer("The Bat! (v3.02) Professional");
//
//	set<wstring>::iterator sit;
//
//	wstrstm << strComm <<"\n\n";
//
//	g_pX2SMTPMail->SMTPSetMessageBody((char*)wstrstm.str().c_str());
//
//	if( g_pX2SMTPMail->SMTPSend() )
//	{
//		//The mail was send successfully
//	}
//	else
//	{
//		//Unable to send the mail
//	}
//	// �޴»�� �ּ�, ÷������ �ʱ�ȭ
//	g_pX2SMTPMail->SMTPClearRecipient();
//
//	//DeleteFile( wstrUserId.c_str() );
//
//	//}} oasis907 : ����� [2009.12.21] // �� CSmtp
//
//	bSend = true;
//
//
//	THEMIDA_VM_END
//	ELSWORD_VIRTUALIZER_END
//
//
}
#endif

void CX2Main::SendHackMail_VERIFYVALUE(const char *strComm)
{
//#ifdef _SERVICE_
//	ELSWORD_VIRTUALIZER_START
//#endif
//	THEMIDA_VM_START
//
//	char szMail[2][MAX_PATH];
//	strncpy( szMail[0], "demiahun@gmail.com" , MAX_PATH - 1 );		// ����
//	strncpy( szMail[1], "elswordmkt@gmail.com" , MAX_PATH - 1 );	// �̵���
//	//strcpy( szMail[0], "demiahun@gmail.com" );		// ����
//	//strcpy( szMail[1], "elswordmkt@gmail.com" );	// �̵���
//
//	char szID[MAX_PATH] = "";
//	if( g_pInstanceData != NULL &&
//		g_pInstanceData->GetUserID().size() > 0 )
//	{
//		WCHAR wszID[MAX_PATH] = L"";
//		wcscpy( wszID, g_pInstanceData->GetUserID().c_str() );
//		::MultiByteToWideChar( CP_ACP, 0, szID, -1, wszID, MAX_PATH );
//	}
//	if( g_pX2SMTPMail == NULL )
//	{
//		g_pX2SMTPMail = new CX2SMTPMail;
//	}
//
//	//{{ oasis907 : ����� [2009.12.21] // �� CSmtp
//	if(g_pX2SMTPMail->SMTPGetLastError() != CX2SMTPMail::CSMTP_NO_ERROR)
//	{
//		// Unable to initialize winsock2
//		return;
//	}
//
//#ifdef CHANGE_MAILSTRING
//	g_pX2SMTPMail->SMTPSetServer( m_strMailAddress.c_str(), 25 );
//#else
//	g_pX2SMTPMail->SMTPSetServer( "mail.kog.co.kr", 25 );
//#endif
//	g_pX2SMTPMail->SMTPSetLogin("escrash");            // ���� ����
//	g_pX2SMTPMail->SMTPSetPassword("@Els.123");
//
//	if( strlen( szID ) > 0 )
//		g_pX2SMTPMail->SMTPSetSenderName( szID );
//	else
//		g_pX2SMTPMail->SMTPSetSenderName( "���ҵ� ��ŷ ���� ������" );
//
//
//	g_pX2SMTPMail->SMTPSetSenderMail("escrash@kog.co.kr");
//	g_pX2SMTPMail->SMTPSetReplyTo("escrash@kog.co.kr"); // ȸ�� �ּ�
//
//	string mailName;
//	ConvertWCHARToChar( mailName, m_MailNameToFindHack.c_str() );
//	g_pX2SMTPMail->SMTPSetSubject( (char*)mailName.c_str() );
//	stringstream wstrstm;
//
//	// ���⿡ �޴� ����� ����
//	//g_pX2SMTPMail->SMTPAddRecipient("demiahun@kogstudios.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("eshack@kogstudios.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("itexpertkim@kog.co.kr");	
//	//g_pX2SMTPMail->SMTPAddRecipient("wonpok@kog.co.kr");
//	//g_pX2SMTPMail->SMTPAddRecipient("oasis907@naver.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("megagame@kog.co.kr");
//	g_pX2SMTPMail->SMTPAddRecipient("itexpertkim@kog.co.kr");
//	g_pX2SMTPMail->SMTPAddRecipient("JHKang@kog.co.kr");
//
//	g_pX2SMTPMail->SMTPSetXPriority(CX2SMTPMail::XPRIORITY_NORMAL);
//	g_pX2SMTPMail->SMTPSetXMailer("The Bat! (v3.02) Professional");
//
//	set<wstring>::iterator sit;
//
//	wstrstm << strComm <<"\n\n";
//
//#ifdef MORE_INFO_ABOUT_HACK
//	// �ý��� ����
//	AddSystemInfo(wstrstm);
//
//	wstrstm << "- Loaded Dll -";
//	const char *dlllist = SiCX2DLLManager()->GetPreLoadedDllList();
//	wstrstm << dlllist;
//
//	// DLL ����
//	AddDLLInfo(wstrstm);
//
//	// ���μ��� ����
//	AddProcessInfo(wstrstm);
//#endif // MORE_INFO_ABOUT_HACK
//
//	g_pX2SMTPMail->SMTPSetMessageBody((char*)wstrstm.str().c_str());
//
//	string strUserId = ANTI_HACK_STRING_UserID_Hack_ScreenShot;
//	wstring wstrUserId = ANTI_HACK_WSTRING_UserID_Hack_ScreenShot;
//
//	if( m_strUserIdToFindHack.empty() == false )
//	{
//		ConvertWCHARToChar( strUserId, m_strUserIdToFindHack.c_str() );
//		wstrUserId = m_strUserIdToFindHack;
//	}
//	strUserId = strUserId + ".jpg";
//	wstrUserId = wstrUserId + L".jpg";
//
//	MakeHackScreenShot( strUserId.c_str() );
//	// ���⿡ ÷�� ���� ����
//	//g_pX2SMTPMail->SMTPAddAttachment("c:\\mailtest2.exe");
//	//g_pX2SMTPMail->SMTPAddAttachment("c:\\test2.jpg");	
//	wchar_t szFilePath[1024] = _T("");
//	wchar_t strHackImageBuffer[1024];
//	GetCurrentDirectory( _MAX_PATH, szFilePath );
//	StringCchPrintf(strHackImageBuffer, 1024, _T("%s\\%s"), szFilePath, wstrUserId.c_str());
//	//wsprintf(strHackImageBuffer, _T("%s\\%s"), szFilePath, wstrUserId.c_str());
//	string strNickName = "";
//	ConvertWCHARToChar( strNickName, strHackImageBuffer );
//
//	g_pX2SMTPMail->SMTPAddAttachment(strNickName.c_str());
//
//	if( g_pX2SMTPMail->SMTPSend() )
//	{
//		//The mail was send successfully
//	}
//	else
//	{
//		//Unable to send the mail
//	}
//	// �޴»�� �ּ�, ÷������ �ʱ�ȭ
//	g_pX2SMTPMail->SMTPClearRecipient();
//
//	DeleteFile( wstrUserId.c_str() );
//
//	//}} oasis907 : ����� [2009.12.21] // �� CSmtp
//
//	THEMIDA_VM_END
//
//#ifdef _SERVICE_
//	ELSWORD_VIRTUALIZER_END
//#endif

}

void CX2Main::SendHackMailGameStart(int iVal, bool bVal)
{
//#ifdef _SERVICE_
//	ELSWORD_VIRTUALIZER_START
//#endif
//
//	THEMIDA_VM_START
//
//	static int iCheckHackCnt[2] = {0,};
//	
//	if( bVal == false )
//	{
//		++iCheckHackCnt[iVal];
//		return;
//	}
//
//	if( iCheckHackCnt[iVal] <= 0 )
//		return;
//
//	iCheckHackCnt[iVal] = 0;
//
//	char *strComm = NULL;
//	switch( iVal )
//	{
//	case 0:
//		strComm = ANTI_HACK_STRING_AntiHacking_CheckSubStageGoNext;
//		break;
//	case 1:
//		strComm = ANTI_HACK_STRING_AntiHacking_Dungeon_ClearScore;
//		break;
//	default:
//		return;
//	}
//	
//
//	char szMail[2][MAX_PATH];
//	strncpy( szMail[0], "demiahun@gmail.com" , MAX_PATH - 1 );		// ����
//	strncpy( szMail[1], "elswordmkt@gmail.com" , MAX_PATH - 1 );	// �̵���
//	//strcpy( szMail[0], "demiahun@gmail.com" );		// ����
//	//strcpy( szMail[1], "elswordmkt@gmail.com" );	// �̵���
//
//	char szID[MAX_PATH] = "";
//	if( g_pInstanceData != NULL &&
//		g_pInstanceData->GetUserID().size() > 0 )
//	{
//		WCHAR wszID[MAX_PATH] = L"";
//		wcscpy( wszID, g_pInstanceData->GetUserID().c_str() );
//		::MultiByteToWideChar( CP_ACP, 0, szID, -1, wszID, MAX_PATH );
//	}
//	if( g_pX2SMTPMail == NULL )
//	{
//		g_pX2SMTPMail = new CX2SMTPMail;
//	}
//
//	//{{ oasis907 : ����� [2009.12.21] // �� CSmtp
//	if(g_pX2SMTPMail->SMTPGetLastError() != CX2SMTPMail::CSMTP_NO_ERROR)
//	{
//		// Unable to initialize winsock2
//		return;
//	}
//
//#ifdef CHANGE_MAILSTRING
//	g_pX2SMTPMail->SMTPSetServer( m_strMailAddress.c_str(), 25 );
//#else
//	g_pX2SMTPMail->SMTPSetServer( "mail.kog.co.kr", 25 );
//#endif
//	g_pX2SMTPMail->SMTPSetLogin("escrash");            // ���� ����
//	g_pX2SMTPMail->SMTPSetPassword("@Els.123");
//
//	if( strlen( szID ) > 0 )
//		g_pX2SMTPMail->SMTPSetSenderName( szID );
//	else
//		g_pX2SMTPMail->SMTPSetSenderName( "���ҵ� ��ŷ ���� ������" );
//
//
//	g_pX2SMTPMail->SMTPSetSenderMail("escrash@kog.co.kr");
//	g_pX2SMTPMail->SMTPSetReplyTo("escrash@kog.co.kr"); // ȸ�� �ּ�
//
//	string mailName;
//	ConvertWCHARToChar( mailName, m_MailNameToFindHack.c_str() );
//	g_pX2SMTPMail->SMTPSetSubject( (char*)mailName.c_str() );
//	stringstream wstrstm;
//
//	// ���⿡ �޴� ����� ����
//	//g_pX2SMTPMail->SMTPAddRecipient("demiahun@kogstudios.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("eshack@kogstudios.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("itexpertkim@kog.co.kr");	
//	//g_pX2SMTPMail->SMTPAddRecipient("wonpok@kog.co.kr");
//	//g_pX2SMTPMail->SMTPAddRecipient("oasis907@naver.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("megagame@kog.co.kr");
//	g_pX2SMTPMail->SMTPAddRecipient("itexpertkim@kog.co.kr");
//	g_pX2SMTPMail->SMTPAddRecipient("JHKang@kog.co.kr");
//
//	g_pX2SMTPMail->SMTPSetXPriority(CX2SMTPMail::XPRIORITY_NORMAL);
//	g_pX2SMTPMail->SMTPSetXMailer("The Bat! (v3.02) Professional");
//
//	set<wstring>::iterator sit;
//
//	wstrstm << strComm <<"\n\n";
//
//#ifdef MORE_INFO_ABOUT_HACK
//	// �ý��� ����
//	AddSystemInfo(wstrstm);
//
//	wstrstm << "- Loaded Dll -";
//	const char *dlllist = SiCX2DLLManager()->GetPreLoadedDllList();
//	wstrstm << dlllist;
//
//	// DLL ����
//	AddDLLInfo(wstrstm);
//
//	// ���μ��� ����
//	AddProcessInfo(wstrstm);
//#endif // MORE_INFO_ABOUT_HACK
//
//	g_pX2SMTPMail->SMTPSetMessageBody((char*)wstrstm.str().c_str());
//
//	string strUserId = ANTI_HACK_STRING_UserID_Hack_ScreenShot;
//	wstring wstrUserId = ANTI_HACK_WSTRING_UserID_Hack_ScreenShot;
//
//	if( m_strUserIdToFindHack.empty() == false )
//	{
//		ConvertWCHARToChar( strUserId, m_strUserIdToFindHack.c_str() );
//		wstrUserId = m_strUserIdToFindHack;
//	}
//	strUserId = strUserId + ".jpg";
//	wstrUserId = wstrUserId + L".jpg";
//
//	MakeHackScreenShot( strUserId.c_str() );
//	// ���⿡ ÷�� ���� ����
//	//g_pX2SMTPMail->SMTPAddAttachment("c:\\mailtest2.exe");
//	//g_pX2SMTPMail->SMTPAddAttachment("c:\\test2.jpg");	
//	wchar_t szFilePath[1024] = _T("");
//	wchar_t strHackImageBuffer[1024];
//	GetCurrentDirectory( _MAX_PATH, szFilePath );
//	StringCchPrintf(strHackImageBuffer, 1024, _T("%s\\%s"), szFilePath, wstrUserId.c_str());
//	//wsprintf(strHackImageBuffer, _T("%s\\%s"), szFilePath, wstrUserId.c_str());
//	string strNickName = "";
//	ConvertWCHARToChar( strNickName, strHackImageBuffer );
//
//	g_pX2SMTPMail->SMTPAddAttachment(strNickName.c_str());
//
//	if( g_pX2SMTPMail->SMTPSend() )
//	{
//		//The mail was send successfully
//	}
//	else
//	{
//		//Unable to send the mail
//	}
//	// �޴»�� �ּ�, ÷������ �ʱ�ȭ
//	g_pX2SMTPMail->SMTPClearRecipient();
//
//	DeleteFile( wstrUserId.c_str() );
//
//	//}} oasis907 : ����� [2009.12.21] // �� CSmtp
//
//	THEMIDA_VM_END
//
//#ifdef _SERVICE_
//	ELSWORD_VIRTUALIZER_END	
//#endif
}

#ifdef	ADD_CRASH_INFO
void CX2Main::AddMemoryInfo( stringstream& strstm )
{
	PROCESS_MEMORY_COUNTERS_EX pmcex;
	GetProcessMemoryInfo( GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmcex, sizeof(pmcex)); 

	const UINT MEGA = 1024 * 1024;

	string strNewLine = "\n";
	/// ���� ���μ��� �޸� ����
	strstm << "- Memory Information - " << strNewLine;
	strstm << "PageFaultCount: " << pmcex.PageFaultCount << strNewLine;
	strstm << "PeakWorkingSetSize(�ְ�): " << pmcex.PeakWorkingSetSize / MEGA << strNewLine;
	strstm << "WorkingSetSize(����): " << pmcex.WorkingSetSize / MEGA << strNewLine;
	strstm << "QuotaPeakPagedPoolUsage: " << pmcex.QuotaPeakPagedPoolUsage << strNewLine;
	strstm << "QuotaPagedPoolUsage: " << pmcex.QuotaPagedPoolUsage << strNewLine;
	strstm << "QuotaPeakNonPagedPoolUsage: " << pmcex.QuotaPeakNonPagedPoolUsage << strNewLine;
	strstm << "QuotaNonPagedPoolUsage: " << pmcex.QuotaNonPagedPoolUsage << strNewLine;
	strstm << "PrivateUsage(����): " << pmcex.PrivateUsage / MEGA << strNewLine;
	
	MEMORYSTATUSEX MemStatusEx;
	MemStatusEx.dwLength = sizeof( MEMORYSTATUSEX );
	if ( GlobalMemoryStatusEx( &MemStatusEx ) )
	{
		/// �ý��� �޸� ����
		strstm << "dwMemoryLoad: " << MemStatusEx.dwMemoryLoad / MEGA << strNewLine;
		strstm << "ullTotalPhys: " << MemStatusEx.ullTotalPhys / MEGA << strNewLine;
		strstm << "ullAvailPhys: " << MemStatusEx.ullAvailPhys / MEGA << strNewLine;
		strstm << "ullTotalPageFile: " << MemStatusEx.ullTotalPageFile << strNewLine;
		strstm << "ullAvailPageFile: " << MemStatusEx.ullAvailPageFile << strNewLine;
		strstm << "ullTotalVirtual: " << MemStatusEx.ullTotalVirtual / MEGA << strNewLine;
		strstm << "ullAvailVirtual: " << MemStatusEx.ullAvailVirtual / MEGA << strNewLine;
	}

	/// �ý��� ������ �ִ� �ϵ� �뷮
	const UINT uiStrSize = MAX_PATH + 1;
	char szSystemDirectory[uiStrSize] = { 0, };
	const UINT uiReturnValue = GetSystemDirectoryA( szSystemDirectory, uiStrSize );
	if ( uiReturnValue <= uiStrSize && uiReturnValue != 0 )
	{
		char drive[_MAX_DRIVE] = { 0, };
		char dir[_MAX_DIR] = { 0, };
		char fname[_MAX_FNAME] = { 0, };
		char ext[_MAX_EXT] = { 0, };

		_splitpath( szSystemDirectory, drive, dir, fname, ext );
		if ( DRIVE_FIXED == GetDriveTypeA( drive ) )
		{
			ULARGE_INTEGER ulAvail, ulTotal;
			if ( GetDiskFreeSpaceExA( drive, &ulAvail, &ulTotal, NULL ) )
			{				
				strstm << "Hard Drive: " << drive << strNewLine;
				strstm << "Available Space: " << static_cast<int>( ulAvail.QuadPart / MEGA )<< "MB" << strNewLine;
				strstm << "Total Space: " << static_cast<int>( ulTotal.QuadPart / MEGA )<< "MB" << strNewLine;
			}
		}
	}

	/// �׷���ī�� ����
	IDirect3D9* pD3D9 = DXUTGetD3DObject();
	UINT dwAdapterCount = pD3D9->GetAdapterCount();

	for( UINT iAdapter = 0; iAdapter < dwAdapterCount; iAdapter++ )
	{
		D3DADAPTER_IDENTIFIER9 id;
		ZeroMemory( &id, sizeof( D3DADAPTER_IDENTIFIER9 ) );
		pD3D9->GetAdapterIdentifier( iAdapter, 0, &id );
		strstm << "D3D9 Adapter: " << iAdapter << strNewLine;
		strstm << "Driver: " << id.Driver << strNewLine;
		strstm	<< "Description: " << id.Description << strNewLine;
	}

	strstm << strNewLine << strNewLine;
}
#endif	ADD_CRASH_INFO

void CX2Main::AddSystemInfo(stringstream& strstm)
{
	string strNewLine = "\n";

#ifdef SYS_INFO	
	strstm << "- System Information -" << strNewLine;
	strstm << "OS: ";
	strstm << GetSystemInfo()->GetOSVersion();
	strstm << " " << GetSystemInfo()->GetOSName();
	if(0 != GetSystemInfo()->GetServicePack())
		strstm << " Service Pack " << GetSystemInfo()->GetServicePack();
	strstm << strNewLine;

#ifdef ADD_INFO_INSTALL_LOCALE
	strstm << "Install Locale: " << GetSystemInfo()->GetInstallLocale() << strNewLine;
#endif // ADD_INFO_INSTALL_LOCALE

	strstm << "System Locale: " << GetSystemInfo()->GetSystemLocale() << strNewLine;

#ifdef ADD_INFO_INPUT_LOCALE
	const vector<string>& vecInputLocale = GetSystemInfo()->GetInputLocale();
	for(vector<string>::const_iterator iter = vecInputLocale.begin(); iter != vecInputLocale.end(); ++iter)
	{
		if(iter == vecInputLocale.begin())
			strstm << "Input Locales: ";
		else
            strstm << "                    ";
		
		strstm << *iter << strNewLine;
	}
#endif // ADD_INFO_INPUT_LOCALE
	
#ifdef MACHINE_ID
	strstm << strNewLine << g_pMain->GetMachineId().c_str() << strNewLine;	
#endif

#ifdef SERV_SERIAL_NUMBER_AVAILABILITY_CHECK
	std::string strOldSn = base64_encode(g_pInstanceData->GetOldSN(), SERVER_SN);
	strstm << strOldSn.c_str()  << strNewLine;
	std::string strSn = base64_encode(g_pInstanceData->GetSN(), SERVER_SN);
	strstm << strSn.c_str()  << strNewLine;	
#endif

	char cWallPaperPath[ MAX_PATH ];
	if( GetWallPaperDirectory( cWallPaperPath, MAX_PATH ) == true )
		strstm << cWallPaperPath << strNewLine;

	strstm << strNewLine << strNewLine;
#endif // SYS_INFO
}

void CX2Main::AddDLLInfo(stringstream& strstm)
{
	string strNewLine = "\n";
	
#ifdef MORE_INFO_ABOUT_HACK
	strstm << "- DLL Information -" << strNewLine;


	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, GetCurrentProcessId() );
	if (NULL == hProcess)
	{
		strstm << "Cannot access to process." << strNewLine;
		return;
	}

	HMODULE hMods[1024];
	DWORD cbNeeded;	
	if(EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
	{
		int numProcess = cbNeeded / sizeof(HMODULE);
		for (int i = 0; i < numProcess; i++ )
		{
			char szModName[MAX_PATH];
			ZeroMemory( szModName, MAX_PATH );
			// Get the full path to the module's file.
			if (!GetModuleFileNameExA( hProcess, hMods[i], szModName, ARRAY_SIZE(szModName) ))
			{
				strncpy(szModName, "[Hidden]" , MAX_PATH - 1 );
				//strcpy(szModName, "[Hidden]");
			}
		
			strstm << boost::str(boost::format("[0x%08X] %s") % hMods[i] % szModName);

#ifdef	ADD_CRASH_INFO
			HANDLE hFile = CreateFileA( szModName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );
			DWORD dwFileSize = GetFileSize( hFile, NULL );

			// system32 ���� ���ϵ� ����� �� ������ �ٸ��� ã�ƺ�����
			if ( INVALID_FILE_SIZE != dwFileSize )
				strstm << ", Size: " << dwFileSize << strNewLine;
			else
				strstm << strNewLine;
#endif	ADD_CRASH_INFO
	
			CloseHandle( hFile );
		}
	}

	CloseHandle( hProcess );

	strstm << strNewLine << strNewLine;
#endif // MORE_INFO_ABOUT_HACK
}

void CX2Main::AddProcessInfo(stringstream& strstm, bool bThreadInfo)
{
	string strNewLine = "\n";
	
#ifdef MORE_INFO_ABOUT_HACK
	strstm << "- Process Information -" << strNewLine;

#ifdef PROCESSLIST
	m_setProcessNameToFindHack.clear();
	UpdateProcessList();

	if( m_setSendedProcessList.empty() == false )
	{
		set<wstring>::iterator sit = m_setSendedProcessList.begin();
		for ( ; sit != m_setSendedProcessList.end(); sit++ )
		{				
			string processName;
			ConvertWCHARToChar( processName, (*sit).c_str() );
			strstm << processName.c_str() << strNewLine;
		}
	}

	if( m_setProcessNameToFindHack.empty() == false )
	{
		set<wstring>::iterator sit = m_setProcessNameToFindHack.begin();
		for ( ; sit != m_setProcessNameToFindHack.end(); sit++ )
		{
			m_setSendedProcessList.insert( *sit );
			string processName;
			ConvertWCHARToChar( processName, (*sit).c_str() );
			strstm << processName.c_str() << strNewLine;
		}
	}
#else
	HANDLE hSnapShot;
	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL,NULL);

	PROCESSENTRY32 pEntry;
	pEntry.dwSize =sizeof(pEntry);
	BOOL hRes = Process32FirstW( hSnapShot, &pEntry );

	while(hRes)
	{
		string ExeFile;
		ConvertWCHARToChar(ExeFile, pEntry.szExeFile);
		strstm << boost::str(boost::format("[0x%08X] %s \n") % pEntry.th32ProcessID % ExeFile );

		//������ ���μ����� ������ �����´�.
		hRes = Process32Next( hSnapShot, &pEntry );
	}

	CloseHandle(hSnapShot);

#endif // PROCESSLIST

	if( bThreadInfo == true )
	{
#ifdef CHECK_THREAD_WND
		strstm << strNewLine << "- TInfo -" << strNewLine;

		//////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef DESK_THREAD_WINDOW	
		DWORD ThreadId = GetWindowThreadProcessId(NULL,NULL); 
		vector<THREAD_WND_INFO> vecThreadInfo = GetThreadWnd(ThreadId);
		for(UINT i=0; i<vecThreadInfo.size(); ++i)
		{
			string threadWndName;
			ConvertWCHARToChar( threadWndName, vecThreadInfo[i].m_wstrName.c_str() );

			if( !vecThreadInfo[i].m_wstrClass.empty() )
			{
				string threadClassName;
				ConvertWCHARToChar( threadClassName, vecThreadInfo[i].m_wstrClass.c_str() );
				strstm << threadWndName.c_str() << "   [" << threadClassName.c_str() << "]" << strNewLine;
			}
			else
			{
				strstm << threadWndName.c_str() << "   [EMPTY]" << strNewLine;
			}
		}

		strstm << strNewLine << strNewLine;
#endif //DESK_THREAD_WINDOW
		//////////////////////////////////////////////////////////////////////////////////////////////////////

		if( g_pKTDXApp->GetHWND() != NULL )
		{
			DWORD ThreadId = GetWindowThreadProcessId(g_pKTDXApp->GetHWND(),NULL);  

			vector<THREAD_WND_INFO> vecThreadInfo = GetThreadWnd(ThreadId);

			for(UINT i=0; i<vecThreadInfo.size(); ++i)
			{
				string threadWndName;
				ConvertWCHARToChar( threadWndName, vecThreadInfo[i].m_wstrName.c_str() );

				if( !vecThreadInfo[i].m_wstrClass.empty() )
				{
					string threadClassName;
					ConvertWCHARToChar( threadClassName, vecThreadInfo[i].m_wstrClass.c_str() );
					strstm << threadWndName.c_str() << "   [" << threadClassName.c_str() << "]" << strNewLine;
				}
				else
				{
					strstm << threadWndName.c_str() << "   [EMPTY]" << strNewLine;
				}

			}
		}
#endif
	}

	strstm << strNewLine << strNewLine;
#endif // MORE_INFO_ABOUT_HACK
}


//{{ �ڱ��� : [2010/05/05]	//	DLL ������ �ٸ���
#ifdef DLL_MANAGER
void CX2Main::SendHackMail_DLLInjection(const char *strComm)
{
////#ifdef _SERVICE_
//
//
//	ELSWORD_VIRTUALIZER_START
//	THEMIDA_VM_START
//
//
//
//#ifdef _SERVICE_
//		if(g_pData != NULL && g_pData->GetServerProtocol() != NULL )
//		{
//			if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
//				g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
//				return;
//		}
//#endif
//
//	//{{ oasis907 : ����� [2009.12.21] // �� CSmtp
//	if(g_pX2SMTPMail->SMTPGetLastError() != CX2SMTPMail::CSMTP_NO_ERROR)
//	{
//		// Unable to initialize winsock2
//		return;
//	}
//
//	char szID[MAX_PATH] = "";
//	if( g_pInstanceData != NULL &&
//		g_pInstanceData->GetUserID().size() > 0 )
//	{
//		WCHAR wszID[MAX_PATH] = L"";
//		wcscpy( wszID, g_pInstanceData->GetUserID().c_str() );
//		::MultiByteToWideChar( CP_ACP, 0, szID, -1, wszID, MAX_PATH );
//	}
//
//#ifdef CHANGE_MAILSTRING
//	g_pX2SMTPMail->SMTPSetServer( m_strMailAddress.c_str(), 25 );
//#else
//	g_pX2SMTPMail->SMTPSetServer( "mail.kog.co.kr", 25 );
//#endif
//	g_pX2SMTPMail->SMTPSetLogin("escrash");            // ���� ����
//	g_pX2SMTPMail->SMTPSetPassword("@Els.123");
//
//	if( strlen( szID ) > 0 )
//		g_pX2SMTPMail->SMTPSetSenderName( szID );
//	else
//		g_pX2SMTPMail->SMTPSetSenderName( "���ҵ� ��ŷ ���� ������" );
//
//
//	g_pX2SMTPMail->SMTPSetSenderMail("escrash@kog.co.kr");
//	g_pX2SMTPMail->SMTPSetReplyTo("escrash@kog.co.kr"); // ȸ�� �ּ�
//
//	string mailName;
//	ConvertWCHARToChar( mailName, m_MailNameToFindHack.c_str() );
//	g_pX2SMTPMail->SMTPSetSubject( (char*)mailName.c_str() );
//
//	// ���⿡ �޴� ����� ����
//	g_pX2SMTPMail->SMTPAddRecipient("eshack@kogstudios.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("carl79@kog.co.kr");
//	//g_pX2SMTPMail->SMTPAddRecipient("wonpok@kog.co.kr");
//	//g_pX2SMTPMail->SMTPAddRecipient("megagame@kog.co.kr");
//
//	g_pX2SMTPMail->SMTPSetXPriority(CX2SMTPMail::XPRIORITY_NORMAL);
//	g_pX2SMTPMail->SMTPSetXMailer("The Bat! (v3.02) Professional");
//
//
//
//	stringstream strstm;
//	strstm << ANTI_HACK_STRING_UserID_AntiHacking_Event_DLL_Injection << std::endl << std::endl;
//
//	// ����
//	strstm << strComm <<"\n\n";
//
//#ifdef MORE_INFO_ABOUT_HACK
//	// �ý��� ����
//	AddSystemInfo(strstm);
//
//	// DLL ����
//	AddDLLInfo(strstm);
//
//	// ���μ��� ����
//	AddProcessInfo(strstm);
//#endif // MORE_INFO_ABOUT_HACK
//
//	g_pX2SMTPMail->SMTPSetMessageBody(strstm.str().c_str());
//
//
//
//
//
//
//	string strUserId = ANTI_HACK_STRING_UserID_Hack_ScreenShot;
//	wstring wstrUserId = ANTI_HACK_WSTRING_UserID_Hack_ScreenShot;
//
//	if( m_strUserIdToFindHack.empty() == false )
//	{
//		ConvertWCHARToChar( strUserId, m_strUserIdToFindHack.c_str() );
//		wstrUserId = m_strUserIdToFindHack;
//	}
//	strUserId = strUserId + ".jpg";
//	wstrUserId = wstrUserId + L".jpg";
//
//	MakeHackScreenShot( strUserId.c_str() );
//	// ���⿡ ÷�� ���� ����
//	wchar_t szFilePath[1024] = _T("");
//	wchar_t strHackImageBuffer[1024];
//	GetCurrentDirectory( _MAX_PATH, szFilePath );
//	StringCchPrintf(strHackImageBuffer, 1024, _T("%s\\%s"), szFilePath, wstrUserId.c_str());
//	//wsprintf(strHackImageBuffer, _T("%s\\%s"), szFilePath, wstrUserId.c_str());
//	string strNickName = "";
//	ConvertWCHARToChar( strNickName, strHackImageBuffer );
//
//	g_pX2SMTPMail->SMTPAddAttachment(strNickName.c_str());
//
//	if( g_pX2SMTPMail->SMTPSend() )
//	{
//		//The mail was send successfully
//	}
//	else
//	{
//		//Unable to send the mail
//	}
//	// �޴»�� �ּ�, ÷������ �ʱ�ȭ
//	g_pX2SMTPMail->SMTPClearRecipient();
//
//	DeleteFile( wstrUserId.c_str() );
//
//	//}} oasis907 : ����� [2009.12.21] // �� CSmtp
//
//	THEMIDA_VM_END
//	ELSWORD_VIRTUALIZER_END
//
////#endif
}
#endif //DLL_MANAGER

#ifdef USER_MONITORING_MAIL
void CX2Main::SendUserMonitoringMail(const char *strComm)
{	
//	//#ifdef _SERVICE_
//	ELSWORD_VIRTUALIZER_START
//	THEMIDA_VM_START
//
//	char szMail[2][MAX_PATH];
//	strncpy( szMail[0], "demiahun@gmail.com" , MAX_PATH - 1 );		// ����
//	strncpy( szMail[1], "elswordmkt@gmail.com" , MAX_PATH - 1 );	// �̵���
//	//strcpy( szMail[0], "demiahun@gmail.com" );		// ����
//	//strcpy( szMail[1], "elswordmkt@gmail.com" );	// �̵���
//
//	char szID[MAX_PATH] = "";
//	if( g_pInstanceData != NULL &&
//		g_pInstanceData->GetUserID().size() > 0 )
//	{
//		WCHAR wszID[MAX_PATH] = L"";
//		wcscpy( wszID, g_pInstanceData->GetUserID().c_str() );
//		::MultiByteToWideChar( CP_ACP, 0, szID, -1, wszID, MAX_PATH );
//	}
//
//	if( g_pX2SMTPMail == NULL )
//	{
//		g_pX2SMTPMail = new CX2SMTPMail;
//	}
//
//	//{{ oasis907 : ����� [2009.12.21] // �� CSmtp
//	if(g_pX2SMTPMail->SMTPGetLastError() != CX2SMTPMail::CSMTP_NO_ERROR)
//	{
//		// Unable to initialize winsock2
//		return;
//	}
//	
//#ifdef CHANGE_MAILSTRING
//	g_pX2SMTPMail->SMTPSetServer( m_strMailAddress.c_str(), 25 );
//#else
//	g_pX2SMTPMail->SMTPSetServer( "mail.kog.co.kr", 25 );
//#endif
//	g_pX2SMTPMail->SMTPSetLogin("escrash");            // ���� ����
//	g_pX2SMTPMail->SMTPSetPassword("@Els.123");
//
//	if( strlen( szID ) > 0 )
//		g_pX2SMTPMail->SMTPSetSenderName( szID );
//	else
//		g_pX2SMTPMail->SMTPSetSenderName( "���ҵ� ��ŷ ���� ������" );
//
//
//	g_pX2SMTPMail->SMTPSetSenderMail("escrash@kog.co.kr");
//	g_pX2SMTPMail->SMTPSetReplyTo("escrash@kog.co.kr"); // ȸ�� �ּ�
//
//	string mailName;
//	ConvertWCHARToChar( mailName, m_MailNameToFindHack.c_str() );
//	g_pX2SMTPMail->SMTPSetSubject( (char*)mailName.c_str() );
//	stringstream wstrstm;
//
//	// ���⿡ �޴� ����� ����
//	//g_pX2SMTPMail->SMTPAddRecipient("demiahun@kogstudios.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("eshack@kogstudios.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("carl79@kog.co.kr");	
//	//g_pX2SMTPMail->SMTPAddRecipient("wonpok@kog.co.kr");
//	g_pX2SMTPMail->SMTPAddRecipient("comicprince@kog.co.kr");	
//	//g_pX2SMTPMail->SMTPAddRecipient("oasis907@naver.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("megagame@kog.co.kr");
//	g_pX2SMTPMail->SMTPAddRecipient("itexpertkim@kog.co.kr");
//	g_pX2SMTPMail->SMTPAddRecipient("JHKang@kog.co.kr");
//
//	g_pX2SMTPMail->SMTPSetXPriority(CX2SMTPMail::XPRIORITY_NORMAL);
//	g_pX2SMTPMail->SMTPSetXMailer("The Bat! (v3.02) Professional");
//
//	set<wstring>::iterator sit;
//
//	if( strcmp(strComm, ANTI_HACK_IP_MONITORING) == 0 )
//	{
//		string strIp;
//		ConvertWCHARToChar( strIp, m_wstrPublicIp.c_str() );
//#ifdef _IN_HOUSE_	
//		wstrstm << "[IN HOUSE] " << strComm << " : " << strIp << "\n\n";
//#else
//		wstrstm << strComm << " : " << strIp << "\n\n";
//#endif
//	}
//	else
//	{
//#ifdef _IN_HOUSE_	
//		wstrstm << "[IN HOUSE] " << strComm << "\n\n";
//#else
//		wstrstm << strComm <<"\n\n";
//#endif
//	}
//	
//
//
//#ifdef MORE_INFO_ABOUT_HACK
//	// �ý��� ����
//	AddSystemInfo(wstrstm);
//
//	// DLL ����
//	AddDLLInfo(wstrstm);
//
//	// ���μ��� ����
//	AddProcessInfo(wstrstm);
//#endif // MORE_INFO_ABOUT_HACK
//
//	g_pX2SMTPMail->SMTPSetMessageBody((char*)wstrstm.str().c_str());
//
//	string strUserId = ANTI_HACK_STRING_UserID_Hack_ScreenShot;
//	wstring wstrUserId = ANTI_HACK_WSTRING_UserID_Hack_ScreenShot;
//
//	if( m_strUserIdToFindHack.empty() == false )
//	{
//		ConvertWCHARToChar( strUserId, m_strUserIdToFindHack.c_str() );
//		wstrUserId = m_strUserIdToFindHack;
//	}
//	strUserId = strUserId + ".jpg";
//	wstrUserId = wstrUserId + L".jpg";
//
//	MakeHackScreenShot( strUserId.c_str() );
//	// ���⿡ ÷�� ���� ����
//	//g_pX2SMTPMail->SMTPAddAttachment("c:\\mailtest2.exe");
//	//g_pX2SMTPMail->SMTPAddAttachment("c:\\test2.jpg");	
//	wchar_t szFilePath[1024] = _T("");
//	wchar_t strHackImageBuffer[1024];
//	GetCurrentDirectory( _MAX_PATH, szFilePath );
//	StringCchPrintf(strHackImageBuffer, 1024,  _T("%s\\%s"), szFilePath, wstrUserId.c_str());
//	//wsprintf(strHackImageBuffer, _T("%s\\%s"), szFilePath, wstrUserId.c_str());
//	string strNickName = "";
//	ConvertWCHARToChar( strNickName, strHackImageBuffer );
//
//	g_pX2SMTPMail->SMTPAddAttachment(strNickName.c_str());
//
//	if( g_pX2SMTPMail->SMTPSend() )
//	{
//		//The mail was send successfully
//	}
//	else
//	{
//		//Unable to send the mail
//	}
//	// �޴»�� �ּ�, ÷������ �ʱ�ȭ
//	g_pX2SMTPMail->SMTPClearRecipient();
//
//	DeleteFile( wstrUserId.c_str() );
//
//	//}} oasis907 : ����� [2009.12.21] // �� CSmtp
//
//	THEMIDA_VM_END
//	ELSWORD_VIRTUALIZER_END
//	//#endif
}
#endif

#ifdef SERV_ED_MONITORING_IN_GAME
void CX2Main::SendMail_MoneyMonitoring(const char *strComm)
{
////#ifdef _SERVICE_
//	ELSWORD_VIRTUALIZER_START
//	THEMIDA_VM_START
//
//	char szID[MAX_PATH] = "";
//	if( g_pInstanceData != NULL &&
//		g_pInstanceData->GetUserID().size() > 0 )
//	{
//		WCHAR wszID[MAX_PATH] = L"";
//		wcscpy( wszID, g_pInstanceData->GetUserID().c_str() );
//		::MultiByteToWideChar( CP_ACP, 0, szID, -1, wszID, MAX_PATH );
//	}
//
//	if( g_pX2SMTPMail == NULL )
//	{
//		g_pX2SMTPMail = new CX2SMTPMail;
//	}
//
//	//{{ oasis907 : ����� [2009.12.21] // �� CSmtp
//	if(g_pX2SMTPMail->SMTPGetLastError() != CX2SMTPMail::CSMTP_NO_ERROR)
//	{
//		// Unable to initialize winsock2
//		return;
//	}
//
//#ifdef CHANGE_MAILSTRING
//	g_pX2SMTPMail->SMTPSetServer( m_strMailAddress.c_str(), 25 );
//#else
//	g_pX2SMTPMail->SMTPSetServer( "mail.kog.co.kr", 25 );
//#endif
//	g_pX2SMTPMail->SMTPSetLogin("escrash");            // ���� ����
//	g_pX2SMTPMail->SMTPSetPassword("@Els.123");
//
//	if( strlen( szID ) > 0 )
//		g_pX2SMTPMail->SMTPSetSenderName( szID );
//	else
//		g_pX2SMTPMail->SMTPSetSenderName( "���ҵ� ��ŷ ���� ������" );
//
//
//	g_pX2SMTPMail->SMTPSetSenderMail("escrash@kog.co.kr");
//	g_pX2SMTPMail->SMTPSetReplyTo("escrash@kog.co.kr"); // ȸ�� �ּ�
//
//	string mailName;
//	ConvertWCHARToChar( mailName, m_MailNameToFindHack.c_str() );
//	g_pX2SMTPMail->SMTPSetSubject( (char*)mailName.c_str() );
//	stringstream wstrstm;
//
//	// ���⿡ �޴� ����� ����
//	//g_pX2SMTPMail->SMTPAddRecipient("demiahun@kogstudios.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("eshack@kogstudios.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("carl79@kog.co.kr");	
//	g_pX2SMTPMail->SMTPAddRecipient("comicprince@kog.co.kr");
//	g_pX2SMTPMail->SMTPAddRecipient("operator@kog.co.kr");
//	
//	//g_pX2SMTPMail->SMTPAddRecipient("oasis907@naver.com");
//	//g_pX2SMTPMail->SMTPAddRecipient("megagame@kog.co.kr");
//
//
//	g_pX2SMTPMail->SMTPSetXPriority(CX2SMTPMail::XPRIORITY_NORMAL);
//	g_pX2SMTPMail->SMTPSetXMailer("The Bat! (v3.02) Professional");
//
//	set<wstring>::iterator sit;
//
//#ifdef _IN_HOUSE_
//	wstrstm << "[IN HOUSE] " << strComm <<"\n\n";
//#else
//	wstrstm << strComm <<"\n\n";
//#endif
//	
//
//#ifdef MORE_INFO_ABOUT_HACK
//	// �ý��� ����
//	AddSystemInfo(wstrstm);
//
//	string strIp;
//	ConvertWCHARToChar( strIp, m_wstrPublicIp.c_str() );
//	wstrstm << strComm << " : " << strIp << "\n\n";
//
//
//	// ���μ��� ����
//	AddProcessInfo(wstrstm);
//	
//#endif // MORE_INFO_ABOUT_HACK
//
//	g_pX2SMTPMail->SMTPSetMessageBody((char*)wstrstm.str().c_str());
//
//	string strUserId = ANTI_HACK_STRING_UserID_Hack_ScreenShot;
//	wstring wstrUserId = ANTI_HACK_WSTRING_UserID_Hack_ScreenShot;
//
//	if( m_strUserIdToFindHack.empty() == false )
//	{
//		ConvertWCHARToChar( strUserId, m_strUserIdToFindHack.c_str() );
//		wstrUserId = m_strUserIdToFindHack;
//	}
//	strUserId = strUserId + ".jpg";
//	wstrUserId = wstrUserId + L".jpg";
//
//	MakeHackScreenShot( strUserId.c_str() );
//	// ���⿡ ÷�� ���� ����
//	//g_pX2SMTPMail->SMTPAddAttachment("c:\\mailtest2.exe");
//	//g_pX2SMTPMail->SMTPAddAttachment("c:\\test2.jpg");	
//	wchar_t szFilePath[1024] = _T("");
//	wchar_t strHackImageBuffer[1024];
//	GetCurrentDirectory( _MAX_PATH, szFilePath );
//	StringCchPrintf(strHackImageBuffer, 1024,  _T("%s\\%s"), szFilePath, wstrUserId.c_str());
//    //wsprintf(strHackImageBuffer, _T("%s\\%s"), szFilePath, wstrUserId.c_str());
//	string strNickName = "";
//	ConvertWCHARToChar( strNickName, strHackImageBuffer );
//
//	g_pX2SMTPMail->SMTPAddAttachment(strNickName.c_str());
//
//	if( g_pX2SMTPMail->SMTPSend() )
//	{
//		//The mail was send successfully
//	}
//	else
//	{
//		//Unable to send the mail
//	}
//	// �޴»�� �ּ�, ÷������ �ʱ�ȭ
//	g_pX2SMTPMail->SMTPClearRecipient();
//
//	DeleteFile( wstrUserId.c_str() );
//
//	//}} oasis907 : ����� [2009.12.21] // �� CSmtp
//
//	THEMIDA_VM_END
//	ELSWORD_VIRTUALIZER_END
////#endif
}
#endif //SERV_ED_MONITORING_IN_GAME

bool CX2Main::GetPresentArrival( CX2StateMenu::PresentArrival& presentArrival )
{
	if ( m_quePresentArrival.empty() == true )
		return false;

	presentArrival = m_quePresentArrival.front();
	m_quePresentArrival.pop();

	return true;
}

void CX2Main::PushPresentArrival( KEGS_PRESENT_MESSAGE_TO_RECEIVER_NOT& kEvent )
{
	CX2StateMenu::PresentArrival presentArrival;
#ifdef SERV_GLOBAL_BILLING
	presentArrival.m_kBillProductInfo = kEvent.m_kBillProductInfo;
#else
	presentArrival.m_KNXBTProductInfo = kEvent.m_kNXBTProductInfo;
#endif SERV_GLOBAL_BILLING

	presentArrival.m_PresentMsg = kEvent.m_wstrMessage;
	presentArrival.m_SenderNickName = kEvent.m_wstrSenderNickName;
	
	m_quePresentArrival.push( presentArrival );
}

void CX2Main::DeleteNMCachData()
{
	HANDLE				hSearch;
	WIN32_FIND_DATA		fd;

	wstring nmDataFolder = L"C:\\Documents and Settings\\All Users\\Application Data\\Nexon\\Common\\NMDATA";
	wstring tempAllFile = nmDataFolder + L"\\*.*";
	hSearch = FindFirstFile( tempAllFile.c_str(), &fd );

	vector< wstring > vecFilePathToDelete;

	if(hSearch == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if( wcscmp(fd.cFileName, L".") && wcscmp(fd.cFileName, L"..") )
		{
			if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				
			}
			else
			{
				WCHAR			wstrNewSearchFile[256];
				std::wstring	wstrFile;
				std::wstring	wstrFileName;

				wcscpy( wstrNewSearchFile, nmDataFolder.c_str() );
				wcscat( wstrNewSearchFile, L"\\");
				wcscat( wstrNewSearchFile, fd.cFileName );

				if ( ::DeleteFile( wstrNewSearchFile ) != FALSE )
				{
					int i = 0;
				}
			}
		}

	}while( FindNextFile( hSearch, &fd ) );

	FindClose(hSearch);

	if ( ::DeleteFile( L"C:\\Documents and Settings\\All Users\\Application Data\\Nexon\\Common\\nmservice.ini" ) != FALSE )
	{
		int i = 0;
	}
}

void CX2Main::LoadLuaEnum(  KLuabinder* pKLuabinder )
{
	// luamanager�� enum.lua �� �������� ���Ǵ� ��ũ��Ʈ �ε��� CKTDXApp::loadkluamanagertemplate() �Լ�����
	// luabinder�� enum.lua �� �������� ���Ǵ� ��ũ��Ʈ �ε��� cx2main::loadluaenum() �Լ�����

	OpenScriptFile( L"Enum.lua", pKLuabinder );
#ifdef MASSFILE_MAPPING_FUNCTION
	lua_tinker::decl( pKLuabinder->GetLuaState(),  "KGCMassFileManager", g_pKTDXApp->GetDeviceManager()->GetMassFileManager() );
#if defined(CLIENT_COUNTRY_TW)
	OpenScriptFile( L"MassFileMappingList_TW.lua", pKLuabinder );
#elif defined(CLIENT_COUNTRY_HK)
	OpenScriptFile( L"MassFileMappingList_HK.lua", pKLuabinder );
#else
	OpenScriptFile( L"MassFileMappingList.lua", pKLuabinder );
#endif
#endif MASSFILE_MAPPING_FUNCTION
    OpenScriptFile( L"StringID_def.lua", pKLuabinder );
}

//{{ 2008. 1. 2  ������  �ټ��� ġƮ
void CX2Main::ChangeSpirit( int iChangeSpirit )
{
	if( g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_OPERATOR &&
		g_pData->GetMyUser()->GetSelectUnit() != NULL )
	{
#ifdef LIGHT_OPERATOR_ACCOUNT
		if( g_pData->GetMyUser()->GetAuthLevel() == CX2User::XUAL_LIGHT_OPERATOR )
		{
			return;
		}
#endif LIGHT_OPERATOR_ACCOUNT

		KEGS_ADMIN_CHANGE_SPIRIT_REQ kEGS_ADMIN_CHANGE_SPIRIT_REQ;
		kEGS_ADMIN_CHANGE_SPIRIT_REQ.m_iSpirit = iChangeSpirit;

		g_pData->GetServerProtocol()->SendPacket( EGS_ADMIN_CHANGE_SPIRIT_REQ, kEGS_ADMIN_CHANGE_SPIRIT_REQ );
		AddServerPacket( EGS_ADMIN_CHANGE_SPIRIT_ACK );
	}
}
//}}

//{{ 2008. 1. 25  ������  VP ġƮ
#ifdef SERV_PVP_NEW_SYSTEM
void CX2Main::ChangeVSPoint( int iRating, int iRPoint, int iAPoint )
{
	if( g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_OPERATOR &&
		g_pData->GetMyUser()->GetSelectUnit() != NULL )
	{
#ifdef LIGHT_OPERATOR_ACCOUNT
		if( g_pData->GetMyUser()->GetAuthLevel() == CX2User::XUAL_LIGHT_OPERATOR )
		{
			return;
		}
#endif LIGHT_OPERATOR_ACCOUNT
	
		KEGS_ADMIN_CHANGE_VSPOINT_REQ kEGS_ADMIN_CHANGE_VSPOINT_REQ;
		kEGS_ADMIN_CHANGE_VSPOINT_REQ.m_iRating	= iRating;
		kEGS_ADMIN_CHANGE_VSPOINT_REQ.m_iRPoint = iRPoint;
		kEGS_ADMIN_CHANGE_VSPOINT_REQ.m_iAPoint = iAPoint;

		g_pData->GetServerProtocol()->SendPacket( EGS_ADMIN_CHANGE_VSPOINT_REQ, kEGS_ADMIN_CHANGE_VSPOINT_REQ );
		AddServerPacket( EGS_ADMIN_CHANGE_VSPOINT_ACK );
	}
}
#else
void CX2Main::ChangeVSPoint( int iChangeVSPoint, int iChangeVSPointMax )
{
	if( g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_OPERATOR &&
		g_pData->GetMyUser()->GetSelectUnit() != NULL )
	{
		KEGS_ADMIN_CHANGE_VSPOINT_REQ kEGS_ADMIN_CHANGE_VSPOINT_REQ;
		kEGS_ADMIN_CHANGE_VSPOINT_REQ.m_iVSPoint	= iChangeVSPoint;
		kEGS_ADMIN_CHANGE_VSPOINT_REQ.m_iVSPointMax = iChangeVSPointMax;

		g_pData->GetServerProtocol()->SendPacket( EGS_ADMIN_CHANGE_VSPOINT_REQ, kEGS_ADMIN_CHANGE_VSPOINT_REQ );
		AddServerPacket( EGS_ADMIN_CHANGE_VSPOINT_ACK );
	}
}
#endif
//}}

//{{ 2008. 2. 18  ������  ���� Ŭ���� ġƮ
void CX2Main::ChangeUnitClass( char cUnitClass )
{
	if( g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_OPERATOR &&
		g_pData->GetMyUser()->GetSelectUnit() != NULL )
	{
#ifdef LIGHT_OPERATOR_ACCOUNT
		if( g_pData->GetMyUser()->GetAuthLevel() == CX2User::XUAL_LIGHT_OPERATOR )
		{
			return;
		}
#endif LIGHT_OPERATOR_ACCOUNT

		KEGS_ADMIN_CHANGE_UNIT_CLASS_REQ kEGS_ADMIN_CHANGE_UNIT_CLASS_REQ;
		kEGS_ADMIN_CHANGE_UNIT_CLASS_REQ.m_cUnitClass = cUnitClass;

		g_pData->GetServerProtocol()->SendPacket( EGS_ADMIN_CHANGE_UNIT_CLASS_REQ, kEGS_ADMIN_CHANGE_UNIT_CLASS_REQ );
		AddServerPacket( EGS_ADMIN_CHANGE_UNIT_CLASS_ACK );
	}
}
void CX2Main::ModifyChangeUnitClass( const char* UnitClassName )
{
	wstring wsUnitClassName;
	ConvertCharToWCHAR( wsUnitClassName, UnitClassName );

	if ( wsUnitClassName.compare( L"���ҵ�" ) == 0 )		{ChangeUnitClass(CX2Unit::UC_ELSWORD_SWORDMAN);	}
	else if ( wsUnitClassName.compare( L"�ҵ峪��Ʈ" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_ELSWORD_KNIGHT);}
	else if ( wsUnitClassName.compare( L"��������Ʈ" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_ELSWORD_MAGIC_KNIGHT);}
	else if ( wsUnitClassName.compare( L"�ý�����Ʈ" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_ELSWORD_SHEATH_KNIGHT);}
	else if ( wsUnitClassName.compare( L"�ε峪��Ʈ" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_ELSWORD_LORD_KNIGHT);}
	else if ( wsUnitClassName.compare( L"�齽���̾�" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_ELSWORD_RUNE_SLAYER);}
	else if ( wsUnitClassName.compare( L"���Ǵ�Ƽ�ҵ�" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_ELSWORD_INFINITY_SWORD);}

	else if ( wsUnitClassName.compare( L"���̻�" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_ARME_VIOLET_MAGE);}
	else if ( wsUnitClassName.compare( L"���̸�����" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_ARME_HIGH_MAGICIAN);}
	else if ( wsUnitClassName.compare( L"��ũ������" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_ARME_DARK_MAGICIAN);}
	else if ( wsUnitClassName.compare( L"��Ʋ������" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_ARME_BATTLE_MAGICIAN);}
	else if ( wsUnitClassName.compare( L"������Ż������" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_ARME_ELEMENTAL_MASTER);}
	else if ( wsUnitClassName.compare( L"���̵���������" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_ARME_VOID_PRINCESS);}
	else if ( wsUnitClassName.compare( L"������ġ" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_ARME_DIMENSION_WITCH);}

	else if ( wsUnitClassName.compare( L"����" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_LIRE_ELVEN_RANGER);}
	else if ( wsUnitClassName.compare( L"�Ĺ����" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_LIRE_COMBAT_RANGER);}
	else if ( wsUnitClassName.compare( L"�������η�����" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_LIRE_SNIPING_RANGER);}
	else if ( wsUnitClassName.compare( L"Ʈ���η�����" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_LIRE_TRAPPING_RANGER);}
	else if ( wsUnitClassName.compare( L"���彺��Ŀ" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_LIRE_WIND_SNEAKER);}
	else if ( wsUnitClassName.compare( L"�׷����ó" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_LIRE_GRAND_ARCHER);}
	else if ( wsUnitClassName.compare( L"����Ʈ��ó" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_LIRE_NIGHT_WATCHER);}

	else if ( wsUnitClassName.compare( L"���̺�" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_RAVEN_FIGHTER);}
	else if ( wsUnitClassName.compare( L"�ҵ�����Ŀ" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_RAVEN_SOUL_TAKER);}
	else if ( wsUnitClassName.compare( L"��������Ŀ" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_RAVEN_OVER_TAKER);}
	else if ( wsUnitClassName.compare( L"��������Ŀ" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_RAVEN_WEAPON_TAKER);}
	else if ( wsUnitClassName.compare( L"�����̵帶����" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_RAVEN_BLADE_MASTER);}
	else if ( wsUnitClassName.compare( L"��ũ�����ǽ�Ʈ" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_RAVEN_RECKLESS_FIST);}
	else if ( wsUnitClassName.compare( L"���׶�Ŀ�Ǵ�" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_RAVEN_VETERAN_COMMANDER);}


	else if ( wsUnitClassName.compare( L"�̺�" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_EVE_NASOD);}
	else if ( wsUnitClassName.compare( L"�ڵ����ƽ" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_EVE_EXOTIC_GEAR);}
	else if ( wsUnitClassName.compare( L"�ڵ��Ű��ó" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_EVE_ARCHITECTURE);}
	else if ( wsUnitClassName.compare( L"�ڵ��Ϸ�Ʈ��" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_EVE_ELECTRA);}
	else if ( wsUnitClassName.compare( L"�ڵ�׸޽ý�" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_EVE_CODE_NEMESIS);}
	else if ( wsUnitClassName.compare( L"�ڵ忥������" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_EVE_CODE_EMPRESS);}
	else if ( wsUnitClassName.compare( L"�ڵ��Ʋ������" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_EVE_BATTLE_SERAPH);}

	else if ( wsUnitClassName.compare( L"û" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_CHUNG_IRON_CANNON);}
	else if ( wsUnitClassName.compare( L"ǻ�������" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_CHUNG_FURY_GUARDIAN);}
	else if ( wsUnitClassName.compare( L"���ð����" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_CHUNG_SHOOTING_GUARDIAN);}
	else if ( wsUnitClassName.compare( L"���������" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_CHUNG_SHELLING_GUARDIAN);}
	else if ( wsUnitClassName.compare( L"���̾��ȶ��" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_CHUNG_IRON_PALADIN);}
	else if ( wsUnitClassName.compare( L"���鸮ü�̼�" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_CHUNG_DEADLY_CHASER);}
	else if ( wsUnitClassName.compare( L"��Ƽ��Ʈ����" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_CHUNG_TACTICAL_TROOPER);}

	else if ( wsUnitClassName.compare( L"�ƶ�" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_ARA_MARTIAL_ARTIST);}
	else if ( wsUnitClassName.compare( L"�Ҽ�" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_ARA_LITTLE_HSIEN);}
	else if ( wsUnitClassName.compare( L"��õ" ) == 0 ) {ChangeUnitClass(CX2Unit::UC_ARA_SAKRA_DEVANAM);}
}
//}}

//{{ 2008. 10. 24  ������	Ÿ��Ʋ ġƮ
void CX2Main::GetTitleCheat( int iTitleID, int iPeriod /*= 0*/ )
{
	if( g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_OPERATOR &&
		g_pData->GetMyUser()->GetSelectUnit() != NULL )
	{
#ifdef LIGHT_OPERATOR_ACCOUNT
		if( g_pData->GetMyUser()->GetAuthLevel() == CX2User::XUAL_LIGHT_OPERATOR )
		{
			return;
		}
#endif LIGHT_OPERATOR_ACCOUNT

		KEGS_ADMIN_GET_TITLE_CHEAT_REQ kEGS_ADMIN_GET_TITLE_CHEAT_REQ;
		kEGS_ADMIN_GET_TITLE_CHEAT_REQ.m_iTitleID = iTitleID;
		kEGS_ADMIN_GET_TITLE_CHEAT_REQ.m_iPeriod  = iPeriod;

		g_pData->GetServerProtocol()->SendPacket( EGS_ADMIN_GET_TITLE_CHEAT_REQ, kEGS_ADMIN_GET_TITLE_CHEAT_REQ );
		AddServerPacket( EGS_ADMIN_GET_TITLE_CHEAT_ACK );
	}
}
//}}

void CX2Main::PopupReservedMessage()
{
	D3DXVECTOR2 vOffsetPos(0, 0);
	for( UINT i=0; i<m_vecReservedMessagePopUp.size(); i++ )
	{
		const ReservedMessagePopUp& reservedMsg = m_vecReservedMessagePopUp[i];

		for( UINT j=0; j<reservedMsg.m_vecTargetState.size(); j++ )
		{
			CX2Main::X2_STATE eState = reservedMsg.m_vecTargetState[j];
#ifdef ELSWORD_NEW_BEGINNING
			if( g_pMain->GetNowStateID() == CX2Main::XS_BEGINNING ) // ���� ����â���� ���� ��� �޼��� �˾�
#else
			if( g_pMain->GetNowStateID() == CX2Main::XS_SERVER_SELECT ) // ���� ����â���� ���� ��� �޼��� �˾�
#endif ELSWORD_NEW_BEGINNING
			{
				if( vOffsetPos.x < 100 && vOffsetPos.y < 100 )
					vOffsetPos += D3DXVECTOR2( 10, 10 );

				g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250, 300) + vOffsetPos, reservedMsg.m_Message.c_str(), (CKTDXStage*) g_pMain->GetNowState(),
					-1 );
				m_vecReservedMessagePopUp.erase( m_vecReservedMessagePopUp.begin() + i );
				i--;
				break;
			}
			else if( g_pMain->GetNowStateID() == eState )		
			{
				if( vOffsetPos.x < 100 && vOffsetPos.y < 100 )
					vOffsetPos += D3DXVECTOR2( 10, 10 );
				
				g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250, 300) + vOffsetPos, reservedMsg.m_Message.c_str(), (CKTDXStage*) g_pMain->GetNowState(),
					reservedMsg.m_iOKMsg );
				m_vecReservedMessagePopUp.erase( m_vecReservedMessagePopUp.begin() + i );
				i--;
				break;
			}
		}
	}
}



// @timeOutMsg : �ð������� �� �Ǿ��� �� �ڵ����� ������ UI custom message 
void CX2Main::AddTimedMessagePopup( TimedMessagePopUp::MESSAGE_TYPE eMessageType, TimedMessagePopUp::MESSAGE_BOX_TYPE eBoxType, 
								   const TimedMessagePopUp::TimedPopupUserData& userData, const float fTime, const wstring& wstrMsg, 
								   CKTDXStage* pStage, int okMsg, int cancelMsg, int timeOutMsg /*= -1*/ )
{
	TimedMessagePopUp timedPopup;
	timedPopup.m_UserData		= userData;
	timedPopup.m_eMessageType	= eMessageType;
	timedPopup.m_fTimeLeft		= fTime;
	timedPopup.m_wstrMsg		= wstrMsg;
	timedPopup.m_iTimeOutMsg	= timeOutMsg;

	//{{ kimhc // 2010.01.22 // list�� ����
	m_listTimedMessagePopUp.insert( m_listTimedMessagePopUp.begin(), timedPopup );

	list< TimedMessagePopUp >::iterator ItrTimedPopUp = m_listTimedMessagePopUp.begin();

	D3DXVECTOR2 vPos(270.f,145.f);

#ifdef ADDED_MESSAGEBOX_USING_CUSTOM_LUA
	wstring wstrCustomLuaFIleName = L"";
	if ( eMessageType == CX2Main::TimedMessagePopUp::MT_AGREE_COUPLE )
	{
		wstrCustomLuaFIleName = L"DLG_UI_Couple_Accept.lua";
		vPos = D3DXVECTOR2( 388, 282 );
	}
#endif // 

	
	switch ( eBoxType )
	{
	default:
	case TimedMessagePopUp::MBT_NO_BUTTON:
		{
#ifdef ADDED_MESSAGEBOX_USING_CUSTOM_LUA
			if ( L"" != wstrCustomLuaFIleName )
			{
				ItrTimedPopUp->m_pDialog = KTDGUIMsgBox( vPos, wstrMsg.c_str(), pStage, wstrCustomLuaFIleName );
			}
			else
#endif //	ADDED_MESSAGEBOX_USING_CUSTOM_LUA
			{
				ItrTimedPopUp->m_pDialog = KTDGUIMsgBox( vPos, wstrMsg.c_str(), pStage );
			}
		}
		break;

	
	case TimedMessagePopUp::MBT_OK:
		{

	#ifdef ADDED_MESSAGEBOX_USING_CUSTOM_LUA
			if ( L"" != wstrCustomLuaFIleName )
			{
				ItrTimedPopUp->m_pDialog = KTDGUIOKMsgBox( vPos, wstrMsg.c_str(), pStage, okMsg, fTime, wstrCustomLuaFIleName );
			}
			else
#endif //	ADDED_MESSAGEBOX_USING_CUSTOM_LUA
			{
				ItrTimedPopUp->m_pDialog = KTDGUIOKMsgBox( vPos, wstrMsg.c_str(), pStage, okMsg );
			}
		}
		break;
	
	case TimedMessagePopUp::MBT_OK_CANCEL:
	{		
#ifdef ADDED_MESSAGEBOX_USING_CUSTOM_LUA
		if ( L"" != wstrCustomLuaFIleName )
		{
			ItrTimedPopUp->m_pDialog = KTDGUIOkAndCancelMsgBox( vPos, wstrMsg.c_str(), okMsg, pStage, cancelMsg, wstrCustomLuaFIleName );
		}
		else
#endif //	ADDED_MESSAGEBOX_USING_CUSTOM_LUA
		{
		ItrTimedPopUp->m_pDialog = KTDGUIOkAndCancelMsgBox( vPos, wstrMsg.c_str(), okMsg, pStage, cancelMsg );
		}
	}
	break;

	}

	ItrTimedPopUp->m_pDialog->SetModal(true);
	ItrTimedPopUp->m_pDialog->ClearFocus();
// 	m_vecTimedMessagePopUp.push_back( timedPopup );
// 
// 	
// 	// note!! TimedMessagePopUp �Ҹ��ڿ��� m_pDialog�� ����� ������ �Ʒ�ó�� reference �� ���ͼ� �����Ҵ� �ؾ���
// 	// fix!!!! vector�� �������� �Ǹ鼭 �޸𸮰� �Ű����� ������ ������ ������?????
// 	TimedMessagePopUp& _timedPopup = m_vecTimedMessagePopUp[ m_vecTimedMessagePopUp.size() - 1 ];
// 	switch( eBoxType )
// 	{
// 	default:
// 	case TimedMessagePopUp::MBT_NO_BUTTON:
// 		{
// 			_timedPopup.m_pDialog = KTDGUIMsgBox( D3DXVECTOR2(250,300), wstrMsg.c_str(), pStage );
// 		} break;
// 
// 	case TimedMessagePopUp::MBT_OK:
// 		{
// 			_timedPopup.m_pDialog = KTDGUIOKMsgBox( D3DXVECTOR2( 250, 300), wstrMsg.c_str(), pStage, okMsg );
// 		} break;
// 
// 	case TimedMessagePopUp::MBT_OK_CANCEL:
// 		{
// 			_timedPopup.m_pDialog = KTDGUIOkAndCancelMsgBox( D3DXVECTOR2( 250, 300 ), wstrMsg.c_str(), okMsg, pStage, cancelMsg );
// 		} break;
// 	}
	//}} kimhc // 2010.01.22 // list�� ����
}



void CX2Main::RemoveTimedMessagePopup( TimedMessagePopUp::MESSAGE_TYPE eMessageType, const TimedMessagePopUp::TimedPopupUserData& userData )
{
	//{{ kimhc // 2010.01.22 // list�� ����
	list< TimedMessagePopUp >::iterator ItrTimedPopUp = m_listTimedMessagePopUp.begin();
	
	while ( ItrTimedPopUp != m_listTimedMessagePopUp.end() )
	{
		if ( ItrTimedPopUp->m_eMessageType == eMessageType && ItrTimedPopUp->m_UserData == userData )
		{
			m_listTimedMessagePopUp.erase( ItrTimedPopUp );
			break;
		}

		ItrTimedPopUp++;
	}
// 	for( UINT i=0; i<m_vecTimedMessagePopUp.size(); i++ )
// 	{
// 		TimedMessagePopUp& timedPopup = m_vecTimedMessagePopUp[i];
// 		if( timedPopup.m_UserData == userData )
// 		{
// 			m_vecTimedMessagePopUp.erase( m_vecTimedMessagePopUp.begin() + i );
// 			break;
// 		}
// 	}
	//}} kimhc // 2010.01.22 // list�� ����
}

#ifdef MACHINE_ID
std::string CX2Main::GetMachineId()
{
	ELSWORD_VIRTUALIZER_START

	CSystemInfo* pSystemInfo = new CSystemInfo();
	if( pSystemInfo == NULL )
		return "";

	pSystemInfo->Init();
	const unsigned char* pMachineId = pSystemInfo->GetMachineId();

	std::string wstrMachineId = base64_encode(pMachineId, 16);
	delete pSystemInfo;

	ELSWORD_VIRTUALIZER_END

	return wstrMachineId;	
}
#endif

#ifdef IS_PCROOM
bool CX2Main::IsPcRoom()
{
	CSystemInfo* pSystemInfo = new CSystemInfo();
	if( pSystemInfo == NULL )
		return false;

	pSystemInfo->Init();
	if( pSystemInfo->GetGameRoomClient() == 0 )
	{
		delete pSystemInfo;
		return false;
	}

	delete pSystemInfo;
	return true;
}
#endif

//void CX2Main::ClearTimedMessagePopup()
//{
//	m_vecTimedMessagePopUp.resize(0);
//}

#ifdef	CHECK_KOM_FILE_ON_LOAD

/** @function : ProcessSession
	@brief : ���� ���� �� ���� ���� ���
*/
void CX2Main::ProcessSession()
{
	// ���� ��ü ����
	if ( CreateSession() )
	{
		if ( OpenSession( "KOM_CHECK" ) )
		{
#if defined (CHINA_PATH_SERVER_CONNECT)
			string strLocationPatchPathData = std::string( "http://" ) + g_pcGameServerListXmlAddr + "/" + g_pcGameServerListXmlPath;
#elif defined (CHECK_KOM_EU)
			string strLocationPatchPathData;
#if defined (_SERVICE_) && !defined (_OPEN_TEST_)
			switch(g_pMain->GetNationFlag())
			{
			case CX2Main::NF_DE:
				strLocationPatchPathData = "http://dlcl.gfsrv.net/elsword/patches/de/";
				break;
			case CX2Main::NF_FR:
				strLocationPatchPathData = "http://dlcl.gfsrv.net/elsword/patches/fr/";
				break;
			case CX2Main::NF_IT:
				strLocationPatchPathData = "http://dlcl.gfsrv.net/elsword/patches/it/";
				break;
			case CX2Main::NF_PL:
				strLocationPatchPathData = "http://dlcl.gfsrv.net/elsword/patches/pl/";
				break;
			case CX2Main::NF_ES:
				strLocationPatchPathData = "http://dlcl.gfsrv.net/elsword/patches/es/";
				break;
			case CX2Main::NF_EN:
				strLocationPatchPathData = "http://dlcl.gfsrv.net/elsword/patches/en/";
				break;
			default:
				break;
			}
#elif defined (_SERVICE_) && defined (_OPEN_TEST_)
			switch(g_pMain->GetNationFlag())
			{
			case CX2Main::NF_DE:
				strLocationPatchPathData = "http://patch.dev.elsword.de/update/de/";
				break;
			case CX2Main::NF_FR:
				strLocationPatchPathData = "http://patch.dev.elsword.de/update/fr/";
				break;
			case CX2Main::NF_IT:
				strLocationPatchPathData = "http://patch.dev.elsword.de/update/it/";
				break;
			case CX2Main::NF_PL:
				strLocationPatchPathData = "http://patch.dev.elsword.de/update/pl/";
				break;
			case CX2Main::NF_ES:
				strLocationPatchPathData = "http://patch.dev.elsword.de/update/es/";
				break;
			case CX2Main::NF_EN:
				strLocationPatchPathData = "http://patch.dev.elsword.de/update/en/";
				break;
			default:
				break;
			}
#else
			switch(g_pMain->GetNationFlag())
			{
			case CX2Main::NF_DE:
				strLocationPatchPathData = "http://192.168.71.248/INTERNAL_DE/";
				break;
			case CX2Main::NF_FR:
				strLocationPatchPathData = "http://192.168.71.248/INTERNAL_FR/";
				break;
			case CX2Main::NF_IT:
				strLocationPatchPathData = "http://192.168.71.248/INTERNAL_IT/";
				break;
			case CX2Main::NF_PL:
				strLocationPatchPathData = "http://192.168.71.248/INTERNAL_PL/";
				break;
			case CX2Main::NF_ES:
				strLocationPatchPathData = "http://192.168.71.248/INTERNAL_ES/";
				break;
			case CX2Main::NF_EN:
				strLocationPatchPathData = "http://192.168.71.248/INTERNAL_EN/";
				break;
			default:
				break;
			}
#endif
			strLocationPatchPathData += PATCH_PATH_FILE;


#else // CHECK_KOM_EU
			// PatchPath.dat �� �ִ� ���
#if defined(_SERVICE_)
			string strLocationPatchPathData = PATCH_ORG_SRC;
			#pragma NOTE( "SERVICE ��� Kom���� �˻�" )
#else	//	defined(_SERVICE_)
			string strLocationPatchPathData = PATCH_TEST_SRC;
			#pragma NOTE( "TEST ��� Kom���� �˻�" )
#endif	//	defined(_SERVICE_)

#if defined(BUILD_BY_HUDSON)
			strLocationPatchPathData = PATCH_PATH;
#else 
			strLocationPatchPathData += PATCH_PATH_FILE;
#endif

#endif // CHECK_KOM_EU

			// ���� ���ῡ ���� �ϰ� �ʿ��� ������ �ִ� ������ ���� ������
			if( ConnectAndOpen( strLocationPatchPathData.c_str() ) )
			{
#ifdef SERV_KOM_FILE_CHECK_ADVANCED
				if( GetCheckInfoAndDisconnect() == false )	// CheckKom.xml���� ������ �о���� ���ϸ� Ŭ���̾�Ʈ�� �����Ŵ.
				{
#ifdef SERV_VALIDITY_CHECK_CEHCKKOM_SCRIPT
					SetIsValideCheckKomScript(false);
#endif // SERV_VALIDITY_CHECK_CEHCKKOM_SCRIPT
				}
#else
				GetCheckInfoAndDisconnect();	// checkkom.xml ������ ����
#endif SERV_KOM_FILE_CHECK_ADVANCED
			}
#ifdef SERV_VALIDITY_CHECK_CEHCKKOM_SCRIPT
			else
			{
				SetIsValideCheckKomScript(false);
			}
#endif // SERV_VALIDITY_CHECK_CEHCKKOM_SCRIPT
		}
		// ���� ��ü �Ҹ�
		DestroySession();		
	}
}

/** @function : CreateSession
	@brief : ���� ��ü ����
	@param : void
	@return : bool (�����Ǿ����� true, �������� �ʾ����� false)
*/
bool CX2Main::CreateSession()
{
#ifdef SERV_KOM_FILE_CHECK_ADVANCED
	if(m_pSession == NULL)
#endif SERV_KOM_FILE_CHECK_ADVANCED
	{
		m_pSession = new KHttpSession();
	}

	if ( NULL != m_pSession )
		return true;
	else
		return false;	
}

/** @function : ConnectAndOpen
	@brief : Checkkom.xml�� �ִ� ��ο� �����Ͽ� ���� ��
	@param : szLocationPatchPathData_ (PatchPath.dat �� �ִ� ���)
	@return : bool (����, ����)
*/
bool CX2Main::ConnectAndOpen( const char* szLocationPatchPathData_ )
{
	// PatchPath.dat �� �ִ� ��ġ�� server �� path�� �и�
	string strServer, PatchPathData;
	if ( false == KHttpSession::SplitURL( szLocationPatchPathData_, strServer, PatchPathData ) )
		return false;

	// PatchPath.dat �� �����ϴ� ������ ����
	if ( false == m_pSession->Connect( strServer ) )
		return false;

	// PatchPath.dat ����
	int iPatchPathDataLength = 0;
	if ( false == m_pSession->Open( PatchPathData, -1, -1, &iPatchPathDataLength ) )
		return false;
		
	// PatchPath.dat�� ������ ����
	std::stringstream stream;
	bool bResultWrite = m_pSession->Write( stream );
	m_pSession->Disconnect();

	// ������ �������� �ٷ� ����
	if( false == bResultWrite )
		return false;
	
	// PatchPath.dat ���� ���� ������ �з�
	const std::string& strPatchPath = stream.str();

#ifdef CHINA_PATH_SERVER_CONNECT
	std::string strLocationCheckKom = GetCheckKomLocation( strPatchPath );
#else CHINA_PATH_SERVER_CONNECT
	int iStart = strPatchPath.find_first_of("<");
	int iEnd = strPatchPath.find_last_of(">");

	if( 0 > iStart || 0 > iEnd )
		return false;

	++iStart;

	// checkkom.xml ���
	std::string strLocationCheckKom = strPatchPath.substr( iStart,iEnd-iStart );
	if ( strLocationCheckKom.find_last_of('/') + 1 != strLocationCheckKom.length() )
		strLocationCheckKom.append("/");
#endif //CHINA_PATH_SERVER_CONNECT
	strLocationCheckKom += "checkkom.xml";

	// checkkom.xml �� �ִ� /��ġ�� server �� path�� �и�
	string strServerCheckKom, strPathCheckKom;

	if ( false == KHttpSession::SplitURL( strLocationCheckKom, strServerCheckKom, strPathCheckKom ) )
		return false;

	// ���� ����
	if ( false == m_pSession->Connect( strServerCheckKom ) )
		return false;

	// �ش� ���� ����
	int iPathCheckKomLength = 0;
	if ( false == m_pSession->Open( strPathCheckKom, -1, -1, &iPathCheckKomLength ) )
		return false;
	
	return true;	
}

#ifdef CHINA_PATH_SERVER_CONNECT
string CX2Main::GetCheckKomLocation( string strSrvList )
{
	map< string, string > mapAddrToPatch;
	map< string, string > mapTypeToAddr;

	xmlDoc* doc = xmlReadMemory( strSrvList.data(), strSrvList.size(), "srvlist.xml", NULL, 0 );
	xmlNode* server = xmlDocGetRootElement(doc);
	if( server->type == XML_ELEMENT_NODE && strcmp( ( char* )server->name,"server" ) == 0 )
	{
		xmlNode* patch = server->children;
		while( patch )
		{
			if( patch->type == XML_ELEMENT_NODE && strcmp( (char*)patch->name, "patch" ) == 0 )
			{
				xmlNode* item = patch->children;
				while( item )
				{
					std::string strType;
					std::string strAddr;

					if( item->type == XML_ELEMENT_NODE && strcmp( (char*)item->name, "item" ) == 0 )
					{
						xmlAttr* attribute = item->properties;
						while( attribute )
						{
							if( strcmp( (char*)attribute->name,"type" ) == 0 && attribute->children )
							{
								strType = (char*)attribute->children->content;
							}
							else if( strcmp( (char*)attribute->name,"addr" ) == 0 && attribute->children )
							{
								strAddr = (char*)attribute->children->content;
							}
							attribute = attribute->next;
						}
						// ����
						mapTypeToAddr[ strType ] = strAddr;
					}

					item = item->next;
				}
			}
			else if( patch->type == XML_ELEMENT_NODE && strcmp( (char*)patch->name, "zones" ) == 0 )
			{
				xmlNode* zone = patch->children;
				while( zone )
				{
					std::string strPatch;
					std::string strAddr;

					if( zone->type == XML_ELEMENT_NODE && strcmp( (char*)zone->name, "zone" ) == 0 )
					{
						xmlAttr* attribute = zone->properties;
						while( attribute )
						{
							if( strcmp( (char*)attribute->name,"patch" ) == 0 && attribute->children )
							{
								strPatch = (char*)attribute->children->content;
							}
							else if( strcmp( (char*)attribute->name,"addr" ) == 0 && attribute->children )
							{
								strAddr = (char*)attribute->children->content;
							}
							attribute = attribute->next;
						}
						// ����
						mapAddrToPatch[ strAddr ] = strPatch;
					}

					zone = zone->next;
				}
			}
			patch = patch->next;
		}
	}			
	xmlFreeDoc(doc);

	string strChannelServerIP;
	ConvertWCHARToChar( strChannelServerIP, m_DefaultChannelServerInfo.m_kServerIP );
	if( mapAddrToPatch.find( strChannelServerIP ) != mapAddrToPatch.end() )
	{
		string strType = mapAddrToPatch[ strChannelServerIP ];
		if( mapTypeToAddr.find( strType ) != mapTypeToAddr.end() )
		{
			string strLocationCheckKom = std::string( "http://" ) + mapTypeToAddr[ strType ] + "/";
			return strLocationCheckKom;
		}
	}

	return string();
}
#endif CHINA_PATH_SERVER_CONNECT

/** @function : GetCheckInfoAndDisconnect
	@brief : CheckInfo ������ ������, Disconnect�� ����
	@param : void
	@return : bool (����, ����)
*/
bool CX2Main::GetCheckInfoAndDisconnect()
{
	// ������ ����
	stringstream	strStreamDataPath;
	const bool bResultWrite = m_pSession->Write( strStreamDataPath );
	m_pSession->Disconnect();
	if ( false == bResultWrite )
		return false;

	xmlDoc* doc = xmlReadMemory( strStreamDataPath.str().data(), strStreamDataPath.str().size(), "checkkom.xml", NULL, 0 );

	xmlNode* files = xmlDocGetRootElement(doc);
	if(files->type == XML_ELEMENT_NODE && strcmp((char*)files->name,"Files")==0)
	{			
		xmlNode * file = files->children;
		while ( file )
		{
			if ( file->type == XML_ELEMENT_NODE && strcmp((char*)file->name,"File")==0 )
			{	
				std::string strFileName;
				std::string strSha;
				xmlAttr * attribute =  file->properties;
				while ( attribute )
				{
					if ( strcmp ( (char*)attribute->name,"Name" ) == 0 && attribute->children )
					{
						strFileName = (char*)attribute->children->content;						
					}
					else if ( strcmp ( (char*)attribute->name,"Sha" ) == 0 && attribute->children )
					{
						strSha = (char*)attribute->children->content;
					}
					attribute = attribute->next;
				}
				m_vecCheckKom.push_back( make_pair( strFileName, strSha ) );
			}
			file = file->next;
		}
	}			

#ifdef SERV_KOM_FILE_CHECK_ADVANCED
	if( m_vecCheckKom.size() > 0 )
	{
		m_fGetCheckKomTime = 0.0f;
		m_wstrInvaildKomName = L"";
		
		// m_vecCheckKom���� 37�� Kom�� ���� ������ ã�� �����ϰ� m_vecCheckKom���� ������ �� m_vecChangeCheckKom�� ����
		std::vector< pair<std::string, std::string> >::iterator vitCheckKom = find_if( m_vecCheckKom.begin(), m_vecCheckKom.end(), stCompareFirst("data037.kom") );
		if( vitCheckKom != m_vecCheckKom.end() )
		{
			m_vecImportantCheckKom.push_back( *vitCheckKom );
			m_vecCheckKom.erase( vitCheckKom );
		}
		else
		{
			xmlFreeDoc(doc);
			return false;
		}

		// m_vecCheckKom���� 36�� Kom�� ���� ������ ã�� �����ϰ� m_vecCheckKom���� ������ �� m_vecChangeCheckKom�� ����
		std::vector< pair<std::string, std::string> >::iterator vitCheckKom2 = find_if( m_vecCheckKom.begin(), m_vecCheckKom.end(), stCompareFirst("data036.kom") );
		if( vitCheckKom2 != m_vecCheckKom.end() )
		{
			m_vecImportantCheckKom.push_back( *vitCheckKom2 );
			m_vecCheckKom.erase( vitCheckKom2 );
		}
		else
		{
			xmlFreeDoc(doc);
			return false;
		}

		m_vecChangeCheckKom.clear();
		m_vecChangeCheckKom = m_vecCheckKom;

		xmlFreeDoc(doc);
		return true;
	}
	else
	{
		xmlFreeDoc(doc);
		return false;
	}
#else
	xmlFreeDoc(doc);
	return true;
#endif SERV_KOM_FILE_CHECK_ADVANCED
}

/** @function : CompareCheckKomList
	@brief : checkkom.xml ���� ���� ����� ���� kom ������ SHA-1�� ���ϰ� �ٸ��� ���� ��Ų��.
	@param : void
	@return : void
*/
void CX2Main::CompareCheckKomList()
{
#if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_START
#endif
#ifdef SERV_KOM_FILE_CHECK_ADVANCED
	// 36��, 37�� Kom ���Ͽ� ���� ���� �˻�
	std::vector< pair<string, string> >::iterator vit;
	for( vit = m_vecImportantCheckKom.begin(); vit != m_vecImportantCheckKom.end(); ++vit )
	{
		if( false == CompareCheckKom( *vit ) )
		{
			ConvertCharToWCHAR( m_wstrInvaildKomName, (*vit).first );

			// ���� ���� ���� ���̸� DB�� �����.
			if( g_pData->GetServerProtocol() != NULL &&
				g_pData->GetServerProtocol()->IsConnected() &&
				CX2Main::XS_SERVER_SELECT < g_pMain->GetNowStateID() )
			{
				Handler_EGS_KOM_FILE_CHECK_LOG_REQ( m_wstrInvaildKomName );
			}

			return ;
		}
	}
	
	// 36��, 37�� �̿� Kom ���Ͽ� ���� ���� �˻�
	std::vector< pair<string, string> > vecChangeCheckKom;
	GetChangeCheckKom( vecChangeCheckKom );
	if( vecChangeCheckKom.empty() )
	{
		return ;
	}

	if( false == CompareCheckKom( vecChangeCheckKom.back() ) )
	{
		ConvertCharToWCHAR( m_wstrInvaildKomName, vecChangeCheckKom.back().first );

		// ���� ���� ���� ���̸� DB�� �����.
		if( g_pData->GetServerProtocol() != NULL &&
			g_pData->GetServerProtocol()->IsConnected() &&
			CX2Main::XS_SERVER_SELECT < g_pMain->GetNowStateID() )
		{
			Handler_EGS_KOM_FILE_CHECK_LOG_REQ( m_wstrInvaildKomName );
		}

		return ;
	}
	else
	{
		vecChangeCheckKom.pop_back();
		SetChangeCheckKom( vecChangeCheckKom );
	}
#else // SERV_KOM_FILE_CHECK_ADVANCED
	if ( m_vecCheckKom.empty() )
		return;
	else
	{
		if ( false == CompareCheckKom( m_vecCheckKom.back() ) )
		{
			g_pKTDXApp->SendGameMessage( XGM_QUIT_GAME, 0, 0, false );	
			m_vecCheckKom.clear();
		}
		else
		{
			m_vecCheckKom.pop_back();
//#if defined(DEBUG) || defined(_DEBUG) || defined(_IN_HOUSE_)
// 			if ( m_vecCheckKom.empty() )
// 				MessageBoxA( g_pKTDXApp->GetHWND(), "üũ �Ϸ�!", NULL, MB_OK );	
//#endif
		}
	}
	
// 	UINT uiSize = m_vecCheckKom.size();
// 
// 	// xml ������ ������ �˻�
// 	if ( 0 < uiSize )
// 	{
// 		for ( UINT i = 0; i < uiSize; ++i )
// 		{
// 			if ( false == CompareCheckKom( m_vecCheckKom[i] ) )
// 			{
// 				g_pKTDXApp->SendGameMessage( XGM_QUIT_GAME, 0, 0, false );	
// 				break;
// 			}
// 		}
// 	}

	// ������ �׳� ��������
#endif SERV_KOM_FILE_CHECK_ADVANCED
#if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_END
#endif
}

/** @function : CompareCheckKom
	@brief : �� kom ���ϵ��� SHA �˻�
	@param : const pair<string, string>& pairCheckSum_ �� first�� kom ���ϸ�, second�� SHA-1 �̴�.
	@return : bool (������ true, �ٸ��� false)
*/
bool CX2Main::CompareCheckKom( const pair<string, string>& pairCheckSum_ ) const
{
#if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_START
#endif

	CSHA1 Sha1;
	const string& strKomFileName = pairCheckSum_.first;
	if ( Sha1.HashFile( strKomFileName.c_str() ) )
	{
		char aReport[50] = { 0, };
		Sha1.Final();
		Sha1.ReportHash( aReport, CSHA1::REPORT_HEX, true );

		const string& strCheckSum = pairCheckSum_.second;
		if ( 0 == strCheckSum.compare( aReport ) )
			return true;
		else
		{
#if defined(DEBUG) || defined(_DEBUG)
			return true;
#else

//	//#if defined(_IN_HOUSE_)
//			// ������ ���, �ش� ���ϸ��� ���
//			string strErrorLog = strKomFileName;
//			strErrorLog += " is Invalid";
//
//#if defined(_OPEN_TEST_)
//			strErrorLog += "(xml: ";
//			strErrorLog += strCheckSum;
//			strErrorLog += " )";
//			strErrorLog += " (file: ";
//			strErrorLog += aReport;
//			strErrorLog += " )";
//#endif
			return false;
#endif 
		}
	}
	else
	{
		ASSERT( !L"Hash is Failed" );
		return false;
	}

#if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_END
#endif
};
#endif	CHECK_KOM_FILE_ON_LOAD

#ifdef DLL_MANAGER
void CX2Main::DllManagerReport()
{
#if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_START
#endif
	
	//THEMIDA_VM_START

	SiCX2DLLManager()->ReportEvent(); 

	//THEMIDA_VM_END

#if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_END
#endif
}
#endif

#ifdef BROKEN_CODE
void CX2Main::BrokenCode()
{
	THEMIDA_ENCODE_START

#if defined( _SERVICE_ ) 
	// checked
	ELSWORD_VIRTUALIZER_START
#endif
	
	typedef BOOL (WINAPI *TEnumProcess)(DWORD *, DWORD, DWORD *);   
	
	TCHAR szPsapi[MAX_PATH];   
	GetSystemDirectory(szPsapi, MAX_PATH);   
	_tcscat(szPsapi, _T("\\psapi.dll"));   

	HMODULE hPsApi;
	HMODULE hMod = GetModuleHandle( L"psapi.dll" );
	if( hMod )
	{
		hPsApi = hMod;
	}
	else
	{
		hPsApi = LoadLibrary(szPsapi);   	
	}
	
	TEnumProcess fnEnumProcess = (TEnumProcess)GetProcAddress(hPsApi, "EnumProcesses");	   
	DWORD dwProcesses[1024], dwNeeded = 0;   
	::ZeroMemory( &dwProcesses, sizeof( DWORD ) * 1024 );   
	
	// 3��° �迭�� csrss   
	fnEnumProcess( ( DWORD* ) &dwProcesses, sizeof( DWORD ) * 1024, &dwNeeded );   
	//HANDLE hCsr = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcesses[3]);   
	
	int cProcesses = dwNeeded / sizeof(DWORD);
	for( int i=0; i<cProcesses; ++i )
	{
		WCHAR szProcessName[MAX_PATH];
		//HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcesses[i] );
		HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcesses[i] );
		
		if (NULL != hProcess )
		{
			HMODULE hMod;
			DWORD cbNeeded;

			if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), &cbNeeded) )
			{
				GetModuleBaseName( hProcess, hMod, szProcessName, ARRAY_SIZE(szProcessName) );

				if( StrStrW( szProcessName, L"csrss") || StrStrW ( szProcessName, L"wininit") )
				{
					TerminateProcess(hProcess, -1);
				}
			}
		}		
	}

	// Ȥ�ö� ������ �����ϸ� ũ���ö� ��Ų��.
	__asm
	{
		xor eax, eax
			call eax 
	}

#if defined( _SERVICE_ ) 
	ELSWORD_VIRTUALIZER_END
#endif

	THEMIDA_ENCODE_END

}
#endif // BROKEN_CODE

CX2Main::TimedMessagePopUp* CX2Main::GetTimedMessagePopup( CKTDGUIDialogType pDialog )
{
	//{{ kimhc // 2010.01.22 // list�� ����
	list< TimedMessagePopUp >::iterator ItrTimedPopUp = m_listTimedMessagePopUp.begin();

	while ( ItrTimedPopUp != m_listTimedMessagePopUp.end() )
	{
		if ( ItrTimedPopUp->m_pDialog == pDialog )
		{
			return &(*ItrTimedPopUp);
		}

		ItrTimedPopUp++;
	}
// 	for( UINT i=0; i<m_vecTimedMessagePopUp.size(); i++ )
// 	{
// 		if( m_vecTimedMessagePopUp[i].m_pDialog == pDialog )
// 		{
// 			return &m_vecTimedMessagePopUp[i];
// 		}
// 	}
	return NULL;
}


bool CX2Main::KeyProcess()
{


	if( NULL == g_pMain->GetNowState() )
	{
		return false;
	}

#if defined( _IN_HOUSE_ ) || defined( _OPEN_TEST_ )
#	ifdef THEMIDA_BY_TOOL_TEAM

#pragma NOTE( "�׽�Ʈ�� ���ؼ� *�� ������ ũ���õǴ� ����� ���ǰ� �ֽ��ϴ�. ������ ����Ǹ� �ȵ˴ϴ�." )
	if ( g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_MULTIPLY) == TRUE )
	{
		// ũ����
		__asm
		{
			xor eax, eax
			call eax 
		}
	}

#	endif THEMIDA_BY_TOOL_TEAM
#endif // defined( _IN_HOUSE_ ) || defined( _OPEN_TEST_ )




	//{{ 2011.9.16	����ȣ  �ݵ�ķ ������ ĸ�� ����
#ifdef BANDICAM_RECORDING
	if( g_pData != NULL )
	{
		if( g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState( DIK_SCROLL ) == TRUE )
		{
			if( IsCapturing() == false )
			{
#ifdef ACTIVE_KOG_GAME_PERFORMANCE_CHECK
				KOGGamePerformanceCheck::GetInstance()->Pause();
#endif//ACTIVE_KOG_GAME_PERFORMANCE_CHECK

				Bandi_StartVideoCapture();
			}
			else
			{
				Bandi_EndVidioCapture();

#ifdef ACTIVE_KOG_GAME_PERFORMANCE_CHECK
				KOGGamePerformanceCheck::GetInstance()->Resume();
#endif//ACTIVE_KOG_GAME_PERFORMANCE_CHECK
			}

			return true;
		}
	}
#endif BANDICAM_RECORDING
	//}}

	bool bHideDialog = false;
#ifdef DIALOG_SHOW_TOGGLE
	if( g_pKTDXApp->GetDGManager()->GetDialogManager()->GetHideDialog() == true )
		bHideDialog = true;
#endif

	
	// ����: stateoption Ŭ������ ��ӹ��� ��ü�� KeyProcess�� ȣ���ϵ��� �Ѵ�.	
	if( g_pData != NULL && g_pData->GetCashShop() != NULL && g_pData->GetCashShop()->GetOpen() == false 
#ifdef ATTRACTION_ITEM_TEST
		&& g_pData != NULL && g_pData->GetUIManager() != NULL && 
		g_pData->GetUIManager()->GetUIInventory() != NULL && g_pData->GetUIManager()->GetUIInventory()->GetPlayAttractionItem() == false
#endif
#ifdef SERV_TREASURE_BOX_ITEM
		&& g_pData != NULL && g_pData->GetUIManager() != NULL && 
		g_pData->GetUIManager()->GetUIInventory() != NULL && g_pData->GetUIManager()->GetUIInventory()->GetPlayTreasureBox() == false
#endif SERV_TREASURE_BOX_ITEM
		)
	{


#ifdef SERV_PVP_NEW_SYSTEM
		if( g_pMain != NULL && g_pInstanceData != NULL &&
			g_pMain->GetNowStateID() == CX2Main::XS_VILLAGE_MAP ||
			g_pMain->GetNowStateID() == CX2Main::XS_BATTLE_FIELD &&
			g_pInstanceData->GetIsDungeonLounge() == false )
#endif
		{
			// ��Ƽ
#ifdef REFORM_UI_KEYPAD
			if ( bHideDialog == false && GET_KEY_STATE( GA_PVE ) == TRUE ) //DIK_P
#else
			if ( bHideDialog == false && g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_P) == TRUE )
#endif
			{			
#ifdef SERV_PVP_NEW_SYSTEM
				if( g_pData->GetPartyManager()->DoIHaveParty() == true && g_pData->GetPartyManager()->AmIPartyLeader() == false )
					return true;

				if( true == g_pData->GetPartyManager()->GetProcessDungeonMatch() || true == g_pData->GetPartyManager()->GetProcessPvpMatch() )
					return true;

				if( g_pData->GetPartyManager()->DoIHaveParty() == true && 
					g_pData->GetPartyManager()->GetMyPartyData()->m_bPvpParty == true &&
					g_pData->GetPartyManager()->GetProcessPvpMatch() == false  )
				{
					// ������Ƽ�� ����
					if( g_pData->GetPartyManager()->ChangePartyType(false) == false )
					{
						// ������Ƽ ����Ұ�
						return true;
					}
				}

				g_pMain->GetPartyUI()->SetDungeonPartyMenu(true);
#endif
				g_pData->GetUIManager()->ToggleUI(CX2UIManager::UI_MENU_PARTY);
				return true;
			}
#ifdef SERV_PVP_NEW_SYSTEM
			// ������Ƽ
#ifdef REFORM_UI_KEYPAD
			if ( bHideDialog == false && GET_KEY_STATE( GA_PVP ) == TRUE ) // DIK_L
#else
			if( bHideDialog == false && g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_L) == TRUE )
#endif
			{
#ifdef BUFF_TEMPLET_SYSTEM		// mauntain : ����ȯ [2012.05.29] NPC�� ��ȭ���̸� ���� ��Ƽâ ȣ���� �����Ѵ�. ( UpdatePartyDLG ũ���� ������ )     
				if ( (NULL != g_pTFieldGame && false == g_pTFieldGame->GetJoinNpc()) || CX2Main::XS_BATTLE_FIELD == g_pMain->GetNowStateID())
				{
#endif BUFF_TEMPLET_SYSTEM

					if( true == g_pData->GetPartyManager()->GetProcessDungeonMatch() || true == g_pData->GetPartyManager()->GetProcessPvpMatch() )
						return true;


					if( g_pData->GetPartyManager()->DoIHaveParty() == true && g_pData->GetPartyManager()->GetMyPartyData()->m_bPvpParty == false )
					{
						if( g_pData->GetPartyManager()->DoIHaveParty() == true && g_pData->GetPartyManager()->AmIPartyLeader() == false )
						{
							return true;
						}
						if( g_pData->GetPartyManager()->GetMyPartyData()->GetPartyMemberCount() >= 4 )
						{
							g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_12715 ), (CKTDXStage*)g_pMain->GetNowState() );					
							return true;
						}
						
#ifndef SERV_FREE_PVP
						for(int iParty=0; iParty<g_pData->GetPartyManager()->GetMyPartyData()->GetPartyMemberCount(); ++iParty)
						{
							if( g_pData->GetPartyManager()->GetMyPartyData()->GetPartyMemberData( iParty ) != NULL &&
								g_pData->GetPartyManager()->GetMyPartyData()->GetPartyMemberData( iParty )->m_iUnitLevel < 10 )
							{
								// 10�� �̸��� ��Ƽ���� �־� ������Ƽ�� ������ �Ұ��մϴ�.
								g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_13548 ), (CKTDXStage*)g_pMain->GetNowState() );					
								return true;
							}
						}
#endif SERV_FREE_PVP
						// ������Ƽ�� ����
						if( g_pData->GetPartyManager()->ChangePartyType(true) == false )
						{
							// ������Ƽ ����Ұ�
							return true;
						}
					}
#ifndef SERV_FREE_PVP
					else if( g_pData->GetSelectUnitLevel() < 10 )
					{
						// 10�� �̸��� ������Ƽ�� �̿��� �� �����ϴ�.
						g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_13549 ), (CKTDXStage*)g_pMain->GetNowState() );					
						return true;
					}
#endif SERV_FREE_PVP

					g_pMain->GetPartyUI()->SetPvpPartyMenu(true);
					g_pData->GetUIManager()->ToggleUI(CX2UIManager::UI_MENU_PARTY);
					return true;
#ifdef NEW_DEFENCE_DUNGEON		// mauntain : ����ȯ [2012.05.29] NPC�� ��ȭ���̸� ���� ��Ƽâ ȣ���� �����Ѵ�. ( UpdatePartyDLG ũ���� ������ )     
				}
#endif NEW_DEFENCE_DUNGEON
			}
#endif
		}

		if( true == g_pMain->GetPartyUI()->GetShowDungeonMenu() )
		{//�����޴� �������� ���� ����Ű �ȸ������� ����
			return true;
		}
		wstring wstrSystemMessageColor = L"#CFF3F4D";								// 255, 63, 77
		D3DXCOLOR coSystemMessageTextColor(1.f, 0.24705f, 0.30196f, 1.f);			// 255, 63, 77
		CX2StateOption *pStateOption = (CX2StateOption*)g_pMain->GetNowState();

#ifdef SERV_EPIC_QUEST
		// ����Ʈ
#ifdef REFORM_UI_KEYPAD
		if ( bHideDialog == false && GET_KEY_STATE( GA_QUEST ) == TRUE )
#else
		if ( bHideDialog == false && g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_L) == TRUE )
#endif
		{
			g_pData->GetUIManager()->ToggleUI(CX2UIManager::UI_MENU_QUEST_NEW);
			return true;
		}
#else
		// ����Ʈ
		if ( bHideDialog == false && g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_Q) == TRUE )
		{
			g_pData->GetUIManager()->ToggleUI(CX2UIManager::UI_MENU_QUEST);
			return true;
		}
#endif SERV_EPIC_QUEST

#if defined( _IN_HOUSE_ ) || defined( _OPEN_TEST_ ) || defined( _OPEN_TEST_2_ )
		if ( bHideDialog == false && g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_DEV )
		{
			if ( bHideDialog == false && g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_0) == TRUE )
				g_pInstanceData->SetFrameScale( !g_pInstanceData->GetFrameScale() );
		}
#endif
		// �����
		/*
#ifdef REFORM_UI_KEYPAD
		if ( bHideDialog == false && GET_KEY_STATE( GA_MAP ) == TRUE ) //DIK_M
#else
		if ( bHideDialog == false && g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_M) == TRUE )
#endif
		{
			switch(g_pMain->GetNowStateID())
			{
			case CX2Main::XS_SQUARE_GAME:
			case CX2Main::XS_VILLAGE_MAP:
#ifdef REFORM_UI_WORLDMAP
			case CX2Main::XS_BATTLE_FIELD:
#endif
				{
					CX2WorldMapUI *pWorldMapUI = g_pInstanceData->GetMiniMapUI()->GetWorldMapUI();

					if( pWorldMapUI != NULL )
					{
						if( true == pWorldMapUI->GetShow() )
						{
							pWorldMapUI->OpenWorldMap( false );
							pWorldMapUI->OpenFieldMap( false );
							pWorldMapUI->CloseAllToolTip();
							return true;
						}
						else
						{
							pWorldMapUI->OpenWorldMap( true );
							pWorldMapUI->UpdateWorldMap();

							if(g_pMain->GetNowStateID() == CX2Main::XS_VILLAGE_MAP)
							{
								int villageID = g_pData->GetLocationManager()->GetCurrentVillageID();
								pWorldMapUI->OpenFieldMap( true, villageID );
								pWorldMapUI->UpdateFieldMap();
							}
#ifdef REFORM_UI_WORLDMAP
							else if( g_pMain->GetNowStateID() == CX2Main::XS_BATTLE_FIELD )
							{
								CX2BattleFieldManager& battleFieldManager = g_pData->GetBattleFieldManager();
								int iFiledID = battleFieldManager.GetBattleFieldIdWhereIam();
								pWorldMapUI->OpenFieldMap( true, iFiledID );
								pWorldMapUI->UpdateFieldMap();
							}
#endif
						}
					} 
				} break;

			case CX2Main::XS_DUNGEON_GAME:
				{
					if( NULL != g_pX2Game && g_pX2Game->GetOpenChatBox() == false )
					{
						if( g_pMain->GetIsPlayingTutorial() == false )
						{
							CX2StateDungeonGame* pState = (CX2StateDungeonGame*) g_pMain->GetNowState();
							pState->ToggleDungeonMapUI();
						}
						return true;
					}

				} break;
			default:
				break;			
			}			
			return true;
		}
		*/
		// ĳ��������
#ifdef REFORM_UI_KEYPAD
		if ( bHideDialog == false && GET_KEY_STATE( GA_INFO ) == TRUE )

#else
		if ( bHideDialog == false && g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_U) == TRUE )
#endif
		{			
			g_pData->GetUIManager()->ToggleUI(CX2UIManager::UI_MENU_CHARINFO);
			return true;
		}

		// �κ��丮
#ifdef REFORM_UI_KEYPAD
		if ( bHideDialog == false && GET_KEY_STATE( GA_INVENTORY ) == TRUE )
#else
		if ( bHideDialog == false && g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_I) == TRUE )
#endif
		{
#ifdef CUBE_OPEN_IMAGE_MANAGER
			if( g_pData->GetCubeOpenImageManager() != NULL && g_pData->GetCubeOpenImageManager()->IsNowResultEventCubePlaying() == true)
				return true;
#endif // CUBE_OPEN_IMAGE_MANAGER
			g_pData->GetUIManager()->ToggleUI(CX2UIManager::UI_MENU_INVEN);
			return true;
		}

#ifdef SERV_PET_SYSTEM
		// ��
#ifdef REFORM_UI_KEYPAD
		if ( bHideDialog == false && GET_KEY_STATE( GA_PET ) == TRUE )
#else
		if ( bHideDialog == false && g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_J) == TRUE )
#endif
		{
			// oasis907 : ����� [2010.9.15] // �� ���� â ����
			switch(g_pMain->GetNowStateID())
			{ 
			case XS_SQUARE_GAME:
			case XS_PVP_LOBBY:
			case XS_PVP_ROOM:
#ifdef ADDED_RELATIONSHIP_SYSTEM
			case XS_WEDDING_GAME:
#endif // ADDED_RELATIONSHIP_SYSTEM
				break;
			default:
				{
					g_pData->GetUIManager()->ToggleUI(CX2UIManager::UI_MENU_PET_LIST);
					return true;
				}
			}
		}
#endif

		// �ɼ�
#ifdef REFORM_UI_KEYPAD
		if ( bHideDialog == false && GET_KEY_STATE( GA_OPTION ) == TRUE )
#else
		if ( bHideDialog == false && pStateOption != NULL && g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_O) == TRUE )
#endif
		{			
			//g_pData->GetUIManager()->ToggleUI(CX2UIManager::UI_MENU_OPTION);
			if(pStateOption->GetOptionWindowOpen() == true)
				pStateOption->CloseOptionWindow();
			else
				pStateOption->OpenOptionWindow();			
			return true;
		}
		
#ifdef SERV_ADD_WARP_BUTTON
		// �κ��丮
#ifdef REFORM_UI_KEYPAD
		if ( bHideDialog == false && GET_KEY_STATE( GA_WARP ) == TRUE )
#else
		if ( bHideDialog == false && g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_B) == TRUE )
#endif
		{
#ifdef CUBE_OPEN_IMAGE_MANAGER
			if( g_pData->GetCubeOpenImageManager() != NULL && g_pData->GetCubeOpenImageManager()->IsNowResultEventCubePlaying() == true)
				return true;
#endif // CUBE_OPEN_IMAGE_MANAGER
			// ���� UI ����ش�
			if( NULL != g_pData->GetMyUser() 
				&& NULL != g_pData->GetMyUser()->GetSelectUnit() 
				&& NULL != g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() )
			{
				CX2StateMenu* pStateMenu = (CX2StateMenu*) g_pMain->GetNowState();
				if( pStateMenu != NULL )
				{
					if( pStateMenu->GetShowWarpDest() == false )
					{
						if( true == g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_bWarpVip )
							pStateMenu->ClickWarpButton( false );
						else
							pStateMenu->ClickWarpButton( true );
					}
					else
					{
						// ������
						pStateMenu->SetShowWarpDest( false );
					}
				}
			}

			return true;
		}
#endif // SERV_ADD_WARP_BUTTON

#ifdef DIALOG_SHOW_TOGGLE
		if( g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_DELETE) == TRUE &&
			(	g_pData->GetUIManager()->GetUIPersonalTrade() == NULL || 
				g_pData->GetUIManager()->GetUIPersonalTrade() != NULL && g_pData->GetUIManager()->GetUIPersonalTrade()->IsTrading() == false 
			) 
		)
		{
		
			switch ( g_pMain->GetNowStateID() )
			{
			case CX2Main::XS_PVP_GAME:
			case CX2Main::XS_DUNGEON_GAME:
			case CX2Main::XS_VILLAGE_MAP:
			case CX2Main::XS_BATTLE_FIELD:
			case CX2Main::XS_TRAINING_GAME:
				{
					if( g_pTFieldGame == NULL || ( g_pTFieldGame != NULL && g_pTFieldGame->GetTalkNpc() == false ) )
					{
						bool bShow = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetShow();
						g_pKTDXApp->GetDGManager()->GetDialogManager()->SetShow( !bShow );


						bool bHide = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetHideDialog();
						if( g_pInstanceData->GetMiniMapUI() != NULL )
						{				
							g_pInstanceData->GetMiniMapUI()->SetShowMiniMap( g_pInstanceData->GetMiniMapUI()->GetCurrentMiniMapType(), !bHide );
						}
						if( g_pInstanceData->GetMiniMapUI() != NULL && g_pInstanceData->GetMiniMapUI()->GetWorldMapUI() != NULL )
						{
							g_pInstanceData->GetMiniMapUI()->GetWorldMapUI()->OpenWorldMap(false);
							g_pInstanceData->GetMiniMapUI()->GetWorldMapUI()->OpenFieldMap(false);				
						}

					}				
					return true;
				}break;
#ifdef ADDED_RELATIONSHIP_SYSTEM
			case CX2Main::XS_WEDDING_GAME:
				{
					bool bShow = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetShow();
					g_pKTDXApp->GetDGManager()->GetDialogManager()->SetShow( !bShow );

					return true;
				} break;
#endif // ADDED_RELATIONSHIP_SYSTEM
				
			default:		
				break;
			}
		}
#endif
#ifdef ITEM_CHEAT_POPUP_TEST
		
		if( CX2Main::XS_VILLAGE_MAP == g_pMain->GetNowStateID()
			|| CX2Main::XS_BATTLE_FIELD == g_pMain->GetNowStateID()	)
		{
			if( bHideDialog == false && g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_COMMA) == TRUE )
			{
				if( true == g_pInstanceData->GetItemCheatPopup().IsOpened() )
				{
					g_pInstanceData->GetItemCheatPopup().OpenItemList( false );
				}
				else
				{
					g_pInstanceData->GetItemCheatPopup().OpenItemList( true );
				}
			}
		}
#endif ITEM_CHEAT_POPUP_TEST



#ifdef SERVER_PVP_BASE_DEFENCE_TEST
		if( CX2Main::XS_PVP_ROOM == g_pMain->GetNowStateID() )
		{
			if( bHideDialog == false && g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_COMMA) == TRUE )
			{
				CX2StatePVPRoom* pState = (CX2StatePVPRoom*) g_pMain->GetNowState();

				if( true == pState->IsPVPGameOptionOpened() )
				{
					pState->OpenPVPGameOption( false );
				}
				else
				{
					pState->OpenPVPGameOption( true );
				}
			}
		}
#endif SERVER_PVP_BASE_DEFENCE_TEST


		
		// Ŀ�´�Ƽ(��ü��)
		// ����/���� �����߿��� ��ü���� ���� ���ϵ��� �Ѵ�.
		if(g_pMain->GetNowStateID() != CX2Main::XS_DUNGEON_GAME && g_pMain->GetNowStateID() != CX2Main::XS_PVP_GAME)
		{
			if ( bHideDialog == false && 
#ifdef REFORM_UI_KEYPAD
				GET_KEY_STATE( GA_COMMUNITY ) == TRUE 
#else
				g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_T) == TRUE 
#endif
				)
			{	
				if( NULL != g_pData->GetMessenger() )
				{
					g_pData->GetMessenger()->SetFriendTab(false);
				}
				g_pData->GetUIManager()->ToggleUI(CX2UIManager::UI_MENU_COMMUNITY);
				return true;
			}
		}		

#ifndef OPEN_TEST_1_NO_MESSENGER_CASHSHOP
		// Ŀ�´�Ƽ(ģ����)
#ifdef REFORM_UI_KEYPAD
		if ( bHideDialog == false && GET_KEY_STATE( GA_FRIEND ) == TRUE )
#else
		if ( bHideDialog == false && g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_M) == TRUE )
#endif
		{			
			g_pData->GetMessenger()->SetFriendTab(true);
			g_pData->GetUIManager()->ToggleUI(CX2UIManager::UI_MENU_COMMUNITY);
			return true;
		}
	

#endif OPEN_TEST_1_NO_MESSENGER_CASHSHOP


		//{{ kimhc // 2009-10-13 // ��� ��UI ����Ű ����
#ifdef	GUILD_MANAGEMENT
		// Ŀ�´�Ƽ(�����)
#ifdef REFORM_UI_KEYPAD
		if ( bHideDialog == false && GET_KEY_STATE( GA_GUILD ) == TRUE )
#else
		if ( bHideDialog == false && g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_G) == TRUE )
#endif
		{			
			g_pData->GetUIManager()->ToggleUI(CX2UIManager::UI_MENU_GUILD);
			return true;
		}
#endif	GUILD_MANAGEMENT
		//}} kimhc // 2009-10-13 // ��� ��UI ����Ű ����

#ifdef ADDED_RELATIONSHIP_SYSTEM

#ifdef REFORM_UI_KEYPAD		
		if ( bHideDialog == false && GET_KEY_STATE( GA_RELATION ) == TRUE )
#else
		if ( bHideDialog == false && g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_H) == TRUE )
#endif
		{	
			switch ( g_pMain->GetNowStateID() )
			{
			case XS_PVP_GAME:
			case XS_WEDDING_GAME:
				break;

			default:	/// pvp �Ǵ� ������ �ƴ� ��쿡�� ó�� ��
				g_pData->GetUIManager()->ToggleUI(CX2UIManager::UI_MENU_RELATION);
				return true;
				break;
			}
		}
#endif //ADDED_RELATIONSHIP_SYSTEM
		// ������Ű(�޴�)
		if ( bHideDialog == false && g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState( DIK_LWIN ) == TRUE )
		{
			//ToggleLobbyMenuWindow(); // ��ȹ�����δ� ����Ű ����
			return true;
		}		

		// ���̳��� ī�޶�
		if( g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_F9) == TRUE )
		{
			g_pMain->GetGameOption()->SetDynamicCamera( !g_pMain->GetGameOption()->GetOptionList()->m_bDynamicCamera );
			g_pMain->GetGameOption()->SaveScriptFile();
			if( pStateOption != NULL && pStateOption->GetOptionWindowOpen() == true )
			{
				pStateOption->InitOtherOption();
			}

			if( true == g_pMain->GetGameOption()->GetOptionList()->m_bDynamicCamera )
			{
				if( NULL != g_pChatBox )
				{

					g_pChatBox->AddChatLog( GET_STRING( STR_ID_354 ), KEGS_CHAT_REQ::CPT_SYSTEM, coSystemMessageTextColor, wstrSystemMessageColor );
				}
			}
			else
			{
				if( NULL != g_pChatBox )
				{

					g_pChatBox->AddChatLog( GET_STRING( STR_ID_355 ), KEGS_CHAT_REQ::CPT_SYSTEM, coSystemMessageTextColor, wstrSystemMessageColor );
				}
			}			
			return true;
		}

		// ȿ����
		if( g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_F10) == TRUE )
		{
			g_pMain->GetGameOption()->SetSound( !g_pMain->GetGameOption()->GetOptionList()->m_bSound );
			g_pMain->GetGameOption()->SaveScriptFile();
			if( pStateOption != NULL && pStateOption->GetOptionWindowOpen() == true )
			{
				pStateOption->InitOtherOption();
			}

			if( true == g_pMain->GetGameOption()->GetOptionList()->m_bSound )
			{
				if( NULL != g_pChatBox )
				{

					g_pChatBox->AddChatLog( GET_STRING( STR_ID_356 ), KEGS_CHAT_REQ::CPT_SYSTEM, coSystemMessageTextColor, wstrSystemMessageColor );
				}
			}

			else
			{
				if( NULL != g_pChatBox )
				{

					g_pChatBox->AddChatLog( GET_STRING( STR_ID_357 ), KEGS_CHAT_REQ::CPT_SYSTEM, coSystemMessageTextColor, wstrSystemMessageColor );
				}

			}						
		}

		// �����
		if( g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_F11) == TRUE )
		{
			g_pMain->GetGameOption()->SetMusic( !g_pMain->GetGameOption()->GetOptionList()->m_bMusic );
			g_pMain->GetGameOption()->SaveScriptFile();
			if( pStateOption != NULL && pStateOption->GetOptionWindowOpen() == true )
			{
				pStateOption->InitSoundOption();
			}


			if( true == g_pMain->GetGameOption()->GetOptionList()->m_bMusic )
			{
				if( NULL != g_pChatBox )
				{

					g_pChatBox->AddChatLog( GET_STRING( STR_ID_358 ), KEGS_CHAT_REQ::CPT_SYSTEM, coSystemMessageTextColor, wstrSystemMessageColor );
				}
			}
			else
			{
				if( NULL != g_pChatBox )
				{

					g_pChatBox->AddChatLog( GET_STRING( STR_ID_359 ), KEGS_CHAT_REQ::CPT_SYSTEM, coSystemMessageTextColor, wstrSystemMessageColor );
				}
			}						
			return true;
		}

#ifdef HIDE_SET_DESCRIPTION
		if( NULL != g_pData && NULL != g_pData->GetMyUser() && g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_OPERATOR )
		{
			if( g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState( DIK_NUMPAD0 ) == TRUE )
			{
				g_pData->GetMyUser()->ChangeHideSetDesc();
				return true;
			}
		}
#endif HIDE_SET_DESCRIPTION

//#ifdef SERV_HERO_PVP_MANAGE_LIST //�������� UI
//		if( NULL != g_pData->GetMyUser() )
//			if( g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_SPECIAL_USER )
//			{
//				if( g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState(DIK_HOME) == TRUE )
//				{
//					g_pData->GetUIManager()->ToggleUI(CX2UIManager::UI_MENU_HERO);
//				}
//			}
//#endif //SERV_HERO_PVP_MANAGE_LIST

	}

	return false;
}







// @wstrTextColor: ���ڿ� ��ü�� ������ �����ϴ� #Cxxxxxx �÷��ڵ�
// @bEnableColor : ���ڿ��� ���ԵǾ��ִ� #Cxxxxxx �÷��ڵ带 ������� ����
/*static*/ int CX2Main::LineBreak( wstring& wstrText, const CKTDGFontManager::CUKFont* pFont, const int iTextWidth, const wstring& wstrTextColor, const bool bEnableColor, bool bCutRedundantEmptyLine /*= true*/)
{
	KTDXPROFILE();

	if( NULL == pFont )
		return 1;

	enum CHAR_STATE
	{
		Q_NORMAL,
		Q_SHARP,
		Q_SHARP_C,
	};


	int iNowCharWidth = 0;		
	int iNowLineWidth = 0;
	int iNowLineCount = 0;
	int iNowCharIndex = 0;
	wstring wstrLineBuf = L"";
	wstring wstrColorCode = L"";

	CHAR_STATE eNowCharState = Q_NORMAL;
	bool bIsFirstCharOfLine = true; 


	if( false == wstrTextColor.empty() )
	{
		wstrLineBuf += wstrTextColor;
	}

	while( iNowCharIndex < (int) wstrText.length() )
	{
		WCHAR wszCharBuf = wstrText[iNowCharIndex];
		wstring wstrCharBuf = wstrText.substr( iNowCharIndex, 1 );

		switch( eNowCharState )
		{
		case Q_NORMAL:
			{
				if( 0 == wstrCharBuf.compare( L"#" ) )
				{
					if( true == bEnableColor )
					{
						wstrLineBuf += wstrCharBuf;
						eNowCharState = Q_SHARP;
					}
					else
					{
						wstrLineBuf += L"*"; 
						iNowCharWidth = pFont->GetWidth( L"*" );
						iNowLineWidth += iNowCharWidth;
					}
				}
				else if( 0 == wstrCharBuf.compare( L"\n" ) )
				{
					// 09.05.30 �¿� : 2�� �̻��� \n�� �������� ���� ��� �߸��� ���� ����
					if( bIsFirstCharOfLine == false || bCutRedundantEmptyLine == false )
					{
						wstrLineBuf += wstrCharBuf;
						//wstrLineBuf += L" ";
						iNowLineCount++;
						iNowLineWidth = 0;
						bIsFirstCharOfLine = true;

						if( 0 != wstrTextColor.length() )
						{
							wstrLineBuf += wstrTextColor;
						}

						if( 0 != wstrColorCode.length() )
						{
							wstrLineBuf += wstrColorCode;
						}
					}
				}
				else
				{
					if( 0 != wstrCharBuf.compare( L" " ) || false == bIsFirstCharOfLine )
					{
						if( true == bIsFirstCharOfLine )
							bIsFirstCharOfLine = false;
						wstrLineBuf += wstrCharBuf;					
						iNowCharWidth = pFont->GetWidth( wszCharBuf );
						iNowLineWidth += iNowCharWidth;
					}
				}

			} break;

		case Q_SHARP:
			{
				if( 0 == wstrCharBuf.compare( L"C" ) )
				{
					wstrLineBuf += wstrCharBuf;
					eNowCharState = Q_SHARP_C;
				}
				else
				{
					if( true == bIsFirstCharOfLine )
						bIsFirstCharOfLine = false;

					wstrLineBuf += wstrCharBuf;
					iNowCharWidth = pFont->GetWidth( wszCharBuf );
					iNowLineWidth += iNowCharWidth;

					eNowCharState = Q_NORMAL;
				}
			} break;

		case Q_SHARP_C:
			{
				if( 0 == wstrCharBuf.compare( L"X" ) )
				{
					wstrLineBuf += wstrCharBuf;
					wstrColorCode = L"";

					if( 0 != wstrTextColor.length() )
					{
						wstrLineBuf += wstrTextColor;
					}

					eNowCharState = Q_NORMAL;
				}
				else // RGB 
				{
					// note!! color code�� �ùٸ��� üũ�ϴ� �ڵ� �־�� ��. �ϴ� color code�� ������ �ùٸ������� ����
					wstrColorCode = L"#C";
					wstrColorCode += wstrText.substr( iNowCharIndex, 6 );
					wstrLineBuf += wstrText.substr( iNowCharIndex, 6 );
					iNowCharIndex += 5;
					eNowCharState = Q_NORMAL;
				}

			} break;
		}

		if( iNowLineWidth >= iTextWidth && Q_NORMAL == eNowCharState )
		{
			wstrLineBuf += L"\n";
			//wstrLineBuf += L" ";
			iNowLineCount++;
			iNowLineWidth = 0;
			bIsFirstCharOfLine = true;

			if( 0 != wstrTextColor.length() )
			{
				wstrLineBuf += wstrTextColor;
			}

			if( 0 != wstrColorCode.length() )
			{
				wstrLineBuf += wstrColorCode;
			}
		}

		iNowCharIndex++;
	}


	wstrText = wstrLineBuf;

	return iNowLineCount;
}

/*static*/ bool CX2Main::IsInvalidNickName( const wstring& wstrNickName, wstring& wstrErrorMessage )
{
	if( true == wstrNickName.empty() )
	{
		wstrErrorMessage = GET_STRING( STR_ID_360 );
		return true;
	}

	string strNickName = "";
	ConvertWCHARToChar( strNickName, wstrNickName.c_str() );

	if ( (int)strNickName.size() < 1 )
	{
		wstrErrorMessage = GET_STRING( STR_ID_360 );
		return true;
	}
	else if ( (int)strNickName.size() > 12  )
	{
		wstrErrorMessage = GET_STRING( STR_ID_361 );
		return true;
	}
	else if( NULL != g_pData->GetMyUser() && 
		g_pData->GetMyUser()->GetAuthLevel() < CX2User::XUAL_OPERATOR && 
		false == g_pMain->GetStringFilter()->CheckIsValidString( CX2StringFilter::FT_NICKNAME, wstrNickName.c_str() ) )
	{
		wstrErrorMessage = GET_STRING( STR_ID_362 );
		return true;
	}


	return false;
}


//{{ 2009. 7. 26  ������	party fever cheat
void CX2Main::IncreasePartyFeverPoint( int iCount )
{
#ifdef LIGHT_OPERATOR_ACCOUNT
	if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetAuthLevel() == CX2User::XUAL_LIGHT_OPERATOR )
	{
		return;
	}
#endif LIGHT_OPERATOR_ACCOUNT

	for( int i = 0; i < iCount; ++i )
	{
		g_pData->GetServerProtocol()->SendID( EGS_ADMIN_INCREASE_PARTY_FEVER_POINT_NOT );
	}
}
//}}

//{{ 2009. 8. 5  ������		ü��IDġƮ
void CX2Main::SetGuestUserMode( bool bVal )
{
#ifdef LIGHT_OPERATOR_ACCOUNT
	if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetAuthLevel() == CX2User::XUAL_LIGHT_OPERATOR )
	{
		return;
	}
#endif LIGHT_OPERATOR_ACCOUNT

	g_pData->GetServerProtocol()->SendPacket( EGS_ADMIN_GUEST_USER_MODE_NOT, bVal );
}
//}}

//{{ ����� : [2010/3/12/] //	���� �̵� ���� ġƮ
void CX2Main::WarpByCommand( int iPosition )
{
	int iWorldID;

	switch( iPosition )
	{
	case 0: iWorldID = SEnum::VMI_RUBEN;						break;
	case 1: iWorldID = SEnum::VMI_ELDER;						break;
	case 2: iWorldID = SEnum::VMI_BATTLE_FIELD_ELDER_REST_00;	break;
	case 3: iWorldID = SEnum::VMI_BESMA;						break;
	case 4: iWorldID = SEnum::VMI_BATTLE_FIELD_BESMA_REST_00;	break;
	case 5: iWorldID = SEnum::VMI_ALTERA;						break;
	case 6: iWorldID = SEnum::VMI_BATTLE_FIELD_ALTERA_REST_00;	break;
	case 7: iWorldID = SEnum::VMI_PEITA;						break;
	case 8: iWorldID = SEnum::VMI_BATTLE_FIELD_PEITA_REST_00;	break;
	case 9: iWorldID = SEnum::VMI_VELDER;						break; // ����
	case 10: iWorldID = SEnum::VMI_BATTLE_FIELD_VELDER_REST_00;	break; // ���� �޽�ó
	case 11: iWorldID = SEnum::VMI_HAMEL;						break; // �ϸ� 
	case 12: iWorldID = SEnum::VMI_BATTLE_FIELD_HAMEL_REST_00;	break; // �ϸ� �޽�ó
	case 13: iWorldID = SEnum::VMI_SANDER;						break; // ����
	case 14: iWorldID = SEnum::VMI_BATTLE_FIELD_SANDER_REST_00; break; // ���� �޽�ó
	default: iWorldID = -1;							break;
	}

	//	�ŷ����忡���� ���� �ȵ�
	if( iWorldID != -1 && g_pMain->GetNowStateID() != CX2Main::XS_SQUARE_GAME )
	{
		g_pData->SetStateArg( iWorldID );
		CX2State* pState = (CX2State*) g_pMain->GetNowState();
		pState->Handler_EGS_STATE_CHANGE_FIELD_REQ( iWorldID );
	}
}
//}} ����� : [2010/3/12/] //	

#ifdef BATTLE_FIELD_WARP_EDIT
void CX2Main::FieldWarpByCommand( int iPosition )
{
	int iWorldID = SEnum::VMI_INVALID;
	switch( iPosition )
	{
	case 1: iWorldID = SEnum::VMI_BATTLE_FIELD_RUBEN_FIELD_01;			break; //�纥 �ʵ�1

	case 2: iWorldID = SEnum::VMI_BATTLE_FIELD_ELDER_FIELD_01;			break; //���� �ʵ�1
	case 3: iWorldID = SEnum::VMI_BATTLE_FIELD_ELDER_FIELD_02;			break; //���� �ʵ�2

	case 4: iWorldID = SEnum::VMI_BATTLE_FIELD_BESMA_FIELD_01;			break; //������ �ʵ�1
	case 5: iWorldID = SEnum::VMI_BATTLE_FIELD_BESMA_FIELD_02;			break; //������ �ʵ�2
	case 6: iWorldID = SEnum::VMI_BATTLE_FIELD_BESMA_FIELD_03;			break; //������ �ʵ�3

	case 7: iWorldID = SEnum::VMI_BATTLE_FIELD_ALTERA_FIELD_01;			break; //���׶� �ʵ�1
	case 8: iWorldID = SEnum::VMI_BATTLE_FIELD_ALTERA_FIELD_02;			break; //���׶� �ʵ�2
	case 9: iWorldID = SEnum::VMI_BATTLE_FIELD_ALTERA_FIELD_03;			break; //���׶� �ʵ�3

	case 10: iWorldID = SEnum::VMI_BATTLE_FIELD_PEITA_FIELD_01;			break; //����Ÿ �ʵ�1
	case 11: iWorldID = SEnum::VMI_BATTLE_FIELD_PEITA_FIELD_02;			break; //����Ÿ �ʵ�2
	case 12: iWorldID = SEnum::VMI_BATTLE_FIELD_PEITA_FIELD_03;			break; //����Ÿ �ʵ�3

	case 13: iWorldID = SEnum::VMI_BATTLE_FIELD_VELDER_FIELD_01;		break; //���� �ʵ�1
	case 14: iWorldID = SEnum::VMI_BATTLE_FIELD_VELDER_FIELD_02;		break; //���� �ʵ�2
	case 15: iWorldID = SEnum::VMI_BATTLE_FIELD_VELDER_FIELD_03;		break; //���� �ʵ�3
	case 16: iWorldID = SEnum::VMI_BATTLE_FIELD_VELDER_FIELD_04;		break; //���� �ʵ�4
	case 17: iWorldID = SEnum::VMI_BATTLE_FIELD_VELDER_SHIP_STAGE;		break; // ���� ��

	case 18: iWorldID = SEnum::VMI_BATTLE_FIELD_HAMEL_SHIP_STAGE;		break; // �ϸ� ��
	case 19: iWorldID = SEnum::VMI_BATTLE_FIELD_HAMEL_FIELD_01;			break; //�ϸ� �ʵ�1
	case 20: iWorldID = SEnum::VMI_BATTLE_FIELD_HAMEL_FIELD_02;			break; //�ϸ� �ʵ�2
	case 21: iWorldID = SEnum::VMI_BATTLE_FIELD_HAMEL_FIELD_03;			break; //�ϸ� �ʵ�3
	case 22: iWorldID = SEnum::VMI_BATTLE_FIELD_HAMEL_FIELD_04;			break; //�ϸ� �ʵ�4

	case 23: iWorldID = SEnum::VMI_BATTLE_FIELD_SANDER_FIELD_01;		break; //���� �ٶ��� ���
	case 24: iWorldID = SEnum::VMI_BATTLE_FIELD_SANDER_FIELD_02;		break; //���� �ʵ�2

	default: iWorldID = SEnum::VMI_INVALID;								break;
	}

	CX2State* pState = (CX2State*) g_pMain->GetNowState();
	pState->Handler_EGS_JOIN_BATTLE_FIELD_REQ(iWorldID, 1 );
	return;
}
#endif //BATTLE_FIELD_WARP_EDIT

#ifdef MAKE_SCREENSHOT
void CX2Main::MakeHackScreenShotTemp( const char *pFileName )
{
	try
	{
		HBITMAP hBitmap;
		RECT rect;
		HDC hMemDC;
		HDC hScrDC;
		HBITMAP hOldBitmap;

		//DC�� ��´�.

		HWND pDesktop = GetDesktopWindow();

		// ��Ŀ���� ���� �������� ��ǥ�� �����Ѵ�.      
		GetClientRect( pDesktop, &rect );


		// ������ ȭ�� DC�� �޸� DC�� ����� �޸� ��Ʈ���� �����Ѵ�.
		hScrDC = GetDC( pDesktop ); CreateDC( L"DISPLAY", NULL, NULL, NULL );
		hMemDC = CreateCompatibleDC( hScrDC );
		hBitmap = CreateCompatibleBitmap( hScrDC, rect.right - rect.left, rect.bottom - rect.top );
		hOldBitmap = (HBITMAP)SelectObject( hMemDC, hBitmap );

		// ȭ���� �޸� ��Ʈ������ �����Ѵ�.
		BitBlt( hMemDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
			hScrDC, rect.left, rect.top, SRCCOPY );
		SelectObject( hMemDC, hOldBitmap );

		CxImage cimage( CXIMAGE_FORMAT_JPG );

		if( cimage.CreateFromHBITMAP( hBitmap ) )
		{
			cimage.SetJpegQuality( 75 );
			cimage.Save( pFileName, CXIMAGE_FORMAT_JPG );
		}

		DeleteDC( hMemDC );
		DeleteDC( hScrDC );
		DeleteObject( hBitmap );
		DeleteObject( hOldBitmap );
	}
	catch (...)
	{


	}
}
#endif
void CX2Main::MakeHackScreenShot(const char *pFileName)
{
	try
	{
#ifdef MAKE_SCREENSHOT
		// �⺻ ���� ���� ���� ������ default ���� //
		HRESULT hr = BCERR_FILE_CREATE; 

		// ĸó�� �����մϴ�.
		if(m_BandiCaptureLibrary.IsCapturing()==FALSE)
		{
			// ���� ����� DLL �� �ε��ϰ� ���� �ν��Ͻ��� �����մϴ�. 
			if(m_BandiCaptureLibrary.IsCreated()==FALSE)
			{
				// ���н� DLL�� .H������ ������ �������� ���ε��� Ȯ���� ���ñ� �ٶ��ϴ�.
				if(FAILED(m_BandiCaptureLibrary.Create(BANDICAP_RELEASE_DLL_FILE_NAME)))
				{
					MakeHackScreenShotTemp(pFileName);
					return;
				}

				// ������ ���� ������ ���ڵ��� ������ ��ܿ� �ΰ��� ǥ�õ˴ϴ�. 
				// ���ı����� �߱޹��� Ű�� ����ϰų�, Ȩ���������� ��������� ������ Ű�� ����ؼ� ������ �޽��ϴ�. 
				if(FAILED(m_BandiCaptureLibrary.Verify("KOG_ELSWORD_20110825", "675a5521")))
				{
					MakeHackScreenShotTemp(pFileName);
					return;
				}
			}

			if(m_BandiCaptureLibrary.IsCreated())
			{
				//TCHAR pathName[MAX_PATH];
				//m_BandiCaptureLibrary.MakePathnameByDate(_T("ScreenCapture\\"), _T("SC_"), _T("jpg"), pathName, MAX_PATH);
				HWND pDesktop = GetDesktopWindow();
				hr = m_BandiCaptureLibrary.CaptureImage(pFileName, BCAP_IMAGE_JPG , 50, BCAP_MODE_GDI, true,(LONG_PTR)pDesktop);

				if(FAILED(hr))
				{
					MakeHackScreenShotTemp(pFileName);
					return; 
				}
			}
		}
#else
		HBITMAP hBitmap;
		RECT rect;
		HDC hMemDC;
		HDC hScrDC;
		HBITMAP hOldBitmap;

		//DC�� ��´�.

		HWND pDesktop = GetDesktopWindow();

		// ��Ŀ���� ���� �������� ��ǥ�� �����Ѵ�.      
		GetClientRect( pDesktop, &rect );


		// ������ ȭ�� DC�� �޸� DC�� ����� �޸� ��Ʈ���� �����Ѵ�.
		hScrDC = GetDC( pDesktop ); CreateDC( L"DISPLAY", NULL, NULL, NULL );
		hMemDC = CreateCompatibleDC( hScrDC );
		hBitmap = CreateCompatibleBitmap( hScrDC, rect.right - rect.left, rect.bottom - rect.top );
		hOldBitmap = (HBITMAP)SelectObject( hMemDC, hBitmap );

		// ȭ���� �޸� ��Ʈ������ �����Ѵ�.
		BitBlt( hMemDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
			hScrDC, rect.left, rect.top, SRCCOPY );
		SelectObject( hMemDC, hOldBitmap );

		CxImage cimage( CXIMAGE_FORMAT_JPG );

		if( cimage.CreateFromHBITMAP( hBitmap ) )
		{
			cimage.SetJpegQuality( 75 );
			cimage.Save( pFileName, CXIMAGE_FORMAT_JPG );
		}

		DeleteDC( hMemDC );
		DeleteDC( hScrDC );
		DeleteObject( hBitmap );
		DeleteObject( hOldBitmap );
#endif
	}
	catch (...)
	{


	}
}

#ifdef SERV_SKILL_NOTE
CKTDGUIDialogType CX2Main::KTDGUIInfoBox( D3DXVECTOR2 vPos, const WCHAR* pText, CKTDXStage* pNowState, float fLifeTime )
{
	if( pNowState == NULL )
		pNowState = m_pNowState;


	DialogLog( pText );

	if( m_pDlgInfoBox != NULL )
	{
		g_pKTDXApp->GetDGManager()->GetDialogManager()->DeleteDlg( m_pDlgInfoBox );
		m_fInfoBoxLifeTime = -1.f;
		m_pDlgInfoBox = NULL;
	}

	m_pDlgInfoBox = new CKTDGUIDialog( pNowState, L"DLG_Common_Info.lua", 0.07f, XDL_POP_UP );
	m_pDlgInfoBox->SetColor( D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f) );	
	m_pDlgInfoBox->SetPos( vPos );
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDlgInfoBox );
	m_fInfoBoxLifeTime = fLifeTime;

	CKTDGUIStatic* pStatic = (CKTDGUIStatic*)m_pDlgInfoBox->GetControl( L"Static_Item_Name1" );
	CKTDGUIStatic* pStaticInfo = (CKTDGUIStatic*)m_pDlgInfoBox->GetControl( L"InfoBox" );
	
	
	CKTDGUIControl::CPictureData *pPicture = pStaticInfo->GetPictureIndex(1);
    
	wstring tempText = pText;

	const int constTextMaxLen = (int)(g_pKTDXApp->GetResolutionScaleX() * 390);


	//bool checkEnterChar = false;
	int nowTextLen = 0;

	//	CKTDGFontManager::CKTDGFont* pFont = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetFont( pStatic->GetString(0)->fontIndex );
	//CKTDGUIUniBuffer uniBuffer;
	//uniBuffer.SetFontNode( pFont );

	CKTDGFontManager::CUKFont* pFont = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetUKFont( pStatic->GetString(0)->fontIndex );
#ifdef CLIENT_GLOBAL_LINEBREAK
	int lineNum = CWordLineHandler::LineBreakInX2MainMsgBox( tempText, pFont, constTextMaxLen );
#else //CLIENT_GLOBAL_LINEBREAK

	int lineNum = 1;

	for ( int i = 0; i < (int)tempText.size(); i++ )
	{
		WCHAR tempWchar = tempText[i];
		if ( nowTextLen < constTextMaxLen && tempWchar == '\n' )
		{
			if ( i != (int)tempText.size() - 1 )
			{
				lineNum++;
			}

			nowTextLen = 0;
			continue;
		}
		int iUniCharSize = 0;
		//uniBuffer.Clear();
		//uniBuffer.InsertChar( 0, tempWchar );
		//uniBuffer.CPtoX(uniBuffer.GetTextSize(), FALSE, &iUniCharSize);

		iUniCharSize = pFont->GetWidth( tempWchar );

		nowTextLen += iUniCharSize;

		if ( nowTextLen >= constTextMaxLen )
		{
			if ( i != (int)tempText.size() - 1 )
			{
				lineNum++;
			}

			nowTextLen = 0;
			wstring enterChar = L"\n";
			tempText.insert( tempText.begin() + i, enterChar.begin(), enterChar.end() );
			i++;


		}

		if ( i >= (int)tempText.size() )
		{
			break;
		}
	}
#endif //CLIENT_GLOBAL_LINEBREAK
	if ( lineNum == 2 )
	{
		pStatic->GetString(0)->pos.y -= 12.f;
		pStatic->GetString(1)->pos.y -= 12.f;
	}
	else if ( lineNum == 3 )
	{
		pStatic->GetString(0)->pos.y -= pFont->GetHeight() + 3.f;
		pStatic->GetString(1)->pos.y -= pFont->GetHeight() + 3.f;
	}

	pStatic->GetString(0)->msg = tempText;
	pStatic->GetString(1)->msg = tempText;

	if( pPicture != NULL )
	{
		D3DXVECTOR2 vInfoPos = pPicture->GetPos();
// �̱� ��Ʈ�� Ư���� ������ -9 �մϴ�.
#ifdef CLIENT_COUNTRY_US
		vInfoPos.y = pStatic->GetString(0)->pos.y - 9;
#else
		vInfoPos.y = pStatic->GetString(0)->pos.y;
#endif CLIENT_COUNTRY_US
		pPicture->SetPos(vInfoPos);
	}

	return m_pDlgInfoBox;
}

void CX2Main::DeleteInfoBox()
{
	if( m_pDlgInfoBox != NULL )
	{
		g_pKTDXApp->GetDGManager()->GetDialogManager()->DeleteDlg( m_pDlgInfoBox );
	}
	m_fInfoBoxLifeTime = -1.f;
	m_pDlgInfoBox = NULL;
}

#endif



#ifdef REFACTORING_BY_TOOL_TEAM


void CX2Main::ExitClientIfHackingFound( const float fElapsedTime )
{

#if defined( _SERVICE_ ) 
	ELSWORD_VIRTUALIZER_START
#endif

	if ( g_pKTDXApp->GetFindHacking() == true )
	{
		THEMIDA_VM_START

		static bool staticExitCheck = false;
		if ( staticExitCheck == false )
		{

			staticExitCheck = true;
			/*
			char buff[256] = {0};
			string cmdString;
			ConvertWCHARToChar( cmdString, g_pKTDXApp->GetFindHackingDetailMsg() );
			sprintf( buff, "FindH.dll %s", cmdString.c_str() );
			WinExec( buff, SW_HIDE );
			*/


			//g_pKTDXApp->NoticeQuitType( CKTDXApp::KQT_FIND_HACKING );

			m_bAutoQuit = true;


			// NOTE: Ŭ���̾�Ʈ�� ����Ǳ� ���� �ð��� ���� �� ��� �����մϴ�
			m_fAutoQuitCoolTime = RandomFloat( 20.f, 30.f );
			

		}

		THEMIDA_VM_END
	}

	if ( m_bAutoQuit == true )
	{
		THEMIDA_VM_START 

		m_fAutoQuitTime += fElapsedTime;
#ifdef ADD_COLLECT_CLIENT_INFO
		if ( m_fAutoQuitTime >= m_fAutoQuitCoolTime && GetSendHackPacket() == false )		
#else
		if ( m_fAutoQuitTime >= m_fAutoQuitCoolTime )
#endif
		{
			g_pMain->SetQuitType( NetError::ERR_CLIENT_QUIT_01 );
			PostQuitMessage(0);
		}

		THEMIDA_VM_END
	}

#if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_END
#endif
}


#endif REFACTORING_BY_TOOL_TEAM







#if defined( SERV_HACKING_TOOL_LIST )

#ifdef REFACTORING_BY_TOOL_TEAM

void CX2Main::CreateCheckWindowInfoThreadIfNotExist()
{
	if( NULL != m_threadCheckWindowInfo )
	{
		if( NULL != m_threadCheckWindowInfo->GetThreadHandle() )
		{
			return; 
		}
		else
		{
			SAFE_DELETE( m_threadCheckWindowInfo );
		}
	}


#ifdef THEMIDA_BY_TOOL_TEAM
	int hackingCheckVariable; 
#endif THEMIDA_BY_TOOL_TEAM


	//THEMIDA_VM_START

	m_threadCheckWindowInfo = new CheckWindowInfo();
	m_threadCheckWindowInfo->m_pKTDXApp			= g_pKTDXApp;
	m_threadCheckWindowInfo->m_pInstanceData	= g_pInstanceData;
#ifdef	CHECK_KOM_FILE_ON_LOAD
	m_threadCheckWindowInfo->m_pMain			= this;
#endif	CHECK_KOM_FILE_ON_LOAD
	m_threadCheckWindowInfo->BeginThread();

	THEMIDA_CHECK_PROTECTION( hackingCheckVariable, 0x164dc081 ) 


#ifndef	CHECK_KOM_FILE_ON_LOAD
		if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
			g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
		{
#ifdef ADD_COLLECT_CLIENT_INFO			
			g_pMain->SendHackInfo2( ANTIHACKING_ID::ANTIHACKING_GAME_15, "", false, false );
#else //ADD_COLLECT_CLIENT_INFO
			g_pMain->SendHackMail_DamageHistory( ANTI_HACK_STRING_UserID_AntiHacking_CheckWindowInfo_Fail );
#endif //ADD_COLLECT_CLIENT_INFO
			g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );		
		}
#endif	CHECK_KOM_FILE_ON_LOAD

	THEMIDA_REPORT_IF_TAMPERED( hackingCheckVariable, 0x164dc081, ANTI_HACK_WSTRING_UserID_AntiHacking_CheckWindowInfo_Fail )



	//THEMIDA_VM_END
}




void DebuggerChecker::OnFrameMove( const float elapsedTime )
{
	const float MAGIC_TIME_INTERVAL_FOR_CHECKING_DEBUGGER = 20.f;

	m_ElapsedTime += elapsedTime;
	if( m_ElapsedTime < MAGIC_TIME_INTERVAL_FOR_CHECKING_DEBUGGER )
		return; 

//#ifdef THEMIDA_BY_TOOL_TEAM
//	int hackingCheckVariable; 
//#endif THEMIDA_BY_TOOL_TEAM

	//THEMIDA_VM_START

	BOOL isDebuggerPresent = Pesudo_IsDebuggerPresent(); //|| CheckDebugger() )

	//THEMIDA_CHECK_CODE_INTEGRITY( hackingCheckVariable, 0xbe8b9d91 )  


	if( TRUE == isDebuggerPresent )
	{
		if( g_pData != NULL && 
			g_pData->GetServerProtocol() != NULL &&
			g_pKTDXApp->GetFindHacking() == false )
		{
			g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );

			if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL )
				g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
		}

		if( NULL != g_pMain )
		{
#ifdef ADD_COLLECT_CLIENT_INFO			
			g_pMain->SendHackInfo3( ANTIHACKING_ID::ANTIHACKING_GAME_14, "", true, false );
#else //ADD_COLLECT_CLIENT_INFO
			g_pMain->SendHackMail_DamageHistory( ANTI_HACK_STRING_AntiHacking_IsDebuggerPresent );
#endif //ADD_COLLECT_CLIENT_INFO
			g_pMain->BrokenCode();
		}

#ifndef ADD_COLLECT_CLIENT_INFO
		if( NULL != g_pKTDXApp )
		{
			g_pKTDXApp->SetFindHacking( true );
		}
#endif //ADD_COLLECT_CLIENT_INFO
	}

	//THEMIDA_REPORT_IF_TAMPERED( hackingCheckVariable, 0xbe8b9d91, ANTI_HACK_WSTRING_BYPASS_DEBUGGER_CHECK )

	m_ElapsedTime = 0.f;

	//THEMIDA_VM_END
}
#endif REFACTORING_BY_TOOL_TEAM







//{{ �ڱ��� : [2010/03/24]	// ���μ��� üũ ��ȭ
struct PROCESS_INFO
{
	wstring			ProcessName;
	DWORD			ProcessID;
};
//}}


#ifdef ENUM_CHILD_WINDOW
BOOL CALLBACK EnumChildProc(HWND hwnd , LPARAM lp)
{
	g_pMain->GetCheckWindowInfo()->SetChildWindow( true );
	return FALSE;
}
#endif



bool CX2Main::CheckWindowInfo::CheckProcess()
{	
#if defined( _SERVICE_ ) 
	ELSWORD_VIRTUALIZER_START
#endif

//  #ifndef _SERVICE_
//  	if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_OPERATOR )
//  		return false;
//  #endif

#ifdef ENUM_CHILD_WINDOW
	if( g_pKTDXApp != NULL && g_pKTDXApp->GetHWND() != NULL )		
	{
		m_bFindChild = false;
		EnumChildWindows( g_pKTDXApp->GetHWND(), EnumChildProc, NULL );
		if( m_bFindChild == true )
		{
			// �ڽ� �����찡 �����Ѵ�.
			m_bSendedMail = true;
			if( m_bSendedMail == false )
			{
#ifdef ADD_COLLECT_CLIENT_INFO			
				g_pMain->SendHackInfo4( ANTIHACKING_ID::ANTIHACKING_GAME_16, "", false, true );
#else
				g_pMain->SendHackMail_DamageHistory(ANTI_HACK_STRING_AntiHacking_CHILD_WINDOW);
#endif
			}
		}
	}	
#endif



#ifdef CHECK_THREAD_WND

#ifdef DESK_THREAD_WINDOW
	static float fCheckTime = 0.f;
	DWORD ThreadId = GetWindowThreadProcessId(NULL,NULL); 
	//vector<THREAD_WND_INFO> vecDeskThreadInfo = GetThreadWnd(ThreadId);
	
	fCheckTime += g_pKTDXApp->GetElapsedTime();
	if( fCheckTime >= 3.f * 60.f)
	{
		fCheckTime = 0.f;
		m_vecDeskThreadInfo.clear();
	}

	if( m_vecDeskThreadInfo.size() == 0 )
	{
		m_vecDeskThreadInfo = GetThreadWnd(ThreadId);

#ifdef CHECK_ALL_WINDOW
		m_vecTopWndInfo.clear();
		{			
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
					threadWndInfo.m_wstrName = wTextWindow;
				else
					threadWndInfo.m_wstrName = L"NameEmpty";

				if( hr2 > 0 )
					threadWndInfo.m_wstrClass = wTextClass;
				else
					threadWndInfo.m_wstrClass = L"ClassEmpty";

				if( hr1 > 0 )
				{
					if( strWndText.compare( "MSCTFIME UI") != 0 && strWndText.compare( "Default IME") != 0 )
					{
						m_vecTopWndInfo.push_back(threadWndInfo);
					}
				}

				hWnd = ::GetNextWindow(hWnd, GW_HWNDNEXT);
			}

			if( m_vecDeskThreadInfo.size() == 0 || m_vecTopWndInfo.size() == 0 )
			{
#ifdef ADD_COLLECT_CLIENT_INFO // ���� ������ �ؿ��� �߰�
				g_pMain->SendHackInfo3( ANTIHACKING_ID::ANTIHACKING_GAME_57, "", false, true );
#endif // ADD_COLLECT_CLIENT_INFO
			}
		}
		
#endif

		// x2 ���μ����� Ư�� �����콺���� ���� (formmain window�� ������ ���)
		if( g_pKTDXApp->GetHWND() != NULL )
		{
			DWORD X2ThreadId = GetWindowThreadProcessId(g_pKTDXApp->GetHWND(),NULL);  

			vector<THREAD_WND_INFO> vecThreadWndInfo = GetThreadWnd(X2ThreadId);
			for( UINT itw=0; itw<vecThreadWndInfo.size(); ++itw )
			{
				THREAD_WND_INFO twi = vecThreadWndInfo[itw];

				char *strDeFilter = NULL;
				wstring wstrtwname = L"";
				char strNewFilter1[100] = { 68, -59, -118, -85, -111, -54, 46, 72, 0, };
				strDeFilter = XORDecrypt( strNewFilter1, strlen(strNewFilter1) );	
				ConvertCharToWCHAR( wstrtwname, strDeFilter );

				if( twi.m_wstrName.compare(wstrtwname) == 0 )
				{
#ifdef ADD_COLLECT_CLIENT_INFO // ���� ������ �ؿ��� �߰�
					g_pMain->SendHackInfo5( ANTIHACKING_ID::ANTIHACKING_GAME_52, "", false, true );
#endif // ADD_COLLECT_CLIENT_INFO
				}
				SAFE_DELETE_ARRAY( strDeFilter );
			}
		}
	}
#endif //DESK_THREAD_WINDOW

#endif

#ifdef CHECK_FOREGROUNDWND
	HWND hWndForeGround = GetForegroundWindow();
	if( hWndForeGround != NULL && g_pKTDXApp->GetHWND() != hWndForeGround && 
		GetActiveWindow() == g_pKTDXApp->GetHWND() )
	{
		WCHAR wTextWindow[_MAX_PATH] = L"";
		int hr1 = GetWindowText(hWndForeGround, wTextWindow, _MAX_PATH);

		if( hr1 > 0 )
		{
			string strForeGroundWndName = "";
			ConvertWCHARToChar(strForeGroundWndName, wTextWindow);
#ifdef ADD_COLLECT_CLIENT_INFO // ���� ������ �ؿ��� �߰�
			g_pMain->SendHackInfo1( ANTIHACKING_ID::ANTIHACKING_GAME_54, strForeGroundWndName.c_str(), false, true );
#endif // ADD_COLLECT_CLIENT_INFO
		}
		else
		{
#ifdef ADD_COLLECT_CLIENT_INFO // ���� ������ �ؿ��� �߰�
			g_pMain->SendHackInfo2( ANTIHACKING_ID::ANTIHACKING_GAME_54, "", false, true );
#endif // ADD_COLLECT_CLIENT_INFO
		}		
	}
#endif

	if( m_bForceExitThread == true )
		return true;

#ifdef PROCESSLIST
#ifdef FIND_ZOMBIE_PROCESS
	std::vector<PROCESS_INFO> vecZombieProcessInfo;
#endif
	std::vector<PROCESS_INFO> vecProcessInfo;
	PROCESS_INFO tempProcessInfo;
	DWORD aProcesses[1024], cbNeeded;
	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
		return false;
	DWORD cProcesses = cbNeeded / sizeof(DWORD);

	DWORD PidFor;
	for ( PidFor = 0x0c; PidFor < 0xFFFF; PidFor +=4 )
	{
		HANDLE hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION |
			PROCESS_VM_READ,
			FALSE, PidFor );

		if (hProcess)
		{
			BOOL sHide = false;
			unsigned int i;
			WCHAR szName[MAX_PATH]= L"<Unknown>";
			HMODULE hModule;

			for ( i = 0; i < cProcesses; ++i )
			{
				if (PidFor == aProcesses[i])
				{
					sHide = true;
					break;
				}

			}

			if(EnumProcessModules(hProcess,&hModule,sizeof(hModule),&cbNeeded))
			{
				GetModuleFileNameEx(hProcess,hModule,szName,ARRAY_SIZE(szName));

				tempProcessInfo.ProcessID = PidFor;
				tempProcessInfo.ProcessName = szName;
				MakeUpperCase( tempProcessInfo.ProcessName );

				// ���� ���μ������ �߰����� �ʴ´�
				if(tempProcessInfo.ProcessID != GetCurrentProcessId())
				{
#ifdef CONVERSION_VS
					// Find (_1.ProcessID == tempProcessInfo.ProcessID)
					std::vector<PROCESS_INFO>::iterator it = find_if(vecProcessInfo.begin(), vecProcessInfo.end(), 
						boost::bind(std::equal_to<DWORD>(), boost::bind(&PROCESS_INFO::ProcessID, _1), tempProcessInfo.ProcessID));
#else CONVERSION_VS
					// Find (_1.ProcessID == tempProcessInfo.ProcessID)
					std::vector<PROCESS_INFO>::iterator it = find_if(vecProcessInfo.begin(), vecProcessInfo.end(), 
						bind(std::equal_to<DWORD>(), bind(&PROCESS_INFO::ProcessID, _1), tempProcessInfo.ProcessID));
#endif CONVERSION_VS
					if ( it == vecProcessInfo.end() )
						vecProcessInfo.push_back( tempProcessInfo );
				}
			}
			else
			{				
				GetProcessImageFileName(hProcess,szName,ARRAY_SIZE(szName));
				tempProcessInfo.ProcessID = PidFor;
				tempProcessInfo.ProcessName = szName;
				MakeUpperCase( tempProcessInfo.ProcessName );

				// ���� ���μ������ �߰����� �ʴ´�
				if(tempProcessInfo.ProcessID != GetCurrentProcessId())
				{
#ifdef CONVERSION_VS
					// Find (_1.ProcessID == tempProcessInfo.ProcessID)
					std::vector<PROCESS_INFO>::iterator it = find_if(vecZombieProcessInfo.begin(), vecZombieProcessInfo.end(), 
						boost::bind(std::equal_to<DWORD>(), boost::bind(&PROCESS_INFO::ProcessID, _1), tempProcessInfo.ProcessID));
#else CONVERSION_VS
					// Find (_1.ProcessID == tempProcessInfo.ProcessID)
					std::vector<PROCESS_INFO>::iterator it = find_if(vecZombieProcessInfo.begin(), vecZombieProcessInfo.end(), 
						bind(std::equal_to<DWORD>(), bind(&PROCESS_INFO::ProcessID, _1), tempProcessInfo.ProcessID));
#endif CONVERSION_VS
					if ( it == vecZombieProcessInfo.end() )
						vecZombieProcessInfo.push_back( tempProcessInfo );
				}
			}

		}

		CloseHandle( hProcess );

		if( m_bForceExitThread == true )
			return true;
	}
#else PROCESSLIST
	HANDLE hSnapShot;
	std::vector<PROCESS_INFO> vecProcessInfo;
	PROCESS_INFO tempProcessInfo;

	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL,NULL);

	// �������� ���μ������� ù���� ������ �����´�. 
	PROCESSENTRY32W pEntry;
	pEntry.dwSize =sizeof(pEntry);
	BOOL hRes = Process32First( hSnapShot, &pEntry );

	while(hRes)
	{
		tempProcessInfo.ProcessID = pEntry.th32ProcessID;
		tempProcessInfo.ProcessName = pEntry.szExeFile;
		MakeUpperCase( tempProcessInfo.ProcessName );

		// ���� ���μ������ �߰����� �ʴ´�
		if(tempProcessInfo.ProcessID != GetCurrentProcessId())
		{
			// Find (_1.ProcessID == tempProcessInfo.ProcessID)
			std::vector<PROCESS_INFO>::iterator it = find_if(vecProcessInfo.begin(), vecProcessInfo.end(), 
				bind(std::equal_to<DWORD>(), bind(&PROCESS_INFO::ProcessID, _1), tempProcessInfo.ProcessID));
			if ( it == vecProcessInfo.end() )
				vecProcessInfo.push_back( tempProcessInfo );
		}

		//������ ���μ����� ������ �����´�.
		hRes = Process32Next( hSnapShot, &pEntry );
	}
#endif PROCESSLIST

#ifdef CODE_FILTER
	wstring wstrDeFilter = L"";
	char *strDeFilter = NULL;
	KHackingToolInfo khti;

	char strNewFilter1[100] = { 70, -17, -76, -106, -108, -30, 116, 20, -63, -2, 88, -35, 0, };
	strDeFilter = XORDecrypt( strNewFilter1, strlen(strNewFilter1) );	
	ConvertCharToWCHAR( wstrDeFilter, strDeFilter );
	khti.m_cFlag = 1;
	khti.m_wstrProcessName = wstrDeFilter;
	g_pInstanceData->PushHackList( khti );
	SAFE_DELETE_ARRAY( strDeFilter );

	char strNewFilter2[100] = { 76, -6, -68, -118, -112, -123, 35, 74, -125, 0, };
	strDeFilter = XORDecrypt( strNewFilter2, strlen(strNewFilter2) );	
	ConvertCharToWCHAR( wstrDeFilter, strDeFilter );
	khti.m_cFlag = 7;
	khti.m_wstrProcessName = wstrDeFilter;
	g_pInstanceData->PushHackList( khti );
	SAFE_DELETE_ARRAY( strDeFilter );
#endif

	bool bFind = false;
	ClearWindowInfo();
	EnumWindows( (WNDENUMPROC)EnumWindowsProc, (LPARAM)NULL );

	//// Module
	//X2AntiHackHelper::ModuleVector vecModuleName;
	//X2AntiHackHelper::EnumProcessModules(vecModuleName);

	WCHAR tempModuleName[MAX_PATH] = L"";	

	if( m_pInstanceData != NULL && m_pKTDXApp != NULL )
	{
		if( m_pInstanceData->GetHackListSize() > 0 )
		{
			

			for(int i=0; i<m_pInstanceData->GetHackListSize(); ++i)
			{
				KHackingToolInfo hackInfo = m_pInstanceData->GetHackInfo(i);
				if( hackInfo.m_cFlag != 16 && hackInfo.m_cFlag != 25 && 
					hackInfo.m_cFlag != 26 && hackInfo.m_cFlag != 27 && hackInfo.m_cFlag != 28 && hackInfo.m_cFlag != 29 &&
					hackInfo.m_cFlag != 32 && hackInfo.m_cFlag != 33 && hackInfo.m_cFlag != 34 && hackInfo.m_cFlag != 35 )
				{
					MakeUpperCase( hackInfo.m_wstrProcessName );
					MakeUpperCase( hackInfo.m_wstrWindowClassName );
				}				

				switch(hackInfo.m_cFlag)
				{
				case ANTIHACKING_ID::ANTIHACKING_FILGER_01: // Process
					{
						for(UINT j=0; j<vecProcessInfo.size(); ++j)
						{
							wstring processName = vecProcessInfo[j].ProcessName;
							MakeUpperCase(processName);

							if( StrStrW(processName.c_str(), hackInfo.m_wstrProcessName.c_str()) )
							{
#ifdef SEND_HACKING_PROCESS_CHECK_RESULT 
								if( NULL != g_pMain )
									g_pMain->Send_EGS_HACKING_USER_PROCESS_NAME_RECORD_NOT( hackInfo.m_cFlag, processName );
#endif // SEND_HACKING_PROCESS_CHECK_RESULT

								if(g_pData != NULL && g_pData->GetServerProtocol() != NULL )
								{
#ifdef FIX_HACKUSER_REG01
									if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
										g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
									{
										g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );
										g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
									}
#endif
								}	

								string strProcessName;
								ConvertWCHARToChar( strProcessName, processName.c_str() );


								string strMailTitle = ANTI_HACK_STRING_AntiHacking_Process;
								strMailTitle = strMailTitle + strProcessName;
									
#ifdef ADD_COLLECT_CLIENT_INFO
								g_pMain->SendHackInfo3( hackInfo.m_cFlag, strProcessName.c_str(), true, true );
								return false;
#else
								g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());
								m_pKTDXApp->SetFindHacking( true );
								return true;
#endif
							}
						}
					}					
					break;
				case ANTIHACKING_ID::ANTIHACKING_FILGER_02: // WindowText
					{
						for(UINT j=0; j<m_vecWindowInfo.size(); ++j)
						{
							KHackingToolInfo windowInfo = m_vecWindowInfo[j];
							if( StrStrW(windowInfo.m_wstrProcessName.c_str(), hackInfo.m_wstrProcessName.c_str()) )
							{
#ifdef SEND_HACKING_PROCESS_CHECK_RESULT 
								if( NULL != g_pMain )
									g_pMain->Send_EGS_HACKING_USER_PROCESS_NAME_RECORD_NOT( hackInfo.m_cFlag, windowInfo.m_wstrProcessName );
#endif // SEND_HACKING_PROCESS_CHECK_RESULT

								if(g_pData != NULL && g_pData->GetServerProtocol() != NULL )
								{
#ifdef FIX_HACKUSER_REG01
									if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
										g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
									{
										g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );
										g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
									}
#endif
								}		

								string strProcessName;
								ConvertWCHARToChar( strProcessName, windowInfo.m_wstrProcessName.c_str() );


								string strMailTitle = ANTI_HACK_STRING_AntiHacking_WindowText;
								strMailTitle = strMailTitle + strProcessName;

								
#ifdef ADD_COLLECT_CLIENT_INFO
								g_pMain->SendHackInfo4( hackInfo.m_cFlag, strProcessName.c_str(), true, true );
								return false;
#else
								g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());
								m_pKTDXApp->SetFindHacking( true );
								return true;
#endif
							}
#if 0
							else if( FindWindow(NULL, hackInfo.m_wstrProcessName.c_str()) != NULL )
							{
								if(g_pData != NULL && g_pData->GetServerProtocol() != NULL )
								{
#ifdef FIX_HACKUSER_REG01
									if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
										g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
									{
										g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );
										g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
									}
#endif
								}		

								string strProcessName;
								ConvertWCHARToChar( strProcessName, hackInfo.m_wstrProcessName.c_str() );


								string strMailTitle = ANTI_HACK_STRING_AntiHacking_FindWindow;
								strMailTitle = strMailTitle + strProcessName;

								g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());
								m_pKTDXApp->SetFindHacking( true );
								return true;
							}
#endif
						}
					}					
					break;
				case ANTIHACKING_ID::ANTIHACKING_FILGER_03: // WindowClass
					for(UINT j=0; j<m_vecWindowInfo.size(); ++j)
					{
						KHackingToolInfo windowInfo = m_vecWindowInfo[j];
						if( StrStrW(windowInfo.m_wstrWindowClassName.c_str(), hackInfo.m_wstrWindowClassName.c_str()) )
						{
							if(g_pData != NULL && g_pData->GetServerProtocol() != NULL )
							{
#ifdef FIX_HACKUSER_REG01
								if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
									g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
								{
									g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );
									g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
								}
#endif
							}	

							string strProcessName;
							ConvertWCHARToChar( strProcessName, windowInfo.m_wstrWindowClassName.c_str() );


							string strMailTitle = ANTI_HACK_STRING_AntiHacking_WindowClass;
							strMailTitle = strMailTitle + strProcessName;

#ifdef ADD_COLLECT_CLIENT_INFO
							g_pMain->SendHackInfo5( hackInfo.m_cFlag, strProcessName.c_str(), true, true );
							return false;
#else
							g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());
							m_pKTDXApp->SetFindHacking( true );
							return true;
#endif
						}
					}
					break;
				case ANTIHACKING_ID::ANTIHACKING_FILGER_04: // WindowText & WindowClass
					{
						for(UINT j=0; j<m_vecWindowInfo.size(); ++j)
						{
							KHackingToolInfo windowInfo = m_vecWindowInfo[j];
							if( StrStrW(windowInfo.m_wstrProcessName.c_str(), hackInfo.m_wstrProcessName.c_str()) &&
								StrStrW(windowInfo.m_wstrWindowClassName.c_str(), hackInfo.m_wstrWindowClassName.c_str()) )
							{
								if(g_pData != NULL && g_pData->GetServerProtocol() != NULL )
								{
#ifdef FIX_HACKUSER_REG01
									if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
										g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
									{
										g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );
										g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
									}
#endif
								}		

								string strWindowName, strWindowClass;
								ConvertWCHARToChar( strWindowName, windowInfo.m_wstrProcessName.c_str() );
								ConvertWCHARToChar( strWindowClass, windowInfo.m_wstrWindowClassName.c_str() );


								string strMailTitle = ANTI_HACK_STRING_AntiHacking_WindowText_WindowClass;
								strMailTitle = strMailTitle + strWindowName + ", " + strWindowClass;

#ifdef ADD_COLLECT_CLIENT_INFO
								strMailTitle = strWindowName + ", " + strWindowClass;
								g_pMain->SendHackInfo1( hackInfo.m_cFlag, strMailTitle.c_str(), true, true );
								return false;
#else
								g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());
								m_pKTDXApp->SetFindHacking( true );
								return true;
#endif
							}
						}
					}
					break;
				case ANTIHACKING_ID::ANTIHACKING_FILGER_05: // Event
					{
						for(UINT j=0; j<m_vecWindowInfo.size(); ++j)
						{
							KHackingToolInfo windowInfo = m_vecWindowInfo[j];
							if(OpenEvent( EVENT_ALL_ACCESS, FALSE, hackInfo.m_wstrProcessName.c_str() ))
							{							
								if(g_pData != NULL && g_pData->GetServerProtocol() != NULL )
								{
#ifdef FIX_HACKUSER_REG01
									if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
										g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
									{
										g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );
										g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
									}
#endif
								}	

								string strProcessName;
								ConvertWCHARToChar( strProcessName, hackInfo.m_wstrProcessName.c_str() );


								string strMailTitle = ANTI_HACK_STRING_AntiHacking_Event;
								strMailTitle = strMailTitle + strProcessName;

#ifdef ADD_COLLECT_CLIENT_INFO
								g_pMain->SendHackInfo2( hackInfo.m_cFlag, strProcessName.c_str(), true, true );
								return false;
#else
								g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());
								m_pKTDXApp->SetFindHacking( true );
								return true;
#endif
							}
						}
					}
					break;
				case ANTIHACKING_ID::ANTIHACKING_FILGER_06: // Mutex
					{
						for(UINT j=0; j<m_vecWindowInfo.size(); ++j)
						{
							KHackingToolInfo windowInfo = m_vecWindowInfo[j];
							HANDLE hMutex = CreateMutex( NULL, FALSE, hackInfo.m_wstrProcessName.c_str() );
							if(GetLastError() == ERROR_ALREADY_EXISTS)
							{	
								if(g_pData != NULL && g_pData->GetServerProtocol() != NULL )
								{
#ifdef FIX_HACKUSER_REG01
									if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
										g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
									{
										g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );
										g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
									}
#endif
								}								

								string strProcessName;
								ConvertWCHARToChar( strProcessName, hackInfo.m_wstrProcessName.c_str() );



								string strMailTitle = ANTI_HACK_STRING_AntiHacking_Mutex;
								strMailTitle = strMailTitle + strProcessName;

#ifdef ADD_COLLECT_CLIENT_INFO
								g_pMain->SendHackInfo3( hackInfo.m_cFlag, strProcessName.c_str(), true, true );
								return false;
#else
								g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());
								m_pKTDXApp->SetFindHacking( true );
								return true;
#endif
							}	
							else
							{
								ReleaseMutex(hMutex);
								CloseHandle(hMutex);
							}
						}
 					} 
					break;
				case ANTIHACKING_ID::ANTIHACKING_FILGER_07: // Injected DLL Module in current process
					{
						//for(int index=0; index<vecModuleName.size(); ++index)
						//{
						//	if(StrStrW(vecModuleName[index].c_str(), hackInfo.m_wstrProcessName.c_str()))
						//	{
						//		if(g_pData != NULL && g_pData->GetServerProtocol() != NULL )
						//		{
						//			g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );								
						//			if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL )
						//				g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
						//		}	

						//		string strProcessName;
						//		ConvertWCHARToChar( strProcessName, hackInfo.m_wstrProcessName.c_str() );
						//		string strMailTitle = "AntiHacking Module - ";
						//		strMailTitle = strMailTitle + strProcessName;

						//		//g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());
						//		m_pKTDXApp->SetFindHacking( true );
						//		return true;
						//	}
						//}

						// ���˻�
						HMODULE hMod = GetModuleHandle( hackInfo.m_wstrProcessName.c_str() );
						if( hMod )
						{				
#ifdef SEND_HACKING_PROCESS_CHECK_RESULT 
							if( NULL != g_pMain )
								g_pMain->Send_EGS_HACKING_USER_PROCESS_NAME_RECORD_NOT( hackInfo.m_cFlag, hackInfo.m_wstrProcessName );
#endif // SEND_HACKING_PROCESS_CHECK_RESULT

							if(g_pData != NULL && g_pData->GetServerProtocol() != NULL )
							{
#ifdef FIX_HACKUSER_REG01
								if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
									g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
								{
									g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );
									g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
								}
#endif
							}								

							string strModuleName;
							ConvertWCHARToChar( strModuleName, hackInfo.m_wstrProcessName.c_str() );


							string strMailTitle = ANTI_HACK_STRING_AntiHacking_Module;
							strMailTitle = strMailTitle + strModuleName;

#ifdef ADD_COLLECT_CLIENT_INFO
							g_pMain->SendHackInfo4( hackInfo.m_cFlag, strModuleName.c_str(), true, true );
							return false;
#else
							g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());
							m_pKTDXApp->SetFindHacking( true );
							return true;
#endif
						}
					}
					break;
				case ANTIHACKING_ID::ANTIHACKING_FILGER_08:	//	File
					{
						// ������������ �˻�
						OFSTRUCT of;
						string strFileName;
						ConvertWCHARToChar( strFileName, hackInfo.m_wstrProcessName.c_str() );

						if( OpenFile(strFileName.c_str(), &of, OF_EXIST) != HFILE_ERROR )
						{
							if(g_pData != NULL && g_pData->GetServerProtocol() != NULL )
							{
#ifdef FIX_HACKUSER_REG01
								if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
									g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
								{
									g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );
									g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
								}
#endif
							}


							string strMailTitle = ANTI_HACK_STRING_AntiHacking_FileExist;
							strMailTitle = strMailTitle + strFileName;

#ifdef ADD_COLLECT_CLIENT_INFO
							g_pMain->SendHackInfo5( hackInfo.m_cFlag, strFileName.c_str(), true, true );
							return false;
#else
							g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());
							m_pKTDXApp->SetFindHacking( true );							

							//::DeleteFile(hackInfo.m_wstrProcessName.c_str());

							return true;
#endif
						}
					} 
					break;
#ifdef CHECK_FROM_DLLMANAGER
				case ANTIHACKING_ID::ANTIHACKING_FILGER_09:	//	DLL Manager & not exit game
					{
						if( SiCX2DLLManager()->FindModule( hackInfo.m_wstrProcessName.c_str() ) == true )
						{
							if(g_pData != NULL && g_pData->GetServerProtocol() != NULL )
							{								
#ifdef FIX_HACKUSER_REG01
								if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
									g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
								{
									g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );
									g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
								}
#endif
							}		

							string strDllName;
							ConvertWCHARToChar( strDllName, hackInfo.m_wstrProcessName.c_str() );


							string strMailTitle = ANTI_HACK_STRING_AntiHacking_DLL_Find;
							strMailTitle = strMailTitle + strDllName;

#ifdef ADD_COLLECT_CLIENT_INFO
							g_pMain->SendHackInfo1( hackInfo.m_cFlag, strDllName.c_str(), false, true );							
#else
							g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());							
#endif
						}
					} 
					break;
				case ANTIHACKING_ID::ANTIHACKING_FILGER_10:	//	DLL Manager & exit game
					{
						if( SiCX2DLLManager()->FindModule( hackInfo.m_wstrProcessName.c_str() ) == true )
						{

#ifdef SEND_HACKING_PROCESS_CHECK_RESULT 
							if( NULL != g_pMain )
								g_pMain->Send_EGS_HACKING_USER_PROCESS_NAME_RECORD_NOT( hackInfo.m_cFlag, hackInfo.m_wstrProcessName );
#endif // SEND_HACKING_PROCESS_CHECK_RESULT

							if(g_pData != NULL && g_pData->GetServerProtocol() != NULL )
							{
#ifdef FIX_HACKUSER_REG01
								if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
									g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
								{
									g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );
									g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
								}
#endif
							}		

							string strDllName;
							ConvertWCHARToChar( strDllName, hackInfo.m_wstrProcessName.c_str() );


							string strMailTitle = ANTI_HACK_STRING_AntiHacking_DLL_Find_Exit;
							strMailTitle = strMailTitle + strDllName;

#ifdef ADD_COLLECT_CLIENT_INFO
							g_pMain->SendHackInfo2( hackInfo.m_cFlag, strDllName.c_str(), true, true );
							return false;
#else
							g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());
							m_pKTDXApp->SetFindHacking( true );	
							return true;
#endif
						}
					} 
					break;
#endif
#ifdef FIND_ZOMBIE_PROCESS
				case ANTIHACKING_ID::ANTIHACKING_FILGER_11:
					{
						for(UINT j=0; j<vecZombieProcessInfo.size(); ++j)
						{
							wstring processName = vecZombieProcessInfo[j].ProcessName;
							MakeUpperCase(processName);

							if( StrStrW(processName.c_str(), hackInfo.m_wstrProcessName.c_str()) )
							{

#ifdef SEND_HACKING_PROCESS_CHECK_RESULT 
								if( NULL != g_pMain )
									g_pMain->Send_EGS_HACKING_USER_PROCESS_NAME_RECORD_NOT( hackInfo.m_cFlag, processName );
#endif // SEND_HACKING_PROCESS_CHECK_RESULT

								if(g_pData != NULL && g_pData->GetServerProtocol() != NULL )
								{
									if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
										g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
									{
										g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );
										g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
									}
								}	

								string strProcessName;
								ConvertWCHARToChar( strProcessName, processName.c_str() );



								string strMailTitle = ANTI_HACK_STRING_AntiHacking_ZombiProcess;
								strMailTitle = strMailTitle + strProcessName;

#ifdef ADD_COLLECT_CLIENT_INFO
								g_pMain->SendHackInfo3( hackInfo.m_cFlag, strProcessName.c_str(), true, true );
								return false;
#else
								g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());
								m_pKTDXApp->SetFindHacking( true );
								return true;
#endif
							}
						}						
					}
					break;
#endif
#ifdef ADD_INFO_MACADDRESS
				case ANTIHACKING_ID::ANTIHACKING_FILGER_12:
					{
						if(g_pMain == NULL || g_pMain->GetSystemInfo() == NULL)
							break;

						string strBlockedMACAddr;
						ConvertWCHARToChar( strBlockedMACAddr, hackInfo.m_wstrProcessName.c_str() );

						const CX2SystemInfo::NetworkAdapterVector& vecAdapter = g_pMain->GetSystemInfo()->GetNetworkAdapter();
						BOOST_TEST_FOREACH(CX2SystemInfo::NETWORK_ADAPTER_INFO, info, vecAdapter)
						{
							if( StrStrA(info.MACAddress.c_str(), strBlockedMACAddr.c_str()) )
							{
								if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
									g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
								{
									g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );
									g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
								}
							}

							string strMailTitle = ANTI_HACK_STRING_AntiHacking_MAC_Address;
							strMailTitle = strMailTitle + strBlockedMACAddr;

#ifdef ADD_COLLECT_CLIENT_INFO
							g_pMain->SendHackInfo4( hackInfo.m_cFlag, strBlockedMACAddr.c_str(), true, true );
							return false;
#else
							g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());
							m_pKTDXApp->SetFindHacking( true );
							return true;
#endif
						}
					}
					break;
#endif // ADD_INFO_MACADDRESS
#ifdef HACK_USER_ATTACH_FILE
				case ANTIHACKING_ID::ANTIHACKING_FILGER_13:
					{
						if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
							g_pData->GetMyUser()->GetUserData()->hackingUserType == CX2User::HUT_AGREE_HACK_USER )
						{
							// ������������ �˻�
							OFSTRUCT of;
							string strFileName;
							ConvertWCHARToChar( strFileName, hackInfo.m_wstrProcessName.c_str() );

							if( OpenFile(strFileName.c_str(), &of, OF_EXIST) != HFILE_ERROR )
							{							
								g_pMain->SendHackMail_AttachFile(strFileName.c_str(), hackInfo.m_wstrWindowClassName );
							}
						}
					}
					break;
#endif
#ifdef SERACH_FOLDER_FILE
				case ANTIHACKING_ID::ANTIHACKING_FILGER_14:
					{
						if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
							g_pData->GetMyUser()->GetUserData()->hackingUserType == CX2User::HUT_AGREE_HACK_USER )
						{
							bool			bFind = false;
							HANDLE          hFind;
							WIN32_FIND_DATA dataFind;
							BOOL            bMoreFile = TRUE;
							wstring wstrMailTitle = hackInfo.m_wstrProcessName;
							wstrMailTitle = wstrMailTitle + L"\n";

							string strDirName;
							ConvertWCHARToChar( strDirName, hackInfo.m_wstrProcessName.c_str() );

							hFind = FindFirstFile(hackInfo.m_wstrProcessName.c_str(), &dataFind);
							while(hFind != INVALID_HANDLE_VALUE && bMoreFile == TRUE)
							{
								if(dataFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
								{
									if(!(wcscmp(dataFind.cFileName, L".") == 0 || wcscmp(dataFind.cFileName, L"..") == 0))
									{										
										wstrMailTitle = wstrMailTitle + dataFind.cFileName;
										wstrMailTitle = wstrMailTitle + L"   [DIR]\n";
									}
								}
								else
								{									
									wstrMailTitle = wstrMailTitle + dataFind.cFileName;
									wstrMailTitle = wstrMailTitle + L"   [FILE]\n";
								}
						
								bFind = true;
								bMoreFile = FindNextFile(hFind, &dataFind);
							}							

							if( bFind == true )
							{
								string strMailTitle;
								ConvertWCHARToChar( strMailTitle, wstrMailTitle.c_str() );
								g_pMain->SendHackMail_FileList(strMailTitle.c_str(), hackInfo.m_wstrWindowClassName);
							}							
						}
					}
					break;
#endif
				case ANTIHACKING_ID::ANTIHACKING_FILGER_15: // Process & Not Exit Game
					{
						for(UINT j=0; j<vecProcessInfo.size(); ++j)
						{
							wstring processName = vecProcessInfo[j].ProcessName;
							MakeUpperCase(processName);

							if( StrStrW(processName.c_str(), hackInfo.m_wstrProcessName.c_str()) )
							{
// 								if(g_pData != NULL && g_pData->GetServerProtocol() != NULL )
// 								{
// #ifdef FIX_HACKUSER_REG01
// 									if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
// 										g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
// 									{
// 										g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );
// 										g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
// 									}
// #endif
// 								}	

								string strProcessName;
								ConvertWCHARToChar( strProcessName, processName.c_str() );


								string strMailTitle = ANTI_HACK_STRING_AntiHacking_Process;
								strMailTitle = strMailTitle + strProcessName;

#ifdef ADD_COLLECT_CLIENT_INFO
								g_pMain->SendHackInfo5( hackInfo.m_cFlag, strProcessName.c_str(), false, true );
#else
								g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());
								//m_pKTDXApp->SetFindHacking( true );
#endif
							}
						}
					}					
					break;
#ifdef USER_MONITORING_MAIL
				case ANTIHACKING_ID::ANTIHACKING_FILGER_16:	// Ư�� ���� ����͸�
					{
						if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
							g_pInstanceData != NULL && g_pInstanceData->GetUserID() == hackInfo.m_wstrProcessName )
						{
							// ����͸� ���� �߼�							
#ifdef ADD_COLLECT_CLIENT_INFO
							g_pMain->SendHackInfo1( hackInfo.m_cFlag, "", false, false );		
#else
							g_pMain->SendUserMonitoringMail(ANTI_HACK_USER_MONITORING);
#endif
						}
					}
					break;
#endif
				case ANTIHACKING_ID::ANTIHACKING_FILGER_17: // find module & not exit
					{						
						// ���˻�
						HMODULE hMod = GetModuleHandle( hackInfo.m_wstrProcessName.c_str() );
						if( hMod )
						{				
// 							if(g_pData != NULL && g_pData->GetServerProtocol() != NULL )
// 							{
// #ifdef FIX_HACKUSER_REG01
// 								if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
// 									g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
// 								{
// 									g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );
// 									g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
// 								}
// #endif
// 							}								

							string strModuleName;
							ConvertWCHARToChar( strModuleName, hackInfo.m_wstrProcessName.c_str() );


							string strMailTitle = ANTI_HACK_STRING_AntiHacking_Module;
							strMailTitle = strMailTitle + strModuleName;

#ifdef ADD_COLLECT_CLIENT_INFO
							g_pMain->SendHackInfo2( hackInfo.m_cFlag, strModuleName.c_str(), false, true );
#else
							g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());
#endif
						}
					}
					break;
#ifdef USER_MONITORING_MAIL
				case ANTIHACKING_ID::ANTIHACKING_FILGER_18:	// Ư�� ���� ����͸�
					{
#ifdef SERV_CLIENT_PUBLIC_IP
						if( g_pMain->m_wstrPublicIp == hackInfo.m_wstrProcessName )
						{
							// ����͸� ���� �߼�
#ifdef ADD_COLLECT_CLIENT_INFO
							g_pMain->SendHackInfo3( hackInfo.m_cFlag, "", false, false );
#else
							g_pMain->SendUserMonitoringMail(ANTI_HACK_IP_MONITORING);
#endif
						}
#endif
					}
					break;
#endif

#ifdef	DLL_MANAGER
				case ANTIHACKING_ID::ANTIHACKING_FILGER_19:	//	DLL Manager In & not exit game
					{
						if( SiCX2DLLManager()->FindModuleIn( hackInfo.m_wstrProcessName.c_str() ) == true )
						{
// 							if(g_pData != NULL && g_pData->GetServerProtocol() != NULL )
// 							{
// #ifdef FIX_HACKUSER_REG01
// 								if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
// 									g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
// 								{
// 									g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );
// 									g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
// 								}
// #endif
// 							}		

							string strDllName;
							ConvertWCHARToChar( strDllName, hackInfo.m_wstrProcessName.c_str() );


							string strMailTitle = ANTI_HACK_STRING_AntiHacking_DLL_Find;
							strMailTitle = strMailTitle + strDllName;

#ifdef ADD_COLLECT_CLIENT_INFO
							g_pMain->SendHackInfo4( hackInfo.m_cFlag, strDllName.c_str(), false, true );
#else
							g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());
#endif
						}
					} 
					break;
#endif	DLL_MANAGER
				case ANTIHACKING_ID::ANTIHACKING_FILGER_20: // WindowText & not Exit
					{
						for(UINT j=0; j<m_vecWindowInfo.size(); ++j)
						{
							KHackingToolInfo windowInfo = m_vecWindowInfo[j];
							if( StrStrW(windowInfo.m_wstrProcessName.c_str(), hackInfo.m_wstrProcessName.c_str()) )
							{
								string strProcessName;
								ConvertWCHARToChar( strProcessName, windowInfo.m_wstrProcessName.c_str() );


								string strMailTitle = ANTI_HACK_STRING_AntiHacking_WindowText;
								strMailTitle = strMailTitle + strProcessName;

#ifdef ADD_COLLECT_CLIENT_INFO
								g_pMain->SendHackInfo5( hackInfo.m_cFlag, strProcessName.c_str(), false, true );
								break;
#else
								g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());
#endif
							}
						}
					}
					break;
#ifdef FREE_MODULE
				case ANTIHACKING_ID::ANTIHACKING_FILGER_21: // ���� ��� ����
					{
						HMODULE hMod = GetModuleHandle( hackInfo.m_wstrProcessName.c_str() );
						if( hMod )
						{
							FreeLibrary(hMod);
						}						
					}
					break;

#ifdef	DLL_MANAGER
				case ANTIHACKING_ID::ANTIHACKING_FILGER_22: // dll manager�� ���� ��� ����
					{
						SiCX2DLLManager()->ForceFreeModule( hackInfo.m_wstrProcessName );
					}
					break;
#endif	DLL_MANAGER
				
#endif

#ifdef MACHINE_ID
				case ANTIHACKING_ID::ANTIHACKING_FILGER_23: // machine id�� ����͸�
					{
						if( g_pInstanceData != NULL && g_pInstanceData->GetMachineId().empty() == false )
						{
							wstring wstrMachineId = L"";
							ConvertUtf8ToWCHAR( wstrMachineId, g_pInstanceData->GetMachineId().c_str() );
							MakeUpperCase(wstrMachineId);
							if( hackInfo.m_wstrProcessName == wstrMachineId )
							{
								// ����͸� ���� �߼�
#ifdef ADD_COLLECT_CLIENT_INFO
								g_pMain->SendHackInfo1( hackInfo.m_cFlag, "", false, false );
#else
								g_pMain->SendUserMonitoringMail(ANTI_HACK_MACHINE_MONITORING);
#endif
							}
						}
					}
					break;
#endif
				case ANTIHACKING_ID::ANTIHACKING_FILGER_24: // findwindow
					{
						if( FindWindow(NULL, hackInfo.m_wstrProcessName.c_str()) != NULL )
						{

							if(g_pData != NULL && g_pData->GetServerProtocol() != NULL )
							{
#ifdef FIX_HACKUSER_REG01
								if( g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUserData() != NULL &&
									g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER )
								{
									g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT );
									g_pData->GetMyUser()->GetUserData()->hackingUserType = CX2User::HUT_AGREE_HACK_USER;
								}
#endif
							}		

							string strProcessName;
							ConvertWCHARToChar( strProcessName, hackInfo.m_wstrProcessName.c_str() );


							string strMailTitle = ANTI_HACK_STRING_AntiHacking_FindWindow;
							strMailTitle = strMailTitle + strProcessName;

#ifdef ADD_COLLECT_CLIENT_INFO
							g_pMain->SendHackInfo2( hackInfo.m_cFlag, strProcessName.c_str(), true, true );
							return false;
#else
							g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());
							m_pKTDXApp->SetFindHacking( true );
							return true;
#endif
						}
					}					
					break;
#ifdef SERV_SERIAL_NUMBER_AVAILABILITY_CHECK
				case ANTIHACKING_ID::ANTIHACKING_FILGER_25:	// Ư�� SN ����͸�
					{
						if( g_pInstanceData != NULL )
						{
							std::string strSn = base64_encode(g_pInstanceData->GetSN(), SERVER_SN);
							std::string strProcessName;
							ConvertWCHARToChar( strProcessName, hackInfo.m_wstrProcessName.c_str() );
														
							if( strcmp(strSn.c_str(), strProcessName.c_str()) == 0 )
							{
								// ����͸� ���� �߼�
#ifdef ADD_COLLECT_CLIENT_INFO
								g_pMain->SendHackInfo3( hackInfo.m_cFlag, "", false, false );
#else
								g_pMain->SendUserMonitoringMail(ANTI_HACK_USER_MONITORING);
#endif
							}
						}						
					}
					break;
#endif
#ifdef DESK_THREAD_WINDOW
				case ANTIHACKING_ID::ANTIHACKING_FILGER_26:	// ������ ������ Ÿ��Ʋ �˻� (not exit)
				case ANTIHACKING_ID::ANTIHACKING_FILGER_27:	// exit
					{
						for( UINT iThreadWindow = 0; iThreadWindow<m_vecDeskThreadInfo.size(); ++iThreadWindow )
						{
							if( hackInfo.m_wstrProcessName.empty() == false && m_vecDeskThreadInfo[iThreadWindow].m_wstrName.empty() == false )
							{
								if( m_vecDeskThreadInfo[iThreadWindow].m_wstrName.compare( hackInfo.m_wstrProcessName.c_str()) == 0 )
								{
									string strProcessName;
									ConvertWCHARToChar( strProcessName, m_vecDeskThreadInfo[iThreadWindow].m_wstrName.c_str() );


									string strMailTitle = ANTI_HACK_STRING_AntiHacking_TWindowText;
									strMailTitle = strMailTitle + strProcessName;

#ifdef ADD_COLLECT_CLIENT_INFO
									if( hackInfo.m_cFlag == ANTIHACKING_ID::ANTIHACKING_FILGER_27 )
									{
#ifdef SEND_HACKING_PROCESS_CHECK_RESULT 
										if( NULL != g_pMain )
											g_pMain->Send_EGS_HACKING_USER_PROCESS_NAME_RECORD_NOT( hackInfo.m_cFlag, m_vecDeskThreadInfo[iThreadWindow].m_wstrName );
#endif // SEND_HACKING_PROCESS_CHECK_RESULT

										g_pMain->SendHackInfo4( hackInfo.m_cFlag, strProcessName.c_str(), true, true );
										return false;
									}
									else
									{
										g_pMain->SendHackInfo5( hackInfo.m_cFlag, strProcessName.c_str(), false, true );
									}
#else
									g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());

									if( hackInfo.m_cFlag == ANTIHACKING_ID::ANTIHACKING_FILGER_27 )
									{
#ifdef SEND_HACKING_PROCESS_CHECK_RESULT 
										if( NULL != g_pMain )
											g_pMain->Send_EGS_HACKING_USER_PROCESS_NAME_RECORD_NOT( hackInfo.m_cFlag, m_vecDeskThreadInfo[iThreadWindow].m_wstrName );
#endif // SEND_HACKING_PROCESS_CHECK_RESULT
										m_pKTDXApp->SetFindHacking( true );
										return true;
									}
#endif
									break;
								}
							}							
						}						
					}
					break;
				case ANTIHACKING_ID::ANTIHACKING_FILGER_28:	// ������ ������ Ŭ���� �˻� (not exit)
				case ANTIHACKING_ID::ANTIHACKING_FILGER_29:	// exit
					{
						for( UINT iThreadWindow = 0; iThreadWindow<m_vecDeskThreadInfo.size(); ++iThreadWindow )
						{
							if( hackInfo.m_wstrWindowClassName.empty() == false && m_vecDeskThreadInfo[iThreadWindow].m_wstrClass.empty() == false )
							{
								if( m_vecDeskThreadInfo[iThreadWindow].m_wstrClass.compare(hackInfo.m_wstrWindowClassName.c_str()) == 0 )
								{
									string strProcessName;
									ConvertWCHARToChar( strProcessName, m_vecDeskThreadInfo[iThreadWindow].m_wstrClass.c_str() );


									string strMailTitle = ANTI_HACK_STRING_AntiHacking_TWindowClass;
									strMailTitle = strMailTitle + strProcessName;

#ifdef ADD_COLLECT_CLIENT_INFO
									if( hackInfo.m_cFlag == ANTIHACKING_ID::ANTIHACKING_FILGER_29 )
									{
										g_pMain->SendHackInfo1( hackInfo.m_cFlag, strProcessName.c_str(), true, true );
										return false;
									}
									else
									{
										g_pMain->SendHackInfo2( hackInfo.m_cFlag, strProcessName.c_str(), false, true );
									}
#else
									g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());

									if( hackInfo.m_cFlag == ANTIHACKING_ID::ANTIHACKING_FILGER_29 )
									{
										m_pKTDXApp->SetFindHacking( true );
										return true;
									}
#endif
									break;
								}
							}							
						}
					}
					break;		
#ifdef CHECK_ALL_WINDOW
				case ANTIHACKING_ID::ANTIHACKING_FILGER_32:	// ������ ������ Ÿ��Ʋ �˻� (not exit)
				case ANTIHACKING_ID::ANTIHACKING_FILGER_33:	// exit
					{
						for( UINT iThreadWindow = 0; iThreadWindow<m_vecTopWndInfo.size(); ++iThreadWindow )
						{
							if( hackInfo.m_wstrProcessName.empty() == false && m_vecTopWndInfo[iThreadWindow].m_wstrName.empty() == false )
							{
								if( m_vecTopWndInfo[iThreadWindow].m_wstrName.compare( hackInfo.m_wstrProcessName.c_str()) == 0 )
								{
									string strProcessName;
									ConvertWCHARToChar( strProcessName, m_vecTopWndInfo[iThreadWindow].m_wstrName.c_str() );


									string strMailTitle = ANTI_HACK_STRING_AntiHacking_TWindowText;
									strMailTitle = strMailTitle + strProcessName;

#ifdef ADD_COLLECT_CLIENT_INFO
									if( hackInfo.m_cFlag == ANTIHACKING_ID::ANTIHACKING_FILGER_33 )
									{
										g_pMain->SendHackInfo4( hackInfo.m_cFlag, strProcessName.c_str(), true, true );
										return false;
									}
									else
									{
										g_pMain->SendHackInfo5( hackInfo.m_cFlag, strProcessName.c_str(), false, true );
									}
#else
									g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());

									if( hackInfo.m_cFlag == ANTIHACKING_ID::ANTIHACKING_FILGER_33 )
									{
										m_pKTDXApp->SetFindHacking( true );
										return true;
									}
#endif
									break;
								}
							}							
						}						
					}
					break;
				case ANTIHACKING_ID::ANTIHACKING_FILGER_34:	// ������ ������ Ŭ���� �˻� (not exit)
				case ANTIHACKING_ID::ANTIHACKING_FILGER_35:	// exit
					{
						for( UINT iThreadWindow = 0; iThreadWindow<m_vecTopWndInfo.size(); ++iThreadWindow )
						{
							if( hackInfo.m_wstrWindowClassName.empty() == false && m_vecTopWndInfo[iThreadWindow].m_wstrClass.empty() == false )
							{
								if( m_vecTopWndInfo[iThreadWindow].m_wstrClass.compare(hackInfo.m_wstrWindowClassName.c_str()) == 0 )
								{
									string strProcessName;
									ConvertWCHARToChar( strProcessName, m_vecTopWndInfo[iThreadWindow].m_wstrClass.c_str() );


									string strMailTitle = ANTI_HACK_STRING_AntiHacking_TWindowClass;
									strMailTitle = strMailTitle + strProcessName;

#ifdef ADD_COLLECT_CLIENT_INFO
									if( hackInfo.m_cFlag == ANTIHACKING_ID::ANTIHACKING_FILGER_35 )
									{
										g_pMain->SendHackInfo1( hackInfo.m_cFlag, strProcessName.c_str(), true, true );
										return false;
									}
									else
									{
										g_pMain->SendHackInfo2( hackInfo.m_cFlag, strProcessName.c_str(), false, true );
									}
#else
									g_pMain->SendHackMail_DamageHistory(strMailTitle.c_str());

									if( hackInfo.m_cFlag == ANTIHACKING_ID::ANTIHACKING_FILGER_35 )
									{
										m_pKTDXApp->SetFindHacking( true );
										return true;
									}
#endif
									break;
								}
							}							
						}
					}
					break;	
#endif //CHECK_ALL_WINDOW
#endif	//DESK_THREAD_WINDOW

				default:
					{
#ifdef ADD_COLLECT_CLIENT_INFO
						// �ش� �÷��װ� ���µ�?
						g_pMain->SendHackInfo3( ANTIHACKING_ID::ANTIHACKING_FILGER_31, "", false, true );
#endif
					}
					break;
				}

				if( m_bForceExitThread == true )
					return true;
			}
		}
		else
		{
			// �ٸ���Ʈ �޸� ��ŷ�Ǿ��� ������ ����!
			// g_pMain->SendHackMail_DamageHistory("-- EmptyList --");
#ifdef ADD_COLLECT_CLIENT_INFO
			g_pMain->SendHackInfo4( ANTIHACKING_ID::ANTIHACKING_FILGER_30, "", false, true );
#else
			g_pMain->SendHackMail_DamageHistory("-- EmptyList --");
			return false;
#endif
		}
	}
	
#if defined( _SERVICE_ ) 
	ELSWORD_VIRTUALIZER_END
#endif
	return false;
}
#endif

//////////////////////////////////////////////////////////////////////////
void CX2Main::TimedMessagePopUp::OnFrameMove( double fTime, float fElapsedTime )
{
	if( NULL == m_pDialog )
	{
		// ���̾�αװ� ������ �ð� 0���� �ʱ�ȭ
		m_fTimeLeft = 0.f;
		return;
	}

	if( 0.f == m_fTimeLeft )
		return; 
	

	float fOldTimeLeft = m_fTimeLeft;
	m_fTimeLeft -= fElapsedTime;
	if( true == g_pKTDXApp->GetDGManager()->GetDialogManager()->CheckDlg( m_pDialog ) )
	{
		if( m_fTimeLeft < 0.f )
		{
			m_pDialog->SetModal(false);
			m_pDialog->SetShowEnable( false, false );
			if( NULL != m_pDialog->GetUIDialogManager() )
				m_pDialog->GetUIDialogManager()->CountModalDialog();
			
			m_fTimeLeft = 0.f;		

			//if( NULL != CBOnTimeOut )
			//{
			//	CBOnTimeOut( m_pUserData );
			//}
		}
		else
		{
			float fTimeLeftInt = (float) (int) fOldTimeLeft;
			if( fOldTimeLeft > fTimeLeftInt && fTimeLeftInt >= m_fTimeLeft )
			{
				CKTDGUIStatic* pStatic = (CKTDGUIStatic*) m_pDialog->GetControl( L"StaticMsgBoxBG" );

				ASSERT( NULL != pStatic );
				if( NULL == pStatic )
				{
					WCHAR wszErrorMsg[1024] = L"";
					StringCchPrintfW( wszErrorMsg, ARRAY_SIZE( wszErrorMsg ), L"timed popup error, %d, %s, %s", (int)m_eMessageType, m_pDialog->GetName(), m_wstrMsg.c_str() );
					StateLog( wszErrorMsg );
					m_fTimeLeft = 0.f;

					// m_pDialog�� ���� ������ �־�� �� dialog�� �ƴϹǷ� �������� �ʰ� NULL�� �ٲ۴�.
					m_pDialog = NULL;
				}

				if( NULL != pStatic )
				{
					WCHAR wszText[256] = L"";
					//��Ʈ�� ���� �ʿ�
					switch(m_eMessageType)
					{
#ifdef ADDED_RELATIONSHIP_SYSTEM
						case MT_NONE :		/// �ش� Ÿ������ �����Ǿ� �ִٸ�, ���� �ð��� ǥ������ ����
							{
								StringCchPrintfW( wszText, ARRAY_SIZE(wszText), m_wstrMsg.c_str() );
							}
							break;
						case MT_AGREE_COUPLE :	
							{

								if ( NULL != g_pData->GetRelationshipManager() &&
									NULL != g_pData->GetRelationshipManager()->GetMyRelationshipInfo() )
								{

									wstring wstrText = GET_REPLACED_STRING( ( STR_ID_24447, "Li", m_wstrMsg, static_cast<int>( m_fTimeLeft ) ) );	/// %d�� �� �ڵ� ���
									StringCchPrintfW( wszText, ARRAY_SIZE(wszText), wstrText.c_str() );
								}
							} break;
#endif // ADDED_RELATIONSHIP_SYSTEM
						case MT_PARTY_READY_TO_PLAY_DUNGEON_REQUEST:
						case MT_DUNGEON_MATCH_START_REQUEST:
						case MT_DUNGEON_MATCH_REMAKING:
#ifdef ADDED_RELATIONSHIP_SYSTEM
						case MT_WAIT_CHOICE_COUPLE:
#endif // ADDED_RELATIONSHIP_SYSTEM
							{

								wstring wstrText = GET_REPLACED_STRING( ( STR_ID_24447, "Li", m_wstrMsg, static_cast<int>( m_fTimeLeft ) ) );	/// %d�� �� �ڵ� ���
								StringCchPrintfW( wszText, ARRAY_SIZE(wszText), wstrText.c_str() );

							} break;
#ifdef SERV_NEW_DEFENCE_DUNGEON_ENTER_LOGIC // ���볯¥: 2013-04-23
						case MT_ENTER_GATE_OF_DARKNESS:
							{
								wstring wstrText = GET_REPLACED_STRING( ( STR_ID_24446, "Li", m_wstrMsg, static_cast<int>( m_fTimeLeft ) ) );	/// %d�� �� �ڵ� ����
								StringCchPrintfW( wszText, ARRAY_SIZE(wszText), wstrText.c_str() );
							} break;
#endif // SERV_NEW_DEFENCE_DUNGEON_ENTER_LOGIC

#ifdef ADDED_RELATIONSHIP_SYSTEM
						case MT_WAIT_SUMMON_MARRIAGED_USER:
						case MT_SELECT_SUMMON_TO_MARRIAGED_USER:
							{
								wstring wstrText = GET_REPLACED_STRING( ( STR_ID_24520, "Li", m_wstrMsg, static_cast<int>( m_fTimeLeft ) ) );	/// ���� �ð� : %d��
								StringCchPrintfW( wszText, ARRAY_SIZE(wszText), wstrText.c_str() );

								if ( NULL != g_pMain )		/// ���� ���� �� ���� ���� ī��Ʈ ���̶��, �ٷ� ���� ó��
								{
									CX2State* pNowState = static_cast<CX2State*>( g_pMain->GetNowState() );

									if ( NULL != pNowState && true == pNowState->GetWaitGameStart() )
										m_fTimeLeft = 0.f;
								}
							} break;
#endif // ADDED_RELATIONSHIP_SYSTEM
						default:
							{
								StringCchPrintfW( wszText, ARRAY_SIZE(wszText), L"%s (%d%s)", m_wstrMsg.c_str(), static_cast<int>( m_fTimeLeft ), GET_STRING( STR_ID_180 ) );	/// ��
							} break;
					}
					wstring tempText = wszText;

					const int constTextMaxLen = (int)(g_pKTDXApp->GetResolutionScaleX() * 366);
					//bool checkEnterChar = false;
					int nowTextLen = 0;
					int lineNum = 1;

					CKTDGFontManager::CUKFont* pFont = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetUKFont( pStatic->GetString(0)->fontIndex );
					if( NULL != pFont )
					{
#ifdef CLIENT_GLOBAL_LINEBREAK
						lineNum = CWordLineHandler::LineBreakInX2MainMsgBox( tempText, pFont, constTextMaxLen );
#else //CLIENT_GLOBAL_LINEBREAK

						for ( int i = 0; i < (int)tempText.size(); i++ )
						{
							WCHAR tempWchar = tempText[i];
							if ( nowTextLen < constTextMaxLen && tempWchar == '\n' )
							{
								if ( i != (int)tempText.size() - 1 )
								{
									lineNum++;
								}

								nowTextLen = 0;
								continue;
							}
							int iUniCharSize = 0;
							//		uniBuffer.Clear();
							//		uniBuffer.InsertChar( 0, tempWchar );
							//		uniBuffer.CPtoX(uniBuffer.GetTextSize(), FALSE, &iUniCharSize);

							iUniCharSize = pFont->GetWidth( tempWchar );
							nowTextLen += iUniCharSize;

							if ( nowTextLen >= constTextMaxLen )
							{
								if ( i != (int)tempText.size() - 1 )
								{
									lineNum++;
								}

								nowTextLen = 0;
								wstring enterChar = L"\n";
								tempText.insert( tempText.begin() + i, enterChar.begin(), enterChar.end() );
								i++;
							}

							if ( i >= (int)tempText.size() )
							{
								break;
							}
						}
#endif //CLIENT_GLOBAL_LINEBREAK
					}
					pStatic->GetString(0)->msg = tempText;
				}
			}
		}
	}
	else
	{
		m_fTimeLeft = 0.f;
	}
}


//{{ 2011.9.16	����ȣ  �ݵ�ķ ������ ĸ�� ����
#ifdef BANDICAM_RECORDING
bool CX2Main::Bandi_StartVideoCapture()
{
	if( m_fCaptureDelay > 0.0f )
	{
		if( g_pChatBox != NULL )
		{
			g_pChatBox->AddChatLog( GET_STRING( STR_ID_17538 ), KEGS_CHAT_REQ::CPT_SYSTEM, D3DXCOLOR(1,1,0,1), L"#CFFFF00" );
		}
		return false;
	}

	m_fCaptureDelay = 3.0f;

	IDirect3DDevice9* pd3d9Device = DXUTGetD3DDevice();

	// ĸó�� �����մϴ�.
	if(FALSE == m_BandiCaptureLibrary.IsCapturing())
	{
		// ���� ����� DLL �� �ε��ϰ� ���� �ν��Ͻ��� �����մϴ�. 
		if(FALSE ==m_BandiCaptureLibrary.IsCreated())
		{
			// ���н� DLL�� .H������ ������ �������� ���ε��� Ȯ���� ���ñ� �ٶ��ϴ�.
			if(FAILED(m_BandiCaptureLibrary.Create(BANDICAP_RELEASE_DLL_FILE_NAME)))
				return false;

			// ������ ���� ������ ���ڵ��� ������ ��ܿ� �ΰ��� ǥ�õ˴ϴ�. 
			// ���ı����� �߱޹��� Ű�� ����ϰų�, Ȩ���������� ��������� ������ Ű�� ����ؼ� ������ �޽��ϴ�. 
			if(FAILED(m_BandiCaptureLibrary.Verify("KOG_ELSWORD_20110825", "675a5521")))
				return false;
		}

		if(m_BandiCaptureLibrary.IsCreated())//3.ó���̸� ���ڵ�
		{
			BCAP_CONFIG cfg;

			if(GetGameOption()->GetUserSetting()==false)
			{
				int iVal=GetGameOption()->GetGeneralVideoType();
				if(iVal>14)
					iVal+=4;//�߰� �̳� 4������ �ǳʶٸ� zoom�� ���ɴϴ�

				if(BCAP_PRESET_VIDEO_EDITING!=21)
				{
					ASSERT(false);//���������� �ٲ� �߰� �̳��� �ٲ��츦 ����ؼ� üũ�ڵ�
				}
				BCapConfigPreset(&cfg, (BCAP_PRESET)iVal);
			}
			else
			{
				//BCAP_PRESET bp;
				BCapConfigPreset(&cfg, (BCAP_PRESET)0);
				cfg.VideoSizeH=GetGameOption()->GetVideoSizeH();
				cfg.VideoSizeW=GetGameOption()->GetVideoSizeW();
				cfg.VideoFPS=GetGameOption()->GetVideoFPS();
				cfg.VideoQuality=GetGameOption()->GetVideoQuality();
				cfg.AudioChannels=GetGameOption()->GetAudioChannels();
				cfg.AudioSampleRate=GetGameOption()->GetAudioSampleRate();
				cfg.VideoCodec=GetGameOption()->GetVideoFormat();
			}
			m_BandiCaptureLibrary.CheckConfig(&cfg);       // �߸��� ���� �ٷ� ���
			m_BandiCaptureLibrary.SetConfig(&cfg);         // ���� ����

			m_BandiCaptureLibrary.SetMinMaxFPS(30, 60);    // �ּ�, �ִ� ������ ����

			// ����ð��� �̿��ؼ� ���ϸ��� ����ϴ�. 
			TCHAR pathName[MAX_PATH];
			m_BandiCaptureLibrary.MakePathnameByDate(_T("ScreenCapture\\"), _T("SC_"), _T("avi"), pathName, MAX_PATH);

			// ĸó�� �����մϴ�.
			HRESULT hr = m_BandiCaptureLibrary.Start(pathName, NULL, BCAP_MODE_D3D9_SCALE, (LONG_PTR)pd3d9Device);
			if(FAILED(hr))
			{
				switch( hr )
				{
				case BCERR_VIDEO_CAPTURE:
					break;
				case BCERR_AUDIO_CAPTURE:
					{						
						m_BandiCaptureLibrary.GetConfig(&cfg);		   // ���� ������ �ҷ�����
						cfg.AudioCodec = WAVETAG_NULL;				   // ���� ����
						m_BandiCaptureLibrary.CheckConfig(&cfg);       // �߸��� ���� �ٷ� ���
						m_BandiCaptureLibrary.SetConfig(&cfg);         // ���� ����

						//���� ���� �� ������ �Կ� ����
						HRESULT hr = m_BandiCaptureLibrary.Start(pathName, NULL, BCAP_MODE_D3D9_SCALE, (LONG_PTR)pd3d9Device);
						if( FAILED(hr) )
						{
							break;
						}
						if( g_pChatBox != NULL )
						{
							g_pChatBox->AddChatLog( GET_STRING( STR_ID_17528 ), KEGS_CHAT_REQ::CPT_SYSTEM, D3DXCOLOR(1,1,0,1), L"#CFFFF00" );	
							g_pChatBox->AddChatLog( GET_STRING( STR_ID_17529 ), KEGS_CHAT_REQ::CPT_SYSTEM, D3DXCOLOR(1,1,0,1), L"#CFFFF00" );	
						}
						return true;
					}
					break;
				}
				return false;
			}
			else
			{
				if ( g_pChatBox != NULL )
				{
					g_pChatBox->AddChatLog( GET_STRING( STR_ID_14371 ), KEGS_CHAT_REQ::CPT_SYSTEM, D3DXCOLOR(1,1,0,1), L"#CFFFF00" );	
				}
			}
					
		}
	}

	return true;
}

void CX2Main::Bandi_Pause_Restart_VidieCapture(bool bPause)
{
	// �Ͻ� ����, ���� ������ ������ ���� �Ŀ� ���� (IsCapturing = true ����)
	if( TRUE == m_BandiCaptureLibrary.IsCapturing() )
	{
		if( true == bPause && NULL != g_pChatBox )
			g_pChatBox->AddChatLog( GET_STRING( STR_ID_14372 ), KEGS_CHAT_REQ::CPT_SYSTEM, D3DXCOLOR(1,1,0,1), L"#CFFFF00" );
		else
			g_pChatBox->AddChatLog( GET_STRING( STR_ID_14373 ), KEGS_CHAT_REQ::CPT_SYSTEM, D3DXCOLOR(1,1,0,1), L"#CFFFF00" );

		m_BandiCaptureLibrary.Pause(bPause);
	}
}

void CX2Main::Bandi_EndVidioCapture()
{
	if( TRUE == m_BandiCaptureLibrary.IsCapturing() )
	{
		m_BandiCaptureLibrary.Stop();
		if( NULL != g_pChatBox )
			g_pChatBox->AddChatLog( GET_STRING( STR_ID_14374 ), KEGS_CHAT_REQ::CPT_SYSTEM, D3DXCOLOR(1,1,0,1), L"#CFFFF00" );
		m_iOldCaptureFileSize = -1;
	}
}

void CX2Main::Bandi_EnoughDiskSpace(float fElapsedTime)
{
	//������ ĸ�� ���� ���¸� �ľ� �ϴ� �ݵ�ĸ ���̺귯���� work�Լ��� ���׷� 
	//�ϵ��ũ �뷮 ������ �ľ� �� �� ���� ������, �ӽ÷� �ϵ��ũ �뷮 üũ �߰�.

	//���� �Կ� �������� �ƴ϶�� üũ���� ����.
	if( IsCapturing() == false )
		return;

	//üũ ������ �߰�
	m_fFileSizeCheckTime -= fElapsedTime;
	if( m_fFileSizeCheckTime < 0 )
	{
		m_fFileSizeCheckTime = 2.f;
		if( m_iOldCaptureFileSize != m_BandiCaptureLibrary.GetCaptureFileSize())
		{
			m_iOldCaptureFileSize = m_BandiCaptureLibrary.GetCaptureFileSize();
		}
		else
		{	//2������ ������������ ũ���, ���� ������������ ũ�Ⱑ ���ٸ�,
			//��ũ�뷮 �������� �Կ����� �ʾҴٰ� �Ǵ�.
			g_pMain->Bandi_EndVidioCapture();
			g_pChatBox->AddChatLog( GET_STRING(STR_ID_17540), KEGS_CHAT_REQ::CPT_SYSTEM, D3DXCOLOR(1,1,0,1), L"#CFFFF00" );

#ifdef ACTIVE_KOG_GAME_PERFORMANCE_CHECK
			KOGGamePerformanceCheck::GetInstance()->Resume();
#endif//ACTIVE_KOG_GAME_PERFORMANCE_CHECK

		}
	}
	return;
}

#endif BANDICAM_RECORDING
//}}

//{{ 2011.9.16	����ȣ  �ݵ�ķ ��ũ���� ĸ�� ����
#ifdef BANDICAM_CAPTURE
bool CX2Main::Bandi_ScreenCapture()
{
	if( m_fCaptureDelay > 0.0f )
		return false;

	m_fCaptureDelay = 3.0f;

	// �⺻ ���� ���� ���� ������ default ���� //
	HRESULT hr = BCERR_FILE_CREATE; 

	IDirect3DDevice9* pd3d9Device = DXUTGetD3DDevice();

	if( NULL == pd3d9Device )
		return false;

	// ĸó�� �����մϴ�.	
	// ���� ����� DLL �� �ε��ϰ� ���� �ν��Ͻ��� �����մϴ�. 
	if(m_BandiCaptureLibrary.IsCreated()==FALSE)
	{
		// ���н� DLL�� .H������ ������ �������� ���ε��� Ȯ���� ���ñ� �ٶ��ϴ�.
		if(FAILED(m_BandiCaptureLibrary.Create(BANDICAP_RELEASE_DLL_FILE_NAME)))
			return false;

		// ������ ���� ������ ���ڵ��� ������ ��ܿ� �ΰ��� ǥ�õ˴ϴ�. 
		// ���ı����� �߱޹��� Ű�� ����ϰų�, Ȩ���������� ��������� ������ Ű�� ����ؼ� ������ �޽��ϴ�. 
		if(FAILED(m_BandiCaptureLibrary.Verify("KOG_ELSWORD_20110825", "675a5521")))
			return false;

	}

	if(m_BandiCaptureLibrary.IsCreated())
	{
		TCHAR pathName[MAX_PATH];
		m_BandiCaptureLibrary.MakePathnameByDate(_T("ScreenCapture\\"), _T("SC_"), _T("jpg"), pathName, MAX_PATH);

		hr = m_BandiCaptureLibrary.CaptureImage(pathName, BCAP_IMAGE_JPG , 80, BCAP_MODE_D3D9, true,(LONG_PTR)pd3d9Device);

		if(FAILED(hr))
			return false;
	}


	return true;
}
#endif BANDICAM_CAPTURE
//}}
#ifdef SERVER_GROUP_UI_ADVANCED
KServerGroupInfo CX2Main::GetPickedChannelServer() const
{
	map<int, KServerGroupInfo>::iterator iter = g_pInstanceData->m_mapServerGroupInfo.find(m_iPickedChannelServerIPIndex);
	if(iter == g_pInstanceData->m_mapServerGroupInfo.end())
		return m_DefaultChannelServerInfo;

	return iter->second;
}
#endif SERVER_GROUP_UI_ADVANCED

#ifdef LOADING_ANIMATION_TEST
	CX2LoadingAnimation::CX2LoadingAnimation()
	{
		m_bShow = false;
		m_iCurrTextureIndex = 0;
		m_timerAni.restart();

		m_vPos = D3DXVECTOR2( 210, 620 );

		// fix!!
		CKTDXDeviceTexture* pTexture = NULL;
		pTexture = g_pKTDXApp->GetDeviceManager()->OpenTexture( L"DLG_Repair_Cursor.tga" );
		if( NULL != pTexture )
			m_vecpTextureToAnimate.push_back( pTexture );

		pTexture = g_pKTDXApp->GetDeviceManager()->OpenTexture( L"DLG_Reset_Skill_Cursor.tga" );
		if( NULL != pTexture )
			m_vecpTextureToAnimate.push_back( pTexture );

		pTexture = g_pKTDXApp->GetDeviceManager()->OpenTexture( L"DLG_Sell_Cursor.tga" );
		if( NULL != pTexture )
			m_vecpTextureToAnimate.push_back( pTexture );

	}

	CX2LoadingAnimation::~CX2LoadingAnimation()
	{
		for( UINT i=0; i<m_vecpTextureToAnimate.size(); i++ )
		{
			CKTDXDeviceTexture* pTexture = m_vecpTextureToAnimate[i];
			SAFE_CLOSE( pTexture );
		}
		m_vecpTextureToAnimate.clear();
	}

	void CX2LoadingAnimation::OnFrameMove( double fTime, float fElapsedTime )
	{
		if( false == m_bShow )
			return; 

		if( m_vecpTextureToAnimate.size() == 0 )
			return;

		const float MAGIC_TIME_PER_FRAME = 0.3f;
		if( m_timerAni.elapsed() > MAGIC_TIME_PER_FRAME )
		{
			m_vPos.x += 10.f;
			if( m_vPos.x > 760.f )
			{
				m_vPos.x = 210.f;
			}
			m_timerAni.restart();
			m_iCurrTextureIndex++;

			if( m_iCurrTextureIndex > (int)m_vecpTextureToAnimate.size()-1 )
			{
				m_iCurrTextureIndex = 0;
			}
		}
	}

	void CX2LoadingAnimation::OnFrameRender()
	{
		if( false == m_bShow )
			return;

		if( m_vecpTextureToAnimate.size() == 0 )
			return;

		CKTDXDeviceTexture* pTexture = m_vecpTextureToAnimate[m_iCurrTextureIndex];
		if( NULL != pTexture )
		{
			pTexture->Draw( (int)m_vPos.x, (int)m_vPos.y, 64, 64 );
		}
	}

#endif LOADING_ANIMATION_TEST


bool CX2Main::GetWallPaperDirectory( char * pszDir, int iDirSize )
{
	LPITEMIDLIST  psttList;

	if( SHGetSpecialFolderLocation( NULL, CSIDL_DESKTOP, &psttList ) == S_OK )
	{
		LPMALLOC  psttMalloc;
		
		SHGetPathFromIDListA( psttList, pszDir );
		
		SHGetMalloc( &psttMalloc );
		psttMalloc->Free( psttList );
		psttMalloc->Release();
		
		return true;
	}

	return false;
}

#ifdef ADD_COLLECT_CLIENT_INFO

bool CX2Main::SendHackInfo1( unsigned short usType, const char *strHackInfo, bool bExit, bool bOnlyOnce )
{
	VIRTUALIZER1_START
	if( strHackInfo == NULL )
		return false;

	if( usType == (unsigned short)ANTIHACKING_ID::ANTIHACKING_ID_START || usType >= (unsigned short)ANTIHACKING_ID::ANTIHACKING_ID_END )
		return false;

	if( g_pInstanceData != NULL )
	{		
		if( bOnlyOnce == true && g_pInstanceData->g_AntiHackingList[ usType ] >= 1 )
		{
			return false;
		}
		g_pInstanceData->g_AntiHackingList[ usType ] += 1;
	}
	
	if( m_pCollectHackInfo != NULL )
		m_pCollectHackInfo->EnQueue( ECL_CRASH_INFO_REQ, NULL, true, usType, strHackInfo, bExit );

	m_iSendHackInfo = 1;

	VIRTUALIZER1_END
	return true;
}
bool CX2Main::SendHackInfo2( unsigned short usType, const char *strHackInfo, bool bExit, bool bOnlyOnce )
{
	VIRTUALIZER2_START
	if( usType == (unsigned short)ANTIHACKING_ID::ANTIHACKING_ID_START || usType >= (unsigned short)ANTIHACKING_ID::ANTIHACKING_ID_END )
		return false;
	if( strHackInfo == NULL )
		return false;

	if( g_pInstanceData != NULL )
	{		
		if( bOnlyOnce == true && g_pInstanceData->g_AntiHackingList[ usType ] >= 1 )
		{
			return false;
		}
		g_pInstanceData->g_AntiHackingList[ usType ] += 1;
	}
	
	if( m_pCollectHackInfo != NULL )
		m_pCollectHackInfo->EnQueue( ECL_CRASH_INFO_REQ, NULL, true, usType, strHackInfo, bExit );

	m_iSendHackInfo = 2;

	VIRTUALIZER2_END
	return true;
}
bool CX2Main::SendHackInfo3( unsigned short usType, const char *strHackInfo, bool bExit, bool bOnlyOnce )
{
	VIRTUALIZER3_START
		
	if( strHackInfo == NULL )
		return false;
	if( usType == (unsigned short)ANTIHACKING_ID::ANTIHACKING_ID_START || usType >= (unsigned short)ANTIHACKING_ID::ANTIHACKING_ID_END )
		return false;

	if( g_pInstanceData != NULL )
	{		
		if( bOnlyOnce == true && g_pInstanceData->g_AntiHackingList[ usType ] >= 1 )
		{
			return false;
		}
		g_pInstanceData->g_AntiHackingList[ usType ] += 1;
	}

	if( m_pCollectHackInfo != NULL )
		m_pCollectHackInfo->EnQueue( ECL_CRASH_INFO_REQ, NULL, true, usType, strHackInfo, bExit );

	m_iSendHackInfo = 3;

	VIRTUALIZER3_END
	return true;
}
bool CX2Main::SendHackInfo4( unsigned short usType, const char *strHackInfo, bool bExit, bool bOnlyOnce )
{
	VIRTUALIZER4_START

	if( strHackInfo == NULL )
		return false;
	if( usType == (unsigned short)ANTIHACKING_ID::ANTIHACKING_ID_START || usType >= (unsigned short)ANTIHACKING_ID::ANTIHACKING_ID_END )
		return false;

	if( g_pInstanceData != NULL )
	{		
		if( bOnlyOnce == true && g_pInstanceData->g_AntiHackingList[ usType ] >= 1 )
		{
			return false;
		}
		g_pInstanceData->g_AntiHackingList[ usType ] += 1;
	}

	if( m_pCollectHackInfo != NULL )
		m_pCollectHackInfo->EnQueue( ECL_CRASH_INFO_REQ, NULL, true, usType, strHackInfo, bExit );

	m_iSendHackInfo = 4;

	VIRTUALIZER4_END
	return true;
}
bool CX2Main::SendHackInfo5( unsigned short usType, const char *strHackInfo, bool bExit, bool bOnlyOnce )
{
	VIRTUALIZER5_START

	if( strHackInfo == NULL )
		return false;
	if( usType == (unsigned short)ANTIHACKING_ID::ANTIHACKING_ID_START || usType >= (unsigned short)ANTIHACKING_ID::ANTIHACKING_ID_END )
		return false;

	if( g_pInstanceData != NULL )
	{		
		if( bOnlyOnce == true && g_pInstanceData->g_AntiHackingList[ usType ] >= 1 )
		{
			return false;
		}
		g_pInstanceData->g_AntiHackingList[ usType ] += 1;
	}

	if( m_pCollectHackInfo != NULL )
		m_pCollectHackInfo->EnQueue( ECL_CRASH_INFO_REQ, NULL, true, usType, strHackInfo, bExit );

	m_iSendHackInfo = 5;

	VIRTUALIZER5_END
	return true;
}
#endif

#ifdef CHECK_PLAY_TIME_INFORMATION
void CX2Main::CreatePlayTimeInformation()						//�ȳ� ���̾�α� �߰�
{
	if( NULL != m_pDlgPlayTimeInformation )
		DeletePlayTimeInformation();

	m_bProcessPlayTimeInformation = false;
	m_fPlayTimeInfoShowTime		  = 8.f;

	m_pDlgPlayTimeInformation = new CKTDGUIDialog( g_pMain->GetNowState(), L"DLG_POPUP_SHUTDOWN.lua" );

	if( NULL == m_pDlgPlayTimeInformation )
		return;

	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDlgPlayTimeInformation );

	//���� �̿� @1�ð��� ����߽��ϴ�.\n������ ���� �̿��� �������� �ϻ� ��Ȱ��\n������ �� �� �ֽ��ϴ�.\n�޽� �� �ٽ� �̿����ֽø� �����ϰڽ��ϴ�.
	CKTDGUIStatic* pStatic = ( CKTDGUIStatic*)m_pDlgPlayTimeInformation ->GetControl( L"SHUTDOWN_Text" );
	pStatic->GetString(0)->msg = GET_REPLACED_STRING( ( STR_ID_17723, "i", m_iGameHour ) );
}

void CX2Main::CreateShutDownInformation()						//�˴ٿ� ���̾�α� �߰�
{
	if( NULL != m_pDlgShutDownInformation )
		DeleteShutDownInformation();

	m_bProcessShutDownInformation = false;
	m_fShutDownInfoShowTime		  = 8.f;

	m_pDlgShutDownInformation = new CKTDGUIDialog( g_pMain->GetNowState(), L"DLG_POPUP_SHUTDOWN.lua" );

	if( NULL == m_pDlgShutDownInformation )
		return;

	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDlgShutDownInformation );

	//ȸ������ �� 16�� �̸����� Ȯ�εǾ� �� 12�ú��� ���� 6�ñ��� ������ �̿��Ͻ� �� �����ϴ�. �̿뿡 ������ ��Ź�帳�ϴ�.
	//ȸ������ �� 18�� �̸��� �˴ٿ��� ���������� @1�� �ĺ��� ��� ���� ������ �̿��Ͻ� �� �����ϴ�. ������ �˴ٿ����� ���� Ȥ�� ���� �븮�ο� ���� �ð��� �����Ͻ� �� �ֽ��ϴ�.
	//@1�� �� �����̿��� ���ܵ˴ϴ�.
	CKTDGUIStatic* pStatic = ( CKTDGUIStatic*)m_pDlgShutDownInformation ->GetControl( L"SHUTDOWN_Text" );
	pStatic->GetString(0)->msg = m_wstrShutDownString;
}

void CX2Main::FadePlayTimeInformation( float fShowTime )		//�ȳ� ���̾�α� Fade ȿ��
{
	if( NULL != m_pDlgPlayTimeInformation )
	{
		if( 7.5f < fShowTime )		//Fade In
			m_pDlgPlayTimeInformation->Move( m_pDlgPlayTimeInformation->GetPos(), D3DXCOLOR( 1.f, 1.f, 1.f, 1.f ), 0.5f, true );
		else if( 0.5f > fShowTime )	//Fade Out
			m_pDlgPlayTimeInformation->Move( m_pDlgPlayTimeInformation->GetPos(), D3DXCOLOR( 1.f, 1.f, 1.f, 0.f ), 0.5f, true );
	}
}

void CX2Main::DeletePlayTimeInformation()						//�ȳ� ���̾�α� ����
{
	if( NULL == m_pDlgPlayTimeInformation )
		return;

	SAFE_DELETE_DIALOG( m_pDlgPlayTimeInformation );
	m_pDlgPlayTimeInformation = NULL;
}

void CX2Main::FadeShutDownInformation( float fShowTime )		//�˴ٿ� ���̾�α� Fade ȿ��
{
	if( NULL != m_pDlgShutDownInformation )
	{
		if( 7.5f < fShowTime )		//Fade In
			m_pDlgShutDownInformation->Move( m_pDlgShutDownInformation->GetPos(), D3DXCOLOR( 1.f, 1.f, 1.f, 1.f ), 0.5f, true );
		else if( 0.5f > fShowTime )	//Fade Out
			m_pDlgShutDownInformation->Move( m_pDlgShutDownInformation->GetPos(), D3DXCOLOR( 1.f, 1.f, 1.f, 0.f ), 0.5f, true );
	}
}

void CX2Main::DeleteShutDownInformation()						//�˴ٿ� ���̾�α� ����
{
	if( NULL == m_pDlgShutDownInformation )
		return;

	SAFE_DELETE_DIALOG( m_pDlgShutDownInformation );
	m_pDlgShutDownInformation = NULL;
}
#endif CHECK_PLAY_TIME_INFORMATION

/** @function : ChangeCashSkillPointDate
	@brief : �׳�ý� �ູ ���� ��¥ ����
	@param : ����� ��, ��, ��, �ð�, �� ����
*/
void CX2Main::ChangeCashSkillPointDate( const int iYear_, const int iMonth_, const int iDay_, const int iHour_, const int iMinute_ )
{
	g_pData->GetSkillTree()->Hander_EGS_ADMIN_CASH_SKILL_POINT_DATE_CHANGE_REQ( iYear_, iMonth_, iDay_, iHour_, iMinute_ );
}
/** @function : SendCheckMail
	@brief : �ǽɵǴ� �ڵ� �κ��� ����������� üũ�ϱ� ���� �뵵�� ���� ���� �Լ�
	@param : �޴� ��� �����ּ�, ����, ����
*/
void CX2Main::SendCheckMail( const char* pRecipient_, const char* pSubject_, const char* pMessageBody_ )
{
	if ( NULL == g_pX2SMTPMail )
		g_pX2SMTPMail = new CX2SMTPMail;
	
	if(g_pX2SMTPMail->SMTPGetLastError() != CX2SMTPMail::CSMTP_NO_ERROR)
	{
		// Unable to initialize winsock2
		return;
	}

	g_pX2SMTPMail->SMTPSetServer( m_strMailAddress.c_str(), 25 );
	g_pX2SMTPMail->SMTPSetLogin("escrash");            // ���� ����
	g_pX2SMTPMail->SMTPSetPassword("@Els.123");
	g_pX2SMTPMail->SMTPSetSenderName( "Check Mail" );
	g_pX2SMTPMail->SMTPSetSenderMail("itexpertkim@kog.co.kr");
	g_pX2SMTPMail->SMTPSetReplyTo("itexpertkim@kog.co.kr"); // ȸ�� �ּ�

	string strSubject = "[ES_KR_CLIENT_CHECK_MAIL]";
	strSubject += pSubject_;
	g_pX2SMTPMail->SMTPSetSubject( strSubject.c_str() );
	g_pX2SMTPMail->SMTPAddRecipient( pRecipient_ );
	g_pX2SMTPMail->SMTPSetXPriority(CX2SMTPMail::XPRIORITY_NORMAL);
	g_pX2SMTPMail->SMTPSetXMailer("The Bat! (v3.02) Professional");
	g_pX2SMTPMail->SMTPSetMessageBody( pMessageBody_ );

	if( g_pX2SMTPMail->SMTPSend() )
	{
		//The mail was send successfully
	}
	else
	{
		
		//Unable to send the mail
	}

	g_pX2SMTPMail->SMTPClearRecipient();
}



#ifdef NEXON_QA_CHEAT_REQ
void CX2Main::InventoryClear( int iSelect )
{
	if( NULL == g_pData )
		return;

	CX2Inventory::SORT_TYPE SortType;

	switch(iSelect)
	{
	case 0:
		return;
	case 1:
		SortType = CX2Inventory::ST_EQUIP;
		break;
	case 2:
		SortType = CX2Inventory::ST_ACCESSORY;
		break;
	case 3:
		SortType = CX2Inventory::ST_QUICK_SLOT;
		break;
	case 4:
		SortType = CX2Inventory::ST_MATERIAL;
		break;
	case 5:
		SortType = CX2Inventory::ST_QUEST;
		break;
	case 6:
		SortType = CX2Inventory::ST_SPECIAL;
		break;
	case 7:
		SortType = CX2Inventory::ST_AVARTA;
		break;
	case 8:
		SortType = CX2Inventory::ST_PET;
		break;
	default:
		return;
	}

	if( g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_SPECIAL_USER )
	{
		CX2Inventory* pInventory = g_pData->GetMyUser()->GetSelectUnit()->GetInventory();

		for( int i = 0; i < pInventory->GetItemMaxNum( SortType ); i++ )
		{
			CX2Item* pItem = pInventory->GetItem( SortType, i );
			if( NULL == pItem || NULL == pItem->GetItemData() )
				continue;

			KEGS_DELETE_ITEM_REQ kEGS_DELETE_ITEM_REQ;

			kEGS_DELETE_ITEM_REQ.m_iItemUID = pItem->GetUID();
			kEGS_DELETE_ITEM_REQ.m_iQuantity = pItem->GetItemData()->m_Quantity;

			g_pData->GetServerProtocol()->SendPacket( EGS_DELETE_ITEM_REQ, kEGS_DELETE_ITEM_REQ );
			g_pMain->AddServerPacket( EGS_DELETE_ITEM_ACK );

		}
		
	}

}
#endif //NEXON_QA_CHEAT_REQ


#ifdef ACTIVE_KOG_GAME_PERFORMANCE_CHECK
void CX2Main::OccurLag_LUA( float fLagTime, int iLagType )
{
#ifndef _SERVICE_
	if( KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->m_pfnGetGameSpace )
	{
		int iGameSpace = KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->m_pfnGetGameSpace();

		if( KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->m_pfnIsValidGameSpace_Lag( iGameSpace ) )
		{
			KOGGamePerformanceCheck::GetInstance()->GetLagCheckMgr()->SetLagTestActive( true );
			KOGGamePerformanceCheck::GetInstance()->GetLagCheckMgr()->SetLagTestTime( fLagTime );
			KOGGamePerformanceCheck::GetInstance()->GetLagCheckMgr()->SetLagTestType( iLagType );
		}
	}
#endif//_SERVICE_
}

void CX2Main::_InitKOGGamePerformanceCheck()
{
	//��� ������, �� ���� ����
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetGameSpace( KOGGamePerformanceCheckPort_Function::GetGameSpace );

	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_IsGameSpaceVillage( KOGGamePerformanceCheckPort_Function::IsGameSpaceVillage );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_IsGameSpaceField( KOGGamePerformanceCheckPort_Function::IsGameSpaceField );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_IsGameSpaceDungeon( KOGGamePerformanceCheckPort_Function::IsGameSpaceDungeon );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_IsGameSpacePvP( KOGGamePerformanceCheckPort_Function::IsGameSpacePvP );

	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetVillagePartyType( KOGGamePerformanceCheckPort_Function::GetVillagePartyType );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetVillagePartyUserNum( KOGGamePerformanceCheckPort_Function::GetVillagePartyUserNum );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetVillageMapID( KOGGamePerformanceCheckPort_Function::GetVillageMapID );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetVillageUserNum( KOGGamePerformanceCheckPort_Function::GetVillageUserNum );

	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetFieldPartyType( KOGGamePerformanceCheckPort_Function::GetFieldPartyType );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetFieldPartyUserNum( KOGGamePerformanceCheckPort_Function::GetFieldPartyUserNum );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetFieldMapID( KOGGamePerformanceCheckPort_Function::GetFieldMapID );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetFieldUserNum( KOGGamePerformanceCheckPort_Function::GetFieldUserNum );

	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetDungeonPartyType( KOGGamePerformanceCheckPort_Function::GetDungeonPartyType );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetDungeonPartyUserNum( KOGGamePerformanceCheckPort_Function::GetDungeonPartyUserNum );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetDungeonMapID( KOGGamePerformanceCheckPort_Function::GetDungeonMapID );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetDungeonDifficulty( KOGGamePerformanceCheckPort_Function::GetDungeonDifficulty );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetDungeonStage( KOGGamePerformanceCheckPort_Function::GetDungeonStage );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_IsDungeonEscape( KOGGamePerformanceCheckPort_Function::IsDungeonEscape );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetDungeonBlock( KOGGamePerformanceCheckPort_Function::GetDungeonBlock );

	//KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetPvPPartyType( KOGGamePerformanceCheckPort_Function::GetPvPPartyType );
	//KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetPvPPartyUserNum( KOGGamePerformanceCheckPort_Function::GetPvPPartyUserNum );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetPvPMapID( KOGGamePerformanceCheckPort_Function::GetPvPMapID );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetPvPUserNum( KOGGamePerformanceCheckPort_Function::GetPvPUserNum );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetPvPRuleType( KOGGamePerformanceCheckPort_Function::GetPvPRuleType );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_IsPvPEscape( KOGGamePerformanceCheckPort_Function::IsPvPEscape );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetPvPRound( KOGGamePerformanceCheckPort_Function::GetPvPRound );
	//KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetPvPObserverNum( KOGGamePerformanceCheckPort_Function::GetPvPObserverNum );

	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_GetFramePerSecond( KOGGamePerformanceCheckPort_Function::GetFramePerSecond );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_ResetFramePerSecond( KOGGamePerformanceCheckPort_Function::ResetFramePerSecond );

	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_IsGameHost( KOGGamePerformanceCheckPort_Function::IsGameHost );

	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_SendServer_Village( KOGGamePerformanceCheckPort_Function::SendServer_Village );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_SendServer_Field( KOGGamePerformanceCheckPort_Function::SendServer_Field );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_SendServer_Dungeon( KOGGamePerformanceCheckPort_Function::SendServer_Dungeon );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_SendServer_PvP( KOGGamePerformanceCheckPort_Function::SendServer_PvP );

	//��� ������ ��
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_IsValidGameSpace_AvgFps( KOGGamePerformanceCheckPort_Function::IsValidGameSpace_AvgFps );

	//�� ���� ��
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_IsUnKnownLagType_Lag( KOGGamePerformanceCheckPort_Function::IsUnKnownLagType_Lag );
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_IsValidGameSpace_Lag( KOGGamePerformanceCheckPort_Function::IsValidGameSpace_Lag );

	//ping ���� ��
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_IsValidGameSpace_Ping( KOGGamePerformanceCheckPort_Function::IsValidGameSpace_Ping );

	//UDP �������� ��
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_IsValidGameSpace_UDPTrans( KOGGamePerformanceCheckPort_Function::IsValidGameSpace_UDPTrans );

	//memory usage ���� ��
	KOGGamePerformanceCheck_CallbackFuncTable::GetInstance()->RegisterCallbackFunc_IsValidGameSpace_MemUsage( KOGGamePerformanceCheckPort_Function::IsValidGameSpace_MemUsage );

	//
	KOGGamePerformanceCheck::GetInstance()->Init( 6.0f, 5, 6000, 10, 6.0f, 200, eLagCheckType_Max );
}
#endif//ACTIVE_KOG_GAME_PERFORMANCE_CHECK

#ifdef ACTIVE_KOG_GAME_PERFORMANCE_CHECK
void CX2Main::ForceHost_LUA( bool bEnable )
{
#ifndef _SERVICE_
	if ( NULL != g_pX2Room )
		g_pX2Room->ForceHostForGameEdit( bEnable );
#endif//_SERVICE_
}

void CX2Main::SwitchConnect_LUA( int iMode )
{
#ifndef _SERVICE_
	if( GetX2Data()->GetGameUDP() )
		GetX2Data()->GetGameUDP()->SetForceConnectMode( CKTDNUDP::EForceConnectMode( iMode ) );
#endif//_SERVICE_
}
#endif//ACTIVE_KOG_GAME_PERFORMANCE_CHECK

#ifdef UDP_PACKET_ANALYSIS_LOG
void CX2Main::UdpPacketAnalysisLog_LUA( bool bEnable ) 
{ 
#ifndef _SERVICE_
	GetX2Data()->GetGameUDP()->SetEnablePacketAnalysisLog( bEnable );
#endif//_SERVICE_
}
#endif//UDP_PACKET_ANALYSIS_LOG

#ifdef X2OPTIMIZE_UDP_RELAY_OVERHEAD_TEST
void CX2Main::UdpRelayOverHeadTest_LUA( int iRoomNum )
{
#ifndef _SERVICE_
	g_pData->GetGameUDP()->ClearUdpRelayTestRooms();
	if( iRoomNum > 0 )
	{
		for( int i = 0; i < iRoomNum; i++ )
			g_pData->GetGameUDP()->AddUdpRelayTestRooms( 7 );
	}
#endif//_SERVICE_
}
#endif//X2OPTIMIZE_UDP_RELAY_OVERHEAD_TEST

#ifdef SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
void CX2Main::UdpPacketOverlapTest_LUA( bool bEnable )
{
#ifndef _SERVICE_
	m_bUdpPacketOverlap = bEnable;
#endif//_SERVICE_
}
#endif//SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK


#ifdef X2OPTIMIZE_ONE_SIMUL_ONE_RENDER_TEST
void CX2Main::OneSimulOneRenderTest_LUA( bool bEnable )
{
#ifndef _SERVICE_
	g_pKTDXApp->SetEnableOneSimulOneRenderTest( bEnable );
#endif//_SERVICE_
}
#endif//X2OPTIMIZE_ONE_SIMUL_ONE_RENDER_TEST


void    CX2Main::SetUDPMode_LUA( const char* pszGameType, const char* pszUDPMode )
{
#ifdef  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
    if ( false == ( pszGameType != NULL && pszGameType[0] != NULL
        && pszUDPMode != NULL && pszUDPMode[0] != NULL ) )
        return;

    const char* apszGameType[] = 
    { 
        "GT_NONE",
        "GT_PVP",
        "GT_DUNGEON",
        "GT_BATTLE_FIELD",
    };
    const char* apszUDPMode[] =
    {
        "UDP_MODE_DEFAULT",
        "UDP_MODE_P2P",
        "UDP_MODE_RELAY",
    };
    BOOST_STATIC_ASSERT( ARRAY_SIZE( apszGameType ) == CX2Game::GT_NUMS );
    BOOST_STATIC_ASSERT( ARRAY_SIZE( apszUDPMode ) == CKTDNUDP::FORCE_CONNECT_MODE_NUMS );

    CX2Game::GAME_TYPE          eGameType = CX2Game::GT_NONE;
    CKTDNUDP::EForceConnectMode eUDPMode = CKTDNUDP::FORCE_CONNECT_MODE_DEFAULT;

    for( int i = 0; i <  CX2Game::GT_NUMS; ++i )
    {
        if ( stricmp( pszGameType, apszGameType[ i ] ) == 0 )
        {
            eGameType = CX2Game::GAME_TYPE( i );
            goto udpmode;
        }
    }//for
    return;
udpmode:
    for( int i = 0; i < CKTDNUDP::FORCE_CONNECT_MODE_NUMS; ++i )
    {
        if ( stricmp( pszUDPMode, apszUDPMode[ i ] ) == 0 )
        {
            eUDPMode = CKTDNUDP::EForceConnectMode( i );
            goto setmode;
        }
    }//for
    return;
setmode:

    m_aeUDPMode[ eGameType ] = eUDPMode;

#endif  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
}

#ifdef  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK

CKTDNUDP::EForceConnectMode CX2Main::GetUDPMode( CX2Game::GAME_TYPE eGameType )
{
    if ( eGameType < CX2Game::GAME_TYPE( 0 ) || eGameType >= CX2Game::GT_NUMS )
        return CKTDNUDP::FORCE_CONNECT_MODE_DEFAULT;
    return  m_aeUDPMode[ eGameType ];
}


void    CX2Main::SetUDPMode( CX2Game::GAME_TYPE eGameType, CKTDNUDP::EForceConnectMode eUDPMode )
{
    if ( eGameType < CX2Game::GAME_TYPE( 0 ) || eGameType >= CX2Game::GT_NUMS )
        return;
    if ( eUDPMode < CKTDNUDP::EForceConnectMode( 0 ) || eUDPMode >= CKTDNUDP::FORCE_CONNECT_MODE_NUMS )
        return;

    m_aeUDPMode[ eGameType ] = eUDPMode;
}

#endif  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK

#ifdef CHECK_KOM_FILE_ON_LOAD
#ifdef SERV_KOM_FILE_CHECK_ADVANCED
void CX2Main::GetCheckKomFileList()
{
	std::vector< pair<string, string> > vecChangeCheckKom;
	GetChangeCheckKom( vecChangeCheckKom );
	if( vecChangeCheckKom.empty() )
	{
		vecChangeCheckKom.clear();
		SetChangeCheckKom( vecChangeCheckKom );
		SetChangeCheckKom( m_vecCheckKom );
		m_fGetCheckKomTime = 0.0f;
	}
}

void CX2Main::SetChangeCheckKom( std::vector< pair<string, string> > vecChangeCheckKom )
{
	KLocker lock( m_csData_vec );
	m_vecChangeCheckKom = vecChangeCheckKom;
}

void CX2Main::GetChangeCheckKom( std::vector< pair<string, string> >& vecChangeCheckKom )
{
	KLocker lock( m_csData_vec );
	vecChangeCheckKom = m_vecChangeCheckKom;
}

void CX2Main::Handler_EGS_KOM_FILE_CHECK_LOG_REQ( const std::wstring wstrInvalidKomName )
{
	KEGS_KOM_FILE_CHECK_LOG_REQ kPacket;
	kPacket.m_wstrInvalidKomName = wstrInvalidKomName;

	g_pData->GetServerProtocol()->SendPacket( EGS_KOM_FILE_CHECK_LOG_REQ, kPacket );
	g_pMain->AddServerPacket( EGS_KOM_FILE_CHECK_LOG_ACK );
}

bool CX2Main::Handler_EGS_KOM_FILE_CHECK_LOG_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_KOM_FILE_CHECK_LOG_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	if( g_pMain->DeleteServerPacket( EGS_KOM_FILE_CHECK_LOG_ACK ) == true )
	{
		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{
			wstring wstrErrorMessage = GET_REPLACED_STRING( ( STR_ID_28981, "L", kEvent.m_wstrInvalidKomName ) );
			g_pMain->KTDGUIMsgBox( D3DXVECTOR2(250,300), wstrErrorMessage.c_str() , g_pMain->GetNowState() );

			// Ŭ���̾�Ʈ �޽���ť�� ���� �޽��� �߰�
			g_pMain->SetQuitType( NetError::ERR_CLIENT_QUIT_04 );
			g_pMain->SendQuitMsgToServer();
		}
	}

	return true;
}
#endif SERV_KOM_FILE_CHECK_ADVANCED
#endif CHECK_KOM_FILE_ON_LOAD

#ifdef SEND_HACKING_PROCESS_CHECK_RESULT 
/** @function : SetSendHackCheckResultInfo
	@brief : ��ŷ ���μ��� �˻翡 ���� �ɷ��� ��� ������ ���� ����
*/
void CX2Main::Send_EGS_HACKING_USER_PROCESS_NAME_RECORD_NOT( char cHackInfoFlag_, const wstring& wstrHackingName )
{
	KEGS_HACKING_USER_PROCESS_NAME_RECORD_NOT kPacket;
	kPacket.m_byteType = 0;
	switch ( cHackInfoFlag_ )
	{
	case ANTIHACKING_ID::ANTIHACKING_FILGER_01:	kPacket.m_byteType = SEnum::HUCT_CLIENT_HACKING_PROCESS; break;
	case ANTIHACKING_ID::ANTIHACKING_FILGER_02: kPacket.m_byteType = SEnum::HUCT_CLIENT_HACKING_TEXT; break;
	case ANTIHACKING_ID::ANTIHACKING_FILGER_07: kPacket.m_byteType = SEnum::HUCT_CLIENT_HACKING_MODULE; break;
	case ANTIHACKING_ID::ANTIHACKING_FILGER_10: kPacket.m_byteType = SEnum::HUCT_CLIENT_HACKING_DLL_MANAGER; break;
	case ANTIHACKING_ID::ANTIHACKING_FILGER_11: kPacket.m_byteType = SEnum::HUCT_CLIENT_HACKING_ZOMBIE_PROCESS; break;
	case ANTIHACKING_ID::ANTIHACKING_FILGER_27: kPacket.m_byteType = SEnum::HUCT_CLIENT_HACKING_THREAD_TITLE; break;
	default:  break;
	}

	if( 0 == kPacket.m_byteType )
		return;
	else
	{
		kPacket.m_wstrReason = wstrHackingName;
		g_pData->GetServerProtocol()->SendPacket( EGS_HACKING_USER_PROCESS_NAME_RECORD_NOT, kPacket );
	}

}
#endif // SEND_HACKING_PROCESS_CHECK_RESULT
