#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-03
// Description: Offline mode - titles and the missions that award them
//              (phase 6).
//
//              KncWX2Server/GameServer/UserTitleManager.cpp for one character.
//              A title is earned by finishing a *mission*, which is built the
//              same way a quest is - a list of sub-missions, each with a clear
//              type and a condition - so this class is deliberately the same
//              shape as CX2OfflineQuest and is driven from the same events.
//
//              The one thing worth knowing before reading it: **a mission's ID
//              and the ID of the title it awards are the same number.** That is
//              not an assumption made here; the real server relies on it
//              outright - KUserTitleManager::CheckNewMission looks a mission up
//              in the *title* map to decide whether it is already done
//              (UserTitleManager.cpp:325), and CheckCompleteMission logs an
//              error if the two ever disagree (:1335). MissionTemplet still
//              carries m_iTitleID, and it is what gets granted, so the two are
//              kept separate in the code even though the data makes them equal.
//
//              What is NOT modelled, and why: every PVP clear type (PvP is out
//              of scope for the whole project), pet feeding, party-size
//              conditions, resurrection-stone counts and "die a certain way" -
//              each needs a subsystem that does not exist offline. Missions
//              using them are still tracked and still shown; they simply never
//              advance, which is what an unimplemented condition should look
//              like.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineDB.h"

class CX2OfflineTitle
{
public:
	static CX2OfflineTitle*	Instance();
	static void				Release();

	bool	Load( UidType nUnitUID );
	void	Clear();

	UidType	GetUnitUID() const					{ return m_nUnitUID; }

	//////////////////////////////////////////////////////////////////////////
	// reads - what EGS_SELECT_UNIT_3_NOT carries

	void	GetMissionInstances( OUT std::vector< KMissionInstance >& vecOut ) const;
	void	GetTitles( OUT std::vector< KTitleInfo >& vecOut ) const;

	bool	HasTitle( int iTitleID ) const;

	//////////////////////////////////////////////////////////////////////////

	/// Missions the character has just become eligible for. Called on login and
	/// again after anything that could change eligibility - a level, a class
	/// change, a title. New rows are persisted before they are returned.
	void	CheckNewMission( const KOfflineUnitRow& kRow,
							 OUT std::vector< KMissionInstance >& vecNew );

	/// Can this title be worn? Returns a NetError, so the caller can put it
	/// straight in EGS_EQUIP_TITLE_ACK. Title 0 means "take it off" and is
	/// always allowed.
	int		CheckEquip( int iTitleID, int iEquippedNow ) const;

	//////////////////////////////////////////////////////////////////////////
	// events. Same contract as CX2OfflineQuest's: what changed is persisted
	// before it is returned, and an empty vector means nothing moved.
	//
	// vecNewTitle collects titles the events just earned; they are already
	// written to SQLite and each wants an EGS_REWARD_TITLE_NOT.

	void	OnTalkNPC( int iNPCID, const KOfflineUnitRow& kRow,
					   OUT std::vector< KMissionInstance >& vecChanged,
					   OUT std::vector< KTitleInfo >& vecNewTitle );

	void	OnNpcDie( int iDungeonID, char cDifficulty, int iNpcID,
					  const KOfflineUnitRow& kRow,
					  OUT std::vector< KMissionInstance >& vecChanged,
					  OUT std::vector< KTitleInfo >& vecNewTitle );

	void	OnDungeonClear( int iDungeonID, char cDifficulty, char cRank,
							int iPlayTime, int iDamage, const KOfflineUnitRow& kRow,
							OUT std::vector< KMissionInstance >& vecChanged,
							OUT std::vector< KTitleInfo >& vecNewTitle );

	void	OnQuestComplete( int iQuestID, const KOfflineUnitRow& kRow,
							 OUT std::vector< KMissionInstance >& vecChanged,
							 OUT std::vector< KTitleInfo >& vecNewTitle );

	void	OnUseItem( int iItemID, const KOfflineUnitRow& kRow,
					   OUT std::vector< KMissionInstance >& vecChanged,
					   OUT std::vector< KTitleInfo >& vecNewTitle );

	/// The inventory-counted types (item collection, and "own N titles") have
	/// no event of their own - they become true when the bag or the title list
	/// changes. Called after a pickup and after a title is granted.
	void	ReCheckPassive( const KOfflineUnitRow& kRow,
							OUT std::vector< KMissionInstance >& vecChanged,
							OUT std::vector< KTitleInfo >& vecNewTitle );

	//////////////////////////////////////////////////////////////////////////

	static const CX2TitleManager::MissionTemplet*		Templet( int iMissionID );
	static const CX2TitleManager::SubMissionTemplet*	SubTemplet( int iSubMissionID );

	/// The end date a permanent title carries. KUserTitleManager::AddTitle
	/// decides m_bInfinity by comparing the date against 2040-01-01, so a
	/// permanent title needs a date past it rather than an empty string.
	static const wchar_t*	INFINITY_END_DATE;

private:
	CX2OfflineTitle();
	~CX2OfflineTitle();

	bool	Save( int iMissionID ) const;

	/// Every sub-mission done? Grant the title, drop the mission, and push the
	/// new KTitleInfo. KUserTitleManager::CheckCompleteMission.
	void	CheckComplete( int iMissionID, OUT std::vector< KTitleInfo >& vecNewTitle );

	/// Is a sub-mission satisfied? The collection types read the bag and the
	/// title-count type reads the title list; everything else is the flag.
	bool	IsSubComplete( const CX2TitleManager::SubMissionTemplet* pSub,
						   const KSubMissionInstance& kInst ) const;

	/// Does this sub-mission's dungeon filter accept the dungeon just played?
	/// The mission templets use a single dungeon plus a difficulty rather than
	/// the set the REFORM_QUEST sub-quests use.
	static bool	DungeonMatches( const CX2TitleManager::SubMissionTemplet* pSub,
								int iDungeonID, char cDifficulty );

	void	MarkChanged( const KMissionInstance& kInst,
						 OUT std::vector< KMissionInstance >& vecChanged );

private:
	static CX2OfflineTitle*					ms_pInstance;

	UidType									m_nUnitUID;
	std::map< int, KMissionInstance >		m_mapMission;
	std::map< int, KTitleInfo >				m_mapTitle;
};

#endif SERV_IRUHADEV_OFFLINE
