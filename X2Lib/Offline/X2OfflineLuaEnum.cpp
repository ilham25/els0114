#include "stdafx.h"
#include "X2OfflineLuaEnum.h"

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineLog.h"
#include "X2OfflineLuaEnumSeed.h"

bool CX2OfflineLuaEnum::ms_bPublished = false;

//////////////////////////////////////////////////////////////////////////

/*static*/ void CX2OfflineLuaEnum::PublishTable( lua_State* pLuaState, const char* szGlobal,
												 const KEntry* pEntry, int iCount )
{
	// Built through the raw Lua C API rather than by running a generated chunk.
	// DoMemoryNotEncript would work too, but it asserts on a leading UTF-8 BOM
	// when the state is in UTF8 mode (KLuabinder.h:60) and a synthesised chunk
	// is one more thing to get subtly wrong; this is the same handful of calls
	// with nothing to encode.
	lua_newtable( pLuaState );

	for( int i = 0; i < iCount; ++i )
	{
		lua_pushstring( pLuaState, pEntry[i].m_szName );
		lua_pushnumber( pLuaState, (lua_Number)pEntry[i].m_iValue );

		// rawset, not settable: these tables are ours and fresh, so there is no
		// metatable to honour and no reason to pay for the lookup.
		lua_rawset( pLuaState, -3 );
	}

	lua_setglobal( pLuaState, szGlobal );
}

/*static*/ bool CX2OfflineLuaEnum::Publish()
{
	if( true == ms_bPublished )
		return true;

	if( NULL == g_pKTDXApp || NULL == g_pKTDXApp->GetLuaBinder() )
		return false;

	lua_State* pLuaState = g_pKTDXApp->GetLuaBinder()->GetLuaState();
	if( NULL == pLuaState )
		return false;

	using namespace X2OfflineLuaEnumSeed;

	PublishTable( pLuaState, "UNIT_CLASS",
		UNIT_CLASS,			sizeof( UNIT_CLASS ) / sizeof( UNIT_CLASS[0] ) );
	PublishTable( pLuaState, "USE_CONDITION",
		USE_CONDITION,		sizeof( USE_CONDITION ) / sizeof( USE_CONDITION[0] ) );
	PublishTable( pLuaState, "PET_UNIT_ID",
		PET_UNIT_ID,		sizeof( PET_UNIT_ID ) / sizeof( PET_UNIT_ID[0] ) );
	PublishTable( pLuaState, "RIDING_PET_UNIT_ID",
		RIDING_PET_UNIT_ID,	sizeof( RIDING_PET_UNIT_ID ) / sizeof( RIDING_PET_UNIT_ID[0] ) );
	PublishTable( pLuaState, "ITEM_TYPE",
		ITEM_TYPE,			sizeof( ITEM_TYPE ) / sizeof( ITEM_TYPE[0] ) );
	PublishTable( pLuaState, "ITEM_GRADE",
		ITEM_GRADE,			sizeof( ITEM_GRADE ) / sizeof( ITEM_GRADE[0] ) );
	PublishTable( pLuaState, "VILLAGE_MAP_ID",
		VILLAGE_MAP_ID,		sizeof( VILLAGE_MAP_ID ) / sizeof( VILLAGE_MAP_ID[0] ) );
	PublishTable( pLuaState, "ENCHANT_TYPE",
		ENCHANT_TYPE,		sizeof( ENCHANT_TYPE ) / sizeof( ENCHANT_TYPE[0] ) );
	PublishTable( pLuaState, "SOCKET_DATA_TYPE",
		SOCKET_DATA_TYPE,	sizeof( SOCKET_DATA_TYPE ) / sizeof( SOCKET_DATA_TYPE[0] ) );

	// RandomItemTable.lua writes `m_bGiveAll = True` - capital T, and not a Lua
	// keyword. On the server it is a global the script environment carries;
	// here it would be nil, which LUA_GET_VALUE reads as its init value, which
	// turns every give-all cube into a single-item draw.
	//
	// Pushed as the NUMBER 1, not as a Lua boolean, and that distinction is the
	// whole point. KLuaManager reads an int field through GET_BY_NAME, which
	// gates on lua_isnumber (KLuaManager.cpp:672) - and in Lua 5.1 lua_isnumber
	// is false for a boolean. A boolean `True` would therefore have read as
	// "field absent" and given exactly the silent single-draw bug this is here
	// to prevent. 1 reads back as 1, and is still truthy to any `if True then`.
	//
	// `False` is not used by any of these four files but is defined for
	// symmetry, because the failure it would cause is the same silent kind.
	lua_pushnumber( pLuaState, 1 );
	lua_setglobal( pLuaState, "True" );

	lua_pushnumber( pLuaState, 0 );
	lua_setglobal( pLuaState, "False" );

	// MapData.lua writes `DEFAULT = TRUE` - all caps, a separate global from the
	// lowercase True/False above (RandomItemTable.lua's spelling). Same reasoning:
	// pushed as a NUMBER so KLuaManager's lua_isnumber gate (LUA_GET_VALUE reads
	// DEFAULT as an int) sees it as present, not as a Lua boolean it would read
	// as absent.
	lua_pushnumber( pLuaState, 1 );
	lua_setglobal( pLuaState, "TRUE" );

	lua_pushnumber( pLuaState, 0 );
	lua_setglobal( pLuaState, "FALSE" );

	ms_bPublished = true;

	CX2OfflineLog::Server( L"LUA      published enum tables: UNIT_CLASS(%d) USE_CONDITION(%d)"
		L" PET_UNIT_ID(%d) RIDING_PET_UNIT_ID(%d) ITEM_TYPE(%d) ITEM_GRADE(%d) VILLAGE_MAP_ID(%d)"
		L" ENCHANT_TYPE(%d) SOCKET_DATA_TYPE(%d),"
		L" plus True/False and TRUE/FALSE",
		(int)( sizeof( UNIT_CLASS ) / sizeof( UNIT_CLASS[0] ) ),
		(int)( sizeof( USE_CONDITION ) / sizeof( USE_CONDITION[0] ) ),
		(int)( sizeof( PET_UNIT_ID ) / sizeof( PET_UNIT_ID[0] ) ),
		(int)( sizeof( RIDING_PET_UNIT_ID ) / sizeof( RIDING_PET_UNIT_ID[0] ) ),
		(int)( sizeof( ITEM_TYPE ) / sizeof( ITEM_TYPE[0] ) ),
		(int)( sizeof( ITEM_GRADE ) / sizeof( ITEM_GRADE[0] ) ),
		(int)( sizeof( VILLAGE_MAP_ID ) / sizeof( VILLAGE_MAP_ID[0] ) ),
		(int)( sizeof( ENCHANT_TYPE ) / sizeof( ENCHANT_TYPE[0] ) ),
		(int)( sizeof( SOCKET_DATA_TYPE ) / sizeof( SOCKET_DATA_TYPE[0] ) ) );

	return true;
}

#endif SERV_IRUHADEV_OFFLINE
