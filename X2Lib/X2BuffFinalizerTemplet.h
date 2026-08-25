/** @file : X2BuffFinalizerTemplet.h
    @breif : ������ ���� �κ��� �����ϴ� Templet ����
*/

#pragma once

class CX2BuffFinalizerTemplet;
typedef boost::shared_ptr<CX2BuffFinalizerTemplet> CX2BuffFinalizerTempletPtr;

/** @class : CX2BuffFinalizerTemplet
	@brief : ������ ���� �κ��� �����ϴ� Ŭ����
	@date : 2012/7/17/
*/
class CX2BuffFinalizerTemplet
{
public:
	static bool ParsingCombinationFinalizer( KLuaManager& luaManager_, OUT vector<CX2BuffFinalizerTempletPtr>& vecBuffFinalizerTempletPtr_ );
	
	CX2BuffFinalizerTemplet( const CX2BuffFinalizerTemplet& rhs_ ) : m_eType( rhs_.m_eType ), m_bReserveToFinish( false )
	{}
	virtual ~CX2BuffFinalizerTemplet() {}

	virtual void OnFrameMove( CX2GameUnit* pGameUnit_ ) {};
	virtual bool SetFactor( const CX2BuffFactor& BuffFactor_, CX2GameUnit* pGameUnit_, const CX2BuffTemplet* pBuffTemplet_ ) = NULL;
	virtual CX2BuffFinalizerTempletPtr GetClonePtr() const = NULL;
	virtual bool DidFinish( CX2GameUnit* pGameUnit_ ) const = NULL;
	virtual void GetFactor( OUT vector<KBuffFinalizerFactor>& vecFactors_, const CX2GameUnit* pGameUnit_ ) const = NULL;
	
	void ReserveToFinish() { m_bReserveToFinish = true; }
	bool DidReserveToFinish() const { return m_bReserveToFinish; }

	bool SetFactorFromPacketTemplateMothod( const KBuffFactor& kBuffFactor_, CX2GameUnit* pGameUnit_ );
	BUFF_FINALIZER_TYPE GetType() const { return m_eType; }

#ifdef SERV_IRUHADEV_BUFF_DURATION_TEXT
	/// Remaining duration in seconds; -1.f sentinel means this finalizer has no timer (hit-count, state-change, etc.)
	virtual float GetRemainTime() const { return -1.f; }
#endif //SERV_IRUHADEV_BUFF_DURATION_TEXT

protected:
	CX2BuffFinalizerTemplet() : m_eType( BFT_NONE ) {}

	void SetType(BUFF_FINALIZER_TYPE val) { m_eType = val; }

	bool		ParsingFinalizerTemplateMethod( KLuaManager& luaManager_, const WCHAR* pwszTableName_ );
	virtual bool ParsingFinalizer( KLuaManager& luaManager_ ) { return true; }
	virtual void SetFactorFromPacket( const KBuffFinalizerFactor& kFactor_, CX2GameUnit* pGameUnit_ ) {};
	
private:
	BUFF_FINALIZER_TYPE		m_eType;
	bool					m_bReserveToFinish;	/// ���Ḧ ���� (�� ���� true �� ���� üũ�ÿ� �����)
};

/** @class : CX2BuffTimeFinalizerTemplet
	@brief : �ð��� ���� ����Ǵ� ������ �����ϴ� Ŭ����
	@date : 2012/7/17/
*/
class CX2BuffTimeFinalizerTemplet : public CX2BuffFinalizerTemplet
{
public:
	enum FACTOR_ORDER
	{
		FO_DURATION_BUFF_TIME_TYPE,		/// ���� �ð�����, ���� �ð�����
		FO_NORMAL_TIME,					/// ������ �ð�
		FO_FORCE_TIME,					/// �������� �ð�
		FO_MIN_TIME = FO_NORMAL_TIME,	/// ������ ��� �ּ� �ð�
		FO_MAX_TIME,					/// ������ ��� �ִ� �ð�
		
		FO_END,					/// enum ����
	};

	static CX2BuffFinalizerTempletPtr CreateBuffFinalizerTempletPtr() { return CX2BuffFinalizerTempletPtr( new CX2BuffTimeFinalizerTemplet ); }

	virtual void OnFrameMove(  CX2GameUnit* pGameUnit_ );
	virtual bool SetFactor( const CX2BuffFactor& BuffFactor_, CX2GameUnit* pGameUnit_, const CX2BuffTemplet* pBuffTemplet_ );
	virtual CX2BuffFinalizerTempletPtr GetClonePtr() const;
	virtual bool DidFinish( CX2GameUnit* pGameUnit_ ) const;
	virtual void GetFactor( OUT vector<KBuffFinalizerFactor>& vecFactors_, const CX2GameUnit* pGameUnit_ ) const;

#ifdef SERV_IRUHADEV_BUFF_DURATION_TEXT
	virtual float GetRemainTime() const { return m_fDurationTime; }
#endif //SERV_IRUHADEV_BUFF_DURATION_TEXT

protected:
	CX2BuffTimeFinalizerTemplet() : CX2BuffFinalizerTemplet(),
		m_fDurationTime( 0.0f ), m_eDurationTimeType( BDTT_NONE )
#ifdef BUFF_ICON_UI //���� ���� ���� �ð� ���
		,m_eBuffTempleteID(BTI_NONE)
		,m_bIsAlreadyNotifyDurationTime(false)
#endif //BUFF_ICON_UI
 {}

	virtual void SetFactorFromPacket( const KBuffFinalizerFactor& kFactor_, CX2GameUnit* pGameUnit_ );
	virtual bool ParsingFinalizer(  KLuaManager& luaManager_ );

private:
	float	m_fDurationTime;						/// ������ ���ӽð�
	BUFF_DURATION_TIME_TYPE m_eDurationTimeType;	/// �����ð�����, �����ð�����
#ifdef BUFF_ICON_UI //���� ���� ���� �ð� ���
	BUFF_TEMPLET_ID			m_eBuffTempleteID;
	bool					m_bIsAlreadyNotifyDurationTime;
#endif //BUFF_ICON_UI
	
};

/** @class : CX2BuffHitCountFinalizerTemplet
	@brief : ���� Ÿ�� ī��Ʈ�� �Ǹ� ����Ǵ� ������ �����ϴ� Ŭ����
	@date : 2012/7/19/
*/
class CX2BuffHitCountFinalizerTemplet : public CX2BuffFinalizerTemplet
{
public:
	static const int	MAX_COUNT = 256;

	enum FACTOR_ORDER
	{
		FO_USE_BUFF_COUNT_TYPE,
		FO_COUNT,			/// FO_RESULT_COUNT ���� ����, FO_USE_BUFF_COUNT_TYPE �� ������ ����/����� ������ �����ϴ� Ÿ���̸� FO_RESULT_COUNT�� ��� �ϱ� ���� ����Ѵ�.
		FO_RESULT_COUNT,	/// ����ī��Ʈ���� ������ ��� count (����ī��Ʈ�� ������� �ʴ� ��� FO_COUNT�� ����)
		FO_END,				/// enum ����
	};

	static CX2BuffFinalizerTempletPtr CreateBuffFinalizerTempletPtr() { return CX2BuffFinalizerTempletPtr( new CX2BuffHitCountFinalizerTemplet ); }

	virtual void OnFrameMove(  CX2GameUnit* pGameUnit_ );
	virtual bool SetFactor( const CX2BuffFactor& BuffFactor_, CX2GameUnit* pGameUnit_, const CX2BuffTemplet* pBuffTemplet_ );
	virtual CX2BuffFinalizerTempletPtr GetClonePtr() const;
	virtual bool DidFinish( CX2GameUnit* pGameUnit_ ) const;
	virtual void GetFactor( OUT vector<KBuffFinalizerFactor>& vecFactors_, const CX2GameUnit* pGameUnit_ ) const;
	virtual void SetFactorFromPacket( const KBuffFinalizerFactor& kFactor_, CX2GameUnit* pGameUnit_ );

protected:
	CX2BuffHitCountFinalizerTemplet() : CX2BuffFinalizerTemplet(),
		m_eUseCountType( BUCT_NOT_USE ), m_iCountToBeFinished( 0 ), m_ucCountWhenStart( 0 ), m_bStart( false ) {}

	virtual bool ParsingFinalizer(  KLuaManager& luaManager_ );
	UINT		 GetNewCount( const KBuffFinalizerFactor* pFactor_, CX2GameUnit* pGameUnit_ ) const;


private:
	BUFF_USE_COUNT_TYPE		m_eUseCountType;
	int						m_iCountToBeFinished;		/// ����Ǳ� ���� �����ؾ��ϴ� ī��Ʈ(- ���� �ɼ� ����)
	UCHAR					m_ucCountWhenStart;			/// ���� ���۽� ī��Ʈ

	/// ���� ���� ����(SetFactor�� ����ɶ��� ó�� ������ �� true�� �����ϰ�
	/// �����κ��� ��Ŷ�� �޾Ƽ� ������ ���� P2P ��Ŷ�� ���� ���Ŀ� true�� ������
	bool					m_bStart;					
};

/** @class : CX2BuffHittedCountFinalizerTemplet
	@brief : ���� �ǰ� ī��Ʈ�� �Ǹ� ����Ǵ� ������ �����ϴ� Ŭ����
	@date : 2012/7/19/
*/
class CX2BuffHittedCountFinalizerTemplet : public CX2BuffFinalizerTemplet
{
public:
	static const int	MAX_COUNT = 256;

	enum FACTOR_ORDER
	{
		FO_USE_BUFF_COUNT_TYPE,
		FO_COUNT,			/// FO_RESULT_COUNT ���� ����, FO_USE_BUFF_COUNT_TYPE �� ������ ����/����� ������ �����ϴ� Ÿ���̸� FO_RESULT_COUNT�� ��� �ϱ� ���� ����Ѵ�.
		FO_RESULT_COUNT,	/// ����ī��Ʈ���� ������ ��� count (����ī��Ʈ�� ������� �ʴ� ��� FO_COUNT�� ����)
		FO_END,				/// enum ����
	};

	static CX2BuffFinalizerTempletPtr CreateBuffFinalizerTempletPtr() { return CX2BuffFinalizerTempletPtr( new CX2BuffHittedCountFinalizerTemplet ); }

	virtual void OnFrameMove(  CX2GameUnit* pGameUnit_ );
	virtual bool SetFactor( const CX2BuffFactor& BuffFactor_, CX2GameUnit* pGameUnit_, const CX2BuffTemplet* pBuffTemplet_ );
	virtual CX2BuffFinalizerTempletPtr GetClonePtr() const;
	virtual bool DidFinish( CX2GameUnit* pGameUnit_ ) const;
	virtual void GetFactor( OUT vector<KBuffFinalizerFactor>& vecFactors_, const CX2GameUnit* pGameUnit_ ) const;
	virtual void SetFactorFromPacket( const KBuffFinalizerFactor& kFactor_, CX2GameUnit* pGameUnit_ );

protected:
	CX2BuffHittedCountFinalizerTemplet() : CX2BuffFinalizerTemplet(),
		m_eUseCountType( BUCT_NOT_USE ), m_iCountToBeFinished( 0 ), m_ucCountWhenStart( 0 ), m_bStart( false ) {}

	virtual bool ParsingFinalizer(  KLuaManager& luaManager_ );
	UINT GetNewCount( const KBuffFinalizerFactor* pFactor_, CX2GameUnit* pGameUnit_ ) const;

private:
	BUFF_USE_COUNT_TYPE		m_eUseCountType;
	int						m_iCountToBeFinished;		/// ����Ǳ� ���� �����ؾ��ϴ� ī��Ʈ(- ���� �ɼ� ����)
	UCHAR					m_ucCountWhenStart;			/// ���� ���۽� ī��Ʈ

	/// ���� ���� ����(SetFactor�� ����ɶ��� ó�� ������ �� true�� �����ϰ�
	/// �����κ��� ��Ŷ�� �޾Ƽ� ������ ���� P2P ��Ŷ�� ���� ���Ŀ� true�� ������
	bool					m_bStart;				
};

/** @class : CX2BuffStateChangeFinalizerTemplet
	@brief : ���� ������ ������Ʈ���� ����Ǹ� ����Ǵ� ������ �����ϴ� Ŭ����
	@date : 2012/7/19/
*/
class CX2BuffStateChangeFinalizerTemplet : public CX2BuffFinalizerTemplet
{
public:
	enum FACTOR_ORDER
	{
		FO_STATE_ID,
		FO_END,				/// enum ����
	};

	static CX2BuffFinalizerTempletPtr CreateBuffFinalizerTempletPtr() { return CX2BuffFinalizerTempletPtr( new CX2BuffStateChangeFinalizerTemplet ); }

	virtual bool SetFactor( const CX2BuffFactor& BuffFactor_, CX2GameUnit* pGameUnit_, const CX2BuffTemplet* pBuffTemplet_ );
	virtual CX2BuffFinalizerTempletPtr GetClonePtr() const;
	virtual bool DidFinish( CX2GameUnit* pGameUnit_ ) const;
	virtual void GetFactor( OUT vector<KBuffFinalizerFactor>& vecFactors_, const CX2GameUnit* pGameUnit_ ) const;
	virtual void SetFactorFromPacket( const KBuffFinalizerFactor& kFactor_, CX2GameUnit* pGameUnit_ );

protected:
	CX2BuffStateChangeFinalizerTemplet() : CX2BuffFinalizerTemplet(),
		m_uiStateID( 0 ) {}

private:
	UINT	m_uiStateID;			/// ������ ���� ���� ���� ������Ʈ ID
};

/** @class : CX2BuffDamageValueFinalizerTemplet
	@brief : ������ �������� ������ ����Ǵ� ������ �����ϴ� Ŭ����
	@date : 2012/7/20/
*/
class CX2BuffDamageValueFinalizerTemplet : public CX2BuffFinalizerTemplet
{
public:
	enum FACTOR_ORDER
	{
		FO_CHANGE_TYPE,				/// ��ȭ�Ǵ� Ÿ��
		FO_VALUE,					/// ��
		FO_RELATION_TYPE,			/// CHANGE_TYPE�� ������ ��� ������ �����ɰ�����
		FO_MULTIPLIER,				/// ����(CHANGE_TYPE�� REATION �϶� ���)
		FO_END,						/// enum ����
	};

	static CX2BuffFinalizerTempletPtr CreateBuffFinalizerTempletPtr() { return CX2BuffFinalizerTempletPtr( new CX2BuffDamageValueFinalizerTemplet ); }

	virtual void OnFrameMove(  CX2GameUnit* pGameUnit_ );
	virtual bool SetFactor( const CX2BuffFactor& BuffFactor_, CX2GameUnit* pGameUnit_, const CX2BuffTemplet* pBuffTemplet_ );
	virtual CX2BuffFinalizerTempletPtr GetClonePtr() const;
	virtual bool DidFinish( CX2GameUnit* pGameUnit_ ) const;
	virtual void GetFactor( OUT vector<KBuffFinalizerFactor>& vecFactors_, const CX2GameUnit* pGameUnit_ ) const;
	virtual void SetFactorFromPacket( const KBuffFinalizerFactor& kFactor_, CX2GameUnit* pGameUnit_ );

protected:
	CX2BuffDamageValueFinalizerTemplet() : CX2BuffFinalizerTemplet(),
		m_eChangeType( BCT_SWAP_VALUE ), m_eRelationType( BRT_END ),
		m_fNowHpOldFrame( 0.0f ), m_fDamageValue( 0.0f ), m_bStart( false )
	{}

	virtual bool ParsingFinalizer(  KLuaManager& luaManager_ );
	float GetValue( const KBuffFinalizerFactor* pFinalizerFactor_, CX2GameUnit* pGameUnit_ ) const;
	void ModifyDamageValue( const CX2GameUnit* pGameUnit_ );

private:
	BUFF_CHANGE_TYPE		m_eChangeType;			/// ����Ÿ��
	BUFF_RELATION_TYPE		m_eRelationType;		/// ����Ÿ��
	float					m_fNowHpOldFrame;		/// ���� �������� Hp
	float					m_fDamageValue;			/// ��������
	/// ���� ���� ����(SetFactor�� ����ɶ��� ó�� ������ �� true�� �����ϰ�
	/// �����κ��� ��Ŷ�� �޾Ƽ� ������ ���� P2P ��Ŷ�� ���� ���Ŀ� true�� ������
	bool					m_bStart;				
};

/** @class : CX2BuffPassHpFinalizerTemplet
	@brief : ���� HP�� ���� �Ǵ� ���� ���� �ϴ� ��쿡 ����Ǵ� ��� ���� Ŭ����
	@date : 2012/7/20/
*/
class CX2BuffPassHpFinalizerTemplet : public CX2BuffFinalizerTemplet
{
public:
	enum FACTOR_ORDER
	{
		FO_PASS_UP,			/// ���� ����? ���� ����?
		FO_CHANGE_TYPE,		/// Ÿ��(% ���� ��ġ ����)
		FO_CRITERION,		/// ���� ��ġ
		FO_END,				/// enum ����
	};

	static CX2BuffFinalizerTempletPtr CreateBuffFinalizerTempletPtr() { return CX2BuffFinalizerTempletPtr( new CX2BuffPassHpFinalizerTemplet ); }

	virtual bool SetFactor( const CX2BuffFactor& BuffFactor_, CX2GameUnit* pGameUnit_, const CX2BuffTemplet* pBuffTemplet_ );
	virtual CX2BuffFinalizerTempletPtr GetClonePtr() const;
	virtual bool DidFinish( CX2GameUnit* pGameUnit_ ) const;
	virtual void GetFactor( OUT vector<KBuffFinalizerFactor>& vecFactors_, const CX2GameUnit* pGameUnit_ ) const;
	virtual void SetFactorFromPacket( const KBuffFinalizerFactor& kFactor_, CX2GameUnit* pGameUnit_ );

protected:
	CX2BuffPassHpFinalizerTemplet() : CX2BuffFinalizerTemplet(),
		m_eBuffChangeType( BCT_SWAP_VALUE ), m_fCriterion( 0.0f ), m_bPassUp( false ) {}

	virtual bool ParsingFinalizer(  KLuaManager& luaManager_ );
	void SelectDelegate();
	bool CheckHpByUp( CX2GameUnit* pGameUnit_ ) const;
	bool CheckHpByDown( CX2GameUnit* pGameUnit_ ) const;

	float GetCriterionByValue( CX2GameUnit* pGameUnit_ ) const;
	float GetCriterionByPercent( CX2GameUnit* pGameUnit_ ) const;

private:
	typedef srutil::delegate1<bool, CX2GameUnit*> DelegateCheckHpByPassType;
	typedef srutil::delegate1<float, CX2GameUnit*> DelegateGetCriterionByChangeType;

	DelegateCheckHpByPassType				m_delegateCheckHpByPassTypeFunc;			/// Ÿ�Կ� ���ؼ� �ٸ� üũ �Լ� ����
	DelegateGetCriterionByChangeType		m_delegateGetCriterionByChangeTypeFunc;		/// Ÿ�Կ� ���ؼ� % �Ǵ� ���� ����
	BUFF_CHANGE_TYPE			m_eBuffChangeType;	/// Ÿ��(% ���� ��ġ ����)
	float						m_fCriterion;		/// ���� ��ġ
	bool						m_bPassUp;			/// ���⵹�� �ΰ� ���⵹���ΰ�?
};

/** @class : CX2BuffPassMpFinalizerTemplet
	@brief : ���� Mp�� ���� �Ǵ� ���� ���� �ϴ� ��쿡 ����Ǵ� ��� ���� Ŭ����
	@date : 2012/7/20/
*/
class CX2BuffPassMpFinalizerTemplet : public CX2BuffFinalizerTemplet
{
public:
	enum FACTOR_ORDER
	{
		FO_PASS_UP,
		FO_CHANGE_TYPE,		/// Ÿ��(% ���� ��ġ ����)
		FO_CRITERION,		/// ���� ��ġ
		FO_END,				/// enum ����
	};

	static CX2BuffFinalizerTempletPtr CreateBuffFinalizerTempletPtr() { return CX2BuffFinalizerTempletPtr( new CX2BuffPassMpFinalizerTemplet ); }
public:
	virtual bool SetFactor( const CX2BuffFactor& BuffFactor_, CX2GameUnit* pGameUnit_, const CX2BuffTemplet* pBuffTemplet_ );
	virtual CX2BuffFinalizerTempletPtr GetClonePtr() const;
	virtual bool DidFinish( CX2GameUnit* pGameUnit_ ) const;
	virtual void GetFactor( OUT vector<KBuffFinalizerFactor>& vecFactors_, const CX2GameUnit* pGameUnit_ ) const;
	virtual void SetFactorFromPacket( const KBuffFinalizerFactor& kFactor_, CX2GameUnit* pGameUnit_ );

protected:
	CX2BuffPassMpFinalizerTemplet() : CX2BuffFinalizerTemplet(),
		m_eBuffChangeType( BCT_SWAP_VALUE ), m_fCriterion( 0.0f ), m_bPassUp( false ) {}

	virtual bool ParsingFinalizer(  KLuaManager& luaManager_ );
	void SelectDelegate();
	bool CheckMpByUp( CX2GameUnit* pGameUnit_ ) const;
	bool CheckMpByDown( CX2GameUnit* pGameUnit_ ) const;

	float GetCriterionByValue( CX2GameUnit* pGameUnit_ ) const;
	float GetCriterionByPercent( CX2GameUnit* pGameUnit_ ) const;

private:
	typedef srutil::delegate1<bool, CX2GameUnit*> DelegateCheckMpByPassType;
	typedef srutil::delegate1<float, CX2GameUnit*> DelegateGetCriterionByChangeType;

	DelegateCheckMpByPassType				m_delegateCheckMpByPassTypeFunc;			/// Ÿ�Կ� ���ؼ� �ٸ� üũ �Լ� ����
	DelegateGetCriterionByChangeType		m_delegateGetCriterionByChangeTypeFunc;		/// Ÿ�Կ� ���ؼ� % �Ǵ� ���� ����
	BUFF_CHANGE_TYPE			m_eBuffChangeType;	/// Ÿ��(% ���� ��ġ ����)
	float						m_fCriterion;		/// ���� ��ġ
	bool						m_bPassUp;			/// ���⵹�� �ΰ� ���⵹���ΰ�?
};

/** @class : CX2BuffImmediateFinalizerTemplet
	@brief : ���� ��� ������ �ǹ̷� �ѹ��� FrameMove ���� �� ����Ǵ� ������ �����ϴ� Ŭ����
	@date : 2012/7/19/
*/
class CX2BuffImmediateFinalizerTemplet : public CX2BuffFinalizerTemplet
{
public:
	static CX2BuffFinalizerTempletPtr CreateBuffFinalizerTempletPtr() { return CX2BuffFinalizerTempletPtr( new CX2BuffImmediateFinalizerTemplet ); }

	virtual bool SetFactor( const CX2BuffFactor& BuffFactor_, CX2GameUnit* pGameUnit_, const CX2BuffTemplet* pBuffTemplet_ );
	virtual CX2BuffFinalizerTempletPtr GetClonePtr() const;
	virtual bool DidFinish( CX2GameUnit* pGameUnit_ ) const;
	virtual void GetFactor( OUT vector<KBuffFinalizerFactor>& vecFactors_, const CX2GameUnit* pGameUnit_ ) const;
	
protected:
	CX2BuffImmediateFinalizerTemplet() : CX2BuffFinalizerTemplet()
	{}
};

/** @class : CX2BuffDungeonStageChangeFinalizerTemplet
	@brief : ������ �������� ���� ���� �Ǹ� ����Ǵ� ���ø�
	@date : 2012/9/1/
*/
class CX2BuffDungeonStageChangeFinalizerTemplet : public CX2BuffFinalizerTemplet
{
public:
	static CX2BuffFinalizerTempletPtr CreateBuffFinalizerTempletPtr() { return CX2BuffFinalizerTempletPtr( new CX2BuffDungeonStageChangeFinalizerTemplet ); }

	virtual bool SetFactor( const CX2BuffFactor& BuffFactor_, CX2GameUnit* pGameUnit_, const CX2BuffTemplet* pBuffTemplet_ );
	virtual CX2BuffFinalizerTempletPtr GetClonePtr() const;
	virtual bool DidFinish( CX2GameUnit* pGameUnit_ ) const;
	virtual void GetFactor( OUT vector<KBuffFinalizerFactor>& vecFactors_, const CX2GameUnit* pGameUnit_ ) const;
	virtual void SetFactorFromPacket( const KBuffFinalizerFactor& kFactor_, CX2GameUnit* pGameUnit_ );

protected:
	CX2BuffDungeonStageChangeFinalizerTemplet() 
		: CX2BuffFinalizerTemplet(), m_uiIndex( 0 )
	{}

private:
	UINT	m_uiIndex;
};

#ifdef ADD_LIVE_CREATOR_FINALIZER
/** @class : CX2BuffLiveCreatorFinalizerTemplet
	@brief : ���� �����ڰ� ����ϰų� �������� ������ ����Ǵ� ���ø�
	@date : 2012/11/15/
*/
class CX2BuffLiveCreatorFinalizerTemplet : public CX2BuffFinalizerTemplet
{
public:
	enum FACTOR_ORDER
	{
		/// int64�� 16��Ʈ�� float 4���� �ɰ��� ����...
		/// ( FO_UNIT_UID_FRONT_UP   FO_UNIT_UID_FRONT_DOWN   FO_UNIT_UID_BACK_UP   FO_UNIT_UID_BACK_DOWN )
		FO_UNIT_UID_BACK_DOWN,		/// ���� UID ���� �Ʒ� ��Ʈ
		FO_UNIT_UID_BACK_UP,		/// ���� UID ���� ��   ��Ʈ
		FO_UNIT_UID_FRONT_DOWN,		/// ���� UID ���� �Ʒ� ��Ʈ
		FO_UNIT_UID_FRONT_UP,		/// ���� UID ���� ��   ��Ʈ
		FO_IS_USER_UNIT,			/// �������� NPC���� ����
	};

	static CX2BuffFinalizerTempletPtr CreateBuffFinalizerTempletPtr() { return CX2BuffFinalizerTempletPtr( new CX2BuffLiveCreatorFinalizerTemplet ); }

	virtual void OnFrameMove(  CX2GameUnit* pGameUnit_ );
	virtual bool SetFactor( const CX2BuffFactor& BuffFactor_, CX2GameUnit* pGameUnit_, const CX2BuffTemplet* pBuffTemplet_ );
	virtual CX2BuffFinalizerTempletPtr GetClonePtr() const;
	virtual bool DidFinish( CX2GameUnit* pGameUnit_ ) const;
	virtual void GetFactor( OUT vector<KBuffFinalizerFactor>& vecFactors_, const CX2GameUnit* pGameUnit_ ) const;
	virtual void SetFactorFromPacket( const KBuffFinalizerFactor& kFactor_, CX2GameUnit* pGameUnit_ );

protected:
	CX2BuffLiveCreatorFinalizerTemplet() 
		: CX2BuffFinalizerTemplet(), m_uidCreatorUID( -1 ), m_bIsLiveCreator( true )
	{}

private:
	//UidType m_iCreatorUID;
	UidType				m_uidCreatorUID;		/// ���� UID
	bool				m_bIsUserUnit;			/// �������� NPC���� ����
	bool				m_bIsLiveCreator;		/// ���� ������ ���� Ȥ�� ���� ����
	CX2GameUnitoPtr		m_optrCreatorUnit;		/// ������ ���� ��ü
};
#endif ADD_LIVE_CREATOR_FINALIZER

#ifdef ADD_BUFF_FINALIZER_FINISH_HYPER
/** @class : CX2BuffFinishHyperFinalizerTemplet
	@brief : ������ ������ ��� ����Ǵ� ���ø�
	@date  : 2013/02/22
*/
class CX2BuffFinishHyperFinalizerTemplet : public CX2BuffFinalizerTemplet
{
public:
	static CX2BuffFinalizerTempletPtr CreateBuffFinalizerTempletPtr() { return CX2BuffFinalizerTempletPtr( new CX2BuffFinishHyperFinalizerTemplet ); }
	
	virtual bool SetFactor( const CX2BuffFactor& BuffFactor_, CX2GameUnit* pGameUnit_, const CX2BuffTemplet* pBuffTemplet_ );
	virtual CX2BuffFinalizerTempletPtr GetClonePtr() const;
	virtual bool DidFinish( CX2GameUnit* pGameUnit_ ) const;
	virtual void GetFactor( OUT vector<KBuffFinalizerFactor>& vecFactors_, const CX2GameUnit* pGameUnit_ ) const;
	virtual void SetFactorFromPacket( const KBuffFinalizerFactor& kFactor_, CX2GameUnit* pGameUnit_ );
	virtual void OnFrameMove(  CX2GameUnit* pGameUnit_ );

protected:
	CX2BuffFinishHyperFinalizerTemplet() 
		: CX2BuffFinalizerTemplet(), m_bHyper( false ), m_fDurationTime( 0.0f ), m_eBuffTempleteID(BTI_NONE)
		, m_bIsAlreadyNotifyDurationTime(false)
	{}

private:
	bool	m_bHyper;			/// ���� ���� Ȯ��
	float	m_fDurationTime;	/// ������ ���ӽð�

	BUFF_TEMPLET_ID		m_eBuffTempleteID;
	bool				m_bIsAlreadyNotifyDurationTime;
};
#endif //ADD_BUFF_FINALIZER_FINISH_HYPER

#ifdef	RIDING_SYSTEM
/** @class : CX2BuffFinishRidingPetOnOrNotFinalizerTemplet
	@brief : ���̵� �꿡 ���ų� ������ ����Ǵ� ���ø�
	@date  : 2013/04/22
*/
class CX2BuffFinishRidingPetOnOrNotFinalizerTemplet : public CX2BuffFinalizerTemplet
{
public:
	static CX2BuffFinalizerTempletPtr CreateBuffFinalizerTempletPtr() { return CX2BuffFinalizerTempletPtr( new CX2BuffFinishRidingPetOnOrNotFinalizerTemplet ); }
	
	virtual bool SetFactor( const CX2BuffFactor& BuffFactor_, CX2GameUnit* pGameUnit_, const CX2BuffTemplet* pBuffTemplet_ );
	virtual CX2BuffFinalizerTempletPtr GetClonePtr() const;
	virtual bool DidFinish( CX2GameUnit* pGameUnit_ ) const;
	virtual void GetFactor( OUT vector<KBuffFinalizerFactor>& vecFactors_, const CX2GameUnit* pGameUnit_ ) const;
	
protected:
	CX2BuffFinishRidingPetOnOrNotFinalizerTemplet() 
		: CX2BuffFinalizerTemplet(), m_bFinishRidingOn( false )
	{}
	
	bool		ParsingFinalizerTemplateMethod( KLuaManager& luaManager_, const WCHAR* pwszTableName_ );

private:

	/// ���̵� �꿡 ���� �� ������ ����Ǿ� �ϸ� true, ������ �� ���� �Ǿ�� �ϸ� false
	bool	m_bFinishRidingOn;
};
#endif	RIDING_SYSTEM

/** @class : CX2BuffDungeonStageChangeFinalizerTemplet
	@brief : X2 ������Ʈ ���� �Ǹ� ���� �Ǵ� ���ø�
	@date : 2013/7/16/
*/
class CX2BuffX2StateChangeFinalizerTemplet : public CX2BuffFinalizerTemplet
{
public:
	static CX2BuffFinalizerTempletPtr CreateBuffFinalizerTempletPtr() { return CX2BuffFinalizerTempletPtr( new CX2BuffX2StateChangeFinalizerTemplet ); }

	virtual bool SetFactor( const CX2BuffFactor& BuffFactor_, CX2GameUnit* pGameUnit_, const CX2BuffTemplet* pBuffTemplet_ );
	virtual CX2BuffFinalizerTempletPtr GetClonePtr() const;
	virtual bool DidFinish( CX2GameUnit* pGameUnit_ ) const;
	virtual void GetFactor( OUT vector<KBuffFinalizerFactor>& vecFactors_, const CX2GameUnit* pGameUnit_ ) const;
	virtual void SetFactorFromPacket( const KBuffFinalizerFactor& kFactor_, CX2GameUnit* pGameUnit_ );

protected:
	CX2BuffX2StateChangeFinalizerTemplet() 
		: CX2BuffFinalizerTemplet(), m_uiIndex( 0 )
	{}

private:
	UINT	m_uiIndex;
};