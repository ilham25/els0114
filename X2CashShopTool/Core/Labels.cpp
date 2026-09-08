//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-08
// Description: X2CashShopTool phase 6 - the General.ess reader, the
// client's own enum -> STR_ID switches transcribed, and the curated table
// for what is left. See the header for the four origins and the two rules.
//////////////////////////////////////////////////////////////////////////
#include "Labels.h"

#ifdef _MANAGED
#error "This TU compiled managed. /clr leaked in - see CppBuild.targets:733-738"
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>		// _wtoi
#include <string.h>

#include "KomArchive.h"		// NarrowPath / JoinPath

namespace
{
	//////////////////////////////////////////////////////////////////////
	// The string ids, taken one at a time out of the client's own code.
	// Every one of these was read back out of the shipped general.ess
	// before it was written down, so the comment and the string agree.

	// X2Lib/X2CashShop.cpp:9194 - wstrstm << m_iSalePrice << GET_STRING( STR_ID_34 )
	const int	STR_CURRENCY			= 34;		// "K-Ching"

	// X2Lib/X2ItemSlotManager.cpp:1270-1274 - what the client prints for m_bFashion
	const int	STR_COSTUME				= 251;		// "Costume"

	// X2Lib/X2ItemSlotManager.cpp:1089-1110 - the ITEM_GRADE switch. The
	// shipped strings carry square brackets; SLabel keeps both forms.
	const int	STR_GRADE_UNIQUE		= 257;		// "[Unique]"
	const int	STR_GRADE_ELITE			= 258;		// "[Elite]"
	const int	STR_GRADE_RARE			= 259;		// "[Rare]"
	const int	STR_GRADE_NORMAL		= 260;		// "[Normal]"
	const int	STR_GRADE_LOW			= 261;		// "[Old]"

	// X2Lib/X2ItemSlotManager.cpp:1199-1265 - the EQIP_POSITION switch.
	const int	STR_EP_HAIR				= 263;		// "Hair"
	const int	STR_EP_BODY				= 264;		// "Top Piece"
	const int	STR_EP_AC_BODY			= 265;		// "Accessory (Top Piece)"
	const int	STR_EP_LEG				= 266;		// "Bottom Piece"
	const int	STR_EP_AC_LEG			= 267;		// "Accessory (Bottom Piece)"
	const int	STR_EP_HAND				= 268;		// "Gloves"
	const int	STR_EP_FOOT				= 269;		// "Shoes"
	const int	STR_EP_WEAPON			= 270;		// "Weapon"
	const int	STR_EP_AC_FACE1			= 271;		// "Face Accessory (Top)"
	const int	STR_EP_AC_FACE2			= 272;		// "Face Accessory (Middle)"
	const int	STR_EP_AC_FACE3			= 273;		// "Face Accessory (Bottom)"
	const int	STR_EP_AC_RING			= 274;		// "Accessory (Ring)"
	const int	STR_EP_AC_NECKLESS		= 275;		// "Accessory (Necklace)"
	const int	STR_EP_AC_ARM			= 276;		// "Accessory (Arm)"
	const int	STR_EP_AC_WEAPON		= 277;		// "Accessory (Weapon)"

	// The same switch, under SERV_NEW_ONE_PIECE_AVATAR_SLOT
	// (X2Lib/X2ItemSlotManager.cpp:1260-1265). The flag IS defined for US
	// (KncWX2Server/Common/OnlyGlobal/ServerDefine/ServerDefine_US.h:300),
	// so this branch is live in this build rather than dead code.
	const int	STR_EP_ONEPIECE			= 28362;	// "Suit"

	// X2Lib/X2ItemSlotManager.cpp:2224-2241 - the ONLY ITEM_TYPE ->
	// STR_ID mapping in the client, two cases wide and sitting inside
	// SHOW_ITEM_TYPE_AT_TOOLTIP, which is defined NOWHERE in this tree
	// (grepped: the #ifdef and its #endif are the only two occurrences).
	// Compiled out is not the same as wrong: the pairs are the studio's
	// own and they cover the two biggest buckets in the catalog, so they
	// are used - and labelled Code, so nobody reads them as strings this
	// build ever paints.
	const int	STR_IT_WEAPON			= 270;		// "Weapon"
	const int	STR_IT_DEFENCE			= 17818;	// "Armor"

	//////////////////////////////////////////////////////////////////////

	const char* const	NOTE_EQIP_SWITCH =
		"the client's own EQIP_POSITION switch, X2Lib/X2ItemSlotManager.cpp:1199-1265";

	const char* const	NOTE_EQIP_ONEPIECE =
		"the same switch under SERV_NEW_ONE_PIECE_AVATAR_SLOT, X2Lib/X2ItemSlotManager.cpp:1260-1265;"
		" the flag is defined for US at ServerDefine_US.h:300";

	const char* const	NOTE_GRADE_SWITCH =
		"the client's own ITEM_GRADE switch, X2Lib/X2ItemSlotManager.cpp:1089-1110;"
		" the shipped string carries square brackets and they are stripped for a caption";

	const char* const	NOTE_ITEMTYPE_CODE =
		"X2Lib/X2ItemSlotManager.cpp:2224-2241, inside SHOW_ITEM_TYPE_AT_TOOLTIP -"
		" a macro defined nowhere in this tree, so this build never paints it";

	const char* const	NOTE_ITEMTYPE_TOOL =
		"the client has no ITEM_TYPE -> STR_ID mapping for this value; this tool's wording";

	const char* const	NOTE_IT_ACCESSORY =
		"this tool's wording. STR_ID_246 is \"Accessories\", but the client pairs no"
		" ITEM_TYPE with it - citing it here would be a guess wearing a citation's clothes";

	const char* const	NOTE_EP_NONE =
		"this tool's wording. EP_NONE is not a slot at all, so no equip-slot string can"
		" name it honestly - and it is the second-largest bucket in the catalog";

	const char* const	NOTE_EP_TOOL =
		"the client's EQIP_POSITION switch has no case for this value; this tool's wording";

	const char* const	NOTE_TAB_TOOL =
		"this tool's wording. The shop's tab captions are TGA regions rather than strings"
		" (see the tier-2 note in Labels.cpp), so there is no game text to cite";

	// Two notes, not one: only the eight sub-tabs that appear in
	// DLG_Cash_Shop_Subpage_Fashion.lua have a control name to offer, and a
	// note claiming one for the other twenty-four would be exactly the sort
	// of overclaim this whole file exists to prevent.
	const char* const	NOTE_SUBTAB_SCRIPT =
		"this tool's wording. The shop paints this tab as a picture rather than as text,"
		" so there is nothing to quote; the studio's own control name is printed beside it";

	const char* const	NOTE_SUBTAB_TOOL =
		"this tool's wording. The shop paints this tab as a picture rather than as text,"
		" and no shipped dialog script in this tree names this one";

	//////////////////////////////////////////////////////////////////////
	// TIER 2 - the studio's own words, out of a shipped dialog script.
	//
	// The cash shop's sub-tab captions are PICTURES, not text:
	// DLG_Cash_Shop_Subpage_Fashion.lua (in data034.kom, 8,560 bytes,
	// luac like everything else) builds each sub-tab as a texture radio
	// button - SetNormalTex( "DLG_Common_New_Texture54_A.TGA",
	// "wapon_normal" ) - and tags it with its CASH_SHOP_SUB_CATEGORY
	// through AddDummyInt. So the nearest thing to a name is the control
	// name, read out of the script's constant pool in the order it lays
	// the controls out.
	//
	// TRANSCRIBED, NOT TIDIED: Tab_Wapon is a typo in the original, and
	// Hood1 / Hood2 are the studio's own words for the top-piece and
	// bottom-piece tabs whose CSSC_* names say BODY and LEG. Correcting
	// either would make the tool disagree with the game for a reason
	// nobody could later reconstruct - the same rule CLAUDE.md states for
	// a live table with an unfinished-looking row.
	//
	// AND THEY ARE NOT THE PRIMARY LABEL, deliberately. "Tab_Hood1" is no
	// more a human name for a thing than "CSSC_FASHION_BODY" is, so the
	// sub-tab labels below are Tool wording and the control name rides
	// along in the details pane. The temptation was to borrow the
	// equip-slot string instead - CSSC_FASHION_BODY does line up with
	// EP_DEFENCE_BODY, "Top Piece" - but the only thing in this repo that
	// asserts that pairing is our own offline emulator
	// (X2Lib/Offline/X2OfflineCashShop.cpp:320-332), so citing it as the
	// game's word would be the tool quoting itself.
	struct SScriptTabName
	{
		const char*	pszEnumName;
		const char*	pszControl;
	};

	const SScriptTabName	SCRIPT_TAB_NAMES[] =
	{
		{ "CSSC_ALL",					"Totalitem"		},
		{ "CSSC_FASHION_WEAPON",		"Tab_Wapon"		},
		{ "CSSC_FASHION_BODY",			"Tab_Hood1"		},
		{ "CSSC_FASHION_LEG",			"Tab_Hood2"		},
		{ "CSSC_FASHION_HAIR",			"Tab_Hair"		},
		{ "CSSC_FASHION_HAND",			"Tab_Glove"		},
		{ "CSSC_FASHION_FOOT",			"Tab_Shoes"		},
		{ "CSSC_FASHION_ONE_PIECE",		"Tab_OnePiece"	},
	};

	//////////////////////////////////////////////////////////////////////
	// The table. One row per enum value, keyed on the SCRIPT'S OWN NAME
	// so nothing here transcribes an ordinal that could drift.
	//
	// iStringID >= 0  -> the label is that General.ess row, and eOrigin
	//                    says whether the mapping that produced it is
	//                    live in this build (Ess) or compiled out (Code).
	// iStringID < 0   -> pszText is used, and eOrigin is Tool.

	struct SLabelDef
	{
		ELabelEnum		eEnum;
		const char*		pszEnumName;
		int				iStringID;
		ELabelOrigin	eOrigin;
		const char*		pszText;		// used only when iStringID < 0
		const char*		pszNote;
		bool			bStripBrackets;
	};

	const SLabelDef	LABEL_DEFS[] =
	{
		//////////////////////////////////////////////////////////////////
		// ITEM_TYPE. Two of thirteen have a mapping in the client and
		// both are compiled out; the other eleven are this tool's words.
		{ LabelEnum_ItemType, "IT_WEAPON",		STR_IT_WEAPON,	LabelOrigin_Code, NULL,						NOTE_ITEMTYPE_CODE,	false },
		{ LabelEnum_ItemType, "IT_DEFENCE",		STR_IT_DEFENCE,	LabelOrigin_Code, NULL,						NOTE_ITEMTYPE_CODE,	false },
		{ LabelEnum_ItemType, "IT_ACCESSORY",	-1,	LabelOrigin_Tool, "Accessory",							NOTE_IT_ACCESSORY,	false },
		{ LabelEnum_ItemType, "IT_SPECIAL",		-1,	LabelOrigin_Tool, "Special or package",					NOTE_ITEMTYPE_TOOL,	false },
		{ LabelEnum_ItemType, "IT_QUEST",		-1,	LabelOrigin_Tool, "Quest item",							NOTE_ITEMTYPE_TOOL,	false },
		{ LabelEnum_ItemType, "IT_QICK_SLOT",	-1,	LabelOrigin_Tool, "Quick-slot consumable",				NOTE_ITEMTYPE_TOOL,	false },
		{ LabelEnum_ItemType, "IT_MATERIAL",	-1,	LabelOrigin_Tool, "Crafting material",					NOTE_ITEMTYPE_TOOL,	false },
		{ LabelEnum_ItemType, "IT_SKILL_MEMO",	-1,	LabelOrigin_Tool, "Skill note",							NOTE_ITEMTYPE_TOOL,	false },
		{ LabelEnum_ItemType, "IT_OUTLAY",		-1,	LabelOrigin_Tool, "Expendable",							NOTE_ITEMTYPE_TOOL,	false },
		{ LabelEnum_ItemType, "IT_SKILL",		-1,	LabelOrigin_Tool, "Skill",								NOTE_ITEMTYPE_TOOL,	false },
		{ LabelEnum_ItemType, "IT_ETC",			-1,	LabelOrigin_Tool, "Other",								NOTE_ITEMTYPE_TOOL,	false },
		{ LabelEnum_ItemType, "IT_NONE",		-1,	LabelOrigin_Tool, "no type",							NOTE_ITEMTYPE_TOOL,	false },
		{ LabelEnum_ItemType, "IT_END",			-1,	LabelOrigin_Tool, "end marker, not a real type",			NOTE_ITEMTYPE_TOOL,	false },

		//////////////////////////////////////////////////////////////////
		// EQIP_POSITION. Sixteen of the twenty values that occur in the
		// catalog come straight out of the client's own switch; the four
		// that do not are marked as this tool's.
		{ LabelEnum_EquipPosition, "EP_DEFENCE_HAIR",	STR_EP_HAIR,			LabelOrigin_Ess,  NULL,	NOTE_EQIP_SWITCH,	false },
		{ LabelEnum_EquipPosition, "EP_DEFENCE_BODY",	STR_EP_BODY,			LabelOrigin_Ess,  NULL,	NOTE_EQIP_SWITCH,	false },
		{ LabelEnum_EquipPosition, "EP_DEFENCE_LEG",	STR_EP_LEG,				LabelOrigin_Ess,  NULL,	NOTE_EQIP_SWITCH,	false },
		{ LabelEnum_EquipPosition, "EP_DEFENCE_HAND",	STR_EP_HAND,			LabelOrigin_Ess,  NULL,	NOTE_EQIP_SWITCH,	false },
		{ LabelEnum_EquipPosition, "EP_DEFENCE_FOOT",	STR_EP_FOOT,			LabelOrigin_Ess,  NULL,	NOTE_EQIP_SWITCH,	false },
		{ LabelEnum_EquipPosition, "EP_WEAPON_HAND",	STR_EP_WEAPON,			LabelOrigin_Ess,  NULL,	NOTE_EQIP_SWITCH,	false },
		{ LabelEnum_EquipPosition, "EP_AC_BODY",		STR_EP_AC_BODY,			LabelOrigin_Ess,  NULL,	NOTE_EQIP_SWITCH,	false },
		{ LabelEnum_EquipPosition, "EP_AC_LEG",			STR_EP_AC_LEG,			LabelOrigin_Ess,  NULL,	NOTE_EQIP_SWITCH,	false },
		{ LabelEnum_EquipPosition, "EP_AC_FACE1",		STR_EP_AC_FACE1,		LabelOrigin_Ess,  NULL,	NOTE_EQIP_SWITCH,	false },
		{ LabelEnum_EquipPosition, "EP_AC_FACE2",		STR_EP_AC_FACE2,		LabelOrigin_Ess,  NULL,	NOTE_EQIP_SWITCH,	false },
		{ LabelEnum_EquipPosition, "EP_AC_FACE3",		STR_EP_AC_FACE3,		LabelOrigin_Ess,  NULL,	NOTE_EQIP_SWITCH,	false },
		{ LabelEnum_EquipPosition, "EP_AC_RING",		STR_EP_AC_RING,			LabelOrigin_Ess,  NULL,	NOTE_EQIP_SWITCH,	false },
		{ LabelEnum_EquipPosition, "EP_AC_NECKLESS",	STR_EP_AC_NECKLESS,		LabelOrigin_Ess,  NULL,	NOTE_EQIP_SWITCH,	false },
		{ LabelEnum_EquipPosition, "EP_AC_ARM",			STR_EP_AC_ARM,			LabelOrigin_Ess,  NULL,	NOTE_EQIP_SWITCH,	false },
		{ LabelEnum_EquipPosition, "EP_AC_WEAPON",		STR_EP_AC_WEAPON,		LabelOrigin_Ess,  NULL,	NOTE_EQIP_SWITCH,	false },
		{ LabelEnum_EquipPosition, "EP_ONEPIECE_FASHION",STR_EP_ONEPIECE,		LabelOrigin_Ess,  NULL,	NOTE_EQIP_ONEPIECE,	false },

		{ LabelEnum_EquipPosition, "EP_NONE",			-1,	LabelOrigin_Tool, "not equipped",					NOTE_EP_NONE,	false },
		{ LabelEnum_EquipPosition, "EP_QUICK_SLOT",		-1,	LabelOrigin_Tool, "Quick slot",						NOTE_EP_TOOL,	false },
		{ LabelEnum_EquipPosition, "EP_DEFENCE_FACE",	-1,	LabelOrigin_Tool, "Face piece",						NOTE_EP_TOOL,	false },
		{ LabelEnum_EquipPosition, "EP_RAVEN_LEFT_ARM",	-1,	LabelOrigin_Tool, "Left arm (Raven)",				NOTE_EP_TOOL,	false },

		//////////////////////////////////////////////////////////////////
		// ITEM_GRADE. Five of six are the game's own text.
		{ LabelEnum_ItemGrade, "IG_UNIQUE",	STR_GRADE_UNIQUE,	LabelOrigin_Ess,  NULL,	NOTE_GRADE_SWITCH,	true },
		{ LabelEnum_ItemGrade, "IG_ELITE",	STR_GRADE_ELITE,	LabelOrigin_Ess,  NULL,	NOTE_GRADE_SWITCH,	true },
		{ LabelEnum_ItemGrade, "IG_RARE",	STR_GRADE_RARE,		LabelOrigin_Ess,  NULL,	NOTE_GRADE_SWITCH,	true },
		{ LabelEnum_ItemGrade, "IG_NORMAL",	STR_GRADE_NORMAL,	LabelOrigin_Ess,  NULL,	NOTE_GRADE_SWITCH,	true },
		{ LabelEnum_ItemGrade, "IG_LOW",	STR_GRADE_LOW,		LabelOrigin_Ess,  NULL,	NOTE_GRADE_SWITCH,	true },
		{ LabelEnum_ItemGrade, "IG_NONE",	-1,	LabelOrigin_Tool, "no grade",
			"the client's ITEM_GRADE switch falls through for IG_NONE and prints nothing; this tool's wording", false },

		//////////////////////////////////////////////////////////////////
		// CASH_SHOP_CATEGORY - the tabs. All Tool wording: the captions
		// the game paints are regions of DLG_Common_New_Texture54_A.TGA,
		// so there is no string to reach for any of them.
		{ LabelEnum_CashCategory, "CSC_FASHION",		-1,	LabelOrigin_Tool, "Costumes",			NOTE_TAB_TOOL,	false },
		{ LabelEnum_CashCategory, "CSC_ACCESSORY",		-1,	LabelOrigin_Tool, "Accessories",		NOTE_TAB_TOOL,	false },
		{ LabelEnum_CashCategory, "CSC_CONSUME",		-1,	LabelOrigin_Tool, "Consumables",		NOTE_TAB_TOOL,	false },
		{ LabelEnum_CashCategory, "CSC_INSTALL",		-1,	LabelOrigin_Tool, "Account and slots",	NOTE_TAB_TOOL,	false },
		{ LabelEnum_CashCategory, "CSC_PET",			-1,	LabelOrigin_Tool, "Pets and mounts",	NOTE_TAB_TOOL,	false },
		{ LabelEnum_CashCategory, "CSC_EVENT",			-1,	LabelOrigin_Tool, "Event",				NOTE_TAB_TOOL,	false },
		{ LabelEnum_CashCategory, "CSC_EVENT_2",		-1,	LabelOrigin_Tool, "Event (second)",		NOTE_TAB_TOOL,	false },
		{ LabelEnum_CashCategory, "CSC_AUTO_PAYMENT",	-1,	LabelOrigin_Tool, "Subscription",
			"this tool's wording. Not a tab of goods: its billing category 1113 is above the"
			" signed-char wire field, so a product placed here can never be browsed to", false },

		//////////////////////////////////////////////////////////////////
		// CASH_SHOP_SUB_CATEGORY - the sub-tabs. Tool wording, with the
		// eight tier-2 control names attached where the shipped dialog
		// script has one.
		{ LabelEnum_CashSubCategory, "CSSC_ALL",					-1,	LabelOrigin_Tool, "Everything",				NOTE_SUBTAB_SCRIPT,	false },

		{ LabelEnum_CashSubCategory, "CSSC_FASHION_WEAPON",			-1,	LabelOrigin_Tool, "Costume weapon",			NOTE_SUBTAB_SCRIPT,	false },
		{ LabelEnum_CashSubCategory, "CSSC_FASHION_BODY",			-1,	LabelOrigin_Tool, "Costume top",			NOTE_SUBTAB_SCRIPT,	false },
		{ LabelEnum_CashSubCategory, "CSSC_FASHION_LEG",			-1,	LabelOrigin_Tool, "Costume bottom",			NOTE_SUBTAB_SCRIPT,	false },
		{ LabelEnum_CashSubCategory, "CSSC_FASHION_HAIR",			-1,	LabelOrigin_Tool, "Costume hair",			NOTE_SUBTAB_SCRIPT,	false },
		{ LabelEnum_CashSubCategory, "CSSC_FASHION_HAND",			-1,	LabelOrigin_Tool, "Costume gloves",			NOTE_SUBTAB_SCRIPT,	false },
		{ LabelEnum_CashSubCategory, "CSSC_FASHION_FOOT",			-1,	LabelOrigin_Tool, "Costume shoes",			NOTE_SUBTAB_SCRIPT,	false },
		{ LabelEnum_CashSubCategory, "CSSC_FASHION_ONE_PIECE",		-1,	LabelOrigin_Tool, "Costume suit",			NOTE_SUBTAB_SCRIPT,	false },

		{ LabelEnum_CashSubCategory, "CSSC_ACCESSORY_FACE",			-1,	LabelOrigin_Tool, "Face accessory",			NOTE_SUBTAB_TOOL,	false },
		{ LabelEnum_CashSubCategory, "CSSC_ACCESSORY_BODY",			-1,	LabelOrigin_Tool, "Top accessory",			NOTE_SUBTAB_TOOL,	false },
		{ LabelEnum_CashSubCategory, "CSSC_ACCESSORY_LEG",			-1,	LabelOrigin_Tool, "Bottom accessory",		NOTE_SUBTAB_TOOL,	false },
		{ LabelEnum_CashSubCategory, "CSSC_ACCESSORY_ARM",			-1,	LabelOrigin_Tool, "Arm accessory",			NOTE_SUBTAB_TOOL,	false },
		{ LabelEnum_CashSubCategory, "CSSC_ACCESSORY_RING",			-1,	LabelOrigin_Tool, "Ring",					NOTE_SUBTAB_TOOL,	false },
		{ LabelEnum_CashSubCategory, "CSSC_ACCESSORY_NECKLACE",		-1,	LabelOrigin_Tool, "Necklace",				NOTE_SUBTAB_TOOL,	false },
		{ LabelEnum_CashSubCategory, "CSSC_ACCESSORY_WEAPON",		-1,	LabelOrigin_Tool, "Weapon accessory",		NOTE_SUBTAB_TOOL,	false },

		{ LabelEnum_CashSubCategory, "CSSC_CONSUME_RESTORE",		-1,	LabelOrigin_Tool, "Potions and recovery",	NOTE_SUBTAB_TOOL,	false },
		{ LabelEnum_CashSubCategory, "CSSC_CONSUME_PREMIUM",		-1,	LabelOrigin_Tool, "Premium services",		NOTE_SUBTAB_TOOL,	false },
		{ LabelEnum_CashSubCategory, "CSSC_CONSUME_INIT",			-1,	LabelOrigin_Tool, "Resets and rebuilds",	NOTE_SUBTAB_TOOL,	false },
		{ LabelEnum_CashSubCategory, "CSSC_CONSUME_ETC",			-1,	LabelOrigin_Tool, "Other consumables",		NOTE_SUBTAB_TOOL,	false },

		{ LabelEnum_CashSubCategory, "CSSC_INSTALL_ADD_SLOT",		-1,	LabelOrigin_Tool, "Extra slots",			NOTE_SUBTAB_TOOL,	false },
		{ LabelEnum_CashSubCategory, "CSSC_INSTALL_PERSONAL_SHOP",	-1,	LabelOrigin_Tool, "Private shop",			NOTE_SUBTAB_TOOL,	false },
		{ LabelEnum_CashSubCategory, "CSSC_INSTALL_ETC",			-1,	LabelOrigin_Tool, "Other account items",	NOTE_SUBTAB_TOOL,	false },

		{ LabelEnum_CashSubCategory, "CSSC_PET_PET",				-1,	LabelOrigin_Tool, "Pets",					NOTE_SUBTAB_TOOL,	false },
		{ LabelEnum_CashSubCategory, "CSSC_PET_CONSUME",			-1,	LabelOrigin_Tool, "Pet food and items",		NOTE_SUBTAB_TOOL,	false },
		{ LabelEnum_CashSubCategory, "CSSC_PET_RIDING",				-1,	LabelOrigin_Tool, "Mounts",					NOTE_SUBTAB_TOOL,	false },

		{ LabelEnum_CashSubCategory, "CSSC_EVENT_PACKAGE",			-1,	LabelOrigin_Tool, "Event packages",			NOTE_SUBTAB_TOOL,	false },
		{ LabelEnum_CashSubCategory, "CSSC_EVENT_FASHION",			-1,	LabelOrigin_Tool, "Event costumes",			NOTE_SUBTAB_TOOL,	false },
		{ LabelEnum_CashSubCategory, "CSSC_EVENT_ACCESSORY",		-1,	LabelOrigin_Tool, "Event accessories",		NOTE_SUBTAB_TOOL,	false },
		{ LabelEnum_CashSubCategory, "CSSC_EVENT_CONSUME",			-1,	LabelOrigin_Tool, "Event consumables",		NOTE_SUBTAB_TOOL,	false },
		{ LabelEnum_CashSubCategory, "CSSC_EVENT_INSTALL",			-1,	LabelOrigin_Tool, "Event account items",	NOTE_SUBTAB_TOOL,	false },
		{ LabelEnum_CashSubCategory, "CSSC_EVENT_PET",				-1,	LabelOrigin_Tool, "Event pets",				NOTE_SUBTAB_TOOL,	false },

		{ LabelEnum_CashSubCategory, "CSSC_AUTO_PAYMENT_BASIC",		-1,	LabelOrigin_Tool, "Basic subscription",		NOTE_SUBTAB_TOOL,	false },
	};

	const size_t	LABEL_DEF_COUNT		= sizeof( LABEL_DEFS ) / sizeof( LABEL_DEFS[0] );
	const size_t	SCRIPT_NAME_COUNT	= sizeof( SCRIPT_TAB_NAMES ) / sizeof( SCRIPT_TAB_NAMES[0] );

	//////////////////////////////////////////////////////////////////////

	const SLabelDef* FindDef( ELabelEnum eEnum, const char* pszEnumName )
	{
		if( NULL == pszEnumName || '\0' == pszEnumName[0] )
			return NULL;

		for( size_t u = 0; u != LABEL_DEF_COUNT; ++u )
		{
			if( LABEL_DEFS[u].eEnum == eEnum && 0 == ::strcmp( LABEL_DEFS[u].pszEnumName, pszEnumName ) )
				return &LABEL_DEFS[u];
		}

		return NULL;
	}

	std::string ScriptNameFor( const char* pszEnumName )
	{
		if( NULL == pszEnumName )
			return std::string();

		for( size_t u = 0; u != SCRIPT_NAME_COUNT; ++u )
		{
			if( 0 == ::strcmp( SCRIPT_TAB_NAMES[u].pszEnumName, pszEnumName ) )
				return std::string( SCRIPT_TAB_NAMES[u].pszControl );
		}

		return std::string();
	}

	// "[Rare]" -> "Rare". The shipped grade strings are bracketed because
	// the game paints them inside an item tooltip; a filter caption wants
	// them off, and SLabel keeps the untouched form as well so the
	// stripping is never invisible.
	std::string StripBrackets( const std::string& str )
	{
		if( str.size() >= 2 && '[' == str[0] && ']' == str[str.size() - 1] )
			return str.substr( 1, str.size() - 2 );

		return str;
	}

	// The last resort, for a value the table does not cover: turn the
	// script's own name into something readable rather than showing the
	// enumerator. "EP_SOME_NEW_SLOT" -> "Some new slot". Marked Tool and
	// noted as derived, because it is this tool's wording by construction
	// - and a value that reaches here is one nobody has looked at, which
	// is worth saying out loud rather than papering over.
	std::string HumanizeEnumName( const char* pszEnumName )
	{
		if( NULL == pszEnumName || '\0' == pszEnumName[0] )
			return std::string();

		std::string str( pszEnumName );

		// Drop the table's prefix: everything up to and including the
		// first underscore, but only when there is something after it.
		const size_t uUnderscore = str.find( '_' );
		if( std::string::npos != uUnderscore && uUnderscore + 1 < str.size() )
			str = str.substr( uUnderscore + 1 );

		for( size_t u = 0; u != str.size(); ++u )
		{
			const unsigned char c = (unsigned char) str[u];

			if( '_' == c )
				str[u] = ' ';
			else if( c >= 'A' && c <= 'Z' )
				str[u] = (char)( c - 'A' + 'a' );
		}

		if( false == str.empty() && str[0] >= 'a' && str[0] <= 'z' )
			str[0] = (char)( str[0] - 'a' + 'A' );

		return str;
	}
}

//////////////////////////////////////////////////////////////////////////

const wchar_t* GeneralStringFileName()
{
	return L"General.ess";
}

const char* LabelOriginName( ELabelOrigin eOrigin )
{
	switch( eOrigin )
	{
	case LabelOrigin_Ess:		return "the game's own text";
	case LabelOrigin_Code:		return "the studio's text, from a mapping this build compiles out";
	case LabelOrigin_Script:	return "the studio's own control name, from a shipped dialog script";
	case LabelOrigin_Tool:		return "this tool's wording";
	default:					break;
	}

	return "nothing";
}

const char* LabelOriginTag( ELabelOrigin eOrigin )
{
	switch( eOrigin )
	{
	case LabelOrigin_Ess:		return "ESS";
	case LabelOrigin_Code:		return "CODE";
	case LabelOrigin_Script:	return "SCRIPT";
	case LabelOrigin_Tool:		return "TOOL";
	default:					break;
	}

	return "-";
}

const char* LabelEnumName( ELabelEnum eEnum )
{
	switch( eEnum )
	{
	case LabelEnum_ItemType:			return "ITEM_TYPE";
	case LabelEnum_EquipPosition:		return "EQIP_POSITION";		// the script's own misspelling
	case LabelEnum_ItemGrade:			return "ITEM_GRADE";
	case LabelEnum_CashCategory:		return "CASH_SHOP_CATEGORY";
	case LabelEnum_CashSubCategory:		return "CASH_SHOP_SUB_CATEGORY";
	default:							break;
	}

	return "?";
}

//////////////////////////////////////////////////////////////////////////

CLabelStore::CLabelStore()
: m_bLoaded( false )
{
}

bool CLabelStore::Load( const std::wstring& wstrDir, std::string& strError )
{
	strError.clear();

	m_vecStrings.clear();
	m_strDegraded.clear();
	m_bLoaded = false;

	m_wstrPath = JoinPath( wstrDir, GeneralStringFileName() );

	if( false == ReadStringTable( m_wstrPath, strError ) )
	{
		m_vecStrings.clear();

		// The degrade line the plan asks for: name the file, and say what
		// its absence costs. Shown once in the status bar and once on the
		// console, never per label.
		m_strDegraded = "General.ess could not be read from the game directory, so every"
			" label below is this tool's own wording rather than the game's - item types,"
			" equip slots, grades, the Costume flag and the K-Ching currency all fall back."
			" The catalog itself is unaffected. Reason: ";
		m_strDegraded += strError;

		return false;
	}

	m_bLoaded = true;
	return true;
}

std::string CLabelStore::String( int iStringID ) const
{
	if( iStringID < 0 || (size_t) iStringID >= m_vecStrings.size() )
		return std::string();

	return m_vecStrings[(size_t) iStringID];
}

SLabel CLabelStore::FromStringID( int iStringID, ELabelOrigin eOrigin, const char* pszNote,
									bool bStripBrackets ) const
{
	SLabel kLabel;

	const std::string strRaw = String( iStringID );

	kLabel.strEssRaw	= strRaw;
	kLabel.iStringID	= iStringID;
	kLabel.strNote		= ( NULL != pszNote ) ? pszNote : "";
	kLabel.eOrigin		= eOrigin;
	kLabel.strText		= bStripBrackets ? StripBrackets( strRaw ) : strRaw;

	return kLabel;
}

SLabel CLabelStore::Label( ELabelEnum eEnum, const char* pszEnumName, int iValue ) const
{
	const SLabelDef* pDef = FindDef( eEnum, pszEnumName );

	//////////////////////////////////////////////////////////////////////
	// A value the table does not cover. Either Enum.lua named it and the
	// table has not caught up, or Enum.lua has no name for it either - in
	// which case the number is all there is, and saying so beats
	// inventing a word for it.

	if( NULL == pDef )
	{
		SLabel kLabel;
		kLabel.eOrigin	= LabelOrigin_Tool;
		kLabel.strText	= HumanizeEnumName( pszEnumName );

		if( kLabel.strText.empty() )
		{
			char szBuffer[64];
			::sprintf_s( szBuffer, sizeof( szBuffer ), "value %d", iValue );

			kLabel.strText	= szBuffer;
			kLabel.strNote	= "neither this tool's label table nor Enum.lua has a name for this"
				" value; the number is all there is";
		}
		else
		{
			kLabel.strNote	= "derived from the script's own name for the value, because this"
				" tool's label table has no entry for it";
		}

		return kLabel;
	}

	SLabel kLabel;

	if( pDef->iStringID >= 0 )
	{
		kLabel = FromStringID( pDef->iStringID, pDef->eOrigin, pDef->pszNote, pDef->bStripBrackets );

		// The strings are gone but the mapping is not, so the origin
		// changes rather than the answer disappearing. This is the degrade
		// path: the fallback is this tool's wording, marked as such.
		if( kLabel.strText.empty() )
		{
			kLabel.eOrigin		= LabelOrigin_Tool;
			kLabel.strText		= HumanizeEnumName( pDef->pszEnumName );
			kLabel.strNote		= "General.ess is missing, so the game's own text for this value"
				" could not be read; this is the fallback";

			// THE CITATION GOES WITH THE STRING. Keeping STR_ID_264 on a
			// label that is no longer that string is the "looks sourced
			// and is not" failure this whole file exists to prevent.
			kLabel.iStringID	= -1;
			kLabel.strEssRaw.clear();
		}
	}
	else
	{
		kLabel.eOrigin	= pDef->eOrigin;
		kLabel.strText	= ( NULL != pDef->pszText ) ? pDef->pszText : "";
		kLabel.strNote	= ( NULL != pDef->pszNote ) ? pDef->pszNote : "";
	}

	kLabel.strScriptName = ScriptNameFor( pDef->pszEnumName );

	return kLabel;
}

SLabel CLabelStore::Costume() const
{
	SLabel kLabel = FromStringID( STR_COSTUME, LabelOrigin_Ess,
		"what the client itself prints for m_bFashion, X2Lib/X2ItemSlotManager.cpp:1270-1274", false );

	if( kLabel.strText.empty() )
	{
		// The fallback is honest because it is the REAL string, read out
		// of the repo and cited in the note - but the origin says TOOL and
		// the iStringID goes, because this run did not read it.
		kLabel.eOrigin		= LabelOrigin_Tool;
		kLabel.strText		= "Costume";
		kLabel.strNote		= "General.ess is missing; this is the fallback for STR_ID_251, the string"
			" the client prints for m_bFashion";
		kLabel.iStringID	= -1;
		kLabel.strEssRaw.clear();
	}

	return kLabel;
}

SLabel CLabelStore::Currency() const
{
	SLabel kLabel = FromStringID( STR_CURRENCY, LabelOrigin_Ess,
		"what the shop appends to every price, X2Lib/X2CashShop.cpp:9194", false );

	if( kLabel.strText.empty() )
	{
		kLabel.eOrigin		= LabelOrigin_Tool;
		kLabel.strText		= "K-Ching";
		kLabel.strNote		= "General.ess is missing; this is the fallback for STR_ID_34, the string"
			" the shop appends to every price";
		kLabel.iStringID	= -1;
		kLabel.strEssRaw.clear();
	}

	return kLabel;
}

void CLabelStore::DumpAll( std::vector<SLabelDumpRow>& vecOut ) const
{
	vecOut.clear();
	vecOut.reserve( LABEL_DEF_COUNT );

	for( size_t u = 0; u != LABEL_DEF_COUNT; ++u )
	{
		SLabelDumpRow kRow;
		kRow.eEnum			= LABEL_DEFS[u].eEnum;
		kRow.strEnumName	= LABEL_DEFS[u].pszEnumName;
		kRow.kLabel			= Label( LABEL_DEFS[u].eEnum, LABEL_DEFS[u].pszEnumName, -1 );

		vecOut.push_back( kRow );
	}
}

//////////////////////////////////////////////////////////////////////////
// The reader.
//
// The whole file at once rather than the client's fgetws loop: 3 MB is
// nothing, the client's 2,048-wchar_t line buffer is a limit this does not
// need, and reading the bytes myself makes the UTF-16LE assumption
// explicit instead of resting on what MSVC's fgetws does to a stream
// opened "rb". Every other step is the client's, in the client's order -
// strip the trailing CRLF, XOR every wchar_t with 16, split on the first
// tab, check the index, then turn the "\n" escape into a real CRLF
// (KTDXLIB/KTDXStringTable.cpp:195-285).

bool CLabelStore::ReadStringTable( const std::wstring& wstrPath, std::string& strError )
{
	FILE* pFile = NULL;
	if( 0 != ::_wfopen_s( &pFile, wstrPath.c_str(), L"rb" ) || NULL == pFile )
	{
		strError = "the file is not there, or could not be opened for reading";
		return false;
	}

	::fseek( pFile, 0, SEEK_END );
	const long lSize = ::ftell( pFile );
	::fseek( pFile, 0, SEEK_SET );

	if( lSize < 2 )
	{
		::fclose( pFile );
		strError = "the file is too short to hold even a byte-order mark";
		return false;
	}

	std::vector<char> vecBytes( (size_t) lSize );
	const size_t uRead = ::fread( &vecBytes[0], 1, (size_t) lSize, pFile );
	::fclose( pFile );

	if( uRead != (size_t) lSize )
	{
		strError = "the file could not be read to its end";
		return false;
	}

	// UTF-16LE with a BOM, and the client skips exactly two bytes without
	// checking it (KTDXStringTable.cpp:210). The tool checks, because a
	// file that is not UTF-16LE would otherwise be read as gibberish that
	// happens to fail the index test for the wrong reason.
	if( (unsigned char) vecBytes[0] != 0xFF || (unsigned char) vecBytes[1] != 0xFE )
	{
		strError = "the file does not start with a UTF-16LE byte-order mark";
		return false;
	}

	const wchar_t*	pText	= (const wchar_t*)( &vecBytes[0] + 2 );
	const size_t	uChars	= ( (size_t) lSize - 2 ) / sizeof( wchar_t );

	m_vecStrings.reserve( 32768 );

	size_t uStart	= 0;
	int    iIndex	= 0;

	while( uStart < uChars )
	{
		size_t uEnd = uStart;
		while( uEnd < uChars && L'\n' != pText[uEnd] )
			++uEnd;

		std::wstring wstrLine( pText + uStart, uEnd - uStart );
		uStart = uEnd + 1;

		// A trailing CR belongs to the line terminator, not to the row.
		if( false == wstrLine.empty() && L'\r' == wstrLine[wstrLine.size() - 1] )
			wstrLine.erase( wstrLine.size() - 1 );

		// A trailing empty segment after the final newline is not a row.
		if( wstrLine.empty() && uStart >= uChars )
			break;

		// The whole line is encrypted, index digits and separating tab
		// included, so this runs before anything is parsed out of it.
		for( size_t u = 0; u != wstrLine.size(); ++u )
			wstrLine[u] = (wchar_t)( wstrLine[u] ^ 16 );

		const size_t uTab = wstrLine.find( L'\t' );

		// THE INDEX CHECK. The client ASSERTs this and then pushes an
		// empty row; the tool refuses the file, because every label after
		// a bad row would be off by one and nothing on screen would say so.
		const std::wstring	wstrIndex	= wstrLine.substr( 0, uTab );
		const int			iRead		= ::_wtoi( wstrIndex.c_str() );

		if( iRead != iIndex )
		{
			char szBuffer[256];
			::sprintf_s( szBuffer, sizeof( szBuffer ),
				"row %d of the string table says its index is %d, so every label after it"
				" would be off by one (the client ASSERTs this at KTDXStringTable.cpp:250)",
				iIndex, iRead );

			strError = szBuffer;
			return false;
		}

		std::wstring wstrContent;
		if( std::wstring::npos != uTab && uTab + 1 < wstrLine.size() )
			wstrContent = wstrLine.substr( uTab + 1 );

		// The client turns the two-character escape into a real line
		// break (KTDXStringTable.cpp:274). Nothing this tool shows is
		// multi-line, but a label carrying a literal backslash-n would be
		// visibly wrong, so the same replacement happens here.
		for( size_t uAt = wstrContent.find( L"\\n" );
			 std::wstring::npos != uAt;
			 uAt = wstrContent.find( L"\\n", uAt + 2 ) )
		{
			wstrContent.replace( uAt, 2, L"\r\n" );
		}

		m_vecStrings.push_back( NarrowPath( wstrContent ) );
		++iIndex;
	}

	if( m_vecStrings.empty() )
	{
		strError = "the file holds no rows";
		return false;
	}

	return true;
}
