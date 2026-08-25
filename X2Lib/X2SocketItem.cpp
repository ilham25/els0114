#include "StdAfx.h"
#include ".\x2socketitem.h"

//{{ 최민철 [2013/1/4]  게임내 정보 스트링을 엑셀파일로 출력
#ifdef PRINT_INGAMEINFO_TO_EXCEL
#include "BasicExcel.hpp"
using namespace YExcel;
#endif PRINT_INGAMEINFO_TO_EXCEL
//}} 최민철 [2013/1/4]  게임내 정보 스트링을 엑셀파일로 출력

//{{ kimhc // 2011-07-05 // 옵션데이타 수치화 작업
#ifndef	NOT_USE_PERCENT_IN_OPTION_DATA
const float CX2SocketItem::SocketData::MAX_MOVESPEED				= 0.2f;
const float CX2SocketItem::SocketData::MAX_JUMPSPEED				= 0.2f;		
const float	CX2SocketItem::SocketData::MAX_ANIMATION_SPEED_RATE		= 20;

const float CX2SocketItem::SocketData::MAX_CRITICAL_RATE			= 1.0f;
const float CX2SocketItem::SocketData::MAX_ANTI_EVADE_RATE			= 0.2f;
const float CX2SocketItem::SocketData::MAX_EVADE_RATE				= 0.2f;
const float CX2SocketItem::SocketData::MAX_HYPERMODECHARGERATE 		= 0.5f;

#endif	NOT_USE_PERCENT_IN_OPTION_DATA
//}} kimhc // 2011-07-05 // 옵션데이타 수치화 작업

const float CX2SocketItem::SocketData::MAX_HYPERMODETIME			= 40.f;

const float CX2SocketItem::SocketData::MAX_REPAIR_PRICE_DISCOUNT	= 0.1f;

const float CX2SocketItem::SocketData::MAX_SUPER_ARMOR_RATE			= 0.5f;
const float CX2SocketItem::SocketData::MAX_SUPER_ARMOR_TIME			= 10.f;
const float CX2SocketItem::SocketData::MAX_DUBLE_ATTACK_RATE		= 0.5f;

#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
/*static*/CX2SocketItem::KItemStatRelLVData CX2SocketItem::ms_KDummyItemStatRelLVData;	
#endif //SERV_NEW_ITEM_SYSTEM_2013_05










CX2SocketItem::CX2SocketItem(void)
{
}

CX2SocketItem::~CX2SocketItem(void)
{
	std::map<int,SocketData*>::iterator iter;
	for( iter = m_mapSocketOptionPool.begin(); iter != m_mapSocketOptionPool.end(); iter++ )
	{
		SocketData* pSocketData = iter->second;
		SAFE_DELETE( pSocketData );
	}
	m_mapSocketOptionPool.clear();
}

//{{ kimhc // 2011-07-05 // 옵션데이타 수치화 작업
#ifdef	NOT_USE_PERCENT_IN_OPTION_DATA
void	CX2SocketItem::OpenScriptFile( const WCHAR* pFileName, const WCHAR* pFormulaFileName_ )
#else	NOT_USE_PERCENT_IN_OPTION_DATA
void	CX2SocketItem::OpenScriptFile( WCHAR* pFileName )
#endif	NOT_USE_PERCENT_IN_OPTION_DATA
//}} kimhc // 2011-07-05 // 옵션데이타 수치화 작업
{
	lua_tinker::decl( g_pKTDXApp->GetLuaBinder()->GetLuaState(),  "g_pCX2SocketItem", this );
	g_pKTDXApp->GetDeviceManager()->LoadLuaTinker( pFileName );	

	//{{ kimhc // 2011-07-05 // 옵션데이타 수치화 작업
#ifdef	NOT_USE_PERCENT_IN_OPTION_DATA
	g_pKTDXApp->GetDeviceManager()->LoadLuaTinker( pFormulaFileName_ );	
#endif	NOT_USE_PERCENT_IN_OPTION_DATA
	//}} kimhc // 2011-07-05 // 옵션데이타 수치화 작업

#ifdef LUA_TRANS_DEVIDE
	g_pKTDXApp->GetDeviceManager()->LoadLuaTinker( L"OptionItemDataTrans.lua" );	
#endif LUA_TRANS_DEVIDE

#ifdef SERV_CASH_ITEM_SOCKET_OPTION
	g_pKTDXApp->GetDeviceManager()->LoadLuaTinker( L"SocketGroupDataForCashAvatar.lua" );	
#endif // SERV_CASH_ITEM_SOCKET_OPTION

#ifdef SERV_NEW_ITEM_SYSTEM_2013_05 // 오현빈
	g_pKTDXApp->GetDeviceManager()->LoadLuaTinker( L"SkillLevelUpSocketGroup.lua" );	
#endif // SERV_NEW_ITEM_SYSTEM_2013_05

}

CX2SocketItem::SocketData* CX2SocketItem::GetSocketData( int key ) const
{
	std::map<int,SocketData*>::const_iterator iter;
	iter = m_mapSocketOptionPool.find( key );
	if( iter == m_mapSocketOptionPool.end() )
		return NULL;

	CX2SocketItem::SocketData* pSocketData = (CX2SocketItem::SocketData*)iter->second;
	return pSocketData;
}

bool CX2SocketItem::AddSocketData_LUA()
{
	KLuaManager luaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState() );
	TableBind( &luaManager, g_pKTDXApp->GetLuaBinder() );

	SocketData* pSocketData = new SocketData();

	//데이터 로드
	LUA_GET_VALUE(		luaManager, L"ID",		pSocketData->m_ID,		0 );
	LUA_GET_VALUE_ENUM( luaManager, L"TYPE",	pSocketData->m_Type,	SOCKET_DATA_TYPE, SDT_NONE );

	//스탯정보
	if( luaManager.BeginTable( L"STAT" ) == true )
	{

		float fData = 0.0f;
		LUA_GET_VALUE( luaManager,		"m_fBaseHP",		fData,		0.0f	);
		pSocketData->m_Stat.m_fBaseHP		= fData;

		LUA_GET_VALUE( luaManager,		"m_fAtkPhysic",		fData,		0.0f	);
		pSocketData->m_Stat.m_fAtkPhysic	= fData;
		LUA_GET_VALUE( luaManager,		"m_fAtkMagic",		fData,		0.0f	);
		pSocketData->m_Stat.m_fAtkMagic		= fData;

		LUA_GET_VALUE( luaManager,		"m_fDefPhysic",		fData,		0.0f	);
		pSocketData->m_Stat.m_fDefPhysic	= fData;
		LUA_GET_VALUE( luaManager,		"m_fDefMagic",		fData,		0.0f	);
		pSocketData->m_Stat.m_fDefMagic		= fData;

		luaManager.EndTable();
	}

#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
	if( luaManager.BeginTable( L"STAT_RELATION_ADD_LEVEL" ) == true )
	{
		LUA_GET_VALUE( luaManager,	"m_fBaseHPRelLV",		pSocketData->m_kStatRelLVData.m_fBaseHPRelLV, 0.f );
		LUA_GET_VALUE( luaManager,	"m_fAtkPhysicRelLV",	pSocketData->m_kStatRelLVData.m_fAtkPhysicRelLV, 0.f );
		LUA_GET_VALUE( luaManager,	"m_fAtkMagicRelLV",		pSocketData->m_kStatRelLVData.m_fAtkMagicRelLV, 0.f );
		LUA_GET_VALUE( luaManager,	"m_fDefPhysicRelLV",	pSocketData->m_kStatRelLVData.m_fDefPhysicRelLV, 0.f );
		LUA_GET_VALUE( luaManager,	"m_fDefMagicRelLV",		pSocketData->m_kStatRelLVData.m_fDefMagicRelLV, 0.f );

		luaManager.EndTable();
	}
#endif //SERV_NEW_ITEM_SYSTEM_2013_05



	LUA_GET_VALUE( luaManager,		"m_fHyperModeTime",			pSocketData->m_fHyperModeTime,			0.0f	);
	LUA_GET_VALUE( luaManager,		"m_fRepairPriceSale",		pSocketData->m_fRepairPriceSale,		0.0f	);
	LUA_GET_VALUE( luaManager,		"m_fMoveSpeed",				pSocketData->m_fMoveSpeed,				0.0f	);
	LUA_GET_VALUE( luaManager,		"m_fJumpSpeed",				pSocketData->m_fJumpSpeed,				0.0f	);
	LUA_GET_VALUE( luaManager,		"m_fHyperModeChargeRate",	pSocketData->m_fHyperModeChargeRate,	0.0f	);
	//LUA_GET_VALUE( luaManager,		"m_fRepairPriceSale",		pSocketData->m_fRepairPriceSale,		0.0f	);  // ???????

	//{{ kimhc // 2011-07-19 // 옵션데이타 수치화 작업
#ifdef	NOT_USE_PERCENT_IN_OPTION_DATA
	LUA_GET_VALUE( luaManager,		"m_fAdditionalAttackValue",		pSocketData->m_fAdditionalAttackValue,	0.0f	);
	LUA_GET_VALUE( luaManager,		"m_fAdditionalDefenceValue",	pSocketData->m_fAdditionalDefenceValue,	0.0f	);
#endif	NOT_USE_PERCENT_IN_OPTION_DATA
	//}} kimhc // 2011-07-19 // 옵션데이타 수치화 작업

	UINT uiBuffFactorId = 0;
	LUA_GET_VALUE( luaManager, "BUFF_FACTOR_ID", uiBuffFactorId, 0 );

	if ( uiBuffFactorId )
		pSocketData->m_ptrBuffFactor = CX2BuffTempletManager::GetInstance()->GetBuffFactorPtr( uiBuffFactorId );
		
	//특수 데미지
	if( luaManager.BeginTable( L"EXTRA_DAMAGE" ) == true )
	{
		CX2DamageManager::ExtraDamageData socketExtraDamage;

		LUA_GET_VALUE_ENUM( luaManager, "EXTRA_DAMAGE_TYPE",	socketExtraDamage.m_ExtraDamageType, CX2DamageManager::EXTRA_DAMAGE_TYPE,	CX2DamageManager::EDT_NONE );
		
		float fValue = 0.f;
		LUA_GET_VALUE( luaManager, "RATE",				fValue,		0.f );
		socketExtraDamage.m_fRate			= fValue;
#ifdef EXTRADAMAGE_RATE_BUG_FIX				
		socketExtraDamage.m_fPureRate		= fValue;
#endif EXTRADAMAGE_RATE_BUG_FIX	

		LUA_GET_VALUE( luaManager, "TIME",				fValue,		0.f );
		socketExtraDamage.m_fTime			= fValue;

		LUA_GET_VALUE( luaManager, "DAMAGE_PER_SEC",	fValue,		0.f );
		socketExtraDamage.m_DamagePerSec	= fValue;

#ifdef NEW_EXTRA_DAMAGE
		int iLevel;
		LUA_GET_VALUE( luaManager, "LEVEL",	iLevel,	-1 );
		socketExtraDamage.m_iLevel = iLevel;

		//kimhc //2010-08-04 // LEVEL을 0으로 했을 경우에 현재 자기 레벨 + ADD_LEVEL로 셋팅
		LUA_GET_VALUE( luaManager, "ADD_LEVEL",			iLevel,	0 );
		socketExtraDamage.m_iAddLevel = iLevel;

		bool bIgnoreResist;
		LUA_GET_VALUE( luaManager, "IGNORE_RESIST",	bIgnoreResist,	false );
		socketExtraDamage.m_bIgnoreResist = bIgnoreResist;
#endif
#ifdef ADDITIONAL_MEMO
		bool bDefenderLevel;
		LUA_GET_VALUE( luaManager, "DEFENDER_LEVEL",	bDefenderLevel,	false );
		socketExtraDamage.m_bDefenderLevel = bDefenderLevel;		
#endif

		switch( socketExtraDamage.m_ExtraDamageType )
		{
		case CX2DamageManager::EDT_NONE:
			{
				// nothing
			} break;
		case CX2DamageManager::EDT_POISON:
		case CX2DamageManager::EDT_SLOW:
		case CX2DamageManager::EDT_FIRE:
		case CX2DamageManager::EDT_CURSE:
		case CX2DamageManager::EDT_SILENCE:
		case CX2DamageManager::EDT_LIGHTNING:
		case CX2DamageManager::EDT_MANA_DAMAGE:
		case CX2DamageManager::EDT_LEG_WOUND:
			{
				pSocketData->m_SocketExtraDamage = socketExtraDamage;
			} break;

		case CX2DamageManager::EDT_FIRE_SMASH:
		case CX2DamageManager::EDT_COLD_SMASH:
		case CX2DamageManager::EDT_POISON_CLOUD:
		case CX2DamageManager::EDT_EVENT_SMASH:	
#ifdef CAMILLA_SMASH
		case CX2DamageManager::EDT_CAMILLA_SMASH:
#endif CAMILLA_SMASH
#ifdef EDT_EVENT_SMASH_THIRD
		case CX2DamageManager::EDT_EVENT_SMASH_3:	
#endif //EDT_EVENT_SMASH_THIRD
#ifdef EDT_EVENT_SMASH_SECOND
		case CX2DamageManager::EDT_EVENT_SMASH_2:	
#endif EDT_EVENT_SMASH_SECOND
		case CX2DamageManager::EDT_DOUBLE_ATTACK:
		case CX2DamageManager::EDT_LIGHTNING_SMASH:

		//{{ kimhc // 2010.12.29 // 하멜 마을 추가시 들어가는 ExtraDamage
#ifdef	ADD_HAMEL_VILLAGE
		case CX2DamageManager::EDT_ICE_STING:
		case CX2DamageManager::EDT_FIRE_BALL:
#endif	ADD_HAMEL_VILLAGE
		//}} kimhc // 2010.12.29 // 하멜 마을 추가시 들어가는 ExtraDamage
#ifdef HAMEL_NEW_SETOPTION_EXTRADAMAGE
		case CX2DamageManager::EDT_WIND_STORM:
		case CX2DamageManager::EDT_EARTH_QUAKE:
#endif HAMEL_NEW_SETOPTION_EXTRADAMAGE

#ifdef	VELDER_SECRET_DUNGEON
		case CX2DamageManager::EDT_CURSE_CLOUD:
#endif	VELDER_SECRET_DUNGEON
#ifdef ADD_SOCKET_FOR_SECRET_SET
		case CX2DamageManager::EDT_FLAME_SMASH:
		case CX2DamageManager::EDT_FIRE_WRAITH:
#endif
#ifdef STARLIGHT_SMASH
		case CX2DamageManager::EDT_STARLIGHT_SMASH:
#endif STARLIGHT_SMASH
#ifdef ADD_SOCKET_OPTION_SANDER_SET
		case CX2DamageManager::EDT_FALLING_ROCK:/// 샌더 셋트 효과 - 바위 소환 
		case CX2DamageManager::EDT_SANDSTORM:/// 샌더 셋트 효과 - 모래돌풍 소환
		case CX2DamageManager::EDT_WIND_VIBRATION:/// 샌더 칭호 진동 타격
		case CX2DamageManager::EDT_BREEZE_ACCEL:/// 샌더 칭호 산들바람의 기운 생성
		case CX2DamageManager::EDT_EVIL_SWORD:
		case CX2DamageManager::EDT_SPIRIT_SWORD:
		case CX2DamageManager::EDT_GHOST_SWORD:
#endif //ADD_SOCKET_OPTION_SANDER_SET
			{
				pSocketData->m_MultipleSocketExtraDamage.push_back( socketExtraDamage );
			} break;
		}

		luaManager.EndTable();
	}


	if( true == luaManager.BeginTable( L"RESIST_ENCHANT" ) )
	{
		LUA_GET_VALUE( luaManager, "BLAZE",		pSocketData->m_aiResistEnchant[ CX2EnchantItem::ET_BLAZE ],		0 );
		LUA_GET_VALUE( luaManager, "WATER",		pSocketData->m_aiResistEnchant[ CX2EnchantItem::ET_WATER ],		0 );
		LUA_GET_VALUE( luaManager, "NATURE",	pSocketData->m_aiResistEnchant[ CX2EnchantItem::ET_NATURE ],	0 );
		LUA_GET_VALUE( luaManager, "WIND",		pSocketData->m_aiResistEnchant[ CX2EnchantItem::ET_WIND ],		0 );
		LUA_GET_VALUE( luaManager, "LIGHT",		pSocketData->m_aiResistEnchant[ CX2EnchantItem::ET_LIGHT ],		0 );
		LUA_GET_VALUE( luaManager, "DARK",		pSocketData->m_aiResistEnchant[ CX2EnchantItem::ET_DARK ],		0 );

		luaManager.EndTable();
	}

#ifdef PET_AURA_SKILL
	if( true == luaManager.BeginTable( L"ATTACK_ENCHANT" ) )
	{
		LUA_GET_VALUE( luaManager, "BLAZE",		pSocketData->m_afAttackEnchant[ CX2EnchantItem::ET_BLAZE ],		0.f );
		LUA_GET_VALUE( luaManager, "WATER",		pSocketData->m_afAttackEnchant[ CX2EnchantItem::ET_WATER ],		0.f );
		LUA_GET_VALUE( luaManager, "NATURE",	pSocketData->m_afAttackEnchant[ CX2EnchantItem::ET_NATURE ],	0.f );
		LUA_GET_VALUE( luaManager, "WIND",		pSocketData->m_afAttackEnchant[ CX2EnchantItem::ET_WIND ],		0.f );
		LUA_GET_VALUE( luaManager, "LIGHT",		pSocketData->m_afAttackEnchant[ CX2EnchantItem::ET_LIGHT ],		0.f );
		LUA_GET_VALUE( luaManager, "DARK",		pSocketData->m_afAttackEnchant[ CX2EnchantItem::ET_DARK ],		0.f );

		luaManager.EndTable();
	}
#endif



	LUA_GET_VALUE(		luaManager, L"DUNGEON_ONLY",		pSocketData->m_bDungeonOnly,		false );
	LUA_GET_VALUE(		luaManager, L"PVP_ONLY",			pSocketData->m_bPVPOnly,			false );

	LUA_GET_VALUE(		luaManager, L"OVER_LAP",			pSocketData->m_bOverlap,			true );

	
	LUA_GET_VALUE(		luaManager, L"m_fDamageReduce",				pSocketData->m_fDamageReduce,					0.0f );
	LUA_GET_VALUE(		luaManager, L"m_fPercentDamageReduce",		pSocketData->m_fPercentDamageReduce,			0.0f );

	wstring monsterIDDamageReduced;
	LUA_GET_VALUE(		luaManager, L"m_SetMonsterIDDamageReduced",	monsterIDDamageReduced,							L"Group 0" );
	if ( monsterIDDamageReduced.empty() == true )
	{
		ErrorLog( KEM_ERROR359 );
	}
	else
	{
		if ( monsterIDDamageReduced.compare( L"Group 0" ) != 0 )
		{
			int stringIndex = monsterIDDamageReduced.find( L" " );
			if ( stringIndex != -1 )
			{
				wstring tempMonsterID;
				while( monsterIDDamageReduced.find( L" ", stringIndex + 1 ) != -1 )
				{
					int debugIndex = monsterIDDamageReduced.find( L" ", stringIndex + 1 );
					tempMonsterID = monsterIDDamageReduced.substr( stringIndex + 1, debugIndex - stringIndex - 1 );
					if ( tempMonsterID.compare( L"0" ) != 0 )
						pSocketData->m_setMonsterIDDamageReduced.insert( _wtoi( tempMonsterID.c_str() ) ); 
					stringIndex = debugIndex;
				}

				tempMonsterID = monsterIDDamageReduced.substr( stringIndex + 1 );
				if ( tempMonsterID.compare( L"0" ) != 0 )
					pSocketData->m_setMonsterIDDamageReduced.insert( _wtoi( tempMonsterID.c_str() ) );
			}
		}
	}


	LUA_GET_VALUE(		luaManager, L"m_fPercentDRByMonsterAttack",				pSocketData->m_fPercentDRByMonsterAttack,			0.0f );
	LUA_GET_VALUE(		luaManager, L"m_fDamageReduceByMonsterAttack",			pSocketData->m_fDamageReduceByMonsterAttack,		0.0f );
	LUA_GET_VALUE(		luaManager, L"m_DescEvadeByMonsterAttack",				pSocketData->m_DescEvadeByMonsterAttack,		L"" );


	LUA_GET_VALUE(		luaManager, L"m_fEvadePercent",							pSocketData->m_fEvadePercent,				0.0f );

	LUA_GET_VALUE(		luaManager, L"m_fAntiEvadePercent",						pSocketData->m_fAntiEvadePercent,			0.0f );



	LUA_GET_VALUE(		luaManager, L"m_iAnimationSpeedRate",					pSocketData->m_fAnimationSpeedRate,			0.0f );


	LUA_GET_VALUE(		luaManager, L"m_fPercentCritical",						pSocketData->m_fPercentCritical,			0.0f );

	LUA_GET_VALUE(		luaManager, L"m_fCritical",								pSocketData->m_fCritical,					0.0f );	

	
	//{{ kimhc // 2011-05-25 // SetOption, TitleOption에만 적용 되는 option 모든 itemOption에 적용 되도록 수정
#ifdef	OPTION_ITEM_DATA_MODIFY

	D3DXVECTOR2 vRateAndHpUpRatio;
	//  공격을 당했을 때 HP 증가 효과가 발동할 확률
	LUA_GET_VALUE(		luaManager, L"m_fHPRecoverRateByAttacked",		vRateAndHpUpRatio.x,		0.0f );

	// 공격을 당했을 때 HP가 증가할 확률이 0보다 크면
	if ( 0.0f < vRateAndHpUpRatio.x )
	{
		LUA_GET_VALUE(		luaManager, L"m_fHPRecoverRatioByAttacked",				vRateAndHpUpRatio.y,		0.0f );
		pSocketData->m_vecHpUpByAttacked.push_back( vRateAndHpUpRatio );
	}

	// HPRecoverValueByAttacked 미적용

	//  공격을 했을 때 HP 증가 효과가 발동할 확률
	LUA_GET_VALUE(		luaManager, L"m_fHPRecoverRateByAttack",			vRateAndHpUpRatio.x,		0.0f );

	// 공격을 했을 때 HP가 증가할 확률이 0보다 크면
	if ( 0.0f < vRateAndHpUpRatio.x )
	{
		LUA_GET_VALUE(		luaManager, L"m_fHPRecoverRatioByAttack",				vRateAndHpUpRatio.y,		0.0f );
		pSocketData->m_vecHpUpByAttack.push_back( vRateAndHpUpRatio );
	}

	D3DXVECTOR2 vRateAndMPValue;
	LUA_GET_VALUE(		luaManager, L"m_fPercentMPUpByAttacked",				vRateAndMPValue.x,		0.0f );

	// 공격을 당했을 때 MP가 증가할 확률이 0보다 크면
	if ( 0.0f < vRateAndMPValue.x )
	{
		LUA_GET_VALUE(		luaManager, L"m_fMPUpByAttacked",						vRateAndMPValue.y,		0.0f );
		pSocketData->m_vecMpUpByAttacked.push_back( vRateAndMPValue );
	}

	// 공격을 했을 때 MP가 증가할 확률이 0보다 크면
	LUA_GET_VALUE(		luaManager, L"m_fPercentMPUpByAttack",					vRateAndMPValue.x,		0.0f );
	if ( 0.0f < vRateAndMPValue.x )
	{
		LUA_GET_VALUE(		luaManager, L"m_fMPUpByAttack",							vRateAndMPValue.y,		0.0f );
		pSocketData->m_vecMpUpByAttack.push_back( vRateAndMPValue );
	}
#else	OPTION_ITEM_DATA_MODIFY
	LUA_GET_VALUE(		luaManager, L"m_fPercentMPUpByAttacked",				pSocketData->m_fPercentMPUpByAttacked,		0.0f );
	LUA_GET_VALUE(		luaManager, L"m_fMPUpByAttacked",						pSocketData->m_fMPUpByAttacked,				0.0f );
	
	LUA_GET_VALUE(		luaManager, L"m_fPercentMPUpByAttack",					pSocketData->m_fPercentMPUpByAttack,		0.0f );
	LUA_GET_VALUE(		luaManager, L"m_fMPUpByAttack",							pSocketData->m_fMPUpByAttack,				0.0f );
#endif	OPTION_ITEM_DATA_MODIFY
	//}} kimhc // 2011-05-25 // SetOption, TitleOption에만 적용 되는 option 모든 itemOption에 적용 되도록 수정

	//{{ 2011.09.05 / 이지헌 / 대만/홍콩 / 마법의 목걸이 개편 / 병합 : 강정훈
#ifdef MAGIC_NEC_FIX
	LUA_GET_VALUE(		luaManager, L"m_fMPMultiplyByAll",						pSocketData->m_fMPMultiplyByAll,				0.0f );
#endif
	//}}

	LUA_GET_VALUE(		luaManager, L"m_fDamageUpByAMADS",						pSocketData->m_fDamageUpByAMADS,			0.0f );

	LUA_GET_VALUE_ENUM( luaManager, L"m_ResistExtraDamageType",					pSocketData->m_ResistExtraDamageType,		CX2DamageManager::EXTRA_DAMAGE_TYPE,	CX2DamageManager::EDT_NONE );
	LUA_GET_VALUE(		luaManager, L"m_fResistExtraDamage",					pSocketData->m_fResistExtraDamage,			0.0f );

	LUA_GET_VALUE(		luaManager, L"m_AllSkillLevelUp",						pSocketData->m_AllSkillLevelUp,				0 );

	LUA_GET_VALUE(		luaManager, L"m_IncreaseHP",							pSocketData->m_Stat.m_ExtraStat.m_fIncreaseHPRate,		0.f );

#ifdef PET_AURA_SKILL
	LUA_GET_VALUE(		luaManager, L"m_IncreaseMP",							pSocketData->m_Stat.m_ExtraStat.m_fIncreaseMPRate,		0.f );
	LUA_GET_VALUE(		luaManager, L"m_IncreaseAtkPhysic",						pSocketData->m_Stat.m_ExtraStat.m_fIncreaseAtkPhysicRate,		0.f );
	LUA_GET_VALUE(		luaManager, L"m_IncreaseAtkMagic",						pSocketData->m_Stat.m_ExtraStat.m_fIncreaseAtkMagicRate,		0.f );
	LUA_GET_VALUE(		luaManager, L"m_IncreaseDefPhysic",						pSocketData->m_Stat.m_ExtraStat.m_fIncreaseDefPhysicRate,		0.f );
	LUA_GET_VALUE(		luaManager, L"m_IncreaseDefMagic",						pSocketData->m_Stat.m_ExtraStat.m_fIncreaseDefMagicRate,		0.f );
#endif

	wstring skillGroupID = L"";
	LUA_GET_VALUE(		luaManager, L"m_SkillLevelUpID",	skillGroupID,							L"Group 0" );

#ifdef UPGRADE_SKILL_SYSTEM_2013 // 오현빈
	int iSkillLvUpNum = 1;
	LUA_GET_VALUE(		luaManager, L"m_iSkillLevelUpValue", iSkillLvUpNum, 1);
#endif // UPGRADE_SKILL_SYSTEM_2013


	if( false == skillGroupID.empty() )
	{
		int stringIndex = skillGroupID.find( L" " );
		if ( stringIndex != -1 )
		{
			wstring tempSkillID;
			while( skillGroupID.find( L" ", stringIndex + 1 ) != -1 )
			{
				int debugIndex = skillGroupID.find( L" ", stringIndex + 1 );
				tempSkillID = skillGroupID.substr( stringIndex + 1, debugIndex - stringIndex - 1 );
				if ( tempSkillID.compare( L"0" ) != 0 )
				{
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05 // 오현빈
	#ifdef UPGRADE_SKILL_SYSTEM_2013 // 오현빈
					if( SDT_SKILL_LV_GROUP == pSocketData->m_Type )
						pSocketData->m_vecSkillLevelUpGroupID.push_back( (CX2SkillTree::SKILL_ID)_wtoi( tempSkillID.c_str() ) ); 
					else
					{
						SkillLevelUpIDAndNum sSkillLevelUpIDAndNum;
						sSkillLevelUpIDAndNum.first = (CX2SkillTree::SKILL_ID)_wtoi( tempSkillID.c_str() );
						sSkillLevelUpIDAndNum.second = iSkillLvUpNum;

						pSocketData->m_vecSkillLevelUpID.push_back( sSkillLevelUpIDAndNum ); 
					}
	#else
					if( SDT_SKILL_LV_GROUP == pSocketData->m_Type )
						pSocketData->m_vecSkillLevelUpGroupID.push_back( (CX2SkillTree::SKILL_ID)_wtoi( tempSkillID.c_str() ) ); 
					else
						pSocketData->m_vecSkillLevelUpID.push_back( (CX2SkillTree::SKILL_ID)_wtoi( tempSkillID.c_str() ) ); 
	#endif // UPGRADE_SKILL_SYSTEM_2013 
#else
					pSocketData->m_vecSkillLevelUpID.push_back( (CX2SkillTree::SKILL_ID)_wtoi( tempSkillID.c_str() ) ); 
#endif // SERV_NEW_ITEM_SYSTEM_2013_05
				}
				stringIndex = debugIndex;
			}

			tempSkillID = skillGroupID.substr( stringIndex + 1 );
			if ( tempSkillID.compare( L"0" ) != 0 )
			{
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05 // 오현빈
	#ifdef UPGRADE_SKILL_SYSTEM_2013 // 오현빈
				if( SDT_SKILL_LV_GROUP == pSocketData->m_Type )
					pSocketData->m_vecSkillLevelUpGroupID.push_back( (CX2SkillTree::SKILL_ID)_wtoi( tempSkillID.c_str() ) ); 
				else
				{
					SkillLevelUpIDAndNum sSkillLevelUpIDAndNum;
					sSkillLevelUpIDAndNum.first = (CX2SkillTree::SKILL_ID)_wtoi( tempSkillID.c_str() );
					sSkillLevelUpIDAndNum.second = iSkillLvUpNum;

					pSocketData->m_vecSkillLevelUpID.push_back( sSkillLevelUpIDAndNum ); 
				}
	#else
				if( SDT_SKILL_LV_GROUP == pSocketData->m_Type )
					pSocketData->m_vecSkillLevelUpGroupID.push_back( (CX2SkillTree::SKILL_ID)_wtoi( tempSkillID.c_str() ) ); 
				else
					pSocketData->m_vecSkillLevelUpID.push_back( (CX2SkillTree::SKILL_ID)_wtoi( tempSkillID.c_str() ) ); 
	#endif
#else
				pSocketData->m_vecSkillLevelUpID.push_back( (CX2SkillTree::SKILL_ID)_wtoi( tempSkillID.c_str() ) ); 
#endif // SERV_NEW_ITEM_SYSTEM_2013_05
			}
		}
	}

#ifdef BUFF_TEMPLET_SYSTEM
	LUA_GET_VALUE(		luaManager, L"m_iAddMpValue",							pSocketData->m_iAddMPValue,					0	 );// MP 증가량
	LUA_GET_VALUE(		luaManager, L"m_iMpChangeValue",						pSocketData->m_iMPChangeValue,				0	 );// MP 자연 회복량
	LUA_GET_VALUE(		luaManager, L"m_fHpRelativeChangeRate",					pSocketData->m_fHPRelativeChangeRate,		0.0f );// HP 전체 대비 자연 회복량
	LUA_GET_VALUE(		luaManager, L"m_fSkillDamageUpRate",					pSocketData->m_fSkillDamageUpRate,			0.0f );// 스킬 공격 증가 데미지값	
	LUA_GET_VALUE(		luaManager, L"m_fAllAttackEnchantRate",					pSocketData->m_fAllAttackEnchantRate,		1.0f );// 속성 발동 확률 % 증가
#endif BUFF_TEMPLET_SYSTEM
#ifdef SUMMON_NPC_SOCKET
	LUA_GET_VALUE(		luaManager, L"m_iSummonNpcId",							pSocketData->m_iSummonNpcId,				0 );
	LUA_GET_VALUE(		luaManager, L"m_fSummonNpcRate",						pSocketData->m_fSummonNpcRate,				0.f );
	LUA_GET_VALUE(		luaManager, L"m_fSummonNpcCoolTime",					pSocketData->m_fSummonNpcCoolTime,			20.f );	
#endif

	LUA_GET_VALUE(		luaManager, L"m_fDamageUpPerAtSpecificMonster",			pSocketData->m_fDamageUpPerAtSpecificMonster,				0 );

	wstring monsterIDForDamageUp = L"";
	LUA_GET_VALUE(		luaManager, L"m_setMonsterIDForDamageUp",				monsterIDForDamageUp,							L"Group 0" );
	if ( monsterIDForDamageUp.empty() == true )
	{
		ErrorLog( KEM_ERROR359 );
	}
	else
	{
		if ( monsterIDForDamageUp.compare( L"Group 0" ) != 0 )
		{
			int stringIndex = monsterIDForDamageUp.find( L" " );
			if ( stringIndex != -1 )
			{
				wstring tempMonsterID;
				while( monsterIDForDamageUp.find( L" ", stringIndex + 1 ) != -1 )
				{
					int debugIndex = monsterIDForDamageUp.find( L" ", stringIndex + 1 );
					tempMonsterID = monsterIDForDamageUp.substr( stringIndex + 1, debugIndex - stringIndex - 1 );
					if ( tempMonsterID.compare( L"0" ) != 0 )
						pSocketData->m_setMonsterIDForDamageUp.insert( _wtoi( tempMonsterID.c_str() ) ); 
					stringIndex = debugIndex;
				}

				tempMonsterID = monsterIDForDamageUp.substr( stringIndex + 1 );
				if ( tempMonsterID.compare( L"0" ) != 0 )
					pSocketData->m_setMonsterIDForDamageUp.insert( _wtoi( tempMonsterID.c_str() ) );
			}
		}
	}



	LUA_GET_VALUE(		luaManager, L"m_fEvadePercentByMonsterAttack",			pSocketData->m_fEvadePercentByMonsterAttack,				0 );

	wstring monsterIDForEvade = L"";
	LUA_GET_VALUE(		luaManager, L"m_setMonsterIDForEvade",					monsterIDForEvade,							L"Group 0" );
	if ( monsterIDForEvade.empty() == true )
	{
		ErrorLog( KEM_ERROR359 );
	}
	else
	{
		if ( monsterIDForEvade.compare( L"Group 0" ) != 0 )
		{
			int stringIndex = monsterIDForEvade.find( L" " );
			if ( stringIndex != -1 )
			{
				wstring tempMonsterID;
				while( monsterIDForEvade.find( L" ", stringIndex + 1 ) != -1 )
				{
					int debugIndex = monsterIDForEvade.find( L" ", stringIndex + 1 );
					tempMonsterID = monsterIDForEvade.substr( stringIndex + 1, debugIndex - stringIndex - 1 );
					if ( tempMonsterID.compare( L"0" ) != 0 )
						pSocketData->m_setMonsterIDForEvade.insert( _wtoi( tempMonsterID.c_str() ) ); 
					stringIndex = debugIndex;
				}

				tempMonsterID = monsterIDForEvade.substr( stringIndex + 1 );
				if ( tempMonsterID.compare( L"0" ) != 0 )
					pSocketData->m_setMonsterIDForEvade.insert( _wtoi( tempMonsterID.c_str() ) );
			}
		}
	}

	LUA_GET_VALUE_ENUM( luaManager, L"m_ImmuneExtraDamageType",					pSocketData->m_ImmuneExtraDamageType,		CX2DamageManager::EXTRA_DAMAGE_TYPE,	CX2DamageManager::EDT_NONE );
	LUA_GET_VALUE(		luaManager, L"m_fPercentImmuneExtraDamage",				pSocketData->m_fPercentImmuneExtraDamage,	0.0f );

	LUA_GET_VALUE(		luaManager, L"m_fSpeedUpManaGather",					pSocketData->m_fSpeedUpManaGather,			0.0f );

	LUA_GET_VALUE(		luaManager, L"m_fIncreaseEDPercent",					pSocketData->m_fIncreaseEDPercent,			0.0f );
	LUA_GET_VALUE(		luaManager, L"m_fIncreaseDCBExpPercent",				pSocketData->m_fIncreaseDCBExpPercent,		0.0f );

	LUA_GET_VALUE(		luaManager, L"m_fPercentSuperArmor",					pSocketData->m_fPercentSuperArmor,		0.0f );
	LUA_GET_VALUE(		luaManager, L"m_fTimeSuperArmor",						pSocketData->m_fTimeSuperArmor,			0.0f );
	
//{{ kimhc // 2010.4.1 // 비밀던전 작업(셋트아이템효과)
#ifdef SERV_SECRET_HELL
	LUA_GET_VALUE_ENUM( luaManager, L"m_eTypeSetItemEffectEx",					pSocketData->m_eTypeSocketItemEffectEx,		SOCKET_ITEM_EFFECT_EX,	SIEE_NONE );
	LUA_GET_VALUE_ENUM(	luaManager, L"m_eStatUpType",							pSocketData->m_eStatUpType,			STAT_UP_TYPE,	SUT_NONE );
	LUA_GET_VALUE(		luaManager, L"m_fDurationTime",							pSocketData->m_fDurationTime,				0.0f );
	LUA_GET_VALUE(		luaManager, L"m_fProbability",							pSocketData->m_fProbability,				0.0f );
	LUA_GET_VALUE(		luaManager, L"m_fIncOrDecRate",							pSocketData->m_fIncOrDecRate,				0.0f );
	LUA_GET_VALUE(		luaManager, L"m_fIncOrDecVal",							pSocketData->m_fIncOrDecVal,				0.0f );
	LUA_GET_VALUE(		luaManager, L"m_iCount",								pSocketData->m_iCount,						0 );
	LUA_GET_VALUE(		luaManager, L"m_fIntervalTime",							pSocketData->m_fIntervalTime,				0.0f );

	if ( luaManager.BeginTable( L"EFFECT_NAME" ) == true )
	{
		wstring wstrEffectName;
		
		for ( int i = 1; luaManager.GetValue( i, wstrEffectName ) == true; i++ )
			pSocketData->m_vecEffectName.push_back( wstrEffectName );
	}
#endif SERV_SECRET_HELL
//}} kimhc // 2010.4.1 // 비밀던전 작업(셋트아이템효과)

#ifdef SERV_GROW_UP_SOCKET
	wstring GrowUpSocketID;
	LUA_GET_VALUE(		luaManager, L"m_vecGrowUpSocketID",						GrowUpSocketID,								L"Group 0" );
	SetGrowUpSocketData( GrowUpSocketID, pSocketData->m_mapGrowUpSocketID );	
#endif //SERV_GROW_UP_SOCKET

#ifdef PVP_SEASON2_SOCKET
	LUA_GET_VALUE( luaManager,	L"m_bIsLevelLinkedStat",	pSocketData->m_bIsLevelLinkedStat,	false	);
	LUA_GET_VALUE( luaManager,	L"m_iAddLevel",				pSocketData->m_iAddLevel,			0		);
#endif

#ifdef SERV_NEW_ITEM_SYSTEM_2013_05

	int iStrID = -1;
	LUA_GET_VALUE( luaManager,	L"m_eDescStringID",	iStrID, -1);
	if( -1 != iStrID )
		m_mapSocketDescStrID.insert(  std::make_pair(pSocketData->m_ID, iStrID));
#endif //SERV_NEW_ITEM_SYSTEM_2013_05

	//{{ kimhc // 2011-07-05 // 옵션데이타 수치화 작업
#ifdef	NOT_USE_PERCENT_IN_OPTION_DATA
	// 소수로 표기된 확률에 100을 곱하여 배율(기획에서 편의상 의미하는 단계)로 사용함
	pSocketData->m_fMoveSpeed			*= 100;
	pSocketData->m_fJumpSpeed			*= 100;
	pSocketData->m_fAntiEvadePercent	*= 100;
	pSocketData->m_fEvadePercent		*= 100;
	pSocketData->m_fPercentCritical		*= 100;
	pSocketData->m_fHyperModeChargeRate	*= 100;

	pSocketData->m_Stat.m_ExtraStat.m_fIncreaseHPRate *= 100;

	// 1초당 1.5% 증가를 나타냄
	pSocketData->m_fHyperModeTime		*= 1.5;

	pSocketData->m_fAdditionalAttackValue	*= 100;
	pSocketData->m_fAdditionalDefenceValue	*= 100;
	// 애니메이션 스피드는 이미 정수로 되어 있어서 제외
#endif	NOT_USE_PERCENT_IN_OPTION_DATA
	//}} kimhc // 2011-07-05 // 옵션데이타 수치화 작업

	if( m_mapSocketOptionPool.find(pSocketData->m_ID) != m_mapSocketOptionPool.end() )
	{
        wstringstream strstm;
        strstm << pSocketData->m_ID;
        ErrorLogMsg( XEM_ERROR127, strstm.str().c_str() );

		SAFE_DELETE(pSocketData); 		
#ifndef _SERVICE_
		MessageBox( g_pKTDXApp->GetHWND(), strstm.str().c_str(), L"SocketItem Error", MB_OK );
#endif
	}
	else
	{
		m_mapSocketOptionPool.insert( std::make_pair(pSocketData->m_ID, pSocketData) );
		return true;
	}

	return false;
}

#ifdef LUA_TRANS_DEVIDE
bool CX2SocketItem::AddSocketDataTrans_LUA()
{
	KLuaManager luaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState() );
	TableBind( &luaManager, g_pKTDXApp->GetLuaBinder() );

	SocketData* pSocketData = new SocketData();

	//데이터 로드
	LUA_GET_VALUE(		luaManager, L"ID",		pSocketData->m_ID,		0 );
	LUA_GET_VALUE(		luaManager, L"m_DescEvadeByMonsterAttack",				pSocketData->m_DescEvadeByMonsterAttack,		L"" );

	// 여기서 치환 해주어야 함.
	std::map<int,SocketData*>::iterator mit;
	mit = m_mapSocketOptionPool.find( pSocketData->m_ID );
	if( mit == m_mapSocketOptionPool.end() )
	{
		wstringstream wstrStream;
		wstrStream << L"OptionItemDataTrans.lua And OptionItemData.lua Not Equal ID..." << (pSocketData->m_ID) << L"_";
		ErrorLogMsg( XEM_ERROR105, wstrStream.str().c_str() );	
		// 에러로그 
		SAFE_DELETE(pSocketData); 
		return false;
	}
	else
	{
		mit->second->m_DescEvadeByMonsterAttack = pSocketData->m_DescEvadeByMonsterAttack;
	}

	return true;
}
#endif LUA_TRANS_DEVIDE


bool CX2SocketItem::GetIsPossibleSocketItemByOnlyItemType( int itemID )
{
	const CX2Item::ItemTemplet* pItemTemplet = g_pData->GetItemManager()->GetItemTemplet( itemID );
	if ( pItemTemplet != NULL )
	{
		if ( pItemTemplet->GetItemType() == CX2Item::IT_WEAPON ||
			pItemTemplet->GetItemType() == CX2Item::IT_DEFENCE )
			return true;
	}

	return false;
}

//{{ 최민철 [2013/1/4]  게임내 정보 스트링을 엑셀파일로 출력
#ifdef PRINT_INGAMEINFO_TO_EXCEL
void CX2SocketItem::PrintOptionInfo_ToExcel( KProtectedType<int> iLevel )
{
	BasicExcel e;
	e.New(1);

	BasicExcelWorksheet* pSheet = e.GetWorksheet((size_t)0);
	int iCal = 1;
	//해당 캐릭터 레벨로 수치가 계산됩니다.
	wstring strlevel;
	strlevel = str(boost::wformat(L"레벨 : %d") %iLevel);

	//칼럼 설명
	pSheet->Cell(0,0)->SetWString(L"Socket ID");	
	pSheet->Cell(0,1)->SetWString(L"Socket 설명");	
	pSheet->Cell(0,2)->SetWString(strlevel.c_str());	

	//테이타 엑셀에 출력
	std::map<int,SocketData*>::iterator iter;
	for( iter = m_mapSocketOptionPool.begin(); iter != m_mapSocketOptionPool.end(); iter++ )
	{
		SocketData* pSocketData = iter->second;

		pSheet->Cell(iCal,0)->SetInteger( pSocketData->m_ID );	
		pSheet->Cell(iCal,1)->SetWString( pSocketData->GetSocketDesc(iLevel , true).c_str());	
		iCal++;

	}
	e.SaveAs("Socket_LIST.xls");
}
#endif PRINT_INGAMEINFO_TO_EXCEL
//}} 최민철 [2013/1/4]  게임내 정보 스트링을 엑셀파일로 출력

//{{ kimhc // 2011-07-05 // 옵션데이타 수치화 작업
#ifdef	NOT_USE_PERCENT_IN_OPTION_DATA
	#ifdef SERV_NEW_ITEM_SYSTEM_2013_05 // 오현빈
	wstring CX2SocketItem::SocketData::GetSocketDesc( const int iSocketLevel_, bool bCompact /*= false*/, CX2Unit::UNIT_CLASS eUnitClass_ /*= CX2Unit::UC_NONE */ )
	#else
	wstring CX2SocketItem::SocketData::GetSocketDesc( const int iSocketLevel_, bool bCompact /*= false*/ )
	#endif // SERV_NEW_ITEM_SYSTEM_2013_05
#else	NOT_USE_PERCENT_IN_OPTION_DATA
wstring CX2SocketItem::SocketData::GetSocketDesc(bool bCompact /*= false*/ )
#endif	NOT_USE_PERCENT_IN_OPTION_DATA
//}} kimhc // 2011-07-05 // 옵션데이타 수치화 작업
{
	wstringstream wstrstm;

#ifdef PVP_SEASON2_SOCKET
	int iLevelValue = 1;
	
	if ( true == m_bIsLevelLinkedStat )
		iLevelValue = iSocketLevel_ + m_iAddLevel;
#endif

#ifdef SERV_SOCKET_NEW
//{{ oasis907 : 김상윤 [2010.4.20] // 고정 옵션 마법석
#ifdef PVP_SEASON2_SOCKET
	if ( m_Stat.m_fAtkPhysic != 0 && m_Stat.m_fDefMagic != 0  ) // 조화의 마법석
	{
		wstrstm << GET_STRING( STR_ID_479 ) << L"  ";
		if ( m_Stat.m_fAtkPhysic > 0 )
			wstrstm << L"+";
		else
			wstrstm << L"-";
		
		wstrstm << abs( static_cast<int>( m_Stat.m_fAtkPhysic * iLevelValue ) );
		wstrstm << L", ";

		wstrstm << GET_STRING( STR_ID_482 ) << L" ";
		if ( m_Stat.m_fDefMagic > 0 )
			wstrstm << L"+";
		else
			wstrstm << L"-";

		wstrstm  << abs( static_cast<int>( m_Stat.m_fDefMagic * iLevelValue ) );
	}
	else if ( m_Stat.m_fAtkMagic != 0 && m_Stat.m_fDefPhysic != 0  ) // 신비의 마법석
	{
		wstrstm << GET_STRING( STR_ID_480 ) << L"  ";
		if ( m_Stat.m_fAtkMagic > 0 )
			wstrstm << L"+";
		else
			wstrstm << L"-";
		
		wstrstm << abs( static_cast<int>( m_Stat.m_fAtkMagic * iLevelValue ) );
		wstrstm << L", ";
		
		wstrstm << GET_STRING( STR_ID_481 ) << L" ";
		if ( m_Stat.m_fDefPhysic > 0 )
			wstrstm << L"+";
		else
			wstrstm << L"-";

		wstrstm << abs( static_cast<int>( m_Stat.m_fDefPhysic * iLevelValue ) );
	}
#else
	if ( m_Stat.m_fAtkPhysic != 0 && m_Stat.m_fDefMagic != 0  ) // 조화의 마법석
	{
		wstrstm << GET_STRING( STR_ID_479 ) << L"  ";
		if ( m_Stat.m_fAtkPhysic > 0 )
		{
			wstrstm << L"+" << abs( static_cast<int>( m_Stat.m_fAtkPhysic ) );
		}
		else
		{
			wstrstm << L"-" << abs( static_cast<int>( m_Stat.m_fAtkPhysic ) );
		}
		wstrstm << L", ";
		wstrstm << GET_STRING( STR_ID_482 ) << L" ";
		if ( m_Stat.m_fDefMagic > 0 )
		{
			wstrstm << L"+" << abs( static_cast<int>( m_Stat.m_fDefMagic ) );
		}
		else
		{
			wstrstm << L"-" << abs( static_cast<int>( m_Stat.m_fDefMagic ) );
		}
	}
	else if ( m_Stat.m_fAtkMagic != 0 && m_Stat.m_fDefPhysic != 0  ) // 신비의 마법석
	{
		wstrstm << GET_STRING( STR_ID_480 ) << L"  ";
		if ( m_Stat.m_fAtkMagic > 0 )
		{
			wstrstm << L"+" << abs( static_cast<int>( m_Stat.m_fAtkMagic ) );
		}
		else
		{
			wstrstm << L"-" << abs( static_cast<int>( m_Stat.m_fAtkMagic ) );
		}
		wstrstm << L", ";
		wstrstm << GET_STRING( STR_ID_481 ) << L" ";
		if ( m_Stat.m_fDefPhysic > 0 )
		{
			wstrstm << L"+" << abs( static_cast<int>( m_Stat.m_fDefPhysic ) );
		}
		else
		{
			wstrstm << L"-" << abs( static_cast<int>( m_Stat.m_fDefPhysic ) );
		}
	}
#endif
	else if( 0 != m_fAnimationSpeedRate && m_fHyperModeTime > 0) // 광휘의 마법석
	{
		//{{ kimhc // 2011-07-05 // 옵션데이타 수치화 작업
#ifdef	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13581, "h", GetLinearAnimationSpeedValue( iSocketLevel_ ) ) );
		wstrstm << L", ";
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13578, "f", m_fHyperModeTime ) );
#else	NOT_USE_PERCENT_IN_OPTION_DATA
		if(bCompact)
		{
			wstrstm << GET_REPLACED_STRING( ( STR_ID_5098, "h", (int)m_fAnimationSpeedRate ) );
			wstrstm << L", ";
			wstrstm << GET_REPLACED_STRING( ( STR_ID_5099, "h", ( int )m_fHyperModeTime) );
		}
		else
		{
			wstrstm << GET_REPLACED_STRING( ( STR_ID_525, "hi", (int)m_fAnimationSpeedRate, (int)CX2SocketItem::SocketData::MAX_ANIMATION_SPEED_RATE ) );
			wstrstm << L", ";
			wstrstm << GET_REPLACED_STRING( ( STR_ID_488, "hi", ( int )m_fHyperModeTime, ( int )CX2SocketItem::SocketData::MAX_HYPERMODETIME ) );
		}
#endif	NOT_USE_PERCENT_IN_OPTION_DATA
		//}} kimhc // 2011-07-05 // 옵션데이타 수치화 작업
	}
	else if ( m_Stat.m_ExtraStat.m_fIncreaseHPRate != 0 && m_fAntiEvadePercent > 0.0f ) // 전투의 마법석
	{
		//{{ kimhc // 2011-07-05 // 옵션데이타 수치화 작업
#ifdef	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_5100, "h", ( int )( m_Stat.m_ExtraStat.m_fIncreaseHPRate * GetHpIncrementValue( iSocketLevel_ ) ) ) );
		wstrstm << L", ";
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13576, "h", GetLinearAntiEvadeValue( iSocketLevel_ ) ) );
#else	NOT_USE_PERCENT_IN_OPTION_DATA
		if(bCompact)
		{
			wstrstm << GET_REPLACED_STRING( ( STR_ID_5100, "h", ( int )( m_Stat.m_ExtraStat.m_fIncreaseHPRate * 100 ) ) );
			wstrstm << L", ";
			wstrstm << GET_REPLACED_STRING( ( STR_ID_5101, "f", ( m_fAntiEvadePercent * 100 ) ) );

		}
		else
		{
			wstrstm << GET_REPLACED_STRING( ( STR_ID_484, "hi", ( int )( m_Stat.m_ExtraStat.m_fIncreaseHPRate * 100 ), ( int )( CX2Stat::Stat::MAX_INCREASE_HP_RATE * 100.f ) ) );
			wstrstm << L", ";
			wstrstm << GET_REPLACED_STRING( ( STR_ID_502, "fi", ( m_fAntiEvadePercent * 100 ), ( int )( CX2SocketItem::SocketData::MAX_ANTI_EVADE_RATE * 100 ) ) );
		}
#endif	NOT_USE_PERCENT_IN_OPTION_DATA
		//}} kimhc // 2011-07-05 // 옵션데이타 수치화 작업
	}
	else if ( m_fMoveSpeed > 0 && m_fJumpSpeed > 0 && m_fHyperModeChargeRate > 0 ) // 증강의 마법석
	{
		//{{ kimhc // 2011-07-05 // 옵션데이타 수치화 작업
#ifdef	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13582, "h", GetLinearMoveSpeedValue( iSocketLevel_ ) ) );
		wstrstm << L", ";
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13583, "h", GetLinearJumpSpeedValue( iSocketLevel_ ) ) );
		wstrstm << L", ";
			wstrstm << GET_REPLACED_STRING( ( STR_ID_13577, "h", GetLinearHyperGageChargeSpeedValue( iSocketLevel_ ) ) );
#else	NOT_USE_PERCENT_IN_OPTION_DATA
		if(bCompact)
		{
			wstrstm << GET_REPLACED_STRING( ( STR_ID_5102, "h", ( int )( m_fMoveSpeed * 100 ) ) );
			wstrstm << L", ";
			wstrstm << GET_REPLACED_STRING( ( STR_ID_5103, "h", ( int )( m_fJumpSpeed * 100 ) ) );
			wstrstm << L", ";
			wstrstm << GET_REPLACED_STRING( ( STR_ID_487, "h", ( int )( m_fHyperModeChargeRate * 100 ) ) );
		}
		else
		{
			wstrstm << GET_REPLACED_STRING( ( STR_ID_485, "hi", ( int )( m_fMoveSpeed * 100 ), ( int )( CX2SocketItem::SocketData::MAX_MOVESPEED * 100.f ) ) );
			wstrstm << L", ";
			wstrstm << GET_REPLACED_STRING( ( STR_ID_486, "hi", ( int )( m_fJumpSpeed * 100 ), ( int )( CX2SocketItem::SocketData::MAX_JUMPSPEED * 100.f ) ) );
			wstrstm << L", ";
			wstrstm << GET_REPLACED_STRING( ( STR_ID_487, "h", ( int )( m_fHyperModeChargeRate * 100 ) ) );
		}
#endif	NOT_USE_PERCENT_IN_OPTION_DATA
		//}} kimhc // 2011-07-05 // 옵션데이타 수치화 작업
	}
	else if ( m_fEvadePercent > 0.0f && m_fPercentCritical > 0.0f ) // 적중의 마법석
	{
		//{{ kimhc // 2011-07-05 // 옵션데이타 수치화 작업
#ifdef	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13579, "h", GetLinearEvadeValue( iSocketLevel_ ) ) );
		wstrstm << L", ";
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13574, "h", GetLinearCriticalValue( iSocketLevel_ ) ) );
#else	NOT_USE_PERCENT_IN_OPTION_DATA
		if(bCompact)
		{
			wstrstm << GET_REPLACED_STRING( ( STR_ID_5104, "f", ( m_fEvadePercent * 100 ) ) );
			wstrstm << L", ";
			wstrstm << GET_REPLACED_STRING( ( STR_ID_504, "if", ( int )( m_fPercentCritical * 100 ), m_fCritical + 1.5f ) );
		}
		else
		{
			wstrstm << GET_REPLACED_STRING( ( STR_ID_503, "fi", ( m_fEvadePercent * 100 ), ( int )( CX2SocketItem::SocketData::MAX_EVADE_RATE * 100 ) ) );
			wstrstm << L", ";
			wstrstm << GET_REPLACED_STRING( ( STR_ID_504, "if", ( int )( m_fPercentCritical * 100 ), m_fCritical + 1.5f ) );
		}
#endif	NOT_USE_PERCENT_IN_OPTION_DATA
		//}} kimhc // 2011-07-05 // 옵션데이타 수치화 작업
	}
	else if ( m_fPercentCritical > 0.0f && m_Stat.m_ExtraStat.m_fIncreaseHPRate > 0.0f )	// 파괴의 마법석
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13574, "h", GetLinearCriticalValue( iSocketLevel_ ) ) );		
		wstrstm << L", ";
		wstrstm << GET_REPLACED_STRING( ( STR_ID_5100, "h", ( int )( m_Stat.m_ExtraStat.m_fIncreaseHPRate * GetHpIncrementValue( iSocketLevel_ ) ) ) );	
	}
	else if ( m_fAdditionalAttackValue > 0.0f && m_fAdditionalDefenceValue > 0 )	// 폭발의 마법석
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13575, "h", GetLinearAdditionalAttackValue( iSocketLevel_ ) ) );		
		wstrstm << L", ";
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13580, "h", GetLinearAdditionalDefenceValue( iSocketLevel_ ) ) );
	}
	else if ( m_fEvadePercent > 0.0f && m_fAntiEvadePercent > 0 ) // 연소의 마법석
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13579, "h", GetLinearEvadeValue( iSocketLevel_ ) ) );
		wstrstm << L", ";
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13576, "h", GetLinearAntiEvadeValue( iSocketLevel_ ) ) );
	}
	else if ( m_fAnimationSpeedRate > 0.0f && m_fMoveSpeed > 0.0f && m_fJumpSpeed > 0.0f ) // 신속의 마법석
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13581, "h", GetLinearAnimationSpeedValue( iSocketLevel_ ) ) );
		wstrstm << L", ";
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13582, "h", GetLinearMoveSpeedValue( iSocketLevel_ ) ) );
		wstrstm << L", ";
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13583, "h", GetLinearJumpSpeedValue( iSocketLevel_ ) ) );
	}
	else if ( m_fHyperModeTime > 0.0f && m_fHyperModeChargeRate > 0.0f ) // 격노의 마법석
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13578, "f", m_fHyperModeTime ) );
		wstrstm << L", ";
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13577, "h", GetLinearHyperGageChargeSpeedValue( iSocketLevel_ ) ) );
	}
	else
//}} oasis907 : 김상윤 [2010.4.20] // 
#endif SERV_SOCKET_NEW
#ifdef PVP_SEASON2_SOCKET
	if ( m_Stat.m_fAtkPhysic != 0 )
	{
		wstrstm << GET_STRING( STR_ID_479 ) << L" ";
		if ( m_Stat.m_fAtkPhysic > 0 )
			wstrstm << L"+";
		else
			wstrstm << L"-";

		wstrstm << abs( static_cast<int>( m_Stat.m_fAtkPhysic * iLevelValue ) );
	}
	else if ( m_Stat.m_fAtkMagic != 0 )
	{
		wstrstm << GET_STRING( STR_ID_480 ) << L" ";
		if ( m_Stat.m_fAtkMagic > 0 )
			wstrstm << L"+";
		else
			wstrstm << L"-";

		wstrstm << abs( static_cast<int>( m_Stat.m_fAtkMagic * iLevelValue ) );
	}
	else if ( m_Stat.m_fDefPhysic != 0 )
	{
		wstrstm << GET_STRING( STR_ID_481 ) << L" ";
		if ( m_Stat.m_fDefPhysic > 0 )
			wstrstm << L"+";
		else
			wstrstm << L"-";
		
		 wstrstm << abs( static_cast<int>( m_Stat.m_fDefPhysic * iLevelValue ) );
	}
	else if ( m_Stat.m_fDefMagic != 0 )
	{
		wstrstm << GET_STRING( STR_ID_482 ) << L" ";
		if ( m_Stat.m_fDefMagic > 0 )
			wstrstm << L"+";
		else
			wstrstm << L"-";

		wstrstm << abs( static_cast<int>( m_Stat.m_fDefMagic * iLevelValue ) );
	}
	else if ( m_Stat.m_fBaseHP > 0 )
	{
		wstrstm << GET_STRING( STR_ID_483 ) << L" ";
		if ( m_Stat.m_fBaseHP > 0 )
			wstrstm << L"+";
		else
			wstrstm << L"-";

		wstrstm << (int) abs( static_cast<int>( m_Stat.m_fBaseHP * iLevelValue ) );
	}
#else
	if ( m_Stat.m_fAtkPhysic != 0 )
	{
		wstrstm << GET_STRING( STR_ID_479 ) << L" ";
		if ( m_Stat.m_fAtkPhysic > 0 )
		{
			wstrstm << L"+" << abs( static_cast<int>( m_Stat.m_fAtkPhysic ) );
		}
		else
		{
			wstrstm << L"-" << abs( static_cast<int>( m_Stat.m_fAtkPhysic ) );
		}

	}
	else if ( m_Stat.m_fAtkMagic != 0 )
	{
		wstrstm << GET_STRING( STR_ID_480 ) << L" ";
		if ( m_Stat.m_fAtkMagic > 0 )
		{
			wstrstm << L"+" << abs( static_cast<int>( m_Stat.m_fAtkMagic ) );
		}
		else
		{
			wstrstm << L"-" << abs( static_cast<int>( m_Stat.m_fAtkMagic ) );
		}

	}
	else if ( m_Stat.m_fDefPhysic != 0 )
	{
		wstrstm << GET_STRING( STR_ID_481 ) << L" ";
		if ( m_Stat.m_fDefPhysic > 0 )
		{
			wstrstm << L"+" << abs( static_cast<int>( m_Stat.m_fDefPhysic ) );
		}
		else
		{
			wstrstm << L"-" << abs( static_cast<int>( m_Stat.m_fDefPhysic ) );
		}
	}
	else if ( m_Stat.m_fDefMagic != 0 )
	{
		wstrstm << GET_STRING( STR_ID_482 ) << L" ";
		if ( m_Stat.m_fDefMagic > 0 )
		{
			wstrstm << L"+" << abs( static_cast<int>( m_Stat.m_fDefMagic ) );
		}
		else
		{
			wstrstm << L"-" << abs( static_cast<int>( m_Stat.m_fDefMagic ) );
		}
	}
	else if ( m_Stat.m_fBaseHP > 0 )
	{
		wstrstm << GET_STRING( STR_ID_483 ) << L" ";
		if ( m_Stat.m_fBaseHP > 0 )
		{
			wstrstm << L"+" << (int) abs( static_cast<int>( m_Stat.m_fBaseHP ) );
		}
		else
		{
			wstrstm << L"-" << (int) abs( static_cast<int>( m_Stat.m_fBaseHP ) );
		}
	}
#endif
#ifndef PET_AURA_SKILL
	else if ( m_Stat.m_ExtraStat.m_fIncreaseHPRate != 0 )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_484, "hi", ( int )( m_Stat.m_ExtraStat.m_fIncreaseHPRate * 100 ), ( int )( CX2Stat::Stat::MAX_INCREASE_HP_RATE * 100.f ) ) );
	}
#else
	else if ( m_Stat.m_ExtraStat.m_fIncreaseHPRate != 0 )
	{
		//{{ kimhc // 2011-07-05 // 옵션데이타 수치화 작업
#ifdef	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_5100, "h", ( int )( m_Stat.m_ExtraStat.m_fIncreaseHPRate * GetHpIncrementValue( iSocketLevel_ ) ) ) );
#else	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_484, "ei", ( m_Stat.m_ExtraStat.m_fIncreaseHPRate * 100 ), ( int )( CX2Stat::Stat::MAX_INCREASE_HP_RATE * 100.f ) ) );
#endif	NOT_USE_PERCENT_IN_OPTION_DATA
		//}} kimhc // 2011-07-05 // 옵션데이타 수치화 작업
	}
	else if ( m_Stat.m_ExtraStat.m_fIncreaseMPRate != 0 )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_9064, "e", ( m_Stat.m_ExtraStat.m_fIncreaseMPRate * 100 ) ) );
	}
	else if ( m_Stat.m_ExtraStat.m_fIncreaseAtkPhysicRate != 0 )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_9065, "e", ( m_Stat.m_ExtraStat.m_fIncreaseAtkPhysicRate * 100 ) ) );
	}
	else if ( m_Stat.m_ExtraStat.m_fIncreaseAtkMagicRate != 0 )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_9066, "e", ( m_Stat.m_ExtraStat.m_fIncreaseAtkMagicRate * 100 ) ) );
	}
	else if ( m_Stat.m_ExtraStat.m_fIncreaseDefPhysicRate != 0 )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_9067, "e", ( m_Stat.m_ExtraStat.m_fIncreaseDefPhysicRate * 100 ) ) );
	}
	else if ( m_Stat.m_ExtraStat.m_fIncreaseDefMagicRate != 0 )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_9068, "e", ( m_Stat.m_ExtraStat.m_fIncreaseDefMagicRate * 100 ) ) );
	}
#endif
	else if ( m_fMoveSpeed > 0 )
	{
		//{{ kimhc // 2011-07-05 // 옵션데이타 수치화 작업
#ifdef	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13582, "h", GetLinearMoveSpeedValue( iSocketLevel_ ) ) );
#else	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_485, "hi", ( int )( m_fMoveSpeed * 100 ), ( int )( CX2SocketItem::SocketData::MAX_MOVESPEED * 100.f ) ) );
#endif	NOT_USE_PERCENT_IN_OPTION_DATA
		//}} kimhc // 2011-07-05 // 옵션데이타 수치화 작업
	}
	else if ( m_fJumpSpeed > 0 )
	{
		//{{ kimhc // 2011-07-05 // 옵션데이타 수치화 작업
#ifdef	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13583, "h", GetLinearJumpSpeedValue( iSocketLevel_ ) ) );
#else	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_486, "hi", ( int )( m_fJumpSpeed * 100 ), ( int )( CX2SocketItem::SocketData::MAX_JUMPSPEED * 100.f ) ) );
#endif	NOT_USE_PERCENT_IN_OPTION_DATA
		//}} kimhc // 2011-07-05 // 옵션데이타 수치화 작업
	}
	else if ( m_fHyperModeChargeRate > 0 )
	{
		//{{ kimhc // 2011-07-05 // 옵션데이타 수치화 작업
#ifdef	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13577, "h", GetLinearHyperGageChargeSpeedValue( iSocketLevel_ ) ) );
#else	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_487, "h", ( int )( m_fHyperModeChargeRate * 100 ) ) );
#endif	NOT_USE_PERCENT_IN_OPTION_DATA
		//}} kimhc // 2011-07-05 // 옵션데이타 수치화 작업
	}
	else if ( m_fHyperModeTime > 0 )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13578, "f", m_fHyperModeTime ) );
	}

	//{{ kimhc // 2011-07-21 // 옵션데이타 수치화 작업
#ifdef	NOT_USE_PERCENT_IN_OPTION_DATA
	else if ( 0 < m_fAdditionalAttackValue )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13575, "h", GetLinearAdditionalAttackValue( iSocketLevel_ ) ) );
	}
	else if ( 0 < m_fAdditionalDefenceValue )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13580, "h", GetLinearAdditionalDefenceValue( iSocketLevel_ ) ) );
	}
#endif	NOT_USE_PERCENT_IN_OPTION_DATA
	//}} kimhc // 2011-07-21 // 옵션데이타 수치화 작업

	else if ( m_vecSkillLevelUpID.empty() == false)
	{		
	#ifdef UPGRADE_SKILL_SYSTEM_2013 // 김태환 - 스킬 시스템 변경
		const SkillLevelUpIDAndNum sSkillLevelUpIDAndNum = m_vecSkillLevelUpID.front();
		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( sSkillLevelUpIDAndNum.first );
	#else // UPGRADE_SKILL_SYSTEM_2013
		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( (CX2SkillTree::SKILL_ID)m_vecSkillLevelUpID[0], 1 );
	#endif // UPGRADE_SKILL_SYSTEM_2013

		if( pSkillTemplet != NULL )
		{
#ifdef UPGRADE_SKILL_SYSTEM_2013 // 오현빈
			if ( pSkillTemplet->m_wstrName.rfind( GET_STRING( STR_ID_489 ) ) == -1 )
			{
				wstrstm << pSkillTemplet->m_wstrName.c_str() << L" ";
				wstrstm << GET_REPLACED_STRING ( (STR_ID_25218, "i", sSkillLevelUpIDAndNum.second ) );
			}
			else
			{
				wstrstm << pSkillTemplet->m_wstrName.substr( 0, pSkillTemplet->m_wstrName.rfind( GET_STRING( 489 ) ) ) << L" " ;
				wstrstm << GET_REPLACED_STRING ( (STR_ID_25218, "i", sSkillLevelUpIDAndNum.second ) );
			}
#else
			if ( pSkillTemplet->m_wstrName.rfind( GET_STRING( 489 ) ) == -1 )
				wstrstm << pSkillTemplet->m_wstrName.c_str() << L" " << GET_STRING( 490 );
			else
			{
				wstrstm << pSkillTemplet->m_wstrName.substr( 0, pSkillTemplet->m_wstrName.rfind( GET_STRING( 489 ) ) ) << L" " << GET_STRING( 490 );
			}
#endif //UPGRADE_SKILL_SYSTEM_2013
		}		
	}
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05 // 스킬 레벨
	else if ( false == m_vecSkillLevelUpGroupID.empty() &&
			  NULL != g_pData->GetSocketItem() )
	{
#ifdef UPGRADE_SKILL_SYSTEM_2013 // 오현빈
		vector<SkillLevelUpIDAndNum> vecSkillLevelUPIDAndNumList;
		g_pData->GetSocketItem()->GetSkillIDByUnicClassAndGropID( vecSkillLevelUPIDAndNumList, eUnitClass_, m_vecSkillLevelUpGroupID.front() );

		// 그룹 ID에 해당하는 모든 스킬 리스트 얻기.
		BOOST_FOREACH( const SkillLevelUpIDAndNum& SkillLevelUPIDAndNumList, vecSkillLevelUPIDAndNumList )
		{
			CX2SkillTree::SKILL_ID eSkillID = SkillLevelUPIDAndNumList.first;
			if( CX2SkillTree::SI_NONE != eSkillID )
			{
				const CX2SkillTree::SkillTemplet* pSkillTemplet = 
					g_pData->GetSkillTree()->GetSkillTemplet( eSkillID );

				if( pSkillTemplet != NULL )
				{
					if ( pSkillTemplet->m_wstrName.rfind( GET_STRING( STR_ID_489 ) ) == -1 )
					{
						wstrstm << pSkillTemplet->m_wstrName.c_str() << L" ";
						wstrstm << GET_REPLACED_STRING ( (STR_ID_25218, "i", SkillLevelUPIDAndNumList.second ) );
					}
					else
					{
						wstrstm << pSkillTemplet->m_wstrName.substr( 0, pSkillTemplet->m_wstrName.rfind( GET_STRING( 489 ) ) ) << L" " ;
						wstrstm << GET_REPLACED_STRING ( (STR_ID_25218, "i", SkillLevelUPIDAndNumList.second ) );
					}
				}
			}
			wstrstm << L"\n" ;
		}
#else
		CX2SkillTree::SKILL_ID eSkillID = g_pData->GetSocketItem()->GetSkillIDByUnicClassAndGropID(eUnitClass_, m_vecSkillLevelUpGroupID.front() );
		if( CX2SkillTree::SI_NONE != eSkillID )
		{
			const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID , 1 );

			if( pSkillTemplet != NULL )
			{

				if ( pSkillTemplet->m_wstrName.rfind( GET_STRING( 489 ) ) == -1 )
					wstrstm << pSkillTemplet->m_wstrName.c_str() << L" " << GET_STRING( 490 );
				else
				{
					wstrstm << pSkillTemplet->m_wstrName.substr( 0, pSkillTemplet->m_wstrName.rfind( GET_STRING( 489 ) ) ) << L" " << GET_STRING( 490 );
				}
			}	
		}
#endif UPGRADE_SKILL_SYSTEM_2013 // 오현빈
	}
#endif // SERV_NEW_ITEM_SYSTEM_2013_05

#ifdef BUFF_TEMPLET_SYSTEM
	else if ( m_fCritical > 0.0f )						//크리티컬 데미지 증가
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_16552,		"i", ( int )( m_fCritical * 100) ) );
	}
	else if ( m_fSkillDamageUpRate > 0.0f )				//스페셜 엑티브 스킬 데미지 증가
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_16551,		"i", ( int )( m_fSkillDamageUpRate * 100 ) ) );
	}
	else if ( m_iAddMPValue > 0 )						//최대 MP 증가
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_16555,		"i", m_iAddMPValue ) );
	}
	else if ( m_iMPChangeValue > 0 )					//MP 자연 회복량
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_16554,		"i",  m_iMPChangeValue ) );
	}
	else if ( m_fHPRelativeChangeRate > 0.0f )			//10초당 HP 회복
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_16553,		"i", ( int )( m_fHPRelativeChangeRate * 100 ) ) );
	}
	else if ( m_fAllAttackEnchantRate > 1.0f )			// 속성 발동 확률 % 증가
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_17101,		"f", m_fAllAttackEnchantRate ) );
	}
#endif BUFF_TEMPLET_SYSTEM

	else if ( m_fRepairPriceSale > 0.f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_491, "ii", ( int )( m_fRepairPriceSale * 100.f ), ( int )( CX2SocketItem::SocketData::MAX_REPAIR_PRICE_DISCOUNT * 100.f ) ) );
	}
	else if ( CX2DamageManager::EDT_NONE != m_SocketExtraDamage.m_ExtraDamageType &&
		m_SocketExtraDamage.m_fRate > 0.f  
	//{{ kimhc // 2010.4.9 // 비밀던전 작업(셋트아이템효과)
	#ifdef SERV_SECRET_HELL
		&& m_eTypeSocketItemEffectEx == CX2SocketItem::SIEE_NONE
	#endif SERV_SECRET_HELL
	//}} kimhc // 2010.4.9 // 비밀던전 작업(셋트아이템효과)
		)
	{
		switch( m_SocketExtraDamage.m_ExtraDamageType )
		{
		case CX2DamageManager::EDT_FIRE:
			{
				wstrstm << GET_REPLACED_STRING( ( STR_ID_492, "if", ( int )( m_SocketExtraDamage.m_fRate * 100 ), (float)m_SocketExtraDamage.m_fTime ) );
			} break;
		case CX2DamageManager::EDT_POISON:
			{
				wstrstm << GET_REPLACED_STRING( ( STR_ID_493, "if", ( int )( m_SocketExtraDamage.m_fRate * 100 ), (float)m_SocketExtraDamage.m_fTime ) );
			} break;
		case CX2DamageManager::EDT_CURSE:
			{
				wstrstm << GET_REPLACED_STRING( ( STR_ID_494, "if", ( int )( m_SocketExtraDamage.m_fRate * 100 ), (float)m_SocketExtraDamage.m_fTime ) );
			} break;
		case CX2DamageManager::EDT_SLOW:
			{
				wstrstm << GET_REPLACED_STRING( ( STR_ID_495, "if", ( int )( m_SocketExtraDamage.m_fRate * 100 ), (float)m_SocketExtraDamage.m_fTime ) );
			} break;
		case CX2DamageManager::EDT_MANA_DAMAGE:
			{
				wstrstm << GET_REPLACED_STRING( ( STR_ID_2552, "if", ( int )( m_SocketExtraDamage.m_fRate * 100 ), (float)m_SocketExtraDamage.m_Damage ) );
			} break;

		default:
			{
				//{{ 최민철 [2013/1/4]  게임내 정보 스트링을 엑셀파일로 출력
					#ifndef PRINT_INGAMEINFO_TO_EXCEL
						ASSERT( !"invalid socket extra damage" );
					#endif PRINT_INGAMEINFO_TO_EXCEL
				//}} 최민철 [2013/1/4]  게임내 정보 스트링을 엑셀파일로 출력
			} break;
		}
	}
	else if( false == m_MultipleSocketExtraDamage.empty() )
	{
		for( UINT i=0; i<m_MultipleSocketExtraDamage.size(); i++ )
		{
			const CX2DamageManager::ExtraDamageData& socketExtraDamage = m_MultipleSocketExtraDamage[i];

			ASSERT( socketExtraDamage.m_fRate > 0.f );

			switch( socketExtraDamage.m_ExtraDamageType )
			{
			case CX2DamageManager::EDT_FIRE_SMASH:
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_496, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;
			case CX2DamageManager::EDT_COLD_SMASH:
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_497, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;
			case CX2DamageManager::EDT_POISON_CLOUD:
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_498, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;

			case CX2DamageManager::EDT_EVENT_SMASH:
#ifdef CAMILLA_SMASH
			case CX2DamageManager::EDT_CAMILLA_SMASH:
#endif CAMILLA_SMASH
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_499, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;
#ifdef EDT_EVENT_SMASH_THIRD
			case CX2DamageManager::EDT_EVENT_SMASH_3:
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_9731, "f", ( socketExtraDamage.m_fRate * 100 ) ) );
				} break;
#endif //EDT_EVENT_SMASH_THIRD
#ifdef EDT_EVENT_SMASH_SECOND
			case CX2DamageManager::EDT_EVENT_SMASH_2:
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_9731, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;
#endif EDT_EVENT_SMASH_SECOND
			case CX2DamageManager::EDT_DOUBLE_ATTACK:
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_2553, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;

			case CX2DamageManager::EDT_LIGHTNING_SMASH:
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_5280, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;

				//{{ kimhc // 2010.12.29 // 하멜 마을 추가시 들어가는 ExtraDamage
#ifdef	ADD_HAMEL_VILLAGE
			case CX2DamageManager::EDT_ICE_STING:
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_10235, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;

			case CX2DamageManager::EDT_FIRE_BALL:
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_10236, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;
#endif	ADD_HAMEL_VILLAGE
				//}} kimhc // 2010.12.29 // 하멜 마을 추가시 들어가는 ExtraDamage

#ifdef HAMEL_NEW_SETOPTION_EXTRADAMAGE
			case CX2DamageManager::EDT_WIND_STORM:
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_13421, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;

			case CX2DamageManager::EDT_EARTH_QUAKE:
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_13422, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;
#endif HAMEL_NEW_SETOPTION_EXTRADAMAGE

#ifdef	VELDER_SECRET_DUNGEON
			case CX2DamageManager::EDT_CURSE_CLOUD:
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_13921, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;
#endif	VELDER_SECRET_DUNGEON

#ifdef ADD_SOCKET_FOR_SECRET_SET
			case CX2DamageManager::EDT_FLAME_SMASH:
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_17562, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;

			case CX2DamageManager::EDT_FIRE_WRAITH:
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_17563, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;
#endif

#ifdef STARLIGHT_SMASH
			case CX2DamageManager::EDT_STARLIGHT_SMASH:
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_19148, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;
#endif STARLIGHT_SMASH
#ifdef ADD_SOCKET_OPTION_SANDER_SET
			case CX2DamageManager::EDT_FALLING_ROCK:/// 샌더 셋트 효과 - 바위 소환 
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_23068, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;
			case CX2DamageManager::EDT_SANDSTORM:/// 샌더 셋트 효과 - 모래돌풍 소환
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_23069, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;
			case CX2DamageManager::EDT_WIND_VIBRATION:/// 샌더 칭호 진동 타격
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_23066, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;
			case CX2DamageManager::EDT_BREEZE_ACCEL:/// 샌더 칭호 산들바람의 기운 생성
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_23067, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;
			case CX2DamageManager::EDT_EVIL_SWORD:
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_23109, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;
			case CX2DamageManager::EDT_SPIRIT_SWORD:
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_23110, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;
			case CX2DamageManager::EDT_GHOST_SWORD:
				{
					wstrstm << GET_REPLACED_STRING( ( STR_ID_23111, "i", static_cast<int>( ( socketExtraDamage.m_fRate * 100 ) + 0.5f ) ) );
				} break;
#endif //ADD_SOCKET_OPTION_SANDER_SET
			default:
				{
					ASSERT( !"invalid multiple socket extra damage" );
				} break;
			}
		}
	}
	else if ( m_fDamageReduce > 0.0f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_500, "ii", ( int )( m_fPercentDamageReduce * 100 ), ( int )( m_fDamageReduce * 100 ) ) );
	}
	else if ( m_fDamageReduceByMonsterAttack > 0.0f )
	{
		set< int >::iterator sit;
		sit = m_setMonsterIDDamageReduced.begin();
        wstringstream wstrPart;
		for ( ; sit != m_setMonsterIDDamageReduced.end(); sit++ )
		{
			int npcID = *sit;
			const CX2UnitManager::NPCUnitTemplet* pNpcTemplet = g_pData->GetUnitManager()->GetNPCUnitTemplet( (CX2UnitManager::NPC_UNIT_ID)npcID );
			if ( pNpcTemplet != NULL )
			{
				wstrPart << pNpcTemplet->m_Name.c_str();
				set< int >::iterator sit2 = sit;
				sit2++;
				if ( sit2 != m_setMonsterIDDamageReduced.end() )
				{
					wstrPart << L",";
				}
			}

		}
		wstrstm << GET_REPLACED_STRING( ( STR_ID_501, "iSi", ( int )( m_fPercentDRByMonsterAttack * 100 ), wstrPart.str().c_str(), ( int )( m_fDamageReduceByMonsterAttack * 100 ) ) );
	}

	else if ( m_fAntiEvadePercent > 0.0f )
	{
		//{{ kimhc // 2011-07-05 // 옵션데이타 수치화 작업
#ifdef	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13576, "h", GetLinearAntiEvadeValue( iSocketLevel_ ) ) );
#else	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_502, "fi", ( m_fAntiEvadePercent * 100 ), ( int )( CX2SocketItem::SocketData::MAX_ANTI_EVADE_RATE * 100 ) ) );
#endif	NOT_USE_PERCENT_IN_OPTION_DATA
		//}} kimhc // 2011-07-05 // 옵션데이타 수치화 작업
	}

	else if ( m_fEvadePercent > 0.0f )
	{
		//{{ kimhc // 2011-07-05 // 옵션데이타 수치화 작업
#ifdef	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13579, "h", GetLinearEvadeValue( iSocketLevel_ ) ) );
#else	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_503, "fi", ( m_fEvadePercent * 100 ), ( int )( CX2SocketItem::SocketData::MAX_EVADE_RATE * 100 ) ) );
#endif	NOT_USE_PERCENT_IN_OPTION_DATA
		//}} kimhc // 2011-07-05 // 옵션데이타 수치화 작업
	}
	else if ( m_fPercentCritical > 0.0f )
	{
		//{{ kimhc // 2011-07-05 // 옵션데이타 수치화 작업
#ifdef	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13574, "h", GetLinearCriticalValue( iSocketLevel_ ) ) );
#else	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_504, "if", ( int )( m_fPercentCritical * 100 ), m_fCritical + 1.5f ) );
#endif	NOT_USE_PERCENT_IN_OPTION_DATA
		//}} kimhc // 2011-07-05 // 옵션데이타 수치화 작업	
	}

	//{{ kimhc // 2011-05-25 // SetOption, TitleOption에만 적용 되는 option 모든 itemOption에 적용 되도록 수정
#ifdef	OPTION_ITEM_DATA_MODIFY
	else if ( false == m_vecHpUpByAttacked.empty() )
	{
		D3DXVECTOR2& vRateAndHpUpRatio = m_vecHpUpByAttacked[0];
#ifdef FIX_HP_DOWN_SOCKET_OPTION
		if( vRateAndHpUpRatio.y < 0.f )
			wstrstm << GET_REPLACED_STRING( ( STR_ID_23389, "ff", vRateAndHpUpRatio.x * 100,
			-vRateAndHpUpRatio.y * 100 ) );
		else
#endif FIX_HP_DOWN_SOCKET_OPTION
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13862, "ff", vRateAndHpUpRatio.x * 100,
			vRateAndHpUpRatio.y * 100 ) );
	}
	else if ( false == m_vecHpUpByAttack.empty() )
	{
		D3DXVECTOR2& vRateAndHpUpRatio = m_vecHpUpByAttack[0];
#ifdef FIX_HP_DOWN_SOCKET_OPTION
		if( vRateAndHpUpRatio.y < 0.f )
			wstrstm << GET_REPLACED_STRING( ( STR_ID_23390, "ff", vRateAndHpUpRatio.x * 100,
			-vRateAndHpUpRatio.y * 100 ) );
		else
#endif FIX_HP_DOWN_SOCKET_OPTION
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13863, "ff", vRateAndHpUpRatio.x * 100,
			vRateAndHpUpRatio.y * 100 ) );
	}
	else if ( false == m_vecMpUpByAttacked.empty() )
	{
		D3DXVECTOR2& vRateAndValue = m_vecMpUpByAttacked[0];
		wstrstm << GET_REPLACED_STRING( ( STR_ID_505, "iSi", ( int )( vRateAndValue.x * 100 ),
			( ( vRateAndValue.y >= 0 ) ? L"+" : L"-" ),
			abs( ( int )vRateAndValue.y ) ) );
	}
	else if ( false == m_vecMpUpByAttack.empty() )
	{
		D3DXVECTOR2& vRateAndValue = m_vecMpUpByAttack[0];
		wstrstm << GET_REPLACED_STRING( ( STR_ID_4470, "iSi", ( int )( vRateAndValue.x * 100 ),
			( ( vRateAndValue.y >= 0 ) ? L"+" : L"-" ),
			abs( ( int )vRateAndValue.y ) ) );
	}
#else	OPTION_ITEM_DATA_MODIFY
	else if ( m_fPercentMPUpByAttacked > 0.0f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_505, "iSi", ( int )( m_fPercentMPUpByAttacked * 100 ),
			( ( m_fMPUpByAttacked >= 0 ) ? L"+" : L"-" ),
			abs( ( int )m_fMPUpByAttacked ) ) );
	}
	else if ( m_fPercentMPUpByAttack > 0.0f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_4470, "iSi", ( int )( m_fPercentMPUpByAttack * 100 ),
			( ( m_fMPUpByAttack >= 0 ) ? L"+" : L"-" ),
			abs( ( int )m_fMPUpByAttack ) ) );
	}
#endif	OPTION_ITEM_DATA_MODIFY
	//}} kimhc // 2011-05-25 // SetOption, TitleOption에만 적용 되는 option 모든 itemOption에 적용 되도록 수정

	//{{ 2011.09.05 / 이지헌 / 대만/홍콩 / 마법의 목걸이 개편 / 병합 : 강정훈
#ifdef MAGIC_NEC_FIX
	else if ( m_fMPMultiplyByAll > 0.0f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_14494, "f", m_fMPMultiplyByAll * 100 ) );
	}
#endif
	//}}

	else if ( m_fDamageUpByAMADS > 0.0f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_506, "f", m_fDamageUpByAMADS + 1.f ) );
	}
	else if ( m_ResistExtraDamageType != CX2DamageManager::EDT_NONE )
	{
		if ( m_ResistExtraDamageType == CX2DamageManager::EDT_FIRE )
		{
			wstrstm << GET_STRING( STR_ID_507 );
		}
		else if ( m_ResistExtraDamageType == CX2DamageManager::EDT_LIGHTNING )
		{
			wstrstm << GET_STRING( STR_ID_508 );
		}

		wstrstm << GET_REPLACED_STRING( ( STR_ID_509, "i", ( int )( m_fResistExtraDamage * 100 ) ) );
	}
	else if ( m_AllSkillLevelUp > 0 )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_510, "i", m_AllSkillLevelUp ) );
	}
	else if ( m_fDamageUpPerAtSpecificMonster > 0 )
	{
		set< int >::iterator sit;
		sit = m_setMonsterIDForDamageUp.begin();
        wstringstream wstmPart;
		for ( ; sit != m_setMonsterIDForDamageUp.end(); sit++ )
		{
			int npcID = *sit;
			const CX2UnitManager::NPCUnitTemplet* pNpcTemplet = g_pData->GetUnitManager()->GetNPCUnitTemplet( (CX2UnitManager::NPC_UNIT_ID)npcID );
			if ( pNpcTemplet != NULL )
			{
				wstmPart << pNpcTemplet->m_Name.c_str();
				set< int >::iterator sit2 = sit;
				sit2++;
				if ( sit2 != m_setMonsterIDForDamageUp.end() )
				{
					#ifdef MONSTERID_TITLE_ADD_LINEBREAK //몬스터 이름 하나당 라인 하나
					wstmPart << L",\n";
					#else //MONSTERID_TITLE_ADD_LINEBREAK
					wstmPart << L",";
					#endif //MONSTERID_TITLE_ADD_LINEBREAK
				}
			}

		}
		wstrstm << GET_REPLACED_STRING( ( STR_ID_511, "Si", wstmPart.str().c_str(), ( int )( m_fDamageUpPerAtSpecificMonster * 100 ) ) );
	}
	else if ( m_fEvadePercentByMonsterAttack > 0 )
	{
		if( false == m_DescEvadeByMonsterAttack.empty() )
		{
			wstrstm << m_DescEvadeByMonsterAttack;
		}
		else
		{
			set< int >::iterator sit;
			sit = m_setMonsterIDForEvade.begin();
            wstringstream wstmPart;
			for ( ; sit != m_setMonsterIDForEvade.end(); sit++ )
			{
				int npcID = *sit;
				const CX2UnitManager::NPCUnitTemplet* pNpcTemplet = g_pData->GetUnitManager()->GetNPCUnitTemplet( (CX2UnitManager::NPC_UNIT_ID)npcID );
				if ( pNpcTemplet != NULL )
				{
					wstmPart << pNpcTemplet->m_Name.c_str();
					set< int >::iterator sit2 = sit;
					sit2++;
					if ( sit2 != m_setMonsterIDForEvade.end() )
					{
						wstmPart << L",";
					}
				}
			}
            wstrstm << GET_REPLACED_STRING( ( STR_ID_512, "iS", ( int )( m_fEvadePercentByMonsterAttack * 100 ), wstmPart.str().c_str() ) );
		}
	}
	else if ( m_ImmuneExtraDamageType != CX2DamageManager::EDT_NONE )
	{
		//{{ 2009.01.19 김태완 : 코드정리 elseif -> switch
        wstringstream wstmPart;
		switch(m_ImmuneExtraDamageType)
		{
		case CX2DamageManager::EDT_POISON:
			{
				wstmPart << GET_STRING( STR_ID_514 );
			} break;
		case CX2DamageManager::EDT_SLOW:
			{
				wstmPart << GET_STRING( STR_ID_515 );
			} break;
		case CX2DamageManager::EDT_FIRE:
			{
				wstmPart << GET_STRING( STR_ID_516 );
			} break;
		case CX2DamageManager::EDT_CURSE:
			{
				wstmPart << GET_STRING( STR_ID_517 );
			} break;
		case CX2DamageManager::EDT_SILENCE:
			{
				wstmPart << GET_STRING( STR_ID_518 );
			} break;
		case CX2DamageManager::EDT_LIGHTNING:
			{
				wstmPart << GET_STRING( STR_ID_519 );
			} break;
		case CX2DamageManager::EDT_REVERSE_LEFT_RIGHT:
			{
				wstmPart << GET_STRING( STR_ID_520 );
			} break;
		case CX2DamageManager::EDT_COLD:
			{
				wstmPart << GET_STRING( STR_ID_521 );
			} break;
		case CX2DamageManager::EDT_MANA_DAMAGE:
			{
				wstmPart << GET_STRING( STR_ID_2554 );
			} break;
		case CX2DamageManager::EDT_Y_PRESSED:
			{
				wstmPart << GET_STRING( STR_ID_5282 );
			} break;

		case CX2DamageManager::EDT_FROZEN:
			{
				wstmPart << GET_STRING( STR_ID_11144 );
			} break;

#ifdef	VELDER_SECRET_DUNGEON
		case CX2DamageManager::EDT_PANIC:
			{
				wstmPart << GET_STRING( STR_ID_13922 );
			} break;
#endif	VELDER_SECRET_DUNGEON

		default:
			break;
		}
		//}}

		wstrstm << GET_REPLACED_STRING( ( STR_ID_513, "iS", ( int )( m_fPercentImmuneExtraDamage * 100 ), wstmPart.str().c_str() ) );
	}
	else if ( m_fSpeedUpManaGather > 0.0f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_522, "i", ( int )( m_fSpeedUpManaGather * 100 ) ) );

	}
	else if ( m_fIncreaseEDPercent > 0.0f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_523, "i", ( int )( m_fIncreaseEDPercent * 100 ) ) );

	}
	else if ( m_fIncreaseDCBExpPercent > 0.0f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_524, "i", ( int )( m_fIncreaseDCBExpPercent * 100 ) ) );

	}
	else if( m_fAnimationSpeedRate > 0.0f )
	{
		//{{ kimhc // 2011-07-05 // 옵션데이타 수치화 작업
#ifdef	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_13581, "h", GetLinearAnimationSpeedValue( iSocketLevel_ ) ) );
#else	NOT_USE_PERCENT_IN_OPTION_DATA
		wstrstm << GET_REPLACED_STRING( ( STR_ID_525, "hi", (int)m_fAnimationSpeedRate, (int)CX2SocketItem::SocketData::MAX_ANIMATION_SPEED_RATE ) );
#endif	NOT_USE_PERCENT_IN_OPTION_DATA
		//}} kimhc // 2011-07-05 // 옵션데이타 수치화 작업
	}
	else if ( m_fPercentSuperArmor > 0.0f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_2555, "if", ( int )( m_fPercentSuperArmor * 100 ), m_fTimeSuperArmor ) );
	}

	else if( 0 != m_aiResistEnchant[ CX2EnchantItem::ET_BLAZE ] &&
		0 != m_aiResistEnchant[ CX2EnchantItem::ET_WATER ] &&
		0 != m_aiResistEnchant[ CX2EnchantItem::ET_NATURE ] &&
		0 != m_aiResistEnchant[ CX2EnchantItem::ET_WIND ] &&
		0 != m_aiResistEnchant[ CX2EnchantItem::ET_LIGHT ] &&
		0 != m_aiResistEnchant[ CX2EnchantItem::ET_DARK ] )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_526, "h", m_aiResistEnchant[ CX2EnchantItem::ET_BLAZE ] ) );
	}
	else if( 0 != m_aiResistEnchant[ CX2EnchantItem::ET_BLAZE ] )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_527, "h", m_aiResistEnchant[ CX2EnchantItem::ET_BLAZE ] ) );
	}
	else if( 0 != m_aiResistEnchant[ CX2EnchantItem::ET_WATER ] )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_528, "h", m_aiResistEnchant[ CX2EnchantItem::ET_WATER ] ) );
	}
	else if( 0 != m_aiResistEnchant[ CX2EnchantItem::ET_NATURE ] )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_529, "h", m_aiResistEnchant[ CX2EnchantItem::ET_NATURE ] ) );
	}
	else if( 0 != m_aiResistEnchant[ CX2EnchantItem::ET_WIND ] )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_530, "h", m_aiResistEnchant[ CX2EnchantItem::ET_WIND ] ) );
	}
	else if( 0 != m_aiResistEnchant[ CX2EnchantItem::ET_LIGHT ] )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_531, "h", m_aiResistEnchant[ CX2EnchantItem::ET_LIGHT ] ) );
	}
	else if( 0 != m_aiResistEnchant[ CX2EnchantItem::ET_DARK ] )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_532, "h", m_aiResistEnchant[ CX2EnchantItem::ET_DARK ] ) );
	}
#ifdef PET_AURA_SKILL
	else if( m_afAttackEnchant[ CX2EnchantItem::ET_BLAZE ] > 0.f &&
		m_afAttackEnchant[ CX2EnchantItem::ET_WATER ] > 0.f &&
		m_afAttackEnchant[ CX2EnchantItem::ET_NATURE ] > 0.f &&
		m_afAttackEnchant[ CX2EnchantItem::ET_WIND ] > 0.f &&
		m_afAttackEnchant[ CX2EnchantItem::ET_LIGHT ] > 0.f &&
		m_afAttackEnchant[ CX2EnchantItem::ET_DARK ] > 0.f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_9087, "e", m_afAttackEnchant[ CX2EnchantItem::ET_BLAZE ]*100.f ) );
	}
	else if( m_afAttackEnchant[ CX2EnchantItem::ET_BLAZE ] > 0.f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_9057, "e", m_afAttackEnchant[ CX2EnchantItem::ET_BLAZE ]*100.f ) );
	}
	else if( m_afAttackEnchant[ CX2EnchantItem::ET_WATER ] > 0.f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_9058, "e", m_afAttackEnchant[ CX2EnchantItem::ET_WATER ]*100.f ) );
	}
	else if( m_afAttackEnchant[ CX2EnchantItem::ET_NATURE ] > 0.f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_9059, "e", m_afAttackEnchant[ CX2EnchantItem::ET_NATURE ]*100.f ) );
	}
	else if( m_afAttackEnchant[ CX2EnchantItem::ET_WIND ] > 0.f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_9060, "e", m_afAttackEnchant[ CX2EnchantItem::ET_WIND ]*100.f ) );
	}
	else if( m_afAttackEnchant[ CX2EnchantItem::ET_LIGHT ] > 0.f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_9061, "e", m_afAttackEnchant[ CX2EnchantItem::ET_LIGHT ]*100.f ) );
	}
	else if( m_afAttackEnchant[ CX2EnchantItem::ET_DARK ] > 0.f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_9062, "e", m_afAttackEnchant[ CX2EnchantItem::ET_DARK ]*100.f ) );
	}
#endif
#ifdef SUMMON_NPC_SOCKET
	else if( m_iSummonNpcId > 0 && m_fSummonNpcRate > 0.f )
	{		
		const CX2UnitManager::NPCUnitTemplet* pNpcTemplet = g_pData->GetUnitManager()->GetNPCUnitTemplet( (CX2UnitManager::NPC_UNIT_ID)m_iSummonNpcId );
		if ( pNpcTemplet != NULL )
		{
			wstrstm << GET_REPLACED_STRING( ( STR_ID_17094, "iS", (int)(m_fSummonNpcRate * 100), pNpcTemplet->m_Name.c_str() ) );
		}		
	}
#endif
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
	else if( m_kStatRelLVData.m_fBaseHPRelLV != 0.f )
	{				
		wstrstm << GET_REPLACED_STRING( ( STR_ID_25059, "f", ( m_kStatRelLVData.m_fBaseHPRelLV ) ));
	}
	else if( m_kStatRelLVData.m_fAtkPhysicRelLV != 0.f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_25055, "f", ( m_kStatRelLVData.m_fAtkPhysicRelLV ) ));
	}
	else if( m_kStatRelLVData.m_fAtkMagicRelLV != 0.f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_25056, "f", ( m_kStatRelLVData.m_fAtkMagicRelLV ) ));
	}
	else if( m_kStatRelLVData.m_fDefPhysicRelLV != 0.f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_25057, "f", ( m_kStatRelLVData.m_fDefPhysicRelLV ) ));
	}
	else if( m_kStatRelLVData.m_fDefMagicRelLV != 0.f )
	{
		wstrstm << GET_REPLACED_STRING( ( STR_ID_25058, "f", ( m_kStatRelLVData.m_fDefMagicRelLV ) ));
	}
#endif // SERV_NEW_ITEM_SYSTEM_2013_05
//{{ kimhc // 2010.4.9 // 비밀던전 작업(셋트아이템효과)
#ifdef SERV_SECRET_HELL
	wstrstm	<< GetSocketDescEx();
#endif SERV_SECRET_HELL
//}} kimhc // 2010.4.9 // 비밀던전 작업(셋트아이템효과)

	wstring tempString = wstrstm.str().c_str();
	if ( tempString.empty() == false )
	{
		if ( m_bOverlap == false )
		{
			wstrstm << L" " << GET_STRING( STR_ID_533 );
		}
		if ( m_bDungeonOnly == true )
		{
			wstrstm << L" " << GET_STRING( STR_ID_534 );
		}
		if( m_bPVPOnly == true )
		{
			wstrstm << L" " << GET_STRING( STR_ID_2694 );
		}
	}

	if ( g_pData != NULL && g_pData->GetMyUser() != NULL )
	{
		if ( g_pData->GetMyUser()->GetAuthLevel() == CX2User::XUAL_DEV )
		{
			wstrstm << L" ID = " << m_ID;
		}
	}

	return wstrstm.str().c_str();
}

//{{ kimhc // 2010.4.9 // 비밀던전 작업(셋트아이템효과)
#ifdef SERV_SECRET_HELL
wstring CX2SocketItem::SocketData::GetSocketDescEx()
{
	wstringstream wstrStream;

	switch ( m_eTypeSocketItemEffectEx )
	{
	default:
	case CX2SocketItem::SIEE_NONE:
		wstrStream << L"";
		break;

	case CX2SocketItem::SIEE_STAT_UP:
		{
			wstring wstrStatUpType =L"";

			switch ( m_eStatUpType )
			{
			case SUT_ALL:
				{
					//{{ 2011.9.8 조효진 조합어 대신에 새로 발급한 풀 스트링 사용
#ifdef USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					wstrStatUpType = GET_STRING( STR_ID_14323 );
#else USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					wstrStatUpType = GET_STRING( STR_ID_5003 );
					wstrStatUpType += GET_STRING( STR_ID_5006 );
#endif USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					//}}
				} break;

			case SUT_BOTH_ATT:
				{
					//{{ 2011.9.8 조효진 조합어 대신에 새로 발급한 풀 스트링 사용
#ifdef USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					wstrStatUpType = GET_STRING( STR_ID_14324 );
#else USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					wstrStatUpType = GET_STRING( STR_ID_5003 );
					wstrStatUpType += GET_STRING( STR_ID_5004 );
#endif USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					//}}
				} break;

			case SUT_BOTH_DEF:
				{
					//{{ 2011.9.8 조효진 조합어 대신에 새로 발급한 풀 스트링 사용
#ifdef USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					wstrStatUpType = GET_STRING( STR_ID_14325 );
#else USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					wstrStatUpType = GET_STRING( STR_ID_5003 );
					wstrStatUpType += GET_STRING( STR_ID_5005 );
#endif USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					//}}
				} break;

			case SUT_MAGIC_ATT:
				{
					//{{ 2011.9.8 조효진 조합어 대신에 새로 발급한 풀 스트링 사용
#ifdef USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					wstrStatUpType = GET_STRING( STR_ID_14326 );
#else USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					wstrStatUpType = GET_STRING( STR_ID_5002 );
					wstrStatUpType += GET_STRING( STR_ID_5004 );
#endif USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					//}}
				} break;

			case SUT_PHYSIC_ATT:
				{
					//{{ 2011.9.8 조효진 조합어 대신에 새로 발급한 풀 스트링 사용
#ifdef USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					wstrStatUpType = GET_STRING( STR_ID_14327 );
#else USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					wstrStatUpType = GET_STRING( STR_ID_5001 );
					wstrStatUpType += GET_STRING( STR_ID_5004 );
#endif USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					//}}
				} break;

			case SUT_MAGIC_DEF:
				{
					//{{ 2011.9.8 조효진 조합어 대신에 새로 발급한 풀 스트링 사용
#ifdef USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					wstrStatUpType = GET_STRING( STR_ID_14328 );
#else USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					wstrStatUpType = GET_STRING( STR_ID_5002 );
					wstrStatUpType += GET_STRING( STR_ID_5005 );
#endif USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					//}}
				} break;

			case SUT_PHYSIC_DEF:
				{
					//{{ 2011.9.8 조효진 조합어 대신에 새로 발급한 풀 스트링 사용
#ifdef USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					wstrStatUpType = GET_STRING( STR_ID_14329 );
#else USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					wstrStatUpType = GET_STRING( STR_ID_5001 );
					wstrStatUpType += GET_STRING( STR_ID_5005 );		// 해외팀 수정
#endif USE_FULLWORD_INSTEAD_OF_COMPOUNDWORD
					//}}
				} break;
			}

			wstrStream << GET_REPLACED_STRING( (STR_ID_4996, "iiLi", static_cast<int>( m_fProbability * 100 ), 
				static_cast<int>( m_fDurationTime ), wstrStatUpType, static_cast<int>( m_fIncOrDecVal ) ) );
		}
		break;

	case CX2SocketItem::SIEE_UP_MANA_RECOVERY_VALUE:
		{
			wstrStream << GET_REPLACED_STRING( (STR_ID_4997, "if", 
				static_cast<int>( m_fDurationTime ), m_fIncOrDecVal ) );
		} break;

	case CX2SocketItem::SIEE_CURE_DEBUFF:
		{
			wstrStream << GET_REPLACED_STRING( (STR_ID_4998, "iii", static_cast<int>( m_fProbability * 100 ), m_iCount, 
				static_cast<int>( m_fIntervalTime ) ) );
		} break;

	case CX2SocketItem::SIEE_DEBUFF_ATTACKER:
		{
			wstring wstrExtraDamageType = L"";

			switch ( m_SocketExtraDamage.m_ExtraDamageType )
			{
			case CX2DamageManager::EDT_POISON:
				{
					wstrExtraDamageType = GET_STRING( STR_ID_5007 );
				} break;
			case CX2DamageManager::EDT_SLOW:
				{
					wstrExtraDamageType = GET_STRING( STR_ID_515 );
				} break;

			case CX2DamageManager::EDT_LEG_WOUND:
				{
					wstrExtraDamageType = GET_STRING( STR_ID_5283 );
				} break;

			default:
				ASSERT ( !L"Wrong path!" );
				break;
			}

			wstrStream << GET_REPLACED_STRING( (STR_ID_4999, "iiL", static_cast<int>( m_fProbability * 100 ), 
				static_cast<int>( m_SocketExtraDamage.m_fTime ), wstrExtraDamageType) );
		} break;

	case CX2SocketItem::SIEE_BONE_SHIELD:
		{
			wstrStream << GET_REPLACED_STRING( (STR_ID_5000, "iiiii", static_cast<int>( m_fProbability * 100 ), 
				static_cast<int>( m_fIncOrDecRate * 100 ), m_iCount, static_cast<int>( m_fDurationTime ), static_cast<int>( m_fIntervalTime ) ) );
		} break;

	case SIEE_ATTACK_SPEED_UP:
		{
			wstrStream << GET_REPLACED_STRING( (STR_ID_5084, "iii", static_cast<int>( m_fProbability * 100 ), 
				static_cast<int>( m_fDurationTime ), static_cast<int>( m_fIncOrDecRate * 100 ) ) );

		} break;

	//{{ kimhc // 2010.12.31 // 하멜 마을 추가시 들어가는 ExtraDamage
#ifdef	ADD_HAMEL_VILLAGE
	case SIEE_ICE_NOVA:
		{
			wstrStream << GET_REPLACED_STRING( ( STR_ID_10234, "i", static_cast<int>( m_fProbability * 100 ) ) );
		} break;
#endif	ADD_HAMEL_VILLAGE
	//}} kimhc // 2010.12.31 // 하멜 마을 추가시 들어가는 ExtraDamage

#ifdef ADD_SOCKET_FOR_SECRET_SET
	case SIEE_POISON_BURST:
		{
			wstrStream << GET_REPLACED_STRING( ( STR_ID_17557, "i", static_cast<int>( m_fProbability * 100 ) ) );
		} break;
	case SIEE_ABSORB_HP:
		{
			wstrStream << GET_REPLACED_STRING( ( STR_ID_17558, "i", static_cast<int>( m_fProbability * 100 ) ) );
		} break;
#endif
	}

	return wstrStream.str();
}
#endif SERV_SECRET_HELL
//}} kimhc // 2010.4.9 // 비밀던전 작업(셋트아이템효과)

#ifdef ELSWORD_WAY_OF_SWORD
float CX2SocketItem::GetOptionRateCorrection( const float fOptionRate_ )
{
	// 옵션 점감선
	float fResultOptionRate;
	if( fOptionRate_ <= 0.2f )
		fResultOptionRate = fOptionRate_;
	else if( fOptionRate_ <= 0.25f )
		fResultOptionRate = ( fOptionRate_ * 0.6f ) + 0.08f;
	else if( fOptionRate_ <= 0.3f )
		fResultOptionRate = ( fOptionRate_ * 0.3f ) + 0.155f;
	else
		fResultOptionRate = ( fOptionRate_ * 0.1f ) + 0.215f;
	

	return fResultOptionRate;
}

#endif ELSWORD_WAY_OF_SWORD


#ifdef SERV_CASH_ITEM_SOCKET_OPTION
/** @function : AddSocketGroupDataForCashAvatar_LUA
	@brief : 캐시로 구매한 아바타를 인벤토리로 가져올 때 선택할 수 있는 소켓 옵션들을 모아놓은 그룹 데이터를 파싱하는 함수
*/
void CX2SocketItem::AddSocketGroupDataForCashAvatar_LUA()
{
	KLuaManager luaManager( g_pKTDXApp->GetLuaBinder()->GetLuaState() );
	TableBind( &luaManager, g_pKTDXApp->GetLuaBinder() );

	int	iGroupID = 0;
	LUA_GET_VALUE(		luaManager, L"GROUP_ID",		iGroupID,		0 );

	/// iGroupID가 지정되어 있고, SOCKET_ID_LIST 테이블이 있으면
	if ( 0 != iGroupID && luaManager.BeginTable( "SOCKET_ID_LIST" ) )
	{
		/// iGroupID에 해당 하는 value_type을 insert 한다.
		std::pair<MapSocketGroupDataForCashAvartar::iterator, bool> pairReturnValueForCheckInsert = 
			m_mapSocketGroupDataForCashAvartar.insert( MapSocketGroupDataForCashAvartar::value_type( iGroupID, vector<int>() ) );

		if ( pairReturnValueForCheckInsert.second )	/// iGroup가 기존에 없던 것이면
		{
			MapSocketGroupDataForCashAvartar::iterator mItr
				= pairReturnValueForCheckInsert.first;
			vector<int>& vecSocketID = mItr->second;

			/// vector에 SOCKET_ID_LIST 내의 값들을 넣어준다.
			for ( int iIndex = 1, iSocketID = 0; luaManager.GetValue( iIndex, iSocketID ) == true; iIndex++ )
				vecSocketID.push_back( iSocketID );
		}
		else	/// iGroupID가 중복되었음
		{
			DisplayErrorMessage( L"GROUP_ID is duplicated" );
		}
		
		luaManager.EndTable();
	}
}

/** @function : GetSocketIdListForCashAvatar
	@brief : 캐시로 구매한 아바타를 인벤토리로 가져올 때 소켓옵션을 선택하는 콤보박스에 들어갈 소켓 옵션들을 가져오는 함수
	@param : 아바타에 지정된 그룹아이디(iGroupID_), 그룹아이디에 속해 있는 소켓아이디들(vecSocketGroupData_), 가져오려는 아이템의 타입(iItemType_)
	@return : 얻어가는데 성공했으면 그룹아이디를 리턴, 실패했으면 0을 리턴
*/
int CX2SocketItem::GetSocketIdListForCashAvatar( const int iGroupID_, OUT vector<int>& vecSocketGroupData_, const int iItemType_ )
{
	int iGroupIdToBeReturned = iGroupID_;
	if ( !m_mapSocketGroupDataForCashAvartar.empty() )
	{
		MapSocketGroupDataForCashAvartar::iterator mItr 
			= m_mapSocketGroupDataForCashAvartar.find( iGroupID_ );

		/// 그룹 아이디가 있으면
		if ( m_mapSocketGroupDataForCashAvartar.end() != mItr )
		{
			vecSocketGroupData_ = mItr->second;
		}
		else /// 그룹 아이디가 없는 경우
		{
			int iFixedGroupIdByItemType = 0;
			switch ( iItemType_ )
			{
			case CX2Item::IT_WEAPON:
				{
#ifdef SERV_GLOBAL_BILLING
					iFixedGroupIdByItemType = 2000;
#else //SERV_GLOBAL_BILLING
					iFixedGroupIdByItemType = -2000;
#endif //SERV_GLOBAL_BILLING
				}
				break;

			case CX2Item::IT_DEFENCE:
				{
#ifdef SERV_GLOBAL_BILLING
					iFixedGroupIdByItemType = 1000;
#else //SERV_GLOBAL_BILLING
					iFixedGroupIdByItemType = -1000;
#endif //SERV_GLOBAL_BILLING
				}
				break;

			default:
				ASSERT( L"Invaild path" );
				break;
			}

			/// 아이템 타입에 따른 지정된 그룹아이디가 있는 경우
			if ( 0 != iFixedGroupIdByItemType )
			{
				MapSocketGroupDataForCashAvartar::iterator mItrByItemType 
							= m_mapSocketGroupDataForCashAvartar.find( iFixedGroupIdByItemType );
				if ( m_mapSocketGroupDataForCashAvartar.end() != mItrByItemType )
				{
					vecSocketGroupData_ = mItrByItemType->second;	/// 아이템 타입에 의한 소켓 옵션들
					iGroupIdToBeReturned = iFixedGroupIdByItemType;	/// 리턴될 그룹 ID를 변경
				}
			}
		}
	}

	/// 얻어가는 것이 없으면 0을 리턴, 얻어가는 소켓이 있으면 최종적으로 결정된 그룹 ID를 리턴
	return ( vecSocketGroupData_.empty() ? 0 : iGroupIdToBeReturned );
}
#endif // SERV_CASH_ITEM_SOCKET_OPTION

#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
int CX2SocketItem::GetSocketPrefixStrID( int iSocketID_ ) const
{
	std::map< int, int >::const_iterator it = m_mapSocketDescStrID.find( iSocketID_ ) ;
	if( m_mapSocketDescStrID.end() == it )
		return -1;

	return it->second;
}
void CX2SocketItem::GetSocketPrefix( IN int iSocketID_, OUT wstring& wstrItemDesc_ ) const
{
	int iStrID = GetSocketPrefixStrID( iSocketID_ );
	if( -1 == iStrID )
		return;

	wstrItemDesc_ += GET_STRING( iStrID );;
	wstrItemDesc_ += L" ";
}
#ifdef EU_NEW_ITEM_SYSTEM_PREFIX_TO_POSTFIX
void CX2SocketItem::GetSocketPostfix( IN int iSocketID_, OUT wstring& wstrItemDesc_ ) const
{
	int iStrID = GetSocketPrefixStrID( iSocketID_ );
	if( -1 == iStrID )
		return;
	wstrItemDesc_ += L" ";
	wstrItemDesc_ += GET_STRING( iStrID );;
}
#endif // EU_NEW_ITEM_SYSTEM_PREFIX_TO_POSTFIX
/** @function : AddSkillLevelUpSocketGroup_LUA
	@brief : 스킬 레벨업 소켓 그룹 등록
*/
void CX2SocketItem::AddSkillLevelUpSocketGroup_LUA( int iGroupID_, 	CX2Unit::UNIT_CLASS eUnitClass_, int iSkillID_
#ifdef UPGRADE_SKILL_SYSTEM_2013// 오현빈
	, int iSkillLvUP_ 
#endif // UPGRADE_SKILL_SYSTEM_2013
	)
{
	if( iGroupID_ <= 0 )
	{
		ASSERT(!"SkillLevelUpSocketGroup.lua Group ID Error");
		return;
	}
	CX2SkillTree::SKILL_ID eSkillID = static_cast<CX2SkillTree::SKILL_ID>(iSkillID_);
	if( eSkillID <= CX2SkillTree::SI_NONE )
	{
		ASSERT(!"SkillLevelUpSocketGroup.lua Skill ID Error");
		return;
	}

	if( eUnitClass_ <= CX2Unit::UC_NONE || eUnitClass_ >= CX2Unit::UC_END)
	{
		ASSERT(!"SkillLevelUpSocketGroup.lua UNIT_CLASS Error");
		return;
	}

#ifdef UPGRADE_SKILL_SYSTEM_2013 // 오현빈
	if( 0 == iSkillLvUP_ )
	{
		iSkillLvUP_ = 1;
	}

	// 그룹ID별 기준으로 레벨업 시킬 스킬 ID와 포인트 설정
	mapUnitClassSkillGroup::iterator it = m_mapSkillUpSocketGroup.find( iGroupID_ );
	if( it == m_mapSkillUpSocketGroup.end() )
	{
		std::vector< SkillLvUpSocketOption > vecSkillGroupTemp;
		vecSkillGroupTemp.push_back( SkillLvUpSocketOption( eUnitClass_, eSkillID, iSkillLvUP_ ) );

		m_mapSkillUpSocketGroup.insert( std::make_pair( iGroupID_, vecSkillGroupTemp ) );
	}
	else
	{
		it->second.push_back( SkillLvUpSocketOption(eUnitClass_, eSkillID, iSkillLvUP_ ) ) ;
	}
#else
	mapUnitClassSkillGroup::iterator it = m_mapSkillUpSocketGroup.find( eUnitClass_ );
	if( it == m_mapSkillUpSocketGroup.end() )
	{
		std::vector< pairSkillGroup > vecSkillGroupTemp;
		vecSkillGroupTemp.push_back( std::make_pair(iGroupID_, eSkillID) );

		m_mapSkillUpSocketGroup.insert( std::make_pair( eUnitClass_, vecSkillGroupTemp ) );
	}
	else
	{
		it->second.push_back( std::make_pair(iGroupID_, eSkillID) ) ;
	}
#endif // UPGRADE_SKILL_SYSTEM_2013
}
/** @function : GetSkillIDByUnicClassAndGropIDZ
	@brief : 해당 전직과 스킬 그룹에 해당하는 스킬 ID 얻기
*/
#ifdef UPGRADE_SKILL_SYSTEM_2013 // 오현빈 
bool CX2SocketItem::GetSkillIDByUnicClassAndGropID( OUT vector<SkillLevelUpIDAndNum>& vecSkillLevelUPIDAndNumList_, IN CX2Unit::UNIT_CLASS eUnitClass_, IN int iGroupID_ ) const
#else
CX2SkillTree::SKILL_ID CX2SocketItem::GetSkillIDByUnicClassAndGropID( CX2Unit::UNIT_CLASS eUnitClass_, int iGroupID_ ) const
#endif //UPGRADE_SKILL_SYSTEM_2013
{
	if( CX2Unit::UC_NONE == eUnitClass_ )
	{
		eUnitClass_ = g_pData->GetSelectUnitClass();
	}

#ifdef UPGRADE_SKILL_SYSTEM_2013 // 오현빈 
	// OUT 인자 초기화 
	vecSkillLevelUPIDAndNumList_.clear();

	// 그룹ID 검색
	mapUnitClassSkillGroup::const_iterator it = m_mapSkillUpSocketGroup.find( iGroupID_ );
	
	// 등록되지 않은 그룹ID라면, false return;
	if( it == m_mapSkillUpSocketGroup.end() )
	{
		return false;
	}
	else
	{
		// 등록된 그룹ID라면, 전직에 맞는 스킬 ID 리스트 전달
		SkillLevelUpIDAndNum sSkillLevelUpIDAndNum;

		BOOST_FOREACH( const SkillLvUpSocketOption& sSkillLvUpSocketOption, it->second )
		{
			// 자기 전직에 맞는 스킬만 등록
			if( eUnitClass_ == sSkillLvUpSocketOption.m_eUnitClass )
			{
				sSkillLevelUpIDAndNum.first = sSkillLvUpSocketOption.m_eSkillID;
				sSkillLevelUpIDAndNum.second = sSkillLvUpSocketOption.m_iSkillLvUPNum;
				vecSkillLevelUPIDAndNumList_.push_back( sSkillLevelUpIDAndNum );
			}
		}

		return true;
	}

	return false;
#else
	mapUnitClassSkillGroup::const_iterator it = m_mapSkillUpSocketGroup.find( eUnitClass_ );
	if( it == m_mapSkillUpSocketGroup.end() )
	{
		return CX2SkillTree::SI_NONE;
	}
	else
	{
		BOOST_FOREACH( pairSkillGroup SkillGroup, it->second )
		{
			if( iGroupID_ == SkillGroup.first )
				return SkillGroup.second;
		}
	}

	return CX2SkillTree::SI_NONE;
#endif //UPGRADE_SKILL_SYSTEM_2013
}

const CX2SocketItem::KItemStatRelLVData& CX2SocketItem::GetStatRelLVDataBySocktID( int iSocketID_ ) const
{
	std::map< int, SocketData* >::const_iterator it = m_mapSocketOptionPool.find( iSocketID_ ) ;
	if( m_mapSocketOptionPool.end() == it )
	{
		ASSERT(!" SocketID Error");
		return ms_KDummyItemStatRelLVData;
	}

	if( NULL != it->second )
		return it->second->m_kStatRelLVData;

	return ms_KDummyItemStatRelLVData;
}

#ifdef UPGRADE_SKILL_SYSTEM_2013 // 김태환

/** @function	: GetSkillLevelIncreaseValue
	@brief		: 스킬 레벨이 증가되는 소켓에 대한 증가치 반환 함수
	@param		: 검사할 스킬 아이디
	@return		: 증가치
*/
void CX2SocketItem::GetSkillLevelIncreaseValue( IN const SocketData* pSocketData, IN const CX2SkillTree::SKILL_ID eSkillID, OUT int& iIncreaseSkillLevelBySocket, IN CX2Unit::UNIT_CLASS eUnitClass_ ) const
{
	if( NULL == pSocketData ||
		NULL == g_pData->GetMyUser() ||
		NULL == g_pData->GetMyUser()->GetSelectUnit() ||
		NULL == g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() ||
		NULL == g_pData->GetSkillTree() )
		return;

	/// 모든 스킬 레벨을 올려주는 소켓 효과
	if( 0.f < pSocketData->m_AllSkillLevelUp )
		iIncreaseSkillLevelBySocket += pSocketData->m_AllSkillLevelUp;

	/// 하나의 스킬 레벨을 올려주는 소켓 효과
	BOOST_FOREACH( SkillLevelUpIDAndNum sSkillLevelUpIDAndNum, pSocketData->m_vecSkillLevelUpID)
	{
		if ( eSkillID == sSkillLevelUpIDAndNum.first )
		{
			iIncreaseSkillLevelBySocket += sSkillLevelUpIDAndNum.second;
		}
	}

	/// 그룹에 포함된 스킬 레벨을 올려주는 소켓 효과
	BOOST_FOREACH( int iVal, pSocketData->m_vecSkillLevelUpGroupID )
	{
		vector<SkillLevelUpIDAndNum> vecSkillLevelUPIDAndNumList;
		g_pData->GetSocketItem()->GetSkillIDByUnicClassAndGropID( vecSkillLevelUPIDAndNumList, eUnitClass_, iVal );

		// 그룹 ID에 해당하는 모든 스킬 리스트 얻기.
		BOOST_FOREACH( const SkillLevelUpIDAndNum& SkillLevelUPIDAndNumList, vecSkillLevelUPIDAndNumList )
		{
			if( eSkillID == SkillLevelUPIDAndNumList.first )
			{
				iIncreaseSkillLevelBySocket += SkillLevelUPIDAndNumList.second;
			}
		}
	}

#ifdef SKILL_LEVEL_UP_BY_POWER_RATE_TYPE
	// SKILL_POWER_RATE_TYPE에 따라 다른 레벨 증가 수치 적용
	if( 0 < pSocketData->m_iAllSkillLevelUpEx &&
		NULL != g_pData->GetSkillTree() )
	{
		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet(eSkillID);
		if( NULL != pSkillTemplet )
		{
			const UINT uiLevelUpNumByPowerRateType = g_pData->GetSkillTree()->GetSkillLevelUpNumByPowerRateType( pSkillTemplet->m_eSkillPowerRateType );
			
			iIncreaseSkillLevelBySocket += static_cast<int>(pSocketData->m_iAllSkillLevelUpEx * uiLevelUpNumByPowerRateType);
		}
	}
#endif // SKILL_LEVEL_UP_BY_POWER_RATE_TYPE

}
#endif UPGRADE_SKILL_SYSTEM_2013 // 김태환

#endif // SERV_NEW_ITEM_SYSTEM_2013_05

#ifdef SERV_GROW_UP_SOCKET
bool CX2SocketItem::SetGrowUpSocketData( IN const wstring& wstrGrowUpSocketID_, OUT map<int, vector<int>>& mapGrowUpSocketID )
{
	// string value 에서 단계까지 구분 하도록 하자
	if ( wstrGrowUpSocketID_.empty() == true )
	{
		ErrorLog( KEM_ERROR359 );
		return false;
	}
	else
	{
		vector<int> vecSocketID;

		if ( wstrGrowUpSocketID_.compare( L"Group 0" ) != 0 )
		{
			int iLevel = 0;

			int leftIndex = wstrGrowUpSocketID_.find( L"(" );
			wstring tempGroup;
			while( wstrGrowUpSocketID_.find( L"(", leftIndex + 1) != -1 )
			{
				int rightIndex = wstrGrowUpSocketID_.find( L")", leftIndex + 1 );
				tempGroup = wstrGrowUpSocketID_.substr( leftIndex + 1, rightIndex - leftIndex - 1 );

				if ( tempGroup.compare( L"Group 0" ) != 0 )
				{
					int stringIndex = tempGroup.find( L" " );
					if ( stringIndex != -1 )
					{
						wstring tempSocketID;
						while( tempGroup.find( L" ", stringIndex + 1 ) != -1 )
						{
							int debugIndex = tempGroup.find( L" ", stringIndex + 1 );
							tempSocketID = tempGroup.substr( stringIndex + 1, debugIndex - stringIndex - 1 );
							if ( tempSocketID.compare( L"0" ) != 0 )
								vecSocketID.push_back(_wtoi( tempSocketID.c_str()));
							stringIndex = debugIndex;
						}

						tempSocketID = tempGroup.substr( stringIndex + 1 );
						if ( tempSocketID.compare( L"0" ) != 0 )
							vecSocketID.push_back(_wtoi( tempSocketID.c_str()));
					}
					iLevel++;
					mapGrowUpSocketID.insert(std::make_pair(iLevel, vecSocketID));
					vecSocketID.clear();
				}
				else
				{
					vecSocketID.clear();
					iLevel++;
					mapGrowUpSocketID.insert( std::make_pair( iLevel, vecSocketID ) );
				}
				leftIndex = rightIndex;
			}
		}
	}

	return true;
}

bool CX2SocketItem::GetGrowUpSocketData( IN const vector<int>& vecOldSocketOption_, OUT vector<int>& vecNewSocketOption_ )
{
	// Note. 소켓 정보는 본인만 필요하므로, MyUser 를 내부에서 호출해도 상관없다.
	//vector<int> vecTempSocketOption = vecSocketOption_;
	vecNewSocketOption_.clear();
	for( UINT j=0; j<vecOldSocketOption_.size(); j++ )
	{
		int socketOptionID = vecOldSocketOption_[j];
		CX2SocketItem::SocketData* pSocketData = g_pData->GetSocketItem()->GetSocketData( socketOptionID );

		if( NULL == pSocketData )
			continue;

		switch( pSocketData->m_Type )
		{
		case CX2SocketItem::SDT_PVP_TAG_POINT:
		case CX2SocketItem::SDT_QUEST_CLEAR_COUNT:
		case CX2SocketItem::SDT_ITEM_EXCHANGE_COUNT:
			{
				// 성장 소켓이다
				// 성장 단계에 해당하는 소켓들을 집어 넣자.
				int iLevel = 1;
				if( NULL != g_pData->GetMyUser() && NULL != g_pData->GetMyUser()->GetSelectUnit() && NULL != g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() )
				{
					iLevel = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->GetGrowUpLevelBySocket(pSocketData->m_Type);
				}
				map<int, vector<int>>::iterator mit = pSocketData->m_mapGrowUpSocketID.find(iLevel);
				if( mit != pSocketData->m_mapGrowUpSocketID.end())
				{
					BOOST_TEST_FOREACH( const int&, iSocketID, mit->second )
					{
						vecNewSocketOption_.push_back( iSocketID );
					}
				}
				else
				{
					// 해당 레벨의 성장 소켓 정보가 없다
					continue;
				}
			}
			break;
		default:
			// 성장 소켓이 아니다
			vecNewSocketOption_.push_back( socketOptionID );
			break;
		}
	}
	return false;
}
#endif //SERV_GROW_UP_SOCKET