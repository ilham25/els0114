#pragma once
#include "ServerDefine.h"
#include "RefreshSingleton.h"
//#include "ServerDefine.h"


//{{ 2011. 10. 12	ÃÖÀ°»ç	ÄÁÅÙÃ÷ °ü¸®ÀÚ
//#ifdef SERV_CONTENT_MANAGER


class KGSContentManager
{
private:
	DeclareRefreshSingleton( KGSContentManager );
	DeclareLuaScriptParser;
	DeclDumpToLogFile;	
	DeclDump;
	DeclToStringW;

public:
	KGSContentManager(void);
	~KGSContentManager(void);

	// for lua
	void SetEnableCashShop_LUA( bool bVal );

	// result
	bool IsEnableCashShop() const	{ return m_bEnableCashShop; }

private:
	bool			m_bEnableCashShop;
};

DefRefreshSingletonInline( KGSContentManager );


//#endif SERV_CONTENT_MANAGER
//}}