#include "stdafx.h"
#include "X2OfflineTitle.h"

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-03
// Description: Offline mode - titles and title missions (phase 6). See the
//              header. Reference is KncWX2Server/GameServer/UserTitleManager.cpp
//              throughout; templets come from the client's CX2TitleManager.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineLog.h"
#include "X2OfflineInventory.h"

CX2OfflineTitle* CX2OfflineTitle::ms_pInstance = NULL;

/*static*/ const wchar_t* CX2OfflineTitle::INFINITY_END_DATE = L"2049-12-31 23:59:00";

CX2OfflineTitle::CX2OfflineTitle()
: m_nUnitUID( 0 )
{
}

CX2OfflineTitle::~CX2OfflineTitle()
{
}

CX2OfflineTitle* CX2OfflineTitle::Instance()
{
	if( NULL == ms_pInstance )
		ms_pInstance = new CX2OfflineTitle;

	return ms_pInstance;
}

void CX2OfflineTitle::Release()
{
	if( NULL == ms_pInstance )
		return;

	delete ms_pInstance;
	ms_pInstance = NULL;
}

//////////////////////////////////////////////////////////////////////////

/*static*/ const CX2TitleManager::MissionTemplet* CX2OfflineTitle::Templet( int iMissionID )
{
	if( NULL == g_pData || NULL == g_pData->GetTitleManager() )
		return NULL;

	return g_pData->GetTitleManager()->GetMissionInfo( iMissionID );
}

/*static*/ const CX2TitleManager::SubMissionTemplet* CX2OfflineTitle::SubTemplet( int iSubMissionID )
{
	if( NULL == g_pData || NULL == g_pData->GetTitleManager() )
		return NULL;

	return g_pData->GetTitleManager()->GetSubMissionInfo( iSubMissionID );
}

//////////////////////////////////////////////////////////////////////////

void CX2OfflineTitle::Clear()
{
	m_nUnitUID = 0;
	m_mapMission.clear();
	m_mapTitle.clear();
}

bool CX2OfflineTitle::Load( UidType nUnitUID )
{
	if( 0 == nUnitUID )
		return false;

	if( nUnitUID == m_nUnitUID )
		return true;

	Clear();
	m_nUnitUID = nUnitUID;

	CX2OfflineDB* pDB = CX2OfflineDB::Instance();

	std::vector< KOfflineTitleRow > vecTitle;
	pDB->LoadTitles( nUnitUID, vecTitle );

	for( size_t i = 0; i < vecTitle.size(); ++i )
	{
		KTitleInfo kInfo;
		kInfo.m_iTitleID	= vecTitle[i].m_iTitleID;
		kInfo.m_kCTime		= vecTitle[i].m_tEndDate;
		kInfo.m_bInfinity	= ( 0 == vecTitle[i].m_tEndDate );
		kInfo.m_wstrEndDate	= ( true == kInfo.m_bInfinity )
								? INFINITY_END_DATE
								: CX2OfflineDB::FormatDate( vecTitle[i].m_tEndDate );

		// A permanent title's expiry fields are never read: CX2TitleManager::
		// UpdateTitle sets record.m_bPeriod = !m_bInfinity (X2TitleManager.cpp:1510)
		// and the title screen only draws a remaining-days line when that is
		// true. The date string is still filled in above rather than left empty,
		// so a row read out of the save file reads as a date either way.
		if( true == kInfo.m_bInfinity )
			kInfo.m_kCTime = 0;

		m_mapTitle[ kInfo.m_iTitleID ] = kInfo;
	}

	std::vector< KOfflineMissionRow > vecRow;
	pDB->LoadMissions( nUnitUID, vecRow );

	for( size_t i = 0; i < vecRow.size(); ++i )
	{
		const CX2TitleManager::MissionTemplet* pTemplet = Templet( vecRow[i].m_iMissionID );
		if( NULL == pTemplet )
		{
			CX2OfflineLog::Server(
				L"TITLE    WARNING mission %d is in the save but has no templet in this"
				L" build - left on disk, not loaded", vecRow[i].m_iMissionID );
			continue;
		}

		// Positional, same as a quest instance: entry i describes
		// MissionTemplet::m_vecSubMission[i]. CheckCompleteMission indexes it
		// that way (UserTitleManager.cpp:1331).
		KMissionInstance kInst;
		kInst.m_iID = vecRow[i].m_iMissionID;

		for( size_t s = 0; s < pTemplet->m_vecSubMission.size(); ++s )
		{
			KSubMissionInstance kSub;
			kSub.m_iID			= pTemplet->m_vecSubMission[s];
			kSub.m_sClearData	= 0;
			kSub.m_bIsSuccess	= false;

			for( size_t r = 0; r < vecRow[i].m_vecSub.size(); ++r )
			{
				if( vecRow[i].m_vecSub[r].m_iSubID != kSub.m_iID )
					continue;

				kSub.m_sClearData	= (short)vecRow[i].m_vecSub[r].m_iClearData;
				kSub.m_bIsSuccess	= vecRow[i].m_vecSub[r].m_bSuccess;
				break;
			}

			kInst.m_vecSubMissionInstance.push_back( kSub );
		}

		m_mapMission[ kInst.m_iID ] = kInst;
	}

	CX2OfflineLog::Server( L"TITLE    loaded for unitUID=%I64d: %u mission(s), %u title(s)",
		(__int64)nUnitUID, (unsigned int)m_mapMission.size(),
		(unsigned int)m_mapTitle.size() );

	return true;
}

bool CX2OfflineTitle::Save( int iMissionID ) const
{
	std::map< int, KMissionInstance >::const_iterator mit = m_mapMission.find( iMissionID );
	if( mit == m_mapMission.end() )
		return false;

	KOfflineMissionRow kRow;
	kRow.m_iMissionID = iMissionID;

	for( size_t i = 0; i < mit->second.m_vecSubMissionInstance.size(); ++i )
	{
		KOfflineSubQuestRow kSub;
		kSub.m_iSubID		= mit->second.m_vecSubMissionInstance[i].m_iID;
		kSub.m_iClearData	= (int)mit->second.m_vecSubMissionInstance[i].m_sClearData;
		kSub.m_bSuccess		= mit->second.m_vecSubMissionInstance[i].m_bIsSuccess;

		kRow.m_vecSub.push_back( kSub );
	}

	return CX2OfflineDB::Instance()->SaveMission( m_nUnitUID, kRow );
}

void CX2OfflineTitle::MarkChanged( const KMissionInstance& kInst,
								   OUT std::vector< KMissionInstance >& vecChanged )
{
	for( size_t i = 0; i < vecChanged.size(); ++i )
	{
		if( vecChanged[i].m_iID == kInst.m_iID )
		{
			vecChanged[i] = kInst;
			return;
		}
	}

	vecChanged.push_back( kInst );
}

//////////////////////////////////////////////////////////////////////////
// reads

void CX2OfflineTitle::GetMissionInstances( OUT std::vector< KMissionInstance >& vecOut ) const
{
	vecOut.clear();

	std::map< int, KMissionInstance >::const_iterator mit;
	for( mit = m_mapMission.begin(); mit != m_mapMission.end(); ++mit )
		vecOut.push_back( mit->second );
}

void CX2OfflineTitle::GetTitles( OUT std::vector< KTitleInfo >& vecOut ) const
{
	vecOut.clear();

	std::map< int, KTitleInfo >::const_iterator mit;
	for( mit = m_mapTitle.begin(); mit != m_mapTitle.end(); ++mit )
		vecOut.push_back( mit->second );
}

bool CX2OfflineTitle::HasTitle( int iTitleID ) const
{
	return m_mapTitle.end() != m_mapTitle.find( iTitleID );
}

int CX2OfflineTitle::CheckEquip( int iTitleID, int iEquippedNow ) const
{
	// 0 is "take the title off", and is always allowed.
	if( 0 == iTitleID )
		return NetError::NET_OK;

	if( NULL == g_pData || NULL == g_pData->GetTitleManager() ||
		NULL == g_pData->GetTitleManager()->GetTitleInfo( iTitleID ) )
	{
		return NetError::ERR_TITLE_03;
	}

	if( iEquippedNow == iTitleID )
		return NetError::ERR_TITLE_01;

	if( false == HasTitle( iTitleID ) )
		return NetError::ERR_TITLE_00;

	return NetError::NET_OK;
}

//////////////////////////////////////////////////////////////////////////

/*static*/ bool CX2OfflineTitle::DungeonMatches( const CX2TitleManager::SubMissionTemplet* pSub,
												 int iDungeonID, char cDifficulty )
{
	if( NULL == pSub )
		return false;

	// DI_NONE means "any dungeon" - the mission scripts leave it out for the
	// conditions that are not about a particular place.
	if( CX2Dungeon::DI_NONE == pSub->m_ClearCondition.m_eDungeonID )
		return true;

	if( (int)pSub->m_ClearCondition.m_eDungeonID != iDungeonID )
		return false;

	if( pSub->m_ClearCondition.m_cDifficulty < 0 )
		return true;

	if( true == pSub->m_ClearCondition.m_bUpperDifficulty )
		return ( pSub->m_ClearCondition.m_cDifficulty <= cDifficulty );

	return ( pSub->m_ClearCondition.m_cDifficulty == cDifficulty );
}

bool CX2OfflineTitle::IsSubComplete( const CX2TitleManager::SubMissionTemplet* pSub,
									 const KSubMissionInstance& kInst ) const
{
	if( NULL == pSub )
		return false;

	if( CX2TitleManager::TMCT_ITEM_COLLECTION == pSub->m_eClearType ||
		CX2TitleManager::TMCT_QUEST_ITEM_COLLECTION == pSub->m_eClearType )
	{
		const int iHave = CX2OfflineInventory::Instance()->CountItemByID(
			pSub->m_ClearCondition.m_iCollectionItemID, true );

		return ( iHave >= pSub->m_ClearCondition.m_iCollectionItemNum );
	}

	if( CX2TitleManager::TMCT_COLLECT_TITLE == pSub->m_eClearType )
	{
		// "own this particular title" when one is named, "own this many" when
		// the count is what the script sets.
		if( pSub->m_ClearCondition.m_iTitleID > 0 )
			return HasTitle( pSub->m_ClearCondition.m_iTitleID );

		return ( (int)m_mapTitle.size() >= pSub->m_ClearCondition.m_nCount );
	}

	return kInst.m_bIsSuccess;
}

void CX2OfflineTitle::CheckComplete( int iMissionID, OUT std::vector< KTitleInfo >& vecNewTitle )
{
	std::map< int, KMissionInstance >::iterator mit = m_mapMission.find( iMissionID );
	if( mit == m_mapMission.end() )
		return;

	const CX2TitleManager::MissionTemplet* pTemplet = Templet( iMissionID );
	if( NULL == pTemplet )
		return;

	for( size_t i = 0; i < mit->second.m_vecSubMissionInstance.size(); ++i )
	{
		const CX2TitleManager::SubMissionTemplet* pSub =
			SubTemplet( mit->second.m_vecSubMissionInstance[i].m_iID );

		if( false == IsSubComplete( pSub, mit->second.m_vecSubMissionInstance[i] ) )
			return;
	}

	// The title the mission pays. m_iTitleID and m_iMissionID are the same
	// number in the shipped data - see the header - but the templet's own field
	// is what gets granted, so a script that ever separated them would still
	// work.
	const int iTitleID = ( 0 != pTemplet->m_iTitleID ) ? pTemplet->m_iTitleID : iMissionID;

	m_mapMission.erase( mit );
	CX2OfflineDB::Instance()->DeleteMission( m_nUnitUID, iMissionID );

	if( true == HasTitle( iTitleID ) )
		return;

	// m_sPeriod is a rental in days; 0 is permanent. Nothing offline issues a
	// rented title today, but honouring the field costs one branch and means a
	// script that sets it is not silently upgraded to permanent.
	KOfflineTitleRow kRow;
	kRow.m_iTitleID	= iTitleID;
	kRow.m_tEndDate	= ( pTemplet->m_sPeriod > 0 )
						? ( (__int64)::_time64( NULL ) + (__int64)pTemplet->m_sPeriod * 24 * 60 * 60 )
						: 0;

	CX2OfflineDB::Instance()->SaveTitle( m_nUnitUID, kRow );

	KTitleInfo kInfo;
	kInfo.m_iTitleID	= iTitleID;
	kInfo.m_bInfinity	= ( 0 == kRow.m_tEndDate );
	kInfo.m_kCTime		= kRow.m_tEndDate;
	kInfo.m_wstrEndDate	= ( true == kInfo.m_bInfinity )
							? INFINITY_END_DATE
							: CX2OfflineDB::FormatDate( kRow.m_tEndDate );

	m_mapTitle[ iTitleID ] = kInfo;
	vecNewTitle.push_back( kInfo );

	CX2OfflineLog::Server( L"TITLE    mission %d '%s' complete -> title %d awarded%s",
		iMissionID, pTemplet->m_wstrMissionName.c_str(), iTitleID,
		kInfo.m_bInfinity ? L"" : L" (rented)" );
}

//////////////////////////////////////////////////////////////////////////

void CX2OfflineTitle::CheckNewMission( const KOfflineUnitRow& kRow,
									   OUT std::vector< KMissionInstance >& vecNew )
{
	if( NULL == g_pData || NULL == g_pData->GetTitleManager() )
		return;

	CX2TitleManager* pTM = g_pData->GetTitleManager();

	CX2Unit::UNIT_TYPE eType = CX2Unit::UT_NONE;

	if( NULL != g_pData->GetUnitManager() )
	{
		const CX2Unit::UnitTemplet* pUnitTemplet =
			g_pData->GetUnitManager()->GetUnitTemplet( (CX2Unit::UNIT_CLASS)kRow.m_iUnitClass );

		if( NULL != pUnitTemplet )
			eType = pUnitTemplet->m_UnitType;
	}

	const std::map< int, CX2TitleManager::MissionTemplet >& mapAll = pTM->GetMapTitleMission();

	std::map< int, CX2TitleManager::MissionTemplet >::const_iterator mit;
	for( mit = mapAll.begin(); mit != mapAll.end(); ++mit )
	{
		const CX2TitleManager::MissionTemplet& kTemplet = mit->second;

		const int iTitleID = ( 0 != kTemplet.m_iTitleID )
								? kTemplet.m_iTitleID : kTemplet.m_iMissionID;

		// Already earned, or already in progress.
		if( true == HasTitle( iTitleID ) )
			continue;

		if( m_mapMission.end() != m_mapMission.find( kTemplet.m_iMissionID ) )
			continue;

		// A secret title is not offered - it is meant to appear only once it is
		// earned. The server reads the flag off the mission templet; the
		// client's copy of that flag lives on the TitleInfo the mission pays,
		// which is the same bit of script data reached from the other end.
		const CX2TitleManager::TitleInfo* pTitleInfo = pTM->GetTitleInfo( iTitleID );
		if( NULL == pTitleInfo )
			continue;

		if( true == pTitleInfo->m_bIsSecretTitle )
			continue;

		if( kTemplet.m_kCondition.m_iLevel > kRow.m_iLevel )
			continue;

		if( CX2Unit::UT_NONE != kTemplet.m_kCondition.m_eUnitType &&
			kTemplet.m_kCondition.m_eUnitType != eType )
		{
			continue;
		}

		if( CX2Unit::UC_NONE != kTemplet.m_kCondition.m_eUnitClass &&
			(int)kTemplet.m_kCondition.m_eUnitClass != kRow.m_iUnitClass )
		{
			continue;
		}

		// The prerequisite is a mission that must already have paid its title.
		bool bBlocked = false;
		for( size_t i = 0; i < kTemplet.m_kCondition.m_vecClearMissionID.size(); ++i )
		{
			if( false == HasTitle( kTemplet.m_kCondition.m_vecClearMissionID[i] ) )
			{
				bBlocked = true;
				break;
			}
		}

		if( true == bBlocked )
			continue;

		KMissionInstance kInst;
		kInst.m_iID = kTemplet.m_iMissionID;

		for( size_t i = 0; i < kTemplet.m_vecSubMission.size(); ++i )
		{
			if( NULL == SubTemplet( kTemplet.m_vecSubMission[i] ) )
			{
				// SERV_SUB_TITLE_MISSION_BUG_FIX: a mission with a missing
				// sub-mission templet is dropped whole rather than started with
				// a short list, which would make it complete on the spot.
				kInst.m_vecSubMissionInstance.clear();
				break;
			}

			KSubMissionInstance kSub;
			kSub.m_iID			= kTemplet.m_vecSubMission[i];
			kSub.m_sClearData	= 0;
			kSub.m_bIsSuccess	= false;

			kInst.m_vecSubMissionInstance.push_back( kSub );
		}

		if( true == kInst.m_vecSubMissionInstance.empty() )
			continue;

		m_mapMission[ kInst.m_iID ] = kInst;
		Save( kInst.m_iID );

		vecNew.push_back( kInst );
	}

	if( false == vecNew.empty() )
	{
		CX2OfflineLog::Server( L"TITLE    %u new mission(s) opened for unitUID=%I64d at level %d",
			(unsigned int)vecNew.size(), (__int64)kRow.m_nUnitUID, kRow.m_iLevel );
	}
}

//////////////////////////////////////////////////////////////////////////
// events

void CX2OfflineTitle::OnTalkNPC( int iNPCID, const KOfflineUnitRow& /*kRow*/,
								 OUT std::vector< KMissionInstance >& vecChanged,
								 OUT std::vector< KTitleInfo >& vecNewTitle )
{
	std::vector< int > vecComplete;

	std::map< int, KMissionInstance >::iterator mit;
	for( mit = m_mapMission.begin(); mit != m_mapMission.end(); ++mit )
	{
		const CX2TitleManager::MissionTemplet* pTemplet = Templet( mit->first );
		if( NULL == pTemplet )
			continue;

		KMissionInstance& kInst = mit->second;
		bool bMoved = false;

		for( size_t i = 0; i < kInst.m_vecSubMissionInstance.size(); ++i )
		{
			const CX2TitleManager::SubMissionTemplet* pSub =
				SubTemplet( kInst.m_vecSubMissionInstance[i].m_iID );

			if( NULL == pSub )
				continue;

			if( CX2TitleManager::TMCT_NPC_TALK != pSub->m_eClearType )
				continue;

			if( true == kInst.m_vecSubMissionInstance[i].m_bIsSuccess )
				continue;

			if( (int)pSub->m_ClearCondition.m_eTalkNPCID != iNPCID )
				continue;

			kInst.m_vecSubMissionInstance[i].m_sClearData	= 1;
			kInst.m_vecSubMissionInstance[i].m_bIsSuccess	= true;
			bMoved = true;

			CX2OfflineLog::Server( L"TITLE    mission %d sub %d NPC_TALK done (NPC %d)",
				mit->first, kInst.m_vecSubMissionInstance[i].m_iID, iNPCID );
		}

		if( false == bMoved )
			continue;

		Save( mit->first );
		MarkChanged( kInst, vecChanged );
		vecComplete.push_back( mit->first );
	}

	// CheckComplete erases from m_mapMission, so it cannot run inside the loop
	// above without invalidating the iterator.
	for( size_t i = 0; i < vecComplete.size(); ++i )
		CheckComplete( vecComplete[i], vecNewTitle );
}

void CX2OfflineTitle::OnNpcDie( int iDungeonID, char cDifficulty, int iNpcID,
								const KOfflineUnitRow& /*kRow*/,
								OUT std::vector< KMissionInstance >& vecChanged,
								OUT std::vector< KTitleInfo >& vecNewTitle )
{
	std::vector< int > vecComplete;

	std::map< int, KMissionInstance >::iterator mit;
	for( mit = m_mapMission.begin(); mit != m_mapMission.end(); ++mit )
	{
		KMissionInstance& kInst = mit->second;
		bool bMoved = false;

		for( size_t i = 0; i < kInst.m_vecSubMissionInstance.size(); ++i )
		{
			const CX2TitleManager::SubMissionTemplet* pSub =
				SubTemplet( kInst.m_vecSubMissionInstance[i].m_iID );

			if( NULL == pSub )
				continue;

			if( CX2TitleManager::TMCT_NPC_HUNT != pSub->m_eClearType )
				continue;

			if( true == kInst.m_vecSubMissionInstance[i].m_bIsSuccess )
				continue;

			if( false == pSub->m_ClearCondition.m_setKillNPCID.empty() &&
				pSub->m_ClearCondition.m_setKillNPCID.end() ==
					pSub->m_ClearCondition.m_setKillNPCID.find(
						(CX2UnitManager::NPC_UNIT_ID)iNpcID ) )
			{
				continue;
			}

			if( false == DungeonMatches( pSub, iDungeonID, cDifficulty ) )
				continue;

			if( pSub->m_ClearCondition.m_iKillNum <=
				(int)kInst.m_vecSubMissionInstance[i].m_sClearData )
			{
				continue;
			}

			++kInst.m_vecSubMissionInstance[i].m_sClearData;

			if( pSub->m_ClearCondition.m_iKillNum <=
				(int)kInst.m_vecSubMissionInstance[i].m_sClearData )
			{
				kInst.m_vecSubMissionInstance[i].m_bIsSuccess = true;
			}

			bMoved = true;
		}

		if( false == bMoved )
			continue;

		Save( mit->first );
		MarkChanged( kInst, vecChanged );
		vecComplete.push_back( mit->first );
	}

	for( size_t i = 0; i < vecComplete.size(); ++i )
		CheckComplete( vecComplete[i], vecNewTitle );
}

void CX2OfflineTitle::OnDungeonClear( int iDungeonID, char cDifficulty, char cRank,
									  int iPlayTime, int iDamage, const KOfflineUnitRow& /*kRow*/,
									  OUT std::vector< KMissionInstance >& vecChanged,
									  OUT std::vector< KTitleInfo >& vecNewTitle )
{
	std::vector< int > vecComplete;

	std::map< int, KMissionInstance >::iterator mit;
	for( mit = m_mapMission.begin(); mit != m_mapMission.end(); ++mit )
	{
		KMissionInstance& kInst = mit->second;
		bool bMoved = false;

		for( size_t i = 0; i < kInst.m_vecSubMissionInstance.size(); ++i )
		{
			const CX2TitleManager::SubMissionTemplet* pSub =
				SubTemplet( kInst.m_vecSubMissionInstance[i].m_iID );

			if( NULL == pSub )
				continue;

			if( true == kInst.m_vecSubMissionInstance[i].m_bIsSuccess )
				continue;

			bool bHit = false;

			switch( pSub->m_eClearType )
			{
			case CX2TitleManager::TMCT_DUNGEON_TIME:
				bHit = ( pSub->m_ClearCondition.m_iDungeonClearTime >= iPlayTime );
				break;

			case CX2TitleManager::TMCT_DUNGEON_RANK:
				bHit = ( (char)pSub->m_ClearCondition.m_eDungeonRank <= cRank );
				break;

			case CX2TitleManager::TMCT_DUNGEON_DAMAGE:
				bHit = ( pSub->m_ClearCondition.m_iDungeonDamage >= iDamage );
				break;

			case CX2TitleManager::TMCT_DUNGEON_CLEAR_COUNT:
				bHit = true;
				break;

			default:
				break;
			}

			if( false == bHit )
				continue;

			if( false == DungeonMatches( pSub, iDungeonID, cDifficulty ) )
				continue;

			// Every one of these four is counted, because the mission scripts
			// give all four an m_iDungeonClearCount - "clear this dungeon at
			// rank A three times" is one condition, not two.
			const int iNeed = ( pSub->m_ClearCondition.m_iDungeonClearCount > 0 )
								? pSub->m_ClearCondition.m_iDungeonClearCount : 1;

			if( iNeed > (int)kInst.m_vecSubMissionInstance[i].m_sClearData )
				++kInst.m_vecSubMissionInstance[i].m_sClearData;

			if( iNeed <= (int)kInst.m_vecSubMissionInstance[i].m_sClearData )
				kInst.m_vecSubMissionInstance[i].m_bIsSuccess = true;

			bMoved = true;

			CX2OfflineLog::Server( L"TITLE    mission %d sub %d dungeon type %d -> %d/%d",
				mit->first, kInst.m_vecSubMissionInstance[i].m_iID,
				(int)pSub->m_eClearType,
				(int)kInst.m_vecSubMissionInstance[i].m_sClearData, iNeed );
		}

		if( false == bMoved )
			continue;

		Save( mit->first );
		MarkChanged( kInst, vecChanged );
		vecComplete.push_back( mit->first );
	}

	for( size_t i = 0; i < vecComplete.size(); ++i )
		CheckComplete( vecComplete[i], vecNewTitle );
}

void CX2OfflineTitle::OnQuestComplete( int iQuestID, const KOfflineUnitRow& /*kRow*/,
									   OUT std::vector< KMissionInstance >& vecChanged,
									   OUT std::vector< KTitleInfo >& vecNewTitle )
{
	std::vector< int > vecComplete;

	std::map< int, KMissionInstance >::iterator mit;
	for( mit = m_mapMission.begin(); mit != m_mapMission.end(); ++mit )
	{
		KMissionInstance& kInst = mit->second;
		bool bMoved = false;

		for( size_t i = 0; i < kInst.m_vecSubMissionInstance.size(); ++i )
		{
			const CX2TitleManager::SubMissionTemplet* pSub =
				SubTemplet( kInst.m_vecSubMissionInstance[i].m_iID );

			if( NULL == pSub )
				continue;

			if( CX2TitleManager::TMCT_QUEST != pSub->m_eClearType )
				continue;

			if( true == kInst.m_vecSubMissionInstance[i].m_bIsSuccess )
				continue;

			if( pSub->m_ClearCondition.m_iQuestID != iQuestID )
				continue;

			kInst.m_vecSubMissionInstance[i].m_sClearData	= 1;
			kInst.m_vecSubMissionInstance[i].m_bIsSuccess	= true;
			bMoved = true;

			CX2OfflineLog::Server( L"TITLE    mission %d sub %d QUEST done (quest %d)",
				mit->first, kInst.m_vecSubMissionInstance[i].m_iID, iQuestID );
		}

		if( false == bMoved )
			continue;

		Save( mit->first );
		MarkChanged( kInst, vecChanged );
		vecComplete.push_back( mit->first );
	}

	for( size_t i = 0; i < vecComplete.size(); ++i )
		CheckComplete( vecComplete[i], vecNewTitle );
}

void CX2OfflineTitle::OnUseItem( int iItemID, const KOfflineUnitRow& /*kRow*/,
								 OUT std::vector< KMissionInstance >& vecChanged,
								 OUT std::vector< KTitleInfo >& vecNewTitle )
{
	std::vector< int > vecComplete;

	std::map< int, KMissionInstance >::iterator mit;
	for( mit = m_mapMission.begin(); mit != m_mapMission.end(); ++mit )
	{
		KMissionInstance& kInst = mit->second;
		bool bMoved = false;

		for( size_t i = 0; i < kInst.m_vecSubMissionInstance.size(); ++i )
		{
			const CX2TitleManager::SubMissionTemplet* pSub =
				SubTemplet( kInst.m_vecSubMissionInstance[i].m_iID );

			if( NULL == pSub )
				continue;

			if( CX2TitleManager::TMCT_USE_ITEM != pSub->m_eClearType )
				continue;

			if( true == kInst.m_vecSubMissionInstance[i].m_bIsSuccess )
				continue;

			if( pSub->m_ClearCondition.m_iItemID != iItemID )
				continue;

			kInst.m_vecSubMissionInstance[i].m_sClearData	= 1;
			kInst.m_vecSubMissionInstance[i].m_bIsSuccess	= true;
			bMoved = true;
		}

		if( false == bMoved )
			continue;

		Save( mit->first );
		MarkChanged( kInst, vecChanged );
		vecComplete.push_back( mit->first );
	}

	for( size_t i = 0; i < vecComplete.size(); ++i )
		CheckComplete( vecComplete[i], vecNewTitle );
}

void CX2OfflineTitle::ReCheckPassive( const KOfflineUnitRow& /*kRow*/,
									  OUT std::vector< KMissionInstance >& vecChanged,
									  OUT std::vector< KTitleInfo >& vecNewTitle )
{
	// The collection and title-count conditions have no counter of their own -
	// IsSubComplete answers them from the bag and the title list every time it
	// is asked - so there is nothing to write here. What this does is notice
	// that such a mission has *become* complete and pay it out.
	//
	// It is deliberately a separate call rather than something the pickup path
	// does inline: a title granted here can open a further mission whose
	// condition is "own that title", so the loop repeats until nothing new
	// appears. Bounded by the number of missions, which is finite and small.
	for( int iPass = 0; iPass < 8; ++iPass )
	{
		std::vector< int > vecComplete;

		std::map< int, KMissionInstance >::const_iterator mit;
		for( mit = m_mapMission.begin(); mit != m_mapMission.end(); ++mit )
		{
			bool bAllDone = true;

			for( size_t i = 0; i < mit->second.m_vecSubMissionInstance.size(); ++i )
			{
				const CX2TitleManager::SubMissionTemplet* pSub =
					SubTemplet( mit->second.m_vecSubMissionInstance[i].m_iID );

				if( false == IsSubComplete( pSub, mit->second.m_vecSubMissionInstance[i] ) )
				{
					bAllDone = false;
					break;
				}
			}

			if( true == bAllDone )
				vecComplete.push_back( mit->first );
		}

		if( true == vecComplete.empty() )
			break;

		for( size_t i = 0; i < vecComplete.size(); ++i )
		{
			std::map< int, KMissionInstance >::const_iterator mitDone =
				m_mapMission.find( vecComplete[i] );

			if( mitDone != m_mapMission.end() )
				MarkChanged( mitDone->second, vecChanged );

			CheckComplete( vecComplete[i], vecNewTitle );
		}
	}
}

#endif SERV_IRUHADEV_OFFLINE
