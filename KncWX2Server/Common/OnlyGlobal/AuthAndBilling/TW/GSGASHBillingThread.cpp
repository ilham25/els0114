#include <winsock2.h>
#include <iomanip>
#include <sstream>

#include <dbg/dbg.hpp>

#include "GSGASHBillingThread.h"

#ifdef SERV_COUNTRY_TWHK

#include "GameServer.h"
#include "NetError.h"

//2010.11.26   ��ȿ��    SQL_Injection ���� �ذ� 
#include <boost/algorithm/string/replace.hpp>

//{{ Iruha : 2026-08-27 // VS2010 port: ImplementDBThread is undefined everywhere in this tree.
// VC7.1 silently parsed it as an implicit-int prototype (harmless, unused); VC10 makes that
// a hard error (C4430). DeclareDBThread already implements the constructor inline, so this
// pairing macro never did anything. Most sibling *DBThread.cpp files already comment it out.
//ImplementDBThread( KGSGASHBillingThread );
//}}
ImplPfID( KGSGASHBillingThread, PI_GS_PUBLISHER_BILLING_DB );

#define CLASS_TYPE KGSGASHBillingThread

KGSGASHBillingThread::~KGSGASHBillingThread()
{
}

void KGSGASHBillingThread::ProcessEvent( const KEventPtr& spEvent_ )
{
    KSerializer ks;
    ks.BeginReading( &spEvent_->m_kbuff );

    switch( spEvent_->m_usEventID )
    {
    CASE( EPUBLISHER_BILLING_BALANCE_REQ );
    CASE( EBILL_USE_COUPON_REQ );
    CASE( EBILL_BUY_PRODUCT_REQ );
	CASE( EBILL_GIFT_ITEM_REQ );

	CASE( EPUBLISHER_REPORT_HACK_USER_NOT );

#ifdef SERV_GOLD_TICKET
	CASE( EPUBLISHER_CHARGE_POINT_REQ );
#endif //SERV_GOLD_TICKET
#ifdef SERV_BILLING_ADD_GASH_POINT
	CASE( EGASH_USE_GASH_LOG_NOT );
#endif //SERV_BILLING_ADD_GASH_POINT
    default:
        START_LOG( cerr, L"�̺�Ʈ �ڵ鷯�� ���ǵ��� �ʾ���. " << spEvent_->GetIDStr() );
    }
}

void KGSGASHBillingThread::SendToServer( unsigned short usEventID )
{
    SendToServer( usEventID, char() );
}

void KGSGASHBillingThread::SendToUser( UidType nTo, unsigned short usEventID )
{
    SendToUser( nTo, usEventID, char() );
}

void KGSGASHBillingThread::SendToKOGBillingDB( UidType nUserUID, unsigned short usEventID )
{
    SendToKOGBillingDB( nUserUID, usEventID, char() );
}

void KGSGASHBillingThread::SendToLogDB( unsigned short usEventID )
{
	SendToLogDB( usEventID, char() );
}

int KGSGASHBillingThread::GetCashPoint( const std::wstring& wstrUserID, int& iCashPoint )
{
    int iRet = NetError::ERR_ODBC_01;
    DO_QUERY_NO_PROFILE( L"exec dbo.GetGamePoint", L"\'%s\'", % wstrUserID );

    int iReturnValue;
    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( iCashPoint
            >> iReturnValue );

        m_kODBC.EndFetch();
    }
    else
    {
        goto end_proc;
    }

    switch( iReturnValue )
    {
    case 1:
        iRet = NetError::NET_OK;
        break;
    case 9:
        iRet = NetError::ERR_GASH_00;
        break;
    default:
        break;
    }

end_proc:
    LOG_SUCCESS( iRet == NetError::NET_OK )
        << BUILD_LOG( iRet )
        << BUILD_LOG( NetError::GetErrStr( iRet ) )
        << BUILD_LOG( wstrUserID )
        << BUILD_LOG( iCashPoint )
        << BUILD_LOGc( iReturnValue )
        << END_LOG;

    return iRet;
}

int KGSGASHBillingThread::UseCoupon( IN const int iServerGroupID, IN const std::wstring& wstrUserID, IN const std::wstring& wstrNickName, IN const std::wstring& wstrCardPassword, 
									 IN const std::wstring& wstrIP, OUT int& iEventID, OUT int& iGiftID )
{
    int iRet = NetError::ERR_ODBC_01;
    DO_QUERY_NO_PROFILE( L"exec dbo.sp_ConvertCenter_Exchange", L"%d, N\'%s\', N\'%s\', N\'%s\', N\'%s\'",
        % iServerGroupID % wstrUserID % wstrNickName % wstrCardPassword % wstrIP );

    int iReturnValue;
    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( iReturnValue
            >> iEventID
            >> iGiftID );

        m_kODBC.EndFetch();
    }
    else
    {
        goto end_proc;
    }

    switch( iReturnValue )
    {
    case 1:
        iRet = NetError::NET_OK;
        break;
    case 2:
        iRet = NetError::ERR_GASH_01;
        break;
    case 4:
        iRet = NetError::ERR_GASH_02;
        break;
    case 8:
        iRet = NetError::ERR_GASH_03;
        break;
	case 10:
		iRet = NetError::ERR_GASH_15;
		break;
    case 12:
        iRet = NetError::ERR_GASH_04;
        break;
    case 13:
        iRet = NetError::ERR_GASH_05;
        break;
    case 14:
        iRet = NetError::ERR_GASH_06;
        break;
    case 33:
        iRet = NetError::ERR_GASH_07;
        break;
    case 99:
        iRet = NetError::ERR_GASH_08;
        break;
    default:
        break;
    }

end_proc:
    LOG_SUCCESS( iRet == NetError::NET_OK )
        << BUILD_LOG( iRet )
        << BUILD_LOG( NetError::GetErrStr( iRet ) )
        << BUILD_LOG( iServerGroupID )
        << BUILD_LOG( wstrUserID )
        << BUILD_LOG( wstrNickName )
        << BUILD_LOG( wstrCardPassword )
        << BUILD_LOG( wstrIP )
        << BUILD_LOG( iReturnValue )
        << BUILD_LOG( iEventID )
        << BUILD_LOG( iGiftID )
        << END_LOG;

    return iRet;
}

int KGSGASHBillingThread::GetCouponItem( IN const int iEventID, IN const int iGiftID,
                  OUT std::vector< KBillCouponItemInfo >& vecGASHCouponItemInfo )
{
    int iRet = NetError::ERR_ODBC_01;
    DO_QUERY_NO_PROFILE( L"exec dbo.spCoupenItemList '%d','%d'", L"", % iEventID % iGiftID );

    while( m_kODBC.Fetch() )
    {
        KBillCouponItemInfo kInfo;

        FETCH_DATA( kInfo.m_iProductNO
            >> kInfo.m_iCount            
            );
		
        vecGASHCouponItemInfo.push_back( kInfo );
    }

    iRet = NetError::NET_OK;

end_proc:
    LOG_SUCCESS( iRet == NetError::NET_OK )
        << BUILD_LOG( iRet )
        << BUILD_LOG( NetError::GetErrStr( iRet ) )
        << BUILD_LOG( iEventID )
        << BUILD_LOG( iGiftID )
        << BUILD_LOG( vecGASHCouponItemInfo.size() )
        << END_LOG;

    return iRet;
}

int KGSGASHBillingThread::InGameLessGPoint( const std::wstring& wstrTransID, int iSvr, const std::wstring& wstrUserID, const std::wstring& wstrNickName, int iPoint,
                                           int iItemID, int iProductID, const std::wstring& wstrMemo )
{
    int iRet = NetError::ERR_ODBC_01;

	std::wstring wstrTemp = wstrMemo;
	boost::replace_all( wstrTemp, L"'",L"_");

	DO_QUERY_NO_PROFILE( L"exec dbo.ELSLessGPoint", L"N\'%s\', N\'%s\', %d, N\'%s\', N\'%s\', %d, %d, %d, N\'%s\'",
		% L"EW" % wstrTransID % iSvr % wstrUserID % wstrNickName % iPoint % iItemID % iProductID % wstrTemp );

    int iGamePoint;
    int iReturnValue;
	
    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( iGamePoint
			>> iReturnValue
            );

        m_kODBC.EndFetch();
    }
    else
    {
        goto end_proc;
    }

    switch( iReturnValue )
    {
    case 1:
        iRet = NetError::NET_OK;
        break;
    case 2:
        iRet = NetError::ERR_GASH_09;
        break;
    case 9:
        iRet = NetError::ERR_GASH_10;
        break;
    default:
        START_LOG( cerr, L"not defined return value" )
            << BUILD_LOG( iReturnValue )
            << END_LOG;
        break;
    }

end_proc:
    LOG_SUCCESS( iRet == NetError::NET_OK )
        << BUILD_LOG( iRet )
        << BUILD_LOG( NetError::GetErrStr( iRet ) )
        << BUILD_LOG( iGamePoint )
        << BUILD_LOG( wstrUserID )
        << BUILD_LOG( wstrNickName )
        << BUILD_LOG( iItemID )
        << BUILD_LOG( iProductID )
        << BUILD_LOG( wstrMemo )
        << END_LOG;

    return iRet;
}

int KGSGASHBillingThread::ReportHackUser( IN UidType iUserUID, IN const std::wstring& wstrUserID, IN UidType iUnitUID, IN const std::wstring& wstrNickName, IN UidType iServerUID, IN const std::string& strIP )
{
	int iRet = NetError::ERR_ODBC_01;
	DO_QUERY_NO_PROFILE( L"exec dbo.sp_KOG_AddSuspend", L"%d, N\'%s\', %d, N\'%s\', %d, N\'%s\'", % iUserUID % wstrUserID % iUnitUID % wstrNickName % iServerUID % strIP.c_str() );

	int iReturnValue;
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iReturnValue );

		m_kODBC.EndFetch();
	}
	else
	{
		goto end_proc;
	}

	START_LOG( clog, L"hack user report" )
		<< BUILD_LOG( iRet )
		<< END_LOG;

end_proc:

	return iRet;
}

IMPL_ON_FUNC( EPUBLISHER_BILLING_BALANCE_REQ )
{
    KEPUBLISHER_BILLING_BALANCE_ACK kPacketAck;
    kPacketAck.m_iOK = NetError::ERR_UNKNOWN;
    kPacketAck.m_ulBalance = 0;
#ifdef SERV_SUPPORT_SEVERAL_CASH_TYPES
	// TW GashDB�� ����Ǿ� �������� ĳ�ø� ���ҵ�� ��ȯ�� �ۺ����� ĳ���� ����.
	kPacketAck.m_iCashType = KGlobalCashInfo::GCT_PUBLISHER_ELSWORD_CASH;
#endif //SERV_SUPPORT_SEVERAL_CASH_TYPES

    int iCashPoint;
    kPacketAck.m_iOK = GetCashPoint( kPacket_.m_wstrAccount, iCashPoint );

    if( kPacketAck.m_iOK == NetError::NET_OK )
    {
        kPacketAck.m_ulBalance = ( unsigned long )iCashPoint;
    }

	SendToUser( FIRST_SENDER_UID, EPUBLISHER_BILLING_BALANCE_ACK, kPacketAck );
}

IMPL_ON_FUNC( EBILL_USE_COUPON_REQ )
{
	int iOK = NetError::ERR_UNKNOWN;
    int iEventID;
    int iGiftID;

    iOK = UseCoupon( kPacket_.m_PurchaserInfo.m_iServerGroupID, kPacket_.m_PurchaserInfo.m_wstrUserName, kPacket_.m_PurchaserInfo.m_wstrUnitNickName, kPacket_.m_wstrSerialCode, kPacket_.m_PurchaserInfo.m_wstrIP, iEventID, iGiftID );

	// �ٲ� ���������� �ϳ��� ������ 2�� �̻��� ��Ī ID �Է� �ϴ°� �Ұ����ϴ�. 
	// �ش� ��� �տ��� �ϳ��� �־��ְ� ���� ó���� �ϰڴ�.

	std::vector< KBillCouponItemInfo > vecCouponItemInfo;
    if( iOK == NetError::NET_OK )
    {
        iOK = GetCouponItem( iEventID, iGiftID, vecCouponItemInfo );
    }
   
	if( iOK == NetError::NET_OK )
	{
		if (vecCouponItemInfo.size() < 1)
		{
			KEBILL_USE_COUPON_ACK kPacketAck;
			kPacketAck.m_iOK = NetError::ERR_UNKNOWN;
			SendToUser( FIRST_SENDER_UID, EBILL_USE_COUPON_ACK, kPacketAck );
			return ;
		}


		KEBILL_USE_COUPON_RESULT_REQ kPacketReq;
		kPacketReq.m_iRet = iOK;
		kPacketReq.m_wstrTransaction = kPacket_.m_wstrTransaction;
		kPacketReq.m_iUserUID = kPacket_.m_PurchaserInfo.m_iUserUID;
		kPacketReq.m_iGSUID = KBaseServer::GetKObj()->GetUID();	// 
		kPacketReq.m_iChargeItem = vecCouponItemInfo[0].m_iProductNO;
		kPacketReq.m_iChargeItemCnt = vecCouponItemInfo[0].m_iCount;	// ī��Ʈ�� �ް��� ������ �ǹ̾��� 1�� �ۿ� �ȵ�
		kPacketReq.m_iChargeCash = 0;	// �߱� ������ ����� ĳ�� �κ� 0
		kPacketReq.m_iChargeBonus = 0;	// �߱� ������ ����� ĳ�� �κ� 0
		SendToKOGBillingDB( kPacketReq.m_iUserUID, EBILL_USE_COUPON_RESULT_REQ, kPacketReq );					  

		if( vecCouponItemInfo.size() > 1 )
		{
			for(int i=1; i < vecCouponItemInfo.size(); i++ )
			{
				// 1 ���� ū �� �� ���� ó��. ������ ���� �ȵ�
				START_LOG( cerr, L"�ϳ��� ������ �� 2�� �̻��� Product�� ����. �ۺ����� ���� ����" )
					<< BUILD_LOG( kPacketReq.m_iUserUID )
					<< BUILD_LOG( kPacketReq.m_iGSUID )
					<< BUILD_LOG( vecCouponItemInfo[i].m_iProductNO )
					<< BUILD_LOG( vecCouponItemInfo[i].m_iCount )
					<< END_LOG;
			}
		}
	}
	else
	{
		KEBILL_USE_COUPON_ACK kPacketAck;
		kPacketAck.m_iOK = iOK;

		SendToUser( FIRST_SENDER_UID, EBILL_USE_COUPON_ACK, kPacketAck );
	}
}

IMPL_ON_FUNC( EBILL_BUY_PRODUCT_REQ )
{
    KEBILL_BUY_PRODUCT_REQ kPacketReq = kPacket_;
    kPacketReq.m_vecBillBuyInfo.clear();
    KEBILL_BUY_PRODUCT_ACK kPacket;
    kPacket.m_iOK = NetError::ERR_UNKNOWN;

    // ���� �ݾ��� ����Ѵ�.
    int iTotalPrice = 0;
    std::vector< KBillBuyInfo >::iterator vit;
    for( vit = kPacket_.m_vecBillBuyInfo.begin(); vit != kPacket_.m_vecBillBuyInfo.end(); vit++ )
    {
        iTotalPrice += vit->m_iPoint;
    }

    // �ܾ��� Ȯ���Ѵ�.
    int iBalance;
    kPacket.m_iOK = GetCashPoint( kPacket_.m_wstrUserID, iBalance );
    if( kPacket.m_iOK != NetError::NET_OK )
    {
        SendToUser( FIRST_SENDER_UID, EBILL_BUY_PRODUCT_ACK, kPacket );
        return;
    }

    if( iBalance < iTotalPrice )
    {
        kPacket.m_iOK = NetError::ERR_GASH_09;
        SendToUser( FIRST_SENDER_UID, EBILL_BUY_PRODUCT_ACK, kPacket );
        return;
    }

    // ����Ʈ�� �ִ� �����ۿ� ���� ���ʷ� ����Ʈ�� �����Ѵ�.
    for( vit = kPacket_.m_vecBillBuyInfo.begin(); vit != kPacket_.m_vecBillBuyInfo.end(); vit++ )
    {
        wchar_t wszNumber[32];
        ::_itow( vit->m_iProductID, wszNumber, 10 );
        std::wstring wstrItemInfo = wszNumber;
        ::_itow( vit->m_iCategory, wszNumber, 10 );
        std::wstring wstrItemType = wszNumber;
        int iRet = InGameLessGPoint( kPacket_.m_wstrTotalOrderID, kPacket_.m_iServerGroupID, kPacket_.m_wstrUserID, kPacket_.m_wstrNickName, vit->m_iPoint, vit->m_iProductID, vit->m_iProductNo, vit->m_wstrMemo );
        if( iRet != NetError::NET_OK )
        {
            START_LOG( cerr, L"���� ����" )
                << BUILD_LOG( iRet )
                << BUILD_LOG( NetError::GetErrStr( iRet ) )
                << BUILD_LOG( kPacket_.m_wstrTotalOrderID )
                << BUILD_LOG( kPacket_.m_wstrUserID )
                << BUILD_LOG( kPacket_.m_wstrNickName )
                << BUILD_LOG( vit->m_iProductNo )
                << BUILD_LOG( vit->m_iProductID )
                << BUILD_LOG( vit->m_iPoint )
                << BUILD_LOG( vit->m_wstrMemo );

            continue;
        }
        else
        {
            kPacketReq.m_vecBillBuyInfo.push_back( *vit );
        }
    }

    SendToKOGBillingDB( FIRST_SENDER_UID, EBILL_BUY_PRODUCT_REQ, kPacketReq );
}

IMPL_ON_FUNC( EBILL_GIFT_ITEM_REQ )
{
	KEBILL_GIFT_ITEM_REQ kPacketReq = kPacket_;
	kPacketReq.m_vecBillBuyInfo.clear();
	KEBILL_GIFT_ITEM_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_UNKNOWN;

	// ���� �ݾ��� ����Ѵ�.
	int iTotalPrice = 0;
	std::vector< KBillBuyInfo >::iterator vit;
	for( vit = kPacket_.m_vecBillBuyInfo.begin(); vit != kPacket_.m_vecBillBuyInfo.end(); vit++ )
	{
		iTotalPrice += vit->m_iPoint;
	}

	// �ܾ��� Ȯ���Ѵ�.
	int iBalance;
	kPacket.m_iOK = GetCashPoint( kPacket_.m_wstrSenderUserID, iBalance );
	if( kPacket.m_iOK != NetError::NET_OK )
	{
		SendToUser( FIRST_SENDER_UID, EBILL_GIFT_ITEM_ACK, kPacket );
		return;
	}

	if( iBalance < iTotalPrice )
	{
		kPacket.m_iOK = NetError::ERR_GASH_09;
		SendToUser( FIRST_SENDER_UID, EBILL_GIFT_ITEM_ACK, kPacket );
		return;
	}

	// ����Ʈ�� �ִ� �����ۿ� ���� ���ʷ� ����Ʈ�� �����Ѵ�.
	for( vit = kPacket_.m_vecBillBuyInfo.begin(); vit != kPacket_.m_vecBillBuyInfo.end(); vit++ )
	{
		wchar_t wszNumber[32];
		::_itow( vit->m_iProductID, wszNumber, 10 );
		std::wstring wstrItemInfo = wszNumber;
		::_itow( vit->m_iCategory, wszNumber, 10 );
		std::wstring wstrItemType = wszNumber;
		int iRet = InGameLessGPoint( kPacket_.m_wstrTotalOrderID, kPacket_.m_iServerGroupID, kPacket_.m_wstrSenderUserID, kPacket_.m_wstrSenderNickName, vit->m_iPoint, vit->m_iProductID, vit->m_iProductNo, vit->m_wstrMemo );
		if( iRet != NetError::NET_OK )
		{
			START_LOG( cerr, L"���� ����" )
				<< BUILD_LOG( iRet )
				<< BUILD_LOG( NetError::GetErrStr( iRet ) )
				<< BUILD_LOG( kPacket_.m_wstrTotalOrderID )
				<< BUILD_LOG( kPacket_.m_wstrSenderUserID )
				<< BUILD_LOG( kPacket_.m_wstrSenderNickName )
				<< BUILD_LOG( kPacket_.m_wstrReceiverUserID )
				<< BUILD_LOG( kPacket_.m_iReceiverUserUID )
				<< BUILD_LOG( kPacket_.m_iReceiverUnitUID )
				<< BUILD_LOG( vit->m_iProductNo )
				<< BUILD_LOG( vit->m_iProductID )
				<< BUILD_LOG( vit->m_iPoint )
				<< BUILD_LOG( vit->m_wstrMemo );

			continue;
		}
		else
		{
			kPacketReq.m_vecBillBuyInfo.push_back( *vit );
		}
	}

#ifdef SERV_SUPPORT_SEVERAL_CASH_TYPES
	kPacketReq.m_iUseCashType = KGlobalCashInfo::GCT_PUBLISHER_CASH;
#endif //SERV_SUPPORT_SEVERAL_CASH_TYPES

	SendToKOGBillingDB( FIRST_SENDER_UID, EBILL_GIFT_ITEM_REQ, kPacketReq );
}


IMPL_ON_FUNC( EPUBLISHER_REPORT_HACK_USER_NOT )
{
	ReportHackUser( kPacket_.m_iUserUID, kPacket_.m_wstrUserID, kPacket_.m_iUnitUID, kPacket_.m_wstrNickName, kPacket_.m_iServerUID, kPacket_.m_strIP );
}

#ifdef SERV_BILLING_ADD_GASH_POINT
int KGSGASHBillingThread::InsertUseGashLog( IN KEGASH_USE_GASH_LOG_NOT kPacket)
{
	int iReturnValue;
	std::wstring wstrErr;

	DO_QUERY_NO_PROFILE( L"exec dbo.ELSLessGashTransLog", L"N\'%s\', %d, N\'%s\', %d, N\'%s\', %d, %d, %d, %d, %d, %d, N\'%s\' , N\'%s\'",
		% kPacket.m_wstrTotalOrderID
		% (BYTE)kPacket.m_iServerGroupID
		% kPacket.m_wstrUserID
		% kPacket.m_iUserUID
		% kPacket.m_wstrNickName
		% kPacket.m_iBeforeCash
		% kPacket.m_iAfterGash
		% kPacket.m_iGashPoint
		% kPacket.m_iItemID
		% kPacket.m_iProductID
		% kPacket.m_iFlag
		% kPacket.m_wstrMemo
		% kPacket.m_wstrUserIP);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iReturnValue);
		m_kODBC.EndFetch();
	}
	else
	{
		goto end_proc;
	}

	switch( iReturnValue )
	{
	case 1:
		break;	
	default:
		break;
	}

end_proc:

	LOG_SUCCESS( iReturnValue == 1 )
		<< BUILD_LOGc( iReturnValue )
		<< END_LOG;

	if(iReturnValue == 1)
		return true;
	else
		return false;
}

IMPL_ON_FUNC( EGASH_USE_GASH_LOG_NOT)
{
	if(!InsertUseGashLog( kPacket_ ))
	{
		START_LOG(cerr, L"GASH �α� ���� : �α� ����µ� �����ߴ�!!")
			<< BUILD_LOG(kPacket_.m_iAfterGash)
			<< BUILD_LOG(kPacket_.m_iBeforeCash)
			<< BUILD_LOG(kPacket_.m_iFlag)
			<< BUILD_LOG(kPacket_.m_iGashPoint)
			<< BUILD_LOG(kPacket_.m_iItemID)
			<< BUILD_LOG(kPacket_.m_iProductID)
			<< BUILD_LOG(kPacket_.m_iServerGroupID)
			<< BUILD_LOG(kPacket_.m_iUserUID)
			<< BUILD_LOG(kPacket_.m_wstrMemo)
			<< BUILD_LOG(kPacket_.m_wstrNickName)
			<< BUILD_LOG(kPacket_.m_wstrTotalOrderID)
			<< BUILD_LOG(kPacket_.m_wstrUserID)
			<< BUILD_LOG(kPacket_.m_wstrUserIP)
			<< END_LOG;
	}
}
#endif SERV_BILLING_ADD_GASH_POINT

#ifdef SERV_GOLD_TICKET
IMPL_ON_FUNC( EPUBLISHER_CHARGE_POINT_REQ )
{
	// ���� ��Ŷ �����
	KEPUBLISHER_CHARGE_POINT_ACK kPacketAck;
	kPacketAck.m_iOK = NetError::ERR_UNKNOWN;
	kPacketAck.m_iChargedCash = 0;

	// ���� �ݾ� Ȯ���Ѵ�.
	// �� �ʿ� ���ڳ�. ������ ID�� �Ѿ���ϱ� ���⼭ ���� ���� �ݾ��� ���� �� ����.
	switch(kPacket_.m_iItemID)
	{
	case 75000700://CXSLItem::SI_GOLD_TICKET_10_ID:
		{
			kPacketAck.m_iChargedCash = 10;
			START_LOG( clog, L"���Ƽ�� ���� �ݾ� ���� �Ϸ�!")
				<< BUILD_LOG( kPacket_.m_iItemID)
				<< BUILD_LOG( kPacketAck.m_iChargedCash)
				<< END_LOG;
		}
		break;
	case 75000701://CXSLItem::SI_GOLD_TICKET_50_ID:
		{
			kPacketAck.m_iChargedCash = 50;
			START_LOG( clog, L"���Ƽ�� ���� �ݾ� ���� �Ϸ�!")
				<< BUILD_LOG( kPacket_.m_iItemID)
				<< BUILD_LOG( kPacketAck.m_iChargedCash)
				<< END_LOG;
		}
		break;
	case 75000702://CXSLItem::SI_GOLD_TICKET_100_ID:
		{
			kPacketAck.m_iChargedCash = 100;
			START_LOG( clog, L"���Ƽ�� ���� �ݾ� ���� �Ϸ�!")
				<< BUILD_LOG( kPacket_.m_iItemID)
				<< BUILD_LOG( kPacketAck.m_iChargedCash)
				<< END_LOG;
		}
		break;
	case 75000703://CXSLItem::SI_GOLD_TICKET_500_ID:
		{
			kPacketAck.m_iChargedCash = 500;
			START_LOG( clog, L"���Ƽ�� ���� �ݾ� ���� �Ϸ�!")
				<< BUILD_LOG( kPacket_.m_iItemID)
				<< BUILD_LOG( kPacketAck.m_iChargedCash)
				<< END_LOG;
		}
		break;
	case 75000704://CXSLItem::SI_GOLD_TICKET_1000_ID:
		{
			kPacketAck.m_iChargedCash = 1000;
			START_LOG( clog, L"���Ƽ�� ���� �ݾ� ���� �Ϸ�!")
				<< BUILD_LOG( kPacket_.m_iItemID)
				<< BUILD_LOG( kPacketAck.m_iChargedCash)
				<< END_LOG;
		}
		break;
	case 75000705://CXSLItem::SI_GOLD_TICKET_2000_ID:
		{
			kPacketAck.m_iChargedCash = 2000;
			START_LOG( clog, L"���Ƽ�� ���� �ݾ� ���� �Ϸ�!")
				<< BUILD_LOG( kPacket_.m_iItemID)
				<< BUILD_LOG( kPacketAck.m_iChargedCash)
				<< END_LOG;
		}
		break;
	default:
		{
			START_LOG( cerr, L"���Ƽ�� ���� �ݾ� ���� ����! �������� �ʴ� ��� Ƽ�� ID��")
				<< BUILD_LOG( kPacket_.m_iItemID)
				<< BUILD_LOG( kPacketAck.m_iChargedCash)
				<< END_LOG;
			goto end_proc;
		}
		break;
	}

	DO_QUERY_NO_PROFILE( L"exec dbo.ELSAddGpoint", L"N\'%s\', %d, %d, %d, %d, %d, N\'%s\'",
		% kPacket_.m_wstrUserID
		% kPacket_.m_iServerGroupID
		% kPacket_.m_iCharUID
		% kPacket_.m_iItemUID
		% kPacket_.m_iItemID
		% kPacketAck.m_iChargedCash						// ���� �ݾ�
		% kPacket_.m_wstrUserIP
		);
		
	int iNowCash = 0;
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacketAck.m_iOK
			>> iNowCash);
		m_kODBC.EndFetch();
	}
	else
	{
		START_LOG(cerr, L"���Ƽ�Ͽ� SP ����. �׷��� SP �������� ���ο��� �α� �����ִϱ� ���⿡�� �� ���� �ʿ� ���� �ʳ�?" )
			<< END_LOG;
		kPacketAck.m_iOK = NetError::ERR_ODBC_01;
		goto end_proc;
	}

end_proc:
	LOG_SUCCESS( kPacketAck.m_iOK == NetError::NET_OK )
		<< BUILD_LOG( kPacketAck.m_iOK )
		<< BUILD_LOG( iNowCash )
		<< BUILD_LOG( kPacketAck.m_iChargedCash )
		<< END_LOG;

	{
		KELOG_CHARGE_POINT_NOT kPacketLog;
		kPacketLog.m_iOK = kPacketAck.m_iOK;
		kPacketLog.m_iChargedCash = kPacketAck.m_iChargedCash;
		kPacketLog.m_iNowCash = iNowCash;
		kPacketLog.m_iItemID = kPacket_.m_iItemID;
		kPacketLog.m_iItemUID = kPacket_.m_iItemUID;
		kPacketLog.m_iCharUID = kPacket_.m_iCharUID;

		SendToLogDB( ELOG_CHARGE_POINT_NOT , kPacketLog);
	}

	SendToUser( FIRST_SENDER_UID, EPUBLISHER_CHARGE_POINT_ACK, kPacketAck );	
}
#endif SERV_GOLD_TICKET

#endif //SERV_COUNTRY_TWHK