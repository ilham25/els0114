#include "stdafx.h"
#include "X2OfflineServer.h"

#ifdef SERV_IRUHADEV_OFFLINE

CX2OfflineServer* CX2OfflineServer::ms_pInstance = NULL;

//////////////////////////////////////////////////////////////////////////

CX2OfflineServer::CX2OfflineServer()
: m_nUserUID( 0 )
, m_iUnitSlots( CX2OfflineDB::DEFAULT_UNIT_SLOTS )
, m_wstrLoginID( L"" )
, m_nNextRoomUID( 1000 )
, m_bQuitRequested( false )
{
}

CX2OfflineServer::~CX2OfflineServer()
{
}

CX2OfflineServer* CX2OfflineServer::Instance()
{
	if( NULL == ms_pInstance )
	{
		CX2OfflineLog::Open();

		ms_pInstance = new CX2OfflineServer;
		g_pX2OfflineHook = ms_pInstance;

		CX2OfflineLog::Server( L"---- offline server up (SERV_IRUHADEV_OFFLINE, phase 4) ----" );

		// els_db.sql sits next to the two logs, in the process working
		// directory - which is the game data\ folder (X2Main mounts the .kom
		// archives through a "./" prefix, so it can be nothing else).
		CX2OfflineDB::Instance()->Open( L"els_db.sql" );

		// CX2OfflineStatTable is NOT loaded here. It reads StatTable.lua through
		// the client's mass-file loader, and the .kom archives are not
		// necessarily mounted at this point - so it loads itself on first use.
	}

	return ms_pInstance;
}

void CX2OfflineServer::Release()
{
	if( NULL == ms_pInstance )
		return;

	CX2OfflineLog::Server( L"---- offline server down ----" );

	g_pX2OfflineHook = NULL;
	delete ms_pInstance;
	ms_pInstance = NULL;

	CX2OfflineCashShop::Release();
	CX2OfflineBattleField::Release();
	CX2OfflineDropTable::Release();
	CX2OfflineResolveTable::Release();
	CX2OfflineEnchantTable::Release();
	CX2OfflineAttribTable::Release();
	CX2OfflineSocketTable::Release();
	CX2OfflineStatTable::Release();
	CX2OfflineDB::Release();
	CX2OfflineLog::Close();
}

/*static*/ const wchar_t* CX2OfflineServer::KindStr( PROXY_KIND eKind )
{
	switch( eKind )
	{
	case PK_GAME:		return L"GS";
	case PK_RELAY:		return L"TR";
	case PK_CHANNEL:	return L"CH";
	case PK_COLLECT:	return L"CS";
	default:			return L"??";
	}
}

/*static*/ CX2OfflineServer::PROXY_KIND CX2OfflineServer::KindFromEventID( unsigned short usEventID )
{
	// KActorProxy::GetClassID() looks like the obvious way to tell the channel
	// proxy from the game proxy, and it is NOT usable from X2Lib:
	//
	//   X2ServerProtocol/StdAfx.h defines ADD_COLLECT_CLIENT_INFO_PROTOCOL
	//   before it includes X2ServerProtocolLib.h, and that macro adds a member
	//   (m_pSADatabase) to KSession. X2Lib reaches the same header through
	//   X2Main.h, where Session.h is included BEFORE X2ServerProtocol.h defines
	//   the macro - so X2Lib's KSession is 4 bytes smaller, and every member of
	//   the derived KActorProxy/KUserProxy reads 4 bytes low from X2Lib.
	//   Observed directly: GetClassID() returned -1, which is the value of
	//   m_nAckOK, the int declared immediately before m_iClassID.
	//
	// Classify from the event ID's own namespace instead - data both projects
	// agree on, since the IDs come from the shared EventID_Client.h.
	const wchar_t* szName = CX2OfflineLog::EventName( usEventID );
	if( NULL == szName )
		return PK_UNKNOWN;

	if( 0 == wcsncmp( szName, L"ECH_", 4 ) )	return PK_CHANNEL;
	if( 0 == wcsncmp( szName, L"ETR_", 4 ) )	return PK_RELAY;
	if( 0 == wcsncmp( szName, L"ECS_", 4 ) )	return PK_COLLECT;

	return PK_GAME;		///< EGS_*, ENX_*, and the E_* system events
}

/*static*/ std::wstring CX2OfflineServer::NowString()
{
	SYSTEMTIME st;
	::GetLocalTime( &st );

	wchar_t szBuff[32];
	_snwprintf( szBuff, 32, L"%04d-%02d-%02d %02d:%02d:%02d",
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond );
	szBuff[31] = L'\0';

	return std::wstring( szBuff );
}

//////////////////////////////////////////////////////////////////////////
// IX2OfflineHook

void CX2OfflineServer::OnSessionConnect( KSession* pSession, const char* szIP, unsigned short usPort )
{
	if( NULL == pSession )
		return;

	// The kind is left PK_UNKNOWN here and pinned by the session's first packet
	// (see OnClientSend / KindFromEventID) - the proxy object itself cannot be
	// asked from X2Lib.
	{
		KLocker lock( m_cs );

		KOfflineSession kSes;
		kSes.m_pSession	= pSession;
		kSes.m_eKind	= PK_UNKNOWN;
		kSes.m_eState	= S_INIT;

		m_mapSession[ pSession ] = kSes;
	}

	std::wstring wstrIP;
	ConvertCharToWCHAR( wstrIP, ( NULL != szIP ) ? szIP : "" );

	CX2OfflineLog::Server( L"CONNECT  session=0x%08X  (would have been %s:%u)",
		(unsigned int)pSession, wstrIP.c_str(), (unsigned int)usPort );
}

void CX2OfflineServer::OnSessionClose( KSession* pSession )
{
	if( NULL == pSession )
		return;

	PROXY_KIND eKind = PK_UNKNOWN;
	bool bFound = false;

	{
		KLocker lock( m_cs );

		std::map< KSession*, KOfflineSession >::iterator mit = m_mapSession.find( pSession );
		if( mit != m_mapSession.end() )
		{
			eKind  = mit->second.m_eKind;
			bFound = true;
			m_mapSession.erase( mit );
		}
	}

	if( true == bFound )
	{
		CX2OfflineLog::Server( L"CLOSE    %s  session=0x%08X",
			KindStr( eKind ), (unsigned int)pSession );
	}
}

bool CX2OfflineServer::OnClientSend( KSession* pSession, const KEvent& kEvent )
{
	if( NULL == pSession )
		return true;

	// A session the offline server never saw connect - nothing sensible to do
	// with the packet, but it must not fall through to the dead socket path.
	KOfflineSession kSes;
	{
		KLocker lock( m_cs );

		std::map< KSession*, KOfflineSession >::iterator mit = m_mapSession.find( pSession );
		if( mit == m_mapSession.end() )
		{
			CX2OfflineLog::Packet( true, L"??", kEvent.m_usEventID,
				kEvent.m_kbuff.GetLength(), L"*** UNKNOWN SESSION - DROPPED ***" );
			return true;
		}

		kSes = mit->second;
	}

	// system-level chatter the emulator has no use for; consume it silently
	switch( kEvent.m_usEventID )
	{
	case E_HEART_BEAT:
	case E_CHECK_SEQUENCE_COUNT_NOT:
		return true;
	default:
		break;
	}

	// Pin which server this session is standing in for, from its first packet.
	if( PK_UNKNOWN == kSes.m_eKind )
	{
		kSes.m_eKind = KindFromEventID( kEvent.m_usEventID );

		CX2OfflineLog::Server( L"SESSION  0x%08X is the %s proxy",
			(unsigned int)pSession, KindStr( kSes.m_eKind ) );
	}

	// One packet at a time, whichever session thread it arrived on - see
	// m_csDispatch. Held across the log's defer buffer and the transaction as
	// well as the dispatch, because all three are process-wide state.
	KLocker lockDispatch( m_csDispatch );

	// The handler's replies are logged from inside Reply(); buffer them so this
	// request's line lands above them in the file (see CX2OfflineLog::DeferEnd).
	CX2OfflineLog::DeferBegin();

	bool			bHandled		= false;
	unsigned long	dwExceptionCode	= 0;

	{
		// One packet is one unit of work: a handler that faults half way
		// through a multi-row change leaves the save file as it was. Committed
		// only on the way out of this scope, and only if nothing faulted.
		KOfflineDBTxn txn;

		bHandled = DispatchProtected( kSes, kEvent, dwExceptionCode );

		if( 0 == dwExceptionCode )
			txn.Commit();
	}

	const wchar_t* szNote = L"*** UNHANDLED ***";

	if( 0 != dwExceptionCode )
	{
		// Deliberately not logged as HANDLED, which is what the old catch(...)
		// did: a fault that reads as a successful packet is a fault nobody
		// finds. The packet is still consumed - see the return below - because
		// letting it fall through to the dead socket path would hang the client
		// on top of whatever already went wrong.
		CX2OfflineLog::Server( L"EXCEPTION in the handler for %s (id=%u), code 0x%08X%s - rolled back and consumed",
			CX2OfflineLog::EventName( kEvent.m_usEventID ), (unsigned int)kEvent.m_usEventID,
			(unsigned int)dwExceptionCode,
			( 0xE06D7363 == dwExceptionCode ) ? L" (a C++ throw)" : L"" );

		szNote = L"*** EXCEPTION - ROLLED BACK ***";
	}
	else if( true == bHandled )
	{
		szNote = L"HANDLED";
	}
	else
	{
		// Nothing claimed it. Say whether that was expected, and count it
		// either way for the census on the way out (X2OfflineIgnore).
		const wchar_t* szReason = CX2OfflineIgnore::Reason( kEvent.m_usEventID );

		CX2OfflineIgnore::Note( kEvent.m_usEventID, szReason );

		if( NULL != szReason )
			szNote = L"--- IGNORED ---";
	}

	CX2OfflineLog::DeferEnd( true, KindStr( kSes.m_eKind ), kEvent.m_usEventID,
		kEvent.m_kbuff.GetLength(), szNote );

	// Write the (possibly advanced) session state back.
	{
		KLocker lock( m_cs );

		std::map< KSession*, KOfflineSession >::iterator mit = m_mapSession.find( pSession );
		if( mit != m_mapSession.end() )
			mit->second = kSes;
	}

	// After the transaction has committed, never inside it: a WAL checkpoint is
	// a no-op while a savepoint is open.
	if( true == m_bQuitRequested )
	{
		m_bQuitRequested = false;
		OnCleanShutdown();
	}

	// Always consume: an unhandled packet must never reach the dead socket path.
	return true;
}

bool CX2OfflineServer::DispatchProtected( KOfflineSession& kSes, const KEvent& kEvent,
										  OUT unsigned long& dwExceptionCode )
{
	// Nothing with a destructor in this function - see the header.
	dwExceptionCode = 0;

	__try
	{
		return Dispatch( kSes, kEvent );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		dwExceptionCode = (unsigned long)::GetExceptionCode();

		// Zero would read as "no fault" to the caller, and a filter can in
		// principle see code 0.
		if( 0 == dwExceptionCode )
			dwExceptionCode = 0xFFFFFFFF;

		return false;
	}
}

void CX2OfflineServer::OnCleanShutdown()
{
	// What the census is for is the next run, not this one: it names every
	// event id the dispatch declined, so the ignore list can be extended
	// deliberately rather than by whoever next reads the packet log.
	CX2OfflineIgnore::LogCensus();

	CX2OfflineDB* pDB = CX2OfflineDB::Instance();

	if( false == pDB->IsOpen() )
		return;

	// Fold els_db.sql-wal back into els_db.sql. Without this the -wal file is
	// left holding committed transactions, and anyone who backs up the save by
	// copying els_db.sql alone silently loses them.
	const bool bCheckpoint = pDB->Checkpoint();

	// A second copy, through the online-backup API. The point of it is a save
	// that survives the *next* run: everything in this project is one editable
	// SQLite file, and one bad migration or one mis-aimed UPDATE would
	// otherwise be the end of a character.
	const bool bBackup = pDB->Backup( L"els_db.sql.bak" );

	CX2OfflineLog::Server( L"SHUTDOWN clean: wal checkpoint %s, els_db.sql.bak %s",
		( true == bCheckpoint ) ? L"ok" : L"FAILED",
		( true == bBackup )     ? L"written" : L"FAILED" );
}

/*static*/ void CX2OfflineServer::FillSpirit( OUT int& iSpirit, OUT int& iSpiritMax )
{
	// Not MAX_SPIRIT, and deliberately not named as though it were: a sentinel
	// whose only job is to make the ratio 1.0. SHRT_MAX because dbo.GSpirit
	// stores Spirit as `smallint`, so it is the largest spirit the shipped
	// schema can hold - which means it cannot be smaller than any dungeon's
	// DungeonData::m_RequireSpirit that the real system could ever have set,
	// and the entry check therefore always passes. Reading the column type is
	// what makes this a bound taken from the repo rather than a number picked
	// to look plausible.
	const int SPIRIT_FULL = SHRT_MAX;

	iSpirit		= SPIRIT_FULL;
	iSpiritMax	= SPIRIT_FULL;
}

bool CX2OfflineServer::ReplyID( KOfflineSession& kSes, unsigned short usEventID )
{
	if( NULL == kSes.m_pSession )
		return false;

	UidType anTrace[2] = { -1, -1 };

	KEventPtr spEvent( new KEvent );
	spEvent->SetData( PI_GS_PROXY, anTrace, usEventID );

	CX2OfflineLog::Packet( false, KindStr( kSes.m_eKind ), usEventID, 0, L"" );

	kSes.m_pSession->QueueingEvent( spEvent );
	return true;
}

//////////////////////////////////////////////////////////////////////////

/*static*/ void CX2OfflineServer::MakeDefaultUnitInfo( KUnitInfo& kOut,
													   UidType nOwnerUserUID,
													   UidType nUnitUID,
													   char cUnitClass,
													   const std::wstring& wstrNickName,
													   int iLevel )
{
	// KUnitInfo::Init() zeroes most of the ~70 fields already; only the ones it
	// leaves alone, plus the ones that actually mean something, are set here.
	kOut.Init();

	kOut.m_iOwnerUserUID	= nOwnerUserUID;
	kOut.m_nUnitUID			= nUnitUID;
	kOut.m_cUnitClass		= cUnitClass;
	kOut.m_wstrNickName		= wstrNickName;
	kOut.m_ucLevel			= (UCHAR)iLevel;
	kOut.m_cAuthLevel		= (char)CX2User::XUAL_NORMAL_USER;

	//{{ Author: Iruha
	// Date: 2026-09-05
	// Description: Offline mode - phase 16. This build has SERV_PVP_NEW_SYSTEM
	// and SERV_2012_PVP_SEASON2 both active (KncWX2Server/Common/ServerDefine.h,
	// KTDXLIB/Always.h's unconditional PVP_SEASON2), so KUnitInfo's PvP field is
	// m_cRank, not the old m_iPVPEmblem/m_iVSPoint pair - those aren't members
	// of this build's KUnitInfo at all. Init() leaves m_cRank at 0
	// (CX2PVPEmblem::PVPRANK_NONE), which PVPEmblem_Season2.lua never registers
	// with CX2PVPEmblem::AddEmblemData_LUA (its lowest key is
	// PVPRANK_RANK_ARRANGE = 1, the "still being placed" rank a real server
	// gives a character with no season games yet) - so GetPVPEmblemData()
	// returns NULL and the character-select screen draws whatever the picture
	// control defaults to instead of an emblem. PVPRANK_RANK_ARRANGE is the
	// structural "no rank yet" the real client actually has a texture for.
	kOut.m_cRank			= (char)CX2PVPEmblem::PVPRANK_RANK_ARRANGE;
	//}}

	// not covered by Init()
	kOut.m_iOfficialMatchCnt	= 0;
	kOut.m_iMaxRating			= 0;
	kOut.m_bIsWinBeforeMatch	= false;
	kOut.m_iPastSeasonWin		= 0;

	// Reuse the client's own EXP table rather than duplicating it.
	if( NULL != g_pData && NULL != g_pData->GetEXPTable() )
	{
		kOut.m_iEXP					= g_pData->GetEXPTable()->GetEXPData( iLevel ).m_nTotalExp;
		kOut.m_nNowBaseLevelEXP		= g_pData->GetEXPTable()->GetEXPData( iLevel ).m_nTotalExp;
		kOut.m_nNextBaseLevelEXP	= g_pData->GetEXPTable()->GetEXPData( iLevel + 1 ).m_nTotalExp;
	}

	kOut.m_iED			= 0;
	FillSpirit( kOut.m_iSpirit, kOut.m_iSpiritMax );

	kOut.m_bDeleted		= false;
	kOut.m_wstrLastDate	= NowString();

	// Base stats. Not optional and not cosmetic: CX2Unit::ResetUnitData feeds
	// m_kStat / m_kGameStat into UnitData::m_Stat / m_GameStat, and
	// CX2GUUser::InitStat then takes m_GameStat when its base HP is above zero
	// and m_Stat otherwise - so leaving both zeroed gives the village unit a max
	// HP of 0.
	//
	// m_kStat is the character's own stat; m_kGameStat is that plus its gear,
	// which is what the real server sends and what CX2GUUser::InitStat prefers
	// when its base HP is above zero. Phase 3 and 4 made the two equal because
	// there was no gear; since phase 5 there is, so the second one is built
	// through MakeGameStat.
	CX2OfflineStatTable::Instance()->GetUnitStat( (int)cUnitClass, iLevel, kOut.m_kStat );

	// The gear half is left to MakeUnitInfoFromRow, which knows the unit UID
	// and can load that unit's inventory first. Callers that only have a class
	// and a level - the create-character reply - have no gear to add anyway.
	kOut.m_kGameStat = kOut.m_kStat;
}

/*static*/ void CX2OfflineServer::MakeGameStat( const KOfflineUnitRow& kRow, OUT KStat& kOut )
{
	CX2OfflineStatTable::Instance()->GetUnitStat( kRow.m_iUnitClass, kRow.m_iLevel, kOut );

	// Equipped gear on top, from *this* character's inventory. The Load is what
	// makes that true: every caller here hands in a row, and the row says which
	// character it is, so the stat cannot silently pick up the gear of whoever
	// happened to be loaded last.
	CX2OfflineInventory* pInven = CX2OfflineInventory::Instance();

	if( true == pInven->Load( kRow.m_nUnitUID ) )
		pInven->AddEquippedStat( kOut );
}


//////////////////////////////////////////////////////////////////////////
// Dispatch lives here rather than in a Handlers_*.cpp so that each of those
// files stays a flat list of handlers as the phases add more of them.

bool CX2OfflineServer::Dispatch( KOfflineSession& kSes, const KEvent& kEvent )
{
	switch( kEvent.m_usEventID )
	{
	//////////////////////////////////////////////////////////////////////////
	// channel server (PI_CHANNEL_USER) - Handlers_Login.cpp
	case ECH_VERIFY_ACCOUNT_REQ:			return Handler_ECH_VERIFY_ACCOUNT_REQ( kSes, kEvent );
	case ECH_GET_SERVERGROUP_LIST_REQ:		return Handler_ECH_GET_SERVERGROUP_LIST_REQ( kSes, kEvent );
	case ECH_GET_CHANNEL_LIST_REQ:			return Handler_ECH_GET_CHANNEL_LIST_REQ( kSes, kEvent );
	case ECH_DISCONNECT_REQ:				return Handler_ECH_DISCONNECT_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// game server (PI_GS_USER), login chain - Handlers_Login.cpp
	case EGS_CONNECT_REQ:					return Handler_EGS_CONNECT_REQ( kSes, kEvent );
	case EGS_VERIFY_ACCOUNT_REQ:			return Handler_EGS_VERIFY_ACCOUNT_REQ( kSes, kEvent );
	case EGS_CHECK_MACHINE_ID_REQ:			return Handler_EGS_CHECK_MACHINE_ID_REQ( kSes, kEvent );
	case EGS_STATE_CHANGE_SERVER_SELECT_REQ:return Handler_EGS_STATE_CHANGE_SERVER_SELECT_REQ( kSes, kEvent );
	case EGS_CURRENT_TIME_REQ:				return Handler_EGS_CURRENT_TIME_REQ( kSes, kEvent );
	case EGS_SELECT_SERVER_SET_REQ:			return Handler_EGS_SELECT_SERVER_SET_REQ( kSes, kEvent );
	case EGS_CHECK_BALANCE_REQ:				return Handler_EGS_CHECK_BALANCE_REQ( kSes, kEvent );
	case EGS_KEYBOARD_MAPPING_INFO_WRITE_REQ:
											return Handler_EGS_KEYBOARD_MAPPING_INFO_WRITE_REQ( kSes, kEvent );
	case EGS_DISCONNECT_FOR_SERVER_SELECT_REQ:
											return Handler_EGS_DISCONNECT_FOR_SERVER_SELECT_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// character CRUD - Handlers_Unit.cpp
	case EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ:
											return Handler_EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ( kSes, kEvent );
	case EGS_CREATE_UNIT_REQ:				return Handler_EGS_CREATE_UNIT_REQ( kSes, kEvent );
	case EGS_DELETE_UNIT_REQ:				return Handler_EGS_DELETE_UNIT_REQ( kSes, kEvent );
	case EGS_FINAL_DELETE_UNIT_REQ:			return Handler_EGS_FINAL_DELETE_UNIT_REQ( kSes, kEvent );
	case EGS_RESTORE_UNIT_REQ:				return Handler_EGS_RESTORE_UNIT_REQ( kSes, kEvent );
	case EGS_SELECT_UNIT_REQ:				return Handler_EGS_SELECT_UNIT_REQ( kSes, kEvent );
	case EGS_GET_MY_INVENTORY_REQ:			return Handler_EGS_GET_MY_INVENTORY_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// village / field entry - Handlers_Field.cpp
	case EGS_STATE_CHANGE_FIELD_REQ:		return Handler_EGS_STATE_CHANGE_FIELD_REQ( kSes, kEvent );
	case EGS_FIELD_LOADING_COMPLETE_REQ:	return Handler_EGS_FIELD_LOADING_COMPLETE_REQ( kSes, kEvent );
	case EGS_OPTION_UPDATE_REQ:				return Handler_EGS_OPTION_UPDATE_REQ( kSes, kEvent );
	case EGS_UPDATE_PLAY_STATUS_NOT:		return Handler_EGS_UPDATE_PLAY_STATUS_NOT( kSes, kEvent );
	case EGS_FIELD_UNIT_SYNC_DATA_NOT:		return Handler_EGS_FIELD_UNIT_SYNC_DATA_NOT( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// Cobo Express, the trade square - Handlers_Square.cpp
	case EGS_SQUARE_LIST_REQ:				return Handler_EGS_SQUARE_LIST_REQ( kSes, kEvent );
	case EGS_JOIN_SQUARE_REQ:				return Handler_EGS_JOIN_SQUARE_REQ( kSes, kEvent );
	case EGS_LEAVE_SQUARE_REQ:				return Handler_EGS_LEAVE_SQUARE_REQ( kSes, kEvent );
	case EGS_SQUARE_UNIT_SYNC_DATA_REQ:	return Handler_EGS_SQUARE_UNIT_SYNC_DATA_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// rooms, the dungeon run inside one, and its result - Handlers_Room.cpp
	case EGS_CREATE_TUTORIAL_ROOM_REQ:		return Handler_EGS_CREATE_TUTORIAL_ROOM_REQ( kSes, kEvent );
	case EGS_CREATE_ROOM_REQ:				return Handler_EGS_CREATE_ROOM_REQ( kSes, kEvent );
	case EGS_QUICK_START_DUNGEON_GAME_REQ:	return Handler_EGS_QUICK_START_DUNGEON_GAME_REQ( kSes, kEvent );
	case EGS_JOIN_BATTLE_FIELD_REQ:			return Handler_EGS_JOIN_BATTLE_FIELD_REQ( kSes, kEvent );
	case EGS_ROOM_LIST_REQ:					return Handler_EGS_ROOM_LIST_REQ( kSes, kEvent );
	case EGS_LEAVE_ROOM_REQ:				return Handler_EGS_LEAVE_ROOM_REQ( kSes, kEvent );

	case EGS_CHANGE_DUNGEON_DIFFICULTY_REQ:	return Handler_EGS_CHANGE_DUNGEON_DIFFICULTY_REQ( kSes, kEvent );
	case EGS_CHANGE_MOTION_REQ:				return Handler_EGS_CHANGE_MOTION_REQ( kSes, kEvent );

	case EGS_STATE_CHANGE_GAME_START_REQ:	return Handler_EGS_STATE_CHANGE_GAME_START_REQ( kSes, kEvent );
	case EGS_GAME_LOADING_REQ:				return Handler_EGS_GAME_LOADING_REQ( kSes, kEvent );
	case EGS_MY_USER_UNIT_INFO_TO_SERVER_REQ:
											return Handler_EGS_MY_USER_UNIT_INFO_TO_SERVER_REQ( kSes, kEvent );
	case EGS_BATTLE_FIELD_NPC_LOAD_COMPLETE_REQ:
											return Handler_EGS_BATTLE_FIELD_NPC_LOAD_COMPLETE_REQ( kSes, kEvent );

	case EGS_DUNGEON_STAGE_LOAD_REQ:		return Handler_EGS_DUNGEON_STAGE_LOAD_REQ( kSes, kEvent );
	case EGS_DUNGEON_STAGE_LOAD_COMPLETE_REQ:
											return Handler_EGS_DUNGEON_STAGE_LOAD_COMPLETE_REQ( kSes, kEvent );
	case EGS_DUNGEON_SUB_STAGE_OPEN_REQ:	return Handler_EGS_DUNGEON_SUB_STAGE_OPEN_REQ( kSes, kEvent );
	case EGS_DUNGEON_SUB_STAGE_GO_NEXT_REQ:	return Handler_EGS_DUNGEON_SUB_STAGE_GO_NEXT_REQ( kSes, kEvent );
	case EGS_DUNGEON_SUB_STAGE_LOAD_COMPLETE_REQ:
											return Handler_EGS_DUNGEON_SUB_STAGE_LOAD_COMPLETE_REQ( kSes, kEvent );
	case EGS_DUNGEON_SUB_STAGE_CLEAR_REQ:	return Handler_EGS_DUNGEON_SUB_STAGE_CLEAR_REQ( kSes, kEvent );
	case EGS_TALK_WITH_NPC_REQ:			return Handler_EGS_TALK_WITH_NPC_REQ( kSes, kEvent );
	case EGS_DUNGEON_KILLALLNPC_CHECK_REQ:	return Handler_EGS_DUNGEON_KILLALLNPC_CHECK_REQ( kSes, kEvent );

	case EGS_NPC_UNIT_CREATE_REQ:			return Handler_EGS_NPC_UNIT_CREATE_REQ( kSes, kEvent );
	case EGS_NPC_UNIT_DIE_REQ:				return Handler_EGS_NPC_UNIT_DIE_REQ( kSes, kEvent );
	case EGS_GET_ITEM_REQ:					return Handler_EGS_GET_ITEM_REQ( kSes, kEvent );
	case EGS_WORLD_TRIGGER_RELOCATION_REQ:	return Handler_EGS_WORLD_TRIGGER_RELOCATION_REQ( kSes, kEvent );

	case EGS_END_GAME_REQ:					return Handler_EGS_END_GAME_REQ( kSes, kEvent );
	case EGS_STATE_CHANGE_RESULT_REQ:		return Handler_EGS_STATE_CHANGE_RESULT_REQ( kSes, kEvent );
	case EGS_RESULT_SUCCESS_REQ:			return Handler_EGS_RESULT_SUCCESS_REQ( kSes, kEvent );
	case EGS_LEAVE_GAME_REQ:				return Handler_EGS_LEAVE_GAME_REQ( kSes, kEvent );

	case EGS_USER_UNIT_DIE_REQ:				return Handler_EGS_USER_UNIT_DIE_REQ( kSes, kEvent );
	case EGS_USER_UNIT_DIE_COMPLETE_REQ:	return Handler_EGS_USER_UNIT_DIE_COMPLETE_REQ( kSes, kEvent );
	case EGS_RESURRECT_TO_CONTINUE_DUNGEON_REQ:
											return Handler_EGS_RESURRECT_TO_CONTINUE_DUNGEON_REQ( kSes, kEvent );
	case EGS_STOP_DUNGEON_CONTINUE_TIME_REQ:
											return Handler_EGS_STOP_DUNGEON_CONTINUE_TIME_REQ( kSes, kEvent );
	case EGS_DUNGEON_SECRET_STAGE_ENTER_CHECK_REQ:
											return Handler_EGS_DUNGEON_SECRET_STAGE_ENTER_CHECK_REQ( kSes, kEvent );
	case EGS_START_REWARD_BOX_SELECT_REQ:	return Handler_EGS_START_REWARD_BOX_SELECT_REQ( kSes, kEvent );
	case EGS_SELECT_REWARD_BOX_REQ:			return Handler_EGS_SELECT_REWARD_BOX_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// inventory, equipment and the ED shop - Handlers_Inventory.cpp
	case EGS_CHANGE_INVENTORY_SLOT_ITEM_REQ:
											return Handler_EGS_CHANGE_INVENTORY_SLOT_ITEM_REQ( kSes, kEvent );
	case EGS_DELETE_ITEM_REQ:				return Handler_EGS_DELETE_ITEM_REQ( kSes, kEvent );
	case EGS_SORT_CATEGORY_ITEM_REQ:		return Handler_EGS_SORT_CATEGORY_ITEM_REQ( kSes, kEvent );
	case EGS_USE_ITEM_IN_INVENTORY_REQ:		return Handler_EGS_USE_ITEM_IN_INVENTORY_REQ( kSes, kEvent );
	case EGS_USE_QUICK_SLOT_REQ:			return Handler_EGS_USE_QUICK_SLOT_REQ( kSes, kEvent );
	case EGS_BUY_ED_ITEM_REQ:				return Handler_EGS_BUY_ED_ITEM_REQ( kSes, kEvent );
	case EGS_SELL_ED_ITEM_REQ:				return Handler_EGS_SELL_ED_ITEM_REQ( kSes, kEvent );
	case EGS_REPAIR_ITEM_REQ:				return Handler_EGS_REPAIR_ITEM_REQ( kSes, kEvent );
	case EGS_ENCHANT_ITEM_REQ:				return Handler_EGS_ENCHANT_ITEM_REQ( kSes, kEvent );
	case EGS_SOCKET_ITEM_REQ:				return Handler_EGS_SOCKET_ITEM_REQ( kSes, kEvent );
	case EGS_ENCHANT_ATTACH_ITEM_REQ:		return Handler_EGS_ENCHANT_ATTACH_ITEM_REQ( kSes, kEvent );
	case EGS_ATTRIB_ENCHANT_ITEM_REQ:		return Handler_EGS_ATTRIB_ENCHANT_ITEM_REQ( kSes, kEvent );
	case EGS_ATTRIB_ATTACH_ITEM_REQ:		return Handler_EGS_ATTRIB_ATTACH_ITEM_REQ( kSes, kEvent );
	case EGS_RESOLVE_ITEM_REQ:				return Handler_EGS_RESOLVE_ITEM_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// quests, and the titles their missions award - Handlers_Quest.cpp
	case EGS_NEW_QUEST_REQ:					return Handler_EGS_NEW_QUEST_REQ( kSes, kEvent );
	case EGS_UPDATE_QUEST_REQ:				return Handler_EGS_UPDATE_QUEST_REQ( kSes, kEvent );
	case EGS_QUEST_COMPLETE_REQ:			return Handler_EGS_QUEST_COMPLETE_REQ( kSes, kEvent );
	case EGS_ALL_COMPLETED_QUEST_COMPLETE_REQ:
											return Handler_EGS_ALL_COMPLETED_QUEST_COMPLETE_REQ( kSes, kEvent );
	case EGS_GIVE_UP_QUEST_REQ:				return Handler_EGS_GIVE_UP_QUEST_REQ( kSes, kEvent );
	case EGS_GATHER_GIVE_UP_QUEST_REQ:		return Handler_EGS_GATHER_GIVE_UP_QUEST_REQ( kSes, kEvent );
	case EGS_EQUIP_TITLE_REQ:				return Handler_EGS_EQUIP_TITLE_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// the skill tree - Handlers_Skill.cpp
	case EGS_GET_SKILL_REQ:					return Handler_EGS_GET_SKILL_REQ( kSes, kEvent );
	case EGS_RESET_SKILL_REQ:				return Handler_EGS_RESET_SKILL_REQ( kSes, kEvent );
	case EGS_INIT_SKILL_TREE_REQ:			return Handler_EGS_INIT_SKILL_TREE_REQ( kSes, kEvent );
	case EGS_CHANGE_SKILL_SLOT_REQ:			return Handler_EGS_CHANGE_SKILL_SLOT_REQ( kSes, kEvent );
	case EGS_SKILL_USE_REQ:					return Handler_EGS_SKILL_USE_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// the cash shop - Handlers_Shop.cpp
	case EGS_BILL_PRODUCT_INFO_REQ:         return Handler_EGS_BILL_PRODUCT_INFO_REQ( kSes, kEvent );
	case EGS_GET_WISH_LIST_REQ:             return Handler_EGS_GET_WISH_LIST_REQ( kSes, kEvent );
	case EGS_MODIFY_WISH_LIST_REQ:          return Handler_EGS_MODIFY_WISH_LIST_REQ( kSes, kEvent );
	case EGS_BILL_INVENTORY_INQUIRY_REQ:    return Handler_EGS_BILL_INVENTORY_INQUIRY_REQ( kSes, kEvent );
	case EGS_BUY_CASH_ITEM_REQ:             return Handler_EGS_BUY_CASH_ITEM_REQ( kSes, kEvent );
	case EGS_BILL_GET_PURCHASED_CASH_ITEM_REQ:
                                        return Handler_EGS_BILL_GET_PURCHASED_CASH_ITEM_REQ( kSes, kEvent );
	case EGS_GET_PURCHASED_PACKAGE_CASH_ITEM_REQ:
                                        return Handler_EGS_GET_PURCHASED_PACKAGE_CASH_ITEM_REQ( kSes, kEvent );
	case EGS_PRESENT_CASH_ITEM_REQ:         return Handler_EGS_PRESENT_CASH_ITEM_REQ( kSes, kEvent );
	case EGS_VISIT_CASH_SHOP_NOT:           return Handler_EGS_VISIT_CASH_SHOP_NOT( kSes, kEvent );
	case EGS_CHECK_PRESENT_CASH_INVENTORY_NOT:
                                        return Handler_EGS_CHECK_PRESENT_CASH_INVENTORY_NOT( kSes, kEvent );
	case EGS_APPLY_COUPON_REQ:              return Handler_EGS_APPLY_COUPON_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// guild - Handlers_Social.cpp
	case EGS_CREATE_GUILD_REQ:              return Handler_EGS_CREATE_GUILD_REQ( kSes, kEvent );
	case EGS_GET_GUILD_USER_LIST_REQ:       return Handler_EGS_GET_GUILD_USER_LIST_REQ( kSes, kEvent );
	case EGS_GET_APPLY_JOIN_GUILD_LIST_REQ: return Handler_EGS_GET_APPLY_JOIN_GUILD_LIST_REQ( kSes, kEvent );
	case EGS_GET_GUILD_AD_LIST_REQ:         return Handler_EGS_GET_GUILD_AD_LIST_REQ( kSes, kEvent );
	case EGS_APPLY_JOIN_GUILD_REQ:          return Handler_EGS_APPLY_JOIN_GUILD_REQ( kSes, kEvent );
	case EGS_ACCEPT_JOIN_GUILD_REQ:         return Handler_EGS_ACCEPT_JOIN_GUILD_REQ( kSes, kEvent );
	case EGS_DELETE_APPLY_JOIN_GUILD_REQ:   return Handler_EGS_DELETE_APPLY_JOIN_GUILD_REQ( kSes, kEvent );
	case EGS_REGISTRATION_GUILD_AD_REQ:     return Handler_EGS_REGISTRATION_GUILD_AD_REQ( kSes, kEvent );
	case EGS_MODIFY_REG_GUILD_AD_REQ:       return Handler_EGS_MODIFY_REG_GUILD_AD_REQ( kSes, kEvent );
	case EGS_INVITE_GUILD_REQ:              return Handler_EGS_INVITE_GUILD_REQ( kSes, kEvent );
	case EGS_INVITE_GUILD_REPLY_REQ:        return Handler_EGS_INVITE_GUILD_REPLY_REQ( kSes, kEvent );
	case EGS_KICK_GUILD_MEMBER_REQ:         return Handler_EGS_KICK_GUILD_MEMBER_REQ( kSes, kEvent );
	case EGS_CHANGE_GUILD_MEMBER_GRADE_REQ: return Handler_EGS_CHANGE_GUILD_MEMBER_GRADE_REQ( kSes, kEvent );
	case EGS_CHANGE_GUILD_MEMBER_MESSAGE_REQ:
                                        return Handler_EGS_CHANGE_GUILD_MEMBER_MESSAGE_REQ( kSes, kEvent );
	case EGS_CHANGE_GUILD_MESSAGE_REQ:      return Handler_EGS_CHANGE_GUILD_MESSAGE_REQ( kSes, kEvent );
	case EGS_CHANGE_GUILD_NAME_CHECK_REQ:   return Handler_EGS_CHANGE_GUILD_NAME_CHECK_REQ( kSes, kEvent );
	case EGS_CHANGE_GUILD_NAME_REQ:         return Handler_EGS_CHANGE_GUILD_NAME_REQ( kSes, kEvent );
	case EGS_GET_GUILD_SKILL_REQ:           return Handler_EGS_GET_GUILD_SKILL_REQ( kSes, kEvent );
	case EGS_GET_GUILD_SKILL_IN_BOARD_REQ:  return Handler_EGS_GET_GUILD_SKILL_IN_BOARD_REQ( kSes, kEvent );
	case EGS_INIT_GUILD_SKILL_REQ:          return Handler_EGS_INIT_GUILD_SKILL_REQ( kSes, kEvent );
	case EGS_RESET_GUILD_SKILL_REQ:         return Handler_EGS_RESET_GUILD_SKILL_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// party - Handlers_Social.cpp
	case EGS_INVITE_PARTY_REQ:              return Handler_EGS_INVITE_PARTY_REQ( kSes, kEvent );
	case EGS_LEAVE_PARTY_REQ:               return Handler_EGS_LEAVE_PARTY_REQ( kSes, kEvent );
	case EGS_CHANGE_PARTY_TYPE_REQ:         return Handler_EGS_CHANGE_PARTY_TYPE_REQ( kSes, kEvent );
	case EGS_PARTY_CHANGE_DUNGEON_REQ:      return Handler_EGS_PARTY_CHANGE_DUNGEON_REQ( kSes, kEvent );
	case EGS_PARTY_CHANGE_HOST_REQ:         return Handler_EGS_PARTY_CHANGE_HOST_REQ( kSes, kEvent );
	case EGS_PARTY_CHANGE_GET_ITEM_TYPE_REQ:
                                        return Handler_EGS_PARTY_CHANGE_GET_ITEM_TYPE_REQ( kSes, kEvent );
	case EGS_PARTY_CHANGE_NUM_OF_PER_REQ:   return Handler_EGS_PARTY_CHANGE_NUM_OF_PER_REQ( kSes, kEvent );
	case EGS_PARTY_CHANGE_CHECK_MIN_LEVEL_REQ:
                                        return Handler_EGS_PARTY_CHANGE_CHECK_MIN_LEVEL_REQ( kSes, kEvent );
	case EGS_PARTY_BAN_USER_REQ:            return Handler_EGS_PARTY_BAN_USER_REQ( kSes, kEvent );
	case EGS_PARTY_GAME_START_REQ:          return Handler_EGS_PARTY_GAME_START_REQ( kSes, kEvent );
	case EGS_REQUEST_MATCH_MAKING_REQ:      return Handler_EGS_REQUEST_MATCH_MAKING_REQ( kSes, kEvent );
	case EGS_AUTO_PARTY_DUNGEON_GAME_REQ:   return Handler_EGS_AUTO_PARTY_DUNGEON_GAME_REQ( kSes, kEvent );
	case EGS_PVP_PARTY_CHANGE_MATCH_INFO_REQ:
                                        return Handler_EGS_PVP_PARTY_CHANGE_MATCH_INFO_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// friends, the messenger and the community window - Handlers_Social.cpp
	case EGS_COMMUNITY_USER_LIST_REQ:       return Handler_EGS_COMMUNITY_USER_LIST_REQ( kSes, kEvent );
	case EGS_COMMUNITY_USER_LIST_MONITORING_MODE_REQ:
                                        return Handler_EGS_COMMUNITY_USER_LIST_MONITORING_MODE_REQ( kSes, kEvent );
	case EGS_GET_RECOMMEND_USER_LIST_REQ:   return Handler_EGS_GET_RECOMMEND_USER_LIST_REQ( kSes, kEvent );
	case EGS_REQUEST_FRIEND_REQ:            return Handler_EGS_REQUEST_FRIEND_REQ( kSes, kEvent );
	case EGS_ACCEPT_FRIEND_REQ:             return Handler_EGS_ACCEPT_FRIEND_REQ( kSes, kEvent );
	case EGS_DENY_FRIEND_REQ:               return Handler_EGS_DENY_FRIEND_REQ( kSes, kEvent );
	case EGS_BLOCK_FRIEND_REQ:              return Handler_EGS_BLOCK_FRIEND_REQ( kSes, kEvent );
	case EGS_UNBLOCK_FRIEND_REQ:            return Handler_EGS_UNBLOCK_FRIEND_REQ( kSes, kEvent );
	case EGS_DELETE_FRIEND_REQ:             return Handler_EGS_DELETE_FRIEND_REQ( kSes, kEvent );
	case EGS_MOVE_FRIEND_REQ:               return Handler_EGS_MOVE_FRIEND_REQ( kSes, kEvent );
	case EGS_MAKE_FRIEND_GROUP_REQ:         return Handler_EGS_MAKE_FRIEND_GROUP_REQ( kSes, kEvent );
	case EGS_RENAME_FRIEND_GROUP_REQ:       return Handler_EGS_RENAME_FRIEND_GROUP_REQ( kSes, kEvent );
	case EGS_DELETE_FRIEND_GROUP_REQ:       return Handler_EGS_DELETE_FRIEND_GROUP_REQ( kSes, kEvent );
	case EGS_UPDATE_FRIEND_INFO_REQ:        return Handler_EGS_UPDATE_FRIEND_INFO_REQ( kSes, kEvent );
	case EGS_KNM_REQUEST_NEW_FRIEND_INFO_REQ:
                                        return Handler_EGS_KNM_REQUEST_NEW_FRIEND_INFO_REQ( kSes, kEvent );
	case EGS_GET_MY_MESSENGER_SN_REQ:       return Handler_EGS_GET_MY_MESSENGER_SN_REQ( kSes, kEvent );
	case EGS_ED_MONITORING_BLOCK_REQ:       return Handler_EGS_ED_MONITORING_BLOCK_REQ( kSes, kEvent );
	case EGS_USER_COMMUNITY_SURVEY_NEW_REQ: return Handler_EGS_USER_COMMUNITY_SURVEY_NEW_REQ( kSes, kEvent );
	case EGS_INVITE_PVP_ROOM_REQ:           return Handler_EGS_INVITE_PVP_ROOM_REQ( kSes, kEvent );
	case EGS_UPDATE_COMMUNITY_OPTION_REQ:   return Handler_EGS_UPDATE_COMMUNITY_OPTION_REQ( kSes, kEvent );
	case EGS_SEARCH_UNIT_REQ:               return Handler_EGS_SEARCH_UNIT_REQ( kSes, kEvent );
	case EGS_GET_CONNECTION_UNIT_INFO_REQ:  return Handler_EGS_GET_CONNECTION_UNIT_INFO_REQ( kSes, kEvent );
	case EGS_WATCH_UNIT_REQ:                return Handler_EGS_WATCH_UNIT_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// mail - Handlers_Social.cpp
	case EGS_GET_POST_LETTER_LIST_REQ:      return Handler_EGS_GET_POST_LETTER_LIST_REQ( kSes, kEvent );
	case EGS_READ_LETTER_REQ:               return Handler_EGS_READ_LETTER_REQ( kSes, kEvent );
	case EGS_DELETE_LETTER_REQ:             return Handler_EGS_DELETE_LETTER_REQ( kSes, kEvent );
	case EGS_SEND_LETTER_REQ:               return Handler_EGS_SEND_LETTER_REQ( kSes, kEvent );
	case EGS_GET_ITEM_FROM_LETTER_REQ:      return Handler_EGS_GET_ITEM_FROM_LETTER_REQ( kSes, kEvent );
	case EGS_GET_POST_BLACK_LIST_REQ:       return Handler_EGS_GET_POST_BLACK_LIST_REQ( kSes, kEvent );
	case EGS_NEW_POST_BLACK_LIST_REQ:       return Handler_EGS_NEW_POST_BLACK_LIST_REQ( kSes, kEvent );
	case EGS_DEL_POST_BLACK_LIST_REQ:       return Handler_EGS_DEL_POST_BLACK_LIST_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// chat - Handlers_Social.cpp
	case EGS_CHAT_REQ:                      return Handler_EGS_CHAT_REQ( kSes, kEvent );
	case EGS_CHAT_OPTION_INFO_WRITE_REQ:    return Handler_EGS_CHAT_OPTION_INFO_WRITE_REQ( kSes, kEvent );
	case EGS_NEW_BLACKLIST_USER_REQ:        return Handler_EGS_NEW_BLACKLIST_USER_REQ( kSes, kEvent );
	case EGS_DEL_BLACKLIST_USER_REQ:        return Handler_EGS_DEL_BLACKLIST_USER_REQ( kSes, kEvent );
	case EGS_USE_MEGAPHONE_REQ:             return Handler_EGS_USE_MEGAPHONE_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// ranking and the local-ranking profile - Handlers_Social.cpp
	case EGS_GET_RANKING_INFO_REQ:          return Handler_EGS_GET_RANKING_INFO_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// the tutor system, the bank, and the rest of the menu - Handlers_Social.cpp
	case EGS_TUTORIAL_STUDENT_LIST_REQ:     return Handler_EGS_TUTORIAL_STUDENT_LIST_REQ( kSes, kEvent );
	case EGS_REQUEST_TUTORIAL_REQ:          return Handler_EGS_REQUEST_TUTORIAL_REQ( kSes, kEvent );
	case EGS_DEL_TUTORIAL_REQ:              return Handler_EGS_DEL_TUTORIAL_REQ( kSes, kEvent );
	case EGS_GET_SHARE_BANK_REQ:            return Handler_EGS_GET_SHARE_BANK_REQ( kSes, kEvent );
	case EGS_GET_TEMP_ITEM_REQ:             return Handler_EGS_GET_TEMP_ITEM_REQ( kSes, kEvent );
	case EGS_DELETE_TEMP_ITEM_REQ:          return Handler_EGS_DELETE_TEMP_ITEM_REQ( kSes, kEvent );
	case EGS_SEARCH_TRADE_BOARD_REQ:        return Handler_EGS_SEARCH_TRADE_BOARD_REQ( kSes, kEvent );
	case EGS_CHECK_SUM_REQ:                 return Handler_EGS_CHECK_SUM_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// pets and riding pets - Handlers_Social.cpp
	case EGS_GET_PET_LIST_REQ:               return Handler_EGS_GET_PET_LIST_REQ( kSes, kEvent );
	case EGS_GET_RIDING_PET_LIST_REQ:        return Handler_EGS_GET_RIDING_PET_LIST_REQ( kSes, kEvent );
	case EGS_CREATE_PET_REQ:                return Handler_EGS_CREATE_PET_REQ( kSes, kEvent );
	case EGS_SUMMON_PET_REQ:                return Handler_EGS_SUMMON_PET_REQ( kSes, kEvent );
	case EGS_FEED_PETS_REQ:                 return Handler_EGS_FEED_PETS_REQ( kSes, kEvent );
	case EGS_COMMANDS_FOR_PETS_REQ:         return Handler_EGS_COMMANDS_FOR_PETS_REQ( kSes, kEvent );
	case EGS_PET_EVOLUTION_REQ:             return Handler_EGS_PET_EVOLUTION_REQ( kSes, kEvent );
	case EGS_CHANGE_PET_NAME_REQ:           return Handler_EGS_CHANGE_PET_NAME_REQ( kSes, kEvent );
	case EGS_SET_AUTO_FEED_PETS_REQ:        return Handler_EGS_SET_AUTO_FEED_PETS_REQ( kSes, kEvent );
	case EGS_CREATE_RIDING_PET_REQ:         return Handler_EGS_CREATE_RIDING_PET_REQ( kSes, kEvent );
	case EGS_SUMMON_RIDING_PET_REQ:         return Handler_EGS_SUMMON_RIDING_PET_REQ( kSes, kEvent );
	case EGS_UNSUMMON_RIDING_PET_REQ:       return Handler_EGS_UNSUMMON_RIDING_PET_REQ( kSes, kEvent );
	case EGS_RELEASE_RIDING_PET_REQ:        return Handler_EGS_RELEASE_RIDING_PET_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// the rest of what a village menu can reach - Handlers_Social.cpp
	case EGS_CHECK_CHANNEL_CHANGE_REQ:      return Handler_EGS_CHECK_CHANNEL_CHANGE_REQ( kSes, kEvent );
	case EGS_JOIN_FIELD_CHANNEL_CHANGE_REQ: return Handler_EGS_JOIN_FIELD_CHANNEL_CHANGE_REQ( kSes, kEvent );
	case EGS_WARP_BY_BUTTON_REQ:            return Handler_EGS_WARP_BY_BUTTON_REQ( kSes, kEvent );
	case EGS_2013_EVENT_MISSION_COMPLETE_REQ:
                                        return Handler_EGS_2013_EVENT_MISSION_COMPLETE_REQ( kSes, kEvent );
	case EGS_REG_SKILL_NOTE_MEMO_REQ:       return Handler_EGS_REG_SKILL_NOTE_MEMO_REQ( kSes, kEvent );
	case EGS_OPEN_RANDOM_ITEM_REQ:          return Handler_EGS_OPEN_RANDOM_ITEM_REQ( kSes, kEvent );
	case EGS_AUTH_SECOND_SECURITY_REQ:      return Handler_EGS_AUTH_SECOND_SECURITY_REQ( kSes, kEvent );
	case EGS_CREATE_SECOND_SECURITY_REQ:    return Handler_EGS_CREATE_SECOND_SECURITY_REQ( kSes, kEvent );
	case EGS_COMPARE_SECOND_SECURITY_PW_REQ:
                                        return Handler_EGS_COMPARE_SECOND_SECURITY_PW_REQ( kSes, kEvent );
	case EGS_CHANGE_SECOND_SECURITY_PW_REQ: return Handler_EGS_CHANGE_SECOND_SECURITY_PW_REQ( kSes, kEvent );
	case EGS_DELETE_SECOND_SECURITY_REQ:    return Handler_EGS_DELETE_SECOND_SECURITY_REQ( kSes, kEvent );
	case EGS_DELETE_SECOND_SECURITY_VERIFY_REQ:
                                        return Handler_EGS_DELETE_SECOND_SECURITY_VERIFY_REQ( kSes, kEvent );

	//////////////////////////////////////////////////////////////////////////
	// answered only because the client blocks on them - Handlers_Stub.cpp
	case EGS_MODULE_INFO_UPDATE_NOT:		return Handler_EGS_MODULE_INFO_UPDATE_NOT( kSes, kEvent );
	case EGS_DUNGEON_PLAY_INFO_TO_SERVER_NOT:
											return Handler_EGS_DUNGEON_PLAY_INFO_TO_SERVER_NOT( kSes, kEvent );
	case EGS_UPDATE_BATTLE_FIELD_USER_POS_NOT:
											return Handler_EGS_UPDATE_BATTLE_FIELD_USER_POS_NOT( kSes, kEvent );
	case EGS_FRAME_AVERAGE_REQ:				return Handler_EGS_FRAME_AVERAGE_REQ( kSes, kEvent );
	case EGS_REQUEST_GET_AUTO_PARTY_BONUS_INFO_NOT:
											return Handler_EGS_REQUEST_GET_AUTO_PARTY_BONUS_INFO_NOT( kSes, kEvent );
	case EGS_CLIENT_QUIT_REQ:				return Handler_EGS_CLIENT_QUIT_REQ( kSes, kEvent );

	default:
		break;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::EnsureAccount( KOfflineSession& kSes, const std::wstring& wstrLoginID )
{
	std::wstring wstrID = wstrLoginID;
	if( true == wstrID.empty() )
		wstrID = m_wstrLoginID;
	if( true == wstrID.empty() )
		wstrID = L"offline";

	// Already resolved on this server object, and the ID has not changed:
	// nothing to look up. The client rebuilds its proxies several times per
	// session, so this runs far more often than an account is created.
	if( 0 != m_nUserUID && wstrID == m_wstrLoginID )
	{
		kSes.m_nUserUID		= m_nUserUID;
		kSes.m_wstrLoginID	= m_wstrLoginID;
		return true;
	}

	UidType nUserUID	= 0;
	int		iUnitSlots	= CX2OfflineDB::DEFAULT_UNIT_SLOTS;

	if( false == CX2OfflineDB::Instance()->GetOrCreateAccount( wstrID, nUserUID, iUnitSlots ) )
	{
		CX2OfflineLog::Server( L"ERROR    could not resolve account '%s' - the save file is unusable",
			wstrID.c_str() );
		return false;
	}

	m_nUserUID		= nUserUID;
	m_iUnitSlots	= iUnitSlots;
	m_wstrLoginID	= wstrID;

	kSes.m_nUserUID		= nUserUID;
	kSes.m_wstrLoginID	= wstrID;

	return true;
}

/*static*/ void CX2OfflineServer::MakeUnitInfoFromRow( KUnitInfo& kOut, const KOfflineUnitRow& kRow )
{
	MakeDefaultUnitInfo( kOut, kRow.m_nUserUID, kRow.m_nUnitUID,
		(char)kRow.m_iUnitClass, kRow.m_wstrNickName, kRow.m_iLevel );

	kOut.m_iEXP					= kRow.m_iEXP;
	kOut.m_iED					= kRow.m_iED;
	kOut.m_iSPoint				= kRow.m_iSP;
	kOut.m_kLastPos.m_iMapID	= kRow.m_iLastPos;

	// REMEMBER_LOGOUT_POSITION_TEST is on, so CX2StateServerSelect::
	// Handler_EGS_STATE_CHANGE_FIELD_REQ rebuilds the spawn point from these two
	// by walking the village line map - which is the whole "log back in where
	// you left off" behaviour. It only does that for a character with EXP above
	// zero; a fresh one is always placed at Ruben start position 1.
	kOut.m_kLastPos.m_ucLastTouchLineIndex	= (unsigned char)kRow.m_iLastLineIndex;
	kOut.m_kLastPos.m_usLastPosValue		= (unsigned short)kRow.m_iLastPosValue;
	kOut.m_wstrLastDate			= CX2OfflineDB::FormatDate( kRow.m_tLastDate );

	// The character-select screen draws its whole delete/restore UI from these
	// three: m_bDeleted picks the deleted slot layout, m_trDelAbleDate is both
	// the date printed on it and the gate on the final-delete button
	// (CreateUnitButton compares it against GetServerCurrentTime64), and
	// m_trRestoreAbleDate would gate restore - offline it never does.
	kOut.m_bDeleted				= kRow.IsDeleted();
	kOut.m_trDelAbleDate		= kRow.IsDeleted()
									? CX2OfflineDB::DelAbleDate( kRow.m_tDelDate )
									: 0LL;
	kOut.m_trRestoreAbleDate	= 0LL;

	// Cash skill points, and the date they run out. Always zero and always in
	// the past: there is no billing offline, so every skill is paid for in
	// plain SP - which is the branch the real server takes for an account with
	// no cash-skill ticket, not a special case.
	kOut.m_iCSPoint			= 0;
	kOut.m_iMaxCSPoint		= 0;
	kOut.m_wstrCSPointEndDate = L"2000-01-01 00:00:00";

	// The worn title (phase 6). This travels on the unit rather than in the
	// title packet, so it has to be here or a title survives a relog in the
	// list and vanishes off the character.
	kOut.m_iTitleID			= kRow.m_iTitleID;

	// WHICH DUNGEONS ARE UNLOCKED (phase 6). Written since phase 4 and read by
	// nobody until now, which is why every dungeon gated on another one stayed
	// locked forever.
	//
	// CX2Unit::Reset copies this map into the unit (X2Unit.cpp:208),
	// CX2Unit::IsClearDungeon answers from it, and
	// CX2DungeonManager::IsActiveDungeon refuses any dungeon whose
	// m_RequireDungeonID is not in it. So an empty map is not "no history", it
	// is "nothing past the first dungeon is playable" - the reported symptom of
	// Ruben's second dungeon still being locked after Banthus was beaten.
	{
		std::vector< KOfflineDungeonClearRow > vecClear;
		CX2OfflineDB::Instance()->LoadDungeonClears( kRow.m_nUnitUID, vecClear );

		for( size_t i = 0; i < vecClear.size(); ++i )
		{
			KDungeonClearInfo kInfo;
			kInfo.m_iDungeonID		= vecClear[i].m_iDungeonID;
			kInfo.m_iMaxScore		= vecClear[i].m_iMaxScore;
			kInfo.m_cMaxTotalRank	= (char)vecClear[i].m_iBestRank;
			kInfo.m_wstrClearTime	= CX2OfflineDB::FormatDate( vecClear[i].m_tClearDate );

			// m_bNew drives the "!" badge on the local map. False on a load:
			// these are dungeons the player has already seen the result screen
			// for, and the client sets the flag itself for one it clears now.
			kInfo.m_bNew			= false;

			kOut.m_mapDungeonClear[ kInfo.m_iDungeonID ] = kInfo;
		}
	}

	// Gear and skills (phase 5). This runs for every unit in the character
	// list, not just the selected one, because the character-select screen
	// renders each slot's model from m_mapEquippedItem - a list built without
	// it shows every character naked.
	//
	// Loading here is cheap for the selected character (the singletons are
	// already on it and return immediately) and a genuine reload per other
	// character in the list, which happens once per visit to the screen.
	CX2OfflineInventory* pInven = CX2OfflineInventory::Instance();
	CX2OfflineSkill*     pSkill = CX2OfflineSkill::Instance();

	if( true == pInven->Load( kRow.m_nUnitUID ) )
		pInven->GetEquippedItems( kOut.m_mapEquippedItem );

	if( true == pSkill->Load( kRow.m_nUnitUID ) )
		pSkill->FillUnitSkillData( kOut.m_UnitSkillData );

	// The game stat is base plus gear, so it has to be rebuilt now that the
	// gear is known - MakeDefaultUnitInfo only had the class and the level.
	MakeGameStat( kRow, kOut.m_kGameStat );
}

void CX2OfflineServer::PushLevelUp( KOfflineSession& kSes, UidType nUnitUID )
{
	KOfflineUnitRow kRow;
	if( false == CX2OfflineDB::Instance()->LoadUnit( nUnitUID, kRow ) )
		return;

	KEGS_CHAR_LEVEL_UP_NOT kNot;
	kNot.m_iUnitUID	= nUnitUID;
	kNot.m_ucLevel	= (UCHAR)kRow.m_iLevel;

	// Both stats, as the real server sends them: the base one straight off the
	// stat table for the new level, and the game one with the gear on top. The
	// client assigns them over its own and rebuilds max HP from the game stat,
	// so sending only one would quietly halve the character.
	CX2OfflineStatTable::Instance()->GetUnitStat( kRow.m_iUnitClass, kRow.m_iLevel,
												 kNot.m_kBaseStat );
	MakeGameStat( kRow, kNot.m_kGameStat );

	Reply( kSes, EGS_CHAR_LEVEL_UP_NOT, kNot );
}

/*static*/ void CX2OfflineServer::MakeGamePlayStatus( const KOfflineUnitRow& kRow,
													 OUT KGamePlayStatus& kOut )
{
	KStat kStat;

	// Base plus gear: a character in a village with armour on has more max HP
	// than one without, and this is the value the HUD's health bar is sized
	// from. Phase 3 read the base stat alone because there was no gear.
	MakeGameStat( kRow, kStat );

	// KGamePlayStatus's own constructor does NOT initialise m_iMaxHP - it is the
	// one field it forgets - so every field here is set explicitly.
	kOut.m_iMaxHP			= kStat.m_iBaseHP;

	// cur_hp 0 is the schema's "never stored" marker, and it is also what a
	// character who died would carry. Either way it must not be handed back as
	// zero: Handler_EGS_GET_MY_INVENTORY_ACK routes a zero-HP character out of a
	// battlefield and back to the village, and there is nowhere in a village to
	// heal. So a stored zero means full health.
	kOut.m_iCurHP			= ( kRow.m_iCurHP > 0 ) ? kRow.m_iCurHP : kStat.m_iBaseHP;

	if( kOut.m_iCurHP > kOut.m_iMaxHP )
		kOut.m_iCurHP		= kOut.m_iMaxHP;		///< a level or gear change shrank max HP

	// ONE_CHARGE * 3 is what CX2GUUser::ResetMaxMP computes for a character with
	// no MP-boosting gear (X2GageUI.h defines ONE_CHARGE as 100). Current MP
	// starts empty, which is what a character has on entering a village.
	kOut.m_iMaxMP			= (int)( ONE_CHARGE * 3.0f );
	kOut.m_iCurMP			= min( kRow.m_iCurMP, kOut.m_iMaxMP );

	kOut.m_iCurHyperGage	= kRow.m_iHyperGage;
	kOut.m_cCurHyperCount	= 0;

	// The per-character resource: Elsword's Way-of-the-Sword points, Chung's
	// cannonballs, Ara's force. The type must match the character or the count
	// reads back as 0 - KGamePlayStatus::GetWSP and its siblings all return 0
	// unless m_cCharAbilType is theirs.
	kOut.m_cCharAbilType	= CX2OfflineStatTable::CharAbilTypeOf( kRow.m_iUnitClass );
	kOut.m_iCharAbilCount	= kRow.m_iAbilCount;

	kOut.m_mapSkillCoolTime.clear();
	kOut.m_mapQuickSlotCoolTime.clear();
	kOut.m_mapPetMP.clear();
}

#endif SERV_IRUHADEV_OFFLINE
