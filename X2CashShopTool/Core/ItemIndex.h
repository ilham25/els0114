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

	SCashCategoryRow()
	: iTabIdx( 0 ), iRealID( 0 ), iSubOrdinal( 0 ), iCsscEnum( 0 ), iBillingCategoryNo( 0 )
	{}
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
	std::vector<SStubbedCall>		vecStubbedCalls;

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
	double	dCategorySeconds;
	double	dTotalSeconds;

	SExtractResult()
	: iRejectedRows( 0 ), iRejectedNoID( 0 ), iRejectedNoName( 0 )
	, iRejectedNoType( 0 ), iRejectedUnusable( 0 )
	, iDuplicateIDs( 0 ), iTransApplied( 0 ), iTransOrphans( 0 )
	, iWithShopImage( 0 ), iZeroItemType( 0 )
	, iEnumUcNone( 0 ), iEnumIgNormal( 0 ), iEnumEpQuickSlot( 0 )
	, dEnumSeconds( 0.0 ), dItemSeconds( 0.0 ), dTransSeconds( 0.0 )
	, dCategorySeconds( 0.0 ), dTotalSeconds( 0.0 )
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
