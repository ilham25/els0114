#include "GSUser.h"
#include <dbg/dbg.hpp>
#include "GSFSM.h"
#include "ProxyManager.h"
#include "DBLayer.h"
#include "NetError.h"
#include "GSSimLayer.h"

#include "ActorManager.h"
#include "ExpTable.h"
#include "SpiritTable.h"
#include "StatTable.h"
#include "BaseServer.h"
#include "Socket/NetCommon.h"

// �ؽ� ����
#include "NexonBillingTCPManager.h"

#include "X2Data/XSLSquareUnit.h"
#include "X2Data/XSLResolveItemManager.h"
#include "GameSysVal/GameSysVal.h"
#include "GameEvent/GameEventManager.h"
#include "RecommendUserTable.h"
#include "TutorialManager.h"
#include "RoomListManager.h"
#include "ChannelManager.h"
//{{ 2008. 12. 10  ������	�κ��׽�Ʈ
#ifdef SERV_ROBOT_TEST
	#include "RobotTestManager.h"
#endif SERV_ROBOT_TEST
//}}

//{{ 2011. 12. 05	������	��Ʋ�ʵ�
#ifdef SERV_BATTLE_FIELD_SYSTEM
	#include "NewPartyListManager.h"
	#include "X2Data/XSLBattleFieldManager.h"
	#include "socket/NetCommon.h"
#else
	#include "PartyListManager.h"
#endif SERV_BATTLE_FIELD_SYSTEM
//}}

//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER
	#include "GSWorldMissionManager.h"
#endif SERV_INSERT_GLOBAL_SERVER
//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�

//{{ 2011. 05. 27    ��μ�    �޸� ���� ���� ����
#ifdef SERV_COME_BACK_USER_REWARD
	#include "ComebackUserManager.h"
//{{ 2012. 02. 03	�ڼ���	�̺�Ʈ �������� ó����� ���� ( Script -> DB )
#ifdef SERV_CHANGE_EVENT_INFO_SCRIPT_TO_DB
	#include "EventDataRefresh/EventRewardID.h"
	#include "EventDataRefresh/EventRewardLevelUpRefreshManager.h"
#else
	#include "RewardTable.h"
#endif SERV_CHANGE_EVENT_INFO_SCRIPT_TO_DB
//}}
#endif SERV_COME_BACK_USER_REWARD
//}} 

//{{ 2011. 06. 21	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
	#include "X2Data/XSLUnit.h"
#endif SERV_PVP_NEW_SYSTEM
//}}
//{{ 2011. 10. 13	������	������ ������
#ifdef SERV_CONTENT_MANAGER
	#include "GSContentManager.h"
#endif SERV_CONTENT_MANAGER
//}}

//{{ 2011.10.14     ��μ�    ��� ����� Ư�� IP ������ ��� ����
#ifdef SERV_USE_GM_CHEAT_RESTRICTED_IP
	#include "CompanyIPTable.h"
#endif SERV_USE_GM_CHEAT_RESTRICTED_IP
//}}

//{{ 2011. 11. 21  ��μ�	���� ���� ������
#ifdef SERV_UNIT_CLASS_CHANGE_ITEM
	#include "ClassChangeTable.h"
#endif SERV_UNIT_CLASS_CHANGE_ITEM
//}}
//{{ 2011. 12. 14	������	��Ŷ ó�� ���� �ð� üũ
#ifdef SERV_PACKET_PROCESSING_LATENCY_CHECK
	#include "PacketMornitoring/PacketMornitoring.h"
#endif SERV_PACKET_PROCESSING_LATENCY_CHECK
//}}

//{{ 2012. 03. 29	��μ�		x2.exe ��� ���� �˻�
#ifdef SERV_CHECK_X2_EXE_MODULE
	#include "Mornitoring/MornitoringManager.h"
#endif SERV_CHECK_X2_EXE_MODULE
//}}

//{{ 2012. 04. 17	�ڼ���	����� �� ����
#ifdef SERV_REFORM_THE_GATE_OF_DARKNESS
	#include "X2Data\XSLItemManager.h"
	#include "X2Data\XSLBuffManager.h"
#endif SERV_REFORM_THE_GATE_OF_DARKNESS
//}}

//{{ 2012. 07. 12	������		���� ť�� ���� ������ �ý���
#ifdef SERV_OPEN_RANDOM_CUBE_DELAY_SYSTEM
	#include "EventDelaySystemManager.h"
#endif SERV_OPEN_RANDOM_CUBE_DELAY_SYSTEM
//}}

//{{ 2012. 09. 26	������		���� ���� ���� ����
#ifdef SERV_POST_COPY_BUG_FIX
	#include "SMSPhoneNumberManager.h"
#endif SERV_POST_COPY_BUG_FIX
//}}

#ifdef SERV_CHANNELING_USER_MANAGER
#include "ChannelingUserManager.h"
#endif SERV_CHANNELING_USER_MANAGER

//{{ 2013. 3. 12	�ڼ���	 ���� ��ŷ �ý���
#ifdef SERV_LOCAL_RANKING_SYSTEM
	#include "GSLocalRankingManager.h"
#endif SERV_LOCAL_RANKING_SYSTEM
//}}

#ifdef SERV_GLOBAL_BILLING
#include "BillManager.h"
#endif // SERV_GLOBAL_BILLING

//{{ 2012. 11. 28 ĳ�� ������ ���� �ɼ� ��� ���� - ��μ�
#ifdef SERV_CASH_ITEM_SOCKET_OPTION
#include "X2Data/XSLSocketItem.h"
#endif SERV_CASH_ITEM_SOCKET_OPTION
//}
#ifdef SERV_EVENT_RIDING_WITH_SUB_QUEST
#include "UserRidingPetManager.h"
#include "X2Data/XSLRidingPetManager.h"
#endif //SERV_EVENT_RIDING_WITH_SUB_QUEST

//{{ 2012. 09. 06	��ȫ��	�۷ι� �̼� �Ŵ���
#ifdef SERV_GLOBAL_MISSION_MANAGER
#include "GSGlobalMissionManager.h"
#endif SERV_GLOBAL_MISSION_MANAGER
//}} 2012. 09. 06	��ȫ��	�۷ι� �̼� �Ŵ���

#include "NetLayer.h"
#include "odbc/Odbc.h"
#include "Enum/Enum.h"

//////////////////////////////////////////////////////////////////////////
#ifdef SERV_GSUSER_CPP
#pragma NOTE( "GSUserFuntion.cpp ���� ������ �˴ϴ�!" )
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//{{ 2011. 04. 04	������	deserializing ���� �α�
#define _CASE_CHECK_ERR( id, packet ) \
case id: \
{ \
	KSerializer ks; \
	ks.BeginReading( &spEvent_->m_kbuff ); \
	packet kPacket; \
	if( !ks.Get(kPacket) ) \
	{ \
		START_LOG( cerr, L"deserialze failed." L#id L" - " L#packet ) \
			<< BUILD_LOG( GetName() ) \
			<< BUILD_LOG( GetCharName() ); \
		m_kUserHackingManager.IncreaseDeserializeFailCount( id ); \
	} \
	else \
	{ \
		ON_##id( spEvent_->m_anTrace, kPacket); \
		spEvent_->m_kbuff.Reset(); \
	} \
} \
break

#define  CASE_CHECK_ERR_NOPARAM(id)           case id: ON_##id( spEvent_->m_anTrace ); break 
#define  CASE_CHECK_ERR(id)                  _CASE_CHECK_ERR(id, K##id)
//}}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//{{ 2011. 06. 08	������	KEvent ��ó ��� ���
#ifdef SERV_KEVENT_FROM
#define _CASE_CHECK_ERR_AND_FILTER( id, packet ) \
case id: \
{ \
	KSerializer ks; \
	ks.BeginReading( &spEvent_->m_kbuff ); \
	packet kPacket; \
	std::wstring wstrPacketName = L#packet; \
	if( CheckIsValidPacketEvent( spEvent_, wstrPacketName.c_str(), ks.Get(kPacket), spEvent_->IsValidEventID() ) ) \
	{ \
		ON_##id( spEvent_->m_anTrace, kPacket); \
		spEvent_->m_kbuff.Reset(); \
	} \
} \
break

#define  CASE_CHECK_ERR_AND_FILTER_NOPARAM(id) \
case id: \
{ \
	if( CheckIsValidPacketEvent( spEvent_, L"", true, spEvent_->IsValidEventID() ) ) \
	{ \
		ON_##id( spEvent_->m_anTrace ); \
	} \
} \
break

#define  CASE_CHECK_ERR_AND_FILTER(id)                  _CASE_CHECK_ERR_AND_FILTER(id, K##id)
#endif SERV_KEVENT_FROM
//}}
//////////////////////////////////////////////////////////////////////////

NiImplementRTTI( KGSUser, KActor );
ImplPfID( KGSUser, PI_GS_USER );

KGSUser::KGSUser(void) 
:
m_uiKNMSerialNum(0),
m_nUnitSlot(0),
//{{ 2012. 02. 02	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
#else
m_nMapID(CXSLMapData::MI_RUBEN),
m_ucLastTouchLineIndex(0), 
m_usLastPosValue(0),
#endif SERV_BATTLE_FIELD_SYSTEM
//}}
m_uiRoomListID(0),
m_iFieldUID(0),
m_iRoomUID(0),
//{{ 2012. 03. 21	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
m_iBeforeRoomUID(0),
#endif SERV_BATTLE_FIELD_SYSTEM
//}}
m_iTradeUID(0),
m_iPersonalShopUID(0)
//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER
, m_iWorldMissionTickCount(0)
#endif SERV_INSERT_GLOBAL_SERVER
//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
//{{ 2012. 10. 29	�ڼ���	�������� �����
#ifdef SERV_ELIOS_INVESTIGATIONS
, m_bEliosInvestigationsReward( true )
#endif SERV_ELIOS_INVESTIGATIONS
//}}
//{{ 2012. 12. 12	�ڼ���	�ܿ� ���� ���� �̺�Ʈ( �ӽ�, �ϵ� �ڵ� )
#ifdef SERV_2012_WINTER_VACATION_EVENT
, m_iWinterVacationEventCount( -1 )
#endif SERV_2012_WINTER_VACATION_EVENT
//}}
#ifdef	SERV_HERO_PVP_MANAGE_LIST // ���볯¥: 2013-04-04
, m_bHeroPvpUser( false )
#endif	// SERV_HERO_PVP_MANAGE_LIST
#ifdef SERV_TIME_EVENT_ONLY_CURRENT_USER_CHAR
, m_bNewUnit( false )
, m_bNewUnit2( false )
, m_bCurrentUnit( false )
#endif //SERV_TIME_EVENT_ONLY_CURRENT_USER_CHAR
#ifdef SERV_SYNC_PACKET_USING_RELAY_WORKINGS_CHECK// �۾���¥: 2013-05-06	// �ڼ���
, m_bCheckUdpKickStateChangeFieldNot( false )
, m_bBlockCheckCountChanged( false )
, m_byteBlockCheckCount( 0 )
#endif // SERV_SYNC_PACKET_USING_RELAY_WORKINGS_CHECK

#ifdef SERV_LOCAL_RANKING_SYSTEM_STATISTICS_LOG// �۾���¥: 2013-05-15	// �ڼ���
, m_usLocalRankingClickCount( 0 )
#endif // SERV_LOCAL_RANKING_SYSTEM_STATISTICS_LOG
//{{ 2012. 09. 06	��ȫ��	�۷ι� �̼� �Ŵ���
#ifdef SERV_GLOBAL_MISSION_MANAGER
, m_iGlobalMissionTickCount(0)
#endif SERV_GLOBAL_MISSION_MANAGER
//}} 2012. 09. 06	��ȫ��	�۷ι� �̼� �Ŵ���
{
	//////////////////////////////////////////////////////////////////////////
	// ���� ���� �ʱ�ȭ
    ResetUnitInfo();

	//////////////////////////////////////////////////////////////////////////
	// �������� �ʱ�ȭ
	//{{ 2011. 08. 09  ��μ� KAccontInfo �߰��� ������
#ifdef SERV_NEW_CREATE_CHAR_EVENT
	m_kAccountInfo.m_iAuthLevel		= SEnum::UAL_NORMAL;
	m_kAccountInfo.m_bInternalUser	= false;
#else
	m_cAuthLevel		= SEnum::UAL_NORMAL;
	m_bInternalUser				= false;
#endif SERV_NEW_CREATE_CHAR_EVENT
	//}}

#ifndef SERV_LUNITCONNECTLOG_BUG_FIX
    m_tConnectTime		= CTime::GetCurrentTime();
#endif SERV_LUNITCONNECTLOG_BUG_FIX

    m_bLoginFail		= true;
    m_bLogoutFail		= true;
    m_kInventory.SetOwner( this );

    m_bReportDisconnectToLogin	= true;

	//nProtect �ѱ����� ��� ��뼳��
    //if( KSimLayer::GetKObj()->CheckCommonFlag( KSimLayer::CommonFlag::CF_NPGG ) )
    //{
    //    m_kGGAuth.SetEnable( true );
    //}
	//{{ 2009. 8. 18  ������	hack shield
	if( KSimLayer::GetKObj()->CheckCommonFlag( KSimLayer::CommonFlag::CF_HSHIELD ) )
	{
        m_kHSAuth.SetEnable( true );
	}
	//}}

#ifdef SERV_USE_XTRAP
	if( KSimLayer::GetKObj()->CheckCommonFlag( KSimLayer::CommonFlag::CF_XTRAP ) )
	{
		m_cXTrapCSM.SetEnable( true	);
		m_cXTrapCSM.InitCSM();
	}
#endif SERV_USE_XTRAP

	// 2011.05.05 lygan_������ //  �߱��� Ŀ�´�Ƽ�� ���� ���� ã��
#ifdef SERV_USER_WATCH_NEW
	m_cCommunity_Tap_type = 0;
#endif //SERV_USER_WATCH_NEW

	//m_kGGAuth.Init();
	m_kRepeatEvent.Reset();

	//{{ 2010. 9. 2	������	���� ��Ŷ ���� ����
#ifdef SERV_REPEAT_FILTER_REFAC
#else	
	InitRepeatEventFilter();
#endif SERV_REPEAT_FILTER_REFAC
	//}}

	InitTransactionEventFilter();


	//{{ 2012. 10. 02	������		ED&������ ����͸� �ý���
#ifdef SERV_ED_AND_ITEM_MORNITORING_SYSTEM
#else
	m_iED.SetMaxValue( SEnum::UI_MAX_ED_CHAR_HAVE );
#endif SERV_ED_AND_ITEM_MORNITORING_SYSTEM
	//}}

	//{{ 2010. 07. 05  ������	���� ����� �Ŵ���
#ifdef SERV_USER_ABUSER_MANAGER
	m_kUserAbuserManager.Clear();
#endif SERV_USER_ABUSER_MANAGER
	//}}
	//{{ 2011. 01. 17	������	ĳ���� ī��Ʈ ����
#ifdef SERV_CHAR_LOG
	m_setMyUnitUIDList.clear();
#endif SERV_CHAR_LOG
	//}}

	//{{ 2011. 05. 02  ��μ�	2�� ���� �ý���
#ifdef SERV_SECOND_SECURITY
	SetIsUseSecondPW( false );
	SetSecondPWFailedCount( 0 );
	SetSuccessSecondSecurity( false );
	SetIsUseOTP( 0 );
	m_iNexonSecurityType = 0;
#endif SERV_SECOND_SECURITY
	//}}

	//{{ 2011. 05. 27    ��μ�    �޸� ���� ���� ����
#ifdef SERV_COME_BACK_USER_REWARD
	m_kComeBackUserInfo.Clear();
#endif SERV_COME_BACK_USER_REWARD
	//}}
	//{{ 2012. 03. 14	��μ�		��Ŷ ������ ī��Ʈ üũ
#ifdef SERV_CHECK_SEQUENCE_COUNT
	m_bCheckSequence = false;
#endif SERV_CHECK_SEQUENCE_COUNT
	//}}

	//{{ 2012. 04. 06	�ڼ���	( ���� ���� ǥ�� )
#ifdef SERV_EVENT_RETURN_USER_MARK_SCRIPT
#else
		//{{ 2012. 03. 27	�ڼ���	�Ƹ����� ���� ������ ���� ����! ( ���� ���� ǥ�� )
	#ifdef SERV_EVENT_RETURN_USER_MARK
		m_bEventMark = false;
		m_bEventReturnUserMark = false;
	#endif SERV_EVENT_RETURN_USER_MARK
		//}}
#endif SERV_EVENT_RETURN_USER_MARK_SCRIPT
	//}}

	//{{ 2012. 03. 27	�ڼ���	�Ƹ����� ���� ������ ���� ����! ( ���� ���� ǥ�� )
#ifdef SERV_EVENT_RETURN_USER_MARK
#else
	//{{ 2012. 05. 16	�ڼ���	ù ���� �� ���̵� ���� ����ֱ�
#ifdef SERV_EVENT_GUIDELINE_POPUP
		m_bPopupTheGuideLine = false;
#endif SERV_EVENT_GUIDELINE_POPUP
	//}}
#endif SERV_EVENT_RETURN_USER_MARK
	//}}
#ifdef SERV_EVENT_MONEY	// ��μ� // ���볯¥: 2013-07-04
		m_iEventMoney = 0;
#endif // SERV_EVENT_MONEY

#ifdef SERV_IDENTITY_CONFIRM_POPUP_MESSAGE
	m_bIdentityConfirmCheck = false;
#endif //SERV_IDENTITY_CONFIRM_POPUP_MESSAGE

#ifdef SERV_GUARANTEE_UNIQUENESS_OF_NAME_CN
	m_bNickNameCheckOnly = true;
	m_iBackUpUnitUID = 0;
#endif //SERV_GUARANTEE_UNIQUENESS_OF_NAME_CN

		//{{ 2012 ���� ����2 ���� ��Ī �̺�Ʈ	- ��μ�
#ifdef SERV_2012_PVP_SEASON2_EVENT
		m_bNowPvpEventTime = false;
		m_bIsPvpEventUser = false;
#endif SERV_2012_PVP_SEASON2_EVENT
		//}}

#ifdef SERV_PARTYPLAY_WITH_DUNGEON_CLEAR_COUNT
		m_bSC1 = false;
		m_bSC2 = false;
#endif
#ifdef SERV_NEW_EVENT_TYPES
		m_iMaxLevelTickCount = 0;
		m_iActiveDungeonTickCount = -1;
#endif //SERV_NEW_EVENT_TYPES

#ifdef SERV_CODE_EVENT
		m_iCodeEventTickCount = 0;
#endif SERV_CODE_EVENT

#ifdef SERV_ACTIVE_KOG_GAME_PERFORMANCE_CHECK
	m_bSendDBSystemInfoStatistics = false;
#endif//SERV_ACTIVE_KOG_GAME_PERFORMANCE_CHECK

#ifdef SERV_COUNTRY_TH
	m_wstrSocektID  = L"";
#endif //SERV_COUNTRY_TH

#ifdef SERV_COUNTRY_PH
	m_usGarenaCyberCafe = 0;
#endif //SERV_COUNTRY_PH

#ifdef SERV_GROW_UP_SOCKET
	m_iExchangeCount = 0;
#endif SERV_GROW_UP_SOCKET

#ifdef SERV_CUSTOM_CONNECT_EVENT
	m_iCustomEventID = 0;
#endif //SERV_CUSTOM_CONNECT_EVENT

#ifdef SERV_STEAM
	m_bSteamClient = false;
#endif //SERV_STEAM

#ifdef SERV_CHANNELING_AERIA
	m_bAeriaClient = false;
#endif //SERV_CHANNELING_AERIA

#ifdef SERV_NEW_YEAR_EVENT_2014
	m_ucOldYearMissionRewardedLevel = 0;
	m_iNewYearMissionStepID = -1;
#endif SERV_NEW_YEAR_EVENT_2014
}

KGSUser::~KGSUser(void)
{
}

ImplToStringW( KGSUser )
{
    return START_TOSTRING_PARENTW( KActor );
}

void KGSUser::Tick()
{
    KActor::Tick();

	
	// note!! ������ ��ų ���� B, ĳ�� ��ų ����Ʈ�� ���������� �Ⱓ�� ǥ���ϴ� ���� UI���� ǥ���ϴ� ������ �ٲ�鼭 ��ų����ü�����޴� ���� �κ��� ���������� �������� �ʴ´�


	// 1. ��ų ���� B�� ���� �Ⱓ üũ
	ExpireSkillSlotB();

	// 2. ĳ�ý�ų ����Ʈ�� ���� �Ⱓ üũ
	ExpireCashSkillPoint();

	// 3. �Ⱓ�� �������� �Ⱓ üũ
    ExpireItem();

	// 4. �ð� �̺�Ʈ�� ���� üũ
	CheckTimeEvent();

	// 5. ���� �������� Ÿ��Ʋ�� ���� �Ⱓ üũ
	ExpireTitle();

	//{{ 2012. 04. 15	�ڼ���	( ���� ���� ǥ�� )
#ifdef SERV_EVENT_RETURN_USER_MARK_SCRIPT
	CheckCriterionEvent();
#endif SERV_EVENT_RETURN_USER_MARK_SCRIPT
	//}}

	//{{ 2009. 10. 14  ������	�ڵ�����
#ifdef AP_RESTONE
	CheckAutoPayment();
#endif AP_RESTONE
	//}}

	//{{ 2011. 12. 22	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	CheckWaitPartyUser();
	CheckAutoPartyBonus();
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}

	//{{ 2010. 02. 16  ������	��ŷ�� ����Ʈ
#ifdef SERV_HACKING_TOOL_LIST
	CheckHackingToolList();
#endif SERV_HACKING_TOOL_LIST
	//}}
	//{{ 2011. 04. 05  ��μ�	���� �� �ŷ� ����
#ifdef SERV_MONITORING_LETTER_AND_TRADE
	CheckLetterAndTradeMonitoring();
#endif SERV_MONITORING_LETTER_AND_TRADE
	//}}

	//{{ 2011. 04. 26	������	deserialize ���� ����
#ifdef SERV_DESERIALIZE_FAIL_DISCONNECT
	CheckDeserializeFailCount();
#endif SERV_DESERIALIZE_FAIL_DISCONNECT
	//}}

	//{{ 2012. 11. 13	������		�ڵ���Ƽ ��⿭ ī��Ʈ ǥ��
#ifdef SERV_AUTO_PARTY_WAIT_USER_COUNT_VIEW
	CheckAutoPartyWaitUserCountRequest();
#endif SERV_AUTO_PARTY_WAIT_USER_COUNT_VIEW
	//}}

    if( !m_pkCurrentState->CheckState( 5, KGSFSM::S_INIT, KGSFSM::S_UID_UPDATED, KGSFSM::S_LOGINED, KGSFSM::S_EXIT, KGSFSM::S_SERVER_SELECT ) )
    {
        if( m_kTimer[TM_DB_UPDATE].elapsed() >= 60.0 * 20.0 )
        {
            WriteUnitInfoToDB( false );
			
			//{{ 2012. 09. 22	�ڼ���	���� �̺�Ʈ
#ifdef SERV_EVENT_BINGO
			BingoEventDBWrite();
#endif SERV_EVENT_BINGO
			//}}
        }
    }

#ifdef SERV_NPROTECT_CS_AUTH_30
	if( KSimLayer::GetKObj()->CheckCommonFlag( KSimLayer::CommonFlag::CF_NPGG ) == true )
	{
		CheckCSAuth3();
	}
#endif SERV_NPROTECT_CS_AUTH_30

#ifdef SERV_USE_XTRAP
	// �����ڴ� XTRAP �������� �˻縦 ���� �ʽ��ϴ�.
	if( GetAuthLevel() < SEnum::UAL_DEVELOPER )
	{
		m_cXTrapCSM.CSMStep1( GetThisPtr<KGSUser>() );
	}
#endif SERV_USE_XTRAP

	//////////////////////////////////////////////////////////////////////////
	//KOREA NPROTECT ����
	//if( m_kGGAuth.Tick( GetThisPtr<KGSUser>() ) == false )
	//{
	//	SetDisconnectReason( KStatistics::eSIColDR_NProtect_Auth_Fail );
	//	ReserveDestroy();

	//	START_LOG( cerr, L"nProtect ���� ����.!" )
	//		<< END_LOG;
	//	return;
	//}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//{{ 2009. 8. 18  ������	hack shield	
	if( m_kHSAuth.Tick( GetThisPtr<KGSUser>() ) == false )
	{
		SetDisconnectReason( KStatistics::eSIColDR_HackShield_Auth_Fail );
		ReserveDestroy();

		START_LOG( cerr, L"hack shield ���� ����.!" )
			<< END_LOG;
		return;
	}
	//}}
	//////////////////////////////////////////////////////////////////////////

	//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER
	CheckUpdateWorldMission();
#endif SERV_INSERT_GLOBAL_SERVER
	//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�

	//{{ 2011. 05. 27    ��μ�    �޸� ���� ���� ����
#ifdef SERV_COME_BACK_USER_REWARD
	CheckComeBackUser();
#endif SERV_COME_BACK_USER_REWARD
	//}} 

	//{{ 2011. 08. 12   ��μ�      ��ϸ� ���� 
#ifdef SERV_NEW_HENIR_TEST
	CheckHenirRewardCountEvent();
#endif SERV_NEW_HENIR_TEST
	//}}

	//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
	CheckBuffPeriod();
#endif SERV_SERVER_BUFF_SYSTEM
	//}}

#ifdef SERV_NEW_UNIT_TRADE_LIMIT
	CheckTradeBlockUnit();
#endif SERV_NEW_UNIT_TRADE_LIMIT

	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
	CheckRelationshipReward();
#endif SERV_RELATIONSHIP_SYSTEM
	//}

    switch( GetStateID() )
	{
    case KGSFSM::S_UID_UPDATED:
		//{{ 2012. 09. 03	������		�ߺ� ���� ���� ����
#ifdef SERV_DUPLICATE_CONNECT_BUG_FIX
		// ���⼭ �ƹ��� ó���� ���� �ʴ´�!
#else
		//if( m_kTimer[TM_UID_UPDATED].elapsed() > 10.0 ) // �ؽ� PC�� ���� ���� Ÿ�� �ƿ� �ð�
		//{
		//	START_LOG_WITH_NAME( cwarn )
		//		<< BUILD_LOG( GetUID() )
		//		<< END_LOG;

		//	StateTransition( KGSFSM::I_TO_LOGINED );
		//	m_bLoginFail = false;

		//	//{{ 2010. 01. 29  ������	PC�� ���ȭ
		//	m_kUserPcBangManager.SetIsPcBang( false );
		//	//}}

		//	KENX_USER_LOGIN_NOT kNot;
		//	kNot.m_bIsGameBang = IsPcBang();
		//	SendPacket( ENX_USER_LOGIN_NOT, kNot );

		//	// last login
		//	SendToAccountDB( DBE_UPDATE_LOGIN_TIME_NOT );

		//	// ���� �ʴ´�. - pc�� ������ ������ �Ǵ��� ������ �ɼ� �ֵ���
		//	//SetDisconnectReason( KStatistics::eSIColDR_PCBang_Auth_Delay );
		//	//ReserveDestroy();
		//	//return;
		//}
#endif SERV_DUPLICATE_CONNECT_BUG_FIX
		//}}        
        break;

	case KGSFSM::S_ROOM:
		break;

    case KGSFSM::S_EXIT:
        KActorManager::GetKObj()->ReserveDelete( m_nUID );    // �ڱ� �ڽ��� ������ ������ �� ����.	
    	break;
    }

	//{{ �ڱ��� : [2010/03/09] //	�߱� �ߵ����� ��ħ�� �ý���
#ifdef SERV_ANTI_ADDICTION_SYSTEM
	m_AntiAddictionManager.Tick();
#endif SERV_ANTI_ADDICTION_SYSTEM
	//}}
	//{{ 2012. 06. 13	��μ�       ������ �˴ٿ���
#ifdef SERV_SELECTIVE_SHUTDOWN_SYSTEM
	CheckRestrictionUser();
#else
		//{{ 2012. 01. 25  ��μ�	������ �˴ٿ��� �ڵ� ���� �˻� ���
	#ifdef SERV_SHUTDOWN_SYSTEM_AUTO_CHECK
	CTime tCurr = CTime::GetCurrentTime();

	if( tCurr.GetHour() == 0 && tCurr.GetMinute() == 0 && tCurr.GetSecond() < 5 )
	{
		if( m_kUserInfoByNexon.IsShutdownUser() >= KUserInfoByNexon::NONE_AUTH )
		{
			// ������ ���� �ʾҰų� �˴ٿ� �����
			// �޽��� ����( �˾�â )
			KEGS_SHUT_DOWN_MESSAGE_NOT kNot;
			kNot.m_iNetErrorID = NetError::ERR_SHUT_DOWN_00;

			SendPacket( EGS_SHUT_DOWN_MESSAGE_NOT, kNot );

			// ���� ���� ����
			SetDisconnectReason( KStatistics::eSIColDR_Policy_Shutdown_Disconnect );
			ReserveDestroy();
		}
	}
	else if( tCurr.GetHour() == 0 && tCurr.GetMinute() == 5 && tCurr.GetSecond() < 5 )
	{
		if( m_kUserInfoByNexon.IsShutdownUser() >= KUserInfoByNexon::NONE_AUTH )
		{
			// ������ ���� �ʾҰų� �˴ٿ� �����
			// �޽��� ����( �˾�â )
			KEGS_SHUT_DOWN_MESSAGE_NOT kNot;
			kNot.m_iNetErrorID = NetError::ERR_SHUT_DOWN_00;

			SendPacket( EGS_SHUT_DOWN_MESSAGE_NOT, kNot );

			// ���� ���� ����
			SetDisconnectReason( KStatistics::eSIColDR_Policy_Shutdown_Disconnect );
			ReserveDestroy();
		}
	}
	#endif SERV_SHUTDOWN_SYSTEM_AUTO_CHECK
		//}}
#endif SERV_SELECTIVE_SHUTDOWN_SYSTEM
	//}}

	//{{ 2012. 04. 02	��μ�		sn ����͸� ��� �и�
#ifdef SERV_SERIAL_NUMBER_MORNITORING
	CheckServerSN_Monitoring();
#endif SERV_SERIAL_NUMBER_MORNITORING
	//}}

	//{{ 2012. 05. 23	��μ�       �Ǹ� �Ұ� ������ �Ǹ� �õ� ��ŷ ������ �Ǵ�
#ifdef SERV_IMPOSSIBLE_SELL_ITEM
	CheckRandomTimeKick();
#endif SERV_IMPOSSIBLE_SELL_ITEM
	//}}

	//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
	{
		char cLastSendType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_NONE;
		if( m_kUserHackingManager.CheckHackingUserRecivedPacket( cLastSendType ) == false )
		{
			START_LOG( cerr, L"��ŷ ���� ��Ŷ�� ���´µ� ������ ����." )
				<< BUILD_LOG( GetCharName() )
				<< BUILD_LOGc( cLastSendType )
				<<END_LOG;

			// Ŭ�󿡰� ��Ŷ�� ����
			SendID( EGS_CHECK_HACK_USER_PACKET_REQUEST_NOT );

			// ű����
			if( SiKGameSysVal()->GetHackingUserKick() == true )
			{
				SetDisconnectReason( KStatistics::eSIColDR_Client_Hacking );
				ReserveDestroy();
			}
		}
	}
#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
	//}}

	//{{ 2012 ���� ����2 ���� ��Ī �̺�Ʈ	- ��μ�
#ifdef SERV_2012_PVP_SEASON2_EVENT
	CheckPvpEvent();
#endif SERV_2012_PVP_SEASON2_EVENT
	//}}

	//{{ 2013. 03. 18	 ������ �̺�Ʈ_���κ��� ���� - ��μ�
#ifdef SERV_APRIL_FOOLS_DAY
	CheckRainbowBuff();
#endif SERV_APRIL_FOOLS_DAY
	//}
	
#ifdef SERV_NEW_EVENT_TYPES
	CheckMaxLevel();
	CheckEnableEventDungeon();
#endif //SERV_NEW_EVENT_TYPES

#ifdef SERV_CODE_EVENT
	CheckEnableCodeEvent();
#endif SERV_CODE_EVENT	
	
#ifdef SERV_ACTIVE_KOG_GAME_PERFORMANCE_CHECK
	_CheckSystemInfoStatistics();
	_CheckKOGGamePerformanceStatistics();
#endif//SERV_ACTIVE_KOG_GAME_PERFORMANCE_CHECK

#ifdef	SERV_RIDING_PET_SYSTM// ���볯¥: 2013-04-21
	m_kAfterWorkStorage.Tick();
#endif	// SERV_RIDING_PET_SYSTM

#ifdef SERV_SYNC_PACKET_USING_RELAY_WORKINGS_CHECK// �۾���¥: 2013-05-06	// �ڼ���
	CheckLanBugOutDisconnect();
#endif // SERV_SYNC_PACKET_USING_RELAY_WORKINGS_CHECK

#ifdef SERV_GATE_OF_DARKNESS_SUPPORT_EVENT
	if( m_kTimer[TM_GATE_OF_DARKNESS_SUPPORT_EVENT_TIMER].elapsed() >= 60.0 )		
	{
		int iElapsedSec = static_cast<int>(m_kTimer[TM_GATE_OF_DARKNESS_SUPPORT_EVENT_TIMER].elapsed());

		if( m_iGateOfDarknessSupportEventTime > 0 )
		{
			if( m_iGateOfDarknessSupportEventTime > iElapsedSec )
			{
				m_iGateOfDarknessSupportEventTime -= iElapsedSec;
			}
			else
			{
				m_iGateOfDarknessSupportEventTime = 0;
			}			
		}

		m_kTimer[TM_GATE_OF_DARKNESS_SUPPORT_EVENT_TIMER].restart();
	}
#endif SERV_GATE_OF_DARKNESS_SUPPORT_EVENT

	//{{ 2012. 09. 06	��ȫ��	�۷ι� �̼� �Ŵ���
#ifdef SERV_GLOBAL_MISSION_MANAGER
	CheckUpdateGlobalMission();
#endif SERV_GLOBAL_MISSION_MANAGER
	//}} 2012. 09. 06	��ȫ��	�۷ι� �̼� �Ŵ���
}

void KGSUser::OnDestroy()
{
#ifdef SERV_CHANNELING_USER_MANAGER
	SiKChannelingUserManager()->UpdateChannelingUserList( false, GetChannelCode() );
#endif SERV_CHANNELING_USER_MANAGER

	//{{ 2011.2.11  ��ȿ��  ����͸��� ���� ��Ʈ üũ �� �����α� ���°� ����
#ifdef SERV_PERMIT_PORT_CHECK
	// ����͸� �� ������ IP������ ������ �Ҽ� ���ٰ� ������ //
	// ����͸� ������ ���� ��Ʈ üũ�� ��� �Ʒ��� �ٸ� ���۵� �� �ʿ����
	if( KBaseServer::GetKObj()->IsMonitoringServerForPortCheck( GetIPStr() ) )
	{
		StateTransition( KGSFSM::I_TO_EXIT );	// 11. ������ü ������ ���� FSM�� S_EXIT�� ��ȯ
		KSession::OnDestroy();
		START_LOG( clog, L"Mornitoring server check Port!" );
		return;
	}
#endif SERV_PERMIT_PORT_CHECK
	//}}

	// 2011.05.05 lygan_������ //  �߱��� Ŀ�´�Ƽ�� ���� ���� ã��
#ifdef SERV_USER_WATCH_NEW
	m_cCommunity_Tap_type = 0;
#endif //SERV_USER_WATCH_NEW

	//{{ �ڱ��� : [2010/03/09] //	�߱� �ߵ����� ��ħ�� �ý���
#ifdef SERV_ANTI_ADDICTION_SYSTEM
	m_AntiAddictionManager.Finalize();
#endif SERV_ANTI_ADDICTION_SYSTEM
	//}}

	// ĳ���� ������ ��ȿ�ϴٸ� ĳ���� ���� ���� �۾��� ��������!
	if( GetCharUID() > 0 )
	{
		//{{ 2012. 11. 22	������		ĳ���� ���� ���� �ڵ� �����丵
		//////////////////////////////////////////////////////////////////////////
//#ifdef SERV_DESELECT_UNIT_CODE_REFACTORING
		//////////////////////////////////////////////////////////////////////////
		// ĳ���� ���� ���� �۾�
		OnDeselectUnit( EGS_CLIENT_QUIT_REQ, true );
		//////////////////////////////////////////////////////////////////////////
//#else
//		//////////////////////////////////////////////////////////////////////////		
//				
//		//{{ 2009. 4. 22  ������	���� �κ� ��Żó��
//		SendLeavePVPLobby();
//		//}}
//
//		//{{ 2011. 07. 11	������	���� ����
//#ifdef SERV_PVP_NEW_SYSTEM
//		SendLeavePVPMatch( NetError::NOT_LEAVE_MATCH_WAIT_REASON_02 );
//#endif SERV_PVP_NEW_SYSTEM
//		//}}
//
//		//{{ 2012. 02. 10	������	��Ʋ�ʵ� �ý���
//#ifdef SERV_BATTLE_FIELD_SYSTEM
//		SendLeaveAutoParty( NetError::NOT_LEAVE_AUTO_PARTY_WAIT_REASON_02 );
//#endif SERV_BATTLE_FIELD_SYSTEM
//		//}}
//
//		// 4. ��Ƽ ��Ż ó��
//		SendLeaveParty( NetError::NOT_LEAVE_ROOM_REASON_01 );
//
//		//////////////////////////////////////////////////////////////////////////
//		// �̹� �ʵ忡 �������ִ�! �ش� �ʵ忡�� ������~!
//		SendLeaveField( NetError::NOT_LEAVE_FIELD_REASON_02 );
//		//////////////////////////////////////////////////////////////////////////
//
//		// 5. ���ΰŷ� ��Ż ó��
//		if( GetTradeUID() != 0 )
//		{
//			KEGS_BREAK_TRADE_REQ kPacket;
//			kPacket.m_iReason	= NetError::NOT_LEAVE_ROOM_REASON_01;
//			_SendToCnRoom( GetTradeUID(), ERM_BREAK_TRADE_REQ, kPacket );
//
//			m_iTradeUID = 0;
//		}
//
//		// 6. ���λ��� ��Ż ó��
//		if( GetPersonalShopUID() != 0 )
//		{
//			//{{ 2011. 05. 02	������	�븮����
//#ifdef SERV_PSHOP_AGENCY
//			if( CXSLRoom::GetRoomType( GetPersonalShopUID() ) == CXSLRoom::RT_PERSONAL_SHOP )
//			{
//				KEGS_BREAK_PERSONAL_SHOP_REQ kPacket;
//				kPacket.m_iReason	= NetError::NOT_LEAVE_ROOM_REASON_01;
//				_SendToCnRoom( GetPersonalShopUID(), ERM_BREAK_PERSONAL_SHOP_REQ, kPacket );
//			}
//			else
//			{
//				KERM_LEAVE_PERSONAL_SHOP_REQ kPacket;
//				kPacket.m_iReason	= NetError::NOT_LEAVE_ROOM_REASON_01;
//				_SendToLoginRoom( GetPersonalShopUID(), ERM_LEAVE_PSHOP_AGENCY_REQ, kPacket );
//			}
//#else
//			KEGS_BREAK_PERSONAL_SHOP_REQ kPacket;
//			kPacket.m_iReason	= NetError::NOT_LEAVE_ROOM_REASON_01;
//			_SendToCnRoom( GetPersonalShopUID(), ERM_BREAK_PERSONAL_SHOP_REQ, kPacket );
//#endif SERV_PSHOP_AGENCY
//			//}}
//
//			SetPersonalShopUID( 0 );
//		}
//
//		// 8. ���� �濡 �ִٸ�..
//		if( GetStateID() == KGSFSM::S_ROOM )
//		{
//			//{{ 2012. 10. 18	������	��Ʋ�ʵ� �ý���
//			//////////////////////////////////////////////////////////////////////////
//#ifdef SERV_BATTLE_FIELD_SYSTEM
//			//////////////////////////////////////////////////////////////////////////
//			switch( CXSLRoom::GetRoomType( GetRoomUID() ) )
//			{
//			case CXSLRoom::RT_SQUARE:
//				{
//					KEGS_LEAVE_SQUARE_REQ kPacket;
//					kPacket.m_iReason	= NetError::NOT_LEAVE_ROOM_REASON_01;
//					SendToCnRoom( ERM_LEAVE_SQUARE_REQ, kPacket );
//				}
//				break;
//
//			case CXSLRoom::RT_TRAININGCENTER:
//				{
//					KERM_LEAVE_ROOM_REQ kPacket;
//
//					// Ŭ���̾�Ʈ ũ������ ��� [Ŭ���̾�Ʈ ũ������ ���Ϳ��� ���� ��Ŷ�� ������ �ʵ���]
//					if( GetDisconnectReason() == KStatistics::eSIColDR_Client_Crash )
//					{
//						kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_23;
//					}
//					else
//					{
//						kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_01;
//					}
//
//					SendToCnRoom( ERM_LEAVE_TC_ROOM_REQ, kPacket );
//				}
//				break;
//
//			case CXSLRoom::RT_BATTLE_FIELD:
//				{
//					KERM_LEAVE_ROOM_REQ kPacket;
//
//					// Ŭ���̾�Ʈ ũ������ ��� [Ŭ���̾�Ʈ ũ������ ���Ϳ��� ���� ��Ŷ�� ������ �ʵ���]
//					switch( GetDisconnectReason() )
//					{
//					case KStatistics::eSIColDR_Client_Crash:	kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_23;	break;
//					case KStatistics::eSIColDR_IOCP_GetStatus:	kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_33; break;
//					case KStatistics::eSIColDR_Channel_Change:	kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_29; break;
//					default:									kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_01; break;
//					}
//
//					SendToCnRoom( ERM_LEAVE_ROOM_REQ, kPacket );
//				}
//				break;
//
//			default:
//				{
//					KERM_LEAVE_ROOM_REQ kPacket;
//
//					// Ŭ���̾�Ʈ ũ������ ��� [Ŭ���̾�Ʈ ũ������ ���Ϳ��� ���� ��Ŷ�� ������ �ʵ���]
//					if( GetDisconnectReason() == KStatistics::eSIColDR_Client_Crash )
//					{
//						kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_23;
//					}
//					else
//					{
//						kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_01;
//					}
//
//					SendToCnRoom( ERM_LEAVE_ROOM_REQ, kPacket );
//				}
//				break;
//			}
//			//////////////////////////////////////////////////////////////////////////
//#else
//			//////////////////////////////////////////////////////////////////////////
//			if( CXSLRoom::GetRoomType( GetRoomUID() ) == CXSLRoom::RT_SQUARE )
//			{
//				KEGS_LEAVE_SQUARE_REQ kPacket;
//				kPacket.m_iReason	= NetError::NOT_LEAVE_ROOM_REASON_01;
//				SendToCnRoom( ERM_LEAVE_SQUARE_REQ, kPacket );
//			}
//			else
//			{
//				u_short usEventID;			
//				if( CXSLRoom::GetRoomType( GetRoomUID() ) == CXSLRoom::RT_TRAININGCENTER )
//				{
//					usEventID = ERM_LEAVE_TC_ROOM_REQ;
//				}
//				else
//				{
//					usEventID = ERM_LEAVE_ROOM_REQ;
//				}
//
//				KERM_LEAVE_ROOM_REQ kPacket;
//
//				// Ŭ���̾�Ʈ ũ������ ��� [Ŭ���̾�Ʈ ũ������ ���Ϳ��� ���� ��Ŷ�� ������ �ʵ���]
//				if( GetDisconnectReason() == KStatistics::eSIColDR_Client_Crash )
//				{
//					kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_23;
//				}
//				else
//				{
//					kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_01;
//				}
//
//				SendToCnRoom( usEventID, kPacket );
//			}
//			//////////////////////////////////////////////////////////////////////////
//#endif SERV_BATTLE_FIELD_SYSTEM
//			//////////////////////////////////////////////////////////////////////////
//			//}}		
//
//			//{{ 2010. 12. 30	������	����Ż ����ó��
//#ifdef SERV_LEAVE_ROOM_BUG_FIX
//			SetRoomListID( 0 );
//#endif SERV_LEAVE_ROOM_BUG_FIX
//			//}}
//
//			//{{ 2012. 02. 07	������	��Ʋ�ʵ� �ý���
//#ifdef SERV_BATTLE_FIELD_SYSTEM
//			SetRoomUID( 0 );
//
//			if( GetStateID() == KGSFSM::S_ROOM )
//			{
//				StateTransition( KGSFSM::I_TO_FIELD_MAP );
//			}
//#endif SERV_BATTLE_FIELD_SYSTEM
//			//}}
//		}	
//
//		//{{ 2010. 01. 11  ������	��õ�θ���Ʈ
//#ifdef SERV_RECOMMEND_LIST
//		SendUpdateRecommendUserInfo( KRecommendUserInfo::RUIT_LOGOUT );
//#endif SERV_RECOMMEND_LIST
//		//}}
//
//		//{{ 2010. 8. 4	������	�� �ý���
//#ifdef SERV_PET_SYSTEM		
//		// ĳ���� ���� ������ �� ���� ó��!
//		m_kUserPetManager.OnDeselectUnit( GetThisPtr<KGSUser>() );
//#endif SERV_PET_SYSTEM
//		//}}
//
//		//{{  2011.11.24     ��μ�    ĳ���� ���� �α�
//#ifdef SERV_CHAR_CONNECT_LOG
//		SendUpdateUnitConnectLog();
//#endif SERV_CHAR_CONNECT_LOG
//		//}}
//
//		//{{ ���� ���� �ý��� - ��μ�
//#ifdef SERV_SERVER_BUFF_SYSTEM
//		CheckDeselectUnitBuff( false );
//#endif SERV_SERVER_BUFF_SYSTEM
//		//}
//
//		// 9. ���� ���� DB�� ������Ʈ
//		WriteUnitInfoToDB( true );
//
//		//{{ 2012. 09. 22	�ڼ���	���� �̺�Ʈ
//#ifdef SERV_EVENT_BINGO
//		BingoEventDBWrite();
//#endif SERV_EVENT_BINGO
//		//}}
//
//		//{{ 2009. 7. 9  ������		�α��μ����� ĳ���� ���� ���췯 ����
//		KELG_UNIT_DISCONNECT_NOT kPacketNot;
//		kPacketNot.m_iUnitUID = GetCharUID();
//		//{{ 2009. 9. 29  ������	���
//#ifdef GUILD_TEST
//		kPacketNot.m_iGuildUID = GetGuildUID();
//#endif GUILD_TEST
//		//}}
//		std::map< UidType, KFriendInfo >::iterator mitFriendInfo;
//		for( mitFriendInfo = m_kMessengerInfo.m_mapFriendInfo.begin(); mitFriendInfo != m_kMessengerInfo.m_mapFriendInfo.end(); mitFriendInfo++ )
//		{
//			kPacketNot.m_setFriendUID.insert( mitFriendInfo->first );
//		}
//		SendToLoginServer( ELG_UNIT_DISCONNECT_NOT, kPacketNot );
//		//}}
//
//		//{{ 2012. 03. 20	������	��Ʋ�ʵ� �ý���
//#ifdef SERV_BATTLE_FIELD_SYSTEM
//		KEGB_UNIT_DISCONNECT_NOT kPacketToGB;
//		kPacketToGB.m_iUnitUID = GetCharUID();
//		SendToGlobalServer( EGB_UNIT_DISCONNECT_NOT, kPacketToGB );
//#endif SERV_BATTLE_FIELD_SYSTEM
//		//}}
//
//		// ��������
//		m_kUserTutorialInfo.DisconnectUpdateUnitInfo( GetThisPtr<KGSUser>() );
//
//		if( m_kUserTutorialInfo.GetTutorUnitType() == KUserTutorialInfo::TUT_STUDENT  &&  SiKTutorialManager()->IsExistbylist( GetCharUID() ) == true )
//		{
//			if( SiKTutorialManager()->DeleteUnit( GetCharUID() ) == false )
//			{
//				START_LOG( cerr, L"�缼�ý����� �������� ����� ����.!" )
//					<< BUILD_LOG( GetCharName() )
//					<< BUILD_LOG( GetLevel() )
//					<< END_LOG;
//			}
//		}

		//////////////////////////////////////////////////////////////////////////
//#endif SERV_DESELECT_UNIT_CODE_REFACTORING
		//////////////////////////////////////////////////////////////////////////
		//}}
	}

#ifdef SERV_MORNITORING_ABUSER_NOTIFY_SMS// �۾���¥: 2013-05-22	// �ڼ���
	if( m_kUserAbuserManager.IsMornitoringAbuser() == true )
	{
		KDBE_SEND_PHONE_MSG_NOT kPacketNot;
		kPacketNot.m_wstrSMSMessage = boost::str( boost::wformat( L"����͸� ���� ����� ���� ����: UserUID(%d), ServerUID(%d), ServerIP(%s)" ) % GetUID() % KBaseServer::GetKObj()->GetUID() % KBaseServer::GetKObj()->GetPublicIP() );
		SiKSMSPhoneNumberManager()->GetPhoneNumberList( KSMSPhoneNumberManager::FS_MORNITORING_ABUSER_NOTIFY, kPacketNot.m_vecPhoneNum );
		if( kPacketNot.m_vecPhoneNum.empty() == false )
		{
			SendToSMSDB( DBE_SEND_PHONE_MSG_NOT, kPacketNot );
		}
	}
#endif // SERV_MORNITORING_ABUSER_NOTIFY_SMS

#ifdef SERV_SYNC_PACKET_USING_RELAY_WORKINGS_CHECK// �۾���¥: 2013-05-22	// �ڼ���
	{
		KDBE_BLOCK_COUNT_CHECK_INFO_UPDATE_NOT kPacketToDB;
		if( GetBlockCountChanged( kPacketToDB.m_byteBlockCheckCount, kPacketToDB.m_wstrBlockCheckResetDate ) == true )
		{
			SendToAccountDB( DBE_BLOCK_COUNT_CHECK_INFO_UPDATE_NOT, kPacketToDB );
		}
	}
#endif // SERV_SYNC_PACKET_USING_RELAY_WORKINGS_CHECK

#ifdef SERV_HACKING_USER_CHECK_COUNT// �۾���¥: 2013-06-02	// �ڼ���
	{
		KDBE_HACKING_USER_CHECK_COUNT_UPDATE_NOT kPacketToDB;
		m_kUserAbuserManager.GetDBUpdateInfo( kPacketToDB.m_mapHackingUserCheckCountDB );
		if( kPacketToDB.m_mapHackingUserCheckCountDB.empty() == false )
		{
			kPacketToDB.m_iUserUID = GetUID();
			SendToAccountDB( DBE_HACKING_USER_CHECK_COUNT_UPDATE_NOT, kPacketToDB );
		}
	}
#endif // SERV_HACKING_USER_CHECK_COUNT

	//{{ 2012. 03. 29	��μ�		�ߺ� sn ���� ����
#ifdef SERV_SERIAL_NUMBER_AVAILABILITY_CHECK_FILTER
	KEGB_UNREGISTER_SERVERSN_NOT kNot;
	m_kUserSecurityManager.GetServerSN( kNot.m_iServerSN );
	kNot.m_iUserUid = GetUID();
	SendToGlobalServer( EGB_UNREGISTER_SERVERSN_NOT, kNot );
#endif SERV_SERIAL_NUMBER_AVAILABILITY_CHECK_FILTER
	//}}

	// 2-1. ���ῡ ���� ��� ����� [USER_FSM]
	const int iServerUID = static_cast<int>(KBaseServer::GetKObj()->GetUID());
	const int iIndex = KStatistics::GetDisconnectUserFSM( GetStateID() );

	if( iIndex >= 0 && iIndex < KStatistics::eSIColDU_Num )
	{
		KStatisticsKey kKey;
		kKey.m_vecIntKey.push_back( iServerUID );
		KSIManager.IncreaseCount( KStatistics::SI_DISCONNECT_USERFSM, kKey, iIndex, 1 );
	}
	else
	{
		START_LOG( cerr, L"disconnect userfsm ��� �ε��� ���� �̻��մϴ�." )
			<< BUILD_LOG( GetStateID() )
			<< BUILD_LOG( iIndex )
			<< END_LOG;
	}

	// 2-2. ���ῡ ���� ��� ����� [REASON]
	KStatisticsKey kKey;
	kKey.m_vecIntKey.push_back( iServerUID );
	KSIManager.IncreaseCount( KStatistics::SI_DISCONNECT_REASON, kKey, GetDisconnectReason(), 1 );

	// 2-3. �� ���� ������ ó��.
	switch( GetDisconnectReason() )
	{
	case KStatistics::eSIColDR_IOCP_GetStatus:
		{
			//{{ 2009. 9. 1  ������		IOCP�������
#ifdef ABNORMAL_REASON

			START_LOG( clog, L"IOCP�������" )
#ifdef SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetUID() )
#else
				<< BUILD_LOG( GetUserName() )
#endif SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetLastIocpFailedError() );

			GetKGSSimLayer()->AddAbnormalDisconnectReason( GetLastIocpFailedError() );

			//{{ 2009. 9. 13  ������	���� �α�
#ifdef USER_LOG
			if( KSIManager.IsUserLog() )
			{
				m_kUserLogManager.DumpLogFile( GetThisPtr<KGSUser>(), KUserLogManager::ULT_ABNORMAL_DICONNECT );
			}
#endif USER_LOG
			//}}
#endif ABNORMAL_REASON
			//}}
		}
		break;

		//{{ 2010. 02. 01  ������	��Ʈ��Ʈ �����α�
#ifdef USER_LOG_HEART_BEAT

	case KStatistics::eSIColDR_Heart_Beat:
		{
			if( KSIManager.IsUserLog() )
			{
				m_kUserLogManager.DumpLogFile( GetThisPtr<KGSUser>(), KUserLogManager::ULT_HEART_BEAT );
			}
		}
		break;

#endif USER_LOG_HEART_BEAT
		//}}

	case KStatistics::eSIColDR_SendBufferFull:
		{
			START_LOG( cout, L"Send Buffer Full ���� ���� ����." )
				<< BUILD_LOG( GetUID() )
#ifndef SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetName() )
#endif SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( KEvent::GetIDStr( GetLastSendEventID() ) )
				<< BUILD_LOG( GetStateIDString() )
				<< BUILD_LOG( GetCharUID() )
#ifndef SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetFieldUID() )
				<< BUILD_LOG( GetRoomListID() )
				<< BUILD_LOG( GetRoomUID() )
				<< BUILD_LOG( GetTradeUID() )
				<< BUILD_LOG( GetPersonalShopUID() )
				<< BUILD_LOG( (int)CXSLRoom::GetRoomType( GetRoomUID() ) );
		}
		break;

	case KStatistics::eSIColDR_Traffic_Attack:
		{
			//{{ 2009. 10. 22  ������	Ʈ���� ���� ���� ��ŷ������ �ڵ����
			KELOG_TRAFFIC_ATTACKER_LOG_NOT kPacketLog;
			kPacketLog.m_wstrUserID = GetName();
			kPacketLog.m_iChannelID = SiKChannelManager()->GetChannelID();
			SendToLogDB( ELOG_TRAFFIC_ATTACKER_LOG_NOT, kPacketLog );
			//}}

			//{{ 2009. 10. 26  ������	Ʈ���Ȱ��ݹ���
			SendID( EGS_TRAFFIC_ATTACKER_NOT );
			//}}
		}
		break;

	default:
		break;
	}

	// 3. LConnectLog ���
	if( GetStateID() >= KGSFSM::S_LOGINED )
	{
		WritePlayTimeToDB();

		//{{ 2011. 05. 02  ��μ�	2�� ���� �ý���
#ifdef SERV_SECOND_SECURITY
		// 11. 2�� ���� �е� �� OTP ��� ���� ���� ���
		SendSecurityType();
#endif SERV_SECOND_SECURITY
		//}}

		//{{ 2011. 05. 27    ��μ�    �޸� ���� ���� ����
#ifdef SERV_COME_BACK_USER_REWARD
		if( m_kComeBackUserInfo.GetIsUnitSelect() == true )
		{
			SendComeBackLogOut();
		}
#endif SERV_COME_BACK_USER_REWARD
		//}}
	}

	//{{ 2011. 09. 14  ��μ�	��ŷ ���μ��� ��� ���� - ���� �ε� ���� �ܰ�
#ifdef SERV_DLL_LIST_CHECK_BEFOR_LOADING
	// ����� ������ ChannalRandomKey �� �����Ѵ�.
//	SendRefreshChannalRandomKey();
#endif SERV_DLL_LIST_CHECK_BEFOR_LOADING
	//}}

    KGSUserPtr spUser( GetThisPtr<KGSUser>() );

    // 2007.12. 27. jseop. �α��� ���ϴ� ������ �ֳ�?
	if( GetStateID() < KGSFSM::S_LOGINED )
    {
		switch( GetDisconnectReason() )
		{
		case KStatistics::eSIColDR_Limit_User_Num:
			{
				START_LOG( cwarn, L"���� ���� �ο� ������." )
					<< BUILD_LOG( GetUID() )
#ifndef SERV_PRIVACY_AGREEMENT
					<< BUILD_LOG( GetName() )
					<< BUILD_LOG( GetIPStr() )
#endif SERV_PRIVACY_AGREEMENT
					<< BUILD_LOG( GetStateID() )
					<< BUILD_LOG( GetStateIDString() )
					<< END_LOG;
			}
			break;

			//{{ 2010. 06. 23  ������	�����������
#ifdef SERV_ACCOUNT_BLOCK
		case KStatistics::eSIColDR_GameServer_Account_Block:
			{
				START_LOG( cwarn, L"��ŷ �ǽ� ���� ���� ���� ��ġ." )
					<< BUILD_LOG( GetUID() )
#ifndef SERV_PRIVACY_AGREEMENT
					<< BUILD_LOG( GetName() )
					<< BUILD_LOG( GetIPStr() )
#endif SERV_PRIVACY_AGREEMENT
					<< BUILD_LOG( GetStateID() )
					<< BUILD_LOG( GetStateIDString() )
					<< END_LOG;
			}
			break;
#endif SERV_ACCOUNT_BLOCK
			//}}

			//{{ 2011. 08. 09	������	�ӽ�ID �������
#ifdef SERV_MACHINE_ID_BLOCK
		case KStatistics::eSIColDR_MachineID_Block:
			{
				START_LOG( cout, L"�ӽ� ID ����." )
					<< BUILD_LOG( GetUID() )
#ifndef SERV_PRIVACY_AGREEMENT
					<< BUILD_LOG( GetName() )
					<< BUILD_LOG( GetIPStr() )
#endif SERV_PRIVACY_AGREEMENT
					<< BUILD_LOG( GetStateID() )
					<< BUILD_LOG( GetStateIDString() )
					<< END_LOG;
			}
			break;
#endif SERV_MACHINE_ID_BLOCK
			//}}

		default:
			{
				START_LOG( cerr2, L"�α��� ���� ����." )
					<< BUILD_LOG( GetUID() )
#ifndef SERV_PRIVACY_AGREEMENT
					<< BUILD_LOG( GetName() )
					<< BUILD_LOG( GetIPStr() )
#endif SERV_PRIVACY_AGREEMENT
					<< BUILD_LOG( GetStateID() )
					<< BUILD_LOG( GetStateIDString() )
					<< END_LOG;
			}
			break;
		}
    }	

//#ifdef	SERV_SHARING_BANK_TEST -- �ǽð� ������Ʈ�� �ٲ�� ������ ���⼭ ó���� �ʿ䰡 ����.
//	KDBE_UPDATE_SHARE_ITEM_REQ kPacketSendDB;
//
//	//	������� ���
//	if( m_kInventory.GetShareItemNeedDBUpdate( kPacketSendDB ) == true )
//	{
//		kPacketSendDB.m_bReload = false;
//		SendToGameDB( DBE_UPDATE_SHARE_ITEM_REQ, kPacketSendDB );
//	}
//#endif	SERV_SHARING_BANK_TEST

	// 10. ��� ���� ���� �غ� �Ϸ� �Ǿ��ٸ� �α��μ����� ������ �������� �뺸
	SendDisconnectEvent();

#ifdef SERV_CHECK_PCBANG_BY_PUBLISHER
#ifndef SERV_COUNTRY_TH	// �±��� ��� PC�� ���ο� ������� �α׾ƿ� ó���� �ؾ���
	if( IsPcBang() == true )
#endif //SERV_COUNTRY_TH
	{
		KELG_PCBANG_LOGOUT_NOT kPacketNot;
		kPacketNot.m_iUserUID = GetUID();
		kPacketNot.m_wstrUserID = GetName();
		kPacketNot.m_wstrIP = KncUtil::toWideString( GetIPStr() );

#ifdef SERV_COUNTRY_TH
		kPacketNot.m_wstrSockID = GetSocketID();
		kPacketNot.m_iGSUID = KBaseServer::GetKObj()->GetUID();
#endif SERV_COUNTRY_TH

		SendToLoginServer( ELG_PCBANG_LOGOUT_NOT, kPacketNot );
	}
#endif //SERV_CHECK_PCBANG_BY_PUBLISHER

	// 11. ������ü ������ ���� FSM�� S_EXIT�� ��ȯ
    StateTransition( KGSFSM::I_TO_EXIT );

	//{{ 2008. 11. 10  ������	����͸� �����	
	//if( m_bIsMornitoringAbuser )
	//{
	//	std::wstring wstrIPAddr = KncUtil::toWideString( GetIPStr(), strlen( GetIPStr() ) );

	//	KDBE_SEND_PHONE_MSG_NOT kPacketNot;
	//	kPacketNot.m_wstrSMSMessage  = L"����� ���� [";
	//	kPacketNot.m_wstrSMSMessage += GetName();
	//	kPacketNot.m_wstrSMSMessage += L"]�� ���������Ͽ����ϴ�! : ";
	//	kPacketNot.m_wstrSMSMessage += wstrIPAddr;

	//	SiKAbuserLogManager()->GetAbuserNotifyPhoneNumList( kPacketNot.m_wstrPhoneNum );

	//	SendToSMSDB( DBE_SEND_PHONE_MSG_NOT, kPacketNot );
	//}
	//}}

	//{{ 2011. 08. 10    ��μ�    ä�θ� SOAP
#ifdef SERV_CHANNELING_SOAP
	m_wsrtToonilandID = L"";
#endif SERV_CHANNELING_SOAP
	//}}

#ifdef SERV_NPROTECT_CS_AUTH_30
	if( KSimLayer::GetKObj()->CheckCommonFlag( KSimLayer::CommonFlag::CF_NPGG ) == true )
	{
		m_CSAuth3.Close();
	}
#endif SERV_NPROTECT_CS_AUTH_30		

    KSession::OnDestroy();
}

bool KGSUser::RoutePacket( const KEvent* pkEvent_ )
{
    //START_LOG( clog, L"�����" )
    //    << BUILD_LOG( pkEvent_->m_kDestPerformer.m_dwPerformerID )
    //    << BUILD_LOG( pkEvent_->m_kDestPerformer.m_iUID )
    //    << BUILD_LOG( pkEvent_->m_usEventID )
    //    << BUILD_LOG( pkEvent_->GetIDStr() );
        
    if( pkEvent_->m_kDestPerformer.m_dwPerformerID != PI_GS_USER )
    {
        int iEval = CompareServerLevel( pkEvent_->m_kDestPerformer.m_dwPerformerID, PI_GS_USER );
        if( iEval < 0 )
        {
            _JIF( !pkEvent_->IsEmptyTrace(), return true );
            KEvent kEvent( *pkEvent_ );
            kEvent.PopTrace();
            LIF( KSession::SendPacket( kEvent ) );

			//{{ 2009. 9. 13  ������	���� �α�
#ifdef USER_LOG
			m_kUserLogManager.SaveSendPacket( kEvent.m_usEventID );
#endif USER_LOG
			//}}

            return true;
        }
        else if( iEval > 0 )
        {
            KEvent kEvent( *pkEvent_ );
            kEvent.PushTrace( GetUID() );
            SiKProxyManager()->SendPacket( GetUID(), kEvent );
            return true;
        }
        else
        {
            KEventPtr spEvent( pkEvent_->Clone() );
            spEvent->PushTrace( GetUID() );

            switch( GetPerformerClass( pkEvent_->m_kDestPerformer.m_dwPerformerID ) )
            {
            case PC_CHARACTER:
                break;
            case PC_SERVER:
                KBaseServer::GetKObj()->QueueingEvent( spEvent );
                return true;
            case PC_GAME_DB:
                SiKDBLayer()->QueueingEvent( spEvent );
                return true;
            default:
                START_LOG( cerr, L"�������� �̻��� ��Ŷ." )
                    << BUILD_LOG( pkEvent_->m_kDestPerformer.m_dwPerformerID )
                    << END_LOG;
                return true;
            }
        }
    }

    return false;
}

void KGSUser::WriteUnitInfoToDB( bool bFinal, int iEventID /*= 0*/ )
{
    // ������ ���õ��� �ʾ����� ������Ʈ ���� ���ƾ� �Ѵ�.
    if( GetCharUID() <= 0 )
    {
#ifdef SERV_BUY_PERSONAL_SHOP_ITEM_ERROR_LOG
		START_LOG( cerr, L"[ERROR] ���� ���� ������Ʈ �ϴµ�, ������ ���õ��� �ʾҴ�.")
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOG( iEventID )
			<< END_LOG;
#endif //SERV_BUY_PERSONAL_SHOP_ITEM_ERROR_LOG
        return;
    }

    KDBE_UPDATE_UNIT_INFO_REQ kPacket;
    kPacket.m_bFinal		= bFinal;
	kPacket.m_iEventID		= iEventID;	// for ä���̵�
    kPacket.m_iUnitUID		= GetCharUID();
	kPacket.m_iEXP			= m_kEXP.GetChangeExp();
    kPacket.m_iLevel        = static_cast<int>(GetLevel());
	//{{ 2012. 10. 02	������		ED&������ ����͸� �ý���
	//////////////////////////////////////////////////////////////////////////
#ifdef SERV_ED_AND_ITEM_MORNITORING_SYSTEM
	//////////////////////////////////////////////////////////////////////////
	//{{ 2012. 04. 17	������	���ΰŷ� DDOS���
#ifdef SERV_TRADE_DDOS_DEFENCE
	if( m_kInventory.IsLocked() == true )
	{
		if( m_kEDManager.GetChangeValue() != 0 )
		{
			START_LOG( cout, L"[�˸�] Inventroy Lock �����ε� ED���а��� �����ִ�! �̷��� ���� �����ٵ�..?" )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOG( GetCharName() )
				<< BUILD_LOG( GetED() )
				<< BUILD_LOG( m_kEDManager.GetChangeValue() );
		}
	}
#endif SERV_TRADE_DDOS_DEFENCE
	//}}
	m_kEDManager.GetDBUpdateInfo( kPacket.m_iED );
	//{{ 2011. 09. 23	������	�α׿��� ���� EDüũ
#ifdef SERV_LOGOUT_ED_CHECK
	kPacket.m_iRealDataED	= GetED();
#endif SERV_LOGOUT_ED_CHECK
	//}}
	//////////////////////////////////////////////////////////////////////////
#else
	//////////////////////////////////////////////////////////////////////////
	kPacket.m_iED			= m_iED.GetChangeValue();		// SetValue()�� �� �Լ� �Ʒ��ʿ� �ֽ��ϴ�.
	//{{ 2011. 09. 23	������	�α׿��� ���� EDüũ
#ifdef SERV_LOGOUT_ED_CHECK
	kPacket.m_iRealDataED	= GetED();
#endif SERV_LOGOUT_ED_CHECK
	//}}
	//{{ 2012. 04. 17	������	���ΰŷ� DDOS���
#ifdef SERV_TRADE_DDOS_DEFENCE
	if( m_kInventory.IsLocked() == true )
	{
		if( m_iED.GetChangeValue() != 0 )
		{
			START_LOG( cout, L"[�˸�] Inventroy Lock �����ε� ED���а��� �����ִ�! �̷��� ���� �����ٵ�..?" )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOG( GetCharName() )
				<< BUILD_LOG( GetED() )
				<< BUILD_LOG( m_iED.GetChangeValue() );
		}
	}
#endif SERV_TRADE_DDOS_DEFENCE
	//}}
	//////////////////////////////////////////////////////////////////////////
#endif SERV_ED_AND_ITEM_MORNITORING_SYSTEM
	//////////////////////////////////////////////////////////////////////////
	//}}	
	kPacket.m_iSPoint		= m_iSPoint.GetChangeValue();
	//{{ 2011. 06. 20	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
	m_kUserPvpManager.GetDBUpdateInfo( kPacket.m_iOfficialMatchCnt, 
										kPacket.m_iRating, 
										kPacket.m_iMaxRating,
										kPacket.m_iRPoint,
										kPacket.m_iAPoint,
										kPacket.m_bIsWinBeforeMatch,
										kPacket.m_iWin, 
										kPacket.m_iLose,
										kPacket.m_cEmblemEnum

										//{{ 2012. 06. 22	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2
										, kPacket.m_fKFactor
#endif SERV_2012_PVP_SEASON2
										//}}
										);
#else
	kPacket.m_iVSPoint		= m_iVSPoint.GetChangeValue();
	kPacket.m_iVSPointMax	= m_iVSPointMax.GetChangeValue();	
	kPacket.m_iWin			= m_iWin;
	kPacket.m_iLose			= m_iLose;
#endif SERV_PVP_NEW_SYSTEM
	//}}
	kPacket.m_kDenyOptions  = m_kUserGameOption.GetDenyOptions();
	
	//{{ 2012. 03. 20	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	m_kUserSpiritManager.GetDBUpdateInfo( kPacket.m_iSpirit, kPacket.m_bIsSpiritUpdated );
#else
	kPacket.m_iSpirit		= m_iSpirit;			 // �ټ���
	kPacket.m_bIsSpiritUpdated = m_bIsSpiritUpdated; // �ټ��� ��� �Ҹ� ������Ʈ
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}	

#ifdef SERV_CHINA_SPIRIT_EVENT
	kPacket.m_arrChinaSpirit[0] = m_arrChinaSpirit[0];
	kPacket.m_arrChinaSpirit[1] = m_arrChinaSpirit[1];
	kPacket.m_arrChinaSpirit[2] = m_arrChinaSpirit[2];
	kPacket.m_arrChinaSpirit[3] = m_arrChinaSpirit[3];
	kPacket.m_arrChinaSpirit[4] = m_arrChinaSpirit[4];
	kPacket.m_arrChinaSpirit[5] = m_arrChinaSpirit[5];
#endif SERV_CHINA_SPIRIT_EVENT

	//{{ 2009. 10. 7  ������	���
#ifdef GUILD_TEST
	kPacket.m_iGuildUID		= GetGuildUID();
	kPacket.m_iHonorPoint	= m_kUserGuildManager.GetHonorPoint();
#endif GUILD_TEST
	//}}
	
	// ��Ȱ��
	kPacket.m_iNumResurrectionStone = m_iNumResurrectionStone.GetChangeValue();
    m_iNumResurrectionStone.SetValue( m_iNumResurrectionStone );

	//{{ 2009. 10. 14  ������	�ڵ����� ��Ȱ��
#ifdef AP_RESTONE
	kPacket.m_iNumAutoPaymentResStone = m_iNumAutoPaymentResStone.GetChangeValue();
	m_iNumAutoPaymentResStone.SetValue( m_iNumAutoPaymentResStone );
#endif AP_RESTONE
	//}}
	//{{ 2012. 02. 21	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	m_kUserUnitManager.GetGamePlayStatus( kPacket.m_kGamePlayStatus );
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}

	//////////////////////////////////////////////////////////////////////////
	// Packing LastPosition

	//07.07.19.  hoon. ���� �����̵��� ������ ���� ���̵� 0���� ���� �Ǵ� ������ ����⶧���� �ӽ÷� �纥�� ������.
	//������ ������ DB�� �����ϸ�ǰ� ������ ���� ������ ������ ���� ��Ŷ�� �ٷ� �ִ´�.
	//{{ 2012. 02. 02	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM	
	// MapID����
	if( bFinal )
	{
		if( SEnum::IsBattleFieldID( GetMapID() ) == true )
		{
			// �׳� ���!
		}
		else
		{
			if( SiCXSLMapData()->CheckMapData( GetMapID() ) == false )
			{
				START_LOG( cerr, L"DB������Ʈ �Ϸ��� MapID�� �̻��ϴ�?" )
#ifdef SERV_PRIVACY_AGREEMENT
					<< BUILD_LOG( GetCharUID() )
#else
					<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
					<< BUILD_LOG( GetMapID() )
					<< BUILD_LOG( SiCXSLMapData()->GetDefaultMapID() )
					<< END_LOG;

				// �⺻���� �־��ش�!		
				SetMapID( SiCXSLMapData()->GetDefaultMapID() );
			}
		}
	}

	kPacket.m_kLastPos = GetLastPosInfo(); // ���̻� packing�ؼ� ������� �ʽ��ϴ�.
#else
	if( bFinal  &&  SiCXSLMapData()->CheckMapData( GetMapID() ) == false )
	{
		START_LOG( cerr, L"DB������Ʈ �Ϸ��� MapID�� �̻��ϴ�?" )
#ifdef SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetCharUID() )
#else
			<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetMapID() )
			<< BUILD_LOG( SiCXSLMapData()->GetDefaultMapID() )
			<< END_LOG;

		// �⺻���� �־��ش�!
		m_nMapID = SiCXSLMapData()->GetDefaultMapID();
	}

	CXSLMapData::PackingLastPosition( GetMapID(), m_ucLastTouchLineIndex, m_usLastPosValue, kPacket.m_iMapID );
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}
	//////////////////////////////////////////////////////////////////////////

    std::map< int, KDungeonClearInfo >::iterator mit;
    for( mit = m_mapDungeonClear.begin(); mit != m_mapDungeonClear.end(); mit++ )
    {
        if( mit->second.m_bNew )
        {
            kPacket.m_mapDungeonClear.insert( std::make_pair( mit->first, mit->second ) );
        }
        mit->second.m_bNew = false;
    }

#ifdef SERV_LIMITED_DUNGEON_PLAY_TIMES
	std::map< int, KDungeonPlayInfo >::iterator mitDungeonPlay;
	for( mitDungeonPlay = m_mapDungeonPlay.begin(); mitDungeonPlay != m_mapDungeonPlay.end(); mitDungeonPlay++ )
	{
		if( mitDungeonPlay->second.m_bNew )
		{
			kPacket.m_mapDungeonPlay.insert( std::make_pair( mitDungeonPlay->first, mitDungeonPlay->second ) );
		}
		mitDungeonPlay->second.m_bNew = false;
	}
#endif SERV_LIMITED_DUNGEON_PLAY_TIMES

	std::map< int, KTCClearInfo >::iterator mitt;
	for( mitt = m_mapTCClear.begin(); mitt != m_mapTCClear.end(); ++mitt )
	{
		if( mitt->second.m_bNew )
		{
			kPacket.m_mapTCClear.insert( std::make_pair( mitt->first, mitt->second ) );
		}
		mitt->second.m_bNew = false;
	}

    m_kInventory.FlushQuantityChange( kPacket.m_kItemQuantityUpdate.m_mapQuantityChange );	
	m_kInventory.FlushEnduranceChange( kPacket.m_kItemEnduranceUpdate );
    m_kInventory.FlushDeletedItem( kPacket.m_kItemQuantityUpdate.m_vecDeleted );
    m_kInventory.FlushPositionChange( kPacket.m_kItemPositionUpdate );

	//QUEST UPDATE
	m_kUserQuestManager.GetUpdatedQuestInfo( kPacket.m_vecQuestData );

	//{{ 2010. 02. 09  ������	���� �̺�Ʈ ����Ʈ
#ifdef SERV_DAY_QUEST
	m_kUserQuestManager.GetReservedGiveUpQuest( kPacket.m_vecGiveUpQuestList );
#endif SERV_DAY_QUEST
	//}}

	// MISSION UPDATE
	//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
#ifdef SERV_TITLE_DATA_SIZE
	m_kUserTitleManager.GetUpdateInfo( IsPcBang(), kPacket.m_iInitTitleID, kPacket.m_iEquippedTitleID, kPacket.m_vecMissionData );
#else
	m_kUserTitleManager.GetUpdateInfo( IsPcBang(), kPacket.m_sInitTitleID, kPacket.m_sEquippedTitleID, kPacket.m_vecMissionData );
#endif SERV_TITLE_DATA_SIZE
	//}}	

	//SKILL SLOT UPDATE
	m_kSkillTree.GetSkillSlot( kPacket.m_vecSkillSlot );
	if( KUserSkillTree::SSBES_EXPIRED == m_kSkillTree.GetSkillSlotBExpirationState() )
	{
		kPacket.m_vecSkillSlot.resize(4); 
	}

	// ��ٱ��� ������Ʈ
	if( bFinal )
	{
		m_kUserWishList.FlushWishListChange( kPacket.m_mapWishList );
	}

	//{{ 2009. 7. 7  ������		��ŷ ����	
	m_kUserRankingManager.GetUpdateInfo( kPacket.m_vecHenirRanking );
	//}}

	//{{ 2009. 12. 8  ������	�̺�Ʈ����
#ifdef CUMULATIVE_TIME_EVENT	
	m_kUserEventManager.GetDBUpdateInfo( kPacket.m_vecUpdateEventTime );	
#endif CUMULATIVE_TIME_EVENT
	//}}

	//{{ 2010. 8. 4	������	�� �ý���
#ifdef SERV_PET_SYSTEM
	m_kUserPetManager.GetDBUpdatePetInfo( kPacket.m_vecPet );
#endif SERV_PET_SYSTEM
	//}}
	//{{ 2011. 01. 17	������	ĳ���� ī��Ʈ ����
#ifdef SERV_CHAR_LOG
	if( bFinal )
	{
		// ���� ��� ���ӽð�
		const int iDurationTime = static_cast<int>( m_kTimer[TM_UNIT_DURATION].elapsed() + 0.5 );
		m_kUserStatistics.IncreaseCharacterCount( KUserStatistics::CGCT_DUNRATION_TIME, iDurationTime / 60 );

		// DB ������Ʈ ���� ���
		m_kUserStatistics.GetDBUpdateInfo( kPacket.m_mapCharGameCount );
	}
#endif SERV_CHAR_LOG
	//}}

	//{{ 2011. 08. 12   ��μ�      ��ϸ� ���� 
#ifdef SERV_NEW_HENIR_TEST
	kPacket.m_kHenirRewardCnt.m_iNormal = m_kUserDungeonManager.GetHenirRewardNormalCount();
	kPacket.m_kHenirRewardCnt.m_iPremium = m_kUserDungeonManager.GetHenirRewardPremiumCount();
	kPacket.m_kHenirRewardCnt.m_iEvent = m_kUserDungeonManager.GetHenirRewardEventCount();
#endif SERV_NEW_HENIR_TEST
	//}}

	//{{ 2013. 3. 17	�ڼ���	 ���� ��ŷ �ý���
#ifdef SERV_LOCAL_RANKING_SYSTEM
	m_kUserLocalRankingInfo.GetChangedValue( kPacket.m_iChangedLocalRankingSpirit, kPacket.m_iChangedLocalRankingAP );
#endif SERV_LOCAL_RANKING_SYSTEM
	//}}

#ifdef	SERV_RIDING_PET_SYSTM// ���볯¥: 2013-04-21
	m_kUserRidingPetManager.GetDBUpdateInfo( kPacket.m_vecRidingPetList );
#endif	// SERV_RIDING_PET_SYSTM
	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
	kPacket.m_cUnitRelationshipType = m_kUserRelationshipManager.GetRelationshipType();
#endif SERV_RELATIONSHIP_SYSTEM
	//}
#ifdef SERV_GATE_OF_DARKNESS_SUPPORT_EVENT
	kPacket.m_iGateOfDarknessSupportEventTime = m_iGateOfDarknessSupportEventTime;
#endif SERV_GATE_OF_DARKNESS_SUPPORT_EVENT

    SendToGameDB( DBE_UPDATE_UNIT_INFO_REQ, kPacket ); // [�߿�] ���� ���� �ʱ�ȭ�� DBE_UPDATE_UNIT_INFO_ACK����..	
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	// DB������Ʈ Ÿ�̸� �ʱ�ȭ
    m_kTimer[TM_DB_UPDATE].restart();
	
	m_kEXP.SetExp( m_kEXP.GetExp() );
	//{{ 2012. 10. 02	������		ED&������ ����͸� �ý���
#ifdef SERV_ED_AND_ITEM_MORNITORING_SYSTEM
	// EDManager���ο��� SetValue�� �մϴ�.
#else
	m_iED.SetValue( m_iED );
#endif SERV_ED_AND_ITEM_MORNITORING_SYSTEM
	//}}
	m_iSPoint.SetValue( m_iSPoint );
	//{{ 2011. 06. 20	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
#else
	m_iVSPoint.SetValue( m_iVSPoint );
	m_iVSPointMax.SetValue( m_iVSPointMax );
#endif SERV_PVP_NEW_SYSTEM
	//}}

	//{{ 2007. 10. 16  ������  ���� ���
	if( bFinal  &&  ( GetAuthLevel() == SEnum::UAL_NORMAL || GetAuthLevel() == SEnum::UAL_BLOCKED_1 ) )
	{
		// ���� ��� ���ӽð�
		const int iDurationTime = static_cast<int>( m_kTimer[TM_UNIT_DURATION].elapsed() + 0.5 );
		m_kUserStatistics.IncreaseCount( KUserStatistics::USTable_EtcData, 0, KUserStatistics::US_Etc_Duration_Time, iDurationTime );

		// ���� ��� ���� ������
		//{{ 2012. 12. 14  ���� �̼� �ý��� ( �������� ����Ʈ ) - ��μ�
#ifdef SERV_ACCOUNT_MISSION_SYSTEM
		const int iObtainedItemCount = m_kUserQuestManager.GetQuestRewardItemCount() + m_kUserAccountQuestManager.GetQuestRewardItemCount();
#else
		const int iObtainedItemCount = m_kUserQuestManager.GetQuestRewardItemCount();
#endif SERV_ACCOUNT_MISSION_SYSTEM
		//}}
		m_kUserStatistics.IncreaseCount( KUserStatistics::USTable_EtcData, 0, KUserStatistics::US_Etc_Obtained_Item, iObtainedItemCount );

		//{{ 2010. 11. 17	������	������ ���� ���
#ifdef SERV_AUTO_HACK_CHECK_GET_ITEM
		m_kUserAbuserManager.IncreaseGetItemCount( GetThisPtr<KGSUser>(), iObtainedItemCount );
#endif SERV_AUTO_HACK_CHECK_GET_ITEM
		//}}

#ifdef SERV_USER_STATISTICS_RENEWAL
		CTime tDisconnectTime = CTime::GetCurrentTime();
#endif //SERV_USER_STATISTICS_RENEWAL

		// �ټ��� �Ҹ�
		const int iUsedSpirit = m_kUserStatistics.GetUsedSpirit();

		KELOG_USER_STATISTICS kPacketLog;
		kPacketLog.m_iUserUID = GetUID();
		kPacketLog.m_iUnitUID = GetCharUID();
#ifdef SERV_USER_STATISTICS_RENEWAL
		kPacketLog.m_iAuthLevel				=	 GetAuthLevel();
		kPacketLog.m_cUnitClass				=	 GetUnitClass();
		kPacketLog.m_iED					=	 GetED();
#ifdef SERV_PVP_NEW_SYSTEM
#else
		kPacketLog.m_iVSPoint				=	 GetVSPoint();
		kPacketLog.m_iVSPointMax			=	 GetVSPointMax();
#endif SERV_PVP_NEW_SYSTEM
		kPacketLog.m_ucLevel				=	 GetLevel();
		kPacketLog.m_uiGiantUID				=	 m_kNexonAccountInfo.m_uiNexonSN;
		kPacketLog.m_wstrLoginTime			=	 ( CStringW )( m_tConnectTime.Format( _T( "%Y-%m-%d %H:%M:%S" ) ) );
		kPacketLog.m_wstrLogoutTime			=	 ( CStringW )( tDisconnectTime.Format( _T( "%Y-%m-%d %H:%M:%S" ) ) );
		kPacketLog.m_wstrNickName			=	 GetCharName();
		kPacketLog.m_wstrAccountCreateDate	=	 m_kUserStatistics.GetAccountRegDate();
		kPacketLog.m_iEXP					=	 GetEXP();
		kPacketLog.m_wstrUnitCreateDate		=	 m_kUserStatistics.GetUnitCreateDate();
		kPacketLog.m_iiDurationTime			=	 iDurationTime;
#endif //SERV_USER_STATISTICS_RENEWAL

#ifdef SERV_USER_STATISTICS_RENEWAL
		m_kUserStatistics.GetCharacterStatistics( kPacketLog.m_mapPvpStatistics, kPacketLog.m_mapDungeonStatistics, kPacketLog.m_mapTCenterStatistics, kPacketLog.m_mapEtcStatistics, kPacketLog.m_mapEDStatistics );
#else //SERV_USER_STATISTICS_RENEWAL
		m_kUserStatistics.GetCharacterStatistics( kPacketLog.m_mapPvpStatistics, kPacketLog.m_mapDungeonStatistics, kPacketLog.m_mapTCenterStatistics, kPacketLog.m_mapEtcStatistics );
#endif //SERV_USER_STATISTICS_RENEWAL
		
		SendToLogDB( ELOG_USER_STATISTICS, kPacketLog );

		// LogDBThread�� �̺�Ʈ ���� �� ���� �ʱ�ȭ��Ų��
		m_kUserStatistics.Clear();

		//{{ 2010. 06. 30  ������	�ټ��� ���
#ifdef SERV_SPIRIT_STATISTICS
		KDBE_SPIRIT_STATISTICS_NOT kPacketNot;
		kPacketNot.m_iUnitUID = GetCharUID();
		//{{ 2012. 11. 14	�ڼ���	Field PT �α� �߰�
#ifdef SERV_FIELD_PLAY_TIME_LOG
		kPacketNot.m_iSpiritPlayTime = m_kUserStatistics.GetPlayTime( KUserStatistics::PT_SPIRIT_PLAY_TIME );
		m_kUserStatistics.ResetPlayTime( KUserStatistics::PT_SPIRIT_PLAY_TIME );
#else
		kPacketNot.m_iDungeonPlayTime = m_kUserStatistics.GetPlayTime( KUserStatistics::PT_DUNGEON_PLAY_TIME );
		m_kUserStatistics.ResetPlayTime( KUserStatistics::PT_DUNGEON_PLAY_TIME );
#endif SERV_FIELD_PLAY_TIME_LOG
		//}}
		kPacketNot.m_iDurationTime = iDurationTime;
		kPacketNot.m_iUsedSpirit = iUsedSpirit;
		SendToLogDB( DBE_SPIRIT_STATISTICS_NOT, kPacketNot );
#endif SERV_SPIRIT_STATISTICS
		//}}
	}
	//}}

	//{{ 2008. 11. 19  ������	����� �ڵ� ���	
	if( GetED() >= KUserAbuserManager::AE_ED_ABUSER_LIMIT )
	{
		//{{ 2010. 07. 05  ������	���� ����� �Ŵ���
#ifdef SERV_USER_ABUSER_MANAGER
		m_kUserAbuserManager.RegEDAbuser( GetThisPtr<KGSUser>() );
#endif SERV_USER_ABUSER_MANAGER
		//}}
	}
	//}}

	//{{ 2010. 9. 8	������	��Ʈ��ũ Ʈ���� ���� ���� ����͸�
#ifdef SERV_TRAFFIC_USER_MONITORING
	if( m_kUserAbuserManager.IsTrafficAbuser() == true )
	{
		KELOG_INSERT_TRAFFIC_ABUSER_LOG_NOT kPacketToLog;
		kPacketToLog.m_iUnitUID = GetCharUID();
		m_kUserAbuserManager.GetTrafficEventCount( kPacketToLog.m_mapTrafficEventCount );
		SendToLogDB( ELOG_INSERT_TRAFFIC_ABUSER_LOG_NOT, kPacketToLog );
	}
#endif SERV_TRAFFIC_USER_MONITORING
	//}}
}

void KGSUser::WritePlayTimeToDB()
{
	CTime tDisconnectTime = CTime::GetCurrentTime();
	KELOG_STAT_PLAY_TIME kPacket;
	kPacket.m_iUserUID		  = GetUID();
	kPacket.m_wstrIP		  = KncUtil::toWideString( GetIPStr() );
	kPacket.m_wstrLoginTime	  = ( CStringW )( m_tConnectTime.Format( _T( "%Y-%m-%d %H:%M:%S" ) ) );
	kPacket.m_wstrLogoutTime  = ( CStringW )( tDisconnectTime.Format( _T( "%Y-%m-%d %H:%M:%S" ) ) );	
	kPacket.m_bLoginFail	  = m_bLoginFail;
	kPacket.m_bLogoutFail	  = m_bLogoutFail;
	kPacket.m_iTotalPlayTime  = m_kUserStatistics.GetPlayTime( KUserStatistics::PT_TOTAL_PLAY_TIME );
	kPacket.m_iPvpPlayTime	  = m_kUserStatistics.GetPlayTime( KUserStatistics::PT_PVP_PLAY_TIME );
	//{{ 2012. 11. 14	�ڼ���	Field PT �α� �߰�
#ifdef SERV_FIELD_PLAY_TIME_LOG
	kPacket.m_iDungeonPlayTime	= m_kUserStatistics.GetPlayTime( KUserStatistics::PT_DUNGEON_PLAY_TIME );
	kPacket.m_iFieldPlayTime	= m_kUserStatistics.GetPlayTime( KUserStatistics::PT_FIELD_PLAY_TIME );
#endif SERV_FIELD_PLAY_TIME_LOG
	//}}
	kPacket.m_iDisconnectReason	= GetDisconnectReason();
	kPacket.m_iDisconnectUserFSM = GetStateID();
	//{{ 2009. 12. 22  ������	������Ȯ��
	kPacket.m_iServerGroupID  = KBaseServer::GetKObj()->GetServerGroupID();
	//}}
	//{{ 2011. 08. 09	������	�ӽ�ID �������
#ifdef SERV_MACHINE_ID_BLOCK
	kPacket.m_wstrMachineID	  = m_kUserSecurityManager.GetWStringMachineID();
#endif SERV_MACHINE_ID_BLOCK
	//}}
	//{{ 2011. 09. 27	��μ�	PC �� ������ ����
#ifdef SERV_PCBANG_INCORRECT_BILLING
	kPacket.m_wstrUnitID	= m_kUserStatistics.GetCharName();
	kPacket.m_bIsPcbang		= IsPcBang();
	kPacket.m_iUnitUID		= GetCharUID();
#endif SERV_PCBANG_INCORRECT_BILLING
	//}}

#ifdef SERV_CONNECT_LOG_CHANNELING
	kPacket.m_ucChannelCode = GetChannelCode();
#endif

#ifdef SERV_STEAM
	kPacket.m_bSteamClient = IsSteamClientUser();
#endif //SERV_STEAM

#ifdef SERV_CHANNELING_AERIA
	kPacket.m_bAeriaClient = IsAeriaClientUser();
#endif //SERV_CHANNELING_AERIA

	// StatisticsDB�� ���
	SendToLogDB( ELOG_STAT_PLAY_TIME, kPacket );

	// AccountDB�� ���� ���
	SendToAccountDB( ELOG_STAT_PLAY_TIME, kPacket );
}

void KGSUser::SendDisconnectEvent( u_short usEventID /*= -1*/ )
{
    if( GetStateID() >= KGSFSM::S_UID_UPDATED && GetStateID() < KGSFSM::STATE_SENTINEL )
    {
        if( m_bReportDisconnectToLogin )
        {
			KELG_USER_DISCONNECT_REQ kPacket;
			kPacket.m_usEventID = usEventID;
			//{{ 2011. 02. 23	������	���� ���� ī��Ʈ
#ifdef SERV_ACCOUNT_COUNT
			m_kUserStatistics.GetAccountDBUpdateInfo( kPacket.m_mapAccCountInfo );
#endif SERV_ACCOUNT_COUNT
			//}}
            SendToLoginServer( ELG_USER_DISCONNECT_REQ, kPacket );
        }
    }
}

//{{ 2009. 4. 3  ������		ġƮ�α�
void KGSUser::WriteCheatLogToDB( const wchar_t* szCheatCode, const std::wstring& wstrCheatContents )
{
	// ġƮ �α�
	KELOG_CHEAT_LOG_NOT kPacket;
	kPacket.m_iUserUID		  = GetUID();
	kPacket.m_iUnitUID		  = GetCharUID();
	kPacket.m_cAuthLevel	  = GetAuthLevel();
	kPacket.m_wsCheatCode	  = szCheatCode;
	kPacket.m_wsCheatContents = wstrCheatContents;

	if( KODBC::IsInvalidMarkInForCheat( kPacket.m_wsCheatCode ) )
	{
		START_LOG( cerr, L"SQL Injection ����" )
			<< BUILD_LOG( kPacket.m_wsCheatCode )
			<< END_LOG;
		return;
	}

	if( KODBC::IsInvalidMarkInForCheat( kPacket.m_wsCheatContents ) )
	{
		START_LOG( cerr, L"SQL Injection ����" )
			<< BUILD_LOG( kPacket.m_wsCheatContents )
			<< END_LOG;
		return;
	}

	SendToLogDB( ELOG_CHEAT_LOG_NOT, kPacket );
}

void KGSUser::WriteCheatLogToDB( const wchar_t* szCheatCode, const CStringW& cwstrCheatContents )
{
	std::wstring wstrCheatContents = static_cast<LPCTSTR>(cwstrCheatContents);
	WriteCheatLogToDB( szCheatCode, wstrCheatContents );
}

void KGSUser::WriteCheatLogToDB( const wchar_t* szCheatCode )
{
	std::wstring wstrTemp;
    WriteCheatLogToDB( szCheatCode, wstrTemp );
}
//}}

//{{ 2011. 01. 17	������	ĳ���� ī��Ʈ ����
#ifdef SERV_CHAR_LOG
void KGSUser::WriteCharacterLogToDB( IN const KELOG_USER_CHARACTER_LOG_NOT::LOG_TYPE eLogType )
{
	WriteCharacterLogToDBWithParam( eLogType, m_kUserStatistics.GetLastClearDungeonID(), 0 );
}

void KGSUser::WriteCharacterLogToDBWithDungeonID( IN const KELOG_USER_CHARACTER_LOG_NOT::LOG_TYPE eLogType, IN const int iDungeonID )
{
	WriteCharacterLogToDBWithParam( eLogType, iDungeonID, 0 );
}

void KGSUser::WriteCharacterLogToDBWithProductNo( IN const KELOG_USER_CHARACTER_LOG_NOT::LOG_TYPE eLogType, IN const int iProductNo )
{
	WriteCharacterLogToDBWithParam( eLogType, m_kUserStatistics.GetLastClearDungeonID(), iProductNo );
}

void KGSUser::WriteCharacterLogToDBWithParam( IN const KELOG_USER_CHARACTER_LOG_NOT::LOG_TYPE eLogType, IN const int iDungeonID, IN const int iProductNo )
{
	if( KELOG_USER_CHARACTER_LOG_NOT::IsValidLogType( eLogType ) == false )
	{
		START_LOG( cerr, L"�߸��� �α� Ÿ�� �Դϴ�!" )
			<< BUILD_LOG( eLogType )
			<< END_LOG;
		return;
	}
	
	// ĳ���� �α� ����� ����� �ƴϸ� ���⼭ ����!
	if( m_kUserStatistics.IsCharCount() == false )
		return;

	const int iDurationTime = static_cast<int>( m_kTimer[TM_UNIT_DURATION].elapsed() + 0.5 ) / 60;
	CTime tCurTime = CTime::GetCurrentTime();

	KELOG_USER_CHARACTER_LOG_NOT kPacketToLog;
	kPacketToLog.m_iUserUID				 = GetUID();
	kPacketToLog.m_iUnitUID				 = GetCharUID();
	kPacketToLog.m_wstrNickName 		 = GetCharName();
	kPacketToLog.m_bOnlyOneUnit 		 = ( m_setMyUnitUIDList.size() == 1 );
	kPacketToLog.m_iServerUID			 = KBaseServer::GetKObj()->GetUID();
	kPacketToLog.m_sLogType				 = eLogType;
	kPacketToLog.m_ucLevel				 = GetLevel();
	kPacketToLog.m_cUnitClass			 = GetUnitClass();
	kPacketToLog.m_iDungeonID			 = iDungeonID;
	kPacketToLog.m_iProductNo			 = iProductNo;
	kPacketToLog.m_iTotalDungeonPlayTime = m_kUserStatistics.GetCharacterCount( KUserStatistics::CGCT_DUNGEON_PLAY_TIME );
	kPacketToLog.m_iTotalPvpPlayTime	 = m_kUserStatistics.GetCharacterCount( KUserStatistics::CGCT_PVP_PLAY_TIME );
	kPacketToLog.m_iDurationTime		 = m_kUserStatistics.GetCharacterCount( KUserStatistics::CGCT_DUNRATION_TIME ) + iDurationTime;
	kPacketToLog.m_wstrAccountCreateDate = m_kUserStatistics.GetAccountRegDate();
	kPacketToLog.m_wstrUnitCreateDate	 = m_kUserStatistics.GetUnitCreateDate();
	kPacketToLog.m_wstrRegDate		 	 = ( std::wstring )( tCurTime.Format( _T( "%Y-%m-%d %H:%M:%S" ) ) );
	SendToLogDB( ELOG_USER_CHARACTER_LOG_NOT, kPacketToLog );
}
#endif SERV_CHAR_LOG
//}}

//{{ 2012. 11. 9	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
void KGSUser::DungeonGameStart_SingleOpenDungeonRoom( IN const char cDungeonMode, IN const int iDungeonID, IN const char cDifficultyLevel, IN const bool bAutoParty )
{
	const int iDungeonIDWithDif = iDungeonID + static_cast<int>(cDifficultyLevel);

	// ���� ���� ����鼭 ���� ������ ��������!
	KERM_OPEN_ROOM_REQ kPacket;
	kPacket.m_kRoomInfo.m_bStartedByAutoParty	= bAutoParty;
	kPacket.m_bQuickJoin						= false;
	kPacket.m_iCurED							= GetED();
	kPacket.m_iCurEXP							= GetEXP();
	kPacket.m_wstrChannelIP						= NetCommon::GetLocalIPW();
	kPacket.m_kRoomInfo.m_RoomType				= CXSLRoom::RT_DUNGEON;
	kPacket.m_kRoomInfo.m_uiRoomListID			= GetRoomListID();
	kPacket.m_kRoomInfo.m_RoomName				= L"ȥ�� �ϴ� ����";
	kPacket.m_kRoomInfo.m_bPublic				= false;
	kPacket.m_kRoomInfo.m_cDungeonMode			= cDungeonMode;
	kPacket.m_kRoomInfo.m_iDungeonID			= iDungeonID;
	kPacket.m_kRoomInfo.m_DifficultyLevel		= cDifficultyLevel;
	kPacket.m_kRoomInfo.m_fPlayTime				= DUNGEON_DEFAULT_LIMIT_PLAY_TIME;
	//{{ 2013. 04. 10	������	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
	kPacket.m_kRoomInfo.m_bDefenceDungeonOpen	= SiKGSWorldMissionManager()->GetIsActive();
#endif SERV_NEW_DEFENCE_DUNGEON
	//}}

	float fPlayTimeLimit = SiCXSLDungeonManager()->GetPlayTimeLimit( iDungeonIDWithDif );
	if( fPlayTimeLimit > 0.0f )
	{
		kPacket.m_kRoomInfo.m_fPlayTime = fPlayTimeLimit;
	}

	// ���� �÷��� ������ ����
	m_kUserDungeonManager.SetDungeonGameInfo( iDungeonID, cDifficultyLevel, cDungeonMode );

	// KRoomUserInfo���
	GetRoomUserInfo( kPacket.m_kRoomUserInfo, CXSLRoom::RT_DUNGEON );

	// �̱� ���� ���� ����!
	SendToCnRoom( ERM_SINGLE_OPEN_DUNGEON_ROOM_REQ, kPacket );
}
#endif SERV_BATTLE_FIELD_SYSTEM
//}}

void KGSUser::ExpireItem( bool bForce/* = false */ )
{
	if( GetStateID() > KGSFSM::S_SERVER_SELECT  &&  GetStateID() < KGSFSM::STATE_SENTINEL )
    {
        if( ( m_kTimer[TM_CHECK_ITEM_EXPIRATION].elapsed() < 60.0 * 5.0 ) && !bForce )
        {
            return;
        }

        m_kTimer[TM_CHECK_ITEM_EXPIRATION].restart();

#ifdef	SERV_RIDING_PET_SYSTM// ���볯¥: 2013-04-21
		CheckRidingPetProcess();
#endif	// SERV_RIDING_PET_SYSTM

#ifdef SERV_PERIOD_PET
		CheckExpirePet();
#endif SERV_PERIOD_PET

        KEGS_ITEM_EXPIRATION_NOT kPacket;
        std::vector< KInventoryItemInfo > vecInventorySlotInfo;
        m_kInventory.ExpireItem( kPacket.m_vecItemUID, vecInventorySlotInfo );
        if( kPacket.m_vecItemUID.empty() )
        {
            return;
        }

        SendPacket( EGS_ITEM_EXPIRATION_NOT, kPacket );

		//{{ 2008. 12. 3  ������	�ʵ忡�� �Ⱓ�ٵ� ������ ����
		if( GetStateID() == KGSFSM::S_FIELD_MAP )
		{
			if( GetFieldUID() <= 0 )
			{
				START_LOG( clog, L"�ʵ� �����ε� FieldUID�� ����." )
					<< BUILD_LOG( GetFieldUID() )
					<< BUILD_LOG( GetCharUID() )
#ifndef SERV_PRIVACY_AGREEMENT
					<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
					<< END_LOG;
				return;
			}

			KERM_CHANGE_EQUIPPED_ITEM_IN_FIELD_NOT kPacketNot;
			kPacketNot.m_iUnitUID = GetCharUID();
			std::vector< KInventoryItemInfo >::iterator vit;
			for( vit = vecInventorySlotInfo.begin(); vit != vecInventorySlotInfo.end(); vit++ )
			{
				if( vit->m_cSlotCategory == CXSLInventory::ST_E_EQUIP )
				{
					kPacketNot.m_vecInventorySlotInfo.push_back( *vit );
				}
			}

			if( kPacketNot.m_vecInventorySlotInfo.empty() )
				return;

			//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
#ifdef SERV_TITLE_DATA_SIZE
			kPacketNot.m_iEquippedTitleID = m_kUserTitleManager.GetEquippedTitleID();
#else
			kPacketNot.m_sEquippedTitleID = m_kUserTitleManager.GetEquippedTitleID();
#endif SERV_TITLE_DATA_SIZE
			//}}			
			SendToGSField( ERM_CHANGE_EQUIPPED_ITEM_IN_FIELD_NOT, kPacketNot );   
		}
		//}}
        else if( GetStateID() == KGSFSM::S_ROOM )
        {
            if( GetRoomUID() <= 0 )
            {
                START_LOG( cerr, L"�� UID �̻�." )
                    << BUILD_LOG( GetRoomUID() )
                    << BUILD_LOG( GetCharUID() )
#ifndef SERV_PRIVACY_AGREEMENT
                    << BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
                    << END_LOG;
                return;
            }

            switch( CXSLRoom::GetRoomType( GetRoomUID() ) )
            {
            case CXSLRoom::RT_PVP:
            case CXSLRoom::RT_DUNGEON:
            case CXSLRoom::RT_TRAININGCENTER:
                {
                    KERM_CHANGE_EQUIPPED_ITEM_IN_ROOM_NOT kPacketNot;
					//{{ 2010. 12. 23	������	��� ���� ��Ŷ Ÿ��
#ifdef SERV_CHANGE_EQUIPPED_PACKET_REFAC
					kPacketNot.m_cChangeEquippedPacketType = KEGS_CHANGE_EQUIPPED_ITEM_IN_ROOM_NOT::CEPT_CHANGE_EQUIP;
#endif SERV_CHANGE_EQUIPPED_PACKET_REFAC
					//}}
                    kPacketNot.m_UnitUID = GetCharUID();
                    std::vector< KInventoryItemInfo >::iterator vit;
                    for( vit = vecInventorySlotInfo.begin(); vit != vecInventorySlotInfo.end(); vit++ )
                    {
                        if( vit->m_cSlotCategory == CXSLInventory::ST_E_EQUIP )
                        {
                            kPacketNot.m_vecInventorySlotInfo.push_back( *vit );
                        }
                    }

                    if( kPacketNot.m_vecInventorySlotInfo.empty() )
                    {
                        return;
                    }

					//080421.hoons.
					//�ູ�� �������� �������� GetRoomUserInfo() ȣ���Ҷ� ������Ʈ�� �Ǳ� ������ �ٸ�ó����
					//�������� �ʴ´�.
                    GetRoomUserInfo( kPacketNot.m_kRoomUserInfo );
                    SendToCnRoom( ERM_CHANGE_EQUIPPED_ITEM_IN_ROOM_NOT, kPacketNot );
                }
                break;

            case CXSLRoom::RT_SQUARE:
                {
                    KERM_CHANGE_EQUIPPED_ITEM_IN_SQUARE_NOT kPacketNot;
                    kPacketNot.m_UnitUID = GetCharUID();
                    std::vector< KInventoryItemInfo >::iterator vit;
                    for( vit = vecInventorySlotInfo.begin(); vit != vecInventorySlotInfo.end(); vit++ )
                    {
                        if( vit->m_cSlotCategory == CXSLInventory::ST_E_EQUIP )
                        {
                            kPacketNot.m_vecInventorySlotInfo.push_back( *vit );
                        }
                    }

                    if( kPacketNot.m_vecInventorySlotInfo.empty() )
                    {
                        return;
                    }

					//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
#ifdef SERV_TITLE_DATA_SIZE
					kPacketNot.m_iEquippedTitleID = m_kUserTitleManager.GetEquippedTitleID();
#else
					kPacketNot.m_sEquippedTitleID = m_kUserTitleManager.GetEquippedTitleID();
#endif SERV_TITLE_DATA_SIZE
					//}}					
                    SendToCnRoom( ERM_CHANGE_EQUIPPED_ITEM_IN_SQUARE_NOT, kPacketNot );
                }
                break;

            default:
                {
                    START_LOG( cerr, L"�̻��� �� Ÿ��." )
                        << BUILD_LOG( CXSLRoom::GetRoomType( GetRoomUID() ) )
                        << BUILD_LOG( GetCharUID() )
                        << END_LOG;
                }
                break;
            }
        }
    }
}

void KGSUser::ExpireCashSkillPoint( bool bForce /*= false*/ )
{
	if( GetStateID() > KGSFSM::S_SERVER_SELECT  &&  GetStateID() < KGSFSM::STATE_SENTINEL )
	{
		if( ( m_kTimer[TM_CHECK_CASH_SKILL_POINT_EXPIRATION].elapsed() < 60.0 * 5.0 ) && !bForce )		
			return;

		m_kTimer[TM_CHECK_CASH_SKILL_POINT_EXPIRATION].restart();

		if( m_kSkillTree.IsCashSkillPointExpired() == false )
		{
			CTime tCurrentTime = CTime::GetCurrentTime();
			if( tCurrentTime > m_kSkillTree.GetCSPointEndDate() )
			{
				//if( false == IsTransactionEnabled( DBE_EXPIRE_CASH_SKILL_POINT_REQ ) )
				//{
				//	START_LOG( cwarn, L"ĳ�� ��ų ����Ʈ �Ⱓ���� ó���Ϸ��µ� �ٸ� ��ų ���� �۾����̼� ó�� ����, 5�� �Ŀ� �ٽ� ó��!" )
				//		<< BUILD_LOG( GetCharUID() )
				//		<< BUILD_LOG( GetCharName() )
				//		<< BUILD_LOGc( GetUnitClass() )
				//		<< END_LOG;
				//	return;
				//}

				// ��� ��ų Ʈ���߿��� CSP�� ��� ��ų �ǵ�����, ���� CSP ����Ʈ ����ϰ� m_iMaxCSPoint�� ���� SP�� �����ְ�
				// ������ ��ų�߿� �ȹ�� ��ų�� ������ Ż���Ѵ�
				KDBE_EXPIRE_CASH_SKILL_POINT_REQ KDBReq;
				KDBReq.m_iUnitUID = GetCharUID();
				m_kSkillTree.CalcExpireCashSkillPoint( KDBReq.m_iRetrievedSPoint, KDBReq.m_vecUserSkillData );
				SendToGameDB( DBE_EXPIRE_CASH_SKILL_POINT_REQ, KDBReq );

				//{{ 2010. 07. 04  ������	��ų ����Ʈ ���� Ȯ��
#ifdef SERV_CHECK_SKILL_POINT
				if( CheckSkillPointError() == false )
				{
					if( GetAuthLevel() < SEnum::UAL_GM )
					{
						START_LOG( cerr, L"ĳ�� ��ų ����Ʈ �Ⱓ ���� ������ ��ų ����Ʈ ���� �������Դϴ�." )
#ifdef SERV_PRIVACY_AGREEMENT
							<< BUILD_LOG( GetUID() )
							<< BUILD_LOG( GetCharUID() )
#else
							<< BUILD_LOG( GetName() )
							<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
							<< BUILD_LOG( KDBReq.m_iRetrievedSPoint )
							<< BUILD_LOG( KDBReq.m_vecUserSkillData.size() )
							<< END_LOG;
					}
				}
#endif SERV_CHECK_SKILL_POINT
				//}}
			}
		}
	}
}

void KGSUser::ExpireSkillSlotB( bool bForce /*= false*/ )
{
	if( GetStateID() > KGSFSM::S_SERVER_SELECT  &&  GetStateID() < KGSFSM::STATE_SENTINEL )
	{
		if( ( m_kTimer[TM_SKILL_SLOT_EXPIRATION].elapsed() < 60.0 * 5.0 ) && !bForce )		
			return;

		m_kTimer[TM_SKILL_SLOT_EXPIRATION].restart();

		// �Ⱓ�� �˻�
		m_kSkillTree.ExpireSkillSlotB();
		SendID( EGS_EXPIRE_SKILL_SLOT_NOT );
	}
}

void KGSUser::ExpireTitle( bool bForce /*= false*/ )
{
	if( GetStateID() > KGSFSM::S_SERVER_SELECT  &&  GetStateID() < KGSFSM::STATE_SENTINEL )
	{
		if( ( m_kTimer[TM_CHECK_TITLE_EXPIRATION].elapsed() < 60.0 * 5.0 ) && !bForce )
			return;

		m_kTimer[TM_CHECK_TITLE_EXPIRATION].restart();

		// �Ⱓ�� �˻�
		m_kUserTitleManager.ExpireTitle( GetThisPtr<KGSUser>() );
	}
}

void KGSUser::CheckTimeEvent()
{
	if( GetStateID() > KGSFSM::S_SERVER_SELECT  &&  GetStateID() < KGSFSM::STATE_SENTINEL )
	{
		if( m_kTimer[TM_EVENT_CHECK].elapsed() < 60.0 )
			return;

		m_kTimer[TM_EVENT_CHECK].restart();

#ifdef SERV_ITEM_IN_INVENTORY_CONNECT_EVENT
		std::set< int > setItemID;
		m_kInventory.GetItemIDSet( setItemID );
#endif SERV_ITEM_IN_INVENTORY_CONNECT_EVENT
		
		// ���� �ð� �̺�Ʈ üũ
		//{{ 2012. 10. 13	�ڼ���	���� �̺�Ʈ ���� ����
#ifdef SERV_CONNECT_EVENT_FACTOR_SET
		char cAPinkEventUserType	= 0;
		int	iAPinkScriptID			= -1;

		KGameEventManager::ConnectEventFactorSet kFactor(
			GetCharUID(),
			GetLevel(),
			IsPcBang(),
			GetChannelCode(),
#ifdef SERV_BUBBLE_FIGHTER_TOGETHER_EVENT	// ���� ������ �ؿ��� �߰�
			GetBF_Team(),
#endif // SERV_BUBBLE_FIGHTER_TOGETHER_EVENT
			GetUnitType(),
			GetUnitClass(),
			false

			//{{ 2012. 04. 13	�ڼ���	( ���� ���� ǥ�� )
#ifdef SERV_EVENT_RETURN_USER_MARK_SCRIPT
#else
			//{{ 2012. 03. 26	�ڼ���	�Ƹ����� ���� ������ ���� ����! ( ���� ���� ǥ�� )
#ifdef SERV_EVENT_RETURN_USER_MARK
			, IsEventReturnUser()
#endif SERV_EVENT_RETURN_USER_MARK
			//}}
#endif SERV_EVENT_RETURN_USER_MARK_SCRIPT
			//}}
			//{{ 2012. 06. 07	�ڼ���	���ϸ��� ���� ����
#ifdef SERV_EVENT_DAILY_GIFT_BOX
			, m_mmapDailyGiftBoxList
#endif SERV_EVENT_DAILY_GIFT_BOX
			//}}
			, m_kComeBackUserInfo.GetIsComeBackUser()
			//{{ 2012. 12. 11	�ڼ���	���� ���� �̺�Ʈ �۾�
#ifdef SERV_FIXED_DATE_EVENT
			, m_kAccountInfo
#endif SERV_FIXED_DATE_EVENT
			//}}
			//{{ 2012. 12. 12	�ڼ���	�ܿ� ���� ���� �̺�Ʈ( �ӽ�, �ϵ� �ڵ� )
#ifdef SERV_2012_WINTER_VACATION_EVENT
			, m_wstrWinterVacationEventRegDate
			, m_iWinterVacationEventCount
#endif SERV_2012_WINTER_VACATION_EVENT
			//}}
			//{{ 2012. 12. 25	�ڼ���	Ư�� ���� ���� ���� �̺�Ʈ
#ifdef SERV_SPECIFIC_USER_CONNECT_EVENT
			, GetUID()
#endif SERV_SPECIFIC_USER_CONNECT_EVENT
			//}}
#ifdef SERV_ITEM_IN_INVENTORY_CONNECT_EVENT
			, setItemID
#endif SERV_ITEM_IN_INVENTORY_CONNECT_EVENT
#ifdef SERV_CUSTOM_CONNECT_EVENT
			, GetCustonEventID()
#endif //SERV_CUSTOM_CONNECT_EVENT
#ifdef	SERV_CRITERION_DATE_EVENT// ���볯¥: 2013-04-11
			, GetCriterionDateEventInfo()
			, GetAccountRegDate()
#endif	// SERV_CRITERION_DATE_EVENT
#ifdef SERV_STEAM_USER_CONNECT_EVENT
			, IsPureSteamUser()
#endif //SERV_STEAM_USER_CONNECT_EVENT
#ifdef SERV_TIME_EVENT_ONLY_CURRENT_USER_CHAR
			,IsNewUnit()	
			,IsNewUnit2()	
			,IsCurrentUnit()
#endif //SERV_TIME_EVENT_ONLY_CURRENT_USER_CHAR
#ifdef SERV_NEW_USER_QUEST_HARD_CODING
			, GetAccountRegDate()
#endif	// SERV_NEW_USER_QUEST_HARD_CODING
			);

		m_kUserEventManager.CheckConnectTimeEvent( GetThisPtr<KGSUser>(), kFactor );
#else
		m_kUserEventManager.CheckConnectTimeEvent( GetThisPtr<KGSUser>()
			//{{ 2012. 06. 07	�ڼ���	���ϸ��� ���� ����
#ifdef SERV_EVENT_DAILY_GIFT_BOX
			, GetCharUID()
			, m_mmapDailyGiftBoxList
#endif SERV_EVENT_DAILY_GIFT_BOX
			//}}
#ifdef SERV_ITEM_IN_INVENTORY_CONNECT_EVENT
			, setItemID
#endif SERV_ITEM_IN_INVENTORY_CONNECT_EVENT
			);
#endif SERV_CONNECT_EVENT_FACTOR_SET
		//}}

		//{{ 2009. 12. 7  ������	�����ð��̺�Ʈ
#ifdef CUMULATIVE_TIME_EVENT
		//{{ 2012. 12. 12	�ڼ���	�ܿ� ���� ���� �̺�Ʈ( �ӽ�, �ϵ� �ڵ� )
#ifdef SERV_2012_WINTER_VACATION_EVENT
		m_kUserEventManager.CheckCumulativeTimeEvent( GetThisPtr<KGSUser>(), kFactor );
#else
		m_kUserEventManager.CheckCumulativeTimeEvent( GetThisPtr<KGSUser>() );
#endif SERV_2012_WINTER_VACATION_EVENT
		//}}
#endif CUMULATIVE_TIME_EVENT
		//}}

		//{{ 2010. 02. 06  ������	���� �̺�Ʈ ����Ʈ
#ifdef SERV_DAY_QUEST
		m_kUserQuestManager.CheckQuestEvent( GetThisPtr<KGSUser>() );
#endif SERV_DAY_QUEST
		//}}
		//{{ QUEST ���� - ��μ�
#ifdef SERV_REFORM_QUEST
		m_kUserQuestManager.CheckResetDayEventQuest_AutoAccept( GetThisPtr<KGSUser>() );
#endif SERV_REFORM_QUEST
		//}}

		//{{ 2013. 03. 21	 ���� ����Ʈ - ���Ϻ� Ȱ��ȭ ��� - ��μ�
#ifdef SERV_ACCOUNT_QUEST_DAY_OF_WEEK
		m_kUserAccountQuestManager.CheckResetDayAccountQuest_AutoAccept( GetThisPtr<KGSUser>() );
#endif SERV_ACCOUNT_QUEST_DAY_OF_WEEK
		//}

#ifdef SERV_ADVERTISEMENT_EVENT
		m_kUserEventManager.CheckAdvertisementEvent( GetThisPtr<KGSUser>() );
#endif SERV_ADVERTISEMENT_EVENT

	}
}

//{{ 2012. 04. 13	�ڼ���	( ���� ���� ǥ�� )
#ifdef SERV_EVENT_RETURN_USER_MARK_SCRIPT
void KGSUser::CheckCriterionEvent( void )
{
	if( ( KGSFSM::S_SERVER_SELECT <= GetStateID() ) && ( GetStateID() < KGSFSM::STATE_SENTINEL ) )
	{
		if( m_kTimer[TM_CRITERION_EVENT_CHECK].elapsed() < 60.0 )
			return;

		m_kTimer[TM_CRITERION_EVENT_CHECK].restart();

		// ���� �ð� ���� ���� �̺�Ʈ üũ
		m_kUserEventManager.CheckCriterionEvent( GetThisPtr<KGSUser>() );
	}
}
#endif SERV_EVENT_RETURN_USER_MARK_SCRIPT
//}}

//{{ 2009. 10. 14  ������	�ڵ�����
#ifdef AP_RESTONE
void KGSUser::CheckAutoPayment()
{
	if( GetStateID() > KGSFSM::S_SERVER_SELECT  &&  GetStateID() < KGSFSM::STATE_SENTINEL )
	{
		if( m_kTimer[TM_CHECK_AUTO_PAYMENT].elapsed() < 300.0 )
			return;

		m_kTimer[TM_CHECK_AUTO_PAYMENT].restart();

		if( m_tAutoPaymentCheckTime.GetTime() == 0 )
			return;

		// ��Ȱ�� ������ 1���̻��̸� üũ���� �ʽ��ϴ�.
		if( m_iNumAutoPaymentResStone > 0 )
			return;

		CTime tCurTime = CTime::GetCurrentTime();
		if( m_tAutoPaymentCheckTime < tCurTime )
		{
			std::vector< unsigned long > vecAutoPaymentProductList;
			SiKNexonBillingTCPManager()->GetAutoPaymentProductList( vecAutoPaymentProductList );

			KENX_BT_CHECK_AUTO_PAYMENT_USER_REQ kPacketReq;
			kPacketReq.m_ulPacketNo		= SiKNexonBillingTCPManager()->GetNextPacketNo();
			kPacketReq.m_bytePacketType	= KNexonBillingTCPPacket::CHECK_AUTO_PAYMENT_USER;
			kPacketReq.m_wstrGameID		= GetName(); // �κ��丮 ������ ���� Nexon ID�� ����.
			kPacketReq.m_wstrUserID		= GetName();			

			if( KSimLayer::GetKObj()->GetBillingFlag() == KSimLayer::BF_NEXON_KOREA )
			{
				kPacketReq.m_ulProductNo = 67215; // ���� : �ں� ����Ʈ ���� 
			}
			else
			{
				kPacketReq.m_ulProductNo = 77697; // �׼� : �ں� ����Ʈ ���� 
			}

			KEventPtr spEvent( new KEvent );
			UidType anTrace[2] = { GetUID(), -1 };
			spEvent->SetData( PI_GS_NX_BILLING_TCP, anTrace, ENX_BT_CHECK_AUTO_PAYMENT_USER_REQ, kPacketReq );
			SiKNexonBillingTCPManager()->QueueingEvent( spEvent );

			// �ڵ����� üũ �ð� �������Ƿ� ������ 6�÷� �ٽ� üũ�ð� ����!
			m_tAutoPaymentCheckTime += CTimeSpan( 1, 0, 0, 0 );
		}
	}
}
#endif AP_RESTONE
//}}

//{{ 2010. 02. 16  ������	��ŷ�� ����Ʈ
#ifdef SERV_HACKING_TOOL_LIST

void KGSUser::CheckHackingToolList()
{
	if( m_kTimer[TM_HACKING_TOOL_LIST].elapsed() < m_kUserHackingManager.GetHackingToolCheckTime() )
		return;

	m_kTimer[TM_HACKING_TOOL_LIST].restart();

	KEGS_HACKING_TOOL_LIST_NOT kNot;
	m_kUserHackingManager.GetHackingToolList( kNot.m_iRandomKey, kNot.m_vecHackingToolList );
	SendPacket( EGS_HACKING_TOOL_LIST_NOT, kNot );
}
#endif SERV_HACKING_TOOL_LIST
//}}

//{{ 2010. 7. 30 ������	�� �ý���
#ifdef SERV_PET_SYSTEM

void KGSUser::CheckAutoFeed()
{
	// �ڵ� ���� ��� üũ!
	KEGS_CHANGED_PET_SATIETY_NOT kNotSatiety;
	KEGS_CHANGED_PET_INTIMACY_NOT kNotIntimacy;
	KEGS_AUTO_FEED_NOT kAutoFeedPacket;
	bool bIsFirstFeed = false;

	//{{ 2011. 05. 23  ��μ�	ť�ǿ� ���� ģ�е� ����
#ifdef SERV_ADD_KEWPIEL_JELLY
	int iAddIntimacy  = 0;
	if( m_kUserPetManager.CheckAutoFeed( GetThisPtr<KGSUser>(), bIsFirstFeed, kNotSatiety, kNotIntimacy, kAutoFeedPacket, iAddIntimacy ) == true )
#else
	if( m_kUserPetManager.CheckAutoFeed( GetThisPtr<KGSUser>(), bIsFirstFeed, kNotSatiety, kNotIntimacy, kAutoFeedPacket ) == true )
#endif SERV_ADD_KEWPIEL_JELLY
	//}}
	{
		SendToCnRoom( ERM_CHANGED_PET_SATIETY_NOT, kNotSatiety );
		if( bIsFirstFeed )
		{
			SendToCnRoom( ERM_CHANGED_PET_INTIMACY_NOT, kNotIntimacy );
		}
		SendPacket( EGS_AUTO_FEED_NOT, kAutoFeedPacket );

		// ù���̶�� ù ���� ��Ŷ�� ������!
		if( bIsFirstFeed == true )
		{
			KEGS_FIRST_FEED_PETS_NOT kPacketNot;
			kPacketNot.m_bAutoFeed = true;
			SendPacket( EGS_FIRST_FEED_PETS_NOT, kPacketNot );
		}		

		// ���� ������ �ֺ� �������� ���� �Դ� �׼��� ������!
		SendPetAction( CXSLPetManager::PAC_EAT );

		//{{ 2011. 05. 16  ��μ�	Īȣ ȹ�� ���� �߰�
#ifdef SERV_ADD_TITLE_CONDITION
		const CXSLItem::ItemTemplet* pItemTemplet = SiCXSLItemManager()->GetItemTemplet( kAutoFeedPacket.m_iFeedItemID );
		if( pItemTemplet == NULL )
		{
			START_LOG( cerr, L"�������� �ʴ� �������Դϴ�!" )
				<< BUILD_LOG( kAutoFeedPacket.m_iFeedItemID )
				<< END_LOG;

			return;
		}

		if( m_kUserTitleManager.GivePetFeed( pItemTemplet->m_ItemGrade, GetThisPtr<KGSUser>() ) == true )
		{
			// �Ϸ�� Īȣ �̼��� ������ �� �̼��� �ִ��� �˻��Ѵ�
			m_kUserTitleManager.CheckNewMission( GetThisPtr<KGSUser>(), true );
		}
#endif SERV_ADD_TITLE_CONDITION
		//}}
	}
}

//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER
void KGSUser::CheckUpdateWorldMission()
{
	//	ƽī��Ʈ�� ���ϰų� 60�ʸ��� �ѹ��� ������Ʈ
	if( SiKGSWorldMissionManager()->GetTickCount() == m_iWorldMissionTickCount )
		return;

	m_iWorldMissionTickCount = SiKGSWorldMissionManager()->GetTickCount();

	//	�����ޱ�
	KEGS_WORLD_MISSION_UPDATE_NOT kNot;

	SiKGSWorldMissionManager()->GetMissionInfo( kNot );

	//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
	CTime tCurr = CTime::GetCurrentTime();

	KELG_UPDATE_USER_UNIT_BUFF_INFO_REQ kBuffReq;
	kBuffReq.m_iUnitUID = GetCharUID();
	int iWorldBuff = SiKGSWorldMissionManager()->GetWorldBuffState();
	int iWorldBuffPriod = SiKGSWorldMissionManager()->GetWorldBuffEndTime();

	//{{ 2013. 04. 15	�ڼ���	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
#ifdef SERV_NEW_DEFENCE_DUNGEON_NO_USE_DEFENSE_BUFF_AND_DEBUFF
#else //SERV_NEW_DEFENCE_DUNGEON_NO_USE_DEFENSE_BUFF_AND_DEBUFF
	if( iWorldBuff == CXSLBuffManager::BTI_NONE )
	{
		DeactivateBuff( CXSLBuffManager::BTI_BUFF_2013_DEFENSE_DEBUFF, kBuffReq.m_vecDeActivateBuff );
	}
	else
	{
		DeactivateBuff( CXSLBuffManager::BTI_BUFF_2013_DEFENSE_FAKE_BUFF, kBuffReq.m_vecDeActivateBuff );
		DeactivateBuff( iWorldBuff, kBuffReq.m_vecDeActivateBuff );
		ActivateBuff( iWorldBuff, kBuffReq.m_vecActivateBuff, tCurr, iWorldBuffPriod );
	}

	const int iIndividualBuffID = SiKGSWorldMissionManager()->GetIndividualBuffID();
	int iBTI_BUFF_2013_DEFENSE_BUFF = CXSLBuffManager::BTI_BUFF_2013_DEFENSE_BUFF;

	if( iIndividualBuffID == CXSLBuffManager::BTI_NONE )
	{
		DeactivateBuff( CXSLBuffManager::BTI_BUFF_2013_DEFENSE_FAKE_BUFF, kBuffReq.m_vecDeActivateBuff );
		DeactivateBuff( iBTI_BUFF_2013_DEFENSE_BUFF, kBuffReq.m_vecDeActivateBuff );
	}
	else if( iIndividualBuffID == CXSLBuffManager::BTI_BUFF_2013_DEFENSE_BUFF )
	{
		KBuffInfo kBuff;
		if( m_kUserBuffManager.GetActivateBuff( CXSLBuffManager::BTI_BUFF_2013_DEFENSE_FAKE_BUFF, kBuff ) == true )
		{
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_2013_DEFENSE_FAKE_BUFF, kBuffReq.m_vecDeActivateBuff );

			CTime tCurrent = CTime::GetCurrentTime();
			int iRemainTime = static_cast<int>( kBuff.m_iBuffEndTime - tCurrent.GetTime() );

			if( 0 < iRemainTime )
			{
				DeactivateBuff( iBTI_BUFF_2013_DEFENSE_BUFF, kBuffReq.m_vecDeActivateBuff );
				ActivateBuff( iBTI_BUFF_2013_DEFENSE_BUFF, kBuffReq.m_vecActivateBuff, 0, iRemainTime );
			}
		}
	}
#endif //SERV_NEW_DEFENCE_DUNGEON_NO_USE_DEFENSE_BUFF_AND_DEBUFF
#else
	if( iWorldBuff != CXSLBuffManager::BTI_NONE )
	{
		DeactivateBuff( CXSLBuffManager::BTI_BUFF_THANKS_OF_RESIDENTS, kBuffReq.m_vecDeActivateBuff );
		DeactivateBuff( CXSLBuffManager::BTI_BUFF_BREATH_OF_DRAGON, kBuffReq.m_vecDeActivateBuff );
		DeactivateBuff( CXSLBuffManager::BTI_BUFF_BLESSING_OF_EL, kBuffReq.m_vecDeActivateBuff );

		switch( iWorldBuff )
		{
		case CXSLBuffManager::BTI_BUFF_THANKS_OF_RESIDENTS:
			{
				ActivateBuff( CXSLBuffManager::BTI_BUFF_THANKS_OF_RESIDENTS, kBuffReq.m_vecActivateBuff, tCurr );
			}break;
		case CXSLBuffManager::BTI_BUFF_BREATH_OF_DRAGON:
			{
				ActivateBuff( CXSLBuffManager::BTI_BUFF_THANKS_OF_RESIDENTS, kBuffReq.m_vecActivateBuff, tCurr );
				ActivateBuff( CXSLBuffManager::BTI_BUFF_BREATH_OF_DRAGON, kBuffReq.m_vecActivateBuff, tCurr );
			}break;
		case CXSLBuffManager::BTI_BUFF_BLESSING_OF_EL:
			{
				ActivateBuff( CXSLBuffManager::BTI_BUFF_THANKS_OF_RESIDENTS, kBuffReq.m_vecActivateBuff, tCurr );
				ActivateBuff( CXSLBuffManager::BTI_BUFF_BREATH_OF_DRAGON, kBuffReq.m_vecActivateBuff, tCurr );
				ActivateBuff( CXSLBuffManager::BTI_BUFF_BLESSING_OF_EL, kBuffReq.m_vecActivateBuff, tCurr );
			}break;
		}
	}
	else
	{
		DeactivateBuff( CXSLBuffManager::BTI_BUFF_THANKS_OF_RESIDENTS, kBuffReq.m_vecDeActivateBuff );
		DeactivateBuff( CXSLBuffManager::BTI_BUFF_BREATH_OF_DRAGON, kBuffReq.m_vecDeActivateBuff );
		DeactivateBuff( CXSLBuffManager::BTI_BUFF_BLESSING_OF_EL, kBuffReq.m_vecDeActivateBuff );
	}
#endif SERV_NEW_DEFENCE_DUNGEON
	//}}

	if(  kBuffReq.m_vecActivateBuff.empty() == false || kBuffReq.m_vecDeActivateBuff.empty() == false )
	{
		SendToLoginServer( ELG_UPDATE_USER_UNIT_BUFF_INFO_REQ, kBuffReq );
	}
#endif SERV_SERVER_BUFF_SYSTEM
	//}

	//	�������� ��Ŷ������
	SendPacket( EGS_WORLD_MISSION_UPDATE_NOT, kNot );
}
#endif SERV_INSERT_GLOBAL_SERVER
//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�

bool KGSUser::AutoFeedInPetInventory( IN bool bOnlyEatSpecial, OUT int& iFeedItemID, OUT std::vector< KInventoryItemInfo >& vecPetInventorySlotInfo )
{
	return m_kInventory.AutoFeedInPetInventory( GetLevel(), bOnlyEatSpecial, iFeedItemID, vecPetInventorySlotInfo );
}

//{{ 2012. 12. 24	�ڼ���	�� ���� ���� ��� �߰�
#ifdef SERV_PET_AUTO_LOOTING
int KGSUser::CanIUseTheAutoLootingItem( IN const UidType iPetUID )
{
	KUserPetPtr spPet = m_kUserPetManager.GetPet( iPetUID );
	if( spPet == NULL )
	{
		START_LOG( cerr, L"���� �ϰ� ���� �ʴ� PetUID�Դϴ�." )
			<< BUILD_LOG( iPetUID )
			<< END_LOG;
		return NetError::ERR_PET_26;
	}

	// 2. ��ȯ�� ���� ������ ���� �����ΰ�?
	if( spPet->GetEvolutionStep() == 0 )
	{
		if( SiCXSLPetManager()->IsEvolutionExceptionPet( spPet->GetPetID() ) == false )
		{
			return NetError::ERR_PET_27;
		}
	}


	// 3. ��ȯ�� ���� �̹� ���� ���� ����� ��������� ������?
	if( spPet->IsAutoLootingPet() == true )
	{
		return NetError::ERR_PET_28;
	}

	return NetError::NET_OK;
}

void KGSUser::SetAutoLootingPet( IN const UidType iPetUID )
{
	KUserPetPtr spPet = m_kUserPetManager.GetPet( iPetUID );
	if( spPet == NULL )
	{
		START_LOG( cerr, L"���� �ϰ� ���� �ʴ� PetUID�Դϴ�." )
			<< BUILD_LOG( iPetUID )
			<< END_LOG;
		return;
	}
	spPet->SetAutoLootingPet();
}
#endif SERV_PET_AUTO_LOOTING
//}}

#ifdef SERV_HALLOWEEN_PUMPKIN_FAIRY_PET
int KGSUser::CanIUseTransformItem( IN const UidType iPetUID )
{
	KUserPetPtr spPet = m_kUserPetManager.GetPet( iPetUID );
	if( spPet == NULL )
	{
		START_LOG( cerr, L"���� �ϰ� ���� �ʴ� PetUID�Դϴ�." )
			<< BUILD_LOG( iPetUID )
			<< END_LOG;
		return NetError::ERR_PET_26;
	}

	// 1. ��ȯ�� ���� ���� ���� ����� �� �ִ� ���ΰ�?
	if( spPet->GetPetID() != CXSLPetManager::PUI_HALLOWEEN_PUMPKIN_TANYA && spPet->GetPetID() != CXSLPetManager::PUI_HALLOWEEN_PUMPKIN_LAEL &&
		spPet->GetPetID() != CXSLPetManager::PUI_HALLOWEEN_PUMPKIN_LOJETA && spPet->GetPetID() != CXSLPetManager::PUI_HALLOWEEN_PUMPKIN_NARENEA &&
		spPet->GetPetID() != CXSLPetManager::PUI_HALLOWEEN_PUMPKIN_TANYA_F && spPet->GetPetID() != CXSLPetManager::PUI_HALLOWEEN_PUMPKIN_LAEL_F &&
		spPet->GetPetID() != CXSLPetManager::PUI_HALLOWEEN_PUMPKIN_LOJETA_F && spPet->GetPetID() != CXSLPetManager::PUI_HALLOWEEN_PUMPKIN_NARENEA_F
		)
	{
		START_LOG( cerr, L"���� ������ ����Ҽ� �ִ� ���� �ƴմϴ�." )
			<< BUILD_LOG( iPetUID )
			<< END_LOG;
		return NetError::ERR_HW_PET_02;
	}

	// 2. ��ȯ�� ���� ������ ����ü �����ΰ�?
	if( spPet->GetEvolutionStep() != 3 )
	{
		if( SiCXSLPetManager()->IsEvolutionExceptionPet( spPet->GetPetID() ) == false )
		{
			return NetError::ERR_HW_PET_02;
		}
	}

	// 3. ��ȯ�� ���� ģ�е� 100% �ΰ�?
	if( ( static_cast<float>(spPet->GetIntimacy()) / static_cast<float>(spPet->GetMaxIntimacy()) * 100.f ) < 100.f  )
	{
		return NetError::ERR_HW_PET_01;
	}
	
	return NetError::NET_OK;
}

void KGSUser::UseTransformItem( IN int iUsedItemID, IN const UidType iPetUID, OUT int& iBeforePetID, OUT int& iAfterPetID )
{
	KUserPetPtr spPet = m_kUserPetManager.GetPet( iPetUID );
	if( spPet == NULL )
	{
		START_LOG( cerr, L"���� �ϰ� ���� �ʴ� PetUID�Դϴ�." )
			<< BUILD_LOG( iPetUID )
			<< END_LOG;
		return;
	}

	iBeforePetID = spPet->GetPetID();

	int iTempBeforePetID = iBeforePetID;

	if( iBeforePetID == CXSLPetManager::PUI_HALLOWEEN_PUMPKIN_TANYA_F || iBeforePetID == CXSLPetManager::PUI_HALLOWEEN_PUMPKIN_LAEL_F ||
		iBeforePetID == CXSLPetManager::PUI_HALLOWEEN_PUMPKIN_LOJETA_F || iBeforePetID == CXSLPetManager::PUI_HALLOWEEN_PUMPKIN_NARENEA_F )
	{
		iTempBeforePetID -= 5;
	}

	iAfterPetID = SiCXSLPetManager()->GetTransformPetID( iUsedItemID, iTempBeforePetID );

	if( iBeforePetID == CXSLPetManager::PUI_HALLOWEEN_PUMPKIN_TANYA_F || iBeforePetID == CXSLPetManager::PUI_HALLOWEEN_PUMPKIN_LAEL_F ||
		iBeforePetID == CXSLPetManager::PUI_HALLOWEEN_PUMPKIN_LOJETA_F || iBeforePetID == CXSLPetManager::PUI_HALLOWEEN_PUMPKIN_NARENEA_F )
	{
		iAfterPetID += 5;
	}

	spPet->ChangePetID( iAfterPetID );

	// ģ�е��� 0���� �ʱ�ȭ ����!
	if( spPet->GetIntimacy() > 0 )
	{
		const int iCurIntimacy = spPet->DecreaseIntimacy( spPet->GetIntimacy() );
		LIF( iCurIntimacy == 0 );
	}
}
#endif //SERV_HALLOWEEN_PUMPKIN_FAIRY_PET

#endif SERV_PET_SYSTEM
//}}

//{{ 2010. 07. 04  ������	��ų ����Ʈ ���� Ȯ��
#ifdef SERV_CHECK_SKILL_POINT

bool KGSUser::CheckSkillPointError()
{
	if( GetCharUID() == 0 )
		return false;

	// fix!! �ӽ÷� �� SP ��ġ�� ������ ���� ������ �α׸� �����
	int iUsedSPoint = 0;
	int iUsedCSPoint = 0;
	m_kSkillTree.CalcUsedSPointAndCSPoint( iUsedSPoint, iUsedCSPoint );

	if( m_kSkillTree.IsCashSkillPointExpired() == false )
	{
#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013 // ���볯¥: 2013-06-27
		int iRetrievedSPoint = 0;
		SiCXSLSkillTree()->GetCalcInitSkillPoint( GetLevel(), iRetrievedSPoint );
		const int iCheckSP = iRetrievedSPoint + m_kSkillTree.GetMaxCSPoint();
#else	// SERV_UPGRADE_SKILL_SYSTEM_2013
/*
		const int iCheckSP = GetLevel() * 2 + m_kSkillTree.GetMaxCSPoint();
*/
#endif	// SERV_UPGRADE_SKILL_SYSTEM_2013
		const int iCurSP = m_iSPoint + iUsedSPoint + iUsedCSPoint + m_kSkillTree.GetCSPoint();

		if( iCheckSP != iCurSP )
		{
			//{{ 2009. 6. 16  ������	�Ϲ������� ��츸 ��������
			if( GetAuthLevel() < SEnum::UAL_GM )
			{
				START_LOG( cerr, L"���� ������ �ִ� SP �������� ������ ���� �� �ֽ��ϴ�.!" )
					<< BUILD_LOG( GetCharUID() )
#ifndef SERV_PRIVACY_AGREEMENT
					<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
					<< BUILD_LOGc( GetLevel() )
					<< BUILD_LOG( iCheckSP )
					<< BUILD_LOG( iCurSP )
					<< BUILD_LOG( m_iSPoint )
					<< BUILD_LOG( iUsedSPoint )
					<< BUILD_LOG( iUsedCSPoint )
					<< BUILD_LOG( m_kSkillTree.GetCSPoint() )
					<< BUILD_LOG( m_kSkillTree.GetMaxCSPoint() )
					<< END_LOG;
			}
			else
			{
				// ��ڱ� �̻��� �׽�Ʈ �������� SP�� ������ �� �ֱ� ������ ������ ���� �ʽ��ϴ�.
				START_LOG( cwarn, L"���� ������ �ִ� SP �������� ������ ���� �� �ֽ��ϴ�.!" )
					<< BUILD_LOG( GetCharUID() )
#ifndef SERV_PRIVACY_AGREEMENT
					<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
					<< BUILD_LOGc( GetLevel() )
					<< BUILD_LOG( iCheckSP )
					<< BUILD_LOG( iCurSP )
					<< BUILD_LOG( m_iSPoint )
					<< BUILD_LOG( iUsedSPoint )
					<< BUILD_LOG( iUsedCSPoint )
					<< BUILD_LOG( m_kSkillTree.GetCSPoint() )
					<< BUILD_LOG( m_kSkillTree.GetMaxCSPoint() )
					<< END_LOG;
			}
			//}}
			return false;
		}
	}
	else
	{
		if( iUsedCSPoint != 0  ||  m_kSkillTree.GetCSPoint() != 0 )
		{
			START_LOG( cerr, L"CSP ���Ⱓ�� ����Ǿ��µ� CSP�� �����ִ�" )
				<< BUILD_LOG( GetCharUID() )
#ifndef SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
				<< BUILD_LOGc( GetLevel() )
				<< BUILD_LOG( m_iSPoint )
				<< BUILD_LOG( iUsedSPoint )
				<< BUILD_LOG( iUsedCSPoint )
				<< BUILD_LOG( m_kSkillTree.GetCSPoint() )
				<< BUILD_LOG( m_kSkillTree.GetMaxCSPoint() )
				<< END_LOG;
			return false;
		}
		else
		{
#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013 // ���볯¥: 2013-06-27
			int iRetrievedSPoint = 0;
			SiCXSLSkillTree()->GetCalcInitSkillPoint( GetLevel(), iRetrievedSPoint );
			const int iCheckSP = iRetrievedSPoint;
#else	// SERV_UPGRADE_SKILL_SYSTEM_2013
			const int iCheckSP = GetLevel() * 2;
#endif	// SERV_UPGRADE_SKILL_SYSTEM_2013

			const int iCurSP = m_iSPoint + iUsedSPoint;

			if( iCheckSP != iCurSP )
			{
				//{{ 2009. 6. 16  ������	�Ϲ������� ��츸 ��������
				if( GetAuthLevel() < SEnum::UAL_GM )
				{
					START_LOG( cerr, L"���� ������ �ִ� SP �������� ������ ���� �� �ֽ��ϴ�.!!" )
						<< BUILD_LOG( GetCharUID() )
#ifndef SERV_PRIVACY_AGREEMENT
						<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
						<< BUILD_LOGc( GetLevel() )
						<< BUILD_LOG( iCheckSP )
						<< BUILD_LOG( iCurSP )
						<< BUILD_LOG( m_iSPoint )
						<< BUILD_LOG( iUsedSPoint )
						<< BUILD_LOG( iUsedCSPoint )
						<< BUILD_LOG( m_kSkillTree.GetCSPoint() )
						<< BUILD_LOG( m_kSkillTree.GetMaxCSPoint() )
						<< END_LOG;
				}
				else
				{
					// ��ڱ� �̻��� �׽�Ʈ �������� SP�� ������ �� �ֱ� ������ ������ ���� �ʽ��ϴ�.
					START_LOG( cwarn, L"���� ������ �ִ� SP �������� ������ ���� �� �ֽ��ϴ�.!!" )
						<< BUILD_LOG( GetCharUID() )
#ifndef SERV_PRIVACY_AGREEMENT
						<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
						<< BUILD_LOGc( GetLevel() )
						<< BUILD_LOG( iCheckSP )
						<< BUILD_LOG( iCurSP )
						<< BUILD_LOG( m_iSPoint )
						<< BUILD_LOG( iUsedSPoint )
						<< BUILD_LOG( iUsedCSPoint )
						<< BUILD_LOG( m_kSkillTree.GetCSPoint() )
						<< BUILD_LOG( m_kSkillTree.GetMaxCSPoint() )
						<< END_LOG;
				}
				//}}
				return false;
			}
		}
	}

	return true;
}

#endif SERV_CHECK_SKILL_POINT
//}}

//{{ 2011. 12. 22	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
void KGSUser::CheckWaitPartyUser()
{
	if( GetStateID() > KGSFSM::S_SERVER_SELECT  &&  GetStateID() < KGSFSM::STATE_SENTINEL )
	{
		// ��Ƽ �ʴ� �ð� üũ
		m_kUserPartyManager.CheckWaitPartyUser();
	}
}
#endif SERV_BATTLE_FIELD_SYSTEM
//}}

//{{ 2011. 06. 17	������	��Ŷ �̺�Ʈ ����
#ifdef SERV_KEVENT_FROM
bool KGSUser::CheckIsValidPacketEvent( IN const KEventPtr& spEvent_, IN const wchar_t* pszPacketStructName, IN const bool bIsDeserializeSuccess, IN const bool bIsValidPacketID )
{
	if( bIsDeserializeSuccess == false )
	{
		START_LOG( cerr, L"deserialze failed." << spEvent_->GetIDStr() << " - " << pszPacketStructName )
#ifdef SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetUID() )
			<< BUILD_LOG( GetCharUID() )
#else
			<< BUILD_LOG( GetName() )
			<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetSessionType() )
			<< BUILD_LOG( spEvent_->GetFromType() )
			<< END_LOG;

		m_kUserHackingManager.IncreaseDeserializeFailCount( spEvent_->m_usEventID );
		return false;
	}

	if( bIsValidPacketID == false )
	{
		START_LOG( cerr, L"invalid packet id." << spEvent_->GetIDStr() )
#ifdef SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetUID() )
			<< BUILD_LOG( GetCharUID() )
#else
			<< BUILD_LOG( GetName() )
			<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetSessionType() )
			<< BUILD_LOG( spEvent_->GetFromType() )
			<< END_LOG;
		return false;
	}

	return true;
}
#endif SERV_KEVENT_FROM
//}}

#ifdef SERV_FIX_KGSUSER_KICK// �۾���¥: 2013-08-22	// �ڼ���
void KGSUser::Kick( IN const int iNetError )
{
	KStatistics::ESIColumn_Disconnect_Reason eDisconnectReason = KStatistics::eSIColDR_Unknown;

	switch( iNetError ) 
	{
	case NetError::ERR_VERIFY_01:
		eDisconnectReason = KStatistics::eSIColDR_Diff_Server_Connect;
		break;

	case NetError::ERR_USER_REGISTRATION_00:
		eDisconnectReason = KStatistics::eSIColDR_Diff_Server_Connect;
		m_bReportDisconnectToLogin = false;
		break;

	case NetError::ERR_ADMIN_COMMAND_07:
		eDisconnectReason = KStatistics::eSIColDR_GM_Kick_User;
		//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
		//{{ 2012. 03. 20	��μ�		Game Server ���� ���� ������ ��ŷ ���� üũ
#ifdef SERV_CHECK_KICK_USER_IN_GAMESERVER
		{
			KEGS_SERVER_CHECK_HACK_USER_REQ kPacket;
			kPacket.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_KICK_BY_GM;
			SendPacket( EGS_SERVER_CHECK_HACK_USER_REQ, kPacket );
		}
#endif SERV_CHECK_KICK_USER_IN_GAMESERVER
		//}}
#else
		//{{ 2012. 03. 20	��μ�		Game Server ���� ���� ������ ��ŷ ���� üũ
#ifdef SERV_CHECK_KICK_USER_IN_GAMESERVER
		{
			KEGS_SERVER_CHECK_HACK_USER_NOT kPacket;
			kPacket.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_KICK_BY_GM;
			SendPacket( EGS_SERVER_CHECK_HACK_USER_NOT, kPacket );
		}
#endif SERV_CHECK_KICK_USER_IN_GAMESERVER
		//}}
#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
		//}}
		break;

	case NetError::ERR_USER_REGISTRATION_01:
	case NetError::ERR_USER_REGISTRATION_02:
	case NetError::ERR_USER_REGISTRATION_03:
	case NetError::ERR_USER_REGISTRATION_04:
	case NetError::ERR_USER_REGISTRATION_05:
	case NetError::ERR_USER_REGISTRATION_06:
	case NetError::ERR_USER_REGISTRATION_07:
	case NetError::ERR_USER_REGISTRATION_08:
	case NetError::ERR_USER_REGISTRATION_09:		
		eDisconnectReason = KStatistics::eSIColDR_User_Reg_Error;
		break;

		//{{ 2009. 3. 5  ������		ä���̵�
	case NetError::ERR_CHANNEL_CHANGE_03:
		// ä���̵� �������� ���� ��������
		eDisconnectReason = KStatistics::eSIColDR_Channel_Change;
		break;
		//}}
		//{{ 2010. 06. 22  ������	�����������
#ifdef SERV_ACCOUNT_BLOCK
	case NetError::ERR_ACCOUNT_BLOCK_00:
		eDisconnectReason = KStatistics::eSIColDR_GameServer_Account_Block;

#ifdef SERV_ACCOUNT_BLOCK_USER_POPUP_TEMP// �۾���¥: 2013-06-18	// �ڼ���
		SendID( EGS_ACCOUNT_BLOCK_USER_POPUP_NOT );
#endif // SERV_ACCOUNT_BLOCK_USER_POPUP_TEMP

		break;
#endif SERV_ACCOUNT_BLOCK
		//}}
		//{{ 2011. 08. 09	������	�ӽ�ID �������
#ifdef SERV_MACHINE_ID_BLOCK
	case NetError::ERR_CHECK_MACHINE_ID_00:
		eDisconnectReason = KStatistics::eSIColDR_MachineID_Block;
		break;
#endif SERV_MACHINE_ID_BLOCK
		//}}

		//{{ 2011. 08. 17	������	�ӽ�ID �ߺ� ���� ����
#ifdef SERV_MACHINE_ID_DUPLICATE_CHECK
	case NetError::ERR_USER_REGISTRATION_10:
		eDisconnectReason = KStatistics::eSIColDR_Same_MachineID_Connect;
		break;
#endif SERV_MACHINE_ID_DUPLICATE_CHECK
		//}}

	default:
		START_LOG( cerr, L"Kick Reason���� �̻��մϴ�." )
			<< BUILD_LOG( iNetError )			
			<< END_LOG;		
		break;
	}

	Kick( eDisconnectReason );
}

void KGSUser::Kick( IN const KStatistics::ESIColumn_Disconnect_Reason eDisconnectReason )
{
	SetDisconnectReason( eDisconnectReason );
	ReserveDestroy();
}
#else // SERV_FIX_KGSUSER_KICK
void KGSUser::Kick( int iReason )
{
    if( iReason == NetError::ERR_USER_REGISTRATION_00 )
    {
        m_bReportDisconnectToLogin = false;
    }

#ifdef SERV_FIX_KGSUSER_KICK// �۾���¥: 2013-08-22	// �ڼ���
#else // SERV_FIX_KGSUSER_KICK
    SendPacket( E_KICK_USER_NOT, iReason );
#endif // SERV_FIX_KGSUSER_KICK

	int iIndex = KStatistics::eSIColDR_Unknown;

	switch( iReason ) 
	{
	case NetError::ERR_VERIFY_01:
	case NetError::ERR_USER_REGISTRATION_00:
		iIndex = KStatistics::eSIColDR_Diff_Server_Connect;
		break;

	case NetError::ERR_ADMIN_COMMAND_07:
		iIndex = KStatistics::eSIColDR_GM_Kick_User;
		//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
		//{{ 2012. 03. 20	��μ�		Game Server ���� ���� ������ ��ŷ ���� üũ
#ifdef SERV_CHECK_KICK_USER_IN_GAMESERVER
		{
			KEGS_SERVER_CHECK_HACK_USER_REQ kPacket;
			kPacket.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_KICK_BY_GM;
			SendPacket( EGS_SERVER_CHECK_HACK_USER_REQ, kPacket );
		}
#endif SERV_CHECK_KICK_USER_IN_GAMESERVER
		//}}
#else
		//{{ 2012. 03. 20	��μ�		Game Server ���� ���� ������ ��ŷ ���� üũ
#ifdef SERV_CHECK_KICK_USER_IN_GAMESERVER
		{
			KEGS_SERVER_CHECK_HACK_USER_NOT kPacket;
			kPacket.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_KICK_BY_GM;
			SendPacket( EGS_SERVER_CHECK_HACK_USER_NOT, kPacket );
		}
#endif SERV_CHECK_KICK_USER_IN_GAMESERVER
		//}}
#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
		//}}
		break;

	case NetError::ERR_USER_REGISTRATION_01:
	case NetError::ERR_USER_REGISTRATION_02:
	case NetError::ERR_USER_REGISTRATION_03:
	case NetError::ERR_USER_REGISTRATION_04:
	case NetError::ERR_USER_REGISTRATION_05:
	case NetError::ERR_USER_REGISTRATION_06:
	case NetError::ERR_USER_REGISTRATION_07:
	case NetError::ERR_USER_REGISTRATION_08:
	case NetError::ERR_USER_REGISTRATION_09:		
		iIndex = KStatistics::eSIColDR_User_Reg_Error;
		break;

		//{{ 2009. 3. 5  ������		ä���̵�
	case NetError::ERR_CHANNEL_CHANGE_03:
		// ä���̵� �������� ���� ��������
		iIndex = KStatistics::eSIColDR_Channel_Change;
		break;
		//}}
		//{{ 2010. 06. 22  ������	�����������
#ifdef SERV_ACCOUNT_BLOCK
	case NetError::ERR_ACCOUNT_BLOCK_00:
		iIndex = KStatistics::eSIColDR_GameServer_Account_Block;

#ifdef SERV_ACCOUNT_BLOCK_USER_POPUP_TEMP// �۾���¥: 2013-06-18	// �ڼ���
		SendID( EGS_ACCOUNT_BLOCK_USER_POPUP_NOT );
#endif // SERV_ACCOUNT_BLOCK_USER_POPUP_TEMP

		break;
#endif SERV_ACCOUNT_BLOCK
		//}}
		//{{ 2011. 08. 09	������	�ӽ�ID �������
#ifdef SERV_MACHINE_ID_BLOCK
	case NetError::ERR_CHECK_MACHINE_ID_00:
		iIndex = KStatistics::eSIColDR_MachineID_Block;
		break;
#endif SERV_MACHINE_ID_BLOCK
		//}}

		//{{ 2011. 08. 17	������	�ӽ�ID �ߺ� ���� ����
#ifdef SERV_MACHINE_ID_DUPLICATE_CHECK
	case NetError::ERR_USER_REGISTRATION_10:
		iIndex = KStatistics::eSIColDR_Same_MachineID_Connect;
		break;
#endif SERV_MACHINE_ID_DUPLICATE_CHECK
		//}}

	default:
		START_LOG( cerr, L"Kick Reason���� �̻��մϴ�." )
			<< BUILD_LOG( iReason )			
			<< END_LOG;		
		break;
	}

	SetDisconnectReason( iIndex );
    ReserveDestroy();
}
#endif // SERV_FIX_KGSUSER_KICK

bool KGSUser::SendID( unsigned short usEventID )
{
	return KSession::SendID( PI_GS_PROXY, GetUID(), NULL, usEventID );
}

void KGSUser::SendToAccountDB( unsigned short usEventID )
{
    SendToAccountDB( usEventID, char() );
}

void KGSUser::SendToGameDB( unsigned short usEventID )
{
    SendToGameDB( usEventID, char() );
}

void KGSUser::SendToLogDB( unsigned short usEventID )
{
    SendToLogDB( usEventID, char() );
}

//{{ 2010. 02. 23  ������	�� ����Ʈ �̺�Ʈ
#ifdef SERV_WEB_POINT_EVENT

void KGSUser::SendToNXWebDB( unsigned short usEventID )
{
	SendToNXWebDB( usEventID, char() );
}

#endif SERV_WEB_POINT_EVENT
//}}

//{{ 2013. 09. 23	������	�Ϻ� �̺�Ʈ �߰�DB�۾�
#ifdef SERV_RELAY_DB_CONNECTION
void KGSUser::SendToRelayDB( unsigned short usEventID )
{
	SendToRelayDB( usEventID );
}
#endif SERV_RELAY_DB_CONNECTION
//}}

#ifdef SERV_GLOBAL_BILLING
void KGSUser::SendToKOGBillingDB( unsigned short usEventID )
{
	SendToKOGBillingDB( usEventID, char() );
}
#endif // SERV_GLOBAL_BILLING


void KGSUser::SendToCnUser( unsigned short usEventID )
{
    SendToCnUser( usEventID, char() );
}

//{{ 2009. 4. 24  ������	������������Ʈ
void KGSUser::SendToCnServer( unsigned short usEventID )
{
	SendToCnServer( usEventID, char() );
}
//}}

void KGSUser::SendToLoginAccountDB( unsigned short usEventID )
{
    SendToLoginAccountDB( usEventID, char() );
}

void KGSUser::SendToCnRoom( unsigned short usEventID )
{
    SendToCnRoom( usEventID, char() );
}

//{{ 2011. 04. 29	������	�븮����
#ifdef SERV_PSHOP_AGENCY

void KGSUser::SendToLoginRoom( unsigned short usEventID )
{
	SendToLoginRoom( usEventID, char() );
}

void KGSUser::SendToPShopRoom( unsigned short usEventID )
{
	SendToPShopRoom( usEventID, char() );
}

#endif SERV_PSHOP_AGENCY
//}}

//{{ 2010. 02. 20  ������	ä�� ��Ƽ ����
#ifdef SERV_CHANNEL_PARTY	
void KGSUser::SendToCnParty( unsigned short usEventID )
{
	SendToCnParty( usEventID, char() );
}
#endif SERV_CHANNEL_PARTY
//}}

void KGSUser::SendToGSField( unsigned short usEventID )
{
	SendToGSField( usEventID, char() );
}

void KGSUser::SendToLoginServer( unsigned short usEventID )
{
	SendToLoginServer( usEventID, char() );
}

//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER
void KGSUser::SendToGlobalServer( unsigned short usEventID )
{
	SendToGlobalServer( usEventID, char() );
}
#endif SERV_INSERT_GLOBAL_SERVER
//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�

void KGSUser::SetUnitInfo( const KUnitInfo& kInfo )
{
	//{{ 2009. 4. 2  ������		ĳ�����ʱ�ȭ �˻�
	if( GetCharUID() > 0 )
	{
		GetKUserManager()->UnregCharInfo( *this );

		START_LOG( cerr, L"ĳ���� �����Ϸ��µ� ���� ĳ���� ������ �ʱ�ȭ �Ǿ����� �ʴ�?" )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOG( kInfo.m_nUnitUID )
			<< END_LOG;
	}
	//}}

    // User UID �� ������ ���� �Ǿ���.
    m_nUnitUID              = kInfo.m_nUnitUID;
	m_uiKNMSerialNum		= kInfo.m_uiKNMSerialNum;
    m_cUnitClass            = kInfo.m_cUnitClass;
    m_wstrNickName          = kInfo.m_wstrNickName;
	m_wstrIP			    = kInfo.m_wstrIP;
	//m_nPort				= kInfo.m_nPort;
	m_ucLevel				= kInfo.m_ucLevel;

	//{{ 2011. 09. 27	��μ�	PC �� ������ ����
#ifdef SERV_PCBANG_INCORRECT_BILLING
	m_kUserStatistics.SetCharName( m_wstrNickName );
#endif SERV_PCBANG_INCORRECT_BILLING
	//}

	//{{ 2012. 10. 02	������		ED&������ ����͸� �ý���
#ifdef SERV_ED_AND_ITEM_MORNITORING_SYSTEM
	SetED( kInfo.m_iED, KUserEDManager::ER_SET_ED_SELECT_UNIT );
#else
	m_iED.SetValue( kInfo.m_iED );
#endif SERV_ED_AND_ITEM_MORNITORING_SYSTEM
	//}}	
	m_kEXP.SetExp( kInfo.m_iEXP );
	//{{ 2011. 06. 20	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM

	m_kUserPvpManager.Init( kInfo.m_iOfficialMatchCnt, 
							kInfo.m_iRating, 
							kInfo.m_iMaxRating, 
							kInfo.m_iRPoint, 
							kInfo.m_iAPoint,
							kInfo.m_bIsWinBeforeMatch, 
							kInfo.m_iWin, 
							kInfo.m_iLose
							//{{ 2012. 06. 20	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2
							, kInfo.m_cRank
							//{{ 2012. 06. 25	�ڼ���	2012 ���� ����2
							, kInfo.m_fKFactor
							, kInfo.m_bIsRedistributionUser
							, kInfo.m_iPastSeasonWin
							, GetCharUID()
#endif SERV_2012_PVP_SEASON2
							//}}
							);
#else
	m_iVSPoint.SetValue( kInfo.m_iVSPoint );
	m_iVSPointMax.SetValue( kInfo.m_iVSPointMax );
#endif SERV_PVP_NEW_SYSTEM
	//}}	
	m_iSPoint.SetValue(  kInfo.m_iSPoint );	
	m_kSkillTree.SetCSPoint( kInfo.m_iCSPoint );
	m_kSkillTree.SetMaxCSPoint( kInfo.m_iMaxCSPoint );
	if( true == kInfo.m_wstrCSPointEndDate.empty() )
	{
		// ���� �ð����� ����
		m_kSkillTree.SetCSPointEndDate( L"2000-01-01 00:00:00" );
	}
	else if( false == m_kSkillTree.SetCSPointEndDate( kInfo.m_wstrCSPointEndDate ) )
	{
		START_LOG( cerr, L"���ڿ� �ð� ��ȯ ����." )
			<< BUILD_LOG( kInfo.m_wstrCSPointEndDate )
			<< END_LOG;

		// ���� �ð����� ����
		m_kSkillTree.SetCSPointEndDate( L"2000-01-01 00:00:00" );
	}

	//{{ 2012. 03. 20	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	m_kUserSpiritManager.Init( kInfo.m_iSpirit, kInfo.m_iSpiritMax );
#else
	m_iSpiritMax			= kInfo.m_iSpiritMax;
	m_iSpirit				= kInfo.m_iSpirit;
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}	

	//{{ 2011. 06. 20	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
#else
	m_ePVPEmblem			= CXSLUnit::GetPVPEmblem( kInfo.m_iVSPointMax );
#endif SERV_PVP_NEW_SYSTEM
	//}}	

    m_nStraightVictories    = kInfo.m_nStraightVictories;
    m_usInventorySize       = 0;

	m_kStat					= kInfo.m_kStat;

	//{{ 2012. 02. 02	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	SetLastPosInfo( kInfo.m_kLastPos );
#else
	m_nMapID			    = kInfo.m_nMapID;
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}    
	//{{ 2011. 06. 20	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
#else
	m_iWin				    = kInfo.m_iWin;
	m_iLose				    = kInfo.m_iLose;
#endif SERV_PVP_NEW_SYSTEM
	//}}	
    m_mapDungeonClear       = kInfo.m_mapDungeonClear;
	m_mapTCClear			= kInfo.m_mapTCClear;
#ifdef SERV_LIMITED_DUNGEON_PLAY_TIMES
	m_mapDungeonPlay       = kInfo.m_mapDungeonPlay;
#endif SERV_LIMITED_DUNGEON_PLAY_TIMES

	//{{ 2008. 10. 7  ������	Ÿ��Ʋ
	//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
#ifdef SERV_TITLE_DATA_SIZE
	m_kUserTitleManager.SetEquippedTitleID( kInfo.m_iTitleID );
#else
	m_kUserTitleManager.SetEquippedTitleID( kInfo.m_sTitleID );
#endif SERV_TITLE_DATA_SIZE
	//}}	
	//}}

	//{{ 2009. 9. 25  ������	���
#ifdef GUILD_TEST
	m_kUserGuildManager.SetGuildInfo( kInfo.m_kUserGuildInfo );
#endif GUILD_TEST
	//}}
	
#ifdef SERV_CHINA_SPIRIT_EVENT
	m_arrChinaSpirit[0] = kInfo.m_arrChinaSpirit[0];
	m_arrChinaSpirit[1] = kInfo.m_arrChinaSpirit[1];
	m_arrChinaSpirit[2] = kInfo.m_arrChinaSpirit[2];
	m_arrChinaSpirit[3] = kInfo.m_arrChinaSpirit[3];
	m_arrChinaSpirit[4] = kInfo.m_arrChinaSpirit[4];
	m_arrChinaSpirit[5] = kInfo.m_arrChinaSpirit[5];
#endif SERV_CHINA_SPIRIT_EVENT

    GetKUserManager()->RegCharInfo( *this );
    m_kTimer[TM_DB_UPDATE].restart();

#ifdef SERV_ADD_WARP_BUTTON
	m_trWarpVipEndDate = kInfo.m_trWarpVipEndData;
#endif SERV_ADD_WARP_BUTTON

#ifdef SERV_GROW_UP_SOCKET
	m_iExchangeCount = kInfo.m_iExchangeCount;
#endif SERV_GROW_UP_SOCKET

#ifdef SERV_NEW_YEAR_EVENT_2014
	m_ucOldYearMissionRewardedLevel = kInfo.m_ucOldYearMissionRewardedLevel;
	m_iNewYearMissionStepID = kInfo.m_iNewYearMissionStepID;
#endif SERV_NEW_YEAR_EVENT_2014
}

void KGSUser::GetUnitInfo( KUnitInfo& kInfo )
{
    kInfo.m_iOwnerUserUID       = GetUID();
    kInfo.m_cAuthLevel          = GetAuthLevel();
    kInfo.m_nUnitUID            = m_nUnitUID;
	kInfo.m_uiKNMSerialNum		= m_uiKNMSerialNum;
    kInfo.m_cUnitClass          = GetUnitClass();
	//{{ 2011. 06. 20	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
#else
	kInfo.m_iPVPEmblem			= (int)m_ePVPEmblem;//GetPVPEmblem();
#endif SERV_PVP_NEW_SYSTEM
	//}}
    kInfo.m_wstrNickName        = m_wstrNickName;
    kInfo.m_wstrIP              = m_wstrIP;
    kInfo.m_usPort				= m_nPort;
    kInfo.m_iED                 = GetED();
    kInfo.m_ucLevel             = GetLevel();
    kInfo.m_nNowBaseLevelEXP    = SiKExpTable()->GetRequireTotalExpbyLevel( static_cast<int>(GetLevel()) );
    kInfo.m_nNextBaseLevelEXP   = SiKExpTable()->GetRequireTotalExpbyLevel( static_cast<int>(GetLevel() + 1) );
	kInfo.m_iEXP                = m_kEXP.GetExp();
	//{{ 2011. 06. 20	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
	kInfo.m_iOfficialMatchCnt	= m_kUserPvpManager.GetOfficialMatchCount();
	kInfo.m_iRating				= GetRating();
	kInfo.m_iMaxRating			= m_kUserPvpManager.GetMaxRating();
	kInfo.m_iRPoint				= GetRPoint();
	kInfo.m_iAPoint				= GetAPoint();
	kInfo.m_bIsWinBeforeMatch	= m_kUserPvpManager.IsWinBeforeMatch();

	//{{ 2012. 06. 25	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2
	kInfo.m_cRank				= GetPvpRankForClient();
	kInfo.m_fKFactor			= m_kUserPvpManager.GetKFactor();
	kInfo.m_iPastSeasonWin		= m_kUserPvpManager.GetPastSeasonWin();
#endif SERV_2012_PVP_SEASON2
	//}}

#else
	kInfo.m_iVSPoint			= m_iVSPoint;
	kInfo.m_iVSPointMax			= m_iVSPointMax;
#endif SERV_PVP_NEW_SYSTEM
	//}}	
    kInfo.m_nStraightVictories  = m_nStraightVictories;
	kInfo.m_kStat				= m_kStat;
	kInfo.m_iSPoint				= m_iSPoint;
	//{{ 2012. 02. 02	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	kInfo.m_kLastPos			= GetLastPosInfo();
#else
	kInfo.m_nMapID              = GetMapID();
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}    
	//{{ 2011. 06. 20	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
	kInfo.m_iWin				= GetWin();
	kInfo.m_iLose				= GetLose();
#else
	kInfo.m_iWin				= m_iWin;
	kInfo.m_iLose				= m_iLose;
#endif SERV_PVP_NEW_SYSTEM
	//}}	
    kInfo.m_mapDungeonClear     = m_mapDungeonClear;
	kInfo.m_mapTCClear			= m_mapTCClear;
#ifdef SERV_LIMITED_DUNGEON_PLAY_TIMES
	kInfo.m_mapDungeonPlay     = m_mapDungeonPlay;
#endif SERV_LIMITED_DUNGEON_PLAY_TIMES
	//m_kUserQuestManager.GetQuestClearInfo( kInfo.m_mapQuestClear );
	m_kInventory.GetEquippedItem( kInfo.m_mapEquippedItem );

	//////////////////////////////////////////////////////////////////////////
	// ��ų����
	kInfo.m_iCSPoint			= m_kSkillTree.GetCSPoint();
	kInfo.m_iMaxCSPoint			= m_kSkillTree.GetMaxCSPoint();
	kInfo.m_wstrCSPointEndDate	= m_kSkillTree.GetCSPointEndDateString();


	LIF( KUserSkillTree::MAX_SKILL_SLOT == 8 );     
	KSkillData aSkillData[ KUserSkillTree::MAX_SKILL_SLOT];
	m_kSkillTree.GetSkillSlot( aSkillData );

	kInfo.m_UnitSkillData.m_aEquippedSkill[0] = aSkillData[KUserSkillTree::SKILL_SLOT_A1];
	kInfo.m_UnitSkillData.m_aEquippedSkill[1] = aSkillData[KUserSkillTree::SKILL_SLOT_A2];
	kInfo.m_UnitSkillData.m_aEquippedSkill[2] = aSkillData[KUserSkillTree::SKILL_SLOT_A3];
	kInfo.m_UnitSkillData.m_aEquippedSkill[3] = aSkillData[KUserSkillTree::SKILL_SLOT_A4];

	kInfo.m_UnitSkillData.m_aEquippedSkillSlotB[0] = aSkillData[KUserSkillTree::SKILL_SLOT_B1];
	kInfo.m_UnitSkillData.m_aEquippedSkillSlotB[1] = aSkillData[KUserSkillTree::SKILL_SLOT_B2];
	kInfo.m_UnitSkillData.m_aEquippedSkillSlotB[2] = aSkillData[KUserSkillTree::SKILL_SLOT_B3];
	kInfo.m_UnitSkillData.m_aEquippedSkillSlotB[3] = aSkillData[KUserSkillTree::SKILL_SLOT_B4];

	kInfo.m_UnitSkillData.m_cSkillSlotBExpirationState = (char) m_kSkillTree.GetSkillSlotBExpirationState();
	kInfo.m_UnitSkillData.m_wstrSkillSlotBEndDate = m_kSkillTree.GetSkillSlotBEndDateString();

	m_kSkillTree.GetPassiveSkillData( kInfo.m_UnitSkillData.m_vecPassiveSkill );

	//{{ 2010. 04. 08  ������	����� ��Ʈ
#ifdef SERV_SKILL_NOTE
	m_kSkillTree.GetSkillNote( kInfo.m_UnitSkillData.m_vecSkillNote );
#endif SERV_SKILL_NOTE
	//}}

	//////////////////////////////////////////////////////////////////////////

	kInfo.m_bIsParty = ( GetPartyUID() > 0 );
	
	//{{ 2012. 03. 20	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	kInfo.m_iSpiritMax			= m_kUserSpiritManager.GetSpiritMax();
	kInfo.m_iSpirit				= m_kUserSpiritManager.GetSpirit();
#else
	kInfo.m_iSpiritMax			= m_iSpiritMax;
	kInfo.m_iSpirit				= m_iSpirit;
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}	
	//{{ 2010. 01. 29  ������	PC�� ���ȭ
	kInfo.m_bIsGameBang         = IsPcBang();
	//}}
#ifdef SERV_PC_BANG_TYPE
	kInfo.m_iPcBangType			= GetPcBangType();
#endif SERV_PC_BANG_TYPE

	//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
#ifdef SERV_TITLE_DATA_SIZE
	kInfo.m_iTitleID			= m_kUserTitleManager.GetEquippedTitleID();
#else
	kInfo.m_sTitleID			= m_kUserTitleManager.GetEquippedTitleID();
#endif SERV_TITLE_DATA_SIZE
	//}}	

	//{{ 2009. 9. 25  ������	���
#ifdef GUILD_TEST
	kInfo.m_kUserGuildInfo.m_iGuildUID			= GetGuildUID();
	kInfo.m_kUserGuildInfo.m_wstrGuildName		= m_kUserGuildManager.GetGuildName();
	kInfo.m_kUserGuildInfo.m_ucMemberShipGrade	= m_kUserGuildManager.GetMemberShipGrade();
	kInfo.m_kUserGuildInfo.m_iHonorPoint		= m_kUserGuildManager.GetHonorPoint();
#endif GUILD_TEST
	//}}

#ifdef SERV_ADD_WARP_BUTTON
	kInfo.m_trWarpVipEndData = m_trWarpVipEndDate;
#endif SERV_ADD_WARP_BUTTON

#ifdef SERV_GROW_UP_SOCKET
	kInfo.m_iExchangeCount = m_iExchangeCount;
#endif SERV_GROW_UP_SOCKET

#ifdef SERV_CHINA_SPIRIT_EVENT
	 kInfo.m_arrChinaSpirit[0] = m_arrChinaSpirit[0];
	 kInfo.m_arrChinaSpirit[1] = m_arrChinaSpirit[1];
	 kInfo.m_arrChinaSpirit[2] = m_arrChinaSpirit[2];
	 kInfo.m_arrChinaSpirit[3] = m_arrChinaSpirit[3];
	 kInfo.m_arrChinaSpirit[4] = m_arrChinaSpirit[4];
	 kInfo.m_arrChinaSpirit[5] = m_arrChinaSpirit[5];
#endif SERV_CHINA_SPIRIT_EVENT

	kInfo.m_kGameStat.Init();

#ifdef SERV_NEW_YEAR_EVENT_2014
	kInfo.m_ucOldYearMissionRewardedLevel = m_ucOldYearMissionRewardedLevel;
	kInfo.m_iNewYearMissionStepID = m_iNewYearMissionStepID;
#endif SERV_NEW_YEAR_EVENT_2014
}

void KGSUser::ResetUnitInfo()
{	
    m_nUnitUID          = 0;
    m_cUnitClass        = 0;
    m_wstrNickName      = std::wstring();    
	//{{ 2012. 10. 02	������		ED&������ ����͸� �ý���
#ifdef SERV_ED_AND_ITEM_MORNITORING_SYSTEM
	m_kEDManager.Clear();
#else
	m_iED.SetValue( 0 );
#endif SERV_ED_AND_ITEM_MORNITORING_SYSTEM
	//}}    
    m_ucLevel           = 0;
	m_kEXP.SetExp( 0 );
	//{{ 2011. 06. 20	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
	m_kUserPvpManager.Clear();
#else
	m_iVSPoint.SetValue( 0 );
	m_iVSPointMax.SetValue( 0 );
#endif SERV_PVP_NEW_SYSTEM
	//}}	
	m_iSPoint.SetValue( 0 );
	m_kSkillTree.SetCSPoint( 0 );
	m_kSkillTree.SetMaxCSPoint( 0 );

    m_nStraightVictories = 0;
    m_usInventorySize   = 0;
    m_ucQuickSlotSize   = 0;

	//{{ 2011. 06. 20	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
#else
	m_ePVPEmblem		= CXSLUnit::PE_NONE;
#endif SERV_PVP_NEW_SYSTEM
	//}}	
	
	//{{ 2012. 03. 20	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	m_kUserSpiritManager.Clear();
#else
	m_iSpiritMax		= 0;
	m_iSpirit			= 0;
	m_bIsSpiritUpdated	= false;	// �ټ��� �Ϸ�ġ ��� �Ҹ� �÷���
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}	

	//{{ 2011. 06. 20	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
#else
	m_iWin				= 0;
	m_iLose				= 0;
#endif SERV_PVP_NEW_SYSTEM
	//}}	

	m_iNumResurrectionStone.SetValue( 0 );
	//{{ 2009. 10. 14  ������	�ڵ����� ��Ȱ��
#ifdef AP_RESTONE
	m_iNumAutoPaymentResStone.SetValue( 0 );
	m_tAutoPaymentCheckTime = CTime();
#endif AP_RESTONE
	//}}

	m_dResolveJackpotTime = 30000.f; // �ӽ� �ִ밪

	m_kStat.Init();
    
	//{{ 2012. 02. 02	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	m_kLastPos.Clear();
	SetMapID( SEnum::VMI_RUBEN );
#else
	m_nMapID            = SEnum::VMI_RUBEN;
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}	
    m_mapDungeonClear.clear();
	m_mapTCClear.clear();
#ifdef SERV_LIMITED_DUNGEON_PLAY_TIMES
	m_mapDungeonPlay.clear();
#endif SERV_LIMITED_DUNGEON_PLAY_TIMES

	//{{ 2010. 01. 08  ������	��õ�θ���Ʈ
	m_kUserRecommendManager.Clear();
	//}}
	//{{ 2012. 03. 21	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	m_iBeforeRoomUID = 0;
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}

	//{{ 2011. 12. 20	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	m_kUserPartyManager.Clear();
#else
	//{{ 2010. 03. 10  ������	ä�� ���� ��Ƽ
	m_iPartyUID = 0;
	m_iCandidatePartyUID = 0;
	//}}
	//{{ 2009. 5. 4  ������		����Ƽ
	m_iQuickJoinPartyUID = 0;
	//}}
	//{{ 2010. 02. 09  ������	������Ƽ ��ɰ���
#ifdef SERV_QUICK_PARTY_UPGRADE
	m_mapKickedPartyUIDList.clear();
#endif SERV_QUICK_PARTY_UPGRADE
	//}}
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}	

	//{{ 2010. 07. 05  ������	���� ����� �Ŵ���
#ifdef SERV_USER_ABUSER_MANAGER
	m_kUserAbuserManager.Clear( true );
#endif SERV_USER_ABUSER_MANAGER
	//}}
	
	//{{ 2009. 10. 26  ������	PC���̺�Ʈ
	m_kUserEventManager.Clear();
	//}}
	m_kUserCashInventory.Clear();	// ���� ĳ�� ����
	m_kUserLetterBox.Clear();		// ���� ������
	m_kUserTitleManager.Clear();	// ���� Ÿ��Ʋ �Ŵ���
	m_mapFriendList.clear();		// ģ������
	m_kUserRankingManager.clear();	// ��ŷ ����
    m_kMessengerInfo.Clear();		// �޽���
	m_kUserStatistics.Clear();		// ĳ���� ���

	//{{ 2009. 9. 22  ������	���
#ifdef GUILD_TEST
	m_kUserGuildManager.Clear();
#endif GUILD_TEST
	//}}
	//{{ 2010. 07. 21  ������	�� �ý���
#ifdef SERV_PET_SYSTEM
	m_kUserPetManager.Clear();
#endif SERV_PET_SYSTEM
	//}}
	//{{ 2011. 02. 21  ��μ�	Ư�� ���� ���� ������ ���� ����
#ifdef SERV_DUNGEON_REQUIRED_ITEM_BUG
	m_kDungeonRequired.Clear();
#endif SERV_DUNGEON_REQUIRED_ITEM_BUG
	//}}
	//{{ 2011. 04. 11	������	�븮 ����
#ifdef SERV_PSHOP_AGENCY
	m_kUserPersonalShopManager.Clear();
#endif SERV_PSHOP_AGENCY
	//}}
	//{{ 2012. 02. 21	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	m_kUserUnitManager.Clear();
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}
	
#ifdef SERV_SHARING_BANK_TEST
	m_kInventory.ClearShareBank();
#endif SERV_SHARING_BANK_TEST

	//{{ 2012. 04. 21	�ڼ���	����� �� ����
#ifdef SERV_REFORM_THE_GATE_OF_DARKNESS
	m_kUserBuffManager.Clear();
#endif SERV_REFORM_THE_GATE_OF_DARKNESS
	//}}

#ifdef SERV_INVISIBLE_GM
	m_bInvisible = false;
#endif SERV_INVISIBLE_GM

#ifdef	SERV_RIDING_PET_SYSTM// ���볯¥: 2013-04-21
	m_kUserRidingPetManager.Clear();
#endif	// SERV_RIDING_PET_SYSTM

#ifdef SERV_GATE_OF_DARKNESS_SUPPORT_EVENT
	m_iGateOfDarknessSupportEventTime = 0;
#endif SERV_GATE_OF_DARKNESS_SUPPORT_EVENT

#ifdef SERV_EVENT_RIDING_WITH_SUB_QUEST
	m_iRidingPetSummon = 0;
	m_bIsEventRidingPetQuest = false;
#endif //SERV_EVENT_RIDING_WITH_SUB_QUEST

#ifdef SERV_CHINA_SPIRIT_EVENT
	m_arrChinaSpirit[0] = 0;
	m_arrChinaSpirit[1] = 0;
	m_arrChinaSpirit[2] = 0;
	m_arrChinaSpirit[3] = 0;
	m_arrChinaSpirit[4] = 0;
	m_arrChinaSpirit[5] = 0;
#endif SERV_CHINA_SPIRIT_EVENT

#ifdef SERV_NEW_YEAR_EVENT_2014
	m_ucOldYearMissionRewardedLevel = 0;
	m_iNewYearMissionStepID = -1;
#endif SERV_NEW_YEAR_EVENT_2014
}

bool KGSUser::CheckExceedRefCount( int nCount )
{
    if( KActor::CheckExceedRefCount( nCount ) )
        return true;

    START_LOG( cerr, L"Invalid Ref-count." )
        << BUILD_LOG( GetRefCount() )
#ifndef SERV_PRIVACY_AGREEMENT
        << BUILD_LOG( GetName() )
#endif SERV_PRIVACY_AGREEMENT
        << BUILD_LOG( GetUID() )
		<< BUILD_LOG( GetRoomListID() )
		<< BUILD_LOG( GetPartyUID() )
        << BUILD_LOG( GetStateID() )
        << BUILD_LOG( GetStateIDString() )
		<< END_LOG;

    return false;
}

//{{ 2009. 5. 29  ������	�Լ� ����
bool KGSUser::OnConnectReq( IN const KEGS_CONNECT_REQ& kReq, OUT KEGS_CONNECT_ACK& kAck )
{
	if( kReq.m_wstrVersion.compare( KBaseServer::GetKObj()->GetMainVersion() ) != 0 )
	{
		START_LOG_WITH_NAME( cwarn )
#ifndef SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetIPStr() )
#endif SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( kReq.m_wstrVersion )
			<< BUILD_LOG( KBaseServer::GetKObj()->GetMainVersion() );

		kAck.m_iOK	= NetError::ERR_CONNECT_00;
		//SendPacket( usAckEventID, kAck );

		//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
		//{{ 2012. 03. 20	��μ�		Game Server ���� ���� ������ ��ŷ ���� üũ
#ifdef SERV_CHECK_KICK_USER_IN_GAMESERVER
		KEGS_SERVER_CHECK_HACK_USER_REQ kPacket;
		kPacket.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_CHECK_GAME_VERSION;
		SendPacket( EGS_SERVER_CHECK_HACK_USER_REQ, kPacket );
#endif SERV_CHECK_KICK_USER_IN_GAMESERVER
		//}}
#else
		//{{ 2012. 03. 20	��μ�		Game Server ���� ���� ������ ��ŷ ���� üũ
#ifdef SERV_CHECK_KICK_USER_IN_GAMESERVER
		KEGS_SERVER_CHECK_HACK_USER_NOT kPacket;
		kPacket.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_CHECK_GAME_VERSION;
		SendPacket( EGS_SERVER_CHECK_HACK_USER_NOT, kPacket );
#endif SERV_CHECK_KICK_USER_IN_GAMESERVER
		//}}
#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
		//}}

		SetDisconnectReason( KStatistics::eSIColDR_Diff_Main_Ver );
		ReserveDestroy();
		return false;
	}

	// ���� ���� �˻�
	//{{ 2011. 12. 16	������	���� �ִ� �ο� �ǽð� ����
#ifdef SERV_REALTIME_MAX_USER
	int iMaxUser = GetKGameServer()->GetMaxUser();
#else
	int iMaxUser = KBaseServer::GetKObj()->GetMaxUser();
#endif SERV_REALTIME_MAX_USER
	//}}	
	if( iMaxUser <= 0 )
	{
		START_LOG( cerr, L"�ִ� ���� �̻�." )
			<< BUILD_LOG( iMaxUser )
			<< END_LOG;

		iMaxUser = 1;
	}

	// ���� �ο� ���ѿ� �ɸ��� ä�ο� �������� ���ϵ��� ó��
	int iNowUser = KActorManager::GetKObj()->GetCountNoLock() + SiKChannelManager()->GetChannelChangeWaitingUserCount();

	// �ڱ� �ڽ��� ���� ����ؾ���.
	if( ( iNowUser - 1 ) >= iMaxUser )
	{
		kAck.m_iOK   = NetError::ERR_CONNECT_04;
		//SendPacket( usAckEventID, kAck );

		SetDisconnectReason( KStatistics::eSIColDR_Limit_User_Num );
		ReserveDestroy();
		return false;
	}

	kAck.m_iOK		 = NetError::NET_OK;
	kAck.m_UDPPort	 = static_cast< int >( KNetLayer::GetKObj()->GetNCUDPPort() );
	//{{ 2009. 4. 17  ������	ä������
	kAck.m_iChannelID = SiKChannelManager()->GetChannelID();
	//}}
	//SendPacket( usAckEventID, kAck );
	return true;
}

bool KGSUser::OnNexonUserAuthenticateAck( IN OUT KELG_NEXON_USER_AUTHENTICATE_ACK& kAck_, OUT KEGS_VERIFY_ACCOUNT_ACK& kAck )
{
	//�������� �޾ƿ��� ���и� ����..
	if( kAck_.m_iOK != NetError::NET_OK )
	{
		START_LOG_WITH_NAME( cwarn )
			<< BUILD_LOG( kAck_.m_iOK )
			<< BUILD_LOG( NetError::GetErrStr( kAck_.m_iOK ) )
			<< BUILD_LOG( kAck_.m_kAccountInfo.m_wstrID )
			<< END_LOG;

		kAck.m_iOK = kAck_.m_iOK;
		kAck.m_kAccountInfo = kAck_.m_kAccountInfo;

		//SendPacket( EGS_VERIFY_ACCOUNT_ACK, kPacket );

		//{{ 2010. 06. 22  ������	�����������
#ifdef SERV_ACCOUNT_BLOCK
		if( kAck_.m_iOK == NetError::ERR_ACCOUNT_BLOCK_00 )
		{
			// Ŭ���̾�Ʈ���Դ� ����ó�� �ϱ� ���� OK�� ����
			kAck.m_iOK = NetError::NET_OK;

			// ���� �������� ���� ���� ����
			SetDisconnectReason( KStatistics::eSIColDR_GameServer_Account_Block );
		}
		else
#endif SERV_ACCOUNT_BLOCK
		//}}
		{
			SetDisconnectReason( KStatistics::eSIColDR_Nexon_Auth_Fail );
		}
		ReserveDestroy();
		return false;
	}

#ifdef SERV_NPROTECT_CS_AUTH_30
	if( KSimLayer::GetKObj()->CheckCommonFlag( KSimLayer::CommonFlag::CF_NPGG ) == true )
	{
		m_CSAuth3.Init( true );
	}
#endif SERV_NPROTECT_CS_AUTH_30

	// ���ϼ��� �� �������� ���� Ȯ��
	KActorPtr spActor( KActorManager::GetKObj()->Get( kAck_.m_kAccountInfo.m_nUserUID ) );
	if( spActor )
	{
		START_LOG( cerr, L"���ϼ����� �������� ����" )
			<< BUILD_LOG( m_strName )
			<< BUILD_LOG( m_nUID )
#ifdef SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( kAck_.m_kAccountInfo.m_nUserUID );
#else
			<< BUILD_LOG( kAck_.m_kAccountInfo.m_nUserUID )
			<< BUILD_LOG( kAck_.m_kAccountInfo.m_wstrID )
			<< BUILD_LOG( kAck_.m_kAccountInfo.m_wstrName );
#endif SERV_PRIVACY_AGREEMENT

		//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
		//{{ 2012. 03. 20	��μ�		Game Server ���� ���� ������ ��ŷ ���� üũ
#ifdef SERV_CHECK_KICK_USER_IN_GAMESERVER
		KEGS_SERVER_CHECK_HACK_USER_REQ kPacket;
		kPacket.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_CHECK_OVERLAP_ACCOUNT;
		SendPacket( EGS_SERVER_CHECK_HACK_USER_REQ, kPacket );
#endif SERV_CHECK_KICK_USER_IN_GAMESERVER
		//}}
#else
		//{{ 2012. 03. 20	��μ�		Game Server ���� ���� ������ ��ŷ ���� üũ
#ifdef SERV_CHECK_KICK_USER_IN_GAMESERVER
		KEGS_SERVER_CHECK_HACK_USER_NOT kPacket;
		kPacket.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_CHECK_OVERLAP_ACCOUNT;
		SendPacket( EGS_SERVER_CHECK_HACK_USER_NOT, kPacket );
#endif SERV_CHECK_KICK_USER_IN_GAMESERVER
		//}}
#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
		//}}

		kAck.m_iOK = NetError::ERR_VERIFY_00;    // ���ϼ����� �������� �õ�
		spActor->SetDisconnectReason( KStatistics::eSIColDR_Same_Server_Connect );
		spActor->ReserveDestroy();                  //���� ����ڵ� ������ ������.		

		//SendPacket( EGS_VERIFY_ACCOUNT_ACK, kPacket );

		SetDisconnectReason( KStatistics::eSIColDR_Same_Server_Connect );
		ReserveDestroy();
		return false;
	}

	// ���� ������ �߰ߵ��� �ʾҴٸ� UID ����. 
	KActorManager::GetKObj()->UpdateUID( kAck_.m_kAccountInfo.m_nUserUID, *this );

	// ���̵�� �̸� ����
	SetName( kAck_.m_kAccountInfo.m_wstrID );

	// ������ user name�� Ȯ���ϱ� ���� �Ǹ� ����.
	//{{ 2011. 08. 09  ��μ� KAccontInfo �߰��� ������

	// ���� ����

	// �ؽ� �Ǵ� �系 �����ΰ�?
#ifdef SERV_NEW_CREATE_CHAR_EVENT
	m_kAccountInfo.m_wstrName = kAck_.m_kAccountInfo.m_wstrName;
	m_kAccountInfo.m_iAuthLevel = kAck_.m_kAccountInfo.m_iAuthLevel;
	m_kAccountInfo.m_bInternalUser = kAck_.m_kAccountInfo.m_bInternalUser;
#else
	m_wstrUserName = kAck_.m_kAccountInfo.m_wstrName;
	m_cAuthLevel = kAck_.m_kAccountInfo.m_iAuthLevel;
	m_bInternalUser = kAck_.m_kAccountInfo.m_bInternalUser;
#endif SERV_NEW_CREATE_CHAR_EVENT
	//}}

	//{{ 2011.10.14     ��μ�    ��� ����� Ư�� IP ������ ��� ����
#ifdef SERV_USE_GM_CHEAT_RESTRICTED_IP
	if( kAck_.m_kAccountInfo.m_iAuthLevel == SEnum::UAL_SUPERDEVELOPER )
	{
		START_LOG( cout2, L"���� ������ �����Դϴ�!!!" )
			<< BUILD_LOG( GetName() )
			<< BUILD_LOG( kAck_.m_kAccountInfo.m_iAuthLevel )
			<< END_LOG;

		// Ŭ���̾�Ʈ���� �ν��� �� �ֵ��� ����� ������ ������� �����Ѵ�.
		kAck_.m_kAccountInfo.m_iAuthLevel = SEnum::UAL_DEVELOPER;
		m_kAccountInfo.m_iAuthLevel = SEnum::UAL_DEVELOPER;
	}
#ifndef SERV_CHEAT_IP_CHECK_LIFT
	else if( kAck_.m_kAccountInfo.m_iAuthLevel > SEnum::UAL_NORMAL && kAck_.m_kAccountInfo.m_iAuthLevel < SEnum::UAL_SUPERDEVELOPER )
	{
		// ġƮŰ ��� ������ �ִ� ������ ��� IP �˻縦 �ѵ� ������ �߱��Ѵ�  // UAL_SUPERDEVELOPER�� ����
		if( CheckAuthLevel() == false )
		{
			// ������ ���� IP���� ���� �ϸ� �Ϲ� �������� ����
			START_LOG( cout2, L"ġƮ ������ ���� IP���� �����Ͽ� ���� ������ 0���� ����ϴ�!" )
				<< BUILD_LOG( GetName() )
				<< BUILD_LOG( kAck_.m_kAccountInfo.m_iAuthLevel )
				<< END_LOG;

			// Ŭ���̾�Ʈ�� ���� �ǹǷ� ���� �� �ش�
			kAck_.m_kAccountInfo.m_iAuthLevel = SEnum::UAL_NORMAL;
			// �������� ������ �����Ƿ� ���� �� �ش�.
			m_kAccountInfo.m_iAuthLevel = SEnum::UAL_NORMAL;
		}
	}
#endif // SERV_CHEAT_IP_CHECK_LIFT
#ifdef	SERV_HACKING_USER_CHECK_COUNT// �۾���¥: 2013-06-02	// ��μ�
	else if( kAck_.m_kAccountInfo.m_iAuthLevel < SEnum::UAL_BLOCKED_9 && kAck_.m_kAccountInfo.m_iAuthLevel > SEnum::UAL_SUPERDEVELOPER )
	{
		// ���������� �����̴�?
		START_LOG( cerr, L"���� ������ ������ ���� ���̴�." )
			<< BUILD_LOG( GetName() )
			<< BUILD_LOG( kAck_.m_kAccountInfo.m_iAuthLevel )
			<< END_LOG;

		kAck_.m_kAccountInfo.m_iAuthLevel = SEnum::UAL_NORMAL;
	}
#endif	// SERV_HACKING_USER_CHECK_COUNT

#endif SERV_USE_GM_CHEAT_RESTRICTED_IP
	//}}

	// ��ڱ� �̻��� ������迡 ���� �Ƚ�Ŵ
	m_kUserStatistics.SetIsNormalUser( GetAuthLevel() );

	// �ؽ� ��ī��Ʈ ���� ����
	m_kNexonAccountInfo = kAck_.m_kNexonAccountInfo;
	// �系 ���������� ���� �����̵�� �ؽ� ������ȣ�� �����Ѵ�.
	if( m_kNexonAccountInfo.m_uiNexonSN == 0)
	{
		m_kNexonAccountInfo.m_uiNexonSN = ( unsigned int )kAck_.m_kAccountInfo.m_nUserUID;
	}
	//{{ 2010. 12. 1	������	NexonSN�߰�
	kAck.m_uiNexonSN = kAck_.m_kNexonAccountInfo.m_uiNexonSN;
	//}}

	// ��õ�� ��� ���� ����
	//{{ 2010. 01. 08  ������	��õ�θ���Ʈ
	m_kUserRecommendManager.SetIsRecommend( kAck_.m_kAccountInfo.m_bIsRecommend );
	//}}

	// ���������� ������ �޾Ƴ��´�.
	//{{ ���� ���� �ϼ� ���� �߰� - ��μ�
#ifdef SERV_ADD_BLOCK_DAY
	if( GetAuthLevel() <= SEnum::UAL_BLOCKED_1 && GetAuthLevel() >= SEnum::UAL_BLOCKED_9 )
#else
	if( GetAuthLevel() <= SEnum::UAL_BLOCKED_1 && GetAuthLevel() >= SEnum::UAL_BLOCKED_6 )
#endif SERV_ADD_BLOCK_DAY
	//}}
	{
		m_kAccountBlockInfo.m_wstrEndTime		= kAck_.m_kAccountInfo.m_kAccountBlockInfo.m_wstrEndTime;
		m_kAccountBlockInfo.m_wstrBlockReason	= kAck_.m_kAccountInfo.m_kAccountBlockInfo.m_wstrBlockReason;
	}

	//080602.hoons. ��ŷ���� ����.
	m_cHackingUserType = kAck_.m_cHackingUserType;
	kAck.m_cHackingUserType = kAck_.m_cHackingUserType;

	//{{ 2008. 9. 2  ������		Ŭ���̾�Ʈ ����
	SiKGameSysVal()->GetAdvertisementURL( kAck.m_wstrAdURL );
	//}}

	//{{ 2010. 8. 3	������	���������߰�
	kAck.m_bSex = kAck_.m_kNexonAccountInfo.m_bSex;
	kAck.m_uiAge = kAck_.m_kNexonAccountInfo.m_uiAge;
	//}}

	//{{ 2011. 02. 23	������	���� ���� ī��Ʈ
#ifdef SERV_ACCOUNT_COUNT
	m_kUserStatistics.InitAccountCountInfo( kAck_.m_kAccountInfo.m_wstrRegDate, kAck_.m_mapAccCountInfo );
#endif SERV_ACCOUNT_COUNT
	//}}

	//{{ 2011. 05. 02  ��μ�	2�� ���� �ý���
#ifdef SERV_SECOND_SECURITY
	SetIsUseOTP( kAck_.m_kNexonAccountInfo.m_uSecureCode  );		// OTP �� ���� PC �� �ϳ��� ���� ������ OTP ����

	//{{ 2011. 08. 09  ��μ� KAccontInfo �߰��� ������
#ifdef SERV_NEW_CREATE_CHAR_EVENT
#else
	SetLastLoginDate( kAck_.m_kAccountInfo.m_wstrLastLogin );
#endif SERV_NEW_CREATE_CHAR_EVENT
	//}
#endif SERV_SECOND_SECURITY
	//}}

	// Auth Level�� ���� ���� ����.
	//switch( GetAuthLevel() )
	//{
	//   case SEnum::UAL_BLOCKED:
	//       {
	//           kPacket.m_nOK = NetError::ERR_VERIFY_11;
	//           SendPacket( EGS_VERIFY_ACCOUNT_ACK, kPacket );

	//		SetDisconnectReason( KStatistics::eSIColDR_Account_Block );
	//           ReserveDestroy();
	//           return;
	//       }
	//case SEnum::UAL_NORMAL:
	//case SEnum::UAL_SUPERIOR:
	//case SEnum::UAL_GM:
	//case SEnum::UAL_DEVELOPER:
	//	break;
	//}

	// IP ����.
	if( GetAuthLevel() < SEnum::UAL_DEVELOPER )
	{
		if( GetKGSSimLayer()->CheckIP( GetIPStr() ) == false )
		{
			kAck.m_iOK = NetError::ERR_VERIFY_14;
			//SendPacket( EGS_VERIFY_ACCOUNT_ACK, kPacket );

			SetDisconnectReason( KStatistics::eSIColDR_IP_Block );
			ReserveDestroy();

			START_LOG_WITH_NAME( cerr )
				<< BUILD_LOG( NetError::GetErrStr( NetError::ERR_VERIFY_14 ) )
#ifdef SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetUID() )
#else
				<< BUILD_LOG( GetIPStr() )
#endif SERV_PRIVACY_AGREEMENT
				<< END_LOG;

			return false;
		}
	}

	//{{
#ifdef SERV_KOG_HAVE_PCBANG_IP_LIST
	// �ۺ����ſ��� �ǽù� üũ �޴� ��쿡 ���⼭ PC�� üũ�� �ʿ� ����
	int tempPcBangType = GetKGameServer()->IsPCBangIP( KncUtil::toWideString( KSession::GetIPStr() ) );
	if( tempPcBangType != 0)
	{
		bool bPreStatePCBang = m_kUserPcBangManager.IsPcBang(); // (PC�� ��迡 �ߺ� ��� �Ǵ°� ���� ���ؼ�)

		m_kUserPcBangManager.SetIsPcBang( true );
#ifdef SERV_PC_BANG_TYPE
		m_kUserPcBangManager.SetPcBangType(tempPcBangType);
#endif //SERV_PC_BANG_TYPE
		//{{ 2007. 11. 13  ������  ���� ��� PC�� 
		if (false == bPreStatePCBang)	//(PC�� ��迡 �ߺ� ��� �Ǵ°� ���� ���ؼ�)
			m_kUserStatistics.IncreaseCount( KUserStatistics::USTable_EtcData, 0, KUserStatistics::US_Etc_IsPcBang,	1 );
		//}}

		START_LOG( clog2, L"PC����...KOG IP CHECK" )
			<< BUILD_LOG( GetUID() )
			<< BUILD_LOG( KncUtil::toWideString( KSession::GetIPStr() ) );
	}
	else
	{
		m_kUserPcBangManager.SetIsPcBang( false );
#ifdef SERV_PC_BANG_TYPE
		m_kUserPcBangManager.SetPcBangType(-1); // PC�� �ƴѰ�� -1�� 0�� �Ǿ�����
#endif //SERV_PC_BANG_TYPE
		START_LOG( clog2, L"PC��ƴ�...KOG IP CHECK" )
			<< BUILD_LOG( GetUID() )
			<< BUILD_LOG( KncUtil::toWideString( KSession::GetIPStr() ) );
	}
#endif //SERV_KOG_HAVE_PCBANG_IP_LIST
	//}}

	//{{ 2013. 05. 29	������	���� ����Ʈ ����
#ifdef SERV_BLOCK_LIST
	std::vector< int > vecBlockType;
	vecBlockType.push_back( KBlockInfo::BT_IP_BLOCK );
	vecBlockType.push_back( KBlockInfo::BT_ACCOUNT_BLOCK );
	vecBlockType.push_back( KBlockInfo::BT_TRADE_BLOCK );
	SendCheckBlockUser( vecBlockType );
#endif SERV_BLOCK_LIST
	//}}

	//{{ 2013. 09. 24	������	�Ϻ� �̺�Ʈ �߰�DB�۾�
#ifdef SERV_RELAY_DB_CONNECTION
	{
		KDBE_CHECK_REWARD_FROM_RELAY_DB_JP_EVENT_REQ kPacketToDB;
		kPacketToDB.m_iUserUID = GetUID();
		kPacketToDB.m_iUnitUID = GetCharUID();
        SendToRelayDB( DBE_CHECK_REWARD_FROM_RELAY_DB_JP_EVENT_REQ, kPacketToDB );
	}	
#endif SERV_RELAY_DB_CONNECTION
	//}}
	return true;
}

//{{ 2012. 12. 10  ĳ���� ���� ��Ŷ ���� - ��μ�
#ifdef SERV_SELECT_UNIT_PACKET_DIVISION
bool KGSUser::OnAccountSelectUnitAck( IN OUT KDBE_SELECT_UNIT_ACK& kAck_, IN bool bIsChannelChange )
#else
bool KGSUser::OnAccountSelectUnitAck( IN OUT KEGS_SELECT_UNIT_ACK& kAck_, IN bool bIsChannelChange )
#endif SERV_SELECT_UNIT_PACKET_DIVISION
//}}
{
	kAck_.m_kUnitInfo.m_iOwnerUserUID = GetUID();
	kAck_.m_kUnitInfo.m_nStraightVictories = 0;
	kAck_.m_kUnitInfo.m_wstrIP = GetP2PIP();
	kAck_.m_kUnitInfo.m_usPort = GetP2PPort();
	kAck_.m_kUnitInfo.m_bIsGameBang = IsPcBang();
	//{{ 2011. 10. 13	������	������ ������
#ifdef SERV_CONTENT_MANAGER
	kAck_.m_bCashShopOpen = SiKGSContentManager()->IsEnableCashShop();
#endif SERV_CONTENT_MANAGER
	//}}

	const CXSLUnit::UnitTemplet* pUnitTemplet = SiCXSLUnitManager()->GetUnitTemplet( static_cast<CXSLUnit::UNIT_CLASS>(kAck_.m_kUnitInfo.m_cUnitClass) );
	if( pUnitTemplet == NULL )
	{
		START_LOG_WITH_NAME( cerr )
			<< BUILD_LOGc( kAck_.m_kUnitInfo.m_cUnitClass )
			<< END_LOG;

		kAck_.m_iOK = NetError::ERR_SELECT_UNIT_01;
		return false;
	}

	//Item ���� ���� �׸�
	kAck_.m_kUnitInfo.m_ucLevel				= SiKExpTable()->CheckLevelUp( 1, kAck_.m_kUnitInfo.m_iEXP );
	kAck_.m_kUnitInfo.m_nNowBaseLevelEXP	= SiKExpTable()->GetRequireTotalExpbyLevel( (int)kAck_.m_kUnitInfo.m_ucLevel );
	kAck_.m_kUnitInfo.m_nNextBaseLevelEXP	= SiKExpTable()->GetRequireTotalExpbyLevel( (int)(kAck_.m_kUnitInfo.m_ucLevel + 1) );

	KStatTable::KUnitStatInfo sUnitStat;
	SiKStatTable()->GetUnitStat( kAck_.m_kUnitInfo.m_cUnitClass, kAck_.m_kUnitInfo.m_ucLevel, sUnitStat );
	kAck_.m_kUnitInfo.m_kStat.m_iBaseHP		+= (int)sUnitStat.m_uiHP;
	kAck_.m_kUnitInfo.m_kStat.m_iAtkPhysic	+= (int)sUnitStat.m_usAtkPhysic;
	kAck_.m_kUnitInfo.m_kStat.m_iAtkMagic	+= (int)sUnitStat.m_usAtkMagic;
	kAck_.m_kUnitInfo.m_kStat.m_iDefPhysic	+= (int)sUnitStat.m_usDefPhysic;
	kAck_.m_kUnitInfo.m_kStat.m_iDefMagic	+= (int)sUnitStat.m_usDefMagic;

	// �ټ��� �ִ밪
	kAck_.m_kUnitInfo.m_iSpiritMax = SiKSpiritTable()->GetMaxSpirit();

	SetUnitInfo( kAck_.m_kUnitInfo );
	
	//{{ 2010. 07. 05  ������	���� ����� �Ŵ���
#ifdef SERV_USER_ABUSER_MANAGER
	// SetUnitInfo()�� ȣ��� ���Ŀ� ȣ���ؾ��Ѵ�.
	m_kUserAbuserManager.InitCharAbuser( GetThisPtr<KGSUser>() );
#endif SERV_USER_ABUSER_MANAGER
	//}}

	//{{ 2012. 09. 26	������		���� ���� ���� ����
#ifdef SERV_POST_COPY_BUG_FIX
	if( GetED() < 0 )
	{
		START_LOG( cout, L"ED�� ���̳ʽ��� ������ �߰ߵǾ����ϴ�~!!" )
			<< BUILD_LOG( GetUID() )
			<< BUILD_LOG( GetUserID() )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOG( GetCharName() )
			<< BUILD_LOG( GetED() );

		KDBE_SEND_PHONE_MSG_NOT kPacketNot;
		kPacketNot.m_wstrSMSMessage = boost::str( boost::wformat( L"ED�� ���̳ʽ��� ���� �߰�! : UserUID(%d), NickName(%s)" ) % GetUID() % GetCharName() );
		//{{ 2012. 10. 8	�ڼ���	SMS ��ȭ��ȣ ���� ����
#ifdef SERV_SMS_TOTAL_MANAGER
		SiKSMSPhoneNumberManager()->GetPhoneNumberList( KSMSPhoneNumberManager::FS_MINUS_ED, kPacketNot.m_vecPhoneNum );
#else
		SiKSMSPhoneNumberManager()->GetPhoneNumberList( KSMSPhoneNumberManager::PNLT_ELSWORD_DEV_TEAM, kPacketNot.m_vecPhoneNum );
#endif SERV_SMS_TOTAL_MANAGER
		//}}
		if( kPacketNot.m_vecPhoneNum.empty() == false )
		{
			SendToSMSDB( DBE_SEND_PHONE_MSG_NOT, kPacketNot );
		}

		// �ڵ����� ����� ���
		m_kUserAbuserManager.RegEDAbuser( GetThisPtr<KGSUser>() );
		m_kUserAbuserManager.RegItemAbuser( GetThisPtr<KGSUser>() );
		//HackUserRegPacketMornitor();
	}
#endif SERV_POST_COPY_BUG_FIX
	//}}

	//////////////////////////////////////////////////////////////////////////
	//{{ 2011. 09. 23	������	�α׿��� ���� EDüũ
#ifdef SERV_LOGOUT_ED_CHECK
	if( kAck_.m_iRealDataED != -1 )
	{
		if( GetAuthLevel() < SEnum::UAL_GM )
		{
			if( GetED() != kAck_.m_iRealDataED )
			{
				START_LOG( cout, L"[�˸�] ĳ���� ���� ���� ������ ����� ED���� ������ �����ϰ� �ִ� ED���� �ٸ���! DB��ŷ�ΰ�!?" )
#ifdef SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetCharUID() )
#else
					<< BUILD_LOG( GetName() )
					<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT					
					<< BUILD_LOG( GetED() )
					<< BUILD_LOG( kAck_.m_iRealDataED );

				//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
				KEGS_SERVER_CHECK_HACK_USER_REQ kDummy;
				kDummy.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_LOGOUT_ED_CHECK;
				HackUserRegRejectedUser( kDummy.m_cHackingCheckType );
				HackUserKick( kDummy );
#else
				KEGS_SERVER_CHECK_HACK_USER_NOT kDummy;
				kDummy.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_LOGOUT_ED_CHECK;
				HackUserRegRejectedUser( kDummy.m_cHackingCheckType );
				HackUserKick( kDummy );
#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
				//}}
			}
		}
		else
		{
			if( GetED() != kAck_.m_iRealDataED )
			{
				START_LOG( cout, L"[�˸�] ��ڱ� �̻� ������ ���ؼ� ED�� ������ �־����� üũ �ٶ��ϴ�!" )
#ifdef SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetCharUID() )
#else
					<< BUILD_LOG( GetName() )
					<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT					
					<< BUILD_LOG( GetED() )
					<< BUILD_LOG( kAck_.m_iRealDataED );
			}
		}
	}
#endif SERV_LOGOUT_ED_CHECK
	//}}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	
	//{{ 2012. 02. 02	������	��Ʋ�ʵ� �ý���
//#ifdef SERV_BATTLE_FIELD_SYSTEM
	// ������ ��ġ ����
	SetLastPosInfo( kAck_.m_kUnitInfo.m_kLastPos );

	// MapID ��ȿ�� �˻�
	if( GetMapID() == SEnum::VMI_TUTORIAL )
	{
		// ĳ���͸� ó�� ������ ���, ������ �ƴϴ�. �׳� �̴�� ����!
	}
	else if( SEnum::IsBattleFieldID( GetMapID() ) == true )		// ��Ʋ�ʵ�
	{
		//{{ 2012. 11. 13	������		���� ������ ���� �˻� �� ����
#ifdef SERV_CHECK_POSSIBLE_ENTER_VILLAGE
		int iRequireLevel = 0;
		int iRequireDungeonID = 0;
		
		const SEnum::BATTLE_FIELD_ID eBattleFieldID = static_cast<SEnum::BATTLE_FIELD_ID>( GetMapID() );
		const int iEnterVillageMapID = SiCXSLBattleFieldManager()->GetEnterVillageMapID( eBattleFieldID );

		// ���� ������ ��Ʋ�ʵ����� �˻�!
		const bool bCanEnterBattleField = SiCXSLBattleFieldManager()->IsCanEnterBattleField( eBattleFieldID, GetLevel(), m_mapDungeonClear, iRequireLevel, iRequireDungeonID );

		// ���� ������ ��Ʋ�ʵ� ���� �������� �˻�!
		const bool bCanEnterVillageMap = CheckEnterTheVillage( iEnterVillageMapID );

		// ���� �ϳ��� üũ�� ��� ���ϸ� ���� ������ �ٽ� ������!
		if( bCanEnterBattleField == false  ||  bCanEnterVillageMap == false )
		{
			// �ִ��� ���� ������ ���� mapid�� ��������!
			KLastPositionInfo kLastPos;
			kLastPos.m_iMapID = SiCXSLMapData()->GetPossibleEnterVillageMapID( GetLevel(), m_mapDungeonClear );

			// �ٽ� üũ!
			if( CheckEnterTheVillage( kLastPos.m_iMapID ) == false )
			{
				kLastPos.m_iMapID = SiCXSLMapData()->GetDefaultMapID();
			}

			START_LOG( cerr, L"���� ������ ��Ʋ�ʵ尡 �ƴѵ� DB�� ����Ǿ� �ֽ��ϴ�!" )
				<< BUILD_LOG( GetCharName() )
				<< BUILD_LOG( kAck_.m_kUnitInfo.m_kLastPos.m_iMapID )
				<< BUILD_LOG( GetMapID() )
				<< END_LOG;

			// ���õ� ĳ������ ������ ����.
			SetLastPosInfo( kLastPos );
			kAck_.m_kUnitInfo.m_kLastPos = kLastPos;
		}
#endif SERV_CHECK_POSSIBLE_ENTER_VILLAGE
		//}}
	}
	else if( SEnum::IsVillageMapID( GetMapID() ) == true  ||  SEnum::IsRestFieldID( GetMapID() ) == true )			// ���� & �޽�ó
	{
		//{{ 2012. 11. 13	������		���� ������ ���� �˻� �� ����
#ifdef SERV_CHECK_POSSIBLE_ENTER_VILLAGE
		// ������ ������ ������ �ȵȴٸ�..
 		if( CheckEnterTheVillage( GetMapID() ) == false )
 		{
 			// �ִ��� ���� ������ ���� mapid�� ��������!
 			KLastPositionInfo kLastPos;
 			kLastPos.m_iMapID = SiCXSLMapData()->GetPossibleEnterVillageMapID( GetLevel(), m_mapDungeonClear );
 
 			// �ٽ� üũ!
 			if( CheckEnterTheVillage( kLastPos.m_iMapID ) == false )
 			{
 				kLastPos.m_iMapID = SiCXSLMapData()->GetDefaultMapID();
 			}
 
 			START_LOG( cerr, L"������ �� ���� �����ε� DB�� ����Ǿ� �־���?" )
#ifdef SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetCharUID() )
#else
 				<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
 				<< BUILD_LOG( kAck_.m_kUnitInfo.m_kLastPos.m_iMapID )
 				<< BUILD_LOG( GetMapID() )
 				<< END_LOG;
 
 			// ���õ� ĳ������ ������ ����.
 			SetLastPosInfo( kLastPos );
 			kAck_.m_kUnitInfo.m_kLastPos = kLastPos;
 		}
#endif SERV_CHECK_POSSIBLE_ENTER_VILLAGE
		//}}
	}
	else
	{
		START_LOG( cerr, L"����� ������ ��ġ�� Ʃ�丮�� ������ ��Ʋ�ʵ嵵 �ƴ� ��ġ��! �Ͼ�� �ȵǴ� ����!" )
			<< BUILD_LOG( GetCharName() )
			<< BUILD_LOG( kAck_.m_kUnitInfo.m_kLastPos.m_iMapID )
			<< BUILD_LOG( GetMapID() )
			<< END_LOG;

		// ���õ� ĳ������ ������ ����.
		KLastPositionInfo kLastPos;
		kLastPos.m_iMapID = SiCXSLMapData()->GetDefaultMapID();
		SetLastPosInfo( kLastPos );
		kAck_.m_kUnitInfo.m_kLastPos = kLastPos;
	}
	//////////////////////////////////////////////////////////////////////////
//#else
//	//////////////////////////////////////////////////////////////////////////
//	// last position unpacking
//	const int iLastPosition = kAck_.m_kUnitInfo.m_nMapID;
//
//	CXSLMapData::UnpackingLastPosition( iLastPosition, m_nMapID, m_ucLastTouchLineIndex, m_usLastPosValue );
//
//	kAck_.m_kUnitInfo.m_nMapID = GetMapID();
//	kAck_.m_kUnitInfo.m_ucLastTouchLineIndex = m_ucLastTouchLineIndex;
//	kAck_.m_kUnitInfo.m_usLastPosValue = m_usLastPosValue;
//
//	//{{ 2009. 3. 30  ������	MapID�˻�
//	if( !SiCXSLMapData()->VerifyMapID( GetMapID() ) )
//	{
//		START_LOG( cerr, L"�������� �ʴ� MapID�� DB�� ����Ǿ� �־���?" )
//			<< BUILD_LOG( GetCharName() )
//			<< BUILD_LOG( iLastPosition )
//			<< BUILD_LOG( GetMapID() )
//			<< BUILD_LOG( (int)m_ucLastTouchLineIndex )
//			<< BUILD_LOG( m_usLastPosValue )
//			<< END_LOG;
//
//		// ���õ� ĳ������ ������ ����.
//		kAck_.m_kUnitInfo.m_nMapID = SiCXSLMapData()->GetDefaultMapID();
//		m_nMapID = kAck_.m_kUnitInfo.m_nMapID;
//		
//		kAck_.m_kUnitInfo.m_ucLastTouchLineIndex = 0;
//		kAck_.m_kUnitInfo.m_usLastPosValue = 0;
//	}
//	else
//	{
//		//{{ 2012. 09. 21   ��μ�   ���� ���� ���� �޽��� ����
//#ifdef SERV_ENTER_VILLAGE_ERROR
//		// ������ ������ ������ �ȵȴٸ�..
//		if( !CheckEnterTheVillage( kAck_.m_kUnitInfo.m_nMapID, iTempLevel, iTempDungeonID ) )
//#else
//		// ������ ������ ������ �ȵȴٸ�..
//		if( !CheckEnterTheVillage( kAck_.m_kUnitInfo.m_nMapID ) )
//#endif SERV_ENTER_VILLAGE_ERROR
//		//}}
//		{
//			kAck_.m_kUnitInfo.m_nMapID = SiCXSLMapData()->GetBeforeMapID( kAck_.m_kUnitInfo.m_nMapID );
//			//{{ 2012. 09. 21   ��μ�   ���� ���� ���� �޽��� ����
//#ifdef SERV_ENTER_VILLAGE_ERROR
//			if( !CheckEnterTheVillage( kAck_.m_kUnitInfo.m_nMapID, iTempLevel, iTempDungeonID ) )
//#else
//			if( !CheckEnterTheVillage( kAck_.m_kUnitInfo.m_nMapID ) )
//#endif SERV_ENTER_VILLAGE_ERROR
//			//}}
//			{
//				kAck_.m_kUnitInfo.m_nMapID = SiCXSLMapData()->GetDefaultMapID();
//			}
//
//			START_LOG( cerr, L"������ �� ���� �����ε� DB�� ����Ǿ� �־���?" )
//				<< BUILD_LOG( GetCharName() )
//				<< BUILD_LOG( kAck_.m_kUnitInfo.m_nMapID )
//				<< BUILD_LOG( GetMapID() )
//				<< END_LOG;
//
//			// ���õ� ĳ������ ������ ����.
//			m_nMapID = kAck_.m_kUnitInfo.m_nMapID;
//
//			kAck_.m_kUnitInfo.m_ucLastTouchLineIndex = 0;
//			kAck_.m_kUnitInfo.m_usLastPosValue = 0;
//		}			
//	}
//#endif SERV_BATTLE_FIELD_SYSTEM
	//}}
	//////////////////////////////////////////////////////////////////////////

	// ������ �����͵� ������ �����ش�.
	//{{ 2011. 07. 11	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
#else
	kAck_.m_kUnitInfo.m_iPVPEmblem		= (int)CXSLUnit::GetPVPEmblem( kAck_.m_kUnitInfo.m_iVSPointMax );
#endif SERV_PVP_NEW_SYSTEM
	//}}	

	KDBE_UPDATE_INVENTORY_ITEM_POS_NOT kPacketToDB;
	if( m_kInventory.Init( kAck_.m_mapInventorySlotSize, kAck_.m_mapItem, kPacketToDB.m_kItemPositionUpdate, bIsChannelChange ) == false )
	{
		START_LOG( cerr, L"�κ��丮 �ʱ�ȭ�� ���� �߻�" )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOGc( GetAuthLevel() )
			<< END_LOG;
	}	

	// �߸��� ������ �κ��丮 ���� ����
	if( !kPacketToDB.m_kItemPositionUpdate.empty() )
	{
		kPacketToDB.m_iUnitUID = GetCharUID();
		SendToGameDB( DBE_UPDATE_INVENTORY_ITEM_POS_NOT, kPacketToDB );
	}

	//////////////////////////////////////////////////////////////////////////
	//{{ 2010. 01. 04  ������	�κ��丮 ��ɰ���
#ifdef SERV_INVENTORY_NEW

	// ��Ŷ ����� ���̱� ���� DB�κ��� ���� �κ��丮 ������ �ϴ� ����ϴ�.
	kAck_.m_mapItem.clear();

	// ������ ������ ������ �����ϱ����� ������� ������ select_unit_ack�� ����ϴ�. ( ���ܻ�Ȳ ���� )
	m_kInventory.GetInventoryInfo( CXSLInventory::ST_E_EQUIP, kAck_.m_mapItem );

	// �κ��丮 ���� ������ ���
	m_kInventory.GetInventorySlotSize( kAck_.m_mapInventorySlotSize );

#else
#pragma ELSE_MESSAGE
	//{{ 2009. 8. 7  ������		�������� ����
	//m_kInventory.GetInventoryInfoWithoutBank( kAck_.m_mapInventorySlotSize, kAck_.m_mapItem );
	//}}
#endif SERV_INVENTORY_NEW
	//}}
	//////////////////////////////////////////////////////////////////////////

	m_iNumResurrectionStone.SetValue( kAck_.m_iNumResurrectionStone );
	//{{ 2009. 10. 14  ������	�ڵ����� ��Ȱ��
#ifdef AP_RESTONE
	m_iNumAutoPaymentResStone.SetValue( kAck_.m_iNumAutoPaymentResStone );  
#endif AP_RESTONE
	//}}	

	StateTransition( KGSFSM::I_TO_FIELD_MAP );

	//////////////////////////////////////////////////////////////////////////	
	//ȹ�� ��ų����Ʈ.
	LIF( KUserSkillTree::MAX_SKILL_SLOT == 8 );
	int aSkillSlot[KUserSkillTree::MAX_SKILL_SLOT];
	aSkillSlot[KUserSkillTree::SKILL_SLOT_A1] = (int)  kAck_.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkill[0].m_iSkillID;
	aSkillSlot[KUserSkillTree::SKILL_SLOT_A2] = (int)  kAck_.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkill[1].m_iSkillID;
	aSkillSlot[KUserSkillTree::SKILL_SLOT_A3] = (int)  kAck_.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkill[2].m_iSkillID;
	aSkillSlot[KUserSkillTree::SKILL_SLOT_A4] = (int)  kAck_.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkill[3].m_iSkillID;
	aSkillSlot[KUserSkillTree::SKILL_SLOT_B1] = (int)  kAck_.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkillSlotB[0].m_iSkillID;
	aSkillSlot[KUserSkillTree::SKILL_SLOT_B2] = (int)  kAck_.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkillSlotB[1].m_iSkillID;
	aSkillSlot[KUserSkillTree::SKILL_SLOT_B3] = (int)  kAck_.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkillSlotB[2].m_iSkillID;
	aSkillSlot[KUserSkillTree::SKILL_SLOT_B4] = (int)  kAck_.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkillSlotB[3].m_iSkillID;

	//{{ 2011. 01. 06  ��μ�  ��ų����ü���� üũ(�κ��丮-�Ⱓ��) ��� ����
#ifdef SERV_SKILL_SLOT_CHANGE_INVENTORY
	m_kInventory.GetSlotChangeBEndDate( CXSLItem::EI_EXPAND_SKILL_SLOT_EVENT, kAck_.m_kUnitInfo.m_UnitSkillData.m_wstrSkillSlotBEndDate );
#endif SERV_SKILL_SLOT_CHANGE_INVENTORY
	//}}

	m_kSkillTree.InitSkill( kAck_.m_vecSkillAcquired, aSkillSlot, kAck_.m_kUnitInfo.m_UnitSkillData.m_wstrSkillSlotBEndDate, kAck_.m_vecSkillUnsealed, GetUnitClass() );

	//{{ 2010. 03. 22  ������	����� ��Ʈ
#ifdef SERV_SKILL_NOTE
	m_kSkillTree.InitSkillNote( kAck_.m_cSkillNoteMaxPageNum, kAck_.m_mapSkillNote );
#endif SERV_SKILL_NOTE
	//}}

	kAck_.m_kUnitInfo.m_UnitSkillData.m_cSkillSlotBExpirationState = (char) m_kSkillTree.GetSkillSlotBExpirationState();
	kAck_.m_kUnitInfo.m_UnitSkillData.m_wstrSkillSlotBEndDate = m_kSkillTree.GetSkillSlotBEndDateString();

	//{{ 2010. 07. 04  ������	��ų ����Ʈ ���� Ȯ��
#ifdef SERV_CHECK_SKILL_POINT
	if( CheckSkillPointError() == false )
	{
		if( GetAuthLevel() < SEnum::UAL_GM )
		{
			START_LOG( cerr, L"ĳ���� ���� ������ ��ų ����Ʈ ���� �������Դϴ�." )
#ifdef SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetUID() )
				<< BUILD_LOG( GetCharUID() )
#else
				<< BUILD_LOG( GetName() )
				<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
				<< END_LOG;
		}

#ifdef SERV_SKILL_POINT_CORRECTION
		if( GetAuthLevel() < SEnum::UAL_GM )
		{
			int iCorrectionSP = 0;
			if( CheckSkillPointCorrection( iCorrectionSP ) == true )
			{
				/* �� ������ ������ ���� �ʰ� DB ��ϸ� ��.
				if( iCorrectionSP > 0 )		// ĳ���� ���ýÿ��� �ش� ������ ���� SP���� ������ ��ŭ SP�� ������.
				{
					m_iSPoint += iCorrectionSP;
					kAck_.m_kUnitInfo.m_iSPoint = m_iSPoint;
				}
				*/

				int iUsedSPoint = 0;
				int iUsedCSPoint = 0;
				m_kSkillTree.CalcUsedSPointAndCSPoint( iUsedSPoint, iUsedCSPoint );

				KDBE_SKILL_POINT_CORRECTION_INFO_NOT kPacketToLogDB;
				kPacketToLogDB.m_iServerGroupID = KBaseServer::GetKObj()->GetServerGroupID();
				kPacketToLogDB.m_iUserUID = GetUID();
				kPacketToLogDB.m_iUnitUID = GetCharUID();
				kPacketToLogDB.m_iUnitClass = GetUnitClass();
				kPacketToLogDB.m_sLevel = GetLevel();
				kPacketToLogDB.m_iSPoint = m_iSPoint;
				kPacketToLogDB.m_iUsedSPoint = iUsedSPoint;
				kPacketToLogDB.m_iCSPoint = m_kSkillTree.GetCSPoint();
				kPacketToLogDB.m_iUsedCSPoint = iUsedCSPoint;
				kPacketToLogDB.m_iMaxCSPoint = m_kSkillTree.GetMaxCSPoint();
				kPacketToLogDB.m_iCorrectionSP = iCorrectionSP;
				SendToLogDB( DBE_SKILL_POINT_CORRECTION_INFO_NOT, kPacketToLogDB );
			}
		}
#endif //SERV_SKILL_POINT_CORRECTION

	}
#endif SERV_CHECK_SKILL_POINT
	//}}

	// Ŀ�´�Ƽ �ɼ� ���� ����
	m_kUserGameOption.SetChatBlackList( kAck_.m_vecChatBlackList );
	m_kUserGameOption.SetDenyOptions( kAck_.m_kDenyOptions );

	//ĳ���� �ٲ𶧸��� ����Ʈ �����͸� ���� �ʱ�ȭ �Ѵ�.
	m_kUserQuestManager.SetUnitQuest( kAck_.m_vecQuest, kAck_.m_vecCompletQuest, GetThisPtr<KGSUser>() );
	//{{ 2011. 08. 29	��μ�       ���� ���� ����Ʈ
#ifdef SERV_RANDOM_DAY_QUEST
	SiCXSLQuestManager()->GetRandomQuestGroupList( kAck_.m_mapRandomQuestList );
#endif SERV_RANDOM_DAY_QUEST
	//}}
	//{{ 2012. 12. 14  ���� �̼� �ý��� ( �������� ����Ʈ ) - ��μ�
#ifdef SERV_ACCOUNT_MISSION_SYSTEM
	m_kUserAccountQuestManager.SetAccountQuest( kAck_.m_vecAccountQuesting, kAck_.m_vecAccountCompleteQuest, kAck_.m_vecQuest, kAck_.m_vecCompletQuest, GetThisPtr<KGSUser>() );
#endif SERV_ACCOUNT_MISSION_SYSTEM
	//}}

	//{{ 2008. 10. 6  ������	Ÿ��Ʋ
#ifdef SERV_PC_BANG_TYPE
	bool bUsePcBangTitle = IsPcBang() & SiKGameSysVal()->IsUsingPcBangTitle( GetPcBangType() );
	m_kUserTitleManager.Init( bUsePcBangTitle, kAck_.m_vecMission, kAck_.m_vecTitle );
#else SERV_PC_BANG_TYPE
	m_kUserTitleManager.Init( IsPcBang(), kAck_.m_vecMission, kAck_.m_vecTitle );
#endif SERV_PC_BANG_TYPE
	//}}

    // ģ�� ����Ʈ
    m_kMessengerInfo = kAck_.m_kMessengerInfo;

	//ĳ�����ý� ���� ���ͼ��� ���������� ��Ͻ��� ���´�.
	{
		KELG_REGISTER_UNIT_NOT kCNNot;
		kCNNot.m_nUnitUID			= GetCharUID();
		kCNNot.m_uiKNMSerialNum		= m_uiKNMSerialNum;
		kCNNot.m_wstrUnitNickName	= GetCharName();
		kCNNot.m_bDenyFriendShip	= m_kUserGameOption.IsDenyFriendShip();
		std::map< UidType, KFriendInfo >::iterator mitFriendInfo;
		for( mitFriendInfo = m_kMessengerInfo.m_mapFriendInfo.begin(); mitFriendInfo != m_kMessengerInfo.m_mapFriendInfo.end(); mitFriendInfo++ )
		{
			kCNNot.m_setFriendUID.insert( mitFriendInfo->first );
		}
		SendToLoginServer( ELG_REGISTER_UNIT_NOT, kCNNot );
	}	

	//{{ 2012. 03. 20	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	// ĳ���� ���ý� �۷ι� �������� ĳ���� ������ ��Ͻ�Ű��!
	{
		KEGB_REGISTER_UNIT_NOT kGBNot;
		kGBNot.m_kUnitInfo.m_iUserUID = GetUID();
		kGBNot.m_kUnitInfo.m_iUnitUID = GetCharUID();
		kGBNot.m_kUnitInfo.m_wstrUnitNickName = GetCharName();
		kGBNot.m_kUnitInfo.m_iGSUID = KBaseServer::GetKObj()->GetUID();
		kGBNot.m_kUnitInfo.m_iChannelID = SiKChannelManager()->GetChannelID();
		kGBNot.m_kUnitInfo.m_wstrIP = NetCommon::GetLocalIPW();
		kGBNot.m_kUnitInfo.m_cAuthLevel = GetAuthLevel();
		//{{ 2012. 12. 17	�ڼ���	���� �̺�Ʈ
#ifdef SERV_EVENT_JACKPOT
		kGBNot.m_kUnitInfo.m_iChannelCode = GetChannelCode();
#endif SERV_EVENT_JACKPOT
		//}}
		SendToGlobalServer( EGB_REGISTER_UNIT_NOT, kGBNot );
	}
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}

	// ���� ��� �ش� ������ ���� �ð� üũ
	m_kTimer[TM_UNIT_DURATION].restart();

	// ���� ���
	m_kTimer[TM_RESOLVE_JACKPOT].restart();
	m_dResolveJackpotTime = SiCXSLResolveItemManager()->GetJackpotTime();

	// ��õ�� ����
	//{{ 2010. 01. 08  ������	��õ�θ���Ʈ
	m_kUserRecommendManager.SetRecommendedUnitUID( kAck_.m_iRecommendUnitUID );
	kAck_.m_bIsRecommend = m_kUserRecommendManager.IsRecommend();
	//}}

	//{{ 2012. 03. 20	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
#else
	// �ټ��� ��� �Ҹ� ���
	m_bIsSpiritUpdated = kAck_.m_bIsSpiritUpdated;
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}	

	// ������ ����� ���� Ȯ��
	//{{ 2010. 07. 05  ������	���� ����� �Ŵ���
#ifdef SERV_USER_ABUSER_MANAGER
	m_kUserAbuserManager.InitCharAbuser( GetThisPtr<KGSUser>() );
#endif SERV_USER_ABUSER_MANAGER
	//}}

#ifdef SERV_ITEM_IN_INVENTORY_CONNECT_EVENT
	std::set< int > setItemID;
	m_kInventory.GetItemIDSet( setItemID );
#endif SERV_ITEM_IN_INVENTORY_CONNECT_EVENT

#ifdef SERV_GATE_OF_DARKNESS_SUPPORT_EVENT
	SetGateOfDarknessSupportEventTime( kAck_.m_iGateOfDarknessSupportEventTime );
#endif SERV_GATE_OF_DARKNESS_SUPPORT_EVENT
#ifdef SERV_RECRUIT_EVENT_BASE
	m_kUserRecommendManager.SetRecruitUnitList( kAck_.m_vecRecruitUnitInfo );
	m_kUserRecommendManager.SetRecruiterUnitList( kAck_.m_vecRecruiterUnitInfo );

	START_LOG( clog, L"��õ��/����õ�� ����� m_kUserRecommendManager �� ���� �Ϸ�" )
		<< BUILD_LOG( kAck_.m_vecRecruitUnitInfo.size() )
		<< BUILD_LOG( kAck_.m_vecRecruiterUnitInfo.size() )
		<< END_LOG;
#endif SERV_RECRUIT_EVENT_BASE

	//{{  2011.11.30     ��μ�    ĳ���ͺ� ���� �̺�Ʈ
#ifdef SERV_CHAR_CONNECT_EVENT
	//{{ 2012. 10. 13	�ڼ���	���� �̺�Ʈ ���� ����
#ifdef SERV_CONNECT_EVENT_FACTOR_SET
	KGameEventManager::ConnectEventFactorSet kFactor(
		GetCharUID(),
		GetLevel(),
		IsPcBang(),
		GetChannelCode(),
#ifdef SERV_BUBBLE_FIGHTER_TOGETHER_EVENT	// ���� ������ �ؿ��� �߰�
		GetBF_Team(),
#endif // SERV_BUBBLE_FIGHTER_TOGETHER_EVENT
		GetUnitType(),
		GetUnitClass(),
		true

		//{{ 2012. 04. 13	�ڼ���	( ���� ���� ǥ�� )
#ifdef SERV_EVENT_RETURN_USER_MARK_SCRIPT
#else
		//{{ 2012. 03. 26	�ڼ���	�Ƹ����� ���� ������ ���� ����! ( ���� ���� ǥ�� )
#ifdef SERV_EVENT_RETURN_USER_MARK
		, IsEventReturnUser()
#endif SERV_EVENT_RETURN_USER_MARK
		//}}
#endif SERV_EVENT_RETURN_USER_MARK_SCRIPT
		//}}
		//{{ 2012. 06. 07	�ڼ���	���ϸ��� ���� ����
#ifdef SERV_EVENT_DAILY_GIFT_BOX
		, m_mmapDailyGiftBoxList
#endif SERV_EVENT_DAILY_GIFT_BOX
		//}}
		, m_kComeBackUserInfo.GetIsComeBackUser()
		//{{ 2012. 12. 11	�ڼ���	���� ���� �̺�Ʈ �۾�
#ifdef SERV_FIXED_DATE_EVENT
		, m_kAccountInfo
#endif SERV_FIXED_DATE_EVENT
		//}}
		//{{ 2012. 12. 12	�ڼ���	�ܿ� ���� ���� �̺�Ʈ( �ӽ�, �ϵ� �ڵ� )
#ifdef SERV_2012_WINTER_VACATION_EVENT
		, m_wstrWinterVacationEventRegDate
		, m_iWinterVacationEventCount
#endif SERV_2012_WINTER_VACATION_EVENT
		//}}
		//{{ 2012. 12. 25	�ڼ���	Ư�� ���� ���� ���� �̺�Ʈ
#ifdef SERV_SPECIFIC_USER_CONNECT_EVENT
		, GetUID()
#endif SERV_SPECIFIC_USER_CONNECT_EVENT
		//}}
#ifdef SERV_ITEM_IN_INVENTORY_CONNECT_EVENT
		, setItemID
#endif SERV_ITEM_IN_INVENTORY_CONNECT_EVENT
#ifdef SERV_CUSTOM_CONNECT_EVENT
		, GetCustonEventID()
#endif //SERV_CUSTOM_CONNECT_EVENT
#ifdef	SERV_CRITERION_DATE_EVENT// ���볯¥: 2013-04-11
		, GetCriterionDateEventInfo()
		, GetAccountRegDate()
#endif	// SERV_CRITERION_DATE_EVENT
#ifdef SERV_STEAM_USER_CONNECT_EVENT
		, IsPureSteamUser()
#endif //SERV_STEAM_USER_CONNECT_EVENT
#ifdef SERV_TIME_EVENT_ONLY_CURRENT_USER_CHAR
		,IsNewUnit()	
		,IsNewUnit2()	
		,IsCurrentUnit()
#endif //SERV_TIME_EVENT_ONLY_CURRENT_USER_CHAR
#ifdef SERV_NEW_USER_QUEST_HARD_CODING
		, GetAccountRegDate()
#endif //SERV_NEW_USER_QUEST_HARD_CODING
		);

	kFactor.m_pVecConnectTimeEvent		= &kAck_.m_vecConnectTimeEvent;
	kFactor.m_pVecCumulativeTimeEvent	= &kAck_.m_vecCumulativeTimeEvent;

#ifdef SERV_ADVERTISEMENT_EVENT
	kFactor.m_pVecCompletedAdvertisementEvent	= &kAck_.m_vecAdvertisementEvent;
#endif SERV_ADVERTISEMENT_EVENT

	m_kUserEventManager.InitTimeEvent( kFactor );
#else
	m_kUserEventManager.InitTimeEvent( 
		GetLevel(),
		IsPcBang(),
		GetChannelCode(),
		GetBF_Team(),
		GetUnitType(),
		kAck_.m_vecConnectTimeEvent,
		kAck_.m_vecCumulativeTimeEvent

		//{{ 2012. 04. 13	�ڼ���	( ���� ���� ǥ�� )
#ifdef SERV_EVENT_RETURN_USER_MARK_SCRIPT
#else
		//{{ 2012. 03. 26	�ڼ���	�Ƹ����� ���� ������ ���� ����! ( ���� ���� ǥ�� )
#ifdef SERV_EVENT_RETURN_USER_MARK
		, IsEventReturnUser()
#endif SERV_EVENT_RETURN_USER_MARK
		//}}
#endif SERV_EVENT_RETURN_USER_MARK_SCRIPT
		//}}

		//{{ 2012. 06. 07	�ڼ���	���ϸ��� ���� ����
#ifdef SERV_EVENT_DAILY_GIFT_BOX
		, GetCharUID()
		, m_mmapDailyGiftBoxList
#endif SERV_EVENT_DAILY_GIFT_BOX
		//}}
#ifdef SERV_ITEM_IN_INVENTORY_CONNECT_EVENT
		, setItemID
#endif SERV_ITEM_IN_INVENTORY_CONNECT_EVENT
		);
#endif SERV_CONNECT_EVENT_FACTOR_SET
	//}}
#endif SERV_CHAR_CONNECT_EVENT
	//}}

	//030406.hoons. ��������Ʈ ���� �� ����ó���� �Ѵ�.
	if( m_kUserTutorialInfo.OnSelectUnitAck( GetThisPtr<KGSUser>(), kAck_.m_vecTutorialDBUnitInfo ) == false )
	{
		START_LOG( cerr, L"��������Ʈ ���� ���ý���.!" )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOGc( GetLevel() )
#ifndef SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
			<< END_LOG;
	}
	else
	{
		if( m_kUserTutorialInfo.GetTutorUnitType() == KUserTutorialInfo::TUT_STUDENT &&
			m_kUserTutorialInfo.IsExistbyTutorData() == false )
		{
			KTutorialUnitInfo kTutInfo;
			kTutInfo.m_iUnitUID = GetCharUID();
			kTutInfo.m_ucLevel = GetLevel();
			kTutInfo.m_wstrNickName = GetCharName();
			kTutInfo.m_bIsOnline = true;
			if( SiKTutorialManager()->AddUnit( kTutInfo ) == false )
			{
				START_LOG( cerr, L"�����ý��� ����Ʈ �߰� ����.!" )
					<< BUILD_LOG( GetCharUID() )
					<< BUILD_LOGc( GetLevel() )
#ifndef SERV_PRIVACY_AGREEMENT
					<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
					<< END_LOG;
			}
		}
	}

	//{{ 2009. 7. 7  ������		��ŷ ����
	m_kUserRankingManager.Init( kAck_.m_vecHenirRanking );
	//}}

	//{{ 2010. 07. 21  ������	�� �ý���
#ifdef SERV_PET_SYSTEM

	//////////////////////////////////////////////////////////////////////////
	// �κ��� ��� ������ �� �����ϱ�
#ifdef SERV_ROBOT_TEST
	SiKRobotTestManager()->RobotPet( GetCharName(), kAck_.m_iSummonedPetUID, kAck_.m_vecPetList );
#endif SERV_ROBOT_TEST
	//////////////////////////////////////////////////////////////////////////

	// �� ���� �ʱ�ȭ!
	int iPetCategorySlotSize = 0;
	std::map< UidType, KInventoryItemInfo > mapPetInventory;
	m_kUserPetManager.Init( GetThisPtr<KGSUser>(), kAck_.m_iSummonedPetUID, kAck_.m_mapPetItem, kAck_.m_vecPetList, iPetCategorySlotSize, mapPetInventory, kAck_.m_vecCanEvolutionPetList, kAck_.m_vecNeverSummonPetList );
	kAck_.m_mapPetItem.clear();

	// ��ȯ�Ǿ� �ִ� ���� �ִٸ� �� �κ��丮 ���� �ʱ�ȭ!
	if( kAck_.m_iSummonedPetUID != 0 )
	{
		std::vector< KInventoryItemInfo > vecPetInventorySlotInfo;
		m_kInventory.InitPetInventory( kAck_.m_iSummonedPetUID, iPetCategorySlotSize, mapPetInventory, vecPetInventorySlotInfo );
	}
#endif SERV_PET_SYSTEM
	//}}

	//{{ 2011. 04. 11	������	�븮 ����
#ifdef SERV_PSHOP_AGENCY
	m_kUserPersonalShopManager.Init( kAck_.m_kPShopAgencyInfo );
#endif SERV_PSHOP_AGENCY
	//}}

	//{{ 2011. 01. 17	������	ĳ���� ī��Ʈ ����
#ifdef SERV_CHAR_LOG
	m_kUserStatistics.InitCharacterCountInfo( kAck_.m_mapCharGameCount, kAck_.m_wstrUnitCreateDate );
	kAck_.m_mapCharGameCount.clear();
#endif SERV_CHAR_LOG
	//}}

	//{{ 2011. 03. 22	������	ĳ���� ù ���� �α�
#ifdef SERV_DAILY_CHAR_FIRST_SELECT
	if( m_kUserStatistics.IsFirstCharSelectToday( kAck_.m_wstrUnitLastLoginDate ) == true )
	{
		// �α� ������!
		WriteCharacterLogToDB( KELOG_USER_CHARACTER_LOG_NOT::LT_DAILY_CHAR_FIRST_SELECT );
	}
#endif SERV_DAILY_CHAR_FIRST_SELECT
	//}}

#ifdef SERV_LUNITCONNECTLOG_BUG_FIX
	m_tConnectTime		= CTime::GetCurrentTime();
#endif SERV_LUNITCONNECTLOG_BUG_FIX

	// ü��ID ����
	if( IsGuestUser() )
	{
		if( GetLevel() > SiKGameSysVal()->GetGuestUserLimitLevel() )
		{
			// ����19 ����
			kAck_.m_iOK = NetError::ERR_GUEST_USER_02;
			return false;
		}
	}

	// Ŭ���̾�Ʈ�� ���۵Ǵ� ACK���� ���ʿ��� ������ ����
	//{{ 2009. 7. 7  ������		��ŷ ����
	kAck_.m_vecHenirRanking.clear();
	//}}
	kAck_.m_vecConnectTimeEvent.clear();

	//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER
	KEGS_WORLD_MISSION_UPDATE_NOT kMission;
	SiKGSWorldMissionManager()->GetMissionInfo( kMission );
	SendPacket( EGS_WORLD_MISSION_UPDATE_NOT, kMission );
#endif SERV_INSERT_GLOBAL_SERVER
	//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�

#ifdef SERV_GLOBAL_MISSION_MANAGER
	KEGS_GLOBAL_MISSION_UPDATE_NOT kGlobalMission;
	SiKGSGlobalMissionManager()->GetGlobalMissionInfo( kGlobalMission );
	SendPacket( EGS_GLOBAL_MISSION_UPDATE_NOT, kGlobalMission );
#endif SERV_GLOBAL_MISSION_MANAGER

	CTime tCurr = CTime::GetCurrentTime();

	//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
	//{{ 2013. 04. 15	�ڼ���	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
	KELG_UPDATE_USER_UNIT_BUFF_INFO_REQ kBuffReq;
	kBuffReq.m_iUnitUID = GetCharUID();
	int iWorldBuff = SiKGSWorldMissionManager()->GetWorldBuffState();
	int iWorldBuffPriod = SiKGSWorldMissionManager()->GetWorldBuffEndTime();

	if( iWorldBuff != CXSLBuffManager::BTI_NONE )
	{
		if( m_kUserBuffManager.IsBuffActivated( iWorldBuff ) == false )
		{
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_THANKS_OF_RESIDENTS, kBuffReq.m_vecDeActivateBuff );
			ActivateBuff( CXSLBuffManager::BTI_BUFF_THANKS_OF_RESIDENTS, kBuffReq.m_vecActivateBuff, tCurr, iWorldBuffPriod );
		}
	}
	else
	{
		DeactivateBuff( CXSLBuffManager::BTI_BUFF_THANKS_OF_RESIDENTS, kBuffReq.m_vecDeActivateBuff );
	}

	if( IsPcBang() == true )
	{
		ActivateBuff( CXSLBuffManager::BTI_BUFF_PREMIUM_PC_ROOM, kBuffReq.m_vecActivateBuff );
	}
#else
	KELG_UPDATE_USER_UNIT_BUFF_INFO_REQ kBuffReq;
	kBuffReq.m_iUnitUID = GetCharUID();
	int iWorldBuff = SiKGSWorldMissionManager()->GetWorldBuffState();
	int iWorldBuffPriod = SiKGSWorldMissionManager()->GetWorldBuffEndTime();

	if( iWorldBuff != CXSLBuffManager::BTI_NONE )
	{
		DeactivateBuff( CXSLBuffManager::BTI_BUFF_THANKS_OF_RESIDENTS, kBuffReq.m_vecDeActivateBuff );
		DeactivateBuff( CXSLBuffManager::BTI_BUFF_BREATH_OF_DRAGON, kBuffReq.m_vecDeActivateBuff );
		DeactivateBuff( CXSLBuffManager::BTI_BUFF_BLESSING_OF_EL, kBuffReq.m_vecDeActivateBuff );

		switch( iWorldBuff )
		{
		case CXSLBuffManager::BTI_BUFF_THANKS_OF_RESIDENTS:
			{
				ActivateBuff( CXSLBuffManager::BTI_BUFF_THANKS_OF_RESIDENTS, kBuffReq.m_vecActivateBuff, tCurr );
			}break;
		case CXSLBuffManager::BTI_BUFF_BREATH_OF_DRAGON:
			{
				ActivateBuff( CXSLBuffManager::BTI_BUFF_THANKS_OF_RESIDENTS, kBuffReq.m_vecActivateBuff, tCurr );
				ActivateBuff( CXSLBuffManager::BTI_BUFF_BREATH_OF_DRAGON, kBuffReq.m_vecActivateBuff, tCurr );
			}break;
		case CXSLBuffManager::BTI_BUFF_BLESSING_OF_EL:
			{
				ActivateBuff( CXSLBuffManager::BTI_BUFF_THANKS_OF_RESIDENTS, kBuffReq.m_vecActivateBuff, tCurr );
				ActivateBuff( CXSLBuffManager::BTI_BUFF_BREATH_OF_DRAGON, kBuffReq.m_vecActivateBuff, tCurr );
				ActivateBuff( CXSLBuffManager::BTI_BUFF_BLESSING_OF_EL, kBuffReq.m_vecActivateBuff, tCurr );
			}break;
		}
	}
	else
	{
		DeactivateBuff( CXSLBuffManager::BTI_BUFF_THANKS_OF_RESIDENTS, kBuffReq.m_vecDeActivateBuff );
		DeactivateBuff( CXSLBuffManager::BTI_BUFF_BREATH_OF_DRAGON, kBuffReq.m_vecDeActivateBuff );
		DeactivateBuff( CXSLBuffManager::BTI_BUFF_BLESSING_OF_EL, kBuffReq.m_vecDeActivateBuff );
	}

	if( IsPcBang() == true )
	{
		ActivateBuff( CXSLBuffManager::BTI_BUFF_PREMIUM_PC_ROOM, kBuffReq.m_vecActivateBuff );
	}
#endif SERV_NEW_DEFENCE_DUNGEON
	//}}
#endif SERV_SERVER_BUFF_SYSTEM
	//}

#ifdef SERV_NEW_UNIT_TRADE_LIMIT
	if( m_kUserAbuserManager.IsTradeBlockUnit() == true )
	{
		ActivateBuff( CXSLBuffManager::BTI_BUFF_TRADE_BLOCK, kBuffReq.m_vecActivateBuff );
	}
#endif // SERV_NEW_UNIT_TRADE_LIMIT

	//{{ 2011. 05. 27    ��μ�    �޸� ���� ���� ����
#ifdef SERV_COME_BACK_USER_REWARD
	if( m_kComeBackUserInfo.GetIsComeBackUser() == true )
	{
		CTime tComeBackBuffEnd = m_kComeBackUserInfo.GetComeBackBuffEnd();

		if( m_kComeBackUserInfo.GetIsComeBackFirst() == true )		// ���� ���� ���ӽø� ����
		{
			// �޸� ���� ���� ���� ť�� ����
			int iRewardItem = 0;
			int iRewardPeriod = 0;

			if( m_kComeBackUserInfo.GiveComeBackUserRewardCube( iRewardItem, tComeBackBuffEnd, iRewardPeriod ) == true )
			{
				// �޸� ���� ���� ���� ť�긦 ����!
				m_kUserUnitSelectRewardManager.AddRewardItemInfo( iRewardItem );
			
				// �޸� ���� ���� ���� Īȣ ����
				KDBE_INSERT_TITLE_REQ kPacketToDB;
				kPacketToDB.m_iUnitUID = GetCharUID();
				//{{ 2012. 02. 03	�ڼ���	�̺�Ʈ �������� ó����� ���� ( Script -> DB )
#ifdef SERV_CHANGE_EVENT_INFO_SCRIPT_TO_DB
				kPacketToDB.m_iTitleID = ERI_COME_BACK_USER_TITLE;
#else
				kPacketToDB.m_iTitleID = KRewardTable::ERI_COME_BACK_USER_TITLE;
#endif SERV_CHANGE_EVENT_INFO_SCRIPT_TO_DB
				//}}
				kPacketToDB.m_sPeriod  = (short)iRewardPeriod;
				SendToGameDB( DBE_INSERT_TITLE_REQ, kPacketToDB );

				// ���� ������������ �˷�����
				//{{ 2012. 02. 03	�ڼ���	�̺�Ʈ �������� ó����� ���� ( Script -> DB )
#ifdef SERV_CHANGE_EVENT_INFO_SCRIPT_TO_DB
				kAck_.m_kUnitInfo.m_iTitleID = ERI_COME_BACK_USER_TITLE;
#else
				kAck_.m_kUnitInfo.m_iTitleID = KRewardTable::ERI_COME_BACK_USER_TITLE;
#endif SERV_CHANGE_EVENT_INFO_SCRIPT_TO_DB
				//}}

				// DB�� �������
				KDBE_WRITE_COME_BACK_REWARD_NOT kNot;
				kNot.m_iUserUID					= GetUID();
				kNot.m_iRewardStep				= m_kComeBackUserInfo.GetComeBackRewardLevel();
				kNot.m_wstrComeBackBuffEndDate	= tComeBackBuffEnd.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
				SendToAccountDB( DBE_WRITE_COME_BACK_REWARD_NOT, kNot );

				// �޸� ���� ���� �ý��� ���� �˸�
				AnnunciateComeBackUser();
			}
		}
		
		//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
		int iEndComeBackBuddTime = static_cast<int>(tComeBackBuffEnd.GetTime() - tCurr.GetTime());
		ActivateBuff( CXSLBuffManager::BTI_BUFF_RETURN_OF_HERO, kBuffReq.m_vecActivateBuff, tCurr, iEndComeBackBuddTime );
#else
		// �޸� ���� ���� �ð� �˷��ֱ�
		KEGS_COME_BACK_BUFF_NOT kComeBack;
		kComeBack.m_bIsFinish = false;
		kComeBack.m_iComeBackBuffEnd = tComeBackBuffEnd.GetTime();
		SendPacket( EGS_COME_BACK_BUFF_NOT, kComeBack );
#endif SERV_SERVER_BUFF_SYSTEM
		//}
	
		// ���� ù �����̶��
		if( m_kUserStatistics.IsFirstUserLoginToday( GetUID(), kAck_.m_wstrUnitLastLoginDate ) == true )
		{
			// ���� ��� ���� �α� ������!
			SendFirstLoginComeBackUser( kAck_.m_kUnitInfo, m_kComeBackUserInfo.GetIsComeBackFirst() );
		}

		// ���� ���� �� ����� ������ ���� ���� ���� ����
		m_kComeBackUserInfo.SetIsComeBackFirst( false );
	}

	m_kComeBackUserInfo.SetIsUnitSelect( true );

#endif SERV_COME_BACK_USER_REWARD
	//}}

	//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
	SendToLoginServer( ELG_UPDATE_USER_UNIT_BUFF_INFO_REQ, kBuffReq );
#endif SERV_SERVER_BUFF_SYSTEM
	//}

	//{{ 2011. 08. 12   ��μ�      ��ϸ� ���� 
#ifdef SERV_NEW_HENIR_TEST
	// ���� �̺�Ʈ ���� ��� ����
	kAck_.m_PacketHenirRewardCount.m_bUnLimited = false;
	kAck_.m_PacketHenirRewardCount.m_iEventMAX = 0;

	// ���� �������� �̺�Ʈ�� �ִ°�?
	SiKGameEventManager()->GetHenirRewardCountEvent( kAck_.m_PacketHenirRewardCount.m_bUnLimited, kAck_.m_PacketHenirRewardCount.m_iEventMAX );
	
	m_kUserDungeonManager.Init( kAck_.m_PacketHenirRewardCount.m_bUnLimited,
								kAck_.m_PacketHenirRewardCount.m_iEventMAX,
								kAck_.m_PacketHenirRewardCount.m_iNormal, 
								kAck_.m_PacketHenirRewardCount.m_iPremium, 
								kAck_.m_PacketHenirRewardCount.m_iEvent,
								IsPcBang()
#ifdef SERV_PC_BANG_TYPE
								, GetPcBangType()
#endif SERV_PC_BANG_TYPE
								);

	m_kUserDungeonManager.GetHenirRewardCountInfo( kAck_.m_PacketHenirRewardCount );
#endif SERV_NEW_HENIR_TEST
	//}}
	//{{  2011.11.24     ��μ�    ĳ���� ���� �α�
#ifdef SERV_CHAR_CONNECT_LOG
	SetLoginLevel( GetLevel() );
	SetUnitLoginTime( tCurr );
#endif SERV_CHAR_CONNECT_LOG
	//}}

	//{{ 2012. 04. 15	�ڼ���	( ���� ���� ǥ�� )
#ifdef SERV_EVENT_RETURN_USER_MARK_SCRIPT
	m_kUserEventManager->PayTheCriterionEventReward( GetThisPtr<KGSUser>() );
#endif SERV_EVENT_RETURN_USER_MARK_SCRIPT
	//}}

	//{{ 2012. 02. 21	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	// �ִ� HP������ ���� ���ϱ� ���� KRoomUserInfo�� ����Ѵ�.
	KRoomUserInfo kRoomUserInfo;
	GetRoomUserInfo( kRoomUserInfo, CXSLRoom::RT_BATTLE_FIELD );

	// ���� ��ų ����
	kAck_.m_kUnitInfo.m_UnitSkillData = kRoomUserInfo.m_UnitSkillData;

	// ���� �Ŵ��� �ʱ�ȭ
	m_kUserUnitManager.Init( kAck_.m_kGamePlayStatus, kRoomUserInfo );
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}

//{{ 2012. 10. 10	�ڼ���	���� �̺�Ʈ
#ifdef SERV_EVENT_BINGO
	kAck_.m_iOK = SetBingoEventInfo( kAck_.m_kBingoEvent );
	if( kAck_.m_iOK != NetError::NET_OK )
	{
		// ���� �̺�Ʈ ���� ���� ����
		// Ŭ���̾�Ʈ���� �˷����� �ʱ� ���� UnKnown���� ó���ϰ� �ִ�.
		return false;
	}
	
	// ���� Ŭ�󿡰� ������ �˷��� �ʿ�� ����.
	kAck_.m_kBingoEvent.clear();
#endif SERV_EVENT_BINGO
	//}}

	//{{ 2012. 05. 16	��μ�       �ɸ��� ���ý� ������ ���� ��ID �� ��ȯ
#ifdef SERV_SELECT_UNIT_CHANGE_MAP_ID
	if( GetAuthLevel() < SEnum::UAL_DEVELOPER )
	{
		int iTempMapID = kAck_.m_kUnitInfo.m_kLastPos.m_iMapID;
		SiCXSLMapData()->CheckMapID( kAck_.m_kUnitInfo.m_ucLevel, iTempMapID );
		// MapID �� ���� �Ǿ��ٸ� Line, PosValue �� ������ ����
		if( iTempMapID != kAck_.m_kUnitInfo.m_kLastPos.m_iMapID )
		{
			kAck_.m_kUnitInfo.m_kLastPos.m_iMapID = iTempMapID;
			kAck_.m_kUnitInfo.m_kLastPos.m_ucLastTouchLineIndex = -1;
			kAck_.m_kUnitInfo.m_kLastPos.m_usLastPosValue = 0;
		}
	}
#endif SERV_SELECT_UNIT_CHANGE_MAP_ID
	//}}

	//{{ 2012. 12. 20	������	�ƶ� ù ���� Ʃ�丮��
#ifdef SERV_ARA_FIRST_SELECT_TUTORIAL
	kAck_.m_bFirstSelect = ( m_kUserStatistics.GetCharacterCount( KUserStatistics::CGCT_SELECT_CHAR ) == 0 );
#endif SERV_ARA_FIRST_SELECT_TUTORIAL
	//}}
	
	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
	m_kUserRelationshipManager.Init( kAck_.m_kRelationshipInfo );
#endif SERV_RELATIONSHIP_SYSTEM
	//}

#ifdef SERV_2013_JUNGCHU_TITLE
	IF_EVENT_ENABLED( CEI_2013_JUNGCHU_TITLE )
	{
		bool bHaveEventTitle = false;	// �̺�Ʈ Ÿ��Ʋ�� ������ �ִ���
		BOOST_TEST_FOREACH( KTitleInfo&, kInfo, kAck_.m_vecTitle )
		{
			if( kInfo.m_iTitleID == KRewardTable::ERI_2013_JUNGCHU_TITLE )
				bHaveEventTitle = true;
		}
		if( bHaveEventTitle == false )
		{
			// �Ȱ����� ������ Ÿ��Ʋ ����
			START_LOG( cout, L"[�׽�Ʈ] �̺�Ʈ �Ⱓ�� Ÿ��Ʋ�� �Ȱ����� �ִ�. Ÿ��Ʋ�� ����" )
				<< BUILD_LOG( kAck_.m_wstrUnitLastLoginDate )
				<< END_LOG;

			KDBE_INSERT_TITLE_REQ kPacketToDB;
			kPacketToDB.m_iUnitUID = GetCharUID();
			kPacketToDB.m_iTitleID = KRewardTable::ERI_2013_JUNGCHU_TITLE;
			kPacketToDB.m_sPeriod  = 15;
			SendToGameDB( DBE_INSERT_TITLE_REQ, kPacketToDB );

			// ���� ������������ �˷�����
			kAck_.m_kUnitInfo.m_iTitleID = KRewardTable::ERI_2013_JUNGCHU_TITLE;
		}
	}
#endif SERV_2013_JUNGCHU_TITLE

#ifdef SERV_CHANNELING_AERIA_EVENT
	IF_EVENT_ENABLED( CEI_CHANNELING_AERIA_EVENT )
	{
		if( true == IsAeriaUser())
		{
			bool bHaveEventTitle = false;	// �̺�Ʈ Ÿ��Ʋ�� ������ �ִ���
			BOOST_TEST_FOREACH( KTitleInfo&, kInfo, kAck_.m_vecTitle )
			{
				if( kInfo.m_iTitleID == KRewardTable::ERI_CHANNELING_AERIA_EVENT_TITLE )
					bHaveEventTitle = true;
			}
			if( bHaveEventTitle == false )
			{
				// �Ȱ����� ������ Ÿ��Ʋ ����
				START_LOG( clog, L"[�׽�Ʈ] �̺�Ʈ �Ⱓ�� Ÿ��Ʋ�� �Ȱ����� �ִ�. Ÿ��Ʋ�� ����" )
					<< BUILD_LOG( kAck_.m_wstrUnitLastLoginDate )
					<< END_LOG;

				KDBE_INSERT_TITLE_REQ kPacketToDB;
				kPacketToDB.m_iUnitUID = GetCharUID();
				kPacketToDB.m_iTitleID = KRewardTable::ERI_CHANNELING_AERIA_EVENT_TITLE;
				kPacketToDB.m_sPeriod  = 30;
				SendToGameDB( DBE_INSERT_TITLE_REQ, kPacketToDB );

				// ���� ������������ �˷�����
				kAck_.m_kUnitInfo.m_iTitleID = KRewardTable::ERI_CHANNELING_AERIA_EVENT_TITLE;
			}
		}
	}
#endif SERV_CHANNELING_AERIA_EVENT

	return true;
}
//}}

//{{ 2009. 10. 14  ������	�ڵ�����
//{{ 2012. 12. 10  ĳ���� ���� ��Ŷ ���� - ��μ�
#ifdef SERV_SELECT_UNIT_PACKET_DIVISION
void KGSUser::OnSelectUnitSuccess( IN const KDBE_SELECT_UNIT_ACK& kAck_, IN bool bIsChannelChange )
#else
void KGSUser::OnSelectUnitSuccess( IN const KEGS_SELECT_UNIT_ACK& kAck_, IN bool bIsChannelChange )
#endif SERV_SELECT_UNIT_PACKET_DIVISION
//}}
{
	// �Ⱓ�� �˻�
	ExpireCashSkillPoint( true );
	ExpireSkillSlotB( true );
	ExpireItem( true );
	ExpireTitle( true );
	
	// ������ ĳ�� ������ �ִ��� Ȯ���Ϸ� ����
	KENX_BT_NISMS_INVENTORY_READABLE_REQ kPacketReq;
	kPacketReq.m_ulPacketNo	  	   = SiKNexonBillingTCPManager()->GetNextPacketNo();
	kPacketReq.m_bytePacketType	   = KNexonBillingTCPPacket::NISMS_INVENTORY_READABLE;
	kPacketReq.m_wstrGameID		   = GetName(); // �κ��丮 ������ ���� Nexon ID�� ����.
	kPacketReq.m_byteShowInventory = 10;		// 10 : true, 30 : false
	kPacketReq.m_byteIsPresent	   = 1;			// 0 : �Ϲݱ���, 1 : ����, 2 : �Ϲݱ���+����

	KEventPtr spEvent( new KEvent );
	UidType anTrace[2] = { GetUID(), -1 };
	spEvent->SetData( PI_GS_NX_BILLING_TCP, anTrace, ENX_BT_NISMS_INVENTORY_READABLE_REQ, kPacketReq );
	SiKNexonBillingTCPManager()->QueueingEvent( spEvent );

	//{{ 2009. 10. 14  ������	�ڵ�����
#ifdef AP_RESTONE
	// �ڵ����� ��ǰ üũ
	// �� �Ϸ翡 �ѹ��� ���������� ��������! ���������� �����ս��� ��ƸԴ� �̺�Ʈ��

	CTime tCurTime = CTime::GetCurrentTime();
	CTime tCheckTime = CTime( tCurTime.GetYear(), tCurTime.GetMonth(), tCurTime.GetDay(), 6, 0, 0 );

	// ���� 6�ø� �������� üũ�ϱ� ���� ���ǰ˻�
	if( tCurTime.GetHour() >= 0  &&  tCurTime.GetHour() < 6 )
	{
		tCheckTime -= CTimeSpan( 1, 0, 0, 0 );
	}

	// ���� �ڵ����� üũ ��Ŷ ���� �ð��� ���� 6�÷� ����!
	m_tAutoPaymentCheckTime = tCheckTime + CTimeSpan( 1, 0, 0, 0 );

	CTime tLastRefillDate;
	if( KncUtil::ConvertStringToCTime( kAck_.m_wstrAutoPayResStoneLastDate, tLastRefillDate ) )
	{
		if( tLastRefillDate < tCheckTime  &&
			m_iNumAutoPaymentResStone == 0 ) // �ڵ����� ��Ȱ���� 0���϶��� üũ.
		{
			std::vector< unsigned long > vecAutoPaymentProductList;
			SiKNexonBillingTCPManager()->GetAutoPaymentProductList( vecAutoPaymentProductList );

			KENX_BT_CHECK_AUTO_PAYMENT_USER_REQ kPacketReq;
			kPacketReq.m_ulPacketNo		= SiKNexonBillingTCPManager()->GetNextPacketNo();
			kPacketReq.m_bytePacketType	= KNexonBillingTCPPacket::CHECK_AUTO_PAYMENT_USER;
			kPacketReq.m_wstrGameID		= GetName(); // �κ��丮 ������ ���� Nexon ID�� ����.
			kPacketReq.m_wstrUserID		= GetName(); // �κ��丮 ������ ���� Nexon ID�� ����.

			if( KSimLayer::GetKObj()->GetBillingFlag() == KSimLayer::BF_NEXON_KOREA )
			{
				kPacketReq.m_ulProductNo = 67215; // ���� : �ں� ����Ʈ ���� 
			}
			else
			{
				kPacketReq.m_ulProductNo = 77697; // �׼� : �ں� ����Ʈ ���� 
			}

			KEventPtr spEvent( new KEvent );
			UidType anTrace[2] = { GetUID(), -1 };
			spEvent->SetData( PI_GS_NX_BILLING_TCP, anTrace, ENX_BT_CHECK_AUTO_PAYMENT_USER_REQ, kPacketReq );
			SiKNexonBillingTCPManager()->QueueingEvent( spEvent );

			START_LOG( clog, L"�ڵ����� �ߴ��� üũ�Ϸ� ����!" )
#ifdef SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetUID() )
#else
				<< BUILD_LOG( GetName() )
#endif SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( kPacketReq.m_ulProductNo );
		}
		else
		{
			START_LOG( clog, L"�ڵ����� üũ�ð��� ������ �׳� ���!" )
#ifdef SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetUID() )
#else
				<< BUILD_LOG( GetName() )
#endif SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( kAck_.m_wstrAutoPayResStoneLastDate )
				<< BUILD_LOG( m_iNumAutoPaymentResStone );
		}
	}
	
#endif AP_RESTONE
	//}}

	// ���� ����Ʈ ��û
	KDBE_GET_POST_LETTER_LIST_REQ kPacketToDB;
	kPacketToDB.m_iUnitUID = GetCharUID();
	//{{ 2012. 11. 08	�ڼ���	�������� �����
#ifdef SERV_ELIOS_INVESTIGATIONS
	kPacketToDB.m_bIsChannelChange				= bIsChannelChange;
	kPacketToDB.m_bEliosInvestigationsReward	= kAck_.m_bEliosInvestigationsReward;
#endif SERV_ELIOS_INVESTIGATIONS
	//}}
	SendToGameDB( DBE_GET_POST_LETTER_LIST_REQ, kPacketToDB );

	//{{ 2010. 02. 09  ������	���� �̺�Ʈ ����Ʈ
#ifdef SERV_DAY_QUEST
	m_kUserQuestManager.InitEventQuest( GetThisPtr<KGSUser>() );
#endif SERV_DAY_QUEST
	//}}
	//{{ QUEST ���� - ��μ�
#ifdef SERV_REFORM_QUEST
	m_kUserQuestManager.CheckEventQuest( GetThisPtr<KGSUser>() );
	m_kUserQuestManager.CheckLimitLevelQuest( GetThisPtr<KGSUser>() );
#endif SERV_REFORM_QUEST
	//}}
	//{{ 2012. 12. 14  ���� �̼� �ý��� ( �������� ����Ʈ ) - ��μ�
#ifdef SERV_ACCOUNT_MISSION_SYSTEM
	m_kUserAccountQuestManager.CheckAccountQuest( GetThisPtr<KGSUser>() );
#endif SERV_ACCOUNT_MISSION_SYSTEM
	//}}

	// ���ο� �̼� �˻�
	m_kUserTitleManager.CheckNewMission( GetThisPtr<KGSUser>(), true );

	//{{ 2009. 9. 22  ������	���
#ifdef GUILD_TEST
	if( GetGuildUID() > 0 )
	{
		KELG_GET_GUILD_INFO_REQ kPacketToLg;
		kPacketToLg.m_iUnitUID = GetCharUID();
		kPacketToLg.m_iGuildUID = GetGuildUID();
		GetGuildMemberInfo( kPacketToLg.m_kLoginGuildMember );
		SendToLoginServer( ELG_GET_GUILD_INFO_REQ, kPacketToLg );
	}
#endif GUILD_TEST
	//}}

	//{{ 2011. 05. 10	������	�븮����
#ifdef SERV_PSHOP_AGENCY
	if( kAck_.m_kPShopAgencyInfo.m_bIsPShopOpen )
	{
        KERM_CHECK_MY_PSHOP_AGENCY_INFO_REQ kPacketToLgRoom;
		kPacketToLgRoom.m_iUnitUID = GetCharUID();
		SendToLoginRoom( ERM_CHECK_MY_PSHOP_AGENCY_INFO_REQ, kPacketToLgRoom );
	}
#endif SERV_PSHOP_AGENCY
	//}}
	
	if( bIsChannelChange == false )
	{
		//{{ 2009. 12. 4  ������	���ӽð��̺�Ʈ�ý��۰���
		KEGS_TIME_EVENT_INFO_NOT kPacketNot;
		kPacketNot.m_cNotifyType = KEGS_TIME_EVENT_INFO_NOT::TENT_INIT;
		m_kUserEventManager.GetNowProcessingTimeEvent( kPacketNot.m_vecTimeEvent );
		if( !kPacketNot.m_vecTimeEvent.empty() )
		{
			SendPacket( EGS_TIME_EVENT_INFO_NOT, kPacketNot );
		}
		//}}

		//{{ 2010. 01. 11  ������	��õ�θ���Ʈ
#ifdef SERV_RECOMMEND_LIST
		SendUpdateRecommendUserInfo( KRecommendUserInfo::RUIT_LOGIN );
#endif SERV_RECOMMEND_LIST
		//}}

		//{{ 2010. 01. 29  ������	PC�� ���ȭ
		// Ŭ���̾�Ʈ�� �޽��� ���
		if( m_kUserPcBangManager.IsReservedPcBangMessage() == true )
		{
			KEGS_PC_BANG_AUTH_RESULT_NOT kPacketNot;
			kPacketNot.m_cType = KEGS_PC_BANG_AUTH_RESULT_NOT::PBRT_MESSAGE;
			kPacketNot.m_uiData = m_kUserPcBangManager.GetPcBangMessageArgument();
			SendPacket( EGS_PC_BANG_AUTH_RESULT_NOT, kPacketNot );
		}
		//}}

		//{{ 2010. 8. 16	������	�Ⱓ ���� ������ �̺�Ʈ
#ifdef SERV_RESET_PERIOD_EVENT
		// �Ⱓ ���� �̺�Ʈ �������� �ִ��� �κ��丮�� ã�ƺ���!
		KDBE_RESET_PERIOD_ITEM_REQ kPacketToDB;
		m_kInventory.GetResetPeriodItemUIDList( kPacketToDB.m_mapExpandPeriodItemList );

		// ������ �Ⱓ �ʱ�ȭ ��Ű�� DB�� ����!
		if( kPacketToDB.m_mapExpandPeriodItemList.empty() == false )
		{
			kPacketToDB.m_iUnitUID = GetCharUID();
			SendToGameDB( DBE_RESET_PERIOD_ITEM_REQ, kPacketToDB );
		}
#endif SERV_RESET_PERIOD_EVENT
		//}}

		//{{ 2011. 01. 17	������	ĳ���� ī��Ʈ ����
#ifdef SERV_CHAR_LOG
		m_kUserStatistics.IncreaseCharacterCount( KUserStatistics::CGCT_SELECT_CHAR );		

		const int iSelectCount = m_kUserStatistics.GetCharacterCount( KUserStatistics::CGCT_SELECT_CHAR );
		if( iSelectCount == 2 )
		{
			WriteCharacterLogToDB( KELOG_USER_CHARACTER_LOG_NOT::LT_SECOND_SELECT );
		}
		else if( iSelectCount == 3 )
		{
			WriteCharacterLogToDB( KELOG_USER_CHARACTER_LOG_NOT::LT_THIRD_SELECT );
		}

		if( GetGuildUID() != 0 )
		{
			m_kUserStatistics.IncreaseCharacterCount( KUserStatistics::CGCT_JOIN_GUILD_SELECT_CHAR );

			const int iJoinGuildSelectCount = m_kUserStatistics.GetCharacterCount( KUserStatistics::CGCT_JOIN_GUILD_SELECT_CHAR );
			if( iJoinGuildSelectCount == 1 )
			{
				WriteCharacterLogToDB( KELOG_USER_CHARACTER_LOG_NOT::LT_JOIN_GUILD_SELECT_CHAR );
			}
		}
#endif SERV_CHAR_LOG
		//}}
	}

	// ����͸� �����	
	//if( m_bIsMornitoringAbuser )
	//{
	//	KDBE_SEND_PHONE_MSG_NOT kPacketNot;
	//	kPacketNot.m_wstrSMSMessage  = L"����� ���� [";
	//	kPacketNot.m_wstrSMSMessage += GetName();
	//	kPacketNot.m_wstrSMSMessage += L"]�� ĳ���� [";
	//	kPacketNot.m_wstrSMSMessage += GetCharName();
	//	kPacketNot.m_wstrSMSMessage += L"]�� �����Ͽ����ϴ�!";			

	//	SiKAbuserLogManager()->GetAbuserNotifyPhoneNumList( kPacketNot.m_wstrPhoneNum );

	//	SendToSMSDB( DBE_SEND_PHONE_MSG_NOT, kPacketNot );
	//}

#ifdef SERV_ANTI_ADDICTION_SYSTEM
	//	if( KSimLayer::GetKObj()->GetAuthFlag() == KSimLayer::AF_GIANT_CHINA )
	{
		if(m_GiantAccountType.IsType(KGiantAccountType::ADULT_PLAYER) == false)
		{
			START_LOG( clog, L"�̼�����" )
				<< BUILD_LOG( GetUID() )
				//<< BUILD_LOG( m_GiantAccountType.m_uiType )
				<< END_LOG;

			// 2010.08.07	�ڱ���		��ħ�� ������
			SendToAccountDB( DBE_GET_ANTI_ADDICTION_INFO_REQ, GetUID() );
		}
	}
#endif SERV_ANTI_ADDICTION_SYSTEM

#ifdef SERV_PERSONAL_SHOP_NO_MOVE
	m_vecPersonalShopSell_ItemInfo.clear();
#endif //SERV_PERSONAL_SHOP_NO_MOVE

	//{{ 2012 ���� ����2 ���� ��Ī �̺�Ʈ	- ��μ�
#ifdef SERV_2012_PVP_SEASON2_EVENT
	// �̺�Ʈ �ð� �ΰ�?
	CTime tCurr = CTime::GetCurrentTime();
	int iIndex = 0;
	if( SiKGameSysVal()->IsPvpEventTime( tCurr, iIndex, kAck_.m_wstrLastPvpEventDate, m_bIsPvpEventUser ) == true )
	{
		// ����� �̺�Ʈ �ð��̴�!
		m_bNowPvpEventTime = true;

		// Ŭ�󿡰� ������ ����� �˷�����
		KEGS_PVP_EVENT_INFO_NOT kNot;
		kNot.m_bEventInfo = true;

		SendPacket( EGS_PVP_EVENT_INFO_NOT, kNot );			
	}
#endif SERV_2012_PVP_SEASON2_EVENT
	//}}

	//{{ 2013. 01. 21  ��Ը� �̺�Ʈ ���� �� ���� ���� ĳ���Ϳ��� �����ϴ� �ý��� - ��μ�
#ifdef SERV_FIRST_SELECT_UNIT_REWARD_SYSTEM
	{
		std::vector<KFirstSelectUnitReward> vecRewardInfo;
		if( m_kUserUnitSelectRewardManager.GetFirstSelectUnitRewardInfo( KBaseServer::GetKObj()->GetServerGroupID(), vecRewardInfo ) == true )
		{
			BOOST_TEST_FOREACH( KFirstSelectUnitReward, kInfo, vecRewardInfo )
			{
				// DB�� ���� ������ ������.
				KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
				kPacketToDB.m_iFromUnitUID	= GetCharUID();
				kPacketToDB.m_iToUnitUID	= GetCharUID();
				kPacketToDB.m_iRewardID		= kInfo.m_iItemID;
				kPacketToDB.m_sQuantity		= static_cast<short>(kInfo.m_iQuantity);
				kPacketToDB.m_iDBIndex		= kInfo.m_iDBIndexID;

				CStringW cwstrMessage;

				switch( kInfo.m_cRewardType )
				{
				case KFirstSelectUnitReward::FSUR_WEB_EVENT:
					{
						kPacketToDB.m_iRewardType	= KPostItemInfo::LT_DB_REWARD; // ���� Ÿ��

						cwstrMessage.Format( L"%d", kInfo.m_iItemID );
						kPacketToDB.m_wstrMessage = static_cast<LPCTSTR>(cwstrMessage);
					}break;
				case KFirstSelectUnitReward::FSUR_SYSTEM_ERROR:
					{
						kPacketToDB.m_iRewardType	= KPostItemInfo::LT_SYSTEM_ERROR_REWARD; // ���� Ÿ��

						cwstrMessage.Format( L"%d", kInfo.m_iItemID );
						kPacketToDB.m_wstrMessage = static_cast<LPCTSTR>(cwstrMessage);
					}break;
				default:
					{
						START_LOG( cerr, L"�������� �ʴ� ĳ���� ���� �� ���� ���� Ÿ���Դϴ�." )
							<< BUILD_LOG( GetUID() )
							<< BUILD_LOGc( kInfo.m_cRewardType )
							<< BUILD_LOG( kInfo.m_iItemID )
							<< BUILD_LOG( kInfo.m_iQuantity )
							<< BUILD_LOG( kInfo.m_iDBIndexID )
							<< END_LOG;
					}
				}

				m_kUserUnitSelectRewardManager.UpdateFirstSelectUnitRewardInfo( kInfo.m_iDBIndexID, GetCharUID() );

				SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
			}
		}
	}
#endif SERV_FIRST_SELECT_UNIT_REWARD_SYSTEM
	//}
}
//}}

//{{ 2010. 8. 4	������	���� �ڵ� ����
//#ifdef SERV_DESELECT_UNIT_REFAC

//{{ 2012. 11. 22	������		ĳ���� ���� ���� �ڵ� �����丵
//////////////////////////////////////////////////////////////////////////
#ifdef SERV_DESELECT_UNIT_CODE_REFACTORING
//////////////////////////////////////////////////////////////////////////
void KGSUser::OnDeselectUnit( IN const u_short usEventID, IN const bool bOnDestroy )
{
	// ĳ���Ͱ� ���� �Ǿ� ���� �ʴٸ� �׳� �Ѿ� ����!
	if( GetCharUID() == 0 )
		return;

	if( GetCharUID() < 0 )
	{
		START_LOG( cerr, L"UnitUID���� ���̳ʽ���! ���� �� ���� ����!" )
			<< BUILD_LOG( GetCharUID() )
#ifndef SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( usEventID )
#endif SERV_PRIVACY_AGREEMENT
			<< END_LOG;
		return;
	}


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// �ϴ� ���� ��Ż���� ���� ó���� ������!

	// 1. ������������Ʈ
	SendLeavePVPLobby();

	//{{ 2011. 07. 11	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
	SendLeavePVPMatch( NetError::NOT_LEAVE_MATCH_WAIT_REASON_02 );
#endif SERV_PVP_NEW_SYSTEM
	//}}

	//{{ 2012. 02. 10	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	SendLeaveAutoParty( NetError::NOT_LEAVE_AUTO_PARTY_WAIT_REASON_02 );
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}

	// 2. ResetUnitInfo���� UnitUID�� 0���� �ʱ�ȭ �ϱ� ������ leaveó���� �������־�� �Ѵ�.
	SendLeaveParty( NetError::NOT_LEAVE_ROOM_REASON_01 );

	// ä���̵����� ���� ĳ���� ����������� Ŭ���̾�Ʈ�� ��Ż ��Ŷ�� �ٷ� ����������!
	if( usEventID == EGS_DISCONNECT_FOR_CHANNEL_CHANGE_REQ )
	{
		if( GetPartyUID() > 0 )
		{
			// ��Ż ó��
			KEGS_LEAVE_PARTY_ACK kPacket;
			kPacket.m_iOK = NetError::NET_OK;
			SendPacket( EGS_LEAVE_PARTY_ACK, kPacket );
		}
	}

	// 3. �ʵ嵵 ��������..
	SendLeaveField( NetError::NOT_LEAVE_FIELD_REASON_02 );

	// 4. ���ΰŷ� ��Ż ó��
	if( GetTradeUID() != 0 )
	{
		KEGS_BREAK_TRADE_REQ kPacket;
		kPacket.m_iReason	= NetError::NOT_LEAVE_ROOM_REASON_01;
		_SendToCnRoom( GetTradeUID(), ERM_BREAK_TRADE_REQ, kPacket );

		SetTradeUID( 0 );
	}

	// 5. ���λ��� ��Ż ó��
	if( GetPersonalShopUID() != 0 )
	{
		//{{ 2011. 05. 02	������	�븮����
#ifdef SERV_PSHOP_AGENCY
		if( CXSLRoom::GetRoomType( GetPersonalShopUID() ) == CXSLRoom::RT_PERSONAL_SHOP )
		{
			KEGS_BREAK_PERSONAL_SHOP_REQ kPacket;
			kPacket.m_iReason	= NetError::NOT_LEAVE_ROOM_REASON_01;
			_SendToCnRoom( GetPersonalShopUID(), ERM_BREAK_PERSONAL_SHOP_REQ, kPacket );
		}
		else
		{
			KERM_LEAVE_PERSONAL_SHOP_REQ kPacket;
			kPacket.m_iReason	= NetError::NOT_LEAVE_ROOM_REASON_01;
			_SendToLoginRoom( GetPersonalShopUID(), ERM_LEAVE_PSHOP_AGENCY_REQ, kPacket );
		}
#else
		KEGS_BREAK_PERSONAL_SHOP_REQ kPacket;
		kPacket.m_iReason	= NetError::NOT_LEAVE_ROOM_REASON_01;
		_SendToCnRoom( GetPersonalShopUID(), ERM_BREAK_PERSONAL_SHOP_REQ, kPacket );
#endif SERV_PSHOP_AGENCY
		//}}

		SetPersonalShopUID( 0 );
	}

	// 6. ���� �濡 �ִٸ�..
	if( GetRoomUID() > 0 )
	{
		switch( CXSLRoom::GetRoomType( GetRoomUID() ) )
		{
		case CXSLRoom::RT_SQUARE:
			{
				KEGS_LEAVE_SQUARE_REQ kPacket;
				kPacket.m_iReason	= NetError::NOT_LEAVE_ROOM_REASON_01;
				SendToCnRoom( ERM_LEAVE_SQUARE_REQ, kPacket );
			}
			break;

		case CXSLRoom::RT_TRAININGCENTER:
			{
				KERM_LEAVE_ROOM_REQ kPacket;

				// Ŭ���̾�Ʈ ũ������ ��� [Ŭ���̾�Ʈ ũ������ ���Ϳ��� ���� ��Ŷ�� ������ �ʵ���]
				if( GetDisconnectReason() == KStatistics::eSIColDR_Client_Crash )
				{
					kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_23;
				}
				else
				{
					kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_01;
				}

				SendToCnRoom( ERM_LEAVE_TC_ROOM_REQ, kPacket );
			}
			break;

		case CXSLRoom::RT_BATTLE_FIELD:
			{
				KERM_LEAVE_ROOM_REQ kPacket;
				kPacket.m_bSendAck = false;	// [2012-12-07][������] �� ��Ŷ ������ ������ ACK�� ���� �ʿ䰡 �����Ƿ� SendAck���� false�� ����.

				// ���� ������ ���
				if( bOnDestroy )
				{
					// ���� ���� ������ Neterror�� ���Ѵ�.
					switch( GetDisconnectReason() )
					{
					case KStatistics::eSIColDR_Client_Crash:	kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_23;	break;
					case KStatistics::eSIColDR_IOCP_GetStatus:	kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_33; break;
					case KStatistics::eSIColDR_Channel_Change:	kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_29; break;
					default:									kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_01; break;
					}
				}
				else
				{
					switch( usEventID )
					{
					case EGS_DISCONNECT_FOR_CHANNEL_CHANGE_REQ:	kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_29; break;
					default:									kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_01; break;
					}
				}
				

				SendToCnRoom( ERM_LEAVE_ROOM_REQ, kPacket );
			}
			break;

		default:
			{
				KERM_LEAVE_ROOM_REQ kPacket;

				// Ŭ���̾�Ʈ ũ������ ��� [Ŭ���̾�Ʈ ũ������ ���Ϳ��� ���� ��Ŷ�� ������ �ʵ���]
				if( GetDisconnectReason() == KStatistics::eSIColDR_Client_Crash )
				{
					kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_23;
				}
				else
				{
					kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_01;
				}

				SendToCnRoom( ERM_LEAVE_ROOM_REQ, kPacket );
			}
			break;
		}

		//{{ 2010. 12. 30	������	����Ż ����ó��
#ifdef SERV_LEAVE_ROOM_BUG_FIX
		SetRoomListID( 0 );
#endif SERV_LEAVE_ROOM_BUG_FIX
		//}}

		//{{ 2012. 02. 07	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
		SetRoomUID( 0 );

		if( GetStateID() == KGSFSM::S_ROOM )
		{
			StateTransition( KGSFSM::I_TO_FIELD_MAP );
		}
#endif SERV_BATTLE_FIELD_SYSTEM
		//}}
	}


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// DB�� ĳ���� ������ ������Ʈ �ϱ� ���� �ؾ��� �۾��� ���⼭ �ص���!

	//{{ 2010. 01. 11  ������	��õ�θ���Ʈ
#ifdef SERV_RECOMMEND_LIST
	// 7. ��õ�� ����Ʈ
	SendUpdateRecommendUserInfo( KRecommendUserInfo::RUIT_LOGOUT );
#endif SERV_RECOMMEND_LIST
	//}}

	//{{ 2012. 07. 12	������		���� ť�� ���� ������ �ý���
#ifdef SERV_OPEN_RANDOM_CUBE_DELAY_SYSTEM
	SiKEventDelaySystemManager()->UnRegOpenRandomCubeEvent( GetCharUID() );
#endif SERV_OPEN_RANDOM_CUBE_DELAY_SYSTEM
	//}}

	//{{ 2010. 8. 4	������	�� �ý���
#ifdef SERV_PET_SYSTEM
	// ĳ���� ���� ������ �� ���� ó��!
	m_kUserPetManager.OnDeselectUnit( GetThisPtr<KGSUser>() );
#endif SERV_PET_SYSTEM
	//}}

//#ifdef	SERV_SHARING_BANK_TEST -- �ǽð� ������Ʈ�� �ٲ�� ������ ���⼭ ó���� �ʿ䰡 ����.
//	KDBE_UPDATE_SHARE_ITEM_REQ kPacketSendDB;
//
//	//	������� ���
//	if( m_kInventory.GetShareItemNeedDBUpdate( kPacketSendDB ) == true )
//	{
//		kPacketSendDB.m_bReload = false;
//		SendToGameDB( DBE_UPDATE_SHARE_ITEM_REQ, kPacketSendDB );
//	}
//#endif	SERV_SHARING_BANK_TEST

	//{{  2011.11.24     ��μ�    ĳ���� ���� �α�
#ifdef SERV_CHAR_CONNECT_LOG
	SendUpdateUnitConnectLog();
#endif SERV_CHAR_CONNECT_LOG
	//}}

	//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
	CheckDeselectUnitBuff();
#endif SERV_SERVER_BUFF_SYSTEM
	//}

	//{{ 2012. 12. 14  ���� �̼� �ý��� ( �������� ����Ʈ ) - ��μ�
#ifdef SERV_ACCOUNT_MISSION_SYSTEM
	SendUpdateAccountQuestInfo();
#endif SERV_ACCOUNT_MISSION_SYSTEM
	//}}

#ifdef	SERV_RIDING_PET_SYSTM// ���볯¥: 2013-04-21
	m_kUserRidingPetManager.OnDeselectUnit();
#endif	// SERV_RIDING_PET_SYSTM

	//�����ý��ۿ� ����� �Ǿ��ִٸ� ������ �������ش�.
	m_kUserTutorialInfo.DisconnectUpdateUnitInfo( GetThisPtr<KGSUser>() );

	if( m_kUserTutorialInfo.GetTutorUnitType() == KUserTutorialInfo::TUT_STUDENT  &&  SiKTutorialManager()->IsExistbylist( GetCharUID() ) == true )
	{
		if( SiKTutorialManager()->DeleteUnit( GetCharUID() ) == false )
		{
			START_LOG( cerr, L"�缼�ý����� �������� ����� ����.!" )
				<< BUILD_LOG( GetCharName() )
				<< BUILD_LOG( GetLevel() )
				<< END_LOG;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// ĳ���� ���� DB������Ʈ
	WriteUnitInfoToDB( true, usEventID );

	//{{ 2012. 09. 22	�ڼ���	���� �̺�Ʈ
#ifdef SERV_EVENT_BINGO
	BingoEventDBWrite();
#endif SERV_EVENT_BINGO
	//}}

#ifdef SERV_SYNC_PACKET_USING_RELAY_WORKINGS_CHECK// �۾���¥: 2013-05-14	// �ڼ���
	SetLanBugOutDisconnectCheck( false );
#endif // SERV_SYNC_PACKET_USING_RELAY_WORKINGS_CHECK

	//{{ 2013. 09. 24	������	�Ϻ� �̺�Ʈ �߰�DB�۾�
#ifdef SERV_RELAY_DB_CONNECTION
	SendUpdateUnitInfoToRelayDB( KDBE_UPDATE_USER_INFO_TO_RELAY_DB_JP_EVENT_NOT::UT_NONE );
#endif SERV_RELAY_DB_CONNECTION
	//}}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// ��質 �α׿� ���õ� ó���� ���⼭ ����!
	
	{
		// �α��� ������ ��ϵ� ĳ���� ���� ����
		KELG_UNIT_DISCONNECT_NOT kPacketNot;
		kPacketNot.m_iUnitUID = GetCharUID();
		//{{ 2009. 9. 29  ������	���
#ifdef GUILD_TEST
		kPacketNot.m_iGuildUID = GetGuildUID();
#endif GUILD_TEST
		//}}
		BOOST_MAP_CONST_FOREACH( UidType, KFriendInfo, kFriend, m_kMessengerInfo.m_mapFriendInfo )
		{
			kPacketNot.m_setFriendUID.insert( kFriend.first );
		}
		SendToLoginServer( ELG_UNIT_DISCONNECT_NOT, kPacketNot );
	}

	{
		//{{ 2012. 03. 20	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
		// �۷ι� ������ ��ϵ� ĳ���� ���� ����
		KEGB_UNIT_DISCONNECT_NOT kPacketNot;
		kPacketNot.m_iUnitUID = GetCharUID();
		//{{ 2012. 12. 17	�ڼ���	���� �̺�Ʈ
#ifdef SERV_EVENT_JACKPOT
		kPacketNot.m_iUserUID	= GetUID();
#endif SERV_EVENT_JACKPOT
		//}}
		SendToGlobalServer( EGB_UNIT_DISCONNECT_NOT, kPacketNot );
#endif SERV_BATTLE_FIELD_SYSTEM
		//}}
	}

	{
		//{{ 2011. 01. 17	������	ĳ���� ī��Ʈ ����
#ifdef SERV_CHAR_LOG
		m_kUserStatistics.IncreaseCharacterCount( KUserStatistics::CGCT_DESELECT_CHAR );

		const int iDeselectCount = m_kUserStatistics.GetCharacterCount( KUserStatistics::CGCT_DESELECT_CHAR );
		if( iDeselectCount == 1 )
		{
			WriteCharacterLogToDB( KELOG_USER_CHARACTER_LOG_NOT::LT_FIRST_DESELECT );
		}
		else if( iDeselectCount == 2 )
		{
			WriteCharacterLogToDB( KELOG_USER_CHARACTER_LOG_NOT::LT_SECOND_DESELECT );
		}
#endif SERV_CHAR_LOG
		//}}
	}

#ifdef SERV_LOCAL_RANKING_SYSTEM_STATISTICS_LOG// �۾���¥: 2013-05-15	// �ڼ���
	if( ( m_kUserLocalRankingInfo.InitCheckUserInfo( GetUID() ) == true ) &&
		( 0 < GetLocalRankingButtonClickCount() )
		)
	{
		KDBE_LOCAL_RANKING_SYSTEM_STATISTICS_LOG_NOT kPacketNot;
		kPacketNot.m_iUserUID					= GetUID();
		kPacketNot.m_iUnitUID					= GetCharUID();
		kPacketNot.m_ucLevel					= GetLevel();
		kPacketNot.m_bLocalRankingUser			= m_kUserLocalRankingInfo.IsValidCategory();
		kPacketNot.m_usLocalRankingClickCount	= GetLocalRankingButtonClickCount();
		SendToLogDB( DBE_LOCAL_RANKING_SYSTEM_STATISTICS_LOG_NOT, kPacketNot );

		ResetLocalRankingButtonClickCount();
	}
#endif // SERV_LOCAL_RANKING_SYSTEM_STATISTICS_LOG
}
//////////////////////////////////////////////////////////////////////////
#else
//////////////////////////////////////////////////////////////////////////
void KGSUser::OnDeselectUnit( IN const bool bFinal, IN const u_short usEventID )
{
	// ������ ���õǾ� �ִٸ� �ش� ���������� DB������Ʈ, �α��μ������� �����, ������ ó���� �Ѵ�.
	if( GetCharUID() == 0 )
		return;

	if( GetCharUID() < 0 )
	{
		START_LOG( cerr, L"UnitUID���� ���̳ʽ���! ���� �� ���� ����!" )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOG( bFinal )
			<< BUILD_LOG( usEventID )
			<< END_LOG;
		return;
	}
	
#ifdef SERV_IDENTITY_CONFIRM_POPUP_MESSAGE
	m_bIdentityConfirmCheck = false;
#endif //SERV_IDENTITY_CONFIRM_POPUP_MESSAGE
	// 1. ������������Ʈ
	SendLeavePVPLobby();

	//{{ 2011. 07. 11	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
	SendLeavePVPMatch( NetError::NOT_LEAVE_MATCH_WAIT_REASON_02 );
#endif SERV_PVP_NEW_SYSTEM
	//}}

	//{{ 2012. 02. 10	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	SendLeaveAutoParty( NetError::NOT_LEAVE_AUTO_PARTY_WAIT_REASON_02 );
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}

	// 2. ResetUnitInfo���� UnitUID�� 0���� �ʱ�ȭ �ϱ� ������ leaveó���� �������־�� �Ѵ�.
	SendLeaveParty( NetError::NOT_LEAVE_ROOM_REASON_01 );

	// ä���̵����� ���� ĳ���� ����������� Ŭ���̾�Ʈ�� ��Ż ��Ŷ�� �ٷ� ����������!
	if( usEventID == EGS_DISCONNECT_FOR_CHANNEL_CHANGE_REQ )
	{
		if( GetPartyUID() > 0 )
		{
			// ��Ż ó��
			KEGS_LEAVE_PARTY_ACK kPacket;
			kPacket.m_iOK = NetError::NET_OK;
			SendPacket( EGS_LEAVE_PARTY_ACK, kPacket );
		}
	}	

	// 3. �ʵ嵵 ��������..
	SendLeaveField( NetError::NOT_LEAVE_FIELD_REASON_01 );

	// 4. ���ΰŷ� ��Ż ó��
	if( GetTradeUID() != 0 )
	{
		START_LOG( cerr, L"�� �ڵ鷯������ TradeUID�� 0�̾���Ѵ�! ���� �ֿ���?" )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOG( GetTradeUID() )
			<< END_LOG;

		KEGS_BREAK_TRADE_REQ kPacket;
		kPacket.m_iReason	= NetError::NOT_LEAVE_ROOM_REASON_01;
		_SendToCnRoom( GetTradeUID(), ERM_BREAK_TRADE_REQ, kPacket );

		SetTradeUID( 0 );
	}

	// 5. ���λ��� ��Ż ó��
	if( GetPersonalShopUID() != 0 )
	{
		START_LOG( cerr, L"�� �ڵ鷯������ PersonalShopUID�� 0�̾���Ѵ�! ���� �ֿ���?" )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOG( GetPersonalShopUID() )
			<< END_LOG;

		//{{ 2011. 05. 02	������	�븮����
#ifdef SERV_PSHOP_AGENCY
		if( CXSLRoom::GetRoomType( GetPersonalShopUID() ) == CXSLRoom::RT_PERSONAL_SHOP )
		{
			KEGS_BREAK_PERSONAL_SHOP_REQ kPacket;
			kPacket.m_iReason	= NetError::NOT_LEAVE_ROOM_REASON_01;
			_SendToCnRoom( GetPersonalShopUID(), ERM_BREAK_PERSONAL_SHOP_REQ, kPacket );
		}
		else
		{
			KERM_LEAVE_PERSONAL_SHOP_REQ kPacket;
			kPacket.m_iReason	= NetError::NOT_LEAVE_ROOM_REASON_01;
			_SendToLoginRoom( GetPersonalShopUID(), ERM_LEAVE_PSHOP_AGENCY_REQ, kPacket );
		}
#else
		KEGS_BREAK_PERSONAL_SHOP_REQ kPacket;
		kPacket.m_iReason	= NetError::NOT_LEAVE_ROOM_REASON_01;
		_SendToCnRoom( GetPersonalShopUID(), ERM_BREAK_PERSONAL_SHOP_REQ, kPacket );
#endif SERV_PSHOP_AGENCY
		//}}

		SetPersonalShopUID( 0 );
	}

	// 6. ���� �濡 �ִٸ�..
	if( GetRoomUID() > 0 )
	{
		START_LOG( cerr, L"�� �ڵ鷯������ RoomUID�� 0�̾���Ѵ�! ���� �ֿ���?" )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOG( GetRoomUID() )
			<< BUILD_LOG( CXSLRoom::GetRoomType( GetRoomUID() ) )
			<< END_LOG;

		//{{ 2012. 10. 18	������	��Ʋ�ʵ� �ý���
		//////////////////////////////////////////////////////////////////////////
#ifdef SERV_BATTLE_FIELD_SYSTEM
		//////////////////////////////////////////////////////////////////////////
		switch( CXSLRoom::GetRoomType( GetRoomUID() ) )
		{
		case CXSLRoom::RT_SQUARE:
			{
				KEGS_LEAVE_SQUARE_REQ kPacket;
				kPacket.m_iReason	= NetError::NOT_LEAVE_ROOM_REASON_01;
				SendToCnRoom( ERM_LEAVE_SQUARE_REQ, kPacket );
			}
			break;

		case CXSLRoom::RT_TRAININGCENTER:
			{
				KERM_LEAVE_ROOM_REQ kPacket;

				// Ŭ���̾�Ʈ ũ������ ��� [Ŭ���̾�Ʈ ũ������ ���Ϳ��� ���� ��Ŷ�� ������ �ʵ���]
				if( GetDisconnectReason() == KStatistics::eSIColDR_Client_Crash )
				{
					kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_23;
				}
				else
				{
					kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_01;
				}

				SendToCnRoom( ERM_LEAVE_TC_ROOM_REQ, kPacket );
			}
			break;

		case CXSLRoom::RT_BATTLE_FIELD:
			{
				KERM_LEAVE_ROOM_REQ kPacket;

				// Ŭ���̾�Ʈ ũ������ ��� [Ŭ���̾�Ʈ ũ������ ���Ϳ��� ���� ��Ŷ�� ������ �ʵ���]
				switch( usEventID )
				{
				case EGS_DISCONNECT_FOR_CHANNEL_CHANGE_REQ:	kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_29; break;
				default:									kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_01; break;
				}

				SendToCnRoom( ERM_LEAVE_ROOM_REQ, kPacket );
			}
			break;

		default:
			{
				KERM_LEAVE_ROOM_REQ kPacket;

				// Ŭ���̾�Ʈ ũ������ ��� [Ŭ���̾�Ʈ ũ������ ���Ϳ��� ���� ��Ŷ�� ������ �ʵ���]
				if( GetDisconnectReason() == KStatistics::eSIColDR_Client_Crash )
				{
					kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_23;
				}
				else
				{
					kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_01;
				}

				SendToCnRoom( ERM_LEAVE_ROOM_REQ, kPacket );
			}
			break;
		}
		//////////////////////////////////////////////////////////////////////////
#else
		//////////////////////////////////////////////////////////////////////////
		if( CXSLRoom::GetRoomType( GetRoomUID() ) == CXSLRoom::RT_SQUARE )
		{
			KEGS_LEAVE_SQUARE_REQ kPacket;
			kPacket.m_iReason	= NetError::NOT_LEAVE_ROOM_REASON_01;
			SendToCnRoom( ERM_LEAVE_SQUARE_REQ, kPacket );
		}
		else
		{
			u_short usEventID;			
			if( CXSLRoom::GetRoomType( GetRoomUID() ) == CXSLRoom::RT_TRAININGCENTER )
			{
				usEventID = ERM_LEAVE_TC_ROOM_REQ;
			}
			else
			{
				usEventID = ERM_LEAVE_ROOM_REQ;
			}

			KERM_LEAVE_ROOM_REQ kPacket;
			kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_01;
			SendToCnRoom( usEventID, kPacket );
		}
		//////////////////////////////////////////////////////////////////////////
#endif SERV_BATTLE_FIELD_SYSTEM
		//////////////////////////////////////////////////////////////////////////
		//}}		

		//{{ 2012. 02. 07	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
		SetRoomUID( 0 );

		if( GetStateID() == KGSFSM::S_ROOM )
		{
			StateTransition( KGSFSM::I_TO_FIELD_MAP );
		}
#endif SERV_BATTLE_FIELD_SYSTEM
		//}}
	}

	//{{ 2010. 01. 11  ������	��õ�θ���Ʈ
#ifdef SERV_RECOMMEND_LIST
	// 7. ��õ�� ����Ʈ
	SendUpdateRecommendUserInfo( KRecommendUserInfo::RUIT_LOGOUT );
#endif SERV_RECOMMEND_LIST
	//}}

	//{{ 2012. 07. 12	������		���� ť�� ���� ������ �ý���
#ifdef SERV_OPEN_RANDOM_CUBE_DELAY_SYSTEM
	SiKEventDelaySystemManager()->UnRegOpenRandomCubeEvent( GetCharUID() );
#endif SERV_OPEN_RANDOM_CUBE_DELAY_SYSTEM
	//}}

	//{{ 2010. 8. 4	������	�� �ý���
#ifdef SERV_PET_SYSTEM
	if( bFinal )
	{
		// ĳ���� ���� ������ �� ���� ó��!
        m_kUserPetManager.OnDeselectUnit( GetThisPtr<KGSUser>() );
	}
#endif SERV_PET_SYSTEM
	//}}

	//{{ 2011. 01. 17	������	ĳ���� ī��Ʈ ����
#ifdef SERV_CHAR_LOG
	m_kUserStatistics.IncreaseCharacterCount( KUserStatistics::CGCT_DESELECT_CHAR );

	const int iDeselectCount = m_kUserStatistics.GetCharacterCount( KUserStatistics::CGCT_DESELECT_CHAR );
	if( iDeselectCount == 1 )
	{
		WriteCharacterLogToDB( KELOG_USER_CHARACTER_LOG_NOT::LT_FIRST_DESELECT );
	}
	else if( iDeselectCount == 2 )
	{
		WriteCharacterLogToDB( KELOG_USER_CHARACTER_LOG_NOT::LT_SECOND_DESELECT );
	}
#endif SERV_CHAR_LOG
	//}}

//#ifdef	SERV_SHARING_BANK_TEST -- �ǽð� ������Ʈ�� �ٲ�� ������ ���⼭ ���� ó���� �ʿ䰡 ����!
//	KDBE_UPDATE_SHARE_ITEM_REQ kPacketSendDB;
//
//	//	������� ���
//	if( m_kInventory.GetShareItemNeedDBUpdate( kPacketSendDB ) == true )
//	{
//		kPacketSendDB.m_bReload = false;
//		SendToGameDB( DBE_UPDATE_SHARE_ITEM_REQ, kPacketSendDB );
//	}
//#endif	SERV_SHARING_BANK_TEST


	//{{  2011.11.24     ��μ�    ĳ���� ���� �α�
#ifdef SERV_CHAR_CONNECT_LOG
	SendUpdateUnitConnectLog();
#endif SERV_CHAR_CONNECT_LOG
	//}}

	//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
	CheckDeselectUnitBuff();
#endif SERV_SERVER_BUFF_SYSTEM
	//}

	// ĳ���� ���� DB������Ʈ
	WriteUnitInfoToDB( bFinal, usEventID );

	//{{ 2012. 09. 22	�ڼ���	���� �̺�Ʈ
#ifdef SERV_EVENT_BINGO
	BingoEventDBWrite();
#endif SERV_EVENT_BINGO
	//}}

	// �α��� ������ ��ϵ� ĳ���� ���� �ʱ�ȭ
	{
		KELG_UNIT_DISCONNECT_NOT kPacketNot;
		kPacketNot.m_iUnitUID = GetCharUID();
		//{{ 2009. 9. 29  ������	���
#ifdef GUILD_TEST
		kPacketNot.m_iGuildUID = GetGuildUID();
#endif GUILD_TEST
		//}}
		BOOST_MAP_CONST_FOREACH( UidType, KFriendInfo, kFriend, m_kMessengerInfo.m_mapFriendInfo )
		{
			kPacketNot.m_setFriendUID.insert( kFriend.first );
		}
		SendToLoginServer( ELG_UNIT_DISCONNECT_NOT, kPacketNot );

		//{{ 2012. 03. 20	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
		KEGB_UNIT_DISCONNECT_NOT kPacketToGB;
		kPacketToGB.m_iUnitUID = GetCharUID();
		//{{ 2012. 12. 17	�ڼ���	���� �̺�Ʈ
#ifdef SERV_EVENT_JACKPOT
		kPacketToGB.m_iUserUID	= GetUID();
#endif SERV_EVENT_JACKPOT
		//}}
		SendToGlobalServer( EGB_UNIT_DISCONNECT_NOT, kPacketToGB );
#endif SERV_BATTLE_FIELD_SYSTEM
		//}}
	}

	//�����ý��ۿ� ����� �Ǿ��ִٸ� ������ �������ش�.
	m_kUserTutorialInfo.DisconnectUpdateUnitInfo( GetThisPtr<KGSUser>() );

	if( m_kUserTutorialInfo.GetTutorUnitType() == KUserTutorialInfo::TUT_STUDENT  &&  SiKTutorialManager()->IsExistbylist( GetCharUID() ) == true )
	{
		if( SiKTutorialManager()->DeleteUnit( GetCharUID() ) == false )
		{
			START_LOG( cerr, L"�缼�ý����� �������� ����� ����.!" )
#ifdef SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetCharUID() )
#else
				<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetLevel() )
				<< END_LOG;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
#endif SERV_DESELECT_UNIT_CODE_REFACTORING
//////////////////////////////////////////////////////////////////////////
//}}
//#endif SERV_DESELECT_UNIT_REFAC
//}}

//{{ 2011. 10. 14	������	������ ��� DB ������Ʈ ����
#ifdef SERV_USE_ITEM_DB_UPDATE_FIX
void KGSUser::OnUseItemInInventory( IN const UidType iItemUID
								   //{{ 2012. 12. 24	�ڼ���	�� ���� ���� ��� �߰�
//#ifdef SERV_PET_AUTO_LOOTING
								   , IN const __int64 iTempCode
//#endif SERV_PET_AUTO_LOOTING
								   //}}
								   , IN const bool bNotDeleteItem /*= false*/
								   , IN const int iWarpPointMapID /*= 0*/
								   )
{
	const int iUsedItemID = m_kInventory.GetItemID( iItemUID );

	//////////////////////////////////////////////////////////////////////////
	// �κ��丮���� ������ ���!
	KEGS_USE_ITEM_IN_INVENTORY_ACK kAck;
	KDBE_USE_ITEM_IN_INVENTORY_REQ kPacketToDB;

	if( m_kInventory.UseItemInInventory( iItemUID, kPacketToDB.m_vecUpdatedInventorySlot, bNotDeleteItem ) == false )
	{
		kAck.m_iOK = NetError::GetLastError();
		SendPacket( EGS_USE_ITEM_IN_INVENTORY_ACK, kAck );
		return;
	}

	//{{ 2012. 10. 31	�ڼ���	�ں� �ͽ������� Ƽ�� �߰�
#ifdef SERV_ADD_COBO_EXPRESS_TICKET
	if( ( iUsedItemID == CXSLItem::SI_USE_FREE_BY_FIELD ) ||
		( iUsedItemID == CXSLItem::SI_USE_COBO_EXPRESS_TICKET )
		)
	{
		if( ( GetMapID() == iWarpPointMapID )
			|| ( SiCXSLMapData()->CheckCOBOExpressTicketMapID( GetMapID() ) == 0 )
			|| ( SiCXSLMapData()->CheckCOBOExpressTicketMapID( iWarpPointMapID ) == 0 )
			)
		{
			KEGS_USE_ITEM_IN_INVENTORY_ACK kPacket;
			kPacket.m_iOK = NetError::ERR_UNKNOWN;
			SendPacket( EGS_USE_ITEM_IN_INVENTORY_ACK, kPacket );
			return;
		}
		else if( iUsedItemID == CXSLItem::SI_USE_COBO_EXPRESS_TICKET )
		{
			int iCOBOExpressTicketCost = SiCXSLMapData()->ComputeCOBOExpressTicketCost( GetMapID(), iWarpPointMapID, GetLevel() );
			if( GetED() < iCOBOExpressTicketCost )
			{
				KEGS_USE_ITEM_IN_INVENTORY_ACK kPacket;
				kPacket.m_iOK = NetError::ERR_USE_ITEM_IN_INVENTORY_08;
				SendPacket( EGS_USE_ITEM_IN_INVENTORY_ACK, kPacket );
				return;
			}

			//{{ 2012. 10. 16	������		ED&������ ����͸� �ý���
#ifdef SERV_ED_AND_ITEM_MORNITORING_SYSTEM
			DecreaseED( iCOBOExpressTicketCost, KUserEDManager::ER_OUT_ED_USE_COBO_EXPRESS_TICKET );
#else
//#ifdef SERV_SEPARATE_DUNGEON_FIELD_ED_LOG// �۾���¥: 2013-06-17	// �ڼ���
			const int iBeforeED = m_iED;
//#endif // SERV_SEPARATE_DUNGEON_FIELD_ED_LOG

			DecreaseED( iCOBOExpressTicketCost );

#ifdef SERV_SEPARATE_DUNGEON_FIELD_ED_LOG// �۾���¥: 2013-05-08	// �ڼ���	// �ؿ��� ���� �߰�
//#ifdef SERV_SEPARATE_DUNGEON_FIELD_ED_LOG// �۾���¥: 2013-06-17	// �ڼ���
			m_kUserAbuserManager.CheckEDAbuser( GetThisPtr<KGSUser>(), KAbuserLogManager::ELS_COBO_EXPRESS_TICKET_COST, iBeforeED );
//#endif // SERV_SEPARATE_DUNGEON_FIELD_ED_LOG

			// ED ��뷮 ���
			KStatisticsKey kKey;
			kKey.m_vecIntKey.push_back( 0 );
			KSIManager.IncreaseCount( KStatistics::SI_ED, kKey, KStatistics::eSIColDB_ED_MCoboExpressTicketED, iCOBOExpressTicketCost );

			// ���� ���
#ifdef SERV_USER_STATISTICS_RENEWAL
			m_kUserStatistics.IncreaseCount( KUserStatistics::USTable_EDData, 0, KUserStatistics::US_ED_MCoboExpressTicketED, iCOBOExpressTicketCost );
#else //SERV_USER_STATISTICS_RENEWAL
			m_kUserStatistics.IncreaseCount( KUserStatistics::USTable_EtcData, 0, KUserStatistics::US_Etc_MCoboExpressTicketED, iCOBOExpressTicketCost );
#endif //SERV_USER_STATISTICS_RENEWAL
#endif // SERV_SEPARATE_DUNGEON_FIELD_ED_LOG
#endif SERV_ED_AND_ITEM_MORNITORING_SYSTEM
			//}}
		}
	}
	kPacketToDB.m_iED = GetED();
#endif SERV_ADD_COBO_EXPRESS_TICKET
	//}}

	//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
	std::vector<KBuffInfo> vecActiveBuff;
	std::vector<int> vecDeactiveBuff;

	// ������ ��� ����
	CTime tCurr = CTime::GetCurrentTime();
	ActivateItemBuff( iUsedItemID, vecActiveBuff, vecDeactiveBuff, tCurr );

	if( vecActiveBuff.empty() == false || vecDeactiveBuff.empty() == false )
	{
		// �α��� ���� ���� �Ŵ����� ������Ʈ
		KELG_UPDATE_USER_UNIT_BUFF_INFO_REQ kReq;
		kReq.m_iUnitUID = GetCharUID();
		kReq.m_vecActivateBuff = vecActiveBuff;
		kReq.m_vecDeActivateBuff = vecDeactiveBuff;
		SendToLoginServer( ELG_UPDATE_USER_UNIT_BUFF_INFO_REQ, kReq );
	}
#endif SERV_SERVER_BUFF_SYSTEM
	//}
	
	kPacketToDB.m_iUsedItemID = iUsedItemID;
#ifdef SERV_GOLD_TICKET
	kPacketToDB.m_iItemUID = iItemUID;
#endif //SERV_GOLD_TICKET
	kPacketToDB.m_iWarpPointMapID = iWarpPointMapID;
	kPacketToDB.m_iUnitUID = GetCharUID();
	//{{ 2012. 12. 24	�ڼ���	�� ���� ���� ��� �߰�
//#ifdef SERV_PET_AUTO_LOOTING
	kPacketToDB.m_iTempCode	= iTempCode;
//#endif SERV_PET_AUTO_LOOTING
	//}}
	m_kInventory.FlushQuantityChange( kPacketToDB.m_kItemQuantityUpdate.m_mapQuantityChange );
	m_kInventory.FlushDeletedItem( kPacketToDB.m_kItemQuantityUpdate.m_vecDeleted );
	SendToGameDB( DBE_USE_ITEM_IN_INVENTORY_REQ, kPacketToDB );
}
#endif SERV_USE_ITEM_DB_UPDATE_FIX
//}}

#ifdef SERV_DUNGEON_CLEAR_PAYMENT_ITEM_EVENT// �۾���¥: 2013-07-14	// �ڼ���
bool KGSUser::CheckCharLevelUp( const bool bHackingCheck /*= true */)
#else
bool KGSUser::CheckCharLevelUp()
#endif // SERV_DUNGEON_CLEAR_PAYMENT_ITEM_EVENT
{
	const u_char ucNowLevel = GetLevel();
	
	m_ucLevel = SiKExpTable()->CheckLevelUp( ucNowLevel, m_kEXP.GetExp() );	

	if( ucNowLevel < m_ucLevel )
	{
		//{{ 2012. 12. 14  ���� �̼� �ý��� ( �������� ����Ʈ ) - ��μ�
#ifdef SERV_ACCOUNT_MISSION_SYSTEM
		m_kUserAccountQuestManager.Handler_CharaterLevelUp_Account( GetThisPtr<KGSUser>() );
#endif SERV_ACCOUNT_MISSION_SYSTEM
		//}}

		//�������� ��� ��Ƽ���� ������Ʈ.
		SendUpdatePartyUnitInfo();
		
		// ü��ID ��� ����
		if( !IsGuestUser() )
		{
#ifdef SERV_RECRUIT_EVENT_BASE
			SendUpdateRecommendUserInfo( KRecommendUserInfo::RUIT_LEVEL_UP );

			std::vector< KRecommendUserInfo > vecRecruiterUnitList;
			m_kUserRecommendManager.GetRecruiterUnitList( vecRecruiterUnitList );

			if( vecRecruiterUnitList.empty() == false )
			{
				int iRewardID = SiKRecommendUserTable()->GetRecommendUserReward( GetLevel() );
				if( iRewardID != 0 )
				{
					// DB�� ���� ������ ������.
					KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
					kPacketToDB.m_iFromUnitUID	= GetCharUID();
					kPacketToDB.m_iToUnitUID	= vecRecruiterUnitList[0].m_iUnitUID; // ��õ�� UnitUID
					kPacketToDB.m_iRewardType	= KPostItemInfo::LT_RECOMMEND; // ���� Ÿ��
					kPacketToDB.m_iRewardID		= iRewardID;

					CStringW cwstrMessage;
					cwstrMessage.Format( L"%s;%d", GetCharName().c_str(), GetLevel() );
					kPacketToDB.m_wstrMessage = static_cast<LPCTSTR>(cwstrMessage);

					SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
				}
			}
			else
#endif SERV_RECRUIT_EVENT_BASE
			// ��õ�� ���� �κ�
			if( m_kUserRecommendManager.GetRecommendedUnitUID() > 0 )
			{
				//{{ 2010. 01. 11  ������	��õ�θ���Ʈ
#ifdef SERV_RECOMMEND_LIST
				SendUpdateRecommendUserInfo( KRecommendUserInfo::RUIT_LEVEL_UP );
#endif SERV_RECOMMEND_LIST
				//}}

				int iRewardID = SiKRecommendUserTable()->GetRecommendUserReward( GetLevel() );
				if( iRewardID != 0 )
				{
					// DB�� ���� ������ ������.
					KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
					kPacketToDB.m_iFromUnitUID	= GetCharUID();
					kPacketToDB.m_iToUnitUID	= m_kUserRecommendManager.GetRecommendedUnitUID(); // ��õ�� UnitUID
					kPacketToDB.m_iRewardType	= KPostItemInfo::LT_RECOMMEND; // ���� Ÿ��
					kPacketToDB.m_iRewardID		= iRewardID;

					CStringW cwstrMessage;
					cwstrMessage.Format( L"%s;%d", GetCharName().c_str(), GetLevel() );
					kPacketToDB.m_wstrMessage = static_cast<LPCTSTR>(cwstrMessage);

					SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
				}

				//{{ 2013. 03. 21	 ��õ�� �ý��� ���� - ��μ�
#ifdef SERV_RECOMMEND_LIST_EVENT
				// ���ҵ� ���谡�� ���� 10�� 1��, 20�� 2��, 30�� 3��
				int iRewardEventID = SiKRecommendUserTable()->GetRecommendUserEventReward( GetLevel() );
				if( iRewardEventID != 0 )
				{
					// DB�� ���� ������ ������.
					KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
					kPacketToDB.m_iFromUnitUID	= GetCharUID();
					kPacketToDB.m_iToUnitUID	= m_kUserRecommendManager.GetRecommendedUnitUID(); // ��õ�� UnitUID
					kPacketToDB.m_iRewardType	= KPostItemInfo::LT_RECOMMEND; // ���� Ÿ��
					kPacketToDB.m_iRewardID		= iRewardEventID;

					CStringW cwstrMessage;
					cwstrMessage.Format( L"%s;%d", GetCharName().c_str(), GetLevel() );
					kPacketToDB.m_wstrMessage = static_cast<LPCTSTR>(cwstrMessage);

					SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
				}
#endif SERV_RECOMMEND_LIST_EVENT
				//}

			}

			//���ڸ���Ʈ�� �ִٸ� ����������Ʈ.
			if( m_kUserTutorialInfo.GetTutorUnitType() == KUserTutorialInfo::TUT_STUDENT &&
				m_kUserTutorialInfo.IsExistbyTutorData() == false )
			{
				if( SiKTutorialManager()->IsExistbylist( GetCharUID() ) == true )
				{
					SiKTutorialManager()->SetLevelData( GetCharUID(), GetLevel() );

					//������ ������ ������ �Ǿ����� ����Ʈ���� �������ش�. 
					if( GetLevel() >= KTutorialManager::UNIT_LEVEL_STUDENT_MAX  &&
						ucNowLevel < KTutorialManager::UNIT_LEVEL_STUDENT_MAX )
					{
						SiKTutorialManager()->DeleteUnit( GetCharUID() );
					}
				}
			}

			//080408.hoons. ���� ���� �κ�.
			m_kUserTutorialInfo.ProcCharLevelUP( ucNowLevel, GetThisPtr<KGSUser>() );
		}

		//{{ 2010. 03. 22  ������	����� ��Ʈ
#ifdef SERV_SKILL_NOTE
		// ���� ����� ��Ʈ�� �����ϰ� �ִٸ�?
		if( m_kSkillTree.IsHaveSkillNote() == true )
		{
			char cSkillNoteMaxPageNum = 0;
			if( m_kSkillTree.GetExpandSkillNotePage( GetLevel(), cSkillNoteMaxPageNum ) == true )
			{
				// ���� ���� ���������� ���纸�� ũ�ٸ� DB�� Ȯ���Ϸ� ����!
				if( cSkillNoteMaxPageNum > m_kSkillTree.GetSkillNoteMaxPageNum() )
				{
					KDBE_EXPAND_SKILL_NOTE_PAGE_REQ kPacketToDB;
					kPacketToDB.m_iUnitUID = GetCharUID();
					kPacketToDB.m_cExpandedMaxPageNum = cSkillNoteMaxPageNum;
					kPacketToDB.m_cRollBackMaxPageNum = m_kSkillTree.GetSkillNoteMaxPageNum();
					SendToGameDB( DBE_EXPAND_SKILL_NOTE_PAGE_REQ, kPacketToDB );
				}
			}
			else
			{
				START_LOG( cerr, L"����� ��Ʈ ������ ������ ���� ��� ����!" )
					<< BUILD_LOG( GetCharUID() )
					<< BUILD_LOGc( GetLevel() )
					<< END_LOG;
			}
		}
#endif SERV_SKILL_NOTE
		//}}

#ifdef SERV_LEVEL_UP_EVENT// �۾���¥: 2013-07-12	// �ڼ���
		{
			std::vector<int> vecRewardID;
			SiKGameEventManager()->GetLevelUpEvent( ucNowLevel
#ifdef SERV_LEVEL_UP_EVENT_CRITERION_DATE// �۾���¥: 2013-06-26	// �ڼ���
				, GetCriterionDateEventInfo()
				, GetAccountRegDate()
#endif // SERV_LEVEL_UP_EVENT_CRITERION_DATE
				, vecRewardID
				);

			BOOST_TEST_FOREACH( int, iRewardID, vecRewardID )
			{
				// DB�� ���� ������ ������.
				KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
				kPacketToDB.m_iFromUnitUID = GetCharUID();
				kPacketToDB.m_iToUnitUID   = GetCharUID();
				kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT; // ���� Ÿ��
				kPacketToDB.m_iRewardID	   = iRewardID;
				SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
			}
		}
#endif // SERV_LEVEL_UP_EVENT

#ifdef SERV_PRESENT_SKILL_INIT_ITEM// �۾���¥: 2013-06-25	// �ڼ���
		if( m_ucLevel == SiKGameSysVal()->GetLimitsLevel() )
		{
			// DB�� ���� ������ ������.
			KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
			kPacketToDB.m_iFromUnitUID = GetCharUID();
			kPacketToDB.m_iToUnitUID   = GetCharUID();
			kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT; // ���� Ÿ��
#ifdef SERV_PRESENT_SKILL_INIT_ITEM_INT
			kPacketToDB.m_iRewardID	   = _CONST_PRESENT_SKILL_INIT_ITEM_INT_::iReachLimitLevel;
#else //SERV_PRESENT_SKILL_INIT_ITEM_INT
			kPacketToDB.m_iRewardID	   = 10553;
#endif //SERV_PRESENT_SKILL_INIT_ITEM_INT
			SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
		}
#endif // SERV_PRESENT_SKILL_INIT_ITEM

		//{{ 2011. 08. 09  ��μ� ä�θ� �̺�Ʈ - ���Ϸ��� ���� 25�� �޼� �̺�Ʈ
#ifdef SERV_CHANNELING_TOONILAND_LEVEL_UP_EVENT
		if( GetChannelCode() == KNexonAccountInfo::CE_TOONILAND_ACCOUNT )  // ���Ϸ��� �����϶�
		{
			if( ucNowLevel < 25  &&  GetLevel() >= 25 )
			{
				// DB�� ���� ������ ������.
				KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
				kPacketToDB.m_iFromUnitUID = GetCharUID();
				kPacketToDB.m_iToUnitUID   = GetCharUID();
				kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT; // ���� Ÿ��
				kPacketToDB.m_iRewardID	   = 10219; // TooniLand Īȣ ITEM
				SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
			}
		}
#endif SERV_CHANNELING_TOONILAND_LEVEL_UP_EVENT
		//}}		

		//{{ 2012. 12. 19  �ƶ� ĳ���� ������ �̺�Ʈ(15, 30, 40 �����޼� ��) - ��μ�
#ifdef SERV_ARA_LEVEL_UP_EVENT
		{
			CTime tCurrentDate = CTime::GetCurrentTime();
			if( ( CTime( 2013, 1, 17, 7, 0, 0 ) <= tCurrentDate ) &&
				( tCurrentDate < CTime( 2013, 2, 14, 7, 0, 0 ) )
				)
			{
				if( GetUnitType() == CXSLUnit::UT_ARA )
				{
					if( ucNowLevel < 15  &&  GetLevel() >= 15 )
					{
						// DB�� ���� ������ ������.
						KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
						kPacketToDB.m_iFromUnitUID = GetCharUID();
						kPacketToDB.m_iToUnitUID   = GetCharUID();
						kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT; // ���� Ÿ��
						kPacketToDB.m_iRewardID	   = 10451; // ��ų ���� ü���� �޴� 30�ϱ� ť�� <-- RewardTable.lua�� ��ϵ� id��
						SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
					}

					if( ucNowLevel < 30  &&  GetLevel() >= 30 )
					{
						// DB�� ���� ������ ������.
						KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
						kPacketToDB.m_iFromUnitUID = GetCharUID();
						kPacketToDB.m_iToUnitUID   = GetCharUID();
						kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT; // ���� Ÿ��
						kPacketToDB.m_iRewardID	   = 10452; // �׳�ý��� �ູ (10SP) 30�ϱ� ť�� <-- RewardTable.lua�� ��ϵ� id��
						SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
					}

					if( ucNowLevel < 40  &&  GetLevel() >= 40 )
					{
						// DB�� ���� ������ ������.
						KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
						kPacketToDB.m_iFromUnitUID = GetCharUID();
						kPacketToDB.m_iToUnitUID   = GetCharUID();
						kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT; // ���� Ÿ��
						kPacketToDB.m_iRewardID	   = 10453; // ������ �˾� 10�� ť�� <-- RewardTable.lua�� ��ϵ� id��
						SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
					}
				}
			}
		}
#endif SERV_ARA_LEVEL_UP_EVENT
		//}}

		//{{ 2013. 01. 25  ������ ���� �̺�Ʈ �۾� - ��μ�
#ifdef SERV_EVENT_LEVEL_UP
		{
			CTime tCurrentDate = CTime::GetCurrentTime();
			if( ( CTime( 2013, 1, 31, 7, 0, 0 ) <= tCurrentDate ) &&
				( tCurrentDate < CTime( 2013, 2, 14, 7, 0, 0 ) )
				)
			{
				if( ucNowLevel < 65  &&  GetLevel() >= 65 )
				{
					// DB�� ���� ������ ������.
					KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
					kPacketToDB.m_iFromUnitUID = GetCharUID();
					kPacketToDB.m_iToUnitUID   = GetCharUID();
					kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT; // ���� Ÿ��
					kPacketToDB.m_iRewardID	   = 10472; // ������ ���̴� �극������ (1����)  <-- RewardTable.lua�� ��ϵ� id��
					SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
				}
			}
		}
#endif SERV_EVENT_LEVEL_UP
		//}
#ifdef SERV_ARA_NEW_FIRST_CLASS_EVENT
		IF_EVENT_ENABLED( CEI_ARA_NEW_FIRST_CLASS_EVENT )
		{
			if(	GetUnitClass() == CXSLUnit::UC_ARA_LITTLE_DEVIL )
			{
				if( ucNowLevel < 20 && GetLevel() >= 20 )
				{
					// DB�� ���� ������ ������.
					KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
					kPacketToDB.m_iFromUnitUID = GetCharUID();
					kPacketToDB.m_iToUnitUID   = GetCharUID();
					kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT; // ���� Ÿ��
					kPacketToDB.m_iRewardID	   = _CONST_ARA_NEW_FIRST_CLASS_EVENT_REWARD_ID::iLevelUpReward1;
					SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
				}

				if( ucNowLevel < 25 && GetLevel() >= 25 )
				{
					// DB�� ���� ������ ������.
					KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
					kPacketToDB.m_iFromUnitUID = GetCharUID();
					kPacketToDB.m_iToUnitUID   = GetCharUID();
					kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT; // ���� Ÿ��
					kPacketToDB.m_iRewardID	   = _CONST_ARA_NEW_FIRST_CLASS_EVENT_REWARD_ID::iLevelUpReward2;
					SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
				}

				if( ucNowLevel < 30 && GetLevel() >= 30 )
				{
					// DB�� ���� ������ ������.
					KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
					kPacketToDB.m_iFromUnitUID = GetCharUID();
					kPacketToDB.m_iToUnitUID   = GetCharUID();
					kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT; // ���� Ÿ��
					kPacketToDB.m_iRewardID	   = _CONST_ARA_NEW_FIRST_CLASS_EVENT_REWARD_ID::iLevelUpReward3;
					SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
				}

				if( ucNowLevel < 35 && GetLevel() >= 35 )
				{
					// DB�� ���� ������ ������.
					KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
					kPacketToDB.m_iFromUnitUID = GetCharUID();
					kPacketToDB.m_iToUnitUID   = GetCharUID();
					kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT; // ���� Ÿ��
					kPacketToDB.m_iRewardID	   = _CONST_ARA_NEW_FIRST_CLASS_EVENT_REWARD_ID::iLevelUpReward4;
					SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
				}
			}
		}
#endif SERV_ARA_NEW_FIRST_CLASS_EVENT

#ifdef SERV_EVENT_LEVEL_UP_ARA
		IF_EVENT_ENABLED( CEI_EVENT_LEVEL_UP_ARA )
		{
			if( GetUnitType() == CXSLUnit::UT_ARA )
			{
				if( ucNowLevel < 3  &&  GetLevel() >= 3 )
				{
					// DB�� ���� ������ ������.
					KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
					kPacketToDB.m_iFromUnitUID = GetCharUID();
					kPacketToDB.m_iToUnitUID   = GetCharUID();
					kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT; // ���� Ÿ��
					kPacketToDB.m_iRewardID	   = 1545; // ���� ����
					SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
				}

				if( ucNowLevel < 7  &&  GetLevel() >= 7 )
				{
					// DB�� ���� ������ ������.
					KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
					kPacketToDB.m_iFromUnitUID = GetCharUID();
					kPacketToDB.m_iToUnitUID   = GetCharUID();
					kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT; // ���� Ÿ��
					kPacketToDB.m_iRewardID	   = 1546; // Ư�� �׼����� 3�ϱ� ť��
					SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
				}

				if( ucNowLevel < 11  &&  GetLevel() >= 11 )
				{
					// DB�� ���� ������ ������.
					KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
					kPacketToDB.m_iFromUnitUID = GetCharUID();
					kPacketToDB.m_iToUnitUID   = GetCharUID();
					kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT; // ���� Ÿ��
					kPacketToDB.m_iRewardID	   = 1547; // ���� ���� ���� 10��
					SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
				}

				if( ucNowLevel < 15  &&  GetLevel() >= 15 )
				{
					// DB�� ���� ������ ������.
					KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
					kPacketToDB.m_iFromUnitUID = GetCharUID();
					kPacketToDB.m_iToUnitUID   = GetCharUID();
					kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT; // ���� Ÿ��
					kPacketToDB.m_iRewardID	   = 1548; // �׳�ý� �ູ(10SP) 15�ϱ�
					SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
				}
			}
		}
#endif SERV_EVENT_LEVEL_UP_ARA

#ifdef	SERV_CHILDREN_DAY_LEVEL_UP_EVENT // ���볯¥: 2013-05-02
		{
			std::wstring wstrAccountRegDate = GetAccountRegDate();
			CTime tRegDate;
			if( KncUtil::ConvertStringToCTime( wstrAccountRegDate, tRegDate ) == true )
			{
#ifdef	SERV_CHILDREN_DAY_LEVEL_UP_EVENT_TEST // ���볯¥: 2013-05-02
				if( CTime( 2013, 4, 30, 0, 0, 0 ) < tRegDate  )	// �̺�Ʈ �Ⱓ ���� ������ ����
				{
					CTime tCurrentDate = CTime::GetCurrentTime();
					if( ( CTime( 2013, 4, 30, 0, 0, 0 ) <= tCurrentDate ) &&
						( tCurrentDate < CTime( 2013, 5, 16, 7, 0, 0 ) )
						)
#else
				if( CTime( 2013, 5, 2, 7, 0, 0 ) < tRegDate  )	// �̺�Ʈ �Ⱓ ���� ������ ����
				{
					CTime tCurrentDate = CTime::GetCurrentTime();
					if( ( CTime( 2013, 5, 2, 7, 0, 0 ) <= tCurrentDate ) &&
						( tCurrentDate < CTime( 2013, 5, 16, 7, 0, 0 ) )
						)
#endif	//SERV_CHILDREN_DAY_LEVEL_UP_EVENT_TEST 
					{
						if( ucNowLevel < 10  &&  GetLevel() >= 10 )
						{
							// DB�� ���� ������ ������.
							KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
							kPacketToDB.m_iFromUnitUID = GetCharUID();
							kPacketToDB.m_iToUnitUID   = GetCharUID();
							kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT; // ���� Ÿ��
							kPacketToDB.m_iRewardID	   = 10530; // �ູ�� �޴� 1�ϱ� ť�� <-- RewardTable.lua�� ��ϵ� id��
							SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
						}

						if( ucNowLevel < 20  &&  GetLevel() >= 20 )
						{
							// DB�� ���� ������ ������.
							KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
							kPacketToDB.m_iFromUnitUID = GetCharUID();
							kPacketToDB.m_iToUnitUID   = GetCharUID();
							kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT; // ���� Ÿ��
							kPacketToDB.m_iRewardID	   = 10531; // �׳�ý��� �ູ(10sp) 30�ϱ� 1�� <-- RewardTable.lua�� ��ϵ� id��
							SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
						}

						if( ucNowLevel < 30  &&  GetLevel() >= 30 )
						{
							// DB�� ���� ������ ������.
							KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
							kPacketToDB.m_iFromUnitUID = GetCharUID();
							kPacketToDB.m_iToUnitUID   = GetCharUID();
							kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT; // ���� Ÿ��
							kPacketToDB.m_iRewardID	   = 10532; // ���ҵ� �ǳ� ���� ���� �׼����� 1���� <-- RewardTable.lua�� ��ϵ� id��
							SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
						}
					}
				}
			}
		}
#endif	// SERV_CHILDREN_DAY_LEVEL_UP_EVENT

		//{{ 2009. 5. 18  ������	10���� �Ǿ����� ����
		//if( ucNowLevel < 10  &&  GetLevel() >= 10 )
		//{
		//	// DB�� ���� ������ ������.
		//	KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
		//	kPacketToDB.m_iFromUnitUID = GetCharUID();
		//	kPacketToDB.m_iToUnitUID   = GetCharUID();
		//	kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT; // ���� Ÿ��
		//	kPacketToDB.m_iRewardID	   = 136; // �ں� Ư�� ���� ť�� <-- RewardTable.lua�� ��ϵ� id��
		//	SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
		//}
		//}}

		//////////////////////////////////////////////////////////////////////////
		//{{ 2009. 12. 22  ������	�ż����̺�Ʈ  "2009-12-23 ~ 2010-01-27"
		//if( KBaseServer::GetKObj()->GetServerGroupID() == SEnum::SGI_GAIA )
		//{
		//	if( ucNowLevel < 10  &&  GetLevel() >= 10 )
		//	{
		//		// DB�� ���� ������ ������.
		//		KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
		//		kPacketToDB.m_iFromUnitUID = GetCharUID();
		//		kPacketToDB.m_iToUnitUID   = GetCharUID();
		//		kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT;
		//		kPacketToDB.m_iRewardID	   = 189; // �ں� ������ ���� ��ǰ(10����)
		//		SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
		//	}

		//	if( ucNowLevel < 20  &&  GetLevel() >= 20 )
		//	{
		//		// DB�� ���� ������ ������.
		//		KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
		//		kPacketToDB.m_iFromUnitUID = GetCharUID();
		//		kPacketToDB.m_iToUnitUID   = GetCharUID();
		//		kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT;
		//		kPacketToDB.m_iRewardID	   = 190; // �ں� ������ ���� ��ǰ(20����), ��� â�� �㰡��
		//		SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
		//	}

		//	if( ucNowLevel < 30  &&  GetLevel() >= 30 )
		//	{
		//		// DB�� ���� ������ ������.
		//		KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
		//		kPacketToDB.m_iFromUnitUID = GetCharUID();
		//		kPacketToDB.m_iToUnitUID   = GetCharUID();
		//		kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT;
		//		kPacketToDB.m_iRewardID	   = 191; // �ں� ������ ���� ��ǰ(30����)
		//		SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
		//	}

		//	if( ucNowLevel < 40  &&  GetLevel() >= 40 )
		//	{
		//		// DB�� ���� ������ ������.
		//		KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
		//		kPacketToDB.m_iFromUnitUID = GetCharUID();
		//		kPacketToDB.m_iToUnitUID   = GetCharUID();
		//		kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT;
		//		kPacketToDB.m_iRewardID	   = 192; // �ں� ������ ���� ��ǰ(40����)
		//		SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
		//	}

		//	if( ucNowLevel < 50  &&  GetLevel() >= 50 )
		//	{
		//		// DB�� ���� ������ ������.
		//		KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
		//		kPacketToDB.m_iFromUnitUID = GetCharUID();
		//		kPacketToDB.m_iToUnitUID   = GetCharUID();
		//		kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT;
		//		kPacketToDB.m_iRewardID	   = 193; // �ں� ������ ���� ��ǰ(50����)
		//		SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
		//	}
		//}
		//}}
		//////////////////////////////////////////////////////////////////////////

		//{{ 2011. 08. 29	������	ĳ���� ������ �̺�Ʈ
#ifdef SERV_CHAR_LEVEL_UP_EVENT
//		bool bIsLevelUpEvent = false;
		if( GetChannelCode() == KNexonAccountInfo::CE_TOONILAND_ACCOUNT )  // ���Ϸ��� �����϶�
		{
			std::vector< int >	vecLevelUpRewardID;
			/* bIsLevelUpEvent = */GetLevelUpRewardItem( ucNowLevel, GetLevel(), vecLevelUpRewardID );

			int iRewardID = 0;
			std::vector< int >::iterator vit = vecLevelUpRewardID.begin();
			for( ; vit != vecLevelUpRewardID.end() ; ++vit )
			{
				if( *vit == 10227 ) // ������ �ֹ��� 1�� (1��) 
				{
					if( m_kUserLetterBox.IsExistRewardInLetter( 10227 ) == true )  // ������ ������ �ߺ� �������� �ʴ´�.
					{
						continue;
					}
				}

				// DB�� ���� ������ ������.
				KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
				kPacketToDB.m_iFromUnitUID = GetCharUID();
				kPacketToDB.m_iToUnitUID   = GetCharUID();
				kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT;
				kPacketToDB.m_iRewardID	   = *vit;
				SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
			}
		}

// 		bool bIsLevelUpEvent = false;
// 		if( ucNowLevel < 21  &&  GetLevel() >= 21 )
// 		{
// 			bIsLevelUpEvent = true;
// 		}
// 		else if( ucNowLevel < 25  &&  GetLevel() >= 25 )
// 		{
// 			bIsLevelUpEvent = true;
// 		}
// 		else if( ucNowLevel < 29  &&  GetLevel() >= 29 )
// 		{
// 			bIsLevelUpEvent = true;
// 		}
// 		else if( ucNowLevel < 33  &&  GetLevel() >= 33 )
// 		{
// 			bIsLevelUpEvent = true;
// 		}
// 		else if( ucNowLevel < 37  &&  GetLevel() >= 37 )
// 		{
// 			bIsLevelUpEvent = true;
// 		}
// 		else if( ucNowLevel < 41  &&  GetLevel() >= 41 )
// 		{
// 			bIsLevelUpEvent = true;
// 		}
// 		else if( ucNowLevel < 45  &&  GetLevel() >= 45 )
// 		{
// 			bIsLevelUpEvent = true;
// 		}
// 		else if( ucNowLevel < 49  &&  GetLevel() >= 49 )
// 		{
// 			bIsLevelUpEvent = true;
// 		}
// 
// 		if( bIsLevelUpEvent )
// 		{
// 			// �̹� ������ ������ �ֹ����� �ִٸ� ������ �������� �ʴ´�.
// 			if( m_kUserLetterBox.IsExistRewardInLetter( 10227 ) == false )
// 			{
// 				// DB�� ���� ������ ������.
// 				KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
// 				kPacketToDB.m_iFromUnitUID = GetCharUID();
// 				kPacketToDB.m_iToUnitUID   = GetCharUID();
// 				kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT;
// 				kPacketToDB.m_iRewardID	   = 10227; // ������ �ֹ��� 1�� (1��)
// 				SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
// 			}
// 		}
#endif SERV_CHAR_LEVEL_UP_EVENT
		//}}
#ifdef	SERV_LOCAL_RANKING_SYSTEM // ���볯¥: 2013-03-28
		m_kUserLocalRankingInfo.SetCharacterLevel( GetLevel() );
		// Login Server�� ���� ���� ���� �˸��� ������.
		IncreasedLocalRankingPoint( 0, 0, true );
#endif	// SERV_LOCAL_RANKING_SYSTEM

		//{{ 2009. 6. 29  ������	���� �޼� ����
		if( GetLevel() == SiKGameSysVal()->GetLimitsLevel()  &&
			GetAuthLevel() < SEnum::UAL_GM )
		{
			KEGS_NOTIFY_MSG_NOT	kPacket;
			kPacket.m_cNotifyType = KEGS_NOTIFY_MSG_NOT::NT_MAX_LEVEL_UP;
			kPacket.m_Count = 1;

			CStringW cwstrMessage;
			cwstrMessage.Format( L"%s;%d", GetCharName().c_str(), GetLevel() );
			kPacket.m_wstrMSG = static_cast<LPCTSTR>(cwstrMessage);
			
			SendToCnServer( ECN_NOTIFY_MSG_NOT, kPacket );

			//{{ 2011. 01. 17	������	ĳ���� ī��Ʈ ����
#ifdef SERV_CHAR_LOG
			WriteCharacterLogToDB( KELOG_USER_CHARACTER_LOG_NOT::LT_FULL_LEVEL );
#endif SERV_CHAR_LOG
			//}}
		}
		//}}

#ifdef SERV_SANDER_UPDATE_EVENT
		if( GetLevel() == 64 )
		{
			// DB�� ���� ������ ������.
			KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
			kPacketToDB.m_iFromUnitUID = GetCharUID();
			kPacketToDB.m_iToUnitUID   = GetCharUID();
			kPacketToDB.m_iRewardType  = KPostItemInfo::LT_EVENT;
			kPacketToDB.m_iRewardID	   = 1540; // ���� ���� ������ ť�� rewardID
			SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
		}
#endif //SERV_SANDER_UPDATE_EVENT

		KStatTable::KUnitStatInfo	sUnitStat;
		if( SiKStatTable()->GetUnitStat( GetUnitClass(), GetLevel(), sUnitStat ) == true )
		{
			m_kStat.m_iBaseHP		= sUnitStat.m_uiHP;
			m_kStat.m_iAtkPhysic	= sUnitStat.m_usAtkPhysic;
			m_kStat.m_iAtkMagic		= sUnitStat.m_usAtkMagic;
			m_kStat.m_iDefPhysic	= sUnitStat.m_usDefPhysic;
			m_kStat.m_iDefMagic		= sUnitStat.m_usDefMagic;

			// ������ �ϸ� sp�� ���� ȹ���Ѵ�
			for( u_char ucLevel = ucNowLevel + 1; ucLevel <= GetLevel(); ++ucLevel )
			{
				int iSPInc = 0;
#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013 // ���볯¥: 2013-06-27
				if( SiCXSLSkillTree()->GetCalcLevelUpIncreaseSkillPoint( ucLevel, iSPInc ) == true )
#else	// SERV_UPGRADE_SKILL_SYSTEM_2013
/*				
				if( SiCXSLSkillTree()->GetIncSkillPoint( ucLevel, iSPInc ) == true )
*/				
#endif	// SERV_UPGRADE_SKILL_SYSTEM_2013
				{
					m_iSPoint += iSPInc;

#ifdef SERV_SKILL_POINT_CORRECTION
					if( GetAuthLevel() < SEnum::UAL_GM )
					{
						int iCorrectionSP = 0;
						if( CheckSkillPointCorrection( iCorrectionSP ) == true)
						{
							/* �� ������ ������ ���� �ʰ� DB ��ϸ� ��.
							if( iCorrectionSP > 0 )			// iCorrectionSP�� ����� ������ ���� SP���� ������ ����.
							{
								m_iSPoint += iCorrectionSP;	// ������ SP��ŭ �߰� ������.
							}
							else							// iCorrectionSP�� ������ ������ ���� SP���� ���� ����.
							{
								if( abs(iCorrectionSP) < iSPInc )			
								{
									m_iSPoint -= abs(iCorrectionSP);	// SP�� ���� �������� ���� SP ��������� ������ SP ����ŭ �����ؼ� ���� SP�� ����.
								}
								else
								{
									m_iSPoint -= iSPInc;				// SP�� ���� �������� ���� SP ��������� ũ�� �̹� ���������� SP�� �������� ����.
								}
							}
							*/

							int iUsedSPoint = 0;
							int iUsedCSPoint = 0;
							m_kSkillTree.CalcUsedSPointAndCSPoint( iUsedSPoint, iUsedCSPoint );

							KDBE_SKILL_POINT_CORRECTION_INFO_NOT kPacketToLogDB;
							kPacketToLogDB.m_iServerGroupID = KBaseServer::GetKObj()->GetServerGroupID();
							kPacketToLogDB.m_iUserUID = GetUID();
							kPacketToLogDB.m_iUnitUID = GetCharUID();
							kPacketToLogDB.m_iUnitClass = GetUnitClass();
							kPacketToLogDB.m_sLevel = GetLevel();
							kPacketToLogDB.m_iSPoint = m_iSPoint;
							kPacketToLogDB.m_iUsedSPoint = iUsedSPoint;
							kPacketToLogDB.m_iCSPoint = m_kSkillTree.GetCSPoint();
							kPacketToLogDB.m_iUsedCSPoint = iUsedCSPoint;
							kPacketToLogDB.m_iMaxCSPoint = m_kSkillTree.GetMaxCSPoint();
							kPacketToLogDB.m_iCorrectionSP = iCorrectionSP;
							SendToLogDB( DBE_SKILL_POINT_CORRECTION_INFO_NOT, kPacketToLogDB );
						}
					}
#endif //SERV_SKILL_POINT_CORRECTION
				}
				else
				{
					START_LOG_WITH_NAME( cerr )
						<< END_LOG;
				}
			}

            // �������� ���� �α׸� DB�� �����.
            KDBE_CHARACTER_LEVELUP_NOT kNot;
            kNot.m_nCharUID = GetCharUID();
            kNot.m_ucLevelBefore = ucNowLevel;
            kNot.m_ucLevelAfter = GetLevel();
            SendToLogDB( DBE_CHARACTER_LEVELUP_NOT, kNot );

			//{{ 2011. 01. 17	������	ĳ���� ī��Ʈ ����
#ifdef SERV_CHAR_LOG
			WriteCharacterLogToDB( KELOG_USER_CHARACTER_LOG_NOT::LT_LEVEL_UP );
#endif SERV_CHAR_LOG
			//}}

			//{{ 2011. 08. 29	������	ĳ���� ������ �̺�Ʈ
			//////////////////////////////////////////////////////////////////////////
#ifdef SERV_CHAR_LEVEL_UP_EVENT
			//////////////////////////////////////////////////////////////////////////
			// ������ �̺�Ʈ �ϴ� ���ȿ��� ������ üũ ����� �����մϴ�!
#else
#ifdef SERV_DUNGEON_CLEAR_PAYMENT_ITEM_EVENT// �۾���¥: 2013-07-14	// �ڼ���
			if( bHackingCheck == true )
			{
#endif // SERV_DUNGEON_CLEAR_PAYMENT_ITEM_EVENT
			//{{ 2010. 11. 17	������	������ ���� ���
#ifdef SERV_AUTO_HACK_CHECK_LEVEL			
			if( m_kUserAbuserManager.IsAutoHackLevelUpUser( GetLevel()
#ifdef SERV_AUTO_HACK_CHECK_LEVEL_FIX
				, GetCharUID()
#endif // SERV_AUTO_HACK_CHECK_LEVEL_FIX
				) == true )
			{
				if( GetAuthLevel() >= SEnum::UAL_GM )
				{
					START_LOG( cout, L"������ �ð� ������ 1�� �̸����� üũ�Ǿ����� ���������� ��ڱ� �̻��̹Ƿ� �ƹ��� ó���� ���� �ʽ��ϴ�!" )
#ifndef SERV_PRIVACY_AGREEMENT
						<< BUILD_LOG( GetName() )
#endif SERV_PRIVACY_AGREEMENT
						<< BUILD_LOG( GetUID() )
						<< BUILD_LOGc( GetAuthLevel() )
#ifndef SERV_PRIVACY_AGREEMENT
						<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
						<< BUILD_LOG( GetCharUID() );
				}
				else
				{
					START_LOG( cout, L"������ �ð� ������ 1�� �̸����� üũ�Ǿ� �ش� ������ ���� ���� ó����!" )
#ifndef SERV_PRIVACY_AGREEMENT
						<< BUILD_LOG( GetName() )
#endif SERV_PRIVACY_AGREEMENT
						<< BUILD_LOG( GetUID() )
						<< BUILD_LOGc( GetAuthLevel() )
#ifndef SERV_PRIVACY_AGREEMENT
						<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
						<< BUILD_LOG( GetCharUID() )
						<< BUILD_LOG( GetED() )
						<< BUILD_LOGc( GetLevel() )
						<< BUILD_LOG( GetEXP() )
						<< BUILD_LOG( GetStateIDString() )
						<< BUILD_LOG( GetRoomUID() )
						<< BUILD_LOG( CXSLRoom::GetRoomType( GetRoomUID() ) );

					if( SiKAbuserLogManager()->IsAbuserCheckLevelUp() == true )
					{
						START_LOG( cout, L"��ŷ ���� ��� �� ���� ���� ���� �մϴ�!" )
#ifndef SERV_PRIVACY_AGREEMENT
							<< BUILD_LOG( GetName() )
#endif SERV_PRIVACY_AGREEMENT
							<< BUILD_LOG( GetUID() )
							<< BUILD_LOGc( GetAuthLevel() )
#ifndef SERV_PRIVACY_AGREEMENT
							<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
							<< BUILD_LOG( GetCharUID() );

						// RejectedUser�� ����ϱ�!
						KELG_REG_REJECTED_USER_NOT kPacketNot;
						kPacketNot.m_iUserUID = GetUID();
						SendToLoginServer( ELG_REG_REJECTED_USER_NOT, kPacketNot );

						if( m_cHackingUserType == SEnum::HUT_NORMAL )
						{
							START_LOG( clog, L"��ŷ ���� �Ű�.!" )
								<< BUILD_LOG( GetUID() )
#ifndef SERV_PRIVACY_AGREEMENT
								<< BUILD_LOG( GetName() )
								<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
								<< BUILD_LOGc( m_cHackingUserType )
								;

								// �������� üũ!
								m_cHackingUserType = SEnum::HUT_DISAGREE_HACK_USER;

								// AccountDB���� �������!
								SendToAccountDB( DBE_REPORT_HACK_USER_NOT, GetUID() );
							}

							//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
							//{{ 2012. 03. 20	��μ�		Game Server ���� ���� ������ ��ŷ ���� üũ
#ifdef SERV_CHECK_KICK_USER_IN_GAMESERVER
							KEGS_SERVER_CHECK_HACK_USER_REQ kPacket;
							kPacket.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_CHECK_FAST_LEVEL_UP;
							SendPacket( EGS_SERVER_CHECK_HACK_USER_REQ, kPacket );
#endif SERV_CHECK_KICK_USER_IN_GAMESERVER
							//}}
#else
							//{{ 2012. 03. 20	��μ�		Game Server ���� ���� ������ ��ŷ ���� üũ
#ifdef SERV_CHECK_KICK_USER_IN_GAMESERVER
							KEGS_SERVER_CHECK_HACK_USER_NOT kPacket;
							kPacket.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_CHECK_FAST_LEVEL_UP;
							SendPacket( EGS_SERVER_CHECK_HACK_USER_NOT, kPacket );
#endif SERV_CHECK_KICK_USER_IN_GAMESERVER
							//}}
#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
							//}}

							SetDisconnectReason( KStatistics::eSIColDR_Client_Hacking );
							ReserveDestroy();
						}

						//KDBE_SEND_PHONE_MSG_NOT kPacketNot;
						//kPacketNot.m_vecPhoneNum.push_back( std::wstring( L"010-4118-3867" ) );
						//kPacketNot.m_vecPhoneNum.push_back( std::wstring( L"010-5014-1853" ) );
						//kPacketNot.m_vecPhoneNum.push_back( std::wstring( L"010-8584-3123" ) );
						//kPacketNot.m_vecPhoneNum.push_back( std::wstring( L"010-4342-0365" ) );
						//kPacketNot.m_vecPhoneNum.push_back( std::wstring( L"010-7753-3789" ) );
						//kPacketNot.m_vecPhoneNum.push_back( std::wstring( L"010-8421-1075" ) );
						//kPacketNot.m_vecPhoneNum.push_back( std::wstring( L"010-9317-0790" ) );
						//kPacketNot.m_wstrSMSMessage += L"�������ݹ߰�! UserUID : ";
						//kPacketNot.m_wstrSMSMessage += boost::str( boost::wformat( L"%d" ) % GetUID() );
						//kPacketNot.m_wstrSMSMessage += L", UnitUID : ";
						//kPacketNot.m_wstrSMSMessage += boost::str( boost::wformat( L"%d" ) % GetCharUID() );
						//kPacketNot.m_wstrSMSMessage += L", NickName : ";
						//kPacketNot.m_wstrSMSMessage += GetCharName();
						//SendToSMSDB( DBE_SEND_PHONE_MSG_NOT, kPacketNot );
					}				
				}			
#endif SERV_AUTO_HACK_CHECK_LEVEL
			//}}
#ifdef SERV_DUNGEON_CLEAR_PAYMENT_ITEM_EVENT// �۾���¥: 2013-07-14	// �ڼ���
			}
#endif // SERV_DUNGEON_CLEAR_PAYMENT_ITEM_EVENT			
			//////////////////////////////////////////////////////////////////////////
#endif SERV_CHAR_LEVEL_UP_EVENT
			//////////////////////////////////////////////////////////////////////////
			//}}

			// ü��ID ��� ����
			if( IsGuestUser() )
			{
				// ü��ID ���� ����
				if( GetLevel() > SiKGameSysVal()->GetGuestUserLimitLevel() )
				{
					SendID( EGS_GUEST_USER_LIMIT_LEVEL_NOT );
				}
			}

			// ���ο� �̼� �˻�
			m_kUserTitleManager.CheckNewMission( GetThisPtr<KGSUser>() );

			//{{ 2009. 10. 8  ������	���
#ifdef GUILD_TEST
			SendUpdateGuildMemberInfo();
#endif GUILD_TEST
			//}}

			//{{ 2011. 08. 29	������	ĳ���� ������ �̺�Ʈ
#ifdef SERV_CHAR_LEVEL_UP_ITEM
			KEGS_UPDATE_UNIT_INFO_NOT kPacketUnitInfo;
			GetUnitInfo( kPacketUnitInfo.m_kUnitInfo );
			SendPacket( EGS_UPDATE_UNIT_INFO_NOT, kPacketUnitInfo );
#endif SERV_CHAR_LEVEL_UP_ITEM
			//}}

			//{{ QUEST ���� - ��μ�
#ifdef SERV_REFORM_QUEST
			m_kUserQuestManager.CheckLimitLevelQuest( GetThisPtr<KGSUser>() );
			m_kUserQuestManager.CheckEventQuest( GetThisPtr<KGSUser>() );
#endif SERV_REFORM_QUEST
			//}}

			// �ǽð� ������
			if( GetStateID() == KGSFSM::S_ROOM  &&  GetRoomUID() != 0 )
			{
				switch( CXSLRoom::GetRoomType( GetRoomUID() ) )
				{
				case CXSLRoom::RT_DUNGEON:
					//{{ 2012. 02. 16	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
				case CXSLRoom::RT_BATTLE_FIELD:
#endif SERV_BATTLE_FIELD_SYSTEM
					//}}				
					{
						KERM_CHAR_LEVEL_UP_NOT kPacketNot;
						kPacketNot.m_kBaseStat = m_kStat;
						GetRoomUserInfo( kPacketNot.m_kRoomUserInfo, CXSLRoom::RT_DUNGEON );
						SendToCnRoom( ERM_CHAR_LEVEL_UP_NOT, kPacketNot );
					}
					break;

				default:
					{
						// Ŭ���̾�Ʈ�� �ٷ� ������!
						KEGS_CHAR_LEVEL_UP_NOT kPacketNot;
						kPacketNot.m_iUnitUID = GetCharUID();
						kPacketNot.m_ucLevel = GetLevel();
						kPacketNot.m_kBaseStat = m_kStat;
						SendPacket( EGS_CHAR_LEVEL_UP_NOT, kPacketNot );
					}
					break;
				}
			}
			else
			{
				// Ŭ���̾�Ʈ�� �ٷ� ������!
				KEGS_CHAR_LEVEL_UP_NOT kPacketNot;
				kPacketNot.m_iUnitUID = GetCharUID();
				kPacketNot.m_ucLevel = GetLevel();
				kPacketNot.m_kBaseStat = m_kStat;
				SendPacket( EGS_CHAR_LEVEL_UP_NOT, kPacketNot );				
			}

			//{{ 2011. 12. 19	������	ĳ���� ������ �ǽð� DB����
#ifdef SERV_LEVEL_UP_REALTIME_DB_UPDATE
			WriteUnitInfoToDB( false );

			//{{ 2012. 09. 22	�ڼ���	���� �̺�Ʈ
#ifdef SERV_EVENT_BINGO
			BingoEventDBWrite();
#endif SERV_EVENT_BINGO
			//}}

#endif SERV_LEVEL_UP_REALTIME_DB_UPDATE
			//}}

			//{{ 2013. 09. 24	������	�Ϻ� �̺�Ʈ �߰�DB�۾�
#ifdef SERV_RELAY_DB_CONNECTION
			SendUpdateUnitInfoToRelayDB( KDBE_UPDATE_USER_INFO_TO_RELAY_DB_JP_EVENT_NOT::UT_LEVEL_UP );
#endif SERV_RELAY_DB_CONNECTION
			//}}

			return true;
		}
        else
		{
			START_LOG_WITH_NAME( cerr )
				<< L"UNIT STAT DATA �Ľ� ����.!(�̷��� �ȵǴµ�)" << dbg::endl
				<< BUILD_LOGc( GetUnitClass() )
				<< BUILD_LOG( GetLevel() )
				<< END_LOG;

			return false;
		}
	}

	return false;
}

bool KGSUser::ResetStat()
{
	//������ ������ ĳ���� �������� ��ü�ϱ����� �Լ�.
	KStatTable::KUnitStatInfo	sUnitStat;
	if( SiKStatTable()->GetUnitStat( GetUnitClass(), m_ucLevel, sUnitStat ) == true )
	{
		m_kStat.m_iBaseHP		= sUnitStat.m_uiHP;
		m_kStat.m_iAtkPhysic	= sUnitStat.m_usAtkPhysic;
		m_kStat.m_iAtkMagic		= sUnitStat.m_usAtkMagic;
		m_kStat.m_iDefPhysic	= sUnitStat.m_usDefPhysic;
		m_kStat.m_iDefMagic		= sUnitStat.m_usDefMagic;

		return true;
	}
	else
	{
		START_LOG_WITH_NAME( cerr )
			<< L"UNIT STAT DATA �Ľ� ����.!(�̷��� �ȵǴµ�)" << dbg::endl
			<< BUILD_LOGc( GetUnitClass() )
			<< BUILD_LOG( m_ucLevel )
			<< END_LOG;

		return false;
	}
}

//{{ 2010. 8. 18	������	���� �ڵ� ����
void KGSUser::ClearTempInventory()
{
	// �ӽ� �κ��丮�� �������� �����ִ��� Ȯ��!
	if( m_kTempInventory.IsEmpty() == true )
		return;
	
	// �ӽ� �κ��� �����!
	m_kTempInventory.Clear();

	// Ŭ���̾�Ʈ���� �˷�����!
	SendID( EGS_CLEAR_TEMP_INVENTORY_NOT );
}
//}}

char KGSUser::GetUnitState()
{
	char cState = static_cast<char>(CXSLUnit::CUS_FIELD_MAP);

	if( GetRoomUID() != 0 )
	{
		switch( CXSLRoom::GetRoomType( GetRoomUID() ) )
		{
		case CXSLRoom::RT_SQUARE:
			cState = static_cast<char>(CXSLUnit::CUS_MARKET);
			break;
		case CXSLRoom::RT_TRAININGCENTER:
			cState = static_cast<char>(CXSLUnit::CUS_TC_PLAY);
			break;
		case CXSLRoom::RT_DUNGEON:
			cState = static_cast<char>(CXSLUnit::CUS_DUNGEON_PLAY);
			break;
		case CXSLRoom::RT_PVP:
			{
				KRoomInfoPtr spRoomInfo = SiKRoomListManager()->GetRoomInfo( GetRoomUID() );
				if( spRoomInfo != NULL )
				{
					switch( spRoomInfo->m_RoomState )
					{
					case CXSLRoom::RS_WAIT: cState = static_cast<char>(CXSLUnit::CUS_PVP_WAIT);
						break;
					case CXSLRoom::RS_PLAY: cState = static_cast<char>(CXSLUnit::CUS_PVP_PLAY);
						break;
					}
				}
			}
			break;
		}
	}
	else
	{
		//{{ 2011. 07. 29	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
		if( GetRoomListID() != 0  &&  GetRoomListID() != SiKRoomListManager()->GetPvpOffcialRoomListID() )
#else
		if( GetRoomListID() != 0 )
#endif SERV_PVP_NEW_SYSTEM
		//}}
			cState = static_cast<char>(CXSLUnit::CUS_PVP_LOBBY);
	}

	return cState;
}

bool KGSUser::GetPVPChannelClass( OUT int& iPvpChannelClass ) const
{
	iPvpChannelClass = KPVPChannelInfo::PCC_NONE; // default

	// RoomListID�� �����鼭 ����� ������? üũ!
	if( GetRoomListID() == 0 )
	{
		return false;
	}

	// RoomListID�� ���� ä�� ���� ���!
	const int iPvpChannelClassByRoomListID = SiKRoomListManager()->GetPVPChannelClassByRoomListID( GetRoomListID() );
	if( iPvpChannelClassByRoomListID == KPVPChannelInfo::PCC_NONE )
	{
		START_LOG( cerr, L"���� ���� �ʴ� ���� ä���� RoomListID���� ������ �ֽ��ϴ�! ���� �� ���� ����!" )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOG( GetRoomListID() )
			<< BUILD_LOG( GetRoomUID() )
			<< BUILD_LOG( GetStateIDString() )
			<< END_LOG;

		return false;
	}

	// ���ϰ� ������Ʈ
	iPvpChannelClass = iPvpChannelClassByRoomListID;

	// RoomUID���� �ִٸ� ���� ä�� ������ ��������!
	if( GetRoomUID() > 0 )
	{
		const int iPvpChannelClassByRoomUID = SiKRoomListManager()->GetPVPChannelClassByRoomUID( GetRoomUID() );
		if( iPvpChannelClassByRoomUID != KPVPChannelInfo::PCC_NONE )
		{
			if( iPvpChannelClassByRoomUID != iPvpChannelClassByRoomListID )
			{
				START_LOG( cerr, L"RoomListID�� ���� ����ä�ΰ� RoomUID�� ���� ���� ä���� �ٸ���? ���� �� ���� ����!" )
					<< BUILD_LOG( GetCharUID() )
					<< BUILD_LOG( GetRoomListID() )
					<< BUILD_LOG( GetRoomUID() )
					<< BUILD_LOG( GetStateIDString() )
					<< BUILD_LOG( iPvpChannelClassByRoomListID )
					<< BUILD_LOG( iPvpChannelClassByRoomUID )
					<< END_LOG;

				return false;
			}
		}
	}

	return true;
}

//{{ 2011. 12. 20	������	 ��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
#else
//{{ 2010. 02. 09  ������	������Ƽ ��ɰ���
//#ifdef SERV_QUICK_PARTY_UPGRADE
//void KGSUser::AddKickedPartyUID( IN UidType iPartyUID )
//{
//	if( CheckKickedPartyUID( iPartyUID ) )
//		return;
//
//	boost::timer tKickedPartyUIDTimer;
//	tKickedPartyUIDTimer.restart();
//	m_mapKickedPartyUIDList.insert( std::make_pair( iPartyUID, tKickedPartyUIDTimer ) );
//}
//
//void KGSUser::CheckExpiredKickedPartyUID()
//{
//	if( m_mapKickedPartyUIDList.empty() )
//		return;
//
//	std::vector< UidType > vecDeletePartyUIDList;
//
//	std::map< UidType, boost::timer >::const_iterator mit;
//	for( mit = m_mapKickedPartyUIDList.begin(); mit != m_mapKickedPartyUIDList.end(); ++mit )
//	{
//		// 20�� �̻� �����ٸ� �����Ѵ�!
//		if( mit->second.elapsed() > 20.0 )
//		{
//			vecDeletePartyUIDList.push_back( mit->first );
//		}
//	}
//
//	BOOST_TEST_FOREACH( const UidType, iDeletePartyUID, vecDeletePartyUIDList )
//	{
//		m_mapKickedPartyUIDList.erase( iDeletePartyUID );
//	}
//}
//
//#endif SERV_QUICK_PARTY_UPGRADE
//}}
#endif SERV_BATTLE_FIELD_SYSTEM
//}}

void KGSUser::GetPartyUserInfo( KPartyUserInfo& kInfo )
{
	//////////////////////////////////////////////////////////////////////////
	//{{ 2010. 02. 17  ������	ä����Ƽ����
#ifdef SERV_CHANNEL_PARTY
	//////////////////////////////////////////////////////////////////////////

	kInfo.m_iGSUID			= KBaseServer::GetKObj()->GetUID();
	kInfo.m_iChannelID		= SiKChannelManager()->GetChannelID();
	kInfo.m_iFieldUID		= GetFieldUID();
	kInfo.m_iUnitUID		= GetCharUID();
	kInfo.m_cUnitClass		= GetUnitClass();
	kInfo.m_wstrNickName	= GetCharName();
	kInfo.m_ucLevel			= GetLevel();
	kInfo.m_bIsGuestUser	= IsGuestUser();
	kInfo.m_cState			= GetUnitState();
	//{{ 2011. 05. 27    ��μ�    �޸� ���� ���� ����
#ifdef SERV_COME_BACK_USER_REWARD
	CTime tComeBackBuffEnd	= m_kComeBackUserInfo.GetComeBackBuffEnd();
	kInfo.m_bComeBack		= m_kComeBackUserInfo.GetIsComeBackUser();
	kInfo.m_wsrtComeBackEnd = tComeBackBuffEnd.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
#endif SERV_COME_BACK_USER_REWARD
	//}} 
	//{{ 2011. 07. 11	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
	//{{ 2012. 09. 11	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2
	kInfo.m_cRank				= GetPvpRankForClient();
	kInfo.m_iOfficialMatchCnt	= GetOfficialMatchCount();
#endif SERV_2012_PVP_SEASON2
	//}}
	kInfo.m_iRating			= GetRating();
	kInfo.m_iRPoint			= GetRPoint();
#endif SERV_PVP_NEW_SYSTEM
	//}}
	if( kInfo.m_cState == CXSLUnit::CUS_FIELD_MAP )
	{
		kInfo.m_iStateCode = GetMapID();
	}	
	//{{ 2012. 05. 29	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	m_kUserUnitManager.GetPartyMemberStatus( kInfo.m_kGamePlayStatus );
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}
	//{{ �� ����üũ ȣ��Ʈ ���� - ��μ�
#ifdef SERV_CHOOSE_FASTEST_HOST
	kInfo.m_wstrIP = GetP2PIP();
	kInfo.m_usPort = GetP2PPort();
#endif SERV_CHOOSE_FASTEST_HOST
	//}

	//{{ 2013. 1. 8	�ڼ���	Merge ����IP ���� ���н� ����IP�� �õ�( ������ )
//#ifdef SERV_KTDX_RETRY_USING_INTERNAL_IP
	kInfo.m_wstrInternalIP = GetP2PInternalIP();
	kInfo.m_usInternalPort = GetP2PInternalPort();

	START_LOG( clog, L"IP, ��Ʈ Ȯ��" )
		<< BUILD_LOG( kInfo.m_wstrInternalIP )
		<< BUILD_LOG( kInfo.m_usInternalPort )
		<< END_LOG;
//#endif SERV_KTDX_RETRY_USING_INTERNAL_IP
	//}}

	//////////////////////////////////////////////////////////////////////////
#else
	//////////////////////////////////////////////////////////////////////////

	//�ڽ��� ������ ��� ��Ƽ�� ������.
	kInfo.m_iUnitUID		= GetCharUID();
	kInfo.m_wstrNickName	= GetCharName();
	kInfo.m_cUnitClass		= GetUnitClass();
	kInfo.m_ucLevel			= GetLevel();
	kInfo.m_cState			= GetUnitState();
	if( kInfo.m_cState == CXSLUnit::CUS_FIELD_MAP )
		kInfo.m_iStateCode = GetMapID();

	//////////////////////////////////////////////////////////////////////////
#endif SERV_CHANNEL_PARTY
	//}}
	//////////////////////////////////////////////////////////////////////////
}

void KGSUser::SendUpdatePartyUnitInfo( bool bChangeField /* = false  */)
{
	if( GetPartyUID() != 0 )
	{
		//KERM_UPDATE_PARTY_USER_INFO_NOT kInfo;

		//kInfo.m_cState		= GetUnitState();
		//kInfo.m_iStateCode	= 0;
		//kInfo.m_cUnitClass	= GetUnitClass();
		//kInfo.m_ucLevel		= GetLevel();

		//if( kInfo.m_cState == CXSLUnit::CUS_FIELD_MAP )
		//	kInfo.m_iStateCode = GetMapID();

		//_SendToCnRoom( m_iPartyUID, ERM_UPDATE_PARTY_USER_INFO_NOT, kInfo );
		//{{ 2010. 02. 22  ������	ä�� ��Ƽ ����
#ifdef SERV_CHANNEL_PARTY
		if( SiKPartyListManager()->E_UpdatePartyUserInfo( GetThisPtr<KGSUser>(), bChangeField ) == false )
#else
		if( SiKPartyManager()->E_UpdatePartyUserInfo( GetPartyUID(), GetThisPtr<KGSUser>(), bChangeField ) == false )
#endif SERV_CHANNEL_PARTY
		//}}
		{
			START_LOG( cerr, L"��Ƽ���� ���� ������Ʈ ����." )
#ifdef SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetCharUID() )
#else
				<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
				<< END_LOG;
		}
	}
}

void KGSUser::SendLeaveParty( int iReason )
{
	//{{ 2010. 03. 10  ������	ä�� ���� ��Ƽ
#ifdef SERV_CHANNEL_PARTY
	if( GetPartyUID() > 0  ||  GetCandidatePartyUID() > 0 )
#else
	if( GetPartyUID() > 0 )
#endif SERV_CHANNEL_PARTY
	//}}	
	{
		KEGS_LEAVE_PARTY_REQ kPacket;
		kPacket.m_iReason = iReason;

		//{{ 2010. 02. 22  ������	ä�� ��Ƽ ����
#ifdef SERV_CHANNEL_PARTY
		if( SiKPartyListManager()->E_LeavePartyReq( GetThisPtr<KGSUser>(), kPacket ) == false )
#else
		if( SiKPartyManager()->E_LeaveParty( GetPartyUID(), GetThisPtr<KGSUser>(), kPacket ) == false )
#endif SERV_CHANNEL_PARTY
		//}}
		{
			START_LOG( cerr, L"��Ƽ ������ ����.!" )
#ifdef SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetCharUID() )
#else
				<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( NetError::GetErrStr(iReason) )
				<< END_LOG;
		}
	}
}

void KGSUser::SendLeaveField( int iReason )
{
	if( GetFieldUID() > 0 )
	{
		KERM_LEAVE_FIELD_REQ kPacket;
		kPacket.m_iReason = iReason;
		_SendToGSField( GetFieldUID(), ERM_LEAVE_FIELD_REQ, kPacket );

		SetFieldUID( 0 );
	}    
}

void KGSUser::SendLeavePVPLobby()
{
	if( GetRoomListID() > 0 )
	{
		SendUpdatePVPUserInfo( KCommunityUserInfo::US_LEAVE_LOBBY );

		SetRoomListID( 0 );

		//{{ 2009. 10. 23  ������	���
#ifdef GUILD_TEST
		SendUpdateGuildMemberInfo();
#endif GUILD_TEST
		//}}
	}
}

//{{ 2011. 07. 11	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
void KGSUser::SendLeavePVPMatch( IN const int iReason )
{
	if( m_kUserPvpManager.IsRequestMatchMaking() == true  ||  GetMatchUID() != 0 )
	{
		KEGB_CANCEL_MATCH_MAKING_REQ kPacketReq;
		kPacketReq.m_iMatchUID = GetMatchUID();
		kPacketReq.m_iPartyUID = GetPartyUID();
		kPacketReq.m_iReason = iReason;
		kPacketReq.m_bSendBySystem = true;
		SendToMatch( EGB_CANCEL_MATCH_MAKING_REQ, kPacketReq );

		m_kUserPvpManager.SetMatchUID( 0 );
	}
}
#endif SERV_PVP_NEW_SYSTEM
//}}

//{{ 2012. 02. 10	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
void KGSUser::SendLeaveAutoParty( IN const int iReason )
{
	if( m_kUserDungeonManager.GetAutoPartyUID() != 0  ||  m_kUserDungeonManager.GetAutoPartyWaitNumber() != 0 )
	{
		// �ϴ��� �ߺ� ������ ���ϱ� ���� ������ ����Ǿ��ִ����� ��� ��Ű��!
		KEGB_CANCEL_AUTO_PARTY_MAKING_REQ kPacketReq;
		kPacketReq.m_iAutoPartyUID = m_kUserDungeonManager.GetAutoPartyUID();
		kPacketReq.m_iAutoPartyWaitNumber = m_kUserDungeonManager.GetAutoPartyWaitNumber();
		kPacketReq.m_iReason = iReason;
		kPacketReq.m_bSendBySystem = true;
		SendToAutoParty( EGB_CANCEL_AUTO_PARTY_MAKING_REQ, kPacketReq );

		m_kUserDungeonManager.SetAutoPartyUID( 0 );
		m_kUserDungeonManager.SetAutoPartyWaitNumber( 0 );
	}
}
#endif SERV_BATTLE_FIELD_SYSTEM
//}}

//{{ 2012. 03. 05	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
void KGSUser::SetRoomUID( IN const UidType iUID )
{
	// �� ���� ������Ʈ
	m_iBeforeRoomUID = GetRoomUID(); // ���� �� ������ ���� �����ص���! ( ��Ʋ�ʵ忡�� ���� �ʵ�� ���ư��� ��� )
	m_iRoomUID = iUID;

	// ��Ƽ �Ҽ��̶�� �� ���忡 ���� ������ ������Ʈ ����!
	if( GetPartyUID() != 0 )
	{
		KEPM_UPDATE_PARTY_USER_JOIN_ROOM_INFO_NOT kPacketNot;
		kPacketNot.m_kJoinRoomInfo.m_iRoomUID = GetRoomUID();
		kPacketNot.m_kJoinRoomInfo.m_iBattleFieldID = GetMapID();
		SendToCnParty( EPM_UPDATE_PARTY_USER_JOIN_ROOM_INFO_NOT, kPacketNot );
	}

	// ���� ó��!
	if( GetBeforeRoomUID() == 0 )
	{
		if( iUID == 0 )
		{
			START_LOG( cerr, L"�濡�� �̹� �����µ� �� ������� ������ �Գ�? �̻��� ������!" )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOG( GetCharName() )
				<< BUILD_LOG( GetStateIDString() )
				<< END_LOG;
		}
		else
		{
            // �濡 ����
		}
	}
	else
	{
		if( iUID == 0 )
		{
			switch( CXSLRoom::GetRoomType( GetBeforeRoomUID() ) )
			{
			case CXSLRoom::RT_DUNGEON:
				{
					//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
					// �����濡 �ִٰ� ��Ż�ϴ� ���!
					std::vector<int> vecDeleteBuff;
					//{{ 2013. 01. 09 ���� ���� �ý��� - ��μ�

					std::vector< KBuffInfo > vecActivateBuff;	// �ؿ��� ��ġ ����
#ifdef SERV_DUNGEON_FORCED_EXIT_SYSTEM
					m_kUserBuffManager.OnLeaveDungeonRoom( GetThisPtr<KGSUser>(), vecActivateBuff, vecDeleteBuff, false );
#else
					m_kUserBuffManager.OnLeaveDungeonRoom( GetThisPtr<KGSUser>(), vecDeleteBuff );
#endif SERV_DUNGEON_FORCED_EXIT_SYSTEM
					//}

					if( vecDeleteBuff.empty() == false )
					{
						// �α��� ���� ���� �Ŵ����� ������Ʈ
						KELG_UPDATE_USER_UNIT_BUFF_INFO_REQ kReq;
						kReq.m_iUnitUID = GetCharUID();
						kReq.m_vecDeActivateBuff = vecDeleteBuff;
						kReq.m_vecActivateBuff = vecActivateBuff;
						SendToLoginServer( ELG_UPDATE_USER_UNIT_BUFF_INFO_REQ, kReq );
					}
#endif SERV_SERVER_BUFF_SYSTEM
					//}}
				}
				break;
				
			case CXSLRoom::RT_BATTLE_FIELD:
				{
					std::vector<int> vecDeleteBuff;
					std::vector< KBuffInfo > vecActivateBuff;
					m_kUserBuffManager.OnLeaveBattleField( GetThisPtr<KGSUser>(), vecActivateBuff, vecDeleteBuff );

					if( vecDeleteBuff.empty() == false )
					{
						// �α��� ���� ���� �Ŵ����� ������Ʈ
						KELG_UPDATE_USER_UNIT_BUFF_INFO_REQ kReq;
						kReq.m_iUnitUID = GetCharUID();
						kReq.m_vecDeActivateBuff = vecDeleteBuff;
						kReq.m_vecActivateBuff = vecActivateBuff;
						SendToLoginServer( ELG_UPDATE_USER_UNIT_BUFF_INFO_REQ, kReq );
					}
				}break;
			}
		}
		else
		{
			START_LOG( cerr, L"�̹� �濡 ������ �ִµ� �� �����Ϸ��� �ϳ�? �̷��� �ȵ�!" )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOG( GetCharName() )
				<< BUILD_LOG( GetStateIDString() )
				<< BUILD_LOG( GetBeforeRoomUID() )
				<< BUILD_LOG( CXSLRoom::GetRoomType( GetBeforeRoomUID() ) )
				<< BUILD_LOG( GetRoomUID() )
				<< BUILD_LOG( CXSLRoom::GetRoomType( GetRoomUID() ) )
				<< END_LOG;
		}
	}
}

void KGSUser::SetFieldUID( IN const UidType iUID )
{
	// �ʵ� ���� ������Ʈ
	const UidType iBeforeFieldUID = GetFieldUID();
	m_iFieldUID = iUID;

	// ���� ó��!
	if( iBeforeFieldUID == 0 )
	{
		if( iUID == 0 )
		{
			START_LOG( cerr, L"�ʵ忡�� �̹� �����µ� �� ������� ������ �Գ�? �̻��� ������!" )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOG( GetCharName() )
				<< BUILD_LOG( GetStateIDString() )
				<< END_LOG;
		}
		else
		{
			// �ʵ忡 ����
		}
	}
	else
	{
		if( iUID == 0 )
		{
			//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
			std::vector< int > vecDeactivateBuff;
			m_kUserBuffManager.OnLeaveField( GetThisPtr<KGSUser>(), vecDeactivateBuff );
			if( vecDeactivateBuff.empty() == false )
			{
				// �α��� ���� ���� �Ŵ����� ������Ʈ
				KELG_UPDATE_USER_UNIT_BUFF_INFO_REQ kReq;
				kReq.m_iUnitUID = GetCharUID();
				kReq.m_vecDeActivateBuff = vecDeactivateBuff;
				SendToLoginServer( ELG_UPDATE_USER_UNIT_BUFF_INFO_REQ, kReq );
			}
#endif SERV_SERVER_BUFF_SYSTEM
			//}}
		}
		else
		{
			// �ٸ� �ʵ忡 ����
		}
	}
}
#endif SERV_BATTLE_FIELD_SYSTEM
//}}			

//{{ 2010. 02. 20  ������	ä�� ��Ƽ ����
//////////////////////////////////////////////////////////////////////////
#ifdef SERV_CHANNEL_PARTY
//////////////////////////////////////////////////////////////////////////

void KGSUser::SendUpdatePartyMemberList( IN UidType iPartyUID, IN const std::set< UidType >& setPartyUIDList )
{
	// ���� ���� ��Ƽ�� �Ҽӵ� �������� UID�� �ʵ忡 �����ش�.

	//ȣ��Ǵ� ���� ��Ƽ�Ŵ�������
	//## 1. E_CreateParty
	//## 2. E_JoinPartyInviteReply
	//## 3. E_InvitePartyReply
	//## 4. E_LeaveParty
	//## 5. E_BanUser

	if( GetStateID() != KGSFSM::S_FIELD_MAP )		
		return;

	if( GetFieldUID() == 0 )
	{
		// �ʵ���������� FieldUID�� 0�� ��찡 �ִ�. [�����κ�, �Ʒüҷκ�]
		START_LOG( clog, L"���� �ʵ� �����ε� UID�� �߸��Ǿ���." )
#ifdef SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetCharUID() )
#else
			<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetFieldUID() )
			<< END_LOG;
		return;
	}

	KERM_UPDATE_PARTY_NOT kNot;
	kNot.m_iPartyUID = iPartyUID;
	kNot.m_setPartyUIDList = setPartyUIDList;
	SendToGSField( ERM_UPDATE_PARTY_NOT, kNot );
}

//////////////////////////////////////////////////////////////////////////
#else
//////////////////////////////////////////////////////////////////////////

void KGSUser::SendUpdatePartyList()
{
	// ���� ���� ��Ƽ�� �Ҽӵ� �������� UID�� �ʵ忡 �����ش�.

	//ȣ��Ǵ� ���� ��Ƽ�Ŵ�������
	//## 1. E_CreateParty
	//## 2. E_JoinPartyInviteReply
	//## 3. E_InvitePartyReply
	//## 4. E_LeaveParty
	//## 5. E_BanUser

	if( GetStateID() != KGSFSM::S_FIELD_MAP )		
		return;

	if( GetFieldUID() == 0 )
	{
		// �ʵ���������� FieldUID�� 0�� ��찡 �ִ�. [�����κ�, �Ʒüҷκ�]
		START_LOG( clog, L"���� �ʵ� �����ε� UID�� �߸��Ǿ���." )
#ifdef SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetCharUID() )
#else
			<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetFieldUID() )
			<< END_LOG;
		return;
	}	

	KERM_UPDATE_PARTY_NOT kNot;
	kNot.m_iPartyUID = GetPartyUID();
	
	if( GetPartyUID() == 0 )
	{
		// ��Ƽ��Ż�� �������� �� ��Ƽ����Ʈ�� ������.
		_SendToGSField( GetFieldUID(), ERM_UPDATE_PARTY_NOT, kNot );
		return;
	}
	else
	{
		// ��Ƽ����Ʈ ������ �ִ� �������� �ڽ��� ������ ��Ƽ����Ʈ�� ������.
		if( SiKPartyManager()->GetUserUIDList( GetPartyUID(), kNot.m_setPartyUIDList ) == true )
		{
			_SendToGSField( GetFieldUID(), ERM_UPDATE_PARTY_NOT, kNot );
		}
		else
		{
			START_LOG( cerr, L"��Ƽ�� UID ����Ʈ �޾ƿ��� ����.!" )
#ifdef SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetCharUID() )
#else
				<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetPartyUID() )
				<< END_LOG;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
#endif SERV_CHANNEL_PARTY
//////////////////////////////////////////////////////////////////////////
//}}

//{{ 2009. 4. 14  ������	ģ������Ʈ
void KGSUser::SendUpdateFriendList()
{
	// �������� ģ������Ʈ�� �ʵ忡 �����ش�.

	if( GetStateID() != KGSFSM::S_FIELD_MAP )
		return;

	if( GetFieldUID() == 0 )
	{
		// �ʵ���������� FieldUID�� 0�� ��찡 �ִ�. [�����κ�, �Ʒüҷκ�]
		START_LOG( clog, L"���� �ʵ� �����ε� UID�� �߸��Ǿ���." )
#ifdef SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetCharUID() )
#else
			<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetFieldUID() )
			<< END_LOG;
		return;
	}

	KERM_UPDATE_FRIEND_NOT kNot;

	std::map< std::wstring, UidType >::const_iterator mit;
	for( mit = m_mapFriendList.begin(); mit != m_mapFriendList.end(); ++mit )
	{
		kNot.m_setFriendUIDList.insert( mit->second );
	}

	SendToGSField( ERM_UPDATE_FRIEND_NOT, kNot );
}
//}}

//{{ 2009. 10. 8  ������	���
#ifdef GUILD_TEST
void KGSUser::SendUpdateGuildMemberInfo()
{
	if( GetGuildUID() > 0 )
	{
		KELG_UPDATE_GUILD_MEMBER_INFO_NOT kPacketNot;
		kPacketNot.m_iGuildUID = GetGuildUID();
		GetGuildMemberInfo( kPacketNot.m_kMemberInfo );
		SendToLoginServer( ELG_UPDATE_GUILD_MEMBER_INFO_NOT, kPacketNot );
	}
}
#endif GUILD_TEST
//}}

//{{ 2010. 01. 11  ������	��õ�θ���Ʈ
void KGSUser::SendUpdateRecommendUserInfo( IN const KRecommendUserInfo::RECOMMEND_USER_INFO_TYPE eType )
{
	if( m_kUserRecommendManager.GetRecommendedUnitUID() > 0 )
	{
		KELG_RECOMMEND_USER_INFO_NOT kPacketToLg;
		kPacketToLg.m_iRecommendedUnitUID = m_kUserRecommendManager.GetRecommendedUnitUID();
		kPacketToLg.m_iRecommendUnitUID = GetCharUID();
		kPacketToLg.m_cType = static_cast<char>(eType);
		kPacketToLg.m_iData = GetLevel();
		SendToLoginServer( ELG_RECOMMEND_USER_INFO_NOT, kPacketToLg );
	}

#ifdef SERV_RECRUIT_EVENT_BASE
	// ����õ�� ����Ʈ �����´�.
	std::vector< KRecommendUserInfo > vecRecruitUnitInfo;
	m_kUserRecommendManager.GetRecruitUnitList( vecRecruitUnitInfo );
	for( int i = 0; i < vecRecruitUnitInfo.size(); ++i )
	{
		KRecommendUserInfo& kRecruitUnitInfo = vecRecruitUnitInfo[i];

		KELG_RECRUIT_RECRUITER_INFO_NOT kPacketToLg;
		kPacketToLg.m_iSenderUnitUID = GetCharUID();
		kPacketToLg.m_iReceiverUnitUID = kRecruitUnitInfo.m_iUnitUID;
		kPacketToLg.m_cType = static_cast<char>( eType );
		kPacketToLg.m_iData = GetLevel();
		SendToLoginServer( ELG_RECRUIT_RECRUITER_INFO_NOT, kPacketToLg );
	}

	// ��õ�� ����Ʈ �����´�. (����μ� �ϳ�����)
	std::vector< KRecommendUserInfo > vecRecruiterUnitInfo;
	m_kUserRecommendManager.GetRecruiterUnitList( vecRecruiterUnitInfo );
	for( int i = 0; i < vecRecruiterUnitInfo.size(); ++i )
	{
		KRecommendUserInfo& kRecruiterUnitInfo = vecRecruiterUnitInfo[i];

		KELG_RECRUIT_RECRUITER_INFO_NOT kPacketToLg;
		kPacketToLg.m_iSenderUnitUID = GetCharUID();
		kPacketToLg.m_iReceiverUnitUID = kRecruiterUnitInfo.m_iUnitUID;
		kPacketToLg.m_cType = static_cast<char>( eType );
		kPacketToLg.m_iData = GetLevel();
		SendToLoginServer( ELG_RECRUIT_RECRUITER_INFO_NOT, kPacketToLg );
	}
#endif SERV_RECRUIT_EVENT_BASE
}
//}}

//{{ 2009. 4. 22  ������	������������Ʈ
void KGSUser::SendUpdatePVPUserInfo( char cState )
{
	// �κ� ����
	if( cState == KCommunityUserInfo::US_ENTER_LOBBY )
	{
		if( GetRoomListID() <= 0 )
		{
			START_LOG( cerr, L"�����κ� �����Ϸ��µ� RoomListID���� �̻��մϴ�!" )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOG( GetRoomListID() )
				<< END_LOG;

			// �����κ� ����ó�� ��������!
			return;
		}

		//{{ 2011. 07. 26	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
		// ���Ĵ��� �κ���?
		if( GetRoomListID() == SiKRoomListManager()->GetPvpOffcialRoomListID() )
			return;
#endif SERV_PVP_NEW_SYSTEM
		//}}

		KECN_NEW_PVP_USER_INFO_NOT kPacketNot;
		kPacketNot.m_nGSUID					  = KBaseServer::GetKObj()->GetUID();
		kPacketNot.m_kUserInfo.m_iUnitUID	  = GetCharUID();
		kPacketNot.m_kUserInfo.m_wstrNickName = GetCharName();
		kPacketNot.m_kUserInfo.m_cUnitClass	  = GetUnitClass();
		kPacketNot.m_kUserInfo.m_ucLevel	  = GetLevel();
		kPacketNot.m_kUserInfo.m_cState		  = cState;
		kPacketNot.m_kUserInfo.m_iStateCode	  = GetRoomListID();
		kPacketNot.m_kUserInfo.m_iRoomUID	  = GetRoomUID();
		SendToCnServer( ECN_NEW_PVP_USER_INFO_NOT, kPacketNot );

		//{{ 2009. 10. 23  ������	���
#ifdef GUILD_TEST
		SendUpdateGuildMemberInfo();
#endif GUILD_TEST
		//}}
	}
	// ���� ����
	else
	{
		//{{ 2011. 07. 26	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
		// ���Ĵ��� �κ���?
		if( GetRoomListID() == SiKRoomListManager()->GetPvpOffcialRoomListID() )
			return;
#endif SERV_PVP_NEW_SYSTEM
		//}}

		KPVPUserSimpleInfo kInfo;
		kInfo.m_iUnitUID	= GetCharUID();
		kInfo.m_cState		= cState;
		kInfo.m_iStateCode	= GetRoomListID();
		kInfo.m_iRoomUID	= GetRoomUID();

        KECN_UPDATE_PVP_USER_INFO_NOT kPacketNot;
		kPacketNot.m_vecUserInfo.push_back( kInfo );		
		SendToCnServer( ECN_UPDATE_PVP_USER_INFO_NOT, kPacketNot );
	}
}

//{{ 2009. 6. 5  ������		�������� ����
void KGSUser::SendUpdateChangeEquippedItem()
{	
	//{{ 2008. 12. 3  ������	�ʵ忡�� �Ⱓ�ٵ� ������ ����
	if( GetStateID() == KGSFSM::S_FIELD_MAP )
	{
		if( GetFieldUID() <= 0 )
		{
			// �ʵ� �����̸鼭 fielduid�� 0�ΰ�쵵 �ִ�.
			return;
		}

		KERM_CHANGE_EQUIPPED_ITEM_IN_FIELD_NOT kPacketNot;
		kPacketNot.m_iUnitUID = GetCharUID();
		//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
#ifdef SERV_TITLE_DATA_SIZE
		kPacketNot.m_iEquippedTitleID = m_kUserTitleManager.GetEquippedTitleID();
#else
		kPacketNot.m_sEquippedTitleID = m_kUserTitleManager.GetEquippedTitleID();
#endif SERV_TITLE_DATA_SIZE
		//}}		
		SendToGSField( ERM_CHANGE_EQUIPPED_ITEM_IN_FIELD_NOT, kPacketNot );
	}
	//}}
	// ���� Ÿ��Ʋ�� �뿡�� �ٲ����� �� ����鿡�� �˷��ش�.
	else if( GetStateID() == KGSFSM::S_ROOM )
	{
		if( GetRoomUID() <= 0 )
		{
			START_LOG( cerr, L"�� UID �̻�." )
				<< BUILD_LOG( GetRoomUID() )
				<< BUILD_LOG( GetCharUID() )
				<< END_LOG;

			return;
		}

		switch( CXSLRoom::GetRoomType( GetRoomUID() ) )
		{
		case CXSLRoom::RT_PVP:
		case CXSLRoom::RT_DUNGEON:
		case CXSLRoom::RT_TRAININGCENTER:
			//{{ 2012. 02. 15	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
		case CXSLRoom::RT_BATTLE_FIELD:
#endif SERV_BATTLE_FIELD_SYSTEM
			//}}
			{
				KERM_CHANGE_EQUIPPED_ITEM_IN_ROOM_NOT kPacketNot;
				//{{ 2010. 12. 23	������	��� ���� ��Ŷ Ÿ��
#ifdef SERV_CHANGE_EQUIPPED_PACKET_REFAC
				kPacketNot.m_cChangeEquippedPacketType = KEGS_CHANGE_EQUIPPED_ITEM_IN_ROOM_NOT::CEPT_CHANGE_EQUIP;
#endif SERV_CHANGE_EQUIPPED_PACKET_REFAC
				//}}
				kPacketNot.m_UnitUID = GetCharUID();
				GetRoomUserInfo( kPacketNot.m_kRoomUserInfo );
				SendToCnRoom( ERM_CHANGE_EQUIPPED_ITEM_IN_ROOM_NOT, kPacketNot );
			}
			break;

		case CXSLRoom::RT_SQUARE:
			{
				KERM_CHANGE_EQUIPPED_ITEM_IN_SQUARE_NOT kPacketNot;
				kPacketNot.m_UnitUID = GetCharUID();
				//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
#ifdef SERV_TITLE_DATA_SIZE
				kPacketNot.m_iEquippedTitleID = m_kUserTitleManager.GetEquippedTitleID();
#else
				kPacketNot.m_sEquippedTitleID = m_kUserTitleManager.GetEquippedTitleID();
#endif SERV_TITLE_DATA_SIZE
				//}}				
				SendToCnRoom( ERM_CHANGE_EQUIPPED_ITEM_IN_SQUARE_NOT, kPacketNot );
			}
			break;

		default:
			{
				START_LOG( cerr, L"�̻��� �� Ÿ��." )
					<< BUILD_LOG( CXSLRoom::GetRoomType( GetRoomUID() ) )
					<< BUILD_LOG( GetCharUID() )
					<< END_LOG;
			}
			break;
		}
	}
}
//}}

//{{ 2010. 02. 08  ������	PC�� ä�� �̵� ���� ����
void KGSUser::SendUpdateChangeEquippedItem( IN const std::vector< KInventoryItemInfo >& vecInventorySlotInfo )
{
	//{{ 2008. 12. 3  ������	�ʵ忡�� �Ⱓ�ٵ� ������ ����
	if( GetStateID() == KGSFSM::S_FIELD_MAP )
	{
		if( GetFieldUID() <= 0 )
		{
			// �ʵ� �����̸鼭 fielduid�� 0�ΰ�쵵 �ִ�.
			return;
		}

		KERM_CHANGE_EQUIPPED_ITEM_IN_FIELD_NOT kPacketNot;
		kPacketNot.m_iUnitUID = GetCharUID();
		std::vector< KInventoryItemInfo >::const_iterator vit;
		for( vit = vecInventorySlotInfo.begin(); vit != vecInventorySlotInfo.end(); ++vit )
		{
			if( vit->m_cSlotCategory == CXSLInventory::ST_E_EQUIP )
			{
				kPacketNot.m_vecInventorySlotInfo.push_back( *vit );
			}
		}

		if( kPacketNot.m_vecInventorySlotInfo.empty() == false )
		{
			//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
#ifdef SERV_TITLE_DATA_SIZE
			kPacketNot.m_iEquippedTitleID = m_kUserTitleManager.GetEquippedTitleID();
#else
			kPacketNot.m_sEquippedTitleID = m_kUserTitleManager.GetEquippedTitleID();
#endif SERV_TITLE_DATA_SIZE
			//}}			
			SendToGSField( ERM_CHANGE_EQUIPPED_ITEM_IN_FIELD_NOT, kPacketNot );
		}
	}
	//}}
	else if( GetStateID() == KGSFSM::S_ROOM )
	{
		if( GetRoomUID() <= 0 )
		{
			START_LOG( cerr, L"�� UID �̻�." )
				<< BUILD_LOG( GetRoomUID() )
				<< BUILD_LOG( GetCharUID() )
				<< END_LOG;

			return;
		}

		switch( CXSLRoom::GetRoomType( GetRoomUID() ) )
		{
		case CXSLRoom::RT_PVP:
		case CXSLRoom::RT_DUNGEON:			
		case CXSLRoom::RT_TRAININGCENTER:
			//{{ 2012. 02. 15	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
		case CXSLRoom::RT_BATTLE_FIELD:
#endif SERV_BATTLE_FIELD_SYSTEM
			//}}
			{
				KERM_CHANGE_EQUIPPED_ITEM_IN_ROOM_NOT kPacketNot;
				//{{ 2010. 12. 23	������	��� ���� ��Ŷ Ÿ��
#ifdef SERV_CHANGE_EQUIPPED_PACKET_REFAC
				kPacketNot.m_cChangeEquippedPacketType = KEGS_CHANGE_EQUIPPED_ITEM_IN_ROOM_NOT::CEPT_CHANGE_EQUIP;
#endif SERV_CHANGE_EQUIPPED_PACKET_REFAC
				//}}
				kPacketNot.m_UnitUID = GetCharUID();
				std::vector< KInventoryItemInfo >::const_iterator vit;
				for( vit = vecInventorySlotInfo.begin(); vit != vecInventorySlotInfo.end(); ++vit )
				{
					if( vit->m_cSlotCategory == CXSLInventory::ST_E_EQUIP )
					{
						kPacketNot.m_vecInventorySlotInfo.push_back( *vit );
					}
				}

				if( kPacketNot.m_vecInventorySlotInfo.empty() == false )
				{
					GetRoomUserInfo( kPacketNot.m_kRoomUserInfo );
					SendToCnRoom( ERM_CHANGE_EQUIPPED_ITEM_IN_ROOM_NOT, kPacketNot );
				}
			}
			break;

		case CXSLRoom::RT_SQUARE:
			{
				KERM_CHANGE_EQUIPPED_ITEM_IN_SQUARE_NOT kPacketNot;
				kPacketNot.m_UnitUID = GetCharUID();
				std::vector< KInventoryItemInfo >::const_iterator vit;
				for( vit = vecInventorySlotInfo.begin(); vit != vecInventorySlotInfo.end(); ++vit )
				{
					if( vit->m_cSlotCategory == CXSLInventory::ST_E_EQUIP )
					{
						kPacketNot.m_vecInventorySlotInfo.push_back( *vit );
					}
				}

				if( kPacketNot.m_vecInventorySlotInfo.empty() == false )
				{
					//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
#ifdef SERV_TITLE_DATA_SIZE
					kPacketNot.m_iEquippedTitleID = m_kUserTitleManager.GetEquippedTitleID();
#else
					kPacketNot.m_sEquippedTitleID = m_kUserTitleManager.GetEquippedTitleID();
#endif SERV_TITLE_DATA_SIZE
					//}}
					SendToCnRoom( ERM_CHANGE_EQUIPPED_ITEM_IN_SQUARE_NOT, kPacketNot );
				}
			}
			break;

		default:
			{
				START_LOG( cerr, L"�̻��� �� Ÿ��." )
					<< BUILD_LOG( CXSLRoom::GetRoomType( GetRoomUID() ) )
					<< BUILD_LOG( GetCharUID() )
					<< END_LOG;
			}
			break;
		}
	}    
}
//}}

//{{ 2010. 02. 10  ������	���� �̺�Ʈ ����Ʈ
#ifdef SERV_DAY_QUEST

void KGSUser::SendUpdateDropQuestItemByIngQuest()
{
	// ������̰� pvp & dungeon ���̸� ����������Ʈ �Ѵ�.
	if( GetStateID() == KGSFSM::S_ROOM )
	{
		switch( CXSLRoom::GetRoomType( GetRoomUID() ) )
		{
		case CXSLRoom::RT_DUNGEON:
			//{{ �ʵ� ��� ���� - ��μ�
#ifdef SERV_REFORM_ITEM_DROP
		case CXSLRoom::RT_BATTLE_FIELD:
#endif SERV_REFORM_ITEM_DROP
			//}}
			{
				//Update Server quest item data
				KERM_SET_QUEST_ITEM_INFO_NOT kInfo;

				//{{ 2010. 10. 26	������	����Ʈ ���� �߰�
#ifdef SERV_QUEST_CLEAR_EXPAND
				m_kUserQuestManager.GetOngoingQuestForRoom( GetThisPtr<KGSUser>(), kInfo.m_mapDropQuestItembyIngQuest );
#else
				m_kUserQuestManager.GetDropQuestitembyIngQuest( kInfo.m_mapDropQuestItembyIngQuest, GetThisPtr<KGSUser>() );
#endif SERV_QUEST_CLEAR_EXPAND
				//}}

				SendToCnRoom( ERM_SET_QUEST_ITEM_INFO_NOT, kInfo );
			}
			break;
		}
	}
}

#endif SERV_DAY_QUEST
//}}

//{{ 2010. 8. 19	������	�� �ý���
#ifdef SERV_PET_SYSTEM

void KGSUser::SendPetAction( IN CXSLPetManager::PET_ACTION_COMMAND ePetAction )
{
	//////////////////////////////////////////////////////////////////////////
	// �������� ������ üũ�ؼ� ������!
	if( GetStateID() == KGSFSM::S_FIELD_MAP )
	{
		if( GetFieldUID() <= 0 )
		{
			KEGS_PET_ACTION_NOT kNot;
			kNot.m_cActionType = ePetAction;
			kNot.m_iUnitUID = GetCharUID();
			SendPacket( EGS_PET_ACTION_NOT, kNot );
		}
		else
		{
			KERM_PET_ACTION_NOT kNot;
			kNot.m_cActionType = ePetAction;
			kNot.m_iUnitUID = GetCharUID();
			SendToGSField( ERM_PET_ACTION_NOT, kNot );
		}
	}
	else if( GetStateID() == KGSFSM::S_ROOM )
	{
		if( GetRoomUID() <= 0 )
		{
			START_LOG( cerr, L"�� UID �̻�." )
				<< BUILD_LOG( GetRoomUID() )
				<< BUILD_LOG( GetCharUID() )
				<< END_LOG;

			return;
		}

		switch( CXSLRoom::GetRoomType( GetRoomUID() ) )
		{
		case CXSLRoom::RT_PVP:
		case CXSLRoom::RT_DUNGEON:
		case CXSLRoom::RT_TRAININGCENTER:
			//{{ 2012. 10. 23	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
		case CXSLRoom::RT_BATTLE_FIELD:
#endif SERV_BATTLE_FIELD_SYSTEM
			//}}
			{
				KERM_PET_ACTION_NOT kNot;
				kNot.m_cActionType = ePetAction;
				kNot.m_iUnitUID = GetCharUID();
				SendToCnRoom( ERM_PET_ACTION_NOT, kNot );
			}
			break;

		case CXSLRoom::RT_SQUARE:
			// �����̸� �Ⱥ�����!
			break;

		default:
			{
				START_LOG( cerr, L"�̻��� �� Ÿ��." )
					<< BUILD_LOG( CXSLRoom::GetRoomType( GetRoomUID() ) )
					<< BUILD_LOG( GetCharUID() )
					<< END_LOG;
			}
			break;
		}
	}
	else
	{
		START_LOG( cerr, L"�̻��� ���¿��� �� �׼��� �����Ϸ��� �մϴ�." )
			<< BUILD_LOG( GetStateIDString() )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOG( ePetAction )
			<< END_LOG;
	}
}

#endif SERV_PET_SYSTEM
//}}

//{{ 2013. 05. 29	������	���� ����Ʈ ����
#ifdef SERV_BLOCK_LIST
void KGSUser::SendCheckBlockUser( IN const KBlockInfo::BLOCK_TYPE eBlockType )
{
	std::vector< int > vecBlockType;
	vecBlockType.push_back( eBlockType );
	SendCheckBlockUser( vecBlockType );
}

void KGSUser::SendCheckBlockUser( IN const std::vector< int >& vecBlockType )
{
	KEGB_CHECK_BLOCK_USER_NOT kNot;
	kNot.m_vecBlockType = vecBlockType;
	kNot.m_iUserUID = GetUID();
	kNot.m_iUnitUID = GetCharUID();
	kNot.m_wstrIP = KncUtil::toWideString( GetIPStr() );
	SendToGlobalServer( EGB_CHECK_BLOCK_USER_NOT, kNot );
}
#endif SERV_BLOCK_LIST
//}}

//{{ 2013. 06. 03	������	������ ����
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
void KGSUser::SendEnchantNotice( IN const int iItemID, IN const int iEnchantLevel, IN const int iRandomItemID /*= 0 */) const
{
#ifdef SERV_EVENT_ENCHANT_RANDOM_CUBE_NO_NOTIFY
	switch(iRandomItemID)
	{
	case 60007610:
	case 60007611:
	case 60007612:
	case 60007613:
	case 60007614:
		return ;
	}
#endif SERV_EVENT_ENCHANT_RANDOM_CUBE_NO_NOTIFY
	// 9�� �̻� ��ȭ ������ ��ü ����!
	if( iEnchantLevel >= 9 )
	{
		const CXSLItem::ItemTemplet* pItemTemplet = SiCXSLItemManager()->GetItemTemplet( iItemID );
		if( pItemTemplet != NULL )
		{
			KEGS_NOTIFY_MSG_NOT	kPacket;
			kPacket.m_cNotifyType = KEGS_NOTIFY_MSG_NOT::NT_DROP_GET_ITEM; // ������ ȹ�� ����
			kPacket.m_Count			= 1;
#ifdef SERV_UNITED_SERVER_EU
			kPacket.m_wstrMSG		+= boost::str( boost::wformat( L"%s;+%d; %s" ) % GetCharName() % iEnchantLevel % pItemTemplet->m_ItemID );
#else //SERV_UNITED_SERVER_EU
			kPacket.m_wstrMSG		+= boost::str( boost::wformat( L"%s;+%d %s" ) % GetCharName() % iEnchantLevel % pItemTemplet->m_Name );
#endif //SERV_UNITED_SERVER_EU
			kPacket.m_iRandomItem	= iRandomItemID;
			SendToCnServer( ECN_NOTIFY_MSG_NOT, kPacket );
		}
		else
		{
			START_LOG( cerr, L"�������� �ʴ� �������� ��ȭ�ߴ�? ���� ������ ����!" )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOG( iItemID )
				<< END_LOG;
		}
	}
}
#endif SERV_NEW_ITEM_SYSTEM_2013_05
//}}

//{{ 2013. 09. 23	������	�Ϻ� �̺�Ʈ �߰�DB�۾�
#ifdef SERV_RELAY_DB_CONNECTION
void KGSUser::SendUpdateUnitInfoToRelayDB( IN const int iUpdateType, IN const bool bDeletedUnit /*= false*/ ) const
{
	KDBE_UPDATE_USER_INFO_TO_RELAY_DB_JP_EVENT_NOT kPacketToDB;
	kPacketToDB.m_wstrUserID	= GetName();
	kPacketToDB.m_iUserUID		= GetUID();
	kPacketToDB.m_iUnitUID		= GetCharUID();
	kPacketToDB.m_wstrNickName	= GetCharName();
	kPacketToDB.m_cLevel		= GetLevel();
	kPacketToDB.m_cUnitClass	= GetUnitClass();
	kPacketToDB.m_bDeletedUnit	= bDeletedUnit;
	kPacketToDB.m_wstrUserRegDate = m_kUserStatistics.GetAccountRegDate();	// ���� ���� ��¥
	kPacketToDB.m_wstrUnitRegDate = m_kUserStatistics.GetUnitCreateDate();	// ĳ���� ���� ��¥
	kPacketToDB.m_iUpdateType	= iUpdateType;
	SendToRelayDB( DBE_UPDATE_USER_INFO_TO_RELAY_DB_JP_EVENT_NOT, kPacketToDB );
}
#endif SERV_RELAY_DB_CONNECTION
//}}

//{{ 2011. 03. 02	������	���� �ŷ� ����
#ifdef SERV_HACK_USER_TRADE_BLOCK
void KGSUser::HackUserRegRejectedUser( IN const char cReason
#ifdef SERV_PERIOD_ACCOUNT_BLOCK// �۾���¥: 2013-05-27	// �ڼ���
									 , IN const char cPeriodUAL /*= 0*/
									 , IN const std::wstring& wstrBlockReason /*= L""*/
#endif // SERV_PERIOD_ACCOUNT_BLOCK
									 )
{
	// �� ���� ���� ���� �ڵ����
	if( SiKGameSysVal()->IsHackUserRegRejected() == false )
		return;

	if( SEnum::UAL_DEVELOPER <= GetAuthLevel() )
	{
		START_LOG( cout, L"���� ������ �����ڱ� �̻��̹Ƿ� ���� ��ġ�� ������ �ʽ��ϴ�!" )
			<< BUILD_LOG( GetName() )
			<< BUILD_LOG( GetUID() )
			<< BUILD_LOGc( GetAuthLevel() )
			<< BUILD_LOG( GetCharName() )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOG( cReason )
#ifdef SERV_PERIOD_ACCOUNT_BLOCK// �۾���¥: 2013-05-27	// �ڼ���
			<< BUILD_LOG( cPeriodUAL )
			<< BUILD_LOG( wstrBlockReason )
#endif // SERV_PERIOD_ACCOUNT_BLOCK
		;
		return;
	}

	// RejectedUser�� ����ϱ�!
	KELG_REG_REJECTED_USER_NOT kPacketNot;
	kPacketNot.m_iUserUID			= GetUID();
	kPacketNot.m_cRejectedReason	= cReason;
#ifdef SERV_PERIOD_ACCOUNT_BLOCK// �۾���¥: 2013-05-27	// �ڼ���
	kPacketNot.m_cPeriodUAL			= cPeriodUAL;
	kPacketNot.m_wstrBlockReason	= wstrBlockReason;
#endif // SERV_PERIOD_ACCOUNT_BLOCK
	SendToLoginServer( ELG_REG_REJECTED_USER_NOT, kPacketNot );

	START_LOG( cout, L"������ ������ ������ �Ǵ�! Rejected User�� ���!" )
#ifdef SERV_PRIVACY_AGREEMENT
		<< BUILD_LOG( GetUID() )
		<< BUILD_LOG( GetCharUID() )
#else
		<< BUILD_LOG( GetUserName() )
		<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
		<< BUILD_LOGc( cReason )
#ifdef SERV_PERIOD_ACCOUNT_BLOCK// �۾���¥: 2013-05-27	// �ڼ���
		<< BUILD_LOGc( cPeriodUAL )
		<< BUILD_LOG( wstrBlockReason )
#endif // SERV_PERIOD_ACCOUNT_BLOCK
		;
}

void KGSUser::HackUserRegTradeBlock( IN const char cReason )
{
	// ���� �ŷ� ���� ��� ����
	if( SiKGameSysVal()->IsHackUserBlockTrade() == false )
		return;

	// �̹� �ŷ� ���� �������� Ȯ��
	if( m_kUserAbuserManager.IsTradeBlockUser() == true )
		return;
	
	// TradeBlock�� ����ϱ�!
	KELG_REG_REJECTED_USER_NOT kPacketNot;
	kPacketNot.m_iUserUID = GetUID();
	kPacketNot.m_cRejectedReason = cReason;
	SendToLoginServer( ELG_REG_TRADE_BLOCK_USER_NOT, kPacketNot );

	// ���ΰŷ� ����!
	m_kUserAbuserManager.SetTradeBlockUser( true );

	START_LOG( cout, L"������ ������ ������ �Ǵ�! Trade Block User�� ���!" )
#ifdef SERV_PRIVACY_AGREEMENT
		<< BUILD_LOG( GetUID() )
		<< BUILD_LOG( GetCharUID() )
#else
		<< BUILD_LOG( GetUserName() )
		<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
		<< BUILD_LOGc( cReason );
}

void KGSUser::HackUserRegMonitorsUser( IN const char cReason )
{
	// ������ ���� ��� �ڵ� ���
	if( SiKGameSysVal()->IsHackUserRegMonitors() == false )
		return;

	if( m_cHackingUserType != SEnum::HUT_NORMAL )
		return;

	START_LOG( clog, L"��ŷ ���� �Ű�.!" )
		<< BUILD_LOG( GetUID() )
#ifdef SERV_PRIVACY_AGREEMENT
		<< BUILD_LOG( GetUID() )
		<< BUILD_LOG( GetCharUID() )
#else
		<< BUILD_LOG( GetUserName() )
		<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
		<< BUILD_LOGc( m_cHackingUserType )
		;

	// �������� üũ!
	m_cHackingUserType = SEnum::HUT_DISAGREE_HACK_USER;

	// AccountDB���� �������!
	SendToAccountDB( DBE_REPORT_HACK_USER_NOT, GetUID() );

	START_LOG( cout, L"������ ������ ������ �Ǵ�! ��ŷ �ǽ� ������ ���!" )
#ifdef SERV_PRIVACY_AGREEMENT
		<< BUILD_LOG( GetUID() )
		<< BUILD_LOG( GetCharUID() )
#else
		<< BUILD_LOG( GetUserName() )
		<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
		<< BUILD_LOGc( cReason );
}

//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
void KGSUser::HackUserKick( IN const KEGS_SERVER_CHECK_HACK_USER_REQ& kPacket )
{
	// ������ ���� ���� ����
	if( SiKGameSysVal()->IsHackUserKick() == false )
		return;

	SendPacket( EGS_SERVER_CHECK_HACK_USER_REQ, kPacket );

	//{{ 2010. 07. 09  ������	������ ����ó��
	SetDisconnectReason( KStatistics::eSIColDR_Client_Hacking );
	ReserveDestroy();
	//}}

	START_LOG( cout, L"������ ������ ������ �Ǵ�! ���� ���� ó��!" )
		<< BUILD_LOG( GetUserName() )
		<< BUILD_LOG( GetCharName() )
		<< BUILD_LOGc( kPacket.m_cHackingCheckType );
}
#else
void KGSUser::HackUserKick( IN const KEGS_SERVER_CHECK_HACK_USER_NOT& kPacket )
{
	// ������ ���� ���� ����
	if( SiKGameSysVal()->IsHackUserKick() == false )
		return;

	SendPacket( EGS_SERVER_CHECK_HACK_USER_NOT, kPacket );

	//{{ 2010. 07. 09  ������	������ ����ó��
	SetDisconnectReason( KStatistics::eSIColDR_Client_Hacking );
	ReserveDestroy();
	//}}

	START_LOG( cout, L"������ ������ ������ �Ǵ�! ���� ���� ó��!" )
#ifdef SERV_PRIVACY_AGREEMENT
		<< BUILD_LOG( GetUID() )
		<< BUILD_LOG( GetCharUID() )
#else
		<< BUILD_LOG( GetUserName() )
		<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
		<< BUILD_LOGc( kPacket.m_cHackingCheckType );
}
#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
//}}
#endif SERV_HACK_USER_TRADE_BLOCK
//}}

//{{ 2011. 09. 08	������	������ ��Ŷ ����͸�
#ifdef SERV_AUTO_HACK_PACKET_MORNITOING
void KGSUser::HackUserRegPacketMornitor()
{
	// ������ ���� ���� ����
	if( SiKGameSysVal()->IsHackUserPacketMornitor() == false )
		return;

	m_kUserAbuserManager.RegPacketMornitoring( GetThisPtr<KGSUser>() );
}
#endif SERV_AUTO_HACK_PACKET_MORNITOING
//}}

//{{ 2009. 9. 7  ������		����Ʈ ��� ����
bool KGSUser::IsDungeonGetExp( int iDungeonID, char cDifficulty )
{
	int nConst = abs( SiCXSLDungeonManager()->GetDungeonMinLevel( static_cast<int>(iDungeonID + cDifficulty) ) - static_cast<int>(GetLevel()) );

	return ( nConst <= 19 ); // ������ 20�� �Ǹ� ����ġ ȹ�� �Ұ� �����̴�!
}
//}}

//{{ 2010. 05. 12  ������	���� ���� ������ ����
#ifdef SERV_INTEGRATION

bool KGSUser::IsSameServerGroupUID( IN UidType iUID )
{
	return ( KncUid::ExtractServerGroupID( iUID ) == KncUid::ExtractServerGroupID( GetCharUID() ) );
}

#endif SERV_INTEGRATION
//}}

#ifdef SERV_PAYMENT_ITEM_ON_GOING_QUEST
bool KGSUser::IsEnoughItemExist( IN const std::map< int, int >& mapRequiredItem, IN const bool bOnlyInventory )
{
	return m_kInventory.IsEnoughItemExist( mapRequiredItem, bOnlyInventory );
}
#endif SERV_PAYMENT_ITEM_ON_GOING_QUEST

//{{ 2010. 01. 11  ������	�����ڵ�����
bool KGSUser::IsEnoughItemExistOnlyInventory( IN const std::map< int, int >& mapRequiredItem )
{
    return m_kInventory.IsEnoughItemExistOnlyInventory( mapRequiredItem );
}

//{{ 2010. 07. 05  ������	���� ����� �Ŵ���
#ifdef SERV_USER_ABUSER_MANAGER

void KGSUser::CheckEDAbuser( IN int iGetEDState, IN int iBeforeED )
{
    m_kUserAbuserManager.CheckEDAbuser( GetThisPtr<KGSUser>(), iGetEDState, iBeforeED );
}

void KGSUser::CheckItemAbuser( IN int iGetItemState, IN const std::map< int, int >& mapGetItemInfo )
{
    m_kUserAbuserManager.CheckItemAbuser( GetThisPtr<KGSUser>(), iGetItemState, mapGetItemInfo );
}

#endif SERV_USER_ABUSER_MANAGER
//}}

void KGSUser::UserStatisticsIncreaseCount( int iTableID, int iSubTableID , int iColumn, int iCount )
{
    m_kUserStatistics.IncreaseCount( iTableID, iSubTableID , iColumn, iCount );

	//{{ 2010. 11. 17	������	������ ���� ���
#ifdef SERV_AUTO_HACK_CHECK_GET_ITEM
	if( iColumn == KUserStatistics::US_Etc_Obtained_Item )
	{
		m_kUserAbuserManager.IncreaseGetItemCount( GetThisPtr<KGSUser>(), iCount );
	}
#endif SERV_AUTO_HACK_CHECK_GET_ITEM
	//}}
}
//}}

//void KGSUser::SetPartyDungeonInfo(int iPartyDungeonID, char cPartyDifficulty)
//{
//	m_iPartyDungeonID	= iPartyDungeonID;
//	m_cPartyDifficulty	= cPartyDifficulty;
//}

//
//void KGSUser::SendUpdatePartyInfo( bool bVerifyParty )
//{
//	if( GetRoomUID() != 0 )
//	{
//		if( GetRoomType( GetRoomUID() ) == CXSLRoom::RT_SQUARE )
//		{
//			SendToCnRoom( ERM_UPDATE_PARTY_INFO_NOT, bVerifyParty );
//		}
//	}
//}

//bool KGSUser::CheckKNPAuthQuery( bool bIsFirst )
//{
//	if( m_pCSA == NULL )
//	{
//		START_LOG( cerr, L"nProtect ��� ������ �̻�.!" )
//			<< BUILD_LOG( GetUID() )
//			<< BUILD_LOG( GetChannelID() )
//			<< END_LOG;
//
//		return false;
//	}
//
//	DWORD dwRet = m_pCSA->GetAuthQuery();
//
//	if( dwRet != ERROR_SUCCESS )
//	{
//		KPacketOK kShowDown;
//		kShowDown.m_iOK = NetError::ERR_KNP_00;
//
//		//����ó��
//		WCHAR			wszError[MAX_PATH] = L"";
//		std::wstring	wstrError;
//
//		wsprintf( wszError, L"[ERRCODE:%d] Query : %08X %08X %08X %08X", 
//							dwRet,
//							m_pCSA->m_AuthQuery.dwIndex, 
//							m_pCSA->m_AuthQuery.dwValue1,
//							m_pCSA->m_AuthQuery.dwValue2,
//							m_pCSA->m_AuthQuery.dwValue3
//							);
//
//		wstrError = wszError;
//
//		START_LOG( cerr, L"nProtect ���� ���� ����.!" )
//			<< BUILD_LOG( GetUID )
//			<< BUILD_LOG( GetCharUID() )
//			<< BUILD_LOG( wstrError )
//			<< END_LOG;
//
//		if( dwRet == 12 || dwRet == 14 )
//		{
//			kShowDown.m_iOK = NetError::ERR_KNP_01;
//		}
//
//		//���� ������ Ŭ���̾�Ʈ���� �˷��־�� �Ѵ�.
//		SendPacket( EGS_KNPROTECT_USER_SHOWDOWN_NOT, kShowDown );
//
//		return false;
//	}
//
//	return true;
//}

//{{ 2012. 03. 20	������	��Ʋ�ʵ� �ý���
//////////////////////////////////////////////////////////////////////////
#ifdef SERV_BATTLE_FIELD_SYSTEM
//////////////////////////////////////////////////////////////////////////
bool KGSUser::CheckingSpirit( IN const int iDungeonID )
{
	// �ټ��� �Ŵ������� �˻�
#ifdef SERV_PC_BANG_TYPE
	bool bSpiritFree = ExistActivateBuff( CXSLBuffManager::BTI_BUFF_PREMIUM_PC_ROOM ) & SiKGameSysVal()->IsSpiritFree( GetPcBangType() );
	return m_kUserSpiritManager.CheckingSpirit( iDungeonID, bSpiritFree );
#else SERV_PC_BANG_TYPE
	return m_kUserSpiritManager.CheckingSpirit( iDungeonID, ExistActivateBuff( CXSLBuffManager::BTI_BUFF_PREMIUM_PC_ROOM )  );
#endif SERV_PC_BANG_TYPE
}

bool KGSUser::DecreaseSpirit( IN const int iDungeonID, IN const char cUserCount, IN const short sStageNpcCount, IN const bool bIsTutorial, IN const bool bIsBattleField )
{
#ifdef GUILD_TEST
	// 4. ��� ������ ��� ����ġ ���
	if( GetGuildUID() > 0 )
	{
		// �ټ��� �̺�Ʈ & PC�� �̺�Ʈ ������ ó�� �մϴ�. �Ҹ��ѰͰ� �����ϰ� ó���ϱ� ���ؼ�..
		// [����] �Ҹ� �ټ��� ���İ���� �������谡 �ƴѰ����� ����ؾ���!
		//{{ 2011. 09. 23	������	������ ���� �ټ��� �Ҹ� �̺�Ʈ
//#ifdef SERV_COEXISTENCE_FESTIVAL
		const int iCalculatedDecreaseSpirit = m_kUserSpiritManager.CalculateDecreaseSpirit( cUserCount, sStageNpcCount, false, false, bIsBattleField );
//#else
//		const int iCalculatedDecreaseSpirit = m_kUserSpiritManager.CalculateDecreaseSpirit( cUserCount, sStageNpcCount, false );
//#endif SERV_COEXISTENCE_FESTIVAL
		//}}

		// �ټ��� �̺�Ʈ & PC�� �̺�Ʈ ������ ó�� �մϴ�. �Ҹ��ѰͰ� �����ϰ� ó���ϱ� ���ؼ�..
		// [����] �Ҹ� �ټ��� ���İ���� �������谡 �ƴѰ����� ����ؾ���!
		m_kUserGuildManager.OnDecreaseSpirit( GetThisPtr<KGSUser>(), iCalculatedDecreaseSpirit );
	}
#endif GUILD_TEST

	//////////////////////////////////////////////////////////////////////////
	// �ټ��� ���� ó��!
#if defined( SERV_LOCAL_RANKING_SYSTEM ) || defined( SERV_CHINA_SPIRIT_EVENT )
	int iAbsoluteDecreaseSpirit = 0;
#endif	// SERV_LOCAL_RANKING_SYSTEM
	int iDecreaseSpirit = 0;
	bool bIsSpiritUpdated = false;
#ifdef SERV_PC_BANG_TYPE
	bool bSpiritFree = IsPcBang() & SiKGameSysVal()->IsSpiritFree( GetPcBangType() );

    m_kUserSpiritManager.DecreaseSpirit( iDungeonID
									   , cUserCount
									   , sStageNpcCount
									   , bIsTutorial
									   , bSpiritFree
									   , bIsBattleField
									   , iDecreaseSpirit
									   , bIsSpiritUpdated
#if defined( SERV_LOCAL_RANKING_SYSTEM ) || defined( SERV_CHINA_SPIRIT_EVENT )
									   , iAbsoluteDecreaseSpirit
#endif	// SERV_LOCAL_RANKING_SYSTEM
									   );	
	
#else SERV_PC_BANG_TYPE
    m_kUserSpiritManager.DecreaseSpirit( iDungeonID
									   , cUserCount
									   , sStageNpcCount
									   , bIsTutorial
									   , IsPcBang()
									   , bIsBattleField
									   , iDecreaseSpirit
									   , bIsSpiritUpdated
#if defined( SERV_LOCAL_RANKING_SYSTEM ) || defined( SERV_CHINA_SPIRIT_EVENT )
									   , iAbsoluteDecreaseSpirit
#endif	// SERV_LOCAL_RANKING_SYSTEM
									   );
#endif SERV_PC_BANG_TYPE

#ifdef SERV_CHINA_SPIRIT_EVENT
	switch( GetEquippedTitleID() )
	{
	case 50120:
		AddChinaSpirit( 0, iAbsoluteDecreaseSpirit );
		break;
	case 50130:
		AddChinaSpirit( 1, iAbsoluteDecreaseSpirit );
		break;
	case 50140:
		AddChinaSpirit( 2, iAbsoluteDecreaseSpirit );
		break;
	case 50150:
		AddChinaSpirit( 3, iAbsoluteDecreaseSpirit );
		break;
	case 50160:
		AddChinaSpirit( 4, iAbsoluteDecreaseSpirit );
		break;
	case 50170:
		AddChinaSpirit( 5, iAbsoluteDecreaseSpirit );
		break;
	default:
		break;
	}
#endif SERV_CHINA_SPIRIT_EVENT

	//{{ 2013. 3. 17	�ڼ���	 ���� ��ŷ �ý���
#ifdef SERV_LOCAL_RANKING_SYSTEM
	if( 0 < iAbsoluteDecreaseSpirit )
	{
		IncreasedLocalRankingPoint( iAbsoluteDecreaseSpirit, 0 );
	}
#endif SERV_LOCAL_RANKING_SYSTEM
	//}}

	if( iDecreaseSpirit > 0 )
	{
		m_kUserStatistics.IncreaseCount( KUserStatistics::USTable_EtcData, 0, KUserStatistics::US_Etc_Used_Spirit, iDecreaseSpirit );

		// local log
		KStatisticsKey kKey;
		kKey.m_vecIntKey.push_back( (int) GetLevel() );
		KSIManager.IncreaseCount( KStatistics::SI_LOC_SPIRIT, kKey, KStatistics::eSIColSpirit_Play, 1 );
		KSIManager.IncreaseCount( KStatistics::SI_LOC_SPIRIT, kKey, KStatistics::eSIColSpirit_UseSpirit, iDecreaseSpirit );

		//{{ 2010. 06. 30  ������	�ټ��� ���
#ifdef SERV_SPIRIT_STATISTICS
		m_kUserStatistics.AddUsedSpirit( iDecreaseSpirit );
#endif SERV_SPIRIT_STATISTICS
		//}}
	}

	if( bIsSpiritUpdated )
	{
		m_kUserStatistics.IncreaseCount( KUserStatistics::USTable_EtcData, 0, KUserStatistics::US_Etc_AllUsed_Spirit_Count,	1 );

		// local log
		KStatisticsKey kKey;
		kKey.m_vecIntKey.push_back( (int) GetLevel() );
		KSIManager.IncreaseCount( KStatistics::SI_LOC_SPIRIT, kKey, KStatistics::eSIColSpirit_SpiritZero, 1 );
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////
#else
//////////////////////////////////////////////////////////////////////////
bool KGSUser::CheckingSpirit( IN const int iDungeonID )
{
	// �ټ����� ���þ��� �����̸� �ٷ� ����ó��
	if( CXSLDungeon::IsTutorialDungeon( iDungeonID )  ||
		CXSLDungeon::IsEventDungeon( iDungeonID ) )
	{
		return true;
	}

	//{{ 2010. 12. 30	������	�ϸ� ���� �߰�
#ifdef SERV_SHIP_DUNGEON
	if( CXSLDungeon::IsShipDungeon( iDungeonID ) )
	{
		return true;
	}
#endif SERV_SHIP_DUNGEON
	//}}

	//{{ 2009. 7. 3  ������		��ϸ� �ð�
	if( SiCXSLDungeonManager()->GetDungeonType( iDungeonID ) == CXSLDungeon::DT_HENIR )
	{
		return true;
	}
	//}}

	// [�̺�Ʈ] 080611.hoons.�ټ��� �̺�Ʈ ���ϰ��..
	if( SiKGameEventManager()->CheckSpiritEvent() == true )
		return true;
	//}}

	//{{ 2009. 10. 26  ������	PC�� ����
	if( IsPcBang() == true )
	{
		return true;
	}
	//}}

	// �ټ����� �����ִ��� �˻�
	if( m_iSpirit > 0 )
		return true;

	return false;
}

bool KGSUser::DecreaseSpirit( IN const int iDungeonID, IN const char cUserCount, IN const short sStageNpcCount, IN const bool bIsTutorial )
{	
	// 1. �ټ��� �Ҹ�ʹ� ���� ������� ������ ��츦 üũ�Ͽ� return��.
	if( CXSLDungeon::IsTutorialDungeon( iDungeonID )  ||
		CXSLDungeon::IsEventDungeon( iDungeonID ) )
	{
		return false;
	}

	//{{ 2010. 12. 30	������	�ϸ� ���� �߰�
#ifdef SERV_SHIP_DUNGEON
	if( CXSLDungeon::IsShipDungeon( iDungeonID ) )
	{
		return false;
	}
#endif SERV_SHIP_DUNGEON
	//}}

	// 2. ��ϸ� �ð��� �ټ����ʹ� ���� ����.
	if( SiCXSLDungeonManager()->GetDungeonType( iDungeonID ) == CXSLDungeon::DT_HENIR )
	{
		return false;
	}

	// 3. ���������� npc�� �Ѹ����� ���ٸ� �ټ��� �Ҹ� ����
	if( sStageNpcCount <= 0 )
	{
		return false;
	}
	
#ifdef GUILD_TEST
	// 4. ��� ������ ��� ����ġ ���
	if( GetGuildUID() > 0 )
	{
		// �ټ��� �̺�Ʈ & PC�� �̺�Ʈ ������ ó�� �մϴ�. �Ҹ��ѰͰ� �����ϰ� ó���ϱ� ���ؼ�..
		// [����] �Ҹ� �ټ��� ���İ���� �������谡 �ƴѰ����� ����ؾ���!
		//{{ 2011. 09. 23	������	������ ���� �ټ��� �Ҹ� �̺�Ʈ
#ifdef SERV_COEXISTENCE_FESTIVAL
		m_kUserGuildManager.OnDecreaseSpirit( GetThisPtr<KGSUser>(), CalculateDecreaseSpirit( cUserCount, sStageNpcCount, false, false ) );
#else
		m_kUserGuildManager.OnDecreaseSpirit( GetThisPtr<KGSUser>(), CalculateDecreaseSpirit( cUserCount, sStageNpcCount, false ) );
#endif SERV_COEXISTENCE_FESTIVAL
		//}}
	}
#endif GUILD_TEST	

	// 5. �������谡 �ƴѰ�� �ټ��� �̺�Ʈ�� PC������ �˻�
	if( bIsTutorial == false )
	{
		// [�̺�Ʈ] 080611.hoons.�ټ��� �̺�Ʈ ���ϰ��..
		if( SiKGameEventManager()->CheckSpiritEvent() == true )
		{
			return false;
		}

		//{{ 2009. 10. 26  ������	PC�� ����
		if( IsPcBang() == true )
		{
			return false;
		}
		//}}
	}

	// 6. �Ҹ��ų �ټ��� ���
	//{{ 2011. 09. 23	������	������ ���� �ټ��� �Ҹ� �̺�Ʈ
#ifdef SERV_COEXISTENCE_FESTIVAL
	int iDecreaseSpirit = CalculateDecreaseSpirit( cUserCount, sStageNpcCount, bIsTutorial, true ); // [����] ���������ϰ�� �ټ����� ����ȴ�.
#else
	int iDecreaseSpirit = CalculateDecreaseSpirit( cUserCount, sStageNpcCount, bIsTutorial ); // [����] ���������ϰ�� �ټ����� ����ȴ�.
#endif SERV_COEXISTENCE_FESTIVAL
	//}}	
	int iRemain = m_iSpirit - iDecreaseSpirit;
	if( iRemain < 0 )
	{
		iDecreaseSpirit += iRemain;
	}

	// �ټ��� �Ҹ� ( ���������ϰ�� ����ó�� )
	m_iSpirit -= iDecreaseSpirit;

	//////////////////////////////////////////////////////////////////////////
	// ���
	if( iDecreaseSpirit > 0 )
	{
		m_kUserStatistics.IncreaseCount( KUserStatistics::USTable_EtcData, 0, KUserStatistics::US_Etc_Used_Spirit, iDecreaseSpirit );

		// local log
		KStatisticsKey kKey;
		kKey.m_vecIntKey.push_back( (int) GetLevel() );
		KSIManager.IncreaseCount( KStatistics::SI_LOC_SPIRIT, kKey, KStatistics::eSIColSpirit_Play, 1 );
		KSIManager.IncreaseCount( KStatistics::SI_LOC_SPIRIT, kKey, KStatistics::eSIColSpirit_UseSpirit, iDecreaseSpirit );

		//{{ 2010. 06. 30  ������	�ټ��� ���
#ifdef SERV_SPIRIT_STATISTICS
		m_kUserStatistics.AddUsedSpirit( iDecreaseSpirit );
#endif SERV_SPIRIT_STATISTICS
		//}}
	}

	// �ټ��� ũ�� ����
	if( m_iSpirit <= 0 )
	{
		m_iSpirit = 0;

		// �ټ��� �Ϸ�ġ ��� �Ҹ�� ī��Ʈ [�ѹ� ī��Ʈ �ö󰡸� �ٽ� ���ž���]
		if( m_bIsSpiritUpdated == false )
		{
			m_kUserStatistics.IncreaseCount( KUserStatistics::USTable_EtcData, 0, KUserStatistics::US_Etc_AllUsed_Spirit_Count,	1 );

			// local log
			KStatisticsKey kKey;
			kKey.m_vecIntKey.push_back( (int) GetLevel() );
			KSIManager.IncreaseCount( KStatistics::SI_LOC_SPIRIT, kKey, KStatistics::eSIColSpirit_SpiritZero, 1 );

			m_bIsSpiritUpdated = true;
		}
	}
	else if( m_iSpirit > SiKSpiritTable()->GetMaxSpirit() )
	{
		m_iSpirit = SiKSpiritTable()->GetMaxSpirit();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
//{{ 2011. 09. 23	������	������ ���� �ټ��� �Ҹ� �̺�Ʈ
//#ifdef SERV_COEXISTENCE_FESTIVAL
int	KGSUser::CalculateDecreaseSpirit( IN const char cUserCount, IN const short sStageNpcCount, IN const bool bIsTutorial, IN const bool bHalfDecreaseEvent )
{
	//////////////////////////////////////////////////////////////////////////
	// �ټ��� ����
	//
	// Y = (42-(2*X+A))/�ش� �������� ���� ��*B
	// ( ���Ͱ� �Ѹ��� ���� �� ��� ��Ƽ���� �ټ��� ���ҷ� = Y )
	// Y �� 1 �� ���, Y = 1�� ó�� �Ѵ�. 

	if( sStageNpcCount <= 0 )
		return 0;

	float fDecreaseSpirit = 0.0f;

	if( bHalfDecreaseEvent )
	{
		fDecreaseSpirit = (float)( 22 - ( 2 * static_cast<int>(cUserCount) + ( bIsTutorial ? 26 : 0 ) ) ) / (float)sStageNpcCount;
	}
	else
	{
		fDecreaseSpirit = (float)( 42 - ( 2 * static_cast<int>(cUserCount) + ( bIsTutorial ? 46 : 0 ) ) ) / (float)sStageNpcCount;
	}

	if( 0.f < fDecreaseSpirit  &&  fDecreaseSpirit < 1.f )
	{
		fDecreaseSpirit = 1.f;
	}
	else if( 0.f > fDecreaseSpirit  &&  fDecreaseSpirit > -1.f  )
	{
		fDecreaseSpirit = -1.f;
	}

	return static_cast<int>(fDecreaseSpirit);
}
//#else
//int	KGSUser::CalculateDecreaseSpirit( IN const char cUserCount, IN const short sStageNpcCount, IN const bool bIsTutorial )
//{
//	//////////////////////////////////////////////////////////////////////////
//	// �ټ��� ����
//	//
//	// Y = (42-(2*X+A))/�ش� �������� ���� ��*B
//	// ( ���Ͱ� �Ѹ��� ���� �� ��� ��Ƽ���� �ټ��� ���ҷ� = Y )
//	// Y �� 1 �� ���, Y = 1�� ó�� �Ѵ�. 
//
//	if( sStageNpcCount <= 0 )
//		return 0;
//	
//	float fDecreaseSpirit = (float)( 42 - ( 2 * static_cast<int>(cUserCount) + ( bIsTutorial ? 46 : 0 ) ) ) / (float)sStageNpcCount;
//
//	if( 0.f < fDecreaseSpirit  &&  fDecreaseSpirit < 1.f )
//	{
//		fDecreaseSpirit = 1.f;
//	}
//	else if( 0.f > fDecreaseSpirit  &&  fDecreaseSpirit > -1.f  )
//	{
//		fDecreaseSpirit = -1.f;
//	}
//
//	return static_cast<int>(fDecreaseSpirit);
//}
//#endif SERV_COEXISTENCE_FESTIVAL
//}}
//////////////////////////////////////////////////////////////////////////
#endif SERV_BATTLE_FIELD_SYSTEM
//////////////////////////////////////////////////////////////////////////
//}}

//{{ 2011. 06. 20	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
bool KGSUser::CheckDungeonPartyCondition( IN const int iDungeonID,
										  IN const char cDifficultyLevel,
										  IN const char cDungeonMode,
										  IN const bool bCheckLowLevel,
										  IN const bool bDontCheckPartyUID )
{
	SET_ERROR( NET_OK );

	////////////////////////////////////////////////////////////////////////// ������ ����
	// DungeonID�˻�
	const int iDungeonIDAndDif = ( iDungeonID + static_cast<int>(cDifficultyLevel) );
	if( iDungeonIDAndDif <= 0 )
	{
		START_LOG( cerr, L"dungeonid���� �̻��ϴ�!" )
			<< BUILD_LOG( iDungeonID )
			<< BUILD_LOGc( cDifficultyLevel )
			<< END_LOG;
	}

	//���� ���̵� �������� üũ�Ѵ�.

#ifdef SERV_NEW_EVENT_TYPES
	bool bIsEventDungeon = SiCXSLDungeonManager()->IsSwitchingWithEventInfo( iDungeonID );
	if( !SiKGameEventManager()->IsEnableDungeon( iDungeonID, bIsEventDungeon ) )
	{
		START_LOG( cerr, L"�����ִ� �̺�Ʈ DungeonID�Դϴ�. Ŭ�󿡼� �˻������ٵ�.." )
			<< BUILD_LOG( GetCharName() )
			<< BUILD_LOG( iDungeonID )
			<< BUILD_LOGc( cDifficultyLevel )
			<< END_LOG;

		SET_ERROR( ERR_PARTY_22 );
		return false;
	}
#endif SERV_NEW_EVENT_TYPES


	//{{ 2012. 03. 03	�ڼ���	����� �� ����
#ifdef SERV_REFORM_THE_GATE_OF_DARKNESS
	if( CXSLDungeon::IsDefenceDungeon( iDungeonID ) == true )
	{
		int iWorldCrystalNum = SiKGSWorldMissionManager()->GetWorldCrystalCount();
		int iWorldCrystalMin, iWorldCrystalMax;

		switch( cDifficultyLevel )
		{
		case CXSLDungeon::DL_NORMAL:	iWorldCrystalMin = 0; iWorldCrystalMax = 6; break;
		case CXSLDungeon::DL_HARD:		iWorldCrystalMin = 7; iWorldCrystalMax = 12; break;
		case CXSLDungeon::DL_EXPERT:	iWorldCrystalMin = 13; iWorldCrystalMax = 15; break;
		}

		if( ( iWorldCrystalNum < iWorldCrystalMin ) || ( iWorldCrystalMax < iWorldCrystalNum ) )
		{
			SET_ERROR( ERR_PARTY_22 );
			return false;
		}
	}
#endif SERV_REFORM_THE_GATE_OF_DARKNESS
	//}}

	//{{ 2009. 10. 16  ������	����
	if( false == CXSLDungeon::IsRubenDungeon( iDungeonID )  &&
		false == CXSLDungeon::IsElderDungeon( iDungeonID )  &&
		false == CXSLDungeon::IsBesmaDungeon( iDungeonID )  &&
		false == CXSLDungeon::IsAlteraDungeon(iDungeonID )  &&
		false == CXSLDungeon::IsPeitaDungeon( iDungeonID )  &&
		false == CXSLDungeon::IsVelderDungeon( iDungeonID ) &&
		false == CXSLDungeon::IsHamelDungeon( iDungeonID )
		//{{ 2012. 10. 29	�ڼ���	���� ���� �߰�
#ifdef SERV_VILLAGE_SANDER
		&& false == CXSLDungeon::IsSanderDungeon( iDungeonID )
#endif SERV_VILLAGE_SANDER
		//}}
		)
	{
		SET_ERROR( ERR_PARTY_22 );
		return false;
	}
	//}}

	//��ϸ�������尡 �������� üũ�Ѵ�.
	if( CXSLDungeon::IsValidDungeonMode( static_cast<CXSLDungeon::DUNGEON_MODE>(cDungeonMode) ) == false )
	{
		START_LOG( cerr, L"���� ��尪�� �̻��մϴ�!" )
			<< BUILD_LOGc( cDungeonMode )
			<< END_LOG;

		SET_ERROR( ERR_PARTY_22 );
		return false;
	}

	////////////////////////////////////////////////////////////////////////// ���� ����
	//{{ 2009. 8. 5  ������		ü��ID����
	if( IsGuestUser()  &&  CXSLDungeon::IsHenirDungeon( iDungeonID ) )
	{
		SET_ERROR( ERR_GUEST_USER_00 );
		return false;
	}
	//}}

	//�̹� ��Ƽ�� �ִ°��..
	if( bDontCheckPartyUID == false )
	{
		if( GetPartyUID() != 0 )
		{
			SET_ERROR( ERR_PARTY_12 );
			return false;
		}
	}

	//���� �κ� �ִ°��.
	if( GetUnitState() == CXSLUnit::CUS_PVP_LOBBY )
	{
		SET_ERROR( ERR_PARTY_13 );
		return false;
	}

	//::1. �ټ��� �˻�.
	if( CheckingSpirit( iDungeonID ) == false )
	{
		SET_ERROR( ERR_ROOM_27 );
		return false;
	}

	//::2. �����ʿ� �����۰˻�.
	int iRequireItemID = 0;
	int iRequireQuantity = 0;

	if( CheckRequiredItemToEnterDungeonRoom( iDungeonIDAndDif, (CXSLDungeon::DUNGEON_MODE) cDungeonMode, iRequireItemID, iRequireQuantity ) == false )
	{
		//{{ 2010. 05. 06  ������	���� �˾�
		KEGS_ERROR_POPUP_NOT kNot;
		kNot.m_iOK = NetError::ERR_DUNGEON_REQUIRE_ITEM_00;
		kNot.m_iErrorData1 = iRequireItemID;
		kNot.m_iErrorData2 = iRequireQuantity;
		SendPacket( EGS_ERROR_POPUP_NOT, kNot );
		//}}

		SET_ERROR( ERR_DUNGEON_REQUIRE_ITEM_00 );
		return false;
	}

#ifdef SERV_LIMITED_DUNGEON_PLAY_TIMES
	if( CheckLimitedPlayTimes( iDungeonIDAndDif ) == false )
	{
		SET_ERROR( ERR_DUNGEON_ENTER_LIMIT_00 );
		return false;
	}
#endif SERV_LIMITED_DUNGEON_PLAY_TIMES

	//��� �̻��� �ƴϸ� üũ�Ѵ�.
	if( GetAuthLevel() < SEnum::UAL_GM )
	{
		//::3. ������� Ŭ��� ���� �ʾ���.
		const int iRequireDungeonID = SiCXSLDungeonManager()->GetRequireDungeonID( iDungeonID );
		if( iRequireDungeonID != 0 )
		{
			std::map<int,KDungeonClearInfo>::iterator mit;
			mit = m_mapDungeonClear.find( iRequireDungeonID );
			if( mit == m_mapDungeonClear.end() )
			{
				SET_ERROR( ERR_ROOM_06 );
				return false;
			}
		}

		//{{ 2010. 12. 30	������	�ϸ� ���� �߰�
#ifdef SERV_SHIP_DUNGEON
		if( CXSLDungeon::IsShipDungeon( iDungeonID ) == true )
		{
			if( CXSLDungeon::IsVelderDungeon( iDungeonID ) == true )
			{
				// �ش� map�� �� �� �ִ��� ���� üũ
				//{{ 2012. 09. 21   ��μ�   ���� ���� ���� �޽��� ����
#ifdef SERV_ENTER_VILLAGE_ERROR
				int iTempLevel = 0;
				int iTempDungeonID = 0;
				if( CheckEnterTheVillage( CXSLMapData::MI_HAMEL, iTempLevel, iTempDungeonID ) == false )
#else
				if( CheckEnterTheVillage( CXSLMapData::MI_HAMEL ) == false )
#endif SERV_ENTER_VILLAGE_ERROR
				//}}
				{
					START_LOG( cwarn, L"������ ������ �� �ִ� ������ �ȵ�." )
						<< BUILD_LOG( GetCharUID() )
						<< BUILD_LOG( iDungeonID )
						<< END_LOG;

					// GetLastError
					return false;
				}
			}
		}
#endif SERV_SHIP_DUNGEON
		//}}
	}
	else
	{
		START_LOG( cout, L"��� ��Ƽ ���� �õ�!" )
#ifdef SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetCharUID() )
#else
			<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( iDungeonID );
	}

	//{{ 2011. 03. 21  ��μ�	���� ���̵� ����
#ifdef SERV_DUNGEON_DIFFICULTY_REVISION
	if( bCheckLowLevel == true )	// ���� ������ ���� �Ǿ� �ִٸ�
	{
		int iDungeonMinLevel = SiCXSLDungeonManager()->GetDungeonMinLevel( iDungeonIDAndDif );

		if( static_cast<int>(GetLevel()) < iDungeonMinLevel - 2 )
		{
			SET_ERROR( ERR_CREATE_PARTY_FAILE );
			return false;
		}
	}
#endif SERV_DUNGEON_DIFFICULTY_REVISION
	//}}

	return true;
}

bool KGSUser::CheckPvpPartyCondition( IN const char cPVPGameType, IN const bool bDontCheckPartyUID )
{
	SET_ERROR( NET_OK );

	////////////////////////////////////////////////////////////////////////// ������ ����
	// DungeonID�˻�

	if( CXSLRoom::IsValidPvpGameType( (CXSLRoom::PVP_GAME_TYPE)cPVPGameType ) == false )
	{
		START_LOG( cerr, L"pvp game type���� �̻��ϴ�!" )
			<< BUILD_LOGc( cPVPGameType )
			<< END_LOG;

		SET_ERROR( ERR_PARTY_12 );
		return false;
	}

	//�̹� ��Ƽ�� �ִ°��..
	if( bDontCheckPartyUID == false )
	{
		if( GetPartyUID() != 0 )
		{
			SET_ERROR( ERR_PARTY_12 );
			return false;
		}
	}

	//��� �̻��� �ƴϸ� üũ�Ѵ�.
	if( GetAuthLevel() >= SEnum::UAL_GM )
	{
		START_LOG( cout, L"��� ���� ��Ƽ ���� �õ�!" )
#ifdef SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetCharUID() )
#else
			<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
			<< BUILD_LOGc( cPVPGameType );
	}
	else
	{
		// ���� ��Ƽ ���� ����
		if( GetLevel() < SEnum::PE_PVP_OFFICIAL_LIMIT_LEVEL )
		{
			START_LOG( cerr, L"���� ������ �ϱ⿡ ������ ĳ���� �����Դϴ�." )
				<< BUILD_LOG( GetUID() )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOG( GetMatchUID() )
				<< BUILD_LOGc( GetLevel() )
				<< END_LOG;

			SET_ERROR( ERR_MATCH_MAKING_04 );
			return false;
		}
	}

	return true;
}
#endif SERV_PVP_NEW_SYSTEM
//}}

//{{ 2012. 05. 10	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
bool KGSUser::CheckForDungeonGameStart( IN const char cDungeonMode, 
										IN const int iDungeonID, 
										IN const char cDifficultyLevel, 
										IN const bool bCheckForAutoParty,
										IN const bool bSendErrorPopUp )
{
	SET_ERROR( NET_OK );

	////////////////////////////////////////////////////////////////////////// ���� �˻�
	//{{ 2009. 8. 5  ������		ü��ID����
	if( IsGuestUser() )
	{
		if( GetLevel() > SiKGameSysVal()->GetGuestUserLimitLevel() )
		{
			START_LOG( cerr, L"Ŭ���̾�Ʈ���� ü��ID�� ����20�Ǿ����� ������ ĳ���������� �̵��������ٵ� ���� ������ �ϰ��ֳ�?" )
				<< BUILD_LOG( GetCharUID() )
				<< END_LOG;

			SET_ERROR( ERR_GUEST_USER_02 );
			return false;
		}

		if( CXSLDungeon::IsHenirDungeon( iDungeonID ) )
		{
			SET_ERROR( ERR_GUEST_USER_00 );
			return false;
		}
	}

	// 5. �ټ��� �˻�
	if( CheckingSpirit( iDungeonID ) == false )
	{
		SET_ERROR( ERR_ROOM_27 );
		return false;
	}

#ifdef SERV_DUNGEON_OPTION_IN_LUA
	if( CXSLDungeon::IsFixedMembers( iDungeonID ) == true )
	{
		int iNumMember = 0;

		if( GetPartyUID() == 0 )
		{
			iNumMember = 1;
		}
		else if( SiKPartyListManager()->GetPartyNumMember( GetPartyUID(), iNumMember ) == false )
		{
			START_LOG( cerr, L"��Ƽ �ɹ� �̻�.!" )
				<< BUILD_LOG( GetPartyUID() )
				<< BUILD_LOG( GetCharName() )
				<< BUILD_LOG( iNumMember )
				<< END_LOG;

			SET_ERROR( ERR_PARTY_46 );
			return false;
		}

		if( iNumMember != CXSLDungeon::GetFixedMembers( iDungeonID ) )
		{
			SET_ERROR( ERR_PARTY_46 );
			return false;
		}
	}
#endif SERV_DUNGEON_OPTION_IN_LUA

#ifdef SERV_NEW_EVENT_TYPES
	bool bIsEventDungeon = SiCXSLDungeonManager()->IsSwitchingWithEventInfo( iDungeonID );
	if( !SiKGameEventManager()->IsEnableDungeon( iDungeonID, bIsEventDungeon ) )
	{
		START_LOG( cerr, L"�����ִ� �̺�Ʈ DungeonID�Դϴ�. Ŭ�󿡼� �˻������ٵ�.." )
			<< BUILD_LOG( GetCharName() )
			<< BUILD_LOG( iDungeonID )
			<< BUILD_LOGc( (int)cDifficultyLevel )
			<< END_LOG;

		SET_ERROR( ERR_PARTY_22 );
		return false;
	}
#endif SERV_NEW_EVENT_TYPES

	// 6. �����ʿ� �����۰˻�.
	int iRequireItemID = 0;
	int iRequireQuantity = 0;
	const int iDungeonIDAndDif = iDungeonID + static_cast<int>(cDifficultyLevel);

	if( CheckRequiredItemToEnterDungeonRoom( iDungeonIDAndDif, (CXSLDungeon::DUNGEON_MODE) cDungeonMode, iRequireItemID, iRequireQuantity ) == false )
	{
		//{{ 2010. 05. 06  ������	���� �˾�
		if( bSendErrorPopUp )
		{
			KEGS_ERROR_POPUP_NOT kNot;
			kNot.m_iOK = NetError::ERR_DUNGEON_REQUIRE_ITEM_00;
			kNot.m_iErrorData1 = iRequireItemID;
			kNot.m_iErrorData2 = iRequireQuantity;
			SendPacket( EGS_ERROR_POPUP_NOT, kNot );
		}
		//}}

		SET_ERROR( ERR_DUNGEON_REQUIRE_ITEM_00 );
		return false;
	}

#ifdef SERV_LIMITED_DUNGEON_PLAY_TIMES
	if( CheckLimitedPlayTimes( iDungeonIDAndDif ) == false )
	{
		SET_ERROR( ERR_DUNGEON_ENTER_LIMIT_00 );
		return false;
	}
#endif SERV_LIMITED_DUNGEON_PLAY_TIMES

	if( bCheckForAutoParty )
	{
		// 7. ĳ���� ���� �˻�
		if( SiCXSLDungeonManager()->CheckMinLevelForEnterDungeon( iDungeonIDAndDif, GetLevel() ) == false )
		{
			SET_ERROR( ERR_DUNGEON_GAME_START_00 );
			return false;
		}

		// 8. ������ ���� �˻�
		{
			//{{ 2012. 10. 09	��μ�	��Ʋ�ʵ� ������ ���� �ý���
#ifdef SERV_BATTLE_FIELD_ITEM_LEVEL
			// ȥ�� �̹Ƿ� ��û�ڸ� üũ����
			std::map< int, KInventoryItemInfo > mapEquippedItem;
			std::set< CXSLItem::ITEM_TYPE > setItemType;

			setItemType.insert( CXSLItem::IT_WEAPON );	// ����
            setItemType.insert( CXSLItem::IT_DEFENCE );	// ��

			// ��ϵ� �ΰ��� �����۸� ���� �˻翡 ����Ѵ�
			// �ƹ�Ÿ�� �ƴ� ��� ��������
			m_kInventory.GetEquippedItem( setItemType, false, mapEquippedItem ); 

			if( CheckItemLevel( mapEquippedItem, iDungeonIDAndDif ) == false )
			{
				SET_ERROR( ERR_DUNGEON_GAME_START_01 );
				return false;
			}
#endif SERV_BATTLE_FIELD_ITEM_LEVEL
			//}}
		}
	}

	// 9. ������� Ŭ��� �˻� --> ��������� ���� üũ���� �ʽ��ϴ�!
	//if( GetAuthLevel() < SEnum::UAL_GM )
	//{
	//	int iRequireDungeonID = SiCXSLDungeonManager()->GetRequireDungeonID( iDungeonIDAndDif );
	//	if( iRequireDungeonID != 0 )
	//	{
	//		std::map<int,KDungeonClearInfo>::iterator mit;
	//		mit = m_mapDungeonClear.find( iRequireDungeonID );
	//		if( mit == m_mapDungeonClear.end() )
	//		{
	//			SET_ERROR( ERR_ROOM_06 );
	//			return false;
	//		}
	//	}
	//}

	//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER
	//	���潺���� �˻�
	if( CXSLDungeon::IsDefenceDungeon( iDungeonID ) == true )
	{
		//	���� ���� �˻�
		if( false == SiKGSWorldMissionManager()->GetIsActive() )
		{
			SET_ERROR( ERR_WORLD_MISSION_02 );
			return false;
		}

		int iWorldCrystalNum = SiKGSWorldMissionManager()->GetWorldCrystalCount();
		int iWorldCrystalMin, iWorldCrystalMax;

		switch( cDifficultyLevel )
		{
		case CXSLDungeon::DL_NORMAL:	iWorldCrystalMin = 0; iWorldCrystalMax = 6; break;
		case CXSLDungeon::DL_HARD:		iWorldCrystalMin = 7; iWorldCrystalMax = 12; break;
		case CXSLDungeon::DL_EXPERT:	iWorldCrystalMin = 13; iWorldCrystalMax = 15; break;
		}

		if( ( iWorldCrystalNum < iWorldCrystalMin ) || ( iWorldCrystalMax < iWorldCrystalNum ) )
		{
			SET_ERROR( ERR_PARTY_22 );
			return false;
		}
	}
#endif SERV_INSERT_GLOBAL_SERVER
	//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�

	return true;
}

bool KGSUser::CheckItemLevel( IN std::map< int, KInventoryItemInfo >& mapEquippedItem, IN int iDungeonIDAndDif )
{
	//////////////////////////////////////////////////////////////////////////
	// ������ ������ ��� ���� ��� ���
	//
	// ( ����* 4 + ���� + ���� + �尩 + �Ź� ) / 8
	//////////////////////////////////////////////////////////////////////////
	int iDungeonItemLevel = SiCXSLDungeonManager()->GetRequireItemLevel( iDungeonIDAndDif );
	if( iDungeonItemLevel == 0 )
		return true;

	std::map< int, KInventoryItemInfo >::iterator mit = mapEquippedItem.begin();
	int iAllEquippedItemLevel = 0;
	for( ; mit != mapEquippedItem.end() ; ++mit )
	{
		int iEquippedItemLevel = 0;
		//{{ 2013. 05. 15	������	������ ����
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
		if( SiCXSLItemManager()->GetItemLevel( GetUnitType(), GetUnitClass(), mit->second.m_kItemInfo, iEquippedItemLevel ) == false )
#else
		if( SiCXSLItemManager()->GetItemLevel( mit->second.m_kItemInfo, iEquippedItemLevel ) == false )
#endif SERV_NEW_ITEM_SYSTEM_2013_05
		//}}
			return false;

		// ������ * 4 �� ���ش�.
		const CXSLItem::ItemTemplet* pItemTemplet = SiCXSLItemManager()->GetItemTemplet( mit->second.m_kItemInfo.m_iItemID );
		if( pItemTemplet == NULL )
		{
			START_LOG( cerr, L"������ ������ �������� �ʽ��ϴ�!" )
				<< BUILD_LOG( mit->second.m_kItemInfo.m_iItemID )
				<< END_LOG;
			return false;
		}

		if( pItemTemplet->m_ItemType == CXSLItem::IT_WEAPON )
		{
			iEquippedItemLevel = iEquippedItemLevel * 4;
		}

		// ������ ������ ������ ��� ���Ѵ�.
		iAllEquippedItemLevel += iEquippedItemLevel;
	}

	iAllEquippedItemLevel = iAllEquippedItemLevel / 8;

	if( iDungeonItemLevel > iAllEquippedItemLevel )
		return false;

	return true;
}
#endif SERV_BATTLE_FIELD_SYSTEM
//}}

bool KGSUser::CheckRequiredItemToEnterDungeonRoom( int iDungeonID, CXSLDungeon::DUNGEON_MODE eDungeonMode )
{
	int iDummy1 = 0;
	int iDummy2 = 0;

	return CheckRequiredItemToEnterDungeonRoom( iDungeonID, eDungeonMode, iDummy1, iDummy2 );
}

bool KGSUser::CheckRequiredItemToEnterDungeonRoom( int iDungeonID, CXSLDungeon::DUNGEON_MODE eDungeonMode, int& iRequireItemID, int& iRequireQuantity )
{
	if( iDungeonID <= 0 )
	{
		START_LOG( cerr, L"����id�� �̻��մϴ�" )
			<< BUILD_LOG( iDungeonID )
			<< BUILD_LOG( (int)eDungeonMode )
			<< BUILD_LOG( GetRoomUID() )
			<< BUILD_LOG( CXSLRoom::GetRoomType( GetRoomUID() ) )
			<< END_LOG;
		
		return false;
	}

	if( CXSLDungeon::IsValidDungeonMode( eDungeonMode ) == false )
	{
		START_LOG( cerr, L"���������� DungeonMode���Դϴ�." )
			<< BUILD_LOG( iDungeonID )
			<< BUILD_LOG( (int)eDungeonMode )
			<< END_LOG;

		return false;
	}

	// [�̺�Ʈ] 080610.hoons.��� �̺�Ʈ
	if( SiKGameEventManager()->CheckSecretDungeonEvent( iDungeonID, static_cast<int>(eDungeonMode) ) == true )
		return true;

	//{{ 2010. 01. 05  ������	PC�� �����̾�
#ifdef SERV_PC_BANG_PRE

	// PC�� �����̾� - PC���̸� ��� �� ��ϸ� �ð� ���� ����
	if( IsPcBang() == true
#ifdef SERV_PC_BANG_TYPE
		&& SiKGameSysVal()->IsSecretDungeonFree( GetPcBangType() ) == true
#endif SERV_PC_BANG_TYPE
		)
	{
		if( eDungeonMode != CXSLDungeon::DM_HENIR_CHALLENGE  &&
			//{{ 2010. 05. 03  ������	��д��� ����
#ifdef SERV_SECRET_HELL
			eDungeonMode != CXSLDungeon::DM_SECRET_HELL  &&
#endif SERV_SECRET_HELL
			//}}
			CXSLDungeon::IsEventDungeon( iDungeonID ) == false )
		{
			return true;
		}
	}

#endif SERV_PC_BANG_PRE
	//}}

#ifdef SERV_ADDITIONAL_TICKET
	std::vector< std::pair< int, int > > vecTicketInformation;
	if( SiCXSLDungeonManager()->GetTicketInformation( iDungeonID, vecTicketInformation ) == false )
#else SERV_ADDITIONAL_TICKET
	if( SiCXSLDungeonManager()->GetRequireItemIDAndCount( iDungeonID, eDungeonMode, iRequireItemID, iRequireQuantity ) == false )
#endif SERV_ADDITIONAL_TICKET
	{
		START_LOG( cerr, L"������ �ʿ��� ������ ������ �������ٰ� �����߽��ϴ�!" )
			<< BUILD_LOG( GetCharUID() )
#ifndef SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( iDungeonID )
			<< BUILD_LOG( eDungeonMode )
			<< END_LOG;

#ifdef SERV_FIX_DUNGEON_REQUIRE_ITEM_INFO_PROCESS// �۾���¥: 2013-09-04	// �ڼ���
		return false;
#endif // SERV_FIX_DUNGEON_REQUIRE_ITEM_INFO_PROCESS
	}

#ifdef SERV_ADDITIONAL_TICKET
	if( vecTicketInformation.empty() == true )
	{
		return true;
	}

	std::vector< std::pair< int, int > >::iterator vitTicketInformation = vecTicketInformation.begin();
	for( ; vitTicketInformation != vecTicketInformation.end(); ++vitTicketInformation )
	{
		iRequireItemID = vitTicketInformation->first;
		iRequireQuantity = vitTicketInformation->second;

		std::map< int, int > mapRequiredItem;
		if( iRequireQuantity <= 0 )
			mapRequiredItem.insert( std::make_pair( iRequireItemID, 1 ) );
		else
			mapRequiredItem.insert( std::make_pair( iRequireItemID, iRequireQuantity ) );

		if( m_kInventory.IsEnoughItemExistOnlyInventory( mapRequiredItem ) == true )
			return true;
	}

	return false;
#else SERV_ADDITIONAL_TICKET
	if( iRequireItemID <= 0 )
	{
		return true;
	}


    std::map< int, int > mapRequiredItem;
	if( iRequireQuantity <= 0 )
		mapRequiredItem.insert( std::make_pair( iRequireItemID, 1 ) );
	else
		mapRequiredItem.insert( std::make_pair( iRequireItemID, iRequireQuantity ) );

	//{{ 2009. 8. 7  ������		����
    return m_kInventory.IsEnoughItemExistOnlyInventory( mapRequiredItem );
	//}}
#endif SERV_ADDITIONAL_TICKET
}

//{{ 2011. 02. 21  ��μ�	Ư�� ���� ���� ������ ���� ����
#ifdef SERV_DUNGEON_REQUIRED_ITEM_BUG
bool KGSUser::DeleteRequiredItemToEnterDungeonRoom( int iDungeonID, CXSLDungeon::DUNGEON_MODE eDungeonMode, std::vector< KInventoryItemInfo >& vecInventorySlotInfo, OUT int & iItemID  )
#else
bool KGSUser::DeleteRequiredItemToEnterDungeonRoom( int iDungeonID, CXSLDungeon::DUNGEON_MODE eDungeonMode, std::vector< KInventoryItemInfo >& vecInventorySlotInfo )
#endif SERV_DUNGEON_REQUIRED_ITEM_BUG
//}}
{
	if( iDungeonID <= 0 )
	{
		START_LOG( cerr, L"����id�� �̻��մϴ�! �Ͼ�� ���� ����!" )
			<< BUILD_LOG( iDungeonID )
			<< BUILD_LOG( GetRoomUID() ) 
			<< BUILD_LOG( CXSLRoom::GetRoomType( GetRoomUID() ) )
			<< END_LOG;

		// �ϴ� ����ó��..
		return true;
	}

	// [�̺�Ʈ] 080610.hoons.��� �̺�Ʈ
	if( SiKGameEventManager()->CheckSecretDungeonEvent( iDungeonID, static_cast<int>(eDungeonMode) ) == true )
		return true;

	//{{ 2010. 01. 05  ������	PC�� �����̾�
#ifdef SERV_PC_BANG_PRE

	// PC�� �����̾� - PC���̸� ��� �� ��ϸ� �ð� ���� ����
	if( IsPcBang() == true
#ifdef SERV_PC_BANG_TYPE
		&& SiKGameSysVal()->IsSecretDungeonFree( GetPcBangType() ) == true
#endif SERV_PC_BANG_TYPE
		)
	{
		if( eDungeonMode != CXSLDungeon::DM_HENIR_CHALLENGE  &&
			//{{ 2010. 05. 03  ������	��д��� ����
#ifdef SERV_SECRET_HELL
			eDungeonMode != CXSLDungeon::DM_SECRET_HELL  &&
#endif SERV_SECRET_HELL
			//}}
			CXSLDungeon::IsEventDungeon( iDungeonID ) == false )
		{
			return true;
		}
	}

#endif SERV_PC_BANG_PRE
	//}}

	//{{ 2011. 02. 21  ��μ�	Ư�� ���� ���� ������ ���� ����
#ifdef SERV_DUNGEON_REQUIRED_ITEM_BUG
	
#else
	int iItemID = 0;
#endif SERV_DUNGEON_REQUIRED_ITEM_BUG
	//}}
	int iQuantity = 0;

#ifdef SERV_ADDITIONAL_TICKET
	std::vector< std::pair< int, int > > vecTicketInformation;
	if( false == SiCXSLDungeonManager()->GetTicketInformation( iDungeonID, vecTicketInformation ) )
#else SERV_ADDITIONAL_TICKET
	if( false == SiCXSLDungeonManager()->GetRequireItemIDAndCount( iDungeonID, eDungeonMode, iItemID, iQuantity ) )
#endif SERV_ADDITIONAL_TICKET
	{
		START_LOG( cerr, L"������ �ʿ��� ������ ������ �������ٰ� �����߽��ϴ�!" )
			<< BUILD_LOG( GetCharUID() )
#ifndef SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( iDungeonID )
			<< BUILD_LOG( (int)eDungeonMode )
			<< END_LOG;

#ifdef SERV_FIX_DUNGEON_REQUIRE_ITEM_INFO_PROCESS// �۾���¥: 2013-09-04	// �ڼ���
		return false;
#endif // SERV_FIX_DUNGEON_REQUIRE_ITEM_INFO_PROCESS
	}

#ifdef SERV_ADDITIONAL_TICKET
	if( vecTicketInformation.empty() == true )
	{
		return true;
	}
	
	// �⺻Ƽ�Ͽ��� �߰�Ƽ�� �ε��� �չ�ȣ���� �޹�ȣ ������ �ִ� ���� Ȯ���ϰ�,
	// ���� ���� �߰ߵ� Ƽ���� �Ҹ��� ��� ����ϴ�.
	// �׸��� �ش� Ƽ���� iItemID �� ������ ���·� �����մϴ�.
	std::vector< std::pair< int, int > >::iterator vitTicketInformation = vecTicketInformation.begin();
	for( ; vitTicketInformation != vecTicketInformation.end(); ++vitTicketInformation )
	{
		iItemID = vitTicketInformation->first;
		iQuantity = vitTicketInformation->second;

		std::map< int, int > mapRequiredItem;
		if( iQuantity <= 0 )
		{
			mapRequiredItem.insert( std::make_pair( iItemID, 1 ) );

			if( m_kInventory.IsEnoughItemExistOnlyInventory( mapRequiredItem ) == true )
				return true;
		}
		else
		{
			mapRequiredItem.insert( std::make_pair( iItemID, iQuantity ) );

			if( m_kInventory.DeleteItemOnlyInventory( mapRequiredItem, vecInventorySlotInfo, KDeletedItemInfo::DR_DUNGEON_REQUIRE ) == true )
			{
				KStatisticsKey kKeyES;
				kKeyES.m_vecIntKey.push_back( iItemID );
				KSIManager.IncreaseCount( KStatistics::SI_LOC_ITEM, kKeyES, KStatistics::SI_ITEM_USE, iQuantity );
				//{{ 2011. 04. 13	������	������ ��� DB���
#ifdef SERV_ITEM_STATISTICS_TO_DB
				KSIManager.IncreaseCount( KStatistics::SI_ITEM_DB, kKeyES, KStatistics::SI_ITEM_USE, iQuantity );
#endif SERV_ITEM_STATISTICS_TO_DB
				//}}

				return true;
			}
		}
	}
	return false;
#else SERV_ADDITIONAL_TICKET
	if( iItemID <= 0 )
	{
		return true;
	}

    std::map< int, int > mapRequiredItem;
	//������ 0�̸� �ִ����� Ȯ��
	if( iQuantity <= 0 )
	{
		mapRequiredItem.insert( std::make_pair( iItemID, 1 ) );

		//{{ 2009. 8. 7  ������		����
		return m_kInventory.IsEnoughItemExistOnlyInventory( mapRequiredItem );
		//}}
	}
	//������ 1�̻��ϰ�� �ش� �������� �����ش�.
	else
	{
		mapRequiredItem.insert( std::make_pair( iItemID, iQuantity ) );

		bool bRet = m_kInventory.DeleteItemOnlyInventory( mapRequiredItem, vecInventorySlotInfo, KDeletedItemInfo::DR_DUNGEON_REQUIRE );
		if( bRet == true )
		{
			KStatisticsKey kKeyES;
			kKeyES.m_vecIntKey.push_back( iItemID );
			KSIManager.IncreaseCount( KStatistics::SI_LOC_ITEM, kKeyES, KStatistics::SI_ITEM_USE, iQuantity );
			//{{ 2011. 04. 13	������	������ ��� DB���
#ifdef SERV_ITEM_STATISTICS_TO_DB
			KSIManager.IncreaseCount( KStatistics::SI_ITEM_DB, kKeyES, KStatistics::SI_ITEM_USE, iQuantity );
#endif SERV_ITEM_STATISTICS_TO_DB
			//}}
		}
		return bRet;
	}
#endif SERV_ADDITIONAL_TICKET

	return false;
}

void KGSUser::GetConnectionUnitInfo( KConnectionUnitInfo& kInfo )
{
	kInfo.m_iUnitUID		= GetCharUID();
	kInfo.m_nKNMSN			= m_uiKNMSerialNum;
	kInfo.m_cUnitClass		= GetUnitClass();
	kInfo.m_ucLevel			= m_ucLevel;
	kInfo.m_wstrCharName	= GetCharName();
	//{{ 2011. 06. 20	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
	//{{ 2012. 09. 11	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2
	kInfo.m_cRank			= GetPvpRankForClient();
#else
	kInfo.m_iRating			= GetRating();
#endif SERV_2012_PVP_SEASON2
	//}}
	kInfo.m_iRPoint			= GetRPoint();
#else
	kInfo.m_iPVPEmblem		= (int)m_ePVPEmblem;
#endif SERV_PVP_NEW_SYSTEM
	//}}	
	kInfo.m_uiPVPRanking	= 0;			//���� ��ŷ �ý��� �ϼ��� �־������
	//{{ 2011. 06. 20	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
	kInfo.m_iWin			= GetWin();
	kInfo.m_iLose			= GetLose();
#else
	kInfo.m_iWin			= m_iWin;
	kInfo.m_iLose			= m_iLose;
#endif SERV_PVP_NEW_SYSTEM
	//}}	
	kInfo.m_uiKillNum		= 0;			//����߰��� �־������.
	kInfo.m_iRoomUID		= GetRoomUID();
	kInfo.m_bPublic			= false;
	kInfo.m_bCanIntrude		= false;
	kInfo.m_iDungeonID		= 0;

	kInfo.m_cState			= GetUnitState();
	kInfo.m_iStateCode		= 0;

	//{{ 2009. 4. 17  ������	�Ÿ���
	kInfo.m_iChannelID		= SiKChannelManager()->GetChannelID();
	kInfo.m_iPartyUID		= GetPartyUID();
	kInfo.m_iRoomListID		= GetRoomListID();
	//}}

	//{{ 2009. 10. 23  ������	���
#ifdef GUILD_TEST
	kInfo.m_wstrGuildName	= m_kUserGuildManager.GetGuildName();
#endif GUILD_TEST
	//}}

#ifdef	SERV_LOCAL_RANKING_SYSTEM // ���볯¥: 2013-03-31
	m_kUserLocalRankingInfo.GetFilteredUserInfo( kInfo.m_kLocalRankingUserInfo );
#endif	// SERV_LOCAL_RANKING_SYSTEM

	switch( kInfo.m_cState )
	{
	case CXSLUnit::CUS_FIELD_MAP:
		kInfo.m_iStateCode = GetMapID();
		break;

	case CXSLUnit::CUS_PVP_WAIT:
	case CXSLUnit::CUS_PVP_PLAY:
	case CXSLUnit::CUS_DUNGEON_PLAY:
		{
			KRoomInfoPtr spRoomInfo = SiKRoomListManager()->GetRoomInfo( GetRoomUID() );
			if( spRoomInfo != NULL )
			{
				kInfo.m_bPublic			= spRoomInfo->m_bPublic;
				kInfo.m_bCanIntrude		= spRoomInfo->m_bCanIntrude;
			}
		}
		break;
	}

	//////////////////////////////////////////////////////////////////////////
	//���忡�� ���������� ���� ��찡 �־.
	//if( GetRoomUID() != 0  &&  GetRoomType( GetRoomUID() ) != CXSLRoom::RT_SQUARE )
	//{
	//	KRoomInfoPtr spRoomInfo = SiKRoomListManager()->GetRoomInfo( GetRoomUID() );

	//	if( spRoomInfo != NULL )
	//	{
	//		kInfo.m_bPublic			= spRoomInfo->m_bPublic;
	//		kInfo.m_bCanIntrude		= spRoomInfo->m_bCanIntrude;

	//		switch( spRoomInfo->m_RoomType )
	//		{
	//		case CXSLRoom::RT_PVP:
	//			if( spRoomInfo->m_RoomState == CXSLRoom::RS_WAIT )
	//			{
	//				kInfo.m_cConnectionState = CXSLUnit::CUS_PVP_WAIT;
	//			}
	//			else
	//			{
	//				kInfo.m_cConnectionState = CXSLUnit::CUS_PVP_PLAY;
	//			}
	//			break;

	//		case CXSLRoom::RT_DUNGEON:				
	//			if( spRoomInfo->m_RoomState == CXSLRoom::RS_WAIT )
	//			{
	//				kInfo.m_cConnectionState = CXSLUnit::CUS_DUNGEON_WAIT;

	//				//if( SiKChannelManager()->VerifyChannelID( GetChannelID() ) == true )
	//				//{
	//				//	kInfo.m_iDungeonID = SiKRoomListManager()->GetDungeonID( GetChannelID() );
	//				//}
	//			}
	//			else
	//			{
	//				kInfo.m_cConnectionState = CXSLUnit::CUS_DUNGEON_PLAY;
	//			}
	//			break;
	//			
	//		case CXSLRoom::RT_ARCADE:
	//			if( spRoomInfo->m_RoomState == CXSLRoom::RS_WAIT )
	//			{
	//				kInfo.m_cConnectionState = CXSLUnit::CUS_ARCADE_WAIT;

	//				if( GetRoomUID() != 0 )
	//				{
	//					kInfo.m_iDungeonID = SiKRoomListManager()->GetDungeonID( GetRoomUID() );
	//				}
	//			}
	//			else
	//			{
	//				kInfo.m_cConnectionState = CXSLUnit::CUS_ARCADE_PLAY;
	//			}
	//			break;
	//		}
	//	}
	//	else
	//	{
	//		START_LOG( cerr, L"roomuid�� 0�� �ƴѵ� �������� ����.!" )
	//			<< BUILD_LOG( GetCharName() )
	//			<< BUILD_LOG( GetRoomUID() )
	//			<< END_LOG
	//			;
	//	}
	//}
}

//{{ 2010. 01. 15  ������	���Խ���
#ifdef SERV_GUILD_AD

void KGSUser::GetApplyJoinGuildInfo( KApplyJoinGuildInfo& kInfo )
{
	//kInfo.m_iApplyJoinGuildUID; // �ƹ��͵� ���� ����
	kInfo.m_iUnitUID	 = GetCharUID();
	kInfo.m_wstrNickName = GetCharName();
	kInfo.m_cUnitClass	 = GetUnitClass();
	kInfo.m_ucLevel		 = GetLevel();
	//kInfo.m_wstrMessage; // �ƹ��͵� ���� ����
	//kInfo.m_wstrRegDate; // �ƹ��͵� ���� ����
}

#endif SERV_GUILD_AD
//}}

//{{ 2011. 06. 23	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
void KGSUser::GetMatchUserInfo( OUT KMatchUserInfo& kInfo )
{
	kInfo.m_iGSUID			= KBaseServer::GetKObj()->GetUID();
	kInfo.m_iUnitUID		= GetCharUID();
	kInfo.m_iPartyUID		= GetPartyUID();
	kInfo.m_wstrNickName	= GetCharName();
	kInfo.m_cUnitClass		= GetUnitClass();
	kInfo.m_ucLevel			= GetLevel();

	//{{ 2012. 06. 20	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2
	kInfo.m_cCurrentRank		= GetPvpRank();
	kInfo.m_iOfficialMatchCnt	= GetOfficialMatchCount();
#endif SERV_2012_PVP_SEASON2
	//}}

	kInfo.m_iRating			= GetRating();
	kInfo.m_iRPoint			= GetRPoint();
}
#endif SERV_PVP_NEW_SYSTEM
//}}

//{{ 2012. 02. 07	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
void KGSUser::GetAutoPartyUserInfo( OUT KAutoPartyUserInfo& kInfo )
{
	kInfo.m_iGSUID			= KBaseServer::GetKObj()->GetUID();
	kInfo.m_iUnitUID		= GetCharUID();
	kInfo.m_wstrNickName	= GetCharName();
	kInfo.m_cUnitClass		= GetUnitClass();
	kInfo.m_ucLevel			= GetLevel();
}
#endif SERV_BATTLE_FIELD_SYSTEM
//}}

//{{ 2012. 04. 23	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
//////////////////////////////////////////////////////////////////////////
bool KGSUser::IsHenirDungeonChallengeMode()
{
	//if( GetRoomUID() == 0 )
	//	return false;

	//if( CXSLRoom::GetRoomType( GetRoomUID() ) != CXSLRoom::RT_DUNGEON )
	//	return false;

	return m_kUserDungeonManager.IsPlayingNowHenirChallengeMode();
}
//////////////////////////////////////////////////////////////////////////
#else
//////////////////////////////////////////////////////////////////////////
bool KGSUser::IsHenirDungeonChallengeMode()
{
	if( GetPartyUID() > 0 )
	{
		int iDungeonID = 0;
		char cDifficultyLevel = 0;
		char cDungeonMode = 0;
		//{{ 2010. 02. 22  ������	ä�� ��Ƽ ����
#ifdef SERV_CHANNEL_PARTY
		if( !SiKPartyListManager()->GetDungeonInfo( GetPartyUID(), iDungeonID, cDifficultyLevel, cDungeonMode ) )
#else
		if( !SiKPartyManager()->GetDungeonInfo( GetPartyUID(), iDungeonID, cDifficultyLevel, cDungeonMode ) )
#endif SERV_CHANNEL_PARTY
			//}}
		{
			START_LOG( cerr, L"���� ������ ���� �� �����ϴ�! ������ ���� ����!" )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOG( GetPartyUID() )
				<< END_LOG;

			return false;
		}

		CXSLDungeon::DUNGEON_TYPE eDungeonType = SiCXSLDungeonManager()->GetDungeonType( iDungeonID + static_cast<int>(cDifficultyLevel) );
		if( eDungeonType != CXSLDungeon::DT_HENIR )
		{
			// ��ϸ� �ð��� �ƴϸ� false
			return false;
		}

		//CXSLDungeon::HENIR_DUNGEON_MODE eHenirDungeonMode = CXSLDungeon::HDM_INVALID;
		//if( !SiKPartyManager()->GetHenirDungeonMode( GetPartyUID(), eHenirDungeonMode ) )
		//{
		//	START_LOG( cerr, L"���� ��带 ���� �� �����ϴ�! ������ ���� ����!" )
		//		<< BUILD_LOG( GetCharUID() )
		//		<< BUILD_LOG( GetPartyUID() )
		//		<< END_LOG;

		//	return false;
		//}

		// ��ϸ� �ð� ������� ����
		return ( (CXSLDungeon::DUNGEON_MODE)cDungeonMode == CXSLDungeon::DM_HENIR_CHALLENGE );
	}

	// ��Ƽ�� �������� �ʴٸ� ��ϸ� �ð��� �ƴϴ�
	return false;
}
//////////////////////////////////////////////////////////////////////////
#endif SERV_BATTLE_FIELD_SYSTEM
//////////////////////////////////////////////////////////////////////////
//}}

#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013 // ���볯¥: 2013-06-27
void KGSUser::GetRoomUserInfo( OUT KRoomUserInfo& kRoomUserInfo, IN const int iRoomType_ )
{
	kRoomUserInfo.m_iGSUID		   = KBaseServer::GetKObj()->GetUID();
	kRoomUserInfo.m_iOwnerUserUID  = GetUID();
	kRoomUserInfo.m_cAuthLevel	   = GetAuthLevel();

	//{{ 2011. 12. 06	������	Ư�� ������ �̺�Ʈ �ý���
	kRoomUserInfo.m_sServerGroupID = KBaseServer::GetKObj()->GetServerGroupID();

	kRoomUserInfo.m_bMale		   = m_kNexonAccountInfo.m_bSex;
	kRoomUserInfo.m_ucAge		   = m_kNexonAccountInfo.m_uiAge;
	kRoomUserInfo.m_nUnitUID	   = GetCharUID();
	kRoomUserInfo.m_uiKNMSerialNum = m_uiKNMSerialNum;
	kRoomUserInfo.m_cUnitClass	   = GetUnitClass();
	kRoomUserInfo.m_ucLevel		   = GetLevel();
	kRoomUserInfo.m_bIsGuestUser   = IsGuestUser();

	//{{ 2012. 04. 17	������	���� �α� �÷� �߰�
	kRoomUserInfo.m_iNumResurrectionStone = m_iNumResurrectionStone;

	//{{ 2011. 07. 11	������	���� ����
	kRoomUserInfo.m_iOfficialMatchCnt = m_kUserPvpManager.GetOfficialMatchCount();
	kRoomUserInfo.m_iRating		   = GetRating();
	kRoomUserInfo.m_iMaxRating	   = m_kUserPvpManager.GetMaxRating();
	kRoomUserInfo.m_bIsWinBeforeMatch = m_kUserPvpManager.IsWinBeforeMatch();

	//{{ 2012. 06. 25	�ڼ���	2012 ���� ����2
	kRoomUserInfo.m_cRank					= GetPvpRankForClient();
	kRoomUserInfo.m_cRankForServer			= GetPvpRank();
	kRoomUserInfo.m_fKFactor				= m_kUserPvpManager.GetKFactor();
	kRoomUserInfo.m_bIsRedistributionUser	= m_kUserPvpManager.IsRedistributionUser();

	kRoomUserInfo.m_wstrNickName   = GetCharName();
	kRoomUserInfo.m_wstrIP		   = GetP2PIP();
	kRoomUserInfo.m_usPort		   = GetP2PPort();

	//{{ 2013. 1. 8	�ڼ���	Merge ����IP ���� ���н� ����IP�� �õ�( ������ )
	kRoomUserInfo.m_wstrInternalIP	= GetP2PInternalIP();
	kRoomUserInfo.m_usInternalPort	= GetP2PInternalPort();

	//{{ 2011. 11. 1	������	��Ʋ�ʵ� �ý���
	kRoomUserInfo.m_iPartyUID	   = GetPartyUID();

	//{{ 2009. 9. 25  ������	���
	kRoomUserInfo.m_iGuildUID	   = GetGuildUID();
	kRoomUserInfo.m_wstrGuildName  = m_kUserGuildManager.GetGuildName();
	kRoomUserInfo.m_ucMemberShipGrade = m_kUserGuildManager.GetMemberShipGrade();

	//{{ 2011. 05. 27    ��μ�    �޸� ���� ���� ����
	kRoomUserInfo.m_bComeBackUser	= m_kComeBackUserInfo.GetIsComeBackUser();

	//{{ 2012. 11. 9	������	��Ʋ�ʵ� �ý���
	m_kUserDungeonManager.IsPossibleHenirReward(); // �˻縸 �Ѵ�. �̺κ� ���߿� �����ؾ��ҵ�.. - ����

	//{{ 2012. 09. 19   ��μ�   ĳ�ü� �湮 ����
	kRoomUserInfo.m_bEnterCashShop	= m_kUserUnitManager.GetEnterCashShop();

	//{{ 2011. 07. 08    ��μ�    �ɼ� ��ġȭ
	unsigned int uiHP_OnePoint = CXSLUnit::UPHP_INIT + ( CXSLUnit::UPHP_INCREASE_ONE_POINT * (GetLevel() - 1)); // ���� ������ 1���� ��
	unsigned int uiUnitLevel = static_cast<unsigned int>(GetLevel());	//	���� ���� �̸� ���

	//080611.hoons.�������� �������� �˻��Ͽ� ȿ�������� �����ϱ�����.
	kRoomUserInfo.m_bIsRingofpvprebirth = m_kInventory.IsEquippedItem( CXSLItem::SI_RING_OF_PVP_REBIRTH ); //::1st. ����������

#ifdef EVENT_REBIRTH_RING_CN
	if(kRoomUserInfo.m_bIsRingofpvprebirth == false)
		kRoomUserInfo.m_bIsRingofpvprebirth = m_kInventory.IsEquippedItem( CXSLItem::SI_RING_OF_PVP_REBIRTH_EVENT ); //�߱� ���� ����������
#endif //EVENT_REBIRTH_RING_CN

#ifdef SERV_GATE_OF_DARKNESS_SUPPORT_EVENT
	kRoomUserInfo.m_iGateOfDarknessSupportEventTime = m_iGateOfDarknessSupportEventTime;
#endif SERV_GATE_OF_DARKNESS_SUPPORT_EVENT
	//�⺻ ����
	KStat kModifiedBaseStatBySkill = m_kStat;

	//{{ 2010. 10. 27	������	�� ���� ��ų
	KStatIncreaseRate kStatIncreaseRateInfo;

	////////////////////////////////////////////////////////////////////////// [����!]
	// ��ų�߿� BaseHP�� ������Ű�� �ͺ��� �̸� �������!
	{
		m_kSkillTree.CheckAddSkillStat_BaseHP( m_kStat, kModifiedBaseStatBySkill );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// RoomType ���
	CXSLRoom::ROOM_TYPE eRoomType = static_cast<CXSLRoom::ROOM_TYPE>(iRoomType_);
	if( iRoomType_ < 0 )
	{
		if( GetRoomUID() == 0 )
		{
			START_LOG( cerr, L"�������� RoomType�� -1�̶� RoomUID�� RoomType�� �������� �ϴµ� RoomUID�� 0�̴�?" )
				<< BUILD_LOG( iRoomType_ )
				<< BUILD_LOG( GetCharName() )
				<< END_LOG;
		}

		eRoomType = CXSLRoom::GetRoomType( GetRoomUID() );
	}

	//{{ 2012. 11. 05	�ڼ���	���� ���� ������ ��� �ʵ忡�� ����
	const bool bIsDungeonRoom = ( ( eRoomType == CXSLRoom::RT_DUNGEON ) || ( eRoomType == CXSLRoom::RT_BATTLE_FIELD ) );

	switch( eRoomType )
	{
	case CXSLRoom::RT_PVP:
		{
			//{{ ���� ���� �ý��� - ��μ�
			m_kUserBuffManager.GetBuffList( kRoomUserInfo.m_vecBuffInfo, CXSLRoom::RT_PVP );

			int iPVPChannelClass = 0;
			//LIF( GetPVPChannelClass( iPVPChannelClass ) );
			GetPVPChannelClass( iPVPChannelClass );

			// PVP Ÿ�Ժ� ���� ���� ����
			CalculatePvpUnitStat( iPVPChannelClass, kModifiedBaseStatBySkill, bIsDungeonRoom, uiHP_OnePoint, uiUnitLevel, kRoomUserInfo );

			//{{ 2010. 05. 11  ������	���� ���� ������ ����
			SiKGameEventManager()->GetRewardEvent( kRoomUserInfo
				, 0
				, GetLevel()
				, GetUnitClass()
				, 0
#ifdef SERV_PCBANG_USER_REWARD_EVENT// �۾���¥: 2013-07-02	// �ڼ���
				, IsPcBang()
#endif // SERV_PCBANG_USER_REWARD_EVENT
				);
		}
		break;

	case CXSLRoom::RT_DUNGEON:
		{
			//{{ ���� ���� �ý��� - ��μ�
			m_kUserBuffManager.GetBuffList( kRoomUserInfo.m_vecBuffInfo, CXSLRoom::RT_DUNGEON );

			// ���� �����ΰ�?
			bool bRevisionDungeon = IsHenirDungeonChallengeMode()  ||  CXSLDungeon::IsEventDungeon( m_kUserDungeonManager.GetDungeonID() );

			// Dungeon ���� ���� ����
			CalculateDungeonUnitStat( bRevisionDungeon, kModifiedBaseStatBySkill, bIsDungeonRoom, uiHP_OnePoint, uiUnitLevel, kRoomUserInfo, kStatIncreaseRateInfo );

			if( bRevisionDungeon == false )
			{
				//{{ 2009. 2. 2  ������		���Ͽɼ� ���� ���ʽ�
				float fAddSocketOptExpRate	= 0.0f;
				float fAddSocketOptEDRate	= 0.0f;

				// ���� ������ ���� �ɼ�
				m_kInventory.GetEquippedItemBonusRate( bIsDungeonRoom, fAddSocketOptExpRate, fAddSocketOptEDRate );

				// Īȣ ���� �ɼ�
				m_kUserTitleManager.GetEquippedTitleBonusRate( bIsDungeonRoom, fAddSocketOptExpRate, fAddSocketOptEDRate );
				//}}

				if( fAddSocketOptExpRate > 0.0f )
				{
					kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_SOCKET_OPT_EXP_RATE, fAddSocketOptExpRate );
				}
				if( fAddSocketOptEDRate > 0.0f )
				{
					kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_SOCKET_OPT_ED_RATE, fAddSocketOptEDRate );
				}
			}
	
			//{{ 2010. 05. 11  ������	���� ���� ������ ����
			SiKGameEventManager()->GetRewardEvent( kRoomUserInfo
				, m_kUserDungeonManager.GetDungeonID()
				, GetLevel()
				, GetUnitClass()
				, 1
#ifdef SERV_PCBANG_USER_REWARD_EVENT// �۾���¥: 2013-07-02	// �ڼ���
				, IsPcBang()
#endif // SERV_PCBANG_USER_REWARD_EVENT
				);
		}
		break;

		//{{ 2011. 11. 7	������	��Ʋ�ʵ� �ý���
	case CXSLRoom::RT_BATTLE_FIELD:
		{
			//{{ ���� ���� �ý��� - ��μ�
			m_kUserBuffManager.GetBuffList( kRoomUserInfo.m_vecBuffInfo, CXSLRoom::RT_BATTLE_FIELD );

			// Dungeon ���� ���� ����
			CalculateBattleFieldUnitStat( kModifiedBaseStatBySkill, bIsDungeonRoom, uiHP_OnePoint, uiUnitLevel, kRoomUserInfo, kStatIncreaseRateInfo );

			//{{ 2009. 2. 2  ������		���Ͽɼ� ���� ���ʽ�
			float fAddSocketOptExpRate	= 0.0f;
			float fAddSocketOptEDRate	= 0.0f;

			// ���� ������ ���� �ɼ�
			m_kInventory.GetEquippedItemBonusRate( bIsDungeonRoom, fAddSocketOptExpRate, fAddSocketOptEDRate );

			// Īȣ ���� �ɼ�
			m_kUserTitleManager.GetEquippedTitleBonusRate( bIsDungeonRoom, fAddSocketOptExpRate, fAddSocketOptEDRate );
			//}}

			//{{ 2010. 05. 11  ������	�̺�Ʈ ���� �ڵ� ����
			if( fAddSocketOptExpRate > 0.0f )
			{
				kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_SOCKET_OPT_EXP_RATE, fAddSocketOptExpRate );
			}
			if( fAddSocketOptEDRate > 0.0f )
			{
				kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_SOCKET_OPT_ED_RATE, fAddSocketOptEDRate );
			}

			//{{ 2012. 12. 18	������	�ʵ忡���� ����ġ �̺�Ʈ ����
			SiKGameEventManager()->GetRewardEvent( kRoomUserInfo
				, m_kUserDungeonManager.GetDungeonID()
				, GetLevel()
				, GetUnitClass()
				, 1
#ifdef SERV_PCBANG_USER_REWARD_EVENT// �۾���¥: 2013-07-02	// �ڼ���
				, IsPcBang()
#endif // SERV_PCBANG_USER_REWARD_EVENT
				);
		}
		break;

	case CXSLRoom::RT_TRAININGCENTER:
		{
			//{{ ���� ���� �ý��� - ��μ�
			m_kUserBuffManager.GetBuffList( kRoomUserInfo.m_vecBuffInfo );

			// TrainingCenter ���� ���� ����
			CalculateTrainingCenterUnitStat( kModifiedBaseStatBySkill, bIsDungeonRoom, uiHP_OnePoint, uiUnitLevel, kRoomUserInfo, kStatIncreaseRateInfo );

			//{{ 2009. 2. 2  ������		���Ͽɼ� ���� ���ʽ�
			float fAddSocketOptExpRate	= 0.0f;
			float fAddSocketOptEDRate	= 0.0f;

			// ���� ������ ���� �ɼ�
			m_kInventory.GetEquippedItemBonusRate( bIsDungeonRoom, fAddSocketOptExpRate, fAddSocketOptEDRate );

			// Īȣ ���� �ɼ�
			m_kUserTitleManager.GetEquippedTitleBonusRate( bIsDungeonRoom, fAddSocketOptExpRate, fAddSocketOptEDRate );
			//}}

			if( fAddSocketOptExpRate > 0.0f )
			{
				kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_SOCKET_OPT_EXP_RATE, fAddSocketOptExpRate );
			}
			if( fAddSocketOptEDRate > 0.0f )
			{
				kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_SOCKET_OPT_ED_RATE, fAddSocketOptEDRate );
			}
		}
		break;

		//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
	case CXSLRoom::RT_WEDDING_HALL:
		{
			kRoomUserInfo.m_kGameStat = kModifiedBaseStatBySkill;
		}break;
#endif SERV_RELATIONSHIP_SYSTEM
		//}

	default:
		{
			START_LOG( cerr, L"�������� ������ �߸��� ��Ÿ��.!" )
				<< BUILD_LOG( CXSLRoom::GetRoomType( GetRoomUID() ) )
#ifdef SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetCharUID() )
#else
				<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
				<< END_LOG;
		}
		break;
	}

	//{{ ���� ���� �ý��� - ��μ�
	m_kUserBuffManager.GetBuffBonusRate( kRoomUserInfo );

	// ��ų ���� ���� ����
	CalculateSkillUnitStat( kRoomUserInfo, kStatIncreaseRateInfo );

	//{{ 2012. 02. 06	�ڼ���	�߷�Ÿ�� ���� �̺�Ʈ
#ifdef SERV_VALENTINE_DAY_EVENT
	if( (SiKGameSysVal()->GetMenBuffEvent() == true ) && ( eRoomType == CXSLRoom::RT_DUNGEON ) )
	{
		switch( GetUnitType() )
		{
		case CXSLUnit::UT_ELSWORD:
		case CXSLUnit::UT_RAVEN:
		case CXSLUnit::UT_CHUNG:
			kRoomUserInfo.m_kGameStat.m_iAtkPhysic += static_cast<int>( kRoomUserInfo.m_kGameStat.m_iAtkPhysic * 0.15f );
			kRoomUserInfo.m_kGameStat.m_iAtkMagic  += static_cast<int>( kRoomUserInfo.m_kGameStat.m_iAtkMagic  * 0.15f );
			break;
		}
	}
#endif SERV_VALENTINE_DAY_EVENT
	//}}

#ifdef SERV_PAYMENT_ITEM_WITH_CONSUMING_OTHER_ITEM
	m_kInventory.GetSpecialItem( kRoomUserInfo.m_mapSpecialItem );
#endif SERV_PAYMENT_ITEM_WITH_CONSUMING_OTHER_ITEM

	m_kInventory.GetEquippedItem( kRoomUserInfo.m_mapEquippedItem );

	LIF( KUserSkillTree::MAX_SKILL_SLOT == 8 );     
	KSkillData aSkillData[ KUserSkillTree::MAX_SKILL_SLOT];
	m_kSkillTree.GetSkillSlot( aSkillData );

	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
	for( int iSkillSlotID = 0 ; iSkillSlotID < KUserSkillTree::MAX_SKILL_SLOT ; ++iSkillSlotID )
	{
		if( aSkillData[iSkillSlotID].m_iSkillID == CXSLSkillTree::SI_ETC_WS_COMMON_LOVE )
		{
			aSkillData[iSkillSlotID].m_cSkillLevel = m_kUserRelationshipManager.GetRelationshipType();
		}
	}
#endif //SERV_RELATIONSHIP_SYSTEM

	// ��ų ���� ����
	kRoomUserInfo.m_UnitSkillData.m_aEquippedSkill[0] = aSkillData[KUserSkillTree::SKILL_SLOT_A1];
	kRoomUserInfo.m_UnitSkillData.m_aEquippedSkill[1] = aSkillData[KUserSkillTree::SKILL_SLOT_A2];
	kRoomUserInfo.m_UnitSkillData.m_aEquippedSkill[2] = aSkillData[KUserSkillTree::SKILL_SLOT_A3];
	kRoomUserInfo.m_UnitSkillData.m_aEquippedSkill[3] = aSkillData[KUserSkillTree::SKILL_SLOT_A4];

	kRoomUserInfo.m_UnitSkillData.m_aEquippedSkillSlotB[0] = aSkillData[KUserSkillTree::SKILL_SLOT_B1];
	kRoomUserInfo.m_UnitSkillData.m_aEquippedSkillSlotB[1] = aSkillData[KUserSkillTree::SKILL_SLOT_B2];
	kRoomUserInfo.m_UnitSkillData.m_aEquippedSkillSlotB[2] = aSkillData[KUserSkillTree::SKILL_SLOT_B3];
	kRoomUserInfo.m_UnitSkillData.m_aEquippedSkillSlotB[3] = aSkillData[KUserSkillTree::SKILL_SLOT_B4];

	kRoomUserInfo.m_UnitSkillData.m_cSkillSlotBExpirationState = (char) m_kSkillTree.GetSkillSlotBExpirationState();
	kRoomUserInfo.m_UnitSkillData.m_wstrSkillSlotBEndDate = m_kSkillTree.GetSkillSlotBEndDateString();

	m_kSkillTree.GetPassiveSkillData( kRoomUserInfo.m_UnitSkillData.m_vecPassiveSkill );

	//{{ 2010. 04. 08  ������	����� ��Ʈ
	m_kSkillTree.GetSkillNote( kRoomUserInfo.m_UnitSkillData.m_vecSkillNote );

	//{{ 2009. 11. 27  ������	��彺ų
	float fAddGuildBonusExpRate = 0.0f;
	float fAddSocketOptEDRate = 0.0f;

	m_kUserGuildManager.GetGuildPassiveSkillData( kRoomUserInfo.m_UnitSkillData.m_vecGuildPassiveSkill );

	m_kUserGuildManager.GetGuildSkillBonusRate( fAddGuildBonusExpRate, fAddSocketOptEDRate );

	if( fAddGuildBonusExpRate > 0.0f )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_GUILD_BONUS_EXP_RATE, fAddGuildBonusExpRate );
	} 
	if( fAddSocketOptEDRate > 0.0f )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_SOCKET_OPT_ED_RATE, fAddSocketOptEDRate );
	}

	//{{ 2013. 03. 14	 ������� �ʴ� RoomUserInfo �� ������ ���� - ��μ�
#ifdef SERV_DELETE_ROOM_USER_INFO_DATA
#else
	//{{ 2012. 03. 20	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	kRoomUserInfo.m_iSpiritMax	= m_kUserSpiritManager.GetSpiritMax();
	kRoomUserInfo.m_iSpirit		= m_kUserSpiritManager.GetSpirit();
#else
	kRoomUserInfo.m_iSpiritMax	= m_iSpiritMax;
	kRoomUserInfo.m_iSpirit		= m_iSpirit;
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}
#endif SERV_DELETE_ROOM_USER_INFO_DATA
	//}

	//{{ 2010. 01. 29  ������	PC�� ���ȭ
	kRoomUserInfo.m_bIsGameBang = IsPcBang();
	//}}    
	
#ifdef SERV_PC_BANG_TYPE
	kRoomUserInfo.m_iPcBangType = GetPcBangType();
#endif SERV_PC_BANG_TYPE

	//2008. 03. 10. hoons : ���� �������� ����Ʈ�� ����Ʈ ������ ��������Ʈ ������ �����´�.
	m_kUserQuestManager.GetOngoingQuestForRoom( GetThisPtr<KGSUser>(), kRoomUserInfo.m_mapOngoingQuest );

	//{{ 2011. 05. 04  ��μ�	���� Ŭ����� ������ ���� ���� �߰�
	m_kUserQuestManager.GetUserAllQuest( kRoomUserInfo.m_setQuestInfo );
#ifdef SERV_PAYMENT_ITEM_ON_GOING_QUEST
	m_kUserQuestManager.GetUserGoingQuest( GetThisPtr<KGSUser>(), kRoomUserInfo.m_setGoingQuestInfo );
#endif SERV_PAYMENT_ITEM_ON_GOING_QUEST

	// ĪȣID
	kRoomUserInfo.m_iTitleID = m_kUserTitleManager.GetEquippedTitleID();

	// ����ġ �޴� ���� �� �߰� ����ġ ����
	CheckInventoryExpMedal( kRoomUserInfo );
	
	//{{ 2009. 5. 5  ������		ä�κ� ���ʽ�
	float fAddChannelBonusExpRate = 0.0f;
	float fAddChannelBonusEDRate = 0.0f; // ���� ������
	SiKChannelManager()->GetChannelBonus( GetLevel(), fAddChannelBonusExpRate, fAddChannelBonusEDRate );

	if( fAddChannelBonusExpRate > 0.0f )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_CHANNEL_BONUS_EXP_RATE, fAddChannelBonusExpRate );
	}

	//{{ 2010. 07. 21  ������	�� �ý���
	m_kUserPetManager.GetSummonedPetInfo( kRoomUserInfo.m_vecPet );

#ifdef	SERV_RIDING_PET_SYSTM// ���볯¥: 2013-04-20
	// Ż�� ��
	m_kUserRidingPetManager.GetSummonedRidingPetInfo( kRoomUserInfo.m_iRidingPetUID, kRoomUserInfo.m_usRidingPetID );
#endif //SERV_RIDING_PET_SYSTM

	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
	kRoomUserInfo.m_iLoverUnitUID = m_kUserRelationshipManager.GetLoverUnitUID();
#endif SERV_RELATIONSHIP_SYSTEM
	//}

#ifdef SERV_GROW_UP_SOCKET
	kRoomUserInfo.m_iExchangeCount = m_iExchangeCount;
#endif SERV_GROW_UP_SOCKET

#ifdef SERV_RECRUIT_EVENT_BASE
	std::vector< KRecommendUserInfo > vecRecruiterUnitList;
	std::vector< KRecommendUserInfo > vecRecruitUnitList;
	m_kUserRecommendManager.GetRecruiterUnitList( vecRecruiterUnitList );
	m_kUserRecommendManager.GetRecruitUnitList( vecRecruitUnitList );

	if( false == vecRecruiterUnitList.empty() )
	{
		// ��õ�� ���
		kRoomUserInfo.m_iRecruiterUnitUID = vecRecruiterUnitList[0].m_iUnitUID;
		START_LOG( cout, L"��õ�� ���" )
			<< BUILD_LOG( kRoomUserInfo.m_iRecruiterUnitUID );
	}
	else if( false == vecRecruitUnitList.empty() )
	{
		// ����õ�� ���
		BOOST_TEST_FOREACH( KRecommendUserInfo&, kInfo, vecRecruitUnitList )
		{
			kRoomUserInfo.m_vecRecruitUnitUID.push_back( kInfo.m_iUnitUID );
			START_LOG( cout, L"����õ�� ���" )
				<< BUILD_LOG( kInfo.m_iUnitUID );
		}
	}
#endif SERV_RECRUIT_EVENT_BASE
}

void KGSUser::CalculatePvpUnitStat( IN int iPVPChannelClass, 
								    IN KStat& kModifiedBaseStatBySkill, 
									IN bool bIsDungeonRoom, 
									IN unsigned int uiHP_OnePoint, 
									IN unsigned int uiUnitLevel, 
									IN OUT KRoomUserInfo& kRoomUserInfo )
{
	switch( iPVPChannelClass )
	{
	default:
	case KPVPChannelInfo::PCC_PLAY:
		{
			// �Ϲ�ä�ΰ� ���ä���� 10% �ɷ�ġ�� �������ش�.
			const float RESIZE_FACTOR = 0.1f;

			// �⺻ �ɷ�ġ�� 1�� �������� gap ���̸� ���δ�
			KStatTable::KUnitStatInfo sUnitStatLv1;
			SiKStatTable()->GetUnitStat( GetUnitClass(), 1,	sUnitStatLv1 );

			// �ѹ�� : kRoomUserInfo.m_kGameStat.m_iBaseHP		= (int)(sUnitStatLv1.m_uiHP)			+ (int)((float)(kModifiedBaseStatBySkill.m_iBaseHP		- sUnitStatLv1.m_uiHP)          * fResizeFactor);
			//}}
			kRoomUserInfo.m_kGameStat.m_iAtkPhysic	= (int)(sUnitStatLv1.m_usAtkPhysic)		+ (int)((float)(kModifiedBaseStatBySkill.m_iAtkPhysic	- sUnitStatLv1.m_usAtkPhysic)   * RESIZE_FACTOR);
			kRoomUserInfo.m_kGameStat.m_iAtkMagic	= (int)(sUnitStatLv1.m_usAtkMagic)		+ (int)((float)(kModifiedBaseStatBySkill.m_iAtkMagic	- sUnitStatLv1.m_usAtkMagic)    * RESIZE_FACTOR);
			kRoomUserInfo.m_kGameStat.m_iDefPhysic	= (int)(sUnitStatLv1.m_usDefPhysic)		+ (int)((float)(kModifiedBaseStatBySkill.m_iDefPhysic	- sUnitStatLv1.m_usDefPhysic)   * RESIZE_FACTOR);
			kRoomUserInfo.m_kGameStat.m_iDefMagic	= (int)(sUnitStatLv1.m_usDefMagic)		+ (int)((float)(kModifiedBaseStatBySkill.m_iDefMagic	- sUnitStatLv1.m_usDefMagic)    * RESIZE_FACTOR);


			// ��ų�� �������� �⺻������ ������ ���°� ����Ʈ�̹Ƿ� 0�������� ���δ�
			float fTotalIncHPRate = 0.0f;
			KStat kStat;

			// ��ų���� �߰�
			m_kSkillTree.GetSkillStat( kStat );
			kStat.MultiplyStat( RESIZE_FACTOR );
			kRoomUserInfo.m_kGameStat.AddStat( kStat );

			//{{ 2009. 11. 26  ������	��彺ų
			m_kUserGuildManager.GetGuildSkillStat( kStat );
			kStat.MultiplyStat( RESIZE_FACTOR );
			kRoomUserInfo.m_kGameStat.AddStat( kStat );					

			m_kInventory.GetEquippedStat( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );

			//{{ 2012. 09. 17	�ڼ���	2012 ���� ����2
			// ���� �������� hp ���� �ɼ��� RESIZE_FACTOR �������� �ʴ´�. ���� hp ��� �κп��� ����Ǵ� ���� ������ �����Ѵ�.
			kStat.MultiplyAttPhysic( RESIZE_FACTOR );
			kStat.MultiplyAtkMagic( RESIZE_FACTOR );
			kStat.MultiplyDefPhysic( RESIZE_FACTOR );
			kStat.MultiplyDefMagic( RESIZE_FACTOR );

			kRoomUserInfo.m_kGameStat.AddStat( kStat );

			m_kUserTitleManager.GetEquippedStat( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );

			kStat.MultiplyStat( RESIZE_FACTOR );
			kRoomUserInfo.m_kGameStat.AddStat( kStat );

			//{{ 2009. 11. 16  ������	�����������Ʈ - (����+���+Īȣ+�нú꽺ų ���ʽ� ��� ����Ȱ�)
			const int iTotalCalcBaseHP = kRoomUserInfo.m_kGameStat.m_iBaseHP + kModifiedBaseStatBySkill.m_iBaseHP;

			kRoomUserInfo.m_kGameStat.m_iBaseHP	= (int)( sUnitStatLv1.m_uiHP * 2.7f ) + (int)((float)( iTotalCalcBaseHP - sUnitStatLv1.m_uiHP ) * 0.6f );
			//}}
		}
		break;

	case KPVPChannelInfo::PCC_FREE:
	case KPVPChannelInfo::PCC_TOURNAMENT:
		{
			//�⺻ �ɷ�ġ�� 1�� �������� gap ���̸� ���δ�
			KStatTable::KUnitStatInfo sUnitStatLv1;
			SiKStatTable()->GetUnitStat( GetUnitClass(), 1,			sUnitStatLv1	);

			kRoomUserInfo.m_kGameStat.m_iBaseHP		= (int)((float)sUnitStatLv1.m_uiHP * 2.7f); // ����ä�ΰ� ��ȸä���� 1���� HP�� 2.7�� ���Ѵ�.
			kRoomUserInfo.m_kGameStat.m_iAtkPhysic	= (int)(sUnitStatLv1.m_usAtkPhysic);
			kRoomUserInfo.m_kGameStat.m_iAtkMagic	= (int)(sUnitStatLv1.m_usAtkMagic);
			kRoomUserInfo.m_kGameStat.m_iDefPhysic	= (int)(sUnitStatLv1.m_usDefPhysic);
			kRoomUserInfo.m_kGameStat.m_iDefMagic	= (int)(sUnitStatLv1.m_usDefMagic);
		}
		break;

	case KPVPChannelInfo::PCC_NONE:
		{
			START_LOG( clog, L"���� ä�� ������ �̻��մϴ�!" )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOG( GetRoomListID() )
				<< BUILD_LOG( GetRoomUID() )
				<< END_LOG;
		}
		break;
	}
}

void KGSUser::CalculateDungeonUnitStat( IN bool bRevisionDungeon, 
									    IN KStat& kModifiedBaseStatBySkill,
										IN bool bIsDungeonRoom, 
										IN unsigned int uiHP_OnePoint, 
										IN unsigned int uiUnitLevel, 
										IN OUT KRoomUserInfo& kRoomUserInfo,
										IN OUT KStatIncreaseRate& kStatIncreaseRateInfo )
{
	//{{ 2012. 04. 23	������	��Ʋ�ʵ� �ý���
	if( bRevisionDungeon == true )			
	{
		//�⺻ �ɷ�ġ�� 1�� �������� gap ���̸� ���δ�
		KStatTable::KUnitStatInfo sUnitStatLv1;

		//{{ 2012. 05. 02	�ڼ���	����� �� ����
		uiUnitLevel = 1;
		SiKStatTable()->GetUnitStat( GetUnitClass(), uiUnitLevel,			sUnitStatLv1	);
		//}}

		kRoomUserInfo.m_kGameStat.m_iBaseHP		= (int)(sUnitStatLv1.m_uiHP);
		kRoomUserInfo.m_kGameStat.m_iAtkPhysic	= (int)(sUnitStatLv1.m_usAtkPhysic);
		kRoomUserInfo.m_kGameStat.m_iAtkMagic	= (int)(sUnitStatLv1.m_usAtkMagic);
		kRoomUserInfo.m_kGameStat.m_iDefPhysic	= (int)(sUnitStatLv1.m_usDefPhysic);
		kRoomUserInfo.m_kGameStat.m_iDefMagic	= (int)(sUnitStatLv1.m_usDefMagic);

		//{{ 2011. 07. 08    ��μ�    �ɼ� ��ġȭ
		uiHP_OnePoint = CXSLUnit::UPHP_INIT; // 1 ������ ���� ������ ����

		// GM�̶�� GM����� ����޵��� ����!
		if( GetAuthLevel() >= SEnum::UAL_GM )
		{
			// ���������� ���� �߰�
			KStat kStat;
			m_kInventory.GetEquippedStatOnlyGMWeapon( kStat );
			kRoomUserInfo.m_kGameStat.AddStat( kStat );
		}
	}
	else
	{
		kRoomUserInfo.m_kGameStat = kModifiedBaseStatBySkill;

		float fTotalIncHPRate = 0.0f;
		KStat kStat;

		// ��ų���� �߰�
		m_kSkillTree.GetSkillStat( kStat );
		kRoomUserInfo.m_kGameStat.AddStat( kStat );

		//{{ 2009. 11. 26  ������	��彺ų

		m_kUserGuildManager.GetGuildSkillStat( kStat );
		kRoomUserInfo.m_kGameStat.AddStat( kStat );

		// ���������� ���� �߰�
		m_kInventory.GetEquippedStat( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );
		kRoomUserInfo.m_kGameStat.AddStat( kStat );

		// Īȣ
		m_kUserTitleManager.GetEquippedStat( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );
		kRoomUserInfo.m_kGameStat.AddStat( kStat );

		// �� ���� ��ų(add stat)
		m_kUserPetManager.GetStatByPetSkill( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );
		kRoomUserInfo.m_kGameStat.AddStat( kStat );

		// �� ���� ��ų(increase rate)
		m_kUserPetManager.GetStatIncreaseRateByPetSkill( bIsDungeonRoom, kStatIncreaseRateInfo );
	}
}

void KGSUser::CalculateBattleFieldUnitStat( IN KStat& kModifiedBaseStatBySkill, 
										    IN bool bIsDungeonRoom, 
											IN unsigned int uiHP_OnePoint, 
											IN unsigned int uiUnitLevel, 
											IN OUT KRoomUserInfo& kRoomUserInfo, 
											IN OUT KStatIncreaseRate& kStatIncreaseRateInfo )
{
	kRoomUserInfo.m_kGameStat = kModifiedBaseStatBySkill;

	float fTotalIncHPRate = 0.0f;
	KStat kStat;

	// ��ų���� �߰�
	m_kSkillTree.GetSkillStat( kStat );
	kRoomUserInfo.m_kGameStat.AddStat( kStat );

	//{{ 2009. 11. 26  ������	��彺ų
	m_kUserGuildManager.GetGuildSkillStat( kStat );
	kRoomUserInfo.m_kGameStat.AddStat( kStat );

	// ���������� ���� �߰�
	m_kInventory.GetEquippedStat( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );
	kRoomUserInfo.m_kGameStat.AddStat( kStat );

	// Īȣ
	m_kUserTitleManager.GetEquippedStat( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );
	kRoomUserInfo.m_kGameStat.AddStat( kStat );

	// �� ���� ��ų(add stat)
	m_kUserPetManager.GetStatByPetSkill( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );
	kRoomUserInfo.m_kGameStat.AddStat( kStat );

	// �� ���� ��ų(increase rate)
	m_kUserPetManager.GetStatIncreaseRateByPetSkill( bIsDungeonRoom, kStatIncreaseRateInfo );
}

void KGSUser::CalculateTrainingCenterUnitStat( IN KStat& kModifiedBaseStatBySkill, 
										   IN bool bIsDungeonRoom, 
										   IN unsigned int uiHP_OnePoint, 
										   IN unsigned int uiUnitLevel, 
										   IN OUT KRoomUserInfo& kRoomUserInfo, 
										   IN OUT KStatIncreaseRate& kStatIncreaseRateInfo )
{
	kRoomUserInfo.m_kGameStat = kModifiedBaseStatBySkill;

	float fTotalIncHPRate = 0.0f;
	KStat kStat;

	// ��ų���� �߰�
	m_kSkillTree.GetSkillStat( kStat );
	kRoomUserInfo.m_kGameStat.AddStat( kStat );

	//{{ 2009. 11. 26  ������	��彺ų
	m_kUserGuildManager.GetGuildSkillStat( kStat );
	kRoomUserInfo.m_kGameStat.AddStat( kStat );

	// ���������� ���� �߰�
	m_kInventory.GetEquippedStat( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );
	kRoomUserInfo.m_kGameStat.AddStat( kStat );

	// Īȣ
	m_kUserTitleManager.GetEquippedStat( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );
	kRoomUserInfo.m_kGameStat.AddStat( kStat );

	// �� ���� ��ų(add stat)
	m_kUserPetManager.GetStatByPetSkill( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );
	kRoomUserInfo.m_kGameStat.AddStat( kStat );

	// �� ���� ��ų(increase rate)
	m_kUserPetManager.GetStatIncreaseRateByPetSkill( bIsDungeonRoom, kStatIncreaseRateInfo );
}

void KGSUser::CalculateSkillUnitStat( IN OUT KRoomUserInfo& kRoomUserInfo, IN OUT KStatIncreaseRate& kStatIncreaseRateInfo )
{
	//{{ 2010. 10. 27	������	�� ���� ��ų
	{
		kRoomUserInfo.m_kGameStat.m_iAtkPhysic	+= static_cast<int>( static_cast<float>(kRoomUserInfo.m_kGameStat.m_iAtkPhysic) * kStatIncreaseRateInfo.m_fIncreaseAtkPhysicRate );
		kRoomUserInfo.m_kGameStat.m_iAtkMagic	+= static_cast<int>( static_cast<float>(kRoomUserInfo.m_kGameStat.m_iAtkMagic) * kStatIncreaseRateInfo.m_fIncreaseAtkMagicRate );
		kRoomUserInfo.m_kGameStat.m_iDefPhysic	+= static_cast<int>( static_cast<float>(kRoomUserInfo.m_kGameStat.m_iDefPhysic) * kStatIncreaseRateInfo.m_fIncreaseDefPhysicRate );
		kRoomUserInfo.m_kGameStat.m_iDefMagic	+= static_cast<int>( static_cast<float>(kRoomUserInfo.m_kGameStat.m_iDefMagic) * kStatIncreaseRateInfo.m_fIncreaseDefMagicRate );
	}

	KStat kModifiedFinalStatBySkill = kRoomUserInfo.m_kGameStat;


	//{{ 2010. 11. 24	������	���� �нú� ��ų
	{
		// oasis907 : ����� [2010.12.3] // �� �����̾� - �������� ����
		const int iSkillLevel = m_kSkillTree.GetSkillLevel( (int) CXSLSkillTree::SI_P_ERS_MAGIC_RESISTANCE );
		if( iSkillLevel > 0 )
		{
			const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( (int)CXSLSkillTree::SI_P_ERS_MAGIC_RESISTANCE );
			if( NULL != pSkillTemplet )
			{
				float fRate = pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_DEF_MAGIC_REL, iSkillLevel );
				kModifiedFinalStatBySkill.m_iDefMagic = (int) ((float)kRoomUserInfo.m_kGameStat.m_iDefMagic * fRate );
			}
		}
	}

	//{{ kimhc // 2010-12-14 // ��ȭ�� �г� - ���� �нú�
	{
		const int iSkillLevel = m_kSkillTree.GetSkillLevel( CXSLSkillTree::SI_P_RRF_SPIRITUALIZED_FURY );
		if( iSkillLevel > 0)
		{
			const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( CXSLSkillTree::SI_P_RRF_SPIRITUALIZED_FURY );
			if( pSkillTemplet != NULL )
			{
				float fIncreasingRate = CXSLSkillTree::CalulateIncreaseingRate( pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_ATK_MAGIC_REL, iSkillLevel ) );
				kModifiedFinalStatBySkill.m_iAtkMagic += static_cast<int>( kRoomUserInfo.m_kGameStat.m_iAtkMagic * fIncreasingRate );
			}
		}
	}

	{
		// oasis907 : ����� [2011.6.28] û 2�� ���� - ���̾� �ȶ�� - ������ ����
		const int iSkillLevel = m_kSkillTree.GetSkillLevel( (int) CXSLSkillTree::SI_P_CIP_IRON_WILL );
		if( iSkillLevel > 0 )
		{
			const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( (int)CXSLSkillTree::SI_P_CIP_IRON_WILL );
			if( NULL != pSkillTemplet )
			{
				float fIncreasingRate = CXSLSkillTree::CalulateIncreaseingRate( pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_ATK_PHYSIC_REL, iSkillLevel ) );

				kModifiedFinalStatBySkill.m_iAtkPhysic += static_cast<int>( kRoomUserInfo.m_kGameStat.m_iAtkPhysic * fIncreasingRate );
			}
		}
	}

	//{{2010. 10. 27	����ȯ	���̺� �����̵� ������ ������ ���� - �¸��� �ڽŰ� ���� ���ݷ� ���� ȿ�� �߰�
	{
		const int iSkillLevel = m_kSkillTree.GetSkillLevel( (int) CXSLSkillTree::SI_P_RRF_CONFIDENCE_VICTORY );
		if( iSkillLevel > 0 )
		{
			const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( (int)CXSLSkillTree::SI_P_RRF_CONFIDENCE_VICTORY );
			if( NULL != pSkillTemplet )
			{
				float fIncreasingRate = CXSLSkillTree::CalulateIncreaseingRate( pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_ATK_PHYSIC_REL, iSkillLevel ) );

				kModifiedFinalStatBySkill.m_iAtkPhysic += static_cast<int>( kRoomUserInfo.m_kGameStat.m_iAtkPhysic * fIncreasingRate );
			}
		}
	}

	{
		const int iSkillLevel = m_kSkillTree.GetSkillLevel( CXSLSkillTree::SI_P_AVP_MAGIC_AMPLIFICATION );
		if( iSkillLevel > 0 )
		{
			const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( CXSLSkillTree::SI_P_AVP_MAGIC_AMPLIFICATION );

			if( NULL != pSkillTemplet )
			{
				float fIncreasingRate = CXSLSkillTree::CalulateIncreaseingRate( pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_ATK_MAGIC_REL, iSkillLevel ) );
				
				kModifiedFinalStatBySkill.m_iAtkMagic += static_cast<int>( kRoomUserInfo.m_kGameStat.m_iAtkMagic * fIncreasingRate );
			}
		}
	}

	{
		const int iSkillLevel = m_kSkillTree.GetSkillLevel( CXSLSkillTree::SI_P_ERS_RUNE_MASTERY );
		if( iSkillLevel > 0 )
		{
			const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( CXSLSkillTree::SI_P_ERS_RUNE_MASTERY );

			if( NULL != pSkillTemplet )
			{
				float fIncreasingRate = CXSLSkillTree::CalulateIncreaseingRate( pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_ATK_MAGIC_REL, iSkillLevel ) );

				kModifiedFinalStatBySkill.m_iAtkMagic += static_cast<int>( kRoomUserInfo.m_kGameStat.m_iAtkMagic * fIncreasingRate );
			}
		}
	}

	// ȣ�Ű���
	{
		const int iSkillLevel = m_kSkillTree.GetSkillLevel( CXSLSkillTree::SI_P_ASD_SELF_PROTECTION_FORTITUDE );
		if( iSkillLevel > 0 )
		{
			const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( CXSLSkillTree::SI_P_ASD_SELF_PROTECTION_FORTITUDE );

			if( NULL != pSkillTemplet )
			{
				// ����
				{
					float fRate = CXSLSkillTree::CalulateIncreaseingRate( pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_DEF_PHYSIC_REL, iSkillLevel ) );
					kModifiedFinalStatBySkill.m_iDefPhysic += static_cast<int>( kRoomUserInfo.m_kGameStat.m_iDefPhysic * fRate );
				}
				// ����
				{
					float fRate = CXSLSkillTree::CalulateIncreaseingRate( pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_DEF_MAGIC_REL, iSkillLevel ) );
					kModifiedFinalStatBySkill.m_iDefMagic += static_cast<int>( kRoomUserInfo.m_kGameStat.m_iDefMagic * fRate );
				}
				// HP ������ KUserSkillTree::CheckAddSkillStat_BaseHP() ���⼭ ó��
			}
		}
	}

	kRoomUserInfo.m_kGameStat = kModifiedFinalStatBySkill;
}

void KGSUser::CheckInventoryExpMedal( IN OUT KRoomUserInfo& kRoomUserInfo )
{
	//080421.hoons. �κ����� �ູ�� �޴� �������� �˻��Ѵ�.
	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_BLESSING_MEDAL ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, SiKGameSysVal()->GetPremiumEXPRate() );
	}

	// [�̺�Ʈ] 081220 ~ 081221 yuksa. ��ĳ���� ��� ����ġ �޴�
	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_EXP_15_PERCENT_MEDAL ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 0.15f );
	}

	//{{ 2009. 3. 30  ������	����ġ 100% �޴�
	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_EXP_100_PERCENT_MEDAL ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 1.0f );
	}

	//{{ 2011. 10. 19    ��μ�    200% ����ġ �߰� �޴�(�̺�Ʈ ������)
	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_EXP_200_PERCENT_MEDAL ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 2.0f );
	}
	
#ifdef NEW_YEAR_2013_EVENT
	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_EXP_10_PERCENT_MEDAL ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 0.10f );
	}
#endif NEW_YEAR_2013_EVENT

#ifdef SERV_EXP_MEDAL_JP
	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_EXP_30_PERCENT_MEDAL ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 0.30f );
	}

	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_EXP_10_PERCENT_MEDAL365 ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 0.10f );
	}

	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_EXP_10_PERCENT_MEDAL1 ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 0.10f );
	}

	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_EXP_50_PERCENT_MEDAL_JP ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 0.50f );
	}

	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_EXP_30_PERCENT_MEDAL_JP ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 0.30f );
	}
#endif

#ifdef SERV_VIP_SYSTEM // �κ����� VIP ƼĹ�� �ִ��� �˻� �Ѵ�.
	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_VIP_TICKET ) == true)
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, SiKGameSysVal()->GetVIPEXPRate() );
	}
#endif //SERV_VIP_SYSTEM

#ifdef SERV_EVENT_VIP_SYSTEM
	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_EVENT_VIP_TICKET ) == true)
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 0.05 );
	}
#endif //SERV_EVENT_VIP_SYSTEM

#ifdef SERV_EVENT_ITEM_US
	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_STARTER_EXP_15_PERCENT_MEDAL ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 0.15f );
	}

	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_EXP_50_PERCENT_MEDAL ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 0.5f );
	}

	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_EXP_30_PERCENT_MEDAL_1DAY ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 0.3f );
	}

	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_EXP_10_PERCENT_MEDAL_US ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 0.1f );
	}
#endif SERV_EVENT_ITEM_US

#ifdef SERV_EXP_MEDAL_TW
	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_EXP_200_PERCENT_MEDAL_TWHK ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 2.0f );
	}

	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_EXP_50_PERCENT_MEDAL_TWHK ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 0.5f );
	}
#endif SERV_EXP_MEDAL_TW

#ifdef SERV_EXP_MEDAL_CN
	if( m_kInventory.IsExistOnlyInventory( 85001972 ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 0.1f );
	}

	if( m_kInventory.IsExistOnlyInventory( 80001331 ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 0.5f );
	}

	if( m_kInventory.IsExistOnlyInventory( 80001473 ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 0.3f );
	}
#endif SERV_EXP_MEDAL_CN
}
#else	// SERV_UPGRADE_SKILL_SYSTEM_2013
/*
void KGSUser::GetRoomUserInfo( OUT KRoomUserInfo& kRoomUserInfo, IN const int iRoomType_ )
{
	kRoomUserInfo.m_iGSUID		   = KBaseServer::GetKObj()->GetUID();
	kRoomUserInfo.m_iOwnerUserUID  = GetUID();
	kRoomUserInfo.m_cAuthLevel	   = GetAuthLevel();
	//{{ 2011. 12. 06	������	Ư�� ������ �̺�Ʈ �ý���
#ifdef SERV_SERVER_GROUP_EVENT_SYSTEM
	kRoomUserInfo.m_sServerGroupID = KBaseServer::GetKObj()->GetServerGroupID();
#endif SERV_SERVER_GROUP_EVENT_SYSTEM
	//}}
	kRoomUserInfo.m_bMale		   = m_kNexonAccountInfo.m_bSex;
	kRoomUserInfo.m_ucAge		   = m_kNexonAccountInfo.m_uiAge;
	kRoomUserInfo.m_nUnitUID	   = GetCharUID();
	kRoomUserInfo.m_uiKNMSerialNum = m_uiKNMSerialNum;
	kRoomUserInfo.m_cUnitClass	   = GetUnitClass();
	//{{ 2012. 04. 17	������	���� �α� �÷� �߰�
#ifdef SERV_ADD_DUNGEON_LOG_COLUMN_NUM_2
	kRoomUserInfo.m_iNumResurrectionStone = m_iNumResurrectionStone;
#endif SERV_ADD_DUNGEON_LOG_COLUMN_NUM_2
	//}}
	//{{ 2011. 07. 11	������	���� ����
	//#ifdef SERV_PVP_NEW_SYSTEM
	kRoomUserInfo.m_iOfficialMatchCnt = m_kUserPvpManager.GetOfficialMatchCount();
	kRoomUserInfo.m_iRating		   = GetRating();
	kRoomUserInfo.m_iMaxRating	   = m_kUserPvpManager.GetMaxRating();
	kRoomUserInfo.m_bIsWinBeforeMatch = m_kUserPvpManager.IsWinBeforeMatch();

	//{{ 2012. 06. 25	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2
	kRoomUserInfo.m_cRank					= GetPvpRankForClient();
	kRoomUserInfo.m_cRankForServer			= GetPvpRank();
	kRoomUserInfo.m_fKFactor				= m_kUserPvpManager.GetKFactor();
	kRoomUserInfo.m_bIsRedistributionUser	= m_kUserPvpManager.IsRedistributionUser();
#endif SERV_2012_PVP_SEASON2
	//}}

	kRoomUserInfo.m_wstrNickName   = GetCharName();

	kRoomUserInfo.m_wstrIP		   = GetP2PIP();
	kRoomUserInfo.m_usPort		   = GetP2PPort();

	//{{ 2013. 1. 8	�ڼ���	Merge ����IP ���� ���н� ����IP�� �õ�( ������ )
#ifdef SERV_RETRY_USING_INTERNAL_IP
	kRoomUserInfo.m_wstrInternalIP	= GetP2PInternalIP();
	kRoomUserInfo.m_usInternalPort	= GetP2PInternalPort();

	START_LOG( clog, L"IP, ��Ʈ Ȯ��" )
		<< BUILD_LOG( kRoomUserInfo.m_wstrIP )
		<< BUILD_LOG( kRoomUserInfo.m_usPort )
		<< BUILD_LOG( kRoomUserInfo.m_wstrInternalIP )
		<< BUILD_LOG( kRoomUserInfo.m_usInternalPort )
		<< END_LOG;
#endif SERV_RETRY_USING_INTERNAL_IP
	//}}

	//{{ 2011. 11. 1	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	kRoomUserInfo.m_iPartyUID	   = GetPartyUID();
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}

	kRoomUserInfo.m_ucLevel		   = GetLevel();
	kRoomUserInfo.m_bIsGuestUser   = IsGuestUser();

	//{{ 2009. 9. 25  ������	���
#ifdef GUILD_TEST
	kRoomUserInfo.m_iGuildUID	   = GetGuildUID();
	kRoomUserInfo.m_wstrGuildName  = m_kUserGuildManager.GetGuildName();
	kRoomUserInfo.m_ucMemberShipGrade = m_kUserGuildManager.GetMemberShipGrade();
#endif GUILD_TEST
	//}}
	//{{ 2011. 05. 27    ��μ�    �޸� ���� ���� ����
#ifdef SERV_COME_BACK_USER_REWARD
	kRoomUserInfo.m_bComeBackUser	= m_kComeBackUserInfo.GetIsComeBackUser();
#endif SERV_COME_BACK_USER_REWARD
	//}} 

	//{{ 2012. 09. 19   ��μ�   ĳ�ü� �湮 ����
#ifdef SERV_VISIT_CASH_SHOP
	kRoomUserInfo.m_bEnterCashShop	= m_kUserUnitManager.GetEnterCashShop();
#endif SERV_VISIT_CASH_SHOP
	//}}
	//{{ 2011. 07. 08    ��μ�    �ɼ� ��ġȭ
#ifdef SERV_USE_PERCENT_IN_OPTION_DATA
	unsigned int uiHP_OnePoint = CXSLUnit::UPHP_INIT + ( CXSLUnit::UPHP_INCREASE_ONE_POINT * (GetLevel() - 1)); // ���� ������ 1���� ��
	unsigned int uiUnitLevel = static_cast<unsigned int>(GetLevel());	//	���� ���� �̸� ���
#endif SERV_USE_PERCENT_IN_OPTION_DATA
	//}} 

	//080611.hoons.�������� �������� �˻��Ͽ� ȿ�������� �����ϱ�����.
	kRoomUserInfo.m_bIsRingofpvprebirth = m_kInventory.IsEquippedItem( CXSLItem::SI_RING_OF_PVP_REBIRTH ); //::1st. ����������

	bool bAllSkillLevelUp = false;

	//�⺻ ����
	KStat kModifiedBaseStatBySkill = m_kStat;

	//{{ 2010. 10. 27	������	�� ���� ��ų
#ifdef SERV_PET_AURA_SKILL
	KStatIncreaseRate kStatIncreaseRateInfo;
#endif SERV_PET_AURA_SKILL
	//}}

	////////////////////////////////////////////////////////////////////////// [����!]
	// ��ų�߿� BaseHP�� ������Ű�� �ͺ��� �̸� �������!
	{
		const int iSkillLevel = m_kSkillTree.GetSkillLevel( (int) CXSLSkillTree::SI_P_COMMON_POWERFUL_VITAL );
		if( iSkillLevel > 0 )
		{
			const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( (int)CXSLSkillTree::SI_P_COMMON_POWERFUL_VITAL, iSkillLevel );
			if( NULL != pSkillTemplet )
			{
				kModifiedBaseStatBySkill.m_iBaseHP += static_cast<int>( m_kStat.m_iBaseHP * CXSLSkillTree::CalulateIncreaseingRate( pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_MAX_HP_REL ) ) );
			}
		}

		//{{ kimhc // 2011.1.14 // û 1�� ����, ǻ��������� ��� ����
		const int iSkillLevelGuardMastery = m_kSkillTree.GetSkillLevel( static_cast<int>( CXSLSkillTree::SI_P_CFG_GUARD_MASTERY ) );
		if ( 0 < iSkillLevelGuardMastery )
		{
			const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( static_cast<int>( CXSLSkillTree::SI_P_CFG_GUARD_MASTERY ), iSkillLevelGuardMastery );
			if ( NULL != pSkillTemplet )
			{
				kModifiedBaseStatBySkill.m_iBaseHP += static_cast<int>( m_kStat.m_iBaseHP * CXSLSkillTree::CalulateIncreaseingRate( pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_MAX_HP_REL ) ) );
			} // if
		} // if
		//}} kimhc // 2011.1.14 // û 1�� ����, ǻ��������� ��� ����
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// RoomType ���
	CXSLRoom::ROOM_TYPE eRoomType = static_cast<CXSLRoom::ROOM_TYPE>(iRoomType_);
	if( iRoomType_ < 0 )
	{
		if( GetRoomUID() == 0 )
		{
			START_LOG( cerr, L"�������� RoomType�� -1�̶� RoomUID�� RoomType�� �������� �ϴµ� RoomUID�� 0�̴�?" )
				<< BUILD_LOG( iRoomType_ )
				<< BUILD_LOG( GetCharName() )
				<< END_LOG;
		}

		eRoomType = CXSLRoom::GetRoomType( GetRoomUID() );
	}

	//{{ 2012. 11. 05	�ڼ���	���� ���� ������ ��� �ʵ忡�� ����
#ifdef SERV_APPLY_DUNGEON_SOCKET_OPTION_TO_FIELD
	const bool bIsDungeonRoom = ( ( eRoomType == CXSLRoom::RT_DUNGEON ) || ( eRoomType == CXSLRoom::RT_BATTLE_FIELD ) );
#else
	const bool bIsDungeonRoom = ( eRoomType == CXSLRoom::RT_DUNGEON );
#endif SERV_APPLY_DUNGEON_SOCKET_OPTION_TO_FIELD
	//}}
	switch( eRoomType )
	{
	case CXSLRoom::RT_PVP:
		{
			//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
			m_kUserBuffManager.GetBuffList( kRoomUserInfo.m_vecBuffInfo, CXSLRoom::RT_PVP );
#endif SERV_SERVER_BUFF_SYSTEM
			//}

			int iPVPChannelClass = 0;
			//LIF( GetPVPChannelClass( iPVPChannelClass ) );
			GetPVPChannelClass( iPVPChannelClass );
			switch( iPVPChannelClass )
			{
			default:
			case KPVPChannelInfo::PCC_PLAY:
				{
					// �Ϲ�ä�ΰ� ���ä���� 10% �ɷ�ġ�� �������ش�.
					const float RESIZE_FACTOR = 0.1f;

					// �⺻ �ɷ�ġ�� 1�� �������� gap ���̸� ���δ�
					KStatTable::KUnitStatInfo sUnitStatLv1;
					SiKStatTable()->GetUnitStat( GetUnitClass(), 1,	sUnitStatLv1 );

					// �ѹ�� : kRoomUserInfo.m_kGameStat.m_iBaseHP		= (int)(sUnitStatLv1.m_uiHP)			+ (int)((float)(kModifiedBaseStatBySkill.m_iBaseHP		- sUnitStatLv1.m_uiHP)          * fResizeFactor);
					//}}
					kRoomUserInfo.m_kGameStat.m_iAtkPhysic	= (int)(sUnitStatLv1.m_usAtkPhysic)		+ (int)((float)(kModifiedBaseStatBySkill.m_iAtkPhysic	- sUnitStatLv1.m_usAtkPhysic)   * RESIZE_FACTOR);
					kRoomUserInfo.m_kGameStat.m_iAtkMagic	= (int)(sUnitStatLv1.m_usAtkMagic)		+ (int)((float)(kModifiedBaseStatBySkill.m_iAtkMagic	- sUnitStatLv1.m_usAtkMagic)    * RESIZE_FACTOR);
					kRoomUserInfo.m_kGameStat.m_iDefPhysic	= (int)(sUnitStatLv1.m_usDefPhysic)		+ (int)((float)(kModifiedBaseStatBySkill.m_iDefPhysic	- sUnitStatLv1.m_usDefPhysic)   * RESIZE_FACTOR);
					kRoomUserInfo.m_kGameStat.m_iDefMagic	= (int)(sUnitStatLv1.m_usDefMagic)		+ (int)((float)(kModifiedBaseStatBySkill.m_iDefMagic	- sUnitStatLv1.m_usDefMagic)    * RESIZE_FACTOR);


					// ��ų�� �������� �⺻������ ������ ���°� ����Ʈ�̹Ƿ� 0�������� ���δ�
					float fTotalIncHPRate = 0.0f;
					KStat kStat;

					// ��ų���� �߰�
					m_kSkillTree.GetSkillStat( kStat );
					kStat.MultiplyStat( RESIZE_FACTOR );
					kRoomUserInfo.m_kGameStat.AddStat( kStat );

					//{{ 2009. 11. 26  ������	��彺ų
#ifdef GUILD_SKILL_TEST					
					m_kUserGuildManager.GetGuildSkillStat( kStat );
					kStat.MultiplyStat( RESIZE_FACTOR );
					kRoomUserInfo.m_kGameStat.AddStat( kStat );					
#endif GUILD_SKILL_TEST
					//}}					

					m_kInventory.GetEquippedStat( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );

					//{{ 2012. 09. 17	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2
					// ���� �������� hp ���� �ɼ��� RESIZE_FACTOR �������� �ʴ´�. ���� hp ��� �κп��� ����Ǵ� ���� ������ �����Ѵ�.
					kStat.MultiplyAttPhysic( RESIZE_FACTOR );
					kStat.MultiplyAtkMagic( RESIZE_FACTOR );
					kStat.MultiplyDefPhysic( RESIZE_FACTOR );
					kStat.MultiplyDefMagic( RESIZE_FACTOR );
#else
					kStat.MultiplyStat( RESIZE_FACTOR );
#endif SERV_2012_PVP_SEASON2
					//}}
					kRoomUserInfo.m_kGameStat.AddStat( kStat );

					m_kUserTitleManager.GetEquippedStat( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );

					kStat.MultiplyStat( RESIZE_FACTOR );
					kRoomUserInfo.m_kGameStat.AddStat( kStat );

					//{{ 2009. 11. 16  ������	�����������Ʈ - (����+���+Īȣ+�нú꽺ų ���ʽ� ��� ����Ȱ�)
					const int iTotalCalcBaseHP = kRoomUserInfo.m_kGameStat.m_iBaseHP + kModifiedBaseStatBySkill.m_iBaseHP;

					kRoomUserInfo.m_kGameStat.m_iBaseHP	= (int)( sUnitStatLv1.m_uiHP * 2.7f ) + (int)((float)( iTotalCalcBaseHP - sUnitStatLv1.m_uiHP ) * 0.6f );
					//}}
				}
				break;

			case KPVPChannelInfo::PCC_FREE:
			case KPVPChannelInfo::PCC_TOURNAMENT:
				{
					//�⺻ �ɷ�ġ�� 1�� �������� gap ���̸� ���δ�
					KStatTable::KUnitStatInfo sUnitStatLv1;
					SiKStatTable()->GetUnitStat( GetUnitClass(), 1,			sUnitStatLv1	);

					kRoomUserInfo.m_kGameStat.m_iBaseHP		= (int)((float)sUnitStatLv1.m_uiHP * 2.7f); // ����ä�ΰ� ��ȸä���� 1���� HP�� 2.7�� ���Ѵ�.
					kRoomUserInfo.m_kGameStat.m_iAtkPhysic	= (int)(sUnitStatLv1.m_usAtkPhysic);
					kRoomUserInfo.m_kGameStat.m_iAtkMagic	= (int)(sUnitStatLv1.m_usAtkMagic);
					kRoomUserInfo.m_kGameStat.m_iDefPhysic	= (int)(sUnitStatLv1.m_usDefPhysic);
					kRoomUserInfo.m_kGameStat.m_iDefMagic	= (int)(sUnitStatLv1.m_usDefMagic);
				}
				break;

			case KPVPChannelInfo::PCC_NONE:
				{
					START_LOG( clog, L"���� ä�� ������ �̻��մϴ�!" )
						<< BUILD_LOG( GetCharUID() )
						<< BUILD_LOG( GetRoomListID() )
						<< BUILD_LOG( GetRoomUID() )
						<< END_LOG;
				}
				break;
			}

			//////////////////////////////////////////////////////////////////////////
			//{{ 2010. 05. 11  ������	���� ���� ������ ����
#ifdef SERV_INTEGRATION
			SiKGameEventManager()->GetRewardEvent( kRoomUserInfo, 0, GetLevel(), GetUnitClass() );
#endif SERV_INTEGRATION
			//}}
			//////////////////////////////////////////////////////////////////////////
		}
		break;

	case CXSLRoom::RT_DUNGEON:
		{
			//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
			m_kUserBuffManager.GetBuffList( kRoomUserInfo.m_vecBuffInfo, CXSLRoom::RT_DUNGEON );
#endif SERV_SERVER_BUFF_SYSTEM
			//}

			//{{ 2012. 04. 23	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
			if( IsHenirDungeonChallengeMode()  ||  CXSLDungeon::IsEventDungeon( m_kUserDungeonManager.GetDungeonID() ) )
#endif SERV_BATTLE_FIELD_SYSTEM
				//}}			
			{
				//�⺻ �ɷ�ġ�� 1�� �������� gap ���̸� ���δ�
				KStatTable::KUnitStatInfo sUnitStatLv1;

				//{{ 2012. 05. 02	�ڼ���	����� �� ����
#ifdef SERV_REFORM_THE_GATE_OF_DARKNESS
				uiUnitLevel = 1;
				SiKStatTable()->GetUnitStat( GetUnitClass(), uiUnitLevel,			sUnitStatLv1	);
#else
				SiKStatTable()->GetUnitStat( GetUnitClass(), 1,			sUnitStatLv1	);
#endif SERV_REFORM_THE_GATE_OF_DARKNESS
				//}}

				kRoomUserInfo.m_kGameStat.m_iBaseHP		= (int)(sUnitStatLv1.m_uiHP);
				kRoomUserInfo.m_kGameStat.m_iAtkPhysic	= (int)(sUnitStatLv1.m_usAtkPhysic);
				kRoomUserInfo.m_kGameStat.m_iAtkMagic	= (int)(sUnitStatLv1.m_usAtkMagic);
				kRoomUserInfo.m_kGameStat.m_iDefPhysic	= (int)(sUnitStatLv1.m_usDefPhysic);
				kRoomUserInfo.m_kGameStat.m_iDefMagic	= (int)(sUnitStatLv1.m_usDefMagic);

				//{{ 2011. 07. 08    ��μ�    �ɼ� ��ġȭ
#ifdef SERV_USE_PERCENT_IN_OPTION_DATA
				uiHP_OnePoint = CXSLUnit::UPHP_INIT; // 1 ������ ���� ������ ����
#endif SERV_USE_PERCENT_IN_OPTION_DATA
				//}} 

				// GM�̶�� GM����� ����޵��� ����!
				if( GetAuthLevel() >= SEnum::UAL_GM )
				{
					// ���������� ���� �߰�
					KStat kStat;
					m_kInventory.GetEquippedStatOnlyGMWeapon( kStat );
					kRoomUserInfo.m_kGameStat.AddStat( kStat );
				}
			}
			else
			{
				kRoomUserInfo.m_kGameStat = kModifiedBaseStatBySkill;

				float fTotalIncHPRate = 0.0f;
				KStat kStat;

				// ��ų���� �߰�
				m_kSkillTree.GetSkillStat( kStat );
				kRoomUserInfo.m_kGameStat.AddStat( kStat );

				//{{ 2009. 11. 26  ������	��彺ų
#ifdef GUILD_SKILL_TEST
				m_kUserGuildManager.GetGuildSkillStat( kStat );
				kRoomUserInfo.m_kGameStat.AddStat( kStat );
#endif GUILD_SKILL_TEST
				//}}

				// ���������� ���� �߰�
				m_kInventory.GetEquippedStat( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );
				kRoomUserInfo.m_kGameStat.AddStat( kStat );

				// Īȣ
				m_kUserTitleManager.GetEquippedStat( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );
				kRoomUserInfo.m_kGameStat.AddStat( kStat );

				// �� ���� ��ų(add stat)
				m_kUserPetManager.GetStatByPetSkill( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );
				kRoomUserInfo.m_kGameStat.AddStat( kStat );

				// �� ���� ��ų(increase rate)
				m_kUserPetManager.GetStatIncreaseRateByPetSkill( bIsDungeonRoom, kStatIncreaseRateInfo );
			}

			//{{ 2009. 2. 2  ������		���Ͽɼ� ���� ���ʽ�
			float fAddSocketOptExpRate	= 0.0f;
			float fAddSocketOptEDRate	= 0.0f;

			// ���� ������ ���� �ɼ�
			m_kInventory.GetEquippedItemBonusRate( bIsDungeonRoom, fAddSocketOptExpRate, fAddSocketOptEDRate );

			// Īȣ ���� �ɼ�
			m_kUserTitleManager.GetEquippedTitleBonusRate( bIsDungeonRoom, fAddSocketOptExpRate, fAddSocketOptEDRate );
			//}}

			if( fAddSocketOptExpRate > 0.0f )
			{
				kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_SOCKET_OPT_EXP_RATE, fAddSocketOptExpRate );
			}
			if( fAddSocketOptEDRate > 0.0f )
			{
				kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_SOCKET_OPT_ED_RATE, fAddSocketOptEDRate );
			}

			//////////////////////////////////////////////////////////////////////////
			//{{ 2010. 05. 11  ������	���� ���� ������ ����
#ifdef SERV_INTEGRATION
			//{{ 2012. 04. 23	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
			SiKGameEventManager()->GetRewardEvent( kRoomUserInfo, m_kUserDungeonManager.GetDungeonID(), GetLevel(), GetUnitClass(), 1  );
#else
			SiKGameEventManager()->GetRewardEvent( kRoomUserInfo, iDungeonID, GetLevel(), GetUnitClass(), iPartyNumMember );
#endif SERV_BATTLE_FIELD_SYSTEM
			//}}
#endif SERV_INTEGRATION
			//}}
			//////////////////////////////////////////////////////////////////////////
		}
		break;

		//{{ 2011. 11. 7	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	case CXSLRoom::RT_BATTLE_FIELD:
		{
			//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
			m_kUserBuffManager.GetBuffList( kRoomUserInfo.m_vecBuffInfo, CXSLRoom::RT_BATTLE_FIELD );
#endif SERV_SERVER_BUFF_SYSTEM
			//}

			kRoomUserInfo.m_kGameStat = kModifiedBaseStatBySkill;

			float fTotalIncHPRate = 0.0f;
			KStat kStat;

			// ��ų���� �߰�
			m_kSkillTree.GetSkillStat( kStat );
			kRoomUserInfo.m_kGameStat.AddStat( kStat );

			//{{ 2009. 11. 26  ������	��彺ų
#ifdef GUILD_SKILL_TEST
			m_kUserGuildManager.GetGuildSkillStat( kStat );
			kRoomUserInfo.m_kGameStat.AddStat( kStat );
#endif GUILD_SKILL_TEST
			//}}

			// ���������� ���� �߰�
			m_kInventory.GetEquippedStat( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );
			kRoomUserInfo.m_kGameStat.AddStat( kStat );

			// Īȣ
			m_kUserTitleManager.GetEquippedStat( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );
			kRoomUserInfo.m_kGameStat.AddStat( kStat );

			// �� ���� ��ų(add stat)
			m_kUserPetManager.GetStatByPetSkill( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );
			kRoomUserInfo.m_kGameStat.AddStat( kStat );

			// �� ���� ��ų(increase rate)
			m_kUserPetManager.GetStatIncreaseRateByPetSkill( bIsDungeonRoom, kStatIncreaseRateInfo );

			//{{ 2009. 2. 2  ������		���Ͽɼ� ���� ���ʽ�
			float fAddSocketOptExpRate	= 0.0f;
			float fAddSocketOptEDRate	= 0.0f;

			// ���� ������ ���� �ɼ�
			m_kInventory.GetEquippedItemBonusRate( bIsDungeonRoom, fAddSocketOptExpRate, fAddSocketOptEDRate );

			// Īȣ ���� �ɼ�
			m_kUserTitleManager.GetEquippedTitleBonusRate( bIsDungeonRoom, fAddSocketOptExpRate, fAddSocketOptEDRate );
			//}}

			//{{ 2010. 05. 11  ������	�̺�Ʈ ���� �ڵ� ����
			if( fAddSocketOptExpRate > 0.0f )
			{
				kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_SOCKET_OPT_EXP_RATE, fAddSocketOptExpRate );
			}
			if( fAddSocketOptEDRate > 0.0f )
			{
				kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_SOCKET_OPT_ED_RATE, fAddSocketOptEDRate );
			}

			//////////////////////////////////////////////////////////////////////////
			//{{ 2012. 12. 18	������	�ʵ忡���� ����ġ �̺�Ʈ ����
#ifdef SERV_EVENT_EXP_BONUS_IN_BATTLE_FIELD
			SiKGameEventManager()->GetRewardEvent( kRoomUserInfo, m_kUserDungeonManager.GetDungeonID(), GetLevel(), GetUnitClass(), 1 );
#endif SERV_EVENT_EXP_BONUS_IN_BATTLE_FIELD
			//}}
			//////////////////////////////////////////////////////////////////////////
		}
		break;
#endif SERV_BATTLE_FIELD_SYSTEM
		//}}

	case CXSLRoom::RT_TRAININGCENTER:
		{
			//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
			m_kUserBuffManager.GetBuffList( kRoomUserInfo.m_vecBuffInfo );
#endif SERV_SERVER_BUFF_SYSTEM
			//}

			kRoomUserInfo.m_kGameStat = kModifiedBaseStatBySkill;

			float fTotalIncHPRate = 0.0f;
			KStat kStat;

			// ��ų���� �߰�
			m_kSkillTree.GetSkillStat( kStat );
			kRoomUserInfo.m_kGameStat.AddStat( kStat );

			//{{ 2009. 11. 26  ������	��彺ų
#ifdef GUILD_SKILL_TEST
			m_kUserGuildManager.GetGuildSkillStat( kStat );
			kRoomUserInfo.m_kGameStat.AddStat( kStat );
#endif GUILD_SKILL_TEST
			//}}

			// ���������� ���� �߰�
			m_kInventory.GetEquippedStat( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );
			kRoomUserInfo.m_kGameStat.AddStat( kStat );

			// Īȣ
			m_kUserTitleManager.GetEquippedStat( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );
			kRoomUserInfo.m_kGameStat.AddStat( kStat );

			// �� ���� ��ų(add stat)
			m_kUserPetManager.GetStatByPetSkill( bIsDungeonRoom, kModifiedBaseStatBySkill, fTotalIncHPRate, kStat, uiHP_OnePoint, uiUnitLevel );
			kRoomUserInfo.m_kGameStat.AddStat( kStat );

			// �� ���� ��ų(increase rate)
			m_kUserPetManager.GetStatIncreaseRateByPetSkill( bIsDungeonRoom, kStatIncreaseRateInfo );

			//{{ 2009. 2. 2  ������		���Ͽɼ� ���� ���ʽ�
			float fAddSocketOptExpRate	= 0.0f;
			float fAddSocketOptEDRate	= 0.0f;

			// ���� ������ ���� �ɼ�
			m_kInventory.GetEquippedItemBonusRate( bIsDungeonRoom, fAddSocketOptExpRate, fAddSocketOptEDRate );

			// Īȣ ���� �ɼ�
			m_kUserTitleManager.GetEquippedTitleBonusRate( bIsDungeonRoom, fAddSocketOptExpRate, fAddSocketOptEDRate );
			//}}

			if( fAddSocketOptExpRate > 0.0f )
			{
				kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_SOCKET_OPT_EXP_RATE, fAddSocketOptExpRate );
			}
			if( fAddSocketOptEDRate > 0.0f )
			{
				kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_SOCKET_OPT_ED_RATE, fAddSocketOptEDRate );
			}
		}
		break;
		//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
	case CXSLRoom::RT_WEDDING_HALL:
		{
			kRoomUserInfo.m_kGameStat = kModifiedBaseStatBySkill;
		}break;
#endif SERV_RELATIONSHIP_SYSTEM
		//}
	default:
		{
			START_LOG( cerr, L"�������� ������ �߸��� ��Ÿ��.!" )
				<< BUILD_LOG( CXSLRoom::GetRoomType( GetRoomUID() ) )
				<< BUILD_LOG( GetCharName() )
				<< END_LOG;
		}
		break;
	}

	//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
	m_kUserBuffManager.GetBuffBonusRate( kRoomUserInfo );
#endif SERV_SERVER_BUFF_SYSTEM
	//}

	//{{ 2010. 10. 27	������	�� ���� ��ų
	//#ifdef SERV_PET_AURA_SKILL
	{
		kRoomUserInfo.m_kGameStat.m_iAtkPhysic	+= static_cast<int>( static_cast<float>(kRoomUserInfo.m_kGameStat.m_iAtkPhysic) * kStatIncreaseRateInfo.m_fIncreaseAtkPhysicRate );
		kRoomUserInfo.m_kGameStat.m_iAtkMagic	+= static_cast<int>( static_cast<float>(kRoomUserInfo.m_kGameStat.m_iAtkMagic) * kStatIncreaseRateInfo.m_fIncreaseAtkMagicRate );
		kRoomUserInfo.m_kGameStat.m_iDefPhysic	+= static_cast<int>( static_cast<float>(kRoomUserInfo.m_kGameStat.m_iDefPhysic) * kStatIncreaseRateInfo.m_fIncreaseDefPhysicRate );
		kRoomUserInfo.m_kGameStat.m_iDefMagic	+= static_cast<int>( static_cast<float>(kRoomUserInfo.m_kGameStat.m_iDefMagic) * kStatIncreaseRateInfo.m_fIncreaseDefMagicRate );
	}
	//#endif SERV_PET_AURA_SKILL
	//}}

	KStat kModifiedFinalStatBySkill = kRoomUserInfo.m_kGameStat;

	{
		const int iSkillLevel = m_kSkillTree.GetSkillLevel( (int) CXSLSkillTree::SI_P_COMMON_POWERFUL_SOUL );
		if( iSkillLevel > 0 )
		{
			const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( (int)CXSLSkillTree::SI_P_COMMON_POWERFUL_SOUL, iSkillLevel );
			if( NULL != pSkillTemplet )
			{
#ifdef SERV_BALANCE_PATCH_20130214
				kModifiedFinalStatBySkill.m_iAtkMagic += (int) ((float)kRoomUserInfo.m_kGameStat.m_iAtkPhysic * pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_ATK_PHYSIC_TO_ATK_MAGIC ) );
#else
				kModifiedFinalStatBySkill.m_iAtkPhysic += (int) ((float)kRoomUserInfo.m_kGameStat.m_iAtkMagic * pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_ATK_MAGIC_TO_ATK_PHYSIC ) );
#endif //SERV_BALANCE_PATCH_20130214
			}
		}
	}	

	{
		const int iSkillLevel = m_kSkillTree.GetSkillLevel( (int) CXSLSkillTree::SI_P_COMMON_PHYSIC_HARMONY );
		if( iSkillLevel > 0 )
		{
			const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( (int)CXSLSkillTree::SI_P_COMMON_PHYSIC_HARMONY, iSkillLevel );
			if( NULL != pSkillTemplet )
			{
				kModifiedFinalStatBySkill.m_iAtkMagic += (int) ((float)kRoomUserInfo.m_kGameStat.m_iAtkPhysic * pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_ATK_PHYSIC_TO_ATK_MAGIC ) );
			}
		}
	}

	{
		const int iSkillLevel = m_kSkillTree.GetSkillLevel( (int) CXSLSkillTree::SI_P_COMMON_FIGHTER_SOUL );
		if( iSkillLevel > 0 )
		{
			const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( (int)CXSLSkillTree::SI_P_COMMON_FIGHTER_SOUL, iSkillLevel );
			if( NULL != pSkillTemplet )
			{
				kModifiedFinalStatBySkill.m_iAtkMagic += (int) ((float)kRoomUserInfo.m_kGameStat.m_iAtkPhysic * pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_ATK_PHYSIC_TO_ATK_MAGIC ) );
			}
		}
	}
#ifdef SERV_RENA_NIGHT_WATCHER
	{
		const int iSkillLevel = m_kSkillTree.GetSkillLevel( (int) CXSLSkillTree::SI_P_RNW_GUARDIAN_OF_ELF );
		if( iSkillLevel > 0 )
		{
			const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( (int)CXSLSkillTree::SI_P_RNW_GUARDIAN_OF_ELF, iSkillLevel );
			if( NULL != pSkillTemplet )
			{
				kModifiedFinalStatBySkill.m_iAtkMagic += (int) ((float)kRoomUserInfo.m_kGameStat.m_iAtkPhysic * pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_ATK_PHYSIC_TO_ATK_MAGIC ) );
			}
		}
	}
#endif SERV_RENA_NIGHT_WATCHER

	//{{ 2010. 11. 24	������	���� �нú� ��ų
#ifdef SERV_MAGIC_RESISTANCE_SKILL
	{
		// oasis907 : ����� [2010.12.3] // �� �����̾� - �������� ����
		const int iSkillLevel = m_kSkillTree.GetSkillLevel( (int) CXSLSkillTree::SI_P_ERS_MAGIC_RESISTANCE );
		if( iSkillLevel > 0 )
		{
			const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( (int)CXSLSkillTree::SI_P_ERS_MAGIC_RESISTANCE, iSkillLevel );
			if( NULL != pSkillTemplet )
			{
				kModifiedFinalStatBySkill.m_iDefMagic = (int) ((float)kRoomUserInfo.m_kGameStat.m_iDefMagic * pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_DEF_MAGIC_REL ) );
			}
		}
	}

	//{{ kimhc // 2010-12-14 // ��ȭ�� �г� - ���� �нú�
	{
		const int iSkillLevel = m_kSkillTree.GetSkillLevel( CXSLSkillTree::SI_P_RRF_SPIRITUALIZED_FURY );
		if( iSkillLevel > 0)
		{
			const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( CXSLSkillTree::SI_P_RRF_SPIRITUALIZED_FURY, iSkillLevel );
			if( pSkillTemplet != NULL )
			{
				float fIncreasingRate = CXSLSkillTree::CalulateIncreaseingRate( pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_ATK_MAGIC_REL ) );

				kModifiedFinalStatBySkill.m_iAtkMagic += static_cast<int>( kRoomUserInfo.m_kGameStat.m_iAtkMagic * fIncreasingRate );
			}
		}
	}
	//}} kimhc // 2010-12-14 // ��ȭ�� �г� - ���� �нú�
#endif SERV_MAGIC_RESISTANCE_SKILL
	//}}

#ifdef SERV_CHUNG_SECOND_CLASS_CHANGE
	{
		// oasis907 : ����� [2011.6.28] û 2�� ���� - ���̾� �ȶ�� - ������ ����
		const int iSkillLevel = m_kSkillTree.GetSkillLevel( (int) CXSLSkillTree::SI_P_CIP_IRON_WILL );
		if( iSkillLevel > 0 )
		{
			const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( (int)CXSLSkillTree::SI_P_CIP_IRON_WILL, iSkillLevel );
			if( NULL != pSkillTemplet )
			{
				float fIncreasingRate = CXSLSkillTree::CalulateIncreaseingRate( pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_ATK_PHYSIC_REL ) );

				kModifiedFinalStatBySkill.m_iAtkPhysic += static_cast<int>( kRoomUserInfo.m_kGameStat.m_iAtkPhysic * fIncreasingRate );
			}
		}
	}
#endif SERV_CHUNG_SECOND_CLASS_CHANGE

	//{{2010. 10. 27	����ȯ	���̺� �����̵� ������ ������ ���� - �¸��� �ڽŰ� ���� ���ݷ� ���� ȿ�� �߰�
#ifdef SERV_CONFIDENCE_VICTORY_PHYSIC_ATTACK
	{
		const int iSkillLevel = m_kSkillTree.GetSkillLevel( (int) CXSLSkillTree::SI_P_COMMON_CONFIDENCE_VICTORY );
		if( iSkillLevel > 0 )
		{
			const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( (int)CXSLSkillTree::SI_P_COMMON_CONFIDENCE_VICTORY, iSkillLevel );
			if( NULL != pSkillTemplet )
			{
				float fIncreasingRate = CXSLSkillTree::CalulateIncreaseingRate( pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_ATK_PHYSIC_REL ) );

				kModifiedFinalStatBySkill.m_iAtkPhysic += static_cast<int>( kRoomUserInfo.m_kGameStat.m_iAtkPhysic * fIncreasingRate );
			}
		}
	}
#endif SERV_CONFIDENCE_VICTORY_PHYSIC_ATTACK
	//}}


	kRoomUserInfo.m_kGameStat = kModifiedFinalStatBySkill;

	//{{ 2012. 02. 06	�ڼ���	�߷�Ÿ�� ���� �̺�Ʈ
#ifdef SERV_VALENTINE_DAY_EVENT
	if( (SiKGameSysVal()->GetMenBuffEvent() == true ) && ( eRoomType == CXSLRoom::RT_DUNGEON ) )
	{
		switch( GetUnitType() )
		{
		case CXSLUnit::UT_ELSWORD:
		case CXSLUnit::UT_RAVEN:
		case CXSLUnit::UT_CHUNG:
			kRoomUserInfo.m_kGameStat.m_iAtkPhysic += static_cast<int>( kRoomUserInfo.m_kGameStat.m_iAtkPhysic * 0.15f );
			kRoomUserInfo.m_kGameStat.m_iAtkMagic  += static_cast<int>( kRoomUserInfo.m_kGameStat.m_iAtkMagic  * 0.15f );
			break;
		}
	}
#endif SERV_VALENTINE_DAY_EVENT
	//}}

	m_kInventory.GetEquippedItem( kRoomUserInfo.m_mapEquippedItem );

	LIF( KUserSkillTree::MAX_SKILL_SLOT == 8 );     
	KSkillData aSkillData[ KUserSkillTree::MAX_SKILL_SLOT];
	m_kSkillTree.GetSkillSlot( aSkillData );

	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
	for( int iSkillSlotID = 0 ; iSkillSlotID < KUserSkillTree::MAX_SKILL_SLOT ; ++iSkillSlotID )
	{
		if( aSkillData[iSkillSlotID].m_iSkillID == CXSLSkillTree::SI_ETC_WS_COMMON_LOVE )
		{
			aSkillData[iSkillSlotID].m_cSkillLevel = m_kUserRelationshipManager.GetRelationshipType();
		}
	}
#endif SERV_RELATIONSHIP_SYSTEM
	//}

	kRoomUserInfo.m_UnitSkillData.m_aEquippedSkill[0] = aSkillData[KUserSkillTree::SKILL_SLOT_A1];
	kRoomUserInfo.m_UnitSkillData.m_aEquippedSkill[1] = aSkillData[KUserSkillTree::SKILL_SLOT_A2];
	kRoomUserInfo.m_UnitSkillData.m_aEquippedSkill[2] = aSkillData[KUserSkillTree::SKILL_SLOT_A3];
	kRoomUserInfo.m_UnitSkillData.m_aEquippedSkill[3] = aSkillData[KUserSkillTree::SKILL_SLOT_A4];

	kRoomUserInfo.m_UnitSkillData.m_aEquippedSkillSlotB[0] = aSkillData[KUserSkillTree::SKILL_SLOT_B1];
	kRoomUserInfo.m_UnitSkillData.m_aEquippedSkillSlotB[1] = aSkillData[KUserSkillTree::SKILL_SLOT_B2];
	kRoomUserInfo.m_UnitSkillData.m_aEquippedSkillSlotB[2] = aSkillData[KUserSkillTree::SKILL_SLOT_B3];
	kRoomUserInfo.m_UnitSkillData.m_aEquippedSkillSlotB[3] = aSkillData[KUserSkillTree::SKILL_SLOT_B4];

	kRoomUserInfo.m_UnitSkillData.m_cSkillSlotBExpirationState = (char) m_kSkillTree.GetSkillSlotBExpirationState();
	kRoomUserInfo.m_UnitSkillData.m_wstrSkillSlotBEndDate = m_kSkillTree.GetSkillSlotBEndDateString();


	m_kSkillTree.GetPassiveSkillData( kRoomUserInfo.m_UnitSkillData.m_vecPassiveSkill );

	//{{ 2010. 04. 08  ������	����� ��Ʈ
#ifdef SERV_SKILL_NOTE
	m_kSkillTree.GetSkillNote( kRoomUserInfo.m_UnitSkillData.m_vecSkillNote );
#endif SERV_SKILL_NOTE
	//}}

	//{{ 2009. 11. 27  ������	��彺ų
#ifdef GUILD_SKILL_TEST
	float fAddGuildBonusExpRate = 0.0f;
	float fAddSocketOptEDRate = 0.0f;

	m_kUserGuildManager.GetGuildPassiveSkillData( kRoomUserInfo.m_UnitSkillData.m_vecGuildPassiveSkill );

	m_kUserGuildManager.GetGuildSkillBonusRate( fAddGuildBonusExpRate, fAddSocketOptEDRate );

	//{{ 2010. 05. 11  ������	�̺�Ʈ ���� �ڵ� ����
	//#ifdef SERV_ROOMUSERINFO_REFAC
	if( fAddGuildBonusExpRate > 0.0f )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_GUILD_BONUS_EXP_RATE, fAddGuildBonusExpRate );
	} 
	if( fAddSocketOptEDRate > 0.0f )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_SOCKET_OPT_ED_RATE, fAddSocketOptEDRate );
	}
	//#endif SERV_ROOMUSERINFO_REFAC
	//}}
#endif GUILD_SKILL_TEST
	//}}

	//{{ 2013. 03. 14	 ������� �ʴ� RoomUserInfo �� ������ ���� - ��μ�
#ifdef SERV_DELETE_ROOM_USER_INFO_DATA
#else
	//{{ 2012. 03. 20	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	kRoomUserInfo.m_iSpiritMax	= m_kUserSpiritManager.GetSpiritMax();
	kRoomUserInfo.m_iSpirit		= m_kUserSpiritManager.GetSpirit();
#else
	kRoomUserInfo.m_iSpiritMax	= m_iSpiritMax;
	kRoomUserInfo.m_iSpirit		= m_iSpirit;
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}
#endif SERV_DELETE_ROOM_USER_INFO_DATA
	//}

	//{{ 2010. 01. 29  ������	PC�� ���ȭ
	kRoomUserInfo.m_bIsGameBang = IsPcBang();
	//}}    

	//2008. 03. 10. hoons : ���� �������� ����Ʈ�� ����Ʈ ������ ��������Ʈ ������ �����´�.
	//{{ 2010. 10. 26	������	����Ʈ ���� �߰�
#ifdef SERV_QUEST_CLEAR_EXPAND
	m_kUserQuestManager.GetOngoingQuestForRoom( GetThisPtr<KGSUser>(), kRoomUserInfo.m_mapOngoingQuest );
#else
	m_kUserQuestManager.GetDropQuestitembyIngQuest( kRoomUserInfo.m_mapDropQuestItembyIngQuest, GetThisPtr<KGSUser>() );
#endif SERV_QUEST_CLEAR_EXPAND
	//}}

	//{{ 2011. 05. 04  ��μ�	���� Ŭ����� ������ ���� ���� �߰�
#ifdef SERV_DUNGEON_CLEAR_PAYMENT_ITEM
	m_kUserQuestManager.GetUserAllQuest( kRoomUserInfo.m_setQuestInfo );
#endif SERV_DUNGEON_CLEAR_PAYMENT_ITEM
	//}}

	// ĪȣID
	//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
	//#ifdef SERV_TITLE_DATA_SIZE
	kRoomUserInfo.m_iTitleID = m_kUserTitleManager.GetEquippedTitleID();
	//#else
	//	kRoomUserInfo.m_sTitleID = m_kUserTitleManager.GetEquippedTitleID();
	//#endif SERV_TITLE_DATA_SIZE
	//}}

	//080421.hoons. �κ����� �ູ�� �޴� �������� �˻��Ѵ�.
	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_BLESSING_MEDAL ) == true )
	{
		//{{ 2010. 05. 11  ������	�̺�Ʈ ���� �ڵ� ����
		//#ifdef SERV_ROOMUSERINFO_REFAC
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, SiKGameSysVal()->GetPremiumEXPRate() );
		//#endif SERV_ROOMUSERINFO_REFAC
		//}}
	}

	// [�̺�Ʈ] 080619~26.hoons.���ѹα��� �� ����Ʈ �̺�Ʈ.
	//if( m_kInventory.IsExistOnlyInventory( 129765 ) == true )
	//	kRoomUserInfo.m_fAddExpRate += 0.15f;

	// [�̺�Ʈ] 080626~ 3��.hoons.��Ʈ3 ���� �̺�Ʈ �޴�
	//if( m_kInventory.IsExistOnlyInventory( 129770 ) == true )
	//	kRoomUserInfo.m_fAddExpRate += 0.15f;

	//080811.hoons. cpa(���־�ü ���� ������)
	//if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_EXP_15_PERCENT_MEDAL ) == true )
	//	kRoomUserInfo.m_fAddExpRate += 0.15f;

	// [�̺�Ʈ] 080828 ~ 080910 yuksa. ���б� ��� ����ġ �޴�
	//if( m_kInventory.IsExistOnlyInventory( 130028 ) == true )
	//	kRoomUserInfo.m_fAddExpRate += 0.15f;

	// [�̺�Ʈ] 081220 ~ 081221 yuksa. ��ĳ���� ��� ����ġ �޴�
	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_EXP_15_PERCENT_MEDAL ) == true )
	{
		//{{ 2010. 05. 11  ������	�̺�Ʈ ���� �ڵ� ����
		//#ifdef SERV_ROOMUSERINFO_REFAC
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 0.15f );
		//#endif SERV_ROOMUSERINFO_REFAC
		//}}
	}

	//{{ 2009. 3. 30  ������	����ġ 100% �޴�
	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_EXP_100_PERCENT_MEDAL ) == true )
	{
		//#ifdef SERV_ROOMUSERINFO_REFAC
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 1.0f );
		//#endif SERV_ROOMUSERINFO_REFAC
		//}}
	}
	//}}

	//{{ 2011. 10. 19    ��μ�    200% ����ġ �߰� �޴�(�̺�Ʈ ������)
#ifdef SERV_ADD_EXP_200_MEDAL
	if( m_kInventory.IsExistOnlyInventory( CXSLItem::SI_EXP_200_PERCENT_MEDAL ) == true )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_PREMIUM_EXP_RATE, 2.0f );
	}
#endif SERV_ADD_EXP_200_MEDAL
	//}} 

	//{{ 2009. 5. 5  ������		ä�κ� ���ʽ�
	float fAddChannelBonusExpRate = 0.0f;
	float fAddChannelBonusEDRate = 0.0f; // ���� ������
	SiKChannelManager()->GetChannelBonus( GetLevel(), fAddChannelBonusExpRate, fAddChannelBonusEDRate );
	//}}

	//{{ 2010. 05. 11  ������	�̺�Ʈ ���� �ڵ� ����
	//#ifdef SERV_ROOMUSERINFO_REFAC
	if( fAddChannelBonusExpRate > 0.0f )
	{
		kRoomUserInfo.AddBonusRate( KRoomUserInfo::BT_CHANNEL_BONUS_EXP_RATE, fAddChannelBonusExpRate );
	}
	//#endif SERV_ROOMUSERINFO_REFAC
	//}}

	//{{ 2010. 07. 21  ������	�� �ý���
#ifdef SERV_PET_SYSTEM
	m_kUserPetManager.GetSummonedPetInfo( kRoomUserInfo.m_vecPet );
#endif SERV_PET_SYSTEM
	//}}

#ifdef	SERV_RIDING_PET_SYSTM// ���볯¥: 2013-04-21
	m_kUserRidingPetManager.GetSummonedRidingPetInfo( kRoomUserInfo.m_iRidingPetUID, kRoomUserInfo.m_usRidingPetID );
#endif	// SERV_RIDING_PET_SYSTM

	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
	kRoomUserInfo.m_iLoverUnitUID = m_kUserRelationshipManager.GetLoverUnitUID();
#endif SERV_RELATIONSHIP_SYSTEM
	//}

}
*/
#endif	// SERV_UPGRADE_SKILL_SYSTEM_2013

void KGSUser::GetTutorUnitUIDList(std::vector<UidType>& vecStudentUnitUID)
{
	if( m_kUserTutorialInfo.GetTutorUnitType() == KUserTutorialInfo::TUT_TEACHER &&
		m_kUserTutorialInfo.IsExistbyTutorData() == true )
	{
		m_kUserTutorialInfo.GetTutorUnitUIDList( vecStudentUnitUID );
	}
}

void KGSUser::GetSquareUserInfo( KSquareUserInfo& kSquareUserInfo )
{
    kSquareUserInfo.m_iGSUID            = KBaseServer::GetKObj()->GetUID();
    //kSquareUserInfo.m_bMale             = m_kNexonAccountInfo.m_bSex;
    //kSquareUserInfo.m_ucAge             = m_kNexonAccountInfo.m_uiAge;
    kSquareUserInfo.m_iUnitUID          = GetCharUID();
    kSquareUserInfo.m_wstrNickName      = GetCharName();

    kSquareUserInfo.m_wstrIP            = GetP2PIP();
    kSquareUserInfo.m_usPort            = GetP2PPort();

    kSquareUserInfo.m_cUnitClass        = GetUnitClass();
    kSquareUserInfo.m_ucLevel           = GetLevel();	
	//{{ 2011. 06. 20	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
	//{{ 2012. 09. 11	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2
	kSquareUserInfo.m_cRank				= GetPvpRankForClient();
#else
	kSquareUserInfo.m_iRating			= GetRating();
#endif SERV_2012_PVP_SEASON2
	//}}
#else
	kSquareUserInfo.m_cPVPEmblem		= static_cast<char>( CXSLUnit::PvpEmblemToPvpEmblemEnum( CXSLUnit::GetPVPEmblem( m_iVSPointMax ) ) );
#endif SERV_PVP_NEW_SYSTEM
	//}}	

    kSquareUserInfo.m_bIsParty          = ( GetPartyUID() > 0 );

	// ���� ���
	m_kInventory.GetEquippedItem( kSquareUserInfo.m_vecEquippedItem );

	// ���λ���
	kSquareUserInfo.m_cPersonalShopState = CXSLSquareUnit::PSS_NONE;
	kSquareUserInfo.m_cPersonalShopType = CXSLSquareUnit::PST_NONE;

	// Ÿ��Ʋ
	//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
#ifdef SERV_TITLE_DATA_SIZE
	kSquareUserInfo.m_iTitleID = m_kUserTitleManager.GetEquippedTitleID();
#else
	kSquareUserInfo.m_sTitleID = m_kUserTitleManager.GetEquippedTitleID();
#endif SERV_TITLE_DATA_SIZE
	//}}	

	//{{ 2009. 9. 25  ������	���
#ifdef GUILD_TEST
	kSquareUserInfo.m_wstrGuildName = m_kUserGuildManager.GetGuildName();
#endif GUILD_TEST
	//}}

	//{{ 2011. 09. 20  ��μ�	���ӳ� ED ���� ����͸�
#ifdef SERV_ED_MONITORING_IN_GAME
	kSquareUserInfo.m_iED = GetED();
#endif SERV_ED_MONITORING_IN_GAME
	//}}

#ifdef SERV_GROW_UP_SOCKET
	kSquareUserInfo.m_iExchangeCount = m_iExchangeCount;
#endif SERV_GROW_UP_SOCKET
}

//{{ 2008. 11. 4  ������	���尳��
void KGSUser::GetFieldUserInfo( KFieldUserInfo& kFieldUserInfo )
{
	kFieldUserInfo.m_iUnitUID			= GetCharUID();
	kFieldUserInfo.m_wstrNickName		= GetCharName();
	kFieldUserInfo.m_cUnitClass			= GetUnitClass();
	kFieldUserInfo.m_ucLevel			= GetLevel();
	//{{ 2011. 06. 20	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
	//{{ 2012. 09. 11	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2
	kFieldUserInfo.m_cRank				= GetPvpRankForClient();
#else
	kFieldUserInfo.m_iRating			= GetRating();
#endif SERV_2012_PVP_SEASON2
	//}}
#else
	kFieldUserInfo.m_cPVPEmblem			= static_cast<char>( CXSLUnit::PvpEmblemToPvpEmblemEnum( m_ePVPEmblem ) );
#endif SERV_PVP_NEW_SYSTEM
	//}}	
	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
	kFieldUserInfo.m_iLoverUnitUID = m_kUserRelationshipManager.GetLoverUnitUID();
#endif SERV_RELATIONSHIP_SYSTEM
	//}
	// ���� ���
	m_kInventory.GetEquippedItem( kFieldUserInfo.m_vecEquippedItem );

	//////////////////////////////////////////////////////////////////////////
	// �κ��� ��� ������ ���� ��������
#ifdef SERV_ROBOT_TEST
	SiKRobotTestManager()->RobotFullEquip( GetCharName(), GetUnitType(), kFieldUserInfo.m_vecEquippedItem );
#endif SERV_ROBOT_TEST
	//////////////////////////////////////////////////////////////////////////

	//{{ 2008. 10. 7  ������	Ÿ��Ʋ
	//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
#ifdef SERV_TITLE_DATA_SIZE
	kFieldUserInfo.m_iTitleID = m_kUserTitleManager.GetEquippedTitleID();
#else
	kFieldUserInfo.m_sTitleID = m_kUserTitleManager.GetEquippedTitleID();
#endif SERV_TITLE_DATA_SIZE
	//}}	
	//}}

	//{{ 2009. 9. 25  ������	���
#ifdef GUILD_TEST
	kFieldUserInfo.m_wstrGuildName = m_kUserGuildManager.GetGuildName();
#endif GUILD_TEST
	//}}

	//{{ 2009. 2. 24  ������	��Ƽ ��ǳ��
	kFieldUserInfo.m_iPartyUID = GetPartyUID();

	//{{ 2011. 09. 20  ��μ�	���ӳ� ED ���� ����͸�
#ifdef SERV_ED_MONITORING_IN_GAME
	kFieldUserInfo.m_iED = GetED();
#endif SERV_ED_MONITORING_IN_GAME
	//}}

	//{{ 2010. 07. 21  ������	�� �ý���
#ifdef SERV_PET_SYSTEM
	m_kUserPetManager.GetSummonedPetInfo( kFieldUserInfo.m_vecPet );
#endif SERV_PET_SYSTEM
	//}}
#ifdef SERV_INVISIBLE_GM
	kFieldUserInfo.m_bInvisible = IsInvisible();
#endif SERV_INVISIBLE_GM

#ifdef	SERV_RIDING_PET_SYSTM// ���볯¥: 2013-04-21
	m_kUserRidingPetManager.GetSummonedRidingPetInfo( kFieldUserInfo.m_iRidingPetUID, kFieldUserInfo.m_usRidingPetID );
#endif	// SERV_RIDING_PET_SYSTM

#ifdef SERV_GROW_UP_SOCKET
	kFieldUserInfo.m_iExchangeCount = m_iExchangeCount;
#endif SERV_GROW_UP_SOCKET
}
//}}

//{{ 2009. 9. 22  ������	���
#ifdef GUILD_TEST
void KGSUser::GetGuildMemberInfo( KGuildMemberInfo& kInfo )
{
	kInfo.m_iGSUID		 = KBaseServer::GetKObj()->GetUID();
	kInfo.m_iUnitUID	 = GetCharUID();
	kInfo.m_wstrNickName = GetCharName();
	kInfo.m_cUnitClass	 = GetUnitClass();
	kInfo.m_ucLevel		 = GetLevel();
	kInfo.m_ucMemberShipGrade = m_kUserGuildManager.GetMemberShipGrade();
	kInfo.m_iChannelID	 = SiKChannelManager()->GetChannelID();
	kInfo.m_iMapID		 = GetMapID();
	kInfo.m_tLogOutTime	 = 0; // ���� ���������Ƿ� 0���� �ʱ�ȭ!
	//kInfo.m_wstrMessage	 = L"";

	// RoomListID�� �����ϸ� PVP�κ��̴�.
	if( GetRoomListID() > 0 )
	{
		kInfo.m_cPosition = CXSLRoom::RT_PVP;
	}
	else
	{
		if( GetRoomUID() > 0 )
		{
			kInfo.m_cPosition = CXSLRoom::GetRoomType( GetRoomUID() );
		}
		else
		{
			kInfo.m_cPosition = -1; // ���� �ƴϸ� -1
		}
	}
}
#endif GUILD_TEST
//}}

bool KGSUser::CheckRequireDungeonID( int iRequireDungeonID )
{
	std::map< int, KDungeonClearInfo >::const_iterator mit;
	mit = m_mapDungeonClear.find( iRequireDungeonID );
	if( mit == m_mapDungeonClear.end() )
		return false;
	
	return true;
}

bool KGSUser::IsExistInventoryItem( int iItemID )
{
	return m_kInventory.IsExistOnlyInventory( iItemID );
}

bool KGSUser::IsExistInventoryItemWithBank( int iItemID )
{
	return m_kInventory.IsExistWithBank( iItemID );
}

//{{ 2010. 10. 26	������	����Ʈ ���� �߰�
#ifdef SERV_QUEST_CLEAR_EXPAND
bool KGSUser::IsEnoughSpaceExist( IN const std::map< int, int >& mapItem )
{
	return m_kInventory.IsEnoughSpaceExist( mapItem );
}
#endif SERV_QUEST_CLEAR_EXPAND
//}}

bool KGSUser::IsCompleteQuest( int iQuestID )
{
    return m_kUserQuestManager.IsCompleteQuest( iQuestID );
}

//{{ 2012. 09. 21   ��μ�   ���� ���� ���� �޽��� ����
#ifdef SERV_ENTER_VILLAGE_ERROR
//{{ 2009. 3. 9  ������		���� ���� ����
bool KGSUser::CheckEnterTheVillage( IN int iMapID, OUT int& iLimitLevel, OUT int& iRequireDungeonID )
{
	//{{ 2009. 4. 21  ������	��ڴ� �׳� ���
	if( GetAuthLevel() >= SEnum::UAL_GM )
		return true;
	//}}

	return SiCXSLMapData()->CheckRequireData( iMapID, GetLevel(), m_mapDungeonClear, iLimitLevel, iRequireDungeonID );
}
#endif SERV_ENTER_VILLAGE_ERROR
//}}

bool KGSUser::CheckEnterTheVillage( IN const int iMapID )
{
	//{{ 2009. 4. 21  ������	��ڴ� �׳� ���
	if( GetAuthLevel() >= SEnum::UAL_GM )
		return true;
	//}}

	int iDummy1 = 0;
	int iDummy2 = 0;
	return SiCXSLMapData()->CheckRequireData( iMapID, GetLevel(), m_mapDungeonClear, iDummy1, iDummy2 );
}

//{{ 2009. 4. 28  ������	�ִ�ED �˻�
bool KGSUser::CheckMaxED( __int64 iAddED )
{
	iAddED += GetED();

	return ( iAddED <= static_cast<__int64>(SEnum::UI_MAX_ED_CHAR_HAVE) );
}
//}}

bool KGSUser::IsClearDungeon(int iDungeonID, char cDiffLevel)
{
	std::map< int, KDungeonClearInfo >::iterator mit;
	mit = m_mapDungeonClear.find(static_cast<int>(iDungeonID+cDiffLevel));

	if( mit == m_mapDungeonClear.end() )
		return false;

	return true;
}

//{{ 2009. 6. 12  ������	������ ����
bool KGSUser::CheckValidNickName( const std::wstring& wstrNickName )
{
	if( wstrNickName.empty()  ||  wstrNickName.size() > 12 )
	{
		// ���������� ũ���� NickName
		SET_ERROR( ERR_CREATE_UNIT_04 );
		return false;
	}

	if( KODBC::IsInvalidMarkIn( wstrNickName ) )
	{
		// ������ �ʴ� Ư�����ڰ� ���ԵǾ�����
		SET_ERROR( ERR_ODBC_00 );
		return false;
	}

	return true;
}
//}}

//{{ 2009. 11. 10  ������	PC�� ä���̵�
void KGSUser::GetChannelChangeInfo( KChannelChangeInfo& kInfo )
{
	// ä���̵� ����� �Բ� ������ ������ ����ϴ�.

	//{{ 2009. 10. 26  ������	���ӽð��̺�Ʈ
	m_kUserEventManager.GetEventTimerInfo( kInfo.m_mapEventDurationTime );
	//}}
	//{{ 2009. 11. 10  ������	Īȣ
	//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
#ifdef SERV_TITLE_DATA_SIZE
	kInfo.m_iEquippedTitleID = m_kUserTitleManager.GetEquippedTitleID();
#else
	kInfo.m_sEquippedTitleID = m_kUserTitleManager.GetEquippedTitleID();
#endif SERV_TITLE_DATA_SIZE
	//}}	
	//}}
	kInfo.m_bIsPcBang = IsPcBang();
#ifdef SERV_PC_BANG_TYPE
	kInfo.m_iPcBangType = GetPcBangType();
#endif SERV_PC_BANG_TYPE

	//{{ 2010. 02. 08  ������	PC�� ä�� �̵� ���� ����
	m_kInventory.GetChannelChangeInventoryInfo( kInfo.m_vecPcBangItemInfo );
	//}}

	//{{ 2012. 06. 14	�ڼ���	���ϸ��� ���� ����
#ifdef SERV_EVENT_DAILY_GIFT_BOX
	kInfo.m_mmapDailyGiftBoxList = m_mmapDailyGiftBoxList;
#endif SERV_EVENT_DAILY_GIFT_BOX
	//}}
	//{{ 2012. 12. 12	�ڼ���	�ܿ� ���� ���� �̺�Ʈ( �ӽ�, �ϵ� �ڵ� )
#ifdef SERV_2012_WINTER_VACATION_EVENT
	kInfo.m_wstrWinterVacationEventRegDate	= m_wstrWinterVacationEventRegDate;
	kInfo.m_iWinterVacationEventCount		= m_iWinterVacationEventCount;
#endif SERV_2012_WINTER_VACATION_EVENT
	//}}
	//{{ 2012. 1. 2		Merge �ڼ���	( 2013.1.2 ���� : �̴ϸ� ä�� �̵� �� �������� ���� �߰� )
#ifdef SERV_CHANNEL_CHANGE_BUG_FIX
	kInfo.m_nUnitSlot = m_nUnitSlot;
#endif SERV_CHANNEL_CHANGE_BUG_FIX
	//}}

#ifdef	SERV_RIDING_PET_SYSTM// ���볯¥: 2013-04-30
	USHORT usRidingPetID;
	m_kUserRidingPetManager.GetSummonedRidingPetInfo( kInfo.m_iSummonedRidingPetUID, usRidingPetID );
#endif	// SERV_RIDING_PET_SYSTM
}
//}}

//{{ 2009. 1. 20  ������	����ġ ����
void KGSUser::KUserExpManager::AddExp( int iExp )
{
	_JIF( iExp >= 0, return; );

	const int iTotalExpByFinalLevel = SiKExpTable()->GetRequireTotalExpbyLevel( SiKGameSysVal()->GetLimitsLevel() );

	if( ( m_iExp + iExp ) > iTotalExpByFinalLevel )
	{
		iExp = iTotalExpByFinalLevel - m_iExp;
	}
	
	if( iExp <= 0 )
		return;

	m_iExp += iExp;
}
//}}

void KGSUser::InitTransactionEventFilter()
{
	m_kTransactionGuarantor.RegisterMutualExclusiveEvent( DBE_EXPIRE_CASH_SKILL_POINT_REQ, DBE_EXPIRE_CASH_SKILL_POINT_ACK, EGS_GET_SKILL_REQ, EGS_GET_SKILL_ACK );
	m_kTransactionGuarantor.RegisterMutualExclusiveEvent( DBE_EXPIRE_CASH_SKILL_POINT_REQ, DBE_EXPIRE_CASH_SKILL_POINT_ACK, EGS_RESET_SKILL_REQ, EGS_RESET_SKILL_ACK );
	m_kTransactionGuarantor.RegisterMutualExclusiveEvent( DBE_EXPIRE_CASH_SKILL_POINT_REQ, DBE_EXPIRE_CASH_SKILL_POINT_ACK, EGS_INIT_SKILL_TREE_REQ, EGS_INIT_SKILL_TREE_ACK );
}

bool KGSUser::IsTransactionEnabled( unsigned short usEventID )
{
	bool bTransactionEnabled = true;
	unsigned short usBlockingEventID = 0;

	switch( usEventID )
	{
	case DBE_EXPIRE_CASH_SKILL_POINT_REQ:
	case EGS_GET_SKILL_REQ:
	case EGS_RESET_SKILL_REQ:
	case EGS_INIT_SKILL_TREE_REQ:
		{
			bTransactionEnabled = m_kTransactionGuarantor.IsTransactionEnabled( usEventID, usBlockingEventID );

			if( false == bTransactionEnabled )
			{
				START_LOG( cwarn, L"��Ŷ ó���ҷ��� �ϴµ� �ٸ� ��Ŷ ó���� ���ļ� Ÿ�ֹ̹����� ���ϱ� ���� ��Ŷ ó�� ���ϰ� ����!" )
					<< BUILD_LOG( GetCharUID() )
#ifndef SERV_PRIVACY_AGREEMENT
					<< BUILD_LOG( GetCharName() )
#endif SERV_PRIVACY_AGREEMENT
					<< BUILD_LOG( usEventID )
					<< BUILD_LOG( usBlockingEventID )
					<< END_LOG;
			}

		} break;
	}

	return bTransactionEnabled;
}

void KGSUser::CheckTransactionReqEvent( unsigned short usEventID )
{
	switch( usEventID )
	{
	case DBE_EXPIRE_CASH_SKILL_POINT_REQ:
	case EGS_GET_SKILL_REQ:
	case EGS_RESET_SKILL_REQ:
	case EGS_INIT_SKILL_TREE_REQ:
		{
			m_kTransactionGuarantor.CheckReqEvent( usEventID );
		} break;
	}
}

void KGSUser::CheckTransactionAckEvent( unsigned short usEventID )
{
	switch( usEventID )
	{
	case DBE_EXPIRE_CASH_SKILL_POINT_ACK:
	case EGS_GET_SKILL_ACK:
	case EGS_RESET_SKILL_ACK:
	case EGS_INIT_SKILL_TREE_ACK:
		{
			m_kTransactionGuarantor.CheckAckEvent( usEventID );
		} break;
	}
}

//////////////////////////////////////////////////////////////////////////
#ifdef SERV_GLOBAL_BILLING

void KGSUser::OnBuyCashItemAck( const KEGS_BUY_CASH_ITEM_ACK& kPacket_ )
{
	// ��� ���� ������ ���� ������
	KEGS_BUY_CASH_ITEM_ACK kPacket = kPacket_;
	kPacket.m_bIsWishList = m_kUserWishList.GetBuyCashItemInWishList();
	SendPacket( EGS_BUY_CASH_ITEM_ACK, kPacket );

	// �ǽð� ��Ȱ��
	if( m_kUserCashInventory.IsCashResurrectionStone() == true  &&  kPacket_.m_iOK != NetError::NET_OK )
	{
		KEGS_RESURRECT_TO_CONTINUE_DUNGEON_ACK kPacketAck;
		kPacketAck.m_iOK = kPacket_.m_iOK;		
		SendPacket( EGS_RESURRECT_TO_CONTINUE_DUNGEON_ACK, kPacketAck );
	}

	//{{ 2011. 02. 08	������	ĳ���� ���� �α�
#ifdef SERV_BUY_CASH_ITEM_LOG
	BOOST_TEST_FOREACH( const KNXBTPurchaseReqInfo&, kReq, kPacket_.m_vecPurchaseReqInfo )
	{
		KNXBTProductInfo kProductInfo;
		if( SiKNexonBillingTCPManager()->GetProductInfo( kReq.m_ulProductNo, kProductInfo ) == false )
		{
			START_LOG( cerr, L"�������� �ʴ� ��ǰ�� �����Ͽ����ϴ�!" )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOG( kReq.m_ulProductNo )
				<< END_LOG;
			continue;
		}

		// ĳ���� ���� ī��Ʈ üũ
		m_kUserStatistics.IncreaseCharacterCount( KUserStatistics::CGCT_BUY_CASH_ITEM );
		m_kUserStatistics.IncreaseAccountCount( KUserStatistics::AGCT_BUY_CASH_ITEM );

		// �ش� ĳ���Ͱ� ���ʷ� ĳ���� �����Ѱ��̶��?
		if( m_kUserStatistics.GetCharacterCount( KUserStatistics::CGCT_BUY_CASH_ITEM ) == 1 )
		{
			//{{ 2011. 01. 17	������	ĳ���� ī��Ʈ ����
#ifdef SERV_CHAR_LOG
			WriteCharacterLogToDBWithProductNo( KELOG_USER_CHARACTER_LOG_NOT::LT_FIRST_BUY_CASH_ITEM, kReq.m_ulProductNo );
#endif SERV_CHAR_LOG
			//}}
		}

		// ĳ���� ���� �α� �����
		CTime tCurTime = CTime::GetCurrentTime();
		KELOG_BUY_CASH_ITEM_LOG_NOT kPacketToLog;
		kPacketToLog.m_iUserUID			= GetUID();
		kPacketToLog.m_iServerGroupID	= KBaseServer::GetKObj()->GetServerGroupID();
		kPacketToLog.m_iUnitUID			= GetCharUID();
		kPacketToLog.m_cUnitClass		= GetUnitClass();
		kPacketToLog.m_ucLevel			= GetLevel();
		kPacketToLog.m_bIsFirstBuy		= m_kUserStatistics.IsFirstBuy();
		kPacketToLog.m_cBuyType			= KELOG_BUY_CASH_ITEM_LOG_NOT::CIBT_BUY;
		kPacketToLog.m_iProductNo		= kProductInfo.m_ulProductNo;
		kPacketToLog.m_iBuyQuantity		= kReq.m_usOrderQuantity;
		kPacketToLog.m_iTotalSalePrice	= kReq.m_usOrderQuantity * kProductInfo.m_ulSalePrice;
		kPacketToLog.m_wstrRegDate		= ( std::wstring )( tCurTime.Format( _T( "%Y-%m-%d %H:%M:%S" ) ) );
		SendToLogDB( ELOG_BUY_CASH_ITEM_LOG_NOT, kPacketToLog );
	}
#endif SERV_BUY_CASH_ITEM_LOG
	//}}
}


int KGSUser::OnGetPurchasedCashItemReq( const KEGS_GET_PURCHASED_CASH_ITEM_REQ& kPacket_, const KEGS_BILL_GET_PURCHASED_CASH_ITEM_REQ& kPacket2_ )
{
	// 1. ��ǰ ������ID ���
	int iItemID = 0;
	KBillProductInfo kBillProductInfo;

	if( !SiKBillManager()->GetProductInfo( kPacket2_.m_iProductNo, kBillProductInfo ) )
	{
		iItemID = 0;
	}
	else
	{
		iItemID = kBillProductInfo.m_iProductID;
	}

	if( iItemID <= 0 )
	{
		START_LOG( cerr, L"������ ID ��ȯ ����." )
			<< BUILD_LOG( kPacket_.m_ulProductNo )
			<< BUILD_LOG( kPacket_.m_wstrProductID )
			<< BUILD_LOG( kPacket2_.m_iTransNo )
			<< BUILD_LOG( kPacket2_.m_iProductNo )
			<< BUILD_LOG( iItemID )
			<< END_LOG;

		return NetError::ERR_NX_SHOP_01;
	}

	std::map< int, int > mapItem; // �κ��丮 �˻��

	// 2. ��ǰ ������ �˻��Ѵ�.
	if( SiKBillManager()->IsPackage( kPacket2_.m_iProductNo ) )
	{
		START_LOG( cerr, L"��Ű�� ������ �������� �õ�" )
			<< BUILD_LOG( GetUID() )
			<< BUILD_LOG( kPacket2_.m_iProductNo )
			<< END_LOG;

		return NetError::ERR_NX_SHOP_01;
	}

	// 3. ���� ������ Ŭ�������� �˻��Ѵ�.
	if( !m_kInventory.CompareUnitClass( iItemID ) )
	{
		return NetError::ERR_NX_SHOP_07;
	}

	// 3-1. ���� ���� ������ �˻��Ѵ�.
	if( kBillProductInfo.m_cLimitLevel > GetLevel() )
	{
		return NetError::ERR_BUY_CASH_ITEM_59;
	}

	// 4. ���������� �������� �κ��丮 �Ǵ� ���࿡ �����鼭 �Ⱓ���̸� ���´�.
	if( m_kInventory.IsExistWithBank( iItemID ) )
	{
		if( kBillProductInfo.m_cPeriod > 0 )
		{
			return NetError::ERR_NX_SHOP_08;
		}
	}

	//{{ 2012. 02. 22	�ڼ���	��� �̸� �����
#ifdef SERV_GUILD_CHANGE_NAME
	switch( iItemID )
	{
	case CXSLItem::CI_GUILD_NAME_CHANGE:
		if( m_kUserGuildManager.IsGuildMaster() == true )
		{
			// ��� �̸� ������� ���� ó��
			KEGS_BILL_GET_PURCHASED_CASH_ITEM_ACK kPacket;
			kPacket.m_iOK = NetError::NET_OK;
			kPacket.m_iExceptionProcessItemID = iItemID;
			SendPacket( EGS_BILL_GET_PURCHASED_CASH_ITEM_ACK, kPacket );

			return NetError::NET_OK;
		}
		else
		{
			return NetError::ERR_BUY_CASH_ITEM_67;
		}
		break;
	}
#endif SERV_GUILD_CHANGE_NAME
		//}}

	// ĳ���� �κ������� ���������� �������� �ʴ� ĳ�þ������� ��쿡 �ߺ����ſ� ���� ���� ó��
	switch( iItemID )
	{
	case CXSLItem::CI_EXPAND_SKILL_SLOT: // 4.1 ���������� �������� ��ų����B(�Ⱓ��) �̸�, �̹� ��ų����B(�Ⱓ��, Ȥ�� ����)�� ������̸� ������ �� ���� ���´�
		{
			KUserSkillTree::SKILL_SLOT_B_EXPIRATION_STATE eSkillSlotBExpireState = m_kSkillTree.GetSkillSlotBExpirationState();
			if( KUserSkillTree::SSBES_EXPIRED != eSkillSlotBExpireState )
			{
				return NetError::ERR_BUY_CASH_ITEM_32;
			}
		} break;


		// 4.2 ĳ�ý�ų����Ʈ�� �̹� ����ϰ� �ִµ� ����Ʈ�� �ٸ� ĳ�ý�ų����Ʈ�� �����ͼ� �Ⱓ ������ �Ϸ��� �ϸ� ���´�
#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013// �۾���¥: 2013-06-25	// �ڼ���
		case CXSLItem::CI_CASH_SKILL_POINT_60_15:
	case CXSLItem::CI_CASH_SKILL_POINT_60_30:
	case CXSLItem::CI_CASH_SKILL_POINT_30_15:
	case CXSLItem::CI_CASH_SKILL_POINT_30_30:
#ifdef SERV_CASH_SKILL_POINT_TW
	case CXSLItem::CI_CASH_SKILL_POINT_30_7:
	case CXSLItem::CI_CASH_SKILL_POINT_60_7:
#endif SERV_CASH_SKILL_POINT_TW
#else
/*
	case CXSLItem::CI_CASH_SKILL_POINT_5:
	case CXSLItem::CI_CASH_SKILL_POINT_10:
*/
#endif // SERV_UPGRADE_SKILL_SYSTEM_2013
		{
			if( false == m_kSkillTree.IsCashSkillPointExpired() )
			{
				const int iCSPoint = SiCXSLItemManager()->GetItemCSPoint( iItemID );

				if( iCSPoint <= 0 )
				{
					return NetError::ERR_RESET_SKILL_01;
				}

				if( iCSPoint != m_kSkillTree.GetMaxCSPoint() )
				{
					return NetError::ERR_BUY_CASH_ITEM_32;				// fix!! �����޼��� ����
				}
			}
		} break;
	}

	// �κ��丮 ���� ���� �˻�
	switch( iItemID )
	{
	case CXSLItem::CI_EXPAND_SKILL_SLOT: 
#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013// �۾���¥: 2013-06-25	// �ڼ���
	case CXSLItem::CI_CASH_SKILL_POINT_60_15:
	case CXSLItem::CI_CASH_SKILL_POINT_60_30:
	case CXSLItem::CI_CASH_SKILL_POINT_30_15:
	case CXSLItem::CI_CASH_SKILL_POINT_30_30:
#ifdef SERV_CASH_SKILL_POINT_TW
	case CXSLItem::CI_CASH_SKILL_POINT_30_7:
	case CXSLItem::CI_CASH_SKILL_POINT_60_7:
#endif SERV_CASH_SKILL_POINT_TW
#else
/*
	case CXSLItem::CI_CASH_SKILL_POINT_5:
	case CXSLItem::CI_CASH_SKILL_POINT_10:
*/
#endif // SERV_UPGRADE_SKILL_SYSTEM_2013
#ifdef SERV_ADD_WARP_BUTTON
	case CXSLItem::CI_WARP_VIP_ITEM:
#ifdef SERV_VIP_SYSTEM
	case CXSLItem::SI_USE_FREE_BY_FIELD_VIP:
#endif SERV_VIP_SYSTEM
#endif // SERV_ADD_WARP_BUTTON
#ifdef SERV_GUILD_CHANGE_NAME
	case CXSLItem::CI_GUILD_NAME_CHANGE:
#endif //SERV_GUILD_CHANGE_NAME
		{
			// ĳ���κ����� �� ĳ���ͷ� ������ �� �� �κ��� ���������� ���� �ʴ� �͵��� �κ��丮 �������� �˻縦 ���� �ʴ´�.

		} break;

	default:
		{
			// 5. �������� �˻縦 ���� �غ�
			int iQuantity = ( int )kBillProductInfo.m_cQuantity;           
			mapItem.insert( std::make_pair( iItemID, iQuantity ) );	

			// 6. �κ��丮�� �������� �˻�
			if( !m_kInventory.IsEnoughSpaceExist( mapItem ) )
			{
				return NetError::ERR_NX_SHOP_02;
			}
		}
		break;
	}

	// 7. �����ۿ� �ο��� ���Ͽɼ�
	//{{ 2012. 11. 28 ĳ�� ������ ���� �ɼ� ��� ���� - ��μ�
#ifdef SERV_CASH_ITEM_SOCKET_OPTION
	if( m_kUserCashInventory.SetSocketForCash( kPacket2_.m_mapSocketForCash, kPacket2_.m_mapSocketGroupIDForCash ) == false )
	{
		// ���������� �ʴ� ���� ������ �����Ϸ� �մϴ�.
		return NetError::ERR_SOCKET_ITEM_09;
	}
#else
	m_kUserCashInventory.SetSocketForCash( kPacket2_.m_mapSocketForCash );
#endif SERV_CASH_ITEM_SOCKET_OPTION
	//}

	KEBILL_PICK_UP_REQ kPacket;
	kPacket.m_iTransNo = kPacket2_.m_iTransNo;
	kPacket.m_iProductNo = kPacket2_.m_iProductNo;
	kPacket.m_iUnitUID = GetCharUID();
	kPacket.m_mapSocketForCash = kPacket2_.m_mapSocketForCash;
#ifdef SERV_GLOBAL_CASH_PACKAGE
	kPacket.m_byteProductKind = kPacket2_.m_byteProductKind;
#endif //SERV_GLOBAL_CASH_PACKAGE
	SendToKOGBillingDB( EBILL_PICK_UP_REQ, kPacket );

	return NetError::NET_OK;
}

bool KGSUser::IsDirectPickUpItem( IN const int iItemID )
{
	switch( iItemID )
	{
	case CXSLItem::CI_RESURRECTION_STONE:
	case CXSLItem::CI_EXPAND_INVENTORY:
	case CXSLItem::CI_EXPAND_INVENTORY_EQUIP:
	case CXSLItem::CI_EXPAND_INVENTORY_ACCESSORY:
	case CXSLItem::CI_EXPAND_INVENTORY_QUICK_SLOT:
	case CXSLItem::CI_EXPAND_INVENTORY_MATERIAL:
	case CXSLItem::CI_EXPAND_INVENTORY_QUEST:
	case CXSLItem::CI_EXPAND_INVENTORY_SPECIAL:
	case CXSLItem::CI_EXPAND_CHAR_SLOT_1:
	case CXSLItem::CI_EXPAND_CHAR_SLOT_2:
	case CXSLItem::CI_EXPAND_SKILL_SLOT_PERMANENT:
		//{{ 2009. 8. 7  ������		����
	case CXSLItem::CI_BANK_MEMBERSHIP_SILVER:
	case CXSLItem::CI_BANK_MEMBERSHIP_GOLD:
	case CXSLItem::CI_BANK_MEMBERSHIP_EMERALD:
	case CXSLItem::CI_BANK_MEMBERSHIP_DIAMOND:
	case CXSLItem::CI_BANK_MEMBERSHIP_PLATINUM:
	case CXSLItem::CI_BANK_MEMBERSHIP_ONE_PACKAGE:
		//}}
		//{{ 2009. 9. 22  ������	����ĳ��
	case CXSLItem::CI_CHANGE_JOB_ELSWORD_KNIGHT:
	case CXSLItem::CI_CHANGE_JOB_MAGIC_KNIGHT:
	case CXSLItem::CI_CHANGE_JOB_COMBAT_RANGER:
	case CXSLItem::CI_CHANGE_JOB_SNIPING_RANGER:
	case CXSLItem::CI_CHANGE_JOB_HIGH_MAGICIAN:
	case CXSLItem::CI_CHANGE_JOB_DARK_MAGICIAN:
	case CXSLItem::CI_CHANGE_JOB_SOUL_TAKER:
	case CXSLItem::CI_CHANGE_JOB_OVER_TAKER:
	case CXSLItem::CI_CHANGE_JOB_EXOTIC_GEAR:
	case CXSLItem::CI_CHANGE_JOB_ARCHITECTURE:

	case CXSLItem::CI_CHANGE_JOB_LORD_KNIGHT:
	case CXSLItem::CI_CHANGE_JOB_RUNE_SLAYER:
	case CXSLItem::CI_CHANGE_JOB_WIND_SNEAKER:
	case CXSLItem::CI_CHANGE_JOB_GRAND_ARCHER:
	case CXSLItem::CI_CHANGE_JOB_ELMENTAL_MASTER:
	case CXSLItem::CI_CHANGE_JOB_VOID_PRINCESS:
	case CXSLItem::CI_CHANGE_JOB_BLADE_MASTER:
	case CXSLItem::CI_CHANGE_JOB_RECKLESS_FIST:
	case CXSLItem::CI_CHANGE_JOB_NEMESIS:
	case CXSLItem::CI_CHANGE_JOB_EMPRESS:
	case CXSLItem::CI_CHANGE_JOB_FURY_GUARDIAN:
	case CXSLItem::CI_CHANGE_JOB_SHOOTER_GUARDIAN:
	case CXSLItem::CI_CHANGE_JOB_IRON_PALADIN:
	case CXSLItem::CI_CHANGE_JOB_DEADLY_CHASER:
#ifdef SERV_ELSWORD_SHEATH_KNIGHT
	case CXSLItem::CI_CHANGE_JOB_SHEATH_KNIGHT:
#endif SERV_ELSWORD_SHEATH_KNIGHT
#ifdef SERV_ADD_ARME_BATTLE_MAGICIAN
	case CXSLItem::CI_CHANGE_JOB_BATTLE_MAGICIAN:
#endif SERV_ADD_ARME_BATTLE_MAGICIAN
#ifdef SERV_TRAPPING_RANGER_TEST
	case CXSLItem::CI_CHANGE_JOB_TRAPPING_RANGER:
#endif SERV_TRAPPING_RANGER_TEST
#ifdef SERV_RAVEN_WEAPON_TAKER
	case CXSLItem::CI_CHANGE_JOB_WEAPON_TAKER:
#endif SERV_RAVEN_WEAPON_TAKER
#ifdef SERV_EVE_ELECTRA
	case CXSLItem::CI_CHANGE_JOB_EVE_ELECTRA:
#endif SERV_EVE_ELECTRA
#ifdef SERV_ADD_CHUNG_SHELLING_GUARDIAN
	case CXSLItem::CI_CHANGE_JOB_SHELLING_GUARDIAN:
#endif
#ifdef SERV_ELSWORD_INFINITY_SWORD
	case CXSLItem::CI_CHANGE_JOB_INFINITY_SWORD:
#endif SERV_ELSWORD_INFINITY_SWORD
#ifdef SERV_ARME_DIMENSION_WITCH
	case CXSLItem::CI_CHANGE_JOB_DIMENSION_WITCH:
#endif SERV_ARME_DIMENSION_WITCH
#ifdef SERV_RENA_NIGHT_WATCHER
	case CXSLItem::CI_CHANGE_JOB_NIGHT_WATCHER:
#endif SERV_RENA_NIGHT_WATCHER
#ifdef SERV_RAVEN_VETERAN_COMMANDER
	case CXSLItem::CI_CHANGE_JOB_VETERAN_COMMANDER:
#endif SERV_RAVEN_VETERAN_COMMANDER
#ifdef SERV_EVE_BATTLE_SERAPH
	case CXSLItem::CI_CHANGE_JOB_BATTLE_SERAPH:
#endif SERV_EVE_BATTLE_SERAPH
#ifdef SERV_CHUNG_TACTICAL_TROOPER
	case CXSLItem::CI_CHANGE_JOB_TACTICAL_TROOPER:
#endif
		//}}

		//{{ 2010. 04. 27  ������	����� ��Ʈ
#ifdef SERV_SKILL_NOTE
	case CXSLItem::CI_CASH_SKILL_NOTE_ITEM:
#endif SERV_SKILL_NOTE
		//}}
		//{{ 2011. 04. 14	������	�븮 ���� / �븮 ������ ĳ�� �κ����� ���� ����� �� �ֵ��� ����
		//#ifdef SERV_PSHOP_AGENCY
		//		case CXSLItem::CI_PERSONAL_SHOP_AGENCY:
		//#endif
		//}}
		//{{ ���� - ���� ���� ����Ʈ, ĳ�� �۾� 
#ifdef SERV_SHARING_BANK_QUEST_CASH
	case CXSLItem::CI_CASH_SHARING_BACK_OPEN:
#endif
		//}}
#ifdef	SERV_EXPAND_QUICK_SLOT
	case CXSLItem::CI_EXPAND_QUICK_SLOT:
#endif  SERV_EXPAND_QUICK_SLOT	
#ifdef	SERV_UNLIMITED_SECOND_CHANGE_JOB
	case CXSLItem::CI_UNLIMITED_CHANGE_SECOND_JOB:
#endif  SERV_UNLIMITED_SECOND_CHANGE_JOB	
		{
			return true;
		}
		break;



		//{{ 2009. 8. 4  ������		���ν�ų
	default:
		{

			if( SiCXSLSkillTree()->IsUnsealItemID( iItemID )  || CXSLItem::IsJobChangeCashItem( iItemID )
#ifdef SERV_UNIT_CLASS_CHANGE_ITEM
				|| CXSLItem::GetClassChangeCashItem(iItemID) != CXSLUnit::UC_NONE
#endif	
				)
			{
				return true;
			}
		}
		break;
		//}}
	}

	return false;
}

int KGSUser::OnPickUpAck( const KENX_BT_NISMS_INVENTORY_PICK_UP_ONCE_ACK& kPacket_, const KEBILL_PICK_UP_ACK& kPacket2_, std::vector< KInventoryItemInfo >& vecInventorySlotInfo )
{
	if( kPacket2_.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"ĳ�� ������ ���� ���� ����." )
			<< BUILD_LOG( GetUID() )
			<< BUILD_LOG( GetName() )
			<< BUILD_LOG( kPacket2_.m_iOK )
			<< BUILD_LOG( NetError::GetErrStr( kPacket2_.m_iOK ) )
			<< END_LOG;

		return NetError::ERR_NX_SHOP_03;
	}

	std::map< int, KItemInfo > mapItem; // �κ��丮�� �� �����۵�
	//{{ 2009. 9. 2  ������		�к�
	std::set< int > setSealCashItem;
	//}}

	int iItemID = 0;
	int iQuantity = 0;
	int iPeriod = 0;
	KBillProductInfo kBillProductInfo;

	SiKBillManager()->GetProductInfo( kPacket2_.m_iProductNo, kBillProductInfo );

	iItemID = kBillProductInfo.m_iProductID;
	iQuantity = ( int )kBillProductInfo.m_cQuantity;
	iPeriod = ( int )kBillProductInfo.m_cPeriod;

	if( iItemID <= 0 )
	{
		START_LOG( cerr, L"������ ID ��ȯ ����." )
			<< BUILD_LOG( kPacket_.m_ulProductNo )
			<< BUILD_LOG( kPacket2_.m_iProductNo )
			<< BUILD_LOG( iItemID )
			<< END_LOG;

		return NetError::ERR_NX_SHOP_01;
	}

	const CXSLItem::ItemTemplet* pItemTemplet = SiCXSLItemManager()->GetItemTemplet( iItemID );
	if( pItemTemplet == NULL )
	{
		START_LOG( cerr, L"������ ���ø��� ���� ����." )
			<< BUILD_LOG( iItemID )
			<< END_LOG;

		return NetError::ERR_NX_SHOP_01;
	}

	// 20080416. ������� ���� ���������� �������� ä�������. �ڵ� ������ �� �� ���� ���.

	// ĳ�� ������ �������� �α�
	KELOG_GET_PURCHASED_CASH_ITEM_NOT kPacketNot;
	kPacketNot.m_iUnitUID = GetCharUID();
	kPacketNot.m_iItemID = iItemID;
	kPacketNot.m_sPeriod = ( short )iPeriod;
	kPacketNot.m_iQuantity = iQuantity;
	kPacketNot.m_ucLevel =  GetLevel();
	kPacketNot.m_cUnitClass = GetUnitClass();

	SendToLogDB( ELOG_GET_PURCHASED_CASH_ITEM_NOT, kPacketNot );


	switch( pItemTemplet->m_ItemID )
	{
	case CXSLItem::CI_RESURRECTION_STONE: // ��Ȱ��
		{
			// ��Ȱ���̸� �ٷ� ä���ش�.
			m_iNumResurrectionStone += iQuantity;

			START_LOG( clog2, L"resurr enter" )
				<< END_LOG;

			// �ǽð� ��Ȱ��
			if( m_kUserCashInventory.IsCashResurrectionStone() )
			{
				START_LOG( clog2, L"resurr send" )
					<< END_LOG;
				KEGS_RESURRECT_TO_CONTINUE_DUNGEON_REQ kPacketReq;
				kPacketReq.m_iUnitUID		= m_kUserCashInventory.GetCashRStoneUnitUID();
				kPacketReq.m_iUsedByUnitUID = GetCharUID();
				SendToCnRoom( ERM_RESURRECT_TO_CONTINUE_DUNGEON_REQ, kPacketReq );

			}
#ifdef GIANT_RESURRECTION_CASHSTONE
			else
			{
				KDBE_RESURRECTION_CASHSTONE_NOT kPacketReq2;
				kPacketReq2.m_iUnitUID = GetCharUID();
				kPacketReq2.m_iNumResurrection_CashStone = iQuantity;
				SendToGameDB(DBE_RESURRECTION_CASHSTONE_NOT,kPacketReq2 );
			}
#endif //GIANT_RESURRECTION_CASHSTONE


			return NetError::NET_OK;
		}
		break;

	case CXSLItem::CI_EXPAND_INVENTORY: // ����ǰ Ȯ��
		{
			// ����ǰ Ȯ���̸� DB�� �̺�Ʈ�� ������.
			int nSlot;
			nSlot = CXSLInventory::SLOT_COUNT_ONE_LINE * iQuantity;

			KDBE_EXPAND_INVENTORY_SLOT_REQ kPacket;
			kPacket.m_usEventID = EGS_GET_PURCHASED_CASH_ITEM_REQ;
			kPacket.m_iUnitUID = GetCharUID();
			kPacket.m_mapExpandedSlot.insert( std::make_pair( ( int )CXSLInventory::ST_EQUIP, nSlot ) );
			kPacket.m_mapExpandedSlot.insert( std::make_pair( ( int )CXSLInventory::ST_ACCESSORY, nSlot ) );
			kPacket.m_mapExpandedSlot.insert( std::make_pair( ( int )CXSLInventory::ST_MATERIAL, nSlot ) );
			kPacket.m_mapExpandedSlot.insert( std::make_pair( ( int )CXSLInventory::ST_SPECIAL, nSlot ) );
			kPacket.m_mapExpandedSlot.insert( std::make_pair( ( int )CXSLInventory::ST_QUEST, nSlot ) );
			kPacket.m_mapExpandedSlot.insert( std::make_pair( ( int )CXSLInventory::ST_QUICK_SLOT, nSlot ) );
			SendToGameDB( DBE_EXPAND_INVENTORY_SLOT_REQ, kPacket );
			return NetError::ERR_NX_SHOP_09;
		}
		break;

	case CXSLItem::CI_EXPAND_INVENTORY_EQUIP: // ����ǰ Ȯ��(���)
		{
			// ����ǰ Ȯ���̸� DB�� �̺�Ʈ�� ������.
			int nSlot = CXSLInventory::SLOT_COUNT_ONE_LINE;
			KDBE_EXPAND_INVENTORY_SLOT_REQ kPacket;
			kPacket.m_usEventID = EGS_GET_PURCHASED_CASH_ITEM_REQ;
			kPacket.m_iUnitUID = GetCharUID();
			kPacket.m_mapExpandedSlot.insert( std::make_pair( ( int )CXSLInventory::ST_EQUIP, nSlot ) );
			SendToGameDB( DBE_EXPAND_INVENTORY_SLOT_REQ, kPacket );
			//continue;
			return NetError::ERR_NX_SHOP_09; // ������ �������� �ʰ� �ٷ� ����
		}
		break;

	case CXSLItem::CI_EXPAND_INVENTORY_ACCESSORY: // ����ǰ Ȯ��(�׼�����)
		{
			// ����ǰ Ȯ���̸� DB�� �̺�Ʈ�� ������.
			int nSlot = CXSLInventory::SLOT_COUNT_ONE_LINE;
			KDBE_EXPAND_INVENTORY_SLOT_REQ kPacket;
			kPacket.m_usEventID = EGS_GET_PURCHASED_CASH_ITEM_REQ;
			kPacket.m_iUnitUID = GetCharUID();
			kPacket.m_mapExpandedSlot.insert( std::make_pair( ( int )CXSLInventory::ST_ACCESSORY, nSlot ) );					
			SendToGameDB( DBE_EXPAND_INVENTORY_SLOT_REQ, kPacket );
			//continue;
			return NetError::ERR_NX_SHOP_09; // ������ �������� �ʰ� �ٷ� ����
		}
		break;

	case CXSLItem::CI_EXPAND_INVENTORY_QUICK_SLOT: // ����ǰ Ȯ��(�Һ�)
		{
			// ����ǰ Ȯ���̸� DB�� �̺�Ʈ�� ������.
			int nSlot = CXSLInventory::SLOT_COUNT_ONE_LINE;
			KDBE_EXPAND_INVENTORY_SLOT_REQ kPacket;
			kPacket.m_usEventID = EGS_GET_PURCHASED_CASH_ITEM_REQ;
			kPacket.m_iUnitUID = GetCharUID();
			kPacket.m_mapExpandedSlot.insert( std::make_pair( ( int )CXSLInventory::ST_QUICK_SLOT, nSlot ) );
			SendToGameDB( DBE_EXPAND_INVENTORY_SLOT_REQ, kPacket );
			//continue;
			return NetError::ERR_NX_SHOP_09; // ������ �������� �ʰ� �ٷ� ����
		}
		break;

	case CXSLItem::CI_EXPAND_INVENTORY_MATERIAL: // ����ǰ Ȯ��(���)
		{
			// ����ǰ Ȯ���̸� DB�� �̺�Ʈ�� ������.
			int nSlot = CXSLInventory::SLOT_COUNT_ONE_LINE;
			KDBE_EXPAND_INVENTORY_SLOT_REQ kPacket;
			kPacket.m_usEventID = EGS_GET_PURCHASED_CASH_ITEM_REQ;
			kPacket.m_iUnitUID = GetCharUID();
			kPacket.m_mapExpandedSlot.insert( std::make_pair( ( int )CXSLInventory::ST_MATERIAL, nSlot ) );					
			SendToGameDB( DBE_EXPAND_INVENTORY_SLOT_REQ, kPacket );
			//continue;
			return NetError::ERR_NX_SHOP_09; // ������ �������� �ʰ� �ٷ� ����
		}
		break;

	case CXSLItem::CI_EXPAND_INVENTORY_QUEST: // ����ǰ Ȯ��(����Ʈ)
		{
			// ����ǰ Ȯ���̸� DB�� �̺�Ʈ�� ������.
			int nSlot = CXSLInventory::SLOT_COUNT_ONE_LINE;
			KDBE_EXPAND_INVENTORY_SLOT_REQ kPacket;
			kPacket.m_usEventID = EGS_GET_PURCHASED_CASH_ITEM_REQ;
			kPacket.m_iUnitUID = GetCharUID();
			kPacket.m_mapExpandedSlot.insert( std::make_pair( ( int )CXSLInventory::ST_QUEST, nSlot ) );
			SendToGameDB( DBE_EXPAND_INVENTORY_SLOT_REQ, kPacket );
			//continue;
			return NetError::ERR_NX_SHOP_09; // ������ �������� �ʰ� �ٷ� ����
		}
		break;

	case CXSLItem::CI_EXPAND_INVENTORY_SPECIAL: // ����ǰ Ȯ��(Ư��)
		{
			// ����ǰ Ȯ���̸� DB�� �̺�Ʈ�� ������.
			int nSlot = CXSLInventory::SLOT_COUNT_ONE_LINE;
			KDBE_EXPAND_INVENTORY_SLOT_REQ kPacket;
			kPacket.m_usEventID = EGS_GET_PURCHASED_CASH_ITEM_REQ;
			kPacket.m_iUnitUID = GetCharUID();
			kPacket.m_mapExpandedSlot.insert( std::make_pair( ( int )CXSLInventory::ST_SPECIAL, nSlot ) );					
			SendToGameDB( DBE_EXPAND_INVENTORY_SLOT_REQ, kPacket );
			//continue;
			return NetError::ERR_NX_SHOP_09; // ������ �������� �ʰ� �ٷ� ����
		}
		break;
		//{{ 2011. 12. 13  ��μ�	�� ���� Ȯ�� ������
#ifdef SERV_EXPAND_QUICK_SLOT
	case CXSLItem::CI_EXPAND_QUICK_SLOT:
		{
			// ����ǰ Ȯ���̸� DB�� �̺�Ʈ�� ������.
			int nSlot = CXSLInventory::EXPAND_QUICK_SLOT;
			KDBE_EXPAND_INVENTORY_SLOT_REQ kPacket;
			kPacket.m_usEventID = EGS_GET_PURCHASED_CASH_ITEM_REQ;
			kPacket.m_iUnitUID = GetCharUID();
			kPacket.m_mapExpandedSlot.insert( std::make_pair( ( int )CXSLInventory::ST_E_QUICK_SLOT, nSlot ) );					
			SendToGameDB( DBE_EXPAND_INVENTORY_SLOT_REQ, kPacket );
			return NetError::ERR_NX_SHOP_09; // ������ �������� �ʰ� �ٷ� ����
		}
		break;
#endif SERV_EXPAND_QUICK_SLOT
		//}}

	case CXSLItem::CI_EXPAND_SKILL_SLOT: // ��ų ���� ü���� �޴�
		{
			KDBE_EXPAND_SKILL_SLOT_REQ kPacket;
			kPacket.m_iUnitUID		= GetCharUID();
			kPacket.m_iPeriodExpire = iPeriod;
			SendToGameDB( DBE_EXPAND_SKILL_SLOT_REQ, kPacket );
			return NetError::ERR_NX_SHOP_09; // ������ �������� �ʰ� �ٷ� ����
		}
		break;

	case CXSLItem::CI_EXPAND_SKILL_SLOT_PERMANENT:
		{
			KDBE_EXPAND_SKILL_SLOT_REQ kPacket;
			kPacket.m_iUnitUID		= GetCharUID();
			kPacket.m_iPeriodExpire = 0;
			SendToGameDB( DBE_EXPAND_SKILL_SLOT_REQ, kPacket );
			return NetError::ERR_NX_SHOP_09; // ������ �������� �ʰ� �ٷ� ����
		}
		break;
#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013
	case CXSLItem::CI_CASH_SKILL_POINT_60_15:
	case CXSLItem::CI_CASH_SKILL_POINT_60_30:
	case CXSLItem::CI_CASH_SKILL_POINT_30_15:
	case CXSLItem::CI_CASH_SKILL_POINT_30_30:
#ifdef SERV_CASH_SKILL_POINT_TW
	case CXSLItem::CI_CASH_SKILL_POINT_30_7:
	case CXSLItem::CI_CASH_SKILL_POINT_60_7:
#endif SERV_CASH_SKILL_POINT_TW
#else
	case CXSLItem::CI_CASH_SKILL_POINT_5:
	case CXSLItem::CI_CASH_SKILL_POINT_10:
#endif SERV_UPGRADE_SKILL_SYSTEM_2013
		{
			const int iCSPoint = SiCXSLItemManager()->GetItemCSPoint( pItemTemplet->m_ItemID );

			if( iCSPoint <= 0 )
			{
				return NetError::ERR_RESET_SKILL_01;
			}

			if( true == m_kSkillTree.IsCashSkillPointExpired() )
			{
				KDBE_INSERT_CASH_SKILL_POINT_REQ kPacket;
				kPacket.m_iUnitUID	= GetCharUID();
				kPacket.m_iCSPoint	= iCSPoint;
				//{{ 2009. 12. 2  ������	ĳ���۱ⰣŸ�Ժ���
				kPacket.m_iPeriod	= iPeriod;
				//}}
				kPacket.m_bUpdateEndDateOnly = false;
				//{{ 2010. 12. 8	������	�̺�Ʈ�� �׳�ý� �ູ
				kPacket.m_iSkillPointItemID = pItemTemplet->m_ItemID;
				//}}
				SendToGameDB( DBE_INSERT_CASH_SKILL_POINT_REQ, kPacket );
			}
			else
			{
				if( iCSPoint == m_kSkillTree.GetMaxCSPoint() )
				{
					KDBE_INSERT_CASH_SKILL_POINT_REQ kPacket;
					kPacket.m_iUnitUID = GetCharUID();
					kPacket.m_iCSPoint = iCSPoint;
					//{{ 2009. 12. 2  ������	ĳ���۱ⰣŸ�Ժ���
					kPacket.m_iPeriod = iPeriod;
					//}}
					kPacket.m_bUpdateEndDateOnly = true;
					//{{ 2010. 12. 8	������	�̺�Ʈ�� �׳�ý� �ູ
					kPacket.m_iSkillPointItemID = pItemTemplet->m_ItemID;
					//}}
					SendToGameDB( DBE_INSERT_CASH_SKILL_POINT_REQ, kPacket );
				}
				else
				{
					START_LOG( cerr, L"���� ����ϰ� �ִ� ĳ�ý�ų����Ʈ�� �ٸ� ������ �Ⱓ���� �Ϸ�����!!" )
						<< BUILD_LOG( GetCharUID() )
						<< BUILD_LOG( pItemTemplet->m_ItemID )
						//{{ 2009. 12. 2  ������	ĳ���۱ⰣŸ�Ժ���
						<< BUILD_LOG( iPeriod )
						//}}
						<< BUILD_LOG( m_kSkillTree.GetMaxCSPoint() )
						<< BUILD_LOG( m_kSkillTree.GetCSPointEndDateString() )
						<< END_LOG;

					return NetError::ERR_NX_SHOP_01;
				}
			}

			return NetError::ERR_NX_SHOP_09; // ������ �������� �ʰ� �ٷ� ����
		} break;
		//{{ 2008. 12. 14  ������	ĳ���� ���� Ȯ��
	case CXSLItem::CI_EXPAND_CHAR_SLOT_1:
	case CXSLItem::CI_EXPAND_CHAR_SLOT_2:
		{
			KDBE_EXPAND_CHAR_SLOT_REQ kPacket;
			kPacket.m_iExpandSlotSize = ( pItemTemplet->m_ItemID == CXSLItem::CI_EXPAND_CHAR_SLOT_1 ? 1 : 2 );
			kPacket.m_iCharSlotMax = CXSLUnit::CHAR_SLOT_MAX;
			SendToGameDB( DBE_EXPAND_CHAR_SLOT_REQ, kPacket );
			return NetError::ERR_NX_SHOP_09;
		}
		break;
		//}}

		//{{ 2009. 8. 7  ������		����
	case CXSLItem::CI_BANK_MEMBERSHIP_SILVER:
	case CXSLItem::CI_BANK_MEMBERSHIP_GOLD:
	case CXSLItem::CI_BANK_MEMBERSHIP_EMERALD:
	case CXSLItem::CI_BANK_MEMBERSHIP_DIAMOND:
	case CXSLItem::CI_BANK_MEMBERSHIP_PLATINUM:
		{
			// ���� Ȯ���Ϸ� ����!
			KDBE_EXPAND_INVENTORY_SLOT_REQ kPacket;
			kPacket.m_usEventID = EGS_GET_PURCHASED_CASH_ITEM_REQ;
			kPacket.m_iUnitUID = GetCharUID();
			kPacket.m_mapExpandedSlot.insert( std::make_pair( ( int )CXSLInventory::ST_BANK, CXSLInventory::SLOT_COUNT_ONE_LINE ) );
			SendToGameDB( DBE_EXPAND_INVENTORY_SLOT_REQ, kPacket );
			return NetError::ERR_NX_SHOP_09;
		}
		break;

	case CXSLItem::CI_BANK_MEMBERSHIP_ONE_PACKAGE:
		{
			// ���� Ȯ���Ϸ� ����!
			KDBE_EXPAND_INVENTORY_SLOT_REQ kPacket;
			kPacket.m_usEventID = EGS_GET_PURCHASED_CASH_ITEM_REQ;
			kPacket.m_iUnitUID = GetCharUID();
			kPacket.m_mapExpandedSlot.insert( std::make_pair( ( int )CXSLInventory::ST_BANK, ( CXSLInventory::SLOT_COUNT_ONE_LINE * 5 ) ) );
			SendToGameDB( DBE_EXPAND_INVENTORY_SLOT_REQ, kPacket );
			return NetError::ERR_NX_SHOP_09;
		}
		break;
		//}}

		//{{ 2010. 04. 09  ������	����� ��Ʈ
#ifdef SERV_SKILL_NOTE
	case CXSLItem::CI_CASH_SKILL_NOTE_ITEM:
		{
			char cSkillNoteMaxPageNum = 0;
			if( m_kSkillTree.GetExpandSkillNotePage( GetLevel(), cSkillNoteMaxPageNum ) == true )
			{
				// �ϴ� ����� ��Ʈ ������ ���� ��������!
				m_kSkillTree.UpdateSkillNoteMaxPageNum( cSkillNoteMaxPageNum );

				// ����� ��Ʈ�� ȹ�� �Ϸ� ����!
				KDBE_EXPAND_SKILL_NOTE_PAGE_REQ kPacketToDB;
				kPacketToDB.m_iUnitUID = GetCharUID();
				kPacketToDB.m_cExpandedMaxPageNum = cSkillNoteMaxPageNum;
				kPacketToDB.m_cRollBackMaxPageNum = 0;
				SendToGameDB( DBE_EXPAND_SKILL_NOTE_PAGE_REQ, kPacketToDB );
				return NetError::ERR_NX_SHOP_09;
			}
			else
			{
				START_LOG( cerr, L"����� ��Ʈ�� �����Ϸ��� �ϴµ� ������ �� ��⸦ �����Ͽ����ϴ�." )
					<< BUILD_LOG( GetCharUID() )
					<< BUILD_LOG( GetLevel() )
					<< END_LOG;
			}
		}
		break;
#endif SERV_SKILL_NOTE
		//}}
		//{{ 2011. 04. 14	������	�븮 ����
#ifdef SERV_PSHOP_AGENCY
	case CXSLItem::CI_PERSONAL_SHOP_AGENCY:
		{
			KDBE_INSERT_PERIOD_PSHOP_AGENCY_REQ kPacketToDB;
			kPacketToDB.m_iUnitUID = GetCharUID();
			kPacketToDB.m_sAgencyPeriod = iPeriod;
			kPacketToDB.m_usEventID = EGS_GET_PURCHASED_CASH_ITEM_REQ;
			SendToGameDB( DBE_INSERT_PERIOD_PSHOP_AGENCY_REQ, kPacketToDB );

			// ���� : ĳ�� �κ� ����� ���� �ؾ� �ϴµ�....
			//		NET_OK �������ָ� ���� ������?
			return NetError::NET_OK;
			//return NetError::ERR_NX_SHOP_09;// ���� : �ӽ�
		}
		break;
#endif SERV_PSHOP_AGENCY
		//}}
		//{{ ���� - ���� ���� ����Ʈ, ĳ�� �۾� 
#ifdef SERV_SHARING_BANK_QUEST_CASH
	case CXSLItem::CI_CASH_SHARING_BACK_OPEN:
		{
			// ĳ���� ���� ����~
			KDBE_SHARING_BACK_OPEN_REQ kPacketToDB;
			kPacketToDB.m_iUnitUID = GetCharUID();
			kPacketToDB.m_iUserUID = GetUID();
			kPacketToDB.m_iOpenType = KDBE_SHARING_BACK_OPEN_REQ::SBOT_CASH;
			SendToGameDB( DBE_SHARING_BACK_OPEN_REQ, kPacketToDB );

			return NetError::ERR_NX_SHOP_09;
		}
		break;
#endif //SERV_SHARING_BANK_QUEST_CASH
		//}}
#ifdef SERV_ADD_WARP_BUTTON
	case CXSLItem::CI_WARP_VIP_ITEM:
#ifdef SERV_VIP_SYSTEM
	case CXSLItem::SI_USE_FREE_BY_FIELD_VIP:
#endif SERV_VIP_SYSTEM
		{
			KDBE_INSERT_WARP_VIP_REQ kPacket;
			kPacket.m_iUnitUID	= GetCharUID();
			kPacket.m_iPeriod	= 30;		
			SendToGameDB( DBE_INSERT_WARP_VIP_REQ, kPacket );

			return NetError::NET_OK; // ������ �������� �ʰ� �ٷ� ����
		}
		break;
#endif // SERV_ADD_WARP_BUTTON
		//{{ 2012. 02. 22	�ڼ���	��� �̸� �����
#ifdef SERV_GUILD_CHANGE_NAME
	case CXSLItem::CI_GUILD_NAME_CHANGE:
		{
			KELG_CHANGE_GUILD_NAME_REQ kPacket;
			kPacket.m_iUnitUID = GetCharUID();
			kPacket.m_iGuildUID = GetGuildUID();
			kPacket.m_wstrOldGuildName = m_kUserGuildManager.GetGuildName();
			// m_wstrNewGuildName�� LoginServer �ʿ��� ���� ���̴�.
			SendToLoginServer( ELG_CHANGE_GUILD_NAME_REQ, kPacket );

			return NetError::NET_OK;
		}
		break;
#endif //SERV_GUILD_CHANGE_NAME
		//{{ 2009. 8. 4  ������		���ν�ų
#ifdef SERV_UNLIMITED_SECOND_CHANGE_JOB
	case CXSLItem::CI_UNLIMITED_CHANGE_SECOND_JOB:
		{
			CStringW cwstrItemID;
			cwstrItemID.Format( L"%d", CXSLItem::CI_UNLIMITED_CHANGE_SECOND_JOB );
			// 2�� ���� item������.
			// DB�� ���� ������ ������.
			KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
			kPacketToDB.m_iFromUnitUID = GetCharUID();
			kPacketToDB.m_iToUnitUID   = GetCharUID();
			kPacketToDB.m_iRewardType  = KPostItemInfo::LT_MESSAGE;
			kPacketToDB.m_iRewardID	   = CXSLItem::SI_SECOND_CHANGE_JOB_CUBE;
			kPacketToDB.m_sQuantity	   = 1;
			kPacketToDB.m_wstrMessage  = cwstrItemID.GetBuffer();
			kPacketToDB.m_bGameServerEvent = false;
			SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
		}
		break;
#endif //SERV_UNLIMITED_SECOND_CHANGE_JOB
	default:
		{
			//{{ 2009. 9. 22  ������	����ĳ��
			if( CXSLItem::IsJobChangeCashItem( iItemID ) )
			{
				// ���� �Ϸ� ����!
				KDBE_CHANGE_UNIT_CLASS_REQ kPacket;
				kPacket.m_usEventID = EGS_BUY_CASH_ITEM_REQ;
				kPacket.m_iUnitUID = GetCharUID();
				kPacket.m_cUnitClass = CXSLItem::GetCashItemChangeUnitClass( pItemTemplet->m_ItemID );

				int iDefaultSkill[6] = {0,};
				if( SiCXSLSkillTree()->GetUnitClassDefaultSkill( kPacket.m_cUnitClass, iDefaultSkill[0], iDefaultSkill[1], iDefaultSkill[2], iDefaultSkill[3], iDefaultSkill[4], iDefaultSkill[5] ) == false )
				{
					START_LOG( cerr, L"������ �����Ϸ��� �⺻ ��ų�� �̻���" )
						<< BUILD_LOG( kPacket.m_iUnitUID )
						<< BUILD_LOGc( kPacket.m_cUnitClass )
						<< END_LOG;
				}

				if( CXSLUnit::IsInitNormalJob( static_cast<CXSLUnit::UNIT_CLASS>(kPacket.m_cUnitClass) ) == true )
				{
					kPacket.m_iNewDefaultSkill1 = iDefaultSkill[0];
					kPacket.m_iNewDefaultSkill2 = iDefaultSkill[1];
				}
				else if( CXSLUnit::IsFirstChangeJob( static_cast<CXSLUnit::UNIT_CLASS>(kPacket.m_cUnitClass) ) == true )
				{
					kPacket.m_iNewDefaultSkill1 = iDefaultSkill[2];
					kPacket.m_iNewDefaultSkill2 = iDefaultSkill[3];
				}
				else if( CXSLUnit::IsSecondChangeJob( static_cast<CXSLUnit::UNIT_CLASS>(kPacket.m_cUnitClass) ) == true )
				{
					kPacket.m_iNewDefaultSkill1 = iDefaultSkill[4];
					kPacket.m_iNewDefaultSkill2 = iDefaultSkill[5];
				}
				SendToGameDB( DBE_CHANGE_UNIT_CLASS_REQ, kPacket );
				return NetError::ERR_NX_SHOP_09;
			}
			//}}
			//{{ 2011. 11. 21  ��μ�	���� ���� ������
			int iUnitClass = 0;
			if( CXSLItem::GetClassChangeCashItem( iItemID ) != CXSLUnit::UC_NONE )
			{
				KDBE_BUY_UNIT_CLASS_CHANGE_REQ kPacketToDB;

				if( GetUnitClassChangeInfo( iItemID, kPacketToDB ) == true )
				{
					SendToGameDB( DBE_BUY_UNIT_CLASS_CHANGE_REQ, kPacketToDB );
				}
				else
				{
					START_LOG( cerr, L"ĳ�� ������ �Ⱦ� �������� ���а� ����! ���� �ؾ� �� �� �ѵ�!!!" )
						<< BUILD_LOG( GetCharUID() )
						<< BUILD_LOG( iItemID )
						<< END_LOG;
				}
				return NetError::ERR_NX_SHOP_09;
			}
			//}}

			//{{ 2009. 8. 4  ������		���ν�ų
			std::vector<int> vecSkillID;
			if( SiCXSLSkillTree()->GetUnsealItemInfo( iItemID, GetUnitClass(), vecSkillID ) == true )
			{
				BOOST_TEST_FOREACH( int, iSkillID, vecSkillID )
				{
					KDBE_UNSEAL_SKILL_REQ kPacketToDB;
					kPacketToDB.m_iUnitUID = GetCharUID();
					kPacketToDB.m_iSkillID = iSkillID;
					SendToGameDB( DBE_UNSEAL_SKILL_REQ, kPacketToDB );
				}
				return NetError::ERR_NX_SHOP_09;
			}
			//}}
		}
		//}}
	}

	KItemInfo kItemInfo;
	kItemInfo.m_iItemID		= iItemID;
	kItemInfo.m_cUsageType	= pItemTemplet->m_PeriodType;
	kItemInfo.m_iQuantity	= iQuantity;
	kItemInfo.m_sEndurance	= ( pItemTemplet->m_PeriodType == CXSLItem::PT_ENDURANCE ) ? pItemTemplet->m_Endurance : 0;
	kItemInfo.m_sPeriod		= ( short )iPeriod;
	//{{ 2009. 8. 27  ������	�к�
	if( SiCXSLItemManager()->IsCanSealForCashItem( iItemID ) == true )
	{
		setSealCashItem.insert( pItemTemplet->m_ItemID );
	}
	//}}

	//{{ 2012. 11. 28 ĳ�� ������ ���� �ɼ� ��� ���� - ��μ�
	int iSocketOption = 0;
	if( m_kUserCashInventory.GetSocketOptionForCash( iItemID, iSocketOption ) )
	{
		int iItemType = static_cast<int>( pItemTemplet->m_ItemType );

		if( SiCXSLSocketItem()->CheckCashItemSocketID( iItemType, iSocketOption ) == true )
		{
			kItemInfo.m_vecItemSocket.push_back( iSocketOption );
		}
	}
	//}
	
	mapItem.insert( std::make_pair( iItemID, kItemInfo ) );


	// �κ��丮�� ������ ���� �غ�
	KDBE_INSERT_ITEM_REQ kPacketReq;
	//{{ 2010. 9. 8	������	������ ȹ�� ����
	kPacketReq.m_cGetItemReason = SEnum::GIR_BUY_CASH_ITEM;

	m_kInventory.PrepareInsert( mapItem, kPacketReq.m_mapInsertedItem, kPacketReq.m_vecUpdatedInventorySlot, kPacketReq.m_vecItemInfo );

	if( !kPacketReq.m_vecItemInfo.empty() )
	{
		//{{ 2009. 9. 2  ������		����
		kPacketReq.m_setSealCashItem = setSealCashItem;
		//}}
		kPacketReq.m_iUnitUID = GetCharUID();
		m_kInventory.FlushQuantityChange( kPacketReq.m_kItemQuantityUpdate.m_mapQuantityChange );
		m_kInventory.FlushEnduranceChange( kPacketReq.m_kItemEnduranceUpdate );
		m_kInventory.FlushDeletedItem( kPacketReq.m_kItemQuantityUpdate.m_vecDeleted );
		SendToGameDB( DBE_INSERT_PURCHASED_CASH_ITEM_REQ, kPacketReq );
		return NetError::ERR_NX_SHOP_09;
	}
	else
	{
		vecInventorySlotInfo = kPacketReq.m_vecUpdatedInventorySlot;
	}

	// ĳ�� ������ ���
	std::map< int, int >::const_iterator mit;
	for( mit = kPacketReq.m_mapInsertedItem.begin(); mit != kPacketReq.m_mapInsertedItem.end(); ++mit )
	{
		KStatisticsKey kKey;
		kKey.m_vecIntKey.push_back( mit->first );
		KSIManager.IncreaseCount( KStatistics::SI_LOC_ITEM, kKey, KStatistics::SI_ITEM_MALL, mit->second );
		//{{ 2011. 04. 13	������	������ ��� DB���
		KSIManager.IncreaseCount( KStatistics::SI_ITEM_DB, kKey, KStatistics::SI_ITEM_MALL, mit->second );
	}

	//{{ 2010. 07. 05  ������	���� ����� �Ŵ���
	// ������ �����
	m_kUserAbuserManager.CheckItemAbuser( GetThisPtr<KGSUser>(), KAbuserLogManager::RS_CASH_ITEM, kPacketReq.m_mapInsertedItem );
	//}}

	return NetError::NET_OK;
}

//
std::wstring KGSUser::MakeOrderID()
{
	std::wstring wstrRet;

	CTime tCurrent = CTime::GetCurrentTime();
	wstrRet = ( CStringW )tCurrent.Format( _T( "%Y%m%d%H%M%S" ) );
	wchar_t wszNumber[32];
	_i64tow( KBaseServer::GetKObj()->GetUID(), wszNumber, 10 );
	wstrRet += L"_";
	wstrRet += wszNumber;
	_i64tow( GetUID(), wszNumber, 10 );
	wstrRet += L"_";
	wstrRet += wszNumber;
	_i64tow( SiKBillManager()->GetNextPurchaseNo(), wszNumber, 10 );
	wstrRet += L"_";
	wstrRet += wszNumber;

	return wstrRet;
}

// �������� ��밡���� ĳ�� ������ �ٸ�
bool KGSUser::IsAbleToUseCashType(int iCashType)
{
	if(iCashType < 0 || iCashType >= KGlobalCashInfo::GCT_TOTAL_NUM)
		return false;

#if defined (SERV_COUNTRY_CN) 
	if (iCashType == KGlobalCashInfo::GCT_KOG_ELSWORD_CASH ||
		iCashType == KGlobalCashInfo::GCT_KOG_ELSWORD_BONUS_POINT)
		return true;
#elif defined(SERV_COUNTRY_TWHK)
	if (iCashType == KGlobalCashInfo::GCT_PUBLISHER_CASH ||
		iCashType == KGlobalCashInfo::GCT_PUBLISHER_ELSWORD_CASH)
		return true;
#elif defined(SERV_COUNTRY_PH)
	if (iCashType == KGlobalCashInfo::GCT_KOG_ELSWORD_CASH )
		return true;
#else // SERV_COUNTRY_XX
	// �߱�,�븸ȫ�� �����ϰ��� ĳ�� ���� �ϳ��ۿ� �Ⱦ�
	if (iCashType == KGlobalCashInfo::GCT_PUBLISHER_CASH)
		return true;
#endif // SERV_COUNTRY_XX

	return false;
}

#endif // SERV_GLOBAL_BILLING
//////////////////////////////////////////////////////////////////////////


void KGSUser::ProcessEvent( const KEventPtr& spEvent_ )
{
	if( RoutePacket( spEvent_.get() ) )
		return;

	//{{ 2010. 9. 8	������	��Ʈ��ũ Ʈ���� ���� ���� ����͸�
#ifdef SERV_TRAFFIC_USER_MONITORING
	m_kUserAbuserManager.CheckTrafficAbuser( spEvent_->m_usEventID );
#endif SERV_TRAFFIC_USER_MONITORING
	//}}

	CheckRepeatEvent( spEvent_->m_usEventID );

	if( false == IsTransactionEnabled( spEvent_->m_usEventID ) )
		return;

	CheckTransactionReqEvent( spEvent_->m_usEventID ); // EGS_
	CheckTransactionAckEvent( spEvent_->m_usEventID ); // DBE_

	//{{ 2010. 07. 05  ������	���� ����� �Ŵ���
#ifdef SERV_USER_ABUSER_MANAGER
	//{{ 2013. 05. 29	������	��Ŷ ����͸� �ټ��� �׸� �߰�
#ifdef SERV_PACKET_MORNITORING_SPIRIT_COL
	m_kUserAbuserManager.CheckMornitoringAbuser( GetUID(), GetCharUID(), GetED(), m_kUserSpiritManager.GetSpirit(), spEvent_->m_usEventID );
#else
	m_kUserAbuserManager.CheckMornitoringAbuser( GetUID(), GetCharUID(), GetED(), spEvent_->m_usEventID );
#endif SERV_PACKET_MORNITORING_SPIRIT_COL
	//}}	
#endif SERV_USER_ABUSER_MANAGER
	//}}

	//{{ 2009. 9. 13  ������	�����α�
#ifdef USER_LOG
	m_kUserLogManager.SaveProcessEvent( spEvent_->m_usEventID );
#endif USER_LOG
	//}}

	DWORD dwElapTime = ::GetTickCount();  // 040428. ��Ŷ ó�� �ҿ�ð��� ���Ѵ�.

	switch( spEvent_->m_usEventID )
	{
	//////////////////////////////////////////////////////////////////////////
		//{{ 2011. 06. 08	������	KEvent ��ó ��� ���
#ifdef SERV_KEVENT_FROM
#	undef EVENT_HANDLER
#	define EVENT_HANDLER( eventid ) CASE_CHECK_ERR_AND_FILTER( eventid );
#	undef EVENT_HANDLER_NOPARAM
#	define EVENT_HANDLER_NOPARAM( eventid ) CASE_CHECK_ERR_AND_FILTER_NOPARAM( eventid );
#	undef _EVENT_HANDLER
#	define _EVENT_HANDLER( eventid, structtype ) _CASE_CHECK_ERR_AND_FILTER( eventid, structtype );
#pragma message( "--------------------------------------------" )
#pragma message( "Server Packet Event Filter!!!!!" )
#pragma message( "--------------------------------------------" )
#else
	//{{ 2011. 04. 04	������	deserializing ���� �α�
#	undef EVENT_HANDLER
#	define EVENT_HANDLER( eventid ) CASE_CHECK_ERR( eventid );
#	undef EVENT_HANDLER_NOPARAM
#	define EVENT_HANDLER_NOPARAM( eventid ) CASE_CHECK_ERR_NOPARAM( eventid );
#	undef _EVENT_HANDLER
#	define _EVENT_HANDLER( eventid, structtype ) _CASE_CHECK_ERR( eventid, structtype );
	//}}
#endif SERV_KEVENT_FROM
	//}}

	// ���⼭ ���� GSUser Event Handler�� ��� ������ �߰��մϴ�.
#	include "GSUserSession.h"
#	include "GSUserSecurity.h"
#	include "GSUserGameCommon.h"
#	include "GSUserField.h"
	//{{ 2011. 12. 05	������	��Ʋ�ʵ�
#ifdef SERV_BATTLE_FIELD_SYSTEM
#	include "GSUserNewParty.h"
#else
#	include "GSUserParty.h"
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}
#	include "GSUserInventory.h"
#	include "GSUserMessenger.h"
#	include "GSUserRoomCommon.h"
#	include "GSUserDungeon.h"
#	include "GSUserPvp.h"
#	include "GSUserTrade.h"
#	include "GSUserPersonalShop.h"
#	include "GSUserCashShop.h"
	//////////////////////////////////////////////////////////////////////////

	default:
		if( ProcessEventExpand( spEvent_ ) == false)
		{
			START_LOG( cerr, L"�̺�Ʈ �ڵ鷯�� ���ǵ��� �ʾ��� - " << spEvent_->GetIDStr() )
				<< BUILD_LOG( spEvent_->m_usEventID );
		}
		break;
	}

	dwElapTime = ::GetTickCount() - dwElapTime;
	if( dwElapTime > 3000 )
	{
		START_LOG_WITH_NAME( cwarn )
			<< BUILD_LOG( spEvent_->GetIDStr() )
			<< BUILD_LOG( spEvent_->m_usEventID )
			<< BUILD_LOG( dwElapTime );
	}

	//{{ 2011. 12. 14	������	��Ŷ ó�� ���� �ð� üũ
#ifdef SERV_PACKET_PROCESSING_LATENCY_CHECK
	SiKPacketMornitoring()->CheckPacketProcessingLatency( spEvent_->m_usEventID, GetCharUID(), dwElapTime );
#endif SERV_PACKET_PROCESSING_LATENCY_CHECK
	//}}
}

// ���� �߰� �Ǵ� �̺�Ʈ�� ���⿡ �߰��ϵ��� �Ѵ�.
bool KGSUser::ProcessEventExpand( const KEventPtr& spEvent_ )
{
	switch( spEvent_->m_usEventID )
	{
#pragma message( "--------------------------------------------" )
#pragma message( "Server Packet Event Filter!!!!! - ProcessEventExpand " )
#pragma message( "--------------------------------------------" )

#	include "GSUserPet.h"
#	include "GSUserGuild.h"
#	include "GSUserLetter.h"
#	include "GSUserAdminCheat.h"
#	include "GSUserMatch.h"
		//{{ 2011. 10. 27	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
#	include "GSUserBattleField.h"
#	include "GSUserAutoParty.h"
#endif SERV_BATTLE_FIELD_SYSTEM
		//}}

//#ifdef SERV_COUNTRY_CN
#	include "GSUserGiant.h"
//#endif SERV_COUNTRY_CN
		//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
#	include "GSUserRelationship.h"
#endif SERV_RELATIONSHIP_SYSTEM
		//}
		default:
			return false;
	}

	return true;
}

//{{ 2011. 04. 05  ��μ�	���� �� �ŷ� ����
#ifdef SERV_MONITORING_LETTER_AND_TRADE
void KGSUser::CheckLetterAndTradeMonitoring()
{
	if( SiKGameSysVal()->GetLetterAndTradeMonitoringIsOn() == false )
		return;

	if( m_kTimer[TM_MONITORING_LETTER].elapsed() > SiKGameSysVal()->GetLetterMonitoringCheckTime() )
	{
		m_kUserHackingManager.ClearLetterMonitoringList();
		m_kTimer[TM_MONITORING_LETTER].restart();
	}

	if( m_kTimer[TM_MONITORING_TRADE].elapsed() > SiKGameSysVal()->GetTradeMonitoringCheckTime() )
	{
		m_kUserHackingManager.ClearTradeMonitoringList();
		m_kTimer[TM_MONITORING_TRADE].restart();
	}	
}
#endif SERV_MONITORING_LETTER_AND_TRADE
//}}

//{{ 2011. 04. 26	������	deserialize ���� ����
#ifdef SERV_DESERIALIZE_FAIL_DISCONNECT
void KGSUser::CheckDeserializeFailCount()
{
	if( m_kTimer[TM_DESERIALIZE_FAIL].elapsed() < 300.0 )
		return;

	m_kTimer[TM_DESERIALIZE_FAIL].restart();

	// üũ ���� ������ ����!
	if( SiKGameSysVal()->IsDeserializeFailCheck() == false )
		return;
	
	// 5�и��� �ѹ��� üũ����!
    if( m_kUserHackingManager.HackingCheckDeserializeFailCount() == true )
	{
		if( GetAuthLevel() >= SEnum::UAL_DEVELOPER )
		{
			START_LOG( cout, L"�������ε� Deserialize Fail Hacking Check�� �ɷ������? �ڵ� �߸� ®��;;" )
#ifdef SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetUID() )
				<< BUILD_LOG( GetCharUID() );
#else
				<< BUILD_LOG( GetName() )
				<< BUILD_LOG( GetCharName() );
#endif SERV_PRIVACY_AGREEMENT
		}
		else
		{
			START_LOG( cout, L"[��ŷ����!] Deserialize Fail Hacking Check�� ������ üũ �Ǿ����ϴ�! ����͸� ������ ����ϰ� �ŷ����� �Ǿ����ϴ�!" )
#ifdef SERV_PRIVACY_AGREEMENT
				<< BUILD_LOG( GetUID() )
				<< BUILD_LOG( GetCharUID() );
#else
				<< BUILD_LOG( GetName() )
				<< BUILD_LOG( GetCharName() );
#endif SERV_PRIVACY_AGREEMENT
			//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
			HackUserRegMonitorsUser( KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_DESERIALIZE_FAIL );
			HackUserRegTradeBlock( KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_DESERIALIZE_FAIL );

			KEGS_SERVER_CHECK_HACK_USER_REQ kPacket;
			kPacket.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_DESERIALIZE_FAIL;			
			SendPacket( EGS_SERVER_CHECK_HACK_USER_REQ, kPacket );

			m_kUserHackingManager.SetHackingUserSendType( KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_DESERIALIZE_FAIL );
#else
			HackUserRegMonitorsUser( KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_DESERIALIZE_FAIL );
			HackUserRegTradeBlock( KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_DESERIALIZE_FAIL );

			KEGS_SERVER_CHECK_HACK_USER_NOT kPacket;
			kPacket.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_DESERIALIZE_FAIL;			
			SendPacket( EGS_SERVER_CHECK_HACK_USER_NOT, kPacket );
#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
			//}}
		}
	}
}
#endif SERV_DESERIALIZE_FAIL_DISCONNECT
//}}

//{{ 2012. 10. 15	������		ED&������ ����͸� �ý���
#ifdef SERV_ED_AND_ITEM_MORNITORING_SYSTEM
void KGSUser::CheckEDAndItemMornitoring()
{
	if( m_kTimer[TM_CHECK_ED_AND_ITEM_MORNITOING].elapsed() < 300.0 )
		return;

	m_kTimer[TM_CHECK_ED_AND_ITEM_MORNITOING].restart();

	// ������ ED ������ üũ�Ͽ� ���������� �ൿ�� �ϴ� ������ ã�Ƴ���!
	const std::map< int, __int64 >& mapEDStatistics = m_kEDManager.GetEDStatistics();
	m_kEDManager.ClearStatistics();


	// ������ ������ ������ üũ�Ͽ� ���������� �ൿ�� �ϴ� ������ ã�Ƴ���!
	const KItemQuantityVariation& kItemQuantity = m_kInventory.GetItemQuantityVariation();
	m_kInventory.ClearItemQuantityVariation();
}
#endif SERV_ED_AND_ITEM_MORNITORING_SYSTEM
//}}


//{{ 2012. 05. 31	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
void KGSUser::CheckAutoPartyBonus()
{
	if( GetStateID() > KGSFSM::S_SERVER_SELECT  &&  GetStateID() < KGSFSM::STATE_SENTINEL )
	{
		if( m_kTimer[TM_AUTO_PARTY_BONUS_CHECK].elapsed() < 5.0 ) // ������ ª�� üũ�� ������!
			return;

		m_kTimer[TM_AUTO_PARTY_BONUS_CHECK].restart();

		// ���� ���ʽ� ���� ��� ����!
		//KEGS_UPDATE_AUTO_PARTY_BONUS_INFO_NOT kPacket;
		//m_kUserDungeonManager.CheckAutoPartyBonusRefresh( kPacket.m_mapDungeonBonusList );

		//if( kPacket.m_mapDungeonBonusList.empty() == false )
		//{
		//	SendPacket( EGS_UPDATE_AUTO_PARTY_BONUS_INFO_NOT, kPacket );
		//}
	}
}
#endif SERV_BATTLE_FIELD_SYSTEM
//}}

//{{ 2011. 05. 02  ��μ�	2�� ���� �ý���
#ifdef SERV_SECOND_SECURITY
bool KGSUser::GetSuccessSecondSecurity()
{ 
	if( GetIsUseOTP() == false )
	{
		return m_bPassSecondSecurity;		
	}
	else
	{
		if( GetIsUseSecondPW() == true )
		{
			return m_bPassSecondSecurity;
		}
		else
		{
			return true;
		}
	}
}

bool KGSUser::IsSameSecondSecurityPW( IN const std::wstring& wstrSecondPW )
{
	if( m_wstrSecondPW.compare( wstrSecondPW ) == 0 )
	{
		return true;
	}

	START_LOG( clog, L"2�� ���� ��й�ȣ�� ���� �ʽ��ϴ�." )
		<< BUILD_LOG( GetUID() )
		<< BUILD_LOG( m_wstrSecondPW )
		<< BUILD_LOG( wstrSecondPW )
		<< END_LOG;

	return false;
}

bool KGSUser::CheckIntegritySecondSecurityPW( IN const std::wstring& wstrSecondPW, OUT int& iOK )
{
	iOK = NetError::NET_OK;

	// ��� �ְų�
	if( wstrSecondPW.empty() == true )	
	{
		iOK = NetError::ERR_SECOND_SECURITY_DEFECT_PW;
		return false;
	}

	// 4�ڸ� ���� �۰ų�
	if( wstrSecondPW.size() < 4 )		
	{
		iOK = NetError::ERR_SECOND_SECURITY_DEFECT_PW;
		return false;
	}

	// 6�ڸ� ���� ũ�ų�
	if( wstrSecondPW.size() > 6 )		
	{
		iOK = NetError::ERR_SECOND_SECURITY_DEFECT_PW;
		return false;
	}

	// ������ �����ΰ�
	{
		int iFirstNum = 0;
		int iNextNum = 0;
		int iCheck = 0;
		WCHAR _wchar;
        
		_wchar = wstrSecondPW[0];
		iFirstNum = _wchar - 48;				// '0' �� ���� 48 �̴�.

		for( unsigned int i = 0 ; i < wstrSecondPW.size() ; ++i )
		{
			_wchar = wstrSecondPW[i];
			iNextNum = _wchar - 48;
			if( iFirstNum == iNextNum )
			{
				++iCheck;
			}
		}

 		if( iCheck == wstrSecondPW.size() )
		{
			START_LOG( clog, L"������ ���ڸ� ��й�ȣ�� �����Ϸ��Ѵ�." )
				<< BUILD_LOG( GetName() )
				<< BUILD_LOG( wstrSecondPW );

			iOK = NetError::ERR_SECOND_SECURITY_DEFECT_PW_1;
			return false;
		}
	}
		
	// ���ӵ� �����ΰ�
	{
		int iFrontNum = 0;
		int iNextNum = 0;
		int iCheck = 1;			// ù�ڸ��� �����ϹǷ� �̸� �ϳ� ����, �񱳴� �ι�° ����
		WCHAR _wchar;
		
		_wchar = wstrSecondPW[0];
		iFrontNum = _wchar - 48;				// '0' �� ���� 48 �̴�.

		for( unsigned int i = 1 ; i < wstrSecondPW.size() ; ++i )	// ������ �˻�
		{
			_wchar = wstrSecondPW[i];
			iNextNum = _wchar - 48;
			
			if( iFrontNum + 1 == iNextNum )
			{
				++iCheck;
			}
			iFrontNum = iNextNum;
		}

		if( iCheck == wstrSecondPW.size() )
		{
			START_LOG( clog, L"���ӵ� ���ڸ� ��й�ȣ�� �����Ϸ��Ѵ�." )
				<< BUILD_LOG( GetName() )
				<< BUILD_LOG( wstrSecondPW );

			iOK = NetError::ERR_SECOND_SECURITY_DEFECT_PW_2;
			return false;
		}

		iFrontNum = 0;
		iNextNum = 0;
		iCheck = 1;			// ù�ڸ��� �����ϹǷ� �̸� �ϳ� ����, �񱳴� �ι�° ����
		
		_wchar = wstrSecondPW[0];
		iFrontNum = _wchar - 48;				// '0' �� ���� 48 �̴�.

		for( unsigned int i = 1 ; i < wstrSecondPW.size() ; ++i )	// ������ �˻�
		{
			_wchar = wstrSecondPW[i];
			iNextNum = _wchar - 48;

			if( iFrontNum - 1 == iNextNum )
			{
				++iCheck;
			}
			iFrontNum = iNextNum;
		}

		if( iCheck == wstrSecondPW.size() )
		{
			START_LOG( clog, L"���ӵ� ���ڸ� ��й�ȣ�� �����Ϸ��Ѵ�." )
				<< BUILD_LOG( GetName() )
				<< BUILD_LOG( wstrSecondPW );

			iOK = NetError::ERR_SECOND_SECURITY_DEFECT_PW_2;
			return false;
		}
	}

	// ���ڰ� �ƴ� ���ڰ� �ְų�
	int _count = (int)wstrSecondPW.size();		

	for( int i = 0 ; i < _count ; ++i )
	{
		if( '0' >  wstrSecondPW[i] && '9' <  wstrSecondPW[i] )
		{
			iOK = NetError::ERR_SECOND_SECURITY_DEFECT_PW;
			return false;
		}
	}					
    
	return true;
}

int KGSUser::GetSecurityType()
{
	// SecurityType ����
	// �̻��							: 0 
	// 2�� �����е� ���				: 1
	// �ڵ��� OTP ���					: 2
	// ���� PC ���						: 3
	// �ؽ� ��ƽ ���					: 4
	// 2�� ���� �е� + �ڵ��� OTP ���	: 5
	// 2�� ���� �е� + ���� PC ���		: 6
	// 2�� ���� �е� + �ؽ� ��ƽ ���	: 7

	int iNexonSecurityType = GetNexonSecurityType();	// 0 OTP �̻��, 1 �ڵ���OTP, 2 ����PC, 3 �ؽ���ƽ
	int iSecurityType = 0;

	if( GetIsUseSecondPW() == true )	// 2�� ���� ��� ��
	{
		switch( iNexonSecurityType )
		{
		case 0:
			{
				iSecurityType = 1;
			}break;
		case 1:
			{
				iSecurityType = 5;
			}break;
		case 2:
			{
				iSecurityType = 6;
			}break;
		case 3:
			{
				iSecurityType = 7;
			}break;
		default:
			{
				START_LOG( cerr, L"GetNexonSecurityType() �� ���� �̻��մϴ�." )
					<< BUILD_LOG( GetUID() )
					<< BUILD_LOG( iNexonSecurityType )
					<< BUILD_LOG( GetIsUseSecondPW() )
					<< END_LOG;
			}
		}
	}
	else		// 2�� ���� �̻�� ��
	{
		switch( iNexonSecurityType )
		{
		case 0:
			{
				iSecurityType = 0;
			}break;
		case 1:
			{
				iSecurityType = 2;
			}break;
		case 2:
			{
				iSecurityType = 3;
			}break;
		case 3:
			{
				iSecurityType = 4;
			}break;
		default:
			{
				START_LOG( cerr, L"GetNexonSecurityType() �� ���� �̻��մϴ�." )
					<< BUILD_LOG( GetUID() )
					<< BUILD_LOG( iNexonSecurityType )
					<< BUILD_LOG( GetIsUseSecondPW() )
					<< END_LOG;
			}
		}
	}

	return iSecurityType;
}

void KGSUser::SendSecurityType()
{
	KDBE_UPDATE_SECURITY_TYPE_NOT kNot;
	kNot.m_iUserUID = GetUID();
	kNot.m_iSecurityType = GetSecurityType();

	SendToAccountDB( DBE_UPDATE_SECURITY_TYPE_NOT, kNot );
}

void KGSUser::SendFirstLoginSecurityInfo()
{
	CTime tCurr = CTime::GetCurrentTime();
    
	KDBE_FIRST_WRITE_SECURITY_TYPE_NOT kNot;

	kNot.m_iUserUID = GetUID();
	kNot.m_iAge	= (int)m_kNexonAccountInfo.m_uiAge;
	kNot.m_iSecurityType = GetSecurityType();
	kNot.m_wstrCurrTime = tCurr.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
	
	SendToLogDB( DBE_FIRST_WRITE_SECURITY_TYPE_NOT, kNot );
}

void KGSUser::SetIsUseOTP( IN const int& iNexonSecurityType )
{
	if( iNexonSecurityType > 0 )	// 0 �� �̻�� , 1~3 �� OTP,����PC,�ؽ���ƽ
	{
		m_bIsUseOTP = true;
	}
	else
	{
		m_bIsUseOTP = false;
	}

	m_iNexonSecurityType = iNexonSecurityType;
}
#endif SERV_SECOND_SECURITY
//}}

//{{ 2011. 05. 27    ��μ�    �޸� ���� ���� ����
#ifdef SERV_COME_BACK_USER_REWARD
void KGSUser::CheckComeBackUser()
{
	if( m_kComeBackUserInfo.GetIsComeBackUser() == true )
	{
		CTime tComeBackBuffEnd = m_kComeBackUserInfo.GetComeBackBuffEnd();

		if( m_kComeBackUserInfo.CheckComeBackBuffEnd() == true )
		{
			// �޸� ���� ���� ���� DB�� ���
			// DB�� �������
			KDBE_WRITE_COME_BACK_END_NOT kNot;
			kNot = GetUID();
			SendToAccountDB( DBE_WRITE_COME_BACK_END_NOT, kNot );

			//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
			KELG_UPDATE_USER_UNIT_BUFF_INFO_REQ kBuffReq;
			kBuffReq.m_iUnitUID = GetCharUID();
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_RETURN_OF_HERO, kBuffReq.m_vecDeActivateBuff );
			if( kBuffReq.m_vecDeActivateBuff.empty() == false )
			{
				SendToLoginServer( ELG_UPDATE_USER_UNIT_BUFF_INFO_REQ, kBuffReq );
			}
#else
			// �޸� ���� ���� �ð� �˷��ֱ�
			KEGS_COME_BACK_BUFF_NOT kComeBack;
			kComeBack.m_bIsFinish = true;
			kComeBack.m_iComeBackBuffEnd = tComeBackBuffEnd.GetTime();
			SendPacket( EGS_COME_BACK_BUFF_NOT, kComeBack );
				return;
		}
		if( m_kTimer[TM_COME_BACK_USER].elapsed() > 60 ) // 60�� ���� ���� ���� �ð� �˷��ֱ�
		{
			m_kTimer[TM_COME_BACK_USER].restart();

			// �޸� ���� ���� �ð� �˷��ֱ�
			KEGS_COME_BACK_BUFF_NOT kComeBack;
			kComeBack.m_bIsFinish = false;
			kComeBack.m_iComeBackBuffEnd = tComeBackBuffEnd.GetTime();
			SendPacket( EGS_COME_BACK_BUFF_NOT, kComeBack );
#endif SERV_SERVER_BUFF_SYSTEM
			//}
		}
	}
}
void KGSUser::AnnunciateComeBackUser()
{
	KFriendInfo FriendUser;

	std::map< UidType, KFriendInfo >::iterator mit = m_kMessengerInfo.m_mapFriendInfo.begin();
	for(  ; mit != m_kMessengerInfo.m_mapFriendInfo.end() ; ++mit )
	{
		FriendUser = mit->second;
		if( FriendUser.m_iUnitUID > 0 )
		{
			KDBE_INSERT_REWARD_TO_POST_REQ kReq;
			kReq.m_iFromUnitUID		= FriendUser.m_iUnitUID;
			kReq.m_iToUnitUID		= FriendUser.m_iUnitUID;
			kReq.m_iRewardType		= KPostItemInfo::LT_COME_BACK_USER_NOTIFY;
			kReq.m_wstrMessage		= GetCharName();
			SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kReq );
		}
	}
}

void KGSUser::SendFirstLoginComeBackUser( IN KUnitInfo& kUnit, IN bool bFirst )
{
	CTime tCurr = CTime::GetCurrentTime();

	KDBE_FIRST_WRITE_COME_BACK_INFO_NOT kNot;

	kNot.m_iUserUID			= GetUID();
	kNot.m_iAge				= static_cast< int >( m_kNexonAccountInfo.m_uiAge );
	kNot.m_iUnitUID			= kUnit.m_nUnitUID;
	kNot.m_iUnitClass		= static_cast< int >( kUnit.m_cUnitClass );
	kNot.m_iUnitLevel		= static_cast< int >( kUnit.m_ucLevel );
	kNot.m_bTodayFirst		= bFirst;
	kNot.m_iRewardLevel		= static_cast< int >( m_kComeBackUserInfo.GetComeBackRewardLevel() );
	kNot.m_wsrtRegDate		= tCurr.Format( _T( "%Y-%m-%d %H:%M:%S" ) );

	SendToLogDB( DBE_FIRST_WRITE_COME_BACK_INFO_NOT, kNot );
}

void KGSUser::SendComeBackLogOut()
{
	CTime tCurr = CTime::GetCurrentTime();

	KDBE_WRITE_COME_BACK_LOGOUT_NOT kNot;
	kNot.m_iUserUID			= GetUID();
	kNot.m_wsrtLogOutDate	= tCurr.Format( _T( "%Y-%m-%d %H:%M:%S" ) );

	SendToAccountDB( DBE_WRITE_COME_BACK_LOGOUT_NOT, kNot );
}
#endif SERV_COME_BACK_USER_REWARD
//}} 

//{{ 2011. 08. 12   ��μ�      ��ϸ� ���� 
#ifdef SERV_NEW_HENIR_TEST
void KGSUser::CheckHenirRewardCountEvent()
{
	if( GetStateID() > KGSFSM::S_SERVER_SELECT  &&  GetStateID() < KGSFSM::STATE_SENTINEL )
	{
		if( m_kTimer[TM_HENIR_REWARD_EVENT].elapsed() < 120.0 )
			return;

		m_kTimer[TM_HENIR_REWARD_EVENT].restart();

		bool bUnlimited = false;
		int  iEventMax = 0;
		KEGS_HENIR_REWARD_COUNT_NOT	kNot;

		SiKGameEventManager()->GetHenirRewardCountEvent( bUnlimited, iEventMax );

		m_kUserDungeonManager.GetHenirRewardCountInfo( kNot );

		m_kUserDungeonManager.SetUnLimitedHenirRewardEvnet( bUnlimited );
		m_kUserDungeonManager.SetHenirRewardEvnetLimitCount( iEventMax );

		if( kNot.m_bUnLimited != bUnlimited || kNot.m_iEventMAX != iEventMax )
		{
			m_kUserDungeonManager.GetHenirRewardCountInfo( kNot );
			SendPacket( EGS_HENIR_REWARD_COUNT_NOT, kNot );
		}
	}
}
#endif SERV_NEW_HENIR_TEST
//}}

//{{ 2011. 09. 14  ��μ�	��ŷ ���μ��� ��� ���� - ���� �ε� ���� �ܰ�
#ifdef SERV_DLL_LIST_CHECK_BEFOR_LOADING
void KGSUser::SendRefreshChannalRandomKey()
{
	KDBE_UPDATE_CHANNEL_RANDOMKEY_ACK kPacket;
	
	kPacket.m_iRandomKey = rand();
	kPacket.m_iRandomKey = kPacket.m_iRandomKey + ( rand() << 15) + 1;  // 0 �ʰ��ϴ� �� ����

	kPacket.m_wstrUserID = GetUserID();
	
	SendToAccountDB( DBE_UPDATE_CHANNEL_RANDOMKEY_NOT, kPacket );
}
#endif SERV_DLL_LIST_CHECK_BEFOR_LOADING
//}}

//{{ 2012. 06. 13	��μ�       ������ �˴ٿ���
#ifdef SERV_SELECTIVE_SHUTDOWN_SYSTEM
bool KGSUser::IsRestrictionUser( IN std::map< unsigned short, KPropertyValue >& mapProperty, IN BYTE bytePolicyResultNo )
{
	bool bRet = false;
	KPropertyValue kProperty;

	std::map< unsigned short, KPropertyValue >::iterator mit = mapProperty.find( 19 ); // �ؽ� ��å ���
	if( mit == mapProperty.end() )
	{
		START_LOG( clog, L"�ؽ� ��å ��� �� 19���� �ش��ϴ� ���� ����!" )
			<< BUILD_LOG( GetUserID() ) 
			<< BUILD_LOG( GetUID() ) 
			<< BUILD_LOG( static_cast<int>(bytePolicyResultNo) ) 
			<< END_LOG;

		return bRet;
	}

	kProperty = mit->second;

	std::map< BYTE, KRolicyResults >::iterator mitProperty = mit->second.m_mapRolicyResult.find( bytePolicyResultNo );
	if( mitProperty == mit->second.m_mapRolicyResult.end() )
	{
		START_LOG( clog, L"�ؽ� ��å ��� �� 19���� ���� �̿� ���ѿ� �ش��ϴ� ���� ����!" )
			<< BUILD_LOG( GetUserID() ) 
			<< BUILD_LOG( GetUID() ) 
			<< BUILD_LOG( static_cast<int>(bytePolicyResultNo) ) 
			<< END_LOG;
		return bRet;
	}

	KRolicyResults kResults = mitProperty->second;
	if( kResults.m_byteRolicyListNo == bytePolicyResultNo )
	{
		if( kResults.m_byteRolicyResult > 0  )
		{
			m_kUserInfoByNexon.SetRestrictionUser( KUserInfoByNexon::SHUTDOWN_USER );
			bRet = true;
		}
		else
		{
			// ���� �̿� ���� ��
			m_kUserInfoByNexon.SetRestrictionUser( KUserInfoByNexon::NOT_SHUTDOWN_USER );
		}

		int iError = KUserInfoByNexon::NONE_AUTH_ERROR;
		std::wstring wstrTime;

		bool bExistTime = false;
		if( SetRestrictionUserMSG( kResults.m_wstrOption, iError, wstrTime, bExistTime ) == true )
		{
			m_kUserInfoByNexon.SetRestrictionUserMSG( iError );
			m_kUserInfoByNexon.SetRestrictionUserTime( wstrTime );
			m_kUserInfoByNexon.SetExistRestrictionTime( bExistTime );

			START_LOG( clog, L"�˴ٿ� ��å�� ����� ����! ������ �� �� ���� ����" )
				<< BUILD_LOG( kResults.m_wstrOption ) 
				<< END_LOG;
		}

		return bRet;
	}

	return bRet;
}

bool KGSUser::SetRestrictionUserMSG( IN std::wstring& wstrOption, OUT int& iError, OUT std::wstring& wstrTime, OUT bool& bExistTime )
{
	//////////////////////////////////////////////////////////////////////////
	// ��Ʈ�� ����
	// time -> YYMMDDHH
	// error - ����

	std::wstring wstrKeyTime = L"time=";
	std::wstring wstrKeyError = L"error=";

	std::wstring::size_type iIndex;

	iIndex = wstrOption.find( wstrKeyTime );
	if( iIndex != std::string::npos ) // ã��
	{
		bExistTime = true;

		std::wstring wstrYY = wstrOption.substr( iIndex + wstrKeyTime.size(), 2 );
		std::wstring wstrMM = wstrOption.substr( iIndex + wstrKeyTime.size() + 2, 2 );
		std::wstring wstrDD = wstrOption.substr( iIndex + wstrKeyTime.size() + 4, 2 );
		std::wstring wstrHH = wstrOption.substr( iIndex + wstrKeyTime.size() + 6, 2 );

		wstrTime = L"20" + wstrYY + L"-" + wstrMM + L"-" + wstrDD + L" " + wstrHH + L":00:00";

		START_LOG( clog, L"���� ���� �ð� Ȯ��." )
			<< BUILD_LOG( GetUID() )
			<< BUILD_LOG( wstrOption )
			<< BUILD_LOG( wstrTime )
			<< END_LOG;
	}
	else	// ��ã��
	{
		START_LOG( clog, L"���� ���� �ð��� ã�� �� ����." )
			<< BUILD_LOG( GetUID() )
			<< BUILD_LOG( wstrOption )
			<< END_LOG;

        bExistTime = false;		
	}

	iIndex = wstrOption.find( wstrKeyError );
	if( iIndex != std::string::npos ) // ã��
	{
		std::wstring wstrError = wstrOption.substr( iIndex + wstrKeyError.size(), 2 );
		std::string strError = KncUtil::toNarrowString( wstrError );
		iError = atoi( strError.c_str() );

		START_LOG( clog, L"���� ���� ���� Ȯ��." )
			<< BUILD_LOG( GetUID() )
			<< BUILD_LOG( wstrOption )
			<< BUILD_LOG( wstrError )
			<< BUILD_LOG( iError )
			<< END_LOG;

	}
	else	// ��ã��
	{
		START_LOG( clog, L"���� ���� ������ ã�� �� ����." )
			<< BUILD_LOG( GetUID() )
			<< BUILD_LOG( wstrOption )
			<< END_LOG;
	}
	return true;
}
#else
//{{ 2011. 09. 29  ��μ�	�˴ٿ��� ����
#ifdef SERV_SHUTDOWN_SYSTEM
bool KGSUser::CheckShutdownUser( IN std::map< unsigned short, KPropertyValue >& mapProperty, IN BYTE bytePolicyResultNo )
{
	KPropertyValue kProperty;

	std::map< unsigned short, KPropertyValue >::iterator mit = mapProperty.find( 19 ); // �ؽ� ��å ���
	if( mit == mapProperty.end() )
	{
		START_LOG( clog, L"�ؽ� ��å ��� �� 19���� �ش��ϴ� ���� ����!" )
			<< BUILD_LOG( GetUserID() ) 
			<< BUILD_LOG( GetUID() ) 
			<< BUILD_LOG( static_cast<int>(bytePolicyResultNo) ) 
			<< END_LOG;

		return false;
	}

	kProperty = mit->second;

	std::map< BYTE, KRolicyResults >::iterator mitProperty = mit->second.m_mapRolicyResult.find( bytePolicyResultNo );
	if( mitProperty == mit->second.m_mapRolicyResult.end() )
	{
		START_LOG( clog, L"�ؽ� ��å ��� �� 19���� �˴ٿ �ش��ϴ� ���� ����!" )
			<< BUILD_LOG( GetUserID() ) 
			<< BUILD_LOG( GetUID() ) 
			<< BUILD_LOG( static_cast<int>(bytePolicyResultNo) ) 
			<< END_LOG;
		return false;
	}

	KRolicyResults kResults = mitProperty->second;
	if( kResults.m_byteRolicyListNo == bytePolicyResultNo )
	{
		if( kResults.m_byteRolicyResult > 0  )
		{
			m_kUserInfoByNexon.SetShutdownUser( KUserInfoByNexon::SHUTDOWN_USER );
		}
		else
		{
			m_kUserInfoByNexon.SetShutdownUser( KUserInfoByNexon::NOT_SHUTDOWN_USER );
		}


		if( CheckShutdownUserMSG( kResults.m_wstrOption ) == true )
		{
			m_kUserInfoByNexon.SetShutdownUserMSG( true );
		}

		START_LOG( clog, L"�˴ٿ� ��å�� ����� ����! ������ �� �� ���� ����" )
			<< BUILD_LOG( kResults.m_wstrOption ) 
			<< END_LOG;

		START_LOG( clog, L"�˴ٿ� ��å�� ����� ����! ������ �� �� ���� ����" )
			<< BUILD_LOG( GetUserID() ) 
			<< BUILD_LOG( GetUID() ) 
			<< BUILD_LOG( static_cast<int>(bytePolicyResultNo) ) 
			<< END_LOG;

		return true;
	}

	return false;
}

bool KGSUser::CheckOutUserRetaining( IN std::map< unsigned short, KPropertyValue >& mapProperty, IN BYTE bytePolicyResultNo, OUT int& iOutUserRetaining  )
{
	KPropertyValue kProperty;
	iOutUserRetaining = 0;

	std::map< unsigned short, KPropertyValue >::iterator mit = mapProperty.find( 19 ); // �ؽ� ��å ���
	if( mit == mapProperty.end() )
	{
		START_LOG( clog, L"�ؽ� ��å ��� �� 19���� �ش��ϴ� ���� ����!" )
			<< BUILD_LOG( GetUserID() ) 
			<< BUILD_LOG( GetUID() ) 
			<< BUILD_LOG( static_cast<int>(bytePolicyResultNo) ) 
			<< END_LOG;

		return false;
	}

	kProperty = mit->second;

	std::map< BYTE, KRolicyResults >::iterator mitProperty = mit->second.m_mapRolicyResult.find( bytePolicyResultNo );
	if( mitProperty == mit->second.m_mapRolicyResult.end() )
	{
		START_LOG( clog, L"�ؽ� ��å ��� �� 19���� ��Ż������ �ش��ϴ� ���� ����!" )
			<< BUILD_LOG( GetUserID() ) 
			<< BUILD_LOG( GetUID() ) 
			<< BUILD_LOG( static_cast<int>(bytePolicyResultNo) ) 
			<< END_LOG;
		return false;
	}

	KRolicyResults kResults = mitProperty->second;
	if( kResults.m_byteRolicyListNo > 0 && kResults.m_byteRolicyResult > 0 )
	{
		iOutUserRetaining = static_cast<int>(kResults.m_byteRolicyResult);

		START_LOG( clog, L"��Ż���� ��å�� ����� ����!" )
			<< BUILD_LOG( GetUserID() ) 
			<< BUILD_LOG( GetUID() ) 
			<< BUILD_LOG( static_cast<int>(bytePolicyResultNo) ) 
			<< BUILD_LOG( iOutUserRetaining )
			<< END_LOG;

		return true;
	}

	return false;
}

bool KGSUser::CheckShutdownUserMSG( IN std::wstring& wstrOption )
{
	std::wstring wstrFind_f = L"&";
	std::wstring wstrFind_s = L"~";  // time= �� ���� ������ ~ �� ���� �Ѵ�.
	size_t stFound_f = 0;
	size_t stFound_s = 0;

	stFound_f = wstrOption.find( wstrFind_f );  // time= &
	if ( stFound_f != std::wstring::npos )
	{
		stFound_s = wstrOption.find( wstrFind_s );

		if ( stFound_s != std::wstring::npos )
		{
			if( stFound_f > stFound_s )
			{
				return true;
			}
		}
	}
	return false;
}
#endif SERV_SHUTDOWN_SYSTEM
//}}

//{{ 2012. 02. 14    ��μ�   �ؽ� �Ƚ� �α��� - ��ǥ ��ġ��� �α���
#ifdef SERV_NEXON_PEACE_OF_MIND_LOGIN
bool KGSUser::CheckPeaceOfMindLoginUser( IN std::map< unsigned short, KPropertyValue >& mapProperty, IN BYTE bytePolicyResultNo, OUT int& iResult )
{
	KPropertyValue kProperty;
	iResult = 0;

	std::map< unsigned short, KPropertyValue >::iterator mit = mapProperty.find( 19 ); // �ؽ� ��å ���
	if( mit == mapProperty.end() )
	{
		START_LOG( clog, L"�ؽ� ��å ��� �� 19���� �ش��ϴ� ���� ����!" )
			<< BUILD_LOG( GetUserID() ) 
			<< BUILD_LOG( GetUID() ) 
			<< BUILD_LOG( static_cast<int>(bytePolicyResultNo) ) 
			<< END_LOG;

		return false;
	}

	kProperty = mit->second;

	std::map< BYTE, KRolicyResults >::iterator mitProperty = mit->second.m_mapRolicyResult.find( bytePolicyResultNo );
	if( mitProperty == mit->second.m_mapRolicyResult.end() )
	{
		START_LOG( clog, L"�ؽ� ��å ��� �� 19���� �Ƚɷα��ο� �ش��ϴ� ���� ����!" )
			<< BUILD_LOG( GetUserID() ) 
			<< BUILD_LOG( GetUID() ) 
			<< BUILD_LOG( static_cast<int>(bytePolicyResultNo) ) 
			<< END_LOG;
		return false;
	}

	KRolicyResults kResults = mitProperty->second;
	if( kResults.m_byteRolicyListNo > 0 && kResults.m_byteRolicyResult > 0 )
	{
		iResult = static_cast<int>(kResults.m_byteRolicyResult);

		START_LOG( clog, L"�Ƚɷα����� ����� ����!" )
			<< BUILD_LOG( GetUserID() ) 
			<< BUILD_LOG( GetUID() ) 
			<< BUILD_LOG( static_cast<int>(bytePolicyResultNo) ) 
			<< BUILD_LOG( iResult )
			<< END_LOG;

		return true;
	}

	return false;
}
#endif SERV_NEXON_PEACE_OF_MIND_LOGIN
//}}
#endif SERV_SELECTIVE_SHUTDOWN_SYSTEM
//}}

//{{ 2011. 08. 29	������	ĳ���� ������ �̺�Ʈ
#ifdef SERV_CHAR_LEVEL_UP_EVENT
bool KGSUser::GetLevelUpRewardItem( IN u_char ucNowLevel, IN u_char ucLevel, OUT std::vector< int >& vecLevelUpRewardID )
{
	if( ucNowLevel >= ucLevel )
		return false;


	//{{ 2012. 02. 03	�ڼ���	�̺�Ʈ �������� ó����� ���� ( Script -> DB )
#ifdef SERV_CHANGE_EVENT_INFO_SCRIPT_TO_DB
	if( SiKEventRewardLevelUpRefreshManager()->GetLevelUpRewardItem( ucLevel, vecLevelUpRewardID ) == false )
#else
	if( SiKRewardTable()->GetLevelUpRewardItem( ucLevel, vecLevelUpRewardID ) == false )
#endif SERV_CHANGE_EVENT_INFO_SCRIPT_TO_DB
	//}}
	{
		START_LOG( clog, L"�ش� �������� ���� �Ǵ� �̺�Ʈ �������� �����ϴ�." )
			<< BUILD_LOG( GetUserID() )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOGc( ucNowLevel )
			<< BUILD_LOGc( ucLevel );

		return false;
	}

	return true;
}
#endif SERV_CHAR_LEVEL_UP_EVENT
//}}

//{{ 2011.10.14     ��μ�    ��� ����� Ư�� IP ������ ��� ����
#ifdef SERV_USE_GM_CHEAT_RESTRICTED_IP
bool KGSUser::CheckAuthLevel()
{
	if( SiKCompanyIPTable()->CheckIP( GetIPStr() ) == true )
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// iP�� ��� ���� ���� ������ ġƮ ���
	// userid
	// ip
	// sms ����
	if( KSimLayer::GetKObj()->GetAuthFlag() == KSimLayer::AF_INTERNAL )
	{
		return false;	
	}

	// ���񽺿����� üũ
	if( KSimLayer::GetKObj()->GetVersionFlag() != KSimLayer::VF_SERVICE )
	{
		return false;	
	}

	KDBE_SEND_PHONE_MSG_NOT kPacketNot;

	//{{ 2012. 10. 9	�ڼ���	SMS ��ȭ��ȣ ���� ����
#ifdef SERV_SMS_TOTAL_MANAGER
	SiKSMSPhoneNumberManager()->GetPhoneNumberList( KSMSPhoneNumberManager::FS_CHECK_AUTH_LEVEL, kPacketNot.m_vecPhoneNum );
#else
	kPacketNot.m_vecPhoneNum.push_back( std::wstring( L"010-8584-3123" ) );		// �ǿ��� �����
	kPacketNot.m_vecPhoneNum.push_back( std::wstring( L"010-9023-1232" ) );		// ����ö
	kPacketNot.m_vecPhoneNum.push_back( std::wstring( L"010-8421-1075" ) );		// ������
	kPacketNot.m_vecPhoneNum.push_back( std::wstring( L"010-5598-4163" ) );		// ��μ�
	kPacketNot.m_vecPhoneNum.push_back( std::wstring( L"010-3323-7753" ) );		// �ڼ���
	//kPacketNot.m_vecPhoneNum.push_back( std::wstring( L"010-9490-8761" ) );		// Ȳ����
#endif SERV_SMS_TOTAL_MANAGER
	//}}
	//{{ 2013. 01. 17	������	������ ��� �缳IP ����
#ifdef SERV_PRIVATE_IP_SERVER_NETWORKING
	kPacketNot.m_wstrSMSMessage = boost::str( boost::wformat( L"���ȵ�IP���� ġƮ��� : UserUID(%d), IP(%s), ServerIP(%s)" ) % GetUID() % GetIPStr() % KBaseServer::GetKObj()->GetPublicIP() );
#else // SERV_PRIVATE_IP_SERVER_NETWORKING
	kPacketNot.m_wstrSMSMessage = boost::str( boost::wformat( L"���ȵ�IP���� ġƮ��� : UserUID(%d), IP(%s)" ) % GetUID() % GetIPStr() );
#endif // SERV_PRIVATE_IP_SERVER_NETWORKING
	SendToSMSDB( DBE_SEND_PHONE_MSG_NOT, kPacketNot );
	//////////////////////////////////////////////////////////////////////////
	
	return false;
}
#endif SERV_USE_GM_CHEAT_RESTRICTED_IP
//}}

//{{ 2011. 11. 21  ��μ�	���� ���� ������
#ifdef SERV_UNIT_CLASS_CHANGE_ITEM
bool KGSUser::GetUnitClassChangeInfo( IN int iItemID, OUT KDBE_BUY_UNIT_CLASS_CHANGE_REQ& kPacketToDB )
{
	int iNewUnitClass = CXSLItem::GetClassChangeCashItem( iItemID ); // ���� �����Ϸ��� ����
	char cItemUnitClass = static_cast<char>(iNewUnitClass);			// ����ó���� �׷� ���̵� �˻� �� ( 2�� -> 1��)
	
	if( cItemUnitClass == CXSLUnit::UC_NONE )
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// ��ų �ʱ�ȭ ����
#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013 // ���볯¥: 2013-06-27
	if( !SiCXSLSkillTree()->GetUnitClassDefaultSkill( iNewUnitClass, kPacketToDB.m_iDefaultSkillID1, kPacketToDB.m_iDefaultSkillID2, kPacketToDB.m_iDefaultSkillID3, 
														kPacketToDB.m_iDefaultSkillID4, kPacketToDB.m_iDefaultSkillID5, kPacketToDB.m_iDefaultSkillID6 ) )
	{
		START_LOG( cerr, L"����Ÿ���� �̻���." )
			<< BUILD_LOG( GetCharName() )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOG( cItemUnitClass )
			<< END_LOG;

		return false;
	}

	// ��� ��ų�� ���ٸ�..
	int iUsedSPoint = 0;
	int iUsedCSPoint = 0;
	int iTotalSP = 0;
	SiCXSLSkillTree()->GetCalcInitSkillPoint( GetLevel(), iTotalSP );


	int iRetrievedSPoint = 0;	// �������� SP
	int iRetrievedCSPoint = 0;	// �������� CSP

	if( false == m_kSkillTree.IsCashSkillPointExpired() )
	{
		iRetrievedSPoint	= iTotalSP;
		iRetrievedCSPoint	= m_kSkillTree.GetMaxCSPoint();
	}
	else
	{
		if( 0 == iUsedCSPoint && 0 == m_kSkillTree.GetCSPoint() )
		{
			iRetrievedSPoint	= iTotalSP;
			iRetrievedCSPoint	= 0;
		}
		else
		{
			START_LOG( cerr, L"cash skill point������ �ٵǾ��µ� ��ųƮ���� csp�� �����ִ�." )
				<< BUILD_LOG( GetCharName() )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOG( iUsedSPoint )
				<< BUILD_LOG( iUsedCSPoint )
				<< BUILD_LOG( m_kSkillTree.GetCSPoint() )
				<< END_LOG;

			return false;
		}
	}

	kPacketToDB.m_iUnitUID		 = GetCharUID();
	kPacketToDB.m_iSPoint		 = iRetrievedSPoint;
	kPacketToDB.m_iCSPoint		 = iRetrievedCSPoint;

	kPacketToDB.m_iNewUnitClass = static_cast<int>(cItemUnitClass);
	kPacketToDB.m_iUnitUID = GetCharUID();

	m_iSPoint.SetValue( iRetrievedSPoint );
	m_kSkillTree.SetCSPoint( iRetrievedCSPoint );
#else	// SERV_UPGRADE_SKILL_SYSTEM_2013
/*
	if( !SiCXSLSkillTree()->GetUnitTypeDefaultSkill( GetUnitType(), kPacketToDB.m_iDefaultSkillID, kPacketToDB.m_iDefaultSkillID2 ) )
	{
		START_LOG( cerr, L"����Ÿ���� �̻���." )
			<< BUILD_LOG( GetCharName() )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOG( cItemUnitClass )
			<< END_LOG;

		return false;
	}

	// ��� ��ų�� ���ٸ�..
	int iUsedSPoint = 0;
	int iUsedCSPoint = 0;
	m_kSkillTree.CalcUsedSPointAndCSPoint( iUsedSPoint, iUsedCSPoint );
	if( (iUsedSPoint <= 2 && iUsedCSPoint <= 0 ) ||
		(iUsedSPoint + iUsedCSPoint <= 0) )
	{
		START_LOG( cwarn, L"��ų�� �ϳ��� ����� �ʾҰų� �⺻ ��ų�� �ִ� �����ε�( ��, ���̻� �ʱ�ȭ �� �ʿ䰡 ���µ�) �ʱ�ȭ �Ϸ��� ��." )
			<< BUILD_LOG( GetCharName() )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOG( iUsedSPoint )
			<< BUILD_LOG( iUsedCSPoint )
			<< END_LOG;
	}


	const int DEFAULT_SKILL_SP = 2;		// default skill 2�� 1������ �����ϰ� �������� SP�� ����ؾ��Ѵ�
	int iRetrievedSPoint = 0 ;	// �������� SP
	int iRetrievedCSPoint = 0;	// �������� CSP


	if( false == m_kSkillTree.IsCashSkillPointExpired() )
	{
		iRetrievedSPoint	= iUsedSPoint + iUsedCSPoint + m_kSkillTree.GetCSPoint() - m_kSkillTree.GetMaxCSPoint() - DEFAULT_SKILL_SP;
		iRetrievedCSPoint	= m_kSkillTree.GetMaxCSPoint();
	}
	else
	{
		if( 0 == iUsedCSPoint && 0 == m_kSkillTree.GetCSPoint() )
		{
			iRetrievedSPoint	= iUsedSPoint - DEFAULT_SKILL_SP;
			iRetrievedCSPoint	= 0;
		}
		else
		{
			START_LOG( cerr, L"cash skill point������ �ٵǾ��µ� ��ųƮ���� csp�� �����ִ�." )
				<< BUILD_LOG( GetCharName() )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOG( iUsedSPoint )
				<< BUILD_LOG( iUsedCSPoint )
				<< BUILD_LOG( m_kSkillTree.GetCSPoint() )
				<< END_LOG;

			return false;
		}
	}

	kPacketToDB.m_iUnitUID		 = GetCharUID();
	kPacketToDB.m_iSPoint		 = m_iSPoint + iRetrievedSPoint;
	kPacketToDB.m_iCSPoint		 = iRetrievedCSPoint;

	kPacketToDB.m_iNewUnitClass = static_cast<int>(cItemUnitClass);
	kPacketToDB.m_iUnitUID = GetCharUID();
*/
#endif	// SERV_UPGRADE_SKILL_SYSTEM_2013


	//////////////////////////////////////////////////////////////////////////
	// ��ų
	std::set< int > setUnSealedSkillID;
	setUnSealedSkillID.clear();
	m_kSkillTree.GetUnSealedSkillList( setUnSealedSkillID );
	if( setUnSealedSkillID.size() > 0 )		// ���� ������ ��ų�� �����Ѵٸ� ��ų ��ȯ ��Ͽ� �߰�
	{
		std::set< int >::iterator sit = setUnSealedSkillID.begin();
		if( sit == setUnSealedSkillID.end() )
		{
			START_LOG( cerr, L"���� ���� ������ ��� - ���� ������ ��ų�� �����Ѵٰ� �ߴµ�...�����Ͱ� ����!" )
				<< BUILD_LOG( GetUserID() )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOGc( GetUnitClass() )
				<< BUILD_LOG( iItemID )
				<< BUILD_LOG( kPacketToDB.m_iNewUnitClass )
				<<END_LOG;
		}
		else
		{
			int iNewSkill = 0;
			for( ; sit != setUnSealedSkillID.end() ; ++sit )
			{
				//{{ Iruha : 2026-08-27 // VS2010 port: VC7.1's std::set::iterator
				// non-conformantly dereferenced to a mutable reference; VC10 dereferences to
				// const, and GetClassChange_NewSkill takes int& (non-const). Copy into a local.
				int iSkillID = *sit;
				if( SiKClassChangeTable()->GetClassChange_NewSkill( iNewUnitClass, iSkillID, iNewSkill ) == true )
				{
					kPacketToDB.m_mapChangeSkill.insert( std::make_pair(iSkillID, iNewSkill) );
				}
				//}}
				iNewSkill = 0;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// �޸�
	std::map< char, int >	mapSkillNote;
	mapSkillNote.clear();
	m_kSkillTree.GetMemoList( mapSkillNote );
	if( mapSkillNote.size() > 0 )		// ���� ������ ��ų�� �����Ѵٸ� ��ų ��ȯ ��Ͽ� �߰�
	{
		std::map< char, int >::iterator mit = mapSkillNote.begin();
		if( mit == mapSkillNote.end() )
		{
			START_LOG( cerr, L"���� ���� ������ ��� - ������ �޸� �����Ѵٰ� �ߴµ�...�����Ͱ� ����!" )
				<< BUILD_LOG( GetUserID() )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOGc( GetUnitClass() )
				<< BUILD_LOG( iItemID )
				<< BUILD_LOG( kPacketToDB.m_iNewUnitClass )
				<<END_LOG;
		}
		else
		{
			int iNewMemo = 0;
			for( ; mit != mapSkillNote.end() ; ++mit )
			{
				if( SiKClassChangeTable()->GetClassChange_NewMemo( iNewUnitClass, mit->second, iNewMemo ) == true )
				{
					kPacketToDB.m_mapChangeMemo.insert( std::make_pair(mit->second, iNewMemo) );
				}
				iNewMemo = 0;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// ������
	std::map< UidType, KInventoryItemInfo > mapInventoryItem;
	std::map< UidType, KInventoryItemInfo >::iterator mitInventoryItem;

	// Ư�� ���� ������ ������ ��� ����(ĳ������ �������� Ư�� �ǿ� �ִ�.)
	m_kInventory.GetInventoryInfo( CXSLInventory::ST_SPECIAL, mapInventoryItem, true );
	// ������ ������ ������ ��� ����
	m_kInventory.GetInventoryInfo( CXSLInventory::ST_BANK, mapInventoryItem, true );
	// �� ���� ������ ������ ��� ����
	m_kInventory.GetInventoryInfo( CXSLInventory::ST_PET, mapInventoryItem, true );

	mitInventoryItem = mapInventoryItem.begin();
	if( mitInventoryItem == mapInventoryItem.end() )	// �������� �����Ѵٸ� ��ȯ ������ ������ ���� Ȯ������
	{
		START_LOG( clog, L"���� ���� ������ ��� - Inventory Item �����Ͱ� ����!" )
			<< BUILD_LOG( GetUserID() )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOGc( GetUnitClass() )
			<< BUILD_LOG( iItemID )
			<< BUILD_LOG( kPacketToDB.m_iNewUnitClass )
			<<END_LOG;
	}
	else
	{
		KInventoryItemInfo kTempItemInfo;
		int	iNewItem = 0;
		for( ; mitInventoryItem != mapInventoryItem.end() ; ++mitInventoryItem )
		{
			kTempItemInfo = mitInventoryItem->second;

			if( SiKClassChangeTable()->GetClassChange_NewItem( iNewUnitClass, kTempItemInfo.m_kItemInfo.m_iItemID, iNewItem ) == false )
			{
				continue;
			}

			kPacketToDB.m_mapChangeItem.insert( std::make_pair( mitInventoryItem->first, iNewItem ) );
			iNewItem = 0;
		}
	}
	
	//////////////////////////////////////////////////////////////////////////
	// ����Ʈ
	// ( �Ϸ�� )
	// �Ϸ� ����Ʈ ���� �� ��� ã��
	{
		std::vector< int > vecCompleteQuest;
		m_kUserQuestManager.GetCompleteChangeClassQuest( vecCompleteQuest );

		std::vector< int >::const_iterator vit = vecCompleteQuest.begin();

		int	iOldQuestID = 0;
		int	iNewQuestID = 0;

		for( ; vit != vecCompleteQuest.end() ; ++vit )
		{
			iOldQuestID = (*vit);
			if( SiKClassChangeTable()->GetClassChange_NewQuest( iNewUnitClass, iOldQuestID, iNewQuestID ) == false )
			{
				continue;
			}
			
			bool bInsert = true;
			
			for( u_short usSize = 0 ; usSize < kPacketToDB.m_vecDeleteCompleteQuest.size() ; ++usSize )
			{
				if( iOldQuestID == kPacketToDB.m_vecDeleteCompleteQuest[usSize] )
				{
					bInsert = false;
					break;
				}
			}

			if( bInsert == true )
			{
				kPacketToDB.m_mapChangeCompleteQuest.insert( std::make_pair( iOldQuestID, iNewQuestID ) );
			}

			iOldQuestID = 0;
			iNewQuestID = 0;
		}
	}

	// ����Ʈ
	// ( ������ )
	{
		std::vector< int > vecInProgressQuest;
		m_kUserQuestManager.GetInProgressClassChangeQuest( vecInProgressQuest );

		std::vector< int >::const_iterator vit = vecInProgressQuest.begin();

		int	iOldQuestID = 0;
		int	iNewQuestID = 0;

		for( ; vit != vecInProgressQuest.end() ; ++vit )
		{
			iOldQuestID = (*vit);
			if( SiKClassChangeTable()->GetClassChange_NewQuest( iNewUnitClass, iOldQuestID, iNewQuestID ) == false )
			{
				continue;
			}

			bool bInsert = true;

			for( u_short usSize = 0 ; usSize < kPacketToDB.m_vecDeleteInProgressQuest.size() ; ++usSize )
			{
				if( iOldQuestID == kPacketToDB.m_vecDeleteInProgressQuest[usSize] )
				{
					bInsert = false;
					break;
				}
			}
	
			if( bInsert == true )
			{
				kPacketToDB.m_mapChangeInProgressQuest.insert( std::make_pair( iOldQuestID, iNewQuestID ) );
			}
			
			iOldQuestID = 0;
			iNewQuestID = 0;
		}
	}
	return true;
}
#endif SERV_UNIT_CLASS_CHANGE_ITEM
//}}


#ifdef SERV_UNLIMITED_SECOND_CHANGE_JOB
bool KGSUser::GetUnlimitedClassChangeInfo( IN int iNewClassID, OUT KDBE_BUY_UNIT_CLASS_CHANGE_REQ& kPacketToDB )
{
	char cItemUnitClass = static_cast<char>( iNewClassID );		// ����ó���� �׷� ���̵� �˻� �� ( 2�� -> 1��)
	bool bValidSecondChangeJob = CXSLUnit::IsSecondChangeJob( static_cast<CXSLUnit::UNIT_CLASS>(GetUnitClass()) );
	CXSLUnit::UNIT_TYPE eOldUnitType = CXSLUnit::GetUnitClassToUnitType( static_cast<CXSLUnit::UNIT_CLASS>( GetUnitClass() ) );
	CXSLUnit::UNIT_TYPE eNewUnitType = CXSLUnit::GetUnitClassToUnitType( static_cast<CXSLUnit::UNIT_CLASS>( iNewClassID ) );

	kPacketToDB.m_bUnlimitedSecondJobItem = true;						// ������ ��������� ��� ����. ���� ���޽� Ȯ���ϱ� ���� �뵵
	kPacketToDB.m_iOldUnitClass = static_cast<int>( GetUnitClass() );
	
	// ������ 35���� �۰ų� 2�� ������ �ƴϰų� ���� ����Ÿ�԰� ���� �� ����Ÿ���� �ٸ��ٸ� ����
	if( eNewUnitType == CXSLUnit::UC_NONE
		|| GetLevel() < 35 
		|| bValidSecondChangeJob == false 
		|| eOldUnitType != eNewUnitType )
	{
		return false;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// ��ų �ʱ�ȭ ����

	if( !SiCXSLSkillTree()->GetUnitClassDefaultSkill( iNewClassID, kPacketToDB.m_iDefaultSkillID1, kPacketToDB.m_iDefaultSkillID2, kPacketToDB.m_iDefaultSkillID3, 
														kPacketToDB.m_iDefaultSkillID4, kPacketToDB.m_iDefaultSkillID5, kPacketToDB.m_iDefaultSkillID6 ) )
	{
		START_LOG( cerr, L"����Ÿ���� �̻���." )
			<< BUILD_LOG( GetCharName() )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOG( cItemUnitClass )
			<< END_LOG;

		return false;
	}

	// ��� ��ų�� ���ٸ�..
	int iUsedSPoint = 0;
	int iUsedCSPoint = 0;
	int iTotalSP = 0;
	SiCXSLSkillTree()->GetCalcInitSkillPoint( GetLevel(), iTotalSP );

	int iRetrievedSPoint = 0;	// �������� SP
	int iRetrievedCSPoint = 0;	// �������� CSP

	if( false == m_kSkillTree.IsCashSkillPointExpired() )
	{
		iRetrievedSPoint	= iTotalSP;
		iRetrievedCSPoint	= m_kSkillTree.GetMaxCSPoint();
	}
	else
	{
		if( 0 == iUsedCSPoint && 0 == m_kSkillTree.GetCSPoint() )
		{
			iRetrievedSPoint	= iTotalSP;
			iRetrievedCSPoint	= 0;
		}
		else
		{
			START_LOG( cerr, L"cash skill point������ �ٵǾ��µ� ��ųƮ���� csp�� �����ִ�." )
				<< BUILD_LOG( GetCharName() )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOG( iUsedSPoint )
				<< BUILD_LOG( iUsedCSPoint )
				<< BUILD_LOG( m_kSkillTree.GetCSPoint() )
				<< END_LOG;

			return false;
		}
	}

	kPacketToDB.m_iUnitUID		 = GetCharUID();
	kPacketToDB.m_iSPoint		 = iRetrievedSPoint;
	kPacketToDB.m_iCSPoint		 = iRetrievedCSPoint;

	kPacketToDB.m_iNewUnitClass = static_cast<int>(cItemUnitClass);
	kPacketToDB.m_iUnitUID = GetCharUID();

	m_iSPoint.SetValue( iRetrievedSPoint );
	m_kSkillTree.SetCSPoint( iRetrievedCSPoint );



	//////////////////////////////////////////////////////////////////////////
	// ��ų
	std::set< int > setUnSealedSkillID;
	setUnSealedSkillID.clear();
	m_kSkillTree.GetUnSealedSkillList( setUnSealedSkillID );
	if( setUnSealedSkillID.size() > 0 )		// ���� ������ ��ų�� �����Ѵٸ� ��ų ��ȯ ��Ͽ� �߰�
	{
		std::set< int >::iterator sit = setUnSealedSkillID.begin();
		if( sit == setUnSealedSkillID.end() )
		{
			START_LOG( cerr, L"���� ���� ������ ��� - ���� ������ ��ų�� �����Ѵٰ� �ߴµ�...�����Ͱ� ����!" )
				<< BUILD_LOG( GetUserID() )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOGc( GetUnitClass() )
				<< BUILD_LOG( kPacketToDB.m_iNewUnitClass )
				<<END_LOG;
		}
		else
		{
			int iNewSkill = 0;
			for( ; sit != setUnSealedSkillID.end() ; ++sit )
			{
				//{{ Iruha : 2026-08-27 // VS2010 port: VC7.1's std::set::iterator
				// non-conformantly dereferenced to a mutable reference; VC10 dereferences to
				// const, and GetClassChange_NewSkill takes int& (non-const). Copy into a local.
				int iSkillID = *sit;
				if( SiKClassChangeTable()->GetClassChange_NewSkill( iNewClassID, iSkillID, iNewSkill ) == true )
				{
					kPacketToDB.m_mapChangeSkill.insert( std::make_pair(iSkillID, iNewSkill) );
				}
				//}}
				iNewSkill = 0;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// �޸�
	std::map< char, int >	mapSkillNote;
	mapSkillNote.clear();
	m_kSkillTree.GetMemoList( mapSkillNote );
	if( mapSkillNote.size() > 0 )		// ���� ������ ��ų�� �����Ѵٸ� ��ų ��ȯ ��Ͽ� �߰�
	{
		std::map< char, int >::iterator mit = mapSkillNote.begin();
		if( mit == mapSkillNote.end() )
		{
			START_LOG( cerr, L"���� ���� ������ ��� - ������ �޸� �����Ѵٰ� �ߴµ�...�����Ͱ� ����!" )
				<< BUILD_LOG( GetUserID() )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOGc( GetUnitClass() )
				<< BUILD_LOG( kPacketToDB.m_iNewUnitClass )
				<<END_LOG;
		}
		else
		{
			int iNewMemo = 0;
			for( ; mit != mapSkillNote.end() ; ++mit )
			{
				if( SiKClassChangeTable()->GetClassChange_NewMemo( iNewClassID, mit->second, iNewMemo ) == true )
				{
					kPacketToDB.m_mapChangeMemo.insert( std::make_pair(mit->second, iNewMemo) );
				}
				iNewMemo = 0;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// ������
	std::map< UidType, KInventoryItemInfo > mapInventoryItem;
	std::map< UidType, KInventoryItemInfo >::iterator mitInventoryItem;

	// Ư�� ���� ������ ������ ��� ����(ĳ������ �������� Ư�� �ǿ� �ִ�.)
	m_kInventory.GetInventoryInfo( CXSLInventory::ST_SPECIAL, mapInventoryItem, true );
	// ������ ������ ������ ��� ����
	m_kInventory.GetInventoryInfo( CXSLInventory::ST_BANK, mapInventoryItem, true );
	// �� ���� ������ ������ ��� ����
	m_kInventory.GetInventoryInfo( CXSLInventory::ST_PET, mapInventoryItem, true );

	mitInventoryItem = mapInventoryItem.begin();
	if( mitInventoryItem == mapInventoryItem.end() )	// �������� �����Ѵٸ� ��ȯ ������ ������ ���� Ȯ������
	{
		START_LOG( clog, L"���� ���� ������ ��� - Inventory Item �����Ͱ� ����!" )
			<< BUILD_LOG( GetUserID() )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOGc( GetUnitClass() )
			<< BUILD_LOG( kPacketToDB.m_iNewUnitClass )
			<<END_LOG;
	}
	else
	{
		KInventoryItemInfo kTempItemInfo;
		int	iNewItem = 0;
		for( ; mitInventoryItem != mapInventoryItem.end() ; ++mitInventoryItem )
		{
			kTempItemInfo = mitInventoryItem->second;

			if( SiKClassChangeTable()->GetClassChange_NewItem( iNewClassID, kTempItemInfo.m_kItemInfo.m_iItemID, iNewItem ) == false )
			{
				continue;
			}

			kPacketToDB.m_mapChangeItem.insert( std::make_pair( mitInventoryItem->first, iNewItem ) );
			iNewItem = 0;
		}
	}
	
	//////////////////////////////////////////////////////////////////////////
	// ����Ʈ
	// ( �Ϸ�� )
	// �Ϸ� ����Ʈ ���� �� ��� ã��
	{
		std::vector< int > vecCompleteQuest;
		m_kUserQuestManager.GetCompleteChangeClassQuest( vecCompleteQuest );

		std::vector< int >::const_iterator vit = vecCompleteQuest.begin();

		int	iOldQuestID = 0;
		int	iNewQuestID = 0;

		for( ; vit != vecCompleteQuest.end() ; ++vit )
		{
			iOldQuestID = (*vit);
			if( SiKClassChangeTable()->GetClassChange_NewQuest( iNewClassID, iOldQuestID, iNewQuestID ) == false )
			{
				continue;
			}
			
			bool bInsert = true;
			
			for( u_short usSize = 0 ; usSize < kPacketToDB.m_vecDeleteCompleteQuest.size() ; ++usSize )
			{
				if( iOldQuestID == kPacketToDB.m_vecDeleteCompleteQuest[usSize] )
				{
					bInsert = false;
					break;
				}
			}

			if( bInsert == true )
			{
				kPacketToDB.m_mapChangeCompleteQuest.insert( std::make_pair( iOldQuestID, iNewQuestID ) );
			}

			iOldQuestID = 0;
			iNewQuestID = 0;
		}
	}

	// ����Ʈ
	// ( ������ )
	{
		std::vector< int > vecInProgressQuest;
		m_kUserQuestManager.GetInProgressClassChangeQuest( vecInProgressQuest );

		std::vector< int >::const_iterator vit = vecInProgressQuest.begin();

		int	iOldQuestID = 0;
		int	iNewQuestID = 0;

		for( ; vit != vecInProgressQuest.end() ; ++vit )
		{
			iOldQuestID = (*vit);
			if( SiKClassChangeTable()->GetClassChange_NewQuest( iNewClassID, iOldQuestID, iNewQuestID ) == false )
			{
				continue;
			}

			bool bInsert = true;

			for( u_short usSize = 0 ; usSize < kPacketToDB.m_vecDeleteInProgressQuest.size() ; ++usSize )
			{
				if( iOldQuestID == kPacketToDB.m_vecDeleteInProgressQuest[usSize] )
				{
					bInsert = false;
					break;
				}
			}
	
			if( bInsert == true )
			{
				kPacketToDB.m_mapChangeInProgressQuest.insert( std::make_pair( iOldQuestID, iNewQuestID ) );
			}
			
			iOldQuestID = 0;
			iNewQuestID = 0;
		}
	}
	return true;
}
#endif SERV_UNLIMITED_SECOND_CHANGE_JOB

//{{  2011.11.24     ��μ�    ĳ���� ���� �α�
#ifdef SERV_CHAR_CONNECT_LOG
void KGSUser::SendUpdateUnitConnectLog()
{
	int iChannelID = SiKChannelManager()->GetChannelID();
	CTime tCurTime = CTime::GetCurrentTime();

	KDBE_UNIT_CONNECT_LOG_NOT kPacketLog;

	SiKChannelManager()->GetServerUIDByChannelID( iChannelID, kPacketLog.m_iServerUID );
	kPacketLog.m_iUnitUID	= GetCharUID();
	kPacketLog.m_iUnitClass = GetUnitClass();
	kPacketLog.m_iExp		= GetEXP();
	kPacketLog.m_iLevel		= GetLevel();
	kPacketLog.m_bPcBang = m_kUserPcBangManager.IsPcBang();
	kPacketLog.m_cAuthLevel = GetAuthLevel();
	kPacketLog.m_iLoginLevel = static_cast<int>(GetLoginLevel());
	kPacketLog.m_wstrName = GetCharName();
	kPacketLog.m_wstrLogin = ( std::wstring )(GetUnitLoginTime().Format( _T( "%Y-%m-%d %H:%M:%S" ) ));
	kPacketLog.m_wstrLogout = ( std::wstring )( tCurTime.Format( _T( "%Y-%m-%d %H:%M:%S" ) ) );
	kPacketLog.m_iPlayTime = static_cast<int>(tCurTime.GetTime() - GetUnitLoginTime().GetTime());

	SendToLogDB( DBE_UNIT_CONNECT_LOG_NOT, kPacketLog );

	START_LOG( clog, L"�ӽ� �ɸ��� ���� �α� �߰�" )
		<<END_LOG;
}
#endif SERV_CHAR_CONNECT_LOG
//}}

//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
void KGSUser::CheckBuffPeriod()
{		
	if( GetStateID() > KGSFSM::S_SERVER_SELECT  &&  GetStateID() < KGSFSM::STATE_SENTINEL )
	{
		// ������ 5�ʷ� �Ӵϴ�. üũ ������ �������� ���� �����ս��� �Ƴ��� ����.
		if( m_kTimer[TM_CHECK_BUFF_PERIOD].elapsed() < 1.0 )
			return;

		m_kTimer[TM_CHECK_BUFF_PERIOD].restart();


		std::vector< int > vecDeactivateBuffList;
		m_kUserBuffManager.CheckBuffPeriod( vecDeactivateBuffList, CXSLRoom::GetRoomType( GetRoomUID() ) );
		if( vecDeactivateBuffList.empty() == false )
		{
			// �α��� ���� ���� �Ŵ����� ������Ʈ
			KELG_UPDATE_USER_UNIT_BUFF_INFO_REQ kReq;
			kReq.m_iUnitUID = GetCharUID();
			kReq.m_vecDeActivateBuff = vecDeactivateBuffList;
			SendToLoginServer( ELG_UPDATE_USER_UNIT_BUFF_INFO_REQ, kReq );
		}
	}
}

void KGSUser::SendUpdateBuffInfo()
{
	if( GetRoomUID() > 0 )
	{
		const int iRoomType = CXSLRoom::GetRoomType( GetRoomUID() );
		switch( iRoomType )
		{
		case CXSLRoom::RT_PVP:
			{
#ifdef SERV_PVP_REMATCH
				if ( GetRoomListID() != 0)
				{
#endif //SERV_PVP_REMATCH
					KEGS_UPDATE_BUFF_INFO_IN_ROOM_NOT kPacketNot;
					kPacketNot.m_iUnitUID = GetCharUID();
					KRoomUserInfo kRoomUserInfo;
					GetRoomUserInfo( kRoomUserInfo, iRoomType );				
					m_kUserBuffManager.GetBuffList( kPacketNot.m_vecActivateBuffList, CXSLRoom::RT_PVP );
					kPacketNot.m_kGameStat = kRoomUserInfo.m_kGameStat;						// �׳� ���� ����� �� ����
					//{{ 2012. 12. 18	������	�ƶ� ��Ƽ �÷��� ���ʽ� ����ġ
#ifdef SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
					kPacketNot.m_mapBonusRate = kRoomUserInfo.m_mapBonusRate;
#endif SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
					//}}
					SendToCnRoom( ERM_UPDATE_BUFF_INFO_IN_ROOM_NOT, kPacketNot );
#ifdef SERV_PVP_REMATCH
				}
#endif //SERV_PVP_REMATCH
			}
			break;
		case CXSLRoom::RT_DUNGEON:
			{
				KEGS_UPDATE_BUFF_INFO_IN_ROOM_NOT kPacketNot;
				kPacketNot.m_iUnitUID = GetCharUID();
				KRoomUserInfo kRoomUserInfo;
				GetRoomUserInfo( kRoomUserInfo, iRoomType );				
				m_kUserBuffManager.GetBuffList( kPacketNot.m_vecActivateBuffList, CXSLRoom::RT_DUNGEON );
				kPacketNot.m_kGameStat = kRoomUserInfo.m_kGameStat;						// �׳� ���� ����� �� ����
				//{{ 2012. 12. 18	������	�ƶ� ��Ƽ �÷��� ���ʽ� ����ġ
#ifdef SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
				kPacketNot.m_mapBonusRate = kRoomUserInfo.m_mapBonusRate;
#endif SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
				//}}
				SendToCnRoom( ERM_UPDATE_BUFF_INFO_IN_ROOM_NOT, kPacketNot );
			}
			break;
		case CXSLRoom::RT_BATTLE_FIELD:
			{
				KEGS_UPDATE_BUFF_INFO_IN_ROOM_NOT kPacketNot;
				kPacketNot.m_iUnitUID = GetCharUID();
				KRoomUserInfo kRoomUserInfo;
				GetRoomUserInfo( kRoomUserInfo, iRoomType );				
				m_kUserBuffManager.GetBuffList( kPacketNot.m_vecActivateBuffList, CXSLRoom::RT_BATTLE_FIELD );
				kPacketNot.m_kGameStat = kRoomUserInfo.m_kGameStat;						// �׳� ���� ����� �� ����
				//{{ 2012. 12. 18	������	�ƶ� ��Ƽ �÷��� ���ʽ� ����ġ
#ifdef SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
				kPacketNot.m_mapBonusRate = kRoomUserInfo.m_mapBonusRate;
#endif SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
				//}}
				SendToCnRoom( ERM_UPDATE_BUFF_INFO_IN_ROOM_NOT, kPacketNot );
			}
			break;

		default:
			{
				KEGS_UPDATE_BUFF_INFO_NOT kPacketNot;
				m_kUserBuffManager.GetBuffList( kPacketNot.m_vecActivateBuffList );
				SendPacket( EGS_UPDATE_BUFF_INFO_NOT, kPacketNot );
			}
			break;
		}
	}
	else
	{
		KEGS_UPDATE_BUFF_INFO_NOT kPacketNot;
		m_kUserBuffManager.GetBuffList( kPacketNot.m_vecActivateBuffList );
		SendPacket( EGS_UPDATE_BUFF_INFO_NOT, kPacketNot );
	}
}

bool KGSUser::ActivateBuff( IN const int iBuffID, OUT std::vector< KBuffInfo >& vecActivatedBuff, IN const CTime tUseTime /* = 0*/, IN int iPeriod /* = 0*/ )
{
	KBuffInfo kBuff;
	kBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = iBuffID;

	// �����ϴ� �������� �˻�
	const CXSLBuffManager::BuffTemplet* pBuffTemplet = SiCXSLBuffManager()->GetBuffTemplet( iBuffID );
	if( pBuffTemplet == NULL )
	{
		START_LOG( cerr, L"���� ���� �ʴ� �����Դϴ�!" )
			<< BUILD_LOG( iBuffID )
			<< END_LOG;
		return false;
	}

	kBuff.m_kFactorInfo.m_iFactorID = pBuffTemplet->m_iFactorID;

	const bool bRet = m_kUserBuffManager.ActivateBuff( kBuff, vecActivatedBuff, tUseTime, iPeriod );

	return bRet;
}

bool KGSUser::ActivateItemBuff( IN const int iUseItemID, OUT std::vector< KBuffInfo >& vecActivatedBuff, OUT std::vector<int>& vecDeactiveBuff, IN const CTime tUseTime /*= 0*/ )
{
	if( SiCXSLItemManager()->IsBuffItem( iUseItemID ) == false )
	{
		START_LOG( clog, L"�������� ������(����)�Դϴ�." )
			<< BUILD_LOG( iUseItemID )
			<< END_LOG;
		return false;
	}

	switch( iUseItemID )
	{
	case CXSLItem::SI_THE_GATE_OF_DARKNESS_ARCANUM_NUT:
	case CXSLItem::SI_THE_GATE_OF_DARKNESS_ARCANUM_PTAH:
	case CXSLItem::SI_THE_GATE_OF_DARKNESS_ARCANUM_RACHEL:
	case CXSLItem::SI_THE_GATE_OF_DARKNESS_ARCANUM_ZADKIEL:
		{
			int iBuffID = 0;

			if( iUseItemID == CXSLItem::SI_THE_GATE_OF_DARKNESS_ARCANUM_NUT )		{ iBuffID = CXSLBuffManager::BTI_BUFF_PROTECTION_OF_NUT; }
			else if( iUseItemID == CXSLItem::SI_THE_GATE_OF_DARKNESS_ARCANUM_PTAH )	{ iBuffID = CXSLBuffManager::BTI_BUFF_PROTECTION_OF_PTAH; }
			else if( iUseItemID == CXSLItem::SI_THE_GATE_OF_DARKNESS_ARCANUM_RACHEL )	{ iBuffID = CXSLBuffManager::BTI_BUFF_PROTECTION_OF_LAHEL; }
			else if( iUseItemID == CXSLItem::SI_THE_GATE_OF_DARKNESS_ARCANUM_ZADKIEL )	{ iBuffID = CXSLBuffManager::BTI_BUFF_PROTECTION_OF_ZACHIEL; }

			// �����ϴ� �������� �˻�
			const CXSLBuffManager::BuffTemplet* pBuffTemplet = SiCXSLBuffManager()->GetBuffTemplet( iBuffID );
			if( pBuffTemplet == NULL )
			{
				START_LOG( cerr, L"���� ���� �ʴ� �����Դϴ�!" )
					<< BUILD_LOG( iBuffID )
					<< END_LOG;
				return false;
			}

			DeactivateBuff( CXSLBuffManager::BTI_BUFF_PROTECTION_OF_NUT, vecDeactiveBuff );
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_PROTECTION_OF_PTAH, vecDeactiveBuff );
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_PROTECTION_OF_LAHEL, vecDeactiveBuff );
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_PROTECTION_OF_ZACHIEL, vecDeactiveBuff );

			// ������ ��� ����
			ActivateBuff( iBuffID, vecActivatedBuff, tUseTime );

			// ��� ��� �����
			const CXSLBuffManager::BuffTemplet* pDeBuffTemplet = SiCXSLBuffManager()->GetBuffTemplet( CXSLBuffManager::BTI_BUFF_STRENGTHENING_BODY );
			if( pDeBuffTemplet == NULL )
			{
				START_LOG( cerr, L"���� ���� �ʴ� �����Դϴ�!" )
					<< BUILD_LOG( CXSLBuffManager::BTI_BUFF_STRENGTHENING_BODY )
					<< END_LOG;
				return false;
			}

			ActivateBuff( CXSLBuffManager::BTI_BUFF_STRENGTHENING_BODY, vecActivatedBuff, tUseTime );
		}
		break;
	case CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_SERAPHIM:
	case CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_GEB:
	case CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_AMON:
	case CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_CRONOS:
		//{{ 2013. 04. 12	�ڼ���	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
	case CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_GIANT_POTION:
	case CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_BLAZING_BOMB:
	case CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_SPIRIT_OF_CHASER:
	case CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_CRADLE_OF_LITTLE_FAIRY:
	case CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_ICE_BALL_OF_DENIPH:
	case CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_FEATHER_OF_VENTUS:
	case CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_FLAME_RING_OF_ROSSO:
	case CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_BIG_HAND_POTION:
#endif SERV_NEW_DEFENCE_DUNGEON
		//}}
#ifdef SERV_EVENT_ARA_NEW_CHAR_THE_ESSENCE_OF_HERETIC
	case CXSLItem::EI_THE_ESSENCE_OF_WEAK_HERETIC_POTION:
	case CXSLItem::EI_THE_ESSENCE_OF_HERETIC_POTION:		
#endif SERV_EVENT_ARA_NEW_CHAR_THE_ESSENCE_OF_HERETIC
#ifdef SERV_EVENT_RURIEL_MANA_ENERGIZE_POTION
	case CXSLItem::EI_BUFF_RURIEL_MANA_ENERGIZE_POTION:		
#endif SERV_EVENT_RURIEL_MANA_ENERGIZE_POTION
		{
			int iBuffID = 0;

			if( iUseItemID == CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_SERAPHIM )					{ iBuffID = CXSLBuffManager::BTI_BUFF_BELSSING_OF_SERAPHIM; }
			else if( iUseItemID == CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_GEB )					{ iBuffID = CXSLBuffManager::BTI_BUFF_BELSSING_OF_GEB; }
			else if( iUseItemID == CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_AMON )					{ iBuffID = CXSLBuffManager::BTI_BUFF_BELSSING_OF_AMON; }
			else if( iUseItemID == CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_CRONOS )				{ iBuffID = CXSLBuffManager::BTI_BUFF_BELSSING_OF_CRONOS; }
			//{{ 2013. 04. 12	�ڼ���	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
			else if( iUseItemID == CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_GIANT_POTION )			{ iBuffID = CXSLBuffManager::BTI_BUFF_ELIXIR_GIANT_POTION; }
			else if( iUseItemID == CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_BLAZING_BOMB )			{ iBuffID = CXSLBuffManager::BTI_BUFF_ELIXIR_BLAZING_BOMB; }
			else if( iUseItemID == CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_SPIRIT_OF_CHASER )		{ iBuffID = CXSLBuffManager::BTI_BUFF_ELIXIR_SPIRIT_OF_CHASER; }
			else if( iUseItemID == CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_CRADLE_OF_LITTLE_FAIRY ){ iBuffID = CXSLBuffManager::BTI_BUFF_ELIXIR_CRADLE_OF_LITTLE_FAIRY; }
			else if( iUseItemID == CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_ICE_BALL_OF_DENIPH )	{ iBuffID = CXSLBuffManager::BTI_BUFF_ELIXIR_ICE_BALL_OF_DENIPH; }
			else if( iUseItemID == CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_FEATHER_OF_VENTUS )		{ iBuffID = CXSLBuffManager::BTI_BUFF_ELIXIR_FEATHER_OF_VENTUS; }
			else if( iUseItemID == CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_FLAME_RING_OF_ROSSO )	{ iBuffID = CXSLBuffManager::BTI_BUFF_ELIXIR_FLAME_RING_OF_ROSSO; }
			else if( iUseItemID == CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_BIG_HAND_POTION )		{ iBuffID = CXSLBuffManager::BTI_BUFF_ELIXIR_BIG_HAND_POTION; }
#endif SERV_NEW_DEFENCE_DUNGEON
			//}}
#ifdef SERV_EVENT_ARA_NEW_CHAR_THE_ESSENCE_OF_HERETIC
			else if( iUseItemID == CXSLItem::EI_THE_ESSENCE_OF_WEAK_HERETIC_POTION )				{ iBuffID = CXSLBuffManager::BTI_BUFF_THE_ESSENCE_OF_WEAK_HERETIC_POTION; }
			else if( iUseItemID == CXSLItem::EI_THE_ESSENCE_OF_HERETIC_POTION )						{ iBuffID = CXSLBuffManager::BTI_BUFF_THE_ESSENCE_OF_HERETIC_POTION; }
#endif SERV_EVENT_ARA_NEW_CHAR_THE_ESSENCE_OF_HERETIC
#ifdef SERV_EVENT_RURIEL_MANA_ENERGIZE_POTION
			else if( iUseItemID == CXSLItem::EI_BUFF_RURIEL_MANA_ENERGIZE_POTION )					{ iBuffID = CXSLBuffManager::BTI_BUFF_RURIEL_MANA_ENERGIZE_POTION; }
#endif SERV_EVENT_RURIEL_MANA_ENERGIZE_POTION


			// �����ϴ� �������� �˻�
			const CXSLBuffManager::BuffTemplet* pBuffTemplet = SiCXSLBuffManager()->GetBuffTemplet( iBuffID );
			if( pBuffTemplet == NULL )
			{
				START_LOG( cerr, L"���� ���� �ʴ� �����Դϴ�!" )
					<< BUILD_LOG( iBuffID )
					<< END_LOG;
				return false;
			}

			DeactivateBuff( CXSLBuffManager::BTI_BUFF_BELSSING_OF_SERAPHIM, vecDeactiveBuff );
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_BELSSING_OF_GEB, vecDeactiveBuff );
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_BELSSING_OF_AMON, vecDeactiveBuff );
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_BELSSING_OF_CRONOS, vecDeactiveBuff );
			//{{ 2013. 04. 12	�ڼ���	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_ELIXIR_GIANT_POTION, vecDeactiveBuff );
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_ELIXIR_BLAZING_BOMB, vecDeactiveBuff );
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_ELIXIR_SPIRIT_OF_CHASER, vecDeactiveBuff );
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_ELIXIR_CRADLE_OF_LITTLE_FAIRY, vecDeactiveBuff );
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_ELIXIR_ICE_BALL_OF_DENIPH, vecDeactiveBuff );
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_ELIXIR_FEATHER_OF_VENTUS, vecDeactiveBuff );
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_ELIXIR_FLAME_RING_OF_ROSSO, vecDeactiveBuff );
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_ELIXIR_BIG_HAND_POTION, vecDeactiveBuff );
#endif SERV_NEW_DEFENCE_DUNGEON
			//}}
#ifdef SERV_EVENT_ARA_NEW_CHAR_THE_ESSENCE_OF_HERETIC
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_THE_ESSENCE_OF_WEAK_HERETIC_POTION, vecDeactiveBuff );
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_THE_ESSENCE_OF_HERETIC_POTION, vecDeactiveBuff );
#endif SERV_EVENT_ARA_NEW_CHAR_THE_ESSENCE_OF_HERETIC
#ifdef SERV_EVENT_RURIEL_MANA_ENERGIZE_POTION
			DeactivateBuff( CXSLBuffManager::BTI_BUFF_RURIEL_MANA_ENERGIZE_POTION, vecDeactiveBuff );
#endif SERV_EVENT_RURIEL_MANA_ENERGIZE_POTION

			// ������ ��� ����
			ActivateBuff( iBuffID, vecActivatedBuff, tUseTime );

		}
		break;
	default:
		{
			START_LOG( cerr, L"���� ������ ���� �������� ��� �߽��ϴ�." )
				<< BUILD_LOG( GetCharName() )
				<< BUILD_LOG( iUseItemID )
				<< END_LOG;

			return false;
		}
	}

	return true;
}

bool KGSUser::DeactivateBuff( IN const int iBuffID, OUT std::vector< int >& vecDeleteBuff )
{
	// �����ϴ� �������� �˻�
	const CXSLBuffManager::BuffTemplet* pBuffTemplet = SiCXSLBuffManager()->GetBuffTemplet( iBuffID );
	if( pBuffTemplet == NULL )
	{
		START_LOG( cerr, L"���� ���� �ʴ� �����Դϴ�!" )
			<< BUILD_LOG( iBuffID )
			<< END_LOG;
		return false;
	}

	if( m_kUserBuffManager.IsBuffActivated( iBuffID ) == false )
		return false;

	const bool bRet = m_kUserBuffManager.DeactivateBuff( iBuffID, vecDeleteBuff );

	return bRet;
}

void KGSUser::CheckDeselectUnitBuff( IN const bool bSendAck /*= true*/ )
{
	std::vector<int> vecDeleteBuff;
	
	// ���� ���� ��(ĳ���� ���� ����) ������ �� ���� ������ ã��.
	
	// ������ ���� ��� (���� ����)
	DeactivateBuff( CXSLBuffManager::BTI_BUFF_WARM_ENERGE_OF_EL, vecDeleteBuff );
	// ������ ���� ( �ڵ���Ƽ���� )
	DeactivateBuff( CXSLBuffManager::BTI_BUFF_RALLY_OF_HERO_LEVEL_1, vecDeleteBuff );
	DeactivateBuff( CXSLBuffManager::BTI_BUFF_RALLY_OF_HERO_LEVEL_2, vecDeleteBuff );
	DeactivateBuff( CXSLBuffManager::BTI_BUFF_RALLY_OF_HERO_LEVEL_3, vecDeleteBuff );
	// ������ ��� ����
	DeactivateBuff( CXSLBuffManager::BTI_BUFF_PVP_REVENGE_MODE_LEVEL_1, vecDeleteBuff );
	DeactivateBuff( CXSLBuffManager::BTI_BUFF_PVP_REVENGE_MODE_LEVEL_2, vecDeleteBuff );
	DeactivateBuff( CXSLBuffManager::BTI_BUFF_PVP_REVENGE_MODE_LEVEL_3, vecDeleteBuff );
	//PC �� ������� �ص� �����
	DeactivateBuff( CXSLBuffManager::BTI_BUFF_PREMIUM_PC_ROOM, vecDeleteBuff );
	//{{ 2012. 12. 18	������	�ƶ� ��Ƽ �÷��� ���ʽ� ����ġ
#ifdef SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
	DeactivateBuff( CXSLBuffManager::BTI_SECRET_OF_THOUSAND_YEARS_FOX, vecDeleteBuff );
#endif SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
	//}}

#ifdef SERV_BONUS_BUFF_SYSTEM
	// �������� ���ʽ��� ���� ������ �� �����ݴϴ�.
	// for ���鼭 ���ʽ� ������ �� ����ô�
	DeactivateBuff( CXSLBuffManager::BTI_BUFF_2YEARS_EVENT_CN_DAMAGE_UP, vecDeleteBuff );
	DeactivateBuff( CXSLBuffManager::BTI_BUFF_2YEARS_EVENT_CN_MOVE_JUMP_SPEED_UP, vecDeleteBuff );
	DeactivateBuff( CXSLBuffManager::BTI_BUFF_2YEARS_EVENT_CN_ACCURACY_AVOID_UP, vecDeleteBuff );
	DeactivateBuff( CXSLBuffManager::BTI_BUFF_2YEARS_EVENT_CN_CRI_SPEED_UP, vecDeleteBuff );
	DeactivateBuff( CXSLBuffManager::BTI_BUFF_2YEARS_EVENT_CN_DAMAGE_INC_DEC_UP, vecDeleteBuff );
	DeactivateBuff( CXSLBuffManager::BTI_BUFF_2YEARS_EVENT_CN_ALL_IN_ONE, vecDeleteBuff );
#endif //SERV_BONUS_BUFF_SYSTEM

#ifdef SERV_NEW_UNIT_TRADE_LIMIT
	if( m_kUserAbuserManager.IsTradeBlockUnit() == true )
	{
		DeactivateBuff( CXSLBuffManager::BTI_BUFF_TRADE_BLOCK, vecDeleteBuff );
	}
#endif // SERV_NEW_UNIT_TRADE_LIMIT

	if( m_kComeBackUserInfo.GetIsComeBackUser() == false )
	{
		DeactivateBuff( CXSLBuffManager::BTI_BUFF_RETURN_OF_HERO, vecDeleteBuff );
	}

	if( vecDeleteBuff.empty() == false )
	{
		KELG_UPDATE_USER_UNIT_BUFF_INFO_REQ kBuffReq;
		kBuffReq.m_iUnitUID = GetCharUID();
		kBuffReq.m_vecDeActivateBuff = vecDeleteBuff;
		kBuffReq.m_bSendAck = bSendAck;
		SendToLoginServer( ELG_UPDATE_USER_UNIT_BUFF_INFO_REQ, kBuffReq );
	}
}
#endif SERV_SERVER_BUFF_SYSTEM
//}

//{{ 2012. 04. 02	��μ�		sn ����͸� ��� �и�
#ifdef SERV_SERIAL_NUMBER_MORNITORING
void KGSUser::CheckServerSN_Monitoring()
{
	if( m_kUserSecurityManager.CheckServerSN_Monitoring() == true )
	{
		//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
		KEGS_SERVER_CHECK_HACK_USER_REQ kNot;
		kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_SERVER_SN_MONITORING;
		SendPacket( EGS_SERVER_CHECK_HACK_USER_REQ, kNot );

		m_kUserHackingManager.SetHackingUserSendType( KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_SERVER_SN_MONITORING );
#else
		KEGS_SERVER_CHECK_HACK_USER_NOT kNot;
		kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_SERVER_SN_MONITORING;
		SendPacket( EGS_SERVER_CHECK_HACK_USER_NOT, kNot );
#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
		//}}

		START_LOG( clog, L"SN ����͸� ����-������Ŷ ������" )
			<< END_LOG;
	}
}
#endif SERV_SERIAL_NUMBER_MORNITORING
//}}

//{{ 2012. 03. 27	��μ�		�ӽ� �ð��� �˻�����(50�� �̻� ���̳��� ����͸�)
#ifdef SERV_CHECK_MACHINE_LOCALTIME
bool KGSUser::CheckMachineLocalTime( SYSTEMTIME& st, std::wstring& wstrClientTime )
{
	CTime tServer = CTime::CTime( st );
	CTimeSpan tSpan = CTimeSpan(0, 9, 0, 0); // �ѱ� �ð� ������
	tServer += tSpan;

	CTime tClient;
	std::wstring wsrtServerTime = tServer.Format( _T( "%Y-%m-%d %H:%M:%S" ) );

	if( KncUtil::ConvertStringToCTime( wstrClientTime, tClient )  == false )
	{
		START_LOG( cerr, L"Ŭ���̾�Ʈ �ð� ���� ��ȯ ����." )
			<< BUILD_LOG( GetName() )
			<< BUILD_LOG( wstrClientTime )
			<< BUILD_LOG( wsrtServerTime )
			<< END_LOG;
		return false;
	}

	CTimeSpan tSpan_min = CTimeSpan( 0, 0, 50, 0);
	if( tClient < tServer - tSpan_min )
	{
		START_LOG( cerr, L"Ŭ���̾�Ʈ �ð� ���� �̻�." )
			<< BUILD_LOG( GetName() )
			<< BUILD_LOG( wstrClientTime )
			<< BUILD_LOG( wsrtServerTime )
			<< END_LOG;

		return false;
	}

	START_LOG( clog, L"Ŭ���̾�Ʈ �ð� ���� �̻�." )
		<< BUILD_LOG( GetName() )
		<< BUILD_LOG( wstrClientTime )
		<< BUILD_LOG( wsrtServerTime )
		<< END_LOG;

	return true;
}
#endif SERV_CHECK_MACHINE_LOCALTIME
//}}

//{{ 2012. 03. 29	��μ�		x2.exe ��� ���� �˻�
#ifdef SERV_CHECK_X2_EXE_MODULE
bool KGSUser::CheckModuleList_X2Module( IN std::vector< std::wstring >& vecModuleList, OUT std::map< std::wstring, bool >& mapExistModule )
{
	// ���� �ؾ� �ϴ� ���
	std::map< std::wstring, bool >::iterator mit;
	SiKMornitoringManager()->GetCheckExistModule( mapExistModule );

	BOOST_TEST_FOREACH( const std::wstring&, wstrName, vecModuleList )
	{
		mit = mapExistModule.find( wstrName );
		if( mit != mapExistModule.end() )
		{
			START_LOG( clog, L"���� �ؾ��ϴ� module �� �ִ�!!!" );

			mit->second = true;
		}
	}

	for( mit = mapExistModule.begin() ; mit != mapExistModule.end() ; ++mit )
	{
		if( mit->second == false )
		{
			START_LOG( cerr, L"���� �ؾ��ϴ� module �� ����!!!" )
				<< BUILD_LOG( mit->first )
				<< END_LOG;

			return false;
		}
	}

	return true;
}
#endif SERV_CHECK_X2_EXE_MODULE
//}}

//{{ 2012. 05. 23	��μ�       �Ǹ� �Ұ� ������ �Ǹ� �õ� ��ŷ ������ �Ǵ�
#ifdef SERV_IMPOSSIBLE_SELL_ITEM
void KGSUser::SetRandomTimeKick( int iReason, int iMinTime, int iRange )
{
	m_kUserSecurityManager.SetKickRandomTime( iMinTime, iRange );
	m_kUserSecurityManager.SetKickState( KUserSecurityManager::KT_AFTER_RANDOM_TIME );
	m_kUserSecurityManager.SetKickReason( iReason );
}

void KGSUser::CheckRandomTimeKick()
{
	if( m_kUserSecurityManager.CheckIsKickUser() == true )
	{
		SetDisconnectReason( m_kUserSecurityManager.GetKickReason() );
		ReserveDestroy();
	}
}
#endif SERV_IMPOSSIBLE_SELL_ITEM
//}}

//{{ 2012. 05. 29	��μ�       ���� ���� �ŷ� ���� ����
#ifdef SERV_TRADE_LOGIC_CHANGE_PERSONAL_SHOP
void KGSUser::SetPersonalShopBuyItemInfo( IN KERM_BUY_PERSONAL_SHOP_ITEM_REQ& kPacket )
{
	m_kUserPersonalShopManager.SetPersonalShopBuyItemUID( kPacket.m_iBuyItemUID );
	m_kUserPersonalShopManager.SetPersonalShopBuyItemID( kPacket.m_iBuyItemID );
	m_kUserPersonalShopManager.SetPersonalShopBuyQuantity( kPacket.m_iBuyQuantity );
	m_kUserPersonalShopManager.SetPersonalShopBuyUpdate( kPacket.m_kItemQuantityUpdateTemp );
	m_kUserPersonalShopManager.SetPersonalShopBuyItemInfo( kPacket.m_vecItemInfoTemp );
}

void KGSUser::GetPersonalShopBuyItemInfo( OUT KERM_BUY_PERSONAL_SHOP_ITEM_REQ& kPacket )
{
	kPacket.m_iBuyItemUID = m_kUserPersonalShopManager.GetPersonalShopBuyItemUID();
	kPacket.m_iBuyItemID = m_kUserPersonalShopManager.GetPersonalShopBuyItemID();
	kPacket.m_iBuyQuantity = m_kUserPersonalShopManager.GetPersonalShopBuyQuantity();
	m_kUserPersonalShopManager.GetPersonalShopBuyUpdate( kPacket.m_kItemQuantityUpdateTemp );
	m_kUserPersonalShopManager.GetPersonalShopBuyQuantity( kPacket.m_vecItemInfoTemp );
	kPacket.m_iBuyUnitNowED = GetED();
}
#endif SERV_TRADE_LOGIC_CHANGE_PERSONAL_SHOP
//}}

//{{ 2012. 05. 31	��μ�       �븮 ���� �ŷ� ���� ����
#ifdef SERV_TRADE_LOGIC_CHANGE_AGENCY_SHOP
void KGSUser::SetAgencyShopPickUpItemInfo( IN std::vector< UidType >& vecPickUpItemList )
{
	m_kUserPersonalShopManager.SetAgencyShopPickUpItemInfo( vecPickUpItemList );
}

void KGSUser::GetAgencyShopPickUpItemInfo( OUT std::vector< UidType >& vecPickUpItemList )
{
	m_kUserPersonalShopManager.GetAgencyShopPickUpItemInfo( vecPickUpItemList );
}
#endif SERV_TRADE_LOGIC_CHANGE_AGENCY_SHOP
//}}

//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
void KGSUser::SetHackingUserSendType( IN char cType )
{
	m_kUserHackingManager.SetHackingUserSendType( cType );
}
#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
//}}

//{{ 2012. 11. 13	������		�ڵ���Ƽ ��⿭ ī��Ʈ ǥ��
#ifdef SERV_AUTO_PARTY_WAIT_USER_COUNT_VIEW
void KGSUser::CheckAutoPartyWaitUserCountRequest()
{
	// ���ʿ� �ڵ� ��Ƽ ��⿭�� ��ϵ� ���°� �ƴ϶�� üũ���� �ʴ´�.
	if( GetAutoPartyWaitNumber() == 0 )
		return;

	if( GetStateID() > KGSFSM::S_SERVER_SELECT  &&  GetStateID() < KGSFSM::STATE_SENTINEL )
	{
		if( m_kTimer[TM_AUTO_PARTY_WAIT_USER_COUNT].elapsed() < 10.0 )
			return;

		m_kTimer[TM_AUTO_PARTY_WAIT_USER_COUNT].restart();
		
		// ���� ��ϵ� ��⿭�� ��� �ο��� ��û�Ѵ�!
		KEGB_REQUEST_AUTO_PARTY_WAIT_USER_COUNT_NOT kPacket;
		kPacket.m_iAutoPartyWaitNumber = GetAutoPartyWaitNumber();
		SendToAutoParty( EGB_REQUEST_AUTO_PARTY_WAIT_USER_COUNT_NOT, kPacket );
	}
}
#endif SERV_AUTO_PARTY_WAIT_USER_COUNT_VIEW
//}}

//{{ 2012. 02. 22	�ڼ���	��� �̸� �����
#ifdef SERV_GUILD_CHANGE_NAME
bool KGSUser::IsGuildMaster( void )
{
	return m_kUserGuildManager.IsGuildMaster();
}
#endif SERV_GUILD_CHANGE_NAME
//}}


//{{ 2012. 06. 13	��μ�       ������ �˴ٿ���
#ifdef SERV_SELECTIVE_SHUTDOWN_SYSTEM
void KGSUser::CheckRestrictionUser()
{
	// 10�ʸ��� Ȯ�� ����
	if( m_kTimer[TM_SELECTIVE_SHUTDOWN_SYSTEM_CHECK].elapsed() < 10 )
	{
		// �н�~
		return;
	}

	m_kTimer[TM_SELECTIVE_SHUTDOWN_SYSTEM_CHECK].restart();

	//1. ���� �̿� ���� ����ڰ� �ƴ϶� �޼��� ����� �Ѵ�.
	if( m_kUserInfoByNexon.GetRestrictionUser() < KUserInfoByNexon::SHUTDOWN_USER )
	{
		if( m_kUserInfoByNexon.IsExistRestrictionTime() == true )
		{
			KEGS_SHUT_DOWN_MESSAGE_NOT kNot;

			switch( m_kUserInfoByNexon.GetRestrictionUserMSG() )
			{
			case KUserInfoByNexon::AGE_AUTH_ERROR: // 1
				{
					kNot.m_iNetErrorID = NetError::ERR_SHUT_DOWN_02;
				}break;
			case KUserInfoByNexon::NAME_AUTH_ERROR: // 2
				{
					kNot.m_iNetErrorID = NetError::ERR_SHUT_DOWN_03;
				}break;
			case KUserInfoByNexon::SHUTDOWN_USER_ERROR: // 3
				{
					kNot.m_iNetErrorID = NetError::ERR_SHUT_DOWN_04;
				}break;
			case KUserInfoByNexon::SELECTIVE_SHUTDOWN_USER_ERROR: // 4
				{
					kNot.m_iNetErrorID = NetError::ERR_SHUT_DOWN_05;
				}break;
			case KUserInfoByNexon::UNKNOWN_ERROR: // 99
			default:
				{
					kNot.m_iNetErrorID = NetError::ERR_SHUT_DOWN_01;
				}
			}

			CTime tCurr = CTime::GetCurrentTime();
			CTime tRestrictionTime;

			std::wstring wstrRestrictionTime;
			m_kUserInfoByNexon.GetRestrictionUserTime( wstrRestrictionTime );
		
			if( KncUtil::ConvertStringToCTime( wstrRestrictionTime, tRestrictionTime ) == false )
			{
				// �ð��� �����Ƿ� �Ǵ� �� ���� ����.
				return;
			}

			// ���� �ð� ������ ������ ���� �ִ�.
			if( m_kUserInfoByNexon.IsSendRestrictionUserMSG() == true )
			{
				return;
			}

			CTimeSpan tMsgTime = CTimeSpan( 0, 0, 10, 0 ); // �̸� �˸� 10�� ��

			if( tCurr < tRestrictionTime && tCurr + tMsgTime > tRestrictionTime )
			{
				m_kUserInfoByNexon.SetSendRestrictionUserMSG( true );

				KEGS_BEFORE_SHUT_DOWN_MESSAGE_NOT kMsgNot;
				kMsgNot.m_iNetErrorID = kNot.m_iNetErrorID;
				kMsgNot.m_wstrTime = wstrRestrictionTime;

				SendPacket( EGS_BEFORE_SHUT_DOWN_MESSAGE_NOT, kMsgNot );

				START_LOG( clog, L"���� �̿� ���� �̸� �˸� ���� ���� Ȯ��." )
					<< BUILD_LOG( GetCharName() )
					<< BUILD_LOG( m_kUserInfoByNexon.GetRestrictionUser() )
					<< BUILD_LOG( m_kUserInfoByNexon.GetRestrictionUserMSG() )
					<< BUILD_LOG( kMsgNot.m_iNetErrorID )
					<< BUILD_LOG( kMsgNot.m_wstrTime )
					<< END_LOG;

				return;
			}
			return;
		}
		else
		{
			return;
		}
	}

	//3. ���� �̿� ���� ����ڶ�� ű���� �ʾҳ�?!
	if( m_kUserInfoByNexon.GetRestrictionUser() == KUserInfoByNexon::SHUTDOWN_USER )
	{
		KEGS_SHUT_DOWN_MESSAGE_NOT kNot;

		switch( m_kUserInfoByNexon.GetRestrictionUserMSG() )
		{
		case KUserInfoByNexon::AGE_AUTH_ERROR: // 1
			{
				kNot.m_iNetErrorID = NetError::ERR_SHUT_DOWN_02;
			}break;
		case KUserInfoByNexon::NAME_AUTH_ERROR: // 2
			{
				kNot.m_iNetErrorID = NetError::ERR_SHUT_DOWN_03;
			}break;
		case KUserInfoByNexon::SHUTDOWN_USER_ERROR: // 3
			{
				kNot.m_iNetErrorID = NetError::ERR_SHUT_DOWN_04;
			}break;
		case KUserInfoByNexon::SELECTIVE_SHUTDOWN_USER_ERROR: // 4
			{
				kNot.m_iNetErrorID = NetError::ERR_SHUT_DOWN_05;
			}break;
		case KUserInfoByNexon::UNKNOWN_ERROR: // 99
		default:
			{
				kNot.m_iNetErrorID = NetError::ERR_SHUT_DOWN_01;
			}
		}

		START_LOG( clog, L"���� �̿� ���� Ȯ��." )
			<< BUILD_LOG( GetCharName() )
			<< BUILD_LOG( m_kUserInfoByNexon.GetRestrictionUser() )
			<< BUILD_LOG( m_kUserInfoByNexon.GetRestrictionUserMSG() )
			<< END_LOG;

		SendPacket( EGS_SHUT_DOWN_MESSAGE_NOT, kNot );

		// ���� ���� ����
		// ���� �ð� �ڿ� ���� �ǵ��� ����
		SetRandomTimeKick( KStatistics::eSIColDR_Policy_Shutdown_Disconnect, 1, 1 );
		m_kUserInfoByNexon.SetRestrictionUser( KUserInfoByNexon::KICK_USER );
		
		return;
	}
}
#endif SERV_SELECTIVE_SHUTDOWN_SYSTEM
//}}

//{{ 2012. 04. 17	������	����Ʈ ���� ���� ����
#ifdef SERV_ENTER_FIELD_QUEST_CLEAR
void KGSUser::AddTempInventoryItem( IN const int iItemID
								  , IN const u_char ucSealData
								  //{{ 2012. 1. 16	�ڼ���	���� �� �ʵ� ��� �����ۿ� ��ȭ �ο�
#ifdef SERV_ENCHANT_ITEM_DROP_EVENT
								  , IN const KItemInfo& kItemInfo
#endif SERV_ENCHANT_ITEM_DROP_EVENT
								  //}}
								  )
{
	// �ӽ� �κ��丮�� �����Ϸ� ����!
	KEGS_GET_ITEM_TEMP_INVENTORY_NOT kNot;
	if( m_kTempInventory.AddTempInventoryItem( iItemID
											 , ucSealData
											 //{{ 2012. 1. 16	�ڼ���	���� �� �ʵ� ��� �����ۿ� ��ȭ �ο�
#ifdef SERV_ENCHANT_ITEM_DROP_EVENT
											 , kItemInfo
#endif SERV_ENCHANT_ITEM_DROP_EVENT
											 //}}
											 , kNot.m_kTempInvenItem
											 ) )
	{
		SendPacket( EGS_GET_ITEM_TEMP_INVENTORY_NOT, kNot );
	}
	else
	{
		START_LOG( cerr, L"�ӽ� �κ��丮�� ������ ���� ����!" )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOG( iItemID )
			<< END_LOG;
	}
}

void KGSUser::SendQuestRewardLetter( IN const int iQuestID, IN const int iRewardItemID, IN const int iQuantity )
{
	CStringW cwstrQuestID;
	cwstrQuestID.Format( L"%d", iQuestID );

	KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDB;
	kPacketToDB.m_iFromUnitUID = GetCharUID();
	kPacketToDB.m_iToUnitUID   = GetCharUID();
	kPacketToDB.m_iRewardType  = KPostItemInfo::LT_QUEST_REWARD;
	kPacketToDB.m_iRewardID	   = iRewardItemID;
	kPacketToDB.m_sQuantity	   = iQuantity;
	kPacketToDB.m_wstrMessage  = cwstrQuestID.GetBuffer();
	kPacketToDB.m_bGameServerEvent = true;
	SendToGameDB( DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDB );
}
#endif SERV_ENTER_FIELD_QUEST_CLEAR
//}}

//{{ 2012. 10. 02	������		ED&������ ����͸� �ý���
#ifdef SERV_ED_AND_ITEM_MORNITORING_SYSTEM
void KGSUser::SetED( IN const int iED, IN const KUserEDManager::ED_REASON eReason )
{
	const int iBeforeED = GetED();
	const int iChangeED = iED - GetED();
	const int iAfterED = iED;

	//////////////////////////////////////////////////////////////////////////
	START_LOG( cout, L"[�׽�Ʈ�α�] ED�� ����!" )
		<< BUILD_LOG( GetCharName() )
		<< BUILD_LOG( eReason )
		<< BUILD_LOG( iBeforeED )
		<< BUILD_LOG( iChangeED )
		<< BUILD_LOG( iAfterED );
	//////////////////////////////////////////////////////////////////////////

	// ED����
	m_kEDManager.Init( iED, eReason );

	// ED����� ����͸�
	m_kUserAbuserManager.CheckEDAbuser( GetThisPtr<KGSUser>(), static_cast<int>(eReason), iBeforeED );
}

void KGSUser::IncreaseED( IN const int iIncreaseED, IN const KUserEDManager::ED_REASON eReason )
{
	if( iIncreaseED == 0 )
		return;

	//////////////////////////////////////////////////////////////////////////
	START_LOG( cout, L"[�׽�Ʈ�α�] ED����!" )
		<< BUILD_LOG( GetCharName() )
		<< BUILD_LOG( GetED() )
		<< BUILD_LOG( iIncreaseED )
		<< BUILD_LOG( eReason );
	//////////////////////////////////////////////////////////////////////////

	const int iBeforeED = GetED();

	// ED����
	m_kEDManager.IncreaseMyED( iIncreaseED, eReason );

	// ED����� ����͸�
	m_kUserAbuserManager.CheckEDAbuser( GetThisPtr<KGSUser>(), static_cast<int>(eReason), iBeforeED );
}

void KGSUser::DecreaseED( IN const int iDecreaseED, IN const KUserEDManager::ED_REASON eReason )
{
	if( iDecreaseED == 0 )
		return;

	//////////////////////////////////////////////////////////////////////////
	START_LOG( cout, L"[�׽�Ʈ�α�] ED����!" )
		<< BUILD_LOG( GetCharName() )
		<< BUILD_LOG( GetED() )
		<< BUILD_LOG( iDecreaseED )
		<< BUILD_LOG( eReason );
	//////////////////////////////////////////////////////////////////////////

	const int iBeforeED = GetED();

	// ED����
	m_kEDManager.DecreaseMyED( iDecreaseED, eReason );

	// ED����� ����͸�
	m_kUserAbuserManager.CheckEDAbuser( GetThisPtr<KGSUser>(), static_cast<int>(eReason), iBeforeED );
}
#endif SERV_ED_AND_ITEM_MORNITORING_SYSTEM
//}}

//{{ 2012. 09. 11	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2
CXSLUnit::PVP_RANK KGSUser::GetPvpRankForClient( void ) const
{
	return ( m_kUserPvpManager.GetOfficialMatchCount() < 10 ) ? CXSLUnit::PVPRANK_RANK_ARRANGE : m_kUserPvpManager.GetPvpRank();
}

void KGSUser::GetPvpRankForClient( IN const int iOfficialMatchCnt, IN OUT char& m_cRank ) const
{
	if( iOfficialMatchCnt < 10 )
	{
		m_cRank = static_cast<char>( CXSLUnit::PVPRANK_RANK_ARRANGE );
	}
}
#endif SERV_2012_PVP_SEASON2
//}}

//{{ 2012 ���� ����2 ���� ��Ī �̺�Ʈ	- ��μ�
#ifdef SERV_2012_PVP_SEASON2_EVENT
void KGSUser::CheckPvpEvent()
{
	// 1�и���
	if( m_kTimer[TM_PVP_EVENT_CHECK].elapsed() < 60.0 )
		return;
	
	m_kTimer[TM_PVP_EVENT_CHECK].restart();

	CTime tCurr = CTime::GetCurrentTime();
	
	// �̺�Ʈ ���(�̺�Ʈ �ð��̶��)
	if( m_bNowPvpEventTime == true )
	{
		int iIndex = 0;
		std::wstring wstrTemp;
		bool bTemp = false;

		// �̺�Ʈ �ð��� ���µ� ������ �̺�Ʈ �ð��� �ƴϴ�? �׷� ���� ������ ���� �˷�����
		if( SiKGameSysVal()->IsPvpEventTime( tCurr, iIndex, wstrTemp, bTemp ) == false )
		{
			m_bNowPvpEventTime = false;

			// Ŭ�󿡰� ������ ����� �˷�����
			KEGS_PVP_EVENT_INFO_NOT kNot;
			kNot.m_bEventInfo = false;

			SendPacket( EGS_PVP_EVENT_INFO_NOT, kNot );
		}
	}
	else	// �̺�Ʈ ����� �ƴ϶��(�̺�Ʈ �ð��� �ƴ϶��)
	{
		int iIndex = 0;
		std::wstring wstrTemp;
		bool bTemp = false;

		// �̺�Ʈ �ð��� �ƴϾ��µ� ������ �̺�Ʈ �ð��̴�? �׷� ���� ������ �˷�����
		if( SiKGameSysVal()->IsPvpEventTime( tCurr, iIndex, wstrTemp, bTemp ) == true )
		{
			m_bNowPvpEventTime = true;
			m_bIsPvpEventUser = true;

			// Ŭ�󿡰� ������ ����� �˷�����
			KEGS_PVP_EVENT_INFO_NOT kNot;
			kNot.m_bEventInfo = true;

			SendPacket( EGS_PVP_EVENT_INFO_NOT, kNot );
		}
	}
}
#endif SERV_2012_PVP_SEASON2_EVENT
//}}

//{{ 2012. 09. 25	�ڼ���	���� �̺�Ʈ
#ifdef SERV_EVENT_BINGO
void KGSUser::BingoEventDBWrite( void )
{
	KDBE_BINGO_EVENT_INFO_WRITE_REQ kPacket;
	if( m_kGSBingoEvent.GetBingoEventInfoForDB( DBE_BINGO_EVENT_INFO_WRITE_ACK, kPacket ) == KGSBingoEvent::BS_OK )
	{
		SendToAccountDB( DBE_BINGO_EVENT_INFO_WRITE_REQ, kPacket );
	}
}
#endif SERV_EVENT_BINGO
//}}

#ifdef SERV_NEW_EVENT_TYPES
void KGSUser::CheckMaxLevel()
{
	int iCurrentEventTickCount = SiKGameEventManager()->GetAppliedEventTick();

	if( iCurrentEventTickCount != m_iMaxLevelTickCount )
	{
		//���� ��������� ������.
		KEGS_UPDATE_MAX_LEVEL_NOT kPacket;
		kPacket.m_iMaxLevel = max( SiKGameEventManager()->GetMaxLevel(), SiKGameSysVal()->GetLimitsLevel() );

		SendPacket( EGS_UPDATE_MAX_LEVEL_NOT, kPacket );

		m_iMaxLevelTickCount = iCurrentEventTickCount;
	}
}
void KGSUser::CheckEnableEventDungeon()
{
	int iCurrentEventTickCount = SiKGameEventManager()->GetAppliedEventTick();

	if( iCurrentEventTickCount != m_iActiveDungeonTickCount )
	{
		//���� ��������� ������.
		KEGS_UPDATE_DUNGEON_STATUS_NOT kPacket;
		SiCXSLDungeonManager()->GetEventDungeons( kPacket.m_mapEnableDungeon );
		SiKGameEventManager()->CheckEnableEventDungeon( kPacket.m_mapEnableDungeon );

		SendPacket( EGS_UPDATE_DUNGEON_STATUS_NOT, kPacket );

		m_iActiveDungeonTickCount = iCurrentEventTickCount;
	}
}
#endif SERV_NEW_EVENT_TYPES

#ifdef SERV_CODE_EVENT
void KGSUser::CheckEnableCodeEvent()
{
	int iCurrentEventTickCount = SiKGameEventManager()->GetAppliedEventTick();

	if( iCurrentEventTickCount != m_iCodeEventTickCount )
	{
		// �ڵ� ��������� ������.
		KEGS_UPDATE_CODE_EVENT_NOT kPacket;
		SiKGameEventManager()->CheckEnableCode( kPacket.m_mapEnableCodeEnum );

		SendPacket( EGS_UPDATE_CODE_EVENT_NOT, kPacket );

		m_iCodeEventTickCount = iCurrentEventTickCount;
	}
}
#endif SERV_CODE_EVENT

//{{ 2012. 12. 10  ĳ���� ���� ��Ŷ ���� - ��μ�
#ifdef SERV_SELECT_UNIT_PACKET_DIVISION
void KGSUser::SendSelectUnitACK( IN const KDBE_SELECT_UNIT_ACK& kPacket )
{
	//////////////////////////////////////////////////////////////////////////
	KEGS_SELECT_UNIT_1_NOT kNot_1;
	kNot_1.m_iOK							= kPacket.m_iOK;
	kNot_1.m_kUnitInfo						= kPacket.m_kUnitInfo;
	kNot_1.m_mapInventorySlotSize			= kPacket.m_mapInventorySlotSize;
	kNot_1.m_mapItem						= kPacket.m_mapItem;
	kNot_1.m_iNumResurrectionStone			= kPacket.m_iNumResurrectionStone;

	//{{ 2009. 10. 14  ������	���ʺ�Ȱ��
#ifdef AP_RESTONE
	kNot_1.m_iNumAutoPaymentResStone		= kPacket.m_iNumAutoPaymentResStone;
	kNot_1.m_wstrAutoPayResStoneLastDate	= kPacket.m_wstrAutoPayResStoneLastDate;
#endif AP_RESTONE
	//}}

	//{{ 2011. 09. 23	������	�α׿��� ���� EDüũ
#ifdef SERV_LOGOUT_ED_CHECK
	kNot_1.m_iRealDataED					= kPacket.m_iRealDataED;
#endif SERV_LOGOUT_ED_CHECK
	//}}

	//{{ 2011. 01. 18	������	ĳ���� ī��Ʈ ����
#ifdef SERV_CHAR_LOG
	kNot_1.m_wstrUnitCreateDate				= kPacket.m_wstrUnitCreateDate;
#endif SERV_CHAR_LOG
	//}}

#ifdef SERV_NEW_UNIT_TRADE_LIMIT
	kNot_1.m_iNewUnitTradeBlockUnitClass	= SiKGameSysVal()->GetNewUnitTradeBlockUnitClass();
	kNot_1.m_bIsNewUnitTradeBlock			= SiKGameSysVal()->IsNewUnitTradeBlock();
	kNot_1.m_iNewUnitTradeBlockDay			= SiKGameSysVal()->GetNewUnitTradeBlockDay();
#endif // SERV_NEW_UNIT_TRADE_LIMIT

	//{{ 2011. 03. 22	������	ĳ���� ù ���� �α�
#ifdef SERV_DAILY_CHAR_FIRST_SELECT
	kNot_1.m_wstrUnitLastLoginDate			= kPacket.m_wstrUnitLastLoginDate;
#endif SERV_DAILY_CHAR_FIRST_SELECT
	//}}

	kNot_1.m_vecSkillAcquired				= kPacket.m_vecSkillAcquired;		// ȹ���� ��ų ����Ʈ
	kNot_1.m_vecSkillUnsealed				= kPacket.m_vecSkillUnsealed;		// ���������� ��ų ����Ʈ

	//{{ 2010. 03. 13  ������	����� ��Ʈ
#ifdef SERV_SKILL_NOTE
	kNot_1.m_cSkillNoteMaxPageNum			= kPacket.m_cSkillNoteMaxPageNum;
	kNot_1.m_mapSkillNote					= kPacket.m_mapSkillNote;			// [key:SlotID, value:MemoID]
#endif SERV_SKILL_NOTE
	//}}
	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
	kNot_1.m_kRelationshipInfo				= kPacket.m_kRelationshipInfo;

	for( int iSkillSlotID = 0 ; iSkillSlotID < KUnitSkillData::EQUIPPED_SKILL_SLOT_COUNT ; ++iSkillSlotID )
	{
		if( kNot_1.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkill[iSkillSlotID].m_iSkillID == CXSLSkillTree::SI_ETC_WS_COMMON_LOVE )
		{
			kNot_1.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkill[iSkillSlotID].m_cSkillLevel = kPacket.m_kRelationshipInfo.m_cRelationshipType;
		}
	}
	for( int iSkillSlotID = 0 ; iSkillSlotID < KUnitSkillData::EQUIPPED_SKILL_SLOT_COUNT ; ++iSkillSlotID )
	{
		if( kNot_1.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkillSlotB[iSkillSlotID].m_iSkillID == CXSLSkillTree::SI_ETC_WS_COMMON_LOVE )
		{
			kNot_1.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkillSlotB[iSkillSlotID].m_cSkillLevel = kPacket.m_kRelationshipInfo.m_cRelationshipType;
		}
	}
#endif SERV_RELATIONSHIP_SYSTEM
	//}

	SendPacket( EGS_SELECT_UNIT_1_NOT, kNot_1 );

	//////////////////////////////////////////////////////////////////////////
	KEGS_SELECT_UNIT_2_NOT kNot_2;
	kNot_2.m_iOK					= kPacket.m_iOK;
	kNot_2.m_vecQuest				= kPacket.m_vecQuest;
	kNot_2.m_vecCompletQuest		= kPacket.m_vecCompletQuest;

	//{{ 2011. 08. 29	��μ�       ���� ���� ����Ʈ
#ifdef SERV_RANDOM_DAY_QUEST
	kNot_2.m_mapRandomQuestList		= kPacket.m_mapRandomQuestList;
#endif SERV_RANDOM_DAY_QUEST
	//}}

	SendPacket( EGS_SELECT_UNIT_2_NOT, kNot_2 );

	//////////////////////////////////////////////////////////////////////////
	KEGS_SELECT_UNIT_3_NOT kNot_3;
	kNot_3.m_iOK							= kPacket.m_iOK;
	//{{ 2008. 10. 6  ������	Ÿ��Ʋ
	kNot_3.m_vecMission						= kPacket.m_vecMission;
	kNot_3.m_vecTitle						= kPacket.m_vecTitle;

	//{{ 2010. 7. 21  ������	�� �ý���
#ifdef SERV_PET_SYSTEM
	kNot_3.m_iSummonedPetUID				= kPacket.m_iSummonedPetUID;
	kNot_3.m_vecPetList						= kPacket.m_vecPetList;
	kNot_3.m_vecCanEvolutionPetList			= kPacket.m_vecCanEvolutionPetList;
	kNot_3.m_vecNeverSummonPetList			= kPacket.m_vecNeverSummonPetList;
#endif SERV_PET_SYSTEM
	//}}
	//{{ 2010. 8. 2	������	�� �ý���
#ifdef SERV_PET_SYSTEM
	kNot_3.m_mapPetItem						= kPacket.m_mapPetItem; // ���� ����
#endif SERV_PET_SYSTEM
	//}}

	SendPacket( EGS_SELECT_UNIT_3_NOT, kNot_3 );

	//////////////////////////////////////////////////////////////////////////
	KEGS_SELECT_UNIT_4_NOT kNot_4;
	kNot_4.m_iOK							= kPacket.m_iOK;
	kNot_4.m_vecChatBlackList				= kPacket.m_vecChatBlackList;
	kNot_4.m_kDenyOptions					= kPacket.m_kDenyOptions;

	//{{ 2008. 3. 31  ������  ��õ��
	kNot_4.m_bIsRecommend					= kPacket.m_bIsRecommend;
	kNot_4.m_iRecommendUnitUID				= kPacket.m_iRecommendUnitUID;
	//080405. hoons. ��������Ʈ ������
	kNot_4.m_vecTutorialDBUnitInfo			= kPacket.m_vecTutorialDBUnitInfo;
	//{{ 2008. 5. 19  ������  ���� �ð� �̺�Ʈ
	kNot_4.m_vecConnectTimeEvent			= kPacket.m_vecConnectTimeEvent;
	//{{ 2009. 12. 7  ������	�����ð��̺�Ʈ
#ifdef CUMULATIVE_TIME_EVENT
	kNot_4.m_vecCumulativeTimeEvent			= kPacket.m_vecCumulativeTimeEvent;
#endif CUMULATIVE_TIME_EVENT
	//}}
	//{{ 2009. 7. 7  ������		��ŷ����
	kNot_4.m_vecHenirRanking				= kPacket.m_vecHenirRanking;

	kNot_4.m_kMessengerInfo					= kPacket.m_kMessengerInfo;
	//{{ 2012. 02. 21	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	kNot_4.m_kGamePlayStatus				= kPacket.m_kGamePlayStatus;
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}

#ifdef SERV_RECRUIT_EVENT_BASE
	kNot_4.m_vecRecruiterUnitInfo			= kPacket.m_vecRecruiterUnitInfo;
	kNot_4.m_vecRecruitUnitInfo				= kPacket.m_vecRecruitUnitInfo;
#endif SERV_RECRUIT_EVENT_BASE

	SendPacket( EGS_SELECT_UNIT_4_NOT, kNot_4 );

	//////////////////////////////////////////////////////////////////////////
	KEGS_SELECT_UNIT_5_NOT kNot_5;
	kNot_5.m_iOK							= kPacket.m_iOK;
	//{{ 2008. 5. 7  ������  ���
	kNot_5.m_bIsSpiritUpdated				= kPacket.m_bIsSpiritUpdated;
	//{{ 2011. 01. 17	������	ĳ���� ī��Ʈ ����
#ifdef SERV_CHAR_LOG
	kNot_5.m_mapCharGameCount				= kPacket.m_mapCharGameCount;
#endif SERV_CHAR_LOG
	//}}
	//{{ 2011. 04. 18	������	�븮 ����
#ifdef SERV_PSHOP_AGENCY
	kNot_5.m_kPShopAgencyInfo				= kPacket.m_kPShopAgencyInfo;
#endif SERV_PSHOP_AGENCY
	//}}
	//{{ 2011. 08. 12   ��μ�      ��ϸ� ���� 
#ifdef SERV_NEW_HENIR_TEST
	kNot_5.m_PacketHenirRewardCount			= kPacket.m_PacketHenirRewardCount;
#endif SERV_NEW_HENIR_TEST
	//}}
	//{{ 2011. 10. 13	������	������ ������
#ifdef SERV_CONTENT_MANAGER
	kNot_5.m_bCashShopOpen					= kPacket.m_bCashShopOpen;
#endif SERV_CONTENT_MANAGER
	//}}
	//{{ 2012. 04. 12	�ڼ���	( ���� ���� ǥ�� )
#ifdef SERV_EVENT_RETURN_USER_MARK_SCRIPT
	kNot_5.m_vecCriterionEvent				= kPacket.m_vecCriterionEvent;
#endif SERV_EVENT_RETURN_USER_MARK_SCRIPT
	//}}
#ifdef SERV_EVENT_MONEY	// ��μ� // ���볯¥: 2013-07-04
	kNot_5.m_iEventMoney					= kPacket.m_iEventMoney;
#endif // SERV_EVENT_MONEY
	//{{ 2012 ���� ����2 ���� ��Ī �̺�Ʈ	- ��μ�
#ifdef SERV_2012_PVP_SEASON2_EVENT
	kNot_5.m_iPvpEventIndex					= kPacket.m_iPvpEventIndex;
	kNot_5.m_wstrLastPvpEventDate			= kPacket.m_wstrLastPvpEventDate;
#endif SERV_2012_PVP_SEASON2_EVENT
	//}}
	//{{ 2012. 10. 10	�ڼ���	���� �̺�Ʈ
#ifdef SERV_EVENT_BINGO
	kNot_5.m_kBingoEvent					= kPacket.m_kBingoEvent;
#endif SERV_EVENT_BINGO
	//}}
	//{{ 2012. 10. 29	�ڼ���	�������� �����
#ifdef SERV_ELIOS_INVESTIGATIONS
	kNot_5.m_bEliosInvestigationsReward		= kPacket.m_bEliosInvestigationsReward;
#endif SERV_ELIOS_INVESTIGATIONS
	//}}
	//{{ 2012. 12. 20	������	�ƶ� ù ���� Ʃ�丮��
#ifdef SERV_ARA_FIRST_SELECT_TUTORIAL
	kNot_5.m_bFirstSelect					= kPacket.m_bFirstSelect;
#endif SERV_ARA_FIRST_SELECT_TUTORIAL
	//}}
#ifdef	SERV_LOCAL_RANKING_SYSTEM // ���볯¥: 2013-03-31
	kNot_5.m_bLocalRankingUser				= ( kPacket.m_wstrLocalRankingUserInfoUpdated.empty() == false );
#endif	// SERV_LOCAL_RANKING_SYSTEM
#ifdef SERV_GATE_OF_DARKNESS_SUPPORT_EVENT
	kNot_5.m_iGateOfDarknessSupportEventTime = kPacket.m_iGateOfDarknessSupportEventTime;
#endif SERV_GATE_OF_DARKNESS_SUPPORT_EVENT

	SendPacket( EGS_SELECT_UNIT_5_NOT, kNot_5 );

	//////////////////////////////////////////////////////////////////////////
	KEGS_SELECT_UNIT_ACK kACK;
	kACK.m_iOK = kPacket.m_iOK;
	SendPacket( EGS_SELECT_UNIT_ACK, kACK );
}
#endif SERV_SELECT_UNIT_PACKET_DIVISION
//}}

//{{ 2012. 12. 14  ���� �̼� �ý��� ( �������� ����Ʈ ) - ��μ�
#ifdef SERV_ACCOUNT_MISSION_SYSTEM
void KGSUser::SendUpdateAccountQuestInfo()
{
	// ���� ����Ʈ ���� update 
	// ���� ���� ������ ������Ʈ �ϸ� �ȴ�

	KDBE_ACCOUNT_QUEST_UPDATE_NOT kNot;
	kNot.m_iUserUID = GetUID();
	m_kUserAccountQuestManager.GetAccountQuesting( kNot.m_vecAccountQuesting );

	if( kNot.m_vecAccountQuesting.empty() == true )
		return;

	

	SendToAccountDB( DBE_ACCOUNT_QUEST_UPDATE_NOT, kNot );
}
#endif SERV_ACCOUNT_MISSION_SYSTEM
//}}

//{{ 2013. 3. 11	�ڼ���	 ���� ��ŷ �ý���
#ifdef SERV_LOCAL_RANKING_SYSTEM
void KGSUser::IncreasedLocalRankingPoint( IN const int iSpirit, IN const int iAP, IN const bool bForce /*= false */)
{
	if( GetLevel() < 10 )
	{
		// 10���� ���� ������ ���� ��ŷ �ý��ۿ��� ����
		return;
	}

	// �����Ͱ� �ʱ�ȭ �Ǿ��ִ°�?
	if( m_kUserLocalRankingInfo.InitCheck( GetUID(), GetCharUID() ) == false )
	{
		// DB���� �����͸� �о�´�.
		KDBE_GAME_LOCAL_RANKING_UNIT_INFO_READ_FOR_INCREASE_REQ kPacket;
		kPacket.m_iUserUID			= GetUID();
		kPacket.m_iUnitUID			= GetCharUID();

		kPacket.m_iIncreasedSpirit	= iSpirit;
		kPacket.m_iIncreasedAP		= iAP;
		kPacket.m_bForce			= bForce;
		SendToGameDB( DBE_GAME_LOCAL_RANKING_UNIT_INFO_READ_FOR_INCREASE_REQ, kPacket );
		return;
	}

	_IncreasedLocalRankingPoint( iSpirit, iAP, bForce );
}

void KGSUser::_IncreasedLocalRankingPoint( IN const int iSpirit, IN const int iAP, IN const bool bForce /*= false */)
{
	/*if( ( iSpirit < 0 ) || ( iAP < 0 ) )
	{
		START_LOG( cerr, L"ELP ����ġ �̻�" )
			<< BUILD_LOG( GetCharName() )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOG( iSpirit )
			<< BUILD_LOG( iAP )
			<< END_LOG;

		KDBE_SEND_PHONE_MSG_NOT kPacketNot;
		kPacketNot.m_wstrSMSMessage = boost::str( boost::wformat( L"ELP ����ġ �̻� ���� �߰�! : %d, %d, %d" ) % GetCharUID() % iSpirit % iAP );
		SiKSMSPhoneNumberManager()->GetPhoneNumberList( KSMSPhoneNumberManager::FS_LOCAL_RANKING_SYSTEM, kPacketNot.m_vecPhoneNum );
		if( kPacketNot.m_vecPhoneNum.empty() == false )
		{
			SendToSMSDB( DBE_SEND_PHONE_MSG_NOT, kPacketNot );
		}
	}*/

	m_kUserLocalRankingInfo.IncreasedUnitInfo( iSpirit, iAP );

	if( ( m_kUserLocalRankingInfo.UpdateTimeCheck() == true ) || ( bForce == true ) )
	{
		KELG_LOCAL_RANKING_CHANGED_RANKER_INFO_NOT kPacket;
		kPacket.m_iUID = m_kUserLocalRankingInfo.GetUID();
		m_kUserLocalRankingInfo.GetUserInfo( kPacket.m_kUserInfo );
		m_kUserLocalRankingInfo.GetUnitInfo( kPacket.m_kUnitInfo );

		if( m_kUserLocalRankingInfo.IsValidCategory() == true )
		{
			KEventPtr	spEvent( new KEvent );
			UidType		anTrace[2] = { GetUID(), -1 };
			spEvent->SetData( PI_LOGIN_SERVER, anTrace, ELG_LOCAL_RANKING_CHANGED_RANKER_INFO_CHECK_NOT, kPacket );
			SiKGSLocalRankingManager()->QueueingEvent( spEvent );
		}

		// ��� UID�� �����Ѵٸ�, ��� �Ŵ������Ե� ������
		kPacket.m_iUID = GetGuildUID();
		if( 0 < kPacket.m_iUID )
		{
			SendToLoginServer( ELG_LOCAL_RANKING_GUILD_CHANGE_INFO_NOT, kPacket );
		}
	}
}

void KGSUser::LocalRankingInquiryProcess( IN const UINT nViewPage
										, IN const char cMainTabIndex
										, IN const char cSubTabIndex
										, IN const byte byteFilter
#ifdef SERV_LOCAL_RANKING_SYSTEM_STATISTICS_LOG// �۾���¥: 2013-05-15	// �ڼ���
										, IN const bool bRankingButtonClick
#endif // SERV_LOCAL_RANKING_SYSTEM_STATISTICS_LOG
										)
{
	switch( cSubTabIndex )
	{
	case SEnum::LRSTC_LOCATION:
		if( m_kUserLocalRankingInfo.IsValidCategory() == true )
		{
			// ������ ����
			KLocalRankingInquiryData kInquiryData;
			kInquiryData.m_nViewPage		= nViewPage;
			kInquiryData.m_cMainTabIndex	= cMainTabIndex;
			kInquiryData.m_cSubTabIndex		= cSubTabIndex;
			kInquiryData.m_byteFilter		= byteFilter;

			m_kUserLocalRankingInfo.GetFilteredUserInfo( kInquiryData.m_kMyRankInfo.m_kUserInfo );
			m_kUserLocalRankingInfo.GetUnitInfo( kInquiryData.m_kMyRankInfo.m_kUnitInfo );

			if( byteFilter & SEnum::LRF_FRIEND )
			{
				std::map< std::wstring, UidType >::const_iterator it;
				for( it = m_mapFriendList.begin(); it != m_mapFriendList.end(); ++it )
				{
					kInquiryData.m_setFriendList.insert( it->second );
				}
			}

			// GSLocalRankingManager�� ������.
			KEventPtr	spEvent( new KEvent );
			UidType		anTrace[2] = { GetUID(), -1 };
			spEvent->SetData( PI_GS_SERVER, anTrace, EGS_LOCAL_RANKING_INQUIRY_REQ, kInquiryData );
			SiKGSLocalRankingManager()->QueueingEvent( spEvent );
		}
		else
		{
			KEGS_LOCAL_RANKING_INQUIRY_ACK kPacket;
			kPacket.m_iOK			= NetError::NET_OK;
			kPacket.m_nViewPage		= nViewPage;
			kPacket.m_cMainTabIndex	= cMainTabIndex;
			kPacket.m_cSubTabIndex	= cSubTabIndex;
			kPacket.m_byteFilter	= byteFilter;
			m_kUserLocalRankingInfo.GetFilteredUserInfo( kPacket.m_kMyRankInfo.m_kUserInfo );
			m_kUserLocalRankingInfo.GetUnitInfo( kPacket.m_kMyRankInfo.m_kUnitInfo );
			SendPacket( EGS_LOCAL_RANKING_INQUIRY_ACK, kPacket );
		}
		break;

	case SEnum::LRSTC_GUILD:
		if( 0 < GetGuildUID() )
		{
			// ������ ����
			KLocalRankingInquiryData kInquiryData;
			kInquiryData.m_nViewPage		= nViewPage;
			kInquiryData.m_cMainTabIndex	= cMainTabIndex;
			kInquiryData.m_cSubTabIndex		= cSubTabIndex;
			kInquiryData.m_byteFilter		= byteFilter;

			m_kUserLocalRankingInfo.GetFilteredUserInfo( kInquiryData.m_kMyRankInfo.m_kUserInfo );
			m_kUserLocalRankingInfo.GetUnitInfo( kInquiryData.m_kMyRankInfo.m_kUnitInfo );

			// ��� �Ŵ����� ������.
			kInquiryData.m_iGuildUID = GetGuildUID();
			if( byteFilter & SEnum::LRF_FRIEND )
			{
				std::map< std::wstring, UidType >::const_iterator it;
				for( it = m_mapFriendList.begin(); it != m_mapFriendList.end(); ++it )
				{
					kInquiryData.m_setFriendList.insert( it->second );
				}
			}
			SendToLoginServer( EGS_LOCAL_RANKING_INQUIRY_REQ, kInquiryData );
		}
		else
		{
			KEGS_LOCAL_RANKING_INQUIRY_ACK kPacket;
			kPacket.m_iOK			= NetError::NET_OK;
			kPacket.m_nViewPage		= nViewPage;
			kPacket.m_cMainTabIndex	= cMainTabIndex;
			kPacket.m_cSubTabIndex	= cSubTabIndex;
			kPacket.m_byteFilter	= byteFilter;
			m_kUserLocalRankingInfo.GetFilteredUserInfo( kPacket.m_kMyRankInfo.m_kUserInfo );
			m_kUserLocalRankingInfo.GetUnitInfo( kPacket.m_kMyRankInfo.m_kUnitInfo );
			SendPacket( EGS_LOCAL_RANKING_INQUIRY_ACK, kPacket );
		}
		break;

	default:
		// ���ǵ��� ���� �� ���³�? �ݺ� ���͸����� �ɷ�������
		break;
	}

#ifdef SERV_LOCAL_RANKING_SYSTEM_STATISTICS_LOG// �۾���¥: 2013-05-15	// �ڼ���
	if( bRankingButtonClick == true )
	{
		IncreaseLocalRankingButtonClickCount();
	}
#endif // SERV_LOCAL_RANKING_SYSTEM_STATISTICS_LOG
}
#endif SERV_LOCAL_RANKING_SYSTEM
//}}


//---------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------//


#ifdef SERV_NEW_UNIT_TRADE_LIMIT
void KGSUser::CheckTradeBlockUnit()
{
	if( GetStateID() > KGSFSM::S_SERVER_SELECT  &&  GetStateID() < KGSFSM::STATE_SENTINEL )
	{
		if( m_kUserAbuserManager.IsTradeBlockUnit() == false )
			return;

		CTime tCurTime = CTime::GetCurrentTime();
		if( SiKGameSysVal()->GetNewUnitTradeBlockDay() != 0 && tCurTime < m_tReleaseTradeBlockTime )
			return;

		if( SiKGameSysVal()->GetNewUnitTradeBlockUnitClass() != 0 && GetUnitClass() < SiKGameSysVal()->GetNewUnitTradeBlockUnitClass() )
			return;

		CStringW cwstrMessage;
		cwstrMessage.Format( L"%s;", GetCharName().c_str() );

		KEGS_NEW_CHARACTER_TRADE_BLOCK_MSG_NOT kPacket;
		kPacket.m_wstrNewCharTBMsg = static_cast<LPCTSTR>(cwstrMessage);
		SendPacket( EGS_NEW_CHARACTER_TRADE_BLOCK_MSG_NOT, kPacket );

		m_kUserAbuserManager.SetTradeBlockUnit( false );

		std::vector< int > vecDeleteBuff;
		DeactivateBuff( CXSLBuffManager::BTI_BUFF_TRADE_BLOCK, vecDeleteBuff );
		if( vecDeleteBuff.empty() == false )
		{
			// �α��� ���� ���� �Ŵ����� ������Ʈ
			KELG_UPDATE_USER_UNIT_BUFF_INFO_REQ kReq;
			kReq.m_iUnitUID = GetCharUID();
			kReq.m_vecDeActivateBuff = vecDeleteBuff;
			SendToLoginServer( ELG_UPDATE_USER_UNIT_BUFF_INFO_REQ, kReq );
		}
	}
}
#endif SERV_NEW_UNIT_TRADE_LIMIT

//{{ 2012. 09. 06	��ȫ��	�۷ι� �̼� �Ŵ���
#ifdef SERV_GLOBAL_MISSION_MANAGER
void KGSUser::CheckUpdateGlobalMission()
{
	//	ƽī��Ʈ�� ���ϰų� 60�ʸ��� �ѹ��� ������Ʈ
	if( SiKGSGlobalMissionManager()->GetTickCount() == m_iGlobalMissionTickCount )
		return;

	m_iGlobalMissionTickCount = SiKGSGlobalMissionManager()->GetTickCount();

	//	�����ޱ�
	KEGS_GLOBAL_MISSION_UPDATE_NOT kNot;
	SiKGSGlobalMissionManager()->GetGlobalMissionInfo( kNot );

	//	�������� ��Ŷ������
	SendPacket( EGS_GLOBAL_MISSION_UPDATE_NOT, kNot );
}
#endif SERV_GLOBAL_MISSION_MANAGER
//}} 2012. 09. 06	��ȫ��	�۷ι� �̼� �Ŵ���

#ifdef SERV_EVENT_RIDING_WITH_SUB_QUEST
int KGSUser::GetiRidingPetSummoned()
{
	// m_iRidingPetSummon �� 
	// 0 : ���̵��� �̼�ȯ, 1 : ĳ�ÿ� ���̵� ��, 2 : �̺�Ʈ�� ���̵� ��

	m_iRidingPetSummon = 0;

	// ���̵� �� �Ŵ����� �����ϸ�
	KUserRidingPetPtr spRidingPet = m_kUserRidingPetManager.GetRidingPetSummoned();
	if( spRidingPet == NULL )
	{
		return m_iRidingPetSummon;
	}

	// ���̵� �� �ı���¥�� �Ǵ�����
	KRidingPetInfo kInfoRP;
	spRidingPet->GetData(kInfoRP);


	// ĳ�ÿ� ���̶��.
	if(kInfoRP.m_wstrDestroyDate.compare(L"") == 0 )
	{
		m_iRidingPetSummon = 1;
	}
	else
	{
		m_iRidingPetSummon = 2;
	}
	
	
	//START_LOG( clog2, L"�輮��_���̵��� �� Ÿ������!" )
	//	<<BUILD_LOG( m_iRidingPetSummon )
	//	<<BUILD_LOG( kInfoRP.m_wstrDestroyDate.c_str() )
	//	<<END_LOG;

	return m_iRidingPetSummon;
}

bool KGSUser::GetbIsEventRidingPetQuest()
{
	m_bIsEventRidingPetQuest = false;

	// ����Ʈ�� �����ϰ� ������ (74590)
	if(m_kUserQuestManager.IsQuest(_CONST_AEVENT_RIDING_WITH_SUB_QUEST::iHasEventQuestID) )
	{
		m_bIsEventRidingPetQuest = true;
	}


	return m_bIsEventRidingPetQuest;
}

#endif //SERV_EVENT_RIDING_WITH_SUB_QUEST



#ifdef SERV_NPROTECT_CS_AUTH_30
void KGSUser::CheckCSAuth3()
{
	//if( KSimLayer::GetKObj()->CheckCommonFlag( KSimLayer::CommonFlag::CF_NPGG ) == false )
	//	return;

	if( GetStateID() > KGSFSM::S_SERVER_SELECT  &&  GetStateID() < KGSFSM::STATE_SENTINEL )
	{
		if( m_kTimer[TM_NPROTECT_CS_AUTH_30].elapsed() < 300.0 )	// 5��
			return;

		m_kTimer[TM_NPROTECT_CS_AUTH_30].restart();

		UINT32 uReturnedPackedSize = 0;
		UINT32 uReturn = m_CSAuth3.Get( &uReturnedPackedSize );

		if( uReturn >= 3000 )
		{
			// ���� ���� ����

			START_LOG( cerr, L"nProtect CS ����3 üũ ����" )
				<< BUILD_LOG( GetName() )
				<< BUILD_LOG( GetCharName() )
				<< BUILD_LOG( uReturn )
				<< END_LOG;

			SetDisconnectReason( KStatistics::eSIColDR_nProtect_Error );
			ReserveDestroy();

			return;
		}

		if( uReturnedPackedSize > 0)
		{
			KEGS_NPROTECT_CSAUTH3_CHECK_NOT KPacketNot; 
			for( UINT32 i = 0; i < uReturnedPackedSize; i++)
			{
				KPacketNot.m_deqCSAuth3.push_back(m_CSAuth3.packet[i]);
			}

			SendPacket( EGS_NPROTECT_CSAUTH3_CHECK_NOT, KPacketNot );
		}
	}
}
#endif SERV_NPROTECT_CS_AUTH_30

//{{ 2012.02.20 ��ȿ��	ĳ���� ���� ���μ��� ���� (���� ��� �Ⱓ ����)
#ifdef SERV_UNIT_WAIT_DELETE
bool KGSUser::LastDateSort(const KUnitInfo& refLUnitInfo, const KUnitInfo& refRUnitInfo)
{
	CTime tLTime;
	CTime tRTime;


	// ������ �ȵǴ� ���� �ֽ����� ��
	if( KncUtil::ConvertStringToCTime(refLUnitInfo.m_wstrLastDate, tLTime) == false )
	{
		return true;
	}
	else if( KncUtil::ConvertStringToCTime(refRUnitInfo.m_wstrLastDate, tRTime) == false )
	{
		// �������� �ֽ��̸� return false
		return false;
	}


	// Left�� �ֽ��̸� True
	if( tLTime > tRTime )
	{
		return true;
	}

	return false;   
}
#endif SERV_UNIT_WAIT_DELETE
//}}

//---------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------//

//{{ 2013. 03. 18	 ������ �̺�Ʈ_���κ��� ���� - ��μ�
#ifdef SERV_APRIL_FOOLS_DAY
void KGSUser::CheckRainbowBuff()
{
	// ���� �ֱ⸶�� ó�� �Ǿ�� �Ѵ�.
	if( m_kTimer[TM_APRIL_FOOLS_DAY_RAINBOW_BUFF].elapsed() < 60.f )
		return;

	// Ÿ�̸� �ʱ�ȭ
	m_kTimer[TM_APRIL_FOOLS_DAY_RAINBOW_BUFF].restart();

	// ���� �ð��� ���� �д�.
	CTime tCurr = CTime::GetCurrentTime();

	// ���� ������ ���� �д�.
	int iDayOfWeek = tCurr.GetDayOfWeek();

	// �߸��� �����̸� ����ó��
	if( iDayOfWeek < 1 || iDayOfWeek > 7 )
	{
		START_LOG( cerr, L"�߸��� ���� ���� �Դϴ�." )
			<< BUILD_LOG( iDayOfWeek )
			<< END_LOG;
		return;
	}

	// Ȱ��ȭ, ��Ȱ��ȭ ���� ���
	int iBuffID = CXSLBuffManager::BTI_BUFF_APRIL_FOOLSDAY;
	std::vector< KBuffInfo > vecActiveBuff;
	std::vector< int > vecDeleteBuff;

	// ���� �̶��(��, ȭ, ��, ��, ��)
	if( iDayOfWeek > 1 && iDayOfWeek < 7 )
	{
		// ���κ��� ������ �ִ�
		if( m_kUserBuffManager.IsBuffActivated( iBuffID ) == true )
		{
			// �ð��� �Ǿ����� �����Ѵ�.
			if( tCurr.GetHour() >= 22 )
			{
				DeactivateBuff( iBuffID, vecDeleteBuff );
			}
		}
		else	// ���κ��� ������ ����
		{
			// �ð��� �Ǿ����� �Ѿ��Ѵ�.
			if( tCurr.GetHour() >= 16 )
			{
				ActivateBuff( iBuffID, vecActiveBuff );
			}
		}
	}
	else	// �ָ� �̶��(��, ��)
	{
		// ���κ��� ������ �ִ�
		if( m_kUserBuffManager.IsBuffActivated( CXSLBuffManager::BTI_BUFF_APRIL_FOOLSDAY ) == true )
		{
			// �ð��� �Ǿ����� �����Ѵ�.
			if( tCurr.GetHour() >= 18 )
			{
				DeactivateBuff( iBuffID, vecDeleteBuff );
			}
		}
		else	// ���κ��� ������ ����
		{
			// �ð��� �Ǿ����� �Ѿ��Ѵ�.
			if( tCurr.GetHour() >= 12 )
			{
				ActivateBuff( iBuffID, vecActiveBuff );
			}
		}
	}

	// ����ų� �߰��Ǵ� ������ �ִٸ�
	if( vecActiveBuff.empty() == false || vecDeleteBuff.empty() == false )
	{
		// �α��� ���� ���� �Ŵ����� ������Ʈ
		KELG_UPDATE_USER_UNIT_BUFF_INFO_REQ kReq;
		kReq.m_iUnitUID = GetCharUID();
		kReq.m_vecActivateBuff = vecActiveBuff;
		kReq.m_vecDeActivateBuff = vecDeleteBuff;
		SendToLoginServer( ELG_UPDATE_USER_UNIT_BUFF_INFO_REQ, kReq );
	}
}
#endif SERV_APRIL_FOOLS_DAY
//}

#ifdef SERV_ACTIVE_KOG_GAME_PERFORMANCE_CHECK
void KGSUser::_CheckSystemInfoStatistics()
{
	//���� ����â�� ��쿡�� ����PC����� �ѹ��� DB ��� ��û�Ѵ�.
	if( GetStateID() == KGSFSM::S_SERVER_SELECT )
	{
		if( !m_bSendDBSystemInfoStatistics )
		{
			// StatisticsDB�� ���
			SendToLogDB( ELOG_STAT_SYSTEM_INFO, m_kUserStatistics.GetSystemInfoStatistics() );

			m_bSendDBSystemInfoStatistics = true;
		}
	}
}

void KGSUser::_CheckKOGGamePerformanceStatistics()
{
	//���� ����â �̻��� ��쿡��...
	if( GetStateID() > KGSFSM::S_SERVER_SELECT )
	{
		//�������� ������ ���� ���� ������ �����Ѵٸ� DB ��� ��û�Ѵ�.
		if( m_kUserStatistics.GetKOGGamePerformanceStatistics_Village().m_iCharUID != -1 )
		{
			// StatisticsDB�� ���
			SendToLogDB( ELOG_STAT_KOG_GAME_PERFORMANCE_VILLAGE, m_kUserStatistics.GetKOGGamePerformanceStatistics_Village() );

			//��û���� ����
			m_kUserStatistics.SetKOGGamePerformanceStatistics_Village( KEGS_KOG_GAME_PERFORMANCE_VILLAGE_LOG() );
		}

		//�ʵ忡�� ������ ���� ���� ������ �����Ѵٸ� DB ��� ��û�Ѵ�.
		if( m_kUserStatistics.GetKOGGamePerformanceStatistics_Field().m_iCharUID != -1 )
		{
			// StatisticsDB�� ���
			SendToLogDB( ELOG_STAT_KOG_GAME_PERFORMANCE_FIELD, m_kUserStatistics.GetKOGGamePerformanceStatistics_Field() );

			//��û���� ����
			m_kUserStatistics.SetKOGGamePerformanceStatistics_Field( KEGS_KOG_GAME_PERFORMANCE_FIELD_LOG() );
		}

		//�������� ������ ���� ���� ������ �����Ѵٸ� DB ��� ��û�Ѵ�.
		if( m_kUserStatistics.GetKOGGamePerformanceStatistics_Dungeon().m_iCharUID != -1 )
		{
			// StatisticsDB�� ���
			SendToLogDB( ELOG_STAT_KOG_GAME_PERFORMANCE_DUNGEON, m_kUserStatistics.GetKOGGamePerformanceStatistics_Dungeon() );

			//��û���� ����
			m_kUserStatistics.SetKOGGamePerformanceStatistics_Dungeon( KEGS_KOG_GAME_PERFORMANCE_DUNGEON_LOG() );
		}

		//�������� ������ ���� ���� ������ �����Ѵٸ� DB ��� ��û�Ѵ�.
		if( m_kUserStatistics.GetKOGGamePerformanceStatistics_PvP().m_iCharUID != -1 )
		{
			// StatisticsDB�� ���
			SendToLogDB( ELOG_STAT_KOG_GAME_PERFORMANCE_PVP, m_kUserStatistics.GetKOGGamePerformanceStatistics_PvP() );

			//��û���� ����
			m_kUserStatistics.SetKOGGamePerformanceStatistics_PvP( KEGS_KOG_GAME_PERFORMANCE_PVP_LOG() );
		}
	}
}
#endif//SERV_ACTIVE_KOG_GAME_PERFORMANCE_CHECK

#ifdef	SERV_HERO_PVP_MANAGE_LIST // ���볯¥: 2013-04-04
void KGSUser::SetRoomListID( IN const UINT uiRoomListID )
{
	if( m_bHeroPvpUser == true )
	{
		const UINT uiTournamentPVPID = SiKRoomListManager()->GetTournamentPVPID();
		if( GetRoomListID() == uiTournamentPVPID )
		{
			if( uiRoomListID != uiTournamentPVPID )
			{
				// ��ȸ ä�� ���� ����
				KEGB_ADMIN_HERO_PVP_USER_TOURNAMENT_CONNECTION_NOT kPacket;
				kPacket.m_iServerGroupID	= KBaseServer::GetKObj()->GetServerGroupID();
				kPacket.m_wstrNickName		= GetCharName();
				kPacket.m_bConnect			= false;
				SendToGlobalServer( EGB_ADMIN_HERO_PVP_USER_TOURNAMENT_CONNECTION_NOT, kPacket );
			}
		}
		else
		{
			if( uiRoomListID == uiTournamentPVPID )
			{
				// ��ȸ ä�� ����
				KEGB_ADMIN_HERO_PVP_USER_TOURNAMENT_CONNECTION_NOT kPacket;
				kPacket.m_iServerGroupID	= KBaseServer::GetKObj()->GetServerGroupID();
				kPacket.m_wstrNickName		= GetCharName();
				kPacket.m_bConnect			= true;
				SendToGlobalServer( EGB_ADMIN_HERO_PVP_USER_TOURNAMENT_CONNECTION_NOT, kPacket );
			}
		}
	}

	m_uiRoomListID = uiRoomListID;
}
#endif	// SERV_HERO_PVP_MANAGE_LIST

#ifdef SERV_SKILL_POINT_CORRECTION
bool KGSUser::CheckSkillPointCorrection( OUT int& iCorrectionSP )
{
	iCorrectionSP = 0;
	int iCorrectionCSP = 0;

	if( GetCharUID() == 0 )
	{
		return false;
	}

	int iUsedSPoint = 0;
	int iUsedCSPoint = 0;
	m_kSkillTree.CalcUsedSPointAndCSPoint( iUsedSPoint, iUsedCSPoint );

	if( m_kSkillTree.IsCashSkillPointExpired() == false )
	{
		// �׳�ý� ����ϰ� �ִ� ����
#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013
		int iRetrievedSPoint = 0;
		SiCXSLSkillTree()->GetCalcInitSkillPoint( GetLevel(), iRetrievedSPoint );
		const int iCheckSP = iRetrievedSPoint + m_kSkillTree.GetMaxCSPoint();
#else
		const int iCheckSP = GetLevel() * 2 + m_kSkillTree.GetMaxCSPoint();
#endif //SERV_UPGRADE_SKILL_SYSTEM_2013
		const int iCurSP = m_iSPoint + iUsedSPoint + iUsedCSPoint + m_kSkillTree.GetCSPoint();

		if( iCheckSP != iCurSP )
		{
			// CSP�� ������ �ִ� ĳ������ ���� ���� SP�� �ش� ĳ������ �� SP�� �ѷ� �񱳴� ���������� SP�� CSP�� ���� ���ؼ��� �ȵ�!
			// ������ �׳�ý� ����ϰ� �ִ� ���¿��� ����� SP�� ��� iUsedCSPoint�� �ջ�Ǳ� ������.
			/*
#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013
			int iRetrievedSPoint = 0;
			SiCXSLSkillTree()->GetCalcInitSkillPoint( GetLevel(), iRetrievedSPoint );
			const int iCheckCorrectSPoint = iRetrievedSPoint;
#else
			const int iCheckCorrectSPoint = GetLevel() * 2;
#endif //SERV_UPGRADE_SKILL_SYSTEM_2013
			const int iCheckCurrentSPoint = m_iSPoint + iUsedSPoint;

			const int iCheckCorrectCSPoint = m_kSkillTree.GetMaxCSPoint();
			const int iCheckCurrentCSPoint = iUsedCSPoint + m_kSkillTree.GetCSPoint();

			if ( iCheckCurrentSPoint != iCheckCorrectSPoint )
			{
				iCorrectionSP = iCheckCorrectSPoint - iCheckCurrentSPoint;
			}

			if ( iCheckCorrectCSPoint != iCheckCurrentCSPoint ) // 2013.06.14 lygan_������ // ĳ�� ��ų ����Ʈ�� �̻��ϸ� ���� �ؾ� �ϴ��� �𸣰����� �켱 �Ѵ�.
			{
				iCorrectionCSP = iCheckCorrectCSPoint - iCheckCurrentCSPoint;
			}
			*/

			// ���� ������ �� ����( �̿ϼ� )
			/*
			if( iUsedCSPoint >= m_kSkillTree.GetMaxCSPoint() )
			{
				iCorrectionSP = ( iUsedCSPoint - m_kSkillTree.GetMaxCSPoint() ) - ( iRetrievedSPoint - ( m_iSPoint + iUsedSPoint ) );
			}
			else
			{
				iCorrectionSP = iRetrievedSPoint - ( m_iSPoint + iUsedSPoint + iUsedCSPoint );
			}
			*/			

			iCorrectionSP = iCheckSP - iCurSP;

			START_LOG( cerr, L"�׳�ý� ����ϰ� �ִ� ����" )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOG( GetCharName() )
				<< BUILD_LOGc( GetLevel() )
				<< BUILD_LOG( iRetrievedSPoint )
				<< BUILD_LOG( m_iSPoint )
				<< BUILD_LOG( iUsedSPoint )
				<< BUILD_LOG( iUsedCSPoint )
				<< BUILD_LOG( m_kSkillTree.GetCSPoint() )
				<< BUILD_LOG( m_kSkillTree.GetMaxCSPoint() )
				<< BUILD_LOG( iCheckSP )
				<< BUILD_LOG( iCurSP )
				<< BUILD_LOG( iCorrectionSP )
				<< END_LOG;

			return true;
		}
	}
	else
	{
		// �׳�ý� ����ϰ� ���� ���� ����
		if( iUsedCSPoint != 0  ||  m_kSkillTree.GetCSPoint() != 0 )
		{
			START_LOG( cerr, L"CSP ���Ⱓ�� ����Ǿ��µ� CSP�� �����ִ�" )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOG( GetCharName() )
				<< BUILD_LOGc( GetLevel() )
				<< BUILD_LOG( iUsedCSPoint )
				<< BUILD_LOG( m_kSkillTree.GetCSPoint() )
				<< BUILD_LOG( m_kSkillTree.GetMaxCSPoint() )
				<< END_LOG;

			return false;
		}
		else
		{
#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013
			int iRetrievedSPoint = 0;
			SiCXSLSkillTree()->GetCalcInitSkillPoint( GetLevel(), iRetrievedSPoint );
			const int iCheckSP = iRetrievedSPoint;
#else
			const int iCheckSP = GetLevel() * 2;
#endif //SERV_UPGRADE_SKILL_SYSTEM_2013
			const int iCurSP = m_iSPoint + iUsedSPoint;

			if( iCheckSP != iCurSP )
			{
				iCorrectionSP = iCheckSP - iCurSP;

				START_LOG( cerr, L"�׳�ý� ����ϰ� ���� ���� ����" )
					<< BUILD_LOG( GetCharUID() )
					<< BUILD_LOG( GetCharName() )
					<< BUILD_LOGc( GetLevel() )
					<< BUILD_LOG( iRetrievedSPoint )
					<< BUILD_LOG( m_iSPoint )
					<< BUILD_LOG( iUsedSPoint )
					<< BUILD_LOG( iCheckSP )
					<< BUILD_LOG( iCurSP )
					<< BUILD_LOG( iCorrectionSP )
					<< END_LOG;
				
				return true;
			}
		}
	}

	return false;
}
#endif //SERV_SKILL_POINT_CORRECTION

#ifdef	SERV_LOCAL_RANKING_SYSTEM // ���볯¥: 2013-04-05
void KGSUser::GetLocalRankingWatchUnitInfo( OUT KLocalRankingUserInfo& kUserInfo, OUT KLocalRankingUnitInfo& kUnitInfo ) const
{
	// ���� ����
	m_kUserLocalRankingInfo.GetFilteredUserInfo( kUserInfo );
	
	// ���� ����
	kUnitInfo.m_wstrNickName	= GetCharName();
	kUnitInfo.m_ucLevel			= GetLevel();
	kUnitInfo.m_cUnitClass		= GetUnitClass();
}
#endif	// SERV_LOCAL_RANKING_SYSTEM

#ifdef	SERV_RIDING_PET_SYSTM// ���볯¥: 2013-04-21
void KGSUser::CheckRidingPetProcess( void )
{
	// ����� �� ����Ʈ ���
	std::vector<UidType> vecRidingPetUID;
	if( m_kUserRidingPetManager.GetExpirationPetUID( vecRidingPetUID ) != KUserRidingPetManager::RIDING_PET_SUCCEED )
	{
		return;
	}

	if( vecRidingPetUID.empty() == true )
	{
		return;
	}	

	KDBE_RELEASE_RIDING_PET_REQ kPacketToDB;
	kPacketToDB.m_iUnitUID = GetCharUID();

	BOOST_TEST_FOREACH( const UidType, iRidingPetUID, vecRidingPetUID )
	{
		// ����� �� �����Ϸ� DB�� ����!!
		kPacketToDB.m_iRidingPetUID = iRidingPetUID;
		SendToGameDB( DBE_RELEASE_RIDING_PET_REQ, kPacketToDB );
	}
}
#endif	// SERV_RIDING_PET_SYSTM

#ifdef SERV_SYNC_PACKET_USING_RELAY_WORKINGS_CHECK// �۾���¥: 2013-05-06	// �ڼ���
void KGSUser::SetLanBugOutDisconnectCheck( IN const bool bSet )
{
	if( bSet == true )
	{
		if( m_bCheckUdpKickStateChangeFieldNot == true )
		{
			// �̹� ���� ���̴�.
			return;
		}

		m_bCheckUdpKickStateChangeFieldNot = true;
		m_kTimer[TM_UDP_KICK_STATE_CHANGE_FIELD_NOT].restart();
	}
	else
	{
		if( m_bCheckUdpKickStateChangeFieldNot == true )
		{
			// üũ ����
			m_bCheckUdpKickStateChangeFieldNot = false;
		}
	}
}

void KGSUser::CheckLanBugOutDisconnect( void )
{
	if( m_bCheckUdpKickStateChangeFieldNot == false )
	{
		return;
	}

	const float fLanBugOutDisconnectCheckTerm = SiKGameSysVal()->GetLanBugOutDisconnectCheckTerm();

	// ��� ��� ������ ���� �뵵
	if( fLanBugOutDisconnectCheckTerm < 0.0f )
	{
		SetLanBugOutDisconnectCheck( false );
		return;
	}

	if( m_kTimer[TM_UDP_KICK_STATE_CHANGE_FIELD_NOT].elapsed() < fLanBugOutDisconnectCheckTerm )
	{
		return;
	}

#ifdef SERV_HACKING_USER_CHECK_COUNT// �۾���¥: 2013-06-02	// �ڼ���
	IncreaseCheckCount( SEnum::HUCT_FIELD_WORKING_CHECK, L"" );
#else // SERV_HACKING_USER_CHECK_COUNT
	IncreaseBlockCount();
#endif // SERV_HACKING_USER_CHECK_COUNT

	// ���� ���� ������ �ŷ��� ����
	// HackUserRegTradeBlock DB�� ������ �ʿ�� ���� �� ���Ƽ�..... ���� �����̴ϱ�
	m_kUserAbuserManager.SetTradeBlockUser( true );

#ifdef SERV_HACKING_USER_CHECK_COUNT// �۾���¥: 2013-06-02	// �ڼ���
#else // SERV_HACKING_USER_CHECK_COUNT
	// ���� ű
	int iLanBugOutMinDisconnectTerm	= 0;
	int iLanBugOutDisconnectTerm	= 0;
	SiKGameSysVal()->GetLanBugOutDisconnectTerm( iLanBugOutMinDisconnectTerm, iLanBugOutDisconnectTerm );
	
	SetRandomTimeKick( KStatistics::eSIColDR_LanBugOutDisconnect, iLanBugOutMinDisconnectTerm, iLanBugOutDisconnectTerm );
#endif // SERV_HACKING_USER_CHECK_COUNT

	m_kTimer[TM_UDP_KICK_STATE_CHANGE_FIELD_NOT].restart();
	SetLanBugOutDisconnectCheck( false );
}

void KGSUser::InitBlockCount( IN const byte byteBlockCheckCount, IN const std::wstring& wstrBlockCheckResetDate )
{
	m_bBlockCheckCountChanged = false;

	if( wstrBlockCheckResetDate.empty() == true )
	{
		CTime tCurrentDate			= CTime::GetCurrentTime();
		m_byteBlockCheckCount		= 0;
		m_wstrBlockCheckResetDate	= tCurrentDate.Format( _T("%Y-%m-%d %H:%M:%S") );
	}
	else
	{
		m_byteBlockCheckCount		= byteBlockCheckCount;
		m_wstrBlockCheckResetDate	= wstrBlockCheckResetDate;
	}
}

void KGSUser::IncreaseBlockCount( void )
{
	if( m_wstrBlockCheckResetDate.empty() == true )
	{
		// �ʱ�ȭ���� �ʾҴ�. DB�� �ٳ����
		SendToAccountDB( DBE_BLOCK_COUNT_CHECK_INFO_READ_REQ );
		return;
	}

	// ī��Ʈ �ʱ�ȭ üũ
	CTime tBlockCheckResetDate;
	if( KncUtil::ConvertStringToCTime( m_wstrBlockCheckResetDate, tBlockCheckResetDate ) == false )
	{
		START_LOG( cerr, L"�ʵ� �۾��� ���� ī��Ʈ ���� �ð� ��ȯ ����" )
			<< BUILD_LOG( GetUID() )
			<< BUILD_LOG( m_wstrBlockCheckResetDate )
			<< END_LOG;
	}
	else
	{
		// ������ �ʱ�ȭ �ð����κ��� 1���� �� ����� ���� 6�ø� ����
		int iDayOfWeek			= tBlockCheckResetDate.GetDayOfWeek();
		int iNextResetDayTerm	= 5 - iDayOfWeek;
		if( iNextResetDayTerm <= 0 )
		{
			iNextResetDayTerm += 7;
		}

		CTimeSpan tNextResetDayTerm( iNextResetDayTerm, 0, 0, 0 );
		CTime tResetTime	= CTime( tBlockCheckResetDate.GetYear(), tBlockCheckResetDate.GetMonth(), tBlockCheckResetDate.GetDay(), 6, 0, 0 ) + tNextResetDayTerm;
		CTime tCurrentTime	= CTime::GetCurrentTime();

		if( tResetTime < tCurrentTime )
		{
			// ī��Ʈ �ʱ�ȭ ����
			m_byteBlockCheckCount		= 0;
			m_wstrBlockCheckResetDate	= tCurrentTime.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
		}
	}

	// ���� ó��
	m_bBlockCheckCountChanged = true;
	if( ++m_byteBlockCheckCount < 3 )
	{
		return;
	}

	m_byteBlockCheckCount = 0;

	SendID( EGS_FIELD_WORKINGS_BLOCK_LOG_REQ );

	// �ڵ� ����
	HackUserRegRejectedUser( KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_LANBUGOUT_DISCONNECT );
}

bool KGSUser::GetBlockCountChanged( OUT byte& byteBlockCheckCount, OUT std::wstring& wstrBlockCheckResetDate )
{
	if( m_bBlockCheckCountChanged == false )
	{
		return false;
	}

	byteBlockCheckCount		= m_byteBlockCheckCount;
	wstrBlockCheckResetDate	= m_wstrBlockCheckResetDate;

	return true;
}
#endif // SERV_SYNC_PACKET_USING_RELAY_WORKINGS_CHECK

//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
void KGSUser::CheckRelationshipReward()
{
	// 1�и��� üũ
	if( m_kTimer[TM_RELATIONSHIP_REWARD].elapsed() < 60.0 )
		return;

	m_kTimer[TM_RELATIONSHIP_REWARD].restart();

	KDBE_INSERT_WEDDING_REWARD_REQ kPacket;
	kPacket.m_iUnitUID = GetCharUID();
	kPacket.m_iLoverUnitUID = m_kUserRelationshipManager.GetLoverUnitUID();

	// ����� ����
	if( m_kUserRelationshipManager.IsRewardDay() == true )
	{
		kPacket.m_bTitleReward = false;

		// ���� ���� �Ϸ� ����
		// Ŀ���̶��
		if( m_kUserRelationshipManager.GetRelationshipType() == SEnum::RT_COUPLE )
		{
			// Ŀ�� ����� ť��
			kPacket.kPostReq.m_iFromUnitUID		= GetCharUID();
			kPacket.kPostReq.m_iToUnitUID		= GetCharUID();
			kPacket.kPostReq.m_iRewardType		= KPostItemInfo::LT_WEDDING_REWARD;
			kPacket.kPostReq.m_iRewardID		= CXSLItem::SI_COUPLE_ANNIVERSARY_DAY_REWARD_ITEM;
			SendToGameDB( DBE_INSERT_WEDDING_REWARD_REQ, kPacket );

			// ���� ���� �� ���� ��� ����
//			SendToLogDB();
		}
		else if( m_kUserRelationshipManager.GetRelationshipType() == SEnum::RT_MARRIED )
		{
			// ��ȥ ����� ť��
			kPacket.kPostReq.m_iFromUnitUID		= GetCharUID();
			kPacket.kPostReq.m_iToUnitUID		= GetCharUID();
			kPacket.kPostReq.m_iRewardType		= KPostItemInfo::LT_WEDDING_REWARD;
			kPacket.kPostReq.m_iRewardID		= CXSLItem::SI_WEDDING_ANNIVERSARY_DAY_REWARD_ITEM;
			SendToGameDB( DBE_INSERT_WEDDING_REWARD_REQ, kPacket );

			// ���� ���� �� ���� ��� ����
//			SendToLogDB();
		}
	}

	// Īȣ ����
	int iTitleItemID = m_kUserRelationshipManager.IsRewardTitleItem( kPacket.m_iTitleRewardStep );
	if( iTitleItemID > 0 )
	{
		kPacket.m_bTitleReward = true;

		// Ÿ��Ʋ ���� ����
		kPacket.kPostReq.m_iFromUnitUID		= GetCharUID();
		kPacket.kPostReq.m_iToUnitUID		= GetCharUID();
		kPacket.kPostReq.m_iRewardType		= KPostItemInfo::LT_WEDDING_REWARD;
		kPacket.kPostReq.m_iRewardID		= iTitleItemID;
		SendToGameDB( DBE_INSERT_WEDDING_REWARD_REQ, kPacket );

		// Ÿ��Ʋ ���� ���� �� ���� ��� ����
//		SendToLogDB();
	}
}
#endif SERV_RELATIONSHIP_SYSTEM
//}

#ifdef SERV_HACKING_USER_CHECK_COUNT// �۾���¥: 2013-06-01	// �ڼ���
void KGSUser::HackUserAccountBlock( IN const byte byteType, IN const std::wstring& wstrReason, IN const std::wstring& wstrEndDate ) const
{
	// �� ���� ���� ���� �ڵ����
	if( SiKGameSysVal()->IsHackUserRegRejected() == false )
		return;

	// Account Block ���� ����ϱ�!
	KDBE_ACCOUNT_BLOCK_NOT kPacketNot;
	kPacketNot.m_iUserUID		= GetUID();
	kPacketNot.m_byteType		= byteType;
	kPacketNot.m_wstrReason		= wstrReason.substr( 0, 255 );
	kPacketNot.m_wstrEndDate	= wstrEndDate;
	SendToAccountDB( DBE_ACCOUNT_BLOCK_NOT, kPacketNot );

	START_LOG( cout, L"Account Block" )
		<< BUILD_LOG( GetUserName() )
		<< BUILD_LOG( GetCharName() )
		<< BUILD_LOG( byteType )
		<< BUILD_LOG( kPacketNot.m_wstrReason )
		<< BUILD_LOG( wstrEndDate )
		;
}

void KGSUser::IncreaseCheckCount( IN const byte byteType, IN const std::wstring& wstrReason, IN const bool bAfterInited /*= false */)
{
	byte byteResult = 0;
	KAbuserLogManager::KHackingUserCheckInfo kInfo;
	
	switch( m_kUserAbuserManager.IncreaseCheckCount( byteType, byteResult, kInfo ) )
	{
	case KUserAbuserManager::HUCR_SUCCEED:
		if( SEnum::CheckFlag( byteResult, KAbuserLogManager::HUCRT_PACKETMORNITORING ) == true )
		{
			// ��Ŷ ����͸� ���
			HackUserRegPacketMornitor();
		}

		if( SEnum::CheckFlag( byteResult, KAbuserLogManager::HUCRT_ACCOUNTBLOCK ) == true )
		{
			std::wstring wstrEndDate;
			if( ( 0 < kInfo.m_usAccountBlockTermDay )		||
				( 0 < kInfo.m_usAccountBlockTermHour )		||
				( 0 < kInfo.m_usAccountBlockTermMinute )
				)
			{
				CTime tEndDate	= CTime::GetCurrentTime() + CTimeSpan( kInfo.m_usAccountBlockTermDay, kInfo.m_usAccountBlockTermHour, kInfo.m_usAccountBlockTermMinute, 0 );
				wstrEndDate		= tEndDate.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
			}

			HackUserAccountBlock( byteType, wstrReason, wstrEndDate );
		}

		if( SEnum::CheckFlag( byteResult, KAbuserLogManager::HUCRT_KICK ) == true )
		{
			if( 0 < ( kInfo.m_byteRandomKickMin + kInfo.m_byteRandomKickTerm ) )
			{
				SetRandomTimeKick( byteType, kInfo.m_byteRandomKickMin, kInfo.m_byteRandomKickTerm );
			}
			else
			{
				SetDisconnectReason( KStatistics::eSIColDR_Account_Block );
				ReserveDestroy();
			}
		}
		break;

	case KUserAbuserManager::HUCR_FAILED:
		// IncreaseCheckCount -> SiKAbuserLogManager()->GetHackingUserCheckInfo ���� ���� �α� ���� ��� ����.
		START_LOG( cerr, L"" )
			<< BUILD_LOG( GetCharUID() )
			<< BUILD_LOG( GetCharName() )
			<< END_LOG;
		break;

	case KUserAbuserManager::HUCR_NOT_INITED:
		if( bAfterInited == true )
		{
			START_LOG( cerr, L"��ŷ ���� üũ ī��Ʈ�� �ʱ�ȭ �� �� �����ϴ�." )
				<< BUILD_LOG( GetCharUID() )
				<< BUILD_LOG( GetCharName() )
				<< END_LOG;
		}
		else
		{
			KDBE_HACKING_USER_CHECK_COUNT_INFO_REQ kPacketToDB;
			kPacketToDB.m_iUserUID		= GetUID();
			kPacketToDB.m_byteType		= byteType;
			kPacketToDB.m_wstrReason	= wstrReason;
			SendToAccountDB( DBE_HACKING_USER_CHECK_COUNT_INFO_REQ, kPacketToDB );
		}
		break;
	}
}

byte KGSUser::GetAccountBlockType( void ) const
{
	if( m_kAccountInfo.m_kAccountBlockInfo.m_wstrBlockEndDate.empty() == true )
	{
		return m_kAccountInfo.m_kAccountBlockInfo.m_byteBlockType;
	}

	CTime tEndDate;
	if( KncUtil::ConvertStringToCTime( m_kAccountInfo.m_kAccountBlockInfo.m_wstrBlockEndDate, tEndDate ) == false )
	{
		return m_kAccountInfo.m_kAccountBlockInfo.m_byteBlockType;
	}

	if( CTime::GetCurrentTime() < tEndDate )
	{
		return m_kAccountInfo.m_kAccountBlockInfo.m_byteBlockType;
	}

	return SEnum::HUCT_NONE;
}
#endif // SERV_HACKING_USER_CHECK_COUNT

#ifdef	SERV_UPGRADE_SKILL_SYSTEM_2013 // ���볯¥: 2013-06-27
bool KGSUser::isCanUseUnsealCashSkillItem( IN int iItemID )
{
	int iUnitClassType = CXSLUnit::UCT_NONE;
	if( SiCXSLSkillTree()->GetCashSkillItemUnitClassType( iItemID, iUnitClassType ) == false )
	{
		return false;
	}

	switch( iUnitClassType )
	{
	case CXSLUnit::UCT_BASIC_CLASS:		// �⺻, 1��, 2�� �����̸� ��� �� �ִ�
		{
			if( CXSLUnit::IsInitNormalJob( static_cast<CXSLUnit::UNIT_CLASS>(GetUnitClass()) ) 
				|| CXSLUnit::IsFirstChangeJob( static_cast<CXSLUnit::UNIT_CLASS>(GetUnitClass()) ) 
				|| CXSLUnit::IsSecondChangeJob( static_cast<CXSLUnit::UNIT_CLASS>(GetUnitClass()) ) )
			{
				return true;
			}
		}break;
	case CXSLUnit::UCT_FIRST_CLASS:		// 1��, 2�� �����̸� ��� �� �ִ�
		{
			if( CXSLUnit::IsFirstChangeJob( static_cast<CXSLUnit::UNIT_CLASS>(GetUnitClass()) ) 
				|| CXSLUnit::IsSecondChangeJob( static_cast<CXSLUnit::UNIT_CLASS>(GetUnitClass()) ) )
			{
				return true;
			}
		}break;
	case CXSLUnit::UCT_SECOND_CLASS:	// 2�� �����̸� ��� �� �ִ�
		{
			if( CXSLUnit::IsSecondChangeJob( static_cast<CXSLUnit::UNIT_CLASS>(GetUnitClass()) ) )
			{
				return true;	
			}
		}break;
	default:
		{
			START_LOG( cerr, L"unit class type �� ���� ���������Դϴ�." )
				<< BUILD_LOG( iItemID )
				<< BUILD_LOG( iUnitClassType )
				<< END_LOG;
		}break;
	}

	return false;
}

#endif	// SERV_UPGRADE_SKILL_SYSTEM_2013

#ifdef SERV_CRITERION_DATE_EVENT// �۾���¥: 2013-06-26	// �ڼ���
byte KGSUser::GetCriterionDateEventInfo( void ) const
{
	byte byteResult = ( m_kComeBackUserInfo.GetIsComeBackUser() == true ) ? SEnum::CUT_COMEBACK : SEnum::CUT_NONE;

#ifdef SERV_CRITERION_DATE_EVENT_JUMPING_CHARACTER// �۾���¥: 2013-06-26	// �ڼ���
#ifdef SERV_JUMPING_CHARACTER// �۾���¥: 2013-07-12	// �ڼ���
	if( m_kUserJumpingCharacterManager.IsJumpingCharacter( GetCharUID() ) == JumpingCharacter::JCR_SUCCEED )
	{
		byteResult |= SEnum::CUT_JUMPING;
	}
#endif // SERV_JUMPING_CHARACTER
#endif // SERV_CRITERION_DATE_EVENT_JUMPING_CHARACTER

	return byteResult;
}
#endif // SERV_CRITERION_DATE_EVENT

#ifdef SERV_JUMPING_CHARACTER// �۾���¥: 2013-07-11	// �ڼ���
int KGSUser::JumpingCharacterCheck( void ) const
{
	int		iTargetLevel = 0;
	CTime	tCurrentDate;
	return JumpingCharacterCheck( iTargetLevel, tCurrentDate );
}

int KGSUser::JumpingCharacterCheck( OUT int& iTargetLevel, OUT CTime& tCurrentDate ) const
{
	if( IsGuestUser() )
	{
		return NetError::ERR_JUMPING_01;	// ���� �̺�Ʈ ����ڰ� �ƴմϴ�.
	}

	if( GetUnitClass() != CXSLUnit::GetUnitClassBaseGrade( static_cast<CXSLUnit::UNIT_CLASS>( GetUnitClass() ) ) )
	{
		return NetError::ERR_JUMPING_02;	// �⺻ ���� ����ڸ� ������ �� �ֽ��ϴ�.
	}

	if( m_kUserJumpingCharacterManager.IsInit() == false )
	{
		return NetError::ERR_JUMPING_03;	// ���� �̺�Ʈ ������ �ʱ�ȭ ���� �ʾҽ��ϴ�.
	}

	CTime	tBeginDate = 0;
	CTime	tEndDate = 0;
	bool	bAccountEvent = true;

	if( SiKGameEventManager()->GetJumpingCharacterEvent( GetUnitType()
		, GetLevel()
		, tBeginDate
		, tEndDate
		, iTargetLevel
		, bAccountEvent
		, tCurrentDate
		) == false )
	{
		return NetError::ERR_JUMPING_04;	// �ش�Ǵ� ���� �̺�Ʈ�� �����ϴ�.
	}

	if( m_kUserJumpingCharacterManager.Check( GetCharUID(), tBeginDate, tEndDate, bAccountEvent ) != JumpingCharacter::JCR_SUCCEED )
	{
		return NetError::ERR_JUMPING_05;	// ���� �̺�Ʈ ����ڰ� �ƴմϴ�.
	}

	return NetError::NET_OK;
}

bool KGSUser::CheckJumpingCharLevelUp( IN const unsigned char ucLevel )
{
	const u_char ucNowLevel = GetLevel();
	
	m_ucLevel = ucLevel;

	if( ucNowLevel < m_ucLevel )
	{
		//�������� ��� ��Ƽ���� ������Ʈ.
		SendUpdatePartyUnitInfo();
		
		// ü��ID ��� ����
		if( !IsGuestUser() )
		{
			// ��õ�� ���� �κ�
			if( m_kUserRecommendManager.GetRecommendedUnitUID() > 0 )
			{
				//{{ 2010. 01. 11  ������	��õ�θ���Ʈ
#ifdef SERV_RECOMMEND_LIST
				SendUpdateRecommendUserInfo( KRecommendUserInfo::RUIT_LEVEL_UP );
#endif SERV_RECOMMEND_LIST
				//}}
			}

			//���ڸ���Ʈ�� �ִٸ� ����������Ʈ.
			if( m_kUserTutorialInfo.GetTutorUnitType() == KUserTutorialInfo::TUT_STUDENT &&
				m_kUserTutorialInfo.IsExistbyTutorData() == false )
			{
				if( SiKTutorialManager()->IsExistbylist( GetCharUID() ) == true )
				{
					SiKTutorialManager()->SetLevelData( GetCharUID(), GetLevel() );

					//������ ������ ������ �Ǿ����� ����Ʈ���� �������ش�. 
					if( GetLevel() >= KTutorialManager::UNIT_LEVEL_STUDENT_MAX  &&
						ucNowLevel < KTutorialManager::UNIT_LEVEL_STUDENT_MAX )
					{
						SiKTutorialManager()->DeleteUnit( GetCharUID() );
					}
				}
			}

			//080408.hoons. ���� ���� �κ�.
			m_kUserTutorialInfo.ProcJumpingCharLevelUP( ucNowLevel, GetThisPtr<KGSUser>() );
		}

#ifdef	SERV_LOCAL_RANKING_SYSTEM // ���볯¥: 2013-03-28
		m_kUserLocalRankingInfo.SetCharacterLevel( GetLevel() );
		// Login Server�� ���� ���� ���� �˸��� ������.
		IncreasedLocalRankingPoint( 0, 0, true );
#endif	// SERV_LOCAL_RANKING_SYSTEM

		//{{ 2009. 6. 29  ������	���� �޼� ����
		if( GetLevel() == SiKGameSysVal()->GetLimitsLevel()  &&
			GetAuthLevel() < SEnum::UAL_GM )
		{
			KEGS_NOTIFY_MSG_NOT	kPacket;
			kPacket.m_cNotifyType = KEGS_NOTIFY_MSG_NOT::NT_MAX_LEVEL_UP;
			kPacket.m_Count = 1;

			CStringW cwstrMessage;
			cwstrMessage.Format( L"%s;%d", GetCharName().c_str(), GetLevel() );
			kPacket.m_wstrMSG = static_cast<LPCTSTR>(cwstrMessage);

			SendToCnServer( ECN_NOTIFY_MSG_NOT, kPacket );

			//{{ 2011. 01. 17	������	ĳ���� ī��Ʈ ����
#ifdef SERV_CHAR_LOG
			WriteCharacterLogToDB( KELOG_USER_CHARACTER_LOG_NOT::LT_FULL_LEVEL );
#endif SERV_CHAR_LOG
			//}}
		}
		//}}

		KStatTable::KUnitStatInfo	sUnitStat;
		if( SiKStatTable()->GetUnitStat( GetUnitClass(), GetLevel(), sUnitStat ) == true )
		{
			m_kStat.m_iBaseHP		= sUnitStat.m_uiHP;
			m_kStat.m_iAtkPhysic	= sUnitStat.m_usAtkPhysic;
			m_kStat.m_iAtkMagic		= sUnitStat.m_usAtkMagic;
			m_kStat.m_iDefPhysic	= sUnitStat.m_usDefPhysic;
			m_kStat.m_iDefMagic		= sUnitStat.m_usDefMagic;

			// ������ �ϸ� sp�� ���� ȹ���Ѵ�
			for( u_char ucLevel = ucNowLevel + 1; ucLevel <= GetLevel(); ++ucLevel )
			{
				int iSPInc = 0;
#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013 // ���볯¥: 2013-06-27
				if( SiCXSLSkillTree()->GetCalcLevelUpIncreaseSkillPoint( ucLevel, iSPInc ) == true )
#else	// SERV_UPGRADE_SKILL_SYSTEM_2013
/*				
				if( SiCXSLSkillTree()->GetIncSkillPoint( ucLevel, iSPInc ) == true )
*/				
#endif	// SERV_UPGRADE_SKILL_SYSTEM_2013
				{
					m_iSPoint += iSPInc;
				}
				else
				{
					START_LOG_WITH_NAME( cerr )
						<< END_LOG;
				}
			}

            // �������� ���� �α׸� DB�� �����.
            KDBE_CHARACTER_LEVELUP_NOT kNot;
            kNot.m_nCharUID = GetCharUID();
            kNot.m_ucLevelBefore = ucNowLevel;
            kNot.m_ucLevelAfter = GetLevel();
            SendToLogDB( DBE_CHARACTER_LEVELUP_NOT, kNot );

			//{{ 2011. 01. 17	������	ĳ���� ī��Ʈ ����
#ifdef SERV_CHAR_LOG
			WriteCharacterLogToDB( KELOG_USER_CHARACTER_LOG_NOT::LT_LEVEL_UP );
#endif SERV_CHAR_LOG
			//}}

			// ü��ID ��� ����
			if( IsGuestUser() )
			{
				// ü��ID ���� ����
				if( GetLevel() > SiKGameSysVal()->GetGuestUserLimitLevel() )
				{
					SendID( EGS_GUEST_USER_LIMIT_LEVEL_NOT );
				}
			}

			// ���ο� �̼� �˻�
			m_kUserTitleManager.CheckNewMission( GetThisPtr<KGSUser>() );

			//{{ 2009. 10. 8  ������	���
#ifdef GUILD_TEST
			SendUpdateGuildMemberInfo();
#endif GUILD_TEST
			//}}

			//{{ QUEST ���� - ��μ�
#ifdef SERV_REFORM_QUEST
			m_kUserQuestManager.CheckLimitLevelQuest( GetThisPtr<KGSUser>() );
			m_kUserQuestManager.CheckEventQuest( GetThisPtr<KGSUser>() );
#endif SERV_REFORM_QUEST
			//}}

			return true;
		}
        else
		{
			START_LOG_WITH_NAME( cerr )
				<< L"UNIT STAT DATA �Ľ� ����.!(�̷��� �ȵǴµ�)" << dbg::endl
				<< BUILD_LOGc( GetUnitClass() )
				<< BUILD_LOG( GetLevel() )
				<< END_LOG;

			return false;
		}
	}

	return false;
}
#endif // SERV_JUMPING_CHARACTER

#ifdef SERV_PERIOD_PET
void KGSUser::CheckExpirePet()
{
	// ����� �� ����Ʈ ���
	std::vector<UidType> vecPetUID;
	if( m_kUserPetManager.GetExpirePetUID( vecPetUID ) == false )
	{
		return;
	}

	if( vecPetUID.empty() == true )
	{
		return;
	}	

	KDBE_RELEASE_PET_REQ kPacketToDB;
	kPacketToDB.m_iUnitUID = GetCharUID();

	BOOST_TEST_FOREACH( const UidType, iPetUID, vecPetUID )
	{
		// ����� �� �����Ϸ� DB�� ����!!
		kPacketToDB.m_iPetUID = iPetUID;
		SendToGameDB( DBE_RELEASE_PET_REQ, kPacketToDB );
	}
}
#endif SERV_PERIOD_PET

#ifdef SERV_LIMITED_DUNGEON_PLAY_TIMES
bool KGSUser::CheckLimitedPlayTimes( IN const int iDungeonID )
{
	int iUnitPlayTimes = 0;
	int iUnitClearTimes = 0;

	KUnitInfo kMyUnitinfo;
	GetUnitInfo( kMyUnitinfo );

	std::map< int, KDungeonPlayInfo >::iterator mitDungeonPlay = kMyUnitinfo.m_mapDungeonPlay.find( iDungeonID );
	if( mitDungeonPlay != kMyUnitinfo.m_mapDungeonPlay.end() )
	{
		iUnitPlayTimes = mitDungeonPlay->second.m_iPlayTimes;
		iUnitClearTimes = mitDungeonPlay->second.m_iClearTimes;
	}

	int iLimitedPlayTimes = 0;
	int iLimitedClearTimes = 0;

	// �÷��� Ƚ�� ���� �����̸� ������ �÷��� Ƚ���� ���ѿ� ���������� üũ ����
	if( SiCXSLDungeonManager()->GetLimitedPlayTimes( iDungeonID, iLimitedPlayTimes ) == true &&
		iLimitedPlayTimes <= iUnitPlayTimes )
	{
		return false;
	}

	// Ŭ���� Ƚ�� ���� �����̸� ������ Ŭ���� Ƚ���� ���ѿ� ���������� üũ ����
	if( SiCXSLDungeonManager()->GetLimitedClearTimes( iDungeonID, iLimitedClearTimes ) == true &&
		iLimitedClearTimes <= iUnitClearTimes )
	{
		return false;
	}

	// �Ѵ� ������ �Ѿ������ üũ ����
	return true;
}
#endif SERV_LIMITED_DUNGEON_PLAY_TIMES

//////////////////////////////////////////////////////////////////////////
#endif SERV_GSUSER_CPP
//////////////////////////////////////////////////////////////////////////
