#include "StdAfx.h"
#include ".\X2SecurityPad.h"

#ifdef SERV_SECOND_SECURITY
CX2SecurityPad::CX2SecurityPad()
{
	m_pDLGSecurityPad	= NULL;
	m_pStaticMemo		= NULL;

	m_bShow				= false;
	m_eState			= SPS_CLOSE;
	m_iPhase			= -1;
	m_bUseSecondPW		= false;

	m_iPasswordIndex	= 0;
	for(int i=0; i<7; i++)
	{
		m_cPassword[i] = 0;
	}

	for(int i=0; i<12; i++)
	{
		m_iButtonSequence[i] = -1;
	}
}

CX2SecurityPad::~CX2SecurityPad()
{
	SAFE_DELETE_DIALOG(m_pDLGSecurityPad);
}

void CX2SecurityPad::SetShow(bool bShow, SECURITY_PAD_STATE eState /* = SPS_CLOSE */)
{
	m_eState = eState;
	m_bShow = bShow;

	if(m_bShow == false || m_eState == SPS_CLOSE)
	{
		m_iPhase = 0;
		ClearInputPassword();

		m_wstrPassword_Temp1.clear();
		m_wstrPassword_Temp2.clear();
		m_pDLGSecurityPad->SetShowEnable(false, false);
		
	}
	else
	{
		// Test
		if(m_pDLGSecurityPad != NULL)
		{
			SAFE_DELETE_DIALOG(m_pDLGSecurityPad);
		}
		
		if(m_pDLGSecurityPad == NULL)
		{
			m_pDLGSecurityPad = new CKTDGUIDialog( g_pMain->GetNowState(), L"DLG_UI_Security_Pad.lua", 0.07f, XDL_OPTION );	
			g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGSecurityPad );

			m_pStaticMemo = static_cast< CKTDGUIStatic* >(m_pDLGSecurityPad->GetControl( L"g_pStatic_Memo" ));
		}

		m_iPhase = 0;
		m_wstrPassword_Temp1.clear();
		m_wstrPassword_Temp2.clear();
		ClearInputPassword();
		ShuffleNumber();
		m_pDLGSecurityPad->SetShowEnable(true, true);
		CKTDGUIStatic* pStaticTip1 = static_cast< CKTDGUIStatic* >(m_pDLGSecurityPad->GetControl( L"g_pStatic_pass_tip1" ));
		CKTDGUIStatic* pStaticTip2 = static_cast< CKTDGUIStatic* >(m_pDLGSecurityPad->GetControl( L"g_pStatic_pass_tip2" ));


		switch(m_eState)
		{
		case SPS_GAME_START:
			{
				m_pStaticMemo->SetString(0, GET_STRING( STR_ID_12537 ));
				pStaticTip1->SetShow(true);
				pStaticTip2->SetShow(false);
			}break;
		case SPS_CREATE_PASSWORD:
			{
				m_pStaticMemo->SetString(0, GET_STRING( STR_ID_12537 ));
				m_pDLGSecurityPad->SetCloseCustomUIEventID( SPUM_EXIT );
				pStaticTip1->SetShow(false);
				pStaticTip2->SetShow(true);
			}break;
		case SPS_CHANGE_PASSWORD:
			{
				m_pStaticMemo->SetString(0, GET_STRING( STR_ID_12537 ));
				m_pDLGSecurityPad->SetCloseCustomUIEventID( SPUM_EXIT );
				pStaticTip1->SetShow(false);
				pStaticTip2->SetShow(true);
			}break;
		case SPS_RELEASE_PASSWORD:
			{
				m_pStaticMemo->SetString(0, GET_STRING( STR_ID_12537 ));
				m_pDLGSecurityPad->SetCloseCustomUIEventID( SPUM_EXIT );
				pStaticTip1->SetShow(false);
				pStaticTip2->SetShow(true);
			}break;
		default:
			{
				
			}break;
		}
	}
	return;
}

void CX2SecurityPad::ShuffleNumber()
{
	if( m_pDLGSecurityPad == NULL)
		return;

	for(int i=0; i<12; i++)
	{
		m_iButtonSequence[i] = -1;
	}
	bool bSequenceCheck[12] = {false,};
	srand( (unsigned) time( NULL ) );
	for(int i=0; i < 12; i++)
	{
		int iRandOffset = rand() % (12 - i);

		for(int j=0; j< 12; j++)
		{
			if(bSequenceCheck[j] == false)
			{
				if(iRandOffset > 0)
				{
					iRandOffset--;
				}
				else
				{
					bSequenceCheck[j] = true;
					m_iButtonSequence[i] = j;
					break;
				}
			}
		}
	}

	for(int i=0; i<12; i++)
	{
		CKTDGUIButton* pButtonDialNumber = static_cast< CKTDGUIButton* >( m_pDLGSecurityPad->GetControl(GET_REPLACED_STRING( ( STR_ID_3738, "Li", std::wstring( L"ButtonDialNumber" ), i ) ) )  );
		
#ifdef SECURITY_PAD_ADD_EXCEPTION_AND_BUG_FIX
		if( pButtonDialNumber == NULL )	// 임규수 일본 추가 예외 처리
			return;
#endif //SECURITY_PAD_ADD_EXCEPTION_AND_BUG_FIX			

		wstring wstrNormalTextrueKeyName = GET_REPLACED_STRING( ( STR_ID_3738, "Li", std::wstring( L"bt_normal_" ), m_iButtonSequence[i] ) );
		wstring wstrOverTextrueKeyName = GET_REPLACED_STRING( ( STR_ID_3738, "Li", std::wstring( L"bt_over_" ), m_iButtonSequence[i] ) ); 

		pButtonDialNumber->SetNormalTex(L"DLG_ui_button18.tga", wstrNormalTextrueKeyName.c_str() );
		pButtonDialNumber->SetOverTex(L"DLG_ui_button18.tga", wstrOverTextrueKeyName.c_str() );
		pButtonDialNumber->SetDownTex(L"DLG_ui_button18.tga", wstrOverTextrueKeyName.c_str() );

	}
	return;
}

HRESULT CX2SecurityPad::OnFrameMove(double fTime, float fElapsedTime)
{


	return S_OK;
}
bool CX2SecurityPad::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{   
	if(m_bShow == false)
		return false;

	return false;
}
void CX2SecurityPad::ClearInputPassword()
{
	m_iPasswordIndex = 0;
	for(int i=0; i<7; i++)
	{
		m_cPassword[i] = 0;
	}
	UpdateUIMarble();
	return;
}

void CX2SecurityPad::ProcessPassword()
{
	switch(m_eState)
	{
	case SPS_GAME_START: // Phase Count 1
		{
			m_wstrPassword_Temp1 = m_cPassword;

			// 입력이 없을 경우 무반응
			if(m_wstrPassword_Temp1 == L"")
				return;

			Handler_EGS_AUTH_SECOND_SECURITY_REQ();
		} break;
	case SPS_CREATE_PASSWORD: // Phase Count 2
		{
			if(m_iPhase == 0)
			{
				m_wstrPassword_Temp1 = m_cPassword;

				// 입력이 없을 경우 무반응
				if(m_wstrPassword_Temp1 == L"")
					return;

				m_iPhase = 1;
				ClearInputPassword();
				UpdateUIMarble();
				m_pStaticMemo->SetString(0, GET_STRING( STR_ID_12538 ));
				ShuffleNumber();
			}
			else if(m_iPhase == 1)
			{
				wstring wstrVerifyPassword = m_cPassword;
				
				// 입력이 없을 경우 무반응
				if(wstrVerifyPassword == L"")
					return;

				if(m_wstrPassword_Temp1 == wstrVerifyPassword)
				{
					Handler_EGS_CREATE_SECOND_SECURITY_REQ();
				}
				else
				{
					// 일치하지 않습니다.
					m_iPhase = 0;
					m_wstrPassword_Temp1.clear();
					m_wstrPassword_Temp2.clear();
					ClearInputPassword();
					UpdateUIMarble();
					m_pStaticMemo->SetString(0, L"");
					g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_12539 ), g_pMain->GetNowState() );
					SetShow(false);
				}
			}
		} break;
	case SPS_RELEASE_PASSWORD:  // Phase Count 1
		{
			m_wstrPassword_Temp1 = m_cPassword;

			// 입력이 없을 경우 무반응
			if(m_wstrPassword_Temp1 == L"")
				return;

			Handler_EGS_DELETE_SECOND_SECURITY_REQ();
		} break;
	case SPS_CHANGE_PASSWORD:  // Phase Count 3
		{
			if(m_iPhase == 0)
			{
				m_wstrPassword_Temp1 = m_cPassword;

				// 입력이 없을 경우 무반응
				if(m_wstrPassword_Temp1 == L"")
					return;

				Handler_EGS_COMPARE_SECOND_SECURITY_PW_REQ();
			}
			else if(m_iPhase == 1)
			{
				m_wstrPassword_Temp2 = m_cPassword;

				// 입력이 없을 경우 무반응
				if(m_wstrPassword_Temp2 == L"")
					return;

				m_iPhase = 2;
				ClearInputPassword();
				UpdateUIMarble();
				m_pStaticMemo->SetString(0, GET_STRING( STR_ID_12538 ));
				ShuffleNumber();
			}
			else if(m_iPhase == 2)
			{
				wstring wstrVerifyPassword = m_cPassword;
				
				// 입력이 없을 경우 무반응
				if(wstrVerifyPassword == L"")
					return;

				if(m_wstrPassword_Temp2 == wstrVerifyPassword)
				{
					Handler_EGS_CHANGE_SECOND_SECURITY_PW_REQ();
				}
				else
				{
					// 일치하지 않습니다.
					m_iPhase = 0;
					m_wstrPassword_Temp1.clear();
					m_wstrPassword_Temp2.clear();
					ClearInputPassword();
					UpdateUIMarble();
					m_pStaticMemo->SetString(0, L"");
					g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_12539 ), g_pMain->GetNowState() );
					SetShow(false);
				}
			}
		} break;
	}

	return;
}

bool CX2SecurityPad::UICustomEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{    

	switch ( wParam )
	{
	case SPUM_EXIT:
		{
			if(m_eState == SPS_GAME_START)
			{
				//CX2State::SUCM_EXIT
				g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_12541 ), CX2State::SUCM_EXIT, g_pMain->GetNowState());
			}
			else
			{
				SetShow( false );
			}
			return true;
		} break;
	case SPUM_OK:
		{
			ProcessPassword();
			UpdateUIMarble();
			return true;
		} break;
	case SPUM_CANCEL:
		{
#ifdef SECURITY_PAD_ADD_EXCEPTION_AND_BUG_FIX
			if( m_iPhase == 0 )		// 임규수 일본 추가
			m_wstrPassword_Temp1.clear();
			if( m_iPhase != 2 )		// 임규수 일본 추가
			m_wstrPassword_Temp2.clear();
#else //SECURITY_PAD_ADD_EXCEPTION_AND_BUG_FIX
			m_wstrPassword_Temp1.clear();
			m_wstrPassword_Temp2.clear();
#endif //SECURITY_PAD_ADD_EXCEPTION_AND_BUG_FIX			
			ClearInputPassword();
			UpdateUIMarble();
			return true;
		} break;
	case SPUM_DIAL_NUMBER_0:
		{
			if(m_iPasswordIndex < 6 && m_iButtonSequence[0] < 10)
			{
				m_cPassword[m_iPasswordIndex] = m_iButtonSequence[0] + 48;
				m_iPasswordIndex++;
				UpdateUIMarble();
			}
			return true;
		} break;
	case SPUM_DIAL_NUMBER_1:
		{
			if(m_iPasswordIndex < 6 && m_iButtonSequence[1] < 10)
			{
				m_cPassword[m_iPasswordIndex] = m_iButtonSequence[1] + 48;
				m_iPasswordIndex++;
				UpdateUIMarble();
			}
			return true;
		} break;
	case SPUM_DIAL_NUMBER_2:
		{
			if(m_iPasswordIndex < 6 && m_iButtonSequence[2] < 10)
			{
				m_cPassword[m_iPasswordIndex] = m_iButtonSequence[2] + 48;
				m_iPasswordIndex++;
				UpdateUIMarble();
			}
			return true;
		} break;
	case SPUM_DIAL_NUMBER_3:
		{
			if(m_iPasswordIndex < 6 && m_iButtonSequence[3] < 10)
			{
				m_cPassword[m_iPasswordIndex] = m_iButtonSequence[3] + 48;
				m_iPasswordIndex++;
				UpdateUIMarble();
			}
			return true;
		} break;
	case SPUM_DIAL_NUMBER_4:
		{
			if(m_iPasswordIndex < 6 && m_iButtonSequence[4] < 10)
			{
				m_cPassword[m_iPasswordIndex] = m_iButtonSequence[4] + 48;
				m_iPasswordIndex++;
				UpdateUIMarble();
			}
			return true;
		} break;
	case SPUM_DIAL_NUMBER_5:
		{
			if(m_iPasswordIndex < 6 && m_iButtonSequence[5] < 10)
			{
				m_cPassword[m_iPasswordIndex] = m_iButtonSequence[5] + 48;
				m_iPasswordIndex++;
				UpdateUIMarble();
			}
			return true;
		} break;
	case SPUM_DIAL_NUMBER_6:
		{
			if(m_iPasswordIndex < 6 && m_iButtonSequence[6] < 10)
			{
				m_cPassword[m_iPasswordIndex] = m_iButtonSequence[6] + 48;
				m_iPasswordIndex++;
				UpdateUIMarble();
			}
			return true;
		} break;
	case SPUM_DIAL_NUMBER_7:
		{
			if(m_iPasswordIndex < 6 && m_iButtonSequence[7] < 10)
			{
				m_cPassword[m_iPasswordIndex] = m_iButtonSequence[7] + 48;
				m_iPasswordIndex++;
				UpdateUIMarble();
			}
			return true;
		} break;
	case SPUM_DIAL_NUMBER_8:
		{
			if(m_iPasswordIndex < 6 && m_iButtonSequence[8] < 10)
			{
				m_cPassword[m_iPasswordIndex] = m_iButtonSequence[8] + 48;
				m_iPasswordIndex++;
				UpdateUIMarble();
			}
			return true;
		} break;
	case SPUM_DIAL_NUMBER_9:
		{
			if(m_iPasswordIndex < 6 && m_iButtonSequence[9] < 10)
			{
				m_cPassword[m_iPasswordIndex] = m_iButtonSequence[9] + 48;
				m_iPasswordIndex++;
				UpdateUIMarble();
			}
			return true;
		} break;
	case SPUM_DIAL_NUMBER_10:
		{
			if(m_iPasswordIndex < 6 && m_iButtonSequence[10] < 10)
			{
				m_cPassword[m_iPasswordIndex] = m_iButtonSequence[10] + 48;
				m_iPasswordIndex++;
				UpdateUIMarble();
			}
			return true;
		} break;
	case SPUM_DIAL_NUMBER_11:
		{
			if(m_iPasswordIndex < 6 && m_iButtonSequence[11] < 10)
			{
				m_cPassword[m_iPasswordIndex] = m_iButtonSequence[11] + 48;
				m_iPasswordIndex++;
				UpdateUIMarble();
			}
			return true;
		} break;
	case SPUM_DELETE_PASSWORD_VERIFY_OK:
		{
			CKTDGUIControl* pControl = NULL;
			pControl		= reinterpret_cast< CKTDGUIControl* >( lParam );
			g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, pControl->GetDialog(), NULL, false );
			Handler_EGS_DELETE_SECOND_SECURITY_VERIFY_REQ();
		} break;
	}
	return false;
}
void CX2SecurityPad::UpdateUIMarble()
{
	CKTDGUIStatic* pStaticSignal = static_cast< CKTDGUIStatic* >( m_pDLGSecurityPad->GetControl( L"StaticSignal" )  );

	for(int i=0; i<6; i++)
	{
		if(i< m_iPasswordIndex)
			pStaticSignal->GetPicture(i)->SetTex( L"DLG_ui_button08.tga", L"radio_over" );
		else
			pStaticSignal->GetPicture(i)->SetTex( L"DLG_ui_button08.tga", L"radio_normal" );
	}

	CKTDGUIButton* pButtonOk = static_cast< CKTDGUIButton* >( m_pDLGSecurityPad->GetControl( L"ButtonOK" )  );
	if(m_iPasswordIndex < 4)
	{
		pButtonOk->SetEnable( false );
	}
	else
	{
		pButtonOk->SetEnable( true );
	}
	return;
}

bool CX2SecurityPad::UIServerEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( wParam )
	{
	case EGS_AUTH_SECOND_SECURITY_ACK:
		{
			return Handler_EGS_AUTH_SECOND_SECURITY_ACK( hWnd, uMsg, wParam, lParam );
		}
		break;
	case EGS_CREATE_SECOND_SECURITY_ACK:
		{
			return Handler_EGS_CREATE_SECOND_SECURITY_ACK( hWnd, uMsg, wParam, lParam );
		}
		break;
	case EGS_DELETE_SECOND_SECURITY_ACK:
		{
			return Handler_EGS_DELETE_SECOND_SECURITY_ACK( hWnd, uMsg, wParam, lParam );
		}
		break;
	case EGS_DELETE_SECOND_SECURITY_VERIFY_ACK:
		{
			return Handler_EGS_DELETE_SECOND_SECURITY_VERIFY_ACK( hWnd, uMsg, wParam, lParam );
		}
		break;
	case EGS_CHANGE_SECOND_SECURITY_PW_ACK:
		{
			return Handler_EGS_CHANGE_SECOND_SECURITY_PW_ACK( hWnd, uMsg, wParam, lParam );
		}
		break;
	case EGS_COMPARE_SECOND_SECURITY_PW_ACK:
		{
			return Handler_EGS_COMPARE_SECOND_SECURITY_PW_ACK( hWnd, uMsg, wParam, lParam );
		}
#ifdef SERV_SECOND_SECURITY_K3R_AUTH
	case ECH_SECURITY_AUTH_ACK:
		{
			return Handler_ECH_SECURITY_AUTH_ACK( hWnd, uMsg, wParam, lParam );
		}
		break;
#endif SERV_SECOND_SECURITY_K3R_AUTH
		break;
	}

	return false;
}

#ifdef SERV_SECOND_SECURITY_K3R_AUTH
bool CX2SecurityPad::Handler_ECH_SECURITY_AUTH_REQ()
{
	// 여긴 미국용 2차 보안 패드 알림 코드임
	KECH_SECURITY_AUTH_REQ kPacket;
	kPacket.m_iSecurityAuthType = KECH_SECURITY_AUTH_REQ::SAT_CHK_SECOND_SECURITY;
	kPacket.m_iUserUID = g_pData->GetMyUser()->GetUID();
	kPacket.m_wstrUserID = g_pInstanceData->GetUserID();

	KServerGroupInfo kInfo = g_pMain->GetPickedChannelServer();

	if( g_pData->GetServerProtocol()->ConnectedToChannelServer( kInfo.m_kServerIP.c_str(), kInfo.m_usMasterPort ) == true )		
	{
		g_pData->GetServerProtocol()->SendChPacket( ECH_SECURITY_AUTH_REQ, kPacket );
		g_pMain->AddServerPacket( ECH_SECURITY_AUTH_ACK );
	}

	return true;
}

bool CX2SecurityPad::Handler_ECH_SECURITY_AUTH_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KECH_SECURITY_AUTH_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	if ( g_pMain->DeleteServerPacket( ECH_SECURITY_AUTH_ACK ) == true )
	{
		g_pData->GetServerProtocol()->DisconnectFromChannelServer();

		if ( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{	
			// 인증 성공
			g_pMain->GetSecurityPad()->SetShow( true, CX2SecurityPad::SPS_CREATE_PASSWORD );
			return true;
		}
		else // 한가지 경우 뿐이므로 일단 코드를 정하지 않습니다.
		{
			// 질문 설정용 홈페이지 띄워주기
			ShellExecuteA( NULL, "open", kEvent.m_strSecondSecurityURL.c_str(), NULL, NULL, SW_SHOW );
			return true;
		}
	}

	return false;
}
#endif //SERV_SECOND_SECURITY_K3R_AUTH


/////////////////////////////////////////////////////////////////
// EGS_AUTH_SECOND_SECURITY ///////////////////////////////////
bool CX2SecurityPad::Handler_EGS_AUTH_SECOND_SECURITY_REQ()  // Phase Count 1
{
	KEGS_AUTH_SECOND_SECURITY_REQ kPacket;

	kPacket.m_iUserUID = g_pData->GetMyUser()->GetUID();
	kPacket.m_wstrSecondPW = m_cPassword;

	
	//kPacket.

	g_pData->GetServerProtocol()->SendPacket( EGS_AUTH_SECOND_SECURITY_REQ, kPacket ); 
	g_pMain->AddServerPacket( EGS_AUTH_SECOND_SECURITY_ACK); 

	return true;
}
bool CX2SecurityPad::Handler_EGS_AUTH_SECOND_SECURITY_ACK(  HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_AUTH_SECOND_SECURITY_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	if ( g_pMain->DeleteServerPacket( EGS_AUTH_SECOND_SECURITY_ACK ) == true )
	{
		if( kEvent.m_iOK == NetError::ERR_SECOND_SECURITY_AUTH_FAILED)
		{
			g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_REPLACED_STRING( ( STR_ID_12351, "i", kEvent.m_iFailedCount ) ), g_pMain->GetNowState() );

			//창을 유지시킨채 계속 물어야한다.
			m_iPhase = 0;
			m_wstrPassword_Temp1.clear();
			m_wstrPassword_Temp2.clear();
			ClearInputPassword();
			UpdateUIMarble();
			m_pStaticMemo->SetString(0, GET_STRING( STR_ID_12537 ));
		}
		else if( kEvent.m_iOK == NetError::ERR_SECOND_SECURITY_AUTH_FAILED_LIMIT )
		{
			g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_12352 ), g_pMain->GetNowState(), CX2State::SUCM_EXIT );
		}
		else if ( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{	
			// 인증 성공
			SetShow(false);
			CX2StateServerSelect* pStateServerSelect = (CX2StateServerSelect*)g_pMain->GetNowState();
			pStateServerSelect->SetEnableUnitSelect(true);
#ifdef SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE
			CX2StateServerSelect* pState = static_cast<CX2StateServerSelect*>(g_pMain->GetNowState());
			pState->Handler_EGS_CHECK_THE_ARCHUANGEL_S_COIN_EVENT_LETTER_REQ();
#endif //SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE
			return true;
		}
	}
	return false;
}
/////////////////////////////////////////////////////////////////
// EGS_CREATE_SECOND_SECURITY ///////////////////////////////////
bool CX2SecurityPad::Handler_EGS_CREATE_SECOND_SECURITY_REQ()  // Phase Count 2
{
	KEGS_CREATE_SECOND_SECURITY_REQ kPacket;
	kPacket.m_wstrSecondPW = m_cPassword;

	g_pData->GetServerProtocol()->SendPacket( EGS_CREATE_SECOND_SECURITY_REQ, kPacket ); 
	g_pMain->AddServerPacket( EGS_CREATE_SECOND_SECURITY_ACK); 

	return true;
}
bool CX2SecurityPad::Handler_EGS_CREATE_SECOND_SECURITY_ACK(  HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_CREATE_SECOND_SECURITY_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	if ( g_pMain->DeleteServerPacket( EGS_CREATE_SECOND_SECURITY_ACK ) == true )
	{
		if ( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{	
			SetUseSecondPW(true);
			CX2StateOption* pStateOption = (CX2StateOption*)g_pMain->GetNowState();
			pStateOption->ShowSecurityOption(true);
			g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_12542 ), g_pMain->GetNowState() );
			SetShow(false);
			return true;
		}
		else
		{
			SetShow(false);
		}
	}
	return false;
}
/////////////////////////////////////////////////////////////////
// EGS_DELETE_SECOND_SECURITY ///////////////////////////////////
bool CX2SecurityPad::Handler_EGS_DELETE_SECOND_SECURITY_REQ()  // Phase Count 1
{
	KEGS_DELETE_SECOND_SECURITY_REQ kPacket;
	kPacket.m_wstrSecondPW = m_cPassword;
	m_wstrPassword_Delete = m_cPassword;

	g_pData->GetServerProtocol()->SendPacket( EGS_DELETE_SECOND_SECURITY_REQ, kPacket ); 
	g_pMain->AddServerPacket( EGS_DELETE_SECOND_SECURITY_ACK); 

	return true;
}

bool CX2SecurityPad::Handler_EGS_DELETE_SECOND_SECURITY_ACK(  HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_DELETE_SECOND_SECURITY_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	if ( g_pMain->DeleteServerPacket( EGS_DELETE_SECOND_SECURITY_ACK ) == true )
	{
		if ( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{	
			g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_12543 ), SPUM_DELETE_PASSWORD_VERIFY_OK, g_pMain->GetNowState());
			return true;
		}
		else
		{
			SetShow(false);
		}
	}
	return false;
}
bool CX2SecurityPad::Handler_EGS_DELETE_SECOND_SECURITY_VERIFY_REQ()  // Phase Count 1
{
	KEGS_DELETE_SECOND_SECURITY_VERIFY_REQ kPacket;
	kPacket.m_bDelete = true;
	kPacket.m_wstrSecondPW = m_wstrPassword_Delete;
	g_pData->GetServerProtocol()->SendPacket( EGS_DELETE_SECOND_SECURITY_VERIFY_REQ, kPacket ); 
	g_pMain->AddServerPacket( EGS_DELETE_SECOND_SECURITY_VERIFY_ACK); 

	return true;
}

bool CX2SecurityPad::Handler_EGS_DELETE_SECOND_SECURITY_VERIFY_ACK(  HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_DELETE_SECOND_SECURITY_VERIFY_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	if ( g_pMain->DeleteServerPacket( EGS_DELETE_SECOND_SECURITY_VERIFY_ACK ) == true )
	{
		if ( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{	
			SetUseSecondPW(false);
			CX2StateOption* pStateOption = (CX2StateOption*)g_pMain->GetNowState();
			pStateOption->ShowSecurityOption(true);
			g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_12544 ), g_pMain->GetNowState() );
			SetShow(false);
			return true;
		}
		else
		{
			SetShow(false);
		}
	}
	return false;
}
/////////////////////////////////////////////////////////////////
// EGS_CHANGE_SECOND_SECURITY_PW_VERIFY //////////////////////////////// 
bool CX2SecurityPad::Handler_EGS_COMPARE_SECOND_SECURITY_PW_REQ() // Phase Count 3
{
	KEGS_COMPARE_SECOND_SECURITY_PW_REQ kPacket;

	kPacket = m_wstrPassword_Temp1;

	g_pData->GetServerProtocol()->SendPacket( EGS_COMPARE_SECOND_SECURITY_PW_REQ, kPacket ); 
	g_pMain->AddServerPacket( EGS_COMPARE_SECOND_SECURITY_PW_ACK); 

	return true;
}

bool CX2SecurityPad::Handler_EGS_COMPARE_SECOND_SECURITY_PW_ACK(  HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_COMPARE_SECOND_SECURITY_PW_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	if ( g_pMain->DeleteServerPacket( EGS_COMPARE_SECOND_SECURITY_PW_ACK ) == true )
	{
		if ( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{	
			m_iPhase = 1;
			ClearInputPassword();
			UpdateUIMarble();
			m_pStaticMemo->SetString(0, GET_STRING( STR_ID_12540 ));
			ShuffleNumber();
		}
		else
		{
			//g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_12545 ), g_pMain->GetNowState() );
			SetShow(false);
			return true;
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////
// EGS_CHANGE_SECOND_SECURITY_PW //////////////////////////////// 
bool CX2SecurityPad::Handler_EGS_CHANGE_SECOND_SECURITY_PW_REQ() // Phase Count 3
{
	KEGS_CHANGE_SECOND_SECURITY_PW_REQ kPacket;

	kPacket.m_wstrOldSecondPW = m_wstrPassword_Temp1;
	kPacket.m_wstrNewSecondPW = m_wstrPassword_Temp2;

	g_pData->GetServerProtocol()->SendPacket( EGS_CHANGE_SECOND_SECURITY_PW_REQ, kPacket ); 
	g_pMain->AddServerPacket( EGS_CHANGE_SECOND_SECURITY_PW_ACK); 

	return true;
}

bool CX2SecurityPad::Handler_EGS_CHANGE_SECOND_SECURITY_PW_ACK(  HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_CHANGE_SECOND_SECURITY_PW_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	if ( g_pMain->DeleteServerPacket( EGS_CHANGE_SECOND_SECURITY_PW_ACK ) == true )
	{
		if ( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{	
			g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_12545 ), g_pMain->GetNowState() );
			SetShow(false);
			return true;
		}
		else
		{
			SetShow(false);
			return true;
		}
	}
	return false;
}
#endif SERV_SECOND_SECURITY