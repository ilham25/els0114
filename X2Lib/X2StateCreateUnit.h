#pragma once

class CX2StateCreateUnit : public CX2State
{
	public:
		friend class CX2StateAutoChanger;

		enum STATE_CREATE_UNIT_UI_CUSTOM_MSG
		{
			SCUUCM_ELSWORD_BUTTON_OVER = 0,
			SCUUCM_ELSWORD_BUTTON_UP,

			SCUUCM_LIRE_BUTTON_OVER,
			SCUUCM_LIRE_BUTTON_UP,

			SCUUCM_ARME_BUTTON_OVER,
			SCUUCM_ARME_BUTTON_UP,

			SCUUCM_RAVEN_BUTTON_OVER,
			SCUUCM_RAVEN_BUTTON_UP,

			SCUUCM_CREATE_UNIT,
			SCUUCM_BACK_TO_UNIT_SELECT,

			SCUUCM_UNIT_ANIMATION_DOWN,
			SCUUCM_UNIT_ANIMATION_UP,
			SCUUCM_UNIT_ANIMATION_OVER,

			SCUUCM_ENTER_TUTORIAL_OK,
			SCUUCM_ENTER_TUTORIAL_CANCEL,

			SCUUCM_EVE_BUTTON_OVER,
			SCUUCM_EVE_BUTTON_UP,

			SCUUCM_CHUNG_BUTTON_OVER,
			SCUUCM_CHUNG_BUTTON_UP,

			SCUUCM_ARA_BUTTON_OVER, // 사용 안 함
			SCUUCM_ARA_BUTTON_UP,

			SCUUCM_SHUT_DOWN_OK,

			SCUUCM_ELESIS_BUTTON_UP,
		};
		
	public:
		CX2StateCreateUnit(void);
		virtual ~CX2StateCreateUnit(void);

		virtual HRESULT OnFrameMove( double fTime, float fElapsedTime );
		virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	protected:
		virtual bool UICustomEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
		virtual bool UIServerEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

		bool Handler_EGS_CREATE_UNIT_REQ();
		bool Handler_EGS_CREATE_UNIT_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );



		void UnSelect();
		void SelectElSword();
		void SelectLire();
		void SelectArme();
		void SelectRaven();
		void SelectEve();

//{{ kimhc // 2010.11.24 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
		void SelectChung();
#endif	NEW_CHARACTER_CHUNG
//}} kimhc // 2010.11.24 //  2010-12-23 New Character CHUNG

#ifdef ARA_CHARACTER_BASE
		void SelectAra();
#endif
#ifdef NEW_CHARACTER_EL
		void SelectElesis();
#endif // NEW_CHARACTER_EL

		bool CreateErrorCheck();

		void InitDummyUnits();
		bool ReadyUnitResources( CX2Unit* pUnit );

		void ReadyPromotionUnitResources();
		void EquipPromotionItem( CX2Unit::UNIT_CLASS eUnitClass );

		
		void StaticControlAnimate();

		void OpenNickNameBox();
		void SetAlarmAboutName( const WCHAR* pContent );
		
		virtual bool LastMsgByESC();
		virtual void LastMsgPopOk();

		//{{ kimhc // 2010.11.30 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
		UCHAR GetPageNowUnitButton() const { return m_ucPageNowUnitButton; }
		void SetPageNowUnitButton(UCHAR ucPageNowUnitButton_) { m_ucPageNowUnitButton = ucPageNowUnitButton_; }

		UCHAR GetPageMaxUnitButton() const { return m_ucPageMaxUnitButton; }
		void SetPageMaxUnitButton(UCHAR ucPageMaxUnitButton_) { m_ucPageMaxUnitButton = ucPageMaxUnitButton_; }
#endif	NEW_CHARACTER_CHUNG
		//}} kimhc // 2010.11.30 //  2010-12-23 New Character CHUNG

		void InitializeUnitButton();
		void SetUnitButton( int iPageNow_ );


	protected:
		CKTDGUIDialogType			m_pDLGCreateUnitBack;
		CKTDGUIDialogType			m_pDLGCreateUnitFront;

		CX2Unit::UNIT_CLASS		m_SelectUnitClass;
		wstring					m_NickName;
		UidType					m_UnitUID;

		CX2Unit*				m_pDummyElsword;
		CX2Unit*				m_pDummyArme;
		CX2Unit*				m_pDummyLire;
		CX2Unit*				m_pDummyRaven;
		CX2Unit*				m_pDummyEve;

//{{ kimhc // 2010.11.24 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
		CX2Unit*				m_pDummyChung;
#endif	NEW_CHARACTER_CHUNG
//}} kimhc // 2010.11.24 //  2010-12-23 New Character CHUNG

#ifdef ARA_CHARACTER_BASE
		CX2Unit*				m_pDummyAra;		/// "아라" 더미
#endif

#ifdef NEW_CHARACTER_EL
		CX2Unit*				m_pDummyEL;
#endif // NEW_CHARACTER_EL


		CX2UnitViewerUI*		m_pUnitViewerUI;

		bool					m_bSelectUnit;		//2d캐릭터 클릭한거.. 

		float					m_fMouseSensitivity;

		float					m_fRemainRotY;
		bool					m_bRotateReturn;
		float					m_fRotSpeed;

		CKTDGUIDialogType			m_pDLGMSGOkBoxEnterTutorial;

		//{{ kimhc // 2010.11.16 // 신캐릭터 청
#ifdef	NEW_CHARACTER_CHUNG

		UCHAR					m_ucPageNowUnitButton;		/// 캐릭터 선택 버튼의 현재 페이지
		UCHAR					m_ucPageMaxUnitButton;		/// 캐릭터 선택 버튼의 최대 페이지
		
#endif	NEW_CHARACTER_CHUNG
		//}} kimhc // 2010.11.16 // 신캐릭터 청

};
