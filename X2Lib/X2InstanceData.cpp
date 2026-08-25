#include "StdAfx.h"
#include ".\x2instancedata.h"

#ifdef SERVER_NAME
#include <boost/assign.hpp>
#endif SERVER_NAME

CX2InstanceData::CX2InstanceData(void) :
m_pMiniMapUI( NULL ),
m_bIsDungeonLounge( false ),
m_bLoadedDungeonLoungeUnits( false ),
m_eDungeonLoungeWorldID( CX2World::WI_NONE ),
m_vLastDungeonGatePos( 0, 0, 0 ),
m_UserID( L"" ),
#ifdef SERV_CHANNELING_AERIA
m_UserSessionID( L"" ),
#endif //SERV_CHANNELING_AERIA
m_UserPassword( L"" ),
#ifdef CLIENT_COUNTRY_TH
m_wstrSocketID( L"" ),
#endif // CLIENT_COUNTRY_TH
#ifdef SERV_KOG_OTP_VERIFY
m_bIDVerified(false),
#endif SERV_KOG_OTP_VERIFY
#ifdef SERV_PURCHASE_TOKEN
m_wstrPurchaseTok( L"" ),
#endif SERV_PURCHASE_TOKEN
#ifdef CLIENT_PURPLE_MODULE_IN_HOUSE_AUTH
m_bInHouseAccount(false),
#endif CLIENT_PURPLE_MODULE_IN_HOUSE_AUTH
m_iConnectChannelServerID( -1 ),
m_bFirstJoinVillage(false)
//{{ kimhc // 실시간 엘소드 중 실시간 ED 획득 관련
#ifdef	REAL_TIME_ELSWORD
, m_iED( 0 )
#endif	REAL_TIME_ELSWORD
//}} kimhc // 실시간 엘소드 중 실시간 ED 획득 관련
, m_bShowCSPandSlotBDate( false )
, m_bIsChangeChannel(false)
, m_bRegisteredRank( false )
//{{ kimhc // 자동결제에서 최근 열리거나 닫힌 웹페이지가 무엇인지를 판단 하는 변수 및 함수 추가
#ifdef	AUTO_PAYMENT
, m_byWebType( 0 )	// 최근 열리거나 닫힌 웹의 종류
#endif	AUTO_PAYMENT
//}} kimhc // 자동결제에서 최근 열리거나 닫힌 웹페이지가 무엇인지를 판단 하는 변수 및 함수 추가

//{{ kimhc // 2009-12-15 // 서버군 추가 작업에 따른 채널 다이얼로그 분리
#ifdef	ADD_SERVER_GROUP
#ifdef EXTEND_SERVER_GROUP_MASK
, m_iServerGroupID( -1 )	// 서버 선택이 안되있는 경우 솔레스로 초기화 
, m_iServerGroupIDCashShop( -1 )	// 현재 가지고 있는 캐시아이템 리스트를 받아온 서버군 ID
, m_iServerGroupIDScript( -1 )
#else
, m_eServerGroupID( SGI_INVALID )	// 서버 선택이 안되있는 경우 솔레스로 초기화 
, m_eServerGroupIDCashShop( SGI_INVALID )	// 현재 가지고 있는 캐시아이템 리스트를 받아온 서버군 ID
, m_eServerGroupIDScript( SGI_INVALID )
#endif // EXTEND_SERVER_GROUP_MASK
#endif	ADD_SERVER_GROUP
//}}  kimhc // 2009-12-15 // 서버군 추가 작업에 따른 채널 다이얼로그 분리
#ifdef DAMAGE_HISTORY
, m_bSendedHackUser(false)
, m_bSendedHackMail(false)
#endif
#ifdef SERV_PROTECT_AUTO_HACK
, m_bSendedHackMailAutoHack(false)
#endif // SERV_PROTECT_AUTO_HACK
, m_bFirstSelect(false)	// kimhc // 로그인 후 처음 캐릭터 선택인가? // PC 방 인벤토리 디폴트 탭선택 작업
, m_bReceiveGameStartNotPacket( false ) // kimhc // 2010-03-23 // STATE_CHANGE_GAME_START_NOT를 받았는가?	
, m_bSendGameStartReqPacket( false ) // kimhc // 2010-03-25 // Change_state_Game_start_req를 전송하면 직후에는 던전 게임이 시작 할 때 까지 스킬슬롯을 바꿀수 없도록 함
, m_fVerifyGageManagerTimer(0.f)
#ifdef SERV_PSHOP_AGENCY
, m_bIsPShopOpen(false)
, m_wstrAgencyShopExpirationDate(L"")
#endif
#ifdef MACHINE_ID
, m_strMachineId("")
#endif
, m_TimerForSendingPlayStatus( 3.0f )
#ifdef SERV_NEW_EVENT_TYPES
, m_iMaxLevel( _CONST_X2GAME_::g_iMaxLevel )
#endif SERV_NEW_EVENT_TYPES
#ifdef UDP_CAN_NOT_SEND_USER_KICK // 2012.06.19 lygan_조성욱 // 대전에서 UDP 패킷이 오지 않는 유저들 킥하는 코드 활성화 여부
, m_bSetUDPKickOff(false)
#endif //UDP_CAN_NOT_SEND_USER_KICK
#ifdef	USE_ACCOUNT_DOMAIN
, m_eDomain( ACCOUNT_DOMAIN_TYPE::ADT_PLAYID )
#endif	USE_ACCOUNT_DOMAIN
#ifdef CLIENT_PORT_CHANGE_REQUEST
, m_bStartPortChangeRequest(false)
, m_iUDPPortSuccessType(0)
#endif //CLIENT_PORT_CHANGE_REQUEST
#ifdef SERV_ID_NETMARBLE_PCBANG
, m_bPublicIP(false)
#endif //SERV_ID_NETMARBLE_PCBANG
#ifdef SERV_COUNTRY_PH
, m_strAuthToken("")
#endif //SERV_COUNTRY_PH
#ifdef SERV_STEAM
, m_iChannelingCode( 0 )
#endif //SERV_STEAM
#ifdef SERV_RECRUIT_EVENT_QUEST_FOR_NEW_USER
, m_bRecruit( false )
#endif SERV_RECRUIT_EVENT_QUEST_FOR_NEW_USER
{

#if defined( _IN_HOUSE_ ) || defined( _OPEN_TEST_ ) || defined( _OPEN_TEST_2_ )
	m_bFrameScale = false;
#endif

#if defined( SERV_HACKING_TOOL_LIST )

	THEMIDA_VM_START

#if defined( _SERVICE_ ) 
	// checked
	ELSWORD_VIRTUALIZER_START

#endif

#if 0
	KHackingToolInfo hackInfo;
	hackInfo.m_cFlag = 2;
	hackInfo.m_wstrProcessName = L"OllyDbg";
	hackInfo.m_wstrWindowClassName = L"";
	PushHackList(hackInfo);
	hackInfo.m_cFlag = 2;
	hackInfo.m_wstrProcessName = L"Ollyice";
	hackInfo.m_wstrWindowClassName = L"";
	PushHackList(hackInfo);
	hackInfo.m_cFlag = 7;
	hackInfo.m_wstrProcessName = L"HanAier.dll";
	hackInfo.m_wstrWindowClassName = L"";
	PushHackList(hackInfo);
	hackInfo.m_cFlag = 7;
	hackInfo.m_wstrProcessName = L"ppd.dll";
	hackInfo.m_wstrWindowClassName = L"";
	PushHackList(hackInfo);
#endif

	m_bChangeHacklist = false;

#ifdef VERIFY_NPC_HP
	m_bVerifyNpcHp = false;
#endif

#ifdef EXTEND_SERVER_GROUP_MASK
#else EXTEND_SERVER_GROUP_MASK
#ifdef SERVER_NAME
	using namespace boost::assign;
	insert( m_mapServerGroupName )
		( SGI_INVALID, L"Invalid ServerGroup" )
		( SGI_SOLES, GET_STRING( STR_ID_4836 ) ) 
		( SGI_GAIA, GET_STRING( STR_ID_4835 ) )
		;
#endif SERVER_NAME
#endif EXTEND_SERVER_GROUP_MASK

#ifdef SERV_EPAY_SYSTEM
	m_usGiantZone = 0;
	m_usGiantGame = 0;
#endif //SERV_EPAY_SYSTEM

#ifdef ADD_KPROTECTEDTYPE_VALUE
	m_nVerifyNpcStateId = 0;
#endif

#ifdef SERV_DLL_MANAGER
	m_bSendedLoadedModuleList = false;
#endif

#ifdef SERV_DLL_LIST_CHECK_BEFOR_LOADING
	m_iChannelRandomKey = -1;
#endif
#ifdef SERV_ED_MONITORING_IN_GAME
	m_bMoneyMonitoring = false;
#endif
#if defined( _SERVICE_ )
	ELSWORD_VIRTUALIZER_END
#endif

#ifdef SERV_SERIAL_NUMBER_AVAILABILITY_CHECK
	memset(m_cServerSN, 0, sizeof(unsigned char) * SERVER_SN);
	memset(m_cOldServerSN, 0, sizeof(unsigned char) * SERVER_SN);
#endif

#ifdef ADD_COLLECT_CLIENT_INFO
	memset( g_AntiHackingList, 0, sizeof(unsigned int) * ANTIHACKING_ID::ANTIHACKING_ID_END );
#endif ADD_COLLECT_CLIENT_INFO

	THEMIDA_VM_END

#endif // defined( SERV_HACKING_TOOL_LIST )

	m_iLastDungeonGatePosLineIndex = 0;

#ifdef X2TOOL
	m_bIsNpcLoad = false;
	m_bCanDungeonPlay = false;
	m_bChangeStage = false;
#endif
}

CX2InstanceData::~CX2InstanceData(void)
{
	SAFE_DELETE( m_pMiniMapUI );

}

void CX2InstanceData::OnFrameMove( double fTime, float fElapsedTime )
{
	//{{ 09.03.23 태완
	if( NULL != m_pMiniMapUI )
	{
		m_pMiniMapUI->OnFrameMove( fTime, fElapsedTime );
	}
	//}}

#ifdef FIX_GAGEMANAGER01
	m_fVerifyGageManagerTimer -= fElapsedTime;

	if( m_fVerifyGageManagerTimer <= 0.f )
		m_fVerifyGageManagerTimer = 0.f;
#endif
}

void CX2InstanceData::ResetMiniMapUI()
{
	SAFE_DELETE( m_pMiniMapUI );
	m_pMiniMapUI = new CX2MiniMapUI;
}

//{{ kimhc // 2009-12-18 // 서버군 추가 작업으로 사용하지 않음
#ifndef	ADD_SERVER_GROUP

int CX2InstanceData::GetChannelIdFromListId(int listId)
{
	// 채널추가
	switch(listId)
	{
	case 1:			// 루벤1
		return 1;
	case 2:			// 루벤2
		return 3;
	case 3:			// 루벤3
		return 5;
	case 4:			// 엘더1
		return 2;
	case 5:			// 엘더2
		return 4;	
	case 6:			// 엘더3
		return 6;
	case 7:			// 베스마1
		return 7;
	case 8:			// 베스마2
		return 9;
	case 9:			// 베스마3
		return 11;
	case 10:		// 베스마4
		return 13;
	case 11:		// 베스마5
		return 15;
	case 12:		// 알테라1
		return 8;
	case 13:		// 알테라2
		return 10;
	case 14:		// 알테라3
		return 12;
	case 15:		// 알테라4
		return 14;
	case 16:		// 알테라5
		return 16;
	}
	return 0;
}

int CX2InstanceData::GetListIdFromChannelId(int channelId)
{
	// 채널추가
	switch(channelId)
	{
	case 1:			// 루벤1
		return 1;
	case 2:			// 엘더1
		return 4;
	case 3:			// 루벤2
		return 2;
	case 4:			// 엘더2
		return 5;
	case 5:			// 루벤3
		return 3;
	case 6:			// 엘더3
		return 6;
	case 7:			// 베스마1
		return 7;
	case 8:			// 알테라1
		return 12;
	case 9:			// 베스마2
		return 8;
	case 10:		// 알테라2
		return 13;
	case 11:		// 베스마3
		return 9;
	case 12:		// 알테라3
		return 14;
	case 13:		// 베스마4
		return 10;
	case 14:		// 알테라4
		return 15;
	case 15:		// 베스마5
		return 11;
	case 16:		// 알테라5
		return 16;
	}

	return 0;
}
#endif	ADD_SERVER_GROUP
//}} kimhc // 2009-12-18 // 서버군 추가 작업으로 사용하지 않음

wstring CX2InstanceData::GetConnectChannelServerName()
{
	return GetChannelServerName( m_iConnectChannelServerID );
}
#ifdef SERVER_INTEGRATION_CHANNEL_NAME_FIX
#ifdef EXTEND_SERVER_GROUP_MASK
wstring CX2InstanceData::GetChannelServerName( int ChannelID, int ServerID)
{
	wstring wstrChannelName = L"";
#ifdef SERV_CHANNEL_LIST_RENEWAL
	if ( m_vecChannelInfo.empty() == false )
	{
		for(int i = 0; i < static_cast<signed>( m_vecChannelInfo.size() ); i ++)
		{
			if(m_vecChannelInfo[i].m_iChannelID == ChannelID)
			{
				wstrChannelName = m_vecChannelInfo[i].m_wstrChannelName;
			}
		}
	}
#else //SERV_CHANNEL_LIST_RENEWAL
	if(ServerID == 1 || ServerID == 3)
	{
		switch(ChannelID)
		{
		case 1:		// 루벤1
			wstrChannelName = GET_STRING( STR_ID_232 );
			break;
		case 2:		// 루벤2
			wstrChannelName = GET_STRING( STR_ID_233 );
			break;
		case 3:		// 루벤3
			wstrChannelName = GET_STRING( STR_ID_236 );
			// 엘더1
			//wstrChannelName = GET_STRING( STR_ID_236 );
			break;
		case 4:		// 루벤4
			//wstrChannelName = GET_STRING( STR_ID_235 );
			// 엘더1
			wstrChannelName = GET_STRING( STR_ID_237 );
			break;
		case 5:		// 엘더1
			//wstrChannelName = GET_STRING( STR_ID_236 );
			// 엘더2
			wstrChannelName = GET_STRING( STR_ID_240 );
			break;
		case 6:		// 엘더2
			//wstrChannelName = GET_STRING( STR_ID_237 );
			// 엘더3
			wstrChannelName = GET_STRING( STR_ID_241 );
			break;
		case 7:		// 엘더3
			//wstrChannelName = GET_STRING( STR_ID_238 );
			// 베스마1
			wstrChannelName = GET_STRING( STR_ID_4751 );
			break;
		case 8:		// 엘더4
			//wstrChannelName = GET_STRING( STR_ID_239 );
			// 베스마2
			wstrChannelName = GET_STRING( STR_ID_4773 );
			break;
		case 9:		// 베스마1
			//wstrChannelName = GET_STRING( STR_ID_240 );
			// 베스마3
			wstrChannelName = GET_STRING( STR_ID_15168 );
			break;
		case 10:	// 베스마2
			//wstrChannelName = GET_STRING( STR_ID_241 );
			// 베스마4
			//wstrChannelName = GET_STRING( STR_ID_243 );
			// 알테라1
			wstrChannelName = GET_STRING( STR_ID_15227 );
			break;
		case 11:	// 베스마3
			//wstrChannelName = GET_STRING( STR_ID_242 );
			// 알테라1
			//wstrChannelName = GET_STRING( STR_ID_4751 );
			// 알테라2
			wstrChannelName = GET_STRING( STR_ID_15228 );
			break;
		case 12:	// 베스마4
			//wstrChannelName = GET_STRING( STR_ID_243 );
			// 알테라2
			wstrChannelName = GET_STRING( STR_ID_15385 );
			break;
		case 13:	// 베스마4
			wstrChannelName = GET_STRING( STR_ID_244 );
			break;
		case 14:	// 알테라4
			wstrChannelName = GET_STRING( STR_ID_4751 );
			break;
		case 15:	// 베스마5
			wstrChannelName = GET_STRING( STR_ID_4780 );	
			break;
		case 16:	// 알테라5
			wstrChannelName = GET_STRING( STR_ID_4775 );		
			break;
		default:
			break;
		}
	}
	else if(ServerID == 2)
	{
		switch(ChannelID)
		{
		case 1:		// 루벤1
			wstrChannelName = GET_STRING( STR_ID_232 );
			break;
		case 2:		// 루벤2
			wstrChannelName = GET_STRING( STR_ID_236 );
			break;
		case 3:		// 엘더1
			wstrChannelName = GET_STRING( STR_ID_240 );
			break;
		case 4:		// 엘더2
			wstrChannelName = GET_STRING( STR_ID_4751 );
			break;
		case 5:		// 베스마1
			wstrChannelName = GET_STRING( STR_ID_4773 );
			break;
		case 6:		// 베스마2
			wstrChannelName = GET_STRING( STR_ID_15227 );
			break;
		case 7:		// 알테라1
			wstrChannelName = GET_STRING( STR_ID_15228 );
			break;
		case 8:		// 알테라2
			wstrChannelName = GET_STRING( STR_ID_15385 );
			break;
		case 9:		// 베스마2
			wstrChannelName = GET_STRING( STR_ID_240 );
			break;
		case 10:	// 알테라2
			wstrChannelName = GET_STRING( STR_ID_241 );
			break;
		case 11:	// 베스마3
			wstrChannelName = GET_STRING( STR_ID_242 );
			break;
		case 12:	// 알테라3
			wstrChannelName = GET_STRING( STR_ID_243 );
			break;
		case 13:	// 베스마4
			wstrChannelName = GET_STRING( STR_ID_244 );
			break;
		case 14:	// 알테라4
			wstrChannelName = GET_STRING( STR_ID_4751 );
			break;
		case 15:	// 베스마5
			wstrChannelName = GET_STRING( STR_ID_4780 );	
			break;
		case 16:	// 알테라5
			wstrChannelName = GET_STRING( STR_ID_4775 );		
			break;
		default:
			break;
		}
	}
	else
	{
		switch(ChannelID)
		{
		case 1:		// 루벤1
			wstrChannelName = GET_STRING( STR_ID_232 );
			break;
		case 2:		// 루벤2
			wstrChannelName = GET_STRING( STR_ID_233 );
			break;
		case 3:		// 엘더1
			wstrChannelName = GET_STRING( STR_ID_234 );
			break;
		case 4:		// 엘더2
			wstrChannelName = GET_STRING( STR_ID_236 );
			break;
		case 5:		// 베스마1
			wstrChannelName = GET_STRING( STR_ID_237 );
			break;
		case 6:		// 베스마2
			wstrChannelName = GET_STRING( STR_ID_238 );
			break;
		case 7:		// 알테라1
			wstrChannelName = GET_STRING( STR_ID_240 );
			break;
		case 8:		// 알테라2
			wstrChannelName = GET_STRING( STR_ID_241 );
			break;
		case 9:		// 베스마2
			wstrChannelName = GET_STRING( STR_ID_242 );
			break;
		case 10:	// 알테라2
			wstrChannelName = GET_STRING( STR_ID_4751 );
			break;
		case 11:	// 베스마3
			wstrChannelName = GET_STRING( STR_ID_4773 );
			break;
		case 12:	// 알테라3
			wstrChannelName = GET_STRING( STR_ID_15168 );
			break;
		case 13:	// 베스마4
			wstrChannelName = GET_STRING( STR_ID_15169 );
			break;
		case 14:	// 알테라4
			wstrChannelName = GET_STRING( STR_ID_15227 );
			break;
		case 15:	// 베스마5
			wstrChannelName = GET_STRING( STR_ID_15228 );	
			break;
		case 16:	// 알테라5
			wstrChannelName = GET_STRING( STR_ID_15385 );		
			break;
		default:
			break;
		}
	}
#endif //SERV_CHANNEL_LIST_RENEWAL

	return wstrChannelName;
}
#else

wstring CX2InstanceData::GetChannelServerName( int ChannelID, SERVER_GROUP_ID ServerID)
{
	wstring wstrChannelName = L"";

#ifdef SERV_CHANNEL_LIST_RENEWAL
	if ( m_vecChannelInfo.empty() == false )
	{
		for( UINT i=0; i < m_vecChannelInfo.size(); ++i )
		{
			if(m_vecChannelInfo[i].m_iChannelID == ChannelID)
			{
				wstrChannelName = m_vecChannelInfo[i].m_wstrChannelName;
			}
		}
	}
#else //SERV_CHANNEL_LIST_RENEWAL
	switch (ServerID)
	{
	case SGI_SOLES:
		{
			switch(ChannelID)
			{
			case 1:		// 루벤1
				wstrChannelName = GET_STRING( STR_ID_232 );
				break;
			case 2:		// 알테라2 
				wstrChannelName = GET_STRING( STR_ID_241 );
				break;
			case 3:		// 루벤2
				wstrChannelName = GET_STRING( STR_ID_234 );
				break;
			case 4:		// 벨더1
				wstrChannelName = GET_STRING( STR_ID_15227 );
				break;
			case 5:		// 엘더1
				wstrChannelName = GET_STRING( STR_ID_233 );
				break;
			case 6:		// 벨더2
				wstrChannelName = GET_STRING( STR_ID_15228 );
				break;
			case 7:		// 엘더2
				wstrChannelName = GET_STRING( STR_ID_235 );
				break;
			case 8:		// 벨더3
				wstrChannelName = GET_STRING( STR_ID_15385 );
				break;
			case 9:		// 베스마1
				wstrChannelName = GET_STRING( STR_ID_238 );
				break;
			case 10:	// 하멜1
				wstrChannelName = GET_STRING( STR_ID_15229 );
				break;
			case 11:	// 베스마2
				wstrChannelName = GET_STRING( STR_ID_240 );
				break;
			case 12:	// 하멜2
				wstrChannelName = GET_STRING( STR_ID_15230 );
				break;
			case 13:	// 알테라1
				wstrChannelName = GET_STRING( STR_ID_239 );
				break;
			case 14:	// 하멜3
				wstrChannelName = GET_STRING( STR_ID_15386 );
				break;
			default:
				break;
			}
		}
		break;
	case SGI_GAIA:
		{
			switch(ChannelID)
			{
			case 1:		// 루벤1
				wstrChannelName = GET_STRING( STR_ID_232 );
				break;
			case 2:		// 알테라1
				wstrChannelName = GET_STRING( STR_ID_239 );
				break;
			case 3:		// 루벤2
				wstrChannelName = GET_STRING( STR_ID_234 );
				break;
			case 4:		// 알테라2
				wstrChannelName = GET_STRING( STR_ID_241 );
				break;
			case 5:		// 루벤3
				wstrChannelName = GET_STRING( STR_ID_236 );
				break;
			case 6:		// 벨더1
				wstrChannelName = GET_STRING( STR_ID_15227 );
				break;
			case 7:		// 엘더1
				wstrChannelName = GET_STRING( STR_ID_233 );
				break;
			case 8:		// 벨더2
				wstrChannelName = GET_STRING( STR_ID_15228 );
				break;
			case 9:		// 엘더2
				wstrChannelName = GET_STRING( STR_ID_235 );
				break;
			case 10:	// 하멜1
				wstrChannelName = GET_STRING( STR_ID_15229 );
				break;
			case 11:	// 베스마1
				wstrChannelName = GET_STRING( STR_ID_238 );
				break;
			case 12:	// 하멜2
				wstrChannelName = GET_STRING( STR_ID_15230 );
				break;
			case 13:	// 베스마2
				wstrChannelName = GET_STRING( STR_ID_240 );
				break;
			default:
				break;
			}
		}
		break;
	default:
		break;
	}
#endif
	return wstrChannelName;
}
#endif EXTEND_SERVER_GROUP_MASK
#endif SERVER_INTEGRATION_CHANNEL_NAME_FIX

wstring CX2InstanceData::GetChannelServerName( int ChannelID )
{
	//{{ kimhc // 2009-12-15 // 서버군 추가 작업
#ifdef	ADD_SERVER_GROUP
	if( 0 == ChannelID )
	{
		ErrorLogMsg(XEM_ERROR143,"ChannelID == 0");
		return L"";
	}
	
	wstring wstrChannelName = L"";
	if ( false == m_vecChannelInfo.empty() && (ChannelID-1) < static_cast<int>(m_vecChannelInfo.size()) )
	{
#ifdef	SERV_CHANNEL_LIST_RENEWAL
		for( UINT i = 0; i < m_vecChannelInfo.size(); ++i )
		{
			if(m_vecChannelInfo[i].m_iChannelID == ChannelID)
			{
				wstrChannelName = m_vecChannelInfo[i].m_wstrChannelName;
			}
		}

		return wstrChannelName;
#else	//SERV_CHANNEL_LIST_RENEWAL
		return m_vecChannelInfo[ChannelID - 1].m_wstrChannelName;
#endif //SERV_CHANNEL_LIST_RENEWAL
	}
	else
	{
		ErrorLog(XEM_ERROR143);
		return wstrChannelName;
	}
#else	ADD_SERVER_GROUP
	// 채널추가
	// 현재 접속한 채널 이름을 설정한다.
	wstring wstrChannelName = L"";

	switch(ChannelID)
	{
	case 1:		// 루벤1
		wstrChannelName = GET_STRING( STR_ID_232 );
		break;
	case 2:		// 엘더1
		wstrChannelName = GET_STRING( STR_ID_233 );
		break;
	case 3:		// 루벤2
		wstrChannelName = GET_STRING( STR_ID_234 );
		break;
	case 4:		// 엘더2
		wstrChannelName = GET_STRING( STR_ID_235 );
		break;
	case 5:		// 루벤3
		wstrChannelName = GET_STRING( STR_ID_236 );
		break;
	case 6:		// 엘더3
		wstrChannelName = GET_STRING( STR_ID_237 );
		break;
	case 7:		// 베스마1
		wstrChannelName = GET_STRING( STR_ID_238 );
		break;
	case 8:		// 알테라1
		wstrChannelName = GET_STRING( STR_ID_239 );
		break;
	case 9:		// 베스마2
		wstrChannelName = GET_STRING( STR_ID_240 );
		break;
	case 10:	// 알테라2
		wstrChannelName = GET_STRING( STR_ID_241 );
		break;
	case 11:	// 베스마3
		wstrChannelName = GET_STRING( STR_ID_242 );
		break;
	case 12:	// 알테라3
		wstrChannelName = GET_STRING( STR_ID_243 );
		break;
	case 13:	// 베스마4
		wstrChannelName = GET_STRING( STR_ID_244 );
		break;
	case 14:	// 알테라4
		wstrChannelName = GET_STRING( STR_ID_4751 );
		break;
	case 15:	// 베스마5
		wstrChannelName = GET_STRING( STR_ID_4780 );	
		break;
	case 16:	// 알테라5
		wstrChannelName = GET_STRING( STR_ID_4775 );		
		break;
	default:
		break;
	}

	return wstrChannelName;
#endif	ADD_SERVER_GROUP
	//}}  kimhc // 2009-12-15 // 서버군 추가 작업
}

//{{ kimhc // 2009-12-15 // 서버군 추가 작업
#ifdef	ADD_SERVER_GROUP

#ifndef CHANNEL_CONGESTION_SCOPE_NO_CHEAT
void CX2InstanceData::SetChannelDistributionByServerGroup( OUT float* pfChCongestionScopeTemp, IN const float* pfChCongestionScope, IN int iChannelIndex )
{
	// 채널 그룹에서 첫번째 채널인지를 판단하는 플래그 ex) 루벤01, 02, 03 채널중 루벤01이 첫번째 채널
	bool bIsFirstChannelInChannelGroup = false;

	switch ( g_pInstanceData->GetServerGroupID() )
	{
	case SGI_GAIA:
		{

			switch ( iChannelIndex )
			{
			case 0:
			case 3:
			case 6:
			case 7:
			case 11:
			case 12:
				{
					bIsFirstChannelInChannelGroup = true;
				}
				break;
			}
		}
		break;

	case SGI_SOLES:
	default:
		{
			switch ( iChannelIndex )
			{
			case 0:
			case 5:
			case 6:
			case 11:
			case 12:
			case 18:
				{
					bIsFirstChannelInChannelGroup = true;
				}
			}
		}
		break;
	}


	if ( bIsFirstChannelInChannelGroup == false )	// 채널 그룹에서 첫번째 채널이 아니면
	{
		std::copy( pfChCongestionScope, pfChCongestionScope + 4, pfChCongestionScopeTemp );
	}
	else
	{
		// 채널그룹중 첫채널은 분포도 다르게 표시
		pfChCongestionScopeTemp[0] = 0.1f;
		pfChCongestionScopeTemp[1] = 0.2f;
		pfChCongestionScopeTemp[2] = 0.3f;
		pfChCongestionScopeTemp[3] = pfChCongestionScope[3];
	}
}
#endif // CHANNEL_CONGESTION_SCOPE_NO_CHEAT

#ifdef SHOW_SERVERGROUP_NAME
void CX2InstanceData::RefreshTitle()
{
#ifdef _SERVICE_
	std::wstring wstrTitle( L"Elsword" );
	switch( m_iServerGroupID )
	{
	case 1:
		// 화동1군
		wstrTitle += std::wstring( L" - " ) + GET_STRING( STR_ID_26141 );
		break;
	case 6:
		// 화동2군
		wstrTitle += std::wstring( L" - " ) + GET_STRING( STR_ID_26142 );
		break;
	case 4:
		// 화중1군
		wstrTitle += std::wstring( L" - " ) + GET_STRING( STR_ID_26143 );
		break;
	case 101:
		// 화북1군
		wstrTitle += std::wstring( L" - " ) + GET_STRING( STR_ID_26144 );
		break;
	default:
		break;
	}

	SetWindowText( g_pKTDXApp->GetHWND(), wstrTitle.c_str() );
#endif _SERVICE_
}
#endif SHOW_SERVERGROUP_NAME

#ifdef EXTEND_SERVER_GROUP_MASK
void CX2InstanceData::SetServerGroupID( int iServerGroupID ) 
{ 
	m_iServerGroupID = iServerGroupID;
#ifdef SHOW_SERVERGROUP_NAME
	RefreshTitle();
#endif SHOW_SERVERGROUP_NAME
}
#else
void CX2InstanceData::SetServerGroupID( SERVER_GROUP_ID eServerGroupID ) 
{ 
	m_eServerGroupID = eServerGroupID;
#ifdef SHOW_SERVERGROUP_NAME
	RefreshTitle();
#endif SHOW_SERVERGROUP_NAME
}
#endif // EXTEND_SERVER_GROUP_MASK
#endif	ADD_SERVER_GROUP
//}}  kimhc // 2009-12-15 // 서버군 추가 작업

#ifdef SERVER_NAME
const wstring CX2InstanceData::GetServerGroupName()
{
#ifdef EXTEND_SERVER_GROUP_MASK
#ifdef SERVER_GROUP_UI_ADVANCED
	map<int, KServerGroupInfo>::iterator iter = m_mapServerGroupInfo.find(GetServerGroupID());
	if( iter != m_mapServerGroupInfo.end() )
		return iter->second.m_wstrServerGroupName;
	else
		return std::wstring();
#else
	if( m_mapServerGroupName.end() != m_mapServerGroupName.find(GetServerGroupID()) )
		return m_mapServerGroupName[m_iServerGroupID];
	else
		return std::wstring();
#endif SERVER_GROUP_UI_ADVANCED
#else
	if( m_mapServerGroupName.end() != m_mapServerGroupName.find(GetServerGroupID()) )
		return m_mapServerGroupName[m_eServerGroupID];
	else
		return std::wstring();
#endif // EXTEND_SERVER_GROUP_MASK
}
#endif SERVER_NAME