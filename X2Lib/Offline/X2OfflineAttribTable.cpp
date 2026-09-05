#include "stdafx.h"
#include "X2OfflineAttribTable.h"

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineLog.h"
#include "X2OfflineLuaEnum.h"

CX2OfflineAttribTable* CX2OfflineAttribTable::ms_pInstance = NULL;

namespace
{
	const wchar_t* const SCRIPT_ATTRIB_ENCHANT	= L"AttribEnchantTable.lua";
	const wchar_t* const SCRIPT_ATTRIB_ATTACH	= L"AttribAttachTable.lua";
}

//////////////////////////////////////////////////////////////////////////

CX2OfflineAttribTable::CX2OfflineAttribTable()
: m_bLoadAttempted( false )
, m_bLoaded( false )
, m_iSingleRows( 0 )
, m_iDualRows( 0 )
, m_iTripleRows( 0 )
, m_iAttachRows( 0 )
, m_iIdentifyRows( 0 )
{
}

CX2OfflineAttribTable::~CX2OfflineAttribTable()
{
}

CX2OfflineAttribTable* CX2OfflineAttribTable::Instance()
{
	if( NULL == ms_pInstance )
		ms_pInstance = new CX2OfflineAttribTable;

	return ms_pInstance;
}

void CX2OfflineAttribTable::Release()
{
	if( NULL == ms_pInstance )
		return;

	delete ms_pInstance;
	ms_pInstance = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Loading

bool CX2OfflineAttribTable::RunScript( const wchar_t* szFileName )
{
	KGCMassFileManager::CMassFile::MASSFILE_MEMBERFILEINFO_POINTER kInfo;
	kInfo = g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LoadDataFile( szFileName );

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
			L"ATTRIB   ERROR '%s' not found in any .kom or on disk.", szFileName );
		CX2OfflineLog::Server(
			L"ATTRIB   XOR-encrypt KncWX2Server/ServerResource/US/%s and pack it into data036.kom.",
			szFileName );
		return false;
	}

	if( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemory( kInfo->pRealData, kInfo->size ) )
		return true;

	if( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemoryNotEncript( kInfo->pRealData, kInfo->size ) )
	{
		CX2OfflineLog::Server(
			L"ATTRIB   NOTE '%s' is NOT encrypted - loaded as plaintext. Fine for testing;"
			L" encrypt it to match every other packed script.", szFileName );
		return true;
	}

	CX2OfflineLog::Server( L"ATTRIB   ERROR '%s' failed to run, encrypted or plaintext.", szFileName );
	return false;
}

void CX2OfflineAttribTable::EnsureLoaded()
{
	if( true == m_bLoadAttempted )
		return;

	m_bLoadAttempted = true;

	if( NULL == g_pKTDXApp ||
		NULL == g_pKTDXApp->GetLuaBinder() ||
		NULL == g_pKTDXApp->GetDeviceManager() ||
		NULL == g_pKTDXApp->GetDeviceManager()->GetMassFileManager() )
	{
		CX2OfflineLog::Server( L"ATTRIB   ERROR engine not ready - attributes will refuse." );
		return;
	}

	lua_State* pLuaState = g_pKTDXApp->GetLuaBinder()->GetLuaState();
	if( NULL == pLuaState )
	{
		CX2OfflineLog::Server( L"ATTRIB   ERROR no lua state - attributes will refuse." );
		return;
	}

	// Both files subscript ENCHANT_TYPE["ET_BLAZE"] and friends.
	if( false == CX2OfflineLuaEnum::Publish() )
	{
		CX2OfflineLog::Server( L"ATTRIB   ERROR could not publish ENCHANT_TYPE - attributes will refuse." );
		return;
	}

	lua_tinker::class_add< CX2OfflineAttribTable >( pLuaState, "CX2OfflineAttribTable" );

	lua_tinker::class_def< CX2OfflineAttribTable >( pLuaState, "AddRandomAttribSingle",
		&CX2OfflineAttribTable::AddRandomAttribSingle_LUA );
	lua_tinker::class_def< CX2OfflineAttribTable >( pLuaState, "AddRandomAttribDual",
		&CX2OfflineAttribTable::AddRandomAttribDual_LUA );
	lua_tinker::class_def< CX2OfflineAttribTable >( pLuaState, "AddRandomAttribTriple",
		&CX2OfflineAttribTable::AddRandomAttribTriple_LUA );
	lua_tinker::class_def< CX2OfflineAttribTable >( pLuaState, "AddIdentifyInfo",
		&CX2OfflineAttribTable::AddIdentifyInfo_LUA );
	lua_tinker::class_def< CX2OfflineAttribTable >( pLuaState, "AddAttribAttachInfo",
		&CX2OfflineAttribTable::AddAttribAttachInfo_LUA );
	lua_tinker::class_def< CX2OfflineAttribTable >( pLuaState, "AddEnchantRequire_LUA",
		&CX2OfflineAttribTable::AddEnchantRequire_LUA );
	lua_tinker::class_def< CX2OfflineAttribTable >( pLuaState, "dump",
		&CX2OfflineAttribTable::Dump_LUA );

	// Borrow g_pCX2EnchantItem. See the header: the two packed files call
	// methods the client's own CX2EnchantItem does not bind, so the global has
	// to point here while they run - and has to point back afterwards, because
	// it is the client's object for everything else in the process.
	lua_tinker::decl( pLuaState, "g_pCX2EnchantItem", this );

	const bool bEnchantOK	= RunScript( SCRIPT_ATTRIB_ENCHANT );
	const bool bAttachOK	= RunScript( SCRIPT_ATTRIB_ATTACH );

	if( NULL != g_pData && NULL != g_pData->GetEnchantItem() )
	{
		lua_tinker::decl( pLuaState, "g_pCX2EnchantItem", g_pData->GetEnchantItem() );
	}
	else
	{
		// Nothing else in the process reads this global after start-up, but a
		// borrowed pointer left in place is exactly the kind of thing that is
		// invisible until something does. Say so rather than hope.
		CX2OfflineLog::Server( L"ATTRIB   WARNING could not restore g_pCX2EnchantItem -"
			L" g_pData->GetEnchantItem() was NULL." );
	}

	// The single lottery is the one that decides the whole feature: without it
	// an El shard [Unknown] cannot become anything. The attach map is separate
	// and its file is allowed to be missing on its own.
	if( false == bEnchantOK || m_iSingleRows <= 0 )
	{
		CX2OfflineLog::Server( L"ATTRIB   ERROR no random-attribute rows - attributes are OFF." );
		return;
	}

	m_bLoaded = true;

	CX2OfflineLog::Server( L"ATTRIB   loaded: %d single, %d dual, %d triple, %d amulet(s),"
		L" %d identify row(s)%s",
		m_iSingleRows, m_iDualRows, m_iTripleRows, m_iAttachRows, m_iIdentifyRows,
		( true == bAttachOK ) ? L"" : L" - AttribAttachTable.lua missing, amulets will refuse" );
}

//////////////////////////////////////////////////////////////////////////
// reads

bool CX2OfflineAttribTable::IsLoaded()
{
	EnsureLoaded();
	return m_bLoaded;
}

char CX2OfflineAttribTable::GetRandomAttribResult( int eCurrentCountType,
												   char cFirstAttribEnchant,
												   char cSecondAttribEnchant )
{
	EnsureLoaded();

	int iResult = (int)CX2EnchantItem::ET_NONE;

	switch( eCurrentCountType )
	{
	case CX2EnchantItem::ACT_NONE:
		iResult = LotteryDecideMulti( m_vecRandomSingle );
		break;

	case CX2EnchantItem::ACT_SINGLE:
		{
			std::map< char, KProbRow >::const_iterator mit =
				m_mapRandomDual.find( cFirstAttribEnchant );

			if( m_mapRandomDual.end() == mit )
			{
				CX2OfflineLog::Server( L"ATTRIB   no dual lottery for attribute %d",
					(int)cFirstAttribEnchant );
				return (char)CX2EnchantItem::ET_NONE;
			}

			iResult = LotteryDecideMulti( mit->second );
		}
		break;

	case CX2EnchantItem::ACT_DUAL:
		{
			std::map< KTripleKey, KProbRow >::const_iterator mit =
				m_mapRandomTriple.find( KTripleKey( cFirstAttribEnchant, cSecondAttribEnchant ) );

			if( m_mapRandomTriple.end() == mit )
			{
				CX2OfflineLog::Server( L"ATTRIB   no triple lottery for attributes %d + %d",
					(int)cFirstAttribEnchant, (int)cSecondAttribEnchant );
				return (char)CX2EnchantItem::ET_NONE;
			}

			iResult = LotteryDecideMulti( mit->second );
		}
		break;

	default:
		CX2OfflineLog::Server( L"ATTRIB   bad attribute count type %d", eCurrentCountType );
		return (char)CX2EnchantItem::ET_NONE;
	}

	// KLottery::CASE_BLANK, i.e. the weights did not sum to 100 and the roll
	// fell past the last case. The real manager logs and returns ET_NONE.
	if( iResult < 0 )
	{
		CX2OfflineLog::Server( L"ATTRIB   random attribute roll fell outside the table"
			L" (count type %d)", eCurrentCountType );
		return (char)CX2EnchantItem::ET_NONE;
	}

	return (char)iResult;
}

bool CX2OfflineAttribTable::IsExistTripleCase( char cFirstAttribEnchant, char cSecondAttribEnchant,
											   char cTypeToAdd )
{
	EnsureLoaded();

	std::map< KTripleKey, KProbRow >::const_iterator mit =
		m_mapRandomTriple.find( KTripleKey( cFirstAttribEnchant, cSecondAttribEnchant ) );

	if( m_mapRandomTriple.end() == mit )
		return false;

	for( size_t i = 0; i < mit->second.size(); ++i )
	{
		if( mit->second[i].first == (int)cTypeToAdd )
			return true;
	}

	return false;
}

bool CX2OfflineAttribTable::GetAttribAttachInfo( int iAttachItemID,
												 OUT char& cAttrib0, OUT char& cAttrib1,
												 OUT char& cAttrib2 )
{
	EnsureLoaded();

	std::map< int, KAttachRow >::const_iterator mit = m_mapAttribAttach.find( iAttachItemID );
	if( m_mapAttribAttach.end() == mit )
		return false;

	cAttrib0 = mit->second.m_cAttrib0;
	cAttrib1 = mit->second.m_cAttrib1;
	cAttrib2 = mit->second.m_cAttrib2;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Bound into Lua as g_pCX2EnchantItem:* while the two files run

void CX2OfflineAttribTable::AddRandomAttribSingle_LUA( char cEnchantType, float fRate )
{
	if( cEnchantType <= (char)CX2EnchantItem::ET_NONE )
		return;

	m_vecRandomSingle.push_back( std::make_pair( (int)cEnchantType, fRate ) );
	++m_iSingleRows;
}

void CX2OfflineAttribTable::AddRandomAttribDual_LUA( char cSingleEnchantType, char cDualEnchantType,
													 float fRate )
{
	if( cSingleEnchantType <= (char)CX2EnchantItem::ET_NONE ||
		cDualEnchantType <= (char)CX2EnchantItem::ET_NONE )
	{
		return;
	}

	m_mapRandomDual[ cSingleEnchantType ].push_back(
		std::make_pair( (int)cDualEnchantType, fRate ) );

	++m_iDualRows;
}

void CX2OfflineAttribTable::AddRandomAttribTriple_LUA( char cSingleEnchantType, char cDualEnchantType,
													   char cTripleEnchantType, float fRate )
{
	if( cSingleEnchantType <= (char)CX2EnchantItem::ET_NONE ||
		cDualEnchantType <= (char)CX2EnchantItem::ET_NONE ||
		cTripleEnchantType <= (char)CX2EnchantItem::ET_NONE )
	{
		return;
	}

	m_mapRandomTriple[ KTripleKey( cSingleEnchantType, cDualEnchantType ) ].push_back(
		std::make_pair( (int)cTripleEnchantType, fRate ) );

	++m_iTripleRows;
}

void CX2OfflineAttribTable::AddIdentifyInfo_LUA( int iSourceItemID, int iRequireItemID,
												 int iResultItemID, float fRate )
{
	if( iSourceItemID <= 0 || iResultItemID <= 0 )
		return;

	KIdentifyRow kRow;
	kRow.m_iRequireItemID	= iRequireItemID;
	kRow.m_iResultItemID	= iResultItemID;
	kRow.m_fRate			= fRate;

	m_mapIdentify[ iSourceItemID ].push_back( kRow );
	++m_iIdentifyRows;
}

void CX2OfflineAttribTable::AddAttribAttachInfo_LUA( int iItemID, char cSlot0, char cSlot1, char cSlot2 )
{
	if( iItemID <= 0 )
		return;

	KAttachRow kRow;
	kRow.m_cAttrib0 = cSlot0;
	kRow.m_cAttrib1 = cSlot1;
	kRow.m_cAttrib2 = cSlot2;

	// insert, not operator[] - the real manager keeps the first row for a
	// duplicated amulet ID.
	m_mapAttribAttach.insert( std::make_pair( iItemID, kRow ) );
	++m_iAttachRows;
}

void CX2OfflineAttribTable::AddEnchantRequire_LUA( int /*iAttribEnchantType*/, int /*iEquipLv*/ )
{
	// Deliberately empty. AttribEnchantRequire.lua is the client's own file and
	// the client has already parsed it into CX2EnchantItem; this exists only so
	// that a call arriving here cannot raise a Lua error.
}

void CX2OfflineAttribTable::Dump_LUA()
{
}

//////////////////////////////////////////////////////////////////////////

/*static*/ int CX2OfflineAttribTable::LotteryDecideMulti( const KProbRow& kRow )
{
	if( true == kRow.empty() )
		return -1;							///< KLottery::CASE_BLANK

	const float fRoll = (float)( rand() % 10000 ) * 0.01f;		///< 0.00 .. 99.99

	float fAccumulate = 0.0f;

	for( size_t i = 0; i < kRow.size(); ++i )
	{
		fAccumulate += kRow[i].second;

		if( fRoll <= fAccumulate )
			return kRow[i].first;
	}

	return -1;
}

#endif SERV_IRUHADEV_OFFLINE
