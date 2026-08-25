#include ".\xslunitmanager.h"
#include ".\XSLDamageManager.h"
#include ".\XSLDungeon.h"

ImplementSingleton( CXSLUnitManager );

CXSLUnitManager::CXSLUnitManager(void)
{
	//lua_tinker::def( "IncludeLua", IncludeLua_ );

	//lua_tinker::class_<CXSLUnitManager>("CXSLUnitManager")
	//	.def("AddUnitTemplet",			CXSLUnitManager::AddUnitTemplet_LUA)
	//	.def("AddNPCTemplet",			CXSLUnitManager::AddNPCTemplet_LUA)
	//	.def("AddNPCStat",				CXSLUnitManager::AddNPCStat_LUA)
	//	;

	//lua_tinker::decl( "g_pUnitManager", this );

	lua_tinker::class_add<CXSLUnitManager>( g_pLua, "CXSLUnitManager" );

	lua_tinker::class_def<CXSLUnitManager>( g_pLua, "AddUnitTemplet",	&CXSLUnitManager::AddUnitTemplet_LUA );
	lua_tinker::class_def<CXSLUnitManager>( g_pLua, "AddNPCTemplet",	&CXSLUnitManager::AddNPCTemplet_LUA );
	lua_tinker::class_def<CXSLUnitManager>( g_pLua, "AddNPCStat",		&CXSLUnitManager::AddNPCStat_LUA );

	lua_tinker::decl( g_pLua, "g_pUnitManager", this );
}

CXSLUnitManager::~CXSLUnitManager(void)
{
	std::map<CXSLUnit::UNIT_CLASS,CXSLUnit::UnitTemplet*>::iterator mit;

	CXSLUnit::UnitTemplet* pUnitTemplet;
	for( mit = m_UnitTempletMap.begin(); mit != m_UnitTempletMap.end(); mit++ )
	{
		pUnitTemplet = mit->second;
		SAFE_DELETE( pUnitTemplet );
	}
	m_UnitTempletMap.clear();

	std::map<NPC_UNIT_ID,NPCUnitTemplet*>::iterator mit2;
	NPCUnitTemplet* pNPCUnitTemplet;
	for( mit2 = m_NPCUnitTemplet.begin(); mit2 != m_NPCUnitTemplet.end(); mit2++ )
	{
		pNPCUnitTemplet = mit2->second;
		SAFE_DELETE( pNPCUnitTemplet );
	}
	m_NPCUnitTemplet.clear();

	std::map<NPC_UNIT_ID, NPCUnitStat*>::iterator mit3;
	NPCUnitStat* pStat;
	for( mit3 = m_NPCUnitStat.begin(); mit3 != m_NPCUnitStat.end(); mit3++ )
	{
		pStat = mit3->second;
		SAFE_DELETE( pStat );
	}
	m_NPCUnitStat.clear();
}

ImplToStringW( CXSLUnitManager )
{
	return stm_;
}

bool CXSLUnitManager::OpenScriptFile( const char* pFileName )
{
	_JIF( 0 == LUA_DOFILE( g_pLua, pFileName ), return false );

	return true;
}

bool CXSLUnitManager::OpenNPCScriptFile( const char* pFileName )
{
	return OpenScriptFile( pFileName );
}

bool CXSLUnitManager::OpenNPCStatScriptFile( const char* pFileName )
{
	return OpenScriptFile( pFileName );
}

const CXSLUnit::UnitTemplet* CXSLUnitManager::GetUnitTemplet( CXSLUnit::UNIT_CLASS unitClass )
{
	std::map<CXSLUnit::UNIT_CLASS,CXSLUnit::UnitTemplet*>::const_iterator iter;
	iter = m_UnitTempletMap.find( unitClass );
	if( iter == m_UnitTempletMap.end() )
		return NULL;

	return iter->second;
}

const CXSLUnitManager::NPCUnitTemplet* CXSLUnitManager::GetNPCUnitTemplet( NPC_UNIT_ID nNPCUnitID )
{
	std::map<NPC_UNIT_ID, NPCUnitTemplet*>::const_iterator mit;
	mit = m_NPCUnitTemplet.find( nNPCUnitID );
	if( mit == m_NPCUnitTemplet.end() )
		return NULL;

	return mit->second;
}

const CXSLUnitManager::NPCUnitStat* CXSLUnitManager::GetNPCUnitStat( NPC_UNIT_ID nNPCUnitID )
{
	std::map<NPC_UNIT_ID, NPCUnitStat*>::const_iterator mit;
	mit = m_NPCUnitStat.find( nNPCUnitID );
	if( mit == m_NPCUnitStat.end() )
		return NULL;

	return mit->second;
}

bool CXSLUnitManager::AddUnitTemplet_LUA()
{
	CXSLUnit::UnitTemplet* pUnitTemplet = new CXSLUnit::UnitTemplet();
	KLuaManager luaManager( g_pLua );
	TableBind( &luaManager );

	LUA_GET_VALUE_RETURN_ENUM( luaManager, "unitType",		pUnitTemplet->m_UnitType,	CXSLUnit::UNIT_TYPE,	CXSLUnit::UT_NONE,	SAFE_DELETE(pUnitTemplet); return false; );
	LUA_GET_VALUE_RETURN_ENUM( luaManager, "unitClass",		pUnitTemplet->m_UnitClass,	CXSLUnit::UNIT_CLASS,	CXSLUnit::UC_NONE,	SAFE_DELETE(pUnitTemplet); return false; );

	//LUA_GET_VALUE_RETURN(	luaManager, L"name",			pUnitTemplet->m_Name,			L"", SAFE_DELETE(pUnitTemplet); return false; );
	//LUA_GET_VALUE_RETURN(	luaManager, L"description",		pUnitTemplet->m_Description,	L"", SAFE_DELETE(pUnitTemplet); return false; );

	// removed by wonpok, 20100525.
	// 모션분리 작업으로 더이상 사용하지 않는 항목이다. 
	//LUA_GET_VALUE_RETURN(	luaManager, L"m_MotionFile",	pUnitTemplet->m_MotionFile,			L"", SAFE_DELETE(pUnitTemplet); return false; );
	
	LUA_GET_VALUE_RETURN( luaManager, "basicWeaponItemID",	pUnitTemplet->m_BasicWeaponItemID,		0, SAFE_DELETE(pUnitTemplet); return false; );
	LUA_GET_VALUE_RETURN( luaManager, "basicHairItemID",	pUnitTemplet->m_BasicHairItemID,		0, SAFE_DELETE(pUnitTemplet); return false; );
	LUA_GET_VALUE_RETURN( luaManager, "basicFaceItemID",	pUnitTemplet->m_BasicFaceItemID,		0, SAFE_DELETE(pUnitTemplet); return false; );
	LUA_GET_VALUE_RETURN( luaManager, "basicBodyItemID",	pUnitTemplet->m_BasicBodyItemID,		0, SAFE_DELETE(pUnitTemplet); return false; );
	LUA_GET_VALUE_RETURN( luaManager, "basicLegItemID",		pUnitTemplet->m_BasicLegItemID,			0, SAFE_DELETE(pUnitTemplet); return false; );
	LUA_GET_VALUE_RETURN( luaManager, "basicHandItemID",	pUnitTemplet->m_BasicHandItemID,		0, SAFE_DELETE(pUnitTemplet); return false; );
	LUA_GET_VALUE_RETURN( luaManager, "basicFootItemID",	pUnitTemplet->m_BasicFootItemID,		0, SAFE_DELETE(pUnitTemplet); return false; );

	LUA_GET_VALUE( luaManager, "m_RavenLeftArmItemID",	pUnitTemplet->m_RavenLeftArmItemID,		0 );
	LUA_GET_VALUE( luaManager, "m_SecondWeaponItemID",	pUnitTemplet->m_SecondWeaponItemID,		0 );


	if( m_UnitTempletMap.find(pUnitTemplet->m_UnitClass) != m_UnitTempletMap.end() )
	{
		SAFE_DELETE(pUnitTemplet); 
		return false;
	}
	else
	{
		m_UnitTempletMap.insert( std::make_pair(pUnitTemplet->m_UnitClass, pUnitTemplet) );
	}

	return true;
}

bool CXSLUnitManager::AddNPCTemplet_LUA()
{
	NPCUnitTemplet* pNPCUnitTemplet = new NPCUnitTemplet();
	KLuaManager luaManager( g_pLua );
	TableBind( &luaManager );

	LUA_GET_VALUE_RETURN_ENUM(	luaManager, L"NPC_ID",			pNPCUnitTemplet->m_nNPCUnitID,		NPC_UNIT_ID,	NUI_NONE,	SAFE_DELETE(pNPCUnitTemplet); return false; );
	LUA_GET_VALUE_RETURN_ENUM(	luaManager, L"CLASS_TYPE",		pNPCUnitTemplet->m_ClassType,			NPC_CLASS_TYPE,	NCT_BASIC,	SAFE_DELETE(pNPCUnitTemplet); return false; );

	//LUA_GET_VALUE_RETURN(	luaManager, L"NAME",				pNPCUnitTemplet->m_Name,				L"", SAFE_DELETE(pNPCUnitTemplet); return false; );
	//LUA_GET_VALUE_RETURN(	luaManager, L"DESC",				pNPCUnitTemplet->m_Description,		L"", SAFE_DELETE(pNPCUnitTemplet); return false; );
	LUA_GET_VALUE(			luaManager, L"LUA_FILE_NAME",		pNPCUnitTemplet->m_LuaFileName,		L"" );
	LUA_GET_VALUE(			luaManager, L"LUA_FILE_NAME_UI",	pNPCUnitTemplet->m_LuaFileNameUI ,	L"" );
	LUA_GET_VALUE_RETURN(	luaManager, L"IS_NPC",				pNPCUnitTemplet->m_bIsNPC,			false, SAFE_DELETE(pNPCUnitTemplet); return false; );

	LUA_GET_VALUE(	luaManager, L"NPC_FACE_TEXTURE",	pNPCUnitTemplet->m_NPCFaceTexture,			L"" );
	LUA_GET_VALUE(	luaManager, L"NPC_FACE_TEXTURE_KEY", pNPCUnitTemplet->m_NPCFaceTextureKey,		L"" );


	if( m_NPCUnitTemplet.find(pNPCUnitTemplet->m_nNPCUnitID) != m_NPCUnitTemplet.end() )
	{
		SAFE_DELETE(pNPCUnitTemplet); 
		return false;
	}
	else
	{
		m_NPCUnitTemplet.insert( std::make_pair(pNPCUnitTemplet->m_nNPCUnitID, pNPCUnitTemplet) );
	}

	return true;
}

bool CXSLUnitManager::AddNPCStat_LUA()
{
	KLuaManager luaManager( g_pLua );
	TableBind( &luaManager );

	NPCUnitStat* pStat = new NPCUnitStat;

	LUA_GET_VALUE_ENUM(	luaManager, L"NPC_ID",			pStat->m_nNPCUnitID,	CXSLUnitManager::NPC_UNIT_ID,	CXSLUnitManager::NUI_NONE );
	LUA_GET_VALUE(		luaManager, L"MAX_HP",			pStat->m_fMaxHP,					0.0f );
	LUA_GET_VALUE(		luaManager, L"ATK_PHYSIC",		pStat->m_fAtkPhysic,				0.0f );
	LUA_GET_VALUE(		luaManager, L"ATK_MAGIC",		pStat->m_fAtkMagic,					0.0f );
	LUA_GET_VALUE(		luaManager, L"DEF_PHYSIC",		pStat->m_fDefPhysic,				0.0f );
	LUA_GET_VALUE(		luaManager, L"DEF_MAGIC",		pStat->m_fDefMagic,					0.0f );


	if( S_OK == luaManager.BeginTable( L"HARD_LEVEL" ) )
	{
		LUA_GET_VALUE(		luaManager, L"HP",				pStat->m_HardLevel.m_fHP,			0.0f );
		LUA_GET_VALUE(		luaManager, L"ATK_PHYSIC",		pStat->m_HardLevel.m_fAtkPhysic,	0.0f );
		LUA_GET_VALUE(		luaManager, L"ATK_MAGIC",		pStat->m_HardLevel.m_fAtkMagic,	0.0f );
		LUA_GET_VALUE(		luaManager, L"DEF_PHYSIC",		pStat->m_HardLevel.m_fDefPhysic,	0.0f );
		LUA_GET_VALUE(		luaManager, L"DEF_MAGIC",		pStat->m_HardLevel.m_fDefMagic,	0.0f );
		LUA_GET_VALUE(		luaManager, L"EXP",				pStat->m_HardLevel.m_fExp,		0.0f );

		luaManager.EndTable();
	}

	LUA_GET_VALUE_ENUM(	luaManager, L"DAMAGE_TYPE",			pStat->m_DamageType,	CXSLDamageManager::DAMAGE_TYPE,	CXSLDamageManager::DT_PHYSIC );

	if( m_NPCUnitStat.find( pStat->m_nNPCUnitID ) != m_NPCUnitStat.end() )
	{
		SAFE_DELETE( pStat );
		return false;
	}
	else
	{
		m_NPCUnitStat[ pStat->m_nNPCUnitID ] = pStat;
	}

	return true;
}

//void CXSLUnitManager::GetNPCUnitList( std::map< CXSLUnitManager::NPC_UNIT_ID, std::wstring >& mapNpcList )
//{
//	mapNpcList.clear();
//	std::map< NPC_UNIT_ID, NPCUnitTemplet* >::iterator mit = m_NPCUnitTemplet.begin();
//	for( ; mit != m_NPCUnitTemplet.end(); mit++ )
//	{
//		NPCUnitTemplet* npcTemp = mit->second;
//		mapNpcList.insert( std::make_pair( npcTemp->m_nNPCUnitID, npcTemp->m_Description ) );
//	}
//}

//{{ 2010. 07. 12  최육사	경험치 공식 변경
CXSLUnitManager::MONSTER_TYPE_FACTOR CXSLUnitManager::GetMonsterTypeFactor( IN bool bIsAttribNpc, IN char cDifficultyLevel, IN char cMonsterGrade )
{
	if( bIsAttribNpc )
	{
		switch( cDifficultyLevel )
		{
		case CXSLDungeon::DL_NORMAL:
		case CXSLDungeon::DL_HARD:
			return MONSTER_TYPE_FACTOR::MTF_LOW_ELITE_NPC;

		case CXSLDungeon::DL_EXPERT:
			return MONSTER_TYPE_FACTOR::MTF_HIGH_ELITE_NPC;

		default:
			START_LOG( cerr, L"던전 난이도가 이상합니다." )
				<< BUILD_LOGc( cDifficultyLevel )
				<< END_LOG;

			return MONSTER_TYPE_FACTOR::MTF_NORMAL_NPC;
		}
	}
	else if( cMonsterGrade == CXSLUnitManager::MG_MIDDLE_BOSS_NPC )
	{
		return CXSLUnitManager::MTF_MIDDLE_BOSS_NPC;
	}
	else if( cMonsterGrade == CXSLUnitManager::MG_BOSS_NPC )
	{
		return CXSLUnitManager::MTF_BOSS_NPC;
	}

	return MONSTER_TYPE_FACTOR::MTF_NORMAL_NPC;
}
//}}

//{{ 필드 드롭 개편 - 김민성
#ifdef SERV_REFORM_ITEM_DROP
CXSLUnitManager::MONSTER_TYPE_FACTOR CXSLUnitManager::GetBattleFieldMonsterTypeFactor( IN int iAttribCount, IN char cMonsterGrade )
{
	if( iAttribCount > 0 )
	{
		if( iAttribCount < 3 )
			return MONSTER_TYPE_FACTOR::MTF_LOW_ELITE_NPC;
		else
			return MONSTER_TYPE_FACTOR::MTF_HIGH_ELITE_NPC;
	}
	else if( cMonsterGrade == CXSLUnitManager::MG_MIDDLE_BOSS_NPC )
	{
		return CXSLUnitManager::MTF_MIDDLE_BOSS_NPC;
	}
	else if( cMonsterGrade == CXSLUnitManager::MG_BOSS_NPC )
	{
		return CXSLUnitManager::MTF_BOSS_NPC;
	}

	return MONSTER_TYPE_FACTOR::MTF_NORMAL_NPC;
}
#endif SERV_REFORM_ITEM_DROP
//}}

//{{ 2011. 10. 24	최육사	배틀필드 시스템
//#ifdef SERV_BATTLE_FIELD_SYSTEM
bool CXSLUnitManager::IsValidMonsterGrade( IN const MONSTER_GRADE eMonsterGrade )
{
	switch( eMonsterGrade )
	{
	case MONSTER_GRADE::MG_NORMAL_NPC:
	case MONSTER_GRADE::MG_BOSS_NPC:
	case MONSTER_GRADE::MG_MIDDLE_BOSS_NPC:
		return true;
	}

	return false;
}
//#endif SERV_BATTLE_FIELD_SYSTEM
//}}

