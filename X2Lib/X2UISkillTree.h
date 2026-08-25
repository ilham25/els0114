#ifndef UPGRADE_SKILL_UI

// X2UISkillTree.h : �Ÿ��� �� ��ųƮ�� UI
// ���� ��ǥ - ��� Data�� X2UserSkillTree�� X2SkillTree���� ������ �� : UI Class�� UI ���Ҹ� �Ѵ�!

#ifdef NEW_SKILL_TREE_UI

#pragma once


class CX2EquippedSkillSlotData;
class CX2SkillTreeSlotData;

class CX2UISkillTree
{
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
	};

public:

	struct SkillSlotUI
	{
		CKTDGUIStatic*	m_pStaticBackGround;
		CKTDGUISlot*	m_pSlot;
		CKTDGUIButton*	m_pButtonPlus;
		CKTDGUIButton*	m_pButtonMinus;
		CKTDGUIButton*	m_pButtonMaster;
		CKTDGUIStatic*	m_pStaticLevelString;
		CKTDGUIStatic*	m_pStaticActiveMark;
		CKTDGUIStatic*	m_pStaticLock;				// ���ν�ų ǥ�� �ڹ���
#ifdef GUIDE_SKILL_LEVEL_MINUS
		CKTDGUIButton*	m_pButtonMinusGuide;
#endif GUIDE_SKILL_LEVEL_MINUS

#ifdef ADDED_RELATIONSHIP_SYSTEM
		CKTDGUIStatic*	m_pStaticRelationType;
#endif // ADDED_RELATIONSHIP_SYSTEM

		SkillSlotUI()
		{
			m_pStaticBackGround = NULL;
			m_pSlot = NULL;
			m_pButtonPlus = NULL;
			m_pButtonMinus = NULL;
			m_pButtonMaster = NULL;
			m_pStaticLevelString = NULL;
			m_pStaticActiveMark = NULL;
			m_pStaticLock = NULL;
#ifdef GUIDE_SKILL_LEVEL_MINUS
			m_pButtonMinusGuide = NULL;
#endif GUIDE_SKILL_LEVEL_MINUS
#ifdef ADDED_RELATIONSHIP_SYSTEM
			m_pStaticRelationType = NULL;
#endif //ADDED_RELATIONSHIP_SYSTEM

		}
		// �Ҹ��ڴ� �ʿ���� : ���� Dialog �Ҹ��ڿ��� �����
	};

	struct ArrowUI
	{
		CKTDGUIStatic*			m_pStaticArrowEnd;
		vector<CKTDGUIStatic*>	m_vpStaticArrowLine;
		CKTDGUIStatic*			m_pStaticArrowHead;

		ArrowUI()
		{
			m_pStaticArrowEnd = NULL;
			m_pStaticArrowHead = NULL;
		}
		// �Ҹ��ڴ� �ʿ���� : ���� Dialog �Ҹ��ڿ��� �����
	};

public:

	CX2UISkillTree( CKTDXStage* pNowStage );
	virtual ~CX2UISkillTree(void);

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
	void UpdateSkillTree( CX2SkillTree::SKILL_ID eSkillID );		// Tree���� �ش� Slot�� ������Ʈ�Ѵ�
	void UpdateBlind();
	void UpdateNavi();

	void UpdateUI( bool bEquipped, bool bSP = true, bool bSkillTree = false );
    
	void CreateSlotUIPreset(SkillSlotUI& UISet, int eSkillID );
	void CreateVerticalArrowUIPreset( ArrowUI& UISet, int iLineLength, int eSkillID );	// iLineLength : Head�� Tail�� ������ ����.
	void CreateHorizonalArrowUIPreset( ArrowUI& UISet, int iLineLength, int eSkillID, bool bRight );

	const map<CX2SkillTree::SKILL_ID, SkillSlotUI>& GetMapSkillSlotUISet(){ return m_mapSkillSlotUISet; }

	//SkillSlotUI& GetSkillSlotUI( CX2SkillTree::SKILL_ID eSkillID );
	//ArrowUI& GetArrowUI( CX2SkillTree::SKILL_ID eSkillID );
	void SetArrowColor( CX2SkillTree::SKILL_ID eSkillID, D3DXCOLOR cCol );

	bool GetRemoveMode(){ return m_bSkillRemoveMode; }
	void InitEquippedSkillSlotData();

	void UpdateSkillCoolTime( float fElapsedTime );
	//void UpdateSkillSlotPosition();
	bool GetSkillSlotSwap()const {return m_bSkillSlotSwap;}
	void SwapSkillSlot(bool bVal, bool bForce = false);
	void SetOffsetPosSkillSLotStatic(const D3DXVECTOR2& vPos_, const D3DXVECTOR2& vAddPos_ = D3DXVECTOR2(0,0));
	void SetOffsetStatic( const WCHAR* strControl_, const D3DXVECTOR2& vPos_);
	void SetShowEnableStatic( const WCHAR* strControl_, bool bShow_, bool bEnable_);

	//////////////////////////////////////////////////////////////////////////

	//** fix!! ��ų �������� ���̾�α׸� ���� �Լ�. �߰� �۾� �ʿ��մϴ�.
	void OpenSkillUnsealInformDLG( const CX2SkillTree::SkillTemplet* pSkillTemplet, D3DXVECTOR2 SlotCenterPos );	

	void OpenDLGSkillUnsealed( bool bOpen );
	void UpdateDLGSkillUnsealed( CX2SkillTree::SKILL_ID eSkillID );


	
	//////////////////////////////////////////////////////////////////////////

	void ShowSkillDesc( bool bShow, CX2SkillTree::SKILL_ID eSkillID = CX2SkillTree::SI_NONE, D3DXVECTOR2 pos = D3DXVECTOR2(0,0), 
						D3DXVECTOR2 size = D3DXVECTOR2(0,0), bool bEquipped = false );
	wstring GetSkillDesc( CX2SkillTree::SKILL_ID eSkillID, bool bEquipped = false );
	void ShowUIDesc( bool bShow, wstring wstr = L"", D3DXVECTOR2 pos = D3DXVECTOR2(0,0), 
					 CX2TalkBoxManagerImp::TALK_BOX_TYPE TBT = CX2TalkBoxManagerImp::TBT_FROM_UP_LEFT,
					 D3DXCOLOR coTextColor = D3DXCOLOR(0,0,0,1), D3DXCOLOR coBackColor = D3DXCOLOR(1, 0.96f, 0.6f, 1 ) );
	//wstring GetExpirationDateDesc( const wstring& wstrExpirationDate );	// kimhc // KTDXCommonFunc�� �ű� // 2009-06-16
	
	//////////////////////////////////////////////////////////////////////////

	void CreateMiniSkillTree( CX2Unit::UNIT_CLASS eUnitClass );
	void SetMiniSkillSlotColor( CX2SkillTree::SKILL_ID eSkillID, D3DXCOLOR cCol );

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

private:
#ifdef ELSWORD_WAY_OF_SWORD
	void					DrawElswordWSSDesc( D3DXVECTOR2 vDrawPos_ );
#endif ELSWORD_WAY_OF_SWORD

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
	

	map<CX2SkillTree::SKILL_ID, SkillSlotUI>	m_mapSkillSlotUISet;
	std::multimap<CX2SkillTree::SKILL_ID, ArrowUI>		m_mapArrowUISet;			// ȭ��ǥ�� ���۵Ǵ� ��ų(���� ��ų) ID�� key�� ��´�. �� ��ų�� ���� ��ų�� ������ �� �����Ƿ� ��Ƽ��.

	map<CX2SkillTree::SKILL_ID, int>	m_mapMiniSkillSlotPictureIndex;
	std::multimap<CX2SkillTree::SKILL_ID, int>		m_mapMiniArrowPictureIndex;			// ȭ��ǥ�� ���۵Ǵ� ��ų(���� ��ų) ID�� key�� ��´�. �� ��ų�� ���� ��ų�� ������ �� �����Ƿ� ��Ƽ��.

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

	bool					m_bSkillSlotSwap;			//��ų ���� ���� �õ�

#ifdef SKILL_SLOT_UI_TYPE_B
	bool					m_bIsSkillUITypeA;			// ��ų ���� Ÿ��
#endif //SKILL_SLOT_UI_TYPE_B

	
};

//////////////////////////////////////////////////////////////////////////

class CX2SkillTreeSlotData : public CKTDGUISlotData
{

public:

	enum SKILL_TREE_SLOT_ADD_PICTURE_TYPE
	{
		STSAPT_DISABLE = 0,
		STSAPT_EQUIPPED,
		STSAPT_SEALED,
		STSAPT_LEVEL,
		STSAPT_CANLEARN,

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


#endif NEW_SKILL_TREE_UI

#endif UPGRADE_SKILL_UI