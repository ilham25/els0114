#pragma once

/* 오현빈 // 2013-05-20
-공존이 축제, 어둠의 문 시작 했을 때 등 
 화면 가운데 나타나는 엠블럼을 관리 하는 매니져 클래스입니다.

-기존 코드는 순서를 관리 할 수 없어서, 
 같은 시간대에 출력 했을 때, 엠블럼이 겹치는 문제가 있었으며
 상기 문제를 해결 하기 위해 작성한 클래스입니다.
*/
#ifdef NEW_EMBLEM_MANAGER
class CX2EmblemManager
{
public:
	enum EMBLEM_ID
	{
		EI_NONE	= -1,
		EI_FESTIVAL = 0,	// 공존의 축제
		EI_DEFENCE_START,	// 어문 침공 시작
		EI_DEFENCE_SUCCESS,	// 성공
		EI_DEFENEC_FAIL,	// 실패
		EI_EVENT_CRAYONPOP,	// 크래용팝 이벤트
		EI_EVENT_ELESIS,	// 엘리시스 이벤트
	};

	struct EmblemTemplet
	{
		EMBLEM_ID	m_eEmblemID;			// ID
		
		bool		m_bIsFirstOnly;			// 매회 첫 캐릭터 접속 시에만 보여 줄 것인가?
		bool		m_bCheckCondition;		// 조건 체크 해서 보여 줄 것인가?(FALSE = 무조건 출력)	
		bool		m_bIsPassCondition;		// 조건 체크 통과 여부

		UINT		m_uiShowOrder;			// 출력 순서( 상대적인 순서, 0부터 시작 )

		wstring		m_wstrTextureFileName;	// 출력 텍스쳐 파일 명
		D3DXVECTOR3 m_vecTime;	// x : 출력 지속 시간(페이드 시간 포함)
								// y : 시작 후 알파값 1이 될 때 까지 소요되는 시간
								// z : 종료 전 알파값 0이 될 때 까지 소요되는 시간

		bool		m_bShowVilleage;
		bool		m_bShowBattleFIeld;
		bool		m_bShowDungeon;

		bool		m_bIsAlreadyShow;

		EmblemTemplet():
		m_bIsFirstOnly(false),
		m_bCheckCondition(false),
		m_uiShowOrder(0),
		m_bShowVilleage(false),
		m_bShowBattleFIeld(false),
		m_bShowDungeon(false),
		m_bIsAlreadyShow(false),
		m_bIsPassCondition(false)
		{}

		void InitShowState()
		{
			m_bIsAlreadyShow = false;
		}		
		void InitIsPassCondition()
		{// 조건 없다면 무조건 true
			if( false == m_bCheckCondition )
				m_bIsPassCondition = true;
			else
				m_bIsPassCondition = false;
		}
	};

public:
	static CX2EmblemManager* GetInstance();
	static void ResetEmblemManager();
	static void DestroyInstance();
	
	void OnFrameMove( float fElpaseTime );
	void InitEmbelmPlayInfo( bool bCharChange = false );	// 엠블럼 진행 관련 정보 초기화

	void PlayEmblem( EMBLEM_ID eEmblemID_ ); 
	bool HasEmblem( EMBLEM_ID eEmblemID_ );
private:
	// 파싱
	bool OpenScriptFile( IN const WCHAR* pFileName_ );
	void ParsingEmblemTemplet( KLuaManager& luaManager );

	// 이펙트 생성
	CKTDGParticleSystem::CParticleEventSequence* CreateEventSequence( const WCHAR* wstrTextureFile_, 
		const D3DXVECTOR3& vTime);
	void PlayEmblemEffect( const EmblemTemplet& sEmblemTemplet_ );
	void PlayEmblemOtehrProcess( const EmblemTemplet& sEmblemTemplet_ );
	void CheckNextPlayingEmblem();

private:
	CX2EmblemManager();
	~CX2EmblemManager();
	
	static CX2EmblemManager* m_pEmblemManager;
	std::vector<EmblemTemplet*>	m_vecEmblemTemplet;

	bool m_bHasNextPlayEmblem;	// 다음에 출력 할 엠블럼이 존재 여부
	float m_fWaitTime; // 출력 전 대기 시간
	CKTDGParticleSystem::CParticleEventSequenceHandle m_hCurrentEmblem;
};

#endif // NEW_EMBLEM_MANAGER

