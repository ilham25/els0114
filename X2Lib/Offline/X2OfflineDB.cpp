#include "stdafx.h"
#include "X2OfflineDB.h"

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineLog.h"
#include "sqlite3.h"

CX2OfflineDB* CX2OfflineDB::ms_pInstance = NULL;

/*static*/ const char* CX2OfflineDB::UNIT_COLUMNS =
	"unit_uid, user_uid, unit_class, nickname, level, exp, ed, sp, spirit, "
	"last_pos, last_line_index, last_pos_value, cur_hp, cur_mp, hyper_gage, abil_count, "
	"reg_date, del_date, last_date";

namespace
{
	__int64 NowEpoch()
	{
		return (__int64)::_time64( NULL );
	}

	const char* const SCHEMA_V1 =
		"CREATE TABLE IF NOT EXISTS account ("
		"  user_uid      INTEGER PRIMARY KEY AUTOINCREMENT,"
		"  login_id      TEXT NOT NULL UNIQUE,"
		"  unit_slots    INTEGER NOT NULL DEFAULT 3,"
		"  cash_balance  INTEGER NOT NULL DEFAULT 0,"
		"  created_at    INTEGER NOT NULL );"

		// mirrors dbo.GUnit; soft delete follows the original's computed column:
		//   Deleted AS (case RegDate when DelDate then 0 else 1 end)
		"CREATE TABLE IF NOT EXISTS unit ("
		"  unit_uid    INTEGER PRIMARY KEY AUTOINCREMENT,"
		"  user_uid    INTEGER NOT NULL REFERENCES account(user_uid),"
		"  unit_class  INTEGER NOT NULL,"
		"  nickname    TEXT NOT NULL,"
		"  level       INTEGER NOT NULL DEFAULT 1,"
		"  exp         INTEGER NOT NULL DEFAULT 0,"
		"  ed          INTEGER NOT NULL DEFAULT 0,"
		"  sp          INTEGER NOT NULL DEFAULT 0,"
		"  spirit      INTEGER NOT NULL DEFAULT 0,"
		"  last_pos    INTEGER NOT NULL DEFAULT 20000,"
		"  reg_date    INTEGER NOT NULL,"
		"  del_date    INTEGER NOT NULL,"		// == reg_date means "not deleted"
		"  last_date   INTEGER NOT NULL );"
		"CREATE INDEX IF NOT EXISTS ix_unit_user ON unit( user_uid );"

		"CREATE TABLE IF NOT EXISTS inventory_size ("
		"  unit_uid  INTEGER NOT NULL,"
		"  category  INTEGER NOT NULL,"
		"  size      INTEGER NOT NULL,"
		"  PRIMARY KEY( unit_uid, category ) );"

		// blob_data / progress hold a serialized KSerBuffer tail on purpose:
		// several client structs have nested state that is not worth
		// normalizing when nothing else queries it. Normalize only what has to
		// be filtered or sorted on.
		"CREATE TABLE IF NOT EXISTS item ("
		"  item_uid  INTEGER PRIMARY KEY AUTOINCREMENT,"
		"  unit_uid  INTEGER NOT NULL,"
		"  category  INTEGER NOT NULL,"
		"  slot      INTEGER NOT NULL,"
		"  item_id   INTEGER NOT NULL,"
		"  count     INTEGER NOT NULL DEFAULT 1,"
		"  equipped  INTEGER NOT NULL DEFAULT 0,"
		"  blob_data BLOB );"
		"CREATE INDEX IF NOT EXISTS ix_item_unit ON item( unit_uid );"

		"CREATE TABLE IF NOT EXISTS unit_skill ("
		"  unit_uid  INTEGER NOT NULL,"
		"  skill_id  INTEGER NOT NULL,"
		"  level     INTEGER NOT NULL DEFAULT 1,"
		"  slot      INTEGER NOT NULL DEFAULT -1,"
		"  PRIMARY KEY( unit_uid, skill_id ) );"

		"CREATE TABLE IF NOT EXISTS unit_quest ("
		"  unit_uid  INTEGER NOT NULL,"
		"  quest_id  INTEGER NOT NULL,"
		"  state     INTEGER NOT NULL DEFAULT 0,"
		"  progress  BLOB,"
		"  PRIMARY KEY( unit_uid, quest_id ) );"

		"CREATE TABLE IF NOT EXISTS unit_dungeon ("
		"  unit_uid    INTEGER NOT NULL,"
		"  dungeon_id  INTEGER NOT NULL,"
		"  clear_count INTEGER NOT NULL DEFAULT 0,"
		"  best_rank   INTEGER NOT NULL DEFAULT 0,"
		"  PRIMARY KEY( unit_uid, dungeon_id ) );"
		;

	// v2 (phase 3): where in the village the character was standing. Additive
	// ALTER TABLEs so an existing v1 save is upgraded instead of wiped - both
	// default to 0, which is the same "start of line 0" the client sends for a
	// character that has never entered a field.
	const char* const SCHEMA_V2 =
		"ALTER TABLE unit ADD COLUMN last_line_index INTEGER NOT NULL DEFAULT 0;"
		"ALTER TABLE unit ADD COLUMN last_pos_value  INTEGER NOT NULL DEFAULT 0;"
		;

	// v3 (phase 3): the live gauges, from EGS_UPDATE_PLAY_STATUS_NOT. cur_hp 0
	// is the "never stored" marker, which MakeGamePlayStatus turns into full
	// health - a character logging in on 0 HP would otherwise be routed straight
	// back out of a battlefield by Handler_EGS_GET_MY_INVENTORY_ACK.
	const char* const SCHEMA_V3 =
		"ALTER TABLE unit ADD COLUMN cur_hp     INTEGER NOT NULL DEFAULT 0;"
		"ALTER TABLE unit ADD COLUMN cur_mp     INTEGER NOT NULL DEFAULT 0;"
		"ALTER TABLE unit ADD COLUMN hyper_gage INTEGER NOT NULL DEFAULT 0;"
		"ALTER TABLE unit ADD COLUMN abil_count INTEGER NOT NULL DEFAULT 0;"
		;
}

//////////////////////////////////////////////////////////////////////////

CX2OfflineDB::CX2OfflineDB()
: m_pDB( NULL )
{
}

CX2OfflineDB::~CX2OfflineDB()
{
	Close();
}

CX2OfflineDB* CX2OfflineDB::Instance()
{
	if( NULL == ms_pInstance )
		ms_pInstance = new CX2OfflineDB;

	return ms_pInstance;
}

void CX2OfflineDB::Release()
{
	if( NULL == ms_pInstance )
		return;

	delete ms_pInstance;
	ms_pInstance = NULL;
}

//////////////////////////////////////////////////////////////////////////

void CX2OfflineDB::LogError( const wchar_t* szWhat )
{
	std::wstring wstrMsg = L"<no handle>";

	if( NULL != m_pDB )
	{
		const void* pMsg = sqlite3_errmsg16( m_pDB );
		if( NULL != pMsg )
			wstrMsg = (const wchar_t*)pMsg;
	}

	CX2OfflineLog::Server( L"DB ERROR %s : %s", szWhat, wstrMsg.c_str() );
}

bool CX2OfflineDB::Exec( const char* szSQL )
{
	if( NULL == m_pDB )
		return false;

	char* szErr = NULL;
	if( SQLITE_OK != sqlite3_exec( m_pDB, szSQL, NULL, NULL, &szErr ) )
	{
		std::wstring wstrErr;
		ConvertCharToWCHAR( wstrErr, ( NULL != szErr ) ? szErr : "" );

		CX2OfflineLog::Server( L"DB ERROR exec : %s", wstrErr.c_str() );

		if( NULL != szErr )
			sqlite3_free( szErr );

		return false;
	}

	return true;
}

sqlite3_stmt* CX2OfflineDB::Prepare( const char* szSQL )
{
	if( NULL == m_pDB )
		return NULL;

	sqlite3_stmt* pStmt = NULL;
	if( SQLITE_OK != sqlite3_prepare_v2( m_pDB, szSQL, -1, &pStmt, NULL ) )
	{
		std::wstring wstrSQL;
		ConvertCharToWCHAR( wstrSQL, szSQL );

		LogError( wstrSQL.c_str() );
		return NULL;
	}

	return pStmt;
}

/*static*/ void CX2OfflineDB::BindText( sqlite3_stmt* pStmt, int iIndex, const std::wstring& wstr )
{
	// UTF-16 in, UTF-8 in the file: SQLite does the conversion, so no wide/narrow
	// convention of ours can drift. See the header comment.
	sqlite3_bind_text16( pStmt, iIndex, wstr.c_str(),
		(int)( wstr.size() * sizeof( wchar_t ) ), SQLITE_TRANSIENT );
}

/*static*/ std::wstring CX2OfflineDB::ColumnText( sqlite3_stmt* pStmt, int iCol )
{
	const void* pText = sqlite3_column_text16( pStmt, iCol );
	if( NULL == pText )
		return std::wstring();

	int nBytes = sqlite3_column_bytes16( pStmt, iCol );

	return std::wstring( (const wchar_t*)pText, (size_t)( nBytes / sizeof( wchar_t ) ) );
}

/*static*/ void CX2OfflineDB::ReadUnitRow( sqlite3_stmt* pStmt, OUT KOfflineUnitRow& kOut )
{
	kOut.m_nUnitUID		= (UidType)sqlite3_column_int64( pStmt, 0 );
	kOut.m_nUserUID		= (UidType)sqlite3_column_int64( pStmt, 1 );
	kOut.m_iUnitClass	= sqlite3_column_int( pStmt, 2 );
	kOut.m_wstrNickName	= ColumnText( pStmt, 3 );
	kOut.m_iLevel		= sqlite3_column_int( pStmt, 4 );
	kOut.m_iEXP			= sqlite3_column_int( pStmt, 5 );
	kOut.m_iED			= sqlite3_column_int( pStmt, 6 );
	kOut.m_iSP			= sqlite3_column_int( pStmt, 7 );
	kOut.m_iSpirit		= sqlite3_column_int( pStmt, 8 );
	kOut.m_iLastPos		= sqlite3_column_int( pStmt, 9 );
	kOut.m_iLastLineIndex	= sqlite3_column_int( pStmt, 10 );
	kOut.m_iLastPosValue	= sqlite3_column_int( pStmt, 11 );
	kOut.m_iCurHP		= sqlite3_column_int( pStmt, 12 );
	kOut.m_iCurMP		= sqlite3_column_int( pStmt, 13 );
	kOut.m_iHyperGage	= sqlite3_column_int( pStmt, 14 );
	kOut.m_iAbilCount	= sqlite3_column_int( pStmt, 15 );
	kOut.m_tRegDate		= (__int64)sqlite3_column_int64( pStmt, 16 );
	kOut.m_tDelDate		= (__int64)sqlite3_column_int64( pStmt, 17 );
	kOut.m_tLastDate	= (__int64)sqlite3_column_int64( pStmt, 18 );
}

/*static*/ std::wstring CX2OfflineDB::FormatDate( __int64 tEpoch )
{
	__time64_t t = (__time64_t)tEpoch;
	struct tm kTM;

	if( 0 != _localtime64_s( &kTM, &t ) )
		return std::wstring( L"2000-01-01 00:00:00" );

	wchar_t szBuff[32];
	_snwprintf( szBuff, 32, L"%04d-%02d-%02d %02d:%02d:%02d",
		kTM.tm_year + 1900, kTM.tm_mon + 1, kTM.tm_mday,
		kTM.tm_hour, kTM.tm_min, kTM.tm_sec );
	szBuff[31] = L'\0';

	return std::wstring( szBuff );
}

/*static*/ __int64 CX2OfflineDB::DelAbleDate( __int64 tDelDate )
{
	if( DELETE_WAIT_SECONDS > 0 )
		return tDelDate + (__int64)DELETE_WAIT_SECONDS;

	return tDelDate - (__int64)DELETE_CLOCK_SLACK_SECONDS;
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineDB::Open( const wchar_t* szPath )
{
	KLocker lock( m_cs );

	if( NULL != m_pDB )
		return true;

	if( SQLITE_OK != sqlite3_open16( szPath, &m_pDB ) )
	{
		LogError( L"open" );

		// sqlite3_open16 hands back a handle even on failure, so it still has
		// to be closed or the error message leaks with it.
		if( NULL != m_pDB )
		{
			sqlite3_close( m_pDB );
			m_pDB = NULL;
		}

		return false;
	}

	Exec( "PRAGMA journal_mode = WAL;" );
	Exec( "PRAGMA foreign_keys = ON;" );
	Exec( "PRAGMA synchronous = NORMAL;" );

	if( false == Migrate() )
	{
		CX2OfflineLog::Server( L"DB ERROR migration failed - offline state will not persist" );
		return false;
	}

	CX2OfflineLog::Server( L"DB       open '%s' (schema v%d, sqlite %S)",
		szPath, (int)SCHEMA_VERSION, sqlite3_libversion() );

	return true;
}

void CX2OfflineDB::Close()
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return;

	sqlite3_close( m_pDB );
	m_pDB = NULL;
}

int CX2OfflineDB::ReadSchemaVersion()
{
	// user_version is a header field, so there is no table to create first and
	// nothing to migrate the migration table itself.
	sqlite3_stmt* pStmt = Prepare( "PRAGMA user_version;" );
	if( NULL == pStmt )
		return -1;

	int iVersion = -1;
	if( SQLITE_ROW == sqlite3_step( pStmt ) )
		iVersion = sqlite3_column_int( pStmt, 0 );

	sqlite3_finalize( pStmt );
	return iVersion;
}

bool CX2OfflineDB::Migrate()
{
	int iFrom = ReadSchemaVersion();

	if( iFrom > (int)SCHEMA_VERSION )
	{
		CX2OfflineLog::Server( L"DB ERROR save is schema v%d, this build only knows v%d",
			iFrom, (int)SCHEMA_VERSION );
		return false;
	}

	if( iFrom == (int)SCHEMA_VERSION )
		return true;

	// The ladder. One rung per version; each rung is additive, so an existing
	// save is upgraded rather than wiped.
	if( iFrom < 1 )
	{
		if( false == Exec( "BEGIN;" ) )			return false;
		if( false == Exec( SCHEMA_V1 ) )		{ Exec( "ROLLBACK;" ); return false; }
		if( false == Exec( "COMMIT;" ) )		return false;

		CX2OfflineLog::Server( L"DB       schema created (v1)" );
	}

	if( iFrom < 2 )
	{
		// A brand-new save has just run SCHEMA_V1, which does not carry these
		// two columns, so the rung applies to a fresh file as well as an
		// upgrade - that is what keeps the ladder the single source of truth.
		if( false == Exec( "BEGIN;" ) )			return false;
		if( false == Exec( SCHEMA_V2 ) )		{ Exec( "ROLLBACK;" ); return false; }
		if( false == Exec( "COMMIT;" ) )		return false;

		CX2OfflineLog::Server( L"DB       schema upgraded to v2 (last field position)" );
	}

	if( iFrom < 3 )
	{
		if( false == Exec( "BEGIN;" ) )			return false;
		if( false == Exec( SCHEMA_V3 ) )		{ Exec( "ROLLBACK;" ); return false; }
		if( false == Exec( "COMMIT;" ) )		return false;

		CX2OfflineLog::Server( L"DB       schema upgraded to v3 (live HP/MP gauges)" );
	}

	char szSetVersion[64];
	_snprintf( szSetVersion, 64, "PRAGMA user_version = %d;", (int)SCHEMA_VERSION );
	szSetVersion[63] = '\0';

	return Exec( szSetVersion );
}

//////////////////////////////////////////////////////////////////////////
// account

bool CX2OfflineDB::GetOrCreateAccount( const std::wstring& wstrLoginID, OUT UidType& nUserUID, OUT int& iUnitSlots )
{
	KLocker lock( m_cs );

	nUserUID	= 0;
	iUnitSlots	= DEFAULT_UNIT_SLOTS;

	if( NULL == m_pDB )
		return false;

	{
		sqlite3_stmt* pStmt = Prepare( "SELECT user_uid, unit_slots FROM account WHERE login_id = ?1;" );
		if( NULL == pStmt )
			return false;

		BindText( pStmt, 1, wstrLoginID );

		bool bFound = false;
		if( SQLITE_ROW == sqlite3_step( pStmt ) )
		{
			nUserUID	= (UidType)sqlite3_column_int64( pStmt, 0 );
			iUnitSlots	= sqlite3_column_int( pStmt, 1 );
			bFound		= true;
		}

		sqlite3_finalize( pStmt );

		if( true == bFound )
			return true;
	}

	sqlite3_stmt* pIns = Prepare(
		"INSERT INTO account( login_id, unit_slots, cash_balance, created_at ) VALUES( ?1, ?2, 0, ?3 );" );
	if( NULL == pIns )
		return false;

	BindText( pIns, 1, wstrLoginID );
	sqlite3_bind_int(   pIns, 2, (int)DEFAULT_UNIT_SLOTS );
	sqlite3_bind_int64( pIns, 3, NowEpoch() );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pIns ) );
	if( false == bOK )
		LogError( L"insert account" );

	sqlite3_finalize( pIns );

	if( false == bOK )
		return false;

	nUserUID	= (UidType)sqlite3_last_insert_rowid( m_pDB );
	iUnitSlots	= DEFAULT_UNIT_SLOTS;

	CX2OfflineLog::Server( L"DB       new account '%s' -> userUID=%I64d", wstrLoginID.c_str(), (__int64)nUserUID );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// units

bool CX2OfflineDB::LoadUnits( UidType nUserUID, OUT std::vector< KOfflineUnitRow >& vecOut )
{
	KLocker lock( m_cs );

	vecOut.clear();

	if( NULL == m_pDB )
		return false;

	std::string strSQL = std::string( "SELECT " ) + UNIT_COLUMNS +
		" FROM unit WHERE user_uid = ?1 ORDER BY unit_uid;";

	sqlite3_stmt* pStmt = Prepare( strSQL.c_str() );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUserUID );

	while( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		KOfflineUnitRow kRow;
		ReadUnitRow( pStmt, kRow );
		vecOut.push_back( kRow );
	}

	sqlite3_finalize( pStmt );
	return true;
}

bool CX2OfflineDB::LoadUnit( UidType nUnitUID, OUT KOfflineUnitRow& kOut )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	std::string strSQL = std::string( "SELECT " ) + UNIT_COLUMNS + " FROM unit WHERE unit_uid = ?1;";

	sqlite3_stmt* pStmt = Prepare( strSQL.c_str() );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );

	bool bFound = false;
	if( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		ReadUnitRow( pStmt, kOut );
		bFound = true;
	}

	sqlite3_finalize( pStmt );
	return bFound;
}

bool CX2OfflineDB::IsNickNameTaken( const std::wstring& wstrNickName )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	// Only live characters hold a name. The real DB nulls dbo.GUnitNickName on
	// delete, which frees the name the same way; keeping the row's nickname
	// intact is what lets the client still draw the deleted slot.
	sqlite3_stmt* pStmt = Prepare(
		"SELECT 1 FROM unit WHERE nickname = ?1 AND del_date = reg_date LIMIT 1;" );
	if( NULL == pStmt )
		return false;

	BindText( pStmt, 1, wstrNickName );

	bool bTaken = ( SQLITE_ROW == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );
	return bTaken;
}

int CX2OfflineDB::CountLiveUnits( UidType nUserUID )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return 0;

	sqlite3_stmt* pStmt = Prepare(
		"SELECT COUNT(*) FROM unit WHERE user_uid = ?1 AND del_date = reg_date;" );
	if( NULL == pStmt )
		return 0;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUserUID );

	int iCount = 0;
	if( SQLITE_ROW == sqlite3_step( pStmt ) )
		iCount = sqlite3_column_int( pStmt, 0 );

	sqlite3_finalize( pStmt );
	return iCount;
}

bool CX2OfflineDB::CreateUnit( UidType nUserUID, int iUnitClass, const std::wstring& wstrNickName,
							   OUT KOfflineUnitRow& kOut )
{
	UidType nNewUID = 0;

	{
		KLocker lock( m_cs );

		if( NULL == m_pDB )
			return false;

		__int64 tNow = NowEpoch();

		sqlite3_stmt* pStmt = Prepare(
			"INSERT INTO unit( user_uid, unit_class, nickname, level, exp, ed, sp, spirit, "
			"                  last_pos, reg_date, del_date, last_date ) "
			"VALUES( ?1, ?2, ?3, 1, 0, 0, 1, 0, ?4, ?5, ?5, ?5 );" );
		if( NULL == pStmt )
			return false;

		// level 1 / exp 0 / ed 0 / SPoint 1 / LastPosition 20000, and
		// del_date == reg_date == last_date, exactly as dbo.gup_create_unit
		// inserts into dbo.GUnit.
		sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUserUID );
		sqlite3_bind_int(   pStmt, 2, iUnitClass );
		BindText(           pStmt, 3, wstrNickName );
		sqlite3_bind_int(   pStmt, 4, (int)DEFAULT_LAST_POSITION );
		sqlite3_bind_int64( pStmt, 5, tNow );

		bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );
		if( false == bOK )
			LogError( L"insert unit" );

		sqlite3_finalize( pStmt );

		if( false == bOK )
			return false;

		nNewUID = (UidType)sqlite3_last_insert_rowid( m_pDB );
	}

	SeedInventorySizes( nNewUID );

	return LoadUnit( nNewUID, kOut );
}

bool CX2OfflineDB::SoftDeleteUnit( UidType nUnitUID, __int64 tDelDate )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	// Never DELETE FROM here. del_date != reg_date is the whole soft-delete
	// mechanism, inherited from dbo.GUnit's computed Deleted column.
	sqlite3_stmt* pStmt = Prepare(
		"UPDATE unit SET del_date = ?2 WHERE unit_uid = ?1 AND del_date = reg_date;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int64( pStmt, 2, tDelDate );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) ) && ( 1 == sqlite3_changes( m_pDB ) );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::RestoreUnit( UidType nUnitUID )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"UPDATE unit SET del_date = reg_date WHERE unit_uid = ?1 AND del_date <> reg_date;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) ) && ( 1 == sqlite3_changes( m_pDB ) );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::FinalDeleteUnit( UidType nUnitUID )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	// The one place a row really goes away. The child tables carry no foreign
	// key back to unit (they are written by later phases and indexed by hand),
	// so clear them explicitly rather than leaning on ON DELETE CASCADE.
	static const char* const szTables[] =
		{ "inventory_size", "item", "unit_skill", "unit_quest", "unit_dungeon" };

	if( false == Exec( "BEGIN;" ) )
		return false;

	bool bOK = true;

	for( int i = 0; i < (int)( sizeof( szTables ) / sizeof( szTables[0] ) ) && true == bOK; ++i )
	{
		std::string strSQL = std::string( "DELETE FROM " ) + szTables[i] + " WHERE unit_uid = ?1;";

		sqlite3_stmt* pStmt = Prepare( strSQL.c_str() );
		if( NULL == pStmt )
		{
			bOK = false;
			break;
		}

		sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
		bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );
		sqlite3_finalize( pStmt );
	}

	if( true == bOK )
	{
		sqlite3_stmt* pStmt = Prepare( "DELETE FROM unit WHERE unit_uid = ?1;" );
		if( NULL == pStmt )
		{
			bOK = false;
		}
		else
		{
			sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
			bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) ) && ( 1 == sqlite3_changes( m_pDB ) );
			sqlite3_finalize( pStmt );
		}
	}

	Exec( true == bOK ? "COMMIT;" : "ROLLBACK;" );

	return bOK;
}

bool CX2OfflineDB::TouchLastDate( UidType nUnitUID )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare( "UPDATE unit SET last_date = ?2 WHERE unit_uid = ?1;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int64( pStmt, 2, NowEpoch() );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::SaveLastPosition( UidType nUnitUID, int iMapID )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare( "UPDATE unit SET last_pos = ?2 WHERE unit_uid = ?1;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int(   pStmt, 2, iMapID );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::SaveLastFieldPos( UidType nUnitUID, int iLineIndex, int iPosValue )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"UPDATE unit SET last_line_index = ?2, last_pos_value = ?3 WHERE unit_uid = ?1;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int(   pStmt, 2, iLineIndex );
	sqlite3_bind_int(   pStmt, 3, iPosValue );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::SaveGamePlayStatus( UidType nUnitUID, int iCurHP, int iCurMP,
									   int iHyperGage, int iAbilCount )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"UPDATE unit SET cur_hp = ?2, cur_mp = ?3, hyper_gage = ?4, abil_count = ?5 "
		"WHERE unit_uid = ?1;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int(   pStmt, 2, iCurHP );
	sqlite3_bind_int(   pStmt, 3, iCurMP );
	sqlite3_bind_int(   pStmt, 4, iHyperGage );
	sqlite3_bind_int(   pStmt, 5, iAbilCount );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::SaveProgress( UidType nUnitUID, int iLevel, int iEXP, int iED )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"UPDATE unit SET level = ?2, exp = ?3, ed = ?4 WHERE unit_uid = ?1;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int(   pStmt, 2, iLevel );
	sqlite3_bind_int(   pStmt, 3, iEXP );
	sqlite3_bind_int(   pStmt, 4, iED );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::AddDungeonClear( UidType nUnitUID, int iDungeonID, int iRank )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	// One statement: SQLite's upsert reaches back to 3.24, and this build ships
	// 3.53. best_rank keeps the *lowest* number, because rank 1 is S and the
	// client counts down (CX2Dungeon::RANK_TYPE).
	sqlite3_stmt* pStmt = Prepare(
		"INSERT INTO unit_dungeon ( unit_uid, dungeon_id, clear_count, best_rank ) "
		"VALUES ( ?1, ?2, 1, ?3 ) "
		"ON CONFLICT( unit_uid, dungeon_id ) DO UPDATE SET "
		"  clear_count = clear_count + 1,"
		"  best_rank   = CASE WHEN best_rank = 0 OR ?3 < best_rank THEN ?3 ELSE best_rank END;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int(   pStmt, 2, iDungeonID );
	sqlite3_bind_int(   pStmt, 3, iRank );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );
	return bOK;
}

//////////////////////////////////////////////////////////////////////////
// inventory

/*static*/ int CX2OfflineDB::BaseSlotSize( int iCategory )
{
	// Mirrors KInventory::GetBaseSlotSize for SERV_COUNTRY_US. The server-side
	// flags it keys off (SERV_REFORM_INVENTORY_*, SERV_PET_SYSTEM, ...) live in
	// ServerDefine.h, which X2Lib does see, so the same #ifdefs resolve the
	// same way on both sides.
	switch( iCategory )
	{
	case CX2Inventory::ST_NONE:
		return 0;

	case CX2Inventory::ST_BANK:
		return 8;

#ifdef SERV_SHARING_BANK_TEST
	case CX2Inventory::ST_SHARE_BANK:
		return 48;
#endif SERV_SHARING_BANK_TEST

	case CX2Inventory::ST_E_EQUIP:
		return CX2Unit::NESI_END;

	case CX2Inventory::ST_E_SKILL:
		return CX2Unit::NSSI_END;

	case CX2Inventory::ST_E_QUICK_SLOT:
		//{{ Iruha : 2026-08-27 // All 6 consumable quick slots open by default
#ifdef SERV_IRUHADEV_QUICK_SLOT_FULL_FREE
		return 6;
#else
		return 3;
#endif SERV_IRUHADEV_QUICK_SLOT_FULL_FREE
		//}}

	case CX2Inventory::ST_AVARTA:
#ifdef SERV_REFORM_INVENTORY_TEST
#ifdef SERV_REFORM_INVENTORY_AVATAR_CHARTER_FOUR
		return 224;
#else SERV_REFORM_INVENTORY_AVATAR_CHARTER_FOUR
		return 168;
#endif SERV_REFORM_INVENTORY_AVATAR_CHARTER_FOUR
#else
		return 160;
#endif SERV_REFORM_INVENTORY_TEST

	case CX2Inventory::ST_SPECIAL:
#ifdef SERV_REFORM_INVENTORY_INT
		return 24;			///< SERV_COUNTRY_US arm of the original switch
#else
		return 32;
#endif SERV_REFORM_INVENTORY_INT

#ifdef SERV_PET_SYSTEM
	case CX2Inventory::ST_PET:
		return 0;			///< pet inventories are sized per pet, not here
#endif SERV_PET_SYSTEM

#ifdef SERV_NEW_DEFENCE_DUNGEON
	case CX2Inventory::ST_E_DEFENCE_QUICK_SLOT:
		return 4;
#endif SERV_NEW_DEFENCE_DUNGEON

	default:
		return 24;
	}
}

bool CX2OfflineDB::SeedInventorySizes( UidType nUnitUID )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	if( false == Exec( "BEGIN;" ) )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"INSERT OR REPLACE INTO inventory_size( unit_uid, category, size ) VALUES( ?1, ?2, ?3 );" );
	if( NULL == pStmt )
	{
		Exec( "ROLLBACK;" );
		return false;
	}

	bool bOK = true;

	for( int iCategory = CX2Inventory::ST_NONE + 1;
		 iCategory < CX2Inventory::ST_END && true == bOK;
		 ++iCategory )
	{
		int iSize = BaseSlotSize( iCategory );
		if( iSize <= 0 )
			continue;

		sqlite3_reset( pStmt );
		sqlite3_clear_bindings( pStmt );

		sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
		sqlite3_bind_int(   pStmt, 2, iCategory );
		sqlite3_bind_int(   pStmt, 3, iSize );

		bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );
	}

	if( false == bOK )
		LogError( L"seed inventory_size" );

	sqlite3_finalize( pStmt );

	Exec( true == bOK ? "COMMIT;" : "ROLLBACK;" );

	return bOK;
}

bool CX2OfflineDB::LoadInventorySizes( UidType nUnitUID, OUT std::map< int, int >& mapOut )
{
	KLocker lock( m_cs );

	mapOut.clear();

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"SELECT category, size FROM inventory_size WHERE unit_uid = ?1;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );

	while( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		int iCategory	= sqlite3_column_int( pStmt, 0 );
		int iSize		= sqlite3_column_int( pStmt, 1 );

		mapOut.insert( std::make_pair( iCategory, iSize ) );
	}

	sqlite3_finalize( pStmt );
	return true;
}

#endif SERV_IRUHADEV_OFFLINE
