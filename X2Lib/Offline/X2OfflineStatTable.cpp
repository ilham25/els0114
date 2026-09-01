#include "stdafx.h"
#include "X2OfflineStatTable.h"

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineLog.h"

CX2OfflineStatTable* CX2OfflineStatTable::ms_pInstance = NULL;

namespace
{
	/// Elsword/Swordman's own curve, read off StatTable.lua's class 1 rows
	/// (level 1 and level 17 give an exact linear fit). Used only when
	/// StatTable.lua cannot be loaded at all, so that a character is playable
	/// with approximate numbers instead of unplayable with zeroes.
	const double SYN_HP_L1			= 11250.0;
	const double SYN_HP_PER_LEVEL	= 1687.5;
	const double SYN_ATKP_L1		= 360.0;
	const double SYN_ATKP_PER_LEVEL	= 10.125;
	const double SYN_ATKM_L1		= 296.0;
	const double SYN_ATKM_PER_LEVEL	= 8.3125;
	const double SYN_DEFP_L1		= 80.0;
	const double SYN_DEFP_PER_LEVEL	= 2.25;
	const double SYN_DEFM_L1		= 66.0;
	const double SYN_DEFM_PER_LEVEL	= 1.875;

	const wchar_t* const SCRIPT_NAME = L"StatTable.lua";
}

//////////////////////////////////////////////////////////////////////////

CX2OfflineStatTable::CX2OfflineStatTable()
: m_bLoadAttempted( false )
, m_iRowsLoaded( 0 )
{
}

CX2OfflineStatTable::~CX2OfflineStatTable()
{
}

CX2OfflineStatTable* CX2OfflineStatTable::Instance()
{
	if( NULL == ms_pInstance )
		ms_pInstance = new CX2OfflineStatTable;

	return ms_pInstance;
}

void CX2OfflineStatTable::Release()
{
	if( NULL == ms_pInstance )
		return;

	delete ms_pInstance;
	ms_pInstance = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Loading

void CX2OfflineStatTable::EnsureLoaded()
{
	if( true == m_bLoadAttempted )
		return;

	// One attempt per process either way: a second try would not find the file
	// any more readily, and GetUnitStat is called often enough that retrying
	// would mean re-running the chunk on every character.
	m_bLoadAttempted = true;

	if( NULL == g_pKTDXApp ||
		NULL == g_pKTDXApp->GetLuaBinder() ||
		NULL == g_pKTDXApp->GetDeviceManager() ||
		NULL == g_pKTDXApp->GetDeviceManager()->GetMassFileManager() )
	{
		CX2OfflineLog::Server( L"STAT     ERROR engine not ready - falling back to a synthetic stat curve." );
		return;
	}

	lua_State* pLuaState = g_pKTDXApp->GetLuaBinder()->GetLuaState();
	if( NULL == pLuaState )
	{
		CX2OfflineLog::Server( L"STAT     ERROR no lua state - falling back to a synthetic stat curve." );
		return;
	}

	// Bind this object as the global the script calls into, exactly as
	// KStatTable::RegisterLuaBind does on the server side
	// (KncWX2Server/GameServer/StatTable.cpp:131-136). ReserveMemory has to be
	// bound too, or the chunk errors on its first line.
	lua_tinker::class_add< CX2OfflineStatTable >( pLuaState, "CX2OfflineStatTable" );
	lua_tinker::class_def< CX2OfflineStatTable >( pLuaState, "SetUnitStat",   &CX2OfflineStatTable::SetUnitStat_LUA );
	lua_tinker::class_def< CX2OfflineStatTable >( pLuaState, "ReserveMemory", &CX2OfflineStatTable::ReserveMemory_LUA );
	lua_tinker::decl( pLuaState, "StatTable", this );

	// Archive first, loose file second - MASS_FILE_FIRST is defined for
	// _SERVICE_ (KTDX.h:92), so this resolves a packed StatTable.lua out of any
	// mounted .kom and only then looks on disk. It unwraps the archive container
	// only; the script's own XOR encryption is undone by DoMemory below.
	KGCMassFileManager::CMassFile::MASSFILE_MEMBERFILEINFO_POINTER kInfo;
	kInfo = g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LoadDataFile( SCRIPT_NAME );

	if( NULL == kInfo )
	{
		CX2OfflineLog::Server(
			L"STAT     ERROR '%s' not found in any .kom or on disk - falling back to a synthetic stat curve.",
			SCRIPT_NAME );
		CX2OfflineLog::Server(
			L"STAT     Pack KncWX2Server/ServerResource/US/StatTable.lua into a .kom, or drop it in the game data folder." );
		return;
	}

	// Every shipped script is XOR-encrypted, and KLuabinder::DoMemory runs
	// XORDecrypt *unconditionally* under _ENCRIPT_SCRIPT_ (KLuabinder.h:25-27) -
	// it does not sniff the content. The encryption is a separate step applied to
	// the .lua before packing; X2MassFileTool does not do it. So DoMemory is the
	// right call for a properly prepared file, packed or loose.
	//
	// A *plaintext* copy would be XOR'd into garbage by that, which is exactly
	// how a loose unencrypted file dropped in for testing behaves. So fall back
	// to DoMemoryNotEncript. Encrypted is tried first because that is the shipped
	// form; a chunk that fails to load has executed nothing, so the Lua state is
	// clean for the retry.
	//
	// Lua's own loader takes source or precompiled bytecode, so neither path
	// cares whether the file went through luac.
	bool bRan = ( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemory( kInfo->pRealData, kInfo->size ) );

	if( false == bRan || 0 == m_iRowsLoaded )
	{
		m_mapStat.clear();
		m_iRowsLoaded = 0;

		bRan = ( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemoryNotEncript( kInfo->pRealData, kInfo->size ) );

		if( true == bRan && m_iRowsLoaded > 0 )
		{
			CX2OfflineLog::Server(
				L"STAT     NOTE '%s' is NOT encrypted - loaded as plaintext. Fine for testing;"
				L" encrypt it (XOREncrypt / FileEncrypt) to match every other packed script.",
				SCRIPT_NAME );
		}
	}

	if( false == bRan )
	{
		CX2OfflineLog::Server(
			L"STAT     ERROR '%s' failed to run, encrypted or plaintext"
			L" - falling back to a synthetic stat curve.", SCRIPT_NAME );
		return;
	}

	if( 0 == m_iRowsLoaded )
	{
		CX2OfflineLog::Server(
			L"STAT     ERROR '%s' ran but produced no rows - falling back to a synthetic stat curve.", SCRIPT_NAME );
		return;
	}

	CX2OfflineLog::Server( L"STAT     '%s' loaded: %u class(es), %d row(s)",
		SCRIPT_NAME, (unsigned int)m_mapStat.size(), m_iRowsLoaded );
}

//////////////////////////////////////////////////////////////////////////
// Bound into Lua as StatTable:*

void CX2OfflineStatTable::ReserveMemory_LUA( int /*iUnitClass*/, int /*iMaxLevel*/ )
{
	// The server pre-sizes a std::vector per class here so that SetUnitStat can
	// index into it. This side keeps a map, so there is nothing to reserve - but
	// the method has to exist, because every class in the file calls it before
	// any SetUnitStat row appears.
}

void CX2OfflineStatTable::SetUnitStat_LUA( int iUnitClass, int iLevel )
{
	if( iUnitClass <= 0 || iLevel <= 0 || iLevel > (int)MAX_LEVEL )
	{
		// Classes 110 and 111 carry rows up to level 99 in the US file. The
		// server drops those too (its vector is sized maxLevel + 1), so
		// ignoring them keeps the two sides agreeing.
		return;
	}

	// The third argument - the { AtkPhysic = ..., HP = ... } table - is not a
	// declared parameter. It is still on the Lua stack, and KLuaManager's
	// default table depth of 1 reads fields out of it, which is exactly how
	// KStatTable::SetUnitStat and CX2UnitManager::AddUnitTemplet_LUA both work.
	KLuaManager luaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState() );

	// Read into the same widths the server uses, so a fractional HP such as
	// 12937.5 truncates identically - rounding would give an offline character
	// one more hit point than the same character online.
	UINT	uiHP		= 0;
	USHORT	usAtkPhysic	= 0;
	USHORT	usAtkMagic	= 0;
	USHORT	usDefPhysic	= 0;
	USHORT	usDefMagic	= 0;

	LUA_GET_VALUE( luaManager, "HP",			uiHP,			0 );
	LUA_GET_VALUE( luaManager, "AtkPhysic",	usAtkPhysic,	0 );
	LUA_GET_VALUE( luaManager, "AtkMagic",	usAtkMagic,		0 );
	LUA_GET_VALUE( luaManager, "DefPhysic",	usDefPhysic,	0 );
	LUA_GET_VALUE( luaManager, "DefMagic",	usDefMagic,		0 );

	if( 0 == uiHP )
		return;

	KStat kStat;
	kStat.m_iBaseHP		= (int)uiHP;
	kStat.m_iAtkPhysic	= (int)usAtkPhysic;
	kStat.m_iAtkMagic	= (int)usAtkMagic;
	kStat.m_iDefPhysic	= (int)usDefPhysic;
	kStat.m_iDefMagic	= (int)usDefMagic;

	m_mapStat[ iUnitClass ][ iLevel ] = kStat;
	++m_iRowsLoaded;
}

//////////////////////////////////////////////////////////////////////////

/*static*/ int CX2OfflineStatTable::UnitTypeOf( int iUnitClass )
{
	if( NULL != g_pData && NULL != g_pData->GetUnitManager() )
	{
		const CX2Unit::UnitTemplet* pTemplet =
			g_pData->GetUnitManager()->GetUnitTemplet( (CX2Unit::UNIT_CLASS)iUnitClass );

		if( NULL != pTemplet && CX2Unit::UT_NONE != pTemplet->m_UnitType )
			return (int)pTemplet->m_UnitType;
	}

	// Identity for the eight base classes; anything else is unknown.
	if( iUnitClass > 0 && iUnitClass < (int)CX2Unit::UT_END )
		return iUnitClass;

	return (int)CX2Unit::UT_NONE;
}

/*static*/ char CX2OfflineStatTable::CharAbilTypeOf( int iUnitClass )
{
	// Mirrors KGSGameDBThread's switch (GSGameDBThread.cpp:792-809) exactly,
	// including the fact that SERV_NEW_CHARACTER_EL is *off* in this build
	// (ServerDefine.h:3788 has it commented out) - so Elesis gets CAC_NONE
	// here just as she does against the live server, rather than CAC_WSP.
	switch( UnitTypeOf( iUnitClass ) )
	{
	case CX2Unit::UT_ELSWORD:
#ifdef SERV_NEW_CHARACTER_EL
	case CX2Unit::UT_ELESIS:
#endif // SERV_NEW_CHARACTER_EL
		return (char)KGamePlayStatus::CAC_WSP;

	case CX2Unit::UT_CHUNG:
		return (char)KGamePlayStatus::CAC_CANNON_BALL_COUNT;

	case CX2Unit::UT_ARA:
		return (char)KGamePlayStatus::CAC_FORCE_POWER;

	default:
		break;
	}

	return (char)KGamePlayStatus::CAC_NONE;
}

/*static*/ void CX2OfflineStatTable::SyntheticStat( int /*iUnitClass*/, int iLevel, OUT KStat& kOut )
{
	const double fSteps = (double)( iLevel - 1 );

	kOut.m_iBaseHP		= (int)( SYN_HP_L1   + SYN_HP_PER_LEVEL   * fSteps );
	kOut.m_iAtkPhysic	= (int)( SYN_ATKP_L1 + SYN_ATKP_PER_LEVEL * fSteps );
	kOut.m_iAtkMagic	= (int)( SYN_ATKM_L1 + SYN_ATKM_PER_LEVEL * fSteps );
	kOut.m_iDefPhysic	= (int)( SYN_DEFP_L1 + SYN_DEFP_PER_LEVEL * fSteps );
	kOut.m_iDefMagic	= (int)( SYN_DEFM_L1 + SYN_DEFM_PER_LEVEL * fSteps );
}

void CX2OfflineStatTable::GetUnitStat( int iUnitClass, int iLevel, OUT KStat& kOut )
{
	EnsureLoaded();

	kOut.Init();

	if( iLevel < 1 )
		iLevel = 1;
	if( iLevel > (int)MAX_LEVEL )
		iLevel = (int)MAX_LEVEL;

	if( true == m_mapStat.empty() )
	{
		SyntheticStat( iUnitClass, iLevel, kOut );
		return;
	}

	std::map< int, std::map< int, KStat > >::const_iterator mitClass = m_mapStat.find( iUnitClass );

	// An unknown class falls back to the character's base class, which is what
	// the base-class rows of the table are for. (Every class the client can
	// actually create is in the file; this covers a job class added later
	// against an older StatTable.lua.)
	if( mitClass == m_mapStat.end() )
	{
		const int iBaseClass = UnitTypeOf( iUnitClass );
		if( iBaseClass != iUnitClass )
			mitClass = m_mapStat.find( iBaseClass );
	}

	if( mitClass == m_mapStat.end() )
	{
		CX2OfflineLog::Server( L"STAT     WARNING no rows for class=%d - using the synthetic curve",
			iUnitClass );

		SyntheticStat( iUnitClass, iLevel, kOut );
		return;
	}

	const std::map< int, KStat >& mapLevel = mitClass->second;

	std::map< int, KStat >::const_iterator mitLevel = mapLevel.find( iLevel );

	// Clamp to the nearest level the file actually carries rather than handing
	// back a zeroed stat, which would mean max HP 0.
	if( mitLevel == mapLevel.end() )
	{
		mitLevel = mapLevel.lower_bound( iLevel );
		if( mitLevel == mapLevel.end() && false == mapLevel.empty() )
			--mitLevel;
	}

	if( mitLevel == mapLevel.end() )
	{
		SyntheticStat( iUnitClass, iLevel, kOut );
		return;
	}

	kOut = mitLevel->second;
}

#endif SERV_IRUHADEV_OFFLINE
