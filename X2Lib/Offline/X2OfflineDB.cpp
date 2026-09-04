#include "stdafx.h"
#include "X2OfflineDB.h"
#include "X2OfflineCashSeed.h"

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineLog.h"
#include "sqlite3.h"

CX2OfflineDB* CX2OfflineDB::ms_pInstance = NULL;

/*static*/ const char* CX2OfflineDB::UNIT_COLUMNS =
	"unit_uid, user_uid, unit_class, nickname, level, exp, ed, sp, spirit, "
	"last_pos, last_line_index, last_pos_value, cur_hp, cur_mp, hyper_gage, abil_count, "
	"reg_date, del_date, last_date, csp, title_id";

/*static*/ const char* CX2OfflineDB::ITEM_COLUMNS =
	"item_uid, unit_uid, category, slot, item_id, count, "
	"endurance, enchant_level, seal_data, usage_type, item_state, period, "
	"attrib0, attrib1, attrib2, sockets, random_sockets";

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

	// v4 (phase 5): everything of KItemInfo that is not the item ID or the
	// quantity, plus cash skill points and the per-skill CSP the skill packets
	// echo back. `blob_data` from v1 is left in place and unused - dropping a
	// column means rewriting the table in SQLite, and an unused column costs
	// nothing.
	//
	// The defaults matter for an upgraded save: a v3 file can already hold item
	// rows (nothing wrote any, but the table existed), and every default here is
	// what a plain unenchanted item carries.
	const char* const SCHEMA_V4 =
		"ALTER TABLE item ADD COLUMN endurance      INTEGER NOT NULL DEFAULT 0;"
		"ALTER TABLE item ADD COLUMN enchant_level  INTEGER NOT NULL DEFAULT 0;"
		"ALTER TABLE item ADD COLUMN seal_data      INTEGER NOT NULL DEFAULT 0;"
		"ALTER TABLE item ADD COLUMN usage_type     INTEGER NOT NULL DEFAULT 0;"
		"ALTER TABLE item ADD COLUMN item_state     INTEGER NOT NULL DEFAULT 0;"
		"ALTER TABLE item ADD COLUMN period         INTEGER NOT NULL DEFAULT 0;"
		"ALTER TABLE item ADD COLUMN attrib0        INTEGER NOT NULL DEFAULT 0;"
		"ALTER TABLE item ADD COLUMN attrib1        INTEGER NOT NULL DEFAULT 0;"
		"ALTER TABLE item ADD COLUMN attrib2        INTEGER NOT NULL DEFAULT 0;"
		"ALTER TABLE item ADD COLUMN sockets        TEXT;"
		"ALTER TABLE item ADD COLUMN random_sockets TEXT;"

		"ALTER TABLE unit ADD COLUMN csp INTEGER NOT NULL DEFAULT 0;"
		"ALTER TABLE unit_skill ADD COLUMN csp INTEGER NOT NULL DEFAULT 0;"
		;

	// v5 (phase 6): quest progress, completed quests, title missions and owned
	// titles, plus the worn title on `unit`.
	//
	// `unit_quest` already exists from v1 and is used unchanged as the header
	// row - one line per quest in progress. Its `progress` BLOB stays empty:
	// the sub-quest counters go in `unit_subquest` instead, one row each, so a
	// stuck quest can be read straight out of the save file with a SELECT
	// rather than being a blob nobody can inspect.
	//
	// Sub-quest *order* is deliberately not a column. The wire format is
	// positional - KQuestInstance::m_vecSubQuestInstance[i] must line up with
	// the templet's m_vecSubQuest[i] - so the order is rebuilt from the templet
	// on every load. Storing it as well would give two sources of truth for the
	// same thing, and the templet is the one the client agrees with.
	const char* const SCHEMA_V5 =
		"CREATE TABLE IF NOT EXISTS unit_subquest ("
		"  unit_uid   INTEGER NOT NULL,"
		"  quest_id   INTEGER NOT NULL,"
		"  sub_id     INTEGER NOT NULL,"
		"  clear_data INTEGER NOT NULL DEFAULT 0,"
		"  success    INTEGER NOT NULL DEFAULT 0,"
		"  PRIMARY KEY( unit_uid, quest_id, sub_id ) );"

		"CREATE TABLE IF NOT EXISTS unit_quest_complete ("
		"  unit_uid       INTEGER NOT NULL,"
		"  quest_id       INTEGER NOT NULL,"
		"  complete_count INTEGER NOT NULL DEFAULT 1,"
		"  complete_date  INTEGER NOT NULL DEFAULT 0,"
		"  PRIMARY KEY( unit_uid, quest_id ) );"

		"CREATE TABLE IF NOT EXISTS unit_mission ("
		"  unit_uid   INTEGER NOT NULL,"
		"  mission_id INTEGER NOT NULL,"
		"  PRIMARY KEY( unit_uid, mission_id ) );"

		"CREATE TABLE IF NOT EXISTS unit_submission ("
		"  unit_uid   INTEGER NOT NULL,"
		"  mission_id INTEGER NOT NULL,"
		"  sub_id     INTEGER NOT NULL,"
		"  clear_data INTEGER NOT NULL DEFAULT 0,"
		"  success    INTEGER NOT NULL DEFAULT 0,"
		"  PRIMARY KEY( unit_uid, mission_id, sub_id ) );"

		// end_date 0 means permanent - KTitleInfo::m_bInfinity.
		"CREATE TABLE IF NOT EXISTS unit_title ("
		"  unit_uid INTEGER NOT NULL,"
		"  title_id INTEGER NOT NULL,"
		"  end_date INTEGER NOT NULL DEFAULT 0,"
		"  PRIMARY KEY( unit_uid, title_id ) );"

		"ALTER TABLE unit ADD COLUMN title_id INTEGER NOT NULL DEFAULT 0;"
		;

	// v6 (phase 6): the two columns a cleared dungeon needs on the wire but
	// v1's `unit_dungeon` never had.
	//
	// KDungeonClearInfo carries a best score and a clear *date* as well as the
	// rank, and the local-map UI draws all three. The rows were being written
	// since phase 4 and read by nobody; now that they are sent, an empty date
	// string would reach the client's date parsing.
	const char* const SCHEMA_V6 =
		"ALTER TABLE unit_dungeon ADD COLUMN max_score  INTEGER NOT NULL DEFAULT 0;"
		"ALTER TABLE unit_dungeon ADD COLUMN clear_date INTEGER NOT NULL DEFAULT 0;"
		;

	// v7 (phase 7): the cash shop's two pieces of account state, a knob table,
	// and pets.
	//
	// `account.cash_balance` is already there from v1 and is used unchanged as
	// the wallet - the column was written into the very first schema for this
	// and never read until now.
	//
	// `cash_order` is the cash deposit: what has been bought and not yet
	// claimed into the bag. On live that lives in a separate billing database
	// behind the LoginServer, which is why the client asks for it with its own
	// packet (EGS_BILL_INVENTORY_INQUIRY_REQ) rather than reading it out of the
	// inventory. Offline it is one table keyed by account, because the deposit
	// is an account-level thing on live too: a character buys, any character on
	// the account can claim.
	//
	// `settings` exists so a knob can be changed by editing the save file
	// instead of by rebuilding the client. Only one key uses it today
	// (`cash_start`), and the point of the table is that the next one does not
	// need a migration.
	const char* const SCHEMA_V7 =
		"CREATE TABLE IF NOT EXISTS cash_order ("
		"  trans_no   INTEGER PRIMARY KEY AUTOINCREMENT,"
		"  user_uid   INTEGER NOT NULL,"
		"  product_no INTEGER NOT NULL,"
		"  item_id    INTEGER NOT NULL,"
		"  quantity   INTEGER NOT NULL DEFAULT 1,"
		"  period     INTEGER NOT NULL DEFAULT 0,"		// rental days, 0 permanent
		"  price      INTEGER NOT NULL DEFAULT 0,"
		"  buy_date   INTEGER NOT NULL );"
		"CREATE INDEX IF NOT EXISTS ix_cash_order_user ON cash_order( user_uid );"

		"CREATE TABLE IF NOT EXISTS wish_list ("
		"  user_uid INTEGER NOT NULL,"
		"  item_id  INTEGER NOT NULL,"
		"  PRIMARY KEY( user_uid, item_id ) );"

		"CREATE TABLE IF NOT EXISTS settings ("
		"  name  TEXT PRIMARY KEY,"
		"  value INTEGER NOT NULL );"

		// dbo.GPet. satiety/intimacy/extroversion/emotion are the four numbers
		// CX2PetManager draws its pet window from; the two dates drive the
		// hunger clock the client ticks on its own.
		"CREATE TABLE IF NOT EXISTS unit_pet ("
		"  pet_uid        INTEGER PRIMARY KEY AUTOINCREMENT,"
		"  unit_uid       INTEGER NOT NULL,"
		"  pet_id         INTEGER NOT NULL,"
		"  name           TEXT NOT NULL DEFAULT '',"
		"  evolution_step INTEGER NOT NULL DEFAULT 0,"
		"  satiety        INTEGER NOT NULL DEFAULT 0,"
		"  intimacy       INTEGER NOT NULL DEFAULT 0,"
		"  extroversion   INTEGER NOT NULL DEFAULT 0,"
		"  emotion        INTEGER NOT NULL DEFAULT 0,"
		"  auto_feed      INTEGER NOT NULL DEFAULT 0,"
		"  auto_looting   INTEGER NOT NULL DEFAULT 0,"
		"  last_feed      INTEGER NOT NULL DEFAULT 0,"
		"  last_summon    INTEGER NOT NULL DEFAULT 0,"
		"  reg_date       INTEGER NOT NULL DEFAULT 0 );"
		"CREATE INDEX IF NOT EXISTS ix_pet_unit ON unit_pet( unit_uid );"

		"CREATE TABLE IF NOT EXISTS unit_riding_pet ("
		"  riding_pet_uid INTEGER PRIMARY KEY AUTOINCREMENT,"
		"  unit_uid       INTEGER NOT NULL,"
		"  riding_pet_id  INTEGER NOT NULL,"
		"  stamina        REAL    NOT NULL DEFAULT 0,"
		"  destroy_date   INTEGER NOT NULL DEFAULT 0,"	// 0 permanent
		"  last_unsummon  INTEGER NOT NULL DEFAULT 0 );"
		"CREATE INDEX IF NOT EXISTS ix_riding_pet_unit ON unit_riding_pet( unit_uid );"
		;

	// v9: the cash-shop catalog itself.
	//
	// Phase 7 shipped with the catalog living only in memory, rebuilt each
	// launch from CashItemPrice.lua. That was the wrong source - it is the
	// GameServer's item-resolve price lookup, not a sale list, and it produced
	// a shop with roughly 3,000 items in it that were never purchasable.
	//
	// The real catalog is dbo.EB_Product in the billing database, which is
	// exactly what EGS_BILL_PRODUCT_INFO_ACK is for. Its 2360 rows are
	// transcribed into X2OfflineCashSeed.h and seeded here, so the shop is
	// queryable and editable in the save file like everything else.
	//
	// `price` is a column rather than a constant so it can be edited per
	// product; every seeded row gets SEED_PRICE.
	const char* const SCHEMA_V9 =
		"CREATE TABLE IF NOT EXISTS cash_product ("
		"  product_no INTEGER PRIMARY KEY,"
		"  item_id    INTEGER NOT NULL,"
		"  category   INTEGER NOT NULL,"
		"  quantity   INTEGER NOT NULL DEFAULT 1,"
		"  price      INTEGER NOT NULL DEFAULT 1,"
		"  is_event   INTEGER NOT NULL DEFAULT 0 );"
		"CREATE INDEX IF NOT EXISTS ix_cash_product_item ON cash_product( item_id );"
		;
}

//////////////////////////////////////////////////////////////////////////

CX2OfflineDB::CX2OfflineDB()
: m_pDB( NULL )
, m_iTxnDepth( 0 )
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
	kOut.m_iCSP			= sqlite3_column_int( pStmt, 19 );
	kOut.m_iTitleID		= sqlite3_column_int( pStmt, 20 );
}

/*static*/ void CX2OfflineDB::ReadItemRow( sqlite3_stmt* pStmt, OUT KOfflineItemRow& kOut )
{
	kOut.m_nItemUID			= (UidType)sqlite3_column_int64( pStmt, 0 );
	kOut.m_nUnitUID			= (UidType)sqlite3_column_int64( pStmt, 1 );
	kOut.m_iCategory		= sqlite3_column_int( pStmt, 2 );
	kOut.m_iSlotID			= sqlite3_column_int( pStmt, 3 );
	kOut.m_iItemID			= sqlite3_column_int( pStmt, 4 );
	kOut.m_iQuantity		= sqlite3_column_int( pStmt, 5 );
	kOut.m_iEndurance		= sqlite3_column_int( pStmt, 6 );
	kOut.m_iEnchantLevel	= sqlite3_column_int( pStmt, 7 );
	kOut.m_iSealData		= sqlite3_column_int( pStmt, 8 );
	kOut.m_iUsageType		= sqlite3_column_int( pStmt, 9 );
	kOut.m_iItemState		= sqlite3_column_int( pStmt, 10 );
	kOut.m_iPeriod			= sqlite3_column_int( pStmt, 11 );
	kOut.m_iAttrib0			= sqlite3_column_int( pStmt, 12 );
	kOut.m_iAttrib1			= sqlite3_column_int( pStmt, 13 );
	kOut.m_iAttrib2			= sqlite3_column_int( pStmt, 14 );

	ParseIntList( ColumnText( pStmt, 15 ), kOut.m_vecSocket );
	ParseIntList( ColumnText( pStmt, 16 ), kOut.m_vecRandomSocket );
}

/*static*/ std::wstring CX2OfflineDB::FormatIntList( const std::vector<int>& vec )
{
	std::wstring wstrOut;

	for( size_t i = 0; i < vec.size(); ++i )
	{
		wchar_t szBuff[16];
		_snwprintf( szBuff, 16, ( 0 == i ) ? L"%d" : L",%d", vec[i] );
		szBuff[15] = L'\0';

		wstrOut += szBuff;
	}

	return wstrOut;
}

/*static*/ void CX2OfflineDB::ParseIntList( const std::wstring& wstr, OUT std::vector<int>& vecOut )
{
	vecOut.clear();

	size_t iStart = 0;
	while( iStart < wstr.size() )
	{
		size_t iComma = wstr.find( L',', iStart );
		if( std::wstring::npos == iComma )
			iComma = wstr.size();

		if( iComma > iStart )
			vecOut.push_back( _wtoi( wstr.substr( iStart, iComma - iStart ).c_str() ) );

		iStart = iComma + 1;
	}
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

	// An open savepoint at this point means a handler faulted and its guard
	// never ran; SQLite would roll it back on close anyway, but say so.
	if( m_iTxnDepth > 0 )
		CX2OfflineLog::Server( L"DB WARN  closing with %d savepoint(s) still open - they roll back", m_iTxnDepth );

	m_iTxnDepth = 0;

	sqlite3_close( m_pDB );
	m_pDB = NULL;
}

//////////////////////////////////////////////////////////////////////////
// save integrity (phase 8) - see the header for why these are savepoints

bool CX2OfflineDB::Begin()
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	char szSQL[64];
	_snprintf( szSQL, 64, "SAVEPOINT sp%d;", m_iTxnDepth );
	szSQL[63] = '\0';

	if( false == Exec( szSQL ) )
		return false;

	++m_iTxnDepth;
	return true;
}

bool CX2OfflineDB::Commit()
{
	KLocker lock( m_cs );

	if( NULL == m_pDB || m_iTxnDepth <= 0 )
		return false;

	--m_iTxnDepth;

	// RELEASE, not COMMIT: releasing the outermost savepoint is what commits.
	char szSQL[64];
	_snprintf( szSQL, 64, "RELEASE sp%d;", m_iTxnDepth );
	szSQL[63] = '\0';

	return Exec( szSQL );
}

bool CX2OfflineDB::Rollback()
{
	KLocker lock( m_cs );

	if( NULL == m_pDB || m_iTxnDepth <= 0 )
		return false;

	--m_iTxnDepth;

	// ROLLBACK TO leaves the savepoint on the stack, so it has to be released
	// as well or the depth counter and SQLite's own stack drift apart.
	char szSQL[96];
	_snprintf( szSQL, 96, "ROLLBACK TO sp%d; RELEASE sp%d;", m_iTxnDepth, m_iTxnDepth );
	szSQL[95] = '\0';

	return Exec( szSQL );
}

bool CX2OfflineDB::Checkpoint()
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	if( m_iTxnDepth > 0 )
	{
		CX2OfflineLog::Server( L"DB WARN  checkpoint skipped - %d savepoint(s) open", m_iTxnDepth );
		return false;
	}

	return Exec( "PRAGMA wal_checkpoint(TRUNCATE);" );
}

bool CX2OfflineDB::Backup( const wchar_t* szPath )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB || NULL == szPath )
		return false;

	// Written to a temporary and renamed over the target only once it is
	// complete. The one caller runs during the client's own shutdown, so the
	// process can go away in the middle of this - and a half-written backup
	// that looks like a backup is worse than no backup at all.
	wchar_t szTemp[MAX_PATH];
	_snwprintf( szTemp, MAX_PATH, L"%s.tmp", szPath );
	szTemp[MAX_PATH - 1] = L'\0';

	::DeleteFileW( szTemp );		// never append to a previous partial

	sqlite3* pDest = NULL;
	if( SQLITE_OK != sqlite3_open16( szTemp, &pDest ) )
	{
		CX2OfflineLog::Server( L"DB ERROR backup could not open '%s'", szTemp );

		if( NULL != pDest )
			sqlite3_close( pDest );

		return false;
	}

	bool bOK = false;

	sqlite3_backup* pBackup = sqlite3_backup_init( pDest, "main", m_pDB, "main" );
	if( NULL != pBackup )
	{
		// -1 pages: the whole database in one step, which is what we want -
		// nothing else is writing, and a partial backup is worse than none.
		sqlite3_backup_step( pBackup, -1 );
		bOK = ( SQLITE_OK == sqlite3_backup_finish( pBackup ) );
	}

	if( false == bOK )
	{
		const void* pMsg = sqlite3_errmsg16( pDest );
		CX2OfflineLog::Server( L"DB ERROR backup to '%s' failed : %s",
			szPath, ( NULL != pMsg ) ? (const wchar_t*)pMsg : L"<no message>" );
	}

	sqlite3_close( pDest );

	if( true == bOK )
	{
		// The destination is a plain SQLite file with no WAL of its own (the
		// backup API writes it in the default journal mode), so a rename is
		// all that is needed to publish it.
		if( 0 == ::MoveFileExW( szTemp, szPath, MOVEFILE_REPLACE_EXISTING ) )
		{
			CX2OfflineLog::Server( L"DB ERROR backup written but could not be renamed to '%s' (win32 %u); it is in '%s'",
				szPath, (unsigned int)::GetLastError(), szTemp );
			bOK = false;
		}
	}
	else
	{
		::DeleteFileW( szTemp );
	}

	return bOK;
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
	//
	// These are the only literal BEGIN/COMMIT left in this file, and they are
	// allowed to stay because Migrate() runs from Open(), before the first
	// packet - so the dispatch transaction (phase 8) cannot be open around
	// them. Anything added below this point that runs during play must use
	// Begin()/Commit()/Rollback() instead, or SQLite will refuse the nested
	// BEGIN and the rung will silently not apply.
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

	if( iFrom < 4 )
	{
		if( false == Exec( "BEGIN;" ) )			return false;
		if( false == Exec( SCHEMA_V4 ) )		{ Exec( "ROLLBACK;" ); return false; }
		if( false == Exec( "COMMIT;" ) )		return false;

		CX2OfflineLog::Server( L"DB       schema upgraded to v4 (item detail, skill points)" );
	}

	if( iFrom < 5 )
	{
		if( false == Exec( "BEGIN;" ) )			return false;
		if( false == Exec( SCHEMA_V5 ) )		{ Exec( "ROLLBACK;" ); return false; }
		if( false == Exec( "COMMIT;" ) )		return false;

		CX2OfflineLog::Server( L"DB       schema upgraded to v5 (quests, titles, missions)" );
	}

	if( iFrom < 6 )
	{
		if( false == Exec( "BEGIN;" ) )			return false;
		if( false == Exec( SCHEMA_V6 ) )		{ Exec( "ROLLBACK;" ); return false; }
		if( false == Exec( "COMMIT;" ) )		return false;

		CX2OfflineLog::Server( L"DB       schema upgraded to v6 (dungeon clear score and date)" );
	}

	if( iFrom < 7 )
	{
		// The starting wallet, and the same amount handed to an account that
		// already exists. A save upgraded from v6 has cash_balance 0 - the
		// column was in the v1 schema and nothing ever wrote it - so without
		// this line the cash shop opens for an existing character and can never
		// buy anything, which is indistinguishable from a broken shop.
		//
		// Only a zero balance is topped up, so re-running the rung (which
		// cannot happen, but the SQL should not depend on that) never refunds a
		// player who has spent down to nothing on a later run.
		char szSeed[256];
		_snprintf( szSeed, 256,
			"INSERT OR IGNORE INTO settings( name, value ) VALUES( 'cash_start', %d );"
			"UPDATE account SET cash_balance = ( SELECT value FROM settings WHERE name = 'cash_start' )"
			" WHERE cash_balance = 0;",
			(int)DEFAULT_CASH_BALANCE );
		szSeed[255] = '\0';

		if( false == Exec( "BEGIN;" ) )			return false;
		if( false == Exec( SCHEMA_V7 ) )		{ Exec( "ROLLBACK;" ); return false; }
		if( false == Exec( szSeed ) )			{ Exec( "ROLLBACK;" ); return false; }
		if( false == Exec( "COMMIT;" ) )		return false;

		CX2OfflineLog::Server( L"DB       schema upgraded to v7 (cash deposit, wish list, pets)" );
	}

	if( iFrom < 8 )
	{
		// The wallet became cosmetic in phase 7's follow-up: a purchase no
		// longer deducts it, so `cash_start` is the number the shop shows for
		// the life of the save rather than a starting balance, and it goes up
		// to 999999.
		//
		// Only a row still sitting on the old default is rewritten. A
		// `cash_start` somebody has edited by hand is theirs, and a migration
		// that stomps a deliberate edit is a migration nobody can trust.
		char szWallet[192];
		_snprintf( szWallet, 192,
			"UPDATE settings SET value = %d WHERE name = 'cash_start' AND value = %d;"
			"INSERT OR IGNORE INTO settings( name, value ) VALUES( 'cash_start', %d );",
			(int)DEFAULT_CASH_BALANCE, (int)LEGACY_CASH_BALANCE, (int)DEFAULT_CASH_BALANCE );
		szWallet[191] = '\0';

		if( false == Exec( "BEGIN;" ) )			return false;
		if( false == Exec( szWallet ) )			{ Exec( "ROLLBACK;" ); return false; }
		if( false == Exec( "COMMIT;" ) )		return false;

		CX2OfflineLog::Server( L"DB       schema upgraded to v8 (cosmetic wallet, no deduction)" );
	}

	if( iFrom < 9 )
	{
		if( false == Exec( "BEGIN;" ) )			return false;
		if( false == Exec( SCHEMA_V9 ) )		{ Exec( "ROLLBACK;" ); return false; }

		// INSERT OR IGNORE, so a re-run cannot duplicate a product and a row the
		// player has edited is left exactly as they left it.
		sqlite3_stmt* pIns = Prepare(
			"INSERT OR IGNORE INTO cash_product( product_no, item_id, category, quantity, price, is_event )"
			" VALUES( ?1, ?2, ?3, ?4, ?5, ?6 );" );

		if( NULL == pIns )
		{
			Exec( "ROLLBACK;" );
			return false;
		}

		int iSeeded = 0;
		for( int i = 0; i < X2OfflineCashSeed::ROW_COUNT; ++i )
		{
			const X2OfflineCashSeed::KRow& kRow = X2OfflineCashSeed::ROWS[i];

			sqlite3_bind_int( pIns, 1, (int)kRow.m_sProductNo );
			sqlite3_bind_int( pIns, 2, kRow.m_iItemID );
			sqlite3_bind_int( pIns, 3, (int)kRow.m_cCategoryNo );
			sqlite3_bind_int( pIns, 4, (int)kRow.m_cQuantity );
			sqlite3_bind_int( pIns, 5, (int)SEED_CASH_PRICE );
			sqlite3_bind_int( pIns, 6, 0 != kRow.m_cEvent ? 1 : 0 );

			if( SQLITE_DONE == sqlite3_step( pIns ) )
				++iSeeded;
			else
				LogError( L"seed cash_product" );

			sqlite3_reset( pIns );
		}

		sqlite3_finalize( pIns );

		if( false == Exec( "COMMIT;" ) )		return false;

		CX2OfflineLog::Server( L"DB       schema upgraded to v9 (cash catalog: %d of %d rows seeded)",
			iSeeded, (int)X2OfflineCashSeed::ROW_COUNT );
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

	// cash_balance comes from the `cash_start` setting the v7 migration seeds,
	// not from a literal, so changing the knob in the save file changes what the
	// next account starts with. COALESCE covers the case of somebody deleting
	// the row.
	sqlite3_stmt* pIns = Prepare(
		"INSERT INTO account( login_id, unit_slots, cash_balance, created_at ) VALUES( ?1, ?2,"
		" COALESCE( ( SELECT value FROM settings WHERE name = 'cash_start' ), 0 ), ?3 );" );
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
		{ "inventory_size", "item", "unit_skill", "unit_quest", "unit_dungeon",
		  "unit_subquest", "unit_quest_complete", "unit_mission", "unit_submission",
		  "unit_title" };

	// Begin()/Commit(), not "BEGIN;": since phase 8 the whole dispatch is
	// already inside a transaction, and a nested BEGIN is an error.
	if( false == Begin() )
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

	if( true == bOK )	Commit();
	else				Rollback();

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
		"INSERT INTO unit_dungeon ( unit_uid, dungeon_id, clear_count, best_rank, clear_date ) "
		"VALUES ( ?1, ?2, 1, ?3, ?4 ) "
		"ON CONFLICT( unit_uid, dungeon_id ) DO UPDATE SET "
		"  clear_count = clear_count + 1,"
		"  best_rank   = CASE WHEN best_rank = 0 OR ?3 < best_rank THEN ?3 ELSE best_rank END,"
		"  clear_date  = ?4;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int(   pStmt, 2, iDungeonID );
	sqlite3_bind_int(   pStmt, 3, iRank );
	sqlite3_bind_int64( pStmt, 4, NowEpoch() );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::LoadDungeonClears( UidType nUnitUID,
									  OUT std::vector< KOfflineDungeonClearRow >& vecOut )
{
	KLocker lock( m_cs );

	vecOut.clear();

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"SELECT dungeon_id, clear_count, best_rank, max_score, clear_date "
		"FROM unit_dungeon WHERE unit_uid = ?1 ORDER BY dungeon_id;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );

	while( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		KOfflineDungeonClearRow kRow;
		kRow.m_iDungeonID	= sqlite3_column_int( pStmt, 0 );
		kRow.m_iClearCount	= sqlite3_column_int( pStmt, 1 );
		kRow.m_iBestRank	= sqlite3_column_int( pStmt, 2 );
		kRow.m_iMaxScore	= sqlite3_column_int( pStmt, 3 );
		kRow.m_tClearDate	= (__int64)sqlite3_column_int64( pStmt, 4 );

		vecOut.push_back( kRow );
	}

	sqlite3_finalize( pStmt );
	return true;
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

	if( false == Begin() )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"INSERT OR REPLACE INTO inventory_size( unit_uid, category, size ) VALUES( ?1, ?2, ?3 );" );
	if( NULL == pStmt )
	{
		Rollback();
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

	if( true == bOK )	Commit();
	else				Rollback();

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

#ifdef SERV_IRUHADEV_OFFLINE_INVENTORY_EXPAND
bool CX2OfflineDB::ExpandInventorySize( UidType nUnitUID, int iCategory, int iRequestedIncrement,
										 int iMaxSize, OUT int& iGranted )
{
	iGranted = 0;

	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	int iCurrent = BaseSlotSize( iCategory );

	sqlite3_stmt* pRead = Prepare(
		"SELECT size FROM inventory_size WHERE unit_uid = ?1 AND category = ?2;" );
	if( NULL == pRead )
		return false;

	sqlite3_bind_int64( pRead, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int(   pRead, 2, iCategory );

	if( SQLITE_ROW == sqlite3_step( pRead ) )
		iCurrent = sqlite3_column_int( pRead, 0 );

	sqlite3_finalize( pRead );

	iGranted = std::min< int >( iRequestedIncrement, iMaxSize - iCurrent );
	if( iGranted <= 0 )
	{
		iGranted = 0;
		return true;			///< already at the cap - not a failure
	}

	sqlite3_stmt* pWrite = Prepare(
		"INSERT OR REPLACE INTO inventory_size( unit_uid, category, size ) VALUES( ?1, ?2, ?3 );" );
	if( NULL == pWrite )
	{
		iGranted = 0;
		return false;
	}

	sqlite3_bind_int64( pWrite, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int(   pWrite, 2, iCategory );
	sqlite3_bind_int(   pWrite, 3, iCurrent + iGranted );

	const bool bOK = ( SQLITE_DONE == sqlite3_step( pWrite ) );
	sqlite3_finalize( pWrite );

	if( false == bOK )
	{
		iGranted = 0;
		LogError( L"expand inventory_size" );
	}

	return bOK;
}
#endif SERV_IRUHADEV_OFFLINE_INVENTORY_EXPAND

//////////////////////////////////////////////////////////////////////////
// items

bool CX2OfflineDB::LoadItems( UidType nUnitUID, OUT std::vector< KOfflineItemRow >& vecOut )
{
	KLocker lock( m_cs );

	vecOut.clear();

	if( NULL == m_pDB )
		return false;

	std::string strSQL = "SELECT ";
	strSQL += ITEM_COLUMNS;
	strSQL += " FROM item WHERE unit_uid = ?1 ORDER BY category, slot;";

	sqlite3_stmt* pStmt = Prepare( strSQL.c_str() );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );

	while( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		KOfflineItemRow kRow;
		ReadItemRow( pStmt, kRow );
		vecOut.push_back( kRow );
	}

	sqlite3_finalize( pStmt );
	return true;
}

bool CX2OfflineDB::InsertItem( IN OUT KOfflineItemRow& kInOut )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"INSERT INTO item( unit_uid, category, slot, item_id, count, equipped, "
		"                  endurance, enchant_level, seal_data, usage_type, item_state, period, "
		"                  attrib0, attrib1, attrib2, sockets, random_sockets ) "
		"VALUES( ?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17 );" );
	if( NULL == pStmt )
		return false;

	// `equipped` is redundant with category == ST_E_EQUIP and is written anyway,
	// because it makes a hand-read of the save file legible.
	const int iEquipped = ( CX2Inventory::ST_E_EQUIP == kInOut.m_iCategory ) ? 1 : 0;

	sqlite3_bind_int64( pStmt,  1, (sqlite3_int64)kInOut.m_nUnitUID );
	sqlite3_bind_int(   pStmt,  2, kInOut.m_iCategory );
	sqlite3_bind_int(   pStmt,  3, kInOut.m_iSlotID );
	sqlite3_bind_int(   pStmt,  4, kInOut.m_iItemID );
	sqlite3_bind_int(   pStmt,  5, kInOut.m_iQuantity );
	sqlite3_bind_int(   pStmt,  6, iEquipped );
	sqlite3_bind_int(   pStmt,  7, kInOut.m_iEndurance );
	sqlite3_bind_int(   pStmt,  8, kInOut.m_iEnchantLevel );
	sqlite3_bind_int(   pStmt,  9, kInOut.m_iSealData );
	sqlite3_bind_int(   pStmt, 10, kInOut.m_iUsageType );
	sqlite3_bind_int(   pStmt, 11, kInOut.m_iItemState );
	sqlite3_bind_int(   pStmt, 12, kInOut.m_iPeriod );
	sqlite3_bind_int(   pStmt, 13, kInOut.m_iAttrib0 );
	sqlite3_bind_int(   pStmt, 14, kInOut.m_iAttrib1 );
	sqlite3_bind_int(   pStmt, 15, kInOut.m_iAttrib2 );

	BindText( pStmt, 16, FormatIntList( kInOut.m_vecSocket ) );
	BindText( pStmt, 17, FormatIntList( kInOut.m_vecRandomSocket ) );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	if( false == bOK )
		LogError( L"insert item" );
	else
		kInOut.m_nItemUID = (UidType)sqlite3_last_insert_rowid( m_pDB );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::MoveItemRow( UidType nItemUID, int iCategory, int iSlotID )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"UPDATE item SET category = ?2, slot = ?3, equipped = ?4 WHERE item_uid = ?1;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nItemUID );
	sqlite3_bind_int(   pStmt, 2, iCategory );
	sqlite3_bind_int(   pStmt, 3, iSlotID );
	sqlite3_bind_int(   pStmt, 4, ( CX2Inventory::ST_E_EQUIP == iCategory ) ? 1 : 0 );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::SetItemQuantity( UidType nItemUID, int iQuantity )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare( "UPDATE item SET count = ?2 WHERE item_uid = ?1;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nItemUID );
	sqlite3_bind_int(   pStmt, 2, iQuantity );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::UpdateItem( const KOfflineItemRow& kRow )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"UPDATE item SET category = ?2, slot = ?3, item_id = ?4, count = ?5, equipped = ?6, "
		"                endurance = ?7, enchant_level = ?8, seal_data = ?9, usage_type = ?10, "
		"                item_state = ?11, period = ?12, attrib0 = ?13, attrib1 = ?14, "
		"                attrib2 = ?15, sockets = ?16, random_sockets = ?17 "
		"WHERE item_uid = ?1;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt,  1, (sqlite3_int64)kRow.m_nItemUID );
	sqlite3_bind_int(   pStmt,  2, kRow.m_iCategory );
	sqlite3_bind_int(   pStmt,  3, kRow.m_iSlotID );
	sqlite3_bind_int(   pStmt,  4, kRow.m_iItemID );
	sqlite3_bind_int(   pStmt,  5, kRow.m_iQuantity );
	sqlite3_bind_int(   pStmt,  6, ( CX2Inventory::ST_E_EQUIP == kRow.m_iCategory ) ? 1 : 0 );
	sqlite3_bind_int(   pStmt,  7, kRow.m_iEndurance );
	sqlite3_bind_int(   pStmt,  8, kRow.m_iEnchantLevel );
	sqlite3_bind_int(   pStmt,  9, kRow.m_iSealData );
	sqlite3_bind_int(   pStmt, 10, kRow.m_iUsageType );
	sqlite3_bind_int(   pStmt, 11, kRow.m_iItemState );
	sqlite3_bind_int(   pStmt, 12, kRow.m_iPeriod );
	sqlite3_bind_int(   pStmt, 13, kRow.m_iAttrib0 );
	sqlite3_bind_int(   pStmt, 14, kRow.m_iAttrib1 );
	sqlite3_bind_int(   pStmt, 15, kRow.m_iAttrib2 );

	BindText( pStmt, 16, FormatIntList( kRow.m_vecSocket ) );
	BindText( pStmt, 17, FormatIntList( kRow.m_vecRandomSocket ) );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::DeleteItemRow( UidType nItemUID )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare( "DELETE FROM item WHERE item_uid = ?1;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nItemUID );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );
	return bOK;
}

//////////////////////////////////////////////////////////////////////////
// skills

bool CX2OfflineDB::LoadSkills( UidType nUnitUID, OUT std::vector< KOfflineSkillRow >& vecOut )
{
	KLocker lock( m_cs );

	vecOut.clear();

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"SELECT skill_id, level, csp, slot FROM unit_skill WHERE unit_uid = ?1;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );

	while( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		KOfflineSkillRow kRow;
		kRow.m_iSkillID	= sqlite3_column_int( pStmt, 0 );
		kRow.m_iLevel	= sqlite3_column_int( pStmt, 1 );
		kRow.m_iCSPoint	= sqlite3_column_int( pStmt, 2 );
		kRow.m_iSlot	= sqlite3_column_int( pStmt, 3 );

		vecOut.push_back( kRow );
	}

	sqlite3_finalize( pStmt );
	return true;
}

bool CX2OfflineDB::SaveSkill( UidType nUnitUID, const KOfflineSkillRow& kRow )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"INSERT OR REPLACE INTO unit_skill( unit_uid, skill_id, level, csp, slot ) "
		"VALUES( ?1, ?2, ?3, ?4, ?5 );" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int(   pStmt, 2, kRow.m_iSkillID );
	sqlite3_bind_int(   pStmt, 3, kRow.m_iLevel );
	sqlite3_bind_int(   pStmt, 4, kRow.m_iCSPoint );
	sqlite3_bind_int(   pStmt, 5, kRow.m_iSlot );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	if( false == bOK )
		LogError( L"save skill" );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::ClearSkills( UidType nUnitUID )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare( "DELETE FROM unit_skill WHERE unit_uid = ?1;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::SetSkillSlot( UidType nUnitUID, int iSlot, int iSkillID )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	// Two statements, in this order: vacate the slot, then claim it. A skill
	// moved between slots would otherwise leave its old slot pointing at it as
	// well, and a reload would find two slots holding the same skill.
	{
		sqlite3_stmt* pStmt = Prepare(
			"UPDATE unit_skill SET slot = -1 WHERE unit_uid = ?1 AND slot = ?2;" );
		if( NULL == pStmt )
			return false;

		sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
		sqlite3_bind_int(   pStmt, 2, iSlot );

		sqlite3_step( pStmt );
		sqlite3_finalize( pStmt );
	}

	if( 0 == iSkillID )
		return true;			///< the slot was being emptied

	sqlite3_stmt* pStmt = Prepare(
		"UPDATE unit_skill SET slot = ?2 WHERE unit_uid = ?1 AND skill_id = ?3;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int(   pStmt, 2, iSlot );
	sqlite3_bind_int(   pStmt, 3, iSkillID );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::SaveSkillPoint( UidType nUnitUID, int iSP, int iCSP )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare( "UPDATE unit SET sp = ?2, csp = ?3 WHERE unit_uid = ?1;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int(   pStmt, 2, iSP );
	sqlite3_bind_int(   pStmt, 3, iCSP );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );
	return bOK;
}

//////////////////////////////////////////////////////////////////////////
// quests

bool CX2OfflineDB::LoadQuests( UidType nUnitUID, OUT std::vector< KOfflineQuestRow >& vecOut )
{
	KLocker lock( m_cs );

	vecOut.clear();

	if( NULL == m_pDB )
		return false;

	// Headers first, then every sub-quest row in one pass, matched back by
	// quest ID. Two statements rather than a join because the caller has to
	// see a quest that has somehow lost its sub-quest rows - that is a broken
	// save worth logging, and a join would silently hide it.
	{
		sqlite3_stmt* pStmt = Prepare(
			"SELECT quest_id FROM unit_quest WHERE unit_uid = ?1 ORDER BY quest_id;" );
		if( NULL == pStmt )
			return false;

		sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );

		while( SQLITE_ROW == sqlite3_step( pStmt ) )
		{
			KOfflineQuestRow kRow;
			kRow.m_iQuestID = sqlite3_column_int( pStmt, 0 );
			vecOut.push_back( kRow );
		}

		sqlite3_finalize( pStmt );
	}

	if( true == vecOut.empty() )
		return true;

	{
		sqlite3_stmt* pStmt = Prepare(
			"SELECT quest_id, sub_id, clear_data, success FROM unit_subquest "
			"WHERE unit_uid = ?1;" );
		if( NULL == pStmt )
			return false;

		sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );

		while( SQLITE_ROW == sqlite3_step( pStmt ) )
		{
			const int iQuestID = sqlite3_column_int( pStmt, 0 );

			KOfflineSubQuestRow kSub;
			kSub.m_iSubID		= sqlite3_column_int( pStmt, 1 );
			kSub.m_iClearData	= sqlite3_column_int( pStmt, 2 );
			kSub.m_bSuccess		= ( 0 != sqlite3_column_int( pStmt, 3 ) );

			for( size_t i = 0; i < vecOut.size(); ++i )
			{
				if( vecOut[i].m_iQuestID == iQuestID )
				{
					vecOut[i].m_vecSub.push_back( kSub );
					break;
				}
			}
		}

		sqlite3_finalize( pStmt );
	}

	return true;
}

bool CX2OfflineDB::SaveQuest( UidType nUnitUID, const KOfflineQuestRow& kRow )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	{
		sqlite3_stmt* pStmt = Prepare(
			"INSERT OR REPLACE INTO unit_quest( unit_uid, quest_id, state ) VALUES( ?1, ?2, 0 );" );
		if( NULL == pStmt )
			return false;

		sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
		sqlite3_bind_int(   pStmt, 2, kRow.m_iQuestID );

		bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );
		sqlite3_finalize( pStmt );

		if( false == bOK )
		{
			LogError( L"save quest header" );
			return false;
		}
	}

	sqlite3_stmt* pStmt = Prepare(
		"INSERT OR REPLACE INTO unit_subquest( unit_uid, quest_id, sub_id, clear_data, success ) "
		"VALUES( ?1, ?2, ?3, ?4, ?5 );" );
	if( NULL == pStmt )
		return false;

	bool bOK = true;

	for( size_t i = 0; i < kRow.m_vecSub.size(); ++i )
	{
		sqlite3_reset( pStmt );
		sqlite3_clear_bindings( pStmt );

		sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
		sqlite3_bind_int(   pStmt, 2, kRow.m_iQuestID );
		sqlite3_bind_int(   pStmt, 3, kRow.m_vecSub[i].m_iSubID );
		sqlite3_bind_int(   pStmt, 4, kRow.m_vecSub[i].m_iClearData );
		sqlite3_bind_int(   pStmt, 5, kRow.m_vecSub[i].m_bSuccess ? 1 : 0 );

		if( SQLITE_DONE != sqlite3_step( pStmt ) )
		{
			LogError( L"save sub-quest" );
			bOK = false;
		}
	}

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::DeleteQuest( UidType nUnitUID, int iQuestID )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	const char* const aszSQL[2] =
	{
		"DELETE FROM unit_subquest WHERE unit_uid = ?1 AND quest_id = ?2;",
		"DELETE FROM unit_quest    WHERE unit_uid = ?1 AND quest_id = ?2;",
	};

	bool bOK = true;

	for( int i = 0; i < 2; ++i )
	{
		sqlite3_stmt* pStmt = Prepare( aszSQL[i] );
		if( NULL == pStmt )
			return false;

		sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
		sqlite3_bind_int(   pStmt, 2, iQuestID );

		if( SQLITE_DONE != sqlite3_step( pStmt ) )
			bOK = false;

		sqlite3_finalize( pStmt );
	}

	return bOK;
}

bool CX2OfflineDB::LoadCompleteQuests( UidType nUnitUID, OUT std::vector< KOfflineCompleteQuestRow >& vecOut )
{
	KLocker lock( m_cs );

	vecOut.clear();

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"SELECT quest_id, complete_count, complete_date FROM unit_quest_complete "
		"WHERE unit_uid = ?1 ORDER BY quest_id;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );

	while( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		KOfflineCompleteQuestRow kRow;
		kRow.m_iQuestID	= sqlite3_column_int( pStmt, 0 );
		kRow.m_iCount	= sqlite3_column_int( pStmt, 1 );
		kRow.m_tDate	= (__int64)sqlite3_column_int64( pStmt, 2 );

		vecOut.push_back( kRow );
	}

	sqlite3_finalize( pStmt );
	return true;
}

bool CX2OfflineDB::SaveCompleteQuest( UidType nUnitUID, const KOfflineCompleteQuestRow& kRow )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"INSERT OR REPLACE INTO unit_quest_complete"
		"( unit_uid, quest_id, complete_count, complete_date ) VALUES( ?1, ?2, ?3, ?4 );" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int(   pStmt, 2, kRow.m_iQuestID );
	sqlite3_bind_int(   pStmt, 3, kRow.m_iCount );
	sqlite3_bind_int64( pStmt, 4, (sqlite3_int64)kRow.m_tDate );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	if( false == bOK )
		LogError( L"save completed quest" );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::SaveUnitClass( UidType nUnitUID, int iUnitClass )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare( "UPDATE unit SET unit_class = ?2 WHERE unit_uid = ?1;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int(   pStmt, 2, iUnitClass );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	if( false == bOK )
		LogError( L"save unit class" );

	sqlite3_finalize( pStmt );
	return bOK;
}

//////////////////////////////////////////////////////////////////////////
// title missions and titles

bool CX2OfflineDB::LoadMissions( UidType nUnitUID, OUT std::vector< KOfflineMissionRow >& vecOut )
{
	KLocker lock( m_cs );

	vecOut.clear();

	if( NULL == m_pDB )
		return false;

	{
		sqlite3_stmt* pStmt = Prepare(
			"SELECT mission_id FROM unit_mission WHERE unit_uid = ?1 ORDER BY mission_id;" );
		if( NULL == pStmt )
			return false;

		sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );

		while( SQLITE_ROW == sqlite3_step( pStmt ) )
		{
			KOfflineMissionRow kRow;
			kRow.m_iMissionID = sqlite3_column_int( pStmt, 0 );
			vecOut.push_back( kRow );
		}

		sqlite3_finalize( pStmt );
	}

	if( true == vecOut.empty() )
		return true;

	{
		sqlite3_stmt* pStmt = Prepare(
			"SELECT mission_id, sub_id, clear_data, success FROM unit_submission "
			"WHERE unit_uid = ?1;" );
		if( NULL == pStmt )
			return false;

		sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );

		while( SQLITE_ROW == sqlite3_step( pStmt ) )
		{
			const int iMissionID = sqlite3_column_int( pStmt, 0 );

			KOfflineSubQuestRow kSub;
			kSub.m_iSubID		= sqlite3_column_int( pStmt, 1 );
			kSub.m_iClearData	= sqlite3_column_int( pStmt, 2 );
			kSub.m_bSuccess		= ( 0 != sqlite3_column_int( pStmt, 3 ) );

			for( size_t i = 0; i < vecOut.size(); ++i )
			{
				if( vecOut[i].m_iMissionID == iMissionID )
				{
					vecOut[i].m_vecSub.push_back( kSub );
					break;
				}
			}
		}

		sqlite3_finalize( pStmt );
	}

	return true;
}

bool CX2OfflineDB::SaveMission( UidType nUnitUID, const KOfflineMissionRow& kRow )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	{
		sqlite3_stmt* pStmt = Prepare(
			"INSERT OR REPLACE INTO unit_mission( unit_uid, mission_id ) VALUES( ?1, ?2 );" );
		if( NULL == pStmt )
			return false;

		sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
		sqlite3_bind_int(   pStmt, 2, kRow.m_iMissionID );

		bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );
		sqlite3_finalize( pStmt );

		if( false == bOK )
		{
			LogError( L"save mission header" );
			return false;
		}
	}

	sqlite3_stmt* pStmt = Prepare(
		"INSERT OR REPLACE INTO unit_submission( unit_uid, mission_id, sub_id, clear_data, success ) "
		"VALUES( ?1, ?2, ?3, ?4, ?5 );" );
	if( NULL == pStmt )
		return false;

	bool bOK = true;

	for( size_t i = 0; i < kRow.m_vecSub.size(); ++i )
	{
		sqlite3_reset( pStmt );
		sqlite3_clear_bindings( pStmt );

		sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
		sqlite3_bind_int(   pStmt, 2, kRow.m_iMissionID );
		sqlite3_bind_int(   pStmt, 3, kRow.m_vecSub[i].m_iSubID );
		sqlite3_bind_int(   pStmt, 4, kRow.m_vecSub[i].m_iClearData );
		sqlite3_bind_int(   pStmt, 5, kRow.m_vecSub[i].m_bSuccess ? 1 : 0 );

		if( SQLITE_DONE != sqlite3_step( pStmt ) )
		{
			LogError( L"save sub-mission" );
			bOK = false;
		}
	}

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::DeleteMission( UidType nUnitUID, int iMissionID )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	const char* const aszSQL[2] =
	{
		"DELETE FROM unit_submission WHERE unit_uid = ?1 AND mission_id = ?2;",
		"DELETE FROM unit_mission    WHERE unit_uid = ?1 AND mission_id = ?2;",
	};

	bool bOK = true;

	for( int i = 0; i < 2; ++i )
	{
		sqlite3_stmt* pStmt = Prepare( aszSQL[i] );
		if( NULL == pStmt )
			return false;

		sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
		sqlite3_bind_int(   pStmt, 2, iMissionID );

		if( SQLITE_DONE != sqlite3_step( pStmt ) )
			bOK = false;

		sqlite3_finalize( pStmt );
	}

	return bOK;
}

bool CX2OfflineDB::LoadTitles( UidType nUnitUID, OUT std::vector< KOfflineTitleRow >& vecOut )
{
	KLocker lock( m_cs );

	vecOut.clear();

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"SELECT title_id, end_date FROM unit_title WHERE unit_uid = ?1 ORDER BY title_id;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );

	while( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		KOfflineTitleRow kRow;
		kRow.m_iTitleID	= sqlite3_column_int( pStmt, 0 );
		kRow.m_tEndDate	= (__int64)sqlite3_column_int64( pStmt, 1 );

		vecOut.push_back( kRow );
	}

	sqlite3_finalize( pStmt );
	return true;
}

bool CX2OfflineDB::SaveTitle( UidType nUnitUID, const KOfflineTitleRow& kRow )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"INSERT OR REPLACE INTO unit_title( unit_uid, title_id, end_date ) VALUES( ?1, ?2, ?3 );" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int(   pStmt, 2, kRow.m_iTitleID );
	sqlite3_bind_int64( pStmt, 3, (sqlite3_int64)kRow.m_tEndDate );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	if( false == bOK )
		LogError( L"save title" );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::SaveEquippedTitle( UidType nUnitUID, int iTitleID )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare( "UPDATE unit SET title_id = ?2 WHERE unit_uid = ?1;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int(   pStmt, 2, iTitleID );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );
	return bOK;
}

//////////////////////////////////////////////////////////////////////////
// the cash wallet, the cash deposit and the wish list (phase 7)

int CX2OfflineDB::GetWallet()
{
	// No lock and no member state: GetSetting takes m_cs itself.
	int iValue = 0;
	if( false == GetSetting( "cash_start", iValue ) )
		return (int)DEFAULT_CASH_BALANCE;		///< row deleted; fall back rather than showing 0

	return iValue < 0 ? 0 : iValue;
}

bool CX2OfflineDB::GetCashBalance( UidType nUserUID, OUT int& iOut )
{
	KLocker lock( m_cs );

	iOut = 0;

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare( "SELECT cash_balance FROM account WHERE user_uid = ?1;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUserUID );

	bool bFound = false;
	if( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		iOut	= sqlite3_column_int( pStmt, 0 );
		bFound	= true;
	}

	sqlite3_finalize( pStmt );
	return bFound;
}

bool CX2OfflineDB::SetCashBalance( UidType nUserUID, int iBalance )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	if( iBalance < 0 )
		iBalance = 0;

	sqlite3_stmt* pStmt = Prepare( "UPDATE account SET cash_balance = ?2 WHERE user_uid = ?1;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUserUID );
	sqlite3_bind_int(   pStmt, 2, iBalance );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	if( false == bOK )
		LogError( L"set cash balance" );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::LoadCashProducts( OUT std::vector< KOfflineCashProductRow >& vecOut )
{
	KLocker lock( m_cs );

	vecOut.clear();

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"SELECT product_no, item_id, category, quantity, price, is_event"
		" FROM cash_product ORDER BY product_no;" );
	if( NULL == pStmt )
		return false;

	while( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		KOfflineCashProductRow kRow;
		kRow.m_iProductNo	= sqlite3_column_int( pStmt, 0 );
		kRow.m_iItemID		= sqlite3_column_int( pStmt, 1 );
		kRow.m_iCategoryNo	= sqlite3_column_int( pStmt, 2 );
		kRow.m_iQuantity	= sqlite3_column_int( pStmt, 3 );
		kRow.m_iPrice		= sqlite3_column_int( pStmt, 4 );
		kRow.m_bEvent		= ( 0 != sqlite3_column_int( pStmt, 5 ) );

		vecOut.push_back( kRow );
	}

	sqlite3_finalize( pStmt );
	return true;
}

bool CX2OfflineDB::LoadCashOrders( UidType nUserUID, OUT std::vector< KOfflineCashOrderRow >& vecOut )
{
	KLocker lock( m_cs );

	vecOut.clear();

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"SELECT trans_no, product_no, item_id, quantity, period, price, buy_date"
		" FROM cash_order WHERE user_uid = ?1 ORDER BY trans_no;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUserUID );

	while( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		KOfflineCashOrderRow kRow;
		kRow.m_nTransNo		= (__int64)sqlite3_column_int64( pStmt, 0 );
		kRow.m_iProductNo	= sqlite3_column_int( pStmt, 1 );
		kRow.m_iItemID		= sqlite3_column_int( pStmt, 2 );
		kRow.m_iQuantity	= sqlite3_column_int( pStmt, 3 );
		kRow.m_iPeriod		= sqlite3_column_int( pStmt, 4 );
		kRow.m_iPrice		= sqlite3_column_int( pStmt, 5 );
		kRow.m_tBuyDate		= (__int64)sqlite3_column_int64( pStmt, 6 );

		vecOut.push_back( kRow );
	}

	sqlite3_finalize( pStmt );
	return true;
}

bool CX2OfflineDB::InsertCashOrder( UidType nUserUID, IN OUT KOfflineCashOrderRow& kInOut )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"INSERT INTO cash_order( user_uid, product_no, item_id, quantity, period, price, buy_date )"
		" VALUES( ?1, ?2, ?3, ?4, ?5, ?6, ?7 );" );
	if( NULL == pStmt )
		return false;

	if( 0 == kInOut.m_tBuyDate )
		kInOut.m_tBuyDate = NowEpoch();

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUserUID );
	sqlite3_bind_int(   pStmt, 2, kInOut.m_iProductNo );
	sqlite3_bind_int(   pStmt, 3, kInOut.m_iItemID );
	sqlite3_bind_int(   pStmt, 4, kInOut.m_iQuantity );
	sqlite3_bind_int(   pStmt, 5, kInOut.m_iPeriod );
	sqlite3_bind_int(   pStmt, 6, kInOut.m_iPrice );
	sqlite3_bind_int64( pStmt, 7, (sqlite3_int64)kInOut.m_tBuyDate );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	if( false == bOK )
		LogError( L"insert cash order" );

	sqlite3_finalize( pStmt );

	if( true == bOK )
		kInOut.m_nTransNo = (__int64)sqlite3_last_insert_rowid( m_pDB );

	return bOK;
}

bool CX2OfflineDB::DeleteCashOrder( UidType nUserUID, __int64 nTransNo )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	// user_uid is in the WHERE as well as trans_no. The trans number is the
	// primary key so it is enough on its own; naming the account too means a
	// malformed request cannot reach a line that is not this account's.
	sqlite3_stmt* pStmt = Prepare( "DELETE FROM cash_order WHERE user_uid = ?1 AND trans_no = ?2;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUserUID );
	sqlite3_bind_int64( pStmt, 2, (sqlite3_int64)nTransNo );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );

	// sqlite3_changes tells "there was no such line" apart from a SQL error, and
	// the caller has to know: claiming a line that is not there must not hand
	// out an item.
	return bOK && ( 0 < sqlite3_changes( m_pDB ) );
}

bool CX2OfflineDB::LoadWishList( UidType nUserUID, OUT std::vector< int >& vecOut )
{
	KLocker lock( m_cs );

	vecOut.clear();

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare( "SELECT item_id FROM wish_list WHERE user_uid = ?1 ORDER BY item_id;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUserUID );

	while( SQLITE_ROW == sqlite3_step( pStmt ) )
		vecOut.push_back( sqlite3_column_int( pStmt, 0 ) );

	sqlite3_finalize( pStmt );
	return true;
}

bool CX2OfflineDB::SetWishListItem( UidType nUserUID, int iItemID, bool bAdd )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare( true == bAdd
		? "INSERT OR IGNORE INTO wish_list( user_uid, item_id ) VALUES( ?1, ?2 );"
		: "DELETE FROM wish_list WHERE user_uid = ?1 AND item_id = ?2;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUserUID );
	sqlite3_bind_int(   pStmt, 2, iItemID );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	if( false == bOK )
		LogError( L"set wish list item" );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::GetSetting( const char* szKey, OUT int& iOut )
{
	KLocker lock( m_cs );

	iOut = 0;

	if( NULL == m_pDB || NULL == szKey )
		return false;

	sqlite3_stmt* pStmt = Prepare( "SELECT value FROM settings WHERE name = ?1;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_text( pStmt, 1, szKey, -1, SQLITE_TRANSIENT );

	bool bFound = false;
	if( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		iOut	= sqlite3_column_int( pStmt, 0 );
		bFound	= true;
	}

	sqlite3_finalize( pStmt );
	return bFound;
}

bool CX2OfflineDB::SetSetting( const char* szKey, int iValue )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB || NULL == szKey )
		return false;

	sqlite3_stmt* pStmt = Prepare( "INSERT OR REPLACE INTO settings( name, value ) VALUES( ?1, ?2 );" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_text( pStmt, 1, szKey, -1, SQLITE_TRANSIENT );
	sqlite3_bind_int(  pStmt, 2, iValue );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );
	return bOK;
}

//////////////////////////////////////////////////////////////////////////
// pets and riding pets (phase 7)

bool CX2OfflineDB::LoadPets( UidType nUnitUID, OUT std::vector< KOfflinePetRow >& vecOut )
{
	KLocker lock( m_cs );

	vecOut.clear();

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"SELECT pet_uid, pet_id, name, evolution_step, satiety, intimacy, extroversion,"
		" emotion, auto_feed, auto_looting, last_feed, last_summon, reg_date"
		" FROM unit_pet WHERE unit_uid = ?1 ORDER BY pet_uid;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );

	while( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		KOfflinePetRow kRow;
		kRow.m_nPetUID			= (__int64)sqlite3_column_int64( pStmt, 0 );
		kRow.m_iPetID			= sqlite3_column_int( pStmt, 1 );
		kRow.m_wstrName			= ColumnText( pStmt, 2 );
		kRow.m_iEvolutionStep	= sqlite3_column_int( pStmt, 3 );
		kRow.m_iSatiety			= sqlite3_column_int( pStmt, 4 );
		kRow.m_iIntimacy		= sqlite3_column_int( pStmt, 5 );
		kRow.m_iExtroversion	= sqlite3_column_int( pStmt, 6 );
		kRow.m_iEmotion			= sqlite3_column_int( pStmt, 7 );
		kRow.m_bAutoFeed		= ( 0 != sqlite3_column_int( pStmt, 8 ) );
		kRow.m_bAutoLooting		= ( 0 != sqlite3_column_int( pStmt, 9 ) );
		kRow.m_tLastFeedDate	= (__int64)sqlite3_column_int64( pStmt, 10 );
		kRow.m_tLastSummonDate	= (__int64)sqlite3_column_int64( pStmt, 11 );
		kRow.m_tRegDate			= (__int64)sqlite3_column_int64( pStmt, 12 );

		vecOut.push_back( kRow );
	}

	sqlite3_finalize( pStmt );
	return true;
}

bool CX2OfflineDB::InsertPet( UidType nUnitUID, IN OUT KOfflinePetRow& kInOut )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"INSERT INTO unit_pet( unit_uid, pet_id, name, evolution_step, satiety, intimacy,"
		" extroversion, emotion, auto_feed, auto_looting, last_feed, last_summon, reg_date )"
		" VALUES( ?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13 );" );
	if( NULL == pStmt )
		return false;

	if( 0 == kInOut.m_tRegDate )
		kInOut.m_tRegDate = NowEpoch();

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int(   pStmt, 2, kInOut.m_iPetID );
	BindText(           pStmt, 3, kInOut.m_wstrName );
	sqlite3_bind_int(   pStmt, 4, kInOut.m_iEvolutionStep );
	sqlite3_bind_int(   pStmt, 5, kInOut.m_iSatiety );
	sqlite3_bind_int(   pStmt, 6, kInOut.m_iIntimacy );
	sqlite3_bind_int(   pStmt, 7, kInOut.m_iExtroversion );
	sqlite3_bind_int(   pStmt, 8, kInOut.m_iEmotion );
	sqlite3_bind_int(   pStmt, 9, true == kInOut.m_bAutoFeed ? 1 : 0 );
	sqlite3_bind_int(   pStmt, 10, true == kInOut.m_bAutoLooting ? 1 : 0 );
	sqlite3_bind_int64( pStmt, 11, (sqlite3_int64)kInOut.m_tLastFeedDate );
	sqlite3_bind_int64( pStmt, 12, (sqlite3_int64)kInOut.m_tLastSummonDate );
	sqlite3_bind_int64( pStmt, 13, (sqlite3_int64)kInOut.m_tRegDate );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	if( false == bOK )
		LogError( L"insert pet" );

	sqlite3_finalize( pStmt );

	if( true == bOK )
		kInOut.m_nPetUID = (__int64)sqlite3_last_insert_rowid( m_pDB );

	return bOK;
}

bool CX2OfflineDB::SavePet( UidType nUnitUID, const KOfflinePetRow& kRow )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"UPDATE unit_pet SET pet_id = ?3, name = ?4, evolution_step = ?5, satiety = ?6,"
		" intimacy = ?7, extroversion = ?8, emotion = ?9, auto_feed = ?10, auto_looting = ?11,"
		" last_feed = ?12, last_summon = ?13"
		" WHERE unit_uid = ?1 AND pet_uid = ?2;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int64( pStmt, 2, (sqlite3_int64)kRow.m_nPetUID );
	sqlite3_bind_int(   pStmt, 3, kRow.m_iPetID );
	BindText(           pStmt, 4, kRow.m_wstrName );
	sqlite3_bind_int(   pStmt, 5, kRow.m_iEvolutionStep );
	sqlite3_bind_int(   pStmt, 6, kRow.m_iSatiety );
	sqlite3_bind_int(   pStmt, 7, kRow.m_iIntimacy );
	sqlite3_bind_int(   pStmt, 8, kRow.m_iExtroversion );
	sqlite3_bind_int(   pStmt, 9, kRow.m_iEmotion );
	sqlite3_bind_int(   pStmt, 10, true == kRow.m_bAutoFeed ? 1 : 0 );
	sqlite3_bind_int(   pStmt, 11, true == kRow.m_bAutoLooting ? 1 : 0 );
	sqlite3_bind_int64( pStmt, 12, (sqlite3_int64)kRow.m_tLastFeedDate );
	sqlite3_bind_int64( pStmt, 13, (sqlite3_int64)kRow.m_tLastSummonDate );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	if( false == bOK )
		LogError( L"save pet" );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::LoadRidingPets( UidType nUnitUID, OUT std::vector< KOfflineRidingPetRow >& vecOut )
{
	KLocker lock( m_cs );

	vecOut.clear();

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"SELECT riding_pet_uid, riding_pet_id, stamina, destroy_date, last_unsummon"
		" FROM unit_riding_pet WHERE unit_uid = ?1 ORDER BY riding_pet_uid;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );

	while( SQLITE_ROW == sqlite3_step( pStmt ) )
	{
		KOfflineRidingPetRow kRow;
		kRow.m_nRidingPetUID		= (__int64)sqlite3_column_int64( pStmt, 0 );
		kRow.m_iRidingPetID			= sqlite3_column_int( pStmt, 1 );
		kRow.m_fStamina				= (float)sqlite3_column_double( pStmt, 2 );
		kRow.m_tDestroyDate			= (__int64)sqlite3_column_int64( pStmt, 3 );
		kRow.m_tLastUnSummonDate	= (__int64)sqlite3_column_int64( pStmt, 4 );

		vecOut.push_back( kRow );
	}

	sqlite3_finalize( pStmt );
	return true;
}

bool CX2OfflineDB::InsertRidingPet( UidType nUnitUID, IN OUT KOfflineRidingPetRow& kInOut )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"INSERT INTO unit_riding_pet( unit_uid, riding_pet_id, stamina, destroy_date, last_unsummon )"
		" VALUES( ?1, ?2, ?3, ?4, ?5 );" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64(  pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int(    pStmt, 2, kInOut.m_iRidingPetID );
	sqlite3_bind_double( pStmt, 3, (double)kInOut.m_fStamina );
	sqlite3_bind_int64(  pStmt, 4, (sqlite3_int64)kInOut.m_tDestroyDate );
	sqlite3_bind_int64(  pStmt, 5, (sqlite3_int64)kInOut.m_tLastUnSummonDate );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	if( false == bOK )
		LogError( L"insert riding pet" );

	sqlite3_finalize( pStmt );

	if( true == bOK )
		kInOut.m_nRidingPetUID = (__int64)sqlite3_last_insert_rowid( m_pDB );

	return bOK;
}

bool CX2OfflineDB::SaveRidingPet( UidType nUnitUID, const KOfflineRidingPetRow& kRow )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"UPDATE unit_riding_pet SET riding_pet_id = ?3, stamina = ?4, destroy_date = ?5,"
		" last_unsummon = ?6 WHERE unit_uid = ?1 AND riding_pet_uid = ?2;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64(  pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int64(  pStmt, 2, (sqlite3_int64)kRow.m_nRidingPetUID );
	sqlite3_bind_int(    pStmt, 3, kRow.m_iRidingPetID );
	sqlite3_bind_double( pStmt, 4, (double)kRow.m_fStamina );
	sqlite3_bind_int64(  pStmt, 5, (sqlite3_int64)kRow.m_tDestroyDate );
	sqlite3_bind_int64(  pStmt, 6, (sqlite3_int64)kRow.m_tLastUnSummonDate );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	if( false == bOK )
		LogError( L"save riding pet" );

	sqlite3_finalize( pStmt );
	return bOK;
}

bool CX2OfflineDB::DeleteRidingPet( UidType nUnitUID, __int64 nRidingPetUID )
{
	KLocker lock( m_cs );

	if( NULL == m_pDB )
		return false;

	sqlite3_stmt* pStmt = Prepare(
		"DELETE FROM unit_riding_pet WHERE unit_uid = ?1 AND riding_pet_uid = ?2;" );
	if( NULL == pStmt )
		return false;

	sqlite3_bind_int64( pStmt, 1, (sqlite3_int64)nUnitUID );
	sqlite3_bind_int64( pStmt, 2, (sqlite3_int64)nRidingPetUID );

	bool bOK = ( SQLITE_DONE == sqlite3_step( pStmt ) );

	sqlite3_finalize( pStmt );
	return bOK && ( 0 < sqlite3_changes( m_pDB ) );
}

#endif SERV_IRUHADEV_OFFLINE
