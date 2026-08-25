#pragma once

#include "RefreshSingleton.h"
#include <set>
#include <map>
#include "Lottery.h"
#include "GameEventMonster.h"
#include "CommonPacket.h"



class KGameEventScriptManager
{
	DeclareRefreshSingleton( KGameEventScriptManager );
	DeclareLuaScriptParser;
	DeclDumpToLogFile;
	DeclDump;
	DeclToStringW;
public:
	struct KAdventNpcEventInfo
	{
		int					m_iEventID;
		int					m_iMonsterDropCount;
		bool				m_bIsAdventInBossStage;
		KLottery			m_kAdventNpcLot;
		std::set< int >		m_setAdventDungeon;
		std::set< int >		m_setNoAdventDungeon;
		std::set< int >		m_setNoAdventStage;
		//{{ 이벤트 몬스터 출현 ( 아군 몬스터	) - 김민성
#ifdef SERV_ALLY_EVENT_MONSTER
		bool				m_bAllyNPC;
#endif SERV_ALLY_EVENT_MONSTER
		//}}
		//{{ 2012. 12. 31	박세훈	필드 이벤트 몬스터
#ifdef SERV_FIELD_EVENT_MONSTER
		int						m_iAddLevel;
		bool					m_bDrop;
		bool					m_bAggressive;
		float					m_fAddPosY;
		std::set<int>			m_setAdventField;
		std::set<int>			m_setNoAdventField;
#endif SERV_FIELD_EVENT_MONSTER
		//}}
#ifdef SERV_ACTIVE_EVENT_MONSTER
		std::map<int, bool>		m_mapMonsterActive;
#endif SERV_ACTIVE_EVENT_MONSTER
		KAdventNpcEventInfo()
			: m_iEventID( 0 )
			, m_iMonsterDropCount( 0 )
			, m_bIsAdventInBossStage( false )
			//{{ 이벤트 몬스터 출현 ( 아군 몬스터	) - 김민성
#ifdef SERV_ALLY_EVENT_MONSTER
			, m_bAllyNPC( false )
#endif SERV_ALLY_EVENT_MONSTER
			//}}
			//{{ 2012. 12. 31	박세훈	필드 이벤트 몬스터
#ifdef SERV_FIELD_EVENT_MONSTER
			, m_iAddLevel( 0 )
			, m_bDrop( false )
			, m_bAggressive( true )
			, m_fAddPosY( 0.0f )
#endif SERV_FIELD_EVENT_MONSTER
			//}}
		{
		}
	};

	//{{ 2012. 12. 11	박세훈	기준 일자 이벤트 작업
#ifdef SERV_FIXED_DATE_EVENT
	enum EVENT_USER_TYPE
	{
		EUT_NONE				= 0,
		EUT_NEW					= 1,
		EUT_COMEBACK			= 2,
		EUT_NEW_EXIST			= 3,	// 생성 일자 기준의 기존 유저 체크
		EUT_COMEBACK_EXIST		= 4,	// 로그 아웃 일자 기준의 기존 유저 체크
		EUT_NEW_COMEBACK_EXIST	= 5,	// 생성 및 로그 아웃 일자 기준의 기존 유저 체크
		EUT_MAX					= 6,
	};
#endif SERV_FIXED_DATE_EVENT
	//}}

	//{{ 2010. 11. 15	최육사	이벤트 스크립트 실시간 패치
#ifdef SERV_EVENT_SCRIPT_REFRESH
	struct EVENT_DATA
	{
		float	m_fEXPRate;
		float	m_fVPRate;
		float	m_fEDRate;
#ifdef	SERV_RANKING_POINT_EVENT // 적용날짜: 2013-03-26
		float	m_fRankingPointRate;
#endif	// SERV_RANKING_POINT_EVENT

		int		m_iDungeonID;
		int		m_iDungeonMode;
		int		m_iUnitLevel;		
		char	m_cUnitType;
		char    m_cUnitClass;
		int		m_iPartyNum;

		// 아이템 이벤트
		float	m_fEventTime;
		int		m_iEventReward;
		//{{ 2011. 05. 31	최육사	접속 이벤트 레벨 체크
#ifdef SERV_CONNECT_EVENT_LEVEL_CHECK
		std::map< std::pair< int, int >, int >	m_mapMinMaxReward;
#endif SERV_CONNECT_EVENT_LEVEL_CHECK
		//}}
		bool	m_bGiveOnce;
		bool	m_bDelEndEvent;
		bool	m_bPcBangEvent;
		bool	m_bDirectReward;
		bool	m_bAccountEvent;
		int		m_iDropCount;
		bool	m_bWithPlayPcBang;
		//{{ 2011. 08. 09	최육사	투니 랜드 채널링 이벤트
#ifdef SERV_TOONILAND_CHANNELING_CONNECT_EVENT
		int		m_iChannelCode;
#endif SERV_TOONILAND_CHANNELING_CONNECT_EVENT
		//}}
		//{{  2011.11.08     김민성    버블파이터 공동 프로모션 이벤트
#ifdef SERV_BUBBLE_FIGHTER_TOGETHER_EVENT
		int		m_iBF_Team;
#endif SERV_BUBBLE_FIGHTER_TOGETHER_EVENT
		//}}
		//{{ 2011. 08. 12   김민성      헤니르 개편 
#ifdef SERV_NEW_HENIR_TEST
		int		m_iHenirRewardEventCount;
		bool	m_bHenirRewardUnLimited;
#endif SERV_NEW_HENIR_TEST
		//}}
		//{{ 2011. 12. 06	최육사	특정 서버군 이벤트 시스템
#ifdef SERV_SERVER_GROUP_EVENT_SYSTEM
		int		m_iServerGroup;
#endif SERV_SERVER_GROUP_EVENT_SYSTEM
		//}}
		//{{ 2012. 06. 20	김민성       이벤트 경험치 중첩 안됨 조건 추가
#ifdef SERV_NOT_ACCUMULATE_EVENT_EXP
		bool	m_bAccumulateEXP;
#endif SERV_NOT_ACCUMULATE_EVENT_EXP
		//}}
		//{{ 2012. 03. 28	박세훈	드롭률 설정 이벤트 개편 Merge 작업 ( 2012.03.20 lygan_조성욱 )
#ifdef DROPEVENT_RENEWAL
		int		m_iStartLevel;
		int		m_iEndLevel;
#endif DROPEVENT_RENEWAL
		//}}
		//{{ 2012. 04. 04	박세훈	( 복귀 유저 표시 )
#ifdef SERV_EVENT_RETURN_USER_MARK_SCRIPT
		std::wstring	m_wstrLastConnectDate;
#endif SERV_EVENT_RETURN_USER_MARK_SCRIPT
		//}}
		//{{ 2012. 06. 29	김민성       접속 시간에 따라 아이템 반복 지급
#ifdef SERV_REPEAT_CONNECT__REWARD_ITEM_EVENT
		bool	m_bRepeatEvent;
#endif SERV_REPEAT_CONNECT__REWARD_ITEM_EVENT
		//}}

#ifdef SERV_CONNECT_EVENT_CONSECUTIVELY_REWARD
		bool	m_bRepeatRewardEvent;
		int		m_iBeforeEventID;
#endif //SERV_CONNECT_EVENT_CONSECUTIVELY_REWARD

		//{{ 2012. 10. 13	박세훈	필드 전야 이벤트 ( 천사의 깃털 재활용 )
#ifdef SERV_THE_PREVIOUS_FIELD_EVENT
		bool	m_bComeBackUserEvent;
#endif SERV_THE_PREVIOUS_FIELD_EVENT
		//}}

		//{{ 2012. 12. 11	박세훈	기준 일자 이벤트 작업
#ifdef SERV_FIXED_DATE_EVENT
		std::wstring	m_wstrFixedDate;
		int				m_iEventUserType;
#endif SERV_FIXED_DATE_EVENT
		//}}
		//{{ 2012. 12. 17	최육사	아라 파티 플레이 보너스 경험치
#ifdef SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
		bool	m_bPlayWithCharEXP;
#endif SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
		//}}
		//{{ 2012. 12. 25	박세훈	특정 유저 전용 접속 이벤트
#ifdef SERV_SPECIFIC_USER_CONNECT_EVENT
		int		m_iEventGroupID;
#endif SERV_SPECIFIC_USER_CONNECT_EVENT
		//}}
		//{{ 2013. 03. 18	 퀘스트 드롭 확률 증가 이벤트 - 김민성
#ifdef SERV_QUEST_ITEM_DROP_EVENT
		float	m_fQuestItemDropRate;
#endif SERV_QUEST_ITEM_DROP_EVENT
		//}

#ifdef SERV_ADVERTISEMENT_EVENT
		std::wstring	m_wstrAdvertisementURL;
#endif SERV_ADVERTISEMENT_EVENT

#ifdef SERV_ITEM_IN_INVENTORY_CONNECT_EVENT
		int		m_iEventItemID;
#endif 

#ifdef SERV_CUSTOM_CONNECT_EVENT
		int				m_iCustomEventID;
#endif //SERV_CUSTOM_CONNECT_EVENT

#ifdef	SERV_DEFENSE_DUNGEON_OPEN_RATE_EVENT// 적용날짜: 2013-04-11
		float	m_fDefenseDungeonOpenRate;
#endif	// SERV_DEFENSE_DUNGEON_OPEN_RATE_EVENT

#ifdef	SERV_CRITERION_DATE_EVENT// 적용날짜: 2013-04-11
		byte	m_byteCriterionUserType;
		byte	m_byteCriterionPresentUserType;
		CTime	m_tCriterionDateBegin;
		CTime	m_tCriterionDateEnd;
#endif	// SERV_CRITERION_DATE_EVENT

#ifdef SERV_STEAM_USER_CONNECT_EVENT
		bool			m_bOnlySteamUser;
		bool			m_bOnlyNotSteamUser;
#endif //SERV_STEAM_USER_CONNECT_EVENT

#ifdef SERV_TIME_EVENT_ONLY_CURRENT_USER_CHAR
		bool	m_bNewUnitEvent;
		bool	m_bNewUnitEvent2;
		bool	m_bCurrentUnitEvent;
#endif //SERV_TIME_EVENT_ONLY_CURRENT_USER_CHAR

#ifdef SERV_UNIT_CLASS_LEVEL_EVENT
		int		m_iUnitClassLevel;
#endif SERV_UNIT_CLASS_LEVEL_EVENT

		EVENT_DATA()
			: m_fEXPRate( 0.f )
			, m_fVPRate( 0.f )
			, m_fEDRate( 0.f )
#ifdef	SERV_RANKING_POINT_EVENT // 적용날짜: 2013-03-26
			, m_fRankingPointRate( 0.0f )
#endif	// SERV_RANKING_POINT_EVENT
			, m_iDungeonID( 0 )
			, m_iDungeonMode( 0 )
			, m_iUnitLevel( 0 )
			, m_cUnitType( 0 )
			, m_cUnitClass( 0 )
			, m_iPartyNum( 0 )
			, m_fEventTime( 0.f )
			, m_iEventReward( 0 )
			, m_bGiveOnce( true )
			, m_bDelEndEvent( false )
			, m_bPcBangEvent( false )
			, m_bDirectReward( false )
			, m_bAccountEvent( false )
			, m_iDropCount( 0 )
			, m_bWithPlayPcBang( false )
			//{{ 2011. 08. 09	최육사	투니 랜드 채널링 이벤트
#ifdef SERV_TOONILAND_CHANNELING_CONNECT_EVENT
			, m_iChannelCode( 0 )
#endif SERV_TOONILAND_CHANNELING_CONNECT_EVENT
			//}}
			//{{  2011.11.08     김민성    버블파이터 공동 프로모션 이벤트
#ifdef SERV_BUBBLE_FIGHTER_TOGETHER_EVENT
			, m_iBF_Team( 0 )
#endif SERV_BUBBLE_FIGHTER_TOGETHER_EVENT
			//}}
			//{{ 2011. 08. 12   김민성      헤니르 개편 
#ifdef SERV_NEW_HENIR_TEST
			, m_iHenirRewardEventCount( 0 )
			, m_bHenirRewardUnLimited( false )
#endif SERV_NEW_HENIR_TEST
			//}}
			//{{ 2011. 12. 06	최육사	특정 서버군 이벤트 시스템
#ifdef SERV_SERVER_GROUP_EVENT_SYSTEM
			, m_iServerGroup( -1 )
#endif SERV_SERVER_GROUP_EVENT_SYSTEM
			//}}
			//{{ 2012. 06. 20	김민성       이벤트 경험치 중첩 안됨 조건 추가
#ifdef SERV_NOT_ACCUMULATE_EVENT_EXP
			, m_bAccumulateEXP( false )
#endif SERV_NOT_ACCUMULATE_EVENT_EXP
			//}}
			//{{ 2012. 03. 28	박세훈	드롭률 설정 이벤트 개편 Merge 작업 ( 2012.03.20 lygan_조성욱 )
#ifdef DROPEVENT_RENEWAL
			, m_iStartLevel( 0 )
			, m_iEndLevel( 0 )
#endif DROPEVENT_RENEWAL
			//}}
			//{{ 2012. 06. 29	김민성       접속 시간에 따라 아이템 반복 지급
#ifdef SERV_REPEAT_CONNECT__REWARD_ITEM_EVENT
			, m_bRepeatEvent( false )
#endif SERV_REPEAT_CONNECT__REWARD_ITEM_EVENT
			//}}
#ifdef SERV_CONNECT_EVENT_CONSECUTIVELY_REWARD
			, m_bRepeatRewardEvent( false )
			, m_iBeforeEventID( -1 )			
#endif //SERV_CONNECT_EVENT_CONSECUTIVELY_REWARD			

			//{{ 2012. 10. 13	박세훈	필드 전야 이벤트 ( 천사의 깃털 재활용 )
#ifdef SERV_THE_PREVIOUS_FIELD_EVENT
			, m_bComeBackUserEvent( false )
#endif SERV_THE_PREVIOUS_FIELD_EVENT
			//}}
			//{{ 2012. 12. 11	박세훈	기준 일자 이벤트 작업
#ifdef SERV_FIXED_DATE_EVENT
			, m_iEventUserType( 0 )
#endif SERV_FIXED_DATE_EVENT
			//}}
			//{{ 2012. 12. 17	최육사	아라 파티 플레이 보너스 경험치
#ifdef SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
			, m_bPlayWithCharEXP( false )
#endif SERV_PLAY_WITH_CHAR_PARTY_BONUS_EXP
			//}}
			//{{ 2012. 12. 25	박세훈	특정 유저 전용 접속 이벤트
#ifdef SERV_SPECIFIC_USER_CONNECT_EVENT
			, m_iEventGroupID( -1 )
#endif SERV_SPECIFIC_USER_CONNECT_EVENT
			//}}
			//{{ 2013. 03. 18	 퀘스트 드롭 확률 증가 이벤트 - 김민성
#ifdef SERV_QUEST_ITEM_DROP_EVENT
			, m_fQuestItemDropRate( 1.f )
#endif SERV_QUEST_ITEM_DROP_EVENT
		//}

#ifdef SERV_ITEM_IN_INVENTORY_CONNECT_EVENT
			, m_iEventItemID( 0 )
#endif 
#ifdef SERV_CUSTOM_CONNECT_EVENT
			, m_iCustomEventID( 0 )
#endif //SERV_CUSTOM_CONNECT_EVENT		
		
#ifdef	SERV_DEFENSE_DUNGEON_OPEN_RATE_EVENT// 적용날짜: 2013-04-11
			, m_fDefenseDungeonOpenRate( 1.0f )
#endif	// SERV_DEFENSE_DUNGEON_OPEN_RATE_EVENT
#ifdef	SERV_CRITERION_DATE_EVENT// 적용날짜: 2013-04-11
			, m_byteCriterionUserType( 0 )
			, m_byteCriterionPresentUserType( 0 )
#endif	// SERV_CRITERION_DATE_EVENT
#ifdef SERV_STEAM_USER_CONNECT_EVENT
			, m_bOnlySteamUser( false )
			, m_bOnlyNotSteamUser( false )
#endif //SERV_STEAM_USER_CONNECT_EVENT
#ifdef SERV_TIME_EVENT_ONLY_CURRENT_USER_CHAR
			, m_bNewUnitEvent( false )
			, m_bNewUnitEvent2( false )
			, m_bCurrentUnitEvent( false )
#endif //SERV_TIME_EVENT_ONLY_CURRENT_USER_CHAR
#ifdef SERV_UNIT_CLASS_LEVEL_EVENT
			, m_iUnitClassLevel( -1 )
#endif SERV_UNIT_CLASS_LEVEL_EVENT
		{
		}
	};
#endif SERV_EVENT_SCRIPT_REFRESH
	//}}

public:
	KGameEventScriptManager(void);
	~KGameEventScriptManager(void);

	// for lua
	bool AddEventMonsterInfo_LUA();
	//{{ 2010. 11. 15	최육사	이벤트 스크립트 실시간 패치
#ifdef SERV_EVENT_SCRIPT_REFRESH
	void AddReward_LUA( IN int iID );
	//{{ 2008. 7. 11  최육사  이벤트 공지
	void AddNotifyMSGServerInfo_LUA( int iServerGroupID, int iServerUID );
	//}}
#endif SERV_EVENT_SCRIPT_REFRESH
	//}}
	//{{ 2012. 12. 25	박세훈	특정 유저 전용 접속 이벤트
#ifdef SERV_SPECIFIC_USER_CONNECT_EVENT
	void AddConnectEventUserGroup_LUA( int iEventGroupID, UidType iUserUID );
	bool CheckEventGroup( IN const int iEventGroupID, IN const UidType iUserUID ) const;
#endif SERV_SPECIFIC_USER_CONNECT_EVENT
	//}}

#ifdef SERV_ADVERTISEMENT_EVENT
	void AddAdvertisement_LUA( IN int iID );
	const EVENT_DATA* GetAdvertisementData( IN int iScriptID ) const;
#endif SERV_ADVERTISEMENT_EVENT

	// function
	//{{ 2010. 11. 15	최육사	이벤트 스크립트 실시간 패치
#ifdef SERV_EVENT_SCRIPT_REFRESH
	void Init();
	bool IsNotifyMSGEvent() const { return m_bNotifyMSGEvent; }
	const EVENT_DATA* GetEventData( IN int iScriptID ) const;
#endif SERV_EVENT_SCRIPT_REFRESH
	//}}
	bool IsExistEvent( IN int iEventID )	{ return ( m_mapEventMonster.find( iEventID ) != m_mapEventMonster.end() ); }
	void GetAdventNpcEventInfo( IN const std::vector< int >& vecEventID, OUT std::vector< KAdventNpcEventInfo >& vecInfo );

private:
	std::map< int, KAdventNpcEventInfo >	m_mapEventMonster;
	//{{ 2010. 11. 15	최육사	이벤트 스크립트 실시간 패치
#ifdef SERV_EVENT_SCRIPT_REFRESH
	std::map< int, EVENT_DATA >				m_mapEventData;

	//{{ 2008. 7. 11  최육사	공지 이벤트
	UidType									m_iNotifyCnServerUID;
	bool									m_bNotifyMSGEvent;
	//}}
#endif SERV_EVENT_SCRIPT_REFRESH
	//}}
	//{{ 2012. 12. 25	박세훈	특정 유저 전용 접속 이벤트
#ifdef SERV_SPECIFIC_USER_CONNECT_EVENT
	std::map< int, std::set<UidType> >		m_mapEventUserGroup;
#endif SERV_SPECIFIC_USER_CONNECT_EVENT
	//}}
#ifdef SERV_ADVERTISEMENT_EVENT
	std::map< int, EVENT_DATA >				m_mapAdvertisementData;
#endif SERV_ADVERTISEMENT_EVENT
};

DefRefreshSingletonInline( KGameEventScriptManager );


