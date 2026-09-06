//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 2 - console entry point plus the icon
// wall that is this phase's exit test. See CASH_SHOP_TOOL_PLAN.md,
// "Phase 2 - Icons".
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
//////////////////////////////////////////////////////////////////////////
#include <msclr/marshal.h>
#include <msclr/marshal_cppstd.h>  // needed for marshal_as<std::string, String^> specifically
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

#include "IconWallForm.h"

using namespace X2CashShopTool;

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
		if( NULL == pImage || pImage->IsEmpty() )
			return nullptr;

		Drawing::Bitmap^ kBitmap = gcnew Drawing::Bitmap(
			pImage->iWidth, pImage->iHeight, Drawing::Imaging::PixelFormat::Format32bppArgb );

		Drawing::Imaging::BitmapData^ kData = kBitmap->LockBits(
			Drawing::Rectangle( 0, 0, pImage->iWidth, pImage->iHeight ),
			Drawing::Imaging::ImageLockMode::WriteOnly,
			Drawing::Imaging::PixelFormat::Format32bppArgb );

		const unsigned char*	pSrc		= &pImage->vecBGRA[0];
		const size_t			uSrcStride	= pImage->Stride();
		unsigned char*			pDst		= (unsigned char*) kData->Scan0.ToPointer();

		for( int y = 0; y < pImage->iHeight; ++y )
			::memcpy( pDst + (size_t) y * kData->Stride, pSrc + (size_t) y * uSrcStride, uSrcStride );

		kBitmap->UnlockBits( kData );
		return kBitmap;
	}

	// The one shipped shop image that is not a DDS at all (a PNG under a
	// .dds name). Reported, then shown anyway - it is a perfectly good
	// picture and the fallback would be a lie about it.
	Drawing::Bitmap^ FromRawBytes( const std::vector<char>& vecBytes )
	{
		if( vecBytes.empty() )
			return nullptr;

		try
		{
			array<Byte>^ abBytes = gcnew array<Byte>( (int) vecBytes.size() );
			System::Runtime::InteropServices::Marshal::Copy(
				IntPtr( (void*) &vecBytes[0] ), abBytes, 0, (int) vecBytes.size() );

			System::IO::MemoryStream^ kStream = gcnew System::IO::MemoryStream( abBytes, false );
			return gcnew Drawing::Bitmap( kStream );
		}
		catch( Exception^ )
		{
			return nullptr;
		}
	}

	//////////////////////////////////////////////////////////////////////

	void PrintBanner()
	{
		SProbeResult r = RunNativeProbe();

		Console::WriteLine( "X2CashShopTool - phase 2 (icons)" );
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

	String^ sDumpName = nullptr;
	String^ sDumpFile = nullptr;

	for( int i = 0; i < args->Length; ++i )
	{
		if( args[i]->Equals( "--rebuild",	StringComparison::OrdinalIgnoreCase ) )	bForceRebuild	= true;
		if( args[i]->Equals( "--items",		StringComparison::OrdinalIgnoreCase ) )	bShowItems		= true;
		if( args[i]->Equals( "--decode-all",StringComparison::OrdinalIgnoreCase ) )	bDecodeAll		= true;
		if( args[i]->Equals( "--no-window",	StringComparison::OrdinalIgnoreCase ) )	bNoWindow		= true;

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
		Console::WriteLine( "extract  : {0:F2} s in total ({1:F2} Enum, {2:F2} Item, {3:F2} ItemTrans, {4:F2} CashShopCategory)",
			kExtracted.dTotalSeconds, kExtracted.dEnumSeconds, kExtracted.dItemSeconds,
			kExtracted.dTransSeconds, kExtracted.dCategorySeconds );
		Console::WriteLine( "cache    : {0} item(s) + {1} category row(s) written in {2} ms",
			(int) kExtracted.vecItems.size(), (int) kExtracted.vecCategories.size(),
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

	Console::WriteLine( "catalog  : {0} item(s), {1} category row(s) loaded from the cache in {2} ms",
		(int) kCatalog.vecItems.size(), (int) kCatalog.vecCategories.size(),
		kLoadWatch->ElapsedMilliseconds );

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
	Console::WriteLine( "switches : --rebuild  --items  --decode-all  --no-window  --dump <name> [outfile]" );

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
