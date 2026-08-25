#pragma once

class CKTDGUIButton : public CKTDGUIControl
{
	public:
		enum BUTTON_CONTROL_STATE
		{
			BCS_NORMAL,
			BCS_MOUSEOVER,
			BCS_MOUSEDOWN,
			BCS_DISABLE,

			BCS_END,
		};

		enum BUTTON_EVENT_MSG
		{
			BEM_BUTTON_MOUSEOVER = 0,
			BEM_BUTTON_MOUSEDOWN,
			BEM_BUTTON_MOUSEUP,
		};

		
	public:
		CKTDGUIButton();
		virtual	~CKTDGUIButton(void);

		virtual	HRESULT	OnFrameMove( double fTime, float fElapsedTime );
		virtual	HRESULT	OnFrameRender();

		virtual void	SetShow( bool bShow );
		virtual void	SetEnable( bool bEnable );

		virtual bool	HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam );
		virtual	bool	HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam );
		virtual void	ChangeState( BUTTON_CONTROL_STATE state, bool bForce = false );

		virtual bool	SetName( const WCHAR* pName );

		virtual	bool	ContainsPoint( POINT pt );

		virtual void	MoveControl( float fx, float fy );
		virtual void	ScaleControl( float fx, float fy );


		bool SetNormalTex( const WCHAR* pFileName, const WCHAR* key );
		void SetOverTex( const WCHAR* pFileName, const WCHAR* key );
		void SetDownTex( const WCHAR* pFileName, const WCHAR* key );
	#ifdef UPGRADE_SKILL_SYSTEM_2013 // 김태환 - 스킬 시스템 변경
		void SetDisableTex( const WCHAR* pFileName, const WCHAR* key );
	#endif // UPGRADE_SKILL_SYSTEM_2013

		void SetNormalAnimTex( CKTDGUIControl::TexChangeData* pTexChangeData );
		void SetOverAnimTex( CKTDGUIControl::TexChangeData* pTexChangeData );
		void SetDownAnimTex( CKTDGUIControl::TexChangeData* pTexChangeData );
		
		void SetNormalTex_LUA( const char* pFileName, const char* key );
		void SetOverTex_LUA( const char* pFileName, const char* key );
		void SetDownTex_LUA( const char* pFileName, const char* key );
		void SetDisableTex_LUA( const char* pFileName, const char* key );

		void SetNormalAnimTex_LUA();
		void SetOverAnimTex_LUA();
		void SetDownAnimTex_LUA();
		
		void SetNormalStatic_LUA( CKTDGUIStatic* pStatic );
		void SetOverStatic_LUA( CKTDGUIStatic* pStatic );
		void SetDownStatic_LUA( CKTDGUIStatic* pStatic );
		void SetDisableStatic_LUA( CKTDGUIStatic* pStatic );

		CKTDGUIStatic* GetNormalStatic();

		void SetNormalPoint_LUA();
		void SetOverPoint_LUA();
		void SetDownPoint_LUA();
		void SetDisablePoint_LUA();

		void SetString_LUA();

		void SetCustomMsgMouseOut( int msg ) { m_CustomMsgMouseOut = msg; }
		void SetCustomMsgMouseOver( int msg ){ m_CustomMsgMouseOver = msg; }
		void SetCustomMsgMouseDown( int msg ){ m_CustomMsgMouseDown = msg; }
		void SetCustomMsgMouseUp( int msg ){ m_CustomMsgMouseUp = msg; }
		int GetCustomMsgMouseUp() { return m_CustomMsgMouseUp; }
		void SetCustomMsgMouseRightUp( int msg ){ m_CustomMsgMouseRightUp = msg; }
		void SetCustomMsgMouseDblClk( int msg ){ m_CustomMsgMouseDblClk = msg; }

		void SetCustomFuncMouseOver( const char* pFuncName ) { 	ConvertUtf8ToWCHAR( m_CustomFuncMouseOver, pFuncName ); }
		void SetCustomFuncMouseDown( const char* pFuncName ) { 	ConvertUtf8ToWCHAR( m_CustomFuncMouseDown, pFuncName ); }
		void SetCustomFuncMouseUp( const char* pFuncName ) { ConvertUtf8ToWCHAR( m_CustomFuncMouseUp, pFuncName ); }


		void SetNormalPoint( CKTDGUIControl::UIPointData* pUIPointData );
		void SetOverPoint( CKTDGUIControl::UIPointData* pUIPointData );
		void SetDownPoint( CKTDGUIControl::UIPointData* pUIPointData );
#ifdef UPGRADE_SKILL_SYSTEM_2013 // 김태환 - 스킬 시스템 변경
		void SetDisablePoint( CKTDGUIControl::UIPointData* pUIPointData );
#endif // UPGRADE_SKILL_SYSTEM_2013		



		D3DXVECTOR2 GetPos();
		int GetWidth();
		int GetHeight();

		void SetOffsetPos( D3DXVECTOR2 pos );

		BUTTON_CONTROL_STATE GetButtonControlState() { return m_ButtonState; }

		const CKTDGUIControl::UITextureData* GetNormalTexData()    const { return m_pNormalTex; }
		const CKTDGUIControl::UITextureData* GetMouseOverTexData() const { return m_pMouseOverTex; }
		const CKTDGUIControl::UITextureData* GetMouseDownTexData() const { return m_pMouseDownTex; }

		CKTDGUIControl::UIPointData* GetNormalPoint()    const { return m_pNormalPoint; }
		CKTDGUIControl::UIPointData* GetMouseOverPoint() const { return m_pMouseOverPoint; }
		CKTDGUIControl::UIPointData* GetMouseDownPoint() const { return m_pMouseDownPoint; }

		void SetOverStateAtNormal( bool bCheck ) { m_bOverStateAtNormal = bCheck; }
		void SetDownStateAtNormal( bool bCheck );

		bool GetEnableClick() const { return m_bEnableClick; }
		void SetEnableClick( bool bEnable ) { m_bEnableClick = bEnable; } 

		// Maximize / Minimize
		virtual void SetMaximize( bool val );

		D3DXVECTOR2		GetGuideDescPos();
		void SetEdge( bool bDrawOut, int _width, D3DXCOLOR _color );
		void SetEdge2( bool bDrawOut, int _width, D3DXCOLOR _color, int enlargeValue );

		void ChangeMouseUpSound( const WCHAR* wszFileName );
		void ChangeMouseUpSound_LUA( const char* szFileName );

		void ChangeMouseDownSound( const WCHAR* wszFileName );
		void ChangeMouseDownSound_LUA( const char* szFileName );

		void ChangeMouseOverSound( const WCHAR* wszFileName );
		void ChangeMouseOverSound_LUA( const char* szFileName );

	protected:

		void ReNameStatics();
		void DrawEdge( bool bDrawOut );

		void LButtonMouseUp();

		void TexChange();

		BUTTON_CONTROL_STATE				m_ButtonState;

		CKTDGUIControl::UITextureData*		m_pNormalTex;
		//map< int, CKTDGUIControl::UITextureData* > m_mapMouseOverTex;
		CKTDGUIControl::UITextureData*		m_pMouseOverTex;
		CKTDGUIControl::UITextureData*		m_pMouseDownTex;
		CKTDGUIControl::UITextureData*		m_pDisableTex;

		CKTDGUIControl::UIPointData*		m_pNormalPoint;
		CKTDGUIControl::UIPointData*		m_pMouseOverPoint;
		CKTDGUIControl::UIPointData*		m_pMouseDownPoint;
		CKTDGUIControl::UIPointData*		m_pDisablePoint;

		CKTDGUIControl::UIStringData*		m_pStringData;

		int									m_CustomMsgMouseOut;
		int									m_CustomMsgMouseOver;
		int									m_CustomMsgMouseDown;
		int									m_CustomMsgMouseUp;
		int									m_CustomMsgMouseRightUp;
		int									m_CustomMsgMouseDblClk;

		wstring								m_CustomFuncMouseOver;
		wstring								m_CustomFuncMouseDown;
		wstring								m_CustomFuncMouseUp;

		CKTDXDeviceSound*					m_pSndMouseOver;
		CKTDXDeviceSound*					m_pSndMouseUp;
		CKTDXDeviceSound*					m_pSndMouseDown;

		bool								m_bIsSameNormalAndOverButton;

		CKTDGUIControl::VERTEX_UI			m_Vertex[4];

		bool								m_bOverStateAtNormal;
		bool								m_bDownStateAtNormal;

		bool								m_bEnableClick;

		bool								m_bClicked;

		bool								m_bMouseOut;	// 마우스가 over된 상태에서 normal 상태로 바뀌는지

		CKTDGUIStatic*						m_StaticControls[BCS_END];

		D3DXCOLOR							m_EdgeColor;
		int									m_EdgeWidth;
		int									m_EdgeEnlargeValue;
		CKTDXDeviceTexture*					m_pEdgeTexture;
		bool								m_bDrawEdgeOut; //바깥쪽에 그릴것인가 안쪽에 그릴것인가?

		map< int, CKTDGUIControl::TexChangeData* >	m_mapMouseOverTex;
		map< int, CKTDGUIControl::TexChangeData* >	m_mapMouseNormalTex;
		map< int, CKTDGUIControl::TexChangeData* >	m_mapMouseDownTex;
};