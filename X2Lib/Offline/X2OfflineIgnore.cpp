#include "stdafx.h"
#include "X2OfflineIgnore.h"

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineLog.h"

/// defined in X2OfflineEventName.cpp
extern const wchar_t* X2OfflineEventName( unsigned short usEventID );

//////////////////////////////////////////////////////////////////////////

namespace
{
	/// One rule. szMatch is a substring of the event name, not a prefix: the
	/// families are spelled inconsistently in EventID_Client.h
	/// (EGS_CREATE_PERSONAL_SHOP_REQ but EGS_JOIN_PERSONAL_SHOP_REQ), so
	/// anchoring on the front would need two or three rules per family and
	/// would still miss one.
	struct KIgnoreRule
	{
		const wchar_t*	szMatch;
		const wchar_t*	szReason;
	};

	//////////////////////////////////////////////////////////////////////////
	// Every entry below is a family a phase note already called refused; the
	// phase and the sentence it was refused with are in the comment. Nothing
	// here was added because it looked unreachable.

	const KIgnoreRule s_kIgnoreRule[] =
	{
		// phase 7: "every EGS_ADMIN_* cheat (auth-gated, and the auth level is
		// never raised offline)". MakeDefaultUnitInfo sets m_cAuthLevel to
		// XUAL_NORMAL_USER and nothing ever raises it, so the real server would
		// have refused these too.
		{ L"EGS_ADMIN_",			L"admin command; the offline auth level is never raised above XUAL_NORMAL_USER" },
		{ L"ELG_ADMIN_",			L"admin command; the offline auth level is never raised above XUAL_NORMAL_USER" },

		// phase 7: "the personal shop and personal trade". PSHOP_AGENCY is the
		// later unattended-stall version of the same feature.
		{ L"PERSONAL_SHOP",			L"the player market needs other players" },
		{ L"PSHOP_AGENCY",			L"the player market needs other players" },
		{ L"_TRADE_",				L"person-to-person trade needs a second player" },

		// phase 7: "PvP rooms and lobbies". Note that the two PvP packets the
		// dispatch does answer - EGS_INVITE_PVP_ROOM_REQ and
		// EGS_PVP_PARTY_CHANGE_MATCH_INFO_REQ - never reach this table, because
		// Reason() only runs on a packet Dispatch already declined.
		{ L"PVP",					L"no PvP offline; a match is peer-to-peer and there is no peer" },

		// phase 7: "the training school". TC is how the ids spell it
		// (EGS_CREATE_TC_ROOM_REQ, EGS_END_TC_GAME_REQ, ...).
		{ L"_TC_",					L"the training school is a multiplayer room" },

		// phase 5: "the item crafting family phase 5 already refuses
		// (manufacture, synthesis, resolve, identify, seal, evaluate, convert)".
		// Listed by id rather than by a substring because the names have no
		// shared token - ITEM_MANUFACTURE, RESOLVE_ITEM, IDENTIFY_ITEM.
		{ L"EGS_ITEM_MANUFACTURE_REQ",		L"the item workshop is not implemented offline" },
		{ L"EGS_RESOLVE_ITEM_REQ",			L"the item workshop is not implemented offline" },
		{ L"EGS_IDENTIFY_ITEM_REQ",			L"the item workshop is not implemented offline" },
		{ L"EGS_SEAL_ITEM_REQ",				L"the item workshop is not implemented offline" },
		{ L"EGS_UNSEAL_ITEM_REQ",			L"the item workshop is not implemented offline" },
		{ L"EGS_ITEM_EVALUATE_REQ",			L"the item workshop is not implemented offline" },
		{ L"EGS_RESTORE_ITEM_EVALUATE_REQ",	L"the item workshop is not implemented offline" },
		{ L"EGS_ITEM_CONVERT_REQ",			L"the item workshop is not implemented offline" },
		{ L"EGS_ITEM_EXCHANGE_REQ",			L"the item workshop is not implemented offline" },
		{ L"EGS_RESTORE_ITEM_REQ",			L"the item workshop is not implemented offline" },
		// EGS_ATTRIB_ENCHANT_ITEM_REQ, EGS_ATTRIB_ATTACH_ITEM_REQ and
		// EGS_ENCHANT_ATTACH_ITEM_REQ stood here until phases 25 and 26; all
		// three are handled now. An ignore rule is a promise that nothing is
		// waiting on a reply, and all three arm an AddServerPacket wait, so
		// leaving them here hung the dialog rather than refusing it.

		// EGS_ADD_ON_STAT_REQ is NOT part of the item workshop and never was -
		// the name is a coincidence. It is an in-match relay: the GameServer
		// forwards it to the room server (GSUserRoomCommon.cpp:4122) and the
		// answer comes back as EGS_ADD_ON_STAT_NOT, which the client applies
		// with CX2GUUser::SetAddOnStat (X2Game.cpp:8660) - a temporary stat
		// buff on a unit inside a dungeon. There is no room server offline and
		// the host applies its own buffs peer-to-peer, so it stays ignored;
		// only the reason was wrong.
		{ L"EGS_ADD_ON_STAT_REQ",			L"an in-match stat relay to the room server, not an item packet; the P2P host applies its own" },

		// phase 7: "the pre-global billing packets that SERV_GLOBAL_BILLING
		// replaces". The handled replacements carry a BILL_ in the middle
		// (EGS_BILL_GET_PURCHASED_CASH_ITEM_REQ), so these anchors cannot
		// swallow them.
		{ L"EGS_CASH_PRODUCT_INFO",			L"pre-global billing; SERV_GLOBAL_BILLING replaced it with the EGS_BILL_* family" },
		{ L"EGS_PURCHASED_CASH_ITEM_LIST",	L"pre-global billing; SERV_GLOBAL_BILLING replaced it with the EGS_BILL_* family" },
		{ L"EGS_GET_PURCHASED_CASH_ITEM",	L"pre-global billing; SERV_GLOBAL_BILLING replaced it with the EGS_BILL_* family" },

		// phase 6, on quests and titles: both of these are *pushes* the offline
		// server would have to originate, and both need a live event schedule
		// it does not have; nothing offline issues a rented title either.
		{ L"EGS_CHANGE_RANDOM_QUEST_NOT",	L"a server-originated push that needs a live event schedule" },
		{ L"EGS_EVENT_QUEST_INFO_NOT",		L"a server-originated push that needs a live event schedule" },
		{ L"EGS_TITLE_EXPIRATION_NOT",		L"nothing offline issues a rented title" },
	};

	const int s_nIgnoreRuleCount = sizeof( s_kIgnoreRule ) / sizeof( s_kIgnoreRule[0] );

	//////////////////////////////////////////////////////////////////////////
	// the census

	struct KDeclined
	{
		int				iCount;
		const wchar_t*	szReason;		///< NULL if it was not on the list

		KDeclined() : iCount( 0 ), szReason( NULL ) {}
	};

	std::map< unsigned short, KDeclined >	s_mapDeclined;
	KncCriticalSection						s_csDeclined;
}

//////////////////////////////////////////////////////////////////////////

/*static*/ const wchar_t* CX2OfflineIgnore::Reason( unsigned short usEventID )
{
	const wchar_t* szName = X2OfflineEventName( usEventID );
	if( NULL == szName )
		return NULL;

	for( int i = 0; i < s_nIgnoreRuleCount; ++i )
	{
		if( NULL != wcsstr( szName, s_kIgnoreRule[i].szMatch ) )
			return s_kIgnoreRule[i].szReason;
	}

	return NULL;
}

/*static*/ void CX2OfflineIgnore::Note( unsigned short usEventID, const wchar_t* szReason )
{
	KLocker lock( s_csDeclined );

	KDeclined& kRow = s_mapDeclined[ usEventID ];

	++kRow.iCount;

	// First reason wins; they cannot differ for one id anyway, and this keeps
	// the row from flapping if a rule is ever added mid-run.
	if( NULL == kRow.szReason )
		kRow.szReason = szReason;
}

/*static*/ void CX2OfflineIgnore::LogCensus()
{
	KLocker lock( s_csDeclined );

	if( true == s_mapDeclined.empty() )
		return;

	int iOnList		= 0;
	int iOffList	= 0;

	std::map< unsigned short, KDeclined >::const_iterator mit;

	for( mit = s_mapDeclined.begin(); mit != s_mapDeclined.end(); ++mit )
	{
		if( NULL != mit->second.szReason )	++iOnList;
		else								++iOffList;
	}

	CX2OfflineLog::Server( L"CENSUS   the dispatch declined %d event id(s): %d on the ignore list, %d NOT",
		(int)s_mapDeclined.size(), iOnList, iOffList );

	// The not-on-the-list half first: that is the half worth reading.
	for( mit = s_mapDeclined.begin(); mit != s_mapDeclined.end(); ++mit )
	{
		if( NULL != mit->second.szReason )
			continue;

		CX2OfflineLog::Server( L"CENSUS   ** UNHANDLED  %-50s x%-5d  needs a handler, or an entry in X2OfflineIgnore.cpp",
			CX2OfflineLog::EventName( mit->first ), mit->second.iCount );
	}

	for( mit = s_mapDeclined.begin(); mit != s_mapDeclined.end(); ++mit )
	{
		if( NULL == mit->second.szReason )
			continue;

		CX2OfflineLog::Server( L"CENSUS      ignored   %-50s x%-5d  %s",
			CX2OfflineLog::EventName( mit->first ), mit->second.iCount,
			mit->second.szReason );
	}
}

#endif SERV_IRUHADEV_OFFLINE
