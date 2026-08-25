/** @file : X2BuffTemplet.h
    @breif : ������ �ൿ�� ������� ������ �ִ� Ŭ���� ���� ����
*/

class CX2BuffTemplet;
typedef boost::shared_ptr<CX2BuffTemplet> CX2BuffTempletPtr;

#pragma once

/** @class : CX2BuffTemplet
	@brief : ������ �ൿ�� ������� ������ �ִ� Ŭ����
	@date : 2012/7/17/
*/
class CX2BuffTemplet
{
public:
	static CX2BuffTempletPtr CreateBuffTempletPtr() { return CX2BuffTempletPtr( new CX2BuffTemplet ); }

public:
	enum UN_COMMON_MEMBER		/// ��� ������ �������� ����Ǵ� ������ �ƴ� ���� ���� �ϱ� ���� enum
	{
		UCM_ACCUMULATION_COUNT_NOW = 0,			/// ���� ��ø ī��Ʈ (��ø������ �ÿ� ���� ���ŭ ��ø�� �Ǿ��°�)
		UCM_ACCUMULATION_MULTIPLIER,		/// ��ø�� ����Ǵ� ����
		UCM_REGIST_TYPE,					/// ���׿� ������ ���� �� � ���׿� ������ �޴��� ����(CX2EnchantItem::ENCHANT_TYPE)
	};

	CX2BuffTemplet( const CX2BuffTemplet& rhs_ ) 
		: m_BuffIdentity( rhs_.m_BuffIdentity ), m_uiAccumulationLimit( rhs_.m_uiAccumulationLimit )
		, m_bDeBuff( rhs_.m_bDeBuff ), m_bDidFinish( rhs_.m_bDidFinish ), m_bWorldBuff( rhs_.m_bWorldBuff )
#ifdef BUFF_ICON_UI
		, m_wstIconFileName ( rhs_.m_wstIconFileName ), m_wstIconKeyName ( rhs_.m_wstIconKeyName )
		, m_iBuffName( rhs_.m_iBuffName ), m_iBuffDesc( rhs_.m_iBuffDesc ), m_bUseBuffIcon ( rhs_.m_bUseBuffIcon )
#endif //BUFF_ICON_UI
	{
		m_vecBehaviorPtr.clear();
		m_vecBehaviorPtr.reserve( rhs_.m_vecBehaviorPtr.size() );
		BOOST_FOREACH( CX2BuffBehaviorTempletPtr templet, rhs_.m_vecBehaviorPtr )
		{
			m_vecBehaviorPtr.push_back( templet->GetClonePtr() );
		}

		m_vecFinalizerPtr.clear();
		m_vecFinalizerPtr.reserve( rhs_.m_vecFinalizerPtr.size() );
		BOOST_FOREACH( CX2BuffFinalizerTempletPtr templet, rhs_.m_vecFinalizerPtr )
		{
			m_vecFinalizerPtr.push_back( templet->GetClonePtr() );
		}

		m_vecDisplayerPtr.clear();
		m_vecDisplayerPtr.reserve( rhs_.m_vecDisplayerPtr.size() );
		BOOST_FOREACH( CX2BuffDisplayerTempletPtr templet, rhs_.m_vecDisplayerPtr )
		{
			m_vecDisplayerPtr.push_back( templet->GetClonePtr() );
		}

		m_mapUnCommonMember.clear();
		m_mapUnCommonMember = rhs_.m_mapUnCommonMember;
	}

	CX2BuffTempletPtr GetClonePtr() const { return CX2BuffTempletPtr( new CX2BuffTemplet( *this ) ); }

	bool ParsingScript( KLuaManager& luaManager_ );
	bool SetFactorToOnlyFinalizer( const CX2BuffFactor& buffFactor_, CX2GameUnit* pGameUnit_ );
	bool SetFactor( const CX2BuffFactor& buffFactor_, CX2GameUnit* pGameUnit_ );
	bool SetFactorFromPacket( const KBuffFactor& kBuffFactor_, CX2GameUnit* pGameUnit_ );

	void OnFrameMove( CX2GameUnit* pGameUnit_ )
	{
		OnFrameMoveBehavior( pGameUnit_ );
		OnFrameMoveFinalizerAndCheckFinish( pGameUnit_ );
		OnFrameMoveDisplayer( pGameUnit_ );
	}
	
	void DoFinishBehavior( CX2GameUnit* pGameUnit_ );
	void DoFinishDisplayer( CX2GameUnit* pGameUnit_ );

	void ReserveToFinish();

	BUFF_TEMPLET_ID GetBuffTempletID() const { return static_cast<BUFF_TEMPLET_ID>( m_BuffIdentity.m_eBuffTempletID ); }
	//void SetBuffTempletID(BUFF_TEMPLET_ID val) { m_eBuffTempletID = val; }

	UINT GetAccumulationLimit() const { return m_uiAccumulationLimit; }
	//void SetAccumulationLimit(UINT val) { m_uiAccumulationLimit = val; }
	
	bool IsWorldBuff() const { return m_bWorldBuff; }
	bool IsDeBuff() const { return m_bDeBuff; }
	//void DeBuff(bool val) { m_bDeBuff = val; }
	
	UINT GetAccumulationCountNow() const;
	void IncreaseAccumulationCountNow();

	float GetAccumulationMultiflier() const;

	UINT GetRegistType() const;

	bool GetDidFinish() const { return m_bDidFinish; }
	void SetDidFinish(bool val) { m_bDidFinish = val; }

	/// �ߺ��� ������ ���� �� ��쿡 ������ �� �ִ� ����(ī��Ʈ)
	UINT GetUniqueNum() const { return m_BuffIdentity.m_uiUniqueNum; }
	void SetUniqueNum( const UINT uiUniqueNum_ ) { m_BuffIdentity.m_uiUniqueNum = uiUniqueNum_; }

	const KBuffIdentity&	GetBuffIdentity() const { return m_BuffIdentity; }

	void GetFactor( OUT KBuffFactor& kBuffFactor_, const CX2GameUnit* pGameUnit_ ) const;

#ifdef BUFF_ICON_UI
	const wstring& GetIconFileName() const { return m_wstIconFileName; }
	const wstring& GetIconKeyName() const { return m_wstIconKeyName; }
	const int GetBuffName() const { return m_iBuffName; }
	const int GetBuffDesc() const { return m_iBuffDesc; }
	const bool GetUseBuffIcon() const { return m_bUseBuffIcon; }
#endif //BUFF_ICON_UI

	void GetFinalizerTempletPtrList( OUT vector<CX2BuffFinalizerTempletPtr>& vecFinalizerPtr_ );
	void ChangeFinalizerTempletPtrList( const vector<CX2BuffFinalizerTempletPtr>& vecFinalizerPtr_ );

#ifdef SERV_IRUHADEV_BUFF_DURATION_TEXT
	/// Remaining seconds of the BFT_TIME finalizer, if any; -1.f if this buff has no time-based finalizer
	float GetRemainDurationTime() const;
#endif //SERV_IRUHADEV_BUFF_DURATION_TEXT

protected:
	CX2BuffTemplet() : m_BuffIdentity(), m_uiAccumulationLimit( 0 ), m_bDeBuff( false )
						,m_bDidFinish( false ),	m_bWorldBuff( false )
#ifdef BUFF_ICON_UI
						,m_iBuffName( -1 ), m_iBuffDesc( -1 ), m_bUseBuffIcon ( false )
#endif //BUFF_ICON_UI

	{
		m_mapUnCommonMember.clear();
	}

	void OnFrameMoveBehavior( CX2GameUnit* pGameUnit_ );
	void OnFrameMoveFinalizerAndCheckFinish( CX2GameUnit* pGameUnit_ );
	void OnFrameMoveDisplayer( CX2GameUnit* pGameUnit_ );

private:
	vector<CX2BuffBehaviorTempletPtr>		m_vecBehaviorPtr;
	vector<CX2BuffFinalizerTempletPtr>		m_vecFinalizerPtr;
	vector<CX2BuffDisplayerTempletPtr>		m_vecDisplayerPtr;
	KBuffIdentity							m_BuffIdentity;
	UINT									m_uiAccumulationLimit;		/// ��ø ���� Ƚ���� 0�� �ߺ�(���� ������ �ɸ�������), 1�� ��ü(���ο� ������ ��ü), 2 �̻� ���ʹ� �ش� Ƚ�� ��ŭ ��ø ����(��ø ������ ���� ����)

	typedef map<UN_COMMON_MEMBER, float>	MAP_UN_COMMON_MEMBER;
	MAP_UN_COMMON_MEMBER					m_mapUnCommonMember;		/// ��� ������ �������� ����Ǵ� ������ �ƴ� ���� ���� �ϱ� ���� Map

	bool									m_bDeBuff;					/// �طο� ȿ���� �ִ� ������� ��� true
	bool									m_bDidFinish;				/// ����� ���� �ΰ�?
	bool									m_bWorldBuff;				/// ���� ������ ��� true

#ifdef BUFF_ICON_UI
	wstring									m_wstIconFileName;
	wstring									m_wstIconKeyName;
	int										m_iBuffName;
	int										m_iBuffDesc;
	bool									m_bUseBuffIcon;				/// ���� ������ ��� ����
#endif //BUFF_ICON_UI

	
};

////////////////////////////////////////////////////////////////////////////////////////////////

/** @class : CX2BuffIdentity
	@brief : KBuffIdentity �� ������ �ش� ������ �������� ���ϴ� Ŭ����
	@date : 2012/7/22/
*/
class CX2BuffIdentity;
typedef boost::shared_ptr<CX2BuffIdentity> CX2BuffIdentityPtr;
class CX2BuffIdentity
{
public:
	CX2BuffIdentity( const KBuffIdentity& BuffIdentity_ )
		: m_DataBuffIdentity( BuffIdentity_ )
	{}
			
	virtual ~CX2BuffIdentity() {}

	/// ���ϰ��� �ϴ� ������ ���� �߻��� ȿ������ Ȯ��
	bool IsSame( const KBuffIdentity& BuffIdentity_ ) const
	{
		if ( m_DataBuffIdentity.m_eBuffTempletID == BuffIdentity_.m_eBuffTempletID 
			&& m_DataBuffIdentity.m_uiUniqueNum == BuffIdentity_.m_uiUniqueNum )
			return true;
		else
			return false;
	}

#ifdef VERIFY_STAT_BY_BUFF
	virtual bool Verify() { return true; }
#endif // VERIFY_STAT_BY_BUFF

protected:
	CX2BuffIdentity() {}

protected:
	KBuffIdentity		m_DataBuffIdentity;		/// � ������ ���� ȿ�� ����
};

/** @function : FindSameBuffIdentity
	@brief : CX2BuffIdentity�� ��ӹ޴� Ŭ������ ��� ����Ʈ ������ ���� �� ���������� ������ ��Ҹ� ã�� �Լ�
	@param : ����Ʈ�����ͺ���(vector_), ��������(BuffIdentity_)
	@return : 
*/
template<typename ITR, typename T>
ITR FindSameBuffIdentity( IN vector<T>& vector_, const KBuffIdentity& BuffIdentity_ )
{
	if ( !vector_.empty() )
	{
		auto FindFunc = [BuffIdentity_]( T ptr_ ) {
			return ptr_->IsSame( BuffIdentity_ );
		};

		vector<T>::iterator vItr = std::find_if( vector_.begin(), vector_.end(), FindFunc );
		return vItr;
	}
	else
		return vector_.end();
}

/** @class : CX2ReverseLefRight
	@brief : �¿���� ȿ�� Ŭ����
	@date : 2012/7/22/
*/
class CX2ReverseLefRight;
typedef boost::shared_ptr<CX2ReverseLefRight> CX2ReverseLefRightPtr;
class CX2ReverseLefRight : public CX2BuffIdentity
{
public:
	CX2ReverseLefRight( const KBuffIdentity& BuffIdentity_ ) 
		: CX2BuffIdentity( BuffIdentity_ ),
		m_bReverse( true )	/// �������� ���� �¿���� ȿ���� �ֱ� ���� true�� ����
	{}

	/** @function : GetReverse
		@brief : ������ ���� ���ΰ�?
		@return : �������̸� true
	*/
	bool GetReverse() const { return m_bReverse; }

	/// �¿���� ���
	void ToggleReverseLeftRightByBuff() { m_bReverse = !m_bReverse; }

protected:
	CX2ReverseLefRight() {}

private:
	bool				m_bReverse;				/// true�� ���� �����ӿ� �¿���� ���̾�� ��
};

/** @class : CX2NeverMoveByBuff
	@brief : ���� ��ġ�� ������Ű�� Ŭ����
	@date : 2012/9/6/
*/
class CX2NeverMoveByBuff;
typedef boost::shared_ptr<CX2NeverMoveByBuff> CX2NeverMoveByBuffPtr;
class CX2NeverMoveByBuff : public CX2BuffIdentity
{
public:
#ifdef NEVER_MOVE_GRAVITY_APPLY
	CX2NeverMoveByBuff( const KBuffIdentity& BuffIdentity_, const D3DXVECTOR3& vPos_, const bool vGravity_ ) 
		: CX2BuffIdentity( BuffIdentity_ ), m_vPos( vPos_ ), m_bIsGravity( vGravity_ )
	{}
#else
	CX2NeverMoveByBuff( const KBuffIdentity& BuffIdentity_, const D3DXVECTOR3& vPos_ ) 
		: CX2BuffIdentity( BuffIdentity_ ), m_vPos( vPos_ )
	{}
#endif //NEVER_MOVE_GRAVITY_APPLY

	const D3DXVECTOR3& GetPos() const { return m_vPos; }
#ifdef NEVER_MOVE_GRAVITY_APPLY
	const bool		   GetGravitiApply() const { return m_bIsGravity; }
#endif //NEVER_MOVE_GRAVITY_APPLY

protected:
	CX2NeverMoveByBuff() 
#ifdef NEVER_MOVE_GRAVITY_APPLY
		: CX2BuffIdentity(), m_vPos( 0.0f, 0.0f, 0.0f )	, m_bIsGravity( false )
#else
		: CX2BuffIdentity(), m_vPos( 0.0f, 0.0f, 0.0f )
#endif //NEVER_MOVE_GRAVITY_APPLY
	{}

private:
	D3DXVECTOR3		m_vPos;
#ifdef NEVER_MOVE_GRAVITY_APPLY
	bool			m_bIsGravity;	//�߷��� �����Ұ��ΰ�. true�� �߷� ����, false�� ��ġ ����
#endif //NEVER_MOVE_GRAVITY_APPLY

};

/** @class : CX2RenderParamByBuff
	@brief : ������ ���ؼ� ����ɼ� �ִ� RenderParam ���� BuffIdentity�� �����Ͽ� ������ �� �ְ��� Ŭ����
	@date : 2012/8/3/
*/

class CX2RenderParamByBuff;
typedef boost::shared_ptr<CX2RenderParamByBuff> CX2RenderParamByBuffPtr;
class CX2RenderParamByBuff : public CX2BuffIdentity
{
public:
	CX2RenderParamByBuff( const KBuffIdentity& BuffIdentity_, const StBuffRenderParam& stBuffRenderParam_ )
		: CX2BuffIdentity( BuffIdentity_ ), m_stBuffRenderParam( stBuffRenderParam_ )
	{}

	const StBuffRenderParam& GetBuffRenderParam() const { return m_stBuffRenderParam; }

private:
	StBuffRenderParam	m_stBuffRenderParam;
};

/** @class : CX2EffectSetImpactPointByBuff
	@brief : Ÿ�ݽ� Ÿ�� ����Ʈ�� ����Ʈ ���� �����ϴ� ����� BuffIdentity�� �����Ͽ� ������ �� �ְ� �� Ŭ����
	@date : 2012/8/20/
*/
class CX2EffectSetImpactPointByBuff;
typedef boost::shared_ptr<CX2EffectSetImpactPointByBuff> CX2EffectSetImpactPointByBuffPtr;
class CX2EffectSetImpactPointByBuff : public CX2BuffIdentity
{
public:
#ifdef SERV_NEW_DEFENCE_DUNGEON // ���볯¥ : 2013-04-09		���� ���� �߰�
	CX2EffectSetImpactPointByBuff( const KBuffIdentity& BuffIdentity_, const WCHAR* pwszEffectSetName_, const WCHAR* pwszSoundName_, const float fCreateGap )
		: CX2BuffIdentity( BuffIdentity_ ), m_wstrEffectSetName( pwszEffectSetName_ ), m_wstrSoundName( pwszSoundName_ ), m_fCreateGap( fCreateGap ), m_fNowCoolTime( 0.f )
		{}
#else // SERV_NEW_DEFENCE_DUNGEON
	CX2EffectSetImpactPointByBuff( const KBuffIdentity& BuffIdentity_, const WCHAR* pwszEffectSetName_, const WCHAR* pwszSoundName_ )
		: CX2BuffIdentity( BuffIdentity_ ), m_wstrEffectSetName( pwszEffectSetName_ ), m_wstrSoundName( pwszSoundName_ )
		{}
#endif // SERV_NEW_DEFENCE_DUNGEON

	void PlayEffectSetImpactPointByBuff( CX2GameUnit* pGameUnit_, const D3DXVECTOR3& vImpactPos_ );
	void PlayEffectSetHittedByBuff( CX2GameUnit* pGameUnit_ );

#ifdef SERV_NEW_DEFENCE_DUNGEON // ���볯¥ : 2013-04-09	
	void PlayEffectSetNormalAttackByBuff( CX2GameUnit* pGameUnit_, const CX2DamageManager::ATTACK_TYPE eAttackType );	/// �Ϲ� ���� Ÿ�ݽ� ����Ʈ �� ���� �Լ�
	void ProcessCoolTime( const float fElapsedTime );		/// ���� ���� ���� �Լ�
#endif // SERV_NEW_DEFENCE_DUNGEON
	
protected:
	inline void PlaySound( CX2GameUnit* pGameUnit_ );

private:
	wstring	m_wstrEffectSetName;
	wstring m_wstrSoundName;
#ifdef SERV_NEW_DEFENCE_DUNGEON // ���볯¥ : 2013-04-09
	const float m_fCreateGap;		/// ���� ����
	float m_fNowCoolTime;			/// ���� ���� ó���� ��Ÿ��
#endif // SERV_NEW_DEFENCE_DUNGEON
};

/** @class : CX2ChangeUnitScaleByBuff
	@brief : ������ Scale�� �����ϴ� ����� BuffIdentity�� �����Ͽ� ������ �� �ְ� �� Ŭ����
	@date : 2012/8/20/
*/
class CX2ChangeUnitScaleByBuff;
typedef boost::shared_ptr<CX2ChangeUnitScaleByBuff> CX2ChangeUnitScaleByBuffPtr;
class CX2ChangeUnitScaleByBuff : public CX2BuffIdentity
{
public:
	CX2ChangeUnitScaleByBuff( const KBuffIdentity& BuffIdentity_, const D3DXVECTOR3& vScale_ )
		: CX2BuffIdentity( BuffIdentity_ ), m_vScale( vScale_ )
	{}

#ifdef VERIFY_STAT_BY_BUFF
	const PROTECT_VECTOR3& GetScale() const { return m_vScale; }
#else	// VERIFY_STAT_BY_BUFF
	const D3DXVECTOR3& GetScale() const { return m_vScale; }
#endif // VERIFY_STAT_BY_BUFF
#ifdef POSSIBLE_ADD_CHANGE_SCALE_BUFF_VALUE
	void	SetScale( const D3DXVECTOR3& vScale_ ) { m_vScale = vScale_; }
	
#ifdef VERIFY_STAT_BY_BUFF
	void	SetScale( const PROTECT_VECTOR3& vScale_ ) { m_vScale = vScale_; }	
#endif // VERIFY_STAT_BY_BUFF

#endif POSSIBLE_ADD_CHANGE_SCALE_BUFF_VALUE

#ifdef VERIFY_STAT_BY_BUFF
	virtual bool Verify() { return m_vScale.Verify(); }
#endif // VERIFY_STAT_BY_BUFF

private:
	
#ifdef VERIFY_STAT_BY_BUFF
	PROTECT_VECTOR3	m_vScale;
#else	// VERIFY_STAT_BY_BUFF
	D3DXVECTOR3 m_vScale;
#endif // VERIFY_STAT_BY_BUFF
};

/** @class : CX2AfterImageByBuff
	@brief : ������ AfterImage�� On �ϴ� ����� BuffIdentity�� ������ Ŭ����
	@date : 2012/8/28/
*/
class CX2AfterImageByBuff;
typedef boost::shared_ptr<CX2AfterImageByBuff> CX2AfterImageByBuffPtr;
class CX2AfterImageByBuff : public CX2BuffIdentity
{
public:
	CX2AfterImageByBuff( const KBuffIdentity& BuffIdentity_, const D3DXCOLOR& d3dxColor_ )
		: CX2BuffIdentity( BuffIdentity_ ), m_d3dxColor( d3dxColor_ )
	{}

	const D3DXCOLOR& GetColor() const { return m_d3dxColor; }

private:
	D3DXCOLOR	m_d3dxColor;
};

#ifdef ADD_DISPLAYER_WEAPON_AFTER_IMAGE
/** @class : CX2WeaponAfterImageByBuff
	@brief : ������ ���� AfterImage�� On �ϴ� ����� BuffIdentity�� ������ Ŭ����
	@date : 2012/8/28/
*/
class CX2WeaponAfterImageByBuff;
typedef boost::shared_ptr<CX2WeaponAfterImageByBuff> CX2WeaponAfterImageByBuffPtr;
class CX2WeaponAfterImageByBuff : public CX2BuffIdentity
{
public:
	CX2WeaponAfterImageByBuff( const KBuffIdentity& BuffIdentity_, const D3DXCOLOR& d3dxColor_ )
		: CX2BuffIdentity( BuffIdentity_ ), m_d3dxColor( d3dxColor_ )
	{}

	const D3DXCOLOR& GetColor() const { return m_d3dxColor; }

private:
	D3DXCOLOR	m_d3dxColor;
};
#endif ADD_DISPLAYER_WEAPON_AFTER_IMAGE

/** @class : CX2CustomFunctionByBuff
	@brief : ������ ���� �ൿ ��CommonStateStart, CommonFrameMove, CommonStateEnd ���� ó���Ǿ� �ϴ� �ϵ��ڵ� �Լ��� �����ϵ��� ó�� �ǵ��� �ϴ� Ŭ����
	@date : 2012/8/21/
*/
class CX2CustomFunctionByBuff;
typedef boost::shared_ptr<CX2CustomFunctionByBuff> CX2CustomFunctionByBuffPtr;
class CX2CustomFunctionByBuff : public CX2BuffIdentity
{
public:
	CX2CustomFunctionByBuff( const KBuffIdentity& BuffIdentity_ )
		: CX2BuffIdentity( BuffIdentity_ )
	{}
	virtual ~CX2CustomFunctionByBuff() {}

	virtual void FunctionInCommonStateStart( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ ) {};
	virtual void FunctionInCommonFrameMove( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ ) {};
	virtual void FunctionInCommonStateEnd( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ ) {};
	virtual void FunctionInDamageReact( CX2GameUnit* pGameUnit_, IN CX2DamageManager::DamageData& damageData_ ) {}
};

/** @class : CX2CustomFunctionByBuffPhantomSword
	@brief : ���Ҽҵ� ���� Ŭ����
	@date : 2012/8/21/
	*/
class CX2CustomFunctionByBuffPhantomSword : public CX2CustomFunctionByBuff
{
public:
	CX2CustomFunctionByBuffPhantomSword( const KBuffIdentity& BuffIdentity_, CX2GameUnit* pGameUnit_ );
	virtual ~CX2CustomFunctionByBuffPhantomSword();

	virtual void FunctionInCommonStateStart( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ );
	virtual void FunctionInCommonFrameMove( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ );
	virtual void FunctionInCommonStateEnd( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ );

private:
	void InitializeDamageEffectThridBlade( CX2GameUnit* pGameUnit_ );
	void InitializeMeshThirdBaldeCenter( CX2GameUnit* pGameUnit_ );
	void CreateAndInsertThirdBladeData( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ );
	void MoveDataDataToDamageEffect( CX2GameUnit* pGameUnit_ );

private:
	struct ThirdBladeMatrix
	{
		D3DXMATRIX		PastCombineMatrix;
		float			fPastTime;
		ThirdBladeMatrix()
		{
			fPastTime	= 0.f;
		}
	};
	typedef boost::shared_ptr<ThirdBladeMatrix> ThirdBladeMatrixPtr;

	struct ThirdBladeDamageData
	{
		CX2DamageManager::DamageData	m_DamageData;
		D3DXVECTOR2						m_vAttackTime;	/// x�� Start, y�� End��
		
		ThirdBladeDamageData() : m_DamageData(), m_vAttackTime( 0.0f, 0.0f )
		{
		}
	};
	typedef boost::shared_ptr<ThirdBladeDamageData> ThirdBladeDamageDataPtr;

	const float MAGIC_PHANTOM_SWORD_DELAY;			/// ���Ҽҵ� ������
	const float MAGIC_PHANTOM_DAMAGE_MULTIPLIER;	/// ���Ҽҵ� 1.2�� ������
	const float MAGIC_PHANTOM_STOP_TIME_DEF;		/// ���Ҽҵ忡 �¾��� �� ��ž�Ǵ� �ð�
	const float MAGIC_MAX_COUNT_CATCH_UP;			/// ���Ҽҵ尡 ��Ÿ�� �ʰ� ���󰡵��� �Ϸ��� ��..��?

	float											m_fThirdBladeTime;				/// ���Ҽҵ� ������� ����� �ð� 
	CX2DamageEffect::CEffect*						m_pDamageEffectThirdBlade;		/// ���Ҽҵ� ����������Ʈ
	CKTDGXMeshPlayer::CXMeshInstanceHandle			m_hThirdBladeCenterMesh;		/// �� �޽� �̹�����...?
	CKTDGXMeshPlayer::CXMeshInstanceHandle			m_hThirdBladeSheathMesh;

 	std::queue<ThirdBladeMatrixPtr>					m_ThirdBladeMatrixQueue;		/// ���Ҽҵ� ������ ��� ť
 	std::list<ThirdBladeDamageDataPtr>				m_ThirdBladeDamageDataList;		/// ���Ҽҵ� ����������Ʈ�� ����� ������ list

	wstring											m_wstrPhantomBladeDamageDataName;	/// ���Ҽҵ� �����������Ͱ� ���� �ִ� ���, ������ ����������Ÿ ���̺� ��
	int												m_iPhantomBladeAttackTimeIndex;		/// ���Ҽҵ尡 ����� ���� Ÿ��
	
	D3DXVECTOR2										m_vPhantomBladeBackSpeed;	/// ���Ҽҵ� �齺�ǵ�
	CX2DamageManager::REACT_TYPE					m_PhantomBladeReActType;	/// ���Ҽҵ��� ReactType
};

/** @class : CX2CustomFunctionByBuffBigBangStream
	@brief : ���Ʈ�� ���� Ŭ����
	@date : 2012/8/30/
	*/
class CX2CustomFunctionByBuffBigBangStream : public CX2CustomFunctionByBuff
{
public:
	CX2CustomFunctionByBuffBigBangStream( const KBuffIdentity& BuffIdentity_, CX2GameUnit* pGameUnit_ )
		: CX2CustomFunctionByBuff( BuffIdentity_ ), m_vStateStartPos( D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) )
	{ m_vecAnimationTime.clear(); }

	~CX2CustomFunctionByBuffBigBangStream()
	{
		m_vecAnimationTime.clear();
		m_vecAnimationTime.swap( vector<float>() );
	}
	virtual void FunctionInCommonStateStart( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ );
	virtual void FunctionInCommonFrameMove( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ );
	
private:
	virtual void DoExtraAttack( CX2GameUnit* pGameUnit_ );

private:
	vector<float>	m_vecAnimationTime;
	D3DXVECTOR3		m_vStateStartPos;
};

/** @class : CX2CustomFunctionByBuffMarkOfCommand
	@brief : ���ְ��� ǥ���� Ŭ����
	@date : 2012/8/30/
	*/
class CX2CustomFunctionByBuffMarkOfCommand : public CX2CustomFunctionByBuff
{
public:
	CX2CustomFunctionByBuffMarkOfCommand( const KBuffIdentity& BuffIdentity_, const KBuffBehaviorFactor kBuffBehaviorFactor_ )
		: CX2CustomFunctionByBuff( BuffIdentity_ ), m_uiDamageEffectCount( 0 ), m_checkElapsedTime( 1.5f )
	{
		m_fBasicPosX[0] = -1000.0f;
		m_fBasicPosX[1] = -900.0f;
		m_fBasicPosX[2] = -1100.0f;
		m_fBasicPosX[3] = -800.0f;
		m_fBasicPosX[4] = -1200.0f;
#ifdef UPGRADE_SKILL_SYSTEM_2013 // û ��ų ����, ������
		/// ������ ����Ʈ�� ����� ������ ����
		m_uiDamageEffectCount = 3;
		m_fAddPowerRate = static_cast<float>( kBuffBehaviorFactor_[1] );
		
		m_uiDamageEffectCount = min( 5, m_uiDamageEffectCount );
		m_fMemoRate = kBuffBehaviorFactor_[0];	/// ������ 0.0�� ���Ե� ���̴�.
#else // UPGRADE_SKILL_SYSTEM_2013 // û ��ų ����, ������
		m_uiDamageEffectCount = static_cast<UINT>( kBuffBehaviorFactor_[0] );
		m_uiDamageEffectCount = min( 5, m_uiDamageEffectCount );
		m_fMemoRate = kBuffBehaviorFactor_[1];	/// ������ 0.0�� ���Ե� ���̴�.
#endif // UPGRADE_SKILL_SYSTEM_2013 // û ��ų ����, ������
	}

	~CX2CustomFunctionByBuffMarkOfCommand()
	{}
	
	virtual void FunctionInCommonFrameMove( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ )
	{
		m_checkElapsedTime.OnFrameMove();
	}

	virtual void FunctionInDamageReact( CX2GameUnit* pGameUnit_, IN CX2DamageManager::DamageData& damageData_ );
	
private:
	void DoMarkOfCommand( CX2GameUnit* pGameUnit_, IN CX2DamageManager::DamageData& damageData_ );

private:
	UINT	m_uiDamageEffectCount;				/// �߻��� ��ź ����
	CKTDXCheckElapsedTime	m_checkElapsedTime;	/// �ƹ��� ���� �̰��� �����δ� �ٽ� �߻����� ����	
	float m_fBasicPosX[5];
	float m_fMemoRate;			/// ���ְ��� ǥ�� �޸� �������� ���� Ȯ��

#ifdef UPGRADE_SKILL_SYSTEM_2013 // û ��ų ����, ������
	float m_fAddPowerRate;
#endif // UPGRADE_SKILL_SYSTEM_2013 // û ��ų ����, ������
};

// /** @class : CX2CustomFunctionByBuffMagicalMakeUp
// 	@brief : �����ø���ũ�� ó�� Ŭ����
// 	@date : 2012/8/23/
// */
// class CX2CustomFunctionByBuffMagicalMakeUp : public CX2CustomFunctionByBuff
// {
// public:
// 	CX2CustomFunctionByBuffMagicalMakeUp( const KBuffIdentity& BuffIdentity_, CX2GameUnit* pGameUnit_ );
// 	virtual ~CX2CustomFunctionByBuffMagicalMakeUp();
// 
// 	virtual void FunctionInCommonStateStart( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ ) {};
// 	virtual void FunctionInCommonFrameMove( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ );
// 	virtual void FunctionInCommonStateEnd( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ ) {};
// 
// private:
// 	const int	MAGICAL_MAKE_UP_WEAPON_ID;
// 	CX2EqipPtr	m_ptrEquipTransform;
// 	bool		m_bNeedToUpdateWeaponEffect;
// 	bool		m_bNotifiedDGManagerChanged;
// };

/** @class : C2StatDataByBuff
	@brief : ������ ���� ���� ������ ���
	@date : 2012/7/23/
*/
class CX2StatFactorByBuff;
typedef boost::shared_ptr<CX2StatFactorByBuff> CX2StatFactorByBuffPtr;
class CX2StatFactorByBuff : public CX2BuffIdentity
{
public:
	CX2StatFactorByBuff( const KBuffIdentity& BuffIdentity_, const float fValue_ )
		: CX2BuffIdentity( BuffIdentity_ ), m_fValue( fValue_ )
	{}

	float GetValue() const { return m_fValue; }
	void SetValue(float val) { m_fValue = val; }

#ifdef VERIFY_STAT_BY_BUFF
	virtual bool Verify() { return m_fValue.Verify(); }
#endif // VERIFY_STAT_BY_BUFF

private:
	KProtectedType<float>		m_fValue;				/// ������ ������ ���� � ���̵� �� �� ����
};

/** @class : CX2AdditionalStatByBuff
	@brief : ������ ���� �߰��Ǵ� ���������� ��� Ŭ����
	@date : 2012/8/6/
*/
class CX2AdditionalStatByBuff
{
public:
	CX2AdditionalStatByBuff() : m_fBasicStatValue( 0.0f ),
		 /*�����׵���� 0 */m_fPassiveSum( 0.0f ), /*�����׵���� 1 */m_fPassiveMultiple( 1.0f ),
		m_fOptionValue( 0.0f ), m_fResultValue( 0.0f ), m_bMustChangeResultValue( false )
	{
		for ( UINT i = 0; i < BCT_END; ++i )
		{
			m_vecStatFactorArray[i].clear();
			m_fArrayResultValueByChangeType[i] = 0.0f;
		}
		m_fArrayResultValueByChangeType[BCT_PERCENT] = 1.0f;
		
		m_bitsetMustChange.reset();		
	}

	~CX2AdditionalStatByBuff()
	{
		for ( UINT i = 0; i < BCT_END; ++i )
		{
			m_vecStatFactorArray[i].clear();
			m_vecStatFactorArray[i].swap( vector<CX2StatFactorByBuffPtr>() );
		}
	}

	float GetBasicStatValue() const { return m_fBasicStatValue; }
	void SetBasicStatValue( const float fValue_ );

	float GetPassiveSum() const { return m_fPassiveSum; }
	void SetPassiveSum( const float fValue_ );

	float GetPassiveMultiple() const { return m_fPassiveMultiple; }
	void SetPassiveMultiple(const float fValue_);

	float GetOptionValue() const { return m_fOptionValue; }
	void SetOptionValue( const float fValue_ );

	float GetResultValue() const { return m_fResultValue; }
	bool GetMustChangeResultValue() const { return m_bMustChangeResultValue; }
	
	void UpdateResultValueByChangeType();

	void UpdateResultValue();			/// �⺻���� ����
	void CheckAndUpdateResultValue();	/// �⺻���� ����

	template<typename Func>
	void UpdateResultValueWithOptionData( Func func_ptr, const int iUserLevel_, const float fModifier_ = 1.0f );	/// Ȯ�彺�� ����

	template<typename Func>
	void CheckAndUpdateResultValueWithOptionData( Func func_ptr, const int iUserLevel_, const float fModifier_ = 1.0f );	/// Ȯ�彺�� ����(�ɼǼ�ġȭ ����)

	void CreateAndInsertStatFactor( const BUFF_CHANGE_TYPE eChangeType_, const KBuffIdentity& BuffIdentity_, const float fStatValue_ );
	bool UpdateAddtionalStatFactor( const BUFF_CHANGE_TYPE eChangeType_, const KBuffIdentity& BuffIdentity_, const float fValue_ );
	void EraseStatFactor( const BUFF_CHANGE_TYPE eChangeType_, const KBuffIdentity& BuffIdentity_ );
	
#ifdef VERIFY_STAT_BY_BUFF
	bool Verify();
#endif // VERIFY_STAT_BY_BUFF

private:
	void SetResultValue(float val) { m_fResultValue = val; }
	void SetMustChangeResultValue(bool val) { m_bMustChangeResultValue = val; }

#ifdef VERIFY_STAT_BY_BUFF
	bool VerifyStatFactor();
	bool VerifyArrayResult();
#endif // VERIFY_STAT_BY_BUFF

private:
	KProtectedType<float>					m_fBasicStatValue;		/// ������ ���� ���޹��� �Ǵ� ��ũ��Ʈ�κ��� ���� ���� �⺻��(�⺻����+���+�нú꽺ų)
	KProtectedType<float>					m_fPassiveSum;			/// �нú꽺ų�� ���� �չ���
	KProtectedType<float>					m_fPassiveMultiple;		/// �нú꽺ų�� ���� ������
	KProtectedType<float>					m_fOptionValue;			/// ���Ͽɼǿ� ���� ��(�ϴ�, ü��, ���ݷ�, ���µ� ���ȿ����� ������� ����)
	vector<CX2StatFactorByBuffPtr>			m_vecStatFactorArray[BCT_END];	/// 0: ������ ġȯ, 1: ������ġ, 2: ������ġ, 3: ����
	KProtectedType<float>					m_fArrayResultValueByChangeType[BCT_END];	/// �� ChangeType �� �����
	
	KProtectedType<float>					m_fResultValue;				/// ��� ChangeType�� ������ �����(���� ����Ʈ���� ������)
	std::bitset<BCT_END>					m_bitsetMustChange;			/// �� ChangeType�� ������� ����Ǿ� �ϴ�������
	bool									m_bMustChangeResultValue;	/// m_fResultValue�� �ٽ� ���ؾ� �ϴ� ��� true

};

/** @function : UpdateResultValueWithOptionData
	@brief : �ɼǼ�ġȭ�� ���õ� ���ȿ� ���� ���� ������� �����ϴ� �Լ�
	@param : �ɼǼ�ġȭ�Լ�������(func_ptr), ���ַ���(iUserLevel_)
*/
template<typename Func>
void CX2AdditionalStatByBuff::UpdateResultValueWithOptionData( Func func_ptr, const int iUserLevel_, const float fModifier_ /*= 1.0f*/ )
{
	if ( !m_vecStatFactorArray[BCT_SWAP_VALUE].empty() )	/// ġȯ��Ұ� �ִٸ�
	{
		/// ������ ġȯ����� ���� ���߿� �ɸ��� ����
		SetResultValue( m_fArrayResultValueByChangeType[BCT_SWAP_VALUE] );
	}
	else	/// ġȯ��Ұ� ���ٸ�
	{
		float fResultValue = GetBasicStatValue();
		bool bShouldMinus = false;
		/// �⺻���� 0.0�ΰ��� �ϴ� 1.0�� �� ���ְ�
		if( !( 0.0f < fResultValue ) )
		{
			fResultValue += 1.0f;
			bShouldMinus = true;
		}

		float fIntermediateValue = GetOptionValue();
		fIntermediateValue += m_fArrayResultValueByChangeType[BCT_FIX_VALUE];
		fIntermediateValue += m_fArrayResultValueByChangeType[BCT_RELATION_VALUE];

		float fMultiplier 
			= ( 1 + func_ptr( fIntermediateValue, iUserLevel_ ) ) * ( 1 + GetPassiveSum() ) 
			* GetPassiveMultiple() * m_fArrayResultValueByChangeType[BCT_PERCENT];
		if ( fMultiplier > 0.0f )
		{
			// kimhc // if ������ fModifier_ ���� �Ǵ��ؼ� �Ʒ� ������ �����ϱ� ���ٴ� �׻� �ϴ°� �� ���� ���̶� �Ǵ���
			fMultiplier -= 1.0f;		/// ���� 1.0�� ���༭ 0.xx ���·� �����. (�������� ���Ѵ�)
			fMultiplier *= fModifier_;	/// �װ��� fModifier_ ���� �����ְ� (�̵�, �����ӵ� ���� ��� 0.5��)
			fMultiplier += 1.0f;

			fResultValue *= fMultiplier;
		}

		if ( bShouldMinus )
			fResultValue -= 1.0f;	/// �ٽ� 1.0�� ���ش�.

		fResultValue = max( fResultValue, 0.0f );	/// Ȥ�� ���̳ʽ��� ��쿡�� �ּҰ��� 0���� ����
		
		SetResultValue( fResultValue );
	}
	SetMustChangeResultValue( false );
}

/** @function : CheckAndUpdateResultValueWithOptionData
	@brief : �ɼǼ�ġȭ�� ������ ���ȿ� ���Ͽ� ����Ǿ����� ���θ� �Ǵ��ϰ� ������ �����ϰ� �ϴ� �Լ�
	@param : �ɼǼ�ġȭ�Լ�������(func_ptr), ���ַ���(iUserLevel_)
*/
template<typename Func>
void CX2AdditionalStatByBuff::CheckAndUpdateResultValueWithOptionData( Func func_ptr, const int iUserLevel_, const float fModifier_ /*= 1.0f*/ )
{
	if ( GetMustChangeResultValue() )
	{
		/// �� Ÿ�Ժ� ����Ʈ ����
		UpdateResultValueByChangeType();

		/// ��ü�� ����
		UpdateResultValueWithOptionData( func_ptr, iUserLevel_, fModifier_ );
	}
}

/** @class : CX2Stun
	@brief : �׷α� ������Ʈ ���� ȿ�� Ŭ����
	@date : 2012/7/22/
*/
class CX2Stun;
typedef boost::shared_ptr<CX2Stun> CX2StunPtr;
class CX2Stun : public CX2BuffIdentity
{
public:
	CX2Stun( const KBuffIdentity& BuffIdentity_ ) 
		: CX2BuffIdentity( BuffIdentity_ )
	{}

protected:
	CX2Stun() {}

private:
};

/** @class : CX2Freeze
	@brief : �׷α� ������Ʈ ���� ȿ�� Ŭ����
	@date : 2012/7/22/
*/
class CX2Freeze;
typedef boost::shared_ptr<CX2Freeze> CX2FreezePtr;
class CX2Freeze : public CX2BuffIdentity
{
public:
	CX2Freeze( const KBuffIdentity& BuffIdentity_ ) 
		: CX2BuffIdentity( BuffIdentity_ )
	{}

protected:
	CX2Freeze() {}

private:
};

/** @class : CX2ChangeAttackByType
	@brief : ���� Ÿ�Կ� ���� ������ Ŭ����
	@date : 2012/11/03/
*/
class CX2ChangeAttackByType;
typedef boost::shared_ptr<CX2ChangeAttackByType> CX2ChangeAttackByTypePtr;
class CX2ChangeAttackByType : public CX2BuffIdentity
{
public:
	CX2ChangeAttackByType( const KBuffIdentity& BuffIdentity_, CX2DamageManager::ATTACK_TYPE eAttackType_, float fValue_ )
		: CX2BuffIdentity( BuffIdentity_ ), m_eAttackType( eAttackType_ ), m_fValue( fValue_ )
	{}

	CX2DamageManager::ATTACK_TYPE GetAttackType() const { return m_eAttackType; }
	float						  GetValue() const { return m_fValue; }

#ifdef VERIFY_STAT_BY_BUFF
	virtual bool	Verify() { return m_fValue.Verify(); }
#endif // VERIFY_STAT_BY_BUFF

private:
	CX2DamageManager::ATTACK_TYPE	m_eAttackType;			///  ���� Ÿ��

#ifdef VERIFY_STAT_BY_BUFF
	KProtectedType<float>			m_fValue;				///  ������ ����ġ
#else
	float							m_fValue;				///  ������ ����ġ
#endif // VERIFY_STAT_BY_BUFF
};

/** @class : CX2ChangeEnchantAttackRate
	@brief : ���� �Ӽ� ���� �ߵ� Ȯ�� Ŭ����
	@date : 2012/11/03/
*/
class CX2ChangeEnchantAttackRate;
typedef boost::shared_ptr<CX2ChangeEnchantAttackRate> CX2ChangeEnchantAttackRatePtr;
class CX2ChangeEnchantAttackRate : public CX2BuffIdentity
{
public:
	CX2ChangeEnchantAttackRate( const KBuffIdentity& BuffIdentity_, float fRate_ )
		: CX2BuffIdentity( BuffIdentity_ ), m_fRate( fRate_ )
	{}

	float			GetRate() const { return m_fRate; }

#ifdef VERIFY_STAT_BY_BUFF
	virtual bool	Verify() { return m_fRate.Verify(); }
#endif // VERIFY_STAT_BY_BUFF

private:

#ifdef VERIFY_STAT_BY_BUFF
	KProtectedType<float> m_fRate;				///  Ȯ��
#else	// VERIFY_STAT_BY_BUFF
	float				 m_fRate;				///  Ȯ��
#endif // VERIFY_STAT_BY_BUFF
};

/** @class : CX2DoubleAttackPtr
	@brief : ���� ���� Ŭ����
	@date : 2012/11/03/
*/
class CX2DoubleAttack;
typedef boost::shared_ptr<CX2DoubleAttack> CX2DoubleAttackPtr;
class CX2DoubleAttack : public CX2BuffIdentity
{
public:
	CX2DoubleAttack( const KBuffIdentity& BuffIdentity_, float fRate_ )
		: CX2BuffIdentity( BuffIdentity_ ), m_fRate( fRate_ )
	{}

	float			GetRate() const { return m_fRate; }

#ifdef VERIFY_STAT_BY_BUFF
	virtual bool	Verify() { return m_fRate.Verify(); }
#endif // VERIFY_STAT_BY_BUFF

private:
	
#ifdef VERIFY_STAT_BY_BUFF
	KProtectedType<float> m_fRate;				///  Ȯ��
#else	// VERIFY_STAT_BY_BUFF
	float				m_fRate;				///	Ȯ��
#endif // VERIFY_STAT_BY_BUFF
};

/** @class : CX2AddSkillLevelByBuffPtr
	@brief : ���� ���� Ŭ����
	@date : 2013/04/16/
*/
class CX2AddSkillLevelByBuff;
typedef boost::shared_ptr<CX2AddSkillLevelByBuff> CX2AddSkillLevelByBuffPtr;
class CX2AddSkillLevelByBuff : public CX2BuffIdentity
{
public:
	CX2AddSkillLevelByBuff( const KBuffIdentity& BuffIdentity_, const int iAddSkillLevel_ )
		: CX2BuffIdentity( BuffIdentity_ ), m_iAddSkillLevel( iAddSkillLevel_ )
	{}

	int				GetAddSkillLevel() const { return m_iAddSkillLevel; }
	void			SetAddSkillLevel(int val) { m_iAddSkillLevel = val; }
	
#ifdef VERIFY_STAT_BY_BUFF
	virtual bool	Verify() { return m_iAddSkillLevel.Verify(); }
#endif // VERIFY_STAT_BY_BUFF

private:
#ifdef VERIFY_STAT_BY_BUFF
	KProtectedType<int> m_iAddSkillLevel;				///  ������ ��ų ����
#else	// VERIFY_STAT_BY_BUFF
	int					m_iAddSkillLevel;				///  ������ ��ų ����
#endif // VERIFY_STAT_BY_BUFF
};

#ifdef SERV_ARA_CHANGE_CLASS_SECOND // ����ȯ

/** @class : CX2ReflectMagicByBuff
	@brief : ���� Ÿ�Կ� ���� ������ Ŭ����
	@date : 2013/07/10/
*/
class CX2ReflectMagicByBuff;
typedef boost::shared_ptr<CX2ReflectMagicByBuff> CX2ReflectMagicByBuffPtr;
class CX2ReflectMagicByBuff : public CX2BuffIdentity
{
public:
	CX2ReflectMagicByBuff( const KBuffIdentity& BuffIdentity_, float fValue_ )
		: CX2BuffIdentity( BuffIdentity_ ), m_fValue( fValue_ )
	{}

	float			GetValue() const { return m_fValue; }
	virtual bool	Verify() { return m_fValue.Verify(); }

private:
	KProtectedType<float>			m_fValue;				/// ������ ����ġ
};

#endif // SERV_ARA_CHANGE_CLASS_SECOND