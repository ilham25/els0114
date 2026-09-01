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
//              GameServer loads (KncWX2Server/ServerResource/US/StatTable.lua)
//              - through the client's own mass-file loader, so the file can
//              live inside a .kom archive exactly like every other script:
//
//                  LoadDataFile( L"StatTable.lua" )   <- archive or loose file
//                  GetLuaBinder()->DoMemory( ... )    <- runs the chunk
//
//              That matters for two reasons. _ENCRIPT_SCRIPT_ is defined for
//              _SERVICE_ (KTDX.h:86), so every shipped script is XOR-encrypted
//              and cannot be read as text. The two layers come off separately:
//              LoadDataFile unwraps the .kom container, and DoMemory runs
//              XORDecrypt unconditionally (KLuabinder.h:25). Encryption is a
//              step applied to the .lua before packing - X2MassFileTool does
//              not do it - so a plaintext copy is handled by falling back to
//              DoMemoryNotEncript. And MASS_FILE_FIRST is defined (KTDX.h:92),
//              so LoadDataFile checks the mounted archives first and falls back
//              to a loose file on disk (KGCMassFileManager.cpp:665) - one code
//              path covers packed and unpacked. No luac step is needed either
//              way, since Lua's own loader takes source or bytecode.
//
//              The chunk calls StatTable:ReserveMemory(...) and
//              StatTable:SetUnitStat( class, level, { ... } ) on a global, so
//              this object is bound into the Lua state under that name exactly
//              as KStatTable::RegisterLuaBind does it, and the trailing table
//              is read off the stack with KLuaManager - the same convention
//              CX2UnitManager::AddUnitTemplet_LUA uses.
//
//              Why any of this matters, and where it shows: without a real
//              base HP, CX2GUUser::InitStat -> SetGameStat leaves the village
//              unit at max HP 0, so the HUD health bar renders empty and the
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

	/// Base stat for a character. Never fails: the table is loaded on first use,
	/// an unknown class falls back to the character's base class, a level past
	/// the end of the table clamps to the last row, and a table that would not
	/// load at all falls back to a synthetic curve (see the .cpp - it is loudly
	/// logged, because the numbers are then wrong, just not zero).
	void	GetUnitStat( int iUnitClass, int iLevel, OUT KStat& kOut );

	bool	IsLoaded() const					{ return false == m_mapStat.empty(); }

	/// KGamePlayStatus::CHAR_ABIL_TYPE for a class, mirroring the real server's
	/// switch in KGSGameDBThread (GSGameDBThread.cpp:792-809), flags included.
	static char	CharAbilTypeOf( int iUnitClass );

	/// CX2Unit::UNIT_TYPE for a class, via the client's own unit manager. Falls
	/// back to the class number, which is the identity for the eight base
	/// classes (UC_ELSWORD_SWORDMAN == UT_ELSWORD == 1, and so on up to
	/// UC_ELESIS_KNIGHT == UT_ELESIS == 8).
	static int	UnitTypeOf( int iUnitClass );

	//////////////////////////////////////////////////////////////////////////
	// Bound into the Lua state as StatTable:*. Public because lua_tinker needs
	// to take their addresses; nothing else should call them.

	/// StatTable:SetUnitStat( class, level, { AtkPhysic = N, ..., HP = N } ).
	/// The trailing table is NOT a declared parameter - it is read off the Lua
	/// stack, which is the same shape KStatTable::SetUnitStat has.
	void	SetUnitStat_LUA( int iUnitClass, int iLevel );

	/// StatTable:ReserveMemory( class, maxLevel ). A no-op here; see the .cpp.
	void	ReserveMemory_LUA( int iUnitClass, int iMaxLevel );

private:
	CX2OfflineStatTable();
	~CX2OfflineStatTable();

	/// Loads on first use. It cannot run at construction time: the offline
	/// server is created around CX2Data::ResetServerProtocol, and the 145 .kom
	/// archives are not necessarily mounted yet - LoadDataFile would simply
	/// return NULL.
	void	EnsureLoaded();

	static void	SyntheticStat( int iUnitClass, int iLevel, OUT KStat& kOut );

private:
	static CX2OfflineStatTable*	ms_pInstance;

	/// class -> level -> stat. Sparse on purpose: the file has rows for classes
	/// 1-8, 10-31 and 100-119 only.
	std::map< int, std::map< int, KStat > >	m_mapStat;

	bool	m_bLoadAttempted;
	int		m_iRowsLoaded;
};

#endif SERV_IRUHADEV_OFFLINE
