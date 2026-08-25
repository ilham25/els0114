#pragma once

class CKTDGUIStatic : public CKTDGUIControl
{
	public:
		CKTDGUIStatic();

		virtual	~CKTDGUIStatic(void);

		virtual	HRESULT	OnFrameMove( double fTime, float fElapsedTime );
		virtual	HRESULT	OnFrameRender();

		bool HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam );

		bool CanHaveFocus();

		void AddPicture( CKTDGUIControl::CPictureData* pPicture ){ m_PictureDataList.push_back(pPicture); }
		void AddString( CKTDGUIControl::UIStringData* pString ){ m_StringDataList.push_back(pString); }

		void AddPictureAt( int iIdx, CKTDGUIControl::CPictureData* pPicture );
		void AddStringAt( int iIdx, CKTDGUIControl::UIStringData* pString );
	
		void AddString_LUA();

		bool ContainsPoint(POINT pt);


		CKTDGUIControl::UIStringData* GetString( int index );
		CKTDGUIControl::CPictureData* GetPicture( int index );
        CKTDGUIControl::CPictureData* GetPictureIndex( int index );

		void SetString( int index, const WCHAR* wszMsg );

		int	GetPictureNum() { return (int)m_PictureDataList.size(); }
		int	GetStringNum() { return (int)m_StringDataList.size(); }

//{{ kimhc // 2010.12.18 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
		void RemovePictureByIndex( int iIndex_ );
#endif	NEW_CHARACTER_CHUNG
//}} kimhc // 2010.12.18 //  2010-12-23 New Character CHUNG

		void RemoveAllPicture();
		void RemoveAllString();

		// Maximize / Minimize
		virtual void SetMaximize( bool val );


		void SetRoundBoxPosSize( D3DXVECTOR2 vPos, D3DXVECTOR2 vRectSize, D3DXVECTOR2 vRoundSize, CKTDGUIStatic::CPictureData** apPictureData );


		virtual void	MoveControl( float fx, float fy );
		virtual void	ScaleControl( float fx, float fy );;

		void SetCustomMsgMouseDblClk( int msg ){ m_CustomMsgMouseDblClk = msg; } // static 더블클릭
		void SetCustomMsgMouseUp(int msg) { m_CustomMsgMouseUp = msg; }

	protected:

		void DrawEdge( bool bDrawOut, int edgeWidth, D3DXCOLOR edgeColor, CKTDGUIControl::UIPointData* pPoint );
		
		vector<CKTDGUIControl::CPictureData*>	m_PictureDataList;
		vector<CKTDGUIControl::UIStringData*>	m_StringDataList;

		CKTDGUIControl::VERTEX_UI				m_Vertex[4];

		CKTDXDeviceTexture*						m_pEdgeTexture;

		int										m_CustomMsgMouseDblClk;
		int										m_CustomMsgMouseUp;
};
