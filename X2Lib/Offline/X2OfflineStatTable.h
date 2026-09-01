#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-01
// Description: Offline mode - per-class, per-level base stats (phase 3).
//
//              This is the one piece of game data the client does NOT have.
//              KUnitInfo::m_kStat / m_kGameStat arrive from the server, which
//              reads them out of dbo.GUnit and then adds
//              KStatTable::GetUnitStat( class, level )
//              (KncWX2Server/GameServer/GSUserFunction.cpp:4490). The client's
//              own CX2Unit::UnitTemplet has an m_Stat field, but
//              CX2UnitManager::AddUnitTemplet_LUA never populates it - it is
//              dead on this side. So there is nothing to "reuse the client's
//              loaded managers" with here, and the offline server has to own
//              the table.
//
//              It reads the studio's own StatTable.lua - the exact file the
//              GameServer loads (KncWX2Server/ServerResource/US/StatTable.lua),
//              copied into the game data\ folder next to els_db.sql. The file
//              is a flat list of
//                  StatTable:SetUnitStat( <class>, <level>,
//                      { AtkPhysic = N, AtkMagic = N, DefPhysic = N,
//                        DefMagic = N, HP = N } )
//              so it is parsed as text rather than through a Lua state: it
//              keeps the client's Lua globals untouched and avoids depending on
//              the mass-file loader for a file that is not in any .kom.
//
//              Why this matters, and where it shows: without a real base HP,
//              CX2GUUser::InitStat -> SetGameStat leaves the village unit at
//              max HP 0, so the HUD health bar renders empty and the
//              zero-HP branch of Handler_EGS_GET_MY_INVENTORY_ACK would send a
//              battlefield character back to the village.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

class CX2OfflineStatTable
{
public:
	enum
	{
		/// Every class in StatTable.lua reserves 80 levels.
		MAX_LEVEL = 80,
	};

public:
	static CX2OfflineStatTable*	Instance();
	static void					Release();

	/// Reads szPath (relative to the working directory, i.e. the game data\
	/// folder). Logs what it found. Returns false if nothing was parsed - the
	/// synthetic fallback below then covers every lookup.
	bool	Load( const wchar_t* szPath );
	bool	IsLoaded() const					{ return false == m_mapStat.empty(); }

	/// Base stat for a character. Never fails: an unknown class falls back to
	/// the character's base class, a level past the end of the table clamps to
	/// the last row, and a table that would not load at all falls back to a
	/// synthetic curve (see the .cpp - it is loudly logged, because the numbers
	/// are then wrong, just not zero).
	void	GetUnitStat( int iUnitClass, int iLevel, OUT KStat& kOut ) const;

	/// KGamePlayStatus::CHAR_ABIL_TYPE for a class, mirroring the real server's
	/// switch in KGSGameDBThread (GSGameDBThread.cpp:792-809), flags included.
	static char	CharAbilTypeOf( int iUnitClass );

	/// CX2Unit::UNIT_TYPE for a class, via the client's own unit manager. Falls
	/// back to the class number, which is the identity for the eight base
	/// classes (UC_ELSWORD_SWORDMAN == UT_ELSWORD == 1, and so on up to
	/// UC_ELESIS_KNIGHT == UT_ELESIS == 8).
	static int	UnitTypeOf( int iUnitClass );

private:
	CX2OfflineStatTable();
	~CX2OfflineStatTable();

	/// One "SetUnitStat(" occurrence. Returns false when the line is not one.
	bool	ParseLine( const char* szLine );

	/// Value of `<szKey> = <number>` anywhere in szLine, or fDefault.
	static double	ReadKey( const char* szLine, const char* szKey, double fDefault );

	static void		SyntheticStat( int iUnitClass, int iLevel, OUT KStat& kOut );

private:
	static CX2OfflineStatTable*	ms_pInstance;

	/// class -> level -> stat. Sparse on purpose: the file has rows for
	/// classes 1-8, 10-31 and 100-119 only, and class 8 has rows without a
	/// matching StatTable:ReserveMemory call (a quirk of the studio's own file
	/// that makes the *server* drop them; nothing here depends on the reserve).
	std::map< int, std::map< int, KStat > >	m_mapStat;
};

#endif SERV_IRUHADEV_OFFLINE
