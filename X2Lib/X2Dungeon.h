#pragma once
// 던전 추가할 때 작업해야할 파일 리스트
//		x2dungeon.h
//		xsldungeon.h
//		dungeondata.lua
//		channels.lua		-- 이제 사용안함
//		dlg_map_local_velder_north.lua
//		dlg_map_local_velder_north_icon.lua
////	dlg_map_local_velder_icon_info.lua

#define CASE_DEFENCE_DUNGEON \
	case CX2Dungeon::DI_DEFENCE_DUNGEON_ELDER_NORMAL:	\
	case CX2Dungeon::DI_DEFENCE_DUNGEON_ELDER_HARD:		\
	case CX2Dungeon::DI_DEFENCE_DUNGEON_ELDER_EXPERT:	\
// 	case CX2Dungeon::DI_DEFENCE_DUNGEON_BESMA_NORMAL:	\
// 	case CX2Dungeon::DI_DEFENCE_DUNGEON_BESMA_HARD:		\
// 	case CX2Dungeon::DI_DEFENCE_DUNGEON_BESMA_EXPERT:	\
// 	case CX2Dungeon::DI_DEFENCE_DUNGEON_ALTERA_NORMAL:	\
// 	case CX2Dungeon::DI_DEFENCE_DUNGEON_ALTERA_HARD:	\
// 	case CX2Dungeon::DI_DEFENCE_DUNGEON_ALTERA_EXPERT:	\
// 	case CX2Dungeon::DI_DEFENCE_DUNGEON_PEITA_NORMAL:	\
// 	case CX2Dungeon::DI_DEFENCE_DUNGEON_PEITA_HARD:		\
// 	case CX2Dungeon::DI_DEFENCE_DUNGEON_PEITA_EXPERT:	\
// 	case CX2Dungeon::DI_DEFENCE_DUNGEON_VELDER_NORMAL:	\
// 	case CX2Dungeon::DI_DEFENCE_DUNGEON_VELDER_HARD:	\
// 	case CX2Dungeon::DI_DEFENCE_DUNGEON_VELDER_EXPERT:	\
// 	case CX2Dungeon::DI_DEFENCE_DUNGEON_HAMEL_NORMAL:	\
// 	case CX2Dungeon::DI_DEFENCE_DUNGEON_HAMEL_HARD:		\
// 	case CX2Dungeon::DI_DEFENCE_DUNGEON_HAMEL_EXPERT:	\
	



// 주위: mapdata.lua 에는 local, village 아이디만 추가, 던전은 관련 없음


class CX2Dungeon
{
	public:
		enum DIFFICULTY_LEVEL
		{
			DL_NORMAL  = 0,
			DL_HARD,
			DL_EXPERT,
		};



#ifdef HENIR_TEST
		enum DUNGEON_TYPE
		{
			DT_INVALID,
			DT_NORMAL, 
			DT_HENIR,
			DT_SECRET,
		};

		enum DUNGEON_MODE
		{
			DM_INVALID,
			DM_HENIR_PRACTICE,
			DM_HENIR_CHALLENGE,
			DM_SECRET_NORMAL,
			DM_SECRET_HELL,
		};
#endif HENIR_TEST



		enum DUNGEON_ID
		{
			DI_NONE = 0,

			DI_TOOL_DUNGEON_0				= 10000,
			DI_TOOL_DUNGEON_1,
			DI_TOOL_DUNGEON_2,
			DI_TOOL_DUNGEON_3,
			DI_TOOL_DUNGEON_4,
			DI_TOOL_DUNGEON_5,
			DI_TOOL_DUNGEON_6,
			DI_TOOL_DUNGEON_7,
			DI_TOOL_DUNGEON_8,
			DI_TOOL_DUNGEON_9,			

			DI_RUBEN_EL_TREE_NORMAL		= 30000,	// 엘의나무
			
			DI_EL_FOREST_WEST_NORMAL		= 30010,	// 엘의나무 서쪽
			DI_EL_FOREST_WEST_HARD,
			DI_EL_FOREST_WEST_EXPERT,
			DI_EL_FOREST_NORTH_NORMAL 		= 30020,	// 엘의나무 북쪽
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


			DI_RUBEN_RUIN_OF_ELF_NORMAL		= 30080,	// 원숭이숲
			DI_RUBEN_RUIN_OF_ELF_HARD,
			DI_RUBEN_RUIN_OF_ELF_EXPERT,

			DI_RUBEN_SWAMP_NORMAL		= 30090,	// 엘의나무 동쪽
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

            DI_BESMA_SECRET_NORMAL              = 30290,
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

			//{{ kimhc // 벨더 던전 (제 3거주지구, 희망의 다리) // 2009-10-16
			DI_VELDER_THIRD_DWELLING_NORMAL			= 33000,
			DI_VELDER_THIRD_DWELLING_HARD, 
			DI_VELDER_THIRD_DWELLING_EXPERT,

			DI_VELDER_BRIDGE_HOPE_NORMAL			= 33010,
			DI_VELDER_BRIDGE_HOPE_HARD, 
			DI_VELDER_BRIDGE_HOPE_EXPERT,
			//}} kimhc // 벨더 던전 (제 3거주지구, 희망의 다리) // 2009-10-16

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

			DI_VELDER_PALACE_ENTRANCE_NORMAL			= 33060,
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

			DI_BATTLE_SHIP_VELDER		= 33100,
			DI_BATTLE_SHIP_HAMEL		= 33110,

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


			// kimjh / 김종훈 / 2013.05.23 / 샌더 5번 던전
			DI_SANDER_SANTILUS_SHIP_NORMAL						= 33240,
			DI_SANDER_SANTILUS_SHIP_HARD						= 33241,
			DI_SANDER_SANTILUS_SHIP_EXPERT						= 33242,

			// kimjh / 김종훈 / 2013.05.23 / 샌더 6번 던전
			DI_SANDER_BEHIMOSS_HEART_NORMAL						= 33250,
			DI_SANDER_BEHIMOSS_HEART_HARD						= 33251,
			DI_SANDER_BEHIMOSS_HEART_EXPERT						= 33252,


			// note: 새로운 던전이 추가되면 던전 ID(숫자)와 던전 이름(한글)을 재왕씨에게 알려주세요~ 


			// 아케이드 모드 던전
			DI_ARCADE_RUBEN							= 38000,
			DI_ARCADE_ELDER							= 38010,
			DI_ARCADE_SEASON1						= 38020,

			DI_ELDER_HENIR_SPACE					= 38100,
// 			DI_BESMA_HENIR_SPACE					= 38110,
// 			DI_ALTERA_HENIR_SPACE					= 38120,
// 			DI_FEITA_HENIR_SPACE					= 38130,
// 			// kimhc // 벨더 헤니르 // 2009-10-27
// 			DI_VELDER_HENIR_SPACE					= 38140,
// 			// JHKang / 강정훈 / 2011.1.26 / 하멜 헤니르
// 			DI_HAMEL_HENIR_SPACE					= 38150,
			
			
			/////////////////////////////	디펜스 던전 ///////////////////////////////////////
			DI_DEFENCE_DUNGEON_ELDER_NORMAL			= 38300,	
			DI_DEFENCE_DUNGEON_ELDER_HARD,
			DI_DEFENCE_DUNGEON_ELDER_EXPERT,


			DI_EVENT_VALENTINE_DAY					= 38400,	// 발렌타인 이벤트 던전

// 			DI_DEFENCE_DUNGEON_BESMA_NORMAL			= 38310,
// 			DI_DEFENCE_DUNGEON_BESMA_HARD,
// 			DI_DEFENCE_DUNGEON_BESMA_EXPERT,
// 
// 			DI_DEFENCE_DUNGEON_ALTERA_NORMAL		= 38320,
// 			DI_DEFENCE_DUNGEON_ALTERA_HARD,
// 			DI_DEFENCE_DUNGEON_ALTERA_EXPERT,
// 
// 			DI_DEFENCE_DUNGEON_PEITA_NORMAL			= 38330,
// 			DI_DEFENCE_DUNGEON_PEITA_HARD,
// 			DI_DEFENCE_DUNGEON_PEITA_EXPERT,
// 
// 			DI_DEFENCE_DUNGEON_VELDER_NORMAL		= 38340,
// 			DI_DEFENCE_DUNGEON_VELDER_HARD,
// 			DI_DEFENCE_DUNGEON_VELDER_EXPERT,
// 
// 			DI_DEFENCE_DUNGEON_HAMEL_NORMAL			= 38350,
// 			DI_DEFENCE_DUNGEON_HAMEL_HARD,
// 			DI_DEFENCE_DUNGEON_HAMEL_EXPERT,
			//////////////////////////////////////////////////////////////////////////////////




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
								
			DI_EVENT_TREE_DAY_ELDER					= 39500,
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
//#ifdef ARA_CHARACTER_BASE
			DI_BATTLE_FIELD_TUTORIAL_ARA,						/// 아라 튜토리얼
//#endif
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
			DI_EVENT_BIRTHDAY_CAKE_DEFENCE_DUNGEON_NORMAL= 41090, 
			//#endif //EVENT_BIRTHDAY_DUNGEON
			//#ifdef SERV_EVENT_SONGKRAN_FESTIVAL_DUNGEON
			DI_EVENT_SONGKRAN_FESTIVAL_DUNGEON			= 41100,
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

		//클라이언트만 필요한 UI Data들 .. 
		struct DungeonDataUI
		{
			wstring		m_TextureName;
			wstring		m_PieceName;

			wstring		m_Explanation;

			D3DXVECTOR2 m_LocalStarPos;					//지역 지도에 던전 별표시 위치 좌표
			D3DXVECTOR2 m_PopUpOffsetPos;				//지역 지도에 던전 팝업창 오프셋 위치 좌표
#ifdef SERV_EPIC_QUEST
			D3DXVECTOR2 m_LocalQuestPos;
#endif SERV_EPIC_QUEST

			vector<wstring>		m_vecAutoRoomTitle;		// 던전방 생성시 자동으로 지정되는 방 제목

			//{{ 2009.1.22 김태완 던전별 로딩화면
#ifdef ENTER_SCREEN_FOR_DUNGEON
			wstring		m_LoadingScreenFileName;
			wstring		m_LoadingScreenFileName2;
#endif
			//}}

			DungeonDataUI()
                : m_TextureName()
                , m_PieceName()
                , m_Explanation()
                , m_LocalStarPos( 0, 0 )
                , m_PopUpOffsetPos( 0, 0 )
#ifdef SERV_EPIC_QUEST
                , m_LocalQuestPos( 0, 0 )
#endif SERV_EPIC_QUEST
                , m_vecAutoRoomTitle()
#ifdef ENTER_SCREEN_FOR_DUNGEON
                , m_LoadingScreenFileName()
                , m_LoadingScreenFileName2()
#endif  ENTER_SCREEN_FOR_DUNGEON
			{
			}

		};

		struct DungeonData
		{
		public:
			typedef std::vector< wstring > EndingSpeech;									// 연결되는 대사 모음
			typedef std::vector< EndingSpeech > EndingSpeechSet;							// 연결되는 대사들의 모음
			typedef std::map< CX2Unit::UNIT_TYPE, EndingSpeechSet > EndingSpeechSetMap;		// 캐릭터 타입별 연결되는 대사들의 모임의 map
			typedef std::map< CX2Unit::UNIT_TYPE, vector<int> >		mapBossDropItem;		// 캐릭터 타입별 던전에서 획득가능한 아이템 map

			

		public:
			std::vector< EndingSpeechSetMap > m_vecEndingSpeechSetMap;

			DUNGEON_ID	m_DungeonID;
			wstring		m_DungeonName;
			wstring		m_DataFileName;
			DUNGEON_ID	m_RequireDungeonID;
			vector< DUNGEON_ID > m_vecExtraRequireDungeonID;		// 비밀던전과 같이 특별히 require dungeon id가 여러개인 경우에 여기에 추가


			bool		m_bHellMode;		// hell mode 던전은 입장조건이 되기 전까지 던전 아이콘도 보이지 않아야 한다
			
			//{{ 2012. 05. 22	최육사	던전 입장 아이템 레벨
#ifdef SERV_DUNGEON_REQUIRE_ITEM_LEVEL
			int				m_RequireItemLevel;
#endif SERV_DUNGEON_REQUIRE_ITEM_LEVEL
			//}}
			
			//{{ 2007. 8. 29  최육사  던전입장 조건 아이템
			int			m_RequireItemID;
			int			m_RequireItemCount;
			//}}
			
			int			m_RequireItemID2;
			int			m_RequireItemCount2;
			

			//{{ 2007. 10. 4  최육사  근성도
			int			m_RequireSpirit;
			//}}
			
			int			m_MinLevel;
			int			m_MaxLevel;

			CX2World::WORLD_ID m_eDefaultDungeonLoungeWorldID;

			DIFFICULTY_LEVEL m_eDifficulty;

			bool		m_bNormalOnly;			// 난이도가 "보통"만 있는 던전이다
			float		m_fTimeLimit;			// 던전 플레이 시간 제한
#ifdef SHOW_REMAIN_TIME_IN_CLEAR_CONDITION
			bool		m_bShowStageTime;
#endif SHOW_REMAIN_TIME_IN_CLEAR_CONDITION

#ifdef SERV_DUNGEON_OPTION_IN_LUA
			bool			m_bLevelEqualized;
			bool			m_bDamageEqualized;
			bool			m_bEventDungeon;
			unsigned short	m_usFixedMembers;
#endif SERV_DUNGEON_OPTION_IN_LUA

#ifdef SERV_LIMITED_DUNGEON_PLAY_TIMES
			int				m_iLimitedPlayTimes;
			int				m_iLimitedClearTimes;
#endif SERV_LIMITED_DUNGEON_PLAY_TIMES

			DungeonDataUI m_UIData;


#ifdef HENIR_TEST
			
			DUNGEON_TYPE	m_eDungeonType;
			bool			m_bRelativeMonsterLevel;

#endif HENIR_TEST
			wstring		m_DungeonDescription;
			vector<int>	m_vecBossItemList;
			mapBossDropItem m_mapBossDropItem;

#ifdef HIDE_LOADING_TIP
			bool		m_bHideLoadingTip;
#endif HIDE_LOADING_TIP

#ifdef X2TOOL
			int				m_iNpcLevel;
#endif
		};

		
		struct DungeonMapData
		{
			D3DXVECTOR2		m_vPos;
			bool			m_bOpen;		// user가 이 stage를 진행했는지
#ifdef NEW_HENIR_TEST
			bool			m_bAutoOpen;	// user가 이 stage를 진행하지 않았지만, 같은 던전맵 좌표에 위치한 다른 stage가 진행되었을 경우의 처리. 
			bool			m_bRestRoomState;
#endif NEW_HENIR_TEST
			bool			m_bIsBossStage;
			bool			m_bIsNextStageRandom;

			DungeonMapData()
			{
				m_vPos = D3DXVECTOR2(0,0);
				m_bOpen = false;
#ifdef NEW_HENIR_TEST
				m_bAutoOpen = false;
				m_bRestRoomState = false;
#endif NEW_HENIR_TEST
				m_bIsBossStage = false;
				m_bIsNextStageRandom = false;
			}
		};

		struct FullStageIndex
		{
			int m_iStageIndex;
			int m_iSubStageIndex;

			FullStageIndex()
			{
				m_iStageIndex = -1;
				m_iSubStageIndex = -1;
			}

			FullStageIndex( int iStageIndex, int iSubStageIndex )
			{
				m_iStageIndex = iStageIndex;
				m_iSubStageIndex = iSubStageIndex;
			}

			bool operator < ( CONST FullStageIndex& rhs ) const
			{
				if( m_iStageIndex < rhs.m_iStageIndex )
					return true;
				else if( m_iStageIndex > rhs.m_iStageIndex )
					return false;

				if( m_iSubStageIndex < rhs.m_iSubStageIndex )
					return true;

				return false;
			}
		};

	public:
		CX2Dungeon( const DungeonData* pDungeonData );
		//{{ 2007. 9. 14  최육사  NPC 스크립트 정보 받기 여부를 설정할 수 있는 생성자
		CX2Dungeon( const DungeonData* pDungeonData, bool bIsNpcLoad );
		//}}		
		~CX2Dungeon(void);

		void OnFrameMove( double fTime, float fElapsedTime );

		CX2DungeonStage*	CreateStage( int stageNum );
        void                CreateStageNPCs();

		void				FlushNPCs();
		CX2DungeonStage*	GetNowStage(){ return m_pNowStage; }

		int					GetNowStageIndex(){ return m_NowStageIndex; }
		int					GetNumberOfStageData(){ return m_StageDataList.size(); }
		
		void				SetStageStaticNPC( int stageNum, std::map<int, KNPCList>& mapNPCData );

		const DungeonData*	GetDungeonData() { return m_pDungeonData; }

#ifdef X2TOOL
		map< std::pair<int,int>, DungeonMapData >& GetMapDungeonMapData4Tool() { return m_mapDungeonMapData; }
#endif
		const map< std::pair<int,int>, DungeonMapData >& GetMapDungeonMapData() const { return m_mapDungeonMapData; }
		const map< std::pair< FullStageIndex, FullStageIndex>, bool >& GetMapStageLine() const { return m_mapStageLine; }
		void SetDungeonMapOpen( int iStageIndex, int iSubStageIndex, bool bOpen );
		bool GetDungeonMapOpen( int iStageIndex, int iSubStageIndex );
		bool IsStageExist( int iStageIndex, int iSubStageIndex );
		bool GetStageLineOpen( int iPrevStageIndex, int iPrevSubStageIndex, int iStageIndex, int iSubStageIndex );

		D3DXCOLOR GetWorldColor() const { return m_WorldColor; }
		D3DXCOLOR GetUnitColor() const { return m_UnitColor; }
		//{{ 2007. 10. 30  최육사  
		void SetWorldColor( D3DXCOLOR worldColor ) { m_WorldColor = worldColor; }
		void SetUnitColor( D3DXCOLOR unitColor ) { m_UnitColor = unitColor; }
		//}}

//#ifdef CORRECTION_DAMAGE_FREE_CHANNEL
		static bool IsEventDungeon( const DUNGEON_ID eDungeonId_ );
		static bool IsHenirDungeon( const DUNGEON_ID eDungeonId_, const bool bChallenge_ );

		static bool IsDamageFreeGame();	// FieldFix: 이게 던전에 있다는것 자체가 잘못 된 것
//#endif

#ifdef SERV_DUNGEON_OPTION_IN_LUA
		static bool IsFixedMembers( const DUNGEON_ID eDungeonID_ );
		static short GetFixedMembers( const DUNGEON_ID eDungeonID_ );
#endif SERV_DUNGEON_OPTION_IN_LUA

		static const WCHAR* GetDungeonDifficultyString( const DUNGEON_TYPE eDungeonType_, const char cDungeonMode_, const DIFFICULTY_LEVEL eDufficulty_ );

#ifdef	SERV_DUNGEON_FORCED_EXIT_SYSTEM
		void						SetBeforeSubStageIndexUsingPacket (const int index_ ) { m_iBeforeSubStageIndexUsingPacket = index_; }		// 이전 SubStage 의 값을 설정, Packet 전송을 위해서 사용한다.
		void						SetBeforeStageIndexUsingPacket (const int index_ ) { m_iBeforeStageIndexUsingPacket = index_; }				// 이전 Stage 의 값을 설정, Packet 전송을 위해서 사용한다.
		int							GetBeforeSubStageIndexUsingPacket () const { return m_iBeforeSubStageIndexUsingPacket; }					// 이전 SubStage 의 값을 반환, Packet 전송을 위해서 사용한다.
		int							GetBeforeStageIndexUsingPacket () const { return m_iBeforeStageIndexUsingPacket; }							// 이전 Stage 의 값을 반환, Packet 전송을 위해서 사용한다.
#endif // SERV_DUNGEON_FORCED_EXIT_SYSTEM	

		//{{ 2007. 9. 18  최육사  던전툴에서 사용하는 함수
	public: // for tool only
		vector<CX2DungeonStage::StageData*>& GetStageDataList() { return m_StageDataList; }
		//}}

	protected:
		bool OpenScriptFile( const WCHAR* pFileName );

		const DungeonData*						m_pDungeonData;			// 현재 던전 데이터 객체, 한번 로드되면 변경 안되도록.
		CX2DungeonStage*						m_pNowStage;			// 현재 플레이하고 있는 스테이지 객체, 로드된 후에 변경 될 수 있다
		int										m_NowStageIndex;
		float									m_fElapsedTime;
		bool									m_bIsNpcLoad;			// NPC 스크립트 정보 불러오기 여부

		vector<CX2DungeonStage::StageData*>		m_StageDataList;

		
		map< std::pair<int,int>, DungeonMapData > m_mapDungeonMapData;	// stage index, substage index - dungeonmapdata
		map< std::pair< FullStageIndex, FullStageIndex>, bool > m_mapStageLine; // 진행한 substage 사이의 선 
		
		FullStageIndex							m_PrevStageIndex;
		
		
		D3DXCOLOR	m_WorldColor;
		D3DXCOLOR	m_UnitColor;


#ifdef	SERV_DUNGEON_FORCED_EXIT_SYSTEM
	private :
		int							m_iBeforeSubStageIndexUsingPacket;			// 이전 서브 스테이지의 값을 저장하는 인자, Packet 을 보낼때만 사용한다.
		int							m_iBeforeStageIndexUsingPacket;				// 이전 스테이지의 값을 저장하는 인자, Packet 을 보낼때만 사용한다.
#endif // SERV_DUNGEON_FORCED_EXIT_SYSTEM	

};

