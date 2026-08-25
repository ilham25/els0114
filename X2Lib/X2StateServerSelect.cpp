#include "StdAfx.h"
#ifndef ELSWORD_NEW_BEGINNING
#include ".\x2stateserverselect.h"

#ifdef CLIENT_PURPLE_MODULE	// 임규수 일본 추가
#include "OnlyGlobal/JP/Auth/PurpleForClient.h"
#endif // CLIENT_PURPLE_MODULE

#ifdef SERVER_GROUP_UI_ADVANCED
const int MAX_SERVERGROUP_SIZE = 5;
#endif SERVER_GROUP_UI_ADVANCED

#ifdef CLIENT_PORT_CHANGE_REQUEST
#define UDP_REQUEST_MAX_COUNT 1 // 2013.05.10 lygan_조성욱 // X2State.cpp 에도 똑같은거 있으니 수정시 같이 수정해줘야 한다.
#endif //CLIENT_PORT_CHANGE_REQUEST

bool CX2StateServerSelect::m_sbSelectedServerSet = false;

// dmlee 2009.03.11
// 기존의 로그인 과정은 로그인 -> 게임서버 접속 -> 계정인증 -> 로딩 100% -> 마을 진입
// 새로 바뀐 로그인 과정은 로그인 -> 로딩 100% -> 채널서버 접속 -> 게임서버선택 -> 게임서버접속 -> 계정인증 -> 마을

// 게임서버 접속 관련 패킷 순서
// connect_req( nexon passport를 갱신받는다) -> verify_account -> port check -> state change server select req -> current time req -> Handler_EGS_SELECT_SERVER_SET_REQ -> Handler_EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ


// 게임서버에 연결이 되면 채널서버 연결을 끊기 위한 REQ를 보낸다. 채널서버에 연결하기전에 게임서버 연결을 끊기 위한 REQ를 보낸다



CX2StateServerSelect::CX2StateServerSelect(void) :
m_pDLGServerSelectBack(NULL),
m_pDLGCheckHack(NULL),
//m_vecUnitSlot;
m_pDLGUnitSelectBack(NULL),
m_pDLGUnitSelectFront(NULL),
m_pDLGDeleteUnitCheck(NULL),
m_pDLGLoadingState(NULL),
m_pDLGRenameMsgBox(NULL),
m_pDLGRenameConfirmMsgBox( NULL ),
m_pDLGRenameSuccessMsgBox( NULL ),
m_pDLGTutorialMsgBox(NULL),
m_pDLGGoToFirstDungeon(NULL),
m_pDLGRecommend(NULL),
m_pDLGRecommendSuccess(NULL),
//m_UnitButtonList;
//m_UnitViewerUIList;
m_pSelectUnit(NULL),
m_pSelectButton(NULL),
m_bUnitClicked(false),
m_fMouseSensitivity(0.01f),
m_fRemainRotY(0.0f),
m_bRotateReturn(true),
m_fRotSpeed(false),
m_pOveredButton(NULL),
m_pOveredName(NULL),
m_bOneTimeInit(true),
m_bStateChangingToUnitCreate(false),
m_bPlayDownAnim(false),
m_fDownAnimTime(0.0f),
m_MaxUnitNum(0),
m_bReserveEnterTutorial(false),
m_bCheckRecommend(false),
m_bCheckRecommendSuccess(false),
m_NowPage(1),
m_MaxPage(1),		// 아래쪽에서 다시 연산
m_bPortCheckWait( false ),
m_fPortCheckWaitTime( 0.f ),
m_PortCheckRetryTime( 0 ),
m_bENX_USER_LOGIN_NOT( false ),
m_bEGS_VERIFY_ACCOUNT_ACK( false ),
m_bIsGameBang( false ),
m_bPortCheckReq( false ),
m_bEGS_VERIFY_ACCOUNT_REQ( false ),
m_fEGS_VERIFY_ACCOUNT_ACK_WaitTime( 0.f ),
#ifdef SERVER_GROUP_UI_ADVANCED
m_bReceivedServerGroupList( false ),
m_pDLGServerGroupSelectBack(NULL),
#endif SERVER_GROUP_UI_ADVANCED
//{{ //2012.02.28 lygan_조성욱 // 버튼에 unitUID를 설정하여 캐릭터 삭제 할때 사용할 용도
#ifdef SERV_UNIT_WAIT_DELETE
m_pFinalDeleteUnitInfo( NULL ),
m_pRestoreUnitInfo( NULL ),
m_pBeforeSelectUnit( NULL ),
#endif SERV_UNIT_WAIT_DELETE
//}}
m_bReceivedChannelList( false ),
m_bSentEGS_CONNECT_REQ( false ),
m_bReceivedEGS_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK( false ),
m_bReservedConnectionToChannelServer( false ), 
m_bWaiting_EGS_DISCONNECT_FOR_SERVER_SELECT_ACK( false ),
m_iTryConnectChannelID( -1 ), 
m_bJustChangedNickName( false ), 
m_NickNameChangeUnitUID( -1 ),
m_NickNameChangeUnitNickName( L"" ),
m_bRemoveUnitAfterReceiveNMSerialNum( false )
//{{ kimhc // 2009-12-15 // 서버군 추가 작업에 따른 채널 다이얼로그 분리
#ifdef	ADD_SERVER_GROUP
, m_pDLGChannelSelectBack( NULL )
, m_iChannelNum( 0 )
//{{ 허상형 : [2011/6/20] //        채널 선택 불편사항 수정
, m_bSelectChannel( false )
//}} 허상형 : [2011/6/20] //        채널 선택 불편사항 수정
#endif	ADD_SERVER_GROUP
//}}  kimhc // 2009-12-15 // 서버군 추가 작업에 따른 채널 다이얼로그 분리
#ifdef SERV_SECOND_SECURITY
, m_bEnableUnitSelectBySecondSecurity( false )
#endif SERV_SECOND_SECURITY
#ifdef REFORM_UI_KEYPAD
, m_bRecivedKeyMap( false )
#endif
#ifdef SERV_CHANGE_NEXON_AUTH_ATL_LEVEL
, m_pDLGATLLimitCheckMsgBox (NULL)
#endif //SERV_CHANGE_NEXON_AUTH_ATL_LEVEL
#ifdef SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE
, m_pDLGEventGuide (NULL)
#endif //SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE
#ifdef MODIFY_ADVERTISEMENT // 적용날짜: 2013-03-28
, m_hMeshAddAdvertisement_Back( INVALID_MESH_INSTANCE_HANDLE )
, m_hMeshAddAdvertisement_Front( INVALID_MESH_INSTANCE_HANDLE )
#endif // MODIFY_ADVERTISEMENT
#ifdef CLIENT_PORT_CHANGE_REQUEST
, m_iUDPRequestCount( 0 )
#endif //CLIENT_PORT_CHANGE_REQUEST
#ifdef ADDED_EVENT_JUMPING_CHARACTER	// 김종훈, 여름방학 이벤트 점핑 캐릭터
, m_pDLGInfoCreateJumpingChar ( NULL )		// 점핑 캐릭터 DLG
#endif // ADDED_EVENT_JUMPING_CHARACTER	// 김종훈, 여름방학 이벤트 점핑 캐릭터
{
#ifndef NEW_SKILL_TREE
	g_pInstanceData->DeleteSkillTreeUI();
#endif


#ifndef NEW_VILLAGE_UI
#ifdef CLIENT_COUNTRY_TW
	m_pDLGUnitSelectBack = new CKTDGUIDialog( this, L"DLG_UI_Character_Selection_Back_TW.lua" );
#elif CLIENT_COUNTRY_HK
	m_pDLGUnitSelectBack = new CKTDGUIDialog( this, L"DLG_UI_Character_Selection_Back_HK.lua" );
#else
	m_pDLGUnitSelectBack = new CKTDGUIDialog( this, L"DLG_UI_Character_Selection_Back.lua" );
#endif
#else
#if defined CLIENT_COUNTRY_TW
	m_pDLGUnitSelectBack = new CKTDGUIDialog( this, L"DLG_Unit_Select_Back_TW.lua" );
#elif defined CLIENT_COUNTRY_HK
	m_pDLGUnitSelectBack = new CKTDGUIDialog( this, L"DLG_Unit_Select_Back_HK.lua" );
#else
	m_pDLGUnitSelectBack = new CKTDGUIDialog( this, L"DLG_Unit_Select_Back.lua" );
#endif
#endif NEW_VILLAGE_UI
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGUnitSelectBack );	

#ifdef MODIFY_ADVERTISEMENT // 적용날짜: 2013-03-28
	AddMeshAdvertisement_Back();
#endif // MODIFY_ADVERTISEMENT

#ifdef NEW_VILLAGE_UI
#ifdef EXPAND_CHANNEL_24
	m_pDLGServerSelectBack = new CKTDGUIDialog( this, L"DLG_UI_Server_Chanal_Selection_Expansion.lua" ); 
#else
#if defined CLIENT_COUNTRY_TW
	m_pDLGServerSelectBack = new CKTDGUIDialog( this, L"DLG_UI_Server_Chanal_Selection_TW.lua" );
#elif defined CLIENT_COUNTRY_HK
	m_pDLGServerSelectBack = new CKTDGUIDialog( this, L"DLG_UI_Server_Chanal_Selection_HK.lua" );
#else
	m_pDLGServerSelectBack = new CKTDGUIDialog( this, L"DLG_UI_Server_Chanal_Selection.lua" );
#endif
#endif //EXPAND_CHANNEL_24
#else
	m_pDLGServerSelectBack = new CKTDGUIDialog( this, L"DLG_Server_Select_Back.lua" );
#endif
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGServerSelectBack );

	
	wstring wstrClientVersion = g_pMain->GetClientVersion();
#ifdef VERSION_DOT
	wstrClientVersion += L".";
#else VERSION_DOT
	wstrClientVersion += L"_";
#endif VERSION_DOT
	wstrClientVersion += g_pMain->GetSubClientVersion_LUA();
	CKTDGUIStatic* pStatic = (CKTDGUIStatic*) m_pDLGServerSelectBack->GetControl( L"Static_Client_Version" );
	if( pStatic != NULL )
	{
		pStatic->GetString(0)->msg = wstrClientVersion;
	}


	g_pKTDXApp->GetDGManager()->GetCamera()->Point( 0,0,-700, 0,0,0 );
	g_pKTDXApp->GetDGManager()->SetProjection( g_pKTDXApp->GetDGManager()->GetNear(), g_pKTDXApp->GetDGManager()->GetFar(), true );

	//GetServerSetDataReq();
#ifdef CHANGE_SERVER_SELECT_BGM
	g_pData->PlayLobbyBGM( L"Lobby2.ogg", true );
#else
	g_pData->PlayLobbyBGM( L"Lobby.ogg", true );
#endif //CHANGE_SERVER_SELECT_BGM

#ifndef OPEN_TEST_1_NO_MESSENGER_CASHSHOP
#ifndef NEW_MESSENGER
	//유닛 선택창

	// fix!! 이 부분 수정해야 합니다. 기존에는 GetMyuser가 NULL되는 경우가 없었으나, 신마을 클라이언트에서는 최초 접속시 getmyuser가 NULL이 됩니다.
	if( NULL != g_pData->GetMyUser() &&
		false == g_pData->GetMyUser()->GetUserData()->m_bIsGuestUser ) // 체험 아이디 제한
	{
		if( g_pMain->GetNexonLoginMessenger() == true && g_pMain->GetNexonVirtualLogin() == true )
		{
			if ( CNMCOClientObject::GetInstance().LogoutVirtual() == FALSE )
				g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2( 250, 300 ), GET_STRING( STR_ID_699 ), this );

			g_pMain->SetNexonVirtualLogin( false );
		}
	}
#endif // #ifndef NEW_MESSENGER
	if ( g_pData->GetMessenger() != NULL )
	{		
		//g_pData->GetMessenger()->ClearChatSessionList();
		g_pData->GetMessenger()->Reset();
	}

#endif OPEN_TEST_1_NO_MESSENGER_CASHSHOP


#ifndef	HEAP_BROKEN_BY_ROOM
	g_pData->DeletePVPRoom();
	g_pData->DeleteDungeonRoom();
	g_pData->DeleteBattleFieldRoom();
#endif // HEAP_BROKEN_BY_ROOM


	if ( g_pMain->GetInformerManager() )
		g_pMain->GetInformerManager()->InvalidInfo();

	g_pMain->GetMemoryHolder()->ReleaseLocalMapDLG();

	//g_pData->DeleteCharacterRoom();
	g_pMain->GetMemoryHolder()->RelaseCommonDLG();

	
#ifdef NEW_VILLAGE_UI
#if defined CLIENT_COUNTRY_TW
	m_pDLGUnitSelectFront = new CKTDGUIDialog( this, L"DLG_UI_Character_Selection_Back_TW.lua" );
#elif defined CLIENT_COUNTRY_HK
	m_pDLGUnitSelectFront = new CKTDGUIDialog( this, L"DLG_UI_Character_Selection_Back_HK.lua" );
#else
	m_pDLGUnitSelectFront = new CKTDGUIDialog( this, L"DLG_UI_Character_Selection_Back.lua" );
#endif
#else
	m_pDLGUnitSelectFront = new CKTDGUIDialog( this, L"DLG_Unit_Select_Front.lua" );
#endif
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGUnitSelectFront );	
	
#ifdef MODIFY_ADVERTISEMENT // 적용날짜: 2013-03-28
	AddMeshAdvertisement_Front();
#endif //MODIFY_ADVERTISEMENT

#ifdef ADD_CREATE_CHARACTER_BUTTON
	//캐릭터 많은 개발자 계정들, 캐릭생성 편하게 하는 기능. 개발사내에만 있어야 함.
	if( NULL != m_pDLGUnitSelectFront )
	{
		CKTDGUIStatic* pStatic = static_cast<CKTDGUIStatic*>(m_pDLGUnitSelectFront->GetControl(L"Create"));
		if( NULL != pStatic )
		{
			pStatic->SetShowEnable(true,true);
		}
	}
#endif //ADD_CREATE_CHARACTER_BUTTON

	g_pKTDXApp->GetDGManager()->GetCamera()->Point( 0,0,-700, 0,0,0 );
	g_pKTDXApp->GetDGManager()->SetProjection( g_pKTDXApp->GetDGManager()->GetNear(),
		g_pKTDXApp->GetDGManager()->GetFar(), false );




	if( NULL != g_pData->GetMyUser() )
	{
		m_MaxUnitNum	= g_pData->GetMyUser()->GetUserData()->maxUnitCount;

		m_NowPage		= 1;
		const int SLOT_COUNT_FOR_CREATE_UNIT = 1;

		if( g_pData->GetMyUser()->GetUnitNum() < m_MaxUnitNum )
		{
			m_MaxPage		= (g_pData->GetMyUser()->GetUnitNum()+SLOT_COUNT_FOR_CREATE_UNIT+UNIT_SELECT_UI_MAX_UNIT_NUM-1) / UNIT_SELECT_UI_MAX_UNIT_NUM;
		}
		else
		{
			m_MaxPage		= (g_pData->GetMyUser()->GetUnitNum()+UNIT_SELECT_UI_MAX_UNIT_NUM-1) / UNIT_SELECT_UI_MAX_UNIT_NUM;
		}
	}

#ifdef FESTIVAL_UI
	if( NULL != g_pMain && NULL != g_pData && NULL != g_pData->GetUIMajorParticle() )
	{
		CKTDGParticleSystem::CParticleEventSequenceHandle SeqFestivalUI = g_pMain->GetFestivalSeq();
		if( INVALID_PARTICLE_HANDLE != SeqFestivalUI)
		{		
			CKTDGParticleSystem::CParticleEventSequence* pParticle = g_pData->GetUIMajorParticle()->GetInstanceSequence( SeqFestivalUI );
			if( pParticle != NULL )
			{
				g_pData->GetUIMajorParticle()->DestroyInstanceHandle( SeqFestivalUI );
				g_pMain->SetFestivalSeq( SeqFestivalUI );
			}
			else
			{
				SeqFestivalUI = INVALID_PARTICLE_HANDLE;
				g_pMain->SetFestivalSeq( SeqFestivalUI );
			}
		}
	}
#endif //FESTIVAL_UI

	//CreateUnitButton();

	if ( m_sbSelectedServerSet == true )
	{
		ClearBeforeUnit( m_pSelectUnit );
		ClearUnitButton();
		CreateUnitButton();

		m_bReceivedEGS_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK = true;

#ifdef SERVER_GROUP_UI_ADVANCED
		if(m_pDLGServerGroupSelectBack != NULL)
			m_pDLGServerGroupSelectBack->SetShowEnable(false, false);
#endif SERVER_GROUP_UI_ADVANCED
		m_pDLGServerSelectBack->SetShowEnable(false, false);		
		m_pDLGUnitSelectFront->SetShowEnable(true, true);
#ifdef SERV_SECOND_SECURITY
		// oasis907 : 김상윤 [2011.6.28] 서버 선택이 되어 있는 상황에서는 2차 보안 체크하지 않으므로. 
		m_bEnableUnitSelectBySecondSecurity = true;
#endif SERV_SECOND_SECURITY

#ifdef SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE
		Handler_EGS_CHECK_THE_ARCHUANGEL_S_COIN_EVENT_LETTER_REQ();
#endif //SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE
	}


	if( NULL != g_pMain->GetTutorSystem() )
	{
		g_pMain->GetTutorSystem()->SetBeenUnitSelect( true );
	}

	if( true == g_pMain->GetIsExitingTutorial() )
	{
		switch( g_pMain->GetStateIDBeforeTutorial() )
		{
		case CX2Main::XS_SERVER_SELECT:
			{
				g_pMain->SetIsExitingTutorial( false );
			} break;
		}
	}

	if( true == g_pMain->GetIsPlayingTutorial() )
	{
		m_pDLGLoadingState = new CKTDGUIDialog( this, L"DLG_PVP_Game_Loading_State.lua" );
		m_pDLGLoadingState->SetColor( D3DXCOLOR(1.0f,1.0f,1.0f,1.0f) );
		// 랜덤 로딩 화면 
		wstringstream wstmStaticName;
		wstmStaticName << L"Static_Loading_BG";
		wstmStaticName << g_pMain->GetTutorialRandomLoadingBG();
		m_pDLGLoadingState->GetControl( wstmStaticName.str().c_str() )->SetShow( true );

		m_pSelectUnit = g_pData->GetMyUser()->GetUnitByIndex( g_pData->GetMyUser()->GetUnitNum() - 1 );
		g_pMain->SetStateIDBeforeTutorial( g_pMain->GetNowStateID() );
		m_bCheckRecommend = true;
		EnterTutorial();
	}



	g_pData->ResetPartyManager(); // 캐릭선택창에서 캐릭터를 바꾸는 경우에 파티매니저 갱신

	if( NULL != g_pData->GetMyUser() )
	{
		g_pData->GetMyUser()->SelectUnit( NULL );
	}

	//{{ kimhc // 2009-12-15 // 이전에 플레이했던 채널 서버군
#ifdef	ADD_SERVER_GROUP
#ifdef EXTEND_SERVER_GROUP_MASK
#else EXTEND_SERVER_GROUP_MASK
	if ( g_pInstanceData->GetServerGroupID() == SGI_INVALID )
		OpenScriptServerGroupFile();
#endif EXTEND_SERVER_GROUP_MASK

#ifdef SERVER_GROUP_UI_ADVANCED
	UpdateServerGroupListUI();
#endif SERVER_GROUP_UI_ADVANCED

	UpdateServerSelectButton( g_pInstanceData->GetServerGroupID() );
	
#endif	ADD_SERVER_GROUP
	//}} kimhc // 2009-12-15 // 이전에 플레이했던 채널 서버군
	UpdateChannelListUI();


//#ifdef _SERVICE_
//	OpenGlobalURL( L"http://Elsword.nexon.com/Elsword/etc/gameinstall.aspx?Section=chceck5" );
//#endif

	//if ( g_pData->GetMyUser()->GetUserData()->hackingUserType == CX2User::HUT_DISAGREE_HACK_USER )
	//{
	//	m_pDLGCheckHack = g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2(250,300), 
	//	L"회사가 제공하지 아니한 프로그램 등을 사용하는 경우, 회사는 해당 프로그램 등의 정보를 수집할 수 있으며 회원은 약관 및 운영정책에 따라 서비스 이용이 제한될 수 있습니다. 동의 하시겠습니까?", 
	//	SSSUCM_HACK_USER_PROCESS_CHECK_OK, this, SSSUCM_HACK_USER_PROCESS_CHECK_CANCEL );
	//}
	//else if ( g_pData->GetMyUser()->GetUserData()->hackingUserType == CX2User::HUT_AGREE_HACK_USER )
	//{
	//	g_pMain->UpdateProcessList();
	//	g_pMain->SendHackMail();
	//}



	g_pData->GetServerProtocol()->SendTRID( ETR_UNIT_DISCONNECT );



	// 게임서버에 접속했다가 시스템 메뉴의 "서버선택" 버튼을 눌러서 돌아온 경우 아직 게임서버와의 연결이 끊어지지 않았다.
	if( false == m_sbSelectedServerSet && 		
		g_pData->GetServerProtocol()->IsConnected() == true )
	{
		m_bENX_USER_LOGIN_NOT		= false;
		m_bEGS_VERIFY_ACCOUNT_REQ	= false;
		m_bEGS_VERIFY_ACCOUNT_ACK	= false;
		m_bPortCheckReq				= false;

		m_bSentEGS_CONNECT_REQ 		= false;
		m_bReceivedChannelList 		= false;
		

		// 게임서버와의 접속을 끊는다
		Handler_EGS_DISCONNECT_FOR_SERVER_SELECT_REQ();
	}
	
	if( false == m_sbSelectedServerSet )
	{
		ConnectToChannelServer();
	}


// 윈도우 title 변경 검사
// #ifdef _SERVICE_
// 	WCHAR wTextWindow[_MAX_PATH] ;
// 	GetWindowText(g_pKTDXApp->GetHWND(),wTextWindow,_MAX_PATH);
// 	if( StrCmpW( wTextWindow, GET_STRING( STR_ID_1 ) ) != 0 )
// 	{
// 		// 윈도우 타이틀 변경됨
// 		g_pMain->SendHackMail_DamageHistory(ANTI_HACK_STRING_MODIFY_TITLE  );
// 		//m_pKTDXApp->SetFindHacking( true );
// 	}
// #endif //_SERVIC

#if 0 // 기능 테스트가 더 필요해 우선 제거한다.
#ifdef CHECKFILTER_IN_MAIN
	if( g_pInstanceData->GetChangeHackList() == false )
		g_pMain->GetCheckWindowInfo()->CheckProcess();
#endif
#endif


#ifdef BANDICAM_RECORDING
	if( g_pMain->IsCapturing() == true )
	{
		g_pMain->Bandi_EndVidioCapture();
	}
#endif

	g_pKTDXApp->SkipFrame();
	
#ifdef GUIDE_FIRST_EPIC_QUEST
	if( NULL != g_pData && NULL != g_pData->GetPlayGuide() )
		g_pData->GetPlayGuide()->SetShowFirstEpicQuestGuide(false);
#endif //GUIDE_FIRST_EPIC_QUEST

#ifdef SHOW_ONLY_ADVERTISEMENT
	m_bHideServerSelectUI = false;
	m_pDLGShowOnlyAdvertise = new CKTDGUIDialog( this, L"DLG_ShowOnlyAdvertisement.lua" );
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGShowOnlyAdvertise );	
#endif // SHOW_ONLY_ADVERTISEMENT


#ifdef ADDED_EVENT_JUMPING_CHARACTER	// 김종훈, 여름방학 이벤트 점핑 캐릭터
	// 전직 선택 가능 알림 창
	m_pDLGInfoCreateJumpingChar = new CKTDGUIDialog( this, L"DLG_UI_JUMPING_EVENT_01.lua" );	
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGInfoCreateJumpingChar );
	m_pDLGInfoCreateJumpingChar->SetShowEnable( false, false );
	
#endif // ADDED_EVENT_JUMPING_CHARACTER	// 김종훈, 여름방학 이벤트 점핑 캐릭터
}

CX2StateServerSelect::~CX2StateServerSelect(void)
{
	//{{ kimhc // 2009-12-15 // 이전에 플레이했던 채널 서버군
#ifdef	ADD_SERVER_GROUP
	SAFE_DELETE_DIALOG( m_pDLGChannelSelectBack );
#endif	ADD_SERVER_GROUP
	//}} kimhc // 2009-12-15 // 이전에 플레이했던 채널 서버군

#ifdef SERVER_GROUP_UI_ADVANCED
	SAFE_DELETE_DIALOG( m_pDLGServerGroupSelectBack );
#endif SERVER_GROUP_UI_ADVANCED
	SAFE_DELETE_DIALOG( m_pDLGServerSelectBack );

	SAFE_DELETE_DIALOG( m_pDLGCheckHack );

	ClearUnitButton();

	SAFE_DELETE_DIALOG( m_pDLGUnitSelectBack );
	SAFE_DELETE_DIALOG( m_pDLGUnitSelectFront );

#ifdef DIALOG_HANDLE_TEST // 2009-8-19 by jintaeks
    SAFE_DELETE_DIALOG_HANDLE( m_pDLGLoadingState );
#else
	SAFE_DELETE( m_pDLGLoadingState ); // note: 얘는 다이얼로그 매니저에서 관리되는 애가 아니라서~ 요렇게
#endif // DIALOG_HANDLE_TEST // 2009-8-19


	SAFE_DELETE_DIALOG( m_pDLGRenameMsgBox );
	SAFE_DELETE_DIALOG( m_pDLGRenameConfirmMsgBox );
	SAFE_DELETE_DIALOG( m_pDLGRenameSuccessMsgBox );

	SAFE_DELETE_DIALOG( m_pDLGTutorialMsgBox );
	SAFE_DELETE_DIALOG( m_pDLGGoToFirstDungeon );
	SAFE_DELETE_DIALOG( m_pDLGRecommend );
#ifdef SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE
	SAFE_DELETE_DIALOG( m_pDLGEventGuide );
#endif //SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE

#ifdef SHOW_ONLY_ADVERTISEMENT
	SAFE_DELETE_DIALOG(m_pDLGShowOnlyAdvertise);
#endif // SHOW_ONLY_ADVERTISEMENT

#ifdef ADDED_EVENT_JUMPING_CHARACTER	// 김종훈, 여름방학 이벤트 점핑 캐릭터
	// 전직 선택 가능 알림창
	SAFE_DELETE_DIALOG ( m_pDLGInfoCreateJumpingChar );
#endif // ADDED_EVENT_JUMPING_CHARACTER	// 김종훈, 여름방학 이벤트 점핑 캐릭터



#ifdef MODIFY_ADVERTISEMENT // 적용날짜: 2013-03-28
	if( INVALID_MESH_INSTANCE_HANDLE != m_hMeshAddAdvertisement_Back )	
		g_pData->GetUIMajorXMeshPlayer()->DestroyInstance( m_hMeshAddAdvertisement_Back );

	if( INVALID_MESH_INSTANCE_HANDLE != m_hMeshAddAdvertisement_Front )
		g_pData->GetUIMajorXMeshPlayer()->DestroyInstance( m_hMeshAddAdvertisement_Front );
#endif // MODIFY_ADVERTISEMENT
}

HRESULT CX2StateServerSelect::OnFrameMove( double fTime, float fElapsedTime )
{
	CX2State::OnFrameMove( fTime, fElapsedTime );


	OnFrameMove_GameServerConnect( fTime, fElapsedTime );

	if( true == m_bReceivedEGS_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK &&
		true == m_bOneTimeInit )
	{
		m_bOneTimeInit = false;

		if( 0 == g_pData->GetMyUser()->GetUnitNum() )
		{
			//{{ kimhc // 2009-12-23 // 서버군에 유닛이 없는 경우의 이동 수정
#ifdef   ADD_SERVER_GROUP
			//{{ 허상형 : [2011/6/20] //        채널 선택 불편사항 수정
			if( false == m_bSelectChannel )
			{
				UnitSelectExit();
				m_bReceivedEGS_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK = false;
				m_bOneTimeInit = true;
			}
			else
#endif   ADD_SERVER_GROUP
			{
				g_pMain->CreateStateChangeDLG( GET_STRING( STR_ID_701 ) );
				g_pKTDXApp->SendGameMessage( XGM_STATE_CHANGE, CX2Main::XS_CREATE_UNIT, NULL, false );
				m_bStateChangingToUnitCreate = true;
			}
			//}} 허상형 : [2011/6/20] //        채널 선택 불편사항 수정
			//}} kimhc // 2009-12-23 // 서버군에 유닛이 없는 경우의 이동 수정

		}
	}


	if ( m_pDLGUnitSelectFront != NULL )
	{
		if ( m_sbSelectedServerSet == true )
		{
#ifdef SERVER_GROUP_UI_ADVANCED
			if(m_pDLGServerGroupSelectBack != NULL)
				m_pDLGServerGroupSelectBack->SetShowEnable(false, false);
#endif SERVER_GROUP_UI_ADVANCED
			m_pDLGServerSelectBack->SetShowEnable(false, false);
			m_pDLGUnitSelectFront->SetShowEnable( true, true );
			//{{ kimhc // 2009-12-15 // 서버군 추가 작업에 따른 채널 다이얼로그 분리
#ifdef	ADD_SERVER_GROUP
			m_pDLGChannelSelectBack->SetShowEnable( false, false );
#endif	ADD_SERVER_GROUP
			//}}  kimhc // 2009-12-15 // 서버군 추가 작업에 따른 채널 다이얼로그 분리

			// 채널번호 표시
			if( g_pInstanceData != NULL && g_pInstanceData->GetConnectChannelServerID() > 0 )
			{
				CKTDGUIStatic* pStatic = (CKTDGUIStatic*)m_pDLGUnitSelectFront->GetControl( L"Channel_Name" );
				if( NULL != pStatic &&
					NULL != pStatic->GetString(0) )
				{
					pStatic->GetString(0)->msg = g_pInstanceData->GetConnectChannelServerName();
				}
			}
#ifdef SHOW_ONLY_ADVERTISEMENT
			if( NULL != m_pDLGShowOnlyAdvertise )
				m_pDLGShowOnlyAdvertise->SetShow(false);
#endif //SHOW_ONLY_ADVERTISEMENT
		}
		else
		{
		
#ifdef SERVER_GROUP_UI_ADVANCED
			if(m_pDLGServerGroupSelectBack != NULL)
				m_pDLGServerGroupSelectBack->SetShowEnable(true, true);
#endif SERVER_GROUP_UI_ADVANCED

#ifdef SHOW_ONLY_ADVERTISEMENT
			if( NULL != m_pDLGShowOnlyAdvertise )
				m_pDLGShowOnlyAdvertise->SetShow(true);

			if( false == m_bHideServerSelectUI )
#endif //SHOW_ONLY_ADVERTISEMENT
			{
				m_pDLGServerSelectBack->SetShowEnable(true, true);
				m_pDLGUnitSelectFront->SetShowEnable( false, false );
				//{{ kimhc // 2009-12-15 // 서버군 추가 작업에 따른 채널 다이얼로그 분리
#ifdef	ADD_SERVER_GROUP
				m_pDLGChannelSelectBack->SetShowEnable( true, true );
#endif	//ADD_SERVER_GROUP
				//}}  kimhc // 2009-12-15 // 서버군 추가 작업에 따른 채널 다이얼로그 분리

			}
		}
	}

	static bool bAutoStart = true;

	if ( bAutoStart == true && g_pMain->m_bAutoStart == true && NULL != g_pData->GetMyUser() )
	{
		bAutoStart = false;

		CX2Unit* pUnit = g_pData->GetMyUser()->GetUnitByIndex( g_pMain->m_AutoLoginUnitIndex );

		if( pUnit == NULL )
			return true;

		UnitButtonUp( pUnit );


		Handler_EGS_SELECT_UNIT_REQ( pUnit->GetUID(), pUnit->GetUnitData()->m_Level );
	}

	//	g_pKTDXApp->GetDGManager()->GetCamera()->UpdateCamera( fElapsedTime );



	UnitHandling( fTime, fElapsedTime );
	DownAnimFrameMove( fTime, fElapsedTime );

	if ( m_pOveredButton != NULL && m_pOveredName != NULL )
	{
		if ( m_pOveredButton->GetMouseOver() == false )
		{
			if ( m_pOveredButton->GetButtonControlState() != CKTDGUIButton::BCS_MOUSEDOWN )
			{
				//SetShowCharacterName( false );
				m_pOveredName->Move( m_pOveredName->GetOffsetPos(), D3DXCOLOR(0,0,0,0), 0.2f, true, false );
				m_pOveredButton = NULL;
				m_pOveredName = NULL;
			}
		}
	}

	//SetShowCharacterNameForSelectedUnit();


#ifndef OPEN_TEST_1_NO_MESSENGER_CASHSHOP

	if ( g_pData->GetMessenger() != NULL )
		g_pData->GetMessenger()->CheckFriendReq();

#endif OPEN_TEST_1_NO_MESSENGER_CASHSHOP



	//if ( m_bCheckPostItem == true )
	//{
	//	if ( g_pKTDXApp->GetDGManager()->GetDialogManager()->CheckDlg( m_pDLGRecommendReward ) == false &&
	//		g_pKTDXApp->GetDGManager()->GetDialogManager()->CheckDlg( m_pDLGTutorReward ) == false &&
	//		g_pKTDXApp->GetDGManager()->GetDialogManager()->CheckDlg( m_pDLGGuestUserReward ) == false )
	//	{
	//		m_bCheckPostItem = false;
	//		Handler_EGS_STATE_CHANGE_VILLAGE_MAP_REQ();
	//	}
	//}

	if ( m_bCheckRecommendSuccess == true )
	{
		if ( g_pKTDXApp->GetDGManager()->GetDialogManager()->CheckDlg( m_pDLGRecommendSuccess ) == false )
		{
			m_bCheckRecommendSuccess = false;
			//Handler_EGS_GET_POST_LETTER_LIST_REQ();
			Handler_EGS_STATE_CHANGE_VILLAGE_MAP_REQ();
		}
	}

	if ( NULL != g_pData->GetMyUser() )
		//g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_DEV )
	{
#ifdef KEY_MAPPING_INT
		if( GET_KEY_STATE( GA_LEFT ) == TRUE )
#else // KEY_MAPPING_INT
		if ( g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState( DIK_LEFT ) == TRUE )
#endif // KEY_MAPPING_INT
		{
			if ( m_NowPage > 1 )
			{
				ClearBeforeUnit( m_pSelectUnit );
				ClearUnitButton();

				m_NowPage--;


				CreateUnitButton();

			}
		}
#ifdef KEY_MAPPING_INT
		else if( GET_KEY_STATE( GA_RIGHT ) == TRUE )
#else // KEY_MAPPING_INT
		else if ( g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState( DIK_RIGHT ) == TRUE )
#endif // KEY_MAPPING_INT
		{
			if ( m_NowPage < m_MaxPage )
			{
				if( (m_NowPage * UNIT_SELECT_UI_MAX_UNIT_NUM) <= g_pData->GetMyUser()->GetUnitNum()+1 )
				{	
					ClearBeforeUnit( m_pSelectUnit );
					ClearUnitButton();

					m_NowPage++;

					CreateUnitButton();
				}
			}
		}
	}

	return S_OK;
}



void CX2StateServerSelect::OnFrameMove_GameServerConnect( double fTime, float fElapsedTime )
{
	if ( m_bPortCheckReq == false && 
		g_pData->GetMyUser() != NULL &&
		m_bENX_USER_LOGIN_NOT == true &&
		m_bEGS_VERIFY_ACCOUNT_ACK == true )
	{
		m_bPortCheckReq = true;


		g_pData->GetMyUser()->SetPCRoom( m_bIsGameBang );

		if( g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_DEV )
		{

#ifndef _SERVICE_
			g_pData->GetServerProtocol()->SendID( E_DISABLE_HB_CHECK_REQ );

			if( NULL == g_fpStream && NULL == g_fpStreamError ) 
			{
				AllocConsole();
				g_fpStream = freopen("CONOUT$", "a", stdout); // redirect printf to console
				g_fpStreamError = freopen("CONOUT$", "a", stderr); // redirect printf to console
				dbg::logfile::SetLogLevel(2);
			}
#endif
		}
		else 
		{
#ifndef _SERVICE_
			FreeConsole();
			if ( g_fpStream != NULL )
				fclose( g_fpStream );
			if ( g_fpStreamError != NULL )
				fclose( g_fpStreamError );
#endif
		}


		Handler_KXPT_PORT_CHECK_REQ();
	}


	if ( m_bEGS_VERIFY_ACCOUNT_REQ == true && m_bEGS_VERIFY_ACCOUNT_ACK == false )
	{
		m_fEGS_VERIFY_ACCOUNT_ACK_WaitTime += fElapsedTime;
		if ( m_fEGS_VERIFY_ACCOUNT_ACK_WaitTime > 180.f )
		{
			//{{ kimhc // 2010-05-07 // 크래시가 아니라 정상 종료 하도록 수정
			// 	 int* x = NULL;	
			// 	 *x = 1;
			g_pKTDXApp->SendGameMessage( XGM_QUIT_GAME, NULL, NULL, false );
			//}} kimhc // 2010-05-07 // 크래시가 아니라 정상 종료 하도록 수정
		}
	}



	if( m_bPortCheckWait == true )
	{
		m_fPortCheckWaitTime -= fElapsedTime;
		if( m_fPortCheckWaitTime < 0.0f )
		{
			m_PortCheckRetryTime++;
			if( m_PortCheckRetryTime < 10 )
			{
				Handler_KXPT_PORT_CHECK_REQ();
			}
			else
			{
				m_bPortCheckWait			= false;
				m_fPortCheckWaitTime		= 0.0f;
				m_PortCheckRetryTime		= 0;

				if( 
#ifdef CLIENT_PORT_CHANGE_REQUEST
					m_iUDPRequestCount < UDP_REQUEST_MAX_COUNT &&
#endif //CLIENT_PORT_CHANGE_REQUEST
					g_pData->GetGameUDP()->IncreaseMyIPIndex() == true 

					)
				{
					Handler_KXPT_PORT_CHECK_REQ();
#ifdef CLIENT_PORT_CHANGE_REQUEST
					m_iUDPRequestCount++;
					if (g_pInstanceData != NULL)
					{
						g_pInstanceData->SetUDPPortSuccessType(1);
					}
#endif //CLIENT_PORT_CHANGE_REQUEST
				}
				else
				{
					//g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), L"포트 체크 응답이 없습니다", this );
					ErrorLogMsg( XEM_ERROR89, L"PortCheckFail" );

#ifdef CLIENT_PORT_CHANGE_REQUEST
					m_iUDPRequestCount = 0;
					if (g_pInstanceData != NULL)
					{
						g_pInstanceData->SetUDPPortSuccessType(2);
					}
#endif //CLIENT_PORT_CHANGE_REQUEST

					KXPT_PORT_CHECK_ACK kXPT_PORT_CHECK_ACK;
#ifdef  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
                    kXPT_PORT_CHECK_ACK.m_IPAddress	= g_pData->GetGameUDP()->GetMyIPAddress();
#else   SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
					kXPT_PORT_CHECK_ACK.m_IP	= g_pData->GetGameUDP()->GetMyIP();
#endif  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
					kXPT_PORT_CHECK_ACK.m_Port	= g_pData->GetGameUDP()->GetMyPort();

#ifdef CLIENT_PORT_CHANGE_REQUEST
					if ( g_pInstanceData != NULL )
					{
						g_pInstanceData->SetStartPortChangeRequest(true);
					}
#endif //CLIENT_PORT_CHANGE_REQUEST

					Handler_KXPT_PORT_CHECK_ACK( kXPT_PORT_CHECK_ACK );
				}				
			}
		}		
	}

	CKTDNUDP::RecvData* pRecvData = g_pData->GetGameUDP()->PopRecvData();
	while( pRecvData != NULL )
	{
		switch( pRecvData->m_ID )
		{
		//case XPT_CONNECT_SERVER_ACK:
		//	{
		//		//if( m_bLoadBalancerWait == true )
		//		//{
		//		//	KSerBuffer ksBuff;
		//		//	ksBuff.Write( pRecvData->m_pRecvBuffer, pRecvData->m_Size );
		//		//	KXPT_CONNECT_SERVER_ACK kXPT_CONNECT_SERVER_ACK;
		//		//	DeSerialize( &ksBuff, &kXPT_CONNECT_SERVER_ACK );
		//		//	Handler_KXPT_CONNECT_SERVER_ACK( &kXPT_CONNECT_SERVER_ACK );
		//		//	
		//		//}
		//	}
		//	break;

		case XPT_PORT_CHECK_ACK:
			{
				if( m_bPortCheckWait == true )
				{
#ifdef  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
                    if ( pRecvData->m_Size == sizeof(KXPT_PORT_CHECK_ACK) )
                    {
                        Handler_KXPT_PORT_CHECK_ACK( *( (const KXPT_PORT_CHECK_ACK*) pRecvData->m_pRecvBuffer ) );
                    }
#else   SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
					KSerBuffer ksBuff;
					ksBuff.Write( pRecvData->m_pRecvBuffer, pRecvData->m_Size );
					KXPT_PORT_CHECK_ACK kXPT_PORT_CHECK_ACK;
					DeSerialize( &ksBuff, &kXPT_PORT_CHECK_ACK );
					Handler_KXPT_PORT_CHECK_ACK( kXPT_PORT_CHECK_ACK );
#endif  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
				}
			}
			break;
		}

		SAFE_DELETE( pRecvData );
		pRecvData = g_pData->GetGameUDP()->PopRecvData();
	}


}

HRESULT CX2StateServerSelect::OnFrameRender()
{
	if( true == m_bStateChangingToUnitCreate )
	{
		return S_OK;
	}

	CX2State::OnFrameRender();

	return S_OK;
}


bool CX2StateServerSelect::EnterTutorial()
{
	if( NULL != m_pSelectUnit )
	{
		m_bReserveEnterTutorial = true;
		Handler_EGS_SELECT_UNIT_REQ( m_pSelectUnit->GetUID(), m_pSelectUnit->GetUnitData()->m_Level );	
		return true;
	}

	return false;
}


bool CX2StateServerSelect::UICustomEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( CX2State::UICustomEventProc( hWnd, uMsg, wParam, lParam ) == true )
		return true;

#ifdef SERV_SECOND_SECURITY
	if( NULL != g_pMain->GetSecurityPad() )
	{
		if( true == g_pMain->GetSecurityPad()->UICustomEventProc( hWnd, uMsg, wParam, lParam ) )
			return true;
	}
#endif SERV_SECOND_SECURITY

#ifdef COME_BACK_REWARD
	if ( NULL != g_pData->GetPremiumBuffManager() )
	{
		if ( true == g_pData->GetPremiumBuffManager()->UICustomEventProc(hWnd, uMsg, wParam, lParam ) )
			return true;
	}
#endif

	switch(wParam)
	{
		case SUSUCM_UNIT_BUTTON_OVER:
			{
				CKTDGUIButton* pButton = (CKTDGUIButton*)lParam;
				return false;
				//return AnimateCharacterName( pButton );				
			}
			break;

		case SUSUCM_UNIT_BUTTON_UP:
			{

#ifdef SERV_SECOND_SECURITY
				if(m_bEnableUnitSelectBySecondSecurity == false)
				{
					return true;
				}
#endif SERV_SECOND_SECURITY



				CKTDGUIButton* pButton = (CKTDGUIButton*)lParam;

				if ( NULL == pButton )
					break;

				
#ifdef FIX_BUTTON_CRASH //JHKang
				for ( UINT i = 0; i < m_vecUnitSlot.size(); ++i )
				{
					CKTDGUIDialogType pUnitSlot =  m_vecUnitSlot[i];

					if ( pUnitSlot == NULL )
						break;

					CKTDGUIButton* pCharButton = reinterpret_cast<CKTDGUIButton*>( pUnitSlot->GetControl( 0 ) );

					if ( NULL != pCharButton )
						pCharButton->SetDownStateAtNormal( false );
				}

				pButton->SetDownStateAtNormal( true );
#else //FIX_BUTTON_CRASH
				pButton->SetDownStateAtNormal( true );

				if ( m_pSelectButton != NULL )
					m_pSelectButton->SetDownStateAtNormal( false );

				if ( m_pSelectButton != pButton )
				{
					m_pSelectButton = pButton;
					m_pSelectButton->SetDownStateAtNormal( true );
				}
#endif //FIX_BUTTON_CRASH

				wstring tempButtonName = pButton->GetName();
				tempButtonName = tempButtonName.substr( 12 );
				UidType unitUID = _wtoi64( tempButtonName.c_str() ); 
				CX2Unit* pUnit = g_pData->GetMyUser()->GetUnitByUID( unitUID );

				if( pUnit == NULL )
					return true;

				UnitButtonUp( pUnit );

				//SetShowCharacterName( false );

				//SetShowCharacterNameForSelectedUnit();

			}
			break;

		case SUSUCM_BACK_TO_LOGIN:
			{
				
			}
			break;

		case SUSUCM_CREATE_UNIT:

#ifdef SERV_SECOND_SECURITY
			if(m_bEnableUnitSelectBySecondSecurity == false)
			{
				return true;
			}
#endif SERV_SECOND_SECURITY

			if ( m_sbSelectedServerSet == false )
			{
				g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_700 ), this );
				return true;
			}

			if ( g_pData->GetMyUser()->GetUnitNum() < m_MaxUnitNum )
			{
				g_pMain->CreateStateChangeDLG( GET_STRING( STR_ID_701 ) );
				g_pKTDXApp->SendGameMessage( XGM_STATE_CHANGE, CX2Main::XS_CREATE_UNIT, NULL, false );
			}
			else
			{
				//if( m_MaxUnitNum == 6 )
				//{
				//	g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), L"캐릭터 슬롯이 부족하여\n더 이상 캐릭터를 생성할 수 없습니다.", this );
				//}
				//else
				//{
					g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_702 ), this );
				//}
			}
			break;	

		case SUSUCM_SELECT_UNIT:
			{
#ifdef SERV_SECOND_SECURITY
				if(m_bEnableUnitSelectBySecondSecurity == false)
				{
					return true;
				}
#endif SERV_SECOND_SECURITY

				if( NULL != m_pSelectUnit )
				{
					m_bReserveEnterTutorial = false;
					Handler_EGS_SELECT_UNIT_REQ( m_pSelectUnit->GetUID(), m_pSelectUnit->GetUnitData()->m_Level );
				}
				return true;
			}

			break;
		case SUSUCM_DELETE_UNIT:
			{	
				if ( m_pSelectUnit != NULL )
				{
					//DeleteUnitReq();

					//{{ 2012.02.20 조효진	캐릭터 삭제 프로세스 변경 (삭제 대기 기간 도입)
#ifdef SERV_UNIT_WAIT_DELETE

					wstring tempNickName = m_pSelectUnit->GetNickName();
					if(tempNickName.length() != 0 )
					{
						if ( tempNickName.find( L"_") != -1 )
						{
							g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_713 ), this );
							return false;
						}
						else
						{
							wstring tempEx = GET_REPLACED_STRING( ( STR_ID_16102, "SI", m_pSelectUnit->GetNickName(), KHanSungPostWordUnicodeWrapper( (WCHAR*) m_pSelectUnit->GetNickName(), STR_ID_198, STR_ID_199 ) ) );
							m_pDLGDeleteUnitCheck = g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2( 250, 300 ), tempEx.c_str(), SUSUCM_DELETE_UNIT_CHECK ,this );
						}
					}

#else SERV_UNIT_WAIT_DELETE
					wstring tempEx = GET_REPLACED_STRING( ( STR_ID_703, "SI", m_pSelectUnit->GetNickName(), KHanSungPostWordUnicodeWrapper( (WCHAR*) m_pSelectUnit->GetNickName(), STR_ID_198, STR_ID_199 ) ) );
					m_pDLGDeleteUnitCheck = g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2( 250, 300 ), tempEx.c_str(), SUSUCM_DELETE_UNIT_CHECK ,this );
#endif SERV_UNIT_WAIT_DELETE
					//}}
				}
				else
				{
					g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_704 ), this );
				}

			}

			break;

		case SUSUCM_DELETE_UNIT_CHECK:
			{
				g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGDeleteUnitCheck, NULL, false );
				m_pDLGDeleteUnitCheck = NULL;
				
#ifdef REMOVE_USER_ONLY_IF_REMOVED_NM_VIRTUAL_USER
				if( true == g_pData->GetMyUser()->GetUserData()->m_bIsGuestUser )
				{
					return Handler_EGS_DELETE_UNIT_REQ();
				}
				else
				{
					m_bRemoveUnitAfterReceiveNMSerialNum = true;
					return Handler_EGS_GET_MY_MESSENGER_SN_REQ( m_pSelectUnit->GetUID() );
				}
#else REMOVE_USER_ONLY_IF_REMOVED_NM_VIRTUAL_USER
				//{{AFX
				return Handler_EGS_DELETE_UNIT_REQ();
				//}}AFX
#endif REMOVE_USER_ONLY_IF_REMOVED_NM_VIRTUAL_USER
				
			}
			break;

			//{{ 2012.02.20 조효진	캐릭터 삭제 프로세스 변경 (삭제 대기 기간 도입)
#ifdef SERV_UNIT_WAIT_DELETE
		case SUSUCM_FINAL_DELETE_UNIT:
			{	

				//{{ 2012.02.28 lygan_조성욱 // 버튼에 캐릭터 UID 정보 빼기
				m_pFinalDeleteUnitInfo = NULL;
				CKTDGUIButton* pButton = (CKTDGUIButton*)lParam;
				wstring tempButtonName = pButton->GetName();
				tempButtonName = tempButtonName.substr( 14 );
				UidType unitUID = _wtoi64( tempButtonName.c_str() ); 
				m_pFinalDeleteUnitInfo = g_pData->GetMyUser()->GetUnitByUID( unitUID );

				if( m_pFinalDeleteUnitInfo == NULL )
				{
					g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_704 ), this );
					return true;
				}
				//}} 2012.02.28 lygan_조성욱 // 버튼에 캐릭터 UID 정보 빼기

				wstring tempEx = GET_REPLACED_STRING( ( STR_ID_16107, "SI", m_pFinalDeleteUnitInfo->GetNickName(), KHanSungPostWordUnicodeWrapper( (WCHAR*) m_pFinalDeleteUnitInfo->GetNickName(), STR_ID_198, STR_ID_199 ) ) );
				m_pDLGDeleteUnitCheck = g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2( 250, 300 ), tempEx.c_str(), SUSUCM_FINAL_DELETE_UNIT_CHECK ,this );

			}

			break;

		case SUSUCM_FINAL_DELETE_UNIT_CHECK:
			{
				g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGDeleteUnitCheck, NULL, false );
				m_pDLGDeleteUnitCheck = NULL;

				return Handler_EGS_FINAL_DELETE_UNIT_REQ();

				// 				if( true == g_pData->GetMyUser().GetUserData().m_bIsGuestUser )
				// 				{
				// 					return Handler_EGS_DELETE_UNIT_REQ();
				// 				}
				// 				else
				// 				{
				// 					m_bRemoveUnitAfterReceiveNMSerialNum = true;
				// 					return Handler_EGS_GET_MY_MESSENGER_SN_REQ( m_pSelectUnit->GetUID() );
				// 				}
			}
			break;

		case SUSUCM_RESTORE_UNIT:
			{	
				//{{ 2012.02.28 lygan_조성욱 // 버튼에 캐릭터 UID 정보 빼기
				m_pRestoreUnitInfo = NULL;
				CKTDGUIButton* pButton = (CKTDGUIButton*)lParam;

				wstring tempButtonName = pButton->GetName();
				tempButtonName = tempButtonName.substr( 15 );
				UidType unitUID = _wtoi64( tempButtonName.c_str() ); 
				assert(errno != ERANGE);	// Overflow!
				m_pRestoreUnitInfo = g_pData->GetMyUser()->GetUnitByUID( unitUID );

				if( m_pRestoreUnitInfo == NULL )
				{
					g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_705 ), this );	// 캐릭터를 선택해 주세요
					return true;
				}
				//}} 2012.02.28 lygan_조성욱 // 버튼에 캐릭터 UID 정보 빼기

				wstring tempEx = GET_REPLACED_STRING( ( STR_ID_16106, "SI", m_pRestoreUnitInfo->GetNickName(), KHanSungPostWordUnicodeWrapper( (WCHAR*) m_pRestoreUnitInfo->GetNickName(), STR_ID_198, STR_ID_199 ) ) );
				m_pDLGDeleteUnitCheck = g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2( 250, 300 ), tempEx.c_str(), SUSUCM_RESTORE_UNIT_CHECK ,this );

			}

			break;

		case SUSUCM_RESTORE_UNIT_CHECK:
			{
				g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGDeleteUnitCheck, NULL, false );
				m_pDLGDeleteUnitCheck = NULL;

				return Handler_EGS_RESTORE_UNIT_REQ();
			}
			break;
#endif SERV_UNIT_WAIT_DELETE
			//}}

		case SUSUCM_UNIT_ANIMATION_DOWN:
			m_bUnitClicked = true;
			break;

		case SUSUCM_UNIT_ANIMATION_UP:
			{
				m_bUnitClicked = false;
			}
			break;

		case SUSUCM_UNIT_ANIMATION_OVER:
			break;

		case SUSUCM_TUTORIAL:
			{
				if( NULL != m_pSelectUnit )
				{
					g_pMain->SetStateIDBeforeTutorial( g_pMain->GetNowStateID() );
					EnterTutorial();
				}
				else
				{
					g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2( 250, 300 ), GET_STRING( STR_ID_705 ), this );
				}
			}
			break;

		case SUSUCM_RENAME_OK:
			{
				if( m_pSelectUnit != NULL && NULL != m_pDLGRenameMsgBox )
				{
					CKTDGUIIMEEditBox* pEditBox = (CKTDGUIIMEEditBox*) m_pDLGRenameMsgBox->GetControl( L"IME_NewNick" );
					if( NULL != pEditBox )
					{
						m_NickNameChangeUnitUID = m_pSelectUnit->GetUID();
						m_NickNameChangeUnitNickName = pEditBox->GetText();

						wstring wstrErrorMessage	= L"";
						if( true == CX2Main::IsInvalidNickName( m_NickNameChangeUnitNickName, wstrErrorMessage ) )
						{
							SetRenameMsgBoxError( wstrErrorMessage );
							return true;
						}

						Handler_EGS_CHANGE_NICK_NAME_REQ( m_NickNameChangeUnitUID, m_NickNameChangeUnitNickName.c_str(), true );
					}
				}
				return true;

			} break;

		case SUSUCM_RENAME_CANCEL:
			{
				OpenRenameMsgBox( false );
				return true;
			} break;

		case SUSUCM_RENAME_CONFIRM_OK:
			{
				g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGRenameConfirmMsgBox, NULL, false );
				m_pDLGRenameConfirmMsgBox = NULL;

				Handler_EGS_CHANGE_NICK_NAME_REQ( m_NickNameChangeUnitUID, m_NickNameChangeUnitNickName.c_str(), false );
				return true;
			} break;


		case SUSUCM_RENAME_CONFIRM_CANCEL:
			{
				g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGRenameConfirmMsgBox, NULL, false );
				m_pDLGRenameConfirmMsgBox = NULL;
				return true;
			} break;


		case SUSUCM_RENAME_SUCCESS_OK:
			{
				g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGRenameSuccessMsgBox, NULL, false );
				m_pDLGRenameSuccessMsgBox = NULL;

				
				m_bJustChangedNickName = true;
				
				const int MAGIC_UNIT_LEVEL = -1;
				Handler_EGS_SELECT_UNIT_REQ( m_NickNameChangeUnitUID, MAGIC_UNIT_LEVEL );
				return true;
			} break;

		case SUSUCM_START_TUTORIAL_OK:
			{
				g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGTutorialMsgBox, NULL, false );
				m_pDLGTutorialMsgBox = NULL;


				g_pMain->SetStateIDBeforeTutorial( g_pMain->GetNowStateID() );
				EnterTutorial();
				return true;
			} break;

		case SUSUCM_START_TUTORIAL_CANCEL:
			{
				g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGTutorialMsgBox, NULL, false );
				m_pDLGTutorialMsgBox = NULL;

				g_pMain->SetIsPlayingTutorial( false );

				m_pDLGGoToFirstDungeon = g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2( 250, 300 ), GET_STRING( STR_ID_706 ), 
					SUSUCM_START_FIRST_DUNGEON_OK, this, SUSUCM_START_FIRST_DUNGEON_CANCEL );


				return true;
			} break;


		case SUSUCM_START_FIRST_DUNGEON_OK:
			{
				g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGGoToFirstDungeon, NULL, false );
				m_pDLGGoToFirstDungeon = NULL;

				if( NULL == m_pSelectUnit )
					return true;

				CX2StateAutoChanger::TARGET_DETAIL targetDetail;
				targetDetail.m_iChannelID = (int) 104; // fix!!! 채널번호 일단 하드코딩
				targetDetail.m_iDungeonID = (int) CX2Dungeon::DI_EL_FOREST_GATE_NORMAL;
				targetDetail.m_iRoomUID = -1;

				g_pMain->GetStateAutoChanger().StartStateChange( (int)g_pMain->GetNowStateID(), CX2Main::XS_DUNGEON_GAME, targetDetail, m_pSelectUnit->GetUID() );

				return true;
			} break;

		case SUSUCM_START_FIRST_DUNGEON_CANCEL:
			{
				g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGGoToFirstDungeon, NULL, false );
				m_pDLGGoToFirstDungeon = NULL;


				if( NULL == m_pSelectUnit )
					return true;

				CX2StateAutoChanger::TARGET_DETAIL targetDetail;
				g_pMain->GetStateAutoChanger().StartStateChange( (int)g_pMain->GetNowStateID(), CX2Main::XS_VILLAGE_MAP, targetDetail, m_pSelectUnit->GetUID() );

				return true;

			} break;

		case SUSUCM_RECOMMEND_OK:
			{
				//REQ날려보자
				CKTDGUIIMEEditBox* pIMEEditBox = (CKTDGUIIMEEditBox*)m_pDLGRecommend->GetControl( L"g_pIMEEditBoxName" );
				if ( pIMEEditBox != NULL )
				{
					return Handler_EGS_RECOMMEND_USER_REQ( pIMEEditBox->GetText() );
				}
			}
			break;

		case SUSUCM_RECOMMEND_CANCEL:
			{
				//그냥 패스 마을맵으로.
				OpenRecommendMsgBox( false );
				//Handler_EGS_GET_POST_LETTER_LIST_REQ();

				return Handler_EGS_RECOMMEND_USER_REQ( L"", true );				
			}
			break;

#ifdef SERV_RECOMMEND_LIST_EVENT
		case SUSUCM_RECOMMEND_NEXT:
			{
				OpenRecommendMsgBox( false );
				return Handler_EGS_STATE_CHANGE_VILLAGE_MAP_REQ();
			}
			break;
#endif //SERV_RECOMMEND_LIST_EVENT


		case SUSUCM_PREV:
			{
				if ( m_NowPage > 1 )
				{
					ClearBeforeUnit( m_pSelectUnit );
					ClearUnitButton();

					--m_NowPage;


					CreateUnitButton();
				}
				//else
				//{
				//	ClearBeforeUnit( m_pSelectUnit );
				//	ClearUnitButton();

				//	m_NowPage = m_MaxPage;

				//	CreateUnitButton();
				//}
			}
			break;

		case SUSUCM_NEXT:
			{
				if ( m_NowPage < m_MaxPage )
				{
					if( (m_NowPage * UNIT_SELECT_UI_MAX_UNIT_NUM) <= g_pData->GetMyUser()->GetUnitNum() + 1)
					{	
						ClearBeforeUnit( m_pSelectUnit );
						ClearUnitButton();

						++m_NowPage;

						CreateUnitButton();
					}
				}
				//else
				//{
				//	ClearBeforeUnit( m_pSelectUnit );
				//	ClearUnitButton();

				//	m_NowPage = 1;

				//	CreateUnitButton();
				//}
			}
			break;

		case SSSUCM_SERVER_CONNECT0:	// 게임 서버 접속
			{	
				// 다른 게임서버와의 연결이 끊어지길 기다리는 중이라면 게임서버에 접속하지 않고 기다리도록 한다.
				if( true == m_bWaiting_EGS_DISCONNECT_FOR_SERVER_SELECT_ACK )		
					return true;
				
				
				CKTDGUIButton* pButton = (CKTDGUIButton*)lParam;
				int iChannelIndex = pButton->GetDummyInt(0)-1;

#ifdef CHANGE_CHANNEL
				const KChannelInfo& channelInfo = g_pInstanceData->m_vecChannelInfo[ iChannelIndex ];
#else
				const KChannelInfo& channelInfo = m_vecChannelInfo[ iChannelIndex ];
#endif
				m_iTryConnectChannelID = channelInfo.m_iChannelID;
				
				//{{ 허상형 : [2011/6/20] //        채널 선택 불편사항 수정
#ifdef ADD_SERVER_GROUP
				m_bSelectChannel = true;
#endif ADD_SERVER_GROUP
				//}} 허상형 : [2011/6/20] //        채널 선택 불편사항 수정

				return Handler_EGS_CONNECT_REQ( channelInfo.m_wstrIP, channelInfo.m_usMasterPort );
				
			} break;

		case SSSUCM_SELECT_SERVER_GROUP:  // 채널 서버 선택 및 접속
			{
				CKTDGUIButton* pButton = (CKTDGUIButton*) lParam;							// fix!! 라디오 버튼으로 변경 해야함 
				int iNewChannelServerIndex = pButton->GetDummyInt(0);

				//{{ kimhc // 2009-12-15 // 서버 그룹 추가
#ifdef	ADD_SERVER_GROUP
				// 선택한 서버군이 바뀌었다면 갱신
#ifdef EXTEND_SERVER_GROUP_MASK
				if ( g_pInstanceData->GetServerGroupID() != iNewChannelServerIndex )
#else
				if ( g_pInstanceData->GetServerGroupID() != SERVER_GROUP_ID( iNewChannelServerIndex ) )
#endif EXTEND_SERVER_GROUP_MASK
				{
#ifdef EXTEND_SERVER_GROUP_MASK
					g_pInstanceData->SetServerGroupID( iNewChannelServerIndex );
#else
					g_pInstanceData->SetServerGroupID( SERVER_GROUP_ID( iNewChannelServerIndex ) );
#endif EXTEND_SERVER_GROUP_MASK
					UpdateServerSelectButton( g_pInstanceData->GetServerGroupID() );	

					if( 
#ifdef  X2OPTIMIZE_X2SERVERPROTOCOL_MULTITHREAD_CRASH_BUG_FIX
                        g_pData->GetServerProtocol() != NULL && g_pData->GetServerProtocol()->IsChUserProxyValid() == true
#else   X2OPTIMIZE_X2SERVERPROTOCOL_MULTITHREAD_CRASH_BUG_FIX
                        g_pData->GetServerProtocol()->GetChUserProxy() != NULL 
#endif  X2OPTIMIZE_X2SERVERPROTOCOL_MULTITHREAD_CRASH_BUG_FIX
                        )
					{
						// 기존에 연결된 채널 서버 연결을 끊고
						// 새로 선택된 채널 서버에 연결한다. 
						Handler_ECH_DISCONNECT_REQ();
						m_bReservedConnectionToChannelServer = true;
					}
					else
					{
						ConnectToChannelServer();
					}
				}
#else	ADD_SERVER_GROUP
				if( g_pMain->GetPickedChannelServerIPIndex() == iNewChannelServerIndex )
				{
					return true;
				}
				else
				{
					if( true == m_sbSelectedServerSet && 
#ifdef  X2OPTIMIZE_X2SERVERPROTOCOL_MULTITHREAD_CRASH_BUG_FIX
                        g_pData->GetServerProtocol() != NULL && g_pData->GetServerProtocol()->IsChUserProxyValid() == true
#else   X2OPTIMIZE_X2SERVERPROTOCOL_MULTITHREAD_CRASH_BUG_FIX
                        g_pData->GetServerProtocol()->GetChUserProxy() != NULL 
#endif  X2OPTIMIZE_X2SERVERPROTOCOL_MULTITHREAD_CRASH_BUG_FIX
                        )
					{
						// 기존에 연결된 채널 서버 연결을 끊고
						// 새로 선택된 채널 서버에 연결한다. 
						Handler_ECH_DISCONNECT_REQ();
					}
					else
					{
						ConnectToChannelServer();
					}
				}
#endif	ADD_SERVER_GROUP
				//}} kimhc // 2009-12-15 // 서버 그룹 추가
								
				return true;

			} break;

			//사용하지 않고 있음.
		//case SSSUCM_SERVER_CONNECT1:
		//	{
		//		return Handler_EGS_SELECT_SERVER_SET_REQ();
		//	}
		//	break;

		//case SSSUCM_SERVER_CONNECT2:
		//	{
		//		return Handler_EGS_SELECT_SERVER_SET_REQ();
		//	}
		//	break;

#ifndef DISABLE_DISAGREE_HACK_USER
		case SSSUCM_HACK_USER_PROCESS_CHECK_OK:
			{
				Handler_EGS_AGREE_HACK_USER_REQ();
			}
			break;

		case SSSUCM_HACK_USER_PROCESS_CHECK_CANCEL:
			{
				g_pKTDXApp->SendGameMessage( XGM_QUIT_GAME, 0, 0, false );
			}
			break;
#endif DISABLE_DISAGREE_HACK_USER
		case SSSUCM_SERVER_SELECT_EXIT:
			{
				CX2State::QuitGame();
			}
			break;

		case SSSUCM_UNIT_SELECT_EXIT:
			{
				UnitSelectExit();		
			}
			break;
#ifdef SERV_CHANGE_NEXON_AUTH_ATL_LEVEL
		case SSSUCM_ATL_LIMIT_CHECK_OK:
			{
				ShellExecute(NULL, L"open", L"https://member.nexon.com/verify/realname.aspx", L"",L"", SW_SHOWNORMAL );
				if( NULL != m_pDLGATLLimitCheckMsgBox )
				{
					g_pKTDXApp->SendGameDlgMessage(XGM_DELETE_DIALOG, m_pDLGATLLimitCheckMsgBox, NULL, false);
				}
				g_pKTDXApp->SendGameMessage( XGM_QUIT_GAME, 0, 0, false );
			}
			break;
#endif //SERV_CHANGE_NEXON_AUTH_ATL_LEVEL
#ifdef SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE
		case SSSUCM_EVENT_GUIDE_OK:
			{
				if( NULL != m_pDLGEventGuide )
				{
					m_pDLGEventGuide->SetShowEnable(false,false);
				}
			}
			break;
#endif //SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE

#ifdef SHOW_ONLY_ADVERTISEMENT
		case SSSUCM_HIDE_UI_MOUSE_OVER:
			{
				SetShowServerSelectUI( false );
			} break;
		case SSSUCM_HIDE_UI_MOUSE_OUT:
			{
				SetShowServerSelectUI( true );
			} break;
#endif //SHOW_ONLY_ADVERTISEMENT


#ifdef ADDED_EVENT_JUMPING_CHARACTER	// 김종훈, 여름방학 이벤트 점핑 캐릭터
		// 전직 선택 가능 알림창 제거
		case SSSUCM_JUMPING_CHARACTER_OK:
			{
				if( NULL != m_pDLGInfoCreateJumpingChar )
					m_pDLGInfoCreateJumpingChar->SetShowEnable(false, false);
			} break;
#endif // ADDED_EVENT_JUMPING_CHARACTER	// 김종훈, 여름방학 이벤트 점핑 캐릭터			


#ifdef NEW_OPTION_SECURITYPAD
		case SSSUCM_SECURITYPAD:
			{
				if( NULL != g_pMain && NULL != g_pMain->GetSecurityPad() )
					g_pMain->GetSecurityPad()->SetShow(true, CX2SecurityPad::SPS_CREATE_PASSWORD);
			}
			break;
#endif NEW_OPTION_SECURITYPAD
	}

	return false;
}

bool CX2StateServerSelect::UIServerEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( CX2State::UIServerEventProc( hWnd, uMsg, wParam, lParam ) == true )
		return true;

#ifdef POSTBOX
    if( g_pMain->GetPostBox() != NULL)
    {
        if( g_pMain->GetPostBox()->UIServerEventProc( hWnd, uMsg, wParam, lParam ) == true )
            return true;
    }
#endif

#ifdef SERV_SECOND_SECURITY
	if( g_pMain->GetSecurityPad() != NULL )
	{
		if( true == g_pMain->GetSecurityPad()->UIServerEventProc( hWnd, uMsg, wParam, lParam ) )
			return true;
	}
#endif SERV_SECOND_SECURITY
	
	switch( wParam )
	{
#ifdef SERV_SELECT_UNIT_PACKET_DIVISION		/// 페킷의 크기가 최대 허용 크기를 넘겨버려서, 분할
	case EGS_SELECT_UNIT_1_NOT:		/// 유닛의 정보, 인벤토리, 스킬
		return Handler_EGS_SELECT_UNIT_1_NOT( hWnd, uMsg, wParam, lParam );
		break;
	case EGS_SELECT_UNIT_2_NOT:		/// 퀘스트
		return Handler_EGS_SELECT_UNIT_2_NOT( hWnd, uMsg, wParam, lParam );
		break;
	case EGS_SELECT_UNIT_3_NOT:		/// 타이틀, 펫
		return Handler_EGS_SELECT_UNIT_3_NOT( hWnd, uMsg, wParam, lParam );
		break;
	case EGS_SELECT_UNIT_4_NOT:		/// 접속 이벤트, 차단, 사제, 랭킹
		return Handler_EGS_SELECT_UNIT_4_NOT( hWnd, uMsg, wParam, lParam );
		break;
	case EGS_SELECT_UNIT_5_NOT:		/// ETC, 1회성 이벤트
		return Handler_EGS_SELECT_UNIT_5_NOT( hWnd, uMsg, wParam, lParam );
		break;
#endif SERV_SELECT_UNIT_PACKET_DIVISION
	case EGS_SELECT_UNIT_ACK:		/// m_iOK 용
		return Handler_EGS_SELECT_UNIT_ACK( hWnd, uMsg, wParam, lParam );
		break;
#if 0 
	case EGS_STATE_CHANGE_VILLAGE_MAP_ACK:
		return Handler_EGS_STATE_CHANGE_VILLAGE_MAP_ACK( hWnd, uMsg, wParam, lParam );
		break;
#endif

	case EGS_DELETE_UNIT_ACK:
		return Handler_EGS_DELETE_UNIT_ACK( hWnd, uMsg, wParam, lParam );
		break;


	case EGS_CHANGE_NICK_NAME_ACK:
		{
			return Handler_EGS_CHANGE_NICK_NAME_ACK( hWnd, uMsg, wParam, lParam );
		} break;

	case EGS_RECOMMEND_USER_ACK:
		{
			return Handler_EGS_RECOMMEND_USER_ACK( hWnd, uMsg, wParam, lParam );
		}
		break;

	//case EGS_GET_POST_LETTER_LIST_ACK:
	//	{
	//		return Handler_EGS_GET_POST_LETTER_LIST_ACK( hWnd, uMsg, wParam, lParam );
	//	}
	//	break;

	// remove!!		
	//case EGS_ITEM_EXPIRATION_NOT:
	//	{
	//		return Handler_EGS_ITEM_EXPIRATION_NOT( hWnd, uMsg, wParam, lParam );
	//	}
	//	break;



	case EGS_SELECT_SERVER_SET_ACK:
		return Handler_EGS_SELECT_SERVER_SET_ACK( hWnd, uMsg, wParam, lParam );
		break;

	case EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK:
		return Handler_EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK( hWnd, uMsg, wParam, lParam );
		break;

	case EGS_AGREE_HACK_USER_ACK:
		return Handler_EGS_AGREE_HACK_USER_ACK( hWnd, uMsg, wParam, lParam );
		break;

	case EGS_STATE_CHANGE_FIELD_ACK:
		return Handler_EGS_STATE_CHANGE_FIELD_ACK( hWnd, uMsg, wParam, lParam );
		break;

	case EGS_CHANGE_USER_LIST_IN_SECTOR_NOT:
		return Handler_EGS_CHANGE_USER_LIST_IN_SECTOR_NOT( hWnd, uMsg, wParam, lParam );
		break;

	case EGS_CREATE_TUTORIAL_ROOM_ACK:
		return Handler_EGS_CREATE_TUTORIAL_ROOM_ACK( hWnd, uMsg, wParam, lParam );
		break;

//#ifdef SERV_KOG_OTP_VERIFY  -- 2009-08-31  최육사 : 주석처리함
	case ECH_VERIFY_ACCOUNT_ACK:
		return Handler_ECH_VERIFY_ACCOUNT_ACK(hWnd, uMsg, wParam, lParam);
		break;
//#endif


	case EGS_CONNECT_ACK:
		{
			return Handler_EGS_CONNECT_ACK( hWnd, uMsg, wParam, lParam );
		} break;

	case EGS_VERIFY_ACCOUNT_ACK:
		{
			return Handler_EGS_VERIFY_ACCOUNT_ACK( hWnd, uMsg, wParam, lParam );
		} break;

	case ENX_USER_LOGIN_NOT:
		{
			return Handler_ENX_USER_LOGIN_NOT( hWnd, uMsg, wParam, lParam );
		} break;

	case EGS_CURRENT_TIME_ACK:
		{
			return Handler_EGS_CURRENT_TIME_ACK( hWnd, uMsg, wParam, lParam );
		} break;

#ifdef SERVER_GROUP_UI_ADVANCED
	case ECH_GET_SERVERGROUP_LIST_ACK:
		{
			return Handler_ECH_GET_SERVERGROUP_LIST_ACK( hWnd, uMsg, wParam, lParam );
		} break;

	case ECH_GET_SERVERGROUP_LIST_NOT:
		{
			return Handler_ECH_GET_SERVERGROUP_LIST_NOT( hWnd, uMsg, wParam, lParam );
		} break;
#endif SERVER_GROUP_UI_ADVANCED

	case ECH_GET_CHANNEL_LIST_ACK:
		{
			return Handler_ECH_GET_CHANNEL_LIST_ACK( hWnd, uMsg, wParam, lParam );
		} break;

	case ECH_GET_CHANNEL_LIST_NOT:
		{
			return Handler_ECH_GET_CHANNEL_LIST_NOT( hWnd, uMsg, wParam, lParam );
		} break;


	case ECH_DISCONNECT_ACK:
		{
			return Handler_ECH_DISCONNECT_ACK( hWnd, uMsg, wParam, lParam );
		} break;

	case EGS_DISCONNECT_FOR_SERVER_SELECT_ACK:
		{
			return Handler_EGS_DISCONNECT_FOR_SERVER_SELECT_ACK( hWnd, uMsg, wParam, lParam );
		} break;

	case EGS_GET_MY_MESSENGER_SN_ACK:
		{
			return Handler_EGS_GET_MY_MESSENGER_SN_ACK( hWnd, uMsg, wParam, lParam );
		} break;

		//{{ kimhc // 2010-01-04 // 기존의 EGS_SELECT_UNIT_ACK에 인벤정보가 포함되어 오던 것 분할
#ifdef	PC_BANG_WORK
	case EGS_SELECT_UNIT_INVENTORY_INFO_NOT:
		{
			return Handler_EGS_SELECT_UNIT_INVENTORY_INFO_NOT( hWnd, uMsg, wParam, lParam );
		} break;

	case EGS_GET_MY_INVENTORY_ACK:
		{
			return Handler_EGS_GET_MY_INVENTORY_ACK( hWnd, uMsg, wParam, lParam );
		} break;
#endif	PC_BANG_WORK
		//}} kimhc // 2010-01-04 // 기존의 EGS_SELECT_UNIT_ACK에 인벤정보가 포함되어 오던 것 분할

#ifdef SERV_SECOND_SECURITY
	case EGS_SECOND_SECURITY_INFO_NOT:
		{
			return Handler_EGS_SECOND_SECURITY_INFO_NOT( hWnd, uMsg, wParam, lParam );
		} break;
#endif SERV_SECOND_SECURITY
		//{{ //2012.03.05 lygan_조성욱 // 조효진이 작업 누락된부분 추가 작업
#ifdef SERV_UNIT_WAIT_DELETE
	case EGS_FINAL_DELETE_UNIT_ACK:
		{
			return Handler_EGS_FINAL_DELETE_UNIT_ACK( hWnd, uMsg, wParam, lParam );
		} break;
	case EGS_RESTORE_UNIT_ACK:
		{
			return Handler_EGS_RESTORE_UNIT_ACK( hWnd, uMsg, wParam, lParam );
		} break;
#endif //SERV_UNIT_WAIT_DELETE
		//}}
#ifdef REFORM_UI_KEYPAD
	case EGS_KEYBOARD_MAPPING_INFO_NOT:
		{
			return Handler_EGS_KEYBOARD_MAPPING_INFO_NOT( hWnd, uMsg, wParam, lParam );
		} break;
#endif
	case EGS_CHAT_OPTION_INFO_NOT:
		{
			return Handler_EGS_CHAT_OPTION_INFO_NOT( hWnd, uMsg, wParam, lParam ); 
		} break;

#ifdef SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE
	case EGS_CHECK_THE_ARCHUANGEL_S_COIN_EVENT_LETTER_ACK:
		{
			return Handler_EGS_CHECK_THE_ARCHUANGEL_S_COIN_EVENT_LETTER_ACK( hWnd, uMsg, wParam, lParam );
		} break;
#endif //SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE

#ifdef SERV_FIRST_SELECT_UNIT_REWARD_SYSTEM
	case EGS_EXIST_FIRST_SELECT_UNIT_REWARD_NOT:
		{
			return Handler_EGS_EXIST_FIRST_SELECT_UNIT_REWARD_NOT( hWnd, uMsg, wParam, lParam );
		}break;
#endif //SERV_FIRST_SELECT_UNIT_REWARD_SYSTEM
#ifdef SERV_CLIENT_DIRECT_CONNECT_AUTH_PCBANG
	case EGS_CLIENT_DIRECT_CONNECT_AUTH_PCBAG_ACK:
		{
			return Handler_EGS_CLIENT_DIRECT_CONNECT_AUTH_PCBAG_ACK( hWnd, uMsg, wParam, lParam );
		}break;
#endif //SERV_CLIENT_DIRECT_CONNECT_AUTH_PCBANG
#ifdef ADDED_EVENT_JUMPING_CHARACTER	// 김종훈, 여름방학 이벤트 점핑 캐릭터
	case EGS_JUMPING_CHARACTER_GUIDE_NOT :
		{
			// 전직 선택 가능 알림창을 띄울 것인가?
			return Handler_EGS_JUMPING_CHARACTER_GUIDE_NOT ( hWnd, uMsg, wParam, lParam);	
		} break;
#endif // ADDED_EVENT_JUMPING_CHARACTER	// 김종훈, 여름방학 이벤트 점핑 캐릭터
	}
	return false;
}






void CX2StateServerSelect::CreateUnitButton()
{
	//유닛 개수를 확인한다.
	//각 유닛 종류에 맞춰 UnitButton을 생성한다.
	//버튼 이름을 유닛 닉네임으로 바꾼다.
	//인덱스에 따라 위치를 바꾼다.
#if 1 
	CKTDGUIDialogType pUnitSlot = NULL;
	CKTDGUIButton* pButton = NULL;

	D3DXVECTOR3 dummyPos;
	bool bAllSlotOccupiedInAPage = false;	// 한번에 볼 수 있는 캐릭터 슬롯수 만큼 캐릭터가 모두 있는지, 없다면 마지막 빈곳에 캐릭터 생성슬롯을 만들기 위해서


	for(UINT i=0; i<m_vecUnitSlot.size(); ++i)
	{
		CKTDGUIDialogType pUnitSlot =  m_vecUnitSlot[i];
		SAFE_DELETE_DIALOG(pUnitSlot);
	}
	m_vecUnitSlot.resize(0);
	
	//if(g_pData->GetMyUser()->GetUnitNum() > 0)
	//{
	//	m_MaxPage = ((g_pData->GetMyUser()->GetUnitNum()-1) / UNIT_SELECT_UI_MAX_UNIT_NUM) + 1;
	//}
	//else
	//{
	//	m_MaxPage = 1;
	//}

	CKTDGUIStatic* pStaticUnitPage = (CKTDGUIStatic*)m_pDLGUnitSelectFront->GetControl( L"number" );
	WCHAR buff[256] = {0,};
	//wsprintf( buff, L"%d / %d", m_NowPage, m_MaxPage );	
	StringCchPrintf( buff, 256, L"%d / %d", m_NowPage, m_MaxPage );	
	pStaticUnitPage->GetString(0)->msg = buff;

	
#ifdef CONVERSION_VS
	int i = (m_NowPage - 1)*UNIT_SELECT_UI_MAX_UNIT_NUM;
	int j = 0;
	for( ; i < g_pData->GetMyUser()->GetUnitNum() && j < UNIT_SELECT_UI_MAX_UNIT_NUM; i++, j++ )
#else CONVERSION_VS
	for( int i = (m_NowPage - 1)*UNIT_SELECT_UI_MAX_UNIT_NUM, j = 0; i < g_pData->GetMyUser()->GetUnitNum() && j < UNIT_SELECT_UI_MAX_UNIT_NUM; i++, j++ )
#endif CONVERSION_VS
	{
		CX2Unit* pUnit = g_pData->GetMyUser()->GetUnitByIndex( i );
		if( pUnit == NULL )
			continue;	
		
		dummyPos = m_pDLGUnitSelectFront->GetDummyPos( j );		

		pUnitSlot = new CKTDGUIDialog( this, L"DLG_UI_Character_Selection_Slot.lua" );
		g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( pUnitSlot );
		pUnitSlot->SetShowEnable(true, true);
		m_vecUnitSlot.push_back(pUnitSlot);

		CKTDGUIStatic* pStaticEmblem = (CKTDGUIStatic*)pUnitSlot->GetControl( L"UnitEmblem" );
		pStaticEmblem->GetPicture(0)->SetShow( true );

		if ( NULL != g_pMain->GetPVPEmblem() )
		{
#ifdef PVP_SEASON2
		CX2PVPEmblem::PVPEmblemData* pPVPEmblemData = g_pMain->GetPVPEmblem()->GetPVPEmblemData( pUnit->GetPvpRank() );
#else
		CX2PVPEmblem::PVPEmblemData* pPVPEmblemData = g_pMain->GetPVPEmblem()->GetPVPEmblemData( pUnit->GetPVPEmblem() );
#endif
			if ( pPVPEmblemData != NULL )
			{
				pStaticEmblem->GetPicture(0)->SetTex( pPVPEmblemData->m_TextureName.c_str(), pPVPEmblemData->m_TextureKey.c_str() );
			}
		}
		


		CX2UnitViewerUI* pUnitViewerUI = CX2UnitViewerUI::CreateUnitViewerUI();
		//pUnitViewerUI->SetLightPos( 1000, 1000, -200 );
		pUnitViewerUI->SetLightPos( 300, 300, -500 );	// 캐릭터뷰어 라이트 위치 변경
		g_pKTDXApp->GetDGManager()->AddObjectChain( pUnitViewerUI );

		D3DXVECTOR3 vUnitPos = dummyPos;
		vUnitPos.x -= 400.f; // 222
		vUnitPos.y -= 110.f; // 302
		
#ifdef RAVEN_SECOND_CLASS_CHANGE
		// 레이븐 기럭지가 너무 길어서 이름을 가리는 관계로 레이븐만 약간 위로 올려준다. 
		if( pUnit != NULL && (pUnit->GetType() == CX2Unit::UT_RAVEN || pUnit->GetType() == CX2Unit::UT_EVE) )
			vUnitPos.y += 5.f;
#endif

		//{{ kimhc // 2010-01-06 // PC방 프리미엄 서비스
#ifdef	PC_BANG_WORK
		if ( pUnit->GetInventory() != NULL )
			pUnit->GetInventory()->RemovePCBangEquips();
#endif	PC_BANG_WORK
		//}} kimhc // 2010-01-06 // PC방 프리미엄 서비스

#ifdef FIX_CLASS_CHANGE_MESH
		pUnitViewerUI->SetClassChange( false );
#endif

#ifdef SEPARATION_MOTION
		pUnitViewerUI->SetUnit( pUnit, CX2UnitViewerUI::UVS_SELECTUNIT ); 
#else
		pUnitViewerUI->SetUnit( pUnit );
#endif
		//pUnitViewerUI->SetOverUI(true);
		pUnitViewerUI->GetMatrix().Move( vUnitPos );
		pUnitViewerUI->GetMatrix().Scale( D3DXVECTOR3( 1.2f, 1.2f, 1.2f ) * pUnitViewerUI->GetScale() );
		pUnitViewerUI->GetMatrix().Rotate(D3DXToRadian(0),D3DXToRadian(0),D3DXToRadian(0));
		//pUnitViewerUI->PlayAnim( L"LobbyFullDown", CKTDGXSkinAnim::XAP_ONE_WAIT );

		pUnitViewerUI->OnFrameMove( g_pKTDXApp->GetTime(),g_pKTDXApp->GetElapsedTime() );

		pUnitViewerUI->SetShowObject( false );

		m_UnitViewerUIList.push_back( pUnitViewerUI );

		pButton = (CKTDGUIButton*)pUnitSlot->GetControl( L"character1" );

		switch( pUnit->GetType() )
		{
		case CX2Unit::UT_ELSWORD:
		case CX2Unit::UT_LIRE:
		case CX2Unit::UT_ARME:
		case CX2Unit::UT_RAVEN:
		case CX2Unit::UT_EVE:
	//{{ kimhc // 2010.11.24 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
		case CX2Unit::UT_CHUNG:
#endif	NEW_CHARACTER_CHUNG
	//}} kimhc // 2010.11.24 //  2010-12-23 New Character CHUNG
#ifdef ARA_CHARACTER_BASE
		case CX2Unit::UT_ARA:
#endif
		case CX2Unit::UT_ELESIS:
			break;


		default:
			{
				g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_707 ), this );
			}break;
		}
		XSkinMeshReady( pUnit->GetUnitTemplet()->m_MotionFile.c_str() );


		wstringstream buttonName;
		buttonName << L"SLOT_BUTTON_" << pUnit->GetUID();
		if( NULL != pButton )
		{
			pButton->SetName( buttonName.str().c_str() );
		}
		
		//{{ 2012.02.20 조효진	캐릭터 삭제 프로세스 변경 (삭제 대기 기간 도입)
#ifdef SERV_UNIT_WAIT_DELETE
		
		// 초기에 값을 세팅하고 여기서는 생성과 출력만 하는게 깔끔할듯
		if( pUnit == NULL )
			continue;	

		CX2Unit::UnitData* pUnitData = pUnit->GetUnitData();
		
		// 지워진 캐릭인 경우 캐릭 위에 다이얼로그 창을 출력해야 함 
		if( NULL == pUnitViewerUI->m_pNewDeleteDlg )
		{
			pUnitViewerUI->m_pNewDeleteDlg = new CKTDGUIDialog( this, L"DLG_UI_Character_Selection_Slot_Delete.lua" );
			g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( pUnitViewerUI->m_pNewDeleteDlg );
			pUnitViewerUI->m_pNewDeleteDlg->SetShowEnable(true, true);
		}

		// 창 생성 정상적으로 처리 됐으면
		if( NULL != pUnitViewerUI->m_pNewDeleteDlg )
		{


			//{{ 2012.02.27 lygan_조성욱 // 캐릭터 삭제 대기 문구와 배경 표시용
			CKTDGUIStatic* pStatic_UnitDeleteInfo = (CKTDGUIStatic*) (pUnitViewerUI->m_pNewDeleteDlg)->GetControl( L"StaticUnitSelectDeleteInfo" );
			CKTDGUIStatic* pStatic_UnitDeleteInfoName = (CKTDGUIStatic*) (pUnitViewerUI->m_pNewDeleteDlg)->GetControl( L"StaticUnitSelectDeleteInfoName" );

			if(NULL != pStatic_UnitDeleteInfo)
			{
				if( pUnitData->m_bDeleted )
				{
					
					//{{ //2012.03.06 lygan_조성욱 // 삭제후 1일지나서 복구 가능한 날짜 알림용
					std::wstring year = L"";
					std::wstring month = L"";
					std::wstring day = L"";
					std::wstring hour = L"";
					std::wstring min = L"";

					CTime tDeleAbleDate(pUnitData->m_trDelAbleDate);

					year = (CStringW)( tDeleAbleDate.Format(_T( "%Y" )) );
					month = (CStringW)( tDeleAbleDate.Format(_T( "%m" )) );
					day = (CStringW)( tDeleAbleDate.Format(_T( "%d" )) );
					hour = (CStringW)( tDeleAbleDate.Format(_T( "%H" )) );
					min = (CStringW)( tDeleAbleDate.Format(_T( "%M" )) );

#ifdef CLIENT_GLOBAL_LINEBREAK
					wstring wstrMessage = CWordLineHandler::GetStrByLineBreakInX2Main(GET_REPLACED_STRING( ( STR_ID_16109, "SSSSS", day.c_str(), month.c_str(), year.c_str(),hour.c_str(), min.c_str())), 200, XUF_DODUM_15_BOLD );
					pStatic_UnitDeleteInfo->GetString(0)->msg = wstrMessage;
#else //CLIENT_GLOBAL_LINEBREAK
	#ifdef CLIENT_GLOBAL_LINEBREAK_FOR_CN
					wstring wstrMessage = g_pMain->GetStrByLienBreak(GET_REPLACED_STRING( ( STR_ID_16109, "SSSSS", day.c_str(), month.c_str(), year.c_str(),hour.c_str(), min.c_str())), 180, pStatic_UnitDeleteInfo->GetString(0)->fontIndex );
					pStatic_UnitDeleteInfo->GetString(0)->msg = wstrMessage;
	#else //CLIENT_GLOBAL_LINEBREAK_FOR_CN
					pStatic_UnitDeleteInfo->GetString(0)->msg = GET_REPLACED_STRING( ( STR_ID_16109, "SSSSS", day.c_str(), month.c_str(), year.c_str(),hour.c_str(), min.c_str()));
	#endif //CLIENT_GLOBAL_LINEBREAK_FOR_CN
#endif //CLIENT_GLOBAL_LINEBREAK

					
					


					pStatic_UnitDeleteInfo->SetOffsetPos(D3DXVECTOR2(dummyPos.x, dummyPos.y));
					pStatic_UnitDeleteInfoName->SetOffsetPos(D3DXVECTOR2(dummyPos.x, dummyPos.y));

					

					CKTDGUIStatic* pStaticID = (CKTDGUIStatic*)pUnitSlot->GetControl( L"StaticUnitSelectStringID" );
					pStaticID->SetShow(false);

					wstring tempNickName = pUnit->GetNickName();
					if ( tempNickName.find( L"_") != -1 )
					{
						pStatic_UnitDeleteInfoName->GetString(0)->msg = L"     ?";
					}
					else
					{
						pStatic_UnitDeleteInfoName->GetString(0)->msg = pUnit->GetNickName();
					}

					pStatic_UnitDeleteInfo->SetShow(true);
					pStatic_UnitDeleteInfoName->SetShow(true);
					

				}
				else
				{
					pStatic_UnitDeleteInfo->SetShow(false);
					pStatic_UnitDeleteInfoName->SetShow(false);
					
				}
			}
			//}}  2012.02.27 lygan_조성욱 // 캐릭터 삭제 대기 문구와 배경 표시용

			// 최종 삭제 버튼 //
			CKTDGUIButton* pBtn_FinalDelete = (CKTDGUIButton*)(pUnitViewerUI->m_pNewDeleteDlg)->GetControl( L"final_delete_unit" );
			if(NULL != pBtn_FinalDelete)
			{
				if( pUnitData->m_bDeleted )
				{
					
					pBtn_FinalDelete->SetOffsetPos(D3DXVECTOR2(dummyPos.x, dummyPos.y)); //2012.02.27 lygan_조성욱 // 캐릭터 삭제 정보 UI 캐릭터 위치에 맞게 자동 조정용
					bool bCheckEnable = false;


					//CTime cClientTime = CTime::GetCurrentTime();
					//__int64 tint64TimeGap = g_pData->GetServerCurrentTime() - cClientTime.GetTime();


					//if(pUnitData->m_trDelAbleDate <  ( g_pData->GetServerCurrentTime() + tint64TimeGap) ) // 2013.01.30 lygan_조성욱 // 시차가 다른 지역에서 게임을 할때도 버튼이 활성화 되게 처리하기
					if(pUnitData->m_trDelAbleDate <  g_pData->GetServerCurrentTime64() )
						bCheckEnable = true;
					
					pBtn_FinalDelete->SetShowEnable(true,bCheckEnable);
					
					//{{ 2012.02.28 lygan_조성욱 // 버튼에 캐릭터 UID 정보 넣기
					if( NULL != pBtn_FinalDelete )
					{

						wstring buttonNameDelete = boost::str(boost::wformat(L"DELETE_BUTTON_%1%") %pUnit->GetUID());

						if( NULL != pBtn_FinalDelete )
						{
							pBtn_FinalDelete->SetName( buttonNameDelete.c_str() );
						}
						/*wstringstream buttonNameDelete;
						buttonNameDelete.clear();
						buttonNameDelete << L"SLOT_BUTTON_" << pUnit->GetUID();
						if( NULL != pBtn_FinalDelete )
						{
							pBtn_FinalDelete->SetName( buttonNameDelete.str().c_str() );
						}*/


						//StringCchPrintfW( buttonName.c_str(), ARRAY_SIZE( buttonName.c_str() ), L"SLOT_BUTTON_%I64d", pUnit->GetUID() );
						
					}
					//}}
				}
				else
				{
					pBtn_FinalDelete->SetShowEnable(false,false);
				}
			}

			// 복구 버튼 //
			CKTDGUIButton* pBtn_Restore = (CKTDGUIButton*)(pUnitViewerUI->m_pNewDeleteDlg)->GetControl( L"restore_unit" );
			if(NULL != pBtn_Restore)
			{
				if( pUnitData->m_bDeleted )
				{
					pBtn_Restore->SetOffsetPos(D3DXVECTOR2(dummyPos.x, dummyPos.y)); //2012.02.27 lygan_조성욱 // 캐릭터 삭제 정보 UI 캐릭터 위치에 맞게 자동 조정용
					pBtn_Restore->SetShowEnable(true,true);
					//{{ 2012.02.28 lygan_조성욱 // 버튼에 캐릭터 UID 정보 넣기
					if( NULL != pBtn_Restore )
					{
						wstring buttonNameRestore = boost::str(boost::wformat(L"RESTORE_BUTTON_%1%") %pUnit->GetUID());
						if( NULL != pBtn_Restore )
						{
							pBtn_Restore->SetName( buttonNameRestore.c_str() );
						}

						/*wstringstream buttonNameRestore;
						buttonNameRestore.clear();
						buttonNameRestore << L"SLOT_BUTTON_" << pUnit->GetUID();
						if( NULL != pBtn_Restore )
						{
							pBtn_Restore->SetName( buttonNameRestore.str().c_str() );
						}*/
					}
					//}}
				}
				else
				{
					pBtn_Restore->SetShowEnable(false,false);
				}
			}
		}
		
#endif SERV_UNIT_WAIT_DELETE
		//}}

		WCHAR buff[256] = {0,};
		//wsprintf( buff, L"%d", (int)pUnit->GetUnitData()->m_Level );
		StringCchPrintf( buff, 256, L"%d", (int)pUnit->GetUnitData()->m_Level );
		CKTDGUIStatic* pStaticLVNum = (CKTDGUIStatic*)pUnitSlot->GetControl( L"StaticUnitSelectStringLVNum" );
		pStaticLVNum->GetString(0)->msg = buff;





		CKTDGUIStatic* pStaticID = (CKTDGUIStatic*)pUnitSlot->GetControl( L"StaticUnitSelectStringID" );
		CKTDGUIStatic* pStaticChangeNickEmblem = (CKTDGUIStatic*) pUnitSlot->GetControl( L"ChangeNick" );
		CKTDGUIStatic* pStaticChangeNickMessage = (CKTDGUIStatic*) pUnitSlot->GetControl( L"standby" );

		wstring tempNickName = pUnit->GetNickName();
		if ( tempNickName.find( L"__DELETED__") != -1 )
		{
			pStaticID->GetString(0)->msg = L"     ?";

			pStaticChangeNickEmblem->SetShowEnable( true, true );
			pStaticChangeNickMessage->SetShowEnable( true, true );
		}
		else
		{
			pStaticID->GetString(0)->msg = pUnit->GetNickName();

			pStaticChangeNickEmblem->SetShowEnable( false, false );
			pStaticChangeNickMessage->SetShowEnable( false, false );
		}

		
		D3DXVECTOR2 buttonPos = D3DXVECTOR2( dummyPos.x, dummyPos.y );

		/*
		D3DXVECTOR2 buttonPos = D3DXVECTOR2( 0, 0 );
		if ( i == 0 )
		{
		buttonPos = D3DXVECTOR2( 378, 123 );
		}
		else if ( i == 1 )
		{
		buttonPos = D3DXVECTOR2( 586, 123 );
		}
		else if ( i == 2 )
		{
		buttonPos = D3DXVECTOR2( 795, 123 );
		}
		else if ( i == 3 )
		{
		buttonPos = D3DXVECTOR2( 378, 404 );
		}
		else if ( i == 4 )
		{
		buttonPos = D3DXVECTOR2( 586, 404 );
		}
		else if ( i == 5 )
		{
		buttonPos = D3DXVECTOR2( 795, 404 );
		}
		*/
		/*
		m_pDLGUnitSelectFront->OpenScriptFile( L"DLG_Unit_Select_Unit_Info.lua" );
		wstring tempName;
		tempName = pUnit->GetNickName();
		tempName += L"UnitSelectUnitInfo";

		CKTDGUIStatic* pStaticUnitInfo = (CKTDGUIStatic*)m_pDLGUnitSelectFront->GetControl( L"UnitSelectUnitInfo" );
		pStaticUnitInfo->SetName( tempName.c_str() );
		pStaticUnitInfo->GetString(0)->msg = pUnit->GetNickName();
		pStaticUnitInfo->GetString(1)->msg = L"콤보의제왕님";

		WCHAR arLV[8] = {0,};
		//wsprintf(arLV, 7, L"%d", pUnit->GetUnitData()->m_Level );
		StringCchPrintf( buff, 8, L"%d", pUnit->GetUnitData()->m_Level );
		pStaticUnitInfo->GetString(2)->msg = arLV;
		*/
		//		buttonPos.x -= 12;
		//		buttonPos.y -= 13;

		//if( NULL != pButton )
		//	pButton->SetOffsetPos( buttonPos );
		//D3DXVECTOR2 staticPos( buttonPos.x + 5, buttonPos.y );

		if( pUnitSlot != NULL)
			pUnitSlot->SetPos(buttonPos);
		if ( j == UNIT_SELECT_UI_MAX_UNIT_NUM - 1 )
			bAllSlotOccupiedInAPage = true;
		//		pStaticUnitInfo->SetOffsetPos( staticPos );
	}

	dummyPos = m_pDLGUnitSelectFront->GetDummyPos( j );
	D3DXVECTOR2 buttonPos = D3DXVECTOR2( dummyPos.x, dummyPos.y );
	/*
	D3DXVECTOR2 buttonPos = D3DXVECTOR2( 0, 0 );
	if ( i == 0 )
	{
	buttonPos = D3DXVECTOR2( 378, 123 );
	}
	else if ( i == 1 )
	{
	buttonPos = D3DXVECTOR2( 586, 123 );
	}
	else if ( i == 2 )
	{
	buttonPos = D3DXVECTOR2( 795, 123 );
	}
	else if ( i == 3 )
	{
	buttonPos = D3DXVECTOR2( 378, 404 );
	}
	else if ( i == 4 )
	{
	buttonPos = D3DXVECTOR2( 586, 404 );
	}
	else if ( i == 5 )
	{
	buttonPos = D3DXVECTOR2( 795, 404 );
	}
	*/
	if( false == bAllSlotOccupiedInAPage &&
		g_pData->GetMyUser()->GetUnitNum() < m_MaxUnitNum )
	{
		pUnitSlot = new CKTDGUIDialog( this, L"DLG_UI_Character_Selection_Make.lua" );
		g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( pUnitSlot );
		pUnitSlot->SetShowEnable(true, true);
		m_vecUnitSlot.push_back(pUnitSlot);	

		//m_pDLGUnitSelectBack->OpenScriptFile( L"DLG_UI_Character_Selection_Slot.lua" );
		pButton = (CKTDGUIButton*)pUnitSlot->GetControl( L"character2" );
		pButton->SetName( L"EmptySlot" );
		pUnitSlot->SetPos(buttonPos);
	}


	PlayDownAnim();

	

	g_pKTDXApp->SkipFrame();
#endif
}

bool CX2StateServerSelect::ClearUnitButton()
{
	for(UINT i=0; i<m_vecUnitSlot.size(); ++i)
	{
		CKTDGUIDialogType pUnitSlot =  m_vecUnitSlot[i];
		SAFE_DELETE_DIALOG(pUnitSlot);
	}
	m_vecUnitSlot.clear();

	ClearUnitViewerList();

	return true;
}

void CX2StateServerSelect::ClearBeforeUnit( CX2Unit* pUnit )
{
	if( pUnit == NULL )
		return;
	
	m_pSelectUnit = NULL;
}

void CX2StateServerSelect::UnitButtonUp( CX2Unit* pUnit )
{
	if ( m_pSelectUnit == pUnit )
	{
		m_bReserveEnterTutorial = false;
#ifdef SERV_UNIT_WAIT_DELETE
		CX2Unit::UnitData* pUnitData = pUnit->GetUnitData();
		if( pUnitData->m_bDeleted != true)
			Handler_EGS_SELECT_UNIT_REQ( m_pSelectUnit->GetUID(), m_pSelectUnit->GetUnitData()->m_Level );

#else //SERV_UNIT_WAIT_DELETE
		Handler_EGS_SELECT_UNIT_REQ( m_pSelectUnit->GetUID(), m_pSelectUnit->GetUnitData()->m_Level );
#endif //SERV_UNIT_WAIT_DELETE

		return;
	}

	if ( m_pSelectUnit != NULL )
	{
		CX2UnitViewerUI* pUnitViewer = GetUnitViewer( m_pSelectUnit );
		if ( pUnitViewer != NULL )
		{
			pUnitViewer->PlayByMotionType( CX2UnitViewerUI::UVUMT_WAIT );
		}
	}

	ClearBeforeUnit( m_pSelectUnit );



	m_pSelectUnit = pUnit;

/*
	CKTDGUIStatic* pStaticCharBG = (CKTDGUIStatic*)m_pDLGUnitSelectBack->GetControl(L"CharBG" );
	if ( pStaticCharBG != NULL )
	{
		for ( int i = 0; i < CX2Unit::UT_END - 1; i++ )
		{
			if ( pStaticCharBG->GetPicture(i) != NULL )
				pStaticCharBG->GetPicture(i)->SetShow( false );
		}

		// note!! 전직 추가되면 수정, 혹은 unit_type으로
		switch( pUnit->GetClass() )
		{
		case CX2Unit::UC_ELSWORD_SWORDMAN:
		case CX2Unit::UC_ELSWORD_KNIGHT:
		case CX2Unit::UC_ELSWORD_MAGIC_KNIGHT:
			{
				if ( pStaticCharBG->GetPicture(0) != NULL )
					pStaticCharBG->GetPicture(0)->SetShow( true );
			}
			break;

		case CX2Unit::UC_LIRE_ELVEN_RANGER:
		case CX2Unit::UC_LIRE_COMBAT_RANGER:
		case CX2Unit::UC_LIRE_SNIPING_RANGER:
			{
				if ( pStaticCharBG->GetPicture(2) != NULL )
					pStaticCharBG->GetPicture(2)->SetShow( true );
			}
			break;

		case CX2Unit::UC_ARME_VIOLET_MAGE:
		case CX2Unit::UC_ARME_HIGH_MAGICIAN:
		case CX2Unit::UC_ARME_DARK_MAGICIAN:
			{
				if ( pStaticCharBG->GetPicture(1) != NULL )
					pStaticCharBG->GetPicture(1)->SetShow( true );
			}
			break;

		case CX2Unit::UC_RAVEN_FIGHTER:
			{
				if ( pStaticCharBG->GetPicture(3) != NULL )
					pStaticCharBG->GetPicture(3)->SetShow( true );
			}
			break;

		}
	}
*/


	CX2UnitViewerUI* pUnitViewer = GetUnitViewer( pUnit );
	if ( pUnitViewer != NULL  )
	{
		pUnitViewer->PlayByMotionType( CX2UnitViewerUI::UVUMT_READY );
#ifdef SERV_UNIT_WAIT_DELETE //2012.03.07 lygan_조성욱 // 캐릭터별 삭제 대기시에 애니메이션 설정
		if(m_pBeforeSelectUnit != NULL)
		{
			CX2Unit::UnitData* pUnitData = m_pBeforeSelectUnit->GetUnitData();
			if (pUnitData != NULL )
				pUnitData->m_bDeletedMotionPlay = false;
		}
		m_pBeforeSelectUnit = pUnit;

#endif //SERV_UNIT_WAIT_DELETE
	}
	/*
	D3DXVECTOR3 unitSize = D3DXVECTOR3( 1.6f, 1.6f, 1.6f );


	//	CKTDGUIStatic* pStatic = (CKTDGUIStatic*)m_pDLGUnitSelectBack->GetControl( L"StaticUnit_Select_UnitInfo" );
	//	pStatic->SetOffsetPos( D3DXVECTOR2( -200, 200 ) );
	//	pStatic->Move( D3DXVECTOR2( 0, 0 ), D3DXCOLOR(1,1,1,1), 0.1f, true );

	switch( pUnit->GetClass() )
	{
	case CX2Unit::UC_ELSWORD_SWORDMAN:
	//m_pDLGUnitSelectBack->OpenScriptFile( L"DLG_Unit_Select_ElSword_BG.lua" );

	//pStatic->GetPicture(0)->SetShow( true );

	m_pUnitViewerUI = CX2UnitViewerUI::CreateUnitViewerUI();
	//m_pUnitViewerUI->SetLightPos( 1000, 1000, -200 );
	pUnitViewerUI->SetLightPos( 300, 300, -500 );	// 캐릭터뷰어 라이트 위치 변경
	g_pKTDXApp->GetDGManager()->AddObjectChain( m_pUnitViewerUI );

	m_pUnitViewerUI->SetUnit( pUnit );
	m_pUnitViewerUI->GetMatrix().Move(  D3DXVECTOR3( -250, -70, 200 ) );
	m_pUnitViewerUI->GetMatrix().Scale( unitSize );
	m_pUnitViewerUI->GetMatrix().Rotate(D3DXToRadian(0),D3DXToRadian(0),D3DXToRadian(0));
	m_pUnitViewerUI->PlayAnim( L"LobbyFullDown", CKTDGXSkinAnim::XAP_ONE_WAIT );

	m_pUnitViewerUI->OnFrameMove( g_pKTDXApp->GetTime(),g_pKTDXApp->GetElapsedTime() );

	break;

	case CX2Unit::UC_LIRE_ELVEN_RANGER:
	//m_pDLGUnitSelectBack->OpenScriptFile( L"DLG_Unit_Select_Lire_BG.lua" );
	//pStatic->GetPicture(2)->SetShow( true );

	m_pUnitViewerUI = CX2UnitViewerUI::CreateUnitViewerUI();
	//m_pUnitViewerUI->SetLightPos( 1000, 1000, -200 );
	pUnitViewerUI->SetLightPos( 300, 300, -500 );	// 캐릭터뷰어 라이트 위치 변경
	g_pKTDXApp->GetDGManager()->AddObjectChain( m_pUnitViewerUI );

	m_pUnitViewerUI->SetUnit( pUnit );
	m_pUnitViewerUI->GetMatrix().Move( D3DXVECTOR3( -250, -70, 200 ) );
	m_pUnitViewerUI->GetMatrix().Scale( unitSize );
	m_pUnitViewerUI->GetMatrix().Rotate(D3DXToRadian(0),D3DXToRadian(0),D3DXToRadian(0));
	m_pUnitViewerUI->PlayAnim( L"LobbyFullDown", CKTDGXSkinAnim::XAP_ONE_WAIT );
	m_pUnitViewerUI->GetXSkinAnim()->SetApplyMotionOffset( false );

	m_pUnitViewerUI->OnFrameMove( g_pKTDXApp->GetTime(),g_pKTDXApp->GetElapsedTime() );
	break;

	case CX2Unit::UC_ARME_VIOLET_MAGE:
	//m_pDLGUnitSelectBack->OpenScriptFile( L"DLG_Unit_Select_Arme_BG.lua" );
	//pStatic->GetPicture(1)->SetShow( true );
	m_pUnitViewerUI = CX2UnitViewerUI::CreateUnitViewerUI();
	//m_pUnitViewerUI->SetLightPos( 1000, 1000, -200 );
	pUnitViewerUI->SetLightPos( 300, 300, -500 );	// 캐릭터뷰어 라이트 위치 변경
	g_pKTDXApp->GetDGManager()->AddObjectChain( m_pUnitViewerUI );

	m_pUnitViewerUI->SetUnit( pUnit );
	m_pUnitViewerUI->GetMatrix().Move( D3DXVECTOR3( -250, -70, 200 ) );
	m_pUnitViewerUI->GetMatrix().Scale( unitSize );
	m_pUnitViewerUI->GetMatrix().Rotate(D3DXToRadian(0),D3DXToRadian(0),D3DXToRadian(0));
	m_pUnitViewerUI->PlayAnim( L"LobbyFullDown", CKTDGXSkinAnim::XAP_ONE_WAIT );

	m_pUnitViewerUI->GetXSkinAnim()->SetApplyMotionOffset( false );

	m_pUnitViewerUI->OnFrameMove( g_pKTDXApp->GetTime(),g_pKTDXApp->GetElapsedTime() );
	break;

	}




	g_pKTDXApp->SkipFrame();
	*/
}


bool CX2StateServerSelect::Handler_EGS_SELECT_UNIT_REQ( UidType unitUID, int iUnitLevel )
{
#ifdef SERV_SECOND_SECURITY
	if(m_bEnableUnitSelectBySecondSecurity == false)
	{
		return true;
	}
#endif SERV_SECOND_SECURITY

#ifdef SERV_KOM_FILE_CHECK_ADVANCED
#ifdef SERV_VALIDITY_CHECK_CEHCKKOM_SCRIPT
	if( false == g_pMain->GetIsValideCheckKomScript() )
	{
		g_pMain->SetQuitType( NetError::ERR_CLIENT_QUIT_03 );
		g_pMain->SendQuitMsgToServer();
	}
#endif SERV_VALIDITY_CHECK_CEHCKKOM_SCRIPT
#endif SERV_KOM_FILE_CHECK_ADVANCED

	// 체험 아이디 제한 
	if( true == g_pData->GetMyUser()->GetUserData()->m_bIsGuestUser &&
		iUnitLevel >= 20 )
	{
		g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(270,350), GET_STRING( STR_ID_40 ), g_pMain->GetNowState() );
		return true;
	}

	KEGS_SELECT_UNIT_REQ	kPacket;
	kPacket.m_iUnitUID		= unitUID;

	g_pData->GetServerProtocol()->SendPacket( EGS_SELECT_UNIT_REQ, kPacket );
	g_pMain->AddServerPacket( EGS_SELECT_UNIT_ACK, 30.0f );

#ifdef LIST_SORT_AT_CHARACTER_SELECT
	g_pData->GetMyUser()->SortFirstByUID(unitUID); 
#endif LIST_SORT_AT_CHARACTER_SELECT
	SAFE_DELETE_DIALOG( m_pDLGMsgBox );
	m_pDLGMsgBox = g_pMain->KTDGUIMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_709 ), this );

#ifdef NEW_EMBLEM_MANAGER
	CX2EmblemManager::GetInstance()->InitEmbelmPlayInfo(true);
#endif // NEW_EMBLEM_MANAGER
	return true;
}

#ifdef SERV_SELECT_UNIT_PACKET_DIVISION		/// 캐릭터 선택 패킷 분할

/// 유닛의 정보, 인벤토리, 스킬
bool CX2StateServerSelect::Handler_EGS_SELECT_UNIT_1_NOT( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_SELECT_UNIT_1_NOT kEvent;
	DeSerialize( pBuff, &kEvent );


	if( NULL != g_pMain && g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
	{
		CX2BattleFieldManager& battleFieldManager = g_pData->GetBattleFieldManager();
		battleFieldManager.SetBattleFieldIdWhereIam( 0 );

		if( NULL != g_pData && NULL != g_pData->GetMyUser() )
		{
			CX2Unit* pUnit = g_pData->GetMyUser()->GetUnitByUID( kEvent.m_kUnitInfo.m_nUnitUID );

			if( NULL != pUnit )
			{
				g_pData->GetMyUser()->SelectUnit( pUnit );
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
				g_pData->GetItemStatCalculator().SetUnitData( pUnit->GetType(), pUnit->GetClass() );
#endif //SERV_NEW_ITEM_SYSTEM_2013_05
				//{{ 오현빈 // 2012-04-06 // 이벤트 받아오기 전 타이머 초기화 시킬 수 있도록 수정
#ifdef FIX_EVENT_TIMER	
				pUnit->m_mapEventData.clear();
				pUnit->SetReceivedGuestUserReward( true );		
#endif //FIX_EVENT_TIMER
				//}} 오현빈 // 2012-04-06 // 이벤트 받아오기 전 타이머 초기화 시킬 수 있도록 수정

				pUnit->Reset( kEvent.m_kUnitInfo, true );

				//{{ kimhc // 2010-01-04 // 기존의 EGS_SELECT_UNIT_ACK에 인벤정보가 포함되어 오던 것 분할
#ifdef	PC_BANG_WORK
				pUnit->ResetInventorySize( kEvent.m_mapInventorySlotSize );
				pUnit->ResetInventoryItems( kEvent.m_mapItem ); // 장착중인 장비만 먼저 Reset
#else	PC_BANG_WORK
				//kimhc // 2008-08-04 // m_mapInventorySlotSize에서 키값이 ST_BANK인 
				// value는 은행슬롯의 사이즈 이고 이 사이즈로 회원 등급을 구분한다.
				pUnit->ResetInventory( kEvent.m_mapInventorySlotSize, kEvent.m_mapItem );
#endif	PC_BANG_WORK
				//}} kimhc // 2010-01-04 // 기존의 EGS_SELECT_UNIT_ACK에 인벤정보가 포함되어 오던 것 분할

				pUnit->SetResurrectionStoneNum( kEvent.m_iNumResurrectionStone );

				//{{ 허상형 : [2009/10/14] //	자동 결제 부활석
#ifdef AUTO_PAYMENT
				pUnit->SetAutoResStoneNum( kEvent.m_iNumAutoPaymentResStone );
#endif	//	AUTO_PAYMENT
				//}} 허상형 : [2009/10/14] //	자동 결제 부활석


#ifdef ADDED_RELATIONSHIP_SYSTEM
				// 이 구문은 스킬 보다 무조건 위에 있어야 합니다!!
				if ( NULL != g_pData->GetMessenger() )
				{
					g_pData->GetMessenger()->ResetRelationDummyUnit ();
				}

				CX2RelationshipManager * pRelationshipManager = g_pData->ResetRelationshipManager();
				CX2RelationshipEffectManager * pRelationshipEffectManager = g_pData->ResetRelationshipEffectManager();

				if ( NULL != pRelationshipManager )
				{
					CX2RelationshipManager::RelationshipInfo * pMyRelationshipInfo = pRelationshipManager->GetMyRelationshipInfo();
					if ( NULL != pMyRelationshipInfo )
					{
						pMyRelationshipInfo->ClearRelationInfo();
						pMyRelationshipInfo->m_eRelationStateType				= (SEnum::RELATIONSHIP_TYPE) kEvent.m_kRelationshipInfo.m_cRelationshipType;
						pMyRelationshipInfo->m_iRelationTargetUserUid			= kEvent.m_kRelationshipInfo.m_iOtherUnitUID;

						CTime ctCurrentTime( g_pData->GetServerCurrentTime() );		
						CTime ctCoupleStartTime = static_cast<CTime> ( kEvent.m_kRelationshipInfo.m_tDate );
						CTimeSpan ctsGapTime = ctCurrentTime - ctCoupleStartTime;


						pMyRelationshipInfo->m_ctsDurationDay					= ctsGapTime;
						pMyRelationshipInfo->m_wstrNotifyLoveMessage			= kEvent.m_kRelationshipInfo.m_wstrLoveWord;
						pMyRelationshipInfo->m_wstrRelationTargetUserNickname	= kEvent.m_kRelationshipInfo.m_wstrOtherNickName;
					}
				}

				// 임시 코드!!
				pRelationshipEffectManager->SetMyRelationEffect ( 1 );
				

#endif // ADDED_RELATIONSHIP_SYSTEM

			
				// skill list
				if( NULL != pUnit->GetUnitData() )
				{
					//pUnit->GetUnitData()->m_UserSkillTree.SetUnitClass( (int) kEvent.m_kUnitInfo.m_cUnitClass );
					pUnit->GetUnitData()->m_UserSkillTree.SetAcquiredSkill( kEvent.m_vecSkillAcquired );
					pUnit->GetUnitData()->m_UserSkillTree.SetUnsealedSkill( kEvent.m_vecSkillUnsealed );
					pUnit->GetUnitData()->m_UserSkillTree.SetEquippedSkill( kEvent.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkill, kEvent.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkillSlotB );
					// 		pUnit->GetUnitData()->m_UserSkillTree.SetSkillSlotBExpirationState( (CX2UserSkillTree::SKILL_SLOT_B_EXPIRATION_STATE) kEvent.m_kUnitInfo.m_UnitSkillData.m_cSkillSlotBExpirationState );
					// 		pUnit->GetUnitData()->m_UserSkillTree.SetSkillSlotBEndDateString( kEvent.m_kUnitInfo.m_UnitSkillData.m_wstrSkillSlotBEndDate );


#ifdef ADDED_RELATIONSHIP_SYSTEM
					if ( NULL != g_pData->GetRelationshipManager() &&
					 	 NULL != g_pData->GetRelationshipManager()->GetMyRelationshipInfo() )
					{
						int iSkillLevel = static_cast<int>(g_pData->GetRelationshipManager()->GetMyRelationshipInfo()->m_eRelationStateType);
						pUnit->GetUnitData()->m_UserSkillTree.SetSkillLevelAndCSP( CX2SkillTree::SI_ETC_WS_COMMON_LOVE, iSkillLevel, 0 );
					}
#endif // ADDED_RELATIONSHIP_SYSTEM
#ifdef SERV_SKILL_NOTE		
					pUnit->SetSkillNote( kEvent.m_mapSkillNote, kEvent.m_cSkillNoteMaxPageNum );
					pUnit->GetUnitData()->m_UserSkillTree.SetEqipSkillMemo( kEvent.m_kUnitInfo.m_UnitSkillData.m_vecSkillNote );
#endif

#ifdef TITLE_SYSTEM
#ifdef SERV_TITLE_DATA_SIZE
					pUnit->GetUnitData()->m_iTitleId = kEvent.m_kUnitInfo.m_iTitleID;
#else
					pUnit->GetUnitData()->m_iTitleId = kEvent.m_kUnitInfo.m_sTitleID;
#endif
#endif TITLE_SYSTEM
				}

				if( NULL != g_pData->GetUIManager() && NULL != g_pData->GetUIManager()->GetUISkillTree() )
				{
					g_pData->GetUIManager()->CreateUISkillTree();
					g_pData->GetUIManager()->GetUISkillTree()->InitSkillTreeUI();
				}

				#ifdef GUILD_SKILL
				//{{  [11/11/2009 : oasis907 ]

				// 임시: 서버에게서 GuildUserSkillTree 정보를 받았다 가정
				/*
				// skill list
				pUnit->GetUnitData()->m_UserSkillTree.SetGuildClass(0);
				pUnit->GetUnitData()->m_UserSkillTree.SetAcquiredGuildSkill( kEvent.m_vecSkillAcquired ); // 함수 임시 변경
				*/		
		
				//pUnit->GetUnitData()->m_GuildUserSkillTree.SetUnsealedSkill( kEvent.m_vecSkillUnsealed );


				// 함수 임시 변경
				//pUnit->GetUnitData()->m_UserSkillTree.SetEquippedSkill( kEvent.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkill, kEvent.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkillSlotB );

				//pUnit->GetUnitData()->m_GuildUserSkillTree.SetSkillSlotBExpirationState( (CX2GuildUserSkillTree::SKILL_SLOT_B_EXPIRATION_STATE) kEvent.m_kUnitInfo.m_UnitSkillData.m_cSkillSlotBExpirationState );
				//pUnit->GetUnitData()->m_GuildUserSkillTree.SetSkillSlotBEndDateString( kEvent.m_kUnitInfo.m_UnitSkillData.m_wstrSkillSlotBEndDate );

				/*
				if( g_pData->GetGuildManager()->GetUIGuild()->GetUIGuildSkillTree() != NULL)
				{
					g_pData->GetGuildManager()->GetUIGuild()->CreateUIGuildSkillTree();
					g_pData->GetGuildManager()->GetUIGuild()->GetUIGuildSkillTree()->InitSkillTreeUI();
				}
				//}}  [11/11/2009 : oasis907 ]
				*/
#endif GUILD_SKILL

#ifdef SERV_TITLE_DATA_SIZE
				pUnit->GetUnitData()->m_iTitleId = kEvent.m_kUnitInfo.m_iTitleID;
#else
				pUnit->GetUnitData()->m_iTitleId = kEvent.m_kUnitInfo.m_sTitleID;
#endif

				//{{ kimhc // 2010-01-04 // 기존의 EGS_SELECT_UNIT_ACK에 인벤정보가 포함되어 오던 것 분할
#ifdef	PC_BANG_WORK
				pUnit->ResetInventorySize( kEvent.m_mapInventorySlotSize );
				pUnit->ResetInventoryItems( kEvent.m_mapItem ); // 장착중인 장비만 먼저 Reset
#else	PC_BANG_WORK
				//kimhc // 2008-08-04 // m_mapInventorySlotSize에서 키값이 ST_BANK인 
				// value는 은행슬롯의 사이즈 이고 이 사이즈로 회원 등급을 구분한다.
				pUnit->ResetInventory( kEvent.m_mapInventorySlotSize, kEvent.m_mapItem );
#endif	PC_BANG_WORK
				//}} kimhc // 2010-01-04 // 기존의 EGS_SELECT_UNIT_ACK에 인벤정보가 포함되어 오던 것 분할

				pUnit->SetResurrectionStoneNum( kEvent.m_iNumResurrectionStone );

				//{{ 허상형 : [2009/10/14] //	자동 결제 부활석
#ifdef AUTO_PAYMENT
				pUnit->SetAutoResStoneNum( kEvent.m_iNumAutoPaymentResStone );
#endif	//	AUTO_PAYMENT
				//}} 허상형 : [2009/10/14] //	자동 결제 부활석

#ifdef SERV_NEW_UNIT_TRADE_LIMIT
				pUnit->SetNewUnitTradeBlockDay( kEvent.m_iNewUnitTradeBlockDay );
				pUnit->SetNewUnitTradeBlockUnitClass( kEvent.m_iNewUnitTradeBlockUnitClass );
				pUnit->SetUnitCreateDate( kEvent.m_wstrUnitCreateDate );

				if( true == kEvent.m_bIsNewUnitTradeBlock )
				{
					CTime tReleaseTradeBlockTime;
					KncUtil::ConvertStringToCTime( kEvent.m_wstrUnitCreateDate, tReleaseTradeBlockTime );
					tReleaseTradeBlockTime += CTimeSpan( kEvent.m_iNewUnitTradeBlockDay, 0, 0, 0 );
					CTime tCurTime = g_pData->GetServerCurrentTime();            

					if( kEvent.m_iNewUnitTradeBlockDay != 0 && tCurTime < tReleaseTradeBlockTime )
					{
						wstring wstrMonth = static_cast< const wchar_t * >( tReleaseTradeBlockTime.Format( _T("%m") ) );
						wstring wstrDay = static_cast< const wchar_t * >( tReleaseTradeBlockTime.Format( _T("%d") ) );
						wstring wstrHour = static_cast< const wchar_t * >( tReleaseTradeBlockTime.Format( _T("%H") ) );
						wstring wstrMinute = static_cast< const wchar_t * >( tReleaseTradeBlockTime.Format( _T("%M") ) );
						wstring wstrNotice = GET_REPLACED_STRING( ( STR_ID_14586 , "LLLL", wstrMonth, wstrDay , wstrHour , wstrMinute ) );

						pUnit->SetTradeBlockUnit( true );
						pUnit->SetTradeBlockChatMsg( wstrNotice );
					}
					else if( pUnit->GetNewUnitTradeBlockUnitClass() != 0 
						&& pUnit->GetClass() < pUnit->GetNewUnitTradeBlockUnitClass() )
					{
						// 전직 후에 거래가능합니다.
						wstring wstrNotice =  GET_STRING( STR_ID_24523 );

						pUnit->SetTradeBlockUnit( true );
						pUnit->SetTradeBlockChatMsg( wstrNotice );
					}
				}
#endif SERV_NEW_UNIT_TRADE_LIMIT
			}
		}

		if( g_pData->GetUIManager() != NULL)
		{
			g_pData->GetUIManager()->CreateUISkillTree();
			g_pData->GetUIManager()->GetUISkillTree()->InitSkillTreeUI();
		}

		
		//{{ kimhc // 2010-01-04 // 기존의 EGS_SELECT_UNIT_ACK에 인벤정보가 포함되어 오던 것 분할
#ifdef	PC_BANG_WORK
		g_pData->GetServerProtocol()->SendID( EGS_GET_MY_INVENTORY_REQ );
		g_pMain->AddServerPacket( EGS_GET_MY_INVENTORY_ACK  );
#else	PC_BANG_WORK
		// 체험 아이디 제한 
		if( false == g_pData->GetMyUser()->GetUserData()->m_bIsGuestUser &&
			kEvent.m_bIsRecommend == false && m_bCheckRecommend == true )
		{
			OpenRecommendMsgBox( true );
		}
		else
		{

			g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_PartyTalkBoxInfo.m_iPartyUID = 0;

			if(g_pMain->GetIsPlayingTutorial() == true)
			{
				// 튜토리얼 진입요청
				Handler_EGS_CREATE_TUTORIAL_ROOM_REQ();
			}
			else
			{
				// 새마을로 진입요청
				Handler_EGS_STATE_CHANGE_FIELD_REQ();
			}
		}
#endif	PC_BANG_WORK
		//}} kimhc // 2010-01-04 // 기존의 EGS_SELECT_UNIT_ACK에 인벤정보가 포함되어 오던 것 분할

	#ifdef UPGRADE_SKILL_SYSTEM_2013 // 김태환 - 스킬 시스템 변경
		if ( NULL != g_pData &&
			 NULL != g_pData->GetMyUser() &&
			 NULL != g_pData->GetMyUser()->GetSelectUnit() )
		{
			/// 스킬 추가 레벨 효과 갱신
			g_pData->GetMyUser()->GetSelectUnit()->ResetIncreaseSkillLevelBySocket();
		}
	#endif // UPGRADE_SKILL_SYSTEM_2013

		return true;
		}

	return false;
}

/// 퀘스트
bool CX2StateServerSelect::Handler_EGS_SELECT_UNIT_2_NOT( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_SELECT_UNIT_2_NOT kEvent;
	DeSerialize( pBuff, &kEvent );

	if( NULL != g_pMain && g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
	{
		//{{ kimhc // 2010.4.12 // 비밀던전 작업(일일퀘스트 시스템)
#ifdef SERV_SECRET_HELL
		if( NULL != g_pInstanceData )
			g_pInstanceData->SetPreCheckTimeForDailyQuest( g_pData->GetServerCurrentTime() );
#endif SERV_SECRET_HELL
		//}} kimhc // 2010.4.12 // 비밀던전 작업(일일퀘스트 시스템)

		if( NULL != g_pData && NULL != g_pData->GetQuestManager() )
			g_pData->GetQuestManager()->SetUnitQuest( kEvent.m_vecQuest, kEvent.m_vecCompletQuest );

		//{{ kimhc // GiveUpForbiddenQuest()를 EGS_GET_MY_INVENTORY_ACK로 이동함
#ifndef	SERV_EPIC_QUEST
		g_pData->GetQuestManager()->GiveUpForbiddenQuest();
#endif	SERV_EPIC_QUEST
		//}} kimhc // GiveUpForbiddenQuest()를 EGS_GET_MY_INVENTORY_ACK로 이동함

#ifdef	SERV_EPIC_QUEST
		// oasis907 : 김상윤 [2010.7.21] // 퀵퀘스트 창 열기 초기화
		if(	g_pData != NULL && g_pData->GetUIManager() != NULL && g_pData->GetUIManager()->GetUIQuestNew() != NULL)
			g_pData->GetUIManager()->GetUIQuestNew()->InitUserShowQuickQuestDLG(true);
#endif	SERV_EPIC_QUEST

		if( NULL != g_pData && NULL != g_pData->GetMyUser() )
		{
			CX2Unit* pUnit = g_pData->GetMyUser()->GetSelectUnit();
			if( NULL != pUnit && NULL != g_pMain && NULL != g_pMain->GetInformerManager() && NULL != g_pMain->GetLVUpEventMgr() )
			{
				g_pMain->GetInformerManager()->Reset();
				g_pMain->GetLVUpEventMgr()->Reset( pUnit->GetUnitData()->m_Level );
			}

			if ( g_pData->GetCashShop() != NULL )
				g_pData->GetCashShop()->ResetUnitViewer( pUnit );

			g_pMain->ResetCrashReport( L"" );
		}

#ifdef SERV_RANDOM_DAY_QUEST
		if( NULL != g_pData && NULL != g_pData->GetQuestManager() )
			g_pData->GetQuestManager()->SetRandomQuestInfo( kEvent.m_mapRandomQuestList );
#endif SERV_RANDOM_DAY_QUEST

#ifdef REFORM_QUEST
		if( NULL != g_pData && NULL != g_pData->GetUIManager() && NULL != g_pData->GetUIManager()->GetUIQuestNew() )
		{
			g_pData->GetUIManager()->GetUIQuestNew()->SetOpenEpicCategory();
			g_pData->GetUIManager()->GetUIQuestNew()->InitNewQuestList();
		}
#endif //REFORM_QUEST

		return true;
	}

	return false;
}

/// 타이틀, 펫
bool CX2StateServerSelect::Handler_EGS_SELECT_UNIT_3_NOT( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_SELECT_UNIT_3_NOT kEvent;
	DeSerialize( pBuff, &kEvent );

	if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
	{

		if( NULL != g_pData && NULL != g_pData->GetMyUser() )
		{
			CX2Unit* pUnit = g_pData->GetMyUser()->GetSelectUnit();

			if( NULL != pUnit )
			{
#ifdef TITLE_SYSTEM
				if(g_pData != NULL && g_pData->GetTitleManager() != NULL)
				{
					g_pData->GetTitleManager()->InitRecord();
					g_pData->GetTitleManager()->AttachTitle(pUnit->GetUnitData()->m_iTitleId);

					g_pData->GetTitleManager()->TakeTitle(kEvent.m_vecTitle);
					g_pData->GetTitleManager()->UpdateMission(kEvent.m_vecMission);
				}                
#endif

#ifdef SERV_PET_SYSTEM

				pUnit->SetSummonPetUid( kEvent.m_iSummonedPetUID );
				//if( g_pData != NULL && g_pData->GetPetManager() != NULL )
				//	g_pData->GetPetManager()->Handler_EGS_SUMMON_PET_REQ( kEvent.m_iSummonedPetUID );
				if( kEvent.m_vecPetList.size() > 0 )
					pUnit->SetFullPetInfo(kEvent.m_vecPetList[0]);

				if( g_pData->GetPetManager() != NULL )
				{
					g_pData->GetPetManager()->AddMyPetList( kEvent.m_vecCanEvolutionPetList );
					g_pData->GetPetManager()->AddNeverSummonPetList( kEvent.m_vecNeverSummonPetList, kEvent.m_iSummonedPetUID );

					// oasis907 : 김상윤 [2010.9.7] // 
					if( kEvent.m_vecPetList.size() > 0 )
					{
						KPetInfo kPetInfo = kEvent.m_vecPetList[0];
						int EvolutionStep = kPetInfo.m_cEvolutionStep;
#ifdef SERV_PETID_DATA_TYPE_CHANGE //2013.07.02
						int iInvenSlotSize = g_pData->GetPetManager()->GetPetInventorySlotSize((CX2PetManager::PET_UNIT_ID) kPetInfo.m_iPetID, EvolutionStep);
#else //SERV_PETID_DATA_TYPE_CHANGE
						int iInvenSlotSize = g_pData->GetPetManager()->GetPetInventorySlotSize((CX2PetManager::PET_UNIT_ID) kPetInfo.m_cPetID, EvolutionStep);
#endif //SERV_PETID_DATA_TYPE_CHANGE

						if( NULL != pUnit->GetInventory() )
							pUnit->GetInventory()->SetItemMaxNum(CX2Inventory::ST_PET, iInvenSlotSize);

						// oasis907 : 김상윤 [2010.9.8] // 자동 먹이 주기 기능
						if( g_pData->GetUIManager()->GetUIInventory() != NULL)
						{
							g_pData->GetUIManager()->GetUIInventory()->SetAutoFeed(kPetInfo.m_bAutoFeed);
						}
						//}}
					}
					//}}
#ifdef SERV_PERIOD_PET
					else
					{
						g_pData->GetMyUser()->GetSelectUnit()->ClearPetInfo();
					}
#endif SERV_PERIOD_PET
				}

#endif SERV_PET_SYSTEM
			}
		}

		return true;
	}

	return false;
}

/// 접속 이벤트, 차단, 사제, 랭킹
bool CX2StateServerSelect::Handler_EGS_SELECT_UNIT_4_NOT( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_SELECT_UNIT_4_NOT kEvent;
	DeSerialize( pBuff, &kEvent );

	if( NULL != g_pMain && g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
	{
		/// 튜토리얼
		if( NULL != g_pMain->GetTutorSystem() )
		{
			g_pMain->GetTutorSystem()->ResetTutorStudent( kEvent.m_vecTutorialDBUnitInfo );
		}

		if ( m_bReserveEnterTutorial == true )
		{
			g_pMain->SetIsPlayingTutorial( true );
		}

		if( NULL != g_pData && NULL != g_pData->GetMyUser() )
		{
			CX2Unit* pUnit = g_pData->GetMyUser()->GetSelectUnit();

			if( NULL != pUnit )
			{
				/// 차단 리스트				
				pUnit->SetBlackList( kEvent.m_vecChatBlackList );

				CX2GageManager* pGageManager = CX2GageManager::GetInstance();

				if( NULL != pGageManager && NULL != pUnit->GetUnitData() )
				{
					pGageManager->CreateMyGageUI( pUnit->GetClass(), pUnit->GetUID() );
					pGageManager->InitMySkillCoolTimeList( pUnit->GetUnitData()->m_UserSkillTree );
					//pGageManager->InitMyQuickSlotCoolTimeList( pUnit->GetInventory()->GetItemMaxNum( CX2Inventory::ST_E_QUICK_SLOT ) );


					ASSERT( !kEvent.m_kGamePlayStatus.m_vecGamePlayStatus.empty() );
					if ( !kEvent.m_kGamePlayStatus.m_vecGamePlayStatus.empty() )
					{
						pGageManager->SetMyPlayStatusFromPacket( kEvent.m_kGamePlayStatus.m_vecGamePlayStatus[0] );
						pGageManager->UpdateNowHpPercent();
						pGageManager->UpdateNowMpPercent();
					}

					pUnit->GetUnitData()->m_UserSkillTree.InitSkillCoolTimeFromGageManager( pGageManager->GetSkillCoolTimeList() );

					if( NULL != g_pData->GetUIManager() && NULL != g_pData->GetUIManager()->GetUIQuickSlot() )
						g_pData->GetUIManager()->GetUIQuickSlot()->InitMyQuickSlotCoolTimeFromGageManager( pGageManager->GetQuickSlotCoolTimeList() );
					//		const float fResultMaxMp = pUnit->ResetMaxMp( ONE_CHARGE * 3.0f );
					//		pGageManager->SetMaxMp( pUnit->GetUID(), fResultMaxMp );
					// GameStat으로 변경 해야 함, 그리고 한번에 넣을 수 있는 법도...
					// 		pGageManager->SetMaxHp( pUnit->GetUID(), static_cast<const float>( kEvent.m_kUnitInfo.m_kStat.m_iBaseHP ) );
					// 		pGageManager->SetNowHp( pUnit->GetUID(), static_cast<const float>( kEvent.m_kUnitInfo.m_kStat.m_iBaseHP ) );	
					// 		pGageManager->SetMaxSoul( pUnit->GetUID(), 100.0f );

					// 휴면유저인지 확인후 팝업을 띄운다. -- 현재 쓰이지 않는 이벤트
					//switch(kEvent.m_iSelectUnitEvent)
					//{
					//case 0:	// 무시
					//	break;
					//case 1:	// 컴백유저
					//	g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_2634 ), g_pMain->GetNowState() );
					//	break;
					//case 2:	// 컴백인데 재접속한 유저
					//	g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_2635 ), g_pMain->GetNowState() );
					//	break;
					//case 3:	// 기존유저 보상
					//	g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_2636 ), g_pMain->GetNowState() );
					//	break;
					//case 4: // 신규 재접속
					//	g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_2640 ), g_pMain->GetNowState() );
					//	break;
					//default:
					//	break;
					//}
				}
			}
		}

		if( NULL != g_pMain->GetGameOption() )
		{
			g_pMain->GetGameOption()->SetDenyInviteGuild( (CX2GameOption::BlackListDenyState) kEvent.m_kDenyOptions.m_cDenyInviteGuild );
			g_pMain->GetGameOption()->SetDenyPersonalTrade( (CX2GameOption::BlackListDenyState) kEvent.m_kDenyOptions.m_cDenyPersonalTrade );
			g_pMain->GetGameOption()->SetDenyFriendship( (CX2GameOption::BlackListDenyState) kEvent.m_kDenyOptions.m_cDenyFriendShip );
			g_pMain->GetGameOption()->SetDenyParty( (CX2GameOption::BlackListDenyState) kEvent.m_kDenyOptions.m_cDenyParty );

			g_pMain->GetGameOption()->SetRefuseParty( (CX2GameOption::BlackListDenyState) kEvent.m_kDenyOptions.m_cDenyParty );
			g_pMain->GetGameOption()->SetRefuseFriend( (CX2GameOption::BlackListDenyState) kEvent.m_kDenyOptions.m_cDenyFriendShip );
			g_pMain->GetGameOption()->SetRefusePersonalTrade( (CX2GameOption::BlackListDenyState) kEvent.m_kDenyOptions.m_cDenyPersonalTrade );
			//{{ kimhc // 2009-10-12 // 길드 초대 거부 추가
#ifdef	GUILD_MANAGEMENT
			g_pMain->GetGameOption()->SetRefuseGuildInvitation( static_cast< CX2GameOption::BlackListDenyState >( kEvent.m_kDenyOptions.m_cDenyInviteGuild ) );
#endif	GUILD_MANAGEMENT
			//}} kimhc // 2009-10-12 // 길드 초대 거부 추가

#ifdef ADDED_RELATIONSHIP_SYSTEM	/// 커플신청 거부 추가
			g_pMain->GetGameOption()->SetRefuseRequestCouple( static_cast< CX2GameOption::BlackListDenyState >( kEvent.m_kDenyOptions.m_cDenyRequestCouple ) );
#endif // ADDED_RELATIONSHIP_SYSTEM
		}

		// 친구채팅창 히스토리 초기화 및 친구리스트 초기 설정
		if(g_pData != NULL && g_pData->GetMessenger() != NULL)
		{
			g_pData->GetMessenger()->ClearChatSessionList();
			g_pData->GetMessenger()->Reset();		
			g_pData->GetMessenger()->ResetHistory();
#ifdef NEW_MESSENGER
			g_pData->GetMessenger()->SetFriendList( kEvent.m_kMessengerInfo );
#else		
			g_pData->GetMessenger()->SyncFriendList();
#endif NEW_MESSENGER
		}

#ifdef ADDED_RELATIONSHIP_SYSTEM
		if ( NULL != g_pData->GetRelationshipManager() &&
			NULL != g_pData->GetRelationshipManager()->GetUIRelationship() )
		{
			g_pData->GetRelationshipManager()->GetUIRelationship()->GetFriendListForInvitingWeddingUser();
		}
#endif // ADDED_RELATIONSHIP_SYSTEM

#ifdef SERV_RECRUIT_EVENT_QUEST_FOR_NEW_USER
		g_pInstanceData->SetRecruit( !kEvent.m_vecRecruiterUnitInfo.empty() );
#endif SERV_RECRUIT_EVENT_QUEST_FOR_NEW_USER

		return true;
	}

	return false;
}

/// ETC, 1회성 이벤트
bool CX2StateServerSelect::Handler_EGS_SELECT_UNIT_5_NOT( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_SELECT_UNIT_5_NOT kEvent;
	DeSerialize( pBuff, &kEvent );

	if( NULL != g_pMain && g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
	{
#ifdef SERV_PSHOP_AGENCY
		if( g_pInstanceData != NULL )
		{
			g_pInstanceData->SetPShopAgencyInfo( kEvent.m_kPShopAgencyInfo.m_bIsPShopOpen, kEvent.m_kPShopAgencyInfo.m_wstrAgencyExpirationDate );
		}
#endif

#ifdef NEW_HENIR_TEST
		if( NULL != g_pData && NULL != g_pData->GetMyUser() && NULL != g_pData->GetMyUser()->GetSelectUnit() )
			g_pData->GetMyUser()->GetSelectUnit()->SetHenirRewardCountInfo( kEvent.m_PacketHenirRewardCount );
#endif NEW_HENIR_TEST

#ifdef SERV_EVENT_MONEY
		if( NULL != g_pData && NULL != g_pData->GetMyUser() )
		{
			g_pData->GetMyUser()->SetUserAngelsFeather(kEvent.m_iEventMoney);
		}
		if( NULL != g_pInstanceData && NULL != g_pInstanceData->GetMiniMapUI() )
		{
			g_pInstanceData->GetMiniMapUI()->UpdateAngelsFeatherCount(kEvent.m_iEventMoney);
		}
#endif //SERV_EVENT_MONEY

		//{{ kimhc // 2009-12-04 // 크리스마스 이벤트
#ifndef	CHRISTMAS_TREE
		// 마케팅 이벤트 시계 표시
		g_pData->GetMyUser()->GetSelectUnit()->GetTimerGuestUser().restart();
		g_pData->GetMyUser()->GetSelectUnit()->SetReceivedGuestUserReward( true );
		g_pData->GetMyUser()->GetSelectUnit()->SetMarketingEventTime( -1.f );

		std::map< int, float >::iterator it;
		for( it = kEvent.m_mapEventTime.begin() ; it != kEvent.m_mapEventTime.end(); it++ )
		{
			g_pData->GetMyUser()->GetSelectUnit()->SetReceivedGuestUserReward( false );
			if( it->second > g_pData->GetMyUser()->GetSelectUnit()->GetMarketingEventTime() )
			{
				g_pData->GetMyUser()->GetSelectUnit()->SetMarketingEventID( it->first );
				g_pData->GetMyUser()->GetSelectUnit()->SetMarketingEventTime( it->second );
			}
		}

#endif	CHRISTMAS_TREE
		//}} kimhc // 2009-12-04 // 크리스마스 이벤트

#ifdef SERV_LOCAL_RANKING_SYSTEM
		g_pData->GetProfileManager()->SetIsProfileSave(kEvent.m_bLocalRankingUser);
#endif //SERV_LOCAL_RANKING_SYSTEM

#ifdef SERV_ARA_FIRST_SELECT_TUTORIAL
		/// 캐릭터의 첫 접속이면
		if ( 1 == g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_Level &&
			0 == g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_EXP )
		{
			m_bReserveEnterTutorial = true;
			g_pMain->SetIsPlayingTutorial( true );	/// 튜토리얼 실행
		}
#endif // SERV_ARA_FIRST_SELECT_TUTORIAL

#ifdef SERV_GATE_OF_DARKNESS_SUPPORT_EVENT
		g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->SetGateOfDarknessSupportEventTime( kEvent.m_iGateOfDarknessSupportEventTime );
#endif SERV_GATE_OF_DARKNESS_SUPPORT_EVENT

		return true;
	}
	
	return false;
}

#endif SERV_SELECT_UNIT_PACKET_DIVISION

/// 케릭터 선택시 페킷을 사용하지 않는 정보 설정 및 m_iOK 처리
bool CX2StateServerSelect::Handler_EGS_SELECT_UNIT_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_SELECT_UNIT_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	SAFE_DELETE_DIALOG( m_pDLGMsgBox );


	if( NULL != g_pMain && false == g_pMain->DeleteServerPacket( EGS_SELECT_UNIT_ACK ) )
	{
		StateLog( L"invalid sel char ack" );
		return false;
	}


	if( NULL != g_pMain && g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
	{
#ifdef SERV_SELECT_UNIT_PACKET_DIVISION

#ifdef ADD_SERVER_GROUP
		if( NULL != g_pData && NULL != g_pInstanceData && g_pInstanceData->GetServerGroupID() != g_pInstanceData->GetServerGroupIDScript() )
		{
			g_pData->LoadItemManager();
			g_pInstanceData->SetServerGroupIDScript( g_pInstanceData->GetServerGroupID() );
		}		
#endif

#ifdef POSTBOX
		if( NULL != g_pMain->GetPostBox() )
			g_pMain->GetPostBox()->InitPostBox();
#endif

		if( NULL != g_pMain->GetPartyUI() && NULL != g_pMain->GetPartyUI()->GetLocalMapUI() )
		{
			g_pMain->GetPartyUI()->GetLocalMapUI()->ResetLocalMapID();
		}
		// 클리어한 던전중에서 가장 높은 레벨의 던전을 자동 선택
		if( NULL != g_pData && NULL != g_pData->GetPartyManager() && NULL != g_pData->GetPartyManager()->GetMyPartyData() &&
			NULL != g_pData->GetMyUser() && NULL != g_pData->GetMyUser()->GetSelectUnit() &&
			NULL != g_pData->GetDungeonManager())
		{
			g_pData->GetPartyManager()->GetMyPartyData()->m_iDungeonID = g_pData->GetMyUser()->GetSelectUnit()->GetLastClearDungeonID();

			// 클리어한 던전이 하나도 없을 경우에 엘의 나무 서쪽 normal을 선택해준다
			switch( g_pData->GetPartyManager()->GetMyPartyData()->m_iDungeonID )
			{
			case -1:
			case CX2Dungeon::DI_NONE:
			case CX2Dungeon::DI_EL_FOREST_GATE_NORMAL:
				//{{ kimhc // 2010-07-09 // 루벤던전 개편으로 추가
			case CX2Dungeon::DI_EL_FOREST_NORTH_NORMAL:
			case CX2Dungeon::DI_EL_FOREST_WEST_NORMAL:
			case CX2Dungeon::DI_EL_FOREST_HELL_NORMAL:
			case CX2Dungeon::DI_RUBEN_SECRET_COMMON:
				//}} kimc	// 2010-07-09 // 루벤던전 개편으로 추가
				{
					g_pData->GetPartyManager()->GetMyPartyData()->m_iDungeonID = CX2Dungeon::DI_RUBEN_EL_TREE_NORMAL;
				} break;
			}

			//{{ kimhc // 2010.5.2 // 비밀던전 작업(파티리스트)
#ifdef SERV_SECRET_HELL
			switch ( ( g_pData->GetDungeonManager()->GetDungeonType( g_pData->GetPartyManager()->GetMyPartyData()->m_iDungeonID ) ) )
			{
			case CX2Dungeon::DT_SECRET:
				{
					g_pData->GetPartyManager()->GetMyPartyData()->m_iDungeonMode = CX2Dungeon::DM_SECRET_NORMAL;
				} break;

			default:
				break;
			}
#endif SERV_SECRET_HELL
			//}} kimhc // 2010.5.2 // 비밀던전 작업(파티리스트)
		}

		if( NULL != g_pData && NULL != g_pData->GetMyUser() )
		{
			CX2Unit* pUnit = g_pData->GetMyUser()->GetSelectUnit();

			if( NULL != pUnit )
			{
				wstringstream wstreamNickNameAndUid;
				wstreamNickNameAndUid << L"UserUID: " << pUnit->GetOwnerUserUID(); 
				wstreamNickNameAndUid << L" NickName: " << pUnit->GetNickName();
				wstreamNickNameAndUid << L" UnitUID: " << pUnit->GetUID();
				StateLog( wstreamNickNameAndUid.str().c_str() );

#ifndef OPEN_TEST_1_NO_MESSENGER_CASHSHOP
#ifndef NEW_MESSENGER
				if ( g_pMain->GetNexonLoginMessenger() == true )
				{
					wstring tempNickName = CX2Community::ConvertFullName( pUnit->GetNickName() );
					switch( g_pMain->GetPublisher() )
					{
#ifdef _NEXON_KR_
					case CX2Main::XP_NEXON_KOREA:
						{
#ifdef _USE_NEXON_MSG_INHOUSE // 접속할 캐릭터의 serialnum을 하드코딩시켜준후 넥슨로그인한다.
							pUnit->GetUnitData()->m_iNMKSerialNum = 52890;	// "지끄무" 캐릭터
							//pUnit->GetUnitData()->m_iNMKSerialNum = 3712941;	// "피똥싼이브" 캐릭터
							//pUnit->GetUnitData()->m_iNMKSerialNum = 1928754;	// "툩" 캐릭터
#endif

							if( CNMCOClientObject::GetInstance().LoginVirtual( tempNickName.c_str(), (_UInt32_)pUnit->GetUnitData()->m_iNMKSerialNum, GET_STRING( STR_ID_0 ) ) == FALSE )						
							{
								g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_710 ), g_pMain->GetNowState() );
							}
							else
							{
								g_pMain->SetNexonVirtualLogin( true );							
							}

							CNMCOClientObject::GetInstance().ChangeMyLevel( ( (UINT32)pUnit->GetUnitData()->m_UnitClass << 24 ) | (UINT32)pUnit->GetUnitData()->m_Level );



							// 닉네임 바꾼 직후라면 닉네임이 바뀌었다고 넥슨메신저에 이벤트 전송
							if( true == m_bJustChangedNickName )
							{
								m_bJustChangedNickName = false;

								NMVirtualKey nmVirtualKey;
								nmVirtualKey.uGameCode		= NEXON_KOREA_ELSWORD_GAMECODE;
								nmVirtualKey.uVirtualIDCode = (UINT32) pUnit->GetUnitData()->m_iNMKSerialNum;

								if( false == g_pData->GetMyUser()->GetUserData()->m_bIsGuestUser ) // 체험 아이디 제한
								{
									if ( CNMCOClientObject::GetInstance().ChangeNickname( nmVirtualKey, tempNickName.c_str() ) == FALSE )
									{
										g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_711 ), this );
									}
								}
							}
#ifdef SEND_URL_TO_NEXON
							// game start
							static bool bSended = false;
							if( bSended == false )
							{
#ifdef SEND_NEXON_WISE_LOG_BY_URL // 김태환
								CX2User::UserData* pUserData = g_pData->GetMyUser()->GetUserData();

								if ( NULL != g_pMain->GetBrowserWrapper() && NULL != pUserData )
								{
									if( false == g_pMain->GetBrowserWrapper()->IsClosed() )
										g_pMain->GetBrowserWrapper()->CloseBrowser();

									RECT		rcWebPage;
									char		buf[255]	= {0,};
									std::string strURL		= "http://elsword.nexon.com/wiselog/gamestart.aspx";				/// 로그 URL
									::WideCharToMultiByte( CP_ACP, 0, pUserData->userID.c_str(), -1, buf, 255, NULL, NULL );	/// wstring을 string 으로 변환

									std::string strNXID		= buf;					/// 넥슨 아이디
									UidType		uidNXUID	= pUserData->userUID;	/// 넥슨 UID

									g_pMain->GetBrowserWrapper()->Create( rcWebPage, g_pKTDXApp->GetHWND() );

									if ( true == g_pData->GetMyUser()->GetFirstCreateCharacter() )		/// 신규 유저 ( 기존에 케릭터를 생성하지 않은 유저 )
										strURL += "?GRU=true";	/// 신규 유저일 경우 추가되는 인자

									StringCchPrintfA( buf, 255, "&NXID=%s&OID=%lld", strNXID.c_str(), uidNXUID );	/// 넥슨 아이디와 UID를 설정

									strURL += buf;				/// 스트링 조합

									g_pMain->GetBrowserWrapper()->Navigate( strURL.c_str(), NULL );					/// 웹페이지 URL로 설정
								}
#endif // SEND_NEXON_WISE_LOG_BY_URL

								NMCOHelpers::WriteToWiselog( L"http://elsword.nexon.com/wiselog/gamestart.aspx" );
								bSended = true;
							}						
#endif
						}
						break;
#endif _NEXON_KR_
					}
				}
#endif NEW_MESSENGER
#endif OPEN_TEST_1_NO_MESSENGER_CASHSHOP
			}
		}

		Handler_EGS_CHECK_BALANCE_REQ();

		if ( g_pInstanceData != NULL )
		{
			g_pInstanceData->SetShowCSPandSlotBDate( true );
#ifdef SERV_HACKING_TOOL_LIST
			g_pInstanceData->SetChangeHackList(true);
#endif
		}

		//{{ 허상형 : [2009/11/3] //	채팅 박스 초기화
		if( g_pChatBox != NULL )
		{
			g_pChatBox->ClearAllChatLog();
			g_pChatBox->ChangeChatWindowAndIMEEditBox( CX2ChatBox::CWM_TOTAL );
		}

#ifdef SKILL_SLOT_UI_TYPE_B
		if( NULL != g_pMain->GetGameOption())
		{
			g_pMain->GetGameOption()->SetIsSkillUITypeA( g_pMain->GetGameOption()->GetIsSkillUITypeA(), false );				
		}
#endif //SKILL_SLOT_UI_TYPE_B

#else  SERV_SELECT_UNIT_PACKET_DIVISION
		CX2BattleFieldManager& battleFieldManager = g_pData->GetBattleFieldManager();
		battleFieldManager.SetBattleFieldIdWhereIam( 0 );

		// 휴면유저인지 확인후 팝업을 띄운다. -- 현재 쓰이지 않는 이벤트
		//switch(kEvent.m_iSelectUnitEvent)
		//{
		//case 0:	// 무시
		//	break;
		//case 1:	// 컴백유저
		//	g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_2634 ), g_pMain->GetNowState() );
		//	break;
		//case 2:	// 컴백인데 재접속한 유저
		//	g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_2635 ), g_pMain->GetNowState() );
		//	break;
		//case 3:	// 기존유저 보상
		//	g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_2636 ), g_pMain->GetNowState() );
		//	break;
		//case 4: // 신규 재접속
		//	g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_2640 ), g_pMain->GetNowState() );
		//	break;
		//default:
		//	break;
		//}

		if( NULL != g_pMain->GetTutorSystem() )
		{
			g_pMain->GetTutorSystem()->ResetTutorStudent( kEvent.m_vecTutorialDBUnitInfo );
		}

		if ( m_bReserveEnterTutorial == true )
		{
			g_pMain->SetIsPlayingTutorial( true );
		}

		CX2Unit* pUnit = g_pData->GetMyUser()->GetUnitByUID( kEvent.m_kUnitInfo.m_nUnitUID );
		g_pData->GetMyUser()->SelectUnit( pUnit );
		pUnit->SetBlackList( kEvent.m_vecChatBlackList ); 
//{{ 오현빈 // 2012-04-06 // 이벤트 받아오기 전 타이머 초기화 시킬 수 있도록 수정
#ifdef FIX_EVENT_TIMER	
		pUnit->m_mapEventData.clear();
		pUnit->SetReceivedGuestUserReward( true );		
#endif //FIX_EVENT_TIMER
//}} 오현빈 // 2012-04-06 // 이벤트 받아오기 전 타이머 초기화 시킬 수 있도록 수정


#ifdef ADD_SERVER_GROUP
		if( g_pData != NULL && g_pInstanceData->GetServerGroupID() != g_pInstanceData->GetServerGroupIDScript() )
		{
			g_pData->LoadItemManager();
			g_pInstanceData->SetServerGroupIDScript( g_pInstanceData->GetServerGroupID() );
		}		
#endif


#ifdef POSTBOX
		g_pMain->GetPostBox()->InitPostBox();
#endif


		
		pUnit->Reset( kEvent.m_kUnitInfo, true );

		//{{ kimhc // 2010-01-04 // 기존의 EGS_SELECT_UNIT_ACK에 인벤정보가 포함되어 오던 것 분할
#ifdef	PC_BANG_WORK
		pUnit->ResetInventorySize( kEvent.m_mapInventorySlotSize );
		pUnit->ResetInventoryItems( kEvent.m_mapItem ); // 장착중인 장비만 먼저 Reset
#else	PC_BANG_WORK
		//kimhc // 2008-08-04 // m_mapInventorySlotSize에서 키값이 ST_BANK인 
		// value는 은행슬롯의 사이즈 이고 이 사이즈로 회원 등급을 구분한다.
		pUnit->ResetInventory( kEvent.m_mapInventorySlotSize, kEvent.m_mapItem );
#endif	PC_BANG_WORK
		//}} kimhc // 2010-01-04 // 기존의 EGS_SELECT_UNIT_ACK에 인벤정보가 포함되어 오던 것 분할

		pUnit->SetResurrectionStoneNum( kEvent.m_iNumResurrectionStone );

		//{{ 허상형 : [2009/10/14] //	자동 결제 부활석
#ifdef AUTO_PAYMENT
		pUnit->SetAutoResStoneNum( kEvent.m_iNumAutoPaymentResStone );
#endif	//	AUTO_PAYMENT
		//}} 허상형 : [2009/10/14] //	자동 결제 부활석

		// skill list
		//pUnit->GetUnitData()->m_UserSkillTree.SetUnitClass( (int) kEvent.m_kUnitInfo.m_cUnitClass );
		pUnit->GetUnitData()->m_UserSkillTree.SetAcquiredSkill( kEvent.m_vecSkillAcquired );
		pUnit->GetUnitData()->m_UserSkillTree.SetUnsealedSkill( kEvent.m_vecSkillUnsealed );
 		pUnit->GetUnitData()->m_UserSkillTree.SetEquippedSkill( kEvent.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkill, kEvent.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkillSlotB );
// 		pUnit->GetUnitData()->m_UserSkillTree.SetSkillSlotBExpirationState( (CX2UserSkillTree::SKILL_SLOT_B_EXPIRATION_STATE) kEvent.m_kUnitInfo.m_UnitSkillData.m_cSkillSlotBExpirationState );
// 		pUnit->GetUnitData()->m_UserSkillTree.SetSkillSlotBEndDateString( kEvent.m_kUnitInfo.m_UnitSkillData.m_wstrSkillSlotBEndDate );


		if( g_pData->GetUIManager() != NULL)
		{
			g_pData->GetUIManager()->CreateUISkillTree();
			g_pData->GetUIManager()->GetUISkillTree()->InitSkillTreeUI();
		}
#ifdef GUILD_SKILL
		//{{  [11/11/2009 : oasis907 ]

		// 임시: 서버에게서 GuildUserSkillTree 정보를 받았다 가정
/*
		// skill list
		pUnit->GetUnitData()->m_UserSkillTree.SetGuildClass(0);
		pUnit->GetUnitData()->m_UserSkillTree.SetAcquiredGuildSkill( kEvent.m_vecSkillAcquired ); // 함수 임시 변경
*/		
		
		//pUnit->GetUnitData()->m_GuildUserSkillTree.SetUnsealedSkill( kEvent.m_vecSkillUnsealed );


		// 함수 임시 변경
		//pUnit->GetUnitData()->m_UserSkillTree.SetEquippedSkill( kEvent.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkill, kEvent.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkillSlotB );

		//pUnit->GetUnitData()->m_GuildUserSkillTree.SetSkillSlotBExpirationState( (CX2GuildUserSkillTree::SKILL_SLOT_B_EXPIRATION_STATE) kEvent.m_kUnitInfo.m_UnitSkillData.m_cSkillSlotBExpirationState );
		//pUnit->GetUnitData()->m_GuildUserSkillTree.SetSkillSlotBEndDateString( kEvent.m_kUnitInfo.m_UnitSkillData.m_wstrSkillSlotBEndDate );

/*
		if( g_pData->GetGuildManager()->GetUIGuild()->GetUIGuildSkillTree() != NULL)
		{
			g_pData->GetGuildManager()->GetUIGuild()->CreateUIGuildSkillTree();
			g_pData->GetGuildManager()->GetUIGuild()->GetUIGuildSkillTree()->InitSkillTreeUI();
		}
		//}}  [11/11/2009 : oasis907 ]
*/
#endif GUILD_SKILL

#ifdef SERV_SKILL_NOTE		
		pUnit->SetSkillNote( kEvent.m_mapSkillNote, kEvent.m_cSkillNoteMaxPageNum );
		pUnit->GetUnitData()->m_UserSkillTree.SetEqipSkillMemo( kEvent.m_kUnitInfo.m_UnitSkillData.m_vecSkillNote );
#endif

#ifdef TITLE_SYSTEM
#ifdef SERV_TITLE_DATA_SIZE
		pUnit->GetUnitData()->m_iTitleId = kEvent.m_kUnitInfo.m_iTitleID;
#else
		pUnit->GetUnitData()->m_iTitleId = kEvent.m_kUnitInfo.m_sTitleID;
#endif

		if(g_pData != NULL && g_pData->GetTitleManager() != NULL)
		{
			g_pData->GetTitleManager()->InitRecord();
			g_pData->GetTitleManager()->AttachTitle(pUnit->GetUnitData()->m_iTitleId);

			g_pData->GetTitleManager()->TakeTitle(kEvent.m_vecTitle);     
			g_pData->GetTitleManager()->UpdateMission(kEvent.m_vecMission);
		}                
#endif

#ifdef SERV_PET_SYSTEM

		pUnit->SetSummonPetUid( kEvent.m_iSummonedPetUID );
		//if( g_pData != NULL && g_pData->GetPetManager() != NULL )
		//	g_pData->GetPetManager()->Handler_EGS_SUMMON_PET_REQ( kEvent.m_iSummonedPetUID );
		if( kEvent.m_vecPetList.size() > 0 )
			pUnit->SetFullPetInfo(kEvent.m_vecPetList[0]);
		
		if( g_pData != NULL && g_pData->GetPetManager() != NULL )
		{
			g_pData->GetPetManager()->AddMyPetList( kEvent.m_vecCanEvolutionPetList );
			g_pData->GetPetManager()->AddNeverSummonPetList( kEvent.m_vecNeverSummonPetList, kEvent.m_iSummonedPetUID );

		}
		
		// oasis907 : 김상윤 [2010.9.7] // 
		if( kEvent.m_vecPetList.size() > 0 )
		{
			KPetInfo kPetInfo = kEvent.m_vecPetList[0];
			int EvolutionStep = kPetInfo.m_cEvolutionStep;
#ifdef SERV_PETID_DATA_TYPE_CHANGE //2013.07.02
			int iInvenSlotSize = g_pData->GetPetManager()->GetPetInventorySlotSize((CX2PetManager::PET_UNIT_ID) kPetInfo.m_iPetID, EvolutionStep);
#else //SERV_PETID_DATA_TYPE_CHANGE
			int iInvenSlotSize = g_pData->GetPetManager()->GetPetInventorySlotSize((CX2PetManager::PET_UNIT_ID) kPetInfo.m_cPetID, EvolutionStep);
#endif //SERV_PETID_DATA_TYPE_CHANGE

			pUnit->GetInventory()->SetItemMaxNum(CX2Inventory::ST_PET, iInvenSlotSize);
		
		

			// oasis907 : 김상윤 [2010.9.8] // 자동 먹이 주기 기능
			if( g_pData->GetUIManager()->GetUIInventory() != NULL)
			{
				g_pData->GetUIManager()->GetUIInventory()->SetAutoFeed(kPetInfo.m_bAutoFeed);
			}
			//}}
		}
		//}}
#endif

		g_pMain->GetGameOption()->SetDenyInviteGuild( (CX2GameOption::BlackListDenyState) kEvent.m_kDenyOptions.m_cDenyInviteGuild );				
		g_pMain->GetGameOption()->SetDenyPersonalTrade( (CX2GameOption::BlackListDenyState) kEvent.m_kDenyOptions.m_cDenyPersonalTrade );
		g_pMain->GetGameOption()->SetDenyFriendship( (CX2GameOption::BlackListDenyState) kEvent.m_kDenyOptions.m_cDenyFriendShip );
		g_pMain->GetGameOption()->SetDenyParty( (CX2GameOption::BlackListDenyState) kEvent.m_kDenyOptions.m_cDenyParty );

		g_pMain->GetGameOption()->SetRefuseParty( (CX2GameOption::BlackListDenyState) kEvent.m_kDenyOptions.m_cDenyParty );
		g_pMain->GetGameOption()->SetRefuseFriend( (CX2GameOption::BlackListDenyState) kEvent.m_kDenyOptions.m_cDenyFriendShip );
		g_pMain->GetGameOption()->SetRefusePersonalTrade( (CX2GameOption::BlackListDenyState) kEvent.m_kDenyOptions.m_cDenyPersonalTrade );
		//{{ kimhc // 2009-10-12 // 길드 초대 거부 추가
#ifdef	GUILD_MANAGEMENT
		g_pMain->GetGameOption()->SetRefuseGuildInvitation( static_cast< CX2GameOption::BlackListDenyState >( kEvent.m_kDenyOptions.m_cDenyInviteGuild ) );
#endif	GUILD_MANAGEMENT
		//}} kimhc // 2009-10-12 // 길드 초대 거부 추가

		if( NULL != g_pMain && NULL != g_pMain->GetPartyUI() && NULL != g_pMain->GetPartyUI()->GetLocalMapUI() )
		{
			g_pMain->GetPartyUI()->GetLocalMapUI()->ResetLocalMapID();
		}
		// 클리어한 던전중에서 가장 높은 레벨의 던전을 자동 선택
		if( NULL != g_pData->GetPartyManager() )
		{
			g_pData->GetPartyManager()->GetMyPartyData()->m_iDungeonID = g_pData->GetMyUser()->GetSelectUnit()->GetLastClearDungeonID();

			// 클리어한 던전이 하나도 없을 경우에 엘의 나무 서쪽 normal을 선택해준다
			switch( g_pData->GetPartyManager()->GetMyPartyData()->m_iDungeonID )
			{
			case -1:
			case CX2Dungeon::DI_NONE:
			case CX2Dungeon::DI_EL_FOREST_GATE_NORMAL:
				//{{ kimhc // 2010-07-09 // 루벤던전 개편으로 추가
			case CX2Dungeon::DI_EL_FOREST_NORTH_NORMAL:
			case CX2Dungeon::DI_EL_FOREST_WEST_NORMAL:
			case CX2Dungeon::DI_EL_FOREST_HELL_NORMAL:
			case CX2Dungeon::DI_RUBEN_SECRET_COMMON:
				//}} kimc	// 2010-07-09 // 루벤던전 개편으로 추가
				{
					g_pData->GetPartyManager()->GetMyPartyData()->m_iDungeonID = CX2Dungeon::DI_RUBEN_EL_TREE_NORMAL;
				} break;
			}

		//{{ kimhc // 2010.5.2 // 비밀던전 작업(파티리스트)
		#ifdef SERV_SECRET_HELL
			switch ( ( g_pData->GetDungeonManager()->GetDungeonType( g_pData->GetPartyManager()->GetMyPartyData()->m_iDungeonID ) ) )
			{
			case CX2Dungeon::DT_SECRET:
				{
					g_pData->GetPartyManager()->GetMyPartyData()->m_iDungeonMode = CX2Dungeon::DM_SECRET_NORMAL;
				} break;

			default:
				break;
			}
		#endif SERV_SECRET_HELL
		//}} kimhc // 2010.5.2 // 비밀던전 작업(파티리스트)
		}

	//{{ kimhc // 2010.4.12 // 비밀던전 작업(일일퀘스트 시스템)
#ifdef SERV_SECRET_HELL
		g_pInstanceData->SetPreCheckTimeForDailyQuest( g_pData->GetServerCurrentTime() );
#endif SERV_SECRET_HELL
	//}} kimhc // 2010.4.12 // 비밀던전 작업(일일퀘스트 시스템)

		g_pData->GetQuestManager()->SetUnitQuest( kEvent.m_vecQuest, kEvent.m_vecCompletQuest );
		
		//{{ kimhc // GiveUpForbiddenQuest()를 EGS_GET_MY_INVENTORY_ACK로 이동함
	#ifndef	SERV_EPIC_QUEST
		g_pData->GetQuestManager()->GiveUpForbiddenQuest();
	#endif	SERV_EPIC_QUEST
		//}} kimhc // GiveUpForbiddenQuest()를 EGS_GET_MY_INVENTORY_ACK로 이동함


#ifdef	SERV_EPIC_QUEST
		// oasis907 : 김상윤 [2010.7.21] // 퀵퀘스트 창 열기 초기화
		if(g_pData->GetUIManager() != NULL &&
			g_pData->GetUIManager()->GetUIQuestNew() != NULL)
		{
			g_pData->GetUIManager()->GetUIQuestNew()->InitUserShowQuickQuestDLG(true);
		}
#endif	SERV_EPIC_QUEST

		g_pMain->GetInformerManager()->Reset();
		g_pMain->GetLVUpEventMgr()->Reset( pUnit->GetUnitData()->m_Level );

		if ( g_pData->GetCashShop() != NULL )
			g_pData->GetCashShop()->ResetUnitViewer( pUnit );

		g_pMain->ResetCrashReport( L"" );


#ifndef OPEN_TEST_1_NO_MESSENGER_CASHSHOP
#ifndef NEW_MESSENGER
		if ( g_pMain->GetNexonLoginMessenger() == true )
		{
			wstring tempNickName = CX2Community::ConvertFullName( pUnit->GetNickName() );
			switch( g_pMain->GetPublisher() )
			{
#ifdef _NEXON_KR_
				case CX2Main::XP_NEXON_KOREA:
					{
#ifdef _USE_NEXON_MSG_INHOUSE // 접속할 캐릭터의 serialnum을 하드코딩시켜준후 넥슨로그인한다.
						pUnit->GetUnitData()->m_iNMKSerialNum = 52890;	// "지끄무" 캐릭터
						//pUnit->GetUnitData()->m_iNMKSerialNum = 3712941;	// "피똥싼이브" 캐릭터
						//pUnit->GetUnitData()->m_iNMKSerialNum = 1928754;	// "툩" 캐릭터
#endif

						if( CNMCOClientObject::GetInstance().LoginVirtual( tempNickName.c_str(), (_UInt32_)pUnit->GetUnitData()->m_iNMKSerialNum, GET_STRING( STR_ID_0 ) ) == FALSE )						
						{
							g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_710 ), g_pMain->GetNowState() );
						}
						else
						{
							g_pMain->SetNexonVirtualLogin( true );							
						}

						CNMCOClientObject::GetInstance().ChangeMyLevel( ( (UINT32)pUnit->GetUnitData()->m_UnitClass << 24 ) | (UINT32)pUnit->GetUnitData()->m_Level );



						// 닉네임 바꾼 직후라면 닉네임이 바뀌었다고 넥슨메신저에 이벤트 전송
						if( true == m_bJustChangedNickName )
						{
							m_bJustChangedNickName = false;

							NMVirtualKey nmVirtualKey;
							nmVirtualKey.uGameCode		= NEXON_KOREA_ELSWORD_GAMECODE;
							nmVirtualKey.uVirtualIDCode = (UINT32) pUnit->GetUnitData()->m_iNMKSerialNum;

							if( false == g_pData->GetMyUser()->GetUserData()->m_bIsGuestUser ) // 체험 아이디 제한
							{
								if ( CNMCOClientObject::GetInstance().ChangeNickname( nmVirtualKey, tempNickName.c_str() ) == FALSE )
								{
									g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_711 ), this );
								}
							}
						}

#ifdef SEND_URL_TO_NEXON
						// game start
						static bool bSended = false;
						if( bSended == false )
						{
							NMCOHelpers::WriteToWiselog( L"http://elsword.nexon.com/wiselog/gamestart.aspx" );
							bSended = true;
						}						
#endif
					}
					break;
#endif _NEXON_KR_
			}
		}
#endif NEW_MESSENGER
#endif OPEN_TEST_1_NO_MESSENGER_CASHSHOP


		Handler_EGS_CHECK_BALANCE_REQ();


		//{{ kimhc // 2010-01-04 // 기존의 EGS_SELECT_UNIT_ACK에 인벤정보가 포함되어 오던 것 분할
#ifdef	PC_BANG_WORK
		g_pData->GetServerProtocol()->SendID( EGS_GET_MY_INVENTORY_REQ );
		g_pMain->AddServerPacket( EGS_GET_MY_INVENTORY_ACK  );
#else	PC_BANG_WORK
		
		// 체험 아이디 제한 
		if( false == g_pData->GetMyUser()->GetUserData()->m_bIsGuestUser &&
			kEvent.m_bIsRecommend == false && m_bCheckRecommend == true )
		{
#ifdef SERV_NO_GAMEINVITE
			Handler_EGS_RECOMMEND_USER_REQ( L"", true );
#else SERV_NO_GAMEINVITE
			OpenRecommendMsgBox( true );
#endif SERV_NO_GAMEINVITE
		}
		else
		{

			g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_PartyTalkBoxInfo.m_iPartyUID = 0;

			if(g_pMain->GetIsPlayingTutorial() == true)
			{
				// 튜토리얼 진입요청
				Handler_EGS_CREATE_TUTORIAL_ROOM_REQ();
			}
			else
			{
				// 새마을로 진입요청
				Handler_EGS_STATE_CHANGE_FIELD_REQ();
			}
		}

#endif	PC_BANG_WORK
		//}} kimhc // 2010-01-04 // 기존의 EGS_SELECT_UNIT_ACK에 인벤정보가 포함되어 오던 것 분할

#ifdef SERV_PSHOP_AGENCY
		if( g_pInstanceData != NULL )
		{
			g_pInstanceData->SetPShopAgencyInfo( kEvent.m_kPShopAgencyInfo.m_bIsPShopOpen, kEvent.m_kPShopAgencyInfo.m_wstrAgencyExpirationDate );
		}
#endif


		//{{ kimhc // 2009-12-04 // 크리스마스 이벤트
#ifndef	CHRISTMAS_TREE
		// 마케팅 이벤트 시계 표시
		g_pData->GetMyUser()->GetSelectUnit()->GetTimerGuestUser().restart();
		g_pData->GetMyUser()->GetSelectUnit()->SetReceivedGuestUserReward( true );
		g_pData->GetMyUser()->GetSelectUnit()->SetMarketingEventTime( -1.f );

		std::map< int, float >::iterator it;
		for( it = kEvent.m_mapEventTime.begin() ; it != kEvent.m_mapEventTime.end(); it++ )
		{
			g_pData->GetMyUser()->GetSelectUnit()->SetReceivedGuestUserReward( false );
			if( it->second > g_pData->GetMyUser()->GetSelectUnit()->GetMarketingEventTime() )
			{
				g_pData->GetMyUser()->GetSelectUnit()->SetMarketingEventID( it->first );
				g_pData->GetMyUser()->GetSelectUnit()->SetMarketingEventTime( it->second );
			}
		}

#endif	CHRISTMAS_TREE
		//}} kimhc // 2009-12-04 // 크리스마스 이벤트


		// 친구채팅창 히스토리 초기화 및 친구리스트 초기 설정
		if(g_pData != NULL && g_pData->GetMessenger() != NULL)
		{
			g_pData->GetMessenger()->ClearChatSessionList();
			g_pData->GetMessenger()->Reset();		
			g_pData->GetMessenger()->ResetHistory();
#ifdef NEW_MESSENGER
			g_pData->GetMessenger()->SetFriendList( kEvent.m_kMessengerInfo );
#else		
			g_pData->GetMessenger()->SyncFriendList();
#endif NEW_MESSENGER
		}




		if ( g_pInstanceData != NULL )
		{
			g_pInstanceData->SetShowCSPandSlotBDate( true );
#ifdef SERV_HACKING_TOOL_LIST
			g_pInstanceData->SetChangeHackList(true);
#endif
		}


		//{{ 허상형 : [2009/11/3] //	채팅 박스 초기화
		if( g_pChatBox != NULL )
		{
			g_pChatBox->ClearAllChatLog();
			g_pChatBox->ChangeChatWindowAndIMEEditBox( CX2ChatBox::CWM_TOTAL );
		}
#ifdef NEW_HENIR_TEST
		g_pData->GetMyUser()->GetSelectUnit()->SetHenirRewardCountInfo( kEvent.m_PacketHenirRewardCount );
#endif NEW_HENIR_TEST

#ifdef SERV_RANDOM_DAY_QUEST
		g_pData->GetQuestManager()->SetRandomQuestInfo( kEvent.m_mapRandomQuestList );
#endif SERV_RANDOM_DAY_QUEST

		CX2GageManager* pGageManager = CX2GageManager::GetInstance();
		pGageManager->CreateMyGageUI( pUnit->GetClass(), pUnit->GetUID() );
		pGageManager->InitMySkillCoolTimeList( pUnit->GetUnitData()->m_UserSkillTree );
		//pGageManager->InitMyQuickSlotCoolTimeList( pUnit->GetInventory()->GetItemMaxNum( CX2Inventory::ST_E_QUICK_SLOT ) );

		
		ASSERT( !kEvent.m_kGamePlayStatus.m_vecGamePlayStatus.empty() );
		if ( !kEvent.m_kGamePlayStatus.m_vecGamePlayStatus.empty() )
		{
			pGageManager->SetMyPlayStatusFromPacket( kEvent.m_kGamePlayStatus.m_vecGamePlayStatus[0] );
			pGageManager->UpdateNowHpPercent();
			pGageManager->UpdateNowMpPercent();
		}

		pUnit->GetUnitData()->m_UserSkillTree.InitSkillCoolTimeFromGageManager( pGageManager->GetSkillCoolTimeList() );
		g_pData->GetUIManager()->GetUIQuickSlot()->InitMyQuickSlotCoolTimeFromGageManager( pGageManager->GetQuickSlotCoolTimeList() );
//		const float fResultMaxMp = pUnit->ResetMaxMp( ONE_CHARGE * 3.0f );
//		pGageManager->SetMaxMp( pUnit->GetUID(), fResultMaxMp );
		// GameStat으로 변경 해야 함, 그리고 한번에 넣을 수 있는 법도...
// 		pGageManager->SetMaxHp( pUnit->GetUID(), static_cast<const float>( kEvent.m_kUnitInfo.m_kStat.m_iBaseHP ) );
// 		pGageManager->SetNowHp( pUnit->GetUID(), static_cast<const float>( kEvent.m_kUnitInfo.m_kStat.m_iBaseHP ) );	
// 		pGageManager->SetMaxSoul( pUnit->GetUID(), 100.0f );

#ifdef REFORM_QUEST
		if( NULL != g_pData && NULL != g_pData->GetUIManager() && NULL != g_pData->GetUIManager()->GetUIQuestNew() )
		{
			g_pData->GetUIManager()->GetUIQuestNew()->SetOpenEpicCategory();
			g_pData->GetUIManager()->GetUIQuestNew()->InitNewQuestList();
		}
#endif //REFORM_QUEST

#ifdef SERV_EVENT_MONEY
		if( NULL != g_pData && NULL != g_pData->GetMyUser() )
		{
			g_pData->GetMyUser()->SetUserAngelsFeather(kEvent.m_iAngelFeather);
		}
		if( NULL != g_pInstanceData && NULL != g_pInstanceData->GetMiniMapUI() )
		{
			g_pInstanceData->GetMiniMapUI()->UpdateAngelsFeatherCount(kEvent.m_iAngelFeather);
		}
#endif //SERV_EVENT_MONEY

#ifdef SKILL_SLOT_UI_TYPE_B
		if( NULL != g_pMain && NULL != g_pMain->GetGameOption())
		{
			g_pMain->GetGameOption()->SetIsSkillUITypeA( g_pMain->GetGameOption()->GetIsSkillUITypeA(), false );				
		}
#endif //SKILL_SLOT_UI_TYPE_B

#endif SERV_SELECT_UNIT_PACKET_DIVISION
	}
	else if( kEvent.m_iOK == NetError::ERR_SELECT_UNIT_06 )
	{
		//닉네임이 없는 캐릭터는 닉네임을 새로 입력하라는 팝업창을 띄운다.
		OpenRenameMsgBox( true );
		SetRenameMsgBoxError( L"" );
	}
	//{{ 2009. 4. 29  최육사	잘못된 상태에서 SELECT_UNIT_REQ를 보내면 돌아오는 ACK 에러 코드에 대한 처리
	else if( kEvent.m_iOK == NetError::ERR_WRONG_STATE_00 )
	{
		// 정상처리?
		return true;
	}
	//}} 2009. 4. 29  최육사
	else
	{
		if( true == g_pMain->GetStateAutoChanger().GetIsAutoChangingState() )
		{
			g_pMain->GetStateAutoChanger().EndStateChange();
		}
	}


	m_bJustChangedNickName = false;


	// kimhc // 2010-02-03 // PC방인 경우 PC방 탭이 디폴드가 되도록 변경
	if ( g_pInstanceData->m_bFirstSelect == false &&
		 g_pData->GetMyUser() != NULL &&
		 g_pData->GetMyUser()->GetIsPCRoom() == true
#ifdef SERV_PC_BANG_TYPE
		 && g_pData->GetPremiumBuffManager() != NULL
		 && g_pData->GetPremiumBuffManager()->IsPcBangAvatar() == true
#endif SERV_PC_BANG_TYPE
		 )
	{
		if ( g_pData->GetUIManager() != NULL &&
			g_pData->GetUIManager()->GetUIInventory() != NULL )
			g_pData->GetUIManager()->GetUIInventory()->SetSortType( CX2Inventory::ST_PC_BANG );
	}

	g_pInstanceData->m_bFirstSelect = true;


	//{{ kimhc // 2009-01-12 // 가이아 서버에만 추천인 리스트 보이는 기능
#ifdef	VIEW_REFEREE_LIST
	if ( g_pData != NULL &&
		 g_pData->GetMessenger() != NULL )
	{
		switch ( g_pInstanceData->GetServerGroupID() )
		{
		case SGI_GAIA:
			{
				g_pData->GetMessenger()->SetRefereeUI( true );
			}
			break;

		default:
			{
				g_pData->GetMessenger()->SetRefereeUI( false );
			}
			break;
		}
	}
#endif	VIEW_REFEREE_LIST
	//}} kimhc // 2009-01-12 // 가이아 서버에만 추천인 리스트 보이는 기능

#ifdef SERV_RECRUIT_EVENT_BASE
	if( g_pData != NULL &&
		g_pData->GetMessenger() != NULL )
	{
		g_pData->GetMessenger()->SetRecruitUI( true );
	}
#endif SERV_RECRUIT_EVENT_BASE

	//펫리스트 UI를 열지 않고 펫 소지 여부를 파악 하기 위해 추가
	if( NULL != g_pData && NULL != g_pData->GetUIManager() )
		g_pData->GetUIManager()->CreatePetList();

#ifdef RIDING_SYSTEM
	if( NULL != g_pData )
	{
		if( NULL != g_pData->GetUIManager() &&
			NULL != g_pData->GetUIManager()->GetUIPetList() )
			g_pData->GetUIManager()->GetUIPetList()->Handler_EGS_GET_RIDING_PET_LIST_REQ(0);
	}
#endif //RIDING_SYSTEM

#ifdef SERV_2012_PVP_SEASON2_EVENT
	if( NULL != g_pData && NULL != g_pData->GetPremiumBuffManager() )		/// 케릭터 클래스 생성 시점부터 영광의 시간 열려있으면, 해당 버프 삽입
		g_pData->GetPremiumBuffManager()->SetLocalTimeOfGloryBuff();
#endif SERV_2012_PVP_SEASON2_EVENT

#ifdef SERV_LOCAL_RANKING_SYSTEM
	g_pData->LoadAreaData();
#endif //SERV_LOCAL_RANKING_SYSTEM


#ifdef ADDED_RELATIONSHIP_SYSTEM
	if ( NULL != g_pData &&
		 NULL != g_pData->GetMessenger() &&
		 NULL != g_pData->GetRelationshipManager() &&
		 NULL != g_pData->GetRelationshipManager()->GetMyRelationshipInfo() )
	{
		SEnum::RELATIONSHIP_TYPE eType = g_pData->GetRelationshipManager()->GetMyRelationshipInfo()->m_eRelationStateType;
		g_pData->GetMessenger()->SetRelationshipUI( eType );
	}
#endif // ADDED_RELATIONSHIP_SYSTEM

#ifdef FESTIVAL_UI
#ifndef NEW_EMBLEM_MANAGER
	if( NULL != g_pMain && NULL != g_pData )
	{
		CKTDGParticleSystem::CParticleEventSequenceHandle SeqFestivalUI = 
			g_pData->GetUIMajorParticle()->CreateSequenceHandle( NULL, L"UI_Festival_StartEffect", 512.f+20.f, 284.f+60.f+20.f, 0.f, 9999.f, 9999.f, -1, 1, -1.0f, true, 2.5f, false );

		if( INVALID_PARTICLE_HANDLE != SeqFestivalUI )
		{
			CKTDGParticleSystem::CParticleEventSequence* pParticle = g_pData->GetUIMajorParticle()->GetInstanceSequence( SeqFestivalUI );
			g_pMain->SetFestivalSeq( SeqFestivalUI );
			if( pParticle != NULL )
			{
				pParticle->SetOverUI( true );
			};
		}
	}
#endif //NEW_EMBLEM_MANAGER
#endif //FESTIVAL_UI

	return true;
}

//bool CX2StateUnitSelect::SelectUnitTimeOutAck( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
//{
//SAFE_DELETE_DIALOG( m_pDLGMsgBox );
//g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), L"서버에 응답이 없습니다\n유닛을 선택할 수 없습니다", this );
//return true;
//}

bool CX2StateServerSelect::Handler_EGS_DELETE_UNIT_REQ()
{
	if( m_pSelectUnit == NULL )
		return true;

	KEGS_DELETE_UNIT_REQ kPacket;
	kPacket.m_iUnitUID		= m_pSelectUnit->GetUID();

	g_pData->GetServerProtocol()->SendPacket( EGS_DELETE_UNIT_REQ, kPacket );
	g_pMain->AddServerPacket( EGS_DELETE_UNIT_ACK  );
	SAFE_DELETE_DIALOG( m_pDLGMsgBox );
	m_pDLGMsgBox = g_pMain->KTDGUIMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_712 ), this );

	return true;
}

bool CX2StateServerSelect::Handler_EGS_DELETE_UNIT_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_DELETE_UNIT_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	SAFE_DELETE_DIALOG( m_pDLGMsgBox );
	if( g_pMain->DeleteServerPacket( EGS_DELETE_UNIT_ACK ) == true )
	{
		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{			
			CX2Unit* pUnit = g_pData->GetMyUser()->GetUnitByUID( kEvent.m_iUnitUID );
			if ( pUnit == NULL )
			{
				g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_713 ), this );
				return true;
			}

			//{{ 2012.02.20 조효진	캐릭터 삭제 프로세스 변경 (삭제 대기 기간 도입)
#ifdef SERV_UNIT_WAIT_DELETE
			// 캐릭터 삭제 상태로 바꾸는 함수가 들어가야 함 
			//{{ //2012.03.06 lygan_조성욱 // 완전 삭제 가능한 날짜 알림용
			std::wstring buffer = L"";
			std::wstring year = L"";
			std::wstring month = L"";
			std::wstring day = L"";
			std::wstring hour = L"";
			std::wstring min = L"";

			CTime tDeleAbleDate(kEvent.m_tDelAbleDate);

			year = (CStringW)( tDeleAbleDate.Format(_T( "%Y" )) );
			month = (CStringW)( tDeleAbleDate.Format(_T( "%m" )) );
			day = (CStringW)( tDeleAbleDate.Format(_T( "%d" )) );
			hour = (CStringW)( tDeleAbleDate.Format(_T( "%H" )) );
			min = (CStringW)( tDeleAbleDate.Format(_T( "%M" )) );

			buffer =  GET_REPLACED_STRING( ( STR_ID_16103, "SSSSS", day.c_str(), month.c_str(), year.c_str(),hour.c_str(), min.c_str()));
			g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), buffer.c_str(), this );
			//}}
			//{{ 2012.03.05 lygan_조성욱 // 조효진이 작업 누락된부분 추가 작업
			ClearBeforeUnit( pUnit );
			ClearUnitButton();
			CX2Unit::UnitData* pUnitData = pUnit->GetUnitData();
			pUnitData->m_bDeleted = true;
			pUnitData->m_trDelAbleDate = kEvent.m_tDelAbleDate;
			CreateUnitButton();
			//}}
#else SERV_UNIT_WAIT_DELETE
			ClearBeforeUnit( pUnit );
			ClearUnitButton();
			g_pData->GetMyUser()->DeleteUnitByUID(kEvent.m_iUnitUID);
			g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_714 ), this );
			CreateUnitButton();
#endif SERV_UNIT_WAIT_DELETE
			//}}
		}
#ifdef SERV_UNIT_WAIT_DELETE
		else if( kEvent.m_iOK == NetError::ERR_DELETE_UNIT_02)
		{
			CX2Unit* pUnit = g_pData->GetMyUser()->GetUnitByUID( kEvent.m_iUnitUID );
			if ( pUnit == NULL )
			{
				g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_713 ), this );
				return true;
			}

			std::wstring buffer = L"";
			std::wstring year = L"";
			std::wstring month = L"";
			std::wstring day = L"";
			std::wstring hour = L"";
			std::wstring min = L"";

			CTime tReDeleAbleDate(kEvent.m_tReDelAbleDate);

			year = (CStringW)( tReDeleAbleDate.Format(_T( "%Y" )) );
			month = (CStringW)( tReDeleAbleDate.Format(_T( "%m" )) );
			day = (CStringW)( tReDeleAbleDate.Format(_T( "%d" )) );
			hour = (CStringW)( tReDeleAbleDate.Format(_T( "%H" )) );
			min = (CStringW)( tReDeleAbleDate.Format(_T( "%M" )) );

			CX2Unit::UnitData* pUnitData = pUnit->GetUnitData();
			pUnitData->m_trDelAbleDate = kEvent.m_tReDelAbleDate;

			buffer =  GET_REPLACED_STRING( ( STR_ID_16112, "SSSSS", day.c_str(), month.c_str(), year.c_str(),hour.c_str(), min.c_str()));
			g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), buffer.c_str(), this );
		}
#endif SERV_UNIT_WAIT_DELETE
	}
	return true;
}

//bool CX2StateUnitSelect::DeleteUnitTimeOutAck( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
//{
//SAFE_DELETE_DIALOG( m_pDLGMsgBox );
//g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), L"서버에 응답이 없습니다\n유닛을 지울 수 없습니다.", this );
//
//return true;
//}


bool CX2StateServerSelect::Handler_EGS_STATE_CHANGE_VILLAGE_MAP_REQ()
{
	if ( g_pMain->GetIsPlayingTutorial() == true )
	{
		SAFE_DELETE_DIALOG( m_pDLGMsgBox );
		m_pDLGMsgBox = g_pMain->KTDGUIMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_716 ), this );
		Handler_EGS_CREATE_TUTORIAL_ROOM_REQ();
	}
	else
	{
		SAFE_DELETE_DIALOG( m_pDLGMsgBox );
		m_pDLGMsgBox = g_pMain->KTDGUIMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_545 ), this );
		Handler_EGS_STATE_CHANGE_FIELD_REQ();
	}

	return true;
}

bool CX2StateServerSelect::Handler_EGS_STATE_CHANGE_VILLAGE_MAP_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//#if 0 
//	KSerBuffer* pBuff = (KSerBuffer*)lParam;
//	KStateChangeAck kEvent;
//	DeSerialize( pBuff, &kEvent );
//
//	SAFE_DELETE_DIALOG( m_pDLGMsgBox );
//	if( g_pMain->DeleteServerPacket( EGS_STATE_CHANGE_VILLAGE_MAP_ACK ) == true )
//	{
//		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
//		{
//			g_pMain->CreateStateChangeDLG( L"마을맵으로 이동중입니다." );
//			g_pKTDXApp->SendGameMessage( XGM_STATE_CHANGE, CX2Main::XS_VILLAGE_MAP, kEvent.m_nMapID, false );
//		}
//	}
//#endif


	return true;
}






bool CX2StateServerSelect::Handler_EGS_CHANGE_NICK_NAME_REQ( UidType uid, const WCHAR* wszNickName, bool bCheckOnly )
{
	if( NULL == wszNickName )
		return false;

	KEGS_CHANGE_NICK_NAME_REQ kPacket;
	kPacket.m_iUnitUID		= uid;
	kPacket.m_wstrNickName	= wszNickName;
	kPacket.m_bCheckOnly	= bCheckOnly;

	g_pData->GetServerProtocol()->SendPacket( EGS_CHANGE_NICK_NAME_REQ, kPacket );
	g_pMain->AddServerPacket( EGS_CHANGE_NICK_NAME_ACK );

	return true;
}


bool CX2StateServerSelect::Handler_EGS_CHANGE_NICK_NAME_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;	
	KEGS_CHANGE_NICK_NAME_ACK kEvent;
	DeSerialize( pBuff, &kEvent );


	SAFE_DELETE_DIALOG( m_pDLGMsgBox );
	if( g_pMain->DeleteServerPacket( EGS_CHANGE_NICK_NAME_ACK ) == true )
	{
		m_bJustChangedNickName = false;



		if( true == kEvent.m_bCheckOnly )
		{
			switch( kEvent.m_iOK )
			{
			case NetError::ERR_RESTORE_NICK_NAME_00:			
			case NetError::ERR_RESTORE_NICK_NAME_01:			
			case NetError::ERR_RESTORE_NICK_NAME_02:			
			case NetError::ERR_RESTORE_NICK_NAME_03:			
			case NetError::ERR_RESTORE_NICK_NAME_04:			
			case NetError::ERR_RESTORE_NICK_NAME_05:			
			case NetError::ERR_RESTORE_NICK_NAME_06:			
				{
					SetRenameMsgBoxError( NetError::GetErrStrF( kEvent.m_iOK ) );
					return true;
				} break;
			}


			if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
			{
				// 닉네임 변경 가능 확인 완료. 진짜 바꿀지 확인 팝업창 띄우기
				SAFE_DELETE_DIALOG( m_pDLGRenameConfirmMsgBox );
				m_pDLGRenameConfirmMsgBox = g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2(250,300), GET_REPLACED_STRING( ( STR_ID_2556, "S", m_NickNameChangeUnitNickName.c_str() ) ), SUSUCM_RENAME_CONFIRM_OK, this, SUSUCM_RENAME_CONFIRM_CANCEL );

				SetRenameMsgBoxError( L"" );
			}
		}
		else
		{
			if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
			{
				// 닉네임 변경 완료. 캐릭터 선택해서 게임입장.
				OpenRenameMsgBox( false );

				m_pDLGRenameSuccessMsgBox = g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_REPLACED_STRING( ( STR_ID_2557, "S", kEvent.m_wstrNickName.c_str() ) ), this, SUSUCM_RENAME_SUCCESS_OK );
			}
		}
	}

	return true;
}

//{{ kimhc // 2010-01-04 // 기존의 EGS_SELECT_UNIT_ACK에 인벤정보가 포함되어 오던 것 분할
#ifdef	PC_BANG_WORK
bool CX2StateServerSelect::Handler_EGS_SELECT_UNIT_INVENTORY_INFO_NOT( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_SELECT_UNIT_INVENTORY_INFO_NOT kEvent;
	DeSerialize( pBuff, &kEvent );

#ifdef SERV_IDENTITY_CONFIRM_POPUP_MESSAGE
	m_bIdentityConfirmCheck = false; // 2011.11.09 lygan_조성욱 // 캐릭터 선택했을 때 방침미 서버에서 인증 받은거 초기화 시키기
#endif //SERV_IDENTITY_CONFIRM_POPUP_MESSAGE

	if ( g_pData == NULL || g_pData->GetMyUser() == NULL )
	{
		ASSERT( !L"g_pData or g_pData->GetMyUser In EGS_SELECT_UNIT_INVENTORY_INFO_NOT is WRONG" );
		return false;
	}

	CX2Unit* pMyUnit = g_pData->GetMyUser()->GetSelectUnit();

	if ( pMyUnit != NULL && pMyUnit->GetInventory() != NULL )
	{
		pMyUnit->GetInventory()->ResetItems( kEvent.m_mapItem );
		return true;
	}
	if( g_pData != NULL )
	{
		g_pData->ResetPlayGuide();
	}
	ASSERT( !L"pMyUnit or GetInventory In EGS_SELECT_UNIT_INVENTORY_INFO_NOT is WRONG" );
	return false;	
}

bool CX2StateServerSelect::Handler_EGS_GET_MY_INVENTORY_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_GET_MY_INVENTORY_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	SAFE_DELETE_DIALOG( m_pDLGMsgBox );

	if( false == g_pMain->DeleteServerPacket( EGS_GET_MY_INVENTORY_ACK ) )
	{
		StateLog( L"Invalid EGS_GET_MY_INVENTORY_ACK" );
		return false;
	}

	if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
	{
#ifdef SERV_RECOMMEND_LIST_EVENT
		g_pData->Set_ShowRecommendUI( !kEvent.m_bIsRecommend );
#endif //SERV_RECOMMEND_LIST_EVENT		
		// 체험 아이디 제한 
		if( false == g_pData->GetMyUser()->GetUserData()->m_bIsGuestUser &&
			kEvent.m_bIsRecommend == false && m_bCheckRecommend == true )
		{
#ifdef SERV_NO_GAMEINVITE
			Handler_EGS_RECOMMEND_USER_REQ( L"", true );
#else SERV_NO_GAMEINVITE
			OpenRecommendMsgBox( true );
#endif SERV_NO_GAMEINVITE
		}
		else
		{
			if ( g_pMain->GetIsPlayingTutorial() == true )
			{
				// 튜토리얼 진입요청
				Handler_EGS_CREATE_TUTORIAL_ROOM_REQ();
			}
			else
			{
				CX2Unit::UnitData* pUnitData = 	g_pData->GetMyUser()->GetSelectUnit()->GetUnitData();

				if ( SEnum::VMI_BATTLE_FIELD_RUBEN_FIELD_01 <= pUnitData->m_nMapID && 
					 SEnum::VMI_BATTLE_FIELD_END > pUnitData->m_nMapID )
				{
					CX2GageManager* pGageManager = CX2GageManager::GetInstance();
					if ( pGageManager->GetMyGageData()->GetNowHp() > 0.0f )
						Handler_EGS_JOIN_BATTLE_FIELD_REQ( pUnitData->m_nMapID, pUnitData->m_ucLastTouchLineIndex, pUnitData->m_usLastPosValue  );
					else
					{
						CX2BattleFieldManager& battleFieldManager = g_pData->GetBattleFieldManager();
						CX2State::Handler_EGS_STATE_CHANGE_FIELD_REQ( battleFieldManager.GetReturnVillageId( pUnitData->m_nMapID ) );	// 새마을로 진입요청
					}
				}
				else
					Handler_EGS_STATE_CHANGE_FIELD_REQ();	// 새마을로 진입요청
			}
		}

		//{{ kimhc // GiveUpForbiddenQuest()를 EGS_SELECT_UNIT_ACK에서 이동함
	#ifdef	SERV_EPIC_QUEST
		g_pData->GetQuestManager()->GiveUpForbiddenQuest();
	#endif	SERV_EPIC_QUEST
		//}} kimhc // GiveUpForbiddenQuest()를 EGS_SELECT_UNIT_ACK에서 이동함

		return true;
	}

	return false;

}
#endif	PC_BANG_WORK
//}} kimhc // 2010-01-04 // 기존의 EGS_SELECT_UNIT_ACK에 인벤정보가 포함되어 오던 것 분할


void CX2StateServerSelect::SetRenameMsgBoxError( const wstring& wstrErrorMessage )
{
	if( NULL == m_pDLGRenameMsgBox )
		return;


	CKTDGUIStatic* pStaticErrorMessage = (CKTDGUIStatic*) m_pDLGRenameMsgBox->GetControl( L"NickNameErrorMessage" );
	if( NULL == pStaticErrorMessage )
		return;


	if( NULL == pStaticErrorMessage->GetString(0) )
		return;

	
	pStaticErrorMessage->GetString(0)->msg = wstrErrorMessage;
}


void CX2StateServerSelect::OpenRenameMsgBox( bool bOpen )
{
	if( true == bOpen )
	{
		if( NULL == m_pDLGRenameMsgBox )
		{
			m_pDLGRenameMsgBox = new CKTDGUIDialog( this, L"DLG_Change_Nickname.lua" );
			g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGRenameMsgBox );
		}

		m_pDLGRenameMsgBox->SetShowEnable( true, true );
	}
	else
	{
		if( NULL != m_pDLGRenameMsgBox )
		{
			m_pDLGRenameMsgBox->SetShowEnable( false, false );
		}
	}
}


bool CX2StateServerSelect::Handler_EGS_RECOMMEND_USER_REQ( const WCHAR* wszNickName, bool bCancel )
{
	KEGS_RECOMMEND_USER_REQ kPacket;
	kPacket.m_wstrNickName = wszNickName;
	kPacket.m_wstrNickName = kPacket.m_wstrNickName.substr( 0, 20 );
	kPacket.m_bCancel = bCancel;

	if ( kPacket.m_bCancel == false && g_pMain->GetStringFilter()->CheckIsValidString( CX2StringFilter::FT_NICKNAME, kPacket.m_wstrNickName.c_str() ) == false )
	{
		g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_717 ), this );
		return true;
	}

	g_pData->GetServerProtocol()->SendPacket( EGS_RECOMMEND_USER_REQ, kPacket );
	g_pMain->AddServerPacket( EGS_RECOMMEND_USER_ACK );

	return true;
}

bool CX2StateServerSelect::Handler_EGS_RECOMMEND_USER_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;	
	KEGS_RECOMMEND_USER_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	if( g_pMain->DeleteServerPacket( EGS_RECOMMEND_USER_ACK ) == true )
	{
		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{
#ifdef SERV_RECOMMEND_LIST_EVENT
			g_pData->Set_ShowRecommendUI( false );
#endif //SERV_RECOMMEND_LIST_EVENT	
			OpenRecommendMsgBox( false );
			g_pData->GetMyUser()->GetSelectUnit()->SetResurrectionStoneNum( g_pData->GetMyUser()->GetSelectUnit()->GetResurrectionStoneNum() + kEvent.m_iRewardResurrectionStone );
			if ( kEvent.m_iRewardResurrectionStone > 0 )
			{
				m_bCheckRecommendSuccess = true;
				wstringstream wstrstm;
				wstrstm << GET_REPLACED_STRING( ( STR_ID_718, "i", kEvent.m_iRewardResurrectionStone ) );
				m_pDLGRecommendSuccess = g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), wstrstm.str().c_str(), this );
			}
			else
			{
				Handler_EGS_STATE_CHANGE_VILLAGE_MAP_REQ();
				//Handler_EGS_GET_POST_LETTER_LIST_REQ();
			}

			return true;
		}
	}

	return false;
}

void CX2StateServerSelect::OpenRecommendMsgBox( bool bOpen )
{
	if ( bOpen == true )
	{
		if ( m_pDLGRecommend != NULL )
			g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGRecommend, NULL, false );

		m_pDLGRecommend = NULL;

#ifdef SERV_RECOMMEND_LIST_EVENT
		m_pDLGRecommend = new CKTDGUIDialog( this, L"DLG_Recommend2.lua" );
#else
		m_pDLGRecommend = new CKTDGUIDialog( this, L"DLG_Recommend.lua" );
#endif //SERV_RECOMMEND_LIST_EVENT

		g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGRecommend );

		CKTDGUIIMEEditBox* pIMEEditBox = (CKTDGUIIMEEditBox*)m_pDLGRecommend->GetControl( L"g_pIMEEditBoxName" );
		if ( pIMEEditBox != NULL )
		{
			pIMEEditBox->RequestFocus();
		}
	}
	else
	{
		if ( m_pDLGRecommend != NULL )
			g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGRecommend, NULL, false );

		m_pDLGRecommend = NULL;
	}
}





bool CX2StateServerSelect::Handler_EGS_SELECT_SERVER_SET_REQ()
{
	if ( m_sbSelectedServerSet == true )
		return true;

	KEGS_SELECT_SERVER_SET_REQ kPacket;
	//kPacket.m_ServerSetID = iServerSetIndex;

	g_pData->GetServerProtocol()->SendPacket( EGS_SELECT_SERVER_SET_REQ, kPacket );
	g_pMain->AddServerPacket( EGS_SELECT_SERVER_SET_ACK );

	return true;	
}

bool CX2StateServerSelect::Handler_EGS_SELECT_SERVER_SET_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;	
	KEGS_SELECT_SERVER_SET_ACK kEvent;
	DeSerialize( pBuff, &kEvent );
	
	
	if( g_pMain->DeleteServerPacket( EGS_SELECT_SERVER_SET_ACK ) == true )
	{
		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )	
		{
			m_sbSelectedServerSet = true;
			Handler_EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ();
		}
		else
		{
			SAFE_DELETE_DIALOG( m_pDLGMsgBox );

			switch( g_pData->GetMyUser()->GetAuthLevel() )
			{
			case -2:
			case -3:
				//{{ 2010. 06. 30  최육사	계정블럭추가
			case -5:
			case -6:
				//}}
				//{{ 계정 제재 일수 상태 추가 - 김민성
//#ifdef SERV_ADD_BLOCK_DAY
			case -7:
			case -8:
			case -9:
//#endif SERV_ADD_BLOCK_DAY
				//}}
				{
                    wstring wstrMsg = GET_REPLACED_STRING( ( STR_ID_719, "L", kEvent.m_kAccountBlockInfo.m_wstrEndTime ) );
					wstrMsg += kEvent.m_kAccountBlockInfo.m_wstrBlockReason;
					g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,150), wstrMsg.c_str(), (CKTDXStage*)this );
				} break;

			case -4:
				{
					g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,150), kEvent.m_kAccountBlockInfo.m_wstrBlockReason.c_str(), (CKTDXStage*)this );
				} break;
			default:
#ifdef SERV_HACKING_USER_CHECK_COUNT// 작업날짜: 2013-06-04	// 박세훈
				if( ( SEnum::HUCT_NONE < kEvent.m_kAccountBlockInfo.m_byteBlockType ) &&
					( kEvent.m_kAccountBlockInfo.m_byteBlockType < SEnum::HUCT_MAX )
					)
				{
					if( ( SEnum::HUCT_NONE < kEvent.m_kAccountBlockInfo.m_byteBlockType ) &&
						( kEvent.m_kAccountBlockInfo.m_byteBlockType < SEnum::HUCT_MAX )
						)
					{
						wstring wstrMsg;
						if( kEvent.m_kAccountBlockInfo.m_wstrBlockEndDate.empty() == false )
						{
							wstrMsg = GET_REPLACED_STRING( ( STR_ID_719, "L", kEvent.m_kAccountBlockInfo.m_wstrBlockEndDate ) );

							if( kEvent.m_kAccountBlockInfo.m_wstrBlockReason2.empty() == false )
							{
								wstrMsg += L"\n";
								wstrMsg += kEvent.m_kAccountBlockInfo.m_wstrBlockReason2;
							}

							g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,150), wstrMsg.c_str(), (CKTDXStage*)this );
						}
						else if( kEvent.m_kAccountBlockInfo.m_wstrBlockReason2.empty() == false )
						{
							wstrMsg = kEvent.m_kAccountBlockInfo.m_wstrBlockReason2;
							g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,150), wstrMsg.c_str(), (CKTDXStage*)this );
						}
					}
				}
#endif // SERV_HACKING_USER_CHECK_COUNT
				break;
			}
#ifdef CLIENT_COUNTRY_TWHK
			// 올바른 빈펀 계정이 아니었다는 NetError인지 여기서 검사해서 클라를 강제종료시킨다
#endif CLIENT_COUNTRY_TWHK
		}
	}

	// 서버선택 및 채널 선택시 서버/채널선택창은 사라지고 유닛선택창이 나온다.
#ifdef SERVER_GROUP_UI_ADVANCED
	if(m_pDLGServerGroupSelectBack != NULL)
		m_pDLGServerGroupSelectBack->SetShowEnable(false, false);
#endif SERVER_GROUP_UI_ADVANCED
	m_pDLGServerSelectBack->SetShowEnable(false, false);
	m_pDLGUnitSelectFront->SetShowEnable(true, true);

	//{{ kimhc // 2009-12-15 // 서버군 추가 작업에 따른 채널 다이얼로그 분리
#ifdef	ADD_SERVER_GROUP
	m_pDLGChannelSelectBack->SetShowEnable( false, false );
#endif	ADD_SERVER_GROUP
	//}}  kimhc // 2009-12-15 // 서버군 추가 작업에 따른 채널 다이얼로그 분리

	// 채널번호 표시
	if( g_pInstanceData->GetConnectChannelServerID() > 0 )
	{
		CKTDGUIStatic* pStatic = (CKTDGUIStatic*)m_pDLGUnitSelectFront->GetControl( L"Channel_Name" );
		pStatic->GetString(0)->msg = g_pInstanceData->GetConnectChannelServerName();
	}
	

	return true;
}

bool CX2StateServerSelect::Handler_EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ()
{
	/*
	KEVENT_X2_SERVER_UNIT_INFO_LIST_REQ	kEVENT_X2_SERVER_UNIT_INFO_LIST_REQ;
	kEVENT_X2_SERVER_UNIT_INFO_LIST_REQ.userUID = g_pData->GetMyUser()->GetUID();
*/

#ifdef SERV_CLIENT_DIRECT_CONNECT_AUTH_PCBANG
	if ( g_pMain != NULL )
	{
		bool bCheckPcBang = false;
		bCheckPcBang = g_pMain->IsStartPcBang();

	}
#endif //SERV_CLIENT_DIRECT_CONNECT_AUTH_PCBANG

	g_pData->GetServerProtocol()->SendID( EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ );
	g_pMain->AddServerPacket( EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK );

	SAFE_DELETE_DIALOG( m_pDLGMsgBox );
	m_pDLGMsgBox = g_pMain->KTDGUIMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_720 ), this );
	return true;
}

bool CX2StateServerSelect::Handler_EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK kEvent;
	DeSerialize( pBuff, &kEvent );
	
	
	if( g_pMain->DeleteServerPacket( EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK ) == true )
	{
		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{
			ClearBeforeUnit( m_pSelectUnit );
			ClearUnitButton();

			g_pData->GetMyUser()->DeleteAllUnit();

			g_pData->GetMyUser()->GetUserData()->maxUnitCount = kEvent.m_nUnitSlot; //생성가능 유닛 슬롯?

			m_MaxUnitNum	= kEvent.m_nUnitSlot;

#ifdef SERV_SHARING_BANK_QUEST_CASH
			// 지헌 : 은행 공유 열었다고 클라에 저장하자.
			g_pData->GetMyUser()->SetSharingBankOpen(kEvent.m_bSharingBank);
			// 은행 퀘스트를 진행중이던 캐릭터라면 포기 시키자
			// 			if( NULL != g_pData->GetQuestManager() && g_pData->GetMyUser()->IsSharingBankOpen() == true )
			// 			{
			// 				g_pData->GetQuestManager()->GiveUpForbiddenQuest(); 
			// 			}
#endif // SERV_SHARING_BANK_QUEST_CASH

            for( int i =0; i < (int)kEvent.m_vecUnitInfo.size(); i++ )
			{	 

				KUnitInfo kUnitInfo = kEvent.m_vecUnitInfo[i];
				CX2Unit* pUnit = new CX2Unit( kUnitInfo );

				pUnit->SetOwnerUserUID( g_pData->GetMyUser()->GetUID() );
				wstring nickNameToChange = L"__DELETED__";
				if ( nickNameToChange == pUnit->GetUnitData()->m_NickName )
				{
					wstringstream tempNickName;
					tempNickName << nickNameToChange.c_str() << i;
					pUnit->GetUnitData()->m_NickName = tempNickName.str().c_str();
				}
				g_pData->GetMyUser()->AddUnit( pUnit );
			}

			m_NowPage		= 1;
			const int SLOT_COUNT_FOR_CREATE_UNIT = 1;
			if( g_pData->GetMyUser()->GetUnitNum() < m_MaxUnitNum )
			{
				m_MaxPage		= (g_pData->GetMyUser()->GetUnitNum()+SLOT_COUNT_FOR_CREATE_UNIT+UNIT_SELECT_UI_MAX_UNIT_NUM-1) / UNIT_SELECT_UI_MAX_UNIT_NUM;
			}
			else
			{
				m_MaxPage		= (g_pData->GetMyUser()->GetUnitNum()+UNIT_SELECT_UI_MAX_UNIT_NUM-1) / UNIT_SELECT_UI_MAX_UNIT_NUM;
			}



			
			CreateUnitButton();

			m_bReceivedEGS_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK = true;

#ifdef SERV_EVENT_RETURN_USER_MARK
			if( true == kEvent.m_bEventMark )
			{
				g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING(  STR_ID_20325 ), g_pMain->GetNowState() );
			}			
#endif //SERV_EVENT_RETURN_USER_MARK

#ifdef SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE
			if( NULL != g_pMain && NULL != g_pMain->GetSecurityPad() 
				&& false == g_pMain->GetSecurityPad()->GetUseSecondPW() )
			{
				Handler_EGS_CHECK_THE_ARCHUANGEL_S_COIN_EVENT_LETTER_REQ();
			}
#endif //SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE

#ifdef SERV_ELIOS_INVESTIGATIONS
			if( false == kEvent.m_bEliosInvestigationsReward )
			{
				g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_20325 ), g_pMain->GetNowState() );
			}
#endif SERV_ELIOS_INVESTIGATIONS

#ifdef SERV_CLIENT_DIRECT_CONNECT_AUTH_PCBANG

#endif //CLIENT_DIRECT_CONNECT_AUTH_PCBANG

		}
		else
		{
			g_pData->DeleteServerProtocol();
			g_pData->DeleteMyUser();
		}
	}

	SAFE_DELETE_DIALOG( m_pDLGMsgBox );
	return true;
}




bool CX2StateServerSelect::Handler_EGS_AGREE_HACK_USER_REQ()
{
	g_pData->GetServerProtocol()->SendID( EGS_AGREE_HACK_USER_REQ );
	g_pMain->AddServerPacket( EGS_AGREE_HACK_USER_ACK, 500.0f );

	return true;
}

bool CX2StateServerSelect::Handler_EGS_AGREE_HACK_USER_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_AGREE_HACK_USER_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	
	if( g_pMain->DeleteServerPacket( EGS_AGREE_HACK_USER_ACK ) == true )
	{
		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{
#ifndef PROCESSLIST
			g_pMain->UpdateProcessList();
#endif
			g_pData->GetMyUser()->GetUserData()->hackingUserType = (CX2User::HACKING_USER_TYPE)kEvent.m_cHackingUserType;
			SAFE_DELETE_DIALOG( m_pDLGCheckHack );
		}
		else
		{
			g_pKTDXApp->SendGameMessage( XGM_QUIT_GAME, 0, 0, false );
		}
	}
	return true;
}
//{{ kimhc // 2009-12-15 // 이전에 플레이했던 채널 서버군
#ifdef	ADD_SERVER_GROUP

void CX2StateServerSelect::UpdateChannelListUI()
{
#ifdef SHOW_ONLY_ADVERTISEMENT
	if( true == m_bHideServerSelectUI )
		return;
#endif //SHOW_ONLY_ADVERTISEMENT

	SAFE_DELETE_DIALOG( m_pDLGChannelSelectBack );
		
#ifdef SERV_CHANNEL_LIST_RENEWAL
	int ServerGroupNum = g_pInstanceData->GetServerGroupID();
	m_pDLGChannelSelectBack = new CKTDGUIDialog( this, L"DLG_UI_Channel_Selection.lua" );

	if(m_pDLGChannelSelectBack == NULL)
		m_pDLGChannelSelectBack =  new CKTDGUIDialog( this, L"DLG_UI_Channel_Selection_Auto.lua" );

#else //SERV_CHANNEL_LIST_RENEWAL

	switch ( g_pInstanceData->GetServerGroupID() )
	{
	case SGI_GAIA:
		{
			m_pDLGChannelSelectBack = new CKTDGUIDialog( this, L"DLG_UI_Channel_Selection02.lua" );
		}
		break;

	case SGI_SOLES:
	default:
		{
			m_pDLGChannelSelectBack = new CKTDGUIDialog( this, L"DLG_UI_Channel_Selection01.lua" ); //new CKTDGUIDialog( this, L"DLG_UI_Channel_Selection01.lua" );
		}
		break;
	}
#endif //SERV_CHANNEL_LIST_RENEWAL

	if ( m_pDLGChannelSelectBack == NULL )
		return;
	
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGChannelSelectBack );

	// 여기서 부터 채널 리스트 셋팅

	const int MAX_CHANNEL_COUNT = m_iChannelNum;	// 채널 갯수

	// 채널 리스트 버튼 초기화
	WCHAR wszControlName[32] = L"";

	// 채널 혼잡도 게이지
	CKTDGUIStatic* pStaticCHCongestionGage	= static_cast< CKTDGUIStatic* >( m_pDLGChannelSelectBack->GetControl( L"chanal_line" ) );		
	// 채널 회색 박스
	CKTDGUIStatic* pStaticCHBox				= static_cast< CKTDGUIStatic* >( m_pDLGChannelSelectBack->GetControl( L"chanal_state" ) );		
	// 채널이름 ex)루벤1
	CKTDGUIStatic* pStaticCHName			= static_cast< CKTDGUIStatic* >( m_pDLGChannelSelectBack->GetControl( L"chanal_num" ) );		
	
#ifdef SERV_CHANNEL_LIST_RENEWAL
	CKTDGUIStatic* pStaticCHBar			= static_cast< CKTDGUIStatic* >( m_pDLGChannelSelectBack->GetControl( L"chanal_bar" ) );	
#endif
	
	// 전체 채널 다 보이지 않도록 초기화
	for ( int i = 0; i < MAX_CHANNEL_COUNT; ++i )
	{
		StringCchPrintf( wszControlName, ARRAY_SIZE(wszControlName), L"chanal%d", i + 1 );
		CKTDGUIButton *pChannelButton = static_cast< CKTDGUIButton* >( m_pDLGChannelSelectBack->GetControl( wszControlName ) );
		
		if( pChannelButton != NULL )
			pChannelButton->SetShowEnable(false, false);

		// 채널 혼잡도 스트링 ex) 보통, 원활
		StringCchPrintf( wszControlName, ARRAY_SIZE(wszControlName), L"statechanal%d", i + 1);
		CKTDGUIStatic* pStaticCHCongestionStr = static_cast< CKTDGUIStatic* >( m_pDLGChannelSelectBack->GetControl( wszControlName ) );
		
		if(pStaticCHCongestionStr != NULL)
			pStaticCHCongestionStr->SetShowEnable(false, false);


#ifdef MODIFY_CHANNEL_MODE_RATE_INT
		const int	iCntChannelGageMode = 4;
		for( int j=0; j< iCntChannelGageMode; ++j )
		{
            if ( pStaticCHCongestionGage != NULL &&
                pStaticCHCongestionGage->GetPicture( (i * iCntChannelGageMode) + j ) != NULL )
			    pStaticCHCongestionGage->GetPicture( (i * iCntChannelGageMode) + j )->SetShow(false);
		}
#else MODIFY_CHANNEL_MODE_RATE_INT
		pStaticCHCongestionGage->GetPicture( i * 3 )->SetShow(false);
		pStaticCHCongestionGage->GetPicture( i * 3 + 1 )->SetShow(false);
		pStaticCHCongestionGage->GetPicture( i * 3 + 2 )->SetShow(false);
#endif MODIFY_CHANNEL_MODE_RATE_INT

		pStaticCHBox->GetPicture( i * 3 )->SetShow(false);
		pStaticCHBox->GetPicture( i * 3 + 1 )->SetShow(false);
		pStaticCHBox->GetPicture( i * 3 + 2 )->SetShow(false);

#ifndef SERV_CHANNEL_LIST_RENEWAL
		pStaticCHName->GetPicture( i )->SetShow(false);
#else
		pStaticCHBar->GetPicture( i + 1 )->SetShow(false);
#endif
	}				

	if( false == m_bReceivedChannelList )	// 채널리스트를 받지 않았으면 리턴
		return;

#ifdef CHANGE_CHANNEL
	if( g_pInstanceData == NULL || g_pInstanceData->m_vecChannelInfo.empty() == true)
		return;
#else
	if( true == m_vecChannelInfo.empty() )
		return;
#endif

#ifdef MODIFY_CHANNEL_MODE_RATE_INT

	float			CHANNEL_CONGESTION_SCOPE_TEMP[] = { 0.1f,	0.2f,	0.5f,	0.99f, };
	const float		CHANNEL_CONGESTION_DISPLAY[]	= { 0.25f,	0.5f,	0.5f,	0.75f,	};

#else // MODIFY_CHANNEL_MODE_RATE_INT

	// 채널 게이지 표기법 변경
	float			CHANNEL_CONGESTION_SCOPE_TEMP[] = { 0.1f,	0.2f,	0.3f,	0.999f, };
#ifndef CHANNEL_CONGESTION_SCOPE_NO_CHEAT
	const float		CHANNEL_CONGESTION_SCOPE[]		= { 0.1f,	0.2f,	0.3f,	0.999f, };//{ 0.2f,	0.4f,	0.6f,	0.999f, };
#endif // CHANNEL_CONGESTION_SCOPE_NO_CHEAT
	const float		CHANNEL_CONGESTION_DISPLAY[]	= { 0.25f,	0.5f,	0.75f,	0.99f,	};

#endif // MODIFY_CHANNEL_MODE_RATE_INT	

	// 채널 리스트 정보 갱신
#ifdef CHANGE_CHANNEL
	for( int i = 0; i < static_cast< int >( g_pInstanceData->m_vecChannelInfo.size() )&& i < MAX_CHANNEL_COUNT; i++ )
#else
	for( int i = 0; i < static_cast< int >( m_vecChannelInfo.size() ) && i < MAX_CHANNEL_COUNT; i++ )
#endif
	{	
#ifdef CHANNEL_DOWN_EMERGENCY
		// oasis907 : 김상윤 [2010.10.28] // 특정채널 임시로 빼기
		if(i == 10 && g_pInstanceData->GetServerGroupID() == SGI_GAIA)
		{
			continue;
		}
#endif CHANNEL_DOWN_EMERGENCY


		StringCchPrintf( wszControlName, ARRAY_SIZE(wszControlName), L"chanal%d", i + 1 );
		CKTDGUIButton *pChannelButton = static_cast< CKTDGUIButton* >( m_pDLGChannelSelectBack->GetControl( wszControlName ) );

		// 채널 혼잡도 스트링 ex) 보통, 원활
		StringCchPrintf( wszControlName, ARRAY_SIZE(wszControlName), L"statechanal%d", i + 1);
		CKTDGUIStatic* pStaticCHCongestionStr = static_cast< CKTDGUIStatic* >( m_pDLGChannelSelectBack->GetControl( wszControlName ) );

#ifndef CHANNEL_CONGESTION_SCOPE_NO_CHEAT
		g_pInstanceData->SetChannelDistributionByServerGroup( CHANNEL_CONGESTION_SCOPE_TEMP, CHANNEL_CONGESTION_SCOPE, i );
#endif // CHANNEL_CONGESTION_SCOPE_NO_CHEAT
		// Show 설정
		pChannelButton->SetShowEnable(true, true);
		pStaticCHCongestionStr->SetShowEnable(true, true);
		pStaticCHBox->GetPicture( i * 3 )->SetShow(true);
		pStaticCHBox->GetPicture( i * 3 + 1 )->SetShow(true);
		pStaticCHBox->GetPicture( i * 3 + 2 )->SetShow(true);

#ifndef SERV_CHANNEL_LIST_RENEWAL
		pStaticCHName->GetPicture( i )->SetShow(true);
#else
		pStaticCHBar->GetPicture( i + 1 )->SetShow(true);
#endif

#ifdef CHANGE_CHANNEL
		const KChannelInfo& channelInfo = g_pInstanceData->m_vecChannelInfo[i];
#else
		const KChannelInfo& channelInfo = m_vecChannelInfo[i];
#endif
		float fChannelCongestionRate = static_cast< float >( channelInfo.m_iCurrentUser ) / static_cast< float >( channelInfo.m_iMaxUser );
		float fChannelCongestionRateDisplay = 0.f;

#ifdef MODIFY_CHANNEL_MODE_RATE_INT

		if(fChannelCongestionRate >= CHANNEL_CONGESTION_SCOPE_TEMP[3])	// 99%~100%
		{
			// FULL 빨간색 , 게이지 길이 100%
			pStaticCHCongestionGage->GetPicture( i * 4 + 3 )->SetSizeX(pStaticCHCongestionGage->GetPicture(i*4+3)->GetOriginalSize().x );
			pStaticCHCongestionGage->GetPicture( i * 4 + 3 )->SetShow( true );
			pStaticCHCongestionStr->GetString( 0 )->msg = GET_STRING( STR_ID_724 );

			pChannelButton->SetShowEnable(true, false);
		}
		else if(fChannelCongestionRate >= CHANNEL_CONGESTION_SCOPE_TEMP[2]) // 50%~98%
		{
			// 혼잡 주황색 , 게이지 길이 75%
			fChannelCongestionRateDisplay = CHANNEL_CONGESTION_DISPLAY[3];
			pStaticCHCongestionGage->GetPicture( i * 4 + 2 )->SetSizeX(pStaticCHCongestionGage->GetPicture(i*4+2)->GetOriginalSize().x * fChannelCongestionRateDisplay );
			pStaticCHCongestionGage->GetPicture( i * 4 + 2 )->SetShow( true );	
			pStaticCHCongestionStr->GetString( 0 )->msg = GET_STRING( STR_ID_723 );
		}
		else if(fChannelCongestionRate >= CHANNEL_CONGESTION_SCOPE_TEMP[1]) // 20%~49%
		{
			// 보통 녹색 , 게이지 길이 50%
			fChannelCongestionRateDisplay = CHANNEL_CONGESTION_DISPLAY[2];
			pStaticCHCongestionGage->GetPicture( i * 4 + 1 )->SetSizeX(pStaticCHCongestionGage->GetPicture(i*4+1)->GetOriginalSize().x * fChannelCongestionRateDisplay );
			pStaticCHCongestionGage->GetPicture( i * 4 + 1)->SetShow( true );
			pStaticCHCongestionStr->GetString( 0 )->msg = GET_STRING( STR_ID_722 );
		}
		else	//	0%~19%
		{
			// 원활 파란색 , 게이지 길이 25% ( 비율 10%까지 ) , 50%
			if( fChannelCongestionRate >= CHANNEL_CONGESTION_SCOPE_TEMP[0] )	// 10%~19%
				fChannelCongestionRateDisplay = CHANNEL_CONGESTION_DISPLAY[1];
			else																// 0%~9%
				fChannelCongestionRateDisplay = CHANNEL_CONGESTION_DISPLAY[0];
			pStaticCHCongestionGage->GetPicture( i * 4 )->SetSizeX(pStaticCHCongestionGage->GetPicture(i*4)->GetOriginalSize().x * fChannelCongestionRateDisplay );
			pStaticCHCongestionGage->GetPicture( i * 4 )->SetShow( true );				
			pStaticCHCongestionStr->GetString( 0 )->msg = GET_STRING( STR_ID_721 );
		}
#else // MODIFY_CHANNEL_MODE_RATE_INT

		if( fChannelCongestionRate > CHANNEL_CONGESTION_SCOPE_TEMP[3] )
		{
			fChannelCongestionRateDisplay = 1.f;
		}
		else if( fChannelCongestionRate > CHANNEL_CONGESTION_SCOPE_TEMP[2] )
		{
			fChannelCongestionRateDisplay = CHANNEL_CONGESTION_DISPLAY[3]; // 혼잡
		}
		else if( fChannelCongestionRate > CHANNEL_CONGESTION_SCOPE_TEMP[1] )
		{
			fChannelCongestionRateDisplay = CHANNEL_CONGESTION_DISPLAY[2]; // 보통
		}
		else if( fChannelCongestionRate > CHANNEL_CONGESTION_SCOPE_TEMP[0] )
		{
			fChannelCongestionRateDisplay = CHANNEL_CONGESTION_DISPLAY[1]; // 원활
		}
		else
		{
			fChannelCongestionRateDisplay = CHANNEL_CONGESTION_DISPLAY[0]; // 원활
		}

		if(fChannelCongestionRateDisplay <= CHANNEL_CONGESTION_DISPLAY[1])
		{
			pStaticCHCongestionGage->GetPicture( i * 3 )->SetSizeX(pStaticCHCongestionGage->GetPicture(i*3)->GetOriginalSize().x * fChannelCongestionRateDisplay );
			pStaticCHCongestionGage->GetPicture( i * 3 )->SetShow( true );				
			pStaticCHCongestionStr->GetString( 0 )->msg = GET_STRING( STR_ID_721 );
		}
		else if(fChannelCongestionRateDisplay <= CHANNEL_CONGESTION_DISPLAY[2])
		{
			pStaticCHCongestionGage->GetPicture( i * 3 + 1 )->SetSizeX(pStaticCHCongestionGage->GetPicture(i*3+1)->GetOriginalSize().x * fChannelCongestionRateDisplay );
			pStaticCHCongestionGage->GetPicture( i * 3 + 1)->SetShow( true );
			pStaticCHCongestionStr->GetString( 0 )->msg = GET_STRING( STR_ID_722 );
		}
		else if(fChannelCongestionRateDisplay <= CHANNEL_CONGESTION_DISPLAY[3])
		{
			pStaticCHCongestionGage->GetPicture( i * 3 + 2 )->SetSizeX(pStaticCHCongestionGage->GetPicture(i*3+2)->GetOriginalSize().x * fChannelCongestionRateDisplay );
			pStaticCHCongestionGage->GetPicture( i * 3 + 2 )->SetShow( true );
			pStaticCHCongestionStr->GetString( 0 )->msg = GET_STRING( STR_ID_723 );
		}
		else
		{
			pStaticCHCongestionGage->GetPicture( i * 3 + 2 )->SetSizeX(pStaticCHCongestionGage->GetPicture(i*3+2)->GetOriginalSize().x );
			pStaticCHCongestionGage->GetPicture( i * 3 + 2 )->SetShow( true );
			pStaticCHCongestionStr->GetString( 0 )->msg = GET_STRING( STR_ID_724 );

			pChannelButton->SetShowEnable(true, false);
		}

#endif // MODIFY_CHANNEL_MODE_RATE_INT

	}


#ifdef SERV_CHANNEL_LIST_RENEWAL
	for(int j = 0; j < static_cast< int >( g_pInstanceData->m_vecChannelInfo.size() ) && j < MAX_CHANNEL_COUNT; ++j)
	{
		wstring wstrtempChannelName = L"";
		wstring wstrtempFieldName = L"";
		wstring wstrtempFieldNameNum = L"";

		int iFieldNum = 0;

		wstrtempChannelName = g_pInstanceData->GetChannelServerName(g_pInstanceData->m_vecChannelInfo[j].m_iChannelID);

		int iCharNum = 0;
		ASSERT(wstrtempChannelName.empty() == false);

		if(wstrtempChannelName.empty() == false)
		{
			for(int k = 0; k < (int)wstrtempChannelName.size(); k++ )
			{
				WCHAR tempChar = wstrtempChannelName[k];
				if ( tempChar < '0' || tempChar > '9' )
					iCharNum++;
			}
			wstrtempFieldName = wstrtempChannelName.substr(0,iCharNum);

			iFieldNum = _wtoi(wstrtempChannelName.substr(iCharNum, (wstrtempChannelName.size()-iCharNum)).c_str());

			if(iFieldNum < 10)
			{
				wstrtempFieldNameNum = L"0";
			}
			wstrtempFieldNameNum += wstrtempChannelName.substr(iCharNum, (wstrtempChannelName.size()-iCharNum));

			CKTDGUIControl::CPictureData* pPicture = NULL;
			pPicture = new CKTDGUIControl::CPictureData();
			CKTDGUIControl::UIPointData* pPointData = new CKTDGUIControl::UIPointData();

			if(pStaticCHName == NULL && pPicture == NULL && pPointData == NULL)
				return;

			int iChannelNameOffset = 0; // 각 국가별로 채널 UI 사이즈가 틀리기 때문에 조절 필요하다
			
#ifdef CHANNEL_UI_JAPAN
			if(wstrtempFieldName == GET_STRING( STR_ID_15886 ))
			{
				wstring wstrtempField = L"FIRSTSTEP";
				wstrtempField += wstrtempFieldNameNum;
				pPicture->SetTex(L"DLG_UI_Title02_A.TGA", wstrtempField.c_str());
#ifdef CHANGE_TGA_JAPAN
				//iChannelNameOffset = 7;
#else // CHANGE_TGA_JAPAN
				iChannelNameOffset = 8;
#endif // CHANGE_TGA_JAPAN
			}
			else
#endif // CHANNEL_UI_JAPAN
			if(wstrtempFieldName == GET_STRING( STR_ID_17731 ))
			{
				wstring wstrtempField = L"RUBEN"; //2011.11.22 lygan_조성욱 //DLG_UI_Title02_A.tet 에 있는 이름을 사용한다
				wstrtempField += wstrtempFieldNameNum;	
#ifdef CHANNEL_UI_EU
				pPicture->SetTex(L"DLG_UI_Title02_B.TGA", wstrtempField.c_str());
#else //CHANNEL_UI_EU
				pPicture->SetTex(L"DLG_UI_Title02_A.TGA", wstrtempField.c_str());
#endif //CHANNEL_UI_EU
#ifdef CHANGE_TGA_JAPAN
				//iChannelNameOffset = 7;
#else // CHANGE_TGA_JAPAN
				iChannelNameOffset = 8;
#endif // CHANGE_TGA_JAPAN
			}
			else if(wstrtempFieldName == GET_STRING( STR_ID_17732 ))
			{

				wstring wstrtempField = L"ELDER";
				wstrtempField += wstrtempFieldNameNum;
#ifdef CHANNEL_UI_EU
				pPicture->SetTex(L"DLG_UI_Title02_B.TGA", wstrtempField.c_str());
#else //CHANNEL_UI_EU
				pPicture->SetTex(L"DLG_UI_Title02_A.TGA", wstrtempField.c_str());	
#endif //CHANNEL_UI_EU
#ifdef CHANGE_TGA_JAPAN
				//iChannelNameOffset = 8;
#else // CHANGE_TGA_JAPAN
				iChannelNameOffset = 8;
#endif // CHANGE_TGA_JAPAN
			}
			else if(wstrtempFieldName == GET_STRING( STR_ID_17733 ))
			{
				wstring wstrtempField = L"BESMA";
				wstrtempField += wstrtempFieldNameNum;
#ifdef CHANNEL_UI_EU
				pPicture->SetTex(L"DLG_UI_Title02_B.TGA", wstrtempField.c_str());
#else //CHANNEL_UI_EU
				pPicture->SetTex(L"DLG_UI_Title02_A.TGA", wstrtempField.c_str());	
#endif //CHANNEL_UI_EU

			}
			else if(wstrtempFieldName == GET_STRING( STR_ID_17734 ))
			{
				wstring wstrtempField = L"ALTERA";
				wstrtempField += wstrtempFieldNameNum;
#ifdef CHANNEL_UI_EU
				pPicture->SetTex(L"DLG_UI_Title02_B.TGA", wstrtempField.c_str());
#else //CHANNEL_UI_EU
				pPicture->SetTex(L"DLG_UI_Title02_A.TGA", wstrtempField.c_str());	
#endif //CHANNEL_UI_EU	
			}
			else if(wstrtempFieldName == GET_STRING( STR_ID_23098 ))
			{
				wstring wstrtempField = L"PEITA";
				wstrtempField += wstrtempFieldNameNum;
#ifdef CHANNEL_UI_EU
				pPicture->SetTex(L"DLG_UI_Title02_B.TGA", wstrtempField.c_str());
#else //CHANNEL_UI_EU
				pPicture->SetTex(L"DLG_UI_Title02_A.TGA", wstrtempField.c_str());	
#endif //CHANNEL_UI_EU	

#ifdef CHANGE_TGA_JAPAN
				//iChannelNameOffset = 8;
#else // CHANGE_TGA_JAPAN
				iChannelNameOffset = 8;	
#endif // CHANGE_TGA_JAPAN
			}
			else if(wstrtempFieldName == GET_STRING( STR_ID_17735 ))
			{

				wstring wstrtempField = L"VELDER";
				wstrtempField += wstrtempFieldNameNum;
#ifdef CHANNEL_UI_EU
				pPicture->SetTex(L"DLG_UI_Title02_B.TGA", wstrtempField.c_str());
#else //CHANNEL_UI_EU
				pPicture->SetTex(L"DLG_UI_Title02_A.TGA", wstrtempField.c_str());	
#endif //CHANNEL_UI_EU	
#ifdef CHANGE_TGA_JAPAN
				//iChannelNameOffset = 7;
#else // CHANGE_TGA_JAPAN
				iChannelNameOffset = 7;
#endif // CHANGE_TGA_JAPAN

			}
			else if(wstrtempFieldName == GET_STRING( STR_ID_17736 ))
			{
				wstring wstrtempField = L"HAMEL";
				wstrtempField += wstrtempFieldNameNum;
#ifdef CHANNEL_UI_EU
				pPicture->SetTex(L"DLG_UI_Title02_B.TGA", wstrtempField.c_str());
#else //CHANNEL_UI_EU
				pPicture->SetTex(L"DLG_UI_Title02_A.TGA", wstrtempField.c_str());	
#endif //CHANNEL_UI_EU	
#ifdef CHANGE_TGA_JAPAN
				//iChannelNameOffset = 7;
#else // CHANGE_TGA_JAPAN
				iChannelNameOffset = 7;
#endif // CHANGE_TGA_JAPAN
			}

			if (iFieldNum >= 10)
				iChannelNameOffset -= 5;

#ifdef CLIENT_COUNTRY_US
			// 미국은 위치 조절 필요 없습니다.
			iChannelNameOffset = 0;
#endif //CLIENT_COUNTRY_US

			if((j+1)%2 == 0)
			{
#ifdef CHANNEL_UI_EU
#ifdef CHANNEL_UI_TH
				pPointData->leftTopPoint = D3DXVECTOR2(678.f+iChannelNameOffset,126 + static_cast< float >( (j/2) * 48));
#else //CHANNEL_UI_TH
				pPointData->leftTopPoint = D3DXVECTOR2(678.f+iChannelNameOffset,132 + static_cast< float >( (j/2) * 48));
#endif CHANNEL_UI_TH
#else //CHANNEL_UI_EU
				pPointData->leftTopPoint = D3DXVECTOR2(678.f+iChannelNameOffset,126 + static_cast< float >( (j/2) * 48));
#endif //CHANNEL_UI_EU
			}
			else
			{
#ifdef CHANNEL_UI_EU
#ifdef CHANNEL_UI_TH
				pPointData->leftTopPoint = D3DXVECTOR2(355.f+iChannelNameOffset,126 + static_cast< float >( (j/2) * 48));
#else //CHANNEL_UI_TH
				pPointData->leftTopPoint = D3DXVECTOR2(355.f+iChannelNameOffset,132 + static_cast< float >( (j/2) * 48));
#endif CHANNEL_UI_TH
#else //CHANNEL_UI_EU
				pPointData->leftTopPoint = D3DXVECTOR2(355.f+iChannelNameOffset,126 + static_cast< float >( (j/2) * 48));
#endif //CHANNEL_UI_EU
			}

			pPicture->SetPoint(pPointData);
			pPicture->SetShow(true);
			pStaticCHName->AddPicture(pPicture);
			pStaticCHName->SetShow(true);
		}

		std::map<int, KChannelBonusInfo>::const_iterator mit2 = g_pInstanceData->m_mapChannelBonusInfo.find(g_pInstanceData->m_vecChannelInfo[j].m_iChannelID);

		//ASSERT(mit2 != g_pInstanceData->m_mapChannelBonusInfo.end());

		if(mit2 != g_pInstanceData->m_mapChannelBonusInfo.end() )
		{


			wstring wstrChannelLv = KncUtil::toWideString( boost::str(boost::format("(LV%1%~%2%)") % mit2->second.m_iBeginLv % mit2->second.m_iEndLv) );


			float fRightChannelStrPos = 678.f;
			float fLeftChannelStrPos = 355.f;
			wstring wstrLvWordCheck = L"";
			bool bStrLvWordCheck = false;

			for ( int istrNumCheck = 0; istrNumCheck< wstrChannelLv.length(); ++istrNumCheck )
			{
				CKTDGUIControl::CPictureData* pLvPicture = new CKTDGUIControl::CPictureData();
				CKTDGUIControl::UIPointData* pLvPointData = new CKTDGUIControl::UIPointData();
				
				wstring wstrsubChannelLv = L"";

				if(pStaticCHName == NULL && pLvPicture == NULL && pLvPointData == NULL)
					continue;

				
				wstrsubChannelLv = wstrChannelLv[istrNumCheck];

				if ( wstrsubChannelLv == L"L" || wstrsubChannelLv == L"V" )
				{
					
					wstrLvWordCheck += wstrChannelLv[istrNumCheck];
					if ( wstrLvWordCheck == L"LV")
					{
						bStrLvWordCheck = true;
					}
					else
					{
						continue;	
					}
			
				}

				if((j+1)%2 == 0)
				{
					pLvPointData->leftTopPoint = D3DXVECTOR2(fRightChannelStrPos,126 +20 + static_cast< float >( (j/2) * 48));
					if ( bStrLvWordCheck == true)
					{
						fRightChannelStrPos += 19.f;
					}
					else
					{
						fRightChannelStrPos += 7.f;
					}
					
				}
				else
				{
					pLvPointData->leftTopPoint = D3DXVECTOR2(fLeftChannelStrPos,126 +20 + static_cast< float >( (j/2) * 48));
					if ( bStrLvWordCheck == true)
					{
						fLeftChannelStrPos += 19.f;
					}
					else
					{
						fLeftChannelStrPos += 7.f;
					}
					
				}



				if ( bStrLvWordCheck == false )
				{
					pLvPicture->SetTex(L"DLG_UI_Title02_A.TGA", wstrsubChannelLv.c_str() );
				}
				else
				{
					pLvPicture->SetTex(L"DLG_UI_Title02_A.TGA", wstrLvWordCheck.c_str() );
					bStrLvWordCheck = false;
				}

				pLvPicture->SetPoint(pLvPointData);
				pLvPicture->SetShow(true);
				pStaticCHName->AddPicture(pLvPicture);

			}

			pStaticCHName->SetShow(true);
		}
	}
#endif //SERV_CHANNEL_LIST_RENEWAL
}

#else	ADD_SERVER_GROUP

void CX2StateServerSelect::UpdateChannelListUI()
{
#ifdef SHOW_ONLY_ADVERTISEMENT
	if( true == m_bHideServerSelectUI )
		return false;
#endif // SHOW_ONLY_ADVERTISEMENT

	if( NULL == m_pDLGServerSelectBack )
		return;

	
	// 채널추가
	const int MAX_CHANNEL_COUNT = 16;


	// 채널 리스트 버튼 초기화
	WCHAR wszControlName[32] = L"";

	CKTDGUIStatic* pStatic1 = (CKTDGUIStatic*)m_pDLGServerSelectBack->GetControl( L"chanal_line" );		// 상태바(3)
	CKTDGUIStatic* pStatic3 = (CKTDGUIStatic*)m_pDLGServerSelectBack->GetControl( L"chanal_state" );	// 3
	CKTDGUIStatic* pStatic4 = (CKTDGUIStatic*)m_pDLGServerSelectBack->GetControl( L"chanal_num" );		// 2
	CKTDGUIStatic* pStaticChannelNum = (CKTDGUIStatic*)m_pDLGServerSelectBack->GetControl( L"channel_num" );

	for(int i=0; i<MAX_CHANNEL_COUNT; ++i)
	{
		StringCchPrintf( wszControlName, ARRAY_SIZE(wszControlName), L"chanal%d", i+1);
		CKTDGUIButton *pButton = (CKTDGUIButton*)m_pDLGServerSelectBack->GetControl( wszControlName );
		if(pButton != NULL)
			pButton->SetShowEnable(false, false);		

		StringCchPrintf( wszControlName, ARRAY_SIZE(wszControlName), L"statechanal%d", i+1);
		CKTDGUIStatic* pStatic2 = (CKTDGUIStatic*)m_pDLGServerSelectBack->GetControl( wszControlName );		// 상태명
		if(pStatic2 != NULL)
			pStatic2->SetShowEnable(false, false);			

		pStatic1->GetPicture(i*3)->SetShow(false);
		pStatic1->GetPicture(i*3+1)->SetShow(false);
		pStatic1->GetPicture(i*3+2)->SetShow(false);
		pStatic3->GetPicture(i*3)->SetShow(false);
		pStatic3->GetPicture(i*3+1)->SetShow(false);
		pStatic3->GetPicture(i*3+2)->SetShow(false);
		pStatic4->GetPicture(i)->SetShow(false);
		//pStatic4->GetPicture(i*2+1)->SetShow(false);
	}				





	if( false == m_bReceivedChannelList )
		return;

#ifdef CHANGE_CHANNEL
	if( g_pInstanceData == NULL || g_pInstanceData->m_vecChannelInfo.empty() == true)
		return;
#else
	if( true == m_vecChannelInfo.empty() )
		return;
#endif

	// 채널/서버 게이지 표기법 변경
	float CHANNEL_CONGESTION_SCOPE_TEMP[] = {0.1f, 0.2f, 0.3f, 0.999f, };
	const float CHANNEL_CONGESTION_SCOPE[] = {0.2f, 0.4f, 0.6f, 0.999f, };
	const float CHANNEL_CONGESTION_DISPLAY[] = {0.25f, 0.5f, 0.75f, 0.99f, };

	const float CHANNEL_CONGESTION_CRITERION[] = { 0.3f, 0.6f, 0.999f, };			// 서버 혼잡 상태 경계값 30%, 60%, 100%
	int numChannel = 0;
	//int serverLevel = 0; 

	int nMaxNum = 0;
	int nCurrentNum = 0;

	// 채널 리스트 정보 갱신
#ifdef CHANGE_CHANNEL
	for( int i=0; i<(int)g_pInstanceData->m_vecChannelInfo.size() && i<MAX_CHANNEL_COUNT; i++ )
#else
	for( int i=0; i<(int)m_vecChannelInfo.size() && i<MAX_CHANNEL_COUNT; i++ )
#endif
	{			
		StringCchPrintf( wszControlName, ARRAY_SIZE(wszControlName), L"chanal%d", i+1);
		CKTDGUIButton *pButton = (CKTDGUIButton*)m_pDLGServerSelectBack->GetControl( wszControlName );
		StringCchPrintf( wszControlName, ARRAY_SIZE(wszControlName), L"statechanal%d", i+1);
		CKTDGUIStatic* pStatic2 = (CKTDGUIStatic*)m_pDLGServerSelectBack->GetControl( wszControlName ); // 상태명

		if(i == 0 || i == 1 || i == 6 || i == 7)
		{
			// 채널그룹중 첫채널은 분포도 다르게 표시
			CHANNEL_CONGESTION_SCOPE_TEMP[0] = 0.1f;
			CHANNEL_CONGESTION_SCOPE_TEMP[1] = 0.2f;
			CHANNEL_CONGESTION_SCOPE_TEMP[2] = 0.3f;
			CHANNEL_CONGESTION_SCOPE_TEMP[3] = 0.999f;
		}
		else
		{
			CHANNEL_CONGESTION_SCOPE_TEMP[0] = CHANNEL_CONGESTION_SCOPE[0];
			CHANNEL_CONGESTION_SCOPE_TEMP[1] = CHANNEL_CONGESTION_SCOPE[1];
			CHANNEL_CONGESTION_SCOPE_TEMP[2] = CHANNEL_CONGESTION_SCOPE[2];
			CHANNEL_CONGESTION_SCOPE_TEMP[3] = CHANNEL_CONGESTION_SCOPE[3];
		}
		

		// Show 설정
		pButton->SetShowEnable(true, true);
		pStatic2->SetShowEnable(true, true);
		pStatic3->GetPicture(i*3)->SetShow(true);
		pStatic3->GetPicture(i*3+1)->SetShow(true);
		pStatic3->GetPicture(i*3+2)->SetShow(true);
		pStatic4->GetPicture(i)->SetShow(true);
		//pStatic4->GetPicture(i*2+1)->SetShow(true);

		numChannel++;

#ifdef CHANGE_CHANNEL
		const KChannelInfo& channelInfo = g_pInstanceData->m_vecChannelInfo[i];
#else
		const KChannelInfo& channelInfo = m_vecChannelInfo[i];
#endif
		float fChannelCongestionRate = (float) channelInfo.m_iCurrentUser / (float) channelInfo.m_iMaxUser;
		float fChannelCongestionRateDisplay = 0.f;
		if(fChannelCongestionRate > CHANNEL_CONGESTION_SCOPE_TEMP[3])
		{
			fChannelCongestionRateDisplay = 1.f;
		}
		else if(fChannelCongestionRate > CHANNEL_CONGESTION_SCOPE_TEMP[2])
		{
			fChannelCongestionRateDisplay = CHANNEL_CONGESTION_DISPLAY[3]; // 혼잡
		}
		else if(fChannelCongestionRate > CHANNEL_CONGESTION_SCOPE_TEMP[1])
		{
			fChannelCongestionRateDisplay = CHANNEL_CONGESTION_DISPLAY[2]; // 보통
		}
		else if(fChannelCongestionRate > CHANNEL_CONGESTION_SCOPE_TEMP[0])
		{
			fChannelCongestionRateDisplay = CHANNEL_CONGESTION_DISPLAY[1]; // 원활
		}
		else
		{
			fChannelCongestionRateDisplay = CHANNEL_CONGESTION_DISPLAY[0]; // 원활
		}


		nMaxNum += channelInfo.m_iMaxUser;
		nCurrentNum += channelInfo.m_iCurrentUser;

		//if( fChannelCongestionRate < CHANNEL_CONGESTION_CRITERION[0] )
		if(fChannelCongestionRateDisplay <= CHANNEL_CONGESTION_DISPLAY[1])
		{
			//serverLevel += 1;
			pStatic1->GetPicture(i*3)->SetSizeX(pStatic1->GetPicture(i*3)->GetOriginalSize().x * fChannelCongestionRateDisplay );
			pStatic1->GetPicture(i*3)->SetShow( true );				
			pStatic2->GetString(0)->msg = GET_STRING( STR_ID_721 );
		}
		//else if( fChannelCongestionRate < CHANNEL_CONGESTION_CRITERION[1] )
		else if(fChannelCongestionRateDisplay <= CHANNEL_CONGESTION_DISPLAY[2])
		{
			//serverLevel += 2;
			pStatic1->GetPicture(i*3+1)->SetSizeX(pStatic1->GetPicture(i*3+1)->GetOriginalSize().x * fChannelCongestionRateDisplay );
			pStatic1->GetPicture(i*3+1)->SetShow( true );
			pStatic2->GetString(0)->msg = GET_STRING( STR_ID_722 );
		}
		//else if( fChannelCongestionRate < CHANNEL_CONGESTION_CRITERION[2] )
		else if(fChannelCongestionRateDisplay <= CHANNEL_CONGESTION_DISPLAY[3])
		{
			//serverLevel += 3;
			pStatic1->GetPicture(i*3+2)->SetSizeX(pStatic1->GetPicture(i*3+2)->GetOriginalSize().x * fChannelCongestionRateDisplay );
			pStatic1->GetPicture(i*3+2)->SetShow( true );
			pStatic2->GetString(0)->msg = GET_STRING( STR_ID_723 );
		}
		else
		{
			//serverLevel += 4;
			pStatic1->GetPicture(i*3+2)->SetSizeX(pStatic1->GetPicture(i*3+2)->GetOriginalSize().x );
			pStatic1->GetPicture(i*3+2)->SetShow( true );
			pStatic2->GetString(0)->msg = GET_STRING( STR_ID_724 );

			pButton->SetShowEnable(true, false);
		}

#if 0
		// 개발자모드에서만 채널상태 보이도록 한다.
		if ( g_pData->GetMyUser()->GetAuthLevel() == CX2User::XUAL_DEV )
		{
			WCHAR staticBuff[64] = {0,};
			//wsprintf(staticBuff, L"%d/%d", channelInfo.m_iCurrentUser, channelInfo.m_iMaxUser);
			StringCchPrintf( buff, 64, L"%d/%d", channelInfo.m_iCurrentUser, channelInfo.m_iMaxUser);
			pStaticChannelNum->GetString(i)->msg = staticBuff;
		}
#endif
	}





	// 서버군 UI 갱신
	const WCHAR SOLRES_GAGE_CONTROL_NAME[] = L"Sever_Soless_Server_Gauge";	// 솔레스 서버군 static control 이름 


	//if( numChannel != 0 )
	//{
	//	serverLevel = serverLevel / numChannel;
	//}

	CKTDGUIStatic* pStatic = (CKTDGUIStatic*)m_pDLGServerSelectBack->GetControl( SOLRES_GAGE_CONTROL_NAME );
	if ( pStatic != NULL )
	{
		for(int i=0; i<6; ++i)
		{
			pStatic->GetPicture(i)->SetShow(false);
		}

		float  fServerCongestionRate = (float)nCurrentNum / (float)nMaxNum;
		float fServerCongestionRateDisplay = 0.f;
		if(fServerCongestionRate > CHANNEL_CONGESTION_SCOPE[3])
		{
			fServerCongestionRateDisplay = 1.f;
		}
		else if(fServerCongestionRate > CHANNEL_CONGESTION_SCOPE[2])
		{
			fServerCongestionRateDisplay = CHANNEL_CONGESTION_DISPLAY[3];
		}
		else if(fServerCongestionRate > CHANNEL_CONGESTION_SCOPE[1])
		{
			fServerCongestionRateDisplay = CHANNEL_CONGESTION_DISPLAY[2];
		}
		else if(fServerCongestionRate > CHANNEL_CONGESTION_SCOPE[0])
		{
			fServerCongestionRateDisplay = CHANNEL_CONGESTION_DISPLAY[1];
		}
		else
		{
			fServerCongestionRateDisplay = CHANNEL_CONGESTION_DISPLAY[0];
		}
		 
		//if( fServerCongestionRate < CHANNEL_CONGESTION_CRITERION[0] )
		if(fServerCongestionRateDisplay <= CHANNEL_CONGESTION_DISPLAY[1])
		{
			pStatic->GetPicture(4)->SetSizeX(pStatic->GetPicture(4)->GetOriginalSize().x * fServerCongestionRateDisplay);
			pStatic->GetPicture(4)->SetShow( true );
			pStatic->GetPicture(5)->SetShow( true );
		}
		//else if( fServerCongestionRate < CHANNEL_CONGESTION_CRITERION[1] )
		else if(fServerCongestionRateDisplay <= CHANNEL_CONGESTION_DISPLAY[2])
		{
			pStatic->GetPicture(2)->SetSizeX(pStatic->GetPicture(2)->GetOriginalSize().x * fServerCongestionRateDisplay);
			pStatic->GetPicture(2)->SetShow( true );
			pStatic->GetPicture(3)->SetShow( true );
		}
		//else if( fServerCongestionRate < CHANNEL_CONGESTION_CRITERION[2] )
		else if(fServerCongestionRateDisplay <= CHANNEL_CONGESTION_DISPLAY[3])
		{
			pStatic->GetPicture(0)->SetSizeX(pStatic->GetPicture(0)->GetOriginalSize().x * fServerCongestionRateDisplay);
			pStatic->GetPicture(0)->SetShow( true );
			pStatic->GetPicture(1)->SetShow( true );
		}
		else
		{
			pStatic->GetPicture(0)->SetSizeX(pStatic->GetPicture(0)->GetOriginalSize().x * fServerCongestionRateDisplay);
			pStatic->GetPicture(0)->SetShow( true );
			pStatic->GetPicture(1)->SetShow( true );
		}

#if 0 
		// 개발자모드에서만 채널상태 보이도록 한다.
		if ( g_pData->GetMyUser()->GetAuthLevel() == CX2User::XUAL_DEV )
		{
			CKTDGUIStatic* pStaticChannelNum = (CKTDGUIStatic*)m_pDLGServerSelectBack->GetControl( L"server_user_num" );

			WCHAR staticBuff[64] = {0,};
			//wsprintf(staticBuff, L"%d/%d", nCurrentNum, nMaxNum);
			StringCchPrintf( buff, 64, L"%d/%d", nCurrentNum, nMaxNum);
			pStaticChannelNum->GetString(0)->msg = staticBuff;
		}
#endif
#if 0 
		switch( serverLevel )
		{
		case KServerSetData::UCL_FREE:
			{
				pStatic->GetPicture(4)->SetSizeX(pStatic->GetPicture(4)->GetOriginalSize().x * 0.3f);
				pStatic->GetPicture(4)->SetShow( true );
				pStatic->GetPicture(5)->SetShow( true );
			} break;
		case KServerSetData::UCL_NORMAL:
			{
				pStatic->GetPicture(4)->SetSizeX(pStatic->GetPicture(4)->GetOriginalSize().x * 0.6f);
				pStatic->GetPicture(2)->SetShow( true );
				pStatic->GetPicture(3)->SetShow( true );
			}
		case KServerSetData::UCL_BUSY:
			{
				pStatic->GetPicture(4)->SetSizeX(pStatic->GetPicture(4)->GetOriginalSize().x * 0.9f);
				pStatic->GetPicture(0)->SetShow( true );
				pStatic->GetPicture(1)->SetShow( true );
			}
		case KServerSetData::UCL_FULL:
			{
				pStatic->GetPicture(0)->SetShow( true );
				pStatic->GetPicture(1)->SetShow( true );
			}
		}
#endif
	}


#ifndef SERVER_GROUP_UI_ADVANCED
	CKTDGUIButton *pServerButton = (CKTDGUIButton*)m_pDLGServerSelectBack->GetControl( L"sever_select" );
	pServerButton->SetDownStateAtNormal(true);
#endif SERVER_GROUP_UI_ADVANCED
}
#endif	ADD_SERVER_GROUP
// kimhc // 2009-12-15 // 이전에 플레이했던 채널 서버군


void CX2StateServerSelect::UnitHandling( double fTime, float fElapsedTime )
{
#ifdef SERV_UNIT_WAIT_DELETE //2012.03.07 lygan_조성욱 // 캐릭터별 삭제 대기시에 애니메이션 설정
	if(g_pData->GetMyUser() != NULL && m_UnitViewerUIList.empty() == false )
	{
		int i = (m_NowPage - 1)*UNIT_SELECT_UI_MAX_UNIT_NUM, j = 0;
		for( ; i < g_pData->GetMyUser()->GetUnitNum() && j < UNIT_SELECT_UI_MAX_UNIT_NUM; i++, j++ )
		{
			CX2Unit* pUnit = g_pData->GetMyUser()->GetUnitByIndex( i );
			if( pUnit == NULL )
				continue;	
			CX2UnitViewerUI* pUnitViewerUI = GetUnitViewer(pUnit);

			if ( pUnitViewerUI == NULL )
				continue;


			pUnitViewerUI->OnFrameMove( fTime, fElapsedTime );

			CX2Unit::UnitData* pUnitData = pUnitViewerUI->GetUnit()->GetUnitData();

			if ( pUnitViewerUI->GetXSkinAnim()->GetState() == CKTDGXSkinAnim::XAP_ONE_WAIT )
			{
				if ( m_pSelectUnit != NULL )
				{
					if ( pUnitViewerUI == GetUnitViewer( m_pSelectUnit ) )
						continue;
				}


				if(pUnitData->m_bDeleted == true)
				{
					if(pUnitData->m_bDeletedMotionPlay == false)
					{
						pUnitViewerUI->PlayByMotionType( CX2UnitViewerUI::UVUMT_WAIT_DELETE );
						pUnitData->m_bDeletedMotionPlay = true;
					}

				}
				else
				{
					pUnitViewerUI->PlayByMotionType( CX2UnitViewerUI::UVUMT_WAIT );
				}
			}
			else if( pUnitData->m_bDeleted == true && pUnitViewerUI->GetXSkinAnim()->GetState() == CKTDGXSkinAnim::XAS_PLAYING )
			{
				if(pUnitData->m_bDeletedMotionPlay == false)
				{
					pUnitViewerUI->PlayByMotionType( CX2UnitViewerUI::UVUMT_WAIT_DELETE );
					pUnitData->m_bDeletedMotionPlay = true;
				}
			}
		}
	}

#else //SERV_UNIT_WAIT_DELETE

	for ( int i = 0; i < (int)m_UnitViewerUIList.size(); i++ )
	{
		CX2UnitViewerUI* pUnitViewerUI = m_UnitViewerUIList[i];

		if ( pUnitViewerUI == NULL )
			continue;

		pUnitViewerUI->OnFrameMove( fTime, fElapsedTime );

		if ( pUnitViewerUI->GetXSkinAnim()->GetState() == CKTDGXSkinAnim::XAP_ONE_WAIT )
		{
			if ( m_pSelectUnit != NULL )
			{
				if ( pUnitViewerUI == GetUnitViewer( m_pSelectUnit ) )
					continue;
			}
			pUnitViewerUI->PlayByMotionType( CX2UnitViewerUI::UVUMT_WAIT );
		}


	}
#endif //SERV_UNIT_WAIT_DELETE
}




void CX2StateServerSelect::ClearUnitViewerList()
{
	for ( int i = 0; i < (int)m_UnitViewerUIList.size(); i++ )
	{
		CX2UnitViewerUI* pUnitViewerUI = m_UnitViewerUIList[i];
		CX2UnitViewerUI::DeleteKTDGObject( pUnitViewerUI );
	}
	m_UnitViewerUIList.resize(0);

#ifdef SERV_UNIT_WAIT_DELETE

	if(g_pData->GetMyUser() != NULL)
	{
		if(g_pData->GetMyUser()->GetUnitNum() != NULL)
		{
			int i = (m_NowPage - 1)*UNIT_SELECT_UI_MAX_UNIT_NUM, j = 0;
			for( ; i < g_pData->GetMyUser()->GetUnitNum() && j < UNIT_SELECT_UI_MAX_UNIT_NUM; i++, j++ )
			{
				CX2Unit* pUnitCheck = g_pData->GetMyUser()->GetUnitByIndex( i );
				if( pUnitCheck == NULL  )
					continue;	

				CX2Unit::UnitData* pUnitData = pUnitCheck->GetUnitData();
				pUnitData->m_bDeletedMotionPlay = false;
			}
		}
	}


#endif //SERV_UNIT_WAIT_DELETE
}

CX2UnitViewerUI* CX2StateServerSelect::GetUnitViewer( CX2Unit* pUnit )
{
	for ( int i = 0; i < (int)m_UnitViewerUIList.size(); i++ )
	{
		CX2UnitViewerUI* pUnitViewerUI = m_UnitViewerUIList[i];
		if ( pUnitViewerUI->GetUnit() == pUnit )
			return pUnitViewerUI;
	}

	return NULL;
}




void CX2StateServerSelect::DownAnimFrameMove( double fTime, float fElapsedTime )
{
	if ( m_bPlayDownAnim == true )
	{
		m_fDownAnimTime += fElapsedTime;

		for ( int i = 0; i < (int)m_UnitViewerUIList.size(); i++ )
		{
			CX2UnitViewerUI* pUnitViewerUI = m_UnitViewerUIList[i];
			if ( pUnitViewerUI != NULL && pUnitViewerUI->GetShowObject() == false )
			{
				if ( m_fDownAnimTime > (i * 0.4f) )
				{
					pUnitViewerUI->SetShowObject( true );
					pUnitViewerUI->PlayByMotionType( CX2UnitViewerUI::UVVMT_DOWN_LANDING );
					pUnitViewerUI->OnFrameMove( g_pKTDXApp->GetTime(),g_pKTDXApp->GetElapsedTime() );
				}
			}
		}

		if ( m_fDownAnimTime >= ( (float)g_pData->GetMyUser()->GetUnitNum() * 0.5f + 1.0f ) )
		{
			m_bPlayDownAnim = false;
		}
	}
}

void CX2StateServerSelect::PlayDownAnim()
{
	m_bPlayDownAnim = true;
	m_fDownAnimTime = 0.0f;
}




bool CX2StateServerSelect::Handler_EGS_STATE_CHANGE_FIELD_REQ(bool bForceMove)
{
	KEGS_STATE_CHANGE_FIELD_REQ kPacket;
	D3DXVECTOR3 vStartPos(0, 0, 0);
	int startPos = 0;

	int iMapId = 0;
	if(g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetSelectUnit() != NULL)
		iMapId = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_nMapID;

	if( g_pData->GetLocationManager()->GetVillageMapTemplet((SEnum::VILLAGE_MAP_ID)iMapId) == NULL || 
		bForceMove == true )
	{
		// 강제 루벤이동
		iMapId = SEnum::VMI_RUBEN;
		//vStartPos = D3DXVECTOR3(-16.f, 1445.f, -2873.f);
		startPos = 1;
	}	
	else
	{
		if( g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_EXP <= 0 )
		{
			// 초기진입으로 본다.

			// 강제 루벤이동
			iMapId = SEnum::VMI_RUBEN;
			startPos = 1;
		}
		else
		{
			startPos = g_pData->GetLocationManager()->GetLoginPos( iMapId );

			// 로그아웃했던 위치를 계산한다.
#ifdef REMEMBER_LOGOUT_POSITION_TEST
			const CX2World::WorldData* pWorldData = g_pData->GetWorldManager()->GetWorldData( (CX2World::WORLD_ID) g_pData->GetLocationManager()->GetWorldID( (SEnum::VILLAGE_MAP_ID) iMapId ) );
			if( NULL != pWorldData )
			{
				CX2World* pWorld = g_pData->GetWorldManager()->CreateWorld( pWorldData->worldID ); // fix!! 월드 생성하지 않고 라인맵 정보 가져올 수 있게 수정해야함
				if( NULL != pWorld )
				{
					pWorld->SetShowObject( false );
					CKTDGLineMap* pLineMap = pWorld->GetLineMap();
					if( NULL != pLineMap )
					{
						CKTDGLineMap::LineData* pLineData = pLineMap->GetLineData( g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_ucLastTouchLineIndex );
						if( NULL != pLineData )
						{
							float fLastPosValue = halfToFloat( g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_usLastPosValue );
							if( fLastPosValue < 0.f )
								fLastPosValue = 0.f;
							if( fLastPosValue > 1.f )
								fLastPosValue = 1.f;

							vStartPos = pLineData->startPos * (1.f - fLastPosValue) + pLineData->endPos * fLastPosValue;
							vStartPos = pLineMap->GetLandPosition( vStartPos, LINE_RADIUS, (int*) NULL );

							startPos = -1;
						}
					}
					SAFE_DELETE( pWorld );
				}
			}
#endif REMEMBER_LOGOUT_POSITION_TEST
		}		
	}	


	if( startPos >= 0 )
	{
		vStartPos = g_pData->GetLocationManager()->GetStartPosLoc(startPos);	
	}

	g_pData->GetLocationManager()->SetLastPos(vStartPos);
	//g_pData->GetLocationManager()->SetLastPosLineIndex();


	kPacket.m_iMapID =  (int)iMapId;	


	g_pData->GetLocationManager()->SetVillage( (SEnum::VILLAGE_MAP_ID)kPacket.m_iMapID, vStartPos );

	g_pData->GetServerProtocol()->SendPacket( EGS_STATE_CHANGE_FIELD_REQ, kPacket );
	g_pMain->AddServerPacket( EGS_STATE_CHANGE_FIELD_ACK );
	

	return true;
}

bool CX2StateServerSelect::Handler_EGS_STATE_CHANGE_FIELD_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_STATE_CHANGE_FIELD_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	
	if( g_pMain->DeleteServerPacket( EGS_STATE_CHANGE_FIELD_ACK ) == true )
	{
		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{
			g_pMain->CreateStateChangeDLG( GET_STRING( STR_ID_541 ) );			

			bool bCanCreateWorld = true;
			CX2World::WORLD_ID	m_iWorldId = CX2World::WI_NONE;
			SEnum::VILLAGE_MAP_ID m_villageId = SEnum::VMI_INVALID;;
			D3DXVECTOR3 m_vPos( 0.0f, 0.0f, 0.0f );	

			// 현재 설정된 village를 생성한다.	
			if(g_pData != NULL && g_pData->GetLocationManager() != NULL)
			{
				g_pData->GetLocationManager()->GetVillage(m_villageId, m_vPos);
				m_iWorldId = (CX2World::WORLD_ID)g_pData->GetLocationManager()->GetWorldID(m_villageId);		
			}
			else
			{
				// 마을을 생성할 수 없음.
				bCanCreateWorld = false;
			}

			if(m_iWorldId <= CX2World::WI_NONE || m_iWorldId >= CX2World::WI_END)
			{
				// 마을을 생성할 수 없음
				bCanCreateWorld = false;
			}

			// 마을 생성		
			if(bCanCreateWorld == true)
			{
//#if 0
//				g_pTFieldGame = new CX2TFieldGame(m_iWorldId);
//				
//				if(g_pTFieldGame != NULL)
//				{
//					g_pTFieldGame->AddUnit( g_pData->GetMyUser()->GetSelectUnit(), true, true );
//					g_pTFieldGame->SetAddedMyUnit(true);					
//				}
//#endif

				g_pKTDXApp->SendGameMessage( XGM_STATE_CHANGE, CX2Main::XS_VILLAGE_MAP, NULL, false );
			}
		}
		else
		{		
			switch ( kEvent.m_iOK )
			{
			case NetError::ERR_FIELD_05:
				{
					g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_REPLACED_STRING((STR_ID_18892, "i",kEvent.m_iLimitLevel)) , g_pMain->GetNowState());

				} break;
			case NetError::ERR_FIELD_06:
				{
					if( NULL != g_pData && NULL != g_pData->GetDungeonManager() )
					{
						wstring DungeonName = g_pData->GetDungeonManager()->MakeDungeonNameString( static_cast<CX2Dungeon::DUNGEON_ID>(kEvent.m_iRequireDungeonID));
						g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), 	GET_REPLACED_STRING((STR_ID_18893, "L", DungeonName )),	g_pMain->GetNowState());
					}
				} break;
			}

			// 강제 루벤이동
			Handler_EGS_STATE_CHANGE_FIELD_REQ(true);

			// 이전위치로 복원
			//g_pData->GetLocationManager()->RestoreVillage();
		}

	}
	return true;
}

bool CX2StateServerSelect::Handler_EGS_CHANGE_USER_LIST_IN_SECTOR_NOT( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_CHANGE_USER_LIST_IN_SECTOR_NOT kEvent;

	DeSerialize( pBuff, &kEvent );		

	if(g_pTFieldGame != NULL)
	{
		for(int i=0; i<(int)kEvent.m_vecNewUserList.size(); ++i)
		{			
			g_pTFieldGame->JoinFieldUnit( &kEvent.m_vecNewUserList[i] );
		}			
	}

	return true;
}







void CX2StateServerSelect::Handler_EGS_CREATE_TUTORIAL_ROOM_REQ()
{
	KEGS_CREATE_TUTORIAL_ROOM_REQ kEGS_CREATE_ROOM_REQ;

	kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_RoomType			= CX2Room::RT_DUNGEON;
	kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_RoomName			= L"tutorial";
	kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_bPublic			= false;
	kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_wstrPassword		= L"a*h97lk2";
	kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_MaxSlot			= 1;
	
	kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_DifficultyLevel	= CX2Dungeon::DL_NORMAL;
	kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_fPlayTime			= 19999.0f;
	kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_cGetItemType		= CX2DungeonRoom::DGIT_RANDOM;

#ifdef REFORM_TUTORIAL
	switch( g_pData->GetMyUser()->GetSelectUnit()->GetType() )
	{
	default:
	case CX2Unit::UT_ELSWORD:
		{
			kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_iDungeonID	= CX2Dungeon::DI_BATTLE_FIELD_TUTORIAL_ELSWORD;
		} break;

	case CX2Unit::UT_ARME:
		{
			kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_iDungeonID	= CX2Dungeon::DI_BATTLE_FIELD_TUTORIAL_AISHA;
		} break;

	case CX2Unit::UT_LIRE:
		{
			kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_iDungeonID	= CX2Dungeon::DI_BATTLE_FIELD_TUTORIAL_LENA;
		} break;

	case CX2Unit::UT_RAVEN:
		{
			kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_iDungeonID	= CX2Dungeon::DI_BATTLE_FIELD_TUTORIAL_RAVEN;
		} break;

	case CX2Unit::UT_EVE:
		{
			kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_iDungeonID	= CX2Dungeon::DI_BATTLE_FIELD_TUTORIAL_EVE;		
		} break;

		//{{ kimhc // 2010.11.24 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
	case CX2Unit::UT_CHUNG:
		{
			kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_iDungeonID	= CX2Dungeon::DI_BATTLE_FIELD_TUTORIAL_CHUNG;
		} break;
#endif	NEW_CHARACTER_CHUNG
		//}} kimhc // 2010.11.24 //  2010-12-23 New Character CHUNG

#ifdef ARA_CHARACTER_BASE
	case CX2Unit::UT_ARA:
		{
			kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_iDungeonID	= CX2Dungeon::DI_BATTLE_FIELD_TUTORIAL_ARA;
		} break;
#endif
#ifdef NEW_CHARACTER_EL
	case CX2Unit::UT_ELESIS:
		{
			kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_iDungeonID	= CX2Dungeon::DI_BATTLE_FIELD_TUTORIAL_EL;
		} break;
#endif // NEW_CHARACTER_EL
	}
#else
	switch( g_pData->GetMyUser()->GetSelectUnit()->GetType() )
	{
	default:
	case CX2Unit::UT_ELSWORD:
		{
			kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_iDungeonID	= CX2Dungeon::DI_TUTORIAL_ELSWORD;
		} break;

	case CX2Unit::UT_ARME:
		{
			kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_iDungeonID	= CX2Dungeon::DI_TUTORIAL_ARME;
		} break;

	case CX2Unit::UT_LIRE:
		{
			kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_iDungeonID	= CX2Dungeon::DI_TUTORIAL_LIRE;
		} break;

	case CX2Unit::UT_RAVEN:
		{
			kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_iDungeonID	= CX2Dungeon::DI_TUTORIAL_RAVEN;
		} break;

	case CX2Unit::UT_EVE:
		{
			kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_iDungeonID	= CX2Dungeon::DI_TUTORIAL_EVE;
		} break;

//{{ kimhc // 2010.11.24 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
	case CX2Unit::UT_CHUNG:
		{
			kEGS_CREATE_ROOM_REQ.m_RoomInfo.m_iDungeonID	= CX2Dungeon::DI_TUTORIAL_CHUNG;
		} break;
#endif	NEW_CHARACTER_CHUNG
//}} kimhc // 2010.11.24 //  2010-12-23 New Character CHUNG
	}
#endif //REFORM_TUTORIAL

	g_pData->GetServerProtocol()->SendPacket( EGS_CREATE_TUTORIAL_ROOM_REQ, kEGS_CREATE_ROOM_REQ );
	g_pMain->AddServerPacket( EGS_CREATE_TUTORIAL_ROOM_ACK, 40.f );	
}

bool CX2StateServerSelect::Handler_EGS_CREATE_TUTORIAL_ROOM_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_CREATE_TUTORIAL_ROOM_ACK kEvent;
	DeSerialize( pBuff, &kEvent );	
	//CX2PacketLog::PrintLog( &kEvent );

	//SAFE_DELETE_DIALOG( m_pDLGCreateDungeonRoom );
	SAFE_DELETE_DIALOG( m_pDLGMsgBox );
	if( g_pMain->DeleteServerPacket( EGS_CREATE_TUTORIAL_ROOM_ACK ) == true )
	{
		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{
/*
			int startPos = 0;
			int iMapId;			
			if(g_pData != NULL && g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetSelectUnit() != NULL)
				iMapId = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_nMapID;

			if(g_pData->GetLocationManager()->GetVillageMapTemplet((SEnum::VILLAGE_MAP_ID)iMapId) == NULL)
			{
				// 강제 루벤이동
				iMapId = SEnum::VMI_RUBEN;
				//vStartPos = D3DXVECTOR3(-16.f, 1445.f, -2873.f);
				startPos = 1;
			}	
			else
			{
				startPos = g_pData->GetLocationManager()->GetLoginPos( iMapId );		
			}	
	
			D3DXVECTOR3 vStartPos = g_pData->GetLocationManager()->GetStartPosLoc(startPos);
			g_pData->GetLocationManager()->SetLastPos(vStartPos);
			//g_pData->GetLocationManager()->SetLastPosLineIndex();

			g_pData->GetLocationManager()->SetVillage((SEnum::VILLAGE_MAP_ID)iMapId, vStartPos);
*/

#ifdef HEAP_BROKEN_BY_ROOM
			CX2Room::InitializeRoomPacketData();
			CX2Room::SetRoomPacketData( kEvent.m_RoomInfo, kEvent.m_vecSlot, 
				vector<KRoomSlotInfo>(), kEvent.m_wstrCNIP );

			g_pData->DeleteAllRooms();
			g_pData->ResetDungeonRoom();
			if ( NULL != g_pX2Room && CX2Room::IsNewDataSet() )		/// 새로운 데이터가 셋팅 되었으면
				g_pX2Room->ApplyRoomPacketData();
#else	HEAP_BROKEN_BY_ROOM
			//방 데이터 제작
			CX2DungeonRoom* pCX2DungeonRoom = g_pData->ResetDungeonRoom();
			pCX2DungeonRoom->Set_KRoomInfo( kEvent.m_RoomInfo );
			pCX2DungeonRoom->Set_KRoomSlotInfoList( kEvent.m_vecSlot );
			pCX2DungeonRoom->ConnectRelayServer( kEvent.m_RoomInfo.m_wstrUDPRelayIP.c_str(), kEvent.m_RoomInfo.m_usUDPRelayPort );
			pCX2DungeonRoom->SetCenterServerIP( kEvent.m_wstrCNIP.c_str() );

#ifdef  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
           if ( g_pData != NULL && g_pData->GetGameUDP() != NULL && g_pMain != NULL )
                g_pData->GetGameUDP()->SetForceConnectMode( g_pMain->GetUDPMode( CX2Game::GT_DUNGEON ) );
#endif  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK

#endif // HEAP_BROKEN_BY_ROOM


			//방으로 스테이트 이동
			if(g_pInstanceData != NULL)
				g_pInstanceData->SetFirstJoinVillage(true);

			g_pMain->CreateStateChangeDLG( GET_STRING( STR_ID_645 ) );
			g_pKTDXApp->SendGameMessage( XGM_STATE_CHANGE, CX2Main::XS_DUNGEON_ROOM, NULL, false );	
			
			//던전 시작위치 저장
			if( NULL != g_pData && NULL != g_pData->GetPartyManager() )
				g_pData->GetPartyManager()->RememberComeBackInfoBeforeInstanceGameStart();

			if( NULL != g_pData && NULL != g_pData->GetPlayGuide() )
			{	//튜토리얼 진행방향 가이드
				g_pData->GetPlayGuide()->SetPlayGuide(CX2PlayGuide::PGT_NOVICE_GUIDE_01, true );
			}
			return true;
		}
	}

	if( true == g_pMain->GetPlayFieldDungeon() )
		g_pMain->SetPlayFieldDungeon( false );

	return false;
}








//////////////////////////////////////////////////////////////////////////
// statelogin에 있던 코드 옮겨온 것
bool CX2StateServerSelect::Handler_EGS_CONNECT_REQ( const wstring& wstrGameServerIP, const int iGameServerPort )
{

	if( true == wstrGameServerIP.empty() )
		return false;

	if( true == m_bSentEGS_CONNECT_REQ )
		return false;


	// game server에 접속하기전에 nexon passport를 갱신받아서 접속한다.
	if( false == g_pMain->GetManualLogin() )
	{
		switch( g_pMain->GetPublisher() )
		{
#ifdef _NEXON_KR_
		case CX2Main::XP_NEXON_KOREA:
			{
				WCHAR passPortStr[PASSPORT_SIZE] = { 0, };
				CNMCOClientObject::GetInstance().GetNexonPassport( passPortStr );
				const WCHAR* pLoginID = CNMCOClientObject::GetInstance().GetMyLoginId();

				if( 0 == wcslen( passPortStr ) && 
					0 != g_pNexonPassport.length() )
				{
					g_pInstanceData->SetUserPassword( g_pNexonPassport.c_str() );	// 체험 아이디인 경우
				}
				else
				{
					g_pInstanceData->SetUserPassword( passPortStr );		// 일반 넥슨 계정인 경우
				}

				g_pInstanceData->SetUserID( pLoginID );

				//{{ kimhc // 2010-06-24 // OTP 작업
			#ifdef	SERV_OTP_AUTH
				// 유저의 하드웨어 ID를 획득
				WCHAR lpNxHWID[NXHWID_SIZE] = { 0, };
				CNMCOClientObject::GetInstance().GetNxHWID( lpNxHWID );
				g_pInstanceData->SetNxHWID( lpNxHWID );
			#endif	SERV_OTP_AUTH
				//}} kimhc // 2010-06-24 // OTP 작업

			} break;
#endif _NEXON_KR_
#ifdef CLIENT_COUNTRY_TWHK
		case CX2Main::XP_GAMANIA_TW:
		case CX2Main::XP_GAMANIA_HK:
			{
				//ID/Pass는 이미 인스턴스 데이터에 세트되어 있다.
				//g_pInstanceData->SetUserID( g_pMain->GetArgvID() );
				//g_pInstanceData->SetUserPassword( g_pMain->GetArgvPass() );

			} break;
#endif CLIENT_COUNTRY_TWHK
		}
	}



	if( LoginErrorCheck() == true )
	{
		//접속시도
		//g_pData->ResetServerProtocol();


		if( true == g_pData->GetServerProtocol()->ConnectedToGameServer( wstrGameServerIP.c_str(), iGameServerPort ) )
		{
			g_pMain->SetConnectedGameServerIP( wstrGameServerIP );
			g_pMain->SetConnectedGameServerPort( iGameServerPort );

			KEGS_CONNECT_REQ kPacket;
			kPacket.m_wstrVersion = g_pMain->GetClientVersion();

			g_pData->GetServerProtocol()->SendPacket( EGS_CONNECT_REQ, kPacket );
			g_pMain->AddServerPacket( EGS_CONNECT_ACK, 60.0f );

			m_bSentEGS_CONNECT_REQ = true;

			SAFE_DELETE_DIALOG( m_pDLGMsgBox );
			m_pDLGMsgBox = g_pMain->KTDGUIMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_725 ), this );


			g_pInstanceData->SetConnectChannelServerID( m_iTryConnectChannelID );
			m_iTryConnectChannelID = -1;

	
			// 현재 접속접속한 game_server ip를 저장한다. (마을진입시 TRServer 접속용)
			if(g_pMain != NULL)
			{
				g_pMain->SetTRServerIp( wstrGameServerIP );
			}

			// 채널 서버와의 접속을 끊는다.
			Handler_ECH_DISCONNECT_REQ();
		}
		else
		{
#ifdef _SERVICE_
			m_pMsgOkFailToConnectServer = g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_726 ), this );
#else
			m_pMsgOkFailToConnectServer = g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_726 ), this );
#endif _SERVICE_

			//m_bFailToConnectServer = true;
		}		
	}
	else
	{
		//g_pData->DeleteServerProtocol();
		//g_pData->DeleteMyUser();		
		g_pKTDXApp->NoticeQuitType( CKTDXApp::KQT_INVALID_ID_OR_PASSWORD );
		g_pKTDXApp->SendGameMessage( XGM_QUIT_GAME, NULL, NULL, false );
	}
	return true;
}

bool CX2StateServerSelect::Handler_EGS_CONNECT_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_CONNECT_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	
	if( g_pMain->DeleteServerPacket( EGS_CONNECT_ACK ) == true )
	{
		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{
//#ifdef _SERVICE_
//			OpenGlobalURL( L"http://Elsword.nexon.com/Elsword/etc/gameinstall.aspx?Section=chceck3" );
//#endif
			g_pMain->SetServerUDPPort( kEvent.m_UDPPort );

#ifdef UDP_CAN_NOT_SEND_USER_KICK
			g_pInstanceData->SetUDPKickOff(kEvent.m_bUDPKickOff);
#endif //UDP_CAN_NOT_SEND_USER_KICK

			Handler_EGS_VERIFY_ACCOUNT_REQ();
		}
		else
		{
			//g_pData->DeleteServerProtocol();
			//g_pData->DeleteMyUser();
			switch( kEvent.m_iOK )
			{
			default:
				{
					g_pKTDXApp->NoticeQuitType( CKTDXApp::KQT_CONNECT_FAILED );
					g_pKTDXApp->SendGameMessage( XGM_QUIT_GAME, NULL, NULL, false );
				} break;

			case NetError::ERR_CONNECT_00:
				{
					MessageBox( g_pKTDXApp->GetHWND(), GET_STRING( STR_ID_727 ), L"Quit", MB_OK );
					g_pKTDXApp->SendGameMessage( XGM_QUIT_GAME, NULL, NULL, false );
				} break;

			case NetError::ERR_CONNECT_04:
				{
					m_bSentEGS_CONNECT_REQ = false;
					m_bConnectFullChannel = true;
					//g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), L"접속인원이 많아서 접속 할 수 없습니다.", this );
				} break;
			}
		}
	}
	return true;
}

bool CX2StateServerSelect::Handler_EGS_VERIFY_ACCOUNT_REQ()
{
	KEGS_VERIFY_ACCOUNT_REQ kPacket;

#ifdef _SERVICE_ 
	ELSWORD_VIRTUALIZER_START
#endif

#ifndef CLIENT_PURPLE_MODULE
	kPacket.m_wstrUserID = g_pInstanceData->GetUserID();
	kPacket.m_wstrPasswd = g_pInstanceData->GetUserPassword();
#else // CLIENT_PURPLE_MODULE

	bool bUsePurpleModule = true;
#ifdef CLIENT_PURPLE_MODULE_IN_HOUSE_AUTH
	if(g_pInstanceData->GetInHouseAccount())
		bUsePurpleModule = false;
#endif // CLIENT_PURPLE_MODULE_IN_HOUSE_AUTH

	if(bUsePurpleModule)
	{
		ConvertCharToWCHAR( kPacket.m_wstrUserID, PurpleGetUserID() );
#ifdef SERV_KOG_OTP_VERIFY
		if (g_pInstanceData->IsIDVerified())
			kPacket.m_wstrPasswd = g_pInstanceData->GetUserPassword();
		else
			ConvertCharToWCHAR( kPacket.m_wstrPasswd, PurpleGetAuthString() );
#else //SERV_KOG_OTP_VERIFY
		ConvertCharToWCHAR( kPacket.m_wstrPasswd, PurpleGetAuthString() );
#endif //SERV_KOG_OTP_VERIFY
	}
	else
	{
		kPacket.m_wstrUserID = g_pInstanceData->GetUserID();
		kPacket.m_wstrPasswd = g_pInstanceData->GetUserPassword();
	}
#endif // CLIENT_PURPLE_MODULE
	

#ifdef SERV_COUNTRY_TH
	kPacket.m_wstrSocketID = g_pInstanceData->GetSocketID();
#endif // SERV_COUNTRY_TH


#ifdef SERV_JAPAN_CHANNELING 
	char szBuffer[256] = {0};

#ifdef CLIENT_PURPLE_MODULE_IN_HOUSE_AUTH
	if( g_pInstanceData->GetInHouseAccount() == false )
	{
#endif // CLIENT_PURPLE_MODULE_IN_HOUSE_AUTH

		PurpleGetValue("site", szBuffer, 255);

#ifdef CLIENT_PURPLE_MODULE_IN_HOUSE_AUTH
	}
	else
	{
		int index = kPacket.m_wstrUserID.find( L'@' );
		if( index != -1 )
		{
			wstring wstrChannelCode = kPacket.m_wstrUserID.substr( index+1 , kPacket.m_wstrUserID.length()-1 );
			string strChannelCode;
			ConvertWCHARToChar( strChannelCode, wstrChannelCode.c_str() );
			sprintf(szBuffer, "%s", strChannelCode.c_str() );
		}
	}
#endif // CLIENT_PURPLE_MODULE_IN_HOUSE_AUTH

	if(strcmp(szBuffer, "MS") == 0)
		kPacket.m_iChannelingCode = KNexonAccountInfo::JCE_MSN;
	else if(strcmp(szBuffer, "NICO") == 0)
		kPacket.m_iChannelingCode = KNexonAccountInfo::JCE_NICO;
	else if(strcmp(szBuffer, "NX") == 0)
		kPacket.m_iChannelingCode = KNexonAccountInfo::JCE_NEXON;
	else
		kPacket.m_iChannelingCode = KNexonAccountInfo::JCE_NHN;
#endif // SERV_JAPAN_CHANNELING

#ifdef SERV_STEAM
	// ChannelingCode 값 정의
	// 0 : 북미 1 : 남미 2 : 스팀 북미 3: 스팀 남미
	kPacket.m_iChannelingCode = g_pInstanceData->GetChannelingCode();
#endif SERV_STEAM

	//{{ kimhc // 2010-06-24 // OTP 작업
#ifdef	SERV_OTP_AUTH
	kPacket.m_wstrHWID = g_pInstanceData->GetNxHWID();
#endif	SERV_OTP_AUTH
	//}} kimhc // 2010-06-24 // OTP 작업

#ifdef ARGUMENT_LOGIN

#ifdef _SERVICE_MANUAL_LOGIN_
	kPacket.m_bDebugAuth = g_pMain->GetManualLogin();		
#else _SERVICE_MANUAL_LOGIN_
#if defined( _IN_HOUSE_ ) || defined( _IN_HOUSE_SERVICE_READY_QA_ )
	kPacket.m_bDebugAuth = true;						// 사내는 true
#else 
	kPacket.m_bDebugAuth = false;						// 자동/수동 로그인을 모두 유저가 사용하므로 서비스 버전의 경우 무조건 false
#endif // _IN_HOUSE_ || _IN_HOUSE_SERVICE_READY_QA_
	
#endif _SERVICE_MANUAL_LOGIN_

#else ARGUMENT_LOGIN
	kPacket.m_bDebugAuth = g_pMain->GetManualLogin();
#endif ARGUMENT_LOGIN
	
#ifdef SERV_MACHINE_ID_DUPLICATE_CHECK 
#ifdef MACHINE_ID
	kPacket.m_strMachineID = g_pMain->GetMachineId();

	if( g_pInstanceData != NULL )
		g_pInstanceData->SetMachineId( kPacket.m_strMachineID );
#endif
#endif

#ifdef SERV_SERIAL_NUMBER_AVAILABILITY_CHECK_IN_GAMESERVER
	if( g_pInstanceData != NULL && g_pInstanceData->GetSN() != NULL )
		memcpy(kPacket.m_charServerSN, g_pInstanceData->GetSN(), sizeof(unsigned char) * SERVER_SN);
#endif

#ifdef SERV_DLL_LIST_CHECK_BEFOR_LOADING
	if( g_pInstanceData != NULL )
		kPacket.m_kChannelRandomKey = g_pInstanceData->GetChannelKey();
#endif

#ifdef SERV_CHECK_MACHINE_LOCALTIME
	CTime tCurTime = CTime::GetCurrentTime();
	kPacket.m_wstrClientTime = (const wchar_t*)tCurTime.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
#endif

	m_bEGS_VERIFY_ACCOUNT_REQ = true;

#ifdef SERV_STEAM
	kPacket.m_bSteamClient = CX2Steam::IsSteamUser();
#endif //SERV_STEAM
#ifdef SERV_CHANNELING_AERIA
	kPacket.m_bAeriaClient = g_pInstanceData->IsAeriaChanneling();
#endif //SERV_CHANNELING_AERIA

	g_pData->GetServerProtocol()->SendPacket( EGS_VERIFY_ACCOUNT_REQ, kPacket );
	g_pMain->AddServerPacket( EGS_VERIFY_ACCOUNT_ACK, 60 );

#ifdef SERV_MACHINE_ID_DUPLICATE_CHECK
	if( kPacket.m_strMachineID.empty() == true )
	{
#ifdef ADD_COLLECT_CLIENT_INFO
		if( g_pMain != NULL )
			g_pMain->SendHackInfo1( ANTIHACKING_ID::ANTIHACKING_GAME_33, "", false, false );
#else
		g_pMain->SendHackMail_VERIFYVALUE( ANTI_HACK_STRING_AntiHacking_MACHINE_ID_EMPTY );
#endif
	}
#endif

#ifdef _SERVICE_ 
	ELSWORD_VIRTUALIZER_END
#endif

	return true;	
}

bool CX2StateServerSelect::Handler_EGS_VERIFY_ACCOUNT_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
#ifdef _SERVICE_ 
	ELSWORD_VIRTUALIZER_START
#endif

	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_VERIFY_ACCOUNT_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	g_pMain->DeleteServerPacket( EGS_VERIFY_ACCOUNT_ACK );
	{
		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{
#ifdef SERV_EVENT_MONEY
			//유닛 선택창에 들어갔을때 팝업 출력 여부 체크 2013-07-02 김창한 
			if( kEvent.m_iEventMoney < 1 )
				g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_25386 ), g_pMain->GetNowState(), -1, -1.f, L"DLG_UI_OKMsgBoxPlus.lua" );
#endif //SERV_EVENT_MONEY

#if defined(SERV_MACHINE_ID_BLOCK) && defined(MACHINE_ID)
			ELSWORD_VIRTUALIZER_START
			Handler_EGS_CHECK_MACHINE_ID_REQ();
			ELSWORD_VIRTUALIZER_END
#endif

//#ifdef _SERVICE_
//			OpenGlobalURL( L"http://Elsword.nexon.com/Elsword/etc/gameinstall.aspx?Section=chceck4" );
//#endif

#ifdef SERV_KOG_OTP_VERIFY
			// 서버에서 발급받은 OTP를 클라이언트에 저장한다.
			if( false == kEvent.m_kAccountInfo.m_wstrOTP.empty() )
			{
				g_pInstanceData->SetIDVerified(true);
				g_pInstanceData->SetUserPassword( kEvent.m_kAccountInfo.m_wstrOTP );	
			}
#endif // SERV_KOG_OTP_VERIFY			

#ifdef _SERVICE_
#ifdef _NEXON_KR_
			//OpenGlobalURL( L"http://Elsword.nexon.com/Elsword/etc/gameinstall.aspx?Section=chceck5" );
			OpenGlobalURL( L"http://elsword.nexon.com/wiselog/gameinstall.aspx?section=check5" );
#endif
#endif
			//////////////////////////////////////////////////////////////////////////
			//{{ 2009. 7. 25  최육사	체크메시지
#ifdef HENIR_TEST
			if( kEvent.m_kAccountInfo.m_iAuthLevel == CX2User::XUAL_DEV )
			{
				//MessageBox( g_pKTDXApp->GetHWND(), L"헤니르 시공 테스트 모드입니다! 현재 버전은 본섭패치하면 안되는 테스트 버전입니다!", L"알림!", MB_OK );
			}
#endif HENIR_TEST
			//}}
			//////////////////////////////////////////////////////////////////////////

			wstringstream wstrstm;
			wstrstm << GET_REPLACED_STRING( ( STR_ID_728, "L", kEvent.m_kAccountInfo.m_wstrID ) );
			g_pMain->SetMailNameToFindHack( wstrstm.str().c_str() );
			g_pMain->SetUserIdToFindHack( kEvent.m_kAccountInfo.m_wstrID.c_str() );
#ifdef SERV_CLIENT_PUBLIC_IP
			g_pMain->SetPublicIp( kEvent.m_wstrClientIP );
#endif
			if(kEvent.m_cHackingUserType == CX2User::HUT_AGREE_HACK_USER)
				g_pMain->SetHackingUser(true);

			m_bEGS_VERIFY_ACCOUNT_ACK = true;
			CX2User::UserData* pUserData = new CX2User::UserData();
			*pUserData = kEvent;
			g_pData->ResetMyUser( pUserData );




			//m_MaxUnitNum	= g_pData->GetMyUser()->GetUserData()->maxUnitCount;
			//const int SLOT_COUNT_FOR_CREATE_UNIT = 1;
			//m_MaxPage		= (g_pData->GetMyUser()->GetUnitNum()+SLOT_COUNT_FOR_CREATE_UNIT+UNIT_SELECT_UI_MAX_UNIT_NUM-1) / UNIT_SELECT_UI_MAX_UNIT_NUM;



#ifndef DISABLE_DISAGREE_HACK_USER
			if ( g_pData->GetMyUser()->GetUserData()->hackingUserType == CX2User::HUT_DISAGREE_HACK_USER )
			{
				m_pDLGCheckHack = g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2(250,300), 
					GET_STRING( STR_ID_729 ), 
					SSSUCM_HACK_USER_PROCESS_CHECK_OK, this, SSSUCM_HACK_USER_PROCESS_CHECK_CANCEL );
			}
//#if 0 // 접속시 핵의심 유저라면 무조건 메일을 날리므로 제거
			else if ( g_pData->GetMyUser()->GetUserData()->hackingUserType == CX2User::HUT_AGREE_HACK_USER )
			{
#ifndef PROCESSLIST
				g_pMain->UpdateProcessList();
#endif
				g_pMain->SendHackMail();
			}
#endif DISABLE_DISAGREE_HACK_USER

			//{{ kimhc // 2010-12-01 // 웹 아바타 이미지 등록
#ifdef	AVATAR_IMAGE_TEST
			g_pData->GetMyUser()->SetNexonSN( kEvent.m_uiNexonSN );
#endif	AVATAR_IMAGE_TEST
			//}} kimhc // 2010-12-01 // 웹 아바타 이미지 등록




			g_pMain->GetGameOption()->SetPlayGuide( kEvent.m_kAccountInfo.m_kAccountOption.m_bPlayGuide, false );

#ifndef OPEN_TEST_1_NO_MESSENGER_CASHSHOP
#ifndef NEW_MESSENGER
			if( false == pUserData->m_bIsGuestUser ) // 체험 아이디 제한
			{
				if( false == g_pMain->GetNexonLoginMessenger() )
				{


					if ( g_pMain->GetNexonLogin() == false )				// 사내접속이면, 즉, ID, password 직접 입력하고 로그인한 경우
					{
						if( false == g_pMain->GetNexonLoginMessenger() )
						{
							NMLoginAuthReplyCode resultAuth = CNMCOClientObject::GetInstance().LoginAuth( g_pInstanceData->GetUserID().c_str(), g_pInstanceData->GetUserPassword().c_str() );
							
							switch( resultAuth )
							{
							case kLoginAuth_OK:
								{
									UINT32 supportFlag = 0;

									supportFlag |= kSupportFlag_ChatSession;
									supportFlag |= kSupportFlag_AddFriendConfirm;
									supportFlag |= kSupportFlag_RequestSession;
									supportFlag |= kSupportFlag_ServerMessage;
									if ( CNMCOClientObject::GetInstance().LoginMessenger( kSupportFlag_All  ) == FALSE )
									{
										g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2( 250, 300), GET_STRING( STR_ID_730 ), this );
									}
									else
									{
										g_pMain->SetNexonLoginMessenger( true );
									}
								}
								break;


							case kLoginAuth_WrongID:
								{
									g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_731 ), g_pMain->GetNowState() );
								}
								break;

							case kLoginAuth_WrongPassword:
								{
									g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_732 ), g_pMain->GetNowState() );
								}
								break;

							case kLoginAuth_ServiceShutdown:
								{
									g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_733 ), g_pMain->GetNowState() );
								}
								break;

							default:
								{
									g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_734 ), g_pMain->GetNowState() );
								}
								break;
							}
						}
					}
					else // 서비스, 오픈테섭 버전 접속이면 
					{
						UINT32 supportFlag = 0;

						supportFlag |= kSupportFlag_ChatSession;
						supportFlag |= kSupportFlag_AddFriendConfirm;
						supportFlag |= kSupportFlag_RequestSession;
						supportFlag |= kSupportFlag_ServerMessage;
						if ( CNMCOClientObject::GetInstance().LoginMessenger( kSupportFlag_All  ) == FALSE )
						{
							g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2( 250, 300), GET_STRING( STR_ID_730 ), this );
						}
						else
						{
							g_pMain->SetNexonLoginMessenger( true );
						}
					}
				}
			}
#endif // #ifndef NEW_MESSENGER
#endif OPEN_TEST_1_NO_MESSENGER_CASHSHOP



#ifdef SERV_COUNTRY_PH
#else //SERV_COUNTRY_PH
#ifndef NO_GAMEGUARD
			if ( CheckNPHackUser() == false )
			{
				CloseNPGameMon();
			}

#ifdef ALWAYS_INTERNAL_NPGE_PACKING
			if( KNP_Init( g_pKTDXApp->GetHInstance(), g_pMain->GetClinetArg() ) == false )
#else //ALWAYS_INTERNAL_NPGE_PACKING
			if( KNP_Init( g_pKTDXApp->GetHInstance() ) == false )
#endif ALWAYS_INTERNAL_NPGE_PACKING
			{
				g_pKTDXApp->SetFindHacking( true );
			}

			KNP_SetHwnd( g_pKTDXApp->GetHWND() );

			//접속성공을 하면 nProtect에 아이디를 등록시킨다.
			KNP_Send( g_pInstanceData->GetUserID().c_str() );
			//게임가드 완료
#endif // NO_GAMEGUARD
#endif //SERV_COUNTRY_PH

#ifdef ACTIVE_KOG_GAME_PERFORMANCE_CHECK
			//유저 pc 사양 정보 전송
			_SendSystemInfo();
#endif//ACTIVE_KOG_GAME_PERFORMANCE_CHECK
		}
		else
		{
			StateLog( L"요거" );

			//g_pData->DeleteServerProtocol();
			//g_pData->DeleteMyUser();
#ifdef SERV_CHANGE_NEXON_AUTH_ATL_LEVEL 
			switch ( kEvent.m_iOK )
			{
			case NetError::ERR_VERIFY_00:
			case NetError::ERR_VERIFY_01:
				{
					g_pKTDXApp->NoticeQuitType( CKTDXApp::KQT_DOUBLE_CONNECT );
					g_pKTDXApp->SendGameMessage( XGM_QUIT_GAME, NULL, NULL, false );
				} break;
			case NetError::ERR_VERIFY_24:
				{
					m_pDLGATLLimitCheckMsgBox = g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(270,350), GET_STRING( STR_ID_17745 ), g_pMain->GetNowState(), SSSUCM_ATL_LIMIT_CHECK_OK );
				} break;
			default:
				{
					g_pKTDXApp->NoticeQuitType( CKTDXApp::KQT_VERIFY_ACCOUNT );
					g_pKTDXApp->SendGameMessage( XGM_QUIT_GAME, NULL, NULL, false );
				} break;
			}
#else
			if ( kEvent.m_iOK == NetError::ERR_VERIFY_00 ||
				kEvent.m_iOK == NetError::ERR_VERIFY_01 )
			{
				g_pKTDXApp->NoticeQuitType( CKTDXApp::KQT_DOUBLE_CONNECT );
			}
			else
				g_pKTDXApp->NoticeQuitType( CKTDXApp::KQT_VERIFY_ACCOUNT );

#ifdef LOGIN_STATE_NOEXIT_WHEN_DISCONNECTED
			m_bSentEGS_CONNECT_REQ = false;
			m_bEGS_VERIFY_ACCOUNT_REQ = false;

			switch( kEvent.m_iOK )
			{
			case NetError::ERR_CONNECT_04:
				break;
			default:
				g_pKTDXApp->SendGameMessage( XGM_QUIT_GAME, NULL, NULL, false );
				break;
			}
#else
			g_pKTDXApp->SendGameMessage( XGM_QUIT_GAME, NULL, NULL, false );
#endif LOGIN_STATE_NOEXIT_WHEN_DISCONNECTED
#endif //SERV_CHANGE_NEXON_AUTH_ATL_LEVEL 

		}
	
	}

#ifdef _SERVICE_ 
	ELSWORD_VIRTUALIZER_END
#endif


	return true;
}

bool CX2StateServerSelect::Handler_ENX_USER_LOGIN_NOT( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KENX_USER_LOGIN_NOT kEvent;
	DeSerialize( pBuff, &kEvent );

	m_bENX_USER_LOGIN_NOT = true;
	m_bIsGameBang = kEvent.m_bIsGameBang;
#ifdef SERV_PC_BANG_TYPE
	// 서버에서 받은 pc방 정보를 클라에 저장해 둡니다.
	if ( NULL != g_pData && NULL != g_pData->GetPremiumBuffManager() )
		g_pData->GetPremiumBuffManager()->SetPcBangReward( kEvent.m_kPcBangReward );
#endif SERV_PC_BANG_TYPE

#ifdef COME_BACK_REWARD
	if ( NULL != g_pData && NULL != g_pData->GetPremiumBuffManager() )
		g_pData->GetPremiumBuffManager()->SetPremiumPC( m_bIsGameBang );
#endif

	return true;
}

bool CX2StateServerSelect::Handler_KXPT_PORT_CHECK_REQ()
{
	m_bPortCheckWait			= true;
	m_fPortCheckWaitTime		= 0.5f;

	KXPT_PORT_CHECK_REQ kXPT_PORT_CHECK_REQ;
	kXPT_PORT_CHECK_REQ.m_UserUID = g_pData->GetMyUser()->GetUID();
	//{{ 2013. 1. 9	박세훈	Merge 공인IP 연결 실패시 내부IP로 시도( 박진웅 )
//#ifdef SERV_KTDX_RETRY_USING_INTERNAL_IP
#ifdef  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
    kXPT_PORT_CHECK_REQ.m_InternalIPAddress = g_pData->GetGameUDP()->GetMyIPAddress();
#else   SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
	kXPT_PORT_CHECK_REQ.m_wstrInternalIP = g_pData->GetGameUDP()->GetMyIP();
#endif  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
	kXPT_PORT_CHECK_REQ.m_usInternalPort = g_pData->GetGameUDP()->GetMyPort();
//#endif SERV_KTDX_RETRY_USING_INTERNAL_IP
	//}}

#ifdef  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
    return g_pData->GetGameUDP()->SendToIP( CKTDNUDP::ConvertIPToAddress( g_pMain->GetConnectedGameServerIP() ), g_pMain->GetServerUDPPort(), XPT_PORT_CHECK_REQ, 
        &kXPT_PORT_CHECK_REQ, sizeof(kXPT_PORT_CHECK_REQ) );
#else   SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
	KSerBuffer buff;
	Serialize( &buff, &kXPT_PORT_CHECK_REQ );
	return g_pData->GetGameUDP()->Send( g_pMain->GetConnectedGameServerIP(), g_pMain->GetServerUDPPort(), XPT_PORT_CHECK_REQ, (char*)buff.GetData(), buff.GetLength() );
	//return g_pData->GetGameUDP()->Send( g_pMain->GetServerIP(), g_pMain->GetServerUDPPort(), XPT_PORT_CHECK_REQ, (char*)buff.GetData(), buff.GetLength() );
#endif  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
}

bool CX2StateServerSelect::Handler_KXPT_PORT_CHECK_ACK( const KXPT_PORT_CHECK_ACK& kXPT_PORT_CHECK_ACK )
{
	dbg::clog << L"MY IP : " 
#ifdef  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
        << CKTDNUDP::ConvertAddressToIP( kXPT_PORT_CHECK_ACK.m_IPAddress )
#else   SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
        << kXPT_PORT_CHECK_ACK.m_IP.c_str() 
#endif  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
        << dbg::endl;
	dbg::clog << L"MY Port : " << g_pMain->GetGameP2PPort() << dbg::endl;
	dbg::clog << L"MY Ext Port : " << kXPT_PORT_CHECK_ACK.m_Port << dbg::endl;

	m_bPortCheckWait			= false;
	m_fPortCheckWaitTime		= 0.0f;
	m_PortCheckRetryTime		= 0;

	g_pData->GetGameUDP()->SetMyExtPort( kXPT_PORT_CHECK_ACK.m_Port );

	Handler_EGS_STATE_CHANGE_SERVER_SELECT_REQ();
	return true;
}



/*virtual*/ bool CX2StateServerSelect::Handler_EGS_STATE_CHANGE_SERVER_SELECT_REQ()
{
	return CX2State::Handler_EGS_STATE_CHANGE_SERVER_SELECT_REQ();
}

/*virtual*/ bool CX2StateServerSelect::Handler_EGS_STATE_CHANGE_SERVER_SELECT_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;	
	KEGS_STATE_CHANGE_SERVER_SELECT_ACK kEvent;
	DeSerialize( pBuff, &kEvent );
	
	if( g_pMain->DeleteServerPacket( EGS_STATE_CHANGE_SERVER_SELECT_ACK ) == true )
	{
		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{
			Handler_EGS_CURRENT_TIME_REQ();
			return true;
		}
	}
	return true;
}



bool CX2StateServerSelect::Handler_EGS_CURRENT_TIME_REQ()
{
	g_pData->GetServerProtocol()->SendID( EGS_CURRENT_TIME_REQ );
	g_pMain->AddServerPacket( EGS_CURRENT_TIME_ACK );

	return true;
}

bool CX2StateServerSelect::Handler_EGS_CURRENT_TIME_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_CURRENT_TIME_ACK kEvent;
	DeSerialize( pBuff, &kEvent );
	//CX2PacketLog::PrintLog( &kEvent );

	if( g_pMain->DeleteServerPacket( EGS_CURRENT_TIME_ACK ) == true )
	{
		g_pData->SetServerCurrentTime( kEvent.m_wstrCurrentTime );
#ifdef SERVER_CURRENT_TIME_INT64
		g_pData->SetServerCurrentTimeInt64(kEvent.m_tCurrentTime);
#endif //SERVER_CURRENT_TIME_INT64
		//StateChangeLoading();

		Handler_EGS_SELECT_SERVER_SET_REQ();

		return true;
	}

	return false;
}


bool CX2StateServerSelect::LoginErrorCheck()
{

	//if( m_UserID.size() < 3 )
	//{
	//	//ID를 4자 미만으로 입력한 경우
	//	g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), L"아이디를 4자 이상 넣으세요.", this );
	//	return false;
	//}
	//if( m_UserPassword.size() < 4 )
	//{
	//	//Password를 4자 미만으로 입력한 경우
	//	g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), L"패스워드를 4자 이상 넣으세요.", this );
	//	return false;
	//}


	return true;
}



bool CX2StateServerSelect::Handler_ECH_GET_CHANNEL_LIST_REQ()
{
	g_pData->GetServerProtocol()->SendChID( ECH_GET_CHANNEL_LIST_REQ );
	g_pMain->AddServerPacket( ECH_GET_CHANNEL_LIST_ACK, 100.0f );

	return true;
}


bool CX2StateServerSelect::Handler_ECH_GET_CHANNEL_LIST_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;	
	KECH_GET_CHANNEL_LIST_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	if( g_pMain->DeleteServerPacket( ECH_GET_CHANNEL_LIST_ACK ) == true )
	{
		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{
			// nothing;
		}
	}
	return true;
}

bool CX2StateServerSelect::Handler_ECH_GET_CHANNEL_LIST_NOT( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KECH_GET_CHANNEL_LIST_NOT kEvent;
	DeSerialize( pBuff, &kEvent );

	m_bReceivedChannelList = true;

#ifdef EXTEND_SERVER_GROUP_MASK
	g_pInstanceData->SetServerGroupID( kEvent.m_iServerGroupID );
#else
	g_pInstanceData->SetServerGroupID( (SERVER_GROUP_ID)kEvent.m_iServerGroupID );
#endif EXTEND_SERVER_GROUP_MASK

#ifdef SERVER_GROUP_UI_ADVANCED
	UpdateServerSelectButton( g_pInstanceData->GetServerGroupID() );
#endif SERVER_GROUP_UI_ADVANCED

#ifdef CHANGE_CHANNEL
	g_pInstanceData->m_vecChannelInfo.resize(0);
#else
	m_vecChannelInfo.resize( 0 );
#endif

	std::map< int, KChannelInfo >::const_iterator cit;
	for( cit = kEvent.m_mapChannelList.begin() ; cit != kEvent.m_mapChannelList.end(); cit++ )
	{
		const KChannelInfo& channelInfo = cit->second;

#ifdef CHANGE_CHANNEL
		if(g_pInstanceData != NULL)
			g_pInstanceData->m_vecChannelInfo.push_back( channelInfo );
#else
		m_vecChannelInfo.push_back( channelInfo );
#endif
	}

#ifdef SERV_CHANNEL_LIST_RENEWAL

	g_pInstanceData->m_mapChannelBonusInfo.clear();

	std::map< int, KChannelBonusInfo >::const_iterator cmit;
	for( cmit = kEvent.m_mapChannelBonusList.begin() ; cmit != kEvent.m_mapChannelBonusList.end(); ++cmit )
	{
		//const KChannelBonusInfo& ChannelBonusInfo = cmit->second;
		if(g_pInstanceData != NULL)
			g_pInstanceData->m_mapChannelBonusInfo.insert(std::make_pair(cmit->first, cmit->second));
	}
#endif //SERV_CHANNEL_LIST_RENEWAL

	//std::sort( m_vecChannelInfo.begin(), m_vecChannelInfo.end(), LessChannelIndex() );

	//{{ kimhc // 2009-12-15 // 서버군 추가 작업
#ifdef	ADD_SERVER_GROUP
    if ( g_pInstanceData->GetMiniMapUI() != NULL )
	    g_pInstanceData->GetMiniMapUI()->SetChannelComboBox();
	m_bReservedConnectionToChannelServer = false;
#endif	ADD_SERVER_GROUP
	//}}  kimhc // 2009-12-15 // 서버군 추가 작업

	// 채널 리스트 UI를 Update한다
	UpdateChannelListUI();

	return true;
}


bool CX2StateServerSelect::Handler_ECH_DISCONNECT_REQ()
{

	if( 
#ifdef  X2OPTIMIZE_X2SERVERPROTOCOL_MULTITHREAD_CRASH_BUG_FIX
            g_pData->GetServerProtocol() != NULL
#else   X2OPTIMIZE_X2SERVERPROTOCOL_MULTITHREAD_CRASH_BUG_FIX
             g_pData->GetServerProtocol()->GetChUserProxy() != NULL 
#endif  X2OPTIMIZE_X2SERVERPROTOCOL_MULTITHREAD_CRASH_BUG_FIX
        )
	{
#ifdef  X2OPTIMIZE_X2SERVERPROTOCOL_MULTITHREAD_CRASH_BUG_FIX
        g_pData->GetServerProtocol()->ChUserProxy_SetIntendedDisconnect( true );
#else   X2OPTIMIZE_X2SERVERPROTOCOL_MULTITHREAD_CRASH_BUG_FIX
		g_pData->GetServerProtocol()->GetChUserProxy()->SetIntendedDisconnect( true );
#endif  X2OPTIMIZE_X2SERVERPROTOCOL_MULTITHREAD_CRASH_BUG_FIX
	}


	g_pData->GetServerProtocol()->SendChID( ECH_DISCONNECT_REQ );
	g_pMain->AddServerPacket( ECH_DISCONNECT_ACK, 100.0f );


	return true;
}


bool CX2StateServerSelect::Handler_ECH_DISCONNECT_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( g_pMain->DeleteServerPacket( ECH_DISCONNECT_ACK ) == true )
	{
		g_pData->GetServerProtocol()->DisconnectFromChannelServer();
	}

	if( true == m_bReservedConnectionToChannelServer )
	{
		ConnectToChannelServer();
	}

	return true;
}


bool CX2StateServerSelect::Handler_EGS_DISCONNECT_FOR_SERVER_SELECT_REQ()
{

	if( 
#ifdef  X2OPTIMIZE_X2SERVERPROTOCOL_MULTITHREAD_CRASH_BUG_FIX
        g_pData->GetServerProtocol() != NULL
#else   X2OPTIMIZE_X2SERVERPROTOCOL_MULTITHREAD_CRASH_BUG_FIX
		g_pData->GetServerProtocol()->GetUserProxy() != NULL
#endif  X2OPTIMIZE_X2SERVERPROTOCOL_MULTITHREAD_CRASH_BUG_FIX
        )
	{
#ifdef  X2OPTIMIZE_X2SERVERPROTOCOL_MULTITHREAD_CRASH_BUG_FIX
        g_pData->GetServerProtocol()->UserProxy_SetIntendedDisconnect( true );
#else   X2OPTIMIZE_X2SERVERPROTOCOL_MULTITHREAD_CRASH_BUG_FIX
		g_pData->GetServerProtocol()->GetUserProxy()->SetIntendedDisconnect( true );
#endif  X2OPTIMIZE_X2SERVERPROTOCOL_MULTITHREAD_CRASH_BUG_FIX
	}

	m_bWaiting_EGS_DISCONNECT_FOR_SERVER_SELECT_ACK = true;
	
	g_pData->GetServerProtocol()->SendID( EGS_DISCONNECT_FOR_SERVER_SELECT_REQ );
	g_pMain->AddServerPacket( EGS_DISCONNECT_FOR_SERVER_SELECT_ACK, 100.0f );


	return true;
}


bool CX2StateServerSelect::Handler_EGS_DISCONNECT_FOR_SERVER_SELECT_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( g_pMain->DeleteServerPacket( EGS_DISCONNECT_FOR_SERVER_SELECT_ACK ) == true )
	{
		m_bWaiting_EGS_DISCONNECT_FOR_SERVER_SELECT_ACK = false;

		g_pData->GetServerProtocol()->DisconnectFromGameServer();


		g_pMain->SetConnectedGameServerIP( L"" );
		g_pMain->SetConnectedGameServerPort( 0 );


	}
	return true;
}







bool CX2StateServerSelect::Handler_EGS_GET_MY_MESSENGER_SN_REQ( UidType unitUID )
{
	KEGS_GET_MY_MESSENGER_SN_REQ kPacket;
	kPacket = unitUID;

	g_pData->GetServerProtocol()->SendPacket( EGS_GET_MY_MESSENGER_SN_REQ, kPacket );
	g_pMain->AddServerPacket( EGS_GET_MY_MESSENGER_SN_ACK );
	SAFE_DELETE_DIALOG( m_pDLGMsgBox );
	m_pDLGMsgBox = g_pMain->KTDGUIMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_2558 ), this );

	return true;
}

bool CX2StateServerSelect::Handler_EGS_GET_MY_MESSENGER_SN_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;	
	KEGS_GET_MY_MESSENGER_SN_ACK kEvent;
	DeSerialize( pBuff, &kEvent );


	SAFE_DELETE_DIALOG( m_pDLGMsgBox );

	if( g_pMain->DeleteServerPacket( EGS_GET_MY_MESSENGER_SN_ACK ) == true )
	{
		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{
			if( true == m_bRemoveUnitAfterReceiveNMSerialNum )
			{
				m_bRemoveUnitAfterReceiveNMSerialNum = false;

#ifndef OPEN_TEST_1_NO_MESSENGER_CASHSHOP
#ifndef NEW_MESSENGER
				NMVirtualKey nmVirtualKey;
				nmVirtualKey.uGameCode = NEXON_KOREA_ELSWORD_GAMECODE;
				nmVirtualKey.uVirtualIDCode = kEvent.m_uiKNMSerialNum;

				if( true == g_pMain->GetNexonLoginMessenger() )
				{
					if( 0 == kEvent.m_uiKNMSerialNum )	// nmserial number가 없는 경우에는 바로 지운다~
					{
						// 2009.05.13.hoons. 바로삭제 조치가 변경되어 팝업으로 메세지만 전달
						//Handler_EGS_DELETE_UNIT_REQ();
						g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_2559 ), this );
						// hoons
					}
					else
					{
						if( CNMCOClientObject::GetInstance().RemoveMyVirtualUser( nmVirtualKey ) == TRUE )
						{
							Handler_EGS_DELETE_UNIT_REQ();
						}
						else
						{
							g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_2560 ), this );
						}
					}
				}
				else
				{
					Handler_EGS_DELETE_UNIT_REQ();
				}
#endif // #ifndef NEW_MESSENGER
#else OPEN_TEST_1_NO_MESSENGER_CASHSHOP
#ifdef NEW_MESSENGER
				Handler_EGS_DELETE_UNIT_REQ();
#endif
#endif OPEN_TEST_1_NO_MESSENGER_CASHSHOP

			}
		}
		else
		{
			if( true == m_bRemoveUnitAfterReceiveNMSerialNum )
			{
				m_bRemoveUnitAfterReceiveNMSerialNum = false;
				g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250, 300), GET_STRING( STR_ID_2561 ), this );
			}
		}
		return true;
	}

	return false;
}



bool CX2StateServerSelect::ConnectToChannelServer()
{
	ASSERT( NULL != g_pData->GetServerProtocol() );

#ifdef SERVER_GROUP_UI_ADVANCED
	if( g_pData->GetServerProtocol()->IsChConnected() == true )
	{
		g_pData->GetServerProtocol()->DisconnectFromChannelServer();
	}

	if( g_pData->GetServerProtocol()->ConnectedToChannelServer( g_pMain->GetPickedChannelServer().m_kServerIP.c_str(), g_pMain->GetPickedChannelServer().m_usMasterPort ) == true )
#else
#ifdef RECONNECT_CHANNEL_SERVER
	if( g_pData->GetServerProtocol()->IsChConnected() == true )
	{
		g_pData->GetServerProtocol()->DisconnectFromChannelServer();
	}
#endif RECONNECT_CHANNEL_SERVER
	if( g_pData->GetServerProtocol()->ConnectedToChannelServer( g_pMain->GetPickedChannelServerIP().c_str(), g_pMain->GetChannelServerPort() ) == true )
#endif SERVER_GROUP_UI_ADVANCED
	{
		//{{ 09.08. 태완 : 서버-클라 접속시 패킷 변경.
//#ifdef SERV_KOG_OTP_VERIFY
		return Handler_ECH_VERIFY_ACCOUNT_REQ();
// #else
// 		return Handler_ECH_GET_CHANNEL_LIST_REQ();
// #endif
		//}}
	}
	else
	{
		g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_735 ), this );	

		g_pKTDXApp->NoticeQuitType( CKTDXApp::KQT_CONNECT_FAILED );									// fix!! id 변경
		g_pKTDXApp->SendGameMessage( XGM_QUIT_GAME, NULL, NULL, false );
	}	

	return false;
}

bool CX2StateServerSelect::Handler_ECH_VERIFY_ACCOUNT_REQ()
{
	KECH_VERIFY_ACCOUNT_REQ kPacket;
#ifndef CLIENT_PURPLE_MODULE
	kPacket.m_wstrID = g_pInstanceData->GetUserID();
	kPacket.m_wstrPassword = g_pInstanceData->GetUserPassword();
#else // CLIENT_PURPLE_MODULE
	
	bool bUsePurpleModule = true;
#ifdef CLIENT_PURPLE_MODULE_IN_HOUSE_AUTH
	if(g_pInstanceData->GetInHouseAccount())
		bUsePurpleModule = false;
#endif // CLIENT_PURPLE_MODULE_IN_HOUSE_AUTH

	if(bUsePurpleModule)
	{
		ConvertCharToWCHAR( kPacket.m_wstrID, PurpleGetUserID() );
#ifdef SERV_KOG_OTP_VERIFY
		if (g_pInstanceData->IsIDVerified())
			kPacket.m_wstrPassword = g_pInstanceData->GetUserPassword();
		else
			ConvertCharToWCHAR( kPacket.m_wstrPassword, PurpleGetAuthString() );
#else //SERV_KOG_OTP_VERIFY
		ConvertCharToWCHAR( kPacket.m_wstrPassword, PurpleGetAuthString() );
#endif //SERV_KOG_OTP_VERIFY
	}
	else
	{
		kPacket.m_wstrID = g_pInstanceData->GetUserID();
		kPacket.m_wstrPassword = g_pInstanceData->GetUserPassword();
	}
#endif // CLIENT_PURPLE_MODULE

#ifdef SERV_KOG_OTP_VERIFY
	kPacket.m_bLogin = !(g_pInstanceData->IsIDVerified());
	kPacket.m_bIDVerified = g_pInstanceData->IsIDVerified();
#else SERV_KOG_OTP_VERIFY
    kPacket.m_bLogin = true;
#endif SERV_KOG_OTP_VERIFY
#ifdef SERV_MACHINE_ID_CHECK_BEFOR_LOADING
#ifdef MACHINE_ID
	if( g_pMain != NULL )
		kPacket.m_strMachineID = g_pMain->GetMachineId();
#endif
#endif
#ifdef SERV_COUNTRY_PH
#ifdef _SERVICE_
	if (g_pInstanceData != NULL)
	{
		kPacket.m_strUserToken = g_pInstanceData->GetAuthToken();
	}
#endif //_SERVICE_
#endif //SERV_COUNTRY_PH

#ifdef SERV_STEAM
	kPacket.m_iChannelingCode = g_pInstanceData->GetChannelingCode();
#endif //SERV_STEAM

	g_pData->GetServerProtocol()->SendChPacket( ECH_VERIFY_ACCOUNT_REQ, kPacket );
	g_pMain->AddServerPacket( ECH_VERIFY_ACCOUNT_ACK );
	
	return true;
}

bool CX2StateServerSelect::Handler_ECH_VERIFY_ACCOUNT_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;	
	KECH_VERIFY_ACCOUNT_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	if( g_pMain->DeleteServerPacket( ECH_VERIFY_ACCOUNT_ACK ) == true )
	{
		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{			
#ifdef SERV_KOG_OTP_VERIFY
			// 최초 Gash 인증 후로는 서버에서 발급받은 Pass를 계속 이용하게 되므로(게임서버 접속도), 아예 변경해 버린다.
			g_pInstanceData->SetIDVerified(true);
			g_pInstanceData->SetUserPassword( kEvent.m_wstrPassport );	
#endif SERV_KOG_OTP_VERIFY

	//{{김준환 서버시간 받아오기
#ifndef SERV_SERVER_TIME_GET
#ifdef SERV_MASSFILE_MAPPING_FUNCTION
			g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->SetServerCurrentTime( kEvent.m_wstrCurrentTime );
			g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->MassFileMapping();
#endif SERV_MASSFILE_MAPPING_FUNCTION
#endif SERV_SERVER_TIME_GET

#ifdef SERVER_GROUP_UI_ADVANCED
			Handler_ECH_GET_SERVERGROUP_LIST_REQ();
			Handler_ECH_GET_CHANNEL_LIST_REQ();

			return true;
#else
			return Handler_ECH_GET_CHANNEL_LIST_REQ();
#endif SERVER_GROUP_UI_ADVANCED
		}
		else
		{			
#ifdef SERV_KOG_OTP_VERIFY
			switch( kEvent.m_iOK )
			{
			case NetError::ERR_VERIFY_20:
				{
					g_pKTDXApp->NoticeQuitType( CKTDXApp::KQT_CONNECT_FAILED );
					g_pKTDXApp->SendGameMessage( XGM_QUIT_GAME, NULL, NULL, false );
				} break;
			default:
				{

				} break;
			}
#endif SERV_KOG_OTP_VERIFY
		}
	}

	return false;
}



#ifdef SERV_SECOND_SECURITY
bool CX2StateServerSelect::Handler_EGS_SECOND_SECURITY_INFO_NOT( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_SECOND_SECURITY_INFO_NOT kEvent;
	DeSerialize( pBuff, &kEvent );

	if( kEvent.m_iOK == NetError::ERR_SECOND_SECURITY_AUTH_FAILED_LIMIT )
	{
		g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_12352 ), g_pMain->GetNowState(), CX2State::SUCM_EXIT );
	}
	else if ( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
	{	
		if(kEvent.m_bUseSecondPW == true)
		{
			g_pMain->GetSecurityPad()->SetShow(true, CX2SecurityPad::SPS_GAME_START);
			g_pMain->GetSecurityPad()->SetUseSecondPW(true);
		}
		else
		{
			m_bEnableUnitSelectBySecondSecurity = true;
		}
	}
	return true;
}
#endif SERV_SECOND_SECURITY

#ifdef SERV_FIRST_SELECT_UNIT_REWARD_SYSTEM
bool CX2StateServerSelect::Handler_EGS_EXIST_FIRST_SELECT_UNIT_REWARD_NOT( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if( NULL == g_pMain)
		return false;
	
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_EXIST_FIRST_SELECT_UNIT_REWARD_NOT kEvent;
	DeSerialize( pBuff, &kEvent );
	
	switch( kEvent)
	{
	case KFirstSelectUnitReward::FSUR_SYSTEM_ERROR:
		g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250, 300), GET_STRING( STR_ID_22430 ), NULL);
		return true;
	case KFirstSelectUnitReward::FSUR_WEB_EVENT:
		g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250, 300), GET_STRING( STR_ID_22429 ), NULL);
		return true;
	default:
		return true;
	}

	return true;
}
#endif //SERV_FIRST_SELECT_UNIT_REWARD_SYSTEM

#ifdef SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE
bool CX2StateServerSelect::Handler_EGS_CHECK_THE_ARCHUANGEL_S_COIN_EVENT_LETTER_REQ()
{
	g_pData->GetServerProtocol()->SendID( EGS_CHECK_THE_ARCHUANGEL_S_COIN_EVENT_LETTER_REQ );
	g_pMain->AddServerPacket( EGS_CHECK_THE_ARCHUANGEL_S_COIN_EVENT_LETTER_ACK );

	return true;
}

bool CX2StateServerSelect::Handler_EGS_CHECK_THE_ARCHUANGEL_S_COIN_EVENT_LETTER_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KDBE_CHECK_THE_ARCHUANGEL_S_COIN_EVENT_LETTER_ACK kEvent;
	DeSerialize( pBuff, &kEvent );
	
	
	if( g_pMain->DeleteServerPacket( EGS_CHECK_THE_ARCHUANGEL_S_COIN_EVENT_LETTER_ACK ) == true )
	{
		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{
			if( false == kEvent.m_bReceiveTheLetter )
			{
				if( NULL == m_pDLGEventGuide )
				{
					m_pDLGEventGuide = new CKTDGUIDialog( g_pMain->GetNowState(), L"DLG_UI_Event_Guide.lua" );
					g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGEventGuide );
					m_pDLGEventGuide->SetShowEnable(true,true);
				}
				else
				{
					m_pDLGEventGuide->SetShowEnable(true,true);
				}
			}
		}
	}
	return true;
}
#endif //SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE


/*virtual*/ bool CX2StateServerSelect::LastMsgByESC()
{
#ifdef SERV_SECOND_SECURITY
	// oasis907 : 김상윤 [2011.6.8] 보안패드 활성시 서버 선택창에서 ESC키 막기
	if(g_pMain->GetSecurityPad() != NULL &&
		g_pMain->GetSecurityPad()->GetShow() == true)
	{
		return true;
	}
#endif SERV_SECOND_SECURITY


	if( true == CX2StateServerSelect::m_sbSelectedServerSet )
	{
		OpenLastMsgPopUp( GET_STRING( STR_ID_736 ) );
	}
	else
	{
		return QuitGame();
	}

	return true;
}

/*virtual*/ void CX2StateServerSelect::LastMsgPopOk()
{
	if ( m_pDLGLastMsgPopUp != NULL )
		g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGLastMsgPopUp, NULL, false );
	m_pDLGLastMsgPopUp = NULL;

	g_pKTDXApp->SendGameMessage( CKTDXApp::KM_UI_CONTROL_CUSTOM_EVENT, SSSUCM_UNIT_SELECT_EXIT, NULL, false );
}

void CX2StateServerSelect::UnitSelectExit()
{
	ClearBeforeUnit( m_pSelectUnit );
	ClearUnitButton();
#ifdef SERVER_GROUP_UI_ADVANCED
	if(m_pDLGServerGroupSelectBack != NULL)
		m_pDLGServerGroupSelectBack->SetShowEnable(true, true);
#endif SERVER_GROUP_UI_ADVANCED
	m_pDLGServerSelectBack->SetShowEnable(true, true);
	m_pDLGUnitSelectFront->SetShowEnable(false, false);

	//{{ kimhc // 2009-12-15 // 서버군 추가 작업에 따른 채널 다이얼로그 분리
#ifdef	ADD_SERVER_GROUP
	m_pDLGChannelSelectBack->SetShowEnable( true, true );
#endif	ADD_SERVER_GROUP
	//}}  kimhc // 2009-12-15 // 서버군 추가 작업에 따른 채널 다이얼로그 분리

	m_pSelectUnit = NULL;
#ifdef SERV_UNIT_WAIT_DELETE
	m_pBeforeSelectUnit = NULL;
#endif //SERV_UNIT_WAIT_DELETE	
	m_sbSelectedServerSet = false;

	m_bENX_USER_LOGIN_NOT		= false;
	m_bEGS_VERIFY_ACCOUNT_REQ	= false;
	m_bEGS_VERIFY_ACCOUNT_ACK	= false;
	m_bPortCheckReq				= false;

	m_bSentEGS_CONNECT_REQ 		= false;
	m_bReceivedChannelList 		= false;

	if( NULL != g_pData->GetServerProtocol() )
	{
		g_pData->GetServerProtocol()->SendTRID( ETR_UNIT_DISCONNECT );
	}
	// 게임서버와의 접속을 끊는다
	Handler_EGS_DISCONNECT_FOR_SERVER_SELECT_REQ();

	// 채널서버에 접속한다.
	ConnectToChannelServer();  

#ifdef SERV_SECOND_SECURITY
	if(g_pMain->GetSecurityPad() != NULL &&
		g_pMain->GetSecurityPad()->GetShow() == true)
	{
		g_pMain->GetSecurityPad()->SetShow(false);
	}
#endif SERV_SECOND_SECURITY
	g_pKTDXApp->SkipFrame();

}

//{{ kimhc // 2009-12-15 // 이전에 플레이 했던 서버군 읽기
#ifdef	ADD_SERVER_GROUP
bool CX2StateServerSelect::OpenScriptServerGroupFile()
{
	string			strFileName;
#ifdef EXTEND_SERVER_GROUP_MASK
	int iServerGroupID = 0;
#else
	SERVER_GROUP_ID eServerGroupID	= SGI_INVALID;
#endif EXTEND_SERVER_GROUP_MASK
	bool			bParsingOK		= false;

	ConvertWCHARToChar( strFileName, g_pData->GetSavedServerGroupFileName() );

	ConvertFileAnsiToUTF8( strFileName, strFileName );

	KLuaManager luaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState(), 0, true );

	KGCMassFileManager::CMassFile::MASSFILE_MEMBERFILEINFO_POINTER Info;
	Info = g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LoadDataFile( g_pData->GetSavedServerGroupFileName() );
	if( Info != NULL )
	{
		if( true == g_pKTDXApp->GetDeviceManager()->LoadLuaTinker( g_pData->GetSavedServerGroupFileName().c_str(), false ) )
		{
			if( true == g_pKTDXApp->GetDeviceManager()->LoadLuaManager( &luaManager, g_pData->GetSavedServerGroupFileName().c_str(), false ) )
			{
#ifdef EXTEND_SERVER_GROUP_MASK //SERVER_GROUP_UI_ADVANCED 도 관련
				LUA_GET_VALUE( luaManager, L"SERVER_GROUP", 				iServerGroupID,			0);
#else
				LUA_GET_VALUE_ENUM( luaManager, L"SERVER_GROUP", 			eServerGroupID,			SERVER_GROUP_ID,		SGI_INVALID	);
#endif EXTEND_SERVER_GROUP_MASK //SERVER_GROUP_UI_ADVANCED 도 관련
			}
		}
	}

#ifdef EXTEND_SERVER_GROUP_MASK
	g_pInstanceData->SetServerGroupID( iServerGroupID );
	return true;
#else
	switch ( eServerGroupID )
	{
	case SGI_SOLES:
	case SGI_GAIA:
		{
			g_pInstanceData->SetServerGroupID( eServerGroupID );
			bParsingOK = true;
		}
		break;

	default:
		{
#ifdef RANDOM_SERVER
			if( g_pMain->GetDefaultChannelServerIPIndex() == SGI_INVALID )
			{
				g_pInstanceData->SetServerGroupID( static_cast<SERVER_GROUP_ID>( (rand() % 2) ) );
			}
			else
			{
				g_pInstanceData->SetServerGroupID( static_cast<SERVER_GROUP_ID>( g_pMain->GetDefaultChannelServerIPIndex() ) );
			}
#else
			g_pInstanceData->SetServerGroupID( static_cast<SERVER_GROUP_ID>( g_pMain->GetDefaultChannelServerIPIndex() ) );
#endif
		}
		break;

	}

	return bParsingOK;
#endif EXTEND_SERVER_GROUP_MASK
}

//{{ 2012.02.20 조효진	캐릭터 삭제 프로세스 변경 (삭제 대기 기간 도입)
#ifdef SERV_UNIT_WAIT_DELETE
bool CX2StateServerSelect::Handler_EGS_FINAL_DELETE_UNIT_REQ()
{
	if( m_pFinalDeleteUnitInfo == NULL)
		return true;

	KEGS_FINAL_DELETE_UNIT_REQ kPacket;
	kPacket.m_iUnitUID		= m_pFinalDeleteUnitInfo->GetUID();

	g_pData->GetServerProtocol()->SendPacket( EGS_FINAL_DELETE_UNIT_REQ, kPacket );
	g_pMain->AddServerPacket( EGS_FINAL_DELETE_UNIT_ACK  );
	SAFE_DELETE_DIALOG( m_pDLGMsgBox );
	m_pDLGMsgBox = g_pMain->KTDGUIMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_16104 ), this );

	return true;
}

bool CX2StateServerSelect::Handler_EGS_FINAL_DELETE_UNIT_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_FINAL_DELETE_UNIT_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	SAFE_DELETE_DIALOG( m_pDLGMsgBox );
	if( g_pMain->DeleteServerPacket( EGS_FINAL_DELETE_UNIT_ACK ) == true )
	{
		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{			
			CX2Unit* pUnit = g_pData->GetMyUser()->GetUnitByUID( kEvent.m_iUnitUID );
			if ( pUnit == NULL )
			{
				g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_713 ), this );
				return true;
			}

			ClearBeforeUnit( pUnit );
			ClearUnitButton();
			g_pData->GetMyUser()->DeleteUnitByUID(kEvent.m_iUnitUID);
			g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_16108 ), this );
			CreateUnitButton();
		}
	}
	return true;
}

bool CX2StateServerSelect::Handler_EGS_RESTORE_UNIT_REQ()
{
	if( m_pRestoreUnitInfo == NULL )
		return true;

	KEGS_RESTORE_UNIT_REQ kPacket;
	kPacket.m_iUnitUID		= m_pRestoreUnitInfo->GetUID();

	g_pData->GetServerProtocol()->SendPacket( EGS_RESTORE_UNIT_REQ, kPacket );
	g_pMain->AddServerPacket( EGS_RESTORE_UNIT_ACK  );
	SAFE_DELETE_DIALOG( m_pDLGMsgBox );
	m_pDLGMsgBox = g_pMain->KTDGUIMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_16105 ), this );

	return true;
}

bool CX2StateServerSelect::Handler_EGS_RESTORE_UNIT_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_RESTORE_UNIT_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	SAFE_DELETE_DIALOG( m_pDLGMsgBox );
	if( g_pMain->DeleteServerPacket( EGS_RESTORE_UNIT_ACK ) == true )
	{
		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{			
			CX2Unit* pUnit = g_pData->GetMyUser()->GetUnitByUID( kEvent.m_iUnitUID );
			if ( pUnit == NULL )
			{
				g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_713 ), this );
				return true;
			}
			//{{ 2012.03.05 lygan_조성욱 // 조효진이 작업 누락된부분 추가 작업
			ClearBeforeUnit( pUnit );
			ClearUnitButton();
			CX2Unit::UnitData* pUnitData = pUnit->GetUnitData();
			pUnitData->m_bDeleted = false;
			g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_16111 ), this );
			CreateUnitButton();

		}
		else if(kEvent.m_iOK == NetError::ERR_RESTORE_UNIT_02)
		{
			CX2Unit* pUnit = g_pData->GetMyUser()->GetUnitByUID( kEvent.m_iUnitUID );
			if ( pUnit == NULL )
			{
				g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_713 ), this );
				return true;
			}

			//{{ //2012.03.06 lygan_조성욱 // 삭제후 1일지나서 복구 가능한 날짜 알림용
			std::wstring buffer = L"";
			std::wstring year = L"";
			std::wstring month = L"";
			std::wstring day = L"";
			std::wstring hour = L"";
			std::wstring min = L"";

			CTime tRestoreAbleDate(kEvent.m_tRestoreAbleDate);

			year = (CStringW)( tRestoreAbleDate.Format(_T( "%Y" )) );
			month = (CStringW)( tRestoreAbleDate.Format(_T( "%m" )) );
			day = (CStringW)( tRestoreAbleDate.Format(_T( "%d" )) );
			hour = (CStringW)( tRestoreAbleDate.Format(_T( "%H" )) );
			min = (CStringW)( tRestoreAbleDate.Format(_T( "%M" )) );

			buffer =  GET_REPLACED_STRING( ( STR_ID_16113, "SSSSS", day.c_str(), month.c_str(), year.c_str(),hour.c_str(), min.c_str()));
			g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), buffer.c_str(), this );

			CX2Unit::UnitData* pUnitData = pUnit->GetUnitData();
			pUnitData->m_trRestoreAbleDate = kEvent.m_tRestoreAbleDate;


			//}}
		}
		//}}
	}
	return true;
}
#endif SERV_UNIT_WAIT_DELETE
//}}

#ifdef SERVER_GROUP_UI_ADVANCED
bool CX2StateServerSelect::Handler_ECH_GET_SERVERGROUP_LIST_REQ()
{
	g_pData->GetServerProtocol()->SendChID( ECH_GET_SERVERGROUP_LIST_REQ );
	g_pMain->AddServerPacket( ECH_GET_SERVERGROUP_LIST_ACK, 100.0f );

	return true;
}

bool CX2StateServerSelect::Handler_ECH_GET_SERVERGROUP_LIST_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;	
	KECH_GET_CHANNEL_LIST_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	if( g_pMain->DeleteServerPacket( ECH_GET_SERVERGROUP_LIST_ACK ) == true )
	{
		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{
			// nothing;
		}
	}
	return true;
}

bool CX2StateServerSelect::Handler_ECH_GET_SERVERGROUP_LIST_NOT( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KECH_GET_SERVERGROUP_LIST_NOT kEvent;
	DeSerialize( pBuff, &kEvent );

	m_bReceivedServerGroupList = true;


#ifdef CRASH_PROBE_LOG
	CrashLog( "{E46CACFD-654A-4c77-9FD7-FABCBA0E703C}" );
#endif CRASH_PROBE_LOG

	g_pInstanceData->m_mapServerGroupInfo.clear();

	g_pInstanceData->m_mapServerGroupInfo = kEvent.m_mapServerGroupList;

	// 채널 리스트 UI를 Update한다
	UpdateServerGroupListUI();


#ifdef CRASH_PROBE_LOG
	CrashLog( "{A45C0907-23CF-41d1-89BF-0D9F3594DA74}" );
#endif CRASH_PROBE_LOG

	return true;
}

bool CompareServerGroup(const KServerGroupInfo& a, const KServerGroupInfo& b)
{
	return a.m_iOrder < b.m_iOrder;
}

void CX2StateServerSelect::UpdateServerGroupListUI()
{
	const D3DXVECTOR2 posDLG(20, 60);
	const D3DXVECTOR2 posButton(18, 18);

	// 생성
	SAFE_DELETE_DIALOG( m_pDLGServerGroupSelectBack );

    m_pDLGServerGroupSelectBack =  new CKTDGUIDialog( this, L"DLG_UI_ServerGroup_Selection.lua" );

	if ( m_pDLGServerGroupSelectBack == NULL )
		return;

	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGServerGroupSelectBack );
	m_pDLGServerGroupSelectBack->SetPos( posDLG );



	// 업데이트
	int iServerGroupSize = 0;
	vector<KServerGroupInfo> vecSortedServerGroup;
	map<int, KServerGroupInfo>::iterator iterEnable = g_pInstanceData->m_mapServerGroupInfo.begin();
	while(iterEnable != g_pInstanceData->m_mapServerGroupInfo.end())
	{
		//if(iterEnable->second.m_bEnable == true)
		{
			++iServerGroupSize;
			vecSortedServerGroup.push_back(iterEnable->second);
		}

		++iterEnable;
	}
	std::sort( vecSortedServerGroup.begin(), vecSortedServerGroup.end(), CompareServerGroup);

	int iMaxServergroup = min(iServerGroupSize, MAX_SERVERGROUP_SIZE);
	int i = 0;

	// 배경
	const float button_height = 61.f-1.f;
	const float ADD_Y = button_height*iMaxServergroup - 13;

	CKTDGUIStatic* pStaticBG = (CKTDGUIStatic*)m_pDLGServerGroupSelectBack->GetControl(L"BG");
	if(pStaticBG)
	{
		CKTDGUIStatic::CPictureData* pPictureMiddle = pStaticBG->GetPicture(1);
		CKTDGUIStatic::CPictureData* pPictureBottom = pStaticBG->GetPicture(2);

		if(pPictureMiddle != NULL && pPictureMiddle->pPoint != NULL)
		{
			pPictureMiddle->pPoint->addSize.y = ADD_Y;
			pPictureMiddle->pPoint->SetAutoPointByTextureSize();
		}
		if(pPictureBottom != NULL && pPictureBottom->pPoint != NULL)
		{
			D3DXVECTOR2 pos = pPictureBottom->GetPos();
			pos.y += ADD_Y;
			pPictureBottom->SetPos( pos );
		}
	}

	// 버튼
	int button_index = 0;
	vector<KServerGroupInfo>::iterator iter = vecSortedServerGroup.begin();
	for(; iter != vecSortedServerGroup.end(); ++iter)
	{
		ASSERT(button_index < MAX_SERVERGROUP_SIZE);

		if(i >= iMaxServergroup)
			break;

		//if(false == iter->second.m_bEnable)
		//	continue;

		boost::wformat fmtButtonName(L"server_button_%1%");
		CKTDGUIButton* pButton = (CKTDGUIButton*)m_pDLGServerGroupSelectBack->GetControl( boost::str(fmtButtonName % button_index).c_str() );
		if(pButton)
		{
			pButton->AddDummyInt(iter->m_iServerGroupUID);
			pButton->SetShow(true);
		}

		boost::wformat fmtButtonCaption(L"server_name_%1%");
		CKTDGUIStatic* pStaticName = (CKTDGUIStatic*)m_pDLGServerGroupSelectBack->GetControl( boost::str(fmtButtonCaption % button_index).c_str() );
		if(pStaticName)
		{
			pStaticName->SetShow(true);

			CKTDGUIControl::UIStringData* pStringName = pStaticName->GetString(0);
			if(pStringName != NULL)
				pStringName->msg = iter->m_wstrServerGroupName;
			CKTDGUIControl::UIStringData* pStringShadow = pStaticName->GetString(1);
			if(pStringShadow != NULL)
				pStringShadow->msg = iter->m_wstrServerGroupName;
			CKTDGUIControl::UIStringData* pStringPostfix = pStaticName->GetString(2);
			if(pStringPostfix != NULL)
			{
				CKTDGFontManager::CUKFont* pFont = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetUKFont( pStringName->fontIndex );
				if(pFont)
				{
					int width = pFont->GetWidth( pStringName->msg.c_str() );
					pStringPostfix->pos.x += width + 10;
				}
			}
		}


		++button_index;
	}

	UpdateServerSelectButton( g_pMain->GetPickedChannelServerIPIndex() );
}
#endif SERVER_GROUP_UI_ADVANCED

#ifdef SERVER_GROUP_UI_ADVANCED
void CX2StateServerSelect::UpdateServerSelectButton( int iID )
{
	if(m_pDLGServerGroupSelectBack != NULL)
	{
		boost::wformat fmtButtonName(L"server_button_%1%");
		for(int i=0; i<MAX_SERVERGROUP_SIZE; ++i)
		{
			CKTDGUIButton* pServerButton = (CKTDGUIButton*)m_pDLGServerGroupSelectBack->GetControl( boost::str(fmtButtonName % i).c_str() );
			if(pServerButton)
			{
				int iServerGroup = pServerButton->GetDummyInt(0);
				if(iServerGroup == iID)
				{
					pServerButton->SetDownStateAtNormal(true);
					//m_iChannelNum = pServerButton->GetDummyInt( 1 );		// 채널 갯수
				}
				else
					pServerButton->SetDownStateAtNormal(false);
			}
		}
#ifdef SERVER_SELECT_INT		
		m_iChannelNum = 24;
#endif // SERVER_SELECT_INT		
		g_pMain->SetPickedChannelServerIPIndex( iID );
	}
}
#else SERVER_GROUP_UI_ADVANCED
void CX2StateServerSelect::UpdateServerSelectButton( SERVER_GROUP_ID eID )
{
	CKTDGUIButton *pServerButtonSoles		= static_cast< CKTDGUIButton* >( m_pDLGServerSelectBack->GetControl( L"sever_select_01" ) ); // 솔레스
	CKTDGUIButton *pServerButtonGaia		= static_cast< CKTDGUIButton* >( m_pDLGServerSelectBack->GetControl( L"sever_select_02" ) );	// 신서버
	

	if ( pServerButtonSoles		== NULL ||
		 pServerButtonGaia	== NULL )
		 return;

	// 모두 다 false로 초기화
	pServerButtonSoles->SetDownStateAtNormal( false );		// 솔레스
	pServerButtonGaia->SetDownStateAtNormal( false );		// 신서버

	switch ( eID )
	{
	case SGI_GAIA:
		{
			pServerButtonGaia->SetDownStateAtNormal(true);
			m_iChannelNum	= pServerButtonGaia->GetDummyInt( 1 );		// 채널 갯수
			//g_pMain->m_iPickedChannelServerIPIndex = 1; // 임시
		}
		break;

	case SGI_SOLES:
	default:
		{
			pServerButtonSoles->SetDownStateAtNormal(true);
			m_iChannelNum	= pServerButtonSoles->GetDummyInt( 1 );		// 채널 갯수
			//g_pMain->m_iPickedChannelServerIPIndex = 0; // 임시
		}
		break;		
	}

	g_pMain->SetPickedChannelServerIPIndex( static_cast< int >( eID ) );
	
}
#endif SERVER_GROUP_UI_ADVANCED
#endif	ADD_SERVER_GROUP
#endif ELSWORD_NEW_BEGINNING
//}}  kimhc // 2009-12-15 // 이전에 플레이 했던 서버군 읽기


#ifdef REFORM_UI_KEYPAD
bool CX2StateServerSelect::Handler_EGS_KEYBOARD_MAPPING_INFO_NOT( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_KEYBOARD_MAPPING_INFO_NOT kEvent;
	DeSerialize( pBuff, &kEvent );

	m_bRecivedKeyMap = true;

	if ( NULL != g_pKTDXApp->GetDIManager() && NULL != g_pKTDXApp->GetDIManager()->Getkeyboard()
		 && true == g_pMain->IsValidPacket( kEvent.m_iOK ) )
	{
		int iTemp = GAMEACTION_END;
		if ( kEvent.m_kKeyboardMappingInfo.m_mapKeyboardMappingInfo.empty() )
		{
			g_pKTDXApp->GetDIManager()->SetDefaultMap();

			if ( NULL != g_pMain && NULL != g_pMain->GetKeyPad() )
			{
				g_pMain->GetKeyPad()->LoadSlotData();
				g_pMain->GetKeyPad()->SaveSlotData( true );

				if ( NULL != g_pData && NULL != g_pData->GetUIManager() && NULL != g_pData->GetUIManager()->GetUIQuickSlot() )
				{
					g_pData->GetUIManager()->GetUIQuickSlot()->ResetQuickSlotUI();
				}
			}
		}
		else if ( GAMEACTION_END == kEvent.m_kKeyboardMappingInfo.m_mapKeyboardMappingInfo.size() )
		{
			SettingUserKeyboardMappingInfo( kEvent.m_kKeyboardMappingInfo );
		}
		// 사이즈가 틀리면 매칭이 안됨 디폴트 키로 세팅됨
		else 
		{
			if ( NULL != g_pMain && NULL != g_pMain->GetKeyPad() )
				g_pMain->GetKeyPad()->InitKeySlotData();

			SettingUserKeyboardMappingInfo( kEvent.m_kKeyboardMappingInfo );
		}
			
	}
	
	return true;
}

void CX2StateServerSelect::SettingUserKeyboardMappingInfo( const KKeyboardMappingInfo& kKeyboardMappingInfo_ )
{
	std::map< short, short >::const_iterator constIter;

	for ( constIter = kKeyboardMappingInfo_.m_mapKeyboardMappingInfo.begin() ;
		constIter != kKeyboardMappingInfo_.m_mapKeyboardMappingInfo.end(); ++constIter )
	{
		// DB에 저장된 Key Setting 값에 맞게 키보드 값을 설정함
		if ( constIter->second < CX2KeyPad::USE_KEY_NUMBER )
			g_pKTDXApp->GetDIManager()->Getkeyboard()->SetAction( GAME_ACTION( constIter->first ), CX2KeyPad::USE_DIK_LIST[constIter->second] );
		else
			g_pKTDXApp->GetDIManager()->Getkeyboard()->SetAction( GAME_ACTION( constIter->first ), 0x00 );
	}

	if ( NULL != g_pMain && NULL != g_pMain->GetKeyPad() )
	{
		g_pMain->GetKeyPad()->LoadSlotData();

		if ( NULL != g_pData && NULL != g_pData->GetUIManager() && NULL != g_pData->GetUIManager()->GetUIQuickSlot() )
		{
			g_pData->GetUIManager()->GetUIQuickSlot()->ResetQuickSlotUI();
		}
	}
}

#endif

bool CX2StateServerSelect::Handler_EGS_CHAT_OPTION_INFO_NOT( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_CHAT_OPTION_INFO_NOT kEvent;
	DeSerialize( pBuff, &kEvent );

	if( true == g_pMain->IsValidPacket( kEvent.m_iOK ) && NULL != g_pChatBox )
	{
		g_pChatBox->SetOptionData(kEvent.m_mapChatOptionSetInfo);			
	}
	return true;
}
#ifdef MODIFY_ADVERTISEMENT // 적용날짜: 2013-03-28
void CX2StateServerSelect::AddMeshAdvertisement_Back()
{
	if( NULL == m_pDLGUnitSelectBack )
		return;

	CKTDGXMeshPlayer::CXMeshInstance* pMeshInst = 
		g_pData->GetUIMajorXMeshPlayer()->CreateInstance( NULL,  L"MeshAdvertisement_Back",  0.f, 0.f, 0.f, 0,0,0, 0,0,0 );
	
	if( NULL != pMeshInst )
	{
		pMeshInst->SetOverUI(true);
		g_pKTDXApp->GetDGManager()->RemoveObjectChain(pMeshInst);
		m_pDLGUnitSelectBack->SetHasUnit( pMeshInst );
		m_hMeshAddAdvertisement_Back = pMeshInst->GetHandle();
	}
}
void CX2StateServerSelect::AddMeshAdvertisement_Front()
{
	if( NULL == m_pDLGUnitSelectFront )
		return;

	CKTDGXMeshPlayer::CXMeshInstance* pMeshInst = 
		g_pData->GetUIMajorXMeshPlayer()->CreateInstance( NULL,  L"MeshAdvertisement_Front", 0.f, 0.f, 0.f, 0,0,0, 0,0,0 );

	if( NULL != pMeshInst )
	{
		pMeshInst->SetOverUI(true);
		g_pKTDXApp->GetDGManager()->RemoveObjectChain(pMeshInst);

		m_pDLGUnitSelectFront->SetHasUnit( pMeshInst );
		m_hMeshAddAdvertisement_Back = pMeshInst->GetHandle();
	}
}
#endif // MODIFY_ADVERTISEMENT

#ifdef ACTIVE_KOG_GAME_PERFORMANCE_CHECK
void CX2StateServerSelect::_SendSystemInfo()
{
	//서버로 전송
	KEGS_SYSTEM_INFO kPacket;
	kPacket.m_iAccountUID = g_pData->GetMyUser()->GetUserData()->userUID;
	kPacket.m_iSystemKey = g_pMain->GetSystemInfo()->GetMacAddress();
	kPacket.m_wstrCPU = g_pMain->GetSystemInfo()->GetCpuString();
	kPacket.m_wstrMemory = g_pMain->GetSystemInfo()->GetMemString();
	kPacket.m_wstrGPU = g_pMain->GetSystemInfo()->GetDisplayString(DXUTGetD3DObject());
	kPacket.m_wstrOS = g_pMain->GetSystemInfo()->GetOsString();

	//
	g_pData->GetServerProtocol()->SendPacket( EGS_SYSTEM_INFO, kPacket );
}
#endif//ACTIVE_KOG_GAME_PERFORMANCE_CHECK

#ifdef SERV_CLIENT_DIRECT_CONNECT_AUTH_PCBANG
bool CX2StateServerSelect::Handler_EGS_CLIENT_DIRECT_CONNECT_AUTH_PCBAG_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_CLIENT_DIRECT_CONNECT_AUTH_PCBAG_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	m_bIsGameBang = kEvent.m_bCheckClinetDirectPcBang;

#ifdef SERV_PC_BANG_TYPE
	// 서버에서 받은 pc방 정보를 클라에 저장해 둡니다.
	if ( NULL != g_pData && NULL != g_pData->GetPremiumBuffManager() )
	{
		g_pData->GetPremiumBuffManager()->SetPremiumPC(kEvent.m_bCheckClinetDirectPcBang);
		g_pData->GetPremiumBuffManager()->SetPcBangReward( kEvent.m_kPcBangReward );
	}
#endif SERV_PC_BANG_TYPE
	
	return true;
}
#endif //SERV_CLIENT_DIRECT_CONNECT_AUTH_PCBANG

#ifdef SHOW_ONLY_ADVERTISEMENT
/** @function : SetShowServerSelectUI
	@brief : 서버 선택창 UI의 Show 상태를 변경하는 함수
	@param : True = 보기, false = 숨기기
*/
void CX2StateServerSelect::SetShowServerSelectUI( bool bVal_ )
{
	m_bHideServerSelectUI = !bVal_;

	if( NULL != m_pDLGUnitSelectBack )
	{
		CKTDGUIStatic* pStatic = static_cast<CKTDGUIStatic*>(m_pDLGUnitSelectBack->GetControl(L"Background"));
		if( NULL != pStatic )
		{
			pStatic->SetShow( bVal_ );
		}
	}

	if( NULL != m_pDLGChannelSelectBack )
		m_pDLGChannelSelectBack->SetShow( bVal_ );

	if( NULL != m_pDLGServerSelectBack )
		m_pDLGServerSelectBack->SetShow( bVal_ );

}
#endif // SHOW_ONLY_ADVERTISEMENT

#ifdef ADDED_EVENT_JUMPING_CHARACTER	// 김종훈, 여름방학 이벤트 점핑 캐릭터
	// 점핑 캐릭터 팝업창 띄우라고 서버에서 주는 not
bool CX2StateServerSelect::Handler_EGS_JUMPING_CHARACTER_GUIDE_NOT ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	
	// 전직 선택 가능 알림창, 없다면 다시 불러오고 있다면 SetShow True 로 설정
	if ( NULL != m_pDLGInfoCreateJumpingChar )
	{
		m_pDLGInfoCreateJumpingChar->SetShowEnable(true, true);
		m_pDLGInfoCreateJumpingChar->SetFront( true );
	}
	else	
	{
		m_pDLGInfoCreateJumpingChar = new CKTDGUIDialog( this, L"DLG_UI_JUMPING_EVENT_01.lua" );	
		g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGInfoCreateJumpingChar );
		m_pDLGInfoCreateJumpingChar->SetShowEnable(true, true);
	}	

	return true;
}

#endif // ADDED_EVENT_JUMPING_CHARACTER	// 김종훈, 여름방학 이벤트 점핑 캐릭터
