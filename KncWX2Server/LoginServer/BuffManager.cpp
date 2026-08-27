#include "LoginSimLayer.h"
#include "BaseServer.h"
#include "buffmanager.h"
#include "X2data/XSLBuffManager.h"

//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM

ImplementSingleton( KBuffManager );

ImplToStringW( KBuffManager )
{
	return stm_;
}

KBuffManager::KBuffManager(void)
{
}

KBuffManager::~KBuffManager(void)
{
}


bool KBuffManager::AddUnitBuffInfo( IN const UidType& iUnitUID, IN const std::vector< KBuffInfo >& vecBuffInfo )
{
	//////////////////////////////////////////////////////////////////////////
	{
		// ���� ���� ����
		BOOST_TEST_FOREACH( const KBuffInfo&, kTestBuff, vecBuffInfo )
		{
			if( kTestBuff.m_kFactorInfo.m_vecBuffBehaviorFactors.size() > 10 )
			{
				START_LOG( cerr, "m_vecBuffBehaviorFactors�� ���� ������ 10���� �Ѿ���!" )
					<< BUILD_LOG( iUnitUID )
					<< BUILD_LOG( kTestBuff.m_kFactorInfo.m_vecBuffBehaviorFactors.size() )
					<< END_LOG;
			}

			if( kTestBuff.m_kFactorInfo.m_vecBuffFinalizerFactors.size() > 10 )
			{
				START_LOG( cerr, "m_vecBuffFinalizerFactors�� ���� ������ 10���� �Ѿ���!" )
					<< BUILD_LOG( iUnitUID )
					<< BUILD_LOG( kTestBuff.m_kFactorInfo.m_vecBuffFinalizerFactors.size() )
					<< END_LOG;
			}

			BOOST_TEST_FOREACH( const KBuffBehaviorFactor&, kBuffBFactor, kTestBuff.m_kFactorInfo.m_vecBuffBehaviorFactors )
			{
				if( kBuffBFactor.m_vecValues.size() > 10 )
				{
					START_LOG( cerr, "KBuffBehaviorFactor�� m_vecValues���� ������ 10���� �Ѿ���!" )
						<< BUILD_LOG( iUnitUID )
						<< BUILD_LOG( kBuffBFactor.m_vecValues.size() )
						<< END_LOG;
				}
			}

			BOOST_TEST_FOREACH( const KBuffFinalizerFactor&, kBuffFFactor, kTestBuff.m_kFactorInfo.m_vecBuffFinalizerFactors )
			{
				if( kBuffFFactor.m_vecValues.size() > 10 )
				{
					START_LOG( cerr, "KBuffFinalizerFactor�� m_vecValues���� ������ 10���� �Ѿ���!" )
						<< BUILD_LOG( iUnitUID )
						<< BUILD_LOG( kBuffFFactor.m_vecValues.size() )
						<< END_LOG;
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////

	std::map< UidType, KUnitBuffInfo >::iterator mit = m_mapUnitBuffList.find( iUnitUID );
	if( mit == m_mapUnitBuffList.end() )
	{
		KUnitBuffInfo kNewUnitBuffInfo;

		// ���� �ԷµǴ� ���̹Ƿ� �ð��� ��������.
		CTime tCurr = CTime::GetCurrentTime();
		kNewUnitBuffInfo.m_iInsertTime = tCurr.GetTime();
		BOOST_TEST_FOREACH( const KBuffInfo&, kNewBuffInfo, vecBuffInfo )
		{
			kNewUnitBuffInfo.m_mapBuffInfo.insert( std::make_pair( kNewBuffInfo.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID, kNewBuffInfo ) );
		}

		// ������ ������ ������ ���� �߰�����
		m_mapUnitBuffList.insert( std::make_pair( iUnitUID, kNewUnitBuffInfo ) );

		if( AddRenewBuffTime( iUnitUID, kNewUnitBuffInfo.m_iInsertTime ) == false )
		{
			// �ð� �߰� ����
			// ��������� ������ �߰� ���� ���Ѱ� �ƴϴϱ� ���� ó���� ���� �ʴ´�.
		}
		return true;
	}
	
	// ������ ������ �����Ѵ�.
	KUnitBuffInfo& kUnitBuffInfo = mit->second;

	// ���� �����߿� ������ �ִ��� ã�� ����.
	BOOST_TEST_FOREACH( const KBuffInfo&, kNewBuff, vecBuffInfo )
	{
		std::map< int, KBuffInfo >::iterator mitBI;
		mitBI = kUnitBuffInfo.m_mapBuffInfo.find( kNewBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID );
		if( mitBI == kUnitBuffInfo.m_mapBuffInfo.end() )
		{
			// ���� ������ �߰��Ѵ�.
			kUnitBuffInfo.m_mapBuffInfo.insert( std::make_pair( kNewBuff.m_kFactorInfo.m_BuffIdentity.m_eBuffTempletID, kNewBuff ) );
		}
		else
		{
			// ���� ����
			mitBI->second = kNewBuff;
		}
	}

	// ���� ���� �Ǿ����Ƿ� �ð��� �����Ѵ�.
	CTime tCurr = CTime::GetCurrentTime();
	__int64 iOldTime = kUnitBuffInfo.m_iInsertTime;
	__int64 iNewTime = tCurr.GetTime();

	if( AddRenewBuffTime( iUnitUID, iNewTime, iOldTime ) == true )
	{
		kUnitBuffInfo.m_iInsertTime = iNewTime;
	}
	else
	{
		// �ð� �߰� ����
		// ��������� ������ �߰� ���� ���Ѱ� �ƴϴϱ� ���� ó���� ���� �ʴ´�.
	}

	return true;
}

bool KBuffManager::DeleteUnitBuffInfo( IN UidType& iUnitUID, IN std::vector<int>& vecBuffID )
{
	std::map< UidType, KUnitBuffInfo >::iterator mit = m_mapUnitBuffList.find( iUnitUID );
	if( mit == m_mapUnitBuffList.end() )
	{
		START_LOG( clog, L"�������� �ʴ� Unit ������ ������� �մϴ�." )
			<< BUILD_LOG( iUnitUID )
			<< END_LOG;

		return false;
	}
	else
	{
		BOOST_TEST_FOREACH( const int, iBuffID, vecBuffID )
		{
			mit->second.m_mapBuffInfo.erase( iBuffID );
		}
		
		if( static_cast<int>(mit->second.m_mapBuffInfo.size()) <= 0 )
		{			
			// �ð� ���� �����
			DeleteBuffTime( mit->second.m_iInsertTime, mit->first );

			// ���� ���� �����
			m_mapUnitBuffList.erase( mit );
		}
	}

	return true;
}

bool KBuffManager::DeleteUnitAllBuffInfo( IN UidType& iUnitUID )
{
	std::map< UidType, KUnitBuffInfo >::iterator mit = m_mapUnitBuffList.find( iUnitUID );
	if( mit != m_mapUnitBuffList.end() )
	{
		START_LOG( clog, L"�������� �ʴ� Unit ������ ������� �մϴ�." )
			<< BUILD_LOG( iUnitUID )
			<< END_LOG;

		return false;
	}
	else
	{
		// �ð� ���� �����
		DeleteBuffTime( mit->second.m_iInsertTime, mit->first );

		// ���� ���� �����
		m_mapUnitBuffList.erase( mit );
	}

	return true;
}

bool KBuffManager::AddRenewBuffTime( IN const UidType& iUnitUID, IN const __int64& iNewTime, IN const __int64 iOleTime /* = 0*/ )
{
	// ������ �ð��� ���� �ԷµǾ� �־���.
	if( iOleTime > 0 )
	{
		// ���� ������ ã�Ƽ� ������
		std::map< __int64, std::vector<UidType> >::iterator mit = m_mapUnitTime.find( iOleTime );
		if( mit != m_mapUnitTime.end() )
		{
			std::vector<UidType>::iterator vit = mit->second.begin();
			for( ; vit != mit->second.end() ; ++vit )
			{
				if( iUnitUID == (*vit) )
				{
					// ��������
					mit->second.erase( vit );

					break;
				}
			}
		}
		else
		{
			START_LOG( clog, L"�ִٰ� �ߴµ� ����...?" )
				<< BUILD_LOG( iUnitUID )
				<< BUILD_LOG( iNewTime )
				<< BUILD_LOG( iOleTime )
				<< END_LOG;

			return false;
		}
	}

	std::map< __int64, std::vector<UidType> >::iterator mit = m_mapUnitTime.find( iNewTime );
	if( mit != m_mapUnitTime.end() )
	{
		BOOST_TEST_FOREACH( UidType, iUID, mit->second )
		{
			if( iUID == iUnitUID )
			{
				START_LOG( cerr, L"�Էµ� �����Ͱ� �ִµ�?" )
					<< BUILD_LOG( iUnitUID )
					<< END_LOG;

				// �ִٸ� �߰� ���ص� �ȴ�. ���д� �ƴ�
				return true;
			}
		}

		mit->second.push_back( iUnitUID );
	}
	else
	{
		std::vector<UidType> vecNew;
		vecNew.push_back( iUnitUID );

		m_mapUnitTime.insert( std::make_pair( iNewTime, vecNew ) );
	}

	return true;
}

void KBuffManager::DeleteBuffTime( IN __int64& iTime, IN UidType iUnitUID )
{
	// ���� ������ ã�Ƽ� ������
	std::map< __int64, std::vector<UidType> >::iterator mit = m_mapUnitTime.find( iTime );
	if( mit != m_mapUnitTime.end() )
	{
		std::vector<UidType>::iterator vit = mit->second.begin();
		for( ; vit != mit->second.end() ; ++vit )
		{
			if( iUnitUID == (*vit) )
			{
				// ��������
				mit->second.erase( vit );

				break;
			}
		}
	}
	else
	{
		START_LOG( clog, L"�ִٰ� �ߴµ� ����...?" )
			<< BUILD_LOG( iUnitUID )
			<< BUILD_LOG( iTime )
			<< END_LOG;
	}
}

bool KBuffManager::GetUnitAllBuffInfo( IN UidType& iUnitUID, OUT KUnitBuffInfo& kUnitBuffInfo )
{
	std::map< UidType, KUnitBuffInfo >::iterator mit = m_mapUnitBuffList.find( iUnitUID );
	if( mit != m_mapUnitBuffList.end() )
	{
		// �����͸� ��û �ߴٴ� ���� ������ ������ ���� ���� �ð��� ���� ���Ѿ� �Ѵ�.
		CTime tCurr = CTime::GetCurrentTime();
		__int64 iOldTime = mit->second.m_iInsertTime;
		__int64 iNewTime = tCurr.GetTime();
		
		if( AddRenewBuffTime( iUnitUID, iNewTime, iOldTime ) == true )
		{
			mit->second.m_iInsertTime = iNewTime;
		}
		else
		{
			// �ð� �߰� ����
			// ��������� ������ �߰� ���� ���Ѱ� �ƴϴϱ� ���� ó���� ���� �ʴ´�.
		}

		kUnitBuffInfo = mit->second;
	}
	else
	{
		START_LOG( clog, L"�����Ͱ� ����." )
			<< BUILD_LOG( iUnitUID )
			<<END_LOG;
		return false;
	}

	return true;
}

void KBuffManager::CheckEndBuff()
{
	// ���� �ð�
	CTime tCurr = CTime::GetCurrentTime();
	// ���� ���� �ð�
	CTimeSpan tSpan = CTimeSpan( 0, CXSLBuffManager::UBKT_UNIT_RESET_TIME_HOUR, 0, 0);
	CTime tDeleteCurr;
	tDeleteCurr = tCurr - tSpan;

	// ������ unit �� ���� �ص� �����̳�
	std::set< UidType > setUnitList;

	// ���� �ð����� ���� �ð��� ������ ã�Ƽ� �����. - �ð� ���� ���� �ǹǷ� ó�� ���� �˻�
	std::map< __int64, std::vector<UidType> >::iterator mit = m_mapUnitTime.begin();
	for( ; mit != m_mapUnitTime.end() ; ++mit )
	{
		if( mit->first > tDeleteCurr.GetTime() )
		{
			START_LOG( clog, L"���� �Ǿ�� �� �ð����� ũ�Ƿ� ������ �����." )
				<< BUILD_LOG( mit->first )
				<< BUILD_LOG( tDeleteCurr.GetTime() )
				<< END_LOG;
			break;
		}

		BOOST_TEST_FOREACH( UidType, iUnitUID, mit->second )
		{
			setUnitList.insert( iUnitUID );
		}
	}

	std::set< UidType >::iterator sit = setUnitList.begin();
	for( ; sit != setUnitList.end() ; ++sit )
	{
		//{{ Iruha : 2026-08-27 // VS2010 port: VC7.1's std::set::iterator non-conformantly
		// dereferenced to a mutable reference; VC10 dereferences to const, and
		// DeleteUnitAllBuffInfo takes UidType& (non-const). Copy into a local instead.
		UidType uidUnit = *sit;
		DeleteUnitAllBuffInfo( uidUnit );
		//}}
	}
}


#endif SERV_SERVER_BUFF_SYSTEM
//}
