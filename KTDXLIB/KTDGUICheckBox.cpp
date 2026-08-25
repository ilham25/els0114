#include "StdAfx.h"
#include ".\ktdguicheckbox.h"



CKTDGUICheckBox::CKTDGUICheckBox()
: m_bPressed( false )
{
	m_ControlType	= CKTDGUIControl::UCT_CHECKBOX;
	m_CheckBoxState	= CBCS_NORMAL;
	m_bChecked		= false;
	m_bEmptyBGWhenChecked	= false;

	m_pNormalTex	= NULL;
	m_pMouseOverTex	= NULL;
	m_pDisableTex	= NULL;
	m_pCheckedTex	= NULL;

	m_pBGPoint		= NULL;
	m_pCheckedPoint	= NULL;
	m_pSndCheck		= NULL;

	m_CustomMsgChecked		= -1;
	m_CustomMsgUnChecked	= -1;

	m_CustomFuncChecked		= L"";
	m_CustomFuncUnChecked	= L"";


//{{ robobeg : 2008-10-28
	//KLuaManager kLuaManager;
    KLuaManager kLuaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState(), 0, true );
//}} robobeg : 2008-10-28

	if(  g_pKTDXApp->GetDeviceManager()->LoadLuaManager( &kLuaManager, L"UI_Control_Sound.lua" ) == false )
	{
		return;
	}

	string checkSndFileName;

	LUA_GET_VALUE( kLuaManager, "CheckBox_Check", checkSndFileName, "" );

	wstring sndFileName;

	ConvertCharToWCHAR( sndFileName, checkSndFileName.c_str() );
	m_pSndCheck = g_pKTDXApp->GetDeviceManager()->OpenSound( sndFileName );
}

CKTDGUICheckBox::~CKTDGUICheckBox(void)
{
	SAFE_DELETE( m_pNormalTex );
	SAFE_DELETE( m_pMouseOverTex );
	SAFE_DELETE( m_pDisableTex );
	SAFE_DELETE( m_pCheckedTex );

	SAFE_DELETE( m_pBGPoint );
	SAFE_DELETE( m_pCheckedPoint );

	SAFE_CLOSE( m_pSndCheck );
}

HRESULT CKTDGUICheckBox::OnFrameMove( double fTime, float fElapsedTime )
{
	KTDXPROFILE();
	if( m_bShow == false )
		return S_OK;

	CKTDGUIControl::OnFrameMove( fTime, fElapsedTime );

	m_fElapsedTime	= fElapsedTime;
	if( m_EndPoint.fChangeTime != m_NowPoint.fChangeTime )
	{
		ChangePoint( &m_NowPoint, &m_EndPoint );
	}

	switch( m_CheckBoxState )
	{
		case CBCS_NORMAL:
			{
				if( m_bMouseOver == true )
					ChangeState( CBCS_MOUSEOVER );
				else if( m_bEnable == false )
					ChangeState( CBCS_DISABLE );
			}
			break;

		case CBCS_MOUSEOVER:
			{
				if( m_bMouseOver == false )
					ChangeState( CBCS_NORMAL );
				else if( m_bEnable == false )
					ChangeState( CBCS_DISABLE );
			}
			break;

		case CBCS_DISABLE:
			{
				if( m_bEnable == true )
					ChangeState( CBCS_NORMAL );
			}
			break;
	}

	return S_OK;
}

HRESULT CKTDGUICheckBox::OnFrameRender()
{
	KTDXPROFILE();
	HRESULT hr	= S_OK;

	if( m_bShow == false )
		return hr;


	if( false == m_bEmptyBGWhenChecked || false == m_bChecked )
	{
		//BG Draw
		UpdateVertex( m_Vertex[CKTDGUIControl::VP_LEFT_TOP], 
			m_Vertex[CKTDGUIControl::VP_RIGHT_TOP], 
			m_Vertex[CKTDGUIControl::VP_LEFT_BOTTOM], 
			m_Vertex[CKTDGUIControl::VP_RIGHT_BOTTOM],
			m_NowPoint );

		RenderVertex( m_Vertex, m_NowPoint );
	}

	//Checked Mark Draw
	if( m_bChecked == true )
	{
		UpdateVertex( m_Vertex[CKTDGUIControl::VP_LEFT_TOP], 
					m_Vertex[CKTDGUIControl::VP_RIGHT_TOP], 
					m_Vertex[CKTDGUIControl::VP_LEFT_BOTTOM], 
					m_Vertex[CKTDGUIControl::VP_RIGHT_BOTTOM],
					*m_pCheckedPoint );

		RenderVertex( m_Vertex, *m_pCheckedPoint );
	}

	return hr;
}

//--------------------------------------------------------------------------------------
bool CKTDGUICheckBox::HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam )
{
	if( m_bEnable == false || m_bShow == false )
		return false;

	switch( uMsg )
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		{
			if( ContainsPoint( pt ) )
			{
				// Pressed while inside the control
				m_bPressed = true;
				SetCapture( DXUTGetHWND() );

				if( !m_bHasFocus )
					m_pDialog->RequestFocus( this );

				return true;
			}

			break;
		}

	case WM_LBUTTONUP:
		{
			if( m_bPressed )
			{
				m_bPressed = false;
				ReleaseCapture();

				// Button click
				if( ContainsPoint( pt ) )
					SetCheckedInternal( !m_bChecked, true );

				return true;
			}

			break;
		}
	};

	return false;
}

void CKTDGUICheckBox::ChangeState( CHECKBOX_CONTROL_STATE state )
{	
	if( m_CheckBoxState == state )
		return;

	m_CheckBoxState = state;

	m_NowPoint = m_EndPoint = *m_pBGPoint;

	switch( m_CheckBoxState )
	{
		case CBCS_NORMAL:
			{
				m_NowPoint.pUITextureData	= m_pNormalTex;		
			}
			break;

		case CBCS_MOUSEOVER:
			{
				if( m_pMouseOverTex != NULL )
					m_NowPoint.pUITextureData	= m_pMouseOverTex;
				else
					m_NowPoint.pUITextureData	= m_pNormalTex;
			}
			break;

		case CBCS_DISABLE:
			{
				if( m_pDisableTex != NULL )
					m_NowPoint.pUITextureData	= m_pDisableTex;
				else
					m_NowPoint.pUITextureData	= m_pNormalTex;

				if( m_bHasFocus == true )
					m_pDialog->ClearFocus();
			}
			break;
	}
}

void CKTDGUICheckBox::SetNormalTex_LUA( const char* pFileName, const char* key )
{
	SAFE_DELETE( m_pNormalTex );

	wstring wstrFileName;
	ConvertUtf8ToWCHAR( wstrFileName, pFileName );

	wstring wstrKeyName;
	ConvertUtf8ToWCHAR( wstrKeyName, key );

	m_pNormalTex = SetTexureData_( wstrFileName.c_str(), wstrKeyName.c_str() );
	if( m_pBGPoint != NULL )
		m_pBGPoint->pUITextureData = m_pNormalTex;
}

void CKTDGUICheckBox::SetOverTex_LUA( const char* pFileName, const char* key )
{
	SAFE_DELETE( m_pMouseOverTex );

	wstring wstrFileName;
	ConvertUtf8ToWCHAR( wstrFileName, pFileName );

	wstring wstrKeyName;
	ConvertUtf8ToWCHAR( wstrKeyName, key );

	m_pMouseOverTex = SetTexureData_( wstrFileName.c_str(), wstrKeyName.c_str() );
}

void CKTDGUICheckBox::SetDisableTex_LUA( const char* pFileName, const char* key )
{
	SAFE_DELETE( m_pDisableTex );
	wstring wstrFileName;
	ConvertUtf8ToWCHAR( wstrFileName, pFileName );

	wstring wstrKeyName;
	ConvertUtf8ToWCHAR( wstrKeyName, key );

	m_pDisableTex = SetTexureData_( wstrFileName.c_str(), wstrKeyName.c_str() );
}

void CKTDGUICheckBox::SetCheckedTex_LUA( const char* pFileName, const char* key )
{
	SAFE_DELETE( m_pCheckedTex );
	wstring wstrFileName;
	ConvertUtf8ToWCHAR( wstrFileName, pFileName );

	wstring wstrKeyName;
	ConvertUtf8ToWCHAR( wstrKeyName, key );

	m_pCheckedTex = SetTexureData_( wstrFileName.c_str(), wstrKeyName.c_str() );
	if( m_pCheckedPoint != NULL )
		m_pCheckedPoint->pUITextureData = m_pCheckedTex;
}

void CKTDGUICheckBox::SetBGPoint_LUA()
{
	SAFE_DELETE( m_pBGPoint );
	m_pBGPoint = SetPointData_();
	m_pBGPoint->pUITextureData = m_pNormalTex;
	m_pBGPoint->SetAutoPointByTextureSize();
	m_NowPoint = *m_pBGPoint;
}

void CKTDGUICheckBox::SetCheckPoint_LUA()
{
	SAFE_DELETE( m_pCheckedPoint );
	m_pCheckedPoint = SetPointData_();
	m_pCheckedPoint->pUITextureData = m_pCheckedTex;
	m_pCheckedPoint->SetAutoPointByTextureSize();
}

void CKTDGUICheckBox::SetCheckedInternal( bool bChecked, bool bFromInput /*= false*/ )
{ 
	m_bChecked = bChecked; 

	if( true == bFromInput )
	{
		if( m_bChecked == true )
		{
			if( m_CustomMsgChecked != -1 )
				SendInternelEvent( g_pKTDXApp->GetHWND(), CKTDXApp::KM_UI_CONTROL_CUSTOM_EVENT, m_CustomMsgChecked, (LPARAM)this );
			else
				SendInternelEvent( g_pKTDXApp->GetHWND(), CKTDXApp::KM_UI_CONTROL_EVENT, CBEM_CHECKBOX_CHECKED, (LPARAM)this );

			if ( m_CustomFuncChecked != L"" )
				SendInternelEvent( g_pKTDXApp->GetHWND(), CKTDXApp::KM_UI_CONTROL_CUSTOM_FUNC, (WPARAM)m_CustomFuncChecked.c_str(), (LPARAM)this );

		}
		else
		{
			if( m_CustomMsgUnChecked != -1 )
				SendInternelEvent( g_pKTDXApp->GetHWND(), CKTDXApp::KM_UI_CONTROL_CUSTOM_EVENT, m_CustomMsgUnChecked, (LPARAM)this );
			else
				SendInternelEvent( g_pKTDXApp->GetHWND(), CKTDXApp::KM_UI_CONTROL_EVENT, CBEM_CHECKBOX_UNCHECKED, (LPARAM)this );

			if ( m_CustomFuncChecked != L"" )
				SendInternelEvent( g_pKTDXApp->GetHWND(), CKTDXApp::KM_UI_CONTROL_CUSTOM_FUNC, (WPARAM)m_CustomFuncUnChecked.c_str(), (LPARAM)this );
		}

		PlaySound( m_pSndCheck );
	}
}
