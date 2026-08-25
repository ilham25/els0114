#pragma once


#pragma pack( push, 1 )

//////////////////////////////////////////////////////////////////////////
#ifdef SERV_GLOBAL_BILLING

#ifdef SERV_SUPPORT_SEVERAL_CASH_TYPES
DECL_DATA( KGlobalCashInfo )
{
	// 여러 종류의 캐시를 서버가 지원해야 할 떄 사용
	
	// 기존 [EBP_BuyItem] SP 전달 파라미터와 Enum값 맞춘 상태라 변경하면 안됨
	enum GLOBAL_CASH_TYPE
	{
		GCT_PUBLISHER_CASH			= 0,		// 일반적인 퍼블리셔 캐시 (한종류의 캐시만 쓰는 경우 이것으로 통일. 실제 캐시 종류가 다르더라도 엘소드 서버가 캐시 구분 할 필요 없을 경우 이것으로 처리함)
		GCT_KOG_ELSWORD_CASH		= 1,		// KOG에서 관리하는 엘소드로 전환된 캐시 (중국 & 동남아시아 사용)
		GCT_KOG_ELSWORD_BONUS_POINT	= 2,		// KOG에서 관리하는 엘소드 추가 보너스 포인트 (중국만 사용)
		GCT_PUBLISHER_ELSWORD_CASH	= 3,		// 퍼블리셔 쪽 엘소드로 전환된 캐시
		GCT_TOTAL_NUM

		// 넣을려면 PUT, GET도 수정해야함
		//GCT_PUBLISHER_BONUS_CASH	= 4,		// 퍼블리셔 쪽 보너스 캐쉬 (현재 사용국가 없어서 주석처리)
	};

	unsigned long				m_ulCash[GCT_TOTAL_NUM];

	KGlobalCashInfo()
	{
		for(int i=0; i < GCT_TOTAL_NUM; i++ )
		{
			m_ulCash[i]	= 0;
		}
	}
};
#endif //SERV_SUPPORT_SEVERAL_CASH_TYPES


DECL_DATA( KBillProductCategory )
{
	char                m_cCategoryNo;
	std::wstring        m_wstrCategoryName;
	char                m_cParentCategoryNo;
	char                m_cDisplayNo;
};

DECL_DATA( KBillProductInfo )
{
	int                 m_iProductNo;
	int                 m_iProductID;
	bool                m_bSale;
	bool                m_bEvent;
	bool                m_bShow;
	bool                m_bIsCashInvenSkip;
	std::wstring        m_wstrProductName;
	char                m_cCategoryNo;
	int                 m_iSalePrice;
	int                 m_iRealPrice;
	char                m_cPeriod;			// 사용 기간
	char                m_cQuantity;
	bool                m_bEnableGift;
	char                m_cLimitLevel;
	std::wstring        m_wstrEndDate;		// 판매 기간
};

DECL_DATA( KBillProductAttribute )
{
	int                 m_iProductNo;
	std::vector< int >  m_vecAttribute;
};

DECL_DATA( KBillPackageInfo )
{
	int                 m_iPackageNo;	// TranNo 임
	std::vector< int >  m_vecProductNo;

#ifdef SERV_GLOBAL_CASH_PACKAGE
	unsigned long       m_ulProductNo;
	int			        m_iProductID;
	std::wstring        m_wstrProductID;
	//{{ 2009. 12. 2  최육사	캐쉬템기간타입변경
	short				m_sProductExpire;
	//}}
	unsigned short      m_usProductPieces;
	std::vector< int >  m_vecSocketOption;
	//{{ 2009. 12. 2  최육사	캐쉬템기간타입변경
	short GetProductExpire() const { return ( m_sProductExpire == -1 ? 0 : m_sProductExpire ); }
	//}}
#endif //SERV_GLOBAL_CASH_PACKAGE
};

DECL_DATA( KBillBuyInfo )
{
	int                 m_iProductNo;
	int                 m_iPoint;
	int                 m_iProductID;
	unsigned short		m_usOrderQuantity;
	int                 m_iCategory;
	std::wstring        m_wstrMemo;


	std::wstring		m_wstrOrderID;


	KBillBuyInfo()
	{
		m_iProductNo		= 0;
		m_iPoint			= 0;
		m_iProductID		= 0;
		m_usOrderQuantity	= 0;
		m_iCategory			= 0;
		m_wstrMemo			= L"";

		m_wstrOrderID		= L"";
	}
};

DECL_DATA( KBillOrderInfo )
{
	//{{ 허상형 : [2010/2/10/] //	캐시 아이템 지급시 Sell Log 추가
	enum BUY_CASH_ITEM_TYPE
	{
		BCIT_INVALID	= 0,
		BCIT_BUY_ITEM,			//	캐시 아이템 구입
		BCIT_COUPON,			//	쿠폰
		BCIT_GIFT,				//	선물
	};
	//}} 허상형 : [2010/2/10/] //	캐시 아이템 지급시 Sell Log 추가

	UidType             m_iTransNo;
	std::wstring        m_wstrOrderID;
	KBillProductInfo    m_kBillProductInfo;
	std::vector< int >  m_vecSocketOption;

	byte				m_byteType;
	std::wstring		m_wstrSenderNickName;
	std::wstring		m_wstrSenderPresentMessage;
	//{{ 허상형 : [2010/8/19/] //	선물 기능 개편
	UidType				m_iFromUnitUID;
	//}} 허상형 : [2010/8/19/] //	선물 기능 개편
#ifdef SERV_GLOBAL_CASH_PACKAGE
	BYTE                m_byteProductKind;
#endif //SERV_GLOBAL_CASH_PACKAGE

	KBillOrderInfo()
	{
		m_iTransNo = -1;
		m_byteType = BCIT_INVALID;
#ifdef	SERV_GLOBAL_CASH_PACKAGE
		m_byteProductKind = 0;
#endif	SERV_GLOBAL_CASH_PACKAGE
	}
};
#ifdef SERV_KEEP_ITEM_SHOW_CASHSHOP
DECL_DATA( kDisCountItemInfo )
{
	int					m_DisCountProNo;
	int					m_BasicProNo;
	std::vector< int >	m_DisCountKeepItem;
};
#endif SERV_KEEP_ITEM_SHOW_CASHSHOP
#endif SERV_GLOBAL_BILLING
//////////////////////////////////////////////////////////////////////////


#ifdef SERV_NEW_PVPROOM_PROCESS

DECL_PACKET( EGS_CHANGE_PVP_ROOM_PUBLIC_REQ )
{
	bool									m_bPublic;
	std::wstring							m_wstrPassword;
};

DECL_PACKET( EGS_CHANGE_PVP_ROOM_PUBLIC_ACK )
{
	bool									m_bPublic;
	int										m_iOK;
};

DECL_PACKET( EGS_CHANGE_PVP_ROOM_PUBLIC_NOT )
{
	bool									m_bPublic;
};

typedef KEGS_CHANGE_PVP_ROOM_PUBLIC_REQ KERM_CHANGE_PVP_ROOM_PUBLIC_REQ;
typedef KEGS_CHANGE_PVP_ROOM_PUBLIC_ACK KERM_CHANGE_PVP_ROOM_PUBLIC_ACK;

DECL_PACKET( EGS_CHANGE_PVP_ROOM_NAME_REQ )
{
	std::wstring							m_wstrRoomName;
};

DECL_PACKET( EGS_CHANGE_PVP_ROOM_NAME_ACK )
{
	std::wstring							m_wstrRoomName;
	int										m_iOK;
};

DECL_PACKET( EGS_CHANGE_PVP_ROOM_NAME_NOT )
{
	std::wstring							m_wstrRoomName;
};

typedef KEGS_CHANGE_PVP_ROOM_NAME_REQ KERM_CHANGE_PVP_ROOM_NAME_REQ;
typedef KEGS_CHANGE_PVP_ROOM_NAME_ACK KERM_CHANGE_PVP_ROOM_NAME_ACK;

#endif SERV_NEW_PVPROOM_PROCESS

#ifdef SERV_PC_BANG_TYPE
DECL_DATA( KPcBangReward )
{
	int		m_iPcBangType;
	float	m_fEXPRate;
	float	m_fEDRate;
	float	m_fVSPointRate;
	bool	m_bSpiritFree;
	float	m_fPetEXPRate;
	int		m_iAdditionalHenirReward;
	bool	m_bSecretDungeonFree;
	bool	m_bAvatar;
	bool	m_bTitle;

	KPcBangReward()
	{
		m_iPcBangType				= -1;
		m_fEXPRate					= 0.0f;
		m_fEDRate					= 0.0f;
		m_fVSPointRate				= 0.0f;
		m_bSpiritFree				= false;
		m_fPetEXPRate				= 0.0f;
		m_iAdditionalHenirReward	= 0;
		m_bSecretDungeonFree		= false;
		m_bAvatar					= false;
		m_bTitle					= false;
	}

	COPYCON_ASSIGNOP( KPcBangReward, right )
	{
		m_iPcBangType				= right.m_iPcBangType;
		m_fEXPRate					= right.m_fEXPRate;
		m_fEDRate					= right.m_fEDRate;
		m_fVSPointRate				= right.m_fVSPointRate;
		m_bSpiritFree				= right.m_bSpiritFree;
		m_fPetEXPRate				= right.m_fPetEXPRate;
		m_iAdditionalHenirReward	= right.m_iAdditionalHenirReward;
		m_bSecretDungeonFree		= right.m_bSecretDungeonFree;
		m_bAvatar					= right.m_bAvatar;
		m_bTitle					= right.m_bTitle;

		return *this;
	}
};
#endif SERV_PC_BANG_TYPE

//{{ 2011. 12. 09	박교현		서버군 리스트
#ifdef SERVER_GROUP_UI_ADVANCED
DECL_DATA( KServerGroupInfo )
{
	int							m_iOrder;
	int							m_iServerGroupUID;
	std::wstring				m_wstrServerGroupName;
	std::wstring				m_kServerIP;
	u_short						m_usMasterPort;
	u_short						m_usNCUDPPort;
	bool						m_bEnable;
};
#endif SERVER_GROUP_UI_ADVANCED
//}}

#ifdef SERV_CATCH_HACKUSER_INFO
DECL_PACKET( EGS_CATCH_HACKUSER_INFO_NOT )
{
	UidType				m_iUserUID;
	UidType				m_iUnitUID;
	int					m_iCrashType;
};
#endif SERV_CATCH_HACKUSER_INFO

#ifdef	SERV_SHARING_BANK_TEST
DECL_DATA( KTradeShareItemResult )
{
	UidType								m_iBeforeItemUID;
	UidType								m_iNewItemUID;

	std::wstring						m_wstrRegDate;

	KTradeShareItemResult()
	{
		m_iBeforeItemUID				= 0;
		m_iNewItemUID					= 0;

		m_wstrRegDate				= L"";
	}
};
#endif	SERV_SHARING_BANK_TEST

//{{ 2013. 05. 14	최육사	아이템 개편
#ifdef SERV_BLOCK_LIST
DECL_DATA( KBlockInfo )
{
	enum BLOCK_TYPE
	{
		BT_TRADE_BLOCK = 0,
		BT_ACCOUNT_BLOCK,
		BT_IP_BLOCK,
		BT_DLL_BLOCK,

		BT_MAX,
	};

    UidType							m_iBlockUID;
	char							m_cBlockType;
	UidType							m_iBlockTarget;
	std::wstring					m_wstrBlockTarget;	
	std::wstring					m_wstrStartDate;
	std::wstring					m_wstrEndDate;

	KBlockInfo()
	{
		m_iBlockUID = 0;
		m_cBlockType = 0;
		m_iBlockTarget = 0;
	}
};
#endif SERV_BLOCK_LIST
//}}

#ifdef SERV_ANTI_ADDICTION_SYSTEM
DECL_DATA( KGiantAccountType )
{
	enum TYPE_MASK
	{ 
		NORMAL_USER			= 0,		// 00: 일반 유저
		SPECIAL_USER		= 2,		// 01: 특수 유저
		VIP_USER			= 4,		// 02: VIP 유저
		EVENT_PLAYER		= 8,		// 03: 이벤트 플레이어
		ADULT_PLAYER		= 16,		// 04: 성인 플레이어
		BLACKLIST_USER		= 32,		// 05: 블랙리스트 유저

		DEALER				= 512,		// 10: 딜러?
		WEB_DEALER			= 1024,		// 11: 웹딜러?
	};

	KGiantAccountType()
	{
		m_uiType = 0;
	}

	bool			IsType(TYPE_MASK mask)			{ return ( m_uiType & mask ) ? true : false; }

	unsigned int	m_uiType;
};
#endif SERV_ANTI_ADDICTION_SYSTEM

#ifdef SERV_DEVELOPER_RANDOM_OPEN_ITEM_LOG
DECL_DATA( KRandomItemResult )
{
	int		m_iItemCount;
	bool	m_bIsSealItem;
	double  m_dRate;
	double  m_dItemTotalCount;
	bool	m_bIsNotifyMsgAttration;

};
#endif//SERV_DEVELOPER_RANDOM_OPEN_ITEM_LOG


#ifdef SERV_ID_NETMARBLE_PCBANG
DECL_PACKET( ECH_PCBANG_IP_AND_MAC_INFO_NOT )
{
	std::wstring							m_wstrPublicIP;
	std::wstring							m_wstrMacInfo;
};

typedef KECH_PCBANG_IP_AND_MAC_INFO_NOT			 KDBE_PCBANG_IP_AND_MAC_INFO_NOT;
#endif //SERV_ID_NETMARBLE_PCBANG


#ifdef SERV_COUNTRY_PH
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef std::vector<uint8> dataBuffer_t;
typedef std::string string_t;
DECL_DATA( kGarenaUserTokInfo_t ) // 2013.07.31 lygan_조성욱 // 동남아시아 퍼블리셔에서 인증 관련으로 제공해준 구조체
{
	uint32				m_Uid;
	std::string			m_Username;
	uint32				m_Timestamp;
	uint16				m_GcaType;
	uint8				m_GoldMemberType;
	uint8				m_IpBonus;
	std::string			m_Reserved;

};
#endif //SERV_COUNTRY_PH



//{{ 2013. 09. 23	최육사	일본 이벤트 중계DB작업
//#ifdef SERV_RELAY_DB_CONNECTION
DECL_DATA( KJPEventRewardInfo )
{
	__int64								m_iRID;
	std::wstring						m_wstrUserID;
	UidType								m_iUserUID;
	UidType								m_iUnitUID;
	int									m_iRewardID;

	KJPEventRewardInfo()
	{
		m_iRID = 0;
		m_iUserUID = 0;
		m_iUnitUID = 0;
		m_iRewardID = 0;
	}
};
//#endif SERV_RELAY_DB_CONNECTION
//}}


#ifdef SERV_CUBE_IN_ITEM_MAPPING
DECL_DATA( kRandomItemMappingToInfoClient )
{
	int		 m_iBeforeGroupID;
	int		 m_iAfterGroupID;
	bool	 m_bCheckMapping;
	std::wstring	 m_tStartTime;
	std::wstring	 m_tEndTime;

	kRandomItemMappingToInfoClient()
	{
		m_iBeforeGroupID = 0;
		m_iAfterGroupID = 0;
		m_bCheckMapping = false;
	}
};

#ifdef SERV_CUBE_IN_ITEM_MAPPING_BY_DBTIME_SETTING
DECL_DATA( KRandomItemMappingToInfoServer )
{
	int		 m_iIndex;
	int		 m_iCubeID;
	int		 m_iKeyItemID;
	int		 m_iBeforeGroupID;
	int		 m_iAfterGroupID;
	std::wstring	 m_tStartTime;
	std::wstring	 m_tEndTime;

	KRandomItemMappingToInfoServer()
	{
		m_iIndex			= 0;
		m_iCubeID			= 0;
		m_iKeyItemID		= 0;
		m_iBeforeGroupID	= 0;
		m_iAfterGroupID		= 0;
		m_tStartTime		= L"";
		m_tEndTime			= L"";
	}
};
#endif SERV_CUBE_IN_ITEM_MAPPING_BY_DBTIME_SETTING
#endif //SERV_CUBE_IN_ITEM_MAPPING

//{{ 2012. 09. 03	임홍락	글로벌 미션 매니저
#ifdef SERV_GLOBAL_MISSION_MANAGER
DECL_DATA( KGlobalMissionReward )
{
	int									m_iRewardType;					// 보상 타입(버프, 접속이벤트 등)
	int									m_iRewardDurationTime;			// 보상 유지 시간(분)
	int									m_iScriptID;					// 보상 스크립트 ID

	KGlobalMissionReward()
	{
		m_iRewardType = 0;
		m_iRewardDurationTime = 0;
		m_iScriptID = 0;
	}
};

DECL_DATA( KGlobalMissionInfo )
{
	bool									m_bActive;						// 진행 중인지 여부
	std::wstring							m_wstrMissionStartTime;			// 실제 글로벌 미션 시작 시간
	int										m_iDurationMinute;				// 글로벌 미션 유지 시간(분)
	int										m_iRemainSecond;				// 글로벌 미션 남은 시간(초)

	int										m_iMissionType;					// 글로벌 미션 타입
	int										m_iClearCondition;				// 글로벌 미션 클리어 조건
	int										m_iClearCount;					// 목표 클리어 수치
	int										m_iNowCount;					// 현재 클리어 수치


	KGlobalMissionInfo()
	{
		m_bActive = false;
		m_wstrMissionStartTime = L"";
		m_iDurationMinute = 0;
		m_iRemainSecond = 0;

		m_iMissionType = 0;
		m_iClearCondition = 0;
		m_iClearCount = 0;
		m_iNowCount = 0;
	}
};
#endif SERV_GLOBAL_MISSION_MANAGER
//}} 2012. 09. 03	임홍락	글로벌 미션 매니저
#pragma pack( pop )