#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-02
// Description: Offline mode - the inventory (phase 5).
//
//              A reduction of KncWX2Server/GameServer/Inventory.cpp's
//              KInventory to one character with no bank, no pet inventory, no
//              temp inventory and no personal shop, backed by the `item` table
//              rather than by 785 stored procedures.
//
//              Two things are deliberately NOT re-derived here:
//
//              * item templet data - CX2ItemManager already has all of it, the
//                same file the real CXSLItemManager read, so
//                g_pData->GetItemManager()->GetItemTemplet() is the source for
//                types, equip positions, stack sizes and prices;
//              * equippability - CX2Unit::CanEquipAsParts is the client's own
//                static check, including the class-compatibility table that
//                X2Lib has no other accessor for, so IsAbleToEquip calls it
//                instead of porting KInventory::CompareUnitClass.
//
//              MoveItem is KInventory::MoveItem (Inventory.cpp:5157) case for
//              case: empty destination moves, same-ID quantity merges, plain
//              swaps, and the "destination category will not take the item
//              that was there" path that relocates it to a free slot instead of
//              refusing. IsSuitableSlot is KInventory::IsSuitableSlot
//              (:946) with CXSL* renamed to CX2* and the templet fields
//              replaced by the optimized templet's accessors.
//
//              Every mutation writes through to SQLite before it returns. There
//              is no flush and no dirty set: the real server batched writes
//              because it had thousands of users and a DB round trip, and here
//              a move is one UPDATE on a local file at human speed. The exit
//              test for this phase is "every change persisted exactly", and
//              write-through is the only shape where that cannot be got wrong.
//
//              What is not implemented, and is not pretending to be: item
//              enhancement, socketing, identification, resolving, manufacture,
//              repair, the bank, pet inventories, the temp inventory, trade and
//              the personal shop. Their handlers refuse rather than half-work.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineDB.h"

class CX2OfflineInventory
{
public:
	static CX2OfflineInventory*	Instance();
	static void					Release();

	/// Load one character's items and slot sizes. Idempotent for the same unit,
	/// so the handlers can call it without tracking whether it has happened.
	bool	Load( UidType nUnitUID );
	void	Clear();

	UidType	GetUnitUID() const					{ return m_nUnitUID; }
	bool	IsLoaded() const					{ return 0 != m_nUnitUID; }

	//////////////////////////////////////////////////////////////////////////
	// reads

	/// Everything, in the shape EGS_SELECT_UNIT_1_NOT::m_mapItem wants it.
	void	GetAllItems( OUT std::map< UidType, KInventoryItemInfo >& mapOut ) const;

	/// Just the equipped ones, keyed by equip slot - KUnitInfo::m_mapEquippedItem.
	void	GetEquippedItems( OUT std::map< int, KInventoryItemInfo >& mapOut ) const;

	const std::map< int, int >&	GetSlotSizes() const	{ return m_mapSlotSize; }

	int		GetSlotSize( int iCategory ) const;
	UidType	GetItemUID( int iCategory, int iSlotID ) const;
	bool	GetItemRow( UidType nItemUID, OUT KOfflineItemRow& kOut ) const;
	int		GetItemID( UidType nItemUID ) const;

	/// How many of an item the character holds, summing stacks.
	/// CX2Inventory::GetNumItemByTID (X2Inventory.cpp:390) with the same
	/// exclusions, because the client answers "is this collection quest done?"
	/// with that exact call and the two must never disagree: with
	/// bExcludeEquipped, a worn item, a quick-slotted one and anything in a
	/// bank do not count.
	int		CountItemByID( int iItemID, bool bExcludeEquipped ) const;

	/// Take iQuantity of an item away, oldest row first, for a quest handing in
	/// what it asked the player to collect. Returns how many it actually
	/// removed, which is short only when the bag did not hold enough.
	int		ConsumeByID( int iItemID, int iQuantity,
						 OUT std::vector< KInventoryItemInfo >& vecChanged );

	/// The equipped-gear stat total, added onto the base stat to make
	/// KUnitInfo::m_kGameStat.
	///
	/// Each item's contribution comes from the client's own
	/// CX2ItemStatCalculator, which is what CX2Item::GetStat itself uses under
	/// SERV_NEW_ITEM_SYSTEM_2013_05 (X2Item.cpp:245) - so the number here is
	/// the same one the character sheet prints. Reading it that way rather than
	/// through CX2Unit::GetEqipStat matters because GetEqipStat can only ever
	/// answer for the unit the player currently has selected, and the character
	/// list needs a stat for every character in it.
	///
	/// Socket and enchant contributions are not added: nothing offline can
	/// enchant or socket an item (both refuse - see Handlers_Inventory.cpp), so
	/// every item's socket list is empty and its enchant level is zero, and the
	/// templet stat is the whole of it.
	void	AddEquippedStat( IN OUT KStat& kInOut ) const;

	//////////////////////////////////////////////////////////////////////////
	// writes. Each fills vecChanged with one entry per touched slot, in the
	// shape UpdateInventorySlotList reads it: an entry whose m_iItemUID is 0
	// means "this slot is now empty".

	bool	MoveItem( int iSrcCategory, int iSrcSlotID, int iDstCategory, int iDstSlotID,
					  OUT std::vector< KInventoryItemInfo >& vecChanged );

	/// Discard some or all of a stack. iQuantity <= 0, or >= the whole stack,
	/// removes the item.
	bool	DeleteItem( UidType nItemUID, int iQuantity,
						OUT KInventoryItemInfo& kOut );

	/// Put iQuantity of an item in. Fills the item's own partial stacks first
	/// and then empty slots in its natural category, exactly as
	/// KInventory::PrepareInsert does; iInserted comes back short when the
	/// inventory filled up.
	bool	InsertItem( int iItemID, int iQuantity, int iEnchantLevel,
						OUT std::vector< KInventoryItemInfo >& vecChanged,
						OUT int& iInserted );

	/// Spend one of a quantity item - a quick-slot potion, a skill-reset scroll.
	bool	ConsumeOne( UidType nItemUID, OUT KInventoryItemInfo& kOut );

	/// Set an item's remaining endurance, for the repair NPC.
	bool	SetEndurance( UidType nItemUID, int iEndurance, OUT KInventoryItemInfo& kOut );

	/// Give a brand-new character the five-piece promotional costume its class
	/// starts with, worn, at NESI fashion slots 10/2/4/6/8.
	///
	/// Item IDs, slots, usage type and endurance are all dbo.GBase_Item's, the
	/// table dbo.gup_create_unit copies into GItem at creation - see the
	/// transcription in the .cpp for where it came from and why the copy under
	/// DataBase/ is not the source. The client's own *_PROMOTION_ITEM_ID arrays
	/// ([X2StateCreateUnit.cpp:8-26](X2Lib/X2StateCreateUnit.cpp#L8)) agree with
	/// it for every class the creation screen covers, which is what makes this
	/// two independent sources rather than one.
	///
	/// Called with the new unit's UID, which is not the selected character yet,
	/// so nothing here may go through IsAbleToEquip - that reads the *live* unit.
	/// Taking the slot from the table sidesteps the question entirely.
	bool	SeedPromotionItems( UidType nUnitUID, int iUnitClass );

	/// Give a brand-new character its class's five-piece beginner equipment -
	/// weapon, body, leg, hand, foot - worn, at NESI slots 9/1/3/5/7.
	///
	/// Worn, because that is what the live server does: dbo.gup_create_unit
	/// copies ten positioned rows out of dbo.GBase_Item for the class (and
	/// aborts with -28 if there are not exactly ten), five gear pieces in the
	/// odd slots and five costume pieces in the even ones. Those slot numbers
	/// are CX2Unit::NESI_* ([X2Unit.h:223](X2Lib/X2Unit.h#L223)), where the odd
	/// entries are the real equip positions and the even ones their _FASHION
	/// twins - which is also why gup_create_unit_set_promotion dates
	/// `SlotID NOT IN (1,3,5,7,9)`: it is putting its 7-day clock on the costume
	/// half only.
	///
	/// The set is UseLevel 2 while a new character is level 1, and that is not a
	/// reason to put it in the bag instead. CX2Unit::ResetEqip filters through
	/// IsPossibleAddEqip -> EqipAbility -> CanEquipAsParts( .., m_Level )
	/// ([X2Unit.cpp:255](X2Lib/X2Unit.cpp#L255)), so the gear stays out of the
	/// stat and render lists until the first level-up - but the equipment window
	/// is drawn from inventory category 9, not from those lists, so it shows in
	/// its slots from the start. Live ships the same data into the same slots at
	/// level 1 and behaves the same way.
	///
	/// **The two available sources disagree, and this is the one place offline
	/// mode has to choose.** dbo.GBase_Item's gear IDs (131641.., 111094..,
	/// 112700.. for Ara) do not appear anywhere in this tree's
	/// ScriptData/ItemTemplet.xlsx, which instead carries a 2013-reform block
	/// laid out as <class>002000 for the weapon and +50/+140/+230/+320 for the
	/// armour. Whether an install resolves one, the other or both depends on
	/// what is packed in its .kom. So: GBase_Item is tried first, because it is
	/// what actually built characters on the live server; the xlsx block is the
	/// fallback, for classes 1-5 where it exists; and the log names which one
	/// each piece came from. Both are real, citable data - neither is derived.
	bool	SeedBeginnerGear( UidType nUnitUID, int iUnitClass );

	/// Compact one category towards slot 0, for EGS_SORT_CATEGORY_ITEM_REQ.
	/// Its ACK carries bare item UIDs in slot order rather than slot infos -
	/// CX2Inventory::UpdateCategorySlotList walks the vector and takes the index
	/// as the slot - so that is what comes back, zero for an empty slot.
	bool	SortCategory( int iCategory, OUT std::vector< UidType >& vecSlotOut );

	/// Is there room for iQuantity of iItemID? The ED shop asks before charging.
	bool	HasRoomFor( int iItemID, int iQuantity ) const;

	//////////////////////////////////////////////////////////////////////////
	// rules, ported from KInventory

	/// KInventory::IsSuitableSlot. Templet-only, so it is static.
	static bool	IsSuitableSlot( int iItemID, int iCategory, int iSlotID );

	/// KInventory::IsAbleToEquip, via CX2Unit::CanEquipAsParts - see the header
	/// comment for why it is not a port.
	static bool	IsAbleToEquip( int iItemID );

	/// KInventory::GetSuitableCategory - where an item of this ID naturally
	/// lives. SERV_AUTOMATICALLY_REGISTER_FOR_CONSUMABLE_ITEM is not defined in
	/// this build, so this is the single-category form and a quick-slot item
	/// goes to ST_QUICK_SLOT rather than straight onto the bar.
	static int	SuitableCategory( int iItemID );

	/// One stack's worth. PT_QUANTITY items stack to the templet's m_Quantity;
	/// everything else is one per slot.
	static int	MaxStack( int iItemID );

	/// The templet, or NULL with a log line. Every caller here has to cope with
	/// NULL: an item ID that is in the save file but not in the client's tables
	/// means the archives and the save disagree, which is worth saying out loud
	/// rather than crashing on.
	static const CX2Item::ItemTemplet*	Templet( int iItemID );

private:
	CX2OfflineInventory();
	~CX2OfflineInventory();

	/// The whole KItemInfo for one row, so a reply can carry it.
	void	MakeItemInfo( const KOfflineItemRow& kRow, OUT KInventoryItemInfo& kOut ) const;

	/// What UpdateInventorySlotList reads as "this slot is empty now".
	static void	MakeEmptySlotInfo( int iCategory, int iSlotID, OUT KInventoryItemInfo& kOut );

	/// Either of the two above, depending on whether the slot holds anything.
	void	MakeSlotInfo( int iCategory, int iSlotID, OUT KInventoryItemInfo& kOut ) const;

	bool	IsEmptySlot( int iCategory, int iSlotID ) const;
	bool	VerifyCategorySlotID( int iCategory, int iSlotID ) const;

	/// First empty slot in the item's natural category.
	bool	GetEmptyCategorySlotForItem( int iItemID, OUT int& iCategory, OUT int& iSlotID ) const;

	/// Place a row's UID in the slot grid and in the row itself, and persist it.
	void	PlaceItem( KOfflineItemRow& kRow, int iCategory, int iSlotID );

	/// Put one item in a named ST_E_EQUIP slot with a given usage type and
	/// endurance, or in the bag if this build has no such slot or something is
	/// already there. The shared half of the two Seed* calls, which take the
	/// slot straight out of dbo.GBase_Item rather than searching for one.
	bool	PlaceAtEquipSlot( UidType nUnitUID, int iItemID, int iSlotID,
							  int iPeriodType, int iEndurance, OUT bool& bWorn );

	KOfflineItemRow*		FindRow( UidType nItemUID );
	const KOfflineItemRow*	FindRow( UidType nItemUID ) const;

private:
	static CX2OfflineInventory*	ms_pInstance;

	UidType											m_nUnitUID;
	std::map< UidType, KOfflineItemRow >			m_mapItem;
	std::map< int, int >							m_mapSlotSize;

	/// category -> slot -> item UID (0 = empty). A vector per category sized
	/// from m_mapSlotSize, which is how KInventory::m_vecInventorySlot is
	/// shaped; keeping the same shape is what makes MoveItem a straight port.
	std::vector< UidType >							m_vecSlot[ CX2Inventory::ST_END ];
};

#endif SERV_IRUHADEV_OFFLINE
