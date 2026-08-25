#include "GameEventHenirReward.h"

//{{ 2011. 08. 12   김민성      헤니르 개편 
#ifdef SERV_NEW_HENIR_TEST

KGameEventHenirReward::KGameEventHenirReward(void) : 
m_iHenirRewardEventCount(0),
//{{ 2011. 12. 06	최육사	특정 서버군 이벤트 시스템
#ifdef SERV_SERVER_GROUP_EVENT_SYSTEM
m_iServerGroupID(-1),
#endif SERV_SERVER_GROUP_EVENT_SYSTEM
//}}
m_bHenirRewardUnLimited( false )
{
}

KGameEventHenirReward::~KGameEventHenirReward(void)
{
}

#endif SERV_NEW_HENIR_TEST
//}}
