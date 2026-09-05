#include "stdafx.h"
#include "X2OfflineMapData.h"

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineLog.h"
#include "X2OfflineLuaEnum.h"

CX2OfflineMapData* CX2OfflineMapData::ms_pInstance = NULL;

namespace
{
	const wchar_t* const SCRIPT_NAME = L"MapData.lua";
}

//////////////////////////////////////////////////////////////////////////

CX2OfflineMapData::CX2OfflineMapData()
: m_bLoadAttempted( false )
, m_bScriptRan( false )
{
}

CX2OfflineMapData::~CX2OfflineMapData()
{
}

CX2OfflineMapData* CX2OfflineMapData::Instance()
{
	if( NULL == ms_pInstance )
		ms_pInstance = new CX2OfflineMapData;

	return ms_pInstance;
}

void CX2OfflineMapData::Release()
{
	if( NULL == ms_pInstance )
		return;

	delete ms_pInstance;
	ms_pInstance = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Loading

void CX2OfflineMapData::EnsureLoaded()
{
	if( true == m_bLoadAttempted )
		return;

	m_bLoadAttempted = true;

	if( NULL == g_pKTDXApp ||
		NULL == g_pKTDXApp->GetLuaBinder() ||
		NULL == g_pKTDXApp->GetDeviceManager() ||
		NULL == g_pKTDXApp->GetDeviceManager()->GetMassFileManager() )
	{
		CX2OfflineLog::Server( L"WARP     ERROR engine not ready - Cobo Express stays refused." );
		return;
	}

	lua_State* pLuaState = g_pKTDXApp->GetLuaBinder()->GetLuaState();
	if( NULL == pLuaState )
	{
		CX2OfflineLog::Server( L"WARP     ERROR no lua state - Cobo Express stays refused." );
		return;
	}

	// VILLAGE_MAP_ID (every VMI_* subscript MapData.lua uses) and the all-caps
	// TRUE it writes as `DEFAULT = TRUE` both have to exist before the chunk
	// runs, or every one of them reads back nil. See CX2OfflineLuaEnum.
	if( false == CX2OfflineLuaEnum::Publish() )
	{
		CX2OfflineLog::Server( L"WARP     ERROR enum tables could not be published - not loading '%s'.",
			SCRIPT_NAME );
		return;
	}

	// Bound as MapData:*, exactly as CXSLMapData::RegisterLuaBind does on the
	// server side (XSLMapData.cpp:29-40).
	lua_tinker::class_add< CX2OfflineMapData >( pLuaState, "CX2OfflineMapData" );
	lua_tinker::class_def< CX2OfflineMapData >( pLuaState, "AddMapData",		&CX2OfflineMapData::AddMapData_LUA );
	lua_tinker::class_def< CX2OfflineMapData >( pLuaState, "AddLocalMapInfo",	&CX2OfflineMapData::AddLocalMapInfo_LUA );
	lua_tinker::decl( pLuaState, "MapData", this );

	// Archive first, loose file second - see X2OfflineStatTable::EnsureLoaded
	// for why (MASS_FILE_FIRST, _SERVICE_).
	KGCMassFileManager::CMassFile::MASSFILE_MEMBERFILEINFO_POINTER kInfo;
	kInfo = g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LoadDataFile( SCRIPT_NAME );

	// NOT `NULL == kInfo`. MASSFILE_MEMBERFILEINFO_POINTER is a STRUCT BY
	// VALUE with an `operator const MASSFILE_MEMBERFILEINFO*() const` that
	// returns `this` (KGCMassFileManager.h), so comparing it against NULL
	// takes the address of a local and is never true. A missing file then
	// falls through to the DoMemory calls and is reported as "failed to run"
	// instead of "not packed" - which is the one diagnosis this branch
	// exists to give. The client's own loader tests the payload
	// (X2ItemManager.cpp:231); so does this.
	if( NULL == kInfo->pRealData || kInfo->size <= 0 )
	{
		CX2OfflineLog::Server(
			L"WARP     ERROR '%s' not found in any .kom or on disk - Cobo Express stays refused.",
			SCRIPT_NAME );
		CX2OfflineLog::Server(
			L"WARP     Pack KncWX2Server/ServerResource/US/MapData.lua into a .kom, or drop it in the game data folder." );
		return;
	}

	// Encrypted first (the shipped form), plaintext second (a loose test copy) -
	// same reasoning as X2OfflineStatTable::EnsureLoaded.
	m_bScriptRan = ( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemory( kInfo->pRealData, kInfo->size ) );

	if( false == m_bScriptRan )
	{
		const bool bPlain = ( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemoryNotEncript( kInfo->pRealData, kInfo->size ) );

		if( true == bPlain )
		{
			m_bScriptRan = true;
			CX2OfflineLog::Server(
				L"WARP     NOTE '%s' is NOT encrypted - loaded as plaintext. Fine for testing;"
				L" encrypt it (XOREncrypt / FileEncrypt) to match every other packed script.",
				SCRIPT_NAME );
		}
	}

	if( false == m_bScriptRan )
	{
		CX2OfflineLog::Server(
			L"WARP     ERROR '%s' failed to run, encrypted or plaintext - Cobo Express stays refused.",
			SCRIPT_NAME );
		return;
	}

	CX2OfflineLog::Server( L"WARP     '%s' loaded - Cobo Express warp button priced.", SCRIPT_NAME );
}

//////////////////////////////////////////////////////////////////////////
// Bound into Lua as MapData:* - both no-ops. See the file header for why they
// still have to exist.

void CX2OfflineMapData::AddMapData_LUA()
{
}

void CX2OfflineMapData::AddLocalMapInfo_LUA( int /*iVillageMapID*/, int /*iDungeonMode*/, int /*iDungeonIDWithDif*/ )
{
}

//////////////////////////////////////////////////////////////////////////

int CX2OfflineMapData::CheckCOBOExpressTicketMapID( int iMapID )
{
	EnsureLoaded();

	if( false == m_bScriptRan )
		return 0;

	return lua_tinker::call< int >( g_pKTDXApp->GetLuaBinder()->GetLuaState(),
		"CheckCOBOExpressTicketMapID", iMapID );
}

int CX2OfflineMapData::ComputeCOBOExpressTicketCost( int iSrcMapID, int iDesMapID, int iLevel )
{
	EnsureLoaded();

	if( false == m_bScriptRan )
		return 0;

	return lua_tinker::call< int >( g_pKTDXApp->GetLuaBinder()->GetLuaState(),
		"ComputeCOBOExpressTicketCost", iSrcMapID, iDesMapID, iLevel );
}

#endif SERV_IRUHADEV_OFFLINE
