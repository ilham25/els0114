//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 0 - build-system probe implementation.
// See CASH_SHOP_TOOL_PLAN.md, "Phase 0 - Build-system probe".
//////////////////////////////////////////////////////////////////////////
#include "Probe.h"

// This TU is the one that actually touches Lua/sqlite3/zlib/libxml, so it -
// unlike Probe.h - is where the canary belongs. If CLRSupport ever leaks
// onto X2CashShopCore (CASH_SHOP_TOOL_PLAN.md section 6, the
// Microsoft.CppBuild.targets:733-738 /clr:nostdlib injection), this file
// must fail to compile here rather than silently emit IL that unwinds
// Lua's C++ exceptions through a managed frame.
#ifdef _MANAGED
#error "This TU compiled managed. /clr leaked in - see CppBuild.targets:733-738"
#endif

// Verified in CASH_SHOP_TOOL_PLAN.md section 7: luaLib.lib exports
// C++-mangled symbols (the whole project compiles CompileAsCpp), so these
// headers are included directly - never lua.hpp, never our own
// extern "C" wrapper. Either would give LNK2019 on every Lua call.
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "zlib.h"

// LIBXML_STATIC must be defined project-wide (see the vcxproj) or
// XMLPUBVAR-declared globals like xmlFree resolve to __declspec(dllimport)
// and every one of them is an unresolved external against the static lib.
#include <libxml/xmlversion.h>

#include "sqlite3.h"

SProbeResult RunNativeProbe()
{
	SProbeResult r;

	r.pszSqliteVersion = sqlite3_libversion();

	lua_State* L = luaL_newstate();
	r.bLuaStateCreated = ( L != NULL );
	r.pszLuaRelease = LUA_RELEASE;
	if( L )
	{
		lua_close( L );
	}

	r.pszZlibVersion = zlibVersion();
	r.pszLibxmlVersion = LIBXML_DOTTED_VERSION;

	return r;
}
