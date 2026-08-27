#include <WinSock2.h>
#include "NexonBillingTCPManager.h"

#include "Socket/NetCommon.h"
#include "KncSend.h"
#include "NexonBillingTCP.h"
#include "BaseServer.h"
#include "SimLayer.h"

#include "X2Data/XSLItem.h"
#include "X2data/XSLCashItemManager.h"

//{{ 2010. 10. 12	������	���� ����͸�
#ifdef SERV_MORNITORING
	#include "Mornitoring/MornitoringManager.h"
#endif SERV_MORNITORING
//}}

//{{ 2013. 01. 10	������	���� TCP ��Ŷ ũ�� ����ó��
#ifdef SERV_BILLING_TCP_RECV_VARIABLE_SIZE_BUFFER
static const unsigned long VARIABLE_BUFFER_MAX_SIZE = 32768;		// 32kb
#endif SERV_BILLING_TCP_RECV_VARIABLE_SIZE_BUFFER
//}}

DWORD KNexonBillingTCPManager::ms_dwHeartBeatGap( 1 * 60 * 1000 );
DWORD KNexonBillingTCPManager::ms_dwConnectionCheckGap( 20 * 1000 );

ImplPfID( KNexonBillingTCPManager, PI_GS_NX_BILLING_TCP );

ImplementSingleton( KNexonBillingTCPManager );

KNexonBillingTCPManager::KNexonBillingTCPManager()
{
	m_kNexonBillingTCPInfo.m_strIP.clear();
	m_kNexonBillingTCPInfo.m_usPort = 0;
    m_kNexonBillingTCPInfo.m_iDomain = 0;
	//{{ 2009. 11. 21  ������	���������ӱ�������
	m_sock = INVALID_SOCKET;
	m_bFirstConnectSucc = false;
	//}}
	m_dwLastHeartBeatTick = ::GetTickCount();
	m_dwLastConnectionCheckTick = ::GetTickCount();
    m_ulRecvCP = 0;
	KCSLOCK_SET_VALUE( m_iReleaseTick, 0 );
	KCSLOCK_SET_VALUE( m_ulCurrentPacketNo, 0 );
	KCSLOCK_SET_VALUE( m_ulCurrentPurchaseNo, 0 );
	KCSLOCK_SET_VALUE( m_ulCurrentProductPage, 0 );
    m_spEvent.reset( CreateEvent( NULL, false, false, NULL ), CloseHandle );
	m_iBillingServerNo = 0;
	//{{ 2010. 11. 22	������	û�� öȸ
#ifdef SERV_NX_BILLING_REFUND
	KCSLOCK_SET_VALUE( m_iCashItemRefundRequestID, 0 );
#endif SERV_NX_BILLING_REFUND
	//}}
}

KNexonBillingTCPManager::~KNexonBillingTCPManager()
{
}

ImplToStringW( KNexonBillingTCPManager )
{
	KLocker lock( m_csProductInfo );

	stm_	<< L"----------[ Nexon Billing TCP Manager ]----------" << std::endl
		<< L"IP Addr : " << KncUtil::toWideString( m_kNexonBillingTCPInfo.m_strIP ) << std::endl
		<< L"Port : " << m_kNexonBillingTCPInfo.m_usPort << std::endl
		<< L"Domain : " << m_kNexonBillingTCPInfo.m_iDomain << std::endl
		<< L"Product Info : " << m_mapProductInfo.size() << std::endl
		;

	//{{ 2013. 01. 23	������	���� ��Ŷ ���� ó�� ���� ����
#ifdef SERV_BILLING_PACKET_RECV_PERFORMANCE_CHECK
	stm_	<< L"----------[ Nexon Billing Recv Performance Dump ]----------" << std::endl
		<< TOSTRINGW( m_kRecvPerformanceDump.m_dwMinLatency )
		<< TOSTRINGW( m_kRecvPerformanceDump.m_wstrMinRegDate )
		<< TOSTRINGW( m_kRecvPerformanceDump.m_dwMaxLatency )
		<< TOSTRINGW( m_kRecvPerformanceDump.m_wstrMaxRegDate )
		;
#endif SERV_BILLING_PACKET_RECV_PERFORMANCE_CHECK
	//}}

	return stm_;
}

#include <KncLua.h>
#include <lua_tinker.h>

void KNexonBillingTCPManager::RegToLua()
{
	lua_tinker::class_add<KNexonBillingTCPManager>( g_pLua, "KNexonBillingTCPManager" );
	lua_tinker::class_def<KNexonBillingTCPManager>( g_pLua, "dump",						&KNexonBillingTCPManager::Dump );
	lua_tinker::class_def<KNexonBillingTCPManager>( g_pLua, "InitNexonBillingTCPInfo",	&KNexonBillingTCPManager::InitNexonBillingTCPInfo );
	lua_tinker::class_def<KNexonBillingTCPManager>( g_pLua, "SetNumThread",				&KNexonBillingTCPManager::Init );

	lua_tinker::decl( g_pLua, "NXBillingTCPManager", this );
}

void KNexonBillingTCPManager::InitNexonBillingTCPInfo( const char* szNexonAuthIP, unsigned short usPort, int iDomain )
{
	m_kNexonBillingTCPInfo.m_strIP		= szNexonAuthIP;
	m_kNexonBillingTCPInfo.m_usPort		= usPort;
    m_kNexonBillingTCPInfo.m_iDomain	= iDomain;	
}

void KNexonBillingTCPManager::Init( int nThreadNum )
{
    m_ulRecvCP = 0;
	//{{ 2013. 01. 10	������	���� TCP ��Ŷ ũ�� ����ó��
#ifdef SERV_BILLING_TCP_RECV_VARIABLE_SIZE_BUFFER
	m_kRecvVariableBuffer.Reserve( VARIABLE_BUFFER_MAX_SIZE );
#endif SERV_BILLING_TCP_RECV_VARIABLE_SIZE_BUFFER
	//}}

    //////////////////////////////////////////////////////////////////////////
    // thread setting : recv�� recvfrom() �Լ����� �˾Ƽ� block�ȴ�.
    //{{ Iruha : 2026-08-27 // VS2010 port: bare Class::Method as a member-function pointer
    // was a VC7.1 extension; VC10 requires the explicit &.
    m_spThreadRecv = boost::shared_ptr< KTThread< KNexonBillingTCPManager > >
        ( new KTThread< KNexonBillingTCPManager >( *this, &KNexonBillingTCPManager::Recv, 50 ) );

    m_spThreadSend = boost::shared_ptr< KTThread< KNexonBillingTCPManager > >
        ( new KTThread< KNexonBillingTCPManager >( *this, &KNexonBillingTCPManager::Send, 100 ) );
    //}}

	//{{ 2009. 11. 21  ������	���������ӱ�������
	m_spThreadKeepConnect = boost::shared_ptr< KTThread< KNexonBillingTCPManager > >
		( new KTThread< KNexonBillingTCPManager >( *this, &KNexonBillingTCPManager::KeepConnectionThread, 1000 ) );
	//}}

	KThreadManager::Init( nThreadNum );
}

KThread* KNexonBillingTCPManager::CreateThread()
{
	return new KNexonBillingTCP;
}

void KNexonBillingTCPManager::BeginThread()
{
	//{{ 2011. 12. 07	������	���� ��ũ��Ʈ �Ŵ���
#ifdef SERV_BILLING_SCRIPT_MANAGER
	// ���� ������ �����ϱ� ���� config���� �Ľ��� ����!
	m_kBillingScriptManager.OpenScriptFile( "BillingConfig.lua" );
#endif SERV_BILLING_SCRIPT_MANAGER
	//}}

	// ������Ŷ ���Ž�����
	if( m_spThreadRecv )
	{
		m_spThreadRecv->Begin();
	}

	// ������Ŷ ���۽�����
	if( m_spThreadSend )
	{
		m_spThreadSend->Begin();
	}

	// �����̺�Ʈ �ڵ鸵������
	KThreadManager::BeginThread();

	//{{ 2009. 11. 21  ������	���������ӱ�������
	// ������������ ��Ʈ��Ʈ üũ �� ���� ������ ���� ��������������
	if( m_spThreadKeepConnect )
	{
		// Thread�� �����ϸ鼭 ���������� ���ӽõ��Ѵ�.
		m_spThreadKeepConnect->Begin();
	}
	//}}
}

//{{ 2010. 8. 30	������	���� ���� ���� ó��
void KNexonBillingTCPManager::EndThread( DWORD dwTimeOut /*= 10000*/ )
{
	//{{ 2009. 11. 21  ������	���������ӱ�������
	if( m_spThreadKeepConnect )
	{
		m_spThreadKeepConnect->End( 3000 );

		START_LOG( cout, L"�ؽ� ���� ���� ���� üũ ������ ����!" );
	}
	//}}

	KThreadManager::EndThread();

    if( m_spThreadSend )
    {
	    m_spThreadSend->End( 10000 );

		START_LOG( cout, L"�ؽ� ���� ���� Send ������ ����!" );
    }

	// recv ������ ���̱� ���� ������ ����!
	CLOSE_SOCKET( m_sock );
    
    if( m_spThreadRecv )
    {
	    m_spThreadRecv->End( 3000 );

		START_LOG( cout, L"�ؽ� ���� ���� Recv ������ ����!" );
    }
}
//}}

void KNexonBillingTCPManager::QueueingSendPacket( const KNexonBillingTCPPacketPtr& spPacket )
{
    JIF( spPacket );

	KCSLOCK_BEGIN( m_kSendQueue )
		m_kSendQueue.push( spPacket );
	KCSLOCK_END()
}

__int64 KNexonBillingTCPManager::GetReleaseTick()
{
	__int64 iReleaseTick = 0;
	KCSLOCK_BEGIN( m_iReleaseTick )
		iReleaseTick = m_iReleaseTick;
	KCSLOCK_END()

    return iReleaseTick;
}

bool KNexonBillingTCPManager::SetReleaseTick( const __int64& iReleaseTick )
{
    bool bRet;

	KCSLOCK_BEGIN( m_iReleaseTick )
		bRet = ( m_iReleaseTick == iReleaseTick );

		m_iReleaseTick = iReleaseTick;
	KCSLOCK_END()

    return bRet;
}

unsigned long KNexonBillingTCPManager::GetNextPacketNo()
{
	unsigned long ulCurrentPacketNo;

	KCSLOCK_BEGIN( m_ulCurrentPacketNo )
		m_ulCurrentPacketNo++;
		if( m_ulCurrentPacketNo > 0xfffffff0 )
		{
			m_ulCurrentPacketNo = 1;
		}
		ulCurrentPacketNo = m_ulCurrentPacketNo;
	KCSLOCK_END()

    return ulCurrentPacketNo;
}

unsigned long KNexonBillingTCPManager::GetNextPurchaseNo()
{
	unsigned long ulCurrentPurchaseNo;

	KCSLOCK_BEGIN( m_ulCurrentPurchaseNo )
		m_ulCurrentPurchaseNo++;
		if( m_ulCurrentPurchaseNo > 0xfffffff0 )
		{
			m_ulCurrentPurchaseNo = 1;
		}
		ulCurrentPurchaseNo = m_ulCurrentPurchaseNo;
	KCSLOCK_END()

    return ulCurrentPurchaseNo;
}

void KNexonBillingTCPManager::InsertPacketNoUserUID( unsigned long ulPacketNo, UidType iUserUID )
{
    if( iUserUID <= 0 )
    {
        START_LOG( cerr, L"���� UID �̻�." )
            << BUILD_LOG( ulPacketNo )
            << BUILD_LOG( iUserUID )
            << END_LOG;

        return;
    }

	KCSLOCK_BEGIN( m_mapPacketNoUserUID )
		std::map< unsigned long, UidType >::iterator mit;
		mit = m_mapPacketNoUserUID.find( ulPacketNo );
		if( mit != m_mapPacketNoUserUID.end() )
		{
			START_LOG( cerr, L"��Ŷ ��ȣ�� �ش��ϴ� UserUID �� �̹� ��ϵǾ� �ִ�." )
				<< BUILD_LOG( mit->first )
				<< BUILD_LOG( mit->second )
				<< BUILD_LOG( ulPacketNo )
				<< BUILD_LOG( iUserUID )
				<< END_LOG;
		}

		// ������ �����ϰ� ������ ���� ����.
		m_mapPacketNoUserUID[ulPacketNo] = iUserUID;
	KCSLOCK_END()
}

void KNexonBillingTCPManager::DeletePacketNoUserUID( unsigned long ulPacketNo )
{
	KCSLOCK_BEGIN( m_mapPacketNoUserUID )
		std::map< unsigned long, UidType >::iterator mit;
		mit = m_mapPacketNoUserUID.find( ulPacketNo );
		if( mit == m_mapPacketNoUserUID.end() )
		{
			START_LOG( cerr, L"��Ŷ ��ȣ�� �ش��ϴ� UserUID �� ��ϵǾ� ���� �ʴ�." )
				<< BUILD_LOG( ulPacketNo )
				<< END_LOG;

			return;
		}

		m_mapPacketNoUserUID.erase( mit );
	KCSLOCK_END()
}

UidType KNexonBillingTCPManager::GetCorrespondingUserUID( unsigned long ulPacketNo )
{
	UidType iUserUID = 0;

	KCSLOCK_BEGIN( m_mapPacketNoUserUID )
		std::map< unsigned long, UidType >::iterator mit;
		mit = m_mapPacketNoUserUID.find( ulPacketNo );
		if( mit == m_mapPacketNoUserUID.end() )
		{
			START_LOG( cerr, L"��Ŷ ��ȣ�� �ش��ϴ� UserUID �� ��ϵǾ� ���� �ʴ�." )
				<< BUILD_LOG( ulPacketNo )
				<< END_LOG;

			return 0;
		}
		iUserUID = mit->second;
	KCSLOCK_END()

    return iUserUID;
}

void KNexonBillingTCPManager::AddProductInfo( KNXBTProductInfo kInfo )
{
    KLocker lock( m_csProductInfo );

    std::map< unsigned long, KNXBTProductInfo >::iterator mit;
    mit = m_mapProductInfo.find( kInfo.m_ulProductNo );
    if( mit != m_mapProductInfo.end() )
    {
        START_LOG( cwarn, L"���δ�Ʈ ������ �̹� ������." )
            << BUILD_LOG( GetReleaseTick() )
            << BUILD_LOG( kInfo.m_ulProductNo )
            << BUILD_LOG( mit->second.m_wstrProductID )
            << BUILD_LOG( kInfo.m_wstrProductID )
            << END_LOG;
    }

	//////////////////////////////////////////////////////////////////////////
	// �Һ� ī�װ����� ������ �ֱ�
	const int iItemID = _wtoi( kInfo.m_wstrProductID.c_str() );

	//{{ 2009. 12. 19  ������	������ Ȯ��
	if( SiCXSLCashItemManager()->IsSellItemInThisServer( iItemID ) == false )
	{
		START_LOG( cout, L"���� ������������ �ǸŵǼ��� �ȵǴ� ��ǰ�Դϴ�. ����Ʈ�� ���Խ�Ű�� �ʽ��ϴ�." )
			<< BUILD_LOG( iItemID )
			<< END_LOG;

		//{{ 2012. 03. 21	��μ�		ĳ�� ��ǰ ����Ʈ ������ �������� DB�� ���
#ifdef SERV_CASH_ITEM_LIST_ADD
		// ���ܵ� ĳ�� ��ǰ�� ���� ������ �д�
		m_mapProductInfo_Excepted[kInfo.m_ulProductNo] = kInfo;
#endif SERV_CASH_ITEM_LIST_ADD
		//}}

		return;
	}
	//}}


	//{{ 2011. 01. 04  ��μ�	product no �� ������ Ȯ��
#ifdef SERV_SERVER_DIVISION_CASHITEM_BY_PRODUCTNO
	if( SiCXSLCashItemManager()->IsSellItemInThisProductNo( kInfo.m_ulProductNo ) == false )
	{
		START_LOG( cout, L"���� ������������ �ǸŵǼ��� �ȵǴ� ��ǰ�Դϴ�. ����Ʈ�� ���Խ�Ű�� �ʽ��ϴ�." )
			<< BUILD_LOG( kInfo.m_ulProductNo )
			<< END_LOG;

		//{{ 2012. 03. 21	��μ�		ĳ�� ��ǰ ����Ʈ ������ �������� DB�� ���
#ifdef SERV_CASH_ITEM_LIST_ADD
		// ���ܵ� ĳ�� ��ǰ�� ���� ������ �д�
		m_mapProductInfo_Excepted[kInfo.m_ulProductNo] = kInfo;
#endif SERV_CASH_ITEM_LIST_ADD
		//}}

		return;
	}
#endif SERV_SERVER_DIVISION_CASHITEM_BY_PRODUCTNO
	//}}

	//{{ 2011. 12. 07	������	���� ��ũ��Ʈ �Ŵ���
#ifdef SERV_BILLING_SCRIPT_MANAGER
	// ���� ����
	const bool bIsService = ( KSimLayer::GetKObj()->GetBillingFlag() == KSimLayer::BF_NEXON_KOREA );

	if( m_kBillingScriptManager.IsExistProductCategoryModifyInfoForItemID( iItemID ) == true )
	{
		if( m_kBillingScriptManager.GetModifiedProductCategoryForItemID( iItemID, bIsService, kInfo.m_ulCategoryNo ) == false )
		{
			START_LOG( cerr, L"��ǰ ī�װ��� ���� ���� ���� ����!" )
				<< BUILD_LOG( iItemID )
				<< BUILD_LOG( bIsService )
				<< END_LOG;
		}
	}
#else
	// ���� ĳ���� and ��ȭ ���� �������� ĳ���� ī�װ��� ����
	//if( CXSLItem::IsJobChangeCashItem( iItemID ) == true  ||
	//	CXSLItem::IsEnchantSupportItem( iItemID ) == true )
	//{
	//	// ���� �����̾� ī�װ���
	//	if( KSimLayer::GetKObj()->GetBillingFlag() == KSimLayer::BF_NEXON_KOREA )
	//	{
	//		kInfo.m_ulCategoryNo = 790;
	//	}
	//	else
	//	{
	//		kInfo.m_ulCategoryNo = 703;
	//	}
	//}

	//switch( iItemID )
	//{
	//case 500010: // ������ ���� (�ڶ� ���μ�Ʈ �Ƿ�)
	//case 500020: // ������ ���� (Ʈ�� ����Ʈ)
	//case 500060: // �ڶ� ���μ�Ʈ �Ƿ�
	//case 500070: // Ʈ�� ����Ʈ
	//case 500080: // ������ ���� (�ڶ� ���μ�Ʈ �Ƿ�) Ư�� ��Ű��
	//case 500090: // ������ ���� (Ʈ�� ����Ʈ) Ư�� ��Ű��
	//case 226860: // '������ ����' �ں� Ư�� ��ǰ��
	//case 500040: // ���� ���� ����
	//case 500130: // �ҷ���ġ Ư�� ��Ű��
	//case 500120: // ������ ���� (�ҷ� ��ġ) Ư�� ��Ű��
	//case 500100: // ������ ���� (�ҷ� ��ġ)
	//case 500140: // ������ ���� (�絹��)
	//case 500150: // ������ ���� (�̽�ƽ)
	//case 500190: // ������ ���� (�̽�ƽ) Ư�� ��Ű��
	//case 500200: // ������ ���� (�̽�ƽ ����� �����-����) Ư�� ��Ű��
	//case 500210: // ������ ���� (�̽�ƽ ����� �����-ȭ��Ʈ) Ư�� ��Ű��
	//case 500220: // ������ ���� (�̽�ƽ ����� �����-����) Ư�� ��Ű�� 
	//case 500230: // ������ ���� (������)
	//case 500240: // ������ ���� (������-��ũ)
	//case 500250: // ������ ���� (������-���̾�)
	//case 500260: // ������ ���� (������-����Ʈ)
	//case 500270: // ������ ���� (������) ��Ű��
	//case 500280: // ������ ���� (������-��ũ) ��Ű��
	//case 500290: // ������ ���� (������-���̾�) ��Ű��
	//case 500300: // ������ ���� (������-����Ʈ) ��Ű��
	//case 500310: // ����Ʈ ���μ�Ʈ �Ƿ�-�̺�
	//case 500320: // ����Ʈ ���μ�Ʈ �Ƿ�-�����̾�
	//case 500330: // ����Ʈ ���μ�Ʈ �Ƿ�-�ҷο�
	//case 500340: // ����Ʈ ���μ�Ʈ �Ƿ�-������
	//case 500350: // �ҷ��� ����Ʈ �Ƿ� ��Ű��(�̺�)
	//case 500360: // �ҷ��� ����Ʈ �Ƿ� ��Ű��(�����̾�)
	//case 500370: // �ҷ��� ����Ʈ �Ƿ� ��Ű��(�ҷο�)
	//case 500380: // �ҷ��� ����Ʈ �Ƿ� ��Ű��(������) 
	//	{
	//		// ���� �����̾� ī�װ���
	//		if( KSimLayer::GetKObj()->GetBillingFlag() == KSimLayer::BF_NEXON_KOREA )
	//		{
	//			kInfo.m_ulCategoryNo = 790;
	//		}
	//		else
	//		{
	//			kInfo.m_ulCategoryNo = 703;
	//		}
	//	}
	//	break;
	//}
#endif SERV_BILLING_SCRIPT_MANAGER
	//}}

	//////////////////////////////////////////////////////////////////////////

	//{{ 2009. 10. 14  ������	�ڵ�������ǰ����Ʈ
	if( kInfo.m_ulPaymentType == NEXON_BILLING_ENUM::AUTO_PAYMENT_TYPE )
	{
		KCSLOCK_BEGIN( m_vecAutoPaymentProductList )
			m_vecAutoPaymentProductList.push_back( kInfo.m_ulProductNo );
		KCSLOCK_END()
	}
	//}}

    // ������ �� ������ ���� ����.
    m_mapProductInfo[kInfo.m_ulProductNo] = kInfo;
}

int KNexonBillingTCPManager::GetItemID( unsigned long ulProductNo )
{
    KLocker lock( m_csProductInfo );

    std::map< unsigned long, KNXBTProductInfo >::iterator mit;
    mit = m_mapProductInfo.find( ulProductNo );
    if( mit == m_mapProductInfo.end() )
    {
        START_LOG( cerr, L"��ǰ ��ȣ �˻� ����." )
            << BUILD_LOG( ulProductNo )
            << END_LOG;

        return 0;
    }

    int iItemID;
    iItemID = ::atoi( KncUtil::toNarrowString( mit->second.m_wstrProductID ).c_str() );
    if( iItemID == 0 )
    {
        START_LOG( cerr, L"������ ID ��ȯ ����." )
            << BUILD_LOG( mit->second.m_wstrProductID )
            << END_LOG;
    }

    return iItemID;
}

//{{ 2010. 01. 28  ������	��������ó��
int KNexonBillingTCPManager::GetProductPageNumber()
{
	unsigned long ulCurrentProductPage;
	KCSLOCK_BEGIN( m_ulCurrentProductPage )
		ulCurrentProductPage = m_ulCurrentProductPage;
	KCSLOCK_END()

	return ulCurrentProductPage;
}
//}}

int KNexonBillingTCPManager::GetNextProductPageNumber()
{
	unsigned long ulCurrentProductPage;
	KCSLOCK_BEGIN( m_ulCurrentProductPage )
		m_ulCurrentProductPage++;
		ulCurrentProductPage = m_ulCurrentProductPage;
	KCSLOCK_END()

    return ulCurrentProductPage;
}

void KNexonBillingTCPManager::BuildProductPage()
{
    KLocker lock( m_csProductInfo );

	//{{ 2010. 04. 26  ������	PC�� ���� ĳ����
	m_vecProductPage[CILT_NORMAL].clear();
	m_vecProductPage[CILT_PC_BANG].clear();
	//}}

	//{{ 2012. 02. 21	��μ�	ĳ�ü� ��Ŷ ������ ����
#ifdef SERV_QUICK_CASH_SHOP
	const int ciNumProductPerPage = KNexonBillingTCPManager::NBC_CASH_ITEM_MAX;
#else
	const int ciNumProductPerPage = 10;
#endif SERV_QUICK_CASH_SHOP
	//}}

	//{{ 2008. 5. 15  ������  ��ǰ ���� ItemID
	KENX_UPDATE_PRODUCT_LIST_NOT kEvent;
	kEvent.m_ulProductNoResStone = 0;
	//}}
    std::vector< KNXBTProductInfo > vecProductInfo[CILT_MAX];
    std::map< unsigned long, KNXBTProductInfo >::reverse_iterator mit;
	for( mit = m_mapProductInfo.rbegin(); mit != m_mapProductInfo.rend(); ++mit )
    {
		const int iCashItemID = _wtoi( mit->second.m_wstrProductID.c_str() );

		//{{ 2011. 02. 23	������	ĳ�� ��ǰ ����Ʈ
#ifdef SERV_CASH_ITEM_LIST
		KCashPruductInfo kCashProductInfo;
		kCashProductInfo.m_ulProductNo		= mit->second.m_ulProductNo;
		kCashProductInfo.m_sProductExpire	= mit->second.m_sProductExpire;
		kCashProductInfo.m_usProductPieces	= mit->second.m_usProductPieces;
		kCashProductInfo.m_iItemID			= iCashItemID;
		kCashProductInfo.m_ulSalePrice		= mit->second.m_ulSalePrice;
		kEvent.m_vecCashProductList.push_back( kCashProductInfo );
#endif SERV_CASH_ITEM_LIST
		//}}

		//{{ 2008. 9. 23  ������	ĳ���� ����
		if( mit->second.IsInfinityProduct()  &&  iCashItemID != CXSLItem::CI_RESURRECTION_STONE )
		{
			KCashItemResolveInfo kPriceInfo;
			kPriceInfo.m_iItemID    = iCashItemID;
			kPriceInfo.m_iSalePrice = mit->second.m_ulSalePrice;

            kEvent.m_vecCashItemRessolveInfo.push_back( kPriceInfo );
		}
		//}}

		// �򸮽�Ʈ�� ������ ��ǰItemID ���
		kEvent.m_vecProductItemID.push_back( iCashItemID );

		//{{ 2008. 6. 10  ������  �ǽð� ��Ȱ��
		if( iCashItemID == CXSLItem::CI_RESURRECTION_STONE  &&  mit->second.m_usProductPieces == 1 )
		{
			kEvent.m_ulProductNoResStone = mit->second.m_ulProductNo;

			// Ŭ���̾�Ʈ�� �������� ��ǰ ������ ����Ʈ���� ����.
			continue;
		}
		//}}

		//{{ 2010. 04. 26  ������	PC�� ���� ĳ����		
		if( CXSLItem::IsPcBangOnlyCashItem( iCashItemID ) == false )
		{
			vecProductInfo[CILT_NORMAL].push_back( mit->second );

			if( vecProductInfo[CILT_NORMAL].size() >= ciNumProductPerPage )
			{
				m_vecProductPage[CILT_NORMAL].push_back( vecProductInfo[CILT_NORMAL] );
				vecProductInfo[CILT_NORMAL].clear();
			}
		}
		//}}

		vecProductInfo[CILT_PC_BANG].push_back( mit->second );

		if( vecProductInfo[CILT_PC_BANG].size() >= ciNumProductPerPage )
		{
			m_vecProductPage[CILT_PC_BANG].push_back( vecProductInfo[CILT_PC_BANG] );
			vecProductInfo[CILT_PC_BANG].clear();
		}
    }
    
    if( !vecProductInfo[CILT_NORMAL].empty() )
    {
        m_vecProductPage[CILT_NORMAL].push_back( vecProductInfo[CILT_NORMAL] );
    }

	if( !vecProductInfo[CILT_PC_BANG].empty() )
	{
		m_vecProductPage[CILT_PC_BANG].push_back( vecProductInfo[CILT_PC_BANG] );
	}

	//{{ 2008. 5. 15  ������  ��ǰ ���� ItemID
	KEventPtr spEvent( new KEvent );
	spEvent->SetData( 0, NULL, ENX_UPDATE_PRODUCT_LIST_NOT, kEvent );
	KBaseServer::GetKObj()->QueueingEvent( spEvent );
	//}}
}

bool KNexonBillingTCPManager::GetProductPage( IN int iPage, IN bool bIsPcBang, OUT std::vector< KNXBTProductInfo >& vecProductInfo )
{
    vecProductInfo.clear();

	//{{ 2010. 04. 26  ������	PC�� ���� ĳ����
	CASH_ITEM_LIST_TYPE eType = ( bIsPcBang ? CILT_PC_BANG : CILT_NORMAL );
	//}}

    if( m_vecProductPage[eType].empty() )
    {
        return true;
    }

    int iIndex = iPage - 1;

    if( iIndex >= int( m_vecProductPage[eType].size() ) )
    {
        iIndex = int( m_vecProductPage[eType].size() ) - 1;
    }

    if( iIndex < 0 )
    {
        iIndex = 0;
    }

    vecProductInfo = m_vecProductPage[eType][iIndex];
    return iIndex == ( int( m_vecProductPage[eType].size() ) - 1 );
}

bool KNexonBillingTCPManager::GetProductInfo( unsigned long ulProductNo, KNXBTProductInfo& kProductInfo )
{
    KLocker lock( m_csProductInfo );

    std::map< unsigned long, KNXBTProductInfo >::iterator mit;
    mit = m_mapProductInfo.find( ulProductNo );
    if( mit == m_mapProductInfo.end() )
    {
        return false;
    }

    kProductInfo = mit->second;
    return true;
}

bool KNexonBillingTCPManager::GetTotalPrice( const std::vector< KNXBTPurchaseReqInfo >& vecPurchaseReqInfo, unsigned long& ulPrice )
{
	KLocker lock( m_csProductInfo );

    ulPrice = 0;

    std::vector< KNXBTPurchaseReqInfo >::const_iterator vit;
    for( vit = vecPurchaseReqInfo.begin(); vit != vecPurchaseReqInfo.end(); vit++ )
    {
        std::map< unsigned long, KNXBTProductInfo >::iterator mit;
        mit = m_mapProductInfo.find( vit->m_ulProductNo );
        if( mit == m_mapProductInfo.end() )
        {
            return false;
        }

        ulPrice += mit->second.m_ulSalePrice;
    }

    return true;
}

void KNexonBillingTCPManager::HandleInitializeAck( bool bSucceed )
{
    m_bInitialized = bSucceed;
    SetEvent( m_spEvent.get() );
}

//{{ 2013. 01. 10	������	���� TCP ��Ŷ ũ�� ����ó��
//////////////////////////////////////////////////////////////////////////
#ifdef SERV_BILLING_TCP_RECV_VARIABLE_SIZE_BUFFER
//////////////////////////////////////////////////////////////////////////

void KNexonBillingTCPManager::Recv()
{
	if( !IsConnected() )
	{
		return;
	}

	int ret;
	ret = ::recv( m_sock,
		m_cRecvBuffer + m_ulRecvCP,
		( int )( MAX_PACKET_SIZE_NBT - m_ulRecvCP ),
		0 );

	START_LOG( clog, L"��Ŷ ����." )
		<< BUILD_LOG( ret );

	if( ret == SOCKET_ERROR )
	{
		//{{ 2009. 11. 21  ������	���������ӱ�������
		if( IsFirstConnectSucc() )
		{
			//			START_LOG( cerr, GET_WSA_MSG );
		}
		//}}

		CLOSE_SOCKET( m_sock );

		//{{ 2010. 10. 13	������	�ؽ� ���� ���� ����
		OnDisconnectBillingServer( std::wstring( L"��ȿ ���� ���� ����" ) );
		//}}
		return;
	}

	if( ret == 0 )
	{
		START_LOG( cerr, L"���� ������ ������." )
			<< END_LOG;

		CLOSE_SOCKET( m_sock );

		//{{ 2010. 10. 13	������	�ؽ� ���� ���� ����
		OnDisconnectBillingServer( std::wstring( L"���������� ���� ����" ) );
		//}}
		return;
	}

	m_ulRecvCP += ret;

	// ��� ���۰� �� ����������� ��Ŷ�� �����Ѵ�.
	while( m_ulRecvCP >= 10 )
	{
		// ���(9) + Ÿ��(1) = 10
		// ��� ��Ŷ�� 10����Ʈ �̻���

		// ���� ���۰� ����� �ִ�!
		if( m_kRecvVariableBuffer.IsAlreadyRecv() == false )
		{
			// ��Ŷ ũ�� �м�!
			unsigned long ulLength;
			::memcpy( &ulLength, m_cRecvBuffer + 1, sizeof( ulLength ) );

			ulLength = ::ntohl( ulLength );
			unsigned long ulTotalPacketSize = ulLength + 5;

			// ���� �ȵǴ� ũ���� ��Ŷ�̶�� ���⼭ ��� ����!
			if( ulTotalPacketSize > VARIABLE_BUFFER_MAX_SIZE )
			{
				//{{ 2013. 01. 08	������	���� ��Ŷ ��ġ�� ���� ã��
#ifdef SERV_BILLING_TCP_PACKET_MAX_OVER_USER_CHECK
				// ���� ���ƴ����� ��������!
				UidType iUserUID = 0;
				BYTE bytePacketType = 0;
				std::wstring wstrPacketTypeName;
				CheckPacketMaxSizeOverUser( m_cRecvBuffer, iUserUID, bytePacketType, wstrPacketTypeName );
#endif SERV_BILLING_TCP_PACKET_MAX_OVER_USER_CHECK
				//}}

				//{{ 2010. 10. 13	������	�ؽ� ���� ���� ����
				std::wstring wstrReason = boost::str( boost::wformat( L"�ִ� ��Ŷ ������ �ʰ�! UserUID : %d, MaxPacketSize : %d, PacketType : %s" ) % iUserUID % ulTotalPacketSize % wstrPacketTypeName );
				OnDisconnectBillingServer( wstrReason );
				//}}

				START_LOG( cerr, L"������ ��Ŷ ������ �̻�." )
					<< BUILD_LOG( iUserUID )
					<< BUILD_LOG( ulTotalPacketSize )
					<< BUILD_LOG( MAX_PACKET_SIZE_NBT )
					<< BUILD_LOG( wstrPacketTypeName )
					<< END_LOG;

				CLOSE_SOCKET( m_sock );
				return;
			}

			//{{ 2013. 01. 23	������	���� ��Ŷ ���� ó�� ���� ����
#ifdef SERV_BILLING_PACKET_RECV_PERFORMANCE_CHECK
			// �������!
			BeginCheckPerformance();
#endif SERV_BILLING_PACKET_RECV_PERFORMANCE_CHECK
			//}}

			// �ϴ� ��Ŷ ũ�⸸ŭ ���� ���۸� ������!
			m_kRecvVariableBuffer.Init( ulTotalPacketSize );

			// ������ ��Ŷ ũ�� �ϳ� ��ŭ �����͸� ��� �޾Ҵ��� Ȯ��
			if( ulTotalPacketSize <= m_ulRecvCP )
			{
				m_kRecvVariableBuffer.CopyBuffer( m_cRecvBuffer, ulTotalPacketSize );

				// ī�� �� ��ŭ ���۸� �̵� ��Ű��!				
				::memmove( m_cRecvBuffer, m_cRecvBuffer + ulTotalPacketSize, m_ulRecvCP - ulTotalPacketSize );
				m_ulRecvCP -= ulTotalPacketSize;

				//////////////////////////////////////////////////////////////////////////
				START_LOG( cout, L"[�׽�Ʈ�α�] ������ ��Ŷ ũ�� �ϳ���ŭ �����͸� �޾Ҵ�! �������ۿ� ���ŵ� ��Ŷ���� ����!" )
					<< BUILD_LOG( ret )
					<< BUILD_LOG( m_ulRecvCP )
					<< BUILD_LOG( ulTotalPacketSize );
				//////////////////////////////////////////////////////////////////////////
			}
			// �� �޾Ҵٸ� �� ���� ��� �ϴ� �־��!
			else
			{
				m_kRecvVariableBuffer.CopyBuffer( m_cRecvBuffer, m_ulRecvCP );

				// ������ �����͸� ������ ī�������� ���۴� �ʱ�ȭ ����!
				m_ulRecvCP = 0;
				//memset( m_cRecvBuffer, 0, MAX_PACKET_SIZE_NBT ); -- ���� �ʱ�ȭ�� �ʿ䰡 ������?

				//////////////////////////////////////////////////////////////////////////
				START_LOG( cout, L"[�׽�Ʈ�α�] ������ ��Ŷ ��ŭ�� �ƴ����� �ϴ� �����͸� �޾Ҵ�! �������ۿ� ���ŵ� ��Ŷ���� ����!" )
					<< BUILD_LOG( ret )
					<< BUILD_LOG( m_ulRecvCP )
					<< BUILD_LOG( ulTotalPacketSize );
				//////////////////////////////////////////////////////////////////////////
			}
		}
		// �׿��ִ� �����Ͱ� �ִ�!
		else
		{
			const unsigned long ulRemainSize = m_kRecvVariableBuffer.GetRemainSize();

			// ���� ��Ŷ��ŭ ��Ŷ�� �����ߴ°�?
			if( ulRemainSize <= m_ulRecvCP )
			{
				m_kRecvVariableBuffer.CopyBuffer( m_cRecvBuffer, ulRemainSize );

				// ī�� �� ��ŭ ���۸� �̵� ��Ű��!
				::memmove( m_cRecvBuffer, m_cRecvBuffer + ulRemainSize, m_ulRecvCP - ulRemainSize );
				m_ulRecvCP -= ulRemainSize;

				//////////////////////////////////////////////////////////////////////////
				START_LOG( cout, L"[�׽�Ʈ�α�] �̹� ���� ���ۿ� �޾� ���� �����Ϳ��ٰ� �� �߰��ؼ� ������ �ϳ��� ��Ŷ���� ������!" )
					<< BUILD_LOG( ret )
					<< BUILD_LOG( m_ulRecvCP )
					<< BUILD_LOG( ulRemainSize );
				//////////////////////////////////////////////////////////////////////////
			}
			// ���� ��Ŷ��ŭ ��Ŷ�� ���޾Ҵ�.
			else
			{
				m_kRecvVariableBuffer.CopyBuffer( m_cRecvBuffer, m_ulRecvCP );

				// ������ �����͸� ������ ī�������� ���۴� �ʱ�ȭ ����!
				m_ulRecvCP = 0;
				//memset( m_cRecvBuffer, 0, MAX_PACKET_SIZE ); -- ���� �ʱ�ȭ�� �ʿ䰡 ������?

				//////////////////////////////////////////////////////////////////////////
				START_LOG( cout, L"[�׽�Ʈ�α�] �̹� ���� ���ۿ� �޾� ���� �����Ϳ��ٰ� �� �߰��ؼ� ���ŵ� ��Ŷ���� ����! �ٵ� �� �޾ƾ߉�!" )
					<< BUILD_LOG( ret )
					<< BUILD_LOG( m_ulRecvCP )
					<< BUILD_LOG( ulRemainSize );
				//////////////////////////////////////////////////////////////////////////
			}
		}

		// ��Ŷ�� �ϼ� �Ǿ��°�?
		if( m_kRecvVariableBuffer.IsComplete() == true )
		{
			//////////////////////////////////////////////////////////////////////////
			START_LOG( cout, L"[�׽�Ʈ�α�] ���� ���ۿ� ������ ��Ŷ �ϳ��� �ϼ�!!!" )
				<< BUILD_LOG( ret )
				<< BUILD_LOG( m_ulRecvCP )
				<< BUILD_LOG( m_kRecvVariableBuffer.GetBufferSize() )
				<< BUILD_LOG( (int)m_kRecvVariableBuffer.GetBuffer() );
			//////////////////////////////////////////////////////////////////////////

			// �ϼ��� ��Ŷ�� �ִ� ��Ŷ ����� �Ѵ��� Ȯ��!
			if( m_kRecvVariableBuffer.GetBufferSize() <= MAX_PACKET_SIZE_NBT )
			{
				// ������ �Ѱ��� ��Ŷ�� ������!
				MakeEventFromReceivedPacket();
			}
			else
			{
				// ���� ���ƴ����� ��������!
				UidType iUserUID = 0;
				BYTE bytePacketType = 0;
				std::wstring wstrPacketTypeName;
				CheckPacketMaxSizeOverUser( m_kRecvVariableBuffer.GetBuffer(), iUserUID, bytePacketType, wstrPacketTypeName );

                START_LOG( cerr, L"�ִ� ��Ŷ ����� �Ѿ�� ��Ŷ�� ���ŵǾ����ϴ�! �ش� ��Ŷ�� �Ҹ� ��ŵ�ϴ�!" )
					<< BUILD_LOG( iUserUID )
					<< BUILD_LOGc( bytePacketType )
					<< BUILD_LOG( wstrPacketTypeName )
					<< END_LOG;
			}

			// ���� ��Ŷ�� �ޱ� ���� ���� �ʱ�ȭ! ( �޸� ���Ҵ��� ���� �ʱ����� Reset()�Լ��� ��� )
			m_kRecvVariableBuffer.Reset();

			//{{ 2013. 01. 23	������	���� ��Ŷ ���� ó�� ���� ����
#ifdef SERV_BILLING_PACKET_RECV_PERFORMANCE_CHECK
			// ��������� �ð��� ��� �ɰ� ����!
			EndCheckPerformance();
#endif SERV_BILLING_PACKET_RECV_PERFORMANCE_CHECK
			//}}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
#else
//////////////////////////////////////////////////////////////////////////
void KNexonBillingTCPManager::Recv()
{
	if( !IsConnected() )
	{
		return;
	}

	int ret;
	ret = ::recv( m_sock,
		m_cRecvBuffer + m_ulRecvCP,
		( int )( MAX_PACKET_SIZE_NBT - m_ulRecvCP ),
		0 );

	START_LOG( clog, L"��Ŷ ����." )
		<< BUILD_LOG( ret );

	if( ret == SOCKET_ERROR )
	{
		//{{ 2009. 11. 21  ������	���������ӱ�������
		if( IsFirstConnectSucc() )
		{
			//			START_LOG( cerr, GET_WSA_MSG );
		}
		//}}

		CLOSE_SOCKET( m_sock );

		//{{ 2010. 10. 13	������	�ؽ� ���� ���� ����
		OnDisconnectBillingServer( std::wstring( L"��ȿ ���� ���� ����" ) );
		//}}
		return;
	}

	if( ret == 0 )
	{
		START_LOG( cerr, L"���� ������ ������." )
			<< END_LOG;

		CLOSE_SOCKET( m_sock );

		//{{ 2010. 10. 13	������	�ؽ� ���� ���� ����
		OnDisconnectBillingServer( std::wstring( L"���������� ���� ����" ) );
		//}}
		return;
	}

	m_ulRecvCP += ret;

	while( m_ulRecvCP >= 10 )
	{
		// ���(9) + Ÿ��(1) = 10
		// ��� ��Ŷ�� 10����Ʈ �̻���

		unsigned long ulLength;
		::memcpy( &ulLength, m_cRecvBuffer + 1, sizeof( ulLength ) );

		ulLength = ::ntohl( ulLength );
		unsigned long ulTotalPacketSize = ulLength + 5;
		if( ulTotalPacketSize > MAX_PACKET_SIZE_NBT )
		{
			//{{ 2013. 01. 08	������	���� ��Ŷ ��ġ�� ���� ã��
#ifdef SERV_BILLING_TCP_PACKET_MAX_OVER_USER_CHECK
			// ���� ���ƴ����� ��������!
			UidType iUserUID = 0;
			BYTE bytePacketType = 0;
			std::wstring wstrPacketTypeName;
			CheckPacketMaxSizeOverUser( m_cRecvBuffer, iUserUID, bytePacketType, wstrPacketTypeName );

			//{{ 2010. 10. 13	������	�ؽ� ���� ���� ����
			std::wstring wstrReason = boost::str( boost::wformat( L"�ִ� ��Ŷ ������ �ʰ�! UserUID : %d, MaxPacketSize : %d, PacketType : %s" ) % iUserUID % ulTotalPacketSize % wstrPacketTypeName );
			OnDisconnectBillingServer( wstrReason );
			//}}

			START_LOG( cerr, L"������ ��Ŷ ������ �̻�." )
				<< BUILD_LOG( iUserUID )
				<< BUILD_LOG( ulTotalPacketSize )
				<< BUILD_LOG( MAX_PACKET_SIZE_NBT )
				<< BUILD_LOG( wstrPacketTypeName )
				<< END_LOG;

#endif SERV_BILLING_TCP_PACKET_MAX_OVER_USER_CHECK	// �ؿ��� ��ġ ����
			//}}

			CLOSE_SOCKET( m_sock );
			return;
		}

		if( ulTotalPacketSize > m_ulRecvCP )
		{
			return;
		}

		//{{ 2013. 01. 23	������	���� ��Ŷ ���� ó�� ���� ����
#ifdef SERV_BILLING_PACKET_RECV_PERFORMANCE_CHECK
		// �������!
		BeginCheckPerformance();
#endif SERV_BILLING_PACKET_RECV_PERFORMANCE_CHECK
		//}}

		MakeEventFromReceivedPacket();

		if( m_ulRecvCP > ulTotalPacketSize )
		{
			::memmove( m_cRecvBuffer, m_cRecvBuffer + ulTotalPacketSize, m_ulRecvCP - ulTotalPacketSize );
		}
		m_ulRecvCP -= ulTotalPacketSize;

		//{{ 2013. 01. 23	������	���� ��Ŷ ���� ó�� ���� ����
#ifdef SERV_BILLING_PACKET_RECV_PERFORMANCE_CHECK
		// ��������� �ð��� ��� �ɰ� ����!
		EndCheckPerformance();
#endif SERV_BILLING_PACKET_RECV_PERFORMANCE_CHECK
		//}}
	}
}
//////////////////////////////////////////////////////////////////////////
#endif SERV_BILLING_TCP_RECV_VARIABLE_SIZE_BUFFER
//////////////////////////////////////////////////////////////////////////
//}}

void KNexonBillingTCPManager::Send()
{
	if( !IsConnected() )
	{
        return;
	}

	int ret;
	char buf[MAX_PACKET_SIZE_NBT] = {0,}; // 2012. 12. 3   ���� �ʱ�ȭ�� �迭�� ������ �κп��� ���ֵ��� ����.
    KNexonBillingTCPPacketPtr spPacket;
	while( GetSendPacket( spPacket ) )
	{
		if( !spPacket )
		{
			unsigned int uiSendQueueSize;
			KCSLOCK_BEGIN( m_kSendQueue )
				uiSendQueueSize = m_kSendQueue.size();
			KCSLOCK_END()

			START_LOG( cerr, L"������ �̻�." )
				<< BUILD_LOG( uiSendQueueSize )
				<< END_LOG;

			continue;
		}

		spPacket->WriteToBuffer( ( BYTE* )buf );
        DumpBuffer( ( BYTE* )buf, false );

        char* cCurBufPos = buf;
		int iRemainedSendSize = spPacket->GetTotalLength();
		while( iRemainedSendSize > 0 )
		{
			ret = ::send( m_sock,
				cCurBufPos,
				iRemainedSendSize,
				0 );

			if( ret == SOCKET_ERROR )
			{
				START_LOG( cwarn, GET_WSA_MSG );
				CLOSE_SOCKET( m_sock );
				return;
			}

			iRemainedSendSize -= ret;
            cCurBufPos += ret;
		}
	}
}

//{{ 2010. 10. 13	������	�ؽ� ���� ���� ����	
void KNexonBillingTCPManager::OnDisconnectBillingServer( IN const std::wstring& wstrReason )
{
	// ���� ������ ���� �������� �ʾұ� ������ ���� ���� �������� ���� �ʴ´�.
	if( IsFirstConnectSucc() == false )
		return;

	//{{ 2010. 10. 11	������	������ ���� ���� �α�
#ifdef SERV_SERVER_DISCONNECT_LOG
	{
		CTime kRegDate = CTime::GetCurrentTime();
		KE_LOCAL_LOG_SERVER_DISCONNECT_NOT kNot;
		kNot.m_cLogType				= KE_LOCAL_LOG_SERVER_DISCONNECT_NOT::SDLT_DISCONNECT;
		kNot.m_wstrSourceName		= boost::str( boost::wformat( L"%d" ) % KBaseServer::GetKObj()->GetUID() );
		kNot.m_wstrDestinationName	= L"Nexon Billing";
		kNot.m_wstrDestinationIP	= KncUtil::toWideString( m_kNexonBillingTCPInfo.m_strIP );
		kNot.m_wstrReason			= wstrReason;
		kNot.m_wstrRegDate			= (const wchar_t*)kRegDate.Format(_T("%Y-%m-%d %H:%M:%S"));
		KSIManager.QueueingEvent( E_LOCAL_LOG_SERVER_DISCONNECT_NOT, kNot );
	}
#endif SERV_SERVER_DISCONNECT_LOG
	//}}
	//{{ 2010. 10. 12	������	���� ����͸�
#ifdef SERV_MORNITORING
	{
		KE_DISCONNECT_SERVER_REPORT_NOT kNot;
		kNot.m_cType = KMornitoringManager::ET_DISCONNECT_GAME_BILLING;
		KEventPtr spEvent( new KEvent );
		spEvent->SetData( 0, NULL, E_DISCONNECT_SERVER_REPORT_NOT, kNot );
		KBaseServer::GetKObj()->QueueingEvent( spEvent );
	}
#endif SERV_MORNITORING
	//}}

	START_LOG( cout, L"�ؽ� ���� ���� ���� ����!" )
		<< BUILD_LOG( wstrReason );
}
//}}

//{{ 2009. 11. 21  ������	���������ӱ�������
void KNexonBillingTCPManager::KeepConnectionThread()
{
	if( !IsFirstConnectSucc() )
	{
		// ���� �������� ���� �õ�. connect()�Լ� ȣ���߿��� Thread Block
		if( Connect() )
		{
			FirstConnectSucc();
		}
		
		return;
	}

	CheckConnection();

	KeepConnection();
}
//}}

bool KNexonBillingTCPManager::Connect()
{
	//{{ [����] Connect()�Լ��� ȣ��Ǵ� ��Ȳ������ RecvThread���� recv()�Լ��� �׻� ȣ������Ѵ�.
	//          ���� m_ulRecvCP������ 0���� �ʱ�ȭ �Ǵ��� �������� ����.
    m_ulRecvCP = 0;
	//}}

	m_sock = ::socket( AF_INET, SOCK_STREAM, 0 );    // ���� ����

	if( INVALID_SOCKET == m_sock )
	{
		START_LOG( cerr, GET_WSA_MSG )
			<< BUILD_LOG( WSAGetLastError() )
			<< END_LOG;

		return false;
	}

	SOCKADDR_IN addr;
	addr.sin_family			= AF_INET;
	addr.sin_addr.s_addr	= inet_addr( m_kNexonBillingTCPInfo.m_strIP.c_str() );
	addr.sin_port			= htons( m_kNexonBillingTCPInfo.m_usPort );

	int ret = ::connect( m_sock, ( struct sockaddr* )&addr, sizeof( addr ) );

	if( SOCKET_ERROR == ret )
	{
		//{{ 2009. 11. 21  ������	���������ӱ�������
		if( IsFirstConnectSucc() )
		{
//			START_LOG( cerr, GET_WSA_MSG )
//				<< BUILD_LOG( WSAGetLastError() )
//				<< BUILD_LOG( m_kNexonBillingTCPInfo.m_strIP )
//				<< BUILD_LOG( m_kNexonBillingTCPInfo.m_usPort )
//				<< END_LOG;
		}
		//}}

		CLOSE_SOCKET( m_sock );
		return false;
	}

    // KENX_BT_INITIALIZE_REQ�� ť���ϱ� ���� false �� ������ �Ѵ�.
    m_bInitialized = false;

    KENX_BT_INITIALIZE_REQ kPacketInit;
    kPacketInit.m_ulPacketNo = GetNextPacketNo();
    kPacketInit.m_bytePacketType = KNexonBillingTCPPacket::INITIALIZE;
    kPacketInit.m_wstrServiceCode = L"ELSWD";
    kPacketInit.m_byteDomain = m_kNexonBillingTCPInfo.m_iDomain;

    boost::shared_ptr< KNexonBillingTCPPacket > spPacketInit( new KNexonBillingTCPPacket );
    spPacketInit->Write( kPacketInit );
    QueueingSendPacket( spPacketInit );

    switch( ::WaitForSingleObject( m_spEvent.get(), 5000 ) )
    {
    case WAIT_OBJECT_0: // ack�� ����� ���� ���.
        START_LOG( cout, L"�ؽ� ���� ���� Initialize." )
            << BUILD_LOG( m_bInitialized );
        break;
    case WAIT_TIMEOUT:  // �ð� �ʰ�
        START_LOG( cerr, L"���� ���� Initialize �ð� �ʰ�." );
        break;
    default:
        START_LOG( cerr, L"���� ���� Initialize ����." );
        break;
    }

    if( !m_bInitialized )
    {
        CLOSE_SOCKET( m_sock );
        return false;
    }

    return true;
}

bool KNexonBillingTCPManager::IsConnected()
{
	return ( m_sock != INVALID_SOCKET );
}

void KNexonBillingTCPManager::KeepConnection()
{
	if( ::GetTickCount() - m_dwLastHeartBeatTick < ms_dwHeartBeatGap )
	{
		return;
	}

    m_dwLastHeartBeatTick = ::GetTickCount();

	//{{ 2008. 5. 30  ������  �ؽ����� �ʱ�ȭ ����
	if( !m_bInitialized  ||  !IsConnected() )
		return;
	//}}

	// ��Ʈ�� ������
    KENX_BT_HEART_BEAT_REQ kPacketNot;
    kPacketNot.m_ulPacketNo = GetNextPacketNo();
    kPacketNot.m_bytePacketType = KNexonBillingTCPPacket::HEART_BEAT;
    kPacketNot.m_iReleaseTick = GetReleaseTick();
    boost::shared_ptr< KNexonBillingTCPPacket > spPacket( new KNexonBillingTCPPacket );
    spPacket->Write( kPacketNot );
    QueueingSendPacket( spPacket );
}

void KNexonBillingTCPManager::CheckConnection()
{
	if( ::GetTickCount() - m_dwLastConnectionCheckTick < ms_dwConnectionCheckGap )
	{
		return;
	}

    m_dwLastConnectionCheckTick = ::GetTickCount();

	if( !IsConnected() )
	{
		// �ʱ�ȭ ���� ���� ���·� ����!
		m_bInitialized = false;

		Connect();
	}
}

bool KNexonBillingTCPManager::GetSendPacket( KNexonBillingTCPPacketPtr& spPacket )
{
	KCSLOCK_BEGIN( m_kSendQueue )
		if( m_kSendQueue.empty() )
		{
			return false;
		}

		spPacket = m_kSendQueue.front();
		m_kSendQueue.pop();
	KCSLOCK_END()

    return true;
}

//{{ 2013. 01. 10	������	���� TCP ��Ŷ ũ�� ����ó��
//////////////////////////////////////////////////////////////////////////
#ifdef SERV_BILLING_TCP_RECV_VARIABLE_SIZE_BUFFER
//////////////////////////////////////////////////////////////////////////
void KNexonBillingTCPManager::MakeEventFromReceivedPacket()
{
	DumpBuffer( ( BYTE* )m_kRecvVariableBuffer.GetBuffer(), false );
	
	// ���� ���ۿ��� ������ �ϳ��� ��Ŷ�� ����!
	KNexonBillingTCPPacket kPacket;
	if( !kPacket.ReadFromBuffer( ( BYTE* )m_kRecvVariableBuffer.GetBuffer() ) )
	{
		START_LOG( cerr, L"���ۿ��� �б� ����." )
			<< END_LOG;

		DumpBuffer( ( BYTE* )m_kRecvVariableBuffer.GetBuffer(), true );
		return;
	}

	switch( kPacket.GetPacketType() )
	{
#   undef  _ENUM
#   define _ENUM( name, id ) \
	case id: \
		{ \
		K##ENX_BT_##name##_ACK kPacketAck; \
		kPacket.Read( kPacketAck ); \
		KEventPtr spEvent( new KEvent ); \
		spEvent->SetData( PI_GS_NX_BILLING_TCP, NULL, ENX_BT_##name##_ACK, kPacketAck ); \
		QueueingEvent( spEvent ); \
		} \
		break;

//#ifdef SERV_NEXON_COUPON_SYSTEM// �۾���¥: 2013-06-23	// �ڼ���
#undef _ENUM_PROCESS
#define _ENUM_PROCESS( name, id, pname ) _ENUM( pname, id )
//#endif // SERV_NEXON_COUPON_SYSTEM

#   include "NexonBillingTCP_def.h"
	default:
		START_LOG( cerr, L"��Ŷ Ÿ���� �̻���." )
			<< BUILD_LOG( kPacket.GetPacketType() )
			<< END_LOG;
		DumpBuffer( ( BYTE* )m_kRecvVariableBuffer.GetBuffer(), true );
		break;
	}
}

void KNexonBillingTCPManager::DumpBuffer( const BYTE* buffer, bool bError )
{
	JIF( buffer );
	JIF( sizeof( buffer ) >= 4 );

	unsigned long ulLength;
	::memcpy( &ulLength, buffer + 1, sizeof( ulLength ) );

	ulLength = ::ntohl( ulLength );
	unsigned long ulTotalPacketSize = ulLength + 5;

	// LIF( ulTotalPacketSize <= MAX_PACKET_SIZE_NBT ); ���� ���۸� ���� �ִ� ��Ŷ ũ�⸦ �Ѿ�� �ִ�!
	//ulTotalPacketSize = std::min< int >( ulTotalPacketSize, MAX_PACKET_SIZE_NBT );

	char szBuffer[MAX_PACKET_SIZE_NBT * 3 + 1];
	char szByte[4];

	for( unsigned long i = 0; i < ulTotalPacketSize; i++ )
	{
		if( ( int )buffer[i] >= 16 )
		{
			::itoa( ( int )buffer[i], szByte, 16 );
			szBuffer[i * 3] = szByte[0];
			szBuffer[i * 3 + 1] = szByte[1];
			szBuffer[i * 3 + 2] = ' ';
		}
		else
		{
			::itoa( ( int )buffer[i], szByte, 16 );
			szBuffer[i * 3] = '0';
			szBuffer[i * 3 + 1] = szByte[0];
			szBuffer[i * 3 + 2] = ' ';
		}
	}
	szBuffer[ulTotalPacketSize * 3] = '\0';

	std::string strBuffer = szBuffer;

	if( bError )
	{
		START_LOG( cerr2, L"BUFFER : ")
			<< BUILD_LOG( strBuffer );
	}
	else
	{
		START_LOG( clog2, L"BUFFER : ")
			<< BUILD_LOG( strBuffer );
	}
}
//////////////////////////////////////////////////////////////////////////
#else
//////////////////////////////////////////////////////////////////////////
void KNexonBillingTCPManager::MakeEventFromReceivedPacket()
{
    DumpBuffer( ( BYTE* )m_cRecvBuffer, false );
	KNexonBillingTCPPacket kPacket;
	if( !kPacket.ReadFromBuffer( ( BYTE* )m_cRecvBuffer ) )
    {
        START_LOG( cerr, L"���ۿ��� �б� ����." )
            << END_LOG;

        DumpBuffer( ( BYTE* )m_cRecvBuffer, true );
        return;
    }

    switch( kPacket.GetPacketType() )
    {
#   undef  _ENUM
#   define _ENUM( name, id ) \
    case id: \
        { \
            K##ENX_BT_##name##_ACK kPacketAck; \
            kPacket.Read( kPacketAck ); \
            KEventPtr spEvent( new KEvent ); \
            spEvent->SetData( PI_GS_NX_BILLING_TCP, NULL, ENX_BT_##name##_ACK, kPacketAck ); \
            QueueingEvent( spEvent ); \
        } \
        break;

//#ifdef SERV_NEXON_COUPON_SYSTEM// �۾���¥: 2013-06-23	// �ڼ���
#undef _ENUM_PROCESS
#define _ENUM_PROCESS( name, id, pname ) _ENUM( pname, id )
//#endif // SERV_NEXON_COUPON_SYSTEM

#   include "NexonBillingTCP_def.h"
    default:
        START_LOG( cerr, L"��Ŷ Ÿ���� �̻���." )
            << BUILD_LOG( kPacket.GetPacketType() )
            << END_LOG;
        DumpBuffer( ( BYTE* )m_cRecvBuffer, true );
        break;
    }
}

void KNexonBillingTCPManager::DumpBuffer( const BYTE* buffer, bool bError )
{
	JIF( buffer );
	JIF( sizeof( buffer ) >= 4 );

	unsigned long ulLength;
	::memcpy( &ulLength, buffer + 1, sizeof( ulLength ) );

	ulLength = ::ntohl( ulLength );
	unsigned long ulTotalPacketSize = ulLength + 5;

	LIF( ulTotalPacketSize <= MAX_PACKET_SIZE_NBT );
	ulTotalPacketSize = std::min< int >( ulTotalPacketSize, MAX_PACKET_SIZE_NBT );

	char szBuffer[MAX_PACKET_SIZE_NBT * 3 + 1];
	char szByte[4];

	for( unsigned long i = 0; i < ulTotalPacketSize; i++ )
	{
		if( ( int )buffer[i] >= 16 )
		{
			::itoa( ( int )buffer[i], szByte, 16 );
			szBuffer[i * 3] = szByte[0];
			szBuffer[i * 3 + 1] = szByte[1];
			szBuffer[i * 3 + 2] = ' ';
		}
		else
		{
			::itoa( ( int )buffer[i], szByte, 16 );
			szBuffer[i * 3] = '0';
			szBuffer[i * 3 + 1] = szByte[0];
			szBuffer[i * 3 + 2] = ' ';
		}
	}
	szBuffer[ulTotalPacketSize * 3] = '\0';

	std::string strBuffer = szBuffer;

	if( bError )
	{
		START_LOG( cerr2, L"BUFFER : ")
			<< BUILD_LOG( strBuffer );
	}
	else
	{
		START_LOG( clog2, L"BUFFER : ")
			<< BUILD_LOG( strBuffer );
	}
}
//////////////////////////////////////////////////////////////////////////
#endif SERV_BILLING_TCP_RECV_VARIABLE_SIZE_BUFFER
//////////////////////////////////////////////////////////////////////////
//}}

void KNexonBillingTCPManager::ResetProductPageNumber()
{
	KCSLOCK_SET_VALUE( m_ulCurrentProductPage, 0 );
}

void KNexonBillingTCPManager::ResetProductInfo()
{
    KLocker lock( m_csProductInfo );

    m_mapProductInfo.clear();
	//{{ 2012. 03. 21	��μ�		ĳ�� ��ǰ ����Ʈ ������ �������� DB�� ���
#ifdef SERV_CASH_ITEM_LIST_ADD
	m_mapProductInfo_Excepted.clear();
#endif SERV_CASH_ITEM_LIST_ADD
	//}}
}

void KNexonBillingTCPManager::GetAutoPaymentProductList( std::vector< unsigned long >& vecProductList )
{
	KCSLOCK_BEGIN( m_vecAutoPaymentProductList )
		vecProductList = m_vecAutoPaymentProductList;
	KCSLOCK_END()
}

//{{ 2011. 02. 23	������	ĳ�� ��ǰ ����Ʈ
#ifdef SERV_CASH_ITEM_LIST
void KNexonBillingTCPManager::GetCashProductList( OUT std::vector< KCashPruductInfo >& vecCashProductList ) const
{
	KLocker lock( m_csProductInfo );

	std::map< unsigned long, KNXBTProductInfo >::const_reverse_iterator mit;
	for( mit = m_mapProductInfo.rbegin(); mit != m_mapProductInfo.rend(); ++mit )
	{
		const int iCashItemID = _wtoi( mit->second.m_wstrProductID.c_str() );

		KCashPruductInfo kCashProductInfo;
		kCashProductInfo.m_ulProductNo		= mit->second.m_ulProductNo;
		kCashProductInfo.m_sProductExpire	= mit->second.m_sProductExpire;
		kCashProductInfo.m_usProductPieces	= mit->second.m_usProductPieces;
		kCashProductInfo.m_iItemID			= iCashItemID;
		kCashProductInfo.m_ulSalePrice		= mit->second.m_ulSalePrice;
		vecCashProductList.push_back( kCashProductInfo );
	}

#ifdef SERV_CASH_ITEM_LIST_ADD	// ���� ������ �ؿ��� �߰�
	std::map< unsigned long, KNXBTProductInfo >::const_reverse_iterator mit_Excepted;
	for( mit_Excepted = m_mapProductInfo_Excepted.rbegin(); mit_Excepted != m_mapProductInfo_Excepted.rend(); ++mit_Excepted )
	{
		const int iCashItemID = _wtoi( mit_Excepted->second.m_wstrProductID.c_str() );

		KCashPruductInfo kCashProductInfo;
		kCashProductInfo.m_ulProductNo		= mit_Excepted->second.m_ulProductNo;
		kCashProductInfo.m_sProductExpire	= mit_Excepted->second.m_sProductExpire;
		kCashProductInfo.m_usProductPieces	= mit_Excepted->second.m_usProductPieces;
		kCashProductInfo.m_iItemID			= iCashItemID;
		kCashProductInfo.m_ulSalePrice		= mit_Excepted->second.m_ulSalePrice;
		vecCashProductList.push_back( kCashProductInfo );
	}
#endif // SERV_CASH_ITEM_LIST_ADD

}
#endif SERV_CASH_ITEM_LIST


//{{ 2010. 11. 22	������	û�� öȸ
#ifdef SERV_NX_BILLING_REFUND
bool KNexonBillingTCPManager::GetCashItemRefundRequestID( OUT std::wstring& wstrRequestID )
{
	__int64 iServerUID = KBaseServer::GetKObj()->GetUID();
	if( iServerUID > 0xFF )
	{
		START_LOG( cerr, L"ServerUID�� 255�� ������ �ȵ˴ϴ�!" )
			<< BUILD_LOG( iServerUID )
			<< END_LOG;
		return false;
	}

	// ��¥ ���� �ֱ�
	{
		CTime tCurTime = CTime::GetCurrentTime();
		wstrRequestID = ( std::wstring )tCurTime.Format( _T( "%Y%m%d%H%M%S" ) );
	}

	// ���Ӽ��� �߱� ID�ֱ�
	{
		__int64 iCashItemRefundRequestID = 0;

		KCSLOCK_BEGIN( m_iCashItemRefundRequestID )
			++m_iCashItemRefundRequestID;
			iCashItemRefundRequestID = m_iCashItemRefundRequestID;
		KCSLOCK_END()

		__int64 iResultRequestID = ( iCashItemRefundRequestID & 0xffffffff00ffffff ) | ( ( ( iServerUID & ( __int64 )0x00000000000000ff ) << 24 ) & 0x00000000ff000000 );

		wstrRequestID += boost::str( boost::wformat( L"%010d" ) % iResultRequestID );
	}
	
	return true;
}
#endif SERV_NX_BILLING_REFUND
//}}

//{{ 2013. 01. 08	������	���� ��Ŷ ��ġ�� ���� ã��
#ifdef SERV_BILLING_TCP_PACKET_MAX_OVER_USER_CHECK
bool KNexonBillingTCPManager::CheckPacketMaxSizeOverUser( IN const char* pBuffer, OUT UidType& iUserUID, OUT BYTE& bytePacketType, OUT std::wstring& wstrPacketTypeName )
{
	iUserUID = 0;
	bytePacketType = 0;

	BYTE byteReservedChar = 0;
	ULONG ulPacketLength = 0;
	ULONG ulPacketNo = 0;
	
	// ���� ����
	::memcpy( &byteReservedChar, pBuffer, sizeof( BYTE ) );
	_JIF( byteReservedChar == 0xAF, return false );

	// ��Ŷ ����
	::memcpy( &ulPacketLength, pBuffer + 1, sizeof( unsigned long ) );
	ulPacketLength = ::ntohl( ulPacketLength );
	//_JIF( ulPacketLength >= HEADER_IN_PACKET_LENGTH_NBT, return false );
	//_JIF( ulPacketLength - HEADER_IN_PACKET_LENGTH_NBT <= MAX_PACKET_CONTENT_SIZE_NBT, return false );

	// ��Ŷ �ѹ�
	::memcpy( &ulPacketNo, pBuffer + 5, sizeof( unsigned long ) );
	ulPacketNo = ::ntohl( ulPacketNo );

	// ��Ŷ Ÿ��
	::memcpy( &bytePacketType, pBuffer + 9, sizeof( BYTE ) );

	// � ������ ��û�� ��Ŷ���� Ȯ������!
	iUserUID = GetCorrespondingUserUID( ulPacketNo );

	// ��Ŷ Ÿ�� �̸��� ����!
	wstrPacketTypeName = KNexonBillingTCPPacket::GetPacketTypeStr( bytePacketType );

	START_LOG( cout, L"[�˸�] �ִ� ��Ŷ ũ�⸦ �Ѿ ��Ŷ�� ��� ����!" )
		<< BUILD_LOGc( byteReservedChar )
		<< BUILD_LOG( ulPacketLength )
		<< BUILD_LOG( ulPacketNo )
		<< BUILD_LOGc( bytePacketType )
		<< BUILD_LOG( wstrPacketTypeName );
	return true;
}
#endif SERV_BILLING_TCP_PACKET_MAX_OVER_USER_CHECK
//}}

//{{ 2013. 01. 23	������	���� ��Ŷ ���� ó�� ���� ����
#ifdef SERV_BILLING_PACKET_RECV_PERFORMANCE_CHECK
void KNexonBillingTCPManager::BeginCheckPerformance()
{
	m_kRecvPerformanceDump.m_dwLastTickCount = ::GetTickCount();
}

void KNexonBillingTCPManager::EndCheckPerformance()
{
	const DWORD dwLatency = ::GetTickCount() - m_kRecvPerformanceDump.m_dwLastTickCount;
	if( dwLatency < m_kRecvPerformanceDump.m_dwMinLatency )
	{
		CTime tCurTime = CTime::GetCurrentTime();

		m_kRecvPerformanceDump.m_dwMinLatency = dwLatency;
        m_kRecvPerformanceDump.m_wstrMinRegDate = (std::wstring)tCurTime.Format( _T("%Y-%m-%d %H:%M:%S") );
	}

	if( dwLatency > m_kRecvPerformanceDump.m_dwMaxLatency )
	{
		CTime tCurTime = CTime::GetCurrentTime();

		m_kRecvPerformanceDump.m_dwMaxLatency = dwLatency;
		m_kRecvPerformanceDump.m_wstrMaxRegDate = (std::wstring)tCurTime.Format( _T("%Y-%m-%d %H:%M:%S") );
	}
}
#endif SERV_BILLING_PACKET_RECV_PERFORMANCE_CHECK
//}}


