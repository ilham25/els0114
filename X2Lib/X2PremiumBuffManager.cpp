/**@file	X2PremiumBuff.cpp
   @breif	�޸� ���� ���� ���� �� ��Ÿ Ư�� ���� ���� 
*/
#pragma once

#include "StdAfx.h"
#include ".\X2PremiumBuffManager.h"

#ifdef COME_BACK_REWARD

#pragma region CX2PremiumBuffManager
/** @function 	: CX2PremuimBuff
	@brief 		: ������
	@param		: ����
	@return		: ����
*/
CX2PremiumBuffManager::CX2PremiumBuffManager()
	//{{ kimhc // 2011-08-08 // ���� ���� ä�θ� �̺�Ʈ
#ifdef	SERV_TOONILAND_CHANNELING_EVENT
//: m_bTooniLandChannelingEvent( false )	/// ���Ϸ��� ä�θ� �̺�Ʈ ���� �ű� ������ ������ ����
#endif	SERV_TOONILAND_CHANNELING_EVENT
//}} kimhc // 2011-08-08 // ���� ���� ä�θ� �̺�Ʈ
#ifdef BUFF_TEMPLET_SYSTEM
: m_bBuffTempletUI( false )
#endif
{
	Clear();
}
#pragma endregion CX2PremiumBuffManager

#pragma region ~CX2PremiumBuffManager
/** @function 	: ~CX2PremuimBuff
	@brief 		: �Ҹ���
	@param		: ����
	@return		: ����
*/
CX2PremiumBuffManager::~CX2PremiumBuffManager()
{
	SAFE_DELETE_DIALOG( m_pDLGWelcomeComeBackUser );
	SAFE_DELETE_DIALOG( m_pDLGComeBackUserBuffDesc );
}
#pragma endregion ~CX2PremiumBuffManager

#pragma region Clear
/** @function 	: Clear
	@brief 		: ��� �ʱ�ȭ
	@param		: ����
	@return		: ����
*/
void CX2PremiumBuffManager::Clear()
{
	//{{ bool
	m_bComeBackUser				= false;
	m_bDescComplete				= false;
	m_bChatViewFirst			= true;
	m_bFirstComeBack			= false;
	m_bFirstComeBackBuffDesc	= false;
	m_bPremiumPC				= false;
	m_bPremiumBuffUI			= false;
	m_iComeBackBuffEnd			= false;
	m_bComeBackuserInParty		= false;

	//{{ int
	m_iComeBackBuffEnd			= 0;
	//}}

	//{{ Pointer
	m_pDLGWelcomeComeBackUser	= NULL;
	m_pDLGComeBackUserBuffDesc	= NULL;
	//}}

#ifdef SERV_PC_BANG_TYPE
	m_kPcBangReward.m_bAvatar					= false;
	m_kPcBangReward.m_iPcBangType				= -1;
	m_kPcBangReward.m_fEXPRate					= 0.0f;
	m_kPcBangReward.m_fEDRate					= 0.0f;
	m_kPcBangReward.m_fVSPointRate				= 0.0f;
	m_kPcBangReward.m_bSpiritFree				= false;
	m_kPcBangReward.m_fPetEXPRate				= 0.0f;
	m_kPcBangReward.m_iAdditionalHenirReward	= 0;
	m_kPcBangReward.m_bSecretDungeonFree		= false;
	m_kPcBangReward.m_bAvatar					= false;
	m_kPcBangReward.m_bTitle					= false;
#endif SERV_PC_BANG_TYPE
}
#pragma endregion Clear

#pragma region IsComeBackUser
/** @function 	: IsComeBackUser
	@brief 		: �޸� ���� ���� �Ǵ�
	@param		: ����
	@return		: bool
*/
bool CX2PremiumBuffManager::IsComeBackUser() const
{
	return m_bComeBackUser;
}
#pragma endregion IsComeBackUser

#pragma region SetComeBackUser
/** @function 	: SetComeBackUser
	@brief 		: �޸� ���� ���� ����
	@param		: bool
	@return		: ����
*/
void CX2PremiumBuffManager::SetComeBackUser( IN bool bComeBackUser_ )
{
	m_bComeBackUser = bComeBackUser_;
}
#pragma endregion SetComeBackUser

#pragma region GetComeBackBuffEnd
/** @function 	: GetComeBackBuffEnd
	@brief 		: ���� ���� �ð� ���� �ѱ�
	@return		: ref INT64
*/
INT64 CX2PremiumBuffManager::GetComeBackBuffEnd() const
{
	return m_iComeBackBuffEnd;
}
#pragma endregion GetComeBackBuffEnd

#pragma region IsFirstComeBack
/** @function 	: IsFirstComeBack
	@brief 		: �޸� ���� ���� ù ���� ���� Ȯ��
	@param		: ����
	@return		: bool
*/
bool CX2PremiumBuffManager::IsFirstComeBack() const
{
	return m_bFirstComeBack;
}
#pragma endregion IsFirstComeBack

#pragma region IsFirstComeBackBuffDesc
/** @function 	: IsFirstComeBackBuffDesc
	@brief 		: �޸� ���� ���� ù ���� ���� ǥ�� ���� Ȯ��
	@param		: ����
	@return		: bool
*/
bool CX2PremiumBuffManager::IsFirstComeBackBuffDesc() const
{
	return m_bFirstComeBackBuffDesc;
}
#pragma endregion IsFirstComeBackBuffDesc

#pragma region IsDescComplete
/** @function 	: IsDescComplete
	@brief 		: Premium Buff ���� Ȯ�� �Ϸ� �� Ÿ��Ʋ Ȱ��ȭ
	@return		: bool
*/
bool CX2PremiumBuffManager::IsDescComplete() const
{
	return m_bDescComplete;
}
#pragma endregion IsDescComplete

#pragma region SetFirstComeBack
/** @function 	: SetFirstComeBack
	@brief 		: �޸� ���� ���� ù ���� ����
	@param		: bool
	@return		: ����
*/
void CX2PremiumBuffManager::SetFirstComeBack( IN bool bFirstComeBack_ )
{
	m_bFirstComeBack = m_bFirstComeBackBuffDesc = bFirstComeBack_;
}
#pragma endregion SetFirstComeBack

#pragma region SetShowFirstComBackUI
/** @function 	: SetShowFirstComBackUI
	@brief 		: �޸� ���� ���� ù ���� ����
	@param		: bool
	@return		: ����
*/
void CX2PremiumBuffManager::SetShowFirstComBackUI( IN bool bShow_ )
{
	switch ( g_pMain->GetNowStateID() )
	{
	case CX2Main::XS_SERVER_SELECT:
		if ( NULL != m_pDLGWelcomeComeBackUser )
		{
#ifdef NO_UI_COMEBACK_USER
			m_pDLGWelcomeComeBackUser->SetShowEnable( false, false );
#else
			m_pDLGWelcomeComeBackUser->SetShowEnable( true, true );
#endif NO_UI_COMEBACK_USER
		}
		break;
	case CX2Main::XS_VILLAGE_MAP:
	case CX2Main::XS_BATTLE_FIELD:
		if ( NULL != m_pDLGComeBackUserBuffDesc )
		{
#ifdef NO_UI_COMEBACK_USER
			m_pDLGComeBackUserBuffDesc->SetShowEnable( false, false );
#else
			m_pDLGComeBackUserBuffDesc->SetShowEnable( true, true );
#endif NO_UI_COMEBACK_USER
		}
		break;
	}
}
#pragma endregion SetShowFirstComBackUI

#pragma region IsPremiumPC
/** @function 	: IsPremiumPC
	@brief 		: PC������ Ȯ��
	@param		: ����
	@return		: bool
*/
bool CX2PremiumBuffManager::IsPremiumPC() const
{
	return m_bPremiumPC;
}
#pragma endregion IsPremiumPC

#pragma region SetPremiumPC
/** @function 	: SetPremiumPC
	@brief 		: PC�� ����
	@param		: bool
	@return		: ����
*/
void CX2PremiumBuffManager::SetPremiumPC( IN bool bPremiumPC_ )
{
	m_bPremiumPC = bPremiumPC_;
}
#pragma endregion SetPremiumPC

#pragma region GetShowPremiumBuffUI
/** @function 	: GetShowPremiumBuffUI
	@brief 		: �����̾� ���� ǥ�� Ȯ��
	@param		: ����
	@return		: bool
*/
bool CX2PremiumBuffManager::GetShowPremiumBuffUI() const
{
	return m_bPremiumBuffUI;
}
#pragma endregion GetShowPremiumBuffUI

#pragma region SetShowPremiumBuffUI
/** @function 	: SetShowPremiumBuffUI
	@brief 		: �����̾� ���� ǥ�� ����
	@param		: bool
	@return		: ����
*/
bool CX2PremiumBuffManager::SetShowPremiumBuffUI( IN bool bShow_ )
{
/*
	switch( g_pMain->GetNowStateID() )
	{
	case CX2Main::XS_VILLAGE_MAP:
	case CX2Main::XS_DUNGEON_GAME:
	//case CX2Main::XS_SERVER_SELECT:
		// ������ ������ ��쿡�� ���� �κ��� ����
		break;

	default:
		// �׿��� State������ �׳� return
		return false;
		break;
	}

	CX2StateMenu* pStateMenu = static_cast<CX2StateMenu*>( g_pMain->GetNowState() );

	if ( NULL == pStateMenu )
		return false;
	
	if ( pStateMenu->SetShowPremiumBuffUI( bShow_ ) )
	{		
		m_bPremiumBuffUI = bShow_;
		return true;
	}
	else
	{
		m_bPremiumBuffUI = bShow_;
		return false;
	}
*/

	return false;
}
#pragma endregion SetShowPremiumBuffUI

#pragma region GetViewBuffChat
/** @function 	: GetViewBuffChat
	@brief 		: ä��â ���� ���� ����
	@return		: bool
*/
bool CX2PremiumBuffManager::GetViewBuffChat() const
{
	return m_bChatViewFirst;
}
#pragma endregion GetViewBuffChat

#pragma region SetViewBuffChat
/** @function 	: SetViewBuffChat
	@brief 		: ä��â ���� ���� ����
	@param		: bool
*/
void CX2PremiumBuffManager::SetViewBuffChat( IN bool bShow_ )
{
	m_bChatViewFirst = bShow_;
}
#pragma endregion SetViewBuffChat

#pragma region IsComeBackUserInParty
/** @function 	: IsComeBackUserInParty
	@brief 		: ��Ƽ �߿� �޸� ���� ������ �ִ��� Ȯ��
	@return		: bool
*/
bool CX2PremiumBuffManager::IsComeBackUserInParty() const
{
	return m_bComeBackuserInParty;
}
#pragma endregion IsComeBackUserInParty

#pragma region SetComeBackUserInParty
/** @function 	: SetComeBackUserInParty
	@brief 		: ��Ƽ �߿� �޸� ���� ������ �ִ��� Ȯ��
	@param		: bool
*/
void CX2PremiumBuffManager::SetComeBackUserInParty( IN bool bShow_ )
{
	m_bComeBackuserInParty = bShow_;
}
#pragma endregion SetComeBackUserInParty

#pragma region UpdateGuideDescPremiumBuffUI
/** @function 	: UpdateGuideDescPremiumBuffUI
	@brief 		: ���� ������ ���
*/
void CX2PremiumBuffManager::UpdateGuideDescPremiumBuffUI()
{
	switch( g_pMain->GetNowStateID() )
	{
	case CX2Main::XS_VILLAGE_MAP:
	case CX2Main::XS_DUNGEON_GAME:
		// ������ ������ ��쿡�� ���� �κ��� ����
		break;

	default:
		// �׿��� State������ �׳� return
		return;
		break;
	}

#ifdef CHECK_INHERIT_STATE_MENU
	if( g_pMain->IsInheritStateMenu() )
#endif //CHECK_INHERIT_STATE_MENU
	{
		CX2StateMenu* pStateMenu = static_cast<CX2StateMenu*>( g_pMain->GetNowState() );

		if ( NULL == pStateMenu )
			return;
		else
			pStateMenu->UpdateGuideDescPremiumBuffUI();
	}
}
#pragma endregion UpdateGuideDescPremiumBuffUI

#pragma region UICustomEventProc
/** @function 	: UICustomEventProc
	@brief 		: UI �̺�Ʈ
	@param		: procedure
	@return		: bool
*/
bool CX2PremiumBuffManager::UICustomEventProc( IN HWND hWnd, IN UINT uMsg, IN WPARAM wParam, IN LPARAM lParam )
{
	switch ( wParam )
	{
	case SCBUCM_WELCOME_OK:
		{
			if ( NULL != m_pDLGWelcomeComeBackUser )
			{
				m_bFirstComeBack = false;
				m_pDLGWelcomeComeBackUser->SetShowEnable( false, false );
			}
		} break;
	case SCBUCM_COME_BACK_BUFF_DESC_OK:
		{
			if ( NULL != m_pDLGComeBackUserBuffDesc )
			{
				m_bDescComplete = true;
				m_bFirstComeBackBuffDesc = false;
				m_pDLGComeBackUserBuffDesc->SetShowEnable( false, false );
#ifdef SERV_EVENT_RETURN_USER_MARK
				//�������� ���̵�� �̺�Ʈ ������ ����

	#ifdef CHECK_INHERIT_STATE_MENU
				if( g_pMain->IsInheritStateMenu() )
	#endif //CHECK_INHERIT_STATE_MENU
				{
					CX2StateMenu* pStateMenu = (CX2StateMenu*) g_pMain->GetNowState();
					pStateMenu->OpenElliosGuide(110);
				}
#endif //SERV_EVENT_RETURN_USER_MARK
			}
		} break;
	}

	return false;
}
#pragma endregion UICustomEventProc

#pragma region UIServerEventProc
/** @function 	: UIServerEventProc
	@brief 		: ���� �̺�Ʈ
	@param		: procedure
	@return		: bool
*/
bool CX2PremiumBuffManager::UIServerEventProc( IN HWND hWnd, IN UINT uMsg, IN WPARAM wParam, IN LPARAM lParam )
{
	KTDXPROFILE();
	switch( wParam )
	{
	case EGS_COME_BACK_USER_NOT:
		{
			return Handler_EGS_COME_BACK_USER_NOT( hWnd, uMsg, wParam, lParam );
		} break;

	case EGS_COME_BACK_BUFF_NOT:
		{
			return Handler_EGS_COME_BACK_BUFF_NOT( hWnd, uMsg, wParam, lParam );
		} break;

#ifdef BUFF_TEMPLET_SYSTEM	
	case EGS_UPDATE_BUFF_INFO_NOT:
		{
			return Handler_EGS_UPDATE_BUFF_INFO_NOT( hWnd, uMsg, wParam, lParam );
		} break;

	case EGS_UPDATE_BUFF_INFO_IN_ROOM_NOT:
		{
			return Handler_EGS_UPDATE_BUFF_INFO_IN_ROOM_NOT( hWnd, uMsg, wParam, lParam );
		} break;
#endif BUFF_TEMPLET_SYSTEM

	default:
		break;
	}
	return false;
}
#pragma endregion UIServerEventProc

#pragma region Handler_EGS_COME_BACK_USER_NOT
/** @function 	: Handler_EGS_COME_BACK_USER_NOT
	@brief 		: �޸� ���� ������ ó�� �������� �� Dialog ǥ�ø� ���ؼ� ���
	@param		: HWND, UINT, WPARAM, LPARAM
	@return		: bool
*/
bool CX2PremiumBuffManager::Handler_EGS_COME_BACK_USER_NOT( IN HWND hWnd, IN UINT uMsg, IN WPARAM wParam, IN LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_COME_BACK_USER_NOT kEvent;
	DeSerialize( pBuff, &kEvent );

	if ( true == kEvent )
	{
		if ( g_pMain->GetNowStateID() == CX2Main::XS_SERVER_SELECT )
		{
			SetFirstComeBack( true );

			m_pDLGWelcomeComeBackUser = new CKTDGUIDialog( g_pMain->GetNowState(), L"DLG_UI_Return_User_Popup_Small.lua" );
			g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGWelcomeComeBackUser );
			m_pDLGWelcomeComeBackUser->SetShowEnable( false, false );

			m_pDLGComeBackUserBuffDesc = new CKTDGUIDialog( g_pMain->GetNowState(), L"DLG_UI_Return_User_Popup_Big.lua" );
			g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGComeBackUserBuffDesc );
			m_pDLGComeBackUserBuffDesc->SetShowEnable( false, false );
		}
	}
	else
	{
		SetFirstComeBack( false );
	}
	
	return true;
}
#pragma endregion Handler_EGS_COME_BACK_USER_NOT

#ifdef BUFF_TEMPLET_SYSTEM


/** @function : Handler_EGS_UPDATE_BUFF_INFO_IN_ROOM_NOT
	@brief : �� �ȿ� �ִ� �������� �ɸ��Ϳ� ���� ���� ���� ó�� ( ����, ����, �ʵ� )
	@param : �������� ������ ���� ������( ������Ѿ� �� ���� ��� ) ( kBuffInfo )
	@return : true
*/
bool CX2PremiumBuffManager::Handler_EGS_UPDATE_BUFF_INFO_IN_ROOM_NOT( IN HWND hWnd, IN UINT uMsg, IN WPARAM wParam, IN LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_UPDATE_BUFF_INFO_IN_ROOM_NOT kEvent;
	DeSerialize( pBuff, &kEvent );

	if ( NULL != g_pX2Game )
	{
		CX2GUUser* pCX2GUUser = g_pX2Game->GetUserUnitByUID( kEvent.m_iUnitUID );

		if ( NULL != pCX2GUUser )
		{
			CX2Unit* pCX2Unit = pCX2GUUser->GetUnit();

			if( NULL != pCX2Unit )
			{
				vector<CX2BuffFactorPtr>& vecWorldBuffFactorPtr = pCX2Unit->GetWorldBuffFactorPtr();

				BOOST_FOREACH( CX2BuffFactorPtr WorldBuffFactorPtr, vecWorldBuffFactorPtr )			/// ��� ���� ���� ����
				{
					pCX2GUUser->EraseBuffTempletFromGameUnit( WorldBuffFactorPtr->GetBuffTempletID() );
				}

				vecWorldBuffFactorPtr.clear();	/// ���� ���� ���� �����̳� �ʱ�ȭ

				BOOST_FOREACH( KBuffInfo& kBuffInfo, kEvent.m_vecActivateBuffList )			/// �������� �� ���� ����
				{
					UINT			 iFactorID	   = kBuffInfo.m_kFactorInfo.m_iFactorID;	/// Factor ID
					CX2BuffFactorPtr ptrBuffFactor = CX2BuffTempletManager::GetInstance()->GetBuffFactorPtr( kBuffInfo.m_kFactorInfo.m_iFactorID );

					if ( NULL != ptrBuffFactor )
					{
						CX2BuffFactorPtr ptrBuffFactorClone = ptrBuffFactor->GetClonePtr();	/// Ŭ�� ���� ���ͷ� ����

						if ( NULL != ptrBuffFactorClone )
						{
							pCX2GUUser->SetBuffFactorToGameUnitByBuffFactorID( static_cast<UINT>( kBuffInfo.m_kFactorInfo.m_iFactorID ) );	/// ���� ����
							vecWorldBuffFactorPtr.push_back( ptrBuffFactorClone );	/// ���� ���� ����
						}
					}
				}
			}
		}
	}

	return true;
}

/** @function : Handler_EGS_UPDATE_BUFF_INFO_NOT
	@brief : ���� �ƴ� ���� �ִ� �ڽ��� �ɸ��Ϳ� ���� ���� ���� ������ ó�� ( ���� )
	@param : �������� ������ ���� ������( ������Ѿ� �� ���� ��� ) ( kBuffInfo )
	@return : true
*/
bool CX2PremiumBuffManager::Handler_EGS_UPDATE_BUFF_INFO_NOT( IN HWND hWnd, IN UINT uMsg, IN WPARAM wParam, IN LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_UPDATE_BUFF_INFO_NOT kEvent;
	DeSerialize( pBuff, &kEvent );

	if( NULL != g_pX2Game )		/// �� �ȿ� �ִµ� �ش� ��Ŷ�� ���ƿ���, ���� �ʽ�Ŵ
		return true;

	if(  NULL != g_pData && NULL != g_pData->GetMyUser() )
	{
		CX2Unit* pCX2Unit = g_pData->GetMyUser()->GetSelectUnit();

		if( NULL != pCX2Unit )
		{
			vector<CX2BuffFactorPtr>& vecWorldBuffFactorPtr = pCX2Unit->GetWorldBuffFactorPtr();

			BOOST_FOREACH( CX2BuffFactorPtr WorldBuffFactorPtr, vecWorldBuffFactorPtr )				/// ��� ���� ���� ������ ����
			{
				CX2BuffTempletPtr ptrBuffTemplet = CX2BuffTempletManager::GetInstance()->GetBuffTempletPtr( WorldBuffFactorPtr->GetBuffTempletID() );

#ifdef BUFF_ICON_UI
				if( NULL != ptrBuffTemplet && true == ptrBuffTemplet->GetUseBuffIcon() )
				{
					CX2GageManager::GetInstance()->EraseBuff( pCX2Unit->GetUID(), ptrBuffTemplet->GetBuffTempletID(), ptrBuffTemplet->IsDeBuff() );
				}
#endif // BUFF_ICON_UI
			}

			vecWorldBuffFactorPtr.clear();	/// ���� ���� ���� �����̳� �ʱ�ȭ

			BOOST_FOREACH( KBuffInfo& kBuffInfo, kEvent.m_vecActivateBuffList )				/// �������� �� ���� ������ ����
			{
				UINT			 iFactorID	   = kBuffInfo.m_kFactorInfo.m_iFactorID;		/// Factor ID
				CX2BuffFactorPtr ptrBuffFactor = CX2BuffTempletManager::GetInstance()->GetBuffFactorPtr( kBuffInfo.m_kFactorInfo.m_iFactorID );

				if ( NULL != ptrBuffFactor )
				{
					CX2BuffFactorPtr ptrBuffFactorClone = ptrBuffFactor->GetClonePtr();		/// Ŭ�� ���� ���ͷ� ����

					if ( NULL != ptrBuffFactorClone )
					{
						CX2BuffTempletPtr ptrBuffTemplet	= CX2BuffTempletManager::GetInstance()->GetBuffTempletPtr( ptrBuffFactor->GetBuffTempletID() );

#ifdef BUFF_ICON_UI
						if ( NULL != ptrBuffTemplet )
						{
							if( true == ptrBuffTemplet->GetUseBuffIcon() )
							{
#ifdef SERV_IRUHADEV_BUFF_DURATION_TEXT
								CX2GageManager::GetInstance()->PushBuff( pCX2Unit->GetUID(), CX2GageUI::BuffIcon( ptrBuffTemplet->GetBuffTempletID(),ptrBuffTemplet->GetIconFileName(),
									ptrBuffTemplet->GetIconKeyName() , ptrBuffTemplet->GetBuffName(), ptrBuffTemplet->GetBuffDesc(), ptrBuffTemplet->GetRemainDurationTime() ), ptrBuffTemplet->IsDeBuff() );	/// ���� ������ ����
#else
								CX2GageManager::GetInstance()->PushBuff( pCX2Unit->GetUID(), CX2GageUI::BuffIcon( ptrBuffTemplet->GetBuffTempletID(),ptrBuffTemplet->GetIconFileName(),
									ptrBuffTemplet->GetIconKeyName() , ptrBuffTemplet->GetBuffName(), ptrBuffTemplet->GetBuffDesc() ), ptrBuffTemplet->IsDeBuff() );	/// ���� ������ ����
#endif //SERV_IRUHADEV_BUFF_DURATION_TEXT
							}
						}
#endif // BUFF_ICON_UI

						vecWorldBuffFactorPtr.push_back( ptrBuffFactorClone );	/// ���� ���� ����

					}
				}
			}
	#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
			/// ������ ���Ͽ� �߰��� ��ų ���� �缳��
			if ( NULL != pCX2Unit->GetUnitData() )
				pCX2Unit->ResetIncreaseSkillLevelByBuff();
	#endif // UPGRADE_SKILL_SYSTEM_2013

		}
	}

	return true;
}
#endif BUFF_TEMPLET_SYSTEM

#pragma region Handler_EGS_COME_BACK_BUFF_NOT
/** @function 	: Handler_EGS_COME_BACK_BUFF_NOT
	@brief 		: �޸� ���� ������ ���
	@param		: HWND, UINT, WPARAM, LPARAM
	@return		: bool
*/
bool CX2PremiumBuffManager::Handler_EGS_COME_BACK_BUFF_NOT( IN HWND hWnd, IN UINT uMsg, IN WPARAM wParam, IN LPARAM lParam )
{
	KSerBuffer* pBuff = (KSerBuffer*)lParam;
	KEGS_COME_BACK_BUFF_NOT kEvent;
	DeSerialize( pBuff, &kEvent );

	if ( false == kEvent.m_bIsFinish )
	{
		if ( g_pMain->GetNowStateID() == CX2Main::XS_SERVER_SELECT )
		{
			SetComeBackUser( true );
			SetShowPremiumBuffUI( true );
			SetViewBuffChat( true );

			m_iComeBackBuffEnd = kEvent.m_iComeBackBuffEnd;
		}
	}
	else
	{
		SetComeBackUser( false );
	} 

	return true;
}
#pragma endregion Handler_EGS_COME_BACK_BUFF_NOT

#pragma region OnFrameMove
HRESULT CX2PremiumBuffManager::OnFrameMove( IN double fTime, IN float fElapsedTime )
{
	//if ( true == IsComeBackUser() || IsComeBackUserInParty() )
	{
		{
			if ( false == GetShowPremiumBuffUI() )
			{
				// NPC�� ��ȭ �ߵ� �ƴϰ�, �ŷ� ���嵵 �ƴϸ�
				if ( ( NULL != g_pTFieldGame && false == g_pTFieldGame->GetJoinNpc() ) &&
					( NULL == g_pSquareGame ) )
					SetShowPremiumBuffUI( true );
			}
			else
			{
				// NPC�� ��ȭ���̰ų�, �ŷ� �����̸�
				if ( ( NULL != g_pTFieldGame && true == g_pTFieldGame->GetJoinNpc() ) ||
					( NULL != g_pSquareGame ) )
					SetShowPremiumBuffUI( false );
				else
					SetShowPremiumBuffUI( true );

				UpdateGuideDescPremiumBuffUI();		
			}
		}		
	}
	//else
	//{
	//	SetShowPremiumBuffUI( false );
	//}

#ifdef BUFF_TEMPLET_SYSTEM

#ifdef CHECK_INHERIT_STATE_MENU
	if( g_pMain->IsInheritStateMenu() )
#endif //CHECK_INHERIT_STATE_MENU
	{
		CX2StateMenu* pStateMenu = static_cast<CX2StateMenu*>( g_pMain->GetNowState() );

		switch( g_pMain->GetNowStateID() )
		{
		case CX2Main::XS_VILLAGE_MAP:
		case CX2Main::XS_DUNGEON_GAME:
		case CX2Main::XS_SQUARE_GAME:
			{
				pStateMenu->SetShowBuffTempletIcon( true );

				if( NULL != g_pData && NULL != g_pData->GetCashShop() )		//ĳ�ü��� ���� ���� ��, ������ ǥ�ø� ���´�.
				{
					if( true == g_pData->GetCashShop()->GetOpen() )
						pStateMenu->SetShowBuffTempletIcon( false );
				}

				// NPC�� ��ȭ���� ��,
				if ( NULL != g_pTFieldGame && true == g_pTFieldGame->GetJoinNpc() )
				{
					pStateMenu->SetShowBuffTempletIcon( false );
				}
			}break;
		default:
			{
				if( GetShowBuffTempletUI() != false )
					pStateMenu->SetShowBuffTempletIcon( false );
			}break;
		}
	}
#endif BUFF_TEMPLET_SYSTEM

	return S_OK;
}
#pragma endregion OnFrameMove


#endif

#ifdef BUFF_TEMPLET_SYSTEM	
void CX2PremiumBuffManager::InsertPremiumBuff( const CX2PremiumBuffTemplet::BUFF_ID ePremiumBuffID_, wstring wstrEndTime )
{
	const WCHAR* pFileName	= NULL;
	const WCHAR* pKeyName	= NULL;
	bool		 bUsingIcon = NULL; //���� ������ ��� ����

	//if ( false == GetFileAndKeyName( ePremiumBuffID_, &pFileName, &pKeyName ) )
	if ( false == GetFileAndKeyName( ePremiumBuffID_, &pFileName, &pKeyName, &bUsingIcon ) )
		return;

	UINT uiIndex = m_vecPremiumBuffList.size();

	CX2PremiumBuffIconPtr ptrPremiumBuff = CX2PremiumBuffIcon::CreatePremiumBuff( ePremiumBuffID_, uiIndex, pFileName, pKeyName, wstrEndTime, bUsingIcon );

	m_vecPremiumBuffList.push_back( ptrPremiumBuff );
}


void CX2PremiumBuffManager::ErasePremiumBuff( const CX2PremiumBuffTemplet::BUFF_ID ePremiumBuffID_ )
{
	if ( false == m_vecPremiumBuffList.empty() )
	{
		auto funcFindBuff = [ePremiumBuffID_]( CX2PremiumBuffIconPtr val_ ) {
			return ( val_->GetPremiumBuffID() == ePremiumBuffID_ );
		};

		vector<CX2PremiumBuffIconPtr>::iterator vItr = find_if( m_vecPremiumBuffList.begin(), m_vecPremiumBuffList.end(), funcFindBuff );

		if ( m_vecPremiumBuffList.end() != vItr )
		{
			m_vecPremiumBuffList.erase( vItr );
			UpdatePremiumBuffIconPosition();
		}
	}
}


bool CX2PremiumBuffManager::GetFileAndKeyName( IN const CX2PremiumBuffTemplet::BUFF_ID ePremiumBuffID_, OUT const WCHAR** ppFileName_, OUT const WCHAR** ppKeyName_, OUT bool* bUsingIcon_ )
{
	switch ( ePremiumBuffID_ )
	{
	case CX2PremiumBuffTemplet::BI_GATHERING_OF_HERO:
		{
			*ppFileName_	= L"DLG_ui_button18.tga";
			*ppKeyName_	= L"PC_BANG";
		} break;

	case CX2PremiumBuffTemplet::BI_REST_FOR_REGROUP:
		{
			*ppFileName_	= L"DLG_ui_button18.tga";
			*ppKeyName_	= L"blood";
		} break;

	case CX2PremiumBuffTemplet::BI_WARM_EL_AURA:
		{
			*ppFileName_	= L"DLG_ui_button18.tga";
			*ppKeyName_	= L"BUBBLE_EVENT";
		} break;

	case CX2PremiumBuffTemplet::BI_PROTECTION_OF_NUT:			// ��Ʈ�� ��ȣ   ( ��� )
		{
			*ppFileName_	= L"DLG_ui_button20.tga";
			*ppKeyName_	= L"NUTE";
		} break;

	case CX2PremiumBuffTemplet::BI_PROTECTION_OF_PUTA:			// ��Ÿ�� ��ȣ   ( ��� )
		{
			*ppFileName_	= L"DLG_ui_button20.tga";
			*ppKeyName_	= L"PTA";
		} break;

	case CX2PremiumBuffTemplet::BI_PROTECTION_OF_LAHELL:		// ������ ��ȣ   ( ��� )
		{
			*ppFileName_	= L"DLG_ui_button20.tga";
			*ppKeyName_	= L"RAHEL";
		} break;

	case CX2PremiumBuffTemplet::BI_PROTECTION_OF_JAKIELL:		// ��Ű���� ��ȣ ( ��� )
		{
			*ppFileName_	= L"DLG_ui_button20.tga";
			*ppKeyName_	= L"JAKIEL";
		} break;

	case CX2PremiumBuffTemplet::BI_BELSSING_OF_SERAPHIM:		// �������� �ູ	 ( ���� )
		{
			*ppFileName_	= L"DLG_ui_button20.tga";
			*ppKeyName_	= L"SERAPHIM";
		} break;

	case CX2PremiumBuffTemplet::BI_BELSSING_OF_GEV:				// �Ժ��� �ູ	 ( ���� )
		{
			*ppFileName_	= L"DLG_ui_button20.tga";
			*ppKeyName_	= L"GEV";
		} break;

	case CX2PremiumBuffTemplet::BI_BELSSING_OF_AMON:			// �Ƹ��� �ູ	 ( ���� )
		{
			*ppFileName_	= L"DLG_ui_button20.tga";
			*ppKeyName_	= L"AMON";
		} break;
		
	case CX2PremiumBuffTemplet::BI_BELSSING_OF_CRONOS:			// ũ�γ뽺�� �ູ ( ���� )
		{
			*ppFileName_	= L"DLG_ui_button20.tga";
			*ppKeyName_	= L"CRONOS";
		} break;

	case CX2PremiumBuffTemplet::BI_SAVE_CRYSTAL_BUFF_LEVEL_1:	// ����� �� ���� ���� ���� Level 1
		{
			*ppFileName_	= L"DLG_ui_button20.tga";
			*ppKeyName_	= L"HERO";
		} break;

	case CX2PremiumBuffTemplet::BI_SAVE_CRYSTAL_BUFF_LEVEL_2:	// ����� �� ���� ���� ���� Level 2
		{
			*ppFileName_	= L"DLG_ui_button20.tga";
			*ppKeyName_	= L"HERO";
		} break;

	case CX2PremiumBuffTemplet::BI_SAVE_CRYSTAL_BUFF_LEVEL_3:	// ����� �� ���� ���� ���� Level 3
		{
			*ppFileName_	= L"DLG_ui_button20.tga";
			*ppKeyName_	= L"HERO";
		} break;

	case CX2PremiumBuffTemplet::BI_SAVE_CRYSTAL_BUFF_LEVEL_4:	// ����� �� ���� ���� ���� Level 4
		{
			*ppFileName_	= L"DLG_ui_button20.tga";
			*ppKeyName_	= L"HERO";
		} break;
	case CX2PremiumBuffTemplet::BI_STRENGTHENING_BODY:			// ��ȭ�� ��ü ( ��� ��� ���� ����� )
		{
			*ppFileName_	= L"DLG_ui_button20.tga";
			*ppKeyName_	= L"DEBUFF";
		} break;

	default:
		{
			//���� �������� ���� ������ ���, �������� ǥ������ �ʴ´�. ( �� : ���� ���� ���� )
			*ppFileName_	= NULL;
			*ppKeyName_		= NULL;
			*bUsingIcon_	= false;	//���� ������ �̻��

			return false;
		} break;
	}
	*bUsingIcon_ = true;				//���� ������ �����
	return true;
}

void CX2PremiumBuffManager::UpdatePremiumBuffIconPosition()
{
	if ( false == m_vecPremiumBuffList.empty() )
	{
		UINT uiNewIndex = 0;
		vector<CX2PremiumBuffIconPtr>::iterator vItr = m_vecPremiumBuffList.begin();
		while ( m_vecPremiumBuffList.end() != vItr )
		{
			if ( (*vItr)->GetIndex() != uiNewIndex )
				(*vItr)->ChangeBuffIconPosition( uiNewIndex );
				

			++uiNewIndex;
			++vItr;
		}
	}
}

void CX2PremiumBuffManager::AddBuffInfo_LUA()
{
	KLuaManager luaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState() );
	//TableBind( &luaManager, g_pKTDXApp->GetLuaBinder() );

	CX2PremiumBuffTempletPtr ptrPremiumBuffTemplet = CX2PremiumBuffTemplet::CreatePremiumBuffTemplet();

	if ( ptrPremiumBuffTemplet->ParsingScript( luaManager ) )
		m_mapPremiumBuffTempletMap.insert( make_pair( ptrPremiumBuffTemplet->GetPremiumBuffID(), ptrPremiumBuffTemplet ) );
	else
	{
		ASSERT( !L"BuffTemplet Parsing Error" );
		StateLog( L"BuffTemplet Parsing Error" );
	}
}

void CX2PremiumBuffManager::OpenScriptFile( const char* szScriptFileName_ )
{
	lua_tinker::decl( g_pKTDXApp->GetLuaBinder()->GetLuaState(),  "BuffManager", this );

	KGCMassFileManager::CMassFile::MASSFILE_MEMBERFILEINFO_POINTER Info;
	Info = g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LoadDataFile( szScriptFileName_ );

	if ( NULL == Info )
	{
		ASSERT( !"LoadDataFile doesn't work!" );
		ErrorLogMsg( XEM_ERROR1, szScriptFileName_ );
		//return false;
	}

	if ( g_pKTDXApp->GetLuaBinder()->DoMemory( Info->pRealData, Info->size ) == E_FAIL )
	{
		ASSERT( !"DoMemory doesn't work!" );
		ErrorLogMsg( XEM_ERROR2, szScriptFileName_ );
		//return false;
	}
}

CX2PremiumBuffTempletPtr CX2PremiumBuffManager::GetPremiumBuffTempletPtr( const CX2PremiumBuffTemplet::BUFF_ID ePremiumBuffID_ )
{
	PremiumBuffTempletMap::iterator mItr = m_mapPremiumBuffTempletMap.find( ePremiumBuffID_ );

	if ( m_mapPremiumBuffTempletMap.end() != mItr)
		return mItr->second;
	else
		return CX2PremiumBuffTempletPtr();
}

void CX2PremiumBuffManager::GetSocketOptions( OUT vector<int>& vecBuffSocetOptions_ )
{
	BOOST_FOREACH( CX2PremiumBuffIconPtr ptrValue, m_vecPremiumBuffList )
	{
		CX2PremiumBuffTempletPtr ptrTemplet = GetPremiumBuffTempletPtr( ptrValue->GetPremiumBuffID() );

		const vector<int>& vecSocketOptionsPerBuff = ptrTemplet->GetSocketOptionsList();
		vecBuffSocetOptions_.insert( vecBuffSocetOptions_.end(), vecSocketOptionsPerBuff.begin(), vecSocketOptionsPerBuff.end() );
	}
}

void CX2PremiumBuffManager::GetSocketOptions( IN const vector<int>& vecBuffIdList_, OUT vector<int>& vecBuffSocetOptions_ )
{
	BOOST_FOREACH( const int& iBuffId, vecBuffIdList_ )
	{
		CX2PremiumBuffTempletPtr ptrTemplet = GetPremiumBuffTempletPtr( static_cast<CX2PremiumBuffTemplet::BUFF_ID>( iBuffId ) );

		const vector<int>& vecSocketOptionsPerBuff = ptrTemplet->GetSocketOptionsList();
		vecBuffSocetOptions_.insert( vecBuffSocetOptions_.end(), vecSocketOptionsPerBuff.begin(), vecSocketOptionsPerBuff.end() );
	}
}
/* �ʵ忡�� ������� ���� ������ ��ǥ
const float				CX2PremiumBuffIcon::POSITION_X		= 264.0f;
const float				CX2PremiumBuffIcon::POSITION_Y		= 686.0f;
*/
const float				CX2PremiumBuffIcon::POSITION_X		= 254.0f;
const float				CX2PremiumBuffIcon::POSITION_Y		= 686.0f;

CX2PremiumBuffIcon::~CX2PremiumBuffIcon()
{
	SAFE_DELETE_DIALOG( m_pDlgBuffIcon );
}

/*static*/ CX2PremiumBuffIconPtr CX2PremiumBuffIcon::CreatePremiumBuff( const CX2PremiumBuffTemplet::BUFF_ID ePremiumBuffID_, const UINT uiIndex_, 
	const WCHAR* pFileName_, const WCHAR* pKeyName_, wstring wstrEndTime, bool m_bUsingIcon )
{
	CX2PremiumBuffIconPtr ptrPremiumBuff( new CX2PremiumBuffIcon( ePremiumBuffID_, uiIndex_, wstrEndTime, m_bUsingIcon ) );
	ptrPremiumBuff->CreateBuffIcon( pFileName_, pKeyName_ );

	return ptrPremiumBuff;
}

void CX2PremiumBuffIcon::CreateBuffIcon( const WCHAR* pFileName_, const WCHAR* pKeyName_ )
{
/*
	m_pDlgBuffIcon = new CKTDGUIDialog( NULL, L"DLG_UI_Buff_Icon.lua" );	//�ӽ� ����

	CKTDGUIButton* pButton = static_cast<CKTDGUIButton*>( m_pDlgBuffIcon->GetControl( L"Button_Buff_Icon" ) );	//�ӽ� ����
	if ( NULL != pButton )
	{
		pButton->SetNormalTex( pFileName_, pKeyName_ );  
		pButton->SetOverTex( pFileName_, pKeyName_ );
		pButton->SetDownTex( pFileName_, pKeyName_ );
		pButton->SetEnable( true );

		if( g_pData->GetPremiumBuffManager()->GetShowBuffTempletUI() == true )
			pButton->SetShow( true );
		else
			pButton->SetShow( false );

		UpdateBuffIconPosition( GetIndex() );

		g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDlgBuffIcon );
	}*/

}

void CX2PremiumBuffIcon::UpdateBuffIconPosition( const UINT uiIndex_ )
{
	UINT uiColumnIndex	= uiIndex_ / NUMBER_OF_ROW;
	UINT uiRowIndex		= uiIndex_ % NUMBER_OF_ROW;

	float fx = static_cast<float>( uiRowIndex * SIZE_OF_ICON );
	float fy = static_cast<float>( uiColumnIndex * SIZE_OF_ICON );

	m_pDlgBuffIcon->SetPos( D3DXVECTOR2( POSITION_X + fx, POSITION_Y - fy ) );
	//pButton_->MoveControl( fx, fy );
}

void CX2PremiumBuffIcon::ChangeBuffIconPosition( const UINT uiNewIndex_ )
{
	if ( NULL == m_pDlgBuffIcon )
		return;

	if ( uiNewIndex_ != GetIndex() )
	{
		SetIndex( uiNewIndex_ );
		UpdateBuffIconPosition( GetIndex() );
	}
}

void CX2PremiumBuffIcon::SetBuffDesc()
{
	CKTDGUIButton* pButton = static_cast<CKTDGUIButton*>( m_pDlgBuffIcon->GetControl( L"Button_Buff_Icon" ) );
	if ( NULL != pButton )
	{
// 		wstring wstrGuideDesc;
// 
// 		g_pData->GetPremiumBuffManager()->
// 		pButton->SetGuideDesc( )
	}
}

bool CX2PremiumBuffTemplet::ParsingScript( IN KLuaManager& luaManager_ )
{
	LUA_GET_VALUE_ENUM( luaManager_,	 "m_eBuffID",		    m_ePremiumBuffID,		     BUFF_ID,		 BI_NONE );

	LUA_GET_VALUE(		luaManager_,	 "m_strBuffName",	    m_uiStringID,			     STR_ID_EMPTY );

	LUA_GET_VALUE(		luaManager_,	 "m_fBuffPeriodSecond", m_fPremiumBuffPeriodSecond,  0			  );

	if ( luaManager_.BeginTable( "USING_BUFF_ROOM_TYPE" ) )
	{
		int iIndex = 1;
		CX2PremiumBuffTemplet::USING_ROOM_TYPE eUsingRoomType = CX2PremiumBuffTemplet::URT_NONE;

		int iTemp = -1;
		while ( luaManager_.GetValue( iIndex, iTemp ) ) { 
			eUsingRoomType = static_cast<CX2PremiumBuffTemplet::USING_ROOM_TYPE>(iTemp);
			m_vUsingRoomType.push_back( eUsingRoomType );
			++iIndex;
		}
		luaManager_.EndTable();
	}

	if ( luaManager_.BeginTable( "SOCKET_OPTION" ) )
	{
		int iIndex = 1;
		int iSocketOption = 0;

		while ( luaManager_.GetValue( iIndex, iSocketOption ) ) {
			m_vecSocketOptions.push_back( iSocketOption );
			++iIndex;
			iSocketOption = 0; // �ʱ�ȭ
		}
		luaManager_.EndTable();
	}

	return true;
}
#endif BUFF_TEMPLET_SYSTEM

#ifdef SERV_PC_BANG_TYPE
bool CX2PremiumBuffManager::IsPcBangAvatar()
{
	return m_kPcBangReward.m_bAvatar;
}

void CX2PremiumBuffManager::SetPcBangReward( KPcBangReward kPcBangReward )
{
	m_kPcBangReward = kPcBangReward;
}

wstring CX2PremiumBuffManager::GetPcBangBuffString()
{
	wstring wstrPcBangBuff;

	switch( m_kPcBangReward.m_iPcBangType )
	{
	case 0:
		// �����̾� PC�� ����
		wstrPcBangBuff = GET_STRING( STR_ID_20839 );
		break;
	case 1:
		// �����̾� PC�� ����
		wstrPcBangBuff = GET_STRING( STR_ID_23944 );
		break;
	case 2:
		// �����̾� PC�� ����++
		wstrPcBangBuff = GET_STRING( STR_ID_23945 );
		break;
	case 3:
		// �����̾� PC�� ����++
		wstrPcBangBuff = GET_STRING( STR_ID_23946 );
		break;
	default:
		return L"";
	}
	wstrPcBangBuff += L"\n";

	// STR_ID_20840
	// �ټ��� ������, ����ġ +20%\nPC�� �κ��丮(���� �ƹ�Ÿ) ��� ����\nPC�� ���� Īȣ ��� ����\n��д��� �Ϲ� ������ ����\n��ϸ��� �ð� ���� Ƚ�� +2ȸ �߰�

	// �ټ��� ������ 
	if( m_kPcBangReward.m_bSpiritFree == true )
	{
		wstrPcBangBuff += GET_STRING( STR_ID_23935 );
		wstrPcBangBuff += _GetLastString( wstrPcBangBuff );
	}

	// ����ġ +@1%
	if( m_kPcBangReward.m_fEXPRate > 0.0f )
	{
		wstrPcBangBuff += GET_REPLACED_STRING( ( STR_ID_23936, "f", m_kPcBangReward.m_fEXPRate * 100.0f ) );
		wstrPcBangBuff += _GetLastString( wstrPcBangBuff );
	}

	// EDȹ�淮 +@1%
	if( m_kPcBangReward.m_fEDRate > 0.0f )
	{
		wstrPcBangBuff += GET_REPLACED_STRING( ( STR_ID_23942, "f", m_kPcBangReward.m_fEDRate * 100.0f ) );
		wstrPcBangBuff += _GetLastString( wstrPcBangBuff );
	}

	// APȹ�淮 +@1%
	if( m_kPcBangReward.m_fVSPointRate > 0.0f )
	{
		wstrPcBangBuff += GET_REPLACED_STRING( ( STR_ID_23943, "f", m_kPcBangReward.m_fVSPointRate * 100.0f ) );
		wstrPcBangBuff += _GetLastString( wstrPcBangBuff );
	}

	// �� ����ġ +@1%
	if( m_kPcBangReward.m_fPetEXPRate > 0.0f )
	{
		wstrPcBangBuff += GET_REPLACED_STRING( ( STR_ID_23941, "f", m_kPcBangReward.m_fPetEXPRate * 100.0f ) );
		wstrPcBangBuff += _GetLastString( wstrPcBangBuff );
	}

	// PC�� �κ��丮(���� �ƹ�Ÿ) ��� ����
	if( m_kPcBangReward.m_bAvatar == true )
	{
		wstrPcBangBuff += GET_STRING( STR_ID_23937 );
		wstrPcBangBuff += _GetLastString( wstrPcBangBuff );
	}

	// PC�� ���� Īȣ ��� ����
	if( m_kPcBangReward.m_bTitle == true )
	{
		wstrPcBangBuff += GET_STRING( STR_ID_23938 );
		wstrPcBangBuff += _GetLastString( wstrPcBangBuff );
	}

	// ��ϸ��� �ð� ���� Ƚ�� +@1ȸ �߰�
	if( m_kPcBangReward.m_iAdditionalHenirReward > 0 )
	{
		wstrPcBangBuff += GET_REPLACED_STRING( ( STR_ID_23940, "i", m_kPcBangReward.m_iAdditionalHenirReward ) );
		wstrPcBangBuff += _GetLastString( wstrPcBangBuff );
	}

	return wstrPcBangBuff;
}
#endif SERV_PC_BANG_TYPE
