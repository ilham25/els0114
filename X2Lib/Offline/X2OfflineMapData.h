#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-05
// Description: Offline mode - Cobo Express warp-button pricing, correcting
//              phase 13.
//
//              Phase 13 fixed the trade square (EGS_SQUARE_LIST_REQ) and
//              believed that was the "You cannot enter the village" defect.
//              It was not: that dialog is what the client shows when
//              EGS_WARP_BY_BUTTON_ACK comes back with an error, and the
//              Cobo Express warp button (press B, pick a village, OK) had
//              been a deliberate refusal since Handlers_Social.cpp was
//              written - "the warp table is server data". That refusal was
//              itself only half right: `CXSLMapData::CheckCOBOExpressTicketMapID`
//              and `ComputeCOBOExpressTicketCost` (KncWX2Server/Common/X2Data/
//              XSLMapData.cpp:606-613) are not a data table at all, they are
//              two plain Lua FUNCTIONS - and the file that defines them,
//              KncWX2Server/ServerResource/US/MapData.lua, is already sitting
//              in this tree. There is nothing to invent: load the real file
//              and call the real functions, exactly the way
//              X2OfflineStatTable and X2OfflinePetData already load
//              StatTable.lua / PetData.lua.
//
//              MapData.lua indexes a `VILLAGE_MAP_ID` table and writes
//              `DEFAULT = TRUE` - both globals that exist only inside a
//              GameServer's Lua state. CX2OfflineLuaEnum now publishes
//              VILLAGE_MAP_ID (read out of the client's own
//              KncWX2Server/Common/Enum/Enum.h - a SHARED header, not two
//              independently maintained copies, so there is no client/server
//              drift to diff against here, unlike UNIT_CLASS/PET_UNIT_ID) and
//              the all-caps TRUE/FALSE this file needs alongside the
//              lowercase True/False RandomItemTable.lua already uses.
//
//              The file also calls `MapData:AddMapData{...}` about twenty
//              times and `MapData:AddLocalMapInfo(...)` about a hundred more,
//              all BEFORE the two functions this exists for are defined
//              further down the chunk. Both have to be bound to something,
//              or the first unbound call raises a Lua error and the chunk
//              stops executing right there - meaning CheckCOBOExpressTicketMapID
//              and ComputeCOBOExpressTicketCost would never be defined at all,
//              and every later call into them would silently fail. Cobo
//              Express does not need what either call would have stored (a
//              level/dungeon-clear gate for entering a village by ID - see
//              Handlers_Field.cpp's EGS_STATE_CHANGE_FIELD_REQ for why this
//              build trusts the client's own menu instead of rebuilding that
//              gate), so both are bound as no-op stubs: enough to let the
//              chunk run to completion, nothing more.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

class CX2OfflineMapData
{
public:
	static CX2OfflineMapData*	Instance();
	static void					Release();

	/// Which Cobo Express "zone" a map ID belongs to (1-14 for the eight
	/// villages/rest stops and their fields; 0 means "not a Cobo Express
	/// destination at all", which is also how the real server refuses a
	/// warp). Loads MapData.lua on first use.
	int		CheckCOBOExpressTicketMapID( int iMapID );

	/// The ED cost of a paid warp between two maps, at the given level -
	/// MapData.lua's own formula, run verbatim.
	int		ComputeCOBOExpressTicketCost( int iSrcMapID, int iDesMapID, int iLevel );

	/// Triggers the load attempt if one has not happened yet, then reports
	/// whether it succeeded. NOT const, on purpose: a caller checking this
	/// before ever calling CheckCOBOExpressTicketMapID / ComputeCOBOExpressTicketCost
	/// must still cause the load, or the file is never even attempted.
	bool	IsLoaded()	{ EnsureLoaded(); return m_bScriptRan; }

	//////////////////////////////////////////////////////////////////////////
	// Bound into Lua as MapData:*. Public because lua_tinker needs to take
	// their addresses; nothing else should call them. Both no-op - see the
	// file header for why the chunk needs them bound at all.
	void	AddMapData_LUA();
	void	AddLocalMapInfo_LUA( int iVillageMapID, int iDungeonMode, int iDungeonIDWithDif );

private:
	CX2OfflineMapData();
	~CX2OfflineMapData();

	/// Loads on first use, once per process - see X2OfflineStatTable for why
	/// this cannot happen at construction time.
	void	EnsureLoaded();

private:
	static CX2OfflineMapData*	ms_pInstance;

	bool	m_bLoadAttempted;
	bool	m_bScriptRan;		///< the chunk executed without error, top to bottom
};

#endif SERV_IRUHADEV_OFFLINE
