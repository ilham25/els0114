#include "StdAfx.h"
#include ".\ktdxdevicexet.h"

CKTDXDeviceXET::CKTDXDeviceXET( const WCHAR* pFileName, CKTDXDevice* pMesh )
				: CKTDXDevice( pFileName, DT_XET )
{
	m_pMesh				= pMesh;
}

CKTDXDeviceXET::~CKTDXDeviceXET(void)
{
	_UnLoad();
}

HRESULT CKTDXDeviceXET::_Load( bool bSkipStateCheck
#ifdef	X2OPTIMIZE_SOUND_BACKROUND_LOAD
				, bool
#endif	X2OPTIMIZE_SOUND_BACKROUND_LOAD		
	)
{
	////CKTDXThread::CLocker locker( g_pKTDXApp->GetDeviceManager()->GetDeviceLock() );

    XETProxy    proxy;

	lua_tinker::decl( g_pKTDXApp->GetLuaBinder()->GetLuaState(),  "g_pXET", &proxy );

	KGCMassFileManager::CMassFile::MASSFILE_MEMBERFILEINFO_POINTER Info;
	Info = g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LoadDataFile( m_DeviceID );
	if( Info == NULL )
	{
		string strName;
		ConvertWCHARToChar( strName, m_DeviceID.c_str() );
		ErrorLogMsg( KEM_ERROR96, strName.c_str());

		return E_FAIL;
	}

    int iSize = Info->size;

	if( g_pKTDXApp->GetLuaBinder()->DoMemoryNotEncript( Info->pRealData, iSize ) == E_FAIL )
	{
		string strName;
		ConvertWCHARToChar( strName, m_DeviceID.c_str() );
		ErrorLogMsg( KEM_ERROR97, strName.c_str());

		return E_FAIL;
	}


	{
        CKTDXDeviceBaseTexture* pTextureTemp1 = NULL;
        CKTDXDeviceBaseTexture* pTextureTemp2 = NULL;

        CSLock  locker( m_csDeviceState );

        if ( bSkipStateCheck == false )
	    {
		    switch( m_eDeviceState )
		    {
		    case DEVICE_STATE_INIT:
		    //case DEVICE_STATE_PENDING:
		    default:
			    ASSERT( 0 );
		    case DEVICE_STATE_LOADING:
		    case DEVICE_STATE_FAILED:
                m_Size = iSize;
                m_ChangeTextureList.swap( proxy.m_ChangeTextureList );
                m_AniDataList.swap( proxy.m_AniDataList );
                pTextureTemp1 = m_MultiTexStage1.pTexture;
                pTextureTemp2 = m_MultiTexStage2.pTexture;
                m_MultiTexStage1 = proxy.m_MultiTexStage1;
                m_MultiTexStage2 = proxy.m_MultiTexStage2;
                proxy.m_MultiTexStage1.pTexture = pTextureTemp1;
                proxy.m_MultiTexStage2.pTexture = pTextureTemp2;
		        m_eDeviceState = DEVICE_STATE_LOADED;
			    break;
		    case DEVICE_STATE_LOADED:
			    break;
		    }//switch
	    }
        else
        {
            m_Size = iSize;
            m_ChangeTextureList.swap( proxy.m_ChangeTextureList );
            m_AniDataList.swap( proxy.m_AniDataList );
            pTextureTemp1 = m_MultiTexStage1.pTexture;
            pTextureTemp2 = m_MultiTexStage2.pTexture;
            m_MultiTexStage1 = proxy.m_MultiTexStage1;
            m_MultiTexStage2 = proxy.m_MultiTexStage2;
            proxy.m_MultiTexStage1.pTexture = pTextureTemp1;
            proxy.m_MultiTexStage2.pTexture = pTextureTemp2;
        }//if.. else..
    }
	
	return true;
}

HRESULT CKTDXDeviceXET::_UnLoad()
{
	////CKTDXThread::CLocker locker( g_pKTDXApp->GetDeviceManager()->GetDeviceLock() );


//     BOOST_TEST_FOREACH( CKTDXCollision::CollisionData*, pData, m_CollisionList )
//     {
//         SAFE_DELETE( pData );
//     }//BOOST_TEST_FOREACH()
// 	m_CollisionList.clear();
// 
//     BOOST_TEST_FOREACH( CKTDXCollision::CollisionData*, pData, m_AttackList )
//     {
//         SAFE_DELETE( pData );
//     }//BOOST_TEST_FOREACH()
// 	m_AttackList.clear();

	ChangeTexture* pChangeTexture = NULL;
	ChangeTextureMap::iterator iter;
	for( iter = m_ChangeTextureList.begin(); iter != m_ChangeTextureList.end(); iter++ )
	{
		pChangeTexture = iter->second;
		SAFE_DELETE( pChangeTexture );
	}
	m_ChangeTextureList.clear();

	m_MultiTexStage1.Release();
	m_MultiTexStage2.Release();

	AniData* pAniData = NULL;
	NameAniDataMap::iterator iter2;
	for( iter2 = m_AniDataList.begin(); iter2 != m_AniDataList.end(); iter2++ )
	{
		pAniData = iter2->second;
		SAFE_DELETE( pAniData );
	}
	m_AniDataList.clear();

	return S_OK;
}


CKTDXDeviceBaseTexture* CKTDXDeviceXET::GetChangeTexture( const WCHAR* pTexName )
{
	if( NULL == pTexName )
		return NULL;


	wstring texName = pTexName;

	if( true == texName.empty() )
		return NULL;

	MakeUpperCase( texName );
	
	ChangeTextureMap::iterator iter = m_ChangeTextureList.find( texName.c_str() );
	if( iter != m_ChangeTextureList.end() )
	{
		return iter->second->pTexture;
	}
	return NULL;
}

const CKTDXDeviceXET::MultiTexureData* CKTDXDeviceXET::GetMultiTex( int stage ) const
{
	if( stage == 1 )
	{
		return &m_MultiTexStage1;
	}
	else if( stage == 2 )
	{
		return &m_MultiTexStage2;
	}
	return NULL;
}

CKTDXDeviceXET::AniData* CKTDXDeviceXET::GetAniData( const WCHAR* pAniName )
{
    if ( pAniName == NULL || pAniName[0] == NULL )
        return NULL;
	NameAniDataMap::iterator iter = m_AniDataList.find( pAniName );
	if( iter != m_AniDataList.end() )
	{
		return iter->second;
	}
	return NULL;
}


void CKTDXDeviceXET::XETProxy::AddChangeTexture_LUA( const char* pOrgTexName, const char* pChangeTexName )
{
	if ( pOrgTexName == NULL || pOrgTexName[0] == NULL
        || pChangeTexName == NULL || pChangeTexName[0] == NULL )
    {
        ASSERT( 0 );
        return;
    }//if
	
	wstring wChangeTexName = L"";	
	ConvertUtf8ToWCHAR( wChangeTexName, pChangeTexName );
	MakeUpperCase( wChangeTexName ); 

	ChangeTexture* pChangeTexture = new ChangeTexture();
	ConvertUtf8ToWCHAR( pChangeTexture->orgTexName, pOrgTexName );
	MakeUpperCase( pChangeTexture->orgTexName );

	pChangeTexture->pTexture = (CKTDXDeviceBaseTexture*) g_pKTDXApp->GetDeviceManager()->OpenTexture( wChangeTexName );

	ChangeTextureMap::iterator iter = m_ChangeTextureList.find( pChangeTexture->orgTexName );
	if( iter == m_ChangeTextureList.end() )
	{
		m_ChangeTextureList.insert( ChangeTextureMap::value_type(pChangeTexture->orgTexName,pChangeTexture) );
	}
	else
	{
		ChangeTexture* temp = iter->second;
		iter->second = pChangeTexture;
		//m_ChangeTextureList.erase( iter );		
		SAFE_DELETE( temp );
		//m_ChangeTextureList.insert( std::make_pair(pChangeTexture->orgTexName,pChangeTexture) );
	}
}


void CKTDXDeviceXET::AddChangeTexture( const WCHAR* wszOrgTexName, CKTDXDeviceBaseTexture* pDeviceTexture )
{
	if( NULL == wszOrgTexName )
		return;

	if( NULL == pDeviceTexture )
		return; 

	wstring wChangeTexName = wszOrgTexName;	
	MakeUpperCase( wChangeTexName ); 

	ChangeTexture* pChangeTexture = new ChangeTexture();
	pChangeTexture->orgTexName = wszOrgTexName;
	MakeUpperCase( pChangeTexture->orgTexName );

	pChangeTexture->pTexture = pDeviceTexture;

	ChangeTextureMap::iterator iter;
	iter = m_ChangeTextureList.find( pChangeTexture->orgTexName );
	if( iter == m_ChangeTextureList.end() )
	{
		m_ChangeTextureList.insert( ChangeTextureMap::value_type(pChangeTexture->orgTexName, pChangeTexture) );
	}
	else
	{
		//SAFE_DELETE( iter->second );
		//m_ChangeTextureList.erase( iter );		
		//m_ChangeTextureList.insert( std::make_pair(pChangeTexture->orgTexName, pChangeTexture) );
		ChangeTexture* temp = iter->second;
		iter->second = pChangeTexture;
		SAFE_DELETE( temp );
	}
}


void CKTDXDeviceXET::XETProxy::SetMultiTexStage1_LUA( const char* pOrgTexName, const char* pBlendTexName, int colorOP )
{
    if ( pBlendTexName == NULL || pBlendTexName[0] == NULL )
        return;
		
	wstring wBlendTexName = L"";	
	ConvertUtf8ToWCHAR( wBlendTexName, pBlendTexName );
	MakeUpperCase( wBlendTexName ); 

    m_MultiTexStage1.Release();
	ConvertUtf8ToWCHAR( m_MultiTexStage1.orgTexName, pOrgTexName );
	MakeUpperCase( m_MultiTexStage1.orgTexName ); 

	m_MultiTexStage1.pTexture = (CKTDXDeviceBaseTexture*)g_pKTDXApp->GetDeviceManager()->OpenTexture( wBlendTexName );
	m_MultiTexStage1.colorOP = (D3DTEXTUREOP)colorOP;
}

void CKTDXDeviceXET::XETProxy::SetMultiTexStage2_LUA( const char* pOrgTexName, const char* pBlendTexName, int colorOP )
{
	if ( pBlendTexName == NULL || pBlendTexName[0] == NULL )
        return;
		
	wstring wBlendTexName = L"";	
	ConvertUtf8ToWCHAR( wBlendTexName, pBlendTexName );
	MakeUpperCase( wBlendTexName ); 

	m_MultiTexStage2.Release();
	ConvertUtf8ToWCHAR( m_MultiTexStage2.orgTexName, pOrgTexName );
	MakeUpperCase( m_MultiTexStage2.orgTexName );

	m_MultiTexStage2.pTexture = (CKTDXDeviceBaseTexture*)g_pKTDXApp->GetDeviceManager()->OpenTexture( wBlendTexName );
	m_MultiTexStage2.colorOP = (D3DTEXTUREOP)colorOP;
}

void CKTDXDeviceXET::SetMultiTexStage1( const char* pOrgTexName, const char* pBlendTexName, int colorOP )
{
	if ( pBlendTexName == NULL || pBlendTexName[0] == NULL )
		return;

	wstring wBlendTexName = L"";	
	ConvertUtf8ToWCHAR( wBlendTexName, pBlendTexName );
	MakeUpperCase( wBlendTexName ); 

	m_MultiTexStage1.Release();

	ConvertUtf8ToWCHAR( m_MultiTexStage1.orgTexName, pOrgTexName );
	MakeUpperCase( m_MultiTexStage1.orgTexName ); 

	m_MultiTexStage1.pTexture = (CKTDXDeviceBaseTexture*)g_pKTDXApp->GetDeviceManager()->OpenTexture( wBlendTexName );
	m_MultiTexStage1.colorOP = (D3DTEXTUREOP)colorOP;
}

void CKTDXDeviceXET::SetMultiTexStage2( const char* pOrgTexName, const char* pBlendTexName, int colorOP )
{
	if ( pBlendTexName == NULL || pBlendTexName[0] == NULL )
		return;

	wstring wBlendTexName = L"";	
	ConvertUtf8ToWCHAR( wBlendTexName, pBlendTexName );
	MakeUpperCase( wBlendTexName ); 

	m_MultiTexStage2.Release();
	ConvertUtf8ToWCHAR( m_MultiTexStage2.orgTexName, pOrgTexName );
	MakeUpperCase( m_MultiTexStage2.orgTexName );

	m_MultiTexStage2.pTexture = (CKTDXDeviceBaseTexture*)g_pKTDXApp->GetDeviceManager()->OpenTexture( wBlendTexName );
	m_MultiTexStage2.colorOP = (D3DTEXTUREOP)colorOP;
}

const CKTDXDeviceXET::AniData* CKTDXDeviceXET::XETProxy::AddAniData_LUA( const char* pAniName, float fTransitionTime, float fAniMaxTime )
{
	//{{ kimhc // 2011-03-16 // 메모리 릭 수정 코드
	if ( pAniName == NULL || pAniName[0] == NULL )
	{
		ASSERT( false );
		//boost::format fmt("유효하지 않은 애니메이션 이름을 사용하였습니다: %1%, %2%");
		//ErrorLogMsg( KEM_ERROR96, boost::str(fmt % m_DeviceID.c_str() % pAniName) );
		//return NULL;
	}

	wstring wstrAniName;
	ConvertUtf8ToWCHAR( wstrAniName, pAniName );

	NameAniDataMap::iterator mItr = m_AniDataList.find( wstrAniName );

	if ( mItr != m_AniDataList.end() )
	{
		ASSERT( false );
		//boost::format fmt("이미 등록된 애니메이션 이름을 사용하였습니다: %1%, %2%");
		//ErrorLogMsg( KEM_ERROR96, boost::str(fmt % m_DeviceID.c_str() % pAniName) );

		return mItr->second;
		//return NULL;
	}
	else
	{
		AniData* pAniData = new AniData();
		pAniData->aniName = wstrAniName;
		pAniData->fTransitionTime     = fTransitionTime;
		pAniData->fAniMaxTime         = fAniMaxTime;
		m_AniDataList.insert( NameAniDataMap::value_type(pAniData->aniName,pAniData) );

		return pAniData;
	}
	//}} kimhc // 2011-03-16 // 메모리 릭 수정 코드

	// kimhc // 2011-03-16 // 이전의 코드
// 	AniData* pAniData = new AniData();
// 	ConvertUtf8ToWCHAR( pAniData->aniName, pAniName );
// 	pAniData->fTransitionTime	= fTransitionTime;
// 	pAniData->fAniMaxTime		= fAniMaxTime;
// 	m_AniDataList.insert( std::make_pair(pAniData->aniName,pAniData) );
// 
// 	return pAniData;
}

CKTDXDeviceXET::XETProxy::~XETProxy()
{
	ChangeTextureMap::iterator mItrChangeTexture = m_ChangeTextureList.begin();
	const ChangeTexture* pChangeTexture = NULL;

	while ( m_ChangeTextureList.end() != mItrChangeTexture )
	{
		pChangeTexture = mItrChangeTexture->second;
		SAFE_DELETE( pChangeTexture );
		++mItrChangeTexture;
	}
	m_ChangeTextureList.clear();

	NameAniDataMap::iterator mItrNameAniData = m_AniDataList.begin();
	const AniData* pAniData = NULL;

	while ( m_AniDataList.end() != mItrNameAniData )
	{
		pAniData = mItrNameAniData->second;
		SAFE_DELETE( pAniData );
		++mItrNameAniData;
	}
	m_ChangeTextureList.clear();	
}

const CKTDXDeviceXET::AniData* CKTDXDeviceXET::XETProxy::AddAniDataByFrame_LUA( const char* pAniName, int transitionFrame, float	fAniMaxTime )
{
	float fTransitionTime = (float)((float)transitionFrame / 30.0f);
	return AddAniData_LUA( pAniName, fTransitionTime,fAniMaxTime );
}

// CKTDXCollision::CollisionData* CKTDXDeviceXET::CreateCollisionData_LUA( const char* pCollisionName, int collisionType, bool bCollision )
// {
// 	CKTDXCollision::CollisionData* pCollisionData = new CKTDXCollision::CollisionData();
// 	ConvertUtf8ToWCHAR( pCollisionData->m_Name, pCollisionName );
// 
// 	pCollisionData->m_CollisionType = (CKTDXCollision::CollisionType)collisionType;
// 	pCollisionData->m_pDeviceMesh	= m_pMesh;
// 	if( bCollision == true )
// 	{
// 		m_CollisionList.push_back( pCollisionData );
// 	}
// 	else
// 	{
// 		m_AttackList.push_back( pCollisionData );
// 	}
// 
// 	return pCollisionData;
// }



CKTDXDeviceXET::ChangeTexture::ChangeTexture()
{
	pTexture = NULL;
}

CKTDXDeviceXET::ChangeTexture::~ChangeTexture()
{
    Release();
}
void    CKTDXDeviceXET::ChangeTexture::Release()
{
	if( g_pKTDXApp != NULL && g_pKTDXApp->GetAppDestroy() != true )
	{
		// note!! render target texture가 XET로 지정된 경우에는 밖에서 delete
		if( NULL != pTexture && 
			pTexture->GetDeviceType() != CKTDXDevice::DT_RENDER_TARGET_TEXTURE )
		{
			SAFE_CLOSE( pTexture );
		}
	}
    pTexture = NULL;
    orgTexName.resize( 0 );
}

CKTDXDeviceXET::MultiTexureData::MultiTexureData()
{
	pTexture	= NULL;
	colorOP		= D3DTOP_MODULATE;
}

CKTDXDeviceXET::MultiTexureData::~MultiTexureData()
{
    Release();
}
void    CKTDXDeviceXET::MultiTexureData::Release()
{
	if( pTexture != NULL && g_pKTDXApp != NULL && g_pKTDXApp->GetAppDestroy() != true )
	{
		SAFE_CLOSE( pTexture );
	}
    pTexture = NULL;
    orgTexName.resize( 0 );
    colorOP		= D3DTOP_MODULATE;
}//CKTDXDeviceXET::MultiTexureData::ReleaseTexture()

CKTDXDeviceXET::AniEventTexChange::AniEventTexChange()
{
	INIT_VECTOR2( eventTime, 0.0f, 0.0f );
	pTexture	= NULL;
}

CKTDXDeviceXET::AniEventTexChange::~AniEventTexChange()
{
	if( g_pKTDXApp != NULL && g_pKTDXApp->GetAppDestroy() != true )
	{
		SAFE_CLOSE( pTexture );
	}
    pTexture = NULL;
}

CKTDXDeviceXET::AniData::AniData()
{
	fTransitionTime	= 0.0f;
	fAniMaxTime		= 0.0f;
}

CKTDXDeviceXET::AniData::~AniData()
{
	AniEventTexChange* pAniEventTexChange = NULL;
	std::multimap<wstring,AniEventTexChange*>::iterator iter;
	
	for( iter = m_TexChangeEventlist.begin(); iter != m_TexChangeEventlist.end(); iter++ )
	{
		pAniEventTexChange = (AniEventTexChange*)iter->second;
		SAFE_DELETE( pAniEventTexChange );
	}
	m_TexChangeEventlist.clear();

}

CKTDXDeviceBaseTexture* CKTDXDeviceXET::AniData::GetAniTexture( float fAniTime, const WCHAR* pTexName )
{
    if ( pTexName == NULL || pTexName[0] == NULL )
        return NULL;
	TexChangeEventMultimap::iterator iter;
	wstring wstrTexName = pTexName;
	MakeUpperCase( wstrTexName ); 
	iter = m_TexChangeEventlist.find( wstrTexName );
	//iter = m_TexChangeEventlist.find( pTexName );

	while( iter != m_TexChangeEventlist.end() )
	{
		AniEventTexChange* pAniEventTexChange = iter->second;
		if( pAniEventTexChange->eventTime.x <= fAniTime
			&& pAniEventTexChange->eventTime.y > fAniTime )
		{
			return pAniEventTexChange->pTexture;
		}
		
		while( true )
		{
			iter++;
			if( iter == m_TexChangeEventlist.end() )
				break;

			// adjacent_find를 사용할 수 있을 것 같은데... 일단 이렇게 해두자. 
			AniEventTexChange* pAniEventTexChange = iter->second;
			if( iter->first == wstrTexName )
				break;			
		}
	}
	return NULL;
}

void CKTDXDeviceXET::AniData::SetTexChangeEvent_LUA( D3DXVECTOR2 eventTime, const char* pOrgTexName, const char* pChangeTexName )
{
	if ( pOrgTexName == NULL || pOrgTexName[0] == NULL
        || pChangeTexName == NULL || pChangeTexName[0] == NULL )
        return;
		
	wstring wChangeTexName;	
	ConvertUtf8ToWCHAR( wChangeTexName, pChangeTexName );
	MakeUpperCase( wChangeTexName );

	AniEventTexChange* pAniEventTexChange = new AniEventTexChange();
	ConvertUtf8ToWCHAR( pAniEventTexChange->orgTexName, pOrgTexName );
	MakeUpperCase( pAniEventTexChange->orgTexName ); 

	pAniEventTexChange->pTexture = (CKTDXDeviceBaseTexture*)g_pKTDXApp->GetDeviceManager()->OpenTexture( wChangeTexName );
	pAniEventTexChange->eventTime = eventTime;

	m_TexChangeEventlist.insert( TexChangeEventMultimap::value_type( pAniEventTexChange->orgTexName, pAniEventTexChange ) );
}



