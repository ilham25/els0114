//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 1 - SQLite index cache implementation.
//////////////////////////////////////////////////////////////////////////
#include "IndexCache.h"

#ifdef _MANAGED
#error "This TU compiled managed. /clr leaked in - see CppBuild.targets:733-738"
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#include "sqlite3.h"

namespace
{
	const char* const	META_KOM_PATH		= "kom_path";
	const char* const	META_KOM_SIZE		= "kom_size";
	const char* const	META_KOM_MTIME		= "kom_mtime";
	const char* const	META_EXTRACTOR		= "extractor_version";

	// The icon locator's own stamp, kept apart from the four above.
	const char* const	META_ICON_VERSION	= "icon_locator_version";
	const char* const	META_ICON_DIR		= "icon_dir";
	const char* const	META_ICON_COUNT		= "icon_count";

	std::string I64ToString( __int64 iValue )
	{
		char szBuffer[32];
		::sprintf_s( szBuffer, sizeof( szBuffer ), "%I64d", iValue );
		return std::string( szBuffer );
	}

	std::string IntToString( int iValue )
	{
		char szBuffer[16];
		::sprintf_s( szBuffer, sizeof( szBuffer ), "%d", iValue );
		return std::string( szBuffer );
	}

	void BindText( sqlite3_stmt* pStmt, int iColumn, const std::string& strValue )
	{
		// SQLITE_TRANSIENT: sqlite copies. The rows being bound are in a
		// vector that outlives the step anyway, but a cache write is not
		// the place to be clever about a copy of a 20-byte name.
		sqlite3_bind_text( pStmt, iColumn, strValue.c_str(), (int) strValue.size(), SQLITE_TRANSIENT );
	}

	std::string ColumnText( sqlite3_stmt* pStmt, int iColumn )
	{
		const unsigned char* pszText = sqlite3_column_text( pStmt, iColumn );
		if( NULL == pszText )
			return std::string();

		return std::string( (const char*) pszText, (size_t) sqlite3_column_bytes( pStmt, iColumn ) );
	}
}

//////////////////////////////////////////////////////////////////////////

// The plan asked for this "next to the tool exe - not in the game
// directory, where a stray .db beside els_db.sql is the sort of thing
// someone deletes at 2am". Those two turn out to be the same place: the
// tool is deployed INTO the game directory and run from there, so
// next-to-the-exe put X2CashShopIndex.db right beside the save file.
//
// The reason given is the load-bearing half of that sentence, so the cache
// goes under %LOCALAPPDATA% instead, and the tool prints the full path on
// every run so it is never hidden. Falls back to next-to-the-exe only if
// the profile directory cannot be resolved at all.
std::wstring DefaultCachePath()
{
	const wchar_t* const	pszFileName	= L"ItemIndex.db";
	const wchar_t* const	pszFolder	= L"X2CashShopTool";

	wchar_t wszLocalAppData[MAX_PATH];
	wszLocalAppData[0] = L'\0';

	const DWORD dwChars = ::GetEnvironmentVariableW( L"LOCALAPPDATA", wszLocalAppData, MAX_PATH );
	if( dwChars > 0 && dwChars < MAX_PATH )
	{
		std::wstring wstrDir( wszLocalAppData );
		wstrDir += L"\\";
		wstrDir += pszFolder;

		// Fine if it already exists; the open below is what actually has
		// to succeed, and it reports its own failure.
		::CreateDirectoryW( wstrDir.c_str(), NULL );

		return wstrDir + L"\\" + pszFileName;
	}

	wchar_t wszModule[MAX_PATH];
	wszModule[0] = L'\0';

	const DWORD dwLength = ::GetModuleFileNameW( NULL, wszModule, MAX_PATH );
	if( 0 == dwLength || dwLength >= MAX_PATH )
		return std::wstring( pszFileName );

	std::wstring wstrPath( wszModule );

	const size_t uSlash = wstrPath.find_last_of( L"\\/" );
	if( std::wstring::npos == uSlash )
		return std::wstring( pszFileName );

	wstrPath.erase( uSlash + 1 );
	wstrPath += pszFileName;
	return wstrPath;
}

//////////////////////////////////////////////////////////////////////////

CIndexCache::CIndexCache()
: m_pDb( NULL )
{
}

CIndexCache::~CIndexCache()
{
	Close();
}

bool CIndexCache::Open( const std::wstring& wstrPath, std::string& strError )
{
	Close();

	m_wstrPath = wstrPath;

	// sqlite3_open16 takes UTF-16 directly, so the path survives whatever
	// the exe happens to sit under.
	if( SQLITE_OK != sqlite3_open16( wstrPath.c_str(), &m_pDb ) || NULL == m_pDb )
	{
		strError = ( NULL != m_pDb ) ? sqlite3_errmsg( m_pDb ) : "sqlite3_open16 failed";
		Close();
		return false;
	}

	// This file is a pure cache, rebuildable from the archives in seconds,
	// so durability is worth nothing here and costs a great deal across
	// 43,000 inserts. The real save file (els_db.sql, phase 3) gets the
	// opposite treatment.
	std::string strIgnored;
	Exec( "PRAGMA journal_mode=MEMORY;", strIgnored );
	Exec( "PRAGMA synchronous=OFF;", strIgnored );

	return CreateSchema( strError );
}

void CIndexCache::Close()
{
	if( NULL != m_pDb )
	{
		sqlite3_close( m_pDb );
		m_pDb = NULL;
	}
}

bool CIndexCache::Exec( const char* pszSql, std::string& strError )
{
	if( NULL == m_pDb )
	{
		strError = "cache is not open";
		return false;
	}

	char* pszMessage = NULL;
	if( SQLITE_OK != sqlite3_exec( m_pDb, pszSql, NULL, NULL, &pszMessage ) )
	{
		strError = ( NULL != pszMessage ) ? pszMessage : "sqlite3_exec failed";
		if( NULL != pszMessage )
			sqlite3_free( pszMessage );
		return false;
	}

	return true;
}

bool CIndexCache::CreateSchema( std::string& strError )
{
	// Phase 1 populated the first three; phase 2 adds the last two. Every
	// statement is IF NOT EXISTS, so a cache written by phase 1 gains the
	// icon tables on its next open without losing the 48,754 items it
	// already holds - which is why the item extractor version did NOT have
	// to be bumped for this phase.
	//
	// The icon tables are a LOCATOR, never the bytes and never decoded
	// bitmaps (plan, "The item index is a cache"). 64x64 BGRA is 16 KB; a
	// cache of 28,000 of those would be 460 MB of something rebuildable in
	// milliseconds from the archive it is sitting next to.
	static const char* const s_pszSchema =
		"CREATE TABLE IF NOT EXISTS index_meta( key TEXT PRIMARY KEY, value TEXT );"
		"CREATE TABLE IF NOT EXISTS item( item_id INTEGER PRIMARY KEY, name TEXT,"
		" shop_image TEXT, item_type INTEGER, item_grade INTEGER,"
		" is_fashion INTEGER, equip_position INTEGER );"
		"CREATE INDEX IF NOT EXISTS ix_item_name ON item( name );"
		"CREATE TABLE IF NOT EXISTS cash_category( tab_idx INTEGER, real_id INTEGER,"
		" sub_ordinal INTEGER, cssc_enum INTEGER, billing_category_no INTEGER );"
		"CREATE TABLE IF NOT EXISTS icon( name TEXT PRIMARY KEY, kom TEXT,"
		" offset INTEGER, comp_size INTEGER, real_size INTEGER );"
		"CREATE TABLE IF NOT EXISTS icon_kom( kom TEXT PRIMARY KEY, size INTEGER,"
		" mtime INTEGER );";

	return Exec( s_pszSchema, strError );
}

bool CIndexCache::ReadMeta( const char* pszKey, std::string& strValue ) const
{
	strValue.clear();

	if( NULL == m_pDb )
		return false;

	sqlite3_stmt* pStmt = NULL;
	if( SQLITE_OK != sqlite3_prepare_v2( m_pDb, "SELECT value FROM index_meta WHERE key = ?;", -1, &pStmt, NULL ) )
		return false;

	sqlite3_bind_text( pStmt, 1, pszKey, -1, SQLITE_STATIC );

	bool bFound = false;
	if( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		strValue = ColumnText( pStmt, 0 );
		bFound = true;
	}

	sqlite3_finalize( pStmt );
	return bFound;
}

bool CIndexCache::WriteMeta( const char* pszKey, const std::string& strValue, std::string& strError )
{
	sqlite3_stmt* pStmt = NULL;
	if( SQLITE_OK != sqlite3_prepare_v2( m_pDb,
		"INSERT OR REPLACE INTO index_meta( key, value ) VALUES( ?, ? );", -1, &pStmt, NULL ) )
	{
		strError = sqlite3_errmsg( m_pDb );
		return false;
	}

	sqlite3_bind_text( pStmt, 1, pszKey, -1, SQLITE_STATIC );
	BindText( pStmt, 2, strValue );

	const bool bOk = ( SQLITE_DONE == sqlite3_step( pStmt ) );
	if( false == bOk )
		strError = sqlite3_errmsg( m_pDb );

	sqlite3_finalize( pStmt );
	return bOk;
}

bool CIndexCache::IsCurrent( const std::wstring& wstrArchivePath, std::string& strReason ) const
{
	if( NULL == m_pDb )
	{
		strReason = "cache is not open";
		return false;
	}

	std::string strVersion;
	if( false == ReadMeta( META_EXTRACTOR, strVersion ) )
	{
		strReason = "no index has been built yet";
		return false;
	}

	if( strVersion != IntToString( ItemExtractorVersion() ) )
	{
		strReason = "built by extractor version " + strVersion
			+ ", this build is " + IntToString( ItemExtractorVersion() );
		return false;
	}

	std::string strPath;
	ReadMeta( META_KOM_PATH, strPath );
	if( UpperAscii( strPath ) != UpperAscii( NarrowPath( wstrArchivePath ) ) )
	{
		strReason = "built from a different archive path";
		return false;
	}

	__int64 iSize	= 0;
	__int64 iMTime	= 0;
	if( false == GetFileStamp( wstrArchivePath, &iSize, &iMTime ) )
	{
		strReason = "the archive it was built from is gone";
		return false;
	}

	std::string strSize;
	std::string strMTime;
	ReadMeta( META_KOM_SIZE, strSize );
	ReadMeta( META_KOM_MTIME, strMTime );

	if( strSize != I64ToString( iSize ) )
	{
		strReason = "the archive's size changed";
		return false;
	}

	if( strMTime != I64ToString( iMTime ) )
	{
		strReason = "the archive was modified";
		return false;
	}

	return true;
}

bool CIndexCache::Store( const SExtractResult& kResult, const std::wstring& wstrArchivePath, std::string& strError )
{
	if( NULL == m_pDb )
	{
		strError = "cache is not open";
		return false;
	}

	// One transaction around all 43,000 inserts. Per-statement commits here
	// would mean 43,000 separate transactions.
	if( false == Exec( "BEGIN;", strError ) )
		return false;

	// Only the keys this half owns are cleared. A blanket DELETE FROM
	// index_meta here would silently drop the icon locator's stamp and
	// force phase 2's index to rebuild every time the catalog did.
	if( false == Exec( "DELETE FROM item; DELETE FROM cash_category;"
			" DELETE FROM index_meta WHERE key IN"
			" ( 'kom_path', 'kom_size', 'kom_mtime', 'extractor_version' );", strError ) )
	{
		std::string strIgnored;
		Exec( "ROLLBACK;", strIgnored );
		return false;
	}

	{
		sqlite3_stmt* pStmt = NULL;
		if( SQLITE_OK != sqlite3_prepare_v2( m_pDb,
			"INSERT INTO item( item_id, name, shop_image, item_type, item_grade,"
			" is_fashion, equip_position ) VALUES( ?, ?, ?, ?, ?, ?, ? );", -1, &pStmt, NULL ) )
		{
			strError = sqlite3_errmsg( m_pDb );
			std::string strIgnored;
			Exec( "ROLLBACK;", strIgnored );
			return false;
		}

		for( size_t u = 0; u != kResult.vecItems.size(); ++u )
		{
			const SItemRow& kRow = kResult.vecItems[u];

			sqlite3_bind_int( pStmt, 1, kRow.iItemID );
			BindText( pStmt, 2, kRow.strName );
			BindText( pStmt, 3, kRow.strShopImage );
			sqlite3_bind_int( pStmt, 4, kRow.iItemType );
			sqlite3_bind_int( pStmt, 5, kRow.iItemGrade );
			sqlite3_bind_int( pStmt, 6, kRow.iIsFashion );
			sqlite3_bind_int( pStmt, 7, kRow.iEquipPosition );

			if( SQLITE_DONE != sqlite3_step( pStmt ) )
			{
				strError = sqlite3_errmsg( m_pDb );
				sqlite3_finalize( pStmt );
				std::string strIgnored;
				Exec( "ROLLBACK;", strIgnored );
				return false;
			}

			sqlite3_reset( pStmt );
		}

		sqlite3_finalize( pStmt );
	}

	{
		sqlite3_stmt* pStmt = NULL;
		if( SQLITE_OK != sqlite3_prepare_v2( m_pDb,
			"INSERT INTO cash_category( tab_idx, real_id, sub_ordinal, cssc_enum,"
			" billing_category_no ) VALUES( ?, ?, ?, ?, ? );", -1, &pStmt, NULL ) )
		{
			strError = sqlite3_errmsg( m_pDb );
			std::string strIgnored;
			Exec( "ROLLBACK;", strIgnored );
			return false;
		}

		for( size_t u = 0; u != kResult.vecCategories.size(); ++u )
		{
			const SCashCategoryRow& kRow = kResult.vecCategories[u];

			sqlite3_bind_int( pStmt, 1, kRow.iTabIdx );
			sqlite3_bind_int( pStmt, 2, kRow.iRealID );
			sqlite3_bind_int( pStmt, 3, kRow.iSubOrdinal );
			sqlite3_bind_int( pStmt, 4, kRow.iCsscEnum );
			sqlite3_bind_int( pStmt, 5, kRow.iBillingCategoryNo );

			if( SQLITE_DONE != sqlite3_step( pStmt ) )
			{
				strError = sqlite3_errmsg( m_pDb );
				sqlite3_finalize( pStmt );
				std::string strIgnored;
				Exec( "ROLLBACK;", strIgnored );
				return false;
			}

			sqlite3_reset( pStmt );
		}

		sqlite3_finalize( pStmt );
	}

	__int64 iSize	= 0;
	__int64 iMTime	= 0;
	GetFileStamp( wstrArchivePath, &iSize, &iMTime );

	// The stamp goes in LAST, so a write interrupted half way leaves a
	// cache with no extractor_version - which reads as "no index has been
	// built yet" and rebuilds, rather than as a valid but partial index.
	bool bOk = true;
	if( bOk )	bOk = WriteMeta( META_KOM_PATH,	NarrowPath( wstrArchivePath ),	strError );
	if( bOk )	bOk = WriteMeta( META_KOM_SIZE,	I64ToString( iSize ),			strError );
	if( bOk )	bOk = WriteMeta( META_KOM_MTIME,	I64ToString( iMTime ),		strError );
	if( bOk )	bOk = WriteMeta( META_EXTRACTOR,	IntToString( ItemExtractorVersion() ), strError );

	if( false == bOk )
	{
		std::string strIgnored;
		Exec( "ROLLBACK;", strIgnored );
		return false;
	}

	return Exec( "COMMIT;", strError );
}

bool CIndexCache::Load( SExtractResult& kResult, std::string& strError ) const
{
	kResult = SExtractResult();

	if( NULL == m_pDb )
	{
		strError = "cache is not open";
		return false;
	}

	{
		sqlite3_stmt* pStmt = NULL;
		if( SQLITE_OK != sqlite3_prepare_v2( m_pDb,
			"SELECT item_id, name, shop_image, item_type, item_grade, is_fashion,"
			" equip_position FROM item ORDER BY item_id;", -1, &pStmt, NULL ) )
		{
			strError = sqlite3_errmsg( m_pDb );
			return false;
		}

		while( SQLITE_ROW == sqlite3_step( pStmt ) )
		{
			SItemRow kRow;
			kRow.iItemID		= sqlite3_column_int( pStmt, 0 );
			kRow.strName		= ColumnText( pStmt, 1 );
			kRow.strShopImage	= ColumnText( pStmt, 2 );
			kRow.iItemType		= sqlite3_column_int( pStmt, 3 );
			kRow.iItemGrade		= sqlite3_column_int( pStmt, 4 );
			kRow.iIsFashion		= sqlite3_column_int( pStmt, 5 );
			kRow.iEquipPosition	= sqlite3_column_int( pStmt, 6 );

			if( false == kRow.strShopImage.empty() )
				++kResult.iWithShopImage;
			if( 0 == kRow.iItemType )
				++kResult.iZeroItemType;

			kResult.vecItems.push_back( kRow );
		}

		sqlite3_finalize( pStmt );
	}

	{
		sqlite3_stmt* pStmt = NULL;
		if( SQLITE_OK != sqlite3_prepare_v2( m_pDb,
			"SELECT tab_idx, real_id, sub_ordinal, cssc_enum, billing_category_no"
			" FROM cash_category ORDER BY tab_idx, sub_ordinal;", -1, &pStmt, NULL ) )
		{
			strError = sqlite3_errmsg( m_pDb );
			return false;
		}

		while( SQLITE_ROW == sqlite3_step( pStmt ) )
		{
			SCashCategoryRow kRow;
			kRow.iTabIdx			= sqlite3_column_int( pStmt, 0 );
			kRow.iRealID			= sqlite3_column_int( pStmt, 1 );
			kRow.iSubOrdinal		= sqlite3_column_int( pStmt, 2 );
			kRow.iCsscEnum			= sqlite3_column_int( pStmt, 3 );
			kRow.iBillingCategoryNo	= sqlite3_column_int( pStmt, 4 );

			kResult.vecCategories.push_back( kRow );
		}

		sqlite3_finalize( pStmt );
	}

	if( kResult.vecItems.empty() )
	{
		strError = "the cache holds no items";
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// The icon locator.

bool CIndexCache::AreIconsCurrent( const std::wstring& wstrDir, std::string& strReason ) const
{
	if( NULL == m_pDb )
	{
		strReason = "cache is not open";
		return false;
	}

	std::string strVersion;
	if( false == ReadMeta( META_ICON_VERSION, strVersion ) )
	{
		strReason = "no icon locator has been built yet";
		return false;
	}

	if( strVersion != IntToString( IconLocatorVersion() ) )
	{
		strReason = "built by icon locator version " + strVersion
			+ ", this build is " + IntToString( IconLocatorVersion() );
		return false;
	}

	// Every archive the locator was built from, stamped. mtime + size
	// rather than a content hash: hashing a gigabyte of archives costs more
	// than re-reading the 145 manifests it is meant to save.
	sqlite3_stmt* pStmt = NULL;
	if( SQLITE_OK != sqlite3_prepare_v2( m_pDb,
		"SELECT kom, size, mtime FROM icon_kom;", -1, &pStmt, NULL ) )
	{
		strReason = "icon_kom cannot be read";
		return false;
	}

	int			iChecked	= 0;
	bool		bStale		= false;
	std::string	strStale;

	while( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		const std::string	strKom	= ColumnText( pStmt, 0 );
		const __int64		iSize	= sqlite3_column_int64( pStmt, 1 );
		const __int64		iMTime	= sqlite3_column_int64( pStmt, 2 );

		++iChecked;

		const std::wstring wstrPath = JoinPath( wstrDir, WidenPath( strKom ) );

		__int64 iNowSize	= 0;
		__int64 iNowMTime	= 0;
		if( false == GetFileStamp( wstrPath, &iNowSize, &iNowMTime ) )
		{
			bStale		= true;
			strStale	= strKom + " is gone";
			break;
		}

		if( iNowSize != iSize || iNowMTime != iMTime )
		{
			bStale		= true;
			strStale	= strKom + " changed";
			break;
		}
	}

	sqlite3_finalize( pStmt );

	if( bStale )
	{
		strReason = strStale;
		return false;
	}

	if( 0 == iChecked )
	{
		strReason = "the icon locator names no archives";
		return false;
	}

	// A 146th archive appearing, or one of the 145 having been absent when
	// the locator was built and present now, both show up as a count
	// mismatch rather than as a stale stamp - nothing above would catch it.
	int iPresent = 0;
	for( int i = 1; i <= 145; ++i )
	{
		wchar_t wszName[64];
		::swprintf_s( wszName, 64, L"data%03d.kom", i );

		const std::wstring wstrPath = JoinPath( wstrDir, wszName );

		if( GetFileStamp( wstrPath, NULL, NULL ) )
			++iPresent;
	}

	if( iPresent != iChecked )
	{
		strReason = "the game directory now holds " + IntToString( iPresent )
			+ " archive(s), the locator was built from " + IntToString( iChecked );
		return false;
	}

	return true;
}

bool CIndexCache::StoreIcons( const std::vector<SIconLocation>& vecLocations,
								const std::vector<SKomStamp>& vecStamps,
								const std::wstring& wstrDir, std::string& strError )
{
	if( NULL == m_pDb )
	{
		strError = "cache is not open";
		return false;
	}

	if( false == Exec( "BEGIN;", strError ) )
		return false;

	if( false == Exec( "DELETE FROM icon; DELETE FROM icon_kom;"
			" DELETE FROM index_meta WHERE key IN"
			" ( 'icon_locator_version', 'icon_dir', 'icon_count' );", strError ) )
	{
		std::string strIgnored;
		Exec( "ROLLBACK;", strIgnored );
		return false;
	}

	{
		sqlite3_stmt* pStmt = NULL;
		if( SQLITE_OK != sqlite3_prepare_v2( m_pDb,
			"INSERT INTO icon( name, kom, offset, comp_size, real_size )"
			" VALUES( ?, ?, ?, ?, ? );", -1, &pStmt, NULL ) )
		{
			strError = sqlite3_errmsg( m_pDb );
			std::string strIgnored;
			Exec( "ROLLBACK;", strIgnored );
			return false;
		}

		for( size_t u = 0; u != vecLocations.size(); ++u )
		{
			const SIconLocation& kRow = vecLocations[u];

			BindText( pStmt, 1, kRow.strName );
			BindText( pStmt, 2, kRow.strKom );
			sqlite3_bind_int64( pStmt, 3, kRow.iOffset );
			sqlite3_bind_int( pStmt, 4, (int) kRow.lCompSize );
			sqlite3_bind_int( pStmt, 5, (int) kRow.lStatedSize );

			if( SQLITE_DONE != sqlite3_step( pStmt ) )
			{
				strError = sqlite3_errmsg( m_pDb );
				sqlite3_finalize( pStmt );
				std::string strIgnored;
				Exec( "ROLLBACK;", strIgnored );
				return false;
			}

			sqlite3_reset( pStmt );
		}

		sqlite3_finalize( pStmt );
	}

	{
		sqlite3_stmt* pStmt = NULL;
		if( SQLITE_OK != sqlite3_prepare_v2( m_pDb,
			"INSERT INTO icon_kom( kom, size, mtime ) VALUES( ?, ?, ? );", -1, &pStmt, NULL ) )
		{
			strError = sqlite3_errmsg( m_pDb );
			std::string strIgnored;
			Exec( "ROLLBACK;", strIgnored );
			return false;
		}

		for( size_t u = 0; u != vecStamps.size(); ++u )
		{
			BindText( pStmt, 1, vecStamps[u].strKom );
			sqlite3_bind_int64( pStmt, 2, vecStamps[u].iSize );
			sqlite3_bind_int64( pStmt, 3, vecStamps[u].iMTime );

			if( SQLITE_DONE != sqlite3_step( pStmt ) )
			{
				strError = sqlite3_errmsg( m_pDb );
				sqlite3_finalize( pStmt );
				std::string strIgnored;
				Exec( "ROLLBACK;", strIgnored );
				return false;
			}

			sqlite3_reset( pStmt );
		}

		sqlite3_finalize( pStmt );
	}

	// Version last, for the same reason Store() writes it last: an
	// interrupted write leaves no version, which reads as "not built yet"
	// and rebuilds, rather than as a valid but half-written locator.
	bool bOk = true;
	if( bOk )	bOk = WriteMeta( META_ICON_DIR,		NarrowPath( wstrDir ),						strError );
	if( bOk )	bOk = WriteMeta( META_ICON_COUNT,	IntToString( (int) vecLocations.size() ),	strError );
	if( bOk )	bOk = WriteMeta( META_ICON_VERSION,	IntToString( IconLocatorVersion() ),		strError );

	if( false == bOk )
	{
		std::string strIgnored;
		Exec( "ROLLBACK;", strIgnored );
		return false;
	}

	return Exec( "COMMIT;", strError );
}

bool CIndexCache::LoadIcons( std::vector<SIconLocation>& vecLocations, std::string& strError ) const
{
	vecLocations.clear();

	if( NULL == m_pDb )
	{
		strError = "cache is not open";
		return false;
	}

	sqlite3_stmt* pStmt = NULL;
	if( SQLITE_OK != sqlite3_prepare_v2( m_pDb,
		"SELECT name, kom, offset, comp_size, real_size FROM icon;", -1, &pStmt, NULL ) )
	{
		strError = sqlite3_errmsg( m_pDb );
		return false;
	}

	while( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		SIconLocation kRow;
		kRow.strName		= ColumnText( pStmt, 0 );
		kRow.strKom			= ColumnText( pStmt, 1 );
		kRow.iOffset		= sqlite3_column_int64( pStmt, 2 );
		kRow.lCompSize		= (long) sqlite3_column_int( pStmt, 3 );
		kRow.lStatedSize	= (long) sqlite3_column_int( pStmt, 4 );

		vecLocations.push_back( kRow );
	}

	sqlite3_finalize( pStmt );

	if( vecLocations.empty() )
	{
		strError = "the cache holds no icon locations";
		return false;
	}

	return true;
}
