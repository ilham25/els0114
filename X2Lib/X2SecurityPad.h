#pragma once

#ifdef SERV_SECOND_SECURITY
class CX2SecurityPad
{
public:
	enum SECURITY_PAD_UI_MSG
	{
		SPUM_DIAL_NUMBER_0			= 49000,
		SPUM_DIAL_NUMBER_1,
		SPUM_DIAL_NUMBER_2,
		SPUM_DIAL_NUMBER_3,
		SPUM_DIAL_NUMBER_4,
		SPUM_DIAL_NUMBER_5,
		SPUM_DIAL_NUMBER_6,
		SPUM_DIAL_NUMBER_7,
		SPUM_DIAL_NUMBER_8,
		SPUM_DIAL_NUMBER_9,
		SPUM_DIAL_NUMBER_10,
		SPUM_DIAL_NUMBER_11,
		SPUM_EXIT,
		SPUM_OK,
		SPUM_CANCEL,
		SPUM_DELETE_PASSWORD_VERIFY_OK,
	};

	enum SECURITY_PAD_STATE
	{
		SPS_CLOSE,
		SPS_GAME_START,
		SPS_CREATE_PASSWORD,
		SPS_CHANGE_PASSWORD,
		SPS_RELEASE_PASSWORD,
	};


public:
	CX2SecurityPad();
	virtual ~CX2SecurityPad();

	bool GetShow(){ return m_bShow; }
	void SetShow(bool bShow, SECURITY_PAD_STATE eState = SPS_CLOSE);
	void ShuffleNumber();

	void ClearInputPassword();
	void ProcessPassword();
	void UpdateUIMarble();

	bool GetUseSecondPW(){ return m_bUseSecondPW; }
	void SetUseSecondPW(bool val){ m_bUseSecondPW = val; }

	HRESULT OnFrameMove( double fTime, float fElapsedTime );
	bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	bool UICustomEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	bool UIServerEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

#ifdef SERV_SECOND_SECURITY_K3R_AUTH
	bool Handler_ECH_SECURITY_AUTH_REQ();
	bool Handler_ECH_SECURITY_AUTH_ACK( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
#endif SERV_SECOND_SECURITY_K3R_AUTH

	bool Handler_EGS_AUTH_SECOND_SECURITY_REQ(); // Phase Count 1
	bool Handler_EGS_AUTH_SECOND_SECURITY_ACK(  HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	bool Handler_EGS_CREATE_SECOND_SECURITY_REQ(); // Phase Count 2
	bool Handler_EGS_CREATE_SECOND_SECURITY_ACK(  HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	bool Handler_EGS_DELETE_SECOND_SECURITY_REQ(); // Phase Count 1
	bool Handler_EGS_DELETE_SECOND_SECURITY_ACK(  HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	bool Handler_EGS_DELETE_SECOND_SECURITY_VERIFY_REQ();
	bool Handler_EGS_DELETE_SECOND_SECURITY_VERIFY_ACK(  HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );


	bool Handler_EGS_COMPARE_SECOND_SECURITY_PW_REQ(); // Phase Count 3
	bool Handler_EGS_COMPARE_SECOND_SECURITY_PW_ACK(  HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	bool Handler_EGS_CHANGE_SECOND_SECURITY_PW_REQ(); // Phase Count 3
	bool Handler_EGS_CHANGE_SECOND_SECURITY_PW_ACK(  HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );


protected:
	CKTDGUIDialogType		    m_pDLGSecurityPad;
	CKTDGUIStatic*				m_pStaticMemo;				

	bool						m_bShow;
	bool						m_bUseSecondPW;
	int							m_iButtonSequence[12];
	WCHAR						m_cPassword[7];
	wstring						m_wstrPassword_Temp1;
	wstring						m_wstrPassword_Temp2;
	wstring						m_wstrPassword_Delete;
	int							m_iPasswordIndex;
	SECURITY_PAD_STATE			m_eState;
	int							m_iPhase;
};
#endif SERV_SECOND_SECURITY