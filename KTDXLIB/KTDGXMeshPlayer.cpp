#include "StdAfx.h"
#include ".\ktdgxmeshplayer.h"


CKTDGXMeshPlayer::CXMeshInstanceHandle CKTDGXMeshPlayer::s_iNextMeshInstanceHandle = 0;




//{{ robobeg : 2008-10-13
static const CKTDGStateManager::KState s_akStates[] = 
{
		KRenderState( D3DRS_ALPHABLENDENABLE,	TRUE ),
		KRenderState( D3DRS_CULLMODE,			D3DCULL_NONE ),
		KTextureStageState( 0, D3DTSS_ALPHAOP,		D3DTOP_MODULATE ),
};//s_akStates[]
//}} robobeg : 2008-10-13


CKTDGXMeshPlayer::CKTDGXMeshPlayer()
{
	InitializeCriticalSection( &m_csMeshPlayerLock );

//{{ robobeg : 2008-10-13
	m_RenderStateID = s_akStates;
//}} robobeg : 2008-10-13

	m_bEnable = true;
	m_bEnableSlashTrace = true;

	m_InstanceList.reserve( 64 );

	m_fElapsedTime = 0.f;
#ifdef NOT_RENDER_EFFECT_MADE_BY_GAME_UNIT
	m_bRenderEffectMadeByGameUnit = true;
#endif //NOT_RENDER_EFFECT_MADE_BY_GAME_UNIT

#ifdef SHOW_ATTACK_BOX_DUMMY
	m_bShowAttackBox = false;
#endif // SHOW_ATTACK_BOX_DUMMY
}

CKTDGXMeshPlayer::~CKTDGXMeshPlayer(void)
{
	DestroyAllInstance();

	EnterCriticalSection( &m_csMeshPlayerLock );
	map<wstring,XMeshTemplet*>::iterator iter;
	XMeshTemplet* pInst;

	for( iter = m_TempletMap.begin(); iter != m_TempletMap.end(); iter++ )
	{
		pInst = (XMeshTemplet*)iter->second;
		SAFE_DELETE( pInst );
	}
	m_TempletMap.clear();
	LeaveCriticalSection( &m_csMeshPlayerLock );

	DeleteCriticalSection( &m_csMeshPlayerLock );
}

void CKTDGXMeshPlayer::OpenScriptFile( const WCHAR* pFilename )
{
	////CKTDXThread::CLocker locker( m_csMeshPlayerLock );

	KGCMassFileManager::CMassFile::MASSFILE_MEMBERFILEINFO_POINTER Info;
	Info = g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LoadDataFile( pFilename );
	if( Info == NULL )
	{
		string strName;
		ConvertWCHARToChar(strName, pFilename );
		ErrorLogMsg( KEM_ERROR43, strName.c_str() );
		return;
	}

	char* pBuffer = Info->pRealData;

#ifdef _ENCRIPT_SCRIPT_
	pBuffer = XORDecrypt( pBuffer, Info->size );
#endif


#ifdef _ENCRIPT_SCRIPT_
	Compile( pBuffer, -1 );
	SAFE_DELETE_ARRAY( pBuffer );	 // XORDecrypt() 함수에서 할당된 메모리 해제
#else
	Compile( pBuffer, Info->size );
#endif _ENCRIPT_SCRIPT_
}


bool CKTDGXMeshPlayer::Compile( const char *pScript, const int iLength/* = -1*/ )
{
	m_strLastError = "0 error(s), you're good to go!";

	try 
	{
		CXMeshPlayerTokenizer tokenizer;

		// parse the character string into tokens.
		tokenizer.Tokenize( pScript, iLength );

		std::vector<CXMeshPlayerToken>::iterator TokenIter	= tokenizer.m_TokenVector.begin();
		std::vector<CXMeshPlayerToken>::iterator EndIter		= tokenizer.m_TokenVector.end();

		if (tokenizer.m_TokenVector.size() < 2) throw("This script is too small to be valid.");

		// the first three tokens out of the gate should be
		// MeshPlayer, followed by a name and version number, then
		// an open brace.
		if (TokenIter->m_Type != KeywordXMeshPlayer) { throw("First word must be XMeshPlayer"); }
		CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);
		if (TokenIter->m_Type != Quote) { throw("Must name XMeshPlayer"); }
		RemoveQuotes(TokenIter->m_strValue);
		ConvertCharToWCHAR( m_Name, TokenIter->m_strValue.c_str() );
		CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);

		if (TokenIter->m_Type != RealNumber) { throw("Must have version number"); }
		CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);
		if (TokenIter->m_Type != OpenBrace) { throw("Missing opening brace for XMeshPlayer block"); }

		ProcessXMeshPlayerBlock(TokenIter, EndIter);
	} 
	catch(char *e) 
	{
		m_strLastError = e; 

		WCHAR wFileName[255] = {0,};
		MultiByteToWideChar( CP_ACP, 0, m_strLastError.c_str(), -1, wFileName, 255 );

		OutputDebugString(wFileName);
		MessageBox( NULL, wFileName, NULL, NULL );
		return(false);
	} 
	catch(const char *e) 
	{
		m_strLastError = e; 
		WCHAR wFileName[255] = {0,};
		MultiByteToWideChar( CP_ACP, 0, m_strLastError.c_str(), -1, wFileName, 255 );

		OutputDebugString(wFileName);
		MessageBox( NULL, wFileName, NULL, NULL );
		return(false);
	} 
	catch (...) 
	{
		m_strLastError = "Unhandled exception in CXMeshPlayer::Compile.";
		WCHAR wFileName[255] = {0,};
		MultiByteToWideChar( CP_ACP, 0, m_strLastError.c_str(), -1, wFileName, 255);

		OutputDebugString(wFileName);
		MessageBox( NULL, wFileName, NULL, NULL );
		return(false);
	}

	return true;
}



// note!! 랜덤성을 추가하자
CKTDGXMeshPlayer::CXMeshInstance* CKTDGXMeshPlayer::CreateInstanceParabolic( CKTDGObject* pKTDGObject, const WCHAR* pTempletName, const D3DXVECTOR3& vPos, const D3DXVECTOR3& vTargetPos, 
																			const D3DXVECTOR3& vAcceleration, const float fTimeToReachTarget, const float fTimeToLive )
{
	// 이동거리 = 초기속도 * 시간 + 가속도 * 시간의 제곱 * 0.5
	D3DXVECTOR3 vDistance = vTargetPos - vPos;
	D3DXVECTOR3 vInitialSpeed;
	vInitialSpeed.x = vDistance.x - vAcceleration.x * fTimeToReachTarget * fTimeToReachTarget * 0.5f;
	vInitialSpeed.y = vDistance.y - vAcceleration.y * fTimeToReachTarget * fTimeToReachTarget * 0.5f;
	vInitialSpeed.z = vDistance.z - vAcceleration.z * fTimeToReachTarget * fTimeToReachTarget * 0.5f;
	vInitialSpeed /= fTimeToReachTarget;

	float fSpeed = D3DXVec3Length( &vInitialSpeed );
	D3DXVECTOR3 vRotation = GetDirVecToDegree( vInitialSpeed );
	
	CKTDGXMeshPlayer::CXMeshInstance* pMeshPlayerInstance = CreateInstance( pKTDGObject, pTempletName, vPos, vRotation, vRotation, -1 );
	if( NULL == pMeshPlayerInstance )
		return NULL;

	pMeshPlayerInstance->SetMaxLifeTime( fTimeToLive );
	pMeshPlayerInstance->SetVelocity( D3DXVECTOR3( fSpeed, 0, 0 ) );
	pMeshPlayerInstance->SetGravity( vAcceleration );
	pMeshPlayerInstance->SetParabolicCurve( true );		

	pMeshPlayerInstance->OnFrameMove( 0.f, 0.0001f );

	return pMeshPlayerInstance;
}

#ifdef EVENT_SCENE
//{{ oasis907 : 김상윤 [2010.6.10] // 
// txt에서 파싱된 Templet들이 등록된 TempletMap과 상관없이 동작할 수 있게금 하였습니다.
// 대신 pTemplet의 내용은 다른 곳에서 수동으로 입력해야합니다. 
CKTDGXMeshPlayer::CXMeshInstance* CKTDGXMeshPlayer::CreateInstanceNonTemplet( CKTDGXMeshPlayer* pMeshPlayer, XMeshTemplet* pTemplet, 
								 const D3DXVECTOR3& pos, const D3DXVECTOR3& angleDegree, const D3DXVECTOR3& moveAxisAngleDegree )
{

	CKTDGXMeshPlayer::CXMeshInstance* pXmesh = CKTDGXMeshPlayer::CXMeshInstance::CreateMeshInstance(pMeshPlayer, pTemplet, pos, angleDegree, moveAxisAngleDegree );


	pXmesh->OnFrameMove( 0.f, 0.0001f );

	pXmesh->SetRenderStateID( m_RenderStateID );

	// 핸들 등록
	pXmesh->SetHandle( s_iNextMeshInstanceHandle );
	s_iNextMeshInstanceHandle++;
	if( s_iNextMeshInstanceHandle > INT_MAX-2 )
	{
		s_iNextMeshInstanceHandle = 0;
	}

	// 렌더 체인에 넣기
	g_pKTDXApp->GetDGManager()->AddObjectChain( pXmesh );

	m_InstanceList.push_back( pXmesh );

	ASSERT( m_mapInstance.find( pXmesh->GetHandle() ) == m_mapInstance.end() );
	m_mapInstance[ pXmesh->GetHandle() ] = pXmesh;


	return pXmesh;
}
//}}
#endif EVENT_SCENE


CKTDGXMeshPlayer::CXMeshInstance* CKTDGXMeshPlayer::CreateInstance( CKTDGObject* pKTDGObject, 
																	const WCHAR* pTempletName, 
																	float x, float y, float z, 
																	float angleXDegree, float angleYDegree, float angleZDegree,
																	float moveAxisXDegree, float moveAxisYDegree, float moveAxisZDegree,
																	int layer )
{
	////CKTDXThread::CLocker locker( m_csMeshPlayerLock );

	ASSERT( NULL != pTempletName );

#ifdef NO_MORE_ERROR_TEST
	if( NULL == pTempletName )
	{
		InHouseLog( "이름이 없는 meshplayer 생성 시도" );
	}
#endif NO_MORE_ERROR_TEST

	if( pTempletName == NULL )
		return NULL;


	if( m_bEnable == false )
	{
		return NULL;
	}

	map<wstring,XMeshTemplet*>::iterator iter = m_TempletMap.find( pTempletName );
	if( iter == m_TempletMap.end() )
	{
		return NULL;
	}


	XMeshTemplet* pTemplet = (XMeshTemplet*)iter->second;

	if( false == IsValidTemplet(pTemplet) )
		return NULL;
	
	CXMeshInstance* pInstance = CXMeshInstance::CreateMeshInstance( this, pTemplet, D3DXVECTOR3(x,y,z), 
		D3DXVECTOR3(angleXDegree,angleYDegree,angleZDegree),
		D3DXVECTOR3(moveAxisXDegree,moveAxisYDegree,moveAxisZDegree) );

	if( pInstance->GetXSkinMesh() == NULL && pInstance->GetXMesh() == NULL )
	{
		SAFE_DELETE_KTDGOBJECT( pInstance );
		return NULL;
	}


#ifdef GIANT_UNIT_GIANT_EFFECT_TEST
	if( NULL != pKTDGObject )
	{
		switch( pKTDGObject->GetObjectType() )
		{
		case CKTDGObject::OT_CX2GUUser:
		case CKTDGObject::OT_CX2GUNPC:
			{
				// 게임 유닛에 의해 생성된 이펙트인 경우 타입 지정(거리에 따라서 DRAW 하지 않을 이펙트를 판단 하는데 사용)
				pInstance->SetObjectType( CKTDGObject::OT_EFFECT_MADE_BY_GAME_UNIT );

		#ifdef PARTICLE_NOTAPPLY_UNITSCALE
				if( pInstance->GetApplyUnitScale() == true )
					pInstance->SetScaleByUnit( pKTDGObject->GetScaleByUnit() );
		#else
				pInstance->SetScaleByUnit( pKTDGObject->GetScaleByUnit() );
		#endif
			} break;
		}
	}
#endif GIANT_UNIT_GIANT_EFFECT_TEST

	pInstance->OnFrameMove( 0.f, 0.0001f );

	pInstance->SetRenderStateID( m_RenderStateID );
	if( layer != -1 )
		pInstance->SetLayer( layer );

	pInstance->SetHandle( s_iNextMeshInstanceHandle );
	s_iNextMeshInstanceHandle++;
	if( s_iNextMeshInstanceHandle > INT_MAX-2 )
	{
		s_iNextMeshInstanceHandle = 0;
	}

	g_pKTDXApp->GetDGManager()->AddObjectChain( pInstance );
	m_InstanceList.push_back( pInstance );

	ASSERT( m_mapInstance.find( pInstance->GetHandle() ) == m_mapInstance.end() );
	m_mapInstance[ pInstance->GetHandle() ] = pInstance;

	return pInstance;
}

HRESULT CKTDGXMeshPlayer::OnFrameMove( double fTime, float fElapsedTime )
{
	KTDXPROFILE();
	////CKTDXThread::CLocker locker( m_csMeshPlayerLock );

	m_fElapsedTime = fElapsedTime;

	for( int i = 0; i < (int)m_InstanceList.size(); i++ )
	{
		CXMeshInstance* pInstance = m_InstanceList[i];
		pInstance->OnFrameMove( fTime, fElapsedTime );
		if( pInstance->GetState() == IS_DIE )
		{
			m_mapInstance.erase( pInstance->GetHandle() );
			SAFE_DELETE_KTDGOBJECT( pInstance );

			m_InstanceList.erase( m_InstanceList.begin() + i );
			i--;
		}
	}

	return S_OK;
}

void CKTDGXMeshPlayer::DestroyAllInstance()
{
	////CKTDXThread::CLocker locker( m_csMeshPlayerLock );

	for( int i = 0; i < (int)m_InstanceList.size(); i++ )
	{
		SAFE_DELETE_KTDGOBJECT( m_InstanceList[i] );
	}
	m_InstanceList.resize(0);
	m_mapInstance.clear();
}


void CKTDGXMeshPlayer::DestroyInstance( CXMeshInstanceHandle& handle )
{
	if( INVALID_MESH_INSTANCE_HANDLE == handle )
		return;

	if( true == m_mapInstance.empty() )
	{
		handle = INVALID_MESH_INSTANCE_HANDLE;
		return;
	}


	CXMeshInstance* pMeshInst = GetMeshInstance( handle );
	if( NULL != pMeshInst )
	{
		m_mapInstance.erase( handle );

		for( int i = 0; i < (int)m_InstanceList.size(); i++ )
		{
			CXMeshInstance* pTempInst = m_InstanceList[i];
			if( pMeshInst == pTempInst )
			{
				SAFE_DELETE_KTDGOBJECT( pMeshInst );
				m_InstanceList.erase( m_InstanceList.begin() + i );
				break;
			}
		}
	}

	handle = INVALID_MESH_INSTANCE_HANDLE;
}




bool CKTDGXMeshPlayer::IsLiveInstanceHandle( const CXMeshInstanceHandle handle )
{
	if( INVALID_MESH_INSTANCE_HANDLE == handle )
		return false;

	if( true == m_mapInstance.empty() )
		return false;


	std::map< CXMeshInstanceHandle, CXMeshInstance* >::iterator it = m_mapInstance.find( handle );
	if( it != m_mapInstance.end() )
	{
		return true;
	}

	return false;
}




CKTDGXMeshPlayer::CXMeshInstance* CKTDGXMeshPlayer::GetMeshInstance( CXMeshInstanceHandle handle )
{
	if( INVALID_MESH_INSTANCE_HANDLE == handle )
		return NULL;

	if( true == m_mapInstance.empty() )
		return NULL;


	std::map< CXMeshInstanceHandle, CXMeshInstance* >::iterator it = m_mapInstance.find( handle );
	if( it != m_mapInstance.end() )
	{
		return it->second;
	}

	return NULL;
}






bool CKTDGXMeshPlayer::ProcessXMeshPlayerBlock( std::vector<CXMeshPlayerToken>::iterator &TokenIter,
												std::vector<CXMeshPlayerToken>::iterator &EndIter )
{
	bool StartedProcessingTemplet = false;
	// move past the opening brace...
	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);

	while (TokenIter->m_Type != CloseBrace)
	{
		CXMeshPlayerToken savedtoken;
		savedtoken = *TokenIter;

		switch (TokenIter->m_Type) 
		{
			case KeywordTemplet:
				{
					StartedProcessingTemplet = true;
					XMeshTemplet *newTemplet = new XMeshTemplet();

					if( !ProcessTempletBlock( *newTemplet, TokenIter, EndIter ) ) 
					{
						m_TempletMap.erase( newTemplet->templetName );
					}
				}
				break;

			default: 
				{
					char buf[256]; 
		#ifdef	CONVERSION_VS
					_snprintf_s(buf, sizeof(buf), _countof(buf), "Unexpected \"%s\" in XMeshPlayer Block!", TokenIter->m_strValue.c_str());
		#else	CONVERSION_VS
					_snprintf(buf, sizeof(buf), "Unexpected \"%s\" in XMeshPlayer Block!", TokenIter->m_strValue.c_str());
		#endif	CONVERSION_VS
					
					throw(&buf[0]);
				}
				break;
		}
	}
	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);
	return(true);
}

bool CKTDGXMeshPlayer::ProcessTempletBlock(	XMeshTemplet &templet,
											std::vector<CXMeshPlayerToken>::iterator &TokenIter,
											std::vector<CXMeshPlayerToken>::iterator &EndIter )
{
	bool StartedProcessingEvents = false;

	// move past the event sequence keyword...
	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);

	// next token should be the name of the sequence...
	if (TokenIter->m_Type != Quote) 
		throw("Must name Templet block!");

	ConvertCharToWCHAR( templet.templetName, RemoveQuotes(TokenIter->m_strValue).c_str() );

	// 똑같은 이름의 mesh player templet 이 있다면 지우고 덮어쓴다
	std::map<wstring,XMeshTemplet*>::iterator it = m_TempletMap.find( templet.templetName );
	ASSERT( it == m_TempletMap.end() );
	if( it != m_TempletMap.end() )
	{
		SAFE_DELETE( (XMeshTemplet*)it->second );
#ifdef EFFECT_TOOL
			MessageBox( NULL, templet.templetName.c_str() , L"Duplication" , NULL );
#endif //EFFECT_TOOL
	}
	m_TempletMap[templet.templetName] = &templet;

#ifdef EFFECT_TOOL
	m_TempletVec.push_back( &templet );
#endif //EFFECT_TOOL

	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);

	// next token should be opening brace...
	if (TokenIter->m_Type != OpenBrace) 
		throw("Expected opening brace for Templet block!");
	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);

	while (TokenIter->m_Type != CloseBrace) 
	{

		CXMeshPlayerToken savedtoken;
		savedtoken = *TokenIter;

		// the first token here can be a SysNumberProperty, SysAlphaBlendModeProperty, SysVectorProperty,
		// or an EventSequence.
		switch (TokenIter->m_Type) 
		{
			case SeqLifeTypeProp:
				{
					if (StartedProcessingEvents) 
						throw("Cannot specify any sequence properties after specifying events.");
					LIFE_TYPE lifeType;
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					if (TokenIter->m_Type != Equals) throw("Expected equals sign!");
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					CXMeshPlayerTokenizer::ProcessLifeType(lifeType, TokenIter, EndIter);

					if (savedtoken.IsLifeType())		{ templet.lifeType = lifeType; }
					else if (savedtoken.IsDieType())	{ templet.dieType = lifeType; }
					else 
					{
						throw("Unknown sequence LifeType property!");
					}
				}
				break;

			case SeqMeshTypeProp:
				{
					if (StartedProcessingEvents) 
						throw("Cannot specify any sequence properties after specifying events.");
					MESH_TYPE meshType;
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					if (TokenIter->m_Type != Equals) throw("Expected equals sign!");
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					CXMeshPlayerTokenizer::ProcessMeshType(meshType, TokenIter, EndIter);

					if (savedtoken.IsMeshType())		{ templet.meshType = meshType; }
					else 
					{
						throw("Unknown sequence meshType property!");
					}
				}
				break;

			case SeqRenderTypeProp:
				{
					if (StartedProcessingEvents) 
						throw("Cannot specify any sequence properties after specifying events.");
					CKTDGXRenderer::RENDER_TYPE renderType;
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					if (TokenIter->m_Type != Equals) throw("Expected equals sign!");
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					CXMeshPlayerTokenizer::ProcessRenderType(renderType, TokenIter, EndIter);

					if (savedtoken.IsRenderType())		{ templet.renderType = renderType; }
					else 
					{
						throw("Unknown sequence renderType property!");
					}
				}
				break;

			case SeqCartoonTexTypeProp:
				{
					if (StartedProcessingEvents) 
						throw("Cannot specify any sequence properties after specifying events.");
					CKTDGXRenderer::CARTOON_TEX_TYPE cartoonTexType;
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					if (TokenIter->m_Type != Equals) throw("Expected equals sign!");
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					CXMeshPlayerTokenizer::ProcessCartoonTexType(cartoonTexType, TokenIter, EndIter);

					if (savedtoken.IsCartoonTexType())	{ templet.cartoonTexType = cartoonTexType; }
					else 
					{
						throw("Unknown sequence cartoonTexType property!");
					}
				}
				break;

			case SeqCullModeProp:
				{
					if (StartedProcessingEvents) 
						throw("Cannot specify any sequence properties after specifying events.");
					D3DCULL cullMode;
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					if (TokenIter->m_Type != Equals) throw("Expected equals sign!");
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					CXMeshPlayerTokenizer::ProcessCullMode(cullMode, TokenIter, EndIter);

					if (savedtoken.IsCullMode())	{ templet.cullMode = cullMode; }
					else 
					{
						throw("Unknown sequence cullMode property!");
					}
				}
				break;

			case SeqNumericProp:
				{
					if (StartedProcessingEvents) 
						throw("Cannot specify any sequence properties after specifying events.");
					CMinMax<float> number;
					// the next 2 tokens should be an equals, and a number.
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					if (TokenIter->m_Type != Equals) throw("Expected equals sign!");
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					CXMeshPlayerTokenizer::ProcessNumber(number, TokenIter, EndIter);

					if (savedtoken.IsPlayCount())			{ templet.playCount.m_Min = (int)number.m_Min; templet.playCount.m_Max = (int)number.m_Max; }
					else if (savedtoken.IsLifeTime())		{ templet.lifeTime = number; }
					else if (savedtoken.IsDiePlayCount())	{ templet.diePlayCount.m_Min = (int)number.m_Min; templet.diePlayCount.m_Max = (int)number.m_Max; }
					else if (savedtoken.IsDieLifeTime())	{ templet.dieLifeTime = number; }
					else if (savedtoken.IsOutLineWide())	{ templet.outLineWide = number; }
					else if (savedtoken.IsLightFlowWide())	{ templet.lightFlowWide = number; }
					else if (savedtoken.IsDrawCount())		{ templet.drawCount.m_Min = (int)number.m_Min; templet.drawCount.m_Max = (int)number.m_Max; }
					else if (savedtoken.IsLandPos())		{ templet.landPosition = number; }
					else if (savedtoken.IsLayer())			{ templet.layer = (int)number.m_Max; }
					else if (savedtoken.IsElasticCoeff())	{ templet.elasticCoeff = number; }
#ifdef ADD_UPDATE_LANDPOS
					else if (savedtoken.IsElasticCoeffX())	{ templet.elasticCoeffX = number; }
#endif
					else if (savedtoken.IsSlashTraceType())	{ templet.iSlashTraceType = (int)number.m_Max; }
					else if (savedtoken.IsSlashTraceCurveType())	{ templet.slashTraceCurveType = (CKTDGSlashTrace::CURVE_TYPE)(int) number.m_Max; }
					else if (savedtoken.IsNotCheckLandTime()) { templet.fNotCheckLandTime = (float)number.m_Max; }
					else 
					{
						throw("Unknown sequence numeric property!");
					}
				}
				break;

			case SeqVector3Prop:
				{
					if (StartedProcessingEvents) 
						throw("Cannot specify any sequence properties after specifying events.");
					CMinMax<D3DXVECTOR3> v;
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					if (TokenIter->m_Type != Equals) throw("Expected equals sign!");
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					CXMeshPlayerTokenizer::ProcessVector3(v, TokenIter, EndIter);

					if (savedtoken.IsLightPos())			{ templet.lightPos = v; }
					else if (savedtoken.IsGravity())		{ templet.gravity = v; }
					else if (savedtoken.IsBlackHolePos())	{ templet.blackHolePosition = v; }
					else 
					{
						throw("Unknown sequence vector property!");
					}
				}
				break;

			case SeqAlphaBlendModeProp:
				{
					if (StartedProcessingEvents) 
						throw("Cannot specify any sequence properties after specifying events.");
					int alphablendmode;
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					if (TokenIter->m_Type != Equals) throw("Expected equals sign!");
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					CXMeshPlayerTokenizer::ProcessAlphaBlendMode(alphablendmode, TokenIter, EndIter);

					if (savedtoken.IsSrcBlendMode())       { templet.srcBlend = (D3DBLEND)alphablendmode; }
					else if (savedtoken.IsDestBlendMode()) { templet.destBlend = (D3DBLEND)alphablendmode; }
					else 
					{
						throw("Unknown sequence alpha blending mode property!");
					}
				}
				break;

			case SeqBillboardTypeProp:
				{
					if (StartedProcessingEvents) 
						throw("Cannot specify any sequence properties after specifying events.");
					CKTDGMatrix::BILLBOARD_TYPE billboardType;
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					if (TokenIter->m_Type != Equals) throw("Expected equals sign!");
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					CXMeshPlayerTokenizer::ProcessBillboardType(billboardType, TokenIter, EndIter);

					if (savedtoken.IsBillboardType())       { templet.billBoardType = billboardType; }
					else 
					{
						throw("Unknown sequence billboardType property!");
					}
				}
				break;

			case SeqBoolProp:
				{
					if (StartedProcessingEvents) 
						throw("Cannot specify any sequence properties after specifying events.");
					bool bValue;
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					if (TokenIter->m_Type != Equals) throw("Expected equals sign!");
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					CXMeshPlayerTokenizer::ProcessBool(bValue, TokenIter, EndIter);

					if (savedtoken.IsUseDieEvent())				{ templet.bUseDieEvent		= bValue; }
					else if (savedtoken.IsZEnable())			{ templet.bZEnable			= bValue; }
					else if (savedtoken.IsZWriteEnable())		{ templet.bZWriteEnable		= bValue; }
					else if (savedtoken.IsUseLand())			{ templet.bUseLand			= bValue; }

					else if (savedtoken.IsSetLandHeightOnStart())			{ templet.bSetLandHeightOnStart			= bValue; }

					else if (savedtoken.IsForceLayer())			{ templet.bForceLayer		= bValue; }
					else if (savedtoken.IsUseSlashTrace())		{ templet.bUseSlashTrace	= bValue; }
					else if (savedtoken.IsCrashLand())			{ templet.bCrashLand		= bValue; }		
#ifdef PARTICLE_NOTAPPLY_UNITSCALE
					else if (savedtoken.IsApplyUnitScale())		{ templet.bApplyUnitScale	= bValue; }				
#endif //PARTICLE_NOTAPPLY_UNITSCALE
					else 
					{
						throw("Unknown sequence bool property!");
					}
				}
				break;

			case SeqStringProp:
				{
					if (StartedProcessingEvents) 
						throw("Cannot specify any sequence properties after specifying events.");
					wstring str;
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					if (TokenIter->m_Type != Equals) throw("Expected equals sign!");
					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
					RemoveQuotes(TokenIter->m_strValue);
					CXMeshPlayerTokenizer::ProcessString(str, TokenIter, EndIter);

					if (savedtoken.IsMeshName())			{ templet.meshName			= str; }
					else if (savedtoken.IsChangeTexXET())	{ templet.changeTexXETName	= str; }
					else if (savedtoken.IsMultiTexXET())	{ templet.multiTexXETName	= str; }
					else if (savedtoken.IsAniXET())			{ templet.aniXETName		= str; }
					else if (savedtoken.IsSlashTraceTextureName() ) { templet.slashTraceTextureName = str; }
					else 
					{
						throw("Unknown sequence String property!");
					}
				}
				break;

				/*
				case KeywordTexture:
				{
				// TODO
				if (StartedProcessingEvents) 
				throw("Cannot specify any sequence properties after specifying events.");
				CMinMax<D3DXVECTOR3> v;
				CParticleEmitterTokenizer::NextToken(TokenIter, EndIter); 
				if (TokenIter->m_Type != Equals) throw("Expected equals sign!");
				CParticleEmitterTokenizer::NextToken(TokenIter, EndIter); 

				if (TokenIter->m_Type != Quote) throw("Expected filename after texture sequence property.");

				WCHAR wFileName[255] = {0,};
				MultiByteToWideChar( CP_ACP, 0, RemoveQuotes(TokenIter->m_strValue).c_str(), -1, wFileName, 255);
				seq.SetTextureName( wFileName );
				CParticleEmitterTokenizer::NextToken(TokenIter, EndIter); 
				}
				break;
				*/

			case KeywordGlobal: 
			case KeywordDie: 
			case KeywordAnim:
			/*
			case KeywordFade: 
			case KeywordAt: 
			case KeywordFrom:
			case KeywordTo: 
			case KeywordInitial: 
			case KeywordFinal:
			*/
				{
					StartedProcessingEvents = true;

					bool IsFade = false;
					bool IsFinal = false;
					CMinMax<float> TimeRange;
					TokenType eventArea;
					eventArea = TokenIter->m_Type;

					CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);
					// parse the time range section of the event line
					
					CXMeshPlayerTokenizer::ProcessTime(TimeRange, IsFade, IsFinal, 0, TokenIter, EndIter);

					if (TokenIter->m_Type != MeshNumericProp &&  
						TokenIter->m_Type != MeshVector3Prop &&
						TokenIter->m_Type != MeshVector2Prop &&
						TokenIter->m_Type != MeshColorProp &&
						TokenIter->m_Type != MeshBoolProp &&
						TokenIter->m_Type != MeshStringProp &&
						TokenIter->m_Type != MeshRenderTypeProp) 
						throw("Expecting meshPlayer property after time specifier!");

					CXMeshEvent* NewEvent = NULL;
					try 
					{
						// create the appropriate event
						NewEvent = EventFactory(TokenIter->m_strValue, IsFinal );

						if (!NewEvent) throw("Unknown event type, or there was an error creating this event.");

						// let the event parse the rest of its properties from the token stream.
						if (IsFade && !NewEvent->FadeAllowed()) throw("Fading is not supported on this event.");
						NewEvent->ProcessTokenStream(TokenIter, EndIter);
						NewEvent->SetActualTime(TimeRange);
						NewEvent->SetFade(IsFade);
						// event 설정이 들어갈 영역 설정
						switch( eventArea )
						{
							case KeywordGlobal:
								{
									templet.globalTimeEventList.push_back(NewEvent);
								}
								break;
							case KeywordDie:
								{
									templet.dieEventList.push_back(NewEvent);
								}
								break;
							case KeywordAnim:
								{
									templet.animTimeEventList.push_back(NewEvent);
								}
								break;
							default:
								throw("Event area do not exist.!(_global/_die/_anim");
								return false;
						}
					}
					catch(char *e) 
					{
						SAFE_DELETE(NewEvent);
						throw(e);
					}
					catch(...) 
					{
						SAFE_DELETE(NewEvent);
						throw("Unhandled exception creating event!");
					}
				}
				break;

			default: 
				{
					char buf[256]; 
		#ifdef	CONVERSION_VS
					_snprintf_s(buf, sizeof(buf), _countof(buf), "Unexpected \"%s\" in Sequence Block!", TokenIter->m_strValue.c_str());
		#else	CONVERSION_VS
					_snprintf(buf, sizeof(buf), "Unexpected \"%s\" in Sequence Block!", TokenIter->m_strValue.c_str());
		#endif	CONVERSION_VS
				
					throw(&buf[0]);
				}
				break;
		}
	}

	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 

	return(true);
}

CKTDGXMeshPlayer::CXMeshEvent* CKTDGXMeshPlayer::EventFactory( std::string EventName, bool bFinal )
{
	CXMeshEvent *event = NULL;

	if (EventName.compare("ANINAME") == 0)				event = new CXMeshEvent_AniName(bFinal);
	else if (EventName.compare("ANISPEED") == 0)		event = new CXMeshEvent_AniSpeed(bFinal);
	else if (EventName.compare("SLASHTRACE") == 0)		event = new CXMeshEvent_SlashTrace(bFinal);
	else if (EventName.compare("COLOR") == 0)			event = new CXMeshEvent_Color(bFinal);
	else if (EventName.compare("OUTLINECOLOR") == 0)	event = new CXMeshEvent_OutLineColor(bFinal);
	else if (EventName.compare("SIZE") == 0)			event = new CXMeshEvent_Size(bFinal);
#ifdef EVENT_SCENE
	else if (EventName.compare("POSITION") == 0)		event = new CXMeshEvent_Position(bFinal);
#endif EVENT_SCENE
	else if (EventName.compare("VELOCITY") == 0)		event = new CXMeshEvent_Velocity(bFinal);
	else if (EventName.compare("TEX0UV") == 0)			event = new CXMeshEvent_Tex0UV(bFinal);
	else if (EventName.compare("TEX1UV") == 0)			event = new CXMeshEvent_Tex1UV(bFinal);
	else if (EventName.compare("TEX2UV") == 0)			event = new CXMeshEvent_Tex2UV(bFinal);
	else if (EventName.compare("ROTATE") == 0)			event = new CXMeshEvent_Rotate(bFinal);
	else if (EventName.compare("DIRSPEED") == 0)		event = new CXMeshEvent_DirSpeed(bFinal);
	else if (EventName.compare("BLACKHOLETIME") == 0)	event = new CXMeshEvent_BlackHoleTime(bFinal);
	else if (EventName.compare("CRASH") == 0)			event = new CXMeshEvent_Crash(bFinal);
	else if (EventName.compare("RESETCRASH") == 0)		event = new CXMeshEvent_ResetCrash(bFinal);
	else if (EventName.compare("LIGHTFLOWIMPACT") == 0)	event = new CXMeshEvent_LightFlowImpact(bFinal);
	else if (EventName.compare("LIGHTFLOWPOINT") == 0)	event = new CXMeshEvent_LightFlowPoint(bFinal);
	else if (EventName.compare("SOUND") == 0)			event = new CXMeshEvent_Sound(bFinal);
	else if (EventName.compare("RENDER_TYPE") == 0)		event = new CXMeshEvent_RenderType(bFinal);


	return(event);
}

const CKTDGXMeshPlayer::XMeshTemplet* CKTDGXMeshPlayer::GetTempletByName( const wstring wstrName )
{
	if( wstrName.length() == 0 )
		return NULL;

	map<wstring, XMeshTemplet*>::iterator it;
	it = m_TempletMap.find( wstrName );
	if( it != m_TempletMap.end() )
	{
		return it->second;
	}
	return NULL;
}




////////////////////////////////////////////
// CXMeshInstance
////////////////////////////////////////////
CKTDGXMeshPlayer::CXMeshInstance::CXMeshInstance( CKTDGXMeshPlayer* pMeshPlayer, XMeshTemplet* pTemplet, D3DXVECTOR3 pos, D3DXVECTOR3 angleDegree, D3DXVECTOR3 moveAxisAngleDegree )
: m_bParabolicCurve( false )
, m_pFrame_TRACE_START0( NULL )
, m_pFrame_TRACE_END0( NULL )
, m_vScale( 1, 1, 1 )
#ifdef ADD_UPDATE_LANDPOS
, m_bToggleRotate(false)
#endif
#ifdef SHOW_ATTACK_BOX_DUMMY
, m_pXMeshSphere ( NULL )
, m_pMatrixSphere ( NULL )
, m_bShowAttackBox ( false )
#endif //SHOW_ATTACK_BOX_DUMMY
//#ifdef GIANT_UNIT_GIANT_EFFECT_TEST
//, m_vScaleByUnit( 1, 1, 1 )
//#endif GIANT_UNIT_GIANT_EFFECT_TEST
{
	SetAlphaObject( true );

	m_Handle = INVALID_MESH_INSTANCE_HANDLE;

	m_State					= IS_PLAY;
	//m_bShow					= true;
	
	m_fDelayTime			= 0.0f;
	m_pMeshPlayer			= pMeshPlayer;
	m_pOrgTemplet			= pTemplet;
	m_TempletData			= *m_pOrgTemplet;

	m_bUseDXMatrix			= false;

	SetLayer( m_TempletData.layer );
	SetForceLayer( m_TempletData.bForceLayer );
	
	m_pXMesh				= NULL;
	m_pXSkinMesh			= NULL;
	m_pXSkinAnim			= NULL;
	m_pChangeTexXET			= NULL;
	m_pMultiTexXET			= NULL;
	m_pAniXET				= NULL;
	m_pAniData				= NULL;
	m_fAniPlaySpeed			= 1.0f;

	m_fElapsedTime			= 0.0f;
	m_fGlobalTimeBefore		= 0.0f;
	m_fGlobalTime			= 0.0f;
	m_fDieTimeBefore		= 0.0f;
	m_fDieTime				= 0.0f;
	m_fAniTimeBefore		= 0.0f;
	m_fAniTime				= 0.0f;
	m_fFinalTime			= 0.0f;
	m_fDieFinalTime			= 0.0f;

	m_bManualDir			= false;
	m_ManualDirDegree		= angleDegree;

	m_vPosBefore			= pos;
	m_vPos					= pos;
	m_vAngleDegree			= angleDegree;
	m_MoveAxisAngleDegree	= moveAxisAngleDegree;
	
	m_NowPlayCount			= 0;
	m_MaxPlayCount			= m_TempletData.playCount.GetRandomNumInRange();
	m_NowLifeTime			= 0.0f;
	m_MaxLifeTime			= m_TempletData.lifeTime.GetRandomNumInRange();
	m_NowDiePlayCount		= 0;
	m_MaxDiePlayCount		= m_TempletData.diePlayCount.GetRandomNumInRange();
	m_NowDieLifeTime		= 0.0f;
	m_MaxDieLifeTime		= m_TempletData.dieLifeTime.GetRandomNumInRange();

	m_LightPos				= m_TempletData.lightPos.GetRandomNumInRange();
	m_fOutLineWide			= m_TempletData.outLineWide.GetRandomNumInRange();
	m_fLightFlowWide		= m_TempletData.lightFlowWide.GetRandomNumInRange();

	m_DrawCount				= m_TempletData.drawCount.GetRandomNumInRange();
	m_Gravity				= m_TempletData.gravity.GetRandomNumInRange();		
	m_BlackHolePosition		= m_TempletData.blackHolePosition.GetRandomNumInRange();
	m_fLandPosition			= m_TempletData.landPosition.GetRandomNumInRange();

	
	m_Color					= D3DXCOLOR(1.0f,1.0f,1.0f,1.0f);
	m_ColorFinal			= D3DXCOLOR(1.0f,1.0f,1.0f,1.0f);

	m_OutLineColor			= D3DXCOLOR(1.0f,1.0f,1.0f,1.0f);
	m_OutLineColorFinal		= D3DXCOLOR(1.0f,1.0f,1.0f,1.0f);

	INIT_VECTOR3( m_vSize, 1.0f, 1.0f, 1.0f );
	INIT_VECTOR3( m_vSizeFinal, 1.0f, 1.0f, 1.0f );
    INIT_VECTOR3( m_vManualSize, 1.0f, 1.0f, 1.0f ); // added by wonpok

#ifdef EVENT_SCENE
	INIT_VECTOR3( m_vPositionGap, 0.0f,0.0f,0.0f );
	INIT_VECTOR3( m_vPositionFinal, 0.0f,0.0f,0.0f );
#endif EVENT_SCENE

	INIT_VECTOR3( m_vVelocity, 0.0f,0.0f,0.0f );
	INIT_VECTOR3( m_vVelocityFinal, 0.0f,0.0f,0.0f );

	INIT_VECTOR2( m_vTexStage0UV, 0.0f,0.0f );
	INIT_VECTOR2( m_vTexStage0UVFinal, 0.0f,0.0f );

	INIT_VECTOR2( m_vTexStage1UV, 0.0f,0.0f );
	INIT_VECTOR2( m_vTexStage1UVFinal, 0.0f,0.0f );

	INIT_VECTOR2( m_vTexStage2UV, 0.0f,0.0f );
	INIT_VECTOR2( m_vTexStage2UVFinal, 0.0f,0.0f );

#ifdef LOCAL_ROTATE_EFFECT
	INIT_VECTOR3( m_vRotateLocal, 0.0f,0.0f,0.0f );
#endif
	INIT_VECTOR3( m_vRotate, 0.0f,0.0f,0.0f );
	INIT_VECTOR3( m_vRotateFinal, 0.0f,0.0f,0.0f );

	m_fDirSpeed			= 0.0f;
	m_fDirSpeedFinal	= 0.0f;

	m_fLightFlowImpact		= 0.0f;
	m_fLightFlowImpactFinal	= 0.0f;

	INIT_VECTOR3( m_LightFlowPoint, 0.0f,0.0f,0.0f );
	INIT_VECTOR3( m_LightFlowPointFinal, 0.0f,0.0f,0.0f );

	INIT_VECTOR3( m_BlackHoleSpeed, 0.0f,0.0f,0.0f );
	m_BlackHoleTime		= 0.0f;

	m_fResetCrash		= 0.01f;
	m_fNowResetCrash	= 0.0f;
	m_fResetCrashFinal	= 0.0f;

	INIT_VECTOR3( m_Crash, 0.0f,0.0f,0.0f );


	m_bSlashTrace		= false;
	m_pSlashTrace		= NULL;
	m_pSlashTraceTip	= NULL;


#ifdef PARTICLE_NOTAPPLY_UNITSCALE
	m_bApplyUnitScale	= true;
#endif

	if( m_TempletData.meshType == X_MESH )
	{
		m_pXMesh			= g_pKTDXApp->GetDeviceManager()->OpenXMesh( m_TempletData.meshName );
		m_pChangeTexXET		= g_pKTDXApp->GetDeviceManager()->OpenXET( m_TempletData.changeTexXETName.c_str() );
		m_pMultiTexXET		= g_pKTDXApp->GetDeviceManager()->OpenXET( m_TempletData.multiTexXETName.c_str() );
		m_pAniXET			= g_pKTDXApp->GetDeviceManager()->OpenXET( m_TempletData.aniXETName.c_str() );
		m_pAniData			= NULL;
		
		if( m_pXMesh != NULL )
		{
			SetCenter( m_pXMesh->GetCenter() );
			SetBoundingRadius( m_pXMesh->GetRadius() );
		}
	}
	else if( m_TempletData.meshType == X_SKIN_MESH )
	{
		m_pXSkinMesh		= g_pKTDXApp->GetDeviceManager()->OpenXSkinMesh( m_TempletData.meshName );
		m_pChangeTexXET		= g_pKTDXApp->GetDeviceManager()->OpenXET( m_TempletData.changeTexXETName.c_str() );
		m_pMultiTexXET		= g_pKTDXApp->GetDeviceManager()->OpenXET( m_TempletData.multiTexXETName.c_str() );
		m_pAniXET			= g_pKTDXApp->GetDeviceManager()->OpenXET( m_TempletData.aniXETName.c_str() );
		m_pAniData			= NULL;

		if( m_pXSkinMesh != NULL )
		{
			m_pXSkinAnim		= CKTDGXSkinAnim::CreateSkinAnim();

			m_pXSkinAnim->SetAnimXSkinMesh( m_pXSkinMesh, m_pAniXET );
			m_pXSkinAnim->AddModelXSkinMesh( m_pXSkinMesh, m_pAniXET, m_pMultiTexXET, m_pChangeTexXET );
			
			m_pXSkinAnim->GetMatrix().Move( m_vPos );
			m_pXSkinAnim->SetBillBoardType( m_TempletData.billBoardType );


			CKTDGXRenderer::RenderParam* pRenderParam = m_pXSkinAnim->GetRenderParam();
			pRenderParam->renderType			= m_TempletData.renderType;
			pRenderParam->cartoonTexType		= m_TempletData.cartoonTexType;
			//pRenderParam->lightPos				= m_LightPos;
			//pRenderParam->fOutLineWide			= GetDistanceToCamera() / m_fOutLineWide;
			//pRenderParam->outLineColor			= m_OutLineColor;
			//pRenderParam->color					= m_Color;
			//pRenderParam->fLightFlowWide		= m_fLightFlowWide;
			//pRenderParam->fLightFlowImpact		= m_fLightFlowImpact;
			//pRenderParam->lightFlowPoint		= m_LightFlowPoint;
			//pRenderParam->bAlphaBlend			= IsAlphaObject();
			pRenderParam->bZWriteEnable			= m_TempletData.bZWriteEnable;
			pRenderParam->bZEnable				= m_TempletData.bZEnable;
			pRenderParam->cullMode				= m_TempletData.cullMode;
			pRenderParam->srcBlend				= m_TempletData.srcBlend;
			pRenderParam->destBlend				= m_TempletData.destBlend;
			//pRenderParam->texOffsetStage0.x		= m_vTexStage0UV.x;
			//pRenderParam->texOffsetStage0.y		= m_vTexStage0UV.y;
			//pRenderParam->texOffsetStage1.x		= m_vTexStage1UV.x;
			//pRenderParam->texOffsetStage1.y		= m_vTexStage1UV.y;
			//pRenderParam->texOffsetStage2.x		= m_vTexStage2UV.x;
			//pRenderParam->texOffsetStage2.y		= m_vTexStage2UV.y;




			ASSERT( true == m_TempletData.bZWriteEnable ||
				( CKTDGXRenderer::RT_CARTOON_BLACK_EDGE != m_TempletData.renderType &&
				CKTDGXRenderer::RT_CARTOON_COLOR_EDGE != m_TempletData.renderType &&
				CKTDGXRenderer::RT_ADD_CARTOON_BLACK_EDGE != m_TempletData.renderType &&
				CKTDGXRenderer::RT_ADD_CARTOON_COLOR_EDGE != m_TempletData.renderType &&
				CKTDGXRenderer::RT_BLACK_EDGE != m_TempletData.renderType &&
				CKTDGXRenderer::RT_COLOR_EDGE != m_TempletData.renderType ) );



			SetCenter( m_pXSkinMesh->GetCenter() );
#ifdef EVENT_SCENE
			// View transform 변경으로 culling 문제 일어날 수 있음. 2D PLANE 메시에 한해 아래와 같이 컬링 안하거나
			// 더 좋은 XMesh 2D PLANE 기능들어가면 이 처리는 필요없습니다. 
			// 예외처리 아직 안됨
			SetBoundingRadius( m_pXSkinMesh->GetBoundingRadius() );
#else
			SetBoundingRadius( m_pXSkinMesh->GetBoundingRadius() );
#endif EVENT_SCENE

		}
	}


	if( m_pXMesh == NULL && m_pXSkinMesh == NULL )
	{
		return;
	}

	GetMatrix().Move( m_vPos );
	m_vRotate = m_vAngleDegree;
	GetMatrix().Rotate( D3DXToRadian(m_vAngleDegree.x), D3DXToRadian(m_vAngleDegree.y), D3DXToRadian(m_vAngleDegree.z) );


	if( true == pMeshPlayer->GetEnableSlashTrace() && 
		true == pTemplet->bUseSlashTrace )
	{
		if( X_SKIN_MESH == m_TempletData.meshType &&
			NULL != m_pXSkinAnim && 
			NULL != m_pXSkinAnim->GetAnimXSkinMesh() )
		{
			m_pFrame_TRACE_START0		= m_pXSkinAnim->GetCloneFrame( L"TRACE_START0" );
			m_pFrame_TRACE_END0			= m_pXSkinAnim->GetCloneFrame( L"TRACE_END0" );

			if( true == pTemplet->slashTraceTextureName.empty() )
			{
				m_pSlashTrace = CKTDGSlashTrace::CreateSlashTrace( 100 );
			}
			else
			{
				m_pSlashTrace = CKTDGSlashTrace::CreateSlashTrace( 100, true, 100.f, pTemplet->slashTraceTextureName );
			}

			if( NULL != m_pSlashTrace )
			{
				m_pSlashTrace->SetCenter( m_pXSkinAnim->GetAnimXSkinMesh()->GetCenter() );
				m_pSlashTrace->SetBoundingRadius( m_pXSkinAnim->GetAnimXSkinMesh()->GetBoundingRadius() * 1.5f );
				m_pSlashTrace->SetDisableTime( 0.4f );
				m_pSlashTrace->SetDrawCount( 1 );
				m_pSlashTrace->SetLayer( GetLayer() );
				m_pSlashTrace->SetCurveType( pTemplet->slashTraceCurveType );
				g_pKTDXApp->GetDGManager()->AddObjectChain( m_pSlashTrace );
			}


			if( true == pTemplet->slashTraceTextureName.empty() )
			{
				m_pSlashTraceTip = CKTDGSlashTrace::CreateSlashTrace( 100 );
			}
			else
			{
				//m_pSlashTraceTip = CKTDGSlashTrace::CreateSlashTrace( 100, true, 100.f, pTemplet->slashTraceTextureName );
			}

			if( NULL != m_pSlashTraceTip )
			{
				m_pSlashTraceTip->SetCenter( m_pXSkinAnim->GetAnimXSkinMesh()->GetCenter() );
				m_pSlashTraceTip->SetBoundingRadius( m_pXSkinAnim->GetAnimXSkinMesh()->GetBoundingRadius() * 1.5f );
				m_pSlashTraceTip->SetDisableTime( 0.5f );
				m_pSlashTraceTip->SetDrawCount( 3 );
				m_pSlashTraceTip->SetLayer( GetLayer() );
				m_pSlashTraceTip->SetCurveType( pTemplet->slashTraceCurveType );
				g_pKTDXApp->GetDGManager()->AddObjectChain( m_pSlashTraceTip );
			}

		}
	}

	OpenResource();
	RunEvent();
#ifdef SHOW_ATTACK_BOX_DUMMY
	if( NULL != m_pMeshPlayer &&
		true == m_pMeshPlayer->GetShowAttackBox())
	{
		m_bShowAttackBox	= true;
		m_pXMeshSphere		= g_pKTDXApp->GetDeviceManager()->OpenXMesh( L"Bounding_Sphere.X" );
		m_pMatrixSphere		= new CKTDGMatrix( g_pKTDXApp->GetDevice() );
		m_DrawCount = max( 1, m_DrawCount );
	}
#endif // SHOW_ATTACK_BOX_DUMMY
}

CKTDGXMeshPlayer::CXMeshInstance::~CXMeshInstance()
{
	//g_pKTDXApp->GetDGManager()->RemoveObjectChain( this );

	SAFE_CLOSE( m_pChangeTexXET );
	SAFE_CLOSE( m_pMultiTexXET );
	SAFE_CLOSE( m_pAniXET );


	SAFE_DELETE_KTDGOBJECT( m_pXSkinAnim );


	SAFE_CLOSE( m_pXSkinMesh );
	SAFE_CLOSE( m_pXMesh );


	SAFE_DELETE_KTDGOBJECT( m_pSlashTrace );
	SAFE_DELETE_KTDGOBJECT( m_pSlashTraceTip );




	map<wstring,CKTDXDeviceSound*>::iterator iter2;
	for( iter2 = m_SoundMap.begin(); iter2 != m_SoundMap.end(); iter2++ )
	{
		CKTDXDeviceSound* pSound = iter2->second;
		SAFE_CLOSE( pSound );
	}
#ifdef SHOW_ATTACK_BOX_DUMMY
		if( NULL != m_pXMeshSphere &&
			NULL != g_pKTDXApp &&
			NULL != g_pKTDXApp->GetDeviceManager() )
		{
			g_pKTDXApp->GetDeviceManager()->CloseDevice( m_pXMeshSphere->GetDeviceID() );
			SAFE_DELETE( m_pMatrixSphere );
		}
#endif // SHOW_ATTACK_BOX_DUMMY
}

HRESULT CKTDGXMeshPlayer::CXMeshInstance::OnFrameMove( double fTime, float fElapsedTime )
{
	m_fElapsedTime		= fElapsedTime;
	m_vPosBefore		= m_vPos;

	if( m_fDelayTime > 0.0f )
	{
		m_fDelayTime -= m_fElapsedTime;
		return S_OK;
	}

#ifdef STOP_UNIT_STOP_EFFECT_TEST

	m_StopTime.OnFrameMove( fTime, fElapsedTime );
	if( m_StopTime.m_fTimeLeft > 0.f )
	{
		return S_OK; 
	}

#endif STOP_UNIT_STOP_EFFECT_TEST



	switch( m_State )
	{
		case IS_PLAY:
			PlayProcess();
			break;

		case IS_DYING:
			DyingProcess();
			break;

		case IS_DIE:
			break;
	}

	//이벤트 적용
	RunEvent();

#ifdef EVENT_SCENE
	m_vPos += m_vPositionGap;

#endif EVENT_SCENE


#ifdef FIX_XMESH_PLAYER_GRAVITY_ERROR_TEST
#else FIX_XMESH_PLAYER_GRAVITY_ERROR_TEST
	//{{AFX
	//중력 적용
	m_vVelocity += m_Gravity * fElapsedTime;
	//}}AFX
#endif FIX_XMESH_PLAYER_GRAVITY_ERROR_TEST


	//블랙홀 포인트
	if( m_BlackHoleTime > 0.0f )
		m_BlackHoleSpeed = (m_BlackHolePosition - m_vPos) / m_BlackHoleTime;



	// 속도를 적용하여 오브젝트를 이동시킨다.
	D3DXMATRIX	mDir;
	D3DXVECTOR4 dirVec4;

 #ifdef ADD_UPDATE_LANDPOS
 	if( GetToggleRotae() == true && IsSamef(m_TempletData.elasticCoeffX.GetRandomNumInRange(), 0.f) == false )
 	{ 		
		//m_vVelocity.x *= m_TempletData.elasticCoeffX.GetRandomNumInRange();
		m_bToggleRotate = false;
 	}
 #endif

  	if( m_MoveAxisAngleDegree == D3DXVECTOR3(0.0f,0.0f,0.0f) )
  	{
  #ifdef FIX_XMESH_PLAYER_GRAVITY_ERROR_TEST
  		//중력 적용
  		m_vVelocity += m_Gravity * fElapsedTime;
  #endif FIX_XMESH_PLAYER_GRAVITY_ERROR_TEST
  
  		m_vPos += m_vVelocity * m_fElapsedTime;
  	}
  	else
	{
#ifdef FIX_XMESH_PLAYER_GRAVITY_ERROR_TEST
		D3DXMatrixRotationYawPitchRoll( &mDir, D3DXToRadian(m_MoveAxisAngleDegree.y), D3DXToRadian(m_MoveAxisAngleDegree.x), D3DXToRadian(m_MoveAxisAngleDegree.z) );
		D3DXMATRIX matInvDir;
		if( NULL != D3DXMatrixInverse( &matInvDir, NULL, &mDir ) )
		{
			//중력 적용
			D3DXVECTOR3 vTransformedGravity;
			D3DXVec3TransformCoord( &vTransformedGravity, &m_Gravity, &matInvDir );

			m_vVelocity += vTransformedGravity * fElapsedTime;			
		}
		D3DXVECTOR3 tempMoveGap = m_vVelocity * fElapsedTime;
		

#else FIX_XMESH_PLAYER_GRAVITY_ERROR_TEST
		//{{AFX
		D3DXVECTOR3 tempMoveGap = m_vVelocity * fElapsedTime;
		D3DXMatrixRotationYawPitchRoll( &mDir, D3DXToRadian(m_MoveAxisAngleDegree.y), D3DXToRadian(m_MoveAxisAngleDegree.x), D3DXToRadian(m_MoveAxisAngleDegree.z) );
		//}}AFX
#endif FIX_XMESH_PLAYER_GRAVITY_ERROR_TEST

		D3DXVec3TransformCoord( &tempMoveGap, &tempMoveGap, &mDir );
		m_vPos		+= tempMoveGap;

		if( true == m_bParabolicCurve )
		{
			m_vRotate = GetDirVecToDegree( tempMoveGap );
		}
	}

	

		//랜드 포지션
	if( m_TempletData.bUseLand == true 
#ifdef ADD_UPDATE_LANDPOS
		&& m_NowLifeTime > m_TempletData.fNotCheckLandTime 
#endif
		)
	{
		if( m_vPos.y < m_fLandPosition )
		{
			m_vPos.y = m_fLandPosition;

			if(m_TempletData.bCrashLand == false)
			{
				if( m_vVelocity.y < 0.0f )
				{
 					if( m_vVelocity.y >= -50.0f
 						&& m_vVelocity.y <= 50.0f )
 					{	
 						m_vVelocity.y = 0.0f;
 					}
 					else
					{	
						m_vVelocity.y *= -m_TempletData.elasticCoeff.GetRandomNumInRange();						
					}
				}

#ifdef ADD_UPDATE_LANDPOS
 				if( IsSamef( m_TempletData.elasticCoeffX.GetRandomNumInRange(), 0.f ) == false && IsSamef(m_vVelocity.x, 0.f) == false )
				{ 					
					if( m_vVelocity.x >= -10.0f && m_vVelocity.x <= 10.0f )
 					{
 						m_vVelocity.x = 0.0f;
						m_TempletData.elasticCoeffX = CMinMax<float>(0.f, 0.f);
 					}
 					else
 					{
						m_vVelocity.x *= m_TempletData.elasticCoeffX.GetRandomNumInRange();
 					}

					if( m_TempletData.meshType == X_SKIN_MESH && m_pXSkinAnim != NULL )
					{					
						m_fAniPlaySpeed *= m_TempletData.elasticCoeffX.GetRandomNumInRange();
						m_pXSkinAnim->SetPlaySpeed( m_fAniPlaySpeed );
					}
				}
#endif
			}
		}
	}


	


	if( m_fDirSpeed != 0.0f )
	{

		if( m_bManualDir == false )
		{
			D3DXVECTOR3 dirVec( 1.0f, 0.0f, 0.0f );	
#ifdef LOCAL_ROTATE_EFFECT
			D3DXMatrixRotationYawPitchRoll( &mDir, D3DXToRadian(m_vRotate.y + m_vRotateLocal.y), D3DXToRadian(m_vRotate.x + m_vRotateLocal.x), D3DXToRadian(m_vRotate.z + m_vRotateLocal.z) );
#else
			D3DXMatrixRotationYawPitchRoll( &mDir, D3DXToRadian(m_vRotate.y), D3DXToRadian(m_vRotate.x), D3DXToRadian(m_vRotate.z) );
#endif
			D3DXVec3Transform( &dirVec4, &dirVec, &mDir );
			dirVec.x	= dirVec4.x;
			dirVec.y	= dirVec4.y;
			dirVec.z	= dirVec4.z;

			m_vPos		+= dirVec * m_fDirSpeed * fElapsedTime;		
		}
		else
		{
			D3DXVECTOR3 dirVec;
			dirVec.x = D3DXToRadian(m_ManualDirDegree.x);
			dirVec.y = D3DXToRadian(m_ManualDirDegree.y);
			dirVec.z = D3DXToRadian(m_ManualDirDegree.z);

			m_vPos		+= dirVec * m_fDirSpeed * fElapsedTime;
			m_vRotate = GetDirVecToDegree( dirVec );
		}		
	}
	else
	{
		m_ManualDirDegree = m_vPos - m_vPosBefore;
		D3DXVec3Normalize( &m_ManualDirDegree, &m_ManualDirDegree );
		m_ManualDirDegree.x = D3DXToDegree(m_ManualDirDegree.x);
		m_ManualDirDegree.y = D3DXToDegree(m_ManualDirDegree.y);
		m_ManualDirDegree.z = D3DXToDegree(m_ManualDirDegree.z);
	}
	m_vPos		+= m_BlackHoleSpeed		* fElapsedTime;






	//메트릭스에 수치 적용
#ifdef LOCAL_ROTATE_EFFECT
	D3DXVECTOR3 vRot = D3DXVECTOR3( D3DXToRadian(m_vRotate.x+m_vRotateLocal.x), D3DXToRadian(m_vRotate.y+m_vRotateLocal.y), D3DXToRadian(m_vRotate.z+m_vRotateLocal.z) );	
#else
	D3DXVECTOR3 vRot = D3DXVECTOR3( D3DXToRadian(m_vRotate.x), D3DXToRadian(m_vRotate.y), D3DXToRadian(m_vRotate.z) );
#endif


	GetMatrix().Rotate( vRot );
	GetMatrix().Move( m_vPos );
    m_vSize.x *= m_vManualSize.x;
    m_vSize.y *= m_vManualSize.y;
    m_vSize.z *= m_vManualSize.z;

#ifdef GIANT_UNIT_GIANT_EFFECT_TEST
	GetMatrix().Scale( D3DXVECTOR3( m_vSize.x * m_vScale.x * GetScaleByUnit().x, 
									m_vSize.y * m_vScale.y * GetScaleByUnit().y, 
									m_vSize.z * m_vScale.z * GetScaleByUnit().z ) );
#else GIANT_UNIT_GIANT_EFFECT_TEST
	GetMatrix().Scale( D3DXVECTOR3(m_vSize.x * m_vScale.x, m_vSize.y * m_vScale.y, m_vSize.z * m_vScale.z ) );
#endif GIANT_UNIT_GIANT_EFFECT_TEST

#ifdef EVENT_SCENE
	m_vPos += m_Crash;
#endif EVENT_SCENE

	

	if( m_TempletData.meshType == X_SKIN_MESH && m_pXSkinAnim != NULL )
	{
		//애니메이션에 파라메터 적용
		CKTDGXRenderer::RenderParam* pParam = m_pXSkinAnim->GetRenderParam();
		if( m_bUseDXMatrix == true )
		{
//{{ robobeg : 2008-10-24
			//pParam->worldMatrix			= m_DXMATRIX;// * GetMatrix().GetMatrix( m_TempletData.billBoardType );
			//m_pXSkinAnim->SetDXMatrix( pParam->worldMatrix );
            m_pXSkinAnim->SetDXMatrix( m_DXMATRIX );
			m_pXSkinAnim->UseDXMatrix( true );
//}} robobeg : 2008-10-24
		}
		//{{ robobeg : 2008-10-24
		//else
		//{
		//	pParam->worldMatrix			= GetMatrix().GetMatrix( m_TempletData.billBoardType );
		//}
		//}} robobeg : 2008-10-24
		pParam->renderType			= m_TempletData.renderType;
		pParam->cartoonTexType		= m_TempletData.cartoonTexType;
		pParam->lightPos			= m_LightPos;
		
		if( 0.f != m_fOutLineWide )
		{
			pParam->fOutLineWide		= GetDistanceToCamera() / m_fOutLineWide;
		}
		else
		{
			pParam->fOutLineWide		= 1.f;
		}
		
		pParam->outLineColor		= m_OutLineColor;
		pParam->color				= m_Color;
		pParam->fLightFlowWide		= m_fLightFlowWide;
		pParam->fLightFlowImpact	= m_fLightFlowImpact;
		pParam->lightFlowPoint		= m_LightFlowPoint;
		pParam->bAlphaBlend			= IsAlphaObject();
		pParam->bZWriteEnable		= m_TempletData.bZWriteEnable;
		pParam->bZEnable			= m_TempletData.bZEnable;
		pParam->cullMode			= m_TempletData.cullMode;
		pParam->srcBlend			= m_TempletData.srcBlend;
		pParam->destBlend			= m_TempletData.destBlend;
		pParam->texOffsetStage0.x	= m_vTexStage0UV.x;
		pParam->texOffsetStage0.y	= m_vTexStage0UV.y;
		pParam->texOffsetStage1.x	= m_vTexStage1UV.x;
		pParam->texOffsetStage1.y	= m_vTexStage1UV.y;
		pParam->texOffsetStage2.x	= m_vTexStage2UV.x;
		pParam->texOffsetStage2.y	= m_vTexStage2UV.y;


		m_pXSkinAnim->GetMatrix().Rotate( vRot );
		m_pXSkinAnim->GetMatrix().Move( m_vPos );
		
#ifdef GIANT_UNIT_GIANT_EFFECT_TEST
		m_pXSkinAnim->GetMatrix().Scale( D3DXVECTOR3( m_vSize.x * m_vScale.x * GetScaleByUnit().x, 
														m_vSize.y * m_vScale.y * GetScaleByUnit().y, 
														m_vSize.z * m_vScale.z * GetScaleByUnit().z ) );
#else GIANT_UNIT_GIANT_EFFECT_TEST
		m_pXSkinAnim->GetMatrix().Scale( D3DXVECTOR3(m_vSize.x * m_vScale.x, m_vSize.y * m_vScale.y, m_vSize.z * m_vScale.z ) );

#endif GIANT_UNIT_GIANT_EFFECT_TEST


		

		m_pXSkinAnim->OnFrameMove( fTime, m_fElapsedTime * m_fAniPlaySpeed );
	}



	if( true == m_pMeshPlayer->GetEnableSlashTrace() )
	{
		if( NULL != m_pSlashTrace )
		{
			m_pSlashTrace->OnFrameMove( m_fElapsedTime );
			m_pSlashTrace->GetMatrix().Move( GetMatrix().GetPos() );
		}

		if( NULL != m_pSlashTraceTip )
		{
			m_pSlashTraceTip->OnFrameMove( m_fElapsedTime );
			m_pSlashTraceTip->GetMatrix().Move( GetMatrix().GetPos() );
		}

	
		if( NULL != m_pFrame_TRACE_START0 && NULL != m_pFrame_TRACE_END0 )
		{
			D3DXVECTOR3 up, down;
			GetFramePos( &up, m_pFrame_TRACE_START0 );
			GetFramePos( &down, m_pFrame_TRACE_END0 );

			const float MAGIC_SLASH_TRACE_TIP_WIDTH = 4.f;
			D3DXVECTOR3 dirTip	= down - up;
			D3DXVec3Normalize( &dirTip, &dirTip );
			D3DXVECTOR3 tipDown = up + dirTip * MAGIC_SLASH_TRACE_TIP_WIDTH;
		

			if( true == m_bSlashTrace )
			{
				switch( m_TempletData.iSlashTraceType )
				{
				case CKTDGSlashTrace::STT_DEFAULT:
					{
						if( NULL != m_pSlashTrace )
							m_pSlashTrace->AddSlashData( up, down, 0xcc5555ff );

						if( NULL != m_pSlashTraceTip )
							m_pSlashTraceTip->AddSlashData( up, tipDown, 0xff5555ff );
					} break;

				case CKTDGSlashTrace::STT_CONSTANT_WIDTH:
					{
						D3DXVECTOR3 vLineDir = GetDirVec();
						D3DXVec3Normalize( &vLineDir, &vLineDir );

						if( NULL != m_pSlashTrace )
							m_pSlashTrace->AddSlashDataConstantWidth( up, down, 0xcc5555ff, vLineDir );

						//if( NULL != m_pSlashTraceTip )
						//	m_pSlashTraceTip->AddSlashData( up, tipDown, D3DXCOLOR( 0, 0, 0, 0 ) );

					} break;

				case CKTDGSlashTrace::STT_CIRCULAR:
					{
						D3DXVECTOR3 vLineDir = GetDirVec();
						D3DXVec3Normalize( &vLineDir, &vLineDir );

						if( NULL != m_pSlashTrace )
							m_pSlashTrace->AddSlashDataCircular( m_vPos, up, down, 0xcc5555ff, vLineDir, 50.f, false );

						//if( NULL != m_pSlashTraceTip )
						//	m_pSlashTraceTip->AddSlashData( up, tipDown, D3DXCOLOR( 0, 0, 0, 0 ) );

					} break;
				}
			}
			else
			{
				if( NULL != m_pSlashTrace )
					m_pSlashTrace->AddSlashData( up, down, D3DXCOLOR( 0, 0, 0, 0 ) );
				if( NULL != m_pSlashTraceTip )
					m_pSlashTraceTip->AddSlashData( up, tipDown, D3DXCOLOR( 0, 0, 0, 0 ) );
			}
		}
	}
#ifdef EVENT_SCENE // 주석 추가 예정
	m_vPos -= m_Crash;
	m_Crash = D3DXVECTOR3(0.f, 0.f, 0.f);
#endif EVENT_SCENE
	return S_OK;
}

D3DXVECTOR3 CKTDGXMeshPlayer::CXMeshInstance::GetDirVec()
{
	D3DXMATRIX	mDir;
	D3DXVECTOR4 dirVec4;
	D3DXVECTOR3 dirVec( 1.0f, 0.0f, 0.0f );					
#ifdef LOCAL_ROTATE_EFFECT
	D3DXMatrixRotationYawPitchRoll( &mDir, D3DXToRadian(m_vRotate.y + m_vRotateLocal.y), D3DXToRadian(m_vRotate.x + m_vRotateLocal.x), D3DXToRadian(m_vRotate.z + m_vRotateLocal.z) );
#else
	D3DXMatrixRotationYawPitchRoll( &mDir, D3DXToRadian(m_vRotate.y), D3DXToRadian(m_vRotate.x), D3DXToRadian(m_vRotate.z) );
#endif
	D3DXVec3Transform( &dirVec4, &dirVec, &mDir );
	dirVec.x	= dirVec4.x;
	dirVec.y	= dirVec4.y;
	dirVec.z	= dirVec4.z;

	return dirVec;
}

void CKTDGXMeshPlayer::CXMeshInstance::PlayProcess()
{
    if( m_pXSkinAnim == NULL )
        return;

	m_fGlobalTimeBefore		= m_fGlobalTime;
	m_fGlobalTime			+= m_fElapsedTime;
	m_fNowResetCrash		+= m_fElapsedTime;
	m_fAniTimeBefore		= m_fAniTime;
    
	//다이 체크
	switch( m_TempletData.lifeType )
	{
		case LT_PLAY_COUNT:
			{
				if( m_TempletData.meshType == X_MESH )
				{
					m_fAniTime	+= m_fElapsedTime * m_fAniPlaySpeed;

					if( m_TempletData.bCrashLand == true && 
						m_fAniTime > m_TempletData.fNotCheckLandTime && 
						m_vPos.y <= m_fLandPosition )
					{
						m_NowPlayCount = m_MaxPlayCount;
#ifdef RAVEN_WEAPON_TAKER
						m_State = IS_DYING_BY_CRASH;
#else RAVEN_WEAPON_TAKER
						PlayEnd();
#endif RAVEN_WEAPON_TAKER
					}

					if( m_fAniTime > m_pAniData->fAniMaxTime )
					{
						m_NowPlayCount++;
						if( m_NowPlayCount == m_MaxPlayCount )
							PlayEnd();											

						m_fAniTime = 0.0f;
					}

					m_fFinalTime = m_pAniData->fAniMaxTime / m_fAniPlaySpeed * m_MaxPlayCount;
				}
				else if( m_TempletData.meshType == X_SKIN_MESH )
				{
					m_fAniTime	= m_pXSkinAnim->GetNowAnimationTime();

					if( m_TempletData.bCrashLand == true && 
						m_fAniTime > m_TempletData.fNotCheckLandTime &&
						m_vPos.y <= m_fLandPosition )
					{
						m_NowPlayCount = m_MaxPlayCount;
#ifdef RAVEN_WEAPON_TAKER
						m_State = IS_DYING_BY_CRASH;
#else RAVEN_WEAPON_TAKER
						PlayEnd();
#endif RAVEN_WEAPON_TAKER
					}

					if( m_pXSkinAnim->IsAnimationEnd() == true )
					{
						m_NowPlayCount++;
						if( m_NowPlayCount == m_MaxPlayCount )
							PlayEnd();					
					}
					m_fFinalTime = m_pXSkinAnim->GetMaxAnimationTime() / m_fAniPlaySpeed * m_MaxPlayCount;
				}
			}
			break;

		case LT_TIME:
			{
				if( m_NowLifeTime >= m_MaxLifeTime )
					PlayEnd();
				else if( m_TempletData.bCrashLand == true && 
					m_NowLifeTime > m_TempletData.fNotCheckLandTime &&
					m_vPos.y <= m_fLandPosition )
				{
					m_NowLifeTime = m_MaxLifeTime;
#ifdef RAVEN_WEAPON_TAKER
					m_State = IS_DYING_BY_CRASH;
#else RAVEN_WEAPON_TAKER
					PlayEnd();
#endif RAVEN_WEAPON_TAKER
				}
				else
					m_NowLifeTime += m_fElapsedTime;

				m_fFinalTime = m_MaxLifeTime;
			}
			break;

		case LT_MANUAL:
			m_fFinalTime = 999999.0f;
			break;
	}
}

void CKTDGXMeshPlayer::CXMeshInstance::DyingProcess()
{
    if( m_pXSkinAnim == NULL )
        return;

	m_fDieTimeBefore	= m_fDieTime;
	m_fDieTime			+= m_fElapsedTime;
	m_fNowResetCrash	+= m_fElapsedTime;
	m_fAniTimeBefore	= m_fAniTime;

	//다이 체크
	switch( m_TempletData.dieType )
	{
		case LT_PLAY_COUNT:
			{
				if( m_TempletData.meshType == X_MESH )
				{
					m_fAniTime	+= m_fElapsedTime * m_fAniPlaySpeed;
					if( m_fAniTime > m_pAniData->fAniMaxTime )
					{
						m_NowPlayCount++;
						if( m_NowDiePlayCount == m_MaxDiePlayCount )
							PlayEnd();

						m_fAniTime = 0.0f;
					}
					m_fFinalTime = m_pAniData->fAniMaxTime / m_fAniPlaySpeed * m_MaxDiePlayCount;
				}
				else if( m_TempletData.meshType == X_SKIN_MESH )
				{
					if( m_pXSkinAnim->IsAnimationEnd() == true )
					{
						m_NowDiePlayCount++;
						if( m_NowDiePlayCount == m_MaxDiePlayCount )
							PlayEnd();
					}
					m_fFinalTime = m_pXSkinAnim->GetMaxAnimationTime() / m_fAniPlaySpeed * m_MaxDiePlayCount;
				}
			}
			break;

		case LT_TIME:
			{
				if( m_NowDieLifeTime >= m_MaxDieLifeTime )
					m_State = IS_DIE;
				else
					m_NowDieLifeTime += m_fElapsedTime;

				m_fFinalTime = m_MaxDieLifeTime;
			}
			break;

		case LT_MANUAL:
			m_fFinalTime = 999999.0f;
			break;
	}
}


//{{ robobeg : 2008-10-24
/*virtual*/
RENDER_HINT   CKTDGXMeshPlayer::CXMeshInstance::OnFrameRender_Prepare()
{
    if ( m_State == IS_DIE || m_fDelayTime > 0.0f )
        return RENDER_HINT_NORENDER;

    if ( m_TempletData.meshType == X_SKIN_MESH && m_pXSkinAnim != NULL && m_DrawCount > 0 )
    {
        m_pXSkinAnim->SetInstantShow( false );
        if ( !m_pXSkinAnim->GetShowObject() )
            return RENDER_HINT_NORENDER;

        RENDER_HINT renderHint = m_pXSkinAnim->OnFrameRender_Prepare();
        if ( renderHint.m_eHint != RENDER_HINT::NORENDER )
            m_pXSkinAnim->SetInstantShow( true );
        return renderHint;

    }//if

    if ( m_TempletData.meshType == X_MESH && m_pXMesh != NULL && m_DrawCount > 0 )
        return RENDER_HINT_XMESH( m_TempletData.renderType, false );

    return RENDER_HINT_NORENDER;
}//CKTDGXMeshPlayer::OnFrameRender_Prepare()


void    CKTDGXMeshPlayer::CXMeshInstance::OnFrameRender_Draw()
{
#ifdef SHOW_ATTACK_BOX_DUMMY
	if( true == m_bShowAttackBox )
		CKTDGStateManager::SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
#endif // SHOW_ATTACK_BOX_DUMMY

#ifdef NOT_RENDER_EFFECT_MADE_BY_GAME_UNIT
	if( false == m_pMeshPlayer->GetRenderEffectMadeByGameUnit() && 
		CKTDGObject::OT_EFFECT_MADE_BY_GAME_UNIT == GetObjectType() )
		return;
#endif //NOT_RENDER_EFFECT_MADE_BY_GAME_UNIT

	if( m_TempletData.meshType == X_SKIN_MESH && m_pXSkinAnim != NULL && m_DrawCount > 0 )
	{
        if ( m_pXSkinAnim->GetShowObject() && m_pXSkinAnim->IsInstantShow() )
        {
		    for( int i = 0; i < m_DrawCount; i++ )
			    m_pXSkinAnim->OnFrameRender_Draw();
        }//if
	}
	else if( m_TempletData.meshType == X_MESH && m_pXMesh != NULL && m_DrawCount > 0 )
	{
//{{ robobeg : 20080-10-14
		//CKTDGXRenderer::RenderParam* pParam = g_pKTDXApp->GetDGManager()->GetXRenderer()->GetRenderParam();

        CKTDGXRenderer::RenderParam renderParam;

//{{ robobeg : 2008-10-24
		//renderParam.worldMatrix			= GetMatrix().GetMatrix( m_TempletData.billBoardType );
        D3DXMATRIX  mWorld = GetMatrix().GetMatrix( m_TempletData.billBoardType );
//}} robobeg : 2008-10-24
		renderParam.renderType			= m_TempletData.renderType;
		renderParam.cartoonTexType		= m_TempletData.cartoonTexType;
		renderParam.lightPos			= m_LightPos;
		renderParam.fOutLineWide		= GetDistanceToCamera() / m_fOutLineWide;
		renderParam.outLineColor		= m_OutLineColor;
		renderParam.color				= m_Color;
		renderParam.fLightFlowWide		= m_fLightFlowWide;
		renderParam.fLightFlowImpact	= m_fLightFlowImpact;
		renderParam.lightFlowPoint		= m_LightFlowPoint;
		renderParam.bAlphaBlend			= IsAlphaObject();
		renderParam.bZWriteEnable		= m_TempletData.bZWriteEnable;
		renderParam.bZEnable			= m_TempletData.bZEnable;
		renderParam.cullMode			= m_TempletData.cullMode;
		renderParam.srcBlend			= m_TempletData.srcBlend;
		renderParam.destBlend			= m_TempletData.destBlend;
		renderParam.texOffsetStage0.x	= m_vTexStage0UV.x;
		renderParam.texOffsetStage0.y	= m_vTexStage0UV.y;
		renderParam.texOffsetStage1.x	= m_vTexStage1UV.x;
		renderParam.texOffsetStage1.y	= m_vTexStage1UV.y;
		renderParam.texOffsetStage2.x	= m_vTexStage2UV.x;
        renderParam.texOffsetStage2.y	= m_vTexStage2UV.y;

        g_pKTDXApp->GetDGManager()->GetXRenderer()->OnFrameRender( renderParam, mWorld, *m_pXMesh, m_pChangeTexXET, m_pMultiTexXET, m_pAniData, m_fAniTime, m_DrawCount );
	}

#ifdef SHOW_ATTACK_BOX_DUMMY
	if( true == m_bShowAttackBox )
	{
		CKTDGStateManager::SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
		BOOST_TEST_FOREACH( CKTDXCollision::CollisionData*, pCollisionData, m_pXSkinAnim->GetAttackDataList() )
		{			
			//pCollisionData = m_pXSkinAnim->m_AttackDataList[i];
			if( pCollisionData->m_CollisionType == CKTDXCollision::CT_SPHERE )
			{
				if( NULL != m_pMatrixSphere )
				{
					m_pMatrixSphere->Move( pCollisionData->GetPointStart() );

					// x, y, z 스케일 고려해서 보여주기
					{
						D3DXVECTOR3 scale;
						if( pCollisionData->m_pCombineMatrix != NULL )
						{
							scale = GetDecomposeScale( pCollisionData->m_pCombineMatrix );
						}
						scale *= pCollisionData->m_fRadius * pCollisionData->m_fRadiusScale;

						m_pMatrixSphere->Scale( scale.x, scale.y, scale.z );
					}
					m_pMatrixSphere->UpdateWorldMatrix();
				}

				if( NULL != m_pXMeshSphere )
					m_pXMeshSphere->Render();
			}
			else if( pCollisionData->m_CollisionType == CKTDXCollision::CT_LINE )
			{
				D3DXVECTOR3 vPos = pCollisionData->GetPointStart();
				D3DXVECTOR3 vPosDelta = pCollisionData->GetPointEnd() - pCollisionData->GetPointStart();
				vPosDelta /= 29.f;
				for( int j=0; j<30; j++ )
				{
					if( NULL != m_pMatrixSphere )
					{
						m_pMatrixSphere->Move( vPos );
						float fScale = pCollisionData->GetScaleRadius();
						m_pMatrixSphere->Scale( 1, 1, 1 );
						m_pMatrixSphere->UpdateWorldMatrix();
					}

					if( NULL != m_pXMeshSphere )
						m_pXMeshSphere->Render();
					vPos += vPosDelta;
				}			
				//g_pKTDXApp->SetWorldTransform( pCollisionData->pCombineMatrix );
				//m_pSMesh->DrawSubset( 0 );
			}
		}
		CKTDGStateManager::SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	}
#endif // SHOW_ATTACK_BOX_DUMMY

//	return S_OK;
}

//}} robobeg : 2008-10-24

void CKTDGXMeshPlayer::CXMeshInstance::PlayEnd()
{
	// dmlee, 2009-10-28 usedieevent 기능 사용하지 않으므로 삭제
	//if( m_TempletData.bUseDieEvent == true && m_State == IS_PLAY )
	//	m_State = IS_DYING;
	//else
		m_State = IS_DIE;
}

void CKTDGXMeshPlayer::CXMeshInstance::OpenResource()
{
	// process any initial events
	CXMeshEvent*		pEvent		= NULL;
	CKTDXDeviceTexture* pTexture	= NULL;
	CKTDXDeviceSound*	pSound		= NULL;
	for( int i = 0; i < (int)m_pOrgTemplet->animTimeEventList.size(); i++ )
	{
		pEvent = (CXMeshEvent*)m_pOrgTemplet->animTimeEventList[i];
		switch( pEvent->GetEventType() )
		{
			case ET_SOUND:
				{
					CXMeshEvent_Sound* pEventSound = (CXMeshEvent_Sound*)pEvent;
					pSound = g_pKTDXApp->GetDeviceManager()->OpenSound( pEventSound->GetSoundName() );
					m_SoundMap.insert( std::make_pair(pSound->GetDeviceID(),pSound) );
				}
				break;
		}
	}
	for( int i = 0; i < (int)m_pOrgTemplet->globalTimeEventList.size(); i++ )
	{
		pEvent = (CXMeshEvent*)m_pOrgTemplet->globalTimeEventList[i];
		switch( pEvent->GetEventType() )
		{
			case ET_SOUND:
				{
					CXMeshEvent_Sound* pEventSound = (CXMeshEvent_Sound*)pEvent;
					pSound = g_pKTDXApp->GetDeviceManager()->OpenSound( pEventSound->GetSoundName() );
					m_SoundMap.insert( std::make_pair(pSound->GetDeviceID(),pSound) );
				}
				break;
		}
	}
	for( int i = 0; i < (int)m_pOrgTemplet->dieEventList.size(); i++ )
	{
		pEvent = (CXMeshEvent*)m_pOrgTemplet->dieEventList[i];
		switch( pEvent->GetEventType() )
		{
			case ET_SOUND:
				{
					CXMeshEvent_Sound* pEventSound = (CXMeshEvent_Sound*)pEvent;
					pSound = g_pKTDXApp->GetDeviceManager()->OpenSound( pEventSound->GetSoundName() );
					m_SoundMap.insert( std::make_pair(pSound->GetDeviceID(),pSound) );
				}
				break;
		}
	}
}

void CKTDGXMeshPlayer::CXMeshInstance::RunEvent()
{
    if( m_pXSkinAnim == NULL )
        return;

	if( m_State == IS_PLAY )
	{
		//애니메이션 타임 이벤트
		for( int i = 0; i < (int)m_pOrgTemplet->animTimeEventList.size(); i++ )
		{
			CXMeshEvent* pEvent = m_pOrgTemplet->animTimeEventList[i];
			if( m_TempletData.meshType == X_MESH )
			{
				if( m_pAniData != NULL )
					pEvent->SetFinalTime( m_pAniData->fAniMaxTime );
				else
					pEvent->SetFinalTime( m_fFinalTime );
			}
			else if( m_TempletData.meshType == X_SKIN_MESH )
			{
				pEvent->SetFinalTime( m_pXSkinAnim->GetMaxAnimationTime() );
			}			

			if( pEvent->IsFade() == true )
			{
				if( m_fAniTime >= pEvent->GetActualTime().m_Min 
					&& m_fAniTime < pEvent->GetActualTime().m_Max )
				{
					pEvent->OnFrameMove( this, m_fElapsedTime, m_fAniTime );
				}
			}
			else
			{
				if( (m_fAniTimeBefore < pEvent->GetActualTime().m_Min 
					&& m_fAniTime >= pEvent->GetActualTime().m_Min)
					|| (pEvent->GetActualTime().m_Min == 0.0f && m_fAniTimeBefore == 0.0f ) )
				{
					pEvent->OnFrameMove( this, m_fElapsedTime, m_fAniTime );
				}
			}
		}
		//글로벌 타임 이벤트
		for( int i = 0; i < (int)m_pOrgTemplet->globalTimeEventList.size(); i++ )
		{
			CXMeshEvent* pEvent = m_pOrgTemplet->globalTimeEventList[i];
			pEvent->SetFinalTime( m_fFinalTime );

			if( pEvent->IsFade() == true )
			{
				if( m_fGlobalTime >= pEvent->GetActualTime().m_Min 
					&& m_fGlobalTime < pEvent->GetActualTime().m_Max )
				{				
					pEvent->OnFrameMove( this, m_fElapsedTime, m_fGlobalTime );
				}
			}
			else
			{
				if( (m_fGlobalTimeBefore < pEvent->GetActualTime().m_Min 
					&& m_fGlobalTime >= pEvent->GetActualTime().m_Min)
					|| (pEvent->GetActualTime().m_Min == 0.0f && m_fGlobalTime == 0.0f ) )
				{
					pEvent->OnFrameMove( this, m_fElapsedTime, m_fGlobalTime );
				}
			}
		}
	}
	else if( m_State == IS_DYING )
	{
		//애니메이션 타임 이벤트
		for( int i = 0; i < (int)m_pOrgTemplet->animTimeEventList.size(); i++ )
		{
			CXMeshEvent* pEvent = m_pOrgTemplet->animTimeEventList[i];
			if( m_TempletData.meshType == X_MESH )
			{
				if( m_pAniData != NULL )
					pEvent->SetFinalTime( m_pAniData->fAniMaxTime );
				else
					pEvent->SetFinalTime( m_fFinalTime );
			}
			else if( m_TempletData.meshType == X_SKIN_MESH )
			{
				pEvent->SetFinalTime( m_pXSkinAnim->GetMaxAnimationTime() );
			}			

			if( pEvent->IsFade() == true )
			{
				if( m_fAniTime >= pEvent->GetActualTime().m_Min 
					&& m_fAniTime < pEvent->GetActualTime().m_Max )
				{
					pEvent->OnFrameMove( this, m_fElapsedTime, m_fAniTime );
				}
			}
			else
			{
				if( (m_fAniTimeBefore < pEvent->GetActualTime().m_Min 
					&& m_fAniTime >= pEvent->GetActualTime().m_Min)
					|| (pEvent->GetActualTime().m_Min == 0.0f && m_fAniTimeBefore == 0.0f ) )
				{
					pEvent->OnFrameMove( this, m_fElapsedTime, m_fAniTime );
				}
			}
		}
		//다잉 타임 이벤트
		for( int i = 0; i < (int)m_pOrgTemplet->dieEventList.size(); i++ )
		{
			CXMeshEvent* pEvent = m_pOrgTemplet->dieEventList[i];
			pEvent->SetFinalTime( m_fFinalTime );

			if( pEvent->IsFade() == true )
			{
				if( m_fDieTime >= pEvent->GetActualTime().m_Min 
					&& m_fDieTime < pEvent->GetActualTime().m_Max )
				{
					pEvent->OnFrameMove( this, m_fElapsedTime, m_fDieTime );
				}
			}
			else
			{
				if( (m_fDieTimeBefore < pEvent->GetActualTime().m_Min 
					&& m_fDieTime >= pEvent->GetActualTime().m_Min)
					|| (pEvent->GetActualTime().m_Min == 0.0f && m_fDieTimeBefore == 0.0f ) )
				{
					pEvent->OnFrameMove( this, m_fElapsedTime, m_fDieTime );
				}
			}
		}
	}
}

void  CKTDGXMeshPlayer::CXMeshInstance::ChangeXSkinMesh( const WCHAR* wszMeshName, const WCHAR* wszChangeTexXETName, 
														const WCHAR* wszMultiTexXETName, const WCHAR* wszAniXETName )
{

	if ( m_TempletData.meshType == X_SKIN_MESH )
	{
		SAFE_CLOSE( m_pXSkinMesh );
		SAFE_CLOSE( m_pChangeTexXET );
		SAFE_CLOSE( m_pMultiTexXET );
		SAFE_CLOSE( m_pAniXET );

		m_pXSkinMesh		= g_pKTDXApp->GetDeviceManager()->OpenXSkinMesh( wszMeshName );
		m_pChangeTexXET		= g_pKTDXApp->GetDeviceManager()->OpenXET( wszChangeTexXETName );
		if ( wszMultiTexXETName != NULL)
			m_pMultiTexXET		= g_pKTDXApp->GetDeviceManager()->OpenXET( wszMultiTexXETName );
		m_pAniXET			= g_pKTDXApp->GetDeviceManager()->OpenXET( wszAniXETName );
		
		m_pAniData			= NULL;

		if( m_pXSkinMesh != NULL )
		{
			SAFE_DELETE_KTDGOBJECT( m_pXSkinAnim );

			m_pXSkinAnim = CKTDGXSkinAnim::CreateSkinAnim();

			m_pXSkinAnim->SetAnimXSkinMesh( m_pXSkinMesh, m_pAniXET );
			m_pXSkinAnim->AddModelXSkinMesh( m_pXSkinMesh, m_pAniXET, m_pMultiTexXET, m_pChangeTexXET );

			m_pXSkinAnim->GetMatrix().Move( m_vPos );

			SetCenter( m_pXSkinMesh->GetCenter() );
			SetBoundingRadius( m_pXSkinMesh->GetBoundingRadius() );

			RunEvent();
		}
	}
}


bool CKTDGXMeshPlayer::CXMeshInstance::EventTimerGlobal( float fTime )
{
#ifdef ELSWORD_SHEATH_KNIGHT
	if( m_fGlobalTimeBefore <= fTime && m_fGlobalTime > fTime )
		return true;
	else if( fTime >= 0 &&										
		fTime < m_fElapsedTime && m_fGlobalTimeBefore - m_fElapsedTime <= fTime )
		return true;
	else
		return false;

#else
	if( m_fGlobalTime <= fTime && m_fGlobalTime + m_fElapsedTime > fTime  )
		return true;
	else
		return false;
#endif ELSWORD_SHEATH_KNIGHT
}

//{{ kimhc // 2009-09-23 // NOT_CHECK_LAND_TIME을 설정하는 함수
void CKTDGXMeshPlayer::CXMeshInstance::SetNotCheckLandTime( float fTime )
{
	m_TempletData.fNotCheckLandTime = fTime;
}

#ifdef NEW_HENIR_TEST
void CKTDGXMeshPlayer::CXMeshInstance::ChangeAnim_LUA( char* cAnimName, CKTDGXSkinAnim::XSKIN_ANIM_PLAYTYPE ePlayType, float fAnimSpeed )
{
	wstring wstrAnimName;
	ConvertUtf8ToWCHAR( wstrAnimName, cAnimName );
	ChangeAnim(wstrAnimName.c_str(), ePlayType, fAnimSpeed);
	return;
}

void CKTDGXMeshPlayer::CXMeshInstance::ChangeAnim( const WCHAR* wcAnimName, CKTDGXSkinAnim::XSKIN_ANIM_PLAYTYPE ePlayType, float fAnimSpeed )
{
	m_pXSkinAnim->ChangeAnim( wcAnimName );
	m_pXSkinAnim->Play( ePlayType );
	m_pXSkinAnim->SetPlaySpeed( fAnimSpeed );
	return;
}
#endif NEW_HENIR_TEST


#ifdef BOOST_SINGLETON_POOL_TEST

	typedef boost::singleton_pool<boost::pool_allocator_tag, sizeof(CKTDGXMeshPlayer::CXMeshInstance)> CXMeshInstancePool;
	
	void* CKTDGXMeshPlayer::CXMeshInstance::operator new(size_t s)
	{
		return CXMeshInstancePool::malloc();
	}

	void CKTDGXMeshPlayer::CXMeshInstance::operator delete(void *p)
	{
		CXMeshInstancePool::free(p);
	}
#endif BOOST_SINGLETON_POOL_TEST



///////////////////////////////////////////////////////////////////////////////
// MeshPlayer Event
///////////////////////////////////////////////////////////////////////////////

void CKTDGXMeshPlayer::CXMeshEvent::ProcessPropEqualsValue(CMinMax<float> &prop,
											std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
											std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	// next token should be =
	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);
	if (TokenIter->m_Type != Equals) throw("Expecting = after property.");

	// next token should be a number
	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);
	CXMeshPlayerTokenizer::ProcessNumber(prop, TokenIter, EndIter);
}

void CKTDGXMeshPlayer::CXMeshEvent::ProcessPropEqualsValue(CMinMax<D3DXVECTOR3> &prop,
											std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
											std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	// next token should be =
	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);
	if (TokenIter->m_Type != Equals) throw("Expecting = after property.");

	// next token should be a number
	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);
	CXMeshPlayerTokenizer::ProcessVector3(prop, TokenIter, EndIter);
}

void CKTDGXMeshPlayer::CXMeshEvent::ProcessPropEqualsValue(CMinMax<D3DXVECTOR2> &prop,
											std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
											std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	// next token should be =
	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);
	if (TokenIter->m_Type != Equals) throw("Expecting = after property.");

	// next token should be a number
	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);
	CXMeshPlayerTokenizer::ProcessVector2(prop, TokenIter, EndIter);
}

void CKTDGXMeshPlayer::CXMeshEvent::ProcessPropEqualsValue(CMinMax<D3DXCOLOR> &prop,
											std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
											std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	// next token should be =
	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);
	if (TokenIter->m_Type != Equals) throw("Expecting = after property.");

	// next token should be a number
	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);
	CXMeshPlayerTokenizer::ProcessColor(prop, TokenIter, EndIter);
}

void CKTDGXMeshPlayer::CXMeshEvent::ProcessPropEqualsValue(wstring &prop,
											std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
											std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
	if (TokenIter->m_Type != Equals) throw("Expected equals sign!");
	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
	CXMeshPlayerTokenizer::ProcessString(prop, TokenIter, EndIter);
}


void CKTDGXMeshPlayer::CXMeshEvent::ProcessPropEqualsValue(bool &prop,
														   std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
														   std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
	if (TokenIter->m_Type != Equals) throw("Expected equals sign!");
	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
	CXMeshPlayerTokenizer::ProcessBool(prop, TokenIter, EndIter);
}

void CKTDGXMeshPlayer::CXMeshEvent::ProcessPropEqualsValue(CKTDGXRenderer::RENDER_TYPE &prop,
														   std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
														   std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
	if (TokenIter->m_Type != Equals) throw("Expected equals sign!");
	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 
	CXMeshPlayerTokenizer::ProcessRenderType(prop, TokenIter, EndIter);
}



void CKTDGXMeshPlayer::CXMeshEvent_AniName::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
    //{{ seojt // 2009-1-12, 14:23
    ASSERT( pInstance != NULL );
    if( pInstance == NULL || pInstance->m_pXSkinAnim == NULL )
    {
        return;
    }//if
    //}} seojt // 2009-1-12, 14:23

	if( pInstance->m_TempletData.meshType == X_MESH )
	{		
		pInstance->m_fAniTime = 0.0f;
		if( pInstance->m_pAniXET != NULL )
			pInstance->m_pAniData = pInstance->m_pAniXET->GetAniData( m_AniName.c_str() );
	}
	else if( pInstance->m_TempletData.meshType == X_SKIN_MESH )
	{
		pInstance->m_pXSkinAnim->ChangeAnim( m_AniName.c_str() );
		pInstance->m_pXSkinAnim->Play( m_PlayType );
#ifdef EVENT_SCENE
		//pInstance->m_pXSkinAnim->SetPlaySpeed( m_fPlaySpeed );
#endif EVENT_SCENE		

	}
}

bool CKTDGXMeshPlayer::CXMeshEvent_AniName::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
																		std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("ANINAME") != 0) throw("Expecting ANINAME!");
	ProcessPropEqualsValue(m_AniName, TokenIter, EndIter);
	
	if (TokenIter->m_Type != Comma) throw("Expected comma sign!");
	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter); 

	if( TokenIter->m_strValue.compare( "XAP_LOOP" ) == 0 )
		m_PlayType = CKTDGXSkinAnim::XAP_LOOP;
	else if( TokenIter->m_strValue.compare( "XAP_ONE" ) == 0 )
		m_PlayType = CKTDGXSkinAnim::XAP_ONE;
	else if( TokenIter->m_strValue.compare( "XAP_ONE_WAIT" ) == 0 )
		m_PlayType = CKTDGXSkinAnim::XAP_ONE_WAIT;
	else
	{
		throw("Invalid AniName!");
		return(false);
	}

	CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);

	return(true);
}


void CKTDGXMeshPlayer::CXMeshEvent_SlashTrace::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
	pInstance->m_bSlashTrace = m_bSlashTrace;
}

bool CKTDGXMeshPlayer::CXMeshEvent_SlashTrace::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
																  std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("SLASHTRACE") != 0) throw("Expecting SLASHTRACE!");
	ProcessPropEqualsValue(m_bSlashTrace, TokenIter, EndIter);
	return(true);
}



void CKTDGXMeshPlayer::CXMeshEvent_AniSpeed::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
	pInstance->m_fAniPlaySpeed = m_AniSpeed.GetRandomNumInRange();
}

bool CKTDGXMeshPlayer::CXMeshEvent_AniSpeed::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
																	   std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("ANISPEED") != 0) throw("Expecting ANISPEED!");
	ProcessPropEqualsValue(m_AniSpeed, TokenIter, EndIter);
	return(true);
}

void CKTDGXMeshPlayer::CXMeshEvent_Color::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
	float fRemainTime = m_ActualTime.m_Max - fNowTime;

	if( !IsFade() || fRemainTime == 0 ) 
		pInstance->m_Color = m_Color.GetRandomNumInRange();
	else
	{
		if( (fNowTime - m_ActualTime.m_Min) <= fElapsedTime )
			pInstance->m_ColorFinal = m_Color.GetRandomNumInRange();

		if( (m_ActualTime.m_Max - fNowTime) < fElapsedTime )
			pInstance->m_Color = pInstance->m_ColorFinal;
		else
		{
			D3DXCOLOR	remainColor = pInstance->m_ColorFinal - pInstance->m_Color;
			pInstance->m_Color += (remainColor / fRemainTime) * fElapsedTime;
		}
	}
}

bool CKTDGXMeshPlayer::CXMeshEvent_Color::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
																		std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("COLOR") != 0) throw("Expecting COLOR!");
	ProcessPropEqualsValue(m_Color, TokenIter, EndIter);
	return(true);
}

void CKTDGXMeshPlayer::CXMeshEvent_OutLineColor::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
	float fRemainTime = m_ActualTime.m_Max - fNowTime;

	if( !IsFade() || fRemainTime == 0 ) 
		pInstance->m_OutLineColor = m_OutLineColor.GetRandomNumInRange();
	else
	{
		if( (fNowTime - m_ActualTime.m_Min) <= fElapsedTime )
			pInstance->m_OutLineColorFinal = m_OutLineColor.GetRandomNumInRange();

		if( (m_ActualTime.m_Max - fNowTime) < fElapsedTime )
			pInstance->m_OutLineColor = pInstance->m_OutLineColorFinal;
		else
		{
			D3DXCOLOR	remainOutLineColor = pInstance->m_OutLineColorFinal - pInstance->m_OutLineColor;
			pInstance->m_OutLineColor += (remainOutLineColor / fRemainTime) * fElapsedTime;
		}
	}
}

bool CKTDGXMeshPlayer::CXMeshEvent_OutLineColor::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
															 std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("OUTLINECOLOR") != 0) throw("Expecting OUTLINECOLOR!");
	ProcessPropEqualsValue(m_OutLineColor, TokenIter, EndIter);
	return(true);
}

#ifdef EVENT_SCENE
bool CKTDGXMeshPlayer::CXMeshEvent_Position::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
																	  std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("POSITION") != 0) throw("Expecting position!");
	ProcessPropEqualsValue(m_Position, TokenIter, EndIter);
	return(true);
}

void CKTDGXMeshPlayer::CXMeshEvent_Position::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
	float fRemainTime = m_ActualTime.m_Max - fNowTime;

	if( !IsFade() || fRemainTime == 0 ) 
	{
		pInstance->m_vPos = m_Position.GetRandomNumInRange();
		INIT_VECTOR3( pInstance->m_vPositionGap, 0.0f, 0.0f, 0.0f );
	}
	else
	{		
		if( (fNowTime - m_ActualTime.m_Min) <= fElapsedTime )
		{
			pInstance->m_vPositionFinal = m_Position.GetRandomNumInRange();
		}

		if( (m_ActualTime.m_Max - fNowTime) < fElapsedTime )
		{
			pInstance->m_vPos = m_Position.GetRandomNumInRange();
			INIT_VECTOR3( pInstance->m_vPositionGap, 0.0f, 0.0f, 0.0f );
		}
		else
		{
			D3DXVECTOR3	remainPositionGap	= pInstance->m_vPositionFinal - pInstance->m_vPos;
			pInstance->m_vPositionGap = (remainPositionGap / fRemainTime) * fElapsedTime;
		}
	}
}
#endif EVENT_SCENE

void CKTDGXMeshPlayer::CXMeshEvent_Size::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
	float fRemainTime = m_ActualTime.m_Max - fNowTime;

	if( !IsFade() || fRemainTime == 0 ) 
        pInstance->m_vSize = m_Size.GetRandomNumInRange();
	else
	{
		if( (fNowTime - m_ActualTime.m_Min) <= fElapsedTime )
			pInstance->m_vSizeFinal = m_Size.GetRandomNumInRange();

		if( (m_ActualTime.m_Max - fNowTime) < fElapsedTime )
			pInstance->m_vSize = pInstance->m_vSizeFinal;
		else
		{
			D3DXVECTOR3	remainSize = pInstance->m_vSizeFinal - pInstance->m_vSize;
			pInstance->m_vSize += (remainSize / fRemainTime) * fElapsedTime;
		}
	}
}

bool CKTDGXMeshPlayer::CXMeshEvent_Size::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
																std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("SIZE") != 0) throw("Expecting SIZE!");
	ProcessPropEqualsValue(m_Size, TokenIter, EndIter);
	return(true);
}

void CKTDGXMeshPlayer::CXMeshEvent_Velocity::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
	float fRemainTime = m_ActualTime.m_Max - fNowTime;

	if( !IsFade() || fRemainTime == 0 ) 
		pInstance->m_vVelocity = m_Velocity.GetRandomNumInRange();
	else
	{
		if( (fNowTime - m_ActualTime.m_Min) <= fElapsedTime )
			pInstance->m_vVelocityFinal = m_Velocity.GetRandomNumInRange();

		if( (m_ActualTime.m_Max - fNowTime) < fElapsedTime )
			pInstance->m_vVelocity = pInstance->m_vVelocityFinal;
		else
		{
			D3DXVECTOR3 remainVelocity = pInstance->m_vVelocityFinal - pInstance->m_vVelocity;
			pInstance->m_vVelocity += (remainVelocity / fRemainTime) * fElapsedTime;
		}
	}
}

bool CKTDGXMeshPlayer::CXMeshEvent_Velocity::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
															std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("VELOCITY") != 0) throw("Expecting VELOCITY!");
	ProcessPropEqualsValue(m_Velocity, TokenIter, EndIter);
	return(true);
}

void CKTDGXMeshPlayer::CXMeshEvent_Tex0UV::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
	float fRemainTime = m_ActualTime.m_Max - fNowTime;

	if( !IsFade() || fRemainTime == 0 ) 
		pInstance->m_vTexStage0UV = m_Tex0UV.GetRandomNumInRange();
	else
	{
		if( (fNowTime - m_ActualTime.m_Min) <= fElapsedTime )
			pInstance->m_vTexStage0UVFinal = m_Tex0UV.GetRandomNumInRange();

		if( (m_ActualTime.m_Max - fNowTime) < fElapsedTime )
			pInstance->m_vTexStage0UV = pInstance->m_vTexStage0UVFinal;
		else
		{
			D3DXVECTOR2 remainTex0UV = pInstance->m_vTexStage0UVFinal - pInstance->m_vTexStage0UV;
			pInstance->m_vTexStage0UV += (remainTex0UV / fRemainTime) * fElapsedTime;
		}
	}
}

bool CKTDGXMeshPlayer::CXMeshEvent_Tex0UV::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
																std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("TEX0UV") != 0) throw("Expecting TEX0UV!");
	ProcessPropEqualsValue(m_Tex0UV, TokenIter, EndIter);
	return(true);
}

void CKTDGXMeshPlayer::CXMeshEvent_Tex1UV::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
	float fRemainTime = m_ActualTime.m_Max - fNowTime;

	if( !IsFade() || fRemainTime == 0 ) 
		pInstance->m_vTexStage1UV = m_Tex1UV.GetRandomNumInRange();
	else
	{
		if( (fNowTime - m_ActualTime.m_Min) <= fElapsedTime )
			pInstance->m_vTexStage1UVFinal = m_Tex1UV.GetRandomNumInRange();

		if( (m_ActualTime.m_Max - fNowTime) < fElapsedTime )
			pInstance->m_vTexStage1UV = pInstance->m_vTexStage1UVFinal;
		else
		{
			D3DXVECTOR2 remainTex1UV = pInstance->m_vTexStage1UVFinal - pInstance->m_vTexStage1UV;
			pInstance->m_vTexStage1UV += (remainTex1UV / fRemainTime) * fElapsedTime;
		}
	}
}

bool CKTDGXMeshPlayer::CXMeshEvent_Tex1UV::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
															  std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("TEX1UV") != 0) throw("Expecting TEX1UV!");
	ProcessPropEqualsValue(m_Tex1UV, TokenIter, EndIter);
	return(true);
}

void CKTDGXMeshPlayer::CXMeshEvent_Tex2UV::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
	float fRemainTime = m_ActualTime.m_Max - fNowTime;

	if( !IsFade() || fRemainTime == 0 ) 
		pInstance->m_vTexStage2UV = m_Tex2UV.GetRandomNumInRange();
	else
	{
		if( (fNowTime - m_ActualTime.m_Min) <= fElapsedTime )
			pInstance->m_vTexStage2UVFinal = m_Tex2UV.GetRandomNumInRange();

		if( (m_ActualTime.m_Max - fNowTime) < fElapsedTime )
			pInstance->m_vTexStage2UV = pInstance->m_vTexStage2UVFinal;
		else
		{
			D3DXVECTOR2 remainTex2UV = pInstance->m_vTexStage2UVFinal - pInstance->m_vTexStage2UV;
			pInstance->m_vTexStage2UV += (remainTex2UV / fRemainTime) * fElapsedTime;
		}
	}
}

bool CKTDGXMeshPlayer::CXMeshEvent_Tex2UV::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
															  std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("TEX2UV") != 0) throw("Expecting TEX2UV!");
	ProcessPropEqualsValue(m_Tex2UV, TokenIter, EndIter);
	return(true);
}

void CKTDGXMeshPlayer::CXMeshEvent_Rotate::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
	float fRemainTime = m_ActualTime.m_Max - fNowTime;	

	if( !IsFade() || fRemainTime == 0 ) 
	{
#ifdef LOCAL_ROTATE_EFFECT
		pInstance->m_vRotateLocal = m_Rotate.GetRandomNumInRange();
#else
		pInstance->m_vRotate = m_Rotate.GetRandomNumInRange();
#endif
	}
	else
	{
		if( (fNowTime - m_ActualTime.m_Min) <= fElapsedTime )
			pInstance->m_vRotateFinal = m_Rotate.GetRandomNumInRange();

		if( (m_ActualTime.m_Max - fNowTime) < fElapsedTime )
		{
#ifdef LOCAL_ROTATE_EFFECT
			pInstance->m_vRotateLocal = pInstance->m_vRotateFinal;
#else
			pInstance->m_vRotate = pInstance->m_vRotateFinal;
#endif
		}
		else
		{
#ifdef LOCAL_ROTATE_EFFECT
			D3DXVECTOR3 remainRotate = pInstance->m_vRotateFinal - pInstance->m_vRotateLocal;
			pInstance->m_vRotateLocal += (remainRotate / fRemainTime) * fElapsedTime;
#else
			D3DXVECTOR3 remainRotate = pInstance->m_vRotateFinal - pInstance->m_vRotate;
			pInstance->m_vRotate += (remainRotate / fRemainTime) * fElapsedTime;
#endif
			
		}
	}
}

bool CKTDGXMeshPlayer::CXMeshEvent_Rotate::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
															  std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("ROTATE") != 0) throw("Expecting ROTATE!");
	ProcessPropEqualsValue(m_Rotate, TokenIter, EndIter); 
	return(true);
}


void CKTDGXMeshPlayer::CXMeshEvent_DirSpeed::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
	float fRemainTime = m_ActualTime.m_Max - fNowTime;

	if( !IsFade() || fRemainTime == 0 ) 
		pInstance->m_fDirSpeed = m_DirSpeed.GetRandomNumInRange();
	else
	{
		if( (fNowTime - m_ActualTime.m_Min) <= fElapsedTime )
			pInstance->m_fDirSpeedFinal = m_DirSpeed.GetRandomNumInRange();

		if( (m_ActualTime.m_Max - fNowTime) < fElapsedTime )
			pInstance->m_fDirSpeed = pInstance->m_fDirSpeedFinal;
		else
		{
			float remainDirSpeed = pInstance->m_fDirSpeedFinal - pInstance->m_fDirSpeed;
			pInstance->m_fDirSpeed += (remainDirSpeed / fRemainTime) * fElapsedTime;
		}
	}
}

bool CKTDGXMeshPlayer::CXMeshEvent_DirSpeed::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
															  std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("DIRSPEED") != 0) throw("Expecting DIRSPEED!");
	ProcessPropEqualsValue(m_DirSpeed, TokenIter, EndIter);
	return(true);
}

void CKTDGXMeshPlayer::CXMeshEvent_BlackHoleTime::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
	pInstance->m_BlackHoleTime = m_BlackHoleTime.GetRandomNumInRange();
}

bool CKTDGXMeshPlayer::CXMeshEvent_BlackHoleTime::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
																std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("BLACKHOLETIME") != 0) throw("Expecting BLACKHOLETIME!");
	ProcessPropEqualsValue(m_BlackHoleTime, TokenIter, EndIter);
	return(true);
}

void CKTDGXMeshPlayer::CXMeshEvent_Crash::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
	if( pInstance->m_fNowResetCrash < pInstance->m_fResetCrash )
	{
		return;
	}
	else
	{
		pInstance->m_fNowResetCrash = 0.0f;
	}
	
	if( (m_ActualTime.m_Max - fNowTime) < fElapsedTime )
		pInstance->m_Crash = D3DXVECTOR3( 0.0f,0.0f,0.0f );
	else
		pInstance->m_Crash = m_Crash.GetRandomNumInRange();
}

bool CKTDGXMeshPlayer::CXMeshEvent_Crash::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
																	 std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("CRASH") != 0) throw("Expecting CRASH!");
	ProcessPropEqualsValue(m_Crash, TokenIter, EndIter);
	return(true);
}


void CKTDGXMeshPlayer::CXMeshEvent_ResetCrash::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
	pInstance->m_fResetCrash = m_ResetCrash.GetRandomNumInRange();
}

bool CKTDGXMeshPlayer::CXMeshEvent_ResetCrash::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
															 std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("RESETCRASH") != 0) throw("Expecting RESETCRASH!");
	ProcessPropEqualsValue(m_ResetCrash, TokenIter, EndIter);
	return(true);
}

void CKTDGXMeshPlayer::CXMeshEvent_LightFlowImpact::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
	float fRemainTime = m_ActualTime.m_Max - fNowTime;

	if( !IsFade() || fRemainTime == 0 ) 
		pInstance->m_fLightFlowImpact = m_LightFlowImpact.GetRandomNumInRange();
	else
	{
		if( (fNowTime - m_ActualTime.m_Min) <= fElapsedTime )
			pInstance->m_fLightFlowImpactFinal = m_LightFlowImpact.GetRandomNumInRange();

		if( (m_ActualTime.m_Max - fNowTime) < fElapsedTime )
			pInstance->m_fLightFlowImpact = pInstance->m_fLightFlowImpactFinal;
		else
		{
			float remainLightFlowImpact = pInstance->m_fLightFlowImpactFinal - pInstance->m_fLightFlowImpact;
			pInstance->m_fLightFlowImpact += (remainLightFlowImpact / fRemainTime) * fElapsedTime;
		}
	}
}

bool CKTDGXMeshPlayer::CXMeshEvent_LightFlowImpact::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
															  std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("LIGHTFLOWIMPACT") != 0) throw("Expecting LIGHTFLOWIMPACT!");
	ProcessPropEqualsValue(m_LightFlowImpact, TokenIter, EndIter);
	return(true);
}


void CKTDGXMeshPlayer::CXMeshEvent_LightFlowPoint::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
	float fRemainTime = m_ActualTime.m_Max - fNowTime;

	if( !IsFade() || fRemainTime == 0 ) 
		pInstance->m_LightFlowPoint = m_LightFlowPoint.GetRandomNumInRange();
	else
	{
		if( (fNowTime - m_ActualTime.m_Min) <= fElapsedTime )
			pInstance->m_LightFlowPointFinal = m_LightFlowPoint.GetRandomNumInRange();

		if( (m_ActualTime.m_Max - fNowTime) < fElapsedTime )
			pInstance->m_LightFlowPoint = pInstance->m_LightFlowPointFinal;
		else
		{
			D3DXVECTOR3 remainLightFlowPoint = pInstance->m_LightFlowPointFinal - pInstance->m_LightFlowPoint;
			pInstance->m_LightFlowPoint += (remainLightFlowPoint / fRemainTime) * fElapsedTime;
		}
	}
}

bool CKTDGXMeshPlayer::CXMeshEvent_LightFlowPoint::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
																	   std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("LIGHTFLOWPOINT") != 0) throw("Expecting LIGHTFLOWPOINT!");
	ProcessPropEqualsValue(m_LightFlowPoint, TokenIter, EndIter);
	return(true);
}

void CKTDGXMeshPlayer::CXMeshEvent_Sound::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
	map<wstring,CKTDXDeviceSound*>::iterator iter;
	iter = pInstance->m_SoundMap.find( m_SoundName );
	if( iter != pInstance->m_SoundMap.end() )
	{
		CKTDXDeviceSound* pSound = iter->second;
		pSound->Set3DPosition( pInstance->m_vPos );
		pSound->Play();
	}
}

bool CKTDGXMeshPlayer::CXMeshEvent_Sound::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
																	  std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("SOUND") != 0) throw("Expecting SOUND!");
	ProcessPropEqualsValue(m_SoundName, TokenIter, EndIter);
	return(true);
}

void CKTDGXMeshPlayer::CXMeshEvent_RenderType::OnFrameMove( CXMeshInstance* pInstance, float fElapsedTime, float fNowTime )
{
	float fRemainTime = m_ActualTime.m_Max - fNowTime;

	pInstance->GetTempletData()->renderType = m_RenderType;
}

bool CKTDGXMeshPlayer::CXMeshEvent_RenderType::ProcessTokenStream(std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
																  std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_strValue.compare("RENDER_TYPE") != 0) throw("Expecting RENDER_TYPE!");
	ProcessPropEqualsValue(m_RenderType, TokenIter, EndIter);
	return(true);
}


///////////////////////////////////////////////////////////////////////////////
// CXMeshPlayerTokenizer
///////////////////////////////////////////////////////////////////////////////

void CKTDGXMeshPlayer::CXMeshPlayerTokenizer::Tokenize( const char *str, const int iLength /*= -1*/ )
{
	enum CurState 
	{
		InWhiteSpace = 1,
		InText,
		InQuote,
		InComment
	};

	CurState				cs		= InWhiteSpace;
	const char*				p		= str;
	CXMeshPlayerToken		token;


#ifdef _ENCRIPT_SCRIPT_
	while( (*p) != 0 ) 
#else
	ASSERT( -1 != iLength ); 
	int iCurrPos = 0;
	while( (*p) != 0 && iCurrPos < iLength ) 
#endif _ENCRIPT_SCRIPT_
	{

		ASSERT( InComment == cs || *p != ';' );


		switch( cs ) 
		{
		case InWhiteSpace:
			{
				// if this letter is not whitespace,
				if( !isspace(*p) ) 
				{
					// add it to the running buffer
					token.m_strValue = (*p);
					// switch to appropriate case
					if( (*p) == '/' && *(p+1) == '/' ) // '//'는 주석
						cs = InComment;
					else if( (*p) == '-' && *(p+1) == '-' ) // '--'도 주석
						cs = InComment;
					else 
						cs = (*p == '\"') ? InQuote : InText;
				} // end if
			}
			break;

		case InText:
			{
				// if this letter is whitespace
				if( isspace(*p) ) 
				{
					// add the completed token to the vector
					AddToken(token);
					// switch to whitespace case
					cs = InWhiteSpace;
				} 
				else
				{ 
					// if this letter is a token terminator
					if ((*p) == '(' || (*p) == ')' || (*p) == ',' || (*p) == '\"' || (*p) == '{' || (*p) == '}' || (*p) == '/') 
					{
						if ((*p) == '/' && *(p+1) == '/') 
							cs = InComment;
						else 
						{
							// add the completed token to the vector
							DetermineTokenType(token);
							AddToken(token);

							// if it was a quote, transition to InQuote state
							if ((*p) == '\"') 
								cs = InQuote; 
							// otherwise, process this one char as a token 
							else 
							{
								token.m_strValue = (*p);
								AddToken(token);
								token.m_strValue = "";	
							}
						}
					} 
					else
					{
						// add this letter to the work in progress token
						token.m_strValue += (*p);
					} // end if
				} // end if
			}
			break;

		case InComment:
			{
				// C++ style comments - skip everything until end of line
				if (*p == '\n') 
				{
					token.m_strValue = ""; 
					cs = InWhiteSpace;
				}
			}
			break;

		case InQuote:
			{
				// unconditionally add this letter to the token until we hit a close quote
				token.m_strValue += (*p);
				if (*p == '\"') 
				{
					AddToken(token);
					// go back to whitespace
					cs = InWhiteSpace;
				}
			}
			break;
		}
		p++;
#ifdef _ENCRIPT_SCRIPT_
#else
		iCurrPos++;
#endif _ENCRIPT_SCRIPT_

	}
	AddToken(token);
}

void CKTDGXMeshPlayer::CXMeshPlayerTokenizer::AddToken( CXMeshPlayerToken token )
{
	if( token.m_strValue.size() ) 
	{
		DetermineTokenType( token );
		m_TokenVector.push_back( token );
	}
}

void CKTDGXMeshPlayer::CXMeshPlayerTokenizer::DetermineTokenType( CXMeshPlayerToken &token )
{
	token.m_Type = UnknownToken;

	// these things are easy to see...
	if (isdigit(token.m_strValue[0]) || token.m_strValue[0] == '-') { token.m_Type = RealNumber; return; }
	if (token.m_strValue[0] == '=')   { token.m_Type = Equals; return; }
	if (token.m_strValue[0] == ',')   { token.m_Type = Comma; return; }
	if (token.m_strValue[0] == '(')   { token.m_Type = OpenParen; return; }
	if (token.m_strValue[0] == ')')   { token.m_Type = CloseParen; return; }
	if (token.m_strValue[0] == '{')   { token.m_Type = OpenBrace; return; }
	if (token.m_strValue[0] == '}')   { token.m_Type = CloseBrace; return; }
	if (token.m_strValue[0] == '\"')  { token.m_Type = Quote; return; }

	// if we got here, it's not a quote... so convert it to uppercase.
	MakeUpperCase(token.m_strValue);

	// keywords are easy to figure out, just look for the text...
	if (token.m_strValue.compare("XMESHPLAYER")			== 0) { token.m_Type = KeywordXMeshPlayer;		return; }
	if (token.m_strValue.compare("TEMPLET")				== 0) { token.m_Type = KeywordTemplet;			return; }
	if (token.m_strValue.compare("RANDOM")				== 0) { token.m_Type = KeywordRandom;			return; }
	if (token.m_strValue.compare("XYZ")					== 0) { token.m_Type = KeywordXYZ;				return; }
	if (token.m_strValue.compare("RGBA")				== 0) { token.m_Type = KeywordColor;			return; }
	if (token.m_strValue.compare("FADE")				== 0) { token.m_Type = KeywordFade;				return; }

	// Event area select
	if (token.m_strValue.compare("GLOBAL")				== 0) {token.m_Type = KeywordGlobal;	return;}
	if (token.m_strValue.compare("DIE")					== 0) {token.m_Type = KeywordDie;		return;}
	if (token.m_strValue.compare("ANIM")				== 0) {token.m_Type = KeywordAnim;		return;}

	if (token.m_strValue.compare("INITIAL")				== 0) { token.m_Type = KeywordInitial;			return; }
	if (token.m_strValue.compare("FINAL")				== 0) { token.m_Type = KeywordFinal;			return; }
	if (token.m_strValue.compare("FROM")				== 0) { token.m_Type = KeywordFrom;				return; }

	// these two keywords are embedded in other words, so we've got to be careful.
	if (token.m_strValue.compare("XY")					== 0) { token.m_Type = KeywordXY; return; }
	if (token.m_strValue.compare("SO")					== 0) { token.m_Type = KeywordSo; return; }
	if (token.m_strValue.compare("AT")					== 0) { token.m_Type = KeywordAt; return; }
	if (token.m_strValue.compare("TO")					== 0) { token.m_Type = KeywordTo; return; }

	// now it gets harder... 
	if (token.m_strValue.find("D3DBLEND_")				== 0) { token.m_Type = AlphaBlendMode; return; }
	if (token.m_strValue.compare("SOURCEBLENDMODE")		== 0 ||
		token.m_strValue.compare("DESTBLENDMODE")		== 0) 
	{ 
		token.m_Type = SeqAlphaBlendModeProp; return; 
	}

	if (token.m_strValue.compare("LIFETYPE")		== 0
		|| token.m_strValue.compare("DIETYPE")		== 0 ) 
	{ 
		token.m_Type = SeqLifeTypeProp; return; 
	}

	if (token.m_strValue.compare("USEDIEEVENT")			== 0
		|| token.m_strValue.compare("ZENABLE")			== 0
		|| token.m_strValue.compare("ZWRITEENABLE")		== 0
		|| token.m_strValue.compare("USELAND")			== 0
		|| token.m_strValue.compare("SET_LAND_HEIGHT_ON_START")			== 0
		|| token.m_strValue.compare("CRASHLAND")		== 0		
		|| token.m_strValue.compare("USESLASHTRACE")	== 0 
		|| token.m_strValue.compare("FORCELAYER")		== 0 
#ifdef PARTICLE_NOTAPPLY_UNITSCALE
		|| token.m_strValue.compare("APPLYUNITSCALE")	== 0
#endif //PARTICLE_NOTAPPLY_UNITSCALE
		)
	{ 
		token.m_Type = SeqBoolProp; return; 
	}

	if (token.m_strValue.compare("PLAYCOUNT")				== 0
		|| token.m_strValue.compare("LIFETIME")				== 0
		|| token.m_strValue.compare("DIEPLAYCOUNT")			== 0
		|| token.m_strValue.compare("DIELIFETIME")			== 0
		|| token.m_strValue.compare("OUTLINEWIDE")			== 0
		|| token.m_strValue.compare("LIGHTFLOWWIDE")		== 0 
		|| token.m_strValue.compare("DRAWCOUNT")			== 0
		|| token.m_strValue.compare("LANDPOS")				== 0
		|| token.m_strValue.compare("LAYER")				== 0
		|| token.m_strValue.compare("ELASTICCOEFF")			== 0
#ifdef ADD_UPDATE_LANDPOS
		|| token.m_strValue.compare("ELASTICCOEFF_X")		== 0 
#endif
		|| token.m_strValue.compare("SLASH_TRACE_TYPE")		== 0
		|| token.m_strValue.compare("SLASH_TRACE_CURVE_TYPE") == 0 
		|| token.m_strValue.compare("NOT_CHECK_LAND_TIME")	== 0 )
	{ 
		token.m_Type = SeqNumericProp; return; 
	}

	if (token.m_strValue.compare("MESHTYPE")		== 0 ) 
	{ 
		token.m_Type = SeqMeshTypeProp; return; 
	}

	if (token.m_strValue.compare("MESHNAME")			== 0
		|| token.m_strValue.compare("CHANGETEXXET")		== 0
		|| token.m_strValue.compare("MULTITEXXET")		== 0
		|| token.m_strValue.compare("ANIPACKAGE")		== 0 
		|| token.m_strValue.compare("SLASH_TRACE_TEXTURE")	== 0 ) 
	{ 
		token.m_Type = SeqStringProp; return; 
	}

	if (token.m_strValue.compare("RENDERTYPE")			== 0 ) 
	{ 
		token.m_Type = SeqRenderTypeProp; return; 
	}

	if (token.m_strValue.compare("CARTOONTEXTYPE")		== 0 ) 
	{ 
		token.m_Type = SeqCartoonTexTypeProp; return; 
	}

	if (token.m_strValue.compare("LIGHTPOS")		== 0
		|| token.m_strValue.compare("GRAVITY")		== 0
		|| token.m_strValue.compare("BLACKHOLEPOS")	== 0) 
	{ 
		token.m_Type = SeqVector3Prop; return; 
	}

	if (token.m_strValue.compare("CULLMODE")	== 0 ) 
	{ 
		token.m_Type = SeqCullModeProp; return; 
	}

	if (token.m_strValue.compare("BILLBOARDTYPE")	== 0 ) 
	{ 
		token.m_Type = SeqBillboardTypeProp; return; 
	}

	if (token.m_strValue.compare("COLOR")			== 0
		|| token.m_strValue.compare("OUTLINECOLOR")	== 0) 
	{ 
		token.m_Type = MeshColorProp; return; 
	}

	if (token.m_strValue.compare("ANINAME")				== 0
		|| token.m_strValue.compare("TEXTURECHANGE")	== 0
		|| token.m_strValue.compare("SOUND")			== 0) 
	{ 
		token.m_Type = MeshStringProp; return; 
	}

	if (token.m_strValue.compare("ANISPEED")			== 0
		|| token.m_strValue.compare("DIRSPEED")			== 0
		|| token.m_strValue.compare("BLACKHOLETIME")	== 0
		|| token.m_strValue.compare("RESETCRASH")		== 0
		|| token.m_strValue.compare("LIGHTFLOWIMPACT")	== 0) 
	{ 
		token.m_Type = MeshNumericProp; return; 
	}

	if (token.m_strValue.compare("SIZE")				== 0
		|| token.m_strValue.compare("VELOCITY")			== 0
		|| token.m_strValue.compare("ROTATE")			== 0
		|| token.m_strValue.compare("CRASH")			== 0
#ifdef EVENT_SCENE
		|| token.m_strValue.compare("POSITION")			== 0
#endif EVENT_SCENE
		|| token.m_strValue.compare("LIGHTFLOWPOINT")	== 0) 
	{ 
		token.m_Type = MeshVector3Prop; return; 
	}

	if (token.m_strValue.compare("TEX0UV")			== 0
		|| token.m_strValue.compare("TEX1UV")		== 0
		|| token.m_strValue.compare("TEX2UV")		== 0) 
	{ 
		token.m_Type = MeshVector2Prop; return; 
	}

	if (token.m_strValue.compare("SLASHTRACE")		== 0 )
	{
		token.m_Type = MeshBoolProp; return; 
	}

	if(token.m_strValue.compare("RENDER_TYPE")		== 0)
	{
		token.m_Type = MeshRenderTypeProp; return;
	}	

}

const int CKTDGXMeshPlayer::CXMeshPlayerTokenizer::NUMBLENDINGMODES = 15;
const CKTDGXMeshPlayer::BLENDINGMODE CKTDGXMeshPlayer::CXMeshPlayerTokenizer::m_BlendingModes[15] = 
{
	{ "D3DBLEND_ZERO",			D3DBLEND_ZERO			},
	{ "D3DBLEND_ONE",			D3DBLEND_ONE			},
	{ "D3DBLEND_SRCCOLOR",		D3DBLEND_SRCCOLOR		},
	{ "D3DBLEND_INVSRCCOLOR",	D3DBLEND_INVSRCCOLOR	},
	{ "D3DBLEND_SRCALPHA",		D3DBLEND_SRCALPHA		},
	{ "D3DBLEND_INVSRCALPHA",	D3DBLEND_INVSRCALPHA	},
	{ "D3DBLEND_DESTALPHA",		D3DBLEND_DESTALPHA		},
	{ "D3DBLEND_INVDESTALPHA",	D3DBLEND_INVDESTALPHA	},
	{ "D3DBLEND_DESTCOLOR",		D3DBLEND_DESTCOLOR		},
	{ "D3DBLEND_INVDESTCOLOR",	D3DBLEND_INVDESTCOLOR	},
	{ "D3DBLEND_SRCALPHASAT",	D3DBLEND_SRCALPHASAT	},
	{ "D3DBLEND_BOTHSRCALPHA",	D3DBLEND_BOTHSRCALPHA	},
	{ "D3DBLEND_BOTHINVSRCALPHA",	D3DBLEND_BOTHINVSRCALPHA	},
	{ "D3DBLEND_BLENDFACTOR",	D3DBLEND_BLENDFACTOR	},	
	{ "D3DBLEND_INVBLENDFACTOR",D3DBLEND_INVBLENDFACTOR },
};

bool CKTDGXMeshPlayer::CXMeshPlayerTokenizer::ProcessLifeType(LIFE_TYPE &lifeType, 
																  std::vector<CXMeshPlayerToken>::iterator &TokenIter,
																  std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if( TokenIter->m_strValue.compare( "LT_PLAY_COUNT" ) == 0 )
		lifeType = LT_PLAY_COUNT;
	else if( TokenIter->m_strValue.compare( "LT_TIME" ) == 0 )
		lifeType = LT_TIME;
	else if( TokenIter->m_strValue.compare( "LT_MANUAL" ) == 0 )
		lifeType = LT_MANUAL;
	else
	{
		throw("Invalid LifeType!");
		return(false);
	}

	NextToken(TokenIter, EndIter);
	return(true);
}

bool CKTDGXMeshPlayer::CXMeshPlayerTokenizer::ProcessMeshType(MESH_TYPE &meshType, 
															  std::vector<CXMeshPlayerToken>::iterator &TokenIter,
															  std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if( TokenIter->m_strValue.compare( "X_MESH" ) == 0 )
		meshType = X_MESH;
	else if( TokenIter->m_strValue.compare( "X_SKIN_MESH" ) == 0 )
		meshType = X_SKIN_MESH;
	else
	{
		throw("Invalid MeshType!");
		return(false);
	}

	NextToken(TokenIter, EndIter);
	return(true);
}

bool CKTDGXMeshPlayer::CXMeshPlayerTokenizer::ProcessRenderType(CKTDGXRenderer::RENDER_TYPE &renderType, 
															  std::vector<CXMeshPlayerToken>::iterator &TokenIter,
															  std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if( TokenIter->m_strValue.compare( "RT_REAL_COLOR" ) == 0 )
		renderType = CKTDGXRenderer::RT_REAL_COLOR;
	else if( TokenIter->m_strValue.compare( "RT_CARTOON_BLACK_EDGE" ) == 0 )
		renderType = CKTDGXRenderer::RT_CARTOON_BLACK_EDGE;
	else if( TokenIter->m_strValue.compare( "RT_CARTOON_COLOR_EDGE" ) == 0 )
		renderType = CKTDGXRenderer::RT_CARTOON_COLOR_EDGE;
	else if( TokenIter->m_strValue.compare( "RT_CARTOON" ) == 0 )
		renderType = CKTDGXRenderer::RT_CARTOON;
	else if( TokenIter->m_strValue.compare( "RT_LIGHT_FLOW_WORLD_X" ) == 0 )
		renderType = CKTDGXRenderer::RT_LIGHT_FLOW_WORLD_X;
	else if( TokenIter->m_strValue.compare( "RT_LIGHT_FLOW_WORLD_Y" ) == 0 )
		renderType = CKTDGXRenderer::RT_LIGHT_FLOW_WORLD_Y;
	else if( TokenIter->m_strValue.compare( "RT_LIGHT_FLOW_WORLD_Z" ) == 0 )
		renderType = CKTDGXRenderer::RT_LIGHT_FLOW_WORLD_Z;
	else if( TokenIter->m_strValue.compare( "RT_LIGHT_FLOW_WORLD_POINT" ) == 0 )
		renderType = CKTDGXRenderer::RT_LIGHT_FLOW_WORLD_POINT;
	else if( TokenIter->m_strValue.compare( "RT_LIGHT_FLOW_LOCAL_X" ) == 0 )
		renderType = CKTDGXRenderer::RT_LIGHT_FLOW_LOCAL_X;
	else if( TokenIter->m_strValue.compare( "RT_LIGHT_FLOW_LOCAL_Y" ) == 0 )
		renderType = CKTDGXRenderer::RT_LIGHT_FLOW_LOCAL_Y;
	else if( TokenIter->m_strValue.compare( "RT_LIGHT_FLOW_LOCAL_Z" ) == 0 )
		renderType = CKTDGXRenderer::RT_LIGHT_FLOW_LOCAL_Z;
	else if( TokenIter->m_strValue.compare( "RT_LIGHT_FLOW_LOCAL_POINT" ) == 0 )
		renderType = CKTDGXRenderer::RT_LIGHT_FLOW_LOCAL_POINT;
	else if( TokenIter->m_strValue.compare( "RT_CARTOON_FADE" ) == 0 )
		renderType = CKTDGXRenderer::RT_CARTOON_FADE;
	else if( TokenIter->m_strValue.compare( "RT_CARTOON_FADE_NONTEX" ) == 0 )
		renderType = CKTDGXRenderer::RT_CARTOON_FADE_NONTEX;
#ifdef ARA_CHARACTER_BASE
	else if( TokenIter->m_strValue.compare( "RT_BLACK_EDGE" ) == 0 )
		renderType = CKTDGXRenderer::RT_BLACK_EDGE;
	else if( TokenIter->m_strValue.compare( "RT_COLOR_EDGE" ) == 0 )
		renderType = CKTDGXRenderer::RT_COLOR_EDGE;
#endif
	else
	{
		throw("Invalid RenderType!");
		return(false);
	}

	NextToken(TokenIter, EndIter);
	return(true);
}

bool CKTDGXMeshPlayer::CXMeshPlayerTokenizer::ProcessCartoonTexType(CKTDGXRenderer::CARTOON_TEX_TYPE &cartoonTexType, 
															  std::vector<CXMeshPlayerToken>::iterator &TokenIter,
															  std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if( TokenIter->m_strValue.compare( "CTT_NORMAL" ) == 0 )
		cartoonTexType = CKTDGXRenderer::CTT_NORMAL;
	else if( TokenIter->m_strValue.compare( "CTT_RED" ) == 0 )
		cartoonTexType = CKTDGXRenderer::CTT_RED;
	else
	{
		throw("Invalid cartoonTexType!");
		return(false);
	}

	NextToken(TokenIter, EndIter);
	return(true);
}

bool CKTDGXMeshPlayer::CXMeshPlayerTokenizer::ProcessCullMode(D3DCULL &cullMode, 
															std::vector<CXMeshPlayerToken>::iterator &TokenIter,
															std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if( TokenIter->m_strValue.compare( "D3DCULL_CCW" ) == 0 )
		cullMode = D3DCULL_CCW;
	else if( TokenIter->m_strValue.compare( "D3DCULL_CW" ) == 0 )
		cullMode = D3DCULL_CW;
	else if( TokenIter->m_strValue.compare( "D3DCULL_NONE" ) == 0 )
		cullMode = D3DCULL_NONE;
	else
	{
		throw("Invalid cullMode!");
		return(false);
	}

	NextToken(TokenIter, EndIter);
	return(true);
}

bool CKTDGXMeshPlayer::CXMeshPlayerTokenizer::ProcessNumber( CMinMax<float> &number, std::vector<CXMeshPlayerToken>::iterator &TokenIter,
																   std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	// the first token is either the random keyword, or it's an actual number.
	switch(TokenIter->m_Type) 
	{
	case KeywordRandom:
		{
			// parse random number into minmax
			NextToken(TokenIter, EndIter);
			if (TokenIter->m_Type != OpenParen) throw("Expecting opening paren after Random keyword.");
			NextToken(TokenIter, EndIter);
			if (TokenIter->m_Type != RealNumber) throw("Expecting first number within Random(...).");
			number.m_Min = (float)atof(TokenIter->m_strValue.c_str());
			NextToken(TokenIter, EndIter);

			if (TokenIter->m_Type != Comma) throw("Expecting comma within Random(...).");
			NextToken(TokenIter, EndIter);

			if (TokenIter->m_Type != RealNumber) throw("Expecting second number within Random(...).");
			number.m_Max = (float)atof(TokenIter->m_strValue.c_str());
			NextToken(TokenIter, EndIter);
			if (TokenIter->m_Type != CloseParen) throw("Missing close paren on Random(...).");
			NextToken(TokenIter, EndIter);
		}
		break;

	case RealNumber:
		// min and max both equal realnumber
		if (TokenIter->m_Type != RealNumber) throw("Expecting number.");

		number.m_Max = number.m_Min = (float)atof(TokenIter->m_strValue.c_str());
		NextToken(TokenIter, EndIter);
		break;

	default: throw("Expecting either Random(...) or a number value.");
	}
	return(true);
}

bool CKTDGXMeshPlayer::CXMeshPlayerTokenizer::ProcessVector3(CMinMax<D3DXVECTOR3> &v, 
																   std::vector<CXMeshPlayerToken>::iterator &TokenIter,
																   std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	// this token needs to be a XYZ keyword.
	if (TokenIter->m_Type != KeywordXYZ) throw("Expecting XYZ(...)!");
	NextToken(TokenIter, EndIter);

	if (TokenIter->m_Type != OpenParen) throw("Expecting ( after XYZ!");

	CMinMax<float> x;
	CMinMax<float> y;
	CMinMax<float> z;

	NextToken(TokenIter, EndIter); ProcessNumber(x, TokenIter, EndIter);
	if (TokenIter->m_Type != Comma) throw("Vector components must be seperated by a comma.");
	NextToken(TokenIter, EndIter); ProcessNumber(y, TokenIter, EndIter);
	if (TokenIter->m_Type != Comma) throw("Vector components must be seperated by a comma.");
	NextToken(TokenIter, EndIter); ProcessNumber(z, TokenIter, EndIter);
	if (TokenIter->m_Type != CloseParen) throw("Expecting ) to close vector.");
	NextToken(TokenIter, EndIter);

	v.m_Min = D3DXVECTOR3(x.m_Min, y.m_Min, z.m_Min);
	v.m_Max = D3DXVECTOR3(x.m_Max, y.m_Max, z.m_Max);

	return(true);
}

bool CKTDGXMeshPlayer::CXMeshPlayerTokenizer::ProcessVector2(CMinMax<D3DXVECTOR2> &v, 
																	std::vector<CXMeshPlayerToken>::iterator &TokenIter,
																	std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	// this token needs to be a XYZ keyword.
	if (TokenIter->m_Type != KeywordXY) throw("Expecting XY(...)!");
	NextToken(TokenIter, EndIter);

	if (TokenIter->m_Type != OpenParen) throw("Expecting ( after XY!");

	CMinMax<float> x;
	CMinMax<float> y;

	NextToken(TokenIter, EndIter); ProcessNumber(x, TokenIter, EndIter);
	if (TokenIter->m_Type != Comma) throw("Vector components must be seperated by a comma.");
	NextToken(TokenIter, EndIter); ProcessNumber(y, TokenIter, EndIter);
	if (TokenIter->m_Type != CloseParen) throw("Expecting ) to close vector.");
	NextToken(TokenIter, EndIter);

	v.m_Min = D3DXVECTOR2(x.m_Min, y.m_Min);
	v.m_Max = D3DXVECTOR2(x.m_Max, y.m_Max);

	return(true);
}

bool CKTDGXMeshPlayer::CXMeshPlayerTokenizer::ProcessColor(CMinMax<D3DXCOLOR> &c, 
																  std::vector<CXMeshPlayerToken>::iterator &TokenIter,
																  std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	// this token needs to be a RGBA keyword.
	if (TokenIter->m_Type != KeywordColor)
		throw("Expecting RGBA(...)!");
	NextToken(TokenIter, EndIter);

	if (TokenIter->m_Type != OpenParen) throw("Expecting ( after RGBA!");

	CMinMax<float> r;
	CMinMax<float> g;
	CMinMax<float> b;
	CMinMax<float> a;

	NextToken(TokenIter, EndIter); ProcessNumber(r, TokenIter, EndIter);
	if (TokenIter->m_Type != Comma) throw("Color components must be seperated by a comma.");
	NextToken(TokenIter, EndIter); ProcessNumber(g, TokenIter, EndIter);
	if (TokenIter->m_Type != Comma) throw("Color components must be seperated by a comma.");
	NextToken(TokenIter, EndIter); ProcessNumber(b, TokenIter, EndIter);
	if (TokenIter->m_Type != Comma) throw("Color components must be seperated by a comma.");
	NextToken(TokenIter, EndIter); ProcessNumber(a, TokenIter, EndIter);
	if (TokenIter->m_Type != CloseParen) throw("Expecting ) to close vector.");
	NextToken(TokenIter, EndIter);

	c.m_Min = D3DXCOLOR(r.m_Min, g.m_Min, b.m_Min, a.m_Min);
	c.m_Max = D3DXCOLOR(r.m_Max, g.m_Max, b.m_Max, a.m_Max);

	return(true);
}

bool CKTDGXMeshPlayer::CXMeshPlayerTokenizer::ProcessAlphaBlendMode(int &alphablendmode, 
																		   std::vector<CXMeshPlayerToken>::iterator &TokenIter,
																		   std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if (TokenIter->m_Type != AlphaBlendMode)
		throw("Expecting alpha blend mode (D3DBLEND_ZERO, D3DBLEND_ONE, etc.).");
	for (int q=0; q < NUMBLENDINGMODES; q++) 
	{
		// careful here... must match on both string and size 
		// (i.e., to differentiate between D3DBLEND_SRCALPHA and D3DBLEND_SRCALPHASAT).
		if (TokenIter->m_strValue.find(m_BlendingModes[q].name) != std::string::npos && 
			TokenIter->m_strValue.size() == strlen(m_BlendingModes[q].name)) 
		{
			alphablendmode = m_BlendingModes[q].mode;
			NextToken(TokenIter, EndIter);
			return(true);
		}
	}

	throw("Invalid alpha blending mode!");
	return(false);
}

bool CKTDGXMeshPlayer::CXMeshPlayerTokenizer::ProcessBillboardType(CKTDGMatrix::BILLBOARD_TYPE &billboardType, 
																		  std::vector<CXMeshPlayerToken>::iterator &TokenIter,
																		  std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if( TokenIter->m_strValue.compare( "BT_ALL" ) == 0 )
	{
		billboardType = CKTDGMatrix::BT_ALL;
	}
	else if( TokenIter->m_strValue.compare( "BT_NONE" ) == 0 )
	{
		billboardType = CKTDGMatrix::BT_NONE;
	}
	else if( TokenIter->m_strValue.compare( "BT_X" ) == 0 )
	{
		billboardType = CKTDGMatrix::BT_X;
	}
	else if( TokenIter->m_strValue.compare( "BT_Y" ) == 0 )
	{
		billboardType = CKTDGMatrix::BT_Y;
	}
	else if( TokenIter->m_strValue.compare( "BT_Z" ) == 0 )
	{
		billboardType = CKTDGMatrix::BT_Z;
	}
	else
	{
		throw("Invalid billboard type!");
		return(false);
	}

	NextToken(TokenIter, EndIter);
	return(true);
}

bool CKTDGXMeshPlayer::CXMeshPlayerTokenizer::ProcessBool(bool &bValue, 
																 std::vector<CXMeshPlayerToken>::iterator &TokenIter,
																 std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	if( TokenIter->m_strValue.compare( "TRUE" ) == 0 )
	{
		bValue = true;
	}
	else if( TokenIter->m_strValue.compare( "FALSE" ) == 0 )
	{
		bValue = false;
	}
	else
	{
		throw("Invalid bool!");
		return(false);
	}

	NextToken(TokenIter, EndIter);
	return(true);
}

bool CKTDGXMeshPlayer::CXMeshPlayerTokenizer::ProcessString(wstring &str, 
																   std::vector<CXMeshPlayerToken>::iterator &TokenIter,
																   std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	ConvertCharToWCHAR( str, RemoveQuotes(TokenIter->m_strValue).c_str() );

	NextToken(TokenIter, EndIter);
	return(true);
}

bool CKTDGXMeshPlayer::CXMeshPlayerTokenizer::ProcessTime(CMinMax<float> &TimeRange, 
														  bool &IsFade, bool &IsFinal, const float InitialTime,
														  std::vector<CXMeshPlayerToken>::iterator &TokenIter, 
														  std::vector<CXMeshPlayerToken>::iterator &EndIter)
{
	// determine if this time specification is a fade directive
	if (TokenIter->m_Type == KeywordFade) 
	{
		// it is... the next token must be "so"
		NextToken(TokenIter, EndIter); 

		if (TokenIter->m_Type != KeywordSo) 
			throw("Expecting \"so\" after \"fade\".");

		// flip the fade flag on
		IsFade = true;

		// move to next token (so that we land on "at" for the code below)
		NextToken(TokenIter, EndIter);
	}
	else 
		IsFade = false; // just to be safe

	switch(TokenIter->m_Type) 
	{
	case KeywordFrom:
		{
			// easy, just grab the time
			NextToken(TokenIter, EndIter);
			ProcessNumber(TimeRange, TokenIter, EndIter);

			float startTime = TimeRange.m_Min;

			if (TokenIter->m_Type != KeywordTo)
				throw("Expecting \"To\" after \"From\".");

			// easy, just grab the time
			NextToken(TokenIter, EndIter);
			ProcessNumber(TimeRange, TokenIter, EndIter);

			TimeRange.m_Min = startTime;
		}
		break;

	case KeywordAt:
		{
			// easy, just grab the time
			NextToken(TokenIter, EndIter);
			ProcessNumber(TimeRange, TokenIter, EndIter);
		}
		break;

	case KeywordInitial:
		{
			// use initial time that was passed in.
			if (IsFade) throw("Cannot use \"fade so\" modifier on \"initial\" times.");
			TimeRange.m_Min = InitialTime;
			TimeRange.m_Max = InitialTime;
			CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);
		}
		break;

	case KeywordFinal:
		{
			// use final time that was passed in.
			TimeRange.m_Min = InitialTime;
			TimeRange.m_Max = InitialTime;
			IsFinal = true;
			CXMeshPlayerTokenizer::NextToken(TokenIter, EndIter);
		}
		break;

	default: throw("Expected time specifier: \"[fade so] at\", \"initial\", or \"[fade so] final\"");
	}

	return(true);
}


#ifdef RAVEN_WEAPON_TAKER
bool CKTDGXMeshPlayer::IsDyingByCrash( CXMeshInstanceHandle hMeshInstanceHandle )
{
	CXMeshInstance* pInstance = GetMeshInstance( hMeshInstanceHandle );
	if( pInstance && pInstance->GetState() == IS_DYING_BY_CRASH )
		return true;
	else
		return false;
}
void CKTDGXMeshPlayer::PlayEnd( CXMeshInstanceHandle hMeshInstanceHandle )
{
	CXMeshInstance* pInstance = GetMeshInstance( hMeshInstanceHandle );
	if( pInstance )
	{
		pInstance->PlayEnd();
	}
}
#endif RAVEN_WEAPON_TAKER


//#ifdef	X2OPTIMIZE_REFACTORING_RESOURCE_BACKGROUND_LOAD
bool	CKTDGXMeshPlayer::AppendToDeviceList( CKTDXDeviceDataList& listInOut_, const WCHAR* pTempletName, CKTDXDeviceManager::EPriority ePriority )
{
	if ( pTempletName == NULL || pTempletName[0] == NULL )
		return false;
	return AppendToDeviceList( listInOut_, std::wstring(pTempletName), ePriority );
}

bool	CKTDGXMeshPlayer::AppendToDeviceList( CKTDXDeviceDataList& listInOut_, const std::wstring& wstrTempletName, CKTDXDeviceManager::EPriority ePriority )
{
	if ( wstrTempletName.empty() == false )
		return false;

	std::map<std::wstring,XMeshTemplet*>::const_iterator iter = m_TempletMap.find( wstrTempletName );
	if ( iter == m_TempletMap.end() || iter->second == NULL )
		return false;

	const XMeshTemplet& kTempletData = *iter->second;
	if( kTempletData.meshType == X_MESH )
	{
		g_pKTDXApp->GetDeviceManager()->AppendXMesh( listInOut_, kTempletData.meshName, ePriority );
		g_pKTDXApp->GetDeviceManager()->AppendXET( listInOut_, kTempletData.changeTexXETName, ePriority );
		g_pKTDXApp->GetDeviceManager()->AppendXET( listInOut_, kTempletData.multiTexXETName, ePriority );
		g_pKTDXApp->GetDeviceManager()->AppendXET( listInOut_, kTempletData.aniXETName, ePriority );
	}
	else if( kTempletData.meshType == X_SKIN_MESH )
	{
		g_pKTDXApp->GetDeviceManager()->AppendXSkinMesh( listInOut_, kTempletData.meshName, ePriority );
		g_pKTDXApp->GetDeviceManager()->AppendXET( listInOut_, kTempletData.changeTexXETName, ePriority );
		g_pKTDXApp->GetDeviceManager()->AppendXET( listInOut_, kTempletData.multiTexXETName, ePriority );
		g_pKTDXApp->GetDeviceManager()->AppendXET( listInOut_, kTempletData.aniXETName, ePriority );
	}

	return true;
}
//#endif	X2OPTIMIZE_REFACTORING_RESOURCE_BACKGROUND_LOAD

#ifdef EFFECT_TOOL
CKTDGXMeshPlayer::XMeshTemplet* CKTDGXMeshPlayer::CreateXMeshTemplet( const WCHAR* strMeshName_ )
{
	// 똑같은 이름의 mesh player templet 이 있다면 지우고 덮어쓴다
	std::map<wstring,XMeshTemplet*>::iterator it = m_TempletMap.find( strMeshName_ );
	if( it != m_TempletMap.end() )
	{
		MessageBox( NULL, strMeshName_ , L"Duplication" , NULL );
		return NULL;
	}

	CKTDGXMeshPlayer::XMeshTemplet* templet = new CKTDGXMeshPlayer::XMeshTemplet;
	templet->templetName = strMeshName_;
	m_TempletMap[strMeshName_] = templet;
	m_TempletVec.push_back( templet );
	return templet;
}
void CKTDGXMeshPlayer::EraseXMeshTemplet( UINT uiMeshIndex_ )
{
	vector<XMeshTemplet*>::iterator it = m_TempletVec.begin() + uiMeshIndex_ ;
	m_TempletMap.erase( (*it)->templetName );
	m_TempletVec.erase( it );
}
#endif // EFFECT_TOOL

bool CKTDGXMeshPlayer::IsValidTemplet( const XMeshTemplet* pXMesshTemplet_ )
{
	if( NULL == pXMesshTemplet_ )
		return false;

	// 라이프타입이 시간인데, 최대 시간이 0초로 지정되어있으면 생성하지 않기.
	if( LT_TIME == pXMesshTemplet_->lifeType &&
		0.f >= pXMesshTemplet_->lifeTime.m_Max )
		return false;

	return true;
}