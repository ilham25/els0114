#include "stdafx.h"
#include "X2OfflineServer.h"

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-08-31
// Description: Offline mode - phase 1. The login chain, from channel-server
//              account verification down to the character list.
//
//              Nothing is persisted yet: the account is fabricated and the
//              character list is one hardcoded unit. SQLite arrives in phase 2.
//
//              Where a reply's contents matter, the reference is the client's
//              own Handler_* in X2Lib/X2StateLogin.cpp and
//              X2Lib/X2StateServerSelect.cpp, and the real server's
//              KncWX2Server/GameServer/GSUserSession.cpp.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

namespace
{
	// The offline "shard". None of these are reachable over a network; the
	// client only needs them to be well-formed and non-empty, because it feeds
	// the channel IP/port straight back into Handler_EGS_CONNECT_REQ (which
	// bails out on an empty IP).
	const int				OFFLINE_SERVER_GROUP_ID	= 1;
	const int				OFFLINE_CHANNEL_ID		= 1;
	const int				OFFLINE_SERVER_SET_ID	= 1;
	const wchar_t* const	OFFLINE_IP				= L"127.0.0.1";
	const unsigned short	OFFLINE_PORT			= 9400;
	const int				OFFLINE_UDP_PORT		= 9500;

	const UidType			OFFLINE_USER_UID		= 1;
	const int				OFFLINE_UNIT_SLOTS		= 3;

	// The one phase-1 character.
	const UidType			DUMMY_UNIT_UID			= 1;
	const char				DUMMY_UNIT_CLASS		= (char)CX2Unit::UC_ELSWORD_SWORDMAN;
	const wchar_t* const	DUMMY_UNIT_NICKNAME		= L"OfflineTest";
	const int				DUMMY_UNIT_LEVEL		= 1;
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Dispatch( KOfflineSession& kSes, const KEvent& kEvent )
{
	switch( kEvent.m_usEventID )
	{
	//////////////////////////////////////////////////////////////////////////
	// channel server (PI_CHANNEL_USER)
	case ECH_VERIFY_ACCOUNT_REQ:			return Handler_ECH_VERIFY_ACCOUNT_REQ( kSes, kEvent );
	case ECH_GET_SERVERGROUP_LIST_REQ:		return Handler_ECH_GET_SERVERGROUP_LIST_REQ( kSes, kEvent );
	case ECH_GET_CHANNEL_LIST_REQ:			return Handler_ECH_GET_CHANNEL_LIST_REQ( kSes, kEvent );
	case ECH_DISCONNECT_REQ:				return Handler_ECH_DISCONNECT_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// game server (PI_GS_USER)
	case EGS_CONNECT_REQ:					return Handler_EGS_CONNECT_REQ( kSes, kEvent );
	case EGS_VERIFY_ACCOUNT_REQ:			return Handler_EGS_VERIFY_ACCOUNT_REQ( kSes, kEvent );
	case EGS_CHECK_MACHINE_ID_REQ:			return Handler_EGS_CHECK_MACHINE_ID_REQ( kSes, kEvent );
	case EGS_STATE_CHANGE_SERVER_SELECT_REQ:return Handler_EGS_STATE_CHANGE_SERVER_SELECT_REQ( kSes, kEvent );
	case EGS_CURRENT_TIME_REQ:				return Handler_EGS_CURRENT_TIME_REQ( kSes, kEvent );
	case EGS_SELECT_SERVER_SET_REQ:			return Handler_EGS_SELECT_SERVER_SET_REQ( kSes, kEvent );
	case EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ:
											return Handler_EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ( kSes, kEvent );
	case EGS_KEYBOARD_MAPPING_INFO_WRITE_REQ:
											return Handler_EGS_KEYBOARD_MAPPING_INFO_WRITE_REQ( kSes, kEvent );
	case EGS_DISCONNECT_FOR_SERVER_SELECT_REQ:
											return Handler_EGS_DISCONNECT_FOR_SERVER_SELECT_REQ( kSes, kEvent );

	default:
		break;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// channel server

bool CX2OfflineServer::Handler_ECH_VERIFY_ACCOUNT_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KECH_VERIFY_ACCOUNT_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Accept anything. The password never matters again: from here on the
	// client uses the passport this reply hands it
	// (CX2StateLogin::Handler_ECH_VERIFY_ACCOUNT_ACK -> SetUserPassword).
	std::wstring wstrID = kReq.m_wstrID;
	if( true == wstrID.empty() )
		wstrID = L"offline";

	m_nUserUID		= OFFLINE_USER_UID;
	m_wstrLoginID	= wstrID;

	kSes.m_nUserUID		= OFFLINE_USER_UID;
	kSes.m_wstrLoginID	= wstrID;
	kSes.m_wstrPassport	= L"OFFLINE_PASSPORT";
	kSes.m_eState		= S_UID_UPDATED;

	CX2OfflineLog::Server( L"LOGIN    accepted id='%s' -> userUID=%d", wstrID.c_str(), (int)OFFLINE_USER_UID );

	KECH_VERIFY_ACCOUNT_ACK kAck;
	kAck.m_iOK				= NetError::NET_OK;
	kAck.m_iUserUID			= OFFLINE_USER_UID;
	kAck.m_wstrPassport		= kSes.m_wstrPassport;		///< must be non-empty or the client stays on the login screen
	kAck.m_iChannelingCode	= -1;						///< -1 keeps the client out of the channeling/publisher branch
	kAck.m_wstrCurrentTime	= NowString();				///< fed to KGCMassFileManager::SetServerCurrentTime, must parse
	kAck.m_wstrUserID		= wstrID;
	kAck.m_wstrPurchaseTok	= L"";
	kAck.m_strAgreementURL	= "";

	return Reply( kSes, ECH_VERIFY_ACCOUNT_ACK, kAck );
}

bool CX2OfflineServer::Handler_ECH_GET_SERVERGROUP_LIST_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	// The _NOT carries the data, the _ACK just closes the client's
	// AddServerPacket() wait. Order matters.
	KServerGroupInfo kGroup;
	kGroup.m_iOrder					= 0;
	kGroup.m_iServerGroupUID		= OFFLINE_SERVER_GROUP_ID;
	kGroup.m_wstrServerGroupName	= L"Offline";
	kGroup.m_kServerIP				= OFFLINE_IP;
	kGroup.m_usMasterPort			= OFFLINE_PORT;
	kGroup.m_usNCUDPPort			= (u_short)OFFLINE_UDP_PORT;
	kGroup.m_bEnable				= true;

	KECH_GET_SERVERGROUP_LIST_NOT kNot;
	kNot.m_mapServerGroupList.insert( std::make_pair( OFFLINE_SERVER_GROUP_ID, kGroup ) );

	Reply( kSes, ECH_GET_SERVERGROUP_LIST_NOT, kNot );

	KECH_GET_SERVERGROUP_LIST_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, ECH_GET_SERVERGROUP_LIST_ACK, kAck );
}

bool CX2OfflineServer::Handler_ECH_GET_CHANNEL_LIST_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	KChannelInfo kChannel;
	kChannel.m_iServerUID			= OFFLINE_CHANNEL_ID;
	kChannel.m_iChannelID			= OFFLINE_CHANNEL_ID;
	kChannel.m_wstrChannelName		= L"Offline-1";
	kChannel.m_iServerGroupID		= OFFLINE_SERVER_GROUP_ID;
	kChannel.m_wstrIP				= OFFLINE_IP;		///< never dialled, but must be non-empty
	kChannel.m_usMasterPort			= OFFLINE_PORT;
	kChannel.m_usNCUDPPort			= (u_short)OFFLINE_UDP_PORT;
	kChannel.m_iMaxUser				= 100;
	kChannel.m_iCurrentUser			= 1;
	kChannel.m_iCurPartyCount		= 0;
	kChannel.m_iPlayGamePartyCount	= 0;

	KECH_GET_CHANNEL_LIST_NOT kNot;
	kNot.m_iServerGroupID = OFFLINE_SERVER_GROUP_ID;
	kNot.m_mapChannelList.insert( std::make_pair( OFFLINE_CHANNEL_ID, kChannel ) );
	// no channel bonus offline

	Reply( kSes, ECH_GET_CHANNEL_LIST_NOT, kNot );

	KECH_GET_CHANNEL_LIST_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, ECH_GET_CHANNEL_LIST_ACK, kAck );
}

bool CX2OfflineServer::Handler_ECH_DISCONNECT_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	// The client drops the channel socket once it is on the game server
	// (CX2StateServerSelect::Handler_ECH_DISCONNECT_ACK). ECH_DISCONNECT_ACK
	// has no payload struct - its client handler reads nothing.
	return ReplyID( kSes, ECH_DISCONNECT_ACK );
}

//////////////////////////////////////////////////////////////////////////
// game server

bool CX2OfflineServer::Handler_EGS_CONNECT_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CONNECT_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Version is deliberately ignored - there is nothing to be compatible with.
	CX2OfflineLog::Server( L"CONNECT  game server, client version '%s' (ignored)", kReq.m_wstrVersion.c_str() );

	KEGS_CONNECT_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_UDPPort		= OFFLINE_UDP_PORT;
	kAck.m_iChannelID	= OFFLINE_CHANNEL_ID;

	return Reply( kSes, EGS_CONNECT_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_VERIFY_ACCOUNT_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_VERIFY_ACCOUNT_REQ kReq;			///< typedef of KUserAuthenticateReq
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	std::wstring wstrID = kReq.m_wstrUserID;
	if( true == wstrID.empty() )
		wstrID = m_wstrLoginID;
	if( true == wstrID.empty() )
		wstrID = L"offline";

	kSes.m_nUserUID		= OFFLINE_USER_UID;
	kSes.m_wstrLoginID	= wstrID;

	KEGS_VERIFY_ACCOUNT_ACK kAck;
	kAck.m_iOK											= NetError::NET_OK;

	kAck.m_kAccountInfo.m_nUserUID						= OFFLINE_USER_UID;
	kAck.m_kAccountInfo.m_wstrID						= wstrID;
	kAck.m_kAccountInfo.m_wstrName						= wstrID;
	kAck.m_kAccountInfo.m_iAuthLevel					= CX2User::XUAL_NORMAL_USER;
	kAck.m_kAccountInfo.m_bInternalUser					= false;
	kAck.m_kAccountInfo.m_kAccountOption.m_bPlayGuide	= false;
	kAck.m_kAccountInfo.m_kAccountBlockInfo.m_wstrEndTime		= L"";	///< not blocked
	kAck.m_kAccountInfo.m_kAccountBlockInfo.m_wstrBlockReason	= L"";
	kAck.m_kAccountInfo.m_bIsRecommend					= false;
	kAck.m_kAccountInfo.m_bIsGuestUser					= false;
	kAck.m_kAccountInfo.m_wstrOTP						= L"";	///< empty leaves the passport in place
	kAck.m_kAccountInfo.m_wstrRegDate					= NowString();
	kAck.m_kAccountInfo.m_wstrLastLogin					= NowString();

	kAck.m_cHackingUserType		= (char)CX2User::HUT_NORMAL;
	kAck.m_wstrAdURL			= L"";
	kAck.m_bSex					= true;
	kAck.m_uiAge				= 20;
	kAck.m_uiNexonSN			= 0;
	kAck.m_wstrClientIP			= OFFLINE_IP;
	kAck.m_uChannelCode			= 0;
	kAck.m_wstrChannelUserID	= L"";

	Reply( kSes, EGS_VERIFY_ACCOUNT_ACK, kAck );

	// ENX_USER_LOGIN_NOT is what actually moves the client to "logged in"
	// (it sets CX2StateServerSelect::m_bENX_USER_LOGIN_NOT, and without it the
	// port-check / server-select chain never starts). See KncRobot.cpp:350.
	KENX_USER_LOGIN_NOT kNot;
	kNot.m_bIsGameBang = false;
	// m_kPcBangReward keeps its own constructor's "no PC-bang" defaults

	Reply( kSes, ENX_USER_LOGIN_NOT, kNot );

	kSes.m_eState = S_LOGINED;

	return true;
}

bool CX2OfflineServer::Handler_EGS_CHECK_MACHINE_ID_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CHECK_MACHINE_ID_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// The ACK MUST echo the machine ID back unchanged: CX2State::Handler_EGS_
	// CHECK_MACHINE_ID_ACK compares it against g_pMain->GetMachineId() and, on
	// a mismatch, reports the player as a hacker (EGS_REPORT_HACK_USER_NOT).
	KEGS_CHECK_MACHINE_ID_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_strMachineID	= kReq.m_strMachineID;

	return Reply( kSes, EGS_CHECK_MACHINE_ID_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_STATE_CHANGE_SERVER_SELECT_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	kSes.m_eState = S_SERVER_SELECT;

	KEGS_STATE_CHANGE_SERVER_SELECT_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_STATE_CHANGE_SERVER_SELECT_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_CURRENT_TIME_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	KEGS_CURRENT_TIME_ACK kAck;
	kAck.m_wstrCurrentTime	= NowString();
	kAck.m_tCurrentTime		= (__int64)::_time64( NULL );

	return Reply( kSes, EGS_CURRENT_TIME_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_SELECT_SERVER_SET_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_SELECT_SERVER_SET_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_SELECT_SERVER_SET_ACK kAck;
	kAck.m_iOK		= NetError::NET_OK;
	kAck.m_wstrIP	= OFFLINE_IP;
	kAck.m_usPort	= OFFLINE_PORT;
	kAck.m_kAccountBlockInfo.m_wstrEndTime		= L"";	///< not blocked
	kAck.m_kAccountBlockInfo.m_wstrBlockReason	= L"";

	return Reply( kSes, EGS_SELECT_SERVER_SET_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	// THE character list. Phase 1 returns one hardcoded unit; phase 2 replaces
	// this body with a SQLite query and nothing above it changes.
	KEGS_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_nUnitSlot	= OFFLINE_UNIT_SLOTS;
	kAck.m_bSharingBank	= false;

	KUnitInfo kUnit;
	MakeDefaultUnitInfo( kUnit, OFFLINE_USER_UID, DUMMY_UNIT_UID,
		DUMMY_UNIT_CLASS, DUMMY_UNIT_NICKNAME, DUMMY_UNIT_LEVEL );

	kAck.m_vecUnitInfo.push_back( kUnit );

	Reply( kSes, EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK, kAck );

	CX2OfflineLog::Server( L"UNITLIST 1 unit, %d slots ('%s', class=%d, lv=%d)",
		OFFLINE_UNIT_SLOTS, DUMMY_UNIT_NICKNAME, (int)DUMMY_UNIT_CLASS, DUMMY_UNIT_LEVEL );

	// Server-push defaults the client would otherwise never receive. Both
	// handlers fall back to sane defaults on an empty collection
	// (CX2StateServerSelect::Handler_EGS_KEYBOARD_MAPPING_INFO_NOT calls
	// CKTDIManager::SetDefaultMap()).
	KEGS_KEYBOARD_MAPPING_INFO_NOT kKeyNot;
	kKeyNot.m_iOK = NetError::NET_OK;
	Reply( kSes, EGS_KEYBOARD_MAPPING_INFO_NOT, kKeyNot );

	KEGS_CHAT_OPTION_INFO_NOT kChatNot;
	kChatNot.m_iOK = NetError::NET_OK;
	Reply( kSes, EGS_CHAT_OPTION_INFO_NOT, kChatNot );

	return true;
}

bool CX2OfflineServer::Handler_EGS_KEYBOARD_MAPPING_INFO_WRITE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	// A direct consequence of the empty EGS_KEYBOARD_MAPPING_INFO_NOT above:
	// the client falls back to CKTDIManager::SetDefaultMap() and immediately
	// writes that default map back, exactly as it would against a real server
	// on a brand-new account. It waits on the ACK
	// (CX2KeyPad::Handler_EGS_KEYBOARD_MAPPING_INFO_WRITE_REQ ->
	// AddServerPacket), so it has to be answered or the wait times out into
	// XGM_SERVER_PACKET_TIMEOUT.
	//
	// Phase 1 acknowledges and discards. Persisting the mapping is phase 5.
	KEGS_KEYBOARD_MAPPING_INFO_WRITE_REQ kReq;			///< typedef of KKeyboardMappingInfo
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineLog::Server( L"KEYMAP   %u entries written (accepted, not persisted until phase 5)",
		(unsigned int)kReq.m_mapKeyboardMappingInfo.size() );

	KEGS_KEYBOARD_MAPPING_INFO_WRITE_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_KEYBOARD_MAPPING_INFO_WRITE_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_DISCONNECT_FOR_SERVER_SELECT_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	// "Go to Channel Selection" on the character-select screen
	// (CX2StateServerSelect::UnitSelectExit -> Handler_EGS_DISCONNECT_FOR_SERVER_
	// SELECT_REQ). Answering is not optional: the ACK is the only thing that
	// clears m_bWaiting_EGS_DISCONNECT_FOR_SERVER_SELECT_ACK, and while that flag
	// is set the SSSUCM_SERVER_CONNECT0 case returns early - so leaving this
	// unhandled leaves the player on the channel list with every channel button
	// inert, unable to get back in.
	//
	// The ACK carries no payload; its client handler reads none. Handling it also
	// drops the game-server proxy (DisconnectFromGameServer), which reaches us
	// again as OnSessionClose.
	kSes.m_eState = S_INIT;

	CX2OfflineLog::Server( L"BACKOUT  returning to channel selection, game session 0x%08X will drop",
		(unsigned int)kSes.m_pSession );

	return ReplyID( kSes, EGS_DISCONNECT_FOR_SERVER_SELECT_ACK );
}

#endif SERV_IRUHADEV_OFFLINE
