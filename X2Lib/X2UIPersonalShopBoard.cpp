#include "StdAfx.h"
#ifdef DEF_TRADE_BOARD // oasis907 : 김상윤 [2010.3.12] // 

#include ".\X2UIPersonalShopBoard.h"
#include < iomanip >

CX2UIPersonalShopBoard::CX2UIPersonalShopBoard(CKTDXStage* pNowStage, const WCHAR* pFileName )
: CX2ItemSlotManager( pNowStage, NULL ),
m_bShow( false ),
m_bOldApplyDelete( false ),
m_bRegMyGuildAd( false ),
m_pNowStage( pNowStage ),
m_pDlgPersonalShopBoard( NULL ),
m_uiNowPage( 1 ),
m_uiMaxPage( 0 ),
m_uiFirstPageinCurrentBoard( 1 ),
m_iComboFirstindex( 0 ),
m_iComboSecondindex( 0 ),
m_iComboCharacterindex( 0 ),
m_iComboClassindex( 0 ),
m_iIMEEditMinLevel( 0 ),
m_iIMEEditMaxLevel( 50 ),
m_fCurrentTime( 0.f ),
m_fLastSearchTime( 0.f ),
m_vPosDlg( 0, 0 ),
m_vSizeDlg( 0, 0 )
{
	RegisterLuaBind();


	m_pDlgPersonalShopBoard = new CKTDGUIDialog( const_cast< CKTDXStage* >( pNowStage ), pFileName );
	if ( m_pDlgPersonalShopBoard == NULL )
	{
		ASSERT( false );
		return;
	}
 
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDlgPersonalShopBoard );
	m_pDlgPersonalShopBoard->SetDisableUnderWindow( true );

	m_vPosDlg.x			= m_pDlgPersonalShopBoard->GetDummyPos( 0 ).x;		// 0은 posion
	m_vPosDlg.y			= m_pDlgPersonalShopBoard->GetDummyPos( 0 ).y;
	
	m_vSizeDlg.x		= m_pDlgPersonalShopBoard->GetDummyPos( 1 ).x;		// 1은 size
	m_vSizeDlg.y		= m_pDlgPersonalShopBoard->GetDummyPos( 1 ).y;




	m_pButtonPrevBoard = static_cast< CKTDGUIButton* >( m_pDlgPersonalShopBoard->GetControl( L"ButtonPrevBoard" ) );
	m_pButtonPrevPage = static_cast< CKTDGUIButton* >( m_pDlgPersonalShopBoard->GetControl( L"ButtonPrevPage" ) );

	m_pButtonNextPage = static_cast< CKTDGUIButton* >( m_pDlgPersonalShopBoard->GetControl( L"ButtonNextPage" ) );
	m_pButtonNextBoard = static_cast< CKTDGUIButton* >( m_pDlgPersonalShopBoard->GetControl( L"ButtonNextBoard" ) );


	m_pButtonSearchItem = static_cast< CKTDGUIButton* >( m_pDlgPersonalShopBoard->GetControl( L"ButtonSearchItem" ) );
	m_pButtonSearchItem->SetEnable(false);
	m_pButtonRefresh = static_cast< CKTDGUIButton* >( m_pDlgPersonalShopBoard->GetControl( L"ButtonRefresh" ) );

	m_pButtonInitSearchCondition = static_cast< CKTDGUIButton* >( m_pDlgPersonalShopBoard->GetControl( L"ButtonInitSearchCondition" ) );
	//m_pButtonBuyItem = static_cast< CKTDGUIButton* >( m_pDlgPersonalShopBoard->GetControl( L"ButtonBuyItem" ) );


	m_pComboFirst = static_cast< CKTDGUIComboBox* >( m_pDlgPersonalShopBoard->GetControl( L"ComboFirst" ) );



	m_pComboSecond = static_cast< CKTDGUIComboBox* >( m_pDlgPersonalShopBoard->GetControl( L"ComboSecond" ) );



	m_pComboCharacter = static_cast< CKTDGUIComboBox* >( m_pDlgPersonalShopBoard->GetControl( L"ComboCharacter" ) );



	m_pComboClass = static_cast< CKTDGUIComboBox* >( m_pDlgPersonalShopBoard->GetControl( L"ComboClass" ) );

	m_pCheckUnique = static_cast< CKTDGUICheckBox* >( m_pDlgPersonalShopBoard->GetControl( L"CheckUnique" ) );
	m_pCheckElite = static_cast< CKTDGUICheckBox* >( m_pDlgPersonalShopBoard->GetControl( L"CheckElite" ) );
	m_pCheckRare = static_cast< CKTDGUICheckBox* >( m_pDlgPersonalShopBoard->GetControl( L"CheckRare" ) );
	m_pCheckNormal = static_cast< CKTDGUICheckBox* >( m_pDlgPersonalShopBoard->GetControl( L"CheckNormal" ) );
	m_pCheckLow = static_cast< CKTDGUICheckBox* >( m_pDlgPersonalShopBoard->GetControl( L"CheckLow" ) );
	m_pCheckUsable = static_cast< CKTDGUICheckBox* >( m_pDlgPersonalShopBoard->GetControl( L"CheckUsable" ) );

	m_pIMEEditItemName = static_cast< CKTDGUIIMEEditBox* >( m_pDlgPersonalShopBoard->GetControl( L"IMEEditItemName" ) );
	m_pIMEEditMinLevel = static_cast< CKTDGUIIMEEditBox* >( m_pDlgPersonalShopBoard->GetControl( L"IMEEditMinLevel" ) );
	m_pIMEEditMaxLevel = static_cast< CKTDGUIIMEEditBox* >( m_pDlgPersonalShopBoard->GetControl( L"IMEEditMaxLevel" ) );

	m_pStaticPageNum = static_cast< CKTDGUIStatic* >( m_pDlgPersonalShopBoard->GetControl( L"StaticPageNum" ) );

	if ( m_pStaticPageNum != NULL )
	{
		m_pStaticPageNum->SetString( 0, L"1" );
		m_pStaticPageNum->SetString( 1, L"2" );
		m_pStaticPageNum->SetString( 2, L"3" );		
		m_pStaticPageNum->SetString( 3, L"4" );				
		m_pStaticPageNum->SetString( 4, L"5" );		
		m_pStaticPageNum->SetString( 5, L"6" );		
		m_pStaticPageNum->SetString( 6, L"7" );	
		m_pStaticPageNum->SetString( 7, L"8" );
		m_pStaticPageNum->SetString( 8, L"9" );
		m_pStaticPageNum->SetString( 9, L"10" );
	}

	for ( int i = 0; i < _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumPagePerBoard; i++ )
	{
		m_pButtonPageNum[i] = static_cast< CKTDGUIButton* >( m_pDlgPersonalShopBoard->GetControl( GET_REPLACED_STRING( ( STR_ID_3738, "Li", std::wstring( L"ButtonPageNum" ), i + 1 ) ) ) );
		m_pButtonPageNum[i]->SetShowEnable(false, false);
	}

	for ( int i = 0; i < _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumItemPerPage; i++ )
	{
		m_pItemList[i] = new CKTDGUIDialog( g_pMain->GetNowState(), L"DLG_Personal_Shop_Board_ItemList.lua" );
		D3DXVECTOR3 tempPos1 = m_pDlgPersonalShopBoard->GetDummyPos(i+2);
		D3DXVECTOR3 tempPos2 = m_pDlgPersonalShopBoard->GetPos();
		m_pItemList[i]->SetPos_LUA(tempPos1.x + tempPos2.x, tempPos1.y + tempPos2.y);

		CKTDGUIButton *pButton = static_cast< CKTDGUIButton* >( m_pItemList[i]->GetControl( L"ButtonBuyItem" ) );
		pButton->AddDummyInt(i);
		g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pItemList[i] );
		//m_pItemList[i]->SetLayer( XDL_GAME_EDIT );
		m_pItemList[i]->SetShowEnable(false, false);
	}


	for ( int i = 0; i < (int)m_SlotList.size(); i++ )
	{
		CX2SlotItem* pItemSlot = (CX2SlotItem*)m_SlotList[i];
		pItemSlot->LostItemUI();
		pItemSlot->SetShow( false );
		pItemSlot->SetEnable( false );
	}



	m_iSelectedPersonalShopUid = -1;

	UpdateItemList();
	InitSearchCondition();


}

CX2UIPersonalShopBoard::~CX2UIPersonalShopBoard()
{
	SAFE_DELETE_DIALOG( m_pDlgPersonalShopBoard );
}

void CX2UIPersonalShopBoard::SetIMMEditMinLevel(int iMinLevel)
{
#ifdef SERV_NEW_EVENT_TYPES
	int iMaxEXP = g_pData->GetEXPTable()->GetEXPData( _CONST_X2GAME_::g_iMaxLevel ).m_nTotalExp;
	if( g_pInstanceData != NULL )
		iMaxEXP = g_pData->GetEXPTable()->GetEXPData( g_pInstanceData->GetMaxLevel() ).m_nTotalExp;
#else SERV_NEW_EVENT_TYPES
	const int iMaxEXP = g_pData->GetEXPTable()->GetEXPData( _CONST_X2GAME_::g_iMaxLevel ).m_nTotalExp;
#endif SERV_NEW_EVENT_TYPES



	if(iMinLevel < 1)
	{
		iMinLevel = 1;
	}
#ifdef SERV_NEW_EVENT_TYPES
	else if( g_pInstanceData != NULL )
	{
		if ( iMinLevel > g_pInstanceData->GetMaxLevel() )
			iMinLevel = g_pInstanceData->GetMaxLevel();
	}
#endif SERV_NEW_EVENT_TYPES
	else if(iMinLevel > _CONST_X2GAME_::g_iMaxLevel )	// 해외팀 변경 LIMIT_MAX_LEVEL
	{
		iMinLevel = _CONST_X2GAME_::g_iMaxLevel;		// 해외팀 변경 LIMIT_MAX_LEVEL
	}
	
	WCHAR buff[256] = {0};
	_itow( iMinLevel, buff, 10 );
	m_pIMEEditMinLevel->SetText(buff, false);


	if(iMinLevel == 1)
	{
		m_iIMEEditMinLevel = 0;
	}
	else
	{
		m_iIMEEditMinLevel = iMinLevel;
	}
	return;
}

void CX2UIPersonalShopBoard::SetIMMEditMaxLevel(int iMaxLevel)
{
	if(iMaxLevel < 1)
	{
		iMaxLevel = 1;
	}
#ifdef SERV_NEW_EVENT_TYPES
	else if( g_pInstanceData != NULL )
	{
		if ( iMaxLevel > g_pInstanceData->GetMaxLevel() )
			iMaxLevel = g_pInstanceData->GetMaxLevel();
	}
#endif SERV_NEW_EVENT_TYPES
	else if(iMaxLevel > _CONST_X2GAME_::g_iMaxLevel )	// 해외팀 변경 LIMIT_MAX_LEVEL
	{
		iMaxLevel = _CONST_X2GAME_::g_iMaxLevel;		// 해외팀 변경 LIMIT_MAX_LEVEL
	}

	WCHAR buff[256] = {0};
	_itow( iMaxLevel, buff, 10 );
	m_pIMEEditMaxLevel->SetText(buff, false);

	if(iMaxLevel == 1)
	{
		m_iIMEEditMaxLevel = 0;
	}
	else
	{
		m_iIMEEditMaxLevel = iMaxLevel;
	}
	return;
}

HRESULT	CX2UIPersonalShopBoard::OnFrameMove( double fTime, float fElapsedTime )
{	
	UpdateButtonSearchItem();


	if(m_pDlgPersonalShopBoard != NULL && m_pDlgPersonalShopBoard->GetIsMouseOver() == true)
	{
		return CX2ItemSlotManager::OnFrameMove( fTime, fElapsedTime );
	}
	else
	{
		CX2ItemSlotManager::InvalidSelectedItem();
		CX2ItemSlotManager::InvalidSlotDesc();
	}
	return S_OK;
}

bool CX2UIPersonalShopBoard::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	// 앞에 모달 다이얼로그가 있으면 메시지를 처리하지 않는다
	if( true == g_pKTDXApp->GetDGManager()->GetDialogManager()->CheckFrontModalDlg( m_pDlgPersonalShopBoard ) )
		return false;

	bool bFlag	= false;

	switch( uMsg )
	{
		// 마우스 메시지들에 대해
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
		//{{ kimhc // 2011-03-27 // 상점검색에서 미리 입어보기
#ifndef	PRE_EQUIP_PERSONAL_SHOP_BOARD
	case WM_RBUTTONUP:
#endif	PRE_EQUIP_PERSONAL_SHOP_BOARD
		//}} kimhc // 2011-03-27 // 상점검색에서 미리 입어보기
	
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
		//case WM_MOUSEWHEEL:
		{
			if ( false == m_pDlgPersonalShopBoard->GetIsMouseOver())
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
			if ( false == m_pDlgPersonalShopBoard->GetIsMouseOver())
			{
				return false;
			}
			g_pData->GetUIManager()->SetUILayerIndex(CX2UIManager::UI_MENU_PERSONAL_SHOP_BOARD, true);
			bFlag = true;			
		}
		break;

		//{{ kimhc // 2011-03-27 // 상점검색에서 미리 입어보기
#ifdef	PRE_EQUIP_PERSONAL_SHOP_BOARD
	case WM_RBUTTONUP:
		{
			POINT mousePoint = { short( LOWORD(lParam) ), short( HIWORD(lParam) ) };
			if ( false == MouseRButtonUp( D3DXVECTOR2( static_cast<float>( mousePoint.x ), static_cast<float>( mousePoint.y) ) ) )
				return false;

			bFlag = true;

		} break;
#endif	PRE_EQUIP_PERSONAL_SHOP_BOARD
		//}} kimhc // 2011-03-27 // 상점검색에서 미리 입어보기
	default:
		break;
	}

	switch( uMsg )
	{
	case WM_LBUTTONUP:
/*
		{			
			POINT mousePoint = { short(LOWORD(lParam)), short(HIWORD(lParam)) };
			D3DXVECTOR2 mousePos = D3DXVECTOR2( (float)mousePoint.x, (float)mousePoint.y );
			m_NowMousePos = mousePos;

			CX2SlotItem* pSlotItem = (CX2SlotItem*)GetSlotInMousePos( mousePos );

			// 			if ( pSlotItem != NULL && pSlotItem->GetSlotType() == CX2Slot::ST_CASH_SHOP_MAIN_NEW )
			// 			{
			// 
			// 				if ( pSlotItem->GetSlotID() >= 0 && pSlotItem->GetSlotID() < (int)m_vecRecipeSlot.size() )
			// 				{
			// 					ManufactureSlot* pRecipeSlot = m_vecRecipeSlot[ pSlotItem->GetSlotID() ];
			// 					if ( pRecipeSlot != NULL )
			// 					{
			// 						g_pKTDXApp->GetDeviceManager()->PlaySound( L"X2_Button_Mouse_Up.ogg" );
			// 						SelectRecipe( pSlotItem->GetSlotID() );
			// 					}
			// 				}
			// 			}
			return true;

		}
*/
		break;
	case WM_MOUSEMOVE:
		{
			POINT mousePoint = { short(LOWORD(lParam)), short(HIWORD(lParam)) };
			D3DXVECTOR2 mousePos = D3DXVECTOR2( (float)mousePoint.x, (float)mousePoint.y );

			MouseMove( mousePos );
			return true;
		}
		break;
	}


	return bFlag;
}

bool CX2UIPersonalShopBoard::UICustomEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int tmp_min = 0;
	int tmp_max = 0;

	switch ( wParam )
	{
	case UPSBCM_EXIT:
		{
			SetShow( false );

			if ( g_pTFieldGame != NULL )
			{
				CX2TFieldNpc *pJoinNpc = g_pTFieldGame->GetFieldNPC( g_pTFieldGame->GetJoinNpcIndex() );
				if ( pJoinNpc != NULL )
				{
					if ( pJoinNpc->GetNpcShop() != NULL )
						pJoinNpc->GetNpcShop()->SetKeyEvent();
				}
			}
			return true;
		}
		break;
//-------------------------------------------------------------------------------------------//
//--------------------- 해외팀 하드코딩 스트링으로 변경 시작 -----------------------------------//
//--------------------- 나중에 아라도 바꿔 줘야 한다~~~~~   -----------------------------------//
//-------------------------------------------------------------------------------------------//
	case UPSBCM_COMBO_FIRST:
		{
			m_iComboFirstindex = m_pComboFirst->GetSelectedItemIndex();
			switch(m_iComboFirstindex)
			{
			case 0:
				m_pComboSecond->RemoveAllItems();
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5144 ), NULL, false);		// 부위(전체)
				m_pComboSecond->SetEnable(false);
				m_iComboSecondindex = 0;

				m_pComboCharacter->RemoveAllItems();
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5145 ), NULL, false);		// 케릭터(전체)
				m_pComboCharacter->SetEnable(false);
				m_iComboCharacterindex = 0;

				m_pComboClass->RemoveAllItems();
				m_pComboClass->AddItem(GET_STRING( STR_ID_5146 ), NULL, false);			// 클래스(전체)
				m_pComboClass->SetEnable(false);
				m_iComboClassindex = 0;
				break;
			case 1:
				m_pComboSecond->RemoveAllItems();
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5148 ), NULL, false);		// 부위(전체)
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5149 ), NULL, false);		// 무기
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5150 ), NULL, false);		// 상의
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5151 ), NULL, false);		// 하의
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5152 ), NULL, false);		// 헤어
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5153 ), NULL, false);		// 장갑
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5154 ), NULL, false);		// 신발
#ifdef SERV_NEW_ONE_PIECE_AVATAR_SLOT
				m_pComboSecond->AddItem(GET_STRING( STR_ID_28368 ), NULL, false);		// 한벌
#endif //SERV_NEW_ONE_PIECE_AVATAR_SLOT
				m_pComboSecond->SetEnable(true);
				m_iComboSecondindex = 0;

				m_pComboCharacter->RemoveAllItems();
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5155 ), NULL, false);		// 케릭터(전체)
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5156 ), NULL, false);		// 엘소드
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5157 ), NULL, false);		// 레나
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5158 ), NULL, false);		// 아이샤
#ifdef SHOPBOARD_NO_SEARCH_RAVEN
#else //SHOPBOARD_NO_SEARCH_RAVEN
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5159 ), NULL, false);		// 레이븐
#endif //SHOPBOARD_NO_SEARCH_RAVEN
#ifdef SHOPBOARD_NO_SEARCH_EVE
#else //SHOPBOARD_NO_SEARCH_EVE
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5160 ), NULL, false);		// 이브
#endif //SHOPBOARD_NO_SEARCH_EVE
#ifdef NEW_CHARACTER_CHUNG
#ifdef SHOPBOARD_NO_SEARCH_CHUNG
#else //SHOPBOARD_NO_SEARCH_CHUNG
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_12773 ), NULL, false);		// 청
#endif //SHOPBOARD_NO_SEARCH_CHUNG
#endif NEW_CHARACTER_CHUNG
#ifndef SERV_NO_ARA
#ifdef ARA_CHARACTER_BASE
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_25372 ), NULL, false);
#endif
#endif // SERV_NO_ARA
#ifdef NEW_CHARACTER_EL
				m_pComboCharacter->AddItem(L"#C0000FF엘리시스#CX", NULL, false);
#endif // NEW_CHARACTER_EL
				m_pComboCharacter->SetEnable(true);
				m_iComboCharacterindex = 0;
				break;
			case 2:
				m_pComboSecond->RemoveAllItems();
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5148 ), NULL, false);		// 부위(전체)
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5149 ), NULL, false);		// 무기
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5150 ), NULL, false);		// 상의
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5151 ), NULL, false);		// 하의
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5153 ), NULL, false);		// 장갑
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5154 ), NULL, false);		// 신발
				m_pComboSecond->SetEnable(true);
				m_iComboSecondindex = 0;

				m_pComboCharacter->RemoveAllItems();
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5155 ), NULL, false);		// 케릭터(전체)
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5156 ), NULL, false);		// 엘소드
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5157 ), NULL, false);		// 레나
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5158 ), NULL, false);		// 아이샤
#ifdef SHOPBOARD_NO_SEARCH_RAVEN
#else //SHOPBOARD_NO_SEARCH_RAVEN
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5159 ), NULL, false);		// 레이븐
#endif //SHOPBOARD_NO_SEARCH_RAVEN
#ifdef SHOPBOARD_NO_SEARCH_EVE
#else //SHOPBOARD_NO_SEARCH_EVE
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5160 ), NULL, false);		// 이브
#endif //SHOPBOARD_NO_SEARCH_EVE
				
#ifdef NEW_CHARACTER_CHUNG
#ifdef SHOPBOARD_NO_SEARCH_CHUNG
#else //SHOPBOARD_NO_SEARCH_CHUNG
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_12773 ), NULL, false);		// 청
#endif //SHOPBOARD_NO_SEARCH_CHUNG
				
#endif NEW_CHARACTER_CHUNG
#ifndef SERV_NO_ARA
#ifdef ARA_CHARACTER_BASE
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_25372 ), NULL, false);	// 아라
#endif
#endif // SERV_NO_ARA

#ifdef NEW_CHARACTER_EL
				m_pComboCharacter->AddItem(L"#C0000FF엘리시스#CX", NULL, false);
#endif // NEW_CHARACTER_EL
				m_pComboCharacter->SetEnable(true);
				m_iComboCharacterindex = 0;
				break;
			case 3:
				m_pComboSecond->RemoveAllItems();
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5148 ), NULL, false);		// 부위(전체)
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5149 ), NULL, false);		// 무기
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5161 ), NULL, false);		// 얼굴(상)
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5162 ), NULL, false);		// 얼굴(중)
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5163 ), NULL, false);		// 얼굴(하)
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5150 ), NULL, false);		// 상의
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5151 ), NULL, false);		// 하의
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5164 ), NULL, false);		// 팔
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5165 ), NULL, false);		// 반지
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5166 ), NULL, false);		// 목걸이
				m_pComboSecond->SetEnable(true);
				m_iComboSecondindex = 0;

				m_pComboCharacter->RemoveAllItems();
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5155 ), NULL, false);		// 케릭터(전체)
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5156 ), NULL, false);		// 엘소드
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5157 ), NULL, false);		// 레나
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5158 ), NULL, false);		// 아이샤
#ifdef SHOPBOARD_NO_SEARCH_RAVEN
#else //SHOPBOARD_NO_SEARCH_RAVEN
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5159 ), NULL, false);		// 레이븐
#endif //SHOPBOARD_NO_SEARCH_RAVEN
#ifdef SHOPBOARD_NO_SEARCH_EVE
#else //SHOPBOARD_NO_SEARCH_EVE
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5160 ), NULL, false);		// 이브
#endif //SHOPBOARD_NO_SEARCH_EVE
				
#ifdef NEW_CHARACTER_CHUNG
#ifdef SHOPBOARD_NO_SEARCH_CHUNG
#else //SHOPBOARD_NO_SEARCH_CHUNG
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_12773 ), NULL, false);		// 청
#endif //SHOPBOARD_NO_SEARCH_CHUNG
				
#endif NEW_CHARACTER_CHUNG
#ifndef SERV_NO_ARA
#ifdef ARA_CHARACTER_BASE
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_25372 ), NULL, false);
#endif
#endif // SERV_NO_ARA
#ifdef NEW_CHARACTER_EL
				m_pComboCharacter->AddItem(L"#C0000FF엘리시스#CX", NULL, false);
#endif // NEW_CHARACTER_EL
				m_pComboCharacter->SetEnable(true);
				m_iComboCharacterindex = 0;
				break;
			case 4:
				m_pComboSecond->RemoveAllItems();
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5147 ), NULL, false);		// 없음
				m_pComboSecond->SetEnable(true);
				m_iComboSecondindex = 0;

				m_pComboCharacter->RemoveAllItems();
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5155 ), NULL, false);		// 케릭터(전체)
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5156 ), NULL, false);		// 엘소드
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5157 ), NULL, false);		// 레나
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5158 ), NULL, false);		// 아이샤
#ifdef SHOPBOARD_NO_SEARCH_RAVEN
#else //SHOPBOARD_NO_SEARCH_RAVEN
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5159 ), NULL, false);		// 레이븐
#endif //SHOPBOARD_NO_SEARCH_RAVEN
#ifdef SHOPBOARD_NO_SEARCH_EVE
#else //SHOPBOARD_NO_SEARCH_EVE
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5160 ), NULL, false);		// 이브
#endif //SHOPBOARD_NO_SEARCH_EVE
				
#ifdef NEW_CHARACTER_CHUNG
#ifdef SHOPBOARD_NO_SEARCH_CHUNG
#else //SHOPBOARD_NO_SEARCH_CHUNG
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_12773 ), NULL, false);		// 청
#endif //SHOPBOARD_NO_SEARCH_CHUNG

#endif NEW_CHARACTER_CHUNG
#ifndef SERV_NO_ARA
#ifdef ARA_CHARACTER_BASE
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_25372 ), NULL, false);
#endif
#endif // SERV_NO_ARA
#ifdef NEW_CHARACTER_EL
				m_pComboCharacter->AddItem(L"#C0000FF엘리시스#CX", NULL, false);
#endif // NEW_CHARACTER_EL
				m_pComboCharacter->SetEnable(true);
				m_iComboCharacterindex = 0;
				break;
			case 5:
				m_pComboSecond->RemoveAllItems();
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5147 ), NULL, false);		// 없음
				m_pComboSecond->SetEnable(true);
				m_iComboSecondindex = 0;

				m_pComboCharacter->RemoveAllItems();
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5155 ), NULL, false);		// 케릭터(전체)
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5156 ), NULL, false);		// 엘소드
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5157 ), NULL, false);		// 레나
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5158 ), NULL, false);		// 아이샤
#ifdef SHOPBOARD_NO_SEARCH_RAVEN
#else //SHOPBOARD_NO_SEARCH_RAVEN
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5159 ), NULL, false);		// 레이븐
#endif //SHOPBOARD_NO_SEARCH_RAVEN
#ifdef SHOPBOARD_NO_SEARCH_EVE
#else //SHOPBOARD_NO_SEARCH_EVE
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5160 ), NULL, false);		// 이브
#endif //SHOPBOARD_NO_SEARCH_EVE
				
#ifdef NEW_CHARACTER_CHUNG
#ifdef SHOPBOARD_NO_SEARCH_CHUNG
#else //SHOPBOARD_NO_SEARCH_CHUNG
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_12773 ), NULL, false);		// 청
#endif //SHOPBOARD_NO_SEARCH_CHUNG
				
#endif NEW_CHARACTER_CHUNG
#ifndef SERV_NO_ARA
#ifdef ARA_CHARACTER_BASE
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_25372 ), NULL, false);
#endif
#endif // SERV_NO_ARA
#ifdef NEW_CHARACTER_EL
				m_pComboCharacter->AddItem(L"#C0000FF엘리시스#CX", NULL, false);
#endif // NEW_CHARACTER_EL
				m_pComboCharacter->SetEnable(true);
				m_iComboCharacterindex = 0;
				break;
			case 6:
				m_pComboSecond->RemoveAllItems();
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5147 ), NULL, false);		// 없음
				m_pComboSecond->SetEnable(true);
				m_iComboSecondindex = 0;

				m_pComboCharacter->RemoveAllItems();
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5155 ), NULL, false);		// 케릭터(전체)
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5156 ), NULL, false);		// 엘소드
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5157 ), NULL, false);		// 레나
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5158 ), NULL, false);		// 아이샤
#ifdef SHOPBOARD_NO_SEARCH_RAVEN
#else //SHOPBOARD_NO_SEARCH_RAVEN
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5159 ), NULL, false);		// 레이븐
#endif //SHOPBOARD_NO_SEARCH_RAVEN
#ifdef SHOPBOARD_NO_SEARCH_EVE
#else //SHOPBOARD_NO_SEARCH_EVE
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5160 ), NULL, false);		// 이브
#endif //SHOPBOARD_NO_SEARCH_EVE
				
#ifdef NEW_CHARACTER_CHUNG
#ifdef SHOPBOARD_NO_SEARCH_CHUNG
#else //SHOPBOARD_NO_SEARCH_CHUNG
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_12773 ), NULL, false);		// 청
#endif //SHOPBOARD_NO_SEARCH_CHUNG

#endif NEW_CHARACTER_CHUNG
#ifndef SERV_NO_ARA
#ifdef ARA_CHARACTER_BASE
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_25372 ), NULL, false);
#endif
#endif // SERV_NO_ARA
#ifdef NEW_CHARACTER_EL
				m_pComboCharacter->AddItem(L"#C0000FF엘리시스#CX", NULL, false);
#endif // NEW_CHARACTER_EL
				m_pComboCharacter->SetEnable(true);
				m_iComboCharacterindex = 0;
				break;
			case 7:
				m_pComboSecond->RemoveAllItems();
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5147 ), NULL, false);		// 없음
				m_pComboSecond->SetEnable(true);
				m_iComboSecondindex = 0;

				m_pComboCharacter->RemoveAllItems();
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5155 ), NULL, false);		// 케릭터(전체)
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5156 ), NULL, false);		// 엘소드
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5157 ), NULL, false);		// 레나
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5158 ), NULL, false);		// 아이샤
#ifdef SHOPBOARD_NO_SEARCH_RAVEN
#else //SHOPBOARD_NO_SEARCH_RAVEN
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5159 ), NULL, false);		// 레이븐
#endif //SHOPBOARD_NO_SEARCH_RAVEN
#ifdef SHOPBOARD_NO_SEARCH_EVE
#else //SHOPBOARD_NO_SEARCH_EVE
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5160 ), NULL, false);		// 이브
#endif //SHOPBOARD_NO_SEARCH_EVE

#ifdef NEW_CHARACTER_CHUNG
#ifdef SHOPBOARD_NO_SEARCH_CHUNG
#else //SHOPBOARD_NO_SEARCH_CHUNG
								m_pComboCharacter->AddItem(GET_STRING( STR_ID_12773 ), NULL, false);		// 청
#endif //SHOPBOARD_NO_SEARCH_CHUNG
#endif NEW_CHARACTER_CHUNG
#ifndef SERV_NO_ARA
#ifdef ARA_CHARACTER_BASE
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_25372 ), NULL, false);
#endif
#endif // SERV_NO_ARA
#ifdef NEW_CHARACTER_EL
				m_pComboCharacter->AddItem(L"#C0000FF엘리시스#CX", NULL, false);
#endif // NEW_CHARACTER_EL
				m_pComboCharacter->SetEnable(true);
				m_iComboCharacterindex = 0;
				break;
			default:
				m_pComboSecond->RemoveAllItems();
				m_pComboSecond->AddItem(GET_STRING( STR_ID_5144 ), NULL, false);		// 부위(전체)
				m_pComboSecond->SetEnable(false);
				m_iComboSecondindex = 0;

				m_pComboCharacter->RemoveAllItems();
				m_pComboCharacter->AddItem(GET_STRING( STR_ID_5145 ), NULL, false);		// 케릭터(전체)
				m_pComboCharacter->SetEnable(false);
				m_iComboCharacterindex = 0;

				m_pComboClass->RemoveAllItems();
				m_pComboClass->AddItem(GET_STRING( STR_ID_5146 ), NULL, false);			// 클래스(전체)
				m_pComboClass->SetEnable(false);
				m_iComboClassindex = 0;
				break;
			}
			UpdateButtonSearchItem();
			return true;
		}
		break;
	case UPSBCM_COMBO_SECOND:
		{
			m_iComboSecondindex = m_pComboSecond->GetSelectedItemIndex();
		}
		break;
	case UPSBCM_COMBO_CHARACTER:
		{
			m_iComboCharacterindex = m_pComboCharacter->GetSelectedItemIndex();
			switch(m_iComboCharacterindex)
			{
			case 0:
				m_pComboClass->RemoveAllItems();
				m_pComboClass->AddItem(GET_STRING( STR_ID_9691 ), NULL, false);		// 클래스(전체)
				m_pComboClass->SetEnable(false);
				m_iComboClassindex = 0;
				break;
			case 1:
				m_pComboClass->RemoveAllItems();
				m_pComboClass->AddItem(GET_STRING( STR_ID_9691 ), NULL, false);		// 클래스(전체)
				m_pComboClass->AddItem(GET_STRING( STR_ID_9692 ), NULL, false);		// 로드 나이트
				m_pComboClass->AddItem(GET_STRING( STR_ID_9693 ), NULL, false);		// 룬 슬레이어
#ifdef SERV_ELSWORD_INFINITY_SWORD
#ifdef SHOPBOARD_NO_SEARCH_ELSWORD_INFINITY_SWORD
#else //SHOPBOARD_NO_SEARCH_ELSWORD_INFINITY_SWORD
				m_pComboClass->AddItem(GET_STRING( STR_ID_21287 ), NULL, false);
#endif //SHOPBOARD_NO_SEARCH_ELSWORD_INFINITY_SWORD
#endif
				m_pComboClass->SetEnable(true);
				m_iComboClassindex = 0;
				break;
			case 2:
				m_pComboClass->RemoveAllItems();
				m_pComboClass->AddItem(GET_STRING( STR_ID_9691 ), NULL, false);		// 클래스(전체)
				m_pComboClass->AddItem(GET_STRING( STR_ID_9694 ), NULL, false);		// 그랜드 아처
				m_pComboClass->AddItem(GET_STRING( STR_ID_9695 ), NULL, false);		// 윈드 스니커
#ifdef SERV_RENA_NIGHT_WATCHER
#ifdef SHOPBOARD_NO_SEARCH_RENA_NIGHT_WATCHER
#else //SHOPBOARD_NO_SEARCH_RENA_NIGHT_WATCHER
				m_pComboClass->AddItem(GET_STRING( STR_ID_21289 ), NULL, false);
#endif //SHOPBOARD_NO_SEARCH_RENA_NIGHT_WATCHER
#endif
				m_pComboClass->SetEnable(true);
				m_iComboClassindex = 0;
				break;
			case 3:
				m_pComboClass->RemoveAllItems();
				m_pComboClass->AddItem(GET_STRING( STR_ID_9691 ), NULL, false);		// 클래스(전체)
				m_pComboClass->AddItem(GET_STRING( STR_ID_9696 ), NULL, false);		// 엘리멘탈 마스터
				m_pComboClass->AddItem(GET_STRING( STR_ID_9697 ), NULL, false);		// 보이드 프린세스
#ifdef SERV_ARME_DIMENSION_WITCH
#ifdef SHOPBOARD_NO_SEARCH_ARME_DIMENSION_WITCH
#else //SHOPBOARD_NO_SEARCH_ARME_DIMENSION_WITCH
				m_pComboClass->AddItem(GET_STRING( STR_ID_21288 ), NULL, false);
#endif //SHOPBOARD_NO_SEARCH_ARME_DIMENSION_WITCH
#endif
				m_pComboClass->SetEnable(true);
				m_iComboClassindex = 0;
				break;
			case 4:
				m_pComboClass->RemoveAllItems();
				m_pComboClass->AddItem(GET_STRING( STR_ID_9691 ), NULL, false);		// 클래스(전체)
#ifdef SHOPBOARD_NO_SEARCH_RAVEN
#else //SHOPBOARD_NO_SEARCH_RAVEN
				m_pComboClass->AddItem(GET_STRING( STR_ID_9698 ), NULL, false);		// 블레이드 마스터
				m_pComboClass->AddItem(GET_STRING( STR_ID_9699 ), NULL, false);		// 레크리스 피스트
#ifdef SERV_RAVEN_VETERAN_COMMANDER
#ifdef SHOPBOARD_NO_SEARCH_RAVEN_VETERAN_COMMANDER
#else //SHOPBOARD_NO_SEARCH_RAVEN_VETERAN_COMMANDER
				m_pComboClass->AddItem(GET_STRING( STR_ID_21290 ), NULL, false);
#endif //SHOPBOARD_NO_SEARCH_RAVEN_VETERAN_COMMANDER
#endif
#endif //SHOPBOARD_NO_SEARCH_RAVEN
				m_pComboClass->SetEnable(true);
				m_iComboClassindex = 0;
				break;
			case 5:
				m_pComboClass->RemoveAllItems();
				m_pComboClass->AddItem(GET_STRING( STR_ID_9691 ), NULL, false);		// 클래스(전체)
#ifdef SHOPBOARD_NO_SEARCH_EVE
#else //SHOPBOARD_NO_SEARCH_EVE
				m_pComboClass->AddItem(GET_STRING( STR_ID_9700 ), NULL, false);		// 네메시스
				m_pComboClass->AddItem(GET_STRING( STR_ID_9701 ), NULL, false);		// 엠프레스
#ifdef SERV_EVE_BATTLE_SERAPH
#ifdef SHOPBOARD_NO_SEARCH_EVE_BATTLE_SERAPH
#else //SHOPBOARD_NO_SEARCH_EVE_BATTLE_SERAPH
				m_pComboClass->AddItem(GET_STRING( STR_ID_21291 ), NULL, false);
#endif //SHOPBOARD_NO_SEARCH_EVE_BATTLE_SERAPH
#endif
#endif //SHOPBOARD_NO_SEARCH_EVE
				m_pComboClass->SetEnable(true);
				m_iComboClassindex = 0;
				break;
#ifdef NEW_CHARACTER_CHUNG
			case 6:
				m_pComboClass->RemoveAllItems();
#ifdef CHUNG_SECOND_CLASS_CHANGE
				m_pComboClass->AddItem(GET_STRING( STR_ID_9691 ), NULL, false);		// 클래스(전체)
#ifdef SHOPBOARD_NO_SEARCH_CHUNG
#else //SHOPBOARD_NO_SEARCH_CHUNG
				m_pComboClass->AddItem(GET_STRING( STR_ID_15051 ), NULL, false);
				m_pComboClass->AddItem(GET_STRING( STR_ID_15052 ), NULL, false);
#ifdef SERV_CHUNG_TACTICAL_TROOPER
#ifdef SHOPBOARD_NO_SEARCH_CHUNG_TACTICAL_TROOPER
#else //SHOPBOARD_NO_SEARCH_CHUNG_TACTICAL_TROOPER
				m_pComboClass->AddItem(GET_STRING( STR_ID_21292 ), NULL, false);
#endif //SHOPBOARD_NO_SEARCH_CHUNG_TACTICAL_TROOPER
#endif
#endif //SHOPBOARD_NO_SEARCH_CHUNG

				m_pComboClass->SetEnable(true);
#endif CHUNG_SECOND_CLASS_CHANGE
				m_iComboClassindex = 0;
				break;
#endif NEW_CHARACTER_CHUNG

#ifndef SERV_NO_ARA
#ifdef ARA_CHARACTER_BASE
			case 7:
				m_pComboClass->RemoveAllItems();
				m_pComboClass->AddItem(GET_STRING( STR_ID_9691 ), NULL, false);
	#ifdef ARA_CHANGE_CLASS_FIRST
				m_pComboClass->AddItem(GET_STRING( STR_ID_25373 ), NULL, false);
				m_pComboClass->SetEnable(true);
	#endif //ARA_CHANGE_CLASS_FIRST
	#ifdef SERV_ARA_CHANGE_CLASS_SECOND
				m_pComboClass->AddItem(GET_STRING( STR_ID_29385 ), NULL, false);
				m_pComboClass->SetEnable(true);
	#endif //SERV_ARA_CHANGE_CLASS_SECOND
				m_iComboClassindex = 0;
				break;
#endif
#endif // SERV_NO_ARA
#ifdef NEW_CHARACTER_EL
			case 8:
				{
					m_pComboClass->RemoveAllItems();
					m_pComboClass->AddItem(L"#C0000FF클래스(전체)#CX", NULL, false);
					m_pComboClass->SetEnable(true);
				} break;
#endif // NEW_CHARACTER_EL
			default:
				m_pComboClass->RemoveAllItems();
				m_pComboClass->AddItem(GET_STRING( STR_ID_9691 ), NULL, false);		// 클래스(전체)
				m_pComboClass->SetEnable(false);
				m_iComboClassindex = 0;
				break;
			}

			return true;
		}
		break;
//-------------------------------------------------------------------------------------------//
//--------------------- 해외팀 하드코딩 스트링으로 변경 끝 -----------------------------------//
//-------------------------------------------------------------------------------------------//		
		
	case UPSBCM_COMBO_CLASS:
		{
			m_iComboClassindex = m_pComboClass->GetSelectedItemIndex();
		}
		break;
	// 체크 박스
	case UPSBCM_CHECK_UNIQUE:
		{
			m_bCheckUnique = m_pCheckUnique->GetChecked();
			UpdateCheckBoxColor();
		}
		break;
	case UPSBCM_CHECK_ELITE:
		{
			m_bCheckElite = m_pCheckElite->GetChecked();
			UpdateCheckBoxColor();
		}
		break;
	case UPSBCM_CHECK_RARE:
		{
			m_bCheckRare = m_pCheckRare->GetChecked();
			UpdateCheckBoxColor();
		}
		break;
	case UPSBCM_CHECK_NORMAL:
		{
			m_bCheckNormal = m_pCheckNormal->GetChecked();
			UpdateCheckBoxColor();
		}
		break;
	case UPSBCM_CHECK_LOW:
		{
			m_bCheckLow = m_pCheckLow->GetChecked();
			UpdateCheckBoxColor();
		}
		break;
	case UPSBCM_CHECK_USABLE:
		{
			m_bCheckUsable = m_pCheckUsable->GetChecked();
			if(m_bCheckUsable)
			{
				UsableFilter();
			}
			else
			{
				UsableUnFilter();
			}
		}
		break;

	case UPSBCM_ENTER_MIN_LEVEL:
		{
			if(m_pIMEEditMinLevel->GetTextLength() == 0)
			{
				m_iIMEEditMinLevel = 0;
			}
			else
			{
				int iIMEEditMinLevel= (int) wcstol( m_pIMEEditMinLevel->GetText(), NULL, 10 );
				SetIMMEditMinLevel(iIMEEditMinLevel);
			}
		}
		break;
	case UPSBCM_ENTER_MAX_LEVEL:
		{
			if(m_pIMEEditMaxLevel->GetTextLength() == 0)
			{
				m_iIMEEditMaxLevel = 0;
			}
			else
			{
				int iIMEEditMaxLevel= (int) wcstol( m_pIMEEditMaxLevel->GetText(), NULL, 10 );
				SetIMMEditMaxLevel(iIMEEditMaxLevel);
			}
		}
		break;
	case UPSBCM_ENTER_ITEM_NAME:
		{
			if(m_pIMEEditItemName->GetTextLength() == 0)
			{
				m_wstrFinder = L"";
			}
			if(m_pIMEEditItemName->GetTextLength() == 1) // 2자 이상 입력 체크
			{
				m_wstrFinder = L"";
				m_pIMEEditItemName->SetText(L"", false);
				g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300),  GET_STRING(STR_ID_4982), g_pMain->GetNowState() );
			}
			else
			{
				m_wstrFinder = m_pIMEEditItemName->GetText();

				m_fCurrentTime = (float)DXUTGetGlobalTimer()->GetTime();

				if( m_fCurrentTime > m_fLastSearchTime + 3.0f)
				{
					if(m_iComboFirstindex != 0 || m_wstrFinder != L"")
					{
						m_fLastSearchTime = (float)DXUTGetGlobalTimer()->GetTime();
						Handler_EGS_SEARCH_TRADE_BOARD_REQ();
					}
				}
			}
			UpdateButtonSearchItem(); // 검색버튼 활성, 비활성화
		}
		break;

	case UPSBCM_MOUSE_OUT_ITEM_NAME:
		{
			if(m_pIMEEditItemName->GetTextLength() == 0)
			{
				m_wstrFinder = L"";
			}
			if(m_pIMEEditItemName->GetTextLength() == 1) // 2자 이상 입력 체크
			{
				m_wstrFinder = L"";
				m_pIMEEditItemName->SetText(L"", false);
				g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300),  GET_STRING(STR_ID_4982), g_pMain->GetNowState() );
			}
			else
			{
				m_wstrFinder = m_pIMEEditItemName->GetText();
			}
			UpdateButtonSearchItem(); // 검색버튼 활성, 비활성화
		}
		break;


	case UPSBCM_PUSH_SEARCH_ITEM:
		{
			m_fLastSearchTime = (float)DXUTGetGlobalTimer()->GetTime();
			Handler_EGS_SEARCH_TRADE_BOARD_REQ();
		}
		break;

	case UPSBCM_PUSH_REFRESH:
		{
			m_fCurrentTime = (float)DXUTGetGlobalTimer()->GetTime();

			if( m_fCurrentTime > m_fLastSearchTime + 3.0f)
			{
				if(m_iComboFirstindex != 0 || m_wstrFinder != L"")
				{
					m_fLastSearchTime = (float)DXUTGetGlobalTimer()->GetTime();
					Handler_EGS_SEARCH_TRADE_BOARD_REQ();
				}
			}
		}
		break;
	case UPSBCM_PUSH_INIT_SEARCH_CONDITION:
		{
			InitSearchCondition();
			m_vecItemInfo.clear();
			m_uiNowPage = 1;
			m_uiFirstPageinCurrentBoard = 1;
			m_uiMaxPage = 0;
			UpdateItemList();
		}
		break;
	case UPSBCM_PUSH_PREV_BOARD:
		{
			m_uiFirstPageinCurrentBoard = m_uiFirstPageinCurrentBoard - _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumPagePerBoard;
			m_uiNowPage = m_uiFirstPageinCurrentBoard;
			UpdateItemList();
		}
		break;
	case UPSBCM_PUSH_PREV_PAGE:
		{
			m_uiNowPage--;
			if(	m_uiNowPage	< m_uiFirstPageinCurrentBoard )
			{	
				m_uiFirstPageinCurrentBoard = m_uiFirstPageinCurrentBoard - _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumPagePerBoard;
			}
			UpdateItemList();
		}
		break;
	case UPSBCM_PUSH_NEXT_PAGE:
		{
			m_uiNowPage++;
			if(	m_uiNowPage	>= m_uiFirstPageinCurrentBoard + _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumPagePerBoard )
			{	
				m_uiFirstPageinCurrentBoard = m_uiFirstPageinCurrentBoard + _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumPagePerBoard;
			}
			UpdateItemList();
		}
		break;
	case UPSBCM_PUSH_NEXT_BOARD:
		{
			m_uiFirstPageinCurrentBoard = m_uiFirstPageinCurrentBoard + _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumPagePerBoard;
			m_uiNowPage = m_uiFirstPageinCurrentBoard;
			UpdateItemList();
		}
		break;
	case UPSBCM_PUSH_PAGE_NUM1:
		{
			m_uiNowPage = m_uiFirstPageinCurrentBoard;
			UpdateItemList();
		}
		break;
	case UPSBCM_PUSH_PAGE_NUM2:
		{
			m_uiNowPage = m_uiFirstPageinCurrentBoard + 1;
			UpdateItemList();
		}
		break;
	case UPSBCM_PUSH_PAGE_NUM3:
		{
			m_uiNowPage = m_uiFirstPageinCurrentBoard + 2;
			UpdateItemList();
		}
		break;
	case UPSBCM_PUSH_PAGE_NUM4:
		{
			m_uiNowPage = m_uiFirstPageinCurrentBoard + 3;
			UpdateItemList();
		}
		break;
	case UPSBCM_PUSH_PAGE_NUM5:
		{
			m_uiNowPage = m_uiFirstPageinCurrentBoard + 4;
			UpdateItemList();
		}
		break;
	case UPSBCM_PUSH_PAGE_NUM6:
		{
			m_uiNowPage = m_uiFirstPageinCurrentBoard + 5;
			UpdateItemList();
		}
		break;
	case UPSBCM_PUSH_PAGE_NUM7:
		{
			m_uiNowPage = m_uiFirstPageinCurrentBoard + 6;
			UpdateItemList();
		}
		break;
	case UPSBCM_PUSH_PAGE_NUM8:
		{
			m_uiNowPage = m_uiFirstPageinCurrentBoard + 7;
			UpdateItemList();
		}
		break;
	case UPSBCM_PUSH_PAGE_NUM9:
		{
			m_uiNowPage = m_uiFirstPageinCurrentBoard + 8;
			UpdateItemList();
		}
		break;
	case UPSBCM_PUSH_PAGE_NUM10:
		{
			m_uiNowPage = m_uiFirstPageinCurrentBoard + 9;
			UpdateItemList();
		}
		break;

	case UPSBCM_PUSH_BUY_ITEM:
		{
			CKTDGUIButton* pButton = (CKTDGUIButton*) lParam;
			//int iTargetPage = pButton->GetDummyInt(0);
			//GotoPage(iTargetPage);
			int iButtonIdx = pButton->GetDummyInt(0);
			
			if( iButtonIdx < 0 || iButtonIdx >= _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumItemPerPage )
				break;

			m_iSelectedPersonalShopUid = m_iPersonalShopUidList[iButtonIdx];
			
			// 임시 처리
			if(m_iSelectedPersonalShopUid != -1 )
			{
				SetShow( false );

				if ( g_pTFieldGame != NULL )
				{
					CX2TFieldNpc *pJoinNpc = g_pTFieldGame->GetFieldNPC( g_pTFieldGame->GetJoinNpcIndex() );
					if ( pJoinNpc != NULL )
					{
						if ( pJoinNpc->GetNpcShop() != NULL )
							pJoinNpc->GetNpcShop()->SetKeyEvent();
					}
				}
				
#ifdef SERV_PSHOP_AGENCY
				if( m_iShopOwnerUidList[iButtonIdx] == g_pData->GetMyUser()->GetSelectUnit()->GetUID() )
					g_pData->GetUIManager()->ToggleUI(CX2UIManager::UI_MENU_PERSONAL_SHOP, true, CX2UIPersonalShop::OPENMYSHOPFROMBOARD);
				else
					g_pData->GetUIManager()->ToggleUI(CX2UIManager::UI_MENU_PERSONAL_SHOP, true, CX2UIPersonalShop::OPENFROMBOARD);
#else
				g_pData->GetUIManager()->ToggleUI(CX2UIManager::UI_MENU_PERSONAL_SHOP, true, CX2UIPersonalShop::OPENFROMBOARD);
#endif

			}
		}
		break;

	default:
		break;
	}

	return false;
}

void CX2UIPersonalShopBoard::UsableFilter()
{
	UINT j = 0;
	m_vecItemInfoBackup.clear();
	m_vecItemInfoBackup.resize( m_vecItemInfo.size() );
	std::copy( m_vecItemInfo.begin(), m_vecItemInfo.end(), m_vecItemInfoBackup.begin() );

	for(UINT i = 0; i< m_vecItemInfo.size(); i++)
	{
		int iItemID = m_vecItemInfo[i].m_kSellItemInfo.m_kInventoryItemInfo.m_kItemInfo.m_iItemID;
		
		if( CX2Unit::CanUse(iItemID, g_pData->GetMyUser()->GetSelectUnit(), 0) == true )
		{
			m_vecItemInfo[j] =  m_vecItemInfo[i];
			j++;
		}
	}
	m_vecItemInfo.resize(j);

	m_uiNowPage = 1;
	m_uiFirstPageinCurrentBoard = 1;
	if(m_vecItemInfo.size() == 0)
	{
		m_uiMaxPage = 0;
	}
	else
	{
		m_uiMaxPage = static_cast< int >( (m_vecItemInfo.size() - 1) / _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumItemPerPage) + 1;
	}

	UpdateItemList();
	return;
}

void CX2UIPersonalShopBoard::UsableUnFilter()
{
	m_vecItemInfo.clear();
	m_vecItemInfo.resize( m_vecItemInfoBackup.size() );
	std::copy( m_vecItemInfoBackup.begin(), m_vecItemInfoBackup.end(), m_vecItemInfo.begin() );

	m_uiNowPage = 1;
	m_uiFirstPageinCurrentBoard = 1;
	if(m_vecItemInfo.size() == 0)
	{
		m_uiMaxPage = 0;
	}
	else
	{
		m_uiMaxPage = static_cast< int >( (m_vecItemInfo.size() - 1) / _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumItemPerPage) + 1;
	}

	UpdateItemList();
	return;
}

wstring CX2UIPersonalShopBoard::GetSlotItemDesc()
{
	KTDXPROFILE();

	wstring itemDesc=  L"";


	if ( m_pNowOverItemSlot != NULL )
	{
		BOOST_TEST_FOREACH( KTradeBoardItemInfo, ItemInfo, m_vecItemInfo )
		{
			if ( ItemInfo.m_kSellItemInfo.m_kInventoryItemInfo.m_iItemUID == m_pNowOverItemSlot->GetItemUID() )
			{
				CX2Item::ItemData* pItemData = new CX2Item::ItemData( ItemInfo.m_kSellItemInfo.m_kInventoryItemInfo );
				CX2Item* pItem = new CX2Item( pItemData, NULL );
				itemDesc = GetSlotItemDescByTID( pItem, pItem->GetItemData()->m_ItemID, false );
				SAFE_DELETE( pItem );
				break;
			}
		}
	}


	return itemDesc;
}


bool CX2UIPersonalShopBoard::UIServerEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( wParam )
	{
	case EGS_SEARCH_TRADE_BOARD_ACK:
		{
			return Handler_EGS_SEARCH_TRADE_BOARD_ACK( hWnd, uMsg, wParam, lParam );
		}
		break;

	default:
		break;
	}

	return false;
}

void CX2UIPersonalShopBoard::InitSearchCondition()
{
	m_iComboFirstindex = 0;
	m_iComboSecondindex = 0;
	m_iComboCharacterindex = 0;
	m_iComboClassindex = 0;

//-------------------------------------------------------------------------------------------//
//--------------------- 해외팀 하드코딩 스트링으로 변경 시작 -----------------------------------//
//-------------------------------------------------------------------------------------------//

	m_pComboFirst->RemoveAllItems();
	m_pComboFirst->AddItem(GET_STRING( STR_ID_5167 ), NULL, false);				// 종류(전체)
	m_pComboFirst->AddItem(GET_STRING( STR_ID_5168 ), NULL, false);				// 아바타
	m_pComboFirst->AddItem(GET_STRING( STR_ID_5169 ), NULL, false);				// 장비
	m_pComboFirst->AddItem(GET_STRING( STR_ID_5170 ), NULL, false);				// 악세사리
	m_pComboFirst->AddItem(GET_STRING( STR_ID_5171 ), NULL, false);				// 소비
	m_pComboFirst->AddItem(GET_STRING( STR_ID_5172 ), NULL, false);				// 특수
	m_pComboFirst->AddItem(GET_STRING( STR_ID_5173 ), NULL, false);				// 재료
	m_pComboFirst->AddItem(GET_STRING( STR_ID_5174 ), NULL, false);				// 퀘스트

	m_pComboSecond->RemoveAllItems();
	m_pComboSecond->AddItem(GET_STRING( STR_ID_5144 ), NULL, false);
	m_pComboSecond->SetEnable(false);

	m_pComboCharacter->RemoveAllItems();
	m_pComboCharacter->AddItem(GET_STRING( STR_ID_5145 ), NULL, false);
	m_pComboCharacter->SetEnable(false);

	m_pComboClass->RemoveAllItems();
	m_pComboClass->AddItem(GET_STRING( STR_ID_5146 ), NULL, false);
	m_pComboClass->SetEnable(false);

	m_iIMEEditMinLevel = 0;
	m_iIMEEditMaxLevel = _CONST_X2GAME_::g_iMaxLevel;	// 해외팀 변경

//-------------------------------------------------------------------------------------------//
//--------------------- 해외팀 하드코딩 스트링으로 변경 끝 -----------------------------------//
//-------------------------------------------------------------------------------------------//

	m_pIMEEditMinLevel->SetText(L"", false);
	m_pIMEEditMaxLevel->SetText(L"", false);


	m_bCheckUnique = true;
	m_bCheckElite = true;
	m_bCheckRare = true;
	m_bCheckNormal = true;
	m_bCheckLow = false;

	m_bCheckUsable = false;

	m_pCheckUnique->SetChecked(true);
	m_pCheckElite->SetChecked(true);
	m_pCheckRare->SetChecked(true);
	m_pCheckNormal->SetChecked(true);
	m_pCheckLow->SetChecked(false);

	m_pCheckUsable->SetChecked(false);

	UpdateCheckBoxColor();

	m_wstrFinder = L"";
	m_pIMEEditItemName->SetText(L"", false);

	UpdateButtonSearchItem();

	return;
}

bool CX2UIPersonalShopBoard::Handler_EGS_SEARCH_TRADE_BOARD_REQ()
{
	KEGS_SEARCH_TRADE_BOARD_REQ kPacket;


	CX2Inventory::SORT_TYPE nowSortType = CX2Inventory::ST_NONE;
	switch(m_iComboFirstindex)
	{
	case 0:
		nowSortType = CX2Inventory::ST_NONE;
		break;
	case 1:
		nowSortType = CX2Inventory::ST_AVARTA;
		break;
	case 2:
		nowSortType = CX2Inventory::ST_EQUIP;
		break;
	case 3:
		nowSortType = CX2Inventory::ST_ACCESSORY;
		break;
	case 4:
		nowSortType = CX2Inventory::ST_QUICK_SLOT;
		break;
	case 5:
		nowSortType = CX2Inventory::ST_SPECIAL;
		break;
	case 6:
		nowSortType = CX2Inventory::ST_MATERIAL;
		break;
	case 7:
		nowSortType = CX2Inventory::ST_QUEST;
		break;
	default:
		nowSortType = CX2Inventory::ST_NONE;
		break;
	}

	if(nowSortType == CX2Inventory::ST_NONE)
	{
		kPacket.m_cSlotCategory = -1;
	}
	else
	{
		kPacket.m_cSlotCategory	= static_cast< char >( nowSortType );
	}


	CX2Unit::EQIP_POSITION nowEquipPosition = CX2Unit::EP_NONE;
	switch(nowSortType)
	{
	case CX2Inventory::ST_NONE:
		nowEquipPosition = CX2Unit::EP_NONE;
		break;
	case CX2Inventory::ST_AVARTA:
		{
			switch(m_iComboSecondindex)
			{
			case 0:
				nowEquipPosition = CX2Unit::EP_NONE;
				break;
			case 1:
				nowEquipPosition = CX2Unit::EP_WEAPON_HAND;
				break;
			case 2:
				nowEquipPosition = CX2Unit::EP_DEFENCE_BODY;
				break;
			case 3:
				nowEquipPosition = CX2Unit::EP_DEFENCE_LEG;
				break;
			case 4:
				nowEquipPosition = CX2Unit::EP_DEFENCE_HAIR;
				break;
			case 5:
				nowEquipPosition = CX2Unit::EP_DEFENCE_HAND;
				break;
			case 6:
				nowEquipPosition = CX2Unit::EP_DEFENCE_FOOT;
				break;
#ifdef SERV_NEW_ONE_PIECE_AVATAR_SLOT
			case 7:
				nowEquipPosition = CX2Unit::EP_ONEPIECE_FASHION;
				break;
#endif //SERV_NEW_ONE_PIECE_AVATAR_SLOT
			default:
				nowEquipPosition = CX2Unit::EP_NONE;
				break;
			}
		}
		break;
	case CX2Inventory::ST_EQUIP:
		{
			switch(m_iComboSecondindex)
			{
			case 0:
				nowEquipPosition = CX2Unit::EP_NONE;
				break;
			case 1:
				nowEquipPosition = CX2Unit::EP_WEAPON_HAND;
				break;
			case 2:
				nowEquipPosition = CX2Unit::EP_DEFENCE_BODY;
				break;
			case 3:
				nowEquipPosition = CX2Unit::EP_DEFENCE_LEG;
				break;
			case 4:
				nowEquipPosition = CX2Unit::EP_DEFENCE_HAND;
				break;
			case 5:
				nowEquipPosition = CX2Unit::EP_DEFENCE_FOOT;
				break;
			default:
				nowEquipPosition = CX2Unit::EP_NONE;
				break;
			}
		}
		break;
	case CX2Inventory::ST_ACCESSORY:
		{
			switch(m_iComboSecondindex)
			{
			case 0:
				nowEquipPosition = CX2Unit::EP_NONE;
				break;
			case 1:
				nowEquipPosition = CX2Unit::EP_AC_WEAPON;
				break;
			case 2:
				nowEquipPosition = CX2Unit::EP_AC_FACE1; // 얼굴 어디?
				break;
			case 3:
				nowEquipPosition = CX2Unit::EP_AC_FACE2; // 얼굴 어디?
				break;
			case 4:
				nowEquipPosition = CX2Unit::EP_AC_FACE3; // 얼굴 어디?
				break;
			case 5:
				nowEquipPosition = CX2Unit::EP_AC_BODY;
				break;
			case 6:
				nowEquipPosition = CX2Unit::EP_AC_LEG;
				break;
			case 7:
				nowEquipPosition = CX2Unit::EP_AC_ARM;
				break;
			case 8:
				nowEquipPosition = CX2Unit::EP_AC_RING;
				break;
			case 9:
				nowEquipPosition = CX2Unit::EP_AC_NECKLESS;
				break;
			default:
				nowEquipPosition = CX2Unit::EP_NONE;
				break;
			}
		}
		break;
	case CX2Inventory::ST_QUICK_SLOT:
		nowEquipPosition = CX2Unit::EP_NONE;
		break;
	case CX2Inventory::ST_SPECIAL:
		nowEquipPosition = CX2Unit::EP_NONE;
		break;
	case CX2Inventory::ST_MATERIAL:
		nowEquipPosition = CX2Unit::EP_NONE;
		break;
	case CX2Inventory::ST_QUEST:
		nowEquipPosition = CX2Unit::EP_NONE;
		break;
	default:
		nowEquipPosition = CX2Unit::EP_NONE;
		break;
	}

	if(nowEquipPosition == CX2Unit::EP_NONE)
	{
		kPacket.m_cEqipPosition = -1;
	}
	else
	{
		kPacket.m_cEqipPosition	= static_cast< char >( nowEquipPosition );
	}


	CX2Unit::UNIT_TYPE nowUnitType = CX2Unit::UT_NONE;
	switch(m_iComboCharacterindex)
	{
	case 0:
		nowUnitType = CX2Unit::UT_NONE;
		break;
	case 1:
		nowUnitType = CX2Unit::UT_ELSWORD;
		break;
	case 2:
		nowUnitType = CX2Unit::UT_LIRE;
		break;
	case 3:
		nowUnitType = CX2Unit::UT_ARME;
		break;
	case 4:
		nowUnitType = CX2Unit::UT_RAVEN;
		break;
	case 5:
		nowUnitType = CX2Unit::UT_EVE;
		break;
#ifdef NEW_CHARACTER_CHUNG
	case 6:
		nowUnitType = CX2Unit::UT_CHUNG;
		break;
#endif NEW_CHARACTER_CHUNG

#ifndef SERV_NO_ARA
#ifdef ARA_CHARACTER_BASE
	case 7:
		nowUnitType = CX2Unit::UT_ARA;
		break;
#endif
#endif // SERV_NO_ARA
#ifdef NEW_CHARACTER_EL
	case 8:
		{
			nowUnitType = CX2Unit::UT_ELESIS;
		} break;
#endif // NEW_CHARACTER_EL
	default:
		nowUnitType = CX2Unit::UT_NONE;
		break;
	}

	if(nowUnitType == CX2Unit::UT_NONE)
	{
		kPacket.m_cUnitType = -1;
	}
	else
	{
		kPacket.m_cUnitType	= static_cast< char >( nowUnitType );
	}

	CX2Unit::UNIT_CLASS nowUnitClass = CX2Unit::UC_NONE;
	switch(nowUnitType)
	{
	case CX2Unit::UT_NONE:
		nowUnitClass = CX2Unit::UC_NONE;
		break;
	case CX2Unit::UT_ELSWORD:
		{
			switch(m_iComboClassindex)
			{
			case 0:
				nowUnitClass = CX2Unit::UC_NONE;
				break;
			case 1:
				nowUnitClass = CX2Unit::UC_ELSWORD_LORD_KNIGHT;
				break;
			case 2:
				nowUnitClass = CX2Unit::UC_ELSWORD_RUNE_SLAYER;
				break;
			case 3:
				nowUnitClass = CX2Unit::UC_ELSWORD_INFINITY_SWORD;
				break;
			default:
				nowUnitClass = CX2Unit::UC_NONE;
				break;
			}
		}
		break;
	case CX2Unit::UT_LIRE:
		{
			switch(m_iComboClassindex)
			{
			case 0:
				nowUnitClass = CX2Unit::UC_NONE;
				break;
			case 1:
				nowUnitClass = CX2Unit::UC_LIRE_GRAND_ARCHER;
				break;
			case 2:
				nowUnitClass = CX2Unit::UC_LIRE_WIND_SNEAKER;
				break;
			case 3:
				nowUnitClass = CX2Unit::UC_LIRE_NIGHT_WATCHER;
				break;
			default:
				nowUnitClass = CX2Unit::UC_NONE;
				break;
			}
		}
		break;
	case CX2Unit::UT_ARME:
		{
			switch(m_iComboClassindex)
			{
			case 0:
				nowUnitClass = CX2Unit::UC_NONE;
				break;
			case 1:
				nowUnitClass = CX2Unit::UC_ARME_ELEMENTAL_MASTER;
				break;
			case 2:
				nowUnitClass = CX2Unit::UC_ARME_VOID_PRINCESS;
				break;
			case 3:
				nowUnitClass = CX2Unit::UC_ARME_DIMENSION_WITCH;
				break;
			default:
				nowUnitClass = CX2Unit::UC_NONE;
				break;
			}
		}
		break;
	case CX2Unit::UT_RAVEN:
		{
			switch(m_iComboClassindex)
			{
			case 0:
				nowUnitClass = CX2Unit::UC_NONE;
				break;
			case 1:
				nowUnitClass = CX2Unit::UC_RAVEN_BLADE_MASTER;
				break;
			case 2:
				nowUnitClass = CX2Unit::UC_RAVEN_RECKLESS_FIST;
				break;
			case 3:
				nowUnitClass = CX2Unit::UC_RAVEN_VETERAN_COMMANDER;
				break;
			default:
				nowUnitClass = CX2Unit::UC_NONE;
				break;
			}
		}
		break;
	case CX2Unit::UT_EVE:
		{
			switch(m_iComboClassindex)
			{
			case 0:
				nowUnitClass = CX2Unit::UC_NONE;
				break;
			case 1:
				nowUnitClass = CX2Unit::UC_EVE_CODE_NEMESIS;
				break;
			case 2:
				nowUnitClass = CX2Unit::UC_EVE_CODE_EMPRESS;
				break;
			case 3:
				nowUnitClass = CX2Unit::UC_EVE_BATTLE_SERAPH;
				break;
			default:
				nowUnitClass = CX2Unit::UC_NONE;
				break;
			}
		}
		break;
#ifdef NEW_CHARACTER_CHUNG
	case CX2Unit::UT_CHUNG:
		{
			switch(m_iComboClassindex)
			{
			case 0:
				nowUnitClass = CX2Unit::UC_NONE;
				break;
#ifdef CHUNG_SECOND_CLASS_CHANGE
			case 1:
				nowUnitClass = CX2Unit::UC_CHUNG_IRON_PALADIN;
				break;
			case 2:
				nowUnitClass = CX2Unit::UC_CHUNG_DEADLY_CHASER;
				break;
#endif CHUNG_SECOND_CLASS_CHANGE
			case 3:
				nowUnitClass = CX2Unit::UC_CHUNG_TACTICAL_TROOPER;
				break;
			default:
				nowUnitClass = CX2Unit::UC_NONE;
				break;
			}
		}
		break;
#endif NEW_CHARACTER_CHUNG

#ifndef SERV_NO_ARA
#ifdef ARA_CHARACTER_BASE
	case CX2Unit::UT_ARA:
		{
			switch(m_iComboClassindex)
			{
			case 0:
				nowUnitClass = CX2Unit::UC_NONE;
				break;
			case 1:
				nowUnitClass = CX2Unit::UC_ARA_SAKRA_DEVANAM;
				break;
#ifdef SERV_ARA_CHANGE_CLASS_SECOND
			case 2:
				nowUnitClass = CX2Unit::UC_ARA_YAMA_RAJA;
				break;
#endif //SERV_ARA_CHANGE_CLASS_SECOND
			default:
				nowUnitClass = CX2Unit::UC_NONE;
				break;
			}
		} break;
#endif
#endif // SERV_NO_ARA
#ifdef NEW_CHARACTER_EL
	case CX2Unit::UT_ELESIS:
		{
			switch(m_iComboClassindex)
			{
			case 0:
				nowUnitClass = CX2Unit::UC_NONE;
				break;
			default:
				nowUnitClass = CX2Unit::UC_NONE;
				break;
			}
		} break;
#endif // NEW_CHARACTER_EL
	default:
		nowUnitClass = CX2Unit::UC_NONE;
		break;
	}

	if(nowUnitClass == CX2Unit::UC_NONE)
	{
		kPacket.m_cUnitClass = -1;
	}
	else
	{
		kPacket.m_cUnitClass	= static_cast< char >( nowUnitClass );
	}


	int uMinLevel = 0;
	int uMaxLevel = 0;

	// 해외팀 변경
	if( m_iIMEEditMinLevel == 0 && m_iIMEEditMaxLevel == 0 ||
		m_iIMEEditMinLevel == 0 && m_iIMEEditMaxLevel == _CONST_X2GAME_::g_iMaxLevel ||
		m_iIMEEditMinLevel == _CONST_X2GAME_::g_iMaxLevel && m_iIMEEditMaxLevel == 0)
	{
		uMinLevel = -1;
		uMaxLevel = -1;
	}
	else if (m_iIMEEditMinLevel <= m_iIMEEditMaxLevel)
	{
		uMinLevel = m_iIMEEditMinLevel;
		uMaxLevel = m_iIMEEditMaxLevel;
	}
	else
	{
		uMinLevel = m_iIMEEditMaxLevel;
		uMaxLevel = m_iIMEEditMinLevel;
	}
	
	kPacket.m_cMinLevel =  static_cast< char >( uMinLevel );
	kPacket.m_cMaxLevel =  static_cast< char >( uMaxLevel );

	// 아이템등급 필요함

	kPacket.m_vecItemGrade.clear();


	if(m_bCheckUnique)
	{
		kPacket.m_vecItemGrade.push_back(  static_cast<char> (CX2Item::IG_UNIQUE)  );
	}

	if(m_bCheckElite)
	{
		kPacket.m_vecItemGrade.push_back(  static_cast<char> (CX2Item::IG_ELITE)  );
	}

	if(m_bCheckRare)
	{
		kPacket.m_vecItemGrade.push_back(  static_cast<char> (CX2Item::IG_RARE)  );
	}

	if(m_bCheckNormal)
	{
		kPacket.m_vecItemGrade.push_back(  static_cast<char> (CX2Item::IG_NORMAL)  );
	}

	if(m_bCheckLow)
	{
		kPacket.m_vecItemGrade.push_back(  static_cast<char> (CX2Item::IG_LOW)  );
	}

	kPacket.m_wstrFinder = m_wstrFinder;

	g_pData->GetServerProtocol()->SendPacket( EGS_SEARCH_TRADE_BOARD_REQ, kPacket ); 
	g_pMain->AddServerPacket( EGS_SEARCH_TRADE_BOARD_ACK); 

	return true;
}
bool CX2UIPersonalShopBoard::Handler_EGS_SEARCH_TRADE_BOARD_ACK(  HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	
	KEGS_SEARCH_TRADE_BOARD_ACK kEvent;
	DeSerialize( pBuff, &kEvent );

	if ( g_pMain->DeleteServerPacket( EGS_SEARCH_TRADE_BOARD_ACK ) == true )
	{
		if ( g_pMain->IsValidPacket( kEvent.m_iOK ) == true )
		{
			m_vecItemInfo.clear();
			m_vecItemInfo.resize( kEvent.m_vecItemInfo.size() );
			std::copy( kEvent.m_vecItemInfo.begin(), kEvent.m_vecItemInfo.end(), m_vecItemInfo.begin() );

			m_uiNowPage = 1;
			m_uiFirstPageinCurrentBoard = 1;
			if(m_vecItemInfo.size() == 0)
			{
				m_uiMaxPage = 0;
			}
			else
			{
				m_uiMaxPage = static_cast< int >( (m_vecItemInfo.size() - 1) / _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumItemPerPage) + 1;
			}
		
			if(GetShow() == true)
			{
				if(m_bCheckUsable == true)
				{
					UsableFilter();
				}
				else
				{
					UpdateItemList();
				}
			}
		}
		else
		{
			m_vecItemInfo.clear();
			m_uiNowPage = 1;
			m_uiFirstPageinCurrentBoard = 1;
			m_uiMaxPage = 0;
			if(GetShow() == true)
			{
				UpdateItemList();
			}
		}
	}
	return true;
}

void CX2UIPersonalShopBoard::UpdateItemList()
{
	int uiFirstItemidx = _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumItemPerPage * (m_uiNowPage - 1);
	int uiLastItemidx = m_vecItemInfo.size() - 1;

	WCHAR buff[256] = {0,};

	for ( int i = 0; i < _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumItemPerPage; i++ )
	{
		CX2SlotItem* pItemSlot = (CX2SlotItem*)GetSlot(i);


		if( uiFirstItemidx + i > uiLastItemidx)
		{
			if ( pItemSlot != NULL )
			{
				pItemSlot->DestroyItemUI();
				pItemSlot->SetShow(false);
				pItemSlot->SetEnable(false);
			}

			m_pItemList[i]->SetShowEnable(false, false);
			m_iPersonalShopUidList[i] = -1;
#ifdef SERV_PSHOP_AGENCY
			m_iShopOwnerUidList[i] = -1;
#endif
		}
		else
		{
			// 아이템 정보를 리스트에 담는다.
			KTradeBoardItemInfo& kTradeBoardItemInfo = m_vecItemInfo[uiFirstItemidx + i];
	
			UidType uItemUID = kTradeBoardItemInfo.m_kSellItemInfo.m_kInventoryItemInfo.m_iItemUID;
			int iItemID = kTradeBoardItemInfo.m_kSellItemInfo.m_kInventoryItemInfo.m_kItemInfo.m_iItemID;
			int iItemQuantity = kTradeBoardItemInfo.m_kSellItemInfo.m_kInventoryItemInfo.m_kItemInfo.m_iQuantity;
			int iED = kTradeBoardItemInfo.m_kSellItemInfo.m_iPricePerOne;

#ifdef SERV_PSHOP_AGENCY_NO_COMMISSION_EVENT
			if( kTradeBoardItemInfo.m_kSellItemInfo.m_cPShopItemType == KSellPersonalShopItemInfo::SPIT_PERSONAL_SHOP )
			{
				int iCommissionED = static_cast<int> (kTradeBoardItemInfo.m_kSellItemInfo.m_iPricePerOne * _CONST_UIPERSONALSHOPBOARD_INFO_::g_dPersonalShopBoardFee);
				iED += iCommissionED;
			}
#else
			int iCommissionED = static_cast<int> (kTradeBoardItemInfo.m_kSellItemInfo.m_iPricePerOne * _CONST_UIPERSONALSHOPBOARD_INFO_::g_dPersonalShopBoardFee);
			iED += iCommissionED;
#endif

			wstring wstrSellerNickName = kTradeBoardItemInfo.m_wstrSellerNickName;

			CX2Item::ITEM_GRADE iGrade = g_pData->GetItemManager()->GetItemTemplet( iItemID )->GetItemGrade();
			m_iSelectedPersonalShopUid = kTradeBoardItemInfo.m_iPersonalShopUID;

			m_iPersonalShopUidList[i] = kTradeBoardItemInfo.m_iPersonalShopUID;
#ifdef SERV_PSHOP_AGENCY
			m_iShopOwnerUidList[i] = kTradeBoardItemInfo.m_iHostUnitUID;
#endif

			if ( pItemSlot != NULL )
			{

				//CX2Item::ItemData* pItemData = new CX2Item::ItemData( kTradeBoardItemInfo.m_kSellItemInfo.m_kInventoryItemInfo);
				//CX2Item* pItem = new CX2Item( pItemData, NULL );

				pItemSlot->CreateItemUI( iItemID );
				pItemSlot->SetItemUID( uItemUID );
				pItemSlot->SetShow(true);
				pItemSlot->SetEnable(true);
			}

			CX2Item::ItemData* pItemData = new CX2Item::ItemData( kTradeBoardItemInfo.m_kSellItemInfo.m_kInventoryItemInfo );
			CX2Item* pItem = new CX2Item( pItemData, NULL );
		
		
			CKTDGUIStatic* pStaticItemName = NULL;
			pStaticItemName = static_cast< CKTDGUIStatic* >( m_pItemList[i]->GetControl(L"StaticItemName") );


			wstring wstrItemName = g_pData->GetItemManager()->GetItemTemplet( iItemID )->GetNameColor_();
			wstrItemName += pItem->GetFullName();

			wstrItemName += L"#CX";
			
#ifdef CLIENT_GLOBAL_LINEBREAK
			// 임홍락 07.24 상점검색창 아이템 이름 길어 UI를 벗어나는 문제 수정
			wstrItemName = CWordLineHandler::GetStrByLineBreakColorInX2Main(wstrItemName.c_str(), 250, SLOT_MANAGER_FONT_INDEX);
#endif //CLIENT_GLOBAL_LINEBREAK

			pStaticItemName->GetString(0)->msg = wstrItemName;
	
			CKTDGUIStatic* pStaticNum = NULL;
			pStaticNum = static_cast< CKTDGUIStatic* >( m_pItemList[i]->GetControl(L"StaticNum") );
			//wsprintf( buff, L"%d개", iItemQuantity );
			// 해외팀 하드코딩 스트링으로 변경
			StringCchPrintf( buff, 256, GET_STRING( STR_ID_5175 ), iItemQuantity );

			pStaticNum->SetString(0, buff);


			CKTDGUIStatic* pStaticED = NULL;
			pStaticED = static_cast< CKTDGUIStatic* >( m_pItemList[i]->GetControl(L"StaticED") );

			wstring wstrED = g_pMain->GetEDString(iED);
			wstrED += L" ED";
			pStaticED->GetString(0)->msg = wstrED;
			pStaticED->SetColor( g_pMain->GetPriceColor( iED ) );


			CKTDGUIStatic* pStaticSeller = NULL;
			pStaticSeller = static_cast< CKTDGUIStatic* >( m_pItemList[i]->GetControl(L"g_pStatic_User") );
			pStaticSeller->SetString(0, wstrSellerNickName.c_str());


			if ( pItem->GetItemData() != NULL && pItem->GetItemData()->m_bIsSealed == true )
			{
				pItemSlot->SetShowSealedImage( true );
			}
			else
			{
				pItemSlot->SetShowSealedImage( false );
			}



			SAFE_DELETE( pItem );

			m_pItemList[i]->SetShowEnable(true, true);

		}
	}

	UINT uiFirstPage = m_uiFirstPageinCurrentBoard;
	int iNumDisplayPageButton = 0;

	// 모든 페이지를 표기해야하는 경우, 즉 페이지 버튼의 이동이 필요없는 경우 
	if( uiFirstPage + _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumPagePerBoard - 1 <= m_uiMaxPage )
	{
		iNumDisplayPageButton = _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumPagePerBoard;
	}
	else
	{
		iNumDisplayPageButton = m_uiMaxPage - uiFirstPage + 1;
	}
	
	float fWidthPageButton = (_CONST_UIPERSONALSHOPBOARD_INFO_::g_fWidthPage) / (_CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumPagePerBoard);
	float fOffsetPageButton = (-fWidthPageButton * iNumDisplayPageButton) + (fWidthPageButton * _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumPagePerBoard);

	// 페이지 버튼을 업데이트 한다.
	for( UINT i = 0; i < _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumPagePerBoard; i++)
	{
		if( uiFirstPage + i > m_uiMaxPage) // 최대 페이지를 넘어선 경우 페이지 스트링 출력 X 
		{
			m_pStaticPageNum->SetString( i, L"");
			m_pButtonPageNum[i]->SetShowEnable(false, false);
		}
		else if( uiFirstPage + i == m_uiNowPage ) // 현재 페이지 버튼
		{
			_itow( uiFirstPage + i, buff, 10 );
			m_pStaticPageNum->SetString( i, buff );
			m_pStaticPageNum->GetString( i )->color = D3DXCOLOR(0.6f,0.0f,0.0f,1.0f);
			m_pButtonPageNum[i]->SetShowEnable(true, false);
			
		}
		else
		{
			_itow( uiFirstPage + i, buff, 10 );
			m_pStaticPageNum->SetString( i, buff );
			m_pStaticPageNum->GetString( i )->color = D3DXCOLOR(0.4f,0.4f,0.4f,1.0f);
			m_pButtonPageNum[i]->SetShowEnable(true, true);
			
		}
		m_pButtonPageNum[i]->SetOffsetPos_LUA(fOffsetPageButton, 0.f);
	}
	m_pStaticPageNum->SetOffsetPos_LUA(fOffsetPageButton, 0.f);

	// 화살표 버튼 업데이트, UI event의 예외 처리 역할도 함
	// <<- //

	
	if(iNumDisplayPageButton == 0)
	{
		m_pButtonPrevBoard->SetShowEnable(false, false);
		m_pButtonPrevPage->SetShowEnable(false, false);
		m_pButtonNextPage->SetShowEnable(false, false);
		m_pButtonNextBoard->SetShowEnable(false, false);
		return;
	}


	float fOffsetArrow = 10.f;
	if(m_uiFirstPageinCurrentBoard == 1)
	{
		m_pButtonPrevBoard->SetShowEnable(true, false);
	}
	else
	{
		m_pButtonPrevBoard->SetShowEnable(true, true);
	}
	m_pButtonPrevBoard->SetOffsetPos_LUA(fOffsetPageButton - fOffsetArrow, 0.f);
	// <- //
	if(m_uiNowPage == 1)
	{
		m_pButtonPrevPage->SetShowEnable(true, false);
	}
	else
	{
		m_pButtonPrevPage->SetShowEnable(true, true);
	}
	m_pButtonPrevPage->SetOffsetPos_LUA(fOffsetPageButton - fOffsetArrow, 0.f);
	// -> //
	if(m_uiNowPage >= m_uiMaxPage)
	{
		m_pButtonNextPage->SetShowEnable(true, false);
	}
	else
	{
		m_pButtonNextPage->SetShowEnable(true, true);
	}
	m_pButtonNextPage->SetOffsetPos_LUA(-fOffsetPageButton + fOffsetArrow, 0.f);
	// ->> //
	if(m_uiMaxPage < m_uiFirstPageinCurrentBoard + _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumPagePerBoard )
	{
		m_pButtonNextBoard->SetShowEnable(true, false);
	}
	else
	{
		m_pButtonNextBoard->SetShowEnable(true, true);
	}
	m_pButtonNextBoard->SetOffsetPos_LUA(-fOffsetPageButton + fOffsetArrow, 0.f);

	
	if( m_uiMaxPage < _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumPagePerBoard + 1 )
	{
		m_pButtonPrevBoard->SetShow(false);
		m_pButtonNextBoard->SetShow(false);
	}
	return;
}


void CX2UIPersonalShopBoard::UpdateCheckBoxColor()
{
	if(m_bCheckLow)
	{
		CKTDGUIStatic* pStaticGrade = NULL;
		pStaticGrade = static_cast< CKTDGUIStatic* >( m_pDlgPersonalShopBoard->GetControl(L"StaticGrade") );
		pStaticGrade->GetString(4)->color = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	}
	else
	{
		CKTDGUIStatic* pStaticGrade = NULL;
		pStaticGrade = static_cast< CKTDGUIStatic* >( m_pDlgPersonalShopBoard->GetControl(L"StaticGrade") );
		pStaticGrade->GetString(4)->color = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
	}
	if(m_bCheckNormal)
	{
		CKTDGUIStatic* pStaticGrade = NULL;
		pStaticGrade = static_cast< CKTDGUIStatic* >( m_pDlgPersonalShopBoard->GetControl(L"StaticGrade") );

		pStaticGrade->GetString(3)->color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		CKTDGUIStatic* pStaticGrade = NULL;
		pStaticGrade = static_cast< CKTDGUIStatic* >( m_pDlgPersonalShopBoard->GetControl(L"StaticGrade") );
		pStaticGrade->GetString(3)->color = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
	}
	if(m_bCheckRare)
	{
		CKTDGUIStatic* pStaticGrade = NULL;
		pStaticGrade = static_cast< CKTDGUIStatic* >( m_pDlgPersonalShopBoard->GetControl(L"StaticGrade") );

		pStaticGrade->GetString(2)->color = D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f);
	}
	else
	{
		CKTDGUIStatic* pStaticGrade = NULL;
		pStaticGrade = static_cast< CKTDGUIStatic* >( m_pDlgPersonalShopBoard->GetControl(L"StaticGrade") );
		pStaticGrade->GetString(2)->color = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
	}
	if(m_bCheckElite)
	{
		CKTDGUIStatic* pStaticGrade = NULL;
		pStaticGrade = static_cast< CKTDGUIStatic* >( m_pDlgPersonalShopBoard->GetControl(L"StaticGrade") );
		pStaticGrade->GetString(1)->color = D3DXCOLOR(1.0f, 0.0f, 1.0f, 1.0f);
	}
	else
	{
		CKTDGUIStatic* pStaticGrade = NULL;
		pStaticGrade = static_cast< CKTDGUIStatic* >( m_pDlgPersonalShopBoard->GetControl(L"StaticGrade") );
		pStaticGrade->GetString(1)->color = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
	}
	if(m_bCheckUnique)
	{
		CKTDGUIStatic* pStaticGrade = NULL;
		pStaticGrade = static_cast< CKTDGUIStatic* >( m_pDlgPersonalShopBoard->GetControl(L"StaticGrade") );
		pStaticGrade->GetString(0)->color = D3DXCOLOR(1.0f, 0.8f, 0.56f, 1.0f);
	}
	else
	{
		CKTDGUIStatic* pStaticGrade = NULL;
		pStaticGrade = static_cast< CKTDGUIStatic* >( m_pDlgPersonalShopBoard->GetControl(L"StaticGrade") );
		pStaticGrade->GetString(0)->color = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
	}

	return;
}



void CX2UIPersonalShopBoard::EnableButtonSearchItem(bool bEnable)
{
	if(bEnable)
	{
		CKTDGUIStatic* pStaticSearch = NULL;
		pStaticSearch = static_cast< CKTDGUIStatic* >( m_pDlgPersonalShopBoard->GetControl(L"StaticSearch") );
		pStaticSearch->GetString(0)->color = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
		pStaticSearch->GetString(1)->color = D3DXCOLOR(0.0f, 0.9f, 1.0f, 1.0f);
		m_pButtonSearchItem->SetEnable(true);
	}
	else
	{
		CKTDGUIStatic* pStaticSearch = NULL;
		pStaticSearch = static_cast< CKTDGUIStatic* >( m_pDlgPersonalShopBoard->GetControl(L"StaticSearch") );
		pStaticSearch->GetString(0)->color = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
		pStaticSearch->GetString(1)->color = D3DXCOLOR(0.9f, 0.9f, 0.9f, 1.0f);
		m_pButtonSearchItem->SetEnable(false);
	}
	return;
}


void CX2UIPersonalShopBoard::UpdateButtonSearchItem()
{
	m_fCurrentTime = (float)DXUTGetGlobalTimer()->GetTime();

	if( m_fCurrentTime > m_fLastSearchTime + 3.0f)
	{
		// 검색 조건에 따라 검색 버튼 활성, 비활성화
		if(m_iComboFirstindex != 0)
		{
			EnableButtonSearchItem(true);
		}
		else if (m_wstrFinder != L"") // 검색어 존재하면 첫번째 카테고리 없어도 검색 가능
		{
			EnableButtonSearchItem(true);
		}
		else
		{
			EnableButtonSearchItem(false);
		}
	}
	else
	{
		EnableButtonSearchItem(false);
	}

	return;
}


bool CX2UIPersonalShopBoard::GetShow() const
{
	return m_bShow;
}

void CX2UIPersonalShopBoard::SetShow( bool bShow )
{
	m_bShow = bShow;

	if ( bShow )
	{
		g_pData->GetUIManager()->UIOpened( CX2UIManager::UI_MENU_PERSONAL_SHOP_BOARD );
		UpdateItemList();
	}
	else
	{
		g_pData->GetUIManager()->UIClosed( CX2UIManager::UI_MENU_PERSONAL_SHOP_BOARD );
	
		for ( int i = 0; i < _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumItemPerPage; i++ )
		{
			m_pItemList[i]->SetShowEnable(false, false);
		}


		for ( int i = 0; i < (int)m_SlotList.size(); i++ )
		{
			CX2SlotItem* pItemSlot = (CX2SlotItem*)GetSlot(i);
			if ( pItemSlot != NULL )
			{
				pItemSlot->DestroyItemUI();
				pItemSlot->SetShow(false);
				pItemSlot->SetEnable(false);
			}
		}
		InvalidSelectedItem();
		InvalidSlotDesc();

	}

	m_pDlgPersonalShopBoard->SetShowEnable( m_bShow, m_bShow );
	
}

D3DXVECTOR2	CX2UIPersonalShopBoard::GetDLGSize() const
{
	return m_vSizeDlg;		// 임시
}

void CX2UIPersonalShopBoard::SetPosition( D3DXVECTOR2 vec )
{
	m_vPosDlg = vec;
	m_pDlgPersonalShopBoard->SetPos( m_vPosDlg );
}

void CX2UIPersonalShopBoard::SetLayer( X2_DIALOG_LAYER layer )
{
	SetSlotManagerLayer(layer);
	SetSlotManagerChangeSequence(false);


	for ( int i = 0; i < _CONST_UIPERSONALSHOPBOARD_INFO_::g_iNumItemPerPage; i++ )
	{
		if(m_pItemList[i] != NULL)
		{
			g_pKTDXApp->GetDGManager()->GetDialogManager()->ChangeLayer(m_pItemList[i], layer);
		}	
	}

	if( m_pDlgPersonalShopBoard != NULL) 
	{
		g_pKTDXApp->GetDGManager()->GetDialogManager()->ChangeLayer( m_pDlgPersonalShopBoard, layer);
		g_pKTDXApp->GetDGManager()->GetDialogManager()->ChangeSequence( m_pDlgPersonalShopBoard, false );
	}
}

//{{ kimhc // 2011-03-27 // 상점검색에서 미리 입어보기
#ifdef	PRE_EQUIP_PERSONAL_SHOP_BOARD

bool CX2UIPersonalShopBoard::MouseRButtonUp( const D3DXVECTOR2& v2MousePos )
{
	CX2SlotItem* pSlotItem = static_cast<CX2SlotItem*>( GetSlotInMousePos( v2MousePos ) );
	if ( NULL == pSlotItem )
		return false;

	const int iItemID = pSlotItem->GetItemTID();
    const CX2Item::ItemTemplet* pItemTemplet = g_pData->GetItemManager()->GetItemTemplet( iItemID );

	// 장착 가능한 타입이 아니면
	if ( CX2Item::UT_NONE == pItemTemplet->GetUseType() )
		return false;

	const CX2Unit* pMyUnit = g_pData->GetMyUser()->GetSelectUnit();

	// 아이템의 장착 조건을 확인
	switch ( pItemTemplet->GetUseCondition() )
	{
	case CX2Item::UC_ANYONE:		// 누구나 장착할 수 있다면 통과
		break;
		
	case CX2Item::UC_ONE_UNIT:		// 유닛의 타입에 따라 장착 할 수 있다면
		{
			// 유닛 타입만 체크한 후, 타입이 다르면 return false
			if ( pMyUnit->GetType() != pItemTemplet->GetUnitType() )
				return false;

			// 타입이 같으면 통과
		} break;

	case CX2Item::UC_ONE_CLASS:		// 한 클래스만 장착 할 수 있다면
		{
			// 클래스를 체크한 후, 클래스가 다르면 return false
			if ( pMyUnit->GetClass() != pItemTemplet->GetUnitClass() )
				return false;

			// 클래스가 같으면 통과
		} break;

	default:
		return false;
		break;
	}	

	g_pData->GetUIManager()->ToggleUI( CX2UIManager::UI_MENU_CHARINFO, true );

	CX2UICharInfo* pUICharInfo = g_pData->GetUIManager()->GetUICharInfo();
	pUICharInfo->GetUnitViewerUI()->AddEquipByTID( iItemID );

	return true;
}

#endif	PRE_EQUIP_PERSONAL_SHOP_BOARD
//}} kimhc // 2011-03-27 // 상점검색에서 미리 입어보기



#endif DEF_TRADE_BOARD

