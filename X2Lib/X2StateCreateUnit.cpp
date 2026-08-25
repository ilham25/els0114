#include "StdAfx.h"
#ifndef ELSWORD_NEW_BEGINNING
#include ".\x2statecreateunit.h"




static const int ELSWORD_PROMOTION_ITEM_ID[]	= { 128000, 128001, 128002, 128003, 128004, };
static const int AISHA_PROMOTION_ITEM_ID[]		= { 128010, 128011, 128012, 128013, 128014, };
static const int RENA_PROMOTION_ITEM_ID[]		= { 128005, 128006, 128007, 128008, 128009, };
static const int RAVEN_PROMOTION_ITEM_ID[]		= { 128072, 128073, 128074, 128075, 128076, };
static const int EVE_PROMOTION_ITEM_ID[]		= { 130134, 130135, 130136, 130137, 130138, };

//{{ kimhc // 2010.12.8 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
static const int CHUNG_PROMOTION_ITEM_ID[]		= { 133125, 133126, 133127, 133128, 133129, };	// 캐릭터 생성 창에서 사용될 청의 프로모션 복장 ID
#endif	NEW_CHARACTER_CHUNG
//}} kimhc // 2010.12.8 //  2010-12-23 New Character CHUNG

#ifdef ARA_CHARACTER_BASE
static const int ARA_PROMOTION_ITEM_ID[]		= { 41460, 41470, 41480, 41490, 41500, };
#endif

#ifdef NEW_CHARACTER_EL // 프로모션 아이템
static const int EL_PROMOTION_ITEM_ID[]		= { 41720, 41721, 41722, 41723, 41724, };
#endif // NEW_CHARACTER_EL

CX2StateCreateUnit::CX2StateCreateUnit(void) :
m_pDLGCreateUnitBack( NULL ),
m_pDLGCreateUnitFront( NULL ),
m_SelectUnitClass( CX2Unit::UC_NONE ),
m_NickName( L"" ),
m_UnitUID( -1 ),
m_pDummyElsword( NULL ),
m_pDummyArme( NULL ),
m_pDummyLire( NULL ),
m_pDummyRaven( NULL ),
m_pDummyEve( NULL ),
//{{ kimhc // 2010.11.24 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
m_pDummyChung( NULL ),
#endif	NEW_CHARACTER_CHUNG
//}} kimhc // 2010.11.24 //  2010-12-23 New Character CHUNG
#ifdef ARA_CHARACTER_BASE
m_pDummyAra( NULL ),
#endif
#ifdef NEW_CHARACTER_EL
m_pDummyEL ( NULL ) ,
#endif // NEW_CHARACTER_EL
m_pUnitViewerUI( NULL ),
m_bSelectUnit( false ),		
m_fMouseSensitivity( 0.01f ),
m_fRemainRotY( 0.f ),
m_bRotateReturn( true ),
m_fRotSpeed( 0.f ),
m_pDLGMSGOkBoxEnterTutorial( NULL )
{
	// variable assign 
#ifdef NEW_CHARACTER_EL // 엘리시스 런칭 시 제거 예정
	m_pDLGCreateUnitBack = new CKTDGUIDialog( this, L"DLG_unit_create_back_Elesis.lua" );
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGCreateUnitBack );

	m_pDLGCreateUnitFront = new CKTDGUIDialog( this, L"DLG_unit_create_front_Elesis.lua" );
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGCreateUnitFront );
#else
	m_pDLGCreateUnitBack = new CKTDGUIDialog( this, L"DLG_Unit_Create_Back.lua" );
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGCreateUnitBack );

	m_pDLGCreateUnitFront = new CKTDGUIDialog( this, L"DLG_Unit_Create_Front.lua" );
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGCreateUnitFront );
#endif //NEW_CHARACTER_EL


	g_pKTDXApp->GetDGManager()->GetCamera()->Point( 0,0,-700, 0,0,0 );

	InitDummyUnits();

	CX2UnitViewerUI::SetUnitClicked( false );

#ifdef CREATE_CHARACTER_ARA_FIRST
	SelectAra();
#endif CREATE_CHARACTER_ARA_FIRST

	g_pKTDXApp->SkipFrame();
}

CX2StateCreateUnit::~CX2StateCreateUnit(void)
{
	SAFE_DELETE_DIALOG( m_pDLGCreateUnitBack );
	SAFE_DELETE_DIALOG( m_pDLGCreateUnitFront );
	SAFE_DELETE_DIALOG( m_pDLGMSGOkBoxEnterTutorial );

	SAFE_DELETE( m_pDummyElsword );
	SAFE_DELETE( m_pDummyArme );
	SAFE_DELETE( m_pDummyLire );
	SAFE_DELETE( m_pDummyRaven );
	SAFE_DELETE( m_pDummyEve );
	SAFE_DELETE( m_pDummyChung );
#ifdef ARA_CHARACTER_BASE
	SAFE_DELETE( m_pDummyAra );
#endif
#ifdef NEW_CHARACTER_EL
	SAFE_DELETE( m_pDummyEL );
#endif // NEW_CHARACTER_EL
	CX2UnitViewerUI::DeleteKTDGObject( m_pUnitViewerUI );
}

HRESULT CX2StateCreateUnit::OnFrameMove( double fTime, float fElapsedTime )
{
	CX2State::OnFrameMove( fTime, fElapsedTime );
//	g_pKTDXApp->GetDGManager()->GetCamera()->UpdateCamera( fElapsedTime );

#ifdef COME_BACK_REWARD
	if ( NULL != g_pData->GetPremiumBuffManager() )
	{
		g_pData->GetPremiumBuffManager()->OnFrameMove( fTime, fElapsedTime );
	}
#endif

	if( m_pUnitViewerUI != NULL )
	{
		if ( CX2UnitViewerUI::GetUnitClicked() == true )
		{
			D3DXVECTOR3 RotateY = D3DXVECTOR3(0, -((float)g_pKTDXApp->GetDIManager()->GetMouse()->GetXDelta() * m_fMouseSensitivity), 0);
			m_pUnitViewerUI->GetMatrix().RotateRel(RotateY);
			m_bRotateReturn = false;

			m_fRotSpeed = 13.0f;
		}
		else
		{
			D3DXVECTOR3 rot = m_pUnitViewerUI->GetMatrix().GetRotate();
			m_pUnitViewerUI->GetMatrix().Rotate( D3DXVECTOR3(0,0,0) );
			int addSpeed = 20;

			if ( m_bRotateReturn == false )
			{
				m_fRotSpeed -= (fElapsedTime * addSpeed);

				if ( m_fRotSpeed <= 0 )
				{
					m_fRotSpeed = fElapsedTime;
				}

				if (m_fRemainRotY >= 0.0f &&  m_fRemainRotY < 3.14f )
				{

					if ( m_fRemainRotY <= 0.0f )
					{
						m_pUnitViewerUI->GetMatrix().Rotate( D3DXVECTOR3(0,0,0) );
						m_fRemainRotY = 0.0f;
						m_bRotateReturn = true;
					}
					else
					{
						m_fRemainRotY -= (fElapsedTime*m_fRotSpeed);
						m_pUnitViewerUI->GetMatrix().Rotate(D3DXVECTOR3(0,m_fRemainRotY,0));

						if ( m_fRemainRotY <= 0.0f )
						{
							m_pUnitViewerUI->GetMatrix().Rotate( D3DXVECTOR3(0,0,0) );
							m_fRemainRotY = 0.0f;
							m_bRotateReturn = true;
						}
					}
				}
				else if ( m_fRemainRotY > 3.14f && m_fRemainRotY <= 6.18f  )
				{
					if ( m_fRemainRotY >= 6.18f )
					{
						m_bRotateReturn = true;
						m_fRemainRotY = 0.0f;
						m_pUnitViewerUI->GetMatrix().Rotate( D3DXVECTOR3(0,0,0) );
					}
					else
					{
						m_fRemainRotY += (fElapsedTime*m_fRotSpeed);
						m_pUnitViewerUI->GetMatrix().Rotate(D3DXVECTOR3(0,m_fRemainRotY,0));

						if ( m_fRemainRotY >= 6.18f )
						{
							m_bRotateReturn = true;
							m_fRemainRotY = 0.0f;
							m_pUnitViewerUI->GetMatrix().Rotate( D3DXVECTOR3(0,0,0) );
						}
					}
				}
				else if ( m_fRemainRotY >= -3.14f && m_fRemainRotY <= 0.0f )
				{
					if ( m_fRemainRotY >= 0.0f )
					{
						m_bRotateReturn = true;
						m_fRemainRotY = 0.0f;
						m_pUnitViewerUI->GetMatrix().Rotate( D3DXVECTOR3(0,0,0) );
					}
					else
					{
						m_fRemainRotY += (fElapsedTime*m_fRotSpeed);
						m_pUnitViewerUI->GetMatrix().Rotate(D3DXVECTOR3(0,m_fRemainRotY,0));

						if ( m_fRemainRotY >= 0.0f )
						{
							m_bRotateReturn = true;
							m_fRemainRotY = 0.0f;
							m_pUnitViewerUI->GetMatrix().Rotate( D3DXVECTOR3(0,0,0) );
						}
					}
				}
				else if (  m_fRemainRotY >= -6.28f && m_fRemainRotY <= -3.14f )
				{
					if ( m_fRemainRotY <= -6.28f )
					{
						m_bRotateReturn = true;
						m_fRemainRotY = 0.0f;
						m_pUnitViewerUI->GetMatrix().Rotate( D3DXVECTOR3(0,0,0) );
					}
					else
					{
						m_fRemainRotY -= (fElapsedTime*m_fRotSpeed);
						m_pUnitViewerUI->GetMatrix().Rotate(D3DXVECTOR3(0,m_fRemainRotY,0));

						if ( m_fRemainRotY <= -6.28f )
						{
							m_bRotateReturn = true;
							m_fRemainRotY = 0.0f;
							m_pUnitViewerUI->GetMatrix().Rotate( D3DXVECTOR3(0,0,0) );
						}
					}
				}
			}
		}

		if ( m_pUnitViewerUI->GetXSkinAnim()->GetState() == CKTDGXSkinAnim::XAP_ONE_WAIT )
		{
			//m_pUnitViewerUI->PlayAnim( L"LobbyWait", CKTDGXSkinAnim::XAP_LOOP, true );
			//m_SelectUnitClass
			m_pUnitViewerUI->PlayByMotionType( CX2UnitViewerUI::UVUMT_WAIT );
		}

		m_pUnitViewerUI->OnFrameMove( fTime, fElapsedTime );
	}

	bool bCheckHaveFocus = false;
	CKTDGUIControl* pControl = m_pDLGCreateUnitFront->GetControl(L"IMEEditBoxUnitCreateNickName");
	if ( pControl != NULL )
	{
		if ( pControl->GetHaveFocusIn() == true )
		{
			bCheckHaveFocus = true;
		}
	}
	
	pControl = m_pDLGCreateUnitFront->GetControl( L"Unit_Create_Attntion_Box" );
	if ( pControl != NULL )
	{
		if ( bCheckHaveFocus == true )
		{
			if ( pControl->GetIsMoving() == false )
				pControl->Move( pControl->GetOffsetPos(), D3DXCOLOR( 1,1,1,1 ), 0.5f, true );
		}
		else
		{
			if ( pControl->GetIsMoving() == false )
				pControl->Move( pControl->GetOffsetPos(), D3DXCOLOR( 1,1,1,0 ), 0.5f, true );
		}
	}
	

	return S_OK;
}

bool CX2StateCreateUnit::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
		case WM_LBUTTONUP:
			{
				if ( CX2UnitViewerUI::GetUnitClicked() == true && m_pUnitViewerUI != NULL )
				{
					D3DXVECTOR3 rot = m_pUnitViewerUI->GetMatrix().GetRotate();
					int degree = (int)D3DXToDegree( rot.y );
					m_fRemainRotY = D3DXToRadian( degree % 360 );
				}
				CX2UnitViewerUI::SetUnitClicked( false );
			}
			break;
	}

	return CX2State::MsgProc( hWnd, uMsg, wParam, lParam );
}

bool CX2StateCreateUnit::UICustomEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( CX2State::UICustomEventProc( hWnd, uMsg, wParam, lParam ) == true )
		return true;

#ifdef COME_BACK_REWARD
	if ( NULL != g_pData->GetPremiumBuffManager() )
	{
		if ( true == g_pData->GetPremiumBuffManager()->UICustomEventProc(hWnd, uMsg, wParam, lParam ) )
			return true;
	}
#endif

#ifdef SERV_SECOND_SECURITY
	if( NULL != g_pMain->GetSecurityPad() )
	{
		if( true == g_pMain->GetSecurityPad()->UICustomEventProc( hWnd, uMsg, wParam, lParam ) )
			return true;
	}
#endif SERV_SECOND_SECURITY


	switch(wParam)
	{
		case SCUUCM_ELSWORD_BUTTON_UP:
			SelectElSword();
			break;

		case SCUUCM_LIRE_BUTTON_UP:
			SelectLire();
			break;

		case SCUUCM_ARME_BUTTON_UP:
			SelectArme();
			break;

		case SCUUCM_RAVEN_BUTTON_UP:
			SelectRaven();
			break;


		case SCUUCM_EVE_BUTTON_UP:
			SelectEve();
			break;

		//{{ kimhc // 2010.11.29 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
		case SCUUCM_CHUNG_BUTTON_UP:
			SelectChung();
			break;
#endif	NEW_CHARACTER_CHUNG
		//}} kimhc // 2010.11.29 //  2010-12-23 New Character CHUNG

#ifdef ARA_CHARACTER_BASE
		case SCUUCM_ARA_BUTTON_UP:
			SelectAra();
			break;
#endif
		case SCUUCM_CREATE_UNIT:
			{
				if( m_bSelectUnit == true )
					Handler_EGS_CREATE_UNIT_REQ();
				else
					g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_559 ), this );
			}
			break;
			
		case SCUUCM_BACK_TO_UNIT_SELECT:
			{
				//{{ kimhc // 2009-12-23 // 서버군 추가로 유닛이 없는 경우 채널 선택창으로 이동 한다는 팝업
#ifdef	ADD_SERVER_GROUP
				if ( g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUnitNum() == 0 )
					g_pMain->CreateStateChangeDLG( GET_STRING( STR_ID_4837 ) );
				else
#endif	ADD_SERVER_GROUP
				//}} kimhc // 2009-12-23 // 서버군 추가로 유닛이 없는 경우 채널 선택창으로 이동 한다는 팝업
				g_pMain->CreateStateChangeDLG( GET_STRING( STR_ID_552 ) );
				g_pKTDXApp->SendGameMessage( XGM_STATE_CHANGE, /*CX2Main::XS_UNIT_SELECT*/CX2Main::XS_SERVER_SELECT, NULL, false );
			}
			break;

		case SCUUCM_UNIT_ANIMATION_DOWN:
			CX2UnitViewerUI::SetUnitClicked( true );
			break;

		case SCUUCM_UNIT_ANIMATION_UP:
			CX2UnitViewerUI::SetUnitClicked( false );
			break;

		case SCUUCM_UNIT_ANIMATION_OVER:
			break;

		case SCUUCM_ENTER_TUTORIAL_OK:
			{
				g_pMain->SetIsPlayingTutorial( true );
				if( m_pDLGMSGOkBoxEnterTutorial != NULL )
				{
					g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGMSGOkBoxEnterTutorial, NULL, false );
					m_pDLGMSGOkBoxEnterTutorial = NULL;
				}

				g_pMain->CreateStateChangeDLG( GET_STRING( STR_ID_552 ) );
				g_pKTDXApp->SendGameMessage( XGM_STATE_CHANGE, /*CX2Main::XS_UNIT_SELECT*/CX2Main::XS_SERVER_SELECT, NULL, false ); 
			}
			break;

		case SCUUCM_ENTER_TUTORIAL_CANCEL:
			{
				g_pMain->SetIsPlayingTutorial( false );
				if( m_pDLGMSGOkBoxEnterTutorial != NULL )
				{
					g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGMSGOkBoxEnterTutorial, NULL, false );
					m_pDLGMSGOkBoxEnterTutorial = NULL;
				}
				

				CX2StateAutoChanger::TARGET_DETAIL targetDetail;
				g_pMain->GetStateAutoChanger().StartStateChange( (int)g_pMain->GetNowStateID(), CX2Main::XS_VILLAGE_MAP, targetDetail, m_UnitUID );

			} break;

		case SCUUCM_SHUT_DOWN_OK:
			{
				CX2State::Handler_EGS_CLIENT_QUIT_REQ();
			} break;
#ifdef NEW_CHARACTER_EL
		case SCUUCM_ELESIS_BUTTON_UP:
			{
				SelectElesis();
			} break;
#endif // NEW_CHARACTER_EL
	}

	return false;
}

bool CX2StateCreateUnit::UIServerEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( CX2State::UIServerEventProc( hWnd, uMsg, wParam, lParam ) == true )
		return true;

#ifdef SERV_SECOND_SECURITY
	if( g_pMain->GetSecurityPad() != NULL )
	{
		if( true == g_pMain->GetSecurityPad()->UIServerEventProc( hWnd, uMsg, wParam, lParam ) )
			return true;
	}
#endif SERV_SECOND_SECURITY

	switch( wParam )
	{
		case EGS_CREATE_UNIT_ACK:
			return Handler_EGS_CREATE_UNIT_ACK( hWnd, uMsg, wParam, lParam );
			break;

		//case EGS_GET_SKILL_ACK:
		//	//return Handler_EGS_GET_SKILL_ACK( hWnd, uMsg, wParam, lParam );
		//	break;

		//case EGS_CHANGE_SKILL_SLOT_ACK:
		//	//return Handler_EGS_CHANGE_SKILL_SLOT_ACK( hWnd, uMsg, wParam, lParam );
		//	break;
	}
	return false;
}





void CX2StateCreateUnit::EquipPromotionItem( CX2Unit::UNIT_CLASS eUnitClass )
{
	switch( eUnitClass )
	{
	case CX2Unit::UC_ELSWORD_SWORDMAN:
		{
			for( int i=0; i<ARRAY_SIZE(ELSWORD_PROMOTION_ITEM_ID); i++ )
			{
				m_pUnitViewerUI->AddEquipByTID( ELSWORD_PROMOTION_ITEM_ID[i] );
			}
		} break;

	case CX2Unit::UC_ARME_VIOLET_MAGE:
		{
			for( int i=0; i<ARRAY_SIZE(AISHA_PROMOTION_ITEM_ID); i++ )
			{
				m_pUnitViewerUI->AddEquipByTID( AISHA_PROMOTION_ITEM_ID[i] );
			}
		} break;

	case CX2Unit::UC_LIRE_ELVEN_RANGER:
		{
			for( int i=0; i<ARRAY_SIZE(RENA_PROMOTION_ITEM_ID); i++ )
			{
				m_pUnitViewerUI->AddEquipByTID( RENA_PROMOTION_ITEM_ID[i] );
			}
		} break;

	case CX2Unit::UC_RAVEN_FIGHTER:
		{
			for( int i=0; i<ARRAY_SIZE(RAVEN_PROMOTION_ITEM_ID); i++ )
			{
				m_pUnitViewerUI->AddEquipByTID( RAVEN_PROMOTION_ITEM_ID[i] );
			}
		} break;

	case CX2Unit::UC_EVE_NASOD:
		{
			for( int i=0; i<ARRAY_SIZE(EVE_PROMOTION_ITEM_ID); i++ )
			{
				m_pUnitViewerUI->AddEquipByTID( EVE_PROMOTION_ITEM_ID[i] );
			}
		} break;

//{{ kimhc // 2010.11.24 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
	case CX2Unit::UC_CHUNG_IRON_CANNON:
		{
			for( int i=0; i<ARRAY_SIZE(CHUNG_PROMOTION_ITEM_ID); i++ )
			{
				m_pUnitViewerUI->AddEquipByTID( CHUNG_PROMOTION_ITEM_ID[i] );
			}
		} break;
#endif	NEW_CHARACTER_CHUNG
//}} kimhc // 2010.11.24 //  2010-12-23 New Character CHUNG

#ifdef ARA_CHARACTER_BASE
	case CX2Unit::UC_ARA_MARTIAL_ARTIST:
		{
			for( int i = 0; i < ARRAY_SIZE( ARA_PROMOTION_ITEM_ID ); ++i )
			{
				m_pUnitViewerUI->AddEquipByTID( ARA_PROMOTION_ITEM_ID[i] );
			}
		} break;
#endif
#ifdef NEW_CHARACTER_EL // 프로모션 아이템
	case CX2Unit::UC_ELESIS_KNIGHT:
		{
			for( int i = 0; i < ARRAY_SIZE( EL_PROMOTION_ITEM_ID ); ++i )
			{
				m_pUnitViewerUI->AddEquipByTID( EL_PROMOTION_ITEM_ID[i] );
			}
		} break;
#endif // NEW_CHARACTER_EL
	}

}

void CX2StateCreateUnit::UnSelect()
{
	CX2UnitViewerUI::DeleteKTDGObject( m_pUnitViewerUI );
	m_pUnitViewerUI = NULL;

	m_SelectUnitClass = CX2Unit::UC_NONE;
	m_bSelectUnit = false;
}

void CX2StateCreateUnit::SelectElSword()
{
	if( m_SelectUnitClass == CX2Unit::UC_ELSWORD_SWORDMAN )
		return;

	UnSelect();

	if ( m_bSelectUnit == false )
	{
		//StaticControlAnimate();	

		m_bSelectUnit = true;
	}

	OpenNickNameBox();

	m_pUnitViewerUI = CX2UnitViewerUI::CreateUnitViewerUI();
	g_pKTDXApp->GetDGManager()->AddObjectChain( m_pUnitViewerUI );

#ifdef SEPARATION_MOTION
	m_pUnitViewerUI->SetUnit( m_pDummyElsword, CX2UnitViewerUI::UVS_CREATEUNIT ); 
#else
	m_pUnitViewerUI->SetUnit( m_pDummyElsword );
#endif
	m_pUnitViewerUI->GetMatrix().Move( D3DXVECTOR3( -250, -70, 200 ) );
	m_pUnitViewerUI->GetMatrix().Rotate(D3DXToRadian(0),D3DXToRadian(0),D3DXToRadian(0));
	m_pUnitViewerUI->GetMatrix().Scale( D3DXVECTOR3(1.6f, 1.6f, 1.6f) );
	//m_pUnitViewerUI->SetLightPos( 1000, 1000, -200 );
	m_pUnitViewerUI->SetLightPos( 300, 300, -500 );	// 캐릭터뷰어 라이트 위치 변경
	m_pUnitViewerUI->PlayAnim( L"LobbyDownLanding", CKTDGXSkinAnim::XAP_ONE_WAIT );

	m_SelectUnitClass = CX2Unit::UC_ELSWORD_SWORDMAN;

	//m_pUnitViewerUI->RemoveAllEquips();
	EquipPromotionItem( m_SelectUnitClass );

	m_pUnitViewerUI->OnFrameMove( g_pKTDXApp->GetTime(),g_pKTDXApp->GetElapsedTime() );






	CKTDGUIStatic* pStatic = NULL;


	for ( int i = 0; i < CX2Unit::UT_END - 1; i++ )
	{
		WCHAR buff[256] = {0};
		StringCchPrintf( buff, 256, L"StaticUnit_Create_Unit%d", i );
		//wsprintf( buff, L"StaticUnit_Create_Unit%d", i );
		pStatic = (CKTDGUIStatic*)m_pDLGCreateUnitBack->GetControl( buff );		// note!! 신캐릭터 추가시 스크립트에 추가
		if ( pStatic != NULL )
			pStatic->SetShow( false );
	}

	pStatic = (CKTDGUIStatic*)m_pDLGCreateUnitBack->GetControl( L"StaticUnit_Create_Unit0" );
	if ( pStatic != NULL )
		pStatic->SetShow( true );





	g_pKTDXApp->SkipFrame();
}

void CX2StateCreateUnit::SelectArme()
{
	if( m_SelectUnitClass == CX2Unit::UC_ARME_VIOLET_MAGE )
		return;

	UnSelect();

	if ( m_bSelectUnit == false )
	{
		//StaticControlAnimate();

		m_bSelectUnit = true;
	}
	
	OpenNickNameBox();


	m_pUnitViewerUI = CX2UnitViewerUI::CreateUnitViewerUI();
	g_pKTDXApp->GetDGManager()->AddObjectChain( m_pUnitViewerUI );

	m_pUnitViewerUI->SetUnit( m_pDummyArme );
	m_pUnitViewerUI->GetMatrix().Move( D3DXVECTOR3( -250, -70, 200 ) );
	m_pUnitViewerUI->GetMatrix().Rotate(D3DXToRadian(0),D3DXToRadian(0),D3DXToRadian(0));
	m_pUnitViewerUI->GetMatrix().Scale( D3DXVECTOR3(1.6f, 1.6f, 1.6f) );
	//m_pUnitViewerUI->SetLightPos( 1000, 1000, -200 );
	m_pUnitViewerUI->SetLightPos( 300, 300, -500 );	// 캐릭터뷰어 라이트 위치 변경
	m_pUnitViewerUI->PlayAnim( L"LobbyDownLanding", CKTDGXSkinAnim::XAP_ONE_WAIT );

	m_SelectUnitClass = CX2Unit::UC_ARME_VIOLET_MAGE;
	//m_pUnitViewerUI->RemoveAllEquips();
	EquipPromotionItem( m_SelectUnitClass );

	m_pUnitViewerUI->GetXSkinAnim()->SetApplyMotionOffset( false );
	m_pUnitViewerUI->OnFrameMove( g_pKTDXApp->GetTime(),g_pKTDXApp->GetElapsedTime() );



	CKTDGUIStatic* pStatic = NULL;


	for ( int i = 0; i < CX2Unit::UT_END - 1; i++ )
	{
		WCHAR buff[256] = {0};
		StringCchPrintf( buff, 256, L"StaticUnit_Create_Unit%d", i );
		//wsprintf( buff, L"StaticUnit_Create_Unit%d", i );
		pStatic = (CKTDGUIStatic*)m_pDLGCreateUnitBack->GetControl( buff );		// note!! 신캐릭터 추가시 스크립트에 추가
		if ( pStatic != NULL )
			pStatic->SetShow( false );
	}

	pStatic = (CKTDGUIStatic*)m_pDLGCreateUnitBack->GetControl( L"StaticUnit_Create_Unit1" );
	if( pStatic != NULL )
		pStatic->SetShow( true );




	g_pKTDXApp->SkipFrame();
}


void CX2StateCreateUnit::SelectLire()
{
	if( m_SelectUnitClass == CX2Unit::UC_LIRE_ELVEN_RANGER )
		return;

	UnSelect();

	if ( m_bSelectUnit == false )
	{	
		//StaticControlAnimate();

		m_bSelectUnit = true;
	}
	OpenNickNameBox();


	m_pUnitViewerUI = CX2UnitViewerUI::CreateUnitViewerUI();
	g_pKTDXApp->GetDGManager()->AddObjectChain( m_pUnitViewerUI );

	m_pUnitViewerUI->SetUnit( m_pDummyLire );
	m_pUnitViewerUI->GetMatrix().Move( D3DXVECTOR3(-250, -70, 200  ) );
	m_pUnitViewerUI->GetMatrix().Rotate(D3DXToRadian(0),D3DXToRadian(0),D3DXToRadian(0));
	m_pUnitViewerUI->GetMatrix().Scale( D3DXVECTOR3(1.6f, 1.6f, 1.6f) );
	//m_pUnitViewerUI->SetLightPos( 1000, 1000, -200 );
	m_pUnitViewerUI->SetLightPos( 300, 300, -500 );	// 캐릭터뷰어 라이트 위치 변경
	m_pUnitViewerUI->PlayAnim( L"LobbyDownLanding", CKTDGXSkinAnim::XAP_ONE_WAIT );

	m_SelectUnitClass = CX2Unit::UC_LIRE_ELVEN_RANGER;
	//m_pUnitViewerUI->RemoveAllEquips();
	EquipPromotionItem( m_SelectUnitClass );

	m_pUnitViewerUI->GetXSkinAnim()->SetApplyMotionOffset( false );
	m_pUnitViewerUI->OnFrameMove( g_pKTDXApp->GetTime(),g_pKTDXApp->GetElapsedTime() );


	CKTDGUIStatic* pStatic = NULL;

	for ( int i = 0; i < CX2Unit::UT_END - 1; i++ )
	{
		WCHAR buff[256] = {0};
		StringCchPrintf( buff, 256, L"StaticUnit_Create_Unit%d", i );
		//wsprintf( buff, L"StaticUnit_Create_Unit%d", i );
		pStatic = (CKTDGUIStatic*)m_pDLGCreateUnitBack->GetControl( buff );			// note!! 신캐릭터 추가시 스크립트에 추가
		if ( pStatic != NULL )
			pStatic->SetShow( false );
	}

	pStatic = (CKTDGUIStatic*)m_pDLGCreateUnitBack->GetControl( L"StaticUnit_Create_Unit2" );
	if ( pStatic != NULL )
		pStatic->SetShow( true );






	g_pKTDXApp->SkipFrame();
}

void CX2StateCreateUnit::SelectRaven()
{
	if( m_SelectUnitClass == CX2Unit::UC_RAVEN_FIGHTER )
		return;

	UnSelect();

	if ( m_bSelectUnit == false )
	{
		//StaticControlAnimate();	

		m_bSelectUnit = true;
	}

	OpenNickNameBox();

	m_pUnitViewerUI = CX2UnitViewerUI::CreateUnitViewerUI();
	g_pKTDXApp->GetDGManager()->AddObjectChain( m_pUnitViewerUI );

	m_pUnitViewerUI->SetUnit( m_pDummyRaven );
	m_pUnitViewerUI->GetMatrix().Move( D3DXVECTOR3( -250, -70, 200 ) );
	m_pUnitViewerUI->GetMatrix().Rotate(D3DXToRadian(0),D3DXToRadian(0),D3DXToRadian(0));
	m_pUnitViewerUI->GetMatrix().Scale( D3DXVECTOR3(1.6f, 1.6f, 1.6f) );
	//m_pUnitViewerUI->SetLightPos( 1000, 1000, -200 );
	m_pUnitViewerUI->SetLightPos( 300, 300, -500 );	// 캐릭터뷰어 라이트 위치 변경
	m_pUnitViewerUI->PlayAnim( L"LobbyDownLanding", CKTDGXSkinAnim::XAP_ONE_WAIT );


	m_SelectUnitClass = CX2Unit::UC_RAVEN_FIGHTER;
	//m_pUnitViewerUI->RemoveAllEquips();
	EquipPromotionItem( m_SelectUnitClass );

	m_pUnitViewerUI->OnFrameMove( g_pKTDXApp->GetTime(),g_pKTDXApp->GetElapsedTime() );






	CKTDGUIStatic* pStatic = NULL;


	for ( int i = 0; i < CX2Unit::UT_END - 1; i++ )
	{
		WCHAR buff[256] = {0};
		StringCchPrintf( buff, 256, L"StaticUnit_Create_Unit%d", i );
		//wsprintf( buff, L"StaticUnit_Create_Unit%d", i );
		pStatic = (CKTDGUIStatic*)m_pDLGCreateUnitBack->GetControl( buff );			// note!! 신캐릭터 추가시 스크립트에 추가
		if ( pStatic != NULL )
			pStatic->SetShow( false );
	}

	pStatic = (CKTDGUIStatic*)m_pDLGCreateUnitBack->GetControl( L"StaticUnit_Create_Unit3" );
	if ( pStatic != NULL )
		pStatic->SetShow( true );



	g_pKTDXApp->SkipFrame();
}


void CX2StateCreateUnit::SelectEve()
{

	if( m_SelectUnitClass == CX2Unit::UC_EVE_NASOD )
		return;

	UnSelect();

	if ( m_bSelectUnit == false )
	{
		//StaticControlAnimate();	

		m_bSelectUnit = true;
	}

	OpenNickNameBox();

	m_pUnitViewerUI = CX2UnitViewerUI::CreateUnitViewerUI();
	g_pKTDXApp->GetDGManager()->AddObjectChain( m_pUnitViewerUI );

	m_pUnitViewerUI->SetUnit( m_pDummyEve );
	m_pUnitViewerUI->GetMatrix().Move( D3DXVECTOR3( -250, -70, 200 ) );
	m_pUnitViewerUI->GetMatrix().Rotate(D3DXToRadian(0),D3DXToRadian(0),D3DXToRadian(0));
	m_pUnitViewerUI->GetMatrix().Scale( D3DXVECTOR3(1.6f, 1.6f, 1.6f) );
	//m_pUnitViewerUI->SetLightPos( 1000, 1000, -200 );
	m_pUnitViewerUI->SetLightPos( 300, 300, -500 );	// 캐릭터뷰어 라이트 위치 변경
	m_pUnitViewerUI->PlayAnim( L"LobbyDownLanding", CKTDGXSkinAnim::XAP_ONE_WAIT );

	m_SelectUnitClass = CX2Unit::UC_EVE_NASOD;
	//m_pUnitViewerUI->RemoveAllEquips();
	EquipPromotionItem( m_SelectUnitClass );

	m_pUnitViewerUI->OnFrameMove( g_pKTDXApp->GetTime(),g_pKTDXApp->GetElapsedTime() );



	CKTDGUIStatic* pStatic = NULL;

	for ( int i = 0; i < CX2Unit::UT_END - 1; i++ )
	{
		WCHAR buff[256] = {0};
		StringCchPrintf( buff, 256, L"StaticUnit_Create_Unit%d", i );
		//wsprintf( buff, L"StaticUnit_Create_Unit%d", i );
		pStatic = (CKTDGUIStatic*)m_pDLGCreateUnitBack->GetControl( buff );			// note!! 신캐릭터 추가시 스크립트에 추가
		if ( pStatic != NULL )
			pStatic->SetShow( false );
	}

	pStatic = (CKTDGUIStatic*)m_pDLGCreateUnitBack->GetControl( L"StaticUnit_Create_Unit4" );	
	if ( pStatic != NULL )
		pStatic->SetShow( true );


	g_pKTDXApp->SkipFrame();
}

//{{ kimhc // 2010.11.24 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG

/** @function : SelectChung
	@brief : 청을 선택했을 때 동작하는 함수로 닉네임박스, 캐릭터 UnitViewer 등을 보여준다.
*/
void CX2StateCreateUnit::SelectChung()
{

	if( m_SelectUnitClass == CX2Unit::UC_CHUNG_IRON_CANNON )
		return;

	UnSelect();

	if ( m_bSelectUnit == false )
		m_bSelectUnit = true;

	OpenNickNameBox();

	m_pUnitViewerUI = CX2UnitViewerUI::CreateUnitViewerUI();
	g_pKTDXApp->GetDGManager()->AddObjectChain( m_pUnitViewerUI );

	m_pUnitViewerUI->SetUnit( m_pDummyChung );
	m_pUnitViewerUI->GetMatrix().Move( D3DXVECTOR3( -250, -70, 200 ) );
	m_pUnitViewerUI->GetMatrix().Rotate(D3DXToRadian(0),D3DXToRadian(0),D3DXToRadian(0));
	m_pUnitViewerUI->GetMatrix().Scale( D3DXVECTOR3(1.6f, 1.6f, 1.6f) );
	m_pUnitViewerUI->SetLightPos( 300, 300, -500 );	// 캐릭터뷰어 라이트 위치 변경
	m_pUnitViewerUI->PlayAnim( L"LobbyDownLanding", CKTDGXSkinAnim::XAP_ONE_WAIT );

	m_SelectUnitClass = CX2Unit::UC_CHUNG_IRON_CANNON;
	EquipPromotionItem( m_SelectUnitClass );

	m_pUnitViewerUI->OnFrameMove( g_pKTDXApp->GetTime(),g_pKTDXApp->GetElapsedTime() );

	const int STR_BUF_SIZE = 256;
	for ( int i = 0; i < CX2Unit::UT_END - 1; i++ )
	{
		WCHAR buff[STR_BUF_SIZE] = {0, };
		StringCchPrintf( buff, 256, L"StaticUnit_Create_Unit%d", i );
		
		CKTDGUIStatic* pStatic = (CKTDGUIStatic*)m_pDLGCreateUnitBack->GetControl( buff );			// note!! 신캐릭터 추가시 스크립트에 추가
		if ( pStatic != NULL )
			pStatic->SetShow( false );
	}

	CKTDGUIStatic* pStatic = (CKTDGUIStatic*)m_pDLGCreateUnitBack->GetControl( L"StaticUnit_Create_Unit5" );	
	
	if ( pStatic != NULL )
		pStatic->SetShow( true );


	g_pKTDXApp->SkipFrame();
}

#endif	NEW_CHARACTER_CHUNG
//}} kimhc // 2010.11.24 //  2010-12-23 New Character CHUNG

#ifdef ARA_CHARACTER_BASE
#pragma region SelectAra
/** @function : SelectAra
	@brief : "아라"을 선택했을 때 동작하는 함수로 닉네임박스, 캐릭터 UnitViewer 등을 보여줌
*/
void CX2StateCreateUnit::SelectAra()
{

	if( m_SelectUnitClass == CX2Unit::UC_ARA_MARTIAL_ARTIST )
		return;

	UnSelect();

	if ( m_bSelectUnit == false )
		m_bSelectUnit = true;

	OpenNickNameBox();

	m_pUnitViewerUI = CX2UnitViewerUI::CreateUnitViewerUI();
	g_pKTDXApp->GetDGManager()->AddObjectChain( m_pUnitViewerUI );

	m_pUnitViewerUI->SetUnit( m_pDummyAra );
	m_pUnitViewerUI->GetMatrix().Move( D3DXVECTOR3( -250, -70, 200 ) );
	m_pUnitViewerUI->GetMatrix().Rotate(D3DXToRadian(0),D3DXToRadian(0),D3DXToRadian(0));
	m_pUnitViewerUI->GetMatrix().Scale( D3DXVECTOR3(1.6f, 1.6f, 1.6f) );
	m_pUnitViewerUI->SetLightPos( 300, 300, -500 );	// 캐릭터뷰어 라이트 위치 변경
	m_pUnitViewerUI->PlayAnim( L"LobbyDownLanding", CKTDGXSkinAnim::XAP_ONE_WAIT );

	m_SelectUnitClass = CX2Unit::UC_ARA_MARTIAL_ARTIST;
	EquipPromotionItem( m_SelectUnitClass );

	m_pUnitViewerUI->OnFrameMove( g_pKTDXApp->GetTime(),g_pKTDXApp->GetElapsedTime() );

	const int STR_BUF_SIZE = 256;
	for ( int i = 0; i < CX2Unit::UT_END - 1; i++ )
	{
		WCHAR buff[STR_BUF_SIZE] = {0, };
		StringCchPrintf( buff, 256, L"StaticUnit_Create_Unit%d", i );

		CKTDGUIStatic* pStatic = (CKTDGUIStatic*)m_pDLGCreateUnitBack->GetControl( buff );	// note!! 신캐릭터 추가시 스크립트에 추가
		if ( pStatic != NULL )
			pStatic->SetShow( false );
	}

	CKTDGUIStatic* pStatic = (CKTDGUIStatic*)m_pDLGCreateUnitBack->GetControl( L"StaticUnit_Create_Unit6" );	

	if ( pStatic != NULL )
		pStatic->SetShow( true );

	g_pKTDXApp->SkipFrame();
}
#pragma endregion 함수
#endif

#ifdef NEW_CHARACTER_EL
void CX2StateCreateUnit::SelectElesis()
{
	if( m_SelectUnitClass == CX2Unit::UC_ELESIS_KNIGHT )
		return;

	UnSelect();

	if ( m_bSelectUnit == false )
		m_bSelectUnit = true;

	OpenNickNameBox();

	m_pUnitViewerUI = CX2UnitViewerUI::CreateUnitViewerUI();
	g_pKTDXApp->GetDGManager()->AddObjectChain( m_pUnitViewerUI );

	m_pUnitViewerUI->SetUnit( m_pDummyEL );
	m_pUnitViewerUI->GetMatrix().Move( D3DXVECTOR3( -250, -70, 200 ) );
	m_pUnitViewerUI->GetMatrix().Rotate(D3DXToRadian(0),D3DXToRadian(0),D3DXToRadian(0));
	m_pUnitViewerUI->GetMatrix().Scale( D3DXVECTOR3(1.6f, 1.6f, 1.6f) );
	m_pUnitViewerUI->SetLightPos( 300, 300, -500 );	// 캐릭터뷰어 라이트 위치 변경
	m_pUnitViewerUI->PlayAnim( L"LobbyDownLanding", CKTDGXSkinAnim::XAP_ONE_WAIT );

	m_SelectUnitClass = CX2Unit::UC_ELESIS_KNIGHT;
	EquipPromotionItem( m_SelectUnitClass );

	m_pUnitViewerUI->OnFrameMove( g_pKTDXApp->GetTime(),g_pKTDXApp->GetElapsedTime() );

	const int STR_BUF_SIZE = 256;
	for ( int i = 0; i < CX2Unit::UT_END - 1; i++ )
	{
		WCHAR buff[STR_BUF_SIZE] = {0, };
		StringCchPrintf( buff, 256, L"StaticUnit_Create_Unit%d", i );

		CKTDGUIStatic* pStatic = (CKTDGUIStatic*)m_pDLGCreateUnitBack->GetControl( buff );
		if ( pStatic != NULL )
			pStatic->SetShow( false );
	}

	CKTDGUIStatic* pStatic = (CKTDGUIStatic*)m_pDLGCreateUnitBack->GetControl( L"StaticUnit_Create_Unit7" );	

	if ( pStatic != NULL )
		pStatic->SetShow( true );

	g_pKTDXApp->SkipFrame();
}
#endif // NEW_CHARACTER_EL


bool CX2StateCreateUnit::Handler_EGS_CREATE_UNIT_REQ()
{											
	
	CKTDGUIIMEEditBox* pIDEditBox = (CKTDGUIIMEEditBox*)m_pDLGCreateUnitFront->GetControl( L"IMEEditBoxUnitCreateNickName" );
	m_NickName = pIDEditBox->GetText();

	if( CreateErrorCheck() == false )
		return true;

	KEGS_CREATE_UNIT_REQ kPacket;
	kPacket.m_iClass			= m_SelectUnitClass;
	kPacket.m_wstrNickName		= m_NickName;

	g_pData->GetServerProtocol()->SendPacket( EGS_CREATE_UNIT_REQ, kPacket );
	g_pMain->AddServerPacket( EGS_CREATE_UNIT_ACK );
	SAFE_DELETE_DIALOG( m_pDLGMsgBox );
	m_pDLGMsgBox = g_pMain->KTDGUIMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_555 ), this );

	return true;	
}

bool CX2StateCreateUnit::Handler_EGS_CREATE_UNIT_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_CREATE_UNIT_ACK kEvent;
	DeSerialize( pBuff, &kEvent );
	CX2PacketLog::PrintLog( &kEvent );

	SAFE_DELETE_DIALOG( m_pDLGMsgBox );
	if( g_pMain->DeleteServerPacket( EGS_CREATE_UNIT_ACK ) == true )
	{
		bool bCheck = false;

		//{{ 2009.01.19 김태완 : 코드정리 elseif -> switch
		switch(kEvent.m_iOK)
		{
		case NetError::ERR_CREATE_UNIT_01:
			{
				SetAlarmAboutName( GET_STRING( STR_ID_556 ) );
				bCheck = true;
			} break;
		case NetError::ERR_CREATE_UNIT_04:
			{

				SetAlarmAboutName( GET_STRING( STR_ID_557 ) );
				bCheck = true;
			} break;
		case NetError::ERR_CREATE_UNIT_06:
			{
				wstringstream wstrstm;
                wstrstm << GET_REPLACED_STRING( ( STR_ID_558, "L", kEvent.m_wstrEnableDate ) );

				SetAlarmAboutName( wstrstm.str().c_str() );
				bCheck = true;
			} break;
		case NetError::ERR_CREATE_UNIT_08:
			{
				SetAlarmAboutName( GET_STRING( STR_ID_362 ) );
				bCheck = true;
			} break;
		default:
			break;
		}//}}


		if ( bCheck == true )
		{
			wstring wstrName( L"Emotion_No" );
			m_pUnitViewerUI->PlayEmotion( CX2Unit::ET_NO, wstrName );

			return true;
		}

		if( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{			
			//{{ 2009. 7. 6  최육사		사용하지 않는 기능이므로 주석처리하였습니다~
			// 처음 캐릭터를 생성한것인지 검사
			//if(kEvent.m_bIsFirstUnit == true)
			//{
			//	g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_2633 ), g_pMain->GetNowState() );
			//}
			//}}

			//유닛 정보 받아오기, 인벤토리 받아오기, 유닛 셀렉트로 이동하기

			KUnitInfo kUnitInfo = kEvent.m_kUnitInfo;
			CX2Unit* pUnit = new CX2Unit( kUnitInfo );

			pUnit->SetOwnerUserUID( g_pData->GetMyUser()->GetUID() );
			g_pData->GetMyUser()->AddUnit( pUnit );	
			m_UnitUID = pUnit->GetUID();
						
			SAFE_DELETE_DIALOG( m_pDLGMSGOkBoxEnterTutorial );

			//m_pDLGMSGOkBoxEnterTutorial = g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2(250, 300), L"튜토리얼을 진행하시겠습니까?",
			//	SCUUCM_ENTER_TUTORIAL_OK, this, SCUUCM_ENTER_TUTORIAL_CANCEL );

#ifndef REFORM_TUTORIAL //개발자 계정이라도 튜토리얼 진입하도록 수정
			if ( g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_OPERATOR )
			{
				CX2StateAutoChanger::TARGET_DETAIL targetDetail;
/*
				g_pMain->SetIsPlayingTutorial( true );
				g_pMain->CreateStateChangeDLG( GET_STRING( STR_ID_552 ) );
				g_pKTDXApp->SendGameMessage( XGM_STATE_CHANGE, / *CX2Main::XS_UNIT_SELECT* /CX2Main::XS_SERVER_SELECT, NULL, false ); 
*/

				g_pMain->GetStateAutoChanger().StartStateChange( (int)g_pMain->GetNowStateID(), CX2Main::XS_VILLAGE_MAP, targetDetail, m_UnitUID );

			}
			else
#endif //REFORM_TUTORIAL
			{
#ifdef SEND_URL_TO_NEXON
				// game start
				static bool bSended = false;
				if( bSended == false )
				{
			#ifdef SEND_NEXON_WISE_LOG_BY_URL // 김태환
					CX2User* pUser = g_pData->GetMyUser();

					if ( NULL != pUser && 
						 NULL != pUser->GetUserData() &&
						 NULL != g_pMain->GetBrowserWrapper() &&
						 pUser->GetUserData()->m_uChannelCode == 0 )	/// 넥슨 로그인 이어야 하고
					{
						if( false == g_pMain->GetBrowserWrapper()->IsClosed() )
							g_pMain->GetBrowserWrapper()->CloseBrowser();

						RECT		rcWebPage;
						char		buf[256]	= {0,};
						std::string strURL		= "http://elsword.nexon.com/wiselog/newplayer.aspx";							/// 로그 URL
						::WideCharToMultiByte( CP_ACP, 0, pUser->GetUserData()->userID.c_str(), -1, buf, 255, NULL, NULL );		/// wstring을 string 으로 변환
						std::string strNXID		= buf;								/// 넥슨 아이디
						UidType		uidNXUID	= pUser->GetUserData()->userUID;	/// 넥슨 UID

						g_pMain->GetBrowserWrapper()->Create( rcWebPage, g_pKTDXApp->GetHWND() );

						if ( pUser->GetUnitNum() == 1 )					/// 현재 생성한 것이 처음인 경우
						{
							/// 기존에 남기던 로그
							NMCOHelpers::WriteToWiselog( L"http://elsword.nexon.com/wiselog/newplayer.aspx" );
							
							/// URL 인자 조합
							strURL += "?GRU=true";	/// 신규 유저일 경우 추가되는 인자

							/// 첫 케릭터 생성 여부 설정
							pUser->SetFirstCreateCharacter( true );
						}

						StringCchPrintfA( buf, 255, "&NXID=%s&OID=%lld", strNXID.c_str(), uidNXUID );	/// 넥슨 아이디와 UID를 설정

						strURL += buf;				/// 스트링 조합

						g_pMain->GetBrowserWrapper()->Navigate( strURL.c_str(), NULL );					/// 웹페이지 URL로 설정

						bSended = true;
					}
			#else // SEND_NEXON_WISE_LOG_BY_URL
					const CX2User* pUser = g_pData->GetMyUser();

					if ( NULL != pUser && NULL != pUser->GetUserData() &&
						pUser->GetUserData()->m_uChannelCode == 0 &&	/// 넥슨 로그인 이어야 하고
						pUser->GetUnitNum() == 1 )	/// 현재 생성한 것이 처음인 경우
					{
						NMCOHelpers::WriteToWiselog( L"http://elsword.nexon.com/wiselog/newplayer.aspx" );
						bSended = true;
					}
			#endif // SEND_NEXON_WISE_LOG_BY_URL
				}						
#endif
				g_pMain->SetIsPlayingTutorial( true );
				if( m_pDLGMSGOkBoxEnterTutorial != NULL )
				{
					g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGMSGOkBoxEnterTutorial, NULL, false );
					m_pDLGMSGOkBoxEnterTutorial = NULL;
				}		

				g_pMain->CreateStateChangeDLG( GET_STRING( STR_ID_552 ) );
				g_pKTDXApp->SendGameMessage( XGM_STATE_CHANGE, /*CX2Main::XS_UNIT_SELECT*/CX2Main::XS_SERVER_SELECT, NULL, false ); 
			}	
		}

	}
	return true;
}

bool CX2StateCreateUnit::CreateErrorCheck()
{
	string multyByteString = "";
	ConvertWCHARToChar( multyByteString, m_NickName.c_str() );

	wstring wstrErrorMessage = L"";

#ifdef SCRIPT_SELECT
	bool bCharacterEnable = false;
	for( int i=0; i<m_pDLGCreateUnitFront->GetControlSize(); i++ )
	{
		CKTDGUIControl* pControl = m_pDLGCreateUnitFront->GetControl( i );
		if( NULL == pControl )
			continue;

		if( CKTDGUIControl::UCT_RADIOBUTTON != pControl->GetType() )
			continue;

		switch(m_SelectUnitClass)
		{
		case CX2Unit::UC_ELSWORD_SWORDMAN:
			{
				if( StrCmpW(pControl->GetName(), L"Elsword_Slot") == 0)
				{
					bCharacterEnable = true;
				}
			} break;
		case CX2Unit::UC_ARME_VIOLET_MAGE:
			{
				if( StrCmpW(pControl->GetName(), L"Aisha_Slot") == 0)
				{
					bCharacterEnable = true;
				}
			} break;
		case CX2Unit::UC_LIRE_ELVEN_RANGER:
			{
				if( StrCmpW(pControl->GetName(), L"Rena_Slot") == 0)
				{
					bCharacterEnable = true;
				}
			} break;
		case CX2Unit::UC_RAVEN_FIGHTER:
			{
				if( StrCmpW(pControl->GetName(), L"Raven_Slot") == 0)
				{
					bCharacterEnable = true;
				}
			} break;
		case CX2Unit::UC_EVE_NASOD:
			{
				if( StrCmpW(pControl->GetName(), L"Eve_Slot") == 0)
				{
					bCharacterEnable = true;
				}
			} break;

		//{{ kimhc // 2010.11.24 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
		case CX2Unit::UC_CHUNG_IRON_CANNON:
			{
				if( StrCmpW(pControl->GetName(), L"Chung_Slot") == 0)	// 임시
				{
					bCharacterEnable = true;
				}
			} break;
#endif	NEW_CHARACTER_CHUNG
//}} kimhc // 2010.11.24 //  2010-12-23 New Character CHUNG

#ifdef ARA_CHARACTER_BASE
		case CX2Unit::UC_ARA_MARTIAL_ARTIST:
			{
				if( StrCmpW(pControl->GetName(), L"Ara_Slot") == 0)
					bCharacterEnable = true;
			} break;
#endif
#ifdef NEW_CHARACTER_EL
		case CX2Unit::UC_EL_KNIGHT:
			{
				if( StrCmpW(pControl->GetName(), L"EL_Slot") == 0)
					bCharacterEnable = true;
			} break;
#endif // NEW_CHARACTER_EL
		}
		if(bCharacterEnable == true)
			break;
	}
	if(bCharacterEnable == false)
		return false;
#endif


	if ( m_SelectUnitClass == CX2Unit::UC_NONE )
	{
		SetAlarmAboutName(  GET_STRING( STR_ID_559 ) );
		return false;
	}
	else if( true == CX2Main::IsInvalidNickName( m_NickName, wstrErrorMessage ) )
	{
		SetAlarmAboutName( wstrErrorMessage.c_str() );
		wstring wstrName( L"Emotion_No" );
		m_pUnitViewerUI->PlayEmotion( CX2Unit::ET_NO, wstrName );

		return false;
	}

	return true;
}

void CX2StateCreateUnit::InitDummyUnits()
{
	KUnitInfo kUnitInfo;
	kUnitInfo.Init();
	kUnitInfo.m_wstrNickName = L"DummyElsword";
	kUnitInfo.m_cUnitClass = CX2Unit::UC_ELSWORD_SWORDMAN;
	m_pDummyElsword = new CX2Unit( kUnitInfo );
	//ReadyUnitResources( m_pDummyElsword );

	kUnitInfo.Init();
	kUnitInfo.m_wstrNickName = L"DummyArme";
	kUnitInfo.m_cUnitClass = CX2Unit::UC_ARME_VIOLET_MAGE;
	m_pDummyArme = new CX2Unit( kUnitInfo );
	//ReadyUnitResources( m_pDummyArme );

	kUnitInfo.Init();
	kUnitInfo.m_wstrNickName = L"DummyLire";
	kUnitInfo.m_cUnitClass = CX2Unit::UC_LIRE_ELVEN_RANGER;
	m_pDummyLire = new CX2Unit( kUnitInfo );
	//ReadyUnitResources( m_pDummyLire );

	kUnitInfo.Init();
	kUnitInfo.m_wstrNickName = L"DummyRaven";
	kUnitInfo.m_cUnitClass = CX2Unit::UC_RAVEN_FIGHTER;
	m_pDummyRaven = new CX2Unit( kUnitInfo );
	//ReadyUnitResources( m_pDummyRaven );



	kUnitInfo.Init();
	kUnitInfo.m_wstrNickName = L"DummyEve";
	kUnitInfo.m_cUnitClass = CX2Unit::UC_EVE_NASOD;
	m_pDummyEve = new CX2Unit( kUnitInfo );
	//ReadyUnitResources( m_pDummyEve );


//{{ kimhc // 2010.11.24 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
	kUnitInfo.Init();
	kUnitInfo.m_wstrNickName = L"DummyChung";
	kUnitInfo.m_cUnitClass = CX2Unit::UC_CHUNG_IRON_CANNON;
	m_pDummyChung = new CX2Unit( kUnitInfo );
	XSkinMeshReady(L"CDC_MiniGun_L_Start.X");
	XSkinMeshReady(L"CDC_MiniGun_R_Start.X");
#endif	NEW_CHARACTER_CHUNG
//}} kimhc // 2010.11.24 //  2010-12-23 New Character CHUNG

#ifdef ARA_CHARACTER_BASE
	kUnitInfo.Init();
	kUnitInfo.m_wstrNickName = L"DummyAra";
	kUnitInfo.m_cUnitClass = CX2Unit::UC_ARA_MARTIAL_ARTIST;
	m_pDummyAra = new CX2Unit( kUnitInfo );
#endif
#ifdef NEW_CHARACTER_EL
	kUnitInfo.Init();
	kUnitInfo.m_wstrNickName = L"DummyEL";
	kUnitInfo.m_cUnitClass = CX2Unit::UC_ELESIS_KNIGHT;
	m_pDummyEL = new CX2Unit( kUnitInfo );
#endif // NEW_CHARACTER_EL
	ReadyPromotionUnitResources();

}


void CX2StateCreateUnit::ReadyPromotionUnitResources()
{
    const CX2Item::ItemTemplet* pTemplet = NULL;

	for( int i=0; i<ARRAY_SIZE(ELSWORD_PROMOTION_ITEM_ID); i++ )
	{
		pTemplet = g_pData->GetItemManager()->GetItemTemplet( ELSWORD_PROMOTION_ITEM_ID[i] );
		if( pTemplet != NULL )
			XSkinMeshReady( pTemplet->GetModelName(0) );
	}

	for( int i=0; i<ARRAY_SIZE(AISHA_PROMOTION_ITEM_ID); i++ )
	{
		pTemplet = g_pData->GetItemManager()->GetItemTemplet( AISHA_PROMOTION_ITEM_ID[i] );
		if( pTemplet != NULL )
			XSkinMeshReady( pTemplet->GetModelName(0) );
	}



	for( int i=0; i<ARRAY_SIZE(RENA_PROMOTION_ITEM_ID); i++ )
	{
		pTemplet = g_pData->GetItemManager()->GetItemTemplet( RENA_PROMOTION_ITEM_ID[i] );
		if( pTemplet != NULL )
			XSkinMeshReady( pTemplet->GetModelName(0) );
	}

	for( int i=0; i<ARRAY_SIZE(RAVEN_PROMOTION_ITEM_ID); i++ )
	{
		pTemplet = g_pData->GetItemManager()->GetItemTemplet( RAVEN_PROMOTION_ITEM_ID[i] );
		if( pTemplet != NULL )
			XSkinMeshReady( pTemplet->GetModelName(0) );
	}


	for( int i=0; i<ARRAY_SIZE(EVE_PROMOTION_ITEM_ID); i++ )
	{
		pTemplet = g_pData->GetItemManager()->GetItemTemplet( EVE_PROMOTION_ITEM_ID[i] );
		if( pTemplet != NULL )
			XSkinMeshReady( pTemplet->GetModelName(0) );
	}

	//{{ kimhc // 2010.11.24 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG 

	for( int i=0; i<ARRAY_SIZE(CHUNG_PROMOTION_ITEM_ID); i++ )
	{
		pTemplet = g_pData->GetItemManager()->GetItemTemplet( CHUNG_PROMOTION_ITEM_ID[i] );
		if( pTemplet != NULL )
			XSkinMeshReady( pTemplet->GetModelName(0) );
	}

#endif	NEW_CHARACTER_CHUNG
//}} kimhc // 2010.11.24 //  2010-12-23 New Character CHUNG

#ifdef ARA_CHARACTER_BASE
	for( int i = 0; i < ARRAY_SIZE( ARA_PROMOTION_ITEM_ID ); ++i )
	{
		pTemplet = g_pData->GetItemManager()->GetItemTemplet( ARA_PROMOTION_ITEM_ID[i] );
		if( pTemplet != NULL )
			XSkinMeshReady( pTemplet->GetModelName(0) );
	}
#endif
#ifdef NEW_CHARACTER_EL // 프로모션 아이템
	for( int i = 0; i < ARRAY_SIZE( EL_PROMOTION_ITEM_ID ); ++i )
	{
		pTemplet = g_pData->GetItemManager()->GetItemTemplet( EL_PROMOTION_ITEM_ID[i] );
		if( pTemplet != NULL )
			XSkinMeshReady( pTemplet->GetModelName(0) );
	}
#endif // NEW_CHARACTER_EL

}


bool CX2StateCreateUnit::ReadyUnitResources( CX2Unit* pUnit )
{
	if ( pUnit == NULL )
		return false;

	for( int j =0; j < (int)pUnit->GetUnitData()->m_NowEqipItemUIDList.size(); j++ )
	{
		CX2Item* pItem = pUnit->GetInventory()->GetItem( pUnit->GetUnitData()->m_NowEqipItemUIDList[j] );
        XSkinMeshReady( pItem->GetItemTemplet()->GetModelName(0) );
	}

    const CX2Item::ItemTemplet* pTemplet = NULL;
	pTemplet = g_pData->GetItemManager()->GetItemTemplet( pUnit->GetUnitTemplet()->m_BasicHairItemID );
	if( pTemplet != NULL )
		XSkinMeshReady( pTemplet->GetModelName(0) );
	pTemplet = g_pData->GetItemManager()->GetItemTemplet( pUnit->GetUnitTemplet()->m_BasicFaceItemID );
	if( pTemplet != NULL )
		XSkinMeshReady( pTemplet->GetModelName(0) );
	pTemplet = g_pData->GetItemManager()->GetItemTemplet( pUnit->GetUnitTemplet()->m_BasicBodyItemID );
	if( pTemplet != NULL )
		XSkinMeshReady( pTemplet->GetModelName(0) );
	pTemplet = g_pData->GetItemManager()->GetItemTemplet( pUnit->GetUnitTemplet()->m_BasicLegItemID );
	if( pTemplet != NULL )
		XSkinMeshReady( pTemplet->GetModelName(0) );
	pTemplet = g_pData->GetItemManager()->GetItemTemplet( pUnit->GetUnitTemplet()->m_BasicHandItemID );
	if( pTemplet != NULL )
		XSkinMeshReady( pTemplet->GetModelName(0) );
	pTemplet = g_pData->GetItemManager()->GetItemTemplet( pUnit->GetUnitTemplet()->m_BasicFootItemID );
	if( pTemplet != NULL )
		XSkinMeshReady( pTemplet->GetModelName(0) );
	pTemplet = g_pData->GetItemManager()->GetItemTemplet( pUnit->GetUnitTemplet()->m_BasicWeaponItemID );
	if( pTemplet != NULL )
		XSkinMeshReady( pTemplet->GetModelName(0) );

	// 레이븐은 다른 캐릭터와 다르게 왼팔, 오른팔 메시를 나누어서 사용한다
	pTemplet = g_pData->GetItemManager()->GetItemTemplet( pUnit->GetUnitTemplet()->m_RavenLeftArmItemID );
	if( pTemplet != NULL )
		XSkinMeshReady( pTemplet->GetModelName(0) );
	// dmlee 2008.12.12 - 두 번째 무기
	pTemplet = g_pData->GetItemManager()->GetItemTemplet( pUnit->GetUnitTemplet()->m_SecondWeaponItemID );
	if( pTemplet != NULL )
		XSkinMeshReady( pTemplet->GetModelName(0) );
	
	return true;
}


void CX2StateCreateUnit::StaticControlAnimate()
{
	m_pDLGCreateUnitFront->GetControl( L"StaticUnit_Create_Info_Window" )->SetOffsetPos( D3DXVECTOR2( -350, 0 ) );
	m_pDLGCreateUnitFront->GetControl( L"StaticUnit_Create_ID_box" )->SetColor( D3DXCOLOR(0,0,0,0) );
	m_pDLGCreateUnitFront->GetControl( L"StaticUnit_Create_ID_box" )->SetOffsetPos( D3DXVECTOR2( -10, 10 ) );
	m_pDLGCreateUnitFront->GetControl( L"StaticUnit_Create_ID_box" )->SetShow( true );
//	m_pDLGCreateUnitFront->GetControl( L"StaticCreateUnitNickName" )->SetOffsetPos( D3DXVECTOR2(-350, 0) );
	m_pDLGCreateUnitFront->GetControl( L"IMEEditBoxUnitCreateNickName" )->SetOffsetPos( D3DXVECTOR2(-10, 10) );


	m_pDLGCreateUnitFront->GetControl( L"StaticUnit_Create_ID_box" )->Move(D3DXVECTOR2(2,-2), D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.05f, true );
	m_pDLGCreateUnitFront->GetControl( L"StaticUnit_Create_ID_box" )->Move(D3DXVECTOR2(0,0), D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.05f, false );
	m_pDLGCreateUnitFront->GetControl( L"StaticUnit_Create_Info_Window" )->Move(D3DXVECTOR2(0,0), D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.2f, false );
//	m_pDLGCreateUnitFront->GetControl( L"StaticCreateUnitNickName" )->Move(D3DXVECTOR2(0,0), D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.2f, true );
	m_pDLGCreateUnitFront->GetControl( L"IMEEditBoxUnitCreateNickName" )->Move(D3DXVECTOR2(2,-2), D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.05f, true );
	m_pDLGCreateUnitFront->GetControl( L"IMEEditBoxUnitCreateNickName" )->Move(D3DXVECTOR2(0,0), D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.05f, false );
}

void CX2StateCreateUnit::OpenNickNameBox()
{
	m_pDLGCreateUnitFront->GetControl( L"Character_Create_ID" )->SetShow( true );
	//m_pDLGCreateUnitFront->GetControl( L"Character_Create")->SetShow( true );
	m_pDLGCreateUnitFront->GetControl( L"ButtonCreate" )->SetShow( true );

	m_pDLGCreateUnitFront->GetControl( L"IMEEditBoxUnitCreateNickName" )->SetEnable( true );
	m_pDLGCreateUnitFront->GetControl( L"IMEEditBoxUnitCreateNickName" )->RequestFocus();
}

void CX2StateCreateUnit::SetAlarmAboutName( const WCHAR* pContent )
{
	CKTDGUIStatic* pStatic = (CKTDGUIStatic*)m_pDLGCreateUnitFront->GetControl(L"Character_Create_ID" );
	if ( pStatic != NULL && pStatic->GetString(0) != NULL )
	{
		pStatic->GetString(0)->msg = pContent;
	}
}




/*virtual*/ bool CX2StateCreateUnit::LastMsgByESC()
{
#ifdef SERV_SECOND_SECURITY
	// oasis907 : 김상윤 [2011.6.8] 보안패드 활성시 서버 선택창에서 ESC키 막기
	if(g_pMain->GetSecurityPad() != NULL &&
		g_pMain->GetSecurityPad()->GetShow() == true)
	{
		return true;
	}
#endif SERV_SECOND_SECURITY


	//{{ kimhc // 2009-12-23 // 서버군 추가로 유닛이 없는 경우 채널 선택창으로 이동 한다는 팝업
#ifdef	ADD_SERVER_GROUP
	if ( g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUnitNum() == 0 )
		OpenLastMsgPopUp( GET_STRING( STR_ID_736 ) );
	else
#endif	ADD_SERVER_GROUP
		//}} kimhc // 2009-12-23 // 서버군 추가로 유닛이 없는 경우 채널 선택창으로 이동 한다는 팝업
	OpenLastMsgPopUp( GET_STRING( STR_ID_560 ) );
	return true;
}

/*virtual*/ void CX2StateCreateUnit::LastMsgPopOk()
{
	if ( m_pDLGLastMsgPopUp != NULL )
		g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGLastMsgPopUp, NULL, false );
	m_pDLGLastMsgPopUp = NULL;

	//{{ kimhc // 2009-12-23 // 서버군 추가로 유닛이 없는 경우 채널 선택창으로 이동 한다는 팝업
#ifdef	ADD_SERVER_GROUP
	if ( g_pData->GetMyUser() != NULL && g_pData->GetMyUser()->GetUnitNum() == 0 )
		g_pMain->CreateStateChangeDLG( GET_STRING( STR_ID_4837 ) );
	else
#endif	ADD_SERVER_GROUP
	g_pMain->CreateStateChangeDLG( GET_STRING( STR_ID_552 ) );
	g_pKTDXApp->SendGameMessage( XGM_STATE_CHANGE, CX2Main::XS_SERVER_SELECT, NULL, false );
}
#endif ELSWORD_NEW_BEGINNING
