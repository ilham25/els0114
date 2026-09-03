#include "stdafx.h"
#include "X2OfflineInventory.h"

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-02
// Description: Offline mode - the inventory (phase 5). See the header.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineLog.h"

CX2OfflineInventory* CX2OfflineInventory::ms_pInstance = NULL;

CX2OfflineInventory::CX2OfflineInventory()
: m_nUnitUID( 0 )
{
}

CX2OfflineInventory::~CX2OfflineInventory()
{
}

CX2OfflineInventory* CX2OfflineInventory::Instance()
{
	if( NULL == ms_pInstance )
		ms_pInstance = new CX2OfflineInventory;

	return ms_pInstance;
}

void CX2OfflineInventory::Release()
{
	if( NULL == ms_pInstance )
		return;

	delete ms_pInstance;
	ms_pInstance = NULL;
}

//////////////////////////////////////////////////////////////////////////

/*static*/ const CX2Item::ItemTemplet* CX2OfflineInventory::Templet( int iItemID )
{
	if( NULL == g_pData || NULL == g_pData->GetItemManager() )
		return NULL;

	const CX2Item::ItemTemplet* pTemplet = g_pData->GetItemManager()->GetItemTemplet( iItemID );

	if( NULL == pTemplet )
	{
		// Worth a line every time. An item ID the client cannot resolve means
		// the save file and the .kom archives disagree - either a drop table
		// naming an item this build does not ship, or a save written by a
		// different region's client.
		CX2OfflineLog::Server( L"ITEM     WARNING no templet for item %d - the save file and the"
			L" client's item tables disagree", iItemID );
	}

	return pTemplet;
}

/*static*/ int CX2OfflineInventory::MaxStack( int iItemID )
{
	const CX2Item::ItemTemplet* pTemplet = Templet( iItemID );
	if( NULL == pTemplet )
		return 1;

	if( CX2Item::PT_QUANTITY != pTemplet->GetPeriodType() )
		return 1;

	const int iStack = pTemplet->GetQuantity();

	return ( iStack > 0 ) ? iStack : 1;
}

/*static*/ int CX2OfflineInventory::SuitableCategory( int iItemID )
{
	// KInventory::GetSuitableCategory (Inventory.cpp:11497). The
	// SERV_AUTOMATICALLY_REGISTER_FOR_CONSUMABLE_ITEM variant that returns a
	// list is not this build's - that flag is undefined here, checked rather
	// than assumed - so a quick-slot item goes to the consumable bag and the
	// player drags it onto the bar.
	const CX2Item::ItemTemplet* pTemplet = Templet( iItemID );
	if( NULL == pTemplet )
		return CX2Inventory::ST_NONE;

	if( true == pTemplet->GetIsPcBang() )
		return CX2Inventory::ST_PC_BANG;

	if( true == pTemplet->GetFashion() )
		return CX2Inventory::ST_AVARTA;

	switch( pTemplet->GetItemType() )
	{
	case CX2Item::IT_NONE:			return CX2Inventory::ST_NONE;
	case CX2Item::IT_WEAPON:
	case CX2Item::IT_DEFENCE:		return CX2Inventory::ST_EQUIP;
	case CX2Item::IT_ACCESSORY:		return CX2Inventory::ST_ACCESSORY;
	case CX2Item::IT_MATERIAL:		return CX2Inventory::ST_MATERIAL;
	case CX2Item::IT_SPECIAL:		return CX2Inventory::ST_SPECIAL;
	case CX2Item::IT_QUEST:			return CX2Inventory::ST_QUEST;
	case CX2Item::IT_QICK_SLOT:		return CX2Inventory::ST_QUICK_SLOT;
	case CX2Item::IT_SKILL_MEMO:	return CX2Inventory::ST_SPECIAL;
	default:						return CX2Inventory::ST_NONE;
	}
}

/*static*/ bool CX2OfflineInventory::IsSuitableSlot( int iItemID, int iCategory, int iSlotID )
{
	// KInventory::IsSuitableSlot (Inventory.cpp:946), field for field.
	const CX2Item::ItemTemplet* pTemplet = Templet( iItemID );
	if( NULL == pTemplet )
		return false;

	// PC-room items outrank every other rule, as they do on the server.
	if( true == pTemplet->GetIsPcBang() )
	{
		if( CX2Inventory::ST_E_EQUIP != iCategory &&
			CX2Inventory::ST_PC_BANG != iCategory )
		{
			return false;
		}
	}

	switch( iCategory )
	{
	case CX2Inventory::ST_EQUIP:
		return ( ( CX2Item::IT_WEAPON  == pTemplet->GetItemType() ||
				   CX2Item::IT_DEFENCE == pTemplet->GetItemType() ) &&
				 false == pTemplet->GetFashion() );

	case CX2Inventory::ST_ACCESSORY:
		return ( CX2Item::IT_ACCESSORY == pTemplet->GetItemType() &&
				 false == pTemplet->GetFashion() );

	case CX2Inventory::ST_MATERIAL:
		return ( CX2Item::IT_MATERIAL == pTemplet->GetItemType() );

	case CX2Inventory::ST_SPECIAL:
		return ( CX2Item::IT_SPECIAL    == pTemplet->GetItemType() ||
				 CX2Item::IT_SKILL_MEMO == pTemplet->GetItemType() );

	case CX2Inventory::ST_QUEST:
		return ( CX2Item::IT_QUEST == pTemplet->GetItemType() );

	case CX2Inventory::ST_QUICK_SLOT:
		return ( CX2Item::IT_QICK_SLOT == pTemplet->GetItemType() );

	case CX2Inventory::ST_AVARTA:
		return pTemplet->GetFashion();

	case CX2Inventory::ST_BANK:
	case CX2Inventory::ST_SHARE_BANK:
		return true;			///< a bank takes anything

	case CX2Inventory::ST_PC_BANG:
		return pTemplet->GetIsPcBang();

	case CX2Inventory::ST_E_SKILL:
		return false;			///< skills have their own slots, not inventory ones

	case CX2Inventory::ST_E_QUICK_SLOT:
		return ( CX2Unit::EP_QUICK_SLOT == pTemplet->GetEqipPosition() );

	case CX2Inventory::ST_E_EQUIP:
		{
			// One equip slot per (position, fashion) pair. The accessory slots
			// deliberately do not test fashion - the server's own switch has
			// that half commented out and both a plain and a fashion accessory
			// go in the same slot.
			const int iPos			= (int)pTemplet->GetEqipPosition();
			const bool bFashion		= pTemplet->GetFashion();

			switch( iSlotID )
			{
			case CX2Unit::NESI_DEFENCE_HAIR_FASHION:
				return ( CX2Unit::EP_DEFENCE_HAIR == iPos && true == bFashion );
			case CX2Unit::NESI_DEFENCE_BODY:
				return ( CX2Unit::EP_DEFENCE_BODY == iPos && false == bFashion );
			case CX2Unit::NESI_DEFENCE_BODY_FASHION:
				return ( CX2Unit::EP_DEFENCE_BODY == iPos && true == bFashion );
			case CX2Unit::NESI_DEFENCE_LEG:
				return ( CX2Unit::EP_DEFENCE_LEG == iPos && false == bFashion );
			case CX2Unit::NESI_DEFENCE_LEG_FASHION:
				return ( CX2Unit::EP_DEFENCE_LEG == iPos && true == bFashion );
			case CX2Unit::NESI_DEFENCE_HAND:
				return ( CX2Unit::EP_DEFENCE_HAND == iPos && false == bFashion );
			case CX2Unit::NESI_DEFENCE_HAND_FASHION:
				return ( CX2Unit::EP_DEFENCE_HAND == iPos && true == bFashion );
			case CX2Unit::NESI_DEFENCE_FOOT:
				return ( CX2Unit::EP_DEFENCE_FOOT == iPos && false == bFashion );
			case CX2Unit::NESI_DEFENCE_FOOT_FASHION:
				return ( CX2Unit::EP_DEFENCE_FOOT == iPos && true == bFashion );
			case CX2Unit::NESI_WEAPON_HAND:
				return ( CX2Unit::EP_WEAPON_HAND == iPos && false == bFashion );
			case CX2Unit::NESI_WEAPON_HAND_FASHION:
				return ( CX2Unit::EP_WEAPON_HAND == iPos && true == bFashion );

			case CX2Unit::NESI_AC_FACE1:	return ( CX2Unit::EP_AC_FACE1 == iPos );
			case CX2Unit::NESI_AC_FACE2:	return ( CX2Unit::EP_AC_FACE2 == iPos );
			case CX2Unit::NESI_AC_FACE3:	return ( CX2Unit::EP_AC_FACE3 == iPos );
			case CX2Unit::NESI_AC_BODY:		return ( CX2Unit::EP_AC_BODY == iPos );
			case CX2Unit::NESI_AC_LEG:		return ( CX2Unit::EP_AC_LEG == iPos );
			case CX2Unit::NESI_AC_ARM:		return ( CX2Unit::EP_AC_ARM == iPos );
			case CX2Unit::NESI_AC_RING:		return ( CX2Unit::EP_AC_RING == iPos );
			case CX2Unit::NESI_AC_NECKLESS:	return ( CX2Unit::EP_AC_NECKLESS == iPos );
			case CX2Unit::NESI_AC_WEAPON:	return ( CX2Unit::EP_AC_WEAPON == iPos );

#ifdef SERV_NEW_ONE_PIECE_AVATAR_SLOT
			case CX2Unit::NESI_ONEPIECE_FASHION:
				return ( CX2Unit::EP_ONEPIECE_FASHION == iPos && true == bFashion );
#endif SERV_NEW_ONE_PIECE_AVATAR_SLOT

			default:
				return false;
			}
		}

	default:
		return false;
	}
}

/*static*/ bool CX2OfflineInventory::IsAbleToEquip( int iItemID )
{
	const CX2Item::ItemTemplet* pTemplet = Templet( iItemID );
	if( NULL == pTemplet )
		return false;

	// KInventory::IsAbleToEquip's own last test, and the only one of the three
	// that has no client-side equivalent.
	if( true == pTemplet->GetNoEquip() )
		return false;

	if( NULL == g_pData || NULL == g_pData->GetMyUser() )
		return false;

	CX2Unit* pUnit = g_pData->GetMyUser()->GetSelectUnit();
	if( NULL == pUnit )
		return false;

	// The level and class halves, through the client's own check. Passing a
	// level other than -1 is what switches its level test on; the value itself
	// is ignored, the unit's own level is what gets compared.
	return CX2Unit::CanEquipAsParts( iItemID, pUnit, 1 );
}

void CX2OfflineInventory::AddEquippedStat( IN OUT KStat& kInOut ) const
{
	if( NULL == g_pData )
		return;

	float fBaseHP		= 0.0f;
	float fAtkPhysic	= 0.0f;
	float fAtkMagic		= 0.0f;
	float fDefPhysic	= 0.0f;
	float fDefMagic		= 0.0f;

	for( int iSlot = 0; iSlot < GetSlotSize( CX2Inventory::ST_E_EQUIP ); ++iSlot )
	{
		const UidType nItemUID = GetItemUID( CX2Inventory::ST_E_EQUIP, iSlot );
		if( 0 == nItemUID )
			continue;

		const KOfflineItemRow* pRow = FindRow( nItemUID );
		if( NULL == pRow )
			continue;

		// A worn-out endurance item contributes nothing, which is
		// CX2Unit::GetEqipStat's own REAL_TIME_ELSWORD test (X2Unit.cpp:1196).
		if( CX2Item::PT_ENDURANCE == pRow->m_iUsageType && pRow->m_iEndurance <= 0 )
			continue;

		const CX2Item::ItemTemplet* pTemplet = Templet( pRow->m_iItemID );
		if( NULL == pTemplet )
			continue;

		CX2Item::KItemFormatStatData kStatData;
		g_pData->GetItemStatCalculator().CalculateItemStat( kStatData, pTemplet );

		fBaseHP		+= kStatData.m_fBaseHP;
		fAtkPhysic	+= kStatData.m_fAtkPhysic;
		fAtkMagic	+= kStatData.m_fAtkMagic;
		fDefPhysic	+= kStatData.m_fDefPhysic;
		fDefMagic	+= kStatData.m_fDefMagic;
	}

	kInOut.m_iBaseHP	+= (int)fBaseHP;
	kInOut.m_iAtkPhysic	+= (int)fAtkPhysic;
	kInOut.m_iAtkMagic	+= (int)fAtkMagic;
	kInOut.m_iDefPhysic	+= (int)fDefPhysic;
	kInOut.m_iDefMagic	+= (int)fDefMagic;
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineInventory::Load( UidType nUnitUID )
{
	if( 0 != nUnitUID && nUnitUID == m_nUnitUID )
		return true;						///< already this character's

	Clear();

	if( 0 == nUnitUID )
		return false;

	CX2OfflineDB* pDB = CX2OfflineDB::Instance();

	pDB->LoadInventorySizes( nUnitUID, m_mapSlotSize );

	// A save written before this phase has its sizes already (phase 2 seeded
	// them at creation), but a size row that is missing entirely would collapse
	// that category to zero slots and quietly hide whatever is in it - so fill
	// any gap from the same table SeedInventorySizes uses.
	for( int iCategory = CX2Inventory::ST_NONE + 1; iCategory < CX2Inventory::ST_END; ++iCategory )
	{
		const int iBase = CX2OfflineDB::BaseSlotSize( iCategory );
		if( iBase <= 0 )
			continue;

		std::map< int, int >::const_iterator mit = m_mapSlotSize.find( iCategory );
		if( mit == m_mapSlotSize.end() )
		{
			m_mapSlotSize[ iCategory ] = iBase;

			CX2OfflineLog::Server( L"ITEM     inventory_size had no row for category %d - using the"
				L" base size %d", iCategory, iBase );
		}
	}

	for( int iCategory = 0; iCategory < CX2Inventory::ST_END; ++iCategory )
	{
		m_vecSlot[ iCategory ].clear();
		m_vecSlot[ iCategory ].resize( (size_t)GetSlotSize( iCategory ), (UidType)0 );
	}

	std::vector< KOfflineItemRow > vecRow;
	pDB->LoadItems( nUnitUID, vecRow );

	int iPlaced	= 0;
	int iLost	= 0;

	for( size_t i = 0; i < vecRow.size(); ++i )
	{
		KOfflineItemRow& kRow = vecRow[i];

		// An item outside its category's current bounds is what stock
		// KInventory::Init calls a wrong-position item: it does not delete it,
		// it relocates it. Same here, and it says so - the one case that can
		// produce it is a slot-count change between builds
		// (SERV_IRUHADEV_QUICK_SLOT_FULL_FREE being reverted, say).
		if( false == VerifyCategorySlotID( kRow.m_iCategory, kRow.m_iSlotID ) ||
			(UidType)0 != m_vecSlot[ kRow.m_iCategory ][ kRow.m_iSlotID ] )
		{
			int iNewCategory = 0;
			int iNewSlot	 = 0;

			m_mapItem[ kRow.m_nItemUID ] = kRow;

			if( true == GetEmptyCategorySlotForItem( kRow.m_iItemID, iNewCategory, iNewSlot ) )
			{
				PlaceItem( m_mapItem[ kRow.m_nItemUID ], iNewCategory, iNewSlot );

				CX2OfflineLog::Server( L"ITEM     item %I64d (id=%d) was at %d/%d which is out of"
					L" bounds or taken - moved to %d/%d",
					(__int64)kRow.m_nItemUID, kRow.m_iItemID,
					kRow.m_iCategory, kRow.m_iSlotID, iNewCategory, iNewSlot );

				++iPlaced;
			}
			else
			{
				// Left in the map but in no slot, so it is neither lost from the
				// save file nor visible in a slot that does not exist.
				CX2OfflineLog::Server( L"ITEM     item %I64d (id=%d) has nowhere to go - it stays in"
					L" the save file but is not in the inventory this session",
					(__int64)kRow.m_nItemUID, kRow.m_iItemID );

				++iLost;
			}

			continue;
		}

		m_mapItem[ kRow.m_nItemUID ] = kRow;
		m_vecSlot[ kRow.m_iCategory ][ kRow.m_iSlotID ] = kRow.m_nItemUID;

		++iPlaced;
	}

	m_nUnitUID = nUnitUID;

	CX2OfflineLog::Server( L"ITEM     inventory loaded for unitUID=%I64d: %d item(s)%s",
		(__int64)nUnitUID, iPlaced,
		( iLost > 0 ) ? L" (some could not be placed - see above)" : L"" );

	return true;
}

void CX2OfflineInventory::Clear()
{
	m_nUnitUID = 0;
	m_mapItem.clear();
	m_mapSlotSize.clear();

	for( int i = 0; i < CX2Inventory::ST_END; ++i )
		m_vecSlot[i].clear();
}

int CX2OfflineInventory::GetSlotSize( int iCategory ) const
{
	std::map< int, int >::const_iterator mit = m_mapSlotSize.find( iCategory );
	if( mit == m_mapSlotSize.end() )
		return 0;

	return mit->second;
}

bool CX2OfflineInventory::VerifyCategorySlotID( int iCategory, int iSlotID ) const
{
	if( iCategory <= CX2Inventory::ST_NONE || iCategory >= CX2Inventory::ST_END )
		return false;

	if( iSlotID < 0 || iSlotID >= (int)m_vecSlot[ iCategory ].size() )
		return false;

	return true;
}

bool CX2OfflineInventory::IsEmptySlot( int iCategory, int iSlotID ) const
{
	if( false == VerifyCategorySlotID( iCategory, iSlotID ) )
		return false;

	return ( (UidType)0 == m_vecSlot[ iCategory ][ iSlotID ] );
}

UidType CX2OfflineInventory::GetItemUID( int iCategory, int iSlotID ) const
{
	if( false == VerifyCategorySlotID( iCategory, iSlotID ) )
		return 0;

	return m_vecSlot[ iCategory ][ iSlotID ];
}

KOfflineItemRow* CX2OfflineInventory::FindRow( UidType nItemUID )
{
	std::map< UidType, KOfflineItemRow >::iterator mit = m_mapItem.find( nItemUID );
	if( mit == m_mapItem.end() )
		return NULL;

	return &( mit->second );
}

const KOfflineItemRow* CX2OfflineInventory::FindRow( UidType nItemUID ) const
{
	std::map< UidType, KOfflineItemRow >::const_iterator mit = m_mapItem.find( nItemUID );
	if( mit == m_mapItem.end() )
		return NULL;

	return &( mit->second );
}

bool CX2OfflineInventory::GetItemRow( UidType nItemUID, OUT KOfflineItemRow& kOut ) const
{
	const KOfflineItemRow* pRow = FindRow( nItemUID );
	if( NULL == pRow )
		return false;

	kOut = *pRow;
	return true;
}

int CX2OfflineInventory::GetItemID( UidType nItemUID ) const
{
	const KOfflineItemRow* pRow = FindRow( nItemUID );
	if( NULL == pRow )
		return 0;

	return pRow->m_iItemID;
}

int CX2OfflineInventory::CountItemByID( int iItemID, bool bExcludeEquipped ) const
{
	int iCount = 0;

	std::map< UidType, KOfflineItemRow >::const_iterator mit;
	for( mit = m_mapItem.begin(); mit != m_mapItem.end(); ++mit )
	{
		const KOfflineItemRow& kRow = mit->second;

		if( kRow.m_iItemID != iItemID )
			continue;

		// The same four exclusions CX2Inventory::GetNumItemByTID makes, in the
		// same order: worn gear, the quick-slot bar, the private bank and the
		// shared bank. The banks are not implemented offline and no row can
		// currently land in one, but leaving them out of the sum keeps this
		// function's answer identical to the client's under any later phase
		// that does implement them.
		if( true == bExcludeEquipped &&
			( CX2Inventory::ST_E_EQUIP == kRow.m_iCategory ||
			  CX2Inventory::ST_E_QUICK_SLOT == kRow.m_iCategory ) )
		{
			continue;
		}

		if( CX2Inventory::ST_BANK == kRow.m_iCategory ||
			CX2Inventory::ST_SHARE_BANK == kRow.m_iCategory )
		{
			continue;
		}

		iCount += kRow.m_iQuantity;
	}

	return iCount;
}

int CX2OfflineInventory::ConsumeByID( int iItemID, int iQuantity,
									  OUT std::vector< KInventoryItemInfo >& vecChanged )
{
	if( iQuantity <= 0 )
		return 0;

	int iLeft = iQuantity;

	// Collected first, so the UIDs come out in insertion order and a partial
	// stack picked up early is the one that goes. DeleteItem mutates m_mapItem,
	// so the candidate list cannot be built by iterating it as we delete.
	std::vector< UidType > vecCandidate;

	std::map< UidType, KOfflineItemRow >::const_iterator mit;
	for( mit = m_mapItem.begin(); mit != m_mapItem.end(); ++mit )
	{
		const KOfflineItemRow& kRow = mit->second;

		if( kRow.m_iItemID != iItemID )
			continue;

		if( CX2Inventory::ST_E_EQUIP == kRow.m_iCategory ||
			CX2Inventory::ST_E_QUICK_SLOT == kRow.m_iCategory ||
			CX2Inventory::ST_BANK == kRow.m_iCategory ||
			CX2Inventory::ST_SHARE_BANK == kRow.m_iCategory )
		{
			continue;
		}

		vecCandidate.push_back( kRow.m_nItemUID );
	}

	for( size_t i = 0; i < vecCandidate.size() && iLeft > 0; ++i )
	{
		const KOfflineItemRow* pRow = FindRow( vecCandidate[i] );
		if( NULL == pRow )
			continue;

		const int iTake = ( pRow->m_iQuantity < iLeft ) ? pRow->m_iQuantity : iLeft;

		KInventoryItemInfo kChanged;
		if( false == DeleteItem( vecCandidate[i], iTake, kChanged ) )
			continue;

		vecChanged.push_back( kChanged );
		iLeft -= iTake;
	}

	return iQuantity - iLeft;
}

//////////////////////////////////////////////////////////////////////////

void CX2OfflineInventory::MakeItemInfo( const KOfflineItemRow& kRow,
										OUT KInventoryItemInfo& kOut ) const
{
	kOut.m_iItemUID			= kRow.m_nItemUID;
	kOut.m_cSlotCategory	= (char)kRow.m_iCategory;
	kOut.m_sSlotID			= (short)kRow.m_iSlotID;

	KItemInfo& kInfo = kOut.m_kItemInfo;

	kInfo.m_iItemID			= kRow.m_iItemID;
	kInfo.m_cUsageType		= (char)kRow.m_iUsageType;
	kInfo.m_iQuantity		= kRow.m_iQuantity;
	kInfo.m_sEndurance		= (short)kRow.m_iEndurance;
	kInfo.m_ucSealData		= (unsigned char)kRow.m_iSealData;
	kInfo.m_cEnchantLevel	= (char)kRow.m_iEnchantLevel;
	kInfo.m_cItemState		= (char)kRow.m_iItemState;
	kInfo.m_sPeriod			= (short)kRow.m_iPeriod;

	// No expiry clock offline. A period item would need one, and inventing a
	// countdown that the save file cannot represent would silently eat items;
	// nothing this phase creates sets a period.
	kInfo.m_wstrExpirationDate	= L"";

	kInfo.m_kAttribEnchantInfo.m_cAttribEnchant0 = (char)kRow.m_iAttrib0;
	kInfo.m_kAttribEnchantInfo.m_cAttribEnchant1 = (char)kRow.m_iAttrib1;
	kInfo.m_kAttribEnchantInfo.m_cAttribEnchant2 = (char)kRow.m_iAttrib2;

	kInfo.m_vecItemSocket	= kRow.m_vecSocket;
	kInfo.m_vecRandomSocket	= kRow.m_vecRandomSocket;
}

/*static*/ void CX2OfflineInventory::MakeEmptySlotInfo( int iCategory, int iSlotID,
														OUT KInventoryItemInfo& kOut )
{
	// UpdateInventorySlotList clears the slot named here and then skips the
	// re-add because m_iItemUID is not above zero. That is how the client is
	// told "this slot is empty now" - there is no separate remove packet.
	kOut.m_iItemUID			= 0;
	kOut.m_cSlotCategory	= (char)iCategory;
	kOut.m_sSlotID			= (short)iSlotID;
	kOut.m_kItemInfo		= KItemInfo();
}

void CX2OfflineInventory::MakeSlotInfo( int iCategory, int iSlotID,
										OUT KInventoryItemInfo& kOut ) const
{
	const UidType nItemUID = GetItemUID( iCategory, iSlotID );

	const KOfflineItemRow* pRow = ( 0 != nItemUID ) ? FindRow( nItemUID ) : NULL;

	if( NULL == pRow )
	{
		MakeEmptySlotInfo( iCategory, iSlotID, kOut );
		return;
	}

	MakeItemInfo( *pRow, kOut );
}

void CX2OfflineInventory::GetAllItems( OUT std::map< UidType, KInventoryItemInfo >& mapOut ) const
{
	mapOut.clear();

	std::map< UidType, KOfflineItemRow >::const_iterator mit;
	for( mit = m_mapItem.begin(); mit != m_mapItem.end(); ++mit )
	{
		// An item that could not be placed (see Load) is not sent: the client
		// would put it in a slot index that does not exist in its own vector.
		if( 0 == GetItemUID( mit->second.m_iCategory, mit->second.m_iSlotID ) )
			continue;

		KInventoryItemInfo kInfo;
		MakeItemInfo( mit->second, kInfo );

		mapOut[ mit->first ] = kInfo;
	}
}

void CX2OfflineInventory::GetEquippedItems( OUT std::map< int, KInventoryItemInfo >& mapOut ) const
{
	mapOut.clear();

	for( int iSlot = 0; iSlot < GetSlotSize( CX2Inventory::ST_E_EQUIP ); ++iSlot )
	{
		const UidType nItemUID = GetItemUID( CX2Inventory::ST_E_EQUIP, iSlot );
		if( 0 == nItemUID )
			continue;

		const KOfflineItemRow* pRow = FindRow( nItemUID );
		if( NULL == pRow )
			continue;

		KInventoryItemInfo kInfo;
		MakeItemInfo( *pRow, kInfo );

		mapOut[ iSlot ] = kInfo;
	}
}

//////////////////////////////////////////////////////////////////////////

void CX2OfflineInventory::PlaceItem( KOfflineItemRow& kRow, int iCategory, int iSlotID )
{
	if( true == VerifyCategorySlotID( kRow.m_iCategory, kRow.m_iSlotID ) &&
		m_vecSlot[ kRow.m_iCategory ][ kRow.m_iSlotID ] == kRow.m_nItemUID )
	{
		m_vecSlot[ kRow.m_iCategory ][ kRow.m_iSlotID ] = 0;
	}

	kRow.m_iCategory	= iCategory;
	kRow.m_iSlotID		= iSlotID;

	if( true == VerifyCategorySlotID( iCategory, iSlotID ) )
		m_vecSlot[ iCategory ][ iSlotID ] = kRow.m_nItemUID;

	CX2OfflineDB::Instance()->MoveItemRow( kRow.m_nItemUID, iCategory, iSlotID );
}

bool CX2OfflineInventory::GetEmptyCategorySlotForItem( int iItemID,
													   OUT int& iCategory, OUT int& iSlotID ) const
{
	iCategory = SuitableCategory( iItemID );

	for( int i = 0; i < GetSlotSize( iCategory ); ++i )
	{
		if( true == IsEmptySlot( iCategory, i ) )
		{
			iSlotID = i;
			return true;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineInventory::MoveItem( int iSrcCategory, int iSrcSlotID,
									int iDstCategory, int iDstSlotID,
									OUT std::vector< KInventoryItemInfo >& vecChanged )
{
	vecChanged.clear();

	// KInventory::MoveItem, step by step. The step numbers below are the
	// server's own comment numbers, so the two can be diffed.

	// 1. is the destination a slot that exists?
	if( false == VerifyCategorySlotID( iDstCategory, iDstSlotID ) )
		return false;

	// 2. the pet inventory needs a summoned pet. There are no pets offline, so
	// this is a flat refusal rather than a check.
	if( CX2Inventory::ST_PET == iDstCategory )
		return false;

	// The shared bank is one account's inventory shared across characters, and
	// its whole point is other characters. Refused rather than half-supported:
	// the packets that reconcile it (DBE_UPDATE_SHARE_ITEM_REQ and friends) do
	// not exist here, and an item moved in would be invisible to every other
	// character in the same save.
	if( CX2Inventory::ST_SHARE_BANK == iDstCategory ||
		CX2Inventory::ST_SHARE_BANK == iSrcCategory )
	{
		CX2OfflineLog::Server( L"ITEM     refused a shared-bank move (%d/%d -> %d/%d) - the shared"
			L" bank is not implemented offline",
			iSrcCategory, iSrcSlotID, iDstCategory, iDstSlotID );
		return false;
	}

	// 3 + 4. the item and where it is
	const UidType nItemUID = GetItemUID( iSrcCategory, iSrcSlotID );
	if( 0 == nItemUID )
		return false;

	KOfflineItemRow* pSrc = FindRow( nItemUID );
	if( NULL == pSrc )
		return false;

	// 5. moving onto itself is a no-op, and a successful one
	if( iSrcCategory == iDstCategory && iSrcSlotID == iDstSlotID )
		return true;

	// 6 + 7. can this item live there at all?
	const int iItemID = pSrc->m_iItemID;

	if( false == IsSuitableSlot( iItemID, iDstCategory, iDstSlotID ) )
		return false;

	// 8. equipping has its own conditions
	if( CX2Inventory::ST_E_EQUIP == iDstCategory )
	{
		if( false == IsAbleToEquip( iItemID ) )
			return false;

		// A worn-out endurance item cannot be worn, and neither can a sealed
		// one. Both are KInventory::MoveItem's own tests.
		if( CX2Item::PT_ENDURANCE == pSrc->m_iUsageType && pSrc->m_iEndurance <= 0 )
			return false;

		if( pSrc->m_iSealData >= 100 )
			return false;
	}

	// The server also restarts a per-item cooldown when something is dragged
	// onto the quick-slot bar mid-dungeon, so a potion cannot be swapped in to
	// dodge its own cooldown. Not carried over: the cooldown map lives in
	// KInventory purely to police that, the client keeps its own copy for the
	// UI, and offline there is nobody to police.

	// 9. an empty destination is a plain move
	if( true == IsEmptySlot( iDstCategory, iDstSlotID ) )
	{
		PlaceItem( *pSrc, iDstCategory, iDstSlotID );

		KInventoryItemInfo kInfo;

		MakeSlotInfo( iSrcCategory, iSrcSlotID, kInfo );
		vecChanged.push_back( kInfo );

		MakeSlotInfo( iDstCategory, iDstSlotID, kInfo );
		vecChanged.push_back( kInfo );

		return true;
	}

	// Otherwise something is already there.
	const UidType nDstItemUID = GetItemUID( iDstCategory, iDstSlotID );

	KOfflineItemRow* pDst = FindRow( nDstItemUID );
	if( NULL == pDst )
		return false;

	// Two stacks of the same quantity item merge instead of swapping.
	if( CX2Item::PT_QUANTITY == pSrc->m_iUsageType &&
		CX2Item::PT_QUANTITY == pDst->m_iUsageType &&
		pSrc->m_iItemID == pDst->m_iItemID )
	{
		const int iStack	= MaxStack( pDst->m_iItemID );
		const int iRoom		= iStack - pDst->m_iQuantity;

		if( iRoom > 0 )
		{
			const int iMoved = ( pSrc->m_iQuantity < iRoom ) ? pSrc->m_iQuantity : iRoom;

			pDst->m_iQuantity += iMoved;
			pSrc->m_iQuantity -= iMoved;

			CX2OfflineDB::Instance()->SetItemQuantity( pDst->m_nItemUID, pDst->m_iQuantity );

			if( pSrc->m_iQuantity <= 0 )
			{
				// The source stack is gone. Clear the slot first, then drop the
				// row - pSrc dangles after the erase.
				m_vecSlot[ iSrcCategory ][ iSrcSlotID ] = 0;

				CX2OfflineDB::Instance()->DeleteItemRow( nItemUID );
				m_mapItem.erase( nItemUID );
			}
			else
			{
				CX2OfflineDB::Instance()->SetItemQuantity( nItemUID, pSrc->m_iQuantity );
			}

			KInventoryItemInfo kInfo;

			MakeSlotInfo( iSrcCategory, iSrcSlotID, kInfo );
			vecChanged.push_back( kInfo );

			MakeSlotInfo( iDstCategory, iDstSlotID, kInfo );
			vecChanged.push_back( kInfo );
		}

		// A full destination stack is not an error - nothing moves, and the
		// server returns true here too so the client just redraws.
		return true;
	}

	// A swap needs the destination item to be legal in the source slot. When it
	// is not - dragging out of the bank or off the equip slots, mostly - the
	// item that was there is relocated to a free slot rather than the move
	// being refused.
	if( false == IsSuitableSlot( pDst->m_iItemID, iSrcCategory, iSrcSlotID ) )
	{
		if( CX2Inventory::ST_BANK != iDstCategory &&
			CX2Inventory::ST_E_EQUIP != iDstCategory )
		{
			return false;
		}

		int iEmptyCategory	= 0;
		int iEmptySlotID	= 0;

		if( false == GetEmptyCategorySlotForItem( pDst->m_iItemID, iEmptyCategory, iEmptySlotID ) )
			return false;			///< nowhere to put it, so nothing moves

		KInventoryItemInfo kInfo;

		// Vacate, move in, relocate - in that order, so the three slot infos
		// below describe the finished state.
		m_vecSlot[ iSrcCategory ][ iSrcSlotID ] = 0;

		PlaceItem( *pSrc, iDstCategory, iDstSlotID );
		PlaceItem( *pDst, iEmptyCategory, iEmptySlotID );

		MakeSlotInfo( iSrcCategory, iSrcSlotID, kInfo );
		vecChanged.push_back( kInfo );

		MakeSlotInfo( iDstCategory, iDstSlotID, kInfo );
		vecChanged.push_back( kInfo );

		MakeSlotInfo( iEmptyCategory, iEmptySlotID, kInfo );
		vecChanged.push_back( kInfo );

		return true;
	}

	// Unequipping by swap: whatever is going into the equip slot has to be
	// equippable too.
	if( CX2Inventory::ST_E_EQUIP == iSrcCategory )
	{
		if( false == IsAbleToEquip( pDst->m_iItemID ) )
			return false;

		if( CX2Item::PT_ENDURANCE == pDst->m_iUsageType && pDst->m_iEndurance <= 0 )
			return false;

		if( pDst->m_iSealData >= 100 )
			return false;
	}

	// A plain swap. Both slots are overwritten directly rather than through
	// PlaceItem, which would clear the slot it is about to fill.
	pSrc->m_iCategory	= iDstCategory;
	pSrc->m_iSlotID		= iDstSlotID;
	pDst->m_iCategory	= iSrcCategory;
	pDst->m_iSlotID		= iSrcSlotID;

	m_vecSlot[ iDstCategory ][ iDstSlotID ] = nItemUID;
	m_vecSlot[ iSrcCategory ][ iSrcSlotID ] = nDstItemUID;

	CX2OfflineDB::Instance()->MoveItemRow( nItemUID,    iDstCategory, iDstSlotID );
	CX2OfflineDB::Instance()->MoveItemRow( nDstItemUID, iSrcCategory, iSrcSlotID );

	KInventoryItemInfo kInfo;

	MakeSlotInfo( iSrcCategory, iSrcSlotID, kInfo );
	vecChanged.push_back( kInfo );

	MakeSlotInfo( iDstCategory, iDstSlotID, kInfo );
	vecChanged.push_back( kInfo );

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineInventory::DeleteItem( UidType nItemUID, int iQuantity,
									  OUT KInventoryItemInfo& kOut )
{
	KOfflineItemRow* pRow = FindRow( nItemUID );
	if( NULL == pRow )
		return false;

	const int iCategory	= pRow->m_iCategory;
	const int iSlotID	= pRow->m_iSlotID;

	// A quantity item can be partly discarded; anything else goes whole. A
	// request for zero or a negative count means the whole stack, which is what
	// the client sends for a non-quantity item.
	if( iQuantity > 0 && iQuantity < pRow->m_iQuantity )
	{
		pRow->m_iQuantity -= iQuantity;

		CX2OfflineDB::Instance()->SetItemQuantity( nItemUID, pRow->m_iQuantity );

		MakeItemInfo( *pRow, kOut );
		return true;
	}

	if( true == VerifyCategorySlotID( iCategory, iSlotID ) )
		m_vecSlot[ iCategory ][ iSlotID ] = 0;

	CX2OfflineDB::Instance()->DeleteItemRow( nItemUID );
	m_mapItem.erase( nItemUID );

	MakeEmptySlotInfo( iCategory, iSlotID, kOut );
	return true;
}

bool CX2OfflineInventory::ConsumeOne( UidType nItemUID, OUT KInventoryItemInfo& kOut )
{
	return DeleteItem( nItemUID, 1, kOut );
}

bool CX2OfflineInventory::SetEndurance( UidType nItemUID, int iEndurance,
										OUT KInventoryItemInfo& kOut )
{
	KOfflineItemRow* pRow = FindRow( nItemUID );
	if( NULL == pRow )
		return false;

	pRow->m_iEndurance = iEndurance;

	CX2OfflineDB::Instance()->UpdateItem( *pRow );

	MakeItemInfo( *pRow, kOut );
	return true;
}

//////////////////////////////////////////////////////////////////////////

namespace
{
	/// dbo.GBase_Item, transcribed.
	///
	/// This is the table dbo.gup_create_unit copies into GItem when a character
	/// is created - ten rows per class, and it aborts with @iOK = -28 if there
	/// are not exactly ten. Read out of the live Game01 database, because the
	/// copy under DataBase/ in this tree predates the GBase_Item refactor, and
	/// the per-class blocks still commented out inside gup_create_unit are what
	/// the table was built from rather than what it holds now.
	///
	/// The two halves are uniform, which is why slot, usage type and endurance
	/// are not repeated per row: the gear half is UsageType 1 (PT_ENDURANCE)
	/// with Endurance 10000 at NESI slots 9/1/3/5/7, and the costume half is
	/// UsageType 0 (PT_INFINITY) with Endurance -1 at 10/2/4/6/8. Both arrays
	/// are in weapon, body, leg, hand, foot order.
	///
	/// Keyed by the database's UnitClass, which is the value the unit row
	/// carries and which equals CX2Unit::UC_* for every base class - plain ints
	/// here rather than the enum names precisely because this is transcribed
	/// data, and because guarding rows behind NEW_CHARACTER_CHUNG and friends
	/// would silently drop a class instead of reporting it.
	///
	/// Classes 7, 8 and 9 all carry Ara's set. That is what the live table says,
	/// verbatim, and it is far more likely unfinished data than intent - Elesis
	/// appears to have been given Ara's row and never corrected, and class 9 is
	/// unreachable anyway because gup_create_unit rejects any class outside
	/// 1..8. Transcribed as-is; the seeders log when a class is handed a set
	/// that is not its own.
	struct KBaseItemSet
	{
		int	m_iUnitClass;
		int	m_aiGear[5];
		int	m_aiCostume[5];
	};

	const KBaseItemSet GBASE_ITEM[] =
	{
		{ 1, { 131641, 111094, 111095, 111096, 111097 }, { 128000, 128001, 128002, 128003, 128004 } },	///< Elsword
		{ 2, { 131642, 111098, 111099, 111100, 111101 }, { 128010, 128011, 128012, 128013, 128014 } },	///< Aisha
		{ 3, { 131643, 111102, 111103, 111104, 111105 }, { 128005, 128006, 128007, 128008, 128009 } },	///< Rena
		{ 4, { 131644, 111106, 111107, 111108, 111109 }, { 128072, 128073, 128074, 128075, 128076 } },	///< Raven
		{ 5, { 131645, 111110, 111111, 111112, 111113 }, { 130134, 130135, 130136, 130137, 130138 } },	///< Eve
		{ 6, { 111114, 111115, 111116, 111117, 111118 }, { 133125, 133126, 133127, 133128, 133129 } },	///< Chung
		{ 7, { 112700, 112701, 112702, 112703, 112704 }, {  41460,  41470,  41480,  41490,  41500 } },	///< Ara
		{ 8, { 112700, 112701, 112702, 112703, 112704 }, {  41460,  41470,  41480,  41490,  41500 } },	///< Elesis - Ara's set, see above
		{ 9, { 112700, 112701, 112702, 112703, 112704 }, {  41460,  41470,  41480,  41490,  41500 } },	///< unreachable, see above
	};

	/// Where each half's five pieces go, in the same order the arrays list them.
	/// The odd/even pairing is the fashion layer ([X2Unit.h:223](X2Lib/X2Unit.h#L223)),
	/// and these ten constants coming out as 9/1/3/5/7 and 10/2/4/6/8 is what
	/// confirms the transcription lines up with the enum.
	const int GEAR_SLOT[5] =
	{
		CX2Unit::NESI_WEAPON_HAND,
		CX2Unit::NESI_DEFENCE_BODY,
		CX2Unit::NESI_DEFENCE_LEG,
		CX2Unit::NESI_DEFENCE_HAND,
		CX2Unit::NESI_DEFENCE_FOOT,
	};

	const int COSTUME_SLOT[5] =
	{
		CX2Unit::NESI_WEAPON_HAND_FASHION,
		CX2Unit::NESI_DEFENCE_BODY_FASHION,
		CX2Unit::NESI_DEFENCE_LEG_FASHION,
		CX2Unit::NESI_DEFENCE_HAND_FASHION,
		CX2Unit::NESI_DEFENCE_FOOT_FASHION,
	};

	/// GBase_Item's Endurance column for every gear row.
	const int GEAR_ENDURANCE = 10000;

	/// The pre-2013 beginner gear the client tables in this tree carry instead:
	/// <class>002000 for the weapon and these offsets for body, leg, hand and
	/// foot. Only a fallback - see SeedBeginnerGear.
	const int GEAR_OFFSET_FALLBACK[5] = { 0, 50, 140, 230, 320 };

	const KBaseItemSet* FindBaseItemSet( int iUnitClass )
	{
		for( int i = 0; i < (int)( sizeof( GBASE_ITEM ) / sizeof( GBASE_ITEM[0] ) ); ++i )
		{
			if( GBASE_ITEM[i].m_iUnitClass == iUnitClass )
				return &GBASE_ITEM[i];
		}

		return NULL;
	}
}

bool CX2OfflineInventory::PlaceAtEquipSlot( UidType nUnitUID, int iItemID, int iSlotID,
										   int iPeriodType, int iEndurance, OUT bool& bWorn )
{
	bWorn = false;

	if( iSlotID < 0 || iSlotID >= GetSlotSize( CX2Inventory::ST_E_EQUIP ) ||
		false == IsEmptySlot( CX2Inventory::ST_E_EQUIP, iSlotID ) )
	{
		// No such slot in this build, or something is already in it. Into the bag,
		// which is where an unpositioned gup_insert_item row goes anyway
		// (InventoryCategory 0, SlotID 0, relocated on load).
		std::vector< KInventoryItemInfo > vecChanged;
		int iInserted = 0;

		return InsertItem( iItemID, 1, 0, vecChanged, iInserted );
	}

	KOfflineItemRow kRow;
	kRow.m_nUnitUID		= nUnitUID;
	kRow.m_iCategory	= CX2Inventory::ST_E_EQUIP;
	kRow.m_iSlotID		= iSlotID;
	kRow.m_iItemID		= iItemID;
	kRow.m_iQuantity	= 1;
	kRow.m_iUsageType	= iPeriodType;
	kRow.m_iItemState	= KItemInfo::IS_NORMAL;
	kRow.m_iEndurance	= iEndurance;

	if( false == CX2OfflineDB::Instance()->InsertItem( kRow ) )
		return false;

	m_mapItem[ kRow.m_nItemUID ] = kRow;
	m_vecSlot[ CX2Inventory::ST_E_EQUIP ][ iSlotID ] = kRow.m_nItemUID;

	bWorn = true;
	return true;
}

bool CX2OfflineInventory::SeedBeginnerGear( UidType nUnitUID, int iUnitClass )
{
	const KBaseItemSet* pSet = FindBaseItemSet( iUnitClass );
	if( NULL == pSet )
	{
		CX2OfflineLog::Server( L"ITEM     NOTE dbo.GBase_Item has no row for class %d, so the"
			L" character starts with no equipment", iUnitClass );
		return false;
	}

	Load( nUnitUID );

	int iWorn	= 0;
	int iBagged	= 0;
	int iMissing	= 0;

	for( int i = 0; i < 5; ++i )
	{
		int iItemID = pSet->m_aiGear[i];

		const CX2Item::ItemTemplet* pTemplet = Templet( iItemID );

		if( NULL == pTemplet && iUnitClass >= 1 && iUnitClass <= 5 )
		{
			// The live table and the client tables in this tree disagree about this
			// half, and which one is right depends on what is packed in the .kom the
			// client actually loads. GBase_Item is tried first because it is what
			// built real characters; ScriptData/ItemTemplet.xlsx's 2013-reform block
			// is the fallback because it is the only other set that can be read out
			// and cited. Both are real data, and the log says which one was used -
			// nothing here is derived or approximated.
			const int iAlt = ( iUnitClass * 1000000 ) + 2000 + GEAR_OFFSET_FALLBACK[i];

			const CX2Item::ItemTemplet* pAlt = Templet( iAlt );
			if( NULL != pAlt )
			{
				CX2OfflineLog::Server( L"ITEM     NOTE GBase_Item item %d is not in this client's"
					L" item table; using the ItemTemplet.xlsx beginner block's %d instead",
					iItemID, iAlt );

				iItemID	= iAlt;
				pTemplet = pAlt;
			}
		}

		if( NULL == pTemplet )
		{
			++iMissing;
			continue;			///< Templet() logged the ID the client cannot resolve
		}

		bool bWorn = false;
		if( false == PlaceAtEquipSlot( nUnitUID, iItemID, GEAR_SLOT[i],
									   CX2Item::PT_ENDURANCE, GEAR_ENDURANCE, bWorn ) )
			continue;

		if( true == bWorn )
			++iWorn;
		else
			++iBagged;

		// The name out of the client's own table, so the log alone is enough to
		// confirm the transcription without opening a spreadsheet.
		CX2OfflineLog::Server( L"ITEM     beginner gear: %d '%s' (level %d) -> slot %d %s",
			iItemID, pTemplet->GetName(), pTemplet->GetUseLevel(), GEAR_SLOT[i],
			( true == bWorn ) ? L"equipped" : L"(bagged)" );
	}

	CX2OfflineLog::Server( L"ITEM     beginner equipment for class %d: %d worn, %d bagged,"
		L" %d unresolvable. dbo.GBase_Item equips all five at creation, so anything not"
		L" worn here is a difference between this install's packed item table and the"
		L" live one", iUnitClass, iWorn, iBagged, iMissing );

	return ( iWorn > 0 || iBagged > 0 );
}

bool CX2OfflineInventory::SeedPromotionItems( UidType nUnitUID, int iUnitClass )
{
	const KBaseItemSet* pSet = FindBaseItemSet( iUnitClass );
	if( NULL == pSet )
	{
		CX2OfflineLog::Server( L"ITEM     NOTE dbo.GBase_Item has no row for class %d, so the"
			L" character starts with no promotional costume", iUnitClass );
		return false;
	}

	// The live table gives classes 7, 8 and 9 the same costume, so say so rather
	// than letting Elesis quietly wear Ara's clothes with no note anywhere.
	if( iUnitClass > 7 )
	{
		CX2OfflineLog::Server( L"ITEM     NOTE dbo.GBase_Item gives class %d the same set as"
			L" class 7 (Ara). That is the live table verbatim, not a substitution made"
			L" here - it looks like data the studio never finished", iUnitClass );
	}

	// Load first: the rows go straight into the slot grid, and the grid has to
	// be this character's.
	Load( nUnitUID );

	int iWorn	= 0;
	int iBagged	= 0;
	int iMissing	= 0;

	for( int i = 0; i < 5; ++i )
	{
		const int iItemID = pSet->m_aiCostume[i];

		// No fallback for the costume half: unlike the gear, there is no second
		// set of costume IDs anywhere in the repo to fall back to, and inventing
		// one is exactly what the project rule forbids.
		if( NULL == Templet( iItemID ) )
		{
			++iMissing;
			continue;
		}

		bool bWorn = false;
		if( false == PlaceAtEquipSlot( nUnitUID, iItemID, COSTUME_SLOT[i],
									   CX2Item::PT_INFINITY, -1, bWorn ) )
			continue;

		if( true == bWorn )
			++iWorn;
		else
			++iBagged;
	}

	// dbo.gup_create_unit_set_promotion gives each of these a GItemPeriod row of
	// 7 days, so on the live server the costume is a rental that expires. It is
	// permanent here: nothing offline implements item-period expiry, and a
	// costume that silently vanished a week in would read as a persistence bug
	// rather than as a feature. Deliberate divergence, recorded in the plan.
	CX2OfflineLog::Server( L"ITEM     promotional costume for class %d: %d worn, %d bagged,"
		L" %d unresolvable (permanent here; live gives it a 7-day period)",
		iUnitClass, iWorn, iBagged, iMissing );

	return ( iWorn > 0 || iBagged > 0 );
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineInventory::InsertItem( int iItemID, int iQuantity, int iEnchantLevel,
									  OUT std::vector< KInventoryItemInfo >& vecChanged,
									  OUT int& iInserted )
{
	vecChanged.clear();
	iInserted = 0;

	if( iQuantity <= 0 )
		return false;

	const CX2Item::ItemTemplet* pTemplet = Templet( iItemID );
	if( NULL == pTemplet )
		return false;

	const int iCategory = SuitableCategory( iItemID );
	if( CX2Inventory::ST_NONE == iCategory )
		return false;

	const bool bQuantity	= ( CX2Item::PT_QUANTITY == pTemplet->GetPeriodType() );
	const int  iStack		= MaxStack( iItemID );

	int iRemaining = iQuantity;

	// Top up the stacks that already exist, lowest slot first. This is
	// KInventory::FillTheUnFullSlot, and it is why a second potion of the same
	// kind does not take a second slot.
	if( true == bQuantity )
	{
		for( int iSlot = 0; iSlot < GetSlotSize( iCategory ) && iRemaining > 0; ++iSlot )
		{
			const UidType nExisting = GetItemUID( iCategory, iSlot );
			if( 0 == nExisting )
				continue;

			KOfflineItemRow* pRow = FindRow( nExisting );
			if( NULL == pRow || pRow->m_iItemID != iItemID )
				continue;

			const int iRoom = iStack - pRow->m_iQuantity;
			if( iRoom <= 0 )
				continue;

			const int iAdd = ( iRemaining < iRoom ) ? iRemaining : iRoom;

			pRow->m_iQuantity	+= iAdd;
			iRemaining			-= iAdd;
			iInserted			+= iAdd;

			CX2OfflineDB::Instance()->SetItemQuantity( nExisting, pRow->m_iQuantity );

			KInventoryItemInfo kInfo;
			MakeSlotInfo( iCategory, iSlot, kInfo );
			vecChanged.push_back( kInfo );
		}
	}

	// Then empty slots.
	for( int iSlot = 0; iSlot < GetSlotSize( iCategory ) && iRemaining > 0; ++iSlot )
	{
		if( false == IsEmptySlot( iCategory, iSlot ) )
			continue;

		const int iAdd = ( true == bQuantity )
							? ( ( iRemaining < iStack ) ? iRemaining : iStack )
							: 1;

		KOfflineItemRow kRow;
		kRow.m_nUnitUID			= m_nUnitUID;
		kRow.m_iCategory		= iCategory;
		kRow.m_iSlotID			= iSlot;
		kRow.m_iItemID			= iItemID;
		kRow.m_iQuantity		= iAdd;
		kRow.m_iUsageType		= (int)pTemplet->GetPeriodType();
		kRow.m_iEnchantLevel	= iEnchantLevel;
		kRow.m_iItemState		= KItemInfo::IS_NORMAL;

		// An endurance item arrives at its templet's starting endurance, which
		// is what KGSUser does for a picked-up item (GSUserRoomCommon.cpp:2616).
		kRow.m_iEndurance		= ( CX2Item::PT_ENDURANCE == pTemplet->GetPeriodType() )
									? pTemplet->GetEndurance()
									: 0;

		if( false == CX2OfflineDB::Instance()->InsertItem( kRow ) )
			break;

		m_mapItem[ kRow.m_nItemUID ] = kRow;
		m_vecSlot[ iCategory ][ iSlot ] = kRow.m_nItemUID;

		iRemaining	-= iAdd;
		iInserted	+= iAdd;

		KInventoryItemInfo kInfo;
		MakeSlotInfo( iCategory, iSlot, kInfo );
		vecChanged.push_back( kInfo );
	}

	if( iRemaining > 0 )
	{
		// Partial. The caller decides what that means - a shop refuses the
		// purchase up front, a drop drops what fits and says so. Nothing is
		// silently discarded without a line here.
		CX2OfflineLog::Server( L"ITEM     inventory full: %d of %d x item %d went in, %d had"
			L" nowhere to go", iInserted, iQuantity, iItemID, iRemaining );
	}

	return ( iInserted > 0 );
}

bool CX2OfflineInventory::HasRoomFor( int iItemID, int iQuantity ) const
{
	if( iQuantity <= 0 )
		return true;

	const CX2Item::ItemTemplet* pTemplet = Templet( iItemID );
	if( NULL == pTemplet )
		return false;

	const int iCategory = SuitableCategory( iItemID );
	if( CX2Inventory::ST_NONE == iCategory )
		return false;

	const bool bQuantity	= ( CX2Item::PT_QUANTITY == pTemplet->GetPeriodType() );
	const int  iStack		= MaxStack( iItemID );

	int iRemaining = iQuantity;

	if( true == bQuantity )
	{
		for( int iSlot = 0; iSlot < GetSlotSize( iCategory ) && iRemaining > 0; ++iSlot )
		{
			const UidType nExisting = GetItemUID( iCategory, iSlot );
			if( 0 == nExisting )
				continue;

			const KOfflineItemRow* pRow = FindRow( nExisting );
			if( NULL == pRow || pRow->m_iItemID != iItemID )
				continue;

			const int iRoom = iStack - pRow->m_iQuantity;
			if( iRoom > 0 )
				iRemaining -= iRoom;
		}
	}

	for( int iSlot = 0; iSlot < GetSlotSize( iCategory ) && iRemaining > 0; ++iSlot )
	{
		if( true == IsEmptySlot( iCategory, iSlot ) )
			iRemaining -= ( true == bQuantity ) ? iStack : 1;
	}

	return ( iRemaining <= 0 );
}

//////////////////////////////////////////////////////////////////////////

bool CX2OfflineInventory::SortCategory( int iCategory,
										OUT std::vector< UidType >& vecSlotOut )
{
	vecSlotOut.clear();

	const int iSize = GetSlotSize( iCategory );
	if( iSize <= 0 )
		return false;

	// Compact towards slot 0, keeping the order the items are already in.
	// KInventory::SortInventory has its own move loop commented out and only
	// reports the category back, so the compaction here is ours - but the reply
	// shape is the server's: every slot in the category, in order.
	std::vector< UidType > vecPacked;

	for( int iSlot = 0; iSlot < iSize; ++iSlot )
	{
		const UidType nItemUID = GetItemUID( iCategory, iSlot );
		if( 0 != nItemUID )
			vecPacked.push_back( nItemUID );
	}

	for( int iSlot = 0; iSlot < iSize; ++iSlot )
		m_vecSlot[ iCategory ][ iSlot ] = 0;

	for( size_t i = 0; i < vecPacked.size(); ++i )
	{
		KOfflineItemRow* pRow = FindRow( vecPacked[i] );
		if( NULL == pRow )
			continue;

		pRow->m_iCategory	= iCategory;
		pRow->m_iSlotID		= (int)i;

		m_vecSlot[ iCategory ][ i ] = pRow->m_nItemUID;

		CX2OfflineDB::Instance()->MoveItemRow( pRow->m_nItemUID, iCategory, (int)i );
	}

	for( int iSlot = 0; iSlot < iSize; ++iSlot )
		vecSlotOut.push_back( GetItemUID( iCategory, iSlot ) );

	return true;
}

#endif SERV_IRUHADEV_OFFLINE
