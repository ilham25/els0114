#include "StdAfx.h"
#include "X2EmblemManager.h"

#ifdef NEW_EMBLEM_MANAGER

CX2EmblemManager* CX2EmblemManager::m_pEmblemManager = NULL;
/*static */CX2EmblemManager* CX2EmblemManager::GetInstance()
{
	if( NULL == m_pEmblemManager)
		m_pEmblemManager = new CX2EmblemManager;

	return m_pEmblemManager;
}

/*static*/ void CX2EmblemManager::ResetEmblemManager()
{
	DestroyInstance();
	GetInstance()->OpenScriptFile(L"Emblem_Manager.lua");;
}

/*static*/ void CX2EmblemManager::DestroyInstance()
{
	SAFE_DELETE( m_pEmblemManager );
}

CX2EmblemManager::CX2EmblemManager():
m_bHasNextPlayEmblem( false ),
m_hCurrentEmblem(INVALID_PARTICLE_HANDLE),
m_fWaitTime(0.f)
{

}

CX2EmblemManager::~CX2EmblemManager()
{
	BOOST_FOREACH( EmblemTemplet* pEmblem, m_vecEmblemTemplet)
	{
		SAFE_DELETE ( pEmblem );
	}
}


bool CX2EmblemManager::OpenScriptFile( IN const WCHAR* pFileName_ )
{
	KLuaManager luaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState(), 0, false );
	lua_tinker::decl( g_pKTDXApp->GetLuaBinder()->GetLuaState(),  "g_pEmblemManager", this );

	KGCMassFileManager::CMassFile::MASSFILE_MEMBERFILEINFO_POINTER Info;
	Info = g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LoadDataFile( pFileName_ );
	if ( Info == NULL )
	{
		std::string strFileName;
		ConvertWCHARToChar( strFileName, pFileName_ );
		ErrorLogMsg( XEM_ERROR149, strFileName.c_str() );

		return false;
	}

	if( g_pKTDXApp->GetLuaBinder()->DoMemory( Info->pRealData, Info->size ) == E_FAIL )
	{
		string strFileName;
		ConvertWCHARToChar( strFileName, pFileName_ );
		ErrorLogMsg( XEM_ERROR149, strFileName.c_str() );

		return false;
	}
	else
	{
		ParsingEmblemTemplet( luaManager );
	}

	return true;
}
/** @function : ParsingEmblemTemplet 
	@brief : 기본 스크립트 파싱
*/
void CX2EmblemManager::ParsingEmblemTemplet( KLuaManager& luaManager )
{
	// 파싱할 엠블럼ID에 대한 리스트 구하기
	vector<std::string> vecEmblemTempletName;
	if( true == luaManager.BeginTable( L"SHOW_EMBLEM_ID_LIST" ) )
	{
		int index	= 1; 
		std::string strValue = "";
		while( luaManager.GetValue( index, strValue ) == true )
		{
			if( false == strValue.empty() )
				vecEmblemTempletName.push_back(strValue);
			index++;
		}
		luaManager.EndTable();
	}

	BOOST_FOREACH( const std::string& strTableName, vecEmblemTempletName )
	{
		if( true == luaManager.BeginTable( strTableName.c_str() ) )
		{
			EmblemTemplet* pEmblemTemplet = new EmblemTemplet;

			LUA_GET_VALUE_ENUM( luaManager, "EMBLEM_ID", pEmblemTemplet->m_eEmblemID, EMBLEM_ID, EI_NONE);

			bool bCheckDuplicate = false;
			BOOST_FOREACH( const EmblemTemplet* pEmblem, m_vecEmblemTemplet)
			{
				if( pEmblem->m_eEmblemID == pEmblemTemplet->m_eEmblemID )
				{
			#ifdef _IN_HOUSE_
					ASSERT(!"Duplicate Emblem Templet");
			#endif // _IN_HOUSE_
					bCheckDuplicate = true;
					break;
				}
			}
			if( true == bCheckDuplicate )
				continue;

			LUA_GET_VALUE( luaManager, "SHOW_STATE_VILLEAGE",		pEmblemTemplet->m_bShowVilleage, false );
			LUA_GET_VALUE( luaManager, "SHOW_STATE_BATTLE_FIELD",	pEmblemTemplet->m_bShowBattleFIeld, false );
			LUA_GET_VALUE( luaManager, "SHOW_STATE_DUNGEON_GAME",	pEmblemTemplet->m_bShowDungeon, false );

			LUA_GET_VALUE( luaManager, "IS_FIRST_ONLY",				pEmblemTemplet->m_bIsFirstOnly, false );
			LUA_GET_VALUE( luaManager, "CHECK_CONDITION",			pEmblemTemplet->m_bCheckCondition, false );

			LUA_GET_VALUE( luaManager, "SHOW_ORDER",			pEmblemTemplet->m_uiShowOrder, 0 );



			// 이펙트 생성 관련 정보 파싱
			LUA_GET_VALUE( luaManager, "TEXTUR_FILE_NAME",		pEmblemTemplet->m_wstrTextureFileName, L"" );

			LUA_GET_VALUE( luaManager, "DURATION_TIME",			pEmblemTemplet->m_vecTime.x, 0.f );
			LUA_GET_VALUE( luaManager, "FADE_IN_TIME",			pEmblemTemplet->m_vecTime.y, 0.f );
			LUA_GET_VALUE( luaManager, "FADE_OUT_TIME",			pEmblemTemplet->m_vecTime.z, 0.f );

			m_vecEmblemTemplet.push_back( pEmblemTemplet );

			luaManager.EndTable();
		}
		else
		{
#ifdef _IN_HOUSE_
			ASSERT(!"Empty Emblem Templet");
#endif // _IN_HOUSE_
		}
	}

	// 출력 순서 기준에 맞게 정렬
	auto SortFunc = [](const EmblemTemplet* lhs_, const EmblemTemplet* rhs_ )->bool
	{
		return rhs_->m_uiShowOrder > lhs_->m_uiShowOrder;
	};	
	std::sort( m_vecEmblemTemplet.begin(), m_vecEmblemTemplet.end(), SortFunc );

}

/** @function : CreateEventSequence 
	@brief : 스크립트 정보를 토대로 파티클 시퀀스 생성
	@param : wstrTextureFile_ : 텍스쳐 파일명
			 vTime.x : LifeTime
			 vTime.y : Fade In Time
			 vTime.z : Fade Out Time
*/
CKTDGParticleSystem::CParticleEventSequence* CX2EmblemManager::CreateEventSequence( const WCHAR* wstrTextureFile_, 
	const D3DXVECTOR3& vTime)
{
	if( NULL == g_pData || NULL == g_pData->GetUIMajorParticle() )
		return NULL;

	CKTDGParticleSystem::CParticleEventSequence* pParticleSequence= 
		CKTDGParticleSystem::CParticleEventSequence::CreateParticleEventSequence( g_pData->GetUIMajorParticle(), NULL );

	pParticleSequence->SetParticleType(CKTDGParticleSystem::PT_2D_PLANE);
	pParticleSequence->SetLayer(21);
	pParticleSequence->SetSrcBlendMode(D3DBLEND_SRCALPHA);
	pParticleSequence->SetDestBlendMode(D3DBLEND_INVSRCALPHA);
	pParticleSequence->SetZEnable(false);
	pParticleSequence->SetResolutionConvert(true);
	pParticleSequence->SetForceLayer(false);
	pParticleSequence->SetMaxParticleNum(1);
	pParticleSequence->SetTriggerCount(1);

	// 전체 지속시간은 스크립트로부터 얻은 값으러 설정
	pParticleSequence->SetLifetime(CMinMax<float>(vTime.x, vTime.x));
	pParticleSequence->SetEmitRate(1000.f,1000.f);
	//pParticleSequence->SetRenderStateID( CKTDGStateManager::RT_REAL_COLOR );

	// 텍스쳐 설정
	CKTDGParticleSystem::CParticleEvent_Texture* pEventTexture = new CKTDGParticleSystem::CParticleEvent_Texture();
	pEventTexture->SetTextureName( wstrTextureFile_ );
	pEventTexture->SetActualTime( CMinMax<float>(0, 0) );
	pEventTexture->SetFade( false );
	pParticleSequence->m_EventList.push_back(pEventTexture);

	// 사이즈 설정
	CKTDXDeviceTexture* pTexture = 
		pTexture = g_pKTDXApp->GetDeviceManager()->OpenTexture( wstrTextureFile_ );
	
	if( NULL != pTexture )
	{
		CKTDGParticleSystem::CParticleEvent_Size* pEventSize = new CKTDGParticleSystem::CParticleEvent_Size();
		D3DXVECTOR3 TextureSize = D3DXVECTOR3( static_cast<float>(pTexture->GetWidth()), 
			static_cast<float>(pTexture->GetHeight()), 1.f);
		pEventSize->SetSize(CMinMax<D3DXVECTOR3> (TextureSize, TextureSize) );
		pEventSize->SetActualTime( CMinMax<float>(0, 0) );
		pEventSize->SetFade( false );
		pParticleSequence->m_EventList.push_back(pEventSize);
	}

	// 컬러 설정
	// Initialize
	CKTDGParticleSystem::CParticleEvent_Color* pEventColor = new CKTDGParticleSystem::CParticleEvent_Color();
	pEventColor->SetColor(CMinMax<D3DXCOLOR>( D3DXCOLOR(1,1,1,0), D3DXCOLOR(1,1,1,0)));
	pEventColor->SetActualTime( CMinMax<float>(0, 0) );
	pEventColor->SetFade( false );
	pParticleSequence->m_EventList.push_back(pEventColor);

	// Fade In
	pEventColor = new CKTDGParticleSystem::CParticleEvent_Color();
	pEventColor->SetColor(CMinMax<D3DXCOLOR>( D3DXCOLOR(1,1,1,1), D3DXCOLOR(1,1,1,1)));
	pEventColor->SetActualTime( CMinMax<float>(0.f, vTime.y) );
	pEventColor->SetFade( true );
	pParticleSequence->m_EventList.push_back(pEventColor);

	// Fade out
	pEventColor = new CKTDGParticleSystem::CParticleEvent_Color();
	pEventColor->SetColor(CMinMax<D3DXCOLOR>( D3DXCOLOR(1,1,1,0), D3DXCOLOR(1,1,1,0)));
	pEventColor->SetActualTime( CMinMax<float>( (vTime.x - vTime.z ), vTime.x) );
	pEventColor->SetFade( true );
	pParticleSequence->m_EventList.push_back(pEventColor);

	return pParticleSequence;
}
/** @function : PlayEmblemEffect 
	@brief : 이펙트 생성 함수
*/
void CX2EmblemManager::PlayEmblemEffect( const EmblemTemplet& sEmblemTemplet_ )
{
#ifdef DIALOG_SHOW_TOGGLE
	// 오현빈 // 2013-07-24
	// 스샷 모드에서는 엠블럼 출력시키지 않기.
	// 별다른 예외 처리 없이 보이지만 않도록 처리
	if( g_pKTDXApp->GetDGManager()->GetDialogManager()->GetHideDialog() == true )
		return;
#endif // DIALOG_SHOW_TOGGLE

	if( NULL == g_pData || NULL == g_pData->GetUIMajorParticle() )
		return;

	CKTDGParticleSystem::CParticleEventSequence* pSequence = 
		CreateEventSequence( sEmblemTemplet_.m_wstrTextureFileName.c_str(), sEmblemTemplet_.m_vecTime );
	pSequence->SetOverUI( true );

	if( NULL != pSequence )
	{
		m_hCurrentEmblem = 
			g_pData->GetUIMajorParticle()->CreateInstanceNonTemplet(pSequence, 
				D3DXVECTOR3(500,300,0), D3DXVECTOR2(-1,-1), D3DXVECTOR2(-1,-1) ); 

	}
}

/**	@function : InitStateChange 
	@biref : 엠블럼 진행 관련 정보 초기화
	@param : bCharChange : 캐릭터 변경 초기화라면 모두 초기화 하기
*/
void CX2EmblemManager::InitEmbelmPlayInfo( bool bCharChange /*= false */)
{
	BOOST_FOREACH( EmblemTemplet* pEmblem, m_vecEmblemTemplet)
	{
		if( false == pEmblem->m_bIsFirstOnly ||
			true == bCharChange )
		{
			pEmblem->InitShowState();
		}
		pEmblem->InitIsPassCondition();
	}
	m_bHasNextPlayEmblem = true;

	// 출력 중인 엠블럼이 있다면 종료
	if( INVALID_PARTICLE_HANDLE != m_hCurrentEmblem &&
		NULL != g_pData->GetUIMajorParticle() )
	{
		g_pData->GetUIMajorParticle()->DestroyInstanceHandle( m_hCurrentEmblem );
	}

	m_fWaitTime = 3.f;
}

void CX2EmblemManager::OnFrameMove( float fElpaseTime )
{
	if( NULL != g_pData->GetCashShop() &&
		true == g_pData->GetCashShop()->GetOpen() )
	{	
		// 출력 중인 엠블럼이 있다면 종료
		if( INVALID_PARTICLE_HANDLE != m_hCurrentEmblem &&
			NULL != g_pData->GetUIMajorParticle() )
		{
			g_pData->GetUIMajorParticle()->DestroyInstanceHandle( m_hCurrentEmblem );
		}
		return;
	}

	if( m_fWaitTime > 0 )
	{
		m_fWaitTime -= fElpaseTime;
		return;
	}

	if( NULL == g_pData->GetUIMajorParticle() )
		return;

	// 보여줘야 할 엠블럼이 하나도 없다면 return;
	if( false == m_bHasNextPlayEmblem )
		return;

	// 진행중인 엠블럼이 있다면 return;
	if( INVALID_PARTICLE_HANDLE != m_hCurrentEmblem &&
		true == g_pData->GetUIMajorParticle()->IsLiveInstanceHandle( m_hCurrentEmblem ))
	{
		return;
	}

	m_hCurrentEmblem = INVALID_PARTICLE_HANDLE;
	BOOST_FOREACH( EmblemTemplet* pEmblem, m_vecEmblemTemplet)
	{
		// 스테이트 체크
		switch( g_pMain->GetNowStateID() )
		{
		case CX2Main::XS_VILLAGE_MAP:
			{
				if( false == pEmblem->m_bShowVilleage )
					continue;
			}
			break;
		case CX2Main::XS_DUNGEON_GAME:
			{
				if( false == pEmblem->m_bShowDungeon )
					continue;

				if( true == g_pMain->GetIsPlayingTutorial() )
				{
					pEmblem->m_bIsAlreadyShow = true;
					continue;
				}
			}
			break;
		case CX2Main::XS_BATTLE_FIELD:
			{
				if( false == pEmblem->m_bShowBattleFIeld )
					continue;
			}
			break;
		default:
			return;
			break;
		}

		if( false == pEmblem->m_bIsAlreadyShow && // 아직 보여주지 않았고
			true == pEmblem->m_bIsPassCondition ) // 조건 통과 했다면
		{
			pEmblem->m_bIsAlreadyShow = true;

			PlayEmblemEffect(*pEmblem);
			PlayEmblemOtehrProcess(*pEmblem);
			CheckNextPlayingEmblem();
			break;
		}
	}
}
/** @function : CheckNextPlayingEmblem
	@brief : 다음에 출력 할 엠블럼이 있는지 체크
*/
void CX2EmblemManager::CheckNextPlayingEmblem()
{
	bool bHasNextPlayEmblem = false;
	BOOST_FOREACH( const EmblemTemplet* pEmblem, m_vecEmblemTemplet)
	{
		// 아직 출력하지 않은 이펙트가 있다면
		if( false == pEmblem->m_bIsAlreadyShow )
			bHasNextPlayEmblem = true;
	}

	m_bHasNextPlayEmblem = bHasNextPlayEmblem;
}
/** @function : PlayEmblem 
	@brief : 조건이 설정된 엠블럼의 조건을 활성화 시켜주는 함수
*/
void CX2EmblemManager::PlayEmblem( EMBLEM_ID eEmblemID_ )
{	
	BOOST_FOREACH( EmblemTemplet* pEmblem, m_vecEmblemTemplet)
	{
		if( eEmblemID_ == pEmblem->m_eEmblemID )
		{
			pEmblem->m_bIsPassCondition = true;
			return;
		}
	}
}
void CX2EmblemManager::PlayEmblemOtehrProcess( const EmblemTemplet& sEmblemTemplet_ )
{
	switch( sEmblemTemplet_.m_eEmblemID )
	{
	case EI_EVENT_ELESIS:
		{
			if ( NULL != g_pX2Game )
			{
				g_pX2Game->GetMyUnit()->SetBuffFactorToGameUnitByBuffFactorID( BFI_EMPTY_EXP_BUFF );
			}
		} break;
	default:
		{
		} break;
	}
}
bool CX2EmblemManager::HasEmblem( EMBLEM_ID eEmblemID_ )
{	
	BOOST_FOREACH( const EmblemTemplet* pEmblem, m_vecEmblemTemplet)
	{
		if( eEmblemID_ == pEmblem->m_eEmblemID )
		{
			return true;
		}
	}
	return false;
}
#endif // NEW_EMBLEM_MANAGER


