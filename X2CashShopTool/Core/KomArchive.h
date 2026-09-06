//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 1 - a fresh reader for V.0.3 .kom
// archives, plus the name -> (archive, member) index over all 145 of them.
// See CASH_SHOP_TOOL_PLAN.md section 3 and its "Trap: every standalone .kom
// reader in this tree is stale" note - X2MassFileTool and X2KomFileViewer
// both handle archive versions 1 and 2 only and would silently misparse
// every shipped archive, and the one correct reader
// (KTDXLIB/KGCMassFileManager.cpp:1355) drags in MFC and the whole engine.
//
// NO third-party header is included here. This header is included from the
// /clr Ui project, and zlib/lua/sqlite3 must stay confined to the .cpp side
// of the split (plan section 6).
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <vector>
#include <map>

//////////////////////////////////////////////////////////////////////////
// Progress sink. Loading Item.lua takes seconds, so the long operations
// report as they go rather than only when they finish. Main.cpp implements
// this over the console; the phase 4 window will implement it over a status
// bar.
class IToolLog
{
public:
	virtual void Line( const char* pszText ) = 0;
	void Linef( const char* pszFormat, ... );

protected:
	~IToolLog() {}
};

//////////////////////////////////////////////////////////////////////////

struct SKomMember
{
	// Uppercased, because that is what the engine matches on: LoadDataFile
	// calls MakeUpperCase before the map lookup
	// (KTDXLIB/KGCMassFileManager.cpp:653).
	std::string	strName;
	__int64		iOffset;		// of the compressed payload, from the start of the .kom
	long		lCompSize;
	long		lStatedSize;	// the manifest's Size attribute - a HINT ONLY, see the .cpp

	SKomMember() : iOffset( 0 ), lCompSize( 0 ), lStatedSize( 0 ) {}
};

// One archive. Opening reads the header and the XML manifest and nothing
// else; payload bytes are read on demand, with the file opened and closed
// per read so the tool never holds 145 handles or interferes with a running
// client.
class CKomArchive
{
public:
	CKomArchive();

	bool							Open( const std::wstring& wstrPath, std::string& strError );

	// Inflates member uIndex. Does NOT decrypt: .lua members are additionally
	// XOR-encrypted and .dds members are not, so that choice belongs to the
	// caller (see LuaXor.h).
	bool							ReadMember( size_t uIndex, std::vector<char>& vecOut, std::string& strError ) const;

	const std::vector<SKomMember>&	Members() const		{ return m_vecMembers; }
	const std::wstring&				Path() const		{ return m_wstrPath; }
	__int64							FileSize() const	{ return m_iFileSize; }
	__int64							MTime() const		{ return m_iMTime; }

private:
	bool							ParseManifest( const char* pXml, size_t uXmlLen, __int64 iFirstOffset, std::string& strError );

	std::wstring			m_wstrPath;
	std::vector<SKomMember>	m_vecMembers;
	__int64					m_iFileSize;
	__int64					m_iMTime;
};

// All 145 archives, mounted in the same order the client mounts them
// (data001.kom .. data145.kom, X2Lib/X2Main.cpp:770-787), so that the
// first archive to claim a name wins here exactly as it does there.
class CKomIndex
{
public:
	CKomIndex();

	bool				Mount( const std::wstring& wstrDir, IToolLog* pLog );

	// pszName is matched case-insensitively (uppercased internally), so
	// callers may pass "Item.lua" as it appears in the manifest.
	bool				Contains( const char* pszName ) const;
	bool				Read( const char* pszName, std::vector<char>& vecOut, std::string& strError ) const;
	const CKomArchive*	ArchiveFor( const char* pszName ) const;

	size_t				ArchiveCount() const	{ return m_vecArchives.size(); }
	size_t				NameCount() const		{ return m_mapName.size(); }
	size_t				ShadowedCount() const	{ return m_uShadowed; }

	// For walking every member of every archive - which is what building
	// the icon locator does (phase 2). Only the archives that actually
	// opened are in here, so the index may be shorter than 145.
	const CKomArchive&	ArchiveAt( size_t uIndex ) const	{ return m_vecArchives[uIndex]; }

private:
	struct SLocation
	{
		size_t	uArchive;
		size_t	uMember;
	};

	const SLocation*			Find( const char* pszName ) const;

	std::vector<CKomArchive>		m_vecArchives;
	std::map<std::string, SLocation>	m_mapName;
	size_t							m_uShadowed;
};

//////////////////////////////////////////////////////////////////////////
// Small shared helpers

// Size and last-write time of a file, without opening a handle. Both are 0
// when the file does not exist. These two are what the index cache keys its
// validity on - a content hash of hundreds of megabytes would cost more
// than the rebuild it is meant to avoid (plan, "The item index is a cache").
bool			GetFileStamp( const std::wstring& wstrPath, __int64* piSize, __int64* piMTime );

std::string		NarrowPath( const std::wstring& wstr );
std::wstring	WidenPath( const std::string& str );
std::string		UpperAscii( const std::string& str );

// dir + leaf, inserting a separator only when the directory does not
// already end in one. Four copies of this had accumulated across the
// tool by phase 2; one is enough.
std::wstring	JoinPath( const std::wstring& wstrDir, const std::wstring& wstrLeaf );

// Reads and inflates one member given only its coordinates, with no
// CKomArchive in hand. This is what lets the icon store work straight off
// the cached locator table - resolving an icon then costs one open, one
// seek and one inflate, and never a re-mount of the 145 manifests.
//
// uSizeHint is the manifest's Size attribute, used only as the initial
// output capacity: 121 of the 88,723 members state a Size smaller than
// what they really inflate to, so the buffer grows rather than trusting it.
bool			InflateMemberAt( const std::wstring& wstrArchivePath, __int64 iOffset,
									long lCompSize, size_t uSizeHint,
									std::vector<char>& vecOut, std::string& strError );

// QueryPerformanceCounter wrapper. Shared because both the extractor and
// the icon locator report their own timings, and a phase that cannot say
// how long it took cannot be judged.
class CToolStopwatch
{
public:
	CToolStopwatch();

	void	Restart();
	double	Seconds() const;

private:
	__int64	m_iStart;
	__int64	m_iFrequency;
};
