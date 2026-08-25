#pragma once

#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN	0x020B // (not always defined)
#endif
#ifndef WM_XBUTTONUP
#define WM_XBUTTONUP	0x020C // (not always defined)
#endif
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL	0x020A // (not always defined)
#endif
#ifndef WHEEL_DELTA
#define WHEEL_DELTA		120 // (not always defined)
#endif


class CKTDGUIDialog;
//typedef CKTDGUIDialogHandle     CKTDGUIDialogType;

#define D3DFVF_VERTEX_UI   ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 )

class CKTDGUIControl : private boost::noncopyable
{
	public:
		struct VERTEX_UI
		{
			float		x, y, z, rhw;
			D3DCOLOR	color;
			float		u, v;

			VERTEX_UI() : x( 0.f ), y( 0.f ), z( 0.f ), rhw( 0.f ), color( 0 ), u( 0.f ), v( 0.f ) {}
			VERTEX_UI( float x_, float y_, float z_, float rhw_, D3DCOLOR color_, float u_, float v_ )
			{
				x = x_; y = y_; z = z_; rhw = rhw_;
				color = color_;
				u = u_; v = v_;
			}
			
			bool    operator == ( const VERTEX_UI& rhs_ ) const
            {
                return  x == rhs_.x && y == rhs_.y && z == rhs_.z && rhw == rhs_.rhw && color == rhs_.color && u == rhs_.u && v == rhs_.v;
            }
            bool    operator != ( const VERTEX_UI& rhs_ ) const
            {
                return  !( operator == ( rhs_ ) );
            }
		};

		enum VERTEX_POSITION
		{
			VP_LEFT_TOP		= 0,
			VP_RIGHT_TOP	= 1,
			VP_LEFT_BOTTOM	= 2,
			VP_RIGHT_BOTTOM	= 3,
		};
		
		enum UI_CONTROL_TYPE
		{
			UCT_NONE		= 0,
			UCT_BUTTON,
			UCT_CHECKBOX,
			UCT_EDITBOX,
			UCT_IME_EDITBOX,
			UCT_STATIC,
			UCT_RADIOBUTTON,
			UCT_SCROLLBAR,
			UCT_COMBOBOX,
			UCT_LISTBOX,
			UCT_SLIDER,
			UCT_NAVIGATION,
#ifdef NEW_SKILL_TREE_UI
			UCT_CONTROLLIST,
			UCT_SLOT,
#endif
		};

		struct UITextureData
		{
			wstring					texName;
			wstring					keyName;
			CKTDXDeviceTexture*		pTexture;
			D3DXVECTOR2				uvOrgTexture[4];
			D3DXVECTOR2				uvTexture[4];

			D3DXVECTOR2				texSize;

			void Init()
			{
				pTexture = NULL;
				for( int i = 0; i < 4; i++ )
				{
					INIT_VECTOR2( uvTexture[i], 0.0f, 0.0f );
					INIT_VECTOR2( uvOrgTexture[i], 0.0f, 0.0f );
				}

				texSize = D3DXVECTOR2( 0, 0 );
			}

			void SetTextureUV()
			{
				if( pTexture == NULL )
					return;

				uvTexture[VP_LEFT_TOP].x		= uvOrgTexture[VP_LEFT_TOP].x		/ pTexture->GetWidth();
				uvTexture[VP_LEFT_TOP].y		= uvOrgTexture[VP_LEFT_TOP].y		/ pTexture->GetHeight();
				uvTexture[VP_RIGHT_TOP].x		= uvOrgTexture[VP_RIGHT_TOP].x		/ pTexture->GetWidth();
				uvTexture[VP_RIGHT_TOP].y		= uvOrgTexture[VP_RIGHT_TOP].y		/ pTexture->GetHeight();
				uvTexture[VP_RIGHT_BOTTOM].x	= uvOrgTexture[VP_RIGHT_BOTTOM].x	/ pTexture->GetWidth();
				uvTexture[VP_RIGHT_BOTTOM].y	= uvOrgTexture[VP_RIGHT_BOTTOM].y	/ pTexture->GetHeight();
				uvTexture[VP_LEFT_BOTTOM].x		= uvOrgTexture[VP_LEFT_BOTTOM].x	/ pTexture->GetWidth();
				uvTexture[VP_LEFT_BOTTOM].y		= uvOrgTexture[VP_LEFT_BOTTOM].y	/ pTexture->GetHeight();
			}

			UITextureData()
			{
				Init();
			}

			UITextureData( const WCHAR* pwszTexName_, const WCHAR* pwszKeyName_ )
				: texName( pwszTexName_ ), keyName( pwszKeyName_ )
			{
				Init();
			}

			~UITextureData();
		};

		struct UIPointData
		{
			D3DXVECTOR2		leftTopPoint;
			D3DXVECTOR2		rightTopPoint;
			D3DXVECTOR2		leftBottomPoint;
			D3DXVECTOR2		rightBottomPoint;

			D3DXCOLOR		color;
			float			fChangeTime;

			UITextureData*	pUITextureData;

			bool			bUseTextureSize;

			D3DXVECTOR2		addSize;
#ifdef RESIZE_TEXTURE
			D3DXVECTOR2		kResize;
#endif RESIZE_TEXTURE

			
			int				m_iDrawCount;
			D3DBLEND		m_eSrcBlend;
			D3DBLEND		m_eDestBlend;
			
			UIPointData()
			{
				INIT_VECTOR2( leftTopPoint,		0.0f, 0.0f );
				INIT_VECTOR2( rightTopPoint,	0.0f, 0.0f );
				INIT_VECTOR2( leftBottomPoint,	0.0f, 0.0f );
				INIT_VECTOR2( rightBottomPoint, 0.0f, 0.0f );

				color			= 0xffffffff;
				fChangeTime		= 0.0f;
				pUITextureData	= NULL;

				bUseTextureSize = true;

				addSize = D3DXVECTOR2( 0, 0 );
#ifdef RESIZE_TEXTURE
				kResize = D3DXVECTOR2( 1, 1 );
#endif RESIZE_TEXTURE

				m_iDrawCount			= 1;
				m_eSrcBlend				= D3DBLEND_SRCALPHA;
				m_eDestBlend			= D3DBLEND_INVSRCALPHA;
			}

			bool SetAutoPointByTextureSize()
			{
				if ( bUseTextureSize == false )
					return false;

				if ( pUITextureData != NULL )
				{
					if ( pUITextureData->pTexture != NULL )
					{
						float width = pUITextureData->texSize.x;
						float height = pUITextureData->texSize.y;
#ifdef RESIZE_TEXTURE
						width *= kResize.x;
						height *= kResize.y;
#endif RESIZE_TEXTURE

						rightTopPoint.x = leftTopPoint.x + width + addSize.x;
						rightTopPoint.y = leftTopPoint.y;

						leftBottomPoint.x = leftTopPoint.x;
						leftBottomPoint.y = leftTopPoint.y + height + addSize.y;

						rightBottomPoint.x = leftTopPoint.x + width + addSize.x;
						rightBottomPoint.y = leftTopPoint.y + height + addSize.y;

						return true;
					}
				}

				return false;
			}

			// 4개의 꼭지점이 정해진 상태에서 해야~
			void Rotate( D3DXVECTOR2 vCenter, float fDegree )
			{
				float fRad = D3DXToRadian( fDegree );
				float fCos = cosf( fRad );
				float fSin = sinf( fRad );

				D3DXVECTOR2 vTemp;
				vTemp = leftTopPoint - vCenter;
				leftTopPoint.x = vTemp.x * fCos - vTemp.y * fSin + vCenter.x;
				leftTopPoint.y = vTemp.x * fSin + vTemp.y * fCos + vCenter.y;
				vTemp = rightTopPoint - vCenter;
				rightTopPoint.x = vTemp.x * fCos - vTemp.y * fSin + vCenter.x;
				rightTopPoint.y = vTemp.x * fSin + vTemp.y * fCos + vCenter.y;
				vTemp = leftBottomPoint - vCenter;
				leftBottomPoint.x = vTemp.x * fCos - vTemp.y * fSin + vCenter.x;
				leftBottomPoint.y = vTemp.x * fSin + vTemp.y * fCos + vCenter.y;
				vTemp = rightBottomPoint - vCenter;
				rightBottomPoint.x = vTemp.x * fCos - vTemp.y * fSin + vCenter.x;
				rightBottomPoint.y = vTemp.x * fSin + vTemp.y * fCos + vCenter.y;
			}



			void Scale( float fx, float fy )
			{
				leftTopPoint.x        *= fx;
				rightTopPoint.x       *= fx;
				leftBottomPoint.x     *= fx;
				rightBottomPoint.x    *= fx;

				leftTopPoint.y        *= fy;
				rightTopPoint.y       *= fy;
				leftBottomPoint.y     *= fy;
				rightBottomPoint.y    *= fy;
			}


			void Move( float fx, float fy )
			{
				leftTopPoint.x        += fx;
				rightTopPoint.x       += fx;
				leftBottomPoint.x     += fx;
				rightBottomPoint.x    += fx;

				leftTopPoint.y        += fy;
				rightTopPoint.y       += fy;
				leftBottomPoint.y     += fy;
				rightBottomPoint.y    += fy;
			}
		};

		struct UIStringData
		{
			int								fontIndex;
			wstring							msg;
			D3DXVECTOR2						pos;
			D3DXCOLOR						color;
			D3DXCOLOR						outlineColor;
			DWORD							sortFlag; 
			CKTDGFontManager::FONT_STYLE	fontStyle;
			bool							bUseUkFont;

			bool							bSpread;
			float							fSpreadTimeGap;
			
			int								iSpreadCount;			
			float							fNowSpreadTime;			


			UIStringData()
			{
				bUseUkFont = true;
				bSpread = false;
				iSpreadCount = 0;
				fNowSpreadTime = 0.f;

				fontIndex = 0;
				sortFlag = 0;
				fontStyle = CKTDGFontManager::FS_NONE;
				fSpreadTimeGap = 0.f;
			}

			void SetMsg_LUA( const char* pMsg ) { ConvertUtf8ToWCHAR( msg, pMsg ); }
			void SetSpreadCount(int val) { iSpreadCount = val; }

			void OnFrameMove( double fTime, float fElapsedTime )
			{
				KTDXPROFILE();

				if( true == bSpread )
				{
					if( iSpreadCount < (int) msg.length() )
					{
						fNowSpreadTime += fElapsedTime;
						if( fNowSpreadTime > fSpreadTimeGap )
						{
							fNowSpreadTime = 0.f;
							iSpreadCount++;
							if( iSpreadCount > 0 )
							{
								if( 0 == msg.find( L"#CX", iSpreadCount-1 ) )
								{
									iSpreadCount += 2;
								}
								else if( 0 == msg.find( L"#C", iSpreadCount-1 ) )
								{
									iSpreadCount += 7;
								}
							}
						}
					}
					else
					{
						bSpread = false;
					}
				}
			}
		};

		class CPictureData
		{
			public:
				struct VIBRATE_DATA
				{
					float 		m_fMaxTime;
					float 		m_fDistance;
					D3DXVECTOR2 m_vDirSpeed;

					float	m_fElapsedTime;
					bool	m_bGoingAway;
					D3DXVECTOR2 m_vOriginalPos;

				public:
					VIBRATE_DATA()
					{
						m_fMaxTime = 0.f;
						m_fDistance = 0.f;
						m_vDirSpeed = D3DXVECTOR2(0, 0 );
						
						m_fElapsedTime = 0.f;
						m_bGoingAway = true;
						m_vOriginalPos = D3DXVECTOR2(0, 0 );

					}
				};

			public:
				CPictureData();
				~CPictureData();

				void OnFrameMove( double fTime, float fElapsedTime );

				void SetTex_LUA2( const char* pFileName );
				bool SetTex( const WCHAR* pFileName );
				void SetTex_LUA( const char* pFileName, const char* key );
				bool SetTex( const WCHAR* pFileName, const WCHAR* key );
				void SetTex( CKTDGUIControl::UITextureData* pNewTexture );
				
				void SetPoint_LUA();
				void SetPoint( CKTDGUIControl::UIPointData* pNewPoint );
				void SetPoint();

				D3DXVECTOR2 GetSize() { return size; }
				D3DXVECTOR2 GetOriginalPos() { return originalPos; }
				D3DXVECTOR2 GetOriginalSize() { return originalSize; }

				void SetSize( D3DXVECTOR2 _size );
				void SetSizeX( float x );
				void SetSizeY( float y );
				void SetSizeY( float y, bool isInverse );

				void SetPos( D3DXVECTOR2 _pos );
				D3DXVECTOR2 GetPos() { return pos; }

				void SetShow(bool _bShow) { bShow = _bShow; }
				bool GetShow() { return bShow; }

				void SetMaximizeType( bool bPos, bool bSize );

				void RotatePoint( D3DXVECTOR2 vCenter, float fDegree )
				{
					if( NULL != pPoint )
						pPoint->Rotate( vCenter, fDegree );
				}

				void SetColor( D3DXCOLOR vColor )
				{
					if( NULL != pPoint )
						pPoint->color = vColor;
				}
			
				D3DXCOLOR GetColor() const
				{
					if( NULL != pPoint )
						return pPoint->color;
					else
						return D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );
				}

				void SetTwinkle( float fTime, bool bLoop = false )
				{
					if( NULL == pPoint )
						return;

					if( true != m_bTwinkle )
					{
						m_fOriginalAlpha = pPoint->color.a;
					}
					
					m_bTwinkle = true;
					m_fTwinkleTime = fTime;
					m_fRemainTwinkleTime = m_fTwinkleTime;
					m_bLoop = bLoop;
				}
#ifdef NEW_SKILL_TREE
				// 09.06.02 태완 : 그림 천천히 깜빡이는 기능.
				void SetFlicker( float fTime, float MaxAlpha, float MinAlpha, bool bLoop = true )
				{
					if( NULL == pPoint )
						return;
					if(fTime == 0)
						return;

					m_bFlicker = true;
					m_fRemainFlickerTime = 0;
					m_fFlickerTime = fTime;
					m_fFlickerMaxAlpha = MaxAlpha;
					m_fFlickerMinAlpha = MinAlpha;
					m_bFlickerGainAlpha = true;
					m_bFlickerLoop = bLoop;
				}
#endif

#ifdef RIDING_SYSTEM
				const bool GetIsNowFlicker(){ return m_bFlicker; }
#endif //RIDING_SYSTEM

				void SetEdge( bool bDrawOut, int edgeWidth, D3DXCOLOR edgeColor )
				{
					m_bDrawEdgeOut = bDrawOut;
					m_EdgeWidth = edgeWidth;
					m_EdgeColor = edgeColor;
				}

				void Vibrate( float fTime, float fDistance, D3DXVECTOR2 vDirSpeed )
				{
					m_VibrateData.m_fMaxTime = fTime;
					m_VibrateData.m_fDistance = fDistance;
					m_VibrateData.m_vDirSpeed = vDirSpeed;

					m_VibrateData.m_vOriginalPos = pos;
				}
                void SetIndex(int inx) { m_nIndex = inx; }
                int  GetIndex() { return m_nIndex; }
				void SetSizeAsTextureSize();

#ifdef BUFF_ICON_UI
				void SetFadeIn( float fTime );
				void SetFadeOut( float fTime );
				void InitAction()
				{
					m_bTwinkle = false;
					m_bFlickerLoop = false;
					m_bFadeIn = false;
					m_bFadeOut = false;
				}
#endif //BUFF_ICON_UI

				CKTDGUIControl::UITextureData*	pTexture;
				CKTDGUIControl::UIPointData*	pPoint;
				bool							bShow;
				bool							bMaximizePos;
				bool							bMaximizeSize;

				D3DXVECTOR2						pos;
				D3DXVECTOR2						size;
				D3DXVECTOR2						originalPos;
				D3DXVECTOR2						originalSize;

				bool							m_bTwinkle;
				float							m_fOriginalAlpha;
				float							m_fTwinkleTime;
				float							m_fRemainTwinkleTime;
				bool							m_bLoop;
#ifdef NEW_SKILL_TREE
				// 09.06.02 태완 : Flicker 관련.
				bool							m_bFlicker;
				float							m_fFlickerTime;
				float							m_fFlickerMaxAlpha;
				float							m_fFlickerMinAlpha;
				float							m_fRemainFlickerTime;
				bool							m_bFlickerGainAlpha;
				bool							m_bFlickerLoop;
#endif

				D3DXCOLOR						m_EdgeColor;
				int								m_EdgeWidth;
				int								m_EdgeEnlarge;
				bool							m_bDrawEdgeOut; //바깥쪽에 그릴것인가 안쪽에 그릴것인가?

				VIBRATE_DATA					m_VibrateData;
                int                             m_nIndex;

#ifdef BUFF_ICON_UI
				bool							m_bFadeIn;
				bool							m_bFadeOut;
				float							m_fFadeTime;
				float							m_fRemainFadeTime;
#endif //BUFF_ICON_UI

		};

		enum TEX_AXIS
		{
			TA_NONE = 0,
			TA_TOP_LEFT,
			TA_TOP_CENTER,
			TA_TOP_RIGHT,
			TA_CENTER_LEFT,
			TA_CENTER_CENTER,
			TA_CENTER_RIGHT,
			TA_BOTTOM_LEFT,
			TA_BOTTOM_CENTER,
			TA_BOTTOM_RIGHT,
			TA_WORLD,
		};



		struct TexChangeData
		{
			int								m_SequenceID;
			float							m_fShowTime;
			UITextureData*					m_pTexData;
			TEX_AXIS						m_RotateAxis;
			D3DXVECTOR2						m_RotateAxisPos;
			float							m_fRotateDegree;
			D3DXVECTOR2						m_Pos;
			TEX_AXIS						m_ScaleAxis;
			D3DXVECTOR2						m_ScaleAxisPos;
			D3DXVECTOR2						m_Scale;
			float							m_fScaleRotationDegree;
			
			bool							m_bEndTexChange;
			int								m_SequenceIDToGo;

			TexChangeData()
			{
				Init();
			}
			
			void Init()
			{
				m_SequenceID = 0;
				m_fShowTime = 0.0f;
				m_pTexData = NULL;
				m_RotateAxis = TA_NONE;
				m_RotateAxisPos = D3DXVECTOR2(0,0);
				m_fRotateDegree = 0.0f;
				m_Pos = D3DXVECTOR2(0,0);
				m_Scale = D3DXVECTOR2(1,1);
				m_fScaleRotationDegree = 0.0f;
				m_ScaleAxis = TA_NONE;
				m_ScaleAxisPos = D3DXVECTOR2(0,0);
				m_bEndTexChange = false;
				m_SequenceIDToGo = -1;
			}

			void CopyExceptTexture( const TexChangeData& rhs )
			{
				if ( this == &rhs ) 
					return;

				m_SequenceID			= rhs.m_SequenceID;
				m_fShowTime				= rhs.m_fShowTime;
				m_pTexData				= NULL;
				m_RotateAxis			= rhs.m_RotateAxis;
				m_RotateAxisPos			= rhs.m_RotateAxisPos;
				m_fRotateDegree			= rhs.m_fRotateDegree;
				m_Pos					= rhs.m_Pos;
				m_Scale					= rhs.m_Scale;
				m_fScaleRotationDegree	= rhs.m_fScaleRotationDegree;
				m_ScaleAxis				= rhs.m_ScaleAxis;
				m_ScaleAxisPos			= rhs.m_ScaleAxisPos;
				m_bEndTexChange			= rhs.m_bEndTexChange;
				m_SequenceIDToGo		= rhs.m_SequenceIDToGo;
			}

			~TexChangeData()
			{
				SAFE_DELETE( m_pTexData );
			}
		};


		struct ControlMoveItem
		{
			D3DXVECTOR2 pos;
			D3DXCOLOR	color;
			float		fChangeTime;
			bool		bIsAutoDelete;

			ControlMoveItem()
			{
				INIT_VECTOR2(pos, 0, 0);
				color = D3DXCOLOR(0xffffffff);
				fChangeTime = 0.0f;
				bIsAutoDelete = false;
			}
			ControlMoveItem& operator=( const ControlMoveItem& data )
			{
				pos				= data.pos;
				color			= data.color;
				fChangeTime		= data.fChangeTime;
				bIsAutoDelete	= data.bIsAutoDelete;

				return *this;
			}

		};

	public:
		CKTDGUIControl();
		virtual	~CKTDGUIControl(void);

		virtual	HRESULT	OnFrameMove( double fTime, float fElapsedTime );
		virtual	HRESULT	OnFrameRender();
		virtual	HRESULT	OnFrameRenderEnd()  { return S_OK; }
		virtual	HRESULT	OnResetDevice()		{ return S_OK; }
		virtual	HRESULT	OnLostDevice()		{ return S_OK; }

		// Windows message handler
		virtual	bool	MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )		{ return false; }
		virtual	bool	HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam )			{ return false; }
		virtual	bool	HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam )	{ return false; }
				
		virtual void	OnFocusIn()		{ m_bHasFocus = true; }
		virtual void	OnFocusOut()	{ m_bHasFocus = false; }

		virtual void	RequestFocus();

		const bool		IsOnMouseOver() const { return m_bMouseOver; } 
		void			OnMouseEnter()	{ m_bMouseOver = true; }
		void			OnMouseLeave()	{ m_bMouseOver = false; }
		virtual	bool	ContainsPoint( POINT pt );

		virtual bool	GetHaveFocusIn() { return m_bHasFocus; }

		virtual	bool	CanHaveFocus()
		{ 
			if( m_bEnable == true && m_bShow == true )
				return true;
			else
				return false;
		}

		bool			SendInternelEvent( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
		
		virtual void	SetDialog( CKTDGUIDialogType pDialog)	{ m_pDialog = pDialog; }
		CKTDGUIDialogType	GetDialog()							{ return m_pDialog; }
		UI_CONTROL_TYPE GetType()							{ return m_ControlType; }
        void            SetType(UI_CONTROL_TYPE ctrlType)   { m_ControlType = ctrlType; }

		void			SetName_LUA( const char* pName );
		virtual bool	SetName( const WCHAR* pName );
		virtual void	SetNameByForce( const WCHAR* pName );
#ifdef CONTROL_SET_NAME_STR_ID
		virtual bool	SetNameStrID_LUA( int iIndex );
#endif CONTROL_SET_NAME_STR_ID
		const WCHAR*	GetName()						{ return m_Name.c_str(); }
		//const char*		GetName_MB()					{ return m_Name_MB.c_str(); }
		//const char*		GetName_LUA()					{ return m_Name_MB.c_str(); }

		void			SetTabControlName_LUA( const char* pTapControlName );
		void			SetTabControlName( const WCHAR* pTapControlName ){ m_TabControlName = pTapControlName; }
		const WCHAR*	GetTabControlName(){ return m_TabControlName.c_str(); }

		void			SetPrevTabControlName_LUA( const char* pPrevTapControlName );
		void			SetPrevTabControlName( const WCHAR* pPrevTapControlName ){ m_PrevTabControlName = pPrevTapControlName; }
		const WCHAR*	GetPrevTabControlName(){ return m_PrevTabControlName.c_str(); }

		void			SetOffsetPos_LUA( float x, float y )	{ SetOffsetPos( D3DXVECTOR2( x, y ) ); }
		virtual void	SetOffsetPos( D3DXVECTOR2 pos )		{ m_OffsetPos = pos; }

		void			SetSize_LUA( float x, float y ) { SetSize( D3DXVECTOR2(x,y) ); }
		virtual void	SetSize( D3DXVECTOR2 size ) { m_Size = size; }



		virtual void	ScaleControl( float fx, float fy ) {};
		virtual void	MoveControl( float fx, float fy ) {};


		D3DXVECTOR2		GetOffsetPos()						{ return m_OffsetPos; }

		CKTDGUIControl::UIPointData& GetNowPoint() { return m_NowPoint; }

		virtual void SetEnable( bool bEnable )	{ m_bEnable = bEnable; }
		virtual bool GetEnable()				{ return m_bEnable; }
		virtual void SetShow( bool bShow )		{ m_bShow = bShow; }
		virtual bool GetShow()					{ return m_bShow; }
#ifdef NEW_SKILL_TREE_UI
		bool GetOutOfControlList(){ return m_bOutofList; }
		void SetOutOfControlList( bool bOut ){ m_bOutofList = bOut; }
#endif

		virtual void SetShowEnable( bool bShow, bool bEnable )	{ SetShow( bShow ); SetEnable( bEnable ); }

		virtual void SetSound( bool bSound )	{ m_bSound = bSound; }
		virtual bool GetSound()					{ return m_bSound; }

		void PlaySound( CKTDXDeviceSound* pSound );

		bool GetIsAutoDeleteEnd()				{ return m_bCheckAutoDeleteEnd; }

		void SetColor( D3DXCOLOR color ) { m_Color = color; }
		D3DXCOLOR GetColor() { return m_Color; }

		void Move( const D3DXVECTOR2& pos, D3DXCOLOR color, float fChangeTime, bool bDirect = false, bool bAutoDelete = false,
			bool bPingPong = false, int repeatNum = 1 );
		void Move_LUA();
		void MoveStop();
		void Animate();

		bool GetMouseOver() { return m_bMouseOver; }
		bool GetIsMoving();

		// Minimize / Maximize
		POINT GetOffsetMaximize() { return m_kMaximizeOffset; }
		void SetOffsetMaximize( int x, int y ) { m_kMaximizeOffset.x = x; m_kMaximizeOffset.y = y; }

		bool GetMaximize() { return m_bMaximize; }
		virtual void SetMaximize( bool val ) { m_bMaximize = val; }

		// Dummy Data
		void			AddDummyPos( D3DXVECTOR3 pos ) { m_DummyPosList.push_back( pos ); }
		D3DXVECTOR3 	GetDummyPos( int index );

		void			AddDummyString_LUA( const char* pDummyString );
		std::wstring	GetDummyString( int index );

		void			AddDummyInt( int dummy ) { m_DummyIntList.push_back( dummy ); }
		int				GetDummyInt( int index );
// #ifdef SERV_DUNGEON_FORCED_EXIT_SYSTEM
		bool			SetDummyInt( int Index, int dummy );
// #endif //SERV_DUNGEON_FORCED_EXIT_SYSTEM

		// 특정범위안에서 컨트롤 못움직이게끔.. 일단 버튼을 위해서..
		void			SetEnableLeftTop( D3DXVECTOR2 pos ) { m_vEnableLeftTop = pos; }
		void			SetEnableRightBottom( D3DXVECTOR2 pos ) { m_vEnableRightBottom = pos; }
		void			SetGuideDesc( const WCHAR* pGuideDesc ) { m_GuideDesc = pGuideDesc; }
        void			SetGuideDesc_LUA( int iStringIndex );
        void            SetGuideDescByString_LUA( const char* pGuideDesc );
        wstring			GetGuideDesc() { return m_GuideDesc; }
#ifdef SHOW_DESCRIPTION_WHEN_DISABLE
		void			SetShowDescriptionWhenDisable( bool bShowDescriptionWhenDisable ) { m_bShowDescriptionWhenDisable = bShowDescriptionWhenDisable; }
#endif SHOW_DESCRIPTION_WHEN_DISABLE

		virtual void			SetGuideDescAbsolutePos( D3DXVECTOR2 pos ) { m_vGuideDescAbsolutePos = pos; }
		virtual D3DXVECTOR2		GetGuideDescPos() { return ( m_vGuideDescAbsolutePos + m_vGuideDescOffsetPos); }
		virtual void			SetGuideDescOffsetPos( D3DXVECTOR2 pos ) { m_vGuideDescOffsetPos = pos; }
		virtual bool			GetIsGuideDescAbsolutePos();
        
        void            SetGuideDescLT(bool val) { m_bGuideDescPosLT = val; }
        bool            GetGuideDescLT() { return m_bGuideDescPosLT; } 

	protected:
		void					ChangePoint( UIPointData* pNowPoint, UIPointData* pEndPoint );
		void					UpdateVertex( VERTEX_UI& vertexLT, VERTEX_UI& vertexRT, VERTEX_UI& vertexLB, VERTEX_UI& vertexRB, 
											const UIPointData& pointData );

		void					UpdateAnimVertex( VERTEX_UI* pVertex, CKTDGUIControl::TexChangeData* pTexChangeData );
		D3DXVECTOR2				GetAxisPos( VERTEX_UI* pVertex, TEX_AXIS texAxis );

		HRESULT					RenderVertex( VERTEX_UI* vertex, const UIPointData& pointData, int renderCount = 1 );
#ifdef DYNAMIC_VERTEX_BUFFER_OPT
		HRESULT                 RenderVertex( VERTEX_UI* vertex, bool bRestoreVertexDecl = false );
#endif
		static UITextureData*	SetTexureData_( const WCHAR* pFileName, const WCHAR* pKeyName );
		static UIPointData*		SetPointData_();
		static UIStringData*	SetStringData_();

		TexChangeData*			SetTexChangeData_();

		void					ShowGuideDesc();

		void					UpdateTexChangeData( CKTDGUIControl::TexChangeData* pTexChangeData );
		void					AddTexChangeData( map< int, CKTDGUIControl::TexChangeData* >& mapTexChangeData,
												CKTDGUIControl::UIPointData* pUIPointData, CKTDGUIControl::TexChangeData* pTexChangeData );

		CKTDGUIDialogType		m_pDialog;

        UI_CONTROL_TYPE		m_ControlType;
		wstring				m_Name;
		//string				m_Name_MB;
		wstring				m_PrevTabControlName;
		wstring				m_TabControlName;
		D3DXVECTOR2			m_OffsetPos;
		D3DXVECTOR2			m_Size;
		D3DXCOLOR			m_Color;

		float				m_fAnimateTime;

		D3DXVECTOR2			m_GoalPos;
		D3DXCOLOR			m_GoalColor;
		float				m_fChangeTime;

		bool				m_bAutoDelete;
		bool				m_bCheckAutoDeleteEnd;

		std::queue< CKTDGUIControl::ControlMoveItem >	m_queMoveItems;

		bool				m_bHasFocus;
		bool				m_bMouseOver;
		bool				m_bEnable;
		bool				m_bShow;
		bool				m_bSound;
#ifdef NEW_SKILL_TREE_UI
		bool				m_bOutofList;
#endif

		float				m_fElapsedTime;
		
		UIPointData			m_NowPoint;
		UIPointData			m_EndPoint;

		bool				m_bMaximize;
		POINT				m_kMaximizeOffset;

		// Dummy Data
		vector< D3DXVECTOR3 >		m_DummyPosList;
		vector< wstring	>			m_DummyStringList;
		vector< int >				m_DummyIntList;

		D3DXVECTOR2			m_vEnableLeftTop;
		D3DXVECTOR2			m_vEnableRightBottom;

		wstring				m_GuideDesc;
		D3DXVECTOR2			m_vGuideDescAbsolutePos;
		D3DXVECTOR2			m_vGuideDescOffsetPos;

		CKTDGUIControl::TexChangeData m_NowTexChangeData;

        bool m_bGuideDescPosLT;

#ifndef DYNAMIC_VERTEX_BUFFER_OPT
		LPDIRECT3DVERTEXBUFFER9 m_pVB;
#endif

#ifdef SHOW_DESCRIPTION_WHEN_DISABLE
		bool	m_bShowDescriptionWhenDisable;
#endif SHOW_DESCRIPTION_WHEN_DISABLE
};
