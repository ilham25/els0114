#pragma once
#include "X2Data/XSLBattleFieldManager.h"


//{{ 2011. 10. 18	최육사	배틀필드 시스템
#ifdef SERV_BATTLE_FIELD_SYSTEM


class KBattleFieldGameManager
{
public:
    KBattleFieldGameManager();
    virtual ~KBattleFieldGameManager();

	// 게임 시작, 종료 처리
	void	StartGame();
	void	EndGame();

	// 정보 얻기
	int		GetDangerousValue() const				{ return m_iDangerousValue; }
	//{{ 2013. 02. 15   필드 중간 보스 - 김민성
#ifdef SERV_BATTLEFIELD_MIDDLE_BOSS
	int		GetOldDangerousValue() const				{ return m_iOldDangerousValue; }
	void	ResetDangerousValue()						{ m_iDangerousValue = 0; m_iOldDangerousValue = 0; }
#else
	void	ResetDangerousValue()					{ m_iDangerousValue = 0; }
#endif SERV_BATTLEFIELD_MIDDLE_BOSS
	//}
	KDangerousEventInfo& GetDangerousEventInfo()	{ return m_kDangerousEvent; }
	void	IncreaseDangerousValue( IN const int iIncreaseValue );
	

	// 처리 함수
	void	OnNpcUnitDie( IN const int iPlayerCount,
						  IN const bool bIsAttribNpc, 
						  IN const char cDifficultyLevel, 
						  IN const char cMonsterGrade 
						  //{{ 2013. 02. 15   필드 중간 보스 - 김민성
#ifdef SERV_BATTLEFIELD_MIDDLE_BOSS
						  , IN const bool bIncreaseDanger 
#endif SERV_BATTLEFIELD_MIDDLE_BOSS
						  //}
						  );

	bool	CheckAndDeleteReservedDangerousEvent( IN const KDangerousEventInfo::DANGEROUS_EVENT eEventEnum );

protected:
	void	CheckReserveWarningEvent( IN const int iBeforeDangerousValue );
	void	CheckReserveEliteMonsterDropEvent( IN const int iBeforeDangerousValue );
	void	CheckReserveBossDropEvent( IN const int iPlayerCount );
	//{{ 2013. 02. 15   필드 중간 보스 - 김민성
#ifdef SERV_BATTLEFIELD_MIDDLE_BOSS
	void	CheckReserveMiddleBossDropEvent( IN const int iPlayerCount );
#endif SERV_BATTLEFIELD_MIDDLE_BOSS
	//}
		
protected:
	int								m_iDangerousValue;			// 위험도
	//{{ 2013. 02. 15   필드 중간 보스 - 김민성
#ifdef SERV_BATTLEFIELD_MIDDLE_BOSS
	int								m_iOldDangerousValue;			// 이전 위험도
#endif SERV_BATTLEFIELD_MIDDLE_BOSS
	//}
	KDangerousEventInfo				m_kDangerousEvent;			// 이벤트 예약
	KLottery						m_kLotEliteMonsterDrop;		// 엘리트 몬스터 드롭 확률
};


#endif SERV_BATTLE_FIELD_SYSTEM
//}}


