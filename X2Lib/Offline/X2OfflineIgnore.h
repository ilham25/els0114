#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-04
// Description: Offline mode - the deliberately-not-implemented list, and the
//              per-run census of everything the dispatch declined (phase 8).
//
//              Phase 7's exit test came back with zero *** UNHANDLED *** lines
//              across a full session, which is the only reason this file can
//              exist: the bucket is empty, so anything that lands in it from
//              now on is new information and worth an interruption. What keeps
//              it that way is naming the packets that are *known* not to be
//              coming - the ones a player can still reach by clicking into a
//              feature that has no offline meaning - and labelling them as such
//              instead of leaving them to look like discoveries.
//
//              Two things this is careful not to be:
//
//              * It is NOT a second dispatch table. Reason() is consulted only
//                after Dispatch() has already declined the packet, and the
//                packet is dropped either way. A rule that matches something it
//                should not therefore costs a misleading label in the log - it
//                can never change what the server does.
//              * It is NOT a list of every unhandled ID. 221 of the client's
//                _REQ ids have no handler; ordinary play reaches none of them.
//                Only the families the phase notes actually named as refused
//                are listed, so the list stays something a person wrote rather
//                than something a script generated.
//
//              The census is the other half. On a clean shutdown every declined
//              packet is written to offline_server.log with its count, split
//              into "on the list" and "not on the list" - which is the sweep
//              the phase asks for, produced by playing rather than by reading.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

class CX2OfflineIgnore
{
public:
	/// Why this event is deliberately not implemented offline, or NULL if it
	/// is not on the list. Matched on the event's own name, the same data
	/// KindFromEventID classifies on - the ids themselves are only stable
	/// within one build of EventID_Client.h, the names are stable across all
	/// of them.
	static const wchar_t* Reason( unsigned short usEventID );

	/// Record one packet the dispatch declined. szReason is what Reason()
	/// returned, NULL included.
	static void Note( unsigned short usEventID, const wchar_t* szReason );

	/// Write the census to offline_server.log. Writes nothing at all if the
	/// dispatch declined nothing, which is the expected outcome.
	static void LogCensus();
};

#endif SERV_IRUHADEV_OFFLINE
