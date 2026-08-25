#pragma once

#include "GameEvent.h"

class KGameEventItemDrop : public KGameEvent
{
public:
	KGameEventItemDrop(void);
	virtual ~KGameEventItemDrop(void);

	//{{ 2010. 07. 09  최육사	드롭률 이벤트 확장	
	void	SetDropCount( int iDropCount )			{ m_iDropCount = iDropCount; }
	int		GetDropCount()							{ return m_iDropCount; }

	void	SetWithPlayPcBang( bool bVal )			{ m_bWithPlayPcBang = bVal; }
	bool	IsWithPlayPcBang()						{ return m_bWithPlayPcBang; }

	//{{ 2012. 03. 28	박세훈	드롭률 설정 이벤트 개편 Merge 작업 ( 2012.03.20 lygan_조성욱 )
#ifdef DROPEVENT_RENEWAL
	void	SetDungeonID( int iDungeonID)			{ m_iDungeongID = iDungeonID; }
	int		GetDungeonID()							{ return m_iDungeongID; }

	void	SetUnitType( char cUnitType )			{ m_cUnitType = cUnitType; }
	int		GetUnitType()							{ return m_cUnitType; }
	void	SetUnitClass( char cUnitClass )			{ m_cUnitClass = cUnitClass; }
	int		GetUnitClass()							{ return m_cUnitClass; }
	void	SetStartLevel(int iStartLevel)			{m_iStartLevel =  iStartLevel; }
	int		GetStartLevel()							{return m_iStartLevel; }
	void	SetEndLevel(int iEndLevel)				{m_iEndLevel =  iEndLevel; }
	int		GetEndLevel()							{return m_iEndLevel; }


#endif //DROPEVENT_RENEWAL

private:
	int		m_iDropCount;
	bool	m_bWithPlayPcBang;

	//{{ 2012. 03. 28	박세훈	드롭률 설정 이벤트 개편 Merge 작업 ( 2012.03.20 lygan_조성욱 )
#ifdef DROPEVENT_RENEWAL
	int		m_iDungeongID;
	char	m_cUnitType;
	char	m_cUnitClass;
	int		m_iStartLevel;
	int		m_iEndLevel;
#endif //DROPEVENT_RENEWAL
	//}}
};
