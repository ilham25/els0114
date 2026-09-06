//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 3 - els_db.sql, read and written.
//////////////////////////////////////////////////////////////////////////
#include "CashDb.h"

#ifdef _MANAGED
#error "This TU compiled managed. /clr leaked in - see CppBuild.targets:733-738"
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <algorithm>

#include "sqlite3.h"

namespace
{
	// X2Lib/Offline/X2OfflineDB.h:399. Not read from that header on purpose:
	// nothing in this tool compiles against X2Lib, and pulling in one header
	// from it would drag the whole client's define set behind it.
	const int	REQUIRED_USER_VERSION	= 11;

	const wchar_t	SEPARATOR	= (wchar_t) 92;		// backslash; see KomArchive.cpp

	// The three files that make up a WAL-mode save. Copying only the first
	// once produced a snapshot reading schema v6 while the live DB was at
	// v7, which is the reason this array exists rather than a single name.
	const wchar_t* const	WAL_SUFFIX[] = { L"", L"-wal", L"-shm" };
	const size_t			WAL_SUFFIX_COUNT = sizeof( WAL_SUFFIX ) / sizeof( WAL_SUFFIX[0] );

	std::wstring ParentDirectory( const std::wstring& wstrPath )
	{
		const size_t uSlash = wstrPath.find_last_of( L"\\/" );
		if( std::wstring::npos == uSlash )
			return std::wstring( L"." );

		if( 0 == uSlash )
			return wstrPath.substr( 0, 1 );

		return wstrPath.substr( 0, uSlash );
	}

	std::wstring FileNameOf( const std::wstring& wstrPath )
	{
		const size_t uSlash = wstrPath.find_last_of( L"\\/" );
		if( std::wstring::npos == uSlash )
			return wstrPath;

		return wstrPath.substr( uSlash + 1 );
	}

	// Another process holding the file. SQLite opens a database with
	// FILE_SHARE_READ | FILE_SHARE_WRITE, so an exclusive CreateFileW fails
	// with ERROR_SHARING_VIOLATION exactly when someone else has it open -
	// which, in the game directory, means the client is running.
	//
	// There is a window between this check and the open below in which the
	// client could start. It is not closed, and it does not need to be: the
	// point is to catch the ordinary "I forgot the game is up" case before
	// a backup is taken across the client's own write, not to hold a lock.
	bool IsHeldByAnotherProcess( const std::wstring& wstrPath, bool& bExists )
	{
		bExists = ( INVALID_FILE_ATTRIBUTES != ::GetFileAttributesW( wstrPath.c_str() ) );
		if( false == bExists )
			return false;

		const HANDLE hFile = ::CreateFileW( wstrPath.c_str(), GENERIC_READ | GENERIC_WRITE,
			0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

		if( INVALID_HANDLE_VALUE != hFile )
		{
			::CloseHandle( hFile );
			return false;
		}

		return ( ERROR_SHARING_VIOLATION == ::GetLastError() );
	}

	std::string SqliteError( sqlite3* pDb, const char* pszWhat )
	{
		std::string strOut( ( NULL != pszWhat ) ? pszWhat : "sqlite" );
		strOut += ": ";
		strOut += ( NULL != pDb ) ? sqlite3_errmsg( pDb ) : "no connection";
		return strOut;
	}

	std::string WinError( const wchar_t* pszWhat, const std::wstring& wstrPath, DWORD dwError )
	{
		char szBuffer[512];
		::_snprintf_s( szBuffer, sizeof( szBuffer ), _TRUNCATE, "%S %S: win32 error %lu",
			pszWhat, wstrPath.c_str(), (unsigned long) dwError );
		return std::string( szBuffer );
	}

	std::string IntToStr( int iValue )
	{
		char szBuffer[16];
		::sprintf_s( szBuffer, sizeof( szBuffer ), "%d", iValue );
		return std::string( szBuffer );
	}
}

//////////////////////////////////////////////////////////////////////////

const wchar_t* SaveFileName()
{
	return L"els_db.sql";
}

const char* CashDbResultName( ECashDbResult eResult )
{
	switch( eResult )
	{
	case CashDb_OK:				return "ok";
	case CashDb_NoFile:			return "no els_db.sql here";
	case CashDb_Locked:			return "another process holds the save";
	case CashDb_NotASave:		return "no cash_product table - not an offline save";
	case CashDb_WrongSchema:	return "unexpected PRAGMA user_version";
	case CashDb_SqliteError:	return "sqlite error";
	default:					break;
	}

	return "unknown";
}

int CCashDb::RequiredUserVersion()
{
	return REQUIRED_USER_VERSION;
}

//////////////////////////////////////////////////////////////////////////

std::wstring DefaultBackupDir( const std::wstring& wstrDbPath )
{
	const std::wstring wstrDir = JoinPath( ParentDirectory( wstrDbPath ), L"db_backup" );

	// Already there in every real install; created rather than failed on,
	// because a missing backup directory must never be the reason a backup
	// does not happen.
	::CreateDirectoryW( wstrDir.c_str(), NULL );

	return wstrDir;
}

std::wstring MakeBackupLabel()
{
	SYSTEMTIME kNow;
	::GetLocalTime( &kNow );

	wchar_t wszBuffer[64];
	::swprintf_s( wszBuffer, 64, L"cashtool-%04u%02u%02u-%02u%02u%02u",
		(unsigned) kNow.wYear, (unsigned) kNow.wMonth, (unsigned) kNow.wDay,
		(unsigned) kNow.wHour, (unsigned) kNow.wMinute, (unsigned) kNow.wSecond );

	return std::wstring( wszBuffer );
}

bool BackupSaveSet( const std::wstring& wstrDbPath, const std::wstring& wstrBackupDir,
					std::wstring& wstrLabel,
					std::vector<std::wstring>& vecWritten, std::string& strError )
{
	vecWritten.clear();

	const std::wstring wstrLeaf = FileNameOf( wstrDbPath );

	// A backup NEVER overwrites another backup.
	//
	// MakeBackupLabel is second-granular, and two writing connections within
	// one second is not hypothetical - the phase 3 round-trip did exactly
	// that and silently replaced its own pre-edit snapshot with a mid-edit
	// one, so the file named ".bak-pre-" held a state that was not "pre"
	// anything. A backup that can be overwritten by a later, worse backup is
	// worse than no backup, because it still looks like one.
	//
	// So the label is uniqued against what is already on disk before a byte
	// is copied, and the copies themselves are bFailIfExists.
	std::wstring wstrTry = wstrLabel;

	for( int iAttempt = 2; iAttempt < 1000; ++iAttempt )
	{
		bool bTaken = false;

		for( size_t u = 0; u != WAL_SUFFIX_COUNT && false == bTaken; ++u )
		{
			const std::wstring wstrDst = JoinPath( wstrBackupDir,
				wstrLeaf + WAL_SUFFIX[u] + L".bak-pre-" + wstrTry );

			bTaken = ( INVALID_FILE_ATTRIBUTES != ::GetFileAttributesW( wstrDst.c_str() ) );
		}

		if( false == bTaken )
			break;

		wchar_t wszSuffix[16];
		::swprintf_s( wszSuffix, 16, L"-%d", iAttempt );
		wstrTry = wstrLabel + wszSuffix;
	}

	wstrLabel = wstrTry;

	for( size_t u = 0; u != WAL_SUFFIX_COUNT; ++u )
	{
		const std::wstring wstrSrc = wstrDbPath + WAL_SUFFIX[u];

		if( INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW( wstrSrc.c_str() ) )
			continue;		///< -wal and -shm are absent on a cleanly closed save

		const std::wstring wstrDst = JoinPath( wstrBackupDir,
			wstrLeaf + WAL_SUFFIX[u] + L".bak-pre-" + wstrLabel );

		if( FALSE == ::CopyFileW( wstrSrc.c_str(), wstrDst.c_str(), TRUE ) )
		{
			strError = WinError( L"cannot back up", wstrSrc, ::GetLastError() );
			return false;
		}

		vecWritten.push_back( wstrDst );
	}

	if( vecWritten.empty() )
	{
		strError = "nothing was backed up - the save file is not there";
		return false;
	}

	return true;
}

bool CopySaveSet( const std::wstring& wstrSrcDb, const std::wstring& wstrDstDb, std::string& strError )
{
	for( size_t u = 0; u != WAL_SUFFIX_COUNT; ++u )
	{
		const std::wstring wstrSrc = wstrSrcDb + WAL_SUFFIX[u];
		const std::wstring wstrDst = wstrDstDb + WAL_SUFFIX[u];

		if( INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW( wstrSrc.c_str() ) )
		{
			// A stale -wal or -shm left over from an earlier copy would be
			// read as this database's, which is worse than not having one.
			::DeleteFileW( wstrDst.c_str() );
			continue;
		}

		if( FALSE == ::CopyFileW( wstrSrc.c_str(), wstrDst.c_str(), FALSE ) )
		{
			strError = WinError( L"cannot copy", wstrSrc, ::GetLastError() );
			return false;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

CCashDb::CCashDb()
: m_pDb( NULL )
, m_bReadOnly( true )
, m_iUserVersion( 0 )
, m_bBackupTaken( false )
{
}

CCashDb::~CCashDb()
{
	Close();
}

ECashDbResult CCashDb::Open( const std::wstring& wstrPath, bool bReadOnly, std::string& strError )
{
	Close();

	strError.clear();

	m_wstrPath		= wstrPath;
	m_bReadOnly		= bReadOnly;
	m_iUserVersion	= 0;
	m_bBackupTaken	= false;
	m_wstrBackupLabel.clear();
	m_vecBackupFiles.clear();

	bool bExists = false;
	if( IsHeldByAnotherProcess( wstrPath, bExists ) )
	{
		strError = "the save is open in another process - close the game (X2_offline.exe) first";
		return CashDb_Locked;
	}

	if( false == bExists )
	{
		strError = "els_db.sql is not in this directory";
		return CashDb_NoFile;
	}

	// SQLITE_OPEN_CREATE is deliberately absent: a mistyped directory must
	// produce an error, never an empty new save file beside the real one.
	const int iFlags = bReadOnly ? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE;

	const std::string strUtf8 = NarrowPath( wstrPath );

	if( SQLITE_OK != sqlite3_open_v2( strUtf8.c_str(), &m_pDb, iFlags, NULL ) || NULL == m_pDb )
	{
		strError = SqliteError( m_pDb, "sqlite3_open_v2" );
		Close();
		return CashDb_SqliteError;
	}

	sqlite3_busy_timeout( m_pDb, 5000 );

	// journal_mode is NOT touched. It is WAL, it is a persistent property of
	// the file, and the client expects to find it that way.
	//
	// synchronous is the opposite of the index cache's: that one is a pure
	// cache rebuildable in half a second, this one is the only copy of the
	// character.
	if( false == bReadOnly )
	{
		std::string strIgnored;
		Exec( "PRAGMA synchronous=FULL;", strIgnored );
	}

	if( false == ReadUserVersion( m_iUserVersion, strError ) )
	{
		Close();
		return CashDb_SqliteError;
	}

	if( false == HasTable( "cash_product" ) )
	{
		strError = "this database has no cash_product table";

		// A refused open leaves the file exactly as it found it, checkpoint
		// included - hence the read-only flag before Close(), which is what
		// Checkpoint() tests.
		m_bReadOnly = true;
		Close();
		return CashDb_NotASave;
	}

	// Fail loud. X2OfflineDB migrates forward only, so a save at some later
	// version has columns and rungs this build knows nothing about, and one
	// at an earlier version has not been through the migrations that made
	// cash_product what this tool assumes.
	if( REQUIRED_USER_VERSION != m_iUserVersion )
	{
		strError = "PRAGMA user_version is " + IntToStr( m_iUserVersion )
			+ ", this tool only writes version " + IntToStr( REQUIRED_USER_VERSION )
			+ " saves. Run the offline client once to migrate it, or use a build of"
			  " the tool that matches.";

		const int iFound = m_iUserVersion;
		m_bReadOnly = true;				///< see the note above; no checkpoint on a refusal
		Close();
		m_iUserVersion = iFound;		///< kept for the caller's message
		return CashDb_WrongSchema;
	}

	return CashDb_OK;
}

void CCashDb::Close()
{
	if( NULL == m_pDb )
		return;

	Checkpoint();

	sqlite3_close( m_pDb );
	m_pDb = NULL;
}

void CCashDb::Checkpoint()
{
	if( NULL == m_pDb || m_bReadOnly )
		return;

	// TRUNCATE folds the WAL back into the main file and empties it, which
	// is the state the client leaves the save in. It can legitimately fail
	// with SQLITE_BUSY if a reader is mid-transaction; there is nothing to
	// do about that but leave the WAL where it is, which is safe.
	std::string strIgnored;
	Exec( "PRAGMA wal_checkpoint(TRUNCATE);", strIgnored );
}

bool CCashDb::Exec( const char* pszSql, std::string& strError )
{
	if( NULL == m_pDb )
	{
		strError = "the save is not open";
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

bool CCashDb::ReadUserVersion( int& iOut, std::string& strError ) const
{
	iOut = 0;

	sqlite3_stmt* pStmt = NULL;
	if( SQLITE_OK != sqlite3_prepare_v2( m_pDb, "PRAGMA user_version;", -1, &pStmt, NULL ) )
	{
		strError = SqliteError( m_pDb, "PRAGMA user_version" );
		return false;
	}

	if( SQLITE_ROW == sqlite3_step( pStmt ) )
		iOut = sqlite3_column_int( pStmt, 0 );

	sqlite3_finalize( pStmt );
	return true;
}

bool CCashDb::HasTable( const char* pszName ) const
{
	sqlite3_stmt* pStmt = NULL;
	if( SQLITE_OK != sqlite3_prepare_v2( m_pDb,
		"SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = ?;", -1, &pStmt, NULL ) )
	{
		return false;
	}

	sqlite3_bind_text( pStmt, 1, pszName, -1, SQLITE_STATIC );

	const bool bFound = ( SQLITE_ROW == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );
	return bFound;
}

//////////////////////////////////////////////////////////////////////////
// Reads

bool CCashDb::LoadProducts( std::vector<SCashProductRow>& vecOut, std::string& strError ) const
{
	vecOut.clear();

	if( NULL == m_pDb )
	{
		strError = "the save is not open";
		return false;
	}

	sqlite3_stmt* pStmt = NULL;
	if( SQLITE_OK != sqlite3_prepare_v2( m_pDb,
		"SELECT product_no, item_id, category, quantity, price, is_event"
		" FROM cash_product ORDER BY product_no;", -1, &pStmt, NULL ) )
	{
		strError = SqliteError( m_pDb, "SELECT cash_product" );
		return false;
	}

	while( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		SCashProductRow kRow;
		kRow.iProductNo		= sqlite3_column_int( pStmt, 0 );
		kRow.iItemID		= sqlite3_column_int( pStmt, 1 );
		kRow.iCategoryNo	= sqlite3_column_int( pStmt, 2 );
		kRow.iQuantity		= sqlite3_column_int( pStmt, 3 );
		kRow.iPrice			= sqlite3_column_int( pStmt, 4 );
		kRow.iIsEvent		= sqlite3_column_int( pStmt, 5 );

		vecOut.push_back( kRow );
	}

	sqlite3_finalize( pStmt );
	return true;
}

bool CCashDb::FindProduct( int iProductNo, SCashProductRow& kOut, std::string& strError ) const
{
	kOut = SCashProductRow();

	if( NULL == m_pDb )
	{
		strError = "the save is not open";
		return false;
	}

	sqlite3_stmt* pStmt = NULL;
	if( SQLITE_OK != sqlite3_prepare_v2( m_pDb,
		"SELECT product_no, item_id, category, quantity, price, is_event"
		" FROM cash_product WHERE product_no = ?;", -1, &pStmt, NULL ) )
	{
		strError = SqliteError( m_pDb, "SELECT one cash_product" );
		return false;
	}

	sqlite3_bind_int( pStmt, 1, iProductNo );

	bool bFound = false;
	if( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		kOut.iProductNo		= sqlite3_column_int( pStmt, 0 );
		kOut.iItemID		= sqlite3_column_int( pStmt, 1 );
		kOut.iCategoryNo	= sqlite3_column_int( pStmt, 2 );
		kOut.iQuantity		= sqlite3_column_int( pStmt, 3 );
		kOut.iPrice			= sqlite3_column_int( pStmt, 4 );
		kOut.iIsEvent		= sqlite3_column_int( pStmt, 5 );
		bFound = true;
	}
	else
	{
		strError = "no product " + IntToStr( iProductNo );
	}

	sqlite3_finalize( pStmt );
	return bFound;
}

bool CCashDb::GetWallet( int& iOut, std::string& strError ) const
{
	iOut = 0;

	if( NULL == m_pDb )
	{
		strError = "the save is not open";
		return false;
	}

	sqlite3_stmt* pStmt = NULL;
	if( SQLITE_OK != sqlite3_prepare_v2( m_pDb,
		"SELECT value FROM settings WHERE name = 'cash_start';", -1, &pStmt, NULL ) )
	{
		strError = SqliteError( m_pDb, "SELECT cash_start" );
		return false;
	}

	bool bFound = false;
	if( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		iOut = sqlite3_column_int( pStmt, 0 );
		bFound = true;
	}
	else
	{
		// CX2OfflineDB::GetWallet falls back to DEFAULT_CASH_BALANCE rather
		// than showing 0, so a missing row is not an error - but the tool
		// says so instead of inventing the number.
		strError = "no cash_start row - the client will fall back to its default";
	}

	sqlite3_finalize( pStmt );
	return bFound;
}

bool CCashDb::NextProductNo( int& iOut, std::string& strError ) const
{
	iOut = 0;

	if( NULL == m_pDb )
	{
		strError = "the save is not open";
		return false;
	}

	sqlite3_stmt* pStmt = NULL;
	if( SQLITE_OK != sqlite3_prepare_v2( m_pDb,
		"SELECT COALESCE( MAX( product_no ), 0 ) FROM cash_product;", -1, &pStmt, NULL ) )
	{
		strError = SqliteError( m_pDb, "MAX( product_no )" );
		return false;
	}

	if( SQLITE_ROW == sqlite3_step( pStmt ) )
		iOut = sqlite3_column_int( pStmt, 0 ) + 1;

	sqlite3_finalize( pStmt );
	return ( iOut > 0 );
}

int CCashDb::ProductCount( std::string& strError ) const
{
	if( NULL == m_pDb )
	{
		strError = "the save is not open";
		return -1;
	}

	sqlite3_stmt* pStmt = NULL;
	if( SQLITE_OK != sqlite3_prepare_v2( m_pDb, "SELECT COUNT(*) FROM cash_product;", -1, &pStmt, NULL ) )
	{
		strError = SqliteError( m_pDb, "COUNT cash_product" );
		return -1;
	}

	int iCount = -1;
	if( SQLITE_ROW == sqlite3_step( pStmt ) )
		iCount = sqlite3_column_int( pStmt, 0 );

	sqlite3_finalize( pStmt );
	return iCount;
}

//////////////////////////////////////////////////////////////////////////
// Validation

void CCashDb::SetKnownItems( const std::vector<SItemRow>& vecItems )
{
	m_vecKnownItems.clear();
	m_vecKnownItems.reserve( vecItems.size() );

	for( size_t u = 0; u != vecItems.size(); ++u )
		m_vecKnownItems.push_back( vecItems[u].iItemID );

	// The catalog arrives sorted, but sorting a already-sorted vector costs
	// nothing and the binary search below is wrong in silence if it is not.
	std::sort( m_vecKnownItems.begin(), m_vecKnownItems.end() );
}

bool CCashDb::KnowsItem( int iItemID ) const
{
	return std::binary_search( m_vecKnownItems.begin(), m_vecKnownItems.end(), iItemID );
}

bool CCashDb::Validate( const SCashProductRow& kRow, bool bIsInsert, std::string& strError ) const
{
	strError.clear();

	if( false == bIsInsert && kRow.iProductNo <= 0 )
	{
		strError = "product_no must be positive";
		return false;
	}

	if( kRow.iItemID <= 0 )
	{
		strError = "item_id must be positive";
		return false;
	}

	// Enforced, not clamped - see the comment on CASH_FIELD_MAX.
	if( kRow.iCategoryNo < CASH_FIELD_MIN || kRow.iCategoryNo > CASH_FIELD_MAX )
	{
		strError = "category is " + IntToStr( kRow.iCategoryNo ) + "; it must be "
			+ IntToStr( (int) CASH_FIELD_MIN ) + ".." + IntToStr( (int) CASH_FIELD_MAX )
			+ ", because the client narrows it to a signed char on the wire"
			  " (X2OfflineCashShop.cpp:181)";
		return false;
	}

	if( kRow.iQuantity < CASH_FIELD_MIN || kRow.iQuantity > CASH_FIELD_MAX )
	{
		strError = "quantity is " + IntToStr( kRow.iQuantity ) + "; it must be "
			+ IntToStr( (int) CASH_FIELD_MIN ) + ".." + IntToStr( (int) CASH_FIELD_MAX )
			+ ", because the client narrows it to a signed char on the wire"
			  " (X2OfflineCashShop.cpp:185) and rewrites a non-positive one to 1 (:94)";
		return false;
	}

	// price is a plain int on both sides and is genuinely unconstrained
	// upwards. Negative is still refused: it is compared against the wallet,
	// and nothing sensible comes of that.
	if( kRow.iPrice < 0 )
	{
		strError = "price cannot be negative";
		return false;
	}

	if( 0 != kRow.iIsEvent && 1 != kRow.iIsEvent )
	{
		strError = "is_event must be 0 or 1";
		return false;
	}

	// The tool picks items FROM the client's own table, so it cannot
	// normally produce a row the client will drop - but validating beats
	// trusting that, and this is the exact check the client makes at load
	// (X2OfflineCashShop.cpp:85-89).
	if( HasKnownItems() )
	{
		if( false == KnowsItem( kRow.iItemID ) )
		{
			strError = "item " + IntToStr( kRow.iItemID )
				+ " has no client templet - the shop would silently drop this row";
			return false;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Writes

bool CCashDb::EnsureBackup( std::string& strError )
{
	if( m_bBackupTaken )
		return true;

	if( m_bReadOnly )
	{
		strError = "opened read-only; nothing to back up";
		return false;
	}

	if( NULL == m_pDb )
	{
		strError = "the save is not open";
		return false;
	}

	// Checkpoint FIRST, so the copied main file is complete on its own and
	// the copied -wal is empty. The convention in db_backup/ is to copy all
	// three either way, and this makes the three consistent with each other
	// rather than a stale main file plus a live WAL.
	Checkpoint();

	// BackupSaveSet uniques the label against db_backup/ and hands back the
	// one it actually used, so this is a request, not a decision.
	m_wstrBackupLabel = MakeBackupLabel();

	if( false == BackupSaveSet( m_wstrPath, DefaultBackupDir( m_wstrPath ),
			m_wstrBackupLabel, m_vecBackupFiles, strError ) )
	{
		m_wstrBackupLabel.clear();
		return false;
	}

	m_bBackupTaken = true;
	return true;
}

void CCashDb::AdoptBackup( const std::wstring& wstrLabel, const std::vector<std::wstring>& vecFiles )
{
	if( wstrLabel.empty() || vecFiles.empty() )
		return;

	m_wstrBackupLabel	= wstrLabel;
	m_vecBackupFiles	= vecFiles;
	m_bBackupTaken		= true;
}

bool CCashDb::BeginWrite( std::string& strError )
{
	if( m_bReadOnly )
	{
		strError = "the save was opened read-only";
		return false;
	}

	if( false == EnsureBackup( strError ) )
		return false;

	// IMMEDIATE rather than DEFERRED: the write lock is taken up front, so a
	// conflict fails here rather than at COMMIT with the work already done.
	return Exec( "BEGIN IMMEDIATE;", strError );
}

bool CCashDb::CommitWrite( std::string& strError )
{
	return Exec( "COMMIT;", strError );
}

void CCashDb::RollbackWrite()
{
	std::string strIgnored;
	Exec( "ROLLBACK;", strIgnored );
}

bool CCashDb::SetWallet( int iValue, std::string& strError )
{
	if( iValue < 0 )
	{
		strError = "the wallet cannot be negative";
		return false;
	}

	if( false == BeginWrite( strError ) )
		return false;

	sqlite3_stmt* pStmt = NULL;
	if( SQLITE_OK != sqlite3_prepare_v2( m_pDb,
		"INSERT OR REPLACE INTO settings( name, value ) VALUES( 'cash_start', ? );",
		-1, &pStmt, NULL ) )
	{
		strError = SqliteError( m_pDb, "UPDATE cash_start" );
		RollbackWrite();
		return false;
	}

	sqlite3_bind_int( pStmt, 1, iValue );

	const bool bOk = ( SQLITE_DONE == sqlite3_step( pStmt ) );
	if( false == bOk )
		strError = SqliteError( m_pDb, "UPDATE cash_start" );

	sqlite3_finalize( pStmt );

	if( false == bOk )
	{
		RollbackWrite();
		return false;
	}

	return CommitWrite( strError );
}

bool CCashDb::Insert( SCashProductRow& kRow, std::string& strError )
{
	if( false == Validate( kRow, true, strError ) )
		return false;

	if( false == BeginWrite( strError ) )
		return false;

	// Allocated inside the transaction, so MAX cannot move between reading
	// it and using it.
	if( kRow.iProductNo <= 0 )
	{
		if( false == NextProductNo( kRow.iProductNo, strError ) )
		{
			RollbackWrite();
			return false;
		}
	}

	sqlite3_stmt* pStmt = NULL;
	if( SQLITE_OK != sqlite3_prepare_v2( m_pDb,
		"INSERT INTO cash_product( product_no, item_id, category, quantity, price, is_event )"
		" VALUES( ?, ?, ?, ?, ?, ? );", -1, &pStmt, NULL ) )
	{
		strError = SqliteError( m_pDb, "INSERT cash_product" );
		RollbackWrite();
		return false;
	}

	sqlite3_bind_int( pStmt, 1, kRow.iProductNo );
	sqlite3_bind_int( pStmt, 2, kRow.iItemID );
	sqlite3_bind_int( pStmt, 3, kRow.iCategoryNo );
	sqlite3_bind_int( pStmt, 4, kRow.iQuantity );
	sqlite3_bind_int( pStmt, 5, kRow.iPrice );
	sqlite3_bind_int( pStmt, 6, kRow.iIsEvent );

	const bool bOk = ( SQLITE_DONE == sqlite3_step( pStmt ) );
	if( false == bOk )
		strError = SqliteError( m_pDb, "INSERT cash_product" );

	sqlite3_finalize( pStmt );

	if( false == bOk )
	{
		RollbackWrite();
		return false;
	}

	return CommitWrite( strError );
}

bool CCashDb::Update( const SCashProductRow& kRow, std::string& strError )
{
	if( false == Validate( kRow, false, strError ) )
		return false;

	if( false == BeginWrite( strError ) )
		return false;

	sqlite3_stmt* pStmt = NULL;
	if( SQLITE_OK != sqlite3_prepare_v2( m_pDb,
		"UPDATE cash_product SET item_id = ?, category = ?, quantity = ?, price = ?,"
		" is_event = ? WHERE product_no = ?;", -1, &pStmt, NULL ) )
	{
		strError = SqliteError( m_pDb, "UPDATE cash_product" );
		RollbackWrite();
		return false;
	}

	sqlite3_bind_int( pStmt, 1, kRow.iItemID );
	sqlite3_bind_int( pStmt, 2, kRow.iCategoryNo );
	sqlite3_bind_int( pStmt, 3, kRow.iQuantity );
	sqlite3_bind_int( pStmt, 4, kRow.iPrice );
	sqlite3_bind_int( pStmt, 5, kRow.iIsEvent );
	sqlite3_bind_int( pStmt, 6, kRow.iProductNo );

	bool bOk = ( SQLITE_DONE == sqlite3_step( pStmt ) );
	if( false == bOk )
		strError = SqliteError( m_pDb, "UPDATE cash_product" );

	sqlite3_finalize( pStmt );

	// An UPDATE that matched nothing is a silent no-op in SQL and a bug
	// here: the caller believes a product changed.
	if( bOk && 0 == sqlite3_changes( m_pDb ) )
	{
		strError = "no product " + IntToStr( kRow.iProductNo ) + " to update";
		bOk = false;
	}

	if( false == bOk )
	{
		RollbackWrite();
		return false;
	}

	return CommitWrite( strError );
}

bool CCashDb::Delete( int iProductNo, std::string& strError )
{
	if( iProductNo <= 0 )
	{
		strError = "product_no must be positive";
		return false;
	}

	if( false == BeginWrite( strError ) )
		return false;

	sqlite3_stmt* pStmt = NULL;
	if( SQLITE_OK != sqlite3_prepare_v2( m_pDb,
		"DELETE FROM cash_product WHERE product_no = ?;", -1, &pStmt, NULL ) )
	{
		strError = SqliteError( m_pDb, "DELETE cash_product" );
		RollbackWrite();
		return false;
	}

	sqlite3_bind_int( pStmt, 1, iProductNo );

	bool bOk = ( SQLITE_DONE == sqlite3_step( pStmt ) );
	if( false == bOk )
		strError = SqliteError( m_pDb, "DELETE cash_product" );

	sqlite3_finalize( pStmt );

	if( bOk && 0 == sqlite3_changes( m_pDb ) )
	{
		strError = "no product " + IntToStr( iProductNo ) + " to delete";
		bOk = false;
	}

	if( false == bOk )
	{
		RollbackWrite();
		return false;
	}

	return CommitWrite( strError );
}

//////////////////////////////////////////////////////////////////////////
// The report

void BuildCashCatalogReport( const std::vector<SCashProductRow>& vecProducts,
							 const std::vector<SItemRow>& vecItems,
							 const std::vector<SCashCategoryRow>& vecCategories,
							 SCashCatalogReport& kOut )
{
	kOut = SCashCatalogReport();

	// The item ids the client can resolve, sorted, for a binary search per
	// product. vecItems is already sorted by id; sorting a copy of just the
	// ids costs a few milliseconds on 48,754 and removes the assumption.
	std::vector<int> vecItemIDs;
	vecItemIDs.reserve( vecItems.size() );
	for( size_t u = 0; u != vecItems.size(); ++u )
		vecItemIDs.push_back( vecItems[u].iItemID );
	std::sort( vecItemIDs.begin(), vecItemIDs.end() );

	// The billing category numbers CashShopCategory.lua actually names, and
	// which tab each belongs to. A cash_product row whose category is not in
	// here reaches no tab in the game, however healthy its item is.
	std::vector<int> vecTabCategory;
	std::vector<int> vecTabIndex;
	for( size_t u = 0; u != vecCategories.size(); ++u )
	{
		vecTabCategory.push_back( vecCategories[u].iBillingCategoryNo );
		vecTabIndex.push_back( vecCategories[u].iTabIdx );
	}

	std::vector<int> vecSeenItem;		///< item ids seen once
	std::vector<int> vecDupeItem;		///< item ids seen more than once, counted once each

	kOut.iRows = (int) vecProducts.size();

	for( size_t u = 0; u != vecProducts.size(); ++u )
	{
		const SCashProductRow& kRow = vecProducts[u];

		const bool bKnownItem = std::binary_search( vecItemIDs.begin(), vecItemIDs.end(), kRow.iItemID );

		if( bKnownItem )
		{
			++kOut.iKept;
		}
		else
		{
			++kOut.iDropped;

			SCashProblemRow kProblem;
			kProblem.iProductNo		= kRow.iProductNo;
			kProblem.iItemID		= kRow.iItemID;
			kProblem.iCategoryNo	= kRow.iCategoryNo;
			kProblem.strDetail		= "no client templet";
			kOut.vecDropped.push_back( kProblem );
		}

		if( kRow.iCategoryNo < CASH_FIELD_MIN || kRow.iCategoryNo > CASH_FIELD_MAX
			|| kRow.iQuantity < CASH_FIELD_MIN || kRow.iQuantity > CASH_FIELD_MAX )
		{
			SCashProblemRow kProblem;
			kProblem.iProductNo		= kRow.iProductNo;
			kProblem.iItemID		= kRow.iItemID;
			kProblem.iCategoryNo	= kRow.iCategoryNo;
			kProblem.strDetail		= "category " + IntToStr( kRow.iCategoryNo )
				+ " / quantity " + IntToStr( kRow.iQuantity ) + " outside 1..127";
			kOut.vecOutOfRange.push_back( kProblem );
		}

		if( 0 == u || kRow.iPrice < kOut.iMinPrice )	kOut.iMinPrice = kRow.iPrice;
		if( 0 == u || kRow.iPrice > kOut.iMaxPrice )	kOut.iMaxPrice = kRow.iPrice;

		// An item sold as two products is DELIBERATE - eighteen of them are,
		// and the catalog index is keyed on product number precisely so they
		// survive (X2OfflineCashShop.cpp:100-103). Counted, never warned
		// about, and never deduplicated.
		if( std::binary_search( vecSeenItem.begin(), vecSeenItem.end(), kRow.iItemID ) )
		{
			if( false == std::binary_search( vecDupeItem.begin(), vecDupeItem.end(), kRow.iItemID ) )
			{
				vecDupeItem.insert(
					std::lower_bound( vecDupeItem.begin(), vecDupeItem.end(), kRow.iItemID ),
					kRow.iItemID );
			}
		}
		else
		{
			vecSeenItem.insert(
				std::lower_bound( vecSeenItem.begin(), vecSeenItem.end(), kRow.iItemID ),
				kRow.iItemID );
		}

		// Per-category tallies, kept ascending by category number.
		size_t uSlot = 0;
		while( uSlot != kOut.vecCategories.size()
			&& kOut.vecCategories[uSlot].iCategoryNo < kRow.iCategoryNo )
		{
			++uSlot;
		}

		if( uSlot == kOut.vecCategories.size()
			|| kOut.vecCategories[uSlot].iCategoryNo != kRow.iCategoryNo )
		{
			SCashCategoryCount kCount;
			kCount.iCategoryNo	= kRow.iCategoryNo;
			kCount.bInTabTable	= false;
			kCount.iTabIdx		= 0;

			for( size_t v = 0; v != vecTabCategory.size(); ++v )
			{
				if( vecTabCategory[v] == kRow.iCategoryNo )
				{
					kCount.bInTabTable	= true;
					kCount.iTabIdx		= vecTabIndex[v];
					break;
				}
			}

			kOut.vecCategories.insert( kOut.vecCategories.begin() + uSlot, kCount );
		}

		++kOut.vecCategories[uSlot].iRows;
		if( false == bKnownItem )
			++kOut.vecCategories[uSlot].iDropped;
	}

	kOut.iDuplicateItems = (int) vecDupeItem.size();

	// Only meaningful when the tab table was actually parsed; with none
	// given, every category reads as "not in the table" and calling them all
	// orphans would be a lie.
	if( false == vecTabCategory.empty() )
	{
		for( size_t u = 0; u != kOut.vecCategories.size(); ++u )
		{
			if( false == kOut.vecCategories[u].bInTabTable )
			{
				++kOut.iOrphanCategories;
				kOut.iOrphanRows += kOut.vecCategories[u].iRows;
			}
		}
	}
}
