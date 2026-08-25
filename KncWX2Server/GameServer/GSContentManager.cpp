#include ".\gscontentmanager.h"


//{{ 2011. 10. 12	ÃÖÀ°»ç	ÄÁÅÙÃ÷ °ü¸®ÀÚ
//#ifdef SERV_CONTENT_MANAGER


ImplementRefreshSingleton( KGSContentManager );

KGSContentManager::KGSContentManager(void)
{
}

KGSContentManager::~KGSContentManager(void)
{
}

ImplToStringW( KGSContentManager )
{
	stm_	<< L"----------[ Game Server Content Manager ]----------" << std::endl
			<< TOSTRINGW( m_bEnableCashShop )
		;

	return stm_;
}

ImplementLuaScriptParser( KGSContentManager )
{
	lua_tinker::class_add<KGSContentManager>( GetLuaState(), "KGSContentManager" );
	lua_tinker::class_def<KGSContentManager>( GetLuaState(), "SetEnableCashShop",		&KGSContentManager::SetEnableCashShop_LUA );
	lua_tinker::class_def<KGSContentManager>( GetLuaState(), "dump",					&KGSContentManager::Dump );

	lua_tinker::decl( GetLuaState(), "GSContentManager", this );
}

void KGSContentManager::SetEnableCashShop_LUA( bool bVal )
{
	m_bEnableCashShop = bVal;

	START_LOG( cout, L"[ÄÁÅÙÃ÷ ¸Å´ÏÀú] Ä³½¬¼¥ ±â´É : " << m_bEnableCashShop );
}


//#endif SERV_CONTENT_MANAGER
//}}


