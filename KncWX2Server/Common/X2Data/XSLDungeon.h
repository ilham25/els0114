#pragma once

#include "XSLDungeonStage.h"

#define DUNGEON_DEFAULT_LIMIT_PLAY_TIME 19999.0f

#define CASE_DEFENCE_DUNGEON \
	case CXSLDungeon::DI_DEFENCE_DUNGEON_ELDER_NORMAL:	\
	case CXSLDungeon::DI_DEFENCE_DUNGEON_ELDER_HARD:	\
	case CXSLDungeon::DI_DEFENCE_DUNGEON_ELDER_EXPERT:	\
	case CXSLDungeon::DI_DEFENCE_DUNGEON_BESMA_NORMAL:	\
	case CXSLDungeon::DI_DEFENCE_DUNGEON_BESMA_HARD:	\
	case CXSLDungeon::DI_DEFENCE_DUNGEON_BESMA_EXPERT:	\
	case CXSLDungeon::DI_DEFENCE_DUNGEON_ALTERA_NORMAL:	\
	case CXSLDungeon::DI_DEFENCE_DUNGEON_ALTERA_HARD:	\
	case CXSLDungeon::DI_DEFENCE_DUNGEON_ALTERA_EXPERT:	\
	case CXSLDungeon::DI_DEFENCE_DUNGEON_PEITA_NORMAL:	\
	case CXSLDungeon::DI_DEFENCE_DUNGEON_PEITA_HARD:	\
	case CXSLDungeon::DI_DEFENCE_DUNGEON_PEITA_EXPERT:	\
	case CXSLDungeon::DI_DEFENCE_DUNGEON_VELDER_NORMAL:	\
	case CXSLDungeon::DI_DEFENCE_DUNGEON_VELDER_HARD:	\
	case CXSLDungeon::DI_DEFENCE_DUNGEON_VELDER_EXPERT:	\
	case CXSLDungeon::DI_DEFENCE_DUNGEON_HAMEL_NORMAL:	\
	case CXSLDungeon::DI_DEFENCE_DUNGEON_HAMEL_HARD:	\
	case CXSLDungeon::DI_DEFENCE_DUNGEON_HAMEL_EXPERT:	\

class CXSLDungeon
{
public:
	enum DIFFICULTY_LEVEL
	{
		DL_NORMAL  = 0,
		DL_HARD,
		DL_EXPERT,

		DL_NUM,
	};

	//{{ 2010. 03. 18  최육사	비밀던전 헬모드	
	enum DUNGEON_TYPE
	{
		DT_INVALID = 0,
		DT_NORMAL, 
		DT_HENIR,
		DT_SECRET,
		//{{ 2013. 04. 15  김민성	어둠의 문 개편
//#ifdef SERV_NEW_DEFENCE_DUNGEON
		DT_DEFENCE_DUNGEON,
//#endif SERV_NEW_DEFENCE_DUNGEON
		//}}

		//{{ 2011. 03. 21  김민성	던전 난이도 보정
#ifdef SERV_DUNGEON_DIFFICULTY_REVISION
		DT_NUM,
#endif SERV_DUNGEON_DIFFICULTY_REVISION
		//}}
	};
	//}}

	//{{ 2010. 03. 18  최육사	비밀던전 헬모드	
	enum DUNGEON_MODE
	{
		DM_INVALID = 0,
		DM_HENIR_PRACTICE,
		DM_HENIR_CHALLENGE,
		DM_SECRET_NORMAL,
		DM_SECRET_HELL,
	};
	//}}

	//{{ 2010. 07. 01  최육사	내구도 개편
	enum DUNGEON_ENDURANCE
	{
		DE_LEAVE_ROOM_ENDURANCE_DAMAGE = 2000,
	};
	//}}

	//{{ 2010. 11. 8	최육사	코드 정리	
	enum RANK_TYPE
	{
		RT_NONE		= 0,
		RT_F,
		RT_E,
		RT_D,
		RT_C,
		RT_B,
		RT_A,
		RT_S,
		//{{ 2011. 02. 22	최육사	던전 랭크 개편
#ifdef SERV_DUNGEON_RANK_NEW
		RT_SS,
#endif SERV_DUNGEON_RANK_NEW
		//}}
	};
	//}}

	//{{ 2012. 05. 14	최육사	던전 몬스터 레벨 지정
#ifdef SERV_DUNGEON_NPC_LEVEL
	enum DUNGEON_ENUM
	{
		DE_CALC_AVERAGE_INCLUDE_LEVEL_GAP = 5,
	};
#endif SERV_DUNGEON_NPC_LEVEL
	//}}

	enum DUNGEON_ID
	{
		DI_NONE = 0,

		DI_RUBEN_EL_TREE_NORMAL		= 30000,
		
		DI_EL_FOREST_WEST_NORMAL		= 30010,
		DI_EL_FOREST_WEST_HARD,
		DI_EL_FOREST_WEST_EXPERT,
		DI_EL_FOREST_NORTH_NORMAL 		= 30020,
		DI_EL_FOREST_NORTH_HARD,
		DI_EL_FOREST_NORTH_EXPERT,		
		

		DI_TUTORIAL_ELSWORD				= 30030,
		DI_TUTORIAL_ARME,
		DI_TUTORIAL_LIRE,
		DI_TUTORIAL_RAVEN,
		DI_TUTORIAL_EVE,
		DI_TUTORIAL_CHUNG,


		DI_EL_FOREST_GATE_NORMAL		= 30040,
		DI_EL_FOREST_GATE_HARD,
		DI_EL_FOREST_GATE_EXPERT,

		DI_ELDER_BENDERS_CAVE_NORMAL	= 30050,
		DI_ELDER_BENDERS_CAVE_HARD,
		DI_ELDER_BENDERS_CAVE_EXPERT,

		DI_ELDER_NATURE_CAVE_NORMAL		= 30060,
		DI_ELDER_NATURE_CAVE_HARD,
		DI_ELDER_NATURE_CAVE_EXPERT,

		DI_ELDER_DRUID_FOREST_NORMAL	= 30070,
		DI_ELDER_DRUID_FOREST_HARD,
		DI_ELDER_DRUID_FOREST_EXPERT,


		DI_RUBEN_RUIN_OF_ELF_NORMAL		= 30080,
		DI_RUBEN_RUIN_OF_ELF_HARD,
		DI_RUBEN_RUIN_OF_ELF_EXPERT,

		DI_RUBEN_SWAMP_NORMAL			= 30090,
		DI_RUBEN_SWAMP_HARD,
		DI_RUBEN_SWAMP_EXPERT,

		DI_ELDER_WALLY_CASTLE_CENTER_NORMAL		= 30100,
		DI_ELDER_WALLY_CASTLE_CENTER_HARD,
		DI_ELDER_WALLY_CASTLE_CENTER_EXPERT,

		DI_ELDER_WALLY_CASTLE_ROOF_NORMAL		= 30110,
		DI_ELDER_WALLY_CASTLE_ROOF_HARD,
		DI_ELDER_WALLY_CASTLE_ROOF_EXPERT,

		DI_ELDER_BELOW_PASSAGE_NORMAL			= 30120,
		DI_ELDER_BELOW_PASSAGE_HARD,
		DI_ELDER_BELOW_PASSAGE_EXPERT,
		
		DI_MONSTER_TEST_NORMAL					= 30130,
		DI_MONSTER_TEST_HARD,
		DI_MONSTER_TEST_EXPERT,

		DI_BESMA_DRAGON_ROAD_NORMAL				= 30140,
		DI_BESMA_DRAGON_ROAD_HARD,
		DI_BESMA_DRAGON_ROAD_EXPERT,

		DI_BESMA_MINE_NORMAL					= 30150,
		DI_BESMA_MINE_HARD,
		DI_BESMA_MINE_EXPERT,

		DI_EL_FOREST_HELL_NORMAL				= 30160,
		DI_EL_FOREST_HELL_HARD,
		DI_EL_FOREST_HELL_EXPERT,


		DI_BESMA_LAKE_NORMAL					= 30170,
		DI_BESMA_LAKE_HARD,
		DI_BESMA_LAKE_EXPERT,


		DI_BESMA_DRAGON_NEST_NORMAL				= 30180,
		DI_BESMA_DRAGON_NEST_HARD,
		DI_BESMA_DRAGON_NEST_EXPERT,


		DI_BESMA_AIRSHIP_NORMAL					= 30190,
		DI_BESMA_AIRSHIP_HARD,
		DI_BESMA_AIRSHIP_EXPERT,

		DI_BESMA_LAKE_NIGHT_NORMAL				= 30200,
		DI_BESMA_LAKE_NIGHT_HARD,
		DI_BESMA_LAKE_NIGHT_EXPERT,

		DI_BESMA_MINE2_NORMAL					= 30210,
		DI_BESMA_MINE2_HARD,
		DI_BESMA_MINE2_EXPERT,

		DI_BESMA_CANYON_NORMAL					= 30220,
		DI_BESMA_CANYON_HARD,
		DI_BESMA_CANYON_EXPERT,

		DI_ELDER_WALLY_CASTLE_LAB_NORMAL		= 30230,
		DI_ELDER_WALLY_CASTLE_LAB_HARD,
		DI_ELDER_WALLY_CASTLE_LAB_EXPERT,

			
		DI_ALTERA_BATTLE_AIR_SHIP_NORMAL		= 30240,
		DI_ALTERA_BATTLE_AIR_SHIP_HARD,
		DI_ALTERA_BATTLE_AIR_SHIP_EXPERT,


		DI_ALTERA_PLAIN_RECYCLE_NORMAL		= 30250,
		DI_ALTERA_PLAIN_RECYCLE_HARD,
		DI_ALTERA_PLAIN_RECYCLE_EXPERT,

        DI_ALTERA_BELOW_TUNNEL_NORMAL       = 30260,
        DI_ALTERA_BELOW_TUNNEL_HARD,
        DI_ALTERA_BELOW_TUNNEL_EXPERT,

        DI_ALTERA_PLAIN_NORMAL              = 30270,
        DI_ALTERA_PLAIN_HARD,  
        DI_ALTERA_PLAIN_EXPERT,

        DI_ALTERA_NASOD_FACTORY_NORMAL      = 30280,
        DI_ALTERA_NASOD_FACTORY_HARD,
        DI_ALTERA_NASOD_FACTORY_EXPERT,

        DI_BESMA_SECRET_NORMAL                = 30290,
        DI_BESMA_SECRET_HARD,
        DI_BESMA_SECRET_EXPERT,

        DI_ALTERA_CORE_NORMAL               = 30300,
        DI_ALTERA_CORE_HARD,
        DI_ALTERA_CORE_EXPERT,

		DI_TEMP0								= 32000,
		DI_TEMP1,
		DI_TEMP2,
		DI_TEMP3,
		DI_TEMP4,
		DI_TEMP5,

        DI_TEST_DUNGEON_NORMAL                  = 32100,
        DI_TEST_DUNGEON_HARD,
        DI_TEST_DUNGEON_EXPERT,

        DI_TEST1_DUNGEON_NORMAL                 = 32200,
        DI_TEST1_DUNGEON_HARD,
        DI_TEST1_DUNGEON_EXPERT,


		DI_TEST2_DUNGEON_NORMAL                 = 32300,
		DI_TEST2_DUNGEON_HARD,
		DI_TEST2_DUNGEON_EXPERT,

		DI_TEST3_DUNGEON_NORMAL                 = 32400,
		DI_TEST3_DUNGEON_HARD,
		DI_TEST3_DUNGEON_EXPERT,




		DI_ELDER_HALLOWEEN_NORMAL               = 32500,
		DI_ELDER_HALLOWEEN_HARD,
		DI_ELDER_HALLOWEEN_EXPERT,



		DI_PEITA_OFFERINGS_NORMAL				= 32600,	
		DI_PEITA_OFFERINGS_HARD,	
		DI_PEITA_OFFERINGS_EXPERT,


		DI_PEITA_SPIRAL_CORRIDOR_NORMAL			= 32700,	
		DI_PEITA_SPIRAL_CORRIDOR_HARD,	
		DI_PEITA_SPIRAL_CORRIDOR_EXPERT,	


		DI_ELDER_NEWYEAR_NORMAL					= 32800,


		DI_ALTERA_SECRET_NORMAL					= 32900,
		DI_ALTERA_SECRET_HARD,					
		DI_ALTERA_SECRET_EXPERT,					



		DI_EVENT_KIDDAY_RUBEN					= 32910,		// 2009 어린이날 이벤트 던전
		DI_EVENT_KIDDAY_ELDER					= 32920,
		DI_EVENT_KIDDAY_BESMA					= 32930,
		DI_EVENT_KIDDAY_ALTERA					= 32940,

		//{{ kimhc // 페이타 던전 추가( 지하 예배당, 지하 정원 ) // 2009-05-14
		DI_PEITA_CHAPEL_NORMAL					= 32950,
		DI_PEITA_CHAPEL_HARD,
		DI_PEITA_CHAPEL_EXPERT,

		DI_PEITA_UNDER_GARDEN_NORMAL			= 32960,
		DI_PEITA_UNDER_GARDEN_HARD,	
		DI_PEITA_UNDER_GARDEN_EXPERT,
		//}} kimhc // 2009-05-14

		//{{ kimhc // 페이타 던전 추가( 신전의 심장부, 봉헌의 제단 ) // 2009-07-07
		DI_PEITA_TOWER_HEART_NORMAL				= 32970,
		DI_PEITA_TOWER_HEART_HARD, 
		DI_PEITA_TOWER_HEART_EXPERT,

		DI_PEITA_OFFERINGS_ALTER_NORMAL			= 32980,
		DI_PEITA_OFFERINGS_ALTER_HARD, 
		DI_PEITA_OFFERINGS_ALTER_EXPERT,
		//}} kimhc // 2009-07-07

		//{{ 2009. 10. 16  최육사	벨더 던전 (제 3거주지구, 희망의 다리)
		DI_VELDER_THIRD_DWELLING_NORMAL			= 33000,
		DI_VELDER_THIRD_DWELLING_HARD,
		DI_VELDER_THIRD_DWELLING_EXPERT,

		DI_VELDER_BRIDGE_HOPE_NORMAL			= 33010,
		DI_VELDER_BRIDGE_HOPE_HARD,
		DI_VELDER_BRIDGE_HOPE_EXPERT,
		//}}

		//{{ kimhc // 비밀던전 개편		// 2010-04-05
		DI_RUBEN_SECRET_COMMON					= 33020,
		DI_RUBEN_SECRET_HELL,

		DI_ELDER_SECRET_COMMON					= 33030,
		DI_ELDER_SECRET_HELL,

		DI_BESMA_SECRET_COMMON					= 33040,
		DI_BESMA_SECRET_HELL,

		DI_ALTERA_SECRET_COMMON                 = 33050,
		DI_ALTERA_SECRET_HELL,
		//}} kimhc // 비밀던전 개편		// 2010-04-05

		DI_VELDER_PALACE_ENTRANCE_NORMAL		= 33060,
		DI_VELDER_PALACE_ENTRANCE_HARD,
		DI_VELDER_PALACE_ENTRANCE_EXPERT,

		DI_VELDER_BRIDGE_BUNNING_NORMAL    = 33070,
		DI_VELDER_BRIDGE_BUNNING_HARD, 
		DI_VELDER_BRIDGE_BUNNING_EXPERT, 

		DI_VELDER_MARKET_NORMAL			= 33080,
		DI_VELDER_MARKET_HARD, 
		DI_VELDER_MARKET_EXPERT, 

		DI_VELDER_GATE_NORMAL      = 33090,
		DI_VELDER_GATE_HARD, 
		DI_VELDER_GATE_EXPERT, 

		DI_BATTLE_SHIP_VELDER					= 33100,
		DI_BATTLE_SHIP_HAMEL					= 33110,

		DI_HAMEL_OUTSKIRTS_RESIAM_NORMAL	= 33120, 
		DI_HAMEL_OUTSKIRTS_RESIAM_HARD,
		DI_HAMEL_OUTSKIRTS_RESIAM_EXPERT,

		DI_HAMEL_BREAK_RESIAM_NORMAL	= 33130,
		DI_HAMEL_BREAK_RESIAM_HARD,
		DI_HAMEL_BREAK_RESIAM_EXPERT,

		DI_HAMEL_ANCIENT_WATERWAY_NORMAL	= 33140,
		DI_HAMEL_ANCIENT_WATERWAY_HARD,
		DI_HAMEL_ANCIENT_WATERWAY_EXPERT,

		DI_HAMEL_ANCIENT_WATERWAY_CORE_NORMAL	= 33150,
		DI_HAMEL_ANCIENT_WATERWAY_CORE_HARD,
		DI_HAMEL_ANCIENT_WATERWAY_CORE_EXPERT,

		//{{ JHKang / 강정훈 / 2011.05.17 / 마그만타의 동굴(5번 던전)
		DI_HAMEL_CAVE_OF_MAGMANTA_NORMAL	= 33160,
		DI_HAMEL_CAVE_OF_MAGMANTA_HARD,
		DI_HAMEL_CAVE_OF_MAGMANTA_EXPERT,
		//}}

		//{{ JHKang / 강정훈 / 2011.7.9 / 얼어붙은 물의 신전, 얼어붙은 물의 전당
		DI_HAMEL_FROZEN_WATER_TEMPLE_NORMAL	= 33170,
		DI_HAMEL_FROZEN_WATER_TEMPLE_HARD,
		DI_HAMEL_FROZEN_WATER_TEMPLE_EXPERT,

		DI_HAMEL_FROZEN_WATER_HALL_NORMAL	= 33180,
		DI_HAMEL_FROZEN_WATER_HALL_HARD,
		DI_HAMEL_FROZEN_WATER_HALL_EXPERT, 
		//}}

		//{{ JHKang / 강정훈 / 2011.8.24 / 벨더 비밀 던전
		DI_VELDER_SECRET_COMMON	= 33190,
		DI_VELDER_SECRET_HELL,
		//}}

		//{{ mauntain / 김태환 / 2012.12.04 / 샌더 던전
		/// 메마른 샌더
		DI_SANDER_DRY_SANDER_NORMAL						 	= 33200,
		DI_SANDER_DRY_SANDER_HARD							= 33201,
		DI_SANDER_DRY_SANDER_EXPERT						 	= 33202,

		/// 가르파이 기암지대
		DI_SANDER_GARPAI_ROCK_NORMAL						= 33210,
		DI_SANDER_GARPAI_ROCK_HARD							= 33211,
		DI_SANDER_GARPAI_ROCK_EXPERT						= 33212,

		/// 트락 소굴
		DI_SANDER_DEN_OF_TROCK_NORMAL						= 33220,
		DI_SANDER_DEN_OF_TROCK_HARD							= 33221,
		DI_SANDER_DEN_OF_TROCK_EXPERT						= 33222,

		/// 칼루소 부족 마을
		DI_SANDER_KARUSO_VILLAGE_NORMAL						= 33230,
		DI_SANDER_KARUSO_VILLAGE_HARD						= 33231,
		DI_SANDER_KARUSO_VILLAGE_EXPERT						= 33232,
		//}}

		/// 샌틸러스 호
		DI_SANDER_SANTILUS_SHIP_NORMAL						= 33240,
		DI_SANDER_SANTILUS_SHIP_HARD						= 33241,
		DI_SANDER_SANTILUS_SHIP_EXPERT						= 33242,

		/// 베히모스의 심장부
		DI_SANDER_BEHIMOSS_HEART_NORMAL						= 33250,
		DI_SANDER_BEHIMOSS_HEART_HARD						= 33251,
		DI_SANDER_BEHIMOSS_HEART_EXPERT						= 33252,




		// 아케이드 모드 던전
		DI_ARCADE_RUBEN							= 38000,
		DI_ARCADE_ELDER							= 38010,
		DI_ARCADE_SEASON1						= 38020,

		DI_ELDER_HENIR_SPACE					= 38100,
		DI_BESMA_HENIR_SPACE					= 38110,
		DI_ALTERA_HENIR_SPACE					= 38120,
		DI_FEITA_HENIR_SPACE					= 38130,
		// kimhc // 벨더 헤니르 // 2009-10-27
		DI_VELDER_HENIR_SPACE					= 38140,
		DI_HAMEL_HENIR_SPACE                 	= 38150,

		/////////////////////////////	디펜스 던전 ///////////////////////////////////////
		DI_DEFENCE_DUNGEON_ELDER_NORMAL			= 38300,	
		DI_DEFENCE_DUNGEON_ELDER_HARD,
		DI_DEFENCE_DUNGEON_ELDER_EXPERT,

		DI_DEFENCE_DUNGEON_BESMA_NORMAL			= 38310,
		DI_DEFENCE_DUNGEON_BESMA_HARD,
		DI_DEFENCE_DUNGEON_BESMA_EXPERT,

		DI_DEFENCE_DUNGEON_ALTERA_NORMAL		= 38320,
		DI_DEFENCE_DUNGEON_ALTERA_HARD,
		DI_DEFENCE_DUNGEON_ALTERA_EXPERT,

		DI_DEFENCE_DUNGEON_PEITA_NORMAL			= 38330,
		DI_DEFENCE_DUNGEON_PEITA_HARD,
		DI_DEFENCE_DUNGEON_PEITA_EXPERT,

		DI_DEFENCE_DUNGEON_VELDER_NORMAL		= 38340,
		DI_DEFENCE_DUNGEON_VELDER_HARD,
		DI_DEFENCE_DUNGEON_VELDER_EXPERT,

		DI_DEFENCE_DUNGEON_HAMEL_NORMAL			= 38350,
		DI_DEFENCE_DUNGEON_HAMEL_HARD,
		DI_DEFENCE_DUNGEON_HAMEL_EXPERT,
		//////////////////////////////////////////////////////////////////////////////////

		DI_EVENT_VALENTINE_DAY					= 38400,	// 발렌타인 이벤트 던전

		// 훈련소용 던전
		DI_TRAINING_FREE						= 39000,			
		DI_TRAINING_COMMON_1					= 39001,					
		DI_TRAINING_COMMON_2,					

		DI_TRAINING_ELSWORD_0					= 39100,
		DI_TRAINING_ELSWORD_1,
		DI_TRAINING_ELSWORD_2,
		DI_TRAINING_ELSWORD_3,
		DI_TRAINING_ELSWORD_4,
		DI_TRAINING_ELSWORD_5,
		DI_TRAINING_ELSWORD_6,


		DI_TRAINING_ARME_0						= 39200,
		DI_TRAINING_ARME_1,
		DI_TRAINING_ARME_2,
		DI_TRAINING_ARME_3,
		DI_TRAINING_ARME_4,
		DI_TRAINING_ARME_5,
		DI_TRAINING_ARME_6,


		DI_TRAINING_LIRE_0						= 39300,
		DI_TRAINING_LIRE_1,
		DI_TRAINING_LIRE_2,
		DI_TRAINING_LIRE_3,
		DI_TRAINING_LIRE_4,
		DI_TRAINING_LIRE_5,
		DI_TRAINING_LIRE_6,

		DI_TRAINING_RAVEN_0						= 39400,
		DI_TRAINING_RAVEN_1,
		DI_TRAINING_RAVEN_2,
		DI_TRAINING_RAVEN_3,
		DI_TRAINING_RAVEN_4,
		DI_TRAINING_RAVEN_5,
		DI_TRAINING_RAVEN_6,

		DI_EVENT_TREE_DAY_ELDER					= 39500,	// 식목일 이벤트 던전
		DI_EVENT_TREE_DAY_BESMA					= 39510,
		DI_EVENT_TREE_DAY_ALTERA				= 39520,
		DI_EVENT_TREE_DAY_PEITA					= 39530,
		DI_EVENT_TREE_DAY_VELDER				= 39540,
		DI_EVENT_TREE_DAY_HAMEL					= 39550,

		DI_BATTLE_FIELD_TUTORIAL_ELSWORD		= 39600,	// 엘소드 튜토리얼
		DI_BATTLE_FIELD_TUTORIAL_AISHA,						// 아이샤 튜토리얼
		DI_BATTLE_FIELD_TUTORIAL_LENA,						// 레나   튜토리얼
		DI_BATTLE_FIELD_TUTORIAL_RAVEN,						// 레이븐 튜토리얼
		DI_BATTLE_FIELD_TUTORIAL_EVE,						// 이브   튜토리얼
		DI_BATTLE_FIELD_TUTORIAL_CHUNG,						// 청     튜토리얼
		DI_BATTLE_FIELD_TUTORIAL_ARA,						/// 아라 튜토리얼
		DI_BATTLE_FIELD_TUTORIAL_EL,						/// 엘리시스 튜토리얼

		/// 해외팀 공통이벤트 펫 던전
		DI_EVENT_PET_TRAINING_DUNGEON_NORMAL		= 41010,
		DI_EVENT_PET_TRAINING_DUNGEON_HARD			= 41011,

		/// 해외팀 미션 던전 이벤트
		DI_EVENT_MISSION_IMPOSSIBLE_NORMAL			= 41020,
		DI_EVENT_MISSION_IMPOSSIBLE_STAGE1_NORMAL	= 41030,
		DI_EVENT_MISSION_IMPOSSIBLE_STAGE2_NORMAL	= 41040,
		DI_EVENT_MISSION_IMPOSSIBLE_STAGE3_NORMAL	= 41050,
		DI_EVENT_MISSION_IMPOSSIBLE_STAGE4_NORMAL	= 41060,
		DI_EVENT_MISSION_IMPOSSIBLE_STAGE5_NORMAL	= 41070,
		DI_EVENT_MISSION_IMPOSSIBLE_STAGE6_NORMAL	= 41080,

//#ifdef EVENT_BIRTHDAY_DUNGEON
		DI_EVENT_BIRTHDAY_CAKE_DEFENCE_DUNGEON_NORMAL = 41090, 
//#endif //EVENT_BIRTHDAY_DUNGEON
//#ifdef SERV_EVENT_SONGKRAN_FESTIVAL_DUNGEON
		DI_EVENT_SONGKRAN_FESTIVAL_DUNGEON          = 41100,
//#endif //SERV_EVENT_SONGKRAN_FESTIVAL_DUNGEON

		/// 해외팀 나이트와쳐 이벤트 던전 2013.07.03 김창한
		DI_EVENT_NIGHT_WATCHER                      = 41110,

		/// 해외팀 천조 지역
		DI_CHINA_EMPEROR_CITY						= 41210,
		DI_CHINA_MAGIC_CITY							= 41220,
		DI_CHINA_ELF_CITY							= 41230,
		DI_CHINA_JANGAN								= 41240,
		DI_CHINA_FACHOK								= 41250,
		DI_CHINA_DEVIL								= 41260,

		/// 해외팀 2013 할로윈 던전
		DI_EVENT_HALLOWEEN_2013						= 41270,
		DI_EVENT_HALLOWEEN_2013_BOSS				= 41280,
		DI_EVENT_HALLOWEEN_2013_LINKER				= 41290,

		DI_END										= 41300,
	};

	struct DungeonData
	{
		DUNGEON_ID		m_DungeonID;
		std::wstring	m_DungeonName;
		std::wstring	m_DataFileName;

		DUNGEON_ID		m_RequireDungeonID;
		int				m_RequireItemID;
		int				m_RequireItemCount;
#ifdef SERV_ADDITIONAL_TICKET
		std::vector< std::pair< int, int > > m_vecAdditionalTicket;
#endif SERV_ADDITIONAL_TICKET

		int				m_RequireItemID2;
		int				m_RequireItemCount2;

		int				m_RequireSpirit;

		//{{ 2012. 05. 22	최육사	던전 입장 아이템 레벨
#ifdef SERV_DUNGEON_REQUIRE_ITEM_LEVEL
		int				m_RequireItemLevel;
#endif SERV_DUNGEON_REQUIRE_ITEM_LEVEL
		//}}

		int				m_MinLevel;
		int				m_MaxLevel;

		//{{ 2012. 05. 10	최육사	배틀필드 시스템
#ifdef SERV_DUNGEON_NPC_LEVEL
		int				m_NpcLevel;
#endif SERV_DUNGEON_NPC_LEVEL
		//}}

		DIFFICULTY_LEVEL m_eDifficulty;

		bool			m_bNormalOnly;	// 노말 난이도만 고를 수 있음
		float			m_fTimeLimit;
		//{{ 2010. 04. 26  최육사	비밀던전 헬모드
#ifdef SERV_SECRET_HELL
		float			m_fPoisonTimeLimit;
		float			m_fExtraStagePoisonTimeLimit;
#endif SERV_SECRET_HELL
		//}}
#ifdef SERV_DUNGEON_OPTION_IN_LUA
		bool			m_bLevelEqualized;
		bool			m_bDamageEqualized;
		bool			m_bEventDungeon;
		unsigned short	m_usFixedMembers;
#endif SERV_DUNGEON_OPTION_IN_LUA

		DUNGEON_TYPE	m_eDungeonType;
		bool			m_bRelativeMonsterLevel;

		//{{ 2011. 04. 13  김민성  글로벌 서버 추가
#ifdef SERV_INSERT_GLOBAL_SERVER
		int				m_iDefenseDungeonClearEXP;
#endif SERV_INSERT_GLOBAL_SERVER
		//}} 2011. 04. 13  김민성  글로벌 서버 추가

#ifdef SERV_NEW_EVENT_TYPES
		bool			m_bSwitchingWithEventInfo;
#endif SERV_NEW_EVENT_TYPES

#ifdef SERV_LIMITED_DUNGEON_PLAY_TIMES
		int				m_iLimitedPlayTimes;
		int				m_iLimitedClearTimes;
#endif SERV_LIMITED_DUNGEON_PLAY_TIMES
	};

public:
	CXSLDungeon( const DungeonData& dungeonData );
	~CXSLDungeon();

	bool	OpenScriptFile();

	const CXSLDungeon::DungeonData& GetDungeonData() const;

	//////////////////////////////////////////////////////////////////////////
	//BEGIN : SERVER 에서만 사용되는 함수
	void	Clear();

#ifdef SERV_CREATED_NPC_LIMITED_DROPS
	int		GetNPCDropTime( IN const int iStageIndex, IN const int iSubStageIndex, IN const int iNPCID ) const;
#endif SERV_CREATED_NPC_LIMITED_DROPS

	//{{ 2012. 05. 10	최육사	던전 몬스터 레벨 지정
#ifdef SERV_DUNGEON_NPC_LEVEL
	bool	GetNPCData( IN const int iStageIndex, IN const int iRelativeMonsterLevel, OUT KEGS_DUNGEON_STAGE_LOAD_NOT& kNot, OUT bool& bIsBossStage ) const;
#else
	bool	GetNPCData( IN const int iStageIndex, IN const int iHighestUserLevel, OUT KEGS_DUNGEON_STAGE_LOAD_NOT& kNot, OUT bool& bIsBossStage ) const;
#endif SERV_DUNGEON_NPC_LEVEL
	//}}	

	int		GetStartPosByRandom( IN int iStageIndex, IN int iSubStageIndex ) const;
	//{{ 2010. 04. 26  최육사	비밀던전 헬모드
#ifdef SERV_SECRET_HELL
	void	GetExtraNpcInfo( IN int iStageIndex, IN int iSubStageIndex, OUT int& iStartPos, OUT bool& bIsRight ) const;
#endif SERV_SECRET_HELL
	//}}
	int		GetSubStageIndexByRandom( IN int iStageIndex );

#ifdef SERV_STAGE_CLEAR_IN_SERVER
	int		GetSecretStageEnteringEvent( IN const int iStageIndex, IN const int iSubStageIndex, IN int iClearConditionIndex ) const;
	bool	GetNextStage( OUT CXSLDungeonSubStage::NextStageData& kNextStageData, IN const int iStageIndex, IN const int iSubStageIndex, IN int iClearConditionIndex, IN int iSecretPadIndex ) const;
#endif SERV_STAGE_CLEAR_IN_SERVER

	//{{ 2010. 03. 24  최육사	비밀던전 헬모드
	CXSLDungeonStage::STAGE_TYPE GetStageType( IN u_int uiStageIndex ) const;
	//}}
	//{{ 2010. 11. 8	최육사	코드 정리	
	static const wchar_t* GetRankString( IN RANK_TYPE eRankType );
	//}}

	static bool	IsTutorialDungeon( IN int iDungeonID );
	static bool	IsTCDungeon( IN int iDungeonID );
	static bool IsArcadeDungeon( IN int iDungeonID );
	static bool IsArcadeSeasonDungeon( IN int iDungeonID );
	static bool IsRubenDungeon( IN int iDungeonID );
	static bool IsElderDungeon( IN int iDungeonID );
	static bool IsBesmaDungeon( IN int iDungeonID );
	static bool IsAlteraDungeon( IN int iDungeonID );
	static bool IsPeitaDungeon( IN int iDungeonID );
	static bool IsVelderDungeon( IN int iDungeonID );
	static bool IsHamelDungeon( IN int iDungeonID );
	//{{ 2012. 10. 29	박세훈	샌더 마을 추가
#ifdef SERV_VILLAGE_SANDER
	static bool IsSanderDungeon( IN int iDungeonID );
#endif SERV_VILLAGE_SANDER
	//}}

#ifdef SERV_DUNGEON_OPTION_IN_LUA
	static bool IsFixedMembers( IN int iDungeonID );
	static short GetFixedMembers( IN int iDungeonID );
#endif SERV_DUNGEON_OPTION_IN_LUA

	//{{ 2009. 4. 24  최육사	이벤트 던전
	static bool IsEventDungeon( IN int iDungeonID );
	//}}
	//{{ 2009. 7. 26  최육사	헤니르 시공
	static bool IsHenirDungeon( IN int iDungeonID );
	//}}
	static bool IsSinglePlayDungeon( IN int iDungeonID );
    //{{ 2009. 3. 3  최육사		드롭지역 제한
	static bool IsItemDropDungeon( IN int iItemID, IN int iDungeonID );
	//}}
	//{{ 2010. 03. 18  최육사	비밀던전 헬모드
	static bool IsValidDungeonMode( DUNGEON_MODE eDungeonMode );
	//}}	
	//{{ 2009. 12. 18  최육사	서버군 확장
	static bool IsScriptCheckDungeon( IN int iDungeonID );
	//}}
	//{{ 2010. 03. 25  최육사	비밀던전 헬모드
	static bool IsSecretDungeon( IN int iDungeonID );
	//}}
	//{{ 2010. 12. 28	최육사	하멜 마을 추가
#ifdef SERV_SHIP_DUNGEON
	static bool IsShipDungeon( IN int iDungeonID );
#endif SERV_SHIP_DUNGEON
	//}}
	//{{ 2011. 04. 13  김민성  글로벌 서버 추가
#ifdef SERV_INSERT_GLOBAL_SERVER
	static bool IsDefenceDungeon( IN int iDungeonID );
	int	GetDefeseDungeonClearEXP()						{ return m_sDungeonData.m_iDefenseDungeonClearEXP; }
#endif SERV_INSERT_GLOBAL_SERVER
	//}} 2011. 04. 13  김민성  글로벌 서버 추가

	//{{ 2013. 02. 01  이벤트 발렌타인 던전 - 김민성
#ifdef SERV_EVENT_VALENTINE_DUNGEON
	static bool IsValentineDungeon( IN int iDungeonID );
#endif SERV_EVENT_VALENTINE_DUNGEON
	//}

	//{{ 2010. 04. 30  최육사	비밀던전 헬모드
#ifdef SERV_SECRET_HELL 
	static float CheckDungeonBalRateByNpcLv( IN const int iUnitLevel, IN const int iNpcLv );
#endif SERV_SECRET_HELL
	//}}
	//{{ 2012. 06. 29	최육사	배틀필드 시스템
#ifdef SERV_BATTLE_FIELD_SYSTEM
	static float GetMonsterLevelBalanceRate( IN const int iUnitLevel, IN const int iNpcLv );
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}

	//END :
	//////////////////////////////////////////////////////////////////////////



private:
	DungeonData										m_sDungeonData;
	std::vector< CXSLDungeonStage::StageData* >		m_vecStageData;
};
