#ifndef _KTDX_H_
#define _KTDX_H_

#ifndef LINE1
#define LINE1(x_) #x_
#endif // #ifndef LINE1
#ifndef LINE
#define LINE(x_) LINE1(x_)
#endif // #ifndef LINE

#ifndef TODO
#define TODO(msg_) message ( __FILE__ "(" LINE(__LINE__)  "): [TODO] " msg_ )
#endif // #ifndef TODO
#ifndef NOTE
#define NOTE(msg_) message ( __FILE__ "(" LINE(__LINE__)  "): [NOTE] " msg_ )
#endif // #ifndef NOTE


//////////////////////////////////////////////////////////////////////////
// 2009-06-01, dmlee, 
// 항상 포함되어야 하는 macro는 _ALWAYS_ 안쪽에 정의해서 사용
// 본섭 패치 버전은 _SERVICE_
// 오픈테섭 패치 버전은 _SERVICE_, _OPEN_TEST_
// 해외 패치 버전은 _FOREIGN_
// 테스트 코드를 포함한 사내 패치 버전은 _IN_HOUSE_
// 테스크 코드를 제외한 사내 패치 버전은 _IN_HOUSE_SERVICE_READY_QA_
//////////////////////////////////////////////////////////////////////////





//#define NVPerfHUD
//#define PROFILE_BUILD // Uncomment this line to enable profiling


#define _ALWAYS_
//#define _SERVICE_MANUAL_LOGIN_ // 본섭에 VC++에서 디버깅으로 접속할 때
//#define _SERVICE_
//#define _OPEN_TEST_
//#define _OPEN_TEST_2_
//#define _NEXON_KR_
//#define _GAMANIA_TW_
//#define _IN_HOUSE_
//#define _IN_HOUSE_SERVICE_READY_QA_

#ifdef _USE_BOOST_CONST_STRING_

#define BOOST_CONST_CSTRING             boost::const_cstring
#define BOOST_CONST_STRING_CREF( x )    boost::cref( x )

#else

#define BOOST_CONST_CSTRING             std::string
#define BOOST_CONST_STRING_CREF( x )    ( x )

#endif

#if defined( _IN_HOUSE_SERVICE_READY_QA_ )
	#pragma NOTE( "ktdx.h 의 _IN_HOUSE_SERVICE_READY_QA_ 실행!" )
	#define NO_GAMEGUARD
	#define MASS_FILE_FIRST			// 실제 파일보다 kom파일에서 먼저 찾기
	#define _ENCRIPT_SCRIPT_

	// 아이템 치트로 사는 인터페이스
	#define ITEM_CHEAT_POPUP_TEST
	// 마을에서 개발자는 캐릭터 빨리 달릴 수 있게
	#define VILLAGE_FLASH_MAN_DEV_ONLY
	// 운영자용 해당 카데고리 아이템 다 팔아버리기 기능. 테스트 용도로 급히 제작. 안정성 보장 못함!!
	#define AUTH_AUTO_SELL_ITEM	 

	// 데미지 로그
//	#define ADD_DAMAGE_LOG
	
	// 사내 테스트용 편의 단축키 추가
//	#define ADD_SHORT_CUT_KEY_IN_HOUSE

	// 다 팔기, 다 버리기
	#define AUTH_AUTO_SELL_ITEM
	#define JUNK_AVATAR
	// 운영자용 랜덤아이템 자동열기 기능
//	#define AUTH_RANDOM_ITEM_AUTO_OPEN
#elif defined( _SERVICE_ ) || defined( _OPEN_TEST_ )
	#pragma NOTE( "ktdx.h 의 _SERVICE_ 실행!" )
	#ifndef _SERVICE_MANUAL_LOGIN_
		#define _ENCRIPT_SCRIPT_
	#endif _SERVICE_MANUAL_LOGIN_
		
//	#define NO_GAMEGUARD	// 해외팀 제거
    // #define CHECK_NP_HACKUSER	// 핵유저에 한해서 게임가드 실행
	//#define BUG_TRAP				// X2Main.cpp 로 이관
	#define MASS_FILE_FIRST			// 실제 파일보다 kom파일에서 먼저 찾기

// 허상형 : [2009/8/4] //	핵실드
//	#define	HACK_SHIELD		// 해외팀 제거
//{{ 허상형 : [2009/10/26] //	핵쉴드 LMP기능
//	#define	HS_LMP			// 해외팀 제거
//{{ 허상형 : [2010/5/24/] //	핵실드 어드민 계정 검사 안함
//	#define HS_NO_ADMIN_NEED	

//	#define ADD_CHKOPT_ABNORMAL_FUNCTION_CALL_V2	// 핵실드 기능 추가	// 해외팀 제거
//	#define ADDITIONAL_HS							// 핵실드 기능 추가	// 해외팀 제거
// 해외팀 제거
//	#define SEND_URL_TO_NEXON	// 어느 매체를 통해 게임 내 유입이 많이 이루어졌는지, 신규가입을 많이 했는지 넥슨쪽에 URL전달

	//#define AVATAR_IMAGE_TEST							// kimhc // 홈페이지 개편에서 아바타 이미지 표현으로 사용될 캐릭터 이미지 캡쳐 기능 테스트

//#ifdef _OPEN_TEST_
	// 아이템 치트로 사는 인터페이스
//	#define ITEM_CHEAT_POPUP_TEST
//	#define ADD_SHORT_CUT_KEY_IN_HOUSE		/// 사내 테스트용 편의 단축키 추가
//#endif	// _OPEN_TEST_
	

#elif defined( _FOREIGN_ )
	#pragma NOTE( "ktdx.h 의 _FOREIGN_ 실행!" )
    #define OPTIMIZED_P2P
	#define _ENCRIPT_SCRIPT_
    #define NO_GAMEGUARD
	

#elif defined( _IN_HOUSE_ )
    #pragma NOTE( "ktdx.h 의 _IN_HOUSE_ 실행!" )
	#define NO_GAMEGUARD
    //#define CHECK_NP_HACKUSER // 사내에서는 전사용자 대상으로 게임가드 실행
	//#define _USE_NEXON_MSG_INHOUSE // 사내에서 본섭 넥슨메신져 열결 (왠만해선 사용하지 말자.)
	
	//#define	HACK_SHIELD							// 해외팀 제거
//#ifdef HACK_SHIELD
//	#define	HS_LMP									// 해외팀 제거
//	#define ADD_CHKOPT_ABNORMAL_FUNCTION_CALL_V2	// 핵실드 기능 추가	// 해외팀 제거
//	#define ADDITIONAL_HS							// 핵실드 기능 추가	// 해외팀 제거
//#endif

#else 
	#pragma NOTE( "ktdx.h 의 define 확인하기.!" )
	#error "Check defines in ktdx.h!"

#endif // _SERVICE_

//#if defined( NDEBUG )	// kimhc // 2010-06-28 // 사내 또는 오픈테섭 릴리즈에서만 적용
//#define	IGA_TEST	//kimhc // 2010-03-03 // 광고 노출도 적용
//#endif	// #if defined( NDEBUG )

// 더미다 적용
//#define IN_TESTING_THEMIDA_BY_TOOL_TEAM


// Always ----------------------------------------------------------------------
#ifdef _ALWAYS_
#	include "Always.h"
#	include "AlwaysButConditionally.h"
#endif _ALWAYS_
// -----------------------------------------------------------------------------





// New_Village -----------------------------------------------------------------
#ifdef NEW_VILLAGE

#ifndef _SERVICE_
#define KHELP_SYSTEM_TEST			
//#define CHECK_OPENED_DEVICE
//#define MEMORY_LEAK_DETECTION_TEST	
#define NEW_VILLAGE_AUTO_SD_UNIT
#endif


// 새 마을 렌더링 테스트
#define NEW_VILLAGE_RENDERING_TEST

//#define SQUARE_CAMERA_TRACKING_TEST		// 광장에서 카메라가 캐릭터 느리게 따라가게 테스트 



// 캐릭터 백그라운드 로딩 테스트
/** skinned mesh의 background 로딩 관련 작업을 테스트하기 시작하다.
- jintaeks on 2008-12-12, 11:12
광장에 진입할 때 X2WorldObjectSkinMesh.cpp 객체의 로딩 작업을 완료하다.
- jintaeks on 2008-12-13, 15:07*/
#define UNIT_BACKGROUND_LOADING_TEST

/** 광장에 진입하고 나서 광장으로 들어오는 다른 유저들의 리소스 로딩 작업을 검토하기 시작하다.
- jintaeks on 2008-12-13, 15:07*/
#define EQUIP_BACKGROUND_LOADING_TEST


// 새마을 버전에 광장 유닛 백그라운드 로딩 적용시키기
#define NEW_VILLAGE_UNIT_BACKGROUND_LOADING_TEST


// 새마을 버전에 fieldnpc 백그라운드 로딩
//#define NEW_VILLAGE_FIELD_NPC_BACKGROUND_LOADING_TEST


// uitextureData에서 SetTex호출 할때 텍스쳐가 바뀐 경우에만 settex호출하도록 수정
#define EFFICIENT_SET_TEX_FOR_UI


#define UNIT_NEW_MOTION // 모션 분리 (광장 이모션)

#define NPC_SOUND   // npc대사치기...
#define OTHER_ROAD	// 갈림길
#define UNIT_EMOTION
#define CHANGE_CHANNEL      // 채널이동	

#endif NEW_VILLAGE


#define X2OPTIMIZE_REMOVE_MASS_FILE_MANAGER_LOCK // mass file manager 의 랙 제거
#define X2OPTIMIZE_MASS_FILE_MANAGER_PER_THREAD_SYNC_IO
#define X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
#define	X2OPTIMIZE_GAME_CHARACTER_BACKGROUND_LOAD
#define	X2OPTIMIZE_REFACTORING_RESOURCE_BACKGROUND_LOAD	// UnitLoader 와 호환성을 가지게 하기 위해
#define X2OPTIMIZE_GAME_PET_BACKGROUND_LOAD
#define	X2OPTIMIZE_DEVICE_HOLDER_VECTOR_TO_LIST
#define	X2OPTIMIZE_INFORM_FIRST_FRAME_OF_SIMULATION_LOOP
#define X2OPTIMIZE_SOUND_BACKROUND_LOAD // 사운드 리소스 백그라운드 로딩 지원, 파일 IO 는 백그라운드에서, fmod 설정은 fmod 쓰레드에서
#define	X2OPTIMIZE_MASS_FILE_BUFFER_MANAGER
#define X2OPTIMIZE_SIMULATION_LAG_LIMIT
#define X2OPTIMIZE_ANIMATION_LOSSY_COMPRESSION
//#define X2OPTIMIZE_KFILESERIALIZER_MODIFIED

//#define X2OPTIMIZE_NPC_LUASPACE_SHARING // 기본으로 사용


#define SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
#define SERV_KTDX_OPTIMIZE_UDP_PACKET_CHECKSUM
#define X2OPTIMIZE_ROBUST_NPC_FINAL_STATE
#define X2OPTIMIZE_ROBUST_NPC_ROTATION
#ifdef  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
#define SERV_KTDX_OPTIMIZE_NEW_UDP_CONNECTION_STRATEGY
#endif  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK

#define X2OPTIMIZE_NPC_NONHOST_SIMULATION
//#ifdef  X2OPTIMIZE_NPC_NONHOST_SIMULATION
//#define X2OPTIMIZE_NPC_NONHOST_SIMULATION_ANALYSIS
//#endif  X2OPTIMIZE_NPC_NONHOST_SIMULATION

#ifdef  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
#define SERV_KTDX_OPTIMIZE_UDP_ROBUST_CONNECTION
#endif  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK

#define X2OPTIMIZE_LINEMAP_LINEGROUP
// 비방장 유저가 NPC 투과 스킬 사용시 방장 컴퓨터에서 NPC 가 밀리는 버그 수정
#define X2OPTIMIZE_USER_PASS_PUSH_UNIT_TIME_BUG_FIX
// 캐릭터/ 캐릭터 충돌검사 원상복구
#ifdef  X2OPTIMIZE_NPC_NONHOST_SIMULATION
// 비방장 시뮬레이션 오버헤드를 최소화한다.
#define X2OPTIMIZE_NPC_NONHOST_SIMULATION_REDUCE_OVERHEAD
#define X2OPTIMIZE_USER_USER_PUSH_PASS_SYNC
// 유저유저 밀림 현상 남아있는 버그를 막는다.
#define X2OPTIMIZE_USER_USER_PUSH_PASS_SYNC_BUG_FIX
#define X2OPTIMIZE_PUSH_PASS_BUG_FIX
// 통신 오류 등으로 인해 npc state 가 범위를 초과했을 때 크래쉬가 발생하는 것을 막는다
#define X2OPTIMIZE_NPC_STATE_INDEXING_CRASH_BUG_FIX
#endif  X2OPTIMIZE_NPC_NONHOST_SIMULATION


//#define X2OPTIMIZE_MASS_FILE_FIRST_BUGFIX
#define X2OPTIMIZE_UNITTYPE_BUG_FIX


// 서로 다른 필드나 마을에 있는 파티원끼리 불필요하게 udp peer 연결하는 문제 방지
#define X2OPTIMIZE_PARTY_UDP_CONNECTION_BUG_FIX

//////////////////////////////////////////////////////////////////////////
// 이름: 김현철
// 아이디: itexpertkim
// 작업시작날짜: 2013-07-09
// 패치날짜: 2013-07-11
// 작업내용: 가르파이 기암지대에서 무한루프에 빠져 클라이언트가 동작하지 않는 문제 (형진이 형님이 수정 하심)
#define X2OPTIMIZE_LINEMAP_BUGFIX
//////////////////////////////////////////////////////////////////////////

#ifndef _SERVICE_
//#define X2OPTIMIZE_VERIFY_NPC_SCRIPT_CONSISTENCY
#endif	//_SERVICE_

//
// -----------------------------------------------------------------------------
// OpenTest 2번 서버 ---------------------------------------------------------------------
// 
//#if defined(_OPEN_TEST_2_ ) || defined(_IN_HOUSE_) // 오픈 테스트 2번 서버용 Define
	// 아이템 치트로 사는 인터페이스
//	#define ITEM_CHEAT_POPUP_TEST

//	#include  "OpenTest2.h"		// Always.h 처럼 작동합니다.
//#endif // _OPEN_TEST_2_			


// -----------------------------------------------------------------------------

// InHouse ---------------------------------------------------------------------
#ifdef _IN_HOUSE_ 

 	#include "InHouseEtc.h"	// 사내 툴 및 기타 기능들....
 	#include "InHouse1.h"	// 김종훈
 	#include "InHouse2.h"	// 김현철
 	#include "InHouse3.h"	// 김태환
 	#include "InHouse4.h"	// 강정훈
 	#include "InHouse5.h"	// 오현빈
	#include "InHouse6.h"	// 김창한

#ifdef WORLD_TOOL
	//#undef NOT_USE_PERCENT_IN_OPTION_DATA
	#undef ADD_PET_INVENTORY_BUTTON
	#undef PET_INVENTORY_BUG_FIX_01
	//#undef PET_AURA_SKILL
	#undef WINDOW_SYSTEM_BUTTON_FUNC
	//#undef COME_BACK_REWARD
	//#undef SERV_SOCKET_NEW
	#undef MACHINE_ID
	#undef SEASON3_MONSTER_2011_7
	//#undef NEW_HENIR_TEST
	#undef GRACE_OF_EL
	//#undef ELSWORD_WAY_OF_SWORD	
	//#undef UPGRADE_RAVEN
	//#undef SECRET_VELDER
	//#undef EVE_ELECTRA
	//#undef ADD_GAME_STAGE_DELETE_DAMAGEEFFECT
	#undef ADD_STATE_EXTRA_ATTACK
	//#undef SERV_ADD_CHUNG_SHELLING_GUARDIAN
	//#undef SERV_ADD_ARME_BATTLE_MAGICIAN
	//#undef ELSWORD_WAY_OF_SWORD
	
	//#undef CHUNG_SECOND_CLASS_CHANGE
	//#undef CHUNG_FIRST_CLASS_CHANGE
	#undef X2OPTIMIZE_ITEM_TEMPLET_PREPROCESSING
	#undef X2OPTIMIZE_MASS_FILE_MANAGER_REAL_FILE_ONLY
	#undef USE_FREE_TYPE
	// commonpacket.h 파일에서 직접 처리해야함
	//#undef SERV_PET_SYSTEM
	//#undef SERV_SHIP_DUNGEON
	//#undef SERV_WORLD_TRIGGER_RELOCATION
	//#undef SERV_TOONILAND_CHANNELING_EVENT
	//#undef SERV_DLL_LIST_CHECK_BEFOR_LOADING
	//#undef SERV_MACHINE_ID_BLOCK
	//#undef SERV_MACHINE_ID_DUPLICATE_CHECK
	//#undef SERV_MACHINE_ID_BLOCK_UPDATE
	//#undef SERV_MACHINE_ID_CHECK_BEFOR_LOADING
	#undef SERV_NEW_ITEM_SYSTEM_2013_05
	#undef UPGRADE_SKILL_UI
	#undef UPGRADE_SKILL_SYSTEM_2013
	#define BUG_TRAP
#endif

#ifdef X2TOOL
	#define SHOW_LINEMAP_FILE_NAME
	#define CHANGE_KEY_DEVICE	
	//#define ADD_POSTEFFECT

	
	#undef NO_P2P_NO_GAME	
	#undef CHECK_KOM_FILE_ON_LOAD
	//#undef MORE_INFO_ABOUT_HACK
	//#undef CHECK_FROM_DLLMANAGER
	//#undef DLL_MANAGER
	//#undef CHECK_ALL_WINDOW
	//#undef CHECK_THREAD_WND
	//#undef CHECK_FOREGROUNDWND
	//#undef DESK_THREAD_WINDOW
	//#undef ADD_COLLECT_CLIENT_INFO
	//#undef COLLECTION_MODULE
	//#undef SYS_INFO
	//#undef MACHINE_ID
#endif

#endif _IN_HOUSE_
// -----------------------------------------------------------------------------



#ifdef _NEXON_KR_

	#define NX_ENDING_FLASH					// 끝날 때 나오는 광고

#endif _NEXON_KR_







#define XOR_KEY0 0xc6f8aa02
#define XOR_KEY1 0x2647abdc
#define XOR_KEY2 0x9800bbef

#ifndef STRICT
#define STRICT
#endif

// If app hasn't chosen, set to work with Windows 98, Windows Me, Windows 2000, Windows XP and beyond
#ifndef WINVER
#define WINVER         0x0410
#endif
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410 
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT   0x0500 
#endif

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif

#pragma warning (disable: 4702)
#pragma warning (disable: 4995)
#pragma warning( disable : 4100 ) // disable unreference formal parameter warnings for /W4 builds

#include <Winsock2.h>
#include <wininet.h>
#include <windows.h>


#include <assert.h>
#include <wchar.h>
#include <commctrl.h> // for InitCommonControls() 
#include <shellapi.h> // for ExtractIcon()
#include <math.h>      
#include <limits.h>      
#include <stdio.h>
#include <time.h>
#include <process.h>
#include <io.h>
#include <shldisp.h>
#include <malloc.h>
//{{ robobeg : 2009-8-19
#include <atlbase.h> // for CComPtr and CComQIPtr
//}} robobeg : 2009-8-19

//////////////////////////////////////////////////////////////////////////
//현재 실행중인 프로세스를 찾아내기위한 함수를 쓰기위해
#pragma comment( lib, "Kernel32.lib" )
#include <TlHelp32.h>
//////////////////////////////////////////////////////////////////////////


#include "KSuperAssert.h"
#include "CrashRpt.h"

#ifndef DLL_BUILD		// 10.03.11 태완 : DLL Build시 다른 DLL에 대한 Dependancy 제거
#include "KncFirewall.h"

// nProtect 오류 메일 보내기
#include "../X2LIB/X2SMTPMail.h"
#pragma comment( lib, "MailMIME.lib" )
#pragma comment( lib, "MailSMTP.lib" )
#endif

#ifdef PROFILE_BUILD
	#include "../Profiler/KProfileMan.h"
#else
	#define KTDXPROFILE()
	#define KTDXPROFILE2(name)
	#define KTDXPROFILE_BEGIN(name)
	#define KTDXPROFILE_END()
#endif

#include "../X2/KOGGamePerformanceCheckDefine.h"

// CRT's memory leak detection
#if defined(DEBUG) || defined(_DEBUG)
	#include <crtdbg.h>
#endif


//{{ seojt // 2008-10-15, 10:47
// Enable extra D3D debugging in debug builds if using the debug DirectX runtime.  
// This makes D3D objects work well in the debugger watch window, but slows down 
// performance slightly.
// "d3d9.h"를 include하기 전에 이 매크로를 활성화시켜 주어야 한다.
// "d3d9.h" 인클루드 후에 있던 정의를 "d3d9.h"를 인클루드하기 전으로
// 옮기다. - jintaeks on 2008-10-15, 10:49
#if defined(DEBUG) || defined(_DEBUG)
	#ifndef D3D_DEBUG_INFO
		#define D3D_DEBUG_INFO
	#endif
#endif
//}} seojt // 2008-10-15, 10:47
// Direct3D includes
#include <d3d9.h>
#include <d3dx9.h>
#ifdef CONVERSION_VS
#	include <dxerr.h>
#else CONVERSION_VS
#	include <dxerr9.h>
#endif CONVERSION_VS
#include <dinput.h>
#include <mmsystem.h>
#include <mmreg.h>
#ifdef CONVERSION_VS
//#	include <dplay8.h>
#else CONVERSION_VS
#include <dplay8.h>
#include <ddraw.h>
#include <dsound.h>
#include <dshow.h>
#endif CONVERSION_VS


#ifndef STRSAFE_NO_DEPRECATE
#pragma deprecated("strncpy")
#pragma deprecated("wcsncpy")
#pragma deprecated("_tcsncpy")
#pragma deprecated("wcsncat")
#pragma deprecated("strncat")
#pragma deprecated("_tcsncat")
#endif

#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )

#include "zlib.h"

#include "DXUT.h"
#include "DXUTmisc.h"
#include "DXUTenum.h"
#include "DXUTmesh.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"



#include <vector>
using std::vector;
#include <stack>
using std::stack;
#include <queue>
using std::queue;
#include <list>
using std::list;
#include <map>
using std::map;
#include <hash_map>
using stdext::hash_map;
#include <string>
using std::wstring;
using std::string;
using std::wstringstream;
using std::stringstream;
#include <fstream>
#include <math.h>
#include <iostream>
using std::wcout;
using std::cout;
#include <set>
using std::set;
//{{ robobeg : 2008-10-10
#include <bitset>
#include    <deque>
#include    <hash_set>
#include    <hash_map>
#include	<boost/test/utils/foreach.hpp>
#include    <boost/foreach.hpp>
#include    <boost/static_assert.hpp>
#include    <boost/type_traits/is_base_of.hpp>
//}} robobeg : 2008-10-10

#include <sys/stat.h>
#include <boost/noncopyable.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/array.hpp>

#ifdef BOOST_SINGLETON_POOL_TEST
	#include <boost/pool/singleton_pool.hpp>
#endif BOOST_SINGLETON_POOL_TEST


#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/scope_exit.hpp>
#include <boost/circular_buffer.hpp>

#ifdef USE_FREE_TYPE
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftbitmap.h>
#pragma comment( lib, "freetype2411.lib" )
#endif USE_FREE_TYPE


#if defined(DEBUG) || defined(_DEBUG)

	// ASSERTwnx
	//
	#if !defined(ASSERT)
		#define ASSERT(exp)         _ASSERTE(exp)
	#endif // #if !defined(ASSERT)

	#ifndef V
		#define V(x)           { hr = x; if( FAILED(hr) ) { DXUTTrace( __FILE__, (DWORD)__LINE__, hr, L#x, true ); } }
	#endif
	#ifndef V_RETURN
		#define V_RETURN(x)    { hr = x; if( FAILED(hr) ) { return DXUTTrace( __FILE__, (DWORD)__LINE__, hr, L#x, true ); } }
	#endif


    #if !defined(TRACE)
        inline void _tracew(const WCHAR* fmt_, ...)
        {
            WCHAR   out[4096];
            va_list argList;

            va_start( argList, fmt_ );
            //StringCchVPrintfW( out, ARRAY_SIZE( out ), fmt_, argList );
#ifdef	CONVERSION_VS
			::vswprintf_s( out, _countof(out), fmt_, argList );
#else	CONVERSION_VS
			::vswprintf( out, fmt_, argList );
#endif	CONVERSION_VS
            va_end( argList );
            ::OutputDebugStringW( out );
        }//_tracew()

        inline void _tracea(const char* pszFormat_, ...)
        {
            char    out[4096];
			ZeroMemory( out, 4096 );
            va_list argList;

            va_start( argList, pszFormat_ );
            //StringCchVPrintfA( out, ARRAY_SIZE( out ), pszFormat_, argList );
#ifdef	CONVERSION_VS
			::vsprintf_s( out, _countof(out), pszFormat_, argList );
#else	CONVERSION_VS
			::vsprintf( out, pszFormat_, argList );
#endif	CONVERSION_VS
            va_end( argList );
            ::OutputDebugStringA( out );
        }//_tracea()
        #if !defined(TRACEA)
        #define TRACEA              _tracea
        #endif // #if !defined(TRACEA)
        #define TRACEW              _tracew

    #ifdef UNICODE // 2005-5-2, 11:40:58
    #define TRACE               _tracew
    #else
    #define TRACE               _tracea
    #endif // #ifdef UNICODE // 2005-5-2, 11:40:58
    #endif // #if !defined(TRACE)

#else

	#if !defined(ASSERT)
		#define ASSERT(exp)
	#endif // #if !defined(ASSERT)

	#ifndef V
		#define V(x)           { hr = x; }
	#endif
	#ifndef V_RETURN
		#define V_RETURN(x)    { hr = x; if( FAILED(hr) ) { return hr; } }
	#endif

    #if !defined(TRACE)
        //#define TRACE               1 ? (void)0 :
        /** The __noop intrinsic specifies that a function should be ignored and the argument list
            be parsed but no code be generated for the arguments. It is intended for use in global
            debug functions that take a variable number of arguments.
            - jintaeks on 2008-03-04, 10:00 */
        #define TRACE               __noop
        #if !defined(TRACEA)
        //#define TRACEA              1 ? (void)0 :
        #define TRACEA              __noop
        #endif // #if !defined(TRACEA)
        //#define TRACEW              1 ? (void)0 :
        #define TRACEW              __noop
    #endif // #if !defined(TRACE)

#endif

//{{ seojt // 2008-10-15, 10:27
//------------------------------------------------------------------------------
// CRT debug macros
//
#ifndef INIT_CRTDEBUG
/** 메모리 할당과 해제 요청이 있을 때마다 모든 메모리를 검사해서
    overwrite나 underwrite를 발견하고 싶으면 flag_에

        INIT_CRTDEBUG(_CRTDBG_CHECK_ALWAYS_DF);

    를 명시한다.
    일반적으로
    
        INIT_CRTDEBUG(0)

    처럼 사용한다.
    - jintaeks on 2008-03-20, 10:36 */
#define INIT_CRTDEBUG(flag_)    ::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | flag_)
#endif // #ifndef INIT_CRTDEBUG


/** @example    ASSERT_MEMORY()
	@code

    #include "KJohnGen.h"
    #include <iostream>

    void main()
    {
        INIT_CRTDEBUG(0);

        int*    p = new int;
        delete p;

        *p = 3;             // using after deletion
        ASSERT_MEMORY();    // see and check call stack window
        delete p;
    }//main()

	@endcode
*/
// ASSERT_MEMORY()가 제대로 동작하기 위해서는 먼저 INIT_CRTDEBUG()가 호출된 상태여야 한다.
#ifndef ASSERT_MEMORY
#define ASSERT_MEMORY()     ASSERT( ::_CrtCheckMemory() )
#endif // #ifndef ASSERT_MEMORY


/** @example    BREAK_ALLOC()
	@code

  //------------------------------------------------------------------------------
  //
  1) memory leak example:

    #include "KJohnGen.h"
    #include <iostream>

    void main()
    {
        INIT_CRTDEBUG(0);

        int*    p = new int;

        *p = 3;
    }//main()

    // debug message:
    Detected memory leaks!
    Dumping objects ->
    {44} normal block at 0x00372DA8, 4 bytes long.
    Data: <    > 03 00 00 00
    Object dump complete.

    // @note    allocation number is {44}, so you can set break point at 44th alloc
    //          see below example


  //------------------------------------------------------------------------------
  //
  2) break on mal-alloc {44}:

    #ifdef PROJECT_PCH
    #define _TOSTR1(x)    #x
    #define _TOSTR(x)     _TOSTR1(x)
    #include _TOSTR(PROJECT_PCH)
    #endif // PROJECT_PCH
    #include "KJohnGen.h"
    #include <iostream>

    void main()
    {
        BREAK_ALLOC( 44 ); // install break CRT point condition
        INIT_CRTDEBUG(0);

        int*    p = new int;

        *p = 3;
    }//main()

	@endcode
*/
// INIT_CRTDEBUG()가 보고한 메모리 블록에 break point를 건다.
#ifndef BREAK_ALLOC
#define BREAK_ALLOC(line_)   ::_CrtSetBreakAlloc( line_ )
#endif // #ifndef BREAK_ALLOC
//}} seojt // 2008-10-15, 10:27






#ifndef DMLEE_NOTE
	#ifdef DMLEE_ONLY
		#define DMLEE_NOTE(msg_) message ( __FILE__ "(" LINE(__LINE__)  "): [DMLEE_NOTE] " msg_ )
		#pragma DMLEE_NOTE( "comment out #define DMLEE_ONLY" )
	#else
		#define DMLEE_NOTE(msg_) message( "" )
	#endif DMLEE_ONLY
#endif DMLEE_NOTE



#define ARRAY_SIZE(a)       (sizeof(a)/sizeof((a)[0]))

#ifndef SAFE_CALL
#define SAFE_CALL(p)			if(p) p
#endif    
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)			{ if(p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release(); (p)=NULL; } }
#endif
#ifndef SAFE_CLOSE
#define SAFE_CLOSE(p)			{ if(p) { g_pKTDXApp->GetDeviceManager()->CloseDevice( (p)->GetDeviceID() );     (p)=NULL; } }
#endif

//{{ robobeg : 2008-01-05

#ifndef SAFE_FREE
#define SAFE_FREE(p)        { if(p) { ::free(p); (p)=NULL; } }
#endif // #ifndef SAFE_FREE

#ifndef SAFE_REALLOC
#define SAFE_REALLOC(p,s)   \
{   if((s)==0){ SAFE_FREE(p); }   \
else if (!(p))    { (p) = ::malloc(s); ASSERT(p); } \
else {  (p) = ::realloc((p),(s)); ASSERT(p); } }
#endif // #ifndef SAFE_REALLOC

#ifndef SAFE_TYPEDREALLOC
#define SAFE_TYPEDREALLOC(type,p,s)   \
{   if((s)==0){ SAFE_FREE(p); }   \
else if (!(p))    { (p) = (type*) ::malloc((s) * sizeof(type)); ASSERT(p); } \
else {  (p) = (type*) ::realloc((p), (s) * sizeof(type)); ASSERT(p); } }
#endif // #ifndef SAFE_TYPEDREALLOC

//}} robobeg : 2008-01-05

#ifndef SAFE_DELETE_DIALOG
//{{ robobeg : 2009-01-07
//#define SAFE_DELETE_DIALOG(p)	{ DialogLog("SAFE_DELETE_DIALOG"); if(p) { g_pKTDXApp->GetDGManager()->GetDialogManager()->DeleteDlg( (p) );		 (p)=NULL; } }
//#define SAFE_DELETE_DIALOG(p)   { DialogLog("SAFE_DELETE_DIALOG"); if ( p ) { bool bRet = g_pKTDXApp->GetDGManager()->GetDialogManager()->DeleteDlg(p); ASSERT( bRet ); (p)=NULL; } }
#define SAFE_DELETE_DIALOG(p)   { DialogLog("SAFE_DELETE_DIALOG"); if ( p != NULL ) { bool bRet = g_pKTDXApp->GetDGManager()->GetDialogManager()->DeleteDlg(p); ASSERT( bRet ); (p)=NULL; } }
//}} robobeg : 2009-01-07
#endif

//{{ seojt // 2009-8-19, 17:21
#ifndef SAFE_DELETE_DIALOG_HANDLE
#define SAFE_DELETE_DIALOG_HANDLE(p)    { CKTDGUIDialog::Destroy(p.GetHandle()); p.Invalidate(); }
#endif // SAFE_DELETE_DIALOG_HANDLE
//}} seojt // 2009-8-19, 17:21

//{{ seojt // 2009-1-13, 15:44
#ifndef SAFE_DELETE_KTDGOBJECT
#define SAFE_DELETE_KTDGOBJECT(p)   { if(p) { CKTDGObject::DeleteKTDGObject(p); (p)=NULL;} }
#endif // SAFE_DELETE_KTDGOBJECT
//}} seojt // 2009-1-13, 15:44


//{{ seojt // 2009-1-13, 15:44
#ifndef SHARED_POINTER_CAST
#define SHARED_POINTER_CAST(TARGET_TYPE,sharedPointer)     boost::static_pointer_cast<TARGET_TYPE>( sharedPointer )
#endif // SHARED_POINTER_CAST()
//}} seojt // 2009-1-13, 15:44


#ifndef SAFE_DELETE_XMESH_PLAYER_INSTANCE
#define SAFE_DELETE_XMESH_PLAYER_INSTANCE(xmesh,p)	{ if(p) { (xmesh)->DestroyInstance( (p) );					 (p)=NULL; } }
#endif
#ifndef CREATE_VECTOR3
#define CREATE_VECTOR3(v,a,b,c)	{ D3DXVECTOR3 (v); (v).x = (a); (v).y = (b); (v).z = (c); }
#endif  
#ifndef INIT_VECTOR3
#define INIT_VECTOR3(v,a,b,c)	{ (v).x = (a); (v).y = (b); (v).z = (c); }
#endif  
#ifndef CREATE_VECTOR2
#define CREATE_VECTOR2(v,a,b)	{ D3DXVECTOR2 (v); (v).x = (a); (v).y = (b); }
#endif  
#ifndef INIT_VECTOR2
#define INIT_VECTOR2(v,a,b)		{ (v).x = (a); (v).y = (b); }
#endif
#ifndef INIT_COLOR
#define INIT_COLOR(v,red,green,blue,alpha)		{ (v).r = (red); (v).g = (green); (v).b = (blue); (v).a = (alpha); }
#endif

#define SETVECTOR4(vector,_x,_y,_z,_w) {vector.x = _x; vector.y = _y; vector.z = _z; vector.w = _w;}
#define SETVECTOR3(vector,_x,_y,_z) {vector.x = _x; vector.y = _y; vector.z = _z;}
#define SETVECTOR2(vector,_x,_y) {vector.x = _x; vector.y = _y;}





static const D3DXVECTOR3 Y_AXIS( 0.f, 1.f, 0.f);





struct TimeLimited
{
	float m_fTimeLeft;

	virtual void OnTimeOutCallBack() {};

	virtual void OnFrameMove( double fTime, float fElapsedTime )
	{
		if( m_fTimeLeft > 0.f )
		{
			m_fTimeLeft -= fElapsedTime;

			if( m_fTimeLeft <= 0.f )
			{
				m_fTimeLeft = 0.f; 
				OnTimeOutCallBack();
			}
		}
	}


};

struct KTDGSphere
{
	D3DXVECTOR3	center;
	float		fRadius;
};

extern double g_NowTime;




#ifdef APPLY_THEMIDA
#	include "../x2lib/ThemidaSDK.h"
#endif


#include "EnumToString.h"
#include <KncSingleton.h>
#include "KTDXRandomNumbers.h"
#include "KTDXStringTable.h"
#include "KVersionControl.h"

#include "KSystemTimer.h"

#include "ktdxsimplehandleinterface.h"
#include "KTDXLog.h"
#include "debug.h"

#ifndef DLL_BUILD
#include "KncFirewall.h"
#endif 

#include "KTDXCommonFunc.h"
#include "KTDXRecyclingArray.h"
#include "../luaLib/KLuaManager.h"
#include "../luaLib/KLuaBinder.h"

inline void TableBind( KLuaManager* pLuaManager, KLuabinder* pLuaBinder )
{
	bool	retVal = true;
	int		index = 1;
	string	buffer;

	retVal = pLuaManager->GetValue(index,buffer);
	while( retVal == true )
	{
		HRESULT hr = pLuaBinder->DoString( buffer.c_str() );
		if( hr != S_OK )
			return;

		retVal = pLuaManager->GetValue(index,buffer);
		index++;
	}
}


#include "delegate.hpp"
#include "CRC_32.h"
#include "KTDXErrorDefine.h"
#include "KProtectRefTable.h"
#include "KProtectedType.h"
#include "KPCHWebBrowser.h"
#include "KBrowserWrapper.h"
#include "KTDXTimer.h"
#include "KTDXThread.h"

#ifdef CLIENT_COUNTRY_ID
// 2013.05.22 lygan_조성욱 // 인도네시아 인증에서 사용할 용도
#include "../X2Lib/OnlyGlobal/ID/KNMBrowserWrapper.h"
#endif //AUTH_CJ_ID

#include "KTDXDeviceHolder.h"
#include "KTDXStage.h"

#ifdef DYNAMIC_VERTEX_BUFFER_OPT
#include "KTDGDynamicVBManager.h"
#endif

//{{ dmlee 2008.05.14 checksum 검사를 별도 스레드에서 하도록 하는 코드 테스트 중
//#include "KStringUtil.h"
#include "CSLock.h"
#include "KJohnThread.h"

#ifdef CHECKSUM_THREAD_TEST
	#include "KFileVerifier.h"
#endif CHECKSUM_THREAD_TEST
//}} dmlee 2008.05.14 checksum 검사를 별도 스레드에서

#ifdef KHELP_SYSTEM_TEST // 2008-10-23
#include "KHelpSystem.h"
#endif // KHELP_SYSTEM_TEST // 2008-10-23

//{{ robobeg : 2008-01-05
#include "KFileSerializer.h"
//}} robobeg : 2008-01-05

//{{ seojt // 2009-4-3, 11:53
/** CKTDGUIDialog에서 crash 발생으로, handle을 사용하는 방식으로 바꾸는 작업을 시작하다.
    : jintaeks on 2009-04-03, 11:54
*/
#include "KJohnObjMan.h"
#include "KHandle.h"
//}} seojt // 2009-4-3, 11:53

#include "KHangulPPWSelector.h"
#include "MinMax.h"
#include <boost/shared_ptr.hpp>
#include <boost/scope_exit.hpp>

#ifdef CONVERSION_VS
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment( lib, "libxml_mtd.lib" )
#else
#pragma comment( lib, "libxml_mt.lib" )
#endif
#endif CONVERSION_VS
#include "KGCMassFileBufferManager.h"
#include "KGCMassFileManager.h"


#include "KTDXCollision.h"


class CKTDGObject;
typedef boost::weak_ptr<CKTDGObject>		CKTDGObjectWeakPtr; 
typedef boost::shared_ptr<CKTDGObject>		CKTDGObjectPtr; 


//#include "KTDXXET.h"

#include "KSmartNull.h"
#include "KObserverPtr.h"

#include "KTDXDevice.h"
#include "KTDXDeviceBaseTexture.h"
#include "KTDXDeviceTexture.h"
#include "KTDXDeviceXET.h"
#include "KTDXDeviceXMesh.h"
#include "KTDXDeviceXSkinMesh.h"

#include "KTDXDeviceRenderTargetTexture.h"
#include "KTDXDeviceSound.h"
#include "KTDXDeviceManager.h"

#include "KTDXEnumToLua.h"


#include "KTDGManager.h"
#include "KTDIManager.h"
#include "KTDSManager.h"
#ifdef CONVERSION_VS
#include "KTDNPacket.h"
#include "KTDNUDP.h"
#else CONVERSION_VS
#include "KTDNManager.h"
#endif CONVERSION_VS

//{{ robobeg : 2008-01-05
#include "KTDXAnimationController.h"
//}} robobeg : 2008-01-05

#ifdef KTDGDEVICEFONT_SIZE_CACHE
#include "KExceptionSafeUtil.h"
#endif

//{{ 09-03-04.hoons. 기존 스샷대용 라이브러리
#ifdef TEST_SCREEN_CAPTURE
	#if defined(DEBUG) || defined(_DEBUG)
		#pragma comment( lib, "jpegD.lib" )
		#pragma comment( lib, "pngd.lib" )
		#pragma comment( lib, "cxImageD.lib" )
	#else
		#pragma comment( lib, "jpeg.lib" )
		#pragma comment( lib, "png.lib" )
		#pragma comment( lib, "CxImage.lib" )
	#endif
	#include "CXImage/ximage.h"
#endif
//}} hoons.


#include "KTDXApp.h"

extern CKTDXApp* g_pKTDXApp;

#endif
