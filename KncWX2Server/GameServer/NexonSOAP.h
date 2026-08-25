//{{ 2012. 04. 02	±è¹Î¼º		³Ø½¼ auth soap
#ifdef SERV_NEXON_AUTH_SOAP

#pragma once

#include "../Common/SOAP/soapH.h" // or whatever it is called, you must already have it


#include "Event.h"
#include "CommonPacket.h"
#include "SystemPacket.h"
#include "ClientPacket.h"
#include "ServerPacket.h"

#include "SubThread.h"
#include "KncException.h"


class KNexonSOAP :public KSubThread
{
	struct KWebMethod
	{
		int m_iSoapAction;
		std::string m_strMethodPath;

		KWebMethod()
		{
			m_iSoapAction = 0;
			m_strMethodPath = std::string();
			m_strMethodPath = "";
		};

		KWebMethod( const KWebMethod& t )
		{
			*this = t;
		};

		KWebMethod& operator=( const KWebMethod& t )
		{
			m_iSoapAction = t.m_iSoapAction;
			m_strMethodPath = t.m_strMethodPath;

			return *this;
		};
	};

#ifdef SERV_NEXON_COUPON_SYSTEM// ÀÛ¾÷³¯Â¥: 2013-06-17	// ¹Ú¼¼ÈÆ
	static	const	std::string	m_strCouponSystemServiceCode;
#endif // SERV_NEXON_COUPON_SYSTEM

	_DeclareException;


public:
#   undef _ENUM
#   define _ENUM( id ) id,
	enum
	{
#   include "NexonSOAP_def.h"
	};

	KNexonSOAP(std::string WsdlFile);
	virtual ~KNexonSOAP();



public:

#ifdef SERV_NEXON_COUPON_SYSTEM// ÀÛ¾÷³¯Â¥: 2013-06-17	// ¹Ú¼¼ÈÆ
	enum NEXON_SERVICE_CODE
	{
		NSC_ELSWORD		= 5059,
	};
#endif // SERV_NEXON_COUPON_SYSTEM

	enum NEXON_GAME_CODE
	{
		NGC_ELSWORD		= 94224,
	};

	virtual void Run();	// Soap Initialize ¸¦ ³Ö±â À§ÇØ ÀçÁ¤ÀÇÇÔ //

	void SoapInitialize();
	void SoapUninitialize();

	static void AddWebMethod( int iSoapAction, std::string szMethod );
	bool GetWebMethod( int iSoapAction, KWebMethod& kInfo );


	//----------------------//
	// Process Event handle //
	//----------------------//
	// derived from KSubThread
	virtual void ProcessEvent( const KEventPtr& spEvent );

	// event handler

	//{{ 2011. 08. 10    ±è¹Î¼º    Ã¤³Î¸µ SOAP
#ifdef SERV_CHANNELING_SOAP
	DECL_ON_FUNC( SOAP_GET_TOONILAND_ID_REQ );
#endif SERV_CHANNELING_SOAP
	//}}

	DECL_ON_FUNC( SOAP_CHECK_NEXON_ID_REQ );

#ifdef	SERV_LOCAL_RANKING_SYSTEM // Àû¿ë³¯Â¥: 2013-03-27
	DECL_ON_FUNC( SOAP_LOCAL_RANKING_NEXON_ID_CHECK_FOR_READ_REQ );
	DECL_ON_FUNC( SOAP_LOCAL_RANKING_NEXON_ID_CHECK_FOR_WRITE_REQ );
#endif	// SERV_LOCAL_RANKING_SYSTEM

	//{{ 2013. 03. 21	 ÃßÃµÀÎ ½Ã½ºÅÛ °³Æí - ±è¹Î¼º
#ifdef SERV_RECOMMEND_LIST_EVENT
	DECL_ON_FUNC( SOAP_CHECK_SAME_USER_REQ );
#endif SERV_RECOMMEND_LIST_EVENT
	//}

#ifdef SERV_NEXON_COUPON_SYSTEM// ÀÛ¾÷³¯Â¥: 2013-06-17	// ¹Ú¼¼ÈÆ
	DECL_ON_FUNC( SOAP_COUPON_LIST_REQ );
	DECL_ON_FUNC( SOAP_REGIST_COUPON_REQ );
	DECL_ON_FUNC( SOAP_COUPON_TARGET_ITEM_REQ );
#endif // SERV_NEXON_COUPON_SYSTEM

protected:
	struct soap* p_nx_soap;

	std::string m_strWsdlPath;

	static std::map< int, KWebMethod >  ms_mapWebMethodPath;
};

#endif SERV_NEXON_AUTH_SOAP
//}}