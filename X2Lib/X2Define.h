#pragma once
//{{ kimhc // 2010.6.7 // �ŷ��� �Ұ����� ��ȭ�� �߰�
#ifdef	NO_TRADE_ATTACH_ITEM
#include <boost/array.hpp>
#endif	NO_TRADE_ATTACH_ITEM
//}} kimhc // 2010.6.7 // �ŷ��� �Ұ����� ��ȭ�� �߰�

#ifdef _SERVICE_
	#ifdef _OPEN_TEST_
		#define NEXON_KOREA_ELSWORD_GAMECODE 0x0001F00A
	#else
		#define NEXON_KOREA_ELSWORD_GAMECODE 0x00017010
	#endif
#else	
	#ifdef _USE_NEXON_MSG_INHOUSE
		#define NEXON_KOREA_ELSWORD_GAMECODE 0x00017010
	#else
		#define NEXON_KOREA_ELSWORD_GAMECODE 0x0001F00B
	#endif
#endif

#define LIMIT_MAX_LEVEL					65 //���� ����	// �ؿ��� ��� ����

#ifdef INT_WIDE_BAR
#define MAX_NOTICE_NUM					1
#else // INT_WIDE_BAR
#define MAX_NOTICE_NUM					2
#endif // INT_WIDE_BAR

#define CHAT_OPTION_TOTAL				7
#define	CHAT_OPTION_PARTY				5
#define	CHAT_OPTION_GUILD				5
#define	CHAT_OPTION_WHISPER				4

#define	LIMIT_DISTANCE					800
#define LIMIT_DISTANCE_SQ				LIMIT_DISTANCE * LIMIT_DISTANCE
//#define LINE_GAP 30.0f
//#define MAX_PLAYER_NUM 8
#define MTU_SIZE						1300

#define STR_LEN_SHORT					10
#define STR_LEN_MEDIUM					50
#define STR_LEN_LONG					255

#define MAX_HAVE_UNIT_NUM				10
#define MAX_EQIP_NUM					20

//{{ robobeg : 2013-06-28
//#define MAX_GAME_USER_UNIT_NUM			30
#define MAX_GAME_USER_UNIT_NUM			16
//}} robobeg : 2013-06-28
#define	MAX_GAME_NPC_UNIT_NUM			80	

#define MAX_PVP_ROOM_USER_NUM			8
#define MAX_DUNGEON_ROOM_USER_NUM		4
#ifdef CLIENT_COUNTRY_EU
#define MAX_ROOM_CHAT_STR_LEN			80		// pvp room, dungeon room chatting UI ����
#else //#ifdef CLIENT_COUNTRY_EU
#define MAX_ROOM_CHAT_STR_LEN			70		// pvp room, dungeon room chatting UI ����
#endif //#define MAX_ROOM_CHAT_STR_LEN			70		// pvp room, dungeon room chatting UI ����
#define INVENTORY_ITEM_IMAGE_SIZE_X		66
#define INVENTORY_ITEM_IMAGE_SIZE_Y		66

#define MY_QUEST_LIST_MAX_SIZE			30





#define PERSONAL_SHOP_ITEM_SLOT_MAX		9
#define PERSONAL_TRADE_ITEM_SLOT_MAX	10

#define MANUFACTURE_LIST_MAX_SLOT_NUM   8


#define NORMAL_MAGIC_STONE_ITEM_ID 109945
#define SPECIAL_MAGIC_STONE_ITEM_ID 109955

// #ifdef SERV_SOCKET_NEW
// //{{ oasis907 : ����� [2010.4.20] // 
// #define HARMONY_MAGIC_STONE_ITEM_ID 132110
// #define MYSTERY_MAGIC_STONE_ITEM_ID 132111
// #define SHINE_MAGIC_STONE_ITEM_ID 132112
// #define BATTLE_MAGIC_STONE_ITEM_ID 132113
// #define INCREASE_MAGIC_STONE_ITEM_ID 132114
// #define HIT_MAGIC_STONE_ITEM_ID 132115
// //}}
// 
// //{{ oasis907 : ����� [2010.4.29] //
// 
// #define SINGLE_FIXED_OPTION_STONE_ITEM_ID_START 132193
// 
// #define SINGLE_FIXED_OPTION_STONE_ITEM_ID_00 132192
// #define SINGLE_FIXED_OPTION_STONE_ITEM_ID_01 132193
// #define SINGLE_FIXED_OPTION_STONE_ITEM_ID_02 132194
// #define SINGLE_FIXED_OPTION_STONE_ITEM_ID_03 132195
// #define SINGLE_FIXED_OPTION_STONE_ITEM_ID_04 132196
// #define SINGLE_FIXED_OPTION_STONE_ITEM_ID_05 132197
// #define SINGLE_FIXED_OPTION_STONE_ITEM_ID_06 132198
// #define SINGLE_FIXED_OPTION_STONE_ITEM_ID_07 132199
// #define SINGLE_FIXED_OPTION_STONE_ITEM_ID_08 132200
// #define SINGLE_FIXED_OPTION_STONE_ITEM_ID_09 132201
// #define SINGLE_FIXED_OPTION_STONE_ITEM_ID_10 132202
// #define SINGLE_FIXED_OPTION_STONE_ITEM_ID_11 132203
// #define SINGLE_FIXED_OPTION_STONE_ITEM_ID_12 132204
// #define SINGLE_FIXED_OPTION_STONE_ITEM_ID_13 132205
// 
// #define SINGLE_FIXED_OPTION_STONE_ITEM_ID_END 132205
// //}}
// 
// #endif SERV_SOCKET_NEW

#define RESOLVE_RESULT_ITEM_LIST_MAX_SLOT_NUM 6

#define MAX_SOCKET_SLOT_NUM 4

#define ITEM_DESC_ADD_SIZE_X 50
#define ITEM_DESC_ADD_SIZE_Y 60

#define CASH_SHOP_MAIN_SLOT_NUM 3

#define CASH_SHOP_DEPOSIT_SLOT_NUM 16

#define CASH_SHOP_SUB_SPECIAL_SLOT_NUM 5

#ifdef	SERV_REFORM_INVENTORY_TEST
	#define MY_INVENTORY_SHOW_MAX_SLOT_NUM 224		/// �κ��丮 �� ī�װ����� �ִ� ���� ����
	#define MY_INVENTORY_SHOW_SLOT_NUM_PER_PAGE 56	/// �κ��丮 �� �������� �ִ� ���� ����

	#define MY_INVENTORY_MAX_SLOT_HEIGHT 7			/// �� �������� �ִ� ���� �� ��
	#define MY_INVENTORY_MAX_SLOT_WIDTH 8			/// �� ���δ� �ִ� ���� ����

#ifdef SERV_REFORM_INVENTORY_INT
	#define MY_INVENTORY_SPECIAL_MAX_SLOT_WIDTH 7	/// Ư�� ī�װ��� �ִ� ���� �� ��
#else // SERV_REFORM_INVENTORY_INT
	#define MY_INVENTORY_SPECIAL_MAX_SLOT_WIDTH 8	/// Ư�� ī�װ��� �ִ� ���� �� ��
#endif // SERV_REFORM_INVENTORY_INT

	#define MY_INVENTORY_AVARTA_MAX_SLOT_WIDTH 28	/// �ƹ�Ÿ ī�װ��� �ִ� ���� �� ��
	#define MY_INVENTORY_PC_BANG_MAX_SLOT_WIDTH 3	/// �ǽù� ī�װ��� �ִ� ���� �� ��
#else	SERV_REFORM_INVENTORY_TEST
	#define MY_INVENTORY_SHOW_MAX_SLOT_NUM 32
#endif	SERV_REFORM_INVENTORY_TEST

#define UNIT_SELECT_UI_MAX_UNIT_NUM 4

#define CASH_SHOP_CHOICED_SLOT_NUM 21

#if defined(CLIENT_COUNTRY_EU) || defined(CLIENT_COUNTRY_CN) || defined(CLIENT_COUNTRY_US)
#define PVP_MAP_SHOW_LIST 14
#else //defined(CLIENT_COUNTRY_EU) || defined(CLIENT_COUNTRY_CN) || defined(CLIENT_COUNTRY_US)
#define PVP_MAP_SHOW_LIST 12
#endif 	//defined(CLIENT_COUNTRY_EU) || defined(CLIENT_COUNTRY_CN) || defined(CLIENT_COUNTRY_US)

#define X2_INTRUDE_EVENT_MAX_VALUE 50

#define DASH_CAMERA_ANGLE				45
#define DASH_CAMERA_FIRST_WAIT_TIME		0.5f
#define DASH_CAMERA_NORMAL_TRACKING_START_TIME 1.1f
#define DASH_CAMERA_TRACKING_TIME		1.7f

#define MAGIC_ENCHANT_LEVEL_LIMIT		10	// ��ȭ�������� ����� �� �ִ� ��ȭ���� ����

#define MAX_QUEST_INDICATOR_NUM			4	// ����Ʈ ���̵� ����
#define MAX_DUNGEON_INDICATOR_NUM		1	// ���� ����Ű�� ����Ʈ ���̵� ����

//{{ kimhc // 2010.4.12 // ��д��� �۾�(��������Ʈ �ý���)
#ifdef SERV_SECRET_HELL
#ifdef CLIENT_COUNTRY_US
const int HOUR_TO_REFRESH_DAILY_QUEST = 0;	// ���� ����Ʈ�� ���� �Ǵ� �ð�(0 AM)
#else // CLIENT_COUNTRY_US
const int HOUR_TO_REFRESH_DAILY_QUEST = 6;	// ���� ����Ʈ�� ���� �Ǵ� �ð�(6 AM)
#endif // CLIENT_COUNTRY_US
#endif SERV_SECRET_HELL
//}} kimhc // 2010.4.12 // ��д��� �۾�(��������Ʈ �ý���)

//{{ kimhc // 2011-08-08 // ���� ���� ä�θ� �̺�Ʈ
#ifdef	SERV_TOONILAND_CHANNELING_EVENT
const int TOONILAND_CHANNELING_EVENT_TITLE = 10710; // ������ ��
#endif	SERV_TOONILAND_CHANNELING_EVENT
//}} kimhc // 2011-08-08 // ���� ���� ä�θ� �̺�Ʈ

#ifdef GRACE_OF_EL
const int GRACE_OF_EL_TITLE = 10765;		/// ���� ��ȣ Īȣ
#endif

#ifdef SERV_EVENT_MONEY
//õ���� ���� ������ID

	#ifdef SERV_FIELD_LAUNCHING_EVENT			/// �ʵ� ��Ī �̺�Ʈ - �ʵ� ��� ��ȭ
			const int ANGELS_FEATHER_ITEM_ID	=	135292;
	#else  SERV_FIELD_LAUNCHING_EVENT
		#ifdef SERV_THE_PREVIOUS_FIELD_EVENT	/// �ʵ� ���� �̺�Ʈ - ������ ���� ����
			const int ANGELS_FEATHER_ITEM_ID	=	135246;
		#else  SERV_THE_PREVIOUS_FIELD_EVENT
			const int ANGELS_FEATHER_ITEM_ID	=	135183;
		#endif SERV_THE_PREVIOUS_FIELD_EVENT
	#endif SERV_FIELD_LAUNCHING_EVENT

#endif //SERV_EVENT_MONEY

// �����ݾ� �ѵ�
static const int MAX_ED_FOR_PLAYER	= 2000000000;
static const int MAX_ED_FOR_LETTER  = 500000000;
#ifdef SERV_COUNTRY_JP
static const int MAX_ED_FOR_TRADE	= 500000000;	// �ӱԼ� �Ϻ� ���� ���� 700000000
#else //SERV_COUNTRY_JP
static const int MAX_ED_FOR_TRADE	= 700000000; //500000000;
#endif //SERV_COUNTRY_JP

static const int MAX_ENCHANT_LEVEL							= 20;			// ��ȭ �ְ� ������ ���� 20����
static const float ENCHANT_STAT_SCALE[] =
{
	1.f, 
		1.03f, 1.06f, 1.09f, 1.16f, 1.23f, 
		1.30f, 1.45f, 1.60f, 1.75f, 2.15f, 
		2.55f, 2.95f, 3.35f, 3.75f, 4.25f,
		4.75f, 5.25f, 5.75f, 6.75f, 8.75f,
};

static const int ENCHANT_WEAPON_EFFECT_LEVEL_0				= 5;			// �⺻ ��ȭ ����Ʈ
static const int ENCHANT_WEAPON_EFFECT_LEVEL_1 				= 7;			// �߱� ��ȭ ����Ʈ
static const int ENCHANT_WEAPON_EFFECT_LEVEL_2 				= 11;			// ��� ��ȭ ����Ʈ ( 14���� 11�� ���� )


#ifdef NEW_SKILL_TREE
	static const int EQUIPPED_SKILL_SLOT_COUNT = 4;		// ���� ��ų ���� ����
#endif NEW_SKILL_TREE

#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
	static const int MAX_LIMITED_SKILL_LEVEL = 30;		// �ִ� ��ų ����
	static const int MAX_UPGRADE_SKILL_LEVEL = 20;		// �ִ� ��ų ��ȭ ����
	static const int OPEN_SKILL_TREE_LEVEL[] =			// ��ųâ �� ��ų ���� ���� �䱸 ����
	{
		0,	5,	10,	15,	20,			/// �⺻ ����
		0,	25,	30,	35,	40,			/// 1�� ����
		0,	45,	50,	55,	60,	65,		/// 2�� ����
	};
#endif // UPGRADE_SKILL_SYSTEM_2013





//////////////////////////////////////////////////////////////////////////
// Ư���� ITEM ID

// ������ ����(��Ī)
static const int ITEM_FOR_SEAL_NORMAL						= 214370;	// �Ƿ��� �ߵ���
static const int ITEM_FOR_SEAL_RARE							= 214380;	
static const int ITEM_FOR_SEAL_ELITE						= 214390;	
static const int ITEM_FOR_SEAL_UNIQUE						= 214400;	

#ifdef SERV_RURIEL_EVENT_ITEM
static const int ITEM_FOR_SEAL_NORMAL_EVENT_RURIEL					= 83000290; // �縮���� �Ƿ��� �� ����(�Ͼ��)
static const int ITEM_FOR_SEAL_RARE_EVENT_RURIEL					= 83000291; // �縮���� �Ƿ��� �� ����(�����)
static const int ITEM_FOR_SEAL_ELITE_EVENT_RURIEL					= 83000292; // �縮���� �Ƿ��� �� ����(�����)
static const int ITEM_FOR_SEAL_UNIQUE_EVENT_RURIEL					= 83000293; // �縮���� �Ƿ��� �� ����(Ȳ�ݻ�)
#endif //SERV_RURIEL_EVENT_ITEM

#ifdef SERV_ARIEL_EVENT_ITEM
static const int ITEM_FOR_SEAL_NORMAL_EVENT_ARIEL					= 82214370; // �Ƹ����� �Ƿ��� �� ����(�Ͼ��)
static const int ITEM_FOR_SEAL_RARE_EVENT_ARIEL						= 82214380; // �Ƹ����� �Ƿ��� �� ����(�����)
static const int ITEM_FOR_SEAL_ELITE_EVENT_ARIEL					= 82214390; // �Ƹ����� �Ƿ��� �� ����(�����)
static const int ITEM_FOR_SEAL_UNIQUE_EVENT_ARIEL					= 82214400; // �Ƹ����� �Ƿ��� �� ����(Ȳ�ݻ�)
#endif //SERV_ARIEL_EVENT_ITEM

#ifdef SERV_COUNTRY_JP
// SERV_COBO_SEAL_ITEM_EVENT ���� Normal : 160372 , Elite : 160373
static const int ITEM_FOR_SEAL_NORMAL_EVENT						= 70001900; // �Ƿ��� �� ����(�Ͼ��)
static const int ITEM_FOR_SEAL_RARE_EVENT						= 70001901; // �Ƿ��� �� ����(�����)
static const int ITEM_FOR_SEAL_ELITE_EVENT						= 70001902; // �Ƿ��� �� ����(�����)
static const int ITEM_FOR_SEAL_UNIQUE_EVENT						= 70001903; // �Ƿ��� �� ����(Ȳ�ݻ�)
#else //SERV_COUNTRY_TWHK
static const int ITEM_FOR_SEAL_NORMAL_EVENT					= 160372;	// �Ƿ��� �ߵ��� �̺�Ʈ��(�븻)
static const int ITEM_FOR_SEAL_ELITE_EVENT					= 160373;	// �Ƿ��� �ߵ��� �̺�Ʈ��(����Ʈ)
#ifdef CHILDRENS_DAY_EVENT_ITEM
static const int ITEM_FOR_SEAL_UNIQUE_EVENT					= 160794;	// �Ƿ��� �ߵ��� �̺�Ʈ��(����ũ) ��̳�
#endif //CHILDRENS_DAY_EVENT_ITEM
#endif //SERV_COUNTRY_TWHK

// �κ��丮 Ȯ�� ĳ�þ�����
static const int INVENTORY_SLOT_ADD_ITEM					= 200750;

#ifdef SERV_REFORM_INVENTORY_INT_CASH_ITEM
static const int INVENTORY_SLOT_ADD_ITEM_EQUIP				= 200751;
static const int INVENTORY_SLOT_ADD_ITEM_ACCESSORY			= 200752;
static const int INVENTORY_SLOT_ADD_ITEM_QUICK_SLOT			= 200753;
static const int INVENTORY_SLOT_ADD_ITEM_MATERIAL			= 200754;
static const int INVENTORY_SLOT_ADD_ITEM_QUEST				= 200755;
static const int INVENTORY_SLOT_ADD_ITEM_SPECIAL			= 200756;
#endif // SERV_REFORM_INVENTORY_INT_CASH_ITEM

// �̺��丮 Ȯ�� �̺�Ʈ ������ SERV_EXPAND_INVENTORY_BY_EVENT_ITEM
static const int INVENTORY_SLOT_ADD_ITEM_EQUIP_EVENT			= 60002281;
static const int INVENTORY_SLOT_ADD_ITEM_ACCESSORY_EVENT		= 60002282;
static const int INVENTORY_SLOT_ADD_ITEM_QUICK_SLOT_EVENT		= 60002283;
static const int INVENTORY_SLOT_ADD_ITEM_MATERIAL_EVENT			= 60002284;
static const int INVENTORY_SLOT_ADD_ITEM_QUEST_EVENT			= 60002285;
static const int INVENTORY_SLOT_ADD_ITEM_SPECIAL_EVENT			= 60002286;

// �κ��丮 ��ũ�� Ȯ�� ���
#ifdef SERV_REFORM_INVENTORY_TEST
static const int INVENTORY_SLOT_MAX_NUM							= 56;
#else
static const int INVENTORY_SLOT_MAX_NUM							= 32;
#endif SERV_REFORM_INVENTORY_TEST

// ���� ���� ȸ����
static const int SILVER_MEMBERSHIP_CARD						= 213460;
static const int GOLD_MEMBERSHIP_CARD						= 213470;
static const int EMERALD_MEMBERSHIP_CARD					= 213480;
static const int DIAMOND_MEMBERSHIP_CARD					= 213490;
static const int PLATINUM_MEMBERSHIP_CARD					= 213500;
static const int COBO_CREDIT_CARD							= 213610;

// �⺻ ��ȭ�� item id
static const int NORMAL_WEAPON_ENCHANT_STONE_ITEM_ID		= 109950;
static const int NORMAL_DEFENCE_ENCHANT_STONE_ITEM_ID 		= 109965;

//static const int SPECIAL_WEAPON_ENCHANT_STONE_ITEM_ID		= 109960;		// obsolete!!
//static const int SPECIAL_DEFENSE_ENCHANT_STONE_ITEM_ID	= 109970;

// ���� �ִ� ��ȭ�� Item ID
static const int NEW_WEAPON_ENCHANT_STONE_ITEM_ID[]			= { 130077, 130078, 130079, 130080, 130081, 130082, 130083, 130084, 130085 };
static const int NEW_DEFENCE_ENCHANT_STONE_ITEM_ID[]		= { 130086, 130087, 130088, 130089, 130090, 130091, 130092, 130093, 130094 };


#ifdef SERV_EVENT_CHARACTER_SLOT_EXPAND_ITEM
static const int CHARACTER_SLOT_EXPAND_ITEM1				= 60000410;		// �κ��丮 ��� �̺�Ʈ�� ĳ���� ���� Ȯ���1
static const int CHARACTER_SLOT_EXPAND_ITEM2				= 60000825;		// �κ��丮 ��� �̺�Ʈ�� ĳ���� ���� Ȯ���2
static const int CHARACTER_SLOT_EXPAND_ITEM3				= 60001965;		// �κ��丮 ��� �̺�Ʈ�� ĳ���� ���� Ȯ���3
#endif SERV_EVENT_CHARACTER_SLOT_EXPAND_ITEM
#ifdef SERV_EVENT_INIT_SKILL_TREE_ITEM
static const int INIT_SKILL_TREE_ITEM						= 60001727;		// ������ �帵ũ
#endif SERV_EVENT_INIT_SKILL_TREE_ITEM
#ifdef SERV_EVENT_RESET_A_SKILL_ITEM
static const int RESET_A_SKILL_ITEM							= 60001726;		// ������ �˾�
#endif SERV_EVENT_RESET_A_SKILL_ITEM
#ifdef SERV_EVENT_RURIEL_RESET_SKILL_ITEM
static const int RURIEL_RESET_A_SKILL_ITEM					= 67006453;		// �縮���� ���� ��ų �ʱ�ȭ �޴�
static const int RURIEL_RESET_SKILL_ITEM					= 67006454;		// �縮���� ��ų �ʱ�ȭ �޴�
static const int RURIEL_RESET_A_SKILL_EVENT_ITEM			= 67006451;		// �縮���� ������ �˾�(��ų 1�� �ǵ����� ������)
#endif SERV_EVENT_RURIEL_RESET_SKILL_ITEM
#ifdef SERV_BATTLEFIELD_COOKIE_PIECE
static const int RID_BATTLEFIELD_COOKIE						= 85002461;		// �ʵ� ��Ű ����
static const int RID_BATTLEFIELD_GOLD_COOKIE				= 85002463;		// �ʵ� Ȳ�� ��Ű ����
#endif SERV_BATTLEFIELD_COOKIE_PIECE

#ifdef SERV_ONE_THIRD_MINI_ATTRACTION_ITEM
static const int ONE_THIRD_MINI_ATTRACTION_ITEM				= 140000110;	// 1/3 Ȯ���� �̴� ������ ������
#endif //SERV_ONE_THIRD_MINI_ATTRACTION_ITEM

#ifdef ADDED_RELATIONSHIP_SYSTEM
static const int COUPLE_RING_ITEM_ID = 269710;
static const int WEDDING_RING_ITEM_ID = 269711;
static const int INVITE_WEDDING_ITEM_ID = 269714;
static const int INVITE_WEDDING_LETTER_ITEM_ID = 269716;
static const int RESERVED_WEDDING_LETTER_ITEM_ID = 269715;
static const int COUPLE_NICKNAME_CHANGE_ITEM_ID = 269718;

//ITEMID_RESERVED_WEDDING_HALL�迭 ���� �ϱ� ���� ���
enum RESERVED_WEDDING_HALL_VILLAGE
{
	RWHV_RUBEN = 0,
	RWHV_SANDER,
};
// ������
// ������ �߰��Ǹ� ������ ����� �Ѱ� ���� �κ� ��������� ��.
// iWeddingThumbnailNumEnd ���������� �˻��ϸ� ��.
// ���̾�α� ���� �����Ϸ��� �ð��� ���� �ɷ��� �̺κ��� �ϵ��ڵ����� ������.
static const int ITEM_ID_RESERVED_WEDDING_HALL[] = { 269712/*�纥*/, 269713/*����*/ };
static const int ITEM_ID_WEDDING_CASH_ITEM[] = { 269719, 269720 };
#endif // ADDED_RELATIONSHIP_SYSTEM	


// �Ǽ��縮 ������( attach_normal ) �߿� alpha object�� ������ �Ǿ� �ϴ� ������ ���� ó��, ���߿��� ��ũ��Ʈ�� ����~
static const int ALPHA_BLEND_ACCESSARY_ITEM_ID[] = {
	202300, 129915, 203730, 205980, 207450, 130533, 219790, 219860, 220070, 131848, 132763, 132766, 132797, 132800, 234140,
	234150, 234160, 234170, 234180, 235100, 235110, 238120, 244600, 244610, 244620, 244630, 81196,	81386,	81450,	81451, 81452,
	81453,	81454,	81455,	251690, 251700, 251710, 259100, 259110, 
	88340,	88348,	88356,	88364,	88372,	88380,	88388, // MK2 ����
	88509,	88517,	88525,	88533,	88541,	88549,	88557,  // MKB ����
	88565,	88573,	88581,	88589,	88597,	88605,	88613,	// MK2 ���� (�Ⱓ��)
	60004931, 60004932, 60004933, 70000110, 65000971, 65000972, 65000973, 65000974, 65001005, 70005631, 
	67004890, 67004891, 67004892, 67004893, 67004894, 67004895, 67004901, 67004902, 67004903, 67004904, 
	67004905, 67004906, 250000517, 250000527, 250000537, 250000547, 250000557, 250000567, 83000120,
	85001856, 85001857, 85001858, 85001859, 82205980, 67005966, 67005967, 60007563, 60007564,
	};
static const int DEST_ALPHA_BLEND_ACCESSARY_ITEM_ID[]			= { 130390, 130391, 130392, 130393, 130394, 133387, 87007, 87017, 87027, 87037, 87047, 87057, 180000, 81911, 81914, 81931, 182545, 182546, 182547, 182548, 250000517, 250000527, 250000537, 250000547, 250000557, 250000567, };	
// �ִϸ��̼� ���� ��� ����, �ִϸ��̼� �ִ� ���� �ڵ忡 �ϵ� �ڵ�
static const int ALPHA_BLEND_REAL_COLOR_ACCESSARY_ITEM_ID[]		= { 209050, 30037, 225190, 133123, 234190, 234200, 234210, 234220,
	234230, 180300, 135185, 256030, 256040, 30181,
	67000037, 70000127, 70005628, 67003530, 67003540, 85001856, 85001857, 85001858, 85001859};	 // �ִϸ��̼� ���� ��� ����, �ִϸ��̼� �ִ� ���� �ڵ忡 �ϵ� �ڵ�
static const int ALL_ALPHA_ACCESSARY_ITEM_ID[]					= { 131835, 131837, 131839, 131841, 131843, 132807, 133343, 133344,
	133345, 133346,  226870, 226880, 226890, 226900, 226910, 132878, 227650, 180058, 239350, 239360, 239370, 239380, 243310, 243320,
	160474, 160481, 160490, 160491, 181489, 181490, 256090, 256100, 81862, 81863, 81864, 81865, 81866, 81867, 81868, 135335, 266080,
	88285,	88293,	88301,	88309,	88317,	88325,	88333,	86577,	86585, 86593, 86601, 86609, 86617, 86625, 268570, 268571, 268560, 268561,
	60001934, 60001945, 64000053, 60004434, 60005100, 60005101, 60005102, 60005103, 60005104, 60005105, 60005106, 60005107, 60005108,
	60005109, 60005110, 60005111, 60005112, 60005113, 60005114, 60005115, 60005116, 60005117, 60005118, 60005119, 67004463, 67004465,
	70004210, 70004370, 70004380, 70004390, 70004400, 65001023, 65001024, 60005410, 60005411,
	85000554, 85000555, 85000556, 115000073, 115000074, 115000075,141000188, 141000189, 141000211, 141000212, 250000100, 250000110, 250000120, 250000130, 250000140, 250000150, 
	70007130, 70007131, 70007140, 70007141, 70007160, 70007161, 70007170, 70007171, 250001380, 250001381, 250001390, 250001391, 250001410, 250001411, 250001420, 250001421, 67004781,
	70007270, 70007271, 70007272, 90000460, 90000461, 90000462, 90001891, 90001892, 90001893, 85003476
};

static const int ALPHA_REAL_COLOR_CULL_NONE_ACCESSARY_ITEM_ID[]	= { 88621,	88629,	88637,	88645,	88653,	88661,	88669,	88677 };



// ��ȭ ����Ʈ ������ �����ϱ� ���� item id
const int ITEM_ID_ENCHANT_TYPE1[] = { 129635, 129645, 129655, 129665, 129675, };
const int ITEM_ID_ENCHANT_TYPE2[] = { 200950, 201000, 201050, 201100, 201150, 201200, 201280, 201290, 201300, 129997, 129998, 129999, 130000, };
const int ITEM_ID_ENCHANT_TYPE3_GOLD[]		= { 81170, 81933, 81941, 87200, 87210, 87220, 87230, 87240, 130550, 130551,130552, 130553, 130554, 131695, 131701, 131707, 131713, 131719 };
const int ITEM_ID_ENCHANT_TYPE3_BLUE[]		= { 130651, 130652, 130653, 130654, 130655, 130656, 130657, 130658, 130659, 130660, 130661, 130662, 130663, 130664, 130665, 130724, 130725, 130726, 130727, 130728, 130729, 130730, 130731, 130732, 130733, 130734, 130735, 130736, 130737, 130738, };
const int ITEM_ID_ENCHANT_TYPE3_GREEN[] 	= { 130666, 130667, 130668, 130669, 130670, 130671, 130672, 130673, 130674, 130675, 130676, 130677, 130678, 130679, 130680, 130739, 130740, 130741, 130742, 130743, 130744, 130745, 130746, 130747, 130748, 130749, 130750, 130751, 130752, 130753, };
const int ITEM_ID_ENCHANT_TYPE3_YELLOW[]	= { 130681, 130682, 130683, 130684, 130685, 130686, 130687, 130688, 130689, 130690, 130691, 130692, 130693, 130694, 130695, };
const int ITEM_ID_ENCHANT_TYPE3_RED[]		= { 130696, 130697, 130698, 130699, 130700, 130701, 130702, 130703, 130704, 130705, 130706, 130707, 130708, 130709, 130710, 130754, 130755, 130756, 130757, 130758, 130759, 130760, 130761, 130762, 130763, 130764, 130765, 130766, 130767, 130768, };
const int ITEM_ID_ENCHANT_TYPE3_PINK[]		= { 220830, 220890, 220950, 221010, 221070, 221130, 221190, 221250, 221310, 221370, };

// kimhc // 2010-01-12 // �ܰ��� ���� UT_ATTACH_ANIM �Ǽ��縮
const int REAL_COLOR_ATTACH_ANIM_ACCESSARY_ITEM_ID[] = { 131465, 131650, 132181, 88396, 88397, 88398, 88399, 88400, 88401 };

// JHKang / 2012.8.29 / ���� �� ó���� UT_ATTACH_ANIM �Ǽ��縮, CARTOON
const int ALPHA_BLEND_ATTACH_ANIM_ACCESSARY_ITEM_ID[] = { 81722, 81723, 81724, 81725, 81726, 81727, 81765, 81766, 81767, 81768, 81769, 81770, 259140, 259150, };

// JHKang / 2012.12.16 / UT_ATTACH_ANIM �Ǽ��縮, �ƶ� ���� ó���ϱ� ���� ����, Alpha, Real_color, SrcAlpha, DestAlpha, zEnable, ZWriteEnable, ���� �ʿ�, �ذ� ��� �ʿ�
const int REAL_COLOR_ALPHA_BLEND_ATTACH_ANIM_ACCESSARY_ITEM_ID[] = { 41580, };

// �ܰ��� ���� ī��ó���� �Ǽ�����
const int CARTOON_ACCESSARY_ITEM_ID[] = { 220790, 81630, 81631, 81632, 81633, 81634, 81635, 181495, 181496,
											60004555, 67002191, 67002192, 67002193, 67002194, 67002195, 67002190, 67002196, 67002197, 67002198, 67002199, 67002200, 67002201, 131650, 132181, };

//  m_UseType = USE_TYPE["UT_ATTACH_ANIM"] Ÿ���� �׼������� SKIN_MESH �� ���̴� �׼�����
const int ATTACHED_SKIN_MESH_FOR_ATTACH_ANIM[] = 
		{	
			88402,	88403,	88404,	88405,	88406,	88407, 
			88334,	88342,	88350,	88358,	88366,	88374,	88382,			// MK2 ����
			88447,	88455,	88463,	88471,	88479,	88487,	88495,			// MK2 ���� PC��
			88053,	88511,	88519,	88527,	88535,	88543,	88551,			// MKB ����
			88622,	88630,	88638,	88646,	88654,	88662,	88670,	88678,	// ����丣 ������ �Ƿ� ��
			60005674, 60005680
		};

#ifdef AVATAR_EMOTION

#define AVATAR_EMOTION_NUM 5

// ��� ��Ʈ
const int ITEM_ID_ET_EMOTION_AVATAR_ELSWORD1[]		= { 1009, 1010, };
const int ITEM_ID_ET_EMOTION_AVATAR_ARME1[]			= { 1011, 1012, };
const int ITEM_ID_ET_EMOTION_AVATAR_LIRE1[]			= { 1013, 1014, };
const int ITEM_ID_ET_EMOTION_AVATAR_RAVEN1[]		= { 1015, 1016, };
const int ITEM_ID_ET_EMOTION_AVATAR_EVE1[]			= { 1017, 1018, };
const int ITEM_ID_ET_EMOTION_AVATAR_CHUNG1[]		= { 1019, 1020, };

// �Ѻ� ��Ʈ	( 1474 ~ 1479 - 2012�� �αù����� ), ( 2382 ~ 2388 - 2013�� ���� )
const int ITEM_ID_ET_EMOTION_AVATAR_ELSWORD2[]		= { 1474, 2382, 10000920 };
const int ITEM_ID_ET_EMOTION_AVATAR_ARME2[]			= { 1475, 2383, 10000921 };
const int ITEM_ID_ET_EMOTION_AVATAR_LIRE2[]			= { 1476, 2384, 10000922 };
const int ITEM_ID_ET_EMOTION_AVATAR_RAVEN2[]		= { 1477, 2385, 10000923 };
const int ITEM_ID_ET_EMOTION_AVATAR_EVE2[]			= { 1478, 2386, 10000924 };
const int ITEM_ID_ET_EMOTION_AVATAR_CHUNG2[]		= { 1479, 2387, 10000925 };
#ifdef ADD_ARA_EMOTION
const int ITEM_ID_ET_EMOTION_AVATAR_ARA2[]			= { 2388, 10000926 };
#endif ADD_ARA_EMOTION

// ������ũ �㽬
const int ITEM_ID_ET_EMOTION_AVATAR_ELSWORD3[]		= { 1545, };
const int ITEM_ID_ET_EMOTION_AVATAR_ARME3[]			= { 1546, };
const int ITEM_ID_ET_EMOTION_AVATAR_LIRE3[]			= { 1547, };
const int ITEM_ID_ET_EMOTION_AVATAR_RAVEN3[]		= { 1548, };
const int ITEM_ID_ET_EMOTION_AVATAR_EVE3[]			= { 1549, };
const int ITEM_ID_ET_EMOTION_AVATAR_CHUNG3[]		= { 1550, };

// ������ũ õ��
const int ITEM_ID_ET_EMOTION_AVATAR_ELSWORD4[]		= { 1882, };
const int ITEM_ID_ET_EMOTION_AVATAR_ARME4[]			= { 1883, };
const int ITEM_ID_ET_EMOTION_AVATAR_LIRE4[]			= { 1884, };
const int ITEM_ID_ET_EMOTION_AVATAR_RAVEN4[]		= { 1885, };
const int ITEM_ID_ET_EMOTION_AVATAR_EVE4[]			= { 1886, };
const int ITEM_ID_ET_EMOTION_AVATAR_CHUNG4[]		= { 1887, };

// ũ������
const int ITEM_ID_ET_EMOTION_AVATAR_ELSWORD5[]		= { 2694, };
const int ITEM_ID_ET_EMOTION_AVATAR_ARME5[]			= { 2695, };
const int ITEM_ID_ET_EMOTION_AVATAR_LIRE5[]			= { 2696, };
const int ITEM_ID_ET_EMOTION_AVATAR_RAVEN5[]		= { 2697, };
const int ITEM_ID_ET_EMOTION_AVATAR_EVE5[]			= { 2698, };
const int ITEM_ID_ET_EMOTION_AVATAR_CHUNG5[]		= { 2699, };
const int ITEM_ID_ET_EMOTION_AVATAR_ARA5[]			= { 2700, };
#endif


// ���ۺ� ���� Item ID
const int ITEM_ID_JIGGLE_TYPE1[] = { 130646, 130647, 130648, 130649, 130650, 131263, 132207, 132208, 132209, 132210, 132211, 81176, 180154, 81939,// (����)
									268640,	268641, 
									81947, // �� ������ �屳 ���� ���� (�����ý�)
									183241, // ���� ��� ����
									183247, // ���� ��� ����(30��)
									85003473, // �Ƹ����� ������ �ҷ��� ����
};



//{{ kimhc // 2009-10-28 // Ư�� ������ ���� �� ������ ������ ������ ���� �ʵ��� �ϴ� ���
#ifdef	NOT_RENDERING_OTHER_EQUIP_POS
// ���, �Ǽ���(��), �Ǽ���(��) �� �ȳ����� �ؾ��ϴ� ������
static const int ITEM_ID_NOT_RENDERING_HAIR_FACE2_FACE3[] = { 131192, 216400, 216410, 131845, 131851, 131852, 131853, 131854, 132845,
	132847, 227640, 182544, 60004225, 60004226, 60004227, 60004228, 60004229, 60004230, 60004231, 60004232, 60004233, 60004234, 60004235,
	60004236, 60004237, 60004238, 60004239, 80001862, 65000344, 65000345, 65001009, 65001010 , 65001011, 65001012, 65001013, 85001851,
	85001852, 70000148, 70000149, 130000165, 130000166, 130000167, 130000168, 130000169, 85003471, 85003472, 85003474, 85003475, 70007280, };
// ��� �� �ȳ����� �ؾ��ϴ� ������
static const int ITEM_ID_NOT_RENDERING_HAIR[] = { 131471, 131353, 233220, 240640, 180187, 242420, 242430, 242440, 242450, 243340,
	182500, 182501, 182502, 182503, 182504, 182505, 182506, 182558, 183198, 183199, 183200, 183201, 183202, 183203, 183204, 271150, 271152, 183244, 183246, 242454, 242455, 242456,
	64000047, 64000050, 60005040, 60005041, 60005042, 60005043, 60005044, 60005045, 200000013, 200000014, 200000015, 200000016, 200000017, 200000018,
	60005060, 60005070, 60005241, 60005242, 60005243, 60005244, 60005245, 60005246, 65000441, 65000443, 67005251, 65000633, 65000636, 67003581, 67004379,
	60001513, 60001514, 60001864, 60001865, 60001866, 60001867, 60001877, 60001878, 60003994, 60003997, 60001385, 60001386, 60001387, 60001496, 
	85001041, 85001042, 85001043, 85001044, 85001045, 85001965,	80000780, 80000783, 80000786, 80000789, 130000702, 130000705,141000202, 141000203, 141000204, 141000205, 130000702, 130000705,
	67006322, 67006323, 67006324, 67006325, 67006326, 67006327, 67006328, 67006340, 67006341, 67006342, 67006343, 67006344, 67006345, 67006346, 152000201, 85003474, 85003475,
	80002300, 80002301, 80002302, 80002303, 80002304, 80002305, 80002306, 80002310, 80002311, 80002312, 80002313, 80002314, 80002315,
	67006390, 67006391, 67006392, 67006393, 67006394, 67006395, 67006396,
	};
// 2012.12.02 darkstarbt_������ // �ѹ� �ƹ�Ÿ �ε� ���Ƕ� ���ǰ� ��� �־ ���� �԰� �ִ� �ƹ�Ÿ ���� ���;� �ϴ� ���
static const int ITEM_ID_ONE_PIECE_AVATAR_RENDERING_BODY[] = { 153000086,153000087,153000088,153000089,153000090,153000091,153000092,153000093,
};

#ifdef ONE_PIECE_AVATAR
static const int ITEM_ID_ONE_PIECE_AVATAR[] = { 180059, 180060, 180061, 180062, 180063, 180064, 236890, 236900, 236910, 236920,
	236930, 236940, 180168, 180170, 180172, 180174, 180176, 180178, 247560, 247570, 247580, 247590, 247600, 247610, 181475, 181477,
	181479, 181481, 181483, 181485, 
	64000004, 60004145, 60004264, 67006480, 67006481, 67006482, 67006483, 67006484, 67006485, 60007820, 60007821, 60007822, 60007823,
	60007824, 60007825,
	};
static const int ITEM_ID_ONE_PIECE_AVATAR_NOT_RENDERING_FACE1[] = { 237290, 237300, 237310, 237320, 237330, 237340, 240250, 240260,
	240270, 240280, 240290, 240300, 240310, 240320, 240330, 240340, 240350, 240360, 240370, 240380, 240390, 240400, 240410, 240420,
	240430, 240440, 240450, 240460, 240470, 240480, 240490, 240500, 240510, 240520, 240530, 240540, 240550, 240560, 240570, 240580,
	240590, 240600, 244200, 244210, 244220, 244230, 244240, 244250, 244260, 244270, 244280, 244290, 244300, 244310, 262900, 262910,
	262920, 262930, 262940, 262950, 262960, 262970, 262980, 262990, 263000, 263010, 263020, 263030, 182558, };
static const int ITEM_ID_ONE_PIECE_AVATAR_NOT_RENDERING_FACE_FACE2_FACE3[] = { 180303, 180304, 180305, 180306, 180307, 180308, 182561, };
// �ѹ� �ƹ�Ÿ �ܿ� ���� ��� �Ǽ������� �ȳ����� �ؾ��ϴ� ������
static const int ITEM_ID_ONE_PIECE_AVATAR_NOT_RENDERING_HAIR_AND_ALL_ACCESSORY[] = { 183206, 183207, 183208, 183209, 183210, 183211, 183212, 183213,
	67006490, 67006491, 67006492, 67006493, 67006494, 67006495, 67006496, 67006497, 
	153000117,153000118,153000119,153000120,153000121,153000122,153000123,153000124,
	153000125,153000126,153000127,153000128,153000129,153000130,153000131,153000132,
	};
#ifdef SERV_NEW_ONE_PIECE_AVATAR_SLOT
	static const int ITEM_ID_ONE_PIECE_AVATAR_RENDERING_HAIR[] = { 67006464, 67006465, 67006466, 67006467, 67006468, 67006469, 67006470, 67006471, 67006472, 67006473,
	67006474, 67006475, 67006476, 67006477, 80002420, 80002421, 80002422, 80002423, 80002424, 80002425,

	};
#endif SERV_NEW_ONE_PIECE_AVATAR_SLOT
#endif ONE_PIECE_AVATAR
#ifdef FACE_OFF_MONSTER_HEAD_TEST
static const int ITEM_ID_NOT_RENDERING_HAIR_FACE_FACE2_FACE3[] = { 180066, 180067, 180068, 180069, 182559,153000104, };
static const int ITEM_ID_NOT_RENDERING_HAND[] = { 180071, 242380, 242390, 242400, 242410, 160403, 242451, 242452, 242453, 67005252, 82242380, 82242390, 82242400, 82242410, };
#endif FACE_OFF_MONSTER_HEAD_TEST
#ifdef NOT_RENDERING_NASOD_HAND_POS
static const int ITEM_ID_NOT_RENDERING_NASOD_HAND_POS[] = { 180071, 242380, 242390, 242400, 242410, 243320, 160403, 249290, 30140, 242451, 242452, 242453, 
	60006062, 60006063, 60006064, 60006065, 60006066, 60006067, 67005252, 60005450, 60005451, 82242380, 82242390, 82242400, 82242410, };
static const int ITEM_ID_NOT_RENDERING_NASOD_ARM_POS[] = { 180166, 180167, 181497, 181498, 30149 };
#endif NOT_RENDERING_NASOD_HAND_POS 
#ifdef NOT_RENDERING_NASOD_HAND
static const int ITEM_ID_NOT_RENDERING_NASOD_HAND[] = { 180174, 181481, 67006483 };
#endif NOT_RENDERING_NASOD_HAND

#ifdef ADDED_RELATIONSHIP_SYSTEM
static const int ITEM_ID_WEDDING_CLOTHES[] = { 183206, 183207, 183208, 183209, 183210, 183211, 183212, 183213,
		67006490, 67006491, 67006492, 67006493, 67006494, 67006495, 67006496, 67006497,
};
#endif //ADDED_RELATIONSHIP_SYSTEM

#ifdef BUY_ONLY_ARA_ITEM
//�ƶ� �� �� �ִ� �����ۿ� ���� ����ó��
static const int ITEM_ID_BUY_ONLY_ARA_ITEM[] = { 261590, 261600, 261610, 261620, 500790, 500800 };
#endif //BUY_ONLY_ARA_ITEM

#ifdef ADDED_RELATIONSHIP_SYSTEM
inline bool IsWeddingClothesItem( int iItemID_ )
{
	int iArraySize = ARRAY_SIZE( ITEM_ID_WEDDING_CLOTHES );

	for ( int i = 0 ; i < iArraySize; ++i )
	{
		if( iItemID_ == ITEM_ID_WEDDING_CLOTHES[i] )
			return true;
	}

	return false;
}
#endif // ADDED_RELATIONSHIP_SYSTEM

inline bool IsAttachedSkinMeshForAttachAnim( int iItemID )	// m_UseType = USE_TYPE["UT_ATTACH_ANIM"] Ÿ���� �׼������� SKIN_MESH �� ���̴� �������ΰ�?
{
	int iArraySize = ARRAY_SIZE( ATTACHED_SKIN_MESH_FOR_ATTACH_ANIM );

	for ( int i = 0 ; i < iArraySize; ++i )
	{
		if( iItemID == ATTACHED_SKIN_MESH_FOR_ATTACH_ANIM[i] )
			return true;
	}

	return false;
}

inline bool IsNoRenderHairFace2Face3Equip( int iItemID )	// ���, �Ǽ���(��), �Ǽ���(��) �� �ȳ����� �ؾ��ϴ� ������ �ΰ�?
{
	int iArraySize = ARRAY_SIZE( ITEM_ID_NOT_RENDERING_HAIR_FACE2_FACE3 );
	
	for ( int i = 0; i < iArraySize; i++ )
	{
		if( iItemID == ITEM_ID_NOT_RENDERING_HAIR_FACE2_FACE3[i] )
			return true;
	}

	return false;	
}

inline bool IsNoRenderHair( int iItemID )	// �� �ȳ����� �ϴ� ������ �ΰ�?
{
	int iArraySize = ARRAY_SIZE( ITEM_ID_NOT_RENDERING_HAIR );

	for ( int i = 0; i < iArraySize; i++ )
	{
		if( iItemID == ITEM_ID_NOT_RENDERING_HAIR[i] )
			return true;
	}

	return false;	
}
#ifdef ONE_PIECE_AVATAR
inline bool IsOnePieceAvatar( int iItemID )	// �ѹ� ���� �ΰ�?
{
	int iArraySize = ARRAY_SIZE( ITEM_ID_ONE_PIECE_AVATAR );

	for ( int i = 0; i < iArraySize; i++ )
	{
		if( iItemID == ITEM_ID_ONE_PIECE_AVATAR[i] )
			return true;
	}
	return false;	
}
inline bool IsOnePieceAvatarNotRenderFace1( int iItemID )	// �Ǽ���(��)�� ���� �ѹ� ���� �ΰ�?
{
	int iArraySize = ARRAY_SIZE( ITEM_ID_ONE_PIECE_AVATAR_NOT_RENDERING_FACE1 );

	for ( int i = 0; i < iArraySize; i++ )
	{
		if( iItemID == ITEM_ID_ONE_PIECE_AVATAR_NOT_RENDERING_FACE1[i] )
			return true;
	}
	return false;	
}
inline bool IsOnePieceAvatarNotRenderFace( int iItemID )	// �⺻ ���� ���� �ѹ� ���� �ΰ�?
{
	int iArraySize = ARRAY_SIZE( ITEM_ID_ONE_PIECE_AVATAR_NOT_RENDERING_FACE_FACE2_FACE3 );

	for ( int i = 0; i < iArraySize; i++ )
	{
		if( iItemID == ITEM_ID_ONE_PIECE_AVATAR_NOT_RENDERING_FACE_FACE2_FACE3[i] )
			return true;
	}
	return false;	
}
inline bool IsOnePieceAvatarNotRenderHairAndAllAccessory( int iItemID )	// ���� ��� �Ǽ������� ���� �ѹ� ���� �ΰ�?
{
	int iArraySize = ARRAY_SIZE( ITEM_ID_ONE_PIECE_AVATAR_NOT_RENDERING_HAIR_AND_ALL_ACCESSORY );

	for ( int i = 0; i < iArraySize; i++ )
	{
		if( iItemID == ITEM_ID_ONE_PIECE_AVATAR_NOT_RENDERING_HAIR_AND_ALL_ACCESSORY[i] )
			return true;
	}
	return false;	
}
#ifdef SERV_NEW_ONE_PIECE_AVATAR_SLOT
inline bool IsOnePieceAvatarRenderHair( int iItemID ) // ���� ��� ���̴� �� �� �ƹ�Ÿ
{
	int iArraySize = ARRAY_SIZE( ITEM_ID_ONE_PIECE_AVATAR_RENDERING_HAIR );

	for ( int i = 0; i < iArraySize; i++ )
	{
		if( iItemID == ITEM_ID_ONE_PIECE_AVATAR_RENDERING_HAIR[i] )
			return true;
	}
	return false;	
}

inline bool IsOnePieceAvatarWithNotRenderHairItem( int iItemID ) // 
{
	int iArraySize = ARRAY_SIZE( ITEM_ID_NOT_RENDERING_HAIR );

	for ( int i = 0; i < iArraySize; i++ )
	{
		if( iItemID == ITEM_ID_NOT_RENDERING_HAIR[i] )
			return true;
	}
	return false;	
}
// 2012.12.02 darkstarbt_������ // �ѹ� �ƹ�Ÿ �ε� ���Ƕ� ���ǰ� ��� �־ ���� �԰� �ִ� �ƹ�Ÿ ���� ���;� �ϴ� ���
inline bool IsOnePieceAvatarWithRenderBodyItem( int iItemID ) // 
{
	int iArraySize = ARRAY_SIZE( ITEM_ID_ONE_PIECE_AVATAR_RENDERING_BODY );

	for ( int i = 0; i < iArraySize; i++ )
	{
		if( iItemID == ITEM_ID_ONE_PIECE_AVATAR_RENDERING_BODY[i] )
			return true;
	}
	return false;	
}

#endif SERV_NEW_ONE_PIECE_AVATAR_SLOT
#endif ONE_PIECE_AVATAR
#ifdef FACE_OFF_MONSTER_HEAD_TEST
inline bool IsNoRenderHairFaceFace2Face3Equip( int iItemID ) // ���, �⺻��, �Ǽ���(��), �Ǽ���(��) �� �ȳ����� �ؾ��ϴ� ������ �ΰ�?
{
	int iArraySize = ARRAY_SIZE( ITEM_ID_NOT_RENDERING_HAIR_FACE_FACE2_FACE3 );

	for ( int i = 0; i < iArraySize; i++ )
	{
		if( iItemID == ITEM_ID_NOT_RENDERING_HAIR_FACE_FACE2_FACE3[i] )
			return true;
	}

	return false;	
}
inline bool IsNoRenderHandEquip( int iItemID ) // �尩�� �ȳ����� �ؾ��ϴ� ������ �ΰ�?
{
	int iArraySize = ARRAY_SIZE( ITEM_ID_NOT_RENDERING_HAND );

	for ( int i = 0; i < iArraySize; i++ )
	{
		if( iItemID == ITEM_ID_NOT_RENDERING_HAND[i] )
			return true;
	}

	return false;	
}
#endif FACE_OFF_MONSTER_HEAD_TEST
#ifdef NOT_RENDERING_NASOD_HAND_POS
inline bool IsNoRenderNasodHandPos( int iItemID) // ���ҵ� �ڵ� ��ġ�� �ȳ����� �ؾ��ϴ� ������ �ΰ�?
{	
	int iArraySize = ARRAY_SIZE( ITEM_ID_NOT_RENDERING_NASOD_HAND_POS );

	for ( int i = 0; i < iArraySize; i++ )
	{
		if( iItemID == ITEM_ID_NOT_RENDERING_NASOD_HAND_POS[i] )
			return true;
	}

	return false;	
}
inline bool IsNoRenderNasodArmPos( int iItemID) // ���ҵ� �ڵ� ��ġ�� �ȳ����� �ؾ��ϴ� ������ �ΰ�?
{	
	int iArraySize = ARRAY_SIZE( ITEM_ID_NOT_RENDERING_NASOD_ARM_POS );

	for ( int i = 0; i < iArraySize; i++ )
	{
		if( iItemID == ITEM_ID_NOT_RENDERING_NASOD_ARM_POS[i] )
			return true;
	}

	return false;
} 
#endif NOT_RENDERING_NASOD_HAND_POS 
#ifdef NOT_RENDERING_NASOD_HAND
inline bool IsNoRenderNasodHand( int iItemID) // ���ҵ� �ڵ� ������ �ʰ� �ؾ��ϴ� ������ �ΰ�?
{	
	int iArraySize = ARRAY_SIZE( ITEM_ID_NOT_RENDERING_NASOD_HAND );

	for ( int i = 0; i < iArraySize; i++ )
	{
		if( iItemID == ITEM_ID_NOT_RENDERING_NASOD_HAND[i] )
			return true;
	}

	return false;	
}
#endif NOT_RENDERING_NASOD_HAND

#ifdef BUY_ONLY_ARA_ITEM
//�ƶ� �� �� �ִ� �����ۿ� ���� ����ó��
inline bool IsCanByeOnlyAraItem( int iItemID ) // �ƶ� �� �� �ִ� �������ΰ�?
{	
	int iArraySize = ARRAY_SIZE( ITEM_ID_BUY_ONLY_ARA_ITEM );

	for ( int i = 0; i < iArraySize; i++ )
	{
		if( iItemID == ITEM_ID_BUY_ONLY_ARA_ITEM[i] )
			return true;
	}

	return false;	
}
#endif //BUY_ONLY_ARA_ITEM


#endif	NOT_RENDERING_OTHER_EQUIP_POS
//}} kimhc // 2009-10-28 // Ư�� ������ ���� �� ������ ������ ������ ���� �ʵ��� �ϴ� ���

//{{ kimhc // 2010.6.8 
#ifdef	HACKING_TEST
inline bool GetValueFromLua( KLuaManager* pLuaManger, char* pszName, KProtectedType<float>& var )
{
	if ( pLuaManger->GetTableDepth() > 0)
	{
		lua_pushstring( pLuaManger->GetLuaState(), pszName);
		lua_gettable( pLuaManger->GetLuaState(), -2);
	}
	else
	{
		lua_getglobal( pLuaManger->GetLuaState(), pszName);
	}

	if ( lua_isnumber( pLuaManger->GetLuaState(), -1 ) != 0)
	{
		var = static_cast<float>( lua_tonumber( pLuaManger->GetLuaState(), -1) );
		lua_pop( pLuaManger->GetLuaState(), 1 ); 
		return true;
	}
	else
	{
		lua_pop( pLuaManger->GetLuaState(), 1 );
		return false;
	}
}
#endif	HACKING_TEST
//}} kimhc // 2010.6.8 

static const int SKILL_SLOT_CHANGE_ITEM_ID			= 200880;		// ��ų ���� ü���� �����ϰ� �ϴ� ������ (�Ⱓ��)

//{{ 2011. 11. 30	������	��Ű�� ��ǰ �߰�
//#ifdef SERV_ADD_PACKAGE_PRODUCT
static const int SKILL_POINT_5_USE_INVEN_ITEM_ID		= 241780;		// �κ��丮���� Ŭ���ؼ� ����ϴ� �׳�ý� �ູ
static const int SKILL_POINT_5_USE_INVEN_ITEM_ID_7_DAY	= 268562;		// �κ��丮���� Ŭ���ؼ� ����ϴ� �׳�ý� �ູ( 7�� )
static const int SKILL_POINT_10_USE_INVEN_ITEM_ID		= 241790;		// �κ��丮���� Ŭ���ؼ� ����ϴ� �׳�ý� �ູ
//#endif SERV_ADD_PACKAGE_PRODUCT
//}}


// �׳�ý� �ູ, �޽��� �ڽ� ���� �ȵǴ� �͵� �߰� ����
static const int SKILL_POINT_5_USE_INVEN_ITEM_ID_15_DAY = 127180;
static const int SKILL_POINT_5_USE_INVEN_ITEM_ID_30_DAY = 127190;
static const int SKILL_POINT_5_USE_INVEN_ITEM_ID_60_DAY = 127200;

static const int SKILL_POINT_10_USE_INVEN_ITEM_ID_30_DAY = 261660;
// �׳�ý� �ູ, �޽��� �ڽ� ���� �ȵǴ� �͵� �߰� ����

#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - �׳�ý� ������ ���
static const int SKILL_POINT_30_USE_INVEN_ITEM_ID_15_DAY	= 271076;
static const int SKILL_POINT_60_USE_INVEN_ITEM_ID_15_DAY	= 271077;
static const int SKILL_POINT_30_USE_INVEN_ITEM_ID_30_DAY	= 271078;
static const int SKILL_POINT_60_USE_INVEN_ITEM_ID_30_DAY	= 271079;
#endif // UPGRADE_SKILL_SYSTEM_2013




static const int INIT_SKILL_TREE_ITEM_ID			= 203800;		// ��ų ��ü �ʱ�ȭ
static const int INIT_SKILL_TREE_EVENT_ITEM_ID		= 130633;		// ��ų ��ü �ʱ�ȭ �̺�Ʈ ������


static const int INIT_SKILL_TREE_EVENT_ITEM_REMAINING_ONE_DAY_FROM_QUEST_ID	  = 135416;		// 2013 ��ų ���� ���, ��ų ��ü �ʱ�ȭ 1�ϱ� �̺�Ʈ ������, ����Ʈ �������� ����
static const int INIT_SKILL_TREE_EVENT_ITEM_REMAINING_THREE_DAY_FROM_QUEST_ID = 135417;		// 2013 ��ų ���� ���, ��ų ��ü �ʱ�ȭ 3�ϱ� �̺�Ʈ ������, ����Ʈ �������� ����

static const int RESET_A_SKILL_ITEM_ID				= 200730;		// ��ų 1�� �ǵ����� ������
static const int RESET_A_SKILL_ITEM_NOT_TRADE_ID	= 135433;		// [�ں�] ������ �˾�

static const int RESET_A_SKILL_EVENT_ITEM_ID		= 130808;		// ��ų 1�� �ǵ����� ������
#ifdef SERV_EVENT_CASH_SKILL_POINT_ITEM_JP
static const int RESET_A_SKILL_EVENT_ITEM_ID_BY_JP	= 67003592;		// ������ �˾�(��ų 1�� �ǵ����� ������)
#endif //SERV_EVENT_CASH_SKILL_POINT_ITEM_JP

#ifdef SERV_SKILL_5_POINT_7_DAY_EU
static const int EVENT_SKILL_5_POINT_7_DAY	= 90000690;	
#endif SERV_SKILL_5_POINT_7_DAY_EU

#ifdef SERV_CN_GNOSIS
static const int EVENT_SKILL_POINT_10_USE_INVEN_7_DAY	= 80001150;	//(�߱�) �̺�Ʈ�� �׳�ý��� �ູ ( 10 SP, 7�ϱ�) // 2012.05.24 lygan_������
static const int EVENT_SKILL_POINT_5_USE_INVEN_1_DAY	= 80001327;	//(�߱�) �̺�Ʈ�� �׳�ý��� �ູ ( 5 SP, 1�ϱ�) // 2012.06.25 lygan_������

static const int EVENT_SKILL_POINT_5_USE_INVEN_7_DAY		= 80001355;	//(�߱�) �̺�Ʈ�� �׳�ý��� �ູ ( 5 SP, 7�ϱ�) // 2012.07.02 lygan_������
static const int EVENT_SKILL_POINT_5_USE_INVEN_7_DAY_TYPE2	= 80001372;	//(�߱�) �̺�Ʈ�� �׳�ý��� �ູ ( 5 SP, 7�ϱ�) // 2012.07.02 lygan_������ // �̰� ���� ���� ���� ���� ���� �ʴ� ������
static const int EVENT_SKILL_POINT_10_USE_INVEN_15_DAY		= 80001367;	//(�߱�) �̺�Ʈ�� �׳�ý��� �ູ ( 10 SP, 15�ϱ�) // 2012.07.02 lygan_������
#endif SERV_CN_GNOSIS

#ifdef SERV_US_GNOSIS
static const int EVENT_SKILL_POINT_5_USE_INVEN_15_DAY	= 90000747;
#endif SERV_US_GNOSIS
#ifdef SERV_COUNTRY_JP
static const int INIT_SKILL_TREE_EVENT_ITEM_ID_JP	= 65000430;		// ��ų ��ü �ʱ�ȭ �̺�Ʈ ������ ���� 130633
#endif //SERV_COUNTRY_JP

#ifdef SERV_QUEST_SKILL_RESET
static const int RESET_A_SKILL_QUEST_ITEM_ID		= 141000000;	// ��ų 1�� �ǵ����� ������
#endif SERV_QUEST_SKILL_RESET

#ifdef SERV_2ND_CLASS_SKILL_RESET
static const int RESET_A_SKILL_2ND_CLASS_CHANGE_EVENT_ITEM_ID		= 141000036;	// ��ų 1�� �ǵ����� ������
#endif SERV_2ND_CLASS_SKILL_RESET



#ifdef GUILD_SKILL
static const int INIT_GUILD_SKILL_TREE_ITEM_ID		= 110858;		// ��� ��ų ��ü �ʱ�ȭ
static const int RESET_A_GUILD_SKILL_ITEM_ID		= 110857;		// ��� ��ų 1�� �ǵ����� ������

static const int GUILD_CASH_SKILL_ITEM_ID			= 110859;		// ��� ��ų �Ⱓ�� + 5

#endif GUILD_SKILL

static const int WARP_ITEM_RUBEN_ITEM_ID					= 109995;		// �̵� Ƽ�ϵ�
static const int WARP_ITEM_ELDER_ITEM_ID					= 109996;
static const int WARP_ITEM_BESMA_ITEM_ID					= 109997;
static const int WARP_ITEM_ALTERA_ITEM_ID					= 109998;
static const int WARP_ITEM_PEITA_DUNGEON_GATE_ITEM_ID		= 110832;
static const int WARP_ITEM_VELDER_ITEM_ID					= 110847; 
static const int WARP_ITEM_FREE_ITEM_ID						= 215660;		// �̵�Ƽ�� ���� �̿�� (215660)
static const int WARP_ITEM_ED_CONSUMPTION_ITEM_ID			= 112323;		// ED�Ҹ� �̵�Ƽ�� �̿�� (215660)

#ifdef SERV_UNLIMITED_SECOND_CHANGE_JOB
static const int UNLIMITED_SECOND_CHANGE_JOB_ITEM_ID		= 153000168;	// 2������ ������ �����
#endif SERV_UNLIMITED_SECOND_CHANGE_JOB

static const int SKILL_NOTE_ITEM_ID							= 99600;
static const int CASH_SKILL_NOTE_ITEM_ID					= 221600;

#ifdef SPECIAL_USE_ITEM
static const int NO_CUBE_OPEN_ITEM_ID[]						= { 77200, 78460, 78470, 78480, 78490, 78500, 78510, 65000360,}; 
#else
static const int SPIRIT_POTION_ITEM_ID						= 77200;	// �ټ� ����
#endif SPECIAL_USE_ITEM

#ifdef VARIOUS_RANDOM_CUBE_UI_NEW
static const int MEMO_RANDOM_CUBE_ITEM_ID					= 132817;	// ���� ã�� ����
static const int CAPSULE_RANDOM_CUBE_ITEM_ID				= 133340;	// �ų� � ĸ��
static const int TREE_DAY_CUBE_ITEM_ID						= 131859;	// oasis907 : ����� [2011.3.29] // �ĸ��� ť�� ������
#ifdef NEW_HENIR_TEST
static const int NEW_HENIR_TIMESPACE_PIECE					= 61205;	 // �ð����� ���� 
static const int NEW_HENIR_CUBE_ITEM_ID						= 61206;	// ������ �ð��� ����
#endif NEW_HENIR_TEST
#endif VARIOUS_RANDOM_CUBE_UI_NEW

#ifdef SERV_EXPAND_QUICK_SLOT
#if defined(SERV_COUNTRY_TWHK) || defined(SERV_COUNTRY_JP)
static const int EXPAND_QUICK_SLOT_ITEM_ID					= 70002820;	// ������ Ȯ���(�븸, �Ϻ�)
#else
static const int EXPAND_QUICK_SLOT_ITEM_ID					= 244560;
#endif SERV_COUNTRY_TWHK
#endif SERV_EXPAND_QUICK_SLOT
#ifdef SKILL_PLUS_ITEM_USE_POPUP
#ifdef SERV_EVENT_CASH_SKILL_POINT_ITEM_JP
static const int SKILL_PLUS_ITEM_ID_JP						= 67000382;	// �̺�Ʈ�� �׳�ý� �ູ(2��) ���� 133131
static const int SKILL_POINT_10_30DAY_USE_INVEN_JP			= 67005710;	// �̺�Ʈ�� �׳�ý� �ູ 10sp 30��
static const int SKILL_POINT_10_15DAY_USE_INVEN_JP			= 70007020; // �̺�Ʈ�� �׳�ý� �ູ(10P 15�ϱ�)
static const int SKILL_POINT_5_7DAY_USE_INVEN_JP			= 70007030; // �̺�Ʈ�� �׳�ý� �ູ(5P 7�ϱ�)
#endif //SERV_EVENT_CASH_SKILL_POINT_ITEM_JP
static const int SKILL_PLUS_ITEM_ID							= 133131;   // �׳�ý� �ູ

#ifdef SERV_EVENT_CASH_SKILL_POINT_ITEM_TWHK
static const int SKILL_POINT_30_7DAY_USE_INVEN				= 60003758; // �̺�Ʈ�� �׳�ý� �ູ 30SP 7��
static const int SKILL_POINT_60_7DAY_USE_INVEN				= 60003759; // �̺�Ʈ�� �׳�ý� �ູ 60SP 7��
static const int SKILL_POINT_60_15DAY_USE_INVEN				= 60004280; // �̺�Ʈ�� �׳�ý� �ູ 60SP 15��
static const int SKILL_POINT_60_30DAY_USE_INVEN				= 60005011;	// �̺�Ʈ�� �׳�ý� �ູ 60SP 30��
#endif SERV_EVENT_CASH_SKILL_POINT_ITEM_TWHK
#ifdef SERV_EVENT_SKILL_POINT_130_1DAY_USE_INVEN
static const int SKILL_POINT_130_1DAY_USE_INVEN				= 67006450; // �̺�Ʈ�� �׳�ý� �ູ 130sp 1��
#endif SERV_EVENT_SKILL_POINT_130_1DAY_USE_INVEN
#ifdef SERV_EVENT_SKILL_POINT_1DAY_USE_INVEN
static const int SKILL_POINT_30_1DAY_USE_INVEN				= 67006500; // �̺�Ʈ�� �׳�ý� �ູ 30sp 1��
static const int SKILL_POINT_60_1DAY_USE_INVEN				= 67006501; // �̺�Ʈ�� �׳�ý� �ູ 60sp 1��
#endif SERV_EVENT_SKILL_POINT_1DAY_USE_INVEN
static const int SKILL_POINT_60_7DAY_USE_INVEN_2			= 60008007;	// �̺�Ʈ�� �׳�ý� �ູ 60SP 7��
#endif SKILL_PLUS_ITEM_USE_POPUP

//{{ ����� : [2009/7/14] //	���ҵ� �ް��� ���� ���
#ifdef NASOD_SCOPE

static const int NASOD_SCOPE_ITEM_ID				= 212350;
static const int NASOD_SCOPE_HIGH_ITEM_ID			= 212360;

#endif
//}} ����� : [2009/7/14] //



#ifdef ATTRACTION_ITEM_TEST
	static const int ATTRACTION_ITEM_ID[]				= { 108900, 108901, 108902, 160259, 160286, 160287, 160324, 160325, 160377, 160559, 160597, 160643, 160666, 160710, 160747, 160867,
	60000700, 60000701, 60000702, 60000703, 60000704, 60000705, 60000706, 60000707, 60000708, 60001619, 64000000, 64000001, 64000002, 64000100, 64000101, 64000102, 64000103, 64000104, 
	60004960, 60004970, 64000105, 64000106, 64000107, 60006151, 60007370, 65000031, 70005100, 
	85001530, 85001531, 85002440, 85002450, 140000110, 70007250, 70007252, 84001844, 70007307, 70007306, 67006284, 67006310, 210000226,
	90000076, 90000595, 90001060, 90001594, 90001900, 90001970, 85003722, }; // ���� �������
	static const int GOLD_ATTRACTION_ITEM_ID[]			= { 135115, 85003720 }; // ��� ���� ������
#ifdef SERV_SERV_MINI_RANDOM_CUBE_REWARD
	static const int MINI_ATTRACTION_ITEM_ID[]			= { 160656, 268544 }; // �̴� ���� ������
#endif SERV_SERV_MINI_RANDOM_CUBE_REWARD
#ifdef SERV_ALL_IN_ONE_RANDOM_CUBE_REWARD
	static const int ALL_IN_ONE_ATTRACTION_ITEM_ID = 160772;
	static const int ALL_IN_ONE_ATTRACTION_BONUS_ITEM_ID = 92377;
#endif //SERV_ALL_IN_ONE_RANDOM_CUBE_REWARD
#ifdef RIDING_SYSTEM
	static const int ANCIENT_FOSSIL_RIDING_PET = 160773;
	static const int ANCIENT_FOSSIL_PIECE = 92378;
#endif //RIDING_SYSTEM
#ifdef SERV_HALLOWEEN_ICE_HEATER
	static const int HALLOWEEN_ATTRACTION_ITEM_ID[] = { 85003498, 85003500 };
#endif //SERV_HALLOWEEN_ICE_HEATER
	static const int ATTRACTION_BONUS_ITEM_ID = 91610;		// ��������
	static const int ATTACH_ITEM_ITEM_ID[]				= { 130147, 130148, 130149, 130150, 130151, 130152, 130543, 130544, 132495, 132496, 132497, 132498, 132499, 132500, 127150, 127160, 127170,	135076, 135427, 135428,
	84000065, 84000003, 85000016, 85002325, 60005987, 85002326, 85002101, 60007250, 60007260, 60007270, 60007280, 60007290, 60007300, 90002373, 152000278, }; // ��Ŭ���ؼ� �ٸ� �����ۿ� ���̴� �����۵�. ���߿��� ��ũ��Ʈ�� ����
	static const int ENCHANT_COUPON_ITEM_ID[]			= { -1, -1, -1, -1, -1, 130147, 130148, 130149, 130150, 130151, 130152, 127150, 127160, 127170, };
	static const int DEFENCE_ENCHANT_COUPON_ITEM_ID[]	= { -1, -1, -1, -1, -1, 130543, 130544, 85002325, };

#ifdef INT_ENCHANT_COUPON_ITEM
	static const int INT_ENCHANT_COUPON_ITEM_ID[]	= { 84000065, 84000003, 60004196, 60004197, 60004198, 85000016, 85002325, 60005987, 85002326, 85002101, 60007250, 60007260, 60007270, 60007280, 60007290, 60007300, 85002325, 90002373};
#endif //INT_ENCHANT_COUPON_ITEM

//{{ kimhc // 2010.6.7 // �ŷ��� �Ұ����� ��ȭ�� �߰�

#ifdef ADDED_ENCHANT_COUPON_HAVE_LEVEL_LIMIT						// ������ / 13-07-03 / ���� ������ ���� ��ȭ�� ����
	static const int HAVE_LIMIT_LEVEL_ENCHANT_COUPON_ITEM_ID[]	= { 135427, 135428, };
	static const int HAVE_LIMIT_LEVEL_ENCHANT_COUPON_ITEM_LIMIT_LEVEL[] = { 0, 30, 40, -1, };
	// �Ʒ� 0 ~ 1�� �迭 = �� �迭 0���� ���� ���� 
	// ( Ex. 130543 ������ ID �� 1 ~ 30 �� ���� ����, 130544 �� 31 ~ 40 

	static const int HAVE_LIMIT_LEVEL_ENCHANT_COUPON_ITEM_ENCHANT_VALUE[]	= { 10, 10 };
	// ��æƮ ���

#endif // ������ / 13-07-03 / ���� ������ ���� ��ȭ�� ����

#ifdef	NO_TRADE_ATTACH_ITEM
	static boost::array<int, 38> ENCHANT_COUPON_ITEM_TO_NOT_TRADE_ID = { -1, -1, -1, -1, -1,  132495, 132496, 132497, 132498, 132499, 132500, 127150, 127160, 127170, 135076, 60004196, 60004197, 60004198, 85000016, 85002325, 60005987, 85002326, 85002101, 60007250, 60007260, 60007270, 60007280, 60007290, 60007300,
	85002810, 85002820, 85002830, 85002840, 85002850, 85002860, 84000065, 84000003, 152000278 };
#endif	NO_TRADE_ATTACH_ITEM
//}} kimhc // 2010.6.7 // �ŷ��� �Ұ����� ��ȭ�� �߰�
#endif

#ifdef SERV_ATTRIBUTE_CHARM // oasis907 : ����� [2011.1.5] // �Ӽ� ��ȭ ���� 
	static const int ATTRIBUTE_CHARM_COUPON_ITEM_ID[]	= {133365, 133366, 133367, 133368, 133369, 133370, 133371, 133372, 133373, 133374, 133375, 133376, 133377, 133378, 133379, };
#endif SERV_ATTRIBUTE_CHARM

static const int NICKNAME_CHANGE_CARD_ITEM_ID		= 209660;

// Īȣ ������ �ߺ���� �Ұ�ó��
static const int TITLE_ITEM[]	= { 130063, 	130158, 	130255, 	130506, 	130591, 	130723, 	213170, 	131131, 	131135,		131151, 	132697, 	111086,
									160037, 	132795, 	60002740,	60002741,	60003471,	160228, 	60004107,	60004108, 	60004109,	60004135,	60004147,	60004343,
									60004440, 	60004490, 	60004550,	60004750,	200000011,	60004882,	80001482,	80001483,	80001496,	60005217,
									210000211,	210000212,	210000213,	210000214,	210000215,	210000216,				90002240 };
static const int TITLE_ID[]		= { 80,			10140,		10160,		10210,		10230,		10260,  	10350,		10360,		10370,		10380,		10480,		10490,
									10670,  	10510,		5117,		5160,		5118,		10680,		5220, 		5230,		5231, 		5250, 		5270, 		5310,
									5320,		5330,		5340,		5350,		100000,		5360,		50080,		50081,		50082,		5380,
									100100,		100110,		100120,		100130,		100140,		100150,		100160,     35220 };

//#define ������ 110000
//#define ������ 110001



//{{ ����� : [2009/9/8] //	���â�� �� ������ ���Ǵ� ������
#ifdef GUILD_MANAGEMENT
static const int GUILD_CREATE_ITEM_ID		= 110845;	// ��� â�� ������
static const int GUILD_EXPANSION_ITEM_ID	= 110846;	// ��� �ο� Ȯ�� ������
static const int GUILD_QUEST_ID				= 7630;		// �ӽ�(�Ƹ����� �θ� lv.1����Ʈ)
static const int EGG_ITEM_ID				= 70000;	// �ް� ������ ID(X2Game�� UseItemSpecialAbility���� ���)

#endif	//	GUILD_MANAGEMENT
//}} ����� : [2009/9/8] //	���â�� �� ������ ���Ǵ� ������

#ifdef SERV_GUILD_CHANGE_NAME
static const int CHANGE_GUILD_NAME_ITEM_ID = 248400;
#endif SERV_GUILD_CHANGE_NAME

//{{ kimhc // 2009-10-21 // PaymentType�� �ڵ������� '��' �̶�� �۾��� �ٿ��ֱ�
#ifdef	AUTO_PAYMENT
static const int AUTO_PAYMENT_TYPE			= 13002; // �ڵ����� PAYMENT_TYPE
#endif	AUTO_PAYMENT
//}} kimhc // 2009-10-21 // PaymentType�� �ڵ������� '��' �̶�� �۾��� �ٿ��ֱ�



static const int BRONZE_ED_ITEM_ID		= 127000;		
static const int SILVER_ED_ITEM_ID		= 127010;		
static const int GOLD_ED_ITEM_ID		= 127020;	

static const int POSTBOX_ED_ITEM_ID		= 127040;	// �����Կ� ����ϴ� ED, �����߿� ��ӵ����� ����

#ifdef SERV_EVENT_DAILY_GIFT_BOX	// mauntain : ����ȯ [2012.06.11] ���ϸ��� ���� ���� ItemID
static const int DAILY_GIFT_BOX_ITEM[]	= { 251760, 251770 };
#endif SERV_EVENT_DAILY_GIFT_BOX




//#pragma NOTE( "���� ĳ�þ����� ���� �۾� ���⿡ ����� �մϴ�. ����ϴ� �� ��� �˻��ؼ� �߰����־�� �մϴ�." )
static const int CLASS_CHANGE_ELSWORD_KNIGHT_ITEM_ID   	= 214410;	/// �ҵ� ����Ʈ�� ����
static const int CLASS_CHANGE_MAGIC_KNIGHT_ITEM_ID     	= 214420;	/// ���� ����Ʈ�� ����
#ifdef ELSWORD_SHEATH_KNIGHT
static const int CLASS_CHANGE_SHEATH_KNIGHT_ITEM_ID     = 242310;	/// �ý� ����Ʈ�� ����
#endif ELSWORD_SHEATH_KNIGHT
static const int CLASS_CHANGE_COMBAT_RANGER_ITEM_ID    	= 214450;	/// �Ĺ� �������� ��ȥ
static const int CLASS_CHANGE_SNIPING_RANGER_ITEM_ID   	= 214460;	/// �������� �������� ��ȥ
#ifdef SERV_TRAPPING_RANGER_TEST
static const int CLASS_CHANGE_TRAPPING_RANGER_ITEM_ID	= 242330;	/// Ʈ���� �������� ��ȥ
#endif
static const int CLASS_CHANGE_HIGH_MAGICIAN_ITEM_ID    	= 214430;	/// ���� �������� �޸������
static const int CLASS_CHANGE_DARK_MAGICIAN_ITEM_ID    	= 214440;	/// ��ũ �������� �޸������
#ifdef SERV_ADD_ARME_BATTLE_MAGICIAN
static const int CLASS_CHANGE_BATTLE_MAGICIAN_ITEM_ID  	= 242320;	/// ��Ʋ �������� �޸������
#endif
static const int CLASS_CHANGE_SOUL_TAKER_ITEM_ID       	= 214470;	/// �ҵ� ����Ŀ�� ���
static const int CLASS_CHANGE_OVER_TAKER_ITEM_ID       	= 214480;	/// ���� ����Ŀ�� ���
#ifdef RAVEN_WEAPON_TAKER
static const int CLASS_CHANGE_WEAPON_TAKER_ITEM_ID  	= 242340;	/// ���� ����Ŀ�� ���
#endif RAVEN_WEAPON_TAKER
static const int CLASS_CHANGE_EXOTIC_GEAR_ITEM_ID      	= 214490;	/// �ڵ� ����ƽ�� �޸�
static const int CLASS_CHANGE_ARCHITECTURE_ITEM_ID     	= 214500;	/// �ڵ� ��Ű��ó�� �޸�
#ifdef EVE_ELECTRA
static const int CLASS_CHANGE_ELECTRA_ITEM_ID			= 242350;	/// �ڵ� �Ϸ�Ʈ���� �޸�
#endif EVE_ELECTRA

static const int CLASS_CHANGE_LORD_NIGHT_ITEM_ID		= 216420;	/// �ε� ����Ʈ �Ӹ���
static const int CLASS_CHANGE_RUNE_SLAYER_ITEM_ID		= 216430;	/// �� �����̾� ¡ǥ
static const int CLASS_CHANGE_WIND_SNEAKER_ITEM_ID		= 217850;	/// ���� ����Ŀ�� ���� ����
static const int CLASS_CHANGE_GRAND_ARCHER_ITEM_ID		= 217860;	/// �׷��� ��ó�� �ü� ����

static const int CLASS_CHANGE_ELEMENTAL_MASTER_ITEM_ID	= 217920;	/// ������Ż �������� ���
static const int CLASS_CHANGE_VOID_PRINCESS_ITEM_ID		= 217930;	/// �׸޽ý� �����̵�

#ifdef RAVEN_SECOND_CLASS_CHANGE
static const int CLASS_CHANGE_BLADE_MASTER_ITEM_ID		= 217950;	/// �����̵� �������� ����
static const int CLASS_CHANGE_RECKLESS_FIST_ITEM_ID		= 217960;	/// ��ũ���� �ǽ�Ʈ�� ��
#endif

//{{ kimhc // 2010.1.22 // �̺� 2�� ����������
#ifdef EVE_SECOND_CLASS_CHANGE
static const int CLASS_CHANGE_NEMESIS_ITEM_ID	= 217970;			/// �׸޽ý� �����̵�
static const int CLASS_CHANGE_EMPRESS_ITEM_ID	= 217980;			/// �������� Ƽ�ƶ�
#endif EVE_SECOND_CLASS_CHANGE
//}} kimhc // 2010.1.22 // �̺� 2�� ����������

//{{ kimhc // 2011.1.3 // û 1�� ����
#ifdef	CHUNG_FIRST_CLASS_CHANGE
static const int CLASS_CHANGE_FURY_GUARDIAN_ID		= 229460;		/// ǻ�� ������� ����
static const int CLASS_CHANGE_SHOOTER_GUARDIAN_ID	= 229470;		/// ���� ������� ����
#endif	CHUNG_FIRST_CLASS_CHANGE
//}} kimhc // 2011.1.3 // û 1�� ����

#ifdef SERV_ADD_CHUNG_SHELLING_GUARDIAN
static const int CLASS_CHANGE_SHELLING_GUARDIAN_ID	= 242360;		// ���� ������� ����
#endif //SERV_ADD_CHUNG_SHELLING_GUARDIAN

//{{ JHKang / ������ / 2011.6.13 / û 2�� ����
#ifdef CHUNG_SECOND_CLASS_CHANGE
static const int CLASS_CHANGE_IRON_PALADIN_ID	= 235120;			/// ��ȣ���� ����
static const int CLASS_CHANGE_DEADLY_CHASER_ID	= 235130;			/// �������� ����
#endif

#ifdef SERV_ELSWORD_INFINITY_SWORD
static const int CLASS_CHANGE_INFINITY_SWORD_ID	= 252640;			/// ���ҵ� 2��
#endif
#ifdef SERV_ARME_DIMENSION_WITCH
static const int CLASS_CHANGE_DIMENSION_WITCH_ID	= 252650;		/// ���̻� 2��
#endif
#ifdef SERV_RENA_NIGHT_WATCHER
static const int CLASS_CHANGE_NIGHT_WATCHER_ID		= 252660;		/// ���� 2��
#endif
#ifdef SERV_RAVEN_VETERAN_COMMANDER
static const int CLASS_CHANGE_VETERAN_COMMANDER_ID	= 252670;		/// ���̺� 2��
#endif
#ifdef SERV_EVE_BATTLE_SERAPH
static const int CLASS_CHANGE_BATTLE_SERAPH_ID		= 252680;		/// �̺� 2�� �ڵ� ��Ʋ ������
#endif
#ifdef SERV_CHUNG_TACTICAL_TROOPER
static const int CLASS_CHANGE_TACTICAL_TROOPER_ID	= 252690;		/// û 2��
#endif

#ifdef ARA_CHANGE_CLASS_FIRST
static const int CLASS_CHANGE_LITTLE_HSIEN_ID		= 264380;		/// �ƶ� �Ҽ� ���� ������
static const int CLASS_CHANGE_SAKRA_DEVANAM_ID		= 264390;		/// �ƶ� ����õ ���� ������
#endif //ARA_CHANGE_CLASS_FIRST

#ifdef NEW_CHARACTER_EL
static const int CLASS_CHANGE_SABER_KNIGHT_ID		= 264391;		/// ���̹� ����Ʈ�� ����
static const int CLASS_CHANGE_PYRO_KNIGHT_ID		= 264392;		/// ���̷� ����Ʈ�� ����
#endif // NEW_CHARACTER_EL

#ifdef SERV_ARA_CHANGE_CLASS_SECOND // ����ȯ
/// ������ ���̵� �߱޵Ǹ� ���� �ʿ�
static const int CLASS_CHANGE_LITTLE_DEVIL_ID		= 264393;		/// �ƶ� �Ҹ� ���� ������
static const int CLASS_CHANGE_YAMA_RAJA_ID				= 264394;		/// �ƶ� ���� ���� ������
#endif // SERV_ARA_CHANGE_CLASS_SECOND

//#pragma NOTE( "���� ���� ĳ�� ������ ���� �۾��� ���⿡ ����� �մϴ�. ����ϴ� �� ��� �˻��ؼ� �߰����־�� �մϴ�." )
#ifdef UNIT_CLASS_CHANGE_ITEM

#pragma region 1�� ����
	static const int UNIT_CLASS_CHANGE_SWORD_KNIGHT_ITEM_ID		= 241950;	/// ���ҵ� �ҵ� ����Ʈ
	static const int UNIT_CLASS_CHANGE_MAGIC_KNIGHT_ITEM_ID    	= 241960;	/// ���ҵ� ���� ����Ʈ
	static const int UNIT_CLASS_CHANGE_SHEATH_KNIGHT_ITEM_ID	= 241970;	/// ���ҵ� �þ� ����Ʈ
	static const int UNIT_CLASS_CHANGE_HIGH_MAGICIAN_ITEM_ID    = 241980;	/// ���̻� ���� ������
	static const int UNIT_CLASS_CHANGE_DARK_MAGICIAN_ITEM_ID    = 241990;	/// ���̻� ��ũ ������
	static const int UNIT_CLASS_CHANGE_BATTLE_MAGICIAN_ITEM_ID	= 242000;	/// ���̻� ��Ʋ ������
	static const int UNIT_CLASS_CHANGE_COMBAT_RANGER_ITEM_ID	= 242010;	/// ���� �Ĺ� ������
	static const int UNIT_CLASS_CHANGE_SNIPING_RANGER_ITEM_ID	= 242020;	/// ���� �������� ������
	static const int UNIT_CLASS_CHANGE_TRAPPING_RANGER_ITEM_ID	= 242030;	/// ���� Ʈ���� ������
	static const int UNIT_CLASS_CHANGE_SOUL_TAKER_ITEM_ID       = 242040;	/// ���̺� �ҿ� ����Ŀ
	static const int UNIT_CLASS_CHANGE_OVER_TAKER_ITEM_ID       = 242050;	/// ���̺� ���� ����Ŀ
	static const int UNIT_CLASS_CHANGE_WEAPON_TAKER_ITEM_ID		= 242060;	/// ���̺� ���� ����Ŀ
	static const int UNIT_CLASS_CHANGE_EXOTIC_GEAR_ITEM_ID      = 242070;	/// �̺� �ڵ� ����ƽ
	static const int UNIT_CLASS_CHANGE_ARCHITECTURE_ITEM_ID     = 242080;	/// �̺� �ڵ� ��Ű����
	static const int UNIT_CLASS_CHANGE_ELECTRA_ITEM_ID			= 242090;	/// �̺� �ڵ� �Ϸ�Ʈ��
	static const int UNIT_CLASS_CHANGE_FURY_GUARDIAN_ID			= 242100;	/// û ǻ�� �����
	static const int UNIT_CLASS_CHANGE_SHOOTING_GUARDIAN_ID		= 242110;	/// û ���� �����
	static const int UNIT_CLASS_CHANGE_SHELLING_GUARDIAN_ID		= 242120;	/// û ���� �����
	static const int UNIT_CLASS_CHANGE_SABER_KNIGHT_ID			= 252754;	/// �����ý� ���̹� ����Ʈ
	static const int UNIT_CLASS_CHANGE_PYRO_KNIGHT_ID			= 252755;	/// �����ý� ���̷� ����Ʈ
#ifdef SERV_ARA_CHANGE_CLASS_SECOND // ����ȯ
	/// ������ �߱� �Ǹ� ���� �ʿ�
	static const int UNIT_CLASS_CHANGE_LITTLE_HSIEN_ID			= 252757;	/// �ƶ� �Ҽ�
	static const int UNIT_CLASS_CHANGE_LITTLE_DEVIL_ID			= 252756;	/// �ƶ� �Ҹ�
#endif // SERV_ARA_CHANGE_CLASS_SECOND
#pragma endregion

#pragma region 2�� ����
	static const int UNIT_CLASS_CHANGE_LORD_KNIGHT_ITEM_ID			= 242130;	/// ���ҵ� �ε� ����Ʈ
	static const int UNIT_CLASS_CHANGE_RUNE_SLAYER_ITEM_ID			= 242140;	/// ���ҵ� �� �����̾�
	static const int UNIT_CLASS_CHANGE_INFINITY_SWORD_ITEM_ID		= 242150;	/// ���ҵ� ���Ǵ�Ƽ ������
	static const int UNIT_CLASS_CHANGE_VOID_PRINCESS_ITEM_ID		= 242160;	/// ���̻� ���̵� ��������
	static const int UNIT_CLASS_CHANGE_ELEMENTAL_MASTER_ITEM_ID		= 242170;	/// ���̻� ������Ż ������
	static const int UNIT_CLASS_CHANGE_DIMENSION_WITCH_ITEM_ID		= 242180;	/// ���̻� ���� ��ġ
	static const int UNIT_CLASS_CHANGE_WIND_SNEAKER_ITEM_ID			= 242190;	/// ���� ���� ����Ŀ
	static const int UNIT_CLASS_CHANGE_GRAND_ARCHER_ITEM_ID			= 242200;	/// ���� �׷��� ��ó
	static const int UNIT_CLASS_CHANGE_NIGHT_WATCHER_ITEM_ID		= 242210;	/// ���� ����Ʈ ����
	static const int UNIT_CLASS_CHANGE_BLADE_MASTER_ITEM_ID			= 242220;	/// ���̺� �����̵� ������
	static const int UNIT_CLASS_CHANGE_RECKLESS_FIST_ITEM_ID		= 242230;	/// ���̺� ��ũ���� �ǽ�Ʈ
	static const int UNIT_CLASS_CHANGE_VETERAN_COMMANDER_ITEM_ID	= 242240;	/// ���̺� ���Ͷ� �ڸǴ�
	static const int UNIT_CLASS_CHANGE_NEMESIS_ITEM_ID				= 242250;	/// �̺� �ڵ� �׸޽ý�
	static const int UNIT_CLASS_CHANGE_EMPRESS_ITEM_ID				= 242260;	/// �̺� �ڵ� ��������
	static const int UNIT_CLASS_CHANGE_BATTLE_SERAPH_ITEM_ID		= 242270;	/// �̺� �ڵ� ��Ʋ ������
	static const int UNIT_CLASS_CHANGE_IRON_PALADIN_ID				= 242280;	/// û ���̾� �ȶ��
	static const int UNIT_CLASS_CHANGE_DEADLY_CHASER_ID				= 242290;	/// û ���鸮 ü�̼�
	static const int UNIT_CLASS_CHANGE_TACTICAL_TROOPER_ID			= 242300;	/// û ��ƽ�� Ʈ����
#ifdef SERV_ARA_CHANGE_CLASS_SECOND // ����ȯ
	static const int UNIT_CLASS_CHANGE_SAKRA_DEVANAM_ID				= 252751;	/// �ƶ� ��õ
	static const int UNIT_CLASS_CHANGE_YAMA_RAJA_ID						= 252752;	/// �ƶ� ����
#endif // SERV_ARA_CHANGE_CLASS_SECOND
#pragma endregion

#endif



static const int MAX_PARTY_FEVER = 4000;// ��Ʋ �ʵ�� ���� �Ǹ鼭 �ִ� �ǹ� ��ġ�� 2000


static const float MAX_PLAY_TIME	= 19999.0f;		// ���������� �ִ�� �÷��� �� �� �ִ� �ð�




static const int TEAM_PVP_KILL_COUNT = 0;
static const int TEAM_DEATH_PVP_KILL_COUNT[]	= { 4, 8, 12, 16, 20, 24, 28 };
static const int SURVIVAL_PVP_KILL_COUNT[]		= { 3, 6, 9, 12 };

static const int TEAM_PVP_PLAY_TIME[]			= { 200, 300, 400 };
static const int TEAM_DEATH_PVP_PLAY_TIME[]		= { 200, 500, 800, 1100 };
static const int SURVIVAL_PVP_PLAY_TIME[]		= { 200, 500, 800, 1100 };



#ifdef DUNGEON_ITEM
// ����
static const int DUNGEON_ITEM1_1 = 77280;	// ������ ���
static const int DUNGEON_ITEM1_2 = 77290;	// ���������� ���
static const int DUNGEON_ITEM1_3 = 77300;	// ������ ���
static const int DUNGEON_ITEM1_4 = 77320;	// ���� ���

// ���ɼ�ȯ��
static const int DUNGEON_ITEM2_1 = 77330;	// ���������� �г�
static const int DUNGEON_ITEM2_2 = 77340;	// ����̾���� �г�
static const int DUNGEON_ITEM2_3 = 77350;	// ����� �г�
static const int DUNGEON_ITEM2_4 = 77360;	// �ϸ��Ǿ��� �ձ�

// �����
static const int DUNGEON_ITEM3_1 = 77370;	// ����� ����
static const int DUNGEON_ITEM3_2 = 77380;	// ������ ����
static const int DUNGEON_ITEM3_3 = 77390;	// ��ȣ�� ����
static const int DUNGEON_ITEM3_4 = 77400;	// ������ȣ�� ����
static const int DUNGEON_ITEM3_5 = 77410;	// �Ұ�ħ�� ����

// �Ʊ����� ��ȯ
static const int DUNGEON_ITEM4_1 = 131857;  // ���� ��ȯ
#ifdef CLIENT_COUNTRY_EU
static const int DUNGEON_ITEM4_2 = 78300;  // ���� ��ȯ
static const int DUNGEON_ITEM4_3 = 78305;  // ���� ��ȯ
static const int DUNGEON_ITEM4_4 = 78310;  // ���� ��ȯ
static const int DUNGEON_ITEM4_5 = 78315;  // ���� ��ȯ
static const int DUNGEON_ITEM4_6 = 78320;  // ���� ��ȯ
#else //CLIENT_COUNTRY_EU
static const int DUNGEON_ITEM4_2 = 135176/*78300*/;  // ���� ��ȯ
static const int DUNGEON_ITEM4_3 = 135177/*78305*/;  // ���� ��ȯ
static const int DUNGEON_ITEM4_4 = 135178/*78310*/;  // ���� ��ȯ
static const int DUNGEON_ITEM4_5 = 135179/*78315*/;  // ���� ��ȯ
static const int DUNGEON_ITEM4_6 = 135180/*78320*/;  // ���� ��ȯ
#endif //CLIENT_COUNTRY_EU
#ifdef ADD_VELDER_ACADEMY_EVENT_MONSTER		/// ���� ��ī���� ���б� �̺�Ʈ
static const int DUNGEON_ITEM5_1 = 78835;	/// ���� ������ �ϰŽ�
static const int DUNGEON_ITEM5_2 = 78836;	/// �л����� ������ �̸ֹ�Ʈ
static const int DUNGEON_ITEM5_3 = 78837;	/// �ױ۸��� Ƽó ��ġ��
static const int DUNGEON_ITEM5_4 = 78838;	/// ���� ������ ����
static const int DUNGEON_ITEM5_5 = 78839;	/// ���� ������ ȣ����
#endif ADD_VELDER_ACADEMY_EVENT_MONSTER

const int DUNGEON_ITEM_LIST1[] = { DUNGEON_ITEM1_1, DUNGEON_ITEM1_2, DUNGEON_ITEM1_3, DUNGEON_ITEM1_4, };	// ���
const int DUNGEON_ITEM_LIST2[] = { DUNGEON_ITEM2_1, DUNGEON_ITEM2_2, DUNGEON_ITEM2_3, DUNGEON_ITEM2_4, };	// ���ɼ�ȯ
const int DUNGEON_ITEM_LIST3[] = { DUNGEON_ITEM3_1, DUNGEON_ITEM3_2, DUNGEON_ITEM3_3, DUNGEON_ITEM3_4, DUNGEON_ITEM3_5, };	// ����
const int DUNGEON_ITEM_LIST4[] = { DUNGEON_ITEM4_1, DUNGEON_ITEM4_2, DUNGEON_ITEM4_3, DUNGEON_ITEM4_4, DUNGEON_ITEM4_5, DUNGEON_ITEM4_6, }; // ���� ��ȯ
#endif

#ifdef RIDING_SYSTEM
//Ż �� ���� ������
static const int RIDING_PET_ITEM_ID[] = { 550000, 550010, 550020, 550001, 550011, 550021, 550030, 550040, 550003, 550013, 550023,
										  67006329, 67006330, 67006331, 67006703, 67006704, 67006705, 67006706, 67006707, 67006712, 67006714,
										  67006720, 67006721, 67006722, 60008021, 84002150, 84002160, 84002170 };
#endif //RIDING_SYSTEM

// ����� ������ 
static const int PET_ITEM_ID[] =	{ 500000, 500010, 500020, 500050, 500060, 500070, 500100, 500110, 500140, 500150, 500160, 500170,
									  500180, 500230, 500240, 500250, 500260, 500310, 500320, 500330, 500340, 500510, 500520, 500530,
									  500540, 500550, 500600, 500610, 500620, 500660, 500670, 500700, 500710, 500730, 500750, 500790,
									  500800, 500815, 500820, 500821, 500822,
									  60001802, 60003470, 90000280, 90000281, 90000282, 75000634, 60003797, 60003798, 60003799, 60003800,
									  60003950, 60007130, 75000724, 75000725, 75000759, 75000760, 210000071, 210000138, 210000140, 84001270,
									  84001271, 84001272, 84001273, 75000633, 75000635, 67005730, 115000301, 115000300, 115000140, 115000141,
									  115000142, 115000130, 115000131, 115000132, 67005830, 67005860, 83000011, 83000012, 80001364, 83000130,
									  83000131, 83000132, 83000133, 210000230, 210000240, 90002050, 90002060, 90002090, 90002100, 130000556,
									  130000557, 130000558, 130000559, 130000680, 130000682, 85003040, 70006030, 70006040, 70006050,
									  90002290, 90002292, 90002293, 90002294, 90002295, 90002296,
									  60007750, 60007751, 60007754, 60007755,
									  85003490, 152000221, 152000373, 85003660, 85003670, 85003680,
									  67006630, 67006632,
									  153000095, 84002210, 84002211,
									  };

static const int PET_ITEM_PET[] =	{ 2, 3,	4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 31,32, 90, 110, 111, 112, 113, 114, 115,125,126, 133, 138, 30004  };
#ifdef CHANGE_PET_NAME
static const int CHANGE_PET_NAME_ITEM_ID = 248390;
#endif CHANGE_PET_NAME
#ifdef CASH_ITEM_REFUND
static const int NON_REFUNDABLE_CASH_ITEM_ID[] = { 213610,213460,213470,213480,213490,213500,200885,200750,206820,206830,214490,214500,214470,214480,214410,214420,214450,214460,214430,214440,217980,217970,217960,217950,216430,216420,217930,217920,217860,217850,215300,215290,215280,215270,215220,215210,215260,215250,215240,215230,213450,213440,213400,213390,213380,213430,213420,213370,213360,213410,213350,213300,213340,213290,213330,213260,213270,213320,213280,213310,127030,131134,221600,229460,229470,242310,241950,241960,241970,241980,241990,242000,242010,242020,242030,242040,242050,242060,242070,242080,242090,242100,242110,242120,242130,242140,242150,242160,242170,242180,242190,242200,242210,242220,242230,242240,242250,242260,242270,242280,242290,242300,242320};
#endif CASH_ITEM_REFUND

#ifdef PET_DROP_ITEM_PICKUP
//�� ������ �ݱ� ��� Ȱ��ȭ ������
static const int ACTIVATION_DROP_ITEM_PICKUP_SKILL = 500720;
#endif //PET_DROP_ITEM_PICKUP

#ifdef SERV_HALLOWEEN_PUMPKIN_FAIRY_PET
static const int HALLOWEEN_TRANSFORM_POSION = 90002300;
#endif //SERV_HALLOWEEN_PUMPKIN_FAIRY_PET


//���, ���� ������ ID
#ifdef BUFF_TEMPLET_SYSTEM
static const int PROTECTION_OF_NUT_ITEM_ID		= 78520;	//��Ʈ�� ��ȣ		- ���	
static const int PROTECTION_OF_PUTA_ITEM_ID		= 78530;	//��Ÿ�� ��ȣ		- ���	
static const int PROTECTION_OF_LAHELL_ITEM_ID	= 78540;	//������ ��ȣ		- ���	
static const int PROTECTION_OF_JAKIELL_ITEM_ID	= 78550;	//��Ű���� ��ȣ		- ���	
static const int BELSSING_OF_SERAPHIM_ITEM_ID	= 78560;	//�������� �ູ		- ����	
static const int BELSSING_OF_GEV_ITEM_ID		= 78570;	//�Ժ��� �ູ		- ����	
static const int BELSSING_OF_AMON_ITEM_ID		= 78580;	//�Ƹ��� �ູ		- ����	
static const int BELSSING_OF_CRONOS_ITEM_ID		= 78590;	//ũ�γ뽺�� �ູ	- ����
#endif BUFF_TEMPLET_SYSTEM

//----------------------------------------------------------------------------------//

#ifdef PET_ITEM_GM
static const int PET_ATTACK_SKILL_GM_ITEM_ID			= 80001820;
static const int PET_CHEER_SKILL_GM_ITEM_ID				= 80001821;
#endif //PET_ITEM_GM

#ifdef SERV_VIP_SYSTEM
static const int VIP_WARP_ITEM_FREE_ITEM_ID					= 80000117;		// VIP �̵�Ƽ�� ���� �̿�� (80000117)
#endif //SERV_VIP_SYSTEM
#ifdef SERV_VIP_SYSTEM
static const int EVENT_NASOD_SCOPE_ITEM_ID				= 80000119;
static const int EVENT_NASOD_SCOPE_HIGH_ITEM_ID			= 80000120;
#endif //SERV_VIP_SYSTEM
#ifdef EVENT_GUILD_ITEM
static const int EVENT_GUILD_CASH_SKILL_ITEM_ID			= 80001181;		// �߱� ���� �̺�Ʈ�� �Ƹ����� �׳�ý� ���ູ 15�� 5GSP // 2012.06.05 lygan_������
static const int EVENT_RESET_A_GUILD_SKILL_ITEM_ID		= 80001182;		// �߱� ���� �̺�Ʈ�� �Ƹ����� �׳�ý� ���ູ 15�� 5GSP // 2012.06.05 lygan_������
static const int EVENT_GUILD_EXPANSION_ITEM_ID	= 80001183;	// �߱� ���� �̺�Ʈ�� ��� �ο� Ȯ��� // 2012.06.04 lygan_������
#endif //EVENT_GUILD_ITEM
#ifdef SERV_READY_TO_SOSUN_EVENT
static const int EVENT_READY_TO_SOSUN_ITEM_ID			= 60007092;
#endif SERV_READY_TO_SOSUN_EVENT

#ifdef SERV_EVENT_GUILD_SKILL_GLOBAL
static const int EVENT_GUILD_SKILL_5_POINT_15_DAY_JP	= 67005930;		// JP, ��� ��ų �Ⱓ�� + 5, �ŷ��Ұ�, 15��
#endif //SERV_EVENT_GUILD_SKILL_GLOBAL

#ifdef HYPER_MODE_EFFECT_ADD_BY_ITEM
// ��� Īȣ, ������ Ư�� ����Ʈ ����
static const int HYPER_EFFECT_ITEM_DARK_DRAGON = 60003795;
#endif HYPER_MODE_EFFECT_ADD_BY_ITEM
#ifdef SERV_SHARING_BANK_QUEST_CASH
#ifdef SERV_COUNTRY_TWHK
static const int SHARING_BANK_OPEN_CASH_ITEM_ID				= 60003460;	// ���� ���� Ȱ��ȭ ������(�븸)
#else
static const int SHARING_BANK_OPEN_CASH_ITEM_ID				= 65000701;	// ���� ���� Ȱ��ȭ ������
#endif SERV_COUNTRY_TWHK
#endif

//{{ 2011.10.24 ������ : ��� Ƽ�� �۾� ( ����ϸ� ĳ�� �ִ� ������ )
#ifdef SERV_GOLD_TICKET
static const int GOLD_TICKET_ID				= 60003600;// ��� Ƽ��
static const int GOLD_TICKET_10_ID			= 75000700;// Gold Ticket (10)
static const int GOLD_TICKET_50_ID			= 75000701;// Gold Ticket (50)
static const int GOLD_TICKET_100_ID			= 75000702;// Gold Ticket (100)
static const int GOLD_TICKET_500_ID			= 75000703;// Gold Ticket (500)
static const int GOLD_TICKET_1000_ID		= 75000704;// Gold Ticket (1000)
static const int GOLD_TICKET_2000_ID		= 75000705;// Gold Ticket (2000)

inline bool IsGoldTicket(const int iItemID )
{
	switch(iItemID)
	{
	case GOLD_TICKET_10_ID:
	case GOLD_TICKET_50_ID:
	case GOLD_TICKET_100_ID:
	case GOLD_TICKET_500_ID:
	case GOLD_TICKET_1000_ID:
	case GOLD_TICKET_2000_ID:
		return true;
	default:
		return false;
	}

	return false;
}

// Ư���� ������(��� Ƽ��) ���� ���� �ȵǵ��� üũ �ϴ� �Լ�
inline bool IsCantShareBankItem(const int iItemID )
{
	if(IsGoldTicket(iItemID) == true)
		return true;
	else
		return false;
}
#endif //SERV_GOLD_TICKET
//}}
//----------------------------------------------------------------------------------//



#ifdef SERV_NEW_DEFENCE_DUNGEON // ���볯¥: 2013-04-11
static const int ELIXIR_GIANT_POTION			= 78894;	/// ����� �� ���� : ����ȭ ����
static const int ELIXIR_BLAZING_BOMB			= 78895;	/// ����� �� ���� : �̱��̱� ����
static const int ELIXIR_SPIRIT_OF_CHASER		= 78896;	/// ����� �� ���� : �������� ȥ
static const int ELIXIR_CRADLE_OF_LITTLE_FAIRY	= 78897;	/// ����� �� ���� : �Ʊ� ���� ���
static const int ELIXIR_ICE_BALL_OF_DENIPH		= 78898;	/// ����� �� ���� : �������� ���� ����
static const int ELIXIR_FEATHER_OF_VENTUS		= 78899;	/// ����� �� ���� : �������� ������
static const int ELIXIR_FLAME_RING_OF_ROSSO		= 78900;	/// ����� �� ���� : �ν��� ȭ������
static const int ELIXIR_BIG_HAND_POTION			= 78901;	/// ����� �� ���� : �Ŵ�� ����
#endif // SERV_NEW_DEFENCE_DUNGEON
#ifdef SERV_EVENT_ARA_NEW_CHAR_THE_ESSENCE_OF_HERETIC
static const int THE_ESSENCE_OF_WEAK_HERETIC_POTION			= 85003830;	// �̺�Ʈ ���� : �̾��� ������ ����
static const int THE_ESSENCE_OF_HERETIC_POTION				= 85003831;	// �̺�Ʈ ���� : ������ ����
#endif SERV_EVENT_ARA_NEW_CHAR_THE_ESSENCE_OF_HERETIC
#ifdef SERV_EVENT_RURIEL_MANA_ENERGIZE_POTION
static const int BUFF_RURIEL_MANA_ENERGIZE_POTION			= 60008058;	// �̺�Ʈ ���� : �縮���� ���� Ȱ��ȭ ����
#endif SERV_EVENT_RURIEL_MANA_ENERGIZE_POTION
// ��ȯ������ npc card list
#ifdef ADD_TRAININGGAME_NPC
static const int NPC_CARD[] = { 78620, 78621, 78622, 78623, 78630, 78631, 78632, 78633, 78634, 78635, 78640, 78641, 78642, 78643, 78644, 78645, 78646, 78650, 78651, 78652, 78653, 78654, 78655, 78656, 78660, 78661, 78662, 78663, 78664, 78665, 78666, 78667, 78669, 78670, 78671, 78672, 78673, 78674, 78675, 78676, 78677, 78678, 78679, 78680, 78681, 78682, 78683, 78684, 78685, 78686, 78687, 78688, 78689, 78690, 78691, 78692, 78693, 78694, 78695, 78696, 78697, 78698, 78699, 78700, 78701, 78702, 78703, 78704, 78705, 78706 ,78707, 78708, 78709, 78710, 78711, 78712, 78713, 78714, 78715, 78716, 78717, 78718, 78719, 78720, 78721, 78722, 78723, 78724, 78725, 78726, 78727, 78728, 78729, 78730, 78731, 78732, 78733, 78734, 78735, 78736, 78737, 78738, 78739, 78740, 78741, 78742, 78743, 78744,	78745, 78746, 78747, 78748, 78749, 78750, 78751, 78752, 78753, 78754, 78811, 78812, 78813, 78814, 78815, 78816, 78817, 78818, 78819, 78820, 78821, 78822, 78823, 78824, 78825, 78826, 78827, 78828, 78829, 78830, 78831, 78832, 78833 };
#endif

#ifdef SERV_NEW_ITEM_SYSTEM_2013_05//�̶�Ŭť��
static const int ITEM_ID_RESTORE_ITEM_EVALUATE[] = 
{ 
	270866, 270848,	270849,	270850,	270851,	270852,	270853,	270854,	270855,	270856, // �Ϲ� 0~9����
	270867, 270857,	270858,	270859,	270860,	270861,	270862,	270863,	270864,	270865, // �ں� 0~9����
	//INT
	60007580, 60007571, 60007572, 60007573, 60007574, 60007575, 60007576, 60007577, 60007578, 60007579, // �縮�� 0~9����

};		   
#endif // SERV_NEW_ITEM_SYSTEM_2013_05

#ifdef SERV_RECRUIT_EVENT_BASE
static const int RECOMMEND_TICKET_ID					= 90000600;
#endif SERV_RECRUIT_EVENT_BASE

enum X2_GAME_MSG
{
	XGM_SYSTEM_BASE = CKTDXApp::KM_GAME_BASE + 1,

	XGM_STATE_CHANGE,
	XGM_DELETE_DIALOG,
	XGM_REQUEST_FOCUS_CONTROL,
	XGM_RECEIVE_FROM_SERVER,
	XGM_SERVER_PACKET_TIMEOUT,
	XGM_NEXON_CALLBACK,
	XGM_QUIT_GAME,	
#ifdef ADD_COLLECT_CLIENT_INFO
	XGM_RECEIVE_FROM_COLLECTSERVER,
#endif ADD_COLLECT_CLIENT_INFO
#ifdef UI_RESOURCE_REALTIME_LOAD
	XGM_REAL_DELETE_DIALOG,
#endif //UI_RESOURCE_REALTIME_LOAD
};

enum X2_FONT
{
	XF_DODUM_15_NORMAL = 0,
	XF_DODUM_11_NORMAL,
};




enum X2_UK_FONT
{
	XUF_DODUM_11_NORMAL = 0,
	XUF_DODUM_13_SEMIBOLD,
	XUF_DODUM_15_BOLD,		// �� ��Ʈ�� ������� 2���� �Ѵ�... CKTDGUIUnibuffer CPtoX ������ ��,�Ѥ�
	XUF_DODUM_20_BOLD,
	XUF_HEADLINE_30_NORMAL, 

	XUF_DODUM_8_NORMAL,

	XUF_DODUM_11_NORMAL_MAPPED,
	XUF_DODUM_15_NORMAL_MAPPED,
	XUF_DODUM_15_EXBOLD,
	//XUF_DODUM_8_NORMAL,

//	XUF_2002L_15_NORMAL,
#if defined(CLIENT_COUNTRY_US) || defined(CLIENT_COUNTRY_PH)
	XUF_BLAM_SPEECH,
	XUF_BLAM_TALK,

	XUF_DODUM_15B_BOLD,
	XUF_BLAM_NPC_TALK,
#endif // CLIENT_COUNTRY_US || defined(CLIENT_COUNTRY_PH)
};

static const int SLOT_MANAGER_FONT_INDEX		= XUF_DODUM_13_SEMIBOLD;
static const int SLOT_MANAGER_FONT_INDEX_SMALL	= XUF_DODUM_11_NORMAL;










enum X2_LAYER
{
	XL_GAME_BASE			= 1,

	XL_SKY_DOME_0,
	XL_SKY_DOME_1,
	XL_SKY_DOME_2,

	XL_SKY_WORLD_BASE_0,
	XL_SKY_WORLD_BASE_1,
	XL_SKY_WORLD_BASE_2,

	XL_SKY_WORLD_OBJECT_0,
	XL_SKY_WORLD_OBJECT_1,
	XL_SKY_WORLD_OBJECT_2,

	XL_UNIT_0,
	XL_UNIT_1,
	XL_UNIT_2,

	XL_EFFECT_0,
	XL_EFFECT_1,
	XL_EFFECT_2,

//{{ kimhc // 2010.6.17 // enum X2_LAYER�� ���̾� �߰��ϴ� �۾�
#ifdef	ADD_X2_LAYER
	XL_SKY_WORLD_OBJECT_OVER_EFFECT_0,
	XL_SKY_WORLD_OBJECT_OVER_EFFECT_1,
	XL_SKY_WORLD_OBJECT_OVER_EFFECT_2,
	XL_SKY_WORLD_OBJECT_OVER_EFFECT_3,

	XL_EFFECT_OVER_ALL_WORLD_OBJECT_0,
	XL_EFFECT_OVER_ALL_WORLD_OBJECT_1,
#endif	ADD_X2_LAYER
//}} kimhc // 2010.6.17 // enum X2_LAYER�� ���̾� �߰��ϴ� �۾�
	
	XL_LENS_FLARE,
	XL_MINIMAP,
};

enum X2_DIALOG_LAYER
{
	XDL_NORMAL			= 0,
	XDL_NORMAL_1		= 1,
	XDL_NORMAL_2		= 2,
	XDL_NORMAL_3		= 3,
	XDL_NORMAL_4		= 4,

	XDL_MESSENGER		= 100,
	XDL_OPTION			= 200,
	XDL_GAME_EDIT		= 300,
	XDL_POP_UP			= 400,
	XDL_DIRECTING		= 500,	//�����߿��� � ������ �� �� ����!!!!
};

enum X2_CHANNEL
{
	XC_PVP_FREE = 1,
};

//{{ kimhc // 2009-12-15 // ������ ����
#ifdef EXTEND_SERVER_GROUP_MASK
#else EXTEND_SERVER_GROUP_MASK
enum SERVER_GROUP_ID
{
	SGI_INVALID			= -1,
	SGI_SOLES			= 0,
	SGI_GAIA,
	SGI_END,
};
#endif EXTEND_SERVER_GROUP_MASK
//}} kimhc // 2009-12-15 // ������ ����
#ifdef	USE_ACCOUNT_DOMAIN
enum ACCOUNT_DOMAIN_TYPE
{
	ADT_INVALID			= -1,
	ADT_PLAYID			= 0,
	ADT_TCG,
};

const static std::wstring ACCOUNT_DOMAIN_NAME[] = { L"THPP", L"THTC" };
#endif	USE_ACCOUNT_DOMAIN

enum X2_ERROR_MSG
{
	XEM_ERROR0 = KEM_BASE_ERROR_MSG,
	XEM_ERROR1,
	XEM_ERROR2,
	XEM_ERROR3,
	XEM_ERROR4,
	XEM_ERROR5,
	XEM_ERROR6,
	XEM_ERROR7,
	XEM_ERROR8,
	XEM_ERROR9,
	XEM_ERROR10,
	XEM_ERROR11,
	XEM_ERROR12,
	XEM_ERROR13,
	XEM_ERROR14,
	XEM_ERROR15,
	XEM_ERROR16,
	XEM_ERROR17,
	XEM_ERROR18,
	XEM_ERROR19,
	XEM_ERROR20,
	XEM_ERROR21,
	XEM_ERROR22,
	XEM_ERROR23,
	XEM_ERROR24,
	XEM_ERROR25,
	XEM_ERROR26,
	XEM_ERROR27,
	XEM_ERROR28,
	XEM_ERROR29,
	XEM_ERROR30,
	XEM_ERROR31,
	XEM_ERROR32,
	XEM_ERROR33,
	XEM_ERROR34,
	XEM_ERROR35,
	XEM_ERROR36,
	XEM_ERROR37,
	XEM_ERROR38,
	XEM_ERROR39,
	XEM_ERROR40,
	XEM_ERROR41,
	XEM_ERROR42,
	XEM_ERROR43,
	XEM_ERROR44,
	XEM_ERROR45,
	XEM_ERROR46,
	XEM_ERROR47,
	XEM_ERROR48,
	XEM_ERROR49,
	XEM_ERROR50,
	XEM_ERROR51,
	XEM_ERROR52,
	XEM_ERROR53,
	XEM_ERROR54,
	XEM_ERROR55,
	XEM_ERROR56,
	XEM_ERROR57,
	XEM_ERROR58,
	XEM_ERROR59,
	XEM_ERROR60,
	XEM_ERROR61,
	XEM_ERROR62,
	XEM_ERROR63,
	XEM_ERROR64,
	XEM_ERROR65,
	XEM_ERROR66,
	XEM_ERROR67,
	XEM_ERROR68,
	XEM_ERROR69,
	XEM_ERROR70,
	XEM_ERROR71,
	XEM_ERROR72,
	XEM_ERROR73,
	XEM_ERROR74,
	XEM_ERROR75,
	XEM_ERROR76,
	XEM_ERROR77,
	XEM_ERROR78,
	XEM_ERROR79,
	XEM_ERROR80,
	XEM_ERROR81,
	XEM_ERROR82,
	XEM_ERROR83,
	XEM_ERROR84,
	XEM_ERROR85,
	XEM_ERROR86,
	XEM_ERROR87, //P2P Fail
	XEM_ERROR88, //P2P OK
	XEM_ERROR89, //Port Check Fail
	XEM_ERROR90, //Port Check OK
	XEM_ERROR91,
	XEM_ERROR92, //P2P Fail in Room Game Start
	XEM_ERROR93,
	XEM_ERROR94,
	XEM_ERROR95,
	XEM_ERROR96,
	XEM_ERROR97,
	XEM_ERROR98,
	XEM_ERROR99,
	XEM_ERROR100,
	XEM_ERROR101,
	XEM_ERROR102,
	XEM_ERROR103,
	XEM_ERROR104,
	XEM_ERROR105,
	XEM_ERROR106,
	XEM_ERROR107,
	XEM_ERROR108,
	XEM_ERROR109,
	XEM_ERROR110,
	XEM_ERROR111,
	XEM_ERROR112,
	XEM_ERROR113,
	XEM_ERROR114,
	XEM_ERROR115,
	XEM_ERROR116,
	XEM_ERROR117,
	XEM_ERROR118,
	XEM_ERROR119,
	XEM_ERROR120,
	XEM_ERROR121,
	XEM_ERROR122,
	XEM_ERROR123,
	XEM_ERROR124,
	XEM_ERROR125,
	XEM_ERROR126,
	XEM_ERROR127,
	XEM_ERROR128,
	XEM_ERROR129,
	XEM_ERROR130,
	XEM_ERROR131,
	XEM_ERROR132,
	XEM_ERROR133,
	XEM_ERROR134,		// ������ �ε�ȭ�� �ε� ����
	XEM_ERROR135,
	XEM_ERROR136,
	XEM_ERROR137,
	XEM_ERROR138,
	XEM_ERROR139,
	XEM_ERROR140,		// ���ƹ�Ÿ �̹��� UploadFile ����
	XEM_ERROR141,		// 
	XEM_ERROR142,		// ����ID�� DungeonData ��� ����
	XEM_ERROR143,		// ä���̸� ������ ����
//{{ robobeg : 2013-03-12
    XEM_ERROR144,       // Item.lua �Ľ� ����
    XEM_ERROR145,       // SetItem.lua �Ľ� ����
    XEM_ERROR146,       // �� ����
//}} robobeg : 2013-03-12
	XEM_ERROR147,		// ��ȥ���� ��ũ��Ʈ �Ľ� ����.
	XEM_ERROR148,		// CharacterStatTable �Ľ� ����
	XEM_ERROR149,		// Emblem_Manager.lua	�Ľ� ����
};



//{{ Iruha : 2026-09-04 // base MP regen rate, in MP per second
#ifdef SERV_IRUHADEV_MP_REGEN_BOOST
const float SERV_IRUHADEV_BASE_MP_REGEN_PER_SEC = 50.0f;
#endif SERV_IRUHADEV_MP_REGEN_BOOST
//}} Iruha : 2026-09-04

