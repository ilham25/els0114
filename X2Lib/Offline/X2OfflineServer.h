#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-08-31
// Description: Offline mode - the in-process server emulator.
//
//              It implements IX2OfflineHook, so KSession hands it every
//              outbound KEvent instead of serializing and sending it, and it
//              answers by calling KSession::QueueingEvent() on the same
//              session - the exact point the real receive path queues into.
//              Everything above the socket layer runs unmodified.
//
//              Phase 1: no SQLite. Login chain plus one hardcoded character.
//              Phase 2: SQLite behind it (X2OfflineDB) and real character CRUD.
//              Phase 3: real base stats (X2OfflineStatTable) and field entry.
//              Phase 4: rooms, the tutorial dungeon, and dungeon results
//                       (X2OfflineDropTable for the EXP and ED numbers).
//              Phase 5: inventory, equipment and skills (X2OfflineInventory
//                       and X2OfflineSkill), and the item drops phase 4
//                       deliberately deferred until there was an inventory to
//                       drop them into.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

#include "OfflineHook.h"
#include "X2OfflineLog.h"
#include "X2OfflineDB.h"
#include "X2OfflineStatTable.h"
#include "X2OfflineDropTable.h"
#include "X2OfflineResolveTable.h"
#include "X2OfflineEnchantTable.h"
#include "X2OfflineAttribTable.h"
#include "X2OfflineSocketTable.h"
#include "X2OfflineBattleField.h"
#include "X2OfflineInventory.h"
#include "X2OfflineSkill.h"
#include "X2OfflineQuest.h"
#include "X2OfflineTitle.h"
#include "X2OfflineCashShop.h"
#include "X2OfflineIgnore.h"

class CX2OfflineServer : public IX2OfflineHook
{
public:
	enum PROXY_KIND
	{
		PK_UNKNOWN = 0,
		PK_GAME,			///< CX2ServerProtocol::GSPROXY_ID
		PK_RELAY,			///< CX2ServerProtocol::RTPROXY_ID
		PK_CHANNEL,			///< CX2ServerProtocol::CHPROXY_ID
		PK_COLLECT,			///< CX2ServerProtocol::CSPROXY_ID
	};

	/// as much of KncWX2Server/Common/GSFSMState_def.h as offline mode needs
	enum FSM_STATE
	{
		S_INIT = 0,
		S_UID_UPDATED,
		S_LOGINED,
		S_SERVER_SELECT,
		S_FIELD_MAP,
		S_ROOM,
	};

	struct KOfflineSession
	{
		KSession*		m_pSession;
		PROXY_KIND		m_eKind;
		FSM_STATE		m_eState;
		UidType			m_nUserUID;
		UidType			m_nSelectedUnitUID;

		/// Last gauge values written to SQLite, so the three-second
		/// EGS_UPDATE_PLAY_STATUS_NOT only touches the file when something
		/// actually moved. -1 means "nothing written yet on this session".
		int				m_iSavedHP;
		int				m_iSavedMP;
		int				m_iSavedHyper;
		int				m_iSavedAbil;
		std::wstring	m_wstrLoginID;
		std::wstring	m_wstrPassport;

		/// Which pet EGS_SUMMON_PET_REQ last summoned - 0 when none is out.
		/// EGS_FEED_PETS_REQ names only the food item, not the pet, so this is
		/// the only place the offline server can look up which one to feed.
		UidType			m_nSummonedPetUID;

		/// Phase 28. Set once by RestoreSummonedPet (from Handler_EGS_SELECT_
		/// UNIT_REQ) when a persisted pet needs to be re-spawned, and cleared by
		/// SendPendingPetRestore the first time EGS_FIELD_LOADING_COMPLETE_REQ
		/// runs afterwards - which may be a village or a dungeon, whichever the
		/// character enters first. That handler fires on every field transition
		/// for the life of the session, so this flag is what keeps the restore
		/// a one-shot instead of re-spawning the pet on every later map change.
		bool			m_bPetRestorePending;

		/// KGSUser::m_kTimer[TM_RESOLVE_JACKPOT] / m_dResolveJackpotTime
		/// (GSUserInventory.cpp:2894-2944), collapsed into one absolute
		/// deadline instead of an elapsed-time/threshold pair: dismantling is
		/// a jackpot attempt once _time64(NULL) reaches this. 0 is guaranteed
		/// to have already passed, so - matching the live server, whose
		/// m_dResolveJackpotTime member starts at its default 0.0 - the very
		/// first dismantle after this session starts is always a jackpot
		/// attempt.
		__int64			m_tNextJackpotAt;

		KOfflineSession()
			: m_pSession( NULL )
			, m_eKind( PK_UNKNOWN )
			, m_eState( S_INIT )
			, m_nUserUID( 0 )
			, m_nSelectedUnitUID( 0 )
			, m_iSavedHP( -1 )
			, m_iSavedMP( -1 )
			, m_iSavedHyper( -1 )
			, m_iSavedAbil( -1 )
			, m_nSummonedPetUID( 0 )
			, m_bPetRestorePending( false )
			, m_tNextJackpotAt( 0 )
		{
		}
	};

	//////////////////////////////////////////////////////////////////////////
	/// The one room, and the one dungeon run inside it.
	///
	/// This lives on the server rather than on KOfflineSession for a reason
	/// that is easy to get wrong: OnClientSend takes a *copy* of the session,
	/// hands it to the handler and writes it back, so anything stored there is
	/// copied twice per packet. That is fine for eleven ints; it is not fine
	/// for the NPC table, which holds one entry per spawned monster and is
	/// written by EGS_NPC_UNIT_CREATE_REQ and read by every
	/// EGS_NPC_UNIT_DIE_REQ - the two busiest packets in a dungeon.
	///
	/// Offline there is exactly one player, so there is exactly one room. No
	/// key, no map, no lifetime problem.
	struct KOfflineRoom
	{
		bool			m_bActive;

		/// The room as the client will see it. Kept whole rather than picked
		/// apart into scalars, because every ACK that carries a room carries
		/// this exact struct - and because the request the client sent already
		/// filled most of it in (room name, play time, difficulty, get-item
		/// type), so echoing it back is both less code and more faithful than
		/// rebuilding it field by field.
		KRoomInfo		m_kInfo;

		UidType			m_nUnitUID;				///< whose room it is

		/// Is this the tutorial room? Set only by
		/// Handler_EGS_CREATE_TUTORIAL_ROOM_REQ, which is the only thing that
		/// makes one.
		///
		/// This exists because the client's own flags cannot answer the question
		/// at the time it has to be asked. Phase 4 gated the dungeon-leave fix
		/// on CX2Main::GetIsExitingTutorial(), which turned out to be wrong
		/// twice over: it is set to true when the tutorial is left
		/// (X2StateDungeonGame.cpp:1448) and cleared only on the way back to
		/// character select or the PvP lobby, so it stays true for the whole
		/// session and suppressed the fix in every later dungeon. Its sibling
		/// GetIsPlayingTutorial() is no use either - the ESC dialog clears it
		/// one line *before* sending the leave request. The room's own identity
		/// is the only thing that is still true when the request arrives.
		bool			m_bTutorial;

		/// NPC UID -> the level and ID the client reported when it asked for the
		/// monster. KEGS_NPC_UNIT_DIE_REQ carries neither, and both are needed
		/// to price the kill.
		///
		/// Never cleared mid-run, deliberately. Clearing on a stage change
		/// looked tidier and loses EXP: a monster killed at the very end of a
		/// stage can report its death after the next stage's
		/// EGS_DUNGEON_STAGE_LOAD_REQ has already arrived, and would then find
		/// no row. A few hundred ints for a whole dungeon is not worth that.
		std::map< int, int >	m_mapNpcLevel;
		std::map< int, int >	m_mapNpcID;

		/// NPC UID -> the two drop gates the client reported with the monster
		/// (phase 27). Both come off KNPCUnitReq, which is the only place they
		/// exist: NO_DROP and ACTIVE are fields of the dungeon script's NPC
		/// block (X2DungeonSubStage.cpp:1535,1553), defaulting to false and
		/// true respectively.
		///
		/// They were not needed while the only item drop was the monster's own
		/// row - scenery has no row, so it dropped nothing anyway. The static
		/// drop is keyed by DUNGEON, so without m_bNoDrop every prop, checker
		/// and quest NPC in the room would roll it and the floor would fill
		/// with Aqua. m_bActive gates the event drop only, exactly as
		/// DungeonRoom.cpp:6600 does.
		std::map< int, bool >	m_mapNpcNoDrop;
		std::map< int, bool >	m_mapNpcActive;

		/// The offline server owns NPC UIDs, exactly as
		/// KRoomMonsterManager::CreateMonster does: the client sends -1 for
		/// every monster and reads the real UID back out of
		/// EGS_NPC_UNIT_CREATE_NOT. Monotonic for the life of the room, so a
		/// stale UID can never alias a live monster.
		int				m_iNextNpcUID;

		/// Drop item UID -> item ID, for the items lying on the floor (phase 5).
		/// The offline server owns these UIDs for the same reason it owns NPC
		/// UIDs: EGS_GET_ITEM_REQ names one, and the client only knows the
		/// number because EGS_DROP_ITEM_NOT told it.
		///
		/// An entry is removed when the item is picked up, so a second
		/// EGS_GET_ITEM_REQ for the same drop finds nothing and duplicates
		/// nothing. Whatever is left over when the room closes was simply never
		/// collected.
		std::map< int, int >	m_mapDropItem;
		int				m_iNextDropUID;

		/// Drop item UID -> the ED that coin is worth, for the ED coins a kill
		/// scatters. Kept apart from m_mapDropItem because a coin is not an
		/// inventory item: picking one up converts it to money and nothing
		/// lands in the bag.
		std::map< int, int >	m_mapDropED;

		/// ED actually picked up this run. The result screen shows this rather
		/// than m_iRewardED, because uncollected coins are money the player
		/// walked past - which is how the real game behaves.
		int				m_iCollectedED;

		/// Accumulated across the run, mirroring KRoomUser::AddRewardEXP /
		/// AddRewardED on the CenterServer.
		int				m_iRewardEXP;
		int				m_iRewardED;
		int				m_iKillNPCNum;

		/// What the client reported about its own performance, from
		/// EGS_MY_USER_UNIT_INFO_TO_SERVER_REQ. Echoed back in the result
		/// packet: the offline server has no way to second-guess it and no
		/// reason to want one.
		KDungeonPlayResultInfo	m_kPlayResult;
		bool			m_bHavePlayResult;

		int				m_iStageID;

		/// A field is populated once, on the first packet that proves the
		/// player's unit exists. See PushFieldNpcs.
		bool			m_bFieldNpcSent;

		//////////////////////////////////////////////////////////////////////////
		/// Field respawn and escalation. All of this is the CenterServer's
		/// KBattleFieldMonsterManager plus KBattleFieldGameManager, reduced to
		/// one field with one player in it.

		/// Which spawn group each live monster belongs to, so a death can be
		/// charged against the right group's cap and queued to respawn into it.
		std::map< int, int >	m_mapNpcGroup;

		/// Live monster counts, per group and per (group, npc). The spawn picker
		/// reads both to honour MAX_NUMBER_OF_MONSTER_IN_THIS_GROUP and
		/// MAX_NUMBER_OF_THIS_MONSTER.
		std::map< int, int >							m_mapAliveByGroup;
		std::map< std::pair< int, int >, int >			m_mapAliveByNpc;

		/// Dead monsters waiting to come back: group ID and the tick it is due.
		/// Keyed by the dead monster's UID purely to match the server's shape;
		/// the UID itself is never reused.
		struct KRespawn
		{
			int		m_iGroupID;
			DWORD	m_dwDueTick;

			KRespawn() : m_iGroupID( 0 ), m_dwDueTick( 0 ) {}
		};
		std::map< int, KRespawn >	m_mapRespawn;

		/// The danger value, and its value before the last kill. The middle-boss
		/// curve is a function of both, because it fires on crossing a threshold
		/// rather than on exceeding one.
		int				m_iDangerValue;
		int				m_iOldDangerValue;

		/// A reserved middle boss has been rolled for but not yet placed; an
		/// alive one blocks any further roll until it dies.
		bool			m_bMiddleBossReserved;
		int				m_iMiddleBossAlive;
		DWORD			m_dwPlayStartTick;

		KOfflineRoom()
		{
			Clear();
		}

		void Clear()
		{
			m_bActive			= false;
			m_kInfo.Initialize();
			m_nUnitUID			= 0;
			m_bTutorial			= false;
			m_mapNpcLevel.clear();
			m_mapNpcID.clear();
			m_mapNpcNoDrop.clear();
			m_mapNpcActive.clear();
			m_iNextNpcUID		= 1;			///< RoomMonsterManager.cpp:20 seeds it the same way
			m_mapDropItem.clear();
			m_iNextDropUID		= 1;
			m_mapDropED.clear();
			m_iCollectedED		= 0;
			m_iRewardEXP		= 0;
			m_iRewardED			= 0;
			m_iKillNPCNum		= 0;
			m_kPlayResult.Clear();
			m_bHavePlayResult	= false;
			m_iStageID			= 0;
			m_bFieldNpcSent		= false;
			m_mapNpcGroup.clear();
			m_mapAliveByGroup.clear();
			m_mapAliveByNpc.clear();
			m_mapRespawn.clear();
			m_iDangerValue		= 0;
			m_iOldDangerValue	= 0;
			m_bMiddleBossReserved = false;
			m_iMiddleBossAlive	= 0;
			m_dwPlayStartTick	= 0;
		}
	};

	//////////////////////////////////////////////////////////////////////////
	/// What a finished quest still owes the client once its ACK has gone out:
	/// the level-up effect, the quest updates the reward triggered, and the
	/// title work. Carried between CompleteOneQuest and AfterQuestComplete
	/// rather than passed as five arguments, because the batch hand-in has to
	/// stash one of these per quest and replay them after a single ACK.
	struct KQuestAfter
	{
		bool	m_bValid;
		int		m_iQuestID;
		int		m_iOldLevel;
		int		m_iNewLevel;
		bool	m_bChangedJob;

		KQuestAfter()
			: m_bValid( false )
			, m_iQuestID( 0 )
			, m_iOldLevel( 0 )
			, m_iNewLevel( 0 )
			, m_bChangedJob( false )
		{
		}
	};

public:
	static CX2OfflineServer*	Instance();
	static void					Release();

	//////////////////////////////////////////////////////////////////////////
	// IX2OfflineHook
	virtual bool OnClientSend( KSession* pSession, const KEvent& kEvent );
	virtual void OnSessionConnect( KSession* pSession, const char* szIP, unsigned short usPort );
	virtual void OnSessionClose( KSession* pSession );

	//////////////////////////////////////////////////////////////////////////
	// helpers every handler uses
	template< typename T > bool ReadReq( const KEvent& kEvent, T& kOut );
	template< typename T > bool Reply( KOfflineSession& kSes, unsigned short usEventID, const T& kData );
	bool ReplyID( KOfflineSession& kSes, unsigned short usEventID );

	/// "YYYY-MM-DD HH:MM:SS" - the only date format KncUtil::ConvertStringToCTime parses
	static std::wstring NowString();

	/// The one place a KUnitInfo gets built. Every later phase reuses it.
	static void MakeDefaultUnitInfo( KUnitInfo& kOut,
									 UidType nOwnerUserUID,
									 UidType nUnitUID,
									 char cUnitClass,
									 const std::wstring& wstrNickName,
									 int iLevel );

	/// MakeDefaultUnitInfo plus everything the `unit` row persists, including
	/// the soft-delete state the character-select screen renders its restore
	/// and final-delete UI from.
	static void MakeUnitInfoFromRow( KUnitInfo& kOut, const KOfflineUnitRow& kRow );

	/// The live HP/MP the client's HUD is seeded from. Sent in
	/// EGS_SELECT_UNIT_4_NOT; without it CX2GageManager never receives a max HP
	/// and the health bar renders empty.
	static void MakeGamePlayStatus( const KOfflineUnitRow& kRow, OUT KGamePlayStatus& kOut );

	/// Base stat for a row plus whatever its equipped gear adds - the value
	/// that belongs in KUnitInfo::m_kGameStat and KRoomUserInfo::m_kGameStat.
	/// Phase 3 and 4 used the base stat alone because there was no gear; now
	/// there is, and every place that sends a game stat has to go through here
	/// or a weapon stops mattering the moment the player leaves the village.
	static void MakeGameStat( const KOfflineUnitRow& kRow, OUT KStat& kOut );

	static const wchar_t* KindStr( PROXY_KIND eKind );

	/// Which server a session stands in for, derived from the event ID's own
	/// namespace. See the comment on the definition for why KActorProxy::
	/// GetClassID() must not be read from X2Lib.
	static PROXY_KIND KindFromEventID( unsigned short usEventID );

private:
	CX2OfflineServer();
	virtual ~CX2OfflineServer();

	/// returns true if a handler consumed the packet, false -> log it UNHANDLED
	bool Dispatch( KOfflineSession& kSes, const KEvent& kEvent );

	/// Dispatch() behind an __except, so one bad packet logs and the process
	/// keeps running (phase 8).
	///
	/// __try, not try/catch. X2Lib's US_SERVICE configuration sets no /EH
	/// switch at all, so a C++ catch(...) - which is what this used to be -
	/// cannot be relied on, and the fault that actually happens in a packet
	/// handler is an access violation rather than a throw. SEH sees both: a
	/// C++ throw reaches the filter as exception code 0xE06D7363.
	///
	/// Kept in a function of its own because MSVC refuses __try in any
	/// function that needs object unwinding, and OnClientSend has the
	/// transaction guard. Nothing with a destructor may be added here.
	///
	/// dwExceptionCode comes back non-zero if the filter fired.
	bool DispatchProtected( KOfflineSession& kSes, const KEvent& kEvent,
							OUT unsigned long& dwExceptionCode );

	/// Checkpoint the WAL and write els_db.sql.bak, then log the packet census.
	/// Driven by EGS_CLIENT_QUIT_REQ setting m_bQuitRequested - the work itself
	/// has to happen after the dispatch transaction has committed, so it cannot
	/// be done inside the handler.
	void OnCleanShutdown();

	/// Resolve (or create) the single offline account this login ID maps to,
	/// caching it on the server and on the session.
	bool EnsureAccount( KOfflineSession& kSes, const std::wstring& wstrLoginID );

	//////////////////////////////////////////////////////////////////////////
	// Handlers_Login.cpp
	bool Handler_ECH_VERIFY_ACCOUNT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_ECH_GET_SERVERGROUP_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_ECH_GET_CHANNEL_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_ECH_DISCONNECT_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	bool Handler_EGS_CONNECT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_VERIFY_ACCOUNT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CHECK_MACHINE_ID_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_STATE_CHANGE_SERVER_SELECT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CURRENT_TIME_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_SELECT_SERVER_SET_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_KEYBOARD_MAPPING_INFO_WRITE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DISCONNECT_FOR_SERVER_SELECT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CHECK_BALANCE_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	//////////////////////////////////////////////////////////////////////////
	// Handlers_Unit.cpp
	bool Handler_EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CREATE_UNIT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DELETE_UNIT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_FINAL_DELETE_UNIT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_RESTORE_UNIT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_SELECT_UNIT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_MY_INVENTORY_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	/// The five SERV_SELECT_UNIT_PACKET_DIVISION notifications, pushed in order
	/// before EGS_SELECT_UNIT_ACK.
	void PushSelectUnitNotifications( KOfflineSession& kSes, const KOfflineUnitRow& kRow );

	//////////////////////////////////////////////////////////////////////////
	// Handlers_Field.cpp
	bool Handler_EGS_STATE_CHANGE_FIELD_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_FIELD_LOADING_COMPLETE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_OPTION_UPDATE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_UPDATE_PLAY_STATUS_NOT( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_FIELD_UNIT_SYNC_DATA_NOT( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_UPDATE_BATTLE_FIELD_USER_POS_NOT( KOfflineSession& kSes, const KEvent& kEvent );

	//////////////////////////////////////////////////////////////////////////
	// Handlers_Square.cpp - Cobo Express, the trade square. Not a KOfflineRoom:
	// it carries no dungeon/battlefield semantics and does not survive a relog.
	bool Handler_EGS_SQUARE_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_JOIN_SQUARE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_LEAVE_SQUARE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_SQUARE_UNIT_SYNC_DATA_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	//////////////////////////////////////////////////////////////////////////
	// Handlers_Room.cpp - rooms, the dungeon run inside one, and its result

	/// entering a room
	bool Handler_EGS_CREATE_TUTORIAL_ROOM_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CREATE_ROOM_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	/// Solo dungeon entry - the one the dungeon button on the local map uses,
	/// and the only way into a normal dungeon. Needs no party: see the handler.
	bool Handler_EGS_QUICK_START_DUNGEON_GAME_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	bool Handler_EGS_JOIN_BATTLE_FIELD_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_ROOM_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_LEAVE_ROOM_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	/// room options, all cosmetic offline but all waited on.
	/// (EGS_CHANGE_DUNGEON_GET_ITEM_TYPE_REQ is absent on purpose:
	/// NOT_USE_DICE_ROLL is defined in this build, so both the packet struct and
	/// the client's sender are compiled out and it can never arrive.)
	bool Handler_EGS_CHANGE_DUNGEON_DIFFICULTY_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CHANGE_MOTION_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	/// starting and loading the game
	bool Handler_EGS_STATE_CHANGE_GAME_START_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GAME_LOADING_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_MY_USER_UNIT_INFO_TO_SERVER_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_BATTLE_FIELD_NPC_LOAD_COMPLETE_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	/// per-stage loading. The two _NOTs are what actually drive the client
	/// forward; the ACKs only clear its waits.
	bool Handler_EGS_DUNGEON_STAGE_LOAD_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DUNGEON_STAGE_LOAD_COMPLETE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DUNGEON_SUB_STAGE_OPEN_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DUNGEON_SUB_STAGE_GO_NEXT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DUNGEON_SUB_STAGE_LOAD_COMPLETE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DUNGEON_SUB_STAGE_CLEAR_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_TALK_WITH_NPC_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DUNGEON_KILLALLNPC_CHECK_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	/// combat. This is where EXP and ED are actually earned.
	bool Handler_EGS_NPC_UNIT_CREATE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_NPC_UNIT_DIE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	/// World triggers - gates, bridges, blockers a dungeon script moves. The
	/// _NOT this sends is the only thing that calls CX2World::ActiveTrigger on a
	/// shipping client, so without it the geometry never changes.
	bool Handler_EGS_WORLD_TRIGGER_RELOCATION_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	/// ending the run
	bool Handler_EGS_END_GAME_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_STATE_CHANGE_RESULT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_RESULT_SUCCESS_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_LEAVE_GAME_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	/// dying, and the result screen's reward box. Every one of these arms an
	/// AddServerPacket wait, so an unanswered one turns a death into a hang.
	bool Handler_EGS_USER_UNIT_DIE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_USER_UNIT_DIE_COMPLETE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_RESURRECT_TO_CONTINUE_DUNGEON_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_STOP_DUNGEON_CONTINUE_TIME_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DUNGEON_SECRET_STAGE_ENTER_CHECK_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_START_REWARD_BOX_SELECT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_SELECT_REWARD_BOX_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	//////////////////////////////////////////////////////////////////////////
	// Handlers_Room.cpp helpers

	/// Fill a KRoomUserInfo for the one player in the room. m_kGameStat here is
	/// NOT cosmetic: CX2Unit::UnitData::SetKRoomUserInfo assigns it straight
	/// over the unit's own game stat, so a zeroed one gives the in-dungeon
	/// character max HP 0 - the same trap phase 3 hit in the village.
	/// Fill the stamina gauge's pair of fields.
	///
	/// Spirit - the gauge the client labels stamina - is NOT modelled offline:
	/// nothing consumes it, nothing regenerates it, and no dungeon is refused
	/// for lack of it. What matters is that it must not be left at zero, which
	/// is what phase 3 did: every consumer computes m_iSpirit / (float)
	/// m_iSpiritMax with no guard, so 0/0 is a NaN that stretches the gauge
	/// bitmap to a garbage width and prints as `-1.$%` in the tooltip
	/// ([X2StateMenu.cpp:3378](X2Lib/X2StateMenu.cpp#L3378),
	/// [X2StateDungeonRoom.cpp:3029](X2Lib/X2StateDungeonRoom.cpp#L3029)).
	///
	/// Only the *ratio* is ever displayed, so reporting the two equal renders
	/// the full gauge that "not modelled" calls for. The real maximum is
	/// MAX_SPIRIT in the server's SpiritTable.lua, which is not in this tree at
	/// all - see the plan for what to do if spirit is ever modelled properly.
	static void FillSpirit( OUT int& iSpirit, OUT int& iSpiritMax );

	void MakeRoomUserInfo( const KOfflineUnitRow& kRow, OUT KRoomUserInfo& kOut );

	/// The single-occupant slot list. The slot is always host: CX2Game::IsHost()
	/// reads it, and the client only drives the simulation - stage loads, NPC
	/// spawns, the end of the game - when it believes it is host.
	void MakeRoomSlots( const KOfflineUnitRow& kRow, int iSlotState,
						OUT std::vector< KRoomSlotInfo >& vecOut );

	/// Fill m_RoomInfo from the room the session is in.
	void MakeRoomInfo( OUT KRoomInfo& kOut );

	/// Give a room to the session and remember it. Returns false if the session
	/// has no character selected.
	bool OpenRoom( KOfflineSession& kSes, int iRoomType, const KRoomInfo& kReqInfo,
				   int iBattleFieldID, OUT KOfflineUnitRow& kRow );

	/// Refill CX2Game::m_AllowFrameCount by pushing EGS_REMAINING_PLAY_TIME_NOT.
	/// Not optional and not cosmetic: without it the client stops gathering
	/// keyboard input roughly seven seconds into every dungeon. Driven off the
	/// three-second EGS_UPDATE_PLAY_STATUS_NOT tick because the emulator has no
	/// timer of its own. See the definition.
	void PushRemainingPlayTime( KOfflineSession& kSes );

	/// Fill a field with its opening monster population and push it as
	/// EGS_BATTLE_FIELD_NPC_LOAD_NOT. Fires once per field visit.
	void PushFieldNpcs( KOfflineSession& kSes );

	/// Record a placed field monster against the caps and the group map. Shared
	/// by the opening fill, respawn and the middle boss so the bookkeeping cannot
	/// drift between them.
	void TrackFieldNpc( const KNPCUnitReq& kReq );

	/// A field monster died: free its cap slot, queue its respawn, add its danger
	/// and roll for the middle boss.
	void OnFieldNpcDie( int iNpcUID );

	/// The field's heartbeat: place anything whose respawn timer has elapsed, and
	/// the middle boss if one is owed. Driven by the position push, since the
	/// offline server has no timer of its own.
	void TickField( KOfflineSession& kSes );

	/// Apply the run's EXP and ED to the character and persist them, levelling
	/// up off the client's own EXP table. Returns the new level.
	///
	/// Since phase 5 it also grants the skill points the levels are worth, out
	/// of the client's own CalcLevelUpIncreaseSkillPoint - a character that
	/// levelled and got no SP would have a skill tree it could never spend on.
	/// Add EXP and ED to a character, level it up if the EXP table says so,
	/// grant the skill points those levels are worth, and persist all of it.
	/// Returns the level afterwards; piOldLevel, when given, reports the
	/// level before, so a caller can tell whether a level-up happened.
	///
	/// Called once per kill, which is what the real server does -
	/// KGSUser's ERM_NPC_UNIT_DIE_NOT handler runs m_kEXP.AddExp( SumEXP() )
	/// and CheckCharLevelUp() on every monster
	/// ([GSUserRoomCommon.cpp:1959](KncWX2Server/GameServer/GSUserRoomCommon.cpp#L1959)).
	int  ApplyDungeonReward( UidType nUnitUID, int iAddEXP, int iAddED,
							 OUT int* piOldLevel = NULL );

	/// Resend the character to the client mid-play, so a level gained during
	/// a run shows up without waiting for the village.
	void PushUnitInfoUpdate( KOfflineSession& kSes, UidType nUnitUID );

	/// Announce a level-up: the packet that plays the effect.
	///
	/// EGS_CHAR_LEVEL_UP_NOT is what sets CX2Unit::SetIsLevelUp( true ) and
	/// calls DisplayLevelUpEffect ([X2StateMenu.cpp:4576](X2Lib/X2StateMenu.cpp#L4576)),
	/// and the village then replays the effect off that flag on its next frame
	/// ([X2TFieldGame.cpp:1932](X2Lib/X2TFieldGame.cpp#L1932)) - which is also
	/// how the dungeon result screen knows to show its level-up animation. It
	/// carries the new level and both stats, and the handler refills HP and MP,
	/// so it is the whole of what a level-up looks like to the client.
	///
	/// Safe to send from anywhere: the village, the dungeon and the battlefield
	/// all dispatch it (X2StateField.cpp:1138, X2StateDungeonGame.cpp:1902,
	/// X2StateBattleField.cpp:361), and unlike EGS_UPDATE_UNIT_INFO_NOT it does
	/// not touch the dungeon leave gate.
	void PushLevelUp( KOfflineSession& kSes, UidType nUnitUID );

	/// Put one dead monster's loot on the floor: the ED it is worth, split into
	/// coins, plus whatever the item lottery drew. One EGS_DROP_ITEM_NOT for
	/// both, because they scatter from the same corpse.
	///
	/// Phase 4 deferred items deliberately - there was no inventory to pick
	/// them up into - and credited ED silently at kill time, which left the
	/// floor of a dungeon empty and the money appearing out of nowhere. Both
	/// halves land here.
	void PushNpcDrop( KOfflineSession& kSes, int iNpcUID, int iNpcID, int iED,
					  const VECTOR3& kDiePos );

	/// GetEDItemID: which coin represents this much ED.
	/// KDropTable::GetEDItemID (KDropTable.cpp:1166) verbatim.
	static int EDCoinItemID( int iED );

	/// The static monster list for one stage, keyed by sub-stage index, in the
	/// shape EGS_DUNGEON_STAGE_LOAD_NOT carries it. Read out of the client's own
	/// copy of the dungeon script; mirrors CXSLDungeon::GetNPCData. Stamps a UID
	/// on every monster and records its level and ID for the kill reward.
	void BuildStageNpcData( int iStageID, OUT std::map< int, KNPCList >& mapOut );

	//////////////////////////////////////////////////////////////////////////
	// Handlers_Inventory.cpp - phase 5

	bool Handler_EGS_CHANGE_INVENTORY_SLOT_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DELETE_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_SORT_CATEGORY_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_USE_ITEM_IN_INVENTORY_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_USE_QUICK_SLOT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_BUY_ED_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_SELL_ED_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	/// Endurance repair at the village NPC. Implementable from client data
	/// alone - the per-point cost is on the item templet - and a no-op in
	/// practice, because nothing offline wears gear down: see the handler.
	bool Handler_EGS_REPAIR_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	/// Answered with a refusal because the client waits on them and their
	/// tables are server-only. See the handlers.
	bool Handler_EGS_ENCHANT_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_SOCKET_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_ENCHANT_ATTACH_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_ATTRIB_ENCHANT_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_ATTRIB_ATTACH_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	/// Dismantling (phase 12). ResolveTable.lua is packed, so this computes
	/// real materials through CX2OfflineResolveTable rather than refusing -
	/// see that class's header for the algorithm and its sources.
	bool Handler_EGS_RESOLVE_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	/// Tell the client its own gear changed. The broadcast is what redraws the
	/// character - phase 4 correction 6 again - and it is a different packet in
	/// a room than in a village.
	void PushEquipChanged( KOfflineSession& kSes,
						   const std::vector< KInventoryItemInfo >& vecChanged );

	//////////////////////////////////////////////////////////////////////////
	// Handlers_Skill.cpp - phase 5

	bool Handler_EGS_GET_SKILL_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_RESET_SKILL_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_INIT_SKILL_TREE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CHANGE_SKILL_SLOT_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	/// Sent by the client every time a skill is fired in a dungeon. Nothing but
	/// SQT_SKILL_USE listens to it, and it expects no reply at all.
	bool Handler_EGS_SKILL_USE_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	/// First unequipped item of this ID in the bag, or 0. Used to spend a
	/// consumable the request did not name.
	static UidType FindItemByID( int iItemID );

	//////////////////////////////////////////////////////////////////////////
	// Handlers_Quest.cpp - phase 6

	bool Handler_EGS_NEW_QUEST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_UPDATE_QUEST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_QUEST_COMPLETE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_ALL_COMPLETED_QUEST_COMPLETE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GIVE_UP_QUEST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GATHER_GIVE_UP_QUEST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_EQUIP_TITLE_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	/// Make sure the quest and title state belongs to the character this
	/// session has selected, and hand back its row. Returns false when there is
	/// no selected character - every quest handler is a no-op then.
	bool LoadQuestState( KOfflineSession& kSes, OUT KOfflineUnitRow& kRow );

	/// One quest's completion: the checks, the reward, the class change, and
	/// the ACK payload. Shared by EGS_QUEST_COMPLETE_REQ and by the batch
	/// EGS_ALL_COMPLETED_QUEST_COMPLETE_REQ, which is literally a vector of the
	/// same request.
	///
	/// It does NOT send anything except through the ACK it fills in. What the
	/// completion sets off is left in m_kQuestAfter for AfterQuestComplete, so
	/// the caller can put the ACK on the wire first - see the comment there.
	int  CompleteOneQuest( KOfflineSession& kSes, const KEGS_QUEST_COMPLETE_REQ& kReq,
						   OUT KEGS_QUEST_COMPLETE_ACK& kAck );

	/// Everything a finished quest sets off, played out after its ACK has gone.
	void AfterQuestComplete( KOfflineSession& kSes );

	/// One EGS_NEW_QUEST_NOT per epic quest the game just handed the player.
	/// A story quest is never asked for - see CX2OfflineQuest::CheckAutoOpen -
	/// so this is the only thing that puts one on screen.
	void PushNewQuest( KOfflineSession& kSes, const std::vector< KQuestInstance >& vecOpened );

	/// Open whatever epic quests this place or this progress unlocks, announce
	/// them, and let them tick their own "you are already here" steps.
	void QuestAutoOpen( KOfflineSession& kSes, int ePlace, int iPlaceID,
						const KOfflineUnitRow& kRow );

	/// EGS_UPDATE_QUEST_NOT, or nothing at all when the vector is empty. The
	/// client redraws its quest UI from this and prints the progress line over
	/// the character's head, so an empty send would be a visible no-op.
	void PushQuestUpdate( KOfflineSession& kSes, const std::vector< KQuestInstance >& vecChanged );

	/// EGS_NEW_MISSION_NOT / EGS_UPDATE_MISSION_NOT / EGS_REWARD_TITLE_NOT, each
	/// only when it has something to say.
	void PushMissionUpdate( KOfflineSession& kSes,
							const std::vector< KMissionInstance >& vecNew,
							const std::vector< KMissionInstance >& vecChanged,
							const std::vector< KTitleInfo >& vecNewTitle );

	//////////////////////////////////////////////////////////////////////////
	// The game events quests and title missions listen to. Each is called from
	// the handler that already owns that moment - the talk handler, the kill
	// handler, the result handler - and does the whole of the quest side:
	// advance, persist, notify.

	void QuestOnTalkNPC( KOfflineSession& kSes, int iNPCID );
	void QuestOnNpcDie( KOfflineSession& kSes, int iNpcID );
	void QuestOnDungeonClear( KOfflineSession& kSes, char cRank, int iPlayTime, int iDamage );
	void QuestOnEnterVillage( KOfflineSession& kSes, int iMapID );
	void QuestOnEnterField( KOfflineSession& kSes, int iBattleFieldID );
	void QuestOnEnterDungeon( KOfflineSession& kSes );
	void QuestOnFindNPC( KOfflineSession& kSes, const std::vector< int >& vecNpcID );
	void QuestOnLevelUp( KOfflineSession& kSes );

	/// A quest instance has just come into existence, from any of the four
	/// routes that can create one. Ticks whatever is already true about where
	/// the character is standing and what level it is.
	///
	/// This is the fix for "the next quest says go to Elder, I am IN Elder, and
	/// nothing happens": the step's trigger fired before the step existed. The
	/// real server re-runs exactly these checks off the back of the accept it
	/// just made (GSUserGameCommon.cpp:1608-1649), which is why a live client
	/// never sees it.
	void QuestCheckHereAndNow( KOfflineSession& kSes, const KOfflineUnitRow& kRow,
							   OUT std::vector< KQuestInstance >& vecChanged );

	/// One call per skill point spent, and one per request for the skills that
	/// went from unlearned to learned.
	void QuestOnUseSkillPoint( KOfflineSession& kSes, int iSkillID, int iCount );
	void QuestOnLearnNewSkill( KOfflineSession& kSes, const std::vector< int >& vecSkillID );

	/// An item consumed from the bag or a quick slot.
	void QuestOnUseItem( KOfflineSession& kSes, int iItemID );

	/// The bag changed, so an item-collection step may have become satisfiable.
	/// The quest half needs no work - the client counts the bag itself and the
	/// completion check does the same - but a title mission with a collection
	/// step has to be noticed and paid out, and nothing else notices.
	void QuestOnInventoryChanged( KOfflineSession& kSes );

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// Handlers_Shop.cpp - phase 7, the cash shop
	bool Handler_EGS_BILL_PRODUCT_INFO_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_WISH_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_MODIFY_WISH_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_BILL_INVENTORY_INQUIRY_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_BUY_CASH_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_BILL_GET_PURCHASED_CASH_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_PURCHASED_PACKAGE_CASH_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_PRESENT_CASH_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_VISIT_CASH_SHOP_NOT( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CHECK_PRESENT_CASH_INVENTORY_NOT( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_APPLY_COUPON_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	//////////////////////////////////////////////////////////////////////////
	// Handlers_Social.cpp - phase 7, the social systems and pets
	bool Handler_EGS_CREATE_GUILD_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_GUILD_USER_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_APPLY_JOIN_GUILD_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_GUILD_AD_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_APPLY_JOIN_GUILD_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_ACCEPT_JOIN_GUILD_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DELETE_APPLY_JOIN_GUILD_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_REGISTRATION_GUILD_AD_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_MODIFY_REG_GUILD_AD_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_INVITE_GUILD_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_INVITE_GUILD_REPLY_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_KICK_GUILD_MEMBER_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CHANGE_GUILD_MEMBER_GRADE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CHANGE_GUILD_MEMBER_MESSAGE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CHANGE_GUILD_MESSAGE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CHANGE_GUILD_NAME_CHECK_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CHANGE_GUILD_NAME_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_GUILD_SKILL_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_GUILD_SKILL_IN_BOARD_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_INIT_GUILD_SKILL_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_RESET_GUILD_SKILL_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_INVITE_PARTY_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_LEAVE_PARTY_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CHANGE_PARTY_TYPE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_PARTY_CHANGE_DUNGEON_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_PARTY_CHANGE_HOST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_PARTY_CHANGE_GET_ITEM_TYPE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_PARTY_CHANGE_NUM_OF_PER_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_PARTY_CHANGE_CHECK_MIN_LEVEL_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_PARTY_BAN_USER_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_PARTY_GAME_START_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_REQUEST_MATCH_MAKING_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_AUTO_PARTY_DUNGEON_GAME_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_PVP_PARTY_CHANGE_MATCH_INFO_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_COMMUNITY_USER_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_COMMUNITY_USER_LIST_MONITORING_MODE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_RECOMMEND_USER_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_REQUEST_FRIEND_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_ACCEPT_FRIEND_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DENY_FRIEND_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_BLOCK_FRIEND_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_UNBLOCK_FRIEND_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DELETE_FRIEND_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_MOVE_FRIEND_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_MAKE_FRIEND_GROUP_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_RENAME_FRIEND_GROUP_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DELETE_FRIEND_GROUP_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_UPDATE_FRIEND_INFO_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_KNM_REQUEST_NEW_FRIEND_INFO_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_MY_MESSENGER_SN_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_ED_MONITORING_BLOCK_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_USER_COMMUNITY_SURVEY_NEW_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_INVITE_PVP_ROOM_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_UPDATE_COMMUNITY_OPTION_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_SEARCH_UNIT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_CONNECTION_UNIT_INFO_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_WATCH_UNIT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_POST_LETTER_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_READ_LETTER_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DELETE_LETTER_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_SEND_LETTER_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_ITEM_FROM_LETTER_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_POST_BLACK_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_NEW_POST_BLACK_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DEL_POST_BLACK_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CHAT_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CHAT_OPTION_INFO_WRITE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_NEW_BLACKLIST_USER_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DEL_BLACKLIST_USER_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_USE_MEGAPHONE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_RANKING_INFO_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_TUTORIAL_STUDENT_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_REQUEST_TUTORIAL_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DEL_TUTORIAL_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_SHARE_BANK_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_TEMP_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DELETE_TEMP_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_SEARCH_TRADE_BOARD_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CHECK_SUM_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_PET_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_GET_RIDING_PET_LIST_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CREATE_PET_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_SUMMON_PET_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_FEED_PETS_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_COMMANDS_FOR_PETS_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_PET_EVOLUTION_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CHANGE_PET_NAME_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_SET_AUTO_FEED_PETS_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CREATE_RIDING_PET_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_SUMMON_RIDING_PET_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_UNSUMMON_RIDING_PET_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_RELEASE_RIDING_PET_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	//////////////////////////////////////////////////////////////////////////
	// Author: Iruha
	// Date: 2026-09-05
	// Description: Phase 28 - persisted summoned pet, split the way live splits
	// it. RestoreSummonedPet decides at character select which pet (if any) is
	// still out; SendPendingPetRestore does the actual spawn once the field has
	// finished loading, since CreateGamePet needs g_pX2Game to already exist.
	// Both live in Handlers_Social.cpp, next to MakePetInfo and
	// Handler_EGS_SUMMON_PET_REQ.
	void RestoreSummonedPet( KOfflineSession& kSes, UidType nUnitUID );
	void SendPendingPetRestore( KOfflineSession& kSes );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Handlers_Social.cpp - phase 7, the rest of what a village menu can reach
	bool Handler_EGS_CHECK_CHANNEL_CHANGE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_JOIN_FIELD_CHANNEL_CHANGE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_WARP_BY_BUTTON_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_2013_EVENT_MISSION_COMPLETE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_REG_SKILL_NOTE_MEMO_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_OPEN_RANDOM_ITEM_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_AUTH_SECOND_SECURITY_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CREATE_SECOND_SECURITY_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_COMPARE_SECOND_SECURITY_PW_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CHANGE_SECOND_SECURITY_PW_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DELETE_SECOND_SECURITY_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DELETE_SECOND_SECURITY_VERIFY_REQ( KOfflineSession& kSes, const KEvent& kEvent );

	// Handlers_Stub.cpp - answered because the client blocks on them, nothing more
	bool Handler_EGS_MODULE_INFO_UPDATE_NOT( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_DUNGEON_PLAY_INFO_TO_SERVER_NOT( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_FRAME_AVERAGE_REQ( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_REQUEST_GET_AUTO_PARTY_BONUS_INFO_NOT( KOfflineSession& kSes, const KEvent& kEvent );
	bool Handler_EGS_CLIENT_QUIT_REQ( KOfflineSession& kSes, const KEvent& kEvent );

private:
	static CX2OfflineServer*					ms_pInstance;

	KncCriticalSection							m_cs;
	std::map< KSession*, KOfflineSession >		m_mapSession;

	/// Serializes the whole of one packet: the log's defer buffer, the
	/// transaction, and the dispatch. There is one KSession::Run thread per
	/// proxy, so without this the channel and game sessions can interleave -
	/// which would braid two packets' replies together in the log and, since
	/// phase 8, nest one packet's transaction inside another's.
	KncCriticalSection							m_csDispatch;

	/// set by Handler_EGS_CLIENT_QUIT_REQ, acted on by OnClientSend
	bool										m_bQuitRequested;

	UidType										m_nUserUID;			///< the single offline account
	int											m_iUnitSlots;
	std::wstring								m_wstrLoginID;

	KOfflineRoom								m_kRoom;			///< the one room; see KOfflineRoom
	UidType										m_nNextRoomUID;

	KQuestAfter									m_kQuestAfter;		///< see KQuestAfter
};

//////////////////////////////////////////////////////////////////////////
// The offline server never touches the wire format of the KEvent envelope,
// only the inner payload - and for that it uses the very same KSerializer the
// real server does.

template< typename T >
bool CX2OfflineServer::ReadReq( const KEvent& kEvent, T& kOut )
{
	KSerBuffer* pBuff = const_cast< KSerBuffer* >( &kEvent.m_kbuff );

	// the client sends with bCompress = false by default, but the real receive
	// path uncompresses unconditionally, so do the same
	pBuff->UnCompress();

	KSerializer ks;
	ks.BeginReading( pBuff );
	bool bOK = ks.Get( kOut );
	ks.EndReading();

	if( false == bOK )
	{
		CX2OfflineLog::Server( L"ERROR  deserialize failed for %s (id=%u)",
			CX2OfflineLog::EventName( kEvent.m_usEventID ), (unsigned int)kEvent.m_usEventID );
	}

	return bOK;
}

template< typename T >
bool CX2OfflineServer::Reply( KOfflineSession& kSes, unsigned short usEventID, const T& kData )
{
	if( NULL == kSes.m_pSession )
		return false;

	UidType anTrace[2] = { -1, -1 };

	KEventPtr spEvent( new KEvent );
	spEvent->SetData( PI_GS_PROXY, anTrace, usEventID, kData );

	CX2OfflineLog::Packet( false, KindStr( kSes.m_eKind ), usEventID,
		spEvent->m_kbuff.GetLength(), L"" );

	kSes.m_pSession->QueueingEvent( spEvent );
	return true;
}

#endif SERV_IRUHADEV_OFFLINE
