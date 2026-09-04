#include "stdafx.h"
#include "X2OfflineLog.h"

#ifdef SERV_IRUHADEV_OFFLINE

#include <stdarg.h>

FILE*						CX2OfflineLog::ms_pServerLog	= NULL;
FILE*						CX2OfflineLog::ms_pPacketLog	= NULL;
size_t						CX2OfflineLog::ms_nServerBytes	= 0;
size_t						CX2OfflineLog::ms_nPacketBytes	= 0;
KncCriticalSection			CX2OfflineLog::ms_cs;
bool						CX2OfflineLog::ms_bOpened		= false;
bool						CX2OfflineLog::ms_bDefer		= false;
std::vector< std::wstring >	CX2OfflineLog::ms_vecDeferred;

/// defined in X2OfflineEventName.cpp
extern const wchar_t* X2OfflineEventName( unsigned short usEventID );

void CX2OfflineLog::Open()
{
	KLocker lock( ms_cs );

	if( true == ms_bOpened )
		return;

	ms_bOpened = true;

	// truncate on every launch - these logs describe one run, not history
	ms_pServerLog = _wfopen( L"offline_server.log", L"wb" );
	ms_pPacketLog = _wfopen( L"offline_packets.log", L"wb" );

	WriteBOM( ms_pServerLog );
	WriteBOM( ms_pPacketLog );

	ms_nServerBytes = 0;
	ms_nPacketBytes = 0;
}

void CX2OfflineLog::WriteBOM( FILE* pFile )
{
	if( NULL == pFile )
		return;

	// UTF-8 BOM so an editor picks the encoding up; nicknames are wide strings.
	// Not counted towards the cap - three bytes either way.
	const unsigned char szBOM[3] = { 0xEF, 0xBB, 0xBF };
	fwrite( szBOM, 1, 3, pFile );
}

void CX2OfflineLog::Rotate( FILE*& pFile, const wchar_t* szName,
							size_t& nBytesWritten, size_t nCapBytes )
{
	// the caller holds ms_cs
	if( NULL == pFile || nBytesWritten < nCapBytes )
		return;

	wchar_t szOld[MAX_PATH];
	_snwprintf( szOld, MAX_PATH, L"%s.1", szName );
	szOld[MAX_PATH - 1] = L'\0';

	fclose( pFile );
	pFile = NULL;

	// MOVEFILE_REPLACE_EXISTING, and only one generation kept, on purpose: two
	// caps' worth is already more play than anyone reads back, and a growing
	// pile of numbered logs in the game directory is its own problem.
	::MoveFileExW( szName, szOld, MOVEFILE_REPLACE_EXISTING );

	pFile = _wfopen( szName, L"wb" );
	WriteBOM( pFile );

	nBytesWritten = 0;

	// First line of the new file says what happened, so a log that starts in
	// the middle of a session cannot be mistaken for a session that started
	// there.
	wchar_t szNote[MAX_PATH + 128];
	_snwprintf( szNote, MAX_PATH + 128,
		L"---- rotated at %u bytes; what came before is in %s ----",
		(unsigned int)nCapBytes, szOld );
	szNote[MAX_PATH + 127] = L'\0';

	nBytesWritten += WriteLine( pFile, szNote );
}

void CX2OfflineLog::Write( bool bPacketLog, const wchar_t* szLine )
{
	// the caller holds ms_cs
	if( true == bPacketLog )
	{
		Rotate( ms_pPacketLog, L"offline_packets.log", ms_nPacketBytes, PACKET_LOG_CAP_BYTES );
		ms_nPacketBytes += WriteLine( ms_pPacketLog, szLine );
	}
	else
	{
		Rotate( ms_pServerLog, L"offline_server.log", ms_nServerBytes, SERVER_LOG_CAP_BYTES );
		ms_nServerBytes += WriteLine( ms_pServerLog, szLine );
	}
}

void CX2OfflineLog::Close()
{
	KLocker lock( ms_cs );

	if( NULL != ms_pServerLog )		{ fclose( ms_pServerLog ); ms_pServerLog = NULL; }
	if( NULL != ms_pPacketLog )		{ fclose( ms_pPacketLog ); ms_pPacketLog = NULL; }

	ms_bDefer = false;
	ms_vecDeferred.clear();
	ms_bOpened = false;

	ms_nServerBytes = 0;
	ms_nPacketBytes = 0;
}

void CX2OfflineLog::TimeStamp( wchar_t* szOut, size_t nCount )
{
	SYSTEMTIME st;
	::GetLocalTime( &st );

	_snwprintf( szOut, nCount, L"[%02d:%02d:%02d.%03d]",
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds );
	szOut[ nCount - 1 ] = L'\0';
}

size_t CX2OfflineLog::WriteLine( FILE* pFile, const wchar_t* szLine )
{
	if( NULL == pFile || NULL == szLine )
		return 0;

	// wide -> UTF-8 so the files stay greppable with ordinary tools
	int nBytes = ::WideCharToMultiByte( CP_UTF8, 0, szLine, -1, NULL, 0, NULL, NULL );
	if( nBytes <= 1 )
		return 0;

	std::vector< char > vecUtf8( nBytes );
	::WideCharToMultiByte( CP_UTF8, 0, szLine, -1, &vecUtf8[0], nBytes, NULL, NULL );

	fwrite( &vecUtf8[0], 1, nBytes - 1, pFile );		// nBytes includes the terminator
	fwrite( "\r\n", 1, 2, pFile );
	fflush( pFile );									// never lose the last line

	return (size_t)( nBytes - 1 ) + 2;
}

void CX2OfflineLog::Server( const wchar_t* szFmt, ... )
{
	if( NULL == szFmt )
		return;

	wchar_t szBody[2048];
	va_list args;
	va_start( args, szFmt );
	_vsnwprintf( szBody, 2048, szFmt, args );
	va_end( args );
	szBody[2047] = L'\0';

	wchar_t szTime[32];
	TimeStamp( szTime, 32 );

	wchar_t szLine[2176];
	_snwprintf( szLine, 2176, L"%s %s", szTime, szBody );
	szLine[2175] = L'\0';

	KLocker lock( ms_cs );
	Write( false, szLine );
}

std::wstring CX2OfflineLog::Compose( bool bClientToServer,
									 const wchar_t* szProxy,
									 unsigned short usEventID,
									 size_t nPayloadBytes,
									 const wchar_t* szNote )
{
	wchar_t szTime[32];
	TimeStamp( szTime, 32 );

	wchar_t szLine[512];
	_snwprintf( szLine, 512, L"%s %s  %-3s %-50s (id=%u, %u bytes)  %s",
		szTime,
		bClientToServer ? L"C->S" : L"S->C",
		( NULL != szProxy ) ? szProxy : L"??",
		EventName( usEventID ),
		(unsigned int)usEventID,
		(unsigned int)nPayloadBytes,
		( NULL != szNote ) ? szNote : L"" );
	szLine[511] = L'\0';

	return std::wstring( szLine );
}

void CX2OfflineLog::Emit( const std::wstring& wstrLine )
{
	KLocker lock( ms_cs );

	if( true == ms_bDefer )
		ms_vecDeferred.push_back( wstrLine );
	else
		Write( true, wstrLine.c_str() );
}

void CX2OfflineLog::Packet( bool bClientToServer,
						    const wchar_t* szProxy,
						    unsigned short usEventID,
						    size_t nPayloadBytes,
						    const wchar_t* szNote )
{
	Emit( Compose( bClientToServer, szProxy, usEventID, nPayloadBytes, szNote ) );
}

void CX2OfflineLog::DeferBegin()
{
	KLocker lock( ms_cs );

	ms_bDefer = true;
	ms_vecDeferred.clear();
}

void CX2OfflineLog::DeferEnd( bool bClientToServer,
							  const wchar_t* szProxy,
							  unsigned short usEventID,
							  size_t nPayloadBytes,
							  const wchar_t* szNote )
{
	// compose outside the lock: Compose() takes none, but keep the order obvious
	std::wstring wstrHead = Compose( bClientToServer, szProxy, usEventID, nPayloadBytes, szNote );

	KLocker lock( ms_cs );

	ms_bDefer = false;

	Write( true, wstrHead.c_str() );

	for( size_t i = 0; i < ms_vecDeferred.size(); ++i )
		Write( true, ms_vecDeferred[i].c_str() );

	ms_vecDeferred.clear();
}

const wchar_t* CX2OfflineLog::EventName( unsigned short usEventID )
{
	return X2OfflineEventName( usEventID );
}

#endif SERV_IRUHADEV_OFFLINE
