//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 1 - the extracted catalog, cached in a
// SQLite file next to the tool exe. See CASH_SHOP_TOOL_PLAN.md, "The item
// index is a cache, and it is the design - not an optimization".
//
// Deliberately NOT in the game directory: a stray .db beside els_db.sql is
// the sort of thing someone deletes at 2am, and this one is rebuildable
// while that one is the only copy of the character.
//
// No sqlite3.h here - see the note at the top of KomArchive.h.
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>

#include "ItemIndex.h"

struct sqlite3;

class CIndexCache
{
public:
	CIndexCache();
	~CIndexCache();

	bool	Open( const std::wstring& wstrPath, std::string& strError );
	void	Close();

	// True when this cache was built by this build of the extractor, from
	// this archive, at the size and mtime it currently has. Any one of the
	// four differing means rebuild.
	//
	// extractor_version is the input people forget: without it an index
	// built before a new field was captured looks perfectly valid and is
	// quietly missing a column.
	bool	IsCurrent( const std::wstring& wstrArchivePath, std::string& strReason ) const;

	bool	Store( const SExtractResult& kResult, const std::wstring& wstrArchivePath, std::string& strError );
	bool	Load( SExtractResult& kResult, std::string& strError ) const;

	const std::wstring&	Path() const	{ return m_wstrPath; }

private:
	CIndexCache( const CIndexCache& );				// not copyable: it owns a handle
	CIndexCache& operator=( const CIndexCache& );

	bool	Exec( const char* pszSql, std::string& strError );
	bool	CreateSchema( std::string& strError );
	bool	ReadMeta( const char* pszKey, std::string& strValue ) const;
	bool	WriteMeta( const char* pszKey, const std::string& strValue, std::string& strError );

	sqlite3*		m_pDb;
	std::wstring	m_wstrPath;
};

// <directory of the running exe>\X2CashShopIndex.db
std::wstring	DefaultCachePath();
