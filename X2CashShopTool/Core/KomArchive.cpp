//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 1 - V.0.3 .kom reader implementation.
// Format verified byte-for-byte against the real archives in the game
// directory; see CASH_SHOP_TOOL_PLAN.md section 3.
//////////////////////////////////////////////////////////////////////////
#include "KomArchive.h"

// This TU touches zlib, so it is one of the files that must never compile
// managed - see CASH_SHOP_TOOL_PLAN.md section 6 and Probe.cpp.
#ifdef _MANAGED
#error "This TU compiled managed. /clr leaked in - see CppBuild.targets:733-738"
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <algorithm>

#include "zlib.h"

namespace
{
	// The header is 50 bytes of version string, then UINT iTotalFileNo at
	// offset 52 (padded), then bool bCompressed at 56; sizeof is 60.
	// KGCMassFileManager.h:238-243, confirmed against data036.kom.
	const size_t	KOM_HEADER_SIZE			= 60;

	// V.0.3 then carries DWORD filetime, DWORD adler32, DWORD headersize,
	// and the plaintext XML manifest starts at 72.
	const size_t	KOM_V3_XML_OFFSET		= 72;

	const char* const	KOM_VERSION_PREFIX	= "KOG GC TEAM MASSFILE V.0.3.";

	// Sanity ceiling on the manifest. The largest real one is ~1.1 MB.
	const DWORD		KOM_MAX_HEADER_SIZE		= 64u * 1024u * 1024u;

	//////////////////////////////////////////////////////////////////////

	HANDLE OpenForRead( const std::wstring& wstrPath )
	{
		// FILE_SHARE_WRITE as well as READ: the client may have these
		// archives open, and the tool has no business failing because of it.
		return ::CreateFileW( wstrPath.c_str(), GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL );
	}

	bool ReadAt( HANDLE hFile, __int64 iOffset, void* pBuffer, DWORD dwBytes )
	{
		LARGE_INTEGER liPos;
		liPos.QuadPart = iOffset;
		if( FALSE == ::SetFilePointerEx( hFile, liPos, NULL, FILE_BEGIN ) )
			return false;

		BYTE*	pCursor		= (BYTE*) pBuffer;
		DWORD	dwRemaining	= dwBytes;
		while( dwRemaining > 0 )
		{
			DWORD dwRead = 0;
			if( FALSE == ::ReadFile( hFile, pCursor, dwRemaining, &dwRead, NULL ) )
				return false;
			if( 0 == dwRead )
				return false;	// premature end of file

			pCursor		+= dwRead;
			dwRemaining	-= dwRead;
		}

		return true;
	}

	// Inflates a whole zlib stream, GROWING the destination rather than
	// trusting the manifest's Size attribute.
	//
	// That is not defensive padding: 121 of the 88,723 members across the
	// 145 archives state CompressedSize == Size while still holding a real
	// zlib stream (they begin 78 9C like every other member), so their
	// stated Size is smaller than what they actually inflate to. A reader
	// that sized its output buffer from Size would get Z_BUF_ERROR on those
	// and drop them - silently, if it treated that as "not present".
	bool InflateBuffer( const char* pSrc, size_t uSrcLen, size_t uSizeHint,
						std::vector<char>& vecOut, std::string& strError )
	{
		vecOut.clear();

		z_stream zs;
		::memset( &zs, 0, sizeof( zs ) );

		if( Z_OK != inflateInit( &zs ) )
		{
			strError = "inflateInit failed";
			return false;
		}

		size_t uCapacity = ( uSizeHint > 0 ) ? uSizeHint : 4096;
		vecOut.resize( uCapacity );

		zs.next_in		= (Bytef*) pSrc;
		zs.avail_in		= (uInt) uSrcLen;
		zs.next_out		= (Bytef*) &vecOut[0];
		zs.avail_out	= (uInt) uCapacity;

		for( ;; )
		{
			const int iResult = inflate( &zs, Z_NO_FLUSH );

			if( Z_STREAM_END == iResult )
				break;

			if( Z_OK == iResult || Z_BUF_ERROR == iResult )
			{
				if( 0 == zs.avail_out )
				{
					const size_t uUsed = uCapacity;
					uCapacity *= 2;
					vecOut.resize( uCapacity );

					zs.next_out		= (Bytef*) &vecOut[uUsed];
					zs.avail_out	= (uInt)( uCapacity - uUsed );
					continue;
				}

				// Room left in the destination and still no progress means
				// the input ran out mid-stream.
				inflateEnd( &zs );
				strError = "truncated deflate stream";
				return false;
			}

			char szMsg[128];
			::sprintf_s( szMsg, sizeof( szMsg ), "inflate failed (%d)", iResult );
			inflateEnd( &zs );
			strError = szMsg;
			return false;
		}

		vecOut.resize( uCapacity - zs.avail_out );
		inflateEnd( &zs );
		return true;
	}

	//////////////////////////////////////////////////////////////////////
	// Minimal attribute scanner for the manifest.
	//
	// The manifest is flat, machine-generated and always of the shape
	// <File Name=".." Size=".." CompressedSize=".." CheckSum=".."
	// FileTime=".." Algorithm=".."/>, so this reads it without taking a
	// dependency on libxml (plan section 5). libxml stays linked and is the
	// fallback if a real parser is ever wanted.

	const char* FindSubstring( const char* pBegin, const char* pEnd, const char* pszNeedle )
	{
		const size_t uNeedle = ::strlen( pszNeedle );
		if( 0 == uNeedle || (size_t)( pEnd - pBegin ) < uNeedle )
			return NULL;

		const char* pLast = pEnd - uNeedle;
		for( const char* p = pBegin; p <= pLast; ++p )
		{
			if( 0 == ::memcmp( p, pszNeedle, uNeedle ) )
				return p;
		}

		return NULL;
	}

	bool ReadAttribute( const char* pElemBegin, const char* pElemEnd,
						const char* pszAttribute, std::string& strValue )
	{
		std::string strKey = " ";
		strKey += pszAttribute;
		strKey += "=\"";

		const char* pKey = FindSubstring( pElemBegin, pElemEnd, strKey.c_str() );
		if( NULL == pKey )
			return false;

		const char* pValue = pKey + strKey.size();
		const char* pQuote = (const char*) ::memchr( pValue, '"', pElemEnd - pValue );
		if( NULL == pQuote )
			return false;

		strValue.assign( pValue, pQuote - pValue );
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////

void IToolLog::Linef( const char* pszFormat, ... )
{
	char szBuffer[1024];

	va_list args;
	va_start( args, pszFormat );
	::vsprintf_s( szBuffer, sizeof( szBuffer ), pszFormat, args );
	va_end( args );

	Line( szBuffer );
}

//////////////////////////////////////////////////////////////////////////

bool GetFileStamp( const std::wstring& wstrPath, __int64* piSize, __int64* piMTime )
{
	if( NULL != piSize )		*piSize = 0;
	if( NULL != piMTime )		*piMTime = 0;

	WIN32_FILE_ATTRIBUTE_DATA kData;
	if( FALSE == ::GetFileAttributesExW( wstrPath.c_str(), GetFileExInfoStandard, &kData ) )
		return false;

	if( NULL != piSize )
	{
		LARGE_INTEGER liSize;
		liSize.HighPart	= (LONG) kData.nFileSizeHigh;
		liSize.LowPart	= kData.nFileSizeLow;
		*piSize = liSize.QuadPart;
	}

	if( NULL != piMTime )
	{
		ULARGE_INTEGER uliTime;
		uliTime.HighPart	= kData.ftLastWriteTime.dwHighDateTime;
		uliTime.LowPart		= kData.ftLastWriteTime.dwLowDateTime;
		*piMTime = (__int64) uliTime.QuadPart;
	}

	return true;
}

// UTF-8 both ways, deliberately: item names come out of the scripts as
// UTF-8 already (KLuaManager converts with CP_UTF8, luaLib/KLuaManager.h:803),
// the index cache stores UTF-8, and the Ui decodes every log line as UTF-8.
// One encoding end to end means no conversion anywhere in between.
std::string NarrowPath( const std::wstring& wstr )
{
	if( wstr.empty() )
		return std::string();

	const int iBytes = ::WideCharToMultiByte( CP_UTF8, 0, wstr.c_str(), (int) wstr.size(), NULL, 0, NULL, NULL );
	if( iBytes <= 0 )
		return std::string();

	std::string strOut( (size_t) iBytes, '\0' );
	::WideCharToMultiByte( CP_UTF8, 0, wstr.c_str(), (int) wstr.size(), &strOut[0], iBytes, NULL, NULL );
	return strOut;
}

std::wstring WidenPath( const std::string& str )
{
	if( str.empty() )
		return std::wstring();

	const int iChars = ::MultiByteToWideChar( CP_UTF8, 0, str.c_str(), (int) str.size(), NULL, 0 );
	if( iChars <= 0 )
		return std::wstring();

	std::wstring wstrOut( (size_t) iChars, L'\0' );
	::MultiByteToWideChar( CP_UTF8, 0, str.c_str(), (int) str.size(), &wstrOut[0], iChars );
	return wstrOut;
}

std::string UpperAscii( const std::string& str )
{
	std::string strOut( str );
	for( size_t u = 0; u != strOut.size(); ++u )
	{
		const unsigned char c = (unsigned char) strOut[u];
		if( c >= 'a' && c <= 'z' )
			strOut[u] = (char)( c - 'a' + 'A' );
	}

	return strOut;
}

//////////////////////////////////////////////////////////////////////////
// CKomArchive

CKomArchive::CKomArchive()
: m_iFileSize( 0 )
, m_iMTime( 0 )
{
}

bool CKomArchive::Open( const std::wstring& wstrPath, std::string& strError )
{
	m_wstrPath = wstrPath;
	m_vecMembers.clear();
	GetFileStamp( wstrPath, &m_iFileSize, &m_iMTime );

	HANDLE hFile = OpenForRead( wstrPath );
	if( INVALID_HANDLE_VALUE == hFile )
	{
		strError = "cannot open file";
		return false;
	}

	// Header plus the three V.0.3 DWORDs in one read.
	char acHeader[KOM_V3_XML_OFFSET];
	if( false == ReadAt( hFile, 0, acHeader, (DWORD) KOM_V3_XML_OFFSET ) )
	{
		::CloseHandle( hFile );
		strError = "cannot read archive header";
		return false;
	}

	// The engine derives the version from two digits inside the string
	// (KGCMassFileManager.cpp:1322). Every shipped archive is V.0.3, and a
	// reader that guessed wrong here would misparse rather than fail, so
	// this refuses anything else outright.
	if( 0 != ::memcmp( acHeader, KOM_VERSION_PREFIX, ::strlen( KOM_VERSION_PREFIX ) ) )
	{
		::CloseHandle( hFile );
		std::string strVersion( acHeader, 50 );
		strError = "unsupported archive version '" + strVersion + "' (only V.0.3 is handled)";
		return false;
	}

	DWORD dwHeaderSize = 0;
	::memcpy( &dwHeaderSize, acHeader + 68, sizeof( DWORD ) );

	if( 0 == dwHeaderSize || dwHeaderSize > KOM_MAX_HEADER_SIZE )
	{
		::CloseHandle( hFile );
		strError = "implausible manifest size";
		return false;
	}

	std::vector<char> vecXml( dwHeaderSize );
	if( false == ReadAt( hFile, (__int64) KOM_V3_XML_OFFSET, &vecXml[0], dwHeaderSize ) )
	{
		::CloseHandle( hFile );
		strError = "cannot read manifest";
		return false;
	}

	::CloseHandle( hFile );

	// Payloads follow the manifest immediately, and each member's offset is
	// the running sum of the CompressedSize values before it. Confirmed by
	// inflating and XOR-decrypting the first member of data036.kom into a
	// 1B 4C 75 61 51 Lua 5.1 bytecode header.
	return ParseManifest( &vecXml[0], dwHeaderSize,
		(__int64)( KOM_V3_XML_OFFSET + dwHeaderSize ), strError );
}

bool CKomArchive::ParseManifest( const char* pXml, size_t uXmlLen, __int64 iFirstOffset, std::string& strError )
{
	const char* const	pEnd	= pXml + uXmlLen;
	const char*			pCursor	= pXml;
	__int64				iOffset	= iFirstOffset;

	std::string strName;
	std::string strSize;
	std::string strCompressed;

	for( ;; )
	{
		const char* pElem = FindSubstring( pCursor, pEnd, "<File " );
		if( NULL == pElem )
			break;

		const char* pElemEnd = (const char*) ::memchr( pElem, '>', pEnd - pElem );
		if( NULL == pElemEnd )
			break;

		if( ReadAttribute( pElem, pElemEnd, "Name", strName )
			&& ReadAttribute( pElem, pElemEnd, "Size", strSize )
			&& ReadAttribute( pElem, pElemEnd, "CompressedSize", strCompressed ) )
		{
			SKomMember kMember;
			kMember.strName		= UpperAscii( strName );
			kMember.lStatedSize	= ::atol( strSize.c_str() );
			kMember.lCompSize	= ::atol( strCompressed.c_str() );
			kMember.iOffset		= iOffset;

			iOffset += kMember.lCompSize;
			m_vecMembers.push_back( kMember );
		}

		pCursor = pElemEnd + 1;
	}

	if( m_vecMembers.empty() )
	{
		strError = "manifest holds no <File> entries";
		return false;
	}

	return true;
}

bool CKomArchive::ReadMember( size_t uIndex, std::vector<char>& vecOut, std::string& strError ) const
{
	vecOut.clear();

	if( uIndex >= m_vecMembers.size() )
	{
		strError = "member index out of range";
		return false;
	}

	const SKomMember& kMember = m_vecMembers[uIndex];
	if( kMember.lCompSize <= 0 )
	{
		strError = "member is empty";
		return false;
	}

	HANDLE hFile = OpenForRead( m_wstrPath );
	if( INVALID_HANDLE_VALUE == hFile )
	{
		strError = "cannot open archive";
		return false;
	}

	std::vector<char> vecCompressed( (size_t) kMember.lCompSize );
	const bool bRead = ReadAt( hFile, kMember.iOffset, &vecCompressed[0], (DWORD) kMember.lCompSize );
	::CloseHandle( hFile );

	if( false == bRead )
	{
		strError = "cannot read member payload";
		return false;
	}

	return InflateBuffer( &vecCompressed[0], vecCompressed.size(),
		(size_t)( kMember.lStatedSize > 0 ? kMember.lStatedSize : 0 ), vecOut, strError );
}

//////////////////////////////////////////////////////////////////////////
// CKomIndex

CKomIndex::CKomIndex()
: m_uShadowed( 0 )
{
}

bool CKomIndex::Mount( const std::wstring& wstrDir, IToolLog* pLog )
{
	m_vecArchives.clear();
	m_mapName.clear();
	m_uShadowed = 0;

	// The client builds exactly this list, data001.kom .. data145.kom, and
	// mounts it in this order (X2Lib/X2Main.cpp:770-787). Generating the
	// same list rather than globbing the directory keeps the tool's view of
	// "which archive owns this name" identical to the client's, including
	// the first-wins rule below.
	const int KOM_FILE_COUNT = 145;

	// No reallocation once reserved, so pointers handed out by ArchiveFor
	// stay valid for the life of the index.
	m_vecArchives.reserve( (size_t) KOM_FILE_COUNT );

	int iMissing = 0;
	int iFailed  = 0;

	for( int i = 1; i <= KOM_FILE_COUNT; ++i )
	{
		wchar_t wszName[64];
		::swprintf_s( wszName, 64, L"data%03d.kom", i );

		std::wstring wstrPath = wstrDir;
		if( false == wstrPath.empty() && L'\\' != wstrPath[wstrPath.size() - 1] && L'/' != wstrPath[wstrPath.size() - 1] )
			wstrPath += L"\\";
		wstrPath += wszName;

		__int64 iSize = 0;
		if( false == GetFileStamp( wstrPath, &iSize, NULL ) )
		{
			++iMissing;
			if( NULL != pLog )
				pLog->Linef( "  archive missing: %ls", wszName );
			continue;
		}

		m_vecArchives.push_back( CKomArchive() );

		std::string strError;
		if( false == m_vecArchives.back().Open( wstrPath, strError ) )
		{
			++iFailed;
			if( NULL != pLog )
				pLog->Linef( "  archive FAILED: %ls - %s", wszName, strError.c_str() );
			m_vecArchives.pop_back();
			continue;
		}

		const size_t				uArchive	= m_vecArchives.size() - 1;
		const std::vector<SKomMember>&	vecMembers	= m_vecArchives.back().Members();

		for( size_t u = 0; u != vecMembers.size(); ++u )
		{
			SLocation kLocation;
			kLocation.uArchive	= uArchive;
			kLocation.uMember	= u;

			// First archive to claim a name wins, matching the engine's
			// map insert. A later archive carrying the same name is
			// shadowed and counted, never silently swapped in.
			if( false == m_mapName.insert( std::make_pair( vecMembers[u].strName, kLocation ) ).second )
				++m_uShadowed;
		}
	}

	if( NULL != pLog )
	{
		pLog->Linef( "archives : mounted %u of %d (%u names, %u shadowed by an earlier archive)",
			(unsigned) m_vecArchives.size(), KOM_FILE_COUNT,
			(unsigned) m_mapName.size(), (unsigned) m_uShadowed );

		if( iMissing > 0 || iFailed > 0 )
			pLog->Linef( "           %d missing, %d failed to parse", iMissing, iFailed );
	}

	return false == m_vecArchives.empty();
}

const CKomIndex::SLocation* CKomIndex::Find( const char* pszName ) const
{
	const std::string strKey = UpperAscii( std::string( pszName ) );

	std::map<std::string, SLocation>::const_iterator iter = m_mapName.find( strKey );
	if( iter == m_mapName.end() )
		return NULL;

	return &iter->second;
}

bool CKomIndex::Contains( const char* pszName ) const
{
	return NULL != Find( pszName );
}

const CKomArchive* CKomIndex::ArchiveFor( const char* pszName ) const
{
	const SLocation* pLocation = Find( pszName );
	if( NULL == pLocation )
		return NULL;

	return &m_vecArchives[pLocation->uArchive];
}

bool CKomIndex::Read( const char* pszName, std::vector<char>& vecOut, std::string& strError ) const
{
	const SLocation* pLocation = Find( pszName );
	if( NULL == pLocation )
	{
		strError = std::string( "'" ) + pszName + "' is not in any mounted archive";
		return false;
	}

	return m_vecArchives[pLocation->uArchive].ReadMember( pLocation->uMember, vecOut, strError );
}
