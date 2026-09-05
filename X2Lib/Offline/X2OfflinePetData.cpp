#include "stdafx.h"
#include "X2OfflinePetData.h"

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineLog.h"
#include "X2OfflineLuaEnum.h"

CX2OfflinePetData* CX2OfflinePetData::ms_pInstance = NULL;

namespace
{
	const wchar_t* const SCRIPT_PET			= L"PetData.lua";
	const wchar_t* const SCRIPT_RIDING_PET	= L"RidingPetData.lua";
}

//////////////////////////////////////////////////////////////////////////

CX2OfflinePetData::CX2OfflinePetData()
: m_bLoadAttempted( false )
, m_iPetRows( 0 )
, m_iRidingPetRows( 0 )
, m_iPetSkipped( 0 )
, m_iRidingPetSkipped( 0 )
, m_dIncreaseSpecialFeedIntimacyRate( 0.0 )
, m_dDecreaseSpecialFeedIntimacyRate( 0.0 )
{
}

CX2OfflinePetData::~CX2OfflinePetData()
{
}

CX2OfflinePetData* CX2OfflinePetData::Instance()
{
	if( NULL == ms_pInstance )
		ms_pInstance = new CX2OfflinePetData;

	return ms_pInstance;
}

void CX2OfflinePetData::Release()
{
	if( NULL == ms_pInstance )
		return;

	delete ms_pInstance;
	ms_pInstance = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Loading

bool CX2OfflinePetData::RunScript( const wchar_t* szName, int& iRowCounter )
{
	// The identical two-layer shape X2OfflineDropTable::RunScript uses: the
	// .kom container comes off in LoadDataFile, the XOR comes off in DoMemory,
	// and DoMemoryNotEncript is the fallback for a file that was packed
	// plaintext. "Did it work" is measured by rows appearing, not by the return
	// code - a chunk that runs and adds nothing is a failure that reports
	// success.
	KGCMassFileManager::CMassFile::MASSFILE_MEMBERFILEINFO_POINTER kInfo;
	kInfo = g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LoadDataFile( szName );

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
			L"PET      ERROR '%s' not found in any .kom or on disk.", szName );
		CX2OfflineLog::Server(
			L"PET      XOR-encrypt KncWX2Server/ServerResource/US/%s and pack it into data036.kom.",
			szName );
		return false;
	}

	const int iBefore = iRowCounter;

	bool bRan	= ( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemory( kInfo->pRealData, kInfo->size ) );
	bool bAdded	= ( iRowCounter > iBefore );

	if( false == bRan || false == bAdded )
	{
		bRan	= ( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemoryNotEncript( kInfo->pRealData, kInfo->size ) );
		bAdded	= ( iRowCounter > iBefore );

		if( true == bRan && true == bAdded )
		{
			CX2OfflineLog::Server(
				L"PET      NOTE '%s' is NOT encrypted - loaded as plaintext. Fine for testing;"
				L" encrypt it to match every other packed script.", szName );
		}
	}

	if( false == bRan || false == bAdded )
	{
		CX2OfflineLog::Server(
			L"PET      ERROR '%s' produced no rows, encrypted or plaintext.", szName );
		return false;
	}

	return true;
}

void CX2OfflinePetData::EnsureLoaded()
{
	if( true == m_bLoadAttempted )
		return;

	m_bLoadAttempted = true;

	if( NULL == g_pKTDXApp ||
		NULL == g_pKTDXApp->GetLuaBinder() ||
		NULL == g_pKTDXApp->GetDeviceManager() ||
		NULL == g_pKTDXApp->GetDeviceManager()->GetMassFileManager() )
	{
		CX2OfflineLog::Server( L"PET      ERROR engine not ready - hatching and mounts stay off." );
		return;
	}

	lua_State* pLuaState = g_pKTDXApp->GetLuaBinder()->GetLuaState();
	if( NULL == pLuaState )
	{
		CX2OfflineLog::Server( L"PET      ERROR no lua state - hatching and mounts stay off." );
		return;
	}

	// Both files subscript enum tables by name. Publishing them is not
	// optional: without it every PET_UNIT_ID[...] is nil, every row loads as
	// pet 0, and the files would appear to load perfectly while mapping every
	// egg to nothing. So a failure here aborts the load rather than degrading
	// it.
	if( false == CX2OfflineLuaEnum::Publish() )
	{
		CX2OfflineLog::Server( L"PET      ERROR enum tables could not be published - not"
			L" loading PetData.lua or RidingPetData.lua, because every row would map to pet 0." );
		return;
	}

	lua_tinker::class_add< CX2OfflinePetData >( pLuaState, "CX2OfflinePetData" );

	lua_tinker::class_def< CX2OfflinePetData >( pLuaState, "SetNewPetInfo",
		&CX2OfflinePetData::SetNewPetInfo_LUA );
	lua_tinker::class_def< CX2OfflinePetData >( pLuaState, "AddPetCreateItemInfo",
		&CX2OfflinePetData::AddPetCreateItemInfo_LUA );
	lua_tinker::class_def< CX2OfflinePetData >( pLuaState, "AddRandomPetCreateItemInfo",
		&CX2OfflinePetData::AddRandomPetCreateItemInfo_LUA );
	lua_tinker::class_def< CX2OfflinePetData >( pLuaState, "AddPetCashFeedItemInfo",
		&CX2OfflinePetData::AddPetCashFeedItemInfo_LUA );
	lua_tinker::class_def< CX2OfflinePetData >( pLuaState, "AddSpecialFeedItemID",
		&CX2OfflinePetData::AddSpecialFeedItemID_LUA );
	lua_tinker::class_def< CX2OfflinePetData >( pLuaState, "AddPetSatietyDecreaseFactor",
		&CX2OfflinePetData::AddPetSatietyDecreaseFactor_LUA );
	lua_tinker::class_def< CX2OfflinePetData >( pLuaState, "AddTransformPetItemInfo",
		&CX2OfflinePetData::AddTransformPetItemInfo_LUA );
	lua_tinker::class_def< CX2OfflinePetData >( pLuaState, "IncreaseSpecialFeedIntimacyRate",
		&CX2OfflinePetData::IncreaseSpecialFeedIntimacyRate_LUA );
	lua_tinker::class_def< CX2OfflinePetData >( pLuaState, "DecreaseSpecialFeedIntimacyRate",
		&CX2OfflinePetData::DecreaseSpecialFeedIntimacyRate_LUA );
	lua_tinker::class_def< CX2OfflinePetData >( pLuaState, "AddRidingPetCreateItemInfo",
		&CX2OfflinePetData::AddRidingPetCreateItemInfo_LUA );
	lua_tinker::class_def< CX2OfflinePetData >( pLuaState, "SetEnableStamina",
		&CX2OfflinePetData::SetEnableStamina_LUA );

	// Both globals point at the same object. The two files never call into each
	// other's half, so one class serving both names costs nothing and keeps the
	// item -> unit lookups in one place.
	lua_tinker::decl( pLuaState, "g_pPetManager", this );
	lua_tinker::decl( pLuaState, "g_pRidingPetManager", this );

	RunScript( SCRIPT_PET,			m_iPetRows );
	RunScript( SCRIPT_RIDING_PET,	m_iRidingPetRows );

	CX2OfflineLog::Server( L"PET      loaded: %d egg(s) -> pet, %d stone(s) -> mount,"
		L" %u random egg(s)", m_iPetRows, m_iRidingPetRows,
		(unsigned int)m_mapRandomPetItem.size() );

	// The skip counts are the interesting number, not the load count: they are
	// the gap between what the shipped .lua describes and what this client can
	// render, and they are why some perfectly ordinary shop item will refuse to
	// open.
	if( m_iPetSkipped > 0 || m_iRidingPetSkipped > 0 )
	{
		CX2OfflineLog::Server( L"PET      NOTE %d egg row(s) and %d stone row(s) name a pet or"
			L" mount this client build has no enum for - the .lua files shipped in"
			L" ServerResource are newer than this source tree. Those items will refuse to"
			L" open and stay in the bag.", m_iPetSkipped, m_iRidingPetSkipped );
	}

	if( 0 == m_iPetRows )
	{
		CX2OfflineLog::Server( L"PET      WARNING no egg -> pet rows. Hatching is off until"
			L" PetData.lua is packed into data036.kom." );
	}

	if( 0 == m_iRidingPetRows )
	{
		CX2OfflineLog::Server( L"PET      WARNING no stone -> mount rows. Mounts are off until"
			L" RidingPetData.lua is packed into data036.kom." );
	}
}

//////////////////////////////////////////////////////////////////////////
// Bound into Lua as g_pPetManager:* / g_pRidingPetManager:*

void CX2OfflinePetData::SetNewPetInfo_LUA( int iEvolutionStep, int iSatiety, int iIntimacy,
										   int iExtroversion, int iEmotion )
{
	m_kNewPet.m_iEvolutionStep	= iEvolutionStep;
	m_kNewPet.m_iSatiety		= iSatiety;
	m_kNewPet.m_iIntimacy		= iIntimacy;
	m_kNewPet.m_iExtroversion	= iExtroversion;
	m_kNewPet.m_iEmotion		= iEmotion;
}

void CX2OfflinePetData::AddPetCreateItemInfo_LUA( int iItemID, int iPetID, int iPeriodDays )
{
	if( iItemID <= 0 )
		return;

	// iPetID 0 is PUI_NONE, which here means the PET_UNIT_ID[...] subscript
	// found no such name - the file is newer than this build's enum. Counted
	// and dropped; GetPetByItem will then refuse the egg with the item ID in
	// the log, which is far more useful than a pet the client cannot draw.
	if( iPetID <= 0 )
	{
		++m_iPetSkipped;
		return;
	}

	KCreateItem kItem;
	kItem.m_iUnitID		= iPetID;
	kItem.m_iPeriodDays	= iPeriodDays;

	m_mapPetItem[ iItemID ] = kItem;
	++m_iPetRows;
}

void CX2OfflinePetData::AddRandomPetCreateItemInfo_LUA( int iItemID, int iPetID, float fRate )
{
	if( iItemID <= 0 || fRate <= 0.0f )
		return;

	if( iPetID <= 0 )
	{
		++m_iPetSkipped;
		return;
	}

	KRandomCase kCase;
	kCase.m_iPetID	= iPetID;
	kCase.m_fRate	= fRate;

	m_mapRandomPetItem[ iItemID ].push_back( kCase );
	++m_iPetRows;
}

void CX2OfflinePetData::AddPetCashFeedItemInfo_LUA( int iItemID, int iIncreaseSatiety )
{
	if( iItemID > 0 )
		m_mapFeedItem[ iItemID ] = iIncreaseSatiety;
}

void CX2OfflinePetData::AddSpecialFeedItemID_LUA( int iItemID )
{
	if( iItemID > 0 )
		m_setSpecialFeedItem.insert( iItemID );
}

void CX2OfflinePetData::AddPetSatietyDecreaseFactor_LUA( int iEvolutionStep, double fFactor )
{
	m_mapSatietyDecreaseFactor[ iEvolutionStep ] = fFactor;
}

void CX2OfflinePetData::AddTransformPetItemInfo_LUA( int /*iTransformID*/, int /*iPetCreateItemID*/,
													 int /*iPetID*/, float /*fRate*/ )
{
}

void CX2OfflinePetData::IncreaseSpecialFeedIntimacyRate_LUA( float fRate )
{
	m_dIncreaseSpecialFeedIntimacyRate = (double)fRate;
}

void CX2OfflinePetData::DecreaseSpecialFeedIntimacyRate_LUA( float fRate )
{
	m_dDecreaseSpecialFeedIntimacyRate = (double)fRate;
}

void CX2OfflinePetData::AddRidingPetCreateItemInfo_LUA( int iItemID, int iRidingPetID, int iPeriodDays )
{
	if( iItemID <= 0 )
		return;

	// The one that actually bites: 7 of the 11 mounts RidingPetData.lua names
	// have no RIDING_PET_UNIT_ID in this build, three of them sold in the cash
	// shop. See the header.
	if( iRidingPetID <= 0 )
	{
		++m_iRidingPetSkipped;
		return;
	}

	KCreateItem kItem;
	kItem.m_iUnitID		= iRidingPetID;
	kItem.m_iPeriodDays	= iPeriodDays;

	m_mapRidingPetItem[ iItemID ] = kItem;
	++m_iRidingPetRows;
}

void CX2OfflinePetData::SetEnableStamina_LUA( float /*fStamina*/ )
{
}

//////////////////////////////////////////////////////////////////////////
// Reads

bool CX2OfflinePetData::GetPetByItem( int iItemID, OUT int& iPetID, OUT int& iPeriodDays )
{
	EnsureLoaded();

	iPetID		= 0;
	iPeriodDays	= -1;

	std::map< int, KCreateItem >::const_iterator mit = m_mapPetItem.find( iItemID );
	if( m_mapPetItem.end() != mit )
	{
		iPetID		= mit->second.m_iUnitID;
		iPeriodDays	= mit->second.m_iPeriodDays;
		return true;
	}

	// Not a fixed egg - try the random table, which is the same fall-through
	// CXSLPetManager::GetPetIDByItemID makes under
	// SERV_HALLOWEEN_PUMPKIN_FAIRY_PET (on in this build).
	std::map< int, std::vector< KRandomCase > >::const_iterator mitRandom
		= m_mapRandomPetItem.find( iItemID );

	if( m_mapRandomPetItem.end() == mitRandom || true == mitRandom->second.empty() )
		return false;

	const std::vector< KRandomCase >& vecCase = mitRandom->second;

	// KLottery::Decision: one roll in [0,100), accumulate in order, first case
	// whose running total reaches the roll wins.
	const float fRoll = ( (float)( rand() % 10000 ) ) / 100.0f;

	float fRunning = 0.0f;
	for( size_t i = 0; i < vecCase.size(); ++i )
	{
		fRunning += vecCase[i].m_fRate;

		if( fRoll < fRunning )
		{
			iPetID		= vecCase[i].m_iPetID;
			iPeriodDays	= -1;
			return true;
		}
	}

	// The rates did not sum to 100. The live lottery would return CASE_BLANK
	// here and the hatch would fail; taking the last case instead would quietly
	// bias the table. Fail, and let the caller keep the egg.
	return false;
}

bool CX2OfflinePetData::GetRidingPetByItem( int iItemID, OUT int& iRidingPetID, OUT int& iPeriodDays )
{
	EnsureLoaded();

	iRidingPetID	= 0;
	iPeriodDays		= -1;

	std::map< int, KCreateItem >::const_iterator mit = m_mapRidingPetItem.find( iItemID );
	if( m_mapRidingPetItem.end() == mit )
		return false;

	iRidingPetID	= mit->second.m_iUnitID;
	iPeriodDays		= mit->second.m_iPeriodDays;

	return true;
}

const CX2OfflinePetData::KNewPetInfo& CX2OfflinePetData::GetNewPetInfo()
{
	EnsureLoaded();
	return m_kNewPet;
}

bool CX2OfflinePetData::HasPetRows()
{
	EnsureLoaded();
	return ( m_iPetRows > 0 );
}

bool CX2OfflinePetData::HasRidingPetRows()
{
	EnsureLoaded();
	return ( m_iRidingPetRows > 0 );
}

bool CX2OfflinePetData::GetFeedSatietyGain( int iItemID, OUT int& iSatietyGain )
{
	EnsureLoaded();

	iSatietyGain = 0;

	std::map< int, int >::const_iterator mit = m_mapFeedItem.find( iItemID );
	if( m_mapFeedItem.end() == mit )
		return false;

	iSatietyGain = mit->second;
	return true;
}

bool CX2OfflinePetData::IsSpecialFeedItem( int iItemID ) const
{
	return ( m_setSpecialFeedItem.end() != m_setSpecialFeedItem.find( iItemID ) );
}

bool CX2OfflinePetData::GetSatietyDecreaseFactor( int iEvolutionStep, OUT double& fFactor ) const
{
	fFactor = 0.0;

	std::map< int, double >::const_iterator mit = m_mapSatietyDecreaseFactor.find( iEvolutionStep );
	if( m_mapSatietyDecreaseFactor.end() == mit )
		return false;

	fFactor = mit->second;
	return true;
}

#endif SERV_IRUHADEV_OFFLINE
