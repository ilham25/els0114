#pragma warning( disable : 4800 )
#include <Winsock2.h>   
#include "SimLayer.h"
#include "NetLayer.h"
#include "BaseServer.h"
#include "ActorManager.h"
#include <KncLua.h>
#include "Thread/Locker.h"
#include <sstream>  // std::ostringstream
#include <boost/bind.hpp>
#include "KAutoPath.h"
//{{ 2013. 02. 15	������	�α� �ý��� ����
#ifdef SERV_LOG_SYSTEM_NEW
	#include "LogManager.h"
#endif SERV_LOG_SYSTEM_NEW
//}}

ImplInstanceKeeping( KSimLayer );
NiImplementRootRTTI( KSimLayer );
//{{ Iruha : 2026-08-27 // VS2010 port: ImplementException(x) is a real macro (KncException.h),
// but this file never includes that header and KSimLayer never uses DeclareException, so this
// call was never actually defined here. VC7.1 silently parsed it as an implicit-int prototype
// (harmless, unused); VC10 makes that a hard error (C4430). Same class of issue as
// ImplementDBThread above.
//ImplementException( KSimLayer );
//}}

KSimLayer::KSimLayer( ) 
:
//{{ 2012. 05. 10	������	���� ����
//#ifdef SERV_VERSION_FLAG
	m_dwVersionFlag( VF_NONE ),
//#endif SERV_VERSION_FLAG
//}}
    m_dwCommonFlag( CF_NONE ),
    m_dwAuthFlag( AF_INTERNAL ),
    m_dwBillingFlag( BF_NONE )
#ifdef SERV_USE_NATION_FLAG
	,m_dwNationFlag( NF_NONE )
#endif SERV_USE_NATION_FLAG
#ifdef SERV_KOG_OTP_VERIFY
	,m_bUseKogOTP( false )
#endif // SERV_KOG_OTP_VERIFY
#ifdef SERV_COUNTRY_TH
	,m_wstrAuthID( L"" )
	,m_wstrAuthPassword( L"" )
	,m_wstrDomain( L"" )
#endif // SERV_COUNTRY_
#ifdef SERV_GLOBAL_BILLING
	,m_dwPublisherBillingConnectType( 0 )
	,m_bCheckCouponByPublisher( false )
#endif // SERV_GLOBAL_BILLING
{	
}

KSimLayer::~KSimLayer()
{
	//{{ 2010. 8. 30	������	���� ���� ���� ó��	
	// ���������� shutdownȣ���ϰ� �ִµ� ���⼭ �� ȣ���ϰ� �ֳ� -�� - ; �ּ�ó����.
    //std::cout << "~KSimLayer" << std::endl;
    //ShutDown();
	//}}
}

void KSimLayer::Init()
{
	//{{ 2013. 02. 15	������	�α� �ý��� ����
#ifdef SERV_LOG_SYSTEM_NEW
	{
		// ��� ������ �������� ��ũ��Ʈ �Ľ��� �ؾ� �ϹǷ� ���⿡�ٰ� �־���.
		KLogManager::RegScriptName( "LogConfig.lua" );
		OPEN_SCRIPT_FILE( KLogManager );
	}
#endif SERV_LOG_SYSTEM_NEW
	//}}

    // run thread manager.
    std::for_each( m_vecpThreadMgr.begin(), m_vecpThreadMgr.end(), boost::bind( &KThreadManager::BeginThread, _1 ) );
}

ImplToStringW( KSimLayer )
{
	//{{ 2010. 8. 29	������	�̺�Ʈ ť ������ ����
#ifdef SERV_MAX_QUEUE_SIZE_DUMP
	KActorManager::GetKObj()->ToString( stm_ );
#endif SERV_MAX_QUEUE_SIZE_DUMP
	//}}

	//{{ 2013. 02. 13	������	Tick�����ս� DB�α� ���
#ifdef SERV_SIMLAYER_TICK_PERFORMANCE_DB_LOG
	return START_TOSTRINGW
		<< TOSTRINGW( KActorManager::GetKObj()->GetCount() )
		<< TOSTRINGW( GetAuthFlag() )
		<< TOSTRINGW( GetBillingFlag() )
		<< TOSTRINGW( GetSimLayerMaxTick() )
		<< TOSTRINGW( GetSimLayerMaxTickRegDate() );
#else
	return START_TOSTRINGW
		<< TOSTRINGW( KActorManager::GetKObj()->GetCount() )
		<< TOSTRINGW( GetAuthFlag() )
		<< TOSTRINGW( GetBillingFlag() );
#endif SERV_SIMLAYER_TICK_PERFORMANCE_DB_LOG
	//}}    
}

void KSimLayer::Run()
{
    START_LOG( clog, L"Enter Run()" );

    DWORD ret;

    while( true )
    {
        ret = ::WaitForSingleObject( m_hKillEvent, 1 );

        if( ret == WAIT_OBJECT_0 ) break;

        if( ret == WAIT_TIMEOUT ) Tick();

        else    START_LOG( cout, L"*** WaitForSingleObject() - return : " << ret );
    }
}

void KSimLayer::Tick()
{
	//{{ 2011. 12. 14	������	SimLayer Tick�����ս� üũ
#ifdef SERV_SIMLAYER_TICK_PERFORMANCE_CHECK
	BEGIN_CHECK_TICK_LATENCY;
#endif SERV_SIMLAYER_TICK_PERFORMANCE_CHECK
	//}}

    KActorManager::GetKObj()->Tick();

    KBaseServer::GetKObj()->Tick();

	//{{ 2013. 02. 	������	Tick�����ս� DB�α� ���
#ifdef SERV_SIMLAYER_TICK_PERFORMANCE_DB_LOG
	m_kTickChecker.CheckSimLayerTick();
#endif SERV_SIMLAYER_TICK_PERFORMANCE_DB_LOG
	//}}

	//{{ 2011. 12. 14	������	SimLayer Tick�����ս� üũ
#ifdef SERV_SIMLAYER_TICK_PERFORMANCE_CHECK
	END_CHECK_TICK_LATENCY;
#endif SERV_SIMLAYER_TICK_PERFORMANCE_CHECK
	//}}
}

void KSimLayer::ShutDown()
{
    End();
	
	// ������ �Ŵ��� ���� ó��
    std::for_each( m_vecpThreadMgr.begin(), m_vecpThreadMgr.end(), boost::bind( &KThreadManager::EndThread, _1, 10000 ) );	

	// SimLayer�� �Ҹ��Ҷ� ����� ������ �ִ� ThreadManager�鵵 ������!
	m_vecpThreadMgr.clear();
	//}}

    KActorManager::ReleaseKObj();
}

//{{ 2009. 7. 13  ������	�ϵ��ڵ� ����
void KSimLayer::AddPath( const char* pStrPath )
{
	KAutoPath::AddPath( std::string( pStrPath ) );
}
//}}

//{{ 2009. 12. 15  ������	��������
void KSimLayer::DumpToLogFile()
{
	std::wstringstream wstrDump;
	ToString( wstrDump );
	dbg::cout << wstrDump.str() << dbg::endl;
}
//}}

#include <lua_tinker.h>

void KSimLayer::RegToLua()
{
	lua_tinker::class_add<KSimLayer>( g_pLua, "KSimLayer" );
	//{{ 2012. 05. 10	������	���� ����
	//#ifdef SERV_VERSION_FLAG
	lua_tinker::class_def<KSimLayer>( g_pLua, "GetVersionFlag",		&KSimLayer::GetVersionFlag );
	lua_tinker::class_def<KSimLayer>( g_pLua, "SetVersionFlag",		&KSimLayer::SetVersionFlag );
	//#endif SERV_VERSION_FLAG
	//}}
	lua_tinker::class_def<KSimLayer>( g_pLua, "AddCommonFlag",		&KSimLayer::AddCommonFlag );
	lua_tinker::class_def<KSimLayer>( g_pLua, "DeleteCommonFlag",	&KSimLayer::DeleteCommonFlag );
	lua_tinker::class_def<KSimLayer>( g_pLua, "CheckCommonFlag",	&KSimLayer::CheckCommonFlag );
	lua_tinker::class_def<KSimLayer>( g_pLua, "GetAuthFlag",		&KSimLayer::GetAuthFlag );
	lua_tinker::class_def<KSimLayer>( g_pLua, "SetAuthFlag",		&KSimLayer::SetAuthFlag );
#ifdef SERV_COUNTRY_TH
	lua_tinker::class_def<KSimLayer>( g_pLua, "SetAuthAccount",		&KSimLayer::SetAuthAccount );
#endif // SERV_COUNTRY_TH
	lua_tinker::class_def<KSimLayer>( g_pLua, "GetBillingFlag",		&KSimLayer::GetBillingFlag );
	lua_tinker::class_def<KSimLayer>( g_pLua, "SetBillingFlag",		&KSimLayer::SetBillingFlag );
#ifdef SERV_COUNTRY_JP
	lua_tinker::class_def<KSimLayer>( g_pLua, "SetHanInitFlag",		&KSimLayer::SetHanInitFlag );
#endif //SERV_COUNTRY_JP

	//{{ 2009. 7. 13  ������	�ϵ��ڵ� ����
	lua_tinker::class_def<KSimLayer>( g_pLua, "AddPath",			&KSimLayer::AddPath );
	//}}	
	//{{ 2011.11.17 ��ȫ�� ���� ���� ����ȭ
#ifdef SERV_USE_NATION_FLAG
	lua_tinker::class_def<KSimLayer>( g_pLua, "GetNationFlag",		&KSimLayer::GetDwNationFlag );
	lua_tinker::class_def<KSimLayer>( g_pLua, "SetNationFlag",		&KSimLayer::SetDwNationFlag );

	lua_tinker::decl( g_pLua, "NF_NONE",				KSimLayer::NF_NONE );
	lua_tinker::decl( g_pLua, "NF_DE",					KSimLayer::NF_DE );
	lua_tinker::decl( g_pLua, "NF_FR",					KSimLayer::NF_FR );
	lua_tinker::decl( g_pLua, "NF_IT",					KSimLayer::NF_IT );
	lua_tinker::decl( g_pLua, "NF_PL",					KSimLayer::NF_PL );
	lua_tinker::decl( g_pLua, "NF_ES",					KSimLayer::NF_ES );
	lua_tinker::decl( g_pLua, "NF_TR",					KSimLayer::NF_TR );
	lua_tinker::decl( g_pLua, "NF_EN",					KSimLayer::NF_EN );

	lua_tinker::decl( g_pLua, "NF_KR",					KSimLayer::NF_KR );
	lua_tinker::decl( g_pLua, "NF_JP",					KSimLayer::NF_JP );
	lua_tinker::decl( g_pLua, "NF_TW",					KSimLayer::NF_TW );
	lua_tinker::decl( g_pLua, "NF_HK",					KSimLayer::NF_HK );
	lua_tinker::decl( g_pLua, "NF_US",					KSimLayer::NF_US );
	lua_tinker::decl( g_pLua, "NF_CN",					KSimLayer::NF_CN );
	lua_tinker::decl( g_pLua, "NF_VN",					KSimLayer::NF_VN );
	lua_tinker::decl( g_pLua, "NF_TH",					KSimLayer::NF_TH );
	lua_tinker::decl( g_pLua, "NF_ID",					KSimLayer::NF_ID );
	lua_tinker::decl( g_pLua, "NF_BR",					KSimLayer::NF_BR );
	lua_tinker::decl( g_pLua, "NF_PH",					KSimLayer::NF_PH );
#endif SERV_USE_NATION_FLAG
	//}} 2011.11.17 ��ȫ�� ���� ���� ����ȭ

#ifdef SERV_KOG_OTP_VERIFY
	lua_tinker::class_def<KSimLayer>( g_pLua, "SetUseKogOTP",		&KSimLayer::SetUseKogOTP );
#endif // SERV_KOG_OTP_VERIFY

#ifdef SERV_GLOBAL_BILLING
	lua_tinker::class_def<KSimLayer>( g_pLua, "SetPublisherBillingConnectType",		&KSimLayer::SetPublisherBillingConnectType );

	lua_tinker::decl( g_pLua, "PBCT_NONE",				KSimLayer::PBCT_NONE );
	lua_tinker::decl( g_pLua, "PBCT_NORMAL",			KSimLayer::PBCT_NORMAL );
	lua_tinker::decl( g_pLua, "PBCT_DB",				KSimLayer::PBCT_DB );
	lua_tinker::decl( g_pLua, "PBCT_UNUSUAL",			KSimLayer::PBCT_UNUSUAL );

	lua_tinker::class_def<KSimLayer>( g_pLua, "SetCheckCouponByPublisher",		&KSimLayer::SetCheckCouponByPublisher );
#endif // SERV_GLOBAL_BILLING

	//{{ 2012. 05. 10	������	���� ����
	//#ifdef SERV_VERSION_FLAG
	lua_tinker::decl( g_pLua, "VF_INTERNAL",			KSimLayer::VF_INTERNAL );
	lua_tinker::decl( g_pLua, "VF_OPEN_TEST",			KSimLayer::VF_OPEN_TEST );
	lua_tinker::decl( g_pLua, "VF_SERVICE",				KSimLayer::VF_SERVICE );
	//#endif SERV_VERSION_FLAG
	//}}
	lua_tinker::decl( g_pLua, "CF_NPGG",				KSimLayer::CF_NPGG );
	lua_tinker::decl( g_pLua, "CF_CHECK_IP",			KSimLayer::CF_CHECK_IP );
	//{{ 2009. 8. 18  ������	hack shield
	lua_tinker::decl( g_pLua, "CF_HSHIELD",				KSimLayer::CF_HSHIELD );
	//}}
#ifdef SERV_USE_XTRAP
	lua_tinker::decl( g_pLua, "CF_XTRAP",				KSimLayer::CF_XTRAP );
#endif SERV_USE_XTRAP

	lua_tinker::decl( g_pLua, "AF_INTERNAL",			KSimLayer::AF_INTERNAL );
	lua_tinker::decl( g_pLua, "AF_NEXON_KOREA",			KSimLayer::AF_NEXON_KOREA );
    lua_tinker::decl( g_pLua, "AF_GLOBAL_SERVICE",      KSimLayer::AF_GLOBAL_SERVICE );
	lua_tinker::decl( g_pLua, "BF_INTERNAL",			KSimLayer::BF_INTERNAL );
	lua_tinker::decl( g_pLua, "BF_NEXON_KOREA_TEST",	KSimLayer::BF_NEXON_KOREA_TEST );
	lua_tinker::decl( g_pLua, "BF_NEXON_KOREA",			KSimLayer::BF_NEXON_KOREA );
	lua_tinker::decl( g_pLua, "BF_GLOBAL_SERVICE",		KSimLayer::BF_GLOBAL_SERVICE );

	
#ifdef SERV_COUNTRY_JP
	lua_tinker::decl( g_pLua, "HIF_ALPHA",				KSimLayer::HIF_ALPHA );
	lua_tinker::decl( g_pLua, "HIF_REAL",				KSimLayer::HIF_REAL );
#endif //SERV_COUNTRY_JP
}

void KSimLayer::DeleteCommonFlag( DWORD dwFlag )
{
	KLocker lock( m_csCommonFlag );
	
	m_dwCommonFlag &= ~dwFlag;

	START_LOG( cout, L"Delete Common Flag Success : " << dwFlag );
}


