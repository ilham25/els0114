#pragma once
#include "ServerDefine.h"

#ifdef	SERV_RIDING_PET_SYSTM// 적용날짜: 2013-04-21
#include "RefreshSingleton.h"

class CXSLRidingPetManager
{
	DeclareRefreshSingleton( CXSLRidingPetManager );
	DeclareLuaScriptParser;
	DeclDumpToLogFile;
	DeclDump;
	DeclToStringW;

public:
	enum RIDING_PET_UNIT_ID
	{
		RPUI_NONE				= 0,
		RPUI_NASOD_MOBI			= 1,	// 나소드 모비
		RPUI_KOUCIKA			= 2,	// 코우시카
		RPUI_ANCIENT_PPORU		= 3,	// 에인션트 뽀루
		RPUI_SCORPION_TYPE_R	= 4,	/// 퐁고 스콜피온 TYPE-R
		RPUI_END,
	};

	enum RIDING_PET_ENUM
	{
		RPE_MAX_PET_COUNT	= 30,
		RPE_PAGE_PER_COUNT	= 6,	// 음수를 넣을 경우 정상 동작을 보장하지 못한다.
	};

	struct RidingPetTemplet
	{
		RIDING_PET_UNIT_ID	m_iRidingPetID;
		USHORT				m_usMaxStamina;
		float				m_fStaminaRecoveryRate;
	};

public:
	CXSLRidingPetManager(void);
	~CXSLRidingPetManager(void);

	// for lua
	bool	AddRidingPetTemplet_LUA();
	bool	AddRidingPetCreateItemInfo_LUA( int iItemID, USHORT usRidingPetID, short sPeriod );
	void	SetEnableStamina_LUA( float fEnableStamina ){	m_fEnableStamina = fEnableStamina;	}
	float	GetEnableStamina( void ) const{	return m_fEnableStamina;	}
	
	// function
	const RidingPetTemplet*	GetRidingPetTemplet( IN const USHORT usRidingPetID ) const;

	bool	IsExistRidingPetID( IN const USHORT usiRidingPetID ) const{	return ( m_mapRidingPetTemplet.find( static_cast<RIDING_PET_UNIT_ID>( usiRidingPetID ) ) != m_mapRidingPetTemplet.end() );	}
	void	GetRidingPetID( IN const int iItemID, OUT USHORT& usCreatePetID, OUT short& sPeriod ) const;
	
	float	GetRecoveryTimePerOne( void );
	
	// static

private:
	std::map< RIDING_PET_UNIT_ID, RidingPetTemplet >		m_mapRidingPetTemplet;			// 라이딩 펫 데이터
	std::map< int, std::pair<RIDING_PET_UNIT_ID, short> >	m_mapRidingPetCreateItem;		// 라이딩 펫 생성 아이템 리스트
	float													m_fEnableStamina;				// 현재 스태미너 수치에 따른 사용 가능 여부 기준
};

DefRefreshSingletonInline( CXSLRidingPetManager );
#endif	// SERV_RIDING_PET_SYSTM