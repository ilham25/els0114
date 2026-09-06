//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 2 - resolves an item's m_ShopImage to
// a decoded 64x64 bitmap, with the client's own HQ_Shop_Ui_Noimage.dds
// fallback and a bounded LRU so browsing does not accumulate 48,000
// bitmaps. See CASH_SHOP_TOOL_PLAN.md, "Phase 2 - Icons".
//
// The fallback rule is the client's, not an invention:
// CX2SlotItem::CreateItemUI asks IsValidFile( pItemTemplet->GetShopImage() )
// and falls back to L"HQ_Shop_Ui_Noimage.dds" when it says no
// (X2Lib/X2SlotItem.cpp:255-265).
//
// No third-party header here - this is included from the /clr Ui project.
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <list>
#include <map>
#include <string>
#include <vector>

#include "DdsDecode.h"
#include "KomArchive.h"

// Where one image lives, with no archive object needed to read it. This is
// what the cache's `icon` table stores, and it is built from manifests
// only - the 60-byte header, the three DWORDs and the XML, then stop. No
// payload is touched while indexing (plan, "The item index is a cache").
struct SIconLocation
{
	std::string	strName;		// uppercased, as the archive index keys it
	std::string	strKom;			// archive FILE NAME, e.g. "data067.kom", not a path
	__int64		iOffset;
	long		lCompSize;
	long		lStatedSize;

	SIconLocation() : iOffset( 0 ), lCompSize( 0 ), lStatedSize( 0 ) {}
};

// Size and mtime of one archive, so the locator can be invalidated without
// re-reading 145 manifests.
struct SKomStamp
{
	std::string	strKom;			// file name, matching SIconLocation::strKom
	__int64		iSize;
	__int64		iMTime;

	SKomStamp() : iSize( 0 ), iMTime( 0 ) {}
};

// What happened to one request. Counted rather than glanced at, because
// "the wall looks right" is not a measurement.
enum EIconOutcome
{
	IconOutcome_Decoded = 0,		// the item's own m_ShopImage decoded
	IconOutcome_FallbackNoName,		// m_ShopImage was empty
	IconOutcome_FallbackNotFound,	// named a file no archive holds - the client's IsValidFile == false
	IconOutcome_FallbackUndecodable,// the file is there but this decoder cannot read it
	IconOutcome_Failed,				// not even the fallback could be produced
	IconOutcome_Count
};

const char*	IconOutcomeName( EIconOutcome eOutcome );

struct SIconLookup
{
	EIconOutcome	eOutcome;
	EDdsFormat		eFormat;		// of whatever was actually decoded, fallback included
	EDdsError		eError;			// only meaningful for FallbackUndecodable / Failed
	std::string		strResolved;	// the name that actually got decoded
	std::string		strDetail;		// why, when something went wrong
	bool			bFromCache;		// served out of the LRU rather than re-inflated

	SIconLookup()
	: eOutcome( IconOutcome_Failed ), eFormat( DdsFormat_None )
	, eError( DdsError_None ), bFromCache( false )
	{}
};

//////////////////////////////////////////////////////////////////////////

class CIconStore
{
public:
	CIconStore();

	// Builds the locator by walking every member of every mounted archive
	// and keeping the .dds ones. Manifests only; nothing is read or decoded.
	void	Build( const CKomIndex& kIndex, IToolLog* pLog );

	// Adopts a locator that came out of the index cache instead. wstrDir is
	// the game directory the archive file names resolve against.
	void	Adopt( const std::vector<SIconLocation>& vecLocations, const std::wstring& wstrDir );

	void	SetDirectory( const std::wstring& wstrDir )	{ m_wstrDir = wstrDir; }

	const std::vector<SIconLocation>&	Locations() const	{ return m_vecLocations; }
	const std::vector<SKomStamp>&		Stamps() const		{ return m_vecStamps; }

	bool	IsEmpty() const		{ return m_mapByName.empty(); }
	size_t	Count() const		{ return m_mapByName.size(); }

	// True when the name is in the locator at all - the tool's equivalent
	// of the client's MassFileManager::IsValidFile, and the check that
	// decides between an item's own icon and the fallback.
	bool	HasImage( const char* pszShopImage ) const;

	// Decodes pszShopImage, falling back to HQ_Shop_Ui_Noimage.dds exactly
	// where the client would.
	//
	// THE RETURNED POINTER IS BORROWED, and is valid only until the next
	// call to Get on this store: the LRU may evict the entry behind it to
	// stay inside its byte budget. Copy the pixels out before calling
	// again - the Ui turns them straight into a Bitmap, which is the
	// pattern a virtualized grid wants anyway.
	const SDecodedImage*	Get( const char* pszShopImage, SIconLookup& kLookup );

	// The fallback on its own, pinned outside the LRU so the most-drawn
	// image in the tool can never be evicted. NULL if the archive does not
	// hold it, which would be a broken install.
	const SDecodedImage*	Fallback() const	{ return m_bHaveFallback ? &m_kFallback : NULL; }

	// LRU budget in bytes. 64x64 BGRA is 16 KB, so the default holds about
	// a thousand icons - far more than any grid shows at once, far less
	// than decoding the whole catalog.
	void	SetByteBudget( size_t uBytes );
	size_t	ByteBudget() const		{ return m_uByteBudget; }
	size_t	BytesHeld() const		{ return m_uBytesHeld; }
	size_t	EntriesHeld() const		{ return m_mapCache.size(); }

	int		CacheHits() const		{ return m_iHits; }
	int		CacheMisses() const		{ return m_iMisses; }
	int		CacheEvictions() const	{ return m_iEvictions; }
	void	ResetCounters();

	// Raw inflated bytes for a name, undecoded. The one shipped shop image
	// that is not a DDS at all (a PNG under a .dds name) is reachable this
	// way, so the Ui can hand it to GDI+ rather than showing the fallback
	// for a file that is perfectly good.
	bool	ReadRaw( const char* pszShopImage, std::vector<char>& vecOut, std::string& strError ) const;

	static const char*	FallbackImageName();

private:
	CIconStore( const CIconStore& );
	CIconStore& operator=( const CIconStore& );

	typedef std::list<std::string>						TLruList;	// front = most recently used
	typedef std::map<std::string, size_t>				TNameMap;

	struct SCacheEntry
	{
		SDecodedImage			kImage;
		TLruList::iterator		iterLru;
	};

	typedef std::map<std::string, SCacheEntry>			TCacheMap;

	void					RebuildNameMap();
	std::wstring			PathFor( const SIconLocation& kLocation ) const;
	const SIconLocation*	FindLocation( const std::string& strUpperName ) const;
	const SDecodedImage*	Decode( const std::string& strUpperName, SIconLookup& kLookup );
	void					Evict();
	void					LoadFallback();

	std::wstring					m_wstrDir;
	std::vector<SIconLocation>		m_vecLocations;
	std::vector<SKomStamp>			m_vecStamps;
	TNameMap						m_mapByName;	// upper name -> index into m_vecLocations

	TCacheMap						m_mapCache;
	TLruList						m_listLru;
	size_t							m_uByteBudget;
	size_t							m_uBytesHeld;

	SDecodedImage					m_kFallback;
	bool							m_bHaveFallback;
	bool							m_bTriedFallback;

	int								m_iHits;
	int								m_iMisses;
	int								m_iEvictions;
};

// Bumped when the SHAPE of the locator changes, the same discipline
// ItemExtractorVersion() follows. Kept separate from it on purpose: the
// item catalog comes out of data036.kom alone, while the locator depends on
// all 145 archives, so the two have genuinely different invalidation
// inputs and sharing one version number would force needless rebuilds of
// whichever half did not change.
int	IconLocatorVersion();
