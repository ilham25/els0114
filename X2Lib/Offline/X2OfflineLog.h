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
//
//              Both are also capped and rotated (phase 8). offline_packets.log
//              grows at roughly a line per packet, which is a few MB per hour
//              of ordinary play and much faster inside a dungeon; left alone it
//              will eventually be too big to grep and then too big to open. At
//              the cap the file is renamed to <name>.1 - replacing whatever
//              was there - and a fresh one is started, so there is always
//              between one and two caps' worth of the most recent history.
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

	/// Byte cap per log file, after which it rotates to <name>.1. The packet
	/// log gets the larger one - it is the file that actually carries the
	/// evidence when something goes wrong.
	enum
	{
		SERVER_LOG_CAP_BYTES	=  8 * 1024 * 1024,
		PACKET_LOG_CAP_BYTES	= 48 * 1024 * 1024,
	};

private:
	/// Rotate pFile to <szName>.1 and reopen it, if it has passed nCapBytes.
	/// nBytesWritten is this file's running total and is reset on a rotation.
	static void Rotate( FILE*& pFile, const wchar_t* szName,
						size_t& nBytesWritten, size_t nCapBytes );

	static void WriteBOM( FILE* pFile );

	/// One line into one of the two files, rotating first if it is over its
	/// cap. The only writer; WriteLine does the encoding, this does the
	/// bookkeeping.
	static void Write( bool bPacketLog, const wchar_t* szLine );

	static std::wstring Compose( bool bClientToServer,
								 const wchar_t* szProxy,
								 unsigned short usEventID,
								 size_t nPayloadBytes,
								 const wchar_t* szNote );
	static void Emit( const std::wstring& wstrLine );		///< respects the defer buffer

	/// wide -> UTF-8 and out; returns how many bytes reached the file
	static size_t WriteLine( FILE* pFile, const wchar_t* szLine );
	static void TimeStamp( wchar_t* szOut, size_t nCount );

	static FILE*						ms_pServerLog;
	static FILE*						ms_pPacketLog;
	static size_t						ms_nServerBytes;
	static size_t						ms_nPacketBytes;
	static KncCriticalSection			ms_cs;
	static bool							ms_bOpened;
	static bool							ms_bDefer;
	static std::vector< std::wstring >	ms_vecDeferred;
};

#endif SERV_IRUHADEV_OFFLINE
