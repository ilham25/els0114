#include "GSUser.h"
#include ".\UserRankingManager.h"
#include "GSRankingManager.h"


KUserRankingManager::KUserRankingManager(void) :
m_bInit( false ),
m_bChanged( false )
{	
}

KUserRankingManager::~KUserRankingManager(void)
{
}

void KUserRankingManager::clear()
{
	m_bInit = false;
	m_bChanged = false;
}

void KUserRankingManager::Init( IN const std::vector< KHenirRankingInfo >& vecRanking )
{
	std::vector< KHenirRankingInfo >::const_iterator vit = vecRanking.begin();
	if( vit != vecRanking.end() )
	{
        m_kHenirRanking = *vit;
        m_bInit = true;
	}
}

void KUserRankingManager::GetUpdateInfo( OUT std::vector< KHenirRankingInfo >& vecRanking )
{
	vecRanking.clear();

	if( m_bInit  &&  m_bChanged )
	{
		vecRanking.push_back( m_kHenirRanking );
	}
}

void KUserRankingManager::GetRankingInfo( IN KGSUserPtr spUser, IN const KEGS_GET_RANKING_INFO_REQ& kReq, OUT KEGS_GET_RANKING_INFO_ACK& kAck )
{
	switch( kReq.m_iRankingType )
	{
	case SEnum::RT_DAY_RANKING:
	case SEnum::RT_WEEK_RANKING:
	case SEnum::RT_MONTH_RANKING:
		{
			if( m_bInit )
			{
                kAck.m_vecMyHenirRanking.push_back( m_kHenirRanking );
			}
		}
		break;

	case SEnum::RT_DUNGEON_RANKING:
		{
			KDungeonRankingInfo kRankingInfo;
			if( SiKGSRankingManager()->GetDungeonRankingByUnitUID( spUser->GetCharUID(), kRankingInfo ) == true )
			{
				kAck.m_vecMyDungeonRanking.push_back( kRankingInfo );
			}
		}
		break;

	case SEnum::RT_PVP_RANKING:
		{
			KPvpRankingInfo kRankingInfo;
			if( SiKGSRankingManager()->GetPvpRankingByUnitUID( spUser->GetCharUID(), kRankingInfo ) == true )
			{
				kAck.m_vecMyPvpRanking.push_back( kRankingInfo );
			}
		}
		break;

	default:
		{
			START_LOG( cerr, L"존재하지 않는 RankingType입니다" )
				<< BUILD_LOG( kReq.m_iRankingType )
				<< END_LOG;
		}
	}
}

void KUserRankingManager::SetRankingInfo( IN const KHenirRankingInfo& kInfo )
{
	m_kHenirRanking = kInfo;
	m_bInit = true;
	m_bChanged = true;
}


