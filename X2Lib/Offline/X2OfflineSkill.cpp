#include "stdafx.h"
#include "X2OfflineSkill.h"

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-02
// Description: Offline mode - the skill tree (phase 5). See the header.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineLog.h"

CX2OfflineSkill* CX2OfflineSkill::ms_pInstance = NULL;

/*static*/ const wchar_t* CX2OfflineSkill::SLOT_B_END_DATE = L"2049-12-31 23:59:00";

CX2OfflineSkill::CX2OfflineSkill()
: m_nUnitUID( 0 )
, m_cSkillNoteMaxPage( 0 )
{
	for( int i = 0; i < MAX_SKILL_SLOT; ++i )
		m_aiSkillSlot[i] = 0;
}

CX2OfflineSkill::~CX2OfflineSkill()
{
}

CX2OfflineSkill* CX2OfflineSkill::Instance()
{
	if( NULL == ms_pInstance )
		ms_pInstance = new CX2OfflineSkill;

	return ms_pInstance;
}

void CX2OfflineSkill::Release()
{
	if( NULL == ms_pInstance )
		return;

	delete ms_pInstance;
	ms_pInstance = NULL;
}

//////////////////////////////////////////////////////////////////////////

/*static*/ const CX2SkillTree::SkillTemplet* CX2OfflineSkill::Templet( int iSkillID )
{
	if( NULL == g_pData || NULL == g_pData->GetSkillTree() )
		return NULL;

	return g_pData->GetSkillTree()->GetSkillTemplet( (CX2SkillTree::SKILL_ID)iSkillID );
}

/*static*/ const CX2SkillTree::SkillTreeTemplet* CX2OfflineSkill::TreeTemplet( int iUnitClass, int iSkillID )
{
	if( NULL == g_pData || NULL == g_pData->GetSkillTree() )
		return NULL;

	return g_pData->GetSkillTree()->GetSkillTreeTemplet( iUnitClass, (CX2SkillTree::SKILL_ID)iSkillID );
}

/*static*/ int CX2OfflineSkill::SkillPointForLevel( int iLevel )
{
	if( iLevel <= 1 )
		return 0;

	if( NULL == g_pData || NULL == g_pData->GetSkillTree() )
		return 0;

	// CX2SkillTree::GetCalcInitSkillPoint sums the client's own Lua
	// `CalcLevelUpIncreaseSkillPoint( level )` over 2..iLevel, which is the same
	// function CXSLSkillTree::GetCalcLevelUpIncreaseSkillPoint read on the
	// server. Calling it rather than reimplementing the sum is the whole point:
	// there is exactly one curve and both sides have to agree with it.
	return g_pData->GetSkillTree()->GetCalcInitSkillPoint( iLevel );
}

/*static*/ int CX2OfflineSkill::SkillPointForLevelUp( int iFromLevel, int iToLevel )
{
	if( iToLevel <= iFromLevel )
		return 0;

	return SkillPointForLevel( iToLevel ) - SkillPointForLevel( iFromLevel );
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineSkill::Load( UidType nUnitUID )
{
	if( 0 != nUnitUID && nUnitUID == m_nUnitUID )
		return true;

	Clear();

	if( 0 == nUnitUID )
		return false;

	std::vector< KOfflineSkillRow > vecRow;
	CX2OfflineDB::Instance()->LoadSkills( nUnitUID, vecRow );

	for( size_t i = 0; i < vecRow.size(); ++i )
	{
		const KOfflineSkillRow& kRow = vecRow[i];

		KSkillState kState;
		kState.m_iLevel		= kRow.m_iLevel;
		kState.m_iCSPoint	= kRow.m_iCSPoint;

		m_mapSkill[ kRow.m_iSkillID ] = kState;

		// A slot only counts when the skill in it is actually learned. A row
		// left pointing at a slot after its skill was reset to level 0 would
		// otherwise put an unusable skill on the bar.
		if( kRow.m_iSlot >= 0 && kRow.m_iSlot < MAX_SKILL_SLOT && kRow.m_iLevel > 0 )
			m_aiSkillSlot[ kRow.m_iSlot ] = kRow.m_iSkillID;
	}

	// Phase 21 / 22: the two sidecar tables, plus the page count that lives on
	// the unit row. Loaded here rather than at the packet handlers so every
	// reader gets them from one place, the way m_mapSkill already works.
	std::vector< int > vecUnsealed;
	CX2OfflineDB::Instance()->LoadUnsealedSkills( nUnitUID, vecUnsealed );

	for( size_t u = 0; u < vecUnsealed.size(); ++u )
		m_setUnsealedSkill.insert( vecUnsealed[u] );

	CX2OfflineDB::Instance()->LoadSkillNotes( nUnitUID, m_mapSkillNote );

	{
		KOfflineUnitRow kUnitRow;
		if( true == CX2OfflineDB::Instance()->LoadUnit( nUnitUID, kUnitRow ) )
			m_cSkillNoteMaxPage = (char)kUnitRow.m_iSkillNotePage;
	}

	m_nUnitUID = nUnitUID;

	CX2OfflineLog::Server( L"SKILL    loaded for unitUID=%I64d: %u skill row(s),"
		L" %u unsealed, %d note page(s) holding %u memo(s)",
		(__int64)nUnitUID, (unsigned int)vecRow.size(),
		(unsigned int)m_setUnsealedSkill.size(), (int)m_cSkillNoteMaxPage,
		(unsigned int)m_mapSkillNote.size() );

	return true;
}

void CX2OfflineSkill::Clear()
{
	m_nUnitUID = 0;
	m_mapSkill.clear();
	m_setUnsealedSkill.clear();
	m_mapSkillNote.clear();
	m_cSkillNoteMaxPage = 0;

	for( int i = 0; i < MAX_SKILL_SLOT; ++i )
		m_aiSkillSlot[i] = 0;
}

void CX2OfflineSkill::Persist( int iSkillID )
{
	std::map< int, KSkillState >::const_iterator mit = m_mapSkill.find( iSkillID );
	if( mit == m_mapSkill.end() )
		return;

	KOfflineSkillRow kRow;
	kRow.m_iSkillID	= iSkillID;
	kRow.m_iLevel	= mit->second.m_iLevel;
	kRow.m_iCSPoint	= mit->second.m_iCSPoint;
	kRow.m_iSlot	= GetSlotOfSkill( iSkillID );

	CX2OfflineDB::Instance()->SaveSkill( m_nUnitUID, kRow );
}

//////////////////////////////////////////////////////////////////////////

int CX2OfflineSkill::GetSkillLevel( int iSkillID ) const
{
	std::map< int, KSkillState >::const_iterator mit = m_mapSkill.find( iSkillID );
	if( mit == m_mapSkill.end() )
		return 0;

	return mit->second.m_iLevel;
}

int CX2OfflineSkill::GetSkillCSPoint( int iSkillID ) const
{
	std::map< int, KSkillState >::const_iterator mit = m_mapSkill.find( iSkillID );
	if( mit == m_mapSkill.end() )
		return 0;

	return mit->second.m_iCSPoint;
}

int CX2OfflineSkill::GetSlotSkillID( int iSlot ) const
{
	if( iSlot < 0 || iSlot >= MAX_SKILL_SLOT )
		return 0;

	return m_aiSkillSlot[ iSlot ];
}

int CX2OfflineSkill::GetSlotOfSkill( int iSkillID ) const
{
	if( 0 == iSkillID )
		return -1;

	for( int i = 0; i < MAX_SKILL_SLOT; ++i )
	{
		if( m_aiSkillSlot[i] == iSkillID )
			return i;
	}

	return -1;
}

bool CX2OfflineSkill::IsDefaultSkill( int iSkillID ) const
{
	if( NULL == g_pData || NULL == g_pData->GetSkillTree() )
		return false;

	return g_pData->GetSkillTree()->isDefaultSkill( (CX2SkillTree::SKILL_ID)iSkillID );
}

bool CX2OfflineSkill::IsAllFollowingSkillLevelZero( int iUnitClass, int iSkillID ) const
{
	// KUserSkillTree::IsAllFollowingSkillLevelZero (UserSkillTree.cpp:872). The
	// server iterates a vector of followers; under UPGRADE_SKILL_SYSTEM_2013
	// this build's SkillTreeTemplet carries a single m_iFollowingSkill instead,
	// so there is one to check.
	const CX2SkillTree::SkillTreeTemplet* pTree = TreeTemplet( iUnitClass, iSkillID );
	if( NULL == pTree )
		return true;

	if( 0 == pTree->m_iFollowingSkill )
		return true;

	return ( GetSkillLevel( pTree->m_iFollowingSkill ) <= 0 );
}

//////////////////////////////////////////////////////////////////////////

void CX2OfflineSkill::GetAcquiredSkills( OUT std::vector< KUserSkillData >& vecOut ) const
{
	vecOut.clear();

	std::map< int, KSkillState >::const_iterator mit;
	for( mit = m_mapSkill.begin(); mit != m_mapSkill.end(); ++mit )
	{
		KUserSkillData kData;
		kData.m_iSkillID		= (short)mit->first;
		kData.m_cSkillLevel		= (UCHAR)mit->second.m_iLevel;
		kData.m_cSkillCSPoint	= (UCHAR)mit->second.m_iCSPoint;

		vecOut.push_back( kData );
	}
}

void CX2OfflineSkill::FillUnitSkillData( OUT KUnitSkillData& kOut ) const
{
	kOut.Init();

	for( int i = 0; i < SLOT_COUNT_PER_SET; ++i )
	{
		const int iSkillA = GetSlotSkillID( i );
		const int iSkillB = GetSlotSkillID( i + SLOT_COUNT_PER_SET );

		kOut.m_aEquippedSkill[i].m_iSkillID			= (short)iSkillA;
		kOut.m_aEquippedSkill[i].m_cSkillLevel		= (UCHAR)GetSkillLevel( iSkillA );

		kOut.m_aEquippedSkillSlotB[i].m_iSkillID	= (short)iSkillB;
		kOut.m_aEquippedSkillSlotB[i].m_cSkillLevel	= (UCHAR)GetSkillLevel( iSkillB );
	}

	// Slot B is permanent, matching SERV_IRUHADEV_SKILL_SLOT_B_FREE. The state
	// and the date have to agree: the client derives the state from the date
	// when it is re-checked, and a date in the past with a PERMANENT state would
	// flip to expired on the next check.
	kOut.m_wstrSkillSlotBEndDate	= SLOT_B_END_DATE;
	kOut.m_cSkillSlotBExpirationState = (char)CX2UserSkillTree::SSBES_PERMANENT;

	// The passive list. Only read for units other than the player
	// (CX2Unit::UnitData::SetKRoomUserInfo gates it on the UID), so nothing
	// offline consumes it - filled anyway because leaving a field of a shared
	// struct empty "because nobody looks" is how phase 3's zeroed game stat
	// happened.
	std::map< int, KSkillState >::const_iterator mit;
	for( mit = m_mapSkill.begin(); mit != m_mapSkill.end(); ++mit )
	{
		if( mit->second.m_iLevel <= 0 )
			continue;

		const CX2SkillTree::SkillTemplet* pTemplet = Templet( mit->first );
		if( NULL == pTemplet )
			continue;

		switch( pTemplet->m_eType )
		{
		case CX2SkillTree::ST_PASSIVE_PHYSIC_ATTACK:
		case CX2SkillTree::ST_PASSIVE_MAGIC_ATTACK:
		case CX2SkillTree::ST_PASSIVE_MAGIC_DEFENCE:
		case CX2SkillTree::ST_PASSIVE_PHYSIC_DEFENCE:
		case CX2SkillTree::ST_PASSIVE:
			{
				KSkillData kData;
				kData.m_iSkillID	= (short)mit->first;
				kData.m_cSkillLevel	= (UCHAR)mit->second.m_iLevel;

				kOut.m_vecPassiveSkill.push_back( kData );
			}
			break;

		default:
			break;
		}
	}

	// The registered skill-note memos, and this one is not filled "anyway" - it
	// is load-bearing. CX2UserSkillTree::SetEqipSkillMemo is a plain assignment
	// (X2UserSkillTree.h:362) and both SetKUnitInfo (X2Unit.cpp:3267) and
	// SetKRoomUserInfo (X2Unit.cpp:3441) call it unconditionally, the latter
	// against the player's own CX2Unit. Leaving the vector empty therefore did not
	// merely fail to send the memos: entering a dungeon room erased the ones
	// EGS_REG_SKILL_NOTE_MEMO_ACK had just put on the unit, and
	// CX2GUUser::IsEquipSkillMemo (X2GUUser.cpp:5845) - which is what makes a
	// memo's effect actually apply in combat - went false in the dungeon the memo
	// was registered for. Invisibly, too: the skill-note UI reads
	// CX2Unit::m_mapSkillNote (X2Unit.cpp:1811), a different member fed by a
	// different packet, so the pages still showed their memos.
	//
	// Same page order as Handler_EGS_REG_SKILL_NOTE_MEMO_REQ sends, from the same
	// map. Order is not actually load-bearing - GetEqipSkillMemo is a linear
	// membership test - but the two lists having one shape is worth keeping.
	std::map< char, int >::const_iterator nit;
	for( nit = m_mapSkillNote.begin(); nit != m_mapSkillNote.end(); ++nit )
	{
		if( nit->second > 0 )
			kOut.m_vecSkillNote.push_back( nit->second );
	}
}

//////////////////////////////////////////////////////////////////////////

int CX2OfflineSkill::LearnSkills( int iUnitLevel, int iUnitClass,
								  IN OUT std::map< int, KGetSkillInfo >& mapInOut,
								  IN int iSPAvailable, OUT int& iSPLeft )
{
	iSPLeft = iSPAvailable;

	if( true == mapInOut.empty() )
		return NetError::ERR_SKILL_09;

	// Pass 1: validate the whole request and total the cost. Nothing is written
	// until it all checks out - a half-applied batch would leave SP spent on
	// skills the client does not think it bought.
	int iTotalSP = 0;

	std::map< int, KGetSkillInfo >::iterator mit;
	for( mit = mapInOut.begin(); mit != mapInOut.end(); ++mit )
	{
		const int iSkillID		= mit->first;
		const int iWantLevel	= mit->second.m_iSkillLevel;
		const int iHaveLevel	= GetSkillLevel( iSkillID );

		const CX2SkillTree::SkillTemplet* pTemplet = Templet( iSkillID );
		if( NULL == pTemplet )
		{
			CX2OfflineLog::Server( L"SKILL    refused: no templet for skill %d", iSkillID );
			return NetError::ERR_SKILL_01;
		}

		const CX2SkillTree::SkillTreeTemplet* pTree = TreeTemplet( iUnitClass, iSkillID );
		if( NULL == pTree )
		{
			CX2OfflineLog::Server( L"SKILL    refused: skill %d is not in class %d's tree",
				iSkillID, iUnitClass );
			return NetError::ERR_SKILL_01;
		}

		if( iWantLevel <= iHaveLevel )
			return NetError::ERR_SKILL_20;			///< nothing to buy, or a downgrade

		if( iWantLevel > pTree->m_iMaxSkillLevel )
			return NetError::ERR_SKILL_21;

		// The character level the requested level needs. The vector is indexed
		// from zero for skill level one.
		if( (int)pTemplet->m_vecRequireCharacterLevel.size() >= iWantLevel )
		{
			if( iUnitLevel < pTemplet->m_vecRequireCharacterLevel[ iWantLevel - 1 ] )
				return NetError::ERR_SKILL_03;
		}

		// The preceding skill has to be learned already, or be in this same
		// request - KUserSkillTree::IsAllPrecedingSkillLearned allows the batch
		// to satisfy its own prerequisites.
		if( pTree->m_iPrecedingSkill > 0 &&
			GetSkillLevel( pTree->m_iPrecedingSkill ) <= 0 &&
			mapInOut.find( pTree->m_iPrecedingSkill ) == mapInOut.end() )
		{
			return NetError::ERR_SKILL_07;
		}

		// GetNecessarySkillPoint (UserSkillTree.cpp:1225): a skill not learned
		// yet costs the learn point plus one upgrade point per level above one;
		// one already learned costs an upgrade point per level gained. The
		// cash-skill branch is not taken - CSP is always expired offline - so
		// every point comes out of plain SP, which is that function's own
		// IsCashSkillPointExpired() == true arm.
		if( iHaveLevel > 0 )
		{
			iTotalSP += pTemplet->m_iRequireUpgradeSkillPoint * ( iWantLevel - iHaveLevel );
		}
		else
		{
			iTotalSP += pTemplet->m_iRequireLearnSkillPoint +
						pTemplet->m_iRequireUpgradeSkillPoint * ( iWantLevel - 1 );
		}
	}

	if( iTotalSP > iSPAvailable )
	{
		CX2OfflineLog::Server( L"SKILL    refused: %d SP needed, %d available", iTotalSP, iSPAvailable );
		return NetError::ERR_SKILL_18;
	}

	// Pass 2: apply.
	for( mit = mapInOut.begin(); mit != mapInOut.end(); ++mit )
	{
		const int iSkillID = mit->first;

		mit->second.m_iSkillID			= iSkillID;
		mit->second.m_iBeforeSkillLevel	= GetSkillLevel( iSkillID );
		mit->second.m_iSpendSkillCSPoint = 0;			///< nothing is paid in CSP offline

		m_mapSkill[ iSkillID ].m_iLevel		= mit->second.m_iSkillLevel;
		m_mapSkill[ iSkillID ].m_iCSPoint	= 0;

		Persist( iSkillID );

		CX2OfflineLog::Server( L"SKILL    skill %d: level %d -> %d",
			iSkillID, mit->second.m_iBeforeSkillLevel, mit->second.m_iSkillLevel );
	}

	iSPLeft = iSPAvailable - iTotalSP;

	CX2OfflineLog::Server( L"SKILL    spent %d SP, %d left", iTotalSP, iSPLeft );

	return NetError::NET_OK;
}

//////////////////////////////////////////////////////////////////////////

int CX2OfflineSkill::ResetSkill( int iSkillID, int iUnitClass, OUT int& iSPRefund )
{
	iSPRefund = 0;

	const CX2SkillTree::SkillTemplet* pTemplet = Templet( iSkillID );
	if( NULL == pTemplet )
		return NetError::ERR_RESET_SKILL_00;

	if( NULL == TreeTemplet( iUnitClass, iSkillID ) )
		return NetError::ERR_RESET_SKILL_00;

	const int iHaveLevel = GetSkillLevel( iSkillID );
	if( iHaveLevel <= 0 )
		return NetError::ERR_RESET_SKILL_04;

	// Where the skill lands. A class default, or one that something else was
	// built on top of, cannot go below level 1 - CheckResetSkill's own rule,
	// and the client's ACK handler draws the same distinction.
	const bool bDefault	= IsDefaultSkill( iSkillID );
	const bool bBlocked	= ( false == IsAllFollowingSkillLevelZero( iUnitClass, iSkillID ) );

	const int iAfterLevel = ( true == bDefault || true == bBlocked ) ? 1 : 0;

	if( iAfterLevel >= iHaveLevel )
		return NetError::ERR_RESET_SKILL_04;

	// A skill that would vanish entirely cannot be sitting on the bar.
	if( iAfterLevel <= 0 && GetSlotOfSkill( iSkillID ) >= 0 )
		return NetError::ERR_RESET_SKILL_05;

	// The refund, from GSUserGameCommon.cpp:4800: one learn point for the first
	// level - only when the skill is being unlearned outright - and one upgrade
	// point for each level above that. The CSP subtraction the server does next
	// is a no-op here, because nothing offline is ever bought with CSP.
	for( int i = 0; i < iHaveLevel; ++i )
	{
		if( 0 == i )
		{
			if( iAfterLevel > 0 )
				continue;

			iSPRefund += pTemplet->m_iRequireLearnSkillPoint;
		}
		else
		{
			iSPRefund += pTemplet->m_iRequireUpgradeSkillPoint;
		}
	}

	m_mapSkill[ iSkillID ].m_iLevel		= iAfterLevel;
	m_mapSkill[ iSkillID ].m_iCSPoint	= 0;

	Persist( iSkillID );

	CX2OfflineLog::Server( L"SKILL    reset skill %d: level %d -> %d, +%d SP",
		iSkillID, iHaveLevel, iAfterLevel, iSPRefund );

	return NetError::NET_OK;
}

//////////////////////////////////////////////////////////////////////////

int CX2OfflineSkill::InitSkillTree( int iUnitLevel, int iUnitClass, OUT int& iSPTotal )
{
	iSPTotal = SkillPointForLevel( iUnitLevel );

	// Every learned skill and every slot go away, then the class defaults come
	// back at level 1 - which is what the client's ACK handler does on its own
	// side (Reset + SetDefaultSkill), so the two have to end up agreeing.
	m_mapSkill.clear();

	for( int i = 0; i < MAX_SKILL_SLOT; ++i )
		m_aiSkillSlot[i] = 0;

	CX2OfflineDB::Instance()->ClearSkills( m_nUnitUID );

	if( false == SeedDefaultSkills( m_nUnitUID, iUnitClass ) )
	{
		CX2OfflineLog::Server( L"SKILL    WARNING class %d has no default-skill list - the tree is"
			L" now empty", iUnitClass );
	}

	CX2OfflineLog::Server( L"SKILL    tree reset for unitUID=%I64d, SP restored to %d",
		(__int64)m_nUnitUID, iSPTotal );

	return NetError::NET_OK;
}

//////////////////////////////////////////////////////////////////////////

int CX2OfflineSkill::ChangeSkillSlot( int iSlotID, int iSkillID,
									  OUT int& iSlotID2, OUT int& iSkillID2 )
{
	// KUserSkillTree::ChangeSkillSlot (UserSkillTree.cpp:1958) verbatim. The
	// second slot is not in the request: it is derived - where the incoming
	// skill currently sits, and what the target slot currently holds - which is
	// how one drag both places a skill and vacates wherever it came from.
	iSlotID2	= GetSlotOfSkill( iSkillID );
	iSkillID2	= GetSlotSkillID( iSlotID );

	if( iSlotID < 0 || iSlotID >= MAX_SKILL_SLOT )
		return NetError::ERR_SKILL_09;

	if( 0 == iSkillID )
	{
		m_aiSkillSlot[ iSlotID ] = 0;
	}
	else
	{
		if( GetSkillLevel( iSkillID ) <= 0 )
			return NetError::ERR_SKILL_10;

		// Slot B is permanently open under SERV_IRUHADEV_SKILL_SLOT_B_FREE, so
		// the server's end-date check here can only pass.
	}

	if( iSlotID2 >= 0 && iSlotID2 < MAX_SKILL_SLOT )
	{
		if( 0 == iSkillID2 )
		{
			m_aiSkillSlot[ iSlotID2 ] = 0;
		}
		else if( GetSkillLevel( iSkillID2 ) <= 0 )
		{
			return NetError::ERR_SKILL_10;
		}
		else
		{
			m_aiSkillSlot[ iSlotID2 ] = iSkillID2;
		}
	}

	m_aiSkillSlot[ iSlotID ] = iSkillID;

	// Persist both ends. SetSkillSlot vacates the slot before claiming it, so
	// the order below cannot leave two slots naming the same skill.
	CX2OfflineDB::Instance()->SetSkillSlot( m_nUnitUID, iSlotID, iSkillID );

	if( iSlotID2 >= 0 && iSlotID2 < MAX_SKILL_SLOT )
		CX2OfflineDB::Instance()->SetSkillSlot( m_nUnitUID, iSlotID2, iSkillID2 );

	CX2OfflineLog::Server( L"SKILL    slot %d <- skill %d (slot %d <- skill %d)",
		iSlotID, iSkillID, iSlotID2, iSkillID2 );

	return NetError::NET_OK;
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineSkill::SeedDefaultSkills( UidType nUnitUID, int iUnitClass )
{
	if( NULL == g_pData || NULL == g_pData->GetSkillTree() )
		return false;

	const std::map< int, std::vector<int> >& mapDefault =
		g_pData->GetSkillTree()->GetMapDefaultSkill();

	std::map< int, std::vector<int> >::const_iterator mit = mapDefault.find( iUnitClass );
	if( mit == mapDefault.end() )
		return false;

	const std::vector<int>& vecSkillID = mit->second;

	int iSlot = 0;

	for( size_t i = 0; i < vecSkillID.size(); ++i )
	{
		const int iSkillID = vecSkillID[i];

		KOfflineSkillRow kRow;
		kRow.m_iSkillID	= iSkillID;
		kRow.m_iLevel	= 1;
		kRow.m_iCSPoint	= 0;
		kRow.m_iSlot	= -1;

		// The equippable defaults go on the bar, which is what the client does
		// for a newly learned skill (Handler_EGS_GET_SKILL_ACK puts a level-1
		// active into the first empty slot). A passive has no slot.
		const CX2SkillTree::SkillTemplet* pTemplet = Templet( iSkillID );

		if( NULL != pTemplet && iSlot < SLOT_COUNT_PER_SET )
		{
			switch( pTemplet->m_eType )
			{
			case CX2SkillTree::ST_ACTIVE:
			case CX2SkillTree::ST_SPECIAL_ACTIVE:
			case CX2SkillTree::ST_BUFF:
				kRow.m_iSlot = iSlot++;
				break;

			default:
				break;
			}
		}

		CX2OfflineDB::Instance()->SaveSkill( nUnitUID, kRow );

		// Keep the live copy in step when the seed is for the character that is
		// already loaded - InitSkillTree seeds the current one.
		if( nUnitUID == m_nUnitUID )
		{
			m_mapSkill[ iSkillID ].m_iLevel		= 1;
			m_mapSkill[ iSkillID ].m_iCSPoint	= 0;

			if( kRow.m_iSlot >= 0 )
				m_aiSkillSlot[ kRow.m_iSlot ] = iSkillID;
		}
	}

	CX2OfflineLog::Server( L"SKILL    seeded %u default skill(s) for class %d on unitUID=%I64d",
		(unsigned int)vecSkillID.size(), iUnitClass, (__int64)nUnitUID );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// skill unsealing (phase 21)
//
// A "secret manual" (Camilla's Secret Manual, 270970-270973) unlocks one skill
// on a character whose class has it. The mapping item -> class type -> skill
// list is SkillData.lua's AddSealSkillInfo, which is *client* data and is
// already parsed into CX2SkillTree::m_mapSealSkillItemTemplet - so the offline
// server resolves it through the client's own accessor rather than carrying a
// table of its own. Nothing here is a guess.
//
// The live path is: EGS_USE_ITEM_IN_INVENTORY_ACK, then DBE_UNSEAL_SKILL_ACK
// pushes EGS_UNSEAL_SKILL_NOT { m_iSkillID } (GSUserGameCommon.cpp:6716-6730).
// The ACK alone does not unlock anything; the NOT is what calls
// CX2UserSkillTree::AddSkillUnsealed and opens the "skill unsealed" dialog.

void CX2OfflineSkill::GetUnsealedSkills( OUT std::vector< short >& vecOut ) const
{
	vecOut.clear();

	std::set< int >::const_iterator sit;
	for( sit = m_setUnsealedSkill.begin(); sit != m_setUnsealedSkill.end(); ++sit )
		vecOut.push_back( (short)( *sit ) );
}

bool CX2OfflineSkill::IsSkillUnsealed( int iSkillID ) const
{
	return ( m_setUnsealedSkill.find( iSkillID ) != m_setUnsealedSkill.end() );
}

bool CX2OfflineSkill::UnsealSkill( int iSkillID )
{
	if( iSkillID <= 0 || 0 == m_nUnitUID )
		return false;

	if( true == IsSkillUnsealed( iSkillID ) )
		return false;

	m_setUnsealedSkill.insert( iSkillID );

	CX2OfflineDB::Instance()->SaveUnsealedSkill( m_nUnitUID, iSkillID );

	return true;
}

/*static*/ int CX2OfflineSkill::SkillForUnsealItem( int iItemID )
{
	if( NULL == g_pData || NULL == g_pData->GetSkillTree() )
		return 0;

	if( false == g_pData->GetSkillTree()->IsUnsealSkillItemID( iItemID ) )
		return 0;

	// Resolves against the selected unit's class and returns SI_NONE when this
	// character's tree has none of the item's skills - the same call, and the
	// same answer, the client used a moment ago to decide whether to send.
	const CX2SkillTree::SKILL_ID eSkillID =
		g_pData->GetSkillTree()->GetUnsealSkillItemInfo( iItemID );

	if( CX2SkillTree::SI_NONE == eSkillID )
		return 0;

	return (int)eSkillID;
}

//////////////////////////////////////////////////////////////////////////
// the skill note (phase 22)

/*static*/ char CX2OfflineSkill::ExpandSkillNotePageForLevel( int iLevel )
{
	// KUserSkillTree::GetExpandSkillNotePage, UserSkillTree.cpp:907-940, copied
	// rather than approximated: it switches on level/10 and returns false below
	// 20 and above 69. Transcribed as a table because that is what it is.
	if( iLevel < 0 )
		return 0;

	switch( iLevel / 10 )
	{
	case 2:		return 1;
	case 3:		return 2;
	case 4:		return 3;
	case 5:		return 4;
	case 6:		return 5;		///< added 2011.07.13 by the same file's comment
	}

	return 0;
}

bool CX2OfflineSkill::SetSkillNoteMaxPage( char cMaxPage )
{
	if( 0 == m_nUnitUID )
		return false;

	// UpdateSkillNoteMaxPageNum assigns, but the caller only ever hands it the
	// count for the current level - so a note used twice, or used after the
	// character out-levelled it, must not shrink what it already owns.
	if( cMaxPage <= m_cSkillNoteMaxPage )
		return false;

	m_cSkillNoteMaxPage = cMaxPage;

	CX2OfflineDB::Instance()->SaveSkillNotePage( m_nUnitUID, (int)cMaxPage );

	return true;
}

int CX2OfflineSkill::RegisterSkillNoteMemo( int iPage, int iMemoID )
{
	if( 0 == m_nUnitUID )
		return NetError::ERR_SKILL_NOTE_00;

	// KUserSkillTree::IsHaveSkillNote - no pages means no note at all.
	if( m_cSkillNoteMaxPage <= 0 )
		return NetError::ERR_SKILL_NOTE_04;

	// KUserSkillTree::IsExistSkillNotePage: m_cSkillNoteMaxPageNum > cPageNum,
	// so pages are zero-based and the last valid one is max-1.
	if( iPage < 0 || iPage >= (int)m_cSkillNoteMaxPage )
		return NetError::ERR_SKILL_NOTE_01;

	// KUserSkillTree::IsExistSkillNoteMemoID - the same memo cannot sit on two
	// pages. Checked against the memo *item ID*, which is what the server
	// stores (GSUserGameCommon.cpp:7260).
	std::map< char, int >::const_iterator mit;
	for( mit = m_mapSkillNote.begin(); mit != m_mapSkillNote.end(); ++mit )
	{
		if( mit->second == iMemoID )
			return NetError::ERR_SKILL_NOTE_02;
	}

	m_mapSkillNote[ (char)iPage ] = iMemoID;

	CX2OfflineDB::Instance()->SaveSkillNote( m_nUnitUID, iPage, iMemoID );

	return NetError::NET_OK;
}

#endif SERV_IRUHADEV_OFFLINE
