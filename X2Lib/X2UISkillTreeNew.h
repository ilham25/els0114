#ifdef UPGRADE_SKILL_SYSTEM_2013

#pragma once

class CX2EquippedSkillSlotData;
class CX2SkillTreeSlotData;

class CX2UISkillTreeNew
{
public:

	enum SKILL_TREE_UI_CUSTOM_MSG
	{
		STUCM_EXIT	= 29000,
	
		STUCM_BUTTON_SKILL_LEVEL_PLUS,
		STUCM_BUTTON_SKILL_LEVEL_PLUS_MOUSEOVER,
		STUCM_BUTTON_SKILL_LEVEL_PLUS_MOUSEOUT,

		STUCM_BUTTON_SKILL_LEVEL_MINUS,
		STUCM_BUTTON_SKILL_LEVEL_MINUS_MOUSEOVER,
		STUCM_BUTTON_SKILL_LEVEL_MINUS_MOUSEOUT,

		STUCM_BUTTON_SKILL_MASTER_MOUSEOVER,
		STUCM_BUTTON_SKILL_MASTER_MOUSEOUT,

		STUCM_EQUIPSLOT_DROPPED,
		STUCM_EQUIPSLOT_RMOUSEUP,
		STUCM_EQUIPSLOT_MOUSEIN,
		STUCM_EQUIPSLOT_MOUSEOUT,	

		STUCM_SKILLTREESLOT_RMOUSEUP,
		STUCM_SKILLTREESLOT_MOUSEIN,
		STUCM_SKILLTREESLOT_MOUSEOUT,

		STUCM_SKILL_RESET_MODE_CHANGE,
		STUCM_SKILL_RESET_MODE_CHANGE_MOUSEOVER,
		STUCM_SKILL_RESET_MODE_CHANGE_MOUSEOUT,

		STUCM_CONTROLLIST_SCROLL,

		STUCM_NAVI_LEFT_CLICKED,
		STUCM_NAVI_DRAGGING,

		STUCM_INGAME_ICON_MOUSE_OVER,
		STUCM_INGAME_ICON_MOUSE_OUT,

		STUCM_FIRST_GUIDE_CLOSE,
		STUCM_FIRST_GUIDE_DONOTSHOW,

		STUCM_CSP_GUIDE_MOUSEIN,
		STUCM_CSP_GUIDE_MOUSEOUT,
		STUCM_SKILLTREESLOTB_GUIDE_MOUSEIN,
		STUCM_SKILLTREESLOTB_GUIDE_MOUSEOUT,

		STUCM_UNSEAL_SKILL_CANCLE,
		STUCM_UNSEAL_SKILL_BUY,

		STUCM_OPEN_COMBO_TREE,

		STUCM_INGAME_ELSWORD_WSS_MOUSE_OVER,
		STUCM_INGAME_ELSWORD_WSS_MOUSE_OUT,

//#ifdef GUIDE_SKILL_SLOTB
		STUCM_SKILLTREESLOTB_BUY_GUIDE_MOUSEIN,
		STUCM_SKILLTREESLOTB_BUY_GUIDE_MOUSEOUT,
//#endif GUIDE_SKILL_SLOTB
//#ifdef GUIDE_SKILL_LEVEL_MINUS
		STUCM_BUTTON_SKILL_LEVEL_MINUS_GUIDE_MOUSEOVER,
		STUCM_BUTTON_SKILL_LEVEL_MINUS_GUIDE_MOUSEOUT, 
//#endif GUIDE_SKILL_LEVEL_MINUS

		STUCM_BUTTON_SKILL_LEVEL_PLUS_RESULT,		/// �����ϱ� ��ư ������ ��
		STUCM_BUTTON_SKILL_LEVEL_PLUS_CANCEL,		/// ��� ��ư�� ������ ��

		STUCM_CHANGE_SKILL_LEVEL_PLUS,				/// ��ų ���� ó��
		STUCM_BUTTON_SKILL_LEVEL_UPGRADE,			/// ��ų ��ȭ ��ư�� ������ ��
		STUCM_BUTTON_EXIT,							/// �ݱ� ��ư�� ������ ��
		STUCM_EXIT_WITH_SKILL_LEVEL_PLUS,			/// �����鼭 ��ų ���� ���� ó��

		STUCM_BUTTON_ENABLE_INIT_SKILL,				/// ��ų Ʈ�� �� �ʱ�ȭ ��ư
		STUCM_BUTTON_INIT_SKILL,					/// ��ų ���� �ʱ�ȭ ��ư
		STUCM_INIT_SKILL,							/// ��ų ���� �ʱ�ȭ ó�� ( 0���� )
		STUCM_INIT_SKILL_ONE_POINT,					/// ��ų ���� �ʱ�ȭ ó�� ( 1�� )
	};

	/// ��ų Ʈ���� �ִ� ��ų ���� Ÿ��
	enum SKILL_TREE_SLOT_TYPE
	{
		STST_NONE				= -1,
		STST_SELECT_SKILL_LEFT	= 0,	/// 2������ ����
		STST_SELECT_SKILL_RIGHT	= 1,	/// 2������ ������
		STST_SINGLE_SKILL		= 2,	/// ���� ���� �̱� ��ų 
		STST_END				= 3,
	};

	/// ��ų Ʈ���� �ִ� ��ų ������ ��ư Ÿ��
	enum SKILL_TREE_SLOT_BUTTON_TYPE
	{
		STSBT_NONE					= -1,
		STSBT_LEARN_SKILL			= 0,	/// ��ų ���� ��ư
		STSBT_UPGRADE_SKILL			= 1,	/// ��ų ��ȭ ��ư
		STSBT_CAN_NOT_LEARN_SKILL	= 2,	/// ��ų ���� �Ұ� ��ư
		STSBT_INIT_SKILL			= 3,	/// ��ų �ʱ�ȭ ��ư
		STSBT_END					= 5,
	};

	struct SkillSlotUI
	{
		CKTDGUISlot*	m_pSlot;					/// ��ų ����
		CKTDGUIStatic*	m_pStaticBG;				/// ��� ����
		CKTDGUIStatic*	m_pStaticSkillBG;			/// ��ų ����
		CKTDGUIStatic*	m_pStaticSkillLock;			/// ��� ��ų
		CKTDGUIButton*	m_pButtonSkillInit;			/// ��ų �ʱ�ȭ ��ư
		CKTDGUIButton*	m_pButtonSkillDesc;			/// ��ų ���� ǥ�ÿ� ��ư
		CKTDGUIButton*	m_pButtonSkillLearn;		/// ��ų ���� ��ư
		CKTDGUIStatic*	m_pStaticSkillMaster;		/// ��ų ������ �̹���
		CKTDGUIStatic*	m_pStaticSkillNoLearn;		/// ��ų ���� �Ұ� �̹���
		CKTDGUIStatic*	m_pStaticStringRequireSP;	/// �䱸 SP
		bool			m_bSelectSkillBlind;		/// 2������ ��ų��, ���õ��� �ʾ� �����ε� ó���ؾ� �� ����
		bool			m_bSkillLineBlind;			/// ������ �������� �ʾ� ��ܹ��� ��ų ���� ó�� ����
		

		SkillSlotUI()
		{
			m_pSlot						= NULL;
			m_pStaticBG					= NULL;
			m_pStaticSkillBG			= NULL;
			m_pStaticSkillLock			= NULL;
			m_pButtonSkillInit			= NULL;
			m_pButtonSkillDesc			= NULL;
			m_pButtonSkillLearn			= NULL;
			m_pStaticSkillMaster		= NULL;
			m_pStaticSkillNoLearn		= NULL;
			m_pStaticStringRequireSP	= NULL;
			m_bSelectSkillBlind			= false;
			m_bSkillLineBlind			= false;
		}

	public:
		void SetSkillLineBlind(bool val) { m_bSkillLineBlind = val; }

		// �Ҹ��ڴ� �ʿ���� : ���� Dialog �Ҹ��ڿ��� �����
	};

	struct SkillPositionKey
	{
		int	m_iRow;		/// ���� ��ġ
		int	m_iCol;		/// ���� ��ġ

		SkillPositionKey() :
		m_iRow( 0 ),
		m_iCol( 0 )
		{
		}

		SkillPositionKey( int iRow, int iCol )
		{
			m_iRow = iRow;
			m_iCol = iCol;
		}

		bool operator < ( const SkillPositionKey& rhs ) const 
		{

			if( m_iRow < rhs.m_iRow )
				return true;
			else if( m_iRow > rhs.m_iRow )
				return false;

			if( m_iCol < rhs.m_iCol )
				return true;

			
			return false;
		}
	};

public:

	CX2UISkillTreeNew( CKTDXStage* pNowStage );
	virtual ~CX2UISkillTreeNew(void);

	HRESULT OnFrameMove( double fTime, float fElapsedTime );
	bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	bool OnMouseWheel(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	bool UICustomEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	
	bool GetShow() { return m_bShow; }
	void SetShow(bool val);
	void SetLayer( X2_DIALOG_LAYER layer );
	D3DXVECTOR2 GetDLGSize(){ return m_DLGSize; }
	void SetPosition( D3DXVECTOR2 vec );

	void InitSkillTreeUI();
	void UpdateSPInfo();
	
	void InitializeSkillEquipSlot();
	CX2EquippedSkillSlotData* GetEquippedSlotData( int index, bool bSlotB = false );
#ifdef REFORM_UI_SKILLSLOT
	CKTDGUISlot* GetEquippedSlot( int index, bool bSlotB = false );
#endif //REFORM_UI_SKILLSLOT
	void UpdateSkillEquipedSlotUI();
	
	void CreateSkillTree( CX2Unit::UNIT_CLASS eUnitClass );
	void UpdateSkillTree();
	void UpdateSkillTree( CX2SkillTree::SKILL_ID eSkillID );			// Tree���� �ش� Slot�� ������Ʈ�Ѵ�
	void UpdateBlind();

	void UpdateUI( bool bEquipped, bool bSP = true, bool bSkillTree = false );
    
	void CreateSlotUIPreset( SkillSlotUI& UISet, int eSkillID, int iCol );

	const map<CX2SkillTree::SKILL_ID, SkillSlotUI>& GetMapSkillSlotUISet(){ return m_mapSkillSlotUISet; }

	bool GetRemoveMode(){ return m_bSkillRemoveMode; }
	void InitEquippedSkillSlotData();

	void UpdateSkillCoolTime( float fElapsedTime );
	//void UpdateSkillSlotPosition();
	bool GetSkillSlotSwap()const {return m_bSkillSlotSwap;}
	void SwapSkillSlot(bool bVal, bool bForce = false);
	void SetOffsetPosSkillSLotStatic(const D3DXVECTOR2& vPos_, const D3DXVECTOR2& vAddPos_ = D3DXVECTOR2(0,0));
	void SetOffsetStatic( const WCHAR* strControl_, const D3DXVECTOR2& vPos_);

	//////////////////////////////////////////////////////////////////////////

	//** fix!! ��ų �������� ���̾�α׸� ���� �Լ�. �߰� �۾� �ʿ��մϴ�.
	void OpenSkillUnsealInformDLG( const CX2SkillTree::SkillTemplet* pSkillTemplet, D3DXVECTOR2 SlotCenterPos );	

	void OpenDLGSkillUnsealed( bool bOpen );
	void UpdateDLGSkillUnsealed( CX2SkillTree::SKILL_ID eSkillID );


	
	//////////////////////////////////////////////////////////////////////////
	void GetPrecedingSkillDesc( OUT wstring& wstrDesc, CX2SkillTree::SKILL_ID ePrecedingSkill ) const;
	void ShowSkillDesc( bool bShow, CX2SkillTree::SKILL_ID eSkillID = CX2SkillTree::SI_NONE, D3DXVECTOR2 pos = D3DXVECTOR2(0,0), 
						D3DXVECTOR2 size = D3DXVECTOR2(0,0), bool bEquipped = false );
	void GetSkillEffectiveDesc( OUT wstring& wstrDesc, IN int iSkillLevel, IN int iIncreasekillLevel, IN const CX2SkillTree::SkillTemplet* pSkillTemplet ) const;
	void GetSkillTitleDesc( OUT wstring& wstrTitleDesc, IN const CX2SkillTree::SkillTemplet* pSkillTemplet, 
							OUT const CX2SkillTree::SkillTreeTemplet* pSkillTreeTemplet,IN int iSkillLevel, IN int iIncreasekillLevel ) const;
	void GetSkillSubDesc( IN const CX2SkillTree::SkillTemplet* pSkillTemplet, IN const int iTotalSkillLevel, OUT wstring& wstrSubDesc ) const;
	void GetSkillUnSealedDesc( OUT wstring& wstrDesc, IN const CX2SkillTree::SKILL_ID eSkillID ) const;

	void ShowUIDesc( bool bShow, wstring wstr = L"", D3DXVECTOR2 pos = D3DXVECTOR2(0,0), 
					 CX2TalkBoxManagerImp::TALK_BOX_TYPE TBT = CX2TalkBoxManagerImp::TBT_FROM_UP_LEFT,
					 D3DXCOLOR coTextColor = D3DXCOLOR(0,0,0,1), D3DXCOLOR coBackColor = D3DXCOLOR(1, 0.96f, 0.6f, 1 ) );
	//wstring GetExpirationDateDesc( const wstring& wstrExpirationDate );	// kimhc // KTDXCommonFunc�� �ű� // 2009-06-16
	
	//////////////////////////////////////////////////////////////////////////

//{{ kimhc // 2010.3.26 // ���� ��ų ���� ����
#ifdef MODIFY_INFINITE_SKILL_BUG
	bool ProcessSkillTreeSlotRMouseUp( LPARAM lParam );	// ��ųƮ������ ���콺 ��Ŭ�� ���� ���� ó��
	bool ProcessEquipSlotDropped( LPARAM lParam );		// ������������ �巡�׾� ��� ���� ���� ó��
	bool ProcessEquipSlotRMouseUp( LPARAM lParam );		// �������Կ��� ���콺 ��Ŭ�� ���� ���� ó��

// 	bool CheckIsReady();					// ���� ���� �����̸� true, ���� ���°� �ƴϸ� false �� ����
// 	void ShowOkAndCancelMsgForUnReady( LPARAM lParam, SKILL_TREE_UI_CUSTOM_MSG eSTUCM ); // ���� ���¸� Ǯ�ڳĴ� ok, cancel �޽��� �ڽ�, ���ڴ� �����Ϸ��� UICustomMSG���� �Ѱ��� lParam��(����)	

	//void ProcessChangeEquipSlotAfterUnReady();
#endif MODIFY_INFINITE_SKILL_BUG
//}} kimhc // 2010.3.26 // ���� ��ų ���� ����

#ifdef REFORM_UI_SKILLSLOT
	CKTDGUIDialogType	GetDLGGameSkillSLot() {return m_pDLGGameSkillSlot; }
//{{ Iruha : 2026-08-25 // Skill Slot B open by default, no medal purchase required
#ifdef SERV_IRUHADEV_SKILL_SLOT_B_FREE
	bool				IsExpandSkillSlot()	{return true; }
#else
	bool				IsExpandSkillSlot()	{return m_bIsExpandSkillSlot; }
#endif SERV_IRUHADEV_SKILL_SLOT_B_FREE
//}}
	bool				GetShowEquipSkillGuide() {return m_bShowEquipSkillGuide; }
#endif //REFORM_UI_SKILLSLOT

#ifdef REFORM_UI_KEYPAD
	void UpdateSkillSlotString();
#endif

#ifdef SKILL_SLOT_UI_TYPE_B
	void				SetSkillUIType( bool bIsTypeA_ );
	bool				GetIsSkillUiTypeA() const { return m_bIsSkillUITypeA; }
	void				SwapSkillSlotTypeB( bool bVal_, bool bForce_= false);
	void				SwapSkillSlotTypeA( bool bVal_, bool bForce_= false);
	void				SetOffsetPosEtcSkillSlotUI();
#endif //SKILL_SLOT_UI_TYPE_B

	/// ���� ��ųâ���� ��ų ������ �ø��� �ִ°�
	bool GetNowLearnSkill() const { return m_bNowLearnSkill; }

	/// ���� ��ųâ���� ��ų ������ �ʱ�ȭ �ϰ� �ִ°�
	bool GetNowInitSkill() const { return m_bNowInitSkill; }

	bool													IsLearnBeforeSelectSkill( const int iNowTierIndex );						/// ���� �ܰ��� 2������ ��ų�� ������� ���θ� ��ȯ

	const int												GetNotLearnTierIndex();														/// ����� ���� �ֻ��� ��ų ���� ��ȯ
	const int												GetNotEnoughLevelTierIndex( const int iLevel );								/// ��� �� ���� ������ �ֻ��� ��ų ���� ��ȯ
	const map<SkillPositionKey,	CX2SkillTree::SKILL_ID>&	GetMapSkillPosition() { return m_mapSkillPosition; }						/// ��ġ�� ���� ��ų ���̵� ���� �����̳� ��ȯ
	const CKTDGUIDialogType									GetDLGUISkillTree() { return m_pDLGUISkillTree; }							/// ��ų Ʈ�� ���̾�α� ��ü ��ȯ
	const int												GetSkillLevelInSkillWimdow( const CX2SkillTree::SKILL_ID eSkillID ) const;	/// ��ųâ �� ��ų ���̵� ��ȯ
	const CX2SkillTree::SKILL_ID							GetAnotherSelectSkillID( CX2SkillTree::SKILL_ID eCheckSkillID ) const;		/// 2������ ��ų �� �ݴ����� �ִ� ��ų ���̵� ��ȯ
	const int												GetRequireSkillLineLevel( int iSkillLineTierIndex ) const					/// ��ų Tire�� ���� ��ų ���� �䱸 ����
	{ 
		if ( iSkillLineTierIndex >= ARRAY_SIZE( OPEN_SKILL_TREE_LEVEL ) )
		{
			/// �� ���κ� ���� ���� �迭���� ū ���� �ε����� �����Ϸ��� �ؿ�!!!
			ASSERT( !L"NowSkillLineLevel Is Large Then OPEN_SKILL_TREE_LEVEL Array Size!!!" );
			return 0;
		}

		return OPEN_SKILL_TREE_LEVEL[iSkillLineTierIndex];
	}

private:
#ifdef ELSWORD_WAY_OF_SWORD
	void					DrawElswordWSSDesc( D3DXVECTOR2 vDrawPos_ );
#endif ELSWORD_WAY_OF_SWORD

	void					CreateSkillTreePortrait( CKTDGUIContolList* pControlList );									/// ��ų Ʈ�� �ʻ�ȭ �� ���м� ���� �Լ�

	void					UpdateAtMoveScrollBar();																	/// ��ũ�� �ٰ� ������ �� ����Ǿ�� �� ������ ó���ϴ� �Լ�

	void					SetSelectSkillSlotPosition();																/// 2������ ��ų�� ���� ���� �Լ�
	void					SetDivisionLine( CKTDGUIContolList* pControlList );											/// ���м� ���� �Լ�
	void					SetLearnSkillInfo( CX2SkillTree::SKILL_ID eSkillID );										/// ��ų ������ ���� KGetSkillInfo �� ���� �Լ�
	void					SetBlindSelectSkill();																		/// 2������ ��ų �� ���õ��� ���� ��ų�� ���� �����ε� ����
	void					SetSkillSlotButton( CKTDGUIButton* pSlotButton, SKILL_TREE_SLOT_BUTTON_TYPE eButtonType );	/// ��ų Ʈ���� ��ų ���� ��ư ���� �Լ�
	void					SetUnitClassImageAndName(	CKTDGUIStatic* pStatic,											/// ��ų Ʈ�� �ʻ�ȭ�� ���� �̹��� �� ������ ���� �Լ�
														CX2Unit::UNIT_CLASS eUnitClass = CX2Unit::UC_ARME_VOID_PRINCESS, 
														CX2Unit::UNIT_CLASS_TYPE UnitClassLevel = CX2Unit::UCT_BASIC_CLASS );

	bool					m_bShow;
	bool					m_bSkillRemoveMode;
	bool					m_bClassInfoVaild;
	//D3DXVECTOR2				m_MovedPosition;
	//D3DXVECTOR2				m_DLGPosition;
	D3DXVECTOR2				m_DLGSize;

	CKTDGUIDialogType			m_pDLGUISkillTree;   
	CKTDGUIDialogType			m_pDLGSkillToolTip;	
	CKTDGUIDialogType			m_pDLGSkillUnsealInform;	
	CKTDGUIDialogType			m_pDLGSkillUnsealed;		// ��ų �������� �Ϸ� �˾�
#ifdef ELSWORD_WAY_OF_SWORD
	CKTDGUIDialogType			m_pDLGElswordWSSDesc;
#endif ELSWORD_WAY_OF_SWORD


	map<CX2SkillTree::SKILL_ID, SkillSlotUI>		m_mapSkillSlotUISet;
	map<SkillPositionKey, CX2SkillTree::SKILL_ID>	m_mapSkillPosition;

	CX2UserSkillTree*		m_pUserSkillTree;
		
	// ���콺 �� ����
	SHORT				m_SumDelta;
	int					m_iPrevSP;	// UpdateSP�� �� �ٶ� ���� SP ���¿� ���ؼ� Tree Update�� �ʿ����� �����Ѵ�

	// Navigation ����
	float				m_fNaviMoveRange;

	// ToolTip : TalkBox �̿��ؼ�.
	CX2TalkBoxManager*		m_pTalkBoxManager;
	bool					m_bUsingCSP;

#ifdef OPEN_SKILL_GUIDE_FIRST_TIME
	CKTDGUIDialogType		m_pDLGFirstGuide;
#endif

#ifdef REFORM_UI_SKILLSLOT
	bool					m_bIsExpandSkillSlot;
	CKTDGUIDialogType		m_pDLGGameSkillSlot;
	bool					m_bShowEquipSkillGuide;
#endif //REFORM_UI_SKILLSLOT

	bool					m_bSkillSlotSwap;			/// ��ų ���� ���� �õ�

#ifdef SKILL_SLOT_UI_TYPE_B
	bool					m_bIsSkillUITypeA;			/// ��ų ���� Ÿ��
#endif //SKILL_SLOT_UI_TYPE_B

	int						m_iMaxSkillRowNum;			/// �ִ� ��ų �迭 ����

	bool					m_bNowLearnSkill;			/// ���� ��ųâ���� ��ų ������ �ø��� �ִ°�
	bool					m_bNowInitSkill;			/// ���� ��ųâ���� ��ų ������ �ʱ�ȭ �ϰ� �ִ°�

	CX2SkillTree::SKILL_ID	m_eCheckSkillID;			/// �޽��� �̺�Ʈ�� ���� ����� ��ų ���̵�

	bool					m_bReplacedEsCounter;		/// ��ƴ���� �ݰݿ� �ϵ� �ڵ� �ڵ�
};

//////////////////////////////////////////////////////////////////////////

class CX2SkillTreeSlotData : public CKTDGUISlotData
{

public:

	enum SKILL_TREE_SLOT_ADD_PICTURE_TYPE
	{
		STSAPT_DISABLE = 0,		/// ��ο� ���
		STSAPT_EQUIPPED,		/// ������ 
		STSAPT_SEALED,			/// ���� ��ũ
		STSAPT_LEVEL,			/// ��ų ����
		STSAPT_CANLEARN,		/// ���� ����
		STSAPT_SELECT,			/// ���� ����

		STSAPT_END,
	};

	CX2SkillTreeSlotData();
	virtual ~CX2SkillTreeSlotData();

	virtual void UpdateUI( CKTDGUISlot* pSlot );
	void ShowSlotPicture(bool bShow, CKTDGUISlot* pSlot, SKILL_TREE_SLOT_ADD_PICTURE_TYPE eType, int val = -1 );

	//CX2SkillTree::SkillTreeTemplet*	pSkillTemplet;
	CX2SkillTree::SKILL_ID m_eSkillID;
};

class CX2EquippedSkillSlotData : public CKTDGUISlotData
{
	enum SKILL_EQUIPPED_SLOT_ADD_PICTURE_TYPE
	{
		SESAPT_DISABLE = 0,
		SESAPT_LEVEL,

		STSAPT_END,
	};

public:
	CX2EquippedSkillSlotData();
	CX2EquippedSkillSlotData(int index, bool bSlotb );
	virtual ~CX2EquippedSkillSlotData();

	int GetSlotID();		// 0~8�� ���Ǵ� ID�� �����ش�
	void ShowSlotPicture(bool bShow, CKTDGUISlot* pSlot, SKILL_EQUIPPED_SLOT_ADD_PICTURE_TYPE eType, int val = -1, bool bIsSkillLevelUpByItem = false );

	virtual void UpdateUI( CKTDGUISlot* pSlot );
	CX2SkillTree::SKILL_ID m_eSkillID;
	int	m_iIndex;
	bool m_bSlotb;



};

#endif // UPGRADE_SKILL_SYSTEM_2013