//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 2 - icon locator and LRU implementation.
//////////////////////////////////////////////////////////////////////////
#include "IconStore.h"

#ifdef _MANAGED
#error "This TU compiled managed. /clr leaked in - see CppBuild.targets:733-738"
#endif

#include <stdio.h>
#include <string.h>

namespace
{
	// The client's own fallback (X2Lib/X2SlotItem.cpp:264). Present in
	// data067.kom in this install, 64x64 DXT5.
	const char* const	FALLBACK_NAME	= "HQ_SHOP_UI_NOIMAGE.DDS";

	const char* const	DDS_SUFFIX		= ".DDS";

	// 16 MB, i.e. about a thousand 64x64 icons. A grid never shows more
	// than a few dozen at once; this is sized so scrolling back up is a
	// hit rather than a re-inflate.
	const size_t		DEFAULT_BUDGET	= 16u * 1024u * 1024u;

	bool EndsWith( const std::string& str, const char* pszSuffix )
	{
		const size_t uSuffix = ::strlen( pszSuffix );
		if( str.size() < uSuffix )
			return false;

		return 0 == ::memcmp( str.c_str() + str.size() - uSuffix, pszSuffix, uSuffix );
	}

	std::string FileNameOf( const std::wstring& wstrPath )
	{
		const size_t uSlash = wstrPath.find_last_of( L"\\/" );
		const std::wstring wstrName = ( std::wstring::npos == uSlash )
			? wstrPath : wstrPath.substr( uSlash + 1 );

		return NarrowPath( wstrName );
	}
}

//////////////////////////////////////////////////////////////////////////

const char* IconOutcomeName( EIconOutcome eOutcome )
{
	switch( eOutcome )
	{
	case IconOutcome_Decoded:				return "decoded";
	case IconOutcome_FallbackNoName:		return "fallback (no m_ShopImage)";
	case IconOutcome_FallbackNotFound:		return "fallback (no such file)";
	case IconOutcome_FallbackUndecodable:	return "fallback (undecodable)";
	default:								break;
	}

	return "failed";
}

int IconLocatorVersion()
{
	// 1 - phase 2: name, kom file name, offset, comp size, stated size,
	//              over every .dds member of every mounted archive.
	return 1;
}

const char* CIconStore::FallbackImageName()
{
	return FALLBACK_NAME;
}

//////////////////////////////////////////////////////////////////////////

CIconStore::CIconStore()
: m_uByteBudget( DEFAULT_BUDGET )
, m_uBytesHeld( 0 )
, m_bHaveFallback( false )
, m_bTriedFallback( false )
, m_iHits( 0 )
, m_iMisses( 0 )
, m_iEvictions( 0 )
{
}

void CIconStore::ResetCounters()
{
	m_iHits			= 0;
	m_iMisses		= 0;
	m_iEvictions	= 0;
}

void CIconStore::SetByteBudget( size_t uBytes )
{
	m_uByteBudget = uBytes;
	Evict();
}

void CIconStore::Build( const CKomIndex& kIndex, IToolLog* pLog )
{
	CToolStopwatch kWatch;

	m_vecLocations.clear();
	m_vecStamps.clear();
	m_mapByName.clear();		// the first-wins test below reads it as it fills
	m_mapCache.clear();
	m_listLru.clear();
	m_uBytesHeld		= 0;
	m_bHaveFallback		= false;
	m_bTriedFallback	= false;

	size_t uTotalMembers = 0;

	for( size_t uArchive = 0; uArchive != kIndex.ArchiveCount(); ++uArchive )
	{
		const CKomArchive& kArchive = kIndex.ArchiveAt( uArchive );

		if( m_wstrDir.empty() )
		{
			// Every archive sits in the same directory, so the first one
			// settles it and the locator can store bare file names.
			const std::wstring& wstrPath = kArchive.Path();
			const size_t uSlash = wstrPath.find_last_of( L"\\/" );
			if( std::wstring::npos != uSlash )
				m_wstrDir = wstrPath.substr( 0, uSlash );
		}

		SKomStamp kStamp;
		kStamp.strKom	= FileNameOf( kArchive.Path() );
		kStamp.iSize	= kArchive.FileSize();
		kStamp.iMTime	= kArchive.MTime();
		m_vecStamps.push_back( kStamp );

		const std::vector<SKomMember>& vecMembers = kArchive.Members();
		uTotalMembers += vecMembers.size();

		for( size_t u = 0; u != vecMembers.size(); ++u )
		{
			if( false == EndsWith( vecMembers[u].strName, DDS_SUFFIX ) )
				continue;

			SIconLocation kLocation;
			kLocation.strName		= vecMembers[u].strName;
			kLocation.strKom		= kStamp.strKom;
			kLocation.iOffset		= vecMembers[u].iOffset;
			kLocation.lCompSize		= vecMembers[u].lCompSize;
			kLocation.lStatedSize	= vecMembers[u].lStatedSize;

			// First archive to claim a name wins, exactly as CKomIndex and
			// the engine's own map do - so a name already present is
			// skipped rather than overwritten.
			if( m_mapByName.end() != m_mapByName.find( kLocation.strName ) )
				continue;

			m_mapByName.insert( std::make_pair( kLocation.strName, m_vecLocations.size() ) );
			m_vecLocations.push_back( kLocation );
		}
	}

	// The map was filled as the vector grew, which is fine, but a rebuild
	// keeps the two provably in step.
	RebuildNameMap();

	if( NULL != pLog )
	{
		pLog->Linef( "icons    : %u .dds locator entr(ies) from %u member(s) across %u archive(s), %.2f s (manifests only)",
			(unsigned) m_vecLocations.size(), (unsigned) uTotalMembers,
			(unsigned) m_vecStamps.size(), kWatch.Seconds() );
	}
}

void CIconStore::Adopt( const std::vector<SIconLocation>& vecLocations, const std::wstring& wstrDir )
{
	m_vecLocations	= vecLocations;
	m_wstrDir		= wstrDir;
	m_vecStamps.clear();

	m_mapCache.clear();
	m_listLru.clear();
	m_uBytesHeld		= 0;
	m_bHaveFallback		= false;
	m_bTriedFallback	= false;

	RebuildNameMap();
}

void CIconStore::RebuildNameMap()
{
	m_mapByName.clear();

	for( size_t u = 0; u != m_vecLocations.size(); ++u )
		m_mapByName.insert( std::make_pair( m_vecLocations[u].strName, u ) );
}

std::wstring CIconStore::PathFor( const SIconLocation& kLocation ) const
{
	return JoinPath( m_wstrDir, WidenPath( kLocation.strKom ) );
}

const SIconLocation* CIconStore::FindLocation( const std::string& strUpperName ) const
{
	TNameMap::const_iterator iter = m_mapByName.find( strUpperName );
	if( m_mapByName.end() == iter )
		return NULL;

	return &m_vecLocations[iter->second];
}

bool CIconStore::HasImage( const char* pszShopImage ) const
{
	if( NULL == pszShopImage || '\0' == pszShopImage[0] )
		return false;

	return NULL != FindLocation( UpperAscii( std::string( pszShopImage ) ) );
}

bool CIconStore::ReadRaw( const char* pszShopImage, std::vector<char>& vecOut, std::string& strError ) const
{
	vecOut.clear();

	if( NULL == pszShopImage || '\0' == pszShopImage[0] )
	{
		strError = "no image name given";
		return false;
	}

	const SIconLocation* pLocation = FindLocation( UpperAscii( std::string( pszShopImage ) ) );
	if( NULL == pLocation )
	{
		strError = std::string( "'" ) + pszShopImage + "' is not in any mounted archive";
		return false;
	}

	return InflateMemberAt( PathFor( *pLocation ), pLocation->iOffset, pLocation->lCompSize,
		(size_t)( pLocation->lStatedSize > 0 ? pLocation->lStatedSize : 0 ), vecOut, strError );
}

void CIconStore::LoadFallback()
{
	if( m_bTriedFallback )
		return;

	m_bTriedFallback = true;

	const SIconLocation* pLocation = FindLocation( std::string( FALLBACK_NAME ) );
	if( NULL == pLocation )
		return;

	std::vector<char>	vecBytes;
	std::string			strError;

	if( false == InflateMemberAt( PathFor( *pLocation ), pLocation->iOffset, pLocation->lCompSize,
			(size_t)( pLocation->lStatedSize > 0 ? pLocation->lStatedSize : 0 ), vecBytes, strError ) )
	{
		return;
	}

	if( vecBytes.empty() )
		return;

	// Pinned outside the LRU: it is the single most-drawn image in the
	// tool, so evicting it would be the one eviction guaranteed to be wrong.
	m_bHaveFallback = DecodeDds( &vecBytes[0], vecBytes.size(), m_kFallback, NULL, NULL );
}

const SDecodedImage* CIconStore::Decode( const std::string& strUpperName, SIconLookup& kLookup )
{
	TCacheMap::iterator iterCached = m_mapCache.find( strUpperName );
	if( m_mapCache.end() != iterCached )
	{
		// Touch: move to the front of the LRU without re-decoding.
		m_listLru.erase( iterCached->second.iterLru );
		m_listLru.push_front( strUpperName );
		iterCached->second.iterLru = m_listLru.begin();

		++m_iHits;
		kLookup.bFromCache	= true;
		kLookup.eFormat		= iterCached->second.kImage.eFormat;
		return &iterCached->second.kImage;
	}

	const SIconLocation* pLocation = FindLocation( strUpperName );
	if( NULL == pLocation )
		return NULL;

	// A miss is "had to go to the archive", so it is counted here rather
	// than after the decode - a file that inflates and then fails to
	// decode cost exactly as much work as one that succeeded.
	++m_iMisses;

	std::vector<char>	vecBytes;
	std::string			strError;

	if( false == InflateMemberAt( PathFor( *pLocation ), pLocation->iOffset, pLocation->lCompSize,
			(size_t)( pLocation->lStatedSize > 0 ? pLocation->lStatedSize : 0 ), vecBytes, strError )
		|| vecBytes.empty() )
	{
		kLookup.eError		= DdsError_Corrupt;
		kLookup.strDetail	= strError.empty() ? "member inflated to nothing" : strError;
		return NULL;
	}

	// NOT XOR-decrypted, deliberately: the .lua members carry the XOR and
	// the .dds members do not - the bytes straight out of the archive
	// already begin 'DDS ' (plan section 4, and verified again here across
	// all 15,098 shop images the catalog names).
	SDecodedImage kImage;
	if( false == DecodeDds( &vecBytes[0], vecBytes.size(), kImage, &kLookup.eError, &kLookup.strDetail ) )
		return NULL;

	SCacheEntry kEntry;
	kEntry.kImage = kImage;

	m_listLru.push_front( strUpperName );
	kEntry.iterLru = m_listLru.begin();

	std::pair<TCacheMap::iterator, bool> kInserted =
		m_mapCache.insert( std::make_pair( strUpperName, kEntry ) );

	m_uBytesHeld += kInserted.first->second.kImage.vecBGRA.size();

	// Evicting BEFORE returning would risk evicting the entry just
	// inserted; the LRU front is this one, so it is the last candidate and
	// safe either way, but the order below makes that explicit.
	Evict();

	kLookup.eFormat = kInserted.first->second.kImage.eFormat;
	return &kInserted.first->second.kImage;
}

void CIconStore::Evict()
{
	while( m_uBytesHeld > m_uByteBudget && m_listLru.size() > 1 )
	{
		const std::string strVictim = m_listLru.back();

		TCacheMap::iterator iter = m_mapCache.find( strVictim );
		if( m_mapCache.end() != iter )
		{
			m_uBytesHeld -= iter->second.kImage.vecBGRA.size();
			m_mapCache.erase( iter );
		}

		m_listLru.pop_back();
		++m_iEvictions;
	}
}

const SDecodedImage* CIconStore::Get( const char* pszShopImage, SIconLookup& kLookup )
{
	kLookup = SIconLookup();

	LoadFallback();

	if( NULL != pszShopImage && '\0' != pszShopImage[0] )
	{
		const std::string strUpper = UpperAscii( std::string( pszShopImage ) );

		if( NULL != FindLocation( strUpper ) )
		{
			const SDecodedImage* pImage = Decode( strUpper, kLookup );
			if( NULL != pImage )
			{
				kLookup.eOutcome	= IconOutcome_Decoded;
				kLookup.strResolved	= strUpper;
				return pImage;
			}

			// The file is there and this decoder could not read it. Kept
			// apart from "no such file" because the one shipped case is a
			// PNG under a .dds name, which ReadRaw can still hand to the
			// Ui - a different situation from a name the client itself
			// would have rejected.
			kLookup.eOutcome = IconOutcome_FallbackUndecodable;
		}
		else
		{
			// Exactly the client's IsValidFile == false branch
			// (X2Lib/X2SlotItem.cpp:257-264).
			kLookup.eOutcome = IconOutcome_FallbackNotFound;
		}
	}
	else
	{
		kLookup.eOutcome = IconOutcome_FallbackNoName;
	}

	if( false == m_bHaveFallback )
	{
		kLookup.eOutcome = IconOutcome_Failed;
		if( kLookup.strDetail.empty() )
			kLookup.strDetail = "HQ_Shop_Ui_Noimage.dds is not in any mounted archive";
		return NULL;
	}

	kLookup.strResolved	= FALLBACK_NAME;
	kLookup.eFormat		= m_kFallback.eFormat;
	return &m_kFallback;
}
