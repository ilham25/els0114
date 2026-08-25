#pragma once

class CX2User
{
	public:

		enum X2_USER_AUTH_LEVEL
		{
			XUAL_NORMAL_USER = 0,
			XUAL_SPECIAL_USER,
			XUAL_OPERATOR,
			XUAL_DEV,
//#ifdef LIGHT_OPERATOR_ACCOUNT
			XUAL_LIGHT_OPERATOR = 10,
//#endif LIGHT_OPERATOR_ACCOUNT
		};

		enum HACKING_USER_TYPE
		{
			HUT_NORMAL = 0,
			HUT_DISAGREE_HACK_USER,
			HUT_AGREE_HACK_USER,

			HUT_TOTAL_NUM,
		};

	public:
		struct UserData
		{
			UidType				userUID;			//유저의 유니크 아이디
			wstring				userID;				//아이디

			bool				m_bMan;				// 성별
			UCHAR				m_ucAge;			// 나이

			int					maxUnitCount;		//내가 만들 수 있는 캐릭터 수(가지고 있는것 포함)

			X2_USER_AUTH_LEVEL	userAuthLevel;
			bool				m_bIsPCRoom;
			bool				m_bIsGuestUser;

			wstring		IP;
			int			port;

			HACKING_USER_TYPE	hackingUserType;

#ifdef SERV_TOONILAND_CHANNELING // SERV_JAPAN_CHANNELING
			UINT8				m_uChannelCode;		// 0 넥슨, 3 투니랜드
			wstring				m_wstrTooniLandID;	// 투니랜드 아이디
#endif // SERV_JAPAN_CHANNELING

			UserData& operator=( const UserData& data )
			{
				userUID			= data.userUID;				//유저의 유니크 아이디
				userID			= data.userID;				//아이디

				m_bMan			= data.m_bMan;				// 성별
				m_ucAge			= data.m_ucAge;				// 나이

				maxUnitCount	= data.maxUnitCount;		//내가 만들 수 있는 캐릭터 수(가지고 있는것 포함)
				
				userAuthLevel	= data.userAuthLevel;
				m_bIsPCRoom		= data.m_bIsPCRoom;
				m_bIsGuestUser	= data.m_bIsGuestUser;

				IP				= data.IP;
				port			= data.port;

				hackingUserType = data.hackingUserType;

#ifdef SERV_TOONILAND_CHANNELING // SERV_JAPAN_CHANNELING
				m_uChannelCode	= data.m_uChannelCode;
				m_wstrTooniLandID = data.m_wstrTooniLandID;	// 투니랜드 아이디
#endif // SERV_JAPAN_CHANNELING
				return *this;
			}
			UserData& operator=( const KEGS_VERIFY_ACCOUNT_ACK& data )
			{
				
				userUID			= data.m_kAccountInfo.m_nUserUID;				//유저의 유니크 아이디
				userID			= data.m_kAccountInfo.m_wstrID;					//아이디

				m_bMan			= data.m_bSex;
				m_ucAge			= static_cast<UCHAR>( data.m_uiAge );

				maxUnitCount	= 0;							//내가 만들 수 있는 캐릭터 수(가지고 있는것 포함)

				userAuthLevel	= (X2_USER_AUTH_LEVEL)data.m_kAccountInfo.m_iAuthLevel;

				m_bIsGuestUser	= data.m_kAccountInfo.m_bIsGuestUser;

				hackingUserType = (CX2User::HACKING_USER_TYPE)data.m_cHackingUserType;

#ifdef SERV_TOONILAND_CHANNELING // SERV_JAPAN_CHANNELING
				m_uChannelCode		= data.m_uChannelCode;
				m_wstrTooniLandID	= data.m_wstrChannelUserID;	// 투니랜드 아이디
#endif // SERV_JAPAN_CHANNELING
				return *this;
			}
			UserData() :
			userUID( 0 ), m_bMan( true ), m_ucAge( 0 ), maxUnitCount( 0 ),
				port( 0 ), userAuthLevel( XUAL_NORMAL_USER ), m_bIsPCRoom( false ),
				m_bIsGuestUser( false ), hackingUserType( CX2User::HUT_NORMAL )

#ifdef SERV_TOONILAND_CHANNELING // SERV_JAPAN_CHANNELING
				, m_uChannelCode( -1 )		/// CE_NONE
				, m_wstrTooniLandID()		
#endif // SERV_JAPAN_CHANNELING
			{}
		};

	public:
		CX2User( UserData* pUserData );
		~CX2User(void);
#ifdef LIST_SORT_AT_CHARACTER_SELECT
		bool		SortFirstByUID(UidType aUnitUID);
#endif LIST_SORT_AT_CHARACTER_SELECT

		bool		AddUnit( CX2Unit* pUnit );
		int			GetUnitNum() const { return (int)m_UnitList.size(); }
		CX2Unit*	GetUnitByUID( UidType unitUID );
		CX2Unit*	GetUnitByIndex( int index );
		CX2Unit*	GetUnitByNickName( const WCHAR* pNickName );

		bool        IsValidUID() const { return m_pUserData->userUID != 0; }
		UidType		GetUID(){ return m_pUserData->userUID; }
		void		SelectUnit( CX2Unit* pUnit ){ m_pSelectUnit = pUnit; }
		CX2Unit*	GetSelectUnit(){ return m_pSelectUnit; }

		bool		DeleteUnitByUID( UidType unitUID );
		void		DeleteAllUnit();

		UserData*	GetUserData() const { return m_pUserData; }

		bool		IsMyUnitByUnitUID( UidType unitUid );

		void		SetPCRoom( bool bPCRoom )
		{ 
			if( m_pUserData != NULL )
				m_pUserData->m_bIsPCRoom = bPCRoom; 
		}
		bool		GetIsPCRoom()
		{ 
			if( m_pUserData != NULL )
				return m_pUserData->m_bIsPCRoom;
			else
				return false;
		}

		X2_USER_AUTH_LEVEL GetAuthLevel() { return m_pUserData->userAuthLevel; }

#ifdef SERV_SUPPORT_SEVERAL_CASH_TYPES
		void				SetGlobalCash(KGlobalCashInfo globalCashInfo);
		
		void				SetGlobalTypeCash(int iCashType, int ulCash);
		unsigned long		GetGlobalCash(int iCashType);
		unsigned long		GetGlobalCashForRescurrection();
		unsigned long		GetGlobalMainCash();	
		unsigned long		GetGlobalTotalCash(){ return m_ulGlobalTotalCash; };
#else //SERV_SUPPORT_SEVERAL_CASH_TYPES
		unsigned long		GetCash() { return m_Cash; }
		void				SetCash( unsigned long cash ) {  m_Cash = cash; }
#endif //SERV_SUPPORT_SEVERAL_CASH_TYPES

		void				SetUserSex( bool bMan ) { m_pUserData->m_bMan = bMan; }
		bool				IsMan() const { return m_pUserData->m_bMan; }

		void				SetUserAge( UCHAR ucAge ) { m_pUserData->m_ucAge = ucAge; }
		UCHAR				GetUserAge() const { return m_pUserData->m_ucAge; }

		//{{ kimhc // 2010-12-01 // 웹 아바타 이미지 등록
#ifdef	AVATAR_IMAGE_TEST
		UINT GetNexonSN() const { return m_uiNexonSN; }
		void SetNexonSN( UINT uiNexonSN_ ) { m_uiNexonSN = uiNexonSN_; }
#endif	AVATAR_IMAGE_TEST
		//}} kimhc // 2010-12-01 // 웹 아바타 이미지 등록

#ifdef SERV_EVENT_MONEY
		void				SetUserAngelsFeather( int iAngelsFeather ) { m_iAngelsFeather = iAngelsFeather; }
		int				    GetUserAngelsFeather() const {return m_iAngelsFeather;}
#endif //SERV_EVENT_MONEY

#ifdef SEND_NEXON_WISE_LOG_BY_URL // 김태환
		void				SetFirstCreateCharacter(bool val) { m_bFirstCreateCharacter = val; }
		bool				GetFirstCreateCharacter() const { return m_bFirstCreateCharacter; }
#endif // SEND_NEXON_WISE_LOG_BY_URL

#ifdef FIX_SKILL_SLOT_COOLTIME_IN_VILLAGE //2013.08.09
		void				ChangeEquippedSkillState( const int iSkillSlotId_, const int iSkillId_ );
#endif //FIX_SKILL_SLOT_COOLTIME_IN_VILLAGE

#ifdef HIDE_SET_DESCRIPTION
		void				ChangeHideSetDesc() { m_bHideSetDesc = !m_bHideSetDesc; }
		bool				GetHideSetDesc() const { return m_bHideSetDesc; }
#endif HIDE_SET_DESCRIPTION

#ifdef SERV_SHARING_BANK_QUEST_CASH
		void		SetSharingBankOpen(bool bOpen) { m_bSharingBankOpen = bOpen; }
		bool		IsSharingBankOpen() const { return m_bSharingBankOpen; }
#endif // SERV_SHARING_BANK_QUEST_CASH

	private:
		

		UserData*			m_pUserData;

		unsigned long		m_Cash;
#ifdef SERV_SUPPORT_SEVERAL_CASH_TYPES
		KGlobalCashInfo		m_GlobalCashInfo;	// 구조체 캐시 Value 생성자에서 초기화 됨
		unsigned long		m_ulGlobalTotalCash;
#endif //SERV_SUPPORT_SEVERAL_CASH_TYPES

		CX2Unit*			m_pSelectUnit;
		vector<CX2Unit*>	m_UnitList;

		//{{ kimhc // 2010-12-01 // 웹 아바타 이미지 등록
#ifdef	AVATAR_IMAGE_TEST
		UINT				m_uiNexonSN;		/// Nexon OID(오 아이디)
#endif	AVATAR_IMAGE_TEST
		//}} kimhc // 2010-12-01 // 웹 아바타 이미지 등록

#ifdef SERV_EVENT_MONEY
		KProtectedType<int>	m_iAngelsFeather;	// 천사의 깃털
#endif //SERV_EVENT_MONEY

#ifdef SEND_NEXON_WISE_LOG_BY_URL // 김태환
		bool				m_bFirstCreateCharacter;	/// 첫 케릭 생성 여부 ( 로그 전송용 )
#endif // SEND_NEXON_WISE_LOG_BY_URL

#ifdef HIDE_SET_DESCRIPTION
		bool				m_bHideSetDesc;
#endif HIDE_SET_DESCRIPTION

#ifdef SERV_SHARING_BANK_QUEST_CASH
		bool				m_bSharingBankOpen;
#endif // SERV_SHARING_BANK_QUEST_CASH
};
