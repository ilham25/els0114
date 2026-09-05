#include "stdafx.h"
#include "X2OfflineServer.h"

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-02
// Description: Offline mode - phase 4. Rooms, the dungeon run inside one, and
//              its result.
//
//              A dungeon is always played inside a "room", even solo, and the
//              room is owned by the CenterServer in the original topology: the
//              GameServer forwards EGS_* to it as ERM_*, and its ERM_* replies
//              come back down as EGS_*. Offline both halves are this file, so
//              a handler that would have been two hops is one function.
//
//              Three ways in, all producing the same single-occupant room:
//
//                EGS_CREATE_TUTORIAL_ROOM_REQ   a fresh character's tutorial
//                EGS_CREATE_ROOM_REQ            a dungeon from the village UI
//                EGS_JOIN_BATTLE_FIELD_REQ      the portal out of a village
//
//              The third is not a separate system: in this build the fields
//              outside a village ARE rooms (CX2Room::RT_BATTLE_FIELD), which is
//              why phase 3 could not deliver a walkable field and had to refuse
//              the portal. That refusal is gone.
//
//              Two things about the room are load-bearing rather than cosmetic:
//
//              * the slot must be marked host. CX2Game::IsHost() reads it, and
//                the client only drives the simulation - stage loads, NPC
//                spawns, sub-stage clears, the end of the game - when it
//                believes it is host. A non-host client sits in an empty
//                dungeon forever.
//
//              * KRoomUserInfo::m_wstrIP / m_usPort must be the client's OWN
//                UDP address. CX2Room::CheckAddPeer adds every occupied slot as
//                a P2P peer, including my own, and CX2StateDungeonGame gates
//                GameLoadingReq( 100 ) on CKTDNUDP::ConnectTestResult() - which
//                returns false while m_PeerMap is empty. With myself in it,
//                ConnectTestToPeer takes its pPeer->m_UID == m_MyUID branch and
//                sends the connect test to m_MyIPAddress / m_MyPort, i.e. the
//                client's own socket over loopback. It answers itself, the peer
//                goes P2P-connected, no relay is required, and loading
//                finishes. Get the address wrong and the dungeon hangs at 80%.
//
//              In-match combat itself never reaches here - it is P2P UDP, and
//              with one player it is nothing at all. What does reach here is
//              the accounting: EGS_NPC_UNIT_CREATE_REQ (which is the only place
//              an NPC's level is ever reported) and EGS_NPC_UNIT_DIE_REQ, which
//              is where EXP and ED are earned. See X2OfflineDropTable.h for
//              where those numbers come from and why the client cannot supply
//              them.
//
//              NOT in this phase: item drops. EGS_DROP_ITEM_NOT is never sent
//              and EGS_GET_ITEM_REQ is acknowledged empty-handed. An item has
//              to land in an inventory that survives a relog, and the inventory
//              does not round-trip through SQLite until phase 5 -
//              EGS_SELECT_UNIT_1_NOT still sends an empty m_mapItem. Dropping
//              items the next login would silently eat is worse than dropping
//              none.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

namespace
{
	/// Never dialled. CX2Room::ApplyRoomPacketData hands this to
	/// CKTDNUDP::SetMyUIDAndRelayIPAddressAndPort and then pings it once; with
	/// the self-peer connecting over loopback the relay is never required (see
	/// CKTDNUDP::ConnectTestResult), so the ping goes nowhere and nothing waits
	/// on it. It has to be a dotted quad all the same - ConvertIPToAddress runs
	/// gethostbyname on anything else.
	const wchar_t* const RELAY_IP	= L"127.0.0.1";
	const unsigned short RELAY_PORT	= 1;

	/// m_wstrCNIP is only ever stored (CX2Room::SetCenterServerIP) and handed
	/// back out; nothing offline connects to it.
	const wchar_t* const CENTER_IP	= L"127.0.0.1";

	/// The client is the only occupant, so it is always slot 0 and always host.
	const char SLOT_INDEX	= 0;
	const char MAX_SLOT		= 1;
}

//////////////////////////////////////////////////////////////////////////
// Building the room the client will see

void CX2OfflineServer::MakeRoomUserInfo( const KOfflineUnitRow& kRow, OUT KRoomUserInfo& kOut )
{
	// The constructor zeroes everything and calls m_UnitSkillData.Init(), so
	// only the fields that mean something are set here.
	kOut = KRoomUserInfo();

	kOut.m_iGSUID			= 1;
	kOut.m_iOwnerUserUID	= kRow.m_nUserUID;
	kOut.m_sServerGroupID	= 1;
	kOut.m_cAuthLevel		= (char)CX2User::XUAL_NORMAL_USER;
	kOut.m_nUnitUID			= kRow.m_nUnitUID;
	kOut.m_cUnitClass		= (char)kRow.m_iUnitClass;
	kOut.m_wstrNickName		= kRow.m_wstrNickName;
	kOut.m_ucLevel			= (unsigned char)kRow.m_iLevel;
	kOut.m_bIsObserver		= false;
	kOut.m_bIsGuestUser		= false;
	kOut.m_bIsPvpNpc		= false;

	// The worn title, and the reason it has to be here rather than only in
	// KUnitInfo. CX2Unit::UnitData::SetKRoomUserInfo does
	//     m_iTitleId = data.m_iTitleID;
	// unconditionally ([X2Unit.cpp:3455](X2Lib/X2Unit.cpp#L3455)), and
	// CX2Room::SlotData::Set_KRoomSlotInfoOfMine runs it against *my own*
	// CX2Unit - the very object the village put the title on
	// ([X2Room.cpp:2170](X2Lib/X2Room.cpp#L2170)). So a zeroed field here does
	// not merely omit the title in the dungeon, it wipes it off the character
	// for the rest of the session: the emblem stops drawing in the dungeon and
	// stays gone in the village afterwards, until the next login re-reads
	// KUnitInfo. That is ISSUES.md #13, and it is why the title looked like it
	// was missing in "field/dungeon" rather than in one or the other.
	//
	// Only m_iTitleID exists: SERV_TITLE_DATA_SIZE is on, so KRoomUserInfo's
	// short m_sTitleID is commented out ([CommonPacket.h:2200](KncWX2Server/Common/CommonPacket.h#L2200))
	// and the client reads the int.
	kOut.m_iTitleID			= kRow.m_iTitleID;

	// No stamina pair here on purpose. SERV_DELETE_ROOM_USER_INFO_DATA is
	// defined in this build, so KRoomUserInfo no longer carries m_iSpirit /
	// m_iSpiritMax and the block in CX2Unit::UnitData::SetKRoomUserInfo that
	// used to read them ([X2Unit.cpp:3446](X2Lib/X2Unit.cpp#L3446)) is compiled
	// out. Entering a room therefore cannot clobber what SetKUnitInfo already
	// put on the unit, which is why fixing KUnitInfo alone fixes the dungeon
	// room's gauge too - it reads the unit, not the slot.

	// The whole reason the room slot has to be built carefully. CX2Unit::
	// UnitData::SetKRoomUserInfo (X2Unit.cpp:3451) does
	//     m_GameStat.SetKStat( data.m_kGameStat );
	// i.e. it assigns straight over the unit's game stat - the same field
	// CX2GUUser::InitStat builds the in-world max HP from. A zeroed one here
	// undoes everything phase 3 did for the village unit, in the dungeon, where
	// it matters most. Base stat plus gear since phase 5 - a dungeon entered
	// with the base stat alone would strip the player's equipment bonuses at
	// exactly the point they are needed.
	MakeGameStat( kRow, kOut.m_kGameStat );

	// The equipped items and the skill state, so the in-dungeon character wears
	// what the village character was wearing and has the same skills on its
	// bar. KUnitInfo carries both and the room slot list is built from this
	// struct, so leaving them out is how a dungeon run ends up unarmed.
	CX2OfflineInventory::Instance()->Load( kRow.m_nUnitUID );
	CX2OfflineSkill::Instance()->Load( kRow.m_nUnitUID );

	CX2OfflineInventory::Instance()->GetEquippedItems( kOut.m_mapEquippedItem );
	CX2OfflineSkill::Instance()->FillUnitSkillData( kOut.m_UnitSkillData );

	// The client's own UDP address, so it becomes its own P2P peer and the
	// connect test succeeds over loopback. See the file header.
	if( NULL != g_pData && NULL != g_pData->GetGameUDP() )
	{
		// SERV_KTDX_OPTIMIZE_UDP_PACKET_PACK is on, so CKTDNUDP keeps its own
		// address as a u_long rather than a string - the same
		// GetMyIPAddress() phase 1's synthesised port-check ACK uses.
		const std::wstring wstrMyIP =
			CKTDNUDP::ConvertAddressToIP( g_pData->GetGameUDP()->GetMyIPAddress() );

		kOut.m_wstrIP			= wstrMyIP;
		kOut.m_usPort			= (USHORT)g_pData->GetGameUDP()->GetMyExtPort();
		kOut.m_wstrInternalIP	= wstrMyIP;
		kOut.m_usInternalPort	= (USHORT)g_pData->GetGameUDP()->GetMyPort();

		// GetMyExtPort() is only set by Handler_KXPT_PORT_CHECK_ACK; phase 1
		// synthesises that ACK, so it is set by the time any room exists. Fall
		// back rather than hand out port 0, which AddPeer would take literally.
		if( 0 == kOut.m_usPort )
			kOut.m_usPort = (USHORT)g_pData->GetGameUDP()->GetMyPort();
	}

	// A live gauge set has to ride along or CX2GUUser starts the dungeon on an
	// empty health bar - same packet shape as EGS_SELECT_UNIT_4_NOT uses.
	KGamePlayStatus kStatus;
	MakeGamePlayStatus( kRow, kStatus );
	kOut.m_vecGamePlayStatus.clear();
	kOut.m_vecGamePlayStatus.push_back( kStatus );
}

void CX2OfflineServer::MakeRoomSlots( const KOfflineUnitRow& kRow, int iSlotState,
									  OUT std::vector< KRoomSlotInfo >& vecOut )
{
	vecOut.clear();

	KRoomSlotInfo kSlot;
	kSlot.m_Index		= SLOT_INDEX;
	kSlot.m_SlotState	= (char)iSlotState;
	kSlot.m_bHost		= true;			///< load-bearing; see the file header
	kSlot.m_bReady		= true;
	kSlot.m_bPitIn		= false;
	kSlot.m_bTrade		= false;
	kSlot.m_TeamNum		= (int)CX2Room::TN_RED;

	MakeRoomUserInfo( kRow, kSlot.m_kRoomUserInfo );

	vecOut.push_back( kSlot );
}

void CX2OfflineServer::MakeRoomInfo( OUT KRoomInfo& kOut )
{
	kOut = m_kRoom.m_kInfo;

	kOut.m_wstrUDPRelayIP	= RELAY_IP;
	kOut.m_usUDPRelayPort	= RELAY_PORT;
	kOut.m_MaxSlot			= MAX_SLOT;
	kOut.m_JoinSlot			= 1;
}

bool CX2OfflineServer::OpenRoom( KOfflineSession& kSes, int iRoomType, const KRoomInfo& kReqInfo,
								 int iBattleFieldID, OUT KOfflineUnitRow& kRow )
{
	if( 0 == kSes.m_nSelectedUnitUID )
	{
		CX2OfflineLog::Server( L"ROOM     ERROR asked for a room with no character selected" );
		return false;
	}

	if( false == CX2OfflineDB::Instance()->LoadUnit( kSes.m_nSelectedUnitUID, kRow ) )
	{
		CX2OfflineLog::Server( L"ROOM     ERROR unitUID=%I64d is not in the save",
			(__int64)kSes.m_nSelectedUnitUID );
		return false;
	}

	m_kRoom.Clear();
	m_kRoom.m_bActive		= true;
	m_kRoom.m_nUnitUID		= kRow.m_nUnitUID;

	// Echo the request's own room description back - room name, password, play
	// time, difficulty, dungeon ID, get-item type are all the client's to
	// choose, and the real server passes them through untouched.
	m_kRoom.m_kInfo					= kReqInfo;
	m_kRoom.m_kInfo.m_RoomType		= (char)iRoomType;
	m_kRoom.m_kInfo.m_RoomUID		= ++m_nNextRoomUID;
	m_kRoom.m_kInfo.m_RoomState		= (char)CX2Room::RS_WAIT;
	m_kRoom.m_kInfo.m_iBattleFieldID= iBattleFieldID;
	m_kRoom.m_kInfo.m_iPartyUID		= 0;

	kSes.m_eState = S_ROOM;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Entering a room

bool CX2OfflineServer::Handler_EGS_CREATE_TUTORIAL_ROOM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CREATE_TUTORIAL_ROOM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Phase 3 forced SetIsPlayingTutorial( false ) in the client so this packet
	// could never be sent, because refusing it has no fallback:
	// CX2StateServerSelect::Handler_EGS_CREATE_TUTORIAL_ROOM_ACK does everything
	// inside if( IsValidPacket( m_iOK ) ) and otherwise just returns false - no
	// village, no error, the player sits on character select. That scaffold is
	// removed as of this phase, so this handler is now the only thing standing
	// between a brand-new character and the game.
	KEGS_CREATE_TUTORIAL_ROOM_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	KOfflineUnitRow kRow;
	if( false == OpenRoom( kSes, (int)CX2Room::RT_DUNGEON, kReq.m_RoomInfo, 0, kRow ) )
	{
		kAck.m_iOK = NetError::ERR_ROOM_00;
		return Reply( kSes, EGS_CREATE_TUTORIAL_ROOM_ACK, kAck );
	}

	// The one place this is set. Leaving the room reads it - see
	// Handler_EGS_LEAVE_ROOM_REQ for why the client's own tutorial flags cannot
	// be used for that.
	m_kRoom.m_bTutorial = true;

	MakeRoomInfo( kAck.m_RoomInfo );
	MakeRoomSlots( kRow, (int)CX2Room::SS_WAIT, kAck.m_vecSlot );
	kAck.m_wstrCNIP = CENTER_IP;

	CX2OfflineLog::Server( L"ROOM     tutorial room %I64d for unitUID=%I64d, dungeonID=%d",
		(__int64)m_kRoom.m_kInfo.m_RoomUID, (__int64)kRow.m_nUnitUID,
		m_kRoom.m_kInfo.m_iDungeonID );

	return Reply( kSes, EGS_CREATE_TUTORIAL_ROOM_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_CREATE_ROOM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CREATE_ROOM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_CREATE_ROOM_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	KOfflineUnitRow kRow;
	if( false == OpenRoom( kSes, (int)kReq.m_RoomInfo.m_RoomType, kReq.m_RoomInfo, 0, kRow ) )
	{
		kAck.m_iOK = NetError::ERR_ROOM_00;
		return Reply( kSes, EGS_CREATE_ROOM_ACK, kAck );
	}

	MakeRoomInfo( kAck.m_RoomInfo );
	MakeRoomSlots( kRow, (int)CX2Room::SS_WAIT, kAck.m_vecSlot );
	kAck.m_vecObserverSlot.clear();
	kAck.m_wstrCNIP = CENTER_IP;

	// title= is here so a play-test can tell a slot that carried the wrong
	// title from a client that declined to draw the right one - the two look
	// identical on screen. See MakeRoomUserInfo.
	CX2OfflineLog::Server( L"ROOM     room %I64d type=%d dungeonID=%d dif=%d for unitUID=%I64d title=%d",
		(__int64)m_kRoom.m_kInfo.m_RoomUID, (int)m_kRoom.m_kInfo.m_RoomType,
		m_kRoom.m_kInfo.m_iDungeonID, (int)m_kRoom.m_kInfo.m_DifficultyLevel,
		(__int64)kRow.m_nUnitUID, kRow.m_iTitleID );

	return Reply( kSes, EGS_CREATE_ROOM_ACK, kAck );
}

void CX2OfflineServer::TrackFieldNpc( const KNPCUnitReq& kReq )
{
	m_kRoom.m_mapNpcLevel[ kReq.m_UID ]	= (int)kReq.m_Level;
	m_kRoom.m_mapNpcID[ kReq.m_UID ]	= kReq.m_NPCID;
	m_kRoom.m_mapNpcNoDrop[ kReq.m_UID ]= kReq.m_bNoDrop;
	m_kRoom.m_mapNpcActive[ kReq.m_UID ]= kReq.m_bActive;
	m_kRoom.m_mapNpcGroup[ kReq.m_UID ]	= kReq.m_iGroupID;

	++m_kRoom.m_mapAliveByGroup[ kReq.m_iGroupID ];
	++m_kRoom.m_mapAliveByNpc[ std::make_pair( kReq.m_iGroupID, kReq.m_NPCID ) ];
}

void CX2OfflineServer::OnFieldNpcDie( int iNpcUID )
{
	// A middle boss is charged at MG_MIDDLE_BOSS_NPC's factor and, more
	// importantly, unblocks the next roll. Its group ID is 0, which is exactly
	// how the server marks one, so the group bookkeeping below skips it.
	const bool bWasMiddleBoss = ( m_kRoom.m_iMiddleBossAlive > 0 &&
								  iNpcUID >= m_kRoom.m_iMiddleBossAlive );

	std::map< int, int >::iterator mitG = m_kRoom.m_mapNpcGroup.find( iNpcUID );
	if( mitG != m_kRoom.m_mapNpcGroup.end() )
	{
		const int iGroupID = mitG->second;

		std::map< int, int >::iterator mitAG = m_kRoom.m_mapAliveByGroup.find( iGroupID );
		if( mitAG != m_kRoom.m_mapAliveByGroup.end() && mitAG->second > 0 )
			--mitAG->second;

		std::map< int, int >::const_iterator mitID = m_kRoom.m_mapNpcID.find( iNpcUID );
		if( mitID != m_kRoom.m_mapNpcID.end() )
		{
			std::map< std::pair< int, int >, int >::iterator mitAN =
				m_kRoom.m_mapAliveByNpc.find( std::make_pair( iGroupID, mitID->second ) );

			if( mitAN != m_kRoom.m_mapAliveByNpc.end() && mitAN->second > 0 )
				--mitAN->second;
		}

		// Queue the refill. Group 0 is the middle boss, which must never respawn
		// on a timer - it comes back only by earning the danger value again.
		if( 0 != iGroupID )
		{
			const int iSec = CX2OfflineBattleField::Instance()->GetRespawnTimeSec();

			KOfflineRoom::KRespawn kR;
			kR.m_iGroupID	= iGroupID;
			kR.m_dwDueTick	= ::GetTickCount() + (DWORD)( iSec * 1000 );

			m_kRoom.m_mapRespawn[ iNpcUID ] = kR;
		}

		m_kRoom.m_mapNpcGroup.erase( mitG );
	}

	if( true == bWasMiddleBoss )
		m_kRoom.m_iMiddleBossAlive = 0;

	//////////////////////////////////////////////////////////////////////////
	// The danger value. KBattleFieldGameManager::OnNpcUnitDie adds the monster's
	// type factor times the event rate, and wraps to zero at the maximum rather
	// than clamping - the wrap is what makes the escalation cyclical.
	CX2OfflineBattleField* pBF = CX2OfflineBattleField::Instance();

	const int iFactor = ( true == bWasMiddleBoss ) ? 4 : 1;	///< MTF_MIDDLE_BOSS_NPC : MTF_NORMAL_NPC

	m_kRoom.m_iOldDangerValue = m_kRoom.m_iDangerValue;
	m_kRoom.m_iDangerValue += iFactor * pBF->GetDangerEventRate();

	if( m_kRoom.m_iDangerValue >= pBF->GetDangerValueMax() )
	{
		m_kRoom.m_iDangerValue		= 0;
		m_kRoom.m_iOldDangerValue	= 0;
	}

	//////////////////////////////////////////////////////////////////////////
	// Roll for the middle boss, with CheckReserveMiddleBossDropEvent's own gates
	// in its own order: enough players, past the warning threshold, nothing
	// already reserved, then the Lua curve decides.
	if( true == m_kRoom.m_bMiddleBossReserved || m_kRoom.m_iMiddleBossAlive > 0 )
		return;

	if( 1 < pBF->GetBossCheckUserCount() )
		return;								///< one player offline, always

	if( m_kRoom.m_iDangerValue < pBF->GetDangerValueWarning() )
		return;

	const float fRate = pBF->GetMiddleBossDropRate( m_kRoom.m_iDangerValue,
													m_kRoom.m_iOldDangerValue );
	if( fRate <= 0.0f )
		return;

	if( (float)( rand() % 10000 ) / 100.0f < fRate )
	{
		m_kRoom.m_bMiddleBossReserved = true;

		CX2OfflineLog::Server( L"FIELDNPC middle boss reserved at danger %d (rate %.0f%%)",
			m_kRoom.m_iDangerValue, fRate );
	}
}

void CX2OfflineServer::TickField( KOfflineSession& kSes )
{
	if( false == m_kRoom.m_bActive ||
		(char)CX2Room::RT_BATTLE_FIELD != m_kRoom.m_kInfo.m_RoomType )
	{
		return;
	}

	CX2OfflineBattleField* pBF = CX2OfflineBattleField::Instance();
	const DWORD dwNow = ::GetTickCount();

	//////////////////////////////////////////////////////////////////////////
	// Respawns whose timer has elapsed, collected per group the way
	// CheckRespawnMonster collects them (BattleFieldMonsterManager.cpp:184-210).
	std::map< int, int > mapReadyByGroup;
	std::vector< int > vecDone;

	std::map< int, KOfflineRoom::KRespawn >::const_iterator mitR;
	for( mitR = m_kRoom.m_mapRespawn.begin(); mitR != m_kRoom.m_mapRespawn.end(); ++mitR )
	{
		// Unsigned tick subtraction, so this stays correct across the 49-day wrap.
		if( ( dwNow - mitR->second.m_dwDueTick ) > 0x80000000UL )
			continue;

		++mapReadyByGroup[ mitR->second.m_iGroupID ];
		vecDone.push_back( mitR->first );
	}

	if( false == mapReadyByGroup.empty() )
	{
		std::vector< KNPCUnitReq > vecNpc;
		pBF->GetRespawnMonsterList( m_kRoom.m_kInfo.m_iBattleFieldID,
			m_kRoom.m_mapAliveByGroup, m_kRoom.m_mapAliveByNpc, mapReadyByGroup, vecNpc );

		// The queue entries are consumed whether or not the group could actually
		// give a monster back - otherwise a group sitting at its cap would retry
		// on every tick forever.
		size_t d = 0;
		for( d = 0; d < vecDone.size(); ++d )
			m_kRoom.m_mapRespawn.erase( vecDone[d] );

		if( false == vecNpc.empty() )
		{
			KEGS_NPC_UNIT_CREATE_NOT kNot;

			size_t i = 0;
			for( i = 0; i < vecNpc.size(); ++i )
			{
				vecNpc[i].m_UID = m_kRoom.m_iNextNpcUID++;
				TrackFieldNpc( vecNpc[i] );

				KNPCUnitNot kOne;
				kOne.m_kNPCUnitReq = vecNpc[i];
				kNot.m_vecNPCUnitAck.push_back( kOne );
			}

			kNot.m_mapAttribNpcInfo.clear();	///< no elite monsters offline

			CX2OfflineLog::Server( L"FIELDNPC respawning %u monster(s), UIDs %d..%d (danger %d)",
				(unsigned int)vecNpc.size(), vecNpc.front().m_UID, vecNpc.back().m_UID,
				m_kRoom.m_iDangerValue );

			Reply( kSes, EGS_NPC_UNIT_CREATE_NOT, kNot );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// And the middle boss, if one has been earned.
	if( false == m_kRoom.m_bMiddleBossReserved || m_kRoom.m_iMiddleBossAlive > 0 )
		return;

	std::vector< KNPCUnitReq > vecBoss;
	pBF->GetMiddleBossMonsterInfo( m_kRoom.m_kInfo.m_iBattleFieldID,
		m_kRoom.m_iDangerValue, vecBoss );

	// The reservation is spent either way: the server deletes the event before it
	// knows whether a boss could be built (BattleFieldMonsterManager.cpp:1115).
	m_kRoom.m_bMiddleBossReserved = false;

	if( true == vecBoss.empty() )
		return;

	KEGS_NPC_UNIT_CREATE_MIDDLE_BOSS_NOT kNot;

	size_t i = 0;
	for( i = 0; i < vecBoss.size(); ++i )
	{
		vecBoss[i].m_UID = m_kRoom.m_iNextNpcUID++;
		TrackFieldNpc( vecBoss[i] );

		// Remember the first boss UID: every UID from here up belongs to this
		// boss set, which is how OnFieldNpcDie recognises one without another map.
		if( 0 == m_kRoom.m_iMiddleBossAlive )
			m_kRoom.m_iMiddleBossAlive = vecBoss[i].m_UID;

		KNPCUnitNot kOne;
		kOne.m_kNPCUnitReq = vecBoss[i];
		kNot.m_kCreatePacket.m_vecNPCUnitAck.push_back( kOne );
	}

	kNot.m_kCreatePacket.m_mapAttribNpcInfo.clear();

	CX2OfflineLog::Server( L"FIELDNPC MIDDLE BOSS: %u monster(s), UIDs %d..%d at danger %d",
		(unsigned int)vecBoss.size(), vecBoss.front().m_UID, vecBoss.back().m_UID,
		m_kRoom.m_iDangerValue );

	Reply( kSes, EGS_NPC_UNIT_CREATE_MIDDLE_BOSS_NOT, kNot );
}

void CX2OfflineServer::PushFieldNpcs( KOfflineSession& kSes )
{
	if( false == m_kRoom.m_bActive || true == m_kRoom.m_bFieldNpcSent )
		return;

	if( (char)CX2Room::RT_BATTLE_FIELD != m_kRoom.m_kInfo.m_RoomType )
		return;

	m_kRoom.m_bFieldNpcSent = true;

	std::vector< KNPCUnitReq > vecNpc;
	CX2OfflineBattleField::Instance()->GetCreateMonsterList(
		m_kRoom.m_kInfo.m_iBattleFieldID, 1, vecNpc );

	if( true == vecNpc.empty() )
	{
		CX2OfflineLog::Server( L"FIELDNPC field %d has no monsters to place",
			m_kRoom.m_kInfo.m_iBattleFieldID );
		return;
	}

	// The offline server owns NPC UIDs here exactly as it does in a dungeon, and
	// for the same reason: KRoomMonsterManager assigns them and the client reads
	// them back. Recording level and ID keeps EGS_NPC_UNIT_DIE_REQ able to price
	// a field kill through the same drop-table path a dungeon kill uses - the
	// battlefield half of the table, which is why GetNpcReward takes a
	// bBattleField flag at all.
	size_t i = 0;
	for( i = 0; i < vecNpc.size(); ++i )
	{
		vecNpc[i].m_UID = m_kRoom.m_iNextNpcUID++;
		TrackFieldNpc( vecNpc[i] );
	}

	KEGS_BATTLE_FIELD_NPC_LOAD_NOT kNot;
	kNot.m_iBattleFieldID	= m_kRoom.m_kInfo.m_iBattleFieldID;
	kNot.m_vecNPCData		= vecNpc;
	kNot.m_mapAttribNpcInfo.clear();	///< attribute-enchanted monsters: not offline

	CX2OfflineLog::Server( L"FIELDNPC field %d placing %u monster(s), UIDs %d..%d",
		m_kRoom.m_kInfo.m_iBattleFieldID, (unsigned int)vecNpc.size(),
		vecNpc.front().m_UID, vecNpc.back().m_UID );

	Reply( kSes, EGS_BATTLE_FIELD_NPC_LOAD_NOT, kNot );

	// No completion handshake: CX2StateBattleField's dispatch handles the NOT and
	// the follow-up Handler_EGS_BATTLE_FIELD_NPC_LOAD_COMPLETE_REQ() call right
	// beneath it is commented out in this build (X2StateBattleField.cpp:286), so
	// the client never answers. Fire and forget is correct.

	// Phase 6: SQT_FIND_NPC in a field. A field's monsters arrive through this
	// packet rather than through EGS_NPC_UNIT_CREATE_REQ, so the hook in that
	// handler never sees them.
	{
		std::vector< int > vecNpcID;
		for( size_t i = 0; i < vecNpc.size(); ++i )
			vecNpcID.push_back( vecNpc[i].m_NPCID );

		QuestOnFindNPC( kSes, vecNpcID );
	}
}

void CX2OfflineServer::PushRemainingPlayTime( KOfflineSession& kSes )
{
	// THE dungeon freeze. Three rounds of wrong guesses ended here, so it is
	// worth writing down exactly what this is.
	//
	// CX2Game::m_AllowFrameCount is a speed-hack detector, and its comment
	// (X2Game.h:1541) states the mechanism outright: it is refilled by
	// EGS_REMAINING_PLAY_TIME_NOT, and "if that packet is not received again in
	// time and it reaches 0, key input becomes impossible". CX2Game::KeyProcess
	// gates *all* input gathering on it:
	//
	//     if ( GetEnableAllKeyProcess() && m_AllowFrameCount > 0 )
	//
	// It is seeded to (int)( 85 * 5.3 ) = 450 and decremented once per
	// OnFrameMove (X2Game.cpp:2741). At 60fps that is 7.5 seconds of play, after
	// which the client silently stops reading the keyboard - no popup, no log,
	// no packet, nothing. The character stands in USI_WAIT while NPCs, UI and
	// the client's own three-second status push all carry on normally, which is
	// precisely the shape of the bug that survived three fixes.
	//
	// Every earlier theory was disproved by the diagnostic, and this one is
	// confirmed by it: input was never blocked. m_bStopUnitChain was false,
	// m_bCanNotInput was false, m_fCanNotInputTime was 0.00. Nothing was
	// suppressing input, because the input was never gathered in the first place.
	//
	// It also explains the asymmetry that looked like two separate bugs: the
	// switch at X2Game.cpp:2733 exempts XS_TRAINING_GAME and XS_BATTLE_FIELD
	// from the decrement. That is exactly why the field became freely walkable
	// once its slot state was fixed, while dungeons kept freezing.
	//
	// The real server broadcasts this every ms_fPlayTimeNotifyGap = 5.0 seconds
	// from KRoom::CheckPlayTime (Room.cpp:551-566) - a deliberately thin margin
	// against the 5.3s allowance, which is the whole point of a speed-hack
	// check. Offline there is no timer to hang this on: the emulator only runs
	// when the client sends something. EGS_UPDATE_PLAY_STATUS_NOT is the one
	// packet with a fixed cadence during a dungeon run - every 3.01s in the
	// logs - so the refill rides on it. Faster than the real server, therefore
	// safely inside the window by the studio's own arithmetic.
	if( NULL == g_pMain || CX2Main::XS_DUNGEON_GAME != g_pMain->GetNowStateID() )
		return;

	if( false == m_kRoom.m_bActive )
		return;

	// Both entry paths ask for 19999.0f (DUNGEON_DEFAULT_LIMIT_PLAY_TIME), so
	// this is effectively "no limit". The fallback covers a room description
	// that arrives without one rather than letting the clock start negative:
	// the client fades the world to black below 60 seconds remaining
	// (CX2DungeonGame::Handler_EGS_REMAINING_PLAY_TIME_NOT), and zero is what
	// the real server turns into a forced end-of-game.
	float fLimit = m_kRoom.m_kInfo.m_fPlayTime;
	if( fLimit <= 0.0f )
		fLimit = 19999.0f;

	const float fElapsed = (float)( ::GetTickCount() - m_kRoom.m_dwPlayStartTick ) / 1000.0f;

	float fRemain = fLimit - fElapsed;
	if( fRemain < 0.0f )
		fRemain = 0.0f;

	// KEGS_REMAINING_PLAY_TIME_NOT is a bare `typedef float`
	// (ClientPacket.h:867), not a struct - the GameServer sends the float
	// straight through as well.
	KEGS_REMAINING_PLAY_TIME_NOT fNot = fRemain;

	Reply( kSes, EGS_REMAINING_PLAY_TIME_NOT, fNot );
}

bool CX2OfflineServer::Handler_EGS_QUICK_START_DUNGEON_GAME_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_QUICK_START_DUNGEON_GAME_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_QUICK_START_DUNGEON_GAME_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_iFailUnitUID	= 0;

	// This is how a normal dungeon is entered, and the plan's guess that it
	// needs the party subsystem was wrong. CX2PartyUI::GameStartCurrentMember
	// (X2PartyUI.cpp:4984-5008) takes its branch when DoIHaveParty() is *false*
	// as readily as when the player leads a party, and picking a dungeon on the
	// local map with no party sends nothing at all - CX2LocalMapUI just writes
	// GetMyPartyData()->m_iDungeonID locally (X2LocalMapUI.cpp:166-175). So solo
	// play needs this one packet, not EGS_PARTY_CHANGE_DUNGEON_REQ, not
	// EGS_CREATE_PARTY_REQ, and no party ever has to exist.
	//
	// The real server's path is three hops - GameServer
	// DungeonGameStart_SingleOpenDungeonRoom -> CenterServer
	// ERM_SINGLE_OPEN_DUNGEON_ROOM_REQ -> a 3-second countdown ->
	// ERM_PARTY_OPEN_DUNGEON_ROOM_NOT back out as EGS_PARTY_GAME_START_NOT - but
	// only the two endpoints are packets the client ever sees. The countdown is
	// EGS_GAME_START_TIME_COUNT_NOT and is skipped here: it exists so a party
	// can see each other commit, and offline it would only add three seconds.
	KRoomInfo kSeed;
	kSeed.Initialize();
	kSeed.m_RoomName		= L"solo";
	kSeed.m_bPublic			= false;
	kSeed.m_iDungeonID		= kReq.m_iDungeonID;
	kSeed.m_DifficultyLevel	= kReq.m_DifficultyLevel;
	kSeed.m_cDungeonMode	= kReq.m_cDungeonMode;
	kSeed.m_cGetItemType	= kReq.m_cGetItemType;

	// DUNGEON_DEFAULT_LIMIT_PLAY_TIME, from XSLDungeon.h - server-side data this
	// project cannot include. The per-dungeon limit that would normally replace
	// it lives in the server's own dungeon table, so every dungeon gets the
	// default clock here. It is 19999 seconds, i.e. effectively none, which is
	// the same value the real server falls back to when a dungeon has no limit.
	kSeed.m_fPlayTime		= 19999.0f;

	KOfflineUnitRow kRow;
	if( false == OpenRoom( kSes, (int)CX2Room::RT_DUNGEON, kSeed, 0, kRow ) )
	{
		kAck.m_iOK = NetError::ERR_ROOM_00;
		return Reply( kSes, EGS_QUICK_START_DUNGEON_GAME_ACK, kAck );
	}

	Reply( kSes, EGS_QUICK_START_DUNGEON_GAME_ACK, kAck );

	// The ACK only releases the client's wait and re-locks its shortcut keys;
	// CX2PartyManager::Handler_EGS_QUICK_START_DUNGEON_GAME_ACK does nothing
	// else. EGS_PARTY_GAME_START_NOT is the packet that moves the player:
	// Handler_EGS_PARTY_GAME_START_NOT (X2PartyManager.cpp:1397) builds the
	// dungeon room out of it and then sends XGM_STATE_CHANGE -> XS_DUNGEON_GAME
	// directly, with no room screen in between. Which is why the room state has
	// to be advanced here rather than in
	// Handler_EGS_STATE_CHANGE_GAME_START_REQ - on this path that packet is
	// never sent, and the loading handler expects a room already in RS_LOADING
	// with its clock started.
	m_kRoom.m_kInfo.m_RoomState	= (char)CX2Room::RS_LOADING;
	m_kRoom.m_dwPlayStartTick	= ::GetTickCount();

	KEGS_PARTY_GAME_START_NOT kNot;
	kNot.m_sWorldID	= 0;
	MakeRoomInfo( kNot.m_RoomInfo );

	// SS_LOADING, not SS_WAIT, for the reason the battlefield join had to learn:
	// CX2Game::AddUserUnit skips SS_CLOSE / SS_EMPTY / SS_WAIT slots
	// (X2Game.cpp:1855-1859), and on this path nothing later promotes the slot
	// before the units are built. It also matches the state the room path is in
	// at the same moment - EGS_STATE_CHANGE_GAME_START_NOT sends SS_LOADING too.
	MakeRoomSlots( kRow, (int)CX2Room::SS_LOADING, kNot.m_vecSlot );
	kNot.m_vecObserverSlot.clear();
	kNot.m_wstrCNIP = CENTER_IP;
	kNot.m_vecInventorySlotInfo.clear();		///< no inventory until phase 5

	CX2OfflineLog::Server( L"ROOM     solo dungeon room %I64d dungeonID=%d dif=%d mode=%d for unitUID=%I64d",
		(__int64)m_kRoom.m_kInfo.m_RoomUID, kReq.m_iDungeonID,
		(int)kReq.m_DifficultyLevel, (int)kReq.m_cDungeonMode,
		(__int64)kRow.m_nUnitUID );

	return Reply( kSes, EGS_PARTY_GAME_START_NOT, kNot );
}

bool CX2OfflineServer::Handler_EGS_JOIN_BATTLE_FIELD_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_JOIN_BATTLE_FIELD_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_JOIN_BATTLE_FIELD_ACK kAck;
	kAck.Initialize();
	kAck.m_iOK = NetError::NET_OK;

	// A battlefield room carries no client-side room description at all - the
	// request is three fields - so build one rather than echoing.
	KRoomInfo kSeed;
	kSeed.Initialize();
	kSeed.m_RoomName		= L"battlefield";
	kSeed.m_bPublic			= true;
	kSeed.m_DifficultyLevel	= (char)CX2Dungeon::DL_NORMAL;
	kSeed.m_fPlayTime		= 0.0f;			///< a field has no clock

	KOfflineUnitRow kRow;
	if( false == OpenRoom( kSes, (int)CX2Room::RT_BATTLE_FIELD, kSeed, kReq.m_iBattleFieldID, kRow ) )
	{
		// Unlike EGS_STATE_CHANGE_FIELD_ACK, the failure path here does not
		// re-send - CX2State::Handler_EGS_JOIN_BATTLE_FIELD_ACK just returns
		// false - so an error is safe. ERR_BATTLEFIELD_00 ("not in a state that
		// can move to that area") is honest and is not one of the two codes the
		// handler special-cases into a "you need level N" / "clear dungeon X"
		// dialog, both of which would be lies.
		kAck.m_iOK = NetError::ERR_BATTLEFIELD_00;
		return Reply( kSes, EGS_JOIN_BATTLE_FIELD_ACK, kAck );
	}

	kAck.m_kBattleFieldJoinInfo.Initialize();
	kAck.m_kBattleFieldJoinInfo.m_iBattleFieldID	= kReq.m_iBattleFieldID;
	kAck.m_kBattleFieldJoinInfo.m_iStartPosIndex	= kReq.m_StartPosIndex;
	kAck.m_kBattleFieldJoinInfo.m_bMoveForMyParty	= false;

	// A battlefield is ALREADY PLAYING when you join it, and both of these have
	// to say so.
	//
	// There is no game-start packet on this path: the ACK handler goes straight
	// to XGM_STATE_CHANGE -> XS_BATTLE_FIELD, so the slot state in this one
	// packet is the slot state the game is built from. CX2Game::AddUserUnit
	// skips every slot in SS_CLOSE, SS_EMPTY *or SS_WAIT* (X2Game.cpp:1855-1859)
	// - so a joining slot marked SS_WAIT means the player's own unit is never
	// added to the world, which looks exactly like "the camera does not follow
	// my character and I cannot move", because there is no character.
	//
	// The dungeon path gets away with SS_WAIT in its create-room ACK only
	// because EGS_STATE_CHANGE_GAME_START_NOT moves the slot to SS_LOADING
	// before AddUserUnit runs.
	//
	// SS_PLAY is also what the real server would send: KRoomSlot::
	// GetRoomSlotInfo derives the state from the room and user FSMs and caps it
	// at SS_PLAY (RoomSlot.cpp:64-74), and a battlefield room is in S_PLAY.
	m_kRoom.m_kInfo.m_RoomState = (char)CX2Room::RS_PLAY;

	MakeRoomInfo( kAck.m_RoomInfo );
	MakeRoomSlots( kRow, (int)CX2Room::SS_PLAY, kAck.m_vecSlot );

	kAck.m_wstrCNIP			= CENTER_IP;
	kAck.m_iLastTouchIndex	= kRow.m_iLastLineIndex;
	kAck.m_iRequireLevel	= 0;
	kAck.m_iRequireDungeonID= 0;
	kAck.m_wstrUDPRelayIP	= RELAY_IP;
	kAck.m_usUDPRelayPort	= RELAY_PORT;
	kAck.m_vecEnterCashShopUser.clear();

	// The battlefield IS the character's map as far as the save is concerned:
	// Handler_EGS_GET_MY_INVENTORY_ACK reads unit.last_pos back on the next
	// login and re-joins the battlefield when it is in the VMI_BATTLE_FIELD_*
	// range. That is the branch phase 3 said would never fire; it fires now.
	CX2OfflineDB::Instance()->SaveLastPosition( kRow.m_nUnitUID, kReq.m_iBattleFieldID );

	CX2OfflineLog::Server( L"ROOM     battlefield %d room %I64d for unitUID=%I64d (startPos=%d)",
		kReq.m_iBattleFieldID, (__int64)m_kRoom.m_kInfo.m_RoomUID,
		(__int64)kRow.m_nUnitUID, kReq.m_StartPosIndex );

	return Reply( kSes, EGS_JOIN_BATTLE_FIELD_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_ROOM_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_ROOM_LIST_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Nobody else is playing, so the list is empty and the count of pages is
	// one - not zero, which the paging UI would divide by.
	KEGS_ROOM_LIST_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_nTotalPage	= 1;
	kAck.m_nViewPage	= kReq.m_nViewPage;
	kAck.m_vRoomInfo.clear();

	return Reply( kSes, EGS_ROOM_LIST_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_LEAVE_ROOM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_LEAVE_ROOM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// From the room screen and the result screen the ACK alone is what leaves:
	// CX2StateDungeonRoom's handler calls Handler_EGS_STATE_CHANGE_FIELD_REQ()
	// and CX2StateDungeonResult's calls ReturnToPlaceWhereBeforeDungeonStart() -
	// which, for the tutorial, means the same thing (X2State.cpp:11131, "the
	// tutorial starts from XS_SERVER_SELECT"). Either way the village is next,
	// and phase 3 already owns that. Leaving from inside the game is different;
	// see below.
	KEGS_LEAVE_ROOM_ACK kAck;
	kAck.m_iOK				= NetError::NET_OK;
	kAck.m_iReason			= kReq.m_iReason;
	kAck.m_iItemID			= 0;
	kAck.m_bNotLeaveParty	= false;
	kAck.m_kBattleFieldJoinInfo.Initialize();

	// Nothing is paid out here any more, and that is the point: EXP is credited
	// per kill in Handler_EGS_NPC_UNIT_DIE_REQ and ED coin by coin as they are
	// picked up (Handler_EGS_GET_ITEM_REQ), so a player leaving mid-run already
	// has everything the run earned. Paying m_iRewardEXP again here would double
	// it, which is exactly the bug this replaced.

	// Leaving a dungeon mid-run needs a second packet, and the comment above was
	// wrong to say the ACK is what leaves - that is true of the room screen and
	// the result screen, but not of the game itself.
	// CX2StateDungeonGame::OnFrameMove (X2StateDungeonGame.cpp:894) waits on TWO
	// flags:
	//
	//     if ( m_bReceive_KEGS_LEAVE_ROOM_ACK && false == m_bStateChangeLocalMap )
	//     {
	//         if ( m_bReceive_KEGS_UNIT_INFO_UPDATE )  -> StateChangeFieldReq()
	//         else if ( m_bLeaveRoomAtTutorial )       -> the Ruben fallback
	//     }
	//
	// and only EGS_UPDATE_UNIT_INFO_NOT sets the second one
	// (X2StateDungeonGame.cpp:3062). With just the ACK, the request is accepted,
	// the flag is set, the condition never passes, and pressing Leave does
	// nothing at all - no error, no transition. That is why the tutorial could
	// always be left and a real dungeon could not: the ESC dialog's
	// DGUCM_GAME_EXIT_OK branches on GetIsPlayingTutorial()
	// (X2StateDungeonGame.cpp:1443), and only the tutorial arm sets
	// m_bLeaveRoomAtTutorial. The real-dungeon arm sends the REQ and nothing else.
	//
	// m_bGameEnd stays FALSE here, which matters: a true would make OnFrameMove
	// at line 984 count down m_fGameEndWaitTime and then fire
	// StateChangeResultReq(), racing the village transition with a results
	// screen for a run that was abandoned rather than finished.
	//
	// Gated on the room's own m_bTutorial, so the tutorial keeps the branch it
	// already leaves correctly through (m_bLeaveRoomAtTutorial) rather than
	// being silently moved onto the other one.
	//
	// This gate used to read g_pMain->GetIsExitingTutorial(), and that was
	// wrong in a way the phase 4 exit test could not see: the client sets that
	// flag true when the tutorial is left (X2StateDungeonGame.cpp:1448) and
	// clears it only on the way back to character select, the PvP lobby or unit
	// select - never on entering a village or a dungeon. So after one tutorial
	// run it stayed true for the whole session, this block never ran again, and
	// pressing Leave in any later dungeon sent EGS_LEAVE_ROOM_ACK with no
	// EGS_UPDATE_UNIT_INFO_NOT behind it: the client set one of its two flags,
	// the condition at X2StateDungeonGame.cpp:894 never passed, and it re-sent
	// the request on every ESC with nothing happening. The packet log showed it
	// exactly - six LEAVE_ROOM_REQ/ACK pairs and not one _NOT.
	//
	// GetIsPlayingTutorial() would not have worked either: the ESC dialog calls
	// SetIsPlayingTutorial( false ) one line before sending this request
	// (X2StateDungeonGame.cpp:1445), so by the time it arrives both client
	// flags say the same thing for both cases.
	if( true == m_kRoom.m_bActive &&
		false == m_kRoom.m_bTutorial &&
		(char)CX2Room::RT_DUNGEON == m_kRoom.m_kInfo.m_RoomType &&
		NULL != g_pMain &&
		CX2Main::XS_DUNGEON_GAME == g_pMain->GetNowStateID() )
	{
		KOfflineUnitRow kAfter;
		if( true == CX2OfflineDB::Instance()->LoadUnit( m_kRoom.m_nUnitUID, kAfter ) )
		{
			KEGS_UPDATE_UNIT_INFO_NOT kUpdate;
			MakeUnitInfoFromRow( kUpdate.m_kUnitInfo, kAfter );
			kUpdate.m_bGameEnd = false;
			kUpdate.m_vecKInventorySlotInfo.clear();
			kUpdate.m_mapItemObtained.clear();

			Reply( kSes, EGS_UPDATE_UNIT_INFO_NOT, kUpdate );
		}
	}

	CX2OfflineLog::Server( L"ROOM     leaving room %I64d (reason=%d)",
		(__int64)m_kRoom.m_kInfo.m_RoomUID, kReq.m_iReason );

	m_kRoom.Clear();
	kSes.m_eState = S_FIELD_MAP;

	return Reply( kSes, EGS_LEAVE_ROOM_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// Room options. Nothing offline cares about any of them; every one is waited on.

bool CX2OfflineServer::Handler_EGS_CHANGE_DUNGEON_DIFFICULTY_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	// KEGS_CHANGE_DUNGEON_DIFFICULTY_REQ is a bare int, not a struct.
	KEGS_CHANGE_DUNGEON_DIFFICULTY_REQ iDifficulty = 0;
	if( false == ReadReq( kEvent, iDifficulty ) )
		return false;

	m_kRoom.m_kInfo.m_DifficultyLevel = (char)iDifficulty;

	KEGS_CHANGE_DUNGEON_DIFFICULTY_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;
	Reply( kSes, EGS_CHANGE_DUNGEON_DIFFICULTY_ACK, kAck );

	// The room screen only redraws the radio button from the broadcast, not from
	// the ACK, so both go out - exactly as the CenterServer sends both.
	KEGS_CHANGE_DUNGEON_DIFFICULTY_NOT iNot = iDifficulty;
	return Reply( kSes, EGS_CHANGE_DUNGEON_DIFFICULTY_NOT, iNot );
}

bool CX2OfflineServer::Handler_EGS_CHANGE_MOTION_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CHANGE_MOTION_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_CHANGE_MOTION_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;
	Reply( kSes, EGS_CHANGE_MOTION_ACK, kAck );

	// The waving animation in the room screen is played from the broadcast, not
	// from the ACK - CX2StateDungeonRoom::Handler_EGS_CHANGE_MOTION_NOT.
	KEGS_CHANGE_MOTION_NOT kNot;
	kNot.m_UnitUID	= m_kRoom.m_nUnitUID;
	kNot.m_cMotionID= kReq.m_cMotionID;

	return Reply( kSes, EGS_CHANGE_MOTION_NOT, kNot );
}

//////////////////////////////////////////////////////////////////////////
// Starting the game

bool CX2OfflineServer::Handler_EGS_STATE_CHANGE_GAME_START_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_STATE_CHANGE_GAME_START_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_STATE_CHANGE_GAME_START_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	KOfflineUnitRow kRow;
	if( false == m_kRoom.m_bActive ||
		false == CX2OfflineDB::Instance()->LoadUnit( m_kRoom.m_nUnitUID, kRow ) )
	{
		kAck.m_iOK = NetError::ERR_ROOM_00;
		return Reply( kSes, EGS_STATE_CHANGE_GAME_START_ACK, kAck );
	}

	Reply( kSes, EGS_STATE_CHANGE_GAME_START_ACK, kAck );

	// The ACK only clears the client's wait. The _NOT is what actually starts
	// the game: CX2StateDungeonRoom::Handler_EGS_STATE_CHANGE_GAME_START_NOT
	// sends XGM_STATE_CHANGE -> XS_DUNGEON_GAME, and only if its own slot comes
	// back host or ready. It also short-circuits the relay check on
	// GetUserNum() == 1, which is exactly our case.
	m_kRoom.m_kInfo.m_RoomState	= (char)CX2Room::RS_LOADING;
	m_kRoom.m_dwPlayStartTick	= ::GetTickCount();

	KEGS_STATE_CHANGE_GAME_START_NOT kNot;
	kNot.m_cRoomState	= (char)CX2Room::RS_LOADING;
	kNot.m_iGameType	= (int)CX2Game::GT_DUNGEON;
	{
		int iSpiritMaxUnused = 0;
		FillSpirit( kNot.m_iSpirit, iSpiritMaxUnused );	///< this packet carries no max
	}
	kNot.m_sWorldID		= kReq.m_sWorldID;
	MakeRoomSlots( kRow, (int)CX2Room::SS_LOADING, kNot.m_vecSlot );
	kNot.m_vecInventorySlotInfo.clear();		///< no inventory until phase 5

	CX2OfflineLog::Server( L"GAME     starting dungeonID=%d dif=%d in room %I64d",
		m_kRoom.m_kInfo.m_iDungeonID, (int)m_kRoom.m_kInfo.m_DifficultyLevel,
		(__int64)m_kRoom.m_kInfo.m_RoomUID );

	return Reply( kSes, EGS_STATE_CHANGE_GAME_START_NOT, kNot );
}

bool CX2OfflineServer::Handler_EGS_GAME_LOADING_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GAME_LOADING_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KPacketOK kAck;
	kAck.m_iOK = NetError::NET_OK;
	Reply( kSes, EGS_GAME_LOADING_ACK, kAck );

	// The client reports 10 / 30 / 80 as it loads, then 100 once its own UDP
	// connect test passes. The progress broadcast is what draws the other
	// players' loading bars; with one player it is drawing its own.
	KEGS_GAME_LOADING_NOT kProgress;
	kProgress.m_iUnitUID		= m_kRoom.m_nUnitUID;
	kProgress.m_iLoadingProgress= kReq.m_iLoadingProgress;
	Reply( kSes, EGS_GAME_LOADING_NOT, kProgress );

	if( kReq.m_iLoadingProgress < 100 )
		return true;

	KOfflineUnitRow kRow;
	if( false == m_kRoom.m_bActive ||
		false == CX2OfflineDB::Instance()->LoadUnit( m_kRoom.m_nUnitUID, kRow ) )
		return true;

	// Everyone has loaded, which offline is true the moment the one player has.
	ReplyID( kSes, EGS_GAME_LOADING_ALL_UNIT_OK_NOT );

	// And this is the packet that hands control to the dungeon:
	// CX2StateDungeonGame::PlayStartNot -> GameStart(), then, because the slot
	// is host, DungeonStageLoadReq( 0 ).
	m_kRoom.m_kInfo.m_RoomState = (char)CX2Room::RS_PLAY;

	KEGS_PLAY_START_NOT kStart;
	kStart.m_RoomState	= (char)CX2Room::RS_PLAY;
	kStart.m_iDungeonID	= m_kRoom.m_kInfo.m_iDungeonID;
	MakeRoomSlots( kRow, (int)CX2Room::SS_PLAY, kStart.m_vecSlot );

	CX2OfflineLog::Server( L"GAME     loading complete, play start (dungeonID=%d)",
		m_kRoom.m_kInfo.m_iDungeonID );

	Reply( kSes, EGS_PLAY_START_NOT, kStart );

	// Phase 6: SQT_VISIT_DUNGEON. The server does this on exactly this packet,
	// one line after sending it (GSUserRoomCommon.cpp:1426-1432) - the dungeon
	// counts as visited when play starts, not when the room was made.
	QuestOnEnterDungeon( kSes );

	return true;
}

bool CX2OfflineServer::Handler_EGS_MY_USER_UNIT_INFO_TO_SERVER_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_MY_USER_UNIT_INFO_TO_SERVER_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Combat scores - HP left, combo, technical, damage taken. The client is
	// trusted with these by design (in-match play was always client-side), and
	// they are all the result screen needs; the rank thresholds live in the
	// server's own Lua and are not reproduced here, so the ranks come back as
	// the default and only the EXP/ED numbers are real. See the plan.
	m_kRoom.m_kPlayResult		= kReq.m_kMyPlayResult;
	m_kRoom.m_bHavePlayResult	= true;

	KPacketOK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_MY_USER_UNIT_INFO_TO_SERVER_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_BATTLE_FIELD_NPC_LOAD_COMPLETE_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	// No request body (SendID). CX2StateBattleField waits on the ACK before it
	// will let the player move.
	KEGS_BATTLE_FIELD_NPC_LOAD_COMPLETE_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	Reply( kSes, EGS_BATTLE_FIELD_NPC_LOAD_COMPLETE_ACK, kAck );

	// Phase 6: SQT_VISIT_FIELD. This is the field's equivalent of
	// EGS_FIELD_LOADING_COMPLETE_REQ in a village - the moment the player can
	// actually move in it, which is when the server counts it as visited
	// (KUserQuestManager::Handler_OnEnterTheBattleField).
	if( true == m_kRoom.m_bActive && 0 != m_kRoom.m_kInfo.m_iBattleFieldID )
		QuestOnEnterField( kSes, m_kRoom.m_kInfo.m_iBattleFieldID );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// The stage's monster list.
//
// Mirrors CXSLDungeon::GetNPCData (KncWX2Server/Common/X2Data/XSLDungeon.cpp:207)
// against the client's own copy of the same dungeon script - which is the whole
// point: the server parsed that script to build this list, and the client has
// the identical file in its .kom. Nothing is duplicated, the parser is the
// studio's own (SubStageData::LoadNPCData, revived under the flag), and this
// function is only the KNPCUnitReq assembly the server did on top of it.

void CX2OfflineServer::BuildStageNpcData( int iStageID, OUT std::map< int, KNPCList >& mapOut )
{
	mapOut.clear();

	if( NULL == g_pData || NULL == g_pData->GetDungeonManager() )
		return;

	// The dungeon ID the client asked to play, difficulty included - the same
	// key CX2DungeonGame::DungeonLoading uses.
	const CX2Dungeon::DUNGEON_ID eDungeonID = (CX2Dungeon::DUNGEON_ID)
		( m_kRoom.m_kInfo.m_iDungeonID + (int)m_kRoom.m_kInfo.m_DifficultyLevel );

	// bIsNpcLoad = true is what makes SubStageData::LoadData read NPC_GROUP.
	// This dungeon object exists only to be read: the live one belongs to
	// CX2DungeonGame and is loaded WITHOUT the flag, so the monsters reach the
	// world through the packet exactly once.
	CX2Dungeon* pDungeon = g_pData->GetDungeonManager()->CreateDungeon( eDungeonID, true );
	if( NULL == pDungeon )
	{
		CX2OfflineLog::Server( L"GAME     ERROR no dungeon script for ID=%d", (int)eDungeonID );
		return;
	}

	// Relative-level dungeons scale their monsters to the party; the client's
	// DungeonData carries the flag but not the server's m_NpcLevel, so the
	// dungeon's own level band stands in for it. Fixed-level dungeons - which
	// every tutorial is - take the script's level directly and need none of this.
	int iRelativeBase = 0;
	bool bRelative = false;

#ifdef HENIR_TEST
	if( NULL != pDungeon->GetDungeonData() )
	{
		bRelative = pDungeon->GetDungeonData()->m_bRelativeMonsterLevel;

		if( true == bRelative )
		{
			iRelativeBase = ( pDungeon->GetDungeonData()->m_MaxLevel > 0 )
								? pDungeon->GetDungeonData()->m_MaxLevel
								: pDungeon->GetDungeonData()->m_MinLevel;

			CX2OfflineLog::Server(
				L"GAME     NOTE dungeon %d scales monsters to the party; using its own level band (%d)"
				L" in place of the server's m_NpcLevel, which the client does not carry",
				(int)eDungeonID, iRelativeBase );
		}
	}
#endif HENIR_TEST

	std::vector< CX2DungeonStage::StageData* >& vecStage = pDungeon->GetStageDataList();

	if( iStageID < 0 || iStageID >= (int)vecStage.size() )
	{
		CX2OfflineLog::Server( L"GAME     ERROR stage %d out of range (dungeon %d has %u)",
			iStageID, (int)eDungeonID, (unsigned int)vecStage.size() );
		SAFE_DELETE( pDungeon );
		return;
	}

	CX2DungeonStage::StageData* pStageData = vecStage[ iStageID ];
	if( NULL == pStageData )
	{
		SAFE_DELETE( pDungeon );
		return;
	}

	for( size_t iSub = 0; iSub < pStageData->m_SubStageDataList.size(); ++iSub )
	{
		CX2DungeonSubStage::SubStageData* pSubStageData = pStageData->m_SubStageDataList[ iSub ];
		if( NULL == pSubStageData )
			continue;

		KNPCList kList;

		for( size_t i = 0; i < pSubStageData->m_NPCDataList.size(); ++i )
		{
			const CX2DungeonSubStage::NPCData* pNpc = pSubStageData->m_NPCDataList[i];
			if( NULL == pNpc || CX2UnitManager::NUI_NONE == pNpc->m_UnitID )
				continue;

			// The per-monster spawn chance, rolled the way GetNPCData rolls it.
			// Sub-NPCs already had their pick made during the parse (only one of
			// a SUB_NPC set gets a non-zero selection), so they are not rolled
			// again here - rolling twice would thin out every group.
			if( false == pNpc->m_bSubNpc )
			{
				if( pNpc->m_Rate < ( rand() % 100 ) )
					continue;
			}

			KNPCUnitReq kReq;
			kReq.Init();

			kReq.m_NPCID			= (int)pNpc->m_UnitID;
			kReq.m_KeyCode			= (char)pNpc->m_KeyCode;
			kReq.m_bFocusCamera		= pNpc->m_bFocusCamera;
			kReq.m_bActive			= pNpc->m_bActive;
			kReq.m_bShowGage		= pNpc->m_bShowGage;
			kReq.m_AddPos.x			= pNpc->m_AddPos.x;
			kReq.m_AddPos.y			= pNpc->m_AddPos.y;
			kReq.m_AddPos.z			= pNpc->m_AddPos.z;
			kReq.m_bHasBossGage		= pNpc->m_bHasBossGage;
			kReq.m_bShowBossName	= pNpc->m_bShowBossName;
			kReq.m_bShowSubBossName	= pNpc->m_bShowSubBossName;
			kReq.m_bSiegeMode		= pNpc->m_bSiegeMode;
			kReq.m_bIsRight			= pNpc->m_bIsRight;
			kReq.m_bNoDrop			= pNpc->m_bNoDrop;
			kReq.m_fUnitScale		= pNpc->m_fUnitScale;

#ifdef SERV_BOSS_GAUGE_HP_LINES
			kReq.m_usBossGaugeHPLines = pNpc->m_usBossGaugeHPLines;
#endif SERV_BOSS_GAUGE_HP_LINES

#ifdef SERV_ALLY_NPC
			kReq.m_bAllyNpc			= pNpc->m_bAllyNpc;
#endif SERV_ALLY_NPC

#ifdef MODIFY_DUNGEON_STAGING
			kReq.m_fShowBossNameDelayTime	= pNpc->m_fShowBossNameDelayTime;
			kReq.m_bStopAtStartState		= pNpc->m_bStopAtStartState;
#endif MODIFY_DUNGEON_STAGING

			// START_POS is a list of candidate line indices; one is picked at
			// random, which is what GetNPCData does (XSLDungeon.cpp:296-310).
			if( true == pNpc->m_vecStartPos.empty() )
				kReq.m_nStartPos = 0;
			else
				kReq.m_nStartPos = pNpc->m_vecStartPos[ rand() % pNpc->m_vecStartPos.size() ];

			kReq.m_Level = (char)( bRelative ? ( iRelativeBase + pNpc->m_Level ) : pNpc->m_Level );

			// m_cMonsterGrade stays MG_NORMAL_NPC: the client's NPCData only
			// carries a grade under X2TOOL, so it is not in this build's copy of
			// the script data. The only thing that costs offline is the boss
			// flag on the reward side; m_bHasBossGage still marks bosses for the
			// UI, and drop-table lookups are per monster ID anyway.

			// The offline server owns the UID, exactly as
			// KRoomMonsterManager::CreateMonster does - the script has none.
			const int iNpcUID = m_kRoom.m_iNextNpcUID++;
			kReq.m_UID = iNpcUID;

			m_kRoom.m_mapNpcLevel[ iNpcUID ]	= (int)kReq.m_Level;
			m_kRoom.m_mapNpcID[ iNpcUID ]		= kReq.m_NPCID;
			m_kRoom.m_mapNpcNoDrop[ iNpcUID ]	= kReq.m_bNoDrop;
			m_kRoom.m_mapNpcActive[ iNpcUID ]	= kReq.m_bActive;

			// One line per placed monster. This is the only view into whether a
			// stage's script data actually arrived, and the ACTIVE flag matters
			// most: CC_KILL_ALL_ACTIVE_NPC counts only active monsters, so an
			// inactive one neither blocks a clear nor can be fought.
			CX2OfflineLog::Server(
				L"GAME     +NPC uid=%d id=%d lv=%d sub=%u startPos=%d%s%s",
				iNpcUID, kReq.m_NPCID, (int)kReq.m_Level, (unsigned int)iSub,
				kReq.m_nStartPos,
				( true == kReq.m_bActive ) ? L"" : L" INACTIVE",
				( true == kReq.m_bHasBossGage ) ? L" BOSS" : L"" );

			kList.m_NPCList.push_back( kReq );
		}

		// Inserted even when empty: SetStageStaticNPC keys off the sub-stage
		// index, and a gap would silently shift nothing - but an explicit empty
		// entry keeps the log's sub-stage count honest.
		mapOut.insert( std::make_pair( (int)iSub, kList ) );
	}

	SAFE_DELETE( pDungeon );
}

//////////////////////////////////////////////////////////////////////////
// Per-stage loading.
//
// The ACKs only clear waits. The two _NOTs are what move the client:
//   EGS_DUNGEON_STAGE_LOAD_NOT   sets m_NextStageNum, so OnFrameMove loads it
//   EGS_DUNGEON_STAGE_START_NOT  calls StageStart(), so the stage begins
// Miss either and the dungeon renders and never starts.

bool CX2OfflineServer::Handler_EGS_DUNGEON_STAGE_LOAD_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_DUNGEON_STAGE_LOAD_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	m_kRoom.m_iStageID = kReq.m_iStageID;

	// The NPC tables are deliberately NOT cleared here - see KOfflineRoom.

	KEGS_DUNGEON_STAGE_LOAD_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;
	Reply( kSes, EGS_DUNGEON_STAGE_LOAD_ACK, kAck );

	KEGS_DUNGEON_STAGE_LOAD_NOT kNot;
	kNot.m_iStageID					= kReq.m_iStageID;
	kNot.m_iNumMember				= 1;
	kNot.m_iItemDropEventProbCount	= 0;
	kNot.m_bWithPlayPcBangEvent		= false;
	kNot.m_mapAttribNpcInfo.clear();	///< attribute monsters are not placed offline

	// THIS is where a dungeon's monsters come from - it is not optional and it
	// is not the client's job.
	//
	// CX2DungeonSubStage::SubStageData::m_NPCDataList, which
	// CX2DungeonSubStage::SwapToGameNPCUnit spawns the stage from, has exactly
	// one live source in the shipping client: CX2Dungeon::SetStageStaticNPC,
	// called from the handler for this packet. The client's own NPC_GROUP
	// parser (SubStageData::LoadNPCData) is dead code - see the note next to
	// the call this phase adds back in X2DungeonSubStage.cpp. So an empty
	// m_mapNPCData does not mean "use your own data", it means "this stage has
	// no monsters", which is what the first run of this phase produced.
	BuildStageNpcData( kReq.m_iStageID, kNot.m_mapNPCData );

	int iNpcTotal = 0;
	std::map< int, KNPCList >::const_iterator mit;
	for( mit = kNot.m_mapNPCData.begin(); mit != kNot.m_mapNPCData.end(); ++mit )
		iNpcTotal += (int)mit->second.m_NPCList.size();

	CX2OfflineLog::Server( L"GAME     stage %d loading, %d NPC(s) across %u sub-stage(s)",
		kReq.m_iStageID, iNpcTotal, (unsigned int)kNot.m_mapNPCData.size() );

	if( 0 == iNpcTotal )
	{
		CX2OfflineLog::Server(
			L"GAME     WARNING stage %d has no monsters. Either the dungeon script is missing"
			L" from the .kom, or its sub-stages use the NPC_GROUP_RATE form this phase does not read.",
			kReq.m_iStageID );
	}

	Reply( kSes, EGS_DUNGEON_STAGE_LOAD_NOT, kNot );

	// Phase 6: SQT_FIND_NPC in a dungeon. A stage's static monsters arrive in
	// this packet, not through EGS_NPC_UNIT_CREATE_REQ, so the hook on that
	// handler covers only the ones a script spawns mid-stage.
	{
		std::vector< int > vecNpcID;

		for( mit = kNot.m_mapNPCData.begin(); mit != kNot.m_mapNPCData.end(); ++mit )
		{
			for( size_t i = 0; i < mit->second.m_NPCList.size(); ++i )
				vecNpcID.push_back( mit->second.m_NPCList[i].m_NPCID );
		}

		QuestOnFindNPC( kSes, vecNpcID );
	}

	return true;
}

bool CX2OfflineServer::Handler_EGS_DUNGEON_STAGE_LOAD_COMPLETE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_DUNGEON_STAGE_LOAD_COMPLETE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KPacketOK kAck;
	kAck.m_iOK = NetError::NET_OK;
	Reply( kSes, EGS_DUNGEON_STAGE_LOAD_COMPLETE_ACK, kAck );

	KEGS_DUNGEON_STAGE_LOAD_COMPLETE_NOT kLoaded;
	kLoaded.m_iSlotID	= SLOT_INDEX;
	kLoaded.m_iUnitUID	= m_kRoom.m_nUnitUID;
	Reply( kSes, EGS_DUNGEON_STAGE_LOAD_COMPLETE_NOT, kLoaded );

	// Everyone has loaded the stage, so start it.
	KEGS_DUNGEON_STAGE_START_NOT kStart;
	kStart.m_iStageID = m_kRoom.m_iStageID;

	CX2OfflineLog::Server( L"GAME     stage %d start%s", m_kRoom.m_iStageID,
		( true == kReq.m_bIsSecretStage ) ? L" (secret)" : L"" );

	return Reply( kSes, EGS_DUNGEON_STAGE_START_NOT, kStart );
}

bool CX2OfflineServer::Handler_EGS_DUNGEON_SUB_STAGE_OPEN_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_DUNGEON_SUB_STAGE_OPEN_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_DUNGEON_SUB_STAGE_OPEN_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;
	Reply( kSes, EGS_DUNGEON_SUB_STAGE_OPEN_ACK, kAck );

	KEGS_DUNGEON_SUB_STAGE_OPEN_NOT kNot;
	kNot.m_iSubStageID = kReq.m_iSubStageID;

	return Reply( kSes, EGS_DUNGEON_SUB_STAGE_OPEN_NOT, kNot );
}

bool CX2OfflineServer::Handler_EGS_DUNGEON_SUB_STAGE_GO_NEXT_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_DUNGEON_SUB_STAGE_GO_NEXT_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_DUNGEON_SUB_STAGE_GO_NEXT_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;
	Reply( kSes, EGS_DUNGEON_SUB_STAGE_GO_NEXT_ACK, kAck );

	// "Everyone has walked through" - which offline is true as soon as one has.
	KEGS_DUNGEON_SUB_STAGE_GO_NEXT_ALL_NOT kNot;
	kNot.m_iBeforeStageID	= kReq.m_iBeforeStageID;
	kNot.m_iNextStageID		= kReq.m_iNextStageID;

	return Reply( kSes, EGS_DUNGEON_SUB_STAGE_GO_NEXT_ALL_NOT, kNot );
}

bool CX2OfflineServer::Handler_EGS_DUNGEON_SUB_STAGE_LOAD_COMPLETE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_DUNGEON_SUB_STAGE_LOAD_COMPLETE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_DUNGEON_SUB_STAGE_LOAD_COMPLETE_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;
	Reply( kSes, EGS_DUNGEON_SUB_STAGE_LOAD_COMPLETE_ACK, kAck );

	KEGS_DUNGEON_SUB_STAGE_START_NOT kNot;
	kNot.m_iSubStageID = kReq.m_iSubStageID;

	return Reply( kSes, EGS_DUNGEON_SUB_STAGE_START_NOT, kNot );
}

bool CX2OfflineServer::Handler_EGS_DUNGEON_SUB_STAGE_CLEAR_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_DUNGEON_SUB_STAGE_CLEAR_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// This is the one packet in the phase where "reuse the client's loaded
	// managers" pays off in full. The real server answers it from its own copy
	// of the dungeon graph (CXSLDungeonManager::GetNextStage), deciding what
	// clearing this sub-stage leads to: another sub-stage, the next stage, or
	// the end of the dungeon. The client has that same graph - it is what draws
	// the map - in CX2DungeonSubStage::SubStageData::m_vecClearCondData, where
	// each clear condition carries a weighted list of NextStageData. So read it
	// instead of shipping a duplicate.
	//
	// Handlers run on the thread that called SendPacket, i.e. the render thread,
	// so the game objects below are the caller's own - not a cross-thread read.
	KEGS_DUNGEON_SUB_STAGE_CLEAR_ACK kAck;
	kAck.m_iOK				= NetError::NET_OK;
	kAck.m_iClearType		= (int)CX2DungeonSubStage::CT_GAME;
	kAck.m_iStageIndex		= -1;
	kAck.m_iSubStageIndex	= -1;

	CX2DungeonSubStage::SubStageData* pSubStageData = NULL;

	if( NULL != g_pMain && CX2Main::XS_DUNGEON_GAME == g_pMain->GetNowStateID() &&
		NULL != g_pX2Game )
	{
		CX2DungeonGame* pDungeonGame = static_cast< CX2DungeonGame* >( g_pX2Game );

		if( NULL != pDungeonGame->GetDungeon() &&
			NULL != pDungeonGame->GetDungeon()->GetNowStage() &&
			NULL != pDungeonGame->GetDungeon()->GetNowStage()->GetNowSubStage() )
		{
			pSubStageData = pDungeonGame->GetDungeon()->GetNowStage()->GetNowSubStage()->GetSubStageData();
		}
	}

	bool bResolved = false;

	if( NULL != pSubStageData &&
		kReq.m_iClearConditionIndex >= 0 &&
		kReq.m_iClearConditionIndex < (int)pSubStageData->m_vecClearCondData.size() )
	{
		const CX2DungeonSubStage::ClearCondtionData& kCond =
			pSubStageData->m_vecClearCondData[ kReq.m_iClearConditionIndex ];

		const int iChoices = (int)kCond.m_vecNextStage.size();
		if( iChoices > 0 )
		{
			// Weighted by m_iRate, the way the server picks between a normal
			// route and a secret one. Almost every sub-stage has a single entry
			// at rate 100, so the common case is the first branch.
			int iPick = 0;

			if( iChoices > 1 )
			{
				int iTotal = 0;
				for( int i = 0; i < iChoices; ++i )
					iTotal += kCond.m_vecNextStage[i].m_iRate;

				if( iTotal > 0 )
				{
					int iRoll = rand() % iTotal;
					for( int i = 0; i < iChoices; ++i )
					{
						iRoll -= kCond.m_vecNextStage[i].m_iRate;
						if( iRoll < 0 )
						{
							iPick = i;
							break;
						}
					}
				}
			}

			const CX2DungeonSubStage::NextStageData& kNext = kCond.m_vecNextStage[ iPick ];

			kAck.m_iClearType		= (int)kNext.m_eClearType;
			kAck.m_iStageIndex		= kNext.m_iStageIndex;
			kAck.m_iSubStageIndex	= kNext.m_iSubStageIndex;
			bResolved = true;
		}
	}

	if( false == bResolved )
	{
		// The real server answers ERR_ROOM_51 here, which makes the client skip
		// ClearSubStage entirely - and then nothing advances the dungeon and the
		// player is stuck in a cleared room. CT_GAME with NET_OK ends the run
		// instead: a dungeon that finishes early is recoverable, a dungeon that
		// hangs is not. CT_GAME ignores the two indices.
		CX2OfflineLog::Server(
			L"GAME     WARNING no next-stage data for clear condition %d (stage %d)"
			L" - ending the dungeon instead of hanging",
			kReq.m_iClearConditionIndex, m_kRoom.m_iStageID );
	}

	return Reply( kSes, EGS_DUNGEON_SUB_STAGE_CLEAR_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_TALK_WITH_NPC_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_TALK_WITH_NPC_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Clicking any village NPC sends this and then waits on the ACK
	// ([X2TFieldNpc.cpp:2198](X2Lib/X2TFieldNpc.cpp#L2198)). Leaving it
	// unanswered is not harmless: the wait registered by AddServerPacket times
	// out and the client raises its own "no reply from the server" dialog, which
	// is where the E_SYSTEM_* code on screen comes from. The NPC conversation
	// itself is entirely client-side, so the ACK is all that was ever missing.
	//
	// KGSUser answers NET_OK and then runs two side effects - the quest
	// manager's talk handler and the title manager's OnTalkWithNpc
	// ([GSUserGameCommon.cpp:1660](KncWX2Server/GameServer/GSUserGameCommon.cpp#L1660)).
	// Both of those are phase 6, and QuestOnTalkNPC below is where they land.
	//
	// Its only refusal is ERR_ITEM_14, for talking while a trade or personal
	// shop is open. Neither can happen with one player, so the answer is
	// unconditional.
	KEGS_TALK_WITH_NPC_ACK kAck;
	kAck.m_iOK		= NetError::NET_OK;
	kAck.m_iQuestID	= 0;

	CX2OfflineLog::Server( L"GAME     talked to NPC %d", kReq.m_iNPCID );

	Reply( kSes, EGS_TALK_WITH_NPC_ACK, kAck );

	// The ACK first, then the quest side, matching the server's order at
	// GSUserGameCommon.cpp:1657-1663: the client's wait is cleared before any
	// EGS_UPDATE_QUEST_NOT the conversation produces reaches its UI.
	QuestOnTalkNPC( kSes, kReq.m_iNPCID );

	return true;
}

bool CX2OfflineServer::Handler_EGS_DUNGEON_KILLALLNPC_CHECK_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	// No request body (SendID). m_bResult MUST be true: on false the client
	// flags itself as a hack user and sends EGS_REPORT_HACK_USER_NOT plus
	// EGS_REQUEST_HACKSHIELD_CHECK_NOT (X2StateDungeonGame.cpp:3530-3540).
	// The check exists to stop a client claiming a clear it did not earn, which
	// is not a threat model that survives "trust the client" anyway.
	KEGS_DUNGEON_KILLALLNPC_CHECK_ACK kAck;
	kAck.m_bResult = true;

	if( false == Reply( kSes, EGS_DUNGEON_KILLALLNPC_CHECK_ACK, kAck ) )
		return false;

	// And then the broadcast, which is the half that actually clears the
	// dungeon. KDungeonRoom sends both - the ACK to the asker and then
	// BroadCastID( ERM_DUNGEON_KILLALLNPC_CHECK_NOT ) to every member when the
	// result is true (CenterServer/DungeonRoom.cpp:8886-8893) - and each
	// GameServer forwards that as EGS_DUNGEON_KILLALLNPC_CHECK_NOT
	// (GSUserSecurity.cpp:1722).
	//
	// It matters because SERV_FIX_NONE_NPC_DUNGEON_LINES *is* defined in this
	// build (ServerDefine.h:2645, reachable from X2Lib via CommonPacket.h), so
	// the ClearDungeonGame() call inside the client's own ACK handler is
	// #ifndef'd out (X2StateDungeonGame.cpp:3555) and
	// Handler_EGS_DUNGEON_KILLALLNPC_CHECK_NOT is the only live caller left
	// (:7102). ClearDungeonGame is what sets m_fEndGameDelay = 7.0f and starts
	// the ending event (X2DungeonSubStage.cpp:1223) - and the end-game countdown
	// is the only thing that ever sends EGS_END_GAME_REQ. Reply with the ACK
	// alone and the dungeon simply never ends: the boss is dead, m_fEndGameDelay
	// is still 0 so the countdown block never runs, and the client sits in the
	// cleared room heartbeating forever with no error anywhere.
	ReplyID( kSes, EGS_DUNGEON_KILLALLNPC_CHECK_NOT );

	CX2OfflineLog::Server( L"GAME     all NPCs dead - dungeon clear broadcast sent" );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Combat - where EXP and ED are actually earned

bool CX2OfflineServer::Handler_EGS_NPC_UNIT_CREATE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_NPC_UNIT_CREATE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KPacketOK kAck;
	kAck.m_iOK = NetError::NET_OK;
	Reply( kSes, EGS_NPC_UNIT_CREATE_ACK, kAck );

	// THE BROADCAST IS NOT OPTIONAL, and it is easy to assume it is.
	//
	// "The host spawned the monster, so it already has it" is wrong. The host
	// does NOT create the NPC when it sends this request - CX2Game::CreateNPCReq
	// only fills in a KNPCUnitReq, sends it and arms a 60s wait
	// (X2Game.cpp:6664-6743). The one call to CX2Game::CreateNPC on the
	// shipping client is inside Handler_EGS_NPC_UNIT_CREATE_NOT
	// (X2Game.cpp:6988); the only local-creation path,
	// CX2Game::FlushCreateNPCReq's else branch, is #ifdef X2TOOL. So the
	// server's broadcast is what actually puts monsters in the world, for the
	// host as much as for anyone else.
	//
	// Acknowledging and dropping this therefore produces a dungeon with no
	// monsters in it - and, because CC_KILL_ALL_ACTIVE_NPC is satisfied the
	// moment a sub-stage has zero live NPCs, one that immediately clears itself
	// and fast-forwards through every stage while the camera chases the fades.
	//
	// The server also OWNS THE NPC UID. The client sends m_UID = -1 for every
	// monster (KNPCUnitReq::Init), and KRoomMonsterManager::CreateMonster
	// stamps it from a per-room counter starting at 1
	// (RoomMonsterManager.cpp:44-67). Keying the reward table off the incoming
	// m_UID put every monster in the room under the single key -1.
	KEGS_NPC_UNIT_CREATE_NOT kNot;
	kNot.m_iBattleFieldID = m_kRoom.m_kInfo.m_iBattleFieldID;
	kNot.m_mapAttribNpcInfo.clear();		///< attribute monsters are not placed offline

	for( size_t i = 0; i < kReq.m_vecNPCUnitReq.size(); ++i )
	{
		KNPCUnitNot kNpcNot;
		kNpcNot.m_kNPCUnitReq = kReq.m_vecNPCUnitReq[i];

		// Issue the UID here, then hand the same struct back with it filled in -
		// which is exactly what the CenterServer does (DungeonRoom.cpp:5700).
		const int iNpcUID = m_kRoom.m_iNextNpcUID++;
		kNpcNot.m_kNPCUnitReq.m_UID = iNpcUID;

		// The only place an NPC's level and ID are ever reported.
		// EGS_NPC_UNIT_DIE_REQ carries neither, and both are needed to price the
		// kill - so without this table every monster is worth nothing.
		m_kRoom.m_mapNpcLevel[ iNpcUID ]	= (int)kNpcNot.m_kNPCUnitReq.m_Level;
		m_kRoom.m_mapNpcID[ iNpcUID ]		= kNpcNot.m_kNPCUnitReq.m_NPCID;

		// Phase 27: the two drop gates travel with the monster and are needed
		// at its death, which reports neither.
		m_kRoom.m_mapNpcNoDrop[ iNpcUID ]	= kNpcNot.m_kNPCUnitReq.m_bNoDrop;
		m_kRoom.m_mapNpcActive[ iNpcUID ]	= kNpcNot.m_kNPCUnitReq.m_bActive;

		kNot.m_vecNPCUnitAck.push_back( kNpcNot );
	}

	if( true == kNot.m_vecNPCUnitAck.empty() )
		return true;

	CX2OfflineLog::Server( L"GAME     spawning %u NPC(s), UIDs %d..%d",
		(unsigned int)kNot.m_vecNPCUnitAck.size(),
		kNot.m_vecNPCUnitAck.front().m_kNPCUnitReq.m_UID,
		kNot.m_vecNPCUnitAck.back().m_kNPCUnitReq.m_UID );

	Reply( kSes, EGS_NPC_UNIT_CREATE_NOT, kNot );

	// Phase 6: SQT_FIND_NPC completes on *seeing* an NPC, so the spawn is the
	// event, not the kill. GSUserRoomCommon.cpp:1810 does the same off the back
	// of this broadcast.
	{
		std::vector< int > vecNpcID;
		for( size_t i = 0; i < kNot.m_vecNPCUnitAck.size(); ++i )
			vecNpcID.push_back( kNot.m_vecNPCUnitAck[i].m_kNPCUnitReq.m_NPCID );

		QuestOnFindNPC( kSes, vecNpcID );
	}

	return true;
}

bool CX2OfflineServer::Handler_EGS_NPC_UNIT_DIE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_NPC_UNIT_DIE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_NPC_UNIT_DIE_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;
	Reply( kSes, EGS_NPC_UNIT_DIE_ACK, kAck );

	int iNpcID		= 0;
	int iNpcLevel	= 0;

	std::map< int, int >::const_iterator mit = m_kRoom.m_mapNpcID.find( kReq.m_nDieNPCUID );
	if( mit != m_kRoom.m_mapNpcID.end() )
		iNpcID = mit->second;

	mit = m_kRoom.m_mapNpcLevel.find( kReq.m_nDieNPCUID );
	if( mit != m_kRoom.m_mapNpcLevel.end() )
		iNpcLevel = mit->second;

	int iEXP = 0;
	int iED  = 0;

	// Logged unconditionally, because the interesting failure is a kill that
	// pays nothing and there is no way to tell from the outside whether the
	// cause was the die state, an unknown UID, or an empty drop-table row.
	static const wchar_t* const DIE_STATE[] =
		{ L"KILL_BY_USER", L"KILL_SELF", L"ALLY", L"ESCAPE", L"BONUS_DROP" };

	const int iDieState = (int)kReq.m_cNpcDieState;

	CX2OfflineLog::Server( L"GAME     NPC uid=%d id=%d lv=%d died: %s",
		kReq.m_nDieNPCUID, iNpcID, iNpcLevel,
		( iDieState >= 0 && iDieState < 5 ) ? DIE_STATE[ iDieState ] : L"?" );

	// Only a kill by a player pays out. NDS_KILL_SELF / NDS_ESCAPE /
	// NDS_BONUS_DROP are a monster removing itself, and the real server's
	// bNoDrop path skips the reward for those too.
	if( KEGS_NPC_UNIT_DIE_REQ::NDS_KILL_BY_USER == kReq.m_cNpcDieState ||
		KEGS_NPC_UNIT_DIE_REQ::NDS_ALLY == kReq.m_cNpcDieState )
	{
		// A battlefield's monsters come out of a different table in the file,
		// keyed by battlefield ID rather than dungeon ID + difficulty.
		const bool bBattleField = ( 0 != m_kRoom.m_kInfo.m_iBattleFieldID );
		const int  iKey			= bBattleField
									? m_kRoom.m_kInfo.m_iBattleFieldID
									: ( m_kRoom.m_kInfo.m_iDungeonID +
										(int)m_kRoom.m_kInfo.m_DifficultyLevel );

		CX2OfflineDropTable::Instance()->GetNpcReward( iKey, bBattleField, iNpcID, iNpcLevel, iEXP, iED );

		// A field scales both rewards. SetBattleFieldEXPFactor / EDFactor come out
		// of BattleFieldServerData.lua; without that file the live US values stand
		// in, so the numbers are right even unpacked.
		if( true == bBattleField )
		{
			CX2OfflineBattleField* pBF = CX2OfflineBattleField::Instance();
			iEXP = (int)( (float)iEXP * pBF->GetEXPFactor() );
			iED  = (int)( (float)iED  * pBF->GetEDFactor() );
		}

		m_kRoom.m_iRewardEXP += iEXP;
		++m_kRoom.m_iKillNPCNum;

		// m_iRewardED is now only a running total of what the run *offered*,
		// for the log line below. The money itself is credited when a coin is
		// picked up - see PushNpcDrop.
		m_kRoom.m_iRewardED  += iED;

		CX2OfflineLog::Server( L"GAME     -> +%d exp, +%d ED (key=%d%s, run total %d exp / %d ED)",
			iEXP, iED, iKey, bBattleField ? L" battlefield" : L"",
			m_kRoom.m_iRewardEXP, m_kRoom.m_iRewardED );

		if( 0 == iEXP && 0 == iED )
		{
			CX2OfflineLog::Server(
				L"GAME     -> NOTE monster %d has no reward row for key %d, and none on the"
				L" wildcard key either - it simply grants nothing, which is common", iNpcID, iKey );
		}

		// Pay the EXP now, not at the end of the run. The real server does it
		// here: KGSUser's ERM_NPC_UNIT_DIE_NOT handler calls
		// m_kEXP.AddExp( SumEXP() ) and then CheckCharLevelUp() on every single
		// monster (GSUserRoomCommon.cpp:1959-1963).
		//
		// Phase 4 accumulated into m_iRewardEXP and paid out at EGS_END_GAME_REQ
		// or EGS_LEAVE_ROOM_REQ instead, which works for a dungeon and is silently
		// broken for a field: a field is never "ended" and never "left" - the
		// player walks out through EGS_STATE_CHANGE_FIELD_REQ - so every point of
		// field EXP was accumulated and then dropped on the floor. The character
		// could grind all day and never level.
		//
		// It looked like it was working because the client adds the EXP to its own
		// unit data as the packet arrives (CX2Game::ProcessExpListByNpcUnitDie,
		// X2Game.cpp:7498) - so the bar filled up and then stuck at full, since
		// only the server ever grants the level.
		if( iEXP > 0 )
		{
			int iOldLevel = 0;
			const int iNewLevel = ApplyDungeonReward( m_kRoom.m_nUnitUID, iEXP, 0,
													  &iOldLevel );

			if( iNewLevel > iOldLevel )
			{
				// The level-up itself: new level, new stats, refilled HP, and the
				// effect over the character. Safe everywhere.
				PushLevelUp( kSes, m_kRoom.m_nUnitUID );

				// And the rest of the character - EXP and the skill points the
				// level just paid - but only in a field. CX2StateBattleField hands
				// EGS_UPDATE_UNIT_INFO_NOT to the battlefield game and it just
				// resets the unit (X2StateBattleField.cpp:1518), whereas
				// CX2StateDungeonGame's handler also sets
				// m_bReceive_KEGS_UNIT_INFO_UPDATE - one of the two flags that let
				// a dungeon be left - so sending it mid-dungeon would poke at the
				// leave gate for no reason. A dungeon resends the character at the
				// result screen a moment later anyway.
				if( 0 != m_kRoom.m_kInfo.m_iBattleFieldID )
					PushUnitInfoUpdate( kSes, m_kRoom.m_nUnitUID );

				// Phase 6: SQT_CHAR_LEVEL_UP, and the title missions a level
				// opens.
				QuestOnLevelUp( kSes );
			}
		}

		// The loot: ED as coins, plus whatever the item lottery drew. Phase 4
		// left both out - items because there was no inventory to hold them,
		// ED because it was credited invisibly instead.
		PushNpcDrop( kSes, kReq.m_nDieNPCUID, iNpcID, iED, kReq.m_DiePos );

		// Phase 6: hunt sub-quests and hunt sub-missions. Only a real kill
		// counts, which is why this is inside the reward branch rather than
		// beside the field bookkeeping below - the server charges the kill to
		// the quest from the same ERM_NPC_UNIT_DIE_NOT that pays the EXP.
		QuestOnNpcDie( kSes, iNpcID );
	}

	// Field bookkeeping: free the cap slot, queue the respawn, add the danger and
	// roll for the middle boss. Deliberately outside the reward branch above -
	// the server charges danger and schedules a refill for *any* death, including
	// a monster that killed itself, and only the reward cares about who landed
	// the blow.
	if( 0 != m_kRoom.m_kInfo.m_iBattleFieldID )
		OnFieldNpcDie( kReq.m_nDieNPCUID );

	// The client shows the EXP it just earned from this packet, and adds it to
	// its own unit data (CX2Game::ProcessExpListByNpcUnitDie). An entry with
	// m_iEXP == 0 is not the same as no entry: the client prints "no experience
	// gained" for the former and nothing at all for the latter, which is the
	// distinction the real server draws too.
	KEGS_NPC_UNIT_DIE_NOT kNot;
	kNot.m_nDieNPCUID	= kReq.m_nDieNPCUID;
	kNot.m_iNPCID		= iNpcID;
	kNot.m_cAttUnitType	= kReq.m_cAttUnitType;
	kNot.m_uiAttUnit	= kReq.m_uiAttUnit;

	if( iEXP > 0 )
	{
		KEXPData kEXPData;
		kEXPData.m_iEXP = iEXP;
		kNot.m_EXPList[ m_kRoom.m_nUnitUID ] = kEXPData;
	}

	return Reply( kSes, EGS_NPC_UNIT_DIE_NOT, kNot );
}

bool CX2OfflineServer::Handler_EGS_WORLD_TRIGGER_RELOCATION_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_WORLD_TRIGGER_RELOCATION_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// The same trap as EGS_NPC_UNIT_CREATE_NOT, a third time: the broadcast is
	// the only thing that performs the action.
	//
	// CX2Game::Handler_EGS_WORLD_TRIGGER_RELOCATION_REQ (X2Game.cpp:14779) sends
	// this and does NOT touch the world - the local ActiveTrigger() call next to
	// it is #if defined(WORLD_TOOL) || defined(X2TOOL). The only live call to
	// CX2World::ActiveTrigger is inside Handler_EGS_WORLD_TRIGGER_RELOCATION_NOT
	// (X2Game.cpp:14813).
	//
	// World triggers are how a dungeon script moves the world: opening a gate,
	// dropping a bridge, relocating a blocker. Answering the ACK alone leaves
	// the geometry exactly as it was, so the player walks into a wall that
	// should have opened - which reads as "I cannot move" rather than as a
	// missing packet.
	KEGS_WORLD_TRIGGER_RELOCATION_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;
	Reply( kSes, EGS_WORLD_TRIGGER_RELOCATION_ACK, kAck );

	KEGS_WORLD_TRIGGER_RELOCATION_NOT kNot;
	kNot.m_iWorldTrigger = kReq.m_iWorldTrigger;

	CX2OfflineLog::Server( L"GAME     world trigger %d activated", kReq.m_iWorldTrigger );

	return Reply( kSes, EGS_WORLD_TRIGGER_RELOCATION_NOT, kNot );
}

/*static*/ int CX2OfflineServer::EDCoinItemID( int iED )
{
	// KDropTable::GetEDItemID (KDropTable.cpp:1166). The three coin IDs are
	// client-side constants too - BRONZE_ED_ITEM_ID / SILVER / GOLD at
	// X2Lib/X2Define.h:1082-1084 - so this needs nothing that is not shipped.
	if( iED <= 0 )		return 0;
	if( iED <= 50 )		return BRONZE_ED_ITEM_ID;
	if( iED <= 150 )	return SILVER_ED_ITEM_ID;

	return GOLD_ED_ITEM_ID;
}

/// The dungeons the event drop is switched off in.
///
/// CXSLDungeon's own list, as DungeonRoom.cpp:6379 spells it: the El Forest
/// Gate, any tutorial dungeon, any training-camp dungeon. Transcribed against
/// the CLIENT's DI_* enum for the same reason IsSkillUseCountedDungeon
/// (Handlers_Skill.cpp) is - most of the names CXSLDungeon lists are commented
/// out of X2Lib/X2Dungeon.h and would not compile. Every enumerator below is
/// one that file already uses, so all four are known-live rather than
/// grep-matched.
///
/// The same three exclusions appear twice on live, once as this gate and once
/// inside CXSLDungeon::IsItemDropDungeon, which refuses items 91620 and 91630
/// in exactly these dungeons - and those two are the event table's own first
/// entries. Two spellings of one rule; one is enough here.
static bool IsEventDropDungeon( int iDungeonID )
{
	switch( iDungeonID )
	{
	case CX2Dungeon::DI_EL_FOREST_GATE_NORMAL:
	case CX2Dungeon::DI_TUTORIAL_ELSWORD:
	case CX2Dungeon::DI_BATTLE_FIELD_TUTORIAL_ELSWORD:
		return false;

	default:
		break;
	}

	// CXSLDungeon::IsTCDungeon, spelled the way Handlers_Skill.cpp spells it:
	// one contiguous range whose upper bound is written as RAVEN_0 + 6 because
	// this client's enum stops at RAVEN_0.
	if( iDungeonID >= (int)CX2Dungeon::DI_TRAINING_FREE &&
		iDungeonID <= (int)CX2Dungeon::DI_TRAINING_RAVEN_0 + 6 )
	{
		return false;
	}

	return true;
}

void CX2OfflineServer::PushNpcDrop( KOfflineSession& kSes, int iNpcUID, int iNpcID, int iED,
									const VECTOR3& kDiePos )
{
	if( iNpcID <= 0 )
		return;

	// Phase 27: NO_DROP suppresses the whole payout, not just the monster's own
	// row. DungeonRoom.cpp:6348 `continue`s past every drop roll AND past the
	// quest-item roll AND past the ED coins for a no-drop monster, so all three
	// go behind this gate.
	//
	// It did not matter until now, because the only item source was the
	// monster's own drop-table row and scenery has none. The static drop is
	// keyed by DUNGEON: every checker, prop, Luto and quest NPC in the room
	// would otherwise roll it and cough up potions when it despawned.
	{
		std::map< int, bool >::const_iterator mitND = m_kRoom.m_mapNpcNoDrop.find( iNpcUID );
		if( mitND != m_kRoom.m_mapNpcNoDrop.end() && true == mitND->second )
		{
			CX2OfflineLog::Server( L"DROP     monster %d (uid=%d) is NO_DROP - no payout",
				iNpcID, iNpcUID );
			return;
		}
	}

	const bool bBattleField = ( 0 != m_kRoom.m_kInfo.m_iBattleFieldID );
	const int  iKey			= bBattleField
								? m_kRoom.m_kInfo.m_iBattleFieldID
								: ( m_kRoom.m_kInfo.m_iDungeonID +
									(int)m_kRoom.m_kInfo.m_DifficultyLevel );

	std::vector<int> vecItemID;
	CX2OfflineDropTable::Instance()->GetNpcItemDrop( iKey, bBattleField, iNpcID, vecItemID );

	//////////////////////////////////////////////////////////////////////////
	// Phase 27: the drop that belongs to the PLACE.
	//
	// This is step 2 of the server's four (DungeonRoom.cpp:6362,
	// BattleFieldRoom.cpp:1985), and until now the offline server did only step
	// 1. It is where every ordinary consumable comes from: "Aqua" (99811) has a
	// static row in nearly every dungeon in the game at 5-10%, a battlefield
	// static row in every field at 4%, and in the whole of DropTable.lua it
	// appears in four AddToGroup lines of which exactly one group is referenced
	// by any monster - and that monster is 3016. Hence ISSUES.md #17: not "drops
	// are broken", but "one entire drop source was never implemented".
	{
		std::vector<int> vecStatic;
		CX2OfflineDropTable::Instance()->GetStaticDrop( iKey, bBattleField, vecStatic );

		for( size_t i = 0; i < vecStatic.size(); ++i )
		{
			// Tagged, because the combined list below cannot say where an item
			// came from and that is exactly the question this phase's exit test
			// asks. Aqua showing up here is the whole fix.
			CX2OfflineLog::Server( L"DROP     static drop for key %d%s: item %d",
				iKey, bBattleField ? L" battlefield" : L"", vecStatic[i] );

			vecItemID.push_back( vecStatic[i] );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Step 3, the event drop. Dungeons only - BattleFieldRoom has no equivalent
	// call - and only for an ACTIVE monster in a dungeon the event is allowed
	// in.
	if( false == bBattleField &&
		true == IsEventDropDungeon( m_kRoom.m_kInfo.m_iDungeonID ) )
	{
		std::map< int, bool >::const_iterator mitA = m_kRoom.m_mapNpcActive.find( iNpcUID );

		// Absent means the monster predates the tracking, not that it is
		// inactive. ACTIVE defaults to true in the script
		// (X2DungeonSubStage.cpp:1535), so an unknown UID is treated as active.
		const bool bActive = ( mitA == m_kRoom.m_mapNpcActive.end() ) ? true : mitA->second;

		if( true == bActive )
		{
			std::vector<int> vecEvent;
			CX2OfflineDropTable::Instance()->GetEventDrop( vecEvent );

			for( size_t i = 0; i < vecEvent.size(); ++i )
			{
				CX2OfflineLog::Server( L"DROP     event drop in dungeon %d: item %d",
					m_kRoom.m_kInfo.m_iDungeonID, vecEvent[i] );

				vecItemID.push_back( vecEvent[i] );
			}
		}
	}

	// Phase 6: the quest items an active collection quest asks for. These do not
	// come out of the drop table at all - on live they are rolled per user from
	// the quests that user is carrying (KRoomUser::GetQuestDropItemInDungeon),
	// which is why a monster that drops nothing for one player drops a quest
	// item for another. Appended to the same list so they scatter from the same
	// corpse and are picked up by the same code.
	{
		KOfflineUnitRow kQuestRow;
		if( true == LoadQuestState( kSes, kQuestRow ) )
		{
			std::vector<int> vecQuestItem;

			CX2OfflineQuest::Instance()->GetQuestItemDrops(
				m_kRoom.m_kInfo.m_iDungeonID, m_kRoom.m_kInfo.m_DifficultyLevel,
				m_kRoom.m_kInfo.m_iBattleFieldID, iNpcID, kQuestRow, vecQuestItem );

			for( size_t i = 0; i < vecQuestItem.size(); ++i )
				vecItemID.push_back( vecQuestItem[i] );
		}
	}

	KEGS_DROP_ITEM_NOT kNot;
	kNot.m_CreatePos = kDiePos;

	//////////////////////////////////////////////////////////////////////////
	// The ED, as coins.
	//
	// KDropTable::NpcDropItem scatters an ordinary kill's ED across [2,3] coins
	// and a boss's across [10,20], each worth the total divided by the count
	// (KDropTable.cpp:1242-1251). Phase 4 skipped this and credited the ED at
	// kill time instead, which was invisible: the floor stayed empty and the
	// money simply grew. Coins are how the player is told a kill paid.
	//
	// The ED is NOT credited here any more. It is credited when a coin is
	// picked up, which means walking past one loses it - exactly as on live.
	if( iED > 0 )
	{
		const int iCoinNum	= ( rand() % 2 ) + 2;
		const int iPerCoin	= iED / iCoinNum;

		if( iPerCoin > 0 )
		{
			for( int i = 0; i < iCoinNum; ++i )
			{
				const int iDropUID = m_kRoom.m_iNextDropUID++;

				m_kRoom.m_mapDropED[ iDropUID ] = iPerCoin;

				KDropItemData kData;
				kData.m_iItemID			= EDCoinItemID( iPerCoin );
				kData.m_iDropItemUID	= iDropUID;
				kData.m_bLeft			= ( 0 == ( rand() % 2 ) );
				kData.m_fSpeed			= 100.0f + (float)( rand() % 100 );
				kData.m_cEnchantLevel	= 0;

				kNot.m_DropItemDataList.push_back( kData );
			}

			CX2OfflineLog::Server( L"DROP     monster %d dropped %d ED as %d coin(s) of %d",
				iNpcID, iED, iCoinNum, iPerCoin );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// The item lottery.
	for( size_t i = 0; i < vecItemID.size(); ++i )
	{
		// The offline server owns the drop UID, exactly as it owns NPC UIDs -
		// the client learns the number from this packet and quotes it back in
		// EGS_GET_ITEM_REQ.
		const int iDropUID = m_kRoom.m_iNextDropUID++;

		m_kRoom.m_mapDropItem[ iDropUID ] = vecItemID[i];

		KDropItemData kData;
		kData.m_iItemID			= vecItemID[i];
		kData.m_iDropItemUID	= iDropUID;

		// Which way it bounces and how fast. The real server randomises both
		// (KRoom's drop scatter); the numbers are cosmetic - the item lands
		// wherever the client's own physics puts it and the pickup is by UID.
		kData.m_bLeft			= ( 0 == ( rand() % 2 ) );
		kData.m_fSpeed			= 100.0f + (float)( rand() % 100 );

		// No random enhancement on drops. SiCXSLEnchantItemManager()->
		// RandomEnchant reads the enchant event tables, which are server data
		// with no client copy, so every drop comes out at +0 rather than at an
		// invented level.
		kData.m_cEnchantLevel	= 0;

		kNot.m_DropItemDataList.push_back( kData );

		CX2OfflineLog::Server( L"DROP     monster %d dropped item %d (dropUID=%d)",
			iNpcID, vecItemID[i], iDropUID );
	}

	if( true == kNot.m_DropItemDataList.empty() )
		return;

	// The _NOT is the only thing that puts an item on the floor:
	// CX2Game::Handler_EGS_DROP_ITEM_NOT is the sole caller of
	// CX2DropItemManager::AddDropItem (X2Game.cpp:8315-8327).
	Reply( kSes, EGS_DROP_ITEM_NOT, kNot );
}

bool CX2OfflineServer::Handler_EGS_GET_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_GET_ITEM_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// The client waits 60s on this ACK, so it is answered first and
	// unconditionally - a pickup that cannot be honoured must still not hang.
	KEGS_GET_ITEM_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	Reply( kSes, EGS_GET_ITEM_ACK, kAck );

	// An ED coin first. It is not an inventory item: picking one up converts it
	// to money, so the whole insert path below is skipped.
	std::map< int, int >::iterator eit = m_kRoom.m_mapDropED.find( kReq.m_iDropItemUID );
	if( eit != m_kRoom.m_mapDropED.end() )
	{
		const int iED = eit->second;

		m_kRoom.m_mapDropED.erase( eit );
		m_kRoom.m_iCollectedED += iED;

		KOfflineUnitRow kRow;
		if( true == CX2OfflineDB::Instance()->LoadUnit( kSes.m_nSelectedUnitUID, kRow ) )
		{
			kRow.m_iED += iED;

			CX2OfflineDB::Instance()->SaveProgress( kRow.m_nUnitUID, kRow.m_iLevel,
				kRow.m_iEXP, kRow.m_iED );
		}

		// m_mapGetED is what the client adds to its own unit's ED and prints in
		// the chat log - CX2Game::ProcessGetEDItem (X2Game.cpp:16932). It is
		// keyed by unit UID, and CX2Game reads only its own unit's entry.
		KEGS_GET_ITEM_NOT kEDNot;
		kEDNot.m_GetUnitUID		= kSes.m_nSelectedUnitUID;
		kEDNot.m_iItemID		= EDCoinItemID( iED );
		kEDNot.m_iDropItemUID	= kReq.m_iDropItemUID;
		kEDNot.m_cGetItemType	= KEGS_GET_ITEM_NOT::GIT_DROP_ITEM;
		kEDNot.m_bIsItemSuccess	= true;
		kEDNot.m_iTotalED		= iED;
		kEDNot.m_cEnchantLevel	= 0;

		KDungeonRewardED kRewardED;
		kRewardED.m_iED			= iED;
		kRewardED.m_iBonusED	= 0;

		kEDNot.m_mapGetED[ kSes.m_nSelectedUnitUID ] = kRewardED;

		Reply( kSes, EGS_GET_ITEM_NOT, kEDNot );

		CX2OfflineLog::Server( L"DROP     picked up %d ED (dropUID=%d, %d ED this run)",
			iED, kReq.m_iDropItemUID, m_kRoom.m_iCollectedED );

		return true;
	}

	// Which item this drop UID was. The request also carries the client's own
	// m_kItemInfo, and the real server ignores it in favour of the room's
	// record; so does this. A UID that is not in the map has either been picked
	// up already or was never dropped by us - either way there is nothing to
	// give, and saying so is better than trusting the request.
	std::map< int, int >::iterator mit = m_kRoom.m_mapDropItem.find( kReq.m_iDropItemUID );
	if( mit == m_kRoom.m_mapDropItem.end() )
	{
		CX2OfflineLog::Server( L"DROP     pickup of dropUID=%d ignored - no such drop on the floor"
			L" (already collected, or the client asked for something it invented)",
			kReq.m_iDropItemUID );

		return true;
	}

	const int iItemID = mit->second;

	// Removed before the insert, so a repeated request cannot duplicate the
	// item even if the insert below fails.
	m_kRoom.m_mapDropItem.erase( mit );

	// The display half. CX2Game::Handler_EGS_GET_ITEM_NOT looks the item up in
	// its own drop manager by UID, prints the pickup and removes the sprite, so
	// this has to go out even though it carries no inventory data.
	//
	// m_mapGetED stays empty and m_iTotalED zero: the kill's ED was already
	// credited in Handler_EGS_NPC_UNIT_DIE_REQ out of the drop table, and the
	// real server pays it here instead by dropping ED coins. Doing both would
	// pay twice.
	KEGS_GET_ITEM_NOT kNot;
	kNot.m_GetUnitUID		= kSes.m_nSelectedUnitUID;
	kNot.m_iItemID			= iItemID;
	kNot.m_iDropItemUID		= kReq.m_iDropItemUID;
	kNot.m_cGetItemType		= KEGS_GET_ITEM_NOT::GIT_DROP_ITEM;
	kNot.m_bIsItemSuccess	= true;
	kNot.m_iTotalED			= 0;
	kNot.m_cEnchantLevel	= 0;

	Reply( kSes, EGS_GET_ITEM_NOT, kNot );

	// The inventory half. Separate packet, and it is the one that actually adds
	// the item - EGS_GET_ITEM_NOT only draws the pickup.
	CX2OfflineInventory* pInven = CX2OfflineInventory::Instance();

	std::vector< KInventoryItemInfo > vecChanged;
	int iInserted = 0;

	if( false == pInven->InsertItem( iItemID, 1, 0, vecChanged, iInserted ) )
	{
		// A full bag. The real server parks the item in a temp inventory the
		// player empties later; that is a whole subsystem
		// (KTempInventory plus EGS_GET_ITEM_TEMP_INVENTORY_NOT and its own
		// screen) and is not implemented, so the item is lost - which is what
		// the log says, rather than the pickup silently doing nothing.
		CX2OfflineLog::Server( L"DROP     item %d could not be picked up - the inventory is full,"
			L" and the temp inventory is not implemented offline, so it is lost", iItemID );

		return true;
	}

	KEGS_GET_ITEM_REALTIME_NOT kRealtime;
	kRealtime.m_vecKInventorySlotInfo = vecChanged;

	Reply( kSes, EGS_GET_ITEM_REALTIME_NOT, kRealtime );

	CX2OfflineLog::Server( L"DROP     picked up item %d (dropUID=%d)",
		iItemID, kReq.m_iDropItemUID );

	// Phase 6: the bag just changed, which is the only way an item-collection
	// step ever becomes satisfied. The quest side needs nothing - the client
	// recounts its own inventory to draw the step, and the completion check
	// counts ours - but a title mission with a collection step has no other
	// moment at which anything would notice it is done.
	QuestOnInventoryChanged( kSes );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Ending the run

void CX2OfflineServer::PushUnitInfoUpdate( KOfflineSession& kSes, UidType nUnitUID )
{
	KOfflineUnitRow kRow;
	if( false == CX2OfflineDB::Instance()->LoadUnit( nUnitUID, kRow ) )
		return;

	KEGS_UPDATE_UNIT_INFO_NOT kUpdate;
	MakeUnitInfoFromRow( kUpdate.m_kUnitInfo, kRow );
	kUpdate.m_bGameEnd = false;			///< true is what ends a run; this is not one
	kUpdate.m_vecKInventorySlotInfo.clear();
	kUpdate.m_mapItemObtained.clear();

	Reply( kSes, EGS_UPDATE_UNIT_INFO_NOT, kUpdate );
}

int CX2OfflineServer::ApplyDungeonReward( UidType nUnitUID, int iAddEXP, int iAddED,
										 OUT int* piOldLevel /*= NULL*/ )
{
	KOfflineUnitRow kRow;
	if( false == CX2OfflineDB::Instance()->LoadUnit( nUnitUID, kRow ) )
		return 0;

	const int iOldLevel = kRow.m_iLevel;

	if( NULL != piOldLevel )
		*piOldLevel = iOldLevel;

	int iEXP = kRow.m_iEXP + iAddEXP;
	int iED  = kRow.m_iED  + iAddED;

	if( iEXP < 0 )	iEXP = 0;		///< only reachable through overflow
	if( iED  < 0 )	iED  = 0;

	// Level up off the client's own EXP table - the same table
	// MakeDefaultUnitInfo already reads, and the same one the client uses to
	// draw the EXP bar. Duplicating it would be the one way to get a character
	// whose level the client disagrees with.
	int iLevel = kRow.m_iLevel;

	if( NULL != g_pData && NULL != g_pData->GetEXPTable() )
	{
		const int iMaxLevel = (int)_CONST_X2GAME_::g_iMaxLevel;

		while( iLevel < iMaxLevel &&
			   iEXP >= g_pData->GetEXPTable()->GetEXPData( iLevel + 1 ).m_nTotalExp )
		{
			++iLevel;
		}

		// At the cap the EXP bar stops rather than wrapping, which is what
		// KGSUser does too (its reward is zeroed once GetLimitsLevel is hit).
		if( iLevel >= iMaxLevel )
			iEXP = g_pData->GetEXPTable()->GetEXPData( iMaxLevel ).m_nTotalExp;
	}

	CX2OfflineDB::Instance()->SaveProgress( nUnitUID, iLevel, iEXP, iED );

	if( iLevel > iOldLevel )
	{
		CX2OfflineLog::Server( L"REWARD   unitUID=%I64d LEVEL UP %d -> %d",
			(__int64)nUnitUID, iOldLevel, iLevel );

		// The skill points those levels are worth. KGSUser does this in the
		// same place (GSUserFunction.cpp:7404-7415), one level at a time out of
		// the skill tree's own curve; SkillPointForLevelUp is the client's copy
		// of that curve, so the two agree by construction rather than by luck.
		//
		// Without this a character levels for ever and can never learn
		// anything, which is the sort of thing that looks like a broken skill
		// UI rather than a missing reward.
		const int iAddSP = CX2OfflineSkill::SkillPointForLevelUp( iOldLevel, iLevel );

		if( iAddSP > 0 )
		{
			kRow.m_iSP += iAddSP;

			CX2OfflineDB::Instance()->SaveSkillPoint( nUnitUID, kRow.m_iSP, kRow.m_iCSP );

			CX2OfflineLog::Server( L"REWARD   unitUID=%I64d +%d SP (%d total)",
				(__int64)nUnitUID, iAddSP, kRow.m_iSP );
		}
	}

	CX2OfflineLog::Server( L"REWARD   unitUID=%I64d +%d exp (%d total), +%d ED (%d total), lv=%d",
		(__int64)nUnitUID, iAddEXP, iEXP, iAddED, iED, iLevel );

	return iLevel;
}

bool CX2OfflineServer::Handler_EGS_END_GAME_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_END_GAME_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KPacketOK kAck;
	kAck.m_iOK = NetError::NET_OK;
	Reply( kSes, EGS_END_GAME_ACK, kAck );

	if( false == m_kRoom.m_bActive )
		return true;

	const bool bWin = ( (int)CX2Room::TN_RED == kReq.m_iWinTeam );

	// Order matters here, and it is the order the real pair of servers happens
	// to produce: the result data first, then the updated character, then the
	// room. CX2StateDungeonResult reads g_pData->GetDungeonResultInfo() in its
	// constructor, and the state change to XS_DUNGEON_RESULT is driven by the
	// client's own timer a few seconds later - but the EXP bar on that screen
	// animates from m_nOldEXP to the unit's current EXP, so the unit has to be
	// updated before the screen is built, not after.

	KOfflineUnitRow kBefore;
	CX2OfflineDB::Instance()->LoadUnit( m_kRoom.m_nUnitUID, kBefore );

	const int iAddEXP = bWin ? m_kRoom.m_iRewardEXP : 0;

	// The ED the player actually collected, which is already in the bank - the
	// coin pickups credited it as they happened. It is carried here only so the
	// result screen can show what the run paid.
	const int iAddED  = m_kRoom.m_iCollectedED;

	// Result_DUNGEON's clear bonus: 30% of what the run earned, on a win only
	// (ResultProcess.cpp:2054). The rank bonus is deliberately not applied -
	// the rank thresholds are server-side Lua this phase does not reproduce, so
	// every rank comes back as the default and a bonus computed from it would be
	// a fabricated number rather than a missing one.
	const int iClearBonusEXP = bWin ? (int)( iAddEXP * 0.3f ) : 0;

	// Only the clear bonus is paid here. iAddEXP is already in the character -
	// every kill credited itself as it happened - so adding it again would pay
	// the run twice. ED is 0 for the same reason: the coins already paid it.
	const int iNewLevel = ApplyDungeonReward( m_kRoom.m_nUnitUID, iClearBonusEXP, 0 );

	// The clear bonus can be the thing that levels the character, and the result
	// screen's own level-up animation keys off CX2Unit::GetIsLevelUp - which only
	// this packet ever sets. Without it a level earned on the last hit of a run
	// appears silently.
	if( iNewLevel > kBefore.m_iLevel )
	{
		PushLevelUp( kSes, m_kRoom.m_nUnitUID );
		QuestOnLevelUp( kSes );
	}

	if( true == bWin && 0 != m_kRoom.m_kInfo.m_iDungeonID )
	{
		// The difficulty is part of the key, not a separate column. The client
		// unlocks against `m_iDungeonID + m_cDifficulty`
		// (X2StateDungeonGame.cpp:3006), and CX2DungeonManager::IsActiveDungeon
		// looks a prerequisite up by that same number - so storing the base ID
		// alone would credit the wrong dungeon on anything above the lowest
		// difficulty. Phase 4 wrote the base ID; it only ever agreed because
		// every run so far has been on difficulty 0.
		CX2OfflineDB::Instance()->AddDungeonClear( m_kRoom.m_nUnitUID,
			m_kRoom.m_kInfo.m_iDungeonID + (int)m_kRoom.m_kInfo.m_DifficultyLevel, 0 );

		// Phase 6: clear-count, clear-time, rank and damage sub-quests, and the
		// same four title-mission types.
		//
		// The rank passed is RT_NONE, because the offline server does not
		// compute one - the thresholds are server-side Lua this project does not
		// reproduce, which is the same reason the result screen's rank bonus is
		// left out (see the clear-bonus comment above). The consequence is
		// visible and one-directional: a sub-quest that asks for rank D or
		// better will not tick. Passing an invented rank would tick it, which is
		// worse - it would hand out a reward the run did not earn.
		//
		// The play time is the same figure the result screen shows, and the
		// damage is the hit count the client reported for itself.
		const int iPlayTime = ( 0 != m_kRoom.m_dwPlayStartTick )
								? (int)( ( ::GetTickCount() - m_kRoom.m_dwPlayStartTick ) / 1000 )
								: 0;

		QuestOnDungeonClear( kSes, (char)CX2DungeonRoom::RT_NONE, iPlayTime,
							 m_kRoom.m_kPlayResult.m_nDamageCount );
	}

	//////////////////////////////////////////////////////////////////////////
	// the result screen's data
	KEGS_END_GAME_DUNGEON_RESULT_DATA_NOT kResult;
	kResult.m_bIsWin			= bWin;
	kResult.m_iDungeonID		= m_kRoom.m_kInfo.m_iDungeonID;
	kResult.m_cDifficulty		= m_kRoom.m_kInfo.m_DifficultyLevel;
	kResult.m_cDungeonMode		= m_kRoom.m_kInfo.m_cDungeonMode;
	kResult.m_bIsHenirDungeon	= false;
	kResult.m_iDungeonEventPoint= 0;
	kResult.m_iHenirEventPoint	= 0;
	kResult.m_bIsClearDefenceDungeon	= false;
	kResult.m_iProtectedSmallCrystal	= 0;
	kResult.m_bComeBackUserInParty		= false;
	kResult.m_bSpecChar1		= false;
	kResult.m_bSpecChar2		= false;

	kResult.m_nSec = ( 0 != m_kRoom.m_dwPlayStartTick )
						? (int)( ( ::GetTickCount() - m_kRoom.m_dwPlayStartTick ) / 1000 )
						: 0;

	KDungeonUnitResultInfo kUnitResult;
	kUnitResult.m_UnitUID			= m_kRoom.m_nUnitUID;
	// THIS FLAG DECIDES WHETHER THE DUNGEON COUNTS AS CLEARED, and reading it
	// off the play result alone was wrong.
	//
	// EGS_MY_USER_UNIT_INFO_TO_SERVER_REQ - the packet that carries the play
	// result - arrives AFTER this one. Measured, not guessed: the client sent
	// EGS_END_GAME_REQ at 20:39:57.425 and its play result at 20:39:57.438, so
	// m_kPlayResult is still Clear()ed here and m_fHP is 0. That made every
	// successful run report the player as dead.
	//
	// And the client acts on it: CX2StateDungeonGame::Handler_EGS_END_GAME_
	// DUNGEON_RESULT_DATA_NOT only calls AddClearDungeon when
	// m_bIsWin && false == bDieMyUnit (X2StateDungeonGame.cpp:3005-3015). With
	// m_bIsDie true it silently skipped the clear, so CX2Unit::m_mapDungeonClear
	// stayed empty and CX2DungeonManager::IsActiveDungeon kept every dungeon
	// gated on this one locked - which is exactly the reported symptom, the
	// second Ruben dungeon still locked after Banthus was beaten.
	//
	// So the play result is only believed when it actually arrived. Without one
	// there is no evidence of a death, and a run that reached EGS_END_GAME_REQ
	// with a win is evidence of the opposite.
	kUnitResult.m_bIsDie			= ( true == m_kRoom.m_bHavePlayResult &&
										m_kRoom.m_kPlayResult.m_fHP <= 0.0f );
	kUnitResult.m_nTotalScore		= m_kRoom.m_kPlayResult.m_TotalScore;
	kUnitResult.m_nComboScore		= m_kRoom.m_kPlayResult.m_ComboScore;
	kUnitResult.m_nTechnicalScore	= m_kRoom.m_kPlayResult.m_TechScore;
	kUnitResult.m_nDamaged			= m_kRoom.m_kPlayResult.m_nDamageCount;
	// kBefore was read after the kills had already credited their EXP, so the
	// "before" figure has to have the run's EXP taken back off - same correction
	// as the ED line below, and for the same reason.
	kUnitResult.m_nOldEXP			= kBefore.m_iEXP - iAddEXP;

	// kBefore was read after the coins were credited, so the "before" ED has to
	// have them taken back off or the result screen animates from the finished
	// total to itself.
	kUnitResult.m_nOldED			= kBefore.m_iED - iAddED;
	kUnitResult.m_nED				= iAddED;
	kUnitResult.m_nEXP				= iAddEXP;
	kUnitResult.m_nClearBonusEXP	= iClearBonusEXP;
	kUnitResult.m_nKillNPCNum		= m_kRoom.m_iKillNPCNum;
	kUnitResult.m_bIsMVP			= false;		///< MVP needs a party of three

	kResult.m_vecDungeonUnitInfo.push_back( kUnitResult );
	kResult.m_mapHaveExpInDungeon[ m_kRoom.m_nUnitUID ] = ( iAddEXP > 0 );

	Reply( kSes, EGS_END_GAME_DUNGEON_RESULT_DATA_NOT, kResult );

	//////////////////////////////////////////////////////////////////////////
	// the character, with its new EXP, ED and level
	KOfflineUnitRow kAfter;
	if( true == CX2OfflineDB::Instance()->LoadUnit( m_kRoom.m_nUnitUID, kAfter ) )
	{
		KEGS_UPDATE_UNIT_INFO_NOT kUpdate;
		MakeUnitInfoFromRow( kUpdate.m_kUnitInfo, kAfter );
		kUpdate.m_bGameEnd = true;
		kUpdate.m_vecKInventorySlotInfo.clear();
		kUpdate.m_mapItemObtained.clear();

		Reply( kSes, EGS_UPDATE_UNIT_INFO_NOT, kUpdate );
	}

	//////////////////////////////////////////////////////////////////////////
	// and the room
	m_kRoom.m_kInfo.m_RoomState = (char)CX2Room::RS_RESULT;

	KEGS_END_GAME_NOT kNot;
	kNot.m_cRoomState	= (char)CX2Room::RS_RESULT;
	kNot.m_iWinTeam		= kReq.m_iWinTeam;
	MakeRoomSlots( kAfter, (int)CX2Room::SS_PLAY, kNot.m_vecSlot );

	CX2OfflineLog::Server( L"GAME     dungeonID=%d %s in %ds: %d kill(s), +%d exp (+%d clear bonus), +%d ED, lv=%d",
		m_kRoom.m_kInfo.m_iDungeonID, bWin ? L"CLEARED" : L"FAILED",
		kResult.m_nSec, m_kRoom.m_iKillNPCNum, iAddEXP, iClearBonusEXP, iAddED, iNewLevel );

	// The run is banked; a second EGS_END_GAME_REQ (the client guards against
	// sending one, but a re-entered room would) must not pay out twice.
	// m_iCollectedED goes with them so a repeat cannot re-report ED either -
	// it is only a display figure now, but a doubled one would still be a lie
	// on the result screen.
	m_kRoom.m_iRewardEXP	= 0;
	m_kRoom.m_iRewardED		= 0;
	m_kRoom.m_iCollectedED	= 0;

	return Reply( kSes, EGS_END_GAME_NOT, kNot );
}

bool CX2OfflineServer::Handler_EGS_STATE_CHANGE_RESULT_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_STATE_CHANGE_RESULT_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	KEGS_STATE_CHANGE_RESULT_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;
	Reply( kSes, EGS_STATE_CHANGE_RESULT_ACK, kAck );

	KOfflineUnitRow kRow;
	if( false == m_kRoom.m_bActive ||
		false == CX2OfflineDB::Instance()->LoadUnit( m_kRoom.m_nUnitUID, kRow ) )
		return true;

	// The _NOT is what moves the client to XS_DUNGEON_RESULT
	// (CX2DungeonGame::Handler_EGS_STATE_CHANGE_RESULT_NOT); the ACK only clears
	// the wait.
	// SS_PLAY, not SS_WAIT: KRoomSlot::GetRoomSlotInfo clamps a user in
	// S_RESULT to SS_PLAY (RoomSlot.cpp:71-74), and SS_WAIT is the one state
	// CX2Game::AddUserUnit treats as "nobody there".
	KEGS_STATE_CHANGE_RESULT_NOT kNot;
	kNot.m_cRoomState = (char)CX2Room::RS_RESULT;
	MakeRoomSlots( kRow, (int)CX2Room::SS_PLAY, kNot.m_vecSlot );

	return Reply( kSes, EGS_STATE_CHANGE_RESULT_NOT, kNot );
}

bool CX2OfflineServer::Handler_EGS_RESULT_SUCCESS_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_RESULT_SUCCESS_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// m_kGamePlayNetworkInfo is the relay/P2P telemetry the studio collected;
	// there is nothing to collect from a loopback peer.
	KEGS_RESULT_SUCCESS_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_iDungeonID	= m_kRoom.m_kInfo.m_iDungeonID;
	Reply( kSes, EGS_RESULT_SUCCESS_ACK, kAck );

	KOfflineUnitRow kRow;
	if( false == m_kRoom.m_bActive ||
		false == CX2OfflineDB::Instance()->LoadUnit( m_kRoom.m_nUnitUID, kRow ) )
		return true;

	// "Everybody has pressed OK on the result screen", which offline is true as
	// soon as one player has. The return-to-field info tells the client whose
	// turn it is to leave; with one occupant that is always this one.
	KEGS_RESULT_ALL_UNIT_SUCCESS_NOT kNot;
	MakeRoomInfo( kNot.m_RoomInfo );
	MakeRoomSlots( kRow, (int)CX2Room::SS_PLAY, kNot.m_vecSlot );

	kNot.m_kReturnToFieldInfo.m_iReturnToFieldUnitUID	= m_kRoom.m_nUnitUID;
	kNot.m_kReturnToFieldInfo.m_iRoomUID				= m_kRoom.m_kInfo.m_RoomUID;
	kNot.m_kReturnToFieldInfo.m_setRemainUnitUID.clear();

	return Reply( kSes, EGS_RESULT_ALL_UNIT_SUCCESS_NOT, kNot );
}

bool CX2OfflineServer::Handler_EGS_LEAVE_GAME_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	// No request body (SendID). The room survives - this is "leave the match",
	// not "leave the room", and EGS_LEAVE_ROOM_REQ follows.
	KEGS_LEAVE_GAME_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_LEAVE_GAME_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////
// Dying, and the result screen's reward box.
//
// These are here because every one of them arms an AddServerPacket wait, and
// an unanswered wait is what turns "the character died" into "the client hangs
// on a black screen". They were found by diffing the set of ACKs the client
// waits on against the set of requests this file answers, rather than by
// running into them - a dungeon run that never loses a life would not have
// reached most of them.

bool CX2OfflineServer::Handler_EGS_USER_UNIT_DIE_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_USER_UNIT_DIE_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// m_fReBirthTime is only read on the PvP path; a dungeon death is driven by
	// the client's own timer. m_EXP / m_VP are the PvP death penalty, which
	// dungeons do not have.
	KEGS_USER_UNIT_DIE_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_fReBirthTime	= 0.0f;
	kAck.m_EXP			= 0;
	kAck.m_VP			= 0;
	Reply( kSes, EGS_USER_UNIT_DIE_ACK, kAck );

	KEGS_USER_UNIT_DIE_NOT kNot;
	kNot.m_KillerNPCUID					= kReq.m_KillerNPCUID;
	kNot.m_KillerUserUnitUID			= kReq.m_KillerUserUnitUID;
	kNot.m_MaxDamageKillerUserUnitUID	= kReq.m_MaxDamageKillerUserUnitUID;
	kNot.m_KilledUserUnitUID			= m_kRoom.m_nUnitUID;

	CX2OfflineLog::Server( L"GAME     unitUID=%I64d died (killer NPC UID=%d)",
		(__int64)m_kRoom.m_nUnitUID, kReq.m_KillerNPCUID );

	return Reply( kSes, EGS_USER_UNIT_DIE_NOT, kNot );
}

bool CX2OfflineServer::Handler_EGS_USER_UNIT_DIE_COMPLETE_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	// No request body (SendID).
	KEGS_USER_UNIT_DIE_COMPLETE_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;
	Reply( kSes, EGS_USER_UNIT_DIE_COMPLETE_ACK, kAck );

	KEGS_USER_UNIT_DIE_COMPLETE_NOT kNot;
	kNot.m_UnitUID = m_kRoom.m_nUnitUID;

	return Reply( kSes, EGS_USER_UNIT_DIE_COMPLETE_NOT, kNot );
}

bool CX2OfflineServer::Handler_EGS_RESURRECT_TO_CONTINUE_DUNGEON_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_RESURRECT_TO_CONTINUE_DUNGEON_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Refused, deliberately, with the code the real server uses when the
	// character has no resurrection stone (GSUserDungeon.cpp:1515). There are
	// none offline - resurrection stones are inventory, which is phase 5 - so
	// answering NET_OK would hand out free unlimited continues, which is a
	// balance change dressed up as emulation. The refusal is safe: the client's
	// handler just clears m_bIsSendResurrectionReq and the normal death path
	// carries on.
	KEGS_RESURRECT_TO_CONTINUE_DUNGEON_ACK kAck;
	kAck.m_iOK						= NetError::ERR_RESURRECT_00;
	kAck.m_iDungeonID				= m_kRoom.m_kInfo.m_iDungeonID;
	kAck.m_iNumResurrectionStone	= 0;
	kAck.m_iNumAutoPaymentResStone	= 0;
	kAck.m_bUseCash					= false;

	CX2OfflineLog::Server( L"GAME     refused resurrect (no stones until phase 5)%s",
		( true == kReq.m_bUseCash ) ? L" - cash request" : L"" );

	return Reply( kSes, EGS_RESURRECT_TO_CONTINUE_DUNGEON_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_STOP_DUNGEON_CONTINUE_TIME_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_STOP_DUNGEON_CONTINUE_TIME_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Pauses the "continue?" countdown after a death. Nothing to pause - the
	// countdown is the client's - but the ACK has to echo m_bIsStop back.
	KEGS_STOP_DUNGEON_CONTINUE_TIME_ACK kAck;
	kAck.m_iOK		= NetError::NET_OK;
	kAck.m_bIsStop	= kReq.m_bIsStop;

	return Reply( kSes, EGS_STOP_DUNGEON_CONTINUE_TIME_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_DUNGEON_SECRET_STAGE_ENTER_CHECK_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	// No request body (SendID). Whether the secret-stage pad may be used. Yes:
	// the client already decided the pad is there, and the check exists to stop
	// a client claiming a stage it has not unlocked.
	KEGS_DUNGEON_SECRET_STAGE_ENTER_CHECK_ACK kAck;
	kAck.m_iOK					= NetError::NET_OK;
	kAck.m_bCanEnterSecretStage	= true;

	return Reply( kSes, EGS_DUNGEON_SECRET_STAGE_ENTER_CHECK_ACK, kAck );
}

bool CX2OfflineServer::Handler_EGS_START_REWARD_BOX_SELECT_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	// No request body, and the ACK carries no payload either - the client's
	// handler takes no lParam. It only clears the wait; the box UI opens on
	// EGS_START_REWARD_BOX_SELECT_NOT, which is not sent because there is
	// nothing in the boxes until item drops land in phase 5.
	return ReplyID( kSes, EGS_START_REWARD_BOX_SELECT_ACK );
}

bool CX2OfflineServer::Handler_EGS_SELECT_REWARD_BOX_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_SELECT_REWARD_BOX_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// Answered so the click is not a hang. The box's contents would arrive in
	// EGS_SELECT_REWARD_BOX_NOT, which needs an inventory - phase 5.
	KEGS_SELECT_REWARD_BOX_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	return Reply( kSes, EGS_SELECT_REWARD_BOX_ACK, kAck );
}

#endif SERV_IRUHADEV_OFFLINE
