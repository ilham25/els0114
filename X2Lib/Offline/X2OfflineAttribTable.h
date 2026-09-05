#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-05
// Description: Offline mode - phase 26 (OFFLINE_MODE_PHASE9_PLAN.md), item
//              attributes: the El shard rolls and the attribute amulets.
//
//              CXSLAttribEnchantItem's client-side twin, but a much smaller
//              one than X2OfflineEnchantTable, because most of that class is
//              ALREADY in the client. The server loads three files; the client
//              loads two of them, and one is shared:
//
//                AttribEnchantRequire.lua   BOTH - the stone count and the ED
//                                           cost, already parsed by
//                                           CX2EnchantItem::AddEnchantRequire_LUA
//                                           and readable through
//                                           GetAttribEnchantRequireMagicStoneCount
//                                           / GetAttribEnchantRequireED.
//                AttribEnchantItem.lua      client only - the damage effects.
//                AttribEnchantTable.lua     SERVER ONLY - what this loads.
//                AttribAttachTable.lua      SERVER ONLY - what this loads.
//
//              So is the *logic*: CX2EnchantItem::CanEnchantAttribute is
//              CXSLAttribEnchantItem::IsPossibleToPush (the fire/water/nature
//              and wind/light/dark exclusions), CX2EnchantItem::GetItemID is
//              GetRequireItemID, and ATI_*, ESI_SLOT_*, ACT_* and ENCHANT_TYPE
//              are all declared in X2EnchantItem.h with the server's values.
//              What has no client copy is exactly two things, and they are
//              this class: the three random-attribute lotteries that resolve
//              ET_RANDOM (an El shard [Unknown]) into a real attribute, and the
//              amulet -> fixed-attributes map.
//
//              **This loader borrows a global and gives it back.** Both files
//              are written against `g_pCX2EnchantItem`, which is also the name
//              the client's own CX2EnchantItem publishes for itself
//              (X2EnchantItem.cpp:33) - and the client binds only
//              AddEnchantData_LUA / AddEnchantRequire_LUA on it, not the five
//              methods these two files call. So EnsureLoaded points that global
//              at this object for the duration of the two DoMemory calls and
//              restores g_pData->GetEnchantItem() immediately afterwards. The
//              client re-publishes the global only from OpenScriptFile, which
//              runs once at start-up, long before any of this.
//
//              The identify table (AddIdentifyInfo, El shard [Unknown] ->
//              coloured shard) is parsed and stored but has no accessor:
//              EGS_IDENTIFY_ITEM_REQ is a different packet and still on the
//              ignore list. Same treatment X2OfflineEnchantTable gives the rows
//              its own phase did not need.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

class CX2OfflineAttribTable
{
public:
	static CX2OfflineAttribTable*	Instance();
	static void						Release();

	/// True once both files have been found, run, and produced rows. Loads on
	/// first call; every accessor below does the same.
	bool	IsLoaded();

	/// CXSLAttribEnchantItem::GetRandomAttribResult - resolve ET_RANDOM into a
	/// real attribute, given how many the item already carries.
	///
	/// eCurrentCountType is CX2EnchantItem::ATTRIB_COUNT_TYPE. For ACT_NONE
	/// neither existing type is read; for ACT_SINGLE only cFirst; for ACT_DUAL
	/// both, and the pair keys the triple table. Returns ET_NONE when the
	/// lottery has no row for the combination, which the caller must treat as
	/// a refusal rather than as "no attribute".
	char	GetRandomAttribResult( int eCurrentCountType,
								   char cFirstAttribEnchant, char cSecondAttribEnchant );

	/// Is cTypeToAdd registered in the triple lottery for this (first, second)
	/// pair? KLottery::IsExistCase, which is how the real server decides
	/// whether an explicitly chosen third attribute is legal.
	bool	IsExistTripleCase( char cFirstAttribEnchant, char cSecondAttribEnchant,
							   char cTypeToAdd );

	/// CXSLAttribEnchantItem::GetAttribAttachInfo - the three attributes an
	/// attribute amulet grants, in slot order. False when the item is not an
	/// amulet at all.
	bool	GetAttribAttachInfo( int iAttachItemID,
								 OUT char& cAttrib0, OUT char& cAttrib1, OUT char& cAttrib2 );

	//////////////////////////////////////////////////////////////////////////
	// Bound into Lua as g_pCX2EnchantItem:* for the duration of the load -
	// public for lua_tinker. Every method the two packed files name is bound,
	// for the reason X2OfflineEnchantTable's header gives.

	void	AddRandomAttribSingle_LUA( char cEnchantType, float fRate );
	void	AddRandomAttribDual_LUA( char cSingleEnchantType, char cDualEnchantType, float fRate );
	void	AddRandomAttribTriple_LUA( char cSingleEnchantType, char cDualEnchantType,
									   char cTripleEnchantType, float fRate );
	void	AddIdentifyInfo_LUA( int iSourceItemID, int iRequireItemID, int iResultItemID, float fRate );
	void	AddAttribAttachInfo_LUA( int iItemID, char cSlot0, char cSlot1, char cSlot2 );

	/// AttribEnchantRequire.lua is loaded by the CLIENT, not by this class, and
	/// its one call would otherwise be unbound if that file were ever routed
	/// through here. Bound as a no-op so a stray call cannot abandon a chunk.
	void	AddEnchantRequire_LUA( int iAttribEnchantType, int iEquipLv );

	void	Dump_LUA();

private:
	/// (case value, weight in percent), in registration order - the same shape
	/// X2OfflineEnchantTable uses, and the same reason: KLottery is a std::map
	/// keyed by case, and the ENCHANT_TYPE values registered here are small
	/// ascending integers, so registration order and key order agree.
	typedef std::vector< std::pair< int, float > >	KProbRow;

	/// (first attribute, second attribute) - CXSLAttribEnchantItem's
	/// KTripleEnchantKey.
	typedef std::pair< char, char >					KTripleKey;

	struct KAttachRow
	{
		char	m_cAttrib0;
		char	m_cAttrib1;
		char	m_cAttrib2;

		KAttachRow() : m_cAttrib0( 0 ), m_cAttrib1( 0 ), m_cAttrib2( 0 ) {}
	};

	struct KIdentifyRow
	{
		int		m_iRequireItemID;
		int		m_iResultItemID;
		float	m_fRate;

		KIdentifyRow() : m_iRequireItemID( 0 ), m_iResultItemID( 0 ), m_fRate( 0.0f ) {}
	};

private:
	CX2OfflineAttribTable();
	~CX2OfflineAttribTable();

	void	EnsureLoaded();

	/// Runs one packed script through the client's mass-file loader and Lua
	/// state, XOR first and plaintext as a fallback. Returns false and logs
	/// when the file is not there at all.
	bool	RunScript( const wchar_t* szFileName );

	/// KLottery::Decision over one registered row. Identical to
	/// CX2OfflineEnchantTable::LotteryDecideMulti; kept as its own copy rather
	/// than shared, because the two classes are twins of two different server
	/// managers and a shared helper would tie their lifetimes together.
	static int	LotteryDecideMulti( const KProbRow& kRow );

private:
	static CX2OfflineAttribTable*	ms_pInstance;

	bool	m_bLoadAttempted;
	bool	m_bLoaded;

	KProbRow									m_vecRandomSingle;
	std::map< char, KProbRow >					m_mapRandomDual;	///< first attrib -> lottery
	std::map< KTripleKey, KProbRow >			m_mapRandomTriple;
	std::map< int, KAttachRow >					m_mapAttribAttach;

	/// Parsed, stored, no accessor - EGS_IDENTIFY_ITEM_REQ is still ignored.
	std::map< int, std::vector< KIdentifyRow > >	m_mapIdentify;

	int		m_iSingleRows;
	int		m_iDualRows;
	int		m_iTripleRows;
	int		m_iAttachRows;
	int		m_iIdentifyRows;
};

#endif SERV_IRUHADEV_OFFLINE
