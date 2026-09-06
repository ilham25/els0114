//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 1 - console entry point and exit test.
// See CASH_SHOP_TOOL_PLAN.md, "Phase 1 - The archive index and the item
// catalog".
//
// This TU is the /clr half. It never includes a Lua, sqlite3, zlib or
// libxml header directly - everything native lives in X2CashShopCore,
// which compiles with CLRSupport absent. That split is the fix for the
// /clr:nostdlib trap documented in the plan's section 6, and it is also
// what keeps Lua's C++ exceptions from ever unwinding a managed frame.
//
// Still SubSystem=Console: the WinForms window arrives in phase 4, and
// with it the EntryPointSymbol=main override (plan section 7).
//////////////////////////////////////////////////////////////////////////
#include <msclr/marshal.h>
#include <msclr/marshal_cppstd.h>  // needed for marshal_as<std::string, String^> specifically
#include <string>
#include <vector>

using namespace System;

#include "../Core/Probe.h"
#include "../Core/KomArchive.h"
#include "../Core/ItemIndex.h"
#include "../Core/IndexCache.h"

namespace
{
	// Every narrow string crossing this boundary is UTF-8 - item names
	// included, because that is what the scripts hold and what KLuaManager
	// decodes with (luaLib/KLuaManager.h:803).
	String^ Utf8( const std::string& str )
	{
		if( str.empty() )
			return String::Empty;

		array<Byte>^ abBytes = gcnew array<Byte>( (int) str.size() );
		System::Runtime::InteropServices::Marshal::Copy(
			IntPtr( (void*) str.data() ), abBytes, 0, (int) str.size() );

		return System::Text::Encoding::UTF8->GetString( abBytes );
	}

	String^ Utf8( const char* psz )
	{
		return Utf8( std::string( ( NULL != psz ) ? psz : "" ) );
	}

	// The progress sink Core writes through. Native class, managed body -
	// which is fine: nothing calls it from inside a lua_pcall.
	class CConsoleLog : public IToolLog
	{
	public:
		virtual void Line( const char* pszText )
		{
			Console::WriteLine( Utf8( pszText ) );
		}
	};

	std::wstring CurrentDirectory()
	{
		return msclr::interop::marshal_as<std::wstring>( System::IO::Directory::GetCurrentDirectory() );
	}

	String^ Pad( String^ s, int iWidth )
	{
		if( nullptr == s )
			s = String::Empty;

		if( s->Length >= iWidth )
			return s->Substring( 0, iWidth );

		return s->PadRight( iWidth );
	}

	void PrintBanner()
	{
		SProbeResult r = RunNativeProbe();

		Console::WriteLine( "X2CashShopTool - phase 1 (archive index + item catalog)" );
		Console::WriteLine( "native libs : sqlite3 {0} / {1} / zlib {2} / libxml {3}",
			Utf8( r.pszSqliteVersion ), Utf8( r.pszLuaRelease ),
			Utf8( r.pszZlibVersion ), Utf8( r.pszLibxmlVersion ) );
		Console::WriteLine();
	}

	void PrintItems( const SExtractResult& kResult )
	{
		Console::WriteLine();
		Console::WriteLine( "--- ten items sampled evenly across the catalog ---" );
		Console::WriteLine( "  {0} {1} {2} {3} {4} {5} {6}",
			Pad( "item_id", 9 ), Pad( "type", 5 ), Pad( "grade", 6 ), Pad( "fsh", 4 ),
			Pad( "eqp", 4 ), Pad( "m_ShopImage", 34 ), "m_Name" );

		const int iSamples = 10;
		const int iCount = (int) kResult.vecItems.size();

		for( int i = 0; i < iSamples && i < iCount; ++i )
		{
			// Evenly spaced rather than the first ten, so the sample spans
			// the whole id range instead of one corner of it.
			const int iIndex = ( iCount <= iSamples ) ? i : (int)( (__int64) i * iCount / iSamples );
			const SItemRow& kRow = kResult.vecItems[iIndex];

			Console::WriteLine( "  {0} {1} {2} {3} {4} {5} {6}",
				Pad( kRow.iItemID.ToString(), 9 ),
				Pad( kRow.iItemType.ToString(), 5 ),
				Pad( kRow.iItemGrade.ToString(), 6 ),
				Pad( kRow.iIsFashion.ToString(), 4 ),
				Pad( kRow.iEquipPosition.ToString(), 4 ),
				Pad( Utf8( kRow.strShopImage ), 34 ),
				Utf8( kRow.strName ) );
		}
	}

	void PrintCategories( const SExtractResult& kResult )
	{
		Console::WriteLine();
		Console::WriteLine( "--- cash shop tabs, from CashShopCategory.lua ---" );
		Console::WriteLine( "  the billing category number is what cash_product.category holds;" );
		Console::WriteLine( "  the CSSC_* enum beside it is NOT (X2CashShop.cpp:6200)." );
		Console::WriteLine( "  {0} {1} {2} {3}",
			Pad( "tab", 5 ), Pad( "REAL_ID", 9 ), Pad( "sub #", 7 ), "cssc enum -> billing category no" );

		int iLastTab = -1;
		for( size_t u = 0; u != kResult.vecCategories.size(); ++u )
		{
			const SCashCategoryRow& kRow = kResult.vecCategories[u];

			String^ sTab		= ( kRow.iTabIdx != iLastTab ) ? kRow.iTabIdx.ToString() : String::Empty;
			String^ sReal	= ( kRow.iTabIdx != iLastTab ) ? kRow.iRealID.ToString() : String::Empty;
			iLastTab = kRow.iTabIdx;

			Console::WriteLine( "  {0} {1} {2} {3} -> {4}",
				Pad( sTab, 5 ), Pad( sReal, 9 ), Pad( kRow.iSubOrdinal.ToString(), 7 ),
				Pad( kRow.iCsscEnum.ToString(), 6 ), kRow.iBillingCategoryNo );
		}

		Console::WriteLine( "  {0} tab(s), {1} sub-category pair(s)",
			( kResult.vecCategories.empty() ? 0 : ( kResult.vecCategories.back().iTabIdx ) ),
			(int) kResult.vecCategories.size() );
	}

	void PrintExtractionStats( const SExtractResult& kResult )
	{
		Console::WriteLine();
		Console::WriteLine( "items    : {0} captured, {1} duplicate id(s) dropped",
			(int) kResult.vecItems.size(), kResult.iDuplicateIDs );
		Console::WriteLine( "rejected : {0} row(s), matching the client's own rule -", kResult.iRejectedRows );
		Console::WriteLine( "             {0} unusable (m_UseCondition == UC_NONE = {1})",
			kResult.iRejectedUnusable, kResult.iEnumUcNone );
		Console::WriteLine( "             {0} with no m_ItemID, {1} with no m_Name, {2} with no m_ItemType",
			kResult.iRejectedNoID, kResult.iRejectedNoName, kResult.iRejectedNoType );
		Console::WriteLine( "           {0} with a non-empty m_ShopImage", kResult.iWithShopImage );
		Console::WriteLine( "           {0} with m_ItemType 0  (all of them would mean Enum.lua did not take)",
			kResult.iZeroItemType );
		Console::WriteLine( "trans    : {0} name(s) overlaid from ItemTrans.lua, {1} orphan id(s)",
			kResult.iTransApplied, kResult.iTransOrphans );

		if( false == kResult.vecStubbedCalls.empty() )
		{
			Console::WriteLine( "stubbed  : method(s) the catch-all __index answered for, most-called first:" );
			for( size_t u = 0; u != kResult.vecStubbedCalls.size() && u < 12; ++u )
			{
				Console::WriteLine( "             {0} x{1}",
					Pad( Utf8( kResult.vecStubbedCalls[u].strName ), 34 ),
					kResult.vecStubbedCalls[u].iCount );
			}
			if( kResult.vecStubbedCalls.size() > 12 )
				Console::WriteLine( "             ... and {0} more", (int) kResult.vecStubbedCalls.size() - 12 );
		}
		else
		{
			Console::WriteLine( "stubbed  : none - the scripts called nothing the stand-ins do not implement" );
		}
	}
}

[STAThread]
int main( array<String^>^ args )
{
	try
	{
		Console::OutputEncoding = System::Text::Encoding::UTF8;
	}
	catch( Exception^ )
	{
		// Redirected output has no console codepage to set. Item names are
		// English in this build, so this costs nothing when it happens.
	}

	bool bForceRebuild = false;
	for( int i = 0; i < args->Length; ++i )
	{
		if( args[i]->Equals( "--rebuild", StringComparison::OrdinalIgnoreCase ) )
			bForceRebuild = true;
	}

	PrintBanner();

	CConsoleLog kLog;

	const std::wstring wstrDataDir = CurrentDirectory();
	Console::WriteLine( "game dir : {0}", msclr::interop::marshal_as<String^>( wstrDataDir ) );

	// The tool is meant to run out of the game directory, for the same
	// reason the client is: the archives are found relative to '.', and
	// els_db.sql lives there too (phase 3).
	std::wstring wstrArchive = wstrDataDir + L"\\" + msclr::interop::marshal_as<std::wstring>( Utf8( ItemCatalogArchiveName() ) );

	__int64 iArchiveSize = 0;
	if( false == GetFileStamp( wstrArchive, &iArchiveSize, NULL ) )
	{
		Console::WriteLine();
		Console::WriteLine( "ERROR: {0} is not in the current directory.", Utf8( ItemCatalogArchiveName() ) );
		Console::WriteLine( "       Run this tool WITH THE GAME DIRECTORY as the working directory," );
		Console::WriteLine( "       the same one X2_offline.exe and els_db.sql live in." );
		return 2;
	}

	//////////////////////////////////////////////////////////////////////
	// The archive index.

	CKomIndex kIndex;
	if( false == kIndex.Mount( wstrDataDir, &kLog ) )
	{
		Console::WriteLine( "ERROR: no archives could be mounted." );
		return 3;
	}

	//////////////////////////////////////////////////////////////////////
	// The item catalog, through the cache.

	const std::wstring wstrCachePath = DefaultCachePath();
	Console::WriteLine( "index    : {0}", msclr::interop::marshal_as<String^>( wstrCachePath ) );

	CIndexCache	kCache;
	std::string	strError;

	if( false == kCache.Open( wstrCachePath, strError ) )
	{
		Console::WriteLine( "ERROR: cannot open the index cache: {0}", Utf8( strError ) );
		return 4;
	}

	std::string strReason;
	const bool bCurrent = kCache.IsCurrent( wstrArchive, strReason );

	if( bForceRebuild || false == bCurrent )
	{
		if( bForceRebuild )
			Console::WriteLine( "           rebuilding: --rebuild was given" );
		else
			Console::WriteLine( "           rebuilding: {0}", Utf8( strReason ) );

		Console::WriteLine();

		SExtractResult kExtracted;
		if( false == ExtractItemCatalog( kIndex, kExtracted, &kLog ) )
		{
			Console::WriteLine();
			Console::WriteLine( "ERROR: the item catalog could not be extracted." );
			return 5;
		}

		PrintExtractionStats( kExtracted );

		System::Diagnostics::Stopwatch^ kWatch = System::Diagnostics::Stopwatch::StartNew();
		if( false == kCache.Store( kExtracted, wstrArchive, strError ) )
		{
			Console::WriteLine( "ERROR: cannot write the index cache: {0}", Utf8( strError ) );
			return 6;
		}
		kWatch->Stop();

		Console::WriteLine();
		Console::WriteLine( "extract  : {0:F2} s in total ({1:F2} Enum, {2:F2} Item, {3:F2} ItemTrans, {4:F2} CashShopCategory)",
			kExtracted.dTotalSeconds, kExtracted.dEnumSeconds, kExtracted.dItemSeconds,
			kExtracted.dTransSeconds, kExtracted.dCategorySeconds );
		Console::WriteLine( "cache    : {0} item(s) + {1} category row(s) written in {2} ms",
			(int) kExtracted.vecItems.size(), (int) kExtracted.vecCategories.size(),
			kWatch->ElapsedMilliseconds );
	}
	else
	{
		Console::WriteLine( "           cache is current - no rebuild needed" );
	}

	// Whichever path got us here, the numbers below come from the cache,
	// which is what every later phase will actually read.
	System::Diagnostics::Stopwatch^ kLoadWatch = System::Diagnostics::Stopwatch::StartNew();

	SExtractResult kLoaded;
	if( false == kCache.Load( kLoaded, strError ) )
	{
		Console::WriteLine( "ERROR: cannot read the index cache back: {0}", Utf8( strError ) );
		return 7;
	}
	kLoadWatch->Stop();

	Console::WriteLine();
	Console::WriteLine( "cached load : {0} item(s), {1} category row(s), {2} ms",
		(int) kLoaded.vecItems.size(), (int) kLoaded.vecCategories.size(),
		kLoadWatch->ElapsedMilliseconds );

	PrintItems( kLoaded );
	PrintCategories( kLoaded );

	Console::WriteLine();
	Console::WriteLine( "Run again to see the cached path on its own; --rebuild forces the slow path." );

	return 0;
}
