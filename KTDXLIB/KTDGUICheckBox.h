#pragma once

class CKTDGUICheckBox : public CKTDGUIControl
{
	public:
		enum CHECKBOX_CONTROL_STATE
		{
			CBCS_NORMAL,
			CBCS_MOUSEOVER,
			CBCS_DISABLE,
		};

		enum CHECKBOX_EVENT_MSG
		{
			CBEM_CHECKBOX_CHECKED	= 0,
			CBEM_CHECKBOX_UNCHECKED,
		};

	public:
		CKTDGUICheckBox(void);
		virtual	~CKTDGUICheckBox(void);

		virtual	HRESULT	OnFrameMove( double fTime, float fElapsedTime );
		virtual	HRESULT	OnFrameRender();

		virtual	bool	HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam );
		virtual	void	ChangeState( CHECKBOX_CONTROL_STATE state );

		void	SetNormalTex_LUA( const char* pFileName, const char* key );
		void	SetOverTex_LUA( const char* pFileName, const char* key );
		void	SetDisableTex_LUA( const char* pFileName, const char* key );
		void	SetCheckedTex_LUA( const char* pFileName, const char* key );

		void	SetBGPoint_LUA();
		void	SetCheckPoint_LUA();

		void	SetCustomMsgChecked( int msg )	{ m_CustomMsgChecked	= msg; }
		void	SetCustomMsgUnChecked( int msg ){ m_CustomMsgUnChecked	= msg; }

		bool	GetChecked()				{ return m_bChecked; }
		void	SetChecked( bool bChecked, bool bFromInput = false )	{ SetCheckedInternal( bChecked, bFromInput ); }
		void	SetCheckedPure( bool bChecked ) { m_bChecked = bChecked; }

		void	SetEmptyBGWhenChecked( bool bVal ) { m_bEmptyBGWhenChecked = bVal; }

		const CKTDGUIControl::UITextureData* GetNormalTexData()    const { return m_pNormalTex; }
		const CKTDGUIControl::UITextureData* GetMouseOverTexData() const { return m_pMouseOverTex; }
		const CKTDGUIControl::UITextureData* GetCheckedTexData() const { return m_pCheckedTex; }

		const CKTDGUIControl::UIPointData* GetBGPoint()    const { return m_pBGPoint; }
		const CKTDGUIControl::UIPointData* GetCheckPoint() const { return m_pCheckedPoint; }

	protected:
		void	SetCheckedInternal( bool bChecked, bool bFromInput = false );

		bool							m_bChecked;
		bool							m_bPressed;
		bool							m_bEmptyBGWhenChecked;	// 체크된 상태에서 배경 텍스쳐를 그릴지
		CHECKBOX_CONTROL_STATE			m_CheckBoxState;

		CKTDGUIControl::UITextureData*	m_pNormalTex;
		CKTDGUIControl::UITextureData*	m_pMouseOverTex;
		CKTDGUIControl::UITextureData*	m_pDisableTex;
		CKTDGUIControl::UITextureData*	m_pCheckedTex;

		CKTDGUIControl::UIPointData*	m_pBGPoint;
		CKTDGUIControl::UIPointData*	m_pCheckedPoint;

		CKTDGUIControl::VERTEX_UI		m_Vertex[4];

		CKTDXDeviceSound*				m_pSndCheck;

		int								m_CustomMsgChecked;
		int								m_CustomMsgUnChecked;

		wstring							m_CustomFuncChecked;
		wstring							m_CustomFuncUnChecked;

		
};
