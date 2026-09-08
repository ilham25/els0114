//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-08
// Description: X2CashShopTool phase 6 - every human label the window
// shows for an enum value, in one place, each one carrying WHERE IT CAME
// FROM. See CASH_SHOP_TOOL_PLAN.md, "Phase 6 - Plain language, and a
// layout that reads".
//
// Phases 1-5 labelled everything with the SCRIPT's name for it -
// CSC_FASHION, IT_DEFENCE, CSSC_FASHION_WEAPON - which is the name of the
// enumerator and not the name of the thing. This file is where a value
// stops being spelled the way a header spells it.
//
// FOUR ORIGINS, and the whole point of the file is that they never blur:
//
//   Ess     the game's own localized string, reached through the client's
//           own enum -> STR_ID switch. General.ess is a LOOSE file in the
//           game directory (KTDXLIB/KTDXApp.cpp:162), not a .kom member:
//           UTF-16LE, one line per row, every wchar_t XOR 16, the row
//           index in the left half of each line and the string in the
//           right (KTDXLIB/KTDXStringTable.cpp:195-285).
//   Code    a mapping that exists in this repo but that THIS BUILD
//           compiles out - the two item types inside
//           SHOW_ITEM_TYPE_AT_TOOLTIP, which is defined nowhere in the
//           tree. The pairs are still the studio's own, so they are used
//           and labelled as what they are.
//   Script  the studio's own control name out of a shipped dialog .lua,
//           for the cash-shop sub-tabs, whose captions are PICTURES and
//           so cannot be reached as strings at all.
//   Tool    this tool's own wording. Never presented as the game's:
//           --labels prints the whole set so every invention in it can be
//           read in one place, and the details pane names the origin of
//           the row on screen.
//
// TWO RULES THIS FILE EXISTS TO ENFORCE:
//
//   1. A label that LOOKS sourced and is not is worse than no label. The
//      live-DB section of CLAUDE.md is about exactly that failure, and it
//      is why IT_ACCESSORY is Tool wording here even though STR_ID_246 is
//      "Accessories" - the client has no IT_ACCESSORY -> STR_ID mapping,
//      so pairing them would be a guess wearing a citation's clothes.
//   2. The label never replaces the number. category, product_no,
//      item_id, quantity and price are what gets written to the save; a
//      label sits beside them.
//
// The table is keyed on the SCRIPT'S OWN ENUM NAME, not on an ordinal,
// because the tool already reads those names out of Enum.lua at extract
// time (SEnumNameRow) - so nothing here transcribes a number that could
// drift from the data. Callers resolve value -> name with
// LookupEnumRowName and pass the name in.
//
// No sqlite3, zlib, lua or libxml header here - see the note at the top of
// KomArchive.h. This file is native and stays native.
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <vector>

//////////////////////////////////////////////////////////////////////////

enum ELabelOrigin
{
	LabelOrigin_None	= 0,	// nothing was asked for
	LabelOrigin_Ess,			// General.ess, through the client's own switch
	LabelOrigin_Code,			// a mapping in this repo that this build compiles out
	LabelOrigin_Script,			// a control name out of a shipped dialog .lua
	LabelOrigin_Tool,			// this tool's wording
};

const char*	LabelOriginName( ELabelOrigin eOrigin );		// "the game's own text", ...
const char*	LabelOriginTag( ELabelOrigin eOrigin );			// "ESS", "CODE", "SCRIPT", "TOOL"

// Which enum a label is being asked for. One enum rather than five
// accessors so --labels can walk the whole table generically.
enum ELabelEnum
{
	LabelEnum_ItemType			= 0,
	LabelEnum_EquipPosition,
	LabelEnum_ItemGrade,
	LabelEnum_CashCategory,		// CASH_SHOP_CATEGORY, the tabs
	LabelEnum_CashSubCategory,	// CASH_SHOP_SUB_CATEGORY, the sub-tabs
	LabelEnum_Count,
};

const char*	LabelEnumName( ELabelEnum eEnum );		// the script's table name, for the technical view

struct SLabel
{
	// What to put on screen. Never empty for a value the tool was asked
	// about: a value with no table entry and no Enum.lua name still comes
	// back as Tool wording rather than as a blank.
	std::string		strText;

	ELabelOrigin	eOrigin;

	// The General.ess row this came from, or -1. Kept so --labels and the
	// details pane can show the citation rather than assert it.
	int				iStringID;

	// The .ess row EXACTLY as the file holds it, before any tidying. Only
	// the grades differ from strText: the shipped strings carry square
	// brackets ("[Rare]") because the game paints them inside an item
	// tooltip, and a filter caption wants them off. Keeping both means the
	// stripping is visible rather than silent. Empty unless eOrigin is Ess
	// or Code.
	std::string		strEssRaw;

	// The studio's own control name for this sub-tab, out of a shipped
	// dialog .lua, or empty. Shown in the details pane and in the
	// technical view; NOT used as the primary label - see the note on the
	// tier-2 table in Labels.cpp for why Tab_Hood1 is not an improvement
	// on CSSC_FASHION_BODY.
	std::string		strScriptName;

	// Why this label reads the way it does: the switch it was transcribed
	// from, or the reason it is the tool's own wording. Printed by
	// --labels and shown in the details pane.
	std::string		strNote;

	SLabel() : eOrigin( LabelOrigin_None ), iStringID( -1 ) {}

	bool IsGameText() const	{ return LabelOrigin_Ess == eOrigin; }
};

//////////////////////////////////////////////////////////////////////////
// One row of --labels: an enum value, the script's name for it, and the
// label with its origin. Built by CLabelStore::DumpAll.

struct SLabelDumpRow
{
	ELabelEnum	eEnum;
	std::string	strEnumName;		// "IT_DEFENCE", "CSSC_FASHION_WEAPON"
	SLabel		kLabel;

	SLabelDumpRow() : eEnum( LabelEnum_ItemType ) {}
};

//////////////////////////////////////////////////////////////////////////
// The label store.
//
// Load() reads General.ess out of the given directory - EVERY RUN, and
// never into the index cache. That is deliberate: the file is 3 MB of
// UTF-16 parsed in tens of milliseconds, and keeping it out of the cache
// is what keeps this phase clear of the HasColumn trap phase 4 documented
// (a version bump does not add a column to a table that already exists).
//
// A store that failed to load still answers every question - with the
// tool's own wording, and with Degraded() naming the file that is missing
// and what that costs. A window that quietly shows IT_DEFENCE because a
// file was absent is indistinguishable from phase 5's window.

class CLabelStore
{
public:
	CLabelStore();

	// wstrDir is the game directory - the same one the archives are
	// mounted from. Returns false when the strings could not be read, and
	// fills strError; the store is still usable, on Tool wording only.
	bool	Load( const std::wstring& wstrDir, std::string& strError );

	bool				HasStrings() const	{ return m_bLoaded; }
	size_t				StringCount() const	{ return m_vecStrings.size(); }
	const std::wstring&	Path() const		{ return m_wstrPath; }

	// Empty when HasStrings(). Otherwise one sentence naming the file and
	// what its absence costs, for the status bar and the console.
	const std::string&	Degraded() const	{ return m_strDegraded; }

	// The raw string table, for the two label-less lookups below and for
	// anything a later phase needs. Empty string for an id out of range -
	// the client's GetString does the same (KTDXStringTable.cpp:26-34).
	std::string	String( int iStringID ) const;

	//////////////////////////////////////////////////////////////////////
	// The labels.

	// pszEnumName is the SCRIPT's own name for the value, as
	// LookupEnumRowName returns it. Pass an empty or unknown name and the
	// answer is Tool wording built from the value - which is information
	// (the value is in the data and the script has no name for it) rather
	// than a blank.
	SLabel	Label( ELabelEnum eEnum, const char* pszEnumName, int iValue ) const;

	// The two standalone strings the window needs, both reached through
	// the client's own use of them:
	//   Costume  - STR_ID_251, what the client prints for m_bFashion
	//              (X2Lib/X2ItemSlotManager.cpp:1270-1274). The tool said
	//              "fashion" until this phase; the game says Costume.
	//   Currency - STR_ID_34, what the shop appends to a price
	//              (X2Lib/X2CashShop.cpp:9194). The tool said ED until
	//              this phase, and the shop says K-Ching.
	SLabel	Costume() const;
	SLabel	Currency() const;

	// Every value of every enum in the table, in table order, so the whole
	// set of this tool's own wording can be read in one place instead of
	// being hunted through a UI. Drives --labels.
	void	DumpAll( std::vector<SLabelDumpRow>& vecOut ) const;

private:
	// The reader. Its failure modes are all reported rather than
	// swallowed, and the index check is one of them: the left half of
	// every line must equal the running line count or every label after
	// the bad row is off by one. The client ASSERTs it and then carries on
	// with an empty row; the tool refuses the file, because a tool that
	// silently mislabels is worse than one that says it cannot label.
	bool	ReadStringTable( const std::wstring& wstrPath, std::string& strError );

	SLabel	FromStringID( int iStringID, ELabelOrigin eOrigin, const char* pszNote,
							bool bStripBrackets ) const;

	std::vector<std::string>	m_vecStrings;	// UTF-8, index == STR_ID
	std::wstring				m_wstrPath;
	std::string					m_strDegraded;
	bool						m_bLoaded;
};

// The loose file the client opens by this name (KTDXLIB/KTDXApp.cpp:162).
// Windows matches it case-insensitively, which is why the shipped
// lower-case general.ess is found by it.
const wchar_t*	GeneralStringFileName();
