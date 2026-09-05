#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-04
// Description: Offline mode - the enum name tables the studio's server-side
//              .lua files index by name (phase 7b).
//
//              Three of the four files this phase needs to load are written
//              against globals that only ever existed inside a GameServer's Lua
//              state:
//
//                RandomItemTable.lua   UNIT_CLASS[...], USE_CONDITION[...]
//                PetData.lua           PET_UNIT_ID[...]
//                RidingPetData.lua     RIDING_PET_UNIT_ID[...]
//                ResolveTable.lua      ITEM_TYPE[...], ITEM_GRADE[...] (phase 12)
//                MapData.lua           VILLAGE_MAP_ID[...], TRUE (Cobo Express fix)
//
//              Nothing in the client defines them. Without them every
//              subscript is nil, every value arrives as 0, and the files load
//              into a table of zeroes that looks fine and is entirely wrong -
//              the worst failure shape there is. So they get published into the
//              client's own Lua state before those files run.
//
//              This is NOT inventing server data. The values are the client's
//              own enums, read straight out of X2Unit.h / X2PetManager.h /
//              X2RidingPetManager.h by a generator, and then diffed against the
//              server's independent copy of each enum - see the header comment
//              in X2OfflineLuaEnumSeed.h for the diff and its one exception.
//              A name this build does not have is deliberately left undefined
//              rather than guessed, so it surfaces as a skipped row and a log
//              line instead of a wrong ID.
//
//              Published once per process, lazily: both loaders call Publish()
//              and the second call is a no-op. It has to be lazy for the same
//              reason the loaders are - the offline server is constructed
//              before the Lua state is usable.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

class CX2OfflineLuaEnum
{
public:
	/// One `NAME = value` pair inside one Lua table. Public because
	/// X2OfflineLuaEnumSeed.h builds arrays of it.
	struct KEntry
	{
		const char*	m_szName;
		int			m_iValue;
	};

public:
	/// Defines UNIT_CLASS, USE_CONDITION, PET_UNIT_ID, RIDING_PET_UNIT_ID,
	/// ITEM_TYPE and ITEM_GRADE as globals in the client's Lua state. Safe to
	/// call repeatedly; only the first call does anything.
	///
	/// Returns false when the Lua state is not up yet, which the caller should
	/// treat as "do not run the scripts" - running them without the tables is
	/// strictly worse than not running them at all.
	static bool	Publish();

private:
	static void	PublishTable( struct lua_State* pLuaState, const char* szGlobal,
							  const KEntry* pEntry, int iCount );

	static bool	ms_bPublished;
};

#endif SERV_IRUHADEV_OFFLINE
