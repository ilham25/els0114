#include "GameServer.h"
#include ".\GSWorldMissionManager.h"
#include "Enum/Enum.h"
#include "socket/NetCommon.h"

//{{ 2011. 04. 13  ��μ�  �۷ι� ���� �߰�
#ifdef SERV_INSERT_GLOBAL_SERVER

//{{ 2012. 04. 23	�ڼ���	����� �� ����
#ifdef SERV_REFORM_THE_GATE_OF_DARKNESS
	#include "..\Common\X2Data\XSLBuffManager.h"
#endif SERV_REFORM_THE_GATE_OF_DARKNESS
//}}

ImplementSingleton( KGSWorldMissionManager );

//{{ 2013. 04. 15	�ڼ���	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
KGSWorldMissionManager::KGSWorldMissionManager(void)
{	
	m_bActive = false;

	m_iDungeonTargetCount = 0;
	m_iWorldCrystalCount = 0;
	m_iWorldMaxCrystalCount = 0;

	m_iMissionTime = 0;

	m_iDungeonPlayTime = 0;

	m_iTickCount = 0;
	m_usWorldBuff = CXSLBuffManager::BTI_NONE;
	m_usWorldBuffDurationTime = 0;

	m_iIndividualBuffID	= CXSLBuffManager::BTI_NONE;
	m_tWorldMissionEndTime = CTime( 0 );
}

KGSWorldMissionManager::~KGSWorldMissionManager(void)
{
}

void KGSWorldMissionManager::Init()
{
	// �α��� ������ ���� �̺�Ʈ ���� ������ ���
	KEGB_GET_MISSION_INFO_REQ kPacketReq;
	kPacketReq.m_iServerUID = KBaseServer::GetKObj()->GetUID();
	SendToGlobalServer( EGB_GET_MISSION_INFO_REQ, kPacketReq );
}

void KGSWorldMissionManager::UpdateMissionInfo( IN const KEGB_GET_MISSION_INFO_ACK& kPacketAck )
{
	switch( kPacketAck.m_iFlag )
	{
	case KEGB_GET_MISSION_INFO_ACK::FT_NONE:
		{
			START_LOG( cerr, L"�޾Ƽ��� �ȵǴ� Ÿ��" )
				<< BUILD_LOG( kPacketAck.m_iFlag )
				<< END_LOG;

			return;
		} break;
	
	case KEGB_GET_MISSION_INFO_ACK::FT_INIT:
			m_mapDefenseOpenTime = kPacketAck.m_mapDefenseOpenTime;
	case KEGB_GET_MISSION_INFO_ACK::FT_UPDATE:
	case KEGB_GET_MISSION_INFO_ACK::FT_BEGIN:
		{
			m_bActive					= kPacketAck.m_bActive;
			m_iWorldCrystalCount		= kPacketAck.m_iWorldCrystalCount;
			m_iWorldMaxCrystalCount		= kPacketAck.m_iWorldMaxCrystalCount;
			m_iMissionTime				= kPacketAck.m_iMissionTime;
			m_iDungeonPlayTime			= kPacketAck.m_iDungeonPlayTime;
			m_usWorldBuffDurationTime	= kPacketAck.m_usWorldBuffDurationTime;

			if( kPacketAck.m_iFlag == KEGB_GET_MISSION_INFO_ACK::FT_BEGIN )
			{
				m_usWorldBuff				= CXSLBuffManager::BTI_NONE;
				m_usWorldBuffDurationTime	= 0;
				m_kTimer.restart();
			}
		}break;
	case KEGB_GET_MISSION_INFO_ACK::FT_SUCCESS:
	case KEGB_GET_MISSION_INFO_ACK::FT_FAIL:
		{
			m_bActive					= kPacketAck.m_bActive;
			m_iWorldCrystalCount		= kPacketAck.m_iWorldCrystalCount;
			m_iWorldMaxCrystalCount		= kPacketAck.m_iWorldMaxCrystalCount;
			m_iMissionTime				= kPacketAck.m_iMissionTime;
			m_iDungeonPlayTime			= kPacketAck.m_iDungeonPlayTime;
			m_usWorldBuffDurationTime	= kPacketAck.m_usWorldBuffDurationTime;
		} break;
	default:
		{
			START_LOG( cerr, L"�޾Ƽ��� �ȵǴ� Ÿ��" )
				<< BUILD_LOG( kPacketAck.m_iFlag )
				<< END_LOG;

			return;
		}break;
	}

	m_iTickCount++;

	START_LOG( clog, L"���� �̼� ���� ����" )
		<< BUILD_LOG( kPacketAck.m_iFlag )
		<< BUILD_LOG( kPacketAck.m_bActive )
		<< BUILD_LOG( kPacketAck.m_iWorldCrystalCount )
		<< BUILD_LOG( kPacketAck.m_iWorldMaxCrystalCount )
		<< BUILD_LOG( kPacketAck.m_iMissionTime )
		<< BUILD_LOG( m_iTickCount )
		<< END_LOG;
}

void KGSWorldMissionManager::GetMissionInfo( OUT KEGS_WORLD_MISSION_UPDATE_NOT &kOut )
{
	kOut.m_bActive					= m_bActive;
	kOut.m_iRemainTime				= GetRemainTime();
	kOut.m_iWorldCrystalCount		= m_iWorldCrystalCount;
	kOut.m_iWorldMaxCrystalCount	= m_iWorldMaxCrystalCount;
	kOut.m_iMissionStartRemainTime	= GetMissionStartRemainTime();

	START_LOG( clog, L"���� �̼� ����" )
		<< BUILD_LOG( kOut.m_bActive )
		<< BUILD_LOG( kOut.m_iRemainTime )
		<< BUILD_LOG( kOut.m_iWorldCrystalCount )
		<< BUILD_LOG( kOut.m_iMissionStartRemainTime )
		<< END_LOG;
}

bool KGSWorldMissionManager::UpdateWorldBuffInfo( IN const KEGB_UPDATE_WORLD_BUFF_NOT& kPacket )
{
	m_bActive = kPacket.m_bActive;

	if( kPacket.m_usSuccessWorldBuff == m_usWorldBuff )
	{
		return false;
	}

	m_usWorldBuff				= kPacket.m_usSuccessWorldBuff;
	m_usWorldBuffDurationTime	= kPacket.m_usWorldBuffDurationTime;

	if( m_usWorldBuff == CXSLBuffManager::BTI_NONE )	// �ǵ����̸� ���� �ȿ� ������ üũ�ϴ� ���� ����.
	{
		START_LOG( clog, L"���� ������ ���� �Ǿ����ϴ�! ���� ���� ����!!" )
			<< BUILD_LOG( m_usWorldBuff )
			<< BUILD_LOG( m_usWorldBuffDurationTime )
			<< END_LOG;
	}
	else
	{
		START_LOG( clog, L"���� ������ ���� �Ǿ����ϴ�! ���� ���� ����!!" )
			<< BUILD_LOG( m_usWorldBuff )
			<< BUILD_LOG( m_usWorldBuffDurationTime )
			<< END_LOG;
	}

	return true;
}

void KGSWorldMissionManager::GetWorldBuff( OUT KEGS_UPDATE_WORLD_BUFF_NOT& kPacket )
{
	kPacket.m_usSuccessWorldBuff		= m_usWorldBuff;
	kPacket.m_usWorldBuffDurationTime	= GetWorldBuffEndTime();
}

int KGSWorldMissionManager::GetWorldBuffState()
{
	int ret = CXSLBuffManager::BTI_NONE;

	if( m_usWorldBuffDurationTime > 0 && m_usWorldBuff > CXSLBuffManager::BTI_NONE )
	{
		ret = static_cast<int>(m_usWorldBuff);
	}

	return ret;
}

//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
int KGSWorldMissionManager::GetWorldBuffEndTime()
{
	CTime tCurrentTime = CTime::GetCurrentTime();

	// ���Ϻ� ���� �ð� ���
	std::map< int, std::vector< KDefenseOpenTime > >::const_iterator mit = m_mapDefenseOpenTime.find( tCurrentTime.GetDayOfWeek() );

	// �ش� ���Ͽ� ����� ���� �ð� ���� ���� �˻�.
	if( mit == m_mapDefenseOpenTime.end() )
	{
		return CXSLBuffManager::BTI_NONE;
	}

	// ���� �ð�
	const std::vector< KDefenseOpenTime >& vecTime = mit->second;

	// �ش� ���Ͽ� ����� ���� �ð����� ��ȸ�ϸ鼭, ���� �ð��뿡 �´� ������ �ִ��� �˻��Ѵ�.
	//{{ Iruha : 2026-08-27 // VS2010 port: it is used after the loop, which VC7.1's
	// non-conformant /Zc:forScope- tolerated; VC10 requires the loop variable to outlive
	// the loop, so it's hoisted here.
	std::vector< KDefenseOpenTime >::const_iterator it;
	//}}
	for( it = vecTime.begin(); it != vecTime.end(); ++it )
	{
		const KDefenseOpenTime& kDefenseOpenTime = *it;
		CTime tStartTime	= CTime( tCurrentTime.GetYear(), tCurrentTime.GetMonth(), tCurrentTime.GetDay(), kDefenseOpenTime.m_iStartHour, kDefenseOpenTime.m_iStartMinute, 0 );
		CTime tBuffEndTime	= tStartTime + CTimeSpan( 0, 0, kDefenseOpenTime.m_iDurationTime + m_usWorldBuffDurationTime, 0 );

		if( ( tStartTime <= tCurrentTime ) &&
			( tCurrentTime < tBuffEndTime )
			)
		{
			break;
		}
	}

	if( it == vecTime.end() )
	{
		return 0;
	}
	else
	{
		m_tWorldMissionEndTime = CTime( tCurrentTime.GetYear(), tCurrentTime.GetMonth(), tCurrentTime.GetDay(), it->m_iStartHour, it->m_iStartMinute + it->m_iDurationTime, 0 );
		
		__int64 iCurrent = tCurrentTime.GetTime();
		int iBuffEndTime = static_cast<int>( ( m_tWorldMissionEndTime.GetTime() + m_usWorldBuffDurationTime ) - iCurrent );

		return iBuffEndTime;
	}
}
#endif SERV_SERVER_BUFF_SYSTEM
//}

//{{ 2013. 04. 15	�ڼ���	����� �� ����
#ifdef SERV_NEW_DEFENCE_DUNGEON
bool KGSWorldMissionManager::UpdateIndividualBuffInfo( IN const KEGB_THE_GATE_OF_DARKNESS_INDIVIDUAL_BUFF_NOT& kPacket_ )
{
	if( m_iIndividualBuffID != kPacket_.m_iTheGateOfDarknessIndividualBuffLevel )
	{
		m_iIndividualBuffID = kPacket_.m_iTheGateOfDarknessIndividualBuffLevel;
		return true;
	}

	return false;
}
#endif SERV_NEW_DEFENCE_DUNGEON
//}}

//{{ 2012. 03. 03	�ڼ���	����� �� ����
#ifdef SERV_REFORM_THE_GATE_OF_DARKNESS
int KGSWorldMissionManager::GetWorldCrystalCount( void )
{ 
	if( m_iWorldCrystalCount > WBSC_STEP_THREE )	// ������ �ִ� ���� ���� ��� �ִ� ���� ����(client UI's crystal)
	{
		m_iWorldCrystalCount = WBSC_STEP_THREE;
	}

	START_LOG( clog, L"���� ũ����Ż ���� Ȯ��" )
		<< BUILD_LOG( m_iWorldCrystalCount )
		<< END_LOG;

	return m_iWorldCrystalCount; 
}

bool KGSWorldMissionManager::GetDefenseDungeonEndTime( OUT CTime& tStartTime, OUT CTime& tEndTime ) const
{
	CTime tCurrentTime = CTime::GetCurrentTime();

	// ���Ϻ� ���� �ð� ���
	std::map< int, std::vector< KDefenseOpenTime > >::const_iterator mit = m_mapDefenseOpenTime.find( tCurrentTime .GetDayOfWeek() );

	// �ش� ���Ͽ� ����� ���� �ð� ���� ���� �˻�.
	if( mit == m_mapDefenseOpenTime.end() )
	{
		START_LOG( cerr, L"���潺 ���� �ð� ���� ���� ���� ���� �ʽ��ϴ�." );
		return false;
	}

	// ���� �ð�
	const std::vector< KDefenseOpenTime >& vecTime = mit->second;

	// �ش� ���Ͽ� ����� ���� �ð����� ��ȸ�ϸ鼭, ���� �ð��뿡 �´� ������ �ִ��� �˻��Ѵ�.
	for( std::vector< KDefenseOpenTime >::const_reverse_iterator rit = vecTime.rbegin(); rit != vecTime.rend(); ++rit )
	{
		const KDefenseOpenTime& kDefenseOpenTime = *rit;
		tStartTime = CTime( tCurrentTime .GetYear(), tCurrentTime .GetMonth(), tCurrentTime .GetDay(), kDefenseOpenTime.m_iStartHour, kDefenseOpenTime.m_iStartMinute, 0 );
		tEndTime = tStartTime + CTimeSpan( 0, 0, kDefenseOpenTime.m_iDurationTime, 0 );

		// ������ ���� �ִٸ�, ���� �ð��� �����ϴ� ���� ���� ���� �ð��� ã�´�.
		if( m_bActive )
		{
			if( ( tStartTime <= tCurrentTime ) && ( tCurrentTime < tEndTime ) )
			{
				return true;
			}
		}
		else if( tEndTime <= tCurrentTime )
		{
			return true;
		}
	}

	std::wstring wstrStartTime = tStartTime.Format(_T("%Y-%m-%d %H:%M:%S"));
	std::wstring wstrEndTime = tEndTime.Format(_T("%Y-%m-%d %H:%M:%S"));
	std::wstring wstrCurrentTime = tCurrentTime.Format(_T("%Y-%m-%d %H:%M:%S"));

	START_LOG( cerr, L"���潺 ���� EndTime �˻� ����")
		<< BUILD_LOG( m_bActive )
		<< BUILD_LOG( wstrCurrentTime )
		<< BUILD_LOG( wstrStartTime )
		<< BUILD_LOG( wstrEndTime )
		<< END_LOG;

	return false;
}
#endif SERV_REFORM_THE_GATE_OF_DARKNESS
//}}

int KGSWorldMissionManager::GetRemainTime()
{
	if( m_bActive ==true )
	{
		int iTime = static_cast<int>( m_iMissionTime - m_kTimer.elapsed() );
		return iTime > 0 ? iTime : 0;
	}
	else
	{
		return 0;
	}
}

void KGSWorldMissionManager::UpdateMissionStartTime( IN std::wstring& wstrDate )
{
	if( KncUtil::ConvertStringToCTime( wstrDate, m_tWorldMissionNextStartTime ) == false )
	{
		START_LOG( cerr, L"�ð� ���� ����!" )
			<< BUILD_LOG( m_tWorldMissionNextStartTime.GetYear() )
			<< BUILD_LOG( m_tWorldMissionNextStartTime.GetMonth() )
			<< BUILD_LOG( m_tWorldMissionNextStartTime.GetDay() )
			<< BUILD_LOG( m_tWorldMissionNextStartTime.GetHour() )
			<< BUILD_LOG( m_tWorldMissionNextStartTime.GetMinute() )
			<< BUILD_LOG( m_tWorldMissionNextStartTime.GetSecond() )
			<< BUILD_LOG( wstrDate )
			<< END_LOG;
	}
}

int KGSWorldMissionManager::GetMissionStartRemainTime()
{
	CTime tCurrent = CTime::GetCurrentTime();

	if( m_tWorldMissionNextStartTime > tCurrent )	// ���� �̼��� ���Ŀ� �����Ѵ�
	{
		CTimeSpan gap = m_tWorldMissionNextStartTime - tCurrent;

		return (int)gap.GetTotalMinutes();
	}
	else if( m_iMissionTime == 0 )					// ���� �̼� ���Ŀ� ����
	{
		return -1;
	}
	else											// ���� �̼��� ���� ���̴�
	{
		return 0;
	}
}
#else
KGSWorldMissionManager::KGSWorldMissionManager(void)
{	
	m_bActive = false;

	m_iDungeonTargetCount = 0;
	m_iWorldCrystalCount = 0;

	m_iMissionTime = 0;

	m_iDungeonPlayTime = 0;

	m_iTickCount = 0;
	m_usWorldBuff = CXSLBuffManager::BTI_NONE;
	m_usWorldBuffDurationTime = 0;
}

KGSWorldMissionManager::~KGSWorldMissionManager(void)
{
}

void KGSWorldMissionManager::Init()
{
	// �α��� ������ ���� �̺�Ʈ ���� ������ ���
	KEGB_GET_MISSION_INFO_REQ kPacketReq;
	kPacketReq.m_iServerUID = KBaseServer::GetKObj()->GetUID();
	SendToGlobalServer( EGB_GET_MISSION_INFO_REQ, kPacketReq );
}

void KGSWorldMissionManager::UpdateMissionInfo( IN const KEGB_GET_MISSION_INFO_ACK& kPacketAck )
{
	switch( kPacketAck.m_iFlag )
	{
	case KEGB_GET_MISSION_INFO_ACK::FT_NONE:
		{
			START_LOG( cerr, L"�޾Ƽ��� �ȵǴ� Ÿ��" )
				<< BUILD_LOG( kPacketAck.m_iFlag )
				<< END_LOG;

			return;
		} break;
		//{{ 2012. 04. 24	�ڼ���	����� �� ����
#ifdef SERV_REFORM_THE_GATE_OF_DARKNESS
		case KEGB_GET_MISSION_INFO_ACK::FT_INIT:	m_mapDefenseOpenTime = kPacketAck.m_mapDefenseOpenTime;	goto UPDATE_MISSION_INFO_FT_PROCESS;
		case KEGB_GET_MISSION_INFO_ACK::FT_UPDATE:	goto UPDATE_MISSION_INFO_FT_PROCESS;
		case KEGB_GET_MISSION_INFO_ACK::FT_BEGIN:	goto UPDATE_MISSION_INFO_FT_PROCESS;
UPDATE_MISSION_INFO_FT_PROCESS:
#else
	case KEGB_GET_MISSION_INFO_ACK::FT_UPDATE:
	case KEGB_GET_MISSION_INFO_ACK::FT_INIT:
	case KEGB_GET_MISSION_INFO_ACK::FT_BEGIN:
#endif SERV_REFORM_THE_GATE_OF_DARKNESS
		//}}
		{
			m_bActive				= kPacketAck.m_bActive;
			m_iWorldCrystalCount	= kPacketAck.m_iWorldCrystalCount;
			m_iMissionTime			= kPacketAck.m_iMissionTime;
			m_iDungeonPlayTime		= kPacketAck.m_iDungeonPlayTime;

			if( kPacketAck.m_iFlag == KEGB_GET_MISSION_INFO_ACK::FT_BEGIN )
			{
				m_usWorldBuff				= CXSLBuffManager::BTI_NONE;
				m_usWorldBuffDurationTime	= 0;
				m_kTimer.restart();
			}
		}break;
	case KEGB_GET_MISSION_INFO_ACK::FT_SUCCESS:
	case KEGB_GET_MISSION_INFO_ACK::FT_FAIL:
		{
			m_bActive				= kPacketAck.m_bActive;
			m_iWorldCrystalCount	= kPacketAck.m_iWorldCrystalCount;
			m_iMissionTime			= kPacketAck.m_iMissionTime;
			m_iDungeonPlayTime		= kPacketAck.m_iDungeonPlayTime;
			m_tWorldMissionEndTime = CTime::GetCurrentTime();
		} break;
	default:
		{
			START_LOG( cerr, L"�޾Ƽ��� �ȵǴ� Ÿ��" )
				<< BUILD_LOG( kPacketAck.m_iFlag )
				<< END_LOG;

			return;
		}break;
	}

	m_iTickCount++;

	START_LOG( clog, L"���� �̼� ���� ����" )
		<< BUILD_LOG( kPacketAck.m_iFlag )
		<< BUILD_LOG( kPacketAck.m_bActive )
		<< BUILD_LOG( kPacketAck.m_iWorldCrystalCount )
		<< BUILD_LOG( kPacketAck.m_iMissionTime )
		<< BUILD_LOG( m_iTickCount )
		<< END_LOG;
}

void KGSWorldMissionManager::GetMissionInfo( OUT KEGS_WORLD_MISSION_UPDATE_NOT &kOut )
{
	kOut.m_bActive					= m_bActive;
	kOut.m_iRemainTime				= GetRemainTime();
	kOut.m_iWorldCrystalCount		= m_iWorldCrystalCount;
	kOut.m_iMissionStartRemainTime	= GetMissionStartRemainTime();

	START_LOG( clog, L"���� �̼� ����" )
		<< BUILD_LOG( kOut.m_bActive )
		<< BUILD_LOG( kOut.m_iRemainTime )
		<< BUILD_LOG( kOut.m_iWorldCrystalCount )
		<< BUILD_LOG( kOut.m_iMissionStartRemainTime )
		<< END_LOG;
}

bool KGSWorldMissionManager::UpdateWorldBuffInfo( IN const KEGB_UPDATE_WORLD_BUFF_NOT& kPacket )
{
#ifdef SERV_REFORM_THE_GATE_OF_DARKNESS_NO_WORLD_BUFF
#else //SERV_REFORM_THE_GATE_OF_DARKNESS_NO_WORLD_BUFF
	if( kPacket.m_usSuccessWorldBuff != CXSLBuffManager::BTI_NONE && m_usWorldBuff < kPacket.m_usSuccessWorldBuff ) // ���� ���� �������� ���� ���̸� ���� �ʿ�
	{
		m_usWorldBuff = kPacket.m_usSuccessWorldBuff;

		if( kPacket.m_bActive == true )
		{
			m_usWorldBuffDurationTime = kPacket.m_usWorldBuffDurationTime;
		}
		else
		{
			CTime tCurrent = CTime::GetCurrentTime();

			CTimeSpan gap  = tCurrent - m_tWorldMissionEndTime;
			
			m_usWorldBuffDurationTime = static_cast<u_short>( kPacket.m_usWorldBuffDurationTime - gap.GetTotalSeconds() );
		}

		START_LOG( clog, L"���� ������ ���� �Ǿ����ϴ�! ���� ���� ����!!" )
			<< BUILD_LOG( m_usWorldBuff )
			<< BUILD_LOG( m_usWorldBuffDurationTime )
			<< END_LOG;

		return true;	// �� ���� �߰� �Ǵ� ��Ȳ
	}

	if( kPacket.m_usSuccessWorldBuff == CXSLBuffManager::BTI_NONE )
	{
		m_usWorldBuff = kPacket.m_usSuccessWorldBuff;
		m_usWorldBuffDurationTime = 0;

		START_LOG( clog, L"���� ������ ���� �Ǿ����ϴ�! ���� ���� ����!!" )
			<< BUILD_LOG( m_usWorldBuff )
			<< BUILD_LOG( m_usWorldBuffDurationTime )
			<< END_LOG;

		return true;
	}
#endif //SERV_REFORM_THE_GATE_OF_DARKNESS_NO_WORLD_BUFF
	
	return false;		// �� ���� �߰� ���� ����
}

void KGSWorldMissionManager::GetWorldBuff( OUT KEGS_UPDATE_WORLD_BUFF_NOT& kPacket )
{
	kPacket.m_usSuccessWorldBuff = m_usWorldBuff;
	
	if( m_bActive == true )
	{
		kPacket.m_usWorldBuffDurationTime = m_usWorldBuffDurationTime;
	}
	else
	{
		CTime tCurrent = CTime::GetCurrentTime();
		std::wstring wstr = m_tWorldMissionEndTime.Format(_T("%Y-%m-%d %H:%M:%S"));
		CTimeSpan gap  = tCurrent - m_tWorldMissionEndTime;
				
		// ���� ���� �ð� ���� ���� �۾ƾ� �Ѵ�.
		if( gap.GetTotalSeconds() < m_usWorldBuffDurationTime )
		{
			kPacket.m_usWorldBuffDurationTime = static_cast<u_short>(m_usWorldBuffDurationTime - gap.GetTotalSeconds());
		}
		else
		{
			kPacket.m_usWorldBuffDurationTime = 0;

			START_LOG( clog, L"���� ���� ���� �ð��� �̻��մϴ�" )
				<< BUILD_LOG( m_usWorldBuff )
				<< BUILD_LOG( m_usWorldBuffDurationTime )
				<< BUILD_LOG( gap.GetTotalSeconds()  )
				<< END_LOG;
		}
	}
}

int KGSWorldMissionManager::GetWorldBuffState()
{
	int ret = CXSLBuffManager::BTI_NONE;

	if( m_usWorldBuffDurationTime > 0 && m_usWorldBuff > CXSLBuffManager::BTI_NONE )
	{
		ret = static_cast<int>(m_usWorldBuff);
	}

	return ret;
}

//{{ ���� ���� �ý��� - ��μ�
#ifdef SERV_SERVER_BUFF_SYSTEM
int KGSWorldMissionManager::GetWorldBuffEndTime()
{
	CTime tCurr = CTime::GetCurrentTime();
	int iBuffEndTime = 0;

	if( m_bActive == true )
	{
		iBuffEndTime = static_cast<int>(m_tWorldMissionEndTime.GetTime() + m_usWorldBuffDurationTime - tCurr.GetTime());
	}
	else
	{
		iBuffEndTime = static_cast<int>( m_usWorldBuffDurationTime );
	}
	return iBuffEndTime;
}
#endif SERV_SERVER_BUFF_SYSTEM
//}

//{{ 2012. 03. 03	�ڼ���	����� �� ����
#ifdef SERV_REFORM_THE_GATE_OF_DARKNESS
int KGSWorldMissionManager::GetWorldCrystalCount( void )
{ 
	if( m_iWorldCrystalCount > WBSC_STEP_THREE )	// ������ �ִ� ���� ���� ��� �ִ� ���� ����(client UI's crystal)
	{
		m_iWorldCrystalCount = WBSC_STEP_THREE;
	}

	START_LOG( clog, L"���� ũ����Ż ���� Ȯ��" )
		<< BUILD_LOG( m_iWorldCrystalCount )
		<< END_LOG;

	return m_iWorldCrystalCount; 
}

void KGSWorldMissionManager::GetIndividualBuffID( OUT int& iBuffLevel, std::vector<int>& vecDeleteBuff )
{
	vecDeleteBuff.clear();

	int iWorldCrystalCount = GetWorldCrystalCount();

	if( DEFENSE_REWARD_BUFF_MAX_NEEDS <= iWorldCrystalCount )
	{
		START_LOG( cerr, L"WorldCrystalCount�� ���� �ִ�ġ�� �ѱ�� �ֽ��ϴ�." )
			<< BUILD_LOG( iWorldCrystalCount )
			<< END_LOG;
		iBuffLevel = CXSLBuffManager::BTI_BUFF_HERO_OF_ELIOS_LEVEL_4;
	}
	else if( DEFENSE_REWARD_BUFF_LEVEL4_NEEDS <= iWorldCrystalCount )
	{
		iBuffLevel = CXSLBuffManager::BTI_BUFF_HERO_OF_ELIOS_LEVEL_4;
		vecDeleteBuff.push_back( CXSLBuffManager::BTI_BUFF_HERO_OF_ELIOS_LEVEL_3 );
		vecDeleteBuff.push_back( CXSLBuffManager::BTI_BUFF_HERO_OF_ELIOS_LEVEL_2 );
		vecDeleteBuff.push_back( CXSLBuffManager::BTI_BUFF_HERO_OF_ELIOS_LEVEL_1 );
	}
	else if( DEFENSE_REWARD_BUFF_LEVEL3_NEEDS <= iWorldCrystalCount )
	{
		iBuffLevel = CXSLBuffManager::BTI_BUFF_HERO_OF_ELIOS_LEVEL_3;
		vecDeleteBuff.push_back( CXSLBuffManager::BTI_BUFF_HERO_OF_ELIOS_LEVEL_2 );
		vecDeleteBuff.push_back( CXSLBuffManager::BTI_BUFF_HERO_OF_ELIOS_LEVEL_1 );
	}
	else if( DEFENSE_REWARD_BUFF_LEVEL2_NEEDS <= iWorldCrystalCount )
	{
		iBuffLevel = CXSLBuffManager::BTI_BUFF_HERO_OF_ELIOS_LEVEL_2;
		vecDeleteBuff.push_back( CXSLBuffManager::BTI_BUFF_HERO_OF_ELIOS_LEVEL_1 );
	}
	else if ( DEFENSE_REWARD_BUFF_LEVEL1_NEEDS <= iWorldCrystalCount )
	{
		iBuffLevel = CXSLBuffManager::BTI_BUFF_HERO_OF_ELIOS_LEVEL_1;
	}
	else
	{
		iBuffLevel = CXSLBuffManager::BTI_BUFF_HERO_OF_ELIOS_LEVEL_1;
		START_LOG( cerr, L"WorldCrystalCount�� ���� ����ġ���� �۽��ϴ�." )
			<< BUILD_LOG( iWorldCrystalCount )
			<< END_LOG;
	}
}

bool KGSWorldMissionManager::GetDefenseDungeonEndTime( OUT CTime& tStartTime, OUT CTime& tEndTime ) const
{
	CTime tCurrentTime = CTime::GetCurrentTime();

	// ���Ϻ� ���� �ð� ���
	std::map< int, std::vector< KDefenseOpenTime > >::const_iterator mit = m_mapDefenseOpenTime.find( tCurrentTime .GetDayOfWeek() );

	// �ش� ���Ͽ� ����� ���� �ð� ���� ���� �˻�.
	if( mit == m_mapDefenseOpenTime.end() )
	{
		START_LOG( cerr, L"���潺 ���� �ð� ���� ���� ���� ���� �ʽ��ϴ�." );
		return false;
	}

	// ���� �ð�
	const std::vector< KDefenseOpenTime >& vecTime = mit->second;

	// �ش� ���Ͽ� ����� ���� �ð����� ��ȸ�ϸ鼭, ���� �ð��뿡 �´� ������ �ִ��� �˻��Ѵ�.
	for( std::vector< KDefenseOpenTime >::const_reverse_iterator rit = vecTime.rbegin(); rit != vecTime.rend(); ++rit )
	{
		const KDefenseOpenTime& kDefenseOpenTime = *rit;
		tStartTime = CTime( tCurrentTime .GetYear(), tCurrentTime .GetMonth(), tCurrentTime .GetDay(), kDefenseOpenTime.m_iStartHour, kDefenseOpenTime.m_iStartMinute, 0 );
		tEndTime = tStartTime + CTimeSpan( 0, 0, kDefenseOpenTime.m_iDurationTime, 0 );

		// ������ ���� �ִٸ�, ���� �ð��� �����ϴ� ���� ���� ���� �ð��� ã�´�.
		if( m_bActive )
		{
			if( ( tStartTime <= tCurrentTime ) && ( tCurrentTime < tEndTime ) )
			{
				return true;
			}
		}
		else if( tEndTime <= tCurrentTime )
		{
			return true;
		}
	}

	std::wstring wstrStartTime = tStartTime.Format(_T("%Y-%m-%d %H:%M:%S"));
	std::wstring wstrEndTime = tEndTime.Format(_T("%Y-%m-%d %H:%M:%S"));
	std::wstring wstrCurrentTime = tCurrentTime.Format(_T("%Y-%m-%d %H:%M:%S"));

	START_LOG( cerr, L"���潺 ���� EndTime �˻� ����")
		<< BUILD_LOG( m_bActive )
		<< BUILD_LOG( wstrCurrentTime )
		<< BUILD_LOG( wstrStartTime )
		<< BUILD_LOG( wstrEndTime )
		<< END_LOG;

	return false;
}
#endif SERV_REFORM_THE_GATE_OF_DARKNESS
//}}

int KGSWorldMissionManager::GetRemainTime()
{
	if( m_bActive ==true )
	{
		int iTime = static_cast<int>( m_iMissionTime - m_kTimer.elapsed() );
		return iTime > 0 ? iTime : 0;
	}
	else
	{
		return 0;
	}
}

void KGSWorldMissionManager::UpdateMissionStartTime( IN std::wstring& wstrDate )
{
	if( KncUtil::ConvertStringToCTime( wstrDate, m_tWorldMissionNextStartTime ) == false )
	{
		START_LOG( cerr, L"�ð� ���� ����!" )
			<< BUILD_LOG( m_tWorldMissionNextStartTime.GetYear() )
			<< BUILD_LOG( m_tWorldMissionNextStartTime.GetMonth() )
			<< BUILD_LOG( m_tWorldMissionNextStartTime.GetDay() )
			<< BUILD_LOG( m_tWorldMissionNextStartTime.GetHour() )
			<< BUILD_LOG( m_tWorldMissionNextStartTime.GetMinute() )
			<< BUILD_LOG( m_tWorldMissionNextStartTime.GetSecond() )
			<< BUILD_LOG( wstrDate )
			<< END_LOG;
	}
}

int KGSWorldMissionManager::GetMissionStartRemainTime()
{
	CTime tCurrent = CTime::GetCurrentTime();

	if( m_tWorldMissionNextStartTime > tCurrent )	// ���� �̼��� ���Ŀ� �����Ѵ�
	{
		CTimeSpan gap = m_tWorldMissionNextStartTime - tCurrent;

		return (int)gap.GetTotalMinutes();
	}
	else if( m_iMissionTime == 0 )					// ���� �̼� ���Ŀ� ����
	{
		return -1;
	}
	else											// ���� �̼��� ���� ���̴�
	{
		return 0;
	}
}
#endif SERV_NEW_DEFENCE_DUNGEON
//}}
#endif SERV_INSERT_GLOBAL_SERVER
//}} 2011. 04. 13  ��μ�  �۷ι� ���� �߰�