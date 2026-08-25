//////////////////////////////////////////////////////////////////////////
//	PayletterBillling.h
//
//	Payletter 빌링 관리 메니져
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <KNCSingleton.h>
#include <Thread/KCSLockType.h>
#include "ThreadManager.h"
#include "BRPayletterBillingPacket.h"

#ifdef SERV_COUNTRY_BR

class KBRPayletterBillingManager : public KThreadManager
{
	DeclDump;
	DeclToStringW;
	DeclPfID;
	DeclareSingleton( KBRPayletterBillingManager );

public:
	KBRPayletterBillingManager();
	virtual ~KBRPayletterBillingManager();

	void RegToLua();

	void InitPayletterBillingInfo(
		const char*     szNexonAuthIP,
		unsigned short  usPort,
		int             iDomain );

	// derived from KThreadManager
	virtual void Init( int nThreadNum );
	virtual KThread* CreateThread();
	virtual void BeginThread();
	virtual void EndThread();
	virtual void Tick();

	bool Connect();

	bool IsConnected();
	void KeepConnection();
	void CheckConnection();

	void HandleInitializeAck( bool bSucceed );

	void OnDisconnectBillingServer( IN const std::wstring& wstrReason );


	void Recv();
	void Send();
	//{{ 2009. 11. 21  최육사	서버간접속구조개선
	void KeepConnectionThread();
	//}}

	//{{ 2009. 11. 21  최육사	서버간접속구조개선	
	bool IsFirstConnectSucc() { return m_bFirstConnectSucc; }
	void FirstConnectSucc() { m_bFirstConnectSucc = true; }
	//}}


	void QueueingSendPacket( const KBRPayletterBillingPacketPtr& spPacket );

	bool GetSendPacket( KBRPayletterBillingPacketPtr& spPacket );
	void MakeEventFromReceivedPacket();
	void DumpBuffer( BYTE* buffer, bool bError );

	void InsertPacketNoUserUID( unsigned long ulPacketNo, UidType iUserUID );
	void DeletePacketNoUserUID( unsigned long ulPacketNo );
	UidType GetCorrespondingUserUID( unsigned long ulPacketNo );

	bool IsInitialized()									{ return m_bInitialized; }

	unsigned long GetNextPacketNo();

	// 넣고 빼는 함수
	bool InsertBuyPacket( IN KEBILL_BUY_PRODUCT_REQ kPacket_ );
	bool GetBuyPacket( IN const UidType iUserUID_, OUT KEBILL_BUY_PRODUCT_REQ& kPacket_ );
	void EraseBuyPacket( IN const UidType iUserUID_ );

	// 넣고 빼는 함수-선물하기
	bool InsertGiftPacket( IN const KEBILL_GIFT_ITEM_REQ kPacket_ );
	bool GetGiftPacket( IN const UidType iUserUID_, OUT KEBILL_GIFT_ITEM_REQ& kPacket_ );
	void EraseGiftPacket( IN const UidType iUserUID_ );

	// 넣고 빼는 함수-쿠폰
	bool InsertCouponPacket( IN KEBILL_USE_COUPON_REQ kPacket_ );
	bool GetCouponPacket( IN const UidType iUserUID_, OUT KEBILL_USE_COUPON_REQ& kPacket_ );
	void EraseCouponPacket( IN const UidType iUserUID_ );


protected:

	struct KPayletterBillingInfo
	{
		std::string         m_strIP;
		unsigned short      m_usPort;
		int                 m_iDomain;
	} m_kPayletterBillingInfo;

	DWORD				m_dwLastHeartBeatTick;											// KeepConnectionThread
	DWORD				m_dwLastConnectionCheckTick;									// KeepConnectionThread
	static DWORD		ms_dwHeartBeatGap;												// KeepConnectionThread
	static DWORD		ms_dwConnectionCheckGap;										// KeepConnectionThread

	SOCKET				m_sock;

	boost::shared_ptr< KTThread< KBRPayletterBillingManager > >    m_spThreadRecv;			// Server Init Thread
	boost::shared_ptr< KTThread< KBRPayletterBillingManager > >    m_spThreadSend;			// Server Init Thread
	//{{ 2009. 11. 21  최육사	서버간접속구조개선
	boost::shared_ptr< KTThread< KBRPayletterBillingManager > >    m_spThreadKeepConnect;	// Server Init Thread
	bool														m_bFirstConnectSucc;	// KeepConnectionThread, SendThread, RecvThread
	//}}

	KCSLOCK_DECLARE( std::queue< KBRPayletterBillingPacketPtr >,	m_kSendQueue );			// SendThread, TCPBillingProcessThread

	bool                                                        m_bInitialized;			// KeepConnectionThread, TCPBillingProcessThread
	boost::shared_ptr< void >                                   m_spEvent;				// KeepConnectionThread, TCPBillingProcessThread

	char                                                        m_cRecvBuffer[MAX_PACKET_SIZE_PLB]; // RecvThread
	unsigned long                                               m_ulRecvCP;				// KeepConnectionThread, RecvThread

	bool														m_bSocketConnect;		// 지헌 : 처음 소켓 연결 됬는지 체크

	KCSLOCK_DECLARE( unsigned long,								m_ulCurrentPacketNo );	// KeepConnectionThread, TCPBillingProcessThread, SimLayerThread
	KCSLOCK_DECLARE( std::map< unsigned long KCOMMA() UidType >, m_mapPacketNoUserUID );// TCPBillingProcessThread

	// 지헌 : 아이템 구매 시 사용될 패킷 넣어두는 곳.
	//		  TCP 통신이라 한큐에 안끝나서, 구매 한 아이템 정보를 저장 해 둘 필요가 있는 듯.
	//		  키는 유저UID 로 사용한다.
	KCSLOCK_DECLARE( std::map<UidType KCOMMA() KEBILL_BUY_PRODUCT_REQ>,	m_mapBuyPacket );
	KCSLOCK_DECLARE( std::map<UidType KCOMMA() KEBILL_GIFT_ITEM_REQ>,	m_mapGiftPacket );
	KCSLOCK_DECLARE( std::map<UidType KCOMMA() KEBILL_USE_COUPON_REQ>,	m_mapCouponPacket );

	typedef std::map<UidType, KEBILL_BUY_PRODUCT_REQ>::iterator MAP_BUYPACKET_ITOR;
	typedef std::map<UidType, KEBILL_GIFT_ITEM_REQ>::iterator MAP_GIFTPACKET_ITOR;
	typedef std::map<UidType, KEBILL_USE_COUPON_REQ>::iterator MAP_COUPONPACKET_ITOR;

    
};

DefSingletonInline( KBRPayletterBillingManager );

#endif SERV_COUNTRY_BR