#include "StdAfx.h"
#include "./X2BattleFieldGame.h"

CX2BattleFieldGame::CX2BattleFieldGame()
: m_pBattleFieldRoom( NULL )
, m_pDlgBattleFieldName( NULL )
, m_TimerForSendingPositionInfo( 10.0f )
, m_iMAX_NUM_OF_NPC( 12 )
, m_uiWorldId( 0 )
, m_bIsNpcLoadComplete(false)
#ifdef QUEST_GUIDE
, m_pNPCIndicator(NULL)
#endif
#ifdef SERV_BATTLEFIELD_MIDDLE_BOSS
, m_pMonsterIndicator( NULL )			// 필드 중보 Indicator, 차후 확장 가능
#endif // SERV_BATTLEFIELD_MIDDLE_BOSS

#ifdef SERV_CHECK_EXIST_MONSTER_UID
, m_ElapsedTimeSinceGettingFirstNpcSyncPacket( 10.0f )
, m_bCanCheckGettingFirstNpcSyncPacket( false )
#endif // SERV_CHECK_EXIST_MONSTER_UID
{
	m_GameType = CX2Game::GT_BATTLE_FIELD;

	m_pDLGContinue	= new CKTDGUIDialog( this, L"DLG_Dungeon_Game_Continue_New.lua" );
	m_pDLGContinue->SetShow( false );
	m_pDLGContinue->SetEnable( false );
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGContinue );
	m_pSoundContinue			= g_pKTDXApp->GetDeviceManager()->OpenSound( L"Continue.ogg" );

#ifdef QUEST_GUIDE
	m_pNPCIndicator = new CX2NPCIndicator();
#endif //QUEST_GUIDE

#ifdef SERV_BATTLEFIELD_MIDDLE_BOSS
	m_hBossDanger = INVALID_PARTICLE_HANDLE;			// 필드 중보 등장 위험 이펙트 핸들
	m_hBossDangerAdd = INVALID_PARTICLE_HANDLE;			// 필드 중보 등장 위험 이펙트 핸들
#endif // SERV_BATTLEFIELD_MIDDLE_BOSS

	//#ifdef SERV_COMMON_AUTO_OPEN_HARD_CODE
	m_bOpenPotalParticle = true;
	//#endif //SERV_COMMON_AUTO_OPEN_HARD_CODE
}

/*virtual*/ CX2BattleFieldGame::~CX2BattleFieldGame()
{
	/*
	Release() 에서 수행
	*/
}

/** @function : Release
	@brief : X2Game의 소멸자에 실행할 것들을 이곳에서 실행함
	(이렇게 따로 만들어 준 이유는 X2Game 소멸자가 X2GameUnit 등을 소멸할 때
	X2GameUnit 등의 소멸자 내부 또는 호출되는 함수 들에 내부에서 다시 X2Game을 참조하는 경우
	가 많아서 다 찾아내기 힘들기에 이렇게 Release로 만들었습니다.
	X2Game이 소멸되는 경우는 현재 Pvp, Dungeon, BattleField의 스테이트가 소멸될 때 이므로
	해당 스테이트의 소멸자에서 Release를 호출 하도록 하였습니다.
	더 좋은 방법이 있으신 분은 알려주세요 - 2012-11-11 (김현철)
*/
/*virtual*/ void CX2BattleFieldGame::Release()
{
	SAFE_DELETE_DIALOG( m_pDlgBattleFieldName );
	SAFE_DELETE_DIALOG( m_pDLGContinue );
	SAFE_CLOSE( m_pSoundContinue );

#ifdef QUEST_GUIDE
	SAFE_DELETE(m_pNPCIndicator);
#endif

//#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
//	g_pData->GetUnitManager()->UnloadAllNPCInitData();
//#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD

#ifdef SERV_BATTLEFIELD_MIDDLE_BOSS
	// 연출 Handle 제거
	if( INVALID_PARTICLE_HANDLE != m_hBossDanger )
		g_pX2Game->GetMinorParticle()->DestroyInstanceHandle( m_hBossDanger );
	if( INVALID_PARTICLE_HANDLE != m_hBossDangerAdd )
		g_pX2Game->GetMinorParticle()->DestroyInstanceHandle( m_hBossDangerAdd );
	SAFE_DELETE( m_pMonsterIndicator );
#endif // SERV_BATTLEFIELD_MIDDLE_BOSS

	CX2Game::Release();	/// 상위 클래스의 소멸자가 나중에 호출
}

/*virtual*/ HRESULT CX2BattleFieldGame::OnFrameMove( double fTime, float fElapsedTime )
{
	CX2Game::OnFrameMove(fTime, fElapsedTime );

#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
	std::deque<CNpcCreatedInfoPtr>& conNpcCreatedInfo = m_deqNpcCreatedInfo;
#else	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
	std::vector<CNpcCreatedInfoPtr>& conNpcCreatedInfo = m_vecNpcCreatedInfo;
#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD

	if ( false == conNpcCreatedInfo.empty() )
 	{
#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
		if ( 
#ifdef	X2OPTIMIZE_INFORM_FIRST_FRAME_OF_SIMULATION_LOOP
			g_pKTDXApp->IsFirstFrameOfSimulationLoop() == true &&
#endif	X2OPTIMIZE_INFORM_FIRST_FRAME_OF_SIMULATION_LOOP
			m_UnitLoader.IsNPCUnitUIDResourceLoadCompleted( conNpcCreatedInfo.front()->GetNpcUid() ) == true )
#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
		{
#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
			CreateBattleFieldNpcByCreatedInfo( conNpcCreatedInfo.front() );
			conNpcCreatedInfo.pop_front();
#ifndef	X2OPTIMIZE_INFORM_FIRST_FRAME_OF_SIMULATION_LOOP
			g_pKTDXApp->SkipFrame();
#endif	X2OPTIMIZE_INFORM_FIRST_FRAME_OF_SIMULATION_LOOP
#else	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
			CreateBattleFieldNpcByCreatedInfo( conNpcCreatedInfo.back() );
			conNpcCreatedInfo.pop_back();
#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD

			if ( conNpcCreatedInfo.empty() )
			{
#ifndef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
				ClearKAttribEnchantNpcInfoMap();
#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
				g_pData->GetServerProtocol()->SendID( EGS_BATTLE_FIELD_NPC_LOAD_COMPLETE_REQ );
				g_pMain->AddServerPacket( EGS_BATTLE_FIELD_NPC_LOAD_COMPLETE_ACK );
			}
		}
 	}

	if( m_GameState == GS_PLAY )
	{
		if( false == GetGameEnd() )
		{
			ResurrectionFrameMove( fTime, fElapsedTime );

			// 10초 마다 한번씩 패킷 전송
			m_TimerForSendingPositionInfo.OnFrameMove();
			if ( m_TimerForSendingPositionInfo.CheckAndResetElapsedTime() )
			{
				m_kGameStatistics.GatherPeriodicStatistics();	/// 프레임 수집
				Send_EGS_UPDATE_BATTLE_FIELD_USER_POS_NOT();

#ifdef ACTIVE_KOG_GAME_PERFORMANCE_CHECK
				CX2Game::StartTcpPingCheck();
#endif//ACTIVE_KOG_GAME_PERFORMANCE_CHECK
			}
		}

#ifdef SERV_CHECK_EXIST_MONSTER_UID
		if ( GetCanCheckGettingFirstNpcSyncPacket() )	/// 첫 싱크 패킷을 받았는지를 체크해야 하는지 여부
		{
			m_ElapsedTimeSinceGettingFirstNpcSyncPacket.OnFrameMove();	/// 체크 타이밍을 계산하기 위한 ElapsedTime의 OnFrameMove
			if ( m_ElapsedTimeSinceGettingFirstNpcSyncPacket.CheckElapsedTime() )	/// 체크 할 타이밍 인지
			{
				// 체크 후 Send!
				CheckNpcNotReceiveFirstSyncPacketAndRequestToCheckNpcUidList();
				/// 이제 다음에 다른 필드에 난입 하는 등으로 인해서 
				/// 필드 NPC의 첫 싱크 패킷을 받기 전까지 체크 할 필요 없음
				SetCanCheckGettingFirstNpcSyncPacket( false );	
			}
		}
#endif // SERV_CHECK_EXIST_MONSTER_UID
	}

#ifdef QUEST_GUIDE
	if( NULL != m_pNPCIndicator )
	{
		m_pNPCIndicator->OnFrameMove();
	}
#endif



#ifdef SERV_BATTLEFIELD_MIDDLE_BOSS
	// 필드 중보를 위해 Indicator 추가
	if( NULL != m_pMonsterIndicator )
	{
		if (null != m_pMonsterIndicator->GetTargetNPC() )
			m_pMonsterIndicator->OnFrameMove();
		else
			SAFE_DELETE ( m_pMonsterIndicator );
	}

#endif // SERV_BATTLEFIELD_MIDDLE_BOSS

	return S_OK;
}

/*virtual*/ HRESULT CX2BattleFieldGame::OnFrameRender()
{
	CX2Game::OnFrameRender();

	return S_OK;
}

/*virtual*/ void CX2BattleFieldGame::GameLoading( CX2Room* pRoom )
{
	m_pBattleFieldRoom		= static_cast<CX2BattleFieldRoom*>( pRoom );
	CX2Game::GameLoading( pRoom );
	m_GameState				= GS_LOADING;
	
	if ( NULL != GetMyUnit() )
	{
		GetMyUnit()->SetForceInvincible( 3.0f );
		GetMyUnit()->SetShowInvincible( 3.0f );
	}

#ifdef REFORM_UI_MINIMAP
	if( NULL == g_pInstanceData->GetMiniMapUI()->GetDungeonMiniMap() )
	{
		g_pInstanceData->GetMiniMapUI()->CreateDungeonMiniMap();
	}

	if( NULL != g_pInstanceData->GetMiniMapUI() )
	{
		g_pInstanceData->GetMiniMapUI()->SetShowMiniMap( CX2MiniMapUI::MMT_DUNGEON, true );
		g_pInstanceData->GetMiniMapUI()->GetDungeonMiniMap()->SetEyeDistance( 2000.f );
	}
#endif

#ifdef	X2OPTIMIZE_MASS_FILE_BUFFER_MANAGER
	if ( g_pKTDXApp->GetDeviceManager() != NULL )
		g_pKTDXApp->GetDeviceManager()->ReleaseAllMemoryBuffers();
#endif	X2OPTIMIZE_MASS_FILE_BUFFER_MANAGER

#ifdef SERV_BATTLEFIELD_MIDDLE_BOSS
	// 필드 중보를 위해 Indicator 추가
	SAFE_DELETE( m_pMonsterIndicator );
	m_pMonsterIndicator = NULL;
#endif // SERV_BATTLEFIELD_MIDDLE_BOSS


}

/*virtual*/ void CX2BattleFieldGame::WorldLoading()
{
	SAFE_DELETE( m_pWorld );

	const UINT uiBattleFieldId = g_pData->GetBattleFieldManager().GetBattleFieldIdWhereIam();

	ASSERT( SEnum::VMI_BATTLE_FIELD_RUBEN_FIELD_01 <= uiBattleFieldId && SEnum::VMI_BATTLE_FIELD_END > uiBattleFieldId );

	const UINT uiWorldId = g_pData->GetBattleFieldManager().GetWorldIdByBattleFieldId( uiBattleFieldId );

	ASSERT( CX2World::WI_NONE < uiWorldId && CX2World::WI_END > uiWorldId );
	SetWorldId( uiWorldId );

	m_pWorld = g_pData->GetWorldManager()->CreateWorld( static_cast<CX2World::WORLD_ID>( uiWorldId ), NULL, true );

	CreatePotalParticle();
	OpenBattleFieldName();
}

// void CX2BattleFieldGame::CreateNpcFromDungeonIntrudeSyncPacket(	const KEGS_DUNGEON_INTRUDE_SYNC_NOT& kPacket_ )
// {
// 	for( int i = 0; i < (int)kPacket_.m_vecNPCData.size(); i++ )
// 	{
// 		const KNPCUnitReq& kNPCUnitReq = kPacket_.m_vecNPCData[i];
// 
// 		// 이 함수는 KNPCUnitReq 자체를 넘기도록 수정하는게 좋을듯
// 		CreateNPC( (CX2UnitManager::NPC_UNIT_ID)kNPCUnitReq.m_NPCID, 
// 			kNPCUnitReq.m_Level,
// 			kNPCUnitReq.m_bActive,
// 			kNPCUnitReq.m_UID, 
// 			D3DXVECTOR3( kNPCUnitReq.m_vPos.x, kNPCUnitReq.m_vPos.y, kNPCUnitReq.m_vPos.z ), 
// 			kNPCUnitReq.m_bIsRight,
// 			kNPCUnitReq.m_fDelayTime,
// 			(int) kNPCUnitReq.m_KeyCode, 
// 			(CX2Room::TEAM_NUM) kNPCUnitReq.m_cTeamNum,
// 			(CX2NPCAI::NPC_AI_TYPE) kNPCUnitReq.m_cAIType,
// 			kNPCUnitReq.m_iAllyUID );
// 	}
// }

void CX2BattleFieldGame::CreatePotalParticle() const
{
	if( NULL != m_pWorld &&
		NULL != m_pWorld->GetLineMap() )
	{
		int iNumOfLineData = m_pWorld->GetLineMap()->GetNumLineData();

		for( int i=0; i < iNumOfLineData; i++ )
		{
			CKTDGLineMap::LineData* pLineData = m_pWorld->GetLineMap()->GetLineData( i );
			if( NULL != pLineData &&
				CKTDGLineMap::LT_POTAL == pLineData->lineType )
			{

#ifdef SERV_SANDER_AUTO_OPEN_HARD_CODE
				IF_EVENT_ENABLED( CEI_SANDER_AUTO_OPEN_HARD_CODE )
				{
					// 2013.03.14 lygan_조성욱 // 코드 비어 있는게 맞습니다.
				}
				ELSE
				{
					if( m_pWorld->GetWorldData()->worldID == CX2World::WI_BATTLE_FIELD_HAMEL_FIELD_03 &&
						pLineData->lineIndex == 170 )
					{
						m_bOpenPotalParticle = false;
						continue;
					}
				}
#endif //SERV_SANDER_AUTO_OPEN_HARD_CODE

				CX2WorldObjectParticle* pParticle = m_pWorld->CreateObjectParticle( g_pX2Game->GetMajorParticle(), "Peita_Teleport_MagicSquare01" );
				if( NULL != pParticle )
				{
					pParticle->SetParticlePos( ( pLineData->startPos + pLineData->endPos ) * 0.5f + D3DXVECTOR3(0, 3, 0 ) );
					pParticle->SetLayer( XL_EFFECT_0 );
				}
			}
		}
	}
}


#ifdef SERV_COMMON_AUTO_OPEN_HARD_CODE
void CX2BattleFieldGame::CreatePotalParticle( const int& iLineIdx ) const
{
	if( NULL != m_pWorld &&
		NULL != m_pWorld->GetLineMap() )
	{
		int iNumOfLineData = m_pWorld->GetLineMap()->GetNumLineData();

		CKTDGLineMap::LineData* pLineData = m_pWorld->GetLineMap()->GetLineData( iLineIdx );
		if( NULL != pLineData &&
			CKTDGLineMap::LT_POTAL == pLineData->lineType )
		{
			CX2WorldObjectParticle* pParticle = m_pWorld->CreateObjectParticle( g_pX2Game->GetMajorParticle(), "Peita_Teleport_MagicSquare01" );
			if( NULL != pParticle )
			{
				pParticle->SetParticlePos( ( pLineData->startPos + pLineData->endPos ) * 0.5f + D3DXVECTOR3(0, 3, 0 ) );
				pParticle->SetLayer( XL_EFFECT_0 );
			}
		}
	}
}
#endif // SERV_COMMON_AUTO_OPEN_HARD_CODE


void CX2BattleFieldGame::SwapWorld()
{
	const UINT uiBattleFieldId = g_pData->GetBattleFieldManager().GetBattleFieldIdWhereIam();
	ASSERT( SEnum::VMI_BATTLE_FIELD_RUBEN_FIELD_01 <= uiBattleFieldId && SEnum::VMI_BATTLE_FIELD_END > uiBattleFieldId );

	const UINT uiWorldId = g_pData->GetBattleFieldManager().GetWorldIdByBattleFieldId( uiBattleFieldId );
	ASSERT( CX2World::WI_NONE < uiWorldId && CX2World::WI_END > uiWorldId );

	// 이동한 필드의 WorldID가 이전과 다르다면
	//if ( GetWorldId() != uiWorldId )
	{
		SetWorldId( uiWorldId );
		CX2World* pNewWorld = g_pData->GetWorldManager()->CreateWorld( static_cast<CX2World::WORLD_ID>( uiWorldId ), NULL, true );

		std::swap( m_pWorld, pNewWorld );

		SAFE_DELETE( pNewWorld );

		CreatePotalParticle();
		OpenBattleFieldName();
	}	

}

/** @function : IsNearPortalLineMap() const
	@brief : 포탈 라인맵에 내 유닛이 닿았는지를 검사
	@param : void
	@return : bool (닿았으면 true)
*/
bool CX2BattleFieldGame::IsNearPortalLineMap()
{
	ASSERT( GetMyUnit() );
	ASSERT( GetLineMap() );

	CX2GUUser* pMyGUUser = GetMyUnit();

	if ( pMyGUUser == NULL )
		return false;

	// 유저가 죽은 상태면 포탈 근처에서 죽었어도 처리 안함
	if ( pMyGUUser->GetGameUnitState() == CX2GameUnit::GUSI_DIE || !( pMyGUUser->GetNowHp()	> 0 ) )
		return false;

	const int iLastTouchedLineMapIndex = GetMyUnit()->GetLastTouchLineIndex( true );
	const CKTDGLineMap::LineData* pLastTouchedLineData = GetLineMap()->GetLineData( iLastTouchedLineMapIndex );

	ASSERT( pLastTouchedLineData );
	if ( NULL != pLastTouchedLineData )
	{		
#ifdef SERV_COMMON_AUTO_OPEN_HARD_CODE
		int iLineIdx = 0;
#endif // SERV_COMMON_AUTO_OPEN_HARD_CODE

#ifdef SERV_SANDER_AUTO_OPEN_HARD_CODE
		IF_EVENT_ENABLED( CEI_SANDER_AUTO_OPEN_HARD_CODE )
		{
			iLineIdx = 170;
		}
		ELSE
		{
			if( m_pWorld->GetWorldData()->worldID == CX2World::WI_BATTLE_FIELD_HAMEL_FIELD_03 &&
				CKTDGLineMap::LT_POTAL == pLastTouchedLineData->lineType &&
				pLastTouchedLineData->lineIndex == 170 )
			{
				return false;
			}
		}
#endif //SERV_SANDER_AUTO_OPEN_HARD_CODE

#ifdef SERV_COMMON_AUTO_OPEN_HARD_CODE
		if( iLineIdx != 0 && m_bOpenPotalParticle == false )
		{
			CreatePotalParticle( iLineIdx );
			m_bOpenPotalParticle = true;
		}
#endif // SERV_COMMON_AUTO_OPEN_HARD_CODE

		// Portal LineMap 이면 true 리턴
		return ( CKTDGLineMap::LT_POTAL == pLastTouchedLineData->lineType );
	}
	else
	{
		StateLog( L"lastTouchedLineData is wrong!" );
		return false;
	}	
}

void CX2BattleFieldGame::CreateBattleFieldNpcAtStart( const std::vector< KNPCUnitReq >& vecNPCDataList_
#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
								, const KAttribEnchantNpcInfoMap& mapAttribNpcInfo_ 
#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
								)
{
	BOOST_FOREACH( const KNPCUnitReq& kNpcUnitData, vecNPCDataList_ )
	{
#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
		const KAttribEnchantNpcInfo* pkAttribInfo = NULL;
		KAttribEnchantNpcInfoMap::const_iterator iter = mapAttribNpcInfo_.find( kNpcUnitData.m_UID );
		if ( iter != mapAttribNpcInfo_.end() )
			pkAttribInfo = &iter->second;
		CreateBattleFieldNpc( kNpcUnitData, pkAttribInfo );
#else	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
		CreateBattleFieldNpc( kNpcUnitData );
#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
	}

#ifdef	BATTLE_FIELD_TEST
	BOOST_FOREACH( CX2GUNPC* pNpc, m_NPCUnitList )
	{
		if ( NULL != pNpc )
			pNpc->SetCreatedBySyncNot( true );
	}
#endif	BATTLE_FIELD_TEST
	
}
void CX2BattleFieldGame::CreateBattleFieldNpcByNotData( const std::vector< KNPCUnitNot >& vecNPCDataList_
#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
								, const KAttribEnchantNpcInfoMap& mapAttribNpcInfo_ 
#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD		
	)
{
#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
	std::vector< KNPCUnitReq > vec;
	vec.reserve( vecNPCDataList_.size() );
	BOOST_FOREACH( const KNPCUnitNot& kNpcUnitData, vecNPCDataList_ )
	{
		vec.push_back( kNpcUnitData.m_kNPCUnitReq );
	}

	CreateNpcCreatedInfo( vec, mapAttribNpcInfo_, false );

#else	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD


	BOOST_FOREACH( const KNPCUnitNot& kNpcUnitData, vecNPCDataList_ )
	{
		CreateBattleFieldNpc( kNpcUnitData.m_kNPCUnitReq );
	}

#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
}


UINT CX2BattleFieldGame::GetEmpyIndexNpcUnitList()
{
	for ( UINT i = 0; i < m_NPCUnitList.size(); ++i )
	{
		if ( NULL == m_NPCUnitList[i] )
			return i;
	}

	return 0;
	ASSERT( !L"There are too many monsters" );
	StateLog( L"There are too many monsters" );
}

void CX2BattleFieldGame::CreateBattleFieldNpc( const KNPCUnitReq& kNpcUnitData_ 
#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
								, const KAttribEnchantNpcInfo* pkAttribNpcInfo_ )
#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
{	
//#ifdef	X2OPTIMIZE_NPC_LUASPACE_SHARING
	const CX2UnitManager::NPCUnitInfo*	pNPCUnitInfo = g_pData->GetUnitManager()->GetNPCUnitInfo(
		static_cast<CX2UnitManager::NPC_UNIT_ID>( kNpcUnitData_.m_NPCID ) );
	ASSERT( pNPCUnitInfo != NULL );
//#else	X2OPTIMIZE_NPC_LUASPACE_SHARING
//	const CX2UnitManager::NPCUnitTemplet* pTemplet = 
//		g_pData->GetUnitManager()->GetNPCUnitTemplet( static_cast<CX2UnitManager::NPC_UNIT_ID>( kNpcUnitData_.m_NPCID ) );
//	ASSERT( NULL != pTemplet );
//#endif	X2OPTIMIZE_NPC_LUASPACE_SHARING

//#ifdef	X2OPTIMIZE_NPC_LUASPACE_SHARING
	if ( NULL != pNPCUnitInfo )
//#else	X2OPTIMIZE_NPC_LUASPACE_SHARING
//	if ( NULL != pTemplet )
//#endif	X2OPTIMIZE_NPC_LUASPACE_SHARING
	{
		// Ally Npc의 경우
		if ( -1 < kNpcUnitData_.m_iAllyUID )
		{
			CX2GUUser* pOwnerUser = GetUserUnitByUID( kNpcUnitData_.m_iAllyUID );

			CreateNPC( static_cast<CX2UnitManager::NPC_UNIT_ID>( kNpcUnitData_.m_NPCID ),
				kNpcUnitData_.m_Level, kNpcUnitData_.m_bActive, kNpcUnitData_.m_UID, 
				D3DXVECTOR3( kNpcUnitData_.m_vPos.x, kNpcUnitData_.m_vPos.y, kNpcUnitData_.m_vPos.z ),
				kNpcUnitData_.m_bIsRight, kNpcUnitData_.m_fDelayTime, kNpcUnitData_.m_KeyCode, 
				static_cast<CX2Room::TEAM_NUM>( kNpcUnitData_.m_cTeamNum ), 
				static_cast<CX2NPCAI::NPC_AI_TYPE>( kNpcUnitData_.m_cAIType ), kNpcUnitData_.m_iAllyUID, 
				0, static_cast<CX2Room::TEAM_NUM>(kNpcUnitData_.m_cAllyTeam));
		}
		else
		{
			CNpcCreatedInfoPtr NpcCreatedInfoPtr( CNpcCreatedInfo::CreateNpcCreatedInfoPtr( 0, kNpcUnitData_ ) );
			CX2GUNPC* pBattleFieldNpc = CX2GUNPC::CreateBattleFieldNpc( NpcCreatedInfoPtr, 
//#ifdef	X2OPTIMIZE_NPC_LUASPACE_SHARING
				pNPCUnitInfo
//#else	X2OPTIMIZE_NPC_LUASPACE_SHARING
//				pTemplet 
//#endif	X2OPTIMIZE_NPC_LUASPACE_SHARING
				);

			ASSERT( NULL != pBattleFieldNpc );
			if ( NULL != pBattleFieldNpc )
			{
				ASSERT( -1 != kNpcUnitData_.m_nStartPos );
				pBattleFieldNpc->Init( false, kNpcUnitData_.m_nStartPos );
#ifndef SERV_BATTLEFIELD_MIDDLE_BOSS
				// 위험도 증가에 따라 보스 몬스터가 나오면 카메라 및 키입력 처리
				if ( true == kNpcUnitData_.m_bFocusCamera )
				{
					pBattleFieldNpc->SetFocusCamera( true );
					GetMyUnit()->SetCanNotInputTime_LUA( 4.f );
					PauseNPCAI( 4.f );
				}
#endif // SERV_BATTLEFIELD_MIDDLE_BOSS
				// 위 소스는 CreateBattleFieldNpcByCreatedInfo 에서 직접 Camera 인자를 받으므로
				// 필요 없어짐
				int iLastTouchedIndex = pBattleFieldNpc->GetLastTouchLineIndex( false );
				pBattleFieldNpc->SetHardLevel( kNpcUnitData_.m_Level, true );
				pBattleFieldNpc->SetActiveMonster( kNpcUnitData_.m_bActive );
				
				AddUnit( pBattleFieldNpc );
				pBattleFieldNpc->SetUnitIndex( GetEmptyNPCIndex() );
				SetNPCUnit( pBattleFieldNpc->GetUnitIndex(), pBattleFieldNpc );

#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
				if ( pkAttribNpcInfo_ != NULL && pkAttribNpcInfo_->IsValidInfo() == true )
					pBattleFieldNpc->ApplyEnchantMonster( *pkAttribNpcInfo_ );
#else	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
				if ( !m_mapAttribNpcInfo.empty() )
				{
					KAttribEnchantNpcInfoMap::const_iterator mitr = m_mapAttribNpcInfo.find( pBattleFieldNpc->GetUID() );

					if ( m_mapAttribNpcInfo.end() != mitr )
					{
						const KAttribEnchantNpcInfo& kAttribEnchantNpcInfo = mitr->second;
						pBattleFieldNpc->ApplyEnchantMonster( kAttribEnchantNpcInfo );
					}
				}
#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD

				pBattleFieldNpc->InitFullName();
				pBattleFieldNpc->InitNameColor( GetMyUnit()->GetUnitLevel() );

				g_pKTDXApp->GetDGManager()->AddObjectChain( pBattleFieldNpc );
			}
			else
				StateLog( "FieldNpc is NULL in CreateBattleFieldNpc" );
		}		
	}
	else
		StateLog( "NpcTemplet is NULL in CreateBattleFieldNpc" );
}

void CX2BattleFieldGame::CreateBattleFieldNpcByCreatedInfo( const CNpcCreatedInfoPtr NpcCreatedInfoPtr_ )
{
//#ifdef	X2OPTIMIZE_NPC_LUASPACE_SHARING
	const CX2UnitManager::NPCUnitInfo*	pNPCUnitInfo = g_pData->GetUnitManager()->GetNPCUnitInfo(
		static_cast<CX2UnitManager::NPC_UNIT_ID>( NpcCreatedInfoPtr_->GetNpcId() ) );
	ASSERT( pNPCUnitInfo != NULL );
//#else	X2OPTIMIZE_NPC_LUASPACE_SHARING
//	const CX2UnitManager::NPCUnitTemplet* pTemplet = 
//		g_pData->GetUnitManager()->GetNPCUnitTemplet( static_cast<CX2UnitManager::NPC_UNIT_ID>( NpcCreatedInfoPtr_->GetNpcId() ) );
//	ASSERT( NULL != pTemplet );
//#endif	X2OPTIMIZE_NPC_LUASPACE_SHARING

//#ifdef	X2OPTIMIZE_NPC_LUASPACE_SHARING
	if ( pNPCUnitInfo != NULL )
//#else	X2OPTIMIZE_NPC_LUASPACE_SHARING
//	if ( NULL != pTemplet )
//#endif	X2OPTIMIZE_NPC_LUASPACE_SHARING
	{
		// Ally Npc의 경우
#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
 		if ( -1 < NpcCreatedInfoPtr_->GetAllyUid() )
 		{
 			//CX2GUUser* pOwnerUser = GetUserUnitByUID( NpcCreatedInfoPtr_->GetAllyUid() );
 
			CX2Room::TEAM_NUM eAllyTeam = CX2Room::TN_NONE;
			if( NULL != GetUserUnitByUID(NpcCreatedInfoPtr_->GetAllyUid()) )
				eAllyTeam = static_cast<CX2Room::TEAM_NUM>(GetUserUnitByUID(NpcCreatedInfoPtr_->GetAllyUid())->GetTeam());

 			CreateNPC( static_cast<CX2UnitManager::NPC_UNIT_ID>( NpcCreatedInfoPtr_->GetNpcId() ),
 				NpcCreatedInfoPtr_->GetLevel(), NpcCreatedInfoPtr_->GetActive(), NpcCreatedInfoPtr_->GetNpcUid(), 
				NpcCreatedInfoPtr_->GetPos(),
 				NpcCreatedInfoPtr_->GetIsRight(), NpcCreatedInfoPtr_->GetDelayTime(), NpcCreatedInfoPtr_->GetKeyCode(), 
 				static_cast<CX2Room::TEAM_NUM>( NpcCreatedInfoPtr_->GetTeamNum() ), 
 				static_cast<CX2NPCAI::NPC_AI_TYPE>( NpcCreatedInfoPtr_->GetAIType() ), NpcCreatedInfoPtr_->GetAllyUid(), 0,
				eAllyTeam ) ;
 		}
 		else
#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
		{
			CX2GUNPC* pBattleFieldNpc = CX2GUNPC::CreateBattleFieldNpc( NpcCreatedInfoPtr_, 
//#ifdef	X2OPTIMIZE_NPC_LUASPACE_SHARING
				pNPCUnitInfo
//#else	X2OPTIMIZE_NPC_LUASPACE_SHARING
//				pTemplet 
//#endif	X2OPTIMIZE_NPC_LUASPACE_SHARING
				);


			ASSERT( NULL != pBattleFieldNpc );
			if ( NULL != pBattleFieldNpc )
			{
				ASSERT( -1 != NpcCreatedInfoPtr_->GetStartPosNum() );

#ifdef NEXON_QA_CHEAT_REQ
				if( NULL != g_pData->GetMyUser() && g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_SPECIAL_USER )
				{
					if(  0 == NpcCreatedInfoPtr_->GetPetrolLineIndexList().size() )
					{
						const D3DXVECTOR3 mPos = NpcCreatedInfoPtr_->GetPos();
						pBattleFieldNpc->InitCheat( mPos, false );
					}
					else
					{
						pBattleFieldNpc->Init( false, NpcCreatedInfoPtr_->GetStartPosNum() );
					}
				}
				else
					pBattleFieldNpc->Init( false, NpcCreatedInfoPtr_->GetStartPosNum() );
#else
				pBattleFieldNpc->Init( false, NpcCreatedInfoPtr_->GetStartPosNum() );
#endif //NEXON_QA_CHEAT_REQ

				int iLastTouchedIndex = pBattleFieldNpc->GetLastTouchLineIndex( false );

#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD

#ifdef SERV_BATTLEFIELD_MIDDLE_BOSS
				UINT uiBossGroupID = NpcCreatedInfoPtr_->GetBossGroupId ();		// Boss Group ID 를 가져와 설정
				pBattleFieldNpc->SetBossGroupID ( uiBossGroupID );
				bool bFocusCamera = NpcCreatedInfoPtr_->GetFocusCamera();		// FocusCamera 를 가져와 설정
				pBattleFieldNpc->SetFocusCamera( bFocusCamera );			
				pBattleFieldNpc->SetIsMainBoss ( false );						// Main Boss 초기화
				if (  0 < uiBossGroupID )										// Boss Group ID 가 있다면
				{	
					if ( true == bFocusCamera )									// Focus Camera 가 있다면, 메인급 보스
					{
						pBattleFieldNpc->SetIsMainBoss( true );					// 메인 보스로 설정하고 indicator 를 무조건 메인급으로 
						SetIndicatorToNpc ( pBattleFieldNpc ); 						
						UpdateCreateUnitDisplayEffect ( pBattleFieldNpc, true);	// 메인 보스 등장 연출
						// 난입으로 들어왔을 때, 메인 보스가 없다면 패킷 받은 곳에서 처음 검색된 몬스터에게 메인 보스를 설정함
					}			
					if ( NULL == m_pMonsterIndicator )							// 처음 잡히는 몬스터를
						SetIndicatorToNpc ( pBattleFieldNpc ); 					// 보스 몬스터로 

					PauseNPCAI( 4.f );			

					if ( false == g_pX2Game->IsHost() )							// 방장이 아니면 DamagedMap 요청 패킷을 전송
					{
						KEGS_SYNC_BATTLE_FIELD_DAMAGED_MAP_REQ kEGS_SYNC_BATTLE_FIELD_DAMAGED_MAP_REQ;
						kEGS_SYNC_BATTLE_FIELD_DAMAGED_MAP_REQ.m_iNPCUID = static_cast<int> ( pBattleFieldNpc->GetUnitUID () );
						g_pData->GetServerProtocol()->SendPacket( EGS_SYNC_BATTLE_FIELD_DAMAGED_MAP_REQ, kEGS_SYNC_BATTLE_FIELD_DAMAGED_MAP_REQ );
						g_pMain->AddServerPacket( EGS_SYNC_BATTLE_FIELD_DAMAGED_MAP_ACK );							
					}
				}
#else // SERV_BATTLEFIELD_MIDDLE_BOSS
				if ( true == NpcCreatedInfoPtr_->GetFocusCamera() )
				{
					pBattleFieldNpc->SetFocusCamera( true );
					GetMyUnit()->SetCanNotInputTime_LUA( 4.f );
					PauseNPCAI( 4.f );
				}
				else
				{
					pBattleFieldNpc->SetFocusCamera( false );
				}

#endif // SERV_BATTLEFIELD_MIDDLE_BOSS

#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD

#ifndef NEXON_QA_CHEAT_REQ
				if ( CX2User::XUAL_NORMAL_USER < g_pData->GetMyUser()->GetAuthLevel() )
				{
					if ( NpcCreatedInfoPtr_->GetPetrolLineIndexList().end() == std::find( NpcCreatedInfoPtr_->GetPetrolLineIndexList().begin(), NpcCreatedInfoPtr_->GetPetrolLineIndexList().end(), iLastTouchedIndex ) )
					{
						WCHAR wLog[512];
						ZeroMemory( wLog, sizeof(WCHAR) * 512 );
						wsprintf( wLog, L"NPC_ID: %d, NPC_UID: %d, nStartPos: %d, LineMapIndex: %d\n", 
							NpcCreatedInfoPtr_->GetNpcId(), NpcCreatedInfoPtr_->GetNpcUid(), NpcCreatedInfoPtr_->GetStartPosNum(), iLastTouchedIndex );
						StringCchCat( wLog, 256, L"PetrolLineIndex: " );

						BOOST_FOREACH( const int& iIndex, NpcCreatedInfoPtr_->GetPetrolLineIndexList() )
						{
							WCHAR wBuffer[10];
							ZeroMemory( wBuffer, sizeof(WCHAR) * 10 );
							wsprintf( wBuffer, L" %d", iIndex );
							StringCchCat( wLog, 512, wBuffer );
						}

						g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2( -999, -999 ), wLog, g_pMain->GetNowState() );

					}
				}
#endif //NEXON_QA_CHEAT_REQ
			//#endif	BATTLE_FIELD_TEST

				pBattleFieldNpc->SetHardLevel( NpcCreatedInfoPtr_->GetLevel(), true );
				pBattleFieldNpc->SetActiveMonster( NpcCreatedInfoPtr_->GetActive() );

				//pBattleFieldNpc->SetKeyCode( kNpcUnitData_.m_KeyCode );

				AddUnit( pBattleFieldNpc );
				pBattleFieldNpc->SetUnitIndex( GetEmptyNPCIndex() );
				SetNPCUnit( pBattleFieldNpc->GetUnitIndex(), pBattleFieldNpc );
#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
				if ( NpcCreatedInfoPtr_->GetAttribEnchantNpcInfo().IsValidInfo() == true )
					pBattleFieldNpc->ApplyEnchantMonster( NpcCreatedInfoPtr_->GetAttribEnchantNpcInfo() );
#else	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
				if ( !m_mapAttribNpcInfo.empty() )
				{
					KAttribEnchantNpcInfoMap::const_iterator mitr = m_mapAttribNpcInfo.find( pBattleFieldNpc->GetUID() );

					if ( m_mapAttribNpcInfo.end() != mitr )
					{
						const KAttribEnchantNpcInfo& kAttribEnchantNpcInfo = mitr->second;
						pBattleFieldNpc->ApplyEnchantMonster( kAttribEnchantNpcInfo );
					}

				}
#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD

				pBattleFieldNpc->InitFullName();
				pBattleFieldNpc->InitNameColor( GetMyUnit()->GetUnitLevel() );

				g_pKTDXApp->GetDGManager()->AddObjectChain( pBattleFieldNpc );
			}
			else
			{
				ASSERT( !L"FieldNpc is NULL in CreateBattleFieldNpcByCreatedInfo" );
				StateLog( "FieldNpc is NULL in CreateBattleFieldNpcByCreatedInfo" );
			}
		}		
	}
	else
	{
		ASSERT( !L"NpcTemplet is NULL in CreateBattleFieldNpcByCreatedInfo" );
		StateLog( "NpcTemplet is NULL in CreateBattleFieldNpcByCreatedInfo" );
	}
}
//*/

bool CX2BattleFieldGame::Handler_EGS_JOIN_BATTLE_FIELD_NOT( KEGS_JOIN_ROOM_NOT& kPacket_ )
{
	return Handler_EGS_JOIN_ROOM_NOT( kPacket_ );
}

/*virtual*/ void CX2BattleFieldGame::UpdateUnitPointer()
{
	CX2Game::UpdateUnitPointer();
	for( int i = 0; i < (int)m_NPCUnitList.size(); i++ )
	{
		CX2GUNPC* pCX2GUNPC = m_NPCUnitList[i];
		if( pCX2GUNPC != NULL )
		{
#ifdef MODIFY_DUNGEON_STAGING //시네메틱 UI 필드에서 의도하지 않게 발생하는 오류 확인을 위한 로그 추가
			if( true == pCX2GUNPC->GetStopAtCurrentState() )
			{	//유저 키입력 제한 NPC AI비활성화
				CanNotInputAndDisalbeNPCAI(true);			}

			if( true == pCX2GUNPC->GetFocusCameraAtCurrentState() )
			{
				m_optrFocusUnit = pCX2GUNPC;
				if( true == pCX2GUNPC->GetApplyCinematicUI() && true == g_pMain->IsInheritStateMenu())
				{
					CX2StateMenu* pStateMenu = static_cast<CX2StateMenu*>( g_pMain->GetNowState() );
					if( NULL != pStateMenu )
					{	//포커스 카메라 동작 시에 유저 UI도 같이 숨기도록 설정
						pStateMenu->SetShowUI(false);
					}
				}
				return;
			}
			else
			{
				CX2StateMenu* pStateMenu = static_cast<CX2StateMenu*>( g_pMain->GetNowState() );
				if( NULL != pStateMenu )
				{
					pStateMenu->SetShowUI(true);
				}
			}
#endif //MODIFY_DUNGEON_STAGING

			//{{ kimhc // 2010.8.7 // 무조건 NPC에게 카메라가 가도록 하는 기능
#ifdef	FOCUS_CAMERA_NPC_FORCE
			if( pCX2GUNPC->GetFocusCameraForce() == true 
				|| ( pCX2GUNPC->GetFocusCamera() == true && pCX2GUNPC->GetStartState() == (int)pCX2GUNPC->GetGameUnitState() ) )
#else	FOCUS_CAMERA_NPC_FORCE
			if( pCX2GUNPC->GetFocusCamera() == true && pCX2GUNPC->GetStartState() == (int)pCX2GUNPC->GetGameUnitState() )
#endif	FOCUS_CAMERA_NPC_FORCE
				//}} kimhc // 2010.8.7 // 무조건 NPC에게 카메라가 가도록 하는 기능
			{
				m_optrFocusUnit = pCX2GUNPC;

#ifdef SERV_BATTLEFIELD_MIDDLE_BOSS
				// 카메라 포커싱을 위해 임시로 제작한 부분,
				// 실제 적용은 몬스터 State 로,  GetFocusCamera 이용해서 할 것
				// 카메라가 2번 뛰는 건, SetFocusCamera 때문 이므로 해당 부분 계속 문제 시 변경 할 것
				if ( true == pCX2GUNPC->GetIsMainBoss() &&					// Main 보스 이고
 					 true == m_pCinematicUI->GetIsCinematicUIActive () )	// Cinematic UI 가 활성화 상태가 아니라면
				{	
					// FocusCameraForce 를 해제하고 포커스를 원래대로 돌린다.
					pCX2GUNPC->SetFocusCameraForce( false );
					m_optrFocusUnit = GetMyUnit();
				}
				else		// 해당 경우가 아니면, FocusCamera 를 유지한다.
#endif // SERV_BATTLEFIELD_MIDDLE_BOSS
					return;
			}
		}
	}

	m_optrFocusUnit = GetMyUnit();
}

void CX2BattleFieldGame::CreateNpcCreatedInfo( const vector<KNPCUnitReq>& vecNpcData_
#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
								, const KAttribEnchantNpcInfoMap& mapAttribNpcInfo_ 
								, bool bSetNPCLoadComplete
#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
								)
{
	const D3DXVECTOR3& vMyPos = GetMyUnit()->GetPos();
#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
	std::deque<CNpcCreatedInfoPtr>& conNpcCreatedInfo = m_deqNpcCreatedInfo;
#else	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
	std::vector<CNpcCreatedInfoPtr>& conNpcCreatedInfo = m_vecNpcCreatedInfo;
#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
	unsigned	uiOldSize = conNpcCreatedInfo.size();

	BOOST_FOREACH( const KNPCUnitReq& kData, vecNpcData_ )
	{
		
		const D3DXVECTOR3 vNpcPos( kData.m_vPos.x, kData.m_vPos.y, kData.m_vPos.z );

		const float fDistance3Sq = GetDistance3Sq( vMyPos, vNpcPos );
#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
		const KAttribEnchantNpcInfo* pkAttribInfo = NULL;
		KAttribEnchantNpcInfoMap::const_iterator iter = mapAttribNpcInfo_.find( kData.m_UID );
		if ( iter != mapAttribNpcInfo_.end() )
			pkAttribInfo = &iter->second;
		CNpcCreatedInfoPtr spNpcCreatedInfo = CNpcCreatedInfo::CreateNpcCreatedInfoPtr( fDistance3Sq, kData, pkAttribInfo );
		//오현빈//2012-11-20//몬스터 소환류 스킬 동작하지 않는 오류 수정을 위해 SetAllyUid 주석처리
		//spNpcCreatedInfo->SetAllyUid( -1 );
				
		
#ifdef SERV_BATTLEFIELD_MIDDLE_BOSS
		
#else	// SERV_BATTLEFIELD_MIDDLE_BOSS
		spNpcCreatedInfo->SetFocusCamera( false );
#endif // SERV_BATTLEFIELD_MIDDLE_BOSS
		conNpcCreatedInfo.push_back( spNpcCreatedInfo );
#else	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
		conNpcCreatedInfo.push_back( CNpcCreatedInfo::CreateNpcCreatedInfoPtr( fDistance3Sq, kData ) );
#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
	}

	

#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
	auto MoreFarDistance3SqCompFunc = []( CNpcCreatedInfoPtr lhs_, CNpcCreatedInfoPtr rhs_ ) {
		return *lhs_ < *rhs_;
	};

	std::sort( conNpcCreatedInfo.begin() + uiOldSize, conNpcCreatedInfo.end(), MoreFarDistance3SqCompFunc );
	unsigned	uiSize = conNpcCreatedInfo.size();
	for( unsigned i = uiOldSize; i < uiSize; i++ )
	{
		m_UnitLoader.AddNPCUnitUIDResource( conNpcCreatedInfo[ i ]->GetNpcUid(), (CX2UnitManager::NPC_UNIT_ID) conNpcCreatedInfo[ i ]->GetNpcId()
			, &conNpcCreatedInfo[ i ]->GetAttribEnchantNpcInfo() );
	}
#else	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
	auto MoreFarDistance3SqCompFunc = []( CNpcCreatedInfoPtr lhs_, CNpcCreatedInfoPtr rhs_ ) {
		return *lhs_ > *rhs_;
	};
	std::sort( conNpcCreatedInfo.begin(), conNpcCreatedInfo.end(), MoreFarDistance3SqCompFunc );
#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD

#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
	if ( bSetNPCLoadComplete == true )
#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
		m_bIsNpcLoadComplete = true;

}

/*virtual*/ bool CX2BattleFieldGame::DeleteNPCUnitByUID( const UINT uiNpcUid_ )
{
	bool bReturn = CX2Game::DeleteNPCUnitByUID( uiNpcUid_ );

	if ( bReturn )
		DeleteNpcInCreatedInfoList( uiNpcUid_ );
	else
		bReturn = DeleteNpcInCreatedInfoList( uiNpcUid_ );

	return bReturn;
}

bool CX2BattleFieldGame::DeleteNpcInCreatedInfoList( const UINT uiNpcUid_ )
{
	bool bReturn = false;

#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
	std::deque<CNpcCreatedInfoPtr>&	conNpcCreatedInfo = m_deqNpcCreatedInfo;
	std::deque<CNpcCreatedInfoPtr>::iterator iter;
#else	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
	std::vector<CNpcCreatedInfoPtr>& conNpcCreatedInfo = m_vecNpcCreatedInfo;
	vector<CNpcCreatedInfoPtr>::iterator iter;
#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD

	if ( false == conNpcCreatedInfo.empty() )
	{
		for( iter = conNpcCreatedInfo.begin(); iter != conNpcCreatedInfo.end(); ++iter )
		{
			if ( (*iter)->GetNpcUid() == uiNpcUid_ )
			{
				conNpcCreatedInfo.erase( iter );
				bReturn = true;
				break;
			}
		}
	}
#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
	m_UnitLoader.RemoveNPCUnitUIDResource( uiNpcUid_ );
#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD

	return bReturn;
}

/*virtual*/ bool CX2BattleFieldGame::Handler_EGS_NPC_UNIT_DIE_NOT( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_NPC_UNIT_DIE_NOT kEvent;
	DeSerialize( pBuff, &kEvent );
	CX2PacketLog::PrintLog( &kEvent );

	CX2GUNPC* pCX2GUNPC = GetNPCUnitByUID( kEvent.m_nDieNPCUID );
	if ( NULL != pCX2GUNPC )
	{
#ifdef  X2OPTIMIZE_ROBUST_NPC_FINAL_STATE
        pCX2GUNPC->MarkNPCUnitDieNotReceived();
#else   X2OPTIMIZE_ROBUST_NPC_FINAL_STATE
        pCX2GUNPC->SetNowHp( 0.f );
#endif  X2OPTIMIZE_ROBUST_NPC_FINAL_STATE
		map<int, int>::iterator it;
		it = m_mapKilledNPCKeyCode.find( pCX2GUNPC->GetKeyCode() );
		if( it != m_mapKilledNPCKeyCode.end() )
		{
			m_mapKilledNPCKeyCode[ pCX2GUNPC->GetKeyCode() ] += 1;
		}
		else
		{
			m_mapKilledNPCKeyCode[ pCX2GUNPC->GetKeyCode() ] = 1;
		}
	}

	DeleteNpcInCreatedInfoList( kEvent.m_nDieNPCUID );
	ProcessExpListByNpcUnitDie( kEvent );
#ifdef ARA_CHARACTER_BASE
	ProcessIKillThisNpc( kEvent );
#endif

	return true;
}

void CX2BattleFieldGame::Send_EGS_UPDATE_BATTLE_FIELD_USER_POS_NOT()
{
	if ( NULL != m_pBattleFieldRoom && NULL != GetMyUnit() )
	{
		KEGS_UPDATE_BATTLE_FIELD_USER_POS_NOT kPacket;

		kPacket.m_kLastPos.m_iMapID = m_pBattleFieldRoom->GetBattleFieldId();
		kPacket.m_kLastPos.m_ucLastTouchLineIndex = GetMyUnit()->GetLastTouchLineIndex();

		
		CKTDGLineMap::LineData* pLineData =
			( NULL == GetLineMap() ? NULL : GetLineMap()->GetLineData( kPacket.m_kLastPos.m_ucLastTouchLineIndex ) );

		if ( NULL != pLineData )
		{
			// 라인데이터의 시작부터 끝까지의 길이제곱
			const float fDistance3SqFromStartToEnd = GetDistance3Sq( pLineData->startPos, pLineData->endPos );
			// 라인데이터의 시작부터 현재 자신의 위치 까지의 길이제곱
			const float fDistance3SqFromStartToMyPos = GetDistance3Sq( pLineData->startPos, GetMyUnit()->GetLandPos() );
			
			// 현재 라인데이터의 몇%에 위치하는지 알아내는지 알아내기 위함
			const float fLastPosValue3Sq = fDistance3SqFromStartToMyPos / fDistance3SqFromStartToEnd;

			if ( fLastPosValue3Sq > 0 )
				kPacket.m_kLastPos.m_usLastPosValue = static_cast<USHORT>( sqrtf( fLastPosValue3Sq ) * 100 );
			else
				kPacket.m_kLastPos.m_usLastPosValue = 1;
		}
		else
			kPacket.m_kLastPos.m_usLastPosValue = 1;

		CX2BattleFieldManager& battleFieldManager = g_pData->GetBattleFieldManager();
		battleFieldManager.SetBattleFieldIdWhereIam( m_pBattleFieldRoom->GetBattleFieldId() );
		battleFieldManager.SetBattleFieldPositionIndexWhereIShouldBe( GetMyUnit()->GetLastTouchLineIndex() );
		battleFieldManager.SetBattleFieldPositionValue( kPacket.m_kLastPos.m_usLastPosValue );
		battleFieldManager.SetNowBattleFieldPositionInfoStartPosition( false );

		g_pData->GetServerProtocol()->SendPacket( EGS_UPDATE_BATTLE_FIELD_USER_POS_NOT, kPacket );
	}
	else
	{
		ASSERT( !"Wrong Path!" );
		return;
	}
	
}

/*virtual*/ bool CX2BattleFieldGame::Handler_EGS_LEAVE_BATTLE_FIELD_NOT( const KEGS_LEAVE_BATTLE_FIELD_NOT& kPacket )
{
	if ( NULL != g_pX2Room && CX2Room::RT_BATTLE_FIELD == g_pX2Room->GetRoomType() )
	{
		DialogLog( "Before DeleteUserUnitByUID" );
		DeleteUserUnitByUID( kPacket.m_iUnitUID );

#ifndef	X2OPTIMIZE_GAME_CHARACTER_BACKGROUND_LOAD
		DialogLog( "Before DeleteUnitLoadData, After DeleteUserUnitByUID" );
		m_UnitLoader.DeleteUnitLoadData( kPacket.m_iUnitUID );
#endif	X2OPTIMIZE_GAME_CHARACTER_BACKGROUND_LOAD

		DialogLog( "Before SetRoomState, After DeleteUnitLoadData" );
		g_pX2Room->SetRoomState( static_cast<CX2Room::ROOM_STATE>( kPacket.m_cRoomState ) );

		DialogLog( "Before Set_KRoomSlotInfoList, After SetRoomState" );
		g_pX2Room->Set_KRoomSlotInfoList( kPacket.m_vecSlot );

		DialogLog( "Before UpdateUnitPointer, After Set_KRoomSlotInfoList" );
		UpdateUnitPointer();
#ifndef  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK

//#ifdef	BATTLE_FIELD_TEST
		//FieldTest
		BOOST_FOREACH( const KRoomSlotInfo& kRoomSlotInfo, kPacket.m_vecSlot )
		{
			if ( true == kRoomSlotInfo.m_bHost )
			{
				CX2GUUser* pHostGUUser = GetUserUnitByUID( kRoomSlotInfo.m_kRoomUserInfo.m_nUnitUID );
				if ( NULL != pHostGUUser)
					g_pX2Game->SetHostGameUnit( pHostGUUser );
				else
					g_pX2Game->ResetHostGameUnit();
			}
		}		
//#endif	BATTLE_FIELD_TEST
#endif  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK

		DialogLog( "After UpdateUnitPointer" );
		return true;
	}
	else
		return false;
	
}

/*virtual*/ void CX2BattleFieldGame::AddUserUnit( CX2Room::SlotData* pSlotData_, int iSlotIndex_ )
{
	ASSERT( static_cast<UINT>( iSlotIndex_ ) < m_UserUnitList.size() );
	if ( static_cast<UINT>( iSlotIndex_ ) < m_UserUnitList.size() )
	{
		CX2GUUser* pCX2UserUnit = m_UserUnitList[iSlotIndex_];

		// 유저리스트에 기존의 정보가 없으면
		if ( NULL == pCX2UserUnit )
		{
#ifndef	X2OPTIMIZE_GAME_CHARACTER_BACKGROUND_LOAD
			::EnterCriticalSection( &m_csGameIntruder );
#endif	X2OPTIMIZE_GAME_CHARACTER_BACKGROUND_LOAD
			pCX2UserUnit = CreateAndInsertUserUnit( pSlotData_, iSlotIndex_ );
#ifndef	X2OPTIMIZE_GAME_CHARACTER_BACKGROUND_LOAD
			::LeaveCriticalSection( &m_csGameIntruder );
#endif	X2OPTIMIZE_GAME_CHARACTER_BACKGROUND_LOAD
		}
		else	// 유저리스트에 기존의 정보가 있고
		{
			// 그것이 룸 슬롯 정보와 다른 유저라면
			if ( pSlotData_->m_UnitUID != pCX2UserUnit->GetUnitUID() )
			{
#ifndef	X2OPTIMIZE_GAME_CHARACTER_BACKGROUND_LOAD
				::EnterCriticalSection( &m_csGameIntruder );
#endif	X2OPTIMIZE_GAME_CHARACTER_BACKGROUND_LOAD
				DeleteUserUnit( iSlotIndex_ );
				pCX2UserUnit = CreateAndInsertUserUnit( pSlotData_, iSlotIndex_ );
#ifndef	X2OPTIMIZE_GAME_CHARACTER_BACKGROUND_LOAD
				::LeaveCriticalSection( &m_csGameIntruder );
#endif	X2OPTIMIZE_GAME_CHARACTER_BACKGROUND_LOAD
			}
			else
			{
				if ( true == pSlotData_->m_bMySlot )
				{
#ifdef  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
                    UpdateMyGameUnit( pSlotData_, pCX2UserUnit );
#else   SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
					m_optrMyUnit = pCX2UserUnit;
					CheckAmIHost();
#endif  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
				}

				if ( true == pSlotData_->m_bMySlot && false == g_pData->GetBattleFieldManager().GetNowBattleFieldPositionInfoStartPosition() )
					pCX2UserUnit->InitPosByBattleFieldPosition();
				else
					pCX2UserUnit->InitPosition( false, g_pData->GetBattleFieldManager().GetBattleFieldPositionIndexWhereIShouldBe() );
			}			

			pCX2UserUnit->ClearRecieveSyncData();
		}

		if ( NULL != pCX2UserUnit )
		{

//#ifdef	BATTLE_FIELD_TEST
			//FieldTest
			if ( pSlotData_->GetIsHost() )
				g_pX2Game->SetHostGameUnit( pCX2UserUnit );
//#endif	BATTLE_FIELD_TEST

			CX2GageManager::GetInstance()->SetPartyMemberGameUnit( pCX2UserUnit );	

#ifdef ADDED_RELATIONSHIP_SYSTEM
			if ( pSlotData_->m_RelationshipTargetUID > 0 )	// 새로 들어온 유저가 커플이라면
			{
				// 해당 유닛의 UID 를 받아와 GUUser 객체에 넣어둔다.
				pCX2UserUnit->SetRelationshipUnitUID ( pSlotData_->m_RelationshipTargetUID );
			}


			if ( NULL != g_pData->GetRelationshipManager() &&
				NULL != g_pData->GetRelationshipManager()->GetMyRelationshipInfo() )
			{
				CX2RelationshipManager * pRelationshipManager = g_pData->GetRelationshipManager();
				CX2RelationshipManager::RelationshipInfo * pRelationshipInfo = pRelationshipManager->GetMyRelationshipInfo();
				
				// 내 상태가 솔로가 아닐 때, 해당 UID 가 내 연인의 UID 라면
				// 내 연인으로 지정한다.
				if ( pRelationshipInfo->m_eRelationStateType >	SEnum::RT_SOLO )
				{
					if ( pRelationshipInfo->m_iRelationTargetUserUid == pCX2UserUnit->GetUnit()->GetUID() )
					{
						pRelationshipManager->SetRelationPartnerInGame ( CX2GameUnitoPtr( pCX2UserUnit ) );
					}
				}
			}

#endif // ADDED_RELATIONSHIP_SYSTEM
		}

	}
}

CX2GUUser* CX2BattleFieldGame::CreateAndInsertUserUnit( CX2Room::SlotData* pSlotData_, int iSlotIndex_ )
{
	CX2GUUser* pCX2UserUnit = CreateGUUser( pSlotData_, iSlotIndex_ );

	if ( NULL != pCX2UserUnit )
	{
		if ( true == pSlotData_->m_bMySlot )
		{
#ifdef  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
            UpdateMyGameUnit( pSlotData_, pCX2UserUnit );
#else   SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
			m_optrMyUnit = pCX2UserUnit;
			CheckAmIHost();
#endif  SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK
		}

		m_UnitList.push_back( static_cast<CX2GameUnit*>( pCX2UserUnit ) );
		m_UserUnitList[iSlotIndex_] = pCX2UserUnit;

		pCX2UserUnit->Init( false, g_pData->GetBattleFieldManager().GetBattleFieldPositionIndexWhereIShouldBe() );
		if ( pCX2UserUnit->IsMyUnit() )
			pCX2UserUnit->SetKBuffFactorFromGageManagerToGameUnit();

		g_pKTDXApp->GetDGManager()->AddObjectChain( pCX2UserUnit );
	}

	return pCX2UserUnit;
}

/*virtual*/ void CX2BattleFieldGame::ResurrectionKeyProcess()
{
	if ( GetMyUnit() != NULL && GetMyUnit()->GetGameUnitState() == CX2GameUnit::GUSI_DIE && 
		m_bIsSendResurrectionReq == false )
	{
		if ( false == GetOpenChatBox() )
		{
			if ( m_InputData.oneZ == true )
			{
				Handler_EGS_RESURRECT_TO_CONTINUE_DUNGEON_REQ( GetMyUnit()->GetUnit()->GetUID() );
			}
			else if ( m_InputData.oneX )
			{
				g_pMain->GetPartyUI()->CancelToWaitForDungeonPartyMatching();
				//Handler_EGS_SKIP_CONTINUE_DUNGEON_NOT();
				if ( NULL != g_pData->GetBattleFieldRoom() )
				{
					UINT uiBattleFieldId = g_pData->GetBattleFieldRoom()->GetBattleFieldId();
					UINT uiVillageId = g_pData->GetBattleFieldManager().GetReturnVillageId( uiBattleFieldId );
					CX2State* pState = static_cast<CX2State*>( g_pMain->GetNowState() );
					if ( NULL != pState )
						pState->Handler_EGS_STATE_CHANGE_FIELD_REQ( uiVillageId );
				}
				m_fResurrectionRemainTime = 0.0f;
			}
		}
	}
}

void CX2BattleFieldGame::OpenBattleFieldName()
{
	SAFE_DELETE_DIALOG( m_pDlgBattleFieldName );

	m_pDlgBattleFieldName = new CKTDGUIDialog( g_pMain->GetNowState(), L"DLG_UI_BATTLE_FIELD_NAME.LUA" );
	ASSERT( NULL != m_pDlgBattleFieldName );
	if ( m_pDlgBattleFieldName != NULL )
	{
		CX2BattleFieldManager& battleFieldManager = g_pData->GetBattleFieldManager();

		CKTDGUIStatic* pStaticBattleFieldName = static_cast<CKTDGUIStatic*>( m_pDlgBattleFieldName->GetControl( L"BATTLE_FIELD_NAME" ) );
		if ( NULL != pStaticBattleFieldName )
		{
			CKTDGUIControl::CPictureData* pPictureBattleFieldName = pStaticBattleFieldName->GetPicture( 0 );
			if ( NULL != pPictureBattleFieldName )
			{
				const WCHAR* pTextureName = NULL;
				const WCHAR* pTextureKey = NULL;
				if ( battleFieldManager.GetBattleFieldTextureNameAndKey( 
					battleFieldManager.GetBattleFieldIdWhereIam(), &pTextureName, &pTextureKey ) )
					pPictureBattleFieldName->SetTex( pTextureName, pTextureKey );
			}
		}

		g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDlgBattleFieldName );		

		m_pDlgBattleFieldName->Move( m_pDlgBattleFieldName->GetPos(), D3DXCOLOR(1.f, 1.f, 1.f, 0.f), 5.0f, false, true );

		if( NULL != g_pInstanceData->GetMiniMapUI() )
		{
			g_pInstanceData->GetMiniMapUI()->SetVillageMapID( battleFieldManager.GetBattleFieldIdWhereIam() );
		}
	}
}


void CX2BattleFieldGame::ResurrectOtherUser()
{
	if ( GetMyUnit() != NULL &&
		m_bIsSendResurrectionReq == false &&
		m_pDLGResurrect == NULL &&
		m_pDLGCheckResurrectByUseCash == NULL)
	{

#ifdef FIX_RESURRECTION_INDEX // 적용날짜: 2013-04-25

		UidType uidSelectUserUID = -1;

		if ( g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState( DIK_F2 ) == TRUE )
		{
			CX2GageManager* pGageManager = CX2GageManager::GetInstance();

			if ( NULL != pGageManager )
				uidSelectUserUID = pGageManager->GetPartyGageUIPositionIndexByUserList( 2 );
		}
		else if ( g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState( DIK_F3 ) == TRUE )
		{
			CX2GageManager* pGageManager = CX2GageManager::GetInstance();

			if ( NULL != pGageManager )
				uidSelectUserUID = pGageManager->GetPartyGageUIPositionIndexByUserList( 1 );
		}
		else if ( g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState( DIK_F4 ) == TRUE )
		{
			CX2GageManager* pGageManager = CX2GageManager::GetInstance();

			if ( NULL != pGageManager )
				uidSelectUserUID = pGageManager->GetPartyGageUIPositionIndexByUserList( 0 );
		}

		if ( uidSelectUserUID != - 1 )
		{
			for ( int i = 0; i < GetUserUnitListSize(); i++ )
			{
				CX2GUUser* pUserUnit = GetUserUnit(i);

				if ( NULL != pUserUnit )
				{
					UidType uidNowUserUID = pUserUnit->GetUnitUID();

					if ( uidNowUserUID == uidSelectUserUID && pUserUnit->GetGameUnitState() == CX2GameUnit::GUSI_DIE )
					{
						m_UnitUIDToResurrect = uidNowUserUID;
						wstring wstrResurrectText = L"";

						wstrResurrectText = GET_REPLACED_STRING( ( STR_ID_140, "S", pUserUnit->GetUnit()->GetNickName() ) );
						SAFE_DELETE_DIALOG( m_pDLGResurrect );
						m_pDLGResurrect = g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2(250, 300), wstrResurrectText.c_str(), 
							CX2State::SUCM_RESURRECT_OK, g_pMain->GetNowState(), 
							CX2State::SUCM_RESURRECT_CANCEL );

						break;
					}
				}
			}
		}

#else // FIX_RESURRECTION_INDEX

		int sequenceToFind = -1;

		if ( g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState( DIK_F2 ) == TRUE )
		{
			sequenceToFind = 3;	
		}
		else if ( g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState( DIK_F3 ) == TRUE )
		{
			sequenceToFind = 2;
		}
		else if ( g_pKTDXApp->GetDIManager()->Getkeyboard()->GetKeyState( DIK_F4 ) == TRUE )
		{
			sequenceToFind = 1;
		}

		if ( sequenceToFind != - 1 )
		{
			int tempCount = 1;
			for ( int i = 0; i < GetUserUnitListSize(); i++ )
			{
				CX2GameUnit* pUnit = GetUnit(i);
				if ( pUnit != NULL && g_pData->GetBattleFieldRoom()->GetMySlot()->m_Index != i )
				{
					if ( tempCount == sequenceToFind )
					{
						if ( pUnit->GetGameUnitState() == CX2GameUnit::GUSI_DIE )
						{
							m_UnitUIDToResurrect = pUnit->GetUnitUID();
							wstringstream wstrstm;
							CX2GUUser* pGUUSer = (CX2GUUser*)pUnit;
							wstrstm << GET_REPLACED_STRING( ( STR_ID_140, "S", pGUUSer->GetUnit()->GetNickName() ) );
							SAFE_DELETE_DIALOG( m_pDLGResurrect );
							m_pDLGResurrect = g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2(250, 300), wstrstm.str().c_str(), CX2State::SUCM_RESURRECT_OK, 
								g_pMain->GetNowState(), CX2State::SUCM_RESURRECT_CANCEL );
						}
						break;
					}
					tempCount++;
				}
			}
		}

#endif // FIX_RESURRECTION_INDEX
	}
}

void	CX2BattleFieldGame::ClearNpcCreatedInfoList() 
{ 
#ifdef	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
	m_deqNpcCreatedInfo.clear(); 
	m_UnitLoader.RemoveAllNPCResources();
#else	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
	m_vecNpcCreatedInfo.clear(); 
#endif	X2OPTIMIZE_FIELD_NPC_BACKGROUND_LOAD
	m_bIsNpcLoadComplete = false;
}

#ifdef SERV_BATTLEFIELD_MIDDLE_BOSS

CX2GUNPC * CX2BattleFieldGame::GetChainedBossGroup ( CX2GUNPC * pGUNPC_ )
{
	// 우선 순위, 1순위 : Main 급 보스, 2순위 : 최우선 검색되는 보스
	if ( pGUNPC_ != NULL )
	{
		int uiGroupID = pGUNPC_->GetBossGroupID();
		int  iUnitID   = pGUNPC_->GetUID();
		CX2GUNPC * pSecondPickedNPC = NULL;		// 2순위 NPC, Main Boss 급이 없을 때 반환되며 
		// 첫번째 검색된 유닛 Pointer 를 갖고 있는다.

		for( int i = 0; i < (int)m_NPCUnitList.size(); i++ )
		{
			CX2GUNPC * pNPC = m_NPCUnitList[i];
			if ( NULL != pNPC )
			{
				// 같은 그룹이고 자기 자신이 아니라면..그 유닛이 같은 그룹에 속한 유닛이다!
				if ( pNPC->GetBossGroupID() == uiGroupID && pNPC->GetUID() != iUnitID )
				{
					if ( pNPC->GetIsMainBoss() )		// Main Boss 를 찾았다면 바로 반환하고 끝
					{
						return pNPC;
					}
					else if ( pSecondPickedNPC == NULL )	// 2순위 NPC 를 설정, 
					{
						pSecondPickedNPC = pNPC;
					}
				}
			}
		}
		return pSecondPickedNPC; // 로직이 여기 왔다는 건 Main boss 가 없다는 것
	}
	return NULL;				 // 해당 NPC 가 NULL 이라면 NULL 반환
}

void CX2BattleFieldGame::UpdateCreateUnitDisplayEffect ( CX2GUNPC * pNPC_, bool bIsBossSummoned_ )
{
	// bIsBossSummoned_ 는 보스가 출현했을때는 true, 죽었을 때는 false

	// SetFadeInOutNotice 함수를 사용, 그룹 이름을 나타낸다.
	CX2BattleFieldManager& battleFieldManager = g_pData->GetBattleFieldManager();
	m_wstrBossGroupName = battleFieldManager.GetBattleFieldBossDataGroupName(battleFieldManager.GetBattleFieldIdWhereIam(), pNPC_->GetBossGroupID() );
	if ( m_wstrBossGroupName != L"NoData" )
	{
		wstring tempEx = L" ";
		if ( true == bIsBossSummoned_ )
		{
			// 부사를 계산, @1 등장하였습니다. ( 가/ 이 ) 
#ifdef REMOVE_POSTPOSITION_IN_REPLACE_STRING
			tempEx = GET_REPLACED_STRING ( ( STR_ID_23588, "L", std::wstring(L"") ) );
#else
			// 부사를 계산, @1 등장하였습니다. ( 가/ 이 ) 
			tempEx = GET_REPLACED_STRING ( ( STR_ID_23588, "I", KHanSungPostWordUnicodeWrapper( ( WCHAR* ) m_wstrBossGroupName.c_str(), STR_ID_806, STR_ID_807 ) ) );
#endif //REMOVE_POSTPOSITION_IN_REPLACE_STRING
			
		}
		else
		{
			// 부사를 계산, @1 등장하였습니다. ( 가/ 이 ) 
#ifdef REMOVE_POSTPOSITION_IN_REPLACE_STRING
			tempEx = GET_REPLACED_STRING ( ( STR_ID_23589, "L", std::wstring(L"") ) );
#else
			// 부사를 계산, @1 처치하였습니다.. ( 을/ 를 ) 
			tempEx = GET_REPLACED_STRING ( ( STR_ID_23589, "I", KHanSungPostWordUnicodeWrapper( ( WCHAR* ) m_wstrBossGroupName.c_str(), STR_ID_198, STR_ID_199 ) ) );
#endif //REMOVE_POSTPOSITION_IN_REPLACE_STRING
			
		}

		m_wstrBossGroupName += tempEx;
		static_cast <CX2State *> ( g_pMain->GetNowState() )->SetFadeInOutNotice( m_wstrBossGroupName.c_str(), 5.f);
	}
	
	// 몬스터가 등장했을 때, 카메라 및 입력 제어, 이펙트를 출력한다.
	if ( true == bIsBossSummoned_ )
	{	
		m_pCinematicUI->SetCinematicUI ( true, 4.f );		// 4초간 Cinematic UI 
		pNPC_->SetFocusCameraForce( true );					// 카메라 강제 고정
		GetMyUnit()->SetCanNotInputTime_LUA( 4.f );			// Input 을 4초 동안 못받게
	}
}


void CX2BattleFieldGame::SetIndicatorToNpc ( CX2GUNPC * pNPC_ )
{
	if ( NULL == m_pMonsterIndicator )
	{
		m_pMonsterIndicator = new CX2MonsterIndicator(CX2MonsterIndicator::MIT_FIELD_MIDDLE_BOSS);
	}
	m_pMonsterIndicator->SetTargetNPC( pNPC_ );
}
#endif // SERV_BATTLEFIELD_MIDDLE_BOSS

#ifdef SERV_CHECK_EXIST_MONSTER_UID
/** @function : StartElapsedTimeSinceGettingFirstNpcSyncPacket
	@brief : 이 함수가 호출 된 후 10초 정도 후에 NPC가 첫 싱크 패킷을 받았는지를 검사한다.
*/
void CX2BattleFieldGame::StartElapsedTimeSinceGettingFirstNpcSyncPacket()
{
	SetCanCheckGettingFirstNpcSyncPacket( true );
	m_ElapsedTimeSinceGettingFirstNpcSyncPacket.ResetSumOfElapsedTime();
}

/** @function : CheckNpcNotReceiveFirstSyncPacketAndRequestToCheckNpcUidList
	@brief : 첫 싱크 패킷을 못받은 NPC를 조사한 후에 서버에 체크를 요청하는 함수
*/
void CX2BattleFieldGame::CheckNpcNotReceiveFirstSyncPacketAndRequestToCheckNpcUidList()
{
	KEGS_CHECK_EXIST_MONSTER_UID_REQ kPacket;
	CheckNpcNotReceiveFirstSyncPacketAndCollectNpcUid( kPacket.m_vecNpcUID );

	if ( false == kPacket.m_vecNpcUID.empty() )
		Handler_EGS_CHECK_EXIST_MONSTER_UID_REQ( kPacket );
}

/** @function : CheckNpcNotReceiveFirstSyncPacketAndCollectNpcUid
	@brief : 싱크패킷을 한번도 못받은 NPC들을 체크하고, 해당 UID를 수집하는 함수
	@param : 싱크패킷을 한번도 못받은 NPC의 UID를 모아놓을 벡터
*/
void CX2BattleFieldGame::CheckNpcNotReceiveFirstSyncPacketAndCollectNpcUid( vector<int>& vecNpcUidToCheck_ ) const
{
	BOOST_FOREACH( CX2GUNPC* pGUNpc, m_NPCUnitList )
	{
		/// 아직 첫 싱크 패킷을 한번도 받지 못한 NPC라면
		if ( NULL != pGUNpc && false == pGUNpc->DidReceiveFirstSyncPacket() )
			vecNpcUidToCheck_.push_back( pGUNpc->GetUID() );	/// 해당 NPC의 UID를 체크 목록에 넣는다.
	}	
}

/** @function : Handler_EGS_CHECK_EXIST_MONSTER_UID_REQ
	@brief : 싱크패킷을 못받은 NPC들의 UID 를 서버에 전달 하는 함수
	@param : 서버로 전송할 EGS_CHECK_EXIST_MONSTER_UID_REQ 패킷으로 그안에는 싱크패킷을 못받은 NPC들의 UID 리스트가 있다.
*/
void CX2BattleFieldGame::Handler_EGS_CHECK_EXIST_MONSTER_UID_REQ( const KEGS_CHECK_EXIST_MONSTER_UID_REQ& kPacket_ )
{
 	g_pData->GetServerProtocol()->SendPacket( EGS_CHECK_EXIST_MONSTER_UID_REQ, kPacket_ );
 	g_pMain->AddServerPacket( EGS_CHECK_EXIST_MONSTER_UID_ACK );
}

/** @function : Handler_EGS_CHECK_EXIST_MONSTER_UID_ACK
	@brief : 현재 필드에 존재하지 않는 몬스터의 UID가 담긴 패킷 처리, 해당 NPC들을 삭제해 준다.
	@return : 패킷을 정상적으로 처리 했으면 true, 아니면 false
*/
bool CX2BattleFieldGame::Handler_EGS_CHECK_EXIST_MONSTER_UID_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_CHECK_EXIST_MONSTER_UID_ACK kPacket;
	DeSerialize( pBuff, &kPacket );

	if( g_pMain->DeleteServerPacket( EGS_CHECK_EXIST_MONSTER_UID_ACK ) == true )
	{
		if ( g_pMain->IsValidPacket( kPacket.m_iOK ) == true )
		{
			if ( false == kPacket.m_vecNotExistNpcUID.empty() )
			{
				StateLog( L"m_vecNotExistNpcUID has elements" );
				BOOST_FOREACH( const int iNpcUid, kPacket.m_vecNotExistNpcUID )
				{
					DeleteNPCUnitByUID( iNpcUid );
				}
			}
			return true;
		}
	}

	return false;
}

#endif // SERV_CHECK_EXIST_MONSTER_UID


#ifdef NEW_CHARACTER_EL
/** @function : GetIsExsitancePartyMemberHPLessThanPer
	@brief : 배틀필드에서는 무조건 false
*/
/*virtual*/ 
bool CX2BattleFieldGame::GetIsExsitancePartyMemberHPLessThanPer( const float fHPRate_, const CX2GUUser* pGUUserWhoCallFunction_ ) const
{
	return false;
}
#endif // NEW_CHARACTER_EL
