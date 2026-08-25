#include "LevelUpAuthManager.h"

#ifdef SERV_COUNTRY_BR

#include "KncSend.h"
#include "BaseServer.h"
#include "LevelUpAuth.h"


ImplToStringW( KLevelUpAuthManager )
{
	return stm_;
}

//ImplPfID( KLevelUpAuthManager, PI_CHANNEL_GF_AUTH );
ImplPfID( KLevelUpAuthManager, PI_NULL );

ImplementSingleton( KLevelUpAuthManager );

KLevelUpAuthManager::KLevelUpAuthManager()
{
	m_strWsdlPath = "";
}

KLevelUpAuthManager::~KLevelUpAuthManager()
{
}

#include <KncLua.h>
#include <lua_tinker.h>
//using namespace lua_tinker;
#include "BaseServer.h"

void KLevelUpAuthManager::RegToLua()
{
	lua_tinker::class_add<KLevelUpAuthManager>( g_pLua, "KLevelUpAuthManager" );
	lua_tinker::class_def<KLevelUpAuthManager>( g_pLua, "Init",				 &KLevelUpAuthManager::Init );
	lua_tinker::class_def<KLevelUpAuthManager>( g_pLua, "dump",				 &KLevelUpAuthManager::Dump );
	lua_tinker::class_def<KLevelUpAuthManager>( g_pLua, "AddWebServiceAddress", &KLevelUpAuthManager::AddWebServiceAddress );
	lua_tinker::class_def<KLevelUpAuthManager>( g_pLua, "AddWebMethod",	&KLevelUpAuthManager::AddWebMethod );

#   undef _ENUM
#   define _ENUM( id ) lua_tinker::decl( g_pLua, #id, KLevelUpAuth::##id );
#   include "LevelUpAuth_def.h"

	
	lua_tinker::decl( g_pLua, "LevelUpAuthManager", this );
}

void KLevelUpAuthManager::Init( int nThreadNum )
{
	// Init 호출 시점은 항상 AddWebServiceAddress 지정후여야 함 //
	// 그 이유는 바로 아래 CreateThread에서 sdlFile과 smlFile이 입력되어야 하기 때문이다.
	KThreadManager::Init( nThreadNum );
}

//    KSimLayer::GetKObj()->Init(); 될때 BeginThread 호출됨


void KLevelUpAuthManager::AddWebServiceAddress( const char* szWSDL )
{
	m_strWsdlPath = szWSDL;
}

void KLevelUpAuthManager::AddWebMethod( int iSoapAction, const char* szMethod )
{
	KLevelUpAuth::AddWebMethod( iSoapAction, szMethod );
}


KThread* KLevelUpAuthManager::CreateThread()
{
	// Init 호출 시점은 항상 AddWebServiceAddress 지정후여야 함 //
	// 그 이유는 바로 아래 생성자에 sdlFile과 smlFile이 입력되어야 하기 때문이다.
	

	return new KLevelUpAuth(m_strWsdlPath.c_str());
}


// void KLevelUpAuthManager::BeginThread()
// {
// 	KThreadManager::BeginThread();        
//}


//{{ 2011. 08. 17	조효진	로그인 SOAP 함수 로그 
#ifdef SERV_SOAP_LOGIN_FUNC_LOG
void KLevelUpAuthManager::SendToLogDB( unsigned short usEventID )
{
	SendToLogDB( usEventID, char() );
}

void KLevelUpAuthManager::SendSoapLoginFuncLog(std::wstring wstrUserID, CTime StartFuncTime, DWORD dwDiffTime, int iCase)
{

	std::wstring strTmpStartFuncTion = (std::wstring)StartFuncTime.Format( _T( "%Y-%m-%d %H:%M:%S" ) );


	KDBE_SOAP_LOGIN_FUNC_LOG kPacketReq;
	kPacketReq.m_wstrUserID = wstrUserID;
	kPacketReq.m_wstrStartFuncTime = strTmpStartFuncTion;
	kPacketReq.m_dwDiffTime = dwDiffTime;
	kPacketReq.m_iCase = iCase;
	kPacketReq.m_iQueueSize = GetQueueSize();


	SendToLogDB( DBE_SOAP_LOGIN_FUNC_LOG, kPacketReq );
	
}
#endif SERV_SOAP_LOGIN_FUNC_LOG
//}}

#endif // SERV_COUNTRY_BR
