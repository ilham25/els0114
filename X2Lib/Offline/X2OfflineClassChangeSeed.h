#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-04
// Description: GENERATED - do not hand-edit. Cash "class change" item ->
//              the CX2Unit::UNIT_CLASS it advances the character to.
//
//              This is the one cash item that does NOT land in the bag. On the
//              live server, claiming it is intercepted before the insert
//              (GSUserCashShop.cpp:2887) and turned into a class change; the
//              item never exists. Offline it was landing in the bag as an
//              ordinary item that did nothing forever, which is what "the
//              change class item isn't working" looks like from the inside.
//
//              Built by scratchpad/gen_classchange.py from two independent
//              sources that have to agree:
//
//                the IDs   X2Lib/X2Define.h's UNIT_CLASS_CHANGE_*_ITEM_ID
//                the map   CXSLItem::GetClassChangeCashItem's switch
//                          (KncWX2Server/Common/X2Data/XSLItem.cpp:950)
//
//              They do: 42 IDs, and every one of them appears in both, with no
//              ID on only one side. That is a real cross-check - the client
//              names the constants and the server maps them, and neither was
//              derived from the other.
//
//              Which branch of the switch counts: every case sits inside an
//              #ifdef naming a transcendence class, with an #else naming the
//              older UC_*_2 placeholder. SERV_ELSWORD_INFINITY_SWORD,
//              SERV_ARME_DIMENSION_WITCH, SERV_RENA_NIGHT_WATCHER,
//              SERV_RAVEN_VETERAN_COMMANDER, SERV_EVE_BATTLE_SERAPH,
//              SERV_CHUNG_TACTICAL_TROOPER and SERV_ARA_CHANGE_CLASS_SECOND are
//              all ON in US_SERVICE (verified by compiler probe), so the live
//              case is the one naming a class the client has - and the #else
//              branch names a UC_*_2 that this build does not compile at all.
//              Reading the file without resolving that gives six items the
//              wrong target class.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

namespace X2OfflineClassChangeSeed
{
	struct KRow
	{
		int	m_iItemID;		///< the cash item
		int	m_iUnitClass;	///< CX2Unit::UNIT_CLASS it advances to
	};

	static const KRow ROWS[] =
	{
		{ 241950    , 10  },	// UC_ELSWORD_KNIGHT
		{ 241960    , 11  },	// UC_ELSWORD_MAGIC_KNIGHT
		{ 241970    , 22  },	// UC_ELSWORD_SHEATH_KNIGHT
		{ 241980    , 14  },	// UC_ARME_HIGH_MAGICIAN
		{ 241990    , 15  },	// UC_ARME_DARK_MAGICIAN
		{ 242000    , 23  },	// UC_ARME_BATTLE_MAGICIAN
		{ 242010    , 12  },	// UC_LIRE_COMBAT_RANGER
		{ 242020    , 13  },	// UC_LIRE_SNIPING_RANGER
		{ 242030    , 24  },	// UC_LIRE_TRAPPING_RANGER
		{ 242040    , 16  },	// UC_RAVEN_SOUL_TAKER
		{ 242050    , 17  },	// UC_RAVEN_OVER_TAKER
		{ 242060    , 25  },	// UC_RAVEN_WEAPON_TAKER
		{ 242070    , 18  },	// UC_EVE_EXOTIC_GEAR
		{ 242080    , 19  },	// UC_EVE_ARCHITECTURE
		{ 242090    , 26  },	// UC_EVE_ELECTRA
		{ 242100    , 20  },	// UC_CHUNG_FURY_GUARDIAN
		{ 242110    , 21  },	// UC_CHUNG_SHOOTING_GUARDIAN
		{ 242120    , 27  },	// UC_CHUNG_SHELLING_GUARDIAN
		{ 242130    , 100 },	// UC_ELSWORD_LORD_KNIGHT
		{ 242140    , 101 },	// UC_ELSWORD_RUNE_SLAYER
		{ 242150    , 112 },	// UC_ELSWORD_INFINITY_SWORD
		{ 242160    , 105 },	// UC_ARME_VOID_PRINCESS
		{ 242170    , 104 },	// UC_ARME_ELEMENTAL_MASTER
		{ 242180    , 113 },	// UC_ARME_DIMENSION_WITCH
		{ 242190    , 102 },	// UC_LIRE_WIND_SNEAKER
		{ 242200    , 103 },	// UC_LIRE_GRAND_ARCHER
		{ 242210    , 114 },	// UC_LIRE_NIGHT_WATCHER
		{ 242220    , 106 },	// UC_RAVEN_BLADE_MASTER
		{ 242230    , 107 },	// UC_RAVEN_RECKLESS_FIST
		{ 242240    , 115 },	// UC_RAVEN_VETERAN_COMMANDER
		{ 242250    , 108 },	// UC_EVE_CODE_NEMESIS
		{ 242260    , 109 },	// UC_EVE_CODE_EMPRESS
		{ 242270    , 116 },	// UC_EVE_BATTLE_SERAPH
		{ 242280    , 110 },	// UC_CHUNG_IRON_PALADIN
		{ 242290    , 111 },	// UC_CHUNG_DEADLY_CHASER
		{ 242300    , 117 },	// UC_CHUNG_TACTICAL_TROOPER
		{ 252751    , 118 },	// UC_ARA_SAKRA_DEVANAM
		{ 252752    , 119 },	// UC_ARA_YAMA_RAJA
		{ 252754    , 29  },	// UC_ELESIS_SABER_KNIGHT
		{ 252755    , 30  },	// UC_ELESIS_PYRO_KNIGHT
		{ 252756    , 31  },	// UC_ARA_LITTLE_DEVIL
		{ 252757    , 28  },	// UC_ARA_LITTLE_HSIEN
	};

	static const int ROW_COUNT = sizeof( ROWS ) / sizeof( ROWS[0] );
}

#endif SERV_IRUHADEV_OFFLINE
