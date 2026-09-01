#include "stdafx.h"
#include "X2OfflineServer.h"

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-01
// Description: Offline mode - phase 2. Character CRUD against SQLite: the
//              character list, create, soft delete, final delete, restore,
//              and selecting a character.
//
//              The reference for what each reply has to contain is the
//              client's own handler in X2Lib/X2StateServerSelect.cpp (and
//              X2Lib/X2StateCreateUnit.cpp for creation); the reference for
//              what the real server *did* is
//              DataBase/.../dbo.gup_create_unit.proc.sql and
//              dbo.gup_delete_unit.proc.sql, which is where the soft-delete
//              rule (del_date == reg_date means alive) comes from.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

namespace
{
	/// dbo.gup_create_unit's own rule, from SERV_NICK_NAME_DOUBLE_CHECK:
	/// ASCII alphanumerics only, at least two characters.
	bool IsAcceptableNickName( const std::wstring& wstrName, OUT int& iError )
	{
		if( wstrName.size() < 2 || wstrName.size() > 16 )
		{
			iError = NetError::ERR_CREATE_UNIT_04;		///< bad length
			return false;
		}

		for( size_t i = 0; i < wstrName.size(); ++i )
		{
			wchar_t c = wstrName[i];

			if( ( c >= L'a' && c <= L'z' ) ||
				( c >= L'A' && c <= L'Z' ) ||
				( c >= L'0' && c <= L'9' ) )
				continue;

			iError = NetError::ERR_CREATE_UNIT_08;		///< unusable nickname
			return false;
		}

		return true;
	}
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	// THE character list. No request body (SendID).
	if( false == EnsureAccount( kSes, kSes.m_wstrLoginID ) )
		return false;

	// Belt and braces for the gate in Handler_EGS_SELECT_UNIT_REQ: only
	// CX2StateServerSelect asks for this list, so receiving it means the player
	// is on the character-select screen whatever the session thought.
	// EGS_STATE_CHANGE_SERVER_SELECT_REQ normally gets there first.
	kSes.m_eState = S_SERVER_SELECT;

	std::vector< KOfflineUnitRow > vecRow;
	CX2OfflineDB::Instance()->LoadUnits( kSes.m_nUserUID, vecRow );

	KEGS_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_nUnitSlot	= m_iUnitSlots;
	kAck.m_bSharingBank	= false;

	int iDeleted = 0;

	// Soft-deleted units belong in this list: KUnitInfo carries m_bDeleted and
	// m_trDelAbleDate, and CreateUnitButton() draws the restore / final-delete
	// slot from them. Leaving them out would silently discard them.
	for( size_t i = 0; i < vecRow.size(); ++i )
	{
		KUnitInfo kUnit;
		MakeUnitInfoFromRow( kUnit, vecRow[i] );

		kAck.m_vecUnitInfo.push_back( kUnit );

		if( true == vecRow[i].IsDeleted() )
			++iDeleted;
	}

	Reply( kSes, EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK, kAck );

	CX2OfflineLog::Server( L"UNITLIST %u unit(s) (%d deleted), %d slots, userUID=%I64d",
		(unsigned int)vecRow.size(), iDeleted, m_iUnitSlots, (__int64)kSes.m_nUserUID );

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

	// Not optional, and nothing asks for it. SERV_SECOND_SECURITY is on, and
	// CX2StateServerSelect::m_bEnableUnitSelectBySecondSecurity starts false;
	// while it is false, SUSUCM_UNIT_BUTTON_UP, SUSUCM_CREATE_UNIT,
	// SUSUCM_SELECT_UNIT and Handler_EGS_SELECT_UNIT_REQ all return early, so
	// every click on the character-select screen is silently a no-op - no
	// packet, no error, no dialog. The only thing that clears it on a first
	// visit is this unprompted push, which the real server sends because the
	// account has no second password set.
	//
	// (The state's other unlock site only runs when the state is *re-entered*
	// with m_sbSelectedServerSet already true - which is why the screen appears
	// to work right after creating a character and is dead on a fresh login.)
	KEGS_SECOND_SECURITY_INFO_NOT kSecNot;
	kSecNot.m_iOK			= NetError::NET_OK;
	kSecNot.m_bUseSecondPW	= false;			///< no second password offline
	Reply( kSes, EGS_SECOND_SECURITY_INFO_NOT, kSecNot );

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_CREATE_UNIT_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_CREATE_UNIT_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	if( false == EnsureAccount( kSes, kSes.m_wstrLoginID ) )
		return false;

	CX2OfflineDB* pDB = CX2OfflineDB::Instance();

	KEGS_CREATE_UNIT_ACK kAck;
	kAck.m_iOK				= NetError::NET_OK;
	kAck.m_wstrEnableDate	= L"";

	int iError = NetError::NET_OK;

	if( false == IsAcceptableNickName( kReq.m_wstrNickName, iError ) )
	{
		kAck.m_iOK = iError;
	}
	else if( true == pDB->IsNickNameTaken( kReq.m_wstrNickName ) )
	{
		kAck.m_iOK = NetError::ERR_CREATE_UNIT_01;		///< nickname already exists
	}
	else if( pDB->CountLiveUnits( kSes.m_nUserUID ) >= m_iUnitSlots )
	{
		kAck.m_iOK = NetError::ERR_CREATE_UNIT_05;		///< out of character slots
	}

	if( NetError::NET_OK != kAck.m_iOK )
	{
		// CX2StateCreateUnit::Handler_EGS_CREATE_UNIT_ACK reads the nickname and
		// class back out of the failed ACK to re-populate its own form, so echo
		// them even on the error path - dbo.gup_create_unit does the same.
		kAck.m_kUnitInfo.m_wstrNickName	= kReq.m_wstrNickName;
		kAck.m_kUnitInfo.m_cUnitClass	= (char)kReq.m_iClass;

		CX2OfflineLog::Server( L"CREATE   rejected '%s' class=%d (m_iOK=%d)",
			kReq.m_wstrNickName.c_str(), kReq.m_iClass, kAck.m_iOK );

		return Reply( kSes, EGS_CREATE_UNIT_ACK, kAck );
	}

	KOfflineUnitRow kRow;
	if( false == pDB->CreateUnit( kSes.m_nUserUID, kReq.m_iClass, kReq.m_wstrNickName, kRow ) )
	{
		kAck.m_iOK						= NetError::ERR_CREATE_UNIT_02;		///< transaction error
		kAck.m_kUnitInfo.m_wstrNickName	= kReq.m_wstrNickName;
		kAck.m_kUnitInfo.m_cUnitClass	= (char)kReq.m_iClass;

		return Reply( kSes, EGS_CREATE_UNIT_ACK, kAck );
	}

	MakeUnitInfoFromRow( kAck.m_kUnitInfo, kRow );

	// No starting gear and no starting skill. That is not an omission: the real
	// dbo.gup_create_unit inserts no items at all, and the one skill it does
	// insert (10000 / 20030 / 30000 / 40010, for classes 1-4 only) predates
	// UPGRADE_SKILL_SYSTEM_2013 - those IDs do not exist in this build's
	// CX2SkillTree::SKILL_ID enum, and that DataBase/ snapshot knows nothing of
	// Eve, Chung, Ara or Elesis either. Seeding from it would write skill IDs
	// the client cannot resolve. The unit_skill table is in the schema and gets
	// populated when the skill tree itself is wired up.
	kAck.m_vecSkillSlot.clear();

	CX2OfflineLog::Server( L"CREATE   '%s' class=%d -> unitUID=%I64d",
		kRow.m_wstrNickName.c_str(), kRow.m_iUnitClass, (__int64)kRow.m_nUnitUID );

	return Reply( kSes, EGS_CREATE_UNIT_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_DELETE_UNIT_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_DELETE_UNIT_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineDB* pDB = CX2OfflineDB::Instance();

	KEGS_DELETE_UNIT_ACK kAck;
	kAck.m_iOK				= NetError::ERR_DELETE_UNIT_00;		///< no such character
	kAck.m_iUnitUID			= kReq.m_iUnitUID;
	kAck.m_uiKNMSerialNum	= 0;
	kAck.m_iGuildUID		= 0;
	kAck.m_ucLevel			= 0;
	kAck.m_cUnitClass		= 0;
	kAck.m_tReDelAbleDate	= 0LL;
	kAck.m_tDelAbleDate		= 0LL;

	KOfflineUnitRow kRow;
	if( false == pDB->LoadUnit( kReq.m_iUnitUID, kRow ) || kRow.m_nUserUID != kSes.m_nUserUID )
		return Reply( kSes, EGS_DELETE_UNIT_ACK, kAck );

	kAck.m_wstrNickName	= kRow.m_wstrNickName;
	kAck.m_ucLevel		= (u_char)kRow.m_iLevel;
	kAck.m_cUnitClass	= (char)kRow.m_iUnitClass;

	if( true == kRow.IsDeleted() )
	{
		// Already pending deletion. ERR_DELETE_UNIT_01 is the "already deleted
		// or already waiting" code; the client then prints m_tReDelAbleDate.
		kAck.m_iOK				= NetError::ERR_DELETE_UNIT_01;
		kAck.m_tReDelAbleDate	= CX2OfflineDB::DelAbleDate( kRow.m_tDelDate );

		return Reply( kSes, EGS_DELETE_UNIT_ACK, kAck );
	}

	__int64 tNow = (__int64)::_time64( NULL );

	// Soft delete only - never DELETE FROM. The row keeps its nickname so the
	// character-select screen can still draw the slot; the name itself is
	// released for reuse because IsNickNameTaken() only looks at live units,
	// which is what dbo.gup_delete_unit achieves by nulling GUnitNickName.
	if( false == pDB->SoftDeleteUnit( kReq.m_iUnitUID, tNow ) )
	{
		kAck.m_iOK = NetError::ERR_DELETE_UNIT_05;		///< delete failed
		return Reply( kSes, EGS_DELETE_UNIT_ACK, kAck );
	}

	kAck.m_iOK			= NetError::NET_OK;
	kAck.m_tDelAbleDate	= CX2OfflineDB::DelAbleDate( tNow );

	CX2OfflineLog::Server( L"DELETE   '%s' (unitUID=%I64d) pending, final delete from %s",
		kRow.m_wstrNickName.c_str(), (__int64)kRow.m_nUnitUID,
		CX2OfflineDB::FormatDate( kAck.m_tDelAbleDate ).c_str() );

	return Reply( kSes, EGS_DELETE_UNIT_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_FINAL_DELETE_UNIT_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_FINAL_DELETE_UNIT_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineDB* pDB = CX2OfflineDB::Instance();

	KEGS_FINAL_DELETE_UNIT_ACK kAck;
	kAck.m_iOK				= NetError::ERR_DELETE_UNIT_00;
	kAck.m_iUnitUID			= kReq.m_iUnitUID;
	kAck.m_uiKNMSerialNum	= 0;
	kAck.m_iGuildUID		= 0;
	kAck.m_ucLevel			= 0;
	kAck.m_cUnitClass		= 0;
	kAck.m_tReDelAbleDate	= 0LL;

	KOfflineUnitRow kRow;
	if( false == pDB->LoadUnit( kReq.m_iUnitUID, kRow ) || kRow.m_nUserUID != kSes.m_nUserUID )
		return Reply( kSes, EGS_FINAL_DELETE_UNIT_ACK, kAck );

	kAck.m_wstrNickName	= kRow.m_wstrNickName;
	kAck.m_ucLevel		= (u_char)kRow.m_iLevel;
	kAck.m_cUnitClass	= (char)kRow.m_iUnitClass;

	if( false == kRow.IsDeleted() )
	{
		kAck.m_iOK = NetError::ERR_DELETE_UNIT_04;		///< not a final-deletable character
		return Reply( kSes, EGS_FINAL_DELETE_UNIT_ACK, kAck );
	}

	__int64 tAble = CX2OfflineDB::DelAbleDate( kRow.m_tDelDate );
	__int64 tNow  = (__int64)::_time64( NULL );

	if( tNow < tAble )
	{
		// The client greys the button out until then, so this is only reachable
		// from a clock change - answer it properly rather than deleting early.
		kAck.m_iOK				= NetError::ERR_DELETE_UNIT_03;		///< not yet
		kAck.m_tReDelAbleDate	= tAble;

		return Reply( kSes, EGS_FINAL_DELETE_UNIT_ACK, kAck );
	}

	if( false == pDB->FinalDeleteUnit( kReq.m_iUnitUID ) )
	{
		kAck.m_iOK = NetError::ERR_DELETE_UNIT_05;
		return Reply( kSes, EGS_FINAL_DELETE_UNIT_ACK, kAck );
	}

	kAck.m_iOK = NetError::NET_OK;

	CX2OfflineLog::Server( L"DELETE   '%s' (unitUID=%I64d) erased for good",
		kRow.m_wstrNickName.c_str(), (__int64)kRow.m_nUnitUID );

	return Reply( kSes, EGS_FINAL_DELETE_UNIT_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_RESTORE_UNIT_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_RESTORE_UNIT_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	CX2OfflineDB* pDB = CX2OfflineDB::Instance();

	KEGS_RESTORE_UNIT_ACK kAck;
	kAck.m_iOK					= NetError::ERR_RESTORE_UNIT_01;	///< not a restorable character
	kAck.m_iUnitUID				= kReq.m_iUnitUID;
	kAck.m_tRestoreAbleDate		= 0LL;

	KOfflineUnitRow kRow;
	if( false == pDB->LoadUnit( kReq.m_iUnitUID, kRow ) ||
		kRow.m_nUserUID != kSes.m_nUserUID ||
		false == kRow.IsDeleted() )
	{
		return Reply( kSes, EGS_RESTORE_UNIT_ACK, kAck );
	}

	// A live character may have taken the name in the meantime; restoring on
	// top of it would give two characters the same nickname.
	if( true == pDB->IsNickNameTaken( kRow.m_wstrNickName ) )
	{
		kAck.m_iOK = NetError::ERR_RESTORE_UNIT_01;

		CX2OfflineLog::Server( L"RESTORE  refused unitUID=%I64d - '%s' is in use again",
			(__int64)kRow.m_nUnitUID, kRow.m_wstrNickName.c_str() );

		return Reply( kSes, EGS_RESTORE_UNIT_ACK, kAck );
	}

	// Restoring is del_date = reg_date, the exact inverse of the soft delete.
	// There is no restore cooldown offline, so ERR_RESTORE_UNIT_02 is never
	// returned and m_tRestoreAbleDate stays 0.
	if( false == pDB->RestoreUnit( kReq.m_iUnitUID ) )
		return Reply( kSes, EGS_RESTORE_UNIT_ACK, kAck );

	kAck.m_iOK = NetError::NET_OK;

	CX2OfflineLog::Server( L"RESTORE  '%s' (unitUID=%I64d) is back",
		kRow.m_wstrNickName.c_str(), (__int64)kRow.m_nUnitUID );

	return Reply( kSes, EGS_RESTORE_UNIT_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_SELECT_UNIT_REQ( KOfflineSession& kSes, const KEvent& kEvent )
{
	KEGS_SELECT_UNIT_REQ kReq;
	if( false == ReadReq( kEvent, kReq ) )
		return false;

	// The FSM gate the real server has, and the offline server did not.
	//
	// KGSUser's handler opens with
	//   VERIFY_STATE_WARN_REPEAT_FILTER( ( 1, KGSFSM::S_SERVER_SELECT ), ... )
	// (GSUserGameCommon.cpp:800), and a successful select moves the user
	// straight to S_FIELD_MAP in KGSUser::OnAccountSelectUnitAck
	// (GSUserFunction.cpp:4823) - so a *second* select is state-rejected.
	//
	// That matters because the client sends one. Within 1-2ms of
	// EGS_STATE_CHANGE_FIELD_ACK it intermittently re-sends
	// EGS_SELECT_UNIT_REQ, and answering it properly restarts the whole
	// character-select tail: five _NOTs, EGS_SELECT_UNIT_ACK,
	// EGS_GET_MY_INVENTORY_REQ, and finally another
	// Handler_EGS_STATE_CHANGE_FIELD_REQ - which is why leaving the village for
	// character select would sometimes throw the player straight back to the
	// village with the "moving to the village area" dialog.
	//
	// The reply is what the house macro sends: the ACK carrying
	// ERR_WRONG_STATE_00. That code is deliberately listed in
	// CX2Main::IsValidPacket's silent-false set (X2Main.cpp:6936), so it clears
	// the client's AddServerPacket wait and shows nothing - whereas dropping the
	// packet outright would leave that wait to time out into a network error.
	if( S_SERVER_SELECT != kSes.m_eState )
	{
		KEGS_SELECT_UNIT_ACK kAck;
		kAck.m_iOK = NetError::ERR_WRONG_STATE_00;

		CX2OfflineLog::Server( L"SELECT   ignored repeat for unitUID=%I64d - session is past character select",
			(__int64)kReq.m_iUnitUID );

		return Reply( kSes, EGS_SELECT_UNIT_ACK, kAck );
	}

	CX2OfflineDB* pDB = CX2OfflineDB::Instance();

	KOfflineUnitRow kRow;
	if( false == pDB->LoadUnit( kReq.m_iUnitUID, kRow ) ||
		kRow.m_nUserUID != kSes.m_nUserUID ||
		true == kRow.IsDeleted() )
	{
		KEGS_SELECT_UNIT_ACK kAck;
		kAck.m_iOK = NetError::ERR_SELECT_UNIT_00;

		CX2OfflineLog::Server( L"SELECT   refused unitUID=%I64d", (__int64)kReq.m_iUnitUID );

		return Reply( kSes, EGS_SELECT_UNIT_ACK, kAck );
	}

	pDB->TouchLastDate( kRow.m_nUnitUID );

	kSes.m_nSelectedUnitUID	= kRow.m_nUnitUID;
	kSes.m_eState			= S_FIELD_MAP;

	// Order matters: the five notifications carry the whole character payload
	// and the ACK is only the m_iOK that closes the client's wait. Sending the
	// ACK first would let CX2StateServerSelect run on a half-built CX2Unit.
	PushSelectUnitNotifications( kSes, kRow );

	KEGS_SELECT_UNIT_ACK kAck;
	kAck.m_iOK = NetError::NET_OK;

	CX2OfflineLog::Server( L"SELECT   '%s' (unitUID=%I64d, class=%d, lv=%d)",
		kRow.m_wstrNickName.c_str(), (__int64)kRow.m_nUnitUID, kRow.m_iUnitClass, kRow.m_iLevel );

	return Reply( kSes, EGS_SELECT_UNIT_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineServer::Handler_EGS_GET_MY_INVENTORY_REQ( KOfflineSession& kSes, const KEvent& /*kEvent*/ )
{
	// Despite the name this carries no inventory - the items already went out
	// in EGS_SELECT_UNIT_1_NOT. It is the last step of character select, and
	// its ACK is what actually leaves the screen: CX2StateServerSelect::
	// Handler_EGS_GET_MY_INVENTORY_ACK calls either
	// Handler_EGS_CREATE_TUTORIAL_ROOM_REQ or Handler_EGS_STATE_CHANGE_FIELD_REQ.
	//
	// No request body (SendID), and the client waits on the ACK, so an
	// unanswered one strands the player on the character-select screen.
	KEGS_GET_MY_INVENTORY_ACK kAck;
	kAck.m_iOK			= NetError::NET_OK;

	// m_bIsRecommend true means "this account has already been through the
	// friend-recommendation flow", which suppresses a popup that would ask for
	// somebody else's nickname. There is nobody to recommend offline.
	kAck.m_bIsRecommend	= true;
	kAck.m_bSharingBank	= false;

	CX2OfflineLog::Server( L"ENTER    unitUID=%I64d leaving character select",
		(__int64)kSes.m_nSelectedUnitUID );

	return Reply( kSes, EGS_GET_MY_INVENTORY_ACK, kAck );
}

//////////////////////////////////////////////////////////////////////////

void CX2OfflineServer::PushSelectUnitNotifications( KOfflineSession& kSes, const KOfflineUnitRow& kRow )
{
	// SERV_SELECT_UNIT_PACKET_DIVISION is on, so the character payload is split
	// across five packets rather than riding on EGS_SELECT_UNIT_ACK.
	//
	// Phase 2 fills _1_NOT (the unit and its inventory shape) and _2_NOT (which
	// has nothing to say yet, but is what the client's quest list is reset
	// from). Phase 3 adds _4_NOT's m_kGamePlayStatus, the live HP/MP the HUD
	// is built from. The rest of _3_NOT / _4_NOT / _5_NOT still goes out with
	// empty collections; they are titles and pets, social state, and events -
	// phases 5 to 7.

	CX2OfflineDB* pDB = CX2OfflineDB::Instance();

	//////////////////////////////////////////////////////////////////////////
	// 1 - unit, inventory, skills
	{
		KEGS_SELECT_UNIT_1_NOT kNot;
		kNot.m_iOK						= NetError::NET_OK;

		MakeUnitInfoFromRow( kNot.m_kUnitInfo, kRow );

		kNot.m_wstrUnitCreateDate		= CX2OfflineDB::FormatDate( kRow.m_tRegDate );
		kNot.m_wstrUnitLastLoginDate	= CX2OfflineDB::FormatDate( kRow.m_tLastDate );
		kNot.m_iRealDataED				= kRow.m_iED;

		pDB->LoadInventorySizes( kRow.m_nUnitUID, kNot.m_mapInventorySlotSize );

		// No items yet - the inventory is phase 5. The sizes still have to be
		// right now, because CX2Unit::ResetInventory sizes every category from
		// this map and anything missing collapses to zero slots.
		kNot.m_mapItem.clear();

		kNot.m_iNumResurrectionStone		= 0;
		kNot.m_iNumAutoPaymentResStone		= 0;
		kNot.m_wstrAutoPayResStoneLastDate	= CX2OfflineDB::FormatDate( kRow.m_tRegDate );

		kNot.m_vecSkillAcquired.clear();
		kNot.m_vecSkillUnsealed.clear();
		kNot.m_cSkillNoteMaxPageNum			= 0;
		kNot.m_mapSkillNote.clear();

		// Trade block is the new-character restriction. Off: it exists to stop
		// gold sellers, and there is nobody to trade with offline. Leaving it
		// false also keeps the client away from ConvertStringToCTime on
		// m_wstrUnitCreateDate.
		kNot.m_bIsNewUnitTradeBlock			= false;
		kNot.m_iNewUnitTradeBlockDay		= 0;
		kNot.m_iNewUnitTradeBlockUnitClass	= 0;

		Reply( kSes, EGS_SELECT_UNIT_1_NOT, kNot );
	}

	//////////////////////////////////////////////////////////////////////////
	// 2 - quests
	{
		KEGS_SELECT_UNIT_2_NOT kNot;
		kNot.m_iOK = NetError::NET_OK;
		// m_vecQuest / m_vecCompletQuest / m_mapRandomQuestList: phase 6

		Reply( kSes, EGS_SELECT_UNIT_2_NOT, kNot );
	}

	//////////////////////////////////////////////////////////////////////////
	// 3 - missions, titles, pets
	{
		KEGS_SELECT_UNIT_3_NOT kNot;
		kNot.m_iOK				= NetError::NET_OK;
		kNot.m_iSummonedPetUID	= 0;

		Reply( kSes, EGS_SELECT_UNIT_3_NOT, kNot );
	}

	//////////////////////////////////////////////////////////////////////////
	// 4 - messenger, blacklist, deny options, time events
	{
		KEGS_SELECT_UNIT_4_NOT kNot;
		kNot.m_iOK					= NetError::NET_OK;
		kNot.m_bIsRecommend			= false;
		kNot.m_iRecommendUnitUID	= 0;

		// The live HP/MP. Phase 2 left this container deliberately empty,
		// because a zeroed entry is worse than none: the handler at
		// X2StateServerSelect.cpp:3115 pushes it straight into
		// CX2GageManager, so an empty max HP would show up as an empty
		// health bar and would send a battlefield character down the
		// "dead, go back to the village" branch of
		// Handler_EGS_GET_MY_INVENTORY_ACK. Phase 3 has a real stat table,
		// so it can be filled properly - and the studio's own
		// ASSERT( !empty() ) right above that line stops firing.
		KGamePlayStatus kStatus;
		MakeGamePlayStatus( kRow, kStatus );
		kNot.m_kGamePlayStatus.Set( kStatus );

		Reply( kSes, EGS_SELECT_UNIT_4_NOT, kNot );
	}

	//////////////////////////////////////////////////////////////////////////
	// 5 - shop agency, henir, one-shot events
	{
		KEGS_SELECT_UNIT_5_NOT kNot;
		kNot.m_iOK				= NetError::NET_OK;
		kNot.m_bIsSpiritUpdated	= false;

		// The cash shop is phase 7, but the flag has to be true now or the shop
		// button is inert and there is no way to find out later whether the
		// rest of the plumbing works.
		kNot.m_bCashShopOpen	= true;
		kNot.m_bFirstSelect		= false;

		Reply( kSes, EGS_SELECT_UNIT_5_NOT, kNot );
	}
}

#endif SERV_IRUHADEV_OFFLINE
