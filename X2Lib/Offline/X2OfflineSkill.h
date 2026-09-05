#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-02
// Description: Offline mode - the skill tree (phase 5).
//
//              KncWX2Server/GameServer/UserSkillTree.cpp's KUserSkillTree,
//              reduced to what one character needs and backed by the
//              `unit_skill` table.
//
//              The tree itself - which skills exist, what they cost, what
//              precedes what, what each class starts with - is client data.
//              CX2SkillTree loads the same SkillData.lua / SkillTree scripts
//              the real CXSLSkillTree read, so every rule here reads out of
//              g_pData->GetSkillTree() and nothing is duplicated. That includes
//              two things worth naming:
//
//              * the SP cost of a skill - m_iRequireLearnSkillPoint and
//                m_iRequireUpgradeSkillPoint on the client's own SkillTemplet,
//                combined exactly as KUserSkillTree::GetNecessarySkillPoint
//                (UserSkillTree.cpp:1225) does it;
//              * the SP a level-up grants - the Lua function
//                `CalcLevelUpIncreaseSkillPoint( level )`, which is what
//                CX2SkillTree::GetCalcInitSkillPoint already calls
//                (X2SkillTree.cpp:4044). The server reached the same number
//                through CXSLSkillTree::GetCalcLevelUpIncreaseSkillPoint.
//
//              Cash skill points are always zero and always expired. There is
//              no billing offline, so every cost is paid in plain SP - which is
//              the same branch the real server takes for an account with no
//              cash-skill ticket, not a special case of ours.
//
//              Skill slot B is open permanently, matching the existing
//              SERV_IRUHADEV_SKILL_SLOT_B_FREE mod: the real
//              KUserSkillTree::ExpandSkillSlotB under that flag writes the
//              sentinel end date 2049-12-31 and never clears the B slots, so
//              the offline server reports the same sentinel and accepts B-slot
//              assignments without a purchase.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineDB.h"

class CX2OfflineSkill
{
public:
	enum
	{
		/// CX2UserSkillTree::MAX_SKILL_SLOT - four A slots then four B slots.
		MAX_SKILL_SLOT		= CX2UserSkillTree::MAX_SKILL_SLOT,

		/// KUnitSkillData::EQUIPPED_SKILL_SLOT_COUNT
		SLOT_COUNT_PER_SET	= KUnitSkillData::EQUIPPED_SKILL_SLOT_COUNT,
	};

	/// The permanent sentinel KUserSkillTree::ExpandSkillSlotB writes under
	/// SERV_IRUHADEV_SKILL_SLOT_B_FREE (UserSkillTree.cpp:771). Anything more
	/// than a year out reads as SSBES_PERMANENT.
	static const wchar_t*	SLOT_B_END_DATE;

public:
	static CX2OfflineSkill*	Instance();
	static void				Release();

	/// Load one character's skills. Idempotent for the same unit.
	bool	Load( UidType nUnitUID );
	void	Clear();

	UidType	GetUnitUID() const					{ return m_nUnitUID; }

	//////////////////////////////////////////////////////////////////////////
	// reads

	/// EGS_SELECT_UNIT_1_NOT::m_vecSkillAcquired. Level-0 rows go out too: the
	/// client's SetAcquiredSkill copies the map verbatim and a reset skill at
	/// level 0 is a different thing from one that was never learned.
	void	GetAcquiredSkills( OUT std::vector< KUserSkillData >& vecOut ) const;

	/// KUnitInfo::m_UnitSkillData - the two equipped-slot arrays, the slot B
	/// end date, and the passive list the room packets carry.
	void	FillUnitSkillData( OUT KUnitSkillData& kOut ) const;

	int		GetSkillLevel( int iSkillID ) const;
	int		GetSkillCSPoint( int iSkillID ) const;
	int		GetSlotSkillID( int iSlot ) const;

	/// Which slot holds this skill, or -1. KUserSkillTree::GetSlotID.
	int		GetSlotOfSkill( int iSkillID ) const;

	//////////////////////////////////////////////////////////////////////////
	// skill unsealing (phase 21)

	/// EGS_SELECT_UNIT_1_NOT::m_vecSkillUnsealed. short, not int, because that
	/// is what the packet carries.
	void	GetUnsealedSkills( OUT std::vector< short >& vecOut ) const;

	bool	IsSkillUnsealed( int iSkillID ) const;

	/// Unlock one skill and persist it. False when it was already unlocked, so
	/// the caller can refuse the item instead of eating it for nothing.
	bool	UnsealSkill( int iSkillID );

	/// Which skill a "secret manual" unlocks for this character, or 0. This is
	/// the client's own CX2SkillTree::GetUnsealSkillItemInfo - SkillData.lua's
	/// AddSealSkillInfo rows are client data, so nothing here is invented. It
	/// resolves against the *currently selected* unit's class, exactly as the
	/// client did a moment earlier when it decided to send the packet.
	static int	SkillForUnsealItem( int iItemID );

	//////////////////////////////////////////////////////////////////////////
	// the skill note (phase 22)

	/// EGS_SELECT_UNIT_1_NOT::m_cSkillNoteMaxPageNum - how many memo pages this
	/// character owns. 0 until it uses a skill note, which is the state the
	/// client reads as "no note" and hides the UI for.
	char	GetSkillNoteMaxPage() const			{ return m_cSkillNoteMaxPage; }

	/// EGS_SELECT_UNIT_1_NOT::m_mapSkillNote - page index -> memo item ID.
	const std::map< char, int >&	GetSkillNotes() const	{ return m_mapSkillNote; }

	/// KUserSkillTree::GetExpandSkillNotePage (UserSkillTree.cpp:907), verbatim:
	/// the page count is a function of level alone - level/10 of 2..6 gives 1..5
	/// pages and anything below level 20 gives none. Returns 0 when the level is
	/// too low, which is the same "false" the server returns.
	static char	ExpandSkillNotePageForLevel( int iLevel );

	/// Absolute set, matching KUserSkillTree::UpdateSkillNoteMaxPageNum. False
	/// when it would not actually grow, so a second note is not eaten.
	bool	SetSkillNoteMaxPage( char cMaxPage );

	/// EGS_REG_SKILL_NOTE_MEMO_REQ. Returns a NetError code; NET_OK on success,
	/// and the page has to already exist and the memo not already be registered,
	/// the same two checks GSUserGameCommon.cpp:7275-7298 makes.
	int		RegisterSkillNoteMemo( int iPage, int iMemoID );

	//////////////////////////////////////////////////////////////////////////
	// writes

	/// EGS_GET_SKILL_REQ. mapInOut is the client's requested end state; on
	/// success each entry comes back with m_iBeforeSkillLevel filled in and
	/// m_iSpendSkillCSPoint at zero, and iSPLeft is what remains. Returns a
	/// NetError code, NET_OK on success.
	int		LearnSkills( int iUnitLevel, int iUnitClass,
						 IN OUT std::map< int, KGetSkillInfo >& mapInOut,
						 IN int iSPAvailable, OUT int& iSPLeft );

	/// EGS_RESET_SKILL_REQ - one skill back down. iSPRefund is what to add to
	/// the character's SP. Returns a NetError code.
	int		ResetSkill( int iSkillID, int iUnitClass, OUT int& iSPRefund );

	/// EGS_INIT_SKILL_TREE_REQ - the whole tree back to the class defaults.
	/// iSPTotal comes back as the SP the character should now have, which is
	/// every point its levels ever granted.
	int		InitSkillTree( int iUnitLevel, int iUnitClass, OUT int& iSPTotal );

	/// EGS_CHANGE_SKILL_SLOT_REQ, including the implicit swap the ACK's
	/// m_iSlotID2 / m_iSkillID2 describe. Returns a NetError code.
	int		ChangeSkillSlot( int iSlotID, int iSkillID,
							 OUT int& iSlotID2, OUT int& iSkillID2 );

	/// Give a new character the skills its class starts with. Read out of the
	/// client's own default-skill map, which is where
	/// CX2UserSkillTree::SetDefaultSkill reads them too - the DataBase/
	/// snapshot's gup_create_unit hardcodes four pre-2013 skill IDs that do not
	/// exist in this build's SKILL_ID enum, so it is not a usable source.
	bool	SeedDefaultSkills( UidType nUnitUID, int iUnitClass );

	/// SP for levelling from iFromLevel to iToLevel, summed out of the client's
	/// own `CalcLevelUpIncreaseSkillPoint` Lua function. Zero when the levels
	/// are equal, so a dungeon that did not level anybody costs nothing.
	static int	SkillPointForLevelUp( int iFromLevel, int iToLevel );

	/// Total SP a character of this level has ever been granted, for the full
	/// tree reset. CX2SkillTree::GetCalcInitSkillPoint, same function.
	static int	SkillPointForLevel( int iLevel );

	//////////////////////////////////////////////////////////////////////////
	/// The two consumables the real server requires. Values cited from
	/// KncWX2Server/Common/X2Data/XSLItem.h:190 and :195 rather than included
	/// from it - that header pulls a server class into X2Lib.
	enum RESET_ITEM
	{
		RI_RESET_ONE_SKILL		= 200730,		///< CXSLItem::CI_SKILL_RESET_ITEM
		RI_RESET_WHOLE_TREE		= 203800,		///< CXSLItem::CI_SKILL_INIT_ITEM
	};

private:
	CX2OfflineSkill();
	~CX2OfflineSkill();

	struct KSkillState
	{
		int		m_iLevel;
		int		m_iCSPoint;

		KSkillState() : m_iLevel( 0 ), m_iCSPoint( 0 ) {}
	};

	/// The client's tree data, or NULL. Every caller has to cope: a skill ID in
	/// the save file that the client cannot resolve means the save and the
	/// scripts disagree, which is worth a log line rather than a crash.
	static const CX2SkillTree::SkillTemplet*		Templet( int iSkillID );
	static const CX2SkillTree::SkillTreeTemplet*	TreeTemplet( int iUnitClass, int iSkillID );

	void	Persist( int iSkillID );

	bool	IsDefaultSkill( int iSkillID ) const;
	bool	IsAllFollowingSkillLevelZero( int iUnitClass, int iSkillID ) const;

private:
	static CX2OfflineSkill*	ms_pInstance;

	UidType								m_nUnitUID;
	std::map< int, KSkillState >		m_mapSkill;
	std::set< int >					m_setUnsealedSkill;
	std::map< char, int >			m_mapSkillNote;
	char							m_cSkillNoteMaxPage;
	int									m_aiSkillSlot[ CX2UserSkillTree::MAX_SKILL_SLOT ];
};

#endif SERV_IRUHADEV_OFFLINE
