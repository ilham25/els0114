#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-03
// Description: Offline mode - quest state for the one character that is
//              logged in (phase 6).
//
//              This is KncWX2Server/GameServer/UserQuestManager.cpp reduced to
//              a single user: the map of quests in progress, the map of quests
//              finished, and the handful of game events that move a sub-quest
//              counter. The *templets* are not duplicated - they are the
//              client's own CX2QuestManager, loaded out of the .kom scripts
//              like every other table this project reuses.
//
//              Two invariants carried over from the real server, both easy to
//              break and both silent when broken:
//
//              * KQuestInstance::m_vecSubQuestInstance is POSITIONAL. Entry i
//                describes QuestTemplet::m_vecSubQuest[i]. Every server handler
//                indexes it that way (UserQuestManager.cpp:1446 and its
//                neighbours), and so does the client's own
//                CX2QuestManager::SetKQuestInstance. The vector is therefore
//                rebuilt from the templet on load rather than from whatever
//                order SQLite handed the rows back in.
//
//              * A sub-quest only advances when every sub-quest in an EARLIER
//                group is already done - QuestTemplet::m_mapSubQuestGroup, the
//                REFORM_QUEST staging that makes a quest read as steps rather
//                than as a checklist. CheckBeforeGroup is that rule.
//
//              Item-collection sub-quests are the exception to all of it: they
//              hold no counter at all. The client decides whether one is done
//              by counting its own inventory (CX2QuestManager::SubQuestInst::
//              IsComplete, X2QuestManager.cpp:2716), and so does this class -
//              which is why the two never disagree about a quest the player is
//              holding items for.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineDB.h"

class CX2OfflineQuest
{
public:
	enum
	{
		/// KUserQuestManager's MAX_ING_QUEST_NUM, the cap ERR_QUEST_28 reports.
		MAX_ING_QUEST_NUM	= 30,

		/// CXSLQuestManager::QE_DAILY_REPEAT_HOUR - a daily quest becomes
		/// available again at 6am, not at midnight.
		DAILY_REPEAT_HOUR	= 6,
	};

public:
	static CX2OfflineQuest*	Instance();
	static void				Release();

	/// Read this character's quests out of SQLite. Cheap to call repeatedly:
	/// it returns immediately when the character is already loaded.
	bool	Load( UidType nUnitUID );
	void	Clear();

	UidType	GetUnitUID() const					{ return m_nUnitUID; }

	//////////////////////////////////////////////////////////////////////////
	// reads - what EGS_SELECT_UNIT_2_NOT carries

	void	GetQuestInstances( OUT std::vector< KQuestInstance >& vecOut ) const;
	void	GetCompleteQuests( OUT std::vector< KCompleteQuestInfo >& vecOut ) const;

	bool	IsQuesting( int iQuestID ) const;
	int		GetCompleteCount( int iQuestID ) const;

	//////////////////////////////////////////////////////////////////////////
	// the packet-driven half

	/// Accept a quest. Returns NetError::NET_OK and fills kOut, or an
	/// ERR_QUEST_* the caller puts straight in the ACK.
	///
	/// bAfterQuest is the real server's IsAfterQuest: false means "the player
	/// clicked this on an NPC", which is the only case where the start-NPC is
	/// checked. True is a chained accept, where there is no NPC to check.
	///
	/// bQuiet suppresses the per-refusal log lines. CheckAutoOpen offers every
	/// epic quest in the game and expects almost all of them to be refused, so
	/// logging each one would bury the log; a player clicking a quest on an NPC
	/// expects exactly one line saying why it did not take.
	int		Accept( int iQuestID, int iTalkNPCID, const KOfflineUnitRow& kRow,
					bool bAfterQuest, OUT KQuestInstance& kOut, bool bQuiet = false );

	//////////////////////////////////////////////////////////////////////////
	/// Where an epic quest is allowed to open itself.
	enum AUTO_OPEN_PLACE
	{
		AOP_BEFORE_QUEST,	///< its prerequisites are all finished; no place test
		AOP_VILLAGE,		///< QuestCondition::m_setEnableVillage
		AOP_DUNGEON,		///< QuestCondition::m_setEnableDungeon
		AOP_BATTLE_FIELD,	///< QuestCondition::m_setEnableBattleField

		/// Named in the just-finished quest's m_vecAfterQuestID. `iPlaceID` is
		/// that quest's ID.
		///
		/// THIS IS THE ONE THAT CARRIES THE STORY. The three place sweeps only
		/// cover ten quests in the whole game, and the prerequisite sweep needs
		/// a before-list most epic quests do not have: 11005 '[Field] Thief
		/// Pursuit' has no prerequisite, no opening village and no opening
		/// dungeon, so the only thing that can ever start it is 11000 listing it
		/// here.
		AOP_AFTER_QUEST,
	};

	/// Epic quests the character has just qualified for - accepted, persisted,
	/// and handed back so the caller can announce them.
	///
	/// THIS IS HOW THE STORY CHAIN IS HANDED OUT. An epic quest is not offered
	/// by an NPC and the player never asks for it: the GameServer walks its own
	/// epic list and accepts on the player's behalf, from four places -
	/// KUserQuestManager::CheckEpicQuest_SuccessStateChange (a village,
	/// UserQuestManager.cpp:5970), CheckEpicQuest_EnterDungeon (:7363),
	/// CheckEpicQuest_EnterBattleField (:7428) and
	/// CheckEpicQuest_NewQuestByBeforeQuest (:6034, run at login and after every
	/// completion). Without it a character finishes the tutorial, walks into
	/// Ruben and is given nothing at all, because there is nobody to ask.
	void	CheckAutoOpen( AUTO_OPEN_PLACE ePlace, int iPlaceID, const KOfflineUnitRow& kRow,
						   OUT std::vector< KQuestInstance >& vecOpened );

	/// How many quests in the whole script list this one as a prerequisite.
	///
	/// Purely a diagnostic, and a decisive one: after a quest is handed in, a
	/// zero here means the chain does not continue automatically at all and the
	/// next step must be an NPC-offered quest the player has to click - which is
	/// a completely different thing to fix than a chain sweep that ran and
	/// refused everything.
	int		CountWaitingOn( int iQuestID ) const;

	/// Everything EGS_QUEST_COMPLETE_REQ verifies before any reward is paid.
	int		CheckCompletable( int iQuestID, const KOfflineUnitRow& kRow ) const;

	/// Move the quest from "in progress" to "completed", bumping the count and
	/// stamping the date. Persists both halves.
	bool	Finish( int iQuestID, OUT KCompleteQuestInfo& kOut );

	bool	GiveUp( int iQuestID );

	//////////////////////////////////////////////////////////////////////////
	// the event-driven half. Every one of these persists what it changed and
	// hands back the quests to put in EGS_UPDATE_QUEST_NOT - empty when nothing
	// moved, which is the common case and is why the caller must not send the
	// packet unconditionally.

	void	OnTalkNPC( int iNPCID, int iVillageMapID, const KOfflineUnitRow& kRow,
					   OUT std::vector< KQuestInstance >& vecChanged );

	void	OnNpcDie( int iDungeonID, char cDifficulty, int iBattleFieldID, int iNpcID,
					  const KOfflineUnitRow& kRow,
					  OUT std::vector< KQuestInstance >& vecChanged );

	/// iDamage is the total this character dealt over the run, for
	/// SQT_DUNGEON_DAMAGE - which is a ceiling, not a floor. See the case body.
	void	OnDungeonClear( int iDungeonID, char cDifficulty, char cRank, int iPlayTime,
							int iDamage, const KOfflineUnitRow& kRow,
							OUT std::vector< KQuestInstance >& vecChanged );

	void	OnVisitVillage( int iMapID, const KOfflineUnitRow& kRow,
							OUT std::vector< KQuestInstance >& vecChanged );

	void	OnVisitField( int iBattleFieldID, const KOfflineUnitRow& kRow,
						  OUT std::vector< KQuestInstance >& vecChanged );

	void	OnVisitDungeon( int iDungeonID, char cDifficulty, const KOfflineUnitRow& kRow,
							OUT std::vector< KQuestInstance >& vecChanged );

	/// The NPCs a room or field just spawned. SQT_FIND_NPC completes on seeing
	/// one, not on killing it.
	void	OnFindNPC( const std::vector< int >& vecNpcID, bool bDungeon,
					   int iDungeonID, char cDifficulty, int iBattleFieldID,
					   const KOfflineUnitRow& kRow,
					   OUT std::vector< KQuestInstance >& vecChanged );

	/// kRow must already carry the NEW level.
	void	OnLevelUp( const KOfflineUnitRow& kRow,
					   OUT std::vector< KQuestInstance >& vecChanged );

	//////////////////////////////////////////////////////////////////////////
	// "you used a feature" - the steps that are finished somewhere other than
	// a map. These are ticked from the packet handler for the feature itself,
	// the way the real server does it, and NOT from any quest-shaped event:
	// there is no event to hang them on, which is why they were the last group
	// still silently stuck.

	/// One call per skill point actually spent - the server calls its own
	/// handler once per skill LEVEL gained, in a loop
	/// (GSUserGameCommon.cpp:4637), not once per request.
	void	OnUseSkillPoint( int iSkillID, const KOfflineUnitRow& kRow,
							 OUT std::vector< KQuestInstance >& vecChanged );

	/// The skills that went from unlearned to learned in one request.
	void	OnLearnNewSkill( const std::vector< int >& vecSkillID,
							 const KOfflineUnitRow& kRow,
							 OUT std::vector< KQuestInstance >& vecChanged );

	/// A skill fired in a dungeon. The caller decides whether this dungeon
	/// counts at all; the shipped server excludes the tutorial, Henir, Ruben
	/// and the training camp (GSUserGameCommon.cpp:8846).
	void	OnUseSkill( int iSkillID, const KOfflineUnitRow& kRow,
						OUT std::vector< KQuestInstance >& vecChanged );

	void	OnUseItem( int iItemID, int iDungeonID, char cDifficulty,
					   int iBattleFieldID, const KOfflineUnitRow& kRow,
					   OUT std::vector< KQuestInstance >& vecChanged );

	/// One enhancement attempt that changed the item's level, from the
	/// blacksmith. iEnchantLevel is the level AFTER the attempt, and the match
	/// is exact: a step that asks for +5 is not satisfied by +6, which is how
	/// KUserQuestManager::Handler_OnEnchantItem compares it
	/// (UserQuestManager.cpp:2943).
	///
	/// The real server calls this from DBE_ENCHANT_ITEM_ACK, i.e. on every
	/// result except "no change" - a no-change attempt never reaches the DB
	/// round trip - so the caller makes that same exclusion rather than this
	/// function guessing at it.
	void	OnEnchantItem( int iItemID, int iEnchantLevel, const KOfflineUnitRow& kRow,
						   OUT std::vector< KQuestInstance >& vecChanged );

	/// One socketing request, from the magic-stone NPC. iSocketUseCount is how
	/// many slots THIS request filled, not the item's running total - the
	/// counter accumulates across requests and is clamped to what the step
	/// asks for (KUserQuestManager::Handler_OnSocketItem,
	/// UserQuestManager.cpp:3066).
	void	OnSocketItem( int iItemID, int iSocketUseCount, const KOfflineUnitRow& kRow,
						  OUT std::vector< KQuestInstance >& vecChanged );


	//////////////////////////////////////////////////////////////////////////

	/// The quest items this kill should scatter, if any.
	/// KRoomUser::GetQuestDropItemInDungeon / ...InBattleField, reduced to one
	/// player: for every SQT_QUEST_ITEM_COLLECTION sub-quest in progress whose
	/// dungeon / field / monster filters match, roll its own drop rate and, if
	/// it hits, drop its item - but never past the number the sub-quest asks
	/// for, counting what the bag already holds.
	void	GetQuestItemDrops( int iDungeonID, char cDifficulty, int iBattleFieldID,
							   int iNpcID, const KOfflineUnitRow& kRow,
							   OUT std::vector< int >& vecItemID );

	/// item ID -> how many a completing quest hands back in. Both collection
	/// sub-quest types; CXSLQuestManager::GetQuestConditionItemList verbatim.
	static void	GetConditionItems( int iQuestID, OUT std::map< int, int >& mapOut );

	/// Does this quest's reward change the character's class, and to what?
	/// CX2Unit::UC_NONE when it does not.
	static int	GetChangeUnitClass( int iQuestID );

	//////////////////////////////////////////////////////////////////////////
	// templet access, so the handlers do not each re-check g_pData

	static const CX2QuestManager::QuestTemplet*		Templet( int iQuestID );
	static const CX2QuestManager::SubQuestTemplet*	SubTemplet( int iSubQuestID );

	/// One-shot census of the quest script, written to the log the first time a
	/// character's quests are loaded.
	///
	/// It exists because "no quest appeared" has three completely different
	/// causes that look identical from the outside: the quest Lua did not load,
	/// or it loaded but every templet is filtered out, or it loaded and is
	/// visible and no quest names this place. The third is a data question; the
	/// first two are bugs, and only a count can tell them apart.
	///
	/// The filter worth naming is `ADD_SERVER_GROUP` (on, `Always.h:1707`):
	/// `CX2QuestManager::GetQuestTemplet` returns NULL for any quest whose
	/// `m_iServerGroupID` is neither -1 nor the group the client believes it is
	/// on ([X2QuestManager.cpp:831](X2Lib/X2QuestManager.cpp#L831)). The offline
	/// login reports group 1; nothing offline sets the client's own idea of it,
	/// so if those disagree, every quest in the game silently disappears - from
	/// this code and from the client's own NPC lists alike.
	static void	LogTempletCensus();

private:
	CX2OfflineQuest();
	~CX2OfflineQuest();

	/// Build a fresh instance for a quest, one sub-quest entry per templet
	/// entry, in templet order.
	static void	MakeInstance( const CX2QuestManager::QuestTemplet* pTemplet,
							  UidType nUnitUID, OUT KQuestInstance& kOut );

	/// True when every sub-quest in a group numbered lower than this one's is
	/// already done. KUserQuestManager::CheckCompleteSubQuest_BeforGroup.
	bool	CheckBeforeGroup( const CX2QuestManager::QuestTemplet* pTemplet,
							  int iSubQuestID ) const;

	/// Why nothing offline can ever advance this clear type, or NULL when
	/// something can. Reported by LogTempletCensus next to how many reachable
	/// steps use the type, which is the only way to tell a gap that matters
	/// from an enum value the shipped script never uses.
	static const wchar_t*	IsClearTypeUndriven( int eClearType );

	/// CXSLQuestManager::IsExistDungeonInSubQuest. iDungeonID is already
	/// dungeon + difficulty, which is how the server passes it around.
	static bool	IsExistDungeonInSub( const CX2QuestManager::SubQuestTemplet* pSub,
									 int iDungeonID );

	/// Is a sub-quest done, by the same rule the client draws it with -
	/// inventory for the collection types, the stored flag for everything else.
	static bool	IsSubComplete( const CX2QuestManager::SubQuestTemplet* pSub,
							   const KSubQuestInstance& kInst );

	/// Common preamble to every event handler: skip a quest whose templet is
	/// missing, whose sub-quest list is empty, or which is an epic quest the
	/// character is not high enough to advance.
	static bool	IsQuestAdvanceable( const CX2QuestManager::QuestTemplet* pTemplet,
									int iLevel );

	/// Push a quest into the changed list at most once, and write it back.
	void	MarkChanged( const KQuestInstance& kInst,
						 OUT std::vector< KQuestInstance >& vecChanged );

	bool	Save( int iQuestID ) const;

private:
	static CX2OfflineQuest*					ms_pInstance;

	UidType									m_nUnitUID;
	std::map< int, KQuestInstance >			m_mapQuesting;
	std::map< int, KCompleteQuestInfo >		m_mapComplete;
};

#endif SERV_IRUHADEV_OFFLINE
