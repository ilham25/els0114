/** @file   KTDXDeviceTexture.h
    @date   jintaeks on 2008-10-15, 15:04
            - managed된 텍스쳐를 local memory에 preload하는 기능을 추가하다.
*/

#pragma once

#define D3DFVF_TEXTURE_RHW_VERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

class CTEXViewer;
class CTexUVOut;

class CKTDXDeviceTexture 
	: public CKTDXDeviceBaseTexture 
{
	public:
		friend CTEXViewer;
		friend CTexUVOut;
	
	public:
		enum DETAIL_LEVEL
		{
			DL_HIGH,
			DL_MEDIUM,
			DL_LOW,
		};

		struct TEXTURE_RHW_VERTEX
		{
			float x, y, z, rhw;
			DWORD color;
			float u, v;

#ifdef DYNAMIC_VERTEX_BUFFER_OPT
            bool operator == ( const TEXTURE_RHW_VERTEX& rhs_ ) const
            {
                return  x == rhs_.x && y == rhs_.y && z == rhs_.z && color == rhs_.color && u == rhs_.u && v == rhs_.v;
            }

            bool operator != ( const TEXTURE_RHW_VERTEX& rhs_ ) const
            {
                return !( operator == ( rhs_ ) );
            }
#endif
		};

		struct TEXTURE_UV
		{
			D3DXVECTOR2 leftTop;
			D3DXVECTOR2	rightBottom;
			D3DXVECTOR2 leftBottom;
			D3DXVECTOR2 rightTop;

			D3DXVECTOR2 rectSize;

			int			rotateValue;

			TEXTURE_UV()
			{
				leftTop		= D3DXVECTOR2( 0, 0 );
				rightBottom	= D3DXVECTOR2( 0, 0 );
				leftBottom	= D3DXVECTOR2( -1, -1 );
				rightTop	= D3DXVECTOR2( -1, -1 );

				rectSize	= D3DXVECTOR2( 0, 0 );

				rotateValue = 0;
			}
		};

        typedef map< wstring, TEXTURE_UV* >   KeyTexUVMap;

        struct  TETProxy
        {
            wstring		    m_DeviceID;
            KeyTexUVMap     m_mapTexUVRect;

            TETProxy( const std::wstring& wstrDeviceID ) : m_DeviceID( wstrDeviceID ) {}
			~TETProxy();
		    void			AddRect_LUA( const char* pKey, D3DXVECTOR2 leftTop, D3DXVECTOR2 rightTop, D3DXVECTOR2 leftBottom, D3DXVECTOR2 rightBottom );
		    void			RotateRect_LUA( const char* pKey, int roateValue );
        };//struct  TETProxy
    protected:

		virtual HRESULT     _Load( bool bSkipStateCheck = false
#ifdef	X2OPTIMIZE_SOUND_BACKROUND_LOAD
				, bool bBackgroundQueueing = false
#endif	X2OPTIMIZE_SOUND_BACKROUND_LOAD			
			);
		virtual HRESULT     _UnLoad();
	public:
		CKTDXDeviceTexture( LPDIRECT3DDEVICE9 pd3dDevice, wstring fileName, DETAIL_LEVEL detailLevel = DL_HIGH, D3DFORMAT texFormat = D3DFMT_UNKNOWN, bool bAlwaysHigh = false );
		virtual ~CKTDXDeviceTexture();

        /** @NOTE:
            Background 로딩을 사용하는 경우, Load()함수는 lua_State*를 사용하지 않는 리소스만을
            로드해야 하고, Load_LuaData()에서는 lua_State*를 사용하는 리소스를 로드하도록 구현해
            주어야 한다. - jintaeks on 2008-10-24, 10:29 */
		//virtual HRESULT Load_LuaData( DWORD dwParam_ );


        /// managed된 텍스쳐를 local memory로 preload하는 일을 한다.
        /// - jintaeks on 2008-10-15, 15:02
        void    PreLoad();

		void SetDeviceTexture( int stage = 0 );
		void SetDetailLevel( DETAIL_LEVEL detailLevel );

		LPDIRECT3DTEXTURE9	GetDeviceTexture()
		{
			return m_pTexture;
		}
		int					GetWidth()
		{
			return m_Info.Width;
		}
		int					GetHeight()
		{
			return m_Info.Height;
		}

		TEXTURE_UV*			GetTexUV( const wstring& key );

		void				Draw( int nX, int nY, int nWidth, int nHeight, D3DCOLOR color = 0xffffffff, float fRotateDegree = 0.f );
		void				Draw( int nX, int nY, int nWidth, int nHeight, TEXTURE_UV* pTextureUV, D3DCOLOR color = 0xffffffff );
		void				Draw( float fX, float fY, float fWidth, float fHeight, int nTexU, int nTexV, int nTexWidth, int nTexHeight, D3DCOLOR color  = 0xffffffff   );
        void                DrawLayer( float nX, float nY, float nWidth, float nHeight, D3DCOLOR color, D3DXVECTOR2 minUV, D3DXVECTOR2 maxUV, int blendType = 0);

		const KeyTexUVMap&  GetMapTexUVRect() const { return m_mapTexUVRect; }

	private:
		//bool				LoadTET( const WCHAR* pFileName );

		
		LPDIRECT3DTEXTURE9				m_pTexture;
		D3DXIMAGE_INFO					m_Info;
        UINT                            m_uWidth;
        UINT                            m_uHeight;

		DETAIL_LEVEL					m_DetailLevel;
		
		
		
		D3DFORMAT						m_LoadTexFormat;
		bool							m_bAlwaysHigh;

		LPDIRECT3DVERTEXBUFFER9			m_pVB;
		KeyTexUVMap                    m_mapTexUVRect;
};
