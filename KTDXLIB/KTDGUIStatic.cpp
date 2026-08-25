#include "StdAfx.h"
#include ".\ktdguistatic.h"

CKTDGUIStatic::CKTDGUIStatic()
{
	m_CustomMsgMouseDblClk	= -1;
	m_CustomMsgMouseUp		= -1;

	m_ControlType	= CKTDGUIControl::UCT_STATIC;
	SetEnable( false );

	m_pEdgeTexture = g_pKTDXApp->GetDeviceManager()->OpenTexture( L"UIEdge.tga" );
}

CKTDGUIStatic::~CKTDGUIStatic(void)
{
	for( int i = 0; i < (int)m_PictureDataList.size(); i++ )
	{		
		CKTDGUIControl::CPictureData* pPictureData = m_PictureDataList[i];
		SAFE_DELETE( pPictureData );
	}
	m_PictureDataList.clear();
	for( int i = 0; i < (int)m_StringDataList.size(); i++ )
	{
		CKTDGUIControl::UIStringData* stringData = m_StringDataList[i];
		SAFE_DELETE( stringData );
	}
	m_StringDataList.clear();

	SAFE_CLOSE( m_pEdgeTexture );
}

HRESULT CKTDGUIStatic::OnFrameMove( double fTime, float fElapsedTime )
{
	KTDXPROFILE();
	if( m_bShow == false )
		return S_OK;

	
	
	KTDXPROFILE_BEGIN("control_framemove");
	CKTDGUIControl::OnFrameMove( fTime, fElapsedTime );
	KTDXPROFILE_END();



	KTDXPROFILE_BEGIN("string_framemove");
	for( UINT i=0; i<m_StringDataList.size(); i++ )
	{
		CKTDGUIControl::UIStringData* pStringData = m_StringDataList[i];
		if( NULL != pStringData )
		{
			if( true == pStringData->bSpread )
			{
				pStringData->OnFrameMove( fTime, fElapsedTime );
			}
		}
	}
	KTDXPROFILE_END();


	KTDXPROFILE_BEGIN("picture_framemove");
	for( int i = 0; i < (int)m_PictureDataList.size(); i++ )
	{		
		CKTDGUIControl::CPictureData* pPictureData = m_PictureDataList[i];
		if( NULL != pPictureData )
		{
			if( true == pPictureData->m_bTwinkle ||
				true == pPictureData->m_bFlicker ||
#ifdef BUFF_ICON_UI
				true == pPictureData->m_bFadeIn ||
				true == pPictureData->m_bFadeOut ||
#endif //BUFF_ICON_UI
				0.f != pPictureData->m_VibrateData.m_fMaxTime )
			{
				pPictureData->OnFrameMove( fTime, fElapsedTime );
			}
		}
	}
	KTDXPROFILE_END();


	m_fElapsedTime = fElapsedTime;

	return S_OK;
}

HRESULT CKTDGUIStatic::OnFrameRender()
{
	KTDXPROFILE();
	if( m_bShow == false )
		return S_OK;

	for( int i = 0; i < (int)m_PictureDataList.size(); i++ )
	{
		KTDXPROFILE_BEGIN("Picture");
		CKTDGUIControl::CPictureData* pPictureData = m_PictureDataList[i];

		if ( pPictureData->GetShow() == true 
			&& pPictureData->pPoint != NULL 
			&& pPictureData->pTexture != NULL 
			&& pPictureData->pTexture->pTexture != NULL )
		{
			if ( pPictureData->m_EdgeWidth != 0 )
				DrawEdge( pPictureData->m_bDrawEdgeOut, pPictureData->m_EdgeWidth, pPictureData->m_EdgeColor, pPictureData->pPoint );

			UpdateVertex( m_Vertex[VP_LEFT_TOP], m_Vertex[VP_RIGHT_TOP],
											m_Vertex[VP_LEFT_BOTTOM], m_Vertex[VP_RIGHT_BOTTOM],
											*pPictureData->pPoint );
			

            KD3DBEGIN()
			    CKTDGStateManager::PushRenderState( D3DRS_SRCBLEND,			pPictureData->pPoint->m_eSrcBlend );
			    CKTDGStateManager::PushRenderState( D3DRS_DESTBLEND,		pPictureData->pPoint->m_eDestBlend );
			    RenderVertex( m_Vertex, *pPictureData->pPoint, pPictureData->pPoint->m_iDrawCount );
            KD3DEND()

			if ( pPictureData->m_EdgeWidth != 0 )
				DrawEdge( pPictureData->m_bDrawEdgeOut, pPictureData->m_EdgeWidth, pPictureData->m_EdgeColor, pPictureData->pPoint );
		}
		KTDXPROFILE_END();
	}

	for( int i = 0; i < (int)m_StringDataList.size(); i++ )
	{
		KTDXPROFILE_BEGIN("Text");
		CKTDGUIControl::UIStringData* pStringData = m_StringDataList[i];
		if( pStringData->msg.size() <= 0 )
			continue;

		if ( pStringData->bUseUkFont == true )
		{
			CKTDGFontManager::CUKFont* pFont = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetUKFont( pStringData->fontIndex );
			D3DXVECTOR2 pos;
			D3DXCOLOR	tempColor;
			D3DXCOLOR	tempOutLineColor;

			tempColor.a = pStringData->color.a * m_pDialog->GetColor().a * m_Color.a;
			tempColor.r = pStringData->color.r * m_pDialog->GetColor().r * m_Color.r;
			tempColor.g = pStringData->color.g * m_pDialog->GetColor().g * m_Color.g;
			tempColor.b = pStringData->color.b * m_pDialog->GetColor().b * m_Color.b;

			tempOutLineColor.a = pStringData->outlineColor.a * m_pDialog->GetColor().a * m_Color.a; 
			tempOutLineColor.r = pStringData->outlineColor.r * m_pDialog->GetColor().r * m_Color.r;
			tempOutLineColor.g = pStringData->outlineColor.g * m_pDialog->GetColor().g * m_Color.g;
			tempOutLineColor.b = pStringData->outlineColor.b * m_pDialog->GetColor().b * m_Color.b;

			pos = pStringData->pos + m_pDialog->GetPos() + m_OffsetPos;		
			//pFont->Draw2DText( pStringData->msg.c_str(), (int)pos.x, (int)pos.y, tempColor, tempOutLineColor, pStringData->sortFlag, pStringData->fontStyle );

			if( true == pStringData->bSpread )
			{
				wstring spreadMsg = pStringData->msg.substr( 0, pStringData->iSpreadCount );
				pFont->OutTextMultiline( (int)pos.x, (int)pos.y, spreadMsg.c_str(), tempColor, pStringData->fontStyle, tempOutLineColor, 1.0f, NULL, pStringData->sortFlag );
			}
			else
			{
				pFont->OutTextMultiline( (int)pos.x, (int)pos.y, pStringData->msg.c_str(), tempColor, pStringData->fontStyle, tempOutLineColor, 1.0f, NULL, pStringData->sortFlag );
			}
		}
		else
		{
#ifdef KTDGDEVICEFONT_SIMULATE_DIRECTX_FONT
			CKTDGFontManager::CUKFont* pFont = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetMappedUKFont( pStringData->fontIndex );
#else
			CKTDGFontManager::CKTDGFont* pFont = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetFont( pStringData->fontIndex );
#endif
			D3DXVECTOR2 pos;
			D3DXCOLOR	tempColor;
			D3DXCOLOR	tempOutLineColor;

			tempColor.a = pStringData->color.a * m_pDialog->GetColor().a * m_Color.a;
			tempColor.r = pStringData->color.r * m_pDialog->GetColor().r * m_Color.r;
			tempColor.g = pStringData->color.g * m_pDialog->GetColor().g * m_Color.g;
			tempColor.b = pStringData->color.b * m_pDialog->GetColor().b * m_Color.b;

			tempOutLineColor.a = pStringData->outlineColor.a * m_pDialog->GetColor().a * m_Color.a; 
			tempOutLineColor.r = pStringData->outlineColor.r * m_pDialog->GetColor().r * m_Color.r;
			tempOutLineColor.g = pStringData->outlineColor.g * m_pDialog->GetColor().g * m_Color.g;
			tempOutLineColor.b = pStringData->outlineColor.b * m_pDialog->GetColor().b * m_Color.b;

			pos = pStringData->pos + m_pDialog->GetPos() + m_OffsetPos;		
			//pFont->Draw2DText( pStringData->msg.c_str(), (int)pos.x, (int)pos.y, tempColor, tempOutLineColor, pStringData->sortFlag, pStringData->fontStyle );

			if( true == pStringData->bSpread )
			{
				wstring spreadMsg = pStringData->msg.substr( 0, pStringData->iSpreadCount );
				//pFont->OutTextMultiline( (int)pos.x, (int)pos.y, spreadMsg.c_str(), tempColor, pStringData->fontStyle, tempOutLineColor, 1.0f, NULL, pStringData->sortFlag );
				pFont->Draw2DText( spreadMsg.c_str(), (int)pos.x, (int)pos.y, tempColor, tempOutLineColor, pStringData->sortFlag, pStringData->fontStyle, (int)spreadMsg.size() );
			}
			else
			{
				//pFont->OutTextMultiline( (int)pos.x, (int)pos.y, pStringData->msg.c_str(), tempColor, pStringData->fontStyle, tempOutLineColor, 1.0f, NULL, pStringData->sortFlag );
				pFont->Draw2DText( pStringData->msg.c_str(), (int)pos.x, (int)pos.y, tempColor, tempOutLineColor, pStringData->sortFlag, pStringData->fontStyle, (int)pStringData->msg.size() );
			}
		}


		
		KTDXPROFILE_END();
	}

	return S_OK;
}


bool CKTDGUIStatic::HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam )
{
	KTDXPROFILE();

	if( m_bEnable == false || m_bShow == false )
		return false;

	switch( uMsg )
	{
		case WM_LBUTTONDOWN:		
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONUP:
			{
				if( ContainsPoint( pt ) == true )
				{
					// 스태틱일 경우, 특별한 메세지 처리를 하지 않는 경우
					// 메세지를 통과 시킴

					//return false;

					if ( uMsg == WM_LBUTTONDOWN )
					{
						if ( m_pDialog != NULL )
							m_pDialog->SetMouseDownForDrag( true );
					}
					else if( uMsg == WM_LBUTTONUP )
					{
						if( m_CustomMsgMouseUp != -1 )
							SendInternelEvent( g_pKTDXApp->GetHWND(), CKTDXApp::KM_UI_CONTROL_CUSTOM_EVENT, m_CustomMsgMouseUp, (LPARAM)this );

						if ( m_pDialog != NULL )
							m_pDialog->SetMouseDownForDrag( false );
					}	
					else if ( uMsg != WM_MOUSEMOVE )
					{
						if ( m_pDialog != NULL )
							m_pDialog->SetMouseDownForDrag( false );
					}
										

					return true;
				}
			}
			break;

		case WM_LBUTTONDBLCLK: // static 더블클릭					
			{
				// Button click
				if( ContainsPoint( pt ) == true )
				{
					if( m_CustomMsgMouseDblClk != -1 )
					{
						SendInternelEvent( g_pKTDXApp->GetHWND(), CKTDXApp::KM_UI_CONTROL_CUSTOM_EVENT, m_CustomMsgMouseDblClk, (LPARAM)this );
						return true;
					}
					//PlaySound( m_pSndMouseUp );				
				}

			}
			break;
	};

	return false;
}

bool CKTDGUIStatic::ContainsPoint( POINT pt )
{
	for( int i = 0; i < (int)m_PictureDataList.size(); i++ )
	{
		CKTDGUIControl::CPictureData* pPictureData = m_PictureDataList[i];
		
		if ( pPictureData != NULL && pPictureData->pPoint != NULL && 
			Pick2DRect( pt, 
			pPictureData->pPoint->leftTopPoint, pPictureData->pPoint->rightTopPoint, 
			pPictureData->pPoint->leftBottomPoint, pPictureData->pPoint->rightBottomPoint ) == true )
		{
			return true;
		}
		/*
		if ( pPictureData != NULL && 
		Pick2DRect( pt, 
			D3DXVECTOR2( pPictureData->GetPos().x, pPictureData->GetPos().y ), 
			D3DXVECTOR2( pPictureData->GetPos().x + pPictureData->GetSize().x, pPictureData->GetPos().y ), 
			D3DXVECTOR2( pPictureData->GetPos().x, pPictureData->GetPos().y + pPictureData->GetSize().y ), 
			D3DXVECTOR2( pPictureData->GetPos().x + pPictureData->GetSize().x, pPictureData->GetPos().y + pPictureData->GetSize().y ) ) == true )
		{
			return true;
		}
		*/
	}

	return false;
}

void CKTDGUIStatic::AddPictureAt( int iIdx, CKTDGUIControl::CPictureData* pPicture )
{
	vector<CKTDGUIControl::CPictureData*>::iterator iV = m_PictureDataList.begin();

	std::advance( iV, iIdx );
	m_PictureDataList.insert( iV, pPicture );
}

void CKTDGUIStatic::AddStringAt( int iIdx, CKTDGUIControl::UIStringData* pString )
{
	vector<CKTDGUIControl::UIStringData*>::iterator iV = m_StringDataList.begin();

	std::advance( iV, iIdx );
	m_StringDataList.insert( iV, pString );
}

bool CKTDGUIStatic::CanHaveFocus()
{
	if( m_bShow == true )
		return true;
	else
		return false;
}

void CKTDGUIStatic::AddString_LUA()
{
	CKTDGUIControl::UIStringData* pString = SetStringData_();
	AddString( pString );
}

CKTDGUIControl::UIStringData* CKTDGUIStatic::GetString( int index )
{
	if( index < 0 || index >= (int)m_StringDataList.size() )
	{
		return NULL;
	}
	else
		return m_StringDataList[index];
}


CKTDGUIControl::CPictureData* CKTDGUIStatic::GetPictureIndex( int index )
{
    if( index <= 0)
	{
        return NULL;
	}

    CKTDGUIControl::CPictureData* pNodePicture;

    for(int i=0; i<(int)m_PictureDataList.size(); ++i)
    {
        pNodePicture = m_PictureDataList[i];
        if(pNodePicture != NULL && pNodePicture->GetIndex() == index)
        {
            return pNodePicture;
        }
    }

    return NULL;
}

CKTDGUIControl::CPictureData* CKTDGUIStatic::GetPicture( int index )
{
	if( index < 0 || index >= (int)m_PictureDataList.size() )
	{
		return NULL;
	}
	else
		return m_PictureDataList[index];
}

void CKTDGUIStatic::SetString( int index, const WCHAR* wszMsg )
{
	if ( GetString(index) != NULL )
	{
		GetString(index)->msg = wszMsg;
	}
}

//{{ kimhc // 2010.12.18 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
void CKTDGUIStatic::RemovePictureByIndex( int iIndex_ )
{
	if ( 0 > iIndex_ || m_PictureDataList.size() <= static_cast<UINT>( iIndex_ ) )
	{
		return;
	} // if
	
	CKTDGUIControl::CPictureData* pPictureData = m_PictureDataList[iIndex_];
	m_PictureDataList.erase( m_PictureDataList.begin() + iIndex_ );
	SAFE_DELETE( pPictureData );
}
#endif	NEW_CHARACTER_CHUNG
//}} kimhc // 2010.12.18 //  2010-12-23 New Character CHUNG

void CKTDGUIStatic::RemoveAllPicture()
{
	for( int i = 0; i < (int)m_PictureDataList.size(); i++ )
	{		
		CKTDGUIControl::CPictureData* pPictureData = m_PictureDataList[i];
		SAFE_DELETE( pPictureData );
	}
	m_PictureDataList.clear();
}

void CKTDGUIStatic::RemoveAllString()
{
	for( int i = 0; i < (int)m_StringDataList.size(); i++ )
	{
		CKTDGUIControl::UIStringData* stringData = m_StringDataList[i];
		SAFE_DELETE( stringData );
	}
	m_StringDataList.clear();
}

void CKTDGUIStatic::SetMaximize( bool val )
{
	CKTDGUIControl::SetMaximize( val );

	int iSign = 1;
	if ( val == false )
		iSign = -1;

	CKTDGUIControl::CPictureData** acPicData = &m_PictureDataList[0];
	for ( unsigned int iS = 0; iS < m_PictureDataList.size(); ++iS )
	{
		CKTDGUIControl::CPictureData* pcPicData = m_PictureDataList[iS];

		D3DXVECTOR2 kPos = pcPicData->GetPos();
		D3DXVECTOR2 kSize = pcPicData->GetSize();

		if ( pcPicData->bMaximizePos )
			pcPicData->SetPos( D3DXVECTOR2( kPos.x - m_kMaximizeOffset.x * iSign, kPos.y - m_kMaximizeOffset.y * iSign ) );

		if ( pcPicData->bMaximizeSize )
			pcPicData->SetSize( D3DXVECTOR2( kSize.x + m_kMaximizeOffset.x * iSign, kSize.y + m_kMaximizeOffset.y * iSign ) );
	}
}


void CKTDGUIStatic::DrawEdge( bool bDrawOut, int edgeWidth, D3DXCOLOR edgeColor, CKTDGUIControl::UIPointData* pPoint )
{
	if ( edgeWidth == 0 || m_pEdgeTexture == NULL )
		return;

	if ( pPoint == NULL )
		return;

	int _width = (int)(pPoint->rightTopPoint.x - pPoint->leftTopPoint.x);
	int _height = (int)(pPoint->rightBottomPoint.y - pPoint->leftTopPoint.y);

	D3DXCOLOR tempColor;

	tempColor.a = edgeColor.a * m_pDialog->GetColor().a * m_Color.a;
	tempColor.r = edgeColor.r * m_pDialog->GetColor().r * m_Color.r;
	tempColor.g = edgeColor.g * m_pDialog->GetColor().g * m_Color.g;
	tempColor.b = edgeColor.b * m_pDialog->GetColor().b * m_Color.b;

	if ( bDrawOut == true )
	{
		// 좌
		m_pEdgeTexture->Draw( (int)(m_pDialog->GetPos().x + m_OffsetPos.x + pPoint->leftTopPoint.x - edgeWidth), 
			(int)(m_pDialog->GetPos().y + m_OffsetPos.y + pPoint->leftTopPoint.y - edgeWidth), 
			edgeWidth , 
			_height + edgeWidth, 
			tempColor );

		// 하

		m_pEdgeTexture->Draw( (int)(m_pDialog->GetPos().x + m_OffsetPos.x + pPoint->leftTopPoint.x - edgeWidth), 
			(int)(m_pDialog->GetPos().y + m_OffsetPos.y + pPoint->leftBottomPoint.y ), 
			_width + edgeWidth, 
			edgeWidth, 
			tempColor );

		// 우

		m_pEdgeTexture->Draw( (int)(m_pDialog->GetPos().x + m_OffsetPos.x + pPoint->rightTopPoint.x ), 
			(int)(m_pDialog->GetPos().y + m_OffsetPos.y + pPoint->leftTopPoint.y ), 
			edgeWidth, 
			_height + edgeWidth, 
			tempColor );

		// 상

		m_pEdgeTexture->Draw( (int)(m_pDialog->GetPos().x + m_OffsetPos.x + pPoint->leftTopPoint.x ), 
			(int)(m_pDialog->GetPos().y + m_OffsetPos.y + pPoint->leftTopPoint.y - edgeWidth ), 
			_width + edgeWidth, 
			edgeWidth, 
			tempColor );
	}
	else
	{
		// 좌
		m_pEdgeTexture->Draw( (int)(m_pDialog->GetPos().x + m_OffsetPos.x + pPoint->leftTopPoint.x ), 
			(int)(m_pDialog->GetPos().y + m_OffsetPos.y + pPoint->leftTopPoint.y ), 
			edgeWidth , 
			_height, 
			tempColor );

		// 하

		m_pEdgeTexture->Draw( (int)(m_pDialog->GetPos().x + m_OffsetPos.x + pPoint->leftTopPoint.x), 
			(int)(m_pDialog->GetPos().y + m_OffsetPos.y + pPoint->leftBottomPoint.y - edgeWidth ), 
			_width, 
			edgeWidth, 
			tempColor );

		// 우

		m_pEdgeTexture->Draw( (int)(m_pDialog->GetPos().x + m_OffsetPos.x + pPoint->rightTopPoint.x - edgeWidth ), 
			(int)(m_pDialog->GetPos().y + m_OffsetPos.y + pPoint->leftTopPoint.y ), 
			edgeWidth, 
			_height, 
			tempColor );

		// 상

		m_pEdgeTexture->Draw( (int)(m_pDialog->GetPos().x + m_OffsetPos.x + pPoint->leftTopPoint.x ), 
			(int)(m_pDialog->GetPos().y + m_OffsetPos.y + pPoint->leftTopPoint.y ), 
			_width , 
			edgeWidth, 
			tempColor );

	}

}


void CKTDGUIStatic::SetRoundBoxPosSize( D3DXVECTOR2 vPos, D3DXVECTOR2 vRectSize, D3DXVECTOR2 vRoundSize, CKTDGUIStatic::CPictureData** apPictureData )
{
	CPictureData* pLeftTop		= apPictureData[0];
	CPictureData* pMiddleTop	= apPictureData[1];
	CPictureData* pRightTop		= apPictureData[2];

	CPictureData* pLeftMiddle	= apPictureData[3];
	CPictureData* pMiddleMiddle	= apPictureData[4];
	CPictureData* pRightMiddle	= apPictureData[5];

	CPictureData* pLeftBottom	= apPictureData[6];
	CPictureData* pMiddleBottom	= apPictureData[7];
	CPictureData* pRightBottom	= apPictureData[8];


	if( NULL != pLeftTop )
	{
		pLeftTop->SetPos( vPos - vRoundSize );
		pLeftTop->SetSize( vRoundSize );
	}

	if( NULL != pMiddleTop )
	{
		pMiddleTop->SetPos( vPos + D3DXVECTOR2( vRoundSize.x, 0 ) );
		pMiddleTop->SetSize( D3DXVECTOR2(vRectSize.x, vRoundSize.y ) );
	}

	if( NULL != pRightTop )
	{
		pRightTop->SetPos( vPos + D3DXVECTOR2( vRoundSize.x, 0) + D3DXVECTOR2( vRoundSize.x, 0 ) );
		pRightTop->SetSize( vRoundSize );
	}



	if( NULL != pLeftMiddle )
	{
		pLeftMiddle->SetPos( vPos - D3DXVECTOR2( vRoundSize.x, 0) );
		pLeftMiddle->SetSize( D3DXVECTOR2( vRoundSize.x, vRectSize.y ) );
	}	

	if( NULL != pMiddleMiddle )
	{
		pMiddleMiddle->SetPos( vPos );
		pMiddleMiddle->SetSize( vRectSize );
	}	

	if( NULL != pRightMiddle )
	{
		pRightMiddle->SetPos( vPos + D3DXVECTOR2( vRectSize.x, 0) );
		pRightMiddle->SetSize( D3DXVECTOR2( vRoundSize.x, vRectSize.y ) );
	}	



	if( NULL != pLeftBottom )
	{
		pLeftBottom->SetPos( vPos - D3DXVECTOR2( vRoundSize.x, 0) + D3DXVECTOR2( 0, vRectSize.y ) );
		pLeftBottom->SetSize( vRoundSize );
	}	

	if( NULL != pMiddleBottom )
	{
		pMiddleBottom->SetPos( vPos + D3DXVECTOR2( 0, vRectSize.y ) );
		pMiddleBottom->SetSize( D3DXVECTOR2(vRectSize.x, vRoundSize.y ) );
	}	

	if( NULL != pRightBottom )
	{
		pRightBottom->SetPos( vPos + vRectSize );
		pRightBottom->SetSize( vRoundSize );
	}	
	
}

/*virtual*/ void CKTDGUIStatic::MoveControl( float fx, float fy )
{
	if( fx == 0.f && fy == 0.f )
		return;



	for( UINT i=0; i<m_PictureDataList.size(); i++ )
	{
		CKTDGUIControl::CPictureData* pPicture = m_PictureDataList[i];
		if( NULL != pPicture &&
			NULL != pPicture->pPoint )
		{
			pPicture->pPoint->Move( fx, fy );
			pPicture->pos = pPicture->pPoint->leftTopPoint;
		}
	}

	//for( UINT i=0; i<m_StringDataList.size(); i++ )
	//{
	//	CKTDGUIControl::UIStringData* pStringData = m_StringDataList[i];
	//	if( NULL != pStringData )
	//	{
	//		pStringData->pos += D3DXVECTOR2( fx, fy );
	//	}
	//}

}

/*virtual*/ void CKTDGUIStatic::ScaleControl( float fx, float fy )
{
	if( fx == 1.f && fy == 1.f )
		return;

	for( UINT i=0; i<m_PictureDataList.size(); i++ )
	{
		CKTDGUIControl::CPictureData* pPicture = m_PictureDataList[i];
		if( NULL != pPicture &&
			NULL != pPicture->pPoint )
		{
			pPicture->pPoint->Scale( fx, fy );
			pPicture->pos = pPicture->pPoint->leftTopPoint;

			// fix!! size는 나중에
		}
	}
}