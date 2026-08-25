#pragma once

#define D3DFVF_DRAWFACE_RHW_VERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#define ONE_CHARGE 100.0f
#define TWO_CHARGE 200.0f
#define THREE_CHARGE 300.0f
#define FULL_HYPER_COUNT 3	/// 3������ �� �� �ִ� ���� ���� ����


class CX2GameUnit;
class CX2RidingPet;
class CX2GageUI;

typedef boost::shared_ptr<CX2GageUI> CX2GageUIPtr;

class CX2GageUI 
{

public:

	// ���� �ϳ��� set(1) �� ���¸� UI�� ������ �ʵ��� ��
// 	enum GAGE_UI_FACTOR_SHOW	/// UI�� ���̰ų� �Ⱥ��̰� �ϴ� ���
// 	{
// 		GUFS_ENABLE_SCREEN_SHOT_MODE = 0,	/// ���� ��忡 ���� ������
// 		GUFS_STATE_CHANGE,					/// ����->�ʵ� �� ���º��濡 ���� �������� ���
// 		GUFS_FAR_DISTANCE,					/// �Ÿ��� �־ �Ⱥ��̴� ���
// 		GUFS_EVE_FLASH_BANG,				/// �̺� �Ϸ�Ʈ�� ��ų (���� �÷���)
// 		GUFS_END,							/// ��ü ���� �� ���� ��Ÿ��
// 	};

	enum PVP_GAME_UI_BAR
	{
		PGUB_MY_MP = 0,
		PGUB_MY_ACTIVE_MP,	/// Detonation(����)
		PGUB_MY_SOUL,		/// HyperGage(����)
	};

	enum PVP_GAME_UI_WAKE_ORB
	{
		PGUWO_ORB1 = 0,
		PGUWO_ORB2,
		PGUWO_ORB3,
		PGUWO_ORB_CHUNG,
		PGUWO_ORB_RAVEN,
		PGUWO_ORB_ARA,
	};

	enum WAY_OF_SWORD_DESTRUCTION_PICTURE
	{
		WOSDP_BG = 0,			// ���
		WOSDP_POINT_BAR,		// ���Ǳ� ��ġ
		WOSDP_DETONATION_CHARGE,// ���� ��ġ
		WOSDP_GAGE_EFFECT,		// ������ ����Ʈ
		WOSDP_EMPTY_BEAD,		// �� ����
		WOSDP_FULL_BEAD,		// �� �� ����
	};



	
#ifdef BUFF_ICON_UI
	struct BuffIcon
	{
		BUFF_TEMPLET_ID					eBuffID;
		wstring							szTextureFileName;
		wstring							szTextureKeyName;
		int								iBuffName;
		int								iBuffDesc;
		bool							bNew;
#ifdef SERV_IRUHADEV_BUFF_DURATION_TEXT
		float							fRemainTime;	/// Remaining seconds shown over the icon; -1.f = no timer
#endif //SERV_IRUHADEV_BUFF_DURATION_TEXT

#ifdef SERV_IRUHADEV_BUFF_DURATION_TEXT
		BuffIcon( BUFF_TEMPLET_ID eBuffID_, const wstring& FileName_, const wstring& KeyName_, const int BuffName_, const int BuffDesc_, const float fRemainTime_ = -1.f ):
		eBuffID(eBuffID_), szTextureFileName(FileName_), szTextureKeyName(KeyName_), iBuffName(BuffName_), iBuffDesc(BuffDesc_), bNew(true), fRemainTime(fRemainTime_)
		{}
#else
		BuffIcon( BUFF_TEMPLET_ID eBuffID_, const wstring& FileName_, const wstring& KeyName_, const int BuffName_, const int BuffDesc_ ):
		eBuffID(eBuffID_), szTextureFileName(FileName_), szTextureKeyName(KeyName_), iBuffName(BuffName_), iBuffDesc(BuffDesc_), bNew(true)
		{}
#endif //SERV_IRUHADEV_BUFF_DURATION_TEXT
	};
#endif //BUFF_ICON_UI

public:
	CX2GageUI() : m_fAlpha( 0.0f ), m_bShow( true )
	{}
	
	virtual ~CX2GageUI();

	virtual void			InitUI(){};	
	virtual void			OnFrameMove() = NULL;
	virtual	void			OnFrameRender() {};

	virtual void			CrashMyStateGageUI( const D3DXVECTOR2& _min, const D3DXVECTOR2& _max, D3DXCOLOR changeColor ) { ASSERT( !L"Crash Base Function is not available" ); }
	virtual void			SetHpTexture( const WCHAR* pTexName_ ) { ASSERT( !L"Hp Base function is not available"); }
	virtual void			SetMpTexture( const WCHAR* pTexName_ ) { ASSERT( !L"Mp Base function is not available"); }

	virtual bool			GetShow() const { return m_bShow; }
	virtual void			SetShow( const bool bShow_ ) { m_bShow = bShow_; }

	virtual float			GetAlpha() const { return m_fAlpha; }
	virtual void			SetAlpha( const float fAlpha_ ) { m_fAlpha = fAlpha_; }

	//void					SetOwnerGameUnit( CX2GameUnit* pOwnerGameUnit_ ) { m_optrGameUnit = pOwnerGameUnit_; }

#ifdef BUFF_ICON_UI
	virtual	void			PushBuff( const BuffIcon& BuffIconInfo_, bool bIsDeBuff_){};
	virtual	void			EraseBuff(BUFF_TEMPLET_ID eBuffID_, bool bIsDeBuff_){};
	virtual	void			ClearBuffList(){};
	virtual	void			UpdateBuffIcon(){};
	virtual void			NotifyDurationTime5sec(BUFF_TEMPLET_ID eBuffID_){};
	virtual void			InitBuffIconFlicker(BUFF_TEMPLET_ID eBuffID_) {};
#endif //BUFF_ICON_UI	

private:

	float					m_fAlpha;			/// HP�� MP�� �������� ���(�������, �� �������� ��)		
	bool					m_bShow;

	//std::bitset<GUFS_END>		m_bitsetGageUiFactorShow;	/// �Ѱ��� set(1)�� ��Ʈ�� ������ UI�� ������ �ʵ��� ��(ó���� ��� 0���� ����)

};

#define KIND_OF_HP_GAGE_BAR_COLOR 7
#define MAX_NUM_OF_POST_FIX_COLOR 5

class CX2BossGageUI;
typedef boost::shared_ptr<CX2BossGageUI> CX2BossGageUIPtr;

class CX2BossGageUI : public CX2GageUI
{
public:
	CX2BossGageUI( CX2GameUnit* pGameUnit_ );
	virtual ~CX2BossGageUI();

	virtual void			InitUI();
	void					SetBossGageTexture( const WCHAR* wszFaceTexName_, const WCHAR* wszFaceTexPieceName_ );
	void					ResetBossUIPos();
	virtual void			OnFrameMove();
	
	virtual void			SetShow( const bool bShow_ )
	{ 
		if ( NULL != m_pDLGBossGage )
		{
			CX2GageUI::SetShow( bShow_ );
			m_pDLGBossGage->SetShow( bShow_ );
		}
	}

	USHORT GetNumOfGage() const { return m_usNumOfGage; }
	void SetNumOfGage( const USHORT usNumOfGage_ ) { m_usNumOfGage = usNumOfGage_; }
	void SetNumberIcon( CKTDGUIControl::CPictureData* pPictureData_, const int iNum_ );

	bool IsOwnerGameUnit( const UidType uidBoss_ );
// 	{
// 		if ( null != m_optrGameUnit || uidBoss_ != m_optrGameUnit->GetUnitUID() )
// 			return false;
// 		else
// 			return true;
// 	}
public:
	// �⺻ ����
	const static float MAGIC_LEFT_HP_BAR_LENGTH;// = 11.f;
	const static float MAGIC_RIGHT_HP_BAR_LENGTH;// = 16.f;
	const static float MAGIC_HP_BAR_LENGTH;// = 626.f + MAGIC_LEFT_HP_BAR_LENGTH + MAGIC_RIGHT_HP_BAR_LENGTH;

	const static WCHAR WCHAR_HP_BAR_POST_FIX[KIND_OF_HP_GAGE_BAR_COLOR][MAX_NUM_OF_POST_FIX_COLOR];// = { L"_V", L"_PB" };

private:
	CX2GameUnitoPtr		m_optrGameUnit;
	CKTDGUIDialogType	m_pDLGBossGage;
	vector<wstring>		m_vecHpBarColor;
	//wstring				m_wstrHPBar[KIND_OF_HP_GAGE_BAR_COLOR];		/// HP Bar ���� Ű ��

	USHORT				m_usNumOfGage;		/// �������� ����
#ifdef SKILL_SLOT_UI_TYPE_B
	bool				m_bSkillSlotUITypeA;
#endif //SKILL_SLOT_UI_TYPE_B
};

class CX2SmallGageUI : public CX2GageUI 
{

public:
	struct DRAWFACE_RHW_VERTEX
	{
		float x, y, z, rhw;
		DWORD color;
		float u, v;
	};

	CX2SmallGageUI( CX2GameUnit* pGameUnit_ );
	virtual ~CX2SmallGageUI();

	virtual void			OnFrameMove();
	virtual	void			OnFrameRender();

	void					SetHpTexture( const WCHAR* pTexName_ );
	void					SetMpTexture( const WCHAR* pTexName_ );

protected:
	void					DrawFace( float fX_, float fY_, float fWidth_, float fHeight_, D3DCOLOR color_ /* = 0xffffffff */);

private:
	void					DrawSmallGage();
	void					DrawHpGage( const D3DXVECTOR3& vOut_ );
	void					DrawMpGage( const D3DXVECTOR3& vOut_ );

	CX2GameUnitoPtr					m_optrGameUnit;
	CKTDGStateManager::KStateID		m_RenderStateID;	/// �������̽� ����� ���� 2D ����Լ�
	CKTDXDeviceTexture*				m_pHPSmallBar;
	CKTDXDeviceTexture*				m_pMPSmallBar;
	float							m_fMPGageAlpha;		/// MP �������� Fade In, Out�� ���
};

class CX2MySmallGageUI : public CX2GageUI
{
public:
	CX2MySmallGageUI( CX2GameUnit* pGameUnit_ );
	virtual ~CX2MySmallGageUI();

	virtual void			InitUI();
	virtual void			OnFrameMove();
	
	virtual bool			GetShow() const;
	virtual void			SetShow( const bool bShow_ );

	void					CreateAirGage();
	void					DestroyAirGage();
	void					UpdateAirGage();

private:
	CX2GameUnitoPtr					m_optrGameUnit;
	CKTDGUIDialogType				m_pDlgAirGage;
};

class CX2StatusGageUI : public CX2GageUI
 {
public:
	CX2StatusGageUI( CX2GageData* pGageData_, const CX2Unit::UNIT_CLASS eOwnerUnitClass_ )
		: CX2GageUI(), m_pGageData( pGageData_ ), m_eOwnerGameUnitClass( eOwnerUnitClass_ )
	{}

	virtual ~CX2StatusGageUI();
	
	//virtual void			CrashMyStateGageBar( const D3DXVECTOR2& _min, const D3DXVECTOR2& _max, D3DXCOLOR changeColor );
	CX2Unit::UNIT_CLASS		GetOwnerUnitClass() const { return m_eOwnerGameUnitClass; }
	void					SetOwnerUnitClass( CX2Unit::UNIT_CLASS eOwnerGameUnitClass_ ) { m_eOwnerGameUnitClass = eOwnerGameUnitClass_; }

#ifdef BUFF_ICON_UI
	virtual	void			PushBuff( const BuffIcon& BuffIconInfo_, bool bIsDeBuff_);
	virtual	void			EraseBuff(BUFF_TEMPLET_ID eBuffID_, bool bIsDeBuff_);
	virtual	void			ClearBuffList();
	virtual	void			UpdateBuffIcon(){}
	virtual void			NotifyDurationTime5sec(BUFF_TEMPLET_ID eBuffID_);
	virtual void			InitBuffIconFlicker(BUFF_TEMPLET_ID eBuffID_);
#endif //BUFF_ICON_UI	

protected:
	virtual void			SetCharacterImage( const CX2Unit::UNIT_CLASS eGameUnitClass_ ) = NULL;
#ifdef REFORM_UI_CHARACTER_INFO
	void	UpdateGageForUV( IN CKTDGUIStatic* pStaticBar_, IN int iPicNum, IN const WCHAR* pUVName_, 
							 IN float fPercent_, IN bool isWidth_ );
#endif //REFORM_UI_CHARACTER_INFO

#ifdef BUFF_ICON_UI
	void	SetBuffIconStatic( const BuffIcon& BuffIconInfo_, int iIndex_, bool bIsDebuff_, const D3DXVECTOR2& vSize_ );
#ifdef SERV_IRUHADEV_BUFF_DURATION_TEXT
	void	UpdateBuffDurationText();
#endif //SERV_IRUHADEV_BUFF_DURATION_TEXT
#endif //BUFF_ICON_UI

protected:
	CX2GageData*				m_pGageData;
	CX2Unit::UNIT_CLASS			m_eOwnerGameUnitClass;
#ifdef BUFF_ICON_UI
	CKTDGUIDialogType			m_pDlgBuffIcon;		//����, ����� ������

	//��� ������ �������ֱ� ���� vector�� �۾�
	vector<BuffIcon>			m_vecBuffList;		//���� �ɷ��ִ� ���� ����Ʈ
	vector<BuffIcon>			m_vecDebuffList;	//���� �ɷ��ִ� ����� ����Ʈ
#endif //BUFF_ICON_UI
};

class CX2MyGageUI : public CX2StatusGageUI 
{
public:

public:
	CX2MyGageUI( CX2GageData* pGageData_, const CX2Unit::UNIT_CLASS eOwnerUnitClass_ );
	virtual ~CX2MyGageUI();

	virtual void			SetShow( const bool bShow_ );
	virtual void			InitUI();
	void					InitStatusUI();
	virtual void			InitWakeOrb();
	
	virtual void			OnFrameMove();
	virtual void			CrashMyStateGageUI( const D3DXVECTOR2& _min, const D3DXVECTOR2& _max, D3DXCOLOR changeColor );
#ifdef REFORM_UI_CHARACTER_INFO
	void					SetOnPopup(bool bVal){m_bOnPopUp = bVal;}
	bool					GetOnPopup() const { return m_bOnPopUp; }
#endif
#ifdef BUFF_ICON_UI
	virtual	void			UpdateBuffIcon();
	void					SetBuffIconGuideDesc();
#endif //BUFF_ICON_UI	
	virtual void			SetCharacterImage( const CX2Unit::UNIT_CLASS eGameUnitClass_ );
#ifdef FIX_CHUNG_GAGE_UI_UPDATE_BUG
	virtual void			ResetGageUIEtc( const CX2Unit::UNIT_CLASS eGameUnitClass_ ){}
#endif // FIX_CHUNG_GAGE_UI_UPDATE_BUG

protected:		
	// ������ // �����ý� ���� ���� ������ ���� �����Լ��� ����
	virtual void			UpdateMpAndHyperAndDetonationGage();
	virtual void			UpdateDetonationGage( CKTDGUIStatic* pStaticStateBar_ );	/// ���� ������

	void					UpdateHyperModeOrb();
	void					UpdateHpGage();
	void					UpdateMpGageAndString( CKTDGUIStatic* pStaticStateBar_ );
	void					UpdateMpGuidePoint();
	void					UpdateHyperGage( CKTDGUIStatic* pStaticStateBar_ );
	

	void					UpdateHyperGageString( const float fNowHyper_, const float fMaxHyper_ );
	void					UpdateHyperModeRemainTime();
	void					UpdatePartyLeaderUI();
#ifdef SERV_DUNGEON_FORCED_EXIT_SYSTEM
	bool					IsMyPlayerFaulty () const { return m_bIsMyPlayerFaulty; }
	void					IsMyPlayerFaulty (bool val_) { m_bIsMyPlayerFaulty = val_; }

#endif // SERV_DUNGEON_FORCED_EXIT_SYSTEM


#ifdef REFORM_UI_CHARACTER_INFO
	void					UpdateInfoString();
#endif

#ifdef SERV_NEW_UNIT_TRADE_LIMIT
	void				    UpdateTradeBlockDesc( std::wstring& wstrBuffDesc );
#endif // SERV_NEW_UNIT_TRADE_LIMIT

protected:
	CKTDGUIDialogType			m_pDLGMyUnit;
#ifdef REFORM_UI_CHARACTER_INFO
	bool	m_bOnPopUp;
	bool	m_bIsShow;
#ifdef SERV_DUNGEON_FORCED_EXIT_SYSTEM
	bool	m_bIsMyPlayerFaulty;
#endif // SERV_DUNGEON_FORCED_EXIT_SYSTEM

#endif
};

class CX2ChungMyGageUI : public CX2MyGageUI
{
public:
	CX2ChungMyGageUI( CX2GageData* pGageData_, const CX2Unit::UNIT_CLASS eOwnerUnitClass_ );

	virtual void			InitUI();
	virtual void			InitWakeOrb();
	void					InitCannonBallUI();

	virtual	void			OnFrameMove();
	
	void					UpdateCharacterImage();
	void					UpdateCannonBallCountUI();

	void					InitNumOfVerticalOfCannonBall( const CX2Unit::UNIT_CLASS eOwnerUnitClass_ );

	bool					GetUseExtraCannonBallGage() const { return m_bUseExtraCannonBallGage; }
	void					SetUseExtraCannonBallGage( const bool bUseExtraCannonBallGage_ ) { m_bUseExtraCannonBallGage = bUseExtraCannonBallGage_; }

#ifdef FIX_CHUNG_GAGE_UI_UPDATE_BUG
	virtual void			ResetGageUIEtc( const CX2Unit::UNIT_CLASS eGameUnitClass_ ){ InitNumOfVerticalOfCannonBall(eGameUnitClass_); InitCannonBallUI();}
#endif // FIX_CHUNG_GAGE_UI_UPDATE_BUG
protected:
	int						GetNumOfVerticalOfCannonBall() const { return m_iNumOfVerticalOfCannonBall; }
	void					SetNumOfVerticalOfCannonBall( const int iNumOfVerticalOfCannonBall_ ) { m_iNumOfVerticalOfCannonBall = iNumOfVerticalOfCannonBall_; }

	void					GetBerserkModeMyStateImage( OUT wstring& wstrFileName, OUT wstring& wstrPieceName, IN const CX2Unit::UNIT_CLASS eClassType );

private:
	int						m_iNumOfVerticalOfCannonBall;	
	bool					m_bUseExtraCannonBallGage;
	
};

class CX2ElswordMyGageUI : public CX2MyGageUI
{
public:
	CX2ElswordMyGageUI( CX2GageData* pGageData_, const CX2Unit::UNIT_CLASS eOwnerUnitClass_ )
		: CX2MyGageUI( pGageData_, eOwnerUnitClass_ ),
		m_pStaticElswordVigor( NULL ), m_pStaticElswordDest( NULL ), m_bShowWspParticle( true )
	{
		m_hSeqVigorEffect[0]	= INVALID_PARTICLE_HANDLE;
		m_hSeqVigorEffect[1]	= INVALID_PARTICLE_HANDLE;
		m_hSeqVigorEffect[2]	= INVALID_PARTICLE_HANDLE;
		m_hSeqDestEffect[0]		= INVALID_PARTICLE_HANDLE;
		m_hSeqDestEffect[1]		= INVALID_PARTICLE_HANDLE;
		m_hSeqDestEffect[2]		= INVALID_PARTICLE_HANDLE;
	}

	virtual ~CX2ElswordMyGageUI();

	virtual void			InitUI();
	virtual void			SetShow( const bool bShow_ );
	virtual	void			OnFrameMove();

	void InitUIElswordWSP();
	void StopUIEffectElswordWS();
	void PlayUIEffectDestruction();
	void PlayUIEffectVigor();	
	void UIFrameMoveElswordWSP();
	void UpdateUIElswordWSP();

protected:
	bool GetShowWspParticle() const { return m_bShowWspParticle; }
	void SetShowWspParticle( const bool bShowWspParticle_ );

private:
	CKTDGUIStatic*										m_pStaticElswordVigor;
	CKTDGUIStatic*										m_pStaticElswordDest;
	CKTDGParticleSystem::CParticleEventSequenceHandle 	m_hSeqVigorEffect[3];
	CKTDGParticleSystem::CParticleEventSequenceHandle 	m_hSeqDestEffect[3];

	bool												m_bShowWspParticle;
	
};

class CX2RavenMyGageUI : public CX2MyGageUI 
{
public:
	CX2RavenMyGageUI( CX2GageData* pGageData_, const CX2Unit::UNIT_CLASS eOwnerUnitClass_ );
	virtual ~CX2RavenMyGageUI();

	virtual void			InitWakeOrb();
	virtual void			OnFrameMove();
	void					UpdateHyperOrb();

private:
	CKTDGParticleSystem::CParticleEventSequenceHandle m_hSeqHyperBall;
};

class CX2PartyMemberGageUI : public CX2StatusGageUI
{

#ifdef SERV_DUNGEON_FORCED_EXIT_SYSTEM		// 13-01-15 / ���� �÷��� ��, �ҷ� ���� ���� ���� �ý���

public :
	enum BAN_VOTE_BUTTON_TYPE	// ��ǥ ��ư Ÿ��
	{ 
		BVBT_NONE = 0,  		// ���� ���� ��, 
		BVBT_NEED_VOTE,			// ��ǥ �ؾ��ϴ� ����
		BVBT_COMPLETE_VOTE,		// ��ǥ �Ϸ�
//		BVBT_FAULTY_USER_VOTE,	// �ҷ� ���� ����
	};

#endif // SERV_DUNGEON_FORCED_EXIT_SYSTEM		// 13-01-15 / ���� �÷��� ��, �ҷ� ���� ���� ���� �ý���




public:
	CX2PartyMemberGageUI( const KPartyUserInfo& kPartyUserInfo_, CX2GageData* pGageData_, const UINT uiPositionIndex_ ) 
		: CX2StatusGageUI( pGageData_, static_cast<CX2Unit::UNIT_CLASS>( kPartyUserInfo_.m_cUnitClass ) ),
		m_pDLGOtherUnit( NULL ), m_wstrNickName( kPartyUserInfo_.m_wstrNickName ),
		m_uiLevel( kPartyUserInfo_.m_ucLevel ), m_uidGameUnit( kPartyUserInfo_.m_iUnitUID ), 
		m_uiPositionIndex( uiPositionIndex_ ), 
#ifdef SERV_DUNGEON_FORCED_EXIT_SYSTEM		// 13-01-15 / ���� �÷��� ��, �ҷ� ���� ���� ���� �ý���
		m_iVoteButtonType ( BVBT_NONE ),		// PartyMemberGageUI �� ���� ��ư Ÿ��
		m_pButtonVotePlayer ( NULL ),			// �ش� ��ư�� Dlg ������
		m_pButtonVoteComplete ( NULL ),			// �ش� ��ư�� Dlg ������
#endif // SERV_DUNGEON_FORCED_EXIT_SYSTEM		// 13-01-15 / ���� �÷��� ��, �ҷ� ���� ���� ���� �ý���
		
#ifdef REFORM_UI_CHARACTER_INFO
		m_bOnPopUp( false )
#endif
#ifdef SKILL_SLOT_UI_TYPE_B
		, m_bIsSkillUITypeA ( false )
#endif //SKILL_SLOT_UI_TYPE_B
	{}

	virtual ~CX2PartyMemberGageUI();

	virtual void			InitUI();	
	void					SetLevelString( const UINT uiLevel_ );
	void					SetNickNameString( const wstring& wstrNickName_ );
	void					SetPosition( const UINT uiPositionIndex_ );

	virtual void			SetShow( const bool bShow_ );

	virtual void			OnFrameMove();
	
	virtual void			CrashMyStateGageUI( const D3DXVECTOR2& _min, const D3DXVECTOR2& _max, D3DXCOLOR changeColor ) { ASSERT( !L"Crash Base Function is not available" ); }
	virtual void			SetHpTexture( const WCHAR* pTexName_ ) { ASSERT( !L"Hp Base function is not available"); }
	virtual void			SetMpTexture( const WCHAR* pTexName_ ) { ASSERT( !L"Mp Base function is not available"); }

	void					UpdateGageDataFromGameUnit();
	void					UpdateResurrectShortKey();
	void					UpdatePartyLeaderUI();

	UINT					GetPositionIndex() const { return m_uiPositionIndex; }
	void					SetPositionIndex( const UINT uiPosIndex_ ) 
	{ 
		m_uiPositionIndex = uiPosIndex_; 
#ifdef SERV_DUNGEON_FORCED_EXIT_SYSTEM
		if( NULL != m_pButtonVotePlayer )
		{
			if( false == m_pButtonVotePlayer->SetDummyInt(0, m_uiPositionIndex ) )
				m_pButtonVotePlayer->AddDummyInt( m_uiPositionIndex );
		}
#endif //SERV_DUNGEON_FORCED_EXIT_SYSTEM
	}
	
	//CX2Unit::UNIT_CLASS GetOwnerUnitClass() const { return m_eOwnerGameUnitClass; }
	//void SetOwnerUnitClass( CX2Unit::UNIT_CLASS eOwnerGameUnitClass_ ) { m_eOwnerGameUnitClass = eOwnerGameUnitClass_; }

#ifdef REFORM_UI_CHARACTER_INFO
	bool					GetOnPopup() const { return m_bOnPopUp; }
#endif

#ifdef BUFF_ICON_UI
	virtual		void		UpdateBuffIcon();
#endif //BUFF_ICON_UI	

#ifdef SERV_DUNGEON_FORCED_EXIT_SYSTEM		// 13-01-15 / ���� �÷��� ��, �ҷ� ���� ���� ���� �ý���
	void					UpdateFaultyPlayerUI ( const bool b_faultyValue_ );											// ��ǥ ��ư�� UI �� Update ��
	void					SetShowRelationVoteButtons ( const bool bVote_, const bool bComplete_ );		// ��ǥ ��ư�� ��ǥ �Ϸ� ��ư�� �����ֱ� ������ ��	
	BAN_VOTE_BUTTON_TYPE	GetVoteButtonType () const { return m_iVoteButtonType; }				// ��ǥ ��ư�� ���¸� ��ȯ
	void					SetVoteButtonType ( const BAN_VOTE_BUTTON_TYPE iVoteButtonType_ ) { m_iVoteButtonType = iVoteButtonType_; }	// ��ǥ ��ư�� ���¸� ����
	CKTDGUIButton *			GetVoteReadyButton () const;				// ��ǥ �غ� ��ư�� ��ȯ
	CKTDGUIButton *			GetVoteCompleteButton () const { return m_pButtonVoteComplete; }		// ��ǥ �Ϸ� ��ư�� ��ȯ
	const wstring &			GetNicknameThisPartyMemeber() const { return m_wstrNickName; }		// �ش� GageUI �� ���� �ִ� �������� ���̵� ��ȯ��
	const UidType &			GetUidTypeThisPartyMember () const { return m_uidGameUnit; }			// �ش� GageUI �� ���� �ִ� �������� UID �� ��ȯ�� 
#endif // SERV_DUNGEON_FORCED_EXIT_SYSTEM		// 13-01-15 / ���� �÷��� ��, �ҷ� ���� ���� ���� �ý���


protected:
	virtual		void CX2PartyMemberGageUI::SetCharacterImage( const CX2Unit::UNIT_CLASS eGameUnitClass_ );



protected:
	CKTDGUIDialogType		m_pDLGOtherUnit;

private:
	wstring					m_wstrNickName;
	UINT					m_uiLevel;
	UidType					m_uidGameUnit;
	UINT					m_uiPositionIndex;

#ifdef REFORM_UI_CHARACTER_INFO
	bool	m_bOnPopUp;
#endif
#ifdef SKILL_SLOT_UI_TYPE_B
	bool					m_bIsSkillUITypeA;
#endif //SKILL_SLOT_UI_TYPE_B.

#ifdef SERV_DUNGEON_FORCED_EXIT_SYSTEM		// 13-01-15 / ���� �÷��� ��, �ҷ� ���� ���� ���� �ý���
	CKTDGUIButton *			m_pButtonVotePlayer;			// �����ϱ� ��ư
	CKTDGUIButton *			m_pButtonVoteComplete;			// ����Ϸ� ��ư
	BAN_VOTE_BUTTON_TYPE	m_iVoteButtonType;				// ���� ��ư�� Ÿ��, 0 = �ƹ� ���µ� �ƴ�, 1 = ���� ��ư �Է� �޴� ��, 2 = �Է� �ޱ� �Ϸ�, 3 = �ҷ� ���� �Ǻ�
#endif // SERV_DUNGEON_FORCED_EXIT_SYSTEM


};

class CX2PVPPlayerGageUI : public CX2StatusGageUI
{
public:
	CX2PVPPlayerGageUI( const CX2Room::SlotData& pPvpSlot_, CX2GageData* pGageData_, const UINT uiPositionIndex_, char cRank_ )
		: CX2StatusGageUI( pGageData_, pPvpSlot_.m_pUnit->GetClass() ), m_wstrNickName( pPvpSlot_.m_pUnit->GetNickName() ),
		m_uiLevel( pPvpSlot_.m_pUnit->GetPrevLevel() ), m_uiPositionIndex( uiPositionIndex_ ), m_uiMyTeam( pPvpSlot_.m_TeamNum ),
		m_cRank(cRank_), m_iNpcId(-1)
#ifdef SKILL_SLOT_UI_TYPE_B
		, m_bIsSkillUITypeA ( false )
#endif //SKILL_SLOT_UI_TYPE_B
	{}


	CX2PVPPlayerGageUI( const CX2Room::RoomNpcSlot& npcSlotData_, CX2GageData* pGageData_, const UINT uiPositionIndex_, char cRank_ )
		: CX2StatusGageUI( pGageData_, CX2Unit::UC_NONE ), m_wstrNickName( npcSlotData_.m_wstrNpcName ),
		m_uiLevel( -1 ), m_uiPositionIndex( uiPositionIndex_ ), m_uiMyTeam(  CX2Room::TN_BLUE ),
		m_cRank(cRank_), m_iNpcId( npcSlotData_.m_iNpcId )
#ifdef SKILL_SLOT_UI_TYPE_B
		, m_bIsSkillUITypeA ( false )
#endif //SKILL_SLOT_UI_TYPE_B
	{}
	
	virtual ~CX2PVPPlayerGageUI();

	virtual void	InitUI();
	void			SetLevelString( IN const UINT uiLevel_ );
	void			SetNickNameString( IN const wstring& wstrNickName );
	void			SetPosition( IN const UINT uiPositionIndex_ );
	virtual void	SetShow( const bool bShow_ );

	virtual void	OnFrameMove();
	void			UpdatePvpRank();

	virtual void	CrashMyStateGageUI( const D3DXVECTOR2& _min, const D3DXVECTOR2& _max, D3DXCOLOR changeColor )
	{
		ASSERT( !L"Crash Base Function is not available" );
	}
	virtual void	SetHpTexture( const WCHAR* pTexName_ )
	{
		ASSERT( !L"Hp Base function is not available");
	}
	virtual void	SetMpTexture( const WCHAR* pTexName_ )
	{
		ASSERT( !L"Mp Base function is not available");
	}

	void	UpdateGageDataFromGameUnit();
	UINT	GetPositionIndex() const
	{
		return m_uiPositionIndex;
	}
	void	SetPositionIndex( const UINT uiPosIndex_ )
	{
		m_uiPositionIndex	= uiPosIndex_;
	}

	void	SetRankImage();

#ifdef BUFF_ICON_UI
	virtual	void			UpdateBuffIcon();
#endif //BUFF_ICON_UI

protected:
	virtual void CX2PVPPlayerGageUI::SetCharacterImage( const CX2Unit::UNIT_CLASS eGameUnitClass_ );


protected:
	CKTDGUIDialogType		m_pDLGOtherUnit;

private:
	wstring		m_wstrNickName;		/// ĳ���͸�
	UINT		m_uiLevel;			/// ����
	UINT		m_uiPositionIndex;	/// ������ ���� �� ī���� ����
	UINT		m_uiMyTeam;			/// RED, BLUE, NONE �� ����
	char		m_cRank;
	int			m_iNpcId;			/// ����NPC�ʻ�ȭ ������ ���� NPCUid
#ifdef SKILL_SLOT_UI_TYPE_B
	bool					m_bIsSkillUITypeA;
#endif //SKILL_SLOT_UI_TYPE_B
};

#pragma region CX2AraMyGageUI
/** @class : CX2AraMyGageUI
	@brief : �ƶ��� ��� UI
	@date  : 2012/11/30
*/
class CX2AraMyGageUI : public CX2MyGageUI
{
public:
	CX2AraMyGageUI( CX2GageData* pGageData_, const CX2Unit::UNIT_CLASS eOwnerUnitClass_ );

	virtual void	InitUI();
	virtual void	InitWakeOrb();
	virtual	void	OnFrameMove();
	
	void	InitForcePowerUI();
	void	UpdateCharacterImage();
	void	UpdateForcePowerUI();
		
protected:
	void	GetBerserkModeMyStateImage( OUT wstring& wstrFileName, OUT wstring& wstrPieceName, IN const CX2Unit::UNIT_CLASS eClassType );
};
#pragma endregion Ŭ����

#ifdef NEW_CHARACTER_EL
class CX2ElesisMyGageUI : public CX2MyGageUI
{
public:
	CX2ElesisMyGageUI( CX2GageData* pGageData_, const CX2Unit::UNIT_CLASS eOwnerUnitClass_ )
		: CX2MyGageUI( pGageData_, eOwnerUnitClass_ ),
		m_pStaticElswordVigor( NULL ), m_pStaticElswordDest( NULL ), m_bShowWspParticle( true )
	{
		m_hSeqVigorEffect[0]	= INVALID_PARTICLE_HANDLE;
		m_hSeqVigorEffect[1]	= INVALID_PARTICLE_HANDLE;
		m_hSeqVigorEffect[2]	= INVALID_PARTICLE_HANDLE;
		m_hSeqDestEffect[0]		= INVALID_PARTICLE_HANDLE;
		m_hSeqDestEffect[1]		= INVALID_PARTICLE_HANDLE;
		m_hSeqDestEffect[2]		= INVALID_PARTICLE_HANDLE;
	}

	virtual ~CX2ElesisMyGageUI();

	virtual void			InitUI();
	virtual void			SetShow( const bool bShow_ );
	virtual	void			OnFrameMove();

	void InitUIElswordWSP();
	void StopUIEffectElswordWS();
	void PlayUIEffectDestruction();
	void PlayUIEffectVigor();	
	void UIFrameMoveElswordWSP();
	void UpdateUIElswordWSP();

protected:
	bool GetShowWspParticle() const { return m_bShowWspParticle; }
	void SetShowWspParticle( const bool bShowWspParticle_ );

	virtual void UpdateMpAndHyperAndDetonationGage();
	virtual void UpdateDetonationGage( CKTDGUIStatic* pStaticStateBar_ );	/// ���� ������

private:
	CKTDGUIStatic*										m_pStaticElswordVigor;
	CKTDGUIStatic*										m_pStaticElswordDest;
	CKTDGParticleSystem::CParticleEventSequenceHandle 	m_hSeqVigorEffect[3];
	CKTDGParticleSystem::CParticleEventSequenceHandle 	m_hSeqDestEffect[3];

	bool												m_bShowWspParticle;

};
#endif // NEW_CHARACTER_EL