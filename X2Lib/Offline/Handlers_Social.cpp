#include "stdafx.h"
#include "X2OfflineServer.h"

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-04
// Description: Offline mode - phase 7. Guild, party, friends, mail, chat,
//              ranking, and the bits of UI that hang off them.
//
//              Every one of these systems needs other players, and offline
//              there are none. The rule this whole file follows, and the reason
//              it exists at all:
//
//              **A list fetch answers with a well-formed EMPTY list, not with
//              an error and not with silence.** An empty guild list is what a
//              player with no guild gets on live; an error box is not, and
//              silence is a spinner that never stops. So opening the guild
//              window, the friends list, the mailbox or a ranking board offline
//              looks exactly like opening it on live with nothing in it.
//
//              **An action that needs somebody else answers with a refusal that
//              is true.** "That nickname does not exist" is the honest reason a
//              friend request fails when the only character in the world is the
//              one sending it - so that is the code sent, rather than a generic
//              failure or an invented success.
//
//              The one thing here that is not a stub is chat: the client does
//              not echo a sent line locally, it waits for the server to
//              broadcast EGS_CHAT_NOT back. So the offline server broadcasts it
//              back to the one player, and the chat box works.
//
//              Pets and riding pets are real too, out of SQLite - see the pet
//              section at the bottom.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineInventory.h"
#include "X2OfflinePetData.h"
#include "X2OfflineRandomItem.h"
#include "X2OfflineMapData.h"
#include "X2OfflineSkill.h"

//////////////////////////////////////////////////////////////////////////
// guild
//
// There is no guild table and no guild code offline. Creation is refused, the
// boards come back empty, and every member-management packet answers with "you
// are not in a guild" - which is the truth.

bool CX2OfflineServer::Handler_EGS_CREATE_GUILD_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CREATE_GUILD_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// The guild-creation item is NOT consumed. The request names it by UID and
	// the real server destroys it as part of a successful creation; refusing
	// here has to leave it where it is, or the player loses a cash item to a
	// feature that does not exist.
	CX2OfflineLog::Server( L"GUILD    creation of '%s' refused - there are no guilds offline"
		L" (the creation item is left in the bag)", kReq.m_wstrGuildName.c_str() );

	KEGS_CREATE_GUILD_ACK kAck;
	kAck.m_iOK = NetError::ERR_GUILD_04;			///< guild creation failed

	return Reply( kSes, EGS_CREATE_GUILD_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_GET_GUILD_USER_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GET_GUILD_USER_LIST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_GET_GUILD_USER_LIST_ACK kAck;
	kAck.m_iOK					= NetError::NET_OK;

	// One page, not zero. A zero total page count makes the paging UI divide by
	// it; one empty page is the shape a real empty list arrives in.
	kAck.m_uiTotalPage			= 1;
	kAck.m_uiViewPage			= 1;
	kAck.m_cGuildUserListType	= kReq.m_cGuildUserListType;
	kAck.m_iLoginedMemberCount	= 0;

	return Reply( kSes, EGS_GET_GUILD_USER_LIST_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_GET_APPLY_JOIN_GUILD_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GET_APPLY_JOIN_GUILD_LIST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_GET_APPLY_JOIN_GUILD_LIST_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_wstrGuildName= L"";
	kAck.m_uiTotalPage	= 1;
	kAck.m_uiViewPage	= 1;

	return Reply( kSes, EGS_GET_APPLY_JOIN_GUILD_LIST_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_GET_GUILD_AD_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GET_GUILD_AD_LIST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// This ACK has no m_iOK at all - the board is drawn from the list alone, so
	// an empty list IS the answer and there is no way to fail it.
	KEGS_GET_GUILD_AD_LIST_ACK kAck;
	kAck.m_cSortType			= kReq.m_cSortType;
	kAck.m_uiTotalPage			= 1;
	kAck.m_uiViewPage			= 1;
	kAck.m_bCanShowApplyList	= false;
	kAck.m_bRegMyGuildAd		= false;

	return Reply( kSes, EGS_GET_GUILD_AD_LIST_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_APPLY_JOIN_GUILD_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_APPLY_JOIN_GUILD_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_APPLY_JOIN_GUILD_ACK kAck;
	kAck.m_iOK = NetError::ERR_GUILD_06;			///< guild info not found

	return Reply( kSes, EGS_APPLY_JOIN_GUILD_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_ACCEPT_JOIN_GUILD_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_ACCEPT_JOIN_GUILD_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_ACCEPT_JOIN_GUILD_ACK kAck;
	kAck.m_iOK					= NetError::ERR_GUILD_19;	///< not in a guild
	kAck.m_wstrReceiverNickName	= L"";

	return Reply( kSes, EGS_ACCEPT_JOIN_GUILD_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_DELETE_APPLY_JOIN_GUILD_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_DELETE_APPLY_JOIN_GUILD_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Deleting an application that does not exist is not a failure - the list
	// is already in the state the player asked for. NET_OK, and the list they
	// re-fetch is empty.
	KEGS_DELETE_APPLY_JOIN_GUILD_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_DELETE_APPLY_JOIN_GUILD_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_REGISTRATION_GUILD_AD_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_REGISTRATION_GUILD_AD_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_REGISTRATION_GUILD_AD_ACK kAck;
	kAck.m_iOK			= NetError::ERR_GUILD_19;
	kAck.m_iED			= 0;
	kAck.m_sPeriod		= kReq.m_sPeriod;
	kAck.m_wstrAdMessage= kReq.m_wstrAdMessage;

	return Reply( kSes, EGS_REGISTRATION_GUILD_AD_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_MODIFY_REG_GUILD_AD_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_MODIFY_REG_GUILD_AD_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_MODIFY_REG_GUILD_AD_ACK kAck;
	kAck.m_iOK			= NetError::ERR_GUILD_19;
	kAck.m_iED			= 0;
	kAck.m_sPeriod		= kReq.m_sPeriod;
	kAck.m_wstrAdMessage= kReq.m_wstrAdMessage;

	return Reply( kSes, EGS_MODIFY_REG_GUILD_AD_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_INVITE_GUILD_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_INVITE_GUILD_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_INVITE_GUILD_ACK kAck;
	kAck.m_iOK					= NetError::ERR_GUILD_19;
	kAck.m_wstrReceiverNickName	= kReq.m_wstrNickName;

	return Reply( kSes, EGS_INVITE_GUILD_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_INVITE_GUILD_REPLY_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_INVITE_GUILD_REPLY_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_INVITE_GUILD_REPLY_ACK kAck;
	kAck.m_iOK = NetError::ERR_GUILD_06;

	return Reply( kSes, EGS_INVITE_GUILD_REPLY_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_KICK_GUILD_MEMBER_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_KICK_GUILD_MEMBER_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_KICK_GUILD_MEMBER_ACK kAck;
	kAck.m_iOK			= NetError::ERR_GUILD_19;
	kAck.m_bKickBySysop	= false;

	return Reply( kSes, EGS_KICK_GUILD_MEMBER_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_CHANGE_GUILD_MEMBER_GRADE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CHANGE_GUILD_MEMBER_GRADE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_CHANGE_GUILD_MEMBER_GRADE_ACK kAck;
	kAck.m_iOK					= NetError::ERR_GUILD_19;
	kAck.m_wstrTargetNickName	= L"";
	kAck.m_ucMemberShipGrade	= kReq.m_ucMemberShipGrade;

	return Reply( kSes, EGS_CHANGE_GUILD_MEMBER_GRADE_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_CHANGE_GUILD_MEMBER_MESSAGE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CHANGE_GUILD_MEMBER_MESSAGE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_CHANGE_GUILD_MEMBER_MESSAGE_ACK kAck;
	kAck.m_iOK		= NetError::ERR_GUILD_19;
	kAck.m_bDelete	= kReq.m_wstrMessage.empty();

	return Reply( kSes, EGS_CHANGE_GUILD_MEMBER_MESSAGE_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_CHANGE_GUILD_MESSAGE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CHANGE_GUILD_MESSAGE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_CHANGE_GUILD_MESSAGE_ACK kAck;
	kAck.m_iOK = NetError::ERR_GUILD_19;

	return Reply( kSes, EGS_CHANGE_GUILD_MESSAGE_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_CHANGE_GUILD_NAME_CHECK_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CHANGE_GUILD_NAME_CHECK_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_CHANGE_GUILD_NAME_CHECK_ACK kAck;
	kAck.m_iOK = NetError::ERR_GUILD_19;

	return Reply( kSes, EGS_CHANGE_GUILD_NAME_CHECK_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_CHANGE_GUILD_NAME_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CHANGE_GUILD_NAME_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_CHANGE_GUILD_NAME_ACK kAck;
	kAck.m_iOK = NetError::ERR_GUILD_19;

	return Reply( kSes, EGS_CHANGE_GUILD_NAME_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_GET_GUILD_SKILL_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GET_GUILD_SKILL_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_GET_GUILD_SKILL_ACK kAck;
	kAck.m_iOK = NetError::ERR_GUILD_19;

	return Reply( kSes, EGS_GET_GUILD_SKILL_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_GET_GUILD_SKILL_IN_BOARD_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GET_GUILD_SKILL_IN_BOARD_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// NET_OK with a default KGuildSkillInfo: the board is showing "this guild
	// has no skills", which for a guild that does not exist is the least
	// confusing thing on screen. A failure here pops a dialog on a window the
	// player only opened to look at.
	KEGS_GET_GUILD_SKILL_IN_BOARD_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_GET_GUILD_SKILL_IN_BOARD_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_INIT_GUILD_SKILL_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_INIT_GUILD_SKILL_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Like guild creation, this names an item by UID that a success would
	// consume. Refused, and the item stays.
	KEGS_INIT_GUILD_SKILL_ACK kAck;
	kAck.m_iOK = NetError::ERR_GUILD_19;

	return Reply( kSes, EGS_INIT_GUILD_SKILL_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_RESET_GUILD_SKILL_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_RESET_GUILD_SKILL_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_RESET_GUILD_SKILL_ACK kAck;
	kAck.m_iOK = NetError::ERR_GUILD_19;

	return Reply( kSes, EGS_RESET_GUILD_SKILL_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// party
//
// The plan calls for "a solo party of one". That turns out to be the wrong
// shape: the client does not have a party until a EGS_REGROUP_PARTY_NOT tells
// it it does, and every button in the party UI then acts on members that do not
// exist. Having no party at all is both simpler and closer to the truth - a
// player alone on live has no party either, and the dungeon flow phase 4 built
// runs entirely without one.
//
// So each of these answers "you are not in a party" and the party window stays
// the empty one the client starts with.

bool CX2OfflineServer::Handler_EGS_INVITE_PARTY_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_INVITE_PARTY_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineLog::Server( L"PARTY    invite to '%s' refused - nobody else is online",
		kReq.m_wstrNickName.c_str() );

	KEGS_INVITE_PARTY_ACK kAck;
	kAck.m_iOK			= NetError::ERR_PARTY_07;		///< cannot be invited right now
	kAck.m_iReceiverUID	= kReq.m_iReceiverUID;

	return Reply( kSes, EGS_INVITE_PARTY_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_LEAVE_PARTY_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_LEAVE_PARTY_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// NET_OK, not an error: "leave the party" when there is no party has
	// already achieved what it asked for, and the client's handler tears down
	// its own party UI on success - which is the state it should be in.
	KEGS_LEAVE_PARTY_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_LEAVE_PARTY_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_CHANGE_PARTY_TYPE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CHANGE_PARTY_TYPE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_CHANGE_PARTY_TYPE_ACK kAck;
	kAck.m_iOK = NetError::ERR_PARTY_09;			///< no party
	return Reply( kSes, EGS_CHANGE_PARTY_TYPE_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_PARTY_CHANGE_DUNGEON_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_PARTY_CHANGE_DUNGEON_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_PARTY_CHANGE_DUNGEON_ACK kAck;
	kAck.m_iOK = NetError::ERR_PARTY_09;
	return Reply( kSes, EGS_PARTY_CHANGE_DUNGEON_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_PARTY_CHANGE_HOST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_PARTY_CHANGE_HOST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_PARTY_CHANGE_HOST_ACK kAck;
	kAck.m_iOK = NetError::ERR_PARTY_09;
	return Reply( kSes, EGS_PARTY_CHANGE_HOST_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_PARTY_CHANGE_GET_ITEM_TYPE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_PARTY_CHANGE_GET_ITEM_TYPE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_PARTY_CHANGE_GET_ITEM_TYPE_ACK kAck;
	kAck.m_iOK = NetError::ERR_PARTY_09;
	return Reply( kSes, EGS_PARTY_CHANGE_GET_ITEM_TYPE_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_PARTY_CHANGE_NUM_OF_PER_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_PARTY_CHANGE_NUM_OF_PER_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_PARTY_CHANGE_NUM_OF_PER_ACK kAck;
	kAck.m_iOK = NetError::ERR_PARTY_09;
	return Reply( kSes, EGS_PARTY_CHANGE_NUM_OF_PER_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_PARTY_CHANGE_CHECK_MIN_LEVEL_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_PARTY_CHANGE_CHECK_MIN_LEVEL_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_PARTY_CHANGE_CHECK_MIN_LEVEL_ACK kAck;
	kAck.m_iOK = NetError::ERR_PARTY_09;
	return Reply( kSes, EGS_PARTY_CHANGE_CHECK_MIN_LEVEL_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_PARTY_BAN_USER_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_PARTY_BAN_USER_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_PARTY_BAN_USER_ACK kAck;
	kAck.m_iOK = NetError::ERR_PARTY_09;
	return Reply( kSes, EGS_PARTY_BAN_USER_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_PARTY_GAME_START_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_PARTY_GAME_START_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// This is the party's "everyone in" button, not the solo one - a solo run
	// goes through EGS_CREATE_ROOM_REQ and EGS_STATE_CHANGE_GAME_START_REQ,
	// which phase 4 handles. Reaching it means the client thinks it has a
	// party, which it should not.
	CX2OfflineLog::Server( L"PARTY    party game start refused - there is no party offline" );

	KEGS_PARTY_GAME_START_ACK kAck;
	kAck.m_iOK = NetError::ERR_PARTY_09;
	return Reply( kSes, EGS_PARTY_GAME_START_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_REQUEST_MATCH_MAKING_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_REQUEST_MATCH_MAKING_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// PvP matchmaking. PvP is out of scope for the whole project, and there is
	// nobody to match with regardless.
	CX2OfflineLog::Server( L"PARTY    PvP matchmaking refused - PvP is out of scope offline" );

	KEGS_REQUEST_MATCH_MAKING_ACK kAck;
	kAck.m_iOK = NetError::ERR_PARTY_23;			///< could not start the game
	return Reply( kSes, EGS_REQUEST_MATCH_MAKING_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_AUTO_PARTY_DUNGEON_GAME_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_AUTO_PARTY_DUNGEON_GAME_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// AI party members (AI_PARTY_PLAN.md phase 1). Auto-party used to be
	// refused with ERR_PARTY_23 because there is nobody offline to be matched
	// with; it now fills the remaining slots with AI-controlled hero NPCs
	// instead of searching for humans.
	//
	// KEGS_AUTO_PARTY_DUNGEON_GAME_REQ is a typedef of
	// KEGS_QUICK_START_DUNGEON_GAME_REQ (ClientPacket.h:7804) - same dungeon
	// id, difficulty, get-item type and mode - so this is the solo path with
	// bots added and a different ACK, sharing OpenDungeonGameRoom and
	// SendDungeonGameStartNot with it rather than copying them. The ONLY thing
	// that differs between the two buttons is the MakePartyBots call below.
	//
	// This is the shortest legal auto-party path: ACK, then straight to
	// EGS_PARTY_GAME_START_NOT. The matchmaking ceremony the client's own flow
	// expects in between - EGS_REG_AUTO_PARTY_WAIT_LIST_SUCCESS_NOT, the accept
	// popup, the cancel path - is phase 3. The player presses the button and
	// arrives in the dungeon with no queue.
	KEGS_AUTO_PARTY_DUNGEON_GAME_ACK kAck;
	kAck.m_iOK					= NetError::NET_OK;
	kAck.m_wstrFailUserNickName	= L"";

	KOfflineUnitRow kRow;
	if( false == OpenDungeonGameRoom( kSes, kReq, kRow ) )
	{
		CX2OfflineLog::Server( L"AIPARTY  auto-party refused - could not open the dungeon room" );

		kAck.m_iOK = NetError::ERR_PARTY_23;
		return Reply( kSes, EGS_AUTO_PARTY_DUNGEON_GAME_ACK, kAck );
	}

	// The one line that separates auto-party from the solo button. Everything
	// downstream - the slot list, the room's join count, the negative UIDs the
	// NPC create handler hands out - reads m_kRoom.m_vecBot, which every other
	// entry point leaves empty.
	MakePartyBots( kRow, AUTO_PARTY_BOT_NUM );

	Reply( kSes, EGS_AUTO_PARTY_DUNGEON_GAME_ACK, kAck );

	return SendDungeonGameStartNot( kSes, kRow );
}

bool CX2OfflineServer::Handler_EGS_PVP_PARTY_CHANGE_MATCH_INFO_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_PVP_PARTY_CHANGE_MATCH_INFO_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_PVP_PARTY_CHANGE_MATCH_INFO_ACK kAck;
	kAck.m_iOK = NetError::ERR_PARTY_09;
	return Reply( kSes, EGS_PVP_PARTY_CHANGE_MATCH_INFO_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// friends, the messenger, and the community window

bool CX2OfflineServer::Handler_EGS_COMMUNITY_USER_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_COMMUNITY_USER_LIST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// "who else is in this village". Nobody. The client holds
	// m_bUpdateFieldUser until this ACK arrives and will not ask again while it
	// is set, so an unanswered request freezes the list for the rest of the
	// session rather than for one refresh.
	KEGS_COMMUNITY_USER_LIST_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_uiTotalPage	= 1;
	kAck.m_uiViewPage	= kReq.m_uiViewPage;

	return Reply( kSes, EGS_COMMUNITY_USER_LIST_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_COMMUNITY_USER_LIST_MONITORING_MODE_REQ( KOfflineSession& kSes,
																			const KEvent& kEvent )
{
	KEGS_COMMUNITY_USER_LIST_MONITORING_MODE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// The GM-only variant of the list above, and it waits on the SAME
	// EGS_COMMUNITY_USER_LIST_ACK (X2Community.cpp:7899) - there is no
	// _MONITORING_MODE_ACK event ID at all. Reachable only at auth level
	// XUAL_DEV, which offline nothing sets, but it costs three lines to be
	// right about.
	KEGS_COMMUNITY_USER_LIST_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_uiTotalPage	= 1;
	kAck.m_uiViewPage	= kReq.m_uiViewPage;

	return Reply( kSes, EGS_COMMUNITY_USER_LIST_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_GET_RECOMMEND_USER_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GET_RECOMMEND_USER_LIST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_GET_RECOMMEND_USER_LIST_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_uiTotalPage	= 1;
	kAck.m_uiViewPage	= 1;

	return Reply( kSes, EGS_GET_RECOMMEND_USER_LIST_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_REQUEST_FRIEND_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_REQUEST_FRIEND_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineLog::Server( L"FRIEND   request to '%s' refused - no such character exists",
		kReq.m_wstrNickName.c_str() );

	KEGS_REQUEST_FRIEND_ACK kAck;
	kAck.m_iOK			= NetError::ERR_MESSENGER_04;	///< no such character
	kAck.m_iUnitUID		= 0;
	kAck.m_wstrNickName	= kReq.m_wstrNickName;

	return Reply( kSes, EGS_REQUEST_FRIEND_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_ACCEPT_FRIEND_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_ACCEPT_FRIEND_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_ACCEPT_FRIEND_ACK kAck;
	kAck.m_iOK		= NetError::ERR_MESSENGER_02;	///< nobody has asked to be your friend
	kAck.m_iUnitUID	= 0;

	return Reply( kSes, EGS_ACCEPT_FRIEND_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_DENY_FRIEND_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_DENY_FRIEND_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Denying a request that is not there leaves the list as the player wanted
	// it, so this succeeds where accepting fails.
	KEGS_DENY_FRIEND_ACK kAck;
	kAck.m_iOK		= NetError::NET_OK;
	kAck.m_iUnitUID	= 0;

	return Reply( kSes, EGS_DENY_FRIEND_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_BLOCK_FRIEND_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_BLOCK_FRIEND_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_BLOCK_FRIEND_ACK kAck;
	kAck.m_iOK		= NetError::ERR_MESSENGER_03;	///< that character is not a friend
	kAck.m_iUnitUID	= 0;

	return Reply( kSes, EGS_BLOCK_FRIEND_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_UNBLOCK_FRIEND_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_UNBLOCK_FRIEND_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_UNBLOCK_FRIEND_ACK kAck;
	kAck.m_iOK		= NetError::NET_OK;
	kAck.m_iUnitUID	= 0;

	return Reply( kSes, EGS_UNBLOCK_FRIEND_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_DELETE_FRIEND_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_DELETE_FRIEND_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_DELETE_FRIEND_ACK kAck;
	kAck.m_iOK		= NetError::NET_OK;
	kAck.m_iUnitUID	= 0;

	return Reply( kSes, EGS_DELETE_FRIEND_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_MOVE_FRIEND_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_MOVE_FRIEND_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_MOVE_FRIEND_ACK kAck;
	kAck.m_iOK				= NetError::ERR_MESSENGER_03;
	kAck.m_iUnitUID			= 0;
	kAck.m_cTargetGroupID	= 0;

	return Reply( kSes, EGS_MOVE_FRIEND_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_MAKE_FRIEND_GROUP_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_MAKE_FRIEND_GROUP_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Friend groups are folders in the player's own list, not other players -
	// but nothing persists them offline, so a group that "worked" would vanish
	// on the next login. Refused, which is at least consistent.
	KEGS_MAKE_FRIEND_GROUP_ACK kAck;
	kAck.m_iOK			= NetError::ERR_MESSENGER_07;	///< cannot make any more groups
	kAck.m_cGroupID		= 0;
	kAck.m_wstrGroupName= L"";

	return Reply( kSes, EGS_MAKE_FRIEND_GROUP_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_RENAME_FRIEND_GROUP_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_RENAME_FRIEND_GROUP_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_RENAME_FRIEND_GROUP_ACK kAck;
	kAck.m_iOK			= NetError::ERR_MESSENGER_04;
	kAck.m_cGroupID		= 0;
	kAck.m_wstrGroupName= L"";

	return Reply( kSes, EGS_RENAME_FRIEND_GROUP_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_DELETE_FRIEND_GROUP_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_DELETE_FRIEND_GROUP_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_DELETE_FRIEND_GROUP_ACK kAck;
	kAck.m_iOK		= NetError::NET_OK;
	kAck.m_cGroupID	= 0;

	return Reply( kSes, EGS_DELETE_FRIEND_GROUP_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_UPDATE_FRIEND_INFO_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_UPDATE_FRIEND_INFO_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_UPDATE_FRIEND_INFO_ACK kAck;
	kAck.m_iOK					= NetError::ERR_MESSENGER_03;
	kAck.m_wstrFriendNickName	= L"";
	kAck.m_iFriendUnitUID		= 0;
	kAck.m_iChannelID			= 0;

	return Reply( kSes, EGS_UPDATE_FRIEND_INFO_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_KNM_REQUEST_NEW_FRIEND_INFO_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_KNM_REQUEST_NEW_FRIEND_INFO_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_KNM_REQUEST_NEW_FRIEND_INFO_ACK kAck;
	kAck.m_iOK				= NetError::ERR_MESSENGER_04;
	kAck.m_uiKNMSerialNum	= 0;
	kAck.m_wstrUnitNickName	= L"";

	return Reply( kSes, EGS_KNM_REQUEST_NEW_FRIEND_INFO_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_GET_MY_MESSENGER_SN_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	// The messenger serial number identifies this character to the KOG
	// messenger service. There is no service; the character's own UID is a
	// stable number that is unique across the save, which is everything the
	// client uses it for.
	KEGS_GET_MY_MESSENGER_SN_ACK kAck;
	kAck.m_iOK				= NetError::NET_OK;
	kAck.m_uiKNMSerialNum	= (u_int)kSes.m_nSelectedUnitUID;

	return Reply( kSes, EGS_GET_MY_MESSENGER_SN_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_ED_MONITORING_BLOCK_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_ED_MONITORING_BLOCK_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_ED_MONITORING_BLOCK_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_ED_MONITORING_BLOCK_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_USER_COMMUNITY_SURVEY_NEW_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_USER_COMMUNITY_SURVEY_NEW_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_USER_COMMUNITY_SURVEY_NEW_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_USER_COMMUNITY_SURVEY_NEW_ACK, kAck );
}


bool CX2OfflineServer::Handler_EGS_INVITE_PVP_ROOM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_INVITE_PVP_ROOM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_INVITE_PVP_ROOM_ACK kAck;
	kAck.m_iOK			= NetError::ERR_MESSENGER_04;
	kAck.m_wstrNickName	= L"";

	return Reply( kSes, EGS_INVITE_PVP_ROOM_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_UPDATE_COMMUNITY_OPTION_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_UPDATE_COMMUNITY_OPTION_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// "refuse whispers / refuse party invites / refuse guild invites". Echoed
	// straight back, so the checkbox the player just ticked stays ticked for
	// the session. Not persisted - there is nobody for the option to keep out.
	KEGS_UPDATE_COMMUNITY_OPTION_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_kDenyOptions	= kReq.m_kDenyOptions;

	return Reply( kSes, EGS_UPDATE_COMMUNITY_OPTION_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_SEARCH_UNIT_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_SEARCH_UNIT_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_SEARCH_UNIT_ACK kAck;
	kAck.m_iOK = NetError::ERR_MESSENGER_04;

	return Reply( kSes, EGS_SEARCH_UNIT_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_GET_CONNECTION_UNIT_INFO_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GET_CONNECTION_UNIT_INFO_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_GET_CONNECTION_UNIT_INFO_ACK kAck;
	kAck.m_iOK = NetError::ERR_MESSENGER_04;

	return Reply( kSes, EGS_GET_CONNECTION_UNIT_INFO_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_WATCH_UNIT_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_WATCH_UNIT_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_WATCH_UNIT_ACK kAck;
	kAck.m_iOK = NetError::ERR_MESSENGER_04;

	return Reply( kSes, EGS_WATCH_UNIT_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// mail

bool CX2OfflineServer::Handler_EGS_GET_POST_LETTER_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GET_POST_LETTER_LIST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// An empty inbox, and NET_OK. Nothing offline writes a letter - not even
	// the system, because the two things that send system mail on live (a
	// disconnected dungeon reward and an expired cash item) do not exist here.
	KEGS_GET_POST_LETTER_LIST_ACK kAck;
	kAck.m_iOK					= NetError::NET_OK;
	kAck.m_iTotalLetterCount	= 0;

	return Reply( kSes, EGS_GET_POST_LETTER_LIST_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_READ_LETTER_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_READ_LETTER_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_READ_LETTER_ACK kAck;
	kAck.m_iOK = NetError::ERR_POST_LETTER_04;		///< no such letter

	return Reply( kSes, EGS_READ_LETTER_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_DELETE_LETTER_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_DELETE_LETTER_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Deleting from an empty inbox succeeds: the mailbox ends up in the state
	// asked for, and the client removes the rows it selected.
	KEGS_DELETE_LETTER_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_vecPostNo	= kReq.m_vecPostNo;

	return Reply( kSes, EGS_DELETE_LETTER_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_SEND_LETTER_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_SEND_LETTER_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// ERR_POST_LETTER_10 is "that nickname does not exist", which is exactly
	// the case: the only character in the save is the sender. Refusing before
	// anything is charged matters here - a letter costs ED and can carry items,
	// and both have to stay where they are.
	CX2OfflineLog::Server( L"MAIL     letter refused - there is no other character to send it to" );

	KEGS_SEND_LETTER_ACK kAck;
	kAck.m_iOK	= NetError::ERR_POST_LETTER_10;
	kAck.m_iED	= 0;

	return Reply( kSes, EGS_SEND_LETTER_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_GET_ITEM_FROM_LETTER_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GET_ITEM_FROM_LETTER_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_GET_ITEM_FROM_LETTER_ACK kAck;
	kAck.m_iOK		= NetError::ERR_POST_LETTER_04;
	kAck.m_iPostNo	= kReq.m_iPostNo;
	kAck.m_iED		= 0;

	return Reply( kSes, EGS_GET_ITEM_FROM_LETTER_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_GET_POST_BLACK_LIST_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	// This ACK has no m_iOK: the list is the whole answer.
	KEGS_GET_POST_BLACK_LIST_ACK kAck;

	return Reply( kSes, EGS_GET_POST_BLACK_LIST_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_NEW_POST_BLACK_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_NEW_POST_BLACK_LIST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_NEW_POST_BLACK_LIST_ACK kAck;
	kAck.m_iOK			= NetError::ERR_POST_LETTER_10;
	kAck.m_wstrNickName	= L"";

	return Reply( kSes, EGS_NEW_POST_BLACK_LIST_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_DEL_POST_BLACK_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_DEL_POST_BLACK_LIST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_DEL_POST_BLACK_LIST_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_wstrNickName	= L"";

	return Reply( kSes, EGS_DEL_POST_BLACK_LIST_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// chat
//
// The only system in this file that actually works, and it has to: the client
// does NOT echo a line it sends. CX2ChatBox::Handler_EGS_CHAT_REQ hands the
// packet to the server and stops; what puts the line on screen is
// EGS_CHAT_NOT coming back, which on live is the broadcast to everyone in
// range including the sender. Drop it and the chat box swallows everything the
// player types.

bool CX2OfflineServer::Handler_EGS_CHAT_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CHAT_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// A whisper needs a recipient, and there is none. ERR_CHAT_04 is "that user
	// is not connected", which is true and which the client turns into a
	// readable message rather than a silent nothing.
	if( KEGS_CHAT_REQ::CPT_WHISPER	== kReq.m_cChatPacketType ||
		KEGS_CHAT_REQ::CPT_MESSENGER== kReq.m_cChatPacketType )
	{
		KEGS_CHAT_ACK kAck;
		kAck.m_iOK = NetError::ERR_CHAT_04;

		return Reply( kSes, EGS_CHAT_ACK, kAck );
	}

	// A megaphone line is a paid broadcast and goes out through
	// EGS_USE_MEGAPHONE_REQ, not here; if one arrives on this path it is
	// treated as ordinary chat, which is what it looks like locally anyway.
	std::wstring wstrNickName;

	KOfflineUnitRow kUnit;
	if( true == CX2OfflineDB::Instance()->LoadUnit( kSes.m_nSelectedUnitUID, kUnit ) )
		wstrNickName = kUnit.m_wstrNickName;

	KEGS_CHAT_NOT kNot;
	kNot.m_cChatPacketType			= kReq.m_cChatPacketType;
	kNot.m_SenderUnitUID			= kSes.m_nSelectedUnitUID;
	kNot.m_wstrSenderUnitNickName	= wstrNickName;
	kNot.m_wstrRecieverUnitNickName	= kReq.m_wstrRecieverUnitNickName;
	kNot.m_RecieverUnitUID			= kReq.m_ToUnitUID;
	kNot.m_cRoomChatType			= kReq.m_cRoomChatType;
	kNot.m_wstrMsg					= kReq.m_wstrMsg;

	return Reply( kSes, EGS_CHAT_NOT, kNot );
}

bool CX2OfflineServer::Handler_EGS_CHAT_OPTION_INFO_WRITE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CHAT_OPTION_INFO_WRITE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Which chat tabs show which channels. Accepted and dropped, the same way
	// the keyboard map was in phase 1 before it was persisted - the client
	// keeps its own copy for the session, so the option works until relog.
	KEGS_CHAT_OPTION_INFO_WRITE_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_CHAT_OPTION_INFO_WRITE_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_NEW_BLACKLIST_USER_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_NEW_BLACKLIST_USER_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_NEW_BLACKLIST_USER_ACK kAck;
	kAck.m_iOK = NetError::ERR_CHAT_04;

	return Reply( kSes, EGS_NEW_BLACKLIST_USER_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_DEL_BLACKLIST_USER_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_DEL_BLACKLIST_USER_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_DEL_BLACKLIST_USER_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_DEL_BLACKLIST_USER_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_USE_MEGAPHONE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_USE_MEGAPHONE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// A megaphone is a consumable that broadcasts to the whole channel. There
	// is no channel, so the megaphone is NOT consumed and the request is
	// refused - spending an item to shout at nobody is worse than the refusal.
	CX2OfflineLog::Server( L"CHAT     megaphone refused - there is nobody to broadcast to"
		L" (the megaphone is left in the bag)" );

	KEGS_USE_MEGAPHONE_ACK kAck;
	kAck.m_iOK = NetError::ERR_CHAT_04;

	return Reply( kSes, EGS_USE_MEGAPHONE_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// ranking
//
// Only the Henir / dungeon / PvP boards are here. The OTHER ranking system -
// "local ranking", the one CX2ProfileManager drives - is compiled out of this
// build entirely: SERV_LOCAL_RANKING is off, so its five packet structs do not
// exist and the client cannot send them. Same story for the relationship
// (marriage) system, which is why there is no EGS_RELATIONSHIP_INFO_REQ
// handler either.

bool CX2OfflineServer::Handler_EGS_GET_RANKING_INFO_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GET_RANKING_INFO_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Every board empty, including "my rank" - which is honest: a ranking of
	// one player against nobody is not a rank. This ACK has no m_iOK, so an
	// empty answer is the only answer there is.
	KEGS_GET_RANKING_INFO_ACK kAck;
	kAck.m_iRankingType	= kReq.m_iRankingType;
	kAck.m_uiViewPage	= kReq.m_uiViewPage;
	kAck.m_uiTotalPage	= 1;

	return Reply( kSes, EGS_GET_RANKING_INFO_ACK, kAck );
}





//////////////////////////////////////////////////////////////////////////
// the tutor / apprentice system

bool CX2OfflineServer::Handler_EGS_TUTORIAL_STUDENT_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_TUTORIAL_STUDENT_LIST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_TUTORIAL_STUDENT_LIST_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_iTotalPage	= 1;
	kAck.m_iViewPage	= 1;

	return Reply( kSes, EGS_TUTORIAL_STUDENT_LIST_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_REQUEST_TUTORIAL_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_REQUEST_TUTORIAL_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_REQUEST_TUTORIAL_ACK kAck;
	kAck.m_iOK = NetError::ERR_TUTORIAL_01;			///< could not fetch the apprentice list

	return Reply( kSes, EGS_REQUEST_TUTORIAL_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_DEL_TUTORIAL_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_DEL_TUTORIAL_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_DEL_TUTORIAL_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_DEL_TUTORIAL_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// trading, the shared bank, and the odds and ends that block a menu

bool CX2OfflineServer::Handler_EGS_GET_SHARE_BANK_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GET_SHARE_BANK_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// The account bank. PRIVATE_BANK is on for this build, so the button is
	// there - but nothing offline stores a bank and CX2OfflineInventory has no
	// ST_BANK category behind it, so it opens empty with a size of zero rather
	// than opening onto slots that swallow items.
	CX2OfflineLog::Server( L"BANK     shared bank opened empty - it is not modelled offline" );

	KEGS_GET_SHARE_BANK_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_iUnitUID		= kSes.m_nSelectedUnitUID;
	kAck.m_wstrNickName	= kReq.m_wstrNickName;
	kAck.m_iBankSize	= 0;

	return Reply( kSes, EGS_GET_SHARE_BANK_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_GET_TEMP_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GET_TEMP_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// The temporary inventory holds items a full bag could not take during a
	// dungeon. Phase 5's InsertItem never overflows into one - it stops when
	// the bag is full and says so - so this list is always empty.
	KEGS_GET_TEMP_ITEM_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_iItemID		= 0;
	kAck.m_iTempItemUID	= 0;

	return Reply( kSes, EGS_GET_TEMP_ITEM_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_DELETE_TEMP_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_DELETE_TEMP_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_DELETE_TEMP_ITEM_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_iTempItemUID	= 0;

	return Reply( kSes, EGS_DELETE_TEMP_ITEM_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_SEARCH_TRADE_BOARD_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_SEARCH_TRADE_BOARD_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_SEARCH_TRADE_BOARD_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_SEARCH_TRADE_BOARD_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_CHECK_SUM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CHECK_SUM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// A client-file integrity challenge. There is no reference to check the
	// answer against offline, and the whole anti-cheat stack is already
	// compiled out of this build (NO_GAMEGUARD, no XTrap) - so it passes.
	KEGS_CHECK_SUM_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_CHECK_SUM_ACK, kAck );
}


//////////////////////////////////////////////////////////////////////////
// pets and riding pets
//
// These are not stubs. A pet is entirely a property of one character - it has
// no other player in it anywhere - so it persists in SQLite like everything
// else, and the pet window works.
//
// What it needs that this tree does not carry packed:
// KncWX2Server/ServerResource/US/PetData.lua and RidingPetData.lua, which are
// what say WHICH pet a given egg hatches into. Without them a pet egg refuses
// with a log line naming the file, the same deal as CashItemPrice.lua and
// StatTable.lua. Hatching is the only thing that needs them; a pet already in
// the save loads, summons and feeds without them.

namespace
{
	/// KPetInfo out of a stored row. The date strings are what the client's
	/// hunger clock parses, so they go out in the one format
	/// KncUtil::ConvertStringToCTime accepts.
	void MakePetInfo( const KOfflinePetRow& kRow, OUT KPetInfo& kOut )
	{
		kOut.m_iPetUID			= (UidType)kRow.m_nPetUID;
#ifdef SERV_PETID_DATA_TYPE_CHANGE
		kOut.m_iPetID			= kRow.m_iPetID;
#else
		kOut.m_cPetID			= (char)kRow.m_iPetID;
#endif SERV_PETID_DATA_TYPE_CHANGE
		kOut.m_wstrPetName		= kRow.m_wstrName;
		kOut.m_cEvolutionStep	= (char)kRow.m_iEvolutionStep;
		kOut.m_sSatiety			= (short)kRow.m_iSatiety;
		kOut.m_iIntimacy		= kRow.m_iIntimacy;
		kOut.m_sExtroversion	= (short)kRow.m_iExtroversion;
		kOut.m_sEmotion			= (short)kRow.m_iEmotion;
		kOut.m_bAutoFeed		= kRow.m_bAutoFeed;

		kOut.m_wstrLastFeedDate		= CX2OfflineDB::FormatDate( kRow.m_tLastFeedDate );
		kOut.m_wstrLastSummonDate	= CX2OfflineDB::FormatDate( kRow.m_tLastSummonDate );
		kOut.m_wstrRegDate			= CX2OfflineDB::FormatDate( kRow.m_tRegDate );

#ifdef SERV_PET_AUTO_LOOTING
		kOut.m_bAutoLooting		= kRow.m_bAutoLooting;
#endif SERV_PET_AUTO_LOOTING
	}

	void MakeRidingPetInfo( const KOfflineRidingPetRow& kRow, OUT KRidingPetInfo& kOut )
	{
		kOut.m_iRidingPetUID		= (UidType)kRow.m_nRidingPetUID;
		kOut.m_usRindingPetID		= (USHORT)kRow.m_iRidingPetID;
		kOut.m_fStamina				= kRow.m_fStamina;
		kOut.m_wstrDestroyDate		= 0 == kRow.m_tDestroyDate
										? std::wstring( L"" )
										: CX2OfflineDB::FormatDate( kRow.m_tDestroyDate );
		kOut.m_iLastUnSummonDate	= kRow.m_tLastUnSummonDate;
	}
}

bool CX2OfflineServer::Handler_EGS_GET_PET_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GET_PET_LIST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Phase 5 answered this with a hardcoded empty list, and had to: an
	// unanswered request sets CX2UIPetInfo::m_bProcessPetList and the pet
	// window never refreshes again for the rest of the run. Now the list is
	// real, out of unit_pet.
	std::vector< KOfflinePetRow > vecPet;
	CX2OfflineDB::Instance()->LoadPets( kSes.m_nSelectedUnitUID, vecPet );

	KEGS_GET_PET_LIST_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_nTotalPage	= 1;					///< one page, never zero - see the guild lists
	kAck.m_nViewPage	= kReq.m_nViewPage;

	for( size_t i = 0; i < vecPet.size(); ++i )
	{
		KPetInfo kInfo;
		MakePetInfo( vecPet[i], kInfo );
		kAck.m_vecPetList.push_back( kInfo );
	}

	return Reply( kSes, EGS_GET_PET_LIST_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_GET_RIDING_PET_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GET_RIDING_PET_LIST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	std::vector< KOfflineRidingPetRow > vecPet;
	CX2OfflineDB::Instance()->LoadRidingPets( kSes.m_nSelectedUnitUID, vecPet );

	// This one is a hard 60-second AddServerPacket wait, so it is answered
	// whatever happens.
	KEGS_GET_RIDING_PET_LIST_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	for( size_t i = 0; i < vecPet.size(); ++i )
	{
		KRidingPetInfo kInfo;
		MakeRidingPetInfo( vecPet[i], kInfo );
		kAck.m_vecRidingPetList.push_back( kInfo );
	}

	return Reply( kSes, EGS_GET_RIDING_PET_LIST_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_CREATE_PET_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CREATE_PET_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineDB*			pDB		= CX2OfflineDB::Instance();
	CX2OfflineInventory*	pInven	= CX2OfflineInventory::Instance();
	CX2OfflinePetData*		pData	= CX2OfflinePetData::Instance();

	KEGS_CREATE_PET_ACK kAck;
	kAck.m_iOK = NetError::ERR_PET_00;

	// KUserPetManager::Handler_EGS_CREATE_PET_REQ's order, kept, because each
	// check has a distinct error the client words differently.

	// 1. The egg has to be in the bag.
	KOfflineItemRow kRow;
	if( false == pInven->GetItemRow( kReq.m_iItemUID, kRow ) )
	{
		kAck.m_iOK = NetError::ERR_ITEM_04;
		return Reply( kSes, EGS_CREATE_PET_ACK, kAck );
	}

	// 2. The name: unique across this character's pets, and at most 12 bytes
	// once narrowed - the live check is on the narrow string's length, so a
	// Korean name gets fewer characters out of it than a Latin one. Same rule.
	std::vector< KOfflinePetRow > vecPet;
	pDB->LoadPets( kSes.m_nSelectedUnitUID, vecPet );

	for( size_t i = 0; i < vecPet.size(); ++i )
	{
		if( vecPet[i].m_wstrName == kReq.m_wstrPetName )
		{
			kAck.m_iOK = NetError::ERR_PET_01;
			return Reply( kSes, EGS_CREATE_PET_ACK, kAck );
		}
	}

	if( KncUtil::toNarrowString( kReq.m_wstrPetName ).size() > 12 )
	{
		kAck.m_iOK = NetError::ERR_PET_02;
		return Reply( kSes, EGS_CREATE_PET_ACK, kAck );
	}

	// 3. Egg -> pet, out of PetData.lua.
	int iPetID		= 0;
	int iPeriodDays	= -1;

	if( false == pData->GetPetByItem( kRow.m_iItemID, iPetID, iPeriodDays ) || iPetID <= 0 )
	{
		if( false == pData->HasPetRows() )
		{
			CX2OfflineLog::Server( L"PET      hatching refused - PetData.lua is not packed into"
				L" data036.kom, so nothing knows what item %d hatches into (the egg is left in"
				L" the bag)", kRow.m_iItemID );
		}
		else
		{
			// Loaded, and this egg still has no pet. Either it is not an egg at
			// all, or - far likelier for something the client offered "use" on -
			// it hatches into a pet whose PET_UNIT_ID this build does not have.
			// See X2OfflinePetData.h.
			CX2OfflineLog::Server( L"PET      hatching refused - item %d has no pet in PetData.lua"
				L" that this client build can render (the egg is left in the bag)",
				kRow.m_iItemID );
		}

		return Reply( kSes, EGS_CREATE_PET_ACK, kAck );
	}

	// 4. Burn the egg. Before the pet row is written, so a failure there cannot
	// leave a pet that was never paid for.
	KInventoryItemInfo kSlotInfo;
	if( false == pInven->ConsumeOne( kReq.m_iItemUID, kSlotInfo ) )
	{
		kAck.m_iOK = NetError::ERR_ITEM_04;
		return Reply( kSes, EGS_CREATE_PET_ACK, kAck );
	}

	// 5. The pet, at PetData.lua's own starting stats - SetNewPetInfo at the top
	// of the file: evolution 0, satiety 3840, the rest 0. Not defaults invented
	// here; a pet that starts hungry or already evolved would drift from the live
	// game in a way nobody could later trace.
	const CX2OfflinePetData::KNewPetInfo& kNew = pData->GetNewPetInfo();

	const __int64 tNow = (__int64)::_time64( NULL );

	KOfflinePetRow kPet;
	kPet.m_iPetID			= iPetID;
	kPet.m_wstrName			= kReq.m_wstrPetName;
	kPet.m_iEvolutionStep	= kNew.m_iEvolutionStep;
	kPet.m_iSatiety			= kNew.m_iSatiety;
	kPet.m_iIntimacy		= kNew.m_iIntimacy;
	kPet.m_iExtroversion	= kNew.m_iExtroversion;
	kPet.m_iEmotion			= kNew.m_iEmotion;
	kPet.m_bAutoFeed		= false;
	kPet.m_bAutoLooting		= false;
	kPet.m_tLastFeedDate	= tNow;
	kPet.m_tLastSummonDate	= tNow;
	kPet.m_tRegDate			= tNow;

	if( false == pDB->InsertPet( kSes.m_nSelectedUnitUID, kPet ) )
	{
		// The egg is already gone. Say so rather than reporting a bland failure -
		// this is the one outcome the player cannot undo.
		CX2OfflineLog::Server( L"PET      ERROR hatched item %d into pet %d but the row would not"
			L" save - the egg has already been consumed", kRow.m_iItemID, iPetID );

		return Reply( kSes, EGS_CREATE_PET_ACK, kAck );
	}

	// A timed pet. unit_pet has no expiry column and adding one is a schema
	// change this is not the place for, so the pet is permanent and the log says
	// which pets that flattered. Only 4 of PetData.lua's 120 rows are timed, and
	// none of those four is sold in the offline shop.
	if( iPeriodDays > 0 )
	{
		CX2OfflineLog::Server( L"PET      NOTE item %d is a %d-day pet; offline it is permanent"
			L" (unit_pet has no expiry column)", kRow.m_iItemID, iPeriodDays );
	}

	MakePetInfo( kPet, kAck.m_kCreatedPetInfo );
	kAck.m_vecKInventorySlotInfo.push_back( kSlotInfo );
	kAck.m_iOK = NetError::NET_OK;

	CX2OfflineLog::Server( L"PET      hatched item %d into pet %d '%s' (petUID=%I64d)",
		kRow.m_iItemID, iPetID, kPet.m_wstrName.c_str(), (__int64)kPet.m_nPetUID );

	return Reply( kSes, EGS_CREATE_PET_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_SUMMON_PET_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_SUMMON_PET_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineDB* pDB = CX2OfflineDB::Instance();

	KEGS_SUMMON_PET_ACK kAck;
	kAck.m_iOK = NetError::ERR_PET_00;

	//////////////////////////////////////////////////////////////////////////
	// Author: Iruha
	// Date: 2026-09-05
	// Description: Phase 14. m_iSummonPetUID == 0 means "unsummon", the same
	// way UserPetManager.cpp:944 reads it on live - not "summon pet id 0". Left
	// to fall into the vecPet search below this always missed (no real pet has
	// UID 0) and answered ERR_PET_00, which the client shows as "Failed to
	// create the pet" for what was actually just the unsummon button.
	if( 0 == kReq.m_iSummonPetUID )
	{
		if( 0 == kSes.m_nSummonedPetUID )
		{
			kAck.m_iOK = NetError::ERR_PET_06; // no summoned pet

			CX2OfflineLog::Server( L"PET      unsummon refused - nothing is summoned"
				L" (kSes.m_nSummonedPetUID==0)" );
		}
		else
		{
			CX2OfflineLog::Server( L"PET      unsummoned petUID=%I64d",
				(__int64)kSes.m_nSummonedPetUID );

			//////////////////////////////////////////////////////////////////////////
			// Author: Iruha
			// Date: 2026-09-05
			// Description: Phase 28. Persist the unsummon - mirrors dbo.gup_
			// update_pet_call( petUID, 0 ), which live writes to the DB immediately
			// rather than only at logout. Without this the pet reads back as still
			// summoned on the next login even though the player put it away.
			std::vector< KOfflinePetRow > vecPet;
			pDB->LoadPets( kSes.m_nSelectedUnitUID, vecPet );

			for( size_t i = 0; i < vecPet.size(); ++i )
			{
				if( kSes.m_nSummonedPetUID != vecPet[i].m_nPetUID )
					continue;

				vecPet[i].m_bSummoned = false;
				pDB->SavePet( kSes.m_nSelectedUnitUID, vecPet[i] );
				break;
			}
			//////////////////////////////////////////////////////////////////////////

			kSes.m_nSummonedPetUID = 0;
			kAck.m_iOK = NetError::NET_OK;
			// kAck.m_kSummonedPetInfo is left default-constructed (m_iPetUID
			// == 0), which is what Handler_EGS_SUMMON_PET_ACK reads as "no pet
			// summoned" and clears the local pet on.
		}

		Reply( kSes, EGS_SUMMON_PET_ACK, kAck );

		//////////////////////////////////////////////////////////////////////////
		// Author: Iruha
		// Date: 2026-09-05
		// Description: Phase 14, second finding. The ACK only stores pet info on
		// the unit (see Handler_EGS_SUMMON_PET_ACK) - it never spawns or removes
		// the visible CX2PET. That happens in Handler_EGS_SUMMON_PET_NOT
		// (CreateGamePet / RemovePet), which live's DBE_SUMMON_PET_ACK always
		// relays back to the summoning player too (GSUserPet.cpp:367, solo-field
		// branch), but only after a successful summon change - never on the
		// ERR_PET_06 refusal above, which live's own handler returns out of
		// before ever reaching that relay. Without this the ACK alone explains
		// "pressed summon repeatedly, nothing happened" - NET_OK every time, no
		// dialog, but no pet either, because CreateGamePet() was never reached.
		// An empty m_vecPetInfo here is exactly what the client reads as
		// "unsummon" (X2PetManager.cpp:2324).
		if( NetError::NET_OK == kAck.m_iOK )
		{
			KEGS_SUMMON_PET_NOT kNot;
			kNot.m_iUnitUID = kSes.m_nSelectedUnitUID;

			return Reply( kSes, EGS_SUMMON_PET_NOT, kNot );
		}

		return true;
		//////////////////////////////////////////////////////////////////////////
	}
	//////////////////////////////////////////////////////////////////////////

	std::vector< KOfflinePetRow > vecPet;
	pDB->LoadPets( kSes.m_nSelectedUnitUID, vecPet );

	for( size_t i = 0; i < vecPet.size(); ++i )
	{
		if( (__int64)kReq.m_iSummonPetUID != vecPet[i].m_nPetUID )
			continue;

		//////////////////////////////////////////////////////////////////////////
		// Author: Iruha
		// Date: 2026-09-05
		// Description: Phase 28. Persist which pet is out - mirrors dbo.gup_
		// update_pet_call's pair of writes (..., 0 on whatever was summoned
		// before, then ..., 1 on the new one), so the pet reads back correctly
		// on the next login instead of needing the summon button pressed again.
		if( 0 != kSes.m_nSummonedPetUID && kSes.m_nSummonedPetUID != vecPet[i].m_nPetUID )
		{
			for( size_t j = 0; j < vecPet.size(); ++j )
			{
				if( kSes.m_nSummonedPetUID != vecPet[j].m_nPetUID )
					continue;

				vecPet[j].m_bSummoned = false;
				pDB->SavePet( kSes.m_nSelectedUnitUID, vecPet[j] );
				break;
			}
		}

		vecPet[i].m_bSummoned = true;
		//////////////////////////////////////////////////////////////////////////

		// The summon date is what the client's satiety clock counts from, so it
		// is written on every summon rather than only on the first.
		vecPet[i].m_tLastSummonDate = (__int64)::_time64( NULL );
		pDB->SavePet( kSes.m_nSelectedUnitUID, vecPet[i] );

		MakePetInfo( vecPet[i], kAck.m_kSummonedPetInfo );
		kAck.m_iOK = NetError::NET_OK;

		// EGS_FEED_PETS_REQ names only the food item, not the pet - this is
		// the only record of which one is out to feed. See KOfflineSession.
		kSes.m_nSummonedPetUID = vecPet[i].m_nPetUID;

		break;
	}

	if( NetError::NET_OK != kAck.m_iOK )
	{
		// This is the one path Phase 14 left uninstrumented and it is the one
		// that matters: requested a UID that matched nothing in vecPet. Log the
		// UID so a repeat of "Failed to create the pet" is provably this branch
		// and not the unsummon one above.
		CX2OfflineLog::Server( L"PET      summon refused - petUID=%I64d matches none of this"
			L" character's pets", (__int64)kReq.m_iSummonPetUID );

		// m_vecPetInventorySlotInfo stays empty: pet inventories are one of the
		// things phase 5 left out (see X2OfflineInventory.h), and an empty list is
		// what a pet whose templet has no inventory sends.
		return Reply( kSes, EGS_SUMMON_PET_ACK, kAck );
	}

	Reply( kSes, EGS_SUMMON_PET_ACK, kAck );

	// See the Phase 14 comment above: the actual spawn happens in
	// Handler_EGS_SUMMON_PET_NOT, driven by this relay-to-self.
	KEGS_SUMMON_PET_NOT kNot;
	kNot.m_iUnitUID = kSes.m_nSelectedUnitUID;
	kNot.m_vecPetInfo.push_back( kAck.m_kSummonedPetInfo );
	return Reply( kSes, EGS_SUMMON_PET_NOT, kNot );
}

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-05
// Description: Phase 28. See the KOfflineSession::m_bPetRestorePending
// comment and X2OfflineServer.h's declaration for the split rationale: which
// pet is out is decided once at character select; the visible spawn waits
// for the field to finish loading, because CreateGamePet needs g_pX2Game to
// already exist (X2PetManager.cpp) and it does not yet on the character-
// select screen.
void CX2OfflineServer::RestoreSummonedPet( KOfflineSession& kSes, UidType nUnitUID )
{
	CX2OfflineDB* pDB = CX2OfflineDB::Instance();

	std::vector< KOfflinePetRow > vecPet;
	pDB->LoadPets( nUnitUID, vecPet );

	for( size_t i = 0; i < vecPet.size(); ++i )
	{
		if( false == vecPet[i].m_bSummoned )
			continue;

		kSes.m_nSummonedPetUID		= vecPet[i].m_nPetUID;
		kSes.m_bPetRestorePending	= true;

		CX2OfflineLog::Server( L"PET      restoring summoned petUID=%I64d on login",
			(__int64)vecPet[i].m_nPetUID );

		return;
	}
}

void CX2OfflineServer::SendPendingPetRestore( KOfflineSession& kSes )
{
	if( false == kSes.m_bPetRestorePending )
		return;

	// One-shot regardless of outcome below - see the m_bPetRestorePending
	// comment for why this must not fire again on the next field transition.
	kSes.m_bPetRestorePending = false;

	if( 0 == kSes.m_nSummonedPetUID )
		return;

	CX2OfflineDB* pDB = CX2OfflineDB::Instance();

	std::vector< KOfflinePetRow > vecPet;
	pDB->LoadPets( kSes.m_nSelectedUnitUID, vecPet );

	for( size_t i = 0; i < vecPet.size(); ++i )
	{
		if( kSes.m_nSummonedPetUID != vecPet[i].m_nPetUID )
			continue;

		KPetInfo kInfo;
		MakePetInfo( vecPet[i], kInfo );

		//////////////////////////////////////////////////////////////////////////
		// Author: Iruha
		// Date: 2026-09-05
		// Description: Phase 28, second finding. The _NOT relay alone spawns the
		// visible CX2PET but leaves the pet menu blank (no render, no name) and
		// its Summon/Unsummon button reading "Summon" - both read
		// CX2Unit::GetPetInfo() (X2UIPetInfo.cpp:1154 etc.), which
		// Handler_EGS_SUMMON_PET_NOT only populates via SetFullPetInfo when
		// bMyPet is false (X2PetManager.cpp:2426) - i.e. it assumes the ACK
		// already did it for your own pet, which is true on a real summon but
		// not here, since there was no EGS_SUMMON_PET_REQ this session to answer.
		// Sending the ACK first - unsolicited, but UIServerEventProc dispatches
		// it unconditionally, and CX2PetManager::Handler_EGS_SUMMON_PET_REQ
		// never registers an AddServerPacket wait for it - reproduces the same
		// two-packet order a live summon uses and is what actually calls
		// SetFullPetInfo.
		KEGS_SUMMON_PET_ACK kAck;
		kAck.m_iOK = NetError::NET_OK;
		kAck.m_kSummonedPetInfo = kInfo;
		Reply( kSes, EGS_SUMMON_PET_ACK, kAck );
		//////////////////////////////////////////////////////////////////////////

		KEGS_SUMMON_PET_NOT kNot;
		kNot.m_iUnitUID = kSes.m_nSelectedUnitUID;
		kNot.m_vecPetInfo.push_back( kInfo );

		Reply( kSes, EGS_SUMMON_PET_NOT, kNot );

		CX2OfflineLog::Server( L"PET      re-summoned petUID=%I64d after relog/char-switch",
			(__int64)vecPet[i].m_nPetUID );

		return;
	}

	// The pet named by the persisted flag no longer exists (released, or the
	// save file was hand-edited) - clear the stale session state instead of
	// silently doing nothing every field load for the rest of the session.
	CX2OfflineLog::Server( L"PET      WARNING petUID=%I64d was marked summoned but no longer"
		L" exists - clearing the session flag", (__int64)kSes.m_nSummonedPetUID );

	kSes.m_nSummonedPetUID = 0;
}
//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_FEED_PETS_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_FEED_PETS_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_FEED_PETS_ACK kAck;
	kAck.m_iOK		= NetError::ERR_PET_00;
	kAck.m_sSatiety	= 0;

	CX2OfflineDB*			pDB		= CX2OfflineDB::Instance();
	CX2OfflineInventory*	pInven	= CX2OfflineInventory::Instance();
	CX2OfflinePetData*		pData	= CX2OfflinePetData::Instance();

	// 1. A pet has to be out. EGS_FEED_PETS_REQ names only the food item, so
	// this session's own record of the last successful EGS_SUMMON_PET_REQ is
	// the only place to find which one.
	if( 0 == kSes.m_nSummonedPetUID )
	{
		kAck.m_iOK = NetError::ERR_PET_06; // no summoned pet
		return Reply( kSes, EGS_FEED_PETS_ACK, kAck );
	}

	// 2. The food has to be in the bag.
	KOfflineItemRow kRow;
	if( false == pInven->GetItemRow( kReq.m_iItemUID, kRow ) )
	{
		kAck.m_iOK = NetError::ERR_ITEM_04;
		return Reply( kSes, EGS_FEED_PETS_ACK, kAck );
	}

	// 3. The item -> satiety table, out of PetData.lua.
	int iSatietyGain = 0;
	if( false == pData->GetFeedSatietyGain( kRow.m_iItemID, iSatietyGain ) )
	{
		if( false == pData->HasPetRows() )
		{
			CX2OfflineLog::Server( L"PET      feeding refused - PetData.lua is not packed into"
				L" data036.kom, so nothing knows what item %d feeds (the item is left in the"
				L" bag)", kRow.m_iItemID );
		}
		else
		{
			CX2OfflineLog::Server( L"PET      feeding refused - item %d is not one of"
				L" PetData.lua's AddPetCashFeedItemInfo rows (the item is left in the bag)",
				kRow.m_iItemID );
		}

		kAck.m_iOK = NetError::ERR_PET_18; // not a food item this pet can eat
		return Reply( kSes, EGS_FEED_PETS_ACK, kAck );
	}

	// 4. The summoned pet's own row.
	std::vector< KOfflinePetRow > vecPet;
	pDB->LoadPets( kSes.m_nSelectedUnitUID, vecPet );

	size_t iPetIndex = vecPet.size();
	for( size_t i = 0; i < vecPet.size(); ++i )
	{
		if( kSes.m_nSummonedPetUID == vecPet[i].m_nPetUID )
		{
			iPetIndex = i;
			break;
		}
	}

	if( vecPet.size() == iPetIndex )
	{
		// The summoned pet was released or renamed away since the last
		// summon; nothing to feed until it is summoned again.
		kSes.m_nSummonedPetUID = 0;
		kAck.m_iOK = NetError::ERR_PET_06;
		return Reply( kSes, EGS_FEED_PETS_ACK, kAck );
	}

	// CX2PetManager::MAX_OF_SATIETY (X2Lib/X2PetManager.cpp:14) - the client's
	// own cap, not a guess.
	const int MAX_SATIETY = 4800;

	if( vecPet[iPetIndex].m_iSatiety >= MAX_SATIETY )
	{
		kAck.m_iOK = NetError::ERR_PET_10; // satiety already full
		return Reply( kSes, EGS_FEED_PETS_ACK, kAck );
	}

	// 5. Burn the food. Before the satiety row is written, same order as
	// CREATE_PET: a failure below cannot leave food consumed for nothing.
	KInventoryItemInfo kSlotInfo;
	if( false == pInven->ConsumeOne( kReq.m_iItemUID, kSlotInfo ) )
	{
		kAck.m_iOK = NetError::ERR_ITEM_04;
		return Reply( kSes, EGS_FEED_PETS_ACK, kAck );
	}

	int iNewSatiety = vecPet[iPetIndex].m_iSatiety + iSatietyGain;
	if( iNewSatiety > MAX_SATIETY )
		iNewSatiety = MAX_SATIETY;

	vecPet[iPetIndex].m_iSatiety		= iNewSatiety;
	vecPet[iPetIndex].m_tLastFeedDate	= (__int64)::_time64( NULL );

	pDB->SavePet( kSes.m_nSelectedUnitUID, vecPet[iPetIndex] );

	kAck.m_iOK		= NetError::NET_OK;
	kAck.m_sSatiety	= (short)iNewSatiety;
	kAck.m_vecInventorySlotInfo.push_back( kSlotInfo );

	CX2OfflineLog::Server( L"PET      fed pet %I64d item %d (+%d satiety, now %d/%d)",
		(__int64)vecPet[iPetIndex].m_nPetUID, kRow.m_iItemID, iSatietyGain, iNewSatiety, MAX_SATIETY );

	return Reply( kSes, EGS_FEED_PETS_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_COMMANDS_FOR_PETS_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_COMMANDS_FOR_PETS_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// "sit", "attack", "cheer". The pet's animation is entirely client side;
	// the server only relays the command to the other players in the room, and
	// there are none. NET_OK so the local pet plays its animation.
	KEGS_COMMANDS_FOR_PETS_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	Reply( kSes, EGS_COMMANDS_FOR_PETS_ACK, kAck );

	//////////////////////////////////////////////////////////////////////////
	// Author: Iruha
	// Date: 2026-09-05
	// Description: Phase 14. The ACK above only carries m_iOK -
	// Handler_EGS_COMMANDS_FOR_PETS_ACK does nothing with it but validate
	// that. The pet's actual state change (StateChange -> resets
	// m_fNowStateTimer) only happens in PlayEmotion, called from
	// Handler_EGS_PET_ACTION_NOT. Live relays this back to the sender too
	// (GSUserFunction.cpp's SendPetAction, solo-field branch) - "the other
	// players in the room, and there are none" above missed that the room
	// always includes yourself. Without it, CX2Pet::OnFrameMove's idle-90s
	// check (nowState stuck at m_WaitState) never clears and re-sends this
	// same REQ every frame - the 12,876x retry storm from the Phase 14 log.
	KEGS_PET_ACTION_NOT kNot;
	kNot.m_iUnitUID		= kSes.m_nSelectedUnitUID;
	kNot.m_cActionType	= kReq.m_cActionType;

	return Reply( kSes, EGS_PET_ACTION_NOT, kNot );
	//////////////////////////////////////////////////////////////////////////
}

bool CX2OfflineServer::Handler_EGS_PET_EVOLUTION_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_PET_EVOLUTION_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Evolution is gated on an intimacy threshold per step, which is
	// m_Evolution_Step_Max in the pet templet - server data again.
	CX2OfflineLog::Server( L"PET      evolution refused - the intimacy thresholds are in"
		L" PetData.lua, which is not packed" );

	KEGS_PET_EVOLUTION_ACK kAck;
	kAck.m_iOK = NetError::ERR_PET_00;

	return Reply( kSes, EGS_PET_EVOLUTION_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_CHANGE_PET_NAME_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CHANGE_PET_NAME_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineDB*			pDB		= CX2OfflineDB::Instance();
	CX2OfflineInventory*	pInven	= CX2OfflineInventory::Instance();

	KEGS_CHANGE_PET_NAME_ACK kAck;
	kAck.m_iOK = NetError::ERR_PET_00;

	std::vector< KOfflinePetRow > vecPet;
	pDB->LoadPets( kSes.m_nSelectedUnitUID, vecPet );

	// Exactly one pet can be renamed at a time and the request names the
	// rename ticket, not the pet - so the pet is the summoned one, which
	// offline is the only one there can be. With more than one stored, the
	// most recently summoned wins.
	int iBest = -1;
	for( size_t i = 0; i < vecPet.size(); ++i )
	{
		if( iBest < 0 || vecPet[i].m_tLastSummonDate > vecPet[iBest].m_tLastSummonDate )
			iBest = (int)i;
	}

	if( iBest < 0 )
		return Reply( kSes, EGS_CHANGE_PET_NAME_ACK, kAck );

	// The ticket is consumed, because unlike a refusal this one succeeds.
	KInventoryItemInfo kUsed;
	if( true == pInven->ConsumeOne( kReq.m_iItemUID, kUsed ) )
		kAck.m_vecKInventorySlotInfo.push_back( kUsed );

	vecPet[iBest].m_wstrName = kReq.m_wstrPetName;
	pDB->SavePet( kSes.m_nSelectedUnitUID, vecPet[iBest] );

	MakePetInfo( vecPet[iBest], kAck.m_kChangedPetInfo );
	kAck.m_iOK = NetError::NET_OK;

	CX2OfflineLog::Server( L"PET      renamed pet %I64d to '%s'",
		vecPet[iBest].m_nPetUID, kReq.m_wstrPetName.c_str() );

	return Reply( kSes, EGS_CHANGE_PET_NAME_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_SET_AUTO_FEED_PETS_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_SET_AUTO_FEED_PETS_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineDB* pDB = CX2OfflineDB::Instance();

	std::vector< KOfflinePetRow > vecPet;
	pDB->LoadPets( kSes.m_nSelectedUnitUID, vecPet );

	// The flag is per pet on the wire but the request does not name one, so it
	// goes on every pet the character owns - which is what a per-account
	// setting looks like from here.
	for( size_t i = 0; i < vecPet.size(); ++i )
	{
		vecPet[i].m_bAutoFeed = kReq.m_bAutoFeed;
		pDB->SavePet( kSes.m_nSelectedUnitUID, vecPet[i] );
	}

	KEGS_SET_AUTO_FEED_PETS_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_bAutoFeed	= kReq.m_bAutoFeed;

	return Reply( kSes, EGS_SET_AUTO_FEED_PETS_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// riding pets

bool CX2OfflineServer::Handler_EGS_CREATE_RIDING_PET_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CREATE_RIDING_PET_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineDB*			pDB		= CX2OfflineDB::Instance();
	CX2OfflineInventory*	pInven	= CX2OfflineInventory::Instance();
	CX2OfflinePetData*		pData	= CX2OfflinePetData::Instance();

	KEGS_CREATE_RIDING_PET_ACK kAck;
	kAck.m_iOK = NetError::ERR_PET_00;

	KOfflineItemRow kRow;
	if( false == pInven->GetItemRow( kReq.m_iItemUID, kRow ) )
	{
		kAck.m_iOK = NetError::ERR_ITEM_04;
		return Reply( kSes, EGS_CREATE_RIDING_PET_ACK, kAck );
	}

	int iRidingPetID	= 0;
	int iPeriodDays		= -1;

	if( false == pData->GetRidingPetByItem( kRow.m_iItemID, iRidingPetID, iPeriodDays ) ||
		iRidingPetID <= 0 )
	{
		if( false == pData->HasRidingPetRows() )
		{
			CX2OfflineLog::Server( L"PET      mount refused - RidingPetData.lua is not packed into"
				L" data036.kom, so nothing knows what item %d summons (the ticket is left in"
				L" the bag)", kRow.m_iItemID );
		}
		else
		{
			// This is the reachable one. RidingPetData.lua names eleven mounts and
			// this build's CX2RidingPetManager knows four; three of the seven
			// mounts the cash shop sells (the Hamelings, 550040-550042) fall in the
			// gap, and the client's own RIDING_PET_ITEM_ID array offers "use" on
			// them regardless. Refusing keeps the ticket; inventing an ID would
			// burn it on a mount the client cannot draw.
			CX2OfflineLog::Server( L"PET      mount refused - item %d summons a mount this client"
				L" build has no RIDING_PET_UNIT_ID for. The ServerResource .lua files are newer"
				L" than this source tree; the ticket is left in the bag.", kRow.m_iItemID );
		}

		return Reply( kSes, EGS_CREATE_RIDING_PET_ACK, kAck );
	}

	KInventoryItemInfo kSlotInfo;
	if( false == pInven->ConsumeOne( kReq.m_iItemUID, kSlotInfo ) )
	{
		kAck.m_iOK = NetError::ERR_ITEM_04;
		return Reply( kSes, EGS_CREATE_RIDING_PET_ACK, kAck );
	}

	KOfflineRidingPetRow kMount;
	kMount.m_iRidingPetID		= iRidingPetID;
	kMount.m_fStamina			= 100.0f;
	kMount.m_tLastUnSummonDate	= (__int64)::_time64( NULL );

	// unit_riding_pet DOES carry an expiry - 0 means permanent, the same
	// convention KOfflineTitleRow uses - so unlike a pet, a timed mount really
	// does expire. 20 of the 38 stone rows are timed, so this is the common case
	// rather than the exception.
	kMount.m_tDestroyDate = ( iPeriodDays > 0 )
		? ( (__int64)::_time64( NULL ) + ( (__int64)iPeriodDays * 24 * 60 * 60 ) )
		: 0;

	if( false == pDB->InsertRidingPet( kSes.m_nSelectedUnitUID, kMount ) )
	{
		CX2OfflineLog::Server( L"PET      ERROR created mount %d from item %d but the row would"
			L" not save - the ticket has already been consumed", iRidingPetID, kRow.m_iItemID );

		return Reply( kSes, EGS_CREATE_RIDING_PET_ACK, kAck );
	}

	MakeRidingPetInfo( kMount, kAck.m_kCreatedPetInfo );
	kAck.m_vecKInventorySlotInfo.push_back( kSlotInfo );
	kAck.m_iOK = NetError::NET_OK;

	CX2OfflineLog::Server( L"PET      item %d created mount %d (ridingPetUID=%I64d, %s)",
		kRow.m_iItemID, iRidingPetID, (__int64)kMount.m_nRidingPetUID,
		( iPeriodDays > 0 ) ? L"timed" : L"permanent" );

	return Reply( kSes, EGS_CREATE_RIDING_PET_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_SUMMON_RIDING_PET_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_SUMMON_RIDING_PET_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineDB* pDB = CX2OfflineDB::Instance();

	KEGS_SUMMON_RIDING_PET_ACK kAck;
	kAck.m_iOK = NetError::ERR_PET_00;

	std::vector< KOfflineRidingPetRow > vecPet;
	pDB->LoadRidingPets( kSes.m_nSelectedUnitUID, vecPet );

	for( size_t i = 0; i < vecPet.size(); ++i )
	{
		if( (__int64)kReq.m_iRidingPetUID != vecPet[i].m_nRidingPetUID )
			continue;

		MakeRidingPetInfo( vecPet[i], kAck.m_kRidingPetInfo );
		kAck.m_iOK = NetError::NET_OK;
		break;
	}

	// m_bNeedAck false means the client is not waiting - but the reply is sent
	// either way, because the ACK is also what tells it the stamina it should
	// start the ride with.
	return Reply( kSes, EGS_SUMMON_RIDING_PET_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_UNSUMMON_RIDING_PET_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_UNSUMMON_RIDING_PET_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineDB* pDB = CX2OfflineDB::Instance();

	KEGS_UNSUMMON_RIDING_PET_ACK kAck;
	kAck.m_iOK				= NetError::ERR_PET_00;
	kAck.m_iRidingPetUID	= kReq.m_iRidingPetUID;

	std::vector< KOfflineRidingPetRow > vecPet;
	pDB->LoadRidingPets( kSes.m_nSelectedUnitUID, vecPet );

	for( size_t i = 0; i < vecPet.size(); ++i )
	{
		if( (__int64)kReq.m_iRidingPetUID != vecPet[i].m_nRidingPetUID )
			continue;

		// The client is trusted for the stamina it spent, exactly as it is
		// trusted for combat: the ride happens entirely on this machine.
		vecPet[i].m_fStamina			= kReq.m_fStamina;
		vecPet[i].m_tLastUnSummonDate	= (__int64)::_time64( NULL );

		pDB->SaveRidingPet( kSes.m_nSelectedUnitUID, vecPet[i] );

		kAck.m_iOK = NetError::NET_OK;
		break;
	}

	return Reply( kSes, EGS_UNSUMMON_RIDING_PET_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_RELEASE_RIDING_PET_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_RELEASE_RIDING_PET_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_RELEASE_RIDING_PET_ACK kAck;
	kAck.m_iRidingPetUID	= kReq.m_iRidingPetUID;
	kAck.m_iOK				= true == CX2OfflineDB::Instance()->DeleteRidingPet(
									kSes.m_nSelectedUnitUID, (__int64)kReq.m_iRidingPetUID )
								? NetError::NET_OK
								: NetError::ERR_PET_00;

	return Reply( kSes, EGS_RELEASE_RIDING_PET_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// the rest of what a village menu can reach
//
// None of these belongs to a system phase 7 set out to build. They are here
// for one reason: each is a button somewhere in the village UI whose request
// arms an AddServerPacket wait, so leaving it unhandled is a five-second freeze
// and a timeout box - which is exactly what the phase's exit test looks for.
//
// They were found by re-running the phase-5 checklist (every SendPacket whose
// next few lines arm a wait) against the dispatch table, and taking the ones
// that need no subsystem the offline server does not have.
//
// Seven more that the checklist named are NOT here, because their packet
// structs do not exist in a US_SERVICE build at all - the spirit reward ladder,
// the jumping-character event, the .kom integrity report, both halves of the
// returning-player retention offer, and the two recruit-a-friend packets. The
// client cannot send what it cannot construct, so a handler for one would be
// dead code that does not compile.

bool CX2OfflineServer::Handler_EGS_CHECK_CHANNEL_CHANGE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CHECK_CHANNEL_CHANGE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// There is one channel offline - Handler_ECH_GET_CHANNEL_LIST_REQ has said
	// so since phase 1 - so a channel change has nowhere to go. Refusing here
	// stops the flow before EGS_CONNECT_CHANNEL_CHANGE_REQ, which would tear
	// the game-server session down and rebuild it.
	CX2OfflineLog::Server( L"CHANNEL  change to channel %d refused - there is only one channel offline",
		kReq.m_iChannelID );

	KEGS_CHECK_CHANNEL_CHANGE_ACK kAck;
	kAck.m_iOK = NetError::ERR_CHANNEL_00;

	return Reply( kSes, EGS_CHECK_CHANNEL_CHANGE_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_JOIN_FIELD_CHANNEL_CHANGE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_JOIN_FIELD_CHANNEL_CHANGE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// The second half of a channel change: re-enter the village on the new
	// channel. Unreachable while the check above refuses, and answered anyway
	// so a client that gets here another way is not left in limbo.
	KEGS_JOIN_FIELD_CHANNEL_CHANGE_ACK kAck;
	kAck.m_iOK = NetError::ERR_CHANNEL_00;

	return Reply( kSes, EGS_JOIN_FIELD_CHANNEL_CHANGE_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_WARP_BY_BUTTON_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_WARP_BY_BUTTON_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Cobo Express: press B, pick a village from CX2StateMenu::CreateWarpDest's
	// list, OK. This was refused unconditionally on the theory that "which warp
	// index leads to which map, and what it costs" was a server-only data table -
	// wrong on both counts. CXSLMapData::CheckCOBOExpressTicketMapID /
	// ComputeCOBOExpressTicketCost (KncWX2Server/Common/X2Data/XSLMapData.cpp:
	// 606-613) are plain Lua FUNCTIONS, not rows, and the file that defines them
	// (KncWX2Server/ServerResource/US/MapData.lua) is already in this tree. See
	// X2OfflineMapData.h for how it is loaded and why AddMapData/AddLocalMapInfo
	// are bound as no-ops.
	const int iDestMapID = (int)kReq.m_iCurrentVillageWarpIndex;

	KEGS_WARP_BY_BUTTON_ACK kAck;
	kAck.m_iOK				= NetError::ERR_UNKNOWN;
	kAck.m_iED				= 0;
	kAck.m_iWarpPointMapID	= 0;

	KOfflineUnitRow kUnit;
	if( 0 == kSes.m_nSelectedUnitUID ||
		false == CX2OfflineDB::Instance()->LoadUnit( kSes.m_nSelectedUnitUID, kUnit ) )
	{
		CX2OfflineLog::Server( L"WARP     button warp %d refused - no character selected", iDestMapID );
		return Reply( kSes, EGS_WARP_BY_BUTTON_ACK, kAck );
	}

	// No CheckEnterTheVillage() here, for the same reason
	// Handlers_Field.cpp's EGS_STATE_CHANGE_FIELD_REQ has none: CreateWarpDest
	// only ever lists a village the player's own progress has already unlocked,
	// so the client is trusted rather than this build re-deriving a level/
	// dungeon-clear gate it has no data for.
	int iCost = 0;

	if( true == kReq.m_bFreeWarp )
	{
		// A "Cobo Express VIP" ticket is active. Trusted the same way - the
		// client only sets m_bFreeWarp from its own m_bWarpVip flag - rather
		// than re-deriving m_trWarpVipEndDate here.
		iCost = 0;
	}
	else if( false == CX2OfflineMapData::Instance()->IsLoaded() )
	{
		CX2OfflineLog::Server( L"WARP     button warp %d refused - MapData.lua not loaded, cost unknown",
			iDestMapID );
		return Reply( kSes, EGS_WARP_BY_BUTTON_ACK, kAck );
	}
	else
	{
		CX2OfflineMapData* pMapData = CX2OfflineMapData::Instance();

		const int iSrcZone	= pMapData->CheckCOBOExpressTicketMapID( kUnit.m_iLastPos );
		const int iDestZone	= pMapData->CheckCOBOExpressTicketMapID( iDestMapID );

		if( kUnit.m_iLastPos == iDestMapID || 0 == iSrcZone || 0 == iDestZone )
		{
			CX2OfflineLog::Server(
				L"WARP     button warp %d refused - not a valid COBO Express pair (from %d)",
				iDestMapID, kUnit.m_iLastPos );
			return Reply( kSes, EGS_WARP_BY_BUTTON_ACK, kAck );
		}

		iCost = pMapData->ComputeCOBOExpressTicketCost( kUnit.m_iLastPos, iDestMapID, kUnit.m_iLevel );

		if( kUnit.m_iED < iCost )
		{
			kAck.m_iOK = NetError::ERR_USE_ITEM_IN_INVENTORY_08;
			CX2OfflineLog::Server( L"WARP     button warp %d refused - %d ED needed, %d held",
				iDestMapID, iCost, kUnit.m_iED );
			return Reply( kSes, EGS_WARP_BY_BUTTON_ACK, kAck );
		}
	}

	kUnit.m_iED -= iCost;
	CX2OfflineDB::Instance()->SaveProgress( kUnit.m_nUnitUID, kUnit.m_iLevel, kUnit.m_iEXP, kUnit.m_iED );

	// Nothing to store for the destination itself: the client answers a
	// successful ACK by sending EGS_STATE_CHANGE_FIELD_REQ (or leaving the
	// square first, which chains into the same thing) with iWarpPointMapID,
	// and that handler already calls SaveLastPosition - see Handlers_Field.cpp.
	kAck.m_iOK				= NetError::NET_OK;
	kAck.m_iED				= kUnit.m_iED;
	kAck.m_iWarpPointMapID	= iDestMapID;

	CX2OfflineLog::Server( L"WARP     unitUID=%I64d button warp %d -> %d, cost=%d, ED now %d",
		(__int64)kUnit.m_nUnitUID, kUnit.m_iLastPos, iDestMapID, iCost, kUnit.m_iED );

	return Reply( kSes, EGS_WARP_BY_BUTTON_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_2013_EVENT_MISSION_COMPLETE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_2013_EVENT_MISSION_COMPLETE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// A timed level-up event from 2013. Its reward table is in the event
	// schedule, which is the same table phase 6 refused to invent for timed
	// quests.
	KEGS_2013_EVENT_MISSION_COMPLETE_ACK kAck;
	kAck.m_iOK				= NetError::ERR_QUEST_01;
	kAck.m_iRewardedLevel	= 0;

	return Reply( kSes, EGS_2013_EVENT_MISSION_COMPLETE_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_REG_SKILL_NOTE_MEMO_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_REG_SKILL_NOTE_MEMO_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Phase 22. The stub this replaces said "the memo IDs come from a server
	// table" - they do not. KGSUser reads the memo ID straight off the item
	// being spent: `const int iMemoID = m_kInventory.GetItemID( m_iItemUID )`
	// (GSUserGameCommon.cpp:7260). There was never a table to be missing.
	//
	// Order matches the live handler exactly: every check first, then the item
	// is consumed, then the page is written. A memo registered against an item
	// that failed to leave the bag would be free.
	CX2OfflineInventory* pInven = CX2OfflineInventory::Instance();
	CX2OfflineSkill*	 pSkill = CX2OfflineSkill::Instance();

	KEGS_REG_SKILL_NOTE_MEMO_ACK kAck;
	kAck.m_iOK					= NetError::ERR_SKILL_NOTE_00;
	kAck.m_cSkillNotePageNum	= kReq.m_cSkillNotePageNum;
	kAck.m_iMemoID				= 0;

	pSkill->Load( kSes.m_nSelectedUnitUID );

	KOfflineItemRow kRow;
	if( false == pInven->GetItemRow( kReq.m_iItemUID, kRow ) )
	{
		CX2OfflineLog::Server( L"SKILL    memo refused - itemUID=%I64d is not in the bag",
			(__int64)kReq.m_iItemUID );

		return Reply( kSes, EGS_REG_SKILL_NOTE_MEMO_ACK, kAck );
	}

	const int iMemoID = kRow.m_iItemID;

	const int iCheck = pSkill->RegisterSkillNoteMemo( (int)kReq.m_cSkillNotePageNum, iMemoID );
	if( NetError::NET_OK != iCheck )
	{
		kAck.m_iOK = iCheck;

		CX2OfflineLog::Server( L"SKILL    memo %d refused for page %d - error %d"
			L" (%d page(s) owned)",
			iMemoID, (int)kReq.m_cSkillNotePageNum, iCheck,
			(int)pSkill->GetSkillNoteMaxPage() );

		return Reply( kSes, EGS_REG_SKILL_NOTE_MEMO_ACK, kAck );
	}

	KInventoryItemInfo kSlotInfo;
	if( false == pInven->ConsumeOne( kReq.m_iItemUID, kSlotInfo ) )
	{
		kAck.m_iOK = NetError::ERR_SKILL_NOTE_03;
		return Reply( kSes, EGS_REG_SKILL_NOTE_MEMO_ACK, kAck );
	}

	kAck.m_iOK		= NetError::NET_OK;
	kAck.m_iMemoID	= iMemoID;
	kAck.m_vecInventorySlotInfo.push_back( kSlotInfo );

	// KUserSkillTree::GetSkillNote - every registered memo, in page order. The
	// client feeds it to SetEqipSkillMemo, which is what makes a memo's passive
	// actually apply.
	{
		const std::map< char, int >& mapNote = pSkill->GetSkillNotes();

		std::map< char, int >::const_iterator mit;
		for( mit = mapNote.begin(); mit != mapNote.end(); ++mit )
			kAck.m_vecSkillNote.push_back( mit->second );
	}

	CX2OfflineLog::Server( L"SKILL    memo %d registered on page %d (%u memo(s) now)",
		iMemoID, (int)kReq.m_cSkillNotePageNum,
		(unsigned int)kAck.m_vecSkillNote.size() );

	return Reply( kSes, EGS_REG_SKILL_NOTE_MEMO_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_OPEN_RANDOM_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_OPEN_RANDOM_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineDB*			pDB		= CX2OfflineDB::Instance();
	CX2OfflineInventory*	pInven	= CX2OfflineInventory::Instance();
	CX2OfflineRandomItem*	pCube	= CX2OfflineRandomItem::Instance();

	KEGS_OPEN_RANDOM_ITEM_ACK kAck;
	kAck.m_iOK = NetError::ERR_RANDOM_ITEM_01;

	KOfflineUnitRow kUnit;
	if( false == pDB->LoadUnit( kSes.m_nSelectedUnitUID, kUnit ) )
		return Reply( kSes, EGS_OPEN_RANDOM_ITEM_ACK, kAck );

	// SERV_CUBE_OPEN_ED_CONDITION is on, and the client does not treat m_iED as
	// a delta - it ASSIGNS it over the character's ED
	// (X2UIInventory.cpp:9121). Every reply out of this handler, refusals
	// included, has to carry the real figure or opening a cube wipes the wallet.
	kAck.m_iED = kUnit.m_iED;

	KOfflineItemRow kRow;
	if( false == pInven->GetItemRow( kReq.m_iItemUID, kRow ) )
	{
		kAck.m_iOK = NetError::ERR_RANDOM_ITEM_00;
		return Reply( kSes, EGS_OPEN_RANDOM_ITEM_ACK, kAck );
	}

	// Which key this cube wants. A cube can be listed several times, once per
	// key it accepts; key 0 is the no-key row. KGSInventory walks the list and
	// takes the first key the player actually holds enough of
	// (Inventory.cpp:8580), and so does this.
	const std::map< int, int >* pmapKey = pCube->GetKeyList( kRow.m_iItemID );

	if( NULL == pmapKey || true == pmapKey->empty() )
	{
		if( false == pCube->IsLoaded() )
		{
			CX2OfflineLog::Server( L"CUBE     refused - RandomItemTable.lua is not packed into"
				L" data036.kom, so nothing knows what is inside item %d (the cube is left in"
				L" the bag)", kRow.m_iItemID );
		}
		else
		{
			CX2OfflineLog::Server( L"CUBE     refused - item %d has no row in RandomItemTable.lua"
				L" (the cube is left in the bag)", kRow.m_iItemID );
		}

		return Reply( kSes, EGS_OPEN_RANDOM_ITEM_ACK, kAck );
	}

	int iKeyItemID		= 0;
	int iKeyCount		= 0;
	bool bHaveKey		= false;

	std::map< int, int >::const_iterator mitKey;
	for( mitKey = pmapKey->begin(); mitKey != pmapKey->end(); ++mitKey )
	{
		if( 0 == mitKey->first )
		{
			// The no-key row. Nothing to hold, nothing to spend.
			bHaveKey = true;
			break;
		}

		if( mitKey->second <= 0 )
			continue;

		if( pInven->CountItemByID( mitKey->first, true ) >= mitKey->second )
		{
			iKeyItemID	= mitKey->first;
			iKeyCount	= mitKey->second;
			bHaveKey	= true;
			break;
		}
	}

	if( false == bHaveKey )
	{
		CX2OfflineLog::Server( L"CUBE     refused - item %d needs a key this character does not"
			L" have", kRow.m_iItemID );

		kAck.m_iOK = NetError::ERR_RANDOM_ITEM_04;
		return Reply( kSes, EGS_OPEN_RANDOM_ITEM_ACK, kAck );
	}

	const CX2OfflineRandomItem::KCube* pData = pCube->GetCube( kRow.m_iItemID, iKeyItemID );
	if( NULL == pData )
	{
		CX2OfflineLog::Server( L"CUBE     refused - item %d has a key list but no row for key %d",
			kRow.m_iItemID, iKeyItemID );

		return Reply( kSes, EGS_OPEN_RANDOM_ITEM_ACK, kAck );
	}

	// Some cubes cost ED to open on top of everything else.
	if( pData->m_iRequiredED > 0 && kUnit.m_iED < pData->m_iRequiredED )
	{
		CX2OfflineLog::Server( L"CUBE     refused - item %d costs %d ED to open, %d held",
			kRow.m_iItemID, pData->m_iRequiredED, kUnit.m_iED );

		kAck.m_iOK = NetError::ERR_RANDOM_ITEM_13;
		return Reply( kSes, EGS_OPEN_RANDOM_ITEM_ACK, kAck );
	}

	// The draw itself, before anything is consumed. A cube whose group is empty
	// or whose odds miss must leave the bag exactly as it found it.
	std::vector< CX2OfflineRandomItem::KResult > vecResult;

	if( false == pCube->GetResult( kUnit.m_iUnitClass, *pData, vecResult ) )
	{
		CX2OfflineLog::Server( L"CUBE     item %d drew nothing for class %d - the cube is left in"
			L" the bag", kRow.m_iItemID, kUnit.m_iUnitClass );

		kAck.m_iOK = NetError::ERR_RANDOM_ITEM_02;
		return Reply( kSes, EGS_OPEN_RANDOM_ITEM_ACK, kAck );
	}

	// Room for everything, checked before anything is destroyed. The live server
	// does this too under SERV_RANDOM_ITEM_CHECK_INVEN, and it matters more for a
	// give-all cube than for a single draw - those hand over a dozen items at
	// once.
	//
	// Approximate in one direction only: two results that stack together are
	// counted as needing two slots rather than one, so this can refuse a cube
	// that would just barely have fit. Refusing a cube that would have fit is
	// recoverable; opening one that does not fit destroys the overflow.
	for( size_t i = 0; i < vecResult.size(); ++i )
	{
		if( false == pInven->HasRoomFor( vecResult[i].m_iItemID, vecResult[i].m_iQuantity ) )
		{
			CX2OfflineLog::Server( L"CUBE     refused - no room for %d x item %d out of item %d"
				L" (the cube is left in the bag)",
				vecResult[i].m_iQuantity, vecResult[i].m_iItemID, kRow.m_iItemID );

			kAck.m_iOK = NetError::ERR_RANDOM_ITEM_07;
			return Reply( kSes, EGS_OPEN_RANDOM_ITEM_ACK, kAck );
		}
	}

	// Phase 20: the stamina cube, checked before anything is destroyed.
	//
	// KEGS_OPEN_RANDOM_ITEM_ACK::m_iRestoreSpirit is **not** the amount to
	// restore, whatever its name suggests. The live server computes
	// `iRestoreSpirit = m_kUserSpiritManager.RestoreSpirit( iRestoreSpirit )`
	// (GSUserInventory.cpp:2099) - that call adds and clamps, and *returns the
	// new absolute spirit* - and the client assigns it straight onto the unit:
	// `m_iSpirit = m_TempOpenRandomItemAck.m_iRestoreSpirit`
	// (X2UIInventory.cpp:9697), printing ( new - old ) / max * 100 as the
	// percentage gained.
	//
	// So passing RandomItemTable.lua's raw `m_iRestoreSpirit = 1440` through was
	// actively destructive: offline pins spirit to SHRT_MAX in FillSpirit, so
	// the potion set the gauge to 1440/32767 and the popup read "-95%". That is
	// ISSUES.md #11, and it was the item 77200 "Stamina Potion".
	//
	// Modelled the way the server does it, then refused when the answer is a
	// no-op. Spirit is pinned full offline (see FillSpirit for why), so there is
	// never anything to restore and the potion is left in the bag with a message
	// rather than eaten for nothing. Written as a real min-with-max rather than
	// an unconditional refusal so that if spirit is ever modelled properly the
	// potion starts working with no further change here.
	if( pData->m_iRestoreSpirit > 0 )
	{
		int iSpirit		= 0;
		int iSpiritMax	= 0;
		FillSpirit( iSpirit, iSpiritMax );

		int iNewSpirit = iSpirit + pData->m_iRestoreSpirit;
		if( iNewSpirit > iSpiritMax )
			iNewSpirit = iSpiritMax;

		if( iNewSpirit <= iSpirit )
		{
			CX2OfflineLog::Server( L"CUBE     refused - item %d restores %d stamina but stamina is"
				L" already %d/%d offline, so it would be spent for nothing"
				L" (the potion is left in the bag)",
				kRow.m_iItemID, pData->m_iRestoreSpirit, iSpirit, iSpiritMax );

			kAck.m_iOK = NetError::ERR_RANDOM_ITEM_05;
			return Reply( kSes, EGS_OPEN_RANDOM_ITEM_ACK, kAck );
		}

		kAck.m_iRestoreSpirit = iNewSpirit;
	}

	//////////////////////////////////////////////////////////////////////////
	// Past here the cube is spent. Consume first, hand over second.

	KInventoryItemInfo kCubeSlot;
	if( false == pInven->ConsumeOne( kReq.m_iItemUID, kCubeSlot ) )
	{
		kAck.m_iOK = NetError::ERR_RANDOM_ITEM_00;
		return Reply( kSes, EGS_OPEN_RANDOM_ITEM_ACK, kAck );
	}

	kAck.m_vecKInventorySlotInfo.push_back( kCubeSlot );

	if( iKeyItemID > 0 && iKeyCount > 0 )
	{
		std::vector< KInventoryItemInfo > vecKeySlot;
		pInven->ConsumeByID( iKeyItemID, iKeyCount, vecKeySlot );

		for( size_t i = 0; i < vecKeySlot.size(); ++i )
			kAck.m_vecKInventorySlotInfo.push_back( vecKeySlot[i] );
	}

	if( pData->m_iRequiredED > 0 )
	{
		kUnit.m_iED -= pData->m_iRequiredED;
		pDB->SaveProgress( kUnit.m_nUnitUID, kUnit.m_iLevel, kUnit.m_iEXP, kUnit.m_iED );

		kAck.m_iED = kUnit.m_iED;
	}

	//////////////////////////////////////////////////////////////////////////
	// Hand over the results.

	int iGiven = 0;

	for( size_t i = 0; i < vecResult.size(); ++i )
	{
		std::vector< KInventoryItemInfo > vecChanged;
		int iInserted = 0;

		pInven->InsertItem( vecResult[i].m_iItemID, vecResult[i].m_iQuantity, 0,
							vecChanged, iInserted );

		for( size_t n = 0; n < vecChanged.size(); ++n )
			kAck.m_vecKInventorySlotInfo.push_back( vecChanged[n] );

		if( iInserted <= 0 )
		{
			// The room check said it would fit. If it did not, the item is gone
			// and the player is owed it - say so loudly rather than let a silent
			// shortfall look like bad luck.
			CX2OfflineLog::Server( L"CUBE     ERROR item %d rolled %d x %d out of item %d and none"
				L" of it fit - the room check disagreed with the insert",
				kRow.m_iItemID, vecResult[i].m_iQuantity, vecResult[i].m_iItemID, kRow.m_iItemID );
			continue;
		}

		// m_mapInsertedItem is item ID -> count, and it is what the client shows
		// in the result popup. Accumulated rather than assigned: a give-all cube
		// can legitimately roll the same item twice.
		kAck.m_mapInsertedItem[ vecResult[i].m_iItemID ] += iInserted;
		++iGiven;

		// Presentation only. Offline there is nobody to broadcast a rare pull to,
		// so the log is the announcement.
		if( true == pCube->IsAnnouncedResult( vecResult[i].m_iItemID ) )
		{
			CX2OfflineLog::Server( L"CUBE     ** rare ** item %d would have been announced"
				L" server-wide on live", vecResult[i].m_iItemID );
		}
	}

	// Carried through untouched, and it is the resurrection stone count only -
	// m_iRestoreSpirit was already computed above, because unlike this one it
	// cannot be a straight copy (see the phase 20 block). Nothing offline
	// consumes the stone count; the client's own popup reads it, and it is what
	// a resurrection cube exists to hand over, so dropping it would empty that
	// cube.
	//
	// Note for whoever meets the resurrection cube: the client treats this field
	// as an absolute total too - `SetResurrectionStoneNum( ack.m_iRessurectionCount )`
	// at X2UIInventory.cpp:9656, with ( new - old ) shown as the gain. Passing
	// the table value straight through is right only while the character holds
	// no stones, which is the offline case today.
	kAck.m_iRessurectionCount	= pData->m_iRessurectionCount;

	kAck.m_iOK = NetError::NET_OK;

	CX2OfflineLog::Server( L"CUBE     opened item %d (%s, key %d): %d item kind(s)%s",
		kRow.m_iItemID,
		true == pData->m_bGiveAll ? L"give-all" : L"single draw",
		iKeyItemID, iGiven,
		true == pCube->IsSealedCube( kRow.m_iItemID ) ? L" [sealed cube]" : L"" );

	return Reply( kSes, EGS_OPEN_RANDOM_ITEM_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// the second-password pad
//
// A per-account PIN that gates the bank and trading on live. Nothing offline
// stores one and nothing offline needs one - the account is a row in a local
// file the player owns. So the pad reports "there is no second password set",
// which is a state the client already handles and which leaves every gated
// feature open.

bool CX2OfflineServer::Handler_EGS_AUTH_SECOND_SECURITY_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_AUTH_SECOND_SECURITY_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// NET_OK: whatever was typed, the pad opens. There is nothing to protect
	// and refusing would lock the player out of their own save.
	KEGS_AUTH_SECOND_SECURITY_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_iFailedCount	= 0;

	return Reply( kSes, EGS_AUTH_SECOND_SECURITY_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_CREATE_SECOND_SECURITY_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CREATE_SECOND_SECURITY_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Refused, not accepted. Accepting would tell the player they have set a
	// PIN that is not stored anywhere - and the next login would silently not
	// ask for it, which is the worst of both.
	CX2OfflineLog::Server( L"SECURITY second-password creation refused - it is not stored offline" );

	KEGS_CREATE_SECOND_SECURITY_ACK kAck;
	kAck.m_iOK = NetError::ERR_SECOND_SECURITY_NO_USE;

	return Reply( kSes, EGS_CREATE_SECOND_SECURITY_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_COMPARE_SECOND_SECURITY_PW_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_COMPARE_SECOND_SECURITY_PW_REQ kReq;		///< a bare std::wstring
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_COMPARE_SECOND_SECURITY_PW_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_COMPARE_SECOND_SECURITY_PW_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_CHANGE_SECOND_SECURITY_PW_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CHANGE_SECOND_SECURITY_PW_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_CHANGE_SECOND_SECURITY_PW_ACK kAck;
	kAck.m_iOK = NetError::ERR_SECOND_SECURITY_NO_USE;

	return Reply( kSes, EGS_CHANGE_SECOND_SECURITY_PW_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_DELETE_SECOND_SECURITY_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_DELETE_SECOND_SECURITY_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Deleting a password that was never set leaves the account exactly as the
	// player asked for it, so this one succeeds.
	KEGS_DELETE_SECOND_SECURITY_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_DELETE_SECOND_SECURITY_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_DELETE_SECOND_SECURITY_VERIFY_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_DELETE_SECOND_SECURITY_VERIFY_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_DELETE_SECOND_SECURITY_VERIFY_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_DELETE_SECOND_SECURITY_VERIFY_ACK, kAck );
}

#endif SERV_IRUHADEV_OFFLINE
