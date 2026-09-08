#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-04 (rebuilt 2026-09-08 - two families, two rules)
// Description: GENERATED - do not hand-edit. Every cash item that changes a
//              character's class, the CX2Unit::UNIT_CLASS it changes to, and
//              the two tables the eligibility gates need.
//
//              These items are the one thing the cash shop sells that must
//              never reach the bag: on live the claim is intercepted before the
//              insert (GSUserCashShop.cpp:2887, KGSUser::OnPickUpAck) and turned
//              into a class change, so the item is consumed by the act of
//              claiming it and no inventory row is ever written. Offline they
//              were landing in the bag as ordinary items that did nothing,
//              ever - which is exactly what "the class change item isn't
//              working" looks like from the inside.
//
//////////////////////////////////////////////////////////////////////////
// TWO FAMILIES, AND THEY DO NOT MEAN THE SAME THING
//
//              This is the whole reason the file was rebuilt. The first pass
//              found only the newer family and then applied the older family's
//              rule to it, which refused every legitimate sibling swap:
//
//                CASH class change refused: item 242080 advances to class 19,
//                which comes after class 5, but unitUID=19 is class 18
//
//              - JOB ADVANCE (m_bJobAdvance = true): CXSLItem::CI_CHANGE_JOB_*,
//                2009-2011. IDs 214410-214500, 216420/30, 217850/60,
//                217920/30, 217950/60, 217970/80, 229460/70, 235120/30,
//                242310-242360, 252640-252690, 264380-264394.
//                Mapped by GetCashItemChangeUnitClass (XSLItem.cpp:63).
//                MEANING: advance one step. Gate is
//                GetUnitClassDownGrade( new ) == now.
//
//              - CLASS CHANGE (m_bJobAdvance = false): CI_CLASS_CHANGE_*,
//                2011.11.21 on. IDs 241950-242300, 252751-252757.
//                Mapped by GetClassChangeCashItem (XSLItem.cpp:950).
//                MEANING: re-pick a class you already have, at the tier you are
//                already on. Gates are IsInitNormalJob( now ) == false - a base
//                class may not use one at all - and then
//                KClassChangeTable::CompareUnitClass( now, new ).
//
//              Both are claimed down the same path, because OnPickUpAck's test
//              is IsJobChangeCashItem, which is the OLD family's mapper
//              (XSLItem.h:1451). The per-family gates are the ones in
//              GSUserCashShop_Global.cpp:1108-1163.
//
//////////////////////////////////////////////////////////////////////////
// HOW THE ROWS WERE CHECKED
//
//              Each family was built from two independent sources that have to
//              agree, and no source was derived from another:
//
//                CLASS_CHANGE   X2Define.h's UNIT_CLASS_CHANGE_*_ITEM_ID
//                (42 rows)      x GetClassChangeCashItem (XSLItem.cpp:950)
//
//                JOB_ADVANCE    GetCashItemChangeUnitClass (XSLItem.cpp:63)
//                (40 rows)      x CX2CashShop::GetClassNameByClassChangeItem
//                               (X2CashShop.cpp:7669, which covers BOTH
//                               families) - 10 shared IDs, zero disagreements
//
//              82 rows, no ID in both families, no conflicting target class.
//              Every #ifdef in every switch was resolved BY COMPILER PROBE,
//              never by reading: all on except SERV_NEW_CHARACTER_EL /
//              NEW_CHARACTER_EL (Elesis). Reading them by eye gives six items
//              the wrong class, because each transcendence case has an #else
//              naming a UC_*_2 placeholder this build does not compile.
//
//////////////////////////////////////////////////////////////////////////
// PARENT_ROWS - CXSLUnit::GetUnitClassDownGrade (XSLUnit.cpp:176)
//
//              A base class maps to itself. Used for the JOB_ADVANCE gate
//              directly, and for the CLASS_CHANGE gate indirectly: walking the
//              chain gives both the character (the root) and the tier (the
//              length), which is all CompareUnitClass actually needs.
//
//              CompareUnitClass compares group IDs that live in a server Lua
//              file, KncWX2Server/ServerResource/US/ClassChangeTable.lua, which
//              is NOT packed into data036.kom. It does not need to be: the file
//              assigns group = character x 10 + tier (11 = Elsword 1st, 12 =
//              Elsword 2nd, 21 = Aisha 1st, ...), so "same group" is exactly
//              "same character and same tier", which the parent chain already
//              yields. That equivalence was VERIFIED against the studio file
//              rather than assumed - all 44 of its rows, of which the 40 this
//              build has classes for map 1:1 onto (root, tier) with no group
//              split across two keys and no key split across two groups. The
//              4 skipped rows are Elesis, whose flag is off here.
//
//              Classes 29/30 (Elesis) have no parent row at all, so both gates
//              must read "no row" as "do not refuse", never as "refuse" - the
//              newer family still sells tickets for those two classes.
//
// SIDE_STEP_ROWS - CompareUnitClass's six hardcoded exceptions
//              (ClassChangeTable.cpp:738-779)
//
//              Each lets a character move INTO the third-branch 1st class even
//              when the groups differ - i.e. a 2nd-class character may take a
//              Sheath Knight / Battle Magician / Trapping Ranger / Weapon Taker
//              / Electra / Shelling Guardian ticket. Those branches were added
//              to the game after the 2nd classes existed, so without this a
//              Lord Knight could never reach Sheath Knight. The required base
//              grade in each case is exactly that class's own parent, so the
//              condition reduces to "same character".
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

namespace X2OfflineClassChangeSeed
{
	struct KRow
	{
		int		m_iItemID;			///< the cash item
		int		m_iUnitClass;		///< CX2Unit::UNIT_CLASS it changes to
		bool	m_bJobAdvance;		///< true = CI_CHANGE_JOB_* (advance one step),
									///< false = CI_CLASS_CHANGE_* (re-pick at this tier)
	};

	static const KRow ROWS[] =
	{
		{ 214410    , 10 , true  },	// UC_ELSWORD_KNIGHT
		{ 214420    , 11 , true  },	// UC_ELSWORD_MAGIC_KNIGHT
		{ 214430    , 14 , true  },	// UC_ARME_HIGH_MAGICIAN
		{ 214440    , 15 , true  },	// UC_ARME_DARK_MAGICIAN
		{ 214450    , 12 , true  },	// UC_LIRE_COMBAT_RANGER
		{ 214460    , 13 , true  },	// UC_LIRE_SNIPING_RANGER
		{ 214470    , 16 , true  },	// UC_RAVEN_SOUL_TAKER
		{ 214480    , 17 , true  },	// UC_RAVEN_OVER_TAKER
		{ 214490    , 18 , true  },	// UC_EVE_EXOTIC_GEAR
		{ 214500    , 19 , true  },	// UC_EVE_ARCHITECTURE
		{ 216420    , 100, true  },	// UC_ELSWORD_LORD_KNIGHT
		{ 216430    , 101, true  },	// UC_ELSWORD_RUNE_SLAYER
		{ 217850    , 102, true  },	// UC_LIRE_WIND_SNEAKER
		{ 217860    , 103, true  },	// UC_LIRE_GRAND_ARCHER
		{ 217920    , 104, true  },	// UC_ARME_ELEMENTAL_MASTER
		{ 217930    , 105, true  },	// UC_ARME_VOID_PRINCESS
		{ 217950    , 106, true  },	// UC_RAVEN_BLADE_MASTER
		{ 217960    , 107, true  },	// UC_RAVEN_RECKLESS_FIST
		{ 217970    , 108, true  },	// UC_EVE_CODE_NEMESIS
		{ 217980    , 109, true  },	// UC_EVE_CODE_EMPRESS
		{ 229460    , 20 , true  },	// UC_CHUNG_FURY_GUARDIAN
		{ 229470    , 21 , true  },	// UC_CHUNG_SHOOTING_GUARDIAN
		{ 235120    , 110, true  },	// UC_CHUNG_IRON_PALADIN
		{ 235130    , 111, true  },	// UC_CHUNG_DEADLY_CHASER
		{ 241950    , 10 , false },	// UC_ELSWORD_KNIGHT
		{ 241960    , 11 , false },	// UC_ELSWORD_MAGIC_KNIGHT
		{ 241970    , 22 , false },	// UC_ELSWORD_SHEATH_KNIGHT
		{ 241980    , 14 , false },	// UC_ARME_HIGH_MAGICIAN
		{ 241990    , 15 , false },	// UC_ARME_DARK_MAGICIAN
		{ 242000    , 23 , false },	// UC_ARME_BATTLE_MAGICIAN
		{ 242010    , 12 , false },	// UC_LIRE_COMBAT_RANGER
		{ 242020    , 13 , false },	// UC_LIRE_SNIPING_RANGER
		{ 242030    , 24 , false },	// UC_LIRE_TRAPPING_RANGER
		{ 242040    , 16 , false },	// UC_RAVEN_SOUL_TAKER
		{ 242050    , 17 , false },	// UC_RAVEN_OVER_TAKER
		{ 242060    , 25 , false },	// UC_RAVEN_WEAPON_TAKER
		{ 242070    , 18 , false },	// UC_EVE_EXOTIC_GEAR
		{ 242080    , 19 , false },	// UC_EVE_ARCHITECTURE
		{ 242090    , 26 , false },	// UC_EVE_ELECTRA
		{ 242100    , 20 , false },	// UC_CHUNG_FURY_GUARDIAN
		{ 242110    , 21 , false },	// UC_CHUNG_SHOOTING_GUARDIAN
		{ 242120    , 27 , false },	// UC_CHUNG_SHELLING_GUARDIAN
		{ 242130    , 100, false },	// UC_ELSWORD_LORD_KNIGHT
		{ 242140    , 101, false },	// UC_ELSWORD_RUNE_SLAYER
		{ 242150    , 112, false },	// UC_ELSWORD_INFINITY_SWORD
		{ 242160    , 105, false },	// UC_ARME_VOID_PRINCESS
		{ 242170    , 104, false },	// UC_ARME_ELEMENTAL_MASTER
		{ 242180    , 113, false },	// UC_ARME_DIMENSION_WITCH
		{ 242190    , 102, false },	// UC_LIRE_WIND_SNEAKER
		{ 242200    , 103, false },	// UC_LIRE_GRAND_ARCHER
		{ 242210    , 114, false },	// UC_LIRE_NIGHT_WATCHER
		{ 242220    , 106, false },	// UC_RAVEN_BLADE_MASTER
		{ 242230    , 107, false },	// UC_RAVEN_RECKLESS_FIST
		{ 242240    , 115, false },	// UC_RAVEN_VETERAN_COMMANDER
		{ 242250    , 108, false },	// UC_EVE_CODE_NEMESIS
		{ 242260    , 109, false },	// UC_EVE_CODE_EMPRESS
		{ 242270    , 116, false },	// UC_EVE_BATTLE_SERAPH
		{ 242280    , 110, false },	// UC_CHUNG_IRON_PALADIN
		{ 242290    , 111, false },	// UC_CHUNG_DEADLY_CHASER
		{ 242300    , 117, false },	// UC_CHUNG_TACTICAL_TROOPER
		{ 242310    , 22 , true  },	// UC_ELSWORD_SHEATH_KNIGHT
		{ 242320    , 23 , true  },	// UC_ARME_BATTLE_MAGICIAN
		{ 242330    , 24 , true  },	// UC_LIRE_TRAPPING_RANGER
		{ 242340    , 25 , true  },	// UC_RAVEN_WEAPON_TAKER
		{ 242350    , 26 , true  },	// UC_EVE_ELECTRA
		{ 242360    , 27 , true  },	// UC_CHUNG_SHELLING_GUARDIAN
		{ 252640    , 112, true  },	// UC_ELSWORD_INFINITY_SWORD
		{ 252650    , 113, true  },	// UC_ARME_DIMENSION_WITCH
		{ 252660    , 114, true  },	// UC_LIRE_NIGHT_WATCHER
		{ 252670    , 115, true  },	// UC_RAVEN_VETERAN_COMMANDER
		{ 252680    , 116, true  },	// UC_EVE_BATTLE_SERAPH
		{ 252690    , 117, true  },	// UC_CHUNG_TACTICAL_TROOPER
		{ 252751    , 118, false },	// UC_ARA_SAKRA_DEVANAM
		{ 252752    , 119, false },	// UC_ARA_YAMA_RAJA
		{ 252754    , 29 , false },	// UC_ELESIS_SABER_KNIGHT
		{ 252755    , 30 , false },	// UC_ELESIS_PYRO_KNIGHT
		{ 252756    , 31 , false },	// UC_ARA_LITTLE_DEVIL
		{ 252757    , 28 , false },	// UC_ARA_LITTLE_HSIEN
		{ 264380    , 28 , true  },	// UC_ARA_LITTLE_HSIEN
		{ 264390    , 118, true  },	// UC_ARA_SAKRA_DEVANAM
		{ 264393    , 31 , true  },	// UC_ARA_LITTLE_DEVIL
		{ 264394    , 119, true  },	// UC_ARA_YAMA_RAJA
	};

	static const int ROW_COUNT = sizeof( ROWS ) / sizeof( ROWS[0] );

	struct KParentRow
	{
		int	m_iUnitClass;		///< CX2Unit::UNIT_CLASS
		int	m_iParentClass;		///< the class it advanced FROM; itself for a base class
	};

	static const KParentRow PARENT_ROWS[] =
	{
		{ 1   , 1    },	// UC_ELSWORD_SWORDMAN
		{ 2   , 2    },	// UC_ARME_VIOLET_MAGE
		{ 3   , 3    },	// UC_LIRE_ELVEN_RANGER
		{ 4   , 4    },	// UC_RAVEN_FIGHTER
		{ 5   , 5    },	// UC_EVE_NASOD
		{ 6   , 6    },	// UC_CHUNG_IRON_CANNON
		{ 7   , 7    },	// UC_ARA_MARTIAL_ARTIST
		{ 10  , 1    },	// UC_ELSWORD_KNIGHT
		{ 11  , 1    },	// UC_ELSWORD_MAGIC_KNIGHT
		{ 12  , 3    },	// UC_LIRE_COMBAT_RANGER
		{ 13  , 3    },	// UC_LIRE_SNIPING_RANGER
		{ 14  , 2    },	// UC_ARME_HIGH_MAGICIAN
		{ 15  , 2    },	// UC_ARME_DARK_MAGICIAN
		{ 16  , 4    },	// UC_RAVEN_SOUL_TAKER
		{ 17  , 4    },	// UC_RAVEN_OVER_TAKER
		{ 18  , 5    },	// UC_EVE_EXOTIC_GEAR
		{ 19  , 5    },	// UC_EVE_ARCHITECTURE
		{ 20  , 6    },	// UC_CHUNG_FURY_GUARDIAN
		{ 21  , 6    },	// UC_CHUNG_SHOOTING_GUARDIAN
		{ 22  , 1    },	// UC_ELSWORD_SHEATH_KNIGHT
		{ 23  , 2    },	// UC_ARME_BATTLE_MAGICIAN
		{ 24  , 3    },	// UC_LIRE_TRAPPING_RANGER
		{ 25  , 4    },	// UC_RAVEN_WEAPON_TAKER
		{ 26  , 5    },	// UC_EVE_ELECTRA
		{ 27  , 6    },	// UC_CHUNG_SHELLING_GUARDIAN
		{ 28  , 7    },	// UC_ARA_LITTLE_HSIEN
		{ 31  , 7    },	// UC_ARA_LITTLE_DEVIL
		{ 100 , 10   },	// UC_ELSWORD_LORD_KNIGHT
		{ 101 , 11   },	// UC_ELSWORD_RUNE_SLAYER
		{ 102 , 12   },	// UC_LIRE_WIND_SNEAKER
		{ 103 , 13   },	// UC_LIRE_GRAND_ARCHER
		{ 104 , 14   },	// UC_ARME_ELEMENTAL_MASTER
		{ 105 , 15   },	// UC_ARME_VOID_PRINCESS
		{ 106 , 16   },	// UC_RAVEN_BLADE_MASTER
		{ 107 , 17   },	// UC_RAVEN_RECKLESS_FIST
		{ 108 , 18   },	// UC_EVE_CODE_NEMESIS
		{ 109 , 19   },	// UC_EVE_CODE_EMPRESS
		{ 110 , 20   },	// UC_CHUNG_IRON_PALADIN
		{ 111 , 21   },	// UC_CHUNG_DEADLY_CHASER
		{ 112 , 22   },	// UC_ELSWORD_INFINITY_SWORD
		{ 113 , 23   },	// UC_ARME_DIMENSION_WITCH
		{ 114 , 24   },	// UC_LIRE_NIGHT_WATCHER
		{ 115 , 25   },	// UC_RAVEN_VETERAN_COMMANDER
		{ 116 , 26   },	// UC_EVE_BATTLE_SERAPH
		{ 117 , 27   },	// UC_CHUNG_TACTICAL_TROOPER
		{ 118 , 28   },	// UC_ARA_SAKRA_DEVANAM
		{ 119 , 31   },	// UC_ARA_YAMA_RAJA
	};

	static const int PARENT_ROW_COUNT = sizeof( PARENT_ROWS ) / sizeof( PARENT_ROWS[0] );

	/// The six classes a CLASS_CHANGE ticket may move into from a DIFFERENT
	/// group, provided the character matches. See SIDE_STEP_ROWS above.
	static const int SIDE_STEP_CLASSES[] =
	{
		22  ,	// UC_ELSWORD_SHEATH_KNIGHT
		23  ,	// UC_ARME_BATTLE_MAGICIAN
		24  ,	// UC_LIRE_TRAPPING_RANGER
		25  ,	// UC_RAVEN_WEAPON_TAKER
		26  ,	// UC_EVE_ELECTRA
		27  ,	// UC_CHUNG_SHELLING_GUARDIAN
	};

	static const int SIDE_STEP_COUNT = sizeof( SIDE_STEP_CLASSES ) / sizeof( SIDE_STEP_CLASSES[0] );
}

#endif SERV_IRUHADEV_OFFLINE
