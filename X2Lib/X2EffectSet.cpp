#include "StdAfx.h"
#include ".\x2effectset.h"




#pragma DMLEE_NOTE( "effect set에 사운드 작업할 수 있게 기능 추가해야 함" )


CKTDXSimpleHandleInterface< CX2EffectSet::EffectSetInstance > CX2EffectSet::s_HandleManager;



CX2EffectSet::CX2EffectSet(void)
{
	m_vecpEffectSetInstance.reserve( 64 );
}

CX2EffectSet::~CX2EffectSet(void)
{
	ClearTempletAndInstance();
}




void CX2EffectSet::ClearTempletAndInstance()
{
	BOOST_TEST_FOREACH( EffectSetInstance*, pEffectSetInstance, m_vecpEffectSetInstance )
	{
		SAFE_DELETE( pEffectSetInstance );
	}
	m_vecpEffectSetInstance.clear();


	BOOST_TEST_FOREACH( EffectSetDataMap::value_type&, value, m_mapEffectSetTemplet )
	{
		SAFE_DELETE( value.second );
	}
	m_mapEffectSetTemplet.clear();

#ifdef EFFECT_TOOL
	m_vecEffectSetTemplet.clear();
	m_mapEffectSetDescription.clear();
#endif //EFFECT_TOOL
}


void CX2EffectSet::OnFrameMove( double fTime, float fElapsedTime )
{

	KTDXPROFILE();
	OnFrameMove_CreateEffect(fTime, fElapsedTime);
	OnFrameMove_DeleteEffect();
	OnFrameMove_UpdateEffect();
	OnFrameMove_StartCameraShake();

}

void CX2EffectSet::OnFrameMove_CreateEffect( double fTime, float fElapsedTime )
{

	KTDXPROFILE();

	// 이펙트 생성 및 effectsetinstance framemove
	BOOST_TEST_FOREACH( EffectSetInstance*, pEffectSetInstance, m_vecpEffectSetInstance )
	{
		pEffectSetInstance->OnFrameMove( fTime, fElapsedTime );

		bool bTraceTargetDead = false;
		for( UINT i=0; i<pEffectSetInstance->m_vecEffectInstanceCreated.size(); i++ )
		{
			if( true == pEffectSetInstance->m_vecEffectInstanceCreated[i] )
				continue;

			EffectData* pEffectData = pEffectSetInstance->m_pEffectSetData->m_vecpEffectData[i];
			if( NULL == pEffectData )
				continue;




			if( false == bTraceTargetDead && 
				true == pEffectData->m_bStartWhenTraceTargetDead &&
				null != pEffectSetInstance->m_optrTraceTargetGameUnit &&
				NULL != g_pX2Game &&
				pEffectSetInstance->m_optrTraceTargetGameUnit->GetNowHp() <= 0.f )						// trace target이 죽으면 이펙트 바로 생성 
			{
				bTraceTargetDead = true;
			}



			if( false == bTraceTargetDead &&
				0.f != pEffectData->m_fStartAnimTime + pEffectSetInstance->m_fDelayTime ) // 0초에 시작이면 무조건 바로 생성
			{
				EFFECT_SET_TIMER_TYPE eTimingType = pEffectSetInstance->m_pEffectSetData->m_eEventTimerType;
				if( false == pEffectSetInstance->m_bUseAnimationTiming &&
					ESTT_ANIM == eTimingType )
				{
					eTimingType = ESTT_CUSTOM;
				}
#ifdef EFFECT_TOOL
				eTimingType = ESTT_CUSTOM;
#endif //EFFECT_TOOL
				switch( eTimingType )
				{
				default:
				case ESTT_ANIM:
					{
#ifdef SERV_PET_SYSTEM
						if( pEffectSetInstance->m_pPet != NULL )
						{
							if( pEffectSetInstance->m_pPet->GetXSkinAnim() != NULL &&
								pEffectSetInstance->m_pPet->GetXSkinAnim()->GetNowAnimationTime() < pEffectData->m_fStartAnimTime + pEffectSetInstance->m_fDelayTime )
								continue;
						}
#ifdef RIDING_SYSTEM
						else if ( NULL != pEffectSetInstance->GetOwnerUnit() && 
								  NULL != pEffectSetInstance->GetOwnerUnit()->GetRidingPet() && 
								  true == pEffectSetInstance->GetOwnerUnit()->GetRidingOn() )
						{
							if( pEffectSetInstance->GetOwnerUnit()->GetRidingPet()->GetXSkinAnimPtr() != NULL &&
								pEffectSetInstance->GetOwnerUnit()->GetRidingPet()->GetXSkinAnimPtr()->GetNowAnimationTime() < pEffectData->m_fStartAnimTime + pEffectSetInstance->m_fDelayTime )
								continue;
						}
#endif //RIDING_SYSTEM
						else
						{
							if( pEffectSetInstance->m_pXSkinAnim->GetNowAnimationTime() < pEffectData->m_fStartAnimTime + pEffectSetInstance->m_fDelayTime )
								continue;
						}						
#else
						if( pEffectSetInstance->m_pXSkinAnim->GetNowAnimationTime() < pEffectData->m_fStartAnimTime + pEffectSetInstance->m_fDelayTime )
							continue;
#endif						
					} break;

				case ESTT_CUSTOM:
					{
						if( pEffectSetInstance->m_fElapsedTime < pEffectData->m_fStartAnimTime + pEffectSetInstance->m_fDelayTime )
						{
							continue;
						}
					} break;
				}
			}

			pEffectSetInstance->m_vecEffectInstanceCreated[i] = true;

			if( true == bTraceTargetDead &&
				false == pEffectData->m_bStartWhenTraceTargetDead )
			{
				continue;
			}

#ifdef MODIFY_CREATE_EFFECT_SET_LIMIT_DISTANCE
			// 거리 제한이 지정되어 있다면
			if ( pEffectData->m_fLimitDistanceSq > 0.f && 
				NULL != g_pX2Game && 
				NULL != g_pX2Game->GetMyUnit() )
			{
#ifdef _IN_HOUSE_
				if( ET_DAMAGE_EFFECT == pEffectData->m_eEffectType )
					DISPLAY_ERROR(L" DamageEffect에는 LIMIT_DISTANCE를 설정 하지 마세요! ");
#endif // _IN_HOUSE_

				// 거리 제한에 걸리면 만든 것으로 체크하고 실제로 만들지는 않도록 설정
				const D3DXVECTOR3& vMyUserPos = g_pX2Game->GetMyUnit()->GetPos();
				if( GetDistance3Sq( vMyUserPos, pEffectSetInstance->m_vPosition ) > pEffectData->m_fLimitDistanceSq )
				{
					pEffectSetInstance->m_vecEffectInstanceCreated[i] = true;
					continue;
				}
			}

#endif // MODIFY_CREATE_EFFECT_SET_LIMIT_DISTANCE

			const D3DXVECTOR3 vEffectPos = pEffectSetInstance->GetEffectPosition( pEffectData );
			const D3DXVECTOR3 vEffectRotation = pEffectSetInstance->GetEffectRotation( pEffectData );
			const D3DXVECTOR3 vEffectScale = pEffectSetInstance->GetEffectScale( pEffectData );

			CX2DamageEffect::CEffect* pEffect = NULL;
			CKTDGXMeshPlayer::CXMeshInstance* pMeshInstance = NULL;
			CKTDGParticleSystem::CParticleEventSequence* pParticleSequence = NULL;

			switch( pEffectData->m_eEffectType )
			{
			case ET_DAMAGE_EFFECT:
				{
				#ifdef SERV_PET_SYSTEM
					CX2GameUnit* pUnit = pEffectSetInstance->GetOwnerUnit();
					bool bPetDamageEffect = ( NULL != pEffectSetInstance->m_pPet ? true : false );
				#endif

					float fPowerRate = pEffectSetInstance->m_fPowerRate;
					if( fPowerRate < 0.f )
					{
#ifdef SERV_PET_SYSTEM
						if( NULL != pUnit )
						{
#ifdef PET_ATTACK_NOBUFF
							if( bPetDamageEffect == true )
							{
								if( pUnit->GetRemainHyperModeTime() > 0.f )
									fPowerRate = 1.2f;
								else
									fPowerRate = 1.f;
							}
							else
#endif
							{
								fPowerRate = pUnit->GetPowerRate();
							}							
						}
						else
						{
							ASSERT( !"effectset owner invalid" );
							pUnit = NULL;
							pEffectSetInstance->m_optrGameUnit.Reset();
						}
#else
						if( null != pEffectSetInstance->m_optrGameUnit )
						{
							fPowerRate = pEffectSetInstance->m_optrGameUnit->GetPowerRate();
						}
						else
						{
							ASSERT( !"effectset owner invalid" );
							pEffectSetInstance->m_optrGameUnit.Reset();
						}
#endif
					}
					
					fPowerRate *= pEffectSetInstance->m_fPowerRateScale;

					if ( NULL != g_pX2Game )
					{
#ifdef SERV_PET_SYSTEM
						if( bPetDamageEffect == true )
						{
						float fPowerRatePet = 1.f;
#ifdef ADD_UPGRADE_PET01
						// 펫 친밀도가 70%이상일 경우 배율 증가
						if( pEffectSetInstance->m_pPet != NULL && pEffectSetInstance->m_pPet->GetNowIntimacy() >= 0.7f )
							fPowerRatePet = ( pEffectSetInstance->m_pPet->GetNowIntimacy() - 0.6f + 1.f );
#endif
						pEffect = g_pX2Game->GetDamageEffect()->CreateInstance( pUnit, pEffectData->m_EffectName.c_str(), 
								fPowerRate * fPowerRatePet, vEffectPos, vEffectRotation, vEffectRotation );
						}
						else
						{
							if( true == pEffectSetInstance->m_bHyper &&
								false == pEffectData->m_EffectNameHyper.empty() )
							{
								pEffect = g_pX2Game->GetDamageEffect()->CreateInstance( pUnit, pEffectData->m_EffectNameHyper.c_str(), 
									fPowerRate, vEffectPos, vEffectRotation, vEffectRotation );
							}
							else
							{
								pEffect = g_pX2Game->GetDamageEffect()->CreateInstance( pUnit, pEffectData->m_EffectName.c_str(), 
									fPowerRate, vEffectPos, vEffectRotation, vEffectRotation );
							}
						}
#else
						if( true == pEffectSetInstance->m_bHyper &&
							false == pEffectData->m_EffectNameHyper.empty() )
						{
							pEffect = g_pX2Game->GetDamageEffect()->CreateInstance( pUnit, pEffectData->m_EffectNameHyper.c_str(), 
								fPowerRate, vEffectPos, vEffectRotation, vEffectRotation );
						}
						else
						{
							pEffect = g_pX2Game->GetDamageEffect()->CreateInstance( pUnit, pEffectData->m_EffectName.c_str(), 
								fPowerRate, vEffectPos, vEffectRotation, vEffectRotation );
						}
#endif
					}

					if( NULL != pEffect )
					{
						pEffect->SetScale( vEffectScale );
#ifdef RIDING_SYSTEM
						if ( NULL != pEffect->GetLockOnData() )
						{
							if ( NULL != pUnit && CX2GameUnit::GUT_USER == pUnit->GetGameUnitType() )
							{
								CX2GUUser* pUser = static_cast<CX2GUUser*>(pUnit);
								pUser->SetLockOn( pEffect );
							}
						}
#endif //RIDING_SYSTEM
					}

				} break;

			case ET_MESH_PLAYER_UI_MAJOR:
				{

					if( true == pEffectSetInstance->m_bHyper &&
						false == pEffectData->m_EffectNameHyper.empty() )
					{
						pMeshInstance = g_pData->GetUIMajorXMeshPlayer()->CreateInstance( NULL,  pEffectData->m_EffectNameHyper.c_str(), vEffectPos, 
							vEffectRotation, vEffectRotation );
					}
					else
					{
						pMeshInstance = g_pData->GetUIMajorXMeshPlayer()->CreateInstance( NULL,  pEffectData->m_EffectName.c_str(), vEffectPos, 
							vEffectRotation, vEffectRotation );
					}

					if( NULL != pMeshInstance )
					{
						pMeshInstance->SetScale( vEffectScale );

						if( true == pMeshInstance->GetIsSetLandHeightOnStart() )
						{
							if( NULL != g_pX2Game &&
								null != pEffectSetInstance->m_optrGameUnit )
							{
								pMeshInstance->SetLandPosition( pEffectSetInstance->m_optrGameUnit->GetLandPos().y );
							}
							else if( pEffectSetInstance->m_pSquareUnit != NULL )
							{
								pMeshInstance->SetLandPosition( pEffectSetInstance->m_pSquareUnit->GetLandPosition().y );
							}
#ifdef SERV_PET_SYSTEM
							else if( pEffectSetInstance->m_pPet != NULL )
							{
								pMeshInstance->SetLandPosition( pEffectSetInstance->m_pPet->GetLandPosition_LUA().y );
							}
#endif
						}
					}

				} break;
			case ET_MESH_PLAYER_UI_MINOR:
				{
					if( true == pEffectSetInstance->m_bHyper &&
						false == pEffectData->m_EffectNameHyper.empty() )
					{
						pMeshInstance = g_pData->GetUIMinorXMeshPlayer()->CreateInstance( NULL,  pEffectData->m_EffectNameHyper.c_str(), vEffectPos, 
							vEffectRotation, vEffectRotation );
					}
					else
					{
						pMeshInstance = g_pData->GetUIMinorXMeshPlayer()->CreateInstance( NULL,  pEffectData->m_EffectName.c_str(), vEffectPos, 
							vEffectRotation, vEffectRotation );
					}


					if( NULL != pMeshInstance )
					{
						pMeshInstance->SetScale( vEffectScale );

						if( true == pMeshInstance->GetIsSetLandHeightOnStart() )
						{
							if( NULL != g_pX2Game &&
								null != pEffectSetInstance->m_optrGameUnit )
							{
								pMeshInstance->SetLandPosition( pEffectSetInstance->m_optrGameUnit->GetLandPos().y );
							}
							else if( pEffectSetInstance->m_pSquareUnit != NULL )
							{
								pMeshInstance->SetLandPosition( pEffectSetInstance->m_pSquareUnit->GetLandPosition().y );
							}
#ifdef SERV_PET_SYSTEM
							else if( pEffectSetInstance->m_pPet != NULL )
							{
								pMeshInstance->SetLandPosition( pEffectSetInstance->m_pPet->GetLandPosition_LUA().y );
							}
#endif

						}
					}
				} break;
			case ET_MESH_PLAYER_GAME_MAJOR:
				{
					if ( NULL != g_pX2Game )
					{
						CX2GameUnit* pOwnerGameUnit = pEffectSetInstance->GetOwnerUnit();

						if( true == pEffectSetInstance->m_bHyper &&
							false == pEffectData->m_EffectNameHyper.empty() )
						{
							pMeshInstance = g_pX2Game->GetMajorXMeshPlayer()->CreateInstance( static_cast<CKTDGObject*>( pOwnerGameUnit ),  pEffectData->m_EffectNameHyper.c_str(), vEffectPos, 
								vEffectRotation, vEffectRotation );
						}
						else
						{
							pMeshInstance = g_pX2Game->GetMajorXMeshPlayer()->CreateInstance( static_cast<CKTDGObject*>( pOwnerGameUnit ),  pEffectData->m_EffectName.c_str(), vEffectPos, 
								vEffectRotation, vEffectRotation );
						}

						if( NULL != pMeshInstance )
						{
							pMeshInstance->SetScale( vEffectScale );

							if( true == pMeshInstance->GetIsSetLandHeightOnStart() )
							{
								if( NULL != pOwnerGameUnit )
								{
									pMeshInstance->SetLandPosition( pOwnerGameUnit->GetLandPos().y );
								}
							}
						}
					}
				} break;

			case ET_MESH_PLAYER_GAME_MINOR:
				{
					if ( NULL != g_pX2Game )
					{
						CX2GameUnit* pOwnerGameUnit = pEffectSetInstance->GetOwnerUnit();

						if( true == pEffectSetInstance->m_bHyper &&
							false == pEffectData->m_EffectNameHyper.empty() )
						{
							pMeshInstance = g_pX2Game->GetMinorXMeshPlayer()->CreateInstance( static_cast<CKTDGObject*>( pOwnerGameUnit ),  pEffectData->m_EffectNameHyper.c_str(), vEffectPos, 
								vEffectRotation, vEffectRotation );
						}
						else
						{
							pMeshInstance = g_pX2Game->GetMinorXMeshPlayer()->CreateInstance( static_cast<CKTDGObject*>( pOwnerGameUnit ),  pEffectData->m_EffectName.c_str(), vEffectPos, 
								vEffectRotation, vEffectRotation );
						}

						if( NULL != pMeshInstance )
						{
							pMeshInstance->SetScale( vEffectScale );

							if( true == pMeshInstance->GetIsSetLandHeightOnStart() )
							{
								if( NULL != pOwnerGameUnit )
								{
									pMeshInstance->SetLandPosition( pOwnerGameUnit->GetLandPos().y );
								}
							}
						}
					}
				
				} break;

			case ET_PARTICLE_UI_MAJOR:
				{
					if( true == pEffectSetInstance->m_bHyper &&
						false == pEffectData->m_EffectNameHyper.empty() )
					{
						pParticleSequence = g_pData->GetUIMajorParticle()->CreateSequence( NULL,  pEffectData->m_EffectNameHyper.c_str(), vEffectPos );
					}
					else
					{
						pParticleSequence = g_pData->GetUIMajorParticle()->CreateSequence( NULL,  pEffectData->m_EffectName.c_str(), vEffectPos );
					}

					if( pParticleSequence != NULL )
					{
						if( D3DXVECTOR3(0, 0, 0) != vEffectRotation )
						{
							pParticleSequence->SetAxisAngle( vEffectRotation );
							pParticleSequence->SetAddRotate( vEffectRotation );
						}

						pParticleSequence->SetScaleFactor( vEffectScale );

#ifdef FIX_ICE_HEATER_EVENT
						/// 파티클에 대하여 Set Over UI 기능 추가
						if ( true == pEffectData->m_bSetOverUI )
							pParticleSequence->SetOverUI( true );
#endif // FIX_ICE_HEATER_EVENT

						if( pParticleSequence->GetUseLand() == true )
						{
							if( NULL != g_pX2Game &&
								null != pEffectSetInstance->m_optrGameUnit )
							{
								pParticleSequence->SetLandPosition( pEffectSetInstance->m_optrGameUnit->GetLandPos().y );
							}
							else if( pEffectSetInstance->m_pSquareUnit != NULL )
							{
								pParticleSequence->SetLandPosition( pEffectSetInstance->m_pSquareUnit->GetLandPosition().y );
							}
#ifdef SERV_PET_SYSTEM
							else if( pEffectSetInstance->m_pPet != NULL )
							{
								pParticleSequence->SetLandPosition( pEffectSetInstance->m_pPet->GetLandPosition_LUA().y );
							}
#endif
						}						
					}
				} break;

			case ET_PARTICLE_UI_MINOR:
				{
					if( true == pEffectSetInstance->m_bHyper &&
						false == pEffectData->m_EffectNameHyper.empty() )
					{
						pParticleSequence = g_pData->GetUIMinorParticle()->CreateSequence( NULL,  pEffectData->m_EffectNameHyper.c_str(), vEffectPos );
					}
					else
					{
						pParticleSequence = g_pData->GetUIMinorParticle()->CreateSequence( NULL,  pEffectData->m_EffectName.c_str(), vEffectPos );
					}


					if( pParticleSequence != NULL )
					{
						if( D3DXVECTOR3(0, 0, 0) != vEffectRotation )
						{
							pParticleSequence->SetAxisAngle( vEffectRotation );
							pParticleSequence->SetAddRotate( vEffectRotation );
						}
						
						//if( true == pEffectData->m_bApplyInstanceScale )
						//{
						//	pParticleSequence->SetScaleFactor( pEffectSetInstance->m_vScale );
						//}
						// 위에 스케일은 effectset에서 지정한 scale을 적용시키지 않는다. 
						// 구현이유는 모름.. 통일성있게 수정함.. 						
						pParticleSequence->SetScaleFactor( vEffectScale ); // modified by wonpok. 2010.03.02

#ifdef FIX_ICE_HEATER_EVENT
						/// 파티클에 대하여 Set Over UI 기능 추가
						if ( true == pEffectData->m_bSetOverUI )
							pParticleSequence->SetOverUI( true );
#endif // FIX_ICE_HEATER_EVENT

						if( pParticleSequence->GetUseLand() == true )
						{
							if( NULL != g_pX2Game &&
								null != pEffectSetInstance->m_optrGameUnit )
							{
								pParticleSequence->SetLandPosition( pEffectSetInstance->m_optrGameUnit->GetLandPos().y );
							}
							else if( pEffectSetInstance->m_pSquareUnit != NULL )
							{
								pParticleSequence->SetLandPosition( pEffectSetInstance->m_pSquareUnit->GetLandPosition().y );
							}
#ifdef SERV_PET_SYSTEM
							else if( pEffectSetInstance->m_pPet != NULL )
							{
								pParticleSequence->SetLandPosition( pEffectSetInstance->m_pPet->GetLandPosition_LUA().y );
							}
#endif
						}						
					}
				} break;

			case ET_PARTICLE_GAME_MAJOR:
				{
					if ( NULL != g_pX2Game )
					{
						CX2GameUnit* pOwnerGameUnit = pEffectSetInstance->GetOwnerUnit();

						if( true == pEffectSetInstance->m_bHyper &&
							false == pEffectData->m_EffectNameHyper.empty() )
						{
							pParticleSequence = g_pX2Game->GetMajorParticle()->CreateSequence( static_cast<CKTDGObject*>( pOwnerGameUnit ),  pEffectData->m_EffectNameHyper.c_str(), vEffectPos );
						}
						else
						{
							pParticleSequence = g_pX2Game->GetMajorParticle()->CreateSequence( static_cast<CKTDGObject*>( pOwnerGameUnit ),  pEffectData->m_EffectName.c_str(), vEffectPos );
						}

						if ( pParticleSequence != NULL )
						{
							if( D3DXVECTOR3(0, 0, 0) != vEffectRotation )
							{
								pParticleSequence->SetAxisAngle( vEffectRotation );
								pParticleSequence->SetAddRotate( vEffectRotation );
							}

							pParticleSequence->SetScaleFactor( vEffectScale );

#ifdef FIX_ICE_HEATER_EVENT
							/// 파티클에 대하여 Set Over UI 기능 추가
							if ( true == pEffectData->m_bSetOverUI )
								pParticleSequence->SetOverUI( true );
#endif // FIX_ICE_HEATER_EVENT

							if( pParticleSequence->GetUseLand() == true )
							{
								if( NULL != pOwnerGameUnit )
								{
									pParticleSequence->SetLandPosition( pOwnerGameUnit->GetLandPos().y );
								}
							}
						}
					}

				} break;

			case ET_PARTICLE_GAME_MINOR:
				{
					if ( NULL != g_pX2Game )
					{
						CX2GameUnit* pOwnerGameUnit = pEffectSetInstance->GetOwnerUnit();

						if( true == pEffectSetInstance->m_bHyper &&
							false == pEffectData->m_EffectNameHyper.empty() )
						{
							pParticleSequence = g_pX2Game->GetMinorParticle()->CreateSequence( static_cast<CKTDGObject*>( pOwnerGameUnit ),  pEffectData->m_EffectNameHyper.c_str(), vEffectPos );
						}
						else
						{
							pParticleSequence = g_pX2Game->GetMinorParticle()->CreateSequence( static_cast<CKTDGObject*>( pOwnerGameUnit ),  pEffectData->m_EffectName.c_str(), vEffectPos );
						}

						if( pParticleSequence != NULL )
						{
							if( D3DXVECTOR3(0, 0, 0) != vEffectRotation )
							{
								pParticleSequence->SetAxisAngle( vEffectRotation );
								pParticleSequence->SetAddRotate( vEffectRotation );
							}

							pParticleSequence->SetScaleFactor( vEffectScale );

#ifdef FIX_ICE_HEATER_EVENT
							/// 파티클에 대하여 Set Over UI 기능 추가
							if ( true == pEffectData->m_bSetOverUI )
								pParticleSequence->SetOverUI( true );
#endif // FIX_ICE_HEATER_EVENT

							if( pParticleSequence->GetUseLand() == true )
							{
								if( NULL != pOwnerGameUnit )
								{
									pParticleSequence->SetLandPosition( pOwnerGameUnit->GetLandPos().y );
								}
							}
						}
					}					
				} break;
			}


			switch( pEffectData->m_eEffectType )
			{
			case ET_DAMAGE_EFFECT:
				{
					if( NULL != pEffect )
					{
						EffectInstance* pEffectInstance = new EffectInstance;
						pEffectInstance->m_pEffectData			= pEffectData;
						pEffectInstance->m_pDamageEffect		= pEffect;
						pEffectInstance->m_hMeshPlayer			= INVALID_MESH_INSTANCE_HANDLE;
						pEffectInstance->m_hParticleSequence	= INVALID_PARTICLE_HANDLE;
						pEffectInstance->m_optrGameUnit			= pEffectSetInstance->m_optrGameUnit;

						pEffectSetInstance->m_vecpEffectInstance.push_back( pEffectInstance );
#ifdef MODIFY_EFFECT_SET_TRACE_BONE // 오현빈
						if( true == pEffectData->m_bBoneTraceTargetMesh && 
							INVALID_MESH_INSTANCE_HANDLE == pEffectSetInstance->m_hTargetMesh )
						{
							pEffectSetInstance->m_hTargetMesh = pEffect->GetMainEffectHandle();
							pEffectSetInstance->m_eTargetMeshEffectType = ET_MESH_PLAYER_GAME_MAJOR;
							pEffectSetInstance->m_bIsTargetMesh = true;
		#ifdef SERV_ARA_CHANGE_CLASS_SECOND // 김태환
							if ( null != pEffectSetInstance->m_optrGameUnit )
							{ 
								/// 유닛의 IsRight 값으로 타겟 매시의 방향을 설정
								if ( true == pEffectSetInstance->m_optrGameUnit->GetIsRight() )
									pEffectSetInstance->m_bIsTargetMeshRight = true;

					#ifdef FIX_TARGET_MESH_OFFSET_POS // 김태환
								/// 유닛의 방향 벡터 값으로 타겟 매시의 방향 벡터를 설정
								pEffectSetInstance->m_vecTargetMeshDirVector	= pEffectSetInstance->m_optrGameUnit->GetDirVector();
								/// 유닛의 Z 벡터 값으로 타겟 매시의 Z 벡터를 설정
								pEffectSetInstance->m_vecTargetMeshZVector		= pEffectSetInstance->m_optrGameUnit->GetZVector();
					#endif // FIX_TARGET_MESH_OFFSET_POS
							}
		#endif // SERV_ARA_CHANGE_CLASS_SECOND
						}
#endif // MODIFY_EFFECT_SET_TRACE_BONE
					}
				} break;

			case ET_MESH_PLAYER_UI_MAJOR:
			case ET_MESH_PLAYER_UI_MINOR:
			case ET_MESH_PLAYER_GAME_MAJOR:
			case ET_MESH_PLAYER_GAME_MINOR:
				{
					if( NULL != pMeshInstance )
					{
						EffectInstance* pEffectInstance = new EffectInstance;
						pEffectInstance->m_pEffectData			= pEffectData;
						pEffectInstance->m_pDamageEffect		= NULL;
						pEffectInstance->m_hMeshPlayer			= pMeshInstance->GetHandle();
						pEffectInstance->m_hParticleSequence	= INVALID_PARTICLE_HANDLE;
						pEffectInstance->m_optrGameUnit			= pEffectSetInstance->m_optrGameUnit;

						pEffectSetInstance->m_vecpEffectInstance.push_back( pEffectInstance );

						if ( pEffectData->m_bUseSubAttackListSet && null != pEffectSetInstance->m_optrGameUnit )
							pEffectSetInstance->m_optrGameUnit->InsertPairSubAttackListSet( pEffectInstance->m_hMeshPlayer, pMeshInstance->GetXSkinAnim() );
					
#ifdef _IN_HOUSE_
						//{{AFX
						if( true == pEffectInstance->m_pEffectData->m_bApplyUnitRotation &&
							CKTDGMatrix::BT_NONE != pMeshInstance->GetTempletData()->billBoardType )
						{
							WCHAR wszMsg[1024] = L"";
							StringCchPrintfW( wszMsg, ARRAY_SIZE(wszMsg), L"%s, %s, %s, %d", 
								pEffectSetInstance->m_pEffectSetData->m_wstrEffectSetName.c_str(), 
								pEffectInstance->m_pEffectData->m_EffectName.c_str(),
								pMeshInstance->GetTempletData()->templetName.c_str(),
								(int) pMeshInstance->GetTempletData()->billBoardType );
							StateLog( wszMsg );

						}
						//}}AFX
#endif _IN_HOUSE_
#ifdef MODIFY_EFFECT_SET_TRACE_BONE // 오현빈
						if( true == pEffectData->m_bBoneTraceTargetMesh && 
							INVALID_MESH_INSTANCE_HANDLE == pEffectSetInstance->m_hTargetMesh )
						{
							pEffectSetInstance->m_hTargetMesh = pMeshInstance->GetHandle();
							pEffectSetInstance->m_eTargetMeshEffectType = pEffectData->m_eEffectType;
							pEffectSetInstance->m_bIsTargetMesh = true;
		#ifdef SERV_ARA_CHANGE_CLASS_SECOND // 김태환
							/// 유닛의 IsRight 값으로 타겟 매시의 방향을 설정
							if ( null != pEffectSetInstance->m_optrGameUnit )
							{
								if ( true == pEffectSetInstance->m_optrGameUnit->GetIsRight() )
									pEffectSetInstance->m_bIsTargetMeshRight = true;

				#ifdef FIX_TARGET_MESH_OFFSET_POS // 김태환
								/// 유닛의 방향 벡터 값으로 타겟 매시의 방향 벡터를 설정
								pEffectSetInstance->m_vecTargetMeshDirVector	= pEffectSetInstance->m_optrGameUnit->GetDirVector();
								/// 유닛의 Z 벡터 값으로 타겟 매시의 Z 벡터를 설정
								pEffectSetInstance->m_vecTargetMeshZVector		= pEffectSetInstance->m_optrGameUnit->GetZVector();
				#endif // FIX_TARGET_MESH_OFFSET_POS
							}
		#endif // SERV_ARA_CHANGE_CLASS_SECOND
						}
#endif // MODIFY_EFFECT_SET_TRACE_BONE


					}
				} break;

			case ET_PARTICLE_UI_MAJOR:
			case ET_PARTICLE_UI_MINOR:
			case ET_PARTICLE_GAME_MAJOR:
			case ET_PARTICLE_GAME_MINOR:
				{
					if( NULL != pParticleSequence )
					{
						EffectInstance* pEffectInstance = new EffectInstance;
						pEffectInstance->m_pEffectData			= pEffectData;
						pEffectInstance->m_pDamageEffect		= NULL;
						pEffectInstance->m_hMeshPlayer			= INVALID_MESH_INSTANCE_HANDLE;
						pEffectInstance->m_hParticleSequence	= pParticleSequence->GetHandle();
						pEffectInstance->m_optrGameUnit			= pEffectSetInstance->m_optrGameUnit;

						pEffectSetInstance->m_vecpEffectInstance.push_back( pEffectInstance );
					}
				} break;
			}
		}
	}
}

void CX2EffectSet::OnFrameMove_DeleteEffect()
{
	KTDXPROFILE();

	// 이펙트 삭제
	for( UINT j=0; j<m_vecpEffectSetInstance.size(); j++ )
	{
		EffectSetInstance* pEffectSetInstance = m_vecpEffectSetInstance[j];
		
#ifdef MODIFY_EFFECT_SET_TRACE_BONE
		if( true == pEffectSetInstance->m_bIsTargetMesh &&
			INVALID_MESH_INSTANCE_HANDLE == pEffectSetInstance->m_hTargetMesh )
		{
			m_vecpEffectSetInstance.erase( m_vecpEffectSetInstance.begin() + j );
			--j;

			SAFE_DELETE( pEffectSetInstance );
			continue;
		}
#endif //MODIFY_EFFECT_SET_TRACE_BONE

#ifdef TRACE_UNIT_DIE_DELETE_EFFECTSET
		if( true == pEffectSetInstance->m_bCreateEffectSetTraceUnit &&
			true == pEffectSetInstance->m_bTraceUnitDieDeleteEffectSet &&
			( null == pEffectSetInstance->m_optrTraceTargetGameUnit || 
			0 >= pEffectSetInstance->m_optrTraceTargetGameUnit->GetNowHp() ))
		{
			m_vecpEffectSetInstance.erase( m_vecpEffectSetInstance.begin() + j );
			--j;

			SAFE_DELETE( pEffectSetInstance );
			continue;
		}
#endif //TRACE_UNIT_DIE_DELETE_EFFECTSET

		// game에 없는 user의 포인터를 가지고 있다면 지운다
		if( ESM_GAME == pEffectSetInstance->m_eMode )
		{
			if( NULL != g_pX2Game && null == pEffectSetInstance->m_optrGameUnit )
			{
				m_vecpEffectSetInstance.erase( m_vecpEffectSetInstance.begin() + j );
				--j;

				SAFE_DELETE( pEffectSetInstance );
				continue;
			}
		}

#ifdef SERV_PET_SYSTEM
		if( pEffectSetInstance->m_pPet != NULL )
		{
			if( g_pData->GetPetManager()->IsValidPet(pEffectSetInstance->m_pPet) == false )
			{
				m_vecpEffectSetInstance.erase( m_vecpEffectSetInstance.begin() + j );
				--j;

				SAFE_DELETE( pEffectSetInstance );
				continue;
			}
		}
#endif

		if( pEffectSetInstance->m_pEffectSetData->m_iDeleteShakeCount > 0 )
		{
			if( pEffectSetInstance->m_iCurrDeleteShakeCount >= pEffectSetInstance->m_pEffectSetData->m_iDeleteShakeCount )
			{
				m_vecpEffectSetInstance.erase( m_vecpEffectSetInstance.begin() + j );
				--j;

				SAFE_DELETE( pEffectSetInstance );
				continue;
			}
			else
			{
				if( null != pEffectSetInstance->m_optrTraceTargetGameUnit && 
					NULL != g_pX2Game )
				{

					switch( pEffectSetInstance->m_optrTraceTargetGameUnit->GetGameUnitType() )
					{
					case CX2GameUnit::GUT_USER:
						{
							CX2GUUser* pCX2GUUser = static_cast<CX2GUUser*>( pEffectSetInstance->m_optrTraceTargetGameUnit.GetObservable() );
							if( pCX2GUUser != NULL )
							{
								if( pCX2GUUser->GetIsRight() != pCX2GUUser->GetIsRightBefore() )
								{
									++pEffectSetInstance->m_iCurrDeleteShakeCount;
								}
							}

						} break;

					case CX2GameUnit::GUT_NPC:
						{

							//CX2GUNPC* pNPC = g_pX2Game->GetNPCUnitByUID( (int)m_HitterUID );
							//if( pNPC != NULL )
							//{
							//	if( pNPC->GetIsRight() != pNPC->GetIsRightBefore() )
							//	{
							//		m_iFollowHitter--;
							//	}
							//}

						} break;
					}
				}
			}
		}

		if( pEffectSetInstance->m_fLifeTime > 0.f )
		{
			if( pEffectSetInstance->m_fElapsedTime > pEffectSetInstance->m_fLifeTime )
			{
				m_vecpEffectSetInstance.erase( m_vecpEffectSetInstance.begin() + j );
				--j;

				SAFE_DELETE( pEffectSetInstance );
				continue;
			}
		}

		bool bEffectSetInstanceAlive = false;
		for( UINT i=0; i<pEffectSetInstance->m_vecEffectInstanceCreated.size(); i++ )
		{
			if( false == pEffectSetInstance->m_vecEffectInstanceCreated[i] )
			{
				bEffectSetInstanceAlive = true;
				break;
			}
		}

		for( UINT i=0; i<pEffectSetInstance->m_vecpEffectInstance.size(); i++ )
		{
			EffectInstance* pEffectInstance = pEffectSetInstance->m_vecpEffectInstance[i];
			if( false == pEffectInstance->m_bAlive )
				continue;

			EffectData* pEffectData = pEffectInstance->m_pEffectData;
			switch( pEffectData->m_eEffectType )
			{
			default:
				{
					ASSERT( !"no type effect" );
					pEffectInstance->m_bAlive = false;
				} break;
			case ET_DAMAGE_EFFECT:
				{
					if( NULL != g_pX2Game && false == g_pX2Game->GetDamageEffect()->IsLiveInstance( pEffectInstance->m_pDamageEffect ) )
					{
						pEffectInstance->m_bAlive = false;
						pEffectInstance->m_pDamageEffect = NULL;
					}
				} break;

			case ET_MESH_PLAYER_UI_MAJOR:
				{
					if( false == g_pData->GetUIMajorXMeshPlayer()->IsLiveInstanceHandle( pEffectInstance->m_hMeshPlayer ) )
					{
						pEffectInstance->m_bAlive = false;
						pEffectInstance->m_hMeshPlayer = INVALID_MESH_INSTANCE_HANDLE;
					}
				} break;
			case ET_MESH_PLAYER_UI_MINOR:
				{
					if( false == g_pData->GetUIMinorXMeshPlayer()->IsLiveInstanceHandle( pEffectInstance->m_hMeshPlayer ) )
					{
						pEffectInstance->m_bAlive = false;
						pEffectInstance->m_hMeshPlayer = INVALID_MESH_INSTANCE_HANDLE;
					}
				} break;
			case ET_MESH_PLAYER_GAME_MAJOR:
				{
					if( NULL != g_pX2Game && false == g_pX2Game->GetMajorXMeshPlayer()->IsLiveInstanceHandle( pEffectInstance->m_hMeshPlayer ) )
					{
						pEffectInstance->m_bAlive = false;
						pEffectInstance->m_hMeshPlayer = INVALID_MESH_INSTANCE_HANDLE;
					}
				} break;
			case ET_MESH_PLAYER_GAME_MINOR:
				{
					if( NULL != g_pX2Game && false == g_pX2Game->GetMinorXMeshPlayer()->IsLiveInstanceHandle( pEffectInstance->m_hMeshPlayer ) )
					{
						pEffectInstance->m_bAlive = false;
						pEffectInstance->m_hMeshPlayer = INVALID_MESH_INSTANCE_HANDLE;
					}
				} break;
			case ET_PARTICLE_UI_MAJOR:
				{
					if( false == g_pData->GetUIMajorParticle()->IsLiveInstanceHandle( pEffectInstance->m_hParticleSequence ) )
					{
						pEffectInstance->m_bAlive = false;
						pEffectInstance->m_hParticleSequence = INVALID_PARTICLE_HANDLE;
					}
				} break;
			case ET_PARTICLE_UI_MINOR:
				{
					if( false == g_pData->GetUIMinorParticle()->IsLiveInstanceHandle( pEffectInstance->m_hParticleSequence ) )
					{
						pEffectInstance->m_bAlive = false;
						pEffectInstance->m_hParticleSequence = INVALID_PARTICLE_HANDLE;
					}
				} break;
			case ET_PARTICLE_GAME_MAJOR:
				{
					if( NULL != g_pX2Game && false == g_pX2Game->GetMajorParticle()->IsLiveInstanceHandle( pEffectInstance->m_hParticleSequence ) )
					{
						pEffectInstance->m_bAlive = false;
						pEffectInstance->m_hParticleSequence = INVALID_PARTICLE_HANDLE;
					}
				} break;
			case ET_PARTICLE_GAME_MINOR:
				{
					if( NULL != g_pX2Game && false == g_pX2Game->GetMinorParticle()->IsLiveInstanceHandle( pEffectInstance->m_hParticleSequence ) )
					{
						pEffectInstance->m_bAlive = false;
						pEffectInstance->m_hParticleSequence = INVALID_PARTICLE_HANDLE;
					}
				} break;
			}

			if( true == pEffectInstance->m_bAlive &&
				false == pEffectData->m_bPassiveEffect )
			{
				bEffectSetInstanceAlive = true;
			}
		}


		if( false == bEffectSetInstanceAlive )
		{
			m_vecpEffectSetInstance.erase( m_vecpEffectSetInstance.begin() + j );
			--j;

			SAFE_DELETE( pEffectSetInstance );
			continue;
		}
	}
}



void CX2EffectSet::OnFrameMove_UpdateEffect()
{

	KTDXPROFILE();

	// 이펙트 위치 갱신	
	BOOST_TEST_FOREACH( EffectSetInstance*, pEffectSetInstance, m_vecpEffectSetInstance )
	{
		for( UINT i=0; i<pEffectSetInstance->m_vecpEffectInstance.size(); i++ )
		{
			EffectInstance* pEffectInstance = pEffectSetInstance->m_vecpEffectInstance[i];
			EffectData* pEffectData = pEffectInstance->m_pEffectData;

			if( false == pEffectData->m_bTrace 
				&& false == pEffectData->m_bTraceMore 
				//{{kimhc // 2011-01-17 // 지정한 Bone의 메트릭스 값을 Trace 함 (chung 코드 참고)
#ifdef	TRACE_MAXTRIX_TEST
				&& false == pEffectData->m_bTraceMatrix
#endif	TRACE_MAXTRIX_TEST
				//}}kimhc // 2011-01-17 // 지정한 Bone의 메트릭스 값을 Trace 함 (chung 코드 참고)
				)
				continue;

			if( false == pEffectInstance->m_bAlive )
				continue;


			D3DXVECTOR3 vEffectPos = pEffectSetInstance->GetEffectPosition( pEffectData );

#ifdef MODIFY_EFFECT_SET_TRACE_BONE
			// 타겟 메시가 사라지면 더이상 위치 갱신 하지 않기
			if( true == pEffectSetInstance->m_bIsTargetMesh &&
				INVALID_MESH_INSTANCE_HANDLE == pEffectSetInstance->m_hTargetMesh )
				break;
#endif // MODIFY_EFFECT_SET_TRACE_BONE

			switch( pEffectData->m_eEffectType )
			{
			case ET_DAMAGE_EFFECT:
				{
					if ( NULL != g_pX2Game &&
						NULL != g_pX2Game->GetDamageEffect() &&
						NULL != pEffectInstance->m_pDamageEffect )
					{
						pEffectInstance->m_pDamageEffect->SetPos( vEffectPos );
					}

				} break;

			case ET_MESH_PLAYER_UI_MAJOR:				
				{
					if( NULL != g_pData->GetUIMajorXMeshPlayer() &&
						INVALID_MESH_INSTANCE_HANDLE != pEffectInstance->m_hMeshPlayer )
					{
						CKTDGXMeshPlayer::CXMeshInstance* pInstance = g_pData->GetUIMajorXMeshPlayer()->GetMeshInstance(pEffectInstance->m_hMeshPlayer);
						if( NULL != pInstance )
						{
							pInstance->SetPos( vEffectPos );
						}
					}
				} break;
			case ET_MESH_PLAYER_UI_MINOR:
				{
					if( NULL != g_pData->GetUIMinorXMeshPlayer() &&
						INVALID_MESH_INSTANCE_HANDLE != pEffectInstance->m_hMeshPlayer )
					{
						CKTDGXMeshPlayer::CXMeshInstance* pInstance = g_pData->GetUIMinorXMeshPlayer()->GetMeshInstance(pEffectInstance->m_hMeshPlayer);
						if( NULL != pInstance )
						{
							pInstance->SetPos( vEffectPos );
						}
					}
				} break;
			case ET_MESH_PLAYER_GAME_MAJOR:
				{
					if( g_pX2Game != NULL && g_pX2Game->GetMajorXMeshPlayer() != NULL && pEffectInstance->m_hMeshPlayer != INVALID_MESH_INSTANCE_HANDLE )						
					{
						CKTDGXMeshPlayer::CXMeshInstance* pInstance = g_pX2Game->GetMajorXMeshPlayer()->GetMeshInstance(pEffectInstance->m_hMeshPlayer);
						if( pInstance != NULL )
						{
							pInstance->SetPos(vEffectPos);
						}
					}
				}
				break;
			case ET_MESH_PLAYER_GAME_MINOR:
				{
					if( g_pX2Game != NULL && g_pX2Game->GetMinorXMeshPlayer() != NULL && pEffectInstance->m_hMeshPlayer != INVALID_MESH_INSTANCE_HANDLE )						
					{
						CKTDGXMeshPlayer::CXMeshInstance* pInstance = g_pX2Game->GetMinorXMeshPlayer()->GetMeshInstance(pEffectInstance->m_hMeshPlayer);
						if( pInstance != NULL )
						{
							pInstance->SetPos(vEffectPos);
						}
					}
				} break;
			case ET_PARTICLE_UI_MAJOR:
				{
					if( g_pData->GetUIMajorParticle() != NULL && 
						pEffectInstance->m_hParticleSequence != INVALID_PARTICLE_HANDLE )						
					{
						CKTDGParticleSystem::CParticleEventSequence* pSeq = g_pData->GetUIMajorParticle()->GetInstanceSequence( pEffectInstance->m_hParticleSequence );
						if( pSeq != NULL )
						{
							pSeq->SetPosition( vEffectPos, pEffectData->m_bTraceMore, pEffectData->m_bTraceMore );
						}
					}
				} break;

			case ET_PARTICLE_UI_MINOR:
				{
					if( g_pData->GetUIMinorParticle() != NULL && 
						pEffectInstance->m_hParticleSequence != INVALID_PARTICLE_HANDLE )						
					{
						CKTDGParticleSystem::CParticleEventSequence* pSeq = g_pData->GetUIMinorParticle()->GetInstanceSequence( pEffectInstance->m_hParticleSequence );
						if( pSeq != NULL )
						{
							pSeq->SetPosition( vEffectPos, pEffectData->m_bTraceMore, pEffectData->m_bTraceMore );
						}
					}
				} break;

			case ET_PARTICLE_GAME_MAJOR:
				{
					if( g_pX2Game != NULL && g_pX2Game->GetMajorParticle() != NULL && pEffectInstance->m_hParticleSequence != INVALID_PARTICLE_HANDLE )						
					{
						CKTDGParticleSystem::CParticleEventSequence* pSeq = g_pX2Game->GetMajorParticle()->GetInstanceSequence( pEffectInstance->m_hParticleSequence );
						if( pSeq != NULL )
						{
							pSeq->SetPosition( vEffectPos, pEffectData->m_bTraceMore, pEffectData->m_bTraceMore );
						}
					}
				}
				break;

			case ET_PARTICLE_GAME_MINOR:
				{
					if( g_pX2Game != NULL && g_pX2Game->GetMinorParticle() != NULL && pEffectInstance->m_hParticleSequence != INVALID_PARTICLE_HANDLE )						
					{
						CKTDGParticleSystem::CParticleEventSequence* pSeq = g_pX2Game->GetMinorParticle()->GetInstanceSequence( pEffectInstance->m_hParticleSequence );
						if( pSeq != NULL )
						{
							pSeq->SetPosition( vEffectPos, pEffectData->m_bTraceMore, pEffectData->m_bTraceMore );
						}
					}						
				}
				break;
			}			

			//{{kimhc // 2011-01-17 // 지정한 Bone의 메트릭스 값을 Trace 함 (chung 코드 참고)
#ifdef	TRACE_MAXTRIX_TEST
			if ( true == pEffectData->m_bTraceMatrix )
			{
				const D3DXMATRIX* pCombineMatrix = pEffectSetInstance->GetCombineMatrix( pEffectData );

				if ( NULL != pCombineMatrix )
				{
					switch( pEffectData->m_eEffectType )
					{
					case ET_DAMAGE_EFFECT:
						{
							if( NULL != g_pX2Game &&
								NULL != g_pX2Game->GetDamageEffect() &&
								NULL != pEffectInstance->m_pDamageEffect )
							{
								CKTDGXMeshPlayer::CXMeshInstance* pInstance = pEffectInstance->m_pDamageEffect->GetMainEffect();
								if( NULL != pInstance )
								{
									pInstance->SetUseDXMatrix( *pCombineMatrix );						
								}
							}
						}
						break;

					case ET_MESH_PLAYER_UI_MAJOR:				
						{
							if( NULL != g_pData->GetUIMajorXMeshPlayer() &&
								INVALID_MESH_INSTANCE_HANDLE != pEffectInstance->m_hMeshPlayer )
							{
								CKTDGXMeshPlayer::CXMeshInstance* pInstance = g_pData->GetUIMajorXMeshPlayer()->GetMeshInstance(pEffectInstance->m_hMeshPlayer);
								if( NULL != pInstance )
								{
									pInstance->SetUseDXMatrix( *pCombineMatrix );						
								}
							}
						} break;
					case ET_MESH_PLAYER_UI_MINOR:
						{
							if( NULL != g_pData->GetUIMinorXMeshPlayer() &&
								INVALID_MESH_INSTANCE_HANDLE != pEffectInstance->m_hMeshPlayer )
							{
								CKTDGXMeshPlayer::CXMeshInstance* pInstance = g_pData->GetUIMinorXMeshPlayer()->GetMeshInstance(pEffectInstance->m_hMeshPlayer);
								if( NULL != pInstance )
								{
									pInstance->SetUseDXMatrix( *pCombineMatrix );						
								}
							}
						} break;
					case ET_MESH_PLAYER_GAME_MAJOR:
						{
							if( g_pX2Game != NULL && g_pX2Game->GetMajorXMeshPlayer() != NULL && pEffectInstance->m_hMeshPlayer != INVALID_MESH_INSTANCE_HANDLE )						
							{
								CKTDGXMeshPlayer::CXMeshInstance* pInstance = g_pX2Game->GetMajorXMeshPlayer()->GetMeshInstance(pEffectInstance->m_hMeshPlayer);
								if( pInstance != NULL )
								{
									pInstance->SetUseDXMatrix( *pCombineMatrix );						
								}
							}
						}
						break;
					case ET_MESH_PLAYER_GAME_MINOR:
						{
							if( g_pX2Game != NULL && g_pX2Game->GetMinorXMeshPlayer() != NULL && pEffectInstance->m_hMeshPlayer != INVALID_MESH_INSTANCE_HANDLE )						
							{
								CKTDGXMeshPlayer::CXMeshInstance* pInstance = g_pX2Game->GetMinorXMeshPlayer()->GetMeshInstance(pEffectInstance->m_hMeshPlayer);
								if( pInstance != NULL )
								{
									pInstance->SetUseDXMatrix( *pCombineMatrix );						
								}
							}
						} break;
#ifdef NEW_HENIR_TEST
					case ET_PARTICLE_GAME_MAJOR:
						{
							if( g_pX2Game != NULL && g_pX2Game->GetMajorParticle() != NULL && pEffectInstance->m_hParticleSequence != INVALID_PARTICLE_HANDLE )						
							{
								CKTDGParticleSystem::CParticleEventSequence* pSeq = g_pX2Game->GetMajorParticle()->GetInstanceSequence( pEffectInstance->m_hParticleSequence );
								if( pSeq != NULL )
								{
									pSeq->SetUseDXMatrix( *pCombineMatrix );
								}
							}
						} break;
#endif NEW_HENIR_TEST

					default:
						break;
					}
				} // if
			} // if
#endif	TRACE_MAXTRIX_TEST
			//}}kimhc // 2011-01-17 // 지정한 Bone의 메트릭스 값을 Trace 함 (chung 코드 참고)

#ifdef ARA_CHANGE_CLASS_FIRST
			if ( true == pEffectData->m_bReverseY )
			{
				switch( pEffectData->m_eEffectType )
				{
				case ET_PARTICLE_GAME_MAJOR:
					{
						if( g_pX2Game != NULL && g_pX2Game->GetMajorParticle() != NULL && pEffectInstance->m_hParticleSequence != INVALID_PARTICLE_HANDLE )						
						{
							CKTDGParticleSystem::CParticleEventSequence* pSeq = g_pX2Game->GetMajorParticle()->GetInstanceSequence( pEffectInstance->m_hParticleSequence );

				#ifdef SERV_ARA_CHANGE_CLASS_SECOND // 김태환
							if( pSeq != NULL )
							{
								CKTDGMatrix::BILLBOARD_TYPE eBillboardType = pSeq->GetBillBoardType();

								if ( CKTDGMatrix::BT_NONE != eBillboardType )
								{
									/// 타겟 매시가 설정되어 있다면, 타겟 매시의 IsRight 값을 사용한다.
									if ( INVALID_MESH_INSTANCE_HANDLE != pEffectSetInstance->m_hTargetMesh )
										pSeq->SetReverseY( !( pEffectSetInstance->m_bIsTargetMeshRight ) );
									/// 타겟 매시가 설정되어 있지 않다면, 유저의 IsRight 값을 사용한다.
									else if ( null != pEffectInstance->m_optrGameUnit )
										pSeq->SetReverseY( !( pEffectInstance->m_optrGameUnit->GetIsRight() ) );
								}
							}
				#else // SERV_ARA_CHANGE_CLASS_SECOND
							if( pSeq != NULL && null != pEffectInstance->m_optrGameUnit && false == pEffectInstance->m_optrGameUnit->GetIsRight() )
							{
								CKTDGMatrix::BILLBOARD_TYPE eBillboardType = pSeq->GetBillBoardType();
								
								if ( CKTDGMatrix::BT_NONE != eBillboardType )
								{
									pSeq->SetReverseY( true );
								}
							}
				#endif // SERV_ARA_CHANGE_CLASS_SECOND
						}
					} break;
				}
			}
#endif
		}
	}
}

void CX2EffectSet::OnFrameMove_StartCameraShake()
{
	KTDXPROFILE();
	
	if ( NULL == g_pX2Game )
		return;

	//camera shake
	BOOST_TEST_FOREACH( EffectSetInstance*, pEffectSetInstance, m_vecpEffectSetInstance )
	{
		EffectSetData* pEffectSetData = pEffectSetInstance->m_pEffectSetData;

		for ( UINT i=0; i<pEffectSetData->m_vecpCameraShakeData.size(); i++ )
		{
			if( true == pEffectSetInstance->m_vecCameraShakeDone[i] )
				continue;

			CameraShakeData* pCameraShakeData = pEffectSetData->m_vecpCameraShakeData[i];


			if( 0.f != pCameraShakeData->m_fStartAnimTime + pEffectSetInstance->m_fDelayTime ) // 0초에 시작이면 무조건 바로 생성
			{
#ifdef EFFECT_TOOL						
				if( pEffectSetInstance->m_fElapsedTime < pCameraShakeData->m_fStartAnimTime + pEffectSetInstance->m_fDelayTime )
				{
					continue;
				}
#else
				switch( pEffectSetInstance->m_pEffectSetData->m_eEventTimerType )
				{
				default:
				case ESTT_ANIM:
					{
#ifdef SERV_PET_SYSTEM						
						if( pEffectSetInstance->m_pPet != NULL )
						{
							if( pEffectSetInstance->m_pPet->GetXSkinAnim() != NULL && 
								pEffectSetInstance->m_pPet->GetXSkinAnim()->GetNowAnimationTime() < pCameraShakeData->m_fStartAnimTime + pEffectSetInstance->m_fDelayTime )
								continue;
						}
#ifdef RIDING_SYSTEM
						else if ( NULL != pEffectSetInstance->GetOwnerUnit() && NULL != pEffectSetInstance->GetOwnerUnit()->GetRidingPet() )
						{
							if( pEffectSetInstance->GetOwnerUnit()->GetRidingPet()->GetXSkinAnimPtr() != NULL &&
								pEffectSetInstance->GetOwnerUnit()->GetRidingPet()->GetXSkinAnimPtr()->GetNowAnimationTime() < pCameraShakeData->m_fStartAnimTime + pEffectSetInstance->m_fDelayTime )
								continue;
						}
#endif //RIDING_SYSTEM
						else
						{
							if( pEffectSetInstance->m_pXSkinAnim->GetNowAnimationTime() < pCameraShakeData->m_fStartAnimTime + pEffectSetInstance->m_fDelayTime )
								continue;
						}
#else
						//if( false == pEffectSetInstance->m_pXSkinAnim->EventTimer( pCameraShakeData->m_fStartAnimTime + pEffectSetInstance->m_fDelayTime ) )
						if( pEffectSetInstance->m_pXSkinAnim->GetNowAnimationTime() < pCameraShakeData->m_fStartAnimTime + pEffectSetInstance->m_fDelayTime )
							continue;
#endif						
					} break;

				case ESTT_CUSTOM:
					{
						//if( pCameraShakeData->m_fStartAnimTime + pEffectSetInstance->m_fDelayTime <= pEffectSetInstance->m_fPrevElapsedTime ||
						//	pCameraShakeData->m_fStartAnimTime + pEffectSetInstance->m_fDelayTime > pEffectSetInstance->m_fElapsedTime )
						if( pEffectSetInstance->m_fElapsedTime < pCameraShakeData->m_fStartAnimTime + pEffectSetInstance->m_fDelayTime )
						{
							continue;
						}
					} break;
				}
#endif //EFFECT_TOOL
			}


			pEffectSetInstance->m_vecCameraShakeDone[i] = true;

#ifdef FIX_OBSERVER_MODE_CRASH
			if ( NULL != g_pX2Game->GetMyUnit() && GetDistance3Sq( g_pX2Game->GetMyUnit()->GetPos(), pEffectSetInstance->m_vPosition ) < pCameraShakeData->m_iLimitDistanceSq )
#else  FIX_OBSERVER_MODE_CRASH
			if ( GetDistance3Sq( g_pX2Game->GetMyUnit()->GetPos(), pEffectSetInstance->m_vPosition ) < pCameraShakeData->m_iLimitDistanceSq )
#endif FIX_OBSERVER_MODE_CRASH
			{
				switch( pCameraShakeData->m_eShakeType )
				{
				case CKTDGCamera::DECT_UP:
					{
						g_pX2Game->GetX2Camera()->GetCamera()->UpCrashCamera( pCameraShakeData->m_fOneDirSpeed, pCameraShakeData->m_fOneDirAccel );
					} break;
				case CKTDGCamera::DECT_DOWN:
					{
						g_pX2Game->GetX2Camera()->GetCamera()->DownCrashCamera( pCameraShakeData->m_fOneDirSpeed, pCameraShakeData->m_fOneDirAccel );
					} break;
				case CKTDGCamera::DECT_UP_DOWN:
					{
						g_pX2Game->GetX2Camera()->GetCamera()->ShakeUpDown( pCameraShakeData->m_fTwoDirGap, pCameraShakeData->m_fTwoDirTime, pCameraShakeData->m_fTwoDirTimeGap );
					} break;
				case CKTDGCamera::DECT_UP_DOWN_NO_RESET:
					{
						g_pX2Game->GetX2Camera()->GetCamera()->UpDownCrashCameraNoReset( pCameraShakeData->m_fTwoDirGap, pCameraShakeData->m_fTwoDirTime );
					} break;

				case CKTDGCamera::DECT_LEFT:
					{
						g_pX2Game->GetX2Camera()->GetCamera()->LeftCrashCamera( pCameraShakeData->m_fOneDirSpeed, pCameraShakeData->m_fOneDirAccel );
					} break;

				case CKTDGCamera::DECT_RIGHT:
					{
						g_pX2Game->GetX2Camera()->GetCamera()->RightCrashCamera( pCameraShakeData->m_fOneDirSpeed, pCameraShakeData->m_fOneDirAccel );
					} break;

				case CKTDGCamera::DECT_LEFT_RIGHT:
					{
						g_pX2Game->GetX2Camera()->GetCamera()->ShakeLeftRight( pCameraShakeData->m_fTwoDirGap, pCameraShakeData->m_fTwoDirTime, pCameraShakeData->m_fTwoDirTimeGap );
					} break;
				case CKTDGCamera::DECT_LEFT_RIGHT_NO_RESET:
					{
						g_pX2Game->GetX2Camera()->GetCamera()->LeftRightCrashCameraNoReset( pCameraShakeData->m_fTwoDirGap, pCameraShakeData->m_fTwoDirTime );
					} break;



				case CKTDGCamera::DECT_RANDOM:
					{
						g_pX2Game->GetX2Camera()->GetCamera()->ShakeRandom( pCameraShakeData->m_iEyeShakeCount, pCameraShakeData->m_EyeMoveSpeed.m_Min, pCameraShakeData->m_EyeMoveSpeed.m_Max, 
							pCameraShakeData->m_EyeMoveRange.m_Min, pCameraShakeData->m_EyeMoveRange.m_Max, 
							pCameraShakeData->m_iLookAtShakeCount, pCameraShakeData->m_LookAtMoveSpeed.m_Min, pCameraShakeData->m_LookAtMoveSpeed.m_Max, 
							pCameraShakeData->m_LookAtMoveRange.m_Min, pCameraShakeData->m_LookAtMoveRange.m_Max, 
							pCameraShakeData->m_bFixedLookVec );
					} break;
				}
			}
		}

	}
}

CX2EffectSet::Handle CX2EffectSet::PlayEffectSet( const wstring& wstrEffectSetName, CX2GameUnit* pGameUnit, CX2GameUnit* pTraceTargetGameUnit /*= NULL*/, 
												 bool bHyper /*= false*/, float fPowerRate /*= -1.f*/, float fLifeTime /*= -1.f */, 
												 D3DXVECTOR3 vScale /*= D3DXVECTOR3( 1, 1, 1 )*/, 
												 bool bCustomPosition /*= false*/, const D3DXVECTOR3& vPosition/* = D3DXVECTOR3(0, 0, 0)*/, 
												 const D3DXVECTOR3& vRotationDegree /*= D3DXVECTOR3(0, 0, 0)*/, const D3DXVECTOR3& vDirVector /*= D3DXVECTOR3(0, 0, 0)*/ )
{
	EffectSetData* pEffectSetData = GetEffectSetTemplet( wstrEffectSetName );
	if( NULL == pEffectSetData )
		return CX2EffectSet::INVALID_HANDLE;

	if( NULL == pGameUnit )
		return CX2EffectSet::INVALID_HANDLE;



	CKTDGXSkinAnimPtr skinAnimPtr;
	switch( pGameUnit->GetGameUnitType() )
	{
	case CX2GameUnit::GUT_USER:
		{
			CX2GUUser* pUser = (CX2GUUser*) pGameUnit;
			skinAnimPtr = pUser->GetXSkinAnimPtr();
		} break;

	case CX2GameUnit::GUT_NPC:
		{
			CX2GUNPC* pNPC = (CX2GUNPC*) pGameUnit;
			skinAnimPtr = pNPC->GetXSkinAnimPtr();
		} break;
	}

	if( skinAnimPtr == NULL )
		return CX2EffectSet::INVALID_HANDLE;


#ifdef SERV_PET_SYSTEM
		EffectSetInstance* pEffectSetInstance = new EffectSetInstance( pEffectSetData, skinAnimPtr, pGameUnit, pTraceTargetGameUnit, CX2SquareUnitPtr(), NULL );
#else
	EffectSetInstance* pEffectSetInstance = new EffectSetInstance( pEffectSetData, skinAnimPtr, pGameUnit, pTraceTargetGameUnit, CX2SquareUnitPtr() );
#endif
	
	if( CX2EffectSet::INVALID_HANDLE == pEffectSetInstance->m_hHandle )
	{
		SAFE_DELETE( pEffectSetInstance );
		return CX2EffectSet::INVALID_HANDLE;
	}

#ifdef EFFECT_USE_LOG
	if( NULL != g_pMain->GetGameEdit() &&
		CX2GameEdit::ELL_EFFECT_LOG_1 <= g_pMain->GetGameEdit()->GetEffectLogLevel() )
	{
		dbg::clog << L"(이펙트셋)" << wstrEffectSetName << dbg::endl;
		KLOG("EffectLog.txt")  << L"EffectSet : " << wstrEffectSetName.c_str() << fileout;
	}
#endif // EFFECT_USE_LOG

	pEffectSetInstance->m_bHyper = bHyper;
	pEffectSetInstance->m_fPowerRate = fPowerRate;
	pEffectSetInstance->m_fLifeTime = fLifeTime;
	pEffectSetInstance->m_vScale = vScale;

#ifdef GIANT_UNIT_GIANT_EFFECT_TEST

	if( true == pEffectSetData->m_bAffectedByUnitScale )
	{
		if( NULL != pGameUnit )
		{
			if( pGameUnit->GetScaleByUnit() != D3DXVECTOR3( 1, 1, 1 ) )
			{
				pEffectSetInstance->m_vScale.x *= pGameUnit->GetScaleByUnit().x;
				pEffectSetInstance->m_vScale.y *= pGameUnit->GetScaleByUnit().y;
				pEffectSetInstance->m_vScale.z *= pGameUnit->GetScaleByUnit().z;
			}
		}
	}

#endif GIANT_UNIT_GIANT_EFFECT_TEST


	pEffectSetInstance->m_bCustomPosition = bCustomPosition;
	if( true == bCustomPosition )
	{
		pEffectSetInstance->m_vPosition			= vPosition;
		pEffectSetInstance->m_vRotationDegree	= vRotationDegree;
		pEffectSetInstance->m_vDirVector		= vDirVector;
		pEffectSetInstance->m_vZVector			= CX2EffectSet::GetZVector( vDirVector );
	}
	else
		pEffectSetInstance->m_vPosition = pGameUnit->GetPos();	// 커스텀포지션이 아닌 경우 이펙트를 발생 시킨 유저의 위치를 넣어줌
	
	m_vecpEffectSetInstance.push_back( pEffectSetInstance );	
	return pEffectSetInstance->m_hHandle;
}


CX2EffectSet::Handle CX2EffectSet::PlayEffectSet( const wstring& wstrEffectSetName, CX2SquareUnitPtr pSquareUnit )
{
	EffectSetData* pEffectSetData = GetEffectSetTemplet( wstrEffectSetName );
	if( NULL == pEffectSetData )
		return CX2EffectSet::INVALID_HANDLE;

	if( pSquareUnit == NULL )
		return CX2EffectSet::INVALID_HANDLE;

	if( pSquareUnit->GetUnitViewer() == NULL )
		return CX2EffectSet::INVALID_HANDLE;



	CKTDGXSkinAnimPtr skinAnimPtr = pSquareUnit->GetUnitViewer()->GetXSkinAnimPtr();
	if( skinAnimPtr == NULL )
		return CX2EffectSet::INVALID_HANDLE;



#ifdef SERV_PET_SYSTEM
	EffectSetInstance* pEffectSetInstance = new EffectSetInstance( pEffectSetData, skinAnimPtr, NULL, NULL, pSquareUnit, NULL );
#else
	EffectSetInstance* pEffectSetInstance = new EffectSetInstance( pEffectSetData, skinAnimPtr, NULL, NULL, pSquareUnit );
#endif

	if( CX2EffectSet::INVALID_HANDLE == pEffectSetInstance->m_hHandle )
	{
		SAFE_DELETE( pEffectSetInstance );
		return CX2EffectSet::INVALID_HANDLE;
	}
#ifdef EFFECT_USE_LOG
	if( NULL != g_pMain->GetGameEdit() &&
		CX2GameEdit::ELL_EFFECT_LOG_1 <= g_pMain->GetGameEdit()->GetEffectLogLevel() )
	{
		dbg::clog << L"(이펙트셋)" << wstrEffectSetName << dbg::endl;
		KLOG("EffectLog.txt")  << L"EffectSet : " << wstrEffectSetName.c_str() << fileout;
	}
#endif // EFFECT_USE_LOG

	m_vecpEffectSetInstance.push_back( pEffectSetInstance );	
	return pEffectSetInstance->m_hHandle;
}
CX2EffectSet::Handle CX2EffectSet::PlayEffectSetCustomPos( const wstring& wstrEffectSetName, CX2SquareUnitPtr pSquareUnit, const D3DXVECTOR3& vPos )
{
	CX2EffectSet::Handle hEffectset = PlayEffectSet(wstrEffectSetName, pSquareUnit );
	if( CX2EffectSet::INVALID_HANDLE != hEffectset )
	{
		EffectSetInstance* pEffect = g_pData->GetGameEffectSet()->GetEffectSetInstance( hEffectset );
		if( NULL != hEffectset)
			pEffect->SetEffectPosition( vPos );
	}

	return hEffectset;
}
#ifdef SERV_PET_SYSTEM
CX2EffectSet::Handle CX2EffectSet::PlayEffectSetByPet_LUA( const char* strEffectSetName, CX2PET* pPet )
{
	wstring wstrEffectSetName;
	ConvertUtf8ToWCHAR( wstrEffectSetName, strEffectSetName );

	return PlayEffectSetByPet( wstrEffectSetName, pPet );
}
CX2EffectSet::Handle CX2EffectSet::PlayEffectSetByPet( const wstring& wstrEffectSetName, CX2PET* pPet )
{
	EffectSetData* pEffectSetData = GetEffectSetTemplet( wstrEffectSetName );
	if( NULL == pEffectSetData )
		return CX2EffectSet::INVALID_HANDLE;

	if( pPet == NULL )
		return CX2EffectSet::INVALID_HANDLE;

	CKTDGXSkinAnim* skinAnim = pPet->GetXSkinAnim();
	if( skinAnim == NULL )
		return CX2EffectSet::INVALID_HANDLE;

	CKTDGXSkinAnimPtr tempPtr;

	EffectSetInstance* pEffectSetInstance = new EffectSetInstance( pEffectSetData, tempPtr, NULL, NULL, CX2SquareUnitPtr(), pPet );
	if( CX2EffectSet::INVALID_HANDLE == pEffectSetInstance->m_hHandle )
	{
		SAFE_DELETE( pEffectSetInstance );
		return CX2EffectSet::INVALID_HANDLE;
	}

	pEffectSetInstance->m_bHyper = false;
	pEffectSetInstance->m_fPowerRate = -1.f;
	pEffectSetInstance->m_fLifeTime = -1.f;
	pEffectSetInstance->m_vScale = D3DXVECTOR3( 1.f, 1.f, 1.f );

	m_vecpEffectSetInstance.push_back( pEffectSetInstance );	
	return pEffectSetInstance->m_hHandle;
}
#endif




void CX2EffectSet::StopEffectSet( CX2EffectSet::Handle& hHandle )
{

	if( CX2EffectSet::INVALID_HANDLE == hHandle )
		return;


	for( UINT i=0; i<m_vecpEffectSetInstance.size(); i++ )
	{
		EffectSetInstance* pEffectSetInstance = m_vecpEffectSetInstance[i];
		if( NULL != pEffectSetInstance )
		{
			if( pEffectSetInstance->m_hHandle == hHandle )
			{
				SAFE_DELETE( pEffectSetInstance );

				m_vecpEffectSetInstance.erase( m_vecpEffectSetInstance.begin() + i );
				break;
			}
		}
	}

	hHandle = CX2EffectSet::INVALID_HANDLE;
}

void CX2EffectSet::StopEffectSetAll()
{
	for ( UINT i = 0; i < m_vecpEffectSetInstance.size(); i++ )
	{
		EffectSetInstance* pEffectSetInstance = m_vecpEffectSetInstance[i];
		
		SAFE_DELETE( pEffectSetInstance );
	}

	m_vecpEffectSetInstance.resize( 0 );
}






CX2EffectSet::EffectSetInstance* CX2EffectSet::GetEffectSetInstance( CX2EffectSet::Handle hHandle )
{
	if( CX2EffectSet::INVALID_HANDLE == hHandle )
		return NULL;

	return s_HandleManager.GetObjectByHandle( hHandle );
}









CX2EffectSet::EffectSetData* CX2EffectSet::GetEffectSetTemplet( const wstring& wstrEffectSetName )
{
	if( true == wstrEffectSetName.empty() )
		return NULL;

	EffectSetDataMap::iterator it = m_mapEffectSetTemplet.find( wstrEffectSetName );
	if( it != m_mapEffectSetTemplet.end() )
	{
		return it->second;
	}

	return NULL;
}



void CX2EffectSet::RegisterLuaBind()
{
	lua_tinker::decl( g_pKTDXApp->GetLuaBinder()->GetLuaState(),  "g_pEffectSet",		this );	
}


bool CX2EffectSet::OpenScriptFile( const wstring& wstrFileName )
{
	RegisterLuaBind();

	if( true == wstrFileName.empty() )
		return false;

	return g_pKTDXApp->GetDeviceManager()->LoadLuaTinker( wstrFileName.c_str() );
}

bool CX2EffectSet::AddEffectSetTemplet_LUA()
{
	KLuaManager luaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState() );
	TableBind( &luaManager, g_pKTDXApp->GetLuaBinder() );


	EffectSetData* pEffectSetData = new EffectSetData;
	LUA_GET_VALUE( luaManager,		"NAME",			pEffectSetData->m_wstrEffectSetName,			L""		);

	ASSERT( false == pEffectSetData->m_wstrEffectSetName.empty() );
	if( true == pEffectSetData->m_wstrEffectSetName.empty() )
		goto effect_set_error_proc;


	ASSERT( NULL == GetEffectSetTemplet( pEffectSetData->m_wstrEffectSetName ) );
	if( NULL != GetEffectSetTemplet( pEffectSetData->m_wstrEffectSetName ) )
		goto effect_set_error_proc;



	LUA_GET_VALUE_ENUM( luaManager,		"EVENT_TIMER_TYPE",			pEffectSetData->m_eEventTimerType,			EFFECT_SET_TIMER_TYPE,	ESTT_ANIM );
	LUA_GET_VALUE( luaManager,		"AFFECTED_BY_UNIT_SCALE",		pEffectSetData->m_bAffectedByUnitScale,		true );

	//LUA_GET_VALUE( luaManager,		"DELETE_WHEN_TRACE_TARGET_DEAD",	pEffectSetData->m_bDeleteWhenTraceTargetDead,				false	);
	LUA_GET_VALUE( luaManager,		"DELETE_SHAKE_COUNT",				pEffectSetData->m_iDeleteShakeCount,						-1	);

	
	
	// damageeffect, meshplayer, particle 
	int iEffectTableIndex = 0;
	while( true == luaManager.BeginTable( L"EFFECT", iEffectTableIndex ) )
	{
		EffectData* pEffectData = new EffectData;

		// 공통 스크립트
		LUA_GET_VALUE_ENUM( luaManager, "EFFECT_TYPE",				pEffectData->m_eEffectType,					EFFECT_TYPE,	ET_INVALID		);
		if( ET_INVALID == pEffectData->m_eEffectType )
			goto effect_error_proc;

		LUA_GET_VALUE( luaManager,		"NAME",						pEffectData->m_EffectName,					L""		);
		ASSERT( false == pEffectData->m_EffectName.empty() );
		if( true == pEffectData->m_EffectName.empty() )
			goto effect_error_proc;

		LUA_GET_VALUE( luaManager,		"HYPER_NAME",				pEffectData->m_EffectNameHyper,				L""		);



		LUA_GET_VALUE( luaManager,		"START_TIME",				pEffectData->m_fStartAnimTime,				0.f		);
		LUA_GET_VALUE( luaManager,		"BONE_NAME",				pEffectData->m_PositionBoneName,			L""		);

		LUA_GET_VALUE( luaManager,		"WEAPON_BONE_NAME",			pEffectData->m_PositionWeaponBoneName,		L""		);
		LUA_GET_VALUE( luaManager,		"WEAPON_BONE_WEAPON_INDEX",	pEffectData->m_iWeaponIndex,				0		);
		LUA_GET_VALUE( luaManager,		"WEAPON_BONE_MODEL_INDEX",	pEffectData->m_iModelIndex,					0		);
		
#ifdef RIDING_SYSTEM
		LUA_GET_VALUE( luaManager,		"RIDING_PET_BONE_NAME",		pEffectData->m_PosRidingPetBoneName,		L""		);
#endif //RIDING_SYSTEM

		LUA_GET_VALUE( luaManager,		"POSITION_OFFSET_X",		pEffectData->m_vPositionOffset.x,			0.f		);
		LUA_GET_VALUE( luaManager,		"POSITION_OFFSET_Y",		pEffectData->m_vPositionOffset.y,			0.f		);
		LUA_GET_VALUE( luaManager,		"POSITION_OFFSET_Z",		pEffectData->m_vPositionOffset.z,			0.f		);
		
		LUA_GET_VALUE( luaManager,		"TRACE",					pEffectData->m_bTrace,						false	);
		LUA_GET_VALUE( luaManager,		"TRACE_MORE",				pEffectData->m_bTraceMore,					false	);

		//{{kimhc // 2011-01-17 // 지정한 Bone의 메트릭스 값을 Trace 함 (chung 코드 참고)
#ifdef	TRACE_MAXTRIX_TEST
		LUA_GET_VALUE( luaManager,		"TRACE_MATRIX",				pEffectData->m_bTraceMatrix,					false	);
#endif	TRACE_MAXTRIX_TEST
		//}}kimhc // 2011-01-17 // 지정한 Bone의 메트릭스 값을 Trace 함 (chung 코드 참고)

#ifdef ARA_CHANGE_CLASS_FIRST
		LUA_GET_VALUE( luaManager,		"REVERSE_Y",				pEffectData->m_bReverseY,					false	);
#endif
				

		LUA_GET_VALUE( luaManager,		"USE_LAND_POSITION",		pEffectData->m_bLandPosition,				false	);
		LUA_GET_VALUE( luaManager,		"ROTATE_X",					pEffectData->m_vRotateRel.x,				0.f		);
		LUA_GET_VALUE( luaManager,		"ROTATE_Y",					pEffectData->m_vRotateRel.y,				0.f		);
		LUA_GET_VALUE( luaManager,		"ROTATE_Z",					pEffectData->m_vRotateRel.z,				0.f		);
		LUA_GET_VALUE( luaManager,		"APPLY_UNIT_ROTATION",		pEffectData->m_bApplyUnitRotation,			true	);

#ifdef SERV_ARA_CHANGE_CLASS_SECOND // 김태환
		/// 회전값 설정시 좌 우 완벽한 대칭을 이루기 위해, 방향에 따른 x회전 값 반전 기능 추가
		LUA_GET_VALUE( luaManager,		"APPLY_UNIT_ROTATION_MIRROR",	pEffectData->m_bApplyUnitRotationMirror, false );

		/// 위의 기능이 설정되어 있다면, APPLY_UNIT_ROTATION은 해제
		if ( true == pEffectData->m_bApplyUnitRotationMirror )
			pEffectData->m_bApplyUnitRotation = false;
#endif // SERV_ARA_CHANGE_CLASS_SECOND


		LUA_GET_VALUE( luaManager,		"APPLY_INSTANCE_SCALE",		pEffectData->m_bApplyInstanceScale,			true	);	// 주의!! templet의 scale을 적용할지 말지 정하는 것이 아니라, instance에 지정된 scale을 적용할지 말지 결정하는 변수 입니다

		LUA_GET_VALUE( luaManager,		"SCALE_X",					pEffectData->m_vScale.x,					0.f		);
		LUA_GET_VALUE( luaManager,		"SCALE_Y",					pEffectData->m_vScale.y,					0.f		);
		LUA_GET_VALUE( luaManager,		"SCALE_Z",					pEffectData->m_vScale.z,					0.f		);


		LUA_GET_VALUE( luaManager,		"SCALE_X_ON_LEFT",			pEffectData->m_vScaleOnLeft.x,				0.f		);
		LUA_GET_VALUE( luaManager,		"SCALE_Y_ON_LEFT",			pEffectData->m_vScaleOnLeft.y,				0.f		);
		LUA_GET_VALUE( luaManager,		"SCALE_Z_ON_LEFT",			pEffectData->m_vScaleOnLeft.z,				0.f		);



	
		LUA_GET_VALUE( luaManager,		"START_WHEN_TRACE_TARGET_DEAD",		pEffectData->m_bStartWhenTraceTargetDead,		false	);
		LUA_GET_VALUE( luaManager,		"PASSIVE_EFFECT",		pEffectData->m_bPassiveEffect,		false	);

		LUA_GET_VALUE( luaManager,		"USE_SUB_ATTACK_LIST_SET",		pEffectData->m_bUseSubAttackListSet,		false	);

#ifdef FIX_ICE_HEATER_EVENT
		/// 파티클에 대하여 Set Over UI 기능 추가
		LUA_GET_VALUE( luaManager,		"SET_OVER_UI",					pEffectData->m_bSetOverUI,					false	);
#endif FIX_ICE_HEATER_EVENT

#ifdef MODIFY_EFFECT_SET_TRACE_BONE // 오현빈
		LUA_GET_VALUE( luaManager,		"BONE_TRACE_TARGET_MESH",		pEffectData->m_bBoneTraceTargetMesh,		false	);
#endif // MODIFY_EFFECT_SET_TRACE_BONE

#ifdef MODIFY_CREATE_EFFECT_SET_LIMIT_DISTANCE
		float fLimitDistance = -1.f;
		LUA_GET_VALUE( luaManager,		"LIMIT_DISTANCE",				fLimitDistance,				-1.f	);
		if( 0.f < fLimitDistance )
		{
			pEffectData->m_fLimitDistanceSq = fLimitDistance * fLimitDistance;
		}
#endif // MODIFY_CREATE_EFFECT_SET_LIMIT_DISTANCE

		// 이펙트 종류별 script
		switch( pEffectData->m_eEffectType )
		{
		case ET_DAMAGE_EFFECT:
			{
				ASSERT( !pEffectData->m_bUseSubAttackListSet );	/// MeshPlayer만 사용가능
#ifdef FIX_ICE_HEATER_EVENT 
				/// 파티클에 대하여 Set Over UI 기능 추가
				ASSERT( !pEffectData->m_bSetOverUI );			/// Particle만 사용가능
#endif // FIX_ICE_HEATER_EVENT

			} break;
		case ET_MESH_PLAYER_UI_MAJOR:
		case ET_MESH_PLAYER_UI_MINOR:
		case ET_MESH_PLAYER_GAME_MAJOR:
		case ET_MESH_PLAYER_GAME_MINOR:
			{
#ifdef FIX_ICE_HEATER_EVENT 
				/// 파티클에 대하여 Set Over UI 기능 추가
				ASSERT( !pEffectData->m_bSetOverUI );			/// Particle만 사용가능
#endif // FIX_ICE_HEATER_EVENT
			} break;
		case ET_PARTICLE_UI_MAJOR:
		case ET_PARTICLE_UI_MINOR:
		case ET_PARTICLE_GAME_MAJOR:
		case ET_PARTICLE_GAME_MINOR:
			{
				ASSERT( !pEffectData->m_bUseSubAttackListSet );	/// MeshPlayer만 사용가능
#ifdef MODIFY_EFFECT_SET_TRACE_BONE
				ASSERT( !pEffectData->m_bBoneTraceTargetMesh );
#endif //MODIFY_EFFECT_SET_TRACE_BONE
			} break;
		}
		

		pEffectSetData->m_vecpEffectData.push_back( pEffectData );
		luaManager.EndTable();		// EFFECT(iEffectTableIndex)
		++iEffectTableIndex;
		continue;

effect_error_proc:
		SAFE_DELETE( pEffectData );
		luaManager.EndTable();		// EFFECT(iEffectTableIndex)
		++iEffectTableIndex;
	}





	// camera shake
	int iCameraShakeTableIndex = 0;
	while( true == luaManager.BeginTable( L"CAMERA_SHAKE", iCameraShakeTableIndex ) )
	{
		CameraShakeData* pCameraShakeData = new CameraShakeData;

		LUA_GET_VALUE_ENUM( luaManager, "SHAKE_TYPE",				pCameraShakeData->m_eShakeType,				CKTDGCamera::SHAKE_TYPE,	CKTDGCamera::DECT_INVALID	);
		if( CKTDGCamera::DECT_INVALID == pCameraShakeData->m_eShakeType )
			goto camera_shake_error_proc;

		LUA_GET_VALUE( luaManager,		"START_TIME",				pCameraShakeData->m_fStartAnimTime,				0.f		);

		/// 카메라 크래시 발생 거리제한
		int iLimitDistance = 0;
		LUA_GET_VALUE( luaManager,		"LIMIT_DISTANCE",			iLimitDistance,			0		);
		if ( 0 != iLimitDistance )	// 입력 값이 있는 경우만 해당 값을 제곱하여 대입
			pCameraShakeData->m_iLimitDistanceSq = iLimitDistance * iLimitDistance;

		LUA_GET_VALUE( luaManager,		"ONE_DIR_SPEED",			pCameraShakeData->m_fOneDirSpeed,				0.f		);
		LUA_GET_VALUE( luaManager,		"ONE_DIR_ACCEL",			pCameraShakeData->m_fOneDirAccel,				0.f		);
		LUA_GET_VALUE( luaManager,		"TWO_DIR_GAP",				pCameraShakeData->m_fTwoDirGap,					0.f		);
		LUA_GET_VALUE( luaManager,		"TWO_DIR_TIME",				pCameraShakeData->m_fTwoDirTime,				0.f		);
		LUA_GET_VALUE( luaManager,		"TWO_DIR_TIME_GAP",			pCameraShakeData->m_fTwoDirTimeGap,				0.05f	);


		LUA_GET_VALUE( luaManager,		"FIXED_LOOK_VEC",			pCameraShakeData->m_bFixedLookVec,				false	);
		LUA_GET_VALUE( luaManager,		"EYE_SHAKE_COUNT",			pCameraShakeData->m_iEyeShakeCount,				0 );
		LUA_GET_VALUE( luaManager,		"EYE_MOVE_SPEED_MIN",		pCameraShakeData->m_EyeMoveSpeed.m_Min,			0.f );
		LUA_GET_VALUE( luaManager,		"EYE_MOVE_SPEED_MAX",		pCameraShakeData->m_EyeMoveSpeed.m_Max,			0.f );

		LUA_GET_VALUE( luaManager,		"EYE_MOVE_RANGE_MIN",		pCameraShakeData->m_EyeMoveRange.m_Min,			0.f );
		LUA_GET_VALUE( luaManager,		"EYE_MOVE_RANGE_MAX",		pCameraShakeData->m_EyeMoveRange.m_Max,			0.f );

		LUA_GET_VALUE( luaManager,		"LOOK_AT_SHAKE_COUNT",		pCameraShakeData->m_iLookAtShakeCount,			0 );
		LUA_GET_VALUE( luaManager,		"LOOK_AT_MOVE_SPEED_MIN",	pCameraShakeData->m_LookAtMoveSpeed.m_Min,		0.f	);
		LUA_GET_VALUE( luaManager,		"LOOK_AT_MOVE_SPEED_MAX",	pCameraShakeData->m_LookAtMoveSpeed.m_Max,		0.f	);

		LUA_GET_VALUE( luaManager,		"LOOK_AT_MOVE_RANGE_MIN",	pCameraShakeData->m_LookAtMoveRange.m_Min,		0.f	);
		LUA_GET_VALUE( luaManager,		"LOOK_AT_MOVE_RANGE_MAX",	pCameraShakeData->m_LookAtMoveRange.m_Max,		0.f	);


		pEffectSetData->m_vecpCameraShakeData.push_back( pCameraShakeData );
		luaManager.EndTable();		// EFFECT(iCameraShakeTableIndex)
		++iCameraShakeTableIndex;
		continue;

camera_shake_error_proc:
		SAFE_DELETE( pCameraShakeData );
		luaManager.EndTable();		// EFFECT(iCameraShakeTableIndex)
		++iCameraShakeTableIndex;
	}




	m_mapEffectSetTemplet[ pEffectSetData->m_wstrEffectSetName ] = pEffectSetData;

#ifdef EFFECT_TOOL
	m_vecEffectSetTemplet.push_back( pEffectSetData );
#endif //EFFECT_TOOL
	return true;

effect_set_error_proc:
	SAFE_DELETE( pEffectSetData );
	return false;
}






//////////////////////////////////////////////////////////////////////////
CX2EffectSet::EffectInstance::~EffectInstance()
{
	switch( m_pEffectData->m_eEffectType )
	{
	default:
		{
			ASSERT( !"no type effect" );
		} break;

	case ET_DAMAGE_EFFECT:
		{
			if( NULL != g_pX2Game &&
				NULL != g_pX2Game->GetDamageEffect() )
			{
				if( NULL != m_pDamageEffect )
				{
					g_pX2Game->GetDamageEffect()->DestroyInstance( m_pDamageEffect );
				}
			}
		} break;

	case ET_MESH_PLAYER_UI_MAJOR:
		{
			if( NULL != g_pData->GetUIMajorXMeshPlayer() )
			{
				if( INVALID_MESH_INSTANCE_HANDLE != m_hMeshPlayer )
				{
					if ( m_pEffectData->m_bUseSubAttackListSet && null != m_optrGameUnit )
						m_optrGameUnit->ErasePairSubAttackListSet( m_hMeshPlayer );

					g_pData->GetUIMajorXMeshPlayer()->DestroyInstance( m_hMeshPlayer );
				}
			}
		} break;

	case ET_MESH_PLAYER_UI_MINOR:
		{
			if( NULL != g_pData->GetUIMinorXMeshPlayer() )
			{
				if( INVALID_MESH_INSTANCE_HANDLE != m_hMeshPlayer )
				{
					if ( m_pEffectData->m_bUseSubAttackListSet && null != m_optrGameUnit )
						m_optrGameUnit->ErasePairSubAttackListSet( m_hMeshPlayer );

					g_pData->GetUIMinorXMeshPlayer()->DestroyInstance( m_hMeshPlayer );
				}
			}
		} break;

	case ET_MESH_PLAYER_GAME_MAJOR:
		{
			if( NULL != g_pX2Game &&
				NULL != g_pX2Game->GetMajorXMeshPlayer() )
			{
				if( INVALID_MESH_INSTANCE_HANDLE != m_hMeshPlayer )
				{
					if ( m_pEffectData->m_bUseSubAttackListSet && null != m_optrGameUnit )
						m_optrGameUnit->ErasePairSubAttackListSet( m_hMeshPlayer );

					g_pX2Game->GetMajorXMeshPlayer()->DestroyInstance( m_hMeshPlayer );
				}
			}
		} break;

	case ET_MESH_PLAYER_GAME_MINOR:
		{
			if( NULL != g_pX2Game &&
				NULL != g_pX2Game->GetMinorXMeshPlayer() )
			{
				if( INVALID_MESH_INSTANCE_HANDLE != m_hMeshPlayer )
				{
					if ( m_pEffectData->m_bUseSubAttackListSet && null != m_optrGameUnit )
						m_optrGameUnit->ErasePairSubAttackListSet( m_hMeshPlayer );

					g_pX2Game->GetMinorXMeshPlayer()->DestroyInstance( m_hMeshPlayer );
				}
			}
		} break;

	case ET_PARTICLE_UI_MAJOR:
		{
			if( NULL != g_pData->GetUIMajorParticle() )
			{
				if( INVALID_PARTICLE_HANDLE != m_hParticleSequence )
				{
					g_pData->GetUIMajorParticle()->DestroyInstanceHandle( m_hParticleSequence );
				}
			}
		} break;

	case ET_PARTICLE_UI_MINOR:
		{
			if( NULL != g_pData->GetUIMinorParticle() )
			{
				if( INVALID_PARTICLE_HANDLE != m_hParticleSequence )
				{
					g_pData->GetUIMinorParticle()->DestroyInstanceHandle( m_hParticleSequence );
				}
			}
		} break;

	case ET_PARTICLE_GAME_MAJOR:
		{
			if( NULL != g_pX2Game &&
				NULL != g_pX2Game->GetMajorParticle() )
			{
				if( INVALID_PARTICLE_HANDLE != m_hParticleSequence )
				{
					g_pX2Game->GetMajorParticle()->DestroyInstanceHandle( m_hParticleSequence );
				}
			}
		} break;

	case ET_PARTICLE_GAME_MINOR:
		{
			if( NULL != g_pX2Game &&
				NULL != g_pX2Game->GetMinorParticle() )
			{
				if( INVALID_PARTICLE_HANDLE != m_hParticleSequence )
				{
					g_pX2Game->GetMinorParticle()->DestroyInstanceHandle( m_hParticleSequence );
				}
			}
		} break;
	}
}



//////////////////////////////////////////////////////////////////////////
CX2EffectSet::EffectSetInstance::~EffectSetInstance()
{
	
	//if( NULL != g_pX2Game )
	//{
	//	g_pX2Game->GetMajorParticle()->DestroyInstanceHandle( m_hSeqStickShake );
	//}

	s_HandleManager.ReleaseHandle( m_hHandle );

	BOOST_TEST_FOREACH( EffectInstance*, pEffectInstance, m_vecpEffectInstance )
	{
		SAFE_DELETE( pEffectInstance );
	}
	m_vecpEffectInstance.clear();
}

#ifdef SERV_PET_SYSTEM
CX2EffectSet::EffectSetInstance::EffectSetInstance( EffectSetData* pEffectSetData, CKTDGXSkinAnimPtr pSkinAnim, CX2GameUnit* pGameUnit, CX2GameUnit* pTraceTargetGameUnit, CX2SquareUnitPtr pSquareUnit, CX2PET *pPet)
#else
CX2EffectSet::EffectSetInstance::EffectSetInstance( EffectSetData* pEffectSetData, CKTDGXSkinAnimPtr pSkinAnim, CX2GameUnit* pGameUnit, CX2GameUnit* pTraceTargetGameUnit, CX2SquareUnitPtr pSquareUnit )
#endif
: m_hHandle( CX2EffectSet::INVALID_HANDLE )
, m_pXSkinAnim( pSkinAnim )
, m_optrGameUnit( pGameUnit )
, m_optrTraceTargetGameUnit( pTraceTargetGameUnit )
, m_pSquareUnit( pSquareUnit )
, m_bHyper( false )
, m_bCustomPosition( false )
, m_vPosition( 0, 0, 0 )		
, m_vRotationDegree( 0, 0, 0 )
, m_vDirVector( 0, 0, 0 )
, m_vZVector( 0, 0, 0 )
, m_fDelayTime( 0.f )
, m_fPowerRate( 0.f )
, m_fPowerRateScale( 1.f )
, m_fLifeTime( 0.f )
, m_vScale( 1, 1, 1 )
, m_bDamageEffectScale( false )
, m_pEffectSetData( pEffectSetData ) 
, m_fPrevElapsedTime( 0.f )
, m_fElapsedTime( 0.f )
, m_bUseAnimationTiming( true )
, m_iCurrDeleteShakeCount( 0 )
#ifdef SERV_PET_SYSTEM
, m_pPet( pPet )
#endif
#ifdef ROTATE_EFFECTSET
, m_vLocalRotateDegree( D3DXVECTOR3(0.f, 0.f, 0.f) )
#endif
#ifdef MODIFY_EFFECT_SET_TRACE_BONE // 오현빈
, m_eTargetMeshEffectType(ET_INVALID)
, m_hTargetMesh ( INVALID_MESH_INSTANCE_HANDLE )
, m_bIsTargetMesh( false ) 
#endif // MODIFY_EFFECT_SET_TRACE_BONE
#ifdef SERV_ARA_CHANGE_CLASS_SECOND // 김태환
, m_bIsTargetMeshRight( false ) 
#endif // SERV_ARA_CHANGE_CLASS_SECOND
#ifdef FIX_TARGET_MESH_OFFSET_POS // 김태환
, m_vecTargetMeshDirVector( D3DXVECTOR3(1.f, 0.f, 0.f) )
, m_vecTargetMeshZVector( D3DXVECTOR3(1.f, 0.f, 0.f) )
#endif // FIX_TARGET_MESH_OFFSET_POS
#ifdef TRACE_UNIT_DIE_DELETE_EFFECTSET
, m_bCreateEffectSetTraceUnit( false )
, m_bTraceUnitDieDeleteEffectSet( true )
#endif //TRACE_UNIT_DIE_DELETE_EFFECTSET
//, m_hSeqStickShake( INVALID_PARTICLE_HANDLE )
{
//	if( NULL == pEffectSetData )
//		return;

// #ifdef SERV_PET_SYSTEM
// 	if( pPet == NULL && pSkinAnim == NULL )
// 		return;
// #else
// 	if( pSkinAnim == NULL )
// 		return;
// #endif

	if( NULL != pGameUnit )
	{
		m_eMode = ESM_GAME;
	}
	else if( pSquareUnit != NULL )
	{
		m_eMode = ESM_FIELD;
	}
#ifdef SERV_PET_SYSTEM
	else if( pPet != NULL )
	{
		m_eMode = ESM_COMMON;
	}
#endif
	else
	{
		m_eMode = ESM_FIELD;
		//return;
	}


	m_vecEffectInstanceCreated.resize(0);
	for( UINT i=0; i<pEffectSetData->m_vecpEffectData.size(); i++ )
	{
		m_vecEffectInstanceCreated.push_back( false );
	}

	m_vecCameraShakeDone.resize(0);
	for( UINT i=0; i<pEffectSetData->m_vecpCameraShakeData.size(); i++ )
	{
		m_vecCameraShakeDone.push_back( false );
	}

	m_hHandle = s_HandleManager.GetNextValidHandle();
	s_HandleManager.AssignHandle( m_hHandle, this );
}

#ifdef GET_EFFECTSET_POSITION_IN_LUA
D3DXVECTOR3 CX2EffectSet::GetEffectPosition_LUA( CX2EffectSet::Handle hHandle, int iEffectIndex )
{
	D3DXVECTOR3 vPos( 0, 0, 0 );

	EffectSetInstance* pEffectSetInstance = GetEffectSetInstance( hHandle );
	if( pEffectSetInstance == NULL )
		return vPos;

	EffectData* pEffectData = pEffectSetInstance->m_pEffectSetData->m_vecpEffectData[iEffectIndex];
	if( pEffectData == NULL )
		return vPos;

	vPos = pEffectSetInstance->GetEffectPosition( pEffectData );

	return vPos;
}
#endif GET_EFFECTSET_POSITION_IN_LUA


D3DXVECTOR3 CX2EffectSet::EffectSetInstance::GetEffectPosition( EffectData* pEffectData )
{
	KTDXPROFILE();

	D3DXVECTOR3 vPos(0, 0, 0);

	if( NULL == pEffectData )
		return vPos;

#ifdef MODIFY_EFFECT_SET_TRACE_BONE // 오현빈
	if( true == GetEffectPositionByTargetMesh( vPos, pEffectData ) )
	{
		SetEffectSetOffset( vPos, pEffectData->m_vPositionOffset );
		return vPos;
	}
#endif // MODIFY_EFFECT_SET_TRACE_BONE

	if( true == m_bCustomPosition )
	{
		vPos = m_vPosition;
		SetEffectSetOffset( vPos, pEffectData->m_vPositionOffset );
		return vPos;
	}

#ifdef EFFECT_TOOL
	SetEffectPoistion_EffectTool( pEffectData, vPos );
	return vPos;
#endif // EFFECT_TOOL


	switch( m_eMode )
	{
	case ESM_GAME:
		{
			if( NULL == g_pX2Game )
				return vPos;

			if( null != m_optrTraceTargetGameUnit )
			{
				if( false == pEffectData->m_PositionBoneName.empty() )
				{
					if( 0 == pEffectData->m_PositionBoneName.compare( L"_HEAD_BONE" ) )
					{
						vPos = m_optrTraceTargetGameUnit->GetHeadBonePos();
					}
					else
					{
						if( false == m_optrTraceTargetGameUnit->GetBonePos( pEffectData->m_PositionBoneName.c_str(), vPos ) )
						{// 본 얻기 실패
#ifdef ADD_RESOURCE_ERROR_LOG
							FindBoneErrorLog( pEffectData->m_EffectName.c_str() , pEffectData->m_PositionBoneName.c_str() );
#endif // ADD_RESOURCE_ERROR_LOG
						}
					}
				}
				else if( false == pEffectData->m_PositionWeaponBoneName.empty() )
				{
					vPos = m_optrTraceTargetGameUnit->GetWeaponBonePos( pEffectData->m_iWeaponIndex, pEffectData->m_PositionWeaponBoneName.c_str(), pEffectData->m_iModelIndex );
				}
				else 
				{
					if( true == pEffectData->m_bLandPosition )
					{
						vPos = m_optrTraceTargetGameUnit->GetLandPos();
					}
					else
					{
						vPos = m_optrTraceTargetGameUnit->GetPos();
					}
				}

				SetEffectSetOffset( vPos, pEffectData->m_vPositionOffset );
			}
			else if( null != m_optrGameUnit )
			{
				if( false == pEffectData->m_PositionBoneName.empty() )
				{
	#ifdef MODIFY_GET_BONE_POS
					if( 0 == pEffectData->m_PositionBoneName.compare( L"_HEAD_BONE" ) )
					{
						vPos = m_optrGameUnit->GetHeadBonePos();
					}
					// 없는 본에 대한 참조를 시도 한다면 헤드본으로 연결 하도록 수정
					else if( false == m_optrGameUnit->GetBonePos( pEffectData->m_PositionBoneName.c_str(), vPos ) )
					{
						vPos = m_optrGameUnit->GetHeadBonePos();
#ifdef ADD_RESOURCE_ERROR_LOG
						FindBoneErrorLog( pEffectData->m_EffectName.c_str() , pEffectData->m_PositionBoneName.c_str() );
#endif // ADD_RESOURCE_ERROR_LOG
					}
	#else
					if( 0 == pEffectData->m_PositionBoneName.compare( L"_HEAD_BONE" ) )
					{
						vPos = m_optrGameUnit->GetHeadBonePos();
					}
					else
					{
					vPos = m_optrGameUnit->GetBonePos( pEffectData->m_PositionBoneName.c_str() );
					}
	#endif //MODIFY_GET_BONE_POS
				}
				else if( false == pEffectData->m_PositionWeaponBoneName.empty() )
				{
					vPos = m_optrGameUnit->GetWeaponBonePos( pEffectData->m_iWeaponIndex, pEffectData->m_PositionWeaponBoneName.c_str(), pEffectData->m_iModelIndex );
				}
#ifdef RIDING_SYSTEM
				else if ( NULL != GetOwnerUnit() && NULL != GetOwnerUnit()->GetRidingPet() && false == pEffectData->m_PosRidingPetBoneName.empty() )
				{
					if( false == GetOwnerUnit()->GetRidingPet()->GetBonePos( pEffectData->m_PosRidingPetBoneName.c_str(), vPos ) )
					{
#ifdef ADD_RESOURCE_ERROR_LOG
						FindBoneErrorLog( pEffectData->m_EffectName.c_str() , pEffectData->m_PosRidingPetBoneName.c_str() );
#endif // ADD_RESOURCE_ERROR_LOG
					}
				}
#endif //RIDING_SYSTEM
				else 
				{
					if( true == pEffectData->m_bLandPosition )
					{
						vPos = m_optrGameUnit->GetLandPos();
					}
					else
					{
						vPos = m_optrGameUnit->GetPos();
					}
				}

				SetEffectSetOffset( vPos, pEffectData->m_vPositionOffset );
			}
			else 
			{
				return vPos;
			}
		} break;

	case ESM_FIELD:
		{
			if( m_pSquareUnit != NULL )
			{

				if( false == pEffectData->m_PositionBoneName.empty() )
				{
					vPos = m_pSquareUnit->GetBonePos( pEffectData->m_PositionBoneName.c_str() );
				}
				else 
				{
					if( true == pEffectData->m_bLandPosition )
					{
						vPos = m_pSquareUnit->GetLandPosition();
					}
					else
					{
						vPos = m_pSquareUnit->GetPos();
					}
				}

				SetEffectSetOffset( vPos, pEffectData->m_vPositionOffset );
			}
			
		} break;

#ifdef SERV_PET_SYSTEM
	case ESM_COMMON:
		{
			if( m_pPet != NULL )
			{
				if( false == pEffectData->m_PositionBoneName.empty() )
				{
					vPos = m_pPet->GetBonePos( pEffectData->m_PositionBoneName.c_str() );
				}
				else 
				{
					if( true == pEffectData->m_bLandPosition )
					{
						vPos = m_pPet->GetLandPosition_LUA();
					}
					else
					{
						vPos = m_pPet->GetPos();
					}
				}


				SetEffectSetOffset( vPos, pEffectData->m_vPositionOffset );
			}
		}
		break;
#endif
	}

	return vPos;
}


D3DXVECTOR3 CX2EffectSet::EffectSetInstance::GetEffectScale( EffectData* pEffectData )
{
	KTDXPROFILE();

	D3DXVECTOR3 vScale( 1, 1, 1 );

	if( 0.f != pEffectData->m_vScale.x )
	{
		vScale.x *= pEffectData->m_vScale.x;
		vScale.y *= pEffectData->m_vScale.y;
		vScale.z *= pEffectData->m_vScale.z;
	}

	if( 0.f != pEffectData->m_vScaleOnLeft.x &&
		false == IsUnitRight() )
	{
		vScale.x *= pEffectData->m_vScaleOnLeft.x;
		vScale.y *= pEffectData->m_vScaleOnLeft.y;
		vScale.z *= pEffectData->m_vScaleOnLeft.z;
	}

	//if( true == pEffectData->m_bApplyInstanceScale ) : 원본
	// UnitScale의 중복 곱을 피하기 위해 Effectset을 타고온 DamageEffect는 UnitScale을 적용하지 않는다.
	if( m_bDamageEffectScale == true || (true == pEffectData->m_bApplyInstanceScale && pEffectData->m_eEffectType != ET_DAMAGE_EFFECT) )	
	{
		vScale.x *= m_vScale.x;
		vScale.y *= m_vScale.y;
		vScale.z *= m_vScale.z;
	}
	
	return vScale;
}



D3DXVECTOR3 CX2EffectSet::EffectSetInstance::GetEffectRotation( EffectData* pEffectData )
{
	KTDXPROFILE();

	D3DXVECTOR3 vEffectRotation = pEffectData->m_vRotateRel;
	if( true == pEffectData->m_bApplyUnitRotation )
	{
#ifdef MODIFY_EFFECT_SET_TRACE_BONE
		if( INVALID_MESH_INSTANCE_HANDLE != m_hTargetMesh )
		{
			CKTDGXMeshPlayer::CXMeshInstance* pMeshInstance = GetMeshInstanceByTargetMesh();
			if( NULL != pMeshInstance )
			{
				vEffectRotation += pMeshInstance->GetRotateDegree();
			}
		}
		else
#endif //MODIFY_EFFECT_SET_TRACE_BONE
		{
			vEffectRotation += GetUnitRotation( pEffectData );
		}
	}
#ifdef SERV_ARA_CHANGE_CLASS_SECOND // 김태환
	/// 회전값 설정시 좌 우 완벽한 대칭을 이루기 위해, 방향에 따른 x회전 값 반전 기능 추가
	else if ( true == pEffectData->m_bApplyUnitRotationMirror )
	{
#ifdef MODIFY_EFFECT_SET_TRACE_BONE
		if( INVALID_MESH_INSTANCE_HANDLE != m_hTargetMesh )
		{
			CKTDGXMeshPlayer::CXMeshInstance* pMeshInstance = GetMeshInstanceByTargetMesh();
			if( NULL != pMeshInstance )
			{
				vEffectRotation += pMeshInstance->GetRotateDegree();
			}
		}
		else
#endif //MODIFY_EFFECT_SET_TRACE_BONE
		{
			vEffectRotation += GetUnitRotation( pEffectData );
		}

		if( false == IsUnitRight() )
		{
			vEffectRotation.x *= -1.f;
		}
	}
#endif // SERV_ARA_CHANGE_CLASS_SECOND

#ifdef ROTATE_EFFECTSET
	vEffectRotation += m_vLocalRotateDegree;
#endif

	return vEffectRotation;
}

D3DXVECTOR3 CX2EffectSet::EffectSetInstance::GetUnitRotation( EffectData* pEffectData )
{
	KTDXPROFILE();

	D3DXVECTOR3 vRotation( 0, 0, 0 );

#ifdef ADD_PET_NINE_TAIL_FOX		/// 펫에도 APPLY_UNIT_ROTATION 적용되도록 수정
	if( NULL == m_pPet && NULL == pEffectData )
#else  ADD_PET_NINE_TAIL_FOX
	if( NULL == pEffectData )
#endif ADD_PET_NINE_TAIL_FOX
		return vRotation;

#ifdef MODIFY_EFFECT_SET_TRACE_BONE
	if( INVALID_MESH_INSTANCE_HANDLE != m_hTargetMesh )
	{
		CKTDGXMeshPlayer::CXMeshInstance* pMeshInstance = GetMeshInstanceByTargetMesh();
		if( NULL != pMeshInstance )
		{
			pMeshInstance->GetRotateDegree();
		}
	}
#endif //MODIFY_EFFECT_SET_TRACE_BONE

	if( true == m_bCustomPosition )
	{
		return m_vRotationDegree;
	}

	switch( m_eMode )
	{
	case ESM_GAME:
		{
			if( NULL == g_pX2Game )
				return vRotation;

			if( null != m_optrTraceTargetGameUnit )
			{
				return m_optrTraceTargetGameUnit->GetRotateDegree();
			}
			else if ( null != m_optrGameUnit )
			{
				return m_optrGameUnit->GetRotateDegree();
			}
			else
				return vRotation;

		} break;

	case ESM_FIELD:
		{
			if( m_pSquareUnit != NULL )
			{
				return m_pSquareUnit->GetDirDegree();
			}
		
		} break;
#ifdef SERV_PET_SYSTEM
	case ESM_COMMON:
		{
			if( m_pPet != NULL )
			{
				return m_pPet->GetDirDegree();
			}
		} break;
#endif
	}

	return vRotation;
}


bool CX2EffectSet::EffectSetInstance::IsUnitRight()
{
	KTDXPROFILE();

	switch( m_eMode )
	{
	case ESM_GAME:
		{
			if( NULL == g_pX2Game )
				return false;

			if( null != m_optrTraceTargetGameUnit )
			{
				return m_optrTraceTargetGameUnit->GetIsRight();
			}
			else if( null != m_optrGameUnit )
			{
				return m_optrGameUnit->GetIsRight();
			}
			else
				return false;

		} break;

	case ESM_FIELD:
		{
			if( m_pSquareUnit != NULL )
			{
				if( NULL == g_pTFieldGame )
					return false;

				return m_pSquareUnit->GetIsRight();
			}
		} break;
	}

	return false;
}

//{{kimhc // 2011-01-17 // 지정한 Bone의 메트릭스 값을 Trace 함 (chung 코드 참고)
#ifdef	TRACE_MAXTRIX_TEST
const D3DXMATRIX* CX2EffectSet::EffectSetInstance::GetCombineMatrix( const EffectData* pEffectData ) const
{
	if( ESM_GAME == m_eMode && null != m_optrGameUnit )
	{
		if( false == pEffectData->m_PositionBoneName.empty() )
		{
			return m_optrGameUnit->GetCombineMatrixFromBoneName( pEffectData->m_PositionBoneName );			
		}
		else if( false == pEffectData->m_PositionWeaponBoneName.empty() )
		{
			return m_optrGameUnit->GetCombineMatrixFromWeaponBoneName( pEffectData->m_iWeaponIndex, pEffectData->m_PositionWeaponBoneName.c_str(), pEffectData->m_iModelIndex );			
		}
#ifdef RIDING_SYSTEM
		else if ( false == pEffectData->m_PosRidingPetBoneName.empty() )
		{
			return m_optrGameUnit->GetCombineMatrixFromRidingPetBoneName( pEffectData->m_PosRidingPetBoneName );
		}
#endif //RIDING_SYSTEM
	}

	return NULL;
}

#endif	TRACE_MAXTRIX_TEST
//}}kimhc // 2011-01-17 // 지정한 Bone의 메트릭스 값을 Trace 함 (chung 코드 참고)

CX2GameUnit* CX2EffectSet::EffectSetInstance::GetOwnerUnit() const
{
	{
		if( null != m_optrGameUnit )
			return m_optrGameUnit.GetObservable();
		else if( NULL != m_pPet )
			return m_pPet->GetMaster();
		else
			return NULL;
	}
}

/*static*/ D3DXVECTOR3 CX2EffectSet::GetZVector( const D3DXVECTOR3& vDirVector )
{
	D3DXVECTOR3 vZVector = vDirVector;
	if( vZVector.y != 0.f )
	{
		vZVector.y = vZVector.z;
		vZVector.z = vZVector.x;
		vZVector.x = -vZVector.y;
		vZVector.y = 0.f;
		D3DXVec3Normalize( &vZVector, &vZVector );
	}
	else
	{
		vZVector.y = vZVector.z;
		vZVector.z = vZVector.x;
		vZVector.x = -vZVector.y;
		vZVector.y = 0.f;
	}
	return vZVector;
}

//{{ kimhc // 2010.4.19 // 비밀던전 작업
#ifdef SERV_SECRET_HELL
CX2EffectSet::Handle CX2EffectSet::PlayEffectSet_LUA( const char* pEffectSetName, CX2GameUnit* pGameUnit )
{
	wstring wstrEffectSetName;
	ConvertUtf8ToWCHAR( wstrEffectSetName, pEffectSetName );

	return PlayEffectSet( wstrEffectSetName, pGameUnit );
}
#ifdef VELDER_BOSS_5
CX2EffectSet::Handle CX2EffectSet::PlayEffectSetWithCustomPos_LUA( const char* pEffectSetName, CX2GameUnit* pGameUnit, D3DXVECTOR3 vPos , D3DXVECTOR3 vRotationDegree)
#else
CX2EffectSet::Handle CX2EffectSet::PlayEffectSetWithCustomPos_LUA( const char* pEffectSetName, CX2GameUnit* pGameUnit, D3DXVECTOR3 vPos )
#endif VELDER_BOSS_5
{
	wstring wstrEffectSetName;
	ConvertUtf8ToWCHAR( wstrEffectSetName, pEffectSetName );

#ifdef VELDER_BOSS_5
	return PlayEffectSet( wstrEffectSetName, pGameUnit, NULL, false, -1.0f, -1.0f, D3DXVECTOR3( 1, 1, 1 ), 
		true, vPos, vRotationDegree);
#else
	return PlayEffectSet( wstrEffectSetName, pGameUnit, NULL, false, -1.0f, -1.0f, D3DXVECTOR3( 1, 1, 1 ), 
		true, vPos );
#endif VELDER_BOSS_5
}

//{{ JHKang / 강정훈 / 2011/01/19 / 이펙트를 가지고 있는 대상(NPC)과 이펙트가 따라다닐 대상(User)를 나누어 사용 시
#ifdef SEASON3_MONSTER_2010_12
CX2EffectSet::Handle CX2EffectSet::PlayEffectSetTraceTarget_LUA( const char* pEffectSetName, CX2GameUnit* pGameUnit, CX2GameUnit* pTraceTargetGameUnit )
{
	wstring wstrEffectSetName;
	ConvertUtf8ToWCHAR( wstrEffectSetName, pEffectSetName );

	return PlayEffectSet( wstrEffectSetName, pGameUnit, pTraceTargetGameUnit );
}
#endif SEASON3_MONSTER_2010_12
//}} JHKang / 강정훈 / 2011/01/19 / 이펙트를 가지고 있는 대상(NPC)과 이펙트가 따라다닐 대상(User)를 나누어 사용 시

void CX2EffectSet::StopEffectSet_LUA( CX2EffectSet::Handle hHandle )
{
	StopEffectSet( hHandle );
}
#endif SERV_SECRET_HELL
//}} kimhc // 2010.4.19 // 비밀던전 작업

#ifdef EFFECT_TOOL
bool CX2EffectSet::CreateEffectSetTemplet( EffectSetData* pEffectSetData_ )
{
	// 중복 체크.
	EffectSetDataMap::iterator it = m_mapEffectSetTemplet.find( pEffectSetData_->m_wstrEffectSetName );
	if( it != m_mapEffectSetTemplet.end() )
	{
		return false;
	}

	// 맵 추가
	m_mapEffectSetTemplet[ pEffectSetData_->m_wstrEffectSetName ] = pEffectSetData_;
	// 벡터 추가.
	m_vecEffectSetTemplet.push_back( pEffectSetData_ );

	return true;
}
bool CX2EffectSet::DeleteEffectSetTemplet( const wstring& wstrEffectSetName_ )
{
	// 존재 여부 체크.
	EffectSetDataMap::iterator mit = m_mapEffectSetTemplet.find( wstrEffectSetName_ );
	if( mit == m_mapEffectSetTemplet.end() )
	{
		return false;
	}

	//맵 제거.
	m_mapEffectSetTemplet.erase( mit );

	//벡터 제거
	std::vector< EffectSetData* >::iterator it = m_vecEffectSetTemplet.begin();
	for( ; it != m_vecEffectSetTemplet.end(); ++it )
	{
		if( (*it)->m_wstrEffectSetName == wstrEffectSetName_ )
		{
			m_vecEffectSetTemplet.erase( it );
			break;
		}
	}

	return true;
}
bool CX2EffectSet::ChangeNameEffectSetTemplet( const wstring& wstrEffectSetName_,  const wstring& wstrChangeEffectSetName_ )
{
	// 존재 여부 체크.
	EffectSetDataMap::iterator mit = m_mapEffectSetTemplet.find( wstrEffectSetName_ );
	if( mit == m_mapEffectSetTemplet.end() )
	{
		return false;
	}
	// 벡터 이름 변경
	std::vector< EffectSetData* >::iterator it = m_vecEffectSetTemplet.begin();
	for( ; it != m_vecEffectSetTemplet.end(); ++it )
	{
		if( (*it)->m_wstrEffectSetName == wstrEffectSetName_ )
		{
			(*it)->m_wstrEffectSetName = wstrChangeEffectSetName_;
			break;
		}
	}

	// 맵 추가.
	// 변경 전 이름의 key값을 가지고 있는 맵은 NULL 가리키도록 변경.	
	m_mapEffectSetTemplet[wstrChangeEffectSetName_] = mit->second;
	mit->second = NULL;
		
	return true;
}

void CX2EffectSet::AddEffectSet_Description_LUA()
{
	KLuaManager luaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState() );
	TableBind( &luaManager, g_pKTDXApp->GetLuaBinder() );

	m_mapEffectSetDescription;

	wstring wstrEffectSetName;
	LUA_GET_VALUE( luaManager,		"NAME",   wstrEffectSetName,			L""		);

	wstring wstrEffectSetDesc;
	luaManager.GetValue( "DESC", wstrEffectSetDesc);

	m_mapEffectSetDescription[wstrEffectSetName] = wstrEffectSetDesc;
}
void CX2EffectSet::SetEffectSetVersion_LUA()
{
	KLuaManager luaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState() );
	TableBind( &luaManager, g_pKTDXApp->GetLuaBinder() );

	m_mapEffectSetDescription;

	LUA_GET_VALUE( luaManager,		"VERSION",   m_wstrToolVersion,			L""		);
}
wstring CX2EffectSet::GetEffectSetDesc( const WCHAR* pEffectSetName_ )
{
	if( m_mapEffectSetDescription.find( pEffectSetName_ ) != m_mapEffectSetDescription.end() )
	{
		return m_mapEffectSetDescription[pEffectSetName_];
	}

	return L"";
}
void CX2EffectSet::AddEffectSet_Description( wstring wstrEffectSetName_, wstring wstrEffectSetDesc_ )
{
	m_mapEffectSetDescription[wstrEffectSetName_] = wstrEffectSetDesc_;
}

void CX2EffectSet::EffectSetInstance::SetEffectPoistion_EffectTool( IN const EffectData* pEffectData_, OUT D3DXVECTOR3& vPos_ )
{
	wstring wstrTargetBoneName = L"";
	if( false == pEffectData_->m_PosRidingPetBoneName.empty() )
	{
		wstrTargetBoneName.assign( pEffectData_->m_PosRidingPetBoneName );
	}
	else if( false == pEffectData_->m_PositionWeaponBoneName.empty() )
	{
		wstrTargetBoneName.assign( pEffectData_->m_PositionWeaponBoneName );
	}
	else
	{
		wstrTargetBoneName.assign( pEffectData_->m_PositionBoneName );
	}

	if( NULL == g_pX2Game )
		return;


	if( null != m_optrGameUnit )
	{
		if( false == wstrTargetBoneName.empty() )
		{
			if( 0 == wstrTargetBoneName.compare( L"_HEAD_BONE" ) ||
				false == m_optrGameUnit->GetBonePos( wstrTargetBoneName.c_str(), vPos_ ))
			{
				vPos_ = m_optrGameUnit->GetHeadBonePos();
			}
		}
		else 
		{
			if( true == pEffectData_->m_bLandPosition )
			{
				vPos_ = m_optrGameUnit->GetLandPos();
			}
			else
			{
				vPos_ = m_optrGameUnit->GetPos();
			}
		}

		SetEffectSetOffset( vPos_, pEffectData_->m_vPositionOffset );
	}
}
#endif //EFFECT_TOOL

/** @function : SetEffectSetOffset
	@brief : 이펙트셋에 설정된 오프셋값을 출력 위치에 적용 시켜 주는 함수
	@param : vPos_  ( 현재 위치에 대한 위치값. OUT 인자)
	         vOffsetPos_ ( EffectSet에 담겨있는 오프셋 정보 )
*/
void CX2EffectSet::EffectSetInstance::SetEffectSetOffset( OUT D3DXVECTOR3& vPos_, IN const D3DXVECTOR3& vOffsetPos_ )
{

	if( null != m_optrGameUnit ) // ESM_GAME
	{
#ifdef SERV_ARA_CHANGE_CLASS_SECOND
		/// 타겟 매시가 설정되어 있다면, 타겟 매시의 방향에 따라 OffSet 위치를 적용한다.
		if ( INVALID_MESH_INSTANCE_HANDLE != m_hTargetMesh )
		{
	#ifdef FIX_TARGET_MESH_OFFSET_POS // 김태환
			if ( false == IsSamef( vOffsetPos_.x, 0.f ) )
			{
				/// 방향벡터의 Y 값을 제거
				D3DXVECTOR3 vDirVector = m_vecTargetMeshDirVector;

				if( 0.f != vDirVector.y )
				{
					vDirVector.y = 0.f;
					D3DXVec3Normalize( &vDirVector, &vDirVector );
				}

				/// 방향에 따른 OffSet X 설정
				vPos_ += m_bIsTargetMeshRight ? ( vOffsetPos_.x * vDirVector ) : ( -vOffsetPos_.x * vDirVector );
			}

			/// 방향에 따른 OffSet Y 설정
			if ( false == IsSamef( vOffsetPos_.y, 0.f ) )
				vPos_.y += vOffsetPos_.y;

			/// 방향에 따른 OffSet Z 설정
			if ( false == IsSamef( vOffsetPos_.z, 0.f ) )
				vPos_ += vOffsetPos_.z * m_vecTargetMeshZVector;
	#else // FIX_TARGET_MESH_OFFSET_POS
			if ( false == IsSamef( vOffsetPos_.x, 0.f ) )
				vPos_.x += m_bIsTargetMeshRight ? vOffsetPos_.x : -vOffsetPos_.x;

			if ( false == IsSamef( vOffsetPos_.y, 0.f ) )
				vPos_.y += m_bIsTargetMeshRight ? vOffsetPos_.y : -vOffsetPos_.y;

			if ( false == IsSamef( vOffsetPos_.z, 0.f ) )
				vPos_.z += m_bIsTargetMeshRight ? vOffsetPos_.z : -vOffsetPos_.z;
	#endif // FIX_TARGET_MESH_OFFSET_POS
		}
		else
#endif //SERV_ARA_CHANGE_CLASS_SECOND
		{
			// X축 오프셋 적용
			if( 0.f != vOffsetPos_.x )
			{
				D3DXVECTOR3 vDirVector = m_optrGameUnit->GetDirVector();
				if( 0.f != vDirVector.y )
				{
					vDirVector.y = 0.f;
					D3DXVec3Normalize( &vDirVector, &vDirVector );
				}

				if( true == m_optrGameUnit->GetIsRight() )
					vPos_ += vDirVector * vOffsetPos_.x;
				else
					vPos_ -= vDirVector * vOffsetPos_.x;
			}

			// Y축 오프셋 적용
			if( 0.f != vOffsetPos_.y )
			{
				vPos_.y += vOffsetPos_.y;
			}

			// Z축 오프셋 적용
			if( 0.f != vOffsetPos_.z )
			{
				vPos_ += m_optrGameUnit->GetZVector() * vOffsetPos_.z;
			}
		}
	}
	else if ( null != m_pSquareUnit ) // ESM_FIELD
	{
		// X축 오프셋 적용
		if( 0.f != vOffsetPos_.x )
		{
			D3DXVECTOR3 vDirVector = m_pSquareUnit->GetDirVector();
			if( 0.f != vDirVector.y )
			{
				vDirVector.y = 0.f;
				D3DXVec3Normalize( &vDirVector, &vDirVector );
			}

			if( true == m_pSquareUnit->GetIsRight() )
				vPos_ += vDirVector * vOffsetPos_.x;
			else
				vPos_ -= vDirVector * vOffsetPos_.x;
		}

		// Y축 오프셋 적용
		if( 0.f != vOffsetPos_.y )
		{
			vPos_.y += vOffsetPos_.y;
		}

		// Z축 오프셋 적용
		if( 0.f != vOffsetPos_.z )
		{
			vPos_ += m_pSquareUnit->GetZVector() * vOffsetPos_.z;
		}
	}
	else if( NULL != m_pPet ) // ESM_COMMON
	{
		if( 0.f != vOffsetPos_.x )
		{
			D3DXVECTOR3 vDirVector = m_pPet->GetDirVector();
			if( 0.f != vDirVector.y )
			{
				vDirVector.y = 0.f;
				D3DXVec3Normalize( &vDirVector, &vDirVector );
			}

			if( true == m_pPet->GetIsRight() )
				vPos_ += vDirVector * vOffsetPos_.x;
			else
				vPos_ -= vDirVector * vOffsetPos_.x;
		}

		if( 0.f != vOffsetPos_.y )
		{
			vPos_.y += vOffsetPos_.y;
		}

		if( 0.f != vOffsetPos_.z )
		{
			vPos_ += m_pPet->GetZVector() * vOffsetPos_.z;
		}
	}
#ifdef MODIFY_EFFECT_SET_TRACE_BONE
	else if ( INVALID_MESH_INSTANCE_HANDLE != m_hTargetMesh)
	{
		vPos_.x += (0.f != vOffsetPos_.x) ? vOffsetPos_.x : 0.f;
		vPos_.y += (0.f != vOffsetPos_.y) ? vOffsetPos_.y : 0.f;
		vPos_.z += (0.f != vOffsetPos_.z) ? vOffsetPos_.z : 0.f;
	}
#endif //MODIFY_EFFECT_SET_TRACE_BONE
}

#ifdef FIX_ICE_HEATER_EVENT

/** @function	: PlayEffectSetByMeshPlayer
	@brief		: 매시에 이펙트 셋을 설정하는 함수
	@param		: 이펫트셋 이름(wstrEffectSetName). 매시 플레이어 객체 핸들(hTargetMesh), 메쉬 이펙트 타입(eTargetMeshType)
	@return		: 이펙트셋 핸들 값
*/

CX2EffectSet::Handle CX2EffectSet::PlayEffectSetByMeshPlayer( const WCHAR* wstrEffectSetName, 
															CKTDGXMeshPlayer::CXMeshInstanceHandle hTargetMesh, EFFECT_TYPE	eTargetMeshType )
{
	EffectSetData* pEffectSetData = GetEffectSetTemplet( wstrEffectSetName );
	if( NULL == pEffectSetData )
		return CX2EffectSet::INVALID_HANDLE;

	if( INVALID_MESH_INSTANCE_HANDLE == hTargetMesh)
		return CX2EffectSet::INVALID_HANDLE;
	
	EffectSetInstance* pEffectSetInstance = new EffectSetInstance( pEffectSetData, CKTDGXSkinAnimPtr(), NULL, NULL, CX2SquareUnitPtr(), NULL );
	if( CX2EffectSet::INVALID_HANDLE == pEffectSetInstance->m_hHandle )
	{
		SAFE_DELETE( pEffectSetInstance );
		return CX2EffectSet::INVALID_HANDLE;
	}

	// 타겟 메쉬 정보 설정
	pEffectSetInstance->m_hTargetMesh = hTargetMesh;
	pEffectSetInstance->m_eTargetMeshEffectType = eTargetMeshType;
	pEffectSetInstance->m_bIsTargetMesh = true;

	m_vecpEffectSetInstance.push_back( pEffectSetInstance );	
	return pEffectSetInstance->m_hHandle;
}

#endif // FIX_ICE_HEATER_EVENT



#ifdef HANABI_VILLAGE
CX2EffectSet::Handle CX2EffectSet::PlayEffectSetEvent( const wstring& wstrEffectSetName, CX2SquareUnitPtr pSquareUnit, D3DXVECTOR3 vPosition )
{
	EffectSetData* pEffectSetData = GetEffectSetTemplet( wstrEffectSetName );
	if( NULL == pEffectSetData )
		return CX2EffectSet::INVALID_HANDLE;

	if( pSquareUnit == NULL )
		return CX2EffectSet::INVALID_HANDLE;

	if( pSquareUnit->GetUnitViewer() == NULL )
		return CX2EffectSet::INVALID_HANDLE;

	CKTDGXSkinAnimPtr skinAnimPtr = pSquareUnit->GetUnitViewer()->GetXSkinAnimPtr();
	if( skinAnimPtr == NULL )
		return CX2EffectSet::INVALID_HANDLE;

	EffectSetInstance* pEffectSetInstance = new EffectSetInstance( pEffectSetData, skinAnimPtr, NULL, NULL, pSquareUnit, NULL );

	if( CX2EffectSet::INVALID_HANDLE == pEffectSetInstance->m_hHandle )
	{
		SAFE_DELETE( pEffectSetInstance );
		return CX2EffectSet::INVALID_HANDLE;
	}

	pEffectSetInstance->m_bCustomPosition = true;
	pEffectSetInstance->m_vPosition			= vPosition;
	pEffectSetInstance->m_vRotationDegree	= D3DXVECTOR3(0, 0, 0);
	pEffectSetInstance->m_vDirVector		= D3DXVECTOR3(0, 0, 0);
	pEffectSetInstance->m_vZVector			= CX2EffectSet::GetZVector( D3DXVECTOR3(0, 0, 0) );

	m_vecpEffectSetInstance.push_back( pEffectSetInstance );	
	return pEffectSetInstance->m_hHandle;
}
#endif HANABI_VILLAGE

#ifdef MODIFY_EFFECT_SET_TRACE_BONE // 오현빈
/** @function : GetMeshInstanceByTargetMesh
	@brief : TargetMesh 의 메쉬 인스턴스 객체 포인터 얻기
			 (타입에 따라 얻어오는 방법이 다르기 때문에 함수로 뺐음)
*/
CKTDGXMeshPlayer::CXMeshInstance* CX2EffectSet::EffectSetInstance::GetMeshInstanceByTargetMesh()
{
	if( INVALID_MESH_INSTANCE_HANDLE == m_hTargetMesh ||
		ET_INVALID == m_eTargetMeshEffectType )
		return NULL;

	CKTDGXMeshPlayer::CXMeshInstance* pMeshInstance = NULL;
	switch( m_eTargetMeshEffectType )
	{
	case ET_MESH_PLAYER_UI_MAJOR:
		{
			if( NULL != g_pData->GetUIMajorXMeshPlayer() )
			{
				pMeshInstance = g_pData->GetUIMajorXMeshPlayer()->GetMeshInstance( m_hTargetMesh );
			}
		} break;
	case ET_MESH_PLAYER_UI_MINOR:
		{
			if( NULL != g_pData->GetUIMinorXMeshPlayer() )
			{
				pMeshInstance = g_pData->GetUIMinorXMeshPlayer()->GetMeshInstance( m_hTargetMesh );
			}
		} break;
	case ET_MESH_PLAYER_GAME_MAJOR:
		{
			if( NULL != g_pX2Game &&
				NULL != g_pX2Game->GetMajorXMeshPlayer() )
			{
				pMeshInstance = g_pX2Game->GetMajorXMeshPlayer()->GetMeshInstance( m_hTargetMesh );
			}
		} break;
	case ET_MESH_PLAYER_GAME_MINOR:
		{
			if( NULL != g_pX2Game &&
				NULL != g_pX2Game->GetMinorXMeshPlayer() )
			{
				pMeshInstance = g_pX2Game->GetMinorXMeshPlayer()->GetMeshInstance( m_hTargetMesh );
			}
		} break;
	default:
		break;
	}

	return pMeshInstance;
}
bool CX2EffectSet::EffectSetInstance::GetEffectPositionByTargetMesh( OUT D3DXVECTOR3& vPos, IN EffectData* pEffectData )
{
	if( INVALID_MESH_INSTANCE_HANDLE == m_hTargetMesh ||
		ET_INVALID == m_eTargetMeshEffectType )
		return false;

	CKTDGXMeshPlayer::CXMeshInstance* pMeshInstance = GetMeshInstanceByTargetMesh();
	if( NULL != pMeshInstance )
	{
		if( false == pEffectData->m_PositionBoneName.empty() )
		{
			if( false == pMeshInstance->GetBonePos( pEffectData->m_PositionBoneName.c_str(), vPos ) )
			{
#ifdef ADD_RESOURCE_ERROR_LOG
				FindBoneErrorLog( pEffectData->m_EffectName.c_str() , pEffectData->m_PositionBoneName.c_str() );
#endif // ADD_RESOURCE_ERROR_LOG
			}
		}
		else if( false == pEffectData->m_PositionWeaponBoneName.empty() )
		{
			if( false == pMeshInstance->GetBonePos( pEffectData->m_PositionWeaponBoneName.c_str(), vPos ) )
			{
#ifdef ADD_RESOURCE_ERROR_LOG
				FindBoneErrorLog( pEffectData->m_EffectName.c_str() , pEffectData->m_PositionWeaponBoneName.c_str() );
#endif // ADD_RESOURCE_ERROR_LOG
			}
		}
		else if ( false == pEffectData->m_PosRidingPetBoneName.empty() )
		{
			if( false == pMeshInstance->GetBonePos( pEffectData->m_PosRidingPetBoneName.c_str() ) )
			{
#ifdef ADD_RESOURCE_ERROR_LOG
				FindBoneErrorLog( pEffectData->m_EffectName.c_str() , pEffectData->m_PosRidingPetBoneName.c_str() );
#endif // ADD_RESOURCE_ERROR_LOG
			}
		}
		else
		{
			vPos = pMeshInstance->GetPos();
		}
	}
	else
	{
		m_hTargetMesh = INVALID_MESH_INSTANCE_HANDLE;
		m_eTargetMeshEffectType = ET_INVALID;
		return false;
	}

	return true;
}
#endif // MODIFY_EFFECT_SET_TRACE_BONE

#ifdef ADD_RESOURCE_ERROR_LOG
void CX2EffectSet::EffectSetInstance::FindBoneErrorLog( const WCHAR* EffectSetName, const WCHAR* wcBoneName_ )
{
	if( m_setEmptyBoneName.end() == m_setEmptyBoneName.find( wcBoneName_ ) )
	{
		m_setEmptyBoneName.insert( wcBoneName_ );

		wstring errorMsg;
		errorMsg = L"Bone 찾기 실패. 이펙트셋 이름: ";
		errorMsg += EffectSetName;
		errorMsg += L" 본 이름: ";
		errorMsg += wcBoneName_;
		ErrorLogMsg( KEM_ERROR430, errorMsg.c_str() );
	}
}
#endif // ADD_RESOURCE_ERROR_LOG