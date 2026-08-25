#include    "StdAfx.h"
#include    "KTDGDynamicVBManager.h"


#define MAX_NUM_EXCEPTIONS_IN_ONE_FRAME  2

static inline unsigned  _ConvertToNumVertices( D3DPRIMITIVETYPE d3dPrimitiveType, unsigned uNumPrimitives )
{
    if ( uNumPrimitives == 0 )
        return 0;
    switch( d3dPrimitiveType )
    {
    case D3DPT_POINTLIST:
        return  uNumPrimitives;
    case D3DPT_LINELIST:
        return  uNumPrimitives * 2;
    case D3DPT_LINESTRIP:
        return  uNumPrimitives + 1;
    case D3DPT_TRIANGLELIST:
        return  uNumPrimitives * 3;
    case D3DPT_TRIANGLESTRIP:
    case D3DPT_TRIANGLEFAN:
        return  uNumPrimitives + 2;
    }//switch
    return 0;
}//_ConvertToNumVertices()


static inline unsigned  _ConvertToNumPrimitives( D3DPRIMITIVETYPE d3dPrimitiveType, unsigned uNumVertices )
{
    switch( d3dPrimitiveType )
    {
    case D3DPT_POINTLIST:
        return  uNumVertices;
    case D3DPT_LINELIST:
        return  uNumVertices / 2;
    case D3DPT_LINESTRIP:
        return  ( uNumVertices > 1 ) ? uNumVertices - 1 : 0;
    case D3DPT_TRIANGLELIST:
        return  uNumVertices / 3;
    case D3DPT_TRIANGLESTRIP:
    case D3DPT_TRIANGLEFAN:
        return  ( uNumVertices > 2 ) ? uNumVertices - 2 : 0;
    }//switch

    return 0;
}//_ConverToNumPrimitives()



CKTDGDynamicVBManager::CKTDGDynamicVBManager()
{
    CustomInitialize( true );
}//CKTDGDynamicVBManager::CKTDGDynamicVBManager()

/*virtual*/
CKTDGDynamicVBManager::~CKTDGDynamicVBManager()
{
    CustomFinalize( true );
}//CKTDGDynamicVBManager::~CKTDGDynamicVBManager()


void        CKTDGDynamicVBManager::CustomInitialize( bool bInConstructor_ )
{

    m_eDynamicVBMode = DYNAMIC_VB_MODE_MANAGED;
    m_uNumMaxPrimitiveCount = 0;
    m_pDrawPrimitiveFn = NULL;
    m_uNumExceptionsOccurred = 0;

    BOOST_FOREACH( DynamicVBInfo& dvbInfo, m_aDynamicVBInfo )
    {
        dvbInfo.Reset();
    }//BOOST_FOREACH()

//#ifdef  QUERY_DYNAMIC_VB_OPT_TO_SERVER

    //m_eDynamicVBMode = DYNAMIC_VB_MODE_MANAGED;

//#else   QUERY_DYNAMIC_VB_OPT_TO_SERVER
//
	m_eDynamicVBMode = DYNAMIC_VB_MODE_DYNAMICVB;

	DWORD   dwMajor = 0;
	DWORD   dwMinor = 0;
	GetOSVersion( dwMajor, dwMinor );
	if ( dwMajor == 5
		&& ( dwMinor == 0 || dwMinor == 1 ) )
		m_eDynamicVBMode = DYNAMIC_VB_MODE_DYNAMICVB_WITH_TRY_CATCH;
//
//#endif  QUERY_DYNAMIC_VB_OPT_TO_SERVER

    RegisterVBType( DVB_TYPE_XYZ_DIFFUSE, D3DFVF_XYZ_DIFFUSE, 1024 );           // D3DFVF_SLASH_TRACE, D3DFVF_LINE_MAP_VERTEX
    RegisterVBType( DVB_TYPE_XYZ_DIFFUSE_TEX1, D3DFVF_XYZ_DIFFUSE_TEX1, 1024 ); // D3DFVF_SLASH_TRACE_TEXTURED, D3DFVF_UNIT_SHADOW_VERTEX, SLVERTEX::FVF, D3DFVF_PARTICLE
    RegisterVBType( DVB_TYPE_XYZ_TEX1, D3DFVF_XYZ_TEX1, 1024 );                 // D3DFVF_RENDERTARGET_VERTEX
    RegisterVBType( DVB_TYPE_XYZRHW_DIFFUSE_TEX1, D3DFVF_XYZRHW_DIFFUSE_TEX1, 1024 );   
    // D3DFVF_LENSFLARE, D3DFVF_VERTEX_UI, D3DFVF_TEXTURE_RHW_VERTEX, S2DUIVertex::FVF, D3DFVF_RENDERTARGET_RHW_VERTEX, D3DFVF_PARTICLE_RHW
    // D3DFVF_DRAWFACE_RHW_VERTEX

#ifdef DXUT_KTDXLIB_SAFE_ON_RESET
    if ( DXUTGetDeviceObjectsReset() == true )
    {
        if ( g_pKTDXApp != NULL && g_pKTDXApp->GetDevice() != NULL )
            g_pKTDXApp->GetDevice()->EvictManagedResources();
        OnResetDevice();
    }//if
#endif

}//CKTDGDynamicVBManager::CustomInitialize()


/*virtual*/
void        CKTDGDynamicVBManager::CustomFinalize( bool bInDestructor_ )
{
    BOOST_FOREACH( DynamicVBInfo& dvbInfo, m_aDynamicVBInfo )
    {
        dvbInfo.Reset();
    }//BOOST_FOREACH()

}//CKTDGDynamicVBManager::CustomFinalize()


bool        CKTDGDynamicVBManager::SetDynamicVBMode( EDYNAMIC_VB_MODE eDynamicVBMode )
{
    if ( eDynamicVBMode < EDYNAMIC_VB_MODE( 0 )
        || eDynamicVBMode >= DYNAMIC_VB_MODE_NUMS )
        return false;

    if ( m_eDynamicVBMode == eDynamicVBMode )
        return true;

#ifdef DXUT_KTDXLIB_SAFE_ON_RESET
    bool bResetObjectsCalled = DXUTGetDeviceObjectsReset();

    if ( bResetObjectsCalled == true )
    {
        OnLostDevice();
    }//if
#else
	OnLostDevice();
#endif

    m_eDynamicVBMode = eDynamicVBMode;

#ifdef DXUT_KTDXLIB_SAFE_ON_RESET
    if ( bResetObjectsCalled == true )
    {
        if ( g_pKTDXApp != NULL && g_pKTDXApp->GetDevice() != NULL )
            g_pKTDXApp->GetDevice()->EvictManagedResources();

        OnResetDevice();
    }//if
#else
	if ( g_pKTDXApp != NULL && g_pKTDXApp->GetDevice() != NULL )
		g_pKTDXApp->GetDevice()->EvictManagedResources();

	OnResetDevice();
#endif

    return true;

}//CKTDGDynamicVBManager::SetDynamicVBMode()


/*virtual*/
HRESULT     CKTDGDynamicVBManager::OnFrameMove( float fElapsedTime )
{
    return  S_OK;
}//CKTDGDynamicVBManager::OnFrameMove()


/*virtual*/
HRESULT     CKTDGDynamicVBManager::OnResetDevice()
{
    m_uNumMaxPrimitiveCount = 0;

    if ( g_pKTDXApp != NULL && g_pKTDXApp->GetDevice() != NULL )
    {
        D3DCAPS9    caps;
        ZeroMemory( &caps, sizeof(caps) );
        HRESULT hr = g_pKTDXApp->GetDevice()->GetDeviceCaps( &caps );
        if ( SUCCEEDED( hr ) )
        {
            m_uNumMaxPrimitiveCount = caps.MaxPrimitiveCount;
        }//if
    }//if

    BOOST_FOREACH( DynamicVBInfo& dvbInfo, m_aDynamicVBInfo )
    {
        dvbInfo.OnResetDevice( m_eDynamicVBMode );
    }//BOOST_FOREACH()

    switch( m_eDynamicVBMode )
    {
    case DYNAMIC_VB_MODE_MANAGED:
        m_pDrawPrimitiveFn = &CKTDGDynamicVBManager::_DrawPrimitiveManaged;
        break;
    case DYNAMIC_VB_MODE_PRIMITIVEUP:
        m_pDrawPrimitiveFn = &CKTDGDynamicVBManager::_DrawPrimitiveUP;
        break;
    case DYNAMIC_VB_MODE_DYNAMICVB:
        m_pDrawPrimitiveFn = &CKTDGDynamicVBManager::_DrawPrimitiveDynamicVB;
        break;
    case DYNAMIC_VB_MODE_DYNAMICVB_WITH_TRY_CATCH:
        m_pDrawPrimitiveFn = &CKTDGDynamicVBManager::_DrawPrimitiveDynamicVBWithTryCatch;
        break;
    }//switch

    return  S_OK;

}//CKTDGDynamicVBManager::OnResetDevice()


/*virtual*/
HRESULT     CKTDGDynamicVBManager::OnLostDevice()
{
    m_pDrawPrimitiveFn = NULL;


    BOOST_FOREACH( DynamicVBInfo& dvbInfo, m_aDynamicVBInfo )
    {
        dvbInfo.OnLostDevice( m_eDynamicVBMode );
    }//BOOST_FOREACH()

    return  S_OK;

}//CKTDGDynamicVBManager::OnLostDevice()

/*static*/
void    CKTDGDynamicVBManager::GetOSVersion( DWORD& dwOutMajor, DWORD& dwOutMinor )
{
    dwOutMajor = 0;
    dwOutMinor = 0;
    OSVERSIONINFOW  info;
    info.dwOSVersionInfoSize = sizeof(info);
    BOOL  bRet = ::GetVersionExW( &info );
    if ( bRet )
    {
        dwOutMajor = info.dwMajorVersion;
        dwOutMinor = info.dwMinorVersion;
    }//if
}//CKTDGDynamicVBManager::GetOSVersion()


bool        CKTDGDynamicVBManager::RegisterVBType( EDYNAMIC_VB_TYPE eType, DWORD dwFVF, unsigned uNumVertices, bool bGenVB )
{
    if ( eType < EDYNAMIC_VB_TYPE( 0 )
        || eType >= DVB_END )
        return false;
    if ( dwFVF == 0 || uNumVertices == 0  )
        return false;

    DWORD   dwVertexStride = D3DXGetFVFVertexSize( dwFVF );
    ASSERT( dwVertexStride != 0 );
    if ( dwVertexStride == 0 )
        return false;

    DynamicVBInfo& dvbInfo = m_aDynamicVBInfo[ eType ];
    if ( dvbInfo.IsRegistered() == true )
        return false;

    dvbInfo.m_uNumVertices = uNumVertices;
    dvbInfo.m_dwFVF = dwFVF;
    dvbInfo.m_dwVertexStride = dwVertexStride;
    dvbInfo.m_uCurVertexIndex = 0;

    if ( bGenVB == true )
    {
        dvbInfo.OnResetDevice( m_eDynamicVBMode );
    }//if

	return true;
}//CKTDGDynamicVBManager::RegisterVBType()








void        CKTDGDynamicVBManager::_DrawPrimitiveManaged( EDYNAMIC_VB_TYPE eType, D3DPRIMITIVETYPE d3dPrimitiveType, unsigned uNumPrimitives, const void* pData, unsigned uNumDraws ) throw()
{
    if ( eType < EDYNAMIC_VB_TYPE( 0 )
        || eType >= DVB_END || uNumPrimitives == 0 || pData == NULL || uNumDraws == 0 )
        return;

    DynamicVBInfo& dvbInfo = m_aDynamicVBInfo[ eType ];
    if ( dvbInfo.IsRegistered() == false || dvbInfo.m_pVB == NULL )
        return;

    if ( g_pKTDXApp == NULL || g_pKTDXApp->GetDevice() == NULL || m_uNumMaxPrimitiveCount == 0 )
        return;

    if ( d3dPrimitiveType == D3DPT_TRIANGLEFAN && uNumPrimitives > m_uNumMaxPrimitiveCount )
        uNumPrimitives = m_uNumMaxPrimitiveCount;

    unsigned    uNumVertices = _ConvertToNumVertices( d3dPrimitiveType, uNumPrimitives );
    if ( uNumVertices == 0 )
        return;
    if ( d3dPrimitiveType == D3DPT_TRIANGLEFAN && uNumVertices > dvbInfo.m_uNumVertices )
    {
        uNumVertices = dvbInfo.m_uNumVertices;
        uNumPrimitives = _ConvertToNumPrimitives( D3DPT_TRIANGLEFAN, uNumVertices );
    }//if

	g_pKTDXApp->GetDevice()->SetFVF( dvbInfo.m_dwFVF );
    g_pKTDXApp->GetDevice()->SetStreamSource( 0, dvbInfo.m_pVB, 0, dvbInfo.m_dwVertexStride );

    void* pLockedVertex = NULL;
    unsigned    uCurVertexIndex = dvbInfo.m_uCurVertexIndex;

    if ( uCurVertexIndex + uNumVertices > dvbInfo.m_uNumVertices )
    {
        uCurVertexIndex = 0;
        unsigned    uNumPrimsPossible = _ConvertToNumPrimitives( d3dPrimitiveType, dvbInfo.m_uNumVertices );
        if ( uNumPrimsPossible > m_uNumMaxPrimitiveCount )
            uNumPrimsPossible = m_uNumMaxPrimitiveCount;
        if ( d3dPrimitiveType == D3DPT_TRIANGLESTRIP && uNumPrimsPossible < uNumPrimitives )
            uNumPrimsPossible = ( uNumPrimsPossible / 2 ) * 2;
        if ( uNumPrimsPossible == 0 )
            return;

        while( uNumPrimsPossible < uNumPrimitives )
        {
            uNumVertices = _ConvertToNumVertices( d3dPrimitiveType, uNumPrimsPossible );
            ASSERT( uNumVertices > 0 && uNumVertices <= dvbInfo.m_uNumVertices );

            HRESULT hr = dvbInfo.m_pVB->Lock( uCurVertexIndex * dvbInfo.m_dwVertexStride
                , uNumVertices * dvbInfo.m_dwVertexStride, &pLockedVertex, 0 );
            if ( FAILED( hr ) )
                return;
            ASSERT( pLockedVertex != NULL );
            if ( pLockedVertex != NULL )
                CopyMemory( pLockedVertex, pData, uNumVertices * dvbInfo.m_dwVertexStride );
            dvbInfo.m_pVB->Unlock();

            if ( uNumDraws == 1 )
                g_pKTDXApp->GetDevice()->DrawPrimitive( d3dPrimitiveType, uCurVertexIndex, uNumPrimsPossible );
            else
            {
                for( unsigned u = 0; u != uNumDraws; u++ )
                {
                    g_pKTDXApp->GetDevice()->DrawPrimitive( d3dPrimitiveType, uCurVertexIndex, uNumPrimsPossible );
                }//for
            }//for

            uCurVertexIndex = 0;
            dvbInfo.m_uCurVertexIndex = uCurVertexIndex;
            uNumPrimitives -= uNumPrimsPossible;
            switch( d3dPrimitiveType )
            {
            case D3DPT_POINTLIST:
            case D3DPT_LINESTRIP:
            case D3DPT_TRIANGLESTRIP:
                pData = static_cast<const BYTE*>( pData ) + dvbInfo.m_dwVertexStride * uNumPrimsPossible;
                break;
            case D3DPT_LINELIST:
                pData = static_cast<const BYTE*>( pData ) + dvbInfo.m_dwVertexStride * ( uNumPrimsPossible * 2 );
                break;
            case D3DPT_TRIANGLELIST:
                pData = static_cast<const BYTE*>( pData ) + dvbInfo.m_dwVertexStride * ( uNumPrimsPossible * 3 );
                break;
            default:
                return;
            }//switch

            unsigned    uNumPrimsPossible = _ConvertToNumPrimitives( d3dPrimitiveType, dvbInfo.m_uNumVertices );
            if ( uNumPrimsPossible > m_uNumMaxPrimitiveCount )
                uNumPrimsPossible = m_uNumMaxPrimitiveCount;
            if ( d3dPrimitiveType == D3DPT_TRIANGLESTRIP && uNumPrimsPossible < uNumPrimitives )
                uNumPrimsPossible = ( uNumPrimsPossible / 2 ) * 2;
            if ( uNumPrimsPossible == 0 )
                return;
        }

        uNumVertices = _ConvertToNumVertices( d3dPrimitiveType, uNumPrimitives );
        ASSERT( uNumVertices <= dvbInfo.m_uNumVertices );
        if ( uNumVertices == 0 || uNumVertices > dvbInfo.m_uNumVertices )
            return;
    }//if

//out:

    HRESULT hr = dvbInfo.m_pVB->Lock( uCurVertexIndex * dvbInfo.m_dwVertexStride
        , uNumVertices * dvbInfo.m_dwVertexStride, &pLockedVertex, 0 );
    if ( FAILED( hr ) )
        return;
    ASSERT( pLockedVertex != NULL );
    if ( pLockedVertex != NULL )
        CopyMemory( pLockedVertex, pData, uNumVertices * dvbInfo.m_dwVertexStride );
    dvbInfo.m_pVB->Unlock();

    if ( uNumDraws == 1 )
        g_pKTDXApp->GetDevice()->DrawPrimitive( d3dPrimitiveType, uCurVertexIndex, uNumPrimitives );
    else
    {
        for( unsigned u = 0; u != uNumDraws; u++ )
        {
            g_pKTDXApp->GetDevice()->DrawPrimitive( d3dPrimitiveType, uCurVertexIndex, uNumPrimitives );
        }//for
    }//for

    dvbInfo.m_uCurVertexIndex = uCurVertexIndex + uNumVertices;

}//CKTDGDynamicVBManager::_DrawPrimitiveManaged()


void    CKTDGDynamicVBManager::_DrawPrimitiveUP( EDYNAMIC_VB_TYPE eType, D3DPRIMITIVETYPE d3dPrimitiveType, unsigned uNumPrimitives, const void* pData, unsigned uNumDraws )  throw() 
{
    if ( eType < EDYNAMIC_VB_TYPE( 0 )
        || eType >= DVB_END || uNumPrimitives == 0 || pData == NULL || uNumDraws == 0 )
        return;

    DynamicVBInfo& dvbInfo = m_aDynamicVBInfo[ eType ];
    if ( dvbInfo.IsRegistered() == false )
        return;

    if ( g_pKTDXApp == NULL || g_pKTDXApp->GetDevice() == NULL || m_uNumMaxPrimitiveCount == 0 )
        return;

    if ( d3dPrimitiveType == D3DPT_TRIANGLEFAN && uNumPrimitives > m_uNumMaxPrimitiveCount )
        uNumPrimitives = m_uNumMaxPrimitiveCount;

	g_pKTDXApp->GetDevice()->SetFVF( dvbInfo.m_dwFVF );

    unsigned    uNumPrimsPossible = m_uNumMaxPrimitiveCount;
    if ( d3dPrimitiveType == D3DPT_TRIANGLESTRIP && uNumPrimsPossible < uNumPrimitives )
        uNumPrimsPossible = ( uNumPrimsPossible / 2 ) * 2;
    if ( uNumPrimsPossible == 0 )
        return;

    while ( uNumPrimsPossible < uNumPrimitives )
    {
        if ( uNumDraws == 1 )
            g_pKTDXApp->GetDevice()->DrawPrimitiveUP( d3dPrimitiveType, uNumPrimsPossible, pData, dvbInfo.m_dwVertexStride );
        else
        {
            for( unsigned u = 0; u != uNumDraws; u++ )
            {
                g_pKTDXApp->GetDevice()->DrawPrimitiveUP( d3dPrimitiveType, uNumPrimsPossible, pData, dvbInfo.m_dwVertexStride );
            }//for
        }//for

        uNumPrimitives -= uNumPrimsPossible;
        switch( d3dPrimitiveType )
        {
        case D3DPT_POINTLIST:
        case D3DPT_LINESTRIP:
        case D3DPT_TRIANGLESTRIP:
            pData = static_cast<const BYTE*>( pData ) + dvbInfo.m_dwVertexStride * uNumPrimsPossible;
            break;
        case D3DPT_LINELIST:
            pData = static_cast<const BYTE*>( pData ) + dvbInfo.m_dwVertexStride * ( uNumPrimsPossible * 2 );
            break;
        case D3DPT_TRIANGLELIST:
            pData = static_cast<const BYTE*>( pData ) + dvbInfo.m_dwVertexStride * ( uNumPrimsPossible * 3 );
            break;
        default:
            return;
        }//switch
        uNumPrimsPossible = m_uNumMaxPrimitiveCount;
        if ( d3dPrimitiveType == D3DPT_TRIANGLESTRIP && uNumPrimsPossible < uNumPrimitives )
            uNumPrimsPossible = ( uNumPrimsPossible / 2 ) * 2;
        if ( uNumPrimsPossible == 0 )
            return;
    }//if

    if ( uNumDraws == 1 )
        g_pKTDXApp->GetDevice()->DrawPrimitiveUP( d3dPrimitiveType, uNumPrimitives, pData, dvbInfo.m_dwVertexStride );
    else
    {
        for( unsigned u = 0; u != uNumDraws; u++ )
        {
            g_pKTDXApp->GetDevice()->DrawPrimitiveUP( d3dPrimitiveType, uNumPrimitives, pData, dvbInfo.m_dwVertexStride );
        }//for
    }//for
}

void        CKTDGDynamicVBManager::_DrawPrimitiveDynamicVB( EDYNAMIC_VB_TYPE eType, D3DPRIMITIVETYPE d3dPrimitiveType, unsigned uNumPrimitives, const void* pData, unsigned uNumDraws )  throw() 
{
#pragma region 예외 처리
    if ( eType < EDYNAMIC_VB_TYPE( 0 )
        || eType >= DVB_END || uNumPrimitives == 0 || pData == NULL || uNumDraws == 0 )
        return;

    DynamicVBInfo& dvbInfo = m_aDynamicVBInfo[ eType ];
    if ( dvbInfo.IsRegistered() == false || dvbInfo.m_pVB == NULL )
        return;

    if ( g_pKTDXApp == NULL || g_pKTDXApp->GetDevice() == NULL || m_uNumMaxPrimitiveCount == 0 )
        return;

    if ( d3dPrimitiveType == D3DPT_TRIANGLEFAN && uNumPrimitives > m_uNumMaxPrimitiveCount )
        uNumPrimitives = m_uNumMaxPrimitiveCount;

    unsigned    uNumVertices = _ConvertToNumVertices( d3dPrimitiveType, uNumPrimitives );
    if ( uNumVertices == 0 )
        return;
    if ( d3dPrimitiveType == D3DPT_TRIANGLEFAN && uNumVertices > dvbInfo.m_uNumVertices )
    {
        uNumVertices = dvbInfo.m_uNumVertices;
        uNumPrimitives = _ConvertToNumPrimitives( D3DPT_TRIANGLEFAN, uNumVertices );
    }//if
#pragma endregion 

	g_pKTDXApp->GetDevice()->SetFVF( dvbInfo.m_dwFVF );
    g_pKTDXApp->GetDevice()->SetStreamSource( 0, dvbInfo.m_pVB, 0, dvbInfo.m_dwVertexStride );

    void* pLockedVertex = NULL;
    unsigned    uCurVertexIndex = dvbInfo.m_uCurVertexIndex;

    if ( uCurVertexIndex + uNumVertices > dvbInfo.m_uNumVertices )
    {
        unsigned uNumPrimsPossible = 0;

        if ( uCurVertexIndex >= dvbInfo.m_uNumVertices
            || d3dPrimitiveType == D3DPT_TRIANGLEFAN )
        {
            uCurVertexIndex = 0;
        }
#pragma region 할당된 버텍스 여유 확인
        uNumPrimsPossible = _ConvertToNumPrimitives( d3dPrimitiveType, dvbInfo.m_uNumVertices - uCurVertexIndex );
        if ( uNumPrimsPossible > m_uNumMaxPrimitiveCount )
            uNumPrimsPossible = m_uNumMaxPrimitiveCount;
        if ( d3dPrimitiveType == D3DPT_TRIANGLESTRIP && uNumPrimsPossible < uNumPrimitives )
            uNumPrimsPossible = ( uNumPrimsPossible / 2 ) * 2;
        if ( uNumPrimsPossible == 0 )
        {
            uCurVertexIndex = 0;
            uNumPrimsPossible = _ConvertToNumPrimitives( d3dPrimitiveType, dvbInfo.m_uNumVertices );
            if ( uNumPrimsPossible > m_uNumMaxPrimitiveCount )
                uNumPrimsPossible = m_uNumMaxPrimitiveCount;
            if ( d3dPrimitiveType == D3DPT_TRIANGLESTRIP && uNumPrimsPossible < uNumPrimitives )
                uNumPrimsPossible = ( uNumPrimsPossible / 2 ) * 2;
            if ( uNumPrimsPossible == 0 )
                return;
        }
#pragma endregion 

		// 버텍스가 그려야할 버텍스의 크기보다 작다면, 한계량보다 많다면
        while( uNumPrimsPossible < uNumPrimitives )
        {
            uNumVertices = _ConvertToNumVertices( d3dPrimitiveType, uNumPrimsPossible );
            ASSERT( uNumVertices > 0 && uNumVertices <= dvbInfo.m_uNumVertices );

            HRESULT hr = dvbInfo.m_pVB->Lock( uCurVertexIndex * dvbInfo.m_dwVertexStride
                , uNumVertices * dvbInfo.m_dwVertexStride, &pLockedVertex, ( uCurVertexIndex == 0 ) ? D3DLOCK_DISCARD : D3DLOCK_NOOVERWRITE );
            if ( FAILED( hr ) )
                return;
            ASSERT( pLockedVertex != NULL );
            if ( pLockedVertex != NULL )
                CopyMemory( pLockedVertex, pData, uNumVertices * dvbInfo.m_dwVertexStride );
            dvbInfo.m_pVB->Unlock();
            if ( uNumDraws == 1 )
                g_pKTDXApp->GetDevice()->DrawPrimitive( d3dPrimitiveType, uCurVertexIndex, uNumPrimsPossible );
            else
            {
                for( unsigned u = 0; u != uNumDraws; u++ )
                {
                    g_pKTDXApp->GetDevice()->DrawPrimitive( d3dPrimitiveType, uCurVertexIndex, uNumPrimsPossible );
                }//for
            }//for

            uCurVertexIndex = uCurVertexIndex + uNumVertices;
            dvbInfo.m_uCurVertexIndex = uCurVertexIndex;
            uNumPrimitives -= uNumPrimsPossible;
            switch( d3dPrimitiveType )
            {
            case D3DPT_POINTLIST:
            case D3DPT_LINESTRIP:
            case D3DPT_TRIANGLESTRIP:
                pData = static_cast<const BYTE*>( pData ) + dvbInfo.m_dwVertexStride * uNumPrimsPossible;
                break;
            case D3DPT_LINELIST:
                pData = static_cast<const BYTE*>( pData ) + dvbInfo.m_dwVertexStride * ( uNumPrimsPossible * 2 );
                break;
            case D3DPT_TRIANGLELIST:
                pData = static_cast<const BYTE*>( pData ) + dvbInfo.m_dwVertexStride * ( uNumPrimsPossible * 3 );
                break;
            default:
                return;
            }//switch

            uNumPrimsPossible = _ConvertToNumPrimitives( d3dPrimitiveType, dvbInfo.m_uNumVertices - uCurVertexIndex );
            if ( uNumPrimsPossible > m_uNumMaxPrimitiveCount )
                uNumPrimsPossible = m_uNumMaxPrimitiveCount;
            if ( d3dPrimitiveType == D3DPT_TRIANGLESTRIP && uNumPrimsPossible < uNumPrimitives )
                uNumPrimsPossible = ( uNumPrimsPossible / 2 ) * 2;
            if ( uNumPrimsPossible == 0 )
            {
                uCurVertexIndex = 0;
                uNumPrimsPossible = _ConvertToNumPrimitives( d3dPrimitiveType, dvbInfo.m_uNumVertices );
                if ( uNumPrimsPossible > m_uNumMaxPrimitiveCount )
                    uNumPrimsPossible = m_uNumMaxPrimitiveCount;
                if ( d3dPrimitiveType == D3DPT_TRIANGLESTRIP && uNumPrimsPossible < uNumPrimitives )
                    uNumPrimsPossible = ( uNumPrimsPossible / 2 ) * 2;
                if ( uNumPrimsPossible == 0 )
                    return;
            }
        }

        uNumVertices = _ConvertToNumVertices( d3dPrimitiveType, uNumPrimitives );
        ASSERT( uNumVertices <= dvbInfo.m_uNumVertices );
        if ( uNumVertices == 0 || uNumVertices > dvbInfo.m_uNumVertices )
            return;
    }//if

//out:

    HRESULT hr = dvbInfo.m_pVB->Lock( uCurVertexIndex * dvbInfo.m_dwVertexStride
        , uNumVertices * dvbInfo.m_dwVertexStride, &pLockedVertex, ( uCurVertexIndex == 0 ) ? D3DLOCK_DISCARD : D3DLOCK_NOOVERWRITE );
    if ( FAILED( hr ) )
        return;
    ASSERT( pLockedVertex != NULL );
    if ( pLockedVertex != NULL )
        CopyMemory( pLockedVertex, pData, uNumVertices * dvbInfo.m_dwVertexStride );
    dvbInfo.m_pVB->Unlock();


    if ( uNumDraws == 1 )
        g_pKTDXApp->GetDevice()->DrawPrimitive( d3dPrimitiveType, uCurVertexIndex, uNumPrimitives );
    else
    {
        for( unsigned u = 0; u != uNumDraws; u++ )
        {
            g_pKTDXApp->GetDevice()->DrawPrimitive( d3dPrimitiveType, uCurVertexIndex, uNumPrimitives );
        }//for
    }//for
    dvbInfo.m_uCurVertexIndex = uCurVertexIndex + uNumVertices;

}//CKTDGDynamicVBManager::_DrawPrimitiveDynamicVB()




void    CKTDGDynamicVBManager::_DrawPrimitiveDynamicVBWithTryCatch( EDYNAMIC_VB_TYPE eType, D3DPRIMITIVETYPE d3dPrimitiveType, unsigned uNumPrimitives, const void* pData, unsigned uNumDraws )  throw()
{
    if ( DXUTGetDeviceLost() == true )
        return;

    if ( eType < EDYNAMIC_VB_TYPE( 0 )
        || eType >= DVB_END || uNumPrimitives == 0 || pData == NULL || uNumDraws == 0 )
        return;

    DynamicVBInfo& dvbInfo = m_aDynamicVBInfo[ eType ];
    if ( dvbInfo.IsRegistered() == false || dvbInfo.m_pVB == NULL )
        return;

    if ( g_pKTDXApp == NULL || g_pKTDXApp->GetDevice() == NULL || m_uNumMaxPrimitiveCount == 0 )
        return;

    if ( d3dPrimitiveType == D3DPT_TRIANGLEFAN && uNumPrimitives > m_uNumMaxPrimitiveCount )
        uNumPrimitives = m_uNumMaxPrimitiveCount;

    unsigned    uNumVertices = _ConvertToNumVertices( d3dPrimitiveType, uNumPrimitives );
    if ( uNumVertices == 0 )
        return;
    if ( d3dPrimitiveType == D3DPT_TRIANGLEFAN && uNumVertices > dvbInfo.m_uNumVertices )
    {
        uNumVertices = dvbInfo.m_uNumVertices;
        uNumPrimitives = _ConvertToNumPrimitives( D3DPT_TRIANGLEFAN, uNumVertices );
    }//if

	g_pKTDXApp->GetDevice()->SetFVF( dvbInfo.m_dwFVF );
    g_pKTDXApp->GetDevice()->SetStreamSource( 0, dvbInfo.m_pVB, 0, dvbInfo.m_dwVertexStride );

    void* pLockedVertex = NULL;
    unsigned    uCurVertexIndex = dvbInfo.m_uCurVertexIndex;

    if ( uCurVertexIndex + uNumVertices > dvbInfo.m_uNumVertices )
    {
        unsigned uNumPrimsPossible = 0;

        if ( uCurVertexIndex >= dvbInfo.m_uNumVertices
            || d3dPrimitiveType == D3DPT_TRIANGLEFAN )
        {
            uCurVertexIndex = 0;
        }
        uNumPrimsPossible = _ConvertToNumPrimitives( d3dPrimitiveType, dvbInfo.m_uNumVertices - uCurVertexIndex );
        if ( uNumPrimsPossible > m_uNumMaxPrimitiveCount )
            uNumPrimsPossible = m_uNumMaxPrimitiveCount;
        if ( d3dPrimitiveType == D3DPT_TRIANGLESTRIP && uNumPrimsPossible < uNumPrimitives )
            uNumPrimsPossible = ( uNumPrimsPossible / 2 ) * 2;
        if ( uNumPrimsPossible == 0 )
        {
            uCurVertexIndex = 0;
            uNumPrimsPossible = _ConvertToNumPrimitives( d3dPrimitiveType, dvbInfo.m_uNumVertices );
            if ( uNumPrimsPossible > m_uNumMaxPrimitiveCount )
                uNumPrimsPossible = m_uNumMaxPrimitiveCount;
            if ( d3dPrimitiveType == D3DPT_TRIANGLESTRIP && uNumPrimsPossible < uNumPrimitives )
                uNumPrimsPossible = ( uNumPrimsPossible / 2 ) * 2;
            if ( uNumPrimsPossible == 0 )
                return;
        }

        while( uNumPrimsPossible < uNumPrimitives )
        {
            uNumVertices = _ConvertToNumVertices( d3dPrimitiveType, uNumPrimsPossible );
            ASSERT( uNumVertices > 0 && uNumVertices <= dvbInfo.m_uNumVertices );

            HRESULT hr = dvbInfo.m_pVB->Lock( uCurVertexIndex * dvbInfo.m_dwVertexStride
                , uNumVertices * dvbInfo.m_dwVertexStride, &pLockedVertex, ( uCurVertexIndex == 0 ) ? D3DLOCK_DISCARD : D3DLOCK_NOOVERWRITE );
            if ( FAILED( hr ) )
                return;
            ASSERT( pLockedVertex != NULL );
            bool    bExceptionOccurred = false;
            if ( pLockedVertex != NULL )
            {
                bExceptionOccurred = KExceptionSafeUtil::ESCopyMemory( pLockedVertex, pData, uNumVertices * dvbInfo.m_dwVertexStride );
            }
            dvbInfo.m_pVB->Unlock();
            if ( bExceptionOccurred == true )
            {
                m_uNumExceptionsOccurred++;
                if ( m_uNumExceptionsOccurred >= MAX_NUM_EXCEPTIONS_IN_ONE_FRAME )
                {
                    DXUTSetDeviceLost( true );
                }//if
                return;
            }//if

            if ( uNumDraws == 1 )
                g_pKTDXApp->GetDevice()->DrawPrimitive( d3dPrimitiveType, uCurVertexIndex, uNumPrimsPossible );
            else
            {
                for( unsigned u = 0; u != uNumDraws; u++ )
                {
                    g_pKTDXApp->GetDevice()->DrawPrimitive( d3dPrimitiveType, uCurVertexIndex, uNumPrimsPossible );
                }//for
            }//for

            uCurVertexIndex = uCurVertexIndex + uNumVertices;
            dvbInfo.m_uCurVertexIndex = uCurVertexIndex;
            uNumPrimitives -= uNumPrimsPossible;
            switch( d3dPrimitiveType )
            {
            case D3DPT_POINTLIST:
            case D3DPT_LINESTRIP:
            case D3DPT_TRIANGLESTRIP:
                pData = static_cast<const BYTE*>( pData ) + dvbInfo.m_dwVertexStride * uNumPrimsPossible;
                break;
            case D3DPT_LINELIST:
                pData = static_cast<const BYTE*>( pData ) + dvbInfo.m_dwVertexStride * ( uNumPrimsPossible * 2 );
                break;
            case D3DPT_TRIANGLELIST:
                pData = static_cast<const BYTE*>( pData ) + dvbInfo.m_dwVertexStride * ( uNumPrimsPossible * 3 );
                break;
            default:
                return;
            }//switch

            uNumPrimsPossible = _ConvertToNumPrimitives( d3dPrimitiveType, dvbInfo.m_uNumVertices - uCurVertexIndex );
            if ( uNumPrimsPossible > m_uNumMaxPrimitiveCount )
                uNumPrimsPossible = m_uNumMaxPrimitiveCount;
            if ( d3dPrimitiveType == D3DPT_TRIANGLESTRIP && uNumPrimsPossible < uNumPrimitives )
                uNumPrimsPossible = ( uNumPrimsPossible / 2 ) * 2;
            if ( uNumPrimsPossible == 0 )
            {
                uCurVertexIndex = 0;
                uNumPrimsPossible = _ConvertToNumPrimitives( d3dPrimitiveType, dvbInfo.m_uNumVertices );
                if ( uNumPrimsPossible > m_uNumMaxPrimitiveCount )
                    uNumPrimsPossible = m_uNumMaxPrimitiveCount;
                if ( d3dPrimitiveType == D3DPT_TRIANGLESTRIP && uNumPrimsPossible < uNumPrimitives )
                    uNumPrimsPossible = ( uNumPrimsPossible / 2 ) * 2;
                if ( uNumPrimsPossible == 0 )
                    return;
            }
        }

        uNumVertices = _ConvertToNumVertices( d3dPrimitiveType, uNumPrimitives );
        ASSERT( uNumVertices <= dvbInfo.m_uNumVertices );
        if ( uNumVertices == 0 || uNumVertices > dvbInfo.m_uNumVertices )
            return;
    }//if

//out:

    HRESULT hr = dvbInfo.m_pVB->Lock( uCurVertexIndex * dvbInfo.m_dwVertexStride
        , uNumVertices * dvbInfo.m_dwVertexStride, &pLockedVertex, ( uCurVertexIndex == 0 ) ? D3DLOCK_DISCARD : D3DLOCK_NOOVERWRITE );
    if ( FAILED( hr ) )
        return;
    ASSERT( pLockedVertex != NULL );
    bool    bExceptionOccurred = false;
    if ( pLockedVertex != NULL )
    {
        bExceptionOccurred = KExceptionSafeUtil::ESCopyMemory( pLockedVertex, pData, uNumVertices * dvbInfo.m_dwVertexStride );                
    }//if
    dvbInfo.m_pVB->Unlock();
    if ( bExceptionOccurred == true )
    {
        m_uNumExceptionsOccurred++;
        if ( m_uNumExceptionsOccurred >= MAX_NUM_EXCEPTIONS_IN_ONE_FRAME )
        {
            DXUTSetDeviceLost( true );
        }//if
        return;
    }//if

    if ( uNumDraws == 1 )
        g_pKTDXApp->GetDevice()->DrawPrimitive( d3dPrimitiveType, uCurVertexIndex, uNumPrimitives );
    else
    {
        for( unsigned u = 0; u != uNumDraws; u++ )
        {
            g_pKTDXApp->GetDevice()->DrawPrimitive( d3dPrimitiveType, uCurVertexIndex, uNumPrimitives );
        }//for
    }//for
    dvbInfo.m_uCurVertexIndex = uCurVertexIndex + uNumVertices;

}//CKTDGDynamicVBManager::_DrawPrimitiveDynamicVBWithTryCatch()



CKTDGDynamicVBManager::DynamicVBInfo::~DynamicVBInfo()
{
    SAFE_RELEASE( m_pVB );

}//CKTDGDynamicVBManager::DynamicVBInfo::~DynamicVBInfo()


void    CKTDGDynamicVBManager::DynamicVBInfo::OnResetDevice( EDYNAMIC_VB_MODE eMode )
{
    SAFE_RELEASE( m_pVB );
    m_uCurVertexIndex  = 0;

    if ( IsRegistered() == false )
        return;

    if ( g_pKTDXApp == NULL || g_pKTDXApp->GetDevice() == NULL )
        return;

    DWORD   dwD3DUsage = 0;
    D3DPOOL eD3DPool = D3DPOOL_DEFAULT;

    switch( eMode )
    {
    case DYNAMIC_VB_MODE_MANAGED:
        dwD3DUsage = D3DUSAGE_WRITEONLY;
        eD3DPool = D3DPOOL_MANAGED;
        break;
    case DYNAMIC_VB_MODE_PRIMITIVEUP:
        return;
    case DYNAMIC_VB_MODE_DYNAMICVB:
    case DYNAMIC_VB_MODE_DYNAMICVB_WITH_TRY_CATCH:
        dwD3DUsage = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;
        eD3DPool = D3DPOOL_DEFAULT;
        break;
    default:
        return;
    }//switch

    HRESULT hr = g_pKTDXApp->GetDevice()->CreateVertexBuffer( m_uNumVertices * m_dwVertexStride, 
		    dwD3DUsage, m_dwFVF, 
		    eD3DPool, &m_pVB, NULL );
    ASSERT( SUCCEEDED( hr ) );

}//CKTDGDynamicVBManager::DynamicVBInfo::OnResetDevice()


void    CKTDGDynamicVBManager::DynamicVBInfo::OnLostDevice( EDYNAMIC_VB_MODE )
{
    SAFE_RELEASE( m_pVB );
    m_uCurVertexIndex = 0;

}//CKTDGDynamicVBManager::DynamicVBInfo::OnLostDevice()
