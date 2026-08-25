#pragma once
#include "CommonPacket.h"


SmartPointer(KGSUser);

class KUserRankingManager
{
public:
	KUserRankingManager(void);
	~KUserRankingManager(void);

	void clear();
	void Init( IN const std::vector< KHenirRankingInfo >& vecRanking );
	void GetUpdateInfo( OUT std::vector< KHenirRankingInfo >& vecRanking );

	void GetRankingInfo( IN KGSUserPtr spUser, IN const KEGS_GET_RANKING_INFO_REQ& kReq, OUT KEGS_GET_RANKING_INFO_ACK& kAck );
	void SetRankingInfo( IN const KHenirRankingInfo& kInfo );

private:
	KHenirRankingInfo		m_kHenirRanking;
	bool					m_bInit;
	bool					m_bChanged;
};
