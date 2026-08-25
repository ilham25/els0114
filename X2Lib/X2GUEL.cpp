#include "StdAfx.h"
#include "X2GUEL.h"

#ifdef NEW_CHARACTER_EL

namespace _CONST_EL_
{
	const float CHIVALRY_BUFF_CONDITION_HP_RATE = 0.3f;	 // 기사의 숙명 버프 조건 연동 체력 배율

	const float MAX_EXTINCTION_CHARGE_TIME		= 1.f;	 // 익스팅션 최대 충전 시간
	const float LIMIT_EXTINCTION_POWER_RATE		= 0.2f;  // 익스팅션 공격력 증가 제한 값

	const float MIN_EXTINCTION_DEGREE_Z			= -45.f; // 익스팅션 최소 각도
	const float MAX_EXTINCTION_DEGREE_Z			= 0.f;   // 익스팅션 최고 각도
	const float BASE_EXTINCTION_DEGREE_Z		= -30.f; // 익스팅션 기본 각도
	const float EXTINCTION_DEGREE_Z_CONTROL		= 100.f; // 익스팅션 각도 조절 값
	
	// 검의 길
	const float GALE_COOL_TIME_DECREASE_RATE		= 0.2f;	// 질풍 스킬 쿨타임 Rate
	const float GALE_COOL_TIME_DECREASE_RATE_PVP	= 0.6f;	// 질풍 스킬 쿨타임 Rate(대전 1/2)

	const float GALE_MP_CONSUME_RATE				= 0.6f; // 질풍 스킬 MP 소모량 Rate
	const float GALE_MP_CONSUME_RATE_PVP			= 0.8f; // 질풍 스킬 MP 소모량 Rate(대전 1/2)

	const float GALE_FORCE_DOWN_DECREASE_RATE		= 0.6f;	// 질풍 커맨드 다운수치 감소Rate
	const float GALE_FORCE_DOWN_DECREASE_RATE_PVP	= 0.8f;	// 질풍 커맨드 다운수치 감소Rate(대전 1/2)

	// 커맨드 
	const UINT MAX_DASH_JUMP_COMBO_SPK_XZ_REATTACK	= 2;	// ESI_DASH_JUMP_COMBO_SPK_XZ 추가 공격 가능 횟수

	// MP 소모
	const float FIRE_BALL_MP_COST					= 6.f;
	const float SPK_DASH_JUMP_XZ_MP_COST			= 4.f;
}

static const CX2GUUser::SkillCutInSet s_SkillCutInSet[] =
{
	{ 
		CX2GUUser::SkillCutIn( L"HQ_CutIn_Elesis.dds"),
		CX2GUUser::SkillCutIn( L"HQ_CutIn_Elesis_Pyro_Knight.dds"),
		CX2GUUser::SkillCutIn( L"HQ_CutIn_Elesis_Saber_Knight.dds"),
	},
};

/*static */CX2GUEL* CX2GUEL::CreateGUEL( int unitIndex, int teamNum, lua_State*   pScriptLuaState, int frameBufferNum, CX2Unit* pUnit )
{
	return new CX2GUEL( unitIndex, teamNum, pScriptLuaState, frameBufferNum, pUnit );
};

CX2GUEL::CX2GUEL( int unitIndex, int teamNum, lua_State*   pScriptLuaState, int frameBufferNum, CX2Unit* pUnit )
: CX2GUUser( unitIndex, teamNum, pScriptLuaState, frameBufferNum, pUnit )
#ifdef ELSWORD_WAY_OF_SWORD // 검의 길
, m_iPreComboCount( -1 )
#endif // ELSWORD_WAY_OF_SWORD
, m_iPressedSkillSlotIndex( -1 )
, m_fMPConsume ( 0.f )
, m_fFireBallPowerRate(1.f)
, m_fChivalryBuffCheckTime ( 0.f )
, m_uiChivalryAttackBuffFactorID ( 0 )
, m_uiChivalryDefenceBuffFactorID ( 0 )
, m_uiChivalryLevel( 0 )
, m_bIsActiveEndurancePower( false )
, m_fExtinctionRemainTime( 0.f )
, m_fExtinctionDegreeZ ( 30.f )
, m_bIsApplyVictoriousSword ( false )
, m_fDodgeAndSlashCoolTimeDecreaseRate ( 1.f )
, m_hExtinctionArrow(INVALID_PARTICLE_HANDLE)
, m_hExctionBG(INVALID_PARTICLE_HANDLE)
, m_pExtinctionArrow(NULL)
, m_pSkillTemplet(NULL)
, m_bIsDashAttackPossible(true)
, m_uiSPKXZReattakCount(0)
, m_uiDodgeAndSlashMaxReuseCount(0)
, m_uiDodgeAndSlashReuseCount(0)
{
	m_fThrowStartTime = 0.66f;
	m_vThrowStartPosOffset = D3DXVECTOR3(40.f, 130.f, 0.f);
	m_vThrowStartVelocity = D3DXVECTOR3(2000.f, 100.f, 0.f);
}

CX2GUEL::~CX2GUEL(void)
{
}

/*static*/ void CX2GUEL::AppendDeviceToDeviceList( CKTDXDeviceDataList& listInOut_ )
{
}

/*static*/ void CX2GUEL::AppendComponentToDeviceList( CKTDXDeviceDataList& listInOut_ )
{
}

/*static*/ void CX2GUEL::AppendEffectToDeviceList( CKTDXDeviceDataList& listInOut_, CX2DamageManager::EXTRA_DAMAGE_TYPE eDamageType_ )
{
	CX2GUEL::AppendMajorParticleToDeviceList( listInOut_ );
	CX2GUEL::AppendMinorParticleToDeviceList( listInOut_ );
	CX2GameUnit::Weapon::AppendEnchantParticleForSwordToDeviceList( listInOut_, eDamageType_ );
}

/*virtual */bool CX2GUEL::CanRidingState()
{
	if( true == CX2GUUser::CanRidingState() )
		return true;

 	switch( GetNowStateID() )
 	{
 	case ESI_SWORD_WAIT:
 		return true;
 	}

	return false;
}

/*virtual*/ void CX2GUEL::Init( bool bUseTeamPos_, int iStartPosIndex_ )
{
	CX2GUUser::Init( bUseTeamPos_, iStartPosIndex_ );

	SetNeedToUpdateWeaponEffect( true );
	UpdateEquipCollisionData();
}
/*virtual*/ void CX2GUEL::ParseCommonRandomState()
{
	if( true == m_LuaManager.BeginTable( L"INIT_COMMON_RANDOM_STATE" ) )
	{
		std::wstring tableName = L"";
		switch( m_pUnit->GetClass() )
		{
		case CX2Unit::UC_ELESIS_KNIGHT:
			{
				tableName = L"EL_KNIGHT";
			} break;
		case CX2Unit::UC_ELESIS_SABER_KNIGHT:
			{
				tableName = L"EL_SABER_KNIGHT";
			} break;
		case CX2Unit::UC_ELESIS_PYRO_KNIGHT:
			{
				tableName = L"EL_PYRO_KNIGHT";
			} break;
		default:
			{
				ASSERT( !"NO" );
			} break;
		}

		CX2GUUser::ParseCommonRandomState(tableName);
		m_LuaManager.EndTable(); // INIT_COMMON_RANDOM_STATE
	}
}
/*virtual*/ void CX2GUEL::InitState()
{
	switch ( GetUnitClass() )
	{
	case CX2Unit::UC_ELESIS_KNIGHT:
	case CX2Unit::UC_ELESIS_SABER_KNIGHT:
	case CX2Unit::UC_ELESIS_PYRO_KNIGHT:
		{
			m_iSkillCutInSetIndex = 0;
		} break;
	default:
	case CX2Unit::UC_NONE:
		{
			ASSERT( !L"Unit Class Is fault!" );
			m_iSkillCutInSetIndex = 0;
		} break;
	}

	InitStateID();
	InitStateCommon();
	InitStateByUnitClass();

	std::wstring wstrStartTableName;
	std::wstring wstrWinTableName;
	std::wstring wstrLoseTableName;

	InitStateCommonRandom( wstrStartTableName, wstrWinTableName, wstrLoseTableName );

	UserUnitStateData stateData;

	stateData.Init();
	stateData.stateID			= USI_START;
	m_LuaManager.MakeTableReference( wstrStartTableName.c_str(), stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, StartEventProcess );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUUser, StartEnd );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_WIN;
	m_LuaManager.MakeTableReference( wstrWinTableName.c_str(), stateData.stateID );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, WinStateStart );	
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_LOSE;
	m_LuaManager.MakeTableReference( wstrLoseTableName.c_str(), stateData.stateID );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, LoseStateStart );	
	m_StateList[stateData.stateID] = stateData;

	InitPassiveSkillState();
	InitEquippedActiveSkillState();

	CX2GUUser::InitState();
}
/*virtual*/ void CX2GUEL::InitComponent()
{
	CX2GUUser::InitComponent();

#ifdef ELSWORD_WAY_OF_SWORD // 검의 길
	if( true == IsMyUnit() )
	{
		ASSERT( NULL != g_pX2Game );
		switch ( g_pX2Game->GetGameType() )
		{
		case CX2Game::GT_PVP:
			{
				static_cast<CX2ElesisGageData*>(m_pGageData)->SetWayOfSwordPoint( 0.f );
				static_cast<CX2ElesisGageData*>(m_pGageData)->SetWayOfSwordState( 0 );
				m_FrameDataFuture.syncData.m_CannonBallCount = GetWayOfSwordState();				
			} break;

		default:
			{
				m_FrameDataFuture.syncData.m_CannonBallCount = GetWayOfSwordState();
			} break;
		}

		switch ( GetWayOfSwordState() )
		{
		case WSS_VIGOR:
			{
				Start_SwordmanVigor();
			} break;
		case WSS_DESTRUCTION:
			{
				Start_SwordmanDestruction();
			} break;
		default:
			break;
		}
	}
#endif // ELSWORD_WAY_OF_SWORD
}
/*virtual*/ void CX2GUEL::InitPassiveSkillState()
{
	CX2GUUser::InitPassiveSkillState();
 	const CX2UserSkillTree& userSkillTree = GetUnit()->GetUnitData()->m_UserSkillTree;
 	CX2SkillTree* const pSkillTree = g_pData->GetSkillTree();
 
	// 집중력 향상 초기화
	SetAddMagicAttackCriticalDamageRate(0.f);
	SetAddMagicAttackCriticalRate(0.f);
	// 순발력 향상 초기화
	SetAddAnimSpeedFactor(0.f);

	// 불의 친화 초기화
	SetFireBallPowerRate(1.f);

	// 기사의 숙명 버프 ID 초기화
	m_uiChivalryAttackBuffFactorID = 0;
	m_uiChivalryDefenceBuffFactorID = 0;
	m_uiChivalryLevel = 0;

	// 닷지&슬래시 재사용 횟수
	m_uiDodgeAndSlashMaxReuseCount = 0;

	// 노전직 공용 스킬
	{
		// 닷지&슬래시
		{
			const CX2SkillTree::SKILL_ID eSkillID = CX2SkillTree::SI_A_EL_DODGE_AND_SLASH;
			const int iSkillLevel = userSkillTree.GetSkillLevel( eSkillID );
			if ( 0 < iSkillLevel )
			{
				const CX2SkillTree::SkillTemplet* pSkillTemplet = pSkillTree->GetSkillTemplet( eSkillID );
				if( NULL != pSkillTemplet )
				{
					// 스킬 재사용 횟수 
					m_uiDodgeAndSlashMaxReuseCount = static_cast<UINT>(pSkillTemplet->GetSkillAbilityValue( CX2SkillTree::SA_EFFECTIVE_COUNT, iSkillLevel  ));
				}
			}
		}
	}

	switch( GetUnitClass() )
	{
	// 1-1 세이버 나이트
	case CX2Unit::UC_ELESIS_SABER_KNIGHT:
		{
			// 순발력 향상
			{
 				const CX2SkillTree::SKILL_ID eSkillID = CX2SkillTree::SI_P_SSK_AGILITY_MASTERY;
 				const int iSkillLevel = userSkillTree.GetSkillLevel( eSkillID );
 				if ( 0 < iSkillLevel )
 				{
 					const CX2SkillTree::SkillTemplet* pSkillTemplet = pSkillTree->GetSkillTemplet( eSkillID );
 
 					if( NULL != pSkillTemplet )
 					{
 						// 이동 속도 증가
 						const float fMoveSpeed = pSkillTemplet->GetSkillAbilityValue( CX2SkillTree::SA_MOVE_SPEED, iSkillLevel  );
 				
						
#ifdef FIXED_INCREASE_MOVE_SPEED_INITIALIZING // 130925 김종훈, 이동속도 증가 효과가 적용되지 않는 문제 수정
						m_AdditionalWalkSpeed.SetPassiveMultiple( m_AdditionalWalkSpeed.GetPassiveMultiple() * ( 1.f + fMoveSpeed ) );
						m_AdditionalDashSpeed.SetPassiveMultiple( m_AdditionalDashSpeed.GetPassiveMultiple() * ( 1.f + fMoveSpeed ) );
#else //  FIXED_INCREASE_MOVE_SPEED_INITIALIZING // 130925 김종훈, 이동속도 증가 효과가 적용되지 않는 문제 수정
						if( 0 > fMoveSpeed)
						{
							m_PhysicParam.fWalkSpeed += m_PhysicParam.fWalkSpeed *= fMoveSpeed;
							m_PhysicParam.fWalkSpeed += m_PhysicParam.fRunSpeed *= fMoveSpeed;
						}
#endif // FIXED_INCREASE_MOVE_SPEED_INITIALIZING // 130925 김종훈, 이동속도 증가 효과가 적용되지 않는 문제 수정

						
						// 동작 속도 증가
						float fAddAnimSpeedFactor = CalculateIncreasingRate( pSkillTemplet->GetSkillAbilityValue( CX2SkillTree::SA_ANI_SPEED, iSkillLevel  ) );
 						SetAddAnimSpeedFactor( max(0.f, fAddAnimSpeedFactor) );
 					}
 				}
			}
			// 기사의 숙명
			{
 				const CX2SkillTree::SKILL_ID eSkillID = CX2SkillTree::SI_P_SSK_CHIVALRY;
 				const int iSkillLevel = userSkillTree.GetSkillLevel( eSkillID );
 				if ( 0 < iSkillLevel )
 				{
					m_uiChivalryLevel = static_cast<UINT>(iSkillLevel);
 					const CX2SkillTree::SkillTemplet* pSkillTemplet = pSkillTree->GetSkillTemplet( eSkillID );
 					if( NULL != pSkillTemplet && 
 						2 <= pSkillTemplet->m_vecBuffFactorID.size() )
 					{					
  						m_uiChivalryAttackBuffFactorID = pSkillTemplet->m_vecBuffFactorID[0];
  						m_uiChivalryDefenceBuffFactorID = pSkillTemplet->m_vecBuffFactorID[1];
 					}
 				}
			}
		} break;	
	// 1-2 파이로 나이트
	case CX2Unit::UC_ELESIS_PYRO_KNIGHT:
		{
			// 집중력 향상
			{
 				const CX2SkillTree::SKILL_ID eSkillID = CX2SkillTree::SI_P_SPK_CONCENTRATION_MASTERY;
 				const int iSkillLevel = userSkillTree.GetSkillLevel( eSkillID );
 				if ( 0 < iSkillLevel )
 				{
 					const CX2SkillTree::SkillTemplet* pSkillTemplet = pSkillTree->GetSkillTemplet( eSkillID );
 					if( NULL != pSkillTemplet )
 					{
 						// 마법 공격력 크리티컬 데미지 배율 증가
						SetAddMagicAttackCriticalDamageRate( pSkillTemplet->GetSkillAbilityValue( CX2SkillTree::SA_ATK_CRITICAL_DAM_ABS, iSkillLevel  ) );
 						// 마법 공격력 크리티컬 발동 확률 증가
 						SetAddMagicAttackCriticalRate( pSkillTemplet->GetSkillAbilityValue( CX2SkillTree::SA_ATK_CRITICAL_RATE_ABS, iSkillLevel  ) );
 					}
 				}
			}
			// 불의 신체
			{
 #ifndef CHUNG_FIRST_CLASS_CHANGE
 				불의 신체 패시브 오류! CX2GUUser::m_EnchantStatFromPassiveSkill 를 사용 할 수 없는 상태
 #endif //CHUNG_FIRST_CLASS_CHANGE
 				const CX2SkillTree::SKILL_ID eSkillID = CX2SkillTree::SI_P_SPK_BODY_OF_FIRE;
 				const int iSkillLevel = userSkillTree.GetSkillLevel( eSkillID );
 				if ( 0 < iSkillLevel )
 				{
 					const CX2SkillTree::SkillTemplet* pSkillTemplet = pSkillTree->GetSkillTemplet( eSkillID );
 					if( NULL != pSkillTemplet )
 					{
 						// 파이어볼 공격력 증가
						SetFireBallPowerRate( GetFireBallPowerRate() * pSkillTemplet->GetSkillAbilityValue( CX2SkillTree::SA_DAMAGE_REL, iSkillLevel  ) );
 
 						// 불 저항 증가방법은 SA_FIRE_DEFENCE_ABS 검색 통해서 확인 하면 됨.
 					}
 				}
			}
		} break;
	default:
		break;
	}
}
/*virtual*/ void CX2GUEL::SetEquippedSkillFuncBySkillSlotIndex( const CX2SkillTree::SKILL_ID eSkillID_, const int iSlotIndex_, const bool bSlotB_ )
{
	if ( iSlotIndex_ < 0 || iSlotIndex_ >= ARRAY_SIZE( _CONST_GUUSER_::aNormalSkillStateID ) )
		return;

	int iNormalStateID = ( bSlotB_ == false ? _CONST_GUUSER_::aNormalSkillStateID[iSlotIndex_] : _CONST_GUUSER_::aNormalSkillStateIDSlotB[iSlotIndex_] );
	UserUnitStateData& normalStateData = m_StateList[ iNormalStateID ];
	normalStateData.Init();
	normalStateData.stateID = iNormalStateID;

	int iHyperStateID = ( bSlotB_ == false ? _CONST_GUUSER_::aHyperSkillStateID[iSlotIndex_] : _CONST_GUUSER_::aHyperSkillStateIDSlotB[iSlotIndex_] );
	UserUnitStateData& hyperStateData  = m_StateList[ iHyperStateID ];
	hyperStateData.Init();	
	hyperStateData.m_bHyperState = true;
	hyperStateData.stateID = iHyperStateID;

	const CX2SkillTree::SkillTemplet* pSkillTempletEquipped = GetEquippedActiveSkillTemplet( eSkillID_ );
	if( NULL == pSkillTempletEquipped )
		return;

	switch( pSkillTempletEquipped->m_eType )
	{	
	case CX2SkillTree::ST_BUFF:
		{	// 버프
			m_LuaManager.MakeTableReference( L"ESI_COMMON_BUFF", normalStateData.stateID );
			normalStateData.OnFrameMove		= SET_CB_FUNC( CX2GUEL, COMMON_BUFF_FrameMove );	
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, COMMON_BUFF_EventProcess );	

			m_LuaManager.MakeTableReference( L"ESI_COMMON_BUFF", hyperStateData.stateID );
			hyperStateData.OnFrameMove		= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess	= normalStateData.OnEventProcess;

			return;
		} break;
	case CX2SkillTree::ST_ACTIVE:
	case CX2SkillTree::ST_SPECIAL_ACTIVE:
		{
			// ST_ACTIVE, ST_SPECIAL_ACTIVE는 아래 구문에서 수행
		} break;
#ifdef ADDED_RELATIONSHIP_SYSTEM
	case CX2SkillTree::ST_RELATIONSHIP_SKILL:
		{
			switch( eSkillID_ )
			{
			case CX2SkillTree::SI_ETC_WS_COMMON_LOVE:
				{
					m_LuaManager.MakeTableReference( L"ESI_THROW_ITEM", normalStateData.stateID );
					normalStateData.OnFrameMove		= SET_CB_FUNC( CX2GUUser, COMMON_RELATIONSHIP_SKILL_FrameMove );	
					normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, COMMON_RELATIONSHIP_SKILL_EventProcess );	

					m_LuaManager.MakeTableReference( L"ESI_THROW_ITEM", hyperStateData.stateID );
					hyperStateData.OnFrameMove		= normalStateData.OnFrameMove;
					hyperStateData.OnEventProcess	= normalStateData.OnEventProcess;
				} break;
			}
			return;
		} break;
#endif // ADDED_RELATIONSHIP_SYSTEM
	default:
		{
			return;
		} break;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////// 여기서 부터 ACTIVE, SPECIAL ACTIVE 스킬//////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region ACTIVE, SPECIAL ACTIVE 스킬
	switch ( eSkillID_ )
	{
	case CX2SkillTree::SI_SA_SSK_SPIRAL_BLAST:
		{	/// 스파이럴 블래스트
			m_LuaManager.MakeTableReference( L"ESI_SI_SA_SSK_SPIRAL_BLAST", normalStateData.stateID );
			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_SPIRAL_BLAST_Init );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_SPIRAL_BLAST_EventProcess );

			m_LuaManager.MakeTableReference( L"ESI_SI_SA_SSK_SPIRAL_BLAST", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
		} break;
	case CX2SkillTree::SI_SA_SSK_EXTINCTION:
		{ // 세이버 - 익스팅션
			m_LuaManager.MakeTableReference( L"ESI_SI_SA_SSK_EXTINCTION_START", normalStateData.stateID );

			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_EXTINCTION_START_Init );
			normalStateData.StateStart		= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_EXTINCTION_START_Start );
			normalStateData.StateStartFuture= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_EXTINCTION_START_StartFuture );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_EXTINCTION_START_EventProcess );

			m_LuaManager.MakeTableReference( L"ESI_SI_SA_SSK_EXTINCTION_START", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.StateInit			= normalStateData.StateInit;
			hyperStateData.StateStart			= normalStateData.StateStart;
			hyperStateData.StateStartFuture		= normalStateData.StateStartFuture;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
			{
				UserUnitStateData stateData;

				stateData.Init();
				stateData.stateID			= ESI_SI_SA_SSK_EXTINCTION_CHARGE;
				m_LuaManager.MakeTableReference( L"ESI_SI_SA_SSK_EXTINCTION_CHARGE", stateData.stateID );
				stateData.OnFrameMove		= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_EXTINCTION_CHARGE_FrameMove );
				stateData.OnFrameMoveFuture = SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_EXTINCTION_CHARGE_FrameMoveFuture );
				stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_EXTINCTION_CHARGE_EventProcess );
				m_StateList[stateData.stateID] = stateData;

				stateData.Init();
				stateData.stateID			= ESI_SI_SA_SSK_EXTINCTION_ATTACK;
				m_LuaManager.MakeTableReference( L"ESI_SI_SA_SSK_EXTINCTION_ATTACK", stateData.stateID );
				stateData.StateStartFuture	= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_EXTINCTION_ATTACK_StartFuture );
				stateData.OnFrameMove		= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_EXTINCTION_ATTACK_FrameMove );
				stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_EXTINCTION_ATTACK_EventProcess );
				m_StateList[stateData.stateID] = stateData;

				stateData.Init();
				stateData.stateID			= ESI_SI_SA_SSK_EXTINCTION_DEGREE_UP;
				m_LuaManager.MakeTableReference( L"ESI_SI_SA_SSK_EXTINCTION_DEGREE", stateData.stateID );
				stateData.OnFrameMove		= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_EXTINCTION_DEGREE_UP_FrameMove );
				stateData.OnFrameMoveFuture = SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_EXTINCTION_DEGREE_UP_FrameMoveFuture );
				stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_EXTINCTION_DEGREE_UP_EventProcess );
				m_StateList[stateData.stateID] = stateData;

				stateData.Init();
				stateData.stateID			= ESI_SI_SA_SSK_EXTINCTION_DEGREE_DOWN;
				m_LuaManager.MakeTableReference( L"ESI_SI_SA_SSK_EXTINCTION_DEGREE", stateData.stateID );
				stateData.OnFrameMove		= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_EXTINCTION_DEGREE_DOWN_FrameMove );
				stateData.OnFrameMoveFuture = SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_EXTINCTION_DEGREE_DOWN_FrameMoveFuture );
				stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_EXTINCTION_DEGREE_DOWN_EventProcess );
				m_StateList[stateData.stateID] = stateData;
			}
		} break;
	case CX2SkillTree::SI_SA_SSK_VICTORIOUS_SWORD:
		{	/// 승리의 검
			m_LuaManager.MakeTableReference( L"ESI_SI_SA_SSK_VICTORIOUS_SWORD", normalStateData.stateID );
			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_VICTORIOUS_SWORD_Init );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_VICTORIOUS_SWORD_EventProcess );

			m_LuaManager.MakeTableReference( L"ESI_SI_SA_SSK_VICTORIOUS_SWORD", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
		} break;
	case CX2SkillTree::SI_SA_SSK_JUDGEMENT_DRIVE:
		{ // 저지먼트 드라이브
			m_LuaManager.MakeTableReference( L"ESI_SI_SA_SSK_JUDGEMENT_DRIVE", normalStateData.stateID );
			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_JUDGEMENT_DRIVE_Init );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SSK_JUDGEMENT_DRIVE_EventProcess );

			m_LuaManager.MakeTableReference( L"ESI_SI_SA_SSK_JUDGEMENT_DRIVE", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
		} break;
	case CX2SkillTree::SI_SA_SPK_BURST_RISING:
		{	/// 버스트 라이징
			m_LuaManager.MakeTableReference( L"ESI_SI_SA_SPK_BURST_RISING", normalStateData.stateID );
			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SPK_BURST_RISING_Init );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SPK_BURST_RISING_EventProcess );

			m_LuaManager.MakeTableReference( L"ESI_SI_SA_SPK_BURST_RISING", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
		} break;
	case CX2SkillTree::SI_SA_SPK_BURST_WAVE:
 		{ // 버스트 웨이브
 			m_LuaManager.MakeTableReference( L"ESI_SI_SA_SPK_BURST_WAVE", normalStateData.stateID );
 			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SPK_BURST_WAVE_Init );
 			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SPK_BURST_WAVE_EventProcess );
 
 			m_LuaManager.MakeTableReference( L"ESI_SI_SA_SPK_BURST_WAVE", hyperStateData.stateID );
 			hyperStateData.m_bHyperState		= true;
 			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
 			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
 		} break;
	case CX2SkillTree::SI_SA_SPK_SWORDFIRE:
		{	/// 소드 파이어
			m_LuaManager.MakeTableReference( L"ESI_SI_SA_SPK_SWORDFIRE", normalStateData.stateID );
			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SPK_SWORDFIRE_Init );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SPK_SWORDFIRE_EventProcess );

			m_LuaManager.MakeTableReference( L"ESI_SI_SA_SPK_SWORDFIRE", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
		} break;
	case CX2SkillTree::SI_SA_SPK_INFERNAL_BLADE:
		{ // 인페르날 블레이드
			m_LuaManager.MakeTableReference( L"ESI_SI_SA_SPK_INFERNAL_BLADE", normalStateData.stateID );
			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SPK_INFERNAL_BLADE_Init );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SPK_INFERNAL_BLADE_EventProcess );

			m_LuaManager.MakeTableReference( L"ESI_SI_SA_SPK_INFERNAL_BLADE", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
		} break;
	case CX2SkillTree::SI_SA_SPK_ETERNAL_FIRE:
		{ // 이터널 파이어
		 	m_LuaManager.MakeTableReference( L"ESI_SI_SA_SPK_ETERNAL_FIRE", normalStateData.stateID );
		 	normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SPK_ETERNAL_FIRE_Init );
		 	normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_SPK_ETERNAL_FIRE_EventProcess );
		 
		 	m_LuaManager.MakeTableReference( L"ESI_SI_SA_SPK_ETERNAL_FIRE", hyperStateData.stateID );
		 	hyperStateData.m_bHyperState		= true;
		 	hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
		 	hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
		} break;
	case CX2SkillTree::SI_A_SPK_SWORD_ERUPTION:
		{ // 소드 이럽션
			m_LuaManager.MakeTableReference( L"ESI_SI_A_SPK_SWORD_ERUPTION", normalStateData.stateID );
			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_A_SPK_SWORD_ERUPTION_Init );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_A_SPK_SWORD_ERUPTION_EventProcess );

			m_LuaManager.MakeTableReference( L"ESI_SI_A_SPK_SWORD_ERUPTION", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
		} break;
	case CX2SkillTree::SI_A_SPK_BIG_BURST:
 	{	/// 대폭살
 		m_LuaManager.MakeTableReference( L"ESI_SI_A_SPK_BIG_BURST", normalStateData.stateID );
 		normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_A_SPK_BIG_BURST_Init );
 		normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_A_SPK_BIG_BURST_EventProcess );
 
 		m_LuaManager.MakeTableReference( L"ESI_SI_A_SPK_BIG_BURST", hyperStateData.stateID );
 		hyperStateData.m_bHyperState		= true;
 		hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
 		hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
 	} break;
	case CX2SkillTree::SI_A_SSK_CROSS_SLASH:
		{	/// 크로스 슬래시
			m_LuaManager.MakeTableReference( L"ESI_SI_A_SSK_CROSS_SLASH", normalStateData.stateID );
			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_A_SSK_CROSS_SLASH_Init );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_A_SSK_CROSS_SLASH_EventProcess );
			
			m_LuaManager.MakeTableReference( L"ESI_SI_A_SSK_CROSS_SLASH", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
		} break;
	case CX2SkillTree::SI_A_SSK_POWER_BREAK:
		{	/// 파워 브레이크
			m_LuaManager.MakeTableReference( L"ESI_SI_A_SSK_POWER_BREAK", normalStateData.stateID );
			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL,ESI_SI_A_SSK_POWER_BREAK_Init );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL,ESI_SI_A_SSK_POWER_BREAK_EventProcess );

			m_LuaManager.MakeTableReference( L"ESI_SI_A_SSK_POWER_BREAK", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.StateStart			= normalStateData.StateStart;
			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
		} break;
	case CX2SkillTree::SI_A_SSK_HEAVY_STUNNER:
		{	/// 헤비 스터너
			m_LuaManager.MakeTableReference( L"ESI_SI_A_SSK_HEAVY_STUNNER", normalStateData.stateID );
			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL,ESI_SI_A_SSK_HEAVY_STUNNER_Init );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL,ESI_SI_A_SSK_HEAVY_STUNNER_EventProcess );

			m_LuaManager.MakeTableReference( L"ESI_SI_A_SSK_HEAVY_STUNNER", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
		} break;
//////////////////////////////////////////////////////////////////////////////////////////////////////
//아래 노전직 스킬
//////////////////////////////////////////////////////////////////////////////////////////////////////		
	case CX2SkillTree::SI_SA_EL_UNLIMITED_BLADE:
		{ // 언리미티드 블레이드
			m_LuaManager.MakeTableReference( L"ESI_SI_SA_EL_UNLIMITED_BLADE", normalStateData.stateID );
			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_EL_UNLIMITED_BLADE_Init );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_EL_UNLIMITED_BLADE_EventProcess );

			m_LuaManager.MakeTableReference( L"ESI_SI_SA_EL_UNLIMITED_BLADE", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
		} break;
	case CX2SkillTree::SI_SA_EL_WILD_SHOCK:
		{	/// 와일드 쇼크
			m_LuaManager.MakeTableReference( L"ESI_SI_SA_EL_WILD_SHOCK", normalStateData.stateID );
			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_EL_WILD_SHOCK_Init );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_EL_WILD_SHOCK_EventProcess );

			m_LuaManager.MakeTableReference( L"ESI_SI_SA_EL_WILD_SHOCK", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
		} break;
	case CX2SkillTree::SI_SA_EL_MEGA_BURSTER:
		{ // 메가 버스터
			m_LuaManager.MakeTableReference( L"ESI_SI_SA_EL_MEGA_BURSTER", normalStateData.stateID );
			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_EL_MEGA_BURSTER_Init );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_EL_MEGA_BURSTER_EventProcess );
	
			m_LuaManager.MakeTableReference( L"ESI_SI_SA_EL_MEGA_BURSTER", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
		} break;
	case CX2SkillTree::SI_SA_EL_POWER_BURSTER:
		{ // 파워 버스터
			m_LuaManager.MakeTableReference( L"ESI_SI_SA_EL_POWER_BURSTER", normalStateData.stateID );
			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_EL_POWER_BURSTER_Init );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_SA_EL_POWER_BURSTER_EventProcess );
	
			m_LuaManager.MakeTableReference( L"ESI_SI_SA_EL_POWER_BURSTER", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
		} break;
	case CX2SkillTree::SI_A_EL_SONIC_ASSAULT_STING:
		{	/// 소닉 어썰트 - 스팅
			m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_SONIC_ASSAULT_STING", normalStateData.stateID );
			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_SONIC_ASSAULT_STING_Init );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_SONIC_ASSAULT_STING_EventProcess );

			m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_SONIC_ASSAULT_STING", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
		} break;
	case CX2SkillTree::SI_A_EL_SONIC_ASSAULT_STAB:
		{	/// 소닉 어썰트 - 스탭
			m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_SONIC_ASSAULT_STAB", normalStateData.stateID );
			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_SONIC_ASSAULT_STAB_Init );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_SONIC_ASSAULT_STAB_EventProcess );
	
			m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_SONIC_ASSAULT_STAB", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
		} break;
	case CX2SkillTree::SI_A_EL_MEGA_SLASH:
		{ // 메가 슬래시
			m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_MEGA_SLASH", normalStateData.stateID );
			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_MEGA_SLASH_Init );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_MEGA_SLASH_EventProcess );

			m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_MEGA_SLASH", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
		} break;
	case CX2SkillTree::SI_A_EL_KICK:
	{ // 발차기
		m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_KICK", normalStateData.stateID );
		normalStateData.StateStart		= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_KICK_Start );
		normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_KICK_Init );
		normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_KICK_EventProcess );
	
		m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_KICK", hyperStateData.stateID );
		hyperStateData.m_bHyperState		= true;
		hyperStateData.StateStart			= normalStateData.StateStart;
		hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
	} break;
	case CX2SkillTree::SI_A_EL_LEAP_CRASH:
	{ // 도약
		m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_LEAP_CRASH", normalStateData.stateID );
		normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_LEAP_CRASH_Init );
		normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_LEAP_CRASH_EventProcess );
	
		m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_LEAP_CRASH", hyperStateData.stateID );
		hyperStateData.m_bHyperState		= true;
		hyperStateData.StateStart			= normalStateData.StateStart;
		hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;

		{
			UserUnitStateData stateData;
			stateData.Init();
			stateData.stateID			= ESI_SI_A_EL_LEAP_CRASH_JUMP;
			m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_LEAP_CRASH_JUMP", stateData.stateID );
			stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_LEAP_CRASH_JUMP_EventProcess );
			m_StateList[stateData.stateID] = stateData;

			stateData.Init();
			stateData.stateID			= ESI_SI_A_EL_LEAP_CRASH_JUMP_LANDING;
			m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_LEAP_CRASH_JUMP_LANDING", stateData.stateID );
			stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_LEAP_CRASH_JUMP_LANDING_EventProcess );
			m_StateList[stateData.stateID] = stateData;
		}
	} break;
	case CX2SkillTree::SI_A_EL_RUSHING_SWORD:
		{ // 러싱 소드
			m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_RUSHING_SWORD", normalStateData.stateID );
			normalStateData.StateStartFuture = SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_RUSHING_SWORD_StartFuture );
			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_RUSHING_SWORD_Init );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_RUSHING_SWORD_EventProcess );

			m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_RUSHING_SWORD", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.StateStartFuture		= normalStateData.StateStartFuture;
			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
			{
				UserUnitStateData stateData;
				stateData.Init();
				stateData.stateID			= ESI_SI_A_EL_RUSHING_SWORD_2nd;
				m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_RUSHING_SWORD_2nd", stateData.stateID );
				stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_RUSHING_SWORD_2nd_EventProcess );
				m_StateList[stateData.stateID] = stateData;

				stateData.Init();
				stateData.stateID			= ESI_SI_A_EL_RUSHING_SWORD_3rd;
				m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_RUSHING_SWORD_3rd", stateData.stateID );
				stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_RUSHING_SWORD_3rd_EventProcess );
				m_StateList[stateData.stateID] = stateData;
			}
		} break;
	case CX2SkillTree::SI_A_EL_DODGE_AND_SLASH:
		{	/// 닷지&슬래시
			m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_DODGE_AND_SLASH", normalStateData.stateID );
			normalStateData.StateStartFuture = SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_DODGE_AND_SLASH_StartFuture );
			normalStateData.StateInit		= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_DODGE_AND_SLASH_Init );
			normalStateData.OnFrameMove		= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_DODGE_AND_SLASH_FrameMove );
			normalStateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_DODGE_AND_SLASH_EventProcess );

			m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_DODGE_AND_SLASH", hyperStateData.stateID );
			hyperStateData.m_bHyperState		= true;
			hyperStateData.StateStartFuture		= normalStateData.StateStartFuture;
			hyperStateData.OnFrameMove			= normalStateData.OnFrameMove;
			hyperStateData.OnEventProcess		= normalStateData.OnEventProcess;
			{
				UserUnitStateData stateData;
				stateData.Init();
				stateData.stateID			= ESI_SI_A_EL_DODGE_AND_SLASH_SECOND;
				m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_DODGE_AND_SLASH", stateData.stateID );
				stateData.StateStartFuture  = SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_DODGE_AND_SLASH_SECOND_StartFuture );
				stateData.OnFrameMove		= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_DODGE_AND_SLASH_FrameMove );	//공용
				stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_DODGE_AND_SLASH_EventProcess ); //공용
				m_StateList[stateData.stateID] = stateData;

				stateData.Init();
				stateData.stateID			= ESI_SI_A_EL_DODGE_AND_SLASH_ATTACK_Z;
				m_LuaManager.MakeTableReference( L"ESI_SI_A_EL_DODGE_AND_SLASH_ATTACK_Z", stateData.stateID );
				stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_DODGE_AND_SLASH_ATTACK_Z_EventProcess );
				m_StateList[stateData.stateID] = stateData;
			}
		} break;
	default:
		break;
	}
#pragma endregion ACTIVE, SPECIAL ACTIVE 스킬
}


/*virtual*/ void CX2GUEL::SetEquippedSkillLevel( const CX2SkillTree::SKILL_ID eSkillID_, const bool bChangeAll_ )
{
	if ( true == bChangeAll_ || CX2SkillTree::SI_A_EL_DODGE_AND_SLASH == eSkillID_ )
	{/// 닷지&슬래시
		const CX2SkillTree::SkillTemplet* pSkillTemplet = GetEquippedActiveSkillTemplet( CX2SkillTree::SI_A_EL_DODGE_AND_SLASH );
		if( NULL != pSkillTemplet )
		{
			SetEquippedSkillLevelStateData( pSkillTemplet, ESI_SI_A_EL_DODGE_AND_SLASH_SECOND );
			SetEquippedSkillLevelStateData( pSkillTemplet, ESI_SI_A_EL_DODGE_AND_SLASH_ATTACK_Z );
		}
	}	
	if ( true == bChangeAll_ || CX2SkillTree::SI_A_EL_RUSHING_SWORD == eSkillID_ )
	{/// 러싱 소드
		const CX2SkillTree::SkillTemplet* pSkillTemplet = GetEquippedActiveSkillTemplet( CX2SkillTree::SI_A_EL_RUSHING_SWORD );
		if( NULL != pSkillTemplet )
		{
			SetEquippedSkillLevelStateData( pSkillTemplet, ESI_SI_A_EL_RUSHING_SWORD_2nd );
			SetEquippedSkillLevelStateData( pSkillTemplet, ESI_SI_A_EL_RUSHING_SWORD_3rd );
		}
	}	
	if ( true == bChangeAll_ || CX2SkillTree::SI_SA_SSK_EXTINCTION == eSkillID_ )
	{/// 세이버 - 익스팅션
		const CX2SkillTree::SkillTemplet* pSkillTemplet = GetEquippedActiveSkillTemplet( CX2SkillTree::SI_SA_SSK_EXTINCTION );
		if( NULL != pSkillTemplet )
		{
			SetEquippedSkillLevelStateData( pSkillTemplet, ESI_SI_SA_SSK_EXTINCTION_CHARGE );
			SetEquippedSkillLevelStateData( pSkillTemplet, ESI_SI_SA_SSK_EXTINCTION_ATTACK );
			SetEquippedSkillLevelStateData( pSkillTemplet, ESI_SI_SA_SSK_EXTINCTION_DEGREE_UP );
			SetEquippedSkillLevelStateData( pSkillTemplet, ESI_SI_SA_SSK_EXTINCTION_DEGREE_DOWN );
		}
	}
	if ( true == bChangeAll_ || CX2SkillTree::SI_A_EL_ENDURANCE_POWER == eSkillID_ )
	{/// 극기 - 강(强)
		const CX2SkillTree::SkillTemplet* pSkillTemplet = GetEquippedActiveSkillTemplet( CX2SkillTree::SI_A_EL_ENDURANCE_POWER );
		if( NULL != pSkillTemplet )
		{
			SetEquippedSkillLevelStateData( pSkillTemplet, ESI_SA_A_EL_ENDURANCE_POWER_ACTIVE );
		}
	}
	if ( true == bChangeAll_ || CX2SkillTree::SI_A_EL_LEAP_CRASH == eSkillID_ )
	{/// 도약
		const CX2SkillTree::SkillTemplet* pSkillTemplet = GetEquippedActiveSkillTemplet( CX2SkillTree::SI_A_EL_LEAP_CRASH );
		if( NULL != pSkillTemplet )
		{
			SetEquippedSkillLevelStateData( pSkillTemplet, ESI_SI_A_EL_LEAP_CRASH_JUMP );
			SetEquippedSkillLevelStateData( pSkillTemplet, ESI_SI_A_EL_LEAP_CRASH_JUMP_LANDING );
		}
	}
		
}
			 
/*virtual*/ void CX2GUEL::CommonStateStartFuture()
{
	KTDXPROFILE();
	CX2GUUser::CommonStateStartFuture();
}
/*virtual*/ void CX2GUEL::CommonStateStart()
{
	CX2GUUser::CommonStateStart();
}
			 
/*virtual*/ void CX2GUEL::CommonFrameMoveFuture()
{
	KTDXPROFILE();
	CX2GUUser::CommonFrameMoveFuture();
#ifdef ELSWORD_WAY_OF_SWORD // 검의 길
	m_FrameDataFuture.syncData.m_CannonBallCount = GetWayOfSwordState();
#endif // ELSWORD_WAY_OF_SWORD
}

/*virtual*/ void CX2GUEL::CommonFrameMove()
{
	KTDXPROFILE();
	CX2GUUser::CommonFrameMove();

#ifdef ELSWORD_WAY_OF_SWORD // 검의 길
	WayOfSwordStateChange();
	GaleBuffFrameMove();
#endif // ELSWORD_WAY_OF_SWORD

	// 기사의 숙명 패시브 처리
	ChivalryProc_FrameMove();
}
			 
/*virtual*/ void CX2GUEL::CommonStateEndFuture()
{
	KTDXPROFILE();
	CX2GUUser::CommonStateEndFuture();
}

/*virtual*/ void CX2GUEL::CommonStateEnd()
{
	KTDXPROFILE();
	CX2GUUser::CommonStateEnd();
}

/*virtual*/ void CX2GUEL::DetonationStateChange( CX2DamageManager::DamageData* pDamageData )
{
#ifdef ELSWORD_WAY_OF_SWORD // 검의 길 섬멸 기폭
	if( WSS_DESTRUCTION == GetWayOfSwordState() )
	{ // 섬멸 게이지로 기폭 시 슈아팅
		DoStateEndurancePowerRage();
		SetNowChargeMpForDetonation( 0.f ) ;
	}
	else
#endif // ELSWORD_WAY_OF_SWORD
	{
		CX2GUUser::DetonationStateChange( pDamageData );
	}
}
			 
/*virtual*/ RENDER_HINT	CX2GUEL::CommonRender_Prepare()
{
	KTDXPROFILE();

#ifdef ELSWORD_WAY_OF_SWORD // 검의 길
	if( GetWayOfSwordState() == WSS_VIGOR )
	{
		m_RenderParam.renderType		= CKTDGXRenderer::RT_CARTOON_BLACK_EDGE;
		m_RenderParam.cartoonTexType	= CKTDGXRenderer::CTT_SKYBLUE;
		m_RenderParam.outLineColor		= 0xffffffff;
	}
	else if( GetWayOfSwordState() == WSS_DESTRUCTION )
	{
		m_RenderParam.renderType		= CKTDGXRenderer::RT_CARTOON_BLACK_EDGE;
		m_RenderParam.cartoonTexType	= CKTDGXRenderer::CTT_SCARLET;
		m_RenderParam.outLineColor		= 0xffffffff;
	}
#endif // ELSWORD_WAY_OF_SWORD

	return CX2GUUser::CommonRender_Prepare();
}
			 
/*virtual*/ bool CX2GUEL::SpecialAttackEventProcess( CX2SkillTree::ACTIVE_SKILL_USE_CONDITION eActiveSkillUseCondition_ /*= CX2SkillTree::ASUT_GROUND */)
{
	KTDXPROFILE();

	if ( false == CanUseSpecialAttack( eActiveSkillUseCondition_ ) )
		return false;

	if( NULL == m_pSkillTemplet )
	{
		return false;
	}
	else
	{
		StateChangeSpecialAttack( m_pSkillTemplet );
	}

#ifdef SERV_SKILL_USE_SUBQUEST
	Send_SKILL_USE_REQ( m_pSkillTemplet->m_eID );
#endif SERV_SKILL_USE_SUBQUEST

	return true;

}
/** @function	: NoStateChangeActionFrameMove
	@brief		: 스테이트의 변화가 없는 스킬 사용 시 수행되는 FrameMove
*/
/*virtual*/ void CX2GUEL::NoStateChangeActionFrameMove()
{
	switch( m_FrameDataNow.syncData.nowAction )
	{
	case UAI_ENDURANCE_POWER:
		{
			if ( false == GetRidingOn() )
				ProcessEndurancePowerSkill();
		} break;
	default:
		break;
	}
}

/*virtual*/ float CX2GUEL::GetActualMPConsume( const CX2SkillTree::SKILL_ID eSkillID_, const int iSkillLevel_ ) const
{
	float fMpConsumption = CX2GUUser::GetActualMPConsume( eSkillID_, iSkillLevel_ );
	
	switch ( eSkillID_ )
	{
		// 닷지&슬래시
	case CX2SkillTree::SI_A_EL_DODGE_AND_SLASH:
		{
			// 승리의 검 버프가 적용 된 상태라면
			if( true == m_bIsApplyVictoriousSword )
			{
				fMpConsumption = 0.f;
			}

		} break;
	default:
		break;
	}

#ifdef ELSWORD_WAY_OF_SWORD
//////////////////////////////////////////////////////////////////////////
// 질풍 스킬 MP 예외처리.	
	// 오현빈
	// GetEquippedActiveSkillTemplet()을 두번 호출 하는 단점이 있지만
	// UI에서도 공용으로 사용 할 수 있도록 현재 위치에서 처리
	if( CX2GUUser::WSS_VIGOR == GetWayOfSwordState() )
	{
		const CX2SkillTree::SkillTemplet* pSkillTemplet 
			= GetEquippedActiveSkillTemplet( eSkillID_ );

		if( NULL != pSkillTemplet && 
			WSS_VIGOR == pSkillTemplet->m_iWOSType )
		{
			// 대전에서는 감소량 1/2
			if( NULL != g_pX2Game && CX2Game::GT_PVP == g_pX2Game->GetGameType())	
				fMpConsumption *= _CONST_EL_::GALE_MP_CONSUME_RATE_PVP;
			else
				fMpConsumption *= _CONST_EL_::GALE_MP_CONSUME_RATE;
		}
	}
#endif // ELSWORD_WAY_OF_SWORD

	return fMpConsumption;
}
float CX2GUEL::GetActualCoolTime( IN const CX2SkillTree::SkillTemplet* pSkillTemplet_, IN int iSkillLevel ) const
{
	if( NULL == pSkillTemplet_ )
		return 0.f;

	float fSkillCoolTime = pSkillTemplet_->GetSkillCoolTimeValue( iSkillLevel ) ;
#ifdef ELSWORD_WAY_OF_SWORD // 검의 길
	if( WSS_VIGOR == GetWayOfSwordState() )
	{ // 질풍 스킬 쿨타임 감소
		if( WSS_VIGOR == pSkillTemplet_->m_iWOSType )
		{
			float fGaleCoolTimeDescraseRate = _CONST_EL_::GALE_COOL_TIME_DECREASE_RATE;

			// 대전에서는 감소량 1/2
			if( NULL != g_pX2Game && CX2Game::GT_PVP == g_pX2Game->GetGameType())	
				fGaleCoolTimeDescraseRate = _CONST_EL_::GALE_COOL_TIME_DECREASE_RATE_PVP;

			fSkillCoolTime *= fGaleCoolTimeDescraseRate;
		}
	}
#endif // ELSWORD_WAY_OF_SWORD

	switch ( pSkillTemplet_->m_eID )
	{
	// 닷지&슬래시
	case CX2SkillTree::SI_A_EL_DODGE_AND_SLASH:
		{
			// 승리의 검 버프가 적용 된 상태라면
			if( true == m_bIsApplyVictoriousSword )
			{
				ASSERT( m_fDodgeAndSlashCoolTimeDecreaseRate < 1.f );
				fSkillCoolTime *= m_fDodgeAndSlashCoolTimeDecreaseRate;
			}

		} break;
	default:
		break;
	}

	return max( 0.f, fSkillCoolTime );
}
/*virtual*/ CX2GageData* CX2GUEL::CreateGageData()
{
	if ( true == IsMyUnit() )
	{
		const CX2ElesisGageData* pElswordGageData = 
			static_cast<const CX2ElesisGageData*>( CX2GageManager::GetInstance()->GetMyGageData() );

		if ( NULL != pElswordGageData )
			return new CX2ElesisGageData( *pElswordGageData );
		else
			return new CX2ElesisGageData();
	}
	else
		return new CX2ElesisGageData();
}

/*virtual*/	bool CX2GUEL::ShouldResetNotBusyTimer()
{	
	if ( false == CX2GUUser::ShouldResetNotBusyTimer() )
		return false;
	else if ( ESI_SWORD_WAIT == m_FrameDataFuture.syncData.nowState )
		return false;

	return true; // 바쁜 상태 라면 리셋 해야 함
}

void CX2GUEL::InitStateID()
{
	m_CommonState.m_Wait	= USI_WAIT;
}

/** @function	: InitStateCommon
	@brief		: 모든 캐릭터가 공통적으로 쓰는 상태 및 전직에 상관없이 공통으로 쓰는 상태 초기화
*/
void CX2GUEL::InitStateCommon()
{
	UserUnitStateData stateData;

	stateData.Init();
	stateData.stateID			= USI_DIE_FRONT;
	m_LuaManager.MakeTableReference( L"ESI_DIE_FRONT", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUEL, DieFrontStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUEL, DieFrontStart );
	stateData.OnFrameMove		= SET_CB_FUNC( CX2GUEL, DieFrontFrameMove );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DieFrontEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DIE_BACK;
	m_LuaManager.MakeTableReference( L"ESI_DIE_BACK", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUEL, DieBackStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUEL, DieBackStart );
	stateData.OnFrameMove		= SET_CB_FUNC( CX2GUEL, DieBackFrameMove );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DieBackEventProcess );
	m_StateList[stateData.stateID] = stateData;


	stateData.Init();
	stateData.stateID			= USI_START_INTRUDE;
	m_LuaManager.MakeTableReference( L"ESI_START_INTRUDE", stateData.stateID );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, StartIntrudeStart );
	stateData.OnFrameMove		= SET_CB_FUNC( CX2GUUser, StartIntrudeFrameMove );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, StartIntrudeEventProcess );
	m_StateList[stateData.stateID] = stateData;


	stateData.Init();
	stateData.stateID			= USI_WAIT;
	m_LuaManager.MakeTableReference( L"ESI_WAIT", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUEL, WaitStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUEL, WaitStart );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, WaitEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_WALK;
	m_LuaManager.MakeTableReference( L"ESI_WALK", stateData.stateID );
	stateData.OnFrameMoveFuture	= SET_CB_FUNC( CX2GUUser, WalkFrameMoveFuture );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, WalkEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_JUMP_READY;
	m_LuaManager.MakeTableReference( L"ESI_JUMP_READY", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, JumpReadyEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_JUMP_UP;
	m_LuaManager.MakeTableReference( L"ESI_JUMP_UP", stateData.stateID );
	stateData.OnFrameMoveFuture	= SET_CB_FUNC( CX2GUUser, JumpFrameMoveFuture );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, JumpUpEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_JUMP_DOWN;
	m_LuaManager.MakeTableReference( L"ESI_JUMP_DOWN", stateData.stateID );
	stateData.OnFrameMoveFuture	= SET_CB_FUNC( CX2GUUser, JumpFrameMoveFuture );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, JumpDownEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_JUMP_LANDING;
	m_LuaManager.MakeTableReference( L"ESI_JUMP_LANDING", stateData.stateID );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, JumpLandingStart );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUEL, JumpLandingStartFuture );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, JumpLandingEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DASH;
	m_LuaManager.MakeTableReference( L"ESI_DASH", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, DashStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, DashStart );
	stateData.OnFrameMoveFuture	= SET_CB_FUNC( CX2GUUser, DashFrameMoveFuture );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DashEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DASH_END;
	m_LuaManager.MakeTableReference( L"ESI_DASH_END", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUEL, DashEndStartFuture );
	stateData.OnFrameMove		= SET_CB_FUNC( CX2GUEL, DashEndFrameMove );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DashEndEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DASH_JUMP;
	m_LuaManager.MakeTableReference( L"ESI_DASH_JUMP", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUEL, DashJumpStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUEL, DashJumpStart );
	stateData.OnFrameMoveFuture	= SET_CB_FUNC( CX2GUEL, DashJumpFrameMoveFuture );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DashJumpEventProcess );
	stateData.StateEndFuture	= SET_CB_FUNC( CX2GUEL, DashJumpEndFuture );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DASH_JUMP_LANDING;
	m_LuaManager.MakeTableReference( L"ESI_DASH_JUMP_LANDING", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUEL, DashJumpLandingStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, JumpLandingStart );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DashJumpLandingEventProcess );
	m_StateList[stateData.stateID] = stateData;


	stateData.Init();
	stateData.stateID			= USI_HYPER_MODE;
	m_LuaManager.MakeTableReference( L"ESI_HYPER_MODE", stateData.stateID );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, HyperModeStart );
	stateData.OnFrameMove		= SET_CB_FUNC( CX2GUEL, HyperModeFrameMove );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, HyperModeEventProcess );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, HyperModeStartFuture );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUUser, HyperModeEnd );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DAMAGE_GROGGY;
	m_LuaManager.MakeTableReference( L"ESI_DAMAGE_GROGGY", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, DAMAGE_GROGGY_EventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DAMAGE_SMALL_FRONT;
	m_LuaManager.MakeTableReference( L"ESI_DAMAGE_SMALL_FRONT", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DamageSmallEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DAMAGE_SMALL_BACK;
	m_LuaManager.MakeTableReference( L"ESI_DAMAGE_SMALL_BACK", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DamageSmallEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DAMAGE_BIG_FRONT;
	m_LuaManager.MakeTableReference( L"ESI_DAMAGE_BIG_FRONT", stateData.stateID );
	stateData.StateStart		= SET_CB_FUNC( CX2GUEL, DamageBigStart );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DamageBigEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DAMAGE_BIG_BACK;
	m_LuaManager.MakeTableReference( L"ESI_DAMAGE_BIG_BACK", stateData.stateID );
	stateData.StateStart		= SET_CB_FUNC( CX2GUEL, DamageBigStart );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DamageBigEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DAMAGE_DOWN_FRONT;
	m_LuaManager.MakeTableReference( L"ESI_DAMAGE_DOWN_FRONT", stateData.stateID );
	stateData.OnFrameMove		= SET_CB_FUNC( CX2GUEL, DamageDownFrontFrameMove );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DamageDownFrontEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DAMAGE_DOWN_BACK;
	m_LuaManager.MakeTableReference( L"ESI_DAMAGE_DOWN_BACK", stateData.stateID );
	stateData.OnFrameMove		= SET_CB_FUNC( CX2GUEL, DamageDownBackFrameMove );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DamageDownBackEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DAMAGE_STANDUP_FRONT;
	m_LuaManager.MakeTableReference( L"ESI_DAMAGE_STANDUP_FRONT", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, DamageStandUpEventProcess );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUUser, DamageStandUpEnd );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DAMAGE_STANDUP_BACK;
	m_LuaManager.MakeTableReference( L"ESI_DAMAGE_STANDUP_BACK", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, DamageStandUpEventProcess );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUUser, DamageStandUpEnd );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DAMAGE_AIR_SMALL;
	m_LuaManager.MakeTableReference( L"ESI_DAMAGE_AIR_SMALL", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, DamageAirSmallStartFuture );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, DamageAirSmallEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DAMAGE_AIR_DOWN;
	m_LuaManager.MakeTableReference( L"ESI_DAMAGE_AIR_DOWN", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, DamageAirDownEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DAMAGE_AIR_DOWN_INVINCIBLE;
	m_LuaManager.MakeTableReference( L"ESI_DAMAGE_AIR_DOWN_INVINCIBLE", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, DamageAirDownInvincibleEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DAMAGE_AIR_DOWN_LANDING;
	m_LuaManager.MakeTableReference( L"ESI_DAMAGE_AIR_DOWN_LANDING", stateData.stateID );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, DamageAirDownLandingStart );
	stateData.OnFrameMove		= SET_CB_FUNC( CX2GUUser, DamageAirDownLandingFrameMove );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DamageAirDownLandingEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DAMAGE_AIR_FALL;
	m_LuaManager.MakeTableReference( L"ESI_DAMAGE_AIR_FALL", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, DamageAirFallEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DAMAGE_AIR_UP;
	m_LuaManager.MakeTableReference( L"ESI_DAMAGE_AIR_UP", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, DamageAirUpEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DAMAGE_AIR_FLY_FRONT;
	m_LuaManager.MakeTableReference( L"ESI_DAMAGE_AIR_FLY_FRONT", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, DamageAirFlyEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DAMAGE_AIR_FLY_BACK;
	m_LuaManager.MakeTableReference( L"ESI_DAMAGE_AIR_FLY_BACK", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, DamageAirFlyEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_DAMAGE_REVENGE;
	m_LuaManager.MakeTableReference( L"ESI_DAMAGE_REVENGE", stateData.stateID );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, DamageRevengeStart );
	stateData.OnCameraMove		= SET_CB_FUNC( CX2GUUser, DamageRevengeCameraMove );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, DamageRevengeEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_THROW_ITEM;
	m_LuaManager.MakeTableReference( L"ESI_THROW_ITEM", stateData.stateID );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, THROW_ITEM_StateStart );
	stateData.OnFrameMoveFuture	= SET_CB_FUNC( CX2GUUser, THROW_ITEM_FrameMoveFuture );
	stateData.OnFrameMove		= SET_CB_FUNC( CX2GUUser, THROW_ITEM_FrameMove );
	stateData.OnCameraMove		= SET_CB_FUNC( CX2GUUser, THROW_ITEM_CameraMove );		
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, THROW_ITEM_EventProcess );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUUser, THROW_ITEM_StateEnd );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_PEPPER_RUN_READY;
	m_LuaManager.MakeTableReference( L"ESI_PEPPER_RUN_READY", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, PEPPER_RUN_READY_EventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_PEPPER_RUN;
	m_LuaManager.MakeTableReference( L"ESI_PEPPER_RUN", stateData.stateID );
	stateData.OnFrameMoveFuture	= SET_CB_FUNC( CX2GUUser, PEPPER_RUN_FrameMoveFuture );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, PEPPER_RUN_EventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_PEPPER_RUN_END;
	m_LuaManager.MakeTableReference( L"ESI_PEPPER_RUN_END", stateData.stateID );
	stateData.OnFrameMove		= SET_CB_FUNC( CX2GUUser, PEPPER_RUN_END_FrameMove );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, PEPPER_RUN_END_EventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_PEPPER_RUN_JUMP_UP;
	m_LuaManager.MakeTableReference( L"ESI_PEPPER_RUN_JUMP_UP", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, PEPPER_RUN_JUMP_UP_StateStartFuture );
	stateData.OnFrameMoveFuture	= SET_CB_FUNC( CX2GUUser, PEPPER_RUN_JUMP_UP_FrameMoveFuture );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, PEPPER_RUN_JUMP_UP_EventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_PEPPER_RUN_JUMP_DOWN;
	m_LuaManager.MakeTableReference( L"ESI_PEPPER_RUN_JUMP_DOWN", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, PEPPER_RUN_JUMP_DOWN_StateStartFuture );
	stateData.OnFrameMoveFuture	= SET_CB_FUNC( CX2GUUser, PEPPER_RUN_JUMP_DOWN_FrameMoveFuture );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, PEPPER_RUN_JUMP_DOWN_EventProcess );
	m_StateList[stateData.stateID] = stateData;

#ifdef RIDING_SYSTEM
	stateData.Init();
	stateData.stateID			= GetRidingStartStateID();
	m_LuaManager.MakeTableReference( L"ESI_RIDING_START", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, RidingStartStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingStartStart );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingStartEventProcess );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUUser, StartEnd );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_ON;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_ON", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, RidingOnStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingOnStart );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingOnEventProcess );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUUser, RidingOnEnd );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_OFF;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_OFF", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingOffEventProcess );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUUser, RidingOffEnd );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_WAIT_HABIT;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_WAIT_HABIT", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, RidingWaitHabitStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingWaitHabitStart );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingWaitHabitEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_WAIT;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_WAIT", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, RidingWaitStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingWaitStart );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingWaitEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_WALK;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_WALK", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, RidingWalkStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingWalkStart );
	stateData.OnFrameMoveFuture	= SET_CB_FUNC( CX2GUUser, WalkFrameMoveFuture );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingWalkEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_JUMP_UP;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_JUMP_UP", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, RidingJumpUpStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingJumpUpStart );
	stateData.OnFrameMoveFuture	= SET_CB_FUNC( CX2GUUser, JumpFrameMoveFuture );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingJumpUpEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_JUMP_DOWN;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_JUMP_DOWN", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, RidingJumpDownStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingJumpDownStart );
	stateData.OnFrameMoveFuture	= SET_CB_FUNC( CX2GUUser, JumpFrameMoveFuture );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingJumpDownEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_JUMP_LANDING;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_JUMP_LANDING", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, RidingJumpLandingStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingJumpLandingStart );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingJumpLandingEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_DASH;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_DASH", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, DashStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingDashStart );
	stateData.OnFrameMoveFuture	= SET_CB_FUNC( CX2GUUser, DashFrameMoveFuture );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingDashEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_DASH_END;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_DASH_END", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, RidingDashEndStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingDashEndStart );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, RidingDashEndStartFuture );
	stateData.OnFrameMove		= SET_CB_FUNC( CX2GUUser, RidingDashEndFrameMove );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingDashEndEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_DASH_JUMP;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_DASH_JUMP", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, RidingDashJumpStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingDashJumpStart );
	stateData.OnFrameMoveFuture	= SET_CB_FUNC( CX2GUUser, RidingDashJumpFrameMoveFuture );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingDashJumpEventProcess );
	stateData.StateEndFuture	= SET_CB_FUNC( CX2GUUser, RidingDashJumpEndFuture );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_DASH_JUMP_LANDING;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_DASH_JUMP_LANDING", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, DashJumpLandingStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingDashJumpLandingStart );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingDashJumpLandingEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_DAMAGE_FRONT;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_DAMAGE_FRONT", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, RidingDamageFrontStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingDamageFrontStart );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingDamageFrontEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_DAMAGE_BACK;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_DAMAGE_BACK", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, RidingDamageBackStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingDamageBackStart );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingDamageBackEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_DIE;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_DIE", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUEL, DieFrontStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUEL, DieFrontStart );
	stateData.OnFrameMove		= SET_CB_FUNC( CX2GUEL, DieFrontFrameMove );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DieFrontEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_ATTACK_Z;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_ATTACK_Z", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, RidingAttackZStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingAttackZStart );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingAttackZEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_JUMP_ATTACK_Z;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_JUMP_ATTACK_Z", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, RidingJumpAttackZStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingJumpAttackZStart );
	stateData.OnFrameMoveFuture	= SET_CB_FUNC( CX2GUUser, RidingJumpAttackZFrameMoveFuture );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingJumpAttackZEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_ATTACK_X;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_ATTACK_X", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, RidingAttackXStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingAttackXStart );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingAttackXEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_SPECIAL_ATTACK;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_ATTACK_SPECIAL", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, RidingSpecialAttackStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingSpecialAttackStart );
#ifdef FIX_SCORPION_SPECIAL_SKILL_BUG
	stateData.OnFrameMove		= SET_CB_FUNC( CX2GUUser, RidingSpecialAttackFrameMove );
#endif // FIX_SCORPION_SPECIAL_SKILL_BUG
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingSpecialAttackEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= USI_RIDING_SPECIAL_MOVE;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_SPECIAL_MOVE", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, RidingSpecialMoveStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingSpecialMoveStart );
	stateData.OnFrameMoveFuture	= SET_CB_FUNC( CX2GUUser, RidingSpecialMoveFrameMoveFuture );
	stateData.OnFrameMove		= SET_CB_FUNC( CX2GUUser, RidingSpecialMoveFrameMove );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingSpecialMoveEventProcess );
	stateData.StateEndFuture	= SET_CB_FUNC( CX2GUUser, RidingSpecialMoveEndFutrue );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUUser, RidingSpecialMoveEnd );
	m_StateList[stateData.stateID] = stateData;
#endif //RIDING_SYSTEM

#ifdef MODIFY_RIDING_PET_AWAKE
	stateData.Init();
	stateData.stateID			= USI_RIDING_HYPER_MODE;
	m_LuaManager.MakeTableReference( L"ESI_RIDING_HYPER_MODE", stateData.stateID );
	stateData.StateStart		= SET_CB_FUNC( CX2GUUser, RidingHyperModeStart );
	stateData.OnFrameMove		= SET_CB_FUNC( CX2GUEL, RidingHyperModeFrameMove );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, RidingHyperModeEventProcess );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUUser, RidingHyperModeStartFuture );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUUser, RidingHyperModeEnd );
	m_StateList[stateData.stateID] = stateData;
#endif // MODIFY_RIDING_PET_AWAKE

	stateData.Init();
	stateData.stateID			= ESI_STANDUP_ROLLING_FRONT_FRONT;
	m_LuaManager.MakeTableReference( L"ESI_STANDUP_ROLLING_FRONT_FRONT", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, StandUpRollingFrontFrontEventProcess );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUEL, StandUpRollingFrontFrontEnd );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_STANDUP_ROLLING_FRONT_BACK;
	m_LuaManager.MakeTableReference( L"ESI_STANDUP_ROLLING_FRONT_BACK", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, StandUpRollingFrontBackEventProcess );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUEL, StandUpRollingFrontBackEnd );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_STANDUP_ROLLING_BACK_FRONT;
	m_LuaManager.MakeTableReference( L"ESI_STANDUP_ROLLING_BACK_FRONT", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, StandUpRollingBackFrontEventProcess );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUEL, StandUpRollingBackFrontEnd );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_STANDUP_ROLLING_BACK_BACK;
	m_LuaManager.MakeTableReference( L"ESI_STANDUP_ROLLING_BACK_BACK", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, StandUpRollingBackBackEventProcess );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUEL, StandUpRollingBackBackEnd );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_STANDUP_ATTACK_FRONT;
	m_LuaManager.MakeTableReference( L"ESI_STANDUP_ATTACK_FRONT", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, StandUpAttackFrontEventProcess );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUEL, StandUpAttackFrontEnd );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_STANDUP_ATTACK_BACK;
	m_LuaManager.MakeTableReference( L"ESI_STANDUP_ATTACK_BACK", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, StandUpAttackBackEventProcess );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUEL, StandUpAttackBackEnd );
	stateData.StateEndFuture	= SET_CB_FUNC( CX2GUEL, StandUpAttackBackStateEndFuture );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_STANDUP_ATTACK_FRONT_NO_DOWN;
	m_LuaManager.MakeTableReference( L"ESI_STANDUP_ATTACK_FRONT_NO_DOWN", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, StandUpAttackFrontNoDownEventProcess );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUEL, StandUpAttackFrontNoDownEnd );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_STANDUP_ATTACK_BACK_NO_DOWN;
	m_LuaManager.MakeTableReference( L"ESI_STANDUP_ATTACK_BACK_NO_DOWN", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, StandUpAttackBackNoDownEventProcess );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUEL, StandUpAttackBackNoDownEnd );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_DASH_JUMP_POWER_LANDING;
	m_LuaManager.MakeTableReference( L"ESI_DASH_JUMP_POWER_LANDING", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUEL, DashJumpPowerLandingStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUEL, DashJumpPowerLandingStart );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DashJumpPowerLandingEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_WALL_LANDING;
	m_LuaManager.MakeTableReference( L"ESI_WALL_LANDING", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUUser, WallLandingEventProcess );
	stateData.StateEndFuture	= SET_CB_FUNC( CX2GUUser, WallLandingEndFuture );
	m_StateList[stateData.stateID] = stateData;

//=====================================================================================
// 공격 스테이트

	stateData.Init();
	stateData.stateID			= ESI_COMBO_Z;
	m_LuaManager.MakeTableReference( L"ESI_COMBO_Z", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ComboZEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_COMBO_ZZ;
	m_LuaManager.MakeTableReference( L"ESI_COMBO_ZZ", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ComboZZEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_COMBO_ZZZ;
	m_LuaManager.MakeTableReference( L"ESI_COMBO_ZZZ", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ComboZZZEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_COMBO_ZZZZ;
	m_LuaManager.MakeTableReference( L"ESI_COMBO_ZZZZ", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ComboZZZZEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_COMBO_ZZZX;
	m_LuaManager.MakeTableReference( L"ESI_COMBO_ZZZX", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ComboZZZXEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_COMBO_X;
	m_LuaManager.MakeTableReference( L"ESI_COMBO_X", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ComboXEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_COMBO_XX;
	m_LuaManager.MakeTableReference( L"ESI_COMBO_XX", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ComboXXEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_COMBO_XXX;
	m_LuaManager.MakeTableReference( L"ESI_COMBO_XXX", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ComboXXXEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_COMBO_XXZ;
	m_LuaManager.MakeTableReference( L"ESI_COMBO_XXZ", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ComboXXZEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_JUMP_ATTACK_Z;
	m_LuaManager.MakeTableReference( L"ESI_JUMP_ATTACK_Z", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, JumpAttackZEventProcess );
	stateData.OnFrameMoveFuture	= SET_CB_FUNC( CX2GUEL, JumpAttackZFrameMoveFuture );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_JUMP_ATTACK_X;
	m_LuaManager.MakeTableReference( L"ESI_JUMP_ATTACK_X", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, JumpAttackXEventProcess );
	stateData.OnFrameMoveFuture	= SET_CB_FUNC( CX2GUEL, JumpAttackXFrameMoveFuture );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_DASH_COMBO_Z;
	m_LuaManager.MakeTableReference( L"ESI_DASH_COMBO_Z", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DashComboZEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_DASH_COMBO_ZZ;
	m_LuaManager.MakeTableReference( L"ESI_DASH_COMBO_ZZ", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DashComboZZEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_DASH_COMBO_X;
	m_LuaManager.MakeTableReference( L"ESI_DASH_COMBO_X", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DashComboXEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_DASH_JUMP_COMBO_Z;
	m_LuaManager.MakeTableReference( L"ESI_DASH_JUMP_COMBO_Z", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DashJumpComboZEventProcess );
	m_StateList[stateData.stateID] = stateData;


	stateData.Init();
	stateData.stateID			= ESI_DASH_JUMP_COMBO_ZZ;
	m_LuaManager.MakeTableReference( L"ESI_DASH_JUMP_COMBO_ZZ", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DashJumpComboZZEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_DASH_JUMP_COMBO_ZZZ;
	m_LuaManager.MakeTableReference( L"ESI_DASH_JUMP_COMBO_ZZZ", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DashJumpComboZZZEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_DASH_JUMP_COMBO_X;
	m_LuaManager.MakeTableReference( L"ESI_DASH_JUMP_COMBO_X", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DashJumpComboXEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_DASH_JUMP_COMBO_XX;
	m_LuaManager.MakeTableReference( L"ESI_DASH_JUMP_COMBO_XX", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DashJumpComboXXEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_DASH_JUMP_COMBO_XX_LANDING;
	m_LuaManager.MakeTableReference( L"ESI_DASH_JUMP_COMBO_XX_LANDING", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, DashJumpComboXXLandingEventProcess );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_QUICK_STANDUP_ATTACK_FRONT;
	m_LuaManager.MakeTableReference( L"ESI_QUICK_STANDUP_ATTACK_FRONT", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, StandUpAttackFrontEventProcess );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUEL, StandUpAttackFrontEnd );
	m_StateList[stateData.stateID] = stateData;

	stateData.Init();
	stateData.stateID			= ESI_QUICK_STANDUP_ATTACK_BACK;
	m_LuaManager.MakeTableReference( L"ESI_QUICK_STANDUP_ATTACK_BACK", stateData.stateID );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, StandUpAttackBackEventProcess );
	stateData.StateEnd			= SET_CB_FUNC( CX2GUEL, StandUpAttackBackEnd );
	stateData.StateEndFuture	= SET_CB_FUNC( CX2GUEL, StandUpAttackBackStateEndFuture );
	m_StateList[stateData.stateID] = stateData;


#ifdef ELSWORD_WAY_OF_SWORD // 검의 길
	stateData.Init();
	stateData.stateID			= ESI_SWORD_WAIT;
	m_LuaManager.MakeTableReference( L"ESI_SWORD_WAIT", stateData.stateID );
	stateData.StateStartFuture	= SET_CB_FUNC( CX2GUEL, SwordWaitStartFuture );
	stateData.StateStart		= SET_CB_FUNC( CX2GUEL, SwordWaitStart );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, SwordWaitEventProcess );
	m_StateList[stateData.stateID] = stateData;
#endif // ELSWORD_WAY_OF_SWORD

	// 극기 - 강(强)
	// 섬멸 기폭에서도 사용하기 위해 스킬 구분 없이 무조건 스테이트 로딩하도록 변경
	stateData.Init();
	stateData.stateID			= ESI_SA_A_EL_ENDURANCE_POWER_ACTIVE;
	m_LuaManager.MakeTableReference( L"ESI_SA_A_EL_ENDURANCE_POWER_ACTIVE", stateData.stateID );
	stateData.StateStart		= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_ENDURANCE_POWER_ACTIVE_StateStart );
	stateData.StateInit			= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_ENDURANCE_POWER_ACTIVE_Init );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_SI_A_EL_ENDURANCE_POWER_ACTIVE_EventProcess );
	m_StateList[stateData.stateID] = stateData;

#ifdef ADD_CAN_NOT_ATTACK_JUMP_DOWN
	stateData.Init();
	stateData.stateID			= ESI_CAN_NOT_ATTACK_JUMP_DOWN;
	m_LuaManager.MakeTableReference( L"ESI_CAN_NOT_ATTACK_JUMP_DOWN", stateData.stateID );
	stateData.OnFrameMoveFuture	= SET_CB_FUNC( CX2GUUser, JumpFrameMoveFuture );
	stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_CAN_NOT_ATTACK_JUMP_DOWN_EventProcess );	
	m_StateList[stateData.stateID] = stateData;
#endif // ADD_CAN_NOT_ATTACK_JUMP_DOWN
}

void CX2GUEL::InitStateByUnitClass()
{
	UserUnitStateData stateData;
	switch ( GetUnitClass() )
	{
	case CX2Unit::UC_ELESIS_KNIGHT:
		{

		} break;
	case CX2Unit::UC_ELESIS_SABER_KNIGHT:
		{
			stateData.Init();
			stateData.stateID			= ESI_DASH_COMBO_SSK_ZZ;
			m_LuaManager.MakeTableReference( L"ESI_DASH_COMBO_SSK_ZZ", stateData.stateID );
			stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_DASH_COMBO_SSK_ZZ_EventProcess );
			m_StateList[stateData.stateID] = stateData;

			stateData.Init();
			stateData.stateID			= ESI_DASH_COMBO_SSK_ZZZ;
			m_LuaManager.MakeTableReference( L"ESI_DASH_COMBO_SSK_ZZZ", stateData.stateID );
			stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_DASH_COMBO_SSK_ZZZ_EventProcess );
			m_StateList[stateData.stateID] = stateData;

			stateData.Init();
			stateData.stateID			= ESI_DASH_COMBO_SSK_ZZX;
			m_LuaManager.MakeTableReference( L"ESI_DASH_COMBO_SSK_ZZX", stateData.stateID );
			stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_DASH_COMBO_SSK_ZZX_EventProcess );
			m_StateList[stateData.stateID] = stateData;
			
			stateData.Init();
			stateData.stateID			= ESI_DASH_JUMP_COMBO_SSK_XX;
			m_LuaManager.MakeTableReference( L"ESI_DASH_JUMP_COMBO_SSK_XX", stateData.stateID );
			stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_DASH_JUMP_COMBO_SSK_XX_EventProcess );
			m_StateList[stateData.stateID] = stateData;

			stateData.Init();
			stateData.stateID			= ESI_DASH_JUMP_COMBO_SSK_XXX;
			m_LuaManager.MakeTableReference( L"ESI_DASH_JUMP_COMBO_SSK_XXX", stateData.stateID );
			stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_DASH_JUMP_COMBO_SSK_XXX_EventProcess );
			m_StateList[stateData.stateID] = stateData;

		} break;
	case CX2Unit::UC_ELESIS_PYRO_KNIGHT:
		{
			stateData.Init();
			stateData.stateID			= ESI_DASH_COMBO_SPK_ZX;
			m_LuaManager.MakeTableReference( L"ESI_DASH_COMBO_SPK_ZX", stateData.stateID );
			stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_DASH_COMBO_SPK_ZX_EventProcess );
			stateData.OnFrameMove		= SET_CB_FUNC( CX2GUEL, ESI_DASH_COMBO_SPK_ZX_FrameMove );
			m_StateList[stateData.stateID] = stateData;
			
			stateData.Init();
			stateData.stateID			= ESI_DASH_COMBO_SPK_ZZ;
			m_LuaManager.MakeTableReference( L"ESI_DASH_COMBO_SPK_ZZ", stateData.stateID );
			stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_DASH_COMBO_SPK_ZZ_EventProcess );
			m_StateList[stateData.stateID] = stateData;

			stateData.Init();
			stateData.stateID			= ESI_DASH_COMBO_SPK_ZZZ;
			m_LuaManager.MakeTableReference( L"ESI_DASH_COMBO_SPK_ZZZ", stateData.stateID );
			stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_DASH_COMBO_SPK_ZZZ_EventProcess );
			stateData.OnFrameMove		= SET_CB_FUNC( CX2GUEL, ESI_DASH_COMBO_SPK_ZZZ_FrameMove );
			m_StateList[stateData.stateID] = stateData;

			stateData.Init();
			stateData.stateID			= ESI_DASH_JUMP_COMBO_SPK_XZ;
			m_LuaManager.MakeTableReference( L"ESI_DASH_JUMP_COMBO_SPK_XZ", stateData.stateID );
			stateData.StateStartFuture	= SET_CB_FUNC( CX2GUEL, ESI_DASH_JUMP_COMBO_SPK_XZ_StartFurture );
			stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_DASH_JUMP_COMBO_SPK_XZ_EventProcess );
			stateData.OnFrameMove		= SET_CB_FUNC( CX2GUEL, ESI_DASH_JUMP_COMBO_SPK_XZ_FrameMove );
			m_StateList[stateData.stateID] = stateData;


			stateData.Init();
			stateData.stateID			= ESI_DASH_JUMP_COMBO_SPK_XZ_REATTACK;
			m_LuaManager.MakeTableReference( L"ESI_DASH_JUMP_COMBO_SPK_XZ", stateData.stateID );
			stateData.StateStartFuture	= SET_CB_FUNC( CX2GUEL, ESI_DASH_JUMP_COMBO_SPK_XZ_REATTACK_StartFurture );
			stateData.OnEventProcess	= SET_CB_FUNC( CX2GUEL, ESI_DASH_JUMP_COMBO_SPK_XZ_EventProcess );
			stateData.OnFrameMove		= SET_CB_FUNC( CX2GUEL, ESI_DASH_JUMP_COMBO_SPK_XZ_FrameMove );
			m_StateList[stateData.stateID] = stateData;
		} break;
	default:
		ASSERT(!"GetUnitClass Error");
		break;
	}
}


void CX2GUEL::SetEquippedSkillLevelStateData( const CX2SkillTree::SkillTemplet* pSkillTemplet_, EL_STATE_ID eStateID_ )
{
	if ( NULL != pSkillTemplet_ )
		SetSkillLevelStateData( pSkillTemplet_, m_StateList[eStateID_] );
}

void CX2GUEL::SetSkillLevelStateData( const CX2SkillTree::SkillTemplet* pSkillTemplet_, UserUnitStateData& stateData_ )
{
	if( NULL == pSkillTemplet_ )
		return;

	if ( NULL == GetUnit() || NULL == GetUnit()->GetUnitData() )
		return;

	const CX2UserSkillTree& userSkillTree = GetUnit()->GetUnitData()->m_UserSkillTree;
	const int iSkillTempletLevel = max( 1, userSkillTree.GetSkillLevel( pSkillTemplet_->m_eID ) );	/// 스킬 레벨

	stateData_.m_SPLevel	= iSkillTempletLevel;
	stateData_.m_fPowerRate	= pSkillTemplet_->GetSkillPowerRateValue( iSkillTempletLevel );
	stateData_.m_eSkillID	= pSkillTemplet_->m_eID;
}
void CX2GUEL::ShowActiveSkillCutInAndLight( float fTimeToShow_, bool bOnlyLight_ /*= false*/ )
{
	if( m_pXSkinAnim->EventTimer( fTimeToShow_ ) == true && EventCheck( fTimeToShow_, false ) == true )
	{
		if ( GetShowCutInAndChangeWorldColor() && GetShowActiveSkillShow() )
		{
			ActiveSkillShow( L"Dummy2_Lhand", s_SkillCutInSet[m_iSkillCutInSetIndex].cutIn[0].fileName.c_str(),
				s_SkillCutInSet[m_iSkillCutInSetIndex].cutIn[0].vSize,
				s_SkillCutInSet[m_iSkillCutInSetIndex].cutIn[0].vPosition, IsHyperState(), bOnlyLight_ );
		}
	}	
}
/*virtual*/ void CX2GUEL::ShowActiveSkillCutInAndLightByScript( float fTimeToShow_, bool bOnlyLight_ )
{
	ShowActiveSkillCutInAndLight( fTimeToShow_, bOnlyLight_ );
}
bool CX2GUEL::CanUseSpecialAttack( CX2SkillTree::ACTIVE_SKILL_USE_CONDITION eActiveSkillUseCondition/* = CX2SkillTree::ASUT_GROUND*/ )
{
	KTDXPROFILE();

	if( NULL != m_pUnit && NULL != m_pUnit->GetUnitData() )
	{
		CX2UserSkillTree& cUserSkillTree = m_pUnit->GetUnitData()->m_UserSkillTree;	// 유저가 배운 스킬 트리

		m_iPressedSkillSlotIndex = -1;
		const CX2UserSkillTree::SkillSlotData* pSkillSlotData = NULL;

		// 사용한 스킬의 스킬슬롯 데이터와 그 인덱스를 얻어옴
		if ( false == CommonSpecialAttackEventProcess( cUserSkillTree, pSkillSlotData, m_iPressedSkillSlotIndex ) )
			return false;

		if( NULL == pSkillSlotData )
			return false;

		// 사용한 스킬
		const CX2SkillTree::SkillTemplet* pSkillTemplet 
			= GetEquippedActiveSkillTemplet( pSkillSlotData->m_eID );

		if( NULL == pSkillTemplet )
			return false;

		const int iSkillTempletLevel = max( 1, cUserSkillTree.GetSkillLevel( pSkillTemplet->m_eID ) );	/// 스킬 레벨

		// 스킬을 사용할 수 있는 조건인지 판단( ex: 공중에서는 사용 할 수 없는 필살기 등)
		if( false == CheckSkillUseCondition( eActiveSkillUseCondition, pSkillTemplet ) )
			return false;

		//소환 필살기 종류는 게임이 끝난 상태에서 사용할 수 없다.
		if( false == CheckSummonSpecialAttack( pSkillTemplet->m_eID ) )
			return false;

		// 공식대전에서 사용 가능한 스킬인지 검사
		if( false == CheckSkillUsePVPOfficial( pSkillTemplet->m_eID ) )
			return false;

#ifdef FIX_NO_STATE_SKILL_BUG
		// 탈 것 탑승 상태에서 사용 가능한 스킬인지 검사
		if( false == IsValideSkillRidingOn( pSkillTemplet->m_eID)  )
			return false;
#endif // FIX_NO_STATE_SKILL_BUG

		// 엠피 소모량을 얻어옴
		m_fMPConsume = GetActualMPConsume( pSkillTemplet->m_eID, iSkillTempletLevel );

		float fCoolTimeLeft = pSkillSlotData->m_fCoolTimeLeft;

		bool bSlotB = (m_iPressedSkillSlotIndex > 3) ? true : false;
		int iSlotIndex = (m_iPressedSkillSlotIndex > 3) ? m_iPressedSkillSlotIndex-4 : m_iPressedSkillSlotIndex;

		//오현빈//2013-01-23//임시로 사내서버에서만 스킬 사용 조건 무시하도록 설정
#ifdef _IN_HOUSE_
		if( false == g_pMain->IsMyAuthLevelHigherThan( CX2User::XUAL_DEV ) )
#endif //_IN_HOUSE_
			//#endif _SERVICE_
		{
			if( 0.f < fCoolTimeLeft )
			{
				g_pX2Game->GetInfoTextManager().PushText( XUF_DODUM_20_BOLD, GET_STRING( STR_ID_226 ), D3DXCOLOR(1,1,1,1),
					D3DXCOLOR(0,0,0,1), DT_CENTER, 1.f, 1.f );
				return false;
			}

			if ( GetNowMp() < m_fMPConsume )
			{
				g_pX2Game->GetInfoTextManager().PushText( XUF_DODUM_20_BOLD, GET_STRING( STR_ID_2549 ), D3DXCOLOR(1,1,1,1),
					D3DXCOLOR(0,0,0,1), DT_CENTER, 1.f, 1.f );
				return false;
			}
		}

		if( NULL == pSkillTemplet )		/// pSkillTemplet을 위에서 새로 대입하기 때문에, NULL처리 추가
			return false;

		m_pSkillTemplet = pSkillTemplet;

		return true;
	}
	else
	{
		return false;
	}

	return false;
}

void CX2GUEL::StateChangeSpecialAttack( IN const CX2SkillTree::SkillTemplet* pSkillTemplet_ )
{
	if( NULL != m_pUnit && NULL != m_pUnit->GetUnitData() )
	{
		CX2UserSkillTree& cUserSkillTree =  m_pUnit->GetUnitData()->m_UserSkillTree;
		
		CountUsedSkill( m_iPressedSkillSlotIndex );
		UpNowMp( -m_fMPConsume );

		const int iSkillTempletLevel = max( 1, cUserSkillTree.GetSkillLevel( pSkillTemplet_->m_eID ) );	/// 스킬 레벨
		cUserSkillTree.SetSkillCoolTimeLeft( pSkillTemplet_->m_eID, GetActualCoolTime( pSkillTemplet_, iSkillTempletLevel) );

		if( false == SpecialAttackNoStageChange( pSkillTemplet_ ) )
		{
			if( GetRemainHyperModeTime() <= 0.f )
			{
				if( m_iPressedSkillSlotIndex > 3 ) //슬롯 B일때
				{
					StateChange( _CONST_GUUSER_::aNormalSkillStateIDSlotB[m_iPressedSkillSlotIndex-4] );
				}
				else
				{
					StateChange( _CONST_GUUSER_::aNormalSkillStateID[m_iPressedSkillSlotIndex] );
				}
			}
			else
			{
				if( m_iPressedSkillSlotIndex > 3 ) //슬롯 B일때
				{
					StateChange( _CONST_GUUSER_::aHyperSkillStateIDSlotB[m_iPressedSkillSlotIndex-4] );
				}
				else
				{
					StateChange( _CONST_GUUSER_::aHyperSkillStateID[m_iPressedSkillSlotIndex] );
				}
			}
		}

		m_iNowSpecialAttack = m_iPressedSkillSlotIndex + 1;
		m_bSpecialAttackEventProcessedAtThisFrame = true;
	}

}


#ifdef FIX_NO_STATE_SKILL_BUG
/** @function : IsValideSkillRidingOn
	@brief : 탈 것 탑승중에 사용 할 수 있는 스킬인지 체크.
	@return : 사용 가능 : true 사용 불가 falle
*/
/*virtual*/ bool CX2GUEL::IsValideSkillRidingOn( CX2SkillTree::SKILL_ID eSkill_ID_ )
{
	if( true == GetRidingOn() )
	{
		switch( eSkill_ID_ ) 
		{
		case CX2SkillTree::SI_A_EL_ENDURANCE_POWER: // 극기 - 강(强)
			{
				return false;
			} break;
		default:
			return true;
			break;
		}
	}
	else
	{
		return true;
	}
}
#endif // FIX_NO_STATE_SKILL_BUG

/*virtual*/ void CX2GUEL::ReInit( bool bTeamPos /*= false*/, int iStartPosIndex /*= -1*/ )
{
	CX2GUUser::ReInit( bTeamPos, iStartPosIndex );
#ifdef ELSWORD_WAY_OF_SWORD // 검의 길
	// 대전 중 지고 있다면 검의길 효과 초기화 
	if( true == IsMyUnit() )
	{
		if( CX2Game::GT_PVP == g_pX2Game->GetGameType() )
		{
			const WAY_OF_SWORD_STATE eWayOfSwordState = static_cast<const WAY_OF_SWORD_STATE>( GetWayOfSwordState() );

			if( GetTeam() == CX2Room::TN_RED && g_pX2Game->GetRedTeamKill() > g_pX2Game->GetBlueTeamKill() ||
				GetTeam() == CX2Room::TN_BLUE && g_pX2Game->GetRedTeamKill() < g_pX2Game->GetBlueTeamKill() )
			{
				if( eWayOfSwordState == WSS_VIGOR )
				{
					End_SwordmanVigor();
				}
				else if( eWayOfSwordState == WSS_DESTRUCTION )
				{
					End_SwordmanDestruction();
				}
				SetWayOfSwordPoint( 0 );
				SetWayOfSwordState( static_cast<int>( WSS_CENTER ) );
			}
			else
			{
				if( eWayOfSwordState == WSS_VIGOR )
				{
					End_SwordmanVigor();
					SetWayOfSwordState( static_cast<int>( WSS_TOWARD_VIGOR ) );
				}
				else if( eWayOfSwordState == WSS_DESTRUCTION )
				{
					End_SwordmanDestruction();
					SetWayOfSwordState( static_cast<int>( WSS_TOWARD_DESTRUCTION ) );
				}
			}
			m_FrameDataFuture.syncData.m_CannonBallCount = GetWayOfSwordState();
		}
		else
		{
			switch ( GetWayOfSwordState() )
			{
			case WSS_VIGOR:
				{
					Start_SwordmanVigor();
				} break;
			case WSS_DESTRUCTION:
				{
					Start_SwordmanDestruction();
				} break;
			default:
				break;
			}
		}
	}
#endif // ELSWORD_WAY_OF_SWORD

}
/*virtual*/ void CX2GUEL::DamageReact( OUT CX2DamageManager::DamageData* pDamageData_ )
{
	KTDXPROFILE();

	if( WSS_DESTRUCTION == GetWayOfSwordState() && // 섬멸 상태 일 때 기폭 예외처리
		true == IsDamagedReaction( pDamageData_ ) &&
		true == DamageFlushMp() )
	{
		// 섬멸 기폭 시 Damage를 입지 않도록 하기 위해 DamageReact 타지 않도록 예외처리
		DetonationStateChange( pDamageData_ );
	}
	// 극기 - 강(强)
	else if( true == m_bIsActiveEndurancePower &&
			 true == CanSpecialReactProcess( pDamageData_->reActType ))
	{
		DoStateEndurancePowerRage();
	}
	else
	{
		CX2GUUser::DamageReact( pDamageData_ );
		CX2GUUser::DamageReactStateChange( pDamageData_ );
	}


#ifdef ELSWORD_WAY_OF_SWORD // 검의 길
	if(	IsMyUnit() == true &&
		pDamageData_->reActResult != CX2DamageManager::RT_REVENGE &&
		true == CanSpecialReactProcess(pDamageData_->reActType) )
	{
		const int iWayOfSwordState = GetWayOfSwordState();
		if( WSS_VIGOR != iWayOfSwordState )
		{ // 엘리 검의길은 질풍 상태에서 소모시키지 않음.
			const float fWSPGain = 
				g_pData->GetDamageManager()->CalcWayOfSwordPointOnDamageReact( pDamageData_->damage.GetAllDamage(), iWayOfSwordState );

			SetWayOfSwordPoint( GetWayOfSwordPoint() + fWSPGain );
		}
	}
#endif //ELSWORD_WAY_OF_SWORD
}
/*virtual*/ void CX2GUEL::Win()
{
	CX2GUUser::Win();
	StateChange( GetWinStateID() );
}
/*virtual*/ void CX2GUEL::Lose()
{
	CX2GUUser::Lose();

	if ( true == IsMyUnit() && 0.f < GetNowHp() )
	{
		StateChange( GetLoseStateID() );
	}
}
/*virtual*/ void CX2GUEL::Draw()
{
	CX2GUUser::Draw();
	StateChange( GetWinStateID() );
}
/*virtual*/ void CX2GUEL::InitEffect()
{
	CX2GUUser::InitEffect();

	CreateMajorParticleArray();
	CreateMinorParticleArray();
}
/*virtual*/ void CX2GUEL::InitEnchantWeaponEffectUnitSpecific()
{
	BOOST_TEST_FOREACH( Weapon*, pWeapon, m_vecpWeapon )
	{
		ASSERT( NULL != pWeapon );
		pWeapon->InitEnchantParticleForSword();
	}
}
/*virtual*/ void CX2GUEL::CreateMajorParticleArray()
{
	CX2GUUser::CreateMajorParticleArray();
	CKTDGParticleSystem::CParticleEventSequence* pSeq = NULL;

	// small damage 타격을 줬을 때 나오는 파티클
	pSeq = SetCommonMajorParticleByEnum( COMMON_MAJOR_PII_DAMAGE_IMPACT,			L"DamageImpact",			2 );
	if( pSeq != NULL )
		pSeq->SetTriggerWait( true );

	pSeq = SetCommonMajorParticleByEnum( COMMON_MAJOR_PII_DAMAGE_IMPACT_CORE,		L"DamageImpactCore",		2 );
	if( pSeq != NULL )
		pSeq->SetTriggerWait( true );

	pSeq = SetCommonMajorParticleByEnum( COMMON_MAJOR_PII_DAMAGE_IMPACT_SLASH,		L"DamageImpactSlash",		2 );
	if( pSeq != NULL )
		pSeq->SetTriggerWait( true );

	// AttackResult의 switch case에서 default:로 처리되는 타격에 대한 파티클 (현재는 small 이외의 파티클)
	pSeq = SetCommonMajorParticleByEnum( COMMON_MAJOR_PII_DAMAGE_IMPACT_RED,		L"DamageImpactRed",			2 );
	if( pSeq != NULL )
		pSeq->SetTriggerWait( true );

	pSeq = SetCommonMajorParticleByEnum( COMMON_MAJOR_PII_DAMAGE_IMPACT_RING_RED,	L"DamageImpactRingRed",		1 );
	if( pSeq != NULL )
		pSeq->SetTriggerWait( true );

	pSeq = SetCommonMajorParticleByEnum( COMMON_MAJOR_PII_DAMAGE_IMPACT_CORE_RED,	L"DamageImpactCoreRed",		1 );
	if( pSeq != NULL )
		pSeq->SetTriggerWait( true );

	pSeq = SetCommonMajorParticleByEnum( COMMON_MAJOR_PII_DAMAGE_IMPACT_SLASH_RED,	L"DamageImpactSlashRed",	1 );
	if( pSeq != NULL )
		pSeq->SetTriggerWait( true );
}
/*virtual*/ void CX2GUEL::CreateMinorParticleArray()
{
	CX2GUUser::CreateMinorParticleArray();
}
/*static*/ void CX2GUEL::AppendMajorParticleToDeviceList( CKTDXDeviceDataList& listInOut_ )
{
	if ( g_pData->GetGameMajorParticle() == NULL )
		return;

	g_pData->GetGameMajorParticle()->AppendToDeviceList( listInOut_, L"DamageImpact" );
	g_pData->GetGameMajorParticle()->AppendToDeviceList( listInOut_, L"DamageImpactCore" );
	g_pData->GetGameMajorParticle()->AppendToDeviceList( listInOut_, L"DamageImpactSlash" );
	g_pData->GetGameMajorParticle()->AppendToDeviceList( listInOut_, L"DamageImpactRed" );
	g_pData->GetGameMajorParticle()->AppendToDeviceList( listInOut_, L"DamageImpactRingRed" );
	g_pData->GetGameMajorParticle()->AppendToDeviceList( listInOut_, L"DamageImpactCoreRed" );
	g_pData->GetGameMajorParticle()->AppendToDeviceList( listInOut_, L"DamageImpactSlashRed" );
}
/*static*/ void CX2GUEL::AppendMinorParticleToDeviceList( CKTDXDeviceDataList& listInOut_ )
{

}
/*virtual*/ bool CX2GUEL::SpecialAttackNoStageChange( IN const CX2SkillTree::SkillTemplet* pSkillTemplet_ )
{
	if( NULL == pSkillTemplet_ )
		return false;

	switch( pSkillTemplet_->m_eID )
	{
	case CX2SkillTree::SI_A_EL_ENDURANCE_POWER:
		{
			m_FrameDataFuture.syncData.nowAction = UAI_ENDURANCE_POWER;
		} break;
	default:
		return false;
		break;
	}
	return true;
}
/*virtual*/ void CX2GUEL::COMMON_BUFF_FrameMove()
{
	if( true == m_pXSkinAnim->EventTimer( 0.2f ) && true == EventCheck( 0.2f, false ) )
	{
		PlayCommonBuffMinorParticle();
		UpDownCrashCamera( 30.0f, 0.4f );
		g_pKTDXApp->GetDGManager()->ClearScreen();

	}
	else if( true == m_pXSkinAnim->EventTimer( 0.25f ) && true == EventCheck( 0.25f, false ) )
	{
		int	iSkillSlotIndex = 0;
		bool bSlotB = false;

		if ( true == GetSkillSlotIndexUsed( iSkillSlotIndex, bSlotB ) )
		{
			CX2Unit::UnitData* pUnitData = m_pUnit->GetUnitData();

			const CX2UserSkillTree::SkillSlotData* pSkillSlotData = pUnitData->m_UserSkillTree.GetSkillSlot( iSkillSlotIndex, bSlotB );
			CX2SkillTree::SKILL_ID eSkillID = CX2SkillTree::SI_NONE;

			if( NULL != pSkillSlotData )
				eSkillID = pSkillSlotData->m_eID;

			ProcessAcceleraterBuff( eSkillID, IsHyperState() );
		}
	}

	CommonFrameMove();
}
/*virtual*/ void CX2GUEL::HyperModeFrameMove()
{
#ifdef MODIFY_RIDING_PET_AWAKE
	CommonHyperModeFrameMove( 0.05f, 0.83f );
#else
	g_pX2Game->GetWorld()->SetWorldColor( 0xff222222 );

#ifdef  X2OPTIMIZE_NPC_ADAPTIVE_FRAME_MOVE
	if( m_pXSkinAnim->EventTimerOneshot( 0.05f ) )
#else   X2OPTIMIZE_NPC_ADAPTIVE_FRAME_MOVE
	if( m_pXSkinAnim->EventTimer( 0.05f ) == true && EventCheck(0.05f, false) == true )
#endif  X2OPTIMIZE_NPC_ADAPTIVE_FRAME_MOVE
	{		
		ShowMinorParticleHyperModeChange();
	}

#ifdef  X2OPTIMIZE_NPC_ADAPTIVE_FRAME_MOVE
	if( m_pXSkinAnim->EventTimerOneshot( 0.83f ) )
#else   X2OPTIMIZE_NPC_ADAPTIVE_FRAME_MOVE
	if( m_pXSkinAnim->EventTimer( 0.83f ) == true && EventCheck(0.83f, false) == true )
#endif  X2OPTIMIZE_NPC_ADAPTIVE_FRAME_MOVE
	{
		ShowMinorParticleHyperModeShock();

		UpDownCrashCamera( 30.0f, 0.4f );
		g_pKTDXApp->GetDGManager()->ClearScreen();

		ShowMinorParticleHyperModeTrace();
		ApplyHyperModeBuff();
	}
	CommonFrameMove();
#endif // MODIFY_RIDING_PET_AWAKE
}
/*virtual*/ void CX2GUEL::HyperModeBuffEffectStart()
{
	CX2GUUser::HyperModeBuffEffectStart();

	if ( INVALID_PARTICLE_HANDLE == m_hHyperBoostRFoot )
		m_hHyperBoostRFoot	= g_pX2Game->GetMinorParticle()->CreateSequenceHandle( (CKTDGObject*) this,  L"HyperBoostRight",	0, 0, 0, 0, 0);

	if ( INVALID_PARTICLE_HANDLE == m_hHyperBoostLFoot )
		m_hHyperBoostLFoot	= g_pX2Game->GetMinorParticle()->CreateSequenceHandle( (CKTDGObject*) this,  L"HyperBoostLeft",	0, 0, 0, 0, 0);

	if ( INVALID_PARTICLE_HANDLE == m_hHyperBoostRArm )
		m_hHyperBoostRArm	= g_pX2Game->GetMinorParticle()->CreateSequenceHandle( (CKTDGObject*) this,  L"HyperBoostRight",	0, 0, 0, 0, 0);

	if ( INVALID_PARTICLE_HANDLE == m_hHyperBoostLArm )
		m_hHyperBoostLArm	= g_pX2Game->GetMinorParticle()->CreateSequenceHandle( (CKTDGObject*) this,  L"HyperBoostLeft",	0, 0, 0, 0, 0);
}
/** @function	: SetSpecificValueByBuffTempletID
	@brief		: 각 유닛마다 특정 버프가 실행 될 때 셋팅해야 하는 함수 실행(ex: 매지컬메이크업이 시전중이라는 플래그 설정 등...)
	@param		: 버프템플릿ID(eBuffTempletId_)
*/
/*virtual*/ void CX2GUEL::SetSpecificValueByBuffTempletID( const BUFF_TEMPLET_ID eBuffTempletId_ )
{
	CX2GUUser::SetSpecificValueByBuffTempletID( eBuffTempletId_ );

 	switch ( eBuffTempletId_ )
 	{
	// 극기 - 강(强)
	case BTI_BUFF_INDURANCE_POWER:
		{
			m_bIsActiveEndurancePower = true;
		} break;
	// 승리의 검
	case BTI_BUFF_VICTORIOUS_SWORD:
		{
			// 닷지&슬래시 쿨타임 감소 비율 적용
			{
				m_fDodgeAndSlashCoolTimeDecreaseRate = 1.f;

				const CX2SkillTree::SKILL_ID eSkillID = CX2SkillTree::SI_SA_SSK_VICTORIOUS_SWORD;
				const int iSkillLevel = GetMyUnitSkillLevel(eSkillID);
				if ( 0 < iSkillLevel )
				{
					const CX2SkillTree::SkillTemplet* pSkillTemplet = GetEquippedActiveSkillTemplet( eSkillID );
					if( NULL != pSkillTemplet )
					{
						// 닷지&슬래시 쿨타임 감소 비율
						m_fDodgeAndSlashCoolTimeDecreaseRate = pSkillTemplet->GetSkillAbilityValue( CX2SkillTree::SA_DECREASE_REL, iSkillLevel );
					}
				}
			}

			// 닷지&슬래시 쿨타임, MP 변경 기능 활성화를 위한 변수 설정
			m_bIsApplyVictoriousSword = true;
		} break;
 	default:
 		break;
 	}
}
/** @function	: UnSetSpecificValueByBuffTempletID
	@brief		: 각 유닛마다 특정 버프가 해제 될 때 셋팅해야 하는 함수 실행(ex: 매지컬메이크업이 해제 됬다는 플래그 설정 등...)
	@param		: 버프템플릿ID(eBuffTempletId_)
*/
/*virtual*/ void CX2GUEL::UnSetSpecificValueByBuffTempletID( const BUFF_TEMPLET_ID eBuffTempletId_ )
{
	CX2GUUser::UnSetSpecificValueByBuffTempletID( eBuffTempletId_ );

	switch ( eBuffTempletId_ )
	{
	// 극기 - 강(强)
	case BTI_BUFF_INDURANCE_POWER:
		{
			m_bIsActiveEndurancePower = false;
		} break;
	// 승리의 검
	case BTI_BUFF_VICTORIOUS_SWORD:
		{
			// 닷지&슬래시 쿨타임, MP 변경 기능 비활성화를 위한 변수 설정
			m_fDodgeAndSlashCoolTimeDecreaseRate = 1.f;
			m_bIsApplyVictoriousSword = false;
		} break;
	default:
		break;
	}
}


#ifdef ELSWORD_WAY_OF_SWORD // 검의 길
// 검의 길
/*virtual*/ int	CX2GUEL::GetWayOfSwordState() const
{ 
	if( NULL != GetELGageData() )
		return GetELGageData()->GetWayOfSwordState();

	return 0;
}
void CX2GUEL::SetWayOfSwordState( const int iWayOfSwordState_ )
{ 
	if( NULL != GetELGageData() )
		GetELGageData()->SetWayOfSwordState( iWayOfSwordState_ );
}
/*virtual*/ float CX2GUEL::GetWayOfSwordPoint() const
{
	if( NULL != GetELGageData() )
		return GetELGageData()->GetWayOfSwordPoint();

	return 0.f;
}

/*virtual*/ void CX2GUEL::SetWayOfSwordPoint( float fWayOfSwordPoint_ )
{
	if( fWayOfSwordPoint_ > g_pData->GetDamageManager()->GetVigorStateThreshold() )
		fWayOfSwordPoint_ = g_pData->GetDamageManager()->GetVigorStateThreshold();

	if( fWayOfSwordPoint_ < g_pData->GetDamageManager()->GetDestStateThreshold() )
		fWayOfSwordPoint_ = g_pData->GetDamageManager()->GetDestStateThreshold();

	if( NULL != GetELGageData() )
		GetELGageData()->SetWayOfSwordPoint( fWayOfSwordPoint_ );
}
// 질풍 시작
void CX2GUEL::Start_SwordmanVigor()
{
	if ( NULL != g_pX2Game->GetWorld() && GetShowCutInAndChangeWorldColor() )
	{
		g_pX2Game->GetWorld()->SetWorldColor( 0xff222255 );
		g_pX2Game->GetWorld()->FadeWorldColor( g_pX2Game->GetWorld()->GetOriginColor(), 4.0f );
	}

	SetBuffFactorToGameUnitByBuffFactorID(BFI_BUFF_GALE_WAY_OF_SWORD);	
	PlaySound( L"elsword_vitality_use_VOX.ogg" );

	m_CommonState.m_Wait = ESI_SWORD_WAIT;
}
// 질풍 종료
void CX2GUEL::End_SwordmanVigor()
{
	EraseBuffTempletFromGameUnit(BTI_BUFF_GALE_WAY_OF_SWORD);

	g_pX2Game->GetEffectSet()->PlayEffectSet( L"EffectSet_VigorEnd", this ); 	
	PlaySound( L"elsword_vitality_end.ogg" );

	m_CommonState.m_Wait = USI_WAIT;
}
// 섬멸 시작
void CX2GUEL::Start_SwordmanDestruction()
{
	if ( NULL != g_pX2Game->GetWorld() && GetShowCutInAndChangeWorldColor() )
	{
		g_pX2Game->GetWorld()->SetWorldColor( 0xff552222 );
		g_pX2Game->GetWorld()->FadeWorldColor( g_pX2Game->GetWorld()->GetOriginColor(), 4.0f );
	}

	if( NULL != g_pX2Game && CX2Game::GT_PVP == g_pX2Game->GetGameType() )
		SetBuffFactorToGameUnitByBuffFactorID(BFI_BUFF_ANNIHILATION_WAY_OF_SWORD_PVP);	
	else
		SetBuffFactorToGameUnitByBuffFactorID(BFI_BUFF_ANNIHILATION_WAY_OF_SWORD);	

	PlaySound( L"elsword_demolition_use_VOX.ogg" );

	m_CommonState.m_Wait = ESI_SWORD_WAIT;
}
// 섬멸 종료
void CX2GUEL::End_SwordmanDestruction()
{
	SetNowChargeMpForDetonation( 0.f ) ;
	EraseBuffTempletFromGameUnit(BTI_BUFF_ANNIHILATION_WAY_OF_SWORD);

	g_pX2Game->GetEffectSet()->PlayEffectSet( L"EffectSet_DestructionEnd", this ); 	
	PlaySound( L"elsword_demolition_end.ogg" );

	m_CommonState.m_Wait = USI_WAIT;
}

void CX2GUEL::WayOfSwordStateChange()
{
	int iNextWayOfSwordState = 0;
	if( IsMyUnit() == true )
	{
		const float fWayOfSwordPoint = GetWayOfSwordPoint();
		if( fWayOfSwordPoint >= g_pData->GetDamageManager()->GetVigorStateThreshold() )
			iNextWayOfSwordState = WSS_VIGOR;
		else if( fWayOfSwordPoint <= g_pData->GetDamageManager()->GetDestStateThreshold() )
			iNextWayOfSwordState = WSS_DESTRUCTION;
		else if( fWayOfSwordPoint > 0 )
			iNextWayOfSwordState = WSS_TOWARD_VIGOR;
		else if( fWayOfSwordPoint < 0 )
			iNextWayOfSwordState = WSS_TOWARD_DESTRUCTION;
		else if( fWayOfSwordPoint == 0 )
			iNextWayOfSwordState = WSS_CENTER;
	}		
	else
	{
		iNextWayOfSwordState = m_FrameDataNow.syncData.m_CannonBallCount;
	}
	
	const WAY_OF_SWORD_STATE eWayofSwordState = static_cast<const WAY_OF_SWORD_STATE>( GetWayOfSwordState() );

	if( iNextWayOfSwordState == WSS_VIGOR && 
		eWayofSwordState != WSS_VIGOR )
	{
		if( eWayofSwordState == WSS_DESTRUCTION )
			End_SwordmanDestruction();

		Start_SwordmanVigor();
		SetWayOfSwordState( static_cast<int>( WSS_VIGOR ) );
	}
	else if( iNextWayOfSwordState == WSS_DESTRUCTION &&
		eWayofSwordState != WSS_DESTRUCTION )
	{
		if( eWayofSwordState == WSS_VIGOR )
			End_SwordmanVigor();

		Start_SwordmanDestruction();
		SetWayOfSwordState( static_cast<int>( WSS_DESTRUCTION ) );
	}
	else if( iNextWayOfSwordState == WSS_TOWARD_VIGOR &&
		eWayofSwordState != WSS_TOWARD_VIGOR && eWayofSwordState != WSS_VIGOR )
	{
		if( eWayofSwordState == WSS_DESTRUCTION )
			End_SwordmanDestruction();

		SetWayOfSwordState( static_cast<int>( WSS_TOWARD_VIGOR ) );
	}
	else if( iNextWayOfSwordState == WSS_TOWARD_DESTRUCTION && 
		eWayofSwordState != WSS_TOWARD_DESTRUCTION && eWayofSwordState != WSS_DESTRUCTION )
	{
		if( eWayofSwordState == WSS_VIGOR )
			End_SwordmanVigor();

		SetWayOfSwordState( static_cast<int>( WSS_TOWARD_DESTRUCTION ) );
	}
	else if( iNextWayOfSwordState == WSS_CENTER && 
		eWayofSwordState != WSS_CENTER )
	{
		if( eWayofSwordState == WSS_VIGOR )
			End_SwordmanVigor();
		else if( eWayofSwordState == WSS_DESTRUCTION )
			End_SwordmanDestruction();

		SetWayOfSwordState( static_cast<int>( WSS_CENTER ) );
	}
	else // Do Nothing
	{
		;
	}
}
/** @function : GaleBuffFrameMove()
	@brief : 매 5회 히트시 MP회수율 증가 버프 적용 시키기 위한 프레임 무브
*/
void CX2GUEL::GaleBuffFrameMove()
{
	if( WSS_VIGOR != GetWayOfSwordState() )
		return;

	const int iComboCount = ( NULL != GetComboManager() ? GetComboManager()->GetCombo() : 0 );

	// 히트 카운트 변화가 없다면 수행하지 않음
	if( iComboCount == m_iPreComboCount )
		return;
	
	m_iPreComboCount = iComboCount;

	// 매 5회 히트 시 MP 회수율 증가 버프 적용
	if( 0 < iComboCount && 0 == ( iComboCount % 5) )
	{
		SetBuffFactorToGameUnitByBuffFactorID( BFI_BUFF_GALE_MP_GAIN_INCREASE );
	}
}
void CX2GUEL::SwordWaitStartFuture()
{
	CommonStateStartFuture();
	PlayAnimationSwordWaitStart( m_pXSkinAnimFuture, m_FrameDataFuture );
}

void CX2GUEL::SwordWaitStart()
{
	CommonStateStart();
	PlayAnimationSwordWaitStart( m_pXSkinAnim.get(), m_FrameDataNow );
}

void CX2GUEL::SwordWaitEventProcess()
{
	if( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
	}
	else if( m_InputData.oneZ == true )
	{
		StateChange( ESI_COMBO_Z );
	}
	else if( m_InputData.oneX == true )
	{
		StateChange( ESI_COMBO_X );
	}
	else if( SpecialAttackEventProcess() == true )
	{
	}
	else if( m_InputData.pureDoubleRight == true || m_InputData.pureDoubleLeft == true )
	{
		StateChangeDashIfPossible();
	}
	else if( m_InputData.pureRight == true || m_InputData.pureLeft == true )
	{
		StateChange( USI_WALK );
	}
	else if( m_InputData.pureUp == true )
	{
		StateChange( USI_JUMP_UP );
		m_PhysicParam.nowSpeed.y = GetJumpSpeed();
		m_FrameDataFuture.syncData.position.y += LINE_RADIUS;
	}
	ELSE_IF_CAN_HYPER_MODE_THEN_STATE_CHANGE( USI_HYPER_MODE )
	else if( m_InputData.oneDown == true )
	{
		if( NULL != g_pX2Game && 
			NULL != g_pX2Game->GetWorld() &&
			NULL != g_pX2Game->GetWorld()->GetLineMap() &&
			true == g_pX2Game->GetWorld()->GetLineMap()->CanDown( m_FrameDataFuture.syncData.position, 
			LINE_RADIUS, &m_FrameDataFuture.unitCondition.landPosition, 
			&m_FrameDataFuture.syncData.lastTouchLineIndex ) )
		{
			StateChange( USI_JUMP_DOWN );
			m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
		}
	}
	else if( GetWayOfSwordState() != WSS_VIGOR && GetWayOfSwordState() != WSS_DESTRUCTION )
	{
		StateChange( USI_WAIT );
	}
#ifdef RIDING_SYSTEM
	else if ( true == GetRidingOn() )
	{
		StateChange( USI_RIDING_ON );
	}
#endif //RIDING_SYSTEM

	CommonEventProcess();
}

void CX2GUEL::PlayAnimationSwordWaitStart( CKTDGXSkinAnim* pXSkinAnim_, const FrameData& refFrameData_ )
{
	CKTDGXSkinAnim::XSKIN_ANIM_PLAYTYPE playType;
	LUA_GET_VALUE_ENUM ( m_LuaManager, "PLAY_TYPE", playType, CKTDGXSkinAnim::XSKIN_ANIM_PLAYTYPE, CKTDGXSkinAnim::XAP_ONE_WAIT );

	// FATAL_HP_PERCENT 보다 HP가 적으면 fatal 애니메이션이 실행
	const float FATAL_HP_PERCENT = 0.25f;

	if( GetMaxHp() * FATAL_HP_PERCENT >= refFrameData_.syncData.fNowHP )
	{
		bool bTransition = true;
		LUA_GET_VALUE( m_LuaManager, "TRANSITION", bTransition, true );

		if( false == refFrameData_.unitCondition.bTransitionChange || false == bTransition )
			pXSkinAnim_->ChangeAnim( L"Fatal", false );
		else
			pXSkinAnim_->ChangeAnim( L"Fatal", true );
	}
	else
	{
		bool bTransition = true;
		LUA_GET_VALUE( m_LuaManager, "TRANSITION", bTransition, true );
		if( false == refFrameData_.unitCondition.bTransitionChange || false == bTransition )
		{
			if( GetWayOfSwordState() == WSS_VIGOR )
				pXSkinAnim_->ChangeAnim( L"Wait_W0", false );
			else if( GetWayOfSwordState() == WSS_DESTRUCTION )
				pXSkinAnim_->ChangeAnim( L"Wait_W1", false );
			else
				pXSkinAnim_->ChangeAnim( L"Wait", false );
		}
		else
		{
			if( GetWayOfSwordState() == WSS_VIGOR )
				pXSkinAnim_->ChangeAnim( L"Wait_W0", true );
			else if( GetWayOfSwordState() == WSS_DESTRUCTION )
				pXSkinAnim_->ChangeAnim( L"Wait_W1", true );
			else
				pXSkinAnim_->ChangeAnim( L"Wait", true );
		}
	}

	m_pXSkinAnim->Play( playType );
}
bool CX2GUEL::CanSpecialReactProcess( CX2DamageManager::REACT_TYPE eReActType_ )
{
	if( 0.f >= GetNowHp() )
		return false;

	if( true == GetInvincible() )
		return false;

	switch( eReActType_ ) 
	{
	case CX2DamageManager::RT_NO_DAMAGE:
	case CX2DamageManager::RT_DUMMY_DAMAGE:
	case CX2DamageManager::RT_SENSOR:
		return false;
	default:
		break;
	}

	return true;
}
/** @function : GetFinalDamage_WayOfSword
	@brief : -GetFinalDamage 계산 할 때 검의길 타입에 따라 데미지 변경 시켜 주기 위한 함수
			 -엘소드와 엘리시스가 다른 동작을 할 수 있도록 virtual 함수로 변경
*/
/*virtual*/float CX2GUEL::GetFinalDamage_WayOfSword( CX2DamageManager::WAY_OF_SWORD_TYPE eWayOfSwordType_ )
{
	float fDamageMagnifications = 1.f;
	if( WSS_DESTRUCTION == GetWayOfSwordState() )
	{
		switch( eWayOfSwordType_ ) 
		{
		case CX2DamageManager::WST_DESTRUCTION_NORMAL:
			{
				if( CX2Game::GT_PVP == g_pX2Game->GetGameType() )
					fDamageMagnifications = 1.25f;
				else 
					fDamageMagnifications = 1.5f;
			} break;
		default:
			break;
		}
	}

	return fDamageMagnifications;
}
/** @function : GetCriticalPercent_WayOfSword
	@brief : 검의길 타입에 따른 크리티컬 배율 변경
*/
/*virtual*/ void CX2GUEL::SetCriticalPercent_WayOfSword( IN CX2DamageManager::WAY_OF_SWORD_TYPE eWayOfSwordType_ ,
												OUT float& fCriticalPercent_)
{
	if( WSS_DESTRUCTION == GetWayOfSwordState() )
	{
		switch( eWayOfSwordType_ ) 
		{
		case CX2DamageManager::WST_DESTRUCTION_NORMAL:
		case CX2DamageManager::WST_DESTRUCTION_ACTIVE:
		case CX2DamageManager::WST_DESTRUCTION_SPECIAL:
			{
				fCriticalPercent_ = 1.f;
			} break;
		default:
			break;
		}
	}
}
#endif // ELSWORD_WAY_OF_SWORD

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 스테이트 함수
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//ESI_DIE_FRONT
void CX2GUEL::DieFrontStartFuture()
{
	CommonStateStartFuture();

#ifdef ELSWORD_WAY_OF_SWORD // 검의 길
	const WAY_OF_SWORD_STATE eWayofSwordState = static_cast<const WAY_OF_SWORD_STATE>( GetWayOfSwordState() );

	if( eWayofSwordState == WSS_VIGOR )
		End_SwordmanVigor();
	else if( eWayofSwordState == WSS_DESTRUCTION )
		End_SwordmanDestruction();
#endif // ELSWORD_WAY_OF_SWORD

}
void CX2GUEL::DieFrontStart()
{
	CommonStateStart();
	DyingStart();
	m_pXSkinAnim->SetPlaySpeed( 0.5f );
}
void CX2GUEL::DieFrontFrameMove()
{
	const float TIME_CREATE_STEP_DUST = 0.19f;		// 연기가 발생되야하는 애니메이션 시간
	const float TIME_SACRIFICE_OF_HERO = 1.12f;		// 영웅의 희생이 실행되어야 하는 애니메이션 시간

	DieFrameMove( TIME_CREATE_STEP_DUST, TIME_SACRIFICE_OF_HERO );
}
void CX2GUEL::DieFrontEventProcess()
{
	if ( GetShowObject() == false && m_pXSkinAnim->IsAnimationEnd() == true )
	{
		StateChange( GUSI_DIE );
	}
}

//ESI_DIE_BACK
void CX2GUEL::DieBackStartFuture()
{
	CommonStateStartFuture();

#ifdef ELSWORD_WAY_OF_SWORD // 검의 길
	const WAY_OF_SWORD_STATE eWayofSwordState = static_cast<const WAY_OF_SWORD_STATE>( GetWayOfSwordState() );

	if( eWayofSwordState == WSS_VIGOR )
		End_SwordmanVigor();
	else if( eWayofSwordState == WSS_DESTRUCTION )
		End_SwordmanDestruction();
#endif // ELSWORD_WAY_OF_SWORD

}
void CX2GUEL::DieBackStart()
{
	CommonStateStart();
	DyingStart();
	m_pXSkinAnim->SetPlaySpeed( 0.5f );
}
void CX2GUEL::DieBackFrameMove()
{
	const float TIME_CREATE_STEP_DUST = 0.19f;		// 연기가 발생되야하는 애니메이션 시간
	const float TIME_SACRIFICE_OF_HERO = 1.12f;		// 영웅의 희생이 실행되어야 하는 애니메이션 시간

	DieFrameMove( TIME_CREATE_STEP_DUST, TIME_SACRIFICE_OF_HERO );
}
void CX2GUEL::DieBackEventProcess()
{
	if ( GetShowObject() == false  && m_pXSkinAnim->IsAnimationEnd() == true )
	{
		StateChange( GUSI_DIE );
	}
}

//ESI_WAIT
void CX2GUEL::WaitStartFuture()
{
	CommonStateStartFuture();
	PlayAnimationWaitStart( m_pXSkinAnimFuture, m_FrameDataFuture );
}
void CX2GUEL::WaitStart()
{
	CommonStateStart();
	PlayAnimationWaitStart( m_pXSkinAnim.get(), m_FrameDataNow );
}
void CX2GUEL::WaitEventProcess()
{
	if( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
	}
	else if( m_InputData.oneZ == true )
	{
		StateChange( ESI_COMBO_Z );
	}
	else if( m_InputData.oneX == true )
	{
		StateChange( ESI_COMBO_X );
	}
	else if( true == SpecialAttackEventProcess() )
	{
	}
	else if( m_InputData.pureDoubleRight == true || m_InputData.pureDoubleLeft == true )
	{
		StateChangeDashIfPossible();
	}
	else if( m_InputData.pureRight == true || m_InputData.pureLeft == true )
	{
		StateChange( USI_WALK );
	}
	else if( m_InputData.pureUp == true )
	{
		StateChange( USI_JUMP_UP );
		m_PhysicParam.nowSpeed.y = GetJumpSpeed();
		m_FrameDataFuture.syncData.position.y += LINE_RADIUS;
	}
	ELSE_IF_CAN_HYPER_MODE_THEN_STATE_CHANGE( USI_HYPER_MODE )
	else if( m_InputData.oneDown == true )
	{
		if( NULL != g_pX2Game && 
			NULL != g_pX2Game->GetWorld() &&
			NULL != g_pX2Game->GetWorld()->GetLineMap() &&
			true == g_pX2Game->GetWorld()->GetLineMap()->CanDown( m_FrameDataFuture.syncData.position, 
				LINE_RADIUS, &m_FrameDataFuture.unitCondition.landPosition, 
				&m_FrameDataFuture.syncData.lastTouchLineIndex ) )
		{
			StateChange( USI_JUMP_DOWN );
			m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
		}
	}
#ifdef ELSWORD_WAY_OF_SWORD // 검의 길
	else if( ( GetWayOfSwordState() == WSS_VIGOR || GetWayOfSwordState() == WSS_DESTRUCTION ) && 
			m_FrameDataNow.unitCondition.fStateTime >= 0.1f )
	{
		StateChange( ESI_SWORD_WAIT );
	}
#endif // ELSWORD_WAY_OF_SWORD
#ifdef RIDING_SYSTEM
	else if ( true == GetRidingOn() )
	{
		StateChange( USI_RIDING_ON );
	}
#endif //RIDING_SYSTEM
	CommonEventProcess();
}

//ESI_WALK
void CX2GUEL::WalkEventProcess()
{
	if( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
	}
	else if( m_InputData.oneZ == true )
	{
		StateChange( ESI_COMBO_Z );
	}
	else if( m_InputData.oneX == true )
	{
		StateChange( ESI_COMBO_X );
	}
	else if( SpecialAttackEventProcess() == true )
	{
	}
	else if( m_InputData.pureRight == false && m_InputData.pureLeft == false )
	{
		StateChange( USI_WAIT );
	}
	else if( m_InputData.pureUp == true )
	{
		StateChange( USI_JUMP_UP );
		m_PhysicParam.nowSpeed.y = GetJumpSpeed();
		m_FrameDataFuture.syncData.position.y += LINE_RADIUS;
	}
	ELSE_IF_CAN_HYPER_MODE_THEN_STATE_CHANGE( USI_HYPER_MODE )
	else if( m_InputData.oneDown == true )
	{
		if( NULL != g_pX2Game && 
			NULL != g_pX2Game->GetWorld() &&
			NULL != g_pX2Game->GetWorld()->GetLineMap() &&
			true == g_pX2Game->GetWorld()->GetLineMap()->CanDown( m_FrameDataFuture.syncData.position, 
			LINE_RADIUS, &m_FrameDataFuture.unitCondition.landPosition, 
			&m_FrameDataFuture.syncData.lastTouchLineIndex ) )
		{
			StateChange( USI_JUMP_DOWN );
			m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
		}
	}
#ifdef RIDING_SYSTEM
	else if ( true == GetRidingOn() )
	{
		StateChange( USI_RIDING_ON );
	}
#endif //RIDING_SYSTEM

	CommonEventProcess();
}

//ESI_JUMP_READY
void CX2GUEL::JumpReadyEventProcess()
{
	if( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS;
	}
	else if( SpecialAttackEventProcess() == true )
	{
	}
	else if( true == m_pXSkinAnimFuture->IsAnimationEnd() || true == m_InputData.pureRight || true == m_InputData.pureLeft )
	{
		StateChange( USI_JUMP_UP );
		m_PhysicParam.nowSpeed.y = GetJumpSpeed();
		m_FrameDataFuture.syncData.position.y += LINE_RADIUS;
	}

	CommonEventProcess();
}

//ESI_JUMP_UP
void CX2GUEL::JumpUpEventProcess()
{
	if( (m_InputData.pureDoubleLeft == true || m_InputData.pureDoubleRight == true ) &&
		GetEnableDash() == true )
	{
		StateChange( USI_DASH_JUMP );
	}
	else if( true == SpecialAttackEventProcess( CX2SkillTree::ASUT_AIR ) )
	{
	}
	else if ( true == m_InputData.oneZ )
	{
		StateChange( ESI_JUMP_ATTACK_Z );
	}
	else if ( true == m_InputData.oneX )
	{
		StateChange( ESI_JUMP_ATTACK_X );
	}
	else if( 0.0f >= m_PhysicParam.nowSpeed.y )
	{
		StateChange( USI_JUMP_DOWN );
	}
#ifdef RIDING_SYSTEM
	else if ( true == GetRidingOn() )
	{
		StateChange( USI_RIDING_ON );
	}
#endif //RIDING_SYSTEM

	CommonEventProcess();
}

//ESI_JUMP_DOWN
void CX2GUEL::JumpDownEventProcess()
{
	if ( true == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_LANDING );
	}
	else if( (m_InputData.pureDoubleLeft == true || m_InputData.pureDoubleRight == true ) &&
		GetEnableDash() == true )
	{
		StateChange( USI_DASH_JUMP );
	}
	else if( true == m_FrameDataFuture.unitCondition.bFootOnWall && 
		( true == m_InputData.oneUp || true == m_InputData.oneDoubleUp ) )
	{
		m_bDisableGravity = true;
		StateChange( ESI_WALL_LANDING );
	}
	else if ( true == SpecialAttackEventProcess( CX2SkillTree::ASUT_AIR ) )
	{
	}
	else if ( true == m_InputData.oneZ )
	{
		StateChange( ESI_JUMP_ATTACK_Z );
	}
	else if ( true == m_InputData.oneX )
	{
		StateChange( ESI_JUMP_ATTACK_X );
	}
	else if ( 0.0f <= m_PhysicParam.nowSpeed.y )
	{
		StateChange( USI_JUMP_UP );
	}
#ifdef RIDING_SYSTEM
	else if ( true == GetRidingOn() )
	{
		StateChange( USI_RIDING_ON );
	}
#endif //RIDING_SYSTEM

	CommonEventProcess();
}

//ESI_JUMP_LANDING
void CX2GUEL::JumpLandingStartFuture()
{
	m_bIsDashAttackPossible = true;
	CommonStateStartFuture();
}
void CX2GUEL::JumpLandingEventProcess()
{
	if( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS;
	}
	else if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	else if( SpecialAttackEventProcess() == true )
	{
	}
	else if ( true == m_InputData.oneZ )
	{
		StateChange( ESI_COMBO_Z );
	}
	else if ( true == m_InputData.oneX )
	{
		StateChange( ESI_COMBO_X );
	}
	else if( true == m_InputData.pureDoubleRight || true == m_InputData.pureDoubleLeft )
	{
		StateChangeDashIfPossible();
	}
	else if( true == m_InputData.pureRight || true == m_InputData.pureLeft )
	{
		StateChange( USI_WALK );
	}
	else if( true == m_InputData.pureUp )
	{
		StateChange( USI_JUMP_UP );
		m_PhysicParam.nowSpeed.y = GetJumpSpeed();
		m_FrameDataFuture.syncData.position.y += LINE_RADIUS;
	}
	ELSE_IF_CAN_HYPER_MODE_THEN_STATE_CHANGE( USI_HYPER_MODE )
	else if( m_InputData.oneDown == true )
	{
		if( NULL != g_pX2Game && 
			NULL != g_pX2Game->GetWorld() &&
			NULL != g_pX2Game->GetWorld()->GetLineMap() &&
			true == g_pX2Game->GetWorld()->GetLineMap()->CanDown( m_FrameDataFuture.syncData.position, 
			LINE_RADIUS, &m_FrameDataFuture.unitCondition.landPosition, 
			&m_FrameDataFuture.syncData.lastTouchLineIndex ) )
		{
			StateChange( USI_JUMP_DOWN );
			m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
		}
	}
	ELSE_IF_KEY_PRESSED_AT_SKIP_POINT_ZXCAS
	{
		StateChange( USI_WAIT, false );
	}

	CommonEventProcess();
}

//ESI_DASH
void CX2GUEL::DashEventProcess()
{
	if( false == IsOnSomethingFuture() )
	{
		StateChange( USI_DASH_JUMP );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
	}
	else if( m_InputData.oneZ == true )
	{
		StateChange( ESI_DASH_COMBO_Z );
	}
	else if( m_InputData.oneX == true )
	{
		StateChange( ESI_DASH_COMBO_X );
	}
	else if ( true == SpecialAttackEventProcess() )
	{
	}
	else if( true == m_InputData.oneUp )
	{
		StateChange( USI_DASH_JUMP );
		m_PhysicParam.nowSpeed.y = GetDashJumpSpeed();
	}
	ELSE_IF_CAN_HYPER_MODE_THEN_STATE_CHANGE( USI_HYPER_MODE )
	else if( false == m_InputData.pureRight && false == m_InputData.pureLeft )
	{
		StateChange( USI_DASH_END );
	}
	else if( m_InputData.pureRight == true && m_InputData.pureLeft == true )
	{
		StateChange( USI_DASH_END );
	}
	else if( m_InputData.pureRight == true && m_FrameDataFuture.syncData.bIsRight == false )
	{
		StateChange( USI_DASH_END );
	}
	else if( m_InputData.pureLeft == true && m_FrameDataFuture.syncData.bIsRight == true )
	{
		StateChange( USI_DASH_END );
	}
	else if( m_InputData.oneDown == true )
	{
		if( NULL != g_pX2Game && 
			NULL != g_pX2Game->GetWorld() &&
			NULL != g_pX2Game->GetWorld()->GetLineMap() &&
			true == g_pX2Game->GetWorld()->GetLineMap()->CanDown( m_FrameDataFuture.syncData.position, 
			LINE_RADIUS, &m_FrameDataFuture.unitCondition.landPosition, 
			&m_FrameDataFuture.syncData.lastTouchLineIndex ) )
		{
			StateChange( USI_DASH_JUMP );
			m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
		}
	}
#ifdef RIDING_SYSTEM
	else if ( true == GetRidingOn() )
	{
		StateChange( USI_RIDING_ON );
	}
#endif //RIDING_SYSTEM

	CommonEventProcess();
}

//ESI_DASH_END
void CX2GUEL::DashEndStartFuture()
{
	CommonStateStartFuture();
	m_PhysicParam.nowSpeed.x = GetDashSpeed();
}
void CX2GUEL::DashEndFrameMove()
{
	if( m_pXSkinAnim->EventTimer( 0.4f ) == true )
		CreateStepDust();

	CommonFrameMove();
}
void CX2GUEL::DashEndEventProcess()
{
	// DashEnd에서 공격 키를 눌렀을 때 대시 공격인지, 일반 공격인지를 정하기 위한 const 값. 
	const float TRIGGER_TIME_FACTOR_DASH_ATTACK = 0.3f;

	if( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
	}
	else if( m_InputData.oneZ == true )
	{
		if( m_pXSkinAnimFuture->GetNowAnimationTime() > m_pXSkinAnimFuture->GetMaxAnimationTime() * TRIGGER_TIME_FACTOR_DASH_ATTACK )
		{
			StateChange( ESI_COMBO_Z );
		}
		else
		{
			StateChange( ESI_DASH_COMBO_Z );
		}
	}
	else if( m_InputData.oneX == true )
	{
		if( m_pXSkinAnimFuture->GetNowAnimationTime() > m_pXSkinAnimFuture->GetMaxAnimationTime() * TRIGGER_TIME_FACTOR_DASH_ATTACK )
		{
			StateChange( ESI_COMBO_X );
		}
		else
		{
			StateChange( ESI_DASH_COMBO_X );
		}
	}
	else if( SpecialAttackEventProcess() == true)
	{
	}
	else if( m_InputData.pureDoubleRight == true || m_InputData.pureDoubleLeft == true )
	{
		StateChangeDashIfPossible();
	}
	else if( m_InputData.oneRight == true && m_FrameDataFuture.syncData.bIsRight == true )
	{
		StateChangeDashIfPossible();
	}
	else if( m_InputData.oneLeft == true && m_FrameDataFuture.syncData.bIsRight == false )
	{
		StateChangeDashIfPossible();
	}
	else if( m_InputData.pureRight == true || m_InputData.pureLeft == true )
	{
		StateChange( USI_WALK );
	}
	else if( m_InputData.oneDown == true )
	{
		if( NULL != g_pX2Game && 
			NULL != g_pX2Game->GetWorld() &&
			NULL != g_pX2Game->GetWorld()->GetLineMap() &&
			true == g_pX2Game->GetWorld()->GetLineMap()->CanDown( m_FrameDataFuture.syncData.position, 
			LINE_RADIUS, &m_FrameDataFuture.unitCondition.landPosition, 
			&m_FrameDataFuture.syncData.lastTouchLineIndex ) )
		{
			StateChange( USI_JUMP_DOWN );
			m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
		}
	}
	else if( m_InputData.oneUp == true )
	{
		StateChange( USI_JUMP_READY );
	}
	ELSE_IF_CAN_HYPER_MODE_THEN_STATE_CHANGE( USI_HYPER_MODE )
	else if( m_pXSkinAnimFuture->IsAnimationEnd() == true )
	{	
		StateChange( USI_WAIT );
	}
#ifdef RIDING_SYSTEM
	else if ( true == GetRidingOn() )
	{
		StateChange( USI_RIDING_ON );
	}
#endif //RIDING_SYSTEM

	CommonEventProcess();
}

//ESI_DASH_JUMP
void CX2GUEL::DashJumpStartFuture()
{
	CommonStateStartFuture();

	const float MAGIC_NUMBER_MULTIPLY_GRAVITY_ACCEL = 1.2f;

	m_PhysicParam.fDownAccel		= m_PhysicParam.fGAccel * MAGIC_NUMBER_MULTIPLY_GRAVITY_ACCEL;
	m_bDownForce					= false;

}
void CX2GUEL::DashJumpStart()
{
	CommonStateStart();

	if( GetMatrix().GetYPos() == m_FrameDataNow.unitCondition.landPosition.y )
		CreateStepDust();
}
void CX2GUEL::DashJumpFrameMoveFuture()
{
	const float MAGIC_NUMBER_MULTIPLY_DASH_SPEED = 1.6f;

	if( m_InputData.pureRight == true && m_FrameDataNow.syncData.bIsRight	== true )
	{
		m_PhysicParam.nowSpeed.x = GetDashSpeed() * MAGIC_NUMBER_MULTIPLY_DASH_SPEED;
	}
	else if( m_InputData.pureLeft == true && m_FrameDataNow.syncData.bIsRight	== false )
	{
		m_PhysicParam.nowSpeed.x = GetDashSpeed() * MAGIC_NUMBER_MULTIPLY_DASH_SPEED;
	}

	CommonFrameMoveFuture();
}
void CX2GUEL::DashJumpEventProcess()
{
	bool &bCanDashJump = m_FrameDataFuture.stateParam.bEventFlagList[0];

		if( true == IsOnSomethingFuture() )
	{
		// 위쪽 화살표를 한번 더 눌렀으면
		if( true == bCanDashJump )
		{
			StateChange( USI_DASH_JUMP );	// 계속해서 대시 점프
			m_PhysicParam.nowSpeed.y = GetDashJumpSpeed();
			m_bDownForce = false;
		}
		else if ( true == m_bDownForce )
		{
			StateChange( ESI_DASH_JUMP_POWER_LANDING );
			m_bDownForce = false;
		} // if
		else
		{
			if( true == m_InputData.pureRight || true == m_InputData.pureLeft )
			{
				StateChangeDashIfPossible();
			}
			else
				StateChange( USI_DASH_JUMP_LANDING );

			m_bDownForce = false;
		}
	}
	else if( true == m_FrameDataFuture.unitCondition.bFootOnWall &&
		( true == m_InputData.oneUp || true == m_InputData.oneDoubleUp ) )
	{
		m_bDisableGravity = true;
		StateChange( ESI_WALL_LANDING );
	}
	else if( true == SpecialAttackEventProcess( CX2SkillTree::ASUT_AIR ) )
	{
	}
	else if ( true == m_InputData.oneZ && true == m_bIsDashAttackPossible)
	{
		StateChange( ESI_DASH_JUMP_COMBO_Z );
	}
	else if ( true == m_InputData.oneX && true == m_bIsDashAttackPossible)
	{
		StateChange( ESI_DASH_JUMP_COMBO_X );
	}
	else if( true == m_InputData.pureRight )
	{
		if( false == m_FrameDataFuture.syncData.bIsRight )
		{
			if( GetJumpSpeed() < m_PhysicParam.nowSpeed.y )
				m_PhysicParam.nowSpeed.y = GetJumpSpeed();

			if( 0.0f < m_PhysicParam.nowSpeed.y )
				StateChange( USI_JUMP_UP );
			else
				StateChange( USI_JUMP_DOWN );
		}
	}
	else if( true == m_InputData.pureLeft )
	{
		if( true == m_FrameDataFuture.syncData.bIsRight )
		{
			if( GetJumpSpeed() < m_PhysicParam.nowSpeed.y )
				m_PhysicParam.nowSpeed.y = GetJumpSpeed();

			if( 0.0f < m_PhysicParam.nowSpeed.y )
				StateChange( USI_JUMP_UP );
			else
				StateChange( USI_JUMP_DOWN );
		}
	}
#ifdef RIDING_SYSTEM
	else if ( true == GetRidingOn() )
	{
		StateChange( USI_RIDING_ON );
	}
#endif //RIDING_SYSTEM

	if( true == m_InputData.oneUp  && 0.0f >= m_PhysicParam.nowSpeed.y  )
	{
		bCanDashJump = true;
	}
	if( true == m_InputData.oneDown )
	{
		m_PhysicParam.nowSpeed.y = m_PhysicParam.fMaxGSpeed * 2;
		m_bDownForce = true;
		bCanDashJump = false;
	}

	CommonEventProcess();
}
void CX2GUEL::DashJumpEndFuture()
{
	CommonStateEndFuture();
	m_PhysicParam.fDownAccel	= 0.0f;
}

//ESI_DASH_JUMP_LANDING
void CX2GUEL::DashJumpLandingEventProcess()
{
	if( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS;
	}
	else if ( true == m_InputData.oneZ )
	{
		StateChange( ESI_COMBO_Z );
	}
	else if ( true == m_InputData.oneX )
	{
		StateChange( ESI_COMBO_X );
	}
	else if( SpecialAttackEventProcess() == true )
	{
	}
	else if( true == m_InputData.pureDoubleRight || true == m_InputData.pureDoubleLeft )
	{
		StateChangeDashIfPossible();
	}
	else if( true == m_InputData.oneRight && true == m_FrameDataFuture.syncData.bIsRight )
	{
		StateChangeDashIfPossible();
	}
	else if( true == m_InputData.oneLeft && false == m_FrameDataFuture.syncData.bIsRight )
	{
		StateChangeDashIfPossible();
	}
	else if( true == m_InputData.pureRight || true == m_InputData.pureLeft )
	{
		StateChange( USI_WALK );
	}
	else if( true == m_InputData.pureUp )
	{
		StateChange( USI_JUMP_READY );
	}
	else if( m_InputData.oneDown == true )
	{
		if( NULL != g_pX2Game && 
			NULL != g_pX2Game->GetWorld() &&
			NULL != g_pX2Game->GetWorld()->GetLineMap() &&
			true == g_pX2Game->GetWorld()->GetLineMap()->CanDown( m_FrameDataFuture.syncData.position, 
			LINE_RADIUS, &m_FrameDataFuture.unitCondition.landPosition, 
			&m_FrameDataFuture.syncData.lastTouchLineIndex ) )
		{
			StateChange( USI_JUMP_DOWN );
			m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
		}
	}
	else if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}

	CommonEventProcess();
}
void CX2GUEL::DashJumpLandingStartFuture()
{
	CX2GUUser::DashJumpLandingStartFuture();
	m_bIsDashAttackPossible = true;
}
//ESI_DAMAGE_SMALL_FRONT
//ESI_DAMAGE_SMALL_BACK
void CX2GUEL::DamageSmallEventProcess()
{
	if( m_FrameDataFuture.syncData.position.y - m_FrameDataFuture.unitCondition.landPosition.y > 200.0f )
	{
		StateChange( USI_DAMAGE_AIR_SMALL );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
	}
	else if( m_pXSkinAnimFuture->IsAnimationEnd() == true )
	{	
		if( false == IsOnSomethingFuture() )
		{
			StateChange( USI_DAMAGE_AIR_SMALL );
			m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
		}
		else
			StateChange( USI_WAIT );
	}	
	CommonEventProcess();
}

//ESI_DAMAGE_BIG_FRONT
void CX2GUEL::DamageBigStart()
{
	CommonStateStart();
	CreateStepDust();
}
void CX2GUEL::DamageBigEventProcess()
{
	float fGapFromUnitToLineMap = m_FrameDataFuture.syncData.position.y - m_FrameDataFuture.unitCondition.landPosition.y;
	const float ACCEPTABLE_GAP = 200.0f;	// 이것보다 거리가 크면 공중에 있는 상태임

	if( ACCEPTABLE_GAP < fGapFromUnitToLineMap )
	{
		StateChange( USI_DAMAGE_AIR_SMALL );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS;
	}
	else if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{	
		if( false == IsOnSomethingFuture() )
		{
			StateChange( USI_DAMAGE_AIR_SMALL );
			m_FrameDataFuture.syncData.position.y -= LINE_RADIUS;
		}
		else
			StateChange( USI_WAIT );
	}
	ELSE_IF_KEY_PRESSED_AT_SKIP_POINT_ZXC
	{
		if( 0.93f < m_pXSkinAnimFuture->GetNowAnimationTime() )
		{
			StateChange( USI_WAIT );

		}
	}

	CommonEventProcess();
}

//ESI_DAMAGE_DOWN_FRONT
void CX2GUEL::DamageDownFrontFrameMove()
{
	if( true == m_pXSkinAnim->EventTimer( 0.19f ) && true == EventCheck( 0.19f, false ) )
		CreateStepDust();

	CommonFrameMove();
}
void CX2GUEL::DamageDownFrontEventProcess()
{
	bool& bChangeStandUpAttackFront			= m_FrameDataFuture.stateParam.bEventFlagList[1];
	bool& bChangeStandUpRollingFrontFront	= m_FrameDataFuture.stateParam.bEventFlagList[2];
	bool& bChangeStandUpRollingFrontBack	= m_FrameDataFuture.stateParam.bEventFlagList[3];
	bool& bChangeStandUpBigAttackFront		= m_FrameDataFuture.stateParam.bEventFlagList[4];

	if( false == IsOnSomethingFuture() )
	{
		StateChange( USI_DAMAGE_AIR_FALL );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS;
	}
	// 넘어진 후
	else if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{	
		if( true == bChangeStandUpAttackFront )
		{
			StateChange( ESI_STANDUP_ATTACK_FRONT );
		}
		else if ( true == bChangeStandUpBigAttackFront )
		{
			if( GetRandomInt() < m_SkillRelatedData.m_iNoDownDamageAtStandUpAttackPercent )
			{
				StateChange( ESI_QUICK_STANDUP_ATTACK_FRONT );
			}
			else
			{
				StateChange( ESI_STANDUP_ATTACK_FRONT );
			}
		}
		else if( true == bChangeStandUpRollingFrontFront )
		{
			StateChange( ESI_STANDUP_ROLLING_FRONT_FRONT );
		}
		else if( true == bChangeStandUpRollingFrontBack )
		{
			StateChange( ESI_STANDUP_ROLLING_FRONT_BACK );
		}
		else
		{
			StateChange( USI_DAMAGE_STANDUP_FRONT );
		}
	}
	else if( 0.33f < m_pXSkinAnimFuture->GetNowAnimationTime() )
	{
		if( true == m_InputData.oneZ )
		{	
			bChangeStandUpAttackFront		= true;
			bChangeStandUpRollingFrontFront	= false;
			bChangeStandUpRollingFrontBack	= false;
			bChangeStandUpBigAttackFront	= false;

		}
		else if ( true == m_InputData.oneX )
		{
			bChangeStandUpAttackFront		= false;
			bChangeStandUpRollingFrontFront	= false;
			bChangeStandUpRollingFrontBack	= false;
			bChangeStandUpBigAttackFront	= true;
		}
		// 우측 방향을 향하고 있는 경우에
		else if( true == m_FrameDataFuture.syncData.bIsRight )
		{
			// 우측 화살표 키보드를 누른 경우
			if( true == m_InputData.pureRight )
			{	
				// 앞으로 롤링
				bChangeStandUpAttackFront		= false;
				bChangeStandUpRollingFrontBack	= false;
				bChangeStandUpRollingFrontFront	= true;
				bChangeStandUpBigAttackFront	= false;

			}
			// 좌측 화살표 키보드를 누른 경우
			else if( true == m_InputData.pureLeft )
			{	
				// 뒤로 롤링
				bChangeStandUpAttackFront		= false;
				bChangeStandUpRollingFrontFront	= false;
				bChangeStandUpRollingFrontBack	= true;
				bChangeStandUpBigAttackFront	= false;

			}
		}
		// 좌측 방향을 향하고 있는 경우에
		else
		{
			// 우측 화살표 키보드를 누른 경우
			if( true == m_InputData.pureRight )
			{	
				// 뒤로 롤링
				bChangeStandUpAttackFront		= false;
				bChangeStandUpRollingFrontFront	= false;
				bChangeStandUpRollingFrontBack	= true;
				bChangeStandUpBigAttackFront	= false;

			}
			else if( true == m_InputData.pureLeft )
			{	
				// 앞을 롤링
				bChangeStandUpAttackFront		= false;
				bChangeStandUpRollingFrontBack	= false;
				bChangeStandUpRollingFrontFront	= true;
				bChangeStandUpBigAttackFront	= false;
			}
		}
	}

	CommonEventProcess();
}

//ESI_DAMAGE_DOWN_BACK
void CX2GUEL::DamageDownBackFrameMove()
{
	if( true == m_pXSkinAnim->EventTimer( 0.32f ) )
		CreateStepDust();

	CommonFrameMove();
}
void CX2GUEL::DamageDownBackEventProcess()
{
	bool& bChangeStandUpAttackBack			= m_FrameDataFuture.stateParam.bEventFlagList[1];
	bool& bChangeStandUpRollingBackFront	= m_FrameDataFuture.stateParam.bEventFlagList[2];
	bool& bChangeStandUpRollingBackBack		= m_FrameDataFuture.stateParam.bEventFlagList[3];
	bool& bChangeStandUpBigAttackBack		= m_FrameDataFuture.stateParam.bEventFlagList[4];
	// 넘어졌는데 밑에 라인맵 등이 없는 경우
	if( false == IsOnSomethingFuture() )
	{
		StateChange( USI_DAMAGE_AIR_FALL );

		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS;
	}
	// 넘어진 후
	else if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{	
		// Z 키를 눌렀을 때
		if( true == bChangeStandUpAttackBack )
		{
			StateChange( ESI_STANDUP_ATTACK_BACK );
		}
		// X 키를 눌렀을 때
		else if ( true == bChangeStandUpBigAttackBack )
		{
			if( GetRandomInt() < m_SkillRelatedData.m_iNoDownDamageAtStandUpAttackPercent )
			{
				StateChange( ESI_QUICK_STANDUP_ATTACK_BACK );
			}
			else
			{
				StateChange( ESI_STANDUP_ATTACK_BACK );
			}
		}
		else if( true == bChangeStandUpRollingBackFront )
		{
			StateChange( ESI_STANDUP_ROLLING_BACK_FRONT );
		}
		else if( true == bChangeStandUpRollingBackBack )
		{
			StateChange( ESI_STANDUP_ROLLING_BACK_BACK );
		}
		else
		{
			StateChange( USI_DAMAGE_STANDUP_BACK );
		}
	}
	else if( 0.33f < m_pXSkinAnimFuture->GetNowAnimationTime() )
	{
		if( true == m_InputData.oneZ )
		{	
			bChangeStandUpAttackBack		= true;
			bChangeStandUpRollingBackFront	= false;
			bChangeStandUpRollingBackBack	= false;
			bChangeStandUpBigAttackBack		= false;
		}
		else if ( true == m_InputData.oneX )
		{
			bChangeStandUpAttackBack		= false;
			bChangeStandUpRollingBackFront	= false;
			bChangeStandUpRollingBackBack	= false;
			bChangeStandUpBigAttackBack		= true;
		}
		// 우측 방향을 향하고 있는 경우에
		else if( true == m_FrameDataFuture.syncData.bIsRight )
		{
			// 우측 화살표 키보드를 누른 경우
			if( true == m_InputData.pureRight )
			{	
				// 앞으로 롤링
				bChangeStandUpAttackBack		= false;
				bChangeStandUpRollingBackBack	= false;
				bChangeStandUpRollingBackFront	= true;
				bChangeStandUpBigAttackBack		= false;

			}
			// 좌측 화살표 키보드를 누른 경우
			else if( true == m_InputData.pureLeft )
			{	
				// 뒤로 롤링
				bChangeStandUpAttackBack		= false;
				bChangeStandUpRollingBackFront	= false;
				bChangeStandUpRollingBackBack	= true;
				bChangeStandUpBigAttackBack		= false;

			}
		}
		// 좌측 방향을 향하고 있는 경우에
		else
		{
			// 우측 화살표 키보드를 누른 경우
			if( true == m_InputData.pureRight )
			{	
				// 뒤로 롤링
				bChangeStandUpAttackBack		= false;
				bChangeStandUpRollingBackFront	= false;
				bChangeStandUpRollingBackBack	= true;
				bChangeStandUpBigAttackBack		= false;

			}
			else if( true == m_InputData.pureLeft )
			{	
				// 앞을 롤링
				bChangeStandUpAttackBack		= false;
				bChangeStandUpRollingBackBack	= false;
				bChangeStandUpRollingBackFront	= true;
				bChangeStandUpBigAttackBack		= false;
			}
		}
	}

	CommonEventProcess();
}

//ESI_DAMAGE_AIR_DOWN_LANDING
void CX2GUEL::DamageAirDownLandingEventProcess()
{
	DamageDownFrontEventProcess();
}

//ESI_STANDUP_ROLLING_FRONT_FRONT
void CX2GUEL::StandUpRollingFrontFrontEventProcess()
{
	if( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );

		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS;
	}
	else if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		if( true == m_InputData.pureRight || true == m_InputData.pureLeft )
		{
			StateChangeDashIfPossible();
		}
		else
			StateChange( USI_WAIT );
	}

	CommonEventProcess();
}
void CX2GUEL::StandUpRollingFrontFrontEnd()
{
	CommonStateEnd();

	SetForceInvincible( 0.3f );
	SetShowInvincible( 0.3f );
}

//ESI_STANDUP_ROLLING_FRONT_BACK
void CX2GUEL::StandUpRollingFrontBackEventProcess()
{
	if( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );

		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS;
	}
	else if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		if( true == m_InputData.pureRight || true == m_InputData.pureLeft )
		{
			StateChangeDashIfPossible();
		}
		else
			StateChange( USI_WAIT );
	}

	CommonEventProcess();
}
void CX2GUEL::StandUpRollingFrontBackEnd()
{
	CommonStateEnd();
	SetForceInvincible( 0.3f );
	SetShowInvincible( 0.3f );
}

//ESI_STANDUP_ROLLING_BACK_FRONT
void CX2GUEL::StandUpRollingBackFrontEventProcess()
{
	if( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
	}
	else if( m_pXSkinAnimFuture->IsAnimationEnd() == true )
	{	
		if( m_InputData.pureRight == true || m_InputData.pureLeft == true )
		{
			StateChangeDashIfPossible();
		}
		else
			StateChange( USI_WAIT );
	}

	CommonEventProcess();
}
void CX2GUEL::StandUpRollingBackFrontEnd()
{
	CommonStateEnd();
	SetForceInvincible( 0.3f );
	SetShowInvincible( 0.3f );
}

//ESI_STANDUP_ROLLING_BACK_BACK
void CX2GUEL::StandUpRollingBackBackEventProcess()
{
	if( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
	}
	else if( m_pXSkinAnimFuture->IsAnimationEnd() == true )
	{	
		if( m_InputData.pureRight == true || m_InputData.pureLeft == true )
		{
			StateChangeDashIfPossible();
		}
		else
			StateChange( USI_WAIT );
	}

	CommonEventProcess();
}
void CX2GUEL::StandUpRollingBackBackEnd()
{
	CommonStateEnd();
	SetForceInvincible( 0.3f );
	SetShowInvincible( 0.3f );
}

//ESI_STANDUP_ATTACK_FRONT
void CX2GUEL::StandUpAttackFrontEventProcess()
{
	if( m_pXSkinAnimFuture->IsAnimationEnd() == true )
	{
		if( false == IsOnSomethingFuture() )
		{
			StateChange( USI_JUMP_DOWN );
			m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
		}
		else if( m_InputData.pureRight == true || m_InputData.pureLeft == true )
		{
			StateChangeDashIfPossible();
		}
		else
		{
			StateChange( USI_WAIT );
		}
	}
	CommonEventProcess();
}
void CX2GUEL::StandUpAttackFrontEnd()
{
	CommonStateEnd();
	SetForceInvincible( 0.3f );
	SetShowInvincible( 0.3f );
}

//ESI_STANDUP_ATTACK_BACK
void CX2GUEL::StandUpAttackBackEventProcess()
{
	if( m_pXSkinAnimFuture->IsAnimationEnd() == true )
	{
		if( false == IsOnSomethingFuture() )
		{
			StateChange( USI_JUMP_DOWN );
			m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
		}
		else if( m_InputData.pureRight == true || m_InputData.pureLeft == true )
		{
			StateChangeDashIfPossible();
		}
		else
		{
			StateChange( USI_WAIT );
		}
	}
	CommonEventProcess();
}
void CX2GUEL::StandUpAttackBackEnd()
{
	CommonStateEnd();
	SetForceInvincible( 0.3f );
	SetShowInvincible( 0.3f );
}
void CX2GUEL::StandUpAttackBackStateEndFuture()
{
	m_FrameDataFuture.syncData.bIsRight = !m_FrameDataFuture.syncData.bIsRight;
	CommonStateEndFuture();
}
//ESI_STANDUP_ATTACK_FRONT_NO_DOWN
void CX2GUEL::StandUpAttackFrontNoDownEventProcess()
{
	if( m_pXSkinAnimFuture->IsAnimationEnd() == true )
	{
		if( false == IsOnSomethingFuture() )
		{
			StateChange( USI_JUMP_DOWN );
			m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
		}
		else if( m_InputData.pureRight == true || m_InputData.pureLeft == true )
		{
			StateChangeDashIfPossible();
		}
		else
		{
			StateChange( USI_WAIT );
		}
	}
	CommonEventProcess();
}
void CX2GUEL::StandUpAttackFrontNoDownEnd()
{
	CommonStateEnd();
	SetForceInvincible( 0.3f );
	SetShowInvincible( 0.3f );
}

//ESI_STANDUP_ATTACK_BACK_NO_DOWN
void CX2GUEL::StandUpAttackBackNoDownEventProcess()
{
	if( m_pXSkinAnimFuture->IsAnimationEnd() == true )
	{
		if( false == IsOnSomethingFuture() )
		{
			StateChange( USI_JUMP_DOWN );
			m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
		}
		else if( m_InputData.pureRight == true || m_InputData.pureLeft == true )
		{
			StateChangeDashIfPossible();
		}
		else
		{
			StateChange( USI_WAIT );
		}
	}
	CommonEventProcess();
}
void CX2GUEL::StandUpAttackBackNoDownEnd()
{
	CommonStateEnd();
	SetForceInvincible( 0.3f );
	SetShowInvincible( 0.3f );
}

//ESI_DASH_JUMP_POWER_LANDING
void CX2GUEL::DashJumpPowerLandingStartFuture()
{
	CommonStateStartFuture();
	m_PhysicParam.nowSpeed.x /= 2.0f;
}
void CX2GUEL::DashJumpPowerLandingStart()
{
	CommonStateStart();
	CreateStepDust();
}
void CX2GUEL::DashJumpPowerLandingEventProcess()
{
	if( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
	}
	else if( m_InputData.oneRight == true || m_InputData.oneLeft == true )
	{
		StateChange( USI_WALK );
	}
	else if( m_pXSkinAnimFuture->GetNowAnimationTime() > m_pXSkinAnimFuture->GetMaxAnimationTime() / 6.0f )
	{
		if( m_InputData.oneZ == true )
		{
			StateChange( ESI_COMBO_Z );
		}
		else if( m_InputData.oneX == true )
		{
			StateChange( ESI_COMBO_X );
		}
		else if( SpecialAttackEventProcess() == true )
		{
		}
		else if( m_InputData.pureDoubleRight == true || m_InputData.pureDoubleLeft == true )
		{
			StateChangeDashIfPossible();
		}
		else if( m_InputData.pureRight == true || m_InputData.pureLeft == true )
		{
			StateChangeDashIfPossible();
		}
		else if( m_InputData.pureUp == true )
		{
			StateChange( USI_JUMP_READY );
		}
		//ELSE_IF_CAN_USE_SPECIAL_ABILITY_THEN_STATE_CHANGE( 10.f, ESSI_REVENGE_WAIT )
		else if( m_InputData.oneDown == true 
			&& g_pX2Game->GetWorld()->GetLineMap()->CanDown( m_FrameDataFuture.syncData.position, 
			LINE_RADIUS, 
			&m_FrameDataFuture.unitCondition.landPosition, 
			&m_FrameDataFuture.syncData.lastTouchLineIndex ) == true )
		{
			StateChange( USI_JUMP_DOWN );
			m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
		}

		else if( m_pXSkinAnimFuture->IsAnimationEnd() == true )
		{
			StateChange( USI_WAIT );
		}
	}

	CommonEventProcess();
}
void CX2GUEL::ComboZEventProcess()
{
	if ( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS;
	}
	else if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 0, 0, m_InputData.oneZ == true, ESI_COMBO_ZZ )
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
	WALK_CANCEL_AFTER( m_fWalkCancelAfter )
	BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
	DASH_CANCEL_AFTER( m_fDashCancelAfter )

	CommonEventProcess();
}
void CX2GUEL::ComboZZEventProcess()
{
	if ( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS;
	}
	else if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 0, 0, m_InputData.oneZ == true, ESI_COMBO_ZZZ )
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
	WALK_CANCEL_AFTER( m_fWalkCancelAfter )
	BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
	DASH_CANCEL_AFTER( m_fDashCancelAfter )
	
	CommonEventProcess();
}
void CX2GUEL::ComboZZZEventProcess()
{
	if ( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS;
	}
	else if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 0, 0, m_InputData.oneZ == true, ESI_COMBO_ZZZZ )
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 1, 1, m_InputData.oneX == true, ESI_COMBO_ZZZX )
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
	WALK_CANCEL_AFTER( m_fWalkCancelAfter )
	BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
	DASH_CANCEL_AFTER( m_fDashCancelAfter )

	CommonEventProcess();
}
void CX2GUEL::ComboZZZZEventProcess()
{
	if ( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS;
	}
	else if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
	WALK_CANCEL_AFTER( m_fWalkCancelAfter )
	BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
	DASH_CANCEL_AFTER( m_fDashCancelAfter )

	CommonEventProcess();
}
void CX2GUEL::ComboZZZXEventProcess()
{
	if ( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS;
	}
	else if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
	WALK_CANCEL_AFTER( m_fWalkCancelAfter )
	BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
	DASH_CANCEL_AFTER( m_fDashCancelAfter )

	CommonEventProcess();
}
void CX2GUEL::ComboXEventProcess()
{
	if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	else if ( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS;
	}
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 0, 0, m_InputData.oneX == true, ESI_COMBO_XX )
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
	WALK_CANCEL_AFTER( m_fWalkCancelAfter )
	BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
	DASH_CANCEL_AFTER( m_fDashCancelAfter )

	CommonEventProcess();
}
void CX2GUEL::ComboXXEventProcess()
{
	if ( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS;
	}
	else if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 0, 0, m_InputData.oneX == true, ESI_COMBO_XXX )
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 1, 1, m_InputData.oneZ == true, ESI_COMBO_XXZ )
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
	WALK_CANCEL_AFTER( m_fWalkCancelAfter )
	BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
	DASH_CANCEL_AFTER( m_fDashCancelAfter )

	CommonEventProcess();
}
void CX2GUEL::ComboXXXEventProcess()
{
	if ( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS;
	}
	else if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )

	CommonEventProcess();
}
void CX2GUEL::ComboXXZEventProcess()
{
	if ( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS;
	}
	else if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )

	CommonEventProcess();
}
void CX2GUEL::JumpAttackZEventProcess()
{
	if ( true == IsOnSomethingFuture() )
	{
		if( m_FrameDataFuture.stateParam.bEventFlagList[0] == true )
		{
			m_fCanNotJumpTime = 0.2f;
		}
		StateChange( USI_JUMP_LANDING );
	}
	// 현재 진행 방향 쪽으로 키보드를 누르면 앞으로 좀 더 나아감
	else if( ( true == m_InputData.pureRight && true == m_FrameDataFuture.syncData.bIsRight ) ||
		( true == m_InputData.pureLeft && false == m_FrameDataFuture.syncData.bIsRight ) )
	{
		m_PhysicParam.nowSpeed.x = GetWalkSpeed();
	}

	CommonEventProcess();
}
void CX2GUEL::JumpAttackZFrameMoveFuture()
{
	if( m_InputData.pureDown == true )
	{
		m_PhysicParam.nowSpeed.y -= m_PhysicParam.fGAccel * m_fElapsedTime;
		m_FrameDataFuture.stateParam.bEventFlagList[0] = true;
	}
	CommonFrameMoveFuture();
}
void CX2GUEL::JumpAttackXEventProcess()
{
	if ( true == IsOnSomethingFuture() )
	{
		if( m_FrameDataFuture.stateParam.bEventFlagList[0] == true )
		{
			m_fCanNotJumpTime = 0.2f;
		}
		StateChange( USI_JUMP_LANDING );
	}
	// 현재 진행 방향 쪽으로 키보드를 누르면 앞으로 좀 더 나아감
	else if( ( true == m_InputData.pureRight && true == m_FrameDataFuture.syncData.bIsRight ) ||
		( true == m_InputData.pureLeft && false == m_FrameDataFuture.syncData.bIsRight ) )
	{
		m_PhysicParam.nowSpeed.x = GetWalkSpeed();
	}

	CommonEventProcess();
}
void CX2GUEL::JumpAttackXFrameMoveFuture()
{
	if( m_InputData.pureDown == true )
	{
		m_PhysicParam.nowSpeed.y -= m_PhysicParam.fGAccel * m_fElapsedTime;
		m_FrameDataFuture.stateParam.bEventFlagList[0] = true;
	}
	CommonFrameMoveFuture();
}
void CX2GUEL::DashComboZEventProcess()
{
	if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 1, 1, m_InputData.oneZ == true && CX2Unit::UC_ELESIS_SABER_KNIGHT == GetUnitClass() ,
										ESI_DASH_COMBO_SSK_ZZ )
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 2, 2, m_InputData.oneZ == true && CX2Unit::UC_ELESIS_PYRO_KNIGHT == GetUnitClass() ,
										ESI_DASH_COMBO_SPK_ZZ )
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 0, 0, m_InputData.oneZ == true, ESI_DASH_COMBO_ZZ )
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 3, 3, true == m_InputData.oneX && CX2Unit::UC_ELESIS_PYRO_KNIGHT == GetUnitClass() ,
										ESI_DASH_COMBO_SPK_ZX )
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )

	CommonEventProcess();
}
void CX2GUEL::DashComboZZEventProcess()
{
	if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
	WALK_CANCEL_AFTER( m_fWalkCancelAfter )
	BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
	DASH_CANCEL_AFTER( m_fDashCancelAfter )

	CommonEventProcess();
}
void CX2GUEL::DashComboXEventProcess()
{
	if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )

	CommonEventProcess();
}
void CX2GUEL::DashJumpComboZEventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() &&
		true == IsOnSomethingFuture() )
	{
		StateChange( USI_DASH_JUMP_LANDING );
	}
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 0, 0, m_InputData.oneZ == true, ESI_DASH_JUMP_COMBO_ZZ );

	CommonEventProcess();
}
void CX2GUEL::DashJumpComboZZEventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() &&
		true == IsOnSomethingFuture() )
	{
		StateChange( USI_DASH_JUMP_LANDING );
	}
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 0, 0, m_InputData.oneZ == true, ESI_DASH_JUMP_COMBO_ZZZ )

	CommonEventProcess();
}
void CX2GUEL::DashJumpComboZZZEventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		if ( true == IsOnSomethingFuture() )
		{
			StateChange( USI_DASH_JUMP_LANDING );
		};
	}

	CommonEventProcess();
}
void CX2GUEL::DashJumpComboXEventProcess()
{
	if ( true == IsOnSomethingFuture() )
	{
		StateChange( USI_DASH_JUMP_LANDING );
	}
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 1, 1, true == m_InputData.oneX && CX2Unit::UC_ELESIS_SABER_KNIGHT == GetUnitClass() ,
										ESI_DASH_JUMP_COMBO_SSK_XX )
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 2, 2, true == m_InputData.oneZ && CX2Unit::UC_ELESIS_PYRO_KNIGHT == GetUnitClass(),
										ESI_DASH_JUMP_COMBO_SPK_XZ )
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 0, 0, true == m_InputData.oneX , ESI_DASH_JUMP_COMBO_XX )

	CommonEventProcess();
}
void CX2GUEL::DashJumpComboXXEventProcess()
{
	if ( true == IsOnSomethingFuture() )
	{
		StateChange( ESI_DASH_JUMP_COMBO_XX_LANDING );
	}

	CommonEventProcess();
}
void CX2GUEL::DashJumpComboXXLandingEventProcess()
{
	if( false == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_DOWN );
		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
	}
	else if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}

	CommonEventProcess();
}
#ifdef ADD_CAN_NOT_ATTACK_JUMP_DOWN
void CX2GUEL::ESI_CAN_NOT_ATTACK_JUMP_DOWN_EventProcess()
{	
	if ( true == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_LANDING );
	}
	else if( (m_InputData.pureDoubleLeft == true || m_InputData.pureDoubleRight == true ) &&
		GetEnableDash() == true )
	{
		StateChange( USI_DASH_JUMP );
	}
	else if( true == m_FrameDataFuture.unitCondition.bFootOnWall && 
		( true == m_InputData.oneUp || true == m_InputData.oneDoubleUp ) )
	{
		m_bDisableGravity = true;
		StateChange( ESI_WALL_LANDING );
	}
	else if ( 0.0f <= m_PhysicParam.nowSpeed.y )
	{
		StateChange( USI_JUMP_UP );
	}
#ifdef RIDING_SYSTEM
	else if ( true == GetRidingOn() )
	{
		StateChange( USI_RIDING_ON );
	}
#endif //RIDING_SYSTEM

	CommonEventProcess();
}
#endif // ADD_CAN_NOT_ATTACK_JUMP_DOWN
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 스킬 스테이트 함수 - 노전직
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  닷지&슬래시
void CX2GUEL::ESI_SI_A_EL_DODGE_AND_SLASH_Init()
{
	TextureReadyInBackground(L"Colorballgray.dds");
	TextureReadyInBackground(L"RingAlpha.dds");
	TextureReadyInBackground(L"Smoke.dds");
	TextureReadyInBackground(L"waldo_trock_specialAttackB_particle.dds");
	XSkinMeshReadyInBackground(L"Mega_Slash_Trace_Dummy.x");
}
void CX2GUEL::ESI_SI_A_EL_DODGE_AND_SLASH_FrameMove()
{
	if( true == m_pXSkinAnim->EventTimer( 0.001f ) && true == EventCheck( 0.001f, false ) )
	{
		const CX2SkillTree::SkillTemplet* pSkillTemplet = GetEquippedActiveSkillTemplet( CX2SkillTree::SI_A_EL_DODGE_AND_SLASH );
		if( NULL != pSkillTemplet )
		{	// 회피 버프 적용
			SetBuffFactorToGameUnit( pSkillTemplet, 0 );
		}
	}

	CommonFrameMove();
}
void CX2GUEL::ESI_SI_A_EL_DODGE_AND_SLASH_StartFuture()
{
	CommonStateStartFuture();
	// 스킬키 재입력 체크를 위해 입력한 스킬키에 대한 정보 설정
	m_sCurrentSkillKeyManager.InitCurrentSkillKeyManager( m_iNowSpecialAttack - 1 );

	// 재사용 횟수 초기화
	m_uiDodgeAndSlashReuseCount = m_uiDodgeAndSlashMaxReuseCount;
}
void CX2GUEL::ESI_SI_A_EL_DODGE_AND_SLASH_EventProcess()
{
	// 스킬 슬롯 체인지 사용 여부 체크
	m_sCurrentSkillKeyManager.CheckSkillSlotSwap();

	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	else if ( m_uiDodgeAndSlashReuseCount > 0 &&
			  true == m_sCurrentSkillKeyManager.IsPressedCurrentSkillKey(m_InputData) )
	{
		StateChange( ESI_SI_A_EL_DODGE_AND_SLASH_SECOND );
	}
	else if ( true == m_InputData.oneZ || true == m_InputData.oneX )
	{
		StateChange( ESI_SI_A_EL_DODGE_AND_SLASH_ATTACK_Z );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
void CX2GUEL::ESI_SI_A_EL_DODGE_AND_SLASH_SECOND_StartFuture()
{
	CommonStateStartFuture();

	// 재사용 횟수 줄이기
	--m_uiDodgeAndSlashReuseCount;
}
/** @function : ESI_SI_A_EL_DODGE_AND_SLASH_ATTACK_Z_EventProcess
	@brief : 닷지&슬래시 중 Z&X 공격
*/
void CX2GUEL::ESI_SI_A_EL_DODGE_AND_SLASH_ATTACK_Z_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
// 도약
void CX2GUEL::ESI_SI_A_EL_LEAP_CRASH_Init()
{
	TextureReadyInBackground(L"Smoke.dds");
	TextureReadyInBackground(L"Colorballgray.dds");
	TextureReadyInBackground(L"GroundShockWave02.dds");
	XSkinMeshReadyInBackground(L"Mega_Slash_Trace_Dummy.x");
}
void CX2GUEL::ESI_SI_A_EL_LEAP_CRASH_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( ESI_SI_A_EL_LEAP_CRASH_JUMP );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )
	CommonEventProcess();
}
void CX2GUEL::ESI_SI_A_EL_LEAP_CRASH_JUMP_EventProcess()
{
	if( true == IsOnSomethingFuture() )
	{
		StateChange( ESI_SI_A_EL_LEAP_CRASH_JUMP_LANDING );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )
	CommonEventProcess();
}
void CX2GUEL::ESI_SI_A_EL_LEAP_CRASH_JUMP_LANDING_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
/** @function : ESI_SI_A_EL_RUSHING_SWORD_StartFuture
	@brief : 러싱 소드
			 2회 추가 공격에 대해서 모두 스테이트 를 분리 함.
			 각 스테이트별로 슈퍼아머를 다르게 설정하기 위한 분리.

*/
void CX2GUEL::ESI_SI_A_EL_RUSHING_SWORD_StartFuture()
{
	CommonStateStartFuture();
	m_sCurrentSkillKeyManager.InitCurrentSkillKeyManager( m_iNowSpecialAttack - 1 );
}
void CX2GUEL::ESI_SI_A_EL_RUSHING_SWORD_Init()
{
	TextureReadyInBackground(L"Arme_Ring2.dds");
	TextureReadyInBackground(L"Colorballgray.dds");
	XSkinMeshReadyInBackground(L"Mega_Slash_Trace_Dummy.x");
	TextureReadyInBackground(L"RingAlpha.dds");
	TextureReadyInBackground(L"Smoke.dds");
	TextureReadyInBackground(L"Whitecircle03.dds");
	TextureReadyInBackground(L"title_StarLight_Smash_spark.dds");
	TextureReadyInBackground(L"waldo_trock_specialAttackB_particle.dds");
}
void CX2GUEL::ESI_SI_A_EL_RUSHING_SWORD_EventProcess()
{
	m_sCurrentSkillKeyManager.CheckSkillSlotSwap();

	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	// 스크립트에서 SKILL_CANCEL_AFTER로 지정된 시간 이후, 스킬 키가 입력 되면 처리
	else if( m_fSkillCancelAfter >= 0.f && m_fSkillCancelAfter < m_pXSkinAnimFuture->GetNowAnimationTime() &&
		true == m_sCurrentSkillKeyManager.IsPressedCurrentSkillKey(m_InputData) )
	{ 
		// MP체크
		if ( GetNowMp() < m_fMPConsume )
		{
			g_pX2Game->GetInfoTextManager().PushText( XUF_DODUM_20_BOLD, GET_STRING( STR_ID_2549 ), D3DXCOLOR(1,1,1,1),
				D3DXCOLOR(0,0,0,1), DT_CENTER, 1.f, 1.f );
		}
		else
		{
			if( true == m_InputData.pureRight )
				m_FrameDataFuture.syncData.bIsRight = true;
			else if( true == m_InputData.pureLeft )
				m_FrameDataFuture.syncData.bIsRight = false;

			UpNowMp( -m_fMPConsume );
			StateChange( ESI_SI_A_EL_RUSHING_SWORD_2nd );
		}
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
void CX2GUEL::ESI_SI_A_EL_RUSHING_SWORD_2nd_EventProcess()
{
	m_sCurrentSkillKeyManager.CheckSkillSlotSwap();

	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	// 스크립트에서 SKILL_CANCEL_AFTER로 지정된 시간 이후, 스킬 키가 입력 되면 처리
	else if( m_fSkillCancelAfter >= 0.f && m_fSkillCancelAfter < m_pXSkinAnimFuture->GetNowAnimationTime() &&
		true == m_sCurrentSkillKeyManager.IsPressedCurrentSkillKey(m_InputData) )
	{ 
		// MP체크
		if ( GetNowMp() < m_fMPConsume )
		{
			g_pX2Game->GetInfoTextManager().PushText( XUF_DODUM_20_BOLD, GET_STRING( STR_ID_2549 ), D3DXCOLOR(1,1,1,1),
				D3DXCOLOR(0,0,0,1), DT_CENTER, 1.f, 1.f );
		}
		else
		{
			if( true == m_InputData.pureRight )
				m_FrameDataFuture.syncData.bIsRight = true;
			else if( true == m_InputData.pureLeft )
				m_FrameDataFuture.syncData.bIsRight = false;

			UpNowMp( -m_fMPConsume );
			StateChange( ESI_SI_A_EL_RUSHING_SWORD_3rd );
		}
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )
}
void CX2GUEL::ESI_SI_A_EL_RUSHING_SWORD_3rd_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

		CommonEventProcess();
}
// 발차기
void CX2GUEL::ESI_SI_A_EL_KICK_Start()
{
	CommonStateStart();
	const CX2SkillTree::SkillTemplet* pSkillTemplet = GetEquippedActiveSkillTemplet( CX2SkillTree::SI_A_EL_KICK );
	if( NULL != pSkillTemplet )
	{
		const CX2UserSkillTree& userSkillTree = GetUnit()->GetUnitData()->m_UserSkillTree;
		const int iSkillLevel = userSkillTree.GetSkillLevel( pSkillTemplet->m_eID );

		m_DamageData.fForceDownValue = -pSkillTemplet->GetSkillAbilityValue( CX2SkillTree::SA_FORCE_DOWN_ABS, iSkillLevel );
	}
}
void CX2GUEL::ESI_SI_A_EL_KICK_Init()
{
	TextureReadyInBackground( L"ColorBallGray.dds" );
	TextureReadyInBackground( L"Condense_Pulse01.dds" );
}
void CX2GUEL::ESI_SI_A_EL_KICK_EventProcess()
{	
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
// 메가 슬래시
void CX2GUEL::ESI_SI_A_EL_MEGA_SLASH_Init()
{
	TextureReadyInBackground(L"EL_FIRE_01.dds");
	TextureReadyInBackground(L"EL_FIRE_02.dds");
	TextureReadyInBackground(L"EL_FIRE_04.dds");
	TextureReadyInBackground(L"Explosion_Fire01.dds");
	TextureReadyInBackground(L"Explosion_Light01.dds");
	TextureReadyInBackground(L"Flame_REVOLVER.dds");
	TextureReadyInBackground(L"SA_EL_MEGA_SLASH_FIRE02.dds");
	TextureReadyInBackground(L"Whitecircle03.dds");
	XSkinMeshReadyInBackground(L"Mega_Slash_Fire_Dummy.x");
	XSkinMeshReadyInBackground(L"Mega_Slash_Trace_Dummy.x");
}
void CX2GUEL::ESI_SI_A_EL_MEGA_SLASH_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
// 소닉 어썰트 - 스탭
void CX2GUEL::ESI_SI_A_EL_SONIC_ASSAULT_STAB_Init()
{
	TextureReadyInBackground(L"EL_FIRE_01.dds");
	TextureReadyInBackground(L"EL_FIRE_02.dds");
	TextureReadyInBackground(L"EL_FIRE_03.dds");
	TextureReadyInBackground(L"EL_FIRE_04.dds");
	TextureReadyInBackground(L"EL_FIRE_07.dds");
	TextureReadyInBackground(L"Explosion_Fire01.dds");
	TextureReadyInBackground(L"Explosion_Light01.dds");
	TextureReadyInBackground(L"Flame_REVOLVER.dds");
	TextureReadyInBackground(L"SA_EL_MEGA_SLASH_FIRE02.dds");
	TextureReadyInBackground(L"Whitecircle03.dds");
	XSkinMeshReadyInBackground(L"Mega_Slash_Trace_Dummy.x");
}
void CX2GUEL::ESI_SI_A_EL_SONIC_ASSAULT_STAB_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
// 소닉 어썰트 - 스팅
void CX2GUEL::ESI_SI_A_EL_SONIC_ASSAULT_STING_Init()
{
	TextureReadyInBackground(L"EL_FIRE_01.dds");
	TextureReadyInBackground(L"EL_FIRE_02.dds");
	TextureReadyInBackground(L"EL_FIRE_03.dds");
	TextureReadyInBackground(L"EL_FIRE_04.dds");
	TextureReadyInBackground(L"EL_FIRE_07.dds");
	TextureReadyInBackground(L"Explosion_Fire01.dds");
	TextureReadyInBackground(L"Explosion_Light01.dds");
	TextureReadyInBackground(L"Flame_REVOLVER.dds");
	TextureReadyInBackground(L"SA_EL_MEGA_SLASH_FIRE02.dds");
	TextureReadyInBackground(L"Whitecircle03.dds");
	XSkinMeshReadyInBackground(L"Mega_Slash_Trace_Dummy.x");
}
void CX2GUEL::ESI_SI_A_EL_SONIC_ASSAULT_STING_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
// 와일드 쇼크
void CX2GUEL::ESI_SI_SA_EL_WILD_SHOCK_Init()
{
	TextureReadyInBackground(L"EL_FIRE_07.dds");
	TextureReadyInBackground(L"Explosion_Fire01.dds");
	TextureReadyInBackground(L"Explosion_Light01.dds");
	TextureReadyInBackground(L"Flame_REVOLVER.dds");
	TextureReadyInBackground(L"Whitecircle03.dds");
	TextureReadyInBackground(L"EL_FIRE_01.dds");
	TextureReadyInBackground(L"EL_FIRE_02.dds");
	TextureReadyInBackground(L"EL_FIRE_04.dds");
	TextureReadyInBackground(L"SA_EL_MEGA_SLASH_FIRE02.dds");
	TextureReadyInBackground(L"SA_EL_Wild_Shock_WAVE.tga");
	XSkinMeshReadyInBackground(L"Mega_Slash_Trace_Dummy.x");
	XSkinMeshReadyInBackground(L"SI_SA_EL_WILD_SHOCK_ATTACKBOX.X");
}
void CX2GUEL::ESI_SI_SA_EL_WILD_SHOCK_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
// 파워 버스터
void CX2GUEL::ESI_SI_SA_EL_POWER_BURSTER_Init()
{
	TextureReadyInBackground(L"EL_FIRE_01.dds");
	TextureReadyInBackground(L"EL_FIRE_02.dds");
	TextureReadyInBackground(L"EL_FIRE_03.dds");
	TextureReadyInBackground(L"EL_FIRE_07.dds");
	TextureReadyInBackground(L"EL_FIRE_10.dds");
	TextureReadyInBackground(L"Explosion_Fire01.dds");
	TextureReadyInBackground(L"Explosion_Light01.dds");
	TextureReadyInBackground(L"Whitecircle03.dds");
	TextureReadyInBackground(L"Flame_REVOLVER.dds");
	TextureReadyInBackground(L"Particle_Blur.DDS");
	TextureReadyInBackground(L"SA_EL_MEGA_SLASH_FIRE02.dds");
	XSkinMeshReadyInBackground(L"Mega_Slash_Trace_Dummy.x");
	XSkinMeshReadyInBackground(L"SA_EL_POWER_BURSTER_FIRE01.X");
	XSkinMeshReadyInBackground(L"Effect_ElSword_FlameGagerLand.X");
	XSkinMeshReadyInBackground(L"Elsword_SI_SA_Phoenix_Talon_Mesh02.X");
}
void CX2GUEL::ESI_SI_SA_EL_POWER_BURSTER_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
// 메가 버스터
void CX2GUEL::ESI_SI_SA_EL_MEGA_BURSTER_Init()			
{
	TextureReadyInBackground(L"AeroTornado04.dds");
	TextureReadyInBackground(L"Arme_Ring2.dds");
	TextureReadyInBackground(L"EL_FIRE_01.dds");
	TextureReadyInBackground(L"EL_FIRE_02.dds");
	TextureReadyInBackground(L"EL_FIRE_03.dds");
	TextureReadyInBackground(L"EL_FIRE_04.dds");
	TextureReadyInBackground(L"EL_FIRE_07.dds");
	TextureReadyInBackground(L"EL_FIRE_10.dds");
	TextureReadyInBackground(L"Flame_REVOLVER.dds");
	TextureReadyInBackground(L"Explosion_Light02.dds");
	TextureReadyInBackground(L"GroundShockWave02.dds");
	TextureReadyInBackground(L"SA_EL_MEGA_SLASH_FIRE02.dds");
	TextureReadyInBackground(L"Particle_Blur.DDS");
	TextureReadyInBackground(L"Particle_Blur.dds");
	XSkinMeshReadyInBackground(L"Light_Sigong_Box_Open01.X");
	XSkinMeshReadyInBackground(L"Mega_Slash_Trace_Dummy.x");
	XSkinMeshReadyInBackground(L"NUI_CUTTYSARK_Dying_Landing_Effect_Hole.X");
	XSkinMeshReadyInBackground(L"NUI_CUTTYSARK_Dying_Landing_Effect_Rock.X");
	XSkinMeshReadyInBackground(L"Elsword_SI_SA_Phoenix_Talon_Mesh02.X");
	XSkinMeshReadyInBackground(L"SA_EL_MEGA_BURSTER_FIRE_CIRCLE.X");
	XSkinMeshReadyInBackground(L"SA_EL_MEGA_BURSTER_FIRE_Earth.X");
}
void CX2GUEL::ESI_SI_SA_EL_MEGA_BURSTER_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
// 언리미티드 블레이드
void CX2GUEL::ESI_SI_SA_EL_UNLIMITED_BLADE_Init()
{
	TextureReadyInBackground(L"AeroTornado04.dds");
	TextureReadyInBackground(L"Arme_Ring2.dds");
	TextureReadyInBackground(L"COMET_CRASHER_Explosion_02.tga");
	TextureReadyInBackground(L"ColorBallFire.dds");
	TextureReadyInBackground(L"Explosion_Fire01.dds");
	TextureReadyInBackground(L"Explosion_Light01.dds");
	TextureReadyInBackground(L"Explosion_Light02.dds");
	TextureReadyInBackground(L"GroundShockWave02.dds");
	TextureReadyInBackground(L"Mesh_Raven_Event_AC_Upbody21_Effect.tga");
	TextureReadyInBackground(L"Particle_Blur.dds");
	TextureReadyInBackground(L"Smoke.dds");
	TextureReadyInBackground(L"Whitecircle03.dds");
	XSkinMeshReadyInBackground(L"Effect_ElSword_FlameGagerLand.X");
	XSkinMeshReadyInBackground(L"Mega_Slash_Trace_Dummy.x");
	XSkinMeshReadyInBackground(L"elsword_SA_SwordBlasting_circle.X");
}
void CX2GUEL::ESI_SI_SA_EL_UNLIMITED_BLADE_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
// 극기 - 강(强)
void CX2GUEL::ESI_SI_A_EL_ENDURANCE_POWER_ACTIVE_Init()
{
	TextureReadyInBackground(L"Arme_Ring2.dds");
	TextureReadyInBackground(L"ColorBallGray.dds");
	XSkinMeshReadyInBackground(L"HummingWind_AttackBox.X");
}
void CX2GUEL::ESI_SI_A_EL_ENDURANCE_POWER_ACTIVE_EventProcess()
{
	// 극기 상태에서 피격시 활성화 되는 스테이트 
 	if( false == IsOnSomethingFuture() )
 	{
#ifdef ADD_CAN_NOT_ATTACK_JUMP_DOWN
		StateChange( ESI_CAN_NOT_ATTACK_JUMP_DOWN );
#else
		StateChange( GetJumpDownStateID() );
#endif // ADD_CAN_NOT_ATTACK_JUMP_DOWN
 		m_FrameDataFuture.syncData.position.y -= LINE_RADIUS * 1.5f;
 	}
	else if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( GetWaitStateID() );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
void CX2GUEL::ESI_SI_A_EL_ENDURANCE_POWER_ACTIVE_StateStart()
{
	m_fGroggyTime = 0.f;
	m_PhysicParam.nowSpeed.x = 0.0f;
	m_PhysicParam.nowSpeed.y = 0.0f;

	CureExtraDamage( CX2DamageManager::EDT_ENCHANT_SHOCK );
	CureExtraDamage( CX2DamageManager::EDT_FROZEN );
	CureExtraDamage( CX2DamageManager::EDT_ENTANGLE );
	CureExtraDamage( CX2DamageManager::EDT_DRYAD );
	CureExtraDamage( CX2DamageManager::EDT_STUN );
	CureExtraDamage( CX2DamageManager::EDT_Y_PRESSED );
	CureExtraDamage( CX2DamageManager::EDT_PANIC );

	EraseBuffTempletFromGameUnit(BTI_DEBUFF_STUN);	
	EraseBuffTempletFromGameUnit(BTI_DEBUFF_PANIC);	
	EraseBuffTempletFromGameUnit(BTI_DEBUFF_COLD);	
	EraseBuffTempletFromGameUnit(BTI_DEBUFF_CONFUSION);	
	EraseBuffTempletFromGameUnit(BTI_BUFF_INDURANCE_POWER);	

	CommonStateStart();
}
void CX2GUEL::DoStateEndurancePowerRage()
{
	StateChange( ESI_SA_A_EL_ENDURANCE_POWER_ACTIVE, false );
}
void CX2GUEL::ProcessEndurancePowerSkill()
{	// 극기 - 강(强)
	const CX2SkillTree::SkillTemplet* pSkillTemplet = GetEquippedActiveSkillTemplet( CX2SkillTree::SI_A_EL_ENDURANCE_POWER );
	if( NULL != pSkillTemplet )
	{
		SetBuffFactorToGameUnit( pSkillTemplet, 0 );

		// 스킬 지속 시간 중 재 사용 했을 때 처리를 위해, 
		// 이펙트셋은 버프로 넣지 않고 따로 생성 함.
		g_pX2Game->GetEffectSet()->PlayEffectSet( L"EffectSet_ELSWORD_ENDURANCE", this ); 
		PlaySound( L"Elsword_Endurance.ogg" );	
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 스킬 스테이트 함수 - 1-1차 세이버 나이트
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CX2GUEL::ESI_DASH_COMBO_SSK_ZZ_EventProcess()			/// DASH_COMBO_ZZ
{
	if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 0, 0, true == m_InputData.oneZ, ESI_DASH_COMBO_SSK_ZZZ )
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 1, 1, true == m_InputData.oneX, ESI_DASH_COMBO_SSK_ZZX )
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
	WALK_CANCEL_AFTER( m_fWalkCancelAfter )
	BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
	DASH_CANCEL_AFTER( m_fDashCancelAfter )
	ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
void CX2GUEL::ESI_DASH_COMBO_SSK_ZZZ_EventProcess()			/// DASH_COMBO_ZZZ
{
	if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
void CX2GUEL::ESI_DASH_COMBO_SSK_ZZX_EventProcess()			/// DASH_COMBO_ZZX
{
	if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
/** @function : ESI_DASH_JUMP_COMBO_SSK_XX_EventProcess
	@brief : 점프 상태에서 내려오면서 입력을 받고,
			 랜딩에서 바로 스테이트 변경시키기 위한 작업
*/
void CX2GUEL::ESI_DASH_JUMP_COMBO_SSK_XX_EventProcess()		/// DASH_JUMP_COMBO_XX
{
	bool& bESI_DASH_JUMP_COMBO_SSK_XXX = m_FrameDataFuture.stateParam.bEventFlagList[0];
	bool& bESI_COMBO_Z = m_FrameDataFuture.stateParam.bEventFlagList[1];

	if( true == m_InputData.oneX )
		bESI_DASH_JUMP_COMBO_SSK_XXX = true;
	else if ( true == m_InputData.oneZ )
		bESI_COMBO_Z = true;

	if ( true == IsOnSomethingFuture() )
	{
		if( true == bESI_COMBO_Z )
			StateChange( ESI_COMBO_Z );
		else if( true == bESI_DASH_JUMP_COMBO_SSK_XXX )
			StateChange( ESI_DASH_JUMP_COMBO_SSK_XXX );
		else
			StateChange( ESI_DASH_JUMP_COMBO_XX_LANDING );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )
	CommonEventProcess();
}
void CX2GUEL::ESI_DASH_JUMP_COMBO_SSK_XXX_EventProcess()		/// DASH_JUMP_COMBO_XXX
{
	if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		if ( false == IsOnSomethingFuture() )
			StateChange( USI_JUMP_DOWN );
		else
			StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

		CommonEventProcess();
}
// 크로스 슬래시
void CX2GUEL::ESI_SI_A_SSK_CROSS_SLASH_Init()
{
	TextureReadyInBackground(L"Arme_Ring2.dds");
	TextureReadyInBackground(L"Mesh_Raven_Event_AC_Upbody21_Effect.tga");
	TextureReadyInBackground(L"Particle_Blur.DDS");
	TextureReadyInBackground(L"Particle_Blur.dds");
	TextureReadyInBackground(L"WhitePoint.dds");
	TextureReadyInBackground(L"Whitecircle02.dds");
	TextureReadyInBackground(L"title_StarLight_Smash_spark.dds");
	XSkinMeshReadyInBackground(L"SA_SSK_EXTINCTION_END_FX_Blade.X");
	XSkinMeshReadyInBackground(L"Lire_SI_A_Low_Kick_Mesh02.X");
	XSkinMeshReadyInBackground(L"AirSlash02.X");
}
void CX2GUEL::ESI_SI_A_SSK_CROSS_SLASH_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}

// 파워 브레이크
void CX2GUEL::ESI_SI_A_SSK_POWER_BREAK_Init()
{
	TextureReadyInBackground(L"Arme_Ring2.dds");
	TextureReadyInBackground(L"ColorBallFire.dds");
}
void CX2GUEL::ESI_SI_A_SSK_POWER_BREAK_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
// 헤비 스터너
void CX2GUEL::ESI_SI_A_SSK_HEAVY_STUNNER_Init()
{
	TextureReadyInBackground(L"Arme_Ring2.dds");
	TextureReadyInBackground(L"ColorBallBlue.dds");
}
void CX2GUEL::ESI_SI_A_SSK_HEAVY_STUNNER_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
// 스파이럴 블래스트
void CX2GUEL::ESI_SI_SA_SSK_SPIRAL_BLAST_Init()	
{
	TextureReadyInBackground(L"ColorBallFire.dds");
	TextureReadyInBackground(L"EL_FIRE_01.dds");
	TextureReadyInBackground(L"EL_FIRE_02.dds");
	TextureReadyInBackground(L"EL_FIRE_03.dds");
	TextureReadyInBackground(L"EL_FIRE_04.dds");
	TextureReadyInBackground(L"EL_FIRE_06.dds");
	TextureReadyInBackground(L"Flame_REVOLVER.dds");
	XSkinMeshReadyInBackground(L"Mega_Slash_Trace_Dummy.x");
	TextureReadyInBackground(L"Mesh_Elsword_Event_AC_Upbody21A_Effect.tga");
	TextureReadyInBackground(L"Particle_Blur.dds");
	TextureReadyInBackground(L"SA_EL_MEGA_SLASH_FIRE02.dds");
	XSkinMeshReadyInBackground(L"SA_EL_SA_SSK_SPIRAL_BLAST_01.X");
	XSkinMeshReadyInBackground(L"SA_EL_SA_SSK_SPIRAL_BLAST_03.X");
	XSkinMeshReadyInBackground(L"SpiralBlast01.X");
}
void CX2GUEL::ESI_SI_SA_SSK_SPIRAL_BLAST_EventProcess()
{	
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
// 세이버 - 익스팅션
#pragma region 세이버 - 익스팅션
void CX2GUEL::InitExtinction()
{
	// 충전 지속 시간 초기화
	m_fExtinctionRemainTime = 0.f;

	// 발사 각도 초기화
	m_fExtinctionDegreeZ = _CONST_EL_::BASE_EXTINCTION_DEGREE_Z;
}
void CX2GUEL::CreateExctionUIEffect()
{
	if( m_pXSkinAnim->EventTimer( 0.001f ) == true && EventCheck( 0.001f, false ) == true )
	{
		// 각도 조절 상태를 알려주기 위한 UI 이펙트 
		if( INVALID_PARTICLE_HANDLE == m_hExctionBG )
		{
			// 각도기 배경UI 생성
			wstring wstrParticleName = L"SA_SSK_EXTINCTION_CHARGE_P14_";
			wstrParticleName += ( m_FrameDataFuture.syncData.bIsRight == true ) ? L"R" : L"L";
			m_hExctionBG = g_pData->GetGameMajorParticle()->CreateSequenceHandle( this,  wstrParticleName.c_str(), GetBonePos(L"Bip01") );	

			CKTDGParticleSystem::CParticleEventSequence* pSeq = 
				g_pData->GetGameMajorParticle()->GetInstanceSequence( m_hExctionBG );
			if( NULL != pSeq )
			{
				pSeq->CreateNewParticle(GetBonePos(L"Bip01"));
			}
		}
		// 각도 조절 상태를 알려주기 위한 UI 이펙트 
		if( INVALID_PARTICLE_HANDLE == m_hExtinctionArrow )
		{
			// 각도기 UI 생성
			m_hExtinctionArrow = g_pData->GetGameMajorParticle()->CreateSequenceHandle( this,  L"SA_SSK_EXTINCTION_CHARGE_P15", GetBonePos(L"Bip01") );	
			if( m_pExtinctionArrow == NULL )
			{
				CKTDGParticleSystem::CParticleEventSequence* pSeq = 
					g_pData->GetGameMajorParticle()->GetInstanceSequence( m_hExtinctionArrow );
				if( NULL != pSeq )
				{
					m_pExtinctionArrow = pSeq->CreateNewParticle(GetBonePos(L"Bip01"));
				}
			}
		}
	}
}
void CX2GUEL::DestoryExctionUIEffect()
{
	g_pData->GetGameMajorParticle()->DestroyInstanceHandle( m_hExtinctionArrow );
	g_pData->GetGameMajorParticle()->DestroyInstanceHandle( m_hExctionBG );
	m_hExtinctionArrow = INVALID_PARTICLE_SEQUENCE_HANDLE;
	m_hExctionBG = INVALID_PARTICLE_SEQUENCE_HANDLE;
	m_pExtinctionArrow = NULL;
}
void CX2GUEL::Extinction_FrameMoveFuture()
{
	CreateExctionUIEffect();
	// 각도 조절 상태를 알려주기 위한 UI 이펙트의 각도 조절
	if( NULL != m_pExtinctionArrow )
	{
		D3DXVECTOR3 vRot(0, (m_FrameDataFuture.syncData.bIsRight == true ) ? 0.f : 180.f ,m_fExtinctionDegreeZ );
		m_pExtinctionArrow->m_vAxisRotateDegree = vRot;
		m_pExtinctionArrow->m_vRotate = vRot;

		CKTDGParticleSystem::CParticleEventSequence* pSeq = g_pData->GetGameMajorParticle()->GetInstanceSequence( m_hExtinctionArrow );
		if( NULL != pSeq )
		{
			pSeq->SetAddRotate( vRot );
			pSeq->SetAxisAngle( vRot );
		}
	}
}
// 세이버 - 익스팅션
void CX2GUEL::ESI_SI_SA_SSK_EXTINCTION_START_Init()				
{
	TextureReadyInBackground(L"ColorBallFire.dds");
	TextureReadyInBackground(L"Mesh_Raven_Event_AC_Upbody21_Effect.tga");
	TextureReadyInBackground(L"Condense_Light01.dds");
	TextureReadyInBackground(L"Explosion_Fire01.dds");
	TextureReadyInBackground(L"Explosion_Light01.dds");
	TextureReadyInBackground(L"Lire_Kick_Impact01.dds");
	TextureReadyInBackground(L"Particle_Blur.dds");
	TextureReadyInBackground(L"WhitePoint.dds");
	TextureReadyInBackground(L"Whitecircle03.dds");
	XSkinMeshReadyInBackground(L"Mega_Slash_Trace_Dummy.x");
	XSkinMeshReadyInBackground(L"EXTINCTION_ATTACK_DUMMY.X");
	XSkinMeshReadyInBackground(L"SA_SSK_EXTINCTION_END_FX_Blade.X");
}
void CX2GUEL::ESI_SI_SA_SSK_EXTINCTION_START_Start()
{
	InitExtinction();
	CommonStateStart();
}
void CX2GUEL::ESI_SI_SA_SSK_EXTINCTION_START_StartFuture()
{
	// 익스팅션 충전 완료를 체크하기 위한 스킬 키 설정
	m_sCurrentSkillKeyManager.InitCurrentSkillKeyManager( m_iNowSpecialAttack - 1 );
	
#ifdef FIX_NOT_DRAWING_EXTINCTION_GUIDE_UI
	DestoryExctionUIEffect();	
#endif // FIX_NOT_DRAWING_EXTINCTION_GUIDE_UI

	CommonStateStartFuture();
}
void CX2GUEL::ESI_SI_SA_SSK_EXTINCTION_START_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( ESI_SI_SA_SSK_EXTINCTION_CHARGE );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )
	CommonEventProcess();
}
void CX2GUEL::ESI_SI_SA_SSK_EXTINCTION_CHARGE_FrameMove()
{
	m_fExtinctionRemainTime += m_fElapsedTime;

	CommonFrameMove();
}
void CX2GUEL::ESI_SI_SA_SSK_EXTINCTION_CHARGE_FrameMoveFuture()
{
	// 이펙트 각도 조절
	Extinction_FrameMoveFuture();

	CommonFrameMoveFuture();
}
void CX2GUEL::ESI_SI_SA_SSK_EXTINCTION_CHARGE_EventProcess()
{
	m_sCurrentSkillKeyManager.CheckSkillSlotSwap();

	// 최대 충전 시간 초과 시 공격 모션으로 변경
	if( m_fExtinctionRemainTime >= _CONST_EL_::MAX_EXTINCTION_CHARGE_TIME )
	{
		StateChange( ESI_SI_SA_SSK_EXTINCTION_ATTACK );		
	}
	else if( true == m_sCurrentSkillKeyManager.IsUnpressedCurrentSkillKey( m_InputData ) )
	{
		StateChange( ESI_SI_SA_SSK_EXTINCTION_ATTACK );
	}
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 0, 0, true == m_InputData.pureUp, ESI_SI_SA_SSK_EXTINCTION_DEGREE_UP )
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 1, 0, true == m_InputData.pureDown, ESI_SI_SA_SSK_EXTINCTION_DEGREE_DOWN )
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
	WALK_CANCEL_AFTER( m_fWalkCancelAfter )
	BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
	DASH_CANCEL_AFTER( m_fDashCancelAfter )
	ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
/// 세이버 - 익스팅션 - Attack
void CX2GUEL::ESI_SI_SA_SSK_EXTINCTION_ATTACK_FrameMove()
{
	if( m_pXSkinAnim->EventTimer( 0.001f ) == true && EventCheck( 0.001f, false ) == true )
	{
		// 충전 시간에 따라 공격력 최대 20%증가.
		m_fExtinctionRemainTime = max( m_fExtinctionRemainTime, 0.f );
		float fMultiplePowerRateRel = ( min( m_fExtinctionRemainTime, 1.f ) * _CONST_EL_::LIMIT_EXTINCTION_POWER_RATE ) + 1.f;

		CX2EffectSet::Handle hEffect = 
			g_pX2Game->GetEffectSet()->PlayEffectSet( L"EffectSet_SA_SSK_EXTINCTION_DAMAGE", this ,
														NULL, IsHyperState(), GetPowerRate() * fMultiplePowerRateRel );

		if( CX2EffectSet::INVALID_HANDLE != hEffect )
		{
			CX2EffectSet::EffectSetInstance* pEffectSet = g_pX2Game->GetEffectSet()->GetEffectSetInstance( hEffect );
			if( NULL != pEffectSet )
			{	// 키 입력을 통해 조절한 각도로 이펙트 생성
				pEffectSet->SetLocalRotateDegree( D3DXVECTOR3(0.f, 0.f, m_fExtinctionDegreeZ ) ) ;
			}
		}
	}

	CommonFrameMove();
}
void CX2GUEL::ESI_SI_SA_SSK_EXTINCTION_ATTACK_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() &&
		true == IsOnSomethingFuture() )
	{
		StateChange( USI_JUMP_LANDING );
	}
	else if( false == IsOnSomethingFuture() )
	{
		// 현재 진행 방향 쪽으로 키보드를 누르면 앞으로 좀 더 나아감
		if( ( true == m_InputData.pureRight && true == m_FrameDataFuture.syncData.bIsRight ) ||
			( true == m_InputData.pureLeft && false == m_FrameDataFuture.syncData.bIsRight ) )
		{
			m_PhysicParam.nowSpeed.x = GetWalkSpeed();
		}
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
void CX2GUEL::ESI_SI_SA_SSK_EXTINCTION_ATTACK_StartFuture() 
{
	DestoryExctionUIEffect();
	CommonStateStartFuture();
}
void CX2GUEL::ESI_SI_SA_SSK_EXTINCTION_DEGREE_UP_FrameMove()	// 각도 조절
{
	m_fExtinctionRemainTime += m_fElapsedTime;

	if( m_fExtinctionDegreeZ < _CONST_EL_::MAX_EXTINCTION_DEGREE_Z )
		m_fExtinctionDegreeZ += m_fElapsedTime * _CONST_EL_::EXTINCTION_DEGREE_Z_CONTROL;
	else
		m_fExtinctionDegreeZ = _CONST_EL_::MAX_EXTINCTION_DEGREE_Z;

	CommonFrameMove();
}
void CX2GUEL::ESI_SI_SA_SSK_EXTINCTION_DEGREE_UP_EventProcess()
{
	m_sCurrentSkillKeyManager.CheckSkillSlotSwap();

	// 최대 충전 시간 초과 시 공격 모션으로 변경
	if( m_fExtinctionRemainTime >= _CONST_EL_::MAX_EXTINCTION_CHARGE_TIME )
	{
		StateChange( ESI_SI_SA_SSK_EXTINCTION_ATTACK );		
	}
	else if( true == m_sCurrentSkillKeyManager.IsUnpressedCurrentSkillKey( m_InputData ) )
	{
		StateChange( ESI_SI_SA_SSK_EXTINCTION_ATTACK );
	}
	// 방향키를 통해 발사 각도 조절
	else if( true == m_InputData.pureDown )
	{
		StateChange( ESI_SI_SA_SSK_EXTINCTION_DEGREE_DOWN );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
void CX2GUEL::ESI_SI_SA_SSK_EXTINCTION_DEGREE_UP_FrameMoveFuture()
{
	// 이펙트 각도 조절
	Extinction_FrameMoveFuture();

	CommonFrameMoveFuture();
}
void CX2GUEL::ESI_SI_SA_SSK_EXTINCTION_DEGREE_DOWN_FrameMove()	// 각도 조절
{
	m_fExtinctionRemainTime += m_fElapsedTime;

	if( m_fExtinctionDegreeZ > _CONST_EL_::MIN_EXTINCTION_DEGREE_Z )
		m_fExtinctionDegreeZ -= m_fElapsedTime * _CONST_EL_::EXTINCTION_DEGREE_Z_CONTROL;
	else
		m_fExtinctionDegreeZ = _CONST_EL_::MIN_EXTINCTION_DEGREE_Z; 

	CommonFrameMove();
}
void CX2GUEL::ESI_SI_SA_SSK_EXTINCTION_DEGREE_DOWN_FrameMoveFuture()
{
	// 이펙트 각도 조절
	Extinction_FrameMoveFuture();

	CommonFrameMoveFuture();
}
void CX2GUEL::ESI_SI_SA_SSK_EXTINCTION_DEGREE_DOWN_EventProcess()
{
	m_sCurrentSkillKeyManager.CheckSkillSlotSwap();

	// 최대 충전 시간 초과 시 공격 모션으로 변경
	if( m_fExtinctionRemainTime >= _CONST_EL_::MAX_EXTINCTION_CHARGE_TIME )
	{
		StateChange( ESI_SI_SA_SSK_EXTINCTION_ATTACK );		
	}
	else if( true == m_sCurrentSkillKeyManager.IsUnpressedCurrentSkillKey( m_InputData ) )
	{
		StateChange( ESI_SI_SA_SSK_EXTINCTION_ATTACK );
	}
	// 방향키를 통해 발사 각도 조절
	else if( true == m_InputData.pureUp )
	{
		StateChange( ESI_SI_SA_SSK_EXTINCTION_DEGREE_UP );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
#pragma endregion 세이버 - 익스팅션
// 승리의 검
void CX2GUEL::ESI_SI_SA_SSK_VICTORIOUS_SWORD_Init()
{
	TextureReadyInBackground(L"AeroTornado04.dds");
	TextureReadyInBackground(L"Arme_Ring2.dds");
	TextureReadyInBackground(L"ColorBallFire.dds");
	TextureReadyInBackground(L"EL_FIRE_03.dds");
	TextureReadyInBackground(L"EL_FIRE_04.dds");
	TextureReadyInBackground(L"Explosion_Light02.dds");
	TextureReadyInBackground(L"GroundShockWave02.dds");
	XSkinMeshReadyInBackground(L"HummingWind_AttackBox.X");
	XSkinMeshReadyInBackground(L"Mega_Slash_Trace_Dummy.x");
	TextureReadyInBackground(L"Mesh_Raven_Event_AC_Upbody21_Effect.tga");
	TextureReadyInBackground(L"Particle_Blur.dds");
	XSkinMeshReadyInBackground(L"SA_SPK_INFERNAL_BLADE_Weapon_Dummy.x");
	TextureReadyInBackground(L"Whitecircle02.dds");
	TextureReadyInBackground(L"Whitecircle03.dds");
	TextureReadyInBackground(L"title_StarLight_Smash_spark.dds");
}
void CX2GUEL::ESI_SI_SA_SSK_VICTORIOUS_SWORD_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
// 저지먼트 드라이브
void CX2GUEL::ESI_SI_SA_SSK_JUDGEMENT_DRIVE_Init()
{
	TextureReadyInBackground(L"AeroTornado04.dds");
	TextureReadyInBackground(L"Arme_Ring2.dds");
	TextureReadyInBackground(L"CenterLight_Gray01.dds");
	TextureReadyInBackground(L"ColorBallFire.dds");
	TextureReadyInBackground(L"Condense_Pulse02.dds");
	XSkinMeshReadyInBackground(L"Elsword_Double_Slash_Mesh01.X");
	XSkinMeshReadyInBackground(L"Elsword_Double_Slash_Mesh02.X");
	TextureReadyInBackground(L"Explosion_Light02.dds");
	TextureReadyInBackground(L"GroundShockWave02.dds");
	XSkinMeshReadyInBackground(L"Lire_SI_SA_Gungnir_Mesh05.X");
	XSkinMeshReadyInBackground(L"Mega_Slash_Trace_Dummy.x");
	TextureReadyInBackground(L"Particle_Blur.dds");
	XSkinMeshReadyInBackground(L"aisha_active_energySpurt_circle.X");
}
void CX2GUEL::ESI_SI_SA_SSK_JUDGEMENT_DRIVE_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 스킬 스테이트 함수 - 2-1차 파이로 나이트
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CX2GUEL::ESI_DASH_COMBO_SPK_ZX_EventProcess()			// ESI_DASH_COMBO_SPK_ZX
{
	if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )	{		StateChange( USI_WAIT );	}	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )	WALK_CANCEL_AFTER( m_fWalkCancelAfter )	BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )	DASH_CANCEL_AFTER( m_fDashCancelAfter )
	ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
void CX2GUEL::ESI_DASH_COMBO_SPK_ZX_FrameMove()
{
	if( m_pXSkinAnim->EventTimer( 0.233f ) == true && EventCheck( 0.233f, false ) == true )
	{
		if(	true == FlushMp( _CONST_EL_::FIRE_BALL_MP_COST ) )
			g_pX2Game->GetEffectSet()->PlayEffectSet( L"EffectSet_A_EL_Dash_Combo_ZX", this, NULL, IsHyperState(), GetPowerRate()  * GetFireBallPowerRate() );
		else
			g_pX2Game->GetEffectSet()->PlayEffectSet( L"EffectSet_A_EL_Dash_Combo_ZX_FAIL", this );
	}
	CommonFrameMove();
}
void CX2GUEL::ESI_DASH_COMBO_SPK_ZZ_EventProcess()			// DASH_COMBO_ZZ
{
	if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT( 0, 0, true == m_InputData.oneZ, ESI_DASH_COMBO_SPK_ZZZ )
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
	WALK_CANCEL_AFTER( m_fWalkCancelAfter )
	BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
	DASH_CANCEL_AFTER( m_fDashCancelAfter )
	ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
void CX2GUEL::ESI_DASH_COMBO_SPK_ZZZ_EventProcess()			// DASH_COMBO_ZZZ
{
	if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
void CX2GUEL::ESI_DASH_COMBO_SPK_ZZZ_FrameMove()
{
	if( m_pXSkinAnim->EventTimer( 0.266f ) == true && EventCheck( 0.266f, false ) == true )
	{
		if(	true == FlushMp( _CONST_EL_::FIRE_BALL_MP_COST ) )
			g_pX2Game->GetEffectSet()->PlayEffectSet( L"EffectSet_A_EL_Dash_Combo_ZZZ", this, NULL, IsHyperState(), GetPowerRate()  * GetFireBallPowerRate() );
		else
			g_pX2Game->GetEffectSet()->PlayEffectSet( L"EffectSet_A_EL_Dash_Combo_ZZZ_FAIL", this );
	}
	CommonFrameMove();
}
void CX2GUEL::ESI_DASH_JUMP_COMBO_SPK_XZ_EventProcess()		// DASH_JUMP_COMBO_XZ
{
	if ( true == IsOnSomethingFuture() )
	{
		StateChange( USI_DASH_JUMP_LANDING );
	}
	else if ( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_JUMP_DOWN );
	}
	ELSE_IF_STATE_CHANGE_ON_EX_STRUCT(0,0, true == m_InputData.oneZ && 0 < m_uiSPKXZReattakCount, ESI_DASH_JUMP_COMBO_SPK_XZ_REATTACK)
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

		CommonEventProcess();
}
void CX2GUEL::ESI_DASH_JUMP_COMBO_SPK_XZ_FrameMove()
{
	if( m_pXSkinAnim->EventTimer( 0.2f ) == true && EventCheck( 0.2f, false ) == true )
	{
		if(	true == FlushMp( _CONST_EL_::SPK_DASH_JUMP_XZ_MP_COST ) )
			g_pX2Game->GetEffectSet()->PlayEffectSet( L"EffectSet_A_EL_DashJump_Combo_XZ", this, NULL, IsHyperState(), GetPowerRate() * GetFireBallPowerRate() );
		else
			g_pX2Game->GetEffectSet()->PlayEffectSet( L"EffectSet_A_EL_DashJump_Combo_XZ_FAIL", this );
	}

	CommonFrameMove();
}
/** @function : ESI_DASH_JUMP_COMBO_SPK_ZX_StartFurture
	@brief : ESI_DASH_JUMP_COMBO_SPK_ZX 이후 추가 X공격(파이어볼) 구현을 위한 함수
*/
void CX2GUEL::ESI_DASH_JUMP_COMBO_SPK_XZ_StartFurture()
{
	m_bIsDashAttackPossible = false;
	m_uiSPKXZReattakCount = _CONST_EL_::MAX_DASH_JUMP_COMBO_SPK_XZ_REATTACK;
	CommonStateStartFuture();
}
/** @function : ESI_DASH_JUMP_COMBO_SPK_ZX_REATTACK_StartFurture
	@brief : ESI_DASH_JUMP_COMBO_SPK_ZX 이후 추가 X공격(파이어볼) 구현을 위한 함수
*/
void CX2GUEL::ESI_DASH_JUMP_COMBO_SPK_XZ_REATTACK_StartFurture()
{
	--m_uiSPKXZReattakCount;
	CommonStateStartFuture();
}
// 소드 이럽션
void CX2GUEL::ESI_SI_A_SPK_SWORD_ERUPTION_Init()
{
	XSkinMeshReadyInBackground(L"A_SPK_SWORD_ERUPTION_BLADE.x");
	XSkinMeshReadyInBackground(L"A_SPK_SWORD_ERUPTION_Square.x");
	TextureReadyInBackground(L"AeroTornado04.dds");
	TextureReadyInBackground(L"Arme_Ring2.dds");
	TextureReadyInBackground(L"COMET_CRASHER_Explosion_02.tga");
	TextureReadyInBackground(L"EL_FIRE_01.dds");
	TextureReadyInBackground(L"EL_FIRE_02.dds");
	TextureReadyInBackground(L"EL_FIRE_04.dds");
	TextureReadyInBackground(L"EL_FIRE_07.dds");
	TextureReadyInBackground(L"Explosion_Light02.dds");
	TextureReadyInBackground(L"Flame_REVOLVER.dds");
	XSkinMeshReadyInBackground(L"Lire_SI_SA_Gungnir_Mesh05.X");
	XSkinMeshReadyInBackground(L"Mega_Slash_Trace_Dummy.x");
	TextureReadyInBackground(L"Particle_Blur.DDS");
}
void CX2GUEL::ESI_SI_A_SPK_SWORD_ERUPTION_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
// 대폭살
void CX2GUEL::ESI_SI_A_SPK_BIG_BURST_Init()					
{
	TextureReadyInBackground(L"Arme_Ring2.dds");
	TextureReadyInBackground(L"ColorBallFire.dds");
	TextureReadyInBackground(L"EL_FIRE_03.dds");
	TextureReadyInBackground(L"EL_FIRE_06.dds");
	TextureReadyInBackground(L"EL_FIRE_09.dds");
	XSkinMeshReadyInBackground(L"DummyAttackBox_50x50x50.X");
}
void CX2GUEL::ESI_SI_A_SPK_BIG_BURST_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
// 버스트 라이징
void CX2GUEL::ESI_SI_SA_SPK_BURST_RISING_Init()		
{
	XSkinMeshReadyInBackground(L"A_SPK_SWORD_ERUPTION_BLADE.x");
	XSkinMeshReadyInBackground(L"A_SPK_SWORD_ERUPTION_Square.x");
	TextureReadyInBackground(L"Arme_Ring2.dds");
	TextureReadyInBackground(L"ColorBallFire.dds");
	TextureReadyInBackground(L"EL_FIRE_03.dds");
	TextureReadyInBackground(L"EL_FIRE_06.dds");
	TextureReadyInBackground(L"EL_FIRE_09.dds");
	XSkinMeshReadyInBackground(L"Lire_SI_SA_Gungnir_Mesh05.X");
	XSkinMeshReadyInBackground(L"SA_EL_MEGA_BURSTER_FIRE_CIRCLE.X");
	XSkinMeshReadyInBackground(L"SA_EL_MEGA_BURSTER_FIRE_Earth.X");
}
void CX2GUEL::ESI_SI_SA_SPK_BURST_RISING_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
// 버스트 웨이브
void CX2GUEL::ESI_SI_SA_SPK_BURST_WAVE_Init()	
{
	TextureReadyInBackground(L"AeroTornado04.dds");
	TextureReadyInBackground(L"Arme_Ring2.dds");
	TextureReadyInBackground(L"Condense_Pulse02.dds");
	TextureReadyInBackground(L"EL_FIRE_01.dds");
	TextureReadyInBackground(L"EL_FIRE_02.dds");
	TextureReadyInBackground(L"EL_FIRE_04.dds");
	TextureReadyInBackground(L"EL_FIRE_07.dds");
	TextureReadyInBackground(L"Explosion_Fire01.dds");
	TextureReadyInBackground(L"Explosion_Light02.dds");
	TextureReadyInBackground(L"Flame_REVOLVER.dds");
	TextureReadyInBackground(L"Particle_Blur.DDS");
	TextureReadyInBackground(L"SA_EL_MEGA_SLASH_FIRE02.dds");
	TextureReadyInBackground(L"EL_FIRE_03.dds");
	TextureReadyInBackground(L"GroundShockWave02.dds");
	TextureReadyInBackground(L"Mesh_Elsword_Event_AC_Upbody21A_Effect.tga");
	TextureReadyInBackground(L"elsword_SA_rageCutter_smogPoint.dds");
	XSkinMeshReadyInBackground(L"Mega_Slash_Trace_Dummy.x");
	XSkinMeshReadyInBackground(L"DummyAttackBox_200x200x200.X");
}
void CX2GUEL::ESI_SI_SA_SPK_BURST_WAVE_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
// 소드 파이어
void CX2GUEL::ESI_SI_SA_SPK_SWORDFIRE_Init()			
{

}
void CX2GUEL::ESI_SI_SA_SPK_SWORDFIRE_EventProcess()
{	
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		if( true == IsOnSomethingFuture() )
		{
			StateChange( USI_JUMP_LANDING );
		}	
		else
		{
			// 현재 진행 방향 쪽으로 키보드를 누르면 앞으로 좀 더 나아감
			if( ( true == m_InputData.pureRight && true == m_FrameDataFuture.syncData.bIsRight ) ||
				( true == m_InputData.pureLeft && false == m_FrameDataFuture.syncData.bIsRight ) )
			{
				m_PhysicParam.nowSpeed.x = GetWalkSpeed();
			}
		}
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
// 인페르날 블레이드
void CX2GUEL::ESI_SI_SA_SPK_INFERNAL_BLADE_Init()
{
	TextureReadyInBackground(L"EL_FIRE_01.dds");
	TextureReadyInBackground(L"EL_FIRE_02.dds");
	TextureReadyInBackground(L"EL_FIRE_03.dds");
	TextureReadyInBackground(L"EL_FIRE_04.dds");
	TextureReadyInBackground(L"Explosion_Fire01.dds");
	TextureReadyInBackground(L"Explosion_Light01.dds");
	TextureReadyInBackground(L"SA_EL_MEGA_SLASH_FIRE02.dds");
	XSkinMeshReadyInBackground(L"SA_SPK_INFERNAL_BLADE_Weapon_Dummy.x");
	TextureReadyInBackground(L"Whitecircle03.dds");
}
void CX2GUEL::ESI_SI_SA_SPK_INFERNAL_BLADE_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )
	CommonEventProcess();
}
// 이터널 파이어
void CX2GUEL::ESI_SI_SA_SPK_ETERNAL_FIRE_Init()		
{
	TextureReadyInBackground(L"Arme_Ring2.dds");
	TextureReadyInBackground(L"EL_FIRE_03.dds");
	TextureReadyInBackground(L"EL_FIRE_10.dds");
	TextureReadyInBackground(L"Explosion_Fire01.dds");
	TextureReadyInBackground(L"GroundShockWave02.dds");
	TextureReadyInBackground(L"Particle_Blur.DDS");
	TextureReadyInBackground(L"Particle_Blur.dds");
	TextureReadyInBackground(L"SA_EL_MEGA_SLASH_FIRE02.dds");
	TextureReadyInBackground(L"state_potion_flare.DDS");
	TextureReadyInBackground(L"stone_AirShip.dds");
	XSkinMeshReadyInBackground(L"Elsword_SI_SA_Phoenix_Talon_Mesh02.X");
	XSkinMeshReadyInBackground(L"SA_SPK_ETERNAL_FIRE_Fire_Circle01.x");
	XSkinMeshReadyInBackground(L"SA_SPK_ETERNAL_FIRE_Fire_Circle02.x");
	XSkinMeshReadyInBackground(L"SA_SPK_ETERNAL_FIRE_Fire_Circle03.x");
}
void CX2GUEL::ESI_SI_SA_SPK_ETERNAL_FIRE_EventProcess()
{
	if( true == m_pXSkinAnimFuture->IsAnimationEnd() )
	{
		StateChange( USI_WAIT );
	}
	SKILL_CANCEL_AFTER( m_fSkillCancelAfter )
		WALK_CANCEL_AFTER( m_fWalkCancelAfter )
		BWALK_CANCEL_AFTER( m_fBWalkCancelAfter )
		DASH_CANCEL_AFTER( m_fDashCancelAfter )
		ZX_CANCEL_AFTER_EX( m_fAttackCancelAfterEx )

	CommonEventProcess();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 패시브 함수
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @function : ChivalryProc_FrameMove
	@brief : 패시브 스킬 - 기사의 숙명 처리 프레임 무브
			 - 해당 버프가 안 걸렸을 때만 체크하여 버프 걸기
*/

void CX2GUEL::ChivalryProc_FrameMove()
{
	// 기사의 숙명을 배우지 않았다면 처리하지 않기
	if( 0 == m_uiChivalryLevel )
		return;

	// 1초 간격으로 처리
	m_fChivalryBuffCheckTime += m_fElapsedTime;
	if( m_fChivalryBuffCheckTime < 1.f )
	{
		return;
	}

	m_fChivalryBuffCheckTime = 0.f;
	// 공격력 버프 적용 여부 체크
	{
		/// 김현철 // 2013-07-18
		// A 클라이언트가 이 함수의 호출자, B 클라이언트가 파티원인 경우
		// B 의 HP가 특정 % 이하로 내려가면 true를 리턴 해야 하나
		// A 클라이언트에서는 B가 파티원이지만
		// B 클라이언트에서는 B가 파티원이 아닌 자기 자신이기 때문에 따로 체크 해야 함
		// 또한 파티원이 아닌 유저의 클라이언트에서는 체크 자체가 안됨
		if( true == g_pX2Game->GetIsExsitancePartyMemberHPLessThanPer( _CONST_EL_::CHIVALRY_BUFF_CONDITION_HP_RATE, this ) )
		{
			// 공격력 버프 적용
			if( 0 < m_uiChivalryAttackBuffFactorID )
			SetBuffFactorToGameUnitByBuffFactorID( m_uiChivalryAttackBuffFactorID, m_uiChivalryLevel);
		}
	}

	// 방어력 버프 적용 여부 체크
	{
		if(  _CONST_EL_::CHIVALRY_BUFF_CONDITION_HP_RATE >= (GetNowHp() / GetMaxHp()) )
		{
			// 방어력 버프 적용
			if( 0 < m_uiChivalryDefenceBuffFactorID )
				SetBuffFactorToGameUnitByBuffFactorID( m_uiChivalryDefenceBuffFactorID, m_uiChivalryLevel );
		}
	}
}

#ifdef MODIFY_SET_DAMAGE_DATA
/*virtual*/void CX2GUEL::SetDamageExceptionProcess( OUT CX2DamageManager::DamageData* pDamageData_ ,OUT float& fPowerRate)
{
	CX2GUUser::SetDamageExceptionProcess(pDamageData_, fPowerRate);

#ifdef ELSWORD_WAY_OF_SWORD // 검의 길
	if( WSS_VIGOR == GetWayOfSwordState() )
	{
		switch( pDamageData_->m_WayofSwordType ) 
		{
		// 질풍 커맨드 다운수치 감소
		case CX2DamageManager::WST_VIGOR_NORMAL:
			{
				float fGaleForceDownDecreaseRate = _CONST_EL_::GALE_FORCE_DOWN_DECREASE_RATE;

				// 대전에서는 감소량 1/2
				if( NULL != g_pX2Game && CX2Game::GT_PVP == g_pX2Game->GetGameType())	
					fGaleForceDownDecreaseRate = _CONST_EL_::GALE_FORCE_DOWN_DECREASE_RATE_PVP;

				pDamageData_->fForceDownValue *= fGaleForceDownDecreaseRate;
			} break;
		default:
			break;
		}
	}
	else if( WSS_DESTRUCTION == GetWayOfSwordState() )
	{
		switch( pDamageData_->m_WayofSwordType ) 
		{
		// 섬멸 스킬, 커맨드 방어 무시 적용
		case CX2DamageManager::WST_DESTRUCTION_NORMAL:
		case CX2DamageManager::WST_DESTRUCTION_ACTIVE:
		case CX2DamageManager::WST_DESTRUCTION_SPECIAL:
			{
				pDamageData_->m_bIgnoreStateDefence = true;
			} break;
		default:
			break;
		}
	}
#endif // ELSWORD_WAY_OF_SWORD
}
#endif //MODIFY_SET_DAMAGE_DATA


#ifdef MODIFY_RIDING_PET_AWAKE
void CX2GUEL::RidingHyperModeFrameMove()
{
	CommonHyperModeFrameMove(0.f, 0.1f, true);
}
void CX2GUEL::CommonHyperModeFrameMove( float fTime1_, float fTime2_, bool bSound_ /*= false*/ )
{
	g_pX2Game->GetWorld()->SetWorldColor( 0xff222222 );

#ifdef  X2OPTIMIZE_NPC_ADAPTIVE_FRAME_MOVE
	if( m_pXSkinAnim->EventTimerOneshot( fTime1_ ) )
#else   X2OPTIMIZE_NPC_ADAPTIVE_FRAME_MOVE
	if( m_pXSkinAnim->EventTimer( fTime1_ ) == true && EventCheck(fTime1_, false) == true )
#endif  X2OPTIMIZE_NPC_ADAPTIVE_FRAME_MOVE
	{		
		ShowMinorParticleHyperModeChange();
	}

#ifdef  X2OPTIMIZE_NPC_ADAPTIVE_FRAME_MOVE
	if( m_pXSkinAnim->EventTimerOneshot( fTime2_ ) )
#else   X2OPTIMIZE_NPC_ADAPTIVE_FRAME_MOVE
	if( m_pXSkinAnim->EventTimer( fTime2_ ) == true && EventCheck(fTime2_, false) == true )
#endif  X2OPTIMIZE_NPC_ADAPTIVE_FRAME_MOVE
	{
		ShowMinorParticleHyperModeShock();

		UpDownCrashCamera( 30.0f, 0.4f );
		g_pKTDXApp->GetDGManager()->ClearScreen();

		ShowMinorParticleHyperModeTrace();
		ApplyHyperModeBuff();

		if( true == bSound_ )
		{
			PlaySound( L"Energy.ogg" );
			PlaySound( L"Break.ogg" );
			PlaySound( L"Change.ogg" );
		}
	}
	CommonFrameMove();
}
#endif //MODIFY_RIDING_PET_AWAKE

#endif // NEW_CHARACTER_EL

