#pragma once
#include "Event.h"
#include "CommonPacket.h"
#include "CacheData.h"
#include "X2Data\XSLUnit.h"


//{{ 2011. 08. 12   김민성      헤니르 개편 
#ifdef SERV_NEW_HENIR_TEST


class KUserDungeonManager
{
public:
	KUserDungeonManager(void);
	~KUserDungeonManager(void);

	enum HENIR_REWARD_LIMIT_COUNT
	{
		HRLC_NORMAL				= 3,		// 일반
		HRLC_PREMIUM			= 2,		// pc방 혜택
	};

	void	Clear();
	void	Init( IN const bool bUnlimitedEvent,		// 무제한 인가?
				  IN const int iLimitEvent,				// Event 추가 횟수 MAX
				  IN const int iNormalCnt,				// 보상 받은 횟수 일반
				  IN const int iPremiumCnt,				// 보상 받은 횟수 pc방
				  IN const int iEventCnt,				// 보상 받은 횟수 이벤트
				  IN const bool bIsPcBang				// PC방인지 정보 넣기
#ifdef SERV_PC_BANG_TYPE
				  , IN const int iPcBangType			// PC방 타입
#endif SERV_PC_BANG_TYPE
				  );

	// DB업데이트 준비
	void	GetDBUpdateInfo( OUT int& iNormalCnt,
						 	 OUT int& iPremiumCnt, 
						 	 OUT int& iEventCnt );

	// DB업데이트 롤백
	void	CheckRollbackInfo( IN const int iNormalCnt,
						   	   IN const int iPremiumCnt,
							   IN const int iEventCnt );

	// 보상 받을 수 있는 가능 횟수 셋팅 - 치트용
	void	SetPossibleHenirRewardNormalCount( IN const int iNormal )		{ m_iHenirRewardCount = HRLC_NORMAL - iNormal; }


	bool	IsPremiumUser() const										{ return m_bIsPcBang; }
#ifdef SERV_PC_BANG_TYPE
	int		GetPcBangType() const										{ return m_iPcBangType; }
#endif SERV_PC_BANG_TYPE

	void	SetPossibleHenirReward();
	bool	IsPossibleHenirReward()								{ SetPossibleHenirReward(); return m_bPossibleHenirReward; }
	bool	IncreaseHenirRewardCount();																// 보상 획득 횟수 증가

	// 이벤트로 인한 값 셋팅
	void	SetUnLimitedHenirRewardEvnet( IN const bool bLimit )		{ m_bUnLimitedHenirRewardEvnet = bLimit; }	// 무한 보상 획득 설정
	void	SetHenirRewardEvnetLimitCount( IN const int iLimit )		{ m_iHenirRewardEventLimitCount = iLimit; }	// 이벤트로 보상 가능 횟수 설정

	// 보상 받을 수 있는 가능 횟수 얻기
	int		GetPossibleHenirRewardNormalCount();
	int		GetPossibleHenirRewardEventCount();
	int		GetPossibleHenirRewardPremiumCount();

	// 실제 보상 받은 횟수 얻기
	int		GetHenirRewardNormalCount()									{ return m_iHenirRewardCount; }
	int		GetHenirRewardEventCount()									{ return m_iHenirRewardEventCount; }
	int		GetHenirRewardPremiumCount()								{ return m_iHenirRewardPremiumCount; }
	
	// 무제한 보상 이벤트 상태 얻기
	bool	GetPossibleHenirRewardEventUnLimited()								{ return m_bUnLimitedHenirRewardEvnet; }

	void	GetHenirRewardCountInfo( IN KEGS_HENIR_REWARD_COUNT_NOT& kNot );

	//{{ 2012. 02. 08	최육사	배틀필드 시스템
#ifdef SERV_BATTLE_FIELD_SYSTEM
	UidType GetAutoPartyWaitNumber() const								{ return m_iAutoPartyWaitNumber; }
	void	SetAutoPartyWaitNumber( IN const UidType iWaitNumber )		{ m_iAutoPartyWaitNumber = iWaitNumber; }
	UidType GetAutoPartyUID() const										{ return m_iAutoPartyUID; }
	void	SetAutoPartyUID( IN const UidType iAutoPartyUID )			{ m_iAutoPartyUID = iAutoPartyUID; }

	void	SetDungeonGameInfo( IN const int iDungeonID, IN const char cDifficulty, IN const char cDungeonMode );
	void	SetDungeonGameInfo( IN const KDungeonGameInfo& kInfo )		{ SetDungeonGameInfo( kInfo.m_iDungeonID, kInfo.m_cDifficulty, kInfo.m_cDungeonMode ); }
	int		GetDungeonID() const										{ return m_kDungeonGameInfo.m_iDungeonID; }
	bool	IsPlayingNowHenirChallengeMode() const;

	// for autoparty
	void	GetAutoPartyBonusDungeonList( IN const int iVillageMapID, IN OUT std::map< int, KAutoPartyDungeonBonusInfo >& mapAutoPartyBonusList );
	void	GetAutoPartyBonusDungeonList( IN const std::vector< KDungeonGameSimpleInfo >& vecDungeonList, IN OUT std::map< int, KAutoPartyDungeonBonusInfo >& mapAutoPartyBonusList );
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}

	//{{ QUEST 개편 - 김민성
#ifdef SERV_REFORM_QUEST
	char		GetDungeonDifficulty() const							{ return m_kDungeonGameInfo.m_cDifficulty; }
#endif SERV_REFORM_QUEST
	//}}

private:
	bool				m_bIsPcBang;
#ifdef SERV_PC_BANG_TYPE
	int					m_iPcBangType;
#endif SERV_PC_BANG_TYPE
	bool				m_bPossibleHenirReward;

//{{-------- 보상 받기 가능 횟수 계산용 데이터 ---------
	// 보상 받은 횟수를 저장
	int					m_iHenirRewardCount;				// 헤니르 보상 사용한 횟수 (일반)
	int					m_iHenirRewardPremiumCount;			// 헤니르 보상 사용한 횟수 (PC방)
	int					m_iHenirRewardEventCount;			// 헤니르 보상 사용한 횟수 (이벤트) 횟수
	
	// 보상 받을 수 있는 횟수
	int					m_iHenirRewardEventLimitCount;		// 헤니르 보상 가능 횟수 (이벤트) 제한
	// 무한 보상 이벤트 상태값
	bool				m_bUnLimitedHenirRewardEvnet;		// 헤니르 보상 가능 횟수 (이벤트) 무제한
//}}----------------------------------------------------

	//{{ 2012. 02. 08	최육사	배틀필드 시스템
#ifdef SERV_BATTLE_FIELD_SYSTEM
	UidType				m_iAutoPartyWaitNumber;
	UidType				m_iAutoPartyUID;

	KDungeonGameInfo	m_kDungeonGameInfo;
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}
};


#endif SERV_NEW_HENIR_TEST
//}}