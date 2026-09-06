//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 0 - build-system probe entry point.
// See CASH_SHOP_TOOL_PLAN.md, "Phase 0 - Build-system probe".
//
// This TU is the /clr half. It never includes a Lua, sqlite3, zlib or
// libxml header directly - everything native lives in X2CashShopCore
// (Probe.h/.cpp), which compiles with CLRSupport absent. That split is
// the fix for the /clr:nostdlib trap documented in the plan's section 6.
//////////////////////////////////////////////////////////////////////////
#include <msclr/marshal.h>
#include <msclr/marshal_cppstd.h>  // needed for marshal_as<std::string, String^> specifically
#include <string>

using namespace System;

#include "../Core/Probe.h"

[STAThread]
int main(array<String^>^ /*args*/)
{
	SProbeResult r = RunNativeProbe();

	// Round-trip a managed String^ through a native std::string and back -
	// proves the CLR is genuinely live and the native call boundary works,
	// not merely that the exe links.
	String^ original = "X2CashShopTool phase 0 probe";
	std::string native = msclr::interop::marshal_as<std::string>( original );
	String^ roundTripped = msclr::interop::marshal_as<String^>( native );

	Console::WriteLine( "sqlite3 : " + gcnew String( r.pszSqliteVersion ) );
	Console::WriteLine( "lua     : " + gcnew String( r.pszLuaRelease ) + " (state created: " + ( r.bLuaStateCreated ? "true" : "false" ) + ")" );
	Console::WriteLine( "zlib    : " + gcnew String( r.pszZlibVersion ) );
	Console::WriteLine( "libxml  : " + gcnew String( r.pszLibxmlVersion ) );
	Console::WriteLine( "CLR round-trip : " + roundTripped );

	return roundTripped->Equals( original ) ? 0 : 1;
}
