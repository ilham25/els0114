#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-08-31
// Description: Offline mode - event ID -> name table for offline_packets.log.
//
//              This has to exist because neither of the two tables already in
//              the tree is usable in a US_SERVICE build:
//
//                * CX2ServerEvent::SERVER_EVENT_ID_STR (X2ServerEvent.cpp) is
//                  compiled down to { L"" } under _SERVICE_, and
//                  CX2ServerProtocol's ctor only hands it to
//                  KEvent::SetEventID() when bIsSERVICE == false;
//                * KEvent::ms_szEventID (X2ServerProtocol/Event.cpp) therefore
//                  stays at the 24-entry EventID_System.h list with
//                  m_EventIDEnd == E_SYSTEM_EVENT_ID_END, so
//                  KEvent::GetIDStr() reads one element past the end of that
//                  array for every EGS_*/ECH_* id.
//
//              So build the full table here, from the same X-macro headers the
//              wire protocol itself is generated from, and only in the offline
//              build.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

#undef  _ENUM
#define _ENUM( id ) L#id,

static const wchar_t* s_szRawEventName[] =
{
#	include "EventID_System.h"
#	include "EventID_Client.h"
};

#undef _ENUM

static const int s_nRawEventNameCount = sizeof( s_szRawEventName ) / sizeof( s_szRawEventName[0] );

// The X-macro stringizes the whole enumerator, so entries written as
// "_ENUM( E_HEART_BEAT = 0 )" come out as L"E_HEART_BEAT = 0". Trim the
// explicit-value tail once, on first use.
static std::vector< std::wstring >	s_vecEventName;
static KncCriticalSection			s_csEventName;
static bool							s_bEventNameBuilt = false;

const wchar_t* X2OfflineEventName( unsigned short usEventID )
{
	{
		KLocker lock( s_csEventName );

		if( false == s_bEventNameBuilt )
		{
			s_vecEventName.reserve( s_nRawEventNameCount );

			for( int i = 0; i < s_nRawEventNameCount; ++i )
			{
				std::wstring wstrName = s_szRawEventName[i];

				std::wstring::size_type nCut = wstrName.find_first_of( L" \t=" );
				if( std::wstring::npos != nCut )
					wstrName.erase( nCut );

				s_vecEventName.push_back( wstrName );
			}

			s_bEventNameBuilt = true;
		}
	}

	if( (int)usEventID < (int)s_vecEventName.size() )
		return s_vecEventName[ usEventID ].c_str();

	return L"<UNKNOWN_EVENT_ID>";
}

#endif SERV_IRUHADEV_OFFLINE
