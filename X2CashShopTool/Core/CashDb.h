//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 3 - the database layer over the game's
// own save file, els_db.sql. See CASH_SHOP_TOOL_PLAN.md, "Phase 3 - The
// database layer".
//
// This is the one file in the tool that WRITES anything, and what it writes
// into is the only copy of the character. Every rule below is there because
// of that:
//
//   * it refuses a save whose PRAGMA user_version is not 11 - X2OfflineDB
//     migrates forward only, and a tool writing into a schema it does not
//     know is how a save gets corrupted (X2Lib/Offline/X2OfflineDB.h:399);
//   * it refuses to open at all while another process holds the file, so a
//     backup can never be taken across the client's own write;
//   * it copies the WHOLE WAL SET into db_backup/ before the first write of
//     a session, not just els_db.sql - copying the main file alone once
//     yielded a snapshot reading schema v6 while the live DB was at v7;
//   * it checkpoints WAL into the main file on close, so the save is left
//     the way the client leaves it.
//
// No sqlite3.h here - see the note at the top of KomArchive.h. This header
// is included from the /clr Ui project.
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <vector>

#include "ItemIndex.h"

struct sqlite3;

//////////////////////////////////////////////////////////////////////////
// One row of `cash_product` (X2Lib/Offline/X2OfflineDB.cpp:293-302).
//
//   CREATE TABLE cash_product (
//     product_no INTEGER PRIMARY KEY,   -- NOT autoincrement; the tool allocates max+1
//     item_id    INTEGER NOT NULL,
//     category   INTEGER NOT NULL,      -- billing category, NOT the CSSC_* enum
//     quantity   INTEGER NOT NULL DEFAULT 1,
//     price      INTEGER NOT NULL DEFAULT 1,
//     is_event   INTEGER NOT NULL DEFAULT 0 );
struct SCashProductRow
{
	int	iProductNo;
	int	iItemID;
	int	iCategoryNo;
	int	iQuantity;
	int	iPrice;
	int	iIsEvent;

	SCashProductRow()
	: iProductNo( 0 ), iItemID( 0 ), iCategoryNo( 0 )
	, iQuantity( 1 ), iPrice( 1 ), iIsEvent( 0 )
	{}
};

// The limits on `category` and `quantity`, in one place.
//
// CX2OfflineCashShop::MakeProductInfo narrows BOTH to a signed char on the
// wire - m_cCategoryNo at X2Lib/Offline/X2OfflineCashShop.cpp:181 and
// m_cQuantity at :185 - so 128 arrives at the client as -128 and matches no
// tab and buys nothing. The lower bound is :94, which silently rewrites a
// non-positive quantity to 1.
//
// ENFORCED, never clamped. A tool that clamps just moves the surprise from
// the moment of typing to the moment of playing.
enum
{
	CASH_FIELD_MIN	= 1,
	CASH_FIELD_MAX	= 127,
};

// What Open() decided. Anything but CashDb_OK means nothing was opened.
enum ECashDbResult
{
	CashDb_OK	= 0,
	CashDb_NoFile,			///< there is no els_db.sql here
	CashDb_Locked,			///< another process holds it - almost always X2_offline.exe
	CashDb_NotASave,		///< opened, but has no cash_product table
	CashDb_WrongSchema,		///< PRAGMA user_version is not 11
	CashDb_SqliteError,

	CashDbResult_Count
};

const char*	CashDbResultName( ECashDbResult eResult );

//////////////////////////////////////////////////////////////////////////

class CCashDb
{
public:
	CCashDb();
	~CCashDb();

	// bReadOnly opens with SQLITE_OPEN_READONLY, so a report cannot write
	// even by accident. Either way the lock check runs first: the plan says
	// refuse if the client holds the file, and it says so without an
	// exception for reads, because a catalog read while the game is running
	// answers a question about a shop the running game has already cached.
	ECashDbResult	Open( const std::wstring& wstrPath, bool bReadOnly, std::string& strError );

	// Checkpoints WAL into the main file (TRUNCATE) before closing, so the
	// save is left in the same shape the client leaves it in.
	void			Close();

	bool						IsOpen() const		{ return NULL != m_pDb; }
	bool						IsReadOnly() const	{ return m_bReadOnly; }
	int							UserVersion() const	{ return m_iUserVersion; }
	const std::wstring&			Path() const		{ return m_wstrPath; }

	// X2Lib/Offline/X2OfflineDB.h:399. Read from the header's own value
	// would be better still, but this tool deliberately compiles against
	// none of X2Lib.
	static int					RequiredUserVersion();

	//////////////////////////////////////////////////////////////////////
	// Reads

	bool	LoadProducts( std::vector<SCashProductRow>& vecOut, std::string& strError ) const;
	bool	FindProduct( int iProductNo, SCashProductRow& kOut, std::string& strError ) const;

	// settings.cash_start - the wallet the shop displays. Cosmetic: a
	// purchase is checked against it but never deducts it
	// (X2Lib/Offline/X2OfflineDB.h:428-430).
	bool	GetWallet( int& iOut, std::string& strError ) const;

	// max( product_no ) + 1. product_no is a plain INTEGER PRIMARY KEY, not
	// AUTOINCREMENT, so nothing allocates it but this.
	bool	NextProductNo( int& iOut, std::string& strError ) const;

	int		ProductCount( std::string& strError ) const;

	//////////////////////////////////////////////////////////////////////
	// Writes. Every one of these takes the session backup first.

	bool	SetWallet( int iValue, std::string& strError );

	// kRow.iProductNo is filled in with the allocated number when it is 0 on
	// the way in, and honoured when it is not.
	bool	Insert( SCashProductRow& kRow, std::string& strError );
	bool	Update( const SCashProductRow& kRow, std::string& strError );
	bool	Delete( int iProductNo, std::string& strError );

	//////////////////////////////////////////////////////////////////////
	// Validation. Shared with the Ui so a bad value is refused where it is
	// typed rather than at the moment of writing.

	bool	Validate( const SCashProductRow& kRow, bool bIsInsert, std::string& strError ) const;

	// Hand the phase 1 catalog over and inserts additionally check that the
	// item exists. Without it Validate says so rather than pretending.
	// vecItems must be sorted by iItemID, which ExtractItemCatalog and
	// CIndexCache::Load both guarantee.
	void	SetKnownItems( const std::vector<SItemRow>& vecItems );
	bool	HasKnownItems() const	{ return false == m_vecKnownItems.empty(); }
	bool	KnowsItem( int iItemID ) const;

	//////////////////////////////////////////////////////////////////////
	// The backup taken before the first write of a session.

	// Public so the Ui can take it up front and show the path, rather than
	// having it happen invisibly inside the first edit. Idempotent.
	bool	EnsureBackup( std::string& strError );

	// Carries a backup already taken this RUN across a Close()/Open() pair.
	//
	// Open() resets the flag, because a fresh connection that writes must be
	// able to guarantee a pre-write snapshot exists. But the tool reopens
	// several times in one run, and without this each reopen would take
	// another backup - clutter at best, and it is what turned up the defect
	// this exists alongside: two backups within one second used to collide
	// on the label and the second overwrote the first.
	void	AdoptBackup( const std::wstring& wstrLabel, const std::vector<std::wstring>& vecFiles );

	bool								BackupTaken() const		{ return m_bBackupTaken; }
	const std::wstring&					BackupLabel() const		{ return m_wstrBackupLabel; }
	const std::vector<std::wstring>&	BackupFiles() const		{ return m_vecBackupFiles; }

private:
	CCashDb( const CCashDb& );				// not copyable: it owns a handle
	CCashDb& operator=( const CCashDb& );

	bool	Exec( const char* pszSql, std::string& strError );
	bool	BeginWrite( std::string& strError );
	bool	CommitWrite( std::string& strError );
	void	RollbackWrite();
	bool	ReadUserVersion( int& iOut, std::string& strError ) const;
	bool	HasTable( const char* pszName ) const;
	void	Checkpoint();

	sqlite3*		m_pDb;
	std::wstring	m_wstrPath;
	bool			m_bReadOnly;
	int				m_iUserVersion;

	bool						m_bBackupTaken;
	std::wstring				m_wstrBackupLabel;
	std::vector<std::wstring>	m_vecBackupFiles;

	std::vector<int>			m_vecKnownItems;	///< sorted item ids, for insert-time validation
};

//////////////////////////////////////////////////////////////////////////
// The dropped-row report.
//
// This is the single most useful thing the tool can say, and it costs one
// join: a cash_product row whose item_id has no client templet is discarded
// at load with nothing but a log line
// (X2Lib/Offline/X2OfflineCashShop.cpp:85-89), so 16% of this catalog is
// invisible in the game and nothing on screen says why.
//
// The item catalog this joins against holds exactly the items the CLIENT
// holds, by construction (see SExtractResult's comment), so the number this
// produces must equal the client's own - which is the check that proves the
// join is right.

struct SCashProblemRow
{
	int			iProductNo;
	int			iItemID;
	int			iCategoryNo;
	std::string	strDetail;

	SCashProblemRow() : iProductNo( 0 ), iItemID( 0 ), iCategoryNo( 0 ) {}
};

struct SCashCategoryCount
{
	int		iCategoryNo;
	int		iRows;
	int		iDropped;
	bool	bInTabTable;	///< false = matches no CashShopCategory.lua pair, so no tab shows it
	int		iTabIdx;

	SCashCategoryCount()
	: iCategoryNo( 0 ), iRows( 0 ), iDropped( 0 ), bInTabTable( false ), iTabIdx( 0 )
	{}
};

struct SCashCatalogReport
{
	int	iRows;			///< cash_product rows read
	int	iKept;			///< rows the client will show
	int	iDropped;		///< rows the client discards for having no item templet

	std::vector<SCashProblemRow>	vecDropped;
	std::vector<SCashProblemRow>	vecOutOfRange;		///< category or quantity outside 1..127
	std::vector<SCashCategoryCount>	vecCategories;		///< ascending by category number

	int	iOrphanCategories;	///< distinct categories matching no tab
	int	iOrphanRows;		///< rows in them
	int	iDuplicateItems;	///< items sold as more than one product - DELIBERATE, not a defect

	int	iMinPrice;
	int	iMaxPrice;

	SCashCatalogReport()
	: iRows( 0 ), iKept( 0 ), iDropped( 0 )
	, iOrphanCategories( 0 ), iOrphanRows( 0 ), iDuplicateItems( 0 )
	, iMinPrice( 0 ), iMaxPrice( 0 )
	{}
};

// vecItems must be sorted by iItemID. vecCategories is CashShopCategory.lua
// as phase 1 parsed it; pass it empty and the tab columns simply read as
// unknown rather than as orphaned.
void	BuildCashCatalogReport( const std::vector<SCashProductRow>& vecProducts,
								const std::vector<SItemRow>& vecItems,
								const std::vector<SCashCategoryRow>& vecCategories,
								SCashCatalogReport& kOut );

//////////////////////////////////////////////////////////////////////////
// Copying a save, WAL set and all.
//
// Used by EnsureBackup and by phase 3's own exit test, which does its first
// round-trip against a COPY of the save and only touches the live one after
// that has passed.
//
// The three names follow the convention db_backup/ already uses:
//   els_db.sql.bak-pre-<label>
//   els_db.sql-wal.bak-pre-<label>
//   els_db.sql-shm.bak-pre-<label>
//
// wstrLabel is IN/OUT: a label already used in that directory gets a "-2",
// "-3" ... suffix, and the copies are bFailIfExists, so a backup can never
// overwrite another backup. See the comment on the definition.
bool	BackupSaveSet( const std::wstring& wstrDbPath, const std::wstring& wstrBackupDir,
						std::wstring& wstrLabel,
						std::vector<std::wstring>& vecWritten, std::string& strError );

// A plain copy of the same three files to a new base name, for the scratch
// copy the self-test works on. Overwrites.
bool	CopySaveSet( const std::wstring& wstrSrcDb, const std::wstring& wstrDstDb, std::string& strError );

// db_backup, beside the save. Created if missing.
std::wstring	DefaultBackupDir( const std::wstring& wstrDbPath );

// "cashtool-20260906-171530"
std::wstring	MakeBackupLabel();

// The save file the offline client writes, as X2OfflineDB names it.
const wchar_t*	SaveFileName();
