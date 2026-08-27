#include "DungeonRoom.h"
#include "NetError.h"
#include "Room/RoomFSM.h"
#include "X2Data/XSLRoom.h"
#include "KDropTable.h"
#include "ResultProcess.h"

#include "x2data/XSLItemManager.h"
#include "X2Data/XSLDungeonManager.h"
#include "X2Data/XSLUnitManager.h"
#include "KAttribNpcTable.h"

//{{ 2009. 5. 22  ������	��ӷ��̺�Ʈ
#include "GameEvent/GameEventManager.h"
//}}
//{{ 2009. 7. 27  ������	��ϸ� �ð�
#include "HenirResultTable.h"
#include "GameSysVal/GameSysVal.h"
//}}
//{{ 2010. 10. 05	������	���ͼ��� ��ŷüũ
#ifdef SERV_CN_SERVER_HACKING_CHECK
	#include "CnHackingCheckManager.h"
#endif SERV_CN_SERVER_HACKING_CHECK
//}}
//{{ 2010. 11. 20	������	Ư�� �ð� ���� ����
#ifdef SERV_TIME_DROP_MONSTER_EVENT
	#include "DungeonEventManager.h"
#endif SERV_TIME_DROP_MONSTER_EVENT
//}}
//{{ 2012. 02. 06    ��μ�    ���� ���� UID �߱� ��� ����
#ifdef SERV_MAKE_MONSTER_UID_CHANGE
	#include <boost/random.hpp>
#endif SERV_MAKE_MONSTER_UID_CHANGE
//}}
//{{ 2012. 03. 05	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	#include "X2Data/XSLBuffManager.h"
#endif SERV_BATTLE_FIELD_SYSTEM
//}}
//{{ 2013. 04. 15	������	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
	#include "X2Data/XSLDefenceDungeonManager.h"
#endif SERV_NEW_DEFENCE_DUNGEON
//}}

NiImplementRTTI( KDungeonRoom, KRoom );

#define CLASS_TYPE KDungeonRoom

KDungeonRoom::KDungeonRoom()
{
	//{{ 2011. 11. 7	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	m_spRoomUserManager = KRoomUserManagerPtr( new KRoomUserManager );
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}

	m_spRoomUserManager->Init( CXSLRoom::RT_DUNGEON, 4 );

	m_bEndPlay				= false;
	m_cGetItemType			= CXSLRoom::DGIT_RANDOM;
	m_usLoadingState		= 0;

	//{{ 2011. 06. 13	������	�������� �α�
#ifdef SERV_DUNGEON_STAGE_LOG
	m_iLoadingCompleteStageID = -1;
#endif SERV_DUNGEON_STAGE_LOG
	//}}

	// �Ӽ� ���� ī��Ʈ
	m_iAttribNpcStageCnt	= 0;
	m_iAttribNpcDropCnt		= 0;

	//{{ 2010. 04. 07  ������	��д��� ����
#ifdef SERV_SECRET_HELL
	m_iExtraStageLoadCount	= 0;
	m_bAdventExtraNpc		= false;	
	m_fPoisonTimeLimit		= 0.0f;
#endif SERV_SECRET_HELL
	//}}

	//{{ 2010. 07. 09  ������	��ӷ� �̺�Ʈ Ȯ��
#ifdef SERV_PC_BANG_DROP_EVENT
	m_iItemDropEventProbCount = REE_DROP_EVENT_PROB_DEFAULT;
	m_bWithPlayPcBangEvent	= false;
#endif SERV_PC_BANG_DROP_EVENT
	//}}

	m_iHighestUserLevelAtDungeonStart = 0;
	//{{ 2012. 05. 10	������	���� ���� ���� ����
#ifdef SERV_DUNGEON_NPC_LEVEL
	m_iAverageUserLevelAtDungeonStart = 0;
#endif SERV_DUNGEON_NPC_LEVEL
	//}}
	m_eDungeonType			= CXSLDungeon::DT_NORMAL; // ��ϸ� �ð�

	//{{ 2009. 7. 10  ������	��� �ý���
	m_iPartyUID				= 0;
	//}}

	//{{ 2010. 06. 17  ������	������ ����
#ifdef SERV_PROTECT_AUTO_HACK
	m_iHackUserCheckCount	= 0;
	m_iNpcDieHackCount		= 0;
	m_iGetItemHackCount		= 0;
	m_iStartedNumMember		= 0;
#endif SERV_PROTECT_AUTO_HACK
	//}}
	//{{ 2012. 03. 15	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	m_bStartedByAutoParty	= false;
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}

	//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER
	m_iDefenseMainCrystalNpcUID = 0;
	m_setDefenseSmallCrystalNpcUID.clear();
#endif SERV_INSERT_GLOBAL_SERVER
	//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�

	//{{ 2012. 02. 06    ��μ�    ���� ���� UID �߱� ��� ����
#ifdef SERV_MAKE_MONSTER_UID_CHANGE
	m_bHackingNpcUid	= false;
#endif SERV_MAKE_MONSTER_UID_CHANGE	
	//}}

	//{{ 2013. 03. 18	 ����Ʈ ��� Ȯ�� ���� �̺�Ʈ - ��μ�
#ifdef SERV_QUEST_ITEM_DROP_EVENT
	m_fQuestItemDropEventProbRate = 1.f;
#endif SERV_QUEST_ITEM_DROP_EVENT
	//}

#ifdef  SERV_CHOOSE_FASTEST_HOST
#ifdef  SERV_OPTIMIZE_CHOOSE_FASTEST_HOST_ENHANCE
    m_iSubStageCount = 0;
#endif  SERV_OPTIMIZE_CHOOSE_FASTEST_HOST_ENHANCE
#endif  SERV_CHOOSE_FASTEST_HOST

#ifdef SERV_STAGE_CLEAR_IN_SERVER
	m_kNextStageData.ClearData();
	m_bFirstStage = true;
	m_bFirstSubStage = true;
	m_iClearConditionIndex = 0;
#endif SERV_STAGE_CLEAR_IN_SERVER
}

KDungeonRoom::~KDungeonRoom()
{
}

ImplToStringW( KDungeonRoom )
{
    return START_TOSTRING_PARENTW( KRoom );
}

void KDungeonRoom::Tick()
{
    KRoom::Tick();
	
	if( GetStateID() == KRoomFSM::S_PLAY )
	{
		//{{ 2010. 05. 06  ������	��д��� ����
#ifdef SERV_SECRET_HELL
		// ��д����̶��..
		if( m_eDungeonType == CXSLDungeon::DT_SECRET )
		{
			if( m_fPoisonTimeLimit > 0.0f )
			{
				// ���� �����ð��� ���� �÷��̽ð��� ���Ѱ��� ��ü �ð����Ѻ��� ������ ���� ���� �ð��̴�!
				if( ( m_fPoisonTimeLimit + GetRemainingPlayTime() ) < GetPlayTimeLimit() )
				{
					KEGS_DUNGEON_EFFECT_TIME_OUT_NOT kPacketNot;
					kPacketNot.m_cType = KEGS_DUNGEON_EFFECT_TIME_OUT_NOT::DDT_POISON;
					BroadCast( ERM_DUNGEON_EFFECT_TIME_OUT_NOT, kPacketNot );

					// �ѹ� ������ ������ -1.f���� �־ �������� �ȳ������� �Ѵ�.
					m_fPoisonTimeLimit = -1.f;
				}
			}
		}
#endif SERV_SECRET_HELL
		//}}

		//{{ 2013. 02. 01  �̺�Ʈ �߷�Ÿ�� ���� - ��μ�
#ifdef SERV_EVENT_VALENTINE_DUNGEON
		if( CXSLDungeon::IsValentineDungeon( GetDungeonIDAndDif() ) == true )
		{
			int iRet = KValentineManager::TT_NONE;

			m_kValentineManager.CheckTimer( KValentineManager::TEM_AFTER_DUNGEON_START, iRet );

			switch( iRet )
			{
			case KValentineManager::TT_SYNC_TIME:
				{
					// ��ο��� �ð� ����
					KEGS_SYNC_DUNGEON_TIMER_NOT kNot;
					kNot.m_iRemainTime = KValentineManager::LDT_VALENTINE - m_kValentineManager.GetRemainTime( KValentineManager::TEM_AFTER_DUNGEON_START );
					BroadCast( ERM_SYNC_DUNGEON_TIMER_NOT, kNot );
				}
				break;
			case KValentineManager::TT_END_GAME:
				{
					// ������ �¸�
					KEGS_SYNC_DUNGEON_TIMER_NOT kNot;
					kNot.m_iRemainTime = 0;
					BroadCast( ERM_SYNC_DUNGEON_TIMER_NOT, kNot );
				}
				break;
			}
		}
#endif SERV_EVENT_VALENTINE_DUNGEON
		//}


		// �������϶� ��������� ���°� EndPlay��� ������ �����Ѵ�!
		if( m_spRoomUserManager->CheckEndPlay() )
		{
			EndPlay();
		}
	}
	//{{ 2013. 04. 15	������	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
	else if( GetStateID() == KRoomFSM::S_WAIT_FOR_DEFENCE )
	{
		// 5�ʰ� ������ ���� �ο��� ������ �״�� ����� �� ���� ����!
        if( m_kTimer[TM_WAIT_FOR_DEFENCE].elapsed() > 5.0 )
		{
			m_kTimer[TM_WAIT_FOR_DEFENCE].restart();

			// ������ ������ ���� ���� ������ 
			std::map< UidType, UidType > mapUserList;
			m_spRoomUserManager->GetUnitUIDListNotPrepareForDefenceDungeon( mapUserList );

			// �濡�� ��Ż ��Ű��!
			HandleTimeOutUser( mapUserList );			

			//{{ 2013. 04. 23	������	����� �� ���� ���� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON_ENTER_LOGIC
			// 5�ʵ��� ����� �� ���� �ǻ縦 ������ ���� ������ ������ ���� ó�� ����!
			m_spRoomUserManager->SetAllEnterDefenceDungeon();
#endif SERV_NEW_DEFENCE_DUNGEON_ENTER_LOGIC
			//}}

			if( IsEmpty() == false )
			{
				//{{ 2013. 04. 23	������	����� �� ���� ���� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON_ENTER_LOGIC
				// ������ ������ ������ ����Ʈ�� ����!
				std::vector< UidType > vecLeaveUnitUID;
				m_spRoomUserManager->GetUnitUIDListDisagreeEnterDefenceDungeon( vecLeaveUnitUID );

				// ����� �� ���� ����!
				EnterDefenceDungeon( vecLeaveUnitUID );

				START_LOG( clog, L"5�ʰ� ���� ���¿��� ����� �� ������ �����մϴ�!" );
#else
				// ����� �� ���� ����!
				EnterDefenceDungeon();
#endif SERV_NEW_DEFENCE_DUNGEON_ENTER_LOGIC
				//}}
			}
			else
			{
				StateTransition( KRoomFSM::I_TO_CLOSE );
			}
		}
	}
#endif SERV_NEW_DEFENCE_DUNGEON
	//}}
}

void KDungeonRoom::SetRoomInfo( const KRoomInfo& kInfo )
{
    m_cRoomType			= kInfo.m_RoomType;
    m_uiRoomListID		= kInfo.m_uiRoomListID;
    m_bPublic			= kInfo.m_bPublic;
    m_bTeamBalance		= false;
    m_wstrPassword		= kInfo.m_wstrPassword;
    m_cGameType			= CXSLRoom::PGT_TEAM;
    m_cWinningNumKill	= 0;
    m_sWorldID			= kInfo.m_WorldID;
    m_cDifficultyLevel	= kInfo.m_DifficultyLevel;
    m_iDungeonID		= kInfo.m_iDungeonID;
	m_cDungeonMode		= kInfo.m_cDungeonMode;
	m_iPartyUID			= kInfo.m_iPartyUID;
	//{{ 2012. 02. 27	�ڼ���	������ ����
#ifdef SERV_COEXISTENCE_FESTIVAL_ROOMBUFF
	m_iBuffType			= kInfo.m_iBuffType;
#endif SERV_COEXISTENCE_FESTIVAL_ROOMBUFF
	//}}
	//{{ 2012. 03. 15	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	m_bStartedByAutoParty = kInfo.m_bStartedByAutoParty;
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}

	if( kInfo.m_cGetItemType <= CXSLRoom::DGIT_NONE || kInfo.m_cGetItemType >= CXSLRoom::DGIT_END )
	{
		START_LOG( cwarn, L"�������� ������ ����Ÿ���� �߸��Ǿ���.!" )
			<< BUILD_LOGc( m_cGetItemType )
			<< BUILD_LOGc( kInfo.m_cGetItemType )
			<< BUILD_LOGc( kInfo.m_iDungeonID )
			<< BUILD_LOGc( kInfo.m_DifficultyLevel )
			<< END_LOG;

		m_cGetItemType		= CXSLRoom::DGIT_RANDOM;		
	}
	else
	{
		m_cGetItemType		= kInfo.m_cGetItemType;
	}

	SetName( kInfo.m_RoomName );
	SetPlayTimeLimit( kInfo.m_fPlayTime );
	//{{ 2013. 04. 10	������	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
	m_kDefenceDungeonManager.SetOpenDefenceDungeon( kInfo.m_bDefenceDungeonOpen );
#endif SERV_NEW_DEFENCE_DUNGEON
	//}}
}

void KDungeonRoom::GetRoomInfo( OUT KRoomInfo& kInfo )
{
	KRoom::GetRoomInfo( kInfo );
	kInfo.m_bStartedByAutoParty = IsStartedByAutoParty();
}

//{{ 2012. 10. 16	������		P2P & Relay ���� ����
#ifdef SERV_COLLECTION_OF_RELAY_AND_P2P_INFO
void KDungeonRoom::LogToDB_GamePlayNetWorkInfo( IN const UidType iUnitUID, IN const KGamePlayNetworkInfoPackage& kInfo, IN const bool bLeaveRoom )
{
	if( kInfo.m_vecGamePlayNetworkInfo.empty() == false )
	{
		CTime tCurTime = CTime::GetCurrentTime();
		KELOG_COLLECTION_OF_RELAY_AND_P2P_INFO_NOT kPacketToLog;

		BOOST_TEST_FOREACH( const KGamePlayNetworkInfo&, kGamePlayNetworkInfo, kInfo.m_vecGamePlayNetworkInfo )
		{
			if( kGamePlayNetworkInfo.m_iOpponentUnitUID == iUnitUID )
				continue;

			KDBUpdateGamePlayNetworkInfo kDBUpdateInfo;
			kDBUpdateInfo.m_iGameType			  = m_cRoomType;
			kDBUpdateInfo.m_iHolePunchingTryCount = kGamePlayNetworkInfo.m_iHolePunchingTryCount;
			kDBUpdateInfo.m_iP2PPlayTime		  = static_cast<int>(kGamePlayNetworkInfo.m_fP2PPlayTime);		// int�� Ÿ�� ĳ�����ϸ� �ʸ� ���´�.
			kDBUpdateInfo.m_iRelayPlayTime		  = static_cast<int>(kGamePlayNetworkInfo.m_fRelayPlayTime);	// int�� Ÿ�� ĳ�����ϸ� �ʸ� ���´�.
			kDBUpdateInfo.m_wstrRelayServerIP	  = NetCommon::GetLocalIPW();
			kDBUpdateInfo.m_wstrRegDate			  = (const wchar_t*)tCurTime.Format(_T("%Y-%m-%d %H:%M:%S"));
			kPacketToLog.m_vecDBUpdateInfo.push_back( kDBUpdateInfo );
		}

		if( kPacketToLog.m_vecDBUpdateInfo.empty() == false )
		{
			SendToLogDB( ELOG_COLLECTION_OF_RELAY_AND_P2P_INFO_NOT, kPacketToLog );
		}
	}
}
#endif SERV_COLLECTION_OF_RELAY_AND_P2P_INFO
//}}

//{{ 2012. 11. 9	������	��Ʋ�ʵ� �ý���
//#ifdef SERV_BATTLE_FIELD_SYSTEM
//{{ 2012. 12. 12	������	��Ʋ�ʵ� �ý���
#ifdef SERV_CHECK_USER_NEVER_RETURN_TO_FIELD
void KDungeonRoom::LogToDB_DungeonLog( IN KRoomUserPtr spRoomUser, IN const int iClearValue, IN const KERM_UPDATE_DUNGEON_UNIT_INFO_NOT& kUpdateUnitInfo, IN const KDungeonUnitResultInfo& kDungeonUnitInfo, IN const int iLeaveReason /*= 0*/ )
#else
void KDungeonRoom::LogToDB_DungeonLog( IN KRoomUserPtr spRoomUser, IN const int iClearValue, IN const KERM_UPDATE_DUNGEON_UNIT_INFO_NOT& kUpdateUnitInfo, IN const KDungeonUnitResultInfo& kDungeonUnitInfo )
#endif SERV_CHECK_USER_NEVER_RETURN_TO_FIELD
//}}
{
	// ��ڴ� ��踦 ������ �ʽ��ϴ�.
	if( spRoomUser->GetAuthLevel() >= SEnum::UAL_GM )
	{
		START_LOG( cout, L"��ڴ� ��赥���͸� ������ �ʽ��ϴ�." )
			<< BUILD_LOGc( spRoomUser->GetAuthLevel() )
#ifdef SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( spRoomUser->GetCID() );
#else
			<< BUILD_LOG( spRoomUser->GetNickName() );
#endif SERV_PRIVACY_AGREEMENT
		return;
	}

	CTime kRegDate = CTime::GetCurrentTime();
	//////////////////////////////////////////////////////////////////////////
	KE_LOCAL_LOG_DUNGEON_NOT kNot;
	kNot.m_iDungeonID				= (m_iDungeonID + static_cast<int>(m_cDifficultyLevel));
	kNot.m_nUnitUID					= spRoomUser->GetCID();
	kNot.m_wstrNickName				= spRoomUser->GetNickName();
	kNot.m_iDifficultyLevel			= (int) m_cDifficultyLevel;
	kNot.m_iIsChallenge				= ( m_cDungeonMode == CXSLDungeon::DM_HENIR_CHALLENGE ? 1 : 0 );
	kNot.m_iUnitClass				= (int) spRoomUser->GetUnitClass_LUA();
	kNot.m_iClear					= iClearValue;
	kNot.m_iStartNumMember			= m_iStartedNumMember;
	kNot.m_iEndNumMember			= GetNumMember();
	kNot.m_ucLevel					= spRoomUser->GetLevel();
	kNot.m_iPlayTime				= (int) GetTotalPlayTime();
	kNot.m_iEXP						= kUpdateUnitInfo.m_iEXP;
	kNot.m_iED						= spRoomUser->GetRewardED();
	kNot.m_cComboRank				= kDungeonUnitInfo.m_cComboRank;
	kNot.m_iComboScore				= spRoomUser->GetComboScore_LUA();
	kNot.m_cTechnicalRank			= kDungeonUnitInfo.m_cTechnicalRank;
	kNot.m_iTechnicalScore			= spRoomUser->GetTechnicalScore_LUA();
	kNot.m_cTimeRank				= kDungeonUnitInfo.m_cTimeRank;
	kNot.m_cDamagedRank				= kDungeonUnitInfo.m_cDamagedRank;
	kNot.m_iDamageNum				= spRoomUser->GetDamageNum_LUA();
	kNot.m_cTotalRank				= kDungeonUnitInfo.m_cTotalRank;
	kNot.m_iRessurectionStoneCount	= spRoomUser->GetUsedRessurectionStoneCount();
	kNot.m_iPassedStageCount		= spRoomUser->GetPassedStageCount();
	kNot.m_iPassedSubStageCount		= spRoomUser->GetPassedSubStageCount();
	//{{ 2010. 9. 14	������	�� �ý���
#ifdef SERV_PET_SYSTEM
	kNot.m_bIsWithPet				= spRoomUser->HavePet();
#endif SERV_PET_SYSTEM
	//}}
	//{{ 2010. 11. 8	������	���� �α� DB
#ifdef SERV_DUNGEON_LOG_DB
	kNot.m_wstrRegDate				= (const wchar_t*)kRegDate.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
#endif SERV_DUNGEON_LOG_DB
	//}}
	//{{ 2011. 03. 16	������	���� �α� �÷� �߰�
#ifdef SERV_ADD_DUNGEON_LOG_COLUMN
	kNot.m_sNpcDieCount				= spRoomUser->GetKillNPC_LUA();
	kNot.m_iGivenDamage				= spRoomUser->GetTotalGivenDamamge_LUA();
	kNot.m_iAttackDamage			= spRoomUser->GetTotalAttackedDamage_LUA();
	kNot.m_sUsingSkillKind			= spRoomUser->GetUsingKindOfSkill_LUA();
	kNot.m_iTotalEXP				= ( spRoomUser->GetRewardEXP() + spRoomUser->GetRewardPartyEXP() + kUpdateUnitInfo.m_iEXP );
#ifdef SERV_PETID_DATA_TYPE_CHANGE // 2013.07.02
	kNot.m_iPetID					= spRoomUser->GetPetID();
#else //SERV_PETID_DATA_TYPE_CHANGE
	kNot.m_cPetID					= spRoomUser->GetPetID();
#endif //SERV_PETID_DATA_TYPE_CHANGE
	kNot.m_cPetEvoStep				= spRoomUser->GetPetEvolutionStep();
#endif SERV_ADD_DUNGEON_LOG_COLUMN
	//}}
	//{{ 2012. 03. 05	��μ�	���� ��� �α�
#ifdef SERV_DUNGEON_DASH_LOG
	kNot.m_iDashCount				= spRoomUser->GetDashCount();
#endif SERV_DUNGEON_DASH_LOG
	//}}
	//{{ 2012. 04. 17	������	���� �α� �÷� �߰�
#ifdef SERV_ADD_DUNGEON_LOG_COLUMN_NUM_2
	kNot.m_iBaseHP					= spRoomUser->GetBaseHP_LUA();
	kNot.m_iEndHP					= spRoomUser->GetLastUpdatedHP();
	kNot.m_iRecovHP					= spRoomUser->GetTotalRecoveryHP_LUA();
	kNot.m_iContinue				= spRoomUser->GetUsedRessurectionStoneCount();
	kNot.m_iStartContinue			= spRoomUser->GetStartTimeNumResurrectionStone();
	kNot.m_iContinueUseStage		= spRoomUser->GetFirstUseResurrectionStoneStageID();
	kNot.m_iContinueUseSub			= spRoomUser->GetFirstUseResurrectionStoneSubStageID();
	kNot.m_iFrame					= spRoomUser->GetInGameAverageFrame();
	kNot.m_wstrChannelIP			= spRoomUser->GetGameChannelIP();
#endif SERV_ADD_DUNGEON_LOG_COLUMN_NUM_2
	//}}
	//{{ 2012. 07. 11 ��μ�
#ifdef SERV_ADD_DUNGEON_LOG_COLUMN_NUM_3
	kNot.m_iUserUID					= spRoomUser->GetUserUID();
#endif SERV_ADD_DUNGEON_LOG_COLUMN_NUM_3
	//}}
	//{{ 2012. 11. 6	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	kNot.m_cAutoPartyPlay			= ( IsStartedByAutoParty() ? 1 : 0 );
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}
	//{{ �ڵ� ��Ƽ�� ���� ���� �� ��Ī ��� �ð� �α� �߰� - ��μ�
#ifdef SERV_AUTO_PARTY_MATCHING_REQUIRED_TIME_LOG
	kNot.m_iAutoPartyWaitTime		= spRoomUser->GetAutoPartyWaitTime();
#endif SERV_AUTO_PARTY_MATCHING_REQUIRED_TIME_LOG
	//}
	//{{ 2012. 12. 12	������	��Ʋ�ʵ� �ý���
#ifdef SERV_CHECK_USER_NEVER_RETURN_TO_FIELD
	kNot.m_iLeaveReason				= iLeaveReason;
	kNot.m_cRoomState				= GetStateID();
#endif SERV_CHECK_USER_NEVER_RETURN_TO_FIELD
	//}}
	//{{ 2012. 12. 12  ��ų ���� ��� Ÿ�� �α� - ��μ�
#ifdef SERV_USE_SKILL_SLOT_TYPE_LOG
	kNot.m_bSkillSlotType				= spRoomUser->GetSkillSlotType();
#endif SERV_USE_SKILL_SLOT_TYPE_LOG
	//}}
	KSIManager.QueueingEvent( E_LOCAL_LOG_DUNGEON_NOT, kNot );
	//////////////////////////////////////////////////////////////////////////
	
	// DB���� ������
	SendToLogDB( DBE_DB_LOG_DUNGEON_NOT, kNot );

	//{{ 2013. 01. 09 ���� ���� �ý��� - ��μ�
#ifdef SERV_DUNGEON_FORCED_EXIT_SYSTEM
	bool bExitResult = false;

	if( iLeaveReason == NetError::NOT_LEAVE_ROOM_REASON_34 )
	{
		bExitResult = true;
	}

	// ���� ���� �ý��� �α� �������
	// ���� �ο��� 2�� �̻��� ���
	if( m_bStartedByAutoParty == true && m_iStartedNumMember >= 2 )
	{
		KBadAttitudeManager::KBadAttitudeInfo kUnitData( spRoomUser->GetCID() );
		if( m_kBadAttitudeManager.GetUnitData( spRoomUser->GetCID(), kUnitData ) == false )
		{
			START_LOG( cerr, L"�������� �ʴ� ������ ������� �Ѵ�." )
				<< BUILD_LOG( spRoomUser->GetCID() )
				<< END_LOG;

			// �׷��� ��� ���� �Ѵ�.
		}
		else
		{
			if( kUnitData.m_iBadAttitudePoint > 0 )
			{
				// ���� �ý������� ���� ���� �α� �߰�
				KERM_BAD_ATTITUDE_USER_LOG_NOT kLog;
				kLog.m_iUnitUID = spRoomUser->GetCID();
				kLog.m_iDungeonIDAndDif = GetDungeonIDAndDif();
				kLog.m_sStartMember = static_cast<short>(m_iStartedNumMember);
				kLog.m_sNowMember = static_cast<short>(GetNumMember());
				kLog.m_sVoteCount = static_cast<short>(kUnitData.m_iVotePoint);
				kLog.m_sExitStage = static_cast<short>(kUnitData.m_kRankInfo.m_iStage);
				kLog.m_sExitSubStage = static_cast<short>(kUnitData.m_kRankInfo.m_iSubStage);
				kLog.m_sBadAttitudePoint = static_cast<short>(kUnitData.m_iBadAttitudePoint);
				kLog.m_wstrRegDate = (const wchar_t*)kRegDate.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
				kLog.m_bExitResult = bExitResult;

				SendToGSCharacter( spRoomUser->GetGSUID(), spRoomUser->GetCID(), ERM_BAD_ATTITUDE_USER_LOG_NOT, kLog );
			}
		}
	}
#endif SERV_DUNGEON_FORCED_EXIT_SYSTEM
	//}

}
//#endif SERV_BATTLE_FIELD_SYSTEM
//}}

void KDungeonRoom::ProcessNativeEvent( const KEventPtr& spEvent_ )
{
    switch( spEvent_->m_usEventID )
    {
   _CASE( ERM_OPEN_DUNGEON_ROOM_REQ, KERM_OPEN_ROOM_REQ );

	//////////////////////////////////////////////////////////////////////////
    // party
	CASE( ERM_PARTY_OPEN_DUNGEON_ROOM_REQ );
   //_CASE( ERM_PARTY_GAME_REPLAY_START_REQ, KERM_PARTY_OPEN_DUNGEON_ROOM_REQ );
   //{{ 2011. 12. 09	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
   _CASE( ERM_SINGLE_OPEN_DUNGEON_ROOM_REQ, KERM_OPEN_ROOM_REQ );
   //_CASE( ERM_SINGLE_DUNGEON_GAME_REPLAY_REQ, KERM_OPEN_ROOM_REQ );
#endif SERV_BATTLE_FIELD_SYSTEM
   //}}
	// party
	//////////////////////////////////////////////////////////////////////////

    CASE( ERM_GAME_START_REQ );
	CASE( ERM_LEAVE_ROOM_REQ );
	//{{ 2011. 11. 1	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
   _CASE( ERM_LEAVE_ROOM_FOR_JOIN_BATTLE_FIELD_REQ, KERM_LEAVE_ROOM_REQ );
   _CASE( ERM_LEAVE_ROOM_FOR_ENTER_THE_VILLAGE_REQ, KERM_LEAVE_ROOM_REQ );
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}
   _CASE( ERM_DUNGEON_STAGE_LOAD_REQ, KEGS_DUNGEON_STAGE_LOAD_REQ );
   _CASE( ERM_DUNGEON_STAGE_LOAD_COMPLETE_REQ, KEGS_DUNGEON_STAGE_LOAD_COMPLETE_REQ );

   _CASE( ERM_DUNGEON_SUB_STAGE_OPEN_REQ,			KEGS_DUNGEON_SUB_STAGE_OPEN_REQ );
   _CASE( ERM_DUNGEON_SUB_STAGE_GO_NEXT_REQ,		KEGS_DUNGEON_SUB_STAGE_GO_NEXT_REQ );
   _CASE( ERM_DUNGEON_SUB_STAGE_LOAD_COMPLETE_REQ,	KEGS_DUNGEON_SUB_STAGE_LOAD_COMPLETE_REQ );

	//{{ 2009. 7. 20  ������	��н�������
	CASE_NOPARAM( ERM_DUNGEON_SECRET_STAGE_ENTER_CHECK_REQ );
   _CASE( ERM_DUNGEON_SECRET_STAGE_ENTER_SELECT_NOT, KEGS_DUNGEON_SECRET_STAGE_ENTER_SELECT_NOT );
	//}}

   _CASE( ERM_NPC_UNIT_CREATE_REQ, KEGS_NPC_UNIT_CREATE_REQ );
   _CASE( ERM_NPC_UNIT_DIE_REQ, KEGS_NPC_UNIT_DIE_REQ );
   _CASE( ERM_USER_UNIT_DIE_REQ, KEGS_USER_UNIT_DIE_REQ );
    CASE_NOPARAM( ERM_USER_UNIT_DIE_COMPLETE_REQ );
   
   _CASE( ERM_GET_ITEM_REQ, KEGS_GET_ITEM_REQ );
	//{{ 2009. 5. 11  ������	�ǽð�������
	CASE( ERM_GET_ITEM_COMPLETE_NOT );
	//}}
	CASE( ERM_MY_USER_UNIT_INFO_TO_SERVER_REQ );
   _CASE( ERM_DUNGEON_PLAY_INFO_TO_SERVER_NOT, KEGS_DUNGEON_PLAY_INFO_TO_SERVER_NOT );
   _CASE( ERM_END_GAME_REQ, KEGS_END_GAME_REQ );
   _CASE( ERM_USE_QUICK_SLOT_NOT, KEGS_USE_QUICK_SLOT_NOT );

   _CASE( ERM_CHECK_INVALID_USER_NOT, UidType );

   _CASE( ERM_RESURRECT_TO_CONTINUE_DUNGEON_REQ, KEGS_RESURRECT_TO_CONTINUE_DUNGEON_REQ );
	CASE_NOPARAM( ERM_SKIP_CONTINUE_DUNGEON_NOT );

    CASE( ERM_TUTORIAL_DUNGEON_ROOM_UPDATE_NOT );
	
   _CASE( ERM_STOP_DUNGEON_CONTINUE_TIME_REQ, KEGS_STOP_DUNGEON_CONTINUE_TIME_REQ );

   _CASE( ERM_FIELD_UNIT_SYNC_DATA_NOT, KEGS_FIELD_UNIT_SYNC_DATA_NOT );

	CASE_NOPARAM( ERM_PARTY_BAN_USER_REQ );

	//{{ 2009. 12. 29  ������	NPCKILLALL��ŷ����
	CASE_NOPARAM( ERM_DUNGEON_KILLALLNPC_CHECK_REQ );
	//}}
	//{{ 2010. 11. 18	������	������ �Ǹ� ����
#ifdef SERV_AUTO_HACK_CHECK_SELL_ITEM
   _CASE( ERM_CHECK_SELL_ED_ITEM_REQ, KEGS_SELL_ED_ITEM_REQ );
#endif SERV_AUTO_HACK_CHECK_SELL_ITEM
	//}}

   //{{ 2011. 01. 25  ��μ�  ���� Ʈ���� ����(������ ������ - ���� PVP Ȯ��)
#ifdef SERV_WORLD_TRIGGER_RELOCATION
   _CASE( ECN_WORLD_TRIGGER_RELOCATION_REQ, KEGS_WORLD_TRIGGER_RELOCATION_REQ );
#endif SERV_WORLD_TRIGGER_RELOCATION
   //}}
#ifdef SERV_CREATE_ELITE_FOR_ADMIN
   _CASE( ERM_ADMIN_NPC_UNIT_CREATE_REQ, KEGS_ADMIN_NPC_UNIT_CREATE_REQ );
#endif SERV_CREATE_ELITE_FOR_ADMIN
   
   //{{ 2012. 09. 02	�ڼ���	Merge ( �������� Ŭ���̾�Ʈ ���۵����� UDP ��Ŷ�� ���ƿ��� �ʴ� ������ �߰ߵǸ� �������� ű�Ѵ�. // 2012.06.11 lygan_������ )
#ifdef UDP_CAN_NOT_SEND_USER_KICK
   _CASE( ERM_UDP_CHECK_KICK_USER_NOT, KEGS_UDP_CHECK_KICK_USER_NOT );
#endif UDP_CAN_NOT_SEND_USER_KICK
   //}}
	//{{ 2012. 11. 9	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	CASE_NOPARAM( ERM_UPDATE_HENIR_REWARD_USER_NOT );
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}
	//{{ 2013. 01. 09 ���� ���� �ý��� - ��μ�
#ifdef SERV_DUNGEON_FORCED_EXIT_SYSTEM
	_CASE( ERM_BAD_ATTITUDE_USER_CHECK_INFO_NOT, KEGS_BAD_ATTITUDE_USER_CHECK_INFO_NOT );
	_CASE( ERM_DEFENCE_DUNGEON_WAVE_ID_NOT, KEGS_DEFENCE_DUNGEON_WAVE_ID_NOT );
	_CASE( ERM_FORCED_EXIT_VOTE_REQ, KEGS_FORCED_EXIT_VOTE_REQ );
#endif SERV_DUNGEON_FORCED_EXIT_SYSTEM
	//}
	//{{ 2013. 02. 01  �̺�Ʈ �߷�Ÿ�� ���� - ��μ�
#ifdef SERV_EVENT_VALENTINE_DUNGEON
	CASE_NOPARAM( ERM_SYNC_DUNGEON_TIMER_NOT );
#endif SERV_EVENT_VALENTINE_DUNGEON
	//}
	//{{ 2013. 04. 15	������	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
	CASE( ERM_PREPARE_FOR_DEFENCE_DUNGEON_ACK );
#endif SERV_NEW_DEFENCE_DUNGEON
	//}}
	//{{ 2013. 04. 22	������	����� �� ���� ���� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON_ENTER_LOGIC
	_CASE( ERM_ENTER_DEFENCE_DUNGEON_POPUP_REPLY_NOT, KEGS_ENTER_DEFENCE_DUNGEON_POPUP_REPLY_NOT );
#endif SERV_NEW_DEFENCE_DUNGEON_ENTER_LOGIC
	//}}

#ifdef  SERV_OPTIMIZE_DETECT_ZOMBIE_HOST
    CASE_NOPARAM( ERM_BATTLE_FIELD_ZOMBIE_ALERT_NOT );
#endif  SERV_OPTIMIZE_DETECT_ZOMBIE_HOST

#ifdef SERV_STAGE_CLEAR_IN_SERVER
	_CASE( ERM_DUNGEON_SUB_STAGE_CLEAR_REQ, KEGS_DUNGEON_SUB_STAGE_CLEAR_REQ );
	_CASE( ERM_SECRET_STAGE_LOAD_REQ, KEGS_SECRET_STAGE_LOAD_REQ );
#endif SERV_STAGE_CLEAR_IN_SERVER

    default:
        START_LOG( cerr, L"�̺�Ʈ �ڵ鷯�� ���ǵ��� �ʾ��� - " << spEvent_->GetIDStr() )
            << BUILD_LOG( spEvent_->m_usEventID );
    }
}

void KDungeonRoom::StartGame()
{
    KRoom::StartGame();

	//{{ 2012. 02. 06    ��μ�    ���� ���� UID �߱� ��� ����
//#ifdef SERV_MAKE_MONSTER_UID_CHANGE
//	m_nNPCUID = GetDungeonNpcRandomInitUid();
//#else
//	m_nNPCUID			= 1;
//#endif SERV_MAKE_MONSTER_UID_CHANGE
	//}}

	//{{ 2011. 10. 24	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	m_kDungeonMonsterManager.StartGame();
	m_kDungeonSecurityManager.StartGame();
#else
	m_nNPCUID			= 1;
	m_mapNPCData.clear();
	m_mapSubStageNpcInfo.clear();	// NPCKILLALL��ŷ����
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}
	m_nDropItemUID		= 1;
	//{{ 2011. 06. 13	������	�������� �α�
#ifdef SERV_DUNGEON_STAGE_LOG
	m_iLoadingCompleteStageID = -1;
#endif SERV_DUNGEON_STAGE_LOG
	//}}
	m_iStageID			= 0;
	m_iSubStageID		= 0;
	m_fTotalPlayTime	= 0.0;
	m_fLoadingTime		= 0.0;

	m_bIsWin			= false;

	m_kDSSGoNextAllNot.m_iBeforeStageID = -1;
	m_kDSSGoNextAllNot.m_iNextStageID	= -1;

	m_mapDropItem.clear();
	//{{ 2009. 5. 13  ������	���꽺�������� drop npc��
	m_mapSubStageExpNpcNum.clear();
	//}}
	m_mapAttribNpcData.clear(); // �Ӽ� ����	
	//{{ 2010. 11. 20	������	Ư�� �ð� ���� ����
#ifdef SERV_TIME_DROP_MONSTER_EVENT
	m_mapTimeDropEventMonster.clear();
#endif SERV_TIME_DROP_MONSTER_EVENT
	//}}
	//{{ 2011. 09. 07  ��μ�	���� �������� DIE NPC �ð� üũ ���
#ifdef SERV_SUB_STAGE_NPC_DIE_CHECK
	m_mapSubStageActiveNpcNum.clear();
#endif SERV_SUB_STAGE_NPC_DIE_CHECK
	//}}

	m_usLoadingState	= 0;

	//{{ 2010. 04. 07  ������	��д��� ����
#ifdef SERV_SECRET_HELL
	m_iExtraStageLoadCount	= 0;
	m_bAdventExtraNpc		= false;	
	m_fPoisonTimeLimit		= SiCXSLDungeonManager()->GetPoisonTimeLimit( GetDungeonIDAndDif() );
#endif SERV_SECRET_HELL
	//}}
    
    const int iPlayerCount = m_spRoomUserManager->GetNumPlaying();
    if( iPlayerCount < 1 || iPlayerCount > 4 )
    {
        START_LOG( cerr, L"�÷��� ���� ���� ���� �̻���." )
            << BUILD_LOG( iPlayerCount )
            << END_LOG;
    }

	// ���� Ÿ�� ���
	m_eDungeonType = SiCXSLDungeonManager()->GetDungeonType( GetDungeonIDAndDif() );
	switch( m_eDungeonType )
	{
	case CXSLDungeon::DT_HENIR:
		{
			// ��ϸ� �ð� ��������� ĳ���� ������ 1�� �����Ǳ� ������ ���� �ְ������� 1�� �д�
			if( m_cDungeonMode == CXSLDungeon::DM_HENIR_CHALLENGE )
			{
				m_iHighestUserLevelAtDungeonStart = 1;
				//{{ 2012. 05. 14	������	���� ���� ���� ����
#ifdef SERV_DUNGEON_NPC_LEVEL
				m_iAverageUserLevelAtDungeonStart = 1;
#endif SERV_DUNGEON_NPC_LEVEL
				//}}
			}
			else
			{
				m_iHighestUserLevelAtDungeonStart = m_spRoomUserManager->GetHighestUserLevel();

				//{{ 2012. 05. 14	������	���� ���� ���� ����
#ifdef SERV_DUNGEON_NPC_LEVEL
				// ��� �������� ���ϵ� �ִ뷹������ ���̰� 5���̻��� ������ ��꿡 �������� ����
				m_iAverageUserLevelAtDungeonStart = m_spRoomUserManager->GetAverageUserLevel( CXSLDungeon::DE_CALC_AVERAGE_INCLUDE_LEVEL_GAP );
#endif SERV_DUNGEON_NPC_LEVEL
				//}}
			}
		}
		break;

	default:
		{
			m_iHighestUserLevelAtDungeonStart = m_spRoomUserManager->GetHighestUserLevel();

			//{{ 2012. 05. 14	������	���� ���� ���� ����
#ifdef SERV_DUNGEON_NPC_LEVEL
			// ��� �������� ���ϵ� �ִ뷹������ ���̰� 5���̻��� ������ ��꿡 �������� ����
			m_iAverageUserLevelAtDungeonStart = m_spRoomUserManager->GetAverageUserLevel( CXSLDungeon::DE_CALC_AVERAGE_INCLUDE_LEVEL_GAP );
#endif SERV_DUNGEON_NPC_LEVEL
			//}}
		}
		break;
	}

	if( GetHighestUserLevelAtDungeonStart() <= 0 )
	{
		START_LOG( cerr, L"���� ���۽ÿ� ���� �ְ����� ��ġ�� �̻��մϴ�." )
			<< BUILD_LOG( iPlayerCount )
			<< BUILD_LOG( GetHighestUserLevelAtDungeonStart() )
			<< BUILD_LOG( GetDungeonID() )
			<< BUILD_LOGc( GetDifficultyLevel() )
			<< END_LOG;

		m_iHighestUserLevelAtDungeonStart = SiKGameSysVal()->GetLimitsLevel();
	}

	//{{ 2012. 05. 10	������	���� ���� ���� ����
#ifdef SERV_DUNGEON_NPC_LEVEL
	if( GetAverageUserLevelAtDungeonStart() <= 0 )
	{
		START_LOG( cerr, L"���� ���۽ÿ� ���� ��շ��� ��ġ�� �̻��մϴ�." )
			<< BUILD_LOG( iPlayerCount )
			<< BUILD_LOG( GetAverageUserLevelAtDungeonStart() )
			<< BUILD_LOG( GetDungeonID() )
			<< BUILD_LOGc( GetDifficultyLevel() )
			<< END_LOG;

		m_iAverageUserLevelAtDungeonStart = SiKGameSysVal()->GetLimitsLevel();
	}
#endif SERV_DUNGEON_NPC_LEVEL
	//}}	

	//////////////////////////////////////////////////////////////////////////
	// [�̺�Ʈ]	�̺�Ʈ ���� ���
#ifdef SERV_EVENT_MONSTER
	m_vecEventMonster.clear();

	if( CXSLDungeon::IsTutorialDungeon( GetDungeonID() ) == false  &&
		CXSLDungeon::IsTCDungeon( GetDungeonID() ) == false  &&
		CXSLDungeon::IsHenirDungeon( GetDungeonID() ) == false )
	{
		std::vector< int > vecEventID;
		SiKGameEventManager()->CheckEventMonster( vecEventID );
		//{{ 2012. 02. 03	�ڼ���	�̺�Ʈ �������� ó����� ���� ( Script -> DB )
#ifdef SERV_CHANGE_EVENT_INFO_SCRIPT_TO_DB
		SiKEventMonsterRefreshManager()->GetAdventNpcEventInfo( vecEventID, m_vecEventMonster );
#else
		SiKGameEventScriptManager()->GetAdventNpcEventInfo( vecEventID, m_vecEventMonster );
#endif SERV_CHANGE_EVENT_INFO_SCRIPT_TO_DB
		//}}
	}
#endif SERV_EVENT_MONSTER
	//////////////////////////////////////////////////////////////////////////
	
	//{{ 2010. 04. 07  ������	�Ӽ���� �̺�Ʈ ��
#ifdef SERV_SECRET_HELL
	m_iAttribNpcStageCnt = 0;
	m_iAttribNpcDropCnt = 0;

	switch( m_eDungeonType )
	{
	case CXSLDungeon::DT_NORMAL:
		{
			// ���� ���������� �Ӽ� ���� �ȳ��´�.
			if( CXSLDungeon::DI_RUBEN_EL_TREE_NORMAL != GetDungeonID() )
			{
				// �Ӽ� ���� ��� ī��Ʈ
				//{{ 2011. 03. 21  ��μ�	���� ���̵� ����
#ifdef SERV_DUNGEON_DIFFICULTY_REVISION
				SiKAttribNpcTable()->GetAttribNpcDropCount( static_cast<char>(m_eDungeonType), m_cDifficultyLevel, iPlayerCount, m_iAttribNpcStageCnt, m_iAttribNpcDropCnt );
#else
				SiKAttribNpcTable()->GetAttribNpcDropCount( static_cast<char>(m_eDungeonType), m_cDifficultyLevel, m_iAttribNpcStageCnt, m_iAttribNpcDropCnt );
#endif SERV_DUNGEON_DIFFICULTY_REVISION
				//}}
			}
		}
		break;

	case CXSLDungeon::DT_HENIR: // ��ϸ� �ð������� �Ӽ����� �ȳ���.
		break;

	case CXSLDungeon::DT_SECRET:
		{
			// �Ӽ� ���� ��� ī��Ʈ
			//{{ 2011. 03. 21  ��μ�	���� ���̵� ����
#ifdef SERV_DUNGEON_DIFFICULTY_REVISION
			SiKAttribNpcTable()->GetAttribNpcDropCount( static_cast<char>(m_eDungeonType), m_cDungeonMode, iPlayerCount, m_iAttribNpcStageCnt, m_iAttribNpcDropCnt );
#else
			SiKAttribNpcTable()->GetAttribNpcDropCount( static_cast<char>(m_eDungeonType), m_cDungeonMode, m_iAttribNpcStageCnt, m_iAttribNpcDropCnt );
#endif SERV_DUNGEON_DIFFICULTY_REVISION
			//}}
		}
		break;

	default:
		{
            START_LOG( cerr, L"�߸��� ���� Ÿ���Դϴ�." )
				<< BUILD_LOG( m_eDungeonType )
				<< END_LOG;
		}
		break;
	}
#else
#pragma ELSE_MESSAGE
	// �Ӽ� ���� ��� ī��Ʈ
	//SiKAttribNpcTable()->GetAttribNpcDropCount( static_cast<int>(m_cDifficultyLevel), m_iAttribNpcStageCnt, m_iAttribNpcDropCnt );
#endif SERV_SECRET_HELL
	//}}

	//{{ 2010. 10. 26	������	����Ʈ ���� �߰�
#ifdef SERV_QUEST_CLEAR_EXPAND
	// �ش� ������ Ư�� ������ ���� ����Ʈ�� �������̸鼭 �������� �����ϰ� �ִ��� Ȯ���Ѵ�.
	m_spRoomUserManager->CheckItemEquipQuestCondition( true );
#endif SERV_QUEST_CLEAR_EXPAND
	//}}

	//{{ 2010. 06. 17  ������	������ ����
#ifdef SERV_PROTECT_AUTO_HACK
	m_iHackUserCheckCount = ( rand() % 6 ) + 5; // 5���� 10������ üũ ���� ���!
	m_iNpcDieHackCount = 0;
	m_iGetItemHackCount = 0;
	m_iStartedNumMember = iPlayerCount; // ���� ������ ����� ���� �� ����!
#endif SERV_PROTECT_AUTO_HACK
	//}}

	//{{ 2010. 7. 28  ������	������ ���� NPC
#ifdef SERV_AUTO_HACK_CHECK_NPC
    m_setFindedHackUserUnitUID.clear();
#endif SERV_AUTO_HACK_CHECK_NPC
	//}}

	//{{ 2010. 11. 18	������	���� �� �������� �ε� üũ
#ifdef SERV_AUTO_HACK_CHECK_STAGE_LOAD
	m_mapStageLoadingCheck.clear();
	m_iLastLoadingUnitUID = 0;
#endif SERV_AUTO_HACK_CHECK_STAGE_LOAD
	//}}

	//{{ 2010. 12. 8	������	���� ���� ��� ���
#ifdef SERV_APPROPRIATE_LEVEL_STAT
	{
		KELOG_APPROPRIATE_LEVEL_STATISTICS_NOT kPacketNot;
		kPacketNot.m_iDungeonID = GetDungeonIDAndDif();
		kPacketNot.m_cDifficultyLevel = GetDifficultyLevel();
		kPacketNot.m_tDungeonStartTime = CTime::GetCurrentTime().GetTime();
		m_spRoomUserManager->GetAppropriateLevelStatistics( kPacketNot );
        SendToLogDB( ELOG_APPROPRIATE_LEVEL_STATISTICS_NOT, kPacketNot );
	}
#endif SERV_APPROPRIATE_LEVEL_STAT
	//}}

	//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER
	m_iDefenseMainCrystalNpcUID = 0;
	m_setDefenseSmallCrystalNpcUID.clear();
#endif SERV_INSERT_GLOBAL_SERVER
	//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�

	//{{ 2012. 02. 06    ��μ�    ���� ���� UID �߱� ��� ����
#ifdef SERV_MAKE_MONSTER_UID_CHANGE
	SetHackingNpcUid( false );
#endif SERV_MAKE_MONSTER_UID_CHANGE	
	//}}
	//{{ 2013. 01. 09 ���� ���� �ý��� - ��μ�
#ifdef SERV_DUNGEON_FORCED_EXIT_SYSTEM
	m_kBadAttitudeManager.init( m_spRoomUserManager, GetDungeonIDAndDif() );
#endif SERV_DUNGEON_FORCED_EXIT_SYSTEM
	//}

	//{{ 2013. 02. 01  �̺�Ʈ �߷�Ÿ�� ���� - ��μ�
#ifdef SERV_EVENT_VALENTINE_DUNGEON
	m_kValentineManager.Init();
#endif SERV_EVENT_VALENTINE_DUNGEON
	//}
	//{{ 2013. 04. 09	������	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
	m_kDefenceDungeonManager.StartGame();
#endif SERV_NEW_DEFENCE_DUNGEON
	//}}
#ifdef SERV_CREATED_NPC_LIMITED_DROPS
	m_mapDroppedTimes.clear();
#endif SERV_CREATED_NPC_LIMITED_DROPS

#ifdef  SERV_CHOOSE_FASTEST_HOST
#ifdef  SERV_OPTIMIZE_CHOOSE_FASTEST_HOST_ENHANCE
    m_iSubStageCount = 0;
    m_kTimer[TM_CHECK_CHANGE_HOST].restart();
    m_bCheckPingScoreExecuted = false;
#endif  SERV_OPTIMIZE_CHOOSE_FASTEST_HOST_ENHANCE
#endif  SERV_CHOOSE_FASTEST_HOST

#ifdef SERV_STAGE_CLEAR_IN_SERVER
	m_kNextStageData.ClearData();
	m_bFirstStage = true;
	m_bFirstSubStage = true;
	m_iClearConditionIndex = 0;
#endif SERV_STAGE_CLEAR_IN_SERVER
}

void KDungeonRoom::StartPlay()
{
	//{{ 2010. 8. 25	������	���� �÷��� ���� üũ!
	if( GetStateID() == KRoomFSM::S_PLAY )
	{
		if( m_spRoomUserManager->GetNumMember() == 1 )
		{
			START_LOG( cout, L"�̹� KRoomFSM::S_PLAY�����ε� �� StartPlay�� �Ϸ��� �ϳ�!? ��ŷ�ΰ�?" )
				<< BUILD_LOG( GetStateIDString() )
				<< BUILD_LOG( GetTotalPlayTime() )
				<< BUILD_LOG( m_kTimer[TM_PLAY].elapsed() )
				<< BUILD_LOG( GetLoadingTime() )
				<< BUILD_LOG( GetDungeonIDAndDif() )
				<< BUILD_LOG( GetStageID() )
				<< BUILD_LOG( GetSubStageID() );

			KRoomUserPtr spUser = m_spRoomUserManager->GetHostUser();
			if( spUser != NULL )
			{
				START_LOG( cout, L"� ������ �̷��� �̻��� ������ �ϴ��� Ȯ���غ���!" )
#ifdef SERV_PRIVACY_AGREEMENT
					<< BUILD_LOG( spUser->GetCID() );
#else
					<< BUILD_LOG( spUser->GetCID() )
					<< BUILD_LOG( spUser->GetNickName() );
#endif SERV_PRIVACY_AGREEMENT

				// ��Ƽ�忡�� ��ŷüũ��Ŷ�� ������!
				//KEGS_SERVER_CHECK_HACK_USER_NOT kNot;
				//kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_DUNGEON_PLAY_START;
				//SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SERVER_CHECK_HACK_USER_NOT, kNot );
			}
		}
	}
	//}}

    KRoom::StartPlay();

	//�����ʱ�ε��� �θ�Ŭ�������� ���Ǵ� �ε��ð��� üũ�ϱ�����
	//�̰����� üũ�� �����Ѵ�.

	// �ε� Ÿ�̸� ����!
	if( m_LoadingTimer.IsStart() == true )
	{
		// �̹� ���۵� ���¶�� ���� ���·� �ʱ�ȭ �մϴ�!
		m_LoadingTimer.End();
	}
	m_LoadingTimer.Start();

	// �������� ó�� �����Ҷ� ���ʷ� ������� Ÿ���� ������!
	if( m_eDungeonType == CXSLDungeon::DT_SECRET  &&  m_fPoisonTimeLimit > 0.0f )
	{
		KEGS_DUNGEON_EFFECT_REMAINING_TIME_NOT kNot;
		kNot.m_fRemainingTime = SiCXSLDungeonManager()->GetPoisonTimeLimit( GetDungeonIDAndDif() );
		BroadCast( ERM_DUNGEON_EFFECT_REMAINING_TIME_NOT, kNot );		
	}

#ifdef  SERV_CHOOSE_FASTEST_HOST
#ifdef  SERV_OPTIMIZE_CHOOSE_FASTEST_HOST_ENHANCE
    m_iSubStageCount = 0;
    m_kTimer[TM_CHECK_CHANGE_HOST].restart();
    m_bCheckPingScoreExecuted = false;
#endif  SERV_OPTIMIZE_CHOOSE_FASTEST_HOST_ENHANCE
#endif  SERV_CHOOSE_FASTEST_HOST
}

void KDungeonRoom::EndPlay()
{
	m_bEndPlay = false;
    KRoom::EndPlay();

    m_fTotalPlayTime = m_kTimer[TM_PLAY].elapsed() - GetLoadingTime();

	// �÷��� Ÿ���� ���������� ���Ǿ� �������� Ȯ���غ���!
	if( GetTotalPlayTime() < 0.0 )
	{
		START_LOG( cout, L"�ε��ð��� �� ���� ���� �÷��� �ð��� ���̳ʽ����̴�! ���� �� ���� ����!" )
			<< BUILD_LOG( GetStateIDString() )
			<< BUILD_LOG( GetTotalPlayTime() )
			<< BUILD_LOG( m_kTimer[TM_PLAY].elapsed() )
			<< BUILD_LOG( GetLoadingTime() )
			<< BUILD_LOG( GetDungeonIDAndDif() )
			<< BUILD_LOG( GetStageID() )
			<< BUILD_LOG( GetSubStageID() )
			<< BUILD_LOG( m_spRoomUserManager->GetNumMember() )
			<< BUILD_LOG( m_iStartedNumMember )
			<< END_LOG;
	}	
}

void KDungeonRoom::EndGame()
{
	// ��Ȱ�� ��� ���
	int iPlayerCount = m_spRoomUserManager->GetUsedRessurectionStonePlayerCount();

	KStatisticsKey kKey;
	kKey.m_vecIntKey.push_back( 0 );
	KSIManager.IncreaseCount( KStatistics::SI_RES_STONE, kKey, KStatistics::eSIColResStone_PlayerCount, iPlayerCount );

    KRoom::EndGame();

	// �����ϴ� ������ �̰����� �ٽ� �ʱ�ȭ�� �غ���.(07.12.28 demiahun)
	m_mapDropItem.clear();
	//{{ 2011. 10. 24	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	ReservedRegroupOldParty();
	m_kDungeonMonsterManager.EndGame();
	m_kDungeonSecurityManager.EndGame();
#else
	m_mapNPCData.clear();
	m_mapSubStageNpcInfo.clear();	// NPCKILLALL��ŷ����
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}
	
	//{{ 2009. 5. 13  ������	���꽺�������� drop npc��
	m_mapSubStageExpNpcNum.clear();
	//}}
	m_mapAttribNpcData.clear(); // �Ӽ� ����	
	//{{ 2010. 11. 20	������	Ư�� �ð� ���� ����
#ifdef SERV_TIME_DROP_MONSTER_EVENT
	m_mapTimeDropEventMonster.clear();
#endif SERV_TIME_DROP_MONSTER_EVENT
	//}}

	//{{ 2011. 09. 07  ��μ�	���� �������� DIE NPC �ð� üũ ���
#ifdef SERV_SUB_STAGE_NPC_DIE_CHECK
	m_mapSubStageActiveNpcNum.clear();
#endif SERV_SUB_STAGE_NPC_DIE_CHECK
	//}}
	//{{ 2013. 04. 09	������	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
	m_kDefenceDungeonManager.EndGame();
#endif SERV_NEW_DEFENCE_DUNGEON
	//}}
}

bool KDungeonRoom::CheckIfPlayEnd()
{
    if( KRoom::CheckIfPlayEnd() )
    {
        return true;
    }

    if( m_spRoomUserManager->IsAllPlayerDie() )
    {
        return true;
    }

    return false;
}

int KDungeonRoom::DecideWinTeam()
{
    if( /*m_spRoomUserManager->IsAllPlayerDie() || */m_bIsWin == false )
    {
        return CXSLRoom::TN_MONSTER;
    }
    else
    {
        return CXSLRoom::TN_RED;
    }
}

int KDungeonRoom::GetPlayMode()
{
    return m_iDungeonID + static_cast<int>(m_cDifficultyLevel);
}

void KDungeonRoom::HandleTimeOutUser( std::map< UidType, UidType >& mapUserCIDNGS )
{
    if( mapUserCIDNGS.empty() )
    {
        return;
    }

    std::map< UidType, UidType >::iterator mit;
    for( mit = mapUserCIDNGS.begin(); mit != mapUserCIDNGS.end(); mit++ )
    {
		if( !m_spRoomUserManager->LeaveRoom( mit->first ) )
		{
			START_LOG( cerr, L"�� ������ ����." )
				<< BUILD_LOG( mit->first )
				<< END_LOG;
		}

        // ���� �������� ��Ŷ�� ������.
        SendToGSCharacter( mit->second, mit->first, ERM_GAME_LOADING_TIME_OUT_NOT );
    }

    // ���� ��� �����鿡�� ���� ������ ������.
    KEGS_LEAVE_ROOM_NOT kPacketNot;
    kPacketNot.m_cRoomState = GetStateID();
    kPacketNot.m_iUnitUID = mit->first;
    GetRoomSlotInfo( kPacketNot.m_vecSlot );
    BroadCast( ERM_LEAVE_ROOM_NOT, kPacketNot );

	//{{ 2010. 11. 23	������	�� �ε� ���� ���� - ũ���� ������ �־ ������ �����մϴ�.
#ifdef SERV_ROOM_LOADING_BUG_FIX
	//////////////////////////////////////////////////////////////////////////
	if( GetStateID() == KRoomFSM::S_LOAD )
	{
		if( m_spRoomUserManager->IsAllPlayerFinishLoading() == true )
		{
			BroadCastID( ERM_GAME_LOADING_ALL_UNIT_OK_NOT );

			// ���������� �ʱ�ȭ.
			StartPlay();

			KEGS_PLAY_START_NOT kNot;
			//{{ 2011. 01. 17	������	ĳ���� ī��Ʈ ����
#ifdef SERV_CHAR_LOG
			kNot.m_iDungeonID = GetDungeonIDAndDif();
#endif SERV_CHAR_LOG
			//}}
			kNot.m_RoomState = GetStateID();
			GetRoomSlotInfo( kNot.m_vecSlot );

			// ��� �������� �÷��� ������ �˸�.
			BroadCast( ERM_PLAY_START_NOT, kNot );

			// ��� ���� ������ �� ����Ʈ�� �����϶�� ������.
			SendRoomListInfo( NetError::ERR_ROOM_15 );

			//{{ 2009. 4. 23  ������	������������Ʈ
			SendAllPVPUserInfo();
			//}}
		}
	}
	else if( GetStateID() == KRoomFSM::S_PLAY )
	{
		CheckLoadingSuccess();
	}
	//////////////////////////////////////////////////////////////////////////	
#else
	//////////////////////////////////////////////////////////////////////////
	if( GetStateID() == KRoomFSM::S_LOAD || GetStateID() == KRoomFSM::S_PLAY )
	{
		//// ��� ������ �ε� �������� not�� ������.
		//if( m_spRoomUserManager->IsAllPlayerFinishLoading() )
		//{
		//    BroadCastID( ERM_GAME_LOADING_ALL_UNIT_OK_NOT );

		//    // ���������� �ʱ�ȭ.
		//    StartPlay();

		//    KEGS_PLAY_START_NOT kNot;
		//    kNot.m_RoomState = GetStateID();
		//    GetRoomSlotInfo( kNot.m_vecSlot );

		//    // ��� �������� �÷��� ������ �˸�.
		//    BroadCast( ERM_PLAY_START_NOT, kNot );
		//}
		
		CheckLoadingSuccess();
	}
	//////////////////////////////////////////////////////////////////////////
#endif SERV_ROOM_LOADING_BUG_FIX
	//}}

    // ���������� ������ �Ǵ� ��� ó��.
    if( GetStateID() == KRoomFSM::S_PLAY )
    {
        if( CheckIfPlayEnd() )
        {
            EndPlay();
        }
		else
		{
			//{{ 2009. 7. 27  ������	��н�������
			if( m_eDungeonType == CXSLDungeon::DT_HENIR )
			{
				CheckEnterSecretStageSuccess();
			}
			//}}
		}
    }

    // �濡�� ��� ������ �������� ���� �ݴ´�.
    if( IsEmpty() )
    {
        StateTransition( KRoomFSM::I_TO_CLOSE );

		//{{ 2010. 06. 21  ������	��� ������
#ifdef SERV_STATISTICS_THREAD
		CTime kEndGameTime = CTime::GetCurrentTime();
		//////////////////////////////////////////////////////////////////////////
		KE_LOCAL_LOG_DUNGEON_ROOM_NOT kNot;
		kNot.m_iDungeonID		= m_iDungeonID + static_cast<int>(m_cDifficultyLevel);
		kNot.m_iDifficultyLevel	= (int) GetDifficultyLevel();
		kNot.m_iIsChallenge		= ( m_cDungeonMode == CXSLDungeon::DM_HENIR_CHALLENGE ? 1 : 0 );
		kNot.m_iWin				= 0;
		kNot.m_iLose			= 0;
		kNot.m_iStartNumMember	= m_iStartedNumMember;
		kNot.m_iEndNumMember	= GetNumMember();
		kNot.m_iTotalPlayTime	= static_cast<int>(GetTotalPlayTime());
		kNot.m_wstrEndGameTime	= (const wchar_t*)kEndGameTime.Format(_T("%H:%M:%S"));
		KSIManager.QueueingEvent( E_LOCAL_LOG_DUNGEON_ROOM_NOT, kNot );
		//////////////////////////////////////////////////////////////////////////
#endif SERV_STATISTICS_THREAD
		//}}
    }
    else
    {
        if( m_kDSSGoNextAllNot.m_iBeforeStageID != -1 &&
            m_kDSSGoNextAllNot.m_iNextStageID	!= -1 )
            SendDSSGoNextAllNot( m_kDSSGoNextAllNot );
    }
}

void KDungeonRoom::CloseRoom()
{
	// ���� ������ �����Ұ͵��� ��������.
	//{{ 2011. 10. 26	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	m_kDungeonMonsterManager.OnCloseRoom();
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}
	//{{ 2013. 04. 09	������	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
	m_kDefenceDungeonManager.OnCloseRoom();
#endif SERV_NEW_DEFENCE_DUNGEON
	//}}

	//������� ������ �ȵǾ� ������ ���ϴ� �����
	if( GetStateID() == KRoomFSM::S_INIT )
	{
		StateTransition( KRoomFSM::I_TO_WAIT );
	}

	StateTransition( KRoomFSM::I_TO_CLOSE );

	//{{ 2013. 01. 07	������	�濡�� �ʵ�� ���ƿ��� ���ϴ� ���� ����
#ifdef SERV_NEVER_RETURN_TO_FIELD_BUG_FIX
	// ���� �ݱ� ���� �ʱ�ȭ �ѹ� ������!
	m_spRoomUserManager->Reset();
#endif SERV_NEVER_RETURN_TO_FIELD_BUG_FIX
	//}}

	KCnBaseRoom::CloseRoom();
}

void KDungeonRoom::CheckResultSuccess()
{
	if( GetStateID() != KRoomFSM::S_RESULT )
		return;

	CheckDungeonUnitInfoSuccess();

	KRoom::CheckResultSuccess();
}

void KDungeonRoom::SendDSSGoNextAllNot( KEGS_DUNGEON_SUB_STAGE_GO_NEXT_ALL_NOT& kDSSGoNextAllNot )
{
	if( m_spRoomUserManager->IsAllPlayerStageID() == true )
	{
		m_spRoomUserManager->ResetStageID();
		//{{ 2012. 04. 17	������	���� �α� �÷� �߰�
#ifdef SERV_ADD_DUNGEON_LOG_COLUMN_NUM_2
		m_spRoomUserManager->ResetSubStageID();
#endif SERV_ADD_DUNGEON_LOG_COLUMN_NUM_2
		//}}

		BroadCast( ERM_DUNGEON_SUB_STAGE_GO_NEXT_ALL_NOT, kDSSGoNextAllNot );

		kDSSGoNextAllNot.m_iBeforeStageID	= -1;
		kDSSGoNextAllNot.m_iNextStageID		= -1;
	}
}

void KDungeonRoom::BroadCastUpdateUnitInfoWithPartyCheck( std::vector< KRoomUserPtr >& vecKRoomUserPtr,
														 unsigned short usEventID, 
														 std::vector< KERM_UPDATE_DUNGEON_UNIT_INFO_NOT >& data, int iDungeonID )
{
	for( u_int ui = 0; ui < vecKRoomUserPtr.size(); ++ui )
	{
		data[ui].m_iDungeonID	= iDungeonID;

		SendToGSCharacter( vecKRoomUserPtr[ui]->GetGSUID(), vecKRoomUserPtr[ui]->GetCID(), usEventID, data[ui] );
	}
}

//{{ 2011. 12.13    ��μ�	���� Ŭ���� �� ������ ���� �̺�Ʈ - ������ �ֹ���(�ߺ� ���� ����)
#ifdef SERV_DUNGEON_CLEAR_PAYMENT_ITEM_EVENT
void KDungeonRoom::BroadCastUpdateUnitInfoWithPartyCheck( std::vector< KRoomUserPtr >& vecKRoomUserPtr,
														 unsigned short usEventID, 
														 std::vector< KERM_UPDATE_DUNGEON_UNIT_INFO_NOT >& data, 
														 int iDungeonID,
														 std::map< UidType, bool >	mapHaveExpInDungeon )
{
	for( u_int ui = 0; ui < vecKRoomUserPtr.size(); ++ui )
	{
		std::map< UidType, bool >::iterator mit = mapHaveExpInDungeon.find( vecKRoomUserPtr[ui]->GetCID() );
		if( mit != mapHaveExpInDungeon.end() )
		{
			data[ui].m_bHaveExpInDungeon = mit->second;
			// �����Ͱ� ������ �⺻ false
		}

		data[ui].m_iDungeonID	= iDungeonID;

		SendToGSCharacter( vecKRoomUserPtr[ui]->GetGSUID(), vecKRoomUserPtr[ui]->GetCID(), usEventID, data[ui] );
	}
}
#endif SERV_DUNGEON_CLEAR_PAYMENT_ITEM_EVENT
//}}

KDropItemData KDungeonRoom::CreateItemProcess( IN int iItemID, IN bool bIsGP /* = false */, IN int iED /* = 0  */ )
{
	KDropItemData	kDropItem;	//not ��Ŷ�� �� ���� ����������
	ITEM_DATA		sItemData;	//�������� ���������� ����������

	int iDropItemUID = m_nDropItemUID++;

	kDropItem.m_iItemID		 = iItemID;
	kDropItem.m_iDropItemUID = iDropItemUID;

	if( (rand()%2) == 1 )
		kDropItem.m_bLeft = true;
	else
		kDropItem.m_bLeft = false;

	kDropItem.m_fSpeed	= (float)(rand()%800) + 1.0f;

	//�������� ����߸��� ���� ���� ���������� üũ,.
	sItemData.m_iItemID	 	 = iItemID;
	sItemData.m_iDropItemUID = iDropItemUID;
	sItemData.m_bIsGet		 = false;
	sItemData.m_bIsGP		 = bIsGP;	

	if( bIsGP == true )
	{
		sItemData.m_iED = iED;
	}
	//{{ 2013. 1. 14	�ڼ���	���� �� �ʵ� ��� �����ۿ� ��ȭ �ο�
#ifdef SERV_ENCHANT_ITEM_DROP_EVENT
	else
	{
		SiKDropTable()->EnchantItemDropEventProcess( iItemID, sItemData.m_cEnchantLevel, sItemData.m_bNotify );
		kDropItem.m_cEnchantLevel = sItemData.m_cEnchantLevel;
	}
#endif SERV_ENCHANT_ITEM_DROP_EVENT
	//}}

	m_mapDropItem.insert( std::make_pair( iDropItemUID, sItemData ) );

	return kDropItem;
}

void KDungeonRoom::CheckLoadingSuccess()
{
	if( m_usLoadingState != 0 )
	{
		switch( m_usLoadingState )
		{
		case ERM_DUNGEON_STAGE_LOAD_COMPLETE_REQ:
			{
				if( m_spRoomUserManager->IsAllPlayerStageLoaded() )
				{
					// Loading ���� ����.
					m_spRoomUserManager->ResetStageLoaded();

					//��� �ε�ó���� ������ ������ �˷���
					KEGS_DUNGEON_STAGE_START_NOT kPacketStartNot;
					kPacketStartNot.m_iStageID = m_iStageID;
					BroadCast( ERM_DUNGEON_STAGE_START_NOT, kPacketStartNot );

					//{{ 2009. 12. 14  ������	��ϸ��ӽ��κ�
					CheckTempInventoryClear();
					//}}

					//{{ 2011. 06. 13	������	�������� �α�
#ifdef SERV_DUNGEON_STAGE_LOG
					// �ε� �Ϸ�� ��������ID�� ����Ѵ�!
					m_iLoadingCompleteStageID = GetStageID();
#endif SERV_DUNGEON_STAGE_LOG
					//}}

#ifdef  SERV_CHOOSE_FASTEST_HOST
#ifdef  SERV_OPTIMIZE_CHOOSE_FASTEST_HOST_ENHANCE
                    if ( m_iSubStageCount == 0 )
                    {
                        m_kTimer[TM_CHECK_CHANGE_HOST].restart();
                        m_bCheckPingScoreExecuted = false;
                    }
#endif  SERV_OPTIMIZE_CHOOSE_FASTEST_HOST_ENHANCE
#endif  SERV_CHOOSE_FASTEST_HOST

				}
			}
			break;

		case ERM_DUNGEON_SUB_STAGE_GO_NEXT_REQ:
			{
				SendDSSGoNextAllNot( m_kDSSGoNextAllNot );
			}
			break;

		case ERM_DUNGEON_SUB_STAGE_LOAD_COMPLETE_REQ:
			{
				if( m_spRoomUserManager->IsAllPlayerStageLoaded() == true )
				{
					//{{ 2010. 8. 16	������	���� �ڵ� ����
					if( DungeonSubStageStartProcess( GetSubStageID() ) == false )
					{
						START_LOG( cerr, L"���� ���� �������� ���� ����!" )
							<< BUILD_LOG( GetSubStageID() )
							<< END_LOG;
					}
					//}}
				}
			}
			break;
		}
	}
}

void KDungeonRoom::CheckDungeonUnitInfoSuccess()
{
	if( m_spRoomUserManager->IsAllPlayerDungeonUnitInfoSet() == true )
	{
		int iPassedStageCount = 0;
		KEGS_END_GAME_DUNGEON_RESULT_DATA_NOT kUnitInfoNot;
		//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER
		kUnitInfoNot.m_bIsClearDefenceDungeon = false;
		kUnitInfoNot.m_iProtectedSmallCrystal= 0;
#endif SERV_INSERT_GLOBAL_SERVER
		//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�

		std::vector< KRoomUserPtr > vecKRoomUserPtr;
		std::vector< KERM_UPDATE_DUNGEON_UNIT_INFO_NOT > vecUpdateUnitInfo;
		std::vector< KEGS_GET_ITEM_NOT > vecResultGetItem;

		//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER
		const int iProtectedCrystalCount = GetProtectedSmallCrystalCount() + 1; // ���� ������ ���� ��� ���� ���� +1 ū ����

		if( CXSLDungeon::IsDefenceDungeon( m_iDungeonID ) == true )
		{
			if( SiKResultProcess()->Result_DefenceDUNGEON(  m_spRoomUserManager, 
															m_kDungeonMonsterManager, 
															m_bIsWin, 
															m_eDungeonType, 
															m_cDungeonMode,
															m_cDifficultyLevel, 
															m_iDungeonID, 
															(int)GetTotalPlayTime(), 
															(int)GetRemainingPlayTime(), 
															m_nDropItemUID, 
															m_iStartedNumMember, 
															//{{ 2012. 04. 16	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
															IsStartedByAutoParty(),
#endif SERV_BATTLE_FIELD_SYSTEM
															//}}
															iPassedStageCount, 
															kUnitInfoNot, 
															vecKRoomUserPtr, 
															vecUpdateUnitInfo,
															iProtectedCrystalCount ) == false )
			{
				START_LOG( cerr, L"������� ó�� ����.!" )
					<< END_LOG;
				return;
			}
		}
		else
#endif SERV_INSERT_GLOBAL_SERVER
			//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
		{
			if( SiKResultProcess()->Result_DUNGEON( m_spRoomUserManager, 
													m_kDungeonMonsterManager, 
													m_bIsWin, 
													m_eDungeonType, 
													m_cDungeonMode,
													m_cDifficultyLevel, 
													m_iDungeonID, 
													//{{ 2011. 08. 22	������	��ϸ� �ð� ����
#ifdef SERV_NEW_HENIR_TEST
													GetStageID(),
#endif SERV_NEW_HENIR_TEST
													//}}
													(int)GetTotalPlayTime(), 
													(int)GetRemainingPlayTime(), 
													m_nDropItemUID, 
													m_iStartedNumMember, 
													//{{ 2012. 04. 16	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
													IsStartedByAutoParty(),
#endif SERV_BATTLE_FIELD_SYSTEM
													//}}
													iPassedStageCount, 
													kUnitInfoNot, 
													vecKRoomUserPtr, 
													vecUpdateUnitInfo ) == false )
			{
				START_LOG( cerr, L"������� ó�� ����.!" )
					<< END_LOG;
				return;
			}
		}

		//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER
		if( CXSLDungeon::IsDefenceDungeon( m_iDungeonID ) == true )
		{
			kUnitInfoNot.m_bIsClearDefenceDungeon = m_bIsWin;
			kUnitInfoNot.m_iProtectedSmallCrystal= GetProtectedSmallCrystalCount();
		}
#endif SERV_INSERT_GLOBAL_SERVER
		//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�

		//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM		
		kUnitInfoNot.m_bComeBackUserInParty = m_spRoomUserManager->IsComeBackUserInRoom();
#endif SERV_SERVER_BUFF_SYSTEM
		//}

#ifdef SERV_PARTYPLAY_WITH_DUNGEON_CLEAR_COUNT
		bool m_bSCHAR1 = false; 
		bool m_bSCHAR2 = false; 
		char m_CheckClass;
		char m_CheckType;

		int m_iRoomUserCnt = m_spRoomUserManager->GetLiveMember();

#ifdef SERV_RELATIONSHIP_EVENT_SUBQUEST
		std::vector<UidType> vecRelation;
#endif SERV_RELATIONSHIP_EVENT_SUBQUEST

		if(m_spRoomUserManager->GetLiveMember() != NULL)
		{
			KRoomUserPtr spRoomUserCheck = m_spRoomUserManager->GetUser( 0 );

			if( spRoomUserCheck != NULL)
			{
				// ���� �ľ�
				for(int rc = 0; rc < m_iRoomUserCnt; rc++)
				{
					spRoomUserCheck = m_spRoomUserManager->GetUser(rc);

					if( spRoomUserCheck != NULL)
					{
						m_CheckType = spRoomUserCheck->GetUnitType();
						m_CheckClass = spRoomUserCheck->GetUnitClass_LUA();
					}

#ifdef SERV_RELATIONSHIP_EVENT_SUBQUEST
					// ���� �ľ�
					KRoomUserInfo kInfo;
					spRoomUserCheck->GetRoomUserInfo( kInfo );

					if( kInfo.m_bCouple == true && kInfo.m_iRelationTargetUserUid > 0 )
					{
						vecRelation.push_back( kInfo.m_iRelationTargetUserUid );
					}
#endif SERV_RELATIONSHIP_EVENT_SUBQUEST

					// ��� �����ϵ��� ����
					if( _CONST_PARTYPLAY_WITH_DUNGEON_CLEAR_COUNT_::cUnitType == 0 ||
						_CONST_PARTYPLAY_WITH_DUNGEON_CLEAR_COUNT_::cUnitType == m_CheckType )
					{
						m_bSCHAR1 = true;
					}
					if( _CONST_PARTYPLAY_WITH_DUNGEON_CLEAR_COUNT_::cUnitClass == 0 ||
						_CONST_PARTYPLAY_WITH_DUNGEON_CLEAR_COUNT_::cUnitClass == m_CheckClass )
					{
						m_bSCHAR2 = true;
					}
				}
			}
		}		

#ifdef SERV_RELATIONSHIP_EVENT_SUBQUEST
		kUnitInfoNot.m_vecRelation = vecRelation;
#endif SERV_RELATIONSHIP_EVENT_SUBQUEST
		kUnitInfoNot.m_bSpecChar1 = m_bSCHAR1; // ���ҵ� ���� �ľ�
		kUnitInfoNot.m_bSpecChar2 = m_bSCHAR2; // �ý�����Ʈ ���� �ľ�
#endif SERV_PARTYPLAY_WITH_DUNGEON_CLEAR_COUNT

		BroadCast( ERM_END_GAME_DUNGEON_RESULT_DATA_NOT, kUnitInfoNot );

		//���Ӽ����� �� UNIT �ڽ��� ����� ������Ʈ
		//{{ 2011. 12.13    ��μ�	���� Ŭ���� �� ������ ���� �̺�Ʈ - ������ �ֹ���(�ߺ� ���� ����)
#ifdef SERV_DUNGEON_CLEAR_PAYMENT_ITEM_EVENT
		BroadCastUpdateUnitInfoWithPartyCheck( vecKRoomUserPtr, ERM_UPDATE_DUNGEON_UNIT_INFO_NOT, vecUpdateUnitInfo, (m_iDungeonID + static_cast<int>(m_cDifficultyLevel)), kUnitInfoNot.m_mapHaveExpInDungeon );
#else
		BroadCastUpdateUnitInfoWithPartyCheck( vecKRoomUserPtr, ERM_UPDATE_DUNGEON_UNIT_INFO_NOT, vecUpdateUnitInfo, (m_iDungeonID + static_cast<int>(m_cDifficultyLevel)) );
#endif SERV_DUNGEON_CLEAR_PAYMENT_ITEM_EVENT
		//}}
		m_spRoomUserManager->ResetDungeonUnitInfo();		

		//{{ 2013. 04. 09	������	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
		// ����� ���� ���۵Ǵ� ������ �����ϴ°�?
		if( m_kDefenceDungeonManager.IsEnterDefenceDungeon() == true  &&  CXSLDungeon::IsDefenceDungeon( GetDungeonID() ) == false )
		{
			// �׷��ٸ� ����� ���� �غ�����!
			PrepareForEnterDefenceDungeon();
		}
#endif SERV_NEW_DEFENCE_DUNGEON
		//}}

		//{{ 2009. 7. 27  ������	��ϸ� �ð� Ŭ����� ��ü����
		if( m_eDungeonType == CXSLDungeon::DT_HENIR  &&  
			m_cDungeonMode == CXSLDungeon::DM_HENIR_CHALLENGE  &&
			SiKHenirResultTable()->IsClearNotifyStage( iPassedStageCount ) == true  &&
			m_bIsWin == true
#ifdef SERV_HENIR_NO_NOTIFY
			&& SiKHenirResultTable()->GetEnbleNotify() == true
#endif // SERV_HENIR_NO_NOTIFY
			)
		{
			// ��ü ���� ������!
			KEGS_NOTIFY_MSG_NOT kNotMsg;
			kNotMsg.m_cNotifyType = KEGS_NOTIFY_MSG_NOT::NT_HENIR_CLEAR;
			kNotMsg.m_Count = 1;

			BOOST_TEST_FOREACH( const KRoomUserPtr, spRoomUser, vecKRoomUserPtr )
			{
				if( spRoomUser == NULL )
				{
					START_LOG( cerr, L"�� ������ �����Ͱ� �̻���." )
						<< END_LOG;
					continue;
				}

				kNotMsg.m_wstrMSG += L"[";
				kNotMsg.m_wstrMSG += spRoomUser->GetNickName();
				kNotMsg.m_wstrMSG += L"]";

				// Ŭ���� �α�
				KELOG_HENIR_DUNGEON_CLEAR_NOT kPacketLog;
				kPacketLog.m_iUserUID = spRoomUser->GetUserUID();
				kPacketLog.m_iUnitUID = spRoomUser->GetCID();
				kPacketLog.m_iPlayTime = static_cast<int>(GetTotalPlayTime());
				SendToLogDB( ELOG_HENIR_DUNGEON_CLEAR_NOT, kPacketLog );
			}
			
			// ���� ������ ����
			KncSend( PI_CN_ROOM, GetUID(), PI_CN_SERVER, KBaseServer::GetKObj()->GetUID(), NULL, ECN_NOTIFY_MSG_NOT, kNotMsg );
		}
		//}}

		// ���� ���� ��� ������ ���
		for ( unsigned int iR = 0; iR < vecKRoomUserPtr.size(); ++iR )
		{
			const KERM_UPDATE_DUNGEON_UNIT_INFO_NOT& kUpdateUnitInfo = vecUpdateUnitInfo[iR];
			const KDungeonUnitResultInfo& kDungeonUnitInfo = kUnitInfoNot.m_vecDungeonUnitInfo[iR];

			KRoomUserPtr spRoomUser = vecKRoomUserPtr[iR];
			if( spRoomUser == NULL )
			{
				START_LOG( cerr, L"�� ������ �����Ͱ� �̻���." )
					<< END_LOG;
				continue;
			}

			//{{ 2012. 11. 9	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
			const int iClearValue = ( m_bIsWin ? KE_LOCAL_LOG_DUNGEON_NOT::DCT_SUCCESS : KE_LOCAL_LOG_DUNGEON_NOT::DCT_FAILED );
			
			// ���� �α�
			LogToDB_DungeonLog( spRoomUser, iClearValue, kUpdateUnitInfo, kDungeonUnitInfo );
#else
//			// ��ڴ� ��踦 ������ �ʽ��ϴ�.
//			//if( spRoomUser->GetAuthLevel() >= SEnum::UAL_GM )
//			//{
//			//	START_LOG( cout, L"��ڴ� ��赥���͸� ������ �ʽ��ϴ�." )
//			//		<< BUILD_LOGc( spRoomUser->GetAuthLevel() )
//			//		<< BUILD_LOG( spRoomUser->GetNickName() );
//			//	continue;
//			//}
//
//			KRoomUserInfo kInfo;
//			spRoomUser->GetRoomUserInfo( kInfo );
//
//			//{{ 2010. 06. 21  ������	��� ������
//			//#ifdef SERV_STATISTICS_THREAD
//			//{{ 2010. 11. 8	������	���� �α� DB
//			//#ifdef SERV_DUNGEON_LOG_DB
//			CTime kRegDate = CTime::GetCurrentTime();
//			//#endif SERV_DUNGEON_LOG_DB
//			//}}
//			//////////////////////////////////////////////////////////////////////////
//			KE_LOCAL_LOG_DUNGEON_NOT kNot;
//			kNot.m_iDungeonID				= (m_iDungeonID + static_cast<int>(m_cDifficultyLevel));
//			kNot.m_nUnitUID					= kInfo.m_nUnitUID;
//			kNot.m_wstrNickName				= kInfo.m_wstrNickName;
//			kNot.m_iDifficultyLevel			= (int) m_cDifficultyLevel;
//			kNot.m_iIsChallenge				= ( m_cDungeonMode == CXSLDungeon::DM_HENIR_CHALLENGE ? 1 : 0 );
//			kNot.m_iUnitClass				= (int) kInfo.m_cUnitClass;
//			kNot.m_iClear					= ( m_bIsWin ? KE_LOCAL_LOG_DUNGEON_NOT::DCT_SUCCESS : KE_LOCAL_LOG_DUNGEON_NOT::DCT_FAILED );
//			kNot.m_iStartNumMember			= m_iStartedNumMember;
//			kNot.m_iEndNumMember			= GetNumMember();
//			kNot.m_ucLevel					= kInfo.m_ucLevel;
//			kNot.m_iPlayTime				= (int) GetTotalPlayTime();
//			kNot.m_iEXP						= kUpdateUnitInfo.m_iEXP;
//			kNot.m_iED						= spRoomUser->GetRewardED();
//			kNot.m_cComboRank				= kDungeonUnitInfo.m_cComboRank;
//			kNot.m_iComboScore				= spRoomUser->GetComboScore_LUA();
//			kNot.m_cTechnicalRank			= kDungeonUnitInfo.m_cTechnicalRank;
//			kNot.m_iTechnicalScore			= spRoomUser->GetTechnicalScore_LUA();
//			kNot.m_cTimeRank				= kDungeonUnitInfo.m_cTimeRank;
//			kNot.m_cDamagedRank				= kDungeonUnitInfo.m_cDamagedRank;			
//			kNot.m_iDamageNum				= spRoomUser->GetDamageNum_LUA();
//			kNot.m_cTotalRank				= kDungeonUnitInfo.m_cTotalRank;
//			kNot.m_iRessurectionStoneCount	= spRoomUser->GetUsedRessurectionStoneCount();
//			kNot.m_iPassedStageCount		= spRoomUser->GetPassedStageCount();
//			kNot.m_iPassedSubStageCount		= spRoomUser->GetPassedSubStageCount();
//			//{{ 2010. 9. 14	������	�� �ý���
//#ifdef SERV_PET_SYSTEM
//			kNot.m_bIsWithPet				= spRoomUser->HavePet();
//#endif SERV_PET_SYSTEM
//			//}}
//			//{{ 2010. 11. 8	������	���� �α� DB
//#ifdef SERV_DUNGEON_LOG_DB
//			kNot.m_wstrRegDate				= (const wchar_t*)kRegDate.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
//#endif SERV_DUNGEON_LOG_DB
//			//}}
//			//{{ 2011. 03. 16	������	���� �α� �÷� �߰�
//#ifdef SERV_ADD_DUNGEON_LOG_COLUMN
//			kNot.m_sNpcDieCount				= spRoomUser->GetKillNPC_LUA();
//			kNot.m_iGivenDamage				= spRoomUser->GetTotalGivenDamamge_LUA();
//			kNot.m_iAttackDamage			= spRoomUser->GetTotalAttackedDamage_LUA();
//			kNot.m_sUsingSkillKind			= spRoomUser->GetUsingKindOfSkill_LUA();
//			kNot.m_iTotalEXP				= ( spRoomUser->GetRewardEXP() + spRoomUser->GetRewardPartyEXP() + kUpdateUnitInfo.m_iEXP );
//			kNot.m_cPetID					= spRoomUser->GetPetID();
//			kNot.m_cPetEvoStep				= spRoomUser->GetPetEvolutionStep();
//#endif SERV_ADD_DUNGEON_LOG_COLUMN
//			//}}
//			//{{ 2012. 03. 05	��μ�	���� ��� �α�
//#ifdef SERV_DUNGEON_DASH_LOG
//			kNot.m_iDashCount				= spRoomUser->GetDashCount();
//#endif SERV_DUNGEON_DASH_LOG
//			//}}
//			//{{ 2012. 04. 17	������	���� �α� �÷� �߰�
//#ifdef SERV_ADD_DUNGEON_LOG_COLUMN_NUM_2
//			kNot.m_iBaseHP					= spRoomUser->GetBaseHP_LUA();
//			kNot.m_iEndHP					= spRoomUser->GetLastUpdatedHP();
//			kNot.m_iRecovHP					= spRoomUser->GetTotalRecoveryHP_LUA();
//			kNot.m_iContinue				= spRoomUser->GetUsedRessurectionStoneCount();
//			kNot.m_iStartContinue			= spRoomUser->GetStartTimeNumResurrectionStone();
//			kNot.m_iContinueUseStage		= spRoomUser->GetFirstUseResurrectionStoneStageID();
//			kNot.m_iContinueUseSub			= spRoomUser->GetFirstUseResurrectionStoneSubStageID();
//			kNot.m_iFrame					= spRoomUser->GetInGameAverageFrame();
//			kNot.m_wstrChannelIP			= spRoomUser->GetGameChannelIP();
//#endif SERV_ADD_DUNGEON_LOG_COLUMN_NUM_2
//			//}}
//			//{{ 2012. 07. 11 ��μ�
//#ifdef SERV_ADD_DUNGEON_LOG_COLUMN_NUM_3
//			kNot.m_iUserUID					= spRoomUser->GetUserUID();
//#endif SERV_ADD_DUNGEON_LOG_COLUMN_NUM_3
//			//}}
//			//{{ 2012. 11. 6	������	��Ʋ�ʵ� �ý���
//#ifdef SERV_BATTLE_FIELD_SYSTEM
//			kNot.m_cAutoPartyPlay			= ( IsStartedByAutoParty() ? 1 : 0 );
//#endif SERV_BATTLE_FIELD_SYSTEM
//			//}}
//			KSIManager.QueueingEvent( E_LOCAL_LOG_DUNGEON_NOT, kNot );
//			//////////////////////////////////////////////////////////////////////////
//			//{{ 2010. 11. 8	������	���� �α� DB
//			//#ifdef SERV_DUNGEON_LOG_DB
//			SendToLogDB( DBE_DB_LOG_DUNGEON_NOT, kNot );
//			//#endif SERV_DUNGEON_LOG_DB
//			//}}
//			//#endif SERV_STATISTICS_THREAD
//			//}}
#endif SERV_BATTLE_FIELD_SYSTEM
			//}}			

			//{{ 2011. 06. 13	������	�������� �α�
#ifdef SERV_DUNGEON_STAGE_LOG			
			{
				KELOG_DUNGEON_STAGE_LOG_NOT kPacketToLog;
				kPacketToLog.m_iDungeonID		  = GetDungeonID();
				kPacketToLog.m_iStageID			  = m_iLoadingCompleteStageID; // ���� ��������
				kPacketToLog.m_iUnitUID			  = spRoomUser->GetCID();
				kPacketToLog.m_cLevel			  = spRoomUser->GetLevel();
				kPacketToLog.m_cDifficulty		  = GetDifficultyLevel();
				kPacketToLog.m_bIsChallenge 	  = ( m_cDungeonMode == CXSLDungeon::DM_HENIR_PRACTICE );
				kPacketToLog.m_cUnitClass		  = spRoomUser->GetUnitClass_LUA();
				kPacketToLog.m_iStagePlayTime	  = static_cast<int>(spRoomUser->GetStagePlayTime()); // �ش� ���������� �÷����� �÷��� Ÿ��
				kPacketToLog.m_iStageDamage		  = ( spRoomUser->GetTotalAttackedDamage_LUA() - spRoomUser->GetStageDamage() ); // �ش� �������� ���� ���� �����
				kPacketToLog.m_iResurrectionCount = spRoomUser->GetUsedRessurectionStoneCount();
				kPacketToLog.m_wstrRegDate		  = KncUtil::GetCurrentTimeString();
				SendToLogDB( ELOG_DUNGEON_STAGE_LOG_NOT, kPacketToLog );
			}
#endif SERV_DUNGEON_STAGE_LOG
			//}}

			//{{ 2007. 12. 18  ������  ���� ��� [����]
			{
				KERM_DUNGEON_USER_STATISTICS_NOT kPacketNot;
				kPacketNot.m_iDungeonID	   = m_iDungeonID + static_cast<int>(m_cDifficultyLevel);
				kPacketNot.m_cGameResult   = ( m_bIsWin == true ? KERM_DUNGEON_USER_STATISTICS_NOT::UGR_WIN : KERM_DUNGEON_USER_STATISTICS_NOT::UGR_LOSE );
				kPacketNot.m_bIsPartyPlay  = ( m_iStartedNumMember > 1 );
				kPacketNot.m_iGamePlayTime = static_cast<int>(GetTotalPlayTime());
				kPacketNot.m_iUsedResurrectionStone = spRoomUser->GetUsedRessurectionStoneCount();
				SendToGSCharacter( spRoomUser->GetGSUID(), spRoomUser->GetCID(), ERM_DUNGEON_USER_STATISTICS_NOT, kPacketNot );
			}
			//}}

			//{{ 2010. 10. 05	������	���ͼ��� ��ŷüũ
#ifdef SERV_CN_SERVER_HACKING_CHECK
			if( SiKCnHackingCheckManager()->IsDungeonResultCheckEnable() == true )
			{
				if( SiKCnHackingCheckManager()->CheckDungeonResultAutoHackUser( GetDungeonID(), 
																		GetDifficultyLevel(),
																		spRoomUser->GetUnitClass_LUA(), 
																		spRoomUser->GetLevel(),
																		m_bIsWin, 
																		m_iStartedNumMember,
																		static_cast<CXSLDungeon::RANK_TYPE>(kDungeonUnitInfo.m_cComboRank), 
																		static_cast<CXSLDungeon::RANK_TYPE>(kDungeonUnitInfo.m_cTechnicalRank), 
																		static_cast<CXSLDungeon::RANK_TYPE>(kDungeonUnitInfo.m_cTimeRank),
																		static_cast<CXSLDungeon::RANK_TYPE>(kDungeonUnitInfo.m_cDamagedRank), 
																		spRoomUser->GetTechnicalScore_LUA(), 
																		spRoomUser->GetUsedRessurectionStoneCount(), 
																		spRoomUser->HavePet()
																		) == true )
				{
					// ��ŷ üũ�� �������� ��Ŷ�� ������!
					//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
					KEGS_SERVER_CHECK_HACK_USER_REQ kNot;
					kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_DUNGEON_CLEAR_RESULT;
#else
					KEGS_SERVER_CHECK_HACK_USER_NOT kNot;
					kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_DUNGEON_CLEAR_RESULT;
#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
					//}}
	
					SendToGSCharacter( spRoomUser->GetGSUID(), spRoomUser->GetCID(), ERM_SERVER_CHECK_HACK_USER_NOT, kNot );

					START_LOG( clog, L"Dungeon Result ��ŷ������ �ǴܵǾ� üũ ����� Ŭ���̾�Ʈ �����Ͽ���!" )
						<< BUILD_LOG( spRoomUser->GetGSUID() )
						<< BUILD_LOG( spRoomUser->GetCID() )
#ifndef SERV_PRIVACY_AGREEMENT
						<< BUILD_LOG( spRoomUser->GetNickName() )
#endif SERV_PRIVACY_AGREEMENT
						<< BUILD_LOG( GetDungeonID() )
						<< BUILD_LOGc( GetDifficultyLevel() )
						<< BUILD_LOGc( spRoomUser->GetUnitClass_LUA() )
						<< BUILD_LOGc( spRoomUser->GetLevel() )
						<< BUILD_LOG( m_bIsWin )
						<< BUILD_LOG( m_iStartedNumMember )
						<< BUILD_LOGc( kDungeonUnitInfo.m_cComboRank )
						<< BUILD_LOGc( kDungeonUnitInfo.m_cTechnicalRank )
						<< BUILD_LOGc( kDungeonUnitInfo.m_cTimeRank )
						<< BUILD_LOGc( kDungeonUnitInfo.m_cDamagedRank )
						<< BUILD_LOG( spRoomUser->GetTechnicalScore_LUA() )
						<< BUILD_LOG( spRoomUser->GetUsedRessurectionStoneCount() )
						<< BUILD_LOG( spRoomUser->HavePet() );
				}
			}
#endif SERV_CN_SERVER_HACKING_CHECK
			//}}
			//{{ 2011. 01. 26	������	���� Ŭ���� Ÿ�� ������ üũ
#ifdef SERV_DUNGEON_CLEAR_TIME_HACK_USER_CHECK
			if( SiKCnHackingCheckManager()->IsDungeonClearTimeCheckEnable() == true )
			{
				if( SiKCnHackingCheckManager()->CheckDungeonClearTimeAutoHackUser( spRoomUser->GetAuthLevel(),
																				spRoomUser->GetUserUID(),
																				GetDungeonID(), 
																				GetDifficultyLevel(),
																				spRoomUser->GetUnitClass_LUA(), 
																				spRoomUser->GetLevel(),
																				m_bIsWin, 
																				m_iStartedNumMember,
																				(int)spRoomUser->GetPlayTime()
																				) == true )
				{
					// ��ŷ üũ�� �������� ��Ŷ�� ������!
					//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
					KEGS_SERVER_CHECK_HACK_USER_REQ kNot;
					kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_DUNGEON_CLEAR_TIME;
#else
					KEGS_SERVER_CHECK_HACK_USER_NOT kNot;
					kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_DUNGEON_CLEAR_TIME;
#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
					//}}
					SendToGSCharacter( spRoomUser->GetGSUID(), spRoomUser->GetCID(), ERM_SERVER_CHECK_HACK_USER_NOT, kNot );

					// ��ŷ üũ�� ������ DB�� �α׷� ������!
					CTime kCurTime = CTime::GetCurrentTime();
					KDBE_REG_DUNGEON_CLEAR_TIME_HACK_USER_NOT kNotReg;
					kNotReg.m_iUserUID = spRoomUser->GetUserUID();
					kNotReg.m_wstrRegDate = (std::wstring)kCurTime.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
					SendToLogDB( DBE_REG_DUNGEON_CLEAR_TIME_HACK_USER_NOT, kNotReg );

					START_LOG( clog, L"Dungeon Clear Time ��ŷ������ �ǴܵǾ� üũ ����� Ŭ���̾�Ʈ �����Ͽ���!" )
						<< BUILD_LOG( spRoomUser->GetGSUID() )
						<< BUILD_LOG( spRoomUser->GetUserUID() )
						<< BUILD_LOG( spRoomUser->GetCID() )
#ifndef SERV_PRIVACY_AGREEMENT
						<< BUILD_LOG( spRoomUser->GetNickName() )
#endif SERV_PRIVACY_AGREEMENT
						<< BUILD_LOG( GetDungeonID() )
						<< BUILD_LOGc( GetDifficultyLevel() )
						<< BUILD_LOGc( spRoomUser->GetUnitClass_LUA() )
						<< BUILD_LOGc( spRoomUser->GetLevel() )
						<< BUILD_LOG( m_bIsWin )
						<< BUILD_LOG( m_iStartedNumMember )
						<< BUILD_LOG( spRoomUser->GetPlayTime() );
				}
			}
#endif SERV_DUNGEON_CLEAR_TIME_HACK_USER_CHECK
			//}}
		}

		//{{ 2010. 06. 21  ������	��� ������
#ifdef SERV_STATISTICS_THREAD
		CTime kEndGameTime = CTime::GetCurrentTime();
		//////////////////////////////////////////////////////////////////////////
		KE_LOCAL_LOG_DUNGEON_ROOM_NOT kNot;
		kNot.m_iDungeonID		= m_iDungeonID + static_cast<int>(m_cDifficultyLevel);
		kNot.m_iDifficultyLevel	= (int) GetDifficultyLevel();
		kNot.m_iIsChallenge		= ( m_cDungeonMode == CXSLDungeon::DM_HENIR_CHALLENGE ? 1 : 0 );
		kNot.m_iWin				= ( m_bIsWin ? 1 : 0 );
		kNot.m_iLose			= ( m_bIsWin ? 0 : 1 );
		kNot.m_iStartNumMember	= m_iStartedNumMember;
		kNot.m_iEndNumMember	= GetNumMember();
		kNot.m_iTotalPlayTime	= static_cast<int>(GetTotalPlayTime());
		kNot.m_wstrEndGameTime	= (const wchar_t*)kEndGameTime.Format(_T("%H:%M:%S"));
		KSIManager.QueueingEvent( E_LOCAL_LOG_DUNGEON_ROOM_NOT, kNot );
		//////////////////////////////////////////////////////////////////////////
#endif SERV_STATISTICS_THREAD
		//}}
	}
}

//{{ 2009. 7. 27  ������	��ϸ� �ð�
void KDungeonRoom::CheckEnterSecretStageSuccess()
{
	if( GetStateID() != KRoomFSM::S_PLAY )
		return;

	// ��� ������ �ǻ� ǥ�ø� �Ͽ��°�?
	if( m_spRoomUserManager->IsAllPlayerAgreed() == true )
	{
		// �׷��ٸ� �ٷ� �̵�ó��!
		KEGS_DUNGEON_SECRET_STAGE_ENTER_RESULT_NOT kNot;
		kNot.m_bProceedToEnterSecretStage = m_spRoomUserManager->IsEnterSecretStage(); // ��� �������� ���� ����
		BroadCast( ERM_DUNGEON_SECRET_STAGE_ENTER_RESULT_NOT, kNot );

		// ���� �ǰ� ���� �ʱ�ȭ
		m_spRoomUserManager->ResetAgreeEnterSecretStage();

		// ���� �ǰ� Ÿ�̸� ����
		//m_kEnterSecretStageTimer.KillTimer();

		// ��� �������� ���� �����̶�� �÷��� �ð��� 5�� �� �÷�����!
		//## des second(hoons) : ��� �������� ��� �߰��� �ð��� �ִ� ��� ������.
		//if( kNot.m_bProceedToEnterSecretStage == true )
		//{
		//	SetRemainingPlayTime( m_fRemainingPlayTime + 300.f );
		//	BroadCast( ERM_REMAINING_PLAY_TIME_NOT, GetRemainingPlayTime() );
		//}
	}	
}
//}}

//{{ 2010. 8. 16	������	���� �ڵ� ����
bool KDungeonRoom::DungeonSubStageStartProcess( IN int iSubStageID )
{
	bool bSuccess = true;

	// Loading ���� ����.
	m_spRoomUserManager->ResetStageLoaded();

	KEGS_DUNGEON_SUB_STAGE_START_NOT kNot;
	kNot.m_iSubStageID = iSubStageID;
	BroadCast( ERM_DUNGEON_SUB_STAGE_START_NOT, kNot );

//////////////////////////////////////////////////////////////////////////
	//{{ 2011. 09. 07  ��μ�	���� �������� DIE NPC �ð� üũ ���
#ifdef SERV_SUB_STAGE_NPC_DIE_CHECK
	const int iDungeonMinLevel = SiCXSLDungeonManager()->GetDungeonMinLevel( GetDungeonIDAndDif() );
	if( m_spRoomUserManager->GetNumMember() == 1 && GetHighestUserLevelAtDungeonStart() - iDungeonMinLevel < 10  )
	{
		// ���� ���� ���������� ��Ƽ�� ���� �� �ʱ�ȭ
		m_iSubStageActiveNpc = 0;
		std::map< int, int >::const_iterator mit;
		mit = m_mapSubStageExpNpcNum.find( iSubStageID );

		if( mit != m_mapSubStageExpNpcNum.end() )
			m_iSubStageActiveNpc = mit->second;
		
		// ���� ������������ ���� ���� ���� �ʱ�ȭ
		m_iSubStageKillActiveNpc = 0;

		// ���� ������������ ó�� ���� ���� �ð��� �ʱ�ȭ	
		m_tSubStageFirstKillTime = CTime( 0 );
	}
	else
	{
		m_iSubStageActiveNpc = 0;
		m_iSubStageKillActiveNpc = 0;
		m_tSubStageFirstKillTime = CTime( 0 );
	}
#endif SERV_SUB_STAGE_NPC_DIE_CHECK
	//}}
//////////////////////////////////////////////////////////////////////////


	//������� �ε��ð��� üũ��.
	const double fLoadingTimerEnd = m_LoadingTimer.End();
	m_fLoadingTime += fLoadingTimerEnd;	

	// Ȥ�ó� �ε� Ÿ���� �÷��� Ÿ�Ӻ��� �� ũ�� �������� �˻��غ���!
	if( m_kTimer[TM_PLAY].elapsed() < GetLoadingTime() )
	{
		START_LOG( cout, L"�ε� Ÿ���� ���� �÷��� Ÿ�Ӻ��� �� ũ�� ������ �̻��� ������ �߻��ߴ�!" )
			<< BUILD_LOG( GetStateIDString() )
			<< BUILD_LOG( m_kTimer[TM_PLAY].elapsed() )
			<< BUILD_LOG( GetLoadingTime() )
			<< BUILD_LOG( fLoadingTimerEnd )
			<< BUILD_LOG( GetDungeonIDAndDif() )
			<< BUILD_LOG( GetStageID() )
			<< BUILD_LOG( GetSubStageID() )
			<< BUILD_LOG( m_spRoomUserManager->GetNumMember() )
			<< BUILD_LOG( m_iStartedNumMember )
			<< END_LOG;

		bSuccess = false;
	}	

	//{{ 2012. 02. 28	������	��Ʋ�ʵ� �ý���
//#ifdef SERV_BATTLE_FIELD_SYSTEM
	//if( IsFirstSubStage() == false )
	//{
	//	IncreasePartyFever();
	//}
//#else
//	if( m_iPartyUID > 0  &&  
//		m_spRoomUserManager->GetNumMember() > 1  &&  
//		IsFirstSubStage() == false )
//	{
//		KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser( 0 ); // 0�� �ε����� ����
//		if( spRoomUser != NULL )
//		{
//			KERM_INCREASE_PARTY_FEVER_NOT kNot;
//			kNot.m_iPartyUID = m_iPartyUID;
//			SendToGSServer( spRoomUser->GetGSUID(), ERM_INCREASE_PARTY_FEVER_NOT, kNot );
//		}
//		else
//		{
//			START_LOG( cerr, L"������ �Ѹ��� ���¹��ΰ�?" )
//				<< BUILD_LOG( GetStateIDString() )
//				<< END_LOG;
//
//			bSuccess = false;
//		}
//	}
//#endif SERV_BATTLE_FIELD_SYSTEM
	//}}


#ifdef  SERV_CHOOSE_FASTEST_HOST
#ifdef  SERV_OPTIMIZE_CHOOSE_FASTEST_HOST_ENHANCE
    m_iSubStageCount++;
    if ( m_iSubStageCount == 2 && m_bCheckPingScoreExecuted == false )
    {
		CheckPingScore();
		m_kTimer[TM_CHECK_CHANGE_HOST].restart();
    }
#endif  SERV_OPTIMIZE_CHOOSE_FASTEST_HOST_ENHANCE
#endif  SERV_CHOOSE_FASTEST_HOST

	return bSuccess;
}
//}}

//{{ 2010. 03. 24  ������	��д��� ����
#ifdef SERV_SECRET_HELL

void KDungeonRoom::NormalDungeonAttribMonsterDrop( IN OUT KEGS_DUNGEON_STAGE_LOAD_NOT& kNot )
{
	int	iRandomSubStage = -1;
	bool bIsBossStage = false;
	bool bDropAttribNpc = false;

	// 1. �Ӽ�NPC ���ī��Ʈ�� 0�̸� ������Ű�� �ʴ´�.
	if( m_iAttribNpcStageCnt <= 0 )
		return;

	// 2. �ʺ��ڽ�, Ʃ�丮��, �Ʒüҿ����� ����Ʈ ���� �ȳ���
	if( m_iDungeonID == CXSLDungeon::DI_EL_FOREST_GATE_NORMAL  ||
		//{{ 2011. 09. 28  �ӱԼ�		�ҷ��� ����
#ifdef SERV_DUNGEON_STAGE_RANDOM_NPC_POS
		m_iDungeonID == CXSLDungeon::DI_ELDER_HALLOWEEN_NORMAL ||
#endif SERV_DUNGEON_STAGE_RANDOM_NPC_POS
		//}}
		CXSLDungeon::IsTutorialDungeon( m_iDungeonID ) == true  ||
		CXSLDungeon::IsTCDungeon( m_iDungeonID ) == true )
	{
		return;
	}

	// 3. �ش� ������������ ���������� ����
	bDropAttribNpc = ( (rand() % 2) == 0 );
	if( bDropAttribNpc == false )
	{
		return;
	}

	// 4. ��� ���� ���������� ����Ʈ ���Ͱ� �������� ����
	if( kNot.m_mapNPCData.size() > 0 )
	{
		iRandomSubStage = rand() % static_cast<int>(kNot.m_mapNPCData.size());
	}

	//{{ 2011. 03. 21  ��μ�	���� ���̵� ����
#ifdef SERV_DUNGEON_DIFFICULTY_REVISION
	const int iPlayerCount = m_spRoomUserManager->GetNumPlaying();
	int tempAttribNpcCnt = 0;				// m_iAttribNpcCnt �� ���� ���۽ÿ��� ���� �ǰ� �Ѵ�.
	switch( m_eDungeonType )
	{
	case CXSLDungeon::DT_NORMAL:
		{
			// ���� ���������� �Ӽ� ���� �ȳ��´�.
			if( CXSLDungeon::DI_RUBEN_EL_TREE_NORMAL != GetDungeonID() )
			{
				SiKAttribNpcTable()->GetAttribNpcDropCount( static_cast<char>(m_eDungeonType), m_cDifficultyLevel, iPlayerCount, tempAttribNpcCnt, m_iAttribNpcDropCnt );
			}
		}break;
	case CXSLDungeon::DT_HENIR: // ��ϸ� �ð������� �Ӽ����� �ȳ���.
		break;
	case CXSLDungeon::DT_SECRET:
		{
			SiKAttribNpcTable()->GetAttribNpcDropCount( static_cast<char>(m_eDungeonType), m_cDungeonMode, iPlayerCount, tempAttribNpcCnt, m_iAttribNpcDropCnt );
		}break;
	default:
		{
			START_LOG( cerr, L"�߸��� ���� Ÿ���Դϴ�." )
				<< BUILD_LOG( m_eDungeonType )
				<< END_LOG;
		}
		break;
	}
#endif SERV_DUNGEON_DIFFICULTY_REVISION
	//}}

	// �Ӽ� ���� �����ϱ�
	std::map< int, KNPCList >::const_iterator mit;
	for( mit = kNot.m_mapNPCData.begin(); mit != kNot.m_mapNPCData.end(); ++mit )
	{
		// �Ӽ� ���͸� ������ KNpcList�� Index����Ʈ
		std::set< int > setAttribNpc;
		
		const KNPCList& kNPCList = mit->second;
		for( u_int ui = 0; ui < kNPCList.m_NPCList.size(); ++ui )
		{
			if( kNPCList.m_NPCList[ui].m_bHasBossGage )
				bIsBossStage = true;

			const CXSLUnitManager::NPCUnitTemplet* pNpcTemplet = SiCXSLUnitManager()->GetNPCUnitTemplet( static_cast<CXSLUnitManager::NPC_UNIT_ID>(kNPCList.m_NPCList[ui].m_NPCID) );
			if( pNpcTemplet == NULL )
			{
				START_LOG( cerr, L"npc templet�� ã���� �����ϴ�." )
					<< BUILD_LOG( GetDungeonID() )
					<< BUILD_LOGc( GetDifficultyLevel() )
					<< BUILD_LOG( kNPCList.m_NPCList[ui].m_NPCID )
					<< BUILD_LOG( ui )
					<< END_LOG;
				continue;
			}

			if( pNpcTemplet->m_ClassType != CXSLUnitManager::NCT_BASIC )
				continue;

			// �Ӽ�NPC�� �����ϱ����� Pool
			if( SiKAttribNpcTable()->IsNoDropNpc( kNPCList.m_NPCList[ui].m_NPCID ) )
				continue;

			setAttribNpc.insert( ui );
		}

		//////////////////////////////////////////////////////////////////////////
		// �Ӽ� NPC ��� - npcUID �߱��Ŀ� ó���Ұ�!

		// 1. �� ���������� �ѹ���..
		if( !bDropAttribNpc )
			continue;

		// 2. ���� �������������� �Ӽ� npc ���� ���ϵ���
		if( bIsBossStage )
			continue;

		// 3. �Ӽ�NPC ���ī��Ʈ
		if( m_iAttribNpcStageCnt <= 0 )
			continue;

		// 4. �ش� SubStage���� ��ӿ��� ����
		if( iRandomSubStage != mit->first )
			continue;

		// 5. ���� ����� �� npc�� �ֳ� �˻�
		if( setAttribNpc.empty() )
			continue;

		// 6. �Ӽ�NPC ���� m_iAttribNpcDropCnt : �Ӽ� NPC count
		for( int iDropCnt = 0; iDropCnt < m_iAttribNpcDropCnt; ++iDropCnt )
		{
			if( setAttribNpc.size() <= 0 )
				break;

			// 6-1. SubStage�� Npc����Ʈ���� ���� ī��Ʈ ���
			int iNpcIdx = rand() % static_cast<int>(setAttribNpc.size());

			// 6-2. �ش� Npc Index���
			int iCnt = 0;
			std::set< int >::const_iterator sit;
			for( sit = setAttribNpc.begin(); sit != setAttribNpc.end(); ++sit )
			{
				if( iNpcIdx == iCnt )
				{
					iNpcIdx = *sit;
					break;
				}

				++iCnt;
			}
			setAttribNpc.erase( iNpcIdx );

			// 6-3. ����ó��
			if( iNpcIdx < 0  ||  iNpcIdx >= static_cast<int>(kNPCList.m_NPCList.size()) )
				continue;

			// 6-4. Attrib Npc ���� ���
			const KNPCUnitReq& kNpcInfo = kNPCList.m_NPCList[iNpcIdx];

			KAttribEnchantNpcInfo kAttribNpcInfo;

			if( m_eDungeonType == CXSLDungeon::DT_NORMAL )
			{
				if( SiKAttribNpcTable()->GetAttribNpcInfo( m_eDungeonType, m_cDifficultyLevel, kNpcInfo.m_Level, kNpcInfo.m_NPCID, kAttribNpcInfo ) == false )
				{
					START_LOG( cerr, L"�Ӽ� NPC���� ��� ����!" )
						<< BUILD_LOG( GetDungeonID() )
						<< BUILD_LOG( m_eDungeonType )
						<< BUILD_LOGc( m_cDifficultyLevel )
						<< BUILD_LOGc( kNpcInfo.m_Level )
						<< BUILD_LOG( kNpcInfo.m_NPCID )
						<< END_LOG;
					continue;
				}
			}
			else
			{
				if( SiKAttribNpcTable()->GetAttribNpcInfo( m_eDungeonType, m_cDungeonMode, kNpcInfo.m_Level, kNpcInfo.m_NPCID, kAttribNpcInfo ) == false )
				{
					START_LOG( cerr, L"�Ӽ� NPC���� ��� ����!" )
						<< BUILD_LOG( GetDungeonID() )
						<< BUILD_LOG( m_eDungeonType )
						<< BUILD_LOGc( m_cDungeonMode )
						<< BUILD_LOGc( kNpcInfo.m_Level )
						<< BUILD_LOG( kNpcInfo.m_NPCID )
						<< END_LOG;
					continue;
				}
			}

			kNot.m_mapAttribNpcInfo.insert( std::make_pair( kNpcInfo.m_UID, kAttribNpcInfo ) );

			// 6-5. �ش� �Ӽ� ������ ������ ���� ���� ����
			m_mapAttribNpcData.insert( std::make_pair( kNpcInfo.m_UID, static_cast<int>(kNpcInfo.m_Level) ) );
		}

		// 7. �ش� SubStage�� �Ӽ�NPC ���� �������Ƿ� ī��Ʈ 1����
		--m_iAttribNpcStageCnt;

		// 8. ����Ʈ ���� �������� ������ ������ ���꽺�������� ���� ��Ű�� ����~!
		bDropAttribNpc = false;

		//////////////////////////////////////////////////////////////////////////
	}
}

void KDungeonRoom::SecretDungeonExtraStageAttribMonsterDrop( IN OUT KEGS_DUNGEON_STAGE_LOAD_NOT& kNot )
{
	bool bIsBossStage = false;

	// �Ӽ� ���� �����ϱ�
	std::map< int, KNPCList >::const_iterator mit;
	for( mit = kNot.m_mapNPCData.begin(); mit != kNot.m_mapNPCData.end(); ++mit )
	{
		// �Ӽ� ���͸� ������ KNpcList�� Index����Ʈ
		std::set< int > setAttribNpc;

		const KNPCList& kNPCList = mit->second;
		for( u_int ui = 0; ui < kNPCList.m_NPCList.size(); ++ui )
		{
			if( kNPCList.m_NPCList[ui].m_bHasBossGage )
				bIsBossStage = true;

			const CXSLUnitManager::NPCUnitTemplet* pNpcTemplet = SiCXSLUnitManager()->GetNPCUnitTemplet( static_cast<CXSLUnitManager::NPC_UNIT_ID>(kNPCList.m_NPCList[ui].m_NPCID) );
			if( pNpcTemplet == NULL )
			{
				START_LOG( cerr, L"npc templet�� ã���� �����ϴ�." )
					<< BUILD_LOG( GetDungeonID() )
					<< BUILD_LOGc( GetDifficultyLevel() )
					<< BUILD_LOG( kNPCList.m_NPCList[ui].m_NPCID )
					<< END_LOG;
				continue;
			}

			if( pNpcTemplet->m_ClassType != CXSLUnitManager::NCT_BASIC )
				continue;

			// �Ӽ�NPC�� �����ϱ����� Pool
			if( SiKAttribNpcTable()->IsNoDropNpc( kNPCList.m_NPCList[ui].m_NPCID ) )
				continue;

			setAttribNpc.insert( ui );
		}

		// 1. ���� �������������� �Ӽ� npc ���� ���ϵ��� ����?
		if( bIsBossStage )
			continue;

		// 2. ���� ����� �� npc�� �ֳ� �˻�
		if( setAttribNpc.empty() )
			continue;

		// 3. �Ӽ�NPC ����
		//{{ Iruha : 2026-08-27 // VS2010 port: missing type specifier defaulted to int under
		// VC7.1's lenient implicit-int rule; VC10 makes that a hard error (C4430). int matches
		// how the value is compared against below (int iDropCnt).
		const int iAttribNpcDropCnt = setAttribNpc.size() / 2; // �ش� ���� ���������� npc������ 2���� 1�� �Ӽ����ͷ� ����!
		//}}

		for( int iDropCnt = 0; iDropCnt < iAttribNpcDropCnt; ++iDropCnt )
		{
			if( setAttribNpc.size() <= 0 )
				break;

			// 4-1. SubStage�� Npc����Ʈ���� ���� ī��Ʈ ���
			int iNpcIdx = rand() % static_cast<int>(setAttribNpc.size());

			// 4-2. �ش� Npc Index���
			int iCnt = 0;
			std::set< int >::const_iterator sit;
			for( sit = setAttribNpc.begin(); sit != setAttribNpc.end(); ++sit )
			{
				if( iNpcIdx == iCnt )
				{
					iNpcIdx = *sit;
					break;
				}

				++iCnt;
			}
			setAttribNpc.erase( iNpcIdx );

			// 4-3. ����ó��
			if( iNpcIdx < 0  ||  iNpcIdx >= static_cast<int>(kNPCList.m_NPCList.size()) )
				continue;

			// 4-4. Attrib Npc ���� ���
			const KNPCUnitReq& kNpcInfo = kNPCList.m_NPCList[iNpcIdx];

			KAttribEnchantNpcInfo kAttribNpcInfo;
			if( SiKAttribNpcTable()->GetAttribNpcInfoExtraStage( kNpcInfo.m_NPCID, kAttribNpcInfo ) == false )
			{
				START_LOG( cerr, L"�Ӽ� NPC���� ��� ����!" )
					<< BUILD_LOG( kNpcInfo.m_NPCID )
					<< END_LOG;
				continue;
			}

			kNot.m_mapAttribNpcInfo.insert( std::make_pair( kNpcInfo.m_UID, kAttribNpcInfo ) );

			// 4-5. �ش� �Ӽ� ������ ������ ���� ���� ����
			m_mapAttribNpcData.insert( std::make_pair( kNpcInfo.m_UID, static_cast<int>(kNpcInfo.m_Level) ) );
		}
	}
}

#endif SERV_SECRET_HELL
//}}

bool KDungeonRoom::IsAttribNpc( IN int iNpcUID, OUT int& iNpcLv )
{
	std::map< int, int >::const_iterator mit;
	mit = m_mapAttribNpcData.find( iNpcUID );
	if( mit == m_mapAttribNpcData.end() )
		return false;

	iNpcLv = mit->second;
	return true;
}

bool KDungeonRoom::IsDropDieState( char cState )
{
	switch( cState )
	{
	case KEGS_NPC_UNIT_DIE_REQ::NDS_KILL_BY_USER:
	case KEGS_NPC_UNIT_DIE_REQ::NDS_KILL_SELF:
	case KEGS_NPC_UNIT_DIE_REQ::NDS_BONUS_DROP:
		return true;

	case KEGS_NPC_UNIT_DIE_REQ::NDS_ALLY:
	case KEGS_NPC_UNIT_DIE_REQ::NDS_ESCAPE:
		return false;
	}

	START_LOG( cerr, L"npc������ ���� ���°� �̻��մϴ�." )
		<< BUILD_LOGc( cState )
		<< END_LOG;

	return false;
}

//{{ 2009. 12. 14  ������	��ϸ��ӽ��κ�
void KDungeonRoom::CheckTempInventoryClear()
{
	// ��ϸ� �ð��̶��..
	if( m_eDungeonType == CXSLDungeon::DT_HENIR )
	{
		// ���ڽ����������� ���� ���������� �Ѿ�� �ӽ� �κ� ����
		if( SiKHenirResultTable()->IsClearTempInventoryStage( m_iStageID ) == true )
		{
			// �ӽ� �κ� ����!
			BroadCastID( ERM_CLEAR_TEMP_INVENTORY_NOT );
		}
	}
	// �Ϲ� �����̶��..
	else
	{
		// �ӽ� �κ� ����!
		BroadCastID( ERM_CLEAR_TEMP_INVENTORY_NOT );
	}
}
//}}

//{{ 2010. 01. 05  ������	����������
short KDungeonRoom::GetSubStageExpNpcNum( IN int iSubStageID ) const
{
	std::map< int, int >::const_iterator mit;
	mit = m_mapSubStageExpNpcNum.find( iSubStageID );
	if( mit == m_mapSubStageExpNpcNum.end() )
		return 0;

	return mit->second;
}
//}}
//{{ 2012. 07. 04	������		��ϸ� �ð� �ڵ� ��Ȱ ���� ����
#ifdef SERV_AUTO_RESURRECTION_BUG_FIX
void KDungeonRoom::HenirAutoResurrection( IN const UidType iUnitUID )
{
	KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser( iUnitUID );
	if( IS_NULL( spRoomUser ) )
	{
		START_LOG( cerr, L"������ �������� �ʽ��ϴ�" )
			<< BUILD_LOG( iUnitUID )
			<< END_LOG;
		return;
	}

	// Ư�� �������� ���� ��Ȱ!
	//{{ 2011. 08. 12   ��μ�      ��ϸ� ���� 
#ifdef SERV_NEW_HENIR_TEST
	if( SiKHenirResultTable()->IsResurrectionStage( GetStageID() ) == true )
#else
	if( SiKHenirResultTable()->IsResurrectionStage( spRoomUser->GetPassedStageCount() ) == true )
#endif SERV_NEW_HENIR_TEST
		//}}
	{
		// ���� �������� Ȯ��!
		if( spRoomUser->IsDie() == true )
		{
			// �ǻ츮��!
			spRoomUser->SetDie( false );

			KEGS_RESURRECT_TO_CONTINUE_DUNGEON_NOT kPacketNot;
			kPacketNot.m_iUnitUID = spRoomUser->GetCID();
			kPacketNot.m_iUsedByUnitUID = spRoomUser->GetCID();
			kPacketNot.m_iStartPosIndex = SiCXSLDungeonManager()->GetStartPosByRandom( (m_iDungeonID + static_cast<int>(m_cDifficultyLevel)), m_iStageID, m_iSubStageID );
			BroadCast( EGS_RESURRECT_TO_CONTINUE_DUNGEON_NOT, kPacketNot );

			//{{ 2008. 6. 17  ������  �ǽð� ��Ȱ��
			m_spRoomUserManager->SetEndPlay( spRoomUser->GetCID(), false );
			//}}
		}
	}	
}
#endif SERV_AUTO_RESURRECTION_BUG_FIX
//}}

//{{ 2012. 02. 28	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
void KDungeonRoom::IncreasePartyFever()
{
	if( m_iPartyUID <= 0 )
		return;

	if( m_spRoomUserManager->GetNumMember() > 1 )
	{
		KRoomUserPtr spRoomUser = m_spRoomUserManager->GetHostUser(); // ��Ƽ��
		if( spRoomUser != NULL )
		{
			KERM_INCREASE_PARTY_FEVER_NOT kNot;
			kNot.m_iPartyUID = m_iPartyUID;
			kNot.m_cRoomType = CXSLRoom::RT_DUNGEON;
			kNot.m_sSubStageNpcCount = GetSubStageExpNpcNum( GetSubStageID() );
			SendToGSServer( spRoomUser->GetGSUID(), ERM_INCREASE_PARTY_FEVER_NOT, kNot );
		}
		else
		{
			START_LOG( cerr, L"������ �Ѹ��� ���¹��ΰ�?" )
				<< BUILD_LOG( GetStateIDString() )
				<< END_LOG;
		}
	}
}

void KDungeonRoom::MaxPartyFever()
{
	if( m_iPartyUID <= 0 )
		return;

	if( m_spRoomUserManager->GetNumMember() > 1 )
	{
		KRoomUserPtr spRoomUser = m_spRoomUserManager->GetHostUser(); // ��Ƽ��
		if( spRoomUser != NULL )
		{
			KERM_MAX_PARTY_FEVER_NOT kNot;
			kNot.m_iPartyUID = m_iPartyUID;
			SendToGSServer( spRoomUser->GetGSUID(), ERM_MAX_PARTY_FEVER_NOT, kNot );
		}
		else
		{
			START_LOG( cerr, L"������ �Ѹ��� ���¹��ΰ�?" )
				<< BUILD_LOG( GetStateIDString() )
				<< END_LOG;
		}
	}
}

void KDungeonRoom::ReservedRegroupOldParty()
{
	if( m_iPartyUID <= 0 )
		return;

	if( IsStartedByAutoParty() == false )
		return;

	if( m_spRoomUserManager->GetNumMember() > 1 )
	{
		KRoomUserPtr spRoomUser = m_spRoomUserManager->GetHostUser(); // ��Ƽ��
		if( spRoomUser != NULL )
		{
			m_vecReservedRegroupOldParty.clear();

			KERM_END_GAME_REGROUP_PARTY_NOT kNot;
			kNot.m_iGSUID = spRoomUser->GetGSUID();
			kNot.m_iPartyUID = m_iPartyUID;
			m_vecReservedRegroupOldParty.push_back( kNot );
		}
		else
		{
			START_LOG( cerr, L"������ �Ѹ��� ���¹��ΰ�?" )
				<< BUILD_LOG( GetStateIDString() )
				<< END_LOG;
		}
	}    
}

void KDungeonRoom::CheckReserveRegroupOldParty()
{
	BOOST_TEST_FOREACH( const KERM_END_GAME_REGROUP_PARTY_NOT&, kPacketNot, m_vecReservedRegroupOldParty )
	{
		SendToGSServer( kPacketNot.m_iGSUID, ERM_END_GAME_REGROUP_PARTY_NOT, kPacketNot );
		break;
	}
		
	m_vecReservedRegroupOldParty.clear();
}
#endif SERV_BATTLE_FIELD_SYSTEM
//}}

//{{ 2012. 05. 23	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
void KDungeonRoom::TimeCountFinishProcess( IN const u_short usEventID )
{
	switch( usEventID )
	{
	case ERM_SINGLE_OPEN_DUNGEON_ROOM_REQ:
		{
			// ���� �ʱ�ȭ.
			StartGame();	

			//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
			// �� ���� ��û�ڿ��� ack�� ������.
			KERM_PARTY_OPEN_DUNGEON_ROOM_NOT kNot;
			GetRoomInfo( kNot.m_RoomInfo );
			GetRoomSlotInfo( kNot.m_vecSlot );
			GetRoomSlotInfo( kNot.m_vecObserverSlot, KRoomUserManager::UT_OBSERVER );
			
			if( IsStartedByAutoParty() )
			{
				AutoPartyBuff( kNot.m_mapActivateBuff );
			}

			//{{ 2012. 12. 17	������	�ƶ� ��Ƽ �÷��� ���ʽ� ����ġ
#ifdef SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
			PlayWithCharBuff( kNot.m_mapActivateBuff );
#endif SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
			//}}

#ifdef SERV_CRAYON_POP_EVENT_BUFF// �۾���¥: 2013-06-08	// �ڼ���
			CrayonPopEventBuff( kNot.m_mapActivateBuff );
#endif // SERV_CRAYON_POP_EVENT_BUFF

#ifdef SERV_BONUS_BUFF_SYSTEM
			PlayWithBonusBuff( kNot.m_mapActivateBuff );
#endif //SERV_BONUS_BUFF_SYSTEM

			BroadCast( ERM_PARTY_OPEN_DUNGEON_ROOM_NOT, kNot );
#endif SERV_SERVER_BUFF_SYSTEM
			//}
		}
		break;

	case ERM_PARTY_OPEN_DUNGEON_ROOM_REQ:
		{
			// ���� �ʱ�ȭ.
			StartGame();

			//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
			// �� ���� ��û�ڿ��� ack�� ������.
			KERM_PARTY_OPEN_DUNGEON_ROOM_NOT kNot;
			GetRoomInfo( kNot.m_RoomInfo );
			GetRoomSlotInfo( kNot.m_vecSlot );
			GetRoomSlotInfo( kNot.m_vecObserverSlot, KRoomUserManager::UT_OBSERVER );

			if( IsStartedByAutoParty() )
			{
				if( AutoPartyBuff( kNot.m_mapActivateBuff ) == true )
				{
					MaxPartyFever();
				}
			}

			//{{ 2012. 12. 17	������	�ƶ� ��Ƽ �÷��� ���ʽ� ����ġ
#ifdef SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
			PlayWithCharBuff( kNot.m_mapActivateBuff );
#endif SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
			//}}

#ifdef SERV_CRAYON_POP_EVENT_BUFF// �۾���¥: 2013-06-08	// �ڼ���
			CrayonPopEventBuff( kNot.m_mapActivateBuff );
#endif // SERV_CRAYON_POP_EVENT_BUFF

#ifdef SERV_BONUS_BUFF_SYSTEM
			PlayWithBonusBuff( kNot.m_mapActivateBuff );
#endif //SERV_BONUS_BUFF_SYSTEM

			BroadCast( ERM_PARTY_OPEN_DUNGEON_ROOM_NOT, kNot );
#endif SERV_SERVER_BUFF_SYSTEM
			//}
		}
		break;

	default:
		{
			START_LOG( cerr, L"����� ���� �ȵȴ�! �Ͼ���� �ȵǴ� ����!" )
				<< BUILD_LOG( GetUID() )
				<< BUILD_LOG( KEvent::GetIDStr( usEventID ) )
				<< END_LOG;
		}
		break;
	}   
}
#endif SERV_BATTLE_FIELD_SYSTEM
//}}

//{{ 2013. 04. 09	������	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
void KDungeonRoom::PrepareForEnterDefenceDungeon()
{
	// ���� ���� ó��
	EndGame();

	// ���� �Ӽ��� ����� ������ �ٲ���!
	m_iDungeonID = CXSLDungeon::DI_DEFENCE_DUNGEON_ELDER_NORMAL;
	m_cDifficultyLevel = 0;
	m_eDungeonType = CXSLDungeon::DUNGEON_TYPE::DT_NORMAL;
	m_cDungeonMode = CXSLDungeon::DUNGEON_MODE::DM_INVALID;

	// ������ ���¸� �ٲ���!
	StateTransition( KRoomFSM::I_TO_WAIT_FOR_DEFENCE );
	m_kTimer[TM_WAIT_FOR_DEFENCE].restart();

	// ��� ���� ���¸� ��� ���·� �ٲ���!
	m_spRoomUserManager->SetAllPrepareForDefenceDungeon( false );

	// ����� ��� �������� ���� ���� ������ ����� ������ �����ϵ��� ó���Ѵ�.
	KERM_PREPARE_FOR_DEFENCE_DUNGEON_REQ kPacketReq;
	kPacketReq.m_kDefenceDungeonInfo.m_iDungeonID = GetDungeonID();
	kPacketReq.m_kDefenceDungeonInfo.m_cDifficulty = GetDifficultyLevel();
	kPacketReq.m_kDefenceDungeonInfo.m_cDungeonMode = m_cDungeonMode;
	BroadCast( ERM_PREPARE_FOR_DEFENCE_DUNGEON_REQ, kPacketReq );
}

//{{ 2013. 04. 22	������	����� �� ���� ���� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON_ENTER_LOGIC
void KDungeonRoom::EnterDefenceDungeon( IN const std::vector< UidType >& vecLeaveUnitUID )
{
	// �ϴ� ���� ��Ż�ڵ���� ��Żó������!
	BOOST_TEST_FOREACH( const UidType, iUnitUID, vecLeaveUnitUID )
	{
		KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser( iUnitUID );
		if( IS_NULL( spRoomUser ) )
		{
			START_LOG( cerr, L"������ �������� �ʽ��ϴ�." )
				<< BUILD_LOG( iUnitUID )
				<< END_LOG;
			continue;
		}

		// �ش� ����� ������Ż!
		if( m_spRoomUserManager->LeaveRoom( iUnitUID ) == false )
		{
			START_LOG( cerr, L"�� ������ ����." )
				<< BUILD_LOG( iUnitUID )
				<< END_LOG;
		}

		// ������ ������ ����� ������ ���� ��Ű�� ���� ���� ������ ����!
		//KEGS_RESULT_ALL_UNIT_SUCCESS_NOT kNot;
		//GetRoomInfo( kNot.m_RoomInfo );
		//GetRoomSlotInfo( kNot.m_vecSlot );
		SendToGSCharacter( spRoomUser->GetGSUID(), spRoomUser->GetCID(), ERM_LEAVE_ROOM_BEFORE_DEFENCE_DUNGEON_START_NOT, char() );

		// ���� ��� �����鿡�� ���� ������ ������.
		KEGS_LEAVE_ROOM_NOT kPacketNot;
		kPacketNot.m_cRoomState = GetStateID();
		kPacketNot.m_iUnitUID = iUnitUID;
		GetRoomSlotInfo( kPacketNot.m_vecSlot );
		BroadCast( ERM_LEAVE_ROOM_NOT, kPacketNot );
	}

	// ��Ż �ϰ����� ���࿡ �濡 �ƹ��� ������ �״�� ���� ���� ��Ű��!
	if( IsEmpty() == true )
	{
		StateTransition( KRoomFSM::I_TO_CLOSE );
		return;
	}

	// ���� ����
	StateTransition( KRoomFSM::I_TO_WAIT );

	//����������¸� ����� �ٲ۴�.
	m_spRoomUserManager->SetAllReady( true );

	// ���� �ʱ�ȭ.
	StartGame();
#ifdef SERV_CRAYON_POP_EVENT_BUFF// �۾���¥: 2013-06-12	// �ڼ���
	{
		KBuffInfo	kBuff;
		KLottery	kLot;
		kLot.AddCase( CXSLBuffManager::BTI_BUFF_ELRIN, 45.0f );
		kLot.AddCase( CXSLBuffManager::BTI_BUFF_SOYUL, 30.0f );
		kLot.AddCase( CXSLBuffManager::BTI_BUFF_KUMI, 5.0f );
		kLot.AddCase( CXSLBuffManager::BTI_BUFF_CHOA, 10.0f );
		kLot.AddCase( CXSLBuffManager::BTI_BUFF_WEY, 10.0f );
		kBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID	= kLot.Decision();

		if( kBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID == KLottery::CASE_BLANK )
		{
			kBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = CXSLBuffManager::BTI_BUFF_SOYUL;
		}

		KERM_ACTIVATE_BUFF_NOT kPacketNot;
		kPacketNot.m_vecActivateBuffList.push_back( kBuff );
		BroadCast( ERM_ACTIVATE_BUFF_NOT, kPacketNot );
	}
#endif // SERV_CRAYON_POP_EVENT_BUFF

	// �ٲ� �Ӽ���� ������ �ٽ� ��������!
	KEGS_DEFENCE_DUNGEON_GAME_START_NOT kDefenceDungeonStart;
	GetRoomInfo( kDefenceDungeonStart.m_RoomInfo );
	GetRoomSlotInfo( kDefenceDungeonStart.m_vecSlot );
	GetRoomSlotInfo( kDefenceDungeonStart.m_vecObserverSlot, KRoomUserManager::UT_OBSERVER );
	BroadCast( ERM_DEFENCE_DUNGEON_GAME_START_NOT, kDefenceDungeonStart );
}
#else
//void KDungeonRoom::EnterDefenceDungeon()
//{
//	// ���� ����
//	StateTransition( KRoomFSM::I_TO_WAIT );
//
//	//����������¸� ����� �ٲ۴�.
//	m_spRoomUserManager->SetAllReady( true );
//
//	// ���� �ʱ�ȭ.
//	StartGame();
//
//	// �ٲ� �Ӽ���� ������ �ٽ� ��������!
//	KEGS_DEFENCE_DUNGEON_GAME_START_NOT kDefenceDungeonStart;
//	GetRoomInfo( kDefenceDungeonStart.m_RoomInfo );
//	GetRoomSlotInfo( kDefenceDungeonStart.m_vecSlot );
//	GetRoomSlotInfo( kDefenceDungeonStart.m_vecObserverSlot, KRoomUserManager::UT_OBSERVER );
//	BroadCast( ERM_DEFENCE_DUNGEON_GAME_START_NOT, kDefenceDungeonStart );    
//}
#endif SERV_NEW_DEFENCE_DUNGEON_ENTER_LOGIC
//}}
#endif SERV_NEW_DEFENCE_DUNGEON
//}}

//////////////////////////////////////////////////////////////////////////
// ����μ��� Ʃ�丮�� ������ �� ��Ŷ���� ������ �����ϰ��ִ�!
//////////////////////////////////////////////////////////////////////////
_IMPL_ON_FUNC( ERM_OPEN_DUNGEON_ROOM_REQ, KERM_OPEN_ROOM_REQ )
{
    VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_INIT ), ERM_OPEN_ROOM_ACK, KEGS_CREATE_ROOM_ACK );

	//{{ 2013. 04. 16	�ڼ���	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
	if( CXSLDungeon::IsDefenceDungeon( kPacket_.m_kRoomInfo.m_iDungeonID ) == true )
	{
		START_LOG( cerr, L"��Ŷ ���� �ǽ� ����! ( ����� �� ������ ���������� �����Ϸ� �Ͽ��� )" )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< BUILD_LOG( GetUID() )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_ROOM_30;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_OPEN_ROOM_ACK, kPacket );
		return;
	}
#endif SERV_NEW_DEFENCE_DUNGEON
	//}}

	//{{ 2010. 03. 14  ������	�� �й� ��� ����
#ifdef SERV_ROOM_COUNT
	SendRoomCountInfo( CXSLRoom::RT_DUNGEON, true );
#endif SERV_ROOM_COUNT
	//}}

    // �� �ɼ� ���� ����.
	SetRoomInfo( kPacket_.m_kRoomInfo );

    // �� �����̹Ƿ� ���� ������ �ʱ�ȭ �Ѵ�.
    m_spRoomUserManager->Reset();
    m_spRoomUserManager->AssignTeam( -1 );

    // ���Կ� ���� ��û�ڸ� �ִ´�.
    if( m_spRoomUserManager->EnterRoom( kPacket_.m_kRoomUserInfo, false ) == false )
    {
        START_LOG( cerr, L"�� ���� ����." )
            << BUILD_LOG( FIRST_SENDER_UID )
            << BUILD_LOG( GetUID() )
            << END_LOG;

        kPacket.m_iOK = NetError::ERR_ROOM_30;
        SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_OPEN_ROOM_ACK, kPacket );
        return;
    }

	//080409.hoons. ���� ����Ʈ�� �����Ѵ�.
	m_spRoomUserManager->SetTutorialUIDList( FIRST_SENDER_UID, kPacket_.m_vecStudentUnitUID );

    // �� ���¸� wait �� �ٲ۴�.
    StateTransition( KRoomFSM::I_TO_WAIT );
    m_kTimer[TM_WAIT].restart();

	//gm �̻��̸� ���ͼ����� ip�� �־�����.
	//if( kPacket_.m_kUnitInfo.m_cAuthLevel >= SEnum::UAL_GM )
	if( m_spRoomUserManager->CheckAuthLevel( FIRST_SENDER_UID ) == true )
	{
		kPacket.m_wstrCNIP = NetCommon::GetLocalIPW();
	}

    // �� ���� ��û�ڿ��� ack�� ������.
    kPacket.m_iOK = NetError::NET_OK;
	GetRoomInfo( kPacket.m_RoomInfo );

	GetRoomSlotInfo( kPacket.m_vecSlot );
	GetRoomSlotInfo( kPacket.m_vecObserverSlot, KRoomUserManager::UT_OBSERVER );

    SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_OPEN_ROOM_ACK, kPacket );

	//������� �������� üũ����.
	m_spRoomUserManager->ZU_CheckStart();
}

IMPL_ON_FUNC( ERM_PARTY_OPEN_DUNGEON_ROOM_REQ )
{
	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_INIT ), ERM_PARTY_OPEN_DUNGEON_ROOM_ACK, KERM_PARTY_OPEN_DUNGEON_ROOM_ACK );

	//{{ 2013. 04. 16	�ڼ���	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
	if( CXSLDungeon::IsDefenceDungeon( kPacket_.m_kRoomInfo.m_iDungeonID ) == true )
	{
		START_LOG( cerr, L"��Ŷ ���� �ǽ� ����! ( ����� �� ������ ���������� �����Ϸ� �Ͽ��� )" )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< BUILD_LOG( GetUID() )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_ROOM_30;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_PARTY_OPEN_DUNGEON_ROOM_ACK, kPacket );
		return;
	}
#endif SERV_NEW_DEFENCE_DUNGEON
	//}}

	//{{ 2010. 03. 14  ������	�� �й� ��� ����
#ifdef SERV_ROOM_COUNT
	SendRoomCountInfo( CXSLRoom::RT_DUNGEON, true );
#endif SERV_ROOM_COUNT
	//}}

	// �� �ɼ� ���� ����.
	SetRoomInfo( kPacket_.m_kRoomInfo );

	// �� �����̹Ƿ� ���� ������ �ʱ�ȭ �Ѵ�.
	m_spRoomUserManager->Reset();
	m_spRoomUserManager->AssignTeam( -1 );

	BOOST_TEST_FOREACH( const KPartyRoomUserInfo&, kInfo, kPacket_.m_vecUserInfo )
	{
		// ���Կ� ���� �ִ´�.
		if( m_spRoomUserManager->EnterRoom( kInfo.m_kRoomUserInfo, false ) == false )
		{
			START_LOG( cerr, L"��Ƽ ����� ���� ����." )
				<< BUILD_LOG( FIRST_SENDER_UID )
				<< BUILD_LOG( GetUID() )
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_ROOM_30;
			SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_PARTY_OPEN_DUNGEON_ROOM_ACK, kPacket );
			return;
		}
		
		m_spRoomUserManager->SetTutorialUIDList( kInfo.m_kRoomUserInfo.m_nUnitUID, kInfo.m_vecStudentUnitUID );
		//{{ 2009. 6. 10  ������	���� ���۽� EXP,ED
		m_spRoomUserManager->UpdateetOldExpED( kInfo.m_kRoomUserInfo.m_nUnitUID, kInfo.m_iCurEXP, kInfo.m_iCurED );
		//}}
		//{{ 2012. 07. 03	������		���� �α� �÷� �߰�
#ifdef SERV_ADD_DUNGEON_LOG_COLUMN_NUM_2
		m_spRoomUserManager->SetGameChannelIP( kInfo.m_kRoomUserInfo.m_nUnitUID, kInfo.m_wstrChannelIP );
#endif SERV_ADD_DUNGEON_LOG_COLUMN_NUM_2
		//}}
		//{{ 2012. 03. 26	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
		m_spRoomUserManager->SetActivatedAutoPartyBonus( kInfo.m_kRoomUserInfo.m_nUnitUID, kInfo.m_bAutoPartyBonus );
		m_spRoomUserManager->SetOldPartyInfo( kInfo.m_kRoomUserInfo.m_nUnitUID, kInfo.m_kOldPartyInfo );
#endif SERV_BATTLE_FIELD_SYSTEM
		//}}
		//{{ �ڵ� ��Ƽ�� ���� ���� �� ��Ī ��� �ð� �α� �߰� - ��μ�
#ifdef SERV_AUTO_PARTY_MATCHING_REQUIRED_TIME_LOG
		m_spRoomUserManager->SetAutoPartyWaitTime( kInfo.m_kRoomUserInfo.m_nUnitUID, kInfo.m_iAutoPartyWaitTime );
#endif SERV_AUTO_PARTY_MATCHING_REQUIRED_TIME_LOG
		//}
	}

	// �ο�����ó���� �߸��Ǿ� ��Ƽ�� �����Ҽ� ����.
	if( true == IsEmpty() )
	{
		kPacket.m_iOK = NetError::ERR_PARTY_23;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_PARTY_OPEN_DUNGEON_ROOM_ACK, kPacket );
		return;
	}

	// �� ���¸� wait �� �ٲ۴�.
	StateTransition( KRoomFSM::I_TO_WAIT );
	m_kTimer[TM_WAIT].restart();

	//����������¸� ����� �ٲ۴�.
	m_spRoomUserManager->SetAllReady(true);

	//////////////////////////////////////////////////////////////////////////
	//{{ 2012. 05. 23	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	// 3�� ī��Ʈ
	TimeCountForStartGame( ERM_PARTY_OPEN_DUNGEON_ROOM_REQ );
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}
	//////////////////////////////////////////////////////////////////////////

	//������ �غ� �Ϸ�Ǿ�ٰ� ��Ƽ�忡�� �˷���.
	kPacket.m_iOK = NetError::NET_OK;
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_PARTY_OPEN_DUNGEON_ROOM_ACK, kPacket );	
}

//{{ 2011. 12. 09	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
_IMPL_ON_FUNC( ERM_SINGLE_OPEN_DUNGEON_ROOM_REQ, KERM_OPEN_ROOM_REQ )
{
	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_INIT ), ERM_SINGLE_OPEN_DUNGEON_ROOM_ACK, KERM_SINGLE_OPEN_DUNGEON_ROOM_ACK );

	//{{ 2013. 04. 16	�ڼ���	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
	if( CXSLDungeon::IsDefenceDungeon( kPacket_.m_kRoomInfo.m_iDungeonID ) == true )
	{
		START_LOG( cerr, L"��Ŷ ���� �ǽ� ����! ( ����� �� ������ ���������� �����Ϸ� �Ͽ��� )" )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< BUILD_LOG( GetUID() )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_ROOM_30;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SINGLE_OPEN_DUNGEON_ROOM_ACK, kPacket );
		return;
	}
#endif SERV_NEW_DEFENCE_DUNGEON
	//}}

	//{{ 2010. 03. 14  ������	�� �й� ��� ����
#ifdef SERV_ROOM_COUNT
	SendRoomCountInfo( CXSLRoom::RT_DUNGEON, true );
#endif SERV_ROOM_COUNT
	//}}

	// �� �ɼ� ���� ����.
	SetRoomInfo( kPacket_.m_kRoomInfo );

	// �� �����̹Ƿ� ���� ������ �ʱ�ȭ �Ѵ�.
	m_spRoomUserManager->Reset();
	m_spRoomUserManager->AssignTeam( -1 );

	// ���Կ� ���� ��û�ڸ� �ִ´�.
	if( m_spRoomUserManager->EnterRoom( kPacket_.m_kRoomUserInfo, false ) == false )
	{
		START_LOG( cerr, L"�� ���� ����." )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< BUILD_LOG( GetUID() )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_ROOM_30;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SINGLE_OPEN_DUNGEON_ROOM_ACK, kPacket );
		return;
	}

	//080409.hoons. ���� ����Ʈ�� �����Ѵ�.
	m_spRoomUserManager->SetTutorialUIDList( FIRST_SENDER_UID, kPacket_.m_vecStudentUnitUID );
	//{{ 2009. 6. 10  ������	���� ���۽� EXP,ED
	m_spRoomUserManager->UpdateetOldExpED( kPacket_.m_kRoomUserInfo.m_nUnitUID, kPacket_.m_iCurEXP, kPacket_.m_iCurED );
	//}}
	//{{ 2012. 07. 03	������		���� �α� �÷� �߰�
#ifdef SERV_ADD_DUNGEON_LOG_COLUMN_NUM_2
	m_spRoomUserManager->SetGameChannelIP( kPacket_.m_kRoomUserInfo.m_nUnitUID, kPacket_.m_wstrChannelIP );
#endif SERV_ADD_DUNGEON_LOG_COLUMN_NUM_2
	//}}

	// �� ���¸� wait �� �ٲ۴�.
	StateTransition( KRoomFSM::I_TO_WAIT );
	m_kTimer[TM_WAIT].restart();

	//gm �̻��̸� ���ͼ����� ip�� �־�����.
	//if( kPacket_.m_kUnitInfo.m_cAuthLevel >= SEnum::UAL_GM )
	//if( m_spRoomUserManager->CheckAuthLevel( FIRST_SENDER_UID ) == true )
	//{
	//	kPacket.m_wstrCNIP = NetCommon::GetLocalIPW();
	//}

	m_spRoomUserManager->SetAllReady(true);

	//////////////////////////////////////////////////////////////////////////
	//{{ 2012. 05. 23	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	// 3�� ī��Ʈ
	TimeCountForStartGame( ERM_SINGLE_OPEN_DUNGEON_ROOM_REQ );
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}
	//////////////////////////////////////////////////////////////////////////
	
	kPacket.m_iOK = NetError::NET_OK;
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SINGLE_OPEN_DUNGEON_ROOM_ACK, kPacket );
}

//_IMPL_ON_FUNC( ERM_SINGLE_DUNGEON_GAME_REPLAY_REQ, KERM_OPEN_ROOM_REQ )
//{
//	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_WAIT ), ERM_SINGLE_DUNGEON_GAME_REPLAY_ACK, KEGS_QUICK_START_DUNGEON_GAME_ACK );
//
//	// �� �ɼ� ���� ����.
//	SetRoomInfo( kPacket_.m_kRoomInfo );
//
//	// �ش� ���������� ������Ʈ�Ѵ�.
//	if( m_spRoomUserManager->SetRoomUserInfo( kPacket_.m_kRoomUserInfo.m_nUnitUID, kPacket_.m_kRoomUserInfo ) == false )
//	{
//		START_LOG( cerr, L"��Ƽ ����� ���÷��� ����. ������Ʈ �Ϸ��� ������ �濡 ����" )
//			<< BUILD_LOG( FIRST_SENDER_UID )
//			<< BUILD_LOG( GetUID() )
//			<< END_LOG;
//
//		kPacket.m_iOK = NetError::ERR_ROOM_30;
//		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SINGLE_DUNGEON_GAME_REPLAY_ACK, kPacket );
//
//		// [����] ���� ó���� �ұ�? ������Ʈ ���� ��Ż�ѰŶ�� �׳༮�� �����ϰ� ������ �����ع�����?
//		return;
//	}
//	else
//	{
//		m_spRoomUserManager->SetTutorialUIDList( kPacket_.m_kRoomUserInfo.m_nUnitUID, kPacket_.m_vecStudentUnitUID );
//		//{{ 2009. 6. 10  ������	���� ���۽� EXP,ED
//		m_spRoomUserManager->UpdateetOldExpED( kPacket_.m_kRoomUserInfo.m_nUnitUID, kPacket_.m_iCurEXP, kPacket_.m_iCurED );
//		//}}
//	}
//
//	//����������¸� ����� �ٲ۴�.
//	m_spRoomUserManager->SetAllReady(true);
//
//	//������ �غ� �Ϸ�Ǿ�ٰ� ��Ƽ�忡�� �˷���.
//	kPacket.m_iOK = NetError::NET_OK;
//	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SINGLE_DUNGEON_GAME_REPLAY_ACK, kPacket );
//
//	// ���� �ʱ�ȭ.
//	StartGame();
//
//	//����� ���� & ���ӽ��������� �����ؼ� Not �� ������.
//	KEGS_PARTY_GAME_START_NOT kNot;
//
//	//���ͼ����� ip�� �־��ְ� gsuser���� ������ �̻��� �ƴϸ� ������.
//	if( m_spRoomUserManager->CheckAuthLevel( FIRST_SENDER_UID ) == true )
//	{
//		kNot.m_wstrCNIP = NetCommon::GetLocalIPW();
//	}
//
//	GetRoomInfo( kNot.m_RoomInfo );
//	GetRoomSlotInfo( kNot.m_vecSlot );
//	GetRoomSlotInfo( kNot.m_vecObserverSlot, KRoomUserManager::UT_OBSERVER );
//
//	BroadCast( ERM_PARTY_GAME_REPLAY_START_NOT, kNot );
//}

#endif SERV_BATTLE_FIELD_SYSTEM
//}}

IMPL_ON_FUNC( ERM_GAME_START_REQ )
{
	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_WAIT ), ERM_GAME_START_ACK, KERM_GAME_START_ACK );

	if( m_spRoomUserManager->IsObserver( FIRST_SENDER_UID ) == false )
	{
		// �������� üũ�Ѵ�.
		VERIFY_HOST( ERM_GAME_START_ACK );
	}

	// ��� �÷��̾ �غ� �������� üũ�Ѵ�.
	if( !m_spRoomUserManager->IsAllPlayerReady() )
	{
		kPacket.m_iOK = NetError::ERR_ROOM_10;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_GAME_START_ACK, kPacket );
		return;
	}

	// ���� �ʱ�ȭ.
	StartGame();

	// ���忡�� ack�� ������. �ټ��� ����ġ�� ����
	kPacket.m_iOK			  = NetError::NET_OK;
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_GAME_START_ACK, kPacket );

	// ���� ��� �����鿡�� not�� ������.
	KERM_GAME_START_NOT kPacketNot;
	kPacketNot.m_cRoomState = GetStateID();
	kPacketNot.m_iGameType	= m_cGameType;
	GetRoomSlotInfo( kPacketNot.m_vecSlot );
	kPacketNot.m_iDungeonID	= m_iDungeonID + static_cast<int>(m_cDifficultyLevel);
	kPacketNot.m_sWorldID	= m_sWorldID;
	BroadCast( ERM_GAME_START_NOT, kPacketNot );
}

//{{ 2011. 10. 25	������	��Ʋ�ʵ� �ý���
//////////////////////////////////////////////////////////////////////////
#ifdef SERV_BATTLE_FIELD_SYSTEM
//////////////////////////////////////////////////////////////////////////

IMPL_ON_FUNC( ERM_LEAVE_ROOM_REQ )
{
	OnLeaveRoom( LAST_SENDER_UID, FIRST_SENDER_UID, kPacket_, ERM_LEAVE_ROOM_ACK );
}

_IMPL_ON_FUNC( ERM_LEAVE_ROOM_FOR_JOIN_BATTLE_FIELD_REQ, KERM_LEAVE_ROOM_REQ )
{
	OnLeaveRoom( LAST_SENDER_UID, FIRST_SENDER_UID, kPacket_, ERM_LEAVE_ROOM_FOR_JOIN_BATTLE_FIELD_ACK );
}

_IMPL_ON_FUNC( ERM_LEAVE_ROOM_FOR_ENTER_THE_VILLAGE_REQ, KERM_LEAVE_ROOM_REQ )
{
	OnLeaveRoom( LAST_SENDER_UID, FIRST_SENDER_UID, kPacket_, ERM_LEAVE_ROOM_FOR_ENTER_THE_VILLAGE_ACK );
}

_IMPL_ON_FUNC( ERM_DUNGEON_STAGE_LOAD_REQ, KEGS_DUNGEON_STAGE_LOAD_REQ )
{
	//////////////////////////////////////////////////////////////////////////
	// ���� ���� ���̽�
	// 1. S_RESULT : 2���̻� ��Ƽ���� �Ѹ� �������¿��� �ٸ� �Ѹ��� �������� Ŭ����� ���ÿ� ������Ż�� �ϸ� �߻�
	//////////////////////////////////////////////////////////////////////////

	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_PLAY ), ERM_DUNGEON_STAGE_LOAD_ACK, KEGS_DUNGEON_STAGE_LOAD_ACK );

#ifdef SERV_STAGE_CLEAR_IN_SERVER
	if( m_spRoomUserManager->CheckAuthLevel( FIRST_SENDER_UID ) == false )
	{
		if( m_bFirstStage == false )
		{
			if( m_kNextStageData.m_eClearType != CXSLDungeonSubStage::CT_STAGE &&
				m_kNextStageData.m_eClearType != CXSLDungeonSubStage::CT_STAGE_FORCE &&
				m_kNextStageData.m_eClearType != CXSLDungeonSubStage::CT_STAGE_SECRET )
			{
				// ������ ����� Ŭ���� Ÿ���� �ƴ�. Ŭ���̾�Ʈ ��ŷ�̴�. by ������
				START_LOG( cerr, L"Ŭ����Ÿ���� �Ʒ��� ������ �� ��Ŷ�� �� ����? ��Ŀ!" )
					<< BUILD_LOG( m_bFirstStage )
					<< BUILD_LOG( m_kNextStageData.m_eClearType )
					<< BUILD_LOG( m_kNextStageData.m_iStageIndex )
					<< END_LOG;

				kPacket.m_iOK = NetError::ERR_ROOM_51;
				SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_STAGE_LOAD_ACK, kPacket );
				return;
			}

			// Ÿ�� ��ü�� ���� ���������� �ٸ��ٸ� ���ø� ����� �ؼ� ���� �����ϰ� ����. by ������
			kPacket_.m_iStageID = m_kNextStageData.m_iStageIndex;
		}
		else
		{
			// �������� �ε� üũ�� �� �ص� �Ǵ°� ó�� �ѹ� ���̴�. by ������
			kPacket_.m_iStageID = 0;
			m_bFirstStage = false;
		}
	}

	// �������� ���� �Ŀ� �ٷ� 0�� ���� ���������� ����� �ϹǷ� ���� by ������
	m_bFirstSubStage = true;

	// ������ �ٽ� ���� ���� �� �ʱ�ȭ.
	m_kNextStageData.ClearData();
#endif SERV_STAGE_CLEAR_IN_SERVER

	//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER
	// ���潺 ������ �ʱ�ȭ
	m_iDefenseMainCrystalNpcUID = 0;
	m_setDefenseSmallCrystalNpcUID.clear();
#endif SERV_INSERT_GLOBAL_SERVER
	//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�

	//{{ 2012. 12. 03 ���� �������� �ߺ� �ε� ���� Ȯ�� - ��μ�
#ifdef SERV_AUTO_HACK_CHECK_STAGE_LOAD_2
	{
		std::map< int, int >::iterator mitSLC = m_mapStageLoadingCheck.find( kPacket_.m_iStageID );
		if( mitSLC != m_mapStageLoadingCheck.end() )
		{
			START_LOG( cout, L"�Ȱ��� �������� �� �ε��߳�? �̰� ����������?" )
				<< BUILD_LOG( LAST_SENDER_UID )
				<< BUILD_LOG( FIRST_SENDER_UID )
				<< BUILD_LOG( GetDungeonIDAndDif() )
				<< BUILD_LOG( GetStageID() )
				<< BUILD_LOG( GetSubStageID() )
				<< BUILD_LOG( m_mapStageLoadingCheck.size() )
				<< BUILD_LOG( mitSLC->second )
				<< BUILD_LOG( kPacket_.m_iStageID )
				<< BUILD_LOG( m_spRoomUserManager->GetNumMember() );

			if( m_iLastLoadingUnitUID != FIRST_SENDER_UID )
			{
				START_LOG( cout, L"������ �ٲ����? �� �� �������� �ε��� ������ �ٸ� ������.." )					
					<< BUILD_LOG( FIRST_SENDER_UID )
					<< BUILD_LOG( m_iLastLoadingUnitUID );
			}

			// ���� �������� �ε� ī��Ʈ�� 1 �ø���!
			++mitSLC->second;

			// 5�� �̻� �Ȱ��� �������� �ε��ϴ� ����� ��ŷ������ �Ǻ�!
			if( mitSLC->second >= 5 )
			{
				START_LOG( cout, L"Stage Loading ��ŷ������ �ǴܵǾ� üũ ����� Ŭ���̾�Ʈ �����Ͽ���!" )
					<< BUILD_LOG( LAST_SENDER_UID )
					<< BUILD_LOG( FIRST_SENDER_UID )
					<< BUILD_LOG( GetDungeonIDAndDif() )
					<< BUILD_LOG( GetStageID() )
					<< BUILD_LOG( m_mapStageLoadingCheck.size() )
					<< BUILD_LOG( mitSLC->second )
					<< BUILD_LOG( kPacket_.m_iStageID );

#ifdef SERV_DUNGEON_STAGE_LOAD_LOG// �۾���¥: 2013-05-15	// �ڼ���
				if( SiKGameSysVal()->GetDungeonStageLoadLog() == true )
				{
					DungonStageLoadLog sDungonStageLoadLog;
					sDungonStageLoadLog.m_iUnitUID					= FIRST_SENDER_UID;
					sDungonStageLoadLog.m_cNormalStage				= kPacket_.m_cNormalStage;
					sDungonStageLoadLog.m_cSecretStage				= kPacket_.m_cSecretStage;
					sDungonStageLoadLog.m_cEliteStage				= kPacket_.m_cEliteStage;
					sDungonStageLoadLog.m_cClearConditionCheckCount	= kPacket_.m_cClearConditionCheckCount;
					sDungonStageLoadLog.m_iDungeonID				= GetDungeonIDAndDif();
					sDungonStageLoadLog.m_iNowStageID				= GetStageID();
					sDungonStageLoadLog.m_iReqStageID				= kPacket_.m_iStageID;
					sDungonStageLoadLog.m_bHost						= m_spRoomUserManager->IsHost( FIRST_SENDER_UID );
					sDungonStageLoadLog.m_cMemberNum				= m_spRoomUserManager->GetNumMember();
					m_vecDungonStageLoadLog.push_back( sDungonStageLoadLog );

					START_LOG( cout, L"�׽�Ʈ�α�" )
						<< BUILD_LOG( GetStateID() )
						<< BUILD_LOG( sDungonStageLoadLog.m_iNowStageID )
						<< END_LOG;

					KDBE_DUNGEON_STAGE_LOAD_LOG_NOT kPacketToDB;
					kPacketToDB.m_iRoomUID				= GetUID();
					kPacketToDB.m_vecDungonStageLoadLog	= m_vecDungonStageLoadLog;
					SendToLogDB( DBE_DUNGEON_STAGE_LOAD_LOG_NOT, kPacketToDB );
					m_vecDungonStageLoadLog.clear();
				}
#endif // SERV_DUNGEON_STAGE_LOAD_LOG

				kPacket.m_iOK = NetError::ERR_ROOM_22;
				SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_STAGE_LOAD_ACK, kPacket );

				// ��Ƽ�忡�� ��ŷüũ��Ŷ�� ������!
				KEGS_SERVER_CHECK_HACK_USER_REQ kNot;
				kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_DUNGEON_STAGE_LOADING;
				SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SERVER_CHECK_HACK_USER_NOT, kNot );
				return;
			}
			else
			{
#ifdef SERV_DUNGEON_STAGE_LOAD_LOG// �۾���¥: 2013-05-15	// �ڼ���
				if( SiKGameSysVal()->GetDungeonStageLoadLog() == true )
				{
					DungonStageLoadLog sDungonStageLoadLog;
					sDungonStageLoadLog.m_iUnitUID					= FIRST_SENDER_UID;
					sDungonStageLoadLog.m_cNormalStage				= kPacket_.m_cNormalStage;
					sDungonStageLoadLog.m_cSecretStage				= kPacket_.m_cSecretStage;
					sDungonStageLoadLog.m_cEliteStage				= kPacket_.m_cEliteStage;
					sDungonStageLoadLog.m_cClearConditionCheckCount	= kPacket_.m_cClearConditionCheckCount;
					sDungonStageLoadLog.m_iDungeonID				= GetDungeonIDAndDif();
					sDungonStageLoadLog.m_iNowStageID				= GetStageID();
					sDungonStageLoadLog.m_iReqStageID				= kPacket_.m_iStageID;
					sDungonStageLoadLog.m_bHost						= m_spRoomUserManager->IsHost( FIRST_SENDER_UID );
					sDungonStageLoadLog.m_cMemberNum				= m_spRoomUserManager->GetNumMember();
					m_vecDungonStageLoadLog.push_back( sDungonStageLoadLog );
				}
#endif // SERV_DUNGEON_STAGE_LOAD_LOG

				// �ѹ��� ó�� �ϵ��� ����
				kPacket.m_iOK = NetError::ERR_ROOM_22;
				SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_STAGE_LOAD_ACK, kPacket );
				return;
			}
		}
		// NOT �� ���������� ��������ID ����� ����
	}	
#else
	/*
	//{{ 2010. 11. 18	������	���� �� �������� �ε� üũ
#ifdef SERV_AUTO_HACK_CHECK_STAGE_LOAD
	{
		std::map< int, int >::iterator mitSLC = m_mapStageLoadingCheck.find( kPacket_.m_iStageID );
		if( mitSLC != m_mapStageLoadingCheck.end() )
		{
			START_LOG( cout, L"�Ȱ��� �������� �� �ε��߳�? �̰� ����������?" )
				<< BUILD_LOG( LAST_SENDER_UID )
				<< BUILD_LOG( FIRST_SENDER_UID )
				<< BUILD_LOG( GetDungeonIDAndDif() )
				<< BUILD_LOG( GetStageID() )
				<< BUILD_LOG( GetSubStageID() )
				<< BUILD_LOG( m_mapStageLoadingCheck.size() )
				<< BUILD_LOG( mitSLC->second )
				<< BUILD_LOG( kPacket_.m_iStageID )
				<< BUILD_LOG( m_spRoomUserManager->GetNumMember() );

			if( m_iLastLoadingUnitUID != FIRST_SENDER_UID )
			{
				START_LOG( cout, L"������ �ٲ����? �� �� �������� �ε��� ������ �ٸ� ������.." )					
					<< BUILD_LOG( FIRST_SENDER_UID )
					<< BUILD_LOG( m_iLastLoadingUnitUID );
			}

			// ���� �������� �ε� ī��Ʈ�� 1 �ø���!
			++mitSLC->second;

			// ���������� �ε� ������ ������ UnitUID�� ����!
			m_iLastLoadingUnitUID = FIRST_SENDER_UID;

			// 5�� �̻� �Ȱ��� �������� �ε��ϴ� ����� ��ŷ������ �Ǻ�!
			if( mitSLC->second >= 5 )
			{
				START_LOG( cout, L"Stage Loading ��ŷ������ �ǴܵǾ� üũ ����� Ŭ���̾�Ʈ �����Ͽ���!" )
					<< BUILD_LOG( LAST_SENDER_UID )
					<< BUILD_LOG( FIRST_SENDER_UID )
					<< BUILD_LOG( GetDungeonIDAndDif() )
					<< BUILD_LOG( GetStageID() )
					<< BUILD_LOG( m_mapStageLoadingCheck.size() )
					<< BUILD_LOG( mitSLC->second )
					<< BUILD_LOG( kPacket_.m_iStageID );

				kPacket.m_iOK = NetError::ERR_ROOM_22;
				SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_STAGE_LOAD_ACK, kPacket );

				// ��Ƽ�忡�� ��ŷüũ��Ŷ�� ������!
				//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
				KEGS_SERVER_CHECK_HACK_USER_REQ kNot;
				kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_DUNGEON_STAGE_LOADING;
#else
				KEGS_SERVER_CHECK_HACK_USER_NOT kNot;
				kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_DUNGEON_STAGE_LOADING;
#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
				//}}
				SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SERVER_CHECK_HACK_USER_NOT, kNot );
				return;
			}
		}
		else
		{
			// �ε����� ���� StageID�� �����Ѵ�.
			m_mapStageLoadingCheck.insert( std::make_pair( kPacket_.m_iStageID, 1 ) );

			// ���������� �ε� ������ ������ UnitUID�� ����!
			m_iLastLoadingUnitUID = FIRST_SENDER_UID;
		}
	}	
#endif SERV_AUTO_HACK_CHECK_STAGE_LOAD
	//}}
	*/
#endif SERV_AUTO_HACK_CHECK_STAGE_LOAD_2
	//}


	//�ε��ð� üũ�� ������..
	//�̰����� ������ �Ұ�� 2������ ��쿡 �ε��ð��� üũ�ϰ� �ȴ�.
	//::1. �����ʱ� �ε���
	//::2. �������� �ε�
	//## �帷�ε��� ����� ��.
	m_LoadingTimer.Start();

	//{{ 2010. 03. 24  ������	��д��� ����
#ifdef SERV_SECRET_HELL
	const CXSLDungeonStage::STAGE_TYPE eStageType = SiCXSLDungeonManager()->GetStageType( GetDungeonIDAndDif(), kPacket_.m_iStageID );
#endif SERV_SECRET_HELL
	//}}

	KEGS_DUNGEON_STAGE_LOAD_NOT kNot;
	kNot.m_iNumMember = m_spRoomUserManager->GetNumMember();

	//{{ 2012. 05. 10	������	���� ���� ���� ����
#ifdef SERV_DUNGEON_NPC_LEVEL
	int iRelativeMonsterLevel = GetAverageUserLevelAtDungeonStart(); // ���� ���� ���۽� ���� ��� ������ ���� ������ ���� ������ �ȴ�.
#else
	int iHighestUserLevelAtDungeonStart = GetHighestUserLevelAtDungeonStart();
#endif SERV_DUNGEON_NPC_LEVEL
	//}}	

	//{{ 2010. 03. 24  ������	��д��� ����
#ifdef SERV_SECRET_HELL
	if( m_eDungeonType == CXSLDungeon::DT_SECRET )
	{
		// ��д����� ��� ���� ���� ����
		{
			const int iDungeonMinLevel = SiCXSLDungeonManager()->GetDungeonMinLevel( GetDungeonIDAndDif() );
			if( iRelativeMonsterLevel < iDungeonMinLevel )
			{
				iRelativeMonsterLevel = iDungeonMinLevel;
			}

			const int iDungeonMaxLevel = SiCXSLDungeonManager()->GetDungeonMaxLevel( GetDungeonIDAndDif() );
			if( iRelativeMonsterLevel > iDungeonMaxLevel )
			{
				iRelativeMonsterLevel = iDungeonMaxLevel;
			}
		}

		if( m_cDungeonMode == CXSLDungeon::DM_SECRET_HELL )
		{
			// ��д��� ���� EXTRA STAGE
			if( eStageType == CXSLDungeonStage::ST_EXTRA )
			{
				// Extra Stage Load Count
				++m_iExtraStageLoadCount;

				iRelativeMonsterLevel = iRelativeMonsterLevel + 5 + SiCXSLDungeonManager()->GetSecretDungeonExtraStageNpcLevel();
			}
			// ��д��� ����
			else
			{
				iRelativeMonsterLevel = iRelativeMonsterLevel + 5;
			}
		}
	}
#endif SERV_SECRET_HELL
	//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER
	else
	{
		if( CXSLDungeon::IsDefenceDungeon( GetDungeonID() ) )
		{
			const int iDungeonMinLevel = SiCXSLDungeonManager()->GetDungeonMinLevel( GetDungeonIDAndDif() );
			if( iRelativeMonsterLevel < iDungeonMinLevel )
			{
				iRelativeMonsterLevel = iDungeonMinLevel;
			}
		}
	}
#endif SERV_INSERT_GLOBAL_SERVER
	//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
	
	// ���� �������� ����
	bool bIsBossStage = false;

	// NPC���� �ҷ�����!
	//{{ 2012. 05. 10	������	���� ���� ���� ����
#ifdef SERV_DUNGEON_NPC_LEVEL
	if( SiCXSLDungeonManager()->GetNPCData( GetDungeonIDAndDif(), kPacket_.m_iStageID, iRelativeMonsterLevel, kNot, bIsBossStage ) == false )
#else
	if( SiCXSLDungeonManager()->GetNPCData( GetDungeonIDAndDif(), kPacket_.m_iStageID, iHighestUserLevelAtDungeonStart, kNot, bIsBossStage ) == false )
#endif SERV_DUNGEON_NPC_LEVEL
	//}}
	{
		kPacket.m_iOK = NetError::ERR_ROOM_22;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_STAGE_LOAD_ACK, kPacket );
		return;
	}
	else
	{
		// �������� StageID��� �ǴܵǸ� ���� ��������ID ������ ������Ʈ ����!
		SetStageID( kPacket_.m_iStageID );

		// �������� StageID���̹Ƿ� Ŭ���̾�Ʈ�� OK�� ����.
		kPacket.m_iOK = NetError::NET_OK;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_STAGE_LOAD_ACK, kPacket );
	}
	//}}	

	//////////////////////////////////////////////////////////////////////////
	//{{ 2010. 07. 16  ������	�̺�Ʈ ���� �ý���	
#ifdef SERV_EVENT_MONSTER
	if( CXSLDungeon::IsTutorialDungeon( GetDungeonID() ) == false  &&
		CXSLDungeon::IsTCDungeon( GetDungeonID() ) == false  &&
		CXSLDungeon::IsHenirDungeon( GetDungeonID() ) == false 
		//{{ 2011. 07. 25    ��μ�    �̺�Ʈ ���� ���� ���� ����
#ifdef SERV_EVENT_MONSTER_DUNGEON_RESTRICT
		&& CXSLDungeon::IsDefenceDungeon( GetDungeonID() ) == false    // ���潺 �������� �̺�Ʈ ���Ͱ� �������� �ʵ����Ѵ�.
#endif SERV_EVENT_MONSTER_DUNGEON_RESTRICT
		//}} 
		)
	{
		//{{ 2012. 02. 03	�ڼ���	�̺�Ʈ �������� ó����� ���� ( Script -> DB )
#ifdef SERV_CHANGE_EVENT_INFO_SCRIPT_TO_DB
		BOOST_TEST_FOREACH( KEventMonsterRefreshManager::KAdventNpcEventInfo&, kInfo, m_vecEventMonster )
#else
		BOOST_TEST_FOREACH( KGameEventScriptManager::KAdventNpcEventInfo&, kInfo, m_vecEventMonster )
#endif SERV_CHANGE_EVENT_INFO_SCRIPT_TO_DB
			//}}
		{
			// ���� ������������ ���� ��ų�� üũ!
			if( kInfo.m_bIsAdventInBossStage == false  &&  bIsBossStage == true )
			{
				continue;
			}

			// ���̻� ���� ��ų ī��Ʈ�� �������� �ʴٸ� �н�!
			if( kInfo.m_iMonsterDropCount <= 0 )
			{
				continue;
			}

			// ������ �� �ִ� �������� üũ!
			if( kInfo.m_setAdventDungeon.empty() == false )
			{
				if( kInfo.m_setAdventDungeon.find( GetDungeonID() ) == kInfo.m_setAdventDungeon.end() )
				{
					continue;
				}
			}

			// ���� �ϸ� �ȵǴ� �������� üũ!
			if( kInfo.m_setNoAdventDungeon.empty() == false )
			{
				if( kInfo.m_setNoAdventDungeon.find( GetDungeonID() ) != kInfo.m_setNoAdventDungeon.end() )
				{
					continue;
				}
			}

			// ���� �ϸ� �ȵǴ� �������� ���� üũ!
			if( kInfo.m_setNoAdventStage.empty() == false )
			{
				if( kInfo.m_setNoAdventStage.find( GetStageID() ) != kInfo.m_setNoAdventStage.end() )
				{
					continue;
				}
			}

			// ������ ���͸� ��������!
			const int iAdventNpcID = kInfo.m_kAdventNpcLot.Decision();
			if( iAdventNpcID == KLottery::CASE_BLANK )
			{
				continue;
			}

			// �̺�Ʈ ���� ���� ó��!
			//{{ �̺�Ʈ ���� ���� ( �Ʊ� ����	) - ��μ�
#ifdef SERV_ALLY_EVENT_MONSTER
#ifdef SERV_ACTIVE_EVENT_MONSTER
			std::map<int, bool>::iterator mit = kInfo.m_mapMonsterActive.find( iAdventNpcID );
			bool bActive = true;

			if(mit != kInfo.m_mapMonsterActive.end())
			{
				bActive = mit->second;
			}

			if( EventDropMonster( iAdventNpcID, kInfo.m_bAllyNPC, GetStageID(), bActive, kNot ) == true )
#else
			if( EventDropMonster( iAdventNpcID, kInfo.m_bAllyNPC, GetStageID(), kNot ) == true )
#endif SERV_ACTIVE_EVENT_MONSTER
#else
			if( EventDropMonster( iAdventNpcID, GetStageID(), kNot ) == true )
#endif SERV_ALLY_EVENT_MONSTER
			//}}
			{
				// ���� ���� �����ߴٸ� ī��Ʈ �ϳ� ����!
				--kInfo.m_iMonsterDropCount;
			}
		}
	}
#endif SERV_EVENT_MONSTER
	//////////////////////////////////////////////////////////////////////////

	//{{ 2010. 11. 20	������	Ư�� �ð� ���� ����
#ifdef SERV_TIME_DROP_MONSTER_EVENT
	if( CXSLDungeon::IsTutorialDungeon( GetDungeonID() ) == false  &&
		CXSLDungeon::IsTCDungeon( GetDungeonID() ) == false  &&
		CXSLDungeon::IsHenirDungeon( GetDungeonID() ) == false )
	{
		if( m_mapTimeDropEventMonster.empty() == true )
		{
			int iTimeDropNpcEventUID = 0;
			int iTimeDropEventNpcID = 0;
			if( SiKDungeonEventManager()->CheckTimeDropEventMonster( GetDungeonIDAndDif(), GetStageID(), kNot, iTimeDropNpcEventUID, iTimeDropEventNpcID ) == true )
			{
				m_mapTimeDropEventMonster.insert( std::make_pair( iTimeDropEventNpcID, iTimeDropNpcEventUID ) );
			}
		}
	}
#endif SERV_TIME_DROP_MONSTER_EVENT
	//}}

	//{{ 2010. 03. 21  ������	��д��� ����
#ifdef SERV_SECRET_HELL
	if( m_cDungeonMode == CXSLDungeon::DM_SECRET_HELL  &&  eStageType == CXSLDungeonStage::ST_EXTRA_NPC )
	{
		// ��д��� extra ���� ���� ó��
		//{{ 2012. 02. 28	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
		SecretDungeonExtraStageMonster( GetStageID(), kPacket_.m_sPartyFever, kNot );
#else
		SecretDungeonExtraStageMonster( GetStageID(), kPacket_.m_cPartyFever, kNot );
#endif SERV_BATTLE_FIELD_SYSTEM
		//}}		
	}
#endif SERV_SECRET_HELL
	//}}

	//{{ 2010. 07. 09  ������	��ӷ� �̺�Ʈ Ȯ��
#ifdef SERV_PC_BANG_DROP_EVENT

	//{{ 2012. 03. 28	�ڼ���	��ӷ� ���� �̺�Ʈ ���� Merge �۾� ( 2012.03.20 lygan_������ )
#ifdef DROPEVENT_RENEWAL
	std::vector<char> vecUintClass;
	std::vector<char> vecUintType;
	std::vector<int> vecUintLevel;
	for( int iU = 0; iU < m_kRoomUserManager.GetNumMember(); ++iU )
	{
		KRoomUserPtr spRoomUser = m_kRoomUserManager.GetUser( iU );
		if( spRoomUser == NULL )
		{
			START_LOG( cerr, L"RoomUser�� �������� ����." )
				<< END_LOG;
			continue;
		}
		vecUintClass.push_back(spRoomUser->GetUnitClass_LUA());
		vecUintType.push_back(spRoomUser->GetUnitType());
		vecUintLevel.push_back(spRoomUser->GetLevel());
	}

	if( SiKGameEventManager()->CheckItemDropProbEvent(GetDungeonID(), vecUintClass, vecUintType, vecUintLevel, m_iItemDropEventProbCount, m_bWithPlayPcBangEvent ) == true )
#else //DROPEVENT_RENEWAL
	if( SiKGameEventManager()->CheckItemDropProbEvent( m_iItemDropEventProbCount, m_bWithPlayPcBangEvent ) == true )
#endif //DROPEVENT_RENEWAL
	{
		// PC�� ������ �Բ� �÷��� �ؾ߸� �̺�Ʈ�� ����Ǵ� ���! ��ȿ� PC�� ������ �ִ��� üũ����!
		if( m_bWithPlayPcBangEvent )
		{
			if( m_spRoomUserManager->IsExistPcBangPlayer() == false )
			{
				// ����Ʈ�� �־��ش�!
				m_iItemDropEventProbCount = REE_DROP_EVENT_PROB_DEFAULT;
				m_bWithPlayPcBangEvent = false;
			}
		}
	}
	else
	{
		m_iItemDropEventProbCount = REE_DROP_EVENT_PROB_DEFAULT;
		m_bWithPlayPcBangEvent = false;
	}

	// Not��Ŷ�� ����
	kNot.m_iItemDropEventProbCount = m_iItemDropEventProbCount;
	kNot.m_bWithPlayPcBangEvent = m_bWithPlayPcBangEvent;
#endif SERV_PC_BANG_DROP_EVENT
	//}}	

	//{{ 2013. 03. 18	 ����Ʈ ��� Ȯ�� ���� �̺�Ʈ - ��μ�
#ifdef SERV_QUEST_ITEM_DROP_EVENT
	float fQuestItemDropEventProbRate = 0.f;
	if( SiKGameEventManager()->CheckQuestItemDropProbEvent( fQuestItemDropEventProbRate ) == true )
	{
		m_fQuestItemDropEventProbRate = fQuestItemDropEventProbRate + 1.f;

		if( m_fQuestItemDropEventProbRate < REE_DROP_EVENT_PROB_DEFAULT )
			m_fQuestItemDropEventProbRate = 1.f;
	}
#endif SERV_QUEST_ITEM_DROP_EVENT
	//}

	//////////////////////////////////////////////////////////////////////////	
	//{{ 2010. 03. 24  ������	��д��� ����
	//#ifdef SERV_SECRET_HELL
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// NPC UID �߱� ó��!

	//{{ 2009. 5. 13  ������	���꽺�������� drop npc��
	m_mapSubStageExpNpcNum.clear();
	//}}

	//{{ 2011. 09. 07  ��μ�	���� �������� DIE NPC �ð� üũ ���
#ifdef SERV_SUB_STAGE_NPC_DIE_CHECK
	m_mapSubStageActiveNpcNum.clear();
#endif SERV_SUB_STAGE_NPC_DIE_CHECK
	//}}

	std::map< int, KNPCList >::iterator mit;
	for( mit = kNot.m_mapNPCData.begin(); mit != kNot.m_mapNPCData.end(); ++mit )
	{
		const int iSubStageID = mit->first;
		KNPCList& kNPCList = mit->second;

		//////////////////////////////////////////////////////////////////////////	
		//{{ 2010. 7. 28  ������	������ ���� NPC
#ifdef SERV_AUTO_HACK_CHECK_NPC
		if( kNPCList.m_NPCList.empty() == false )
		{
			KNPCUnitReq kNPCData;
			kNPCData.Init();

			//NPC DATA SET..
			kNPCData.m_NPCID			= CXSLUnitManager::NUI_DUNGEON_CHECKER1 + ( rand() % 5 );
			kNPCData.m_KeyCode			= -1;
			kNPCData.m_bFocusCamera		= false;
			kNPCData.m_nStartPos		= SiCXSLDungeonManager()->GetStartPosByRandom( GetDungeonIDAndDif(), GetStageID(), iSubStageID );
			kNPCData.m_bActive			= false;
			kNPCData.m_bShowGage		= false;
			kNPCData.m_Level			= SiKGameSysVal()->GetLimitsLevel();
			kNPCData.m_bNoDrop			= true;

			if( kNPCData.m_nStartPos != -1 )
			{
				kNPCList.m_NPCList.push_back( kNPCData );
			}
			else
			{
				// ���� ��ġ�� -1�̸� �̹� ���� ������ NPC�� ��ġ�� ����!
				BOOST_TEST_FOREACH( const KNPCUnitReq&, kNpc, kNPCList.m_NPCList )
				{
					kNPCData.m_nStartPos = kNpc.m_nStartPos;
					kNPCList.m_NPCList.push_back( kNPCData );
					break;
				}
			}
		}
#endif SERV_AUTO_HACK_CHECK_NPC
		//}}
		//////////////////////////////////////////////////////////////////////////

		
		int iExpNpcNum = 0;

		//{{ 2011. 09. 07  ��μ�	���� �������� DIE NPC �ð� üũ ���
#ifdef SERV_SUB_STAGE_NPC_DIE_CHECK
		int iSubStageActiveNpc = 0;
#endif SERV_SUB_STAGE_NPC_DIE_CHECK
		//}}

		for( int i = 0; i < (int)kNPCList.m_NPCList.size(); ++i )
		{
			const KNPCUnitReq& kNpcInfo = kNPCList.m_NPCList[i];

			// ���� ����!
			LIF( m_kDungeonMonsterManager.CreateMonster( kNpcInfo, kNPCList.m_NPCList[i].m_UID ) );
			const int iNpcUID = kNPCList.m_NPCList[i].m_UID;

			//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER
			if( kNPCList.m_NPCList[i].m_NPCID == CXSLUnitManager::NUI_DEFENCE_CRYSTAL )
			{
				if( m_iDefenseMainCrystalNpcUID != 0 )
				{
					START_LOG( cerr, L"ū ������ 1 ���� ����? ����� �Ͼ���� �ȵǴ� ����!" )
						<< BUILD_LOG( m_iDefenseMainCrystalNpcUID )
						<< BUILD_LOG( kNPCList.m_NPCList[i].m_NPCID )
						<< END_LOG;
				}
				else
				{
					m_iDefenseMainCrystalNpcUID = iNpcUID;
				}
			}
			if( kNPCList.m_NPCList[i].m_NPCID == CXSLUnitManager::NUI_DEFENCE_CRYSTAL_SMALL )
			{
				if( m_setDefenseSmallCrystalNpcUID.size() > MAX_SMALL_CRYSTAL )
				{
					START_LOG( cerr, L"�� ������ 4 ���� ����? ����� �Ͼ���� �ȵǴ� ����!" )
						<< BUILD_LOG( m_setDefenseSmallCrystalNpcUID.size() )
						<< END_LOG;
				}
				else
				{
					m_setDefenseSmallCrystalNpcUID.insert( iNpcUID );
				}
			}
#endif SERV_INSERT_GLOBAL_SERVER
			//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�

			//////////////////////////////////////////////////////////////////////////			
			//{{ 2009. 5. 13  ������	���꽺�������� ����ġ �ִ� npc��
			if( m_eDungeonType == CXSLDungeon::DT_HENIR )
			{
				if( SiKDropTable()->IsHenirExpDropNpc( static_cast<CXSLDungeon::DUNGEON_MODE>(m_cDungeonMode), kNpcInfo.m_NPCID ) == true  /*&&  !npcData.m_bNoDrop*/ )
				{
					iExpNpcNum++;
				}
				//{{ 2011. 09. 07  ��μ�	���� �������� DIE NPC �ð� üũ ���
#ifdef SERV_SUB_STAGE_NPC_DIE_CHECK
				if( kNpcInfo.m_bActive == true )
				{
					iSubStageActiveNpc++;
				}
#endif SERV_SUB_STAGE_NPC_DIE_CHECK
				//}}
			}
			else
			{
				if( SiKDropTable()->IsExpDropNpc( m_iDungeonID + static_cast<int>(m_cDifficultyLevel), kNpcInfo.m_NPCID ) == true /*&&  !npcData.m_bNoDrop*/ )
				{
					iExpNpcNum++;
				}
				//{{ 2011. 09. 07  ��μ�	���� �������� DIE NPC �ð� üũ ���
#ifdef SERV_SUB_STAGE_NPC_DIE_CHECK
				if( kNpcInfo.m_bActive == true )
				{
					iSubStageActiveNpc++;
				}
#endif SERV_SUB_STAGE_NPC_DIE_CHECK
				//}}
			}
			//}}
			//////////////////////////////////////////////////////////////////////////
		}

		// ��ŷ üũ�� ���� ���� �������� NPC������ �������!
		m_kDungeonMonsterManager.InsertSubStageNpcList( GetStageID(), iSubStageID, kNPCList.m_NPCList );

		// drop npc��
		m_mapSubStageExpNpcNum.insert( std::make_pair( iSubStageID, iExpNpcNum ) );

		//{{ 2011. 09. 07  ��μ�	���� �������� DIE NPC �ð� üũ ���
#ifdef SERV_SUB_STAGE_NPC_DIE_CHECK
		m_mapSubStageActiveNpcNum.insert( std::make_pair( iSubStageID, iExpNpcNum ) );
#endif SERV_SUB_STAGE_NPC_DIE_CHECK
		//}}
	}

	//////////////////////////////////////////////////////////////////////////
	// �Ӽ�NPC ���� - NPC UID�߱ް��� ���� �����ϴ�.
	if( eStageType == CXSLDungeonStage::ST_EXTRA )
	{
		// ù��° Extra Stage�����ΰ�?
		if( m_iExtraStageLoadCount == 1 )
		{
			// ���׶� ����� ��쿡�� �÷��� �ð� �ʱ�ȭ!
			if( CXSLDungeon::IsAlteraDungeon( GetDungeonIDAndDif() ) == true )
			{
				// ���� �ð� �ʱ�ȭ
				SetRemainingPlayTime( GetPlayTimeLimit() );
				BroadCast( ERM_REMAINING_PLAY_TIME_NOT, GetRemainingPlayTime() );

				// EXTRA STAGE ���� ������� Ÿ���� ����!
				m_fPoisonTimeLimit = SiCXSLDungeonManager()->GetExtraStagePoisonTimeLimit( GetDungeonIDAndDif() );

				KEGS_DUNGEON_EFFECT_REMAINING_TIME_NOT kNot;
				kNot.m_fRemainingTime = m_fPoisonTimeLimit;
				BroadCast( ERM_DUNGEON_EFFECT_REMAINING_TIME_NOT, kNot );
			}
		}

		SecretDungeonExtraStageAttribMonsterDrop( kNot );
	}
	else
	{
		NormalDungeonAttribMonsterDrop( kNot );
	}

	//////////////////////////////////////////////////////////////////////////	
	//#endif SERV_SECRET_HELL
	//}}
	//////////////////////////////////////////////////////////////////////////

	BroadCast( ERM_DUNGEON_STAGE_LOAD_NOT, kNot );

	//{{ 2012. 12. 03 ���� �������� �ߺ� �ε� ���� Ȯ�� - ��μ�
#ifdef SERV_AUTO_HACK_CHECK_STAGE_LOAD_2
	// DUNGEON_STAGE_LOAD_NOT �� �������� ����Ѵ�.
	{
		// �ε����� ���� StageID�� �����Ѵ�.
		m_mapStageLoadingCheck.insert( std::make_pair( kPacket_.m_iStageID, 1 ) );

		// ���������� �ε� ������ ������ UnitUID�� ����!
		m_iLastLoadingUnitUID = FIRST_SENDER_UID;
	}
#endif SERV_AUTO_HACK_CHECK_STAGE_LOAD_2
	//}
}

//////////////////////////////////////////////////////////////////////////
#else
//////////////////////////////////////////////////////////////////////////

//IMPL_ON_FUNC( ERM_LEAVE_ROOM_REQ )
//{
//	// �ǵ����� Leave Room ���� ������ ������ �� �� �� �� Leave Room �ϰ� �Ǵ� ��찡 �ִ�.
//	KEGS_LEAVE_ROOM_ACK kPacket;
//	if( GetStateID() == KRoomFSM::S_INIT || GetStateID() == KRoomFSM::S_CLOSE )
//	{
//		kPacket.m_iOK = NetError::ERR_ROOM_09;
//		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_LEAVE_ROOM_ACK, kPacket );
//		return;
//	}
//
//	// ��Ż�� ���.
//	if( GetStateID() == KRoomFSM::S_LOAD || GetStateID() == KRoomFSM::S_PLAY )
//	{
//
//		//=== ������ ��Ż ���ó�� ===
//		START_LOG( clog, L"=== DUNGEON GAME : ������ ��Ż�� ���ó�� ===" )
//			<< BUILD_LOG( FIRST_SENDER_UID )
//			<< END_LOG;
//
//		KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser( FIRST_SENDER_UID );
//
//		if( !spRoomUser )
//		{
//			spRoomUser = m_spRoomUserManager->GetUser( FIRST_SENDER_UID, KRoomUserManager::UT_OBSERVER );
//		}
//
//		if( !spRoomUser )
//		{
//			START_LOG( cerr, L"�� ���� ������ �̻�" )
//				<< BUILD_LOG( FIRST_SENDER_UID )
//				<< END_LOG;
//		}
//		else
//		{
//			if( spRoomUser->IsObserver() == false )
//			{
//				//Ʃ�丮�� ����̸� �г�Ƽ�� ����..
//				int iEXP = 0;
//				int iED = 0;
//				if( CXSLDungeon::IsTutorialDungeon( m_iDungeonID ) == false )
//				{
//					KERM_UPDATE_DUNGEON_UNIT_INFO_NOT kPacket;
//					kPacket.m_bOutRoom = true;
//					kPacket.m_iUserCount = GetNumMember();
//					kPacket.m_iDungeonID = m_iDungeonID + static_cast<int>(m_cDifficultyLevel);
//					kPacket.m_iPlayTime = ( int )( m_kTimer[TM_PLAY].elapsed() + 0.5 - GetLoadingTime() );
//					kPacket.m_iEXP = 0;
//
//					if( spRoomUser->IsDie() == true )
//					{
//						// �ǽð� ����ġ
//						m_spRoomUserManager->GetRewardEXP( FIRST_SENDER_UID, iEXP );
//						m_spRoomUserManager->GetRewardPartyEXP( FIRST_SENDER_UID, iEXP );
//
//						//���κ� ������ �����ֱ�
//						m_spRoomUserManager->GetItemList( FIRST_SENDER_UID, kPacket.m_mapGetItem );
//					}
//					else
//					{
//						// ED���Ƽ
//						SiKResultProcess()->Result_DUNGEON_BREAK( spRoomUser, kPacket );
//					}
//
//					// [�߿�] Ŭ���̾�Ʈ ũ���� �� ��ŷ���� ���� �� ��Ż�� ��� ���Ӽ����� ������Ŷ�� ������ �ʽ��ϴ�!
//					if( kPacket_.m_iReason != NetError::NOT_LEAVE_ROOM_REASON_23 )
//					{
//						SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_UPDATE_DUNGEON_UNIT_INFO_NOT, kPacket );
//
//						//{{ 2009. 5. 14  ������	������ ����
//						KERM_DECREASE_ENDURANCE_DUNGEON_NOT kNot;
//						kNot.m_iEnduranceDamage = CXSLDungeon::DE_LEAVE_ROOM_ENDURANCE_DAMAGE;
//						kNot.m_cPartyNumMember  = kPacket.m_iUserCount;
//						SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DECREASE_ENDURANCE_DUNGEON_NOT, kNot );
//						//}}
//					}
//
//					iED = kPacket.m_iED;
//				}
//
//				//////////////////////////////////////////////////////////////////////////
//				// ���� ���� ��Ż �α�
//				// ��ڴ� ��踦 ������ �ʽ��ϴ�.
//				if( spRoomUser->GetAuthLevel() >= SEnum::UAL_GM )
//				{
//					START_LOG( cout, L"��ڴ� ��赥���͸� ������ �ʽ��ϴ�." )
//						<< BUILD_LOGc( spRoomUser->GetAuthLevel() )
//						<< BUILD_LOG( spRoomUser->GetNickName() );
//				}
//				else
//				{
//					KRoomUserInfo kInfo;
//					spRoomUser->GetRoomUserInfo( kInfo );
//
//					int iClear = ( spRoomUser->IsDie() ? KE_LOCAL_LOG_DUNGEON_NOT::DCT_UNIT_DIE_LEAVE_ROOM : KE_LOCAL_LOG_DUNGEON_NOT::DCT_UNIT_ALIVE_LEAVE_ROOM ); // 2. ĳ���� �װ� ��Ż   3. ĳ���� ���װ� ��Ż	
//
//					if( kPacket_.m_iReason == NetError::NOT_LEAVE_ROOM_REASON_23 ) // 4. Ŭ���̾�Ʈ ũ���� ��Ż
//					{
//						iClear = KE_LOCAL_LOG_DUNGEON_NOT::DCT_CLIENT_CRASH_LEAVE_ROOM;
//					}
//					else if( kPacket_.m_iReason == NetError::NOT_LEAVE_ROOM_REASON_25 ) // 5. Ŭ���̾�Ʈ ��ŷ���� ���� ��Ż
//					{
//						iClear = KE_LOCAL_LOG_DUNGEON_NOT::DCT_CLIENT_HACKING_LEAVE_ROOM;
//					}
//
//					//{{ 2010. 06. 21  ������	��� ������
////#ifdef SERV_STATISTICS_THREAD
//					//{{ 2010. 11. 8	������	���� �α� DB
////#ifdef SERV_DUNGEON_LOG_DB
//					CTime kRegDate = CTime::GetCurrentTime();
////#endif SERV_DUNGEON_LOG_DB
//					//}}
//					//////////////////////////////////////////////////////////////////////////					
//					KE_LOCAL_LOG_DUNGEON_NOT kNot;
//					kNot.m_iDungeonID				= (m_iDungeonID + static_cast<int>(m_cDifficultyLevel));
//					kNot.m_nUnitUID					= kInfo.m_nUnitUID;
//					kNot.m_wstrNickName				= kInfo.m_wstrNickName;
//					kNot.m_iDifficultyLevel			= (int) m_cDifficultyLevel;
//					kNot.m_iIsChallenge				= ( m_cDungeonMode == CXSLDungeon::DM_HENIR_CHALLENGE ? 1 : 0 );
//					kNot.m_iUnitClass				= (int) kInfo.m_cUnitClass;
//					kNot.m_iClear					= iClear;
//					kNot.m_iStartNumMember			= m_iStartedNumMember;
//					kNot.m_iEndNumMember			= GetNumMember();
//					kNot.m_ucLevel					= kInfo.m_ucLevel;
//					kNot.m_iPlayTime				= (int) spRoomUser->GetPlayTime();
//					kNot.m_iEXP						= iEXP;
//					kNot.m_iED						= iED;
//					kNot.m_cComboRank				= CXSLDungeon::RT_F;
//					kNot.m_iComboScore				= spRoomUser->GetComboScore_LUA();
//					kNot.m_cTechnicalRank			= CXSLDungeon::RT_F;
//					kNot.m_iTechnicalScore			= spRoomUser->GetTechnicalScore_LUA();
//					kNot.m_cTimeRank				= CXSLDungeon::RT_F;
//					kNot.m_cDamagedRank				= CXSLDungeon::RT_F;
//					kNot.m_iDamageNum				= spRoomUser->GetDamageNum_LUA();
//					kNot.m_cTotalRank				= CXSLDungeon::RT_F;
//					kNot.m_iRessurectionStoneCount	= spRoomUser->GetRessurectionStoneCount();
//					kNot.m_iPassedStageCount		= spRoomUser->GetPassedStageCount();
//					kNot.m_iPassedSubStageCount		= spRoomUser->GetPassedSubStageCount();
//					//{{ 2010. 9. 14	������	�� �ý���
//#ifdef SERV_PET_SYSTEM
//					kNot.m_bIsWithPet				= spRoomUser->HavePet();
//#endif SERV_PET_SYSTEM
//					//}}
//					//{{ 2010. 11. 8	������	���� �α� DB
//#ifdef SERV_DUNGEON_LOG_DB
//					kNot.m_wstrRegDate				= (const wchar_t*)kRegDate.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
//#endif SERV_DUNGEON_LOG_DB
//					//}}
//					//{{ 2011. 03. 16	������	���� �α� �÷� �߰�
//#ifdef SERV_ADD_DUNGEON_LOG_COLUMN
//					kNot.m_sNpcDieCount				= spRoomUser->GetKillNPC_LUA();
//					kNot.m_iGivenDamage				= 0;
//					kNot.m_iAttackDamage			= 0;
//					kNot.m_sUsingSkillKind			= 0;
//					kNot.m_iTotalEXP				= ( spRoomUser->GetRewardEXP() + spRoomUser->GetRewardPartyEXP() );
//					kNot.m_cPetID					= spRoomUser->GetPetID();
//					kNot.m_cPetEvoStep				= spRoomUser->GetPetEvolutionStep();
//#endif SERV_ADD_DUNGEON_LOG_COLUMN
//					//}}
//					//{{ 2012. 04. 17	������	���� �α� �÷� �߰�
//#ifdef SERV_ADD_DUNGEON_LOG_COLUMN_NUM_2
//					kNot.m_iBaseHP					= spRoomUser->GetBaseHP_LUA();
//					kNot.m_iEndHP					= spRoomUser->GetLastUpdatedHP();
//					kNot.m_iRecovHP					= spRoomUser->GetTotalRecoveryHP_LUA();
//					kNot.m_iContinue				= spRoomUser->GetUsedRessurectionStoneCount();
//					kNot.m_iStartContinue			= spRoomUser->GetStartTimeNumResurrectionStone();
//					kNot.m_iContinueUseStage		= spRoomUser->GetFirstUseResurrectionStoneStageID();
//					kNot.m_iContinueUseSub			= spRoomUser->GetFirstUseResurrectionStoneSubStageID();
//					kNot.m_iFrame					= spRoomUser->GetInGameAverageFrame();
//					kNot.m_wstrChannelIP			= spRoomUser->GetGameChannelIP();
//#endif SERV_ADD_DUNGEON_LOG_COLUMN_NUM_2
//					//}}
//					//{{ 2012. 07. 11 ��μ�
//#ifdef SERV_ADD_DUNGEON_LOG_COLUMN_NUM_3
//					kNot.m_iUserUID					= spRoomUser->GetUserUID();
//#endif SERV_ADD_DUNGEON_LOG_COLUMN_NUM_3
//					//}}
//					//{{ 2012. 11. 6	������	��Ʋ�ʵ� �ý���
//#ifdef SERV_BATTLE_FIELD_SYSTEM
//					kNot.m_cAutoPartyPlay			= ( IsStartedByAutoParty() ? 1 : 0 );
//#endif SERV_BATTLE_FIELD_SYSTEM
//					//}}
//					KSIManager.QueueingEvent( E_LOCAL_LOG_DUNGEON_NOT, kNot );
//					//////////////////////////////////////////////////////////////////////////
//					//{{ 2010. 11. 8	������	���� �α� DB
////#ifdef SERV_DUNGEON_LOG_DB
//					SendToLogDB( DBE_DB_LOG_DUNGEON_NOT, kNot );
////#endif SERV_DUNGEON_LOG_DB
//					//}}
////#endif SERV_STATISTICS_THREAD
//					//}}
//				}
//				//////////////////////////////////////////////////////////////////////////
//
//				//{{ 2007. 12. 18  ������  ���� ��� [����]
//				KERM_DUNGEON_USER_STATISTICS_NOT kPacketNot;
//				kPacketNot.m_iDungeonID	   = m_iDungeonID + static_cast<int>(m_cDifficultyLevel);
//				kPacketNot.m_cGameResult   = KERM_DUNGEON_USER_STATISTICS_NOT::UGR_DROP;
//				kPacketNot.m_bIsPartyPlay  = ( m_iStartedNumMember > 1 );
//				kPacketNot.m_iGamePlayTime = ( int )( m_kTimer[TM_PLAY].elapsed() + 0.5 - GetLoadingTime() );
//				kPacketNot.m_iUsedResurrectionStone = m_spRoomUserManager->GetRessurectionStoneCount( FIRST_SENDER_UID );
//				SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_USER_STATISTICS_NOT, kPacketNot );
//				//}}
//
//				//{{ 2007. 12. 25  ������  ��Ȱ�� ���
//				if( m_spRoomUserManager->GetRessurectionStoneCount( FIRST_SENDER_UID ) > 0 )
//				{
//					KStatisticsKey kKey;
//					kKey.m_vecIntKey.push_back( 0 );
//					KSIManager.IncreaseCount( KStatistics::SI_RES_STONE, kKey, KStatistics::eSIColResStone_PlayerCount, 1 );
//				}
//				//}}
//			}
//		}
//	}
//
//	kPacket.m_iOK = NetError::NET_OK;
//
//	//{{ 2011. 02. 21  ��μ�	Ư�� ���� ���� ������ ���� ����
//	//#ifdef SERV_DUNGEON_REQUIRED_ITEM_BUG
//	//�������� ����������� ������� ó��..
//	if( kPacket_.m_iReason == NetError::NOT_LEAVE_ROOM_REASON_22 )
//	{
//		kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_22;
//		kPacket.m_iItemID = kPacket_.m_iItemID;
//	}
//	//{{ 2011. 11. 1	������	��Ʋ�ʵ� �ý���
//#ifdef SERV_BATTLE_FIELD_SYSTEM
//	else if( kPacket_.m_iReason == NetError::NOT_LEAVE_ROOM_REASON_27 )
//	{
//		kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_27;
//		kPacket.m_iBattleFieldID = kPacket_.m_iBattleFieldID;
//	}
//#endif SERV_BATTLE_FIELD_SYSTEM
//	//}}
//	else
//	{
//		kPacket.m_iReason = NetError::NET_OK;
//		kPacket.m_iItemID = 0;
//	}
//	//#endif SERV_DUNGEON_REQUIRED_ITEM_BUG
//	//}}
//
//	// ���� ���� ����.
//	if( !m_spRoomUserManager->LeaveRoom( FIRST_SENDER_UID ) )
//	{
//		// ���Կ��� ������ ����.
//		kPacket.m_iOK = NetError::ERR_SLOT_04;
//		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_LEAVE_ROOM_ACK, kPacket );
//		return;
//	}
//	else
//	{
//		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_LEAVE_ROOM_ACK, kPacket );
//	}
//
//	if( IsEmpty() == false || IsEmpty( KRoomUserManager::UT_OBSERVER ) == false )
//	{
//		// ���� ��� �����鿡�� ���� ������ ������.
//		KEGS_LEAVE_ROOM_NOT kPacketNot;
//		kPacketNot.m_cRoomState = GetStateID();
//		kPacketNot.m_iUnitUID = FIRST_SENDER_UID;
//		GetRoomSlotInfo( kPacketNot.m_vecSlot );
//		GetRoomSlotInfo( kPacketNot.m_vecObserverSlot, KRoomUserManager::UT_OBSERVER );
//		BroadCast( ERM_LEAVE_ROOM_NOT, kPacketNot );
//	}
//
//	if( (GetStateID() == KRoomFSM::S_LOAD || GetStateID() == KRoomFSM::S_PLAY) && IsEmpty() == false )
//	{
//		//{{ 2010. 11. 23	������	�� �ε� ���� ���� - ũ���� ������ �־ ������ �����մϴ�.
//#ifdef SERV_ROOM_LOADING_BUG_FIX
//		//////////////////////////////////////////////////////////////////////////
//		if( GetStateID() == KRoomFSM::S_LOAD )
//		{
//			if( m_spRoomUserManager->IsAllPlayerFinishLoading() == true )
//			{
//				BroadCastID( ERM_GAME_LOADING_ALL_UNIT_OK_NOT );
//
//				// ���������� �ʱ�ȭ.
//				StartPlay();
//
//				KEGS_PLAY_START_NOT kNot;
//				//{{ 2011. 01. 17	������	ĳ���� ī��Ʈ ����
//#ifdef SERV_CHAR_LOG
//				kNot.m_iDungeonID = GetDungeonIDAndDif();
//#endif SERV_CHAR_LOG
//				//}}
//				kNot.m_RoomState = GetStateID();
//				GetRoomSlotInfo( kNot.m_vecSlot );
//
//				// ��� �������� �÷��� ������ �˸�.
//				BroadCast( ERM_PLAY_START_NOT, kNot );
//
//				// ��� ���� ������ �� ����Ʈ�� �����϶�� ������.
//				SendRoomListInfo( NetError::ERR_ROOM_15 );
//
//				//{{ 2009. 4. 23  ������	������������Ʈ
//				SendAllPVPUserInfo();
//				//}}
//			}
//		}
//		else if( GetStateID() == KRoomFSM::S_PLAY )
//		{
//			CheckLoadingSuccess();
//		}
//		//////////////////////////////////////////////////////////////////////////	
//#else
//		//////////////////////////////////////////////////////////////////////////
//		CheckLoadingSuccess();
//		//////////////////////////////////////////////////////////////////////////
//#endif SERV_ROOM_LOADING_BUG_FIX
//		//}}
//	}
//
//	// ���������� ������ �Ǵ� ��� ó��.
//	if( GetStateID() == KRoomFSM::S_PLAY && IsEmpty() == false )
//	{
//		if( CheckIfPlayEnd() )
//		{
//			EndPlay();
//		}
//		else
//		{
//			//{{ 2009. 7. 27  ������	��н�������
//			if( m_eDungeonType == CXSLDungeon::DT_HENIR )
//			{
//				CheckEnterSecretStageSuccess();
//			}
//			//}}
//		}
//	}
//
//	//���â�� ���������� ���� �������ó��
//	if( GetStateID() == KRoomFSM::S_RESULT && IsEmpty() == false )
//	{
//		CheckResultSuccess();
//	}
//
//	// �濡�� ��� ������ �������� ���� �ݴ´�.
//	if( IsEmpty() == true && IsEmpty( KRoomUserManager::UT_OBSERVER ) == true )
//	{
//		StateTransition( KRoomFSM::I_TO_CLOSE );
//
//		//{{ 2010. 06. 21  ������	��� ������
//#ifdef SERV_STATISTICS_THREAD
//		CTime kEndGameTime = CTime::GetCurrentTime();
//		//////////////////////////////////////////////////////////////////////////
//		KE_LOCAL_LOG_DUNGEON_ROOM_NOT kNot;
//		kNot.m_iDungeonID		= m_iDungeonID + static_cast<int>(m_cDifficultyLevel);
//		kNot.m_iDifficultyLevel	= (int) GetDifficultyLevel();
//		kNot.m_iIsChallenge		= ( m_cDungeonMode == CXSLDungeon::DM_HENIR_CHALLENGE ? 1 : 0 );
//		kNot.m_iWin				= 0;
//		kNot.m_iLose			= 0;
//		kNot.m_iStartNumMember	= m_iStartedNumMember;
//		kNot.m_iEndNumMember	= GetNumMember();
//		kNot.m_iTotalPlayTime	= static_cast<int>(GetTotalPlayTime());
//		kNot.m_wstrEndGameTime	= (const wchar_t*)kEndGameTime.Format(_T("%H:%M:%S"));
//		KSIManager.QueueingEvent( E_LOCAL_LOG_DUNGEON_ROOM_NOT, kNot );
//		//////////////////////////////////////////////////////////////////////////
//#endif SERV_STATISTICS_THREAD
//		//}}
//	}
//	else
//	{
//		if( m_kDSSGoNextAllNot.m_iBeforeStageID != -1 &&
//			m_kDSSGoNextAllNot.m_iNextStageID	!= -1 )
//			SendDSSGoNextAllNot( m_kDSSGoNextAllNot );
//	}
//}

//_IMPL_ON_FUNC( ERM_DUNGEON_STAGE_LOAD_REQ, KEGS_DUNGEON_STAGE_LOAD_REQ )
//{
//	//////////////////////////////////////////////////////////////////////////
//	// ���� ���� ���̽�
//	// 1. S_RESULT : 2���̻� ��Ƽ���� �Ѹ� �������¿��� �ٸ� �Ѹ��� �������� Ŭ����� ���ÿ� ������Ż�� �ϸ� �߻�
//	//////////////////////////////////////////////////////////////////////////
//
//	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_PLAY ), ERM_DUNGEON_STAGE_LOAD_ACK, KEGS_DUNGEON_STAGE_LOAD_ACK );
//
//	//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
//#ifdef SERV_INSERT_GLOBAL_SERVER
//	// ���潺 ������ �ʱ�ȭ
//	m_iDefenseMainCrystalNpcUID = 0;
//	m_setDefenseSmallCrystalNpcUID.clear();
//#endif SERV_INSERT_GLOBAL_SERVER
//	//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
//
//	//{{ 2010. 11. 18	������	���� �� �������� �ε� üũ
//#ifdef SERV_AUTO_HACK_CHECK_STAGE_LOAD
//	{
//		std::map< int, int >::iterator mitSLC = m_mapStageLoadingCheck.find( kPacket_.m_iStageID );
//		if( mitSLC != m_mapStageLoadingCheck.end() )
//		{
//			START_LOG( cout, L"�Ȱ��� �������� �� �ε��߳�? �̰� ����������?" )
//				<< BUILD_LOG( LAST_SENDER_UID )
//				<< BUILD_LOG( FIRST_SENDER_UID )
//				<< BUILD_LOG( GetDungeonIDAndDif() )
//				<< BUILD_LOG( GetStageID() )
//				<< BUILD_LOG( GetSubStageID() )
//				<< BUILD_LOG( m_mapStageLoadingCheck.size() )
//				<< BUILD_LOG( mitSLC->second )
//				<< BUILD_LOG( kPacket_.m_iStageID )
//				<< BUILD_LOGc( kPacket_.m_cPartyFever )
//				<< BUILD_LOG( m_spRoomUserManager->GetNumMember() );
//
//			if( m_iLastLoadingUnitUID != FIRST_SENDER_UID )
//			{
//				START_LOG( cout, L"������ �ٲ����? �� �� �������� �ε��� ������ �ٸ� ������.." )					
//					<< BUILD_LOG( FIRST_SENDER_UID )
//					<< BUILD_LOG( m_iLastLoadingUnitUID );
//			}
//
//			// ���� �������� �ε� ī��Ʈ�� 1 �ø���!
//			++mitSLC->second;
//
//			// ���������� �ε� ������ ������ UnitUID�� ����!
//			m_iLastLoadingUnitUID = FIRST_SENDER_UID;
//
//			// 5�� �̻� �Ȱ��� �������� �ε��ϴ� ����� ��ŷ������ �Ǻ�!
//			if( mitSLC->second >= 5 )
//			{
//				START_LOG( cout, L"Stage Loading ��ŷ������ �ǴܵǾ� üũ ����� Ŭ���̾�Ʈ �����Ͽ���!" )
//					<< BUILD_LOG( LAST_SENDER_UID )
//					<< BUILD_LOG( FIRST_SENDER_UID )
//					<< BUILD_LOG( GetDungeonIDAndDif() )
//					<< BUILD_LOG( GetStageID() )
//					<< BUILD_LOG( m_mapStageLoadingCheck.size() )
//					<< BUILD_LOG( mitSLC->second )
//					<< BUILD_LOG( kPacket_.m_iStageID );
//
//				kPacket.m_iOK = NetError::ERR_ROOM_22;
//				SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_STAGE_LOAD_ACK, kPacket );
//
//				// ��Ƽ�忡�� ��ŷüũ��Ŷ�� ������!
//				KEGS_SERVER_CHECK_HACK_USER_NOT kNot;
//				kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_DUNGEON_STAGE_LOADING;
//				SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SERVER_CHECK_HACK_USER_NOT, kNot );
//				return;
//			}
//		}
//		else
//		{
//			// �ε����� ���� StageID�� �����Ѵ�.
//			m_mapStageLoadingCheck.insert( std::make_pair( kPacket_.m_iStageID, 1 ) );
//
//			// ���������� �ε� ������ ������ UnitUID�� ����!
//			m_iLastLoadingUnitUID = FIRST_SENDER_UID;
//		}
//	}	
//#endif SERV_AUTO_HACK_CHECK_STAGE_LOAD
//	//}}
//
//	//�ε��ð� üũ�� ������..
//	//�̰����� ������ �Ұ�� 2������ ��쿡 �ε��ð��� üũ�ϰ� �ȴ�.
//	//::1. �����ʱ� �ε���
//	//::2. �������� �ε�
//	//## �帷�ε��� ����� ��.
//	m_LoadingTimer.Start();
//
//	//{{ 2010. 03. 24  ������	��д��� ����
//#ifdef SERV_SECRET_HELL
//	const CXSLDungeonStage::STAGE_TYPE eStageType = SiCXSLDungeonManager()->GetStageType( GetDungeonIDAndDif(), kPacket_.m_iStageID );
//#endif SERV_SECRET_HELL
//	//}}
//
//	KEGS_DUNGEON_STAGE_LOAD_NOT kNot;
//	kNot.m_iNumMember = m_spRoomUserManager->GetNumMember();
//
//	int iHighestUserLevelAtDungeonStart = m_iHighestUserLevelAtDungeonStart;
//
//	//{{ 2010. 03. 24  ������	��д��� ����
//#ifdef SERV_SECRET_HELL
//	if( m_eDungeonType == CXSLDungeon::DT_SECRET )
//	{
//		// ��д����� ��� ���� ���� ����
//		{
//			const int iDungeonMinLevel = SiCXSLDungeonManager()->GetDungeonMinLevel( GetDungeonIDAndDif() );
//			if( iHighestUserLevelAtDungeonStart < iDungeonMinLevel )
//			{
//				iHighestUserLevelAtDungeonStart = iDungeonMinLevel;
//			}
//
//			const int iDungeonMaxLevel = SiCXSLDungeonManager()->GetDungeonMaxLevel( GetDungeonIDAndDif() );
//			if( iHighestUserLevelAtDungeonStart > iDungeonMaxLevel )
//			{
//				iHighestUserLevelAtDungeonStart = iDungeonMaxLevel;
//			}
//		}
//
//		if( m_cDungeonMode == CXSLDungeon::DM_SECRET_HELL )
//		{
//			// ��д��� ���� EXTRA STAGE
//			if( eStageType == CXSLDungeonStage::ST_EXTRA )
//			{
//				// Extra Stage Load Count
//				++m_iExtraStageLoadCount;
//
//				iHighestUserLevelAtDungeonStart = iHighestUserLevelAtDungeonStart + 5 + SiCXSLDungeonManager()->GetSecretDungeonExtraStageNpcLevel();
//			}
//			// ��д��� ����
//			else
//			{
//				iHighestUserLevelAtDungeonStart = iHighestUserLevelAtDungeonStart + 5;
//			}
//		}
//	}
//#endif SERV_SECRET_HELL
//	//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
//#ifdef SERV_INSERT_GLOBAL_SERVER
//	else
//	{
//		if( CXSLDungeon::IsDefenceDungeon( GetDungeonID() ) )
//		{
//			const int iDungeonMinLevel = SiCXSLDungeonManager()->GetDungeonMinLevel( GetDungeonIDAndDif() );
//			if( iHighestUserLevelAtDungeonStart < iDungeonMinLevel )
//			{
//				iHighestUserLevelAtDungeonStart = iDungeonMinLevel;
//			}
//		}
//	}
//#endif SERV_INSERT_GLOBAL_SERVER
//	//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
//
//	// ���� �������� ����
//	bool bIsBossStage = false;
//
//	// NPC���� �ҷ�����!
//	if( SiCXSLDungeonManager()->GetNPCData( GetDungeonIDAndDif(), kPacket_.m_iStageID, iHighestUserLevelAtDungeonStart, kNot, bIsBossStage ) == false )
//	{
//		kPacket.m_iOK = NetError::ERR_ROOM_22;
//		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_STAGE_LOAD_ACK, kPacket );
//		return;
//	}
//	else
//	{
//		// �������� StageID��� �ǴܵǸ� ���� ��������ID ������ ������Ʈ ����!
//		SetStageID( kPacket_.m_iStageID );
//
//		// �������� StageID���̹Ƿ� Ŭ���̾�Ʈ�� OK�� ����.
//		kPacket.m_iOK = NetError::NET_OK;
//		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_STAGE_LOAD_ACK, kPacket );
//	}
//	//}}	
//
//	//////////////////////////////////////////////////////////////////////////
//	//{{ 2010. 07. 16  ������	�̺�Ʈ ���� �ý���	
//#ifdef SERV_EVENT_MONSTER
//	if( CXSLDungeon::IsTutorialDungeon( GetDungeonID() ) == false  &&
//		CXSLDungeon::IsTCDungeon( GetDungeonID() ) == false  &&
//		CXSLDungeon::IsHenirDungeon( GetDungeonID() ) == false 
//		//{{ 2011. 07. 25    ��μ�    �̺�Ʈ ���� ���� ���� ����
//#ifdef SERV_EVENT_MONSTER_DUNGEON_RESTRICT
//		&& CXSLDungeon::IsDefenceDungeon( GetDungeonID() ) == false    // ���潺 �������� �̺�Ʈ ���Ͱ� �������� �ʵ����Ѵ�.
//#endif SERV_EVENT_MONSTER_DUNGEON_RESTRICT
//		//}} 
//		)
//	{
//		BOOST_TEST_FOREACH( KGameEventScriptManager::KAdventNpcEventInfo&, kInfo, m_vecEventMonster )
//		{
//			// ���� ������������ ���� ��ų�� üũ!
//			if( kInfo.m_bIsAdventInBossStage == false  &&  bIsBossStage == true )
//				continue;
//
//			// ���̻� ���� ��ų ī��Ʈ�� �������� �ʴٸ� �н�!
//			if( kInfo.m_iMonsterDropCount <= 0 )
//				continue;
//
//			// ������ �� �ִ� �������� üũ!
//			if( kInfo.m_setAdventDungeon.empty() == false )
//			{
//				if( kInfo.m_setAdventDungeon.find( GetDungeonID() ) == kInfo.m_setAdventDungeon.end() )
//					continue;
//			}
//
//			// ���� �ϸ� �ȵǴ� �������� üũ!
//			if( kInfo.m_setNoAdventDungeon.empty() == false )
//			{
//				if( kInfo.m_setNoAdventDungeon.find( GetDungeonID() ) != kInfo.m_setNoAdventDungeon.end() )
//					continue;
//			}
//
//			// ���� �ϸ� �ȵǴ� �������� ���� üũ!
//			if( kInfo.m_setNoAdventStage.empty() == false )
//			{
//				if( kInfo.m_setNoAdventStage.find( GetStageID() ) != kInfo.m_setNoAdventStage.end() )				
//					continue;
//			}
//
//			// ������ ���͸� ��������!
//			const int iAdventNpcID = kInfo.m_kAdventNpcLot.Decision();
//			if( iAdventNpcID == KLottery::CASE_BLANK )
//				continue;
//
//			// �̺�Ʈ ���� ���� ó��!
//			if( EventDropMonster( iAdventNpcID, GetStageID(), kNot ) == true )
//			{
//				// ���� ���� �����ߴٸ� ī��Ʈ �ϳ� ����!
//				--kInfo.m_iMonsterDropCount;
//			}
//		}
//	}
//#endif SERV_EVENT_MONSTER
//	//////////////////////////////////////////////////////////////////////////
//
//	//{{ 2010. 11. 20	������	Ư�� �ð� ���� ����
//#ifdef SERV_TIME_DROP_MONSTER_EVENT
//	if( CXSLDungeon::IsTutorialDungeon( GetDungeonID() ) == false  &&
//		CXSLDungeon::IsTCDungeon( GetDungeonID() ) == false  &&
//		CXSLDungeon::IsHenirDungeon( GetDungeonID() ) == false )
//	{
//		if( m_mapTimeDropEventMonster.empty() == true )
//		{
//			int iTimeDropNpcEventUID = 0;
//			int iTimeDropEventNpcID = 0;
//			if( SiKDungeonEventManager()->CheckTimeDropEventMonster( GetDungeonIDAndDif(), GetStageID(), kNot, iTimeDropNpcEventUID, iTimeDropEventNpcID ) == true )
//			{
//				m_mapTimeDropEventMonster.insert( std::make_pair( iTimeDropEventNpcID, iTimeDropNpcEventUID ) );
//			}
//		}
//	}
//#endif SERV_TIME_DROP_MONSTER_EVENT
//	//}}
//
//	//{{ 2010. 03. 21  ������	��д��� ����
//#ifdef SERV_SECRET_HELL
//
//	if( m_cDungeonMode == CXSLDungeon::DM_SECRET_HELL  &&  eStageType == CXSLDungeonStage::ST_EXTRA_NPC )
//	{
//		// ��д��� extra ���� ���� ó��
//		SecretDungeonExtraStageMonster( GetStageID(), kPacket_.m_cPartyFever, kNot );
//	}
//#endif SERV_SECRET_HELL
//	//}}
//
//	//{{ 2010. 07. 09  ������	��ӷ� �̺�Ʈ Ȯ��
//#ifdef SERV_PC_BANG_DROP_EVENT	
//	if( SiKGameEventManager()->CheckItemDropProbEvent( m_iItemDropEventProbCount, m_bWithPlayPcBangEvent ) == true )
//	{
//		// PC�� ������ �Բ� �÷��� �ؾ߸� �̺�Ʈ�� ����Ǵ� ���! ��ȿ� PC�� ������ �ִ��� üũ����!
//		if( m_bWithPlayPcBangEvent )
//		{
//			if( m_spRoomUserManager->IsExistPcBangPlayer() == false )
//			{
//				// ����Ʈ�� �־��ش�!
//				m_iItemDropEventProbCount = REE_DROP_EVENT_PROB_DEFAULT;
//				m_bWithPlayPcBangEvent = false;
//			}
//		}
//	}
//	else
//	{
//		m_iItemDropEventProbCount = REE_DROP_EVENT_PROB_DEFAULT;
//		m_bWithPlayPcBangEvent = false;
//	}
//
//	// Not��Ŷ�� ����
//	kNot.m_iItemDropEventProbCount = m_iItemDropEventProbCount;
//	kNot.m_bWithPlayPcBangEvent = m_bWithPlayPcBangEvent;
//#endif SERV_PC_BANG_DROP_EVENT
//	//}}	
//
//	//////////////////////////////////////////////////////////////////////////	
//	//{{ 2010. 03. 24  ������	��д��� ����
//	//#ifdef SERV_SECRET_HELL
//	//////////////////////////////////////////////////////////////////////////
//
//	//////////////////////////////////////////////////////////////////////////
//	// NPC UID �߱� ó��!
//
//	NPC_DATA	npcData;
//
//	//{{ 2009. 5. 13  ������	���꽺�������� drop npc��
//	m_mapSubStageExpNpcNum.clear();
//	//}}
//
//	//{{ 2011. 09. 07  ��μ�	���� �������� DIE NPC �ð� üũ ���
//#ifdef SERV_SUB_STAGE_NPC_DIE_CHECK
//	m_mapSubStageActiveNpcNum.clear();
//#endif SERV_SUB_STAGE_NPC_DIE_CHECK
//	//}}
//
//	std::map< int, KNPCList >::iterator mit;
//	for( mit = kNot.m_mapNPCData.begin(); mit != kNot.m_mapNPCData.end(); ++mit )
//	{
//		const int iSubStageID = mit->first;
//		KNPCList& kNPCList = mit->second;
//
//		//////////////////////////////////////////////////////////////////////////	
//		//{{ 2010. 7. 28  ������	������ ���� NPC
//#ifdef SERV_AUTO_HACK_CHECK_NPC
//		if( kNPCList.m_NPCList.empty() == false )
//		{
//			KNPCUnitReq kNPCData;
//			kNPCData.Init();
//
//			//NPC DATA SET..
//			kNPCData.m_NPCID			= CXSLUnitManager::NUI_DUNGEON_CHECKER1 + ( rand() % 5 );
//			kNPCData.m_KeyCode			= -1;
//			kNPCData.m_bFocusCamera		= false;
//			kNPCData.m_nStartPos		= SiCXSLDungeonManager()->GetStartPosByRandom( GetDungeonIDAndDif(), GetStageID(), iSubStageID );
//			kNPCData.m_bActive			= false;
//			kNPCData.m_bShowGage		= false;
//			kNPCData.m_Level			= SiKGameSysVal()->GetLimitsLevel();
//			kNPCData.m_bNoDrop			= true;
//			
//			if( kNPCData.m_nStartPos != -1 )
//			{
//				kNPCList.m_NPCList.push_back( kNPCData );
//			}
//			else
//			{
//				// ���� ��ġ�� -1�̸� �̹� ���� ������ NPC�� ��ġ�� ����!
//				BOOST_TEST_FOREACH( const KNPCUnitReq&, kNpc, kNPCList.m_NPCList )
//				{
//					kNPCData.m_nStartPos = kNpc.m_nStartPos;
//					kNPCList.m_NPCList.push_back( kNPCData );
//					break;
//				}
//			}
//		}
//#endif SERV_AUTO_HACK_CHECK_NPC
//		//}}
//		//////////////////////////////////////////////////////////////////////////
//
//		//{{ 2009. 12. 29  ������	NPCKILLALL��ŷ����
//		SUB_STAGE_NPC_INFO sSubStageNpcInfo;
//		//}}
//		int iExpNpcNum = 0;
//
//		//{{ 2011. 09. 07  ��μ�	���� �������� DIE NPC �ð� üũ ���
//#ifdef SERV_SUB_STAGE_NPC_DIE_CHECK
//		int iSubStageActiveNpc = 0;
//#endif SERV_SUB_STAGE_NPC_DIE_CHECK
//		//}}
//		
//		for( int i = 0; i < (int)kNPCList.m_NPCList.size(); ++i )
//		{
//			// 1. NPC UID �߱�
//			const int nNPCUID = m_nNPCUID++;
//
//			// 2. NPC UID ����
//			kNPCList.m_NPCList[i].m_UID = nNPCUID;
//
//			// 3. NPC(Monster) ������ ���� data ����
//			npcData.m_bDie		= false;
//			npcData.m_cLevel	= kNPCList.m_NPCList[i].m_Level;
//			npcData.m_iNPCID	= kNPCList.m_NPCList[i].m_NPCID;
//			npcData.m_bActive	= kNPCList.m_NPCList[i].m_bActive;
//			npcData.m_bNoDrop	= kNPCList.m_NPCList[i].m_bNoDrop;
//			npcData.m_bIsBoss	= ( kNPCList.m_NPCList[i].m_cMonsterGrade == CXSLUnitManager::MG_BOSS_NPC );	// ���� ������� ������������ üũ.
//			//npcData.m_bIsBoss	= kNPCList.m_NPCList[i].m_bHasBossGage;	//������������ üũ.
//			//{{ 2009. 12. 18  ������	��������ġ����
//			npcData.m_cMonsterGrade = kNPCList.m_NPCList[i].m_cMonsterGrade; //�Ϲݸ����� �߰��������� �������� üũ.
//			//}}
//
//			//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
//#ifdef SERV_INSERT_GLOBAL_SERVER
//			if( kNPCList.m_NPCList[i].m_NPCID == CXSLUnitManager::NUI_DEFENCE_CRYSTAL )
//			{
//				if( m_iDefenseMainCrystalNpcUID != 0 )
//				{
//					START_LOG( cerr, L"ū ������ 1 ���� ����? ����� �Ͼ���� �ȵǴ� ����!" )
//						<< BUILD_LOG( m_iDefenseMainCrystalNpcUID )
//						<< BUILD_LOG( kNPCList.m_NPCList[i].m_NPCID )
//						<< END_LOG;
//				}
//				else
//				{
//					m_iDefenseMainCrystalNpcUID = nNPCUID;
//				}
//			}
//			if( kNPCList.m_NPCList[i].m_NPCID == CXSLUnitManager::NUI_DEFENCE_CRYSTAL_SMALL )
//			{
//				if( m_setDefenseSmallCrystalNpcUID.size() > MAX_SMALL_CRYSTAL )
//				{
//					START_LOG( cerr, L"�� ������ 4 ���� ����? ����� �Ͼ���� �ȵǴ� ����!" )
//						<< BUILD_LOG( m_setDefenseSmallCrystalNpcUID.size() )
//						<< END_LOG;
//				}
//				else
//				{
//					m_setDefenseSmallCrystalNpcUID.insert( nNPCUID );
//				}
//			}
//#endif SERV_INSERT_GLOBAL_SERVER
//			//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
//
//			m_mapNPCData.insert( std::make_pair( nNPCUID, npcData ) );
//			
//			// 4. NPCKILLALL��ŷ����
//			if( npcData.m_cMonsterGrade == CXSLUnitManager::MG_BOSS_NPC )
//			{
//				sSubStageNpcInfo.m_setBossNpcUID.insert( nNPCUID );
//			}
//			else if( npcData.m_cMonsterGrade == CXSLUnitManager::MG_MIDDLE_BOSS_NPC )
//			{
//				sSubStageNpcInfo.m_setMiddleBossNpcUID.insert( nNPCUID );
//			}
//			else if( npcData.m_bActive )
//			{
//				sSubStageNpcInfo.m_setNormalNpcUID.insert( nNPCUID );
//			}
//
//			//////////////////////////////////////////////////////////////////////////			
//			//{{ 2009. 5. 13  ������	���꽺�������� ����ġ �ִ� npc��
//			if( m_eDungeonType == CXSLDungeon::DT_HENIR )
//			{
//				if( SiKDropTable()->IsHenirExpDropNpc( static_cast<CXSLDungeon::DUNGEON_MODE>(m_cDungeonMode), npcData.m_iNPCID ) == true  /*&&  !npcData.m_bNoDrop*/ )
//				{
//					iExpNpcNum++;
//				}
//				//{{ 2011. 09. 07  ��μ�	���� �������� DIE NPC �ð� üũ ���
//#ifdef SERV_SUB_STAGE_NPC_DIE_CHECK
//				if( npcData.m_bActive == true )
//				{
//					iSubStageActiveNpc++;
//				}
//#endif SERV_SUB_STAGE_NPC_DIE_CHECK
//				//}}
//			}
//			else
//			{
//				if( SiKDropTable()->IsExpDropNpc( m_iDungeonID + static_cast<int>(m_cDifficultyLevel), npcData.m_iNPCID ) == true /*&&  !npcData.m_bNoDrop*/ )
//				{
//					iExpNpcNum++;
//				}
//				//{{ 2011. 09. 07  ��μ�	���� �������� DIE NPC �ð� üũ ���
//#ifdef SERV_SUB_STAGE_NPC_DIE_CHECK
//				if( npcData.m_bActive == true )
//				{
//					iSubStageActiveNpc++;
//				}
//#endif SERV_SUB_STAGE_NPC_DIE_CHECK
//				//}}
//			}
//			//}}
//			//////////////////////////////////////////////////////////////////////////
//		}
//
//		//{{ 2009. 12. 29  ������	NPCKILLALL��ŷ����
//		KSubStageKey kSubStageKey( kNot.m_iStageID, iSubStageID );
//		m_mapSubStageNpcInfo.insert( std::make_pair( kSubStageKey, sSubStageNpcInfo ) );
//		//}}
//
//		// drop npc��
//		m_mapSubStageExpNpcNum.insert( std::make_pair( iSubStageID, iExpNpcNum ) );
//
//		//{{ 2011. 09. 07  ��μ�	���� �������� DIE NPC �ð� üũ ���
//#ifdef SERV_SUB_STAGE_NPC_DIE_CHECK
//		m_mapSubStageActiveNpcNum.insert( std::make_pair( iSubStageID, iExpNpcNum ) );
//#endif SERV_SUB_STAGE_NPC_DIE_CHECK
//		//}}
//	}
//
//	//////////////////////////////////////////////////////////////////////////
//	// �Ӽ�NPC ���� - NPC UID�߱ް��� ���� �����ϴ�.
//	if( eStageType == CXSLDungeonStage::ST_EXTRA )
//	{
//		// ù��° Extra Stage�����ΰ�?
//		if( m_iExtraStageLoadCount == 1 )
//		{
//			// ���׶� ����� ��쿡�� �÷��� �ð� �ʱ�ȭ!
//			if( CXSLDungeon::IsAlteraDungeon( GetDungeonIDAndDif() ) == true )
//			{
//				// ���� �ð� �ʱ�ȭ
//				SetRemainingPlayTime( GetPlayTimeLimit() );
//				BroadCast( ERM_REMAINING_PLAY_TIME_NOT, GetRemainingPlayTime() );
//
//				// EXTRA STAGE ���� ������� Ÿ���� ����!
//				m_fPoisonTimeLimit = SiCXSLDungeonManager()->GetExtraStagePoisonTimeLimit( GetDungeonIDAndDif() );
//
//				KEGS_DUNGEON_EFFECT_REMAINING_TIME_NOT kNot;
//				kNot.m_fRemainingTime = m_fPoisonTimeLimit;
//				BroadCast( ERM_DUNGEON_EFFECT_REMAINING_TIME_NOT, kNot );
//			}
//		}
//
//		SecretDungeonExtraStageAttribMonsterDrop( kNot );
//	}
//	else
//	{
//		NormalDungeonAttribMonsterDrop( kNot );
//	}
//	
//	//////////////////////////////////////////////////////////////////////////	
////#endif SERV_SECRET_HELL
//	//}}
//	//////////////////////////////////////////////////////////////////////////
//	
//	BroadCast( ERM_DUNGEON_STAGE_LOAD_NOT, kNot );
//}

//////////////////////////////////////////////////////////////////////////
#endif SERV_BATTLE_FIELD_SYSTEM
//////////////////////////////////////////////////////////////////////////
//}}

//{{ �̺�Ʈ ���� ���� ( �Ʊ� ����	) - ��μ�
#ifdef SERV_ALLY_EVENT_MONSTER
#ifdef SERV_ACTIVE_EVENT_MONSTER
bool KDungeonRoom::EventDropMonster( IN int iNpcID, IN bool bAllyNpc, IN int iStageID, IN bool bActiveNpc, IN OUT KEGS_DUNGEON_STAGE_LOAD_NOT& kNot )
#else
bool KDungeonRoom::EventDropMonster( IN int iNpcID, IN bool bAllyNpc, IN int iStageID, IN OUT KEGS_DUNGEON_STAGE_LOAD_NOT& kNot )
#endif SERV_ACTIVE_EVENT_MONSTER
#else
bool KDungeonRoom::EventDropMonster( IN int iNpcID, IN int iStageID, IN OUT KEGS_DUNGEON_STAGE_LOAD_NOT& kNot )
#endif SERV_ALLY_EVENT_MONSTER
//}}
{
	// �����ϴ� ���Ͱ� �Ѹ����� ������ �̺�Ʈ ���͵� ������Ű�� �ʴ´�.
	if( kNot.m_mapNPCData.size() <= 0 )
	{
		START_LOG( cerr, L"�̺�Ʈ ���� ���� ������ ���� ������ ����?" )
			<< BUILD_LOG( iNpcID )
			<< BUILD_LOG( iStageID )
			<< BUILD_LOG( kNot.m_mapNPCData.size() )
			<< END_LOG;
		return false;
	}

	// ���° SubStage�� �����ų�� �����ϰ� ����!
	int iRandomSubStage = rand() % static_cast<int>(kNot.m_mapNPCData.size());

	std::map< int, KNPCList >::const_iterator mit;
	int iSubStageCount = 0;
	for( mit = kNot.m_mapNPCData.begin(); mit != kNot.m_mapNPCData.end(); ++mit )
	{
		if( iRandomSubStage == iSubStageCount )
		{
			iRandomSubStage = mit->first;
			break;
		}
		++iSubStageCount;
	}

	// �������� �̺�Ʈ ���� ���
	bool bEventMonDrop = false;
	if( ( rand() % 2 ) == 0 )
	{
		bEventMonDrop = true;
	}
	// �������� ������� ������ ��� ���� ������������ ������ ����Ѵ�.
	else
	{
		std::map< int, KNPCList >::const_iterator mit;
		for( mit = kNot.m_mapNPCData.begin(); mit != kNot.m_mapNPCData.end(); ++mit )
		{
			const KNPCList& kNPCList = mit->second;
			for( u_int ui = 0; ui < kNPCList.m_NPCList.size(); ++ui )
			{
				if( kNPCList.m_NPCList[ui].m_bHasBossGage == true )
				{
					bEventMonDrop = true;
					break;
				}
			}
		}
	}
	
	if( bEventMonDrop )
	{
		KNPCUnitReq kNPCData;
		kNPCData.Init();

		//NPC DATA SET..
		kNPCData.m_NPCID			= iNpcID;
		kNPCData.m_KeyCode			= -1;
		kNPCData.m_bFocusCamera		= false;
		kNPCData.m_nStartPos		= SiCXSLDungeonManager()->GetStartPosByRandom( (m_iDungeonID + static_cast<int>(m_cDifficultyLevel)), iStageID, iRandomSubStage );
#ifdef SERV_ACTIVE_EVENT_MONSTER
		kNPCData.m_bActive			= bActiveNpc;
#else
		kNPCData.m_bActive			= true;
#endif SERV_ACTIVE_EVENT_MONSTER
		kNPCData.m_bShowGage		= true;
		kNPCData.m_AddPos.x			= 0.0f;
		kNPCData.m_AddPos.y			= 0.0f;
		kNPCData.m_AddPos.z			= 0.0f;
		//{{ 2012. 05. 10	������	���� ���� ���� ����
#ifdef SERV_DUNGEON_NPC_LEVEL
		kNPCData.m_Level			= SiCXSLDungeonManager()->GetDungeonNpcLevel( m_iDungeonID );
#else
		kNPCData.m_Level			= SiCXSLDungeonManager()->GetDungeonMinLevel( m_iDungeonID );
#endif SERV_DUNGEON_NPC_LEVEL
		//}}
		kNPCData.m_bHasBossGage		= false;
		kNPCData.m_bShowBossName	= false;
		kNPCData.m_bShowSubBossName	= false;
		kNPCData.m_bSiegeMode		= false;
		kNPCData.m_bIsRight			= (rand()%2) ? true : false;
		kNPCData.m_bNoDrop			= false;
		kNPCData.m_fUnitScale		= -1.0f;

		//{{ �̺�Ʈ ���� ���� ( �Ʊ� ����	) - ��μ�
#ifdef SERV_ALLY_EVENT_MONSTER
		kNPCData.m_cTeamNum = ( bAllyNpc == true ) ? CXSLRoom::TN_RED : CXSLRoom::TN_MONSTER;
		kNPCData.m_cAIType = ( bAllyNpc == true ) ? CXSLUnit::NAT_ALLY : CXSLUnit::NAT_NORMAL;
		kNPCData.m_bAllyNpc = bAllyNpc;
#endif SERV_ALLY_EVENT_MONSTER
		//}}

		//{{ 2008. 9. 12  ������	npc���� ����������?
		if( kNPCData.m_nStartPos == -1 )
		{
			bEventMonDrop = false;
		}
		else
		{
			kNot.m_mapNPCData[iRandomSubStage].m_NPCList.push_back( kNPCData );
		}
		//}}
	}

	return bEventMonDrop;
}

//{{ 2012. 02. 28	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
void KDungeonRoom::SecretDungeonExtraStageMonster( IN const int iStageID, IN const short sPartyFever, IN OUT KEGS_DUNGEON_STAGE_LOAD_NOT& kNot )
#else
void KDungeonRoom::SecretDungeonExtraStageMonster( IN int iStageID, IN char cPartyFever, IN OUT KEGS_DUNGEON_STAGE_LOAD_NOT& kNot )
#endif SERV_BATTLE_FIELD_SYSTEM
//}}
{
	// �̹� ���������� �ִٸ�?
	if( m_bAdventExtraNpc == true )
		return;

	if( kNot.m_mapNPCData.size() <= 0 )
	{
		START_LOG( cerr, L"�̺�Ʈ ���� ���� ������ ���� ������ ����?" )
			<< BUILD_LOG( iStageID )
			<< BUILD_LOG( kNot.m_mapNPCData.size() )
			<< END_LOG;
		return;
	}

	// �������� ������ ����
	//{{ 2012. 05. 10	������	���� ���� ���� ����
#ifdef SERV_DUNGEON_NPC_LEVEL
	const char cLevel = static_cast<char>(GetAverageUserLevelAtDungeonStart());
#else
	const char cLevel = static_cast<char>(m_iHighestUserLevelAtDungeonStart);
#endif SERV_DUNGEON_NPC_LEVEL
	//}}	
	//{{ 2012. 02. 28	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	float fExtraNpcRate = static_cast<float>(sPartyFever) * SiCXSLDungeonManager()->GetExtraStageFactor() * SiCXSLDungeonManager()->GetExtraStageConditionFactor( cLevel );
#else
	float fExtraNpcRate = static_cast<float>(cPartyFever) * 2.0f * SiCXSLDungeonManager()->GetExtraStageConditionFactor( cLevel );
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}	
	if( fExtraNpcRate > 100.f )
		fExtraNpcRate = 100.f;

	KLottery kLot;
	kLot.AddCase( KLottery::CASE_BLANK + 1, fExtraNpcRate );

	if( kLot.Decision() == KLottery::CASE_BLANK )
	{
		return;
	}
	else
	{
		// extra npc ���� ó��!
		m_bAdventExtraNpc = true;
	}

	// ���� ��ġ ����
	int iRandomSubStage = rand() % static_cast<int>(kNot.m_mapNPCData.size());

	std::map<int, KNPCList>::iterator mit;
	int iCount = 0;
	for( mit = kNot.m_mapNPCData.begin(); mit != kNot.m_mapNPCData.end(); ++mit )
	{
		if( iRandomSubStage == iCount )
		{
			iRandomSubStage = mit->first;
			break;
		}
		++iCount;
	}

	// Extra�������� ���� ���
	{
		KNPCUnitReq kNPCData;
		kNPCData.Init();

		//NPC DATA SET..
		kNPCData.m_NPCID			= CXSLUnitManager::NUI_LUTO;
		kNPCData.m_KeyCode			= -1;
		kNPCData.m_bFocusCamera		= true;
		kNPCData.m_bActive			= false;
		kNPCData.m_bShowGage		= true;
		kNPCData.m_AddPos.x			= 0.0f;
		kNPCData.m_AddPos.y			= 0.0f;
		kNPCData.m_AddPos.z			= 0.0f;
		//{{ 2012. 05. 10	������	���� ���� ���� ����
#ifdef SERV_DUNGEON_NPC_LEVEL
		kNPCData.m_Level			= GetAverageUserLevelAtDungeonStart() + 5 + SiCXSLDungeonManager()->GetSecretDungeonExtraStageNpcLevel();
#else
		kNPCData.m_Level			= SiCXSLDungeonManager()->GetDungeonMinLevel( m_iDungeonID );
#endif SERV_DUNGEON_NPC_LEVEL
		//}}		
		kNPCData.m_bHasBossGage		= false;
		kNPCData.m_bShowBossName	= false;
		kNPCData.m_bShowSubBossName	= false;
		kNPCData.m_bSiegeMode		= false;
		kNPCData.m_bNoDrop			= true;
		kNPCData.m_fUnitScale		= -1.0f;

		// ���� ��ũ��Ʈ���� EXTRA NPC���� ���
		SiCXSLDungeonManager()->GetExtraNpcInfo( (m_iDungeonID + static_cast<int>(m_cDifficultyLevel)), iStageID, iRandomSubStage, kNPCData.m_nStartPos, kNPCData.m_bIsRight );

		//{{ 2008. 9. 12  ������	npc���� ����������?
		if( kNPCData.m_nStartPos == -1 )
		{
			START_LOG( cwarn, L"Npc�� ���� ����������?" )
				<< BUILD_LOG( iStageID )
				<< BUILD_LOG( iRandomSubStage )
				<< END_LOG;
		}
		else
		{
			kNot.m_mapNPCData[iRandomSubStage].m_NPCList.push_back( kNPCData );
		}
		//}}
	}
}

_IMPL_ON_FUNC( ERM_DUNGEON_STAGE_LOAD_COMPLETE_REQ, KEGS_DUNGEON_STAGE_LOAD_COMPLETE_REQ )
{
	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_PLAY ), ERM_DUNGEON_STAGE_LOAD_COMPLETE_ACK, KPacketOK );

	m_usLoadingState = ERM_DUNGEON_STAGE_LOAD_COMPLETE_REQ;

    // loading ���¸� ��ȭ��Ų��.
    if( !m_spRoomUserManager->SetStageLoaded( FIRST_SENDER_UID, true ) )
    {
        kPacket.m_iOK = NetError::ERR_ROOM_13;
        SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_STAGE_LOAD_COMPLETE_ACK, kPacket );
        return;
    }

    // �ε� ���°� ����� ���� ID�� �˾Ƴ���.    
	KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser( FIRST_SENDER_UID );
    if( spRoomUser == NULL )
    {
        kPacket.m_iOK = NetError::ERR_ROOM_02;
        SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_STAGE_LOAD_COMPLETE_ACK, kPacket );
        return;
    }
	const int iSlotID = spRoomUser->GetSlotID();

	//{{ 2009. 7. 21  ������	����� �������� ī��Ʈ
	if( m_eDungeonType == CXSLDungeon::DT_NORMAL  ||  kPacket_.m_bIsSecretStage == false )
	{
		spRoomUser->IncreasePassedStageCount();
	}
	//}}

	//{{ 2011. 06. 13	������	�������� �α�
#ifdef SERV_DUNGEON_STAGE_LOG
	// ù��° ���������� �ƴ� ��쿡�� ������!
	if( m_iLoadingCompleteStageID != -1 )
	{
		KELOG_DUNGEON_STAGE_LOG_NOT kPacketToLog;
		kPacketToLog.m_iDungeonID		  = GetDungeonID();
		kPacketToLog.m_iStageID			  = m_iLoadingCompleteStageID; // ���� ��������
		kPacketToLog.m_iUnitUID			  = spRoomUser->GetCID();
		kPacketToLog.m_cLevel			  = spRoomUser->GetLevel();
		kPacketToLog.m_cDifficulty		  = GetDifficultyLevel();
		kPacketToLog.m_bIsChallenge 	  = ( m_cDungeonMode == CXSLDungeon::DM_HENIR_PRACTICE );
		kPacketToLog.m_cUnitClass		  = spRoomUser->GetUnitClass_LUA();
		kPacketToLog.m_iStagePlayTime	  = static_cast<int>(spRoomUser->GetStagePlayTime()); // �ش� ���������� �÷����� �÷��� Ÿ��
		kPacketToLog.m_iStageDamage		  = ( kPacket_.m_iDamage - spRoomUser->GetStageDamage() ); // �ش� �������� ���� ���� �����
		kPacketToLog.m_iResurrectionCount = spRoomUser->GetUsedRessurectionStoneCount();
		kPacketToLog.m_wstrRegDate		  = KncUtil::GetCurrentTimeString();
		SendToLogDB( ELOG_DUNGEON_STAGE_LOG_NOT, kPacketToLog );

		// �������� ���� ����� ���� ���
        spRoomUser->SetStageDamage( kPacket_.m_iDamage );
	}
#endif SERV_DUNGEON_STAGE_LOG
	//}}

    // ���� ������� ack�� ������.
    kPacket.m_iOK = NetError::NET_OK;
    SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_STAGE_LOAD_COMPLETE_ACK, kPacket );

	// � ������ �ε��Ϸ� �Ǿ����� ��ε�ĳ����
	KEGS_DUNGEON_STAGE_LOAD_COMPLETE_NOT kPacketNot;
	kPacketNot.m_iSlotID = iSlotID;
#ifdef  SERV_KTDX_OPTIMIZE_UDP_ROBUST_CONNECTION
    kPacketNot.m_iUnitUID = FIRST_SENDER_UID;
#endif  SERV_KTDX_OPTIMIZE_UDP_ROBUST_CONNECTION
	BroadCast( ERM_DUNGEON_STAGE_LOAD_COMPLETE_NOT, kPacketNot );

    // ��� ������ �ε� �������� not�� ������.
    if( m_spRoomUserManager->IsAllPlayerStageLoaded() )
    {
        // Loading ���� ����.
        m_spRoomUserManager->ResetStageLoaded();

		//��� �ε�ó���� ������ ������ �˷���
		KEGS_DUNGEON_STAGE_START_NOT kPacketStartNot;
		kPacketStartNot.m_iStageID = m_iStageID;
		BroadCast( ERM_DUNGEON_STAGE_START_NOT, kPacketStartNot );

		//{{ 2009. 12. 14  ������	��ϸ��ӽ��κ�
		CheckTempInventoryClear();
		//}}

		//{{ 2011. 06. 13	������	�������� �α�
#ifdef SERV_DUNGEON_STAGE_LOG
		// �ε� �Ϸ�� ��������ID�� ����Ѵ�!
		m_iLoadingCompleteStageID = GetStageID();
#endif SERV_DUNGEON_STAGE_LOG
		//}}
#ifdef  SERV_CHOOSE_FASTEST_HOST
#ifdef  SERV_OPTIMIZE_CHOOSE_FASTEST_HOST_ENHANCE
        if ( m_iSubStageCount == 0 )
        {
            m_kTimer[TM_CHECK_CHANGE_HOST].restart();
            m_bCheckPingScoreExecuted = false;
        }
#endif  SERV_OPTIMIZE_CHOOSE_FASTEST_HOST_ENHANCE
#endif  SERV_CHOOSE_FASTEST_HOST

    }

	//{{ 2012. 07. 04	������		�ڵ���Ȱ ���� ����
#ifdef SERV_AUTO_RESURRECTION_BUG_FIX
#else
	//////////////////////////////////////////////////////////////////////////
	//{{ 2009. 7. 3  ������		��ϸ� �ð�	
	if( m_eDungeonType == CXSLDungeon::DT_HENIR )
	{
		// Ư�� �������� ���� ��Ȱ!
		//{{ 2011. 08. 12   ��μ�      ��ϸ� ���� 
#ifdef SERV_NEW_HENIR_TEST
		if( SiKHenirResultTable()->IsResurrectionStage( GetStageID() ) == true )
#else
		if( SiKHenirResultTable()->IsResurrectionStage( spRoomUser->GetPassedStageCount() ) == true )
#endif SERV_NEW_HENIR_TEST
		//}}
		{
			// ���� �������� Ȯ��!
			if( spRoomUser->IsDie() == true )
			{
				// �ǻ츮��!
				spRoomUser->SetDie( false );

				KEGS_RESURRECT_TO_CONTINUE_DUNGEON_NOT kPacketNot;
				kPacketNot.m_iUnitUID = spRoomUser->GetCID();
				kPacketNot.m_iUsedByUnitUID = spRoomUser->GetCID();
				kPacketNot.m_iStartPosIndex = SiCXSLDungeonManager()->GetStartPosByRandom( (m_iDungeonID + static_cast<int>(m_cDifficultyLevel)), m_iStageID, m_iSubStageID );
				BroadCast( EGS_RESURRECT_TO_CONTINUE_DUNGEON_NOT, kPacketNot );

				//{{ 2008. 6. 17  ������  �ǽð� ��Ȱ��
				m_spRoomUserManager->SetEndPlay( spRoomUser->GetCID(), false );
				//}}
			}
		}
	}
	//}}
	//////////////////////////////////////////////////////////////////////////
#endif SERV_AUTO_RESURRECTION_BUG_FIX
	//}}
}

#ifdef SERV_STAGE_CLEAR_IN_SERVER
_IMPL_ON_FUNC( ERM_DUNGEON_SUB_STAGE_CLEAR_REQ, KEGS_DUNGEON_SUB_STAGE_CLEAR_REQ )
{
	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_PLAY ), ERM_DUNGEON_SUB_STAGE_CLEAR_ACK, KEGS_DUNGEON_SUB_STAGE_CLEAR_ACK );

	kPacket.m_iOK = NetError::NET_OK;

	if( SiCXSLDungeonManager()->GetNextStage( m_kNextStageData, GetDungeonIDAndDif(), GetStageID(), GetSubStageID(), kPacket_.m_iClearConditionIndex ) == false )
	{
		// ���� ���������� ã�µ� ����. ��ŷ�̴� by ������
		START_LOG( cerr, L"���� ���������� ã�µ� ����. ��ŷ�̴�." )
			<< BUILD_LOG( GetDungeonIDAndDif() )
			<< BUILD_LOG( GetStageID() )
			<< BUILD_LOG( GetSubStageID() )
			<< BUILD_LOG( kPacket_.m_iClearConditionIndex )
			<< END_LOG;

		m_kNextStageData.m_eClearType = CXSLDungeonSubStage::CT_GAME;
		m_kNextStageData.m_iStageIndex = -1;
		m_kNextStageData.m_iSubStageIndex = -1;
		m_kNextStageData.m_iRate = 100;
		m_kNextStageData.m_iDungeonEndingSpeechTableIndex = 0;

		kPacket.m_iOK = NetError::ERR_ROOM_51;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_SUB_STAGE_CLEAR_ACK, kPacket );
		return;
	}
	// ��� �� ������ ���� ������
	m_iClearConditionIndex = kPacket_.m_iClearConditionIndex;

	kPacket.m_iClearType = static_cast<int>( m_kNextStageData.m_eClearType );
	kPacket.m_iStageIndex = m_kNextStageData.m_iStageIndex;
	kPacket.m_iSubStageIndex = m_kNextStageData.m_iSubStageIndex;
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_SUB_STAGE_CLEAR_ACK, kPacket );
}

_IMPL_ON_FUNC( ERM_SECRET_STAGE_LOAD_REQ, KEGS_SECRET_STAGE_LOAD_REQ )
{
	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_PLAY ), ERM_DUNGEON_STAGE_LOAD_ACK, KEGS_DUNGEON_STAGE_LOAD_ACK );

	if( m_kNextStageData.m_eClearType != CXSLDungeonSubStage::CT_STAGE_SECRET )
	{
		// ������ ����� Ŭ���� Ÿ���� �ƴ�. Ŭ���̾�Ʈ ��ŷ�̴�. by ������
		START_LOG( cerr, L"Ŭ����Ÿ���� �Ʒ��� ������ �� ��Ŷ�� �� ����? ��Ŀ!" )
			<< BUILD_LOG( m_kNextStageData.m_eClearType )
			<< BUILD_LOG( m_iClearConditionIndex )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_ROOM_51;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_STAGE_LOAD_ACK, kPacket );
		return;
	}

	if( SiCXSLDungeonManager()->GetNextStage( m_kNextStageData, GetDungeonIDAndDif(), GetStageID(), GetSubStageID(), m_iClearConditionIndex, kPacket_.m_iPadID ) == false )
	{
		// ���� ���������� ã�µ� ����. ��ŷ�̴� by ������
		START_LOG( cerr, L"���� ���������� ã�µ� ����. ��ŷ�̴�." )
			<< BUILD_LOG( GetDungeonIDAndDif() )
			<< BUILD_LOG( GetStageID() )
			<< BUILD_LOG( GetSubStageID() )
			<< BUILD_LOG( m_iClearConditionIndex )
			<< BUILD_LOG( kPacket_.m_iPadID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_ROOM_51;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_STAGE_LOAD_ACK, kPacket );
		return;
	}

	KEGS_DUNGEON_STAGE_LOAD_REQ kPacketReq;
	kPacketReq.m_iStageID = m_kNextStageData.m_iStageIndex;
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SECRET_STAGE_LOAD_ACK, kPacketReq );
}
#endif SERV_STAGE_CLEAR_IN_SERVER

_IMPL_ON_FUNC( ERM_DUNGEON_SUB_STAGE_OPEN_REQ, KEGS_DUNGEON_SUB_STAGE_OPEN_REQ )
{
	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_PLAY ), ERM_DUNGEON_SUB_STAGE_OPEN_ACK, KEGS_DUNGEON_SUB_STAGE_OPEN_ACK );

	kPacket.m_iOK = NetError::NET_OK;

#ifdef SERV_STAGE_CLEAR_IN_SERVER
	if( m_spRoomUserManager->CheckAuthLevel( FIRST_SENDER_UID ) == false )
	{
		if( m_bFirstSubStage == false )
		{
			if( m_kNextStageData.m_eClearType != CXSLDungeonSubStage::CT_SUB_STAGE )
			{
				// ������ ����� Ŭ���� Ÿ���� �ƴ�. Ŭ���̾�Ʈ ��ŷ�̴�. by ������
				START_LOG( cerr, L"Ŭ����Ÿ���� �Ʒ��� ������ �� ��Ŷ�� �� ����? ��Ŀ!" )
					<< BUILD_LOG( m_bFirstStage )
					<< BUILD_LOG( m_kNextStageData.m_eClearType )
					<< BUILD_LOG( m_kNextStageData.m_iStageIndex )
					<< END_LOG;

				kPacket.m_iOK = NetError::ERR_ROOM_51;
				SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_SUB_STAGE_OPEN_ACK, kPacket );
				return;
			}

			// Ÿ�� ��ü�� ���� ���� ���������� �ٸ��ٸ� ���ø� ����� �ؼ� ���� �����ϰ� ����. by ������
			kPacket_.m_iSubStageID = m_kNextStageData.m_iSubStageIndex;
		}
		else
		{
			// ���꽺������ üũ�� �� �ص� �Ǵ°� �������� ���� �� ó�� �ѹ� ���̴�. by ������
			kPacket_.m_iSubStageID = 0;
			m_bFirstSubStage = false;
		}
	}

	// ������ �ٽ� ���� ���� �� �ʱ�ȭ.
	m_kNextStageData.ClearData();
#endif SERV_STAGE_CLEAR_IN_SERVER
	
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_SUB_STAGE_OPEN_ACK, kPacket );

	// ��� �������� ���꽺������ �����غ� �϶�� �˸���. 
	KEGS_DUNGEON_SUB_STAGE_OPEN_NOT kNot;
	kNot.m_iSubStageID = kPacket_.m_iSubStageID;
	m_iSubStageID = kPacket_.m_iSubStageID;
	BroadCast( ERM_DUNGEON_SUB_STAGE_OPEN_NOT, kNot );

	//{{ 2012. 07. 04	������		�ڵ� ��Ȱ ���� ����
#ifdef SERV_AUTO_RESURRECTION_BUG_FIX
	if( m_eDungeonType == CXSLDungeon::DT_HENIR )
	{
		for( int idx = 0; idx < m_kRoomUserManager.GetNumMember(); ++idx )
		{
			KRoomUserPtr spRoomUser  = m_kRoomUserManager.GetUser( idx );
			if( IS_NULL( spRoomUser ) )
			{
				START_LOG( cerr, L"�� ������ �����Ͱ� �̻���." )
					<< END_LOG;
				continue;
			}

			HenirAutoResurrection( spRoomUser->GetCID() );
		}
	}
#endif SERV_AUTO_RESURRECTION_BUG_FIX
	//}}
}

_IMPL_ON_FUNC( ERM_DUNGEON_SUB_STAGE_GO_NEXT_REQ, KEGS_DUNGEON_SUB_STAGE_GO_NEXT_REQ )
{
	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_PLAY ), ERM_DUNGEON_SUB_STAGE_GO_NEXT_ACK, KEGS_DUNGEON_SUB_STAGE_GO_NEXT_ACK );

	m_usLoadingState = ERM_DUNGEON_SUB_STAGE_GO_NEXT_REQ;

	if( m_spRoomUserManager->SetStageID( FIRST_SENDER_UID, kPacket_.m_iNextStageID ) == false )
	{
		START_LOG( cerr, L"�������� �̵� ������ ���� ����.!" )
			<< BUILD_LOGc( kPacket_.m_iNextStageID )
			<< BUILD_LOGc( kPacket_.m_iBeforeStageID )
			<< END_LOG;
	}

	kPacket.m_iOK = NetError::NET_OK;
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_SUB_STAGE_GO_NEXT_ACK, kPacket );

	//�������ǿ� ���� �Ѹ��̻��� User Unit�� ������� ���� ������ �����ϱ����� ��Ƶд�.
	m_kDSSGoNextAllNot.m_iBeforeStageID	= kPacket_.m_iBeforeStageID;
	m_kDSSGoNextAllNot.m_iNextStageID	= kPacket_.m_iNextStageID;

	SendDSSGoNextAllNot( m_kDSSGoNextAllNot );
}

_IMPL_ON_FUNC( ERM_DUNGEON_SUB_STAGE_LOAD_COMPLETE_REQ, KEGS_DUNGEON_SUB_STAGE_LOAD_COMPLETE_REQ )
{
	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_PLAY ), ERM_DUNGEON_SUB_STAGE_LOAD_COMPLETE_ACK, KEGS_DUNGEON_SUB_STAGE_LOAD_COMPLETE_ACK );

	m_usLoadingState = ERM_DUNGEON_SUB_STAGE_LOAD_COMPLETE_REQ;

	// loading ���¸� ��ȭ��Ų��.
	if( !m_spRoomUserManager->SetStageLoaded( FIRST_SENDER_UID, true ) )
	{
		kPacket.m_iOK = NetError::ERR_ROOM_13;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_SUB_STAGE_LOAD_COMPLETE_ACK, kPacket );
		return;
	}

	//{{ 2012. 04. 17	������	���� �α� �÷� �߰�
#ifdef SERV_ADD_DUNGEON_LOG_COLUMN_NUM_2
	if( m_spRoomUserManager->SetSubStageID( FIRST_SENDER_UID, kPacket_.m_iSubStageID ) == false )
	{
		START_LOG( cerr, L"SubStageID�� ������ ������ ����! ������ �˻��ߴµ�?" )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< BUILD_LOG( kPacket_.m_iSubStageID )
			<< END_LOG;
	}
#endif SERV_ADD_DUNGEON_LOG_COLUMN_NUM_2
	//}}

	// ���� ������� ack�� ������.
	kPacket.m_iOK = NetError::NET_OK;
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_SUB_STAGE_LOAD_COMPLETE_ACK, kPacket );

	// ��� ������ �ε� �������� not�� ������.
	if( m_spRoomUserManager->IsAllPlayerStageLoaded() == true )
	{
		//{{ 2010. 8. 16	������	���� �ڵ� ����
		if( DungeonSubStageStartProcess( kPacket_.m_iSubStageID ) == false )
		{
			START_LOG( cerr, L"���� ���� �������� ���� ����!" )
				<< BUILD_LOG( kPacket_.m_iSubStageID )
				<< END_LOG;
		}
		//}}
	}

	//{{ 2008. 9. 17  ������	���� �������� �ε� ī��Ʈ
	KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser( FIRST_SENDER_UID );
	if( spRoomUser != NULL )
	{
		spRoomUser->IncreasePassedSubStageCount();

		//{{ QUEST ���� - ��μ�
#ifdef SERV_REFORM_QUEST
		// ���� �������� npc ����
		KERM_UPDATE_QUEST_INFO_NOT kNot;
		m_kDungeonMonsterManager.GetSubStageNpcList( GetStageID(), kPacket_.m_iSubStageID, kNot.m_vecNPC );

		if(kNot.m_vecNPC.empty() == false )
		{
			
			kNot.m_isDungeon = true;

			if( spRoomUser->CheckFindNPCQuest( GetDungeonIDAndDif(), kNot.m_vecNPC, true ) == true )
			{
				SendToGSCharacter( spRoomUser->GetGSUID(), spRoomUser->GetCID(), ERM_UPDATE_QUEST_INFO_NOT, kNot );
			}
		}
#endif SERV_REFORM_QUEST
		//}}
	}
	//}}
}

//{{ 2009. 7. 20  ������	��н�������
IMPL_ON_FUNC_NOPARAM( ERM_DUNGEON_SECRET_STAGE_ENTER_CHECK_REQ )
{
	KEGS_DUNGEON_SECRET_STAGE_ENTER_CHECK_ACK kPacket;

	if( GetStateID() != KRoomFSM::S_PLAY )
	{
		START_LOG( cerr, L"���� ����. Name : " << m_strName )
			<< L"    ���� ���� : " << GetStateIDString() << dbg::endl
			<< L"    ��� ���� : KRoomFSM::S_PLAY" << END_LOG;

		kPacket.m_bCanEnterSecretStage = false;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_SECRET_STAGE_ENTER_CHECK_ACK, kPacket );
		return;
	}

	// ��ϸ� �ð��� �³�?
	if( m_eDungeonType != CXSLDungeon::DT_HENIR )
	{
		START_LOG( cerr, L"��ϸ� �ð��� �ƴѵ� �� �̺�Ʈ�� �ֿ���? �Ͼ���� �ȵǴ� ����!" )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< BUILD_LOG( m_eDungeonType )
			<< END_LOG;
	}

	// ��� ������ ����ִ°�?
	if( m_spRoomUserManager->IsAllPlayerAlive() == false )
	{
		kPacket.m_bCanEnterSecretStage = false;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_SECRET_STAGE_ENTER_CHECK_ACK, kPacket );
		return;
	}

	// ���� �ǰ� �ʱ�ȭ
	m_spRoomUserManager->ResetAgreeEnterSecretStage();

	// ��� �������� �ǰ� Ÿ�̸� �۵�!
	//m_kEnterSecretStageTimer.CreateTimer();

	// ��� �������� ���� ���� ������!
	kPacket.m_bCanEnterSecretStage = true;
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_SECRET_STAGE_ENTER_CHECK_ACK, kPacket );

	// ��� �������� ��� �������� ���� ���� Ȯ���ϱ�
#ifdef SERV_STAGE_CLEAR_IN_SERVER
	KEGS_DUNGEON_SECRET_STAGE_ENTER_CHECK_NOT kPacketNot;
	kPacketNot.m_iStartSecretStageEnteringEvent = SiCXSLDungeonManager()->GetSecretStageEnteringEvent( GetDungeonIDAndDif(), GetStageID(), GetSubStageID(), m_iClearConditionIndex );

	BroadCast( ERM_DUNGEON_SECRET_STAGE_ENTER_CHECK_NOT, kPacketNot );
#else SERV_STAGE_CLEAR_IN_SERVER
	BroadCastID( ERM_DUNGEON_SECRET_STAGE_ENTER_CHECK_NOT );
#endif SERV_STAGE_CLEAR_IN_SERVER
}

_IMPL_ON_FUNC( ERM_DUNGEON_SECRET_STAGE_ENTER_SELECT_NOT, KEGS_DUNGEON_SECRET_STAGE_ENTER_SELECT_NOT )
{
	VERIFY_STATE( ( 1, KRoomFSM::S_PLAY ) );

	// ��ϸ� �ð��� �³�?
	if( m_eDungeonType != CXSLDungeon::DT_HENIR )
	{
		START_LOG( cerr, L"��ϸ� �ð��� �ƴѵ� �� �̺�Ʈ�� �ֿ���? �Ͼ���� �ȵǴ� ����!" )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< BUILD_LOG( m_eDungeonType )
			<< END_LOG;
	}

	// �ǻ� ǥ�� Ÿ�̸Ӱ� ����ִ°�?
	//if( m_kEnterSecretStageTimer.IsExist() == false )
	//{	
	//	return;
	//}

    // ���� ó��
	if( kPacket_.m_bWantToEnterSecretStage )
	{
		m_spRoomUserManager->AgreeEnterSecretStage( FIRST_SENDER_UID, CXSLRoom::ESS_AGREE );
	}
	else
	{
		m_spRoomUserManager->AgreeEnterSecretStage( FIRST_SENDER_UID, CXSLRoom::ESS_NOT_AGREE );
	}
	
	// ��� ������ �ǻ� ǥ�ø� �Ͽ��°�?
	CheckEnterSecretStageSuccess();
}
//}}

//{{ 2011. 10. 25	������	��Ʋ�ʵ� �ý���
//////////////////////////////////////////////////////////////////////////
#ifdef SERV_BATTLE_FIELD_SYSTEM
//////////////////////////////////////////////////////////////////////////

_IMPL_ON_FUNC( ERM_NPC_UNIT_CREATE_REQ, KEGS_NPC_UNIT_CREATE_REQ )
{
	VERIFY_ROOM_STATE_AND_WARN( ( 1, KRoomFSM::S_PLAY ), (1, KRoomFSM::S_RESULT), ERM_NPC_UNIT_CREATE_ACK, KPacketOK );

	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_NPC_UNIT_CREATE_ACK, kPacket );

	KEGS_NPC_UNIT_CREATE_NOT kPacketNot;	

	//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER
	KEGS_CREATE_ATTRIB_NPC_NOT kAttribNPCNot;
#endif SERV_INSERT_GLOBAL_SERVER
	//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�


	BOOST_TEST_FOREACH( KNPCUnitReq&, kCreateNpcInfo, kPacket_.m_vecNPCUnitReq )
	{
		KNPCUnitNot kNPCInfo;

		switch( kCreateNpcInfo.m_NPCID )
		{
		case CXSLUnitManager::NUI_EVENT_TEACHER_ECHO:
			{
				kCreateNpcInfo.m_bNoDrop = false; // ED �뷮 ��� 
				
				if( m_eDungeonType == CXSLDungeon::DT_NORMAL )
				{
					if( CXSLDungeon::IsEventDungeon( GetDungeonID() ) == true )
					{
						kCreateNpcInfo.m_Level = 1;
					}
					else
					{
						kCreateNpcInfo.m_Level = SiCXSLDungeonManager()->GetDungeonNpcLevel( GetDungeonIDAndDif() );
					}
				}
				else
				{
					kCreateNpcInfo.m_Level = GetAverageUserLevelAtDungeonStart();
				}
			}break;
		default:
			{
				//////////////////////////////////////////////////////////////////////////				
				// Ŭ���̾�Ʈ���� NoDrop = false�� �ؼ� ����� ���Ѿ��ϴ� ���Ͱ� �����Ѵ�. ���� �� �ڵ�� �ٽ� �ּ�ó����. -- ����
				//////////////////////////////////////////////////////////////////////////

				//kCreateNpcInfo.m_bNoDrop = true; // Ŭ���̾�Ʈ�� ���� ��û ������ ���ʹ� ���Ȼ� �ݵ�� NoDrop���� ó���Ѵ�!
			}break;	
		}

		kNPCInfo.m_kNPCUnitReq = kCreateNpcInfo;

#ifdef SERV_CREATED_NPC_LIMITED_DROPS
		// �ϴ��� ������ ���� ó�� // �ϴ� �����ص�
		bool bTempNoDrop = kCreateNpcInfo.m_bNoDrop;
		kCreateNpcInfo.m_bNoDrop = true;

		int iNpcDropTime = SiCXSLDungeonManager()->GetNPCDropTime( GetDungeonIDAndDif(), GetStageID(), GetSubStageID(), kCreateNpcInfo.m_NPCID );
		if( iNpcDropTime > 0 )
		{
			NPC_INFO_QUADRUPLE npcInfoQuadruple;
			npcInfoQuadruple.iDungeonID		= GetDungeonIDAndDif();
			npcInfoQuadruple.iStageID		= GetStageID();
			npcInfoQuadruple.iSubStageID	= GetSubStageID();
			npcInfoQuadruple.iNpcID			= kCreateNpcInfo.m_NPCID;

			// ��ȯ��û�Ǿ��� ���� ��ü ��������� �� �̸��� ���� ��� ����
			if( m_mapDroppedTimes[npcInfoQuadruple] < iNpcDropTime )
			{
				m_mapDroppedTimes[npcInfoQuadruple] = m_mapDroppedTimes[npcInfoQuadruple] + 1;
				kCreateNpcInfo.m_bNoDrop = bTempNoDrop;
			}
		}
#endif SERV_CREATED_NPC_LIMITED_DROPS

		// ���� ����!
		LIF( m_kDungeonMonsterManager.CreateMonster( kCreateNpcInfo, kNPCInfo.m_kNPCUnitReq.m_UID ) );
		const int iNpcUID = kNPCInfo.m_kNPCUnitReq.m_UID;

#ifdef SERV_PAYMENT_ITEM_WITH_ALLY_NPC
		if( kCreateNpcInfo.m_iAllyUID > 0 )
			m_spRoomUserManager->AddAllyNPC( kCreateNpcInfo.m_iAllyUID, kNPCInfo.m_kNPCUnitReq );
#endif SERV_PAYMENT_ITEM_WITH_ALLY_NPC

		//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER
		if( kCreateNpcInfo.m_bAttributeNpc == true &&  CXSLDungeon::IsDefenceDungeon( GetDungeonID() ) == true ) // ���潺 ����, ����Ʈ ����
		{
			KAttribNpcUnitInfo kAttribNpcInfo;
			kAttribNpcInfo.m_sNPCUnitReq = kNPCInfo.m_kNPCUnitReq;
			kAttribNpcInfo.m_sNPCUnitReq.m_UID = kNPCInfo.m_kNPCUnitReq.m_UID;

			if( SiKAttribNpcTable()->GetAttribNpcInfo( m_eDungeonType, GetDifficultyLevel(), kCreateNpcInfo.m_Level, kCreateNpcInfo.m_NPCID, kAttribNpcInfo.m_sAttribEnchantNpcInfo ) == false )
			{
				START_LOG( cerr, L"�Ӽ� NPC���� ��� ����!" )
					<< BUILD_LOG( GetDungeonID() )
					<< BUILD_LOG( m_eDungeonType )
					<< BUILD_LOGc( GetDifficultyLevel() )
					<< BUILD_LOGc( kCreateNpcInfo.m_Level )
					<< BUILD_LOG( kCreateNpcInfo.m_NPCID )
					<< END_LOG;
				continue;
			}

			kAttribNPCNot.m_vecAttribNPCUnit.push_back( kAttribNpcInfo );
			continue;
		}
#endif SERV_INSERT_GLOBAL_SERVER
		//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�

		//{{ 2013. 01. 09 ���� ���� �ý��� - ��μ�
#ifdef SERV_DUNGEON_FORCED_EXIT_SYSTEM
		if( CXSLDungeon::IsDefenceDungeon( GetDungeonID() ) == true )
		{
			const char cMonsterTypeFactor = static_cast<char>( CXSLUnitManager::GetMonsterTypeFactor( kCreateNpcInfo.m_bAttributeNpc, GetDifficultyLevel(), kCreateNpcInfo.m_cMonsterGrade ) );
			m_kBadAttitudeManager.IncreaseSubStageMonsterDieCount( GetStageID(), GetSubStageID(), cMonsterTypeFactor, true );
		}
#endif SERV_DUNGEON_FORCED_EXIT_SYSTEM
		//}
		kPacketNot.m_vecNPCUnitAck.push_back( kNPCInfo );

		//{{ 2013. 04. 15	������	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
		// ����� �� ������ ���
		if( CXSLDungeon::IsDefenceDungeon( GetDungeonID() ) == true )
		{
			// ���� ���Ͱ� ��ȣ�� ���Ͷ�� ����� �ϳ� �ø���!
			if( m_kDefenceDungeonManager.IncreaseDefenceDungeonPhase( kCreateNpcInfo.m_NPCID ) == true )
			{
				// ������ 2�ܰ谡 �ɶ����� ������ �������� ���� ����!
				switch( m_kDefenceDungeonManager.GetDefenceDungeonPhase() )
				{
				case CXSLDefenceDungeonManager::DDP_PHASE_2:
				case CXSLDefenceDungeonManager::DDP_PHASE_3:
					{
						KERM_CHANGE_DEFENCE_DUNGEON_PHASE_NOT kPacketNot;
						kPacketNot.m_iDefenceDungeonPhase = m_kDefenceDungeonManager.GetDefenceDungeonPhase();
						BroadCast( ERM_CHANGE_DEFENCE_DUNGEON_PHASE_NOT, kPacketNot );
					}
					break;
				}
			}
		}
#endif SERV_NEW_DEFENCE_DUNGEON
		//}}
	}

	BroadCast( ERM_NPC_UNIT_CREATE_NOT, kPacketNot );

	//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER
	if( kAttribNPCNot.m_vecAttribNPCUnit.size() > 0 )
	{
		BroadCast( ERM_CREATE_ATTRIB_NPC_NOT, kAttribNPCNot );
	}
#endif SERV_INSERT_GLOBAL_SERVER
	//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
}

//{{ �ʵ� ��� ���� - ��μ�
#ifdef SERV_REFORM_ITEM_DROP
_IMPL_ON_FUNC( ERM_NPC_UNIT_DIE_REQ, KEGS_NPC_UNIT_DIE_REQ )
{
	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_PLAY ), ERM_NPC_UNIT_DIE_ACK, KERM_NPC_UNIT_DIE_ACK );

	std::set< int >::iterator sit = m_setDefenseSmallCrystalNpcUID.find( kPacket_.m_nDieNPCUID );
	if( sit != m_setDefenseSmallCrystalNpcUID.end() )
	{
		kPacket.m_iOK		= NetError::NET_OK;
		kPacket.m_iNPCID	= kPacket_.m_nDieNPCUID;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_NPC_UNIT_DIE_ACK, kPacket );

		// std::set ���� ����
		m_setDefenseSmallCrystalNpcUID.erase( sit );

		START_LOG( clog, L"���潺 ���� ���� ������ ������!!" )
			<< END_LOG;
		return;
	}

	// ���Ͱ� ���� ����ִ��� üũ!
	if( m_kDungeonMonsterManager.IsMonsterAlive( kPacket_.m_nDieNPCUID ) == false )
	{
		kPacket.m_iOK = NetError::ERR_ROOM_21;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_NPC_UNIT_DIE_ACK, kPacket );
		return;
	}

	// ���Ͱ� ���� ����ִٸ� ���� ���� ��û!
	KRoomMonsterManager::NPC_DATA kDieNpcInfo;
	if( m_kDungeonMonsterManager.GetNpcData( kPacket_.m_nDieNPCUID, kDieNpcInfo ) == false )
	{
		kPacket.m_iOK = NetError::ERR_ROOM_20;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_NPC_UNIT_DIE_ACK, kPacket );
		return;
	}

	// ���� ����ִٸ� ���λ���� ���������� ����..
	LIF( m_kDungeonMonsterManager.SetMonsterDie( kPacket_.m_nDieNPCUID, kPacket_.m_uiAttUnit ) );

	kPacket.m_iOK		= NetError::NET_OK;
	kPacket.m_iNPCID	= kDieNpcInfo.m_iNPCID;
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_NPC_UNIT_DIE_ACK, kPacket );

#ifdef SERV_PAYMENT_ITEM_WITH_ALLY_NPC
	if( kPacket_.m_cNpcDieState != KEGS_NPC_UNIT_DIE_REQ::NDS_KILL_SELF )
		m_spRoomUserManager->DeleteAllyNPC( kPacket_.m_nDieNPCUID );
#endif SERV_PAYMENT_ITEM_WITH_ALLY_NPC

	//{{ 2013. 04. 09	������	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
	// ���� ���Ͱ� ���� ���Ͷ�� ����� �� ���� ���� üũ
	if( kDieNpcInfo.m_cMonsterGrade == CXSLUnitManager::MG_BOSS_NPC  &&  CXSLDungeon::IsDefenceDungeon( GetDungeonID() ) == false )
	{
		// ���� ��� ���� ���Ͱ� �׾��°�?
		if( m_kDungeonMonsterManager.IsAllBossMonstersDead() == true )
		{
			// ����� �� ���� Ȯ�� �̺�Ʈ
			float fAddDefenseDungeonOpenRate = 0.0f;
			SiKGameEventManager()->GetDefenseDungeonOpenRate( fAddDefenseDungeonOpenRate );

			// �� ��Ƽ���� ĳ���� ���� ������ �־�� �Ѵ�.
			std::map< UidType, int > mapUnitLevelList;
			m_spRoomUserManager->GetUnitLevelList( mapUnitLevelList );

			// ����� �� ���� ����!
			//{{ 2013. 04. 29	������	����� �� ���� ���� ����
#ifdef SERV_MODIFY_DEFENCE_DUNGEON_ENTER_LOGIC_IN_SECRET_DUNGEON
#ifdef SERV_NEW_DEFENCE_DUNGEON_ENTER_100_PERCENT_EVENT
			bool b100PercentCheck = false;

			IF_EVENT_ENABLED( CEI_NEW_DEFENCE_DUNGEON_ENTER_100_PERCENT_EVENT )
			{
				b100PercentCheck = true;
			}
			if( m_kDefenceDungeonManager.DecisionEnterDefenceDungeon( GetDungeonID(), kDieNpcInfo.m_cLevel, mapUnitLevelList, fAddDefenseDungeonOpenRate, b100PercentCheck ) == true )
#else //SERV_NEW_DEFENCE_DUNGEON_ENTER_100_PERCENT_EVENT
			if( m_kDefenceDungeonManager.DecisionEnterDefenceDungeon( GetDungeonID(), kDieNpcInfo.m_cLevel, mapUnitLevelList, fAddDefenseDungeonOpenRate ) == true )
#endif //SERV_NEW_DEFENCE_DUNGEON_ENTER_100_PERCENT_EVENT
#else
			if( m_kDefenceDungeonManager.DecisionEnterDefenceDungeon( GetDungeonID(), mapUnitLevelList, fAddDefenseDungeonOpenRate ) == true )
#endif SERV_MODIFY_DEFENCE_DUNGEON_ENTER_LOGIC_IN_SECRET_DUNGEON
			//}}			
			{
				// ����� ��� �������� ����� �� ���� ������ �˸���!
				KEGS_DECISION_ENTER_DEFENCE_DUNGEON_NOT kPacketNot;
				kPacketNot.m_DieBossPos = kPacket_.m_DiePos;
				BroadCast( ERM_DECISION_ENTER_DEFENCE_DUNGEON_NOT, kPacketNot );
			}
		}
	}
#endif SERV_NEW_DEFENCE_DUNGEON
	//}}

	//{{ 2009. 5. 11  ������	npc��������
	const bool bDropDieState = IsDropDieState( kPacket_.m_cNpcDieState );
	const bool bAddRoomUserKillCount = ( bDropDieState  &&  kPacket_.m_cNpcDieState != KEGS_NPC_UNIT_DIE_REQ::NDS_KILL_SELF );
	//}}

	const CXSLUnitManager::NPCUnitTemplet* pNPCTemplet = SiCXSLUnitManager()->GetNPCUnitTemplet( (CXSLUnitManager::NPC_UNIT_ID)kDieNpcInfo.m_iNPCID );
	if( pNPCTemplet == NULL )
	{
		START_LOG( cwarn, L"NPC Unit Templet �б� ����.!" )
			<< BUILD_LOG( kDieNpcInfo.m_iNPCID )
			<< END_LOG;
		return;
	}

	//////////////////////////////////////////////////////////////////////////	
	{
		std::map< int, int >::const_iterator mitTDM = m_mapTimeDropEventMonster.find( kDieNpcInfo.m_iNPCID );
		if( mitTDM != m_mapTimeDropEventMonster.end() )
		{
			KDBE_UPDATE_EVENT_MONSTER_KILL_INFO_NOT kPacketToLog;
			kPacketToLog.m_iEventUID = mitTDM->second;
			kPacketToLog.m_iKillUnitUID = kPacket_.m_uiAttUnit;
			kPacketToLog.m_tNpcDieDate = CTime::GetCurrentTime().GetTime();
			SendToLogDB( DBE_UPDATE_EVENT_MONSTER_KILL_INFO_NOT, kPacketToLog );

			// �̺�Ʈ ���� ������ �ʱ�ȭ �մϴ�.
			m_mapTimeDropEventMonster.clear();
		}
	}
	//////////////////////////////////////////////////////////////////////////	
	if( m_spRoomUserManager->GetNumMember() == 1 )
	{
		switch( kDieNpcInfo.m_iNPCID )
		{
		case CXSLUnitManager::NUI_DUNGEON_CHECKER1:
		case CXSLUnitManager::NUI_DUNGEON_CHECKER2:
		case CXSLUnitManager::NUI_DUNGEON_CHECKER3:
		case CXSLUnitManager::NUI_DUNGEON_CHECKER4:
		case CXSLUnitManager::NUI_DUNGEON_CHECKER5:
			{
				// �������� ���� �����Ű�� ���� ���� �س���!
				m_setFindedHackUserUnitUID.insert( FIRST_SENDER_UID );

				// ��Ƽ�忡�� ��ŷüũ��Ŷ�� ������!
				KEGS_SERVER_CHECK_HACK_USER_REQ kNot;
				kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_DUNGEON_CANT_KILL_NPC_FIND;
				SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SERVER_CHECK_HACK_USER_NOT, kNot );
#ifdef SERV_MINIMUM_LOG_HAKC_USER
				START_LOG( cout, L"Cant Kill Npc pos3. UID : " << FIRST_SENDER_UID );
#else //SERV_MINIMUM_LOG_HAKC_USER
				START_LOG( cout, L"Cant Kill Npc ��ŷ������ �ǴܵǾ� üũ ����� Ŭ���̾�Ʈ �����Ͽ���!" )
					<< BUILD_LOG( LAST_SENDER_UID )
					<< BUILD_LOG( FIRST_SENDER_UID )
					<< BUILD_LOG( kPacket_.m_nDieNPCUID )
					<< BUILD_LOGc( kPacket_.m_cAttUnitType )
					<< BUILD_LOG( kPacket_.m_uiAttUnit )
					<< BUILD_LOG( kPacket_.m_uiMDAttUnit )
					<< BUILD_LOG( kPacket_.m_DiePos.x )
					<< BUILD_LOG( kPacket_.m_DiePos.y )
					<< BUILD_LOG( kPacket_.m_DiePos.z )
					<< BUILD_LOGc( kPacket_.m_cNpcDieState );
#endif //SERV_MINIMUM_LOG_HAKC_USER
			}
			break;
		}
	}	
	//////////////////////////////////////////////////////////////////////////	

	//////////////////////////////////////////////////////////////////////////
	if( SiKGameSysVal()->GetSubStageNpcDieCheck() == true )
	{
		// ���� �ΰ�? // ��Ƽ�� NPC �� 5���� �̻��ΰ�?
		if( m_spRoomUserManager->GetNumMember() == 1 
			&& m_iSubStageActiveNpc >= 5 
			&& kDieNpcInfo.m_bActive == true )
		{
			++m_iSubStageKillActiveNpc;

			if( m_iSubStageKillActiveNpc == 1 )
			{
				// ���� ������������ ó�� ��Ƽ�� ���͸� ���� �ð�
				m_tSubStageFirstKillTime = CTime::GetCurrentTime();
			}
			else
			{
				if( m_iSubStageActiveNpc <= m_iSubStageKillActiveNpc ) // ������ ���͸� ��Ҵ�!
				{
					CTime tLastKillTime = CTime::GetCurrentTime();
					CTimeSpan tSpan = CTimeSpan( 0, 0, 0, 1);
					if( tLastKillTime <= m_tSubStageFirstKillTime + tSpan ) 		// ó�� ���͸� ���� �ð����� 1������ �ۿ� �ɸ��� �ʾҴ�
					{
						// ��Ƽ�忡�� ��ŷüũ��Ŷ�� ������!
						KEGS_SERVER_CHECK_HACK_USER_REQ kNot;
						kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_CHECK_NPC_DIE_TIME;
						SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SERVER_CHECK_HACK_USER_NOT, kNot );

						START_LOG( cout, L"Sub Stage�� ��� Npc�� ���� �ð��� 1�ʹۿ� �ɸ��� �ʴ´�." )
							<< BUILD_LOG( LAST_SENDER_UID )
							<< BUILD_LOG( FIRST_SENDER_UID )
							<< BUILD_LOG( m_tSubStageFirstKillTime.GetMinute() )
							<< BUILD_LOG( m_tSubStageFirstKillTime.GetSecond() )
							<< BUILD_LOG( tLastKillTime.GetMinute() )
							<< BUILD_LOG( tLastKillTime.GetSecond() )
							<< BUILD_LOG( m_iSubStageActiveNpc )
							<< BUILD_LOG( m_iSubStageKillActiveNpc )
							<< END_LOG;
					}
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////
	// �Ӽ� �������� ���� �˻�	
	int iNpcLv = 0;
	const bool bIsAttribNpc = IsAttribNpc( kPacket_.m_nDieNPCUID, iNpcLv );

	// ���� Ÿ�� ��� ���
	const char cMonsterTypeFactor = static_cast<char>( CXSLUnitManager::GetMonsterTypeFactor( bIsAttribNpc, GetDifficultyLevel(), kDieNpcInfo.m_cMonsterGrade ) );

	// ��Ƽ �÷����� ��� ������ ��� Ȯ�� ���
	const float fPartyFactor = GetDungeonPartyBonusRate() + 1.0f;
	// ���� ��� ��� Ȯ�� ���
	const float fDungeonFactor = static_cast<float>(cMonsterTypeFactor);
	//////////////////////////////////////////////////////////////////////////
	// uiAttUnit���� -1�̸� ������ ������ ���� �ʰ� npc�� �ڻ��Ѱ���. [��:��������]

	// �÷��̾ ���͸� ���ΰŶ�� �ش� �÷��̾��� ų���ͼ� üũ
	if( pNPCTemplet->m_bIsNPC  &&  bAddRoomUserKillCount == true  &&  kPacket_.m_uiAttUnit != -1 )
	{
		if( m_spRoomUserManager->AddKillNPC( kPacket_.m_uiAttUnit ) == false )
		{
			if( m_kDungeonMonsterManager.IsExistMonster( static_cast<int>(kPacket_.m_uiAttUnit) ) == true )
			{
				START_LOG( clog, L"������ �׿���!!" )
					<< BUILD_LOG( kDieNpcInfo.m_iNPCID )
					<< BUILD_LOG( kPacket_.m_uiAttUnit );

				// ���Ͱ� ������ �ɷ� �״°���� : �������� �״�� ���ó����.
			}
			else
			{
				// ���� ���� ������ ��ÿ� ���� ���� ���� ���� ���� ��� 1���� ��츸 üũ!
				if( m_iStartedNumMember == 1  &&  m_spRoomUserManager->GetNumMember() == 1 )
				{
					START_LOG( cerr, L"������ ���ΰ� �ƴϳ�? ��ŷ������ �ǽɵȴ�!" )
						<< BUILD_LOG( LAST_SENDER_UID )
						<< BUILD_LOG( FIRST_SENDER_UID )
						<< BUILD_LOG( kPacket_.m_nDieNPCUID )
						<< BUILD_LOGc( kPacket_.m_cAttUnitType )
						<< BUILD_LOG( kPacket_.m_uiAttUnit )
						<< BUILD_LOG( kPacket_.m_uiMDAttUnit )
						<< BUILD_LOG( kPacket_.m_DiePos.x )
						<< BUILD_LOG( kPacket_.m_DiePos.y )
						<< BUILD_LOG( kPacket_.m_DiePos.z )
						<< BUILD_LOGc( kPacket_.m_cNpcDieState )
						<< BUILD_LOG( kDieNpcInfo.m_iNPCID )
						<< END_LOG;

					// ������ üũ ī��Ʈ ����!
					++m_iNpcDieHackCount;

					// �� ������ 5ȸ �̻� �߻��Ѵٸ� �װ��� �߸��� ���̴�!
					if( m_iNpcDieHackCount == m_iHackUserCheckCount )
					{
						// ��Ƽ�忡�� ��ŷüũ��Ŷ�� ������!
						KEGS_SERVER_CHECK_HACK_USER_REQ kNot;
						kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_DUNGEON_NPC_UNIT_DIE;
						SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SERVER_CHECK_HACK_USER_NOT, kNot );

						START_LOG( cout, L"NpcDie ��ŷ������ �ǴܵǾ� üũ ����� Ŭ���̾�Ʈ �����Ͽ���!" )
							<< BUILD_LOG( LAST_SENDER_UID )
							<< BUILD_LOG( FIRST_SENDER_UID )
							<< BUILD_LOG( m_iNpcDieHackCount );
					}
				}
			}
		}
	}	

	//////////////////////////////////////////////////////////////////////////
	if( kDieNpcInfo.m_bNoDrop == false  &&  bDropDieState == true )
	{
		if( pNPCTemplet->m_bIsNPC == true  &&  kDieNpcInfo.m_bActive == true )
		{
			m_kDungeonMonsterManager.IncreaseMonsterDieCount( cMonsterTypeFactor );
			//{{ 2013. 01. 09 ���� ���� �ý��� - ��μ�
#ifdef SERV_DUNGEON_FORCED_EXIT_SYSTEM
			if( CXSLDungeon::IsDefenceDungeon( GetDungeonIDAndDif() ) == false )
			{
				m_kBadAttitudeManager.IncreaseSubStageMonsterDieCount( GetStageID(), GetSubStageID(), cMonsterTypeFactor, false );
			}
#endif SERV_DUNGEON_FORCED_EXIT_SYSTEM
			//}
		}

		// ����� �ο����� �˷��� ������ ���� ����..
		KERM_NPC_UNIT_DIE_NOT kNpcDietNot;
		kNpcDietNot.m_iDungeonID		 = GetDungeonID();
		kNpcDietNot.m_cDifficulty		 = GetDifficultyLevel();
		kNpcDietNot.m_nDieNPCUID		 = kPacket_.m_nDieNPCUID;
		kNpcDietNot.m_iNPCID			 = kDieNpcInfo.m_iNPCID;
		kNpcDietNot.m_cAttUnitType		 = kPacket_.m_cAttUnitType;
		kNpcDietNot.m_uiAttUnit			 = kPacket_.m_uiAttUnit;
		kNpcDietNot.m_cUserCount		 = static_cast<char>( m_spRoomUserManager->GetNumMember() );
		kNpcDietNot.m_bNoDrop			 = kDieNpcInfo.m_bNoDrop;
		kNpcDietNot.m_sSubStageExpNpcNum = GetSubStageExpNpcNum( GetSubStageID() );
		kNpcDietNot.m_bDecreaseEndurance = false;
		kNpcDietNot.m_cDungeonMode		 = m_cDungeonMode;
		kNpcDietNot.m_EXPList.clear();
		kNpcDietNot.m_bQuestComplete	 = true;

		// ��Ƽ �⿩��
		float fPartyContribution = 0.f;
		float fPartyTotalDamage = 0.f;

		std::map< UidType, float >::iterator mitParty = kPacket_.m_mapDamageByUser.begin();
		for( ; mitParty != kPacket_.m_mapDamageByUser.end() ; ++mitParty )
		{
			fPartyTotalDamage += mitParty->second;
		}
		
		fPartyContribution = fPartyTotalDamage / kPacket_.m_iNpcMaxHP;

		if( fPartyContribution < 1.f )
			fPartyContribution = 1.f;

		if( fPartyContribution > 1.5f )
			fPartyContribution = 1.5f;

		// ���� �⿩��
		// ������ ��Ƽ �⿩���� ���� �⿩��
		const float fContribution = fPartyContribution;
		
		// ����ִ� ��Ƽ�� ��
		int iPartyUserCount = m_spRoomUserManager->GetLiveMember();

		// �������� EXP, ED, DropItem ���
		std::vector< UidType > vecLiveMember;
		m_spRoomUserManager->GetLiveMemberList( vecLiveMember );

		BOOST_TEST_FOREACH( UidType, iPartyUserUID, vecLiveMember )
		{
			KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser( iPartyUserUID );
			if( IS_NULL( spRoomUser ) )
			{
				START_LOG( cerr, L"���� �����Ͱ��� �̻��մϴ�" )
					<< BUILD_LOG( iPartyUserUID )
					<< END_LOG;
				continue;
			}

			// ���� factor�� ���ϱ�
			const float fLevelFactor = CXSLDungeon::GetMonsterLevelBalanceRate( spRoomUser->GetLevel(), kDieNpcInfo.m_cLevel );

			// ���� ������ Ȯ���� ������ ���
			KDropTable::DROP_DATA sDropDataNpc;

			//////////////////////////////////////////////////////////////////////////			
			// �����۰� �⺻ ����ġ �� ���
			switch( GetDungeonType() )
			{
			case CXSLDungeon::DT_HENIR:
				{
					//{{ 2012. 12. 16  ��� �̺�Ʈ - ��μ�
#ifdef SERV_ITEM_DROP_EVENT
					if( SiKDropTable()->HenirNpcDropItem( static_cast<CXSLDungeon::DUNGEON_MODE>(m_cDungeonMode), 
						kDieNpcInfo.m_iNPCID,
						kDieNpcInfo.m_cLevel,
						fDungeonFactor,
						fContribution,
						fPartyFactor,
						iPartyUserCount,
						m_iItemDropEventProbCount,
						kNpcDietNot.m_bDecreaseEndurance,
						sDropDataNpc,
						kDieNpcInfo.m_bIsBoss ) == false )
#else
					if( SiKDropTable()->HenirNpcDropItem( static_cast<CXSLDungeon::DUNGEON_MODE>(m_cDungeonMode), 
						kDieNpcInfo.m_iNPCID,
						kDieNpcInfo.m_cLevel,
						fDungeonFactor,
						fContribution,
						fPartyFactor,
						iPartyUserCount,
						kNpcDietNot.m_bDecreaseEndurance,
						sDropDataNpc,
						kDieNpcInfo.m_bIsBoss ) == false )
#endif SERV_ITEM_DROP_EVENT
					//}}
					{
						START_LOG( cerr, L"Henir Drop Item Data ������ ���� ===" )
							<< BUILD_LOG( kPacket_.m_nDieNPCUID )
							<< BUILD_LOG( kDieNpcInfo.m_iNPCID )
							<< BUILD_LOGc( m_cDungeonMode )
							<< END_LOG;
					}

					// ������̺��� EXP�� ���õǾ��ִ� ���Ͷ�� ������ ���� üũ�� �Ѵ�.
					if( sDropDataNpc.m_iExp > 0 )
					{
						sDropDataNpc.m_iExp = 0; // ��ϸ� �ð��� ����ġ ȹ���� �ϸ� �ȵǱ� ������ 0���� �����Ѵ�.
					}
				}
				break;

			case CXSLDungeon::DT_SECRET:
				{
					// EXTRA STAGE�� ���..
					if( IsEnteredExtraStage() == true )
					{
						if( SiKDropTable()->ExtraStageNpcDropItem( kDieNpcInfo.m_iNPCID,
							kDieNpcInfo.m_cLevel,
							fLevelFactor,
							fDungeonFactor,
							fContribution,
							fPartyFactor,
							iPartyUserCount,
							//{{ 2012. 12. 16  ��� �̺�Ʈ - ��μ�
#ifdef SERV_ITEM_DROP_EVENT
							m_iItemDropEventProbCount,
#endif SERV_ITEM_DROP_EVENT
							//}}
							kNpcDietNot.m_bDecreaseEndurance,
							sDropDataNpc,
							kDieNpcInfo.m_bIsBoss ) == false )
						{
							START_LOG( cerr, L"Henir Drop Item Data ������ ���� ===" )
								<< BUILD_LOG( kPacket_.m_nDieNPCUID )
								<< BUILD_LOG( kDieNpcInfo.m_iNPCID )
								<< END_LOG;
						}
					}
					else
					{
						if( SiKDropTable()->NormalNpcDropItem( GetDungeonIDAndDif(), 
							kDieNpcInfo.m_iNPCID,
							kDieNpcInfo.m_cLevel,
							fLevelFactor,
							fDungeonFactor,
							fContribution,
							fPartyFactor,
							iPartyUserCount,
							//{{ 2012. 12. 16  ��� �̺�Ʈ - ��μ�
#ifdef SERV_ITEM_DROP_EVENT
							m_iItemDropEventProbCount,
#endif SERV_ITEM_DROP_EVENT
							//}}
							kNpcDietNot.m_bDecreaseEndurance,
							sDropDataNpc,
							kDieNpcInfo.m_bIsBoss ) == false )
						{
							START_LOG( cerr, L"Drop Item Data ������ ���� ===" )
								<< BUILD_LOG( kPacket_.m_nDieNPCUID )
								<< BUILD_LOG( kDieNpcInfo.m_iNPCID )
								<< BUILD_LOGc( GetDifficultyLevel() )
								<< END_LOG;
						}
					}
				}
				break;	

			default:
				{
					if( SiKDropTable()->NormalNpcDropItem( GetDungeonIDAndDif(), 
						kDieNpcInfo.m_iNPCID,
						kDieNpcInfo.m_cLevel,
						fLevelFactor,
						fDungeonFactor,
						fContribution,
						fPartyFactor,
						iPartyUserCount,
						//{{ 2012. 12. 16  ��� �̺�Ʈ - ��μ�
#ifdef SERV_ITEM_DROP_EVENT
						m_iItemDropEventProbCount,
#endif SERV_ITEM_DROP_EVENT
						//}}
						kNpcDietNot.m_bDecreaseEndurance,
						sDropDataNpc,
						kDieNpcInfo.m_bIsBoss ) == false )
					{
						START_LOG( cerr, L"Drop Item Data ������ ���� ===" )
							<< BUILD_LOG( kPacket_.m_nDieNPCUID )
							<< BUILD_LOG( kDieNpcInfo.m_iNPCID )
							<< BUILD_LOGc( GetDifficultyLevel() )
							<< END_LOG;
					}
				}
				break;
			}

			//{{ 2012. 11. 12	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
			// ���� ����, ������ ����, �߰�����, ���� ����� ���
			SiKDropTable()->NpcSpecialDropItem( kDieNpcInfo.m_iNPCID, kDieNpcInfo.m_cMonsterGrade, kPacket_.m_cNpcDieState, sDropDataNpc.m_vecItemID );
#else
//			//{{ 2009. 5. 18  ������	��������
//			switch( kDieNpcInfo.m_iNPCID )
//			{
//			case CXSLUnitManager::NUI_MANA_EATER:
//			case CXSLUnitManager::NUI_HALLOWEEN_MANA_EATER:
//			case CXSLUnitManager::NUI_DEFENCE_MANA_EATER:
//				{
//					if( kPacket_.m_cNpcDieState == KEGS_NPC_UNIT_DIE_REQ::NDS_BONUS_DROP )
//					{
//						// ��� ���̺��� Ȯ���ؼ� 'Special Drop'������� ��ü�ҿ���
//						sDropDataNpc.m_vecItemID.push_back( 70055 ); // 70055
//					}
//				}
//				break;
//
//			case CXSLUnitManager::NUI_BLOOD_EATER:
//			case CXSLUnitManager::NUI_DEFENCE_BLOOD_EATER:
//				{
//					if( kPacket_.m_cNpcDieState == KEGS_NPC_UNIT_DIE_REQ::NDS_BONUS_DROP )
//					{
//						// ��� ���̺��� Ȯ���ؼ� 'Special Drop'������� ��ü�ҿ���
//						sDropDataNpc.m_vecItemID.push_back( 70056 ); // 70055
//					}
//				}
//				break;
//			}		
//			//}}
//
//			//{{ ����, �ߺ� ���� �� ������ ��� �Ǵ� ���(MonsterGrade ����) - ��μ�
//#ifdef SERV_KILL_BOSS_MIDDLE_BOSS_DROP_ITEM
//			
//			if( kDieNpcInfo.m_cMonsterGrade == CXSLUnitManager::MG_MIDDLE_BOSS_NPC )
//			{
//				// ��� ���̺��� Ȯ���ؼ� 'Special Drop'������� ��ü�ҿ���
//				if( rand() % 100 < 50 )	// �ߺ����� 50% Ȯ���θ� ���
//				{
//					sDropDataNpc.m_vecItemID.push_back( 70057 ); // 70057
//				}
//			}
//			else if( kDieNpcInfo.m_cMonsterGrade == CXSLUnitManager::MG_BOSS_NPC )
//			{
//				// ��� ���̺��� Ȯ���ؼ� 'Special Drop'������� ��ü�ҿ���
//				sDropDataNpc.m_vecItemID.push_back( 70058 ); // 70057
//			}
//#endif SERV_KILL_BOSS_MIDDLE_BOSS_DROP_ITEM
//			//}	
#endif SERV_BATTLE_FIELD_SYSTEM
			//}}

			//////////////////////////////////////////////////////////////////////////		
			//if( sDropDataNpc.m_iExp > 0 )
			{
				m_spRoomUserManager->AddDungeonRewardEXP( spRoomUser->GetCID(), sDropDataNpc.m_iExp, GetDungeonIDAndDif(), GetDifficultyLevel(), fLevelFactor, kNpcDietNot );
			}

			//{{ 2012. 11. 15	������		�ǹ� ���� ���� ����
#ifdef SERV_FEVER_INCREASE_BUG_FIX
			//////////////////////////////////////////////////////////////////////////
			// �������� ��� ���ʹ� ����ġ�� �ִ� ���� �̹Ƿ� ��Ƽ fever�� ������Ű��!
			if( kNpcDietNot.m_bDecreaseEndurance )
			{
				// �ѹ��� �÷��־�� �Ѵ�.
				if( spRoomUser->GetCID() == FIRST_SENDER_UID )
				{
					IncreasePartyFever(); // ����ġ ������ �ִ� ���͸� ���ΰ�� party fever��ġ�� �ø���!
				}
			}
#endif SERV_FEVER_INCREASE_BUG_FIX
			//}}

			//////////////////////////////////////////////////////////////////////////
			// ���� ��Ƽ���鿡�� ���� ��� �Ǵ� ���� ���� ( ����ġ ���� ���� )
			SendToGSCharacter( spRoomUser->GetGSUID(), spRoomUser->GetCID(), ERM_NPC_UNIT_DIE_NOT, kNpcDietNot );

			// ����� ���� �ʴ� npc��� ����� ���� �ʴ´�.
			if( kDieNpcInfo.m_bNoDrop == true )
				continue;

			// ��ӻ��°� �ƴ�ä�� ����npc��� ������ ����� ���� �ʴ´�.
			if( bDropDieState == false )
				continue;

			// ���� �⿩�� * ��Ƽ ���ʽ�
			const float fTotalDropRate = fContribution;// * fPartyFactor;

			//////////////////////////////////////////////////////////////////////////
			// 2. ������ �������� ��� �������� �����Ǵ� ���� ���
			KDropTable::DROP_DATA sDropDataStatic;

			if( SiKDropTable()->StaticDropItem( GetDungeonIDAndDif(), sDropDataStatic, fTotalDropRate ) == false )
			{
				START_LOG( cerr, L"Drop Item Data ������ ���� ===" )
					<< BUILD_LOG( kPacket_.m_nDieNPCUID )
					<< BUILD_LOG( kDieNpcInfo.m_iNPCID )
					<< BUILD_LOG( GetDungeonID() )
					<< BUILD_LOGc( GetDifficultyLevel() )
					<< END_LOG;
			}

			//////////////////////////////////////////////////////////////////////////	
			// 3. �̺�Ʈ ������ ��� üũ
			KDropTable::DROP_DATA sDropDataEvent;

			if( kDieNpcInfo.m_bActive == true )
			{
				// �̺�Ʈ �������� �ʽ��ڽ�, Ʃ�丮��, �Ʒüҿ��� ��ӵǼ��� �ȵȴ�!
				if( CXSLDungeon::DI_EL_FOREST_GATE_NORMAL != GetDungeonID()  &&
					CXSLDungeon::IsTutorialDungeon( GetDungeonID() ) == false &&
					CXSLDungeon::IsTCDungeon( GetDungeonID() ) == false )
				{
					if( SiKDropTable()->EventDropItem( fTotalDropRate, sDropDataEvent ) == false )
					{
						START_LOG( cerr, L"Event Drop Item Data ������ ���� ===" )
							<< BUILD_LOG( kPacket_.m_nDieNPCUID )
							<< BUILD_LOG( kDieNpcInfo.m_iNPCID )
							<< BUILD_LOGc( GetDifficultyLevel() )
							<< END_LOG;
					}
				}
			}

			//////////////////////////////////////////////////////////////////////////
			// 4. �Ӽ� ���� ��� ������
			KDropTable::DROP_DATA sDropDataAttribNpc;

			if( bIsAttribNpc == true  /*&&  bDropDieState == true*/ )
			{
				char cDifficultyLevel = GetDifficultyLevel();

				// ��д����ΰ�?
				if( m_eDungeonType == CXSLDungeon::DT_SECRET )
				{
					if( m_cDungeonMode == CXSLDungeon::DM_SECRET_NORMAL )
					{
						cDifficultyLevel = CXSLDungeon::DL_HARD;
					}
					else if( m_cDungeonMode == CXSLDungeon::DM_SECRET_HELL )
					{
						// Extra Stage�ΰ�?
						if( IsEnteredExtraStage() == true )
						{
							cDifficultyLevel = CXSLDungeon::DL_NORMAL;
						}
						else
						{
							cDifficultyLevel = CXSLDungeon::DL_EXPERT;
						}
					}
				}

				//{{ 2012. 12. 16  ��� �̺�Ʈ - ��μ�
#ifdef SERV_ITEM_DROP_EVENT
				if( SiKAttribNpcTable()->AttribNpcDropItem( static_cast<int>(cDifficultyLevel), iNpcLv, fTotalDropRate, m_iItemDropEventProbCount, sDropDataAttribNpc.m_vecItemID ) == false )
#else
				if( SiKAttribNpcTable()->AttribNpcDropItem( static_cast<int>(cDifficultyLevel), iNpcLv, fTotalDropRate, sDropDataAttribNpc.m_vecItemID ) == false )
#endif SERV_ITEM_DROP_EVENT
				//}}
				{
					START_LOG( cerr, L"�Ӽ� ���� ������ ��� ����!" )
						<< BUILD_LOGc( cDifficultyLevel )
						<< BUILD_LOG( iNpcLv )
						<< END_LOG;
				}

				switch( cDifficultyLevel )
				{
				case CXSLDungeon::DL_NORMAL:
				case CXSLDungeon::DL_HARD:
					{
						sDropDataNpc.m_nGPNum = 2 * sDropDataNpc.m_nGPNum;
					}
					break;

				case CXSLDungeon::DL_EXPERT:
					{
						sDropDataNpc.m_nGPNum = 3 * sDropDataNpc.m_nGPNum;
					}
					break;

				default:
					START_LOG( cerr, L"���� ���̵��� �̻��մϴ�." )
						<< BUILD_LOGc( cDifficultyLevel )
						<< END_LOG;
					break;
				}

				if( SiKDropTable()->AttribNpcSpecialDrop( cDifficultyLevel, fTotalDropRate, sDropDataAttribNpc.m_vecItemID ) == false )
				{
					START_LOG( cerr, L"�Ӽ� ���� ����� ������ ��� ����!" )
						<< BUILD_LOGc( cDifficultyLevel )
						<< END_LOG;
				}
			}	

			//////////////////////////////////////////////////////////////////////////
			// 5. ����Ʈ ��� ������ ó��

			std::vector< KRoomUser::QUEST_ITEM_DROP_INFO > vecDropInfo;

			// �ش� ĳ���Ͱ� ����Ʈ ��� �������� �ִ��� �˻�..
			//{{ 2013. 03. 18	 ����Ʈ ��� Ȯ�� ���� �̺�Ʈ - ��μ�
#ifdef SERV_QUEST_ITEM_DROP_EVENT
			if( m_spRoomUserManager->GetQuestDropItem( spRoomUser->GetCID(), GetDungeonID(), GetDifficultyLevel(), m_cDungeonMode, kDieNpcInfo.m_iNPCID, vecDropInfo, m_fQuestItemDropEventProbRate ) == true )
#else
			if( m_spRoomUserManager->GetQuestDropItem( spRoomUser->GetCID(), GetDungeonID(), GetDifficultyLevel(), m_cDungeonMode, kDieNpcInfo.m_iNPCID, vecDropInfo ) == true )
#endif SERV_QUEST_ITEM_DROP_EVENT
			//}
			{
				BOOST_TEST_FOREACH( const KRoomUser::QUEST_ITEM_DROP_INFO&, kDropInfo, vecDropInfo )
				{
					//��ӵ� ������ ����ŭ ó���� ����..
					BOOST_TEST_FOREACH( const int, iDropQuestItemID, kDropInfo.m_vecDropQuestItem )
					{
						KEGS_GET_ITEM_NOT kDropNot;
						kDropNot.m_cGetItemType = KEGS_GET_ITEM_NOT::GIT_QUEST_ITEM;
						kDropNot.m_GetUnitUID	= spRoomUser->GetCID();
						kDropNot.m_iItemID		= iDropQuestItemID;
						kDropNot.m_iDropItemUID = m_nDropItemUID++;
						SendToGSCharacter( spRoomUser->GetGSUID(), spRoomUser->GetCID(), ERM_GET_ITEM_INSERT_TO_INVENTORY_NOT, kDropNot );
					}
				}
			}
			else
			{
				if( NetError::GetLastError() != NetError::NET_OK )
				{
					START_LOG( cerr, L"����Ʈ ��� ������ �޾ƿ��� ����.!" )
						<< BUILD_LOG( kPacket_.m_uiAttUnit )
						<< BUILD_LOG( m_iDungeonID )
						<< BUILD_LOG( NetError::GetLastErrMsg() )
						<< END_LOG;
				}
			}

			//////////////////////////////////////////////////////////////////////////
			// 6. ��ϸ� �ð� ����� ��� ó��
			KDropTable::DROP_DATA sDropDataHenir;

			if( CXSLDungeon::IsHenirDungeon( GetDungeonID() ) == true )
			{
				//{{ �ʵ� ��� ���� - ��μ�
#ifdef SERV_REFORM_ITEM_DROP
				SiKDropTable()->GetHenirSpecialNpcDropInfo( kDieNpcInfo.m_iNPCID, fTotalDropRate, sDropDataHenir );
#else
				SiKDropTable()->GetHenirSpecialNpcDropInfo( kDieNpcInfo.m_iNPCID, sDropDataHenir );
#endif SERV_REFORM_ITEM_DROP
					//}}
			}

			//////////////////////////////////////////////////////////////////////////	
			// ������ ������ ��� ������ ����Ʈ�� ���� �������� ������ ���� ó��
			KEGS_DROP_ITEM_NOT kDropItemNot;
			kDropItemNot.m_CreatePos = kPacket_.m_DiePos;

			// ��ϸ� �����̶�� ��� ������ ���������� Ȯ�� ����.
			if( CXSLDungeon::IsHenirDungeon( GetDungeonID() ) == true )
			{
				// ���� ���� �� �ִ� ������ ����.
				if( spRoomUser->IsHenirRewardUser() == false )
				{
					CheckDeleteHenirDropItem( sDropDataNpc, sDropDataStatic, sDropDataEvent, sDropDataAttribNpc, sDropDataHenir );
				}
			}

			if( spRoomUser->ExistActiveBuff( CXSLBuffManager::BTI_BUFF_RETURN_OF_HERO ) == true )
			{
				if( m_cRoomType == CXSLRoom::RT_DUNGEON )
				{
					if( CXSLDungeon::IsHenirDungeon( GetDungeonID() )  == true )
					{
						if( spRoomUser->IsHenirRewardUser() == false )
						{
							if( SiKDropTable()->GetComeBackBuffDropCount( m_spRoomUserManager->GetNumMember() ) == true )
							{
								// ��� ���̺��� Ȯ���ؼ� 'Special Drop'������� ��ü�ҿ���
								kDropItemNot.m_DropItemDataList.push_back( CreateItemProcess( 127260 ) );		// ���ο� ������ ¡ǥ
							}
						}
					}
					else
					{
						if( SiKDropTable()->GetComeBackBuffDropCount( m_spRoomUserManager->GetNumMember() ) == true )
						{
							// ��� ���̺��� Ȯ���ؼ� 'Special Drop'������� ��ü�ҿ���
							kDropItemNot.m_DropItemDataList.push_back( CreateItemProcess( 127260 ) );		// ���ο� ������ ¡ǥ
						}
					}
				}
			}
			
			//1.ED ���ó��
			if( sDropDataNpc.m_iED != -1 )
			{
				for( int i = 0; i < sDropDataNpc.m_nGPNum; ++i )
				{
					kDropItemNot.m_DropItemDataList.push_back( CreateItemProcess( sDropDataNpc.m_iGPItemID, true, sDropDataNpc.m_iED ) );
				}
			}

			//2.NPC ITEM ���ó��	
			if( sDropDataNpc.m_vecItemID.empty() == false )
			{
				BOOST_TEST_FOREACH( const int, iDropItemID, sDropDataNpc.m_vecItemID )
				{
					// Ư�� �������� Ư���������� ��ӵ��� �ʵ��� �ϴ� �ڵ�
					if( !CXSLDungeon::IsItemDropDungeon( iDropItemID, m_iDungeonID ) )
						continue;

					kDropItemNot.m_DropItemDataList.push_back( CreateItemProcess( iDropItemID ) );
				}
			}

			//{{ 2009. 4. 22  ������	��ü���
			if( sDropDataStatic.m_vecItemID.empty() == false )
			{
				BOOST_TEST_FOREACH( const int, iDropItemID, sDropDataStatic.m_vecItemID )
				{
					// Ư�� �������� Ư���������� ��ӵ��� �ʵ��� �ϴ� �ڵ�
					if( !CXSLDungeon::IsItemDropDungeon( iDropItemID, m_iDungeonID ) )
						continue;

					kDropItemNot.m_DropItemDataList.push_back( CreateItemProcess( iDropItemID ) );
				}
			}
			//}}

			//3.EVENT ITEM ���ó��
			if( kDieNpcInfo.m_bActive == true && sDropDataEvent.m_vecItemID.empty() == false )
			{
				BOOST_TEST_FOREACH( const int, iDropItemID, sDropDataEvent.m_vecItemID )
				{
					// Ư�� �������� Ư���������� ��ӵ��� �ʵ��� �ϴ� �ڵ�
					if( !CXSLDungeon::IsItemDropDungeon( iDropItemID, m_iDungeonID ) )
						continue;

					kDropItemNot.m_DropItemDataList.push_back( CreateItemProcess( iDropItemID ) );
				}
			}

			//4.ATTRIB NPC ITEM ���ó��
			if( kDieNpcInfo.m_bActive == true && sDropDataAttribNpc.m_vecItemID.empty() == false )
			{
				BOOST_TEST_FOREACH( const int, iDropItemID, sDropDataAttribNpc.m_vecItemID )
				{
					kDropItemNot.m_DropItemDataList.push_back( CreateItemProcess( iDropItemID ) );
				}
			}

			//5.HENIR NPC SPECIAL ���ó��
			if( sDropDataHenir.m_vecItemID.empty() == false )
			{
				BOOST_TEST_FOREACH( const int, iDropItemID, sDropDataHenir.m_vecItemID )
				{
					kDropItemNot.m_DropItemDataList.push_back( CreateItemProcess( iDropItemID ) );
				}
			}

			//���ó���� �������� ������ ��Ŷ�� ������.
			if( kDropItemNot.m_DropItemDataList.empty() == false )
			{	
				SendToGSCharacter( spRoomUser->GetGSUID(), spRoomUser->GetCID(), ERM_DROP_ITEM_NOT, kDropItemNot );
			}
		}	// for �� end


		//////////////////////////////////////////////////////////////////////////
		// Ư�� �ð� ������ ��� �̺�Ʈ
		if( kDieNpcInfo.m_bActive == true )
		{
			// �̺�Ʈ �������� �ʽ��ڽ�, Ʃ�丮��, �Ʒüҿ��� ��ӵǼ��� �ȵȴ�!
			if( CXSLDungeon::DI_EL_FOREST_GATE_NORMAL != GetDungeonID()  &&
				CXSLDungeon::IsTutorialDungeon( GetDungeonID() ) == false &&
				CXSLDungeon::IsTCDungeon( GetDungeonID() ) == false )
			{
				//////////////////////////////////////////////////////////////////////////
				// Ư�� �ð��� NPC���������� �����ִ� ���
				if( kPacket_.m_cNpcDieState == KEGS_NPC_UNIT_DIE_REQ::NDS_KILL_BY_USER )
				{
					KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser( kPacket_.m_uiAttUnit );
					if( spRoomUser != NULL )
					{
						int iTimeDropItemID = 0;
						int iTimeDropID = 0;
						if( SiKDropTable()->CheckTimeDrop( iTimeDropID, iTimeDropItemID ) )
						{
							const CXSLItem::ItemTemplet* pItemTemplet = SiCXSLItemManager()->GetItemTemplet( iTimeDropItemID );
							if( pItemTemplet != NULL )
							{
								// �ϴ� DB�� ���� �� ������ ���� �޾Ƶ� �Ǵ� �������� Ȯ������!
								KDBE_CHECK_TIME_DROP_RESULT_REQ kGetTimeDrop;
								kGetTimeDrop.m_iGSUID		 = spRoomUser->GetGSUID();
								kGetTimeDrop.m_iUserUID		 = spRoomUser->GetUserUID();
								kGetTimeDrop.m_iUnitUID		 = spRoomUser->GetCID();
								kGetTimeDrop.m_wstrNickName	 = spRoomUser->GetNickName();
								kGetTimeDrop.m_iTimeDropID	 = iTimeDropID;
								kGetTimeDrop.m_iRewardItemID = iTimeDropItemID;
								kGetTimeDrop.m_wstrRewardItemName = pItemTemplet->m_Name;
								kGetTimeDrop.m_iDropItemUID	 = m_nDropItemUID++;
								SendToLogDB( DBE_CHECK_TIME_DROP_RESULT_REQ, kGetTimeDrop );
							}
							else
							{
								START_LOG( cerr, L"�������� �ʴ� itemid�̴�. ���ÿ����ΰ�?" )
									<< BUILD_LOG( iTimeDropItemID )
									<< END_LOG;
							}
						}
					}
					else
					{
						START_LOG( cwarn, L"�̺�Ʈ ������ ȹ�� ó�� �Ϸ��µ� ���������� ����!" )
							<< BUILD_LOG( kPacket_.m_uiAttUnit )
							<< END_LOG;
					}
				}
				//////////////////////////////////////////////////////////////////////////
			}
		}
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// ���� ��Ƽ���鿡�� �ش� npc�� �����Ϳ� ���� ������ ��ε� ĳ���� �Ѵ�! ( ����ġ ���� ���� )
		// ����� �ο����� �˷��� ������ ���� ����..
		KERM_NPC_UNIT_DIE_NOT kNpcDietNot;
		kNpcDietNot.m_iDungeonID	= GetDungeonID();
		kNpcDietNot.m_cDifficulty	= GetDifficultyLevel();
		kNpcDietNot.m_nDieNPCUID	= kPacket_.m_nDieNPCUID;
		kNpcDietNot.m_iNPCID		= kDieNpcInfo.m_iNPCID;
		kNpcDietNot.m_cAttUnitType	= kPacket_.m_cAttUnitType;
		kNpcDietNot.m_uiAttUnit		= kPacket_.m_uiAttUnit;
		kNpcDietNot.m_cUserCount	= static_cast<char>( m_spRoomUserManager->GetNumMember() );
		kNpcDietNot.m_bNoDrop		= kDieNpcInfo.m_bNoDrop;
		kNpcDietNot.m_sSubStageExpNpcNum = GetSubStageExpNpcNum( m_iSubStageID );
		kNpcDietNot.m_bDecreaseEndurance = false;
		kNpcDietNot.m_cDungeonMode	= m_cDungeonMode;
		kNpcDietNot.m_EXPList.clear();
		kNpcDietNot.m_bQuestComplete = true;

		BroadCast( ERM_NPC_UNIT_DIE_NOT, kNpcDietNot );
	}
}

//////////////////////////////////////////////////////////////////////////
#else			// SERV_REFORM_ITEM_DROP
//////////////////////////////////////////////////////////////////////////

//_IMPL_ON_FUNC( ERM_NPC_UNIT_DIE_REQ, KEGS_NPC_UNIT_DIE_REQ )
//{
//	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_PLAY ), ERM_NPC_UNIT_DIE_ACK, KERM_NPC_UNIT_DIE_ACK );
//
//	//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
//#ifdef SERV_INSERT_GLOBAL_SERVER
//	std::set< int >::iterator sit = m_setDefenseSmallCrystalNpcUID.find( kPacket_.m_nDieNPCUID );
//	if( sit != m_setDefenseSmallCrystalNpcUID.end() )
//	{
//		kPacket.m_iOK		= NetError::NET_OK;
//		kPacket.m_iNPCID	= kPacket_.m_nDieNPCUID;
//		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_NPC_UNIT_DIE_ACK, kPacket );
//
//		// std::set ���� ����
//		m_setDefenseSmallCrystalNpcUID.erase( sit );
//
//		START_LOG( clog, L"���潺 ���� ���� ������ ������!!" )
//			<< END_LOG;
//		return;
//	}
//#endif SERV_INSERT_GLOBAL_SERVER
//	//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
//
//	//������ ���� ����Ʈ���� ã�µ� ���� ���..
//	KRoomMonsterManager::NPC_DATA kDieNpcInfo;
//	if( m_kDungeonMonsterManager.GetNpcData( kPacket_.m_nDieNPCUID, kDieNpcInfo ) == false )
//	{
//		kPacket.m_iOK = NetError::ERR_ROOM_20;
//		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_NPC_UNIT_DIE_ACK, kPacket );
//
//		//{{ 2012. 02. 06    ��μ�    ���� ���� UID �߱� ��� ����
//#ifdef SERV_MAKE_MONSTER_UID_CHANGE
//		// �߱� ���� �ʴ� NPC UID �� Ŭ���̾�Ʈ�� ���´�! 
//		// ��ŷ���� �����Ͽ� ������ ��� ����! 
//		// �ش� �뿡�� �ѹ���....
//		if( GetHackingNpcUid() == false )
//		{
//			SetHackingNpcUid( true );
//
//			//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
//#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
//			KEGS_SERVER_CHECK_HACK_USER_REQ kNot;
//			kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_VERIFY_DUNGEON_NPC_UID;
//#else
//			KEGS_SERVER_CHECK_HACK_USER_NOT kNot;
//			kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_VERIFY_DUNGEON_NPC_UID;
//#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
//			//}}
//			SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SERVER_CHECK_HACK_USER_NOT, kNot );
//
//			START_LOG( clog, L"�߱� ���� �ʴ� NPC UID �� Ŭ���̾�Ʈ�� ���´�!" )
//				<< BUILD_LOG( LAST_SENDER_UID )
//				<< BUILD_LOGc( kNot.m_cHackingCheckType )
//				<< END_LOG;
//		}
//#endif SERV_MAKE_MONSTER_UID_CHANGE	
//		//}}
//
//		return;
//	}
//
//	//�ִٸ� �̹� �׾����� üũ..
//	if( m_kDungeonMonsterManager.IsMonsterAlive( kPacket_.m_nDieNPCUID ) == false )
//	{
//		kPacket.m_iOK = NetError::ERR_ROOM_21;
//		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_NPC_UNIT_DIE_ACK, kPacket );
//		return;
//	}
//
//	//���� ����ִٸ� ���λ���� ���������� ����..
//	LIF( m_kDungeonMonsterManager.SetMonsterDie( kPacket_.m_nDieNPCUID, kPacket_.m_uiAttUnit ) );
//
//	kPacket.m_iOK		= NetError::NET_OK;
//	kPacket.m_iNPCID	= kDieNpcInfo.m_iNPCID;
//	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_NPC_UNIT_DIE_ACK, kPacket );
//
//	//����� �ο����� �˷��� ������ ���� ����..
//	KERM_NPC_UNIT_DIE_NOT kNpcDietNot;
//	kNpcDietNot.m_iDungeonID	= m_iDungeonID;
//	kNpcDietNot.m_cDifficulty	= m_cDifficultyLevel;
//
//	kNpcDietNot.m_nDieNPCUID	= kPacket_.m_nDieNPCUID;
//	kNpcDietNot.m_iNPCID		= kDieNpcInfo.m_iNPCID;
//	kNpcDietNot.m_cAttUnitType	= kPacket_.m_cAttUnitType;
//	kNpcDietNot.m_uiAttUnit		= kPacket_.m_uiAttUnit;
//
//	kNpcDietNot.m_cUserCount	= static_cast<char>( m_spRoomUserManager->GetNumMember() );
//	kNpcDietNot.m_bNoDrop		= kDieNpcInfo.m_bNoDrop;
//
//	//{{ 2009. 12. 31  ������	�������Ҹ���
//	kNpcDietNot.m_sSubStageExpNpcNum = GetSubStageExpNpcNum( m_iSubStageID );
//	kNpcDietNot.m_bDecreaseEndurance = false;
//	//}}
//
//	//{{ 2011. 08. 31  ��μ�	��д��� ����
//#ifdef SERV_RANDOM_DAY_QUEST
//	kNpcDietNot.m_cDungeonMode	= m_cDungeonMode;
//#endif SERV_RANDOM_DAY_QUEST
//	//}}
//
//	//{{ 2009. 5. 11  ������	npc��������
//	const bool bDropDieState = IsDropDieState( kPacket_.m_cNpcDieState );
//	const bool bAddRoomUserKillCount = ( bDropDieState  &&  kPacket_.m_cNpcDieState != KEGS_NPC_UNIT_DIE_REQ::NDS_KILL_SELF );
//	//}}
//
//	const CXSLUnitManager::NPCUnitTemplet* pNPCTemplet = SiCXSLUnitManager()->GetNPCUnitTemplet( (CXSLUnitManager::NPC_UNIT_ID)kDieNpcInfo.m_iNPCID );
//	if( pNPCTemplet == NULL )
//	{
//		START_LOG( cwarn, L"NPC Unit Templet �б� ����.!" )
//			<< BUILD_LOG( kDieNpcInfo.m_iNPCID )
//			<< END_LOG;
//		return;
//	}
//
//	//////////////////////////////////////////////////////////////////////////	
//	//{{ 2010. 11. 20	������	Ư�� �ð� ���� ����
//#ifdef SERV_TIME_DROP_MONSTER_EVENT
//	{
//		std::map< int, int >::const_iterator mitTDM = m_mapTimeDropEventMonster.find( kDieNpcInfo.m_iNPCID );
//		if( mitTDM != m_mapTimeDropEventMonster.end() )
//		{
//			KDBE_UPDATE_EVENT_MONSTER_KILL_INFO_NOT kPacketToLog;
//			kPacketToLog.m_iEventUID = mitTDM->second;
//			kPacketToLog.m_iKillUnitUID = kPacket_.m_uiAttUnit;
//			kPacketToLog.m_tNpcDieDate = CTime::GetCurrentTime().GetTime();
//			SendToLogDB( DBE_UPDATE_EVENT_MONSTER_KILL_INFO_NOT, kPacketToLog );
//
//			// �̺�Ʈ ���� ������ �ʱ�ȭ �մϴ�.
//			m_mapTimeDropEventMonster.clear();
//		}
//	}
//#endif SERV_TIME_DROP_MONSTER_EVENT
//	//}}
//
//	//////////////////////////////////////////////////////////////////////////	
//	//{{ 2010. 7. 28  ������	������ ���� NPC
//#ifdef SERV_AUTO_HACK_CHECK_NPC
//	if( kNpcDietNot.m_cUserCount == 1 )
//	{
//		switch( kDieNpcInfo.m_iNPCID )
//		{
//		case CXSLUnitManager::NUI_DUNGEON_CHECKER1:
//		case CXSLUnitManager::NUI_DUNGEON_CHECKER2:
//		case CXSLUnitManager::NUI_DUNGEON_CHECKER3:
//		case CXSLUnitManager::NUI_DUNGEON_CHECKER4:
//		case CXSLUnitManager::NUI_DUNGEON_CHECKER5:
//			{
//				// �������� ���� �����Ű�� ���� ���� �س���!
//				m_setFindedHackUserUnitUID.insert( FIRST_SENDER_UID );
//
//				// ��Ƽ�忡�� ��ŷüũ��Ŷ�� ������!
//				//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
//#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
//				KEGS_SERVER_CHECK_HACK_USER_REQ kNot;
//				kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_DUNGEON_CANT_KILL_NPC_FIND;
//#else
//				KEGS_SERVER_CHECK_HACK_USER_NOT kNot;
//				kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_DUNGEON_CANT_KILL_NPC_FIND;
//#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
//				//}}
//				SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SERVER_CHECK_HACK_USER_NOT, kNot );
//
//				START_LOG( cout, L"Cant Kill Npc ��ŷ������ �ǴܵǾ� üũ ����� Ŭ���̾�Ʈ �����Ͽ���!" )
//					<< BUILD_LOG( LAST_SENDER_UID )
//					<< BUILD_LOG( FIRST_SENDER_UID )
//					<< BUILD_LOG( kPacket_.m_nDieNPCUID )
//					<< BUILD_LOGc( kPacket_.m_cAttUnitType )
//					<< BUILD_LOG( kPacket_.m_uiAttUnit )
//					<< BUILD_LOG( kPacket_.m_uiMDAttUnit )
//					<< BUILD_LOG( kPacket_.m_DiePos.x )
//					<< BUILD_LOG( kPacket_.m_DiePos.y )
//					<< BUILD_LOG( kPacket_.m_DiePos.z )
//					<< BUILD_LOGc( kPacket_.m_cNpcDieState );
//			}
//			break;
//		}
//	}	
//#endif SERV_AUTO_HACK_CHECK_NPC
//	//}}
//	//////////////////////////////////////////////////////////////////////////	
//
//	//////////////////////////////////////////////////////////////////////////
//	//{{ 2011. 09. 07  ��μ�	���� �������� DIE NPC �ð� üũ ���
//#ifdef SERV_SUB_STAGE_NPC_DIE_CHECK
//	if( SiKGameSysVal()->GetSubStageNpcDieCheck() == true )
//	{
//		// ���� �ΰ�? // ��Ƽ�� NPC �� 5���� �̻��ΰ�?
//		if( m_spRoomUserManager->GetNumMember() == 1 
//			&& m_iSubStageActiveNpc >= 5 
//			&& kDieNpcInfo.m_bActive == true )
//		{
//			++m_iSubStageKillActiveNpc;
//
//			if( m_iSubStageKillActiveNpc == 1 )
//			{
//				// ���� ������������ ó�� ��Ƽ�� ���͸� ���� �ð�
//				m_tSubStageFirstKillTime = CTime::GetCurrentTime();
//			}
//			else
//			{
//				if( m_iSubStageActiveNpc <= m_iSubStageKillActiveNpc ) // ������ ���͸� ��Ҵ�!
//				{
//					CTime tLastKillTime = CTime::GetCurrentTime();
//					CTimeSpan tSpan = CTimeSpan( 0, 0, 0, 1);
//					if( tLastKillTime <= m_tSubStageFirstKillTime + tSpan ) 		// ó�� ���͸� ���� �ð����� 1������ �ۿ� �ɸ��� �ʾҴ�
//					{
//						// ��Ƽ�忡�� ��ŷüũ��Ŷ�� ������!
//						//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
//#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
//						KEGS_SERVER_CHECK_HACK_USER_REQ kNot;
//						kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_CHECK_NPC_DIE_TIME;
//#else
//						KEGS_SERVER_CHECK_HACK_USER_NOT kNot;
//						kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_CHECK_NPC_DIE_TIME;
//#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
//						//}}
//						SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SERVER_CHECK_HACK_USER_NOT, kNot );
//
//						START_LOG( cout, L"Sub Stage�� ��� Npc�� ���� �ð��� 1�ʹۿ� �ɸ��� �ʴ´�." )
//							<< BUILD_LOG( LAST_SENDER_UID )
//							<< BUILD_LOG( FIRST_SENDER_UID )
//							<< BUILD_LOG( m_tSubStageFirstKillTime.GetMinute() )
//							<< BUILD_LOG( m_tSubStageFirstKillTime.GetSecond() )
//							<< BUILD_LOG( tLastKillTime.GetMinute() )
//							<< BUILD_LOG( tLastKillTime.GetSecond() )
//							<< BUILD_LOG( m_iSubStageActiveNpc )
//							<< BUILD_LOG( m_iSubStageKillActiveNpc )
//							<< END_LOG;
//					}
//				}
//			}
//		}
//	}
//#endif SERV_SUB_STAGE_NPC_DIE_CHECK
//	//}}
//	//////////////////////////////////////////////////////////////////////////
//
//
//	// ��Ƽ �÷����� ��� ������ ��� ī��Ʈ ����
//	const int iDropItemNum = ( m_spRoomUserManager->GetLiveMember() >= 3 ? 2 : 1 );
//	//{{ 2010. 07. 09  ������	��ӷ� �̺�Ʈ Ȯ��
//#ifdef SERV_PC_BANG_DROP_EVENT
//	const int iEventDropItemNum = m_iItemDropEventProbCount * iDropItemNum;
//#endif SERV_PC_BANG_DROP_EVENT
//	//}}
//
//	// �Ӽ� �������� ���� �˻�	
//	int iNpcLv = 0;
//	const bool bIsAttribNpc = IsAttribNpc( kPacket_.m_nDieNPCUID, iNpcLv );
//
//	// ���� Ÿ�� ��� ���
//	const char cMonsterTypeFactor = static_cast<char>( CXSLUnitManager::GetMonsterTypeFactor( bIsAttribNpc, GetDifficultyLevel(), kDieNpcInfo.m_cMonsterGrade ) );	
//
//	//////////////////////////////////////////////////////////////////////////
//	// uiAttUnit���� -1�̸� ������ ������ ���� �ʰ� npc�� �ڻ��Ѱ���. [��:��������]
//
//	// �÷��̾ ���͸� ���ΰŶ�� �ش� �÷��̾��� ų���ͼ� üũ
//	if( pNPCTemplet->m_bIsNPC  &&  bAddRoomUserKillCount == true  &&  kPacket_.m_uiAttUnit != -1 )
//	{
//		if( m_spRoomUserManager->AddKillNPC( kPacket_.m_uiAttUnit ) == false )
//		{
//			if( m_kDungeonMonsterManager.IsExistMonster( static_cast<int>(kPacket_.m_uiAttUnit) ) == true )
//			{
//				START_LOG( clog, L"������ �׿���!!" )
//					<< BUILD_LOG( kDieNpcInfo.m_iNPCID )
//					<< BUILD_LOG( kPacket_.m_uiAttUnit );
//
//				// ���Ͱ� ������ �ɷ� �״°���� : �������� �״�� ���ó����.
//			}
//			else
//			{
//				//{{ 2010. 06. 17  ������	������ ����
//#ifdef SERV_PROTECT_AUTO_HACK
//
//				// ���� ���� ������ ��ÿ� ���� ���� ���� ���� ���� ��� 1���� ��츸 üũ!
//				if( m_iStartedNumMember == 1  &&  kNpcDietNot.m_cUserCount == 1 )
//				{
//					START_LOG( cerr, L"������ ���ΰ� �ƴϳ�? ��ŷ������ �ǽɵȴ�!" )
//						<< BUILD_LOG( LAST_SENDER_UID )
//						<< BUILD_LOG( FIRST_SENDER_UID )
//						<< BUILD_LOG( kPacket_.m_nDieNPCUID )
//						<< BUILD_LOGc( kPacket_.m_cAttUnitType )
//						<< BUILD_LOG( kPacket_.m_uiAttUnit )
//						<< BUILD_LOG( kPacket_.m_uiMDAttUnit )
//						<< BUILD_LOG( kPacket_.m_DiePos.x )
//						<< BUILD_LOG( kPacket_.m_DiePos.y )
//						<< BUILD_LOG( kPacket_.m_DiePos.z )
//						<< BUILD_LOGc( kPacket_.m_cNpcDieState )
//						<< BUILD_LOG( kDieNpcInfo.m_iNPCID )
//						<< END_LOG;
//
//					// ������ üũ ī��Ʈ ����!
//					++m_iNpcDieHackCount;
//
//					// �� ������ 5ȸ �̻� �߻��Ѵٸ� �װ��� �߸��� ���̴�!
//					if( m_iNpcDieHackCount == m_iHackUserCheckCount )
//					{
//						// ��Ƽ�忡�� ��ŷüũ��Ŷ�� ������!
//						//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
//#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
//						KEGS_SERVER_CHECK_HACK_USER_REQ kNot;
//						kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_DUNGEON_NPC_UNIT_DIE;
//#else
//						KEGS_SERVER_CHECK_HACK_USER_NOT kNot;
//						kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_DUNGEON_NPC_UNIT_DIE;
//#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
//						//}}
//						SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SERVER_CHECK_HACK_USER_NOT, kNot );
//
//						START_LOG( cout, L"NpcDie ��ŷ������ �ǴܵǾ� üũ ����� Ŭ���̾�Ʈ �����Ͽ���!" )
//							<< BUILD_LOG( LAST_SENDER_UID )
//							<< BUILD_LOG( FIRST_SENDER_UID )
//							<< BUILD_LOG( m_iNpcDieHackCount );
//					}
//				}
//#endif SERV_PROTECT_AUTO_HACK
//				//}}
//			}
//		}
//	}	
//
//	//////////////////////////////////////////////////////////////////////////
//	// 1. ���� ������ ���� ��� �������� �����Ǵ� ���� ���
//	KDropTable::DROP_DATA sDropDataNpc;
//
//	if( kDieNpcInfo.m_bNoDrop == false  &&  bDropDieState == true )
//	{
//		//{{ 2011. 01. 20	������	���� ��ũ ����
//#ifdef SERV_DUNGEON_RANK_NEW
//		if( pNPCTemplet->m_bIsNPC == true  &&  kDieNpcInfo.m_bActive == true )
//		{
//			m_kDungeonMonsterManager.IncreaseMonsterDieCount( cMonsterTypeFactor );
//		}
//#endif SERV_DUNGEON_RANK_NEW
//		//}}
//
//		//{{ 2009. 7. 26  ������	��ϸ� �ð�
//		//ITEM
//		switch( m_eDungeonType )
//		{
//		case CXSLDungeon::DT_HENIR:
//			{
//				if( SiKDropTable()->HenirNpcDropItem( static_cast<CXSLDungeon::DUNGEON_MODE>(m_cDungeonMode), kDieNpcInfo.m_iNPCID, kDieNpcInfo.m_cLevel, cMonsterTypeFactor, sDropDataNpc, iEventDropItemNum, kDieNpcInfo.m_bIsBoss ) == false )
//				{
//					START_LOG( cerr, L"Henir Drop Item Data ������ ���� ===" )
//						<< BUILD_LOG( kPacket_.m_nDieNPCUID )
//						<< BUILD_LOG( kDieNpcInfo.m_iNPCID )
//						<< BUILD_LOGc( m_cDungeonMode )
//						<< END_LOG;
//				}
//
//				//{{ 2010. 01. 05  ������	����������
//				// ������̺��� EXP�� ���õǾ��ִ� ���Ͷ�� ������ ���� üũ�� �Ѵ�.
//				if( sDropDataNpc.m_iExp > 0 )
//				{
//					kNpcDietNot.m_bDecreaseEndurance = true;
//
//					sDropDataNpc.m_iExp = 0; // ��ϸ� �ð��� ����ġ ȹ���� �ϸ� �ȵǱ� ������ 0���� �����Ѵ�.
//				}
//				//}}			
//			}
//			break;
//
//			//{{ 2010. 05. 02  ������	��д��� ����
//#ifdef SERV_SECRET_HELL
//		case CXSLDungeon::DT_SECRET:
//			{
//				// EXTRA STAGE�� ���..
//				if( IsEnteredExtraStage() == true )
//				{
//					if( SiKDropTable()->ExtraStageNpcDropItem( kDieNpcInfo.m_iNPCID, kDieNpcInfo.m_cLevel, cMonsterTypeFactor, sDropDataNpc, iEventDropItemNum, kDieNpcInfo.m_bIsBoss ) == false )
//					{
//						START_LOG( cerr, L"Henir Drop Item Data ������ ���� ===" )
//							<< BUILD_LOG( kPacket_.m_nDieNPCUID )
//							<< BUILD_LOG( kDieNpcInfo.m_iNPCID )
//							<< END_LOG;
//					}
//				}
//				else
//				{
//					if( SiKDropTable()->NormalNpcDropItem( (m_iDungeonID + static_cast<int>(m_cDifficultyLevel)), kDieNpcInfo.m_iNPCID, kDieNpcInfo.m_cLevel, cMonsterTypeFactor, sDropDataNpc, iEventDropItemNum, kDieNpcInfo.m_bIsBoss ) == false )
//					{
//						START_LOG( cerr, L"Drop Item Data ������ ���� ===" )
//							<< BUILD_LOG( kPacket_.m_nDieNPCUID )
//							<< BUILD_LOG( kDieNpcInfo.m_iNPCID )
//							<< BUILD_LOGc( GetDifficultyLevel() )
//							<< END_LOG;
//					}
//				}
//
//				//{{ 2010. 01. 05  ������	����������
//				// ������̺��� EXP�� ���õǾ��ִ� ���Ͷ�� ������ ���� üũ�� �Ѵ�.
//				if( sDropDataNpc.m_iExp > 0 )
//				{
//					kNpcDietNot.m_bDecreaseEndurance = true;
//				}
//				//}}
//			}
//			break;	
//#endif SERV_SECRET_HELL
//			//}}
//
//		default:
//			{
//				if( SiKDropTable()->NormalNpcDropItem( GetDungeonIDAndDif(), kDieNpcInfo.m_iNPCID, kDieNpcInfo.m_cLevel, cMonsterTypeFactor, sDropDataNpc, iEventDropItemNum, kDieNpcInfo.m_bIsBoss ) == false )
//				{
//					START_LOG( cerr, L"Drop Item Data ������ ���� ===" )
//						<< BUILD_LOG( kPacket_.m_nDieNPCUID )
//						<< BUILD_LOG( kDieNpcInfo.m_iNPCID )
//						<< BUILD_LOGc( GetDifficultyLevel() )
//						<< END_LOG;
//				}
//
//				//{{ 2010. 01. 05  ������	����������
//				// ������̺��� EXP�� ���õǾ��ִ� ���Ͷ�� ������ ���� üũ�� �Ѵ�.
//				if( sDropDataNpc.m_iExp > 0 )
//				{
//					kNpcDietNot.m_bDecreaseEndurance = true;
//				}
//				//}}
//			}
//		}
//		//}}
//
//		//{{ 2009. 5. 18  ������	��������
//		switch( kDieNpcInfo.m_iNPCID )
//		{
//		case CXSLUnitManager::NUI_MANA_EATER:
//		case CXSLUnitManager::NUI_HALLOWEEN_MANA_EATER:
//			//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
//#ifdef SERV_INSERT_GLOBAL_SERVER
//		case CXSLUnitManager::NUI_DEFENCE_MANA_EATER:
//#endif SERV_INSERT_GLOBAL_SERVER
//			//}}
//			{
//				if( kPacket_.m_cNpcDieState == KEGS_NPC_UNIT_DIE_REQ::NDS_BONUS_DROP )
//				{
//					// ��� ���̺��� Ȯ���ؼ� 'Special Drop'������� ��ü�ҿ���
//					sDropDataNpc.m_vecItemID.push_back( 70055 ); // 70055
//				}
//			}
//			break;
//
//		case CXSLUnitManager::NUI_BLOOD_EATER:
//			//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
//#ifdef SERV_INSERT_GLOBAL_SERVER
//		case CXSLUnitManager::NUI_DEFENCE_BLOOD_EATER:
//#endif SERV_INSERT_GLOBAL_SERVER
//			//}}
//			{
//				if( kPacket_.m_cNpcDieState == KEGS_NPC_UNIT_DIE_REQ::NDS_BONUS_DROP )
//				{
//					// ��� ���̺��� Ȯ���ؼ� 'Special Drop'������� ��ü�ҿ���
//					sDropDataNpc.m_vecItemID.push_back( 70056 ); // 70055
//				}
//			}
//			break;
//		}		
//		//}}
//
//		//{{ 2011. 01. 19	������	���� �̵� ����
//#ifdef SERV_SHIP_DUNGEON
//		if( CXSLDungeon::IsShipDungeon( GetDungeonID() ) == true )
//		{
//			// ���� �̵� �������� ���� ���ʹ� ����ġ�� �����ϴ�.
//			sDropDataNpc.m_iExp = 0;
//		}
//#endif SERV_SHIP_DUNGEON
//		//}}
//
//		//////////////////////////////////////////////////////////////////////////		
//		//{{ 2009. 12. 18  ������	��������ġ����
//#ifdef DUNGEON_EXP_NEW
//		if( sDropDataNpc.m_iExp > 0 )
//		{
//			// ���� ���� ���̺��κ��� ����ġ �����͸� �����;���!
//			const int nTotalExp = SiKDropTable()->GetNpcExp( kDieNpcInfo.m_cLevel ) * static_cast<int>(cMonsterTypeFactor);
//
//			//{{ 2011. 03. 21  ��μ�	���� ���̵� ����
//			//#ifdef SERV_DUNGEON_DIFFICULTY_REVISION
//			m_spRoomUserManager->AddDungeonRewardEXP( nTotalExp, GetDungeonIDAndDif(), kDieNpcInfo.m_cLevel, kNpcDietNot, GetDifficultyLevel() );
//			//#else
//			//			//{{ 2010. 04. 30  ������	��д��� ����
//			//#ifdef SERV_SECRET_HELL
//			//			m_spRoomUserManager->AddDungeonRewardEXP( nTotalExp, GetDungeonIDAndDif(), mit->second.m_cLevel, kNpcDietNot );
//			//#endif SERV_SECRET_HELL
//			//			//}}
//			//#endif SERV_DUNGEON_DIFFICULTY_REVISION
//			//}}
//
//			//{{ 2012. 02. 28	������	��Ʋ�ʵ� �ý���
//#ifdef SERV_BATTLE_FIELD_SYSTEM
//			IncreasePartyFever(); // ����ġ ������ �ִ� ���͸� ���ΰ�� party fever��ġ�� �ø���!
//#endif SERV_BATTLE_FIELD_SYSTEM
//			//}}
//		}
//#endif DUNGEON_EXP_NEW
//		//}}
//		//////////////////////////////////////////////////////////////////////////		
//	}
//
//
//	//////////////////////////////////////////////////////////////////////////
//	// ���� ��Ƽ���鿡�� �ش� npc�� �����Ϳ� ���� ������ ��ε� ĳ���� �Ѵ�! ( ����ġ ���� ���� )
//	BroadCast( ERM_NPC_UNIT_DIE_NOT, kNpcDietNot );
//
//
//	// ����� ���� �ʴ� npc��� ����� ���� �ʴ´�.
//	if( kDieNpcInfo.m_bNoDrop == true )
//		return;
//
//	// ��ӻ��°� �ƴ�ä�� ����npc��� ������ ����� ���� �ʴ´�.
//	if( bDropDieState == false )
//		return;
//
//
//	//////////////////////////////////////////////////////////////////////////
//	// 2. ������ �������� ��� �������� �����Ǵ� ���� ���
//	KDropTable::DROP_DATA sDropDataStatic;
//
//	//if( mit->second.m_bNoDrop == false  &&  bDropDieState == true )
//	{
//		if( SiKDropTable()->StaticDropItem( (m_iDungeonID + static_cast<int>(m_cDifficultyLevel)), sDropDataStatic, iDropItemNum ) == false )
//		{
//			START_LOG( cerr, L"Drop Item Data ������ ���� ===" )
//				<< BUILD_LOG( kPacket_.m_nDieNPCUID )
//				<< BUILD_LOG( kDieNpcInfo.m_iNPCID )
//				<< BUILD_LOG( m_iDungeonID )
//				<< BUILD_LOGc( m_cDifficultyLevel )
//				<< END_LOG;
//		}
//	}
//
//
//	//////////////////////////////////////////////////////////////////////////	
//	// 3. �̺�Ʈ ������ ��� üũ
//	KDropTable::DROP_DATA sDropDataEvent;
//
//	if( /*mit->second.m_bNoDrop == false  &&*/
//		kDieNpcInfo.m_bActive == true  /*&&*/
//		/*bDropDieState == true*/ )
//	{
//		// �̺�Ʈ �������� �ʽ��ڽ�, Ʃ�丮��, �Ʒüҿ��� ��ӵǼ��� �ȵȴ�!
//		if( CXSLDungeon::DI_EL_FOREST_GATE_NORMAL != m_iDungeonID  &&
//			CXSLDungeon::IsTutorialDungeon( m_iDungeonID ) == false &&
//			CXSLDungeon::IsTCDungeon( m_iDungeonID ) == false )
//		{
//			if( SiKDropTable()->EventDropItem( iDropItemNum, sDropDataEvent ) == false )
//			{
//				START_LOG( cerr, L"Event Drop Item Data ������ ���� ===" )
//					<< BUILD_LOG( kPacket_.m_nDieNPCUID )
//					<< BUILD_LOG( kDieNpcInfo.m_iNPCID )
//					<< BUILD_LOGc( m_cDifficultyLevel )
//					<< END_LOG;
//			}
//
//			//////////////////////////////////////////////////////////////////////////
//			// Ư�� �ð��� NPC���������� �����ִ� ���
//			if( kPacket_.m_cNpcDieState == KEGS_NPC_UNIT_DIE_REQ::NDS_KILL_BY_USER )
//			{
//				KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser( kPacket_.m_uiAttUnit );
//				if( spRoomUser != NULL )
//				{
//					// ü��ID�� �ƴ� ������ ����ó��
//					if( spRoomUser->IsGuestUser() == false )
//					{
//						int iTimeDropItemID = 0;
//						int iTimeDropID = 0;
//						if( SiKDropTable()->CheckTimeDrop( iTimeDropID, iTimeDropItemID ) )
//						{
//							const CXSLItem::ItemTemplet* pItemTemplet = SiCXSLItemManager()->GetItemTemplet( iTimeDropItemID );
//							if( pItemTemplet != NULL )
//							{
//								// �ϴ� DB�� ���� �� ������ ���� �޾Ƶ� �Ǵ� �������� Ȯ������!
//								KDBE_CHECK_TIME_DROP_RESULT_REQ kGetTimeDrop;
//								kGetTimeDrop.m_iGSUID		 = spRoomUser->GetGSUID();
//								kGetTimeDrop.m_iUserUID		 = spRoomUser->GetUserUID();
//								kGetTimeDrop.m_iUnitUID		 = spRoomUser->GetCID();
//								kGetTimeDrop.m_wstrNickName	 = spRoomUser->GetNickName();
//								kGetTimeDrop.m_iTimeDropID	 = iTimeDropID;
//								kGetTimeDrop.m_iRewardItemID = iTimeDropItemID;
//								kGetTimeDrop.m_wstrRewardItemName = pItemTemplet->m_Name;
//								kGetTimeDrop.m_iDropItemUID	 = m_nDropItemUID++;
//								SendToLogDB( DBE_CHECK_TIME_DROP_RESULT_REQ, kGetTimeDrop );
//							}
//							else
//							{
//								START_LOG( cerr, L"�������� �ʴ� itemid�̴�. ���ÿ����ΰ�?" )
//									<< BUILD_LOG( iTimeDropItemID )
//									<< END_LOG;
//							}
//						}
//					}
//				}
//				else
//				{
//					START_LOG( cwarn, L"�̺�Ʈ ������ ȹ�� ó�� �Ϸ��µ� ���������� ����!" )
//						<< BUILD_LOG( kPacket_.m_uiAttUnit )
//						<< END_LOG;
//				}
//			}
//			//////////////////////////////////////////////////////////////////////////
//		}
//	}
//
//
//	//////////////////////////////////////////////////////////////////////////
//	// 4. �Ӽ� ���� ��� ������
//	KDropTable::DROP_DATA sDropDataAttribNpc;
//
//	if( bIsAttribNpc == true  /*&&  bDropDieState == true*/ )
//	{
//		char cDifficultyLevel = GetDifficultyLevel();
//
//		//{{ 2010. 05. 02  ������	��д��� ����
//#ifdef SERV_SECRET_HELL
//		// ��д����ΰ�?
//		if( m_eDungeonType == CXSLDungeon::DT_SECRET )
//		{
//			if( m_cDungeonMode == CXSLDungeon::DM_SECRET_NORMAL )
//			{
//				cDifficultyLevel = CXSLDungeon::DL_HARD;
//			}
//			else if( m_cDungeonMode == CXSLDungeon::DM_SECRET_HELL )
//			{
//				// Extra Stage�ΰ�?
//				if( IsEnteredExtraStage() == true )
//				{
//					cDifficultyLevel = CXSLDungeon::DL_NORMAL;
//				}
//				else
//				{
//					cDifficultyLevel = CXSLDungeon::DL_EXPERT;
//				}
//			}
//		}
//#endif SERV_SECRET_HELL
//		//}}
//
//		if( SiKAttribNpcTable()->AttribNpcDropItem( static_cast<int>(cDifficultyLevel), iNpcLv, iEventDropItemNum, sDropDataAttribNpc.m_vecItemID ) == false )
//		{
//			START_LOG( cerr, L"�Ӽ� ���� ������ ��� ����!" )
//				<< BUILD_LOGc( cDifficultyLevel )
//				<< BUILD_LOG( iNpcLv )
//				<< END_LOG;
//		}
//
//		switch( cDifficultyLevel )
//		{
//		case CXSLDungeon::DL_NORMAL:
//		case CXSLDungeon::DL_HARD:
//			{
//				sDropDataNpc.m_nGPNum = 2 * sDropDataNpc.m_nGPNum;
//			}
//			break;
//
//		case CXSLDungeon::DL_EXPERT:
//			{
//				sDropDataNpc.m_nGPNum = 3 * sDropDataNpc.m_nGPNum;
//			}
//			break;
//
//		default:
//			START_LOG( cerr, L"���� ���̵��� �̻��մϴ�." )
//				<< BUILD_LOGc( cDifficultyLevel )
//				<< END_LOG;
//			break;
//		}
//
//		//{{ 2009. 12. 18  ������	������������
//#ifdef DUNGEON_ITEM
//
//		if( SiKDropTable()->AttribNpcSpecialDrop( cDifficultyLevel, sDropDataAttribNpc.m_vecItemID ) == false )
//		{
//			START_LOG( cerr, L"�Ӽ� ���� ����� ������ ��� ����!" )
//				<< BUILD_LOGc( cDifficultyLevel )
//				<< END_LOG;
//		}
//
//#endif DUNGEON_ITEM
//		//}}
//	}	
//
//
//
//	//////////////////////////////////////////////////////////////////////////
//	// 5. ����Ʈ ��� ������ ó��
//
//	//if( mit->second.m_bNoDrop == false )
//	{
//		std::vector< KRoomUser::QUEST_ITEM_DROP_INFO > vecDropInfo;
//
//		//���ֺ� ����Ʈ ��� �������� �ִ��� �˻�..
//		//{{ 2011. 08. 31  ��μ�	��д��� ����
//#ifdef SERV_RANDOM_DAY_QUEST
//		if( m_spRoomUserManager->GetQuestDropItem( m_iDungeonID, m_cDifficultyLevel, m_cDungeonMode, kDieNpcInfo.m_iNPCID, vecDropInfo ) == true )
//#else
//		if( m_spRoomUserManager->GetQuestDropItem( m_iDungeonID, m_cDifficultyLevel, kDieNpcInfo.m_iNPCID, vecDropInfo ) == true )
//#endif SERV_RANDOM_DAY_QUEST
//			//}}
//		{
//			//�ִٸ� ���ֺ��� ��Ǫ�� ���鼭..
//			BOOST_TEST_FOREACH( const KRoomUser::QUEST_ITEM_DROP_INFO&, kDropInfo, vecDropInfo )
//			{
//				//��ӵ� ������ ����ŭ ó���� ����..
//				BOOST_TEST_FOREACH( const int, iDropQuestItemID, kDropInfo.m_vecDropQuestItem )
//				{
//					KEGS_GET_ITEM_NOT kDropNot;
//					kDropNot.m_cGetItemType = KEGS_GET_ITEM_NOT::GIT_QUEST_ITEM;
//					kDropNot.m_GetUnitUID	= kDropInfo.m_iUnitUID;
//					kDropNot.m_iItemID		= iDropQuestItemID;
//					//{{ 2009. 7. 9  ������		�ӽ� �κ��丮
//					kDropNot.m_iDropItemUID = m_nDropItemUID++;
//					//}}
//					SendToGSCharacter( kDropInfo.m_iGSUID, kDropInfo.m_iUnitUID, ERM_GET_ITEM_INSERT_TO_INVENTORY_NOT, kDropNot );
//				}
//			}
//		}
//		else
//		{
//			if( NetError::GetLastError() != NetError::NET_OK )
//			{
//				START_LOG( cerr, L"����Ʈ ��� ������ �޾ƿ��� ����.!" )
//					<< BUILD_LOG( kPacket_.m_uiAttUnit )
//					<< BUILD_LOG( m_iDungeonID )
//					<< BUILD_LOG( NetError::GetLastErrMsg() )
//					<< END_LOG;
//			}
//		}
//	}
//
//
//	//////////////////////////////////////////////////////////////////////////	
//	//{{ 2011. 08. 23	������	��ϸ� �ð� ����
//#ifdef SERV_NEW_HENIR_TEST
//	// 6. ��ϸ� �ð� ����� ��� ó��
//	KDropTable::DROP_DATA sDropDataHenir;
//
//	if( CXSLDungeon::IsHenirDungeon( GetDungeonID() ) == true )
//	{
//		SiKDropTable()->GetHenirSpecialNpcDropInfo( kDieNpcInfo.m_iNPCID, sDropDataHenir );
//	}
//#endif SERV_NEW_HENIR_TEST
//	//}}
//
//
//	//////////////////////////////////////////////////////////////////////////	
//	// ������ ������ ��� ������ ����Ʈ�� ���� �������� ������ ���� ó��
//	KEGS_DROP_ITEM_NOT kDropItemNot;
//	kDropItemNot.m_CreatePos = kPacket_.m_DiePos;
//
//	//{{ 2011. 08. 12   ��μ�      ��ϸ� ���� 
//#ifdef SERV_NEW_HENIR_TEST
//	// ��ϸ� �����̶�� ��� ������ ���������� Ȯ�� ����.
//	if( CXSLDungeon::IsHenirDungeon( GetDungeonID() ) == true )
//	{
//		// ���� ���� �� �ִ� ������ ����.
//		if( m_spRoomUserManager->GetHenirRewardUserNum() <= 0 )
//		{
//			CheckDeleteHenirDropItem( sDropDataNpc, sDropDataStatic, sDropDataEvent, sDropDataAttribNpc, sDropDataHenir );
//		}
//	}
//#endif SERV_NEW_HENIR_TEST
//	//}}
//
//	//{{ 2011. 05. 27    ��μ�    �޸� ���� ���� ���� ��� ������
//#ifdef SERV_COME_BACK_USER_REWARD
//	if( m_spRoomUserManager->IsComeBackUserInRoom() == true )
//	{
//		if( m_cRoomType == CXSLRoom::RT_DUNGEON )
//		{
//			//{{ 2011. 08. 12   ��μ�      ��ϸ� ���� 
//#ifdef SERV_NEW_HENIR_TEST
//			if( CXSLDungeon::IsHenirDungeon( m_iDungeonID )  == true)
//			{
//				if( m_spRoomUserManager->GetHenirRewardUserNum() > 0 )
//				{
//					if( SiKDropTable()->GetComeBackBuffDropCount( m_spRoomUserManager->GetNumMember() ) == true )
//					{
//						// ��� ���̺��� Ȯ���ؼ� 'Special Drop'������� ��ü�ҿ���
//						kDropItemNot.m_DropItemDataList.push_back( CreateItemProcess( 127260 ) );		// ���ο� ������ ¡ǥ
//					}
//				}
//			}
//			else
//#endif SERV_NEW_HENIR_TEST
//				//}}
//			{
//				if( SiKDropTable()->GetComeBackBuffDropCount( m_spRoomUserManager->GetNumMember() ) == true )
//				{
//					// ��� ���̺��� Ȯ���ؼ� 'Special Drop'������� ��ü�ҿ���
//					kDropItemNot.m_DropItemDataList.push_back( CreateItemProcess( 127260 ) );		// ���ο� ������ ¡ǥ
//				}
//			}
//		}
//	}
//#endif SERV_COME_BACK_USER_REWARD
//	//}} 
//
//	//1.ED ���ó��
//	if( sDropDataNpc.m_iED != -1 )
//	{
//		for( int i = 0; i < sDropDataNpc.m_nGPNum; ++i )
//		{
//			kDropItemNot.m_DropItemDataList.push_back( CreateItemProcess( sDropDataNpc.m_iGPItemID, true, sDropDataNpc.m_iED ) );
//		}
//	}
//
//	//2.NPC ITEM ���ó��	
//	if( sDropDataNpc.m_vecItemID.empty() == false )
//	{
//		BOOST_TEST_FOREACH( const int, iDropItemID, sDropDataNpc.m_vecItemID )
//		{
//			// Ư�� �������� Ư���������� ��ӵ��� �ʵ��� �ϴ� �ڵ�
//			if( !CXSLDungeon::IsItemDropDungeon( iDropItemID, m_iDungeonID ) )
//				continue;
//
//			kDropItemNot.m_DropItemDataList.push_back( CreateItemProcess( iDropItemID ) );
//		}
//	}
//
//	//{{ 2009. 4. 22  ������	��ü���
//	if( sDropDataStatic.m_vecItemID.empty() == false )
//	{
//		BOOST_TEST_FOREACH( const int, iDropItemID, sDropDataStatic.m_vecItemID )
//		{
//			// Ư�� �������� Ư���������� ��ӵ��� �ʵ��� �ϴ� �ڵ�
//			if( !CXSLDungeon::IsItemDropDungeon( iDropItemID, m_iDungeonID ) )
//				continue;
//
//			kDropItemNot.m_DropItemDataList.push_back( CreateItemProcess( iDropItemID ) );
//		}
//	}
//	//}}
//
//	//3.EVENT ITEM ���ó��
//	if( /*mit->second.m_bNoDrop == false &&*/ kDieNpcInfo.m_bActive == true && sDropDataEvent.m_vecItemID.empty() == false )
//	{
//		BOOST_TEST_FOREACH( const int, iDropItemID, sDropDataEvent.m_vecItemID )
//		{
//			// Ư�� �������� Ư���������� ��ӵ��� �ʵ��� �ϴ� �ڵ�
//			if( !CXSLDungeon::IsItemDropDungeon( iDropItemID, m_iDungeonID ) )
//				continue;
//
//			kDropItemNot.m_DropItemDataList.push_back( CreateItemProcess( iDropItemID ) );
//		}
//	}
//
//	//4.ATTRIB NPC ITEM ���ó��
//	if( /*mit->second.m_bNoDrop == false &&*/ kDieNpcInfo.m_bActive == true && sDropDataAttribNpc.m_vecItemID.empty() == false )
//	{
//		BOOST_TEST_FOREACH( const int, iDropItemID, sDropDataAttribNpc.m_vecItemID )
//		{
//			kDropItemNot.m_DropItemDataList.push_back( CreateItemProcess( iDropItemID ) );
//		}
//	}
//
//	//{{ 2011. 08. 23	������	��ϸ� �ð� ����
//#ifdef SERV_NEW_HENIR_TEST
//	//5.HENIR NPC SPECIAL ���ó��
//	if( /*mit->second.m_bNoDrop == false &&*/ sDropDataHenir.m_vecItemID.empty() == false )
//	{
//		BOOST_TEST_FOREACH( const int, iDropItemID, sDropDataHenir.m_vecItemID )
//		{
//			kDropItemNot.m_DropItemDataList.push_back( CreateItemProcess( iDropItemID ) );
//		}
//	}
//#endif SERV_NEW_HENIR_TEST
//	//}}
//
//	//���ó���� �������� ������ ��Ŷ�� ������.
//	if( kDropItemNot.m_DropItemDataList.empty() == false )
//	{	
//		BroadCast( ERM_DROP_ITEM_NOT, kDropItemNot );
//	}
//}
#endif SERV_REFORM_ITEM_DROP
//}}
//////////////////////////////////////////////////////////////////////////
#endif SERV_BATTLE_FIELD_SYSTEM
//////////////////////////////////////////////////////////////////////////
//}}

_IMPL_ON_FUNC( ERM_USER_UNIT_DIE_REQ, KEGS_USER_UNIT_DIE_REQ )
{
	//{{ 2011. 09. 19  ��μ�	Īȣ ȹ�� ���� �߰�
#ifdef SERV_ADD_TITLE_CONDITION
	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_PLAY ), ERM_USER_UNIT_DIE_ACK, KERM_USER_UNIT_DIE_ACK );
#else
	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_PLAY ), ERM_USER_UNIT_DIE_ACK, KEGS_USER_UNIT_DIE_ACK );
#endif SERV_ADD_TITLE_CONDITION
	//}}

	// Die üũ
	if( !m_spRoomUserManager->SetDie( FIRST_SENDER_UID, true ) )
	{
		kPacket.m_iOK = NetError::ERR_SLOT_07;
		//{{ 2011. 09. 19  ��μ�	Īȣ ȹ�� ���� �߰�
#ifdef SERV_ADD_TITLE_CONDITION
		KERM_USER_UNIT_DIE_ACK kAck;
		kAck.m_iOK = kPacket.m_iOK;
		kAck.m_ucDieReason = kPacket_.m_ucDieReason;
		kAck.m_fReBirthTime	= kPacket.m_fReBirthTime;
		kAck.m_VP			= kPacket.m_VP;
		kAck.m_EXP			= kPacket.m_EXP;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_USER_UNIT_DIE_ACK, kAck );
#else
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_USER_UNIT_DIE_ACK, kPacket );
#endif SERV_ADD_TITLE_CONDITION
		//}}
		return;
	}

	// �����ϸ� ������Ŷ�� �ش��������� ������
	kPacket.m_iOK = NetError::NET_OK;

	//{{ 2011. 09. 19  ��μ�	Īȣ ȹ�� ���� �߰�
#ifdef SERV_ADD_TITLE_CONDITION
	KERM_USER_UNIT_DIE_ACK kAck;
	kAck.m_iOK = kPacket.m_iOK;
	kAck.m_ucDieReason = kPacket_.m_ucDieReason;
	kAck.m_fReBirthTime	= kPacket.m_fReBirthTime;
	kAck.m_VP			= kPacket.m_VP;
	kAck.m_EXP			= kPacket.m_EXP;
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_USER_UNIT_DIE_ACK, kAck );
#else
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_USER_UNIT_DIE_ACK, kPacket );
#endif SERV_ADD_TITLE_CONDITION
	//}}

	

	// ���� ��� �������� ������ �˸���.
	KERM_USER_UNIT_DIE_NOT kPacketNot;
	kPacketNot.m_iGameType			= m_cGameType;
	kPacketNot.m_KillerNPCUID = kPacket_.m_KillerNPCUID;
	kPacketNot.m_KilledUserUnitUID = FIRST_SENDER_UID;
	BroadCast( ERM_USER_UNIT_DIE_NOT, kPacketNot );

	//{{ 2008. 6. 17  ������  �ǽð� ��Ȱ��
	m_spRoomUserManager->ReserveEndPlay( FIRST_SENDER_UID );
	//}}

    if( !CheckIfPlayEnd() )
	{
		//������ �ٸ� ������ ���� ������ ���� �������� üũ�� �Ѵ�.
		if( m_kDSSGoNextAllNot.m_iBeforeStageID != -1 &&
			m_kDSSGoNextAllNot.m_iNextStageID	!= -1 )
			SendDSSGoNextAllNot( m_kDSSGoNextAllNot );
	}
}

IMPL_ON_FUNC_NOPARAM( ERM_USER_UNIT_DIE_COMPLETE_REQ )
{
	VERIFY_ROOM_STATE_AND_WARN( ( 1, KRoomFSM::S_PLAY ), (1, KRoomFSM::S_RESULT), ERM_USER_UNIT_DIE_COMPLETE_ACK, KEGS_USER_UNIT_DIE_COMPLETE_ACK );	

	KEGS_USER_UNIT_DIE_COMPLETE_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_USER_UNIT_DIE_COMPLETE_ACK, kAck );

	KEGS_USER_UNIT_DIE_COMPLETE_NOT kNot;
	kNot.m_UnitUID = FIRST_SENDER_UID;

	BroadCast( ERM_USER_UNIT_DIE_COMPLETE_NOT, kNot );
}

//{{ �ʵ� ��� ���� - ��μ�
#ifdef SERV_REFORM_ITEM_DROP
_IMPL_ON_FUNC( ERM_GET_ITEM_REQ, KEGS_GET_ITEM_REQ )
{
	VERIFY_ROOM_STATE( ( 2, KRoomFSM::S_PLAY, KRoomFSM::S_RESULT ), ERM_GET_ITEM_ACK, KEGS_GET_ITEM_ACK );

	kPacket.m_iOK = NetError::NET_OK;

	KEGS_GET_ITEM_NOT kNot;

	std::map< UidType, UidType > mapHenirRewardUserList;

	std::map< int, ITEM_DATA >::iterator mit;
	mit = m_mapDropItem.find( kPacket_.m_iDropItemUID );

	// 1. �����ҷ��� item�� ���°��
	if( mit == m_mapDropItem.end() )
	{
		START_LOG( cerr, L"�����ҷ��� �������� ����" )
			<< BUILD_LOG( kPacket_.m_iDropItemUID )
			<< BUILD_LOG( kPacket_.m_kItemInfo.m_iItemID )
			<< END_LOG;

		// ����� �Ͼ �� ���� ����!
		if( kPacket_.m_iDropItemUID == -1  &&  kPacket_.m_kItemInfo.m_iItemID == -1 )
		{
			++m_iGetItemHackCount;

			if( m_iGetItemHackCount == m_iHackUserCheckCount )
			{
				// ��Ƽ�忡�� ��ŷüũ��Ŷ�� ������!
				KEGS_SERVER_CHECK_HACK_USER_REQ kNot;
				kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_DUNGEON_GET_ITEM;
				SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SERVER_CHECK_HACK_USER_NOT, kNot );

				START_LOG( cout, L"GetItem ��ŷ������ �ǴܵǾ� üũ ����� Ŭ���̾�Ʈ �����Ͽ���!" )
					<< BUILD_LOG( LAST_SENDER_UID )
					<< BUILD_LOG( FIRST_SENDER_UID )
					<< BUILD_LOG( m_iGetItemHackCount );
			}
		}


		kPacket.m_iOK = NetError::ERR_ITEM_00;
		goto end_proc;
	}

	ITEM_DATA& sItemData = mit->second;

	// 2. ������ ���� �˻�
	if( kPacket_.m_kItemInfo.m_iItemID != sItemData.m_iItemID )
	{
		START_LOG( cerr, L"�����ҷ��� �������� ������ �߸��Ǿ���." )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< BUILD_LOG( kPacket_.m_iDropItemUID )
			<< BUILD_LOG( kPacket_.m_kItemInfo.m_iItemID )
			<< BUILD_LOG( sItemData.m_iItemID )
			<< END_LOG;

		// ����� �Ͼ �� ���� ����!
		if( kPacket_.m_kItemInfo.m_iItemID == -1 )
		{
			++m_iGetItemHackCount;

			if( m_iGetItemHackCount == m_iHackUserCheckCount )
			{
				// ��Ƽ�忡�� ��ŷüũ��Ŷ�� ������!
				KEGS_SERVER_CHECK_HACK_USER_REQ kNot;
				kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_DUNGEON_GET_ITEM;

				SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SERVER_CHECK_HACK_USER_NOT, kNot );

				START_LOG( cout, L"GetItem ��ŷ������ �ǴܵǾ� üũ ����� Ŭ���̾�Ʈ �����Ͽ���!" )
					<< BUILD_LOG( LAST_SENDER_UID )
					<< BUILD_LOG( FIRST_SENDER_UID )
					<< BUILD_LOG( m_iGetItemHackCount );
			}
		}

		kPacket.m_iOK = NetError::ERR_ITEM_05;
		goto end_proc;
	}

	// 3. �ٸ� ������ �̹� �����Ѱǰ�
	if( sItemData.m_bIsGet == true )
	{
		kPacket.m_iOK = NetError::ERR_ITEM_01;
		goto end_proc;
	}

	kNot.m_iDropItemUID	= kPacket_.m_iDropItemUID;
	kNot.m_iItemID		= kPacket_.m_kItemInfo.m_iItemID;
	//{{ 2013. 1. 14	�ڼ���	���� �� �ʵ� ��� �����ۿ� ��ȭ �ο�
#ifdef SERV_ENCHANT_ITEM_DROP_EVENT
	kNot.m_cEnchantLevel= sItemData.m_cEnchantLevel;
#endif SERV_ENCHANT_ITEM_DROP_EVENT
	//}}

	// �����Һ� ������
	bool bNotCreateItem = false;
	UidType iGSUID = LAST_SENDER_UID; // ServerUID����

// 	const bool bIsGetItemDistribute = ( kPacket_.m_kItemInfo.m_iItemID == CXSLItem::HDR_TIMESPACE_PIECE );
// 	if( bIsGetItemDistribute == true )
// 	{
// 		m_spRoomUserManager->GetHenirRewardUserList( mapHenirRewardUserList );
// 	}
// 	else
	{
		// 4. ���� �������� ED�ΰ�? Item�ΰ�?
		if( sItemData.m_bIsGP == true )
		{
			kNot.m_iTotalED = sItemData.m_iED;
			kNot.m_GetUnitUID = FIRST_SENDER_UID; // Ŭ���̾�Ʈ ȭ�鿡 ����� ED�Ծ��� ǥ�����ٶ� ���
			const float fPartyBonusRate = GetDungeonPartyBonusRate();
			if( !m_spRoomUserManager->AddDungeonRewardED( sItemData.m_iED, FIRST_SENDER_UID, fPartyBonusRate, GetDungeonID(), GetDifficultyLevel(), kNot ) )

			{
				START_LOG( cerr, L"ED����ó���� �̻��մϴ�!" )
					<< BUILD_LOG( sItemData.m_iED )
					<< END_LOG;

				kPacket.m_iOK = NetError::ERR_ITEM_04;
				goto end_proc;
			}

			bNotCreateItem = true;
		}
		else		// ������
		{
			const CXSLItem::ItemTemplet* pItemTemplet = SiCXSLItemManager()->GetItemTemplet( sItemData.m_iItemID );
			if( pItemTemplet == NULL )
			{
				START_LOG( cerr, L"Item Templet ������ ����. �̻��� ������ �Ծ���." )
					<< BUILD_LOG( FIRST_SENDER_UID )
					<< BUILD_LOG( sItemData.m_iItemID )
					<< END_LOG;

				kPacket.m_iOK = NetError::ERR_ITEM_04;

				sItemData.m_bIsGet = true; // �����ɷ� ó��
				goto end_proc;
			}
			else
			{
				kNot.m_GetUnitUID	= FIRST_SENDER_UID;

				// �����Һ� �������̸� �κ��� ���� �ʱ�����
				if( pItemTemplet->m_ItemType == CXSLItem::IT_OUTLAY )
				{
					bNotCreateItem = true;
				}
			}
		}
	}

	sItemData.m_bIsGet	= true;	//������ ����� �������� ���������� üũ

	// ack�� ���� ������
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_GET_ITEM_ACK, kPacket );

	//////////////////////////////////////////////////////////////////////////	
	if( m_setFindedHackUserUnitUID.empty() == false )
	{
		if( m_spRoomUserManager->GetNumMember() == 1 )
		{
			// �������� �´��� �˻�!
			if( m_setFindedHackUserUnitUID.find( FIRST_SENDER_UID ) != m_setFindedHackUserUnitUID.end() )
			{
				// ��Ƽ�忡�� ��ŷüũ��Ŷ�� ������!
				KEGS_SERVER_CHECK_HACK_USER_REQ kNot;
				kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_DUNGEON_CANT_KILL_NPC_KICK;

				SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SERVER_CHECK_HACK_USER_NOT, kNot );

#ifdef SERV_MINIMUM_LOG_HAKC_USER
				START_LOG( cout, L"Cant Kill Npc pos1. UnitUID : " << FIRST_SENDER_UID  );
#else //SERV_MINIMUM_LOG_HAKC_USER
				START_LOG( cout, L"Cant Kill Npc ��ŷ������ �Ǵܵ� ������ �������� ���������Ű�� ���� ��Ŷ ����!" )
					<< BUILD_LOG( LAST_SENDER_UID )
					<< BUILD_LOG( FIRST_SENDER_UID )
					<< BUILD_LOG( kPacket_.m_iDropItemUID )
					<< BUILD_LOG( kPacket_.m_kItemInfo.m_iItemID )
					<< BUILD_LOG( m_setFindedHackUserUnitUID.size() );
#endif //SERV_MINIMUM_LOG_HAKC_USER
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////	

// 	if( bIsGetItemDistribute )
// 	{
// 		std::map< UidType, UidType>::iterator mit = mapHenirRewardUserList.begin();
// 		for( ; mit != mapHenirRewardUserList.end() ; ++mit )
// 		{
// 			kNot.m_GetUnitUID = mit->first;
// 
// 			// �����鿡�� ������ ȹ���� �˸�
//			BroadCast( ERM_GET_ITEM_NOT, kNot );
//
//			// ������ ������ ȹ�� ó���� �� ��Ŷ�� ȹ�� �������� ����
//			SendToGSCharacter( mit->second, kNot.m_GetUnitUID, ERM_GET_ITEM_INSERT_TO_INVENTORY_NOT, kNot );
//			//}}
// 		}
// 	}
// 	else

	{
		// ������ ȹ�� ��� ������
		if( bNotCreateItem == false )
		{
			// ������ ȹ���� �������� ����

			// �����鿡�� ������ ȹ���� �˸�
			BroadCast( ERM_GET_ITEM_NOT, kNot );

			// ������ ������ ȹ�� ó���� �� ��Ŷ�� ȹ�� �������� ����
			SendToGSCharacter( iGSUID, kNot.m_GetUnitUID, ERM_GET_ITEM_INSERT_TO_INVENTORY_NOT, kNot );
			//}}


			//{{ ��� ȹ�� ���� ������ - ���������, �ŷ¾�����
			//{{ 2013. 1. 14	�ڼ���	���� �� �ʵ� ��� �����ۿ� ��ȭ �ο�
#ifdef SERV_ENCHANT_ITEM_DROP_EVENT
			if(		( sItemData.m_bNotify == true )
				||	( SiKDropTable()->IsNotifyDropItem( sItemData.m_iItemID ) )
				)
#else
			if( SiKDropTable()->IsNotifyDropItem( sItemData.m_iItemID ) )
#endif SERV_ENCHANT_ITEM_DROP_EVENT
				//}}
			{
				KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser( kNot.m_GetUnitUID );
				if( spRoomUser != NULL )
				{
					const CXSLItem::ItemTemplet* pItemTemplet = SiCXSLItemManager()->GetItemTemplet( sItemData.m_iItemID );
					if( pItemTemplet != NULL )
					{
						KEGS_NOTIFY_MSG_NOT	kPacket;
						kPacket.m_cNotifyType = KEGS_NOTIFY_MSG_NOT::NT_DROP_GET_ITEM; // ������ ȹ�� ����
						kPacket.m_Count = 1;
						kPacket.m_wstrMSG = spRoomUser->GetNickName();
						kPacket.m_wstrMSG += L";";
						//{{ 2012. 1. 17	�ڼ���	���� �� �ʵ� ��� �����ۿ� ��ȭ �ο�
#ifdef SERV_ENCHANT_ITEM_DROP_EVENT
						if( sItemData.m_bNotify == true )
						{
#ifdef SERV_UNITED_SERVER_EU
							kPacket.m_wstrMSG += boost::str( boost::wformat( L"+%d; " ) % static_cast<int>( sItemData.m_cEnchantLevel ) );
#else //SERV_UNITED_SERVER_EU
							kPacket.m_wstrMSG += boost::str( boost::wformat( L"+%d " ) % static_cast<int>( sItemData.m_cEnchantLevel ) );
#endif //SERV_UNITED_SERVER_EU
						}
#endif SERV_ENCHANT_ITEM_DROP_EVENT
						//}}
#ifdef SERV_UNITED_SERVER_EU
						kPacket.m_wstrMSG += pItemTemplet->m_ItemID;
#else //SERV_UNITED_SERVER_EU
						kPacket.m_wstrMSG += pItemTemplet->m_Name;
#endif //SERV_UNITED_SERVER_EU						
						KncSend( PI_CN_ROOM, GetUID(), PI_CN_SERVER, KBaseServer::GetKObj()->GetUID(), NULL, ECN_NOTIFY_MSG_NOT, kPacket );
					}
					else
					{
						START_LOG( cerr, L"��� ȹ�� ������ ������ �������µ� �������� �ʴ� �������Դϴ�" )
							<< BUILD_LOG( kNot.m_GetUnitUID )
							<< BUILD_LOG( sItemData.m_iItemID )
							<< END_LOG;
					}
				}
				else
				{
					START_LOG( cerr, L"��� ȹ�� ������ ������ �������µ� �濡 ������ �����ϴ�" )
						<< BUILD_LOG( kNot.m_GetUnitUID )
						<< END_LOG;
				}
			}
		}
		else
		{

			if( true == SiKDropTable()->CheckPVPItem( kPacket_.m_kItemInfo.m_iItemID ) )
			{
				if( static_cast<float>(::rand()%100) > SiKDropTable()->GetPVPDropFailRate( KDropTable::PF_LOSETEAM ) )
					kNot.m_bIsItemSuccess = true;
			}

			// �����Һ� �����۰� ED�������� ��ε�ĳ����
			BroadCast( ERM_GET_ITEM_NOT, kNot );
		}
	}

	return;

end_proc:
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_GET_ITEM_ACK, kPacket );	
}

#else		// SERV_REFORM_ITEM_DROP

_IMPL_ON_FUNC( ERM_GET_ITEM_REQ, KEGS_GET_ITEM_REQ )
{
	VERIFY_ROOM_STATE( ( 2, KRoomFSM::S_PLAY, KRoomFSM::S_RESULT ), ERM_GET_ITEM_ACK, KEGS_GET_ITEM_ACK );

	kPacket.m_iOK = NetError::NET_OK;

	KEGS_GET_ITEM_NOT kNot;

	//{{ 2011. 08. 12   ��μ�      ��ϸ� ���� 
#ifdef SERV_NEW_HENIR_TEST
	std::map< UidType, UidType > mapHenirRewardUserList;
#endif SERV_NEW_HENIR_TEST
	//}}

	std::map< int, ITEM_DATA >::iterator mit;
	mit = m_mapDropItem.find( kPacket_.m_iDropItemUID );

	// 1. �����ҷ��� item�� ���°��
	if( mit == m_mapDropItem.end() )
	{
		START_LOG( cerr, L"�����ҷ��� �������� ����" )
			<< BUILD_LOG( kPacket_.m_iDropItemUID )
			<< BUILD_LOG( kPacket_.m_kItemInfo.m_iItemID )
			<< END_LOG;

		//{{ 2010. 06. 17  ������	������ ����
#ifdef SERV_PROTECT_AUTO_HACK
		// ����� �Ͼ �� ���� ����!
		if( kPacket_.m_iDropItemUID == -1  &&  kPacket_.m_kItemInfo.m_iItemID == -1 )
		{
			++m_iGetItemHackCount;

			if( m_iGetItemHackCount == m_iHackUserCheckCount )
			{
				// ��Ƽ�忡�� ��ŷüũ��Ŷ�� ������!
				//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
				KEGS_SERVER_CHECK_HACK_USER_REQ kNot;
				kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_DUNGEON_GET_ITEM;
#else
				KEGS_SERVER_CHECK_HACK_USER_NOT kNot;
				kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_DUNGEON_GET_ITEM;
#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
				//}}
				SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SERVER_CHECK_HACK_USER_NOT, kNot );

				START_LOG( cout, L"GetItem ��ŷ������ �ǴܵǾ� üũ ����� Ŭ���̾�Ʈ �����Ͽ���!" )
					<< BUILD_LOG( LAST_SENDER_UID )
					<< BUILD_LOG( FIRST_SENDER_UID )
					<< BUILD_LOG( m_iGetItemHackCount );
			}
		}
#endif SERV_PROTECT_AUTO_HACK
		//}}

		kPacket.m_iOK = NetError::ERR_ITEM_00;
		goto end_proc;
	}

	ITEM_DATA& sItemData = mit->second;

	// 2. ������ ���� �˻�
	if( kPacket_.m_kItemInfo.m_iItemID != sItemData.m_iItemID )
	{
		START_LOG( cerr, L"�����ҷ��� �������� ������ �߸��Ǿ���." )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< BUILD_LOG( kPacket_.m_iDropItemUID )
			<< BUILD_LOG( kPacket_.m_kItemInfo.m_iItemID )
			<< BUILD_LOG( sItemData.m_iItemID )
			<< END_LOG;

		//{{ 2010. 06. 17  ������	������ ����
#ifdef SERV_PROTECT_AUTO_HACK
		// ����� �Ͼ �� ���� ����!
		if( kPacket_.m_kItemInfo.m_iItemID == -1 )
		{
			++m_iGetItemHackCount;

			if( m_iGetItemHackCount == m_iHackUserCheckCount )
			{
				// ��Ƽ�忡�� ��ŷüũ��Ŷ�� ������!
				//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
				KEGS_SERVER_CHECK_HACK_USER_REQ kNot;
				kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_DUNGEON_GET_ITEM;
#else
				KEGS_SERVER_CHECK_HACK_USER_NOT kNot;
				kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_DUNGEON_GET_ITEM;
#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
				//}}
				SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SERVER_CHECK_HACK_USER_NOT, kNot );

				START_LOG( cout, L"GetItem ��ŷ������ �ǴܵǾ� üũ ����� Ŭ���̾�Ʈ �����Ͽ���!" )
					<< BUILD_LOG( LAST_SENDER_UID )
					<< BUILD_LOG( FIRST_SENDER_UID )
					<< BUILD_LOG( m_iGetItemHackCount );
			}
		}
#endif SERV_PROTECT_AUTO_HACK
		//}}

		kPacket.m_iOK = NetError::ERR_ITEM_05;
		goto end_proc;
	}

	// 3. �ٸ� ������ �̹� �����Ѱǰ�
	if( sItemData.m_bIsGet == true )
	{
		kPacket.m_iOK = NetError::ERR_ITEM_01;
		goto end_proc;
	}

	kNot.m_iDropItemUID	= kPacket_.m_iDropItemUID;
	kNot.m_iItemID		= kPacket_.m_kItemInfo.m_iItemID;

	// �����Һ� ������
	bool bNotCreateItem = false;
	UidType iGSUID = LAST_SENDER_UID; // ServerUID����

	//{{ 2011. 08. 12   ��μ�      ��ϸ� ���� 
#ifdef SERV_NEW_HENIR_TEST
	const bool bIsGetItemDistribute = ( kPacket_.m_kItemInfo.m_iItemID == CXSLItem::HDR_TIMESPACE_PIECE );
	if( bIsGetItemDistribute == true )
	{
		m_spRoomUserManager->GetHenirRewardUserList( mapHenirRewardUserList );
	}
	else
#endif SERV_NEW_HENIR_TEST
		//}}
	{
		// 4. ���� �������� ED�ΰ�? Item�ΰ�?
		if( sItemData.m_bIsGP == true )
		{
			kNot.m_iTotalED = sItemData.m_iED;
			kNot.m_GetUnitUID = FIRST_SENDER_UID; // Ŭ���̾�Ʈ ȭ�鿡 ����� ED�Ծ��� ǥ�����ٶ� ���

			//{{ 2011. 08. 12   ��μ�      ��ϸ� ���� 
#ifdef SERV_NEW_HENIR_TEST
			if( !m_spRoomUserManager->AddDungeonRewardED( sItemData.m_iED, GetDungeonID(), kNot ) )
#else
			if( !m_spRoomUserManager->AddDungeonRewardED( sItemData.m_iED, kNot ) )
#endif SERV_NEW_HENIR_TEST
				//}}
			{
				START_LOG( cerr, L"ED����ó���� �̻��մϴ�!" )
					<< BUILD_LOG( sItemData.m_iED )
					<< END_LOG;

				kPacket.m_iOK = NetError::ERR_ITEM_04;
				goto end_proc;
			}

			bNotCreateItem = true;
		}
		else		// ������
		{
			const CXSLItem::ItemTemplet* pItemTemplet = SiCXSLItemManager()->GetItemTemplet( sItemData.m_iItemID );
			if( pItemTemplet == NULL )
			{
				START_LOG( cerr, L"Item Templet ������ ����. �̻��� ������ �Ծ���." )
					<< BUILD_LOG( FIRST_SENDER_UID )
					<< BUILD_LOG( sItemData.m_iItemID )
					<< END_LOG;

				kPacket.m_iOK = NetError::ERR_ITEM_04;

				sItemData.m_bIsGet = true; // �����ɷ� ó��
				goto end_proc;
			}
			else
			{
				kNot.m_GetUnitUID	= FIRST_SENDER_UID;

				//�����Һ� �������̸� �κ��� ���� �ʱ�����
				if( pItemTemplet->m_ItemType != CXSLItem::IT_OUTLAY )
				{
					//������ �����Ŀ� ����..
					if( m_cGetItemType == CXSLRoom::DGIT_RANDOM )
					{
						// Item�� ���� ������ �� �� ���� ������ ������ ���� Unit�� UID�� ��ȯ���ش�.
						//{{ 2011. 08. 12   ��μ�      ��ϸ� ���� 
#ifdef SERV_NEW_HENIR_TEST
						if( m_spRoomUserManager->GetItemWithRandom( sItemData.m_iItemID, GetDungeonID(), kNot.m_GetUnitUID, iGSUID ) == false )
#else
						if( m_spRoomUserManager->GetItemWithRandom( sItemData.m_iItemID,  kNot.m_GetUnitUID, iGSUID ) == false )
#endif SERV_NEW_HENIR_TEST
							//}}
						{
							kPacket.m_iOK = NetError::ERR_ITEM_03;
							goto end_proc;
						}
					}
				}
				else
				{
					bNotCreateItem = true;
				}
			}
		}
	}

	sItemData.m_bIsGet	= true;	//������ ����� �������� ���������� üũ

	// ack�� ���� ������
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_GET_ITEM_ACK, kPacket );

	//////////////////////////////////////////////////////////////////////////	
	//{{ 2010. 7. 28  ������	������ ���� NPC
#ifdef SERV_AUTO_HACK_CHECK_NPC
	if( m_setFindedHackUserUnitUID.empty() == false )
	{
		if( m_spRoomUserManager->GetNumMember() == 1 )
		{
			// �������� �´��� �˻�!
			if( m_setFindedHackUserUnitUID.find( FIRST_SENDER_UID ) != m_setFindedHackUserUnitUID.end() )
			{
				// ��Ƽ�忡�� ��ŷüũ��Ŷ�� ������!
				//{{ 2012. 06. 04	��μ�       ���� ��ŷ ���� üũ ��û �������� ����
#ifdef SERV_SERVER_CHECK_HACK_USER_REQ_ACK
				KEGS_SERVER_CHECK_HACK_USER_REQ kNot;
				kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_REQ::HCT_DUNGEON_CANT_KILL_NPC_KICK;
#else
				KEGS_SERVER_CHECK_HACK_USER_NOT kNot;
				kNot.m_cHackingCheckType = KEGS_SERVER_CHECK_HACK_USER_NOT::HCT_DUNGEON_CANT_KILL_NPC_KICK;
#endif SERV_SERVER_CHECK_HACK_USER_REQ_ACK
				//}}
				SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_SERVER_CHECK_HACK_USER_NOT, kNot );

#ifdef SERV_MINIMUM_LOG_HAKC_USER
				START_LOG( cout, L"Cant Kill Npc pos2. UnitUID : " << FIRST_SENDER_UID  );
#else //SERV_MINIMUM_LOG_HAKC_USER
				START_LOG( cout, L"Cant Kill Npc ��ŷ������ �Ǵܵ� ������ �������� ���������Ű�� ���� ��Ŷ ����!" )
					<< BUILD_LOG( LAST_SENDER_UID )
					<< BUILD_LOG( FIRST_SENDER_UID )
					<< BUILD_LOG( kPacket_.m_iDropItemUID )
					<< BUILD_LOG( kPacket_.m_kItemInfo.m_iItemID )
					<< BUILD_LOG( m_setFindedHackUserUnitUID.size() );
#endif //SERV_MINIMUM_LOG_HAKC_USER
			}
		}
	}
#endif SERV_AUTO_HACK_CHECK_NPC
	//}}
	//////////////////////////////////////////////////////////////////////////	

	//{{ 2011. 08. 12   ��μ�      ��ϸ� ���� 
#ifdef SERV_NEW_HENIR_TEST
	if( bIsGetItemDistribute )
	{
		std::map< UidType, UidType>::iterator mit = mapHenirRewardUserList.begin();
		for( ; mit != mapHenirRewardUserList.end() ; ++mit )
		{
			kNot.m_GetUnitUID = mit->first;

			// �����鿡�� ������ ȹ���� �˸�
			BroadCast( ERM_GET_ITEM_NOT, kNot );

			// ������ ������ ȹ�� ó���� �� ��Ŷ�� ȹ�� �������� ����
			SendToGSCharacter( mit->second, kNot.m_GetUnitUID, ERM_GET_ITEM_INSERT_TO_INVENTORY_NOT, kNot );
			//}}
		}
	}
	else
#endif SERV_NEW_HENIR_TEST
		//}}
	{
		// ������ ȹ�� ��� ������
		if( bNotCreateItem == false )
		{
			// ������ ȹ���� �������� ����
			//{{ 2010. 03. 03  ������	ä�� ���� ��Ƽ
#ifdef SERV_CHANNEL_PARTY
			//{{ 2010. 03. 18  ������	������ ȹ�� �ڵ� ����

			// �����鿡�� ������ ȹ���� �˸�
			BroadCast( ERM_GET_ITEM_NOT, kNot );

			// ������ ������ ȹ�� ó���� �� ��Ŷ�� ȹ�� �������� ����
			SendToGSCharacter( iGSUID, kNot.m_GetUnitUID, ERM_GET_ITEM_INSERT_TO_INVENTORY_NOT, kNot );
			//}}

#else
#pragma ELSE_MESSAGE
			//SendToGSCharacter( LAST_SENDER_UID, kNot.m_GetUnitUID, ERM_GET_ITEM_INSERT_TO_INVENTORY_NOT, kNot );
#endif SERV_CHANNEL_PARTY
			//}}

			//{{ 2009. 6. 23  ������	��� ȹ�� ���� ������ - ���������, �ŷ¾�����
			if( SiKDropTable()->IsNotifyDropItem( sItemData.m_iItemID ) )
			{
				KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser( kNot.m_GetUnitUID );
				if( spRoomUser != NULL )
				{
					const CXSLItem::ItemTemplet* pItemTemplet = SiCXSLItemManager()->GetItemTemplet( sItemData.m_iItemID );
					if( pItemTemplet != NULL )
					{
						KEGS_NOTIFY_MSG_NOT	kPacket;
						kPacket.m_cNotifyType = KEGS_NOTIFY_MSG_NOT::NT_DROP_GET_ITEM; // ������ ȹ�� ����
						kPacket.m_Count = 1;
						kPacket.m_wstrMSG = spRoomUser->GetNickName();
						kPacket.m_wstrMSG += L";";
#ifdef SERV_UNITED_SERVER_EU
						kPacket.m_wstrMSG += pItemTemplet->m_ItemID;
#else //SERV_UNITED_SERVER_EU
						kPacket.m_wstrMSG += pItemTemplet->m_Name;
#endif //SERV_UNITED_SERVER_EU
						KncSend( PI_CN_ROOM, GetUID(), PI_CN_SERVER, KBaseServer::GetKObj()->GetUID(), NULL, ECN_NOTIFY_MSG_NOT, kPacket );
					}
					else
					{
						START_LOG( cerr, L"��� ȹ�� ������ ������ �������µ� �������� �ʴ� �������Դϴ�" )
							<< BUILD_LOG( kNot.m_GetUnitUID )
							<< BUILD_LOG( sItemData.m_iItemID )
							<< END_LOG;
					}
				}
				else
				{
					START_LOG( cerr, L"��� ȹ�� ������ ������ �������µ� �濡 ������ �����ϴ�" )
						<< BUILD_LOG( kNot.m_GetUnitUID )
						<< END_LOG;
				}
			}
			//}}
		}
		else
		{
#ifdef DUNGEON_ITEM
			if( true == SiKDropTable()->CheckPVPItem( kPacket_.m_kItemInfo.m_iItemID ) )
			{
				if( static_cast<float>(::rand()%100) > SiKDropTable()->GetPVPDropFailRate( KDropTable::PF_LOSETEAM ) )
					kNot.m_bIsItemSuccess = true;
			}
#endif DUNGEON_ITEM
			// �����Һ� �����۰� ED�������� ��ε�ĳ����
			BroadCast( ERM_GET_ITEM_NOT, kNot );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//{{ 2009. 7. 3  ������		��ϸ� �ð�	
	//## des second(hoons) : �ð��� �߰��� �ִ� �������� ������.
	//if( m_eDungeonType == CXSLDungeon::DT_HENIR )
	//{
	//	if( kPacket.m_iOK == NetError::NET_OK  &&  sItemData.m_iItemID == 70085 ) // ������ �𷡽ð�
	//	{
	//		SetRemainingPlayTime( m_fRemainingPlayTime + 300.f );
	//		BroadCast( ERM_REMAINING_PLAY_TIME_NOT, GetRemainingPlayTime() );
	//	}
	//}
	//}}
	//////////////////////////////////////////////////////////////////////////
	return;

end_proc:
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_GET_ITEM_ACK, kPacket );	
}
#endif SERV_REFORM_ITEM_DROP
//}}

//{{ 2009. 5. 11  ������	�ǽð�������
IMPL_ON_FUNC( ERM_GET_ITEM_COMPLETE_NOT )
{
	if( GetStateID() != KRoomFSM::S_PLAY  &&
		GetStateID() != KRoomFSM::S_RESULT )
	{
		return;
	}

	std::map< int, int >::const_iterator mit;
	for( mit = kPacket_.m_mapGetItem.begin(); mit != kPacket_.m_mapGetItem.end(); ++mit )
	{
		if( !m_spRoomUserManager->AddItem( kPacket_.m_iUnitUID, mit->first ) )
		{
			START_LOG( cerr, L"������ ȹ�� ���� ������Ʈ ����!" )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( mit->first )
				<< END_LOG;
		}
	}
}
//}}

IMPL_ON_FUNC( ERM_MY_USER_UNIT_INFO_TO_SERVER_REQ )
{
	//�������� �θ� room�� end_game ���� wait���� ��ü�� �ϱ⶧����
	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_RESULT ), ERM_MY_USER_UNIT_INFO_TO_SERVER_ACK, KPacketOK );

	//{{ 2010. 05. 12  ������	���� ���� ������ ����
#ifdef SERV_INTEGRATION
	if( m_spRoomUserManager->SetRoomUserInfo( FIRST_SENDER_UID, kPacket_.m_kRoomUserInfo ) == false )
	{
		kPacket.m_iOK = NetError::ERR_ROOM_17;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_MY_USER_UNIT_INFO_TO_SERVER_ACK, kPacket );
	}

	// RoomUserInfo�� ���������� ������
	BroadCast( ERM_CHANGE_ROOM_USER_INFO_NOT, kPacket_.m_kRoomUserInfo );
#endif SERV_INTEGRATION
	//}}

	if( m_spRoomUserManager->SetDungeonUnitInfo( FIRST_SENDER_UID, kPacket_.m_kDungeonUnitInfo ) == false )
	{
		kPacket.m_iOK = NetError::ERR_ROOM_01;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_MY_USER_UNIT_INFO_TO_SERVER_ACK, kPacket );
	}

	kPacket.m_iOK       = NetError::NET_OK;
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_MY_USER_UNIT_INFO_TO_SERVER_ACK, kPacket );

	//////////////////////////////////////////////////////////////////////////
	//===Result===
	CheckDungeonUnitInfoSuccess();
}

//{{ 2008. 9. 19  ������		���
_IMPL_ON_FUNC( ERM_DUNGEON_PLAY_INFO_TO_SERVER_NOT, KEGS_DUNGEON_PLAY_INFO_TO_SERVER_NOT )
{
	if( GetStateID() != KRoomFSM::S_PLAY )
	{
		START_LOG( cwarn, L"��Ŷ�� ���Ǵ� �� ���°� �ƴ�." )
			<< BUILD_LOG( GetStateIDString() )
			<< END_LOG;

		return;
	}

	if( !m_spRoomUserManager->SetDungeonPlayInfo( FIRST_SENDER_UID, kPacket_ ) )
	{
		START_LOG( cwarn, L"������Ʈ �����߳�?" )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< END_LOG;
	}
}
//}}

_IMPL_ON_FUNC( ERM_END_GAME_REQ, KEGS_END_GAME_REQ )
{
	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_PLAY ), ERM_END_GAME_ACK, KPacketOK );

	// �������� üũ�Ѵ�.
	VERIFY_HOST( ERM_END_GAME_ACK );

	// ���� ������� ack�� ������.
	kPacket.m_iOK = NetError::NET_OK;
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_END_GAME_ACK, kPacket );

	if( kPacket_.m_iWinTeam == CXSLRoom::TN_RED )
    {
        m_bIsWin = true;

		//////////////////////////////////////////////////////////////////////////
		//{{ 2009. 7. 3  ������		��ϸ� �ð�
		// �̺�Ʈ ������ Ŭ�����ϸ� �׾��� ��� ������ �ٽ� �ǻ츰��!
		if( m_eDungeonType == CXSLDungeon::DT_HENIR )
		{
			for( int iU = 0; iU < m_spRoomUserManager->GetNumMember(); ++iU )
			{
				KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser( iU );
				if( spRoomUser == NULL )
				{
					START_LOG( cerr, L"RoomUser�� �������� ����." )
						<< END_LOG;
					continue;
				}

				// ����ִ� ������ �н�~
				if( spRoomUser->IsDie() == false )
					continue;

				// �ǻ츮��!
				spRoomUser->SetDie( false );

				KEGS_RESURRECT_TO_CONTINUE_DUNGEON_NOT kPacketNot;
				kPacketNot.m_iUnitUID = spRoomUser->GetCID();
				kPacketNot.m_iUsedByUnitUID = spRoomUser->GetCID();
				kPacketNot.m_iStartPosIndex = SiCXSLDungeonManager()->GetStartPosByRandom( (m_iDungeonID + static_cast<int>(m_cDifficultyLevel)), m_iStageID, m_iSubStageID );
				BroadCast( EGS_RESURRECT_TO_CONTINUE_DUNGEON_NOT, kPacketNot );

				//{{ 2008. 6. 17  ������  �ǽð� ��Ȱ��
				m_spRoomUserManager->SetEndPlay( spRoomUser->GetCID(), false );
				//}}
			}
		}
		//}}

		//{{ 2012. 02. 28	������	��Ʋ�ʵ� �ý���
//#ifdef SERV_BATTLE_FIELD_SYSTEM
//		IncreasePartyFever();
//#else
//		if( m_iPartyUID > 0  &&  
//			m_spRoomUserManager->GetNumMember() > 1 )
//		{
//			KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser( 0 ); // 0�� �ε����� ����
//			if( spRoomUser != NULL )
//			{
//				KERM_INCREASE_PARTY_FEVER_NOT kNot;
//				kNot.m_iPartyUID = m_iPartyUID;
//				SendToGSServer( spRoomUser->GetGSUID(), ERM_INCREASE_PARTY_FEVER_NOT, kNot );
//			}
//			else
//			{
//				START_LOG( cerr, L"������ �Ѹ��� ���¹��ΰ�?" )
//					<< BUILD_LOG( GetStateIDString() )
//					<< END_LOG;
//			}
//		}
//#endif SERV_BATTLE_FIELD_SYSTEM
		//}}
		//////////////////////////////////////////////////////////////////////////
    }

	// ������ ������.
	EndPlay();
}

_IMPL_ON_FUNC( ERM_USE_QUICK_SLOT_NOT, KEGS_USE_QUICK_SLOT_NOT )
{
	VERIFY_STATE( ( 1, KRoomFSM::S_PLAY ) );

#ifdef SERV_DUNGEON_CLEAR_PAYMENT_ITEM_FIX
	kPacket_.m_bUseItemNeedPayment = false;

	for ( int iR = 0; iR < m_spRoomUserManager->GetNumMember(); ++iR )
	{
		KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser(iR);
		// ���� UID �����ͼ� üũ ��, ���� ���ֿ��Ը� �� ���� �ֽʤ�.��
		if( spRoomUser->GetCID() == kPacket_.m_UnitUID )
		{
			// �켱 ����� �������� �ùٸ� �������ΰ� üũ
			if(SiKDropTable()->CheckPaymentUseItem(kPacket_.m_ItemID) == true)
			{
				//spRoomUser->SetUseItemForPayment( true );
				kPacket_.m_bUseItemNeedPayment = true;
				START_LOG(clog, L"����α�-�׽�Ʈ : ������ ��� üũ �Ϸ�!")
					<< BUILD_LOG(kPacket_.m_bUseItemNeedPayment)
					<< END_LOG;
				break;
			}

		}
	}
#endif //SERV_DUNGEON_CLEAR_PAYMENT_ITEM_FIX

    BroadCast( ERM_USE_QUICK_SLOT_NOT, kPacket_ );
}

_IMPL_ON_FUNC( ERM_CHECK_INVALID_USER_NOT, UidType )
{
	std::vector<KRoomUserManager::KLeaveUserInfo>	vecDelUserUID;
	int iCount = m_spRoomUserManager->DeleteUserByGsUID( kPacket_, vecDelUserUID );

	START_LOG_WITH_NAME( clog )
		<< BUILD_LOG( kPacket_ )
		<< BUILD_LOG( iCount )
		<< BUILD_LOG( m_spRoomUserManager->GetNumMember() );

	//����濡�� ������ ������ ������ ����.
	if( iCount == 0 )
		return;

	for( int i = 0; i < (int)vecDelUserUID.size(); i++ )
	{
		m_spRoomUserManager->LeaveRoom( vecDelUserUID[i].m_iUnitUID );
	}

	for( int i = 0; i < (int)vecDelUserUID.size(); i++ )
	{
		if( IsEmpty() == false )
		{
			// ���� ��� �����鿡�� ���� ������ ������.
			KEGS_LEAVE_ROOM_NOT kPacketNot;
			kPacketNot.m_cRoomState		= GetStateID();
			kPacketNot.m_iUnitUID		= vecDelUserUID[i].m_iUnitUID;

			GetRoomSlotInfo( kPacketNot.m_vecSlot );
			BroadCast( ERM_LEAVE_ROOM_NOT, kPacketNot );
		}
	}

	if( (GetStateID() == KRoomFSM::S_LOAD || GetStateID() == KRoomFSM::S_PLAY) && IsEmpty() == false )
	{
		CheckLoadingSuccess();
	}

	// ���������� ������ �Ǵ� ��� ó��.
	if( GetStateID() == KRoomFSM::S_PLAY && IsEmpty() == false )
	{
		if( CheckIfPlayEnd() )
		{
			EndPlay();
		}
		else
		{
			//{{ 2009. 7. 27  ������	��н�������
			if( m_eDungeonType == CXSLDungeon::DT_HENIR )
			{
				CheckEnterSecretStageSuccess();
			}
			//}}
		}
	}

	//���â�� ���������� ���� �������ó��
	if( GetStateID() == KRoomFSM::S_RESULT && IsEmpty() == false )
	{
		CheckResultSuccess();
	}

	// �濡�� ��� ������ �������� ���� �ݴ´�.
	if( IsEmpty() == true )
	{
		StateTransition( KRoomFSM::I_TO_CLOSE );

		//{{ 2010. 06. 21  ������	��� ������
#ifdef SERV_STATISTICS_THREAD
		CTime kEndGameTime = CTime::GetCurrentTime();
		//////////////////////////////////////////////////////////////////////////
		KE_LOCAL_LOG_DUNGEON_ROOM_NOT kNot;
		kNot.m_iDungeonID		= m_iDungeonID + static_cast<int>(m_cDifficultyLevel);
		kNot.m_iDifficultyLevel	= (int) GetDifficultyLevel();
		kNot.m_iIsChallenge		= ( m_cDungeonMode == CXSLDungeon::DM_HENIR_CHALLENGE ? 1 : 0 );
		kNot.m_iWin				= 0;
		kNot.m_iLose			= 0;
		kNot.m_iStartNumMember	= m_iStartedNumMember;
		kNot.m_iEndNumMember	= GetNumMember();
		kNot.m_iTotalPlayTime	= static_cast<int>(GetTotalPlayTime());
		kNot.m_wstrEndGameTime	= (const wchar_t*)kEndGameTime.Format(_T("%H:%M:%S"));
		KSIManager.QueueingEvent( E_LOCAL_LOG_DUNGEON_ROOM_NOT, kNot );
		//////////////////////////////////////////////////////////////////////////
#endif SERV_STATISTICS_THREAD
		//}}
	}
	else
	{
		//������ �����ִ� �������� ������������ �̵��� ��Ű�� ����
		if( m_kDSSGoNextAllNot.m_iBeforeStageID != -1 &&
			m_kDSSGoNextAllNot.m_iNextStageID	!= -1 )
			SendDSSGoNextAllNot( m_kDSSGoNextAllNot );
	}
}

_IMPL_ON_FUNC( ERM_RESURRECT_TO_CONTINUE_DUNGEON_REQ, KEGS_RESURRECT_TO_CONTINUE_DUNGEON_REQ )
{
	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_PLAY ), EGS_RESURRECT_TO_CONTINUE_DUNGEON_ACK, KEGS_RESURRECT_TO_CONTINUE_DUNGEON_ACK );

	bool bDie;
	if( !m_spRoomUserManager->IsDie( kPacket_.m_iUnitUID, bDie ) )
	{
		kPacket.m_iOK = NetError::ERR_ROOM_17;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, EGS_RESURRECT_TO_CONTINUE_DUNGEON_ACK, kPacket );

		return;
	}

	if( !bDie )
	{
		kPacket.m_iOK = NetError::ERR_RESURRECT_01;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, EGS_RESURRECT_TO_CONTINUE_DUNGEON_ACK, kPacket );

		return;
	}

	if( !m_spRoomUserManager->SetDie( kPacket_.m_iUnitUID, false ) )
	{
		kPacket.m_iOK = NetError::ERR_ROOM_17;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, EGS_RESURRECT_TO_CONTINUE_DUNGEON_ACK, kPacket );

		return;
	}

	kPacket.m_iDungeonID = (m_iDungeonID + static_cast<int>(m_cDifficultyLevel));
	kPacket.m_iOK = NetError::NET_OK;
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, EGS_RESURRECT_TO_CONTINUE_DUNGEON_ACK, kPacket );

	KEGS_RESURRECT_TO_CONTINUE_DUNGEON_NOT kPacketNot;
	kPacketNot.m_iUnitUID = kPacket_.m_iUnitUID;

	KRoomUserPtr roomSlotPtr = m_spRoomUserManager->GetUser( kPacketNot.m_iUnitUID );
	if( roomSlotPtr == NULL )
	{
		START_LOG( cerr, L"�濡 ������ ��ü�� �������� ����." )
			<< BUILD_LOG( m_cRoomType )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< END_LOG;
	}
	else
	{
		//{{ 2007. 11. 5  ������  ��Ȱ�� ��� ī��Ʈ
		roomSlotPtr->IncreaseUsedResurrectionStoneCount();
		//}}
		//{{ 2007. 12. 25  ������  ��Ȱ�� ��� ���
		KStatisticsKey kKey;
		kKey.m_vecIntKey.push_back( 0 );
		KSIManager.IncreaseCount( KStatistics::SI_RES_STONE, kKey, KStatistics::eSIColResStone_UseCount, 1 );
		//}}
	}

	kPacketNot.m_iUsedByUnitUID = kPacket_.m_iUsedByUnitUID;
	kPacketNot.m_iStartPosIndex = SiCXSLDungeonManager()->GetStartPosByRandom( (m_iDungeonID + static_cast<int>(m_cDifficultyLevel)), m_iStageID, m_iSubStageID );
	BroadCast( EGS_RESURRECT_TO_CONTINUE_DUNGEON_NOT, kPacketNot );

	//{{ 2008. 6. 17  ������  �ǽð� ��Ȱ��
	m_spRoomUserManager->SetEndPlay( kPacket_.m_iUnitUID, false );
	//}}
}

IMPL_ON_FUNC_NOPARAM( ERM_SKIP_CONTINUE_DUNGEON_NOT )
{
	if( GetStateID() != KRoomFSM::S_PLAY )
	{
		START_LOG( cwarn, L"��Ŷ�� ���Ǵ� �� ���°� �ƴ�." )
			<< BUILD_LOG( GetStateIDString() )
			<< END_LOG;

		return;
	}

	//{{ 2008. 6. 17  ������  �ǽð� ��Ȱ��
	if( m_spRoomUserManager->CheckEndPlayAnotherMember( FIRST_SENDER_UID ) )
		EndPlay();
	//}}
}

IMPL_ON_FUNC( ERM_TUTORIAL_DUNGEON_ROOM_UPDATE_NOT )
{
	if( GetStateID() != KRoomFSM::S_WAIT )
	{
		START_LOG( cwarn, L"��Ŷ�� ���Ǵ� �� ���°� �ƴ�." )
			<< BUILD_LOG( GetStateIDString() )
			<< END_LOG;
		//return;
	}

	m_spRoomUserManager->SetTutorialUIDList( FIRST_SENDER_UID, kPacket_.m_vecStudentUnitUID );
}

//{{ 2008. 6. 16  ������  �ǽð� ��Ȱ��
_IMPL_ON_FUNC( ERM_STOP_DUNGEON_CONTINUE_TIME_REQ, KEGS_STOP_DUNGEON_CONTINUE_TIME_REQ )
{
	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_PLAY ), ERM_STOP_DUNGEON_CONTINUE_TIME_ACK, KEGS_STOP_DUNGEON_CONTINUE_TIME_ACK );

	// ��Ƽ�� �ð� ���߱� or Ǯ��
	m_spRoomUserManager->StopDungeonContinueTime( FIRST_SENDER_UID, kPacket_.m_bIsStop );

	kPacket.m_iOK	  = NetError::NET_OK;
	kPacket.m_bIsStop = kPacket_.m_bIsStop;
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_STOP_DUNGEON_CONTINUE_TIME_ACK, kPacket );
}
//}}

//{{ 2009. 2. 4  ������	���� ���÷��̹� ��ũ
_IMPL_ON_FUNC( ERM_FIELD_UNIT_SYNC_DATA_NOT, KEGS_FIELD_UNIT_SYNC_DATA_NOT )
{
	if( GetStateID() != KRoomFSM::S_WAIT )
	{
		START_LOG( cwarn, L"��Ŷ�� ���Ǵ� �� ���°� �ƴ�." )
			<< BUILD_LOG( GetStateIDString() )
			<< END_LOG;
		return;
	}

	KEGS_FIELD_UNIT_SYNC_DATA_BROAD_NOT kPacketNot;
	kPacketNot.m_UnitUID		= FIRST_SENDER_UID;
	kPacketNot.m_vecFieldFrame	= kPacket_.m_vecFieldFrame;
	kPacketNot.m_vPos			= kPacket_.m_vPos;
	kPacketNot.m_bIsRight		= kPacket_.m_bIsRight;	
	BroadCast( ERM_FIELD_UNIT_SYNC_DATA_BROAD_NOT, kPacketNot );
}
//}}

//{{ 2009. 3. 18  ������	��Ƽ������
IMPL_ON_FUNC_NOPARAM( ERM_PARTY_BAN_USER_REQ )
{
	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_WAIT ), ERM_PARTY_BAN_USER_ACK, KERM_PARTY_BAN_USER_ACK );

	// �濡 �ִ� �����ΰ�?
	KRoomUserPtr pkRoomUser = m_spRoomUserManager->GetUser( FIRST_SENDER_UID );	
	if( pkRoomUser == NULL )
	{
		START_LOG( cerr, L"�����ų ������ ã�����" )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_SLOT_05;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_PARTY_BAN_USER_ACK, kPacket );
		return;
	}

	// �� ���� ���� ����.
	if( !m_spRoomUserManager->LeaveRoom( FIRST_SENDER_UID ) )
	{
		START_LOG( cerr, L"�� ��Ż ����!" )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< END_LOG;

		// ���� ����.
		kPacket.m_iOK = NetError::ERR_SLOT_05;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_PARTY_BAN_USER_ACK, kPacket );
		return;
	}

	// ���� ����ڿ��� not�� ������.
	kPacket.m_iOK = NetError::NET_OK;
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_PARTY_BAN_USER_ACK, kPacket );

	//������ ��� ����� ���� ��� ������ ������
	if( IsEmpty() )
	{
		StateTransition( KRoomFSM::I_TO_CLOSE );

		//{{ 2010. 06. 21  ������	��� ������
#ifdef SERV_STATISTICS_THREAD
		CTime kEndGameTime = CTime::GetCurrentTime();
		//////////////////////////////////////////////////////////////////////////
		KE_LOCAL_LOG_DUNGEON_ROOM_NOT kNot;
		kNot.m_iDungeonID		= m_iDungeonID + static_cast<int>(m_cDifficultyLevel);
		kNot.m_iDifficultyLevel	= (int) GetDifficultyLevel();
		kNot.m_iIsChallenge		= ( m_cDungeonMode == CXSLDungeon::DM_HENIR_CHALLENGE ? 1 : 0 );
		kNot.m_iWin				= 0;
		kNot.m_iLose			= 0;
		kNot.m_iStartNumMember	= m_iStartedNumMember;
		kNot.m_iEndNumMember	= GetNumMember();
		kNot.m_iTotalPlayTime	= static_cast<int>(GetTotalPlayTime());
		kNot.m_wstrEndGameTime	= (const wchar_t*)kEndGameTime.Format(_T("%H:%M:%S"));
		KSIManager.QueueingEvent( E_LOCAL_LOG_DUNGEON_ROOM_NOT, kNot );
		//////////////////////////////////////////////////////////////////////////
#endif SERV_STATISTICS_THREAD
		//}}
	}
}
//}}

//{{ 2011. 10. 25	������	��Ʋ�ʵ� �ý���
//////////////////////////////////////////////////////////////////////////
#ifdef SERV_BATTLE_FIELD_SYSTEM
//////////////////////////////////////////////////////////////////////////

IMPL_ON_FUNC_NOPARAM( ERM_DUNGEON_KILLALLNPC_CHECK_REQ )
{
#ifdef SERV_FIX_NONE_NPC_DUNGEON_LINES// �۾���¥: 2013-05-22	// �ڼ���
	if( IsHost( FIRST_SENDER_UID ) == false )
	{
		return;
	}
#endif // SERV_FIX_NONE_NPC_DUNGEON_LINES

	if( GetStateID() != KRoomFSM::S_PLAY )
	{
		START_LOG( cerr, L"��Ŷ�� ���Ǵ� �� ���°� �ƴ�." )
			<< BUILD_LOG( GetStateIDString() )
			<< END_LOG;
		return;
	}

	KEGS_DUNGEON_KILLALLNPC_CHECK_ACK kPacket;
	kPacket.m_bResult = true;

	//{{ 2010. 12. 28	������	�ϸ� ���� �߰�
#ifdef SERV_SHIP_DUNGEON
	if( CXSLDungeon::IsShipDungeon( GetDungeonID() ) == true )
	{
		START_LOG( clog, L"���� �̵� �����̸� �׳� ���� ó�� �Ѵ�!" )
			<< BUILD_LOG( GetDungeonID() );

		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_KILLALLNPC_CHECK_ACK, kPacket );
		return;
	}
#endif SERV_SHIP_DUNGEON
	//}}
	//{{ 2013. 02. 01  �̺�Ʈ �߷�Ÿ�� ���� - ��μ�
#ifdef SERV_EVENT_VALENTINE_DUNGEON
	if( CXSLDungeon::IsValentineDungeon( GetDungeonID() ) == true )
	{
		START_LOG( clog, L"�̺�Ʈ �߷�Ÿ�� �����̸� �׳� ���� ó�� �Ѵ�!" )
			<< BUILD_LOG( GetDungeonID() );

		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_KILLALLNPC_CHECK_ACK, kPacket );
		return;
	}
#endif SERV_EVENT_VALENTINE_DUNGEON
	//}

	// �ش� ���� ���������� ��� ���Ͱ� �׾����� üũ����!
	if( m_kDungeonMonsterManager.IsAllSubStageNpcDie( GetStageID(), GetSubStageID() ) == false )
	{
		START_LOG( cerr, L"Npc�� ���� ���������ε� ���NPC �׿��޶�� ��û��Ŷ�� �Գ�? ��ŷ�ΰ�?" )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< BUILD_LOG( GetStageID() )
			<< BUILD_LOG( GetSubStageID() )
			<< END_LOG;

		kPacket.m_bResult = false;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_KILLALLNPC_CHECK_ACK, kPacket );
		return;
	}

	// Ŭ���̾�Ʈ�� ��� ����
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_KILLALLNPC_CHECK_ACK, kPacket );

#ifdef SERV_FIX_NONE_NPC_DUNGEON_LINES// �۾���¥: 2013-05-22	// �ڼ���
	// Ȥ�ó� �𸣴� ��� üũ ����
	if( kPacket.m_bResult == true )
	{
		BroadCastID( ERM_DUNGEON_KILLALLNPC_CHECK_NOT );
	}
#endif // SERV_FIX_NONE_NPC_DUNGEON_LINES
}

//////////////////////////////////////////////////////////////////////////
#else
//////////////////////////////////////////////////////////////////////////

//IMPL_ON_FUNC_NOPARAM( ERM_DUNGEON_KILLALLNPC_CHECK_REQ )
//{
//	if( GetStateID() != KRoomFSM::S_PLAY )
//	{
//		START_LOG( cerr, L"��Ŷ�� ���Ǵ� �� ���°� �ƴ�." )
//			<< BUILD_LOG( GetStateIDString() )
//			<< END_LOG;
//		return;
//	}
//
//	KEGS_DUNGEON_KILLALLNPC_CHECK_ACK kPacket;
//	kPacket.m_bResult = true;
//
//	// ��� NPC�� �׿��޶�� ��û�� ���� ���������� ������ �ִ� ���� ������������ Ȯ������!
//	KSubStageKey kSubStageKey( m_iStageID, m_iSubStageID );
//
//	std::map< KSubStageKey, SUB_STAGE_NPC_INFO >::const_iterator mit;
//	mit = m_mapSubStageNpcInfo.find( kSubStageKey );
//	if( mit == m_mapSubStageNpcInfo.end() )
//	{
//		START_LOG( cerr, L"Npc�� ���� ���������ε� ���NPC �׿��޶�� ��û��Ŷ�� �Գ�? ��ŷ�ΰ�?" )
//			<< BUILD_LOG( FIRST_SENDER_UID )
//			<< BUILD_LOG( m_iStageID )
//			<< BUILD_LOG( m_iSubStageID )
//			<< END_LOG;
//
//		kPacket.m_bResult = false;
//		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_KILLALLNPC_CHECK_ACK, kPacket );
//		return;
//	}
//
//	const SUB_STAGE_NPC_INFO& sSubStageNpcInfo = mit->second;
//
//	// ���� �������� ���� �Ϲ� �������� ���� Ȯ��
//	if( sSubStageNpcInfo.m_setBossNpcUID.empty() )
//	{
//		// �ش� ���� ���������� ��� �Ϲ� ���Ͱ� �׾����� Ȯ��!
//		std::set< int >::const_iterator sit;
//		for( sit = sSubStageNpcInfo.m_setNormalNpcUID.begin(); sit != sSubStageNpcInfo.m_setNormalNpcUID.end(); ++sit )
//		{
//			const int iNpcUID = *sit;
//			std::map< int, NPC_DATA >::const_iterator mitND;
//			mitND = m_mapNPCData.find( iNpcUID );
//			if( mitND == m_mapNPCData.end() )
//			{
//				START_LOG( cerr, L"�Ϲ� npc ����Ʈ�� �ִ� NpcUID�� �˻��ߴ��� NPC_DATA�� ����?" )
//					<< BUILD_LOG( FIRST_SENDER_UID )
//					<< BUILD_LOG( m_iStageID )
//					<< BUILD_LOG( m_iSubStageID )
//					<< BUILD_LOG( iNpcUID )
//					<< END_LOG;
//				continue;
//			}
//
//			//{{ 2010. 12. 28	������	�ϸ� ���� �߰�
//#ifdef SERV_SHIP_DUNGEON
//			if( CXSLDungeon::IsShipDungeon( GetDungeonID() ) == true )
//			{
//				START_LOG( clog, L"���� �̵� �����̸� �׳� ���� ó�� �Ѵ�!" )
//					<< BUILD_LOG( GetDungeonID() );
//				break;
//			}
//#endif SERV_SHIP_DUNGEON
//			//}}
//
//			// npc�� ���� �ʾҴٸ�?
//			if( mitND->second.m_bDie == false )
//			{
//				START_LOG( cerr, L"��� npc�� ������ �ʾҴµ� ��� ���� �׿��޶�� ��Ŷ�� �Գ�.. ��ŷ�ΰ�?" )
//					<< BUILD_LOG( FIRST_SENDER_UID )
//					<< BUILD_LOG( m_iStageID )
//					<< BUILD_LOG( m_iSubStageID )
//					<< BUILD_LOG( iNpcUID )
//					<< END_LOG;
//
//				kPacket.m_bResult = false;
//				break;
//			}
//		}
//	}
//	else
//	{
//		// �ش� ���� ���������� ��� ���� ���Ͱ� �׾����� Ȯ��!
//		std::set< int >::const_iterator sit;
//		for( sit = sSubStageNpcInfo.m_setBossNpcUID.begin(); sit != sSubStageNpcInfo.m_setBossNpcUID.end(); ++sit )
//		{
//			const int iNpcUID = *sit;
//			std::map< int, NPC_DATA >::const_iterator mitND;
//			mitND = m_mapNPCData.find( iNpcUID );
//			if( mitND == m_mapNPCData.end() )
//			{
//				START_LOG( cerr, L"���� ����Ʈ�� �ִ� NpcUID�� �˻��ߴ��� NPC_DATA�� ����?" )
//					<< BUILD_LOG( FIRST_SENDER_UID )
//					<< BUILD_LOG( m_iStageID )
//					<< BUILD_LOG( m_iSubStageID )
//					<< BUILD_LOG( iNpcUID )
//					<< END_LOG;
//				continue;
//			}
//
//			//{{ 2010. 12. 28	������	�ϸ� ���� �߰�
//#ifdef SERV_SHIP_DUNGEON
//			if( CXSLDungeon::IsShipDungeon( GetDungeonID() ) == true )
//			{
//				START_LOG( clog, L"���� �̵� �����̸� �׳� ���� ó�� �Ѵ�!" )
//					<< BUILD_LOG( GetDungeonID() );
//				break;
//			}
//#endif SERV_SHIP_DUNGEON
//			//}}
//
//			// ������ ���� �ʾҴٸ�?
//			if( mitND->second.m_bDie == false )
//			{
//				START_LOG( cerr, L"������ ������ �ʾҴµ� ��� ���� �׿��޶�� ��Ŷ�� �Գ�.. ��ŷ�ΰ�?" )
//					<< BUILD_LOG( FIRST_SENDER_UID )
//					<< BUILD_LOG( m_iStageID )
//					<< BUILD_LOG( m_iSubStageID )
//					<< BUILD_LOG( iNpcUID )
//					<< END_LOG;
//
//				kPacket.m_bResult = false;
//				break;
//			}
//		}
//	}
//
//	// Ŭ���̾�Ʈ�� ��� ����
//	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_DUNGEON_KILLALLNPC_CHECK_ACK, kPacket );
//}

//////////////////////////////////////////////////////////////////////////
#endif SERV_BATTLE_FIELD_SYSTEM
//////////////////////////////////////////////////////////////////////////
//}}

//{{ 2010. 11. 18	������	������ �Ǹ� ����
#ifdef SERV_AUTO_HACK_CHECK_SELL_ITEM
_IMPL_ON_FUNC( ERM_CHECK_SELL_ED_ITEM_REQ, KEGS_SELL_ED_ITEM_REQ )
{
	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_WAIT ), ERM_CHECK_SELL_ED_ITEM_ACK, KERM_CHECK_SELL_ED_ITEM_ACK );

	kPacket.m_iOK = NetError::NET_OK;
	kPacket.m_iItemUID = kPacket_.m_iItemUID;
	kPacket.m_iQuantity = kPacket_.m_iQuantity;
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_CHECK_SELL_ED_ITEM_ACK, kPacket );
}
#endif SERV_AUTO_HACK_CHECK_SELL_ITEM
//}}

#ifdef SERV_CREATE_ELITE_FOR_ADMIN
_IMPL_ON_FUNC( ERM_ADMIN_NPC_UNIT_CREATE_REQ, KEGS_ADMIN_NPC_UNIT_CREATE_REQ )
{
	VERIFY_ROOM_STATE_AND_WARN( ( 1, KRoomFSM::S_PLAY ), (1, KRoomFSM::S_RESULT), ERM_NPC_UNIT_CREATE_ACK, KPacketOK );

	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_ADMIN_NPC_UNIT_CREATE_ACK, kPacket );

	KEGS_ADMIN_NPC_UNIT_CREATE_NOT kPacketNot;
	kPacketNot.m_kNPCUnitNot = kPacket_.m_kNPCUnitReq;
	kPacketNot.m_cAttrib1 = kPacket_.m_cAttrib1;
	kPacketNot.m_cAttrib2 = kPacket_.m_cAttrib2;
	kPacketNot.m_cSA1 = kPacket_.m_cSA1;
	kPacketNot.m_cSA2 = kPacket_.m_cSA2;
	
// #ifdef SERV_CREATED_NPC_LIMITED_DROPS
// 	// �ϴ��� ������ ���� ó�� // �ϴ� �����ص�
// 	bool bTempNoDrop = kPacket_.m_kNPCUnitReq.m_bNoDrop;
// 	kPacket_.m_kNPCUnitReq.m_bNoDrop = true;
// 
// 	int iNpcDropTime = SiCXSLDungeonManager()->GetNPCDropTime( GetDungeonIDAndDif(), GetStageID(), GetSubStageID(), kPacket_.m_kNPCUnitReq.m_NPCID );
// 	if( iNpcDropTime > 0 )
// 	{
// 		NPC_INFO_QUADRUPLE npcInfoQuadruple;
// 		npcInfoQuadruple.iDungeonID		= GetDungeonIDAndDif();
// 		npcInfoQuadruple.iStageID		= GetStageID();
// 		npcInfoQuadruple.iSubStageID	= GetSubStageID();
// 		npcInfoQuadruple.iNpcID			= kPacket_.m_kNPCUnitReq.m_NPCID;
// 
// 		// ��ȯ��û�Ǿ��� ���� ��ü ��������� �� �̸��� ���� ��� ����
// 		if( m_mapDroppedTimes[npcInfoQuadruple] < iNpcDropTime )
// 		{
// 			m_mapDroppedTimes[npcInfoQuadruple] = m_mapDroppedTimes[npcInfoQuadruple] + 1;
// 			kPacket_.m_kNPCUnitReq.m_bNoDrop = bTempNoDrop;
// 		}
// 	}
//#endif SERV_CREATED_NPC_LIMITED_DROPS
    
	// ���� ���� �Լ� ȣ��!
	LIF( m_kDungeonMonsterManager.CreateMonster( kPacket_.m_kNPCUnitReq, kPacketNot.m_kNPCUnitNot.m_UID ) );	

	// �Ӽ� ������ ������� 
	if( kPacket_.m_cAttrib1 != 0 || kPacket_.m_cAttrib2 != 0 || kPacket_.m_cSA1 != 0 || kPacket_.m_cSA2 != 0 )
	{
		m_mapAttribNpcData.insert( std::make_pair( kPacketNot.m_kNPCUnitNot.m_UID, static_cast<int>(kPacketNot.m_kNPCUnitNot.m_Level) ) );
	}

	BroadCast( ERM_ADMIN_NPC_UNIT_CREATE_NOT, kPacketNot );
}
#endif SERV_CREATE_ELITE_FOR_ADMIN

//{{ 2011. 01. 25  ��μ�  ���� Ʈ���� ����(������ ������ - ���� PVP Ȯ��)
#ifdef SERV_WORLD_TRIGGER_RELOCATION
_IMPL_ON_FUNC( ECN_WORLD_TRIGGER_RELOCATION_REQ, KEGS_WORLD_TRIGGER_RELOCATION_REQ )
{
	VERIFY_ROOM_STATE( ( 1, KRoomFSM::S_PLAY ), ECN_WORLD_TRIGGER_RELOCATION_ACK, KEGS_WORLD_TRIGGER_RELOCATION_ACK );

	// ��ȿ� ��� �������� ��ε� ĳ��Ʈ	
	KEGS_WORLD_TRIGGER_RELOCATION_NOT _Not;
	_Not.m_iWorldTrigger = kPacket_.m_iWorldTrigger;
	BroadCast( ECN_WORLD_TRIGGER_RELOCATION_NOT, _Not );
	
	// GSUser���� ACK����
	kPacket.m_iOK = NetError::NET_OK;
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ECN_WORLD_TRIGGER_RELOCATION_ACK, kPacket );
}
#endif SERV_WORLD_TRIGGER_RELOCATION
//}}

//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER
int KDungeonRoom::GetProtectedSmallCrystalCount()
{	
	const int iCount = m_setDefenseSmallCrystalNpcUID.size();
	return ( m_bIsWin == true ) ? iCount : 0;
}
#endif SERV_INSERT_GLOBAL_SERVER
//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�

//{{ 2011. 08. 12   ��μ�      ��ϸ� ���� 
#ifdef SERV_NEW_HENIR_TEST
void KDungeonRoom::CheckDeleteHenirDropItem( IN OUT KDropTable::DROP_DATA& sDropDataNpc, 
											 IN OUT KDropTable::DROP_DATA& sDropDataStatic, 
											 IN OUT KDropTable::DROP_DATA& sDropDataEvent, 
											 IN OUT KDropTable::DROP_DATA& sDropDataAttribNpc,
											 IN OUT KDropTable::DROP_DATA& sDropDataHenir )
{
	std::vector< int > vecItemID;
	std::vector< int >::iterator vit;
	
	// 1. sDropDataNpc �˻�
	vecItemID = sDropDataNpc.m_vecItemID;
	sDropDataNpc.Clear();

	for( vit = vecItemID.begin(); vit != vecItemID.end() ; ++vit )
	{
		const CXSLItem::ItemTemplet* pItemTemplet = SiCXSLItemManager()->GetItemTemplet( *vit );
		if( pItemTemplet == NULL )
		{
			START_LOG( cerr, L"sDropDataNpc �˻� ������ ������ ����." )
				<< BUILD_LOG( *vit );
			continue;
		}

		if( pItemTemplet->m_ItemType == CXSLItem::IT_OUTLAY )
		{
			sDropDataNpc.m_vecItemID.push_back( *vit );
		}
	}

	// 2. sDropDataStatic �˻�
	vecItemID = sDropDataStatic.m_vecItemID;
	sDropDataStatic.Clear();

	for( vit = vecItemID.begin(); vit != vecItemID.end() ; ++vit )
	{
		const CXSLItem::ItemTemplet* pItemTemplet = SiCXSLItemManager()->GetItemTemplet( *vit );
		if( pItemTemplet == NULL )
		{
			START_LOG( cerr, L"sDropDataStatic �˻� ������ ������ ����." )
				<< BUILD_LOG( *vit );
			continue;
		}

		if( pItemTemplet->m_ItemType == CXSLItem::IT_OUTLAY )
		{
			sDropDataStatic.m_vecItemID.push_back( *vit );
		}
	}

	// 3. sDropDataEvent �˻�
	vecItemID = sDropDataEvent.m_vecItemID;
	sDropDataEvent.Clear();

	for( vit = vecItemID.begin(); vit != vecItemID.end() ; ++vit )
	{
		const CXSLItem::ItemTemplet* pItemTemplet = SiCXSLItemManager()->GetItemTemplet( *vit );
		if( pItemTemplet == NULL )
		{
			START_LOG( cerr, L"sDropDataEvent �˻� ������ ������ ����." )
				<< BUILD_LOG( *vit );
			continue;
		}

		if( pItemTemplet->m_ItemType == CXSLItem::IT_OUTLAY )
		{
			sDropDataEvent.m_vecItemID.push_back( *vit );
		}
	}

	// 4. sDropDataAttribNpc �˻�
	vecItemID = sDropDataAttribNpc.m_vecItemID;
	sDropDataAttribNpc.Clear();

	for( vit = vecItemID.begin(); vit != vecItemID.end() ; ++vit )
	{
		const CXSLItem::ItemTemplet* pItemTemplet = SiCXSLItemManager()->GetItemTemplet( *vit );
		if( pItemTemplet == NULL )
		{
			START_LOG( cerr, L"sDropDataAttribNpc �˻� ������ ������ ����." )
				<< BUILD_LOG( *vit );
			continue;
		}

		if( pItemTemplet->m_ItemType == CXSLItem::IT_OUTLAY )
		{
			sDropDataAttribNpc.m_vecItemID.push_back( *vit );
		}
	}

	// 5. sDropDataAttribNpc �˻�
	vecItemID = sDropDataHenir.m_vecItemID;
	sDropDataHenir.Clear();

	for( vit = vecItemID.begin(); vit != vecItemID.end() ; ++vit )
	{
		const CXSLItem::ItemTemplet* pItemTemplet = SiCXSLItemManager()->GetItemTemplet( *vit );
		if( pItemTemplet == NULL )
		{
			START_LOG( cerr, L"sDropDataAttribNpc �˻� ������ ������ ����." )
				<< BUILD_LOG( *vit );
			continue;
		}

		if( pItemTemplet->m_ItemType == CXSLItem::IT_OUTLAY )
		{
			sDropDataHenir.m_vecItemID.push_back( *vit );
		}
	}
}
#endif SERV_NEW_HENIR_TEST
//}}

//{{ 2011. 11. 1	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
void KDungeonRoom::OnLeaveRoom( IN const UidType iLAST_SENDER_UID, IN const UidType iFIRST_SENDER_UID, IN const KERM_LEAVE_ROOM_REQ& kPacket_, IN const u_short usEventIDAck )
{
	KRoom::OnLeaveRoom( iLAST_SENDER_UID, iFIRST_SENDER_UID, kPacket_, usEventIDAck );

	//{{ 2013. 02. 25   ���� ��Ż �� ��ũ ��� - ��μ�
#ifdef SERV_LEAVE_ROOM_DUNGEON_RANK
	const int iEndNumMember = m_spRoomUserManager->GetNumMember();
	int iPartyTotalGivenDamage = 0;
	for( int idx = 0; idx < m_spRoomUserManager->GetNumMember(); ++idx )
	{
		KRoomUserPtr pkRoomUser  = m_spRoomUserManager->GetUser( idx );
		_JIF( pkRoomUser != NULL, continue; );

		iPartyTotalGivenDamage	+= pkRoomUser->GetTotalGivenDamamge_LUA();
	}
#endif SERV_LEAVE_ROOM_DUNGEON_RANK
	//}

	
	// ��Ż�� ���.
	if( GetStateID() == KRoomFSM::S_LOAD || GetStateID() == KRoomFSM::S_PLAY )
	{

		//=== ������ ��Ż ���ó�� ===
		START_LOG( clog, L"=== DUNGEON GAME : ������ ��Ż�� ���ó�� ===" )
			<< BUILD_LOG( iFIRST_SENDER_UID )
			<< END_LOG;

		KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser( iFIRST_SENDER_UID );

		if( !spRoomUser )
		{
			spRoomUser = m_spRoomUserManager->GetUser( iFIRST_SENDER_UID, KRoomUserManager::UT_OBSERVER );
		}

		if( !spRoomUser )
		{
			START_LOG( cerr, L"�� ���� ������ �̻�" )
				<< BUILD_LOG( iFIRST_SENDER_UID )
				<< END_LOG;
		}
		else
		{
			if( spRoomUser->IsObserver() == false )
			{
				//Ʃ�丮�� ����̸� �г�Ƽ�� ����..
				int iEXP = 0;
				int iED = 0;
				if( CXSLDungeon::IsTutorialDungeon( m_iDungeonID ) == false )
				{
					KERM_UPDATE_DUNGEON_UNIT_INFO_NOT kPacket;
					kPacket.m_bOutRoom = true;
					kPacket.m_iUserCount = GetNumMember();
					kPacket.m_iDungeonID = m_iDungeonID + static_cast<int>(m_cDifficultyLevel);
					kPacket.m_iPlayTime = ( int )( m_kTimer[TM_PLAY].elapsed() + 0.5 - GetLoadingTime() );
					kPacket.m_iEXP = 0;

					if( spRoomUser->IsDie() == true )
					{
						// �ǽð� ����ġ
						m_spRoomUserManager->GetRewardEXP( iFIRST_SENDER_UID, iEXP );
						m_spRoomUserManager->GetRewardPartyEXP( iFIRST_SENDER_UID, iEXP );

						//���κ� ������ �����ֱ�
						m_spRoomUserManager->GetItemList( iFIRST_SENDER_UID, kPacket.m_mapGetItem );
					}
					else
					{
						// ED���Ƽ
						SiKResultProcess()->Result_DUNGEON_BREAK( spRoomUser, kPacket );
					}

					// [�߿�] Ŭ���̾�Ʈ ũ���� �� ��ŷ���� ���� �� ��Ż�� ��� ���Ӽ����� ������Ŷ�� ������ �ʽ��ϴ�!
					if( kPacket_.m_iReason != NetError::NOT_LEAVE_ROOM_REASON_23 )
					{
						SendToGSCharacter( iLAST_SENDER_UID, iFIRST_SENDER_UID, ERM_UPDATE_DUNGEON_UNIT_INFO_NOT, kPacket );

						//{{ 2009. 5. 14  ������	������ ����
						KERM_DECREASE_ENDURANCE_DUNGEON_NOT kNot;
						kNot.m_iEnduranceDamage = CXSLDungeon::DE_LEAVE_ROOM_ENDURANCE_DAMAGE;
						kNot.m_cPartyNumMember  = kPacket.m_iUserCount;
						SendToGSCharacter( iLAST_SENDER_UID, iFIRST_SENDER_UID, ERM_DECREASE_ENDURANCE_DUNGEON_NOT, kNot );
						//}}
					}

					iED = kPacket.m_iED;
				}

				//{{ 2012. 11. 9	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
				int iClearValue = ( spRoomUser->IsDie() ? KE_LOCAL_LOG_DUNGEON_NOT::DCT_UNIT_DIE_LEAVE_ROOM : KE_LOCAL_LOG_DUNGEON_NOT::DCT_UNIT_ALIVE_LEAVE_ROOM ); // 2. ĳ���� �װ� ��Ż   3. ĳ���� ���װ� ��Ż
				
				if( kPacket_.m_iReason == NetError::NOT_LEAVE_ROOM_REASON_23 ) // 4. Ŭ���̾�Ʈ ũ���� ��Ż
				{
					iClearValue = KE_LOCAL_LOG_DUNGEON_NOT::DCT_CLIENT_CRASH_LEAVE_ROOM;
				}
				else if( kPacket_.m_iReason == NetError::NOT_LEAVE_ROOM_REASON_25 ) // 5. Ŭ���̾�Ʈ ��ŷ���� ���� ��Ż
				{
					iClearValue = KE_LOCAL_LOG_DUNGEON_NOT::DCT_CLIENT_HACKING_LEAVE_ROOM;
				}

				// ���� �α�
				KERM_UPDATE_DUNGEON_UNIT_INFO_NOT kUpdateUnitInfoDummy;
				KDungeonUnitResultInfo kDungeonUnitInfoDummy;

				//{{ 2013. 02. 25   ���� ��Ż �� ��ũ ��� - ��μ�
#ifdef SERV_LEAVE_ROOM_DUNGEON_RANK
				SiKResultProcess()->LeaveRoomResultRank( m_iDungeonID, m_cDifficultyLevel, spRoomUser, m_kDungeonMonsterManager, iEndNumMember, iPartyTotalGivenDamage, (int)GetTotalPlayTime(), kUpdateUnitInfoDummy, kDungeonUnitInfoDummy );
#endif SERV_LEAVE_ROOM_DUNGEON_RANK
				//}

				//{{ 2012. 12. 12	������	��Ʋ�ʵ� �ý���
#ifdef SERV_CHECK_USER_NEVER_RETURN_TO_FIELD
				LogToDB_DungeonLog( spRoomUser, iClearValue, kUpdateUnitInfoDummy, kDungeonUnitInfoDummy, kPacket_.m_iReason );
#else
				LogToDB_DungeonLog( spRoomUser, iClearValue, kUpdateUnitInfoDummy, kDungeonUnitInfoDummy );
#endif SERV_CHECK_USER_NEVER_RETURN_TO_FIELD
				//}}
#else
//				//////////////////////////////////////////////////////////////////////////
//				// ���� ���� ��Ż �α�
//				// ��ڴ� ��踦 ������ �ʽ��ϴ�.
//				//if( spRoomUser->GetAuthLevel() >= SEnum::UAL_GM )
//				//{
//				//	START_LOG( cout, L"��ڴ� ��赥���͸� ������ �ʽ��ϴ�." )
//				//		<< BUILD_LOGc( spRoomUser->GetAuthLevel() )
//				//		<< BUILD_LOG( spRoomUser->GetNickName() );
//				//}
//				//else
//				{
//					KRoomUserInfo kInfo;
//					spRoomUser->GetRoomUserInfo( kInfo );
//
//					int iClear = ( spRoomUser->IsDie() ? KE_LOCAL_LOG_DUNGEON_NOT::DCT_UNIT_DIE_LEAVE_ROOM : KE_LOCAL_LOG_DUNGEON_NOT::DCT_UNIT_ALIVE_LEAVE_ROOM ); // 2. ĳ���� �װ� ��Ż   3. ĳ���� ���װ� ��Ż
//
//					if( kPacket_.m_iReason == NetError::NOT_LEAVE_ROOM_REASON_23 ) // 4. Ŭ���̾�Ʈ ũ���� ��Ż
//					{
//						iClear = KE_LOCAL_LOG_DUNGEON_NOT::DCT_CLIENT_CRASH_LEAVE_ROOM;
//					}
//					else if( kPacket_.m_iReason == NetError::NOT_LEAVE_ROOM_REASON_25 ) // 5. Ŭ���̾�Ʈ ��ŷ���� ���� ��Ż
//					{
//						iClear = KE_LOCAL_LOG_DUNGEON_NOT::DCT_CLIENT_HACKING_LEAVE_ROOM;
//					}
//
//					//{{ 2010. 06. 21  ������	��� ������
//					//#ifdef SERV_STATISTICS_THREAD
//					//{{ 2010. 11. 8	������	���� �α� DB
//					//#ifdef SERV_DUNGEON_LOG_DB
//					CTime kRegDate = CTime::GetCurrentTime();
//					//#endif SERV_DUNGEON_LOG_DB
//					//}}
//					//////////////////////////////////////////////////////////////////////////					
//					KE_LOCAL_LOG_DUNGEON_NOT kNot;
//					kNot.m_iDungeonID				= (m_iDungeonID + static_cast<int>(m_cDifficultyLevel));
//					kNot.m_nUnitUID					= kInfo.m_nUnitUID;
//					kNot.m_wstrNickName				= kInfo.m_wstrNickName;
//					kNot.m_iDifficultyLevel			= (int) m_cDifficultyLevel;
//					kNot.m_iIsChallenge				= ( m_cDungeonMode == CXSLDungeon::DM_HENIR_CHALLENGE ? 1 : 0 );
//					kNot.m_iUnitClass				= (int) kInfo.m_cUnitClass;
//					kNot.m_iClear					= iClear;
//					kNot.m_iStartNumMember			= m_iStartedNumMember;
//					kNot.m_iEndNumMember			= GetNumMember();
//					kNot.m_ucLevel					= kInfo.m_ucLevel;
//					kNot.m_iPlayTime				= (int) spRoomUser->GetPlayTime();
//					kNot.m_iEXP						= iEXP;
//					kNot.m_iED						= iED;
//					kNot.m_cComboRank				= CXSLDungeon::RT_F;
//					kNot.m_iComboScore				= spRoomUser->GetComboScore_LUA();
//					kNot.m_cTechnicalRank			= CXSLDungeon::RT_F;
//					kNot.m_iTechnicalScore			= spRoomUser->GetTechnicalScore_LUA();
//					kNot.m_cTimeRank				= CXSLDungeon::RT_F;
//					kNot.m_cDamagedRank				= CXSLDungeon::RT_F;
//					kNot.m_iDamageNum				= spRoomUser->GetDamageNum_LUA();
//					kNot.m_cTotalRank				= CXSLDungeon::RT_F;
//					kNot.m_iRessurectionStoneCount	= spRoomUser->GetUsedRessurectionStoneCount();
//					kNot.m_iPassedStageCount		= spRoomUser->GetPassedStageCount();
//					kNot.m_iPassedSubStageCount		= spRoomUser->GetPassedSubStageCount();
//					//{{ 2010. 9. 14	������	�� �ý���
//#ifdef SERV_PET_SYSTEM
//					kNot.m_bIsWithPet				= spRoomUser->HavePet();
//#endif SERV_PET_SYSTEM
//					//}}
//					//{{ 2010. 11. 8	������	���� �α� DB
//#ifdef SERV_DUNGEON_LOG_DB
//					kNot.m_wstrRegDate				= (const wchar_t*)kRegDate.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
//#endif SERV_DUNGEON_LOG_DB
//					//}}
//					//{{ 2011. 03. 16	������	���� �α� �÷� �߰�
//#ifdef SERV_ADD_DUNGEON_LOG_COLUMN
//					kNot.m_sNpcDieCount				= spRoomUser->GetKillNPC_LUA();
//					kNot.m_iGivenDamage				= 0;
//					kNot.m_iAttackDamage			= 0;
//					kNot.m_sUsingSkillKind			= 0;
//					kNot.m_iTotalEXP				= ( spRoomUser->GetRewardEXP() + spRoomUser->GetRewardPartyEXP() );
//					kNot.m_cPetID					= spRoomUser->GetPetID();
//					kNot.m_cPetEvoStep				= spRoomUser->GetPetEvolutionStep();
//#endif SERV_ADD_DUNGEON_LOG_COLUMN
//					//}}
//					//{{ 2012. 04. 17	������	���� �α� �÷� �߰�
//#ifdef SERV_ADD_DUNGEON_LOG_COLUMN_NUM_2
//					kNot.m_iBaseHP					= spRoomUser->GetBaseHP_LUA();
//					kNot.m_iEndHP					= spRoomUser->GetLastUpdatedHP();
//					kNot.m_iRecovHP					= spRoomUser->GetTotalRecoveryHP_LUA();
//					kNot.m_iContinue				= spRoomUser->GetUsedRessurectionStoneCount();
//					kNot.m_iStartContinue			= spRoomUser->GetStartTimeNumResurrectionStone();
//					kNot.m_iContinueUseStage		= spRoomUser->GetFirstUseResurrectionStoneStageID();
//					kNot.m_iContinueUseSub			= spRoomUser->GetFirstUseResurrectionStoneSubStageID();
//					kNot.m_iFrame					= spRoomUser->GetInGameAverageFrame();
//					kNot.m_wstrChannelIP			= spRoomUser->GetGameChannelIP();
//#endif SERV_ADD_DUNGEON_LOG_COLUMN_NUM_2
//					//}}
//					//{{ 2012. 07. 11 ��μ�
//#ifdef SERV_ADD_DUNGEON_LOG_COLUMN_NUM_3
//					kNot.m_iUserUID					= spRoomUser->GetUserUID();
//#endif SERV_ADD_DUNGEON_LOG_COLUMN_NUM_3
//					//}}
//					//{{ 2012. 11. 6	������	��Ʋ�ʵ� �ý���
//#ifdef SERV_BATTLE_FIELD_SYSTEM
//					kNot.m_cAutoPartyPlay			= ( IsStartedByAutoParty() ? 1 : 0 );
//#endif SERV_BATTLE_FIELD_SYSTEM
//					//}}
//					KSIManager.QueueingEvent( E_LOCAL_LOG_DUNGEON_NOT, kNot );
//					//////////////////////////////////////////////////////////////////////////
//					//{{ 2010. 11. 8	������	���� �α� DB
//					//#ifdef SERV_DUNGEON_LOG_DB
//					SendToLogDB( DBE_DB_LOG_DUNGEON_NOT, kNot );
//					//#endif SERV_DUNGEON_LOG_DB
//					//}}
//					//#endif SERV_STATISTICS_THREAD
//					//}}
//				}
//				//////////////////////////////////////////////////////////////////////////
#endif SERV_BATTLE_FIELD_SYSTEM
				//}}

				

				//{{ 2007. 12. 18  ������  ���� ��� [����]
				KERM_DUNGEON_USER_STATISTICS_NOT kPacketNot;
				kPacketNot.m_iDungeonID	   = m_iDungeonID + static_cast<int>(m_cDifficultyLevel);
				kPacketNot.m_cGameResult   = KERM_DUNGEON_USER_STATISTICS_NOT::UGR_DROP;
				kPacketNot.m_bIsPartyPlay  = ( m_iStartedNumMember > 1 );
				kPacketNot.m_iGamePlayTime = ( int )( m_kTimer[TM_PLAY].elapsed() + 0.5 - GetLoadingTime() );
				kPacketNot.m_iUsedResurrectionStone = m_spRoomUserManager->GetUsedRessurectionStoneCount( iFIRST_SENDER_UID );
				SendToGSCharacter( iLAST_SENDER_UID, iFIRST_SENDER_UID, ERM_DUNGEON_USER_STATISTICS_NOT, kPacketNot );
				//}}

				//{{ 2007. 12. 25  ������  ��Ȱ�� ���
				if( m_spRoomUserManager->GetUsedRessurectionStoneCount( iFIRST_SENDER_UID ) > 0 )
				{
					KStatisticsKey kKey;
					kKey.m_vecIntKey.push_back( 0 );
					KSIManager.IncreaseCount( KStatistics::SI_RES_STONE, kKey, KStatistics::eSIColResStone_PlayerCount, 1 );
				}
				//}}
			}
		}
	}

	KEGS_LEAVE_ROOM_ACK kPacket;
	kPacket.m_iOK = NetError::NET_OK;
	kPacket.m_iItemID = kPacket_.m_iItemID;
	kPacket.m_kBattleFieldJoinInfo = kPacket_.m_kBattleFieldJoinInfo;

	//{{ 2011. 02. 21  ��μ�	Ư�� ���� ���� ������ ���� ����
	//#ifdef SERV_DUNGEON_REQUIRED_ITEM_BUG
	//�������� ����������� ������� ó��..
	if( kPacket_.m_iReason == NetError::NOT_LEAVE_ROOM_REASON_22 )
	{
		kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_22;
	}
	else if( kPacket_.m_iReason == NetError::NOT_LEAVE_ROOM_REASON_27 )
	{
		kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_27;
	}
	else
	{
		kPacket.m_iReason = NetError::NET_OK;
	}
	//#endif SERV_DUNGEON_REQUIRED_ITEM_BUG
	//}}

	//{{ 2012. 01. 31	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	// ��� ���°ų� �ʵ�� ������ �����϶� ��Ż�ϴ°Ŷ��
	if( GetStateID() == KRoomFSM::S_WAIT  ||  GetStateID() == KRoomFSM::S_RETURN_TO_FIELD )
	{
		// ��Ƽ ��Ż ó���� ���� �ʴ´�.
		kPacket.m_bNotLeaveParty = true;
	}
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}

	//{{ 2012. 12. 18	������	�ƶ� ��Ƽ �÷��� ���ʽ� ����ġ
#ifdef SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
	const bool bPlayWithCharLeaveBefore = m_spRoomUserManager->IsExistCharType( CXSLUnit::UT_ARA );
#endif SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
	//}}

	// ���� ���� ����.
	if( !m_spRoomUserManager->LeaveRoom( iFIRST_SENDER_UID ) )
	{
		// ���Կ��� ������ ����.
		kPacket.m_iOK = NetError::ERR_SLOT_04;
		SendToGSCharacter( iLAST_SENDER_UID, iFIRST_SENDER_UID, usEventIDAck, kPacket );
		return;
	}
	else
	{
		//{{ 2013. 01. 09 ���� ���� �ý��� - ��μ�
#ifdef SERV_DUNGEON_FORCED_EXIT_SYSTEM
		if( kPacket_.m_iReason == NetError::NOT_LEAVE_ROOM_REASON_34 )
		{
			kPacket.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_34;
		}
#endif SERV_DUNGEON_FORCED_EXIT_SYSTEM
		//}

		SendToGSCharacter( iLAST_SENDER_UID, iFIRST_SENDER_UID, usEventIDAck, kPacket );

		//{{ 2012. 10. 16	������		P2P & Relay ���� ����
#ifdef SERV_COLLECTION_OF_RELAY_AND_P2P_INFO
		LogToDB_GamePlayNetWorkInfo( iFIRST_SENDER_UID, kPacket_.m_kGamePlayNetworkInfo, true );
#endif SERV_COLLECTION_OF_RELAY_AND_P2P_INFO
		//}}

		//{{ 2012. 12. 18	������	�ƶ� ��Ƽ �÷��� ���ʽ� ����ġ
#ifdef SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
		if( bPlayWithCharLeaveBefore == true )
		{
			// ��Ż ����ڴ� ���� �Ҹ�
			KERM_DEACTIVATE_BUFF_NOT kPacketNot;
			kPacketNot.m_vecDeactivateBuffID.push_back( CXSLBuffManager::BTI_SECRET_OF_THOUSAND_YEARS_FOX );
			SendToGSCharacter( iLAST_SENDER_UID, iFIRST_SENDER_UID, ERM_DEACTIVATE_BUFF_NOT, kPacketNot );

			// �ƶ� ��Ƽ�� �ƿ� ���ٸ�?
			const bool bPlayWithCharLeaveAfter = m_spRoomUserManager->IsExistCharType( CXSLUnit::UT_ARA );
			if( bPlayWithCharLeaveAfter == false )
			{
                // ��Ƽ�� ��ü���� ���� �Ҹ�
				BroadCast( ERM_DEACTIVATE_BUFF_NOT, kPacketNot );
			}
		}
#endif SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
		//}}
	}

	//{{ 2013. 01. 09 ���� ���� �ý��� - ��μ�
#ifdef SERV_DUNGEON_FORCED_EXIT_SYSTEM
	KBadAttitudeManager::KBadAttitudeInfo kUnitData( iFIRST_SENDER_UID );
	m_kBadAttitudeManager.RemoveUnit( iFIRST_SENDER_UID, kUnitData );
#endif SERV_DUNGEON_FORCED_EXIT_SYSTEM
	//}

	if( IsEmpty() == false || IsEmpty( KRoomUserManager::UT_OBSERVER ) == false )
	{
		// ���� ��� �����鿡�� ���� ������ ������.
		KEGS_LEAVE_ROOM_NOT kPacketNot;
		kPacketNot.m_cRoomState = GetStateID();
		kPacketNot.m_iUnitUID = iFIRST_SENDER_UID;
		GetRoomSlotInfo( kPacketNot.m_vecSlot );
		GetRoomSlotInfo( kPacketNot.m_vecObserverSlot, KRoomUserManager::UT_OBSERVER );
		BroadCast( ERM_LEAVE_ROOM_NOT, kPacketNot );
	}

	if( (GetStateID() == KRoomFSM::S_LOAD || GetStateID() == KRoomFSM::S_PLAY) && IsEmpty() == false )
	{
		//{{ 2010. 11. 23	������	�� �ε� ���� ���� - ũ���� ������ �־ ������ �����մϴ�.
#ifdef SERV_ROOM_LOADING_BUG_FIX
		//////////////////////////////////////////////////////////////////////////
		if( GetStateID() == KRoomFSM::S_LOAD )
		{
			if( m_spRoomUserManager->IsAllPlayerFinishLoading() == true )
			{
				BroadCastID( ERM_GAME_LOADING_ALL_UNIT_OK_NOT );

				// ���������� �ʱ�ȭ.
				StartPlay();

				KEGS_PLAY_START_NOT kNot;
				//{{ 2011. 01. 17	������	ĳ���� ī��Ʈ ����
#ifdef SERV_CHAR_LOG
				kNot.m_iDungeonID = GetDungeonIDAndDif();
#endif SERV_CHAR_LOG
				//}}
				kNot.m_RoomState = GetStateID();
				GetRoomSlotInfo( kNot.m_vecSlot );

				// ��� �������� �÷��� ������ �˸�.
				BroadCast( ERM_PLAY_START_NOT, kNot );

				// ��� ���� ������ �� ����Ʈ�� �����϶�� ������.
				SendRoomListInfo( NetError::ERR_ROOM_15 );

				//{{ 2009. 4. 23  ������	������������Ʈ
				SendAllPVPUserInfo();
				//}}
			}
		}
		else if( GetStateID() == KRoomFSM::S_PLAY )
		{
			CheckLoadingSuccess();
		}
		//////////////////////////////////////////////////////////////////////////	
#else
		//////////////////////////////////////////////////////////////////////////
		CheckLoadingSuccess();
		//////////////////////////////////////////////////////////////////////////
#endif SERV_ROOM_LOADING_BUG_FIX
		//}}
	}

	// ���������� ������ �Ǵ� ��� ó��.
	if( GetStateID() == KRoomFSM::S_PLAY && IsEmpty() == false )
	{
		if( CheckIfPlayEnd() )
		{
			EndPlay();
		}
		else
		{
			//{{ 2009. 7. 27  ������	��н�������
			if( m_eDungeonType == CXSLDungeon::DT_HENIR )
			{
				CheckEnterSecretStageSuccess();
			}
			//}}
		}
	}

	//���â�� ���������� ���� �������ó��
	if( GetStateID() == KRoomFSM::S_RESULT && IsEmpty() == false )
	{
		CheckResultSuccess();
	}

	//{{ 2012. 12. 12	������	���â ��Ż ���� �α�
#ifdef SERV_CHECK_USER_NEVER_RETURN_TO_FIELD
	if( GetStateID() == KRoomFSM::S_RESULT )
	{
		CTime kCurTime = CTime::GetCurrentTime();
		KDBE_DUNGEON_RESULT_LEAVE_LOG_NOT kPacketToLog;
		kPacketToLog.m_iUnitUID = iFIRST_SENDER_UID;
		kPacketToLog.m_wstrLeaveReason = NetError::GetErrStr( kPacket_.m_iReason );
		kPacketToLog.m_wstrRoomState = GetStateIDString();
		kPacketToLog.m_wstrRegDate = (const wchar_t*)kCurTime.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
		SendToLogDB( DBE_DUNGEON_RESULT_LEAVE_LOG_NOT, kPacketToLog );
	}
#endif SERV_CHECK_USER_NEVER_RETURN_TO_FIELD
	//}}

	//{{ 2013. 04. 22	������	����� �� ���� ���� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON_ENTER_LOGIC
	if( GetStateID() == KRoomFSM::S_WAIT_FOR_DEFENCE )
	{
		// ��� ������ ������ ���ŵǾ����� Ȯ���Ѵ�!
		std::vector< UidType > vecLeaveUnitUID;
		if( m_spRoomUserManager->IsAllPlayerPrepareForDefenceDungeon( vecLeaveUnitUID ) == true )
		{
			// ����� �� ���� ����!
			EnterDefenceDungeon( vecLeaveUnitUID );

			START_LOG( clog, L"�� ��Ż�ڿ� ���� ����� �� ������ �����մϴ�!" );
		}
	}
#else
	//{{ 2013. 04. 15	������	����� �� ����
//#ifdef SERV_NEW_DEFENCE_DUNGEON
//	if( GetStateID() == KRoomFSM::S_WAIT_FOR_DEFENCE )
//	{
//		// ��� ������ ������ ���ŵǾ����� Ȯ���Ѵ�!
//		if( m_spRoomUserManager->IsAllPlayerPrepareForDefence() == true )
//		{
//			// ����� �� ���� ����!
//			EnterDefenceDungeon();
//		}
//	}
//#endif SERV_NEW_DEFENCE_DUNGEON
	//}}
#endif SERV_NEW_DEFENCE_DUNGEON_ENTER_LOGIC
	//}}

	// �濡�� ��� ������ �������� ���� �ݴ´�.
	if( IsEmpty() == true && IsEmpty( KRoomUserManager::UT_OBSERVER ) == true )
	{
		StateTransition( KRoomFSM::I_TO_CLOSE );

		//{{ 2010. 06. 21  ������	��� ������
#ifdef SERV_STATISTICS_THREAD
		CTime kEndGameTime = CTime::GetCurrentTime();
		//////////////////////////////////////////////////////////////////////////
		KE_LOCAL_LOG_DUNGEON_ROOM_NOT kNot;
		kNot.m_iDungeonID		= m_iDungeonID + static_cast<int>(m_cDifficultyLevel);
		kNot.m_iDifficultyLevel	= (int) GetDifficultyLevel();
		kNot.m_iIsChallenge		= ( m_cDungeonMode == CXSLDungeon::DM_HENIR_CHALLENGE ? 1 : 0 );
		kNot.m_iWin				= 0;
		kNot.m_iLose			= 0;
		kNot.m_iStartNumMember	= m_iStartedNumMember;
		kNot.m_iEndNumMember	= GetNumMember();
		kNot.m_iTotalPlayTime	= static_cast<int>(GetTotalPlayTime());
		kNot.m_wstrEndGameTime	= (const wchar_t*)kEndGameTime.Format(_T("%H:%M:%S"));
		KSIManager.QueueingEvent( E_LOCAL_LOG_DUNGEON_ROOM_NOT, kNot );
		//////////////////////////////////////////////////////////////////////////
#endif SERV_STATISTICS_THREAD
		//}}
	}
	else
	{
		if( m_kDSSGoNextAllNot.m_iBeforeStageID != -1 &&
			m_kDSSGoNextAllNot.m_iNextStageID	!= -1 )
			SendDSSGoNextAllNot( m_kDSSGoNextAllNot );
	}
}
#endif SERV_BATTLE_FIELD_SYSTEM
//}}

//{{ 2012. 02. 29	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
bool KDungeonRoom::AutoPartyBuff( IN OUT std::map< UidType, std::vector< KBuffInfo > >& mapActivateBuffList )
{
	bool bMaxPartyFever = false;
	bool bExistComeBackUser = m_spRoomUserManager->IsComeBackUserInRoom();
	const int iNumMember = m_spRoomUserManager->GetNumMember();

	for( int iIndex = 0; iIndex < iNumMember; ++iIndex )
	{
		KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser( iIndex );
		if( spRoomUser == NULL )
		{
			START_LOG( cerr, L"�������� �ʴ� �����Դϴ�!" )
				<< BUILD_LOG( iIndex )
				<< END_LOG;
			continue;
		}

		// �Ĺ� ������ �ִ�.
		if( bExistComeBackUser == true )
		{
			if( spRoomUser->IsComeBackUser() == false )
			{
				std::map< UidType, std::vector< KBuffInfo > >::iterator mit = mapActivateBuffList.find( spRoomUser->GetCID() );
				if( mit == mapActivateBuffList.end() )
				{
					// �ش� ������ ���� ������ ����
					std::vector< KBuffInfo > ActivateBuffList;

					KBuffInfo kBuff;
					kBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = CXSLBuffManager::BTI_BUFF_RETURN_OF_HERO;
					ActivateBuffList.push_back( kBuff );

					mapActivateBuffList.insert( std::make_pair( spRoomUser->GetCID(), ActivateBuffList ) );
				}
				else
				{
					KBuffInfo kBuff;
					kBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = CXSLBuffManager::BTI_BUFF_RETURN_OF_HERO;
					mit->second.push_back( kBuff );
				}
			}
		}

		// ���� �ڵ���Ƽ �ο��� 1������ ���ƾ� ������ ���� ������ �Ǵ�!
		if( iNumMember > 1 )
		{
			// �ڵ���Ƽ�� ���������� �����ߴٸ� '������ ����' ������ �ɾ�����!
			//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
			if( spRoomUser->GetOldPartyMemberCount() == 3 ) // �ڵ���Ƽ ��û�� ��Ƽ���� 3���̾��ٸ�
			{
				std::map< UidType, std::vector< KBuffInfo > >::iterator mit = mapActivateBuffList.find( spRoomUser->GetCID() );
				if( mit == mapActivateBuffList.end() )
				{
					std::vector< KBuffInfo > ActivateBuffList;

					KBuffInfo kBuff;
					kBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = CXSLBuffManager::BTI_BUFF_RALLY_OF_HERO_LEVEL_1;
					ActivateBuffList.push_back( kBuff );

					// ���� �ڵ���Ƽ �ο��� 1������ ���ƾ� �������� ���� �޽� ������ �Ǵ�!
					KBuffInfo kDebuff;
					kDebuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = CXSLBuffManager::BTI_DEBUFF_REST_OF_RELLY;
					ActivateBuffList.push_back( kDebuff );

					mapActivateBuffList.insert( std::make_pair( spRoomUser->GetCID(), ActivateBuffList ) );
				}
				else
				{
					KBuffInfo kBuff;
					kBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = CXSLBuffManager::BTI_BUFF_RALLY_OF_HERO_LEVEL_1;
					mit->second.push_back( kBuff );

					// ���� �ڵ���Ƽ �ο��� 1������ ���ƾ� �������� ���� �޽� ������ �Ǵ�!
					KBuffInfo kDebuff;
					kDebuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = CXSLBuffManager::BTI_DEBUFF_REST_OF_RELLY;
					mit->second.push_back( kDebuff );
				}

				bMaxPartyFever = true;
			}
			else if( spRoomUser->GetOldPartyMemberCount() == 2 )	// �ڵ���Ƽ ��û�� ��Ƽ���� 2���̾��ٸ�
			{
				std::map< UidType, std::vector< KBuffInfo > >::iterator mit = mapActivateBuffList.find( spRoomUser->GetCID() );
				if( mit == mapActivateBuffList.end() )
				{
					std::vector< KBuffInfo > ActivateBuffList;

					KBuffInfo kBuff;
					kBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = CXSLBuffManager::BTI_BUFF_RALLY_OF_HERO_LEVEL_2;
					ActivateBuffList.push_back( kBuff );

					// ���� �ڵ���Ƽ �ο��� 1������ ���ƾ� �������� ���� �޽� ������ �Ǵ�!
					KBuffInfo kDebuff;
					kDebuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = CXSLBuffManager::BTI_DEBUFF_REST_OF_RELLY;
					ActivateBuffList.push_back( kDebuff );

					mapActivateBuffList.insert( std::make_pair( spRoomUser->GetCID(), ActivateBuffList ) );
				}
				else
				{
					KBuffInfo kBuff;
					kBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = CXSLBuffManager::BTI_BUFF_RALLY_OF_HERO_LEVEL_2;
					mit->second.push_back( kBuff );

					// ���� �ڵ���Ƽ �ο��� 1������ ���ƾ� �������� ���� �޽� ������ �Ǵ�!
					KBuffInfo kDebuff;
					kDebuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = CXSLBuffManager::BTI_DEBUFF_REST_OF_RELLY;
					mit->second.push_back( kDebuff );
				}
				
				bMaxPartyFever = true;
			}
			else if( spRoomUser->GetOldPartyMemberCount() == 0 )	// �ڵ���Ƽ ��û�� ��Ƽ�� �ƴϾ��ٸ�
			{
				std::map< UidType, std::vector< KBuffInfo > >::iterator mit = mapActivateBuffList.find( spRoomUser->GetCID() );
				if( mit == mapActivateBuffList.end() )
				{
					std::vector< KBuffInfo > ActivateBuffList;

					KBuffInfo kBuff;
					kBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = CXSLBuffManager::BTI_BUFF_RALLY_OF_HERO_LEVEL_3;
					ActivateBuffList.push_back( kBuff );

					// ���� �ڵ���Ƽ �ο��� 1������ ���ƾ� �������� ���� �޽� ������ �Ǵ�!
					KBuffInfo kDebuff;
					kDebuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = CXSLBuffManager::BTI_DEBUFF_REST_OF_RELLY;
					ActivateBuffList.push_back( kDebuff );

					mapActivateBuffList.insert( std::make_pair( spRoomUser->GetCID(), ActivateBuffList ) );
				}
				else
				{
					KBuffInfo kBuff;
					kBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = CXSLBuffManager::BTI_BUFF_RALLY_OF_HERO_LEVEL_3;
					mit->second.push_back( kBuff );

					// ���� �ڵ���Ƽ �ο��� 1������ ���ƾ� �������� ���� �޽� ������ �Ǵ�!
					KBuffInfo kDebuff;
					kDebuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = CXSLBuffManager::BTI_DEBUFF_REST_OF_RELLY;
					mit->second.push_back( kDebuff );
				}

				bMaxPartyFever = true;
			}
#endif SERV_SERVER_BUFF_SYSTEM
			//}
		}
	}
	
	return bMaxPartyFever;
}
#endif SERV_BATTLE_FIELD_SYSTEM
//}}

//{{ 2012. 12. 17	������	�ƶ� ��Ƽ �÷��� ���ʽ� ����ġ
#ifdef SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
bool KDungeonRoom::PlayWithCharBuff( IN OUT std::map< UidType, std::vector< KBuffInfo > >& mapActivateBuffList )
{
	// �ϴ� ���� �Ƹ��� �ִ��� Ȯ���Ѵ�!
	if( m_spRoomUserManager->IsExistCharType( CXSLUnit::UT_ARA ) == false )
		return false;

	// �Ƹ��� ��Ƽ�� ������ ��Ƽ�� ��ü���� ĳ���Ϳ� �Բ� �÷��� ���ʽ� ����ġ ������ �����Ų��.	
	const int iNumMember = m_spRoomUserManager->GetNumMember();
	for( int iIndex = 0; iIndex < iNumMember; ++iIndex )
	{
		KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser( iIndex );
		if( IS_NULL( spRoomUser ) )
		{
			START_LOG( cerr, L"�������� �ʴ� �����Դϴ�!" )
				<< BUILD_LOG( iIndex )
				<< END_LOG;
			continue;
		}
		
		// ���� ������ ��´�!
		std::map< UidType, std::vector< KBuffInfo > >::iterator mit = mapActivateBuffList.find( spRoomUser->GetCID() );
		if( mit == mapActivateBuffList.end() )
		{
			// �ش� ������ ���� ������ ����
			std::vector< KBuffInfo > ActivateBuffList;

			KBuffInfo kBuff;
			kBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = CXSLBuffManager::BTI_SECRET_OF_THOUSAND_YEARS_FOX;
			ActivateBuffList.push_back( kBuff );

			mapActivateBuffList.insert( std::make_pair( spRoomUser->GetCID(), ActivateBuffList ) );
		}
		else
		{
			KBuffInfo kBuff;
			kBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = CXSLBuffManager::BTI_SECRET_OF_THOUSAND_YEARS_FOX;
			mit->second.push_back( kBuff );
		}
	}

	return true;
}
#endif SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
//}}


//{{ �ʵ� ��� ���� - ��μ�
#ifdef SERV_REFORM_ITEM_DROP
float KDungeonRoom::GetDungeonPartyBonusRate()
{ 
	int iPartyCount = m_spRoomUserManager->GetLiveMember();

	return m_spRoomUserManager->GetPartyBonusRate( iPartyCount ); 
}
#endif SERV_REFORM_ITEM_DROP
//}}

//{{ 2012. 09. 02	�ڼ���	Merge ( �������� Ŭ���̾�Ʈ ���۵����� UDP ��Ŷ�� ���ƿ��� �ʴ� ������ �߰ߵǸ� �������� ű�Ѵ�. // 2012.06.11 lygan_������ )
#ifdef UDP_CAN_NOT_SEND_USER_KICK
_IMPL_ON_FUNC( ERM_UDP_CHECK_KICK_USER_NOT, KEGS_UDP_CHECK_KICK_USER_NOT )
{
	// PVP ������ ����ϰ� �����Ƿ� ���������� �ƹ� ��ɵ� ���� �ʴ´�.
	// ���Ŀ� ���������� ��� �߰��� ���⿡ �ϸ�ȴ�.
}
#endif UDP_CAN_NOT_SEND_USER_KICK
//}}

//{{ 2012. 11. 9	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
IMPL_ON_FUNC_NOPARAM( ERM_UPDATE_HENIR_REWARD_USER_NOT )
{
	VERIFY_STATE( ( 3, KRoomFSM::S_WAIT, KRoomFSM::S_LOAD, KRoomFSM::S_PLAY ) );

	KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser( FIRST_SENDER_UID );
	if( IS_NULL( spRoomUser ) )
	{
		START_LOG( cerr, L"�������� �ʴ� �������� ��ϸ� ���� ���� ������ ������Ʈ �Ϸ��� �߽��ϴ�!" )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< END_LOG;
		return;
	}

	// ��ϸ� ���� ���� ����!
	spRoomUser->SetHenirRewardUser( true );
}
#endif SERV_BATTLE_FIELD_SYSTEM
//}}

//{{ 2013. 01. 09 ���� ���� �ý��� - ��μ�
#ifdef SERV_DUNGEON_FORCED_EXIT_SYSTEM
_IMPL_ON_FUNC( ERM_BAD_ATTITUDE_USER_CHECK_INFO_NOT, KEGS_BAD_ATTITUDE_USER_CHECK_INFO_NOT )
{
	VERIFY_STATE( ( 1, KRoomFSM::S_PLAY ) );

	// �ش� ���� �������� or wave �� ��ũ�� ���Ѵ�.
	// �ش� ���� �������� ���� ���� ������

	// �����̸� �˻����� ����
	int iPartyMemberCount = m_spRoomUserManager->GetNumMember();
	if( iPartyMemberCount <= 1 )
		return;

	// �ڵ� ��Ƽ�� �˻��Ѵ�.
	if( m_bStartedByAutoParty == false )
		return;

	m_kBadAttitudeManager.SetUnitSubStageInfo(  FIRST_SENDER_UID, 
												kPacket_.m_iStageID, 
												kPacket_.m_iSubStageID, 
												GetDungeonIDAndDif(), 
												m_spRoomUserManager->GetLiveMember(),
												kPacket_.m_iComboScore,
												kPacket_.m_iTechScore );
	
	// ���� ��������, ���� �������� ��� ������ ��ũ�� �� ���ߴ°�? 
	if( m_kBadAttitudeManager.IsAllUnitGetScore() == true )
	{
		std::vector<UidType> vecNewBadAttitudeUser;
		std::vector<UidType> vecNewForceExitUser;

		// �ҷ� ������ ã��
		// ���� ������ ã��
		m_kBadAttitudeManager.CheckBadAttitudeUnit( vecNewBadAttitudeUser, vecNewForceExitUser );

		// �ҷ� ������ ã�Ҵ�
		if( vecNewBadAttitudeUser.empty() == false )
		{
			BOOST_TEST_FOREACH( UidType, iBadAttitudeUserUID, vecNewBadAttitudeUser )
			{
				// �ҷ� �������� �ڽ��� �ҷ� ������ �Ǿ����� �����϶�� �޽����� ����ϰ� �Ѵ�.
				KRoomUserPtr pUser = m_spRoomUserManager->GetUser( iBadAttitudeUserUID );
				if( pUser != NULL )
				{
					KEGS_BAD_ATTITUDE_USER_MSG_NOT kPacket;

					kPacket.m_MsgType = KEGS_BAD_ATTITUDE_USER_MSG_NOT::MT_BAD_ATTITUDE_USER_WARNING;
					kPacket.m_iUserUID = iBadAttitudeUserUID;

					SendToGSCharacter( pUser->GetGSUID(), pUser->GetCID(), ERM_BAD_ATTITUDE_USER_MSG_NOT, kPacket );
				}

				// �ҷ� ������ ã������ ��ǥ �����ϰ� �˷��ش�.
				KEGS_BAD_ATTITUDE_USER_NOT kNot;
				kNot.m_iUserUID = iBadAttitudeUserUID;
				BroadCast( ERM_BAD_ATTITUDE_USER_NOT, kNot );
			}
		}

		// ���� ������ ã�Ҵ�
		if( vecNewForceExitUser.empty() == false )
		{
			BOOST_TEST_FOREACH( UidType, iForceExitUserUID, vecNewForceExitUser )
			{
				KRoomUserPtr pUser = m_spRoomUserManager->GetUser( iForceExitUserUID );
				if( pUser != NULL )
				{
					KERM_LEAVE_ROOM_REQ kPacketReq;
					kPacketReq.m_iReason = NetError::NOT_LEAVE_ROOM_REASON_34;

					UidType iLAST_SENDER_UID  = pUser->GetGSUID();
					UidType iFIRST_SENDER_UID = pUser->GetCID();

					// ���� ��Ŵ
					OnLeaveRoom( pUser->GetGSUID(), pUser->GetCID(), kPacketReq, ERM_LEAVE_ROOM_ACK );

					// �̷� �̷� ������ �ʴ� �ð����� �˷��ش�.
					KEGS_BAD_ATTITUDE_USER_MSG_NOT kKick;
					kKick.m_MsgType = KEGS_BAD_ATTITUDE_USER_MSG_NOT::MT_FORCED_EXIT;
					kKick.m_iUserUID = iForceExitUserUID;
					SendToGSCharacter( iLAST_SENDER_UID, iFIRST_SENDER_UID, ERM_BAD_ATTITUDE_USER_MSG_NOT, kKick );

					// �̷� �̷� ������ �� ������ �������� �˷��ش�.
					KEGS_BAD_ATTITUDE_USER_MSG_NOT kNot;
					kNot.m_MsgType = KEGS_BAD_ATTITUDE_USER_MSG_NOT::MT_BAD_ATTITUDE_USER_EXIT;
					kNot.m_iUserUID = iForceExitUserUID;
					BroadCast( ERM_BAD_ATTITUDE_USER_MSG_NOT, kNot );
				}
			}
		}
	}
};

_IMPL_ON_FUNC( ERM_DEFENCE_DUNGEON_WAVE_ID_NOT, KEGS_DEFENCE_DUNGEON_WAVE_ID_NOT )
{
	m_kBadAttitudeManager.SetDefenceDungeonWaveID( kPacket_.m_iWaveID );
}

_IMPL_ON_FUNC( ERM_FORCED_EXIT_VOTE_REQ, KEGS_FORCED_EXIT_VOTE_REQ )
{
	KEGS_FORCED_EXIT_VOTE_ACK kPacket;

	// ���� ��ǥ�� �Ѵ�!
	if( kPacket_.m_bWantForceExit == false )
	{
		START_LOG( cerr, L"��ǥ �Ѵٸ鼭 �� ���� false?" )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< BUILD_LOG( kPacket_.m_iUserUID )
			<< BUILD_LOG( kPacket_.m_bWantForceExit )
			<< END_LOG;
	}

	// ��ǥ�ϴ� ����� ã��
	KRoomUserPtr pVoteUser = m_spRoomUserManager->GetUser( FIRST_SENDER_UID );
	if( pVoteUser == NULL )
	{
		START_LOG( cerr, L"�������� �ʴ� ������ ��ǥ�Ϸ� �Ѵ�." )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< BUILD_LOG( kPacket_.m_iUserUID )
			<< END_LOG;
	
		kPacket.m_iOK = NetError::ERR_BAD_ATTITUDE_00;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_FORCED_EXIT_VOTE_ACK, kPacket );
		return;
	}

	// ��ǥ ���� ����� ã��
	KRoomUserPtr pBadUser = m_spRoomUserManager->GetUser( kPacket_.m_iUserUID );
	if( pBadUser == NULL )
	{
		START_LOG( cerr, L"�������� �ʴ� ������ ��ǥ�Ϸ� �Ѵ�." )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< BUILD_LOG( kPacket_.m_iUserUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_BAD_ATTITUDE_01;
		SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_FORCED_EXIT_VOTE_ACK, kPacket );
		return;
	}

	// ��ǥ���� ���� ��Ű��
	m_kBadAttitudeManager.IncreaseVoteOnePoint( kPacket_.m_iUserUID, FIRST_SENDER_UID );

	kPacket.m_iUserUID = kPacket_.m_iUserUID;
	kPacket.m_iOK = NetError::NET_OK;
	SendToGSCharacter( LAST_SENDER_UID, FIRST_SENDER_UID, ERM_FORCED_EXIT_VOTE_ACK, kPacket );
}
#endif SERV_DUNGEON_FORCED_EXIT_SYSTEM
//}

//{{ 2013. 02. 01  �̺�Ʈ �߷�Ÿ�� ���� - ��μ�
#ifdef SERV_EVENT_VALENTINE_DUNGEON
IMPL_ON_FUNC_NOPARAM( ERM_SYNC_DUNGEON_TIMER_NOT )
{
	if( IsHost( FIRST_SENDER_UID ) == false )
	{
		return;
	}

	// �߷�Ÿ�� ������ �ش��
	if( CXSLDungeon::IsValentineDungeon( GetDungeonIDAndDif() ) == false )
	{
		return;
	}

	m_kValentineManager.SetStartTimer( KValentineManager::TEM_AFTER_DUNGEON_START, true );

	KEGS_SYNC_DUNGEON_TIMER_NOT kNot;
	kNot.m_iRemainTime = KValentineManager::LDT_VALENTINE - m_kValentineManager.GetRemainTime( KValentineManager::TEM_AFTER_DUNGEON_START );
	BroadCast( ERM_SYNC_DUNGEON_TIMER_NOT, kNot );
}
#endif SERV_EVENT_VALENTINE_DUNGEON
//}

//{{ 2013. 04. 22	������	����� �� ���� ���� ����
//////////////////////////////////////////////////////////////////////////
#ifdef SERV_NEW_DEFENCE_DUNGEON_ENTER_LOGIC
//////////////////////////////////////////////////////////////////////////
IMPL_ON_FUNC( ERM_PREPARE_FOR_DEFENCE_DUNGEON_ACK )
{
	VERIFY_STATE( ( 1, KRoomFSM::S_WAIT_FOR_DEFENCE ) );

	// ����� �� ������ �����ϱ� ���� RoomUserInfo���� ���� üũ
	if( m_spRoomUserManager->SetPrepareForDefenceDungeon( FIRST_SENDER_UID, true ) == false )
	{
		START_LOG( cerr, L"�������� �ʴ� ���� �Դϴ�!" )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< END_LOG;
		return;
	}

	// ���� ���� ����!
	m_spRoomUserManager->SetRoomUserInfo( FIRST_SENDER_UID, kPacket_.m_kRoomUserInfo );
}

_IMPL_ON_FUNC( ERM_ENTER_DEFENCE_DUNGEON_POPUP_REPLY_NOT, KEGS_ENTER_DEFENCE_DUNGEON_POPUP_REPLY_NOT )
{
	VERIFY_STATE( ( 1, KRoomFSM::S_WAIT_FOR_DEFENCE ) );

	// ����� �� ���� �˾�
	if( m_spRoomUserManager->SetEnterDefenceDungeon( FIRST_SENDER_UID, kPacket_.m_bAgree ) == false )
	{
		START_LOG( cerr, L"�������� �ʴ� ���� �Դϴ�!" )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< BUILD_LOG( kPacket_.m_bAgree )
			<< END_LOG;
		return;
	}

	START_LOG( clog, L"����� �� ���忡 ���� ������ �޾ҽ��ϴ�." )
		<< BUILD_LOG( FIRST_SENDER_UID )
		<< BUILD_LOG( kPacket_.m_bAgree );

	// ��� ������ ������ ���ŵǾ����� Ȯ���Ѵ�!
	std::vector< UidType > vecLeaveUnitUID;
	if( m_spRoomUserManager->IsAllPlayerPrepareForDefenceDungeon( vecLeaveUnitUID ) == true )
	{
		// ����� �� ���� ����!
		EnterDefenceDungeon( vecLeaveUnitUID );
	}
}
//////////////////////////////////////////////////////////////////////////
#else
//////////////////////////////////////////////////////////////////////////
//{{ 2013. 04. 15	������	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
IMPL_ON_FUNC( ERM_PREPARE_FOR_DEFENCE_DUNGEON_ACK )
{
	VERIFY_STATE( ( 1, KRoomFSM::S_WAIT_FOR_DEFENCE ) );

	if( m_spRoomUserManager->SetPrepareForDefenceDungeon( FIRST_SENDER_UID, true ) == false )
	{
		START_LOG( cerr, L"�������� �ʴ� ���� �Դϴ�!" )
			<< BUILD_LOG( FIRST_SENDER_UID )
			<< END_LOG;
		return;
	}

	// ���� ���� ����!
	m_spRoomUserManager->SetRoomUserInfo( FIRST_SENDER_UID, kPacket_.m_kRoomUserInfo );

	// ��� ������ ������ ���ŵǾ����� Ȯ���Ѵ�!
	if( m_spRoomUserManager->IsAllPlayerPrepareForDefenceDungeon() == true )
	{
		// ����� �� ���� ����!
		EnterDefenceDungeon();
	}
}
#endif SERV_NEW_DEFENCE_DUNGEON
//}}
//////////////////////////////////////////////////////////////////////////
#endif SERV_NEW_DEFENCE_DUNGEON_ENTER_LOGIC
//////////////////////////////////////////////////////////////////////////
//}}

#ifdef SERV_CRAYON_POP_EVENT_BUFF// �۾���¥: 2013-06-10	// �ڼ���
void KDungeonRoom::CrayonPopEventBuff( IN OUT std::map< UidType, std::vector< KBuffInfo > >& mapActivateBuffList ) const
{
	int			iBuffTempletID;
	KLottery	kLot;
	kLot.AddCase( CXSLBuffManager::BTI_BUFF_ELRIN, 45.0f );
	kLot.AddCase( CXSLBuffManager::BTI_BUFF_SOYUL, 30.0f );
	kLot.AddCase( CXSLBuffManager::BTI_BUFF_KUMI, 5.0f );
	kLot.AddCase( CXSLBuffManager::BTI_BUFF_CHOA, 10.0f );
	kLot.AddCase( CXSLBuffManager::BTI_BUFF_WEY, 10.0f );
	iBuffTempletID = kLot.Decision();

	if( iBuffTempletID == KLottery::CASE_BLANK )
	{
		iBuffTempletID = CXSLBuffManager::BTI_BUFF_SOYUL;
	}

	const int iNumMember = m_spRoomUserManager->GetNumMember();

	for( int iIndex = 0; iIndex < iNumMember; ++iIndex )
	{
		KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser( iIndex );
		if( spRoomUser == NULL )
		{
			START_LOG( cerr, L"�������� �ʴ� �����Դϴ�!" )
				<< BUILD_LOG( iIndex )
				<< END_LOG;
			continue;
		}

		std::map< UidType, std::vector< KBuffInfo > >::iterator mit = mapActivateBuffList.find( spRoomUser->GetCID() );
		if( mit == mapActivateBuffList.end() )
		{
			// �ش� ������ ���� ������ ����
			std::vector< KBuffInfo > ActivateBuffList;

			KBuffInfo kBuff;
			kBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = iBuffTempletID;
			ActivateBuffList.push_back( kBuff );

			mapActivateBuffList.insert( std::make_pair( spRoomUser->GetCID(), ActivateBuffList ) );
		}
		else
		{
			KBuffInfo kBuff;
			kBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = iBuffTempletID;
			mit->second.push_back( kBuff );
		}
	}
}
#endif // SERV_CRAYON_POP_EVENT_BUFF

#ifdef  SERV_OPTIMIZE_DETECT_ZOMBIE_HOST
IMPL_ON_FUNC_NOPARAM( ERM_BATTLE_FIELD_ZOMBIE_ALERT_NOT )
{
}
#endif  SERV_OPTIMIZE_DETECT_ZOMBIE_HOST


#ifdef SERV_BONUS_BUFF_SYSTEM
bool KDungeonRoom::PlayWithBonusBuff( IN OUT std::map< UidType, std::vector< KBuffInfo > >& mapActivateBuffList )
{
	// ���⼭ ���� äũ �ϰ�, ���� �ɾ�����

	const int iNumMember = m_spRoomUserManager->GetNumMember();
	for( int iIndex = 0; iIndex < iNumMember; ++iIndex )
	{
		KRoomUserPtr spRoomUser = m_spRoomUserManager->GetUser( iIndex );
		if( IS_NULL( spRoomUser ) )
		{
			START_LOG( cerr, L"�������� �ʴ� �����Դϴ�!" )
				<< BUILD_LOG( iIndex )
				<< END_LOG;
			continue;
		}

		// �̺�Ʈ �Ǽ��縮 ���������� Ȯ���Ѵ�
		KRoomUserInfo kRoomUserInfo;
		spRoomUser->GetRoomUserInfo(kRoomUserInfo);
		std::map< int, KInventoryItemInfo >::const_iterator mit = kRoomUserInfo.m_mapEquippedItem.find(CXSLUnit::NESI_AC_FACE1);
		
		if( kRoomUserInfo.m_mapEquippedItem.end() != mit 
			&& BUFF_CONDITION_ITEM_ID_BEGIN <= mit->second.m_kItemInfo.m_iItemID && BUFF_CONDITION_ITEM_ID_END >= mit->second.m_kItemInfo.m_iItemID )
		{
			// �̺�Ʈ �Ǽ��縮 ���� ���̹Ƿ� ������ �����Ų��.
			// �켱 ��� ������ �����ų�� ���Ѵ�.
			int iBuffID = 0;
			KLottery	kLot;
			kLot.AddCase( CXSLBuffManager::BTI_BUFF_2YEARS_EVENT_CN_DAMAGE_UP, 20.0f );
			kLot.AddCase( CXSLBuffManager::BTI_BUFF_2YEARS_EVENT_CN_MOVE_JUMP_SPEED_UP, 20.0f );
			kLot.AddCase( CXSLBuffManager::BTI_BUFF_2YEARS_EVENT_CN_ACCURACY_AVOID_UP, 20.0f );
			kLot.AddCase( CXSLBuffManager::BTI_BUFF_2YEARS_EVENT_CN_CRI_SPEED_UP, 20.0f );
			kLot.AddCase( CXSLBuffManager::BTI_BUFF_2YEARS_EVENT_CN_DAMAGE_INC_DEC_UP, 18.0f );
			kLot.AddCase( CXSLBuffManager::BTI_BUFF_2YEARS_EVENT_CN_ALL_IN_ONE, 2.0f );
			iBuffID = kLot.Decision();

			if( iBuffID == KLottery::CASE_BLANK )
			{
				iBuffID = CXSLBuffManager::BTI_BUFF_2YEARS_EVENT_CN_ALL_IN_ONE;
			}

			// ���� ������ ��´�!
			std::map< UidType, std::vector< KBuffInfo > >::iterator mit = mapActivateBuffList.find( spRoomUser->GetCID() );
			if( mit == mapActivateBuffList.end() )
			{
				// �ش� ������ ���� ������ ����
				std::vector< KBuffInfo > ActivateBuffList;

				KBuffInfo kBuff;
				kBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = iBuffID;
				ActivateBuffList.push_back( kBuff );

				mapActivateBuffList.insert( std::make_pair( spRoomUser->GetCID(), ActivateBuffList ) );
			}
			else
			{
				KBuffInfo kBuff;
				kBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID = iBuffID;
				mit->second.push_back( kBuff );
			}
		}
	}

	return true;
}
#endif SERV_BONUS_BUFF_SYSTEM