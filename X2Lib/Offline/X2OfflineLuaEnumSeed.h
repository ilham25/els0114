#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-04
// Description: GENERATED - do not hand-edit. See X2OfflineLuaEnum.h.
//
//              The name -> value tables the studio's server-side .lua files
//              index by name: RandomItemTable.lua writes
//              USE_CONDITION["UC_ANYONE"] and UNIT_CLASS["UC_NONE"], PetData.lua
//              writes PET_UNIT_ID["PUI_PETTE_PPORU"], RidingPetData.lua writes
//              RIDING_PET_UNIT_ID["RPUI_NASOD_MOBI"]. On the live server those
//              globals exist because the server built them from its own enums;
//              in the client's Lua state nothing defines them at all, so every
//              such subscript would come back nil and every row would load as
//              zero.
//
//              Every value here was read out of the CLIENT's own header - not
//              retyped, not copied from the server - by
//              scratchpad/gen_luaenum.py:
//
//                UNIT_CLASS          X2Lib/X2Unit.h
//                PET_UNIT_ID         X2Lib/X2PetManager.h
//                RIDING_PET_UNIT_ID  X2Lib/X2RidingPetManager.h
//
//              ITEM_TYPE and ITEM_GRADE were added by hand in phase 12, not by
//              the generator (ResolveTable.lua is the only file that needs
//              them so far) - transcribed from X2Lib/X2Item_Preprocessing.inl
//              and diffed the same way against
//              KncWX2Server/Common/X2Data/XSLItem.h: identical, value for
//              value, no exceptions.
//
//              and then diffed against the server's copy of the same enum
//              (KncWX2Server/Common/X2Data/XSL{Unit,PetManager,RidingPetManager}.h),
//              which is a genuinely independent transcription of the same
//              contract. Result: PET_UNIT_ID 103/103 identical,
//              RIDING_PET_UNIT_ID 6/6 identical, UNIT_CLASS 58 shared and 0
//              mismatched. The 7 UNIT_CLASS names the server has and the client
//              does not are the transcendence classes (UC_*_2) plus UC_MAX,
//              compiled out of this build.
//
//              USE_CONDITION is the one exception: the client has no copy of it
//              at all, so its four values are CXSLItem::USE_CONDITION verbatim
//              (KncWX2Server/Common/X2Data/XSLItem.h:68-73).
//
//              A name the .lua uses and this build does not have stays absent
//              on purpose. It resolves to nil, arrives as 0, and the loader
//              logs and skips that row - which is the honest outcome, because
//              the shipped ServerResource .lua files are NEWER than this source
//              tree and name things it cannot render. See
//              CX2OfflinePetData::EnsureLoaded for the list that actually bites.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

namespace X2OfflineLuaEnumSeed
{
	// CX2Unit::UNIT_CLASS - X2Lib/X2Unit.h
	static const CX2OfflineLuaEnum::KEntry UNIT_CLASS[] =
	{
		{ "UC_NONE",                                 0 },
		{ "UC_ELSWORD_SWORDMAN",                     1 },
		{ "UC_ARME_VIOLET_MAGE",                     2 },
		{ "UC_LIRE_ELVEN_RANGER",                    3 },
		{ "UC_RAVEN_FIGHTER",                        4 },
		{ "UC_EVE_NASOD",                            5 },
		{ "UC_CHUNG_IRON_CANNON",                    6 },
		{ "UC_ARA_MARTIAL_ARTIST",                   7 },
		{ "UC_ELESIS_KNIGHT",                        8 },
		{ "UC_ELSWORD_KNIGHT",                       10 },
		{ "UC_ELSWORD_MAGIC_KNIGHT",                 11 },
		{ "UC_LIRE_COMBAT_RANGER",                   12 },
		{ "UC_LIRE_SNIPING_RANGER",                  13 },
		{ "UC_ARME_HIGH_MAGICIAN",                   14 },
		{ "UC_ARME_DARK_MAGICIAN",                   15 },
		{ "UC_RAVEN_SOUL_TAKER",                     16 },
		{ "UC_RAVEN_OVER_TAKER",                     17 },
		{ "UC_EVE_EXOTIC_GEAR",                      18 },
		{ "UC_EVE_ARCHITECTURE",                     19 },
		{ "UC_CHUNG_FURY_GUARDIAN",                  20 },
		{ "UC_CHUNG_SHOOTING_GUARDIAN",              21 },
		{ "UC_ELSWORD_SHEATH_KNIGHT",                22 },
		{ "UC_ARME_BATTLE_MAGICIAN",                 23 },
		{ "UC_LIRE_TRAPPING_RANGER",                 24 },
		{ "UC_RAVEN_WEAPON_TAKER",                   25 },
		{ "UC_EVE_ELECTRA",                          26 },
		{ "UC_CHUNG_SHELLING_GUARDIAN",              27 },
		{ "UC_ARA_LITTLE_HSIEN",                     28 },
		{ "UC_ELESIS_SABER_KNIGHT",                  29 },
		{ "UC_ELESIS_PYRO_KNIGHT",                   30 },
		{ "UC_ARA_LITTLE_DEVIL",                     31 },
		{ "UC_ELSWORD_SHIELD_KNIGHT",                32 },
		{ "UC_AISHA_SWORD_MAGICIAN",                 33 },
		{ "UC_RENA_STRING_RANGER",                   34 },
		{ "UC_RAVEN_GLOBAL_1",                       35 },
		{ "UC_EVE_GLOBAL_1",                         36 },
		{ "UC_CHUNG_PRINCESS_GUARDIAN",              37 },
		{ "UC_ARA_GLOBAL_1",                         38 },
		{ "UC_ELSWORD_LORD_KNIGHT",                  100 },
		{ "UC_ELSWORD_RUNE_SLAYER",                  101 },
		{ "UC_LIRE_WIND_SNEAKER",                    102 },
		{ "UC_LIRE_GRAND_ARCHER",                    103 },
		{ "UC_ARME_ELEMENTAL_MASTER",                104 },
		{ "UC_ARME_VOID_PRINCESS",                   105 },
		{ "UC_RAVEN_BLADE_MASTER",                   106 },
		{ "UC_RAVEN_RECKLESS_FIST",                  107 },
		{ "UC_EVE_CODE_NEMESIS",                     108 },
		{ "UC_EVE_CODE_EMPRESS",                     109 },
		{ "UC_CHUNG_IRON_PALADIN",                   110 },
		{ "UC_CHUNG_DEADLY_CHASER",                  111 },
		{ "UC_ELSWORD_INFINITY_SWORD",               112 },
		{ "UC_ARME_DIMENSION_WITCH",                 113 },
		{ "UC_LIRE_NIGHT_WATCHER",                   114 },
		{ "UC_RAVEN_VETERAN_COMMANDER",              115 },
		{ "UC_EVE_BATTLE_SERAPH",                    116 },
		{ "UC_CHUNG_TACTICAL_TROOPER",               117 },
		{ "UC_ARA_SAKRA_DEVANAM",                    118 },
		{ "UC_ARA_YAMA_RAJA",                        119 },
		{ "UC_VALIDE_END",                           120 },
		{ "UC_END",                                  121 },
	};

	// CXSLItem::USE_CONDITION - the client has no copy of this one
	static const CX2OfflineLuaEnum::KEntry USE_CONDITION[] =
	{
		{ "UC_NONE",                                 0 },
		{ "UC_ANYONE",                               1 },
		{ "UC_ONE_UNIT",                             2 },
		{ "UC_ONE_CLASS",                            3 },
	};

	// CX2PetManager::PET_UNIT_ID - X2Lib/X2PetManager.h
	static const CX2OfflineLuaEnum::KEntry PET_UNIT_ID[] =
	{
		{ "PUI_NONE",                                0 },
		{ "PUI_TEST",                                1 },
		{ "PUI_PETTE_PPORU",                         2 },
		{ "PUI_PETTE_ANCIENT_PPORU",                 3 },
		{ "PUI_TREE_KNIGHT",                         4 },
		{ "PUI_PETTE_PPORU_EVENT",                   5 },
		{ "PUI_PETTE_ANCIENT_PPORU_F",               6 },
		{ "PUI_TREE_KNIGHT_F",                       7 },
		{ "PUI_HOLLO_WITCH",                         8 },
		{ "PUI_HOLLO_WITCH_F",                       9 },
		{ "PUI_XMAS2010_PPORU",                      10 },
		{ "PUI_MYSTIC_NORM",                         11 },
		{ "PUI_MYSTIC_BLK",                          12 },
		{ "PUI_MYSTIC_WHT",                          13 },
		{ "PUI_MYSTIC_RED",                          14 },
		{ "PUI_HATCHLING_NORM",                      15 },
		{ "PUI_HATCHLING_DARK",                      16 },
		{ "PUI_HATCHLING_FIRE",                      17 },
		{ "PUI_HATCHLING_LIGHT",                     18 },
		{ "PUI_PETIT_ANCIENT_HALLOW_DARK",           19 },
		{ "PUI_PETIT_ANCIENT_HALLOW_FLAME",          20 },
		{ "PUI_PETIT_ANCIENT_HALLOW_SPOOK",          21 },
		{ "PUI_PETIT_ANCIENT_HALLOW_SPRITE",         22 },
		{ "PUI_PETIT_PPORU_BOOMHILL",                23 },
		{ "PUI_TREE_KNIGHT_XMAS2011",                24 },
		{ "PUI_TREE_KNIGHT_SANTA2011",               25 },
		{ "PUI_PETIT_LIGHTBRINGER_UNIEL",            26 },
		{ "PUI_PETIT_DARKBRINGER_UNIRING",           27 },
		{ "PUI_PET_SILVERFOX_MIHO",                  28 },
		{ "PUI_PET_FIREFOX_SHIHO",                   29 },
		{ "PUI_PET_DARKFOX_NAHO",                    30 },
		{ "PUI_PET_BLOOD_EATER",                     31 },
		{ "PUI_PET_MANA_EATER",                      32 },
		{ "PUI_PET_MILLENNIAFOX_EUN",                33 },
		{ "PUI_PETIT_PPORU_HATS_ON",                 34 },
		{ "PUI_STINKO_HATS_ON",                      35 },
		{ "PUI_EBONY_HATS_ON",                       36 },
		{ "PUI_PANDA_STICK_RED",                     96 },
		{ "PUI_PANDA_STICK_BLUE",                    97 },
		{ "PUI_PANDA_STICK_RED_F",                   98 },
		{ "PUI_PANDA_STICK_BLUE_F",                  99 },
		{ "PUI_PETTE_ANCIENT_PPORU_SHADOW",          100 },
		{ "PUI_GAMANIA_PETTE_PPORU",                 101 },
		{ "PUI_PETTE_ANCIENT_PPORU_SHADOW_F",        102 },
		{ "PUI_TREE_KNIGHT_SPRING",                  110 },
		{ "PUI_TREE_KNIGHT_FALL",                    111 },
		{ "PUI_TREE_KNIGHT_WINTER",                  112 },
		{ "PUI_TREE_KNIGHT_SPRING_F",                113 },
		{ "PUI_TREE_KNIGHT_FALL_F",                  114 },
		{ "PUI_TREE_KNIGHT_WINTER_F",                115 },
		{ "PUI_HATCHLING_FIRE_F",                    116 },
		{ "PUI_HATCHLING_LIGHT_F",                   117 },
		{ "PUI_HATCHLING_STAR_F",                    118 },
		{ "PUI_HATCHLING_NORM_F",                    119 },
		{ "PUI_HATCHLING_DARK_F",                    120 },
		{ "PUI_CHINAJOY_PPORU_EVENT",                121 },
		{ "PUI_HATCHLING_BLUE",                      122 },
		{ "PUI_DEATH_SCYTHE",                        123 },
		{ "PUI_DEATH_SCYTHE_F",                      124 },
		{ "PUI_PANDA_ROYAL_RED",                     125 },
		{ "PUI_PANDA_ROYAL_WHITE",                   126 },
		{ "PUI_MYSTIC_NORM_F",                       127 },
		{ "PUI_MYSTIC_BLK_F",                        128 },
		{ "PUI_MYSTIC_WHT_F",                        129 },
		{ "PUI_MYSTIC_RED_F",                        130 },
		{ "PUI_MYSTIC_BLUE",                         131 },
		{ "PUI_MYSTIC_BLUE_F",                       132 },
		{ "PUI_PETIT_LIGHTBRINGER_UNIEL_F",          133 },
		{ "PUI_SHERIFF_NORM",                        134 },
		{ "PUI_SHERIFF_WHITE",                       135 },
		{ "PUI_SHERIFF_NORM_F",                      136 },
		{ "PUI_SHERIFF_WHITE_F",                     137 },
		{ "PUI_PETIT_DARKBRINGER_UNIRING_F",         138 },
		{ "PUI_PET_BLOOD_EATER_EVENT",               139 },
		{ "PUI_HATCHLING_BLUE_F",                    140 },
		{ "PUI_PET_SILVERFOX_MIHO_F",                30000 },
		{ "PUI_PET_FIREFOX_SHIHO_F",                 30001 },
		{ "PUI_PET_DARKFOX_NAHO_F",                  30002 },
		{ "PUI_PET_MANA_EATER_EVENT_INT",            30003 },
		{ "PUI_PET_MERCENARY_PPORU_EVENT_INT",       30004 },
		{ "PUI_RAINCOAT_CAT_BLACK",                  30005 },
		{ "PUI_RAINCOAT_CAT_YELLOW",                 30006 },
		{ "PUI_RAINCOAT_CAT_BLACK_F",                30007 },
		{ "PUI_RAINCOAT_CAT_YELLOW_F",               30008 },
		{ "PUI_EVENT_EAT_FOOD_PPORU_F",              30009 },
		{ "PUI_HOLLO_WITCH_EVENT_F",                 30010 },
		{ "PUI_PETTE_PPORU_EVENT_BRAZIL",            30011 },
		{ "PUI_HALLOWEEN_PUMPKIN_TANYA",             30012 },
		{ "PUI_HALLOWEEN_PUMPKIN_LAEL",              30013 },
		{ "PUI_HALLOWEEN_PUMPKIN_LOJETA",            30014 },
		{ "PUI_HALLOWEEN_PUMPKIN_NARENEA",           30015 },
		{ "PUI_HALLOWEEN_PUMPKIN_ELDENA",            30016 },
		{ "PUI_HALLOWEEN_PUMPKIN_TANYA_F",           30017 },
		{ "PUI_HALLOWEEN_PUMPKIN_LAEL_F",            30018 },
		{ "PUI_HALLOWEEN_PUMPKIN_LOJETA_F",          30019 },
		{ "PUI_HALLOWEEN_PUMPKIN_NARENEA_F",         30020 },
		{ "PUI_HALLOWEEN_PUMPKIN_ELDENA_F",          30021 },
		{ "PUI_FIREWORK_PPORU",                      30024 },
		{ "PUI_PET_SNOWBUMP",                        30025 },
		{ "PUI_PET_HUNTER_PENGUIN",                  30026 },
		{ "PUI_TREE_KNIGHT_EVENT_F",                 30027 },
		{ "PUI_HATCHLING_NORM_EVENT_F",              30028 },
		{ "PUI_MYSTIC_EVENT_NORM_F",                 30029 },
	};

	// CX2RidingPetManager::RIDING_PET_UNIT_ID - X2Lib/X2RidingPetManager.h
	static const CX2OfflineLuaEnum::KEntry RIDING_PET_UNIT_ID[] =
	{
		{ "RPUI_NONE",                               0 },
		{ "RPUI_NASOD_MOBI",                         1 },
		{ "RPUI_KOUCIKA",                            2 },
		{ "RPUI_ANCIENT_PPORU",                      3 },
		{ "RPUI_SCORPION_TYPE_R",                    4 },
		{ "RPUI_END",                                5 },
	};

	// CX2Item::ITEM_TYPE - X2Lib/X2Item_Preprocessing.inl. Added for phase 12
	// (ResolveTable.lua's ITEM_TYPE["IT_WEAPON"] / ["IT_DEFENCE"]). Verified
	// against the server's independent copy
	// (KncWX2Server/Common/X2Data/XSLItem.h:16-32): identical, value for value.
	static const CX2OfflineLuaEnum::KEntry ITEM_TYPE[] =
	{
		{ "IT_NONE",                                 0 },
		{ "IT_WEAPON",                               1 },
		{ "IT_DEFENCE",                              2 },
		{ "IT_ACCESSORY",                            3 },
		{ "IT_SKILL",                                4 },
		{ "IT_QICK_SLOT",                            5 },
		{ "IT_MATERIAL",                             6 },
		{ "IT_SPECIAL",                              7 },
		{ "IT_QUEST",                                8 },
		{ "IT_OUTLAY",                               9 },
		{ "IT_ETC",                                  10 },
		{ "IT_SKILL_MEMO",                           11 },
	};

	// CX2Item::ITEM_GRADE - X2Lib/X2Item_Preprocessing.inl. Added for phase 12
	// (ResolveTable.lua's ITEM_GRADE["IG_UNIQUE"] etc). Verified against the
	// server's independent copy (XSLItem.h:34-42): identical, value for value.
	static const CX2OfflineLuaEnum::KEntry ITEM_GRADE[] =
	{
		{ "IG_NONE",                                 0 },
		{ "IG_UNIQUE",                               1 },
		{ "IG_ELITE",                                2 },
		{ "IG_RARE",                                 3 },
		{ "IG_NORMAL",                               4 },
		{ "IG_LOW",                                  5 },
	};

	// SEnum::VILLAGE_MAP_ID - KncWX2Server/Common/Enum/Enum.h. Added for the
	// Cobo Express fix (MapData.lua's VILLAGE_MAP_ID["VMI_RUBEN"] etc). This is
	// the ONE table here with no client/server diff to do: Enum.h is a SHARED
	// header X2Lib already compiles (X2ServerPacket.h -> CommonPacket.h:16), not
	// two independently maintained copies, so there is nothing to drift.
	static const CX2OfflineLuaEnum::KEntry VILLAGE_MAP_ID[] =
	{
		{ "VMI_INVALID",                             0 },

		{ "VMI_BATTLE_FIELD_ELDER_REST_00",          1000 },
		{ "VMI_BATTLE_FIELD_BESMA_REST_00",          1001 },
		{ "VMI_BATTLE_FIELD_ALTERA_REST_00",         1002 },
		{ "VMI_BATTLE_FIELD_PEITA_REST_00",          1003 },
		{ "VMI_BATTLE_FIELD_VELDER_REST_00",         1004 },
		{ "VMI_BATTLE_FIELD_HAMEL_REST_00",          1005 },
		{ "VMI_BATTLE_FIELD_SANDER_REST_00",         1006 },
		{ "VMI_BATTLE_FIELD_REST_END",               1007 },

		{ "VMI_TUTORIAL",                            2000 },

		{ "VMI_RUBEN",                               20000 },
		{ "VMI_ELDER",                                20001 },
		{ "VMI_BESMA",                                20002 },
		{ "VMI_ALTERA",                               20003 },
		{ "VMI_VELDER",                               20004 },
		{ "VMI_PEITA",                                20005 },
		{ "VMI_HAMEL",                                20006 },
		{ "VMI_SANDER",                               20007 },
		{ "VMI_VILLAGE_MAP_END",                      20008 },

		{ "VMI_DUNGEON_GATE_RUBEN",                   10000 },
		{ "VMI_DUNGEON_GATE_ELDER",                   10001 },
		{ "VMI_DUNGEON_GATE_BESMA",                   10002 },
		{ "VMI_DUNGEON_GATE_ALTERA",                  10003 },
		{ "VMI_DUNGEON_GATE_VELDER",                  10004 },
		{ "VMI_DUNGEON_GATE_HAMEL",                   10005 },

		{ "VMI_DUNGEON_LOUNGE_RUBEN",                 30000 },
		{ "VMI_DUNGEON_LOUNGE_ELDER",                 30001 },
		{ "VMI_DUNGEON_LOUNGE_BESMA",                 30002 },
		{ "VMI_DUNGEON_LOUNGE_ALTERA",                30003 },
		{ "VMI_DUNGEON_LOUNGE_PEITA",                 30004 },
		{ "VMI_DUNGEON_LOUNGE_VELDER",                30005 },
		{ "VMI_DUNGEON_LOUNGE_HAMEL",                 30006 },

		{ "VMI_BATTLE_FIELD_RUBEN_FIELD_01",          40000 },
		{ "VMI_BATTLE_FIELD_ELDER_FIELD_01",          40001 },
		{ "VMI_BATTLE_FIELD_ELDER_FIELD_02",          40002 },
		{ "VMI_BATTLE_FIELD_BESMA_FIELD_01",          40003 },
		{ "VMI_BATTLE_FIELD_BESMA_FIELD_02",          40004 },
		{ "VMI_BATTLE_FIELD_BESMA_FIELD_03",          40005 },
		{ "VMI_BATTLE_FIELD_ALTERA_FIELD_01",         40006 },
		{ "VMI_BATTLE_FIELD_ALTERA_FIELD_02",         40007 },
		{ "VMI_BATTLE_FIELD_ALTERA_FIELD_03",         40008 },
		{ "VMI_BATTLE_FIELD_PEITA_FIELD_01",          40009 },
		{ "VMI_BATTLE_FIELD_PEITA_FIELD_02",          40010 },
		{ "VMI_BATTLE_FIELD_PEITA_FIELD_03",          40011 },
		{ "VMI_BATTLE_FIELD_VELDER_FIELD_01",         40012 },
		{ "VMI_BATTLE_FIELD_VELDER_FIELD_02",         40013 },
		{ "VMI_BATTLE_FIELD_VELDER_FIELD_03",         40014 },
		{ "VMI_BATTLE_FIELD_VELDER_FIELD_04",         40015 },
		{ "VMI_BATTLE_FIELD_HAMEL_FIELD_01",          40016 },
		{ "VMI_BATTLE_FIELD_HAMEL_FIELD_02",          40017 },
		{ "VMI_BATTLE_FIELD_HAMEL_FIELD_03",          40018 },
		{ "VMI_BATTLE_FIELD_HAMEL_FIELD_04",          40019 },
		{ "VMI_BATTLE_FIELD_VELDER_SHIP_STAGE",       40020 },
		{ "VMI_BATTLE_FIELD_HAMEL_SHIP_STAGE",        40021 },
		{ "VMI_BATTLE_FIELD_SANDER_FIELD_01",         40022 },
		{ "VMI_BATTLE_FIELD_SANDER_FIELD_02",         40023 },
		{ "VMI_BATTLE_FIELD_SANDER_FIELD_03",         40024 },
		{ "VMI_BATTLE_FIELD_SANDER_FIELD_04",         40025 },
		{ "VMI_BATTLE_FIELD_END",                     40026 },
	};

}

#endif SERV_IRUHADEV_OFFLINE
