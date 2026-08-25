#include "StdAfx.h"

#ifdef SERV_SKILL_NOTE

#include "X2UISkillNote.h"

CX2UISkillNote::CX2UISkillNote( CKTDXStage* pNowStage, const WCHAR* pFileName ) :
m_iRegisterPage(0),
m_iCurrentPage(0),
m_iUpdatePage(0),
m_bShow(false),
m_bRegisterMemo(false),
m_bUpdateMemo(false),
m_iRegisterMemo(0),
m_pDLGSkillNote(NULL),
m_pDLGWrittingNote(NULL),
m_pDLGQuery(NULL),
m_fWriteMemoTime(0.f),
m_iViewPage(0)
{
	m_pDLGSkillNote = new CKTDGUIDialog( g_pMain->GetNowState(), pFileName );
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGSkillNote );
	m_pDLGSkillNote->SetDisableUnderWindow(true);
	m_pDLGSkillNote->SetFront(true);
	m_pDLGSkillNote->SetDisableUnderWindow(true);
	m_pDLGSkillNote->SetShowEnable(false, false);


	CKTDGUIButton *pButton = (CKTDGUIButton*)m_pDLGSkillNote->GetControl(L"_Prev_button");
	if( pButton ) pButton->SetEnable(false);
	pButton = (CKTDGUIButton*)m_pDLGSkillNote->GetControl(L"_Next_button");
	if( pButton ) pButton->SetEnable(false);


	m_hParticle1 = INVALID_PARTICLE_HANDLE;	
	m_hParticle2 = INVALID_PARTICLE_HANDLE;	

#ifdef COUNTRY_LV_STRING
	switch(g_pMain->GetNationFlag())
	{
		// 유럽
	case CX2Main::NF_FR:	
		m_vecSlotLv.push_back(L"Nv.20");
		m_vecSlotLv.push_back(L"Nv.30");
		m_vecSlotLv.push_back(L"Nv.40");
		m_vecSlotLv.push_back(L"Nv.50");
		m_vecSlotLv.push_back(L"Nv.60");
		m_vecSlotLv.push_back(L"Nv.??");
		break;

	case CX2Main::NF_PL:	
		m_vecSlotLv.push_back(L"Pz.20");
		m_vecSlotLv.push_back(L"Pz.30");
		m_vecSlotLv.push_back(L"Pz.40");
		m_vecSlotLv.push_back(L"Pz.50");
		m_vecSlotLv.push_back(L"Pz.60");
		m_vecSlotLv.push_back(L"Pz.??");
		break;

	case CX2Main::NF_ES:
		m_vecSlotLv.push_back(L"Niv.20");
		m_vecSlotLv.push_back(L"Niv.30");
		m_vecSlotLv.push_back(L"Niv.40");
		m_vecSlotLv.push_back(L"Niv.50");
		m_vecSlotLv.push_back(L"Niv.60");
		m_vecSlotLv.push_back(L"Niv.??");
		break;

	default:
		// 각 슬롯 스트링 설정
		m_vecSlotLv.push_back(L"Lv.20");
		m_vecSlotLv.push_back(L"Lv.30");
		m_vecSlotLv.push_back(L"Lv.40");
		m_vecSlotLv.push_back(L"Lv.50");
		m_vecSlotLv.push_back(L"Lv.60");
		m_vecSlotLv.push_back(L"Lv.??");
		break;
	}
#else //COUNTRY_LV_STRING
	// 각 슬롯 스트링 설정
    m_vecSlotLv.push_back(L"Lv.20");
	m_vecSlotLv.push_back(L"Lv.30");
	m_vecSlotLv.push_back(L"Lv.40");
	m_vecSlotLv.push_back(L"Lv.50");
	m_vecSlotLv.push_back(L"Lv.60");
	m_vecSlotLv.push_back(L"Lv.??");
#endif //COUNTRY_LV_STRING


}
CX2UISkillNote::~CX2UISkillNote(void)
{
	SAFE_DELETE_DIALOG( m_pDLGSkillNote );
	SAFE_DELETE_DIALOG( m_pDLGWrittingNote );
}
 
HRESULT CX2UISkillNote::OnFrameMove( double fTime, float fElapsedTime )
{

	KTDXPROFILE();	

	if( GetShow() == false )
	{
		m_bUpdateMemo = false;
		return S_OK;
	}

	CX2State* pState = (CX2State*)g_pMain->GetNowState();
	if( pState == NULL )
	{
		if( m_bUpdateMemo == true )
		{
			SAFE_DELETE_DIALOG( m_pDLGWrittingNote );
			m_pDLGWrittingNote = NULL;
			ResetSkillNote();
		}
		
		SetShow(false);
		return S_OK;
	}

	if( pState->GetCursor() != NULL &&
		pState->GetCursor()->GetCurorState() == CX2Cursor::XCS_REGISTER_MEMO )
	{	
		m_pDLGInfoBox = NULL;

		if( m_pDLGQuery != NULL )
		{
			if( g_pMain->GetInfoBox() != NULL )
				g_pMain->DeleteInfoBox();
		}
		else
		{
			if( g_pMain->GetInfoBox() == NULL )
				g_pMain->KTDGUIInfoBox( D3DXVECTOR2( 250, 200 ), GET_STRING( STR_ID_5009), g_pMain->GetNowState() );
		}
	}
	else
	{
		if( m_bUpdateMemo == false )
		{
			if( g_pMain->GetInfoBox() != NULL && m_pDLGInfoBox == NULL )
				g_pMain->DeleteInfoBox();
		}
		else
		{
			if( m_fWriteMemoTime > 0.f )
			{
				m_fWriteMemoTime -= fElapsedTime;
				
				float fProgressNote = 1.f;
				fProgressNote -= (m_fWriteMemoTime / 2.f);

				if( m_pDLGWrittingNote != NULL )
				{
					CKTDGUIStatic *pStatic = (CKTDGUIStatic*)m_pDLGWrittingNote->GetControl(L"g_pStaticNote_Loading");
					CKTDGUIControl::CPictureData *pPicture = pStatic->GetPictureIndex(1);
					float fProgressRate = pPicture->GetOriginalSize().x * fProgressNote;
					pPicture->SetSizeX( fProgressRate );
				}

				if( m_fWriteMemoTime <= 0.f && m_pDLGInfoBox == NULL )
				{
					SAFE_DELETE_DIALOG( m_pDLGWrittingNote );
					m_pDLGWrittingNote = NULL;

					m_iViewPage = 0;
					UpdateMemoSlot((int)m_iUpdatePage);
					UpdateMemoEffect();
					m_pDLGInfoBox = g_pMain->KTDGUIInfoBox( D3DXVECTOR2( 250, 200 ), GET_STRING( STR_ID_5010), g_pMain->GetNowState(), 3.f);
					m_fWriteMemoTime = 0.f;					
					m_bUpdateMemo = false;
					m_iUpdatePage = 0;

					g_pKTDXApp->GetDeviceManager()->PlaySound( L"TechNote_CompleteWriting.ogg", false, false );
				}
			}
		}
	}

	return S_OK;
}

bool CX2UISkillNote::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// 앞에 모달 다이얼로그가 있으면 메시지를 처리하지 않는다
	if( true == g_pKTDXApp->GetDGManager()->GetDialogManager()->CheckFrontModalDlg( m_pDLGSkillNote ) )
		return false;

	bool bFlag = false;
	switch( uMsg )
	{
		// 마우스 메시지들에 대해
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	//case WM_MOUSEWHEEL:
		{
			if ( false == m_pDLGSkillNote->GetIsMouseOver())
			{
				return false;
			}
			
			bFlag = true;
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		{
			if ( false == m_pDLGSkillNote->GetIsMouseOver())
			{
				return false;
			}
			g_pData->GetUIManager()->SetUILayerIndex(CX2UIManager::UI_SKILL_NOTE, true);
			bFlag = true;			
		}
		break;
	default:
		break;
	}
	return bFlag;
}

bool CX2UISkillNote::UICustomEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch(wParam)
	{
	case USNM_EXIT:
		{
			SetShow(false);
			return true;
		}
		break;
	case USNM_REGISTER_VIEW_SLOT:
		{
			CKTDGUIButton* pButton = (CKTDGUIButton*) lParam;							// fix!! 라디오 버튼으로 변경 해야함 
			m_iRegisterPage = (char)pButton->GetDummyInt(0);

			CX2State* pState = (CX2State*)g_pMain->GetNowState();
			if( pState != NULL && 
				pState->GetCursor() != NULL &&
				pState->GetCursor()->GetCurorState() != CX2Cursor::XCS_REGISTER_MEMO )
			{
				m_bRegisterMemo = false;
				m_iRegisterMemo = 0;				
			}

			if( m_bRegisterMemo == true )
			{
				// 메모등록	확인
				CheckRegisterMemo();
			}
			else
			{
				// 메모뷰
				ViewMemo( (int)(m_iRegisterPage-1) );
			}

			return true;
		}
		break;
	case USNM_SLOT_PAGE_LEFT:
		{
			if( m_iCurrentPage > 0)
				--m_iCurrentPage;
			return true;
		}
		break;
	case USNM_SLOT_PAGE_RIGHT:
		{
			++m_iCurrentPage;
			return true;
		}
		break;
	case USNM_REGISTER_OK:
		{
			//메모등록
			g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGQuery, NULL, false );
			m_pDLGQuery = NULL;

			Handler_EGS_REG_SKILL_NOTE_MEMO_REQ( m_iRegisterPage );

			CX2State* pState = (CX2State*)g_pMain->GetNowState();
			if( pState != NULL && pState->GetCursor() != NULL )
				pState->GetCursor()->ChangeCursorState( CX2Cursor::XCS_NORMAL );
			m_bRegisterMemo = false;
			m_iRegisterMemo = 0;
			m_iRegisterPage = 0;
			return true;
		}
		break;
	case USNM_REGISTER_CANCEL:
		{
			g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGQuery, NULL, false );
			m_pDLGQuery = NULL;
		}
		break;
	case USNM_PAGE_PREV:
		{
			ViewMemo( m_iViewPage - 3 );
		}
		break;
	case USNM_PAGE_NEXT:
		{
			ViewMemo( m_iViewPage + 1 );
		}
		break;
	default:
		break;
	}

	return false;
}

bool CX2UISkillNote::UIServerEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	
	return false;
}

void CX2UISkillNote::SetShow(bool val)
{
	m_iViewPage = 0;

	if( val )
	{
		g_pData->GetUIManager()->CloseAllLUDlg();
		g_pData->GetUIManager()->CloseAllNPCDlg();
		g_pData->GetUIManager()->UIOpened(CX2UIManager::UI_SKILL_NOTE);
		
		if( m_bShow == false )
		{			
			m_iCurrentPage = 0;
			UpdateMemoSlot();
			ViewMemo(0);
		}

		m_bShow = val;
		m_pDLGSkillNote->SetShowEnable(val, val);
	}
	else
	{
		m_bShow = val;
		m_pDLGSkillNote->SetShowEnable(val, val);

		g_pData->GetUIManager()->UIClosed(CX2UIManager::UI_SKILL_NOTE);
		m_iRegisterMemo = 0;
		m_bRegisterMemo = false;


		CX2State* pState = (CX2State*)g_pMain->GetNowState();
		if( pState != NULL )
			pState->GetCursor()->ChangeCursorState( CX2Cursor::XCS_NORMAL );

		ResetSkillNote();		
	}	
}



bool CX2UISkillNote::OpenDialog()
{
	m_bShow = true;
	m_pDLGSkillNote->SetShowEnable(true, true);

	// 나 열렸어요~
	g_pData->GetUIManager()->UIOpened(CX2UIManager::UI_SKILL_NOTE);
	return true;

}

void CX2UISkillNote::SetLayer(X2_DIALOG_LAYER layer)
{	
	if(m_pDLGSkillNote != NULL)
	{
		g_pKTDXApp->GetDGManager()->GetDialogManager()->ChangeLayer(m_pDLGSkillNote, layer);
		g_pKTDXApp->GetDGManager()->GetDialogManager()->ChangeSequence( m_pDLGSkillNote, false );
	}
	
	// 유닛뷰어는 다이얼로그랑 같이 그려진다.

}

void CX2UISkillNote::RegisterMemo(UidType iMemoId)
{
	m_bRegisterMemo = true;
	m_iRegisterMemo = iMemoId;
}

void CX2UISkillNote::Handler_EGS_REG_SKILL_NOTE_MEMO_REQ(char iPage)
{
	if( iPage <= 0 )
		return;

	CX2State* pState = (CX2State*)g_pMain->GetNowState();

	if( pState != NULL && 
		pState->GetCursor() != NULL &&
		pState->GetCursor()->GetCurorState() == CX2Cursor::XCS_REGISTER_MEMO &&
		m_bRegisterMemo == true && m_iRegisterMemo > 0 )
	{
		KEGS_REG_SKILL_NOTE_MEMO_REQ kEGS_REG_SKILL_NOTE_MEMO_REQ;

		kEGS_REG_SKILL_NOTE_MEMO_REQ.m_cSkillNotePageNum = iPage - 1;
		kEGS_REG_SKILL_NOTE_MEMO_REQ.m_iItemUID = m_iRegisterMemo;

		g_pData->GetServerProtocol()->SendPacket( EGS_REG_SKILL_NOTE_MEMO_REQ, kEGS_REG_SKILL_NOTE_MEMO_REQ );
		g_pMain->AddServerPacket( EGS_REG_SKILL_NOTE_MEMO_ACK );
	}
}



void CX2UISkillNote::UpdateMemoSlot(int iPage)
{
	if( m_pDLGSkillNote == NULL )
		return;

	char cMaxPage = g_pData->GetMyUser()->GetSelectUnit()->GetMaxSkillNoteSlot();
	CKTDGUIStatic *pLvStatic = (CKTDGUIStatic*)m_pDLGSkillNote->GetControl(L"Static_LV");
	CKTDGUIStatic *pPageStatic = (CKTDGUIStatic*)m_pDLGSkillNote->GetControl(L"Static_Slot_Page");
	CKTDGUIStatic *pNoteStatic = (CKTDGUIStatic*)m_pDLGSkillNote->GetControl(L"g_pStaticic_Note_of_Skill");

	
	for(int i=0; i<6; ++i)
	{
		WCHAR wcControlName[20] = {0,};

		//wsprintf(wcControlName, L"_Icon_button%d", i);
		StringCchPrintf( wcControlName, 20, L"_Icon_button%d", i);
		CKTDGUIButton *pButton = (CKTDGUIButton*)m_pDLGSkillNote->GetControl(wcControlName);
		if( pButton ) pButton->SetEnable(false);
	
		if( pLvStatic )
			pLvStatic->GetString(i)->msg = m_vecSlotLv[i + (int)m_iCurrentPage];

		if( pPageStatic )
		{
		#ifdef CLIENT_COUNTRY_EU
			// 2011.08.03 임홍락 페이지를 나타내는 문자열을 스트링 테이블로 빼냄
			WCHAR wcPageNum[2];
			wstring wstrPage;
			_itow(i+(int)m_iCurrentPage+1, wcPageNum, 10);
			wstrPage = GET_STRING(STR_ID_13663);
			wstrPage += L" ";
			wstrPage += wcPageNum;
			
			pPageStatic->GetString(i)->msg = wstrPage;
		#else //CLIENT_COUNTRY_EU
			//wsprintf( wcControlName, L"%d Page", i+(int)m_iCurrentPage+1);
			StringCchPrintf( wcControlName, 20, L"%d Page", i+(int)m_iCurrentPage+1);
			pPageStatic->GetString(i)->msg = wcControlName;
		#endif //CLIENT_COUNTRY_EU
		
		}

		if( pNoteStatic )
		{
			CKTDGUIControl::CPictureData *pPicture =  pNoteStatic->GetPictureIndex(i+1);
			pPicture->SetTex(L"DLG_UI_Common_Texture29.TGA", L"Lock");
			pPicture->SetShow(true);
		}
	}
	

	// open된 각 슬롯 검사하여 메모등록된 슬롯인지 빈 슬롯인지 검사	
	for(char cSlotId = 0; cSlotId < 6 && cSlotId < cMaxPage; ++cSlotId)
	{
		if( cSlotId < m_iCurrentPage )
			continue;		

		int iMemoId = g_pData->GetMyUser()->GetSelectUnit()->GetSkillMemoIdBySlot( cSlotId );

		if( iMemoId < 0 )
		{
			// 빈 슬롯
			if( pNoteStatic )
			{
				CKTDGUIControl::CPictureData *pPicture =  pNoteStatic->GetPictureIndex(cSlotId+1);
				pPicture->SetShow(false);
			}
		}
		else
		{
			// 등록된 슬롯
			if( pLvStatic )
			{
				pLvStatic->GetString((int)cSlotId)->msg = L"";
			}
			if( pNoteStatic )
			{				
				CKTDGUIControl::CPictureData *pPicture =  pNoteStatic->GetPictureIndex(cSlotId+1);
				pPicture->SetTex(
                    g_pData->GetItemManager()->GetItemTemplet(iMemoId)->GetShopImage()
                    );				
				pPicture->SetShow(true);
			}
		}

		WCHAR wcControlName[20] = {0,};
		//wsprintf(wcControlName, L"_Icon_button%d", (int)cSlotId);
		StringCchPrintf( wcControlName, 20, L"_Icon_button%d", (int)cSlotId);
		CKTDGUIButton *pButton = (CKTDGUIButton*)m_pDLGSkillNote->GetControl(wcControlName);
		if( pButton ) 
		{	
			pButton->SetEnable(true);
			if( iMemoId > 0 )
			{
                const CX2Item::ItemTemplet* pkTemplet = g_pData->GetItemManager()->GetItemTemplet(iMemoId);
                wstring wstrMemoDesc = pkTemplet->GetName();
				wstrMemoDesc += L"\n\n";
#ifdef CLIENT_GLOBAL_LINEBREAK
				wstrMemoDesc += CWordLineHandler::GetStrByLineBreakInX2Main( g_pData->GetItemManager()->GetItemTemplet(iMemoId)->GetDescription(), 330, 1);
#else //CLIENT_GLOBAL_LINEBREAK
				wstrMemoDesc += pkTemplet->GetDescription();
#endif //CLIENT_GLOBAL_LINEBREAK
				pButton->SetGuideDesc( wstrMemoDesc.c_str() );
			}
			else
				pButton->SetGuideDesc(L"");
		}
	}

	ViewMemo(iPage);
}

void CX2UISkillNote::UpdateMemo(bool bVal, char cPage)
{ 
	m_bUpdateMemo = bVal; 
	m_iUpdatePage = cPage;
	m_fWriteMemoTime = 2.f;

	if( m_bUpdateMemo == true )
	{
		SAFE_DELETE_DIALOG( m_pDLGWrittingNote );
		m_pDLGWrittingNote = NULL;
		// 노트 쓰기 씬
		m_pDLGWrittingNote = new CKTDGUIDialog( g_pMain->GetNowState(), L"DLG_Note_Loading.lua" );
		g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGWrittingNote );
		m_pDLGSkillNote->SetFront(true);		

		g_pKTDXApp->GetDeviceManager()->PlaySound( L"TechNote_Writing.ogg", false, false );
	}
}

void CX2UISkillNote::ViewMemo(int iPage)
{
	char cMaxPage = g_pData->GetMyUser()->GetSelectUnit()->GetMaxSkillNoteSlot();

	if( g_pData == NULL )
		return;	

	if( m_pDLGSkillNote == NULL )
		return;

	if( iPage % 2 != 0 )
		iPage -= 1;

	if( m_iViewPage == iPage+1 )
		return;

	if( iPage < 0 || iPage >= cMaxPage )	
		return;		

	m_iViewPage = iPage+1;

	g_pKTDXApp->GetDeviceManager()->PlaySound( L"TechNote_FlipBook.ogg", false, false );

	WCHAR wcBuf[10] = {0,};
	int itemId1 = g_pData->GetMyUser()->GetSelectUnit()->GetSkillMemoIdBySlot(iPage);
	int itemId2 = g_pData->GetMyUser()->GetSelectUnit()->GetSkillMemoIdBySlot(iPage+1);
    const CX2Item::ItemTemplet* pkTemplet1 = g_pData->GetItemManager()->GetItemTemplet(itemId1);
    const CX2Item::ItemTemplet* pkTemplet2 = g_pData->GetItemManager()->GetItemTemplet(itemId2);

#ifdef SKILL_NOTE_SCROLLBAR
	CKTDGUIListBox* pListBox1 = (CKTDGUIListBox*)m_pDLGSkillNote->GetControl(L"MemoDesc1");
	CKTDGUIListBox* pListBox2 = (CKTDGUIListBox*)m_pDLGSkillNote->GetControl(L"MemoDesc2");
	if (pListBox1 != NULL)
		pListBox1->RemoveAllItems();
	if (pListBox2 != NULL)
		pListBox2->RemoveAllItems();
#endif //SKILL_NOTE_SCROLLBAR

	// 페이지
	CKTDGUIStatic *pPageStatic = (CKTDGUIStatic*)m_pDLGSkillNote->GetControl(L"Static_Note_Page");
	if( pPageStatic != NULL )
	{
	#ifdef CLIENT_COUNTRY_EU
			// 2011.08.03 임홍락 페이지를 나타내는 문자열을 스트링 테이블로 빼냄
		WCHAR wcPageNum[2];
		wstring wstrPage;

		_itow(iPage + 1, wcPageNum, 10);
		wstrPage = GET_STRING(STR_ID_13663);
		wstrPage += L" ";
		wstrPage += wcPageNum;
				
		//wsprintf(wcBuf, L"%d Page", iPage + 1);
		//StringCchPrintf( wcBuf, 10, L"%d Page", iPage + 1);
		pPageStatic->GetString(0)->msg = wstrPage;

		wstrPage.clear();

		_itow(iPage + 2, wcPageNum, 10);
		wstrPage = GET_STRING(STR_ID_13663);
		wstrPage += L" ";
		wstrPage += wcPageNum;
				
		//wsprintf(wcBuf, L"%d Page", iPage + 2);
		//StringCchPrintf( wcBuf, 10, L"%d Page", iPage + 2);
		pPageStatic->GetString(1)->msg = wstrPage;
	#else
		//wsprintf(wcBuf, L"%d Page", iPage + 1);
		StringCchPrintf( wcBuf, 10, L"%d Page", iPage + 1);
		pPageStatic->GetString(0)->msg = wcBuf;
		//wsprintf(wcBuf, L"%d Page", iPage + 2);
		StringCchPrintf( wcBuf, 10, L"%d Page", iPage + 2);
		pPageStatic->GetString(1)->msg = wcBuf;
	#endif
	}

	// 타이틀
	CKTDGUIStatic *pTitleStatic = NULL;	
	pTitleStatic = (CKTDGUIStatic*)m_pDLGSkillNote->GetControl(L"Static_Title_A");
	if( pTitleStatic != NULL )
	{
		if( itemId1 > 0 && pkTemplet1 != NULL )
		{	
#ifdef CLIENT_GLOBAL_LINEBREAK
			wstring wstrTitleA = L"";
			wstrTitleA = CWordLineHandler::GetStrByLineBreakColorInX2Main( pkTemplet1->GetName(), 190, 1 );
			pTitleStatic->GetString(0)->msg = wstrTitleA;
#else //CLIENT_GLOBAL_LINEBREAK
			pTitleStatic->GetString(0)->msg = pkTemplet1->GetName();
#endif //CLIENT_GLOBAL_LINEBREAK	
		}
		else
		{
			pTitleStatic->GetString(0)->msg = L"";
		}

	}	
	pTitleStatic = (CKTDGUIStatic*)m_pDLGSkillNote->GetControl(L"g_pStatic_Title_B");
	if( pTitleStatic != NULL )
	{
		if( itemId2 > 0 && pkTemplet2 != NULL )
		{
#ifdef CLIENT_GLOBAL_LINEBREAK
			wstring wstrTitleB = L"";
			wstrTitleB = CWordLineHandler::GetStrByLineBreakColorInX2Main( pkTemplet2->GetName(), 190, 1 );
			pTitleStatic->GetString(0)->msg = wstrTitleB;
#else //CLIENT_GLOBAL_LINEBREAK
			pTitleStatic->GetString(0)->msg = pkTemplet2->GetName();
#endif //CLIENT_GLOBAL_LINEBREAK	
		}
		else
		{
			pTitleStatic->GetString(0)->msg = L"";
		}
	}

	// 본문
	CKTDGUIStatic *pMemoStatic = NULL;
	pMemoStatic = (CKTDGUIStatic*)m_pDLGSkillNote->GetControl(L"g_pStatic_Note_A");
	if( pMemoStatic != NULL )
	{
		wstring wstrItemDesc = L"";		

		if( itemId1 > 0 && pkTemplet1 != NULL )
		{			
			wstrItemDesc += L"#C0185EB";
#ifdef FIX_TOOLTIP
			wstrItemDesc += 
                pkTemplet1->GetDescription();
#ifdef CLIENT_GLOBAL_LINEBREAK
			wstrItemDesc = CWordLineHandler::GetStrByLineBreakColorInX2Main( wstrItemDesc.c_str(), 305, 1 );
#else //CLIENT_GLOBAL_LINEBREAK
			wstrItemDesc = g_pMain->GetStrByLineBreakColor( wstrItemDesc.c_str(), 300, 1 );
#endif //CLIENT_GLOBAL_LINEBREAK
#else
			wstrItemDesc += ReplaceDesc_( 
                pkTemplet1->GetDescription(), 
                L"\n", L"\n#C0185EB" );
#endif FIX_TOOLTIP
			wstrItemDesc += L"#CX\n\n";

#ifdef FIX_TOOLTIP
			wstring wstrMemoDesc = 
                pkTemplet1->GetDescriptionInSkillNote();
#ifdef CLIENT_GLOBAL_LINEBREAK
			wstrMemoDesc = CWordLineHandler::GetStrByLineBreakColorInX2Main( wstrMemoDesc.c_str(), 305, 1 );
#else //CLIENT_GLOBAL_LINEBREAK
			wstrMemoDesc = g_pMain->GetStrByLineBreakColor( wstrMemoDesc.c_str(), 300, 1 );
#endif //CLIENT_GLOBAL_LINEBREAK
			wstrItemDesc += wstrMemoDesc;
#else
			wstrItemDesc += 
                pkTemplet1->GetDescriptionInSkillNote()
#endif
		}

		pMemoStatic->GetString(0)->msg = wstrItemDesc;

#ifdef SKILL_NOTE_SCROLLBAR
		pMemoStatic->SetShow(false);

		wstring::size_type lastPosCond = wstrItemDesc.find_first_not_of(L"\n", 0);
		wstring::size_type posCond     = wstrItemDesc.find_first_of(L"\n", lastPosCond);

		while (wstring::npos != posCond || wstring::npos != lastPosCond)
		{        
			wstring tempStr = wstrItemDesc.substr(lastPosCond, posCond - lastPosCond);
			pListBox1->AddItem(tempStr.c_str(), NULL);

			lastPosCond = wstrItemDesc.find_first_not_of(L"\n", posCond);
			posCond = wstrItemDesc.find_first_of(L"\n", lastPosCond);
		}
#endif // SKILL_NOTE_SCROLLBAR
		
	}

	pMemoStatic = (CKTDGUIStatic*)m_pDLGSkillNote->GetControl(L"g_pStatic_Note_b");
	if( pMemoStatic != NULL )
	{
		wstring wstrItemDesc = L"";		

		if( itemId2 > 0 && pkTemplet2 != NULL )
		{
			wstrItemDesc += L"#C0185EB";
#ifdef FIX_TOOLTIP
			wstrItemDesc += 
                pkTemplet2->GetDescription();
#ifdef CLIENT_GLOBAL_LINEBREAK
			wstrItemDesc = CWordLineHandler::GetStrByLineBreakColorInX2Main( wstrItemDesc.c_str(), 305, 1 );
#else //CLIENT_GLOBAL_LINEBREAK
			wstrItemDesc = g_pMain->GetStrByLineBreakColor( wstrItemDesc.c_str(), 300, 1 );
#endif //CLIENT_GLOBAL_LINEBREAK
#else
			wstrItemDesc += ReplaceDesc_( 
                pkTemplet2->GetDescription(),
                L"\n", L"\n#C0185EB" );
#endif FIX_TOOLTIP
			wstrItemDesc += L"#CX\n\n";

#ifdef FIX_TOOLTIP
			wstring wstrMemoDesc = 
                pkTemplet2->GetDescriptionInSkillNote();
#ifdef CLIENT_GLOBAL_LINEBREAK
			wstrMemoDesc = CWordLineHandler::GetStrByLineBreakColorInX2Main(wstrMemoDesc.c_str(), 305, 1 );
#else //CLIENT_GLOBAL_LINEBREAK
			wstrMemoDesc = g_pMain->GetStrByLineBreakColor( wstrMemoDesc.c_str(), 300, 1 );
#endif //CLIENT_GLOBAL_LINEBREAK
			wstrItemDesc += wstrMemoDesc;
#else
			wstrItemDesc += 
                pkTemplet2->GetDescriptionInSkillNote();
#endif
		}

		pMemoStatic->GetString(0)->msg = wstrItemDesc;


#ifdef SKILL_NOTE_SCROLLBAR
		pMemoStatic->SetShow(false);

		wstring::size_type lastPosCond = wstrItemDesc.find_first_not_of(L"\n", 0);
		wstring::size_type posCond     = wstrItemDesc.find_first_of(L"\n", lastPosCond);

		while (wstring::npos != posCond || wstring::npos != lastPosCond)
		{        
			wstring tempStr = wstrItemDesc.substr(lastPosCond, posCond - lastPosCond);
			pListBox2->AddItem(tempStr.c_str(), NULL);

			lastPosCond = wstrItemDesc.find_first_not_of(L"\n", posCond);
			posCond = wstrItemDesc.find_first_of(L"\n", lastPosCond);
		}
#endif // SKILL_NOTE_SCROLLBAR
	}

}

void CX2UISkillNote::CheckRegisterMemo()
{
	int iMemo = 0;
	int iItemId = 0;
	CX2Item *pItem = NULL;

	if( g_pData == NULL || 
		g_pData->GetMyUser() == NULL || 
		g_pData->GetMyUser()->GetSelectUnit() == NULL || 
		g_pData->GetMyUser()->GetSelectUnit()->GetInventory() == NULL )
		return;
	pItem = g_pData->GetMyUser()->GetSelectUnit()->GetInventory()->GetItem( m_iRegisterMemo );
	if( pItem == NULL )
		return;	
    iItemId = pItem->GetItemTemplet()->GetItemID();
	

	if( m_iRegisterPage > 0 && iItemId > 0 )
	{
		char cMaxPage = g_pData->GetMyUser()->GetSelectUnit()->GetMaxSkillNoteSlot();
		for(char i=0; i<cMaxPage; ++i)
		{
			iMemo = g_pData->GetMyUser()->GetSelectUnit()->GetSkillMemoIdBySlot(i);			
			if( iMemo > 0 && iMemo == iItemId )
			{
				// 동일 메모가 등록되어 있음
				if( m_pDLGQuery == NULL )
				{
					m_pDLGQuery = g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_4992), g_pMain->GetNowState() );
				}
				return;
			}
		}

		iMemo = g_pData->GetMyUser()->GetSelectUnit()->GetSkillMemoIdBySlot(m_iRegisterPage-1);
		if( iMemo > 0 )
		{
			// 등록할 페이지에 다른 메모가 등록되어 있음
			if( m_pDLGQuery == NULL )
			{
				m_pDLGQuery = g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_4991), USNM_REGISTER_OK, g_pMain->GetNowState(), USNM_REGISTER_CANCEL );
			}			
			return;
		}
		else
		{
			// 비여있는 페이지
			if( m_pDLGQuery == NULL )
			{
				m_pDLGQuery = g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_4990), USNM_REGISTER_OK, g_pMain->GetNowState(), USNM_REGISTER_CANCEL );
			}			
			return;
		}
	}
	else
	{
		// 메모등록 불가상태
		return;
	}
}

void CX2UISkillNote::UpdateMemoEffect()
{
	if( g_pData != NULL && g_pData->GetUIMajorParticle() != NULL && m_pDLGSkillNote != NULL && GetShow() == true )
	{
		WCHAR wcControlName[20] = {0,};
		//wsprintf(wcControlName, L"_Icon_button%d", m_iUpdatePage - m_iCurrentPage);
		StringCchPrintf(wcControlName, 20, L"_Icon_button%d", m_iUpdatePage - m_iCurrentPage);
		CKTDGUIButton *pButton = (CKTDGUIButton*)m_pDLGSkillNote->GetControl(wcControlName);

		D3DXVECTOR2 vEffectPos = D3DXVECTOR2(0.f, 0.f);			

		vEffectPos = pButton->GetDialog()->GetPos() + pButton->GetPos();
		const float fButtonSizeX = (float)pButton->GetWidth();
		const float fButtonSizeY = (float)pButton->GetHeight();
		vEffectPos.x += fButtonSizeX/2;
		vEffectPos.y += fButtonSizeY/2;

		CKTDGParticleSystem::CParticleEventSequence* pParticle1 = 
			g_pData->GetUIMajorParticle()->CreateSequence( NULL, L"UI_Memo_StartEffect01", vEffectPos.x, vEffectPos.y, 0  );
		if( pParticle1 != NULL )
		{
			if( m_hParticle1 != INVALID_PARTICLE_HANDLE )
			{
				g_pData->GetUIMajorParticle()->DestroyInstanceHandle(m_hParticle1);
			}

			pParticle1->SetOverUI( true );
			m_hParticle1 = pParticle1->GetHandle();
		}

		CKTDGParticleSystem::CParticleEventSequence* pParticle2 = 
			g_pData->GetUIMajorParticle()->CreateSequence( NULL, L"UI_Memo_StartEffect02", vEffectPos.x, vEffectPos.y, 0  );
		if( pParticle2 != NULL )
		{
			if( m_hParticle2 != INVALID_PARTICLE_HANDLE )
			{
				g_pData->GetUIMajorParticle()->DestroyInstanceHandle(m_hParticle2);
			}

			pParticle2->SetOverUI( true );
			m_hParticle2 = pParticle2->GetHandle();
		}
	}	
}

void CX2UISkillNote::ResetSkillNote()
{
	m_bRegisterMemo = false;
	m_iRegisterMemo = 0;
	m_iUpdatePage = 0;
	m_bUpdateMemo = false;
	m_fWriteMemoTime = 0.f;	

	if( g_pMain->GetInfoBox() != NULL )
		g_pMain->DeleteInfoBox();

	if( m_hParticle1 != INVALID_PARTICLE_HANDLE )
		g_pData->GetUIMajorParticle()->DestroyInstanceHandle(m_hParticle1);	
	if( m_hParticle2 != INVALID_PARTICLE_HANDLE )
		g_pData->GetUIMajorParticle()->DestroyInstanceHandle(m_hParticle2);	
}

wstring CX2UISkillNote::ReplaceDesc_(const wchar_t* pwstrOri, const wchar_t* pwstrPattern, const wchar_t* pwstrReplace)
{
	wstring result = pwstrOri;   
	wstring::size_type pos = 0;   
	wstring::size_type offset = 0;   

    unsigned uSizePattern = wcslen( pwstrPattern );
    unsigned uSizeReplace = wcslen( pwstrReplace );

	while((pos = result.find(pwstrPattern, offset, uSizePattern)) != wstring::npos)   
	{   
		result.replace(result.begin() + pos, result.begin() + pos + uSizePattern, pwstrReplace, uSizeReplace);   
		offset = pos + uSizeReplace;   
	}   

	return result;   
}


#endif
