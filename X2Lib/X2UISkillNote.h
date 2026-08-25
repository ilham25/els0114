#ifdef SERV_SKILL_NOTE

#pragma once

class CX2UISkillNote
{
public:
	enum UI_SKILL_NOTE_MSG
	{
		USNM_EXIT = 39000,		
		USNM_REGISTER_VIEW_SLOT,
		USNM_SLOT_PAGE_LEFT,
		USNM_SLOT_PAGE_RIGHT,
		USNM_REGISTER_OK,
		USNM_REGISTER_CANCEL,
		USNM_PAGE_PREV,
		USNM_PAGE_NEXT,
	};

public:

	CX2UISkillNote( CKTDXStage* pNowStage, const WCHAR* pFileName );
	virtual ~CX2UISkillNote(void);

	virtual HRESULT OnFrameMove( double fTime, float fElapsedTime );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	virtual bool UICustomEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual bool UIServerEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	void SetShow(bool val);
	bool GetShow(){ return m_bShow; }
	
	bool OpenDialog();

	void SetLayer(X2_DIALOG_LAYER layer);

	void RegisterMemo(UidType iMemoId);
	void SetRegisterMemo(bool bVal) { m_bRegisterMemo = bVal; }
	bool GetRegisterMemo() { return m_bRegisterMemo; }
    
	void UpdateMemoSlot(int iPage = 0);
	void UpdateMemo(bool bVal, char cPage);	

	char GetUpdatePage() { return m_iUpdatePage; }
	bool GetUpdateMemo() { return m_bUpdateMemo; }
	void ResetSkillNote();

	float GetWriteMemoTime() { return m_fWriteMemoTime; }
private:	
	void ViewMemo(int iPage);
	void Handler_EGS_REG_SKILL_NOTE_MEMO_REQ(char iPage);	
	void CheckRegisterMemo();
	void UpdateMemoEffect();	
    wstring ReplaceDesc_(const wchar_t* pwstrOri, const wchar_t* pwstrPattern, const wchar_t* pwstrReplace);

private:
	char					m_iRegisterPage;
	char					m_iCurrentPage;	
	char					m_iUpdatePage;
	bool					m_bShow;
	bool					m_bRegisterMemo;
	bool					m_bUpdateMemo;	
	UidType					m_iRegisterMemo;
	int						m_iViewPage;

	CKTDGUIDialogType		m_pDLGSkillNote;
	CKTDGUIDialogType		m_pDLGWrittingNote;
	CKTDGUIDialogType		m_pDLGQuery;
	CKTDGUIDialogType		m_pDLGInfoBox;

	std::vector<wstring>	m_vecSlotLv;

	float					m_fWriteMemoTime;

	CKTDGParticleSystem::CParticleEventSequenceHandle m_hParticle1;	
	CKTDGParticleSystem::CParticleEventSequenceHandle m_hParticle2;	
};

#endif
