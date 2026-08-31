#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-08-31
// Description: Offline mode - the two log files the offline server writes
//              into the process working directory (the game data\ folder):
//
//                offline_server.log   lifecycle, errors, warnings
//                offline_packets.log  one line per packet, both directions
//
//              Both are flushed after every line: a crash must not lose the
//              last packet, because that line is usually the cause.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

class CX2OfflineLog
{
public:
	static void Open();
	static void Close();

	/// offline_server.log - printf-style, wide.
	static void Server( const wchar_t* szFmt, ... );

	/// offline_packets.log - one packet, one line.
	/// szNote is appended verbatim; use L"HANDLED", L"*** UNHANDLED ***", ...
	static void Packet( bool bClientToServer,
						const wchar_t* szProxy,
						unsigned short usEventID,
						size_t nPayloadBytes,
						const wchar_t* szNote );

	/// Replies are written from inside a handler, i.e. before the request line
	/// that caused them can be composed - its HANDLED / *** UNHANDLED *** tag is
	/// only known once the handler has returned. Buffer the replies so the file
	/// still reads in causal order:
	///     DeferBegin();  ...dispatch...;  DeferEnd( <the request line> );
	static void DeferBegin();
	static void DeferEnd( bool bClientToServer,
						  const wchar_t* szProxy,
						  unsigned short usEventID,
						  size_t nPayloadBytes,
						  const wchar_t* szNote );

	/// Name of an event ID, from the offline name table in X2OfflineEventName.cpp.
	/// KEvent::GetIDStr() is NOT usable here: under _SERVICE_ the studio's own
	/// table (CX2ServerEvent::SERVER_EVENT_ID_STR) collapses to a single empty
	/// string, and KEvent::GetIDStr() then reads one past the end of a
	/// system-only array for every client event ID.
	static const wchar_t* EventName( unsigned short usEventID );

private:
	static std::wstring Compose( bool bClientToServer,
								 const wchar_t* szProxy,
								 unsigned short usEventID,
								 size_t nPayloadBytes,
								 const wchar_t* szNote );
	static void Emit( const std::wstring& wstrLine );		///< respects the defer buffer
	static void WriteLine( FILE* pFile, const wchar_t* szLine );
	static void TimeStamp( wchar_t* szOut, size_t nCount );

	static FILE*						ms_pServerLog;
	static FILE*						ms_pPacketLog;
	static KncCriticalSection			ms_cs;
	static bool							ms_bOpened;
	static bool							ms_bDefer;
	static std::vector< std::wstring >	ms_vecDeferred;
};

#endif SERV_IRUHADEV_OFFLINE
