#include "ActorManager.h"
#include "BRPayletterBilling.h"

#ifdef SERV_COUNTRY_BR

#include "KncSend.h"
#include "BRPayletterBillingManager.h"
#include "NetError.h"

#include "BillManager.h"



#define CLASS_TYPE  KBRPayletterBilling

KBRPayletterBilling::KBRPayletterBilling( void )
{
	int a = 0;
	a = 1;
}

KBRPayletterBilling::~KBRPayletterBilling( void )
{
	End( 15000 );
}

void KBRPayletterBilling::ProcessEvent( const KEventPtr& spEvent_ )
{
	DWORD dwElapTime = ::GetTickCount();
	

	//KSerializer ks;
	//ks.BeginReading( &spEvent_->m_kbuff );

	switch( spEvent_->m_usEventID )
	{
		CASE( EPL_BR_HEALTH_CHECK_ACK );
		
		CASE( EPUBLISHER_BILLING_BALANCE_REQ );
		CASE( EPL_BR_BALANCE_CHECK_ACK );

		CASE( EBILL_BUY_PRODUCT_REQ );
		CASE( EPL_BR_PURCHASEITEM_ACK );

		CASE( EBILL_GIFT_ITEM_REQ );
		CASE( EPL_BR_GIFTITEM_ACK );
		
		CASE( EBILL_USE_COUPON_REQ );
		CASE( EPL_BR_REGCOUPON_ACK );
		
	default:
		START_LOG( cerr, L"이벤트 핸들러가 정의되지 않았음. " << spEvent_->GetIDStr() );
	}

	dwElapTime = ::GetTickCount() - dwElapTime;
	if( dwElapTime > 3000 )
	{
		START_LOG( cwarn, L"이벤트 처리 소요 시간." )
			<< BUILD_LOG( spEvent_->GetIDStr() )
			<< BUILD_LOG( spEvent_->m_usEventID )
			<< BUILD_LOG( dwElapTime );
	}

	return;
}


void KBRPayletterBilling::SendToServer( unsigned short usEventID )
{
	SendToServer( usEventID, char() );
}

void KBRPayletterBilling::SendToUser( UidType nTo, unsigned short usEventID )
{
	SendToUser( nTo, usEventID, char() );
}

void KBRPayletterBilling::SendToKOGBillingDB( UidType nUserUID, unsigned short usEventID )
{
	SendToKOGBillingDB( nUserUID, usEventID, char() );
}


IMPL_ON_FUNC( EPL_BR_HEALTH_CHECK_ACK )
{
	// Payletter 빌링이 정상인가?
	bool bSucceed = ( kPacket_.m_usRetCode == 0 );

	SiKBRPayletterBillingManager()->HandleInitializeAck( bSucceed );

	LOG_SUCCESS( bSucceed )
		<< BUILD_LOG( kPacket_.m_usRetCode )
		<< END_LOG;
}


IMPL_ON_FUNC( EPUBLISHER_BILLING_BALANCE_REQ )
{
	//{{ 2008. 8. 13  최육사	초기화 되지 않았다면 날리지 않는다.
	if( !SiKBRPayletterBillingManager()->IsInitialized() ) 
	{
		KEPUBLISHER_BILLING_BALANCE_ACK kPacketAck;
		kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_28;// 빌링 서버가 초기화 되지 않았습니다.	
		kPacketAck.m_ulBalance = 0;
		SendToUser( FIRST_SENDER_UID, EPUBLISHER_BILLING_BALANCE_ACK, kPacketAck );
		return;
	}
	//}}
	

	KEPL_BR_BALANCE_CHECK_REQ kPacket;
	kPacket.m_usReqLen = sizeof(KEPL_BR_BALANCE_CHECK_REQ);
	kPacket.m_ulReqKey = SiKBRPayletterBillingManager()->GetNextPacketNo();
	kPacket.m_usReqType = KBRPayletterBillingPacket::BALANCE_CHECK;

	//kPacket.m_ulUserNo = (unsigned long)GetUID();
	kPacket.m_ulUserNo = _wtoi(kPacket_.m_wstrAccount.c_str());

	kPacket.m_wstrUserID = kPacket_.m_wstrAccount;

	
	SiKBRPayletterBillingManager()->InsertPacketNoUserUID( kPacket.m_ulReqKey, FIRST_SENDER_UID );

	boost::shared_ptr< KBRPayletterBillingPacket > spPacket( new KBRPayletterBillingPacket );
	spPacket->Write( kPacket );
	SiKBRPayletterBillingManager()->QueueingSendPacket( spPacket );
}


IMPL_ON_FUNC( EPL_BR_BALANCE_CHECK_ACK )
{
	bool bSucceed = ( kPacket_.m_usRetCode == 0 );

	UidType iUserUID = SiKBRPayletterBillingManager()->GetCorrespondingUserUID( kPacket_.m_ulReqKey );
	SiKBRPayletterBillingManager()->DeletePacketNoUserUID( kPacket_.m_ulReqKey );

	if( iUserUID <= 0 )
	{
		START_LOG( cerr, L"패킷 번호에 해당하는 UserUID를 찾지 못함." )
			<< BUILD_LOG( kPacket_.m_ulReqKey )
			<< END_LOG;

		return;
	}

	KEPUBLISHER_BILLING_BALANCE_ACK kPacketAck;
#ifdef SERV_SUPPORT_SEVERAL_CASH_TYPES
	// 캐시 종류를 하나로 보는곳은 일반적으로 퍼블캐쉬로 본다.
	kPacketAck.m_iCashType = KGlobalCashInfo::GCT_PUBLISHER_CASH;
#endif //SERV_SUPPORT_SEVERAL_CASH_TYPES

	switch( kPacket_.m_usRetCode )
	{
		// Success
	case 0:	kPacketAck.m_iOK = NetError::NET_OK;	break;

		// Fail
	case 5001:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_01;	break;
	case 5002:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_02;	break;
	case 5011:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_03;	break;
	case 5012:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_04;	break;
	case 5013:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_05;	break;
	case 5014:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_06;	break;
	case 5017:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_07;	break;
	case 5018:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_08;	break;
	case 5021:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_09;	break;
	case 5041:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_10;	break;
	case 5049:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_11;	break;
	case 5050:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_12;	break;
	case 5051:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_13;	break;
	case 5052:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_14;	break;
	case 5053:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_15;	break;
	case 5054:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_16;	break;
	case 5055:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_17;	break;
	case 5056:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_18;	break;
	case 5057:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_19;	break;
	case 5060:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_20;	break;
	case 5070:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_21;	break;
	case 5550:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_22;	break;
	case 5558:	kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_23;	break;
	default:	
		kPacketAck.m_iOK = NetError::ERR_NX_SHOP_04;	
		break;
	}

	if(kPacketAck.m_iOK == NetError::NET_OK)
	{
		START_LOG( clog2, L"잔액조회 성공" )						
			<< END_LOG;	
	}
	else
	{
		START_LOG( cerr, L"잔액조회 실패" )			
			<< BUILD_LOG(  kPacket_.m_usRetCode )
			<< BUILD_LOG( kPacket_.m_wstrRetMsg )
			<< END_LOG;	
	}

	// 캐쉬 값을 넣자
	kPacketAck.m_ulBalance = kPacket_.m_ulRealCash + kPacket_.m_ulBonusCash;
	SendToUser( iUserUID, EPUBLISHER_BILLING_BALANCE_ACK, kPacketAck );
}


IMPL_ON_FUNC( EBILL_BUY_PRODUCT_REQ )
{
	//{{ 2008. 8. 13  최육사	초기화 되지 않았다면 날리지 않는다.
	if( !SiKBRPayletterBillingManager()->IsInitialized() )
	{
		KEBILL_BUY_PRODUCT_ACK kPacketAck;
		kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_28;// 빌링 서버가 초기화 되지 않았습니다.
		SendToUser( FIRST_SENDER_UID, EBILL_BUY_PRODUCT_ACK, kPacketAck );

		return;
	}
	//}}

	//SiKBRPayletterBillingManager()->InsertPacketNoUserUID( kPacketReq.m_ulReqKey, FIRST_SENDER_UID );

	// 응답오면 사용하기 위해서 패킷을 저장시켜 둔다.
	if( SiKBRPayletterBillingManager()->InsertBuyPacket(kPacket_) == false )
	{
		KEBILL_BUY_PRODUCT_ACK kPacketAck;
		kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_29;// 빌링 서버가 초기화 되지 않았습니다.
		SendToUser( FIRST_SENDER_UID, EBILL_BUY_PRODUCT_ACK, kPacketAck );

		return;
	}

	boost::shared_ptr< KBRPayletterBillingPacket > spPacket( new KBRPayletterBillingPacket );
	spPacket->Write( kPacket_ );
	SiKBRPayletterBillingManager()->QueueingSendPacket( spPacket );
}

IMPL_ON_FUNC( EPL_BR_PURCHASEITEM_ACK )
{
	if( kPacket_.m_usRetCode == NetError::NET_OK )
	{
		KEBILL_BUY_PRODUCT_REQ kPacket;
		if( SiKBRPayletterBillingManager()->GetBuyPacket(kPacket_.m_iUserUID, kPacket ) == true )
		{
			SendToKOGBillingDB( kPacket_.m_iUserUID, EBILL_BUY_PRODUCT_REQ, kPacket);
		}
		else
		{
			START_LOG( cerr, L"[빌링] 구매 성공 패킷이 왔는데 보관중인 패킷이 없다?!, 이러면 차감만 되고 아이템 못받으니 서둘러 고치자" )
				<< BUILD_LOG( kPacket_.m_iUserUID )
				<< BUILD_LOG( kPacket_.m_wstrOrderID )
				<< END_LOG;

			KEBILL_BUY_PRODUCT_ACK kPacketAck;
			kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_30;// 빌링 서버가 초기화 되지 않았습니다.
			SendToUser( kPacket_.m_iUserUID, EBILL_BUY_PRODUCT_ACK, kPacketAck );
		}		
	}
	else
	{
		// 에러 패킷 처리는 아래 함수에서 한다
		_ProcessErrorPacket( kPacket_ );
	}
}

IMPL_ON_FUNC( EBILL_GIFT_ITEM_REQ )
{
	//{{ 2008. 8. 13  최육사	초기화 되지 않았다면 날리지 않는다.
	if( !SiKBRPayletterBillingManager()->IsInitialized() )
	{
		KEBILL_GIFT_ITEM_ACK kPacketAck;
		kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_28;// 빌링 서버가 초기화 되지 않았습니다.
		SendToUser( kPacket_.m_iSenderUserUID, EBILL_GIFT_ITEM_ACK, kPacketAck );
			
		return;
	}
	//}}

	if( SiKBRPayletterBillingManager()->InsertGiftPacket(kPacket_) == false )
	{
		KEBILL_GIFT_ITEM_ACK kPacketAck;
		kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_29;// 구매 처리 중입니다. 잠시 후에 다시 시도해 주세요.
		SendToUser( kPacket_.m_iSenderUserUID, EBILL_GIFT_ITEM_ACK, kPacketAck );

		return;
	}

	boost::shared_ptr< KBRPayletterBillingPacket > spPacket( new KBRPayletterBillingPacket );
	spPacket->Write( kPacket_ );
	SiKBRPayletterBillingManager()->QueueingSendPacket( spPacket );
}


IMPL_ON_FUNC( EPL_BR_GIFTITEM_ACK )
{	
	if( kPacket_.m_usRetCode == NetError::NET_OK )
	{
		KEBILL_GIFT_ITEM_REQ kPacket;
		
		if( SiKBRPayletterBillingManager()->GetGiftPacket( kPacket_.m_iUserUID, kPacket ) == true )
		{
			SendToKOGBillingDB( kPacket_.m_iUserUID, EBILL_GIFT_ITEM_REQ, kPacket );

			START_LOG( clog, L"구매성공" )
				<< BUILD_LOG( kPacket_.m_usRetCode )
				<< BUILD_LOG( kPacket_.m_wstrRetMsg )
				<< BUILD_LOG( FIRST_SENDER_UID )
				<< BUILD_LOG( LAST_SENDER_UID )
				<< BUILD_LOG( kPacket_.m_iUserUID )
				<< END_LOG;
		}
		else
		{
			START_LOG( cerr, L"[빌링] 선물 성공 패킷이 왔는데 보관중인 패킷이 없다?!, 이러면 차감만 되고 아이템 못받으니 서둘러 고치자" )
				<< BUILD_LOG( kPacket_.m_iUserUID )
				<< BUILD_LOG( kPacket_.m_wstrOrderID )
				<< END_LOG;

			KEBILL_GIFT_ITEM_ACK kPacketAck;
			kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_30;// 상품 구매중 문제가 발생하였습니다.

			SendToUser( kPacket_.m_iUserUID, EBILL_GIFT_ITEM_ACK, kPacketAck );
		}
	}
	else
	{
		// 에러 패킷 처리는 아래 함수에서 한다
		_ProcessErrorPacket( kPacket_ );
	}
}


IMPL_ON_FUNC( EBILL_USE_COUPON_REQ )
{
	//boost::shared_ptr< KBRPayletterBillingPacket > spaaaa = NULL;
	//spaaaa->Write(kPacket_);

	//{{ 2008. 8. 13  최육사	초기화 되지 않았다면 날리지 않는다.
	if( !SiKBRPayletterBillingManager()->IsInitialized() )
	{
		KEBILL_USE_COUPON_ACK kPacketAck;
		kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_28;// 빌링 서버가 초기화 되지 않았습니다.
		SendToUser( FIRST_SENDER_UID, EBILL_USE_COUPON_ACK, kPacketAck );

		return;
	}
	//}}


	// 응답오면 사용하기 위해서 패킷을 저장시켜 둔다.
	if( SiKBRPayletterBillingManager()->InsertCouponPacket(kPacket_) == false )
	{
		KEBILL_USE_COUPON_ACK kPacketAck;
		kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_29;// 빌링 서버가 초기화 되지 않았습니다.
		SendToUser( FIRST_SENDER_UID, EBILL_USE_COUPON_ACK, kPacketAck );

		return;
	}

	boost::shared_ptr< KBRPayletterBillingPacket > spPacket( new KBRPayletterBillingPacket );	
	spPacket->Write( kPacket_ );
	SiKBRPayletterBillingManager()->QueueingSendPacket( spPacket );

}

IMPL_ON_FUNC( EPL_BR_REGCOUPON_ACK )
{
	if( kPacket_.m_usRetCode == NetError::NET_OK )
	{
		KEBILL_USE_COUPON_REQ kPacket;
		if( SiKBRPayletterBillingManager()->GetCouponPacket( kPacket_.m_iUserUID, kPacket ) == true )
		{
			std::vector< KBillCouponItemInfo > vecCouponItemInfo;
		

			std::vector< KBillBuyInfo >::iterator vit;
			for( vit = kPacket_.m_vecBillBuyInfo.begin(); vit != kPacket_.m_vecBillBuyInfo.end(); vit++ )
			{	
				KBillCouponItemInfo kInfo;
				kInfo.m_iProductNO = vit->m_iProductNo;
				kInfo.m_iCount = vit->m_usOrderQuantity;

				vecCouponItemInfo.push_back(kInfo);
			}
			
			if (vecCouponItemInfo.size() < 1)
			{
				KEBILL_USE_COUPON_ACK kPacketAck;
				kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_30;// 상품 구매중 문제가 발생하였습니다.

				SendToUser( kPacket_.m_iUserUID, EBILL_USE_COUPON_ACK, kPacketAck );
			}


			KEBILL_USE_COUPON_RESULT_REQ kPacketReq;
			kPacketReq.m_iRet = NetError::NET_OK;
			kPacketReq.m_wstrTransaction = kPacket.m_wstrTransaction;
			kPacketReq.m_iUserUID = kPacket.m_PurchaserInfo.m_iUserUID;
			kPacketReq.m_iGSUID = KBaseServer::GetKObj()->GetUID();	// 
			kPacketReq.m_iChargeItem = vecCouponItemInfo[0].m_iProductNO;
			kPacketReq.m_iChargeItemCnt = vecCouponItemInfo[0].m_iCount;	// 카운트도 받고는 있지만 의미없음 1개 밖에 안됨
			kPacketReq.m_iChargeCash = 0;	// 중국 제외한 나라는 캐시 부분 0
			kPacketReq.m_iChargeBonus = 0;	// 중국 제외한 나라는 캐시 부분 0
			SendToKOGBillingDB( kPacketReq.m_iUserUID, EBILL_USE_COUPON_RESULT_REQ, kPacketReq );					  

			if( vecCouponItemInfo.size() > 1 )
			{
				for(int i=1; i < vecCouponItemInfo.size(); i++ )
				{
					// 1 보다 큰 거 다 에러 처리. 아이템 지급 안됨
					START_LOG( cerr, L"하나의 쿠폰에 왜 2개 이상의 Product가 있음. 퍼블리셔 세팅 오류" )
						<< BUILD_LOG( kPacketReq.m_iUserUID )
						<< BUILD_LOG( kPacketReq.m_iGSUID )
						<< BUILD_LOG( vecCouponItemInfo[i].m_iProductNO )
						<< BUILD_LOG( vecCouponItemInfo[i].m_iCount )
						<< END_LOG;
				}
			}


			START_LOG( clog, L"쿠폰 사용 성공" )
				<< BUILD_LOG( kPacket_.m_usRetCode )
				<< BUILD_LOG( kPacket_.m_wstrRetMsg )
				<< BUILD_LOG( FIRST_SENDER_UID )
				<< BUILD_LOG( LAST_SENDER_UID )
				<< BUILD_LOG( kPacket_.m_iUserUID )
				<< END_LOG;
		}
		else
		{
			START_LOG( cerr, L"[빌링] 쿠폰 사용 성공 패킷이 왔는데 보관중인 패킷이 없다?!, 이러면 쿠폰사용만 되고 아이템 못받으니 서둘러 고치자" )
				<< BUILD_LOG( kPacket_.m_iUserUID )
				<< BUILD_LOG( kPacket_.m_wstrRetMsg )
				<< END_LOG;

			KEBILL_USE_COUPON_ACK kPacketAck;
			kPacketAck.m_iOK = NetError::ERR_PAYLETTER_BILLING_30;// 상품 구매중 문제가 발생하였습니다.

			SendToUser( kPacket_.m_iUserUID, EBILL_USE_COUPON_ACK, kPacketAck );
		}
	}
	else
	{
		// 에러 패킷 처리는 아래 함수에서 한다
		_ProcessErrorPacket( kPacket_ );
	}

}


void KBRPayletterBilling::_ProcessErrorPacket( IN const KEPL_BR_PURCHASEITEM_ACK kPacket_ )
{
	START_LOG( cerr, L"[빌링] 구매오류" )
		<< BUILD_LOG( kPacket_.m_usRetCode )
		<< BUILD_LOG( kPacket_.m_wstrRetMsg )
		<< BUILD_LOG( kPacket_.m_wstrUserID.c_str() )
		<< BUILD_LOG( kPacket_.m_bResurrectionStone )
		<< END_LOG;

	int iOK = _GetErrorPacketType( kPacket_.m_usRetCode );

	if( iOK != NetError::NET_OK )
	{
		SiKBRPayletterBillingManager()->EraseBuyPacket(kPacket_.m_iUserUID);

		KEBILL_BUY_PRODUCT_ACK kPacketAck;
		kPacketAck.m_iOK = iOK;// 빌링 서버가 초기화 되지 않았습니다.
		SendToUser( kPacket_.m_iUserUID, EBILL_BUY_PRODUCT_ACK, kPacketAck );
	}
	else
	{
		// 오류 처리 왔는데 리턴 값이 정상이다?
		START_LOG( cerr, L"[빌링] 패킷 오류 처리 하는데, 넘어온 값이 오류값이 아니다? 있을 수 없는 에러" )
			<< END_LOG;
	}
}

void KBRPayletterBilling::_ProcessErrorPacket( IN const KEPL_BR_REGCOUPON_ACK kPacket_ )
{
	START_LOG( cerr, L"[빌링] 쿠폰 사용 오류" )
		<< BUILD_LOG( kPacket_.m_usRetCode )
		<< BUILD_LOG( kPacket_.m_wstrRetMsg )
		<< END_LOG;

	SiKBRPayletterBillingManager()->EraseCouponPacket(kPacket_.m_iUserUID);

	int iOK = _GetErrorPacketType( kPacket_.m_usRetCode );

	if( iOK != NetError::NET_OK )
	{
		KEBILL_USE_COUPON_ACK kPacket;
		kPacket.m_iOK = iOK;
		SendToUser( kPacket_.m_iUserUID, EBILL_USE_COUPON_ACK, kPacket );
	}
	else
	{
		// 오류 처리 왔는데 리턴 값이 정상이다?
		START_LOG( cerr, L"[빌링] 패킷 오류 처리 하는데, 넘어온 값이 오류값이 아니다? 있을 수 없는 에러" )
			<< END_LOG;
	}
}

void KBRPayletterBilling::_ProcessErrorPacket( IN const KEPL_BR_GIFTITEM_ACK kPacket_ )
{
	START_LOG( cerr, L"[빌링] 선물 오류" )
		<< BUILD_LOG( kPacket_.m_usRetCode )
		<< BUILD_LOG( kPacket_.m_wstrRetMsg )
		<< BUILD_LOG( kPacket_.m_wstrOrderID )
		<< BUILD_LOG( kPacket_.m_iUserUID )
		<< END_LOG;

	SiKBRPayletterBillingManager()->EraseGiftPacket(kPacket_.m_iUserUID);

	int iOK = _GetErrorPacketType( kPacket_.m_usRetCode );
	
	if( iOK != NetError::NET_OK )
	{
		KEBILL_GIFT_ITEM_ACK kPacketAck;
		kPacketAck.m_iOK = iOK;
		
		SendToUser( kPacket_.m_iUserUID, EBILL_GIFT_ITEM_ACK, kPacketAck );
	}
	else
	{
		// 오류 처리 왔는데 리턴 값이 정상이다?
		START_LOG( cerr, L"[빌링] 패킷 오류 처리 하는데, 넘어온 값이 오류값이 아니다? 있을 수 없는 에러" )
			<< END_LOG;
	}
}

int KBRPayletterBilling::_GetErrorPacketType( IN const unsigned short usRetCode_ )
{
	switch( usRetCode_ )
	{
		// 에러 처리 하는데 정상 패킷이 들어왔다!?
	case 0:	break;

	case 5001:		
		return NetError::ERR_PAYLETTER_BILLING_01;
	case 5002:
		return NetError::ERR_PAYLETTER_BILLING_02;
	case 5005:
		return NetError::ERR_PAYLETTER_BILLING_24;
	case 5007:
		return NetError::ERR_PAYLETTER_BILLING_25;
	case 5008:
		return NetError::ERR_PAYLETTER_BILLING_27;
	case 5011:
		return NetError::ERR_PAYLETTER_BILLING_03;
	case 5012:
		return NetError::ERR_PAYLETTER_BILLING_04;
	case 5013:
		return NetError::ERR_PAYLETTER_BILLING_05;
	case 5014:
		return NetError::ERR_PAYLETTER_BILLING_06;
	case 5017:
		return NetError::ERR_PAYLETTER_BILLING_07;
	case 5018:
		return NetError::ERR_PAYLETTER_BILLING_08;
	case 5021:
		return NetError::ERR_PAYLETTER_BILLING_09;
	case 5041:
		return NetError::ERR_PAYLETTER_BILLING_10;
	case 5049:
		return NetError::ERR_PAYLETTER_BILLING_11;
	case 5051:
		return NetError::ERR_PAYLETTER_BILLING_13;
	case 5052:
		return NetError::ERR_PAYLETTER_BILLING_14;
	case 5053:
		return NetError::ERR_PAYLETTER_BILLING_15;
	case 5054:
		return NetError::ERR_PAYLETTER_BILLING_16;
	case 5055:
		return NetError::ERR_PAYLETTER_BILLING_17;
	case 5056:
		return NetError::ERR_PAYLETTER_BILLING_18;
	case 5057:
		return NetError::ERR_PAYLETTER_BILLING_19;
	case 5060:
		return NetError::ERR_PAYLETTER_BILLING_20;
	case 5550:
		return NetError::ERR_PAYLETTER_BILLING_22;//ERR_PAYLETTER_BILLING_12
	case 5558:
	case 5094:
		return NetError::ERR_PAYLETTER_BILLING_23;
	case 5570:
		return NetError::ERR_PAYLETTER_BILLING_26;//ERR_PAYLETTER_BILLING_21	
	default:
		return NetError::ERR_BUY_CASH_ITEM_01;
	}

	START_LOG( cerr, L"[빌링] 패킷 오류 처리 하는데, 넘어온 값이 오류값이 아니다? 있을 수 없는 에러" )
		<< BUILD_LOG( usRetCode_ )
		<< END_LOG;

	return NetError::NET_OK;
}

#endif //SERV_COUNTRY_BR