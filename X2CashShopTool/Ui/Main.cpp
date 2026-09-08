//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phases 2-3 - console entry point, the icon
// wall that was phase 2's exit test, and the els_db.sql report and
// round-trip that are phase 3's. See CASH_SHOP_TOOL_PLAN.md.
//
// This TU is the /clr half. It never includes a Lua, sqlite3, zlib or
// libxml header directly - everything native lives in X2CashShopCore,
// which compiles with CLRSupport absent. That split is the fix for the
// /clr:nostdlib trap documented in the plan's section 6, and it is also
// what keeps Lua's C++ exceptions from ever unwinding a managed frame.
//
// Still SubSystem=Console, even now that there is a window: a
// console-subsystem /clr exe opens a Form perfectly well, and keeping
// stdout means the icon census and the wall come out of the same run.
// Phase 4 is where SubSystem flips to Windows and EntryPointSymbol=main
// becomes necessary (plan section 7).
//
// Switches:
//   --rebuild      force both the item catalog and the icon locator to rebuild
//   --items        also print the phase 1 detail (ten items, the tab table)
//   --decode-all   decode EVERY icon the catalog names, not just the wall's
//   --no-window    census only, do not open the wall
//   --dump <name> [outfile]
//                  decode one image and write its raw BGRA out, so the
//                  decoder can be checked against a second implementation
//                  by arithmetic rather than by eye
//   --db           open els_db.sql read-only and print the catalog and the
//                  dropped-row report; no icons, no window
//   --db-test      --db, then the insert/edit/delete round-trip against a
//                  COPY of the save
//   --live         with --db-test, run the same round-trip against the LIVE
//                  save afterwards - and only if the copy passed
//   --picker-test  build phase 5's item picker headless and print what it
//                  cost: the one-off marshalling of 48,754 items, a filter
//                  pass over all of them for a handful of queries, and the
//                  managed delta across a full sweep of every row's icon.
//                  Opens no window and touches els_db.sql not at all
//   --labels       print every human label the window can show, every value
//                  of every enum it covers, and WHERE EACH ONE CAME FROM -
//                  the game's own string table, a mapping this build
//                  compiles out, a shipped dialog script, or this tool's
//                  own wording. Opens no window and reads no save. It
//                  exists so this tool's own inventions are auditable in
//                  one read instead of being hunted through a UI
//   --layout-test  build the editor over a COPY of the save, show it
//                  off-screen at the 900x560 minimum and at 1920x1080, and
//                  report every pair of overlapping controls, every clipped
//                  or collapsed one, and whether the product grid can still
//                  be scrolled to its last row at each size. Opens no
//                  visible window and never touches the live save
//   --technical    start the editor with View -> Technical details already
//                  on: the script's own enum names and every row's numbers
//                  back on screen, which is phase 5's presentation. The
//                  setting persists either way in X2CashShopTool.ini
//   --db-path <file>
//                  open that save instead of ./els_db.sql. The archives are
//                  still read from the working directory. Exists so the two
//                  refusal paths (a schema that is not 11, a file another
//                  process holds) can be exercised against a real file
//                  without going near the live save
//////////////////////////////////////////////////////////////////////////
#include <msclr/marshal.h>
#include <msclr/marshal_cppstd.h>  // needed for marshal_as<std::string, String^> specifically
#include <algorithm>
#include <map>
#include <string>
#include <string.h>
#include <vector>

using namespace System;

#include "../Core/Probe.h"
#include "../Core/KomArchive.h"
#include "../Core/ItemIndex.h"
#include "../Core/DdsDecode.h"
#include "../Core/IconStore.h"
#include "../Core/IndexCache.h"
#include "../Core/CashDb.h"
#include "../Core/Labels.h"

#include "IconWallForm.h"
#include "MainForm.h"

using namespace X2CashShopTool;

namespace
{
	// Every narrow string crossing this boundary is UTF-8 - item names
	// included, because that is what the scripts hold and what KLuaManager
	// decodes with (luaLib/KLuaManager.h:803).
	// Phase 4 moved the body into MainForm.h's NativeBridge, so the window
	// and the console report share one conversion rather than two copies
	// that can disagree about the encoding.
	String^ Utf8( const std::string& str )
	{
		return NativeBridge::Utf8( str );
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

	//////////////////////////////////////////////////////////////////////
	// The one place native pixels become managed ones.
	//
	// CIconStore::Get hands back a BORROWED pointer, valid only until the
	// next call to Get - so the copy happens here, immediately, before
	// anything else touches the store. Format32bppArgb's scanline layout
	// is B,G,R,A on a little-endian machine, which is exactly what
	// SDecodedImage holds, so this is a straight row-by-row memcpy and not
	// a per-pixel shuffle.
	Drawing::Bitmap^ ToBitmap( const SDecodedImage* pImage )
	{
		return NativeBridge::ToBitmap( pImage );
	}

	// The one shipped shop image that is not a DDS at all (a PNG under a
	// .dds name). Reported, then shown anyway - it is a perfectly good
	// picture and the fallback would be a lie about it.
	Drawing::Bitmap^ FromRawBytes( const std::vector<char>& vecBytes )
	{
		return NativeBridge::FromRawBytes( vecBytes );
	}

	//////////////////////////////////////////////////////////////////////

	void PrintBanner()
	{
		SProbeResult r = RunNativeProbe();

		Console::WriteLine( "X2CashShopTool - the offline cash shop catalog editor" );
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
		Console::WriteLine( "  {0} {1} {2} {3}",
			Pad( "tab", 5 ), Pad( "REAL_ID", 9 ), Pad( "sub #", 7 ), "cssc enum -> billing category no" );

		int iLastTab = -1;
		for( size_t u = 0; u != kResult.vecCategories.size(); ++u )
		{
			const SCashCategoryRow& kRow = kResult.vecCategories[u];

			String^ sTab	= ( kRow.iTabIdx != iLastTab ) ? kRow.iTabIdx.ToString() : String::Empty;
			String^ sReal	= ( kRow.iTabIdx != iLastTab ) ? kRow.iRealID.ToString() : String::Empty;
			iLastTab = kRow.iTabIdx;

			Console::WriteLine( "  {0} {1} {2} {3} -> {4}",
				Pad( sTab, 5 ), Pad( sReal, 9 ), Pad( kRow.iSubOrdinal.ToString(), 7 ),
				Pad( kRow.iCsscEnum.ToString(), 6 ), kRow.iBillingCategoryNo );
		}
	}

	//////////////////////////////////////////////////////////////////////
	// Phase 6 - --labels.
	//
	// The whole point of this switch is RULE 1: this tool's own wording is
	// never presented as the game's. That is only a real guarantee if the
	// full set of it can be read in one place, so every label the window
	// can show is printed here with its origin, and the TOOL ones are
	// gathered again at the end so the inventions are countable rather
	// than scattered through a UI.
	//
	// The three item enums are walked over the values that ACTUALLY OCCUR
	// in the catalog, with counts, because a coverage claim about values
	// nobody has is worthless. The two cash-shop enums are walked over
	// CashShopCategory.lua's own table for the same reason.

	void CountField( const SExtractResult& kResult, int iWhich, std::map<int, int>& mapOut )
	{
		mapOut.clear();

		for( size_t u = 0; u != kResult.vecItems.size(); ++u )
		{
			const SItemRow& kRow = kResult.vecItems[u];

			const int iValue = ( 0 == iWhich ) ? kRow.iItemType
							 : ( 1 == iWhich ) ? kRow.iEquipPosition
											   : kRow.iItemGrade;

			++mapOut[iValue];
		}
	}

	// The citation, or nothing. A fallback label has no STR_ID because it
	// is no longer that string - see the three fallback paths in
	// Labels.cpp - so printing "STR_ID_-1" would be the tool citing a row
	// it did not read.
	String^ Cite( const SLabel& kLabel )
	{
		if( kLabel.iStringID < 0 )
			return String::Empty;

		return String::Format( "   STR_ID_{0}", kLabel.iStringID );
	}

	// One line per value, in the same shape for all five enums so the
	// origin column reads as a column.
	void PrintLabelRow( int iValue, const std::string& strEnumName, const SLabel& kLabel, int iCount )
	{
		String^ sCount = ( iCount >= 0 ) ? String::Format( "{0,7:N0}", iCount ) : Pad( String::Empty, 7 );

		Console::WriteLine( "  {0} {1} {2} {3} {4}",
			Pad( iValue.ToString(), 5 ),
			Pad( strEnumName.empty() ? gcnew String( "(no Enum.lua name)" ) : Utf8( strEnumName ), 26 ),
			Pad( Utf8( kLabel.strText ), 28 ),
			Pad( gcnew String( LabelOriginTag( kLabel.eOrigin ) ), 7 ),
			sCount );

		// The citation, and the untouched .ess row when it differs from
		// the label - which is only the grades, whose shipped strings
		// carry square brackets.
		if( kLabel.iStringID >= 0 )
		{
			Console::WriteLine( "        STR_ID_{0}{1}", kLabel.iStringID,
				( false == kLabel.strEssRaw.empty() && kLabel.strEssRaw != kLabel.strText )
					? String::Format( "   the file holds \"{0}\"", Utf8( kLabel.strEssRaw ) )
					: String::Empty );
		}

		if( false == kLabel.strScriptName.empty() )
		{
			Console::WriteLine( "        the studio calls this control {0}, out of DLG_Cash_Shop_Subpage_Fashion.lua",
				Utf8( kLabel.strScriptName ) );
		}
	}

	void PrintLabelHeading( const char* pszTitle )
	{
		Console::WriteLine();
		Console::WriteLine( "--- {0} ---", Utf8( pszTitle ) );
		Console::WriteLine( "  {0} {1} {2} {3} {4}",
			Pad( "value", 5 ), Pad( "Enum.lua", 26 ), Pad( "label", 28 ),
			Pad( "origin", 7 ), Pad( "items", 7 ) );
	}

	void PrintOneItemEnum( const CLabelStore& kStore, const SExtractResult& kResult,
							ELabelEnum eEnum, const char* pszTable, int iWhich )
	{
		PrintLabelHeading( LabelEnumName( eEnum ) );

		std::map<int, int> mapCount;
		CountField( kResult, iWhich, mapCount );

		int iEss = 0;
		int iCode = 0;
		int iTool = 0;

		for( std::map<int, int>::const_iterator it = mapCount.begin(); it != mapCount.end(); ++it )
		{
			const std::string	strName	= LookupEnumRowName( kResult.vecEnumNames, pszTable, it->first );
			const SLabel		kLabel	= kStore.Label( eEnum, strName.c_str(), it->first );

			PrintLabelRow( it->first, strName, kLabel, it->second );

			if( LabelOrigin_Ess == kLabel.eOrigin )			++iEss;
			else if( LabelOrigin_Code == kLabel.eOrigin )	++iCode;
			else											++iTool;
		}

		Console::WriteLine( "  {0} of {1} value(s) occurring in the catalog are the game's own text; {2} from a mapping this build compiles out; {3} this tool's",
			iEss, (int) mapCount.size(), iCode, iTool );
	}

	void PrintLabels( const CLabelStore& kStore, const SExtractResult& kResult )
	{
		Console::WriteLine();
		Console::WriteLine( "=== every label this tool can show, and where each one comes from ===" );
		Console::WriteLine();
		Console::WriteLine( "  ESS      the game's own localized string, reached through the client's own" );
		Console::WriteLine( "           enum -> STR_ID switch" );
		Console::WriteLine( "  CODE     a mapping that exists in this repo and that THIS BUILD compiles out" );
		Console::WriteLine( "  SCRIPT   the studio's own control name out of a shipped dialog .lua -" );
		Console::WriteLine( "           printed under the label it belongs to, never AS the label, because a" );
		Console::WriteLine( "           control name is no more a human name for a thing than an enum is" );
		Console::WriteLine( "  TOOL     this tool's wording - listed together at the end as well, so the" );
		Console::WriteLine( "           whole set of it can be read in one place" );
		Console::WriteLine();

		// The "strings :" line is already printed by the loader above, so
		// only the DEGRADE case is repeated here - and it is repeated on
		// purpose, because it changes what every line below means.
		if( false == kStore.HasStrings() )
		{
			Console::WriteLine();
			Console::WriteLine( "*** General.ess WAS NOT READ, so EVERY LABEL BELOW IS THIS TOOL'S OWN WORDING. ***" );
		}

		//////////////////////////////////////////////////////////////////
		// The two labels that are not per-value.

		const SLabel kCurrency	= kStore.Currency();
		const SLabel kCostume	= kStore.Costume();

		Console::WriteLine();
		Console::WriteLine( "--- the two standalone strings ---" );
		Console::WriteLine( "  the currency the shop prints after every price : {0}   [{1}]{2}",
			Utf8( kCurrency.strText ), Utf8( LabelOriginTag( kCurrency.eOrigin ) ), Cite( kCurrency ) );
		Console::WriteLine( "  what the client itself calls m_bFashion         : {0}   [{1}]{2}",
			Utf8( kCostume.strText ), Utf8( LabelOriginTag( kCostume.eOrigin ) ), Cite( kCostume ) );

		//////////////////////////////////////////////////////////////////
		// The three item enums, over the values that occur.

		PrintOneItemEnum( kStore, kResult, LabelEnum_ItemType,		EnumTableItemType(),		0 );
		PrintOneItemEnum( kStore, kResult, LabelEnum_EquipPosition,	EnumTableEquipPosition(),	1 );
		PrintOneItemEnum( kStore, kResult, LabelEnum_ItemGrade,		EnumTableItemGrade(),		2 );

		//////////////////////////////////////////////////////////////////
		// The shop's own tabs, over CashShopCategory.lua's table.

		PrintLabelHeading( "the shop's tabs and sub-tabs, from CashShopCategory.lua" );

		int iLastTab = -1;
		for( size_t u = 0; u != kResult.vecCategories.size(); ++u )
		{
			const SCashCategoryRow& kRow = kResult.vecCategories[u];

			if( kRow.iTabIdx != iLastTab )
			{
				iLastTab = kRow.iTabIdx;

				const SLabel kTab = kStore.Label( LabelEnum_CashCategory, kRow.strTabName.c_str(), kRow.iRealID );
				PrintLabelRow( kRow.iRealID, kRow.strTabName, kTab, -1 );
			}

			const SLabel kSub = kStore.Label( LabelEnum_CashSubCategory, kRow.strSubName.c_str(), kRow.iCsscEnum );
			PrintLabelRow( kRow.iBillingCategoryNo, kRow.strSubName, kSub, -1 );
		}

		Console::WriteLine( "  the value column is REAL_ID for a tab and cash_product.category for a sub-tab -" );
		Console::WriteLine( "  the number the save actually holds, which no label ever replaces" );

		//////////////////////////////////////////////////////////////////
		// And every TOOL label again, together. This is the audit: it is
		// the complete list of words in this tool that are not the game's.

		std::vector<SLabelDumpRow> vecAll;
		kStore.DumpAll( vecAll );

		Console::WriteLine();
		Console::WriteLine( "--- every label this tool worded itself ---" );

		int iToolTotal = 0;
		for( size_t u = 0; u != vecAll.size(); ++u )
		{
			if( LabelOrigin_Tool != vecAll[u].kLabel.eOrigin )
				continue;

			++iToolTotal;

			Console::WriteLine( "  {0} {1} {2}",
				Pad( Utf8( LabelEnumName( vecAll[u].eEnum ) ), 24 ),
				Pad( Utf8( vecAll[u].strEnumName ), 26 ),
				Utf8( vecAll[u].kLabel.strText ) );

			Console::WriteLine( "        {0}", Utf8( vecAll[u].kLabel.strNote ) );
		}

		Console::WriteLine();
		Console::WriteLine( "{0} of the {1} entries in the label table are this tool's own wording.",
			iToolTotal, (int) vecAll.size() );
		Console::WriteLine();
		Console::WriteLine( "Nothing was opened and nothing was changed: --labels reads no save." );
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
		Console::WriteLine( "trans    : {0} name(s) overlaid from ItemTrans.lua, {1} orphan id(s)",
			kResult.iTransApplied, kResult.iTransOrphans );

		// Phase 5. The hidden-package set is the shop's SECOND silent-drop
		// rule and, unlike the missing-templet one, nothing in the game's
		// own logs mentions it - so the tool prints it every rebuild.
		if( kResult.bPackageDataRan )
		{
			Console::WriteLine( "packages : {0} PackageItemData.lua row(s); {1} distinct item(s) the shop will not",
				kResult.iPackageRows, (int) kResult.vecHiddenPackageItems.size() );
			Console::WriteLine( "           show on their own (declared package contents with bShowItem false)" );
		}
		else
		{
			Console::WriteLine( "packages : PackageItemData.lua did NOT run - the shop's hidden-package rule is" );
			Console::WriteLine( "           unknown for this index. The item catalog itself is unaffected." );
		}

		Console::WriteLine( "enums    : {0} value(s) named out of Enum.lua's own tables, for the picker's filters",
			(int) kResult.vecEnumNames.size() );

		if( false == kResult.vecStubbedCalls.empty() )
		{
			Console::WriteLine( "stubbed  : method(s) the catch-all __index answered for, most-called first:" );
			for( size_t u = 0; u != kResult.vecStubbedCalls.size() && u < 12; ++u )
			{
				Console::WriteLine( "             {0} x{1}",
					Pad( Utf8( kResult.vecStubbedCalls[u].strName ), 34 ),
					kResult.vecStubbedCalls[u].iCount );
			}
		}
		else
		{
			Console::WriteLine( "stubbed  : none - the scripts called nothing the stand-ins do not implement" );
		}
	}

	//////////////////////////////////////////////////////////////////////
	// Phase 3 - els_db.sql.
	//
	// The report first, then the round-trip. The round-trip runs against a
	// COPY of the save and only touches the live one when --live is also
	// given, and only after the copy has passed - the plan's own ordering,
	// and the reason is that els_db.sql is the only copy of the character.

	String^ DbOpenLine( ECashDbResult eResult, const CCashDb& kDb, const std::string& strError )
	{
		if( CashDb_OK == eResult )
		{
			return String::Format( "ok ({0}), PRAGMA user_version = {1}",
				kDb.IsReadOnly() ? "read-only" : "read-write", kDb.UserVersion() );
		}

		return String::Format( "REFUSED - {0}: {1}",
			Utf8( CashDbResultName( eResult ) ), Utf8( strError ) );
	}

	void PrintCatalogReport( const SCashCatalogReport& kReport )
	{
		Console::WriteLine();
		Console::WriteLine( "--- the dropped-row report ---" );
		Console::WriteLine( "  {0} cash_product row(s)", kReport.iRows );
		Console::WriteLine( "  {0} the client will show", kReport.iKept );
		Console::WriteLine( "  {0} dropped for having no item templet", kReport.iDropped );
		Console::WriteLine( "      (the client logs exactly this as"
			" \"CASH  catalog: {0} product(s) from {1} cash_product row(s); {2} dropped\")",
			kReport.iKept, kReport.iRows, kReport.iDropped );

		if( false == kReport.vecDropped.empty() )
		{
			Console::WriteLine( "  the first ten dropped:" );
			for( size_t u = 0; u != kReport.vecDropped.size() && u < 10; ++u )
			{
				Console::WriteLine( "    product {0} item {1} category {2} - {3}",
					Pad( kReport.vecDropped[u].iProductNo.ToString(), 6 ),
					Pad( kReport.vecDropped[u].iItemID.ToString(), 9 ),
					Pad( kReport.vecDropped[u].iCategoryNo.ToString(), 5 ),
					Utf8( kReport.vecDropped[u].strDetail ) );
			}
		}

		Console::WriteLine();
		Console::WriteLine( "  per billing category - the number cash_product.category holds:" );
		Console::WriteLine( "    {0} {1} {2} {3}",
			Pad( "cat", 6 ), Pad( "rows", 7 ), Pad( "dropped", 9 ), "tab" );

		for( size_t u = 0; u != kReport.vecCategories.size(); ++u )
		{
			const SCashCategoryCount& kCount = kReport.vecCategories[u];

			Console::WriteLine( "    {0} {1} {2} {3}",
				Pad( kCount.iCategoryNo.ToString(), 6 ),
				Pad( kCount.iRows.ToString(), 7 ),
				Pad( kCount.iDropped.ToString(), 9 ),
				kCount.bInTabTable
					? String::Format( "{0}", kCount.iTabIdx )
					: "*** no tab in CashShopCategory.lua shows this ***" );
		}

		Console::WriteLine();
		Console::WriteLine( "  {0} categor(ies) reaching no tab, holding {1} row(s)",
			kReport.iOrphanCategories, kReport.iOrphanRows );
		Console::WriteLine( "  {0} row(s) with category or quantity outside 1..127", (int) kReport.vecOutOfRange.size() );
		Console::WriteLine( "  {0} item(s) sold as more than one product - deliberate, not a defect",
			kReport.iDuplicateItems );
		Console::WriteLine( "  price ranges {0}..{1}", kReport.iMinPrice, kReport.iMaxPrice );
	}

	// An item the catalog holds that nothing currently sells - so the
	// round-trip's insert is a row that could not have been there already.
	// Returns an index into kCatalog.vecItems, or -1.
	int PickUnsoldItem( const SExtractResult& kCatalog, const std::vector<SCashProductRow>& vecProducts )
	{
		std::vector<int> vecSold;
		for( size_t u = 0; u != vecProducts.size(); ++u )
			vecSold.push_back( vecProducts[u].iItemID );
		std::sort( vecSold.begin(), vecSold.end() );

		for( size_t u = 0; u != kCatalog.vecItems.size(); ++u )
		{
			if( false == std::binary_search( vecSold.begin(), vecSold.end(),
					kCatalog.vecItems[u].iItemID ) )
			{
				return (int) u;
			}
		}

		return -1;
	}

	bool CheckRefused( const CCashDb& kDb, const SCashProductRow& kRow, String^ sWhat )
	{
		std::string strError;
		const bool bRefused = ( false == kDb.Validate( kRow, true, strError ) );

		Console::WriteLine( "    {0} {1}", bRefused ? "refused " : "*** ACCEPTED ***", sWhat );
		if( bRefused )
			Console::WriteLine( "               {0}", Utf8( strError ) );

		return bRefused;
	}

	// insert -> edit -> delete -> reopen -> confirm, against whatever save
	// file it is pointed at. Ends with the table exactly as it found it, so
	// it is safe to run against the live save once the copy has passed.
	bool RunRoundTrip( const std::wstring& wstrDbPath, const SExtractResult& kCatalog )
	{
		std::string strError;
		CCashDb kDb;

		const ECashDbResult eOpen = kDb.Open( wstrDbPath, false, strError );
		Console::WriteLine( "  open     : {0}", DbOpenLine( eOpen, kDb, strError ) );
		if( CashDb_OK != eOpen )
			return false;

		kDb.SetKnownItems( kCatalog.vecItems );

		std::vector<SCashProductRow> vecBefore;
		if( false == kDb.LoadProducts( vecBefore, strError ) )
		{
			Console::WriteLine( "  ERROR    : {0}", Utf8( strError ) );
			return false;
		}

		int iWalletBefore = 0;
		kDb.GetWallet( iWalletBefore, strError );

		int iNext = 0;
		kDb.NextProductNo( iNext, strError );

		Console::WriteLine( "  baseline : {0} product(s), next free product_no {1}, wallet {2}",
			(int) vecBefore.size(), iNext, iWalletBefore );

		const int iItemIndex = PickUnsoldItem( kCatalog, vecBefore );
		if( iItemIndex < 0 )
		{
			Console::WriteLine( "  ERROR    : every catalog item is already sold - nothing to insert." );
			return false;
		}

		const int	iItemID	= kCatalog.vecItems[iItemIndex].iItemID;
		String^		sName	= Utf8( kCatalog.vecItems[iItemIndex].strName );

		const int iCategory = kCatalog.vecCategories.empty()
			? 11 : kCatalog.vecCategories[0].iBillingCategoryNo;

		//////////////////////////////////////////////////////////////////
		// Validation runs BEFORE the first write, and nothing here may
		// take the backup - which is checked, not assumed.

		Console::WriteLine( "  refusals : the limits are enforced, not clamped -" );

		SCashProductRow kBad;
		kBad.iItemID	= iItemID;
		kBad.iCategoryNo= iCategory;
		kBad.iQuantity	= 1;
		kBad.iPrice		= 1;

		int iRefused = 0;

		SCashProductRow kTry = kBad;	kTry.iQuantity		= 0;
		iRefused += CheckRefused( kDb, kTry, "quantity 0" ) ? 1 : 0;

		kTry = kBad;					kTry.iQuantity		= 128;
		iRefused += CheckRefused( kDb, kTry, "quantity 128" ) ? 1 : 0;

		kTry = kBad;					kTry.iCategoryNo	= 0;
		iRefused += CheckRefused( kDb, kTry, "category 0" ) ? 1 : 0;

		kTry = kBad;					kTry.iCategoryNo	= 128;
		iRefused += CheckRefused( kDb, kTry, "category 128" ) ? 1 : 0;

		kTry = kBad;					kTry.iItemID		= 999999999;
		iRefused += CheckRefused( kDb, kTry, "an item id no templet resolves" ) ? 1 : 0;

		kTry = kBad;					kTry.iPrice			= -1;
		iRefused += CheckRefused( kDb, kTry, "price -1" ) ? 1 : 0;

		if( 6 != iRefused )
		{
			Console::WriteLine( "  FAILED   : {0} of 6 bad rows were refused.", iRefused );
			return false;
		}

		if( kDb.BackupTaken() )
		{
			Console::WriteLine( "  FAILED   : a refusal took the backup - validation must not write." );
			return false;
		}

		//////////////////////////////////////////////////////////////////
		// Insert.

		SCashProductRow kRow;
		kRow.iItemID		= iItemID;
		kRow.iCategoryNo	= iCategory;
		kRow.iQuantity		= 7;
		kRow.iPrice			= 4242;
		kRow.iIsEvent		= 1;

		if( false == kDb.Insert( kRow, strError ) )
		{
			Console::WriteLine( "  FAILED   : insert: {0}", Utf8( strError ) );
			return false;
		}

		Console::WriteLine( "  insert   : product_no {0} allocated (max+1 was {1}), item {2} \"{3}\","
			" category {4}, qty 7, price 4242, event",
			kRow.iProductNo, iNext, kRow.iItemID, sName, kRow.iCategoryNo );

		if( kRow.iProductNo != iNext )
		{
			Console::WriteLine( "  FAILED   : product_no {0}, expected {1}.", kRow.iProductNo, iNext );
			return false;
		}

		//////////////////////////////////////////////////////////////////
		// The backup. Taken by the first write, once, and verified on disk
		// rather than believed.

		if( false == kDb.BackupTaken() )
		{
			Console::WriteLine( "  FAILED   : the first write took no backup." );
			return false;
		}

		Console::WriteLine( "  backup   : {0}", msclr::interop::marshal_as<String^>( kDb.BackupLabel() ) );

		const std::vector<std::wstring>& vecBackup = kDb.BackupFiles();
		for( size_t u = 0; u != vecBackup.size(); ++u )
		{
			String^ sPath = msclr::interop::marshal_as<String^>( vecBackup[u] );

			if( false == System::IO::File::Exists( sPath ) )
			{
				Console::WriteLine( "  FAILED   : the backup names {0}, which is not on disk.", sPath );
				return false;
			}

			Console::WriteLine( "             {0}  ({1} byte(s))",
				sPath, ( gcnew System::IO::FileInfo( sPath ) )->Length );
		}

		// The backup is re-checked at the END of the round-trip too, against
		// the row count it should hold. A file that is the right size at the
		// moment it is written and the wrong content by the time the run
		// finishes is exactly the defect this phase found, and only a check
		// after everything else has run can see it.
		String^ sMainBackup = ( vecBackup.empty() )
			? nullptr : msclr::interop::marshal_as<String^>( vecBackup[0] );

		//////////////////////////////////////////////////////////////////
		// Edit, then close and reopen - because a value that only survives
		// inside the connection that wrote it has proved nothing.

		SCashProductRow kEdited = kRow;
		kEdited.iQuantity	= 3;
		kEdited.iPrice		= 777;
		kEdited.iIsEvent	= 0;

		if( false == kDb.Update( kEdited, strError ) )
		{
			Console::WriteLine( "  FAILED   : update: {0}", Utf8( strError ) );
			return false;
		}

		// The backup this run already took is carried across the reopen. Open
		// resets the flag by design, and without this the next write would
		// take a SECOND backup - which, before BackupSaveSet learned to
		// unique its label, landed on the first one's name within the same
		// second and replaced a pre-edit snapshot with a mid-edit one.
		const std::wstring				wstrBackupLabel	= kDb.BackupLabel();
		const std::vector<std::wstring>	vecBackupFiles	= kDb.BackupFiles();

		kDb.Close();

		if( CashDb_OK != kDb.Open( wstrDbPath, false, strError ) )
		{
			Console::WriteLine( "  FAILED   : reopen: {0}", Utf8( strError ) );
			return false;
		}
		kDb.SetKnownItems( kCatalog.vecItems );
		kDb.AdoptBackup( wstrBackupLabel, vecBackupFiles );

		SCashProductRow kReadBack;
		if( false == kDb.FindProduct( kRow.iProductNo, kReadBack, strError ) )
		{
			Console::WriteLine( "  FAILED   : the inserted product is gone after reopening: {0}",
				Utf8( strError ) );
			return false;
		}

		const bool bMatches = ( kReadBack.iItemID == kEdited.iItemID )
			&& ( kReadBack.iCategoryNo	== kEdited.iCategoryNo )
			&& ( kReadBack.iQuantity	== kEdited.iQuantity )
			&& ( kReadBack.iPrice		== kEdited.iPrice )
			&& ( kReadBack.iIsEvent		== kEdited.iIsEvent );

		Console::WriteLine( "  reopen   : product {0} reads back item {1}, category {2}, qty {3},"
			" price {4}, event {5} - {6}",
			kReadBack.iProductNo, kReadBack.iItemID, kReadBack.iCategoryNo,
			kReadBack.iQuantity, kReadBack.iPrice, kReadBack.iIsEvent,
			bMatches ? "matches the edit" : "*** DOES NOT MATCH ***" );

		if( false == bMatches )
			return false;

		//////////////////////////////////////////////////////////////////
		// The wallet, out and back, so the live save ends where it started.

		int iWalletNow = 0;
		if( false == kDb.SetWallet( iWalletBefore + 1, strError )
			|| false == kDb.GetWallet( iWalletNow, strError )
			|| iWalletNow != iWalletBefore + 1 )
		{
			Console::WriteLine( "  FAILED   : wallet write: {0}", Utf8( strError ) );
			return false;
		}

		if( false == kDb.SetWallet( iWalletBefore, strError )
			|| false == kDb.GetWallet( iWalletNow, strError )
			|| iWalletNow != iWalletBefore )
		{
			Console::WriteLine( "  FAILED   : wallet restore: {0}", Utf8( strError ) );
			return false;
		}

		Console::WriteLine( "  wallet   : settings.cash_start {0} -> {1} -> {2}, restored",
			iWalletBefore, iWalletBefore + 1, iWalletNow );

		//////////////////////////////////////////////////////////////////
		// Delete, reopen, confirm - and confirm the table is back to what
		// it was, which is what makes this safe against the live save.

		if( false == kDb.Delete( kRow.iProductNo, strError ) )
		{
			Console::WriteLine( "  FAILED   : delete: {0}", Utf8( strError ) );
			return false;
		}

		kDb.Close();

		if( CashDb_OK != kDb.Open( wstrDbPath, false, strError ) )
		{
			Console::WriteLine( "  FAILED   : reopen after delete: {0}", Utf8( strError ) );
			return false;
		}
		kDb.AdoptBackup( wstrBackupLabel, vecBackupFiles );

		SCashProductRow kGone;
		if( kDb.FindProduct( kRow.iProductNo, kGone, strError ) )
		{
			Console::WriteLine( "  FAILED   : product {0} is still there after the delete.", kRow.iProductNo );
			return false;
		}

		std::vector<SCashProductRow> vecAfter;
		if( false == kDb.LoadProducts( vecAfter, strError ) )
		{
			Console::WriteLine( "  FAILED   : {0}", Utf8( strError ) );
			return false;
		}

		bool bIdentical = ( vecAfter.size() == vecBefore.size() );
		for( size_t u = 0; bIdentical && u != vecAfter.size(); ++u )
		{
			bIdentical = ( vecAfter[u].iProductNo	== vecBefore[u].iProductNo )
				&& ( vecAfter[u].iItemID		== vecBefore[u].iItemID )
				&& ( vecAfter[u].iCategoryNo	== vecBefore[u].iCategoryNo )
				&& ( vecAfter[u].iQuantity		== vecBefore[u].iQuantity )
				&& ( vecAfter[u].iPrice			== vecBefore[u].iPrice )
				&& ( vecAfter[u].iIsEvent		== vecBefore[u].iIsEvent );
		}

		Console::WriteLine( "  delete   : product {0} gone; {1} row(s) left, {2}",
			kRow.iProductNo, (int) vecAfter.size(),
			bIdentical ? "identical row for row to the catalog this started with"
					   : "*** THE CATALOG CHANGED ***" );

		kDb.Close();

		if( false == bIdentical )
			return false;

		//////////////////////////////////////////////////////////////////
		// The backup, re-read now that everything else has run. It must
		// still be the PRE-edit catalog - the whole word "pre" in its name
		// is the claim being checked.

		if( nullptr == sMainBackup )
		{
			Console::WriteLine( "  FAILED   : no backup file to re-check." );
			return false;
		}

		CCashDb kBackup;
		const ECashDbResult eBackup = kBackup.Open(
			msclr::interop::marshal_as<std::wstring>( sMainBackup ), true, strError );

		if( CashDb_OK != eBackup )
		{
			Console::WriteLine( "  FAILED   : the backup will not open: {0}", Utf8( strError ) );
			return false;
		}

		const int iBackupRows = kBackup.ProductCount( strError );
		kBackup.Close();

		// Reading a WAL-mode database creates a -wal and a -shm beside it,
		// and beside a backup those land in db_backup/ as
		// "els_db.sql.bak-pre-<label>-wal" - one character away from the real
		// "els_db.sql-wal.bak-pre-<label>" and holding something completely
		// different. db_backup/ is the safety net; it has to stay legible.
		// Only an EMPTY -wal is removed, which is the only shape a read can
		// leave behind.
		for each( String^ sSuffix in gcnew array<String^>{ "-wal", "-shm" } )
		{
			String^ sSide = sMainBackup + sSuffix;

			if( false == System::IO::File::Exists( sSide ) )
				continue;

			if( sSuffix->Equals( "-wal" ) && ( gcnew System::IO::FileInfo( sSide ) )->Length != 0 )
				continue;

			try					{ System::IO::File::Delete( sSide ); }
			catch( Exception^ )	{ /* someone else has it; leaving it is harmless */ }
		}

		Console::WriteLine( "  backup   : re-read at the end - {0} row(s), {1}",
			iBackupRows,
			( iBackupRows == (int) vecBefore.size() )
				? "the catalog as it was before the first write"
				: "*** NOT THE PRE-EDIT CATALOG ***" );

		return ( iBackupRows == (int) vecBefore.size() );
	}
}

//////////////////////////////////////////////////////////////////////////

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

	// This machine's locale writes 0,45 for a decimal, so the managed
	// {0:F2} timings came out with commas while Core's own printf timings
	// came out with dots - one report, two number formats, and a reader
	// left wondering which line to trust. Every figure this tool prints is
	// a measurement to be compared against another run, so they all get
	// the invariant form. (CultureInfo::DefaultThreadCurrentCulture is
	// .NET 4.5; this is the 4.0 way.)
	System::Threading::Thread::CurrentThread->CurrentCulture =
		System::Globalization::CultureInfo::InvariantCulture;

	bool bForceRebuild	= false;
	bool bShowItems		= false;
	bool bDecodeAll		= false;
	bool bNoWindow		= false;
	bool bWall			= false;
	bool bDb			= false;
	bool bDbTest		= false;
	bool bLive			= false;
	bool bPickerTest	= false;
	bool bLabels		= false;
	bool bTechnical		= false;
	bool bLayoutTest	= false;

	String^ sDbPath		= nullptr;

	String^ sDumpName = nullptr;
	String^ sDumpFile = nullptr;

	for( int i = 0; i < args->Length; ++i )
	{
		if( args[i]->Equals( "--rebuild",	StringComparison::OrdinalIgnoreCase ) )	bForceRebuild	= true;
		if( args[i]->Equals( "--items",		StringComparison::OrdinalIgnoreCase ) )	bShowItems		= true;
		if( args[i]->Equals( "--decode-all",StringComparison::OrdinalIgnoreCase ) )	bDecodeAll		= true;
		if( args[i]->Equals( "--no-window",	StringComparison::OrdinalIgnoreCase ) )	bNoWindow		= true;
		if( args[i]->Equals( "--wall",		StringComparison::OrdinalIgnoreCase ) )	bWall			= true;
		if( args[i]->Equals( "--db",		StringComparison::OrdinalIgnoreCase ) )	bDb				= true;
		if( args[i]->Equals( "--db-test",	StringComparison::OrdinalIgnoreCase ) )	bDbTest			= true;
		if( args[i]->Equals( "--live",		StringComparison::OrdinalIgnoreCase ) )	bLive			= true;
		if( args[i]->Equals( "--picker-test", StringComparison::OrdinalIgnoreCase ) ) bPickerTest	= true;
		if( args[i]->Equals( "--labels",	StringComparison::OrdinalIgnoreCase ) )	bLabels			= true;
		if( args[i]->Equals( "--layout-test", StringComparison::OrdinalIgnoreCase ) ) bLayoutTest	= true;
		if( args[i]->Equals( "--technical",	StringComparison::OrdinalIgnoreCase ) )	bTechnical		= true;

		if( args[i]->Equals( "--db-path", StringComparison::OrdinalIgnoreCase ) && i + 1 < args->Length )
		{
			sDbPath	= args[i + 1];
			bDb		= true;
		}

		if( args[i]->Equals( "--dump", StringComparison::OrdinalIgnoreCase ) && i + 1 < args->Length )
		{
			sDumpName = args[i + 1];
			if( i + 2 < args->Length )
				sDumpFile = args[i + 2];
		}
	}

	PrintBanner();

	CConsoleLog kLog;

	const std::wstring wstrDataDir = CurrentDirectory();
	Console::WriteLine( "game dir : {0}", msclr::interop::marshal_as<String^>( wstrDataDir ) );

	std::wstring wstrArchive = JoinPath( wstrDataDir,
		msclr::interop::marshal_as<std::wstring>( Utf8( ItemCatalogArchiveName() ) ) );

	if( false == GetFileStamp( wstrArchive, NULL, NULL ) )
	{
		Console::WriteLine();
		Console::WriteLine( "ERROR: {0} is not in the current directory.", Utf8( ItemCatalogArchiveName() ) );
		Console::WriteLine( "       Run this tool WITH THE GAME DIRECTORY as the working directory," );
		Console::WriteLine( "       the same one X2_offline.exe and els_db.sql live in." );
		return 2;
	}

	//////////////////////////////////////////////////////////////////////
	// The index cache. Opened before the archives are mounted, because
	// whether the archives have to be mounted at all depends on what the
	// cache already holds.

	const std::wstring wstrCachePath = DefaultCachePath();
	Console::WriteLine( "index    : {0}", msclr::interop::marshal_as<String^>( wstrCachePath ) );

	CIndexCache	kCache;
	std::string	strError;

	if( false == kCache.Open( wstrCachePath, strError ) )
	{
		Console::WriteLine( "ERROR: cannot open the index cache: {0}", Utf8( strError ) );
		return 4;
	}

	std::string strItemReason;
	std::string strIconReason;

	const bool bItemsCurrent = ( false == bForceRebuild ) && kCache.IsCurrent( wstrArchive, strItemReason );
	const bool bIconsCurrent = ( false == bForceRebuild ) && kCache.AreIconsCurrent( wstrDataDir, strIconReason );

	if( bForceRebuild )
	{
		strItemReason = "--rebuild was given";
		strIconReason = "--rebuild was given";
	}

	//////////////////////////////////////////////////////////////////////
	// Mounting the 145 manifests is only needed when something has to be
	// rebuilt from them. When both halves of the cache are current, the
	// tool never opens an archive until an icon is actually drawn - which
	// is the whole reason the locator is persisted.

	CKomIndex	kIndex;
	bool		bMounted = false;

	if( false == bItemsCurrent || false == bIconsCurrent )
	{
		CToolStopwatch kMountWatch;

		if( false == kIndex.Mount( wstrDataDir, &kLog ) )
		{
			Console::WriteLine( "ERROR: no archives could be mounted." );
			return 3;
		}

		bMounted = true;
		Console::WriteLine( "           mount took {0:F2} s", kMountWatch.Seconds() );
	}
	else
	{
		Console::WriteLine( "archives : not mounted - both halves of the index are current" );
	}

	//////////////////////////////////////////////////////////////////////
	// The item catalog.

	if( false == bItemsCurrent )
	{
		Console::WriteLine( "items    : rebuilding: {0}", Utf8( strItemReason ) );
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
		Console::WriteLine( "extract  : {0:F2} s in total ({1:F2} Enum, {2:F2} Item, {3:F2} ItemTrans,"
			" {4:F2} PackageItemData, {5:F2} CashShopCategory)",
			kExtracted.dTotalSeconds, kExtracted.dEnumSeconds, kExtracted.dItemSeconds,
			kExtracted.dTransSeconds, kExtracted.dPackageSeconds, kExtracted.dCategorySeconds );
		Console::WriteLine( "cache    : {0} item(s) + {1} category row(s) + {2} enum name(s)"
			" + {3} hidden package item(s) written in {4} ms",
			(int) kExtracted.vecItems.size(), (int) kExtracted.vecCategories.size(),
			(int) kExtracted.vecEnumNames.size(), (int) kExtracted.vecHiddenPackageItems.size(),
			kWatch->ElapsedMilliseconds );
	}

	System::Diagnostics::Stopwatch^ kLoadWatch = System::Diagnostics::Stopwatch::StartNew();

	SExtractResult kCatalog;
	if( false == kCache.Load( kCatalog, strError ) )
	{
		Console::WriteLine( "ERROR: cannot read the index cache back: {0}", Utf8( strError ) );
		return 7;
	}
	kLoadWatch->Stop();

	Console::WriteLine( "catalog  : {0} item(s), {1} category row(s), {2} enum name(s) loaded from the cache in {3} ms",
		(int) kCatalog.vecItems.size(), (int) kCatalog.vecCategories.size(),
		(int) kCatalog.vecEnumNames.size(), kLoadWatch->ElapsedMilliseconds );

	//////////////////////////////////////////////////////////////////////
	// Phase 6 - the string table.
	//
	// A LOOSE file in the game directory, not a .kom member, and read
	// EVERY RUN rather than cached: 3 MB of UTF-16 parsed in tens of
	// milliseconds is not worth a cache, and keeping it out of ItemIndex.db
	// is what keeps this phase clear of the HasColumn trap phase 4
	// documented for anything that adds a column.
	//
	// Failing is not fatal. Every label falls back to this tool's own
	// wording, and the reason is printed here and shown once in the
	// window's status bar - because a window quietly showing IT_DEFENCE
	// because a file was absent is indistinguishable from phase 5's.

	CLabelStore	kLabelStore;
	std::string	strLabelError;

	System::Diagnostics::Stopwatch^ kLabelWatch = System::Diagnostics::Stopwatch::StartNew();
	const bool bLabelsLoaded = kLabelStore.Load( wstrDataDir, strLabelError );
	kLabelWatch->Stop();

	if( bLabelsLoaded )
	{
		Console::WriteLine( "strings  : {0:N0} row(s) from {1} in {2} ms",
			(int) kLabelStore.StringCount(),
			msclr::interop::marshal_as<String^>( kLabelStore.Path() ),
			kLabelWatch->ElapsedMilliseconds );
	}
	else
	{
		Console::WriteLine( "strings  : *** NOT READ *** {0}",
			msclr::interop::marshal_as<String^>( kLabelStore.Path() ) );
		Console::WriteLine( "           {0}", Utf8( kLabelStore.Degraded() ) );
	}

	if( bLabels )
	{
		PrintLabels( kLabelStore, kCatalog );
		return 0;
	}

	Console::WriteLine( "packages : {0}",
		kCatalog.bPackageDataRan
			? String::Format( "{0} item(s) the shop will not show on their own",
				(int) kCatalog.vecHiddenPackageItems.size() )
			: "unknown - PackageItemData.lua did not run when this index was built" );

	//////////////////////////////////////////////////////////////////////
	// Phase 3 - els_db.sql.
	//
	// Returns before the icon locator is even loaded: nothing in this
	// section needs a picture, and the whole point of persisting the
	// locator was that a run which does not draw never opens an archive.

	if( bDb || bDbTest )
	{
		const std::wstring wstrSavePath = ( nullptr != sDbPath )
			? msclr::interop::marshal_as<std::wstring>( System::IO::Path::GetFullPath( sDbPath ) )
			: JoinPath( wstrDataDir, SaveFileName() );

		Console::WriteLine();
		Console::WriteLine( "--- els_db.sql ---" );
		Console::WriteLine( "save     : {0}", msclr::interop::marshal_as<String^>( wstrSavePath ) );

		{
			// Read-only for the report: it cannot write even by accident,
			// and it does not have to, so it does not get the chance.
			CCashDb kDb;
			const ECashDbResult eOpen = kDb.Open( wstrSavePath, true, strError );

			Console::WriteLine( "open     : {0}", DbOpenLine( eOpen, kDb, strError ) );

			if( CashDb_OK != eOpen )
			{
				Console::WriteLine();
				Console::WriteLine( "Nothing was opened and nothing was changed." );
				return 12;
			}

			int iWallet = 0;
			if( kDb.GetWallet( iWallet, strError ) )
				Console::WriteLine( "wallet   : settings.cash_start = {0}", iWallet );
			else
				Console::WriteLine( "wallet   : {0}", Utf8( strError ) );

			std::vector<SCashProductRow> vecProducts;
			if( false == kDb.LoadProducts( vecProducts, strError ) )
			{
				Console::WriteLine( "ERROR: {0}", Utf8( strError ) );
				return 13;
			}

			int iNext = 0;
			kDb.NextProductNo( iNext, strError );

			Console::WriteLine( "products : {0} row(s), next free product_no {1}",
				(int) vecProducts.size(), iNext );

			SCashCatalogReport kReport;
			BuildCashCatalogReport( vecProducts, kCatalog.vecItems, kCatalog.vecCategories, kReport );
			PrintCatalogReport( kReport );
		}

		if( bDbTest )
		{
			// The copy first, always. Nothing in this phase touches the
			// live save until this has passed.
			String^ sScratchDir = System::IO::Path::Combine(
				System::IO::Path::GetDirectoryName(
					msclr::interop::marshal_as<String^>( wstrCachePath ) ), "selftest" );

			System::IO::Directory::CreateDirectory( sScratchDir );

			const std::wstring wstrScratch = msclr::interop::marshal_as<std::wstring>(
				System::IO::Path::Combine( sScratchDir, gcnew String( SaveFileName() ) ) );

			Console::WriteLine();
			Console::WriteLine( "--- round-trip 1 of 2: a COPY of the save ---" );
			Console::WriteLine( "  copy     : {0}", msclr::interop::marshal_as<String^>( wstrScratch ) );

			if( false == CopySaveSet( wstrSavePath, wstrScratch, strError ) )
			{
				Console::WriteLine( "  ERROR    : {0}", Utf8( strError ) );
				return 14;
			}

			if( false == RunRoundTrip( wstrScratch, kCatalog ) )
			{
				Console::WriteLine();
				Console::WriteLine( "FAILED on the copy. The live save was not touched." );
				return 15;
			}

			Console::WriteLine( "  PASSED   : the copy round-tripped." );

			if( false == bLive )
			{
				Console::WriteLine();
				Console::WriteLine( "--- round-trip 2 of 2: the LIVE save --- skipped. Pass --live to run it." );
			}
			else
			{
				Console::WriteLine();
				Console::WriteLine( "--- round-trip 2 of 2: the LIVE save ---" );

				if( false == RunRoundTrip( wstrSavePath, kCatalog ) )
				{
					Console::WriteLine();
					Console::WriteLine( "FAILED on the live save. The backup named above is in db_backup/." );
					return 16;
				}

				Console::WriteLine( "  PASSED   : the live save round-tripped and ends as it started." );
			}
		}

		Console::WriteLine();
		Console::WriteLine( "peak working set : {0:F1} MB",
			(double) System::Diagnostics::Process::GetCurrentProcess()->PeakWorkingSet64 / ( 1024.0 * 1024.0 ) );

		return 0;
	}

	//////////////////////////////////////////////////////////////////////
	// The icon locator.

	CIconStore kIcons;

	if( false == bIconsCurrent )
	{
		Console::WriteLine( "icons    : rebuilding locator: {0}", Utf8( strIconReason ) );

		if( false == bMounted )
		{
			// Reachable only if the item half was current and the icon
			// half was not, and the mount above was therefore skipped -
			// which the condition guarding it prevents, but a wrong answer
			// here would be an empty icon store rather than an error.
			Console::WriteLine( "ERROR: the icon locator needs the archives mounted." );
			return 8;
		}

		kIcons.Build( kIndex, &kLog );

		System::Diagnostics::Stopwatch^ kWatch = System::Diagnostics::Stopwatch::StartNew();
		if( false == kCache.StoreIcons( kIcons.Locations(), kIcons.Stamps(), wstrDataDir, strError ) )
		{
			Console::WriteLine( "ERROR: cannot write the icon locator: {0}", Utf8( strError ) );
			return 9;
		}
		kWatch->Stop();

		Console::WriteLine( "           {0} locator row(s) + {1} archive stamp(s) written in {2} ms",
			(int) kIcons.Locations().size(), (int) kIcons.Stamps().size(), kWatch->ElapsedMilliseconds );
	}
	else
	{
		System::Diagnostics::Stopwatch^ kWatch = System::Diagnostics::Stopwatch::StartNew();

		std::vector<SIconLocation> vecLocations;
		if( false == kCache.LoadIcons( vecLocations, strError ) )
		{
			Console::WriteLine( "ERROR: cannot read the icon locator back: {0}", Utf8( strError ) );
			return 10;
		}

		kIcons.Adopt( vecLocations, wstrDataDir );
		kWatch->Stop();

		Console::WriteLine( "icons    : {0} locator row(s) loaded from the cache in {1} ms",
			(int) kIcons.Count(), kWatch->ElapsedMilliseconds );
	}

	Console::WriteLine( "           fallback {0}: {1}", Utf8( CIconStore::FallbackImageName() ),
		kIcons.HasImage( CIconStore::FallbackImageName() ) ? "present" : "*** MISSING ***" );

	//////////////////////////////////////////////////////////////////////
	// Phase 5 - the picker, measured rather than felt.
	//
	// The exit test says the picker "opens instantly and scrolls the full
	// ~43k rows without stutter or growth in memory". Two of those three
	// are numbers, and this is where they get produced: the one-off
	// marshalling cost and its managed footprint, the cost of a filter
	// pass over all 48,754 rows for a handful of representative queries,
	// and what the two icon caches hold after a simulated scroll from one
	// end of the list to the other. "It felt smooth" is not a
	// measurement; the third part - whether it feels smooth to a person -
	// is the half only running it answers.
	if( bPickerTest )
	{
		Console::WriteLine();
		Console::WriteLine( "--- the picker, headless ---" );

		ItemCatalogView^ kViewTest = gcnew ItemCatalogView( &kCatalog,
			gcnew LabelBridge( &kCatalog, &kLabelStore ) );

		Console::WriteLine( "build    : {0:N0} item(s) marshalled in {1} ms, {2:N0} KB managed",
			kViewTest->All->Length, kViewTest->BuildMs, kViewTest->ManagedBytes / 1024 );
		Console::WriteLine( "filters  : {0} item type(s) and {1} equip slot(s) actually occur",
			kViewTest->TypeChoices->Count - 1, kViewTest->EquipChoices->Count - 1 );
		Console::WriteLine( "packages : {0}",
			kViewTest->PackageDataKnown
				? String::Format( "{0:N0} item(s) the shop hides as package components",
					kViewTest->HiddenPackageCount )
				: "unknown - PackageItemData.lua did not run" );

		// Exercised through the same public surface the window uses, so a
		// number here is a number about the shipped path.
		ItemPickerForm^ kPicker = gcnew ItemPickerForm( kViewTest, gcnew IconProvider( &kIcons, 1500 ),
			gcnew LabelBridge( &kCatalog, &kLabelStore ), bTechnical );

		Console::WriteLine();
		Console::WriteLine( "  a filter pass is over ALL {0:N0} rows; no index, no debounce:", kViewTest->All->Length );

		static const wchar_t* const s_apszQueries[] =
		{
			L"", L"hat", L"aisha", L"aisha hat", L"1316", L"131641", L"zzzznothing",
		};

		for( size_t u = 0; u != sizeof( s_apszQueries ) / sizeof( s_apszQueries[0] ); ++u )
		{
			int iRows	= 0;
			int iMs		= 0;
			kPicker->MeasureFilter( gcnew String( s_apszQueries[u] ), iRows, iMs );

			Console::WriteLine( "    {0}   {1,7:N0} row(s)   {2} ms",
				Pad( ( 0 == ::wcslen( s_apszQueries[u] ) ) ? "(empty)" : gcnew String( s_apszQueries[u] ), 14 ),
				iRows, iMs );
		}

		//////////////////////////////////////////////////////////////////
		// Can the list reach its last row at all?

		{
			int iRows	= 0;
			int iMs		= 0;
			kPicker->MeasureFilter( String::Empty, iRows, iMs );

			int iCanvas	= 0;
			int iReached	= 0;
			int iRowReached	= 0;
			kPicker->MeasureScrollExtent( iCanvas, iReached, iRowReached );

			Console::WriteLine();
			Console::WriteLine( "  extent   : {0:N0}-pixel canvas for {1:N0} row(s) at 44 px",
				iCanvas, iRows );
			Console::WriteLine( "             scrolled to the end: y = {0:N0}, last row visible = {1:N0}   {2}",
				iReached, iRowReached,
				( iRowReached >= iRows ) ? "-> the last row is reachable"
										 : "*** THE END OF THE LIST CANNOT BE REACHED ***" );
		}

		//////////////////////////////////////////////////////////////////
		// The scroll. Every icon a full sweep of the list would draw, in
		// list order, through the same IconProvider the panel uses - so
		// the FIFO cap and CIconStore's LRU are both exercised the way
		// scrolling exercises them.

		IconProvider^ kScrollIcons = gcnew IconProvider( &kIcons, 1500 );
		kIcons.ResetCounters();

		const long long iBeforeScroll = GC::GetTotalMemory( true );

		System::Diagnostics::Stopwatch^ kScrollWatch = System::Diagnostics::Stopwatch::StartNew();

		int iDrawn = 0;
		for( int i = 0; i < kViewTest->All->Length; ++i )
		{
			kScrollIcons->Get( kViewTest->All[i]->ShopImage );
			++iDrawn;
		}

		kScrollWatch->Stop();

		const long long iAfterScroll = GC::GetTotalMemory( true );

		Console::WriteLine();
		Console::WriteLine( "  scroll   : {0:N0} icon fetch(es) - every row in the list - in {1:F2} s",
			iDrawn, kScrollWatch->Elapsed.TotalSeconds );
		Console::WriteLine( "             {0:F1} us per row; a screenful is about 20 rows",
			kScrollWatch->Elapsed.TotalMilliseconds * 1000.0 / (double) Math::Max( 1, iDrawn ) );
		Console::WriteLine( "  managed  : {0:N0} KB before, {1:N0} KB after - delta {2:N0} KB",
			iBeforeScroll / 1024, iAfterScroll / 1024, ( iAfterScroll - iBeforeScroll ) / 1024 );
		Console::WriteLine( "  native   : {0} hit(s), {1} miss(es), {2} eviction(s); {3} entr(ies), {4} of {5} KB",
			kIcons.CacheHits(), kIcons.CacheMisses(), kIcons.CacheEvictions(),
			(int) kIcons.EntriesHeld(), (int)( kIcons.BytesHeld() / 1024 ), (int)( kIcons.ByteBudget() / 1024 ) );
		Console::WriteLine( "  peak ws  : {0:F1} MB",
			(double) System::Diagnostics::Process::GetCurrentProcess()->PeakWorkingSet64 / ( 1024.0 * 1024.0 ) );

		Console::WriteLine();
		Console::WriteLine( "Both bounded caches are capped by construction, so the delta above is the" );
		Console::WriteLine( "whole answer to \"growth in memory\": it is what a full sweep of 48,754 rows" );
		Console::WriteLine( "leaves behind, not what one screenful costs." );

		return 0;
	}

	//////////////////////////////////////////////////////////////////////
	// --dump exists so the decoder can be checked by ARITHMETIC and not by
	// eye. It writes the decoded surface out as raw BGRA, which anything
	// can read without an image library, so a second implementation of the
	// same block format can be compared against it pixel for pixel. Two
	// independent transcriptions of the DXT bit arithmetic agreeing is a
	// real check; "the wall looked right" is not one.
	if( nullptr != sDumpName )
	{
		SIconLookup kLookup;
		const std::string strName = msclr::interop::marshal_as<std::string>( sDumpName );
		const SDecodedImage* pImage = kIcons.Get( strName.c_str(), kLookup );

		Console::WriteLine();
		Console::WriteLine( "--- dump {0} ---", sDumpName );
		Console::WriteLine( "  outcome  : {0}", Utf8( IconOutcomeName( kLookup.eOutcome ) ) );
		Console::WriteLine( "  resolved : {0}", Utf8( kLookup.strResolved ) );

		if( false == kLookup.strDetail.empty() )
			Console::WriteLine( "  detail   : {0}", Utf8( kLookup.strDetail ) );

		if( NULL == pImage )
			return 11;

		Console::WriteLine( "  {0}x{1} {2}, {3} byte(s) of BGRA",
			pImage->iWidth, pImage->iHeight, Utf8( DdsFormatName( pImage->eFormat ) ),
			(int) pImage->vecBGRA.size() );

		String^ sOut = ( nullptr != sDumpFile ) ? sDumpFile : "icon_dump.bgra";

		array<Byte>^ abBytes = gcnew array<Byte>( (int) pImage->vecBGRA.size() );
		System::Runtime::InteropServices::Marshal::Copy(
			IntPtr( (void*) &pImage->vecBGRA[0] ), abBytes, 0, abBytes->Length );

		System::IO::File::WriteAllBytes( sOut, abBytes );
		Console::WriteLine( "  written  : {0}", System::IO::Path::GetFullPath( sOut ) );

		return 0;
	}

	//////////////////////////////////////////////////////////////////////
	// Phase 6 - the layout probe.
	//
	// Against a COPY of the save, the same discipline phase 3's round-trip
	// took: the probe has to build a real MainForm to measure one, and a
	// real MainForm opens its save read-write.

	if( bLayoutTest )
	{
		Console::WriteLine();
		Console::WriteLine( "--- the layout, measured ---" );

		// Beside the index cache, the same place phase 3's round-trip puts
		// its copy. NOT Path::GetTempPath - windows.h has already
		// #defined GetTempPath to GetTempPathW, which turns that call into
		// a member lookup for GetTempPathW on System::IO::Path.
		String^ sScratch = System::IO::Path::Combine(
			System::IO::Path::GetDirectoryName(
				msclr::interop::marshal_as<String^>( wstrCachePath ) ), "layouttest" );

		System::IO::Directory::CreateDirectory( sScratch );

		const std::wstring wstrLive = JoinPath( wstrDataDir, SaveFileName() );
		const std::wstring wstrCopy = msclr::interop::marshal_as<std::wstring>(
			System::IO::Path::Combine( sScratch, gcnew String( SaveFileName() ) ) );

		if( false == CopySaveSet( wstrLive, wstrCopy, strError ) )
		{
			Console::WriteLine( "ERROR: the save could not be copied: {0}", Utf8( strError ) );
			return 20;
		}

		CCashDb kDb;
		const ECashDbResult eOpen = kDb.Open( wstrCopy, false, strError );

		Console::WriteLine( "copy     : {0}", msclr::interop::marshal_as<String^>( wstrCopy ) );
		Console::WriteLine( "open     : {0}", DbOpenLine( eOpen, kDb, strError ) );

		if( CashDb_OK != eOpen )
			return 21;

		kDb.SetKnownItems( kCatalog.vecItems );

		Application::EnableVisualStyles();
		Application::SetCompatibleTextRenderingDefault( false );

		int iTotal = 0;

		// Both views, because the toggle rebuilds both lists and the
		// technical strings are the LONGER ones - if anything is going to
		// be clipped it is those.
		for( int iPass = 0; iPass < 2; ++iPass )
		{
			const bool bPassTechnical = ( 1 == iPass );

			MainForm^ kForm = gcnew MainForm( &kDb, &kIcons, &kCatalog, &kLabelStore, bPassTechnical );

			static const int s_aiSizes[][2] = { { 900, 560 }, { 1920, 1080 } };

			for( size_t u = 0; u != sizeof( s_aiSizes ) / sizeof( s_aiSizes[0] ); ++u )
			{
				int iRows		= 0;
				int iCanvas		= 0;
				int iReached	= 0;

				Console::WriteLine();
				Console::WriteLine( "  {0}x{1}, technical {2}:",
					s_aiSizes[u][0], s_aiSizes[u][1], bPassTechnical ? "on" : "off" );

				const int iComplaints = kForm->MeasureLayout(
					s_aiSizes[u][0], s_aiSizes[u][1], iRows, iCanvas, iReached );

				iTotal += iComplaints;

				Console::WriteLine( "    {0}", ( 0 == iComplaints )
					? "no overlapping, clipped or collapsed control anywhere in the tree"
					: String::Format( "{0} complaint(s) above", iComplaints ) );

				Console::WriteLine( "    grid: {0:N0} row(s), {1:N0}-pixel canvas, scrolled to the end reaches {2:N0}   {3}",
					iRows, iCanvas, iReached,
					( iReached >= iCanvas ) ? "-> the last row is reachable"
											: "*** THE LAST ROW CANNOT BE REACHED ***" );
			}

			kForm->Close();
		}

		kDb.Close();

		//////////////////////////////////////////////////////////////////
		// And the one claim the layout passes cannot make: that the
		// toggle SURVIVES A RESTART. The menu item's handler is one call
		// to WriteSettingBool and the editor's startup is one call to
		// ReadSettingBool, so this exercises both against the real ini and
		// puts back whatever was there.

		const std::wstring wstrIni = DefaultSettingsPath();

		const bool bWas = ReadSettingBool( wstrIni, L"technical", false );

		WriteSettingBool( wstrIni, L"technical", true );
		const bool bReadTrue = ReadSettingBool( wstrIni, L"technical", false );

		WriteSettingBool( wstrIni, L"technical", false );
		const bool bReadFalse = ReadSettingBool( wstrIni, L"technical", true );

		WriteSettingBool( wstrIni, L"technical", bWas );

		Console::WriteLine();
		Console::WriteLine( "settings : {0}", msclr::interop::marshal_as<String^>( wstrIni ) );
		Console::WriteLine( "           wrote on -> read {0}; wrote off -> read {1}   {2}",
			bReadTrue ? "on" : "off", bReadFalse ? "on" : "off",
			( bReadTrue && false == bReadFalse )
				? "-> the toggle survives a restart"
				: "*** THE TOGGLE DOES NOT PERSIST ***" );
		Console::WriteLine( "           restored to {0}, the value this run found", bWas ? "on" : "off" );

		if( false == bReadTrue || bReadFalse )
			++iTotal;

		Console::WriteLine();
		Console::WriteLine( "{0} complaint(s) in total across both sizes and both views.", iTotal );
		Console::WriteLine( "The live save was not opened: this ran against the copy above." );

		return ( 0 == iTotal ) ? 0 : 22;
	}

	//////////////////////////////////////////////////////////////////////
	// Phase 4 - the catalog editor, and the DEFAULT action of the tool.
	//
	// Everything below this block is phase 2's icon wall and its census,
	// which is now reached with --wall (or --no-window / --decode-all, both
	// of which are census switches). The wall is kept because it is the
	// evidence phase 2's exit test rests on and re-running it is the only
	// way to re-check the decoder against a changed archive set.

	if( false == bWall && false == bNoWindow && false == bDecodeAll )
	{
		const std::wstring wstrSavePath = ( nullptr != sDbPath )
			? msclr::interop::marshal_as<std::wstring>( System::IO::Path::GetFullPath( sDbPath ) )
			: JoinPath( wstrDataDir, SaveFileName() );

		Console::WriteLine();
		Console::WriteLine( "--- the catalog editor ---" );
		Console::WriteLine( "save     : {0}", msclr::interop::marshal_as<String^>( wstrSavePath ) );

		// Read-WRITE, and the lock check inside Open is what makes that
		// safe: the tool refuses to open a save the client still holds, so
		// there is no window in which both processes write.
		CCashDb kDb;
		const ECashDbResult eOpen = kDb.Open( wstrSavePath, false, strError );

		Console::WriteLine( "open     : {0}", DbOpenLine( eOpen, kDb, strError ) );

		if( CashDb_OK != eOpen )
		{
			Console::WriteLine();
			Console::WriteLine( "Nothing was opened and nothing was changed." );

			MessageBox::Show(
				String::Format( "{0}\r\n\r\n{1}\r\n\r\nNothing was opened and nothing was changed.",
					msclr::interop::marshal_as<String^>( wstrSavePath ),
					Utf8( strError ) ),
				"X2CashShopTool", MessageBoxButtons::OK, MessageBoxIcon::Error );

			return 12;
		}

		// So an edit naming an item the client cannot resolve is refused
		// where it is typed, instead of becoming a row the shop silently
		// drops (X2OfflineCashShop.cpp:85-89).
		kDb.SetKnownItems( kCatalog.vecItems );

		if( bShowItems )
		{
			PrintItems( kCatalog );
			PrintCategories( kCatalog );
		}

		Console::WriteLine();
		Console::WriteLine( "opening the catalog editor - every write is echoed here as well as to the" );
		Console::WriteLine( "status bar, so the trail survives the window being closed." );
		Console::WriteLine();

		Application::EnableVisualStyles();
		Application::SetCompatibleTextRenderingDefault( false );
		// bTechnical: --technical wins for this run; otherwise whatever
		// the last session left in X2CashShopTool.ini. The setting is
		// deliberately NOT in ItemIndex.db, which is a cache that gets
		// thrown away whenever an archive changes.
		const bool bStartTechnical = bTechnical
			|| ReadSettingBool( DefaultSettingsPath(), L"technical", false );

		Application::Run( gcnew MainForm( &kDb, &kIcons, &kCatalog, &kLabelStore, bStartTechnical ) );

		// Checkpoints the WAL into the main file, so the save is left the
		// way the client leaves it.
		kDb.Close();

		Console::WriteLine();
		Console::WriteLine( "peak working set : {0:F1} MB",
			(double) System::Diagnostics::Process::GetCurrentProcess()->PeakWorkingSet64 / ( 1024.0 * 1024.0 ) );

		return 0;
	}

	//////////////////////////////////////////////////////////////////////
	// The census the exit test asks for: how many items resolve to a real
	// file versus the fallback. Locator lookups only - no payload is read,
	// so this is the same question the client's IsValidFile answers
	// (X2Lib/X2SlotItem.cpp:257) and it costs nothing to ask for all
	// 48,754 of them.

	int iResolved	= 0;
	int iNoName		= 0;
	int iNotFound	= 0;

	System::Collections::Generic::HashSet<String^>^ kDistinct =
		gcnew System::Collections::Generic::HashSet<String^>( StringComparer::OrdinalIgnoreCase );

	for( size_t u = 0; u != kCatalog.vecItems.size(); ++u )
	{
		const SItemRow& kRow = kCatalog.vecItems[u];

		if( kRow.strShopImage.empty() )
		{
			++iNoName;
			continue;
		}

		if( kIcons.HasImage( kRow.strShopImage.c_str() ) )
		{
			++iResolved;
			kDistinct->Add( Utf8( kRow.strShopImage ) );
		}
		else
		{
			++iNotFound;
		}
	}

	Console::WriteLine();
	Console::WriteLine( "--- icon resolution across the whole catalog ---" );
	Console::WriteLine( "  {0} item(s) total", (int) kCatalog.vecItems.size() );
	Console::WriteLine( "  {0} resolve to a real file  ({1} distinct image(s) - many items share one icon)",
		iResolved, kDistinct->Count );
	Console::WriteLine( "  {0} fall back: m_ShopImage is empty", iNoName );
	Console::WriteLine( "  {0} fall back: m_ShopImage names a file no archive holds", iNotFound );
	Console::WriteLine( "  {0} would show HQ_Shop_Ui_Noimage.dds in the client, and will here",
		iNoName + iNotFound );

	//////////////////////////////////////////////////////////////////////
	// The wall. Two blocks: a sampler that hunts down at least a few of
	// every decode outcome, then an even sweep of the catalog.

	System::Collections::Generic::List<IconTile^>^ kTiles =
		gcnew System::Collections::Generic::List<IconTile^>();

	array<int>^	aiFormatSeen	= gcnew array<int>( (int) DdsFormat_Count );
	array<int>^	aiOutcomeSeen	= gcnew array<int>( (int) IconOutcome_Count );
	int			iRawContainers	= 0;
	int			iFailures		= 0;

	const int	SAMPLER_PER_KIND	= 6;
	const int	SAMPLER_SCAN_LIMIT	= 4000;
	const int	WALL_TOTAL			= 240;

	// One list of tiles per kind, so the wall can be laid out in labelled
	// groups instead of a soup in which a wrong DXT5 alpha is invisible.
	const int	KIND_COUNT = (int) DdsFormat_Count + (int) IconOutcome_Count;

	array<System::Collections::Generic::List<IconTile^>^>^ akGroups =
		gcnew array<System::Collections::Generic::List<IconTile^>^>( KIND_COUNT );
	for( int i = 0; i < KIND_COUNT; ++i )
		akGroups[i] = gcnew System::Collections::Generic::List<IconTile^>();

	array<String^>^ asGroupLabel = gcnew array<String^>( KIND_COUNT );
	for( int i = 0; i < (int) DdsFormat_Count; ++i )
		asGroupLabel[i] = String::Format( "decoded - {0}", Utf8( DdsFormatName( (EDdsFormat) i ) ) );
	for( int i = 0; i < (int) IconOutcome_Count; ++i )
		asGroupLabel[(int) DdsFormat_Count + i] = Utf8( IconOutcomeName( (EIconOutcome) i ) );

	CToolStopwatch kDecodeWatch;
	int iDecodeCalls = 0;

	// Pass 1 - the sampler.
	for( size_t u = 0; u != kCatalog.vecItems.size() && (int) u < SAMPLER_SCAN_LIMIT; ++u )
	{
		const SItemRow& kRow = kCatalog.vecItems[u];

		SIconLookup kLookup;
		const SDecodedImage* pImage = kIcons.Get( kRow.strShopImage.c_str(), kLookup );
		++iDecodeCalls;

		const int iGroup = ( IconOutcome_Decoded == kLookup.eOutcome )
			? (int) kLookup.eFormat
			: (int) DdsFormat_Count + (int) kLookup.eOutcome;

		if( akGroups[iGroup]->Count >= SAMPLER_PER_KIND )
			continue;

		IconTile^ kTile = gcnew IconTile();
		kTile->Image		= ToBitmap( pImage );
		kTile->Caption		= kRow.iItemID.ToString();
		kTile->IsFallback	= ( IconOutcome_Decoded != kLookup.eOutcome );
		kTile->IsFailure	= ( IconOutcome_Failed == kLookup.eOutcome );
		kTile->Detail		= String::Format( "{0}\n{1}\nm_ShopImage: {2}\ndecoded: {3} ({4})\n{5}",
			kRow.iItemID, Utf8( kRow.strName ), Utf8( kRow.strShopImage ),
			Utf8( kLookup.strResolved ), Utf8( DdsFormatName( kLookup.eFormat ) ),
			Utf8( IconOutcomeName( kLookup.eOutcome ) ) );

		if( IconOutcome_FallbackUndecodable == kLookup.eOutcome && DdsError_NotDds == kLookup.eError )
		{
			std::vector<char>	vecRaw;
			std::string			strRawError;

			if( kIcons.ReadRaw( kRow.strShopImage.c_str(), vecRaw, strRawError ) )
			{
				Drawing::Bitmap^ kRawBitmap = FromRawBytes( vecRaw );
				if( nullptr != kRawBitmap )
				{
					kTile->Image	= kRawBitmap;
					kTile->Detail	= kTile->Detail + "\n(not a DDS - shown through GDI+)";
					++iRawContainers;
				}
			}
		}

		akGroups[iGroup]->Add( kTile );
	}

	// Pass 2 - an even sweep, filling the wall out to WALL_TOTAL.
	{
		int iRemaining = WALL_TOTAL;
		for( int i = 0; i < KIND_COUNT; ++i )
			iRemaining -= akGroups[i]->Count;

		const int iCount = (int) kCatalog.vecItems.size();
		for( int i = 0; i < iRemaining && iCount > 0; ++i )
		{
			const int iIndex = (int)( (__int64) i * iCount / ( ( iRemaining > 0 ) ? iRemaining : 1 ) );
			const SItemRow& kRow = kCatalog.vecItems[iIndex];

			SIconLookup kLookup;
			const SDecodedImage* pImage = kIcons.Get( kRow.strShopImage.c_str(), kLookup );
			++iDecodeCalls;

			const int iGroup = ( IconOutcome_Decoded == kLookup.eOutcome )
				? (int) kLookup.eFormat
				: (int) DdsFormat_Count + (int) kLookup.eOutcome;

			IconTile^ kTile = gcnew IconTile();
			kTile->Image		= ToBitmap( pImage );
			kTile->Caption		= kRow.iItemID.ToString();
			kTile->IsFallback	= ( IconOutcome_Decoded != kLookup.eOutcome );
			kTile->IsFailure	= ( IconOutcome_Failed == kLookup.eOutcome );
			kTile->Detail		= String::Format( "{0}\n{1}\nm_ShopImage: {2}\ndecoded: {3} ({4})\n{5}",
				kRow.iItemID, Utf8( kRow.strName ), Utf8( kRow.strShopImage ),
				Utf8( kLookup.strResolved ), Utf8( DdsFormatName( kLookup.eFormat ) ),
				Utf8( IconOutcomeName( kLookup.eOutcome ) ) );

			akGroups[iGroup]->Add( kTile );
		}
	}

	const double dWallSeconds = kDecodeWatch.Seconds();

	// Flatten into the wall, labelling the first tile of each non-empty
	// group, and count what the wall actually holds.
	for( int i = 0; i < KIND_COUNT; ++i )
	{
		if( 0 == akGroups[i]->Count )
			continue;

		akGroups[i][0]->GroupLabel = String::Format( "{0}  ({1})", asGroupLabel[i], akGroups[i]->Count );

		for( int j = 0; j < akGroups[i]->Count; ++j )
		{
			if( i < (int) DdsFormat_Count )
				++aiFormatSeen[i];
			else
				++aiOutcomeSeen[i - (int) DdsFormat_Count];

			if( akGroups[i][j]->IsFailure )
				++iFailures;

			kTiles->Add( akGroups[i][j] );
		}
	}

	Console::WriteLine();
	Console::WriteLine( "--- the wall: {0} tile(s), {1} Get() call(s), {2:F2} s ---",
		kTiles->Count, iDecodeCalls, dWallSeconds );

	for( int i = 1; i < (int) DdsFormat_Count; ++i )
	{
		if( aiFormatSeen[i] > 0 )
			Console::WriteLine( "  {0} decoded {1}", Pad( Utf8( DdsFormatName( (EDdsFormat) i ) ), 14 ), aiFormatSeen[i] );
	}

	for( int i = 1; i < (int) IconOutcome_Count; ++i )
	{
		if( aiOutcomeSeen[i] > 0 )
			Console::WriteLine( "  {0} {1}", Pad( Utf8( IconOutcomeName( (EIconOutcome) i ) ), 30 ), aiOutcomeSeen[i] );
	}

	if( iRawContainers > 0 )
		Console::WriteLine( "  {0} shown through GDI+ (a real image under a .dds name, not a DDS)", iRawContainers );

	Console::WriteLine( "  LRU: {0} hit(s), {1} miss(es), {2} eviction(s), {3} entr(ies) holding {4} KB of {5} KB",
		kIcons.CacheHits(), kIcons.CacheMisses(), kIcons.CacheEvictions(),
		(int) kIcons.EntriesHeld(), (int)( kIcons.BytesHeld() / 1024 ), (int)( kIcons.ByteBudget() / 1024 ) );

	//////////////////////////////////////////////////////////////////////
	// The full sweep, on request: decode every distinct image the catalog
	// names. This is the measurement that says the decoder handles the
	// whole shipped set rather than the 240 tiles that happened to be on
	// screen - and it is also the only thing that exercises eviction.

	if( bDecodeAll )
	{
		Console::WriteLine();
		Console::WriteLine( "--- decoding every distinct image the catalog names ---" );

		kIcons.ResetCounters();

		array<int>^	aiAllFormats	= gcnew array<int>( (int) DdsFormat_Count );
		array<int>^	aiAllOutcomes	= gcnew array<int>( (int) IconOutcome_Count );

		System::Collections::Generic::List<String^>^ kProblems =
			gcnew System::Collections::Generic::List<String^>();

		System::Collections::Generic::HashSet<String^>^ kDone =
			gcnew System::Collections::Generic::HashSet<String^>( StringComparer::OrdinalIgnoreCase );

		CToolStopwatch kAllWatch;

		for( size_t u = 0; u != kCatalog.vecItems.size(); ++u )
		{
			const SItemRow& kRow = kCatalog.vecItems[u];
			if( kRow.strShopImage.empty() )
				continue;

			if( false == kDone->Add( Utf8( kRow.strShopImage ) ) )
				continue;

			SIconLookup kLookup;
			kIcons.Get( kRow.strShopImage.c_str(), kLookup );

			if( IconOutcome_Decoded == kLookup.eOutcome )
			{
				++aiAllFormats[(int) kLookup.eFormat];
			}
			else
			{
				++aiAllOutcomes[(int) kLookup.eOutcome];

				if( IconOutcome_FallbackNotFound != kLookup.eOutcome && kProblems->Count < 20 )
				{
					kProblems->Add( String::Format( "{0} - {1}: {2}",
						Utf8( kRow.strShopImage ), Utf8( IconOutcomeName( kLookup.eOutcome ) ),
						Utf8( kLookup.strDetail ) ) );
				}
			}
		}

		const double dAllSeconds = kAllWatch.Seconds();

		Console::WriteLine( "  {0} distinct name(s), {1:F2} s", kDone->Count, dAllSeconds );

		for( int i = 1; i < (int) DdsFormat_Count; ++i )
		{
			if( aiAllFormats[i] > 0 )
				Console::WriteLine( "  {0} {1}", Pad( Utf8( DdsFormatName( (EDdsFormat) i ) ), 14 ), aiAllFormats[i] );
		}

		for( int i = 1; i < (int) IconOutcome_Count; ++i )
		{
			if( aiAllOutcomes[i] > 0 )
				Console::WriteLine( "  {0} {1}", Pad( Utf8( IconOutcomeName( (EIconOutcome) i ) ), 30 ), aiAllOutcomes[i] );
		}

		if( kProblems->Count > 0 )
		{
			Console::WriteLine( "  the ones that are there but did not decode as a DDS:" );
			for( int i = 0; i < kProblems->Count; ++i )
				Console::WriteLine( "    {0}", kProblems[i] );
		}

		Console::WriteLine( "  LRU: {0} hit(s), {1} miss(es), {2} eviction(s), {3} entr(ies) holding {4} KB of {5} KB",
			kIcons.CacheHits(), kIcons.CacheMisses(), kIcons.CacheEvictions(),
			(int) kIcons.EntriesHeld(), (int)( kIcons.BytesHeld() / 1024 ), (int)( kIcons.ByteBudget() / 1024 ) );
	}

	if( bShowItems )
	{
		PrintItems( kCatalog );
		PrintCategories( kCatalog );
	}

	Console::WriteLine();
	Console::WriteLine( "peak working set : {0:F1} MB",
		(double) System::Diagnostics::Process::GetCurrentProcess()->PeakWorkingSet64 / ( 1024.0 * 1024.0 ) );
	Console::WriteLine( "switches : --rebuild  --items  --wall  --decode-all  --no-window  --dump <name> [outfile]" );
	Console::WriteLine( "           --db  --db-test [--live]  --db-path <file>  --picker-test" );
	Console::WriteLine( "           --labels  --technical  --layout-test" );

	if( bNoWindow )
		return 0;

	//////////////////////////////////////////////////////////////////////
	// The window. Every tile is already a managed Bitmap, so nothing
	// below this line touches the icon store or its borrowed pointers.

	String^ sSummary = String::Format(
		"{0} item(s) in the catalog - {1} resolve to a real file, {2} fall back "
		"({3} with no m_ShopImage, {4} naming a file no archive holds)\r\n"
		"wall: {5} tile(s) in labelled groups, {6} failure(s). "
		"Tiles are drawn over a checkerboard, so any pixel whose alpha decoded wrongly shows as a hard square.\r\n"
		"A gold border means the fallback (HQ_Shop_Ui_Noimage.dds) was used. Hover a tile for its item, name, image and format.",
		(int) kCatalog.vecItems.size(), iResolved, iNoName + iNotFound, iNoName, iNotFound,
		kTiles->Count, iFailures );

	Console::WriteLine();
	Console::WriteLine( "opening the icon wall - close it to exit (--no-window skips it)" );

	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault( false );
	Application::Run( gcnew IconWallForm( kTiles, sSummary ) );

	return 0;
}
