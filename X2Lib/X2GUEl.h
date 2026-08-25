#pragma once


#ifdef NEW_CHARACTER_EL

class CX2GUEL : public CX2GUUser
{
public:
	enum EL_STATE_ID
	{
		//전 캐릭터 공통 스테이트
		ESI_BASE = USI_END + 1,
		ESI_STANDUP_ROLLING_FRONT_FRONT,
		ESI_STANDUP_ROLLING_FRONT_BACK,
		ESI_STANDUP_ROLLING_BACK_FRONT,
		ESI_STANDUP_ROLLING_BACK_BACK,
		ESI_STANDUP_ATTACK_FRONT,
		ESI_STANDUP_ATTACK_BACK,
		ESI_STANDUP_ATTACK_FRONT_NO_DOWN,
		ESI_STANDUP_ATTACK_BACK_NO_DOWN,
		ESI_DASH_JUMP_POWER_LANDING,
		ESI_WALL_LANDING,

		//EL 스테이트
		///////////////////////////
		ESI_COMBO_Z,			/// 콤보 Z
		ESI_COMBO_ZZ,			/// 콤보 ZZ
		ESI_COMBO_ZZZ,			/// 콤보 ZZZ
		ESI_COMBO_ZZZZ,			/// 콤보 ZZZZ
		ESI_COMBO_ZZZX,			/// 콤보 ZZZX

		ESI_COMBO_X,			/// 콤보 X
		ESI_COMBO_XX,			/// 콤보 XX
		ESI_COMBO_XXX,			/// 콤보 XXX
		ESI_COMBO_XXZ,			/// 콤보 XXZ

		ESI_JUMP_ATTACK_Z,		/// 점프 공격 Z
		ESI_JUMP_ATTACK_X,		/// 점프 공격 X

		ESI_DASH_COMBO_Z,		/// 대쉬 콤보 Z
		ESI_DASH_COMBO_ZZ,		/// 대쉬 콤보 ZZ

		ESI_DASH_COMBO_X,		/// 대쉬 콤보 X

		ESI_DASH_JUMP_COMBO_Z,	/// 대쉬 점프 콤보 Z
		ESI_DASH_JUMP_COMBO_ZZ,	/// 대쉬 점프 콤보 ZZ
		ESI_DASH_JUMP_COMBO_ZZZ,/// 대쉬 점프 콤보 ZZZ

		ESI_DASH_JUMP_COMBO_X,	/// 대쉬 점프 콤보 X
		ESI_DASH_JUMP_COMBO_XX, /// 대쉬 점프 콤보 XX
		ESI_DASH_JUMP_COMBO_XX_LANDING, /// 대쉬 점프 콤보 XX Landing

		ESI_QUICK_STANDUP_ATTACK_FRONT,	// 기회의 포착 STAND_FRONT
		ESI_QUICK_STANDUP_ATTACK_BACK,	// 기회의 포착 STAND_BACK 

		///////////////////////////			
		
		ESI_SWORD_WAIT,			/// 검의 길 대기

/////////////////////////////////////////////////////////////////////////////////////////////////
// 나이트
		ESI_SI_A_EL_DODGE_AND_SLASH,			/// 닷지&슬래시
		ESI_SI_A_EL_DODGE_AND_SLASH_SECOND,		/// 닷지&슬래시 2회 입력
		ESI_SI_A_EL_DODGE_AND_SLASH_ATTACK_Z,	/// 닷지&슬래시 Z공격

		ESI_SI_A_EL_LEAP_CRASH,					// 도약
		ESI_SI_A_EL_LEAP_CRASH_JUMP,
		ESI_SI_A_EL_LEAP_CRASH_JUMP_LANDING,

		ESI_SI_A_EL_RUSHING_SWORD,				/// 러싱 소드
		ESI_SI_A_EL_RUSHING_SWORD_2nd,
		ESI_SI_A_EL_RUSHING_SWORD_3rd,

		ESI_SI_A_EL_KICK,						/// 발차기

		ESI_SI_A_EL_MEGA_SLASH,					/// 메가 슬래시

		ESI_SI_A_EL_SONIC_ASSAULT_STAB,			/// 소닉 어썰트 - 스탭
		ESI_SI_A_EL_SONIC_ASSAULT_STING,		/// 소닉 어썰트 - 스팅

		ESI_SI_SA_EL_WILD_SHOCK,				/// 와일드 쇼크

		ESI_SI_SA_EL_POWER_BURSTER,				/// 파워 버스터

		ESI_SI_SA_EL_MEGA_BURSTER,				/// 메가 버스터

		ESI_SI_SA_EL_UNLIMITED_BLADE,			/// 언리미티드 블레이드
/////////////////////////////////////////////////////////////////////////////////////////////////
// 1차 공용
		ESI_SA_A_EL_ENDURANCE_POWER_ACTIVE,		/// 극기 - 강(强)

/////////////////////////////////////////////////////////////////////////////////////////////////
// 세이버 나이트

		ESI_DASH_COMBO_SSK_ZZ,					/// 대시 콤보 ZZ
		ESI_DASH_COMBO_SSK_ZZZ,					/// 대시 콤보 ZZZ
		ESI_DASH_COMBO_SSK_ZZX,					/// 대시 콤보 ZZX
		ESI_DASH_JUMP_COMBO_SSK_XX,				/// 대시 점프 콤보 XX
		ESI_DASH_JUMP_COMBO_SSK_XXX,			/// 대시 점프 콤보 XXX

		ESI_SI_A_SSK_CROSS_SLASH,				/// 크로스 슬래시

		ESI_SI_A_SSK_POWER_BREAK,				/// 파워 브레이크
		ESI_SI_A_SSK_HEAVY_STUNNER,				/// 헤비 스터너

		ESI_SI_SA_SSK_SPIRAL_BLAST,				/// 스파이럴 블래스트
		
		ESI_SI_SA_SSK_EXTINCTION_START,			/// 세이버 - 익스팅션
		ESI_SI_SA_SSK_EXTINCTION_CHARGE,		
		ESI_SI_SA_SSK_EXTINCTION_ATTACK,
		ESI_SI_SA_SSK_EXTINCTION_DEGREE_UP,
		ESI_SI_SA_SSK_EXTINCTION_DEGREE_DOWN,

		ESI_SI_SA_SSK_VICTORIOUS_SWORD,			/// 승리의 검

		ESI_SI_SA_SSK_JUDGEMENT_DRIVE,			/// 저지먼트 드라이브

/////////////////////////////////////////////////////////////////////////////////////////////////
// 파이로 나이트

		ESI_DASH_COMBO_SPK_ZX,					/// 대시 콤보 ZX
		ESI_DASH_COMBO_SPK_ZZ,					/// 대시 콤보 ZZZ
		ESI_DASH_COMBO_SPK_ZZZ,					/// 대시 콤보 ZZZ
		ESI_DASH_JUMP_COMBO_SPK_XZ,				/// 대시 점프 콤보 XZ
		ESI_DASH_JUMP_COMBO_SPK_XZ_REATTACK,	/// 대시 콤보 ZX 추가 공격

		ESI_SI_A_SPK_SWORD_ERUPTION,			/// 소드 이럽션
		
		ESI_SI_A_SPK_SCALDIC_SWORD,				/// 스칼딕 소드

		ESI_SI_A_SPK_BIG_BURST,					/// 대폭살

		ESI_SI_SA_SPK_BURST_RISING,				/// 버스트 라이징
		
		ESI_SI_SA_SPK_BURST_WAVE,				/// 버스트 웨이브

		ESI_SI_SA_SPK_SWORDFIRE,				/// 소드 파이어

		ESI_SI_SA_SPK_INFERNAL_BLADE,			/// 인페르날 블레이드

		ESI_SI_SA_SPK_ETERNAL_FIRE,				/// 이터널 파이어
	};

private:
	CX2GUEL( int unitIndex, int teamNum, lua_State*   pScriptLuaState, int frameBufferNum, CX2Unit* pUnit );
	virtual ~CX2GUEL(void);

protected:

	virtual void Init( bool bUseTeamPos_, int iStartPosIndex_ );
	virtual void ParseCommonRandomState();
	virtual void InitState();
	virtual void InitComponent();
	virtual void InitPassiveSkillState();
	virtual void SetEquippedSkillFuncBySkillSlotIndex( const CX2SkillTree::SKILL_ID eSkillID_, const int iSlotIndex_, const bool bSlotB_ );
	virtual void SetEquippedSkillLevel( const CX2SkillTree::SKILL_ID eSkillID_, const bool bChangeAll_ );
				 
	virtual void CommonStateStartFuture();
	virtual void CommonStateStart();
				 
	virtual void CommonFrameMoveFuture();
	virtual void CommonFrameMove();
				 
	virtual void CommonStateEndFuture();
	virtual void CommonStateEnd();

	virtual void DetonationStateChange( CX2DamageManager::DamageData* pDamageData );

	virtual RENDER_HINT	CommonRender_Prepare();

	virtual bool SpecialAttackEventProcess( CX2SkillTree::ACTIVE_SKILL_USE_CONDITION eActiveSkillUseCondition_ = CX2SkillTree::ASUT_GROUND );
	virtual void NoStateChangeActionFrameMove();
	virtual float GetActualMPConsume( const CX2SkillTree::SKILL_ID eSkillID_, const int iSkillLevel_ ) const;

	float GetActualCoolTime( IN const CX2SkillTree::SkillTemplet* pSkillTemplet_, IN int iSkillLevel ) const;

	virtual CX2GageData* CreateGageData();

	virtual	bool ShouldResetNotBusyTimer();

	void InitStateID();
	void InitStateCommon();
	void InitStateByUnitClass();

	void SetEquippedSkillLevelStateData( const CX2SkillTree::SkillTemplet* pSkillTemplet_, EL_STATE_ID eStateID_ );
	void SetSkillLevelStateData( const CX2SkillTree::SkillTemplet* pSkillTemplet_, UserUnitStateData& stateData_ );


	void ShowActiveSkillCutInAndLight( float fTimeToShow_, bool bOnlyLight_ = false );
	virtual void ShowActiveSkillCutInAndLightByScript( float fTimeToShow_, bool bOnlyLight_ );

	bool CanUseSpecialAttack( CX2SkillTree::ACTIVE_SKILL_USE_CONDITION eActiveSkillUseCondition = CX2SkillTree::ASUT_GROUND );
	void StateChangeSpecialAttack( IN const CX2SkillTree::SkillTemplet* pSkillTemplet_ );
#ifdef FIX_NO_STATE_SKILL_BUG
	virtual bool IsValideSkillRidingOn( CX2SkillTree::SKILL_ID eSkill_ID_ );
#endif // FIX_NO_STATE_SKILL_BUG

	CX2ElesisGageData* GetELGageData() { return static_cast<CX2ElesisGageData*>( GetGageData() ); }
	const CX2ElesisGageData* GetELGageData() const { return static_cast<const CX2ElesisGageData*>( GetGageData() ); }
////////////////////////////////////////////////
//검의 길
#ifdef ELSWORD_WAY_OF_SWORD
	void SwordWaitStartFuture();
	void SwordWaitStart();
	void SwordWaitEventProcess();
	void PlayAnimationSwordWaitStart( CKTDGXSkinAnim* pXSkinAnim_, const FrameData& refFrameData_ );	

	void WayOfSwordStateChange();
	void GaleBuffFrameMove();

	bool CanSpecialReactProcess( CX2DamageManager::REACT_TYPE eReActType_ );
#endif // ELSWORD_WAY_OF_SWORD

	float GetFireBallPowerRate() const { return m_fFireBallPowerRate; }
	void SetFireBallPowerRate(float val) { m_fFireBallPowerRate = val; }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 공통 스테이트 함수
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//ESI_DIE_FRONT
	void DieFrontStartFuture();
	void DieFrontStart();
	void DieFrontFrameMove();
	void DieFrontEventProcess();

	//ESI_DIE_BACK
	void DieBackStartFuture();
	void DieBackStart();
	void DieBackFrameMove();
	void DieBackEventProcess();

	//ESI_WAIT
	void WaitStartFuture();
	void WaitStart();
	void WaitEventProcess();

	//ESI_WALK
	void WalkEventProcess();

	//ESI_JUMP_READY
	void JumpReadyEventProcess();

	//ESI_JUMP_UP
	void JumpUpEventProcess();

	//ESI_JUMP_DOWN
	void JumpDownEventProcess();

	//ESI_JUMP_LANDING
	void JumpLandingStartFuture();
	void JumpLandingEventProcess();

	//ESI_DASH
	void DashEventProcess();

	//ESI_DASH_END
	void DashEndStartFuture();
	void DashEndFrameMove();
	void DashEndEventProcess();

	//ESI_DASH_JUMP
	void DashJumpStartFuture();
	void DashJumpStart();
	void DashJumpFrameMoveFuture();
	void DashJumpEventProcess();
	void DashJumpEndFuture();

	//ESI_DASH_JUMP_LANDING
	void DashJumpLandingEventProcess();
	void DashJumpLandingStartFuture();

	//ESI_DAMAGE_SMALL_FRONT
	//USI_DAMAGE_SMALL_BACK
	void DamageSmallEventProcess();

	//ESI_DAMAGE_BIG_FRONT
	//ESI_DAMAGE_BIG_BACK
	void DamageBigStart();
	void DamageBigEventProcess();

	//ESI_DAMAGE_DOWN_FRONT
	void DamageDownFrontFrameMove();
	void DamageDownFrontEventProcess();

	//ESI_DAMAGE_DOWN_BACK
	void DamageDownBackFrameMove();
	void DamageDownBackEventProcess();

	//ESI_DAMAGE_AIR_DOWN_LANDING
	void DamageAirDownLandingEventProcess();

	//ESSI_STANDUP_ROLLING_FRONT_FRONT
	void StandUpRollingFrontFrontEventProcess();
	void StandUpRollingFrontFrontEnd();

	//ESSI_STANDUP_ROLLING_FRONT_BACK
	void StandUpRollingFrontBackEventProcess();
	void StandUpRollingFrontBackEnd();

	//ESSI_STANDUP_ROLLING_BACK_FRONT
	void StandUpRollingBackFrontEventProcess();
	void StandUpRollingBackFrontEnd();

	//ESSI_STANDUP_ROLLING_BACK_BACK
	void StandUpRollingBackBackEventProcess();
	void StandUpRollingBackBackEnd();

	//ESI_STANDUP_ATTACK_FRONT
	void StandUpAttackFrontEventProcess();
	void StandUpAttackFrontEnd();

	//ESI_STANDUP_ATTACK_BACK
	void StandUpAttackBackEventProcess();
	void StandUpAttackBackEnd();
	void StandUpAttackBackStateEndFuture();

	//ESI_STANDUP_ATTACK_FRONT_NO_DOWN
	void StandUpAttackFrontNoDownEventProcess();
	void StandUpAttackFrontNoDownEnd();

	//ESI_STANDUP_ATTACK_BACK_NO_DOWN
	void StandUpAttackBackNoDownEventProcess();
	void StandUpAttackBackNoDownEnd();

	//ESI_DASH_JUMP_POWER_LANDING
	void DashJumpPowerLandingStartFuture();
	void DashJumpPowerLandingStart();
	void DashJumpPowerLandingEventProcess();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 공통 스테이트 함수
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void ComboZEventProcess();
	void ComboZZEventProcess();

	void ComboZZZEventProcess();
	void ComboZZZZEventProcess();
	void ComboZZZXEventProcess();

	void ComboXEventProcess();
	void ComboXXEventProcess();
	void ComboXXXEventProcess();
	void ComboXXZEventProcess();

	void JumpAttackZEventProcess();
	void JumpAttackZFrameMoveFuture();

	void JumpAttackXEventProcess();
	void JumpAttackXFrameMoveFuture();

	void DashComboZEventProcess();
	void DashComboZZEventProcess();

	void DashComboXEventProcess();

	void DashJumpComboZEventProcess();
	void DashJumpComboZZEventProcess();
	void DashJumpComboZZZEventProcess();

	void DashJumpComboXEventProcess();
	void DashJumpComboXXEventProcess();
	void DashJumpComboXXLandingEventProcess();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 나이트 스테이트 함수
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ESI_SI_A_EL_DODGE_AND_SLASH_StartFuture();			// 닷지 & 슬래시
	void ESI_SI_A_EL_DODGE_AND_SLASH_Init();
	void ESI_SI_A_EL_DODGE_AND_SLASH_FrameMove();
	void ESI_SI_A_EL_DODGE_AND_SLASH_EventProcess();
	void ESI_SI_A_EL_DODGE_AND_SLASH_SECOND_StartFuture();
	void ESI_SI_A_EL_DODGE_AND_SLASH_ATTACK_Z_EventProcess();

	void ESI_SI_A_EL_LEAP_CRASH_Init();						// 도약
	void ESI_SI_A_EL_LEAP_CRASH_EventProcess();
	void ESI_SI_A_EL_LEAP_CRASH_JUMP_EventProcess();
	void ESI_SI_A_EL_LEAP_CRASH_JUMP_LANDING_EventProcess();

	void ESI_SI_A_EL_RUSHING_SWORD_StartFuture();			// 러싱 소드
	void ESI_SI_A_EL_RUSHING_SWORD_Init();
	void ESI_SI_A_EL_RUSHING_SWORD_EventProcess();
	void ESI_SI_A_EL_RUSHING_SWORD_2nd_EventProcess();
	void ESI_SI_A_EL_RUSHING_SWORD_3rd_EventProcess();

	void ESI_SI_A_EL_KICK_Init();							// 발차기
	void ESI_SI_A_EL_KICK_Start();
	void ESI_SI_A_EL_KICK_EventProcess();
	
	void ESI_SI_A_EL_MEGA_SLASH_Init();						// 메가 슬래시
	void ESI_SI_A_EL_MEGA_SLASH_EventProcess();

	void ESI_SI_A_EL_SONIC_ASSAULT_STAB_Init();				// 소닉 어썰트 - 스탭
	void ESI_SI_A_EL_SONIC_ASSAULT_STAB_EventProcess();

	void ESI_SI_A_EL_SONIC_ASSAULT_STING_Init();			// 소닉 어썰트 - 스팅
	void ESI_SI_A_EL_SONIC_ASSAULT_STING_EventProcess();

	void ESI_SI_SA_EL_WILD_SHOCK_Init();					// 와일드 쇼크
	void ESI_SI_SA_EL_WILD_SHOCK_EventProcess();

	void ESI_SI_SA_EL_POWER_BURSTER_Init();					// 파워 버스터	
	void ESI_SI_SA_EL_POWER_BURSTER_EventProcess();

	void ESI_SI_SA_EL_MEGA_BURSTER_Init();					// 메가 버스터
	void ESI_SI_SA_EL_MEGA_BURSTER_EventProcess();

	void ESI_SI_SA_EL_UNLIMITED_BLADE_Init();				// 언리미티드 블레이드
	void ESI_SI_SA_EL_UNLIMITED_BLADE_EventProcess();

	void ESI_SI_A_EL_ENDURANCE_POWER_ACTIVE_EventProcess();	// 극기 - 강(强)
	void ESI_SI_A_EL_ENDURANCE_POWER_ACTIVE_Init();
	void ESI_SI_A_EL_ENDURANCE_POWER_ACTIVE_StateStart();
	void DoStateEndurancePowerRage();
	void ProcessEndurancePowerSkill();
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 세이버 나이트 스테이트 함수
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void ESI_DASH_COMBO_SSK_ZZ_EventProcess();				/// DASH_COMBO_ZZ
	void ESI_DASH_COMBO_SSK_ZZZ_EventProcess();				/// DASH_COMBO_ZZZ
	void ESI_DASH_COMBO_SSK_ZZX_EventProcess();				/// DASH_COMBO_ZZX
	void ESI_DASH_JUMP_COMBO_SSK_XX_EventProcess();			/// DASH_JUMP_COMBO_XX
	void ESI_DASH_JUMP_COMBO_SSK_XXX_EventProcess();		/// DASH_JUMP_COMBO_XXX

	void ESI_SI_A_SSK_CROSS_SLASH_Init();					// 크로스 슬래시
	void ESI_SI_A_SSK_CROSS_SLASH_EventProcess();

	void ESI_SI_A_SSK_POWER_BREAK_Init();					// 파워 브레이크
	void ESI_SI_A_SSK_POWER_BREAK_EventProcess();

	void ESI_SI_A_SSK_HEAVY_STUNNER_Init();					// 헤비 스터너
	void ESI_SI_A_SSK_HEAVY_STUNNER_EventProcess();

	void ESI_SI_SA_SSK_SPIRAL_BLAST_Init();					// 스파이럴 블래스트
	void ESI_SI_SA_SSK_SPIRAL_BLAST_EventProcess();

	void ESI_SI_SA_SSK_EXTINCTION_START_Init();				// 세이버 - 익스팅션
	void ESI_SI_SA_SSK_EXTINCTION_START_Start();
	void ESI_SI_SA_SSK_EXTINCTION_START_StartFuture();
	void ESI_SI_SA_SSK_EXTINCTION_START_EventProcess();
	void ESI_SI_SA_SSK_EXTINCTION_CHARGE_FrameMove();
	void ESI_SI_SA_SSK_EXTINCTION_CHARGE_FrameMoveFuture();
	void ESI_SI_SA_SSK_EXTINCTION_CHARGE_EventProcess();
	void ESI_SI_SA_SSK_EXTINCTION_ATTACK_StartFuture();		// 발사 스테이트
	void ESI_SI_SA_SSK_EXTINCTION_ATTACK_FrameMove();		
	void ESI_SI_SA_SSK_EXTINCTION_ATTACK_EventProcess();
	void ESI_SI_SA_SSK_EXTINCTION_DEGREE_UP_FrameMove();	// 각도 조절
	void ESI_SI_SA_SSK_EXTINCTION_DEGREE_UP_EventProcess();
	void ESI_SI_SA_SSK_EXTINCTION_DEGREE_UP_FrameMoveFuture();
	void ESI_SI_SA_SSK_EXTINCTION_DEGREE_DOWN_FrameMove();	
	void ESI_SI_SA_SSK_EXTINCTION_DEGREE_DOWN_EventProcess();
	void ESI_SI_SA_SSK_EXTINCTION_DEGREE_DOWN_FrameMoveFuture();
	void InitExtinction();									// 익스팅션에 사용되는 변수 초기화
	void Extinction_FrameMoveFuture();						// 익스팅션 공통 프레임무브
	void CreateExctionUIEffect();
	void DestoryExctionUIEffect();

	void ESI_SI_SA_SSK_VICTORIOUS_SWORD_Init();				// 승리의 검
	void ESI_SI_SA_SSK_VICTORIOUS_SWORD_EventProcess();

	void ESI_SI_SA_SSK_JUDGEMENT_DRIVE_Init();				// 저지먼트 드라이브
	void ESI_SI_SA_SSK_JUDGEMENT_DRIVE_EventProcess();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 파이로 나이트 스테이트 함수
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void ESI_DASH_COMBO_SPK_ZX_EventProcess();				// DASH_COMBO_ZX
	void ESI_DASH_COMBO_SPK_ZX_FrameMove();
	void ESI_DASH_COMBO_SPK_ZZ_EventProcess();				// DASH_COMBO_ZZ
	void ESI_DASH_COMBO_SPK_ZZZ_EventProcess();				// DASH_COMBO_ZZZ
	void ESI_DASH_COMBO_SPK_ZZZ_FrameMove();
	void ESI_DASH_JUMP_COMBO_SPK_XZ_EventProcess();			// DASH_JUMP_COMBO_XZ
	void ESI_DASH_JUMP_COMBO_SPK_XZ_FrameMove();
	void ESI_DASH_JUMP_COMBO_SPK_XZ_StartFurture();
	void ESI_DASH_JUMP_COMBO_SPK_XZ_REATTACK_StartFurture();

	void ESI_SI_A_SPK_SWORD_ERUPTION_Init();				// 소드 이럽션
	void ESI_SI_A_SPK_SWORD_ERUPTION_EventProcess();
	
	void ESI_SI_A_SPK_BIG_BURST_Init();						// 대폭살
	void ESI_SI_A_SPK_BIG_BURST_EventProcess(); 

	void ESI_SI_SA_SPK_BURST_RISING_Init();					// 버스트 라이징
	void ESI_SI_SA_SPK_BURST_RISING_EventProcess();

	void ESI_SI_SA_SPK_BURST_WAVE_Init();					// 버스트 웨이브
	void ESI_SI_SA_SPK_BURST_WAVE_EventProcess();

	void ESI_SI_SA_SPK_SWORDFIRE_Init();					// 소드 파이어
	void ESI_SI_SA_SPK_SWORDFIRE_EventProcess();

	void ESI_SI_SA_SPK_INFERNAL_BLADE_Init();				// 인페르날 블레이드
	void ESI_SI_SA_SPK_INFERNAL_BLADE_EventProcess();

	void ESI_SI_SA_SPK_ETERNAL_FIRE_Init();					// 이터널 파이어
	void ESI_SI_SA_SPK_ETERNAL_FIRE_EventProcess();
	//~ protected 함수
public:
	static CX2GUEL* CreateGUEL( int unitIndex, int teamNum, lua_State*   pScriptLuaState, int frameBufferNum, CX2Unit* pUnit );

	static void			AppendDeviceToDeviceList( CKTDXDeviceDataList& listInOut_ );
	static void			AppendComponentToDeviceList( CKTDXDeviceDataList& listInOut_ );
	static void			AppendEffectToDeviceList( CKTDXDeviceDataList& listInOut_, CX2DamageManager::EXTRA_DAMAGE_TYPE eDamageType_ );

	// 탈 것
	virtual bool CanRidingState();

	virtual void ReInit( bool bTeamPos = false, int iStartPosIndex = -1 );
	virtual void DamageReact( OUT CX2DamageManager::DamageData* pDamageData_ );

	virtual void Win();
	virtual void Lose();
	virtual void Draw();

	virtual void InitEffect();

	virtual void CreateMajorParticleArray();
	virtual void CreateMinorParticleArray();

	static void AppendMajorParticleToDeviceList( CKTDXDeviceDataList& listInOut_ );
	static void AppendMinorParticleToDeviceList( CKTDXDeviceDataList& listInOut_ );

	virtual bool SpecialAttackNoStageChange( IN const CX2SkillTree::SkillTemplet* pSkillTemplet_ );
	virtual void COMMON_BUFF_FrameMove();

	virtual void HyperModeFrameMove();
#ifdef MODIFY_RIDING_PET_AWAKE
	void RidingHyperModeFrameMove();
	void CommonHyperModeFrameMove( float fTime1_, float fTime2_, bool bSound_ = false );
#endif //MODIFY_RIDING_PET_AWAKE
	virtual void HyperModeBuffEffectStart();

	virtual void SetSpecificValueByBuffTempletID( const BUFF_TEMPLET_ID eBuffTempletId_ );
	virtual void UnSetSpecificValueByBuffTempletID( const BUFF_TEMPLET_ID eBuffTempletId_ );

	virtual void InitEnchantWeaponEffectUnitSpecific();

	virtual int GetComboZStateID() const { return ESI_COMBO_Z; }
	virtual int GetComboXStateID() const { return ESI_COMBO_X; }
	////////////////////////////////////////////////
	//검의 길
#ifdef ELSWORD_WAY_OF_SWORD
	virtual int			GetWayOfSwordState() const;
	void SetWayOfSwordState( const int iWayOfSwordState_ );

	virtual float		GetWayOfSwordPoint() const;
	virtual void		SetWayOfSwordPoint( float fWayOfSwordPoint_ );

	void				Start_SwordmanVigor();
	void				End_SwordmanVigor();
	void				Start_SwordmanDestruction();
	void				End_SwordmanDestruction();

	virtual float GetFinalDamage_WayOfSword( CX2DamageManager::WAY_OF_SWORD_TYPE eWayOfSwordType_ );
	virtual void SetCriticalPercent_WayOfSword( IN CX2DamageManager::WAY_OF_SWORD_TYPE eWayOfSwordType_,
		OUT float& fCriticalPercent_);
#endif // ELSWORD_WAY_OF_SWORD

	////////////////////////////////////////////////////
	/// 패시브 관련 함수
	// 기사의 숙명 패시브 처리 프레임 무브
	void ChivalryProc_FrameMove( );

#ifdef MODIFY_SET_DAMAGE_DATA
	virtual	void SetDamageExceptionProcess( OUT CX2DamageManager::DamageData* pDamageData_ ,OUT float& fPowerRate);
#endif //MODIFY_SET_DAMAGE_DATA

	//~ Public 함수
protected:
	// 스킬 사용 관련
	const CX2SkillTree::SkillTemplet*	m_pSkillTemplet;	/// 스킬 템플릿 포인터
	int		m_iPressedSkillSlotIndex;
	float	m_fMPConsume;

#ifdef ELSWORD_WAY_OF_SWORD
	int		m_iPreComboCount;
#endif // ELSWORD_WAY_OF_SWORD

	// 닷지&슬래시 재사용 횟수
	UINT m_uiDodgeAndSlashMaxReuseCount;	// 최대 재사용 횟수
	UINT m_uiDodgeAndSlashReuseCount;		// 현재 남은 재사용 횟수

	/////////////////////////////////////////////////////
	/// 패시브 스킬 관련

	// 불의 신체
	float m_fFireBallPowerRate; /// 파이어볼 강화 배율. 기본값 1.f

	// 기사의 숙명
	float m_fChivalryBuffCheckTime;		// 기사의 숙명 버프 적용 체크 타임

	UINT m_uiChivalryAttackBuffFactorID;  // 공격력 버프 팩터 ID
	UINT m_uiChivalryDefenceBuffFactorID; // 방어력 버프 팩터 ID
	UINT m_uiChivalryLevel;				  // 기사의 숙명 스킬 레벨

	// 극기 - 강(强)
	bool m_bIsActiveEndurancePower;		/// 활성화 여부

	/////////////////////////////////////////////////////
	/// 액티브, 스페셜 액티브 스킬 관련


	// 세이버 - 익스팅션
	float m_fExtinctionRemainTime;		/// 익스팅션 유지 시간 체크
	float m_fExtinctionDegreeZ;			/// 익스팅션 발사 각도

	CKTDGParticleSystem::CParticleEventSequenceHandle	m_hExctionBG;
	CKTDGParticleSystem::CParticleEventSequenceHandle	m_hExtinctionArrow;
	CKTDGParticleSystem::CParticle*						m_pExtinctionArrow;

	// 승리의 검
	bool m_bIsApplyVictoriousSword;		/// 승리의 검 적용 여부. 닷지&슬래시 관련 효과 적용하기 위한 변수
	float m_fDodgeAndSlashCoolTimeDecreaseRate; /// 닷지&슬래시 쿨타임 감소 비율

	// ESI_DASH_JUMP_COMBO_SPK_XZ
	bool m_bIsDashAttackPossible;		// 대시 공격 불가 상태 // 대점ZX 연속 사용 하는 것을 막기 위한 코드
	UINT m_uiSPKXZReattakCount;
	//~ protected 변수
public:

	//~ Public 변수
};

#endif // NEW_CHARACTER_EL