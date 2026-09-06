//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 1 - runs the shipped scripts in an
// embedded Lua 5.1 state and collects the item catalog and the cash shop
// tab table. See CASH_SHOP_TOOL_PLAN.md section 3.
//////////////////////////////////////////////////////////////////////////
#include "ItemIndex.h"

// Lua is the whole reason this file must stay native: luaconf.h takes its
// C++ branch in this build, so LUAI_THROW is a C++ throw and every Lua
// error is an exception. lua_pcall catches its own, but no managed frame
// may ever sit between the call and these callbacks (plan sections 6/7).
#ifdef _MANAGED
#error "This TU compiled managed. /clr leaked in - see CppBuild.targets:733-738"
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <algorithm>
#include <map>

// Included directly, never via lua.hpp and never behind our own extern "C":
// luaLib.lib exports C++-mangled symbols because luaLib_2010.vcxproj
// compiles the whole project as C++ (plan section 7).
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "LuaXor.h"

namespace
{
	const char* const	SCRIPT_ENUM		= "Enum.lua";
	const char* const	SCRIPT_ITEM		= "Item.lua";
	const char* const	SCRIPT_TRANS	= "ItemTrans.lua";
	const char* const	SCRIPT_CATEGORY	= "CashShopCategory.lua";

	const char* const	ARCHIVE_NAME	= "data036.kom";

	//////////////////////////////////////////////////////////////////////

	// Phase 2 moved this into KomArchive.h as CToolStopwatch so the icon
	// locator can report its own timings from the same clock. The local
	// name is kept because it reads better at the call sites here.
	typedef CToolStopwatch	CStopwatch;

	//////////////////////////////////////////////////////////////////////
	// What the Lua callbacks write into.

	struct SCollector
	{
		SExtractResult*			pResult;
		std::map<int, size_t>	mapIdToIndex;
		std::map<std::string, int>	mapStubbedCalls;

		// Resolved from Enum.lua once, before Item.lua runs.
		int	iUcNone;
		int	iUcAnyone;
		int	iIgNormal;
		int	iEpQuickSlot;

		SCollector()
		: pResult( NULL ), iUcNone( 0 ), iUcAnyone( 1 ), iIgNormal( 0 ), iEpQuickSlot( 0 )
		{}
	};

	//////////////////////////////////////////////////////////////////////
	// Field readers.
	//
	// Every one of these leaves the stack exactly as it found it, and none
	// of them calls lua_tolstring on a value that is not already a string -
	// that call converts numbers to strings IN PLACE, which would mutate
	// the very table being read.

	// pbPresent distinguishes "the key is absent" from "the key holds this
	// value", which the client's own macros distinguish too: LUA_GET_VALUE
	// falls back to a default, LUA_GET_VALUE_RETURN rejects the whole item.
	int GetIntField( lua_State* pLua, int iTable, const char* pszKey, int iDefault, bool* pbPresent = NULL )
	{
		lua_getfield( pLua, iTable, pszKey );

		int iValue = iDefault;
		const bool bPresent = ( 0 != lua_isnumber( pLua, -1 ) );
		if( bPresent )
			iValue = (int) lua_tointeger( pLua, -1 );

		if( NULL != pbPresent )
			*pbPresent = bPresent;

		lua_pop( pLua, 1 );
		return iValue;
	}

	// A named member of one of Enum.lua's tables, e.g. USE_CONDITION.UC_NONE.
	// Reading these instead of hardcoding them means the tool cannot drift
	// from the data, and it works whether the script publishes flat globals
	// or tables - this build turns out to publish only tables.
	int ReadEnumConstant( lua_State* pLua, const char* pszTable, const char* pszMember,
						int iFallback, bool* pbFound )
	{
		int iValue = iFallback;
		if( NULL != pbFound )
			*pbFound = false;

		lua_getglobal( pLua, pszTable );
		if( lua_istable( pLua, -1 ) )
		{
			lua_getfield( pLua, -1, pszMember );
			if( lua_isnumber( pLua, -1 ) )
			{
				iValue = (int) lua_tointeger( pLua, -1 );
				if( NULL != pbFound )
					*pbFound = true;
			}
			lua_pop( pLua, 1 );
		}
		lua_pop( pLua, 1 );

		return iValue;
	}

	int GetBoolField( lua_State* pLua, int iTable, const char* pszKey )
	{
		lua_getfield( pLua, iTable, pszKey );

		int iValue = 0;
		if( false == ( LUA_TNIL == lua_type( pLua, -1 ) ) )
			iValue = lua_toboolean( pLua, -1 ) ? 1 : 0;

		lua_pop( pLua, 1 );
		return iValue;
	}

	bool GetStringField( lua_State* pLua, int iTable, const char* pszKey, std::string& strOut )
	{
		strOut.clear();

		lua_getfield( pLua, iTable, pszKey );

		const bool bPresent = ( LUA_TSTRING == lua_type( pLua, -1 ) );
		if( bPresent )
		{
			size_t		uLength	= 0;
			const char*	pszValue	= lua_tolstring( pLua, -1, &uLength );
			if( NULL != pszValue )
				strOut.assign( pszValue, uLength );
		}

		lua_pop( pLua, 1 );
		return bPresent;
	}

	// The scripts call these both ways in principle - as a method
	// (g_pItemManager:AddItemTemplet{...}, which puts the receiver in slot 1
	// and the field table in slot 2) and, for the trans overlay, possibly
	// bare. Taking the first table argument found covers both without
	// caring which.
	int FindTableArgument( lua_State* pLua )
	{
		const int iTop = lua_gettop( pLua );

		if( iTop >= 2 && lua_istable( pLua, 2 ) )
			return 2;
		if( iTop >= 1 && lua_istable( pLua, 1 ) )
			return 1;

		return 0;
	}

	SCollector* Collector( lua_State* pLua )
	{
		return (SCollector*) lua_touserdata( pLua, lua_upvalueindex( 1 ) );
	}

	//////////////////////////////////////////////////////////////////////
	// The stubs.

	int l_AddItemTemplet( lua_State* pLua )
	{
		SCollector* pCollector = Collector( pLua );

		const int iTable = FindTableArgument( pLua );
		if( NULL == pCollector || 0 == iTable )
		{
			if( NULL != pCollector )
				++pCollector->pResult->iRejectedRows;

			lua_pushboolean( pLua, 0 );
			return 1;
		}

		SExtractResult* const pResult = pCollector->pResult;

		// The four rejections below are the client's, in the client's own
		// terms (X2Lib/X2ItemManager_Preprocessing.cpp:13-35): an item
		// missing any of m_ItemID / m_Name / m_ItemType is refused outright
		// by LUA_GET_VALUE_RETURN, and one whose m_UseCondition is UC_NONE
		// is "an item that cannot be used" and is refused as well.
		//
		// This is not tidiness. The catalog exists to be joined against
		// cash_product in phase 3 and to reproduce the client's own count
		// of rows dropped for naming an item it cannot resolve. A catalog
		// holding items the client refuses would make that join disagree
		// with the game, silently and in the flattering direction.
		SItemRow kRow;

		bool bHasID = false;
		kRow.iItemID = GetIntField( pLua, iTable, "m_ItemID", 0, &bHasID );

		if( false == bHasID || 0 == kRow.iItemID )
		{
			++pResult->iRejectedRows;
			++pResult->iRejectedNoID;
			lua_pushboolean( pLua, 0 );
			return 1;
		}

		const int iUseCondition = GetIntField( pLua, iTable, "m_UseCondition", pCollector->iUcAnyone );
		if( iUseCondition == pCollector->iUcNone )
		{
			++pResult->iRejectedRows;
			++pResult->iRejectedUnusable;
			lua_pushboolean( pLua, 0 );
			return 1;
		}

		if( false == GetStringField( pLua, iTable, "m_Name", kRow.strName ) )
		{
			++pResult->iRejectedRows;
			++pResult->iRejectedNoName;
			lua_pushboolean( pLua, 0 );
			return 1;
		}

		bool bHasType = false;
		kRow.iItemType = GetIntField( pLua, iTable, "m_ItemType", 0, &bHasType );

		if( false == bHasType )
		{
			++pResult->iRejectedRows;
			++pResult->iRejectedNoType;
			lua_pushboolean( pLua, 0 );
			return 1;
		}

		if( pCollector->mapIdToIndex.find( kRow.iItemID ) != pCollector->mapIdToIndex.end() )
		{
			// The client calls this DUPLICATED_ITEM_TEMPLET and drops it
			// (X2ItemManager_Preprocessing.cpp:491-497). First definition
			// wins here for the same reason.
			++pResult->iDuplicateIDs;
			lua_pushboolean( pLua, 0 );
			return 1;
		}

		GetStringField( pLua, iTable, "m_ShopImage", kRow.strShopImage );

		// These two fall back to the client's defaults, not to zero: an
		// absent m_ItemGrade means IG_NORMAL and an absent m_EqipPosition
		// means EP_QUICK_SLOT (X2ItemManager.cpp:430, 486).
		kRow.iItemGrade		= GetIntField( pLua, iTable, "m_ItemGrade", pCollector->iIgNormal );
		kRow.iIsFashion		= GetBoolField( pLua, iTable, "m_bFashion" );
		kRow.iEquipPosition	= GetIntField( pLua, iTable, "m_EqipPosition", pCollector->iEpQuickSlot );

		pCollector->mapIdToIndex.insert(
			std::make_pair( kRow.iItemID, pCollector->pResult->vecItems.size() ) );
		pCollector->pResult->vecItems.push_back( kRow );

		lua_pushboolean( pLua, 1 );
		return 1;
	}

	int l_AddItemTempletTrans( lua_State* pLua )
	{
		SCollector* pCollector = Collector( pLua );

		const int iTable = FindTableArgument( pLua );
		if( NULL == pCollector || 0 == iTable )
		{
			lua_pushboolean( pLua, 0 );
			return 1;
		}

		const int iItemID = GetIntField( pLua, iTable, "m_ItemID", 0 );

		std::map<int, size_t>::const_iterator iter = pCollector->mapIdToIndex.find( iItemID );
		if( 0 == iItemID || iter == pCollector->mapIdToIndex.end() )
		{
			// The client logs this as "ItemTrans.Lua And Item.lua Not Equal
			// ItemID" and drops it (X2ItemManager.cpp:958-966).
			++pCollector->pResult->iTransOrphans;
			lua_pushboolean( pLua, 0 );
			return 1;
		}

		std::string strName;
		GetStringField( pLua, iTable, "m_Name", strName );

		if( false == strName.empty() )
		{
			pCollector->pResult->vecItems[iter->second].strName = strName;
			++pCollector->pResult->iTransApplied;
		}

		lua_pushboolean( pLua, 1 );
		return 1;
	}

	// Answers for anything the stand-ins do not implement. Without it, one
	// unknown method name raises "attempt to call a nil value" and the
	// whole 34 MB chunk is lost; with it, the run completes and the names
	// it saw are reported, enumerating the full method list in one pass.
	int l_NoOp( lua_State* pLua )
	{
		lua_pushboolean( pLua, 1 );
		return 1;
	}

	int l_CatchAllIndex( lua_State* pLua )
	{
		SCollector* pCollector = Collector( pLua );

		if( NULL != pCollector && LUA_TSTRING == lua_type( pLua, 2 ) )
		{
			const char* pszKey = lua_tostring( pLua, 2 );
			if( NULL != pszKey )
				++pCollector->mapStubbedCalls[std::string( pszKey )];
		}

		// Upvalue 2 is the one shared no-op, created once when the stand-in
		// was installed - not a fresh closure per lookup.
		lua_pushvalue( pLua, lua_upvalueindex( 2 ) );
		return 1;
	}

	//////////////////////////////////////////////////////////////////////

	void PushCollectorClosure( lua_State* pLua, lua_CFunction pfn, SCollector* pCollector )
	{
		lua_pushlightuserdata( pLua, pCollector );
		lua_pushcclosure( pLua, pfn, 1 );
	}

	// Builds one stand-in receiver: a plain table of C functions with a
	// catch-all metatable. lua_tinker is not needed here - its value is
	// binding C++ classes with overload resolution, and this is a handful
	// of functions reading one table each (plan section 3).
	void InstallStandIn( lua_State* pLua, const char* pszGlobal, bool bWithItemMethods, SCollector* pCollector )
	{
		lua_newtable( pLua );

		if( bWithItemMethods )
		{
			PushCollectorClosure( pLua, l_AddItemTemplet, pCollector );
			lua_setfield( pLua, -2, "AddItemTemplet" );

			PushCollectorClosure( pLua, l_AddItemTempletTrans, pCollector );
			lua_setfield( pLua, -2, "AddItemTempletTrans" );
		}

		// metatable = { __index = <closure(collector, shared no-op)> }
		lua_newtable( pLua );
		lua_pushlightuserdata( pLua, pCollector );
		lua_pushcfunction( pLua, l_NoOp );
		lua_pushcclosure( pLua, l_CatchAllIndex, 2 );
		lua_setfield( pLua, -2, "__index" );
		lua_setmetatable( pLua, -2 );

		lua_setglobal( pLua, pszGlobal );
	}

	void InstallStubs( lua_State* pLua, SCollector* pCollector )
	{
		InstallStandIn( pLua, "g_pItemManager",			true,	pCollector );
		InstallStandIn( pLua, "g_pCX2SetItemManager",	false,	pCollector );

		// Also as bare globals. The client binds these as methods on
		// g_pItemManager (X2Lib/X2Main.cpp:3309, 3338), but the two call
		// shapes are indistinguishable from outside the bytecode and
		// FindTableArgument handles either, so registering both costs
		// nothing and removes the guess.
		PushCollectorClosure( pLua, l_AddItemTemplet, pCollector );
		lua_setglobal( pLua, "AddItemTemplet" );

		PushCollectorClosure( pLua, l_AddItemTempletTrans, pCollector );
		lua_setglobal( pLua, "AddItemTempletTrans" );
	}

	//////////////////////////////////////////////////////////////////////

	bool RunScript( lua_State* pLua, const CKomIndex& kIndex, const char* pszName,
					IToolLog* pLog, double* pdSeconds )
	{
		CStopwatch kWatch;

		std::vector<char>	vecScript;
		std::string			strError;

		if( false == kIndex.Read( pszName, vecScript, strError ) )
		{
			if( NULL != pLog )
				pLog->Linef( "  %-22s ERROR %s", pszName, strError.c_str() );
			return false;
		}

		if( vecScript.empty() )
		{
			if( NULL != pLog )
				pLog->Linef( "  %-22s ERROR inflated to nothing", pszName );
			return false;
		}

		const size_t uSize = vecScript.size();

		// Every packed .lua is XOR-encrypted; the archive container and the
		// encryption are two separate layers (X2OfflineStatTable.cpp:122-135).
		XorLuaScript( &vecScript[0], uSize );

		const bool bIsBytecode = ( uSize >= 4 && 0x1B == (unsigned char) vecScript[0]
			&& 'L' == vecScript[1] && 'u' == vecScript[2] && 'a' == vecScript[3] );

		const int iLoad = luaL_loadbuffer( pLua, &vecScript[0], uSize, pszName );

		// luaL_loadbuffer copies, so the plaintext can go now rather than
		// sitting next to Lua's own copy for the length of the call. For
		// Item.lua that is 34 MB released before the chunk even runs.
		std::vector<char>().swap( vecScript );

		if( 0 != iLoad )
		{
			const char* pszMessage = lua_tostring( pLua, -1 );
			if( NULL != pLog )
				pLog->Linef( "  %-22s ERROR load failed: %s", pszName, ( NULL != pszMessage ) ? pszMessage : "?" );
			lua_pop( pLua, 1 );
			return false;
		}

		const double dLoadSeconds = kWatch.Seconds();

		if( 0 != lua_pcall( pLua, 0, 0, 0 ) )
		{
			const char* pszMessage = lua_tostring( pLua, -1 );
			if( NULL != pLog )
				pLog->Linef( "  %-22s ERROR run failed: %s", pszName, ( NULL != pszMessage ) ? pszMessage : "?" );
			lua_pop( pLua, 1 );
			return false;
		}

		const double dTotalSeconds = kWatch.Seconds();
		if( NULL != pdSeconds )
			*pdSeconds = dTotalSeconds;

		if( NULL != pLog )
		{
			pLog->Linef( "  %-22s %10u bytes  %s  load %.2fs  run %.2fs", pszName,
				(unsigned) uSize, bIsBytecode ? "luac" : "text",
				dLoadSeconds, dTotalSeconds - dLoadSeconds );
		}

		return true;
	}

	// Enum.lua is the sole source of these names: the client's
	// CKTDXEnumToLua::LoadEnumToLua does nothing but run the script
	// (KTDXLIB/KTDXEnumToLua.cpp:12-31), it publishes no C++ enums. So if
	// none of these resolved, the later scripts would read every enum as
	// nil, lua_tonumber would yield 0, and the catalog would come out
	// silently mis-typed rather than erroring. Report what resolved and
	// refuse to continue if nothing did.
	bool ProbeEnumGlobals( lua_State* pLua, SCollector& kCollector, IToolLog* pLog )
	{
		// Enum.lua publishes TABLES, not flat globals: ITEM_TYPE is a table
		// and IT_WEAPON is a key inside it, so Item.lua's fields read
		// ITEM_TYPE.IT_WEAPON rather than a bare IT_WEAPON. Confirmed by
		// probing both shapes on the real file - the flat names all came
		// back nil while every table resolved.
		static const char* const s_apszTables[] =
		{
			"ITEM_TYPE", "ITEM_GRADE", "USE_CONDITION", "EQIP_POSITION",
			"CASH_SHOP_CATEGORY", "CASH_SHOP_SUB_CATEGORY",
		};

		std::string	strFound;
		int			iResolved = 0;

		for( size_t u = 0; u != sizeof( s_apszTables ) / sizeof( s_apszTables[0] ); ++u )
		{
			lua_getglobal( pLua, s_apszTables[u] );

			const int iType = lua_type( pLua, -1 );
			if( LUA_TTABLE == iType )
				++iResolved;

			if( false == strFound.empty() )
				strFound += "  ";
			strFound += s_apszTables[u];
			strFound += "=";
			strFound += lua_typename( pLua, iType );

			lua_pop( pLua, 1 );
		}

		if( NULL != pLog )
			pLog->Linef( "    enum tables : %s", strFound.c_str() );

		if( 0 == iResolved )
		{
			if( NULL != pLog )
			{
				pLog->Line( "    ERROR Enum.lua ran but defined none of the expected tables." );
				pLog->Line( "          Refusing to continue: every enum would read back as 0." );
			}
			return false;
		}

		bool bFoundUcNone		= false;
		bool bFoundUcAnyone		= false;
		bool bFoundIgNormal		= false;
		bool bFoundEpQuickSlot	= false;

		kCollector.iUcNone		= ReadEnumConstant( pLua, "USE_CONDITION", "UC_NONE",		0, &bFoundUcNone );
		kCollector.iUcAnyone	= ReadEnumConstant( pLua, "USE_CONDITION", "UC_ANYONE",		1, &bFoundUcAnyone );
		kCollector.iIgNormal	= ReadEnumConstant( pLua, "ITEM_GRADE",    "IG_NORMAL",		0, &bFoundIgNormal );
		kCollector.iEpQuickSlot	= ReadEnumConstant( pLua, "EQIP_POSITION", "EP_QUICK_SLOT",	0, &bFoundEpQuickSlot );

		kCollector.pResult->iEnumUcNone		= kCollector.iUcNone;
		kCollector.pResult->iEnumIgNormal	= kCollector.iIgNormal;
		kCollector.pResult->iEnumEpQuickSlot	= kCollector.iEpQuickSlot;

		if( NULL != pLog )
		{
			pLog->Linef( "    enum values : UC_NONE=%d%s  UC_ANYONE=%d%s  IG_NORMAL=%d%s  EP_QUICK_SLOT=%d%s",
				kCollector.iUcNone,		bFoundUcNone		? "" : " (FALLBACK)",
				kCollector.iUcAnyone,	bFoundUcAnyone		? "" : " (FALLBACK)",
				kCollector.iIgNormal,	bFoundIgNormal		? "" : " (FALLBACK)",
				kCollector.iEpQuickSlot,	bFoundEpQuickSlot	? "" : " (FALLBACK)" );
		}

		return true;
	}

	// CASH_SHOP_REAL_CATEGORY_ID = {
	//   [CASH_SHOP_CATEGORY.CSC_FASHION] = {
	//       REAL_ID = <n>,
	//       CASH_SHOP_REAL_SUB_CATEGORY_ID = { { <CSSC_* enum>, <billing no> }, ... },
	//   }, ... }
	// The client walks it the same way at X2Lib/X2CashShop.cpp:3133-3180,
	// reading the pair by index 1 and 2 - which is why the billing number
	// is the second element and not the enum.
	bool ReadCategoryTable( lua_State* pLua, SExtractResult& kResult, IToolLog* pLog )
	{
		lua_getglobal( pLua, "CASH_SHOP_REAL_CATEGORY_ID" );

		if( false == lua_istable( pLua, -1 ) )
		{
			lua_pop( pLua, 1 );
			if( NULL != pLog )
				pLog->Line( "    ERROR CASH_SHOP_REAL_CATEGORY_ID is not a table" );
			return false;
		}

		lua_pushnil( pLua );
		while( 0 != lua_next( pLua, -2 ) )
		{
			// key at -2, value at -1. The key is read with lua_tointeger
			// and never with lua_tolstring, which would convert it in place
			// and break this traversal.
			if( lua_isnumber( pLua, -2 ) && lua_istable( pLua, -1 ) )
			{
				const int iTabIdx = (int) lua_tointeger( pLua, -2 );
				const int iRealID = GetIntField( pLua, -1, "REAL_ID", 0 );

				lua_getfield( pLua, -1, "CASH_SHOP_REAL_SUB_CATEGORY_ID" );
				if( lua_istable( pLua, -1 ) )
				{
					for( int iOrdinal = 1; ; ++iOrdinal )
					{
						lua_rawgeti( pLua, -1, iOrdinal );
						if( false == lua_istable( pLua, -1 ) )
						{
							lua_pop( pLua, 1 );
							break;
						}

						SCashCategoryRow kRow;
						kRow.iTabIdx		= iTabIdx;
						kRow.iRealID		= iRealID;
						kRow.iSubOrdinal	= iOrdinal;

						lua_rawgeti( pLua, -1, 1 );
						kRow.iCsscEnum = lua_isnumber( pLua, -1 ) ? (int) lua_tointeger( pLua, -1 ) : 0;
						lua_pop( pLua, 1 );

						lua_rawgeti( pLua, -1, 2 );
						kRow.iBillingCategoryNo = lua_isnumber( pLua, -1 ) ? (int) lua_tointeger( pLua, -1 ) : 0;
						lua_pop( pLua, 1 );

						kResult.vecCategories.push_back( kRow );

						lua_pop( pLua, 1 );		// the pair
					}
				}
				lua_pop( pLua, 1 );				// CASH_SHOP_REAL_SUB_CATEGORY_ID
			}

			lua_pop( pLua, 1 );					// value, leaving the key for lua_next
		}

		lua_pop( pLua, 1 );						// CASH_SHOP_REAL_CATEGORY_ID

		return false == kResult.vecCategories.empty();
	}

	//////////////////////////////////////////////////////////////////////

	bool LessByItemID( const SItemRow& kLeft, const SItemRow& kRight )
	{
		return kLeft.iItemID < kRight.iItemID;
	}

	bool LessByTabThenOrdinal( const SCashCategoryRow& kLeft, const SCashCategoryRow& kRight )
	{
		if( kLeft.iTabIdx != kRight.iTabIdx )
			return kLeft.iTabIdx < kRight.iTabIdx;

		return kLeft.iSubOrdinal < kRight.iSubOrdinal;
	}

	bool MoreByCount( const SStubbedCall& kLeft, const SStubbedCall& kRight )
	{
		return kLeft.iCount > kRight.iCount;
	}
}

//////////////////////////////////////////////////////////////////////////

int ItemExtractorVersion()
{
	// Bump whenever the captured fields OR the acceptance rule change -
	// either one alters the contents while leaving an old cache looking
	// perfectly valid.
	//
	// 1 - phase 1: item_id, name, shop_image, item_type, item_grade,
	//              is_fashion, equip_position, plus the cash category table.
	// 2 - phase 1: adopted the client's own rejection rule (m_UseCondition
	//              UC_NONE, and m_ItemID / m_Name / m_ItemType required),
	//              and the client's defaults for grade and equip position.
	//              A version-1 cache holds ~6,000 items the client refuses.
	return 2;
}

const char* ItemCatalogArchiveName()
{
	return ARCHIVE_NAME;
}

bool ExtractItemCatalog( const CKomIndex& kIndex, SExtractResult& kResult, IToolLog* pLog )
{
	CStopwatch kTotal;

	kResult = SExtractResult();

	SCollector kCollector;
	kCollector.pResult = &kResult;

	lua_State* pLua = luaL_newstate();
	if( NULL == pLua )
	{
		if( NULL != pLog )
			pLog->Line( "  ERROR could not create a lua state" );
		return false;
	}

	luaL_openlibs( pLua );
	InstallStubs( pLua, &kCollector );

	bool bOk = true;

	if( bOk )	bOk = RunScript( pLua, kIndex, SCRIPT_ENUM, pLog, &kResult.dEnumSeconds );
	if( bOk )	bOk = ProbeEnumGlobals( pLua, kCollector, pLog );
	if( bOk )	bOk = RunScript( pLua, kIndex, SCRIPT_ITEM, pLog, &kResult.dItemSeconds );
	if( bOk )	bOk = RunScript( pLua, kIndex, SCRIPT_TRANS, pLog, &kResult.dTransSeconds );
	if( bOk )	bOk = RunScript( pLua, kIndex, SCRIPT_CATEGORY, pLog, &kResult.dCategorySeconds );
	if( bOk )	bOk = ReadCategoryTable( pLua, kResult, pLog );

	// Closed the moment extraction finishes, before anything else exists -
	// this is where the hundreds of megabytes of Proto/TString go back.
	lua_close( pLua );
	pLua = NULL;

	if( false == bOk )
		return false;

	for( size_t u = 0; u != kResult.vecItems.size(); ++u )
	{
		if( false == kResult.vecItems[u].strShopImage.empty() )
			++kResult.iWithShopImage;

		if( 0 == kResult.vecItems[u].iItemType )
			++kResult.iZeroItemType;
	}

	for( std::map<std::string, int>::const_iterator iter = kCollector.mapStubbedCalls.begin();
		iter != kCollector.mapStubbedCalls.end(); ++iter )
	{
		SStubbedCall kCall;
		kCall.strName	= iter->first;
		kCall.iCount	= iter->second;
		kResult.vecStubbedCalls.push_back( kCall );
	}

	std::sort( kResult.vecItems.begin(), kResult.vecItems.end(), LessByItemID );
	std::sort( kResult.vecCategories.begin(), kResult.vecCategories.end(), LessByTabThenOrdinal );
	std::sort( kResult.vecStubbedCalls.begin(), kResult.vecStubbedCalls.end(), MoreByCount );

	kResult.dTotalSeconds = kTotal.Seconds();

	// The canary from the plan: a catalog where nothing carries a type is
	// what an Enum.lua that failed to take looks like from the outside.
	if( false == kResult.vecItems.empty()
		&& kResult.iZeroItemType == (int) kResult.vecItems.size() )
	{
		if( NULL != pLog )
			pLog->Line( "  ERROR every item came back with m_ItemType 0 - Enum.lua did not take." );
		return false;
	}

	return false == kResult.vecItems.empty();
}
