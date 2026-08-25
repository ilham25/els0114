#pragma once

#include "GameEvent.h"
//#include "TimerManager.h"

//{{ 2009. 10. 26  최육사	PC방이벤트
//struct KUserCumulativeTimeEventInfo
//{
//	int				m_iEventUID;
//	int				m_iEventID;
//	bool			m_bInit;
//	std::wstring	m_wstrEventName;
//	float			m_fEventTime;
//	KTimerManager	m_kEventTimer;
//	bool			m_bDirectReward;
//
//	KUserCumulativeTimeEventInfo()
//	{
//		m_iEventUID		= 0;
//		m_iEventID		= 0;
//		m_bInit			= false;
//		m_fEventTime	= 0.0f;
//		m_bDirectReward = false;
//	}
//};
//}}

class KGameEventCumulativeTime : public KGameEvent
{
public:
	KGameEventCumulativeTime(void);
	virtual ~KGameEventCumulativeTime(void);
	
	void	SetEventTime( float fEventTime )	{ m_fEventTime = fEventTime; } // 이벤트 타임 단위는 분단위입니다.
	float	GetEventTime()						{ return m_fEventTime; }

	void	SetEventID( int iEventID )			{ m_iEventID = iEventID; }
	int		GetEventID()						{ return m_iEventID; }
	
	void	SetRewardID( int iRewardID )		{ m_iRewardID = iRewardID; }
	int		GetRewardID()						{ return m_iRewardID; }

	//{{ 2009. 10. 26  최육사	PC방이벤트
	void	SetPcBangEvent( bool bPcBangEvent )	{ m_bPcBangEvent = bPcBangEvent; }
	bool	IsPcBangEvent()						{ return m_bPcBangEvent; }
	//}}

	//{{ 2009. 12. 8  최육사	접속시간이벤트
	void	SetDirectReward( bool bVal )		{ m_bDirectReward = bVal; }
	bool	IsDirectReward()					{ return m_bDirectReward; }
	//}}

	//{{ 2010. 06. 11  최육사	계정단위 접속시간 이벤트
	void	SetAccountEvent( bool bAccountEvent )	{ m_bAccountEvent = bAccountEvent; }
	bool	IsAccountEvent()						{ return m_bAccountEvent; }
	//}}
	//{{ 2011. 08. 09	최육사	투니 랜드 채널링 이벤트
#ifdef SERV_TOONILAND_CHANNELING_CONNECT_EVENT
	void	SetChannelCode( int iChannelCode )		{ m_iChannelCode = iChannelCode; }
	int		GetChannelCode()						{ return m_iChannelCode; }
#endif SERV_TOONILAND_CHANNELING_CONNECT_EVENT
	//}}
	//{{ 2012. 12. 11	박세훈	기준 일자 이벤트 작업
#ifdef SERV_FIXED_DATE_EVENT
	void				SetFixedDate( IN const std::wstring& wstrFixedDate ){	m_wstrFixedDate = wstrFixedDate;	}
	const std::wstring	GetFixedDate( void ) const {	return m_wstrFixedDate;	}

	void	SetEventUserType( IN const int iEventUserType ){	m_iEventUserType = iEventUserType;	}
	int		GetEventUserType( void ) const {	return m_iEventUserType;	}
#endif SERV_FIXED_DATE_EVENT
	//}}
	//{{ 2013. 1. 8	박세훈	누적 이벤트에 반복 기능 추가
#ifdef SERV_REPEAT_CUMULATIVE_REWARD_ITEM_EVENT
	void	SetRepeatEvent( const bool bRepeat )	{	m_bRepeatEvent = bRepeat;	}
	bool	GetRepeatEvent( void ) const			{	return m_bRepeatEvent;	}
#endif SERV_REPEAT_CUMULATIVE_REWARD_ITEM_EVENT
	//}}

#ifdef SERV_CUSTOM_CONNECT_EVENT
	void	SetCustomEventID( IN const int iCustomEventID ){	m_iCustomEventID = iCustomEventID;	}
	int		GetCustomEventID( void ) const	{ return m_iCustomEventID;	}
#endif SERV_CUSTOM_CONNECT_EVENT

#ifdef SERV_TIME_EVENT_ONLY_CURRENT_USER_CHAR
	void	SetNewUnitEvent( bool bNewUnitEvent )		{ m_bNewUnitEvent = bNewUnitEvent; }	
	bool	IsNewUnitEvent()							{ return m_bNewUnitEvent; }

	void	SetNewUnitEvent2( bool bNewUnitEvent2 )		{ m_bNewUnitEvent2 = bNewUnitEvent2; }	
	bool	IsNewUnitEvent2()							{ return m_bNewUnitEvent2; }

	void	SetCurrentUnitEvent( bool bCurrentUnitEvent )		{ m_bCurrentUnitEvent = bCurrentUnitEvent; }	
	bool	IsCurrentUnitEvent()							{ return m_bCurrentUnitEvent; }
#endif //SERV_TIME_EVENT_ONLY_CURRENT_USER_CHAR

private:
	float	m_fEventTime;
	int		m_iEventID;
	int		m_iRewardID;
	//{{ 2009. 10. 26  최육사	PC방이벤트
	bool	m_bPcBangEvent;
	//}}
	//{{ 2009. 12. 8  최육사	접속시간이벤트
	bool	m_bDirectReward;
	//}}
	//{{ 2010. 06. 11  최육사	계정단위 접속시간 이벤트
	bool	m_bAccountEvent;
	//}}
	//{{ 2011. 08. 09	최육사	투니 랜드 채널링 이벤트
#ifdef SERV_TOONILAND_CHANNELING_CONNECT_EVENT
	int		m_iChannelCode;
#endif SERV_TOONILAND_CHANNELING_CONNECT_EVENT
	//}}
	//{{ 2012. 12. 11	박세훈	기준 일자 이벤트 작업
#ifdef SERV_FIXED_DATE_EVENT
	std::wstring	m_wstrFixedDate;
	int				m_iEventUserType;
#endif SERV_FIXED_DATE_EVENT
	//}}
	//{{ 2013. 1. 8	박세훈	누적 이벤트에 반복 기능 추가
#ifdef SERV_REPEAT_CUMULATIVE_REWARD_ITEM_EVENT
	bool	m_bRepeatEvent;
#endif SERV_REPEAT_CUMULATIVE_REWARD_ITEM_EVENT
	//}}
#ifdef SERV_CUSTOM_CONNECT_EVENT
	int		m_iCustomEventID;
#endif //SERV_CUSTOM_CONNECT_EVENT

#ifdef SERV_TIME_EVENT_ONLY_CURRENT_USER_CHAR
		bool	m_bNewUnitEvent;
		bool	m_bNewUnitEvent2;
		bool	m_bCurrentUnitEvent;
#endif //SERV_TIME_EVENT_ONLY_CURRENT_USER_CHAR
};
