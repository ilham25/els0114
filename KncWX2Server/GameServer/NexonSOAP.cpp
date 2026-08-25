#include "ActorManager.h"
#include "NexonSOAP.h"

#include "NetError.h"


#include "../Common/SOAP/soapH.h" // or whatever it is called, you must already have it
#include "../Common/SOAP/elswordgameSoap.nsmap" // this is what you have to add to fix the problem
//#include "../Common/SOAP/elswordgameSoap12.nsmap" // this is what you have to add to fix the problem
//#include "../Common/SOAP/soapStub.h" // or whatever it is called, you must already have it

//{{ 2012. 04. 02	김민성		넥슨 auth soap
#ifdef SERV_NEXON_AUTH_SOAP

#define CLASS_TYPE  KNexonSOAP

std::map< int, KNexonSOAP::KWebMethod >  KNexonSOAP::ms_mapWebMethodPath;

#ifdef SERV_NEXON_COUPON_SYSTEM// 작업날짜: 2013-06-17	// 박세훈
const	std::string	KNexonSOAP::m_strCouponSystemServiceCode = "SVG026";
#endif // SERV_NEXON_COUPON_SYSTEM

_ImplementException( KNexonSOAP );



KNexonSOAP::KNexonSOAP(std::string strWsdlPath)
{
	m_strWsdlPath = strWsdlPath;
//	m_WsmlFile = WsmlFile;

	p_nx_soap = NULL;
}

KNexonSOAP::~KNexonSOAP()
{
	End( 1500 );
}

// 2010.10.24 조효진. 
// 현재 GameForgeAuth의 매니저가 등록된 곳이 KChannelSimLayer의 m_vecpThreadMgr 이므로
// 구조상 Lua의 AddWebMethod가 KGameForgeAuth::Run() 보다 빠르게 수행된다. 
void KNexonSOAP::Run()     // derived from KThread
{
	std::cout << "KSubThread::Run(), thread start." << std::endl;
	START_LOG( cout2, L"[Test Log] KGameForgeAuth Start" )
		<< END_LOG;


	srand( (unsigned) time( NULL ) );

	// 추가된 코드 //
	SoapInitialize();
	//


	START_LOG( cout2, L"KNexonSOAP Soap Initialized" )
		<< END_LOG;
	 
	while( true )
	{
		//{{ 2010/11/11 조효진 SOAP 모듈 정상 생성 실패 시 다시 생성을 위해 //
		if( p_nx_soap == NULL ){
			SoapInitialize();
		}
		//}}

		DWORD ret = ::WaitForSingleObject( m_hKillEvent, 1 );

		if( ret == WAIT_OBJECT_0 ) break;

		if( ret == WAIT_TIMEOUT ) Loop();

		else std::cout << "*** WaitForSingleObject() - return :" << ret << std::endl;

	}

	// 추가된 코드 //
	SoapUninitialize();
	//

	START_LOG( cout2, L"KNexonSOAP Soap End" )
		<< END_LOG;
}

 
void KNexonSOAP::SoapInitialize()
{
	p_nx_soap = soap_new();
	if( !p_nx_soap )	//// couldn't allocate: stop
	{
		p_nx_soap = NULL;

		START_LOG( cerr, L"Soap_new Error" )
			<< END_LOG;

		return;
	}
}


void KNexonSOAP::SoapUninitialize()
{
	if( p_nx_soap != NULL )
	{
		soap_destroy(p_nx_soap);
		soap_end(p_nx_soap);
		p_nx_soap = NULL;
	}
}


/////////////////////////////////////////////////////////////////////////////
void KNexonSOAP::AddWebMethod( int iSoapAction, std::string szMethod )
{
	if( iSoapAction < 0 || iSoapAction >= NXSOAP_NUM )
	{
		START_LOG( cerr, L"이상한 SOAP Action." )
			<< BUILD_LOG( iSoapAction )
			<< END_LOG;

		return;
	}


	if( szMethod == "" )
	{
		START_LOG( cerr, L"method 이상." )
			<< BUILD_LOG( iSoapAction )
			<< END_LOG;

		return;
	}

	KWebMethod kMethod;
	kMethod.m_iSoapAction = iSoapAction;
	kMethod.m_strMethodPath = szMethod;

	std::map< int, KWebMethod >::iterator mit = ms_mapWebMethodPath.find( iSoapAction );
	if( mit != ms_mapWebMethodPath.end() )
	{
		START_LOG( cerr, L"이미 존재하는 soap action." )
			<< BUILD_LOG( iSoapAction )
			<< END_LOG;

		return;
	}

	// 무조건 넣는다.
	ms_mapWebMethodPath[iSoapAction] = kMethod;
}

bool KNexonSOAP::GetWebMethod( int iSoapAction, KWebMethod& kInfo )
{
	if( iSoapAction < 0 || iSoapAction >= NXSOAP_NUM )
	{
		START_LOG( cerr, L"이상한 SOAP Action." )
			<< BUILD_LOG( iSoapAction )
			<< END_LOG;

		return false;
	}

	std::map< int, KWebMethod >::iterator mit = ms_mapWebMethodPath.find( iSoapAction );
	if( mit == ms_mapWebMethodPath.end() )
	{
		START_LOG( cerr, L"존재하지 않는 soap action." )
			<< BUILD_LOG( iSoapAction )
			<< END_LOG;

		return false;
	}

	kInfo = mit->second;

	return true;
}


//=========================================================================//
//			Process Event handle			
//=========================================================================//

void KNexonSOAP::ProcessEvent( const KEventPtr& spEvent_ )
{
	DWORD dwElapTime = ::GetTickCount();

	switch( spEvent_->m_usEventID )
	{
	
		//{{ 2011. 08. 10    김민성    채널링 SOAP
#ifdef SERV_CHANNELING_SOAP
		CASE( SOAP_GET_TOONILAND_ID_REQ );
#endif SERV_CHANNELING_SOAP
		//}}

		CASE( SOAP_CHECK_NEXON_ID_REQ );
#ifdef	SERV_LOCAL_RANKING_SYSTEM // 적용날짜: 2013-03-27
		CASE( SOAP_LOCAL_RANKING_NEXON_ID_CHECK_FOR_READ_REQ );
		CASE( SOAP_LOCAL_RANKING_NEXON_ID_CHECK_FOR_WRITE_REQ );
#endif	// SERV_LOCAL_RANKING_SYSTEM

		//{{ 2013. 03. 21	 추천인 시스템 개편 - 김민성
#ifdef SERV_RECOMMEND_LIST_EVENT
		CASE( SOAP_CHECK_SAME_USER_REQ );
#endif SERV_RECOMMEND_LIST_EVENT
		//}

#ifdef SERV_NEXON_COUPON_SYSTEM// 작업날짜: 2013-06-17	// 박세훈
		CASE( SOAP_COUPON_LIST_REQ );
		CASE( SOAP_REGIST_COUPON_REQ );
		CASE( SOAP_COUPON_TARGET_ITEM_REQ );
#endif // SERV_NEXON_COUPON_SYSTEM

	default:
		START_LOG( cerr, "핸들러가 지정되지 않은 이벤트." )
			<< BUILD_LOG( spEvent_->m_usEventID )
			<< BUILD_LOG( spEvent_->GetIDStr() )
			<< END_LOG;
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


//////////////////////////////////////////////////////





//{{ 2011. 08. 10    김민성    채널링 SOAP
#ifdef SERV_CHANNELING_SOAP
IMPL_ON_FUNC( SOAP_GET_TOONILAND_ID_REQ )
{
	if( kPacket_.m_wstrNexonID.empty() == true )
	{
		START_LOG( cerr, L"임시 NexonID가 비었다. 있을 수 없는 상황." )
			<< BUILD_LOG( LAST_SENDER_UID )
			<< BUILD_LOG( kPacket_.m_wstrNexonID )
			<< END_LOG;
		return;
	}

	int iResult = 0;

	const std::string strTemp = KncUtil::toNarrowString( kPacket_.m_wstrNexonID );
	char arrNexonID[100];
	strncpy( arrNexonID, strTemp.c_str(), strTemp.size() );

	_ns1__GetChannelingInfo				ChannelingInfo;
	_ns1__GetChannelingInfoResponse		ChannelingInfoResponse;

#ifdef SERV_NEXON_COUPON_SYSTEM// 작업날짜: 2013-06-17	// 박세훈
	ChannelingInfo.n4ServiceCode = KNexonSOAP::NSC_ELSWORD;
#else // SERV_NEXON_COUPON_SYSTEM
	ChannelingInfo.n4ServiceCode = 5059;
#endif // SERV_NEXON_COUPON_SYSTEM
	ChannelingInfo.strNexonID = arrNexonID;

	iResult = soap_call___ns1__GetChannelingInfo( p_nx_soap, NULL, NULL, &ChannelingInfo, &ChannelingInfoResponse  );

	START_LOG( clog, L"SOAP 함수 결과 보자" )
		<< BUILD_LOG( LAST_SENDER_UID )
		<< BUILD_LOG( kPacket_.m_wstrNexonID )
		<< BUILD_LOG( ChannelingInfo.n4ServiceCode )
		<< BUILD_LOG( ChannelingInfo.strNexonID )
		<< BUILD_LOG( ChannelingInfoResponse.strChannelingUID )
		<< BUILD_LOG( ChannelingInfoResponse.strChannelingID )
		<< BUILD_LOG( iResult )
		<< END_LOG;

	KSOAP_GET_TOONILAND_ID_ACK kPacketAck;
	kPacketAck.m_iOK = iResult;
	kPacketAck.m_wstrChannelingUID = KncUtil::toWideString( ChannelingInfoResponse.strChannelingUID );
	kPacketAck.m_wstrToonilandID = KncUtil::toWideString( ChannelingInfoResponse.strChannelingID );

	KEventPtr spEvent( new KEvent );
	spEvent->SetData( PI_GS_USER, NULL, SOAP_GET_TOONILAND_ID_ACK, kPacketAck );
	KActorManager::GetKObj()->QueueingTo( LAST_SENDER_UID, spEvent );
}
#endif SERV_CHANNELING_SOAP
//}}

IMPL_ON_FUNC( SOAP_CHECK_NEXON_ID_REQ )
{
	if( kPacket_.m_wstrPassword.empty() == true )
	{
		START_LOG( cerr, L"비밀번호가 비었다. 있을 수 없는 상황." )
			<< BUILD_LOG( LAST_SENDER_UID )
			<< BUILD_LOG( kPacket_.m_wstrPassword )
			<< END_LOG;

		KSOAP_CHECK_NEXON_ID_ACK kPacketAck;

		kPacketAck.m_iOK = NetError::ERR_BUY_CASH_ITEM_26;

		KEventPtr spEvent( new KEvent );
		spEvent->SetData( PI_GS_USER, NULL, SOAP_CHECK_NEXON_ID_ACK, kPacketAck );
		KActorManager::GetKObj()->QueueingTo( LAST_SENDER_UID, spEvent );
		return;
	}

	int iResult = 0;

	const std::string strPassword = KncUtil::toNarrowString( kPacket_.m_wstrPassword );
	char arrNexonPW[100] = {0,};
	strncpy( arrNexonPW, strPassword.c_str(), strPassword.size() );

	_ns1__CheckPasswordByMemberSN				kInfo;
	_ns1__CheckPasswordByMemberSNResponse		iResponse;

	iResponse.CheckPasswordByMemberSNResult = -1;
	kInfo.n4ServiceCode = kPacket_.m_iServiceCode;
	kInfo.memberSN = kPacket_.m_iNexonSN;
	kInfo.password = arrNexonPW;

	iResult = soap_call___ns2__CheckPasswordByMemberSN( p_nx_soap, NULL, NULL, &kInfo, &iResponse  );

	START_LOG( clog, L"SOAP 함수 결과 보자" )
		<< BUILD_LOG( LAST_SENDER_UID )
		<< BUILD_LOG( kInfo.memberSN )
		<< BUILD_LOG( kInfo.n4ServiceCode )
		<< BUILD_LOG( iResponse.CheckPasswordByMemberSNResult )
		<< BUILD_LOG( iResult )
		<< END_LOG;

	if( 0 != iResult )
	{
		KSOAP_CHECK_NEXON_ID_ACK kPacketAck;

		kPacketAck.m_iOK = NetError::ERR_BUY_CASH_ITEM_26;

		KEventPtr spEvent( new KEvent );
		spEvent->SetData( PI_GS_USER, NULL, SOAP_CHECK_NEXON_ID_ACK, kPacketAck );
		KActorManager::GetKObj()->QueueingTo( LAST_SENDER_UID, spEvent );

		START_LOG( cerr, L"SOAP 함수 홒출 실패!" )
			<< BUILD_LOG( LAST_SENDER_UID )
			<< BUILD_LOG( kInfo.memberSN )
			<< BUILD_LOG( kInfo.n4ServiceCode )
			<< BUILD_LOG( iResponse.CheckPasswordByMemberSNResult )
			<< BUILD_LOG( iResult )
			<< END_LOG;

		return;
	}

	KSOAP_CHECK_NEXON_ID_ACK kPacketAck;

	kPacketAck.m_iOK = iResponse.CheckPasswordByMemberSNResult;
	kPacketAck.m_wstrLoginID = kPacket_.m_wstrLoginID;
	kPacketAck.m_wstrReceiverNickName = kPacket_.m_wstrReceiverNickName;
	kPacketAck.m_wstrMessage = kPacket_.m_wstrMessage;
	kPacketAck.m_vecPurchaseReqInfo = kPacket_.m_vecPurchaseReqInfo;
#ifdef SERV_NEXON_COUPON_SYSTEM// 작업날짜: 2013-07-29	// 박세훈
	kPacketAck.m_bUseCoupon		= kPacket_.m_bUseCoupon;
#endif // SERV_NEXON_COUPON_SYSTEM

	KEventPtr spEvent( new KEvent );
	spEvent->SetData( PI_GS_USER, NULL, SOAP_CHECK_NEXON_ID_ACK, kPacketAck );
	KActorManager::GetKObj()->QueueingTo( LAST_SENDER_UID, spEvent );
}

#ifdef	SERV_LOCAL_RANKING_SYSTEM // 적용날짜: 2013-03-27
IMPL_ON_FUNC( SOAP_LOCAL_RANKING_NEXON_ID_CHECK_FOR_READ_REQ )
{
	KSOAP_LOCAL_RANKING_NEXON_ID_CHECK_FOR_READ_ACK kPacketAck;
	kPacketAck.m_iOK = NetError::ERR_LOCALRANKING_07;

	if( kPacket_.m_wstrPassword.empty() == true )
	{
		START_LOG( cerr, L"비밀번호가 비었다. 있을 수 없는 상황." )
			<< BUILD_LOG( LAST_SENDER_UID )
			<< BUILD_LOG( kPacket_.m_wstrPassword )
			<< END_LOG;

		KEventPtr spEvent( new KEvent );
		spEvent->SetData( PI_GS_USER, NULL, SOAP_LOCAL_RANKING_NEXON_ID_CHECK_FOR_READ_ACK, kPacketAck );
		KActorManager::GetKObj()->QueueingTo( LAST_SENDER_UID, spEvent );
		return;
	}

	int iResult = 0;

	const std::string strPassword = KncUtil::toNarrowString( kPacket_.m_wstrPassword );
	char arrNexonPW[100] = {0,};
	strncpy( arrNexonPW, strPassword.c_str(), strPassword.size() );

	_ns1__CheckPasswordByMemberSN				kInfo;
	_ns1__CheckPasswordByMemberSNResponse		iResponse;

	iResponse.CheckPasswordByMemberSNResult = -1;
#ifdef SERV_NEXON_COUPON_SYSTEM// 작업날짜: 2013-06-17	// 박세훈
	kInfo.n4ServiceCode = KNexonSOAP::NSC_ELSWORD;
#else // SERV_NEXON_COUPON_SYSTEM
	kInfo.n4ServiceCode = 5059;
#endif // SERV_NEXON_COUPON_SYSTEM
	kInfo.memberSN = kPacket_.m_iNexonSN;
	kInfo.password = arrNexonPW;

#ifdef	SERV_LOCAL_RANKING_SYSTEM_INHOUSE// 적용날짜: 사내 전용
	iResult = 0;
#else	// SERV_LOCAL_RANKING_SYSTEM_INHOUSE
	iResult = soap_call___ns2__CheckPasswordByMemberSN( p_nx_soap, NULL, NULL, &kInfo, &iResponse  );
#endif	// SERV_LOCAL_RANKING_SYSTEM_INHOUSE

	START_LOG( clog, L"SOAP 함수 결과 보자" )
		<< BUILD_LOG( LAST_SENDER_UID )
		<< BUILD_LOG( kInfo.memberSN )
		<< BUILD_LOG( kInfo.n4ServiceCode )
		<< BUILD_LOG( iResponse.CheckPasswordByMemberSNResult )
		<< BUILD_LOG( iResult )
		<< END_LOG;

	if( 0 != iResult )
	{
		KEventPtr spEvent( new KEvent );
		spEvent->SetData( PI_GS_USER, NULL, SOAP_LOCAL_RANKING_NEXON_ID_CHECK_FOR_READ_ACK, kPacketAck );
		KActorManager::GetKObj()->QueueingTo( LAST_SENDER_UID, spEvent );

		START_LOG( cerr, L"SOAP 함수 호출 실패!" )
			<< BUILD_LOG( LAST_SENDER_UID )
			<< BUILD_LOG( kInfo.memberSN )
			<< BUILD_LOG( kInfo.n4ServiceCode )
			<< BUILD_LOG( iResponse.CheckPasswordByMemberSNResult )
			<< BUILD_LOG( iResult )
			<< END_LOG;
		return;
	}

#ifdef	SERV_LOCAL_RANKING_SYSTEM_INHOUSE// 적용날짜: 사내 전용
	kPacketAck.m_iOK = NetError::NET_OK;
#else	// SERV_LOCAL_RANKING_SYSTEM_INHOUSE
	kPacketAck.m_iOK = iResponse.CheckPasswordByMemberSNResult;
#endif	// SERV_LOCAL_RANKING_SYSTEM_INHOUSE

	KEventPtr spEvent( new KEvent );
	spEvent->SetData( PI_GS_USER, NULL, SOAP_LOCAL_RANKING_NEXON_ID_CHECK_FOR_READ_ACK, kPacketAck );
	KActorManager::GetKObj()->QueueingTo( LAST_SENDER_UID, spEvent );
}

IMPL_ON_FUNC( SOAP_LOCAL_RANKING_NEXON_ID_CHECK_FOR_WRITE_REQ )
{
	KSOAP_LOCAL_RANKING_NEXON_ID_CHECK_FOR_WRITE_ACK kPacketAck;
	kPacketAck.m_iOK = NetError::ERR_LOCALRANKING_07;

	if( kPacket_.m_wstrPassword.empty() == true )
	{
		START_LOG( cerr, L"비밀번호가 비었다. 있을 수 없는 상황." )
			<< BUILD_LOG( LAST_SENDER_UID )
			<< BUILD_LOG( kPacket_.m_wstrPassword )
			<< END_LOG;

		KEventPtr spEvent( new KEvent );
		spEvent->SetData( PI_GS_USER, NULL, SOAP_LOCAL_RANKING_NEXON_ID_CHECK_FOR_WRITE_ACK, kPacketAck );
		KActorManager::GetKObj()->QueueingTo( LAST_SENDER_UID, spEvent );
		return;
	}

	int iResult = 0;

	const std::string strPassword = KncUtil::toNarrowString( kPacket_.m_wstrPassword );
	char arrNexonPW[100] = {0,};
	strncpy( arrNexonPW, strPassword.c_str(), strPassword.size() );

	_ns1__CheckPasswordByMemberSN				kInfo;
	_ns1__CheckPasswordByMemberSNResponse		iResponse;

	iResponse.CheckPasswordByMemberSNResult = -1;
#ifdef SERV_NEXON_COUPON_SYSTEM// 작업날짜: 2013-06-17	// 박세훈
	kInfo.n4ServiceCode = KNexonSOAP::NSC_ELSWORD;
#else // SERV_NEXON_COUPON_SYSTEM
	kInfo.n4ServiceCode = 5059;
#endif // SERV_NEXON_COUPON_SYSTEM
	kInfo.memberSN = kPacket_.m_iNexonSN;
	kInfo.password = arrNexonPW;

#ifdef	SERV_LOCAL_RANKING_SYSTEM_INHOUSE// 적용날짜: 사내 전용
	iResult = 0;
#else	// SERV_LOCAL_RANKING_SYSTEM_INHOUSE
	iResult = soap_call___ns2__CheckPasswordByMemberSN( p_nx_soap, NULL, NULL, &kInfo, &iResponse  );
#endif	// SERV_LOCAL_RANKING_SYSTEM_INHOUSE

	START_LOG( clog, L"SOAP 함수 결과 보자" )
		<< BUILD_LOG( LAST_SENDER_UID )
		<< BUILD_LOG( kInfo.memberSN )
		<< BUILD_LOG( kInfo.n4ServiceCode )
		<< BUILD_LOG( iResponse.CheckPasswordByMemberSNResult )
		<< BUILD_LOG( iResult )
		<< END_LOG;

	if( 0 != iResult )
	{
		KEventPtr spEvent( new KEvent );
		spEvent->SetData( PI_GS_USER, NULL, SOAP_LOCAL_RANKING_NEXON_ID_CHECK_FOR_WRITE_ACK, kPacketAck );
		KActorManager::GetKObj()->QueueingTo( LAST_SENDER_UID, spEvent );

		START_LOG( cerr, L"SOAP 함수 호출 실패!" )
			<< BUILD_LOG( LAST_SENDER_UID )
			<< BUILD_LOG( kInfo.memberSN )
			<< BUILD_LOG( kInfo.n4ServiceCode )
			<< BUILD_LOG( iResponse.CheckPasswordByMemberSNResult )
			<< BUILD_LOG( iResult )
			<< END_LOG;
		return;
	}

#ifdef	SERV_LOCAL_RANKING_SYSTEM_INHOUSE// 적용날짜: 사내 전용
	kPacketAck.m_iOK	= NetError::NET_OK;
#else	// SERV_LOCAL_RANKING_SYSTEM_INHOUSE
	kPacketAck.m_iOK	= iResponse.CheckPasswordByMemberSNResult;
#endif	// SERV_LOCAL_RANKING_SYSTEM_INHOUSE
	kPacketAck.m_kInfo	= kPacket_.m_kInfo;

	KEventPtr spEvent( new KEvent );
	spEvent->SetData( PI_GS_USER, NULL, SOAP_LOCAL_RANKING_NEXON_ID_CHECK_FOR_WRITE_ACK, kPacketAck );
	KActorManager::GetKObj()->QueueingTo( LAST_SENDER_UID, spEvent );
}
#endif	// SERV_LOCAL_RANKING_SYSTEM

//{{ 2013. 03. 21	 추천인 시스템 개편 - 김민성
#ifdef SERV_RECOMMEND_LIST_EVENT
IMPL_ON_FUNC( SOAP_CHECK_SAME_USER_REQ )
{
	START_LOG( clog, L"SOAP_CHECK_SAME_USER_REQ 리시브 완료" )
		<< END_LOG;

	KSOAP_CHECK_SAME_USER_ACK kPacketAck;

	kPacketAck.m_iOK = NetError::ERR_ODBC_00;
	kPacketAck.m_iUnitUID = kPacket_.m_iUnitUID;
	kPacketAck.m_wstrNickName = kPacket_.m_wstrNickName;

	if( kPacket_.m_iNexonSN_A <= 0 || kPacket_.m_iNexonSN_B <= 0 )
	{
		START_LOG( cerr, L"넥슨 OID 값이 이상합니다." )
			<< BUILD_LOG( LAST_SENDER_UID )
			<< BUILD_LOG( kPacket_.m_iNexonSN_A )
			<< BUILD_LOG( kPacket_.m_iNexonSN_B )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_wstrNickName )
			<< END_LOG;

		kPacketAck.m_iOK = NetError::ERR_ODBC_00;

		KEventPtr spEvent( new KEvent );
		spEvent->SetData( PI_GS_USER, NULL, SOAP_CHECK_SAME_USER_ACK, kPacketAck );
		KActorManager::GetKObj()->QueueingTo( LAST_SENDER_UID, spEvent );
		return;
	}

	int iResult = 0;

	_ns1__CheckSameUser				kInfo;
	_ns1__CheckSameUserResponse		iResponse;

	iResponse.CheckSameUserResult = -1;

	kInfo.n4ServiceCode = kPacket_.m_iServiceCode;
	kInfo.memberSN = kPacket_.m_iNexonSN_A;
	kInfo.compareMemberSN = kPacket_.m_iNexonSN_B;

	iResult = soap_call___ns2__CheckSameUser( p_nx_soap, NULL, NULL, &kInfo, &iResponse  );

	START_LOG( clog, L"SOAP 함수 결과 보자" )
		<< BUILD_LOG( LAST_SENDER_UID )
		<< BUILD_LOG( kInfo.memberSN )
		<< BUILD_LOG( kInfo.compareMemberSN )
		<< BUILD_LOG( kInfo.n4ServiceCode )
		<< BUILD_LOG( iResponse.CheckSameUserResult )
		<< BUILD_LOG( iResult )
		<< END_LOG;

	if( iResult != 0 )
	{
		kPacketAck.m_iOK = -1;

		KEventPtr spEvent( new KEvent );
		spEvent->SetData( PI_GS_USER, NULL, SOAP_CHECK_SAME_USER_ACK, kPacketAck );
		KActorManager::GetKObj()->QueueingTo( LAST_SENDER_UID, spEvent );

		START_LOG( cerr, L"SOAP 함수 홒출 실패!" )
			<< BUILD_LOG( LAST_SENDER_UID )
			<< BUILD_LOG( kInfo.memberSN )
			<< BUILD_LOG( kInfo.compareMemberSN )
			<< BUILD_LOG( kInfo.n4ServiceCode )
			<< BUILD_LOG( iResponse.CheckSameUserResult )
			<< BUILD_LOG( iResult )
			<< END_LOG;

		return;
	}

	kPacketAck.m_iOK = iResponse.CheckSameUserResult;

	KEventPtr spEvent( new KEvent );
	spEvent->SetData( PI_GS_USER, NULL, SOAP_CHECK_SAME_USER_ACK, kPacketAck );
	KActorManager::GetKObj()->QueueingTo( LAST_SENDER_UID, spEvent );
		
	START_LOG( clog, L"SOAP 함수 홒출 성공!" )
		<< BUILD_LOG( LAST_SENDER_UID )
		<< BUILD_LOG( kInfo.memberSN )
		<< BUILD_LOG( kInfo.compareMemberSN )
		<< BUILD_LOG( kInfo.n4ServiceCode )
		<< BUILD_LOG( iResponse.CheckSameUserResult )
		<< BUILD_LOG( iResult )
		<< END_LOG;
}
#endif SERV_RECOMMEND_LIST_EVENT
//}

#ifdef SERV_NEXON_COUPON_SYSTEM// 작업날짜: 2013-06-17	// 박세훈
IMPL_ON_FUNC( SOAP_COUPON_LIST_REQ )
{
	KSOAP_COUPON_LIST_ACK kPacket;
	kPacket.m_iOK				= NetError::NET_OK;
	kPacket.m_bUsed				= kPacket_.m_bUsed;
	kPacket.m_sCouponBoxType	= kPacket_.m_sCouponBoxType;
	kPacket.m_iStartRowIndex	= kPacket_.m_iStartRowIndex;

	//if( kPacket_.m_bUsed == true )
	//{
	//	// 기사용 쿠폰 카운트 얻기
	//	{
	//		_ns1__GetCouponBoxUsedListCountByNexonIDForUser			kInfo;
	//		_ns1__GetCouponBoxUsedListCountByNexonIDForUserResponse	iResponse;

	//		kInfo.nexonID		= kPacket_.m_strNexonID.c_str();
	//		kInfo.serviceCode	= m_strCouponSystemServiceCode.c_str();
	//		kInfo.couponboxType	= kPacket_.m_sCouponBoxType;

	//		iResponse.GetCouponBoxUsedListCountByNexonIDForUserResult = 0;

	//		const int iResult = soap_call___ns2__GetCouponBoxUsedListCountByNexonIDForUser( p_nx_soap, NULL, NULL, kInfo, iResponse );

	//		START_LOG( clog, L"SOAP 함수 결과 보자" )
	//			<< BUILD_LOG( iResult )
	//			<< BUILD_LOG( LAST_SENDER_UID )
	//			<< BUILD_LOG( kInfo.couponboxType )
	//			<< BUILD_LOG( iResponse.GetCouponBoxUsedListCountByNexonIDForUserResult )
	//			<< END_LOG;

	//		if( iResult != 0 )
	//		{
	//			START_LOG( cerr, L"SOAP 함수 호출 실패!" )
	//				<< BUILD_LOG( iResult )
	//				<< BUILD_LOG( LAST_SENDER_UID )
	//				<< BUILD_LOG( kInfo.nexonID )
	//				<< BUILD_LOG( kInfo.serviceCode )
	//				<< BUILD_LOG( kInfo.couponboxType )
	//				<< BUILD_LOG( iResponse.GetCouponBoxUsedListCountByNexonIDForUserResult )
	//				<< END_LOG;
	//			
	//			//kPacket.m_iOK = NetError::??;	// 쿠폰 정보 읽기에 실패하였습니다.
	//			goto end_proc;
	//		}

	//		kPacket.m_iCouponCount = iResponse.GetCouponBoxUsedListCountByNexonIDForUserResult;
	//	}

	//	// 기사용 쿠폰 리스트 얻기
	//	if( kPacket.m_iStartRowIndex < kPacket.m_iCouponCount )	// 가져올 페이지에 해당하는 쿠폰 갯수가 없다면 쿠폰 리스트를 가져올 필요가 없다.
	//	{
	//		_ns1__SelectCouponBoxUsedListByNexonIDForUser			kInfo;
	//		_ns1__SelectCouponBoxUsedListByNexonIDForUserResponse	iResponse;

	//		kInfo.nexonID		= kPacket_.m_strNexonID.c_str();
	//		kInfo.serviceCode	= m_strCouponSystemServiceCode.c_str();
	//		kInfo.startRowIndex	= kPacket_.m_iStartRowIndex;
	//		kInfo.maximumRows	= SEnum::CL_INDEX_PER_PAGE;
	//		kInfo.couponboxType	= kPacket_.m_sCouponBoxType;

	//		iResponse.SelectCouponBoxUsedListByNexonIDForUserResult = NULL;

	//		const int iResult = soap_call___ns2__SelectCouponBoxUsedListByNexonIDForUser( p_nx_soap, NULL, NULL, kInfo, iResponse );

	//		START_LOG( clog, L"SOAP 함수 결과 보자" )
	//			<< BUILD_LOG( iResult )
	//			<< BUILD_LOG( LAST_SENDER_UID )
	//			<< BUILD_LOG( kInfo.startRowIndex )
	//			<< BUILD_LOG( kInfo.maximumRows )
	//			<< BUILD_LOG( kInfo.couponboxType )
	//			<< END_LOG;

	//		if( iResult != 0 )
	//		{
	//			START_LOG( cerr, L"SOAP 함수 호출 실패!" )
	//				<< BUILD_LOG( iResult )
	//				<< BUILD_LOG( LAST_SENDER_UID )
	//				<< BUILD_LOG( kInfo.nexonID )
	//				<< BUILD_LOG( kInfo.serviceCode )
	//				<< BUILD_LOG( kInfo.couponboxType )
	//				<< END_LOG;
	//			
	//			//kPacket.m_iOK = NetError::??;	// 쿠폰 정보 읽기에 실패하였습니다.
	//			goto end_proc;
	//		}

	//		if( ( iResponse.SelectCouponBoxUsedListByNexonIDForUserResult != NULL ) &&
	//			( iResponse.SelectCouponBoxUsedListByNexonIDForUserResult->ClientCouponBox != NULL )
	//			)
	//		{
	//			const int iNum = iResponse.SelectCouponBoxUsedListByNexonIDForUserResult->__sizeClientCouponBox;
	//			
	//			/*
	//			// iResponse.SelectCouponBoxListByNexonIDForUserResult->__sizeClientCouponBox를 사용할 수 없다면 다음과 같이 하자.
	//			int iNum	= kPacket.m_iCouponCount - kPacket.m_iStartRowIndex;
	//			iNum		= min( iNum, SEnum::CL_INDEX_PER_PAGE );
	//			*/

	//			for( int i=0; i < iNum; ++i )
	//			{
	//				const ns1__ClientCouponBox&	ns1ClientCouponBox = iResponse.SelectCouponBoxUsedListByNexonIDForUserResult->ClientCouponBox[i];
	//				KClientCouponBox kClientCouponBox;
	//				kClientCouponBox.m_wstrContractDetailName	= std::wstring( ns1ClientCouponBox.ContractDetailName, ns1ClientCouponBox.ContractDetailName + strlen( ns1ClientCouponBox.ContractDetailName ) );
	//				kClientCouponBox.m_iContractDetailNo		= ns1ClientCouponBox.ContractDetailNo;
	//				kClientCouponBox.m_wstrCouponBoxType		= std::wstring( ns1ClientCouponBox.CouponBoxType, ns1ClientCouponBox.CouponBoxType + strlen( ns1ClientCouponBox.CouponBoxType ) );
	//				kClientCouponBox.m_wstrCouponBoxTypeText	= std::wstring( ns1ClientCouponBox.CouponBoxTypeText, ns1ClientCouponBox.CouponBoxTypeText + strlen( ns1ClientCouponBox.CouponBoxTypeText ) );
	//				kClientCouponBox.m_iCouponCardNo			= ns1ClientCouponBox.CouponCardNo;
	//				kClientCouponBox.m_wstrCouponCardStatus		= std::wstring( ns1ClientCouponBox.CouponCardStatus, ns1ClientCouponBox.CouponCardStatus + strlen( ns1ClientCouponBox.CouponCardStatus ) );
	//				kClientCouponBox.m_wstrCouponCardStatusText	= std::wstring( ns1ClientCouponBox.CouponCardStatusText, ns1ClientCouponBox.CouponCardStatusText + strlen( ns1ClientCouponBox.CouponCardStatusText ) );
	//				kClientCouponBox.m_wstrDiscountValue		= std::wstring( ns1ClientCouponBox.DiscountValue, ns1ClientCouponBox.DiscountValue + strlen( ns1ClientCouponBox.DiscountValue ) );
	//				kClientCouponBox.m_wstrExpireDateTime		= std::wstring( ns1ClientCouponBox.ExpireDateTime, ns1ClientCouponBox.ExpireDateTime + strlen( ns1ClientCouponBox.ExpireDateTime ) );
	//				kClientCouponBox.m_wstrObtainDateTime		= std::wstring( ns1ClientCouponBox.ObtainDateTime, ns1ClientCouponBox.ObtainDateTime + strlen( ns1ClientCouponBox.ObtainDateTime ) );
	//				kClientCouponBox.m_wstrValidDateRange		= std::wstring( ns1ClientCouponBox.ValidDateRange, ns1ClientCouponBox.ValidDateRange + strlen( ns1ClientCouponBox.ValidDateRange ) );
	//				kPacket.m_vecUsedCouponList.push_back( kClientCouponBox );
	//			}
	//		}
	//	}
	//}
	//else
	//{
	//	// 미사용 쿠폰 카운트 얻기
	//	{
	//		_ns1__GetCouponBoxListCountByNexonIDForUser			kInfo;
	//		_ns1__GetCouponBoxListCountByNexonIDForUserResponse	iResponse;

	//		kInfo.nexonID		= kPacket_.m_strNexonID.c_str();
	//		kInfo.serviceCode	= m_strCouponSystemServiceCode.c_str();
	//		kInfo.couponboxType	= kPacket_.m_sCouponBoxType;

	//		iResponse.GetCouponBoxListCountByNexonIDForUserResult = 0;

	//		const int iResult = soap_call___ns2__GetCouponBoxListCountByNexonIDForUser( p_nx_soap, NULL, NULL, kInfo, iResponse );

	//		START_LOG( clog, L"SOAP 함수 결과 보자" )
	//			<< BUILD_LOG( iResult )
	//			<< BUILD_LOG( LAST_SENDER_UID )
	//			<< BUILD_LOG( kInfo.couponboxType )
	//			<< BUILD_LOG( iResponse.GetCouponBoxListCountByNexonIDForUserResult )
	//			<< END_LOG;

	//		if( iResult != 0 )
	//		{
	//			START_LOG( cerr, L"SOAP 함수 호출 실패!" )
	//				<< BUILD_LOG( iResult )
	//				<< BUILD_LOG( LAST_SENDER_UID )
	//				<< BUILD_LOG( kInfo.nexonID )
	//				<< BUILD_LOG( kInfo.serviceCode )
	//				<< BUILD_LOG( kInfo.couponboxType )
	//				<< BUILD_LOG( iResponse.GetCouponBoxListCountByNexonIDForUserResult )
	//				<< END_LOG;
	//			
	//			//kPacket.m_iOK = NetError::??;	// 쿠폰 정보 읽기에 실패하였습니다.
	//			goto end_proc;
	//		}

	//		kPacket.m_iCouponCount = iResponse.GetCouponBoxListCountByNexonIDForUserResult;
	//	}

	//	// 미사용 쿠폰 리스트 얻기
	//	if( kPacket.m_iStartRowIndex < kPacket.m_iCouponCount )	// 가져올 페이지에 해당하는 쿠폰 갯수가 없다면 쿠폰 리스트를 가져올 필요가 없다.
	//	{
	//		_ns1__SelectCouponBoxListByNexonIDForUser			kInfo;
	//		_ns1__SelectCouponBoxListByNexonIDForUserResponse	iResponse;

	//		kInfo.nexonID		= kPacket_.m_strNexonID.c_str();
	//		kInfo.serviceCode	= m_strCouponSystemServiceCode.c_str();
	//		kInfo.startRowIndex	= kPacket_.m_iStartRowIndex;
	//		kInfo.maximumRows	= SEnum::CL_INDEX_PER_PAGE;
	//		kInfo.couponboxType	= kPacket_.m_sCouponBoxType;

	//		iResponse.SelectCouponBoxListByNexonIDForUserResult = NULL;

	//		const int iResult = soap_call___ns2__SelectCouponBoxListByNexonIDForUser( p_nx_soap, NULL, NULL, kInfo, iResponse );

	//		START_LOG( clog, L"SOAP 함수 결과 보자" )
	//			<< BUILD_LOG( iResult )
	//			<< BUILD_LOG( LAST_SENDER_UID )
	//			<< BUILD_LOG( kInfo.startRowIndex )
	//			<< BUILD_LOG( kInfo.maximumRows )
	//			<< BUILD_LOG( kInfo.couponboxType )
	//			<< END_LOG;

	//		if( iResult != 0 )
	//		{
	//			START_LOG( cerr, L"SOAP 함수 호출 실패!" )
	//				<< BUILD_LOG( iResult )
	//				<< BUILD_LOG( LAST_SENDER_UID )
	//				<< BUILD_LOG( kInfo.nexonID )
	//				<< BUILD_LOG( kInfo.serviceCode )
	//				<< BUILD_LOG( kInfo.couponboxType )
	//				<< END_LOG;
	//			
	//			//kPacket.m_iOK = NetError::??;	// 쿠폰 정보 읽기에 실패하였습니다.
	//			goto end_proc;
	//		}

	//		if( ( iResponse.SelectCouponBoxListByNexonIDForUserResult != NULL ) &&
	//			( iResponse.SelectCouponBoxListByNexonIDForUserResult->ClientCouponBox != NULL )
	//			)
	//		{
	//			const int iNum = iResponse.SelectCouponBoxListByNexonIDForUserResult->__sizeClientCouponBox;
	//			
	//			/*
	//			// iResponse.SelectCouponBoxListByNexonIDForUserResult->__sizeClientCouponBox를 사용할 수 없다면 다음과 같이 하자.
	//			int iNum	= kPacket.m_iCouponCount - kPacket.m_iStartRowIndex;
	//			iNum		= min( iNum, SEnum::CL_INDEX_PER_PAGE );
	//			*/

	//			for( int i=0; i < iNum; ++i )
	//			{
	//				const ns1__ClientCouponBox&	ns1ClientCouponBox = iResponse.SelectCouponBoxListByNexonIDForUserResult->ClientCouponBox[i];
	//				KClientCouponBox kClientCouponBox;
	//				kClientCouponBox.m_wstrContractDetailName	= std::wstring( ns1ClientCouponBox.ContractDetailName, ns1ClientCouponBox.ContractDetailName + strlen( ns1ClientCouponBox.ContractDetailName ) );
	//				kClientCouponBox.m_iContractDetailNo		= ns1ClientCouponBox.ContractDetailNo;
	//				kClientCouponBox.m_wstrCouponBoxType		= std::wstring( ns1ClientCouponBox.CouponBoxType, ns1ClientCouponBox.CouponBoxType + strlen( ns1ClientCouponBox.CouponBoxType ) );
	//				kClientCouponBox.m_wstrCouponBoxTypeText	= std::wstring( ns1ClientCouponBox.CouponBoxTypeText, ns1ClientCouponBox.CouponBoxTypeText + strlen( ns1ClientCouponBox.CouponBoxTypeText ) );
	//				kClientCouponBox.m_iCouponCardNo			= ns1ClientCouponBox.CouponCardNo;
	//				kClientCouponBox.m_wstrCouponCardStatus		= std::wstring( ns1ClientCouponBox.CouponCardStatus, ns1ClientCouponBox.CouponCardStatus + strlen( ns1ClientCouponBox.CouponCardStatus ) );
	//				kClientCouponBox.m_wstrCouponCardStatusText	= std::wstring( ns1ClientCouponBox.CouponCardStatusText, ns1ClientCouponBox.CouponCardStatusText + strlen( ns1ClientCouponBox.CouponCardStatusText ) );
	//				kClientCouponBox.m_wstrDiscountValue		= std::wstring( ns1ClientCouponBox.DiscountValue, ns1ClientCouponBox.DiscountValue + strlen( ns1ClientCouponBox.DiscountValue ) );
	//				kClientCouponBox.m_wstrExpireDateTime		= std::wstring( ns1ClientCouponBox.ExpireDateTime, ns1ClientCouponBox.ExpireDateTime + strlen( ns1ClientCouponBox.ExpireDateTime ) );
	//				kClientCouponBox.m_wstrObtainDateTime		= std::wstring( ns1ClientCouponBox.ObtainDateTime, ns1ClientCouponBox.ObtainDateTime + strlen( ns1ClientCouponBox.ObtainDateTime ) );
	//				kClientCouponBox.m_wstrValidDateRange		= std::wstring( ns1ClientCouponBox.ValidDateRange, ns1ClientCouponBox.ValidDateRange + strlen( ns1ClientCouponBox.ValidDateRange ) );
	//				kPacket.m_vecUsedCouponList.push_back( kClientCouponBox );
	//			}
	//		}
	//	}
	//}

end_proc:
	KEventPtr spEvent( new KEvent );
	spEvent->SetData( PI_GS_USER, NULL, SOAP_COUPON_LIST_ACK, kPacket );
	KActorManager::GetKObj()->QueueingTo( LAST_SENDER_UID, spEvent );
}

IMPL_ON_FUNC( SOAP_REGIST_COUPON_REQ )
{
	KEGS_REGIST_COUPON_ACK kPacket;
	kPacket.m_iOK = NetError::NET_OK;

	//_ns1__AddCouponBoxByCouponPinByNexonId			kInfo;
	//_ns1__AddCouponBoxByCouponPinByNexonIdResponse	iResponse;

	//kInfo.nexonID		= kPacket_.m_strNexonID.c_str();
	//kInfo.serviceCode	= m_strCouponSystemServiceCode.c_str();
	//kInfo.couponPin		= kPacket_.m_strCouponPin.c_str();

	//iResponse.AddCouponBoxByCouponPinByNexonIdResult = -1;

	//const int iResult = soap_call___ns2__AddCouponBoxByCouponPinByNexonId( p_nx_soap, NULL, NULL, kInfo, iResponse );

	//START_LOG( clog, L"SOAP 함수 결과 보자" )
	//	<< BUILD_LOG( iResult )
	//	<< BUILD_LOG( LAST_SENDER_UID )
	//	<< BUILD_LOG( kInfo.couponPin )
	//	<< BUILD_LOG( iResponse.AddCouponBoxByCouponPinByNexonIdResult )
	//	<< END_LOG;

	//if( iResult != 0 )
	//{
	//	START_LOG( cerr, L"SOAP 함수 호출 실패!" )
	//		<< BUILD_LOG( iResult )
	//		<< BUILD_LOG( LAST_SENDER_UID )
	//		<< BUILD_LOG( kInfo.nexonID )
	//		<< BUILD_LOG( kInfo.serviceCode )
	//		<< BUILD_LOG( kInfo.couponPin )
	//		<< END_LOG;

	//	//kPacket.m_iOK = NetError::??;	// 쿠폰 등록에 실패하였습니다.
	//	goto end_proc;
	//}

	//switch( iResponse.AddCouponBoxByCouponPinByNexonIdResult )
	//{
	//case 0:
	//	break;

	//case 1:
	//	//kPacket.m_iOK = NetError::??;	// 이미 사용된 쿠폰입니다.
	//	goto end_proc;

	//case 2:
	//	//kPacket.m_iOK = NetError::??;	// 만료된 쿠폰입니다.
	//	goto end_proc;

	//case 4:
	//	//kPacket.m_iOK = NetError::??;	// 취소된 쿠폰입니다.
	//	goto end_proc;

	//case 12:
	//	//kPacket.m_iOK = NetError::??;	// 쿠폰핀 규칙이 맞지 않습니다.
	//	goto end_proc;

	//case 15:
	//	//kPacket.m_iOK = NetError::??;	// 사용 가능일의 종료일이 지났습니다.
	//	goto end_proc:

	//case 31:
	//	//kPacket.m_iOK = NetError::??;	// 사용 제한 수량을 초과하였습니다.
	//	goto end_proc:

	//default:
	//	//kPacket.m_iOK = NetError::??;	// 쿠폰 등록에 실패하였습니다.
	//	goto end_proc:
	//}

end_proc:
	KEventPtr spEvent( new KEvent );
	UidType anTrace[2] = { LAST_SENDER_UID, -1 };
	spEvent->SetData( PI_GS_PROXY, anTrace, EGS_REGIST_COUPON_ACK, kPacket );
	KActorManager::GetKObj()->QueueingTo( LAST_SENDER_UID, spEvent );
}

IMPL_ON_FUNC( SOAP_COUPON_TARGET_ITEM_REQ )

{
	KSOAP_COUPON_TARGET_ITEM_ACK kPacket;
	kPacket.m_iOK				= NetError::NET_OK;
	kPacket.m_iCouponCardNo		= kPacket_.m_iCouponCardNo;
	kPacket.m_iStartRowIndex	= kPacket_.m_iStartRowIndex;

	//// 쿠폰함 리스트에서 선택된 쿠폰에 유효한 아이템 카운트 얻기
	//{
	//	_ns1__GetCouponBoxTargetItemListCount			kInfo;
	//	_ns1__GetCouponBoxTargetItemListCountResponse	iResponse;

	//	kInfo.couponCardNo = kPacket_.m_iCouponCardNo;

	//	iResponse.GetCouponBoxTargetItemListCountResult = 0;

	//	const int iResult = soap_call___ns2__GetCouponBoxTargetItemListCount( p_nx_soap, NULL, NULL, kInfo, iResponse );

	//	START_LOG( clog, L"SOAP 함수 결과 보자" )
	//		<< BUILD_LOG( iResult )
	//		<< BUILD_LOG( LAST_SENDER_UID )
	//		<< BUILD_LOG( kInfo.couponCardNo )
	//		<< BUILD_LOG( iResponse.GetCouponBoxTargetItemListCountResult )
	//		<< END_LOG;

	//	if( iResult != 0 )
	//	{
	//		START_LOG( cerr, L"SOAP 함수 호출 실패!" )
	//			<< BUILD_LOG( iResult )
	//			<< BUILD_LOG( LAST_SENDER_UID )
	//			<< BUILD_LOG( kInfo.couponCardNo )
	//			<< BUILD_LOG( iResponse.GetCouponBoxTargetItemListCountResult )
	//			<< END_LOG;

	//		//kPacket.m_iOK = NetError::??;	// 해당 쿠폰의 할인 적용 가능 아이템 목록 읽기에 실패하였습니다.
	//		goto end_proc;
	//	}

	//	kPacket.m_iTargetItemCount = iResponse.GetCouponBoxTargetItemListCountResult;
	//}

	//// 쿠폰함 리스트에서 선택된 쿠폰에 유효한 아이템 리스트 얻기
	//if( kPacket.m_iStartRowIndex < kPacket.m_iTargetItemCount )	// 가져올 페이지에 해당하는 아이템 정보가 없다면 리스트를 가져올 필요가 없다.
	//{
	//	_ns1__SelectCouponBoxTargetItemList			kInfo;
	//	_ns1__SelectCouponBoxTargetItemListResponse	iResponse;

	//	kInfo.couponCardNo	= kPacket_.m_iCouponCardNo;
	//	kInfo.startRowIndex	= kPacket_.m_iStartRowIndex;
	//	kInfo.maximumRows	= SEnum::CL_INDEX_PER_PAGE;

	//	iResponse.SelectCouponBoxTargetItemListResult = NULL;

	//	const int iResult = soap_call___ns2__SelectCouponBoxTargetItemList( p_nx_soap, NULL, NULL, kInfo, iResponse );

	//	START_LOG( clog, L"SOAP 함수 결과 보자" )
	//		<< BUILD_LOG( iResult )
	//		<< BUILD_LOG( LAST_SENDER_UID )
	//		<< BUILD_LOG( kInfo.couponCardNo )
	//		<< BUILD_LOG( kInfo.startRowIndex )
	//		<< BUILD_LOG( kInfo.maximumRows )
	//		<< END_LOG;

	//	if( iResult != 0 )
	//	{
	//		START_LOG( cerr, L"SOAP 함수 호출 실패!" )
	//			<< BUILD_LOG( iResult )
	//			<< BUILD_LOG( LAST_SENDER_UID )
	//			<< BUILD_LOG( kInfo.couponCardNo )
	//			<< BUILD_LOG( kInfo.startRowIndex )
	//			<< BUILD_LOG( kInfo.maximumRows )
	//			<< END_LOG;

	//		//kPacket.m_iOK = NetError::??;	// 해당 쿠폰의 할인 적용 가능 아이템 목록 읽기에 실패하였습니다.
	//		goto end_proc;
	//	}

	//	if( ( iResponse.SelectCouponBoxTargetItemListResult != NULL ) &&
	//		( iResponse.SelectCouponBoxTargetItemListResult->CouponBoxTargetItem != NULL )
	//		)
	//	{
	//		const int iNum = iResponse.SelectCouponBoxTargetItemListResult->__sizeCouponBoxTargetItem;

	//		/*
	//		// iResponse.SelectCouponBoxTargetItemListResult->__sizeCouponBoxTargetItem를 사용할 수 없다면 다음과 같이 하자.
	//		int iNum	= kPacket.m_iTargetItemCount - kPacket.m_iStartRowIndex;
	//		iNum		= min( iNum, SEnum::CL_INDEX_PER_PAGE );
	//		*/

	//		for( int i=0; i < iNum; ++i )
	//		{
	//			const ns1__CouponBoxTargetItem&	ns1CouponBoxTargetItem = iResponse.SelectCouponBoxTargetItemListResult->CouponBoxTargetItem[i];
	//			KCouponBoxTargetItem kCouponBoxTargetItem;
	//			kCouponBoxTargetItem.m_wstrItemCode	= std::wstring( ns1CouponBoxTargetItem.ItemCode, ns1CouponBoxTargetItem.ItemCode + strlen( ns1CouponBoxTargetItem.ItemCode ) );
	//			kCouponBoxTargetItem.m_wstrItemName	= std::wstring( ns1CouponBoxTargetItem.ItemName, ns1CouponBoxTargetItem.ItemName + strlen( ns1CouponBoxTargetItem.ItemName ) );
	//			kPacket.m_vecTargetItem.push_back( kCouponBoxTargetItem );
	//		}
	//	}
	//}

end_proc:
	KEventPtr spEvent( new KEvent );
	spEvent->SetData( PI_GS_USER, NULL, SOAP_COUPON_TARGET_ITEM_ACK, kPacket );
	KActorManager::GetKObj()->QueueingTo( LAST_SENDER_UID, spEvent );
}
#endif // SERV_NEXON_COUPON_SYSTEM

#endif SERV_NEXON_AUTH_SOAP
//}}