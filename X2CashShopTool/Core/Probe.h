//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 0 - build-system probe. Proves the
// native static lib (sqlite3, lua, zlib, libxml) links and runs, and that
// this translation unit was never compiled managed. See
// CASH_SHOP_TOOL_PLAN.md, "Phase 0 - Build-system probe".
//////////////////////////////////////////////////////////////////////////
#pragma once

// NOTE: this header is deliberately included from BOTH sides of the split -
// natively from Probe.cpp inside X2CashShopCore, and managed from Main.cpp
// in the /clr Ui project (that is how the Ui exe calls into Core at all).
// So the _MANAGED canary does NOT belong here - it belongs only in the .cpp
// files that actually touch Lua/sqlite3/zlib/libxml, where _MANAGED being
// defined really would mean CLRSupport leaked onto X2CashShopCore. See
// Probe.cpp, and CASH_SHOP_TOOL_PLAN.md section 6.

struct SProbeResult
{
	const char* pszSqliteVersion;
	const char* pszLuaRelease;
	const char* pszZlibVersion;
	const char* pszLibxmlVersion;
	bool        bLuaStateCreated;
};

// Exercises sqlite3, lua 5.1 (luaLib.lib), zlib and libxml just enough to
// prove all four link and initialize. Called from the /clr Ui exe, but this
// function itself is compiled entirely inside the native Core static lib.
SProbeResult RunNativeProbe();
