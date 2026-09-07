//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 1 - the item catalog, extracted by
// running the shipped scripts in an embedded Lua 5.1 state.
// See CASH_SHOP_TOOL_PLAN.md section 3.
//
// Item.lua is 34 MB of luac bytecode, not source: there is no regex and no
// text scrape, the scripts have to be executed. No lua header appears here
// - that stays in the .cpp, on the native side of the /clr split, because
// Lua raises errors as C++ exceptions in this build and those must never
// unwind through a managed frame (plan sections 6 and 7).
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <vector>

#include "KomArchive.h"

// Every string below is UTF-8: that is what the scripts hold and what
// KLuaManager decodes with (luaLib/KLuaManager.h:803), so nothing is
// converted on the way in or on the way out to the cache.
struct SItemRow
{
	int			iItemID;
	std::string	strName;			// m_Name, after any ItemTrans.lua overlay
	std::string	strShopImage;		// m_ShopImage, a verbatim filename
	int			iItemType;			// m_ItemType
	int			iItemGrade;			// m_ItemGrade
	int			iIsFashion;			// m_bFashion
	int			iEquipPosition;		// m_EqipPosition (the script's own spelling)

	SItemRow()
	: iItemID( 0 ), iItemType( 0 ), iItemGrade( 0 ), iIsFashion( 0 ), iEquipPosition( 0 )
	{}
};

// One (tab, sub-tab) pair out of CashShopCategory.lua's
// CASH_SHOP_REAL_CATEGORY_ID. iBillingCategoryNo is the number that
// cash_product.category actually holds - the SECOND element of each pair,
// not the CSSC_* enum in the first (X2Lib/X2CashShop.cpp:6200, and the
// warning in X2Lib/Offline/X2OfflineCashShop.h:23-31).
struct SCashCategoryRow
{
	int	iTabIdx;
	int	iRealID;
	int	iSubOrdinal;
	int	iCsscEnum;
	int	iBillingCategoryNo;

	// The CSC_* and CSSC_* names, reversed out of Enum.lua's own
	// CASH_SHOP_CATEGORY and CASH_SHOP_SUB_CATEGORY tables rather than
	// transcribed into the tool - the same discipline phase 1 adopted for
	// UC_NONE and IG_NORMAL, and for the same reason: a transcribed name
	// drifts from the data and nothing says so.
	//
	// These are the SCRIPT's names, not the captions the game paints on
	// its tabs - those are localized strings this tool does not read. They
	// are here so a tab in the tool can be matched against a tab in the
	// game by something better than an ordinal. Empty when a value appears
	// in CashShopCategory.lua and not in Enum.lua, which is information
	// rather than an error.
	std::string	strTabName;
	std::string	strSubName;

	SCashCategoryRow()
	: iTabIdx( 0 ), iRealID( 0 ), iSubOrdinal( 0 ), iCsscEnum( 0 ), iBillingCategoryNo( 0 )
	{}
};

// One value -> name pair out of one of Enum.lua's tables, e.g.
// ITEM_TYPE.IT_WEAPON = 3 arrives here as ( "ITEM_TYPE", 3, "IT_WEAPON" ).
//
// Phase 5 added these because the picker filters by item type and equip
// slot, and a filter reading "item type 4" is a filter nobody can use. The
// names are reversed out of the script's own tables rather than
// transcribed - the same discipline phase 1 took for UC_NONE and phase 4
// for the CSC_* tab names, and for the same reason: a transcribed table
// drifts from the data and nothing says so.
struct SEnumNameRow
{
	std::string	strTable;		// "ITEM_TYPE", "EQIP_POSITION", ...
	int			iValue;
	std::string	strName;		// "IT_WEAPON", "EP_QUICK_SLOT", ...

	SEnumNameRow() : iValue( 0 ) {}
};

// A method name the catch-all __index answered for, and how often. Logging
// these enumerates the full method list in one run instead of one run per
// missing stub.
struct SStubbedCall
{
	std::string	strName;
	int			iCount;

	SStubbedCall() : iCount( 0 ) {}
};

struct SExtractResult
{
	std::vector<SItemRow>			vecItems;			// sorted by iItemID
	std::vector<SCashCategoryRow>	vecCategories;		// sorted by tab then ordinal
	std::vector<SEnumNameRow>		vecEnumNames;		// sorted by table then value
	std::vector<SStubbedCall>		vecStubbedCalls;

	// Items the shop will NOT show even with a perfectly valid
	// cash_product row, and the second silent-drop rule in this client
	// after the missing-templet one.
	//
	// CX2ItemManager::AddCashItem sets m_bShow from IsShowPackageItem
	// (X2Lib/X2ItemManager.cpp:1868) and GetAllCashItemList erases every
	// entry whose m_bShow is false (:2866-2880), so an item in this set is
	// filtered out of the shop AFTER the catalog packet has carried it.
	// The set is the items PackageItemData.lua declares as package
	// contents with bShowItem false - AddPackageItemData_LUA at
	// X2ItemManager.cpp:3288-3292 - i.e. things meant to arrive inside a
	// bundle and never to be sold on their own.
	//
	// Sorted ascending; use IsHiddenPackageItem to test membership. Empty
	// when PackageItemData.lua could not be run, which is reported rather
	// than assumed - see the note in ExtractItemCatalog.
	std::vector<int>				vecHiddenPackageItems;

	int	iPackageRows;			// AddPackageItemData calls seen
	int	iPackageHiddenRows;		// of those, the ones with bShowItem false
	bool	bPackageDataRan;	// false = the script failed; the set is empty and unknown, not empty and true

	// The catalog holds exactly the items the CLIENT would hold, because
	// its whole job is to be joined against cash_product and reproduce the
	// client's own "dropped for having no item templet" count. So the four
	// rejections below mirror CX2ItemManager::KProxy::AddItemTemplet_LUA
	// (X2Lib/X2ItemManager_Preprocessing.cpp:13-35) exactly. Dropping them
	// would inflate the catalog by ~6,000 items the client cannot resolve.
	int	iRejectedRows;			// total of the four below
	int	iRejectedNoID;			// no m_ItemID
	int	iRejectedNoName;		// no m_Name
	int	iRejectedNoType;		// no m_ItemType
	int	iRejectedUnusable;		// m_UseCondition == UC_NONE

	int	iDuplicateIDs;		// second and later definitions of an id already seen
	int	iTransApplied;		// ItemTrans.lua rows that matched an item
	int	iTransOrphans;		// ItemTrans.lua rows naming an id Item.lua never defined
	int	iWithShopImage;
	int	iZeroItemType;		// the Enum.lua canary - see the .cpp

	// Read out of Enum.lua's own tables rather than hardcoded, so the tool
	// cannot drift from the data the way a transcribed enum would.
	int	iEnumUcNone;
	int	iEnumIgNormal;
	int	iEnumEpQuickSlot;

	// Wall-clock seconds, filled in per script so a slow step is visible.
	double	dEnumSeconds;
	double	dItemSeconds;
	double	dTransSeconds;
	double	dPackageSeconds;
	double	dCategorySeconds;
	double	dTotalSeconds;

	SExtractResult()
	: iPackageRows( 0 ), iPackageHiddenRows( 0 ), bPackageDataRan( false )
	, iRejectedRows( 0 ), iRejectedNoID( 0 ), iRejectedNoName( 0 )
	, iRejectedNoType( 0 ), iRejectedUnusable( 0 )
	, iDuplicateIDs( 0 ), iTransApplied( 0 ), iTransOrphans( 0 )
	, iWithShopImage( 0 ), iZeroItemType( 0 )
	, iEnumUcNone( 0 ), iEnumIgNormal( 0 ), iEnumEpQuickSlot( 0 )
	, dEnumSeconds( 0.0 ), dItemSeconds( 0.0 ), dTransSeconds( 0.0 )
	, dPackageSeconds( 0.0 ), dCategorySeconds( 0.0 ), dTotalSeconds( 0.0 )
	{}
};

// Runs Enum.lua -> Item.lua -> ItemTrans.lua -> CashShopCategory.lua in one
// lua_State, in that order, and collects the result. The order is not
// optional: Item.lua reads its m_ItemType = IT_WEAPON as plain globals, and
// with Enum.lua not yet run those are nil, read back as 0, and produce a
// silently wrong index rather than an error.
bool ExtractItemCatalog( const CKomIndex& kIndex, SExtractResult& kResult, IToolLog* pLog );

// Bumped whenever the SET OF CAPTURED FIELDS changes, so an index built by
// an older build is discarded rather than quietly missing a column. This is
// the invalidation input people forget (plan, "The item index is a cache").
int	ItemExtractorVersion();

// The one archive every script above comes from.
const char*	ItemCatalogArchiveName();

//////////////////////////////////////////////////////////////////////////
// Reading the two tables above back, phase 5.

// The Enum.lua table names the picker's two filters are built from,
// returned rather than spelled out at the call site so the extractor and
// the Ui cannot disagree about the spelling - EQIP_POSITION in particular
// is the script's own misspelling and typing it twice is asking for it.
const char*	EnumTableItemType();		// "ITEM_TYPE"
const char*	EnumTableEquipPosition();	// "EQIP_POSITION"
const char*	EnumTableItemGrade();		// "ITEM_GRADE"

// Empty when that table has no name for that value, which is information
// (the value is in the data and not in Enum.lua) and not an error.
std::string	LookupEnumRowName( const std::vector<SEnumNameRow>& vecNames,
								const char* pszTable, int iValue );

// vecSorted must be SExtractResult::vecHiddenPackageItems, which is sorted
// ascending by both ExtractItemCatalog and CIndexCache::Load.
bool	IsHiddenPackageItem( const std::vector<int>& vecSorted, int iItemID );
