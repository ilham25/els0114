#pragma once
#include "Event.h"
#include "CommonPacket.h"


//{{ 2012. 03. 20	최육사	배틀필드 시스템
#ifdef SERV_BATTLE_FIELD_SYSTEM


class KUserSpiritManager
{
public:
	KUserSpiritManager(void);
	~KUserSpiritManager(void);

	void	Clear();
	void	Init( IN const int iSpirit, IN const int iSpiritMax );	
    
	int		GetSpirit() const										{ return m_iSpirit; }
	void	SetSpirit( IN const int iSpirit )						{ m_iSpirit = iSpirit; }
	int		GetSpiritMax() const									{ return m_iSpiritMax; }
	void	SetSpiritMax( IN const int iSpiritMax )					{ m_iSpiritMax = iSpiritMax; }
	bool	IsSpiritUpdated() const									{ return m_bIsSpiritUpdated; }
	void	SetIsSpiritUpdated( IN const bool bIsSpiritUpdated )	{ m_bIsSpiritUpdated =  bIsSpiritUpdated; }

	int		RestoreSpirit( IN const int iRestoreSpirit );

	bool	CheckingSpirit( IN const int iDungeonID, IN const bool bIsPcBang ) const;
	int		CalculateDecreaseSpirit( IN const char cUserCount, 
									 IN const short sStageNpcCount, 
									 IN const bool bIsTutorial, 
									 IN const bool bHalfDecreaseEvent,
									 IN const bool bIsBattleField );

	bool	DecreaseSpirit( IN const int iDungeonID
						  , IN const char cUserCount
						  , IN const short sStageNpcCount
						  , IN const bool bIsTutorial
						  , IN const bool bIsPcBang
						  , IN const bool bIsBattleField
						  , OUT int& iDecreaseSpirit
						  , OUT bool& bIsSpiritUpdated
#if defined( SERV_LOCAL_RANKING_SYSTEM ) || defined( SERV_CHINA_SPIRIT_EVENT )
						  , OUT int& iAbsoluteDecreaseSpirit
#endif	// SERV_LOCAL_RANKING_SYSTEM
						  );

	void	GetDBUpdateInfo( OUT int& iSpirit, OUT bool& bIsSpiritUpdated );

private:
	int								m_iSpiritMax;
	int								m_iSpirit;
	bool							m_bIsSpiritUpdated;		// 근성도 하루치를 다 썼을경우 true
};


#endif SERV_BATTLE_FIELD_SYSTEM
//}}