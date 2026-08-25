#pragma once

class CKTDXDeviceXET : public CKTDXDevice
{
	public:
		struct ChangeTexture
		{
			wstring						orgTexName;
			CKTDXDeviceBaseTexture* 	pTexture;

			ChangeTexture();
			~ChangeTexture();
            void    Release();
		};

		struct MultiTexureData
		{
			wstring						orgTexName;
			CKTDXDeviceBaseTexture* 	pTexture;
			D3DTEXTUREOP				colorOP;

			MultiTexureData();
			~MultiTexureData();
            void    Release();
		};

		struct AniEventTexChange
		{
			D3DXVECTOR2					eventTime;
			wstring						orgTexName;
			CKTDXDeviceBaseTexture* 	pTexture;

			AniEventTexChange();
			~AniEventTexChange();
		};

		struct AniData
		{
			wstring					aniName;
			float					fTransitionTime;
			float					fAniMaxTime;
			
            typedef std::multimap<wstring,AniEventTexChange*>  TexChangeEventMultimap;
			TexChangeEventMultimap	m_TexChangeEventlist;

			AniData();
			~AniData();

			CKTDXDeviceBaseTexture* GetAniTexture( float fAniTime, const WCHAR* pTexName );
			void SetTexChangeEvent_LUA( D3DXVECTOR2 eventTime, const char* pOrgTexName, const char* pChangeTexName );
		};

		
        typedef map<wstring, ChangeTexture*>    ChangeTextureMap;
        typedef map<wstring, AniData*>          NameAniDataMap;

        struct  XETProxy
        {
    		ChangeTextureMap            m_ChangeTextureList;
	    	MultiTexureData			    m_MultiTexStage1;
		    MultiTexureData			    m_MultiTexStage2;
		    NameAniDataMap              m_AniDataList;

			~XETProxy();

		    void AddChangeTexture_LUA( const char* pOrgTexName, const char* pChangeTexName );
		    void SetMultiTexStage1_LUA( const char* pOrgTexName, const char* pBlendTexName, int colorOP );
		    void SetMultiTexStage2_LUA( const char* pOrgTexName, const char* pBlendTexName, int colorOP );
		    const AniData* AddAniData_LUA( const char* pAniName, float fTransitionTime = 0.0f, float	fAniMaxTime = 0.0f );
		    const AniData* AddAniDataByFrame_LUA( const char* pAniName, int transitionFrame = 0, float	fAniMaxTime = 0.0f );
        };//struct  XETProxy
	public:
		CKTDXDeviceXET( const WCHAR* pFileName, CKTDXDevice* pMesh = NULL );
		 virtual ~CKTDXDeviceXET(void);


		CKTDXDeviceBaseTexture* GetChangeTexture( const WCHAR* pTexName );

		const MultiTexureData*  GetMultiTex( int stage ) const;
		AniData*				GetAniData( const WCHAR* pAniName );

		void AddChangeTexture( const WCHAR* wszOrgTexName, CKTDXDeviceBaseTexture* pChangeTexture );

		void SetMultiTexStage1( const char* pOrgTexName, const char* pBlendTexName, int colorOP );
		void SetMultiTexStage2( const char* pOrgTexName, const char* pBlendTexName, int colorOP );

		void DeleteMultiTexStage1()	
		{
            m_MultiTexStage1.Release();
		}
		void DeleteMultiTexStage2()	
		{
            m_MultiTexStage2.Release();
		}
// 		CKTDXCollision::CollisionData* CreateCollisionData_LUA( const char* pCollisionName, int collisionType, bool bCollision );
// 
//         const CKTDXCollision::CollisionDataList&         GetCollisionList() const { return m_CollisionList; }
//         const CKTDXCollision::CollisionDataList&         GetAttackList() const { return m_AttackList; }

    protected:

		virtual HRESULT _Load( bool bSkipStateCheck = false
#ifdef	X2OPTIMIZE_SOUND_BACKROUND_LOAD
				, bool bBackgroundQueueing = false
#endif	X2OPTIMIZE_SOUND_BACKROUND_LOAD			
			);
		virtual HRESULT _UnLoad();
	private:
		CKTDXDevice*				m_pMesh;
				
		ChangeTextureMap            m_ChangeTextureList;
		MultiTexureData			    m_MultiTexStage1;
		MultiTexureData			    m_MultiTexStage2;

		NameAniDataMap              m_AniDataList;

// 		CKTDXCollision::CollisionDataList                m_CollisionList;
// 		CKTDXCollision::CollisionDataList                m_AttackList;        
};

/*
CT_LINE = 0
CT_SPHERE = 1

--텍스쳐 변경 만들기
g_pXET:AddChangeTexture( "orgTex.tga", "changeTex.tga" )

--멀티텍스쳐 만들기
g_pXET:SetMultiTexStage1( "orgTex.tga", "blendTex1.tga", D3DTOP_ADD )
g_pXET:SetMultiTexStage2( "orgTex.tga", "blendTex2.tga", D3DTOP_ADD )

--애니메이션 패키지 만들기
g_pTestAni = g_pXET:AddAniData( "Wait", 0.5, 1.0 )
g_pTestAni:SetTexChangeEvent( D3DXVECTOR2(0.3,0.9), "orgTex.tga", "changeTex.tga" )

--충돌데이타 만들기
g_pTestCollisionData = g_pXET:CreateCollisionData( "TestCollisionData", CT_LINE, false )
g_pTestCollisionData:SetPoint1( "frameName", D3DXVECTOR3(0,0,0) )
g_pTestCollisionData:SetPoint2( "frameNameLineEnd", D3DXVECTOR3(0,0,0) )
*/




