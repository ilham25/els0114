#include "stdafx.h"
#include "X2OfflineStatTable.h"

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineLog.h"

CX2OfflineStatTable* CX2OfflineStatTable::ms_pInstance = NULL;

namespace
{
	/// Elsword/Swordman's own curve, read off StatTable.lua's class 1 rows
	/// (level 1 and level 17 give an exact linear fit). Used only when
	/// StatTable.lua is missing entirely, so that a character is playable with
	/// approximate numbers instead of unplayable with zeroes.
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
}

//////////////////////////////////////////////////////////////////////////

CX2OfflineStatTable::CX2OfflineStatTable()
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

/*static*/ double CX2OfflineStatTable::ReadKey( const char* szLine, const char* szKey, double fDefault )
{
	const char* p = strstr( szLine, szKey );
	if( NULL == p )
		return fDefault;

	p += strlen( szKey );

	while( ' ' == *p || '\t' == *p )
		++p;

	if( '=' != *p )
		return fDefault;

	++p;

	return atof( p );
}

bool CX2OfflineStatTable::ParseLine( const char* szLine )
{
	const char* p = strstr( szLine, "SetUnitStat" );
	if( NULL == p )
		return false;

	p = strchr( p, '(' );
	if( NULL == p )
		return false;

	int iUnitClass	= 0;
	int iLevel		= 0;

	if( 2 != sscanf( p + 1, " %d , %d", &iUnitClass, &iLevel ) )
		return false;

	if( iUnitClass <= 0 || iLevel <= 0 || iLevel > (int)MAX_LEVEL )
		return false;

	// The stat block is only the part inside the braces; keeping the search
	// anchored there stops a key from being picked up out of a trailing comment.
	const char* pBrace = strchr( p, '{' );
	if( NULL == pBrace )
		return false;

	KStat kStat;

	// The server reads these into UINT / USHORT via LUA_GET_VALUE, so a
	// fractional HP such as 12937.5 truncates. Truncate here too, or an offline
	// character has one more hit point than the same character online.
	kStat.m_iBaseHP		= (int)ReadKey( pBrace, "HP",			0.0 );
	kStat.m_iAtkPhysic	= (int)ReadKey( pBrace, "AtkPhysic",	0.0 );
	kStat.m_iAtkMagic	= (int)ReadKey( pBrace, "AtkMagic",		0.0 );
	kStat.m_iDefPhysic	= (int)ReadKey( pBrace, "DefPhysic",	0.0 );
	kStat.m_iDefMagic	= (int)ReadKey( pBrace, "DefMagic",		0.0 );

	if( 0 >= kStat.m_iBaseHP )
		return false;

	m_mapStat[ iUnitClass ][ iLevel ] = kStat;

	return true;
}

bool CX2OfflineStatTable::Load( const wchar_t* szPath )
{
	m_mapStat.clear();

	FILE* pFile = _wfopen( szPath, L"rb" );
	if( NULL == pFile )
	{
		CX2OfflineLog::Server(
			L"STAT     ERROR '%s' not found - falling back to a synthetic stat curve.",
			szPath );
		CX2OfflineLog::Server(
			L"STAT     Copy KncWX2Server/ServerResource/US/StatTable.lua into the game data folder." );
		return false;
	}

	char szLine[1024];
	int iRows = 0;

	while( NULL != fgets( szLine, (int)sizeof( szLine ), pFile ) )
	{
		if( true == ParseLine( szLine ) )
			++iRows;
	}

	fclose( pFile );

	if( 0 == iRows )
	{
		CX2OfflineLog::Server(
			L"STAT     ERROR '%s' parsed to zero rows - falling back to a synthetic stat curve.",
			szPath );
		return false;
	}

	CX2OfflineLog::Server( L"STAT     '%s' loaded: %u class(es), %d row(s)",
		szPath, (unsigned int)m_mapStat.size(), iRows );

	return true;
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

/*static*/ void CX2OfflineStatTable::SyntheticStat( int iUnitClass, int iLevel, OUT KStat& kOut )
{
	const double fSteps = (double)( iLevel - 1 );

	kOut.m_iBaseHP		= (int)( SYN_HP_L1   + SYN_HP_PER_LEVEL   * fSteps );
	kOut.m_iAtkPhysic	= (int)( SYN_ATKP_L1 + SYN_ATKP_PER_LEVEL * fSteps );
	kOut.m_iAtkMagic	= (int)( SYN_ATKM_L1 + SYN_ATKM_PER_LEVEL * fSteps );
	kOut.m_iDefPhysic	= (int)( SYN_DEFP_L1 + SYN_DEFP_PER_LEVEL * fSteps );
	kOut.m_iDefMagic	= (int)( SYN_DEFM_L1 + SYN_DEFM_PER_LEVEL * fSteps );
}

void CX2OfflineStatTable::GetUnitStat( int iUnitClass, int iLevel, OUT KStat& kOut ) const
{
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
	// the base-class row of the table is for. (Every class the client can
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

	// Clamp down to the highest level the file actually carries rather than
	// handing back a zeroed stat, which would mean max HP 0.
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
