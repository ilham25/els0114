/** @file : X2BuffTemplet.cpp
    @breif : ������ �ൿ�� ������� ������ �ִ� Ŭ���� ���� ����
*/
#pragma once

#include "StdAfx.h"
#include ".\X2BuffTemplet.h"

/** @function : ParsingScript
	@brief : ���� ���ø� ��ũ��Ʈ�� �о����
	@param : ���� ��ũ��Ʈ�� ��ƸŴ���(luaManager_)
	@return : �Ľ� ���� true, ���� false ����
*/
bool CX2BuffTemplet::ParsingScript( KLuaManager& luaManager_ )
{
	/// ���� ���̵�
	LUA_GET_VALUE_RETURN_ENUM( luaManager_, "BUFF_TEMPLET_ID", m_BuffIdentity.m_eBuffTempletID, 
		BUFF_TEMPLET_ID, BTI_NONE, return DISPLAY_ERROR( L"BUFF_TEMPLET_ID" ) );

	/// ���� ����
	LUA_GET_VALUE_RETURN( luaManager_, "IS_WORLD_BUFF", m_bWorldBuff, false, return DISPLAY_ERROR( L"IS_WORLD_BUFF" ) );

	/// ����, ����� ����
	LUA_GET_VALUE_RETURN( luaManager_, "IS_DEBUF", m_bDeBuff, false, return DISPLAY_ERROR( L"IS_DEBUF" ) );

	/// �ߺ�, ��ü, ��ø ����
	LUA_GET_VALUE_RETURN( luaManager_, "ACCUMULATION_LIMIT", m_uiAccumulationLimit, 0, return DISPLAY_ERROR( L"ACCUMULATION_LIMIT" ) );

	if ( 1 < m_uiAccumulationLimit )	/// ��ø Ƚ���� ������ ������ (0 �� 1�� ��ø�� �ƴ�)
	{
		/// ��ø ������ �о����
		float fAccumulationMultiplier = 0.0f;
		LUA_GET_VALUE_RETURN( luaManager_, "ACCUMULATION_MULTIPLIER", fAccumulationMultiplier, 0, return DISPLAY_ERROR( L"ACCUMULATION_MULTIPLIER" ) );

		m_mapUnCommonMember.insert( make_pair( UCM_ACCUMULATION_MULTIPLIER, fAccumulationMultiplier ) );

		/// ���� ��ø ī��Ʈ
		m_mapUnCommonMember.insert( make_pair( UCM_ACCUMULATION_COUNT_NOW, 0.0f ) );
	}

	/// ���� Ÿ��
	float fRegistType = 0.0f;
	LUA_GET_VALUE_RETURN( luaManager_, "REGIST_TYPE", fRegistType, 0.0f, return DISPLAY_ERROR( L"REGIST_TYPE" ) );

	/// ���� Ÿ���� ET_NONE(0) �� �ƴϸ�
	if ( 0.0f < fRegistType )
		m_mapUnCommonMember.insert( make_pair( UCM_REGIST_TYPE, fRegistType ) );


	/// �ൿ ��� �Ľ�
	if ( luaManager_.BeginTable( "BEHAVIOR" ) )
	{
		BOOST_SCOPE_EXIT( (&luaManager_) ) {
			luaManager_.EndTable();
		} BOOST_SCOPE_EXIT_END

		if ( !CX2BuffBehaviorTemplet::ParsingCombinationBehavior( luaManager_, m_vecBehaviorPtr ) )
			return false;		
	}
	else
		return DISPLAY_ERROR( L"BEHAVIOR Templet doesn't exist" );


	/// ���� ��� �Ľ�
	if ( luaManager_.BeginTable( "FINALIZER" ) )
	{
		BOOST_SCOPE_EXIT( (&luaManager_) ) {
			luaManager_.EndTable();
		} BOOST_SCOPE_EXIT_END

			if ( !CX2BuffFinalizerTemplet::ParsingCombinationFinalizer( luaManager_, m_vecFinalizerPtr ) )
				return false;		
	}
	else
		return DISPLAY_ERROR( L"FINALIZER Templet doesn't exist" );

	/// ���� ��� �Ľ�
	if ( luaManager_.BeginTable( "DISPLAYER" ) )
	{
		BOOST_SCOPE_EXIT( (&luaManager_) ) {
			luaManager_.EndTable();
		} BOOST_SCOPE_EXIT_END

			if ( !CX2BuffDisplayerTemplet::ParsingCombinationDisplayer( luaManager_, m_vecDisplayerPtr ) )
				return false;		
	}
	// Displayer�� ���� ���� �����Ƿ� else �Ǵ��� ���� �ʴ´�.

#ifdef BUFF_ICON_UI
	LUA_GET_VALUE( luaManager_, "BUFF_ICON_TEXTURE_NAME", m_wstIconFileName, L"" );
	m_bUseBuffIcon = ( true == m_wstIconFileName.empty() ) ? false : true ;
	if( true == m_bUseBuffIcon )
	{
		LUA_GET_VALUE( luaManager_, "BUFF_ICON_TEXTURE_KEY",  m_wstIconKeyName,  L"" );
		ASSERT( m_wstIconKeyName != L"" );

		LUA_GET_VALUE( luaManager_, "BUFF_NAME",			  m_iBuffName,		 STR_ID_2674 );
		LUA_GET_VALUE( luaManager_, "BUFF_DESC",			  m_iBuffDesc,		 STR_ID_2606 );
	}
#endif //BUFF_ICON_UI
	return true;
}

/** @function : SetFactor
	@brief : ���� ��Ҹ� �޾Ƽ� ���� ���ø��� �����ϰ�, ������ ���� ��Ű�� �Լ�
	@param : ���� ��� (buffFactor_), ������ ����Ǿ��� ����(pGameUnit_)
	@return : ������ true, ���н� false
*/
bool CX2BuffTemplet::SetFactor( const CX2BuffFactor& buffFactor_, CX2GameUnit* pGameUnit_ )
{
	/// �ൿ���� ����
	BOOST_FOREACH( CX2BuffBehaviorTempletPtr ptrBehavior, m_vecBehaviorPtr )
	{
		if ( NULL != ptrBehavior && ptrBehavior->SetFactor( buffFactor_, pGameUnit_, this ) )
			ptrBehavior->SetStart( true );	/// ���� ����
		else
			return false;
	}

	/// �������� ����
	if ( false == SetFactorToOnlyFinalizer( buffFactor_, pGameUnit_ ) )
		return false;

	if ( GetAccumulationLimit() > 1 )	/// ��ø �Ǵ� ���̶�� ���� �׸���� ���� �� ��ø ī��Ʈ �÷���
		IncreaseAccumulationCountNow();

	/// ǥ������ ����
	BOOST_FOREACH( CX2BuffDisplayerTempletPtr ptrDisplayer, m_vecDisplayerPtr )
	{
		if ( NULL != ptrDisplayer )
		{
			ptrDisplayer->StartDisplayer( pGameUnit_, this );
			ptrDisplayer->SetStart( true );
		}
	}

#ifdef BUFF_ICON_UI
	if( true == GetUseBuffIcon() )
	{
#ifdef SERV_IRUHADEV_BUFF_DURATION_TEXT
		CX2GageUI::BuffIcon BuffIconData( buffFactor_.GetBuffTempletID(), GetIconFileName(),
			GetIconKeyName(), GetBuffName(), GetBuffDesc(), GetRemainDurationTime() );
#else
		CX2GageUI::BuffIcon BuffIconData( buffFactor_.GetBuffTempletID(), GetIconFileName(),
			GetIconKeyName(), GetBuffName(), GetBuffDesc() );
#endif //SERV_IRUHADEV_BUFF_DURATION_TEXT
		CX2GageManager::GetInstance()->PushBuff( pGameUnit_->GetUnitUID(), BuffIconData, IsDeBuff() );
	}
#endif //BUFF_ICON_UI

	return true;
}

/** @function : SetFactorFromPacket
	@brief : ������ ���� ���� ������ҷ� �������ø��� ���� ��Ŵ
	@param : �����κ��� ���� �������(kBuffFactor_), ������ ����� ��������(pGameUnit_)
	@return : ����(true), ����(false)
*/
bool CX2BuffTemplet::SetFactorFromPacket( const KBuffFactor& kBuffFactor_, CX2GameUnit* pGameUnit_ )
{
	SetUniqueNum( kBuffFactor_.m_BuffIdentity.m_uiUniqueNum );

	if ( GetAccumulationLimit() > 1 )	/// ��ø ������ ���̸�
	{
		const float fAccumulationCountNow = static_cast<float>( kBuffFactor_.m_ucAccumulationCountNow );
		
		pair<MAP_UN_COMMON_MEMBER::iterator, bool> pairReturn
			= m_mapUnCommonMember.insert( make_pair( UCM_ACCUMULATION_COUNT_NOW, fAccumulationCountNow ) ); 

		/// ������ Ű���� ���� �ߴٸ�
		if ( !pairReturn.second )
			pairReturn.first->second = fAccumulationCountNow;
	}
	
	/// �ൿ���� ����
	BOOST_FOREACH( CX2BuffBehaviorTempletPtr ptrBehavior, m_vecBehaviorPtr )
	{
		if ( NULL != ptrBehavior && ptrBehavior->SetFactorFromPacketTemplateMothod( kBuffFactor_, pGameUnit_ ) )
			ptrBehavior->SetStart( true );	/// ���� ����
		else
			return false;
	}

	/// �������� ����
	BOOST_FOREACH( CX2BuffFinalizerTempletPtr ptrFinalizer, m_vecFinalizerPtr )
	{
		if ( NULL != ptrFinalizer && !ptrFinalizer->SetFactorFromPacketTemplateMothod( kBuffFactor_, pGameUnit_ ) )
			return false;
	}

	/// ǥ������ ����
	BOOST_FOREACH( CX2BuffDisplayerTempletPtr ptrDisplayer, m_vecDisplayerPtr )
	{
		if ( NULL != ptrDisplayer )
		{
			ptrDisplayer->StartDisplayer( pGameUnit_, this );
			ptrDisplayer->SetStart( true );
		}
	}

	return true;
}

/** @function : GetAccumulationCountNow
	@brief : ���� �� ��ø���� ����
	@return : ���� ��ø Ƚ�� ����(UINT, ��ø�� �ȵǴ� ��� 0�� ����)
*/
UINT CX2BuffTemplet::GetAccumulationCountNow() const
{
	if ( !m_mapUnCommonMember.empty() )
	{
 		MAP_UN_COMMON_MEMBER::const_iterator mItr = m_mapUnCommonMember.find( UCM_ACCUMULATION_COUNT_NOW );
 		if ( m_mapUnCommonMember.end() != mItr )
 			return static_cast<UINT>( mItr->second );
	}

	return 0;
}

/** @function : IncreaseAccumulationCountNow
	@brief : ��ø�� �Ǵ� ��� ��ø Ƚ�� 1����
*/
void CX2BuffTemplet::IncreaseAccumulationCountNow()
{
	if ( !m_mapUnCommonMember.empty() )
	{
		MAP_UN_COMMON_MEMBER::iterator mItr = m_mapUnCommonMember.find( UCM_ACCUMULATION_COUNT_NOW );
		if ( m_mapUnCommonMember.end() != mItr )
			mItr->second += 1;
	}
}

/** @function : GetRegistType
	@brief : ����� ���� Ÿ���� ����
	@return : ����Ÿ��(UINT)���� �ַ� CX2EnchantItem::ENCHANT_TYPE���� ��ȯ�Ͽ� ���
*/
UINT CX2BuffTemplet::GetRegistType() const
{
	if ( !m_mapUnCommonMember.empty() )
	{
		MAP_UN_COMMON_MEMBER::const_iterator mItr = m_mapUnCommonMember.find( UCM_REGIST_TYPE );
		if ( m_mapUnCommonMember.end() != mItr )
			return static_cast<UINT>( mItr->second );
	}

	return 0;
}

/** @function : GetAccumulationMultiflier
	@brief : ��ø ������ ��� ��ø ������ ����
	@return : ��ø ����(float)
*/
float CX2BuffTemplet::GetAccumulationMultiflier() const
{
	if ( !m_mapUnCommonMember.empty() )
	{
		MAP_UN_COMMON_MEMBER::const_iterator mItr = m_mapUnCommonMember.find( UCM_ACCUMULATION_MULTIPLIER );
		if ( m_mapUnCommonMember.end() != mItr )
			return mItr->second;
	}

	DISPLAY_ERROR( L"UCM_ACCUMULATION_COUNT_NOW doesn't exist" );

	return 0.0f;
}

/** @function : OnFrameMoveBehavior
	@brief : �������ø��� �ൿ ��ҵ��� OnFrameMove ����
	@param : ������ ����� ����(pGameUnit_)
*/
void CX2BuffTemplet::OnFrameMoveBehavior( CX2GameUnit* pGameUnit_ )
{
	BOOST_FOREACH( CX2BuffBehaviorTempletPtr ptrBehavior, m_vecBehaviorPtr )
	{
		if ( NULL != ptrBehavior )
			ptrBehavior->OnFrameMove( pGameUnit_, this );
	}
}

/** @function : OnFrameMoveFinalizerAndCheckFinish
	@brief : �������ø��� ���� ��ҵ��� OnFrameMove ���� �� ���Ῡ�θ� �Ǵ�
	@param : ������ ����� ����(pGameUnit_)
*/
void CX2BuffTemplet::OnFrameMoveFinalizerAndCheckFinish( CX2GameUnit* pGameUnit_ )
{
	BOOST_FOREACH( CX2BuffFinalizerTempletPtr ptrFinalizer, m_vecFinalizerPtr )
	{
		if ( NULL != ptrFinalizer )
		{
			ptrFinalizer->OnFrameMove( pGameUnit_ );

			if ( ptrFinalizer->DidFinish( pGameUnit_ ) )
				SetDidFinish( true );
		}
	}
}


/** @function : OnFrameMoveDisplayer
	@brief : �������ø��� ǥ�� ��ҵ��� OnFrameMove ����
	@param : ������ ����� ����(pGameUnit_)
*/
void CX2BuffTemplet::OnFrameMoveDisplayer( CX2GameUnit* pGameUnit_ )
{
	if ( !m_vecDisplayerPtr.empty() )
	{
		BOOST_FOREACH( CX2BuffDisplayerTempletPtr ptrDisplayer, m_vecDisplayerPtr )
		{
			ptrDisplayer->OnFrameMove( pGameUnit_ );
		}
	}
}

/** @function : DoFinishBehavior
	@brief : ���� ����ǰ� �ִ� ������ ����� �� ���ø��� �ൿ��Ұ� ����� �� ����Ǿ��ϴ� �Լ����� ����
	@param : ������ �������� ��������(pGameUnit_)
*/
void CX2BuffTemplet::DoFinishBehavior( CX2GameUnit* pGameUnit_ )
{
	BOOST_FOREACH( CX2BuffBehaviorTempletPtr ptrBehavior, m_vecBehaviorPtr )
	{
		ptrBehavior->DoFinish( pGameUnit_, this );
	}
}

/** @function : DoFinishBehavior
	@brief : ���� ����ǰ� �ִ� ���ø��� �ൿ��Ұ� ����� �� ����Ǿ��ϴ� �Լ����� ����
	@param : ������ �������� ��������(pGameUnit_)
*/
void CX2BuffTemplet::DoFinishDisplayer( CX2GameUnit* pGameUnit_ )
{
	BOOST_FOREACH( CX2BuffDisplayerTempletPtr ptrDisplayer, m_vecDisplayerPtr )
	{
		ptrDisplayer->DoFinish( pGameUnit_, this );
	}
}

/** @function : GetFactor
	@brief : ���� ����ǰ� �ִ� ������ ������ ������ ������ �Լ�(������ ���� ��Ŷ�����ϱ� ����)
	@param : ������ ���� �����(kBuffFactor_), ������ ������ �ִ� ��������(pGameUnit_)
*/
void CX2BuffTemplet::GetFactor( OUT KBuffFactor& kBuffFactor_, const CX2GameUnit* pGameUnit_ ) const
{
	if ( !GetDidFinish() )
	{
		BOOST_FOREACH( CX2BuffBehaviorTempletPtr ptr, m_vecBehaviorPtr )
		{
			ptr->GetFactor( kBuffFactor_.m_vecBuffBehaviorFactors );
		}

		BOOST_FOREACH( CX2BuffFinalizerTempletPtr ptr, m_vecFinalizerPtr )
		{
			ptr->GetFactor( kBuffFactor_.m_vecBuffFinalizerFactors, pGameUnit_ );
		}

		/// ǥ�������� �����ڿ��� ������ �ʿ䰡 �����Ƿ� �������� �ʴ´�.(m_vecDisplayerPtr)

		kBuffFactor_.m_BuffIdentity = m_BuffIdentity;

		if ( GetAccumulationLimit() > 1 )
		{
			kBuffFactor_.m_fAccumulationMultiplier = GetAccumulationMultiflier();
			kBuffFactor_.m_ucAccumulationCountNow = GetAccumulationCountNow();
		}
	}
}

/** @function : SetFactorToOnlyFinalizer
	@brief : �������ǿ��� ���� ����(��øī��Ʈ�� �ִ��� ���Ŀ��� ���������� ��� ��ø�� ���ؼ� ���ŵǵ��� �ϱ� ���� ���)
	@param : ���� ��� (buffFactor_), ������ ����Ǿ��� ����(pGameUnit_)
	@return : ������ true, ���н� false
*/
bool CX2BuffTemplet::SetFactorToOnlyFinalizer( const CX2BuffFactor& buffFactor_, CX2GameUnit* pGameUnit_ )
{
	/// �������� ����
	BOOST_FOREACH( CX2BuffFinalizerTempletPtr ptrFinalizer, m_vecFinalizerPtr )
	{
		if ( !ptrFinalizer->SetFactor( buffFactor_, pGameUnit_, this ) )
			return false;
	}
	return true;
}

/** @function : GetFinalizerTempletPtrList
	@brief : �������ǵ��� ������ �Լ�
	@param : ���� ���������� ���� �Լ�
*/
void CX2BuffTemplet::GetFinalizerTempletPtrList( OUT vector<CX2BuffFinalizerTempletPtr>& vecFinalizerPtr_ )
{
	vecFinalizerPtr_.reserve( m_vecFinalizerPtr.size() );
	vecFinalizerPtr_ = m_vecFinalizerPtr;
}

/** @function : SetFinalizerTempletPtrList
	@brief : �������ǵ��� ���� �ϴ� �Լ�
	@param : ������ ��������
*/
void CX2BuffTemplet::ChangeFinalizerTempletPtrList( const vector<CX2BuffFinalizerTempletPtr>& vecFinalizerPtr_ )
{
	m_vecFinalizerPtr.resize( vecFinalizerPtr_.size() );
	m_vecFinalizerPtr = vecFinalizerPtr_;
}

#ifdef SERV_IRUHADEV_BUFF_DURATION_TEXT
//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-08-25
// Description: Remaining seconds of this buff's BFT_TIME finalizer, if any
float CX2BuffTemplet::GetRemainDurationTime() const
{
	BOOST_FOREACH( CX2BuffFinalizerTempletPtr ptrFinalizer, m_vecFinalizerPtr )
	{
		if ( NULL != ptrFinalizer && BFT_TIME == ptrFinalizer->GetType() )
			return ptrFinalizer->GetRemainTime();
	}
	return -1.f;
}
//////////////////////////////////////////////////////////////////////////
#endif //SERV_IRUHADEV_BUFF_DURATION_TEXT

/** @function : ReserveToFinish
	@brief : ���� üũ�� ����ǵ��� ���� �س��� �Լ�
*/
void CX2BuffTemplet::ReserveToFinish()
{
	/// �������� ����
	BOOST_FOREACH( CX2BuffFinalizerTempletPtr ptrFinalizer, m_vecFinalizerPtr )
	{
		if ( NULL != ptrFinalizer )
			ptrFinalizer->ReserveToFinish();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/** @function : SetBasicStatValue
	@brief :������ �⺻���� �����ϴ� �Լ�(��ũ��Ʈ�� �Ǵ� ���� �������� ������ �� ��)
	@param : ������ ��(fValue_)
*/
void CX2AdditionalStatByBuff::SetBasicStatValue( const float fValue_ )
{ 
	if ( !IsSamef( GetBasicStatValue(), fValue_ ) )
	{
		m_fBasicStatValue = fValue_;
		SetMustChangeResultValue( true ); 
	}
}

/** @function : SetOptionValue
	@brief :������ ���� �ɼǿ� ���� ���� �����ϴ� �Լ�
	@param : ������ ��(fValue_)
*/
void CX2AdditionalStatByBuff::SetOptionValue( const float fValue_ )
{
	if ( !IsSamef( GetOptionValue(), fValue_ ) )
	{
		m_fOptionValue = fValue_;
		SetMustChangeResultValue( true ); 
	}
}

/** @function : SetPassiveSum
	@brief : ������ �ø��� �нú� �� 0.0�� �������� ���ҽÿ��� ���̳ʽ�(-), �����ÿ��� (+) ���� ��Ҹ� �����ϴ� �Լ�
	@param : ������ ��(fValue_)
*/
void CX2AdditionalStatByBuff::SetPassiveSum( const float fValue_ )
{
	if ( !IsSamef( GetPassiveSum(), fValue_ ) )
	{
		m_fPassiveSum = fValue_;
		SetMustChangeResultValue( true ); 
	}
}

/** @function : SetPassiveMultiple
	@brief : ������ �ø��� �нú� �� 1.0�� �������� ���ҽÿ��� 0.0~1.0, �����ÿ��� 1.0 ~ ���� ��Ҹ� ����
	@param : ������ ��(fValue_)
*/
void CX2AdditionalStatByBuff::SetPassiveMultiple( const float fValue_ )
{
	if ( !IsSamef( GetPassiveMultiple(), fValue_ ) )
	{
		m_fPassiveMultiple = fValue_;
		SetMustChangeResultValue( true ); 
	}
}

/** @function : UpdateResultValueByChangeType
	@brief : ���� Ÿ�Ժ�(ġȯ, ������ġ, ������ġ, %) ����� ����
*/
void CX2AdditionalStatByBuff::UpdateResultValueByChangeType()
{
	if ( m_bitsetMustChange.any() )	/// ����� ���� �ִٸ� ����
	{
		if ( m_bitsetMustChange.test( BCT_SWAP_VALUE ) )
		{
			/// ������ ġȯ����� ���� ���߿� �ɸ��� ����
			if ( m_vecStatFactorArray[BCT_SWAP_VALUE].empty() )
				m_fArrayResultValueByChangeType[BCT_SWAP_VALUE] = 0.0f;
			else
			{
				CX2StatFactorByBuffPtr& ptrStatFactor = m_vecStatFactorArray[BCT_SWAP_VALUE].back();
				m_fArrayResultValueByChangeType[BCT_SWAP_VALUE] = ptrStatFactor->GetValue();
			}
		}			

		if ( m_bitsetMustChange.test( BCT_FIX_VALUE ) )
		{
			float fResultValueByChangeType = 0.0f;

			auto FuncPlus = [&fResultValueByChangeType]( CX2StatFactorByBuffPtr ptrStatFactor_ ) {
				fResultValueByChangeType += ptrStatFactor_->GetValue();
			};

			if ( !m_vecStatFactorArray[BCT_FIX_VALUE].empty() )
				for_each( m_vecStatFactorArray[BCT_FIX_VALUE].begin(), m_vecStatFactorArray[BCT_FIX_VALUE].end(), FuncPlus );

			m_fArrayResultValueByChangeType[BCT_FIX_VALUE] = fResultValueByChangeType;
		}

		if ( m_bitsetMustChange.test( BCT_RELATION_VALUE ) )
		{
			float fResultValueByChangeType = 0.0f;

			auto FuncPlus = [&fResultValueByChangeType]( CX2StatFactorByBuffPtr ptrStatFactor_ ) {
				fResultValueByChangeType += ptrStatFactor_->GetValue();
			};

			if ( !m_vecStatFactorArray[BCT_RELATION_VALUE].empty() )
				for_each( m_vecStatFactorArray[BCT_RELATION_VALUE].begin(), m_vecStatFactorArray[BCT_RELATION_VALUE].end(), FuncPlus );

			m_fArrayResultValueByChangeType[BCT_RELATION_VALUE] = fResultValueByChangeType;
		}

		if ( m_bitsetMustChange.test( BCT_PERCENT ) )
		{
			float fResultValueByChangeType = 1.0f;

			auto FuncMultiplies = [&fResultValueByChangeType]( CX2StatFactorByBuffPtr ptrStatFactor_ ) {
				fResultValueByChangeType *= ptrStatFactor_->GetValue();
			};

			if ( !m_vecStatFactorArray[BCT_PERCENT].empty() )
				for_each( m_vecStatFactorArray[BCT_PERCENT].begin(), m_vecStatFactorArray[BCT_PERCENT].end(), FuncMultiplies );

			m_fArrayResultValueByChangeType[BCT_PERCENT] = fResultValueByChangeType;
		}

		m_bitsetMustChange.reset();	/// ������ ���� �����Ƿ� �ʱ�ȭ
	}		
}

/** @function : UpdateResultValue
	@brief : �ɼǼ�ġȭ�� ������� �Ǵ� �⺻���� �����ϴ� ������ ������� ������Ʈ �ϴ� �Լ�
*/
void CX2AdditionalStatByBuff::UpdateResultValue()
{
	if ( !m_vecStatFactorArray[BCT_SWAP_VALUE].empty() )	/// ġȯ��Ұ� �ִٸ�
	{
		/// ������ ġȯ����� ���� ���߿� �ɸ��� ����
		SetResultValue( m_fArrayResultValueByChangeType[BCT_SWAP_VALUE] );
	}
	else	/// ġȯ��Ұ� ���ٸ�
	{
		float fResultValue = GetBasicStatValue();
		fResultValue += GetOptionValue();
		fResultValue += m_fArrayResultValueByChangeType[BCT_FIX_VALUE];
		fResultValue += m_fArrayResultValueByChangeType[BCT_RELATION_VALUE];

		/// �нú꽺ų�� �չ����� ������ �� ������ percent ������ ��� ����
		const float fMultiplier 
			= ( 1 + GetPassiveSum() ) * GetPassiveMultiple() * m_fArrayResultValueByChangeType[BCT_PERCENT];
		if ( fMultiplier > 0.0f )	/// ȿ���� ���ҵǴ� ��쿡�� 0���� ũ�� 1.0���� ����, �����Ǵ� ��� 1.0���� ŭ
			fResultValue += GetBasicStatValue() * (fMultiplier - 1.0f);
#ifdef _IN_HOUSE_
		else
		{
			// ������ // 2013-08-16
			// 0���� ������� �� �߻��� �� �ִ� ���� ���׿� ���� 
			// ����ó���� ���� ��ȹ������ ��� �� �� ������ �����߽��ϴ�.
			// ex ) �ν������� ��� Ư�� ������ ������ �������� 1�� �ֱ� ����, 
			//		������ ���� ��Ƽ�. 90% ���Ҹ� ���ѵ� 1�� �ǰ� �Ǵµ�,
			//		100% ���ð� �߻��ϸ� �� ���� ���� �������� �� �� �ֱ� ������ ����.
			DISPLAY_ERROR(L"BattleFieldBuffFactor.lua ���� ����.\nPERCENT ���� 0���� ���� �� �� �����ϴ�.");
		}
#endif // _IN_HOUSE_

		SetResultValue( fResultValue );
	}
	SetMustChangeResultValue( false );
}

/** @function : UpdateResultValue
	@brief : �ɼǼ�ġȭ�� ������� �Ǵ� �⺻���� �����ϴ� ������ ����Ǿ������� �˻��ϰ�, ������� ������Ʈ �ϴ� �Լ�
*/
void CX2AdditionalStatByBuff::CheckAndUpdateResultValue()
{ 
	if ( GetMustChangeResultValue() )
	{
		/// �� Ÿ�Ժ� ����Ʈ ����
		UpdateResultValueByChangeType();

		/// ��ü�� ����
		UpdateResultValue();
	}
}

/** @function : CreateAndInsertStatFactor
	@brief : ������ ���� �����Ǵ� ��Ҹ� ����� ������
	@param : ����, ���� ��ġ�� ����Ÿ��(eChangeType_), � ������ ���� ��ȭ���� ����(BuffIdentity_), ���Ȱ�(fStatValue_)
*/
void CX2AdditionalStatByBuff::CreateAndInsertStatFactor( const BUFF_CHANGE_TYPE eChangeType_, const KBuffIdentity& BuffIdentity_, const float fStatValue_ )
{
	UINT uiIndex = eChangeType_;
	if ( uiIndex < ARRAY_SIZE( m_vecStatFactorArray ) )
	{
		CX2StatFactorByBuffPtr ptrStatFActor 
			= CX2StatFactorByBuffPtr( new CX2StatFactorByBuff( BuffIdentity_, fStatValue_ ) );

		m_vecStatFactorArray[eChangeType_].push_back( ptrStatFActor );
		m_bitsetMustChange.set( eChangeType_ );
		SetMustChangeResultValue( true );
	}				
}

/** @function : UpdateAddtionalStatFactor
	@brief : ������ Identity�� ��ġ�ϴ� ���� ã�Ƽ� ���� ����
	@param : ����, ���� ��ġ�� ����Ÿ��(eChangeType_), � ������ ���� ��ȭ���� ����(BuffIdentity_), ���Ⱥ��氪(fStatValue_)
	@return: ����(true), ����(false) �� �����ϸ� �ܺο��� Create ����
*/
bool CX2AdditionalStatByBuff::UpdateAddtionalStatFactor( const BUFF_CHANGE_TYPE eChangeType_, const KBuffIdentity& BuffIdentity_, const float fValue_ )
{
	if ( !m_vecStatFactorArray[eChangeType_].empty() )
	{
		vector<CX2StatFactorByBuffPtr>::iterator vItr
			= FindSameBuffIdentity<vector<CX2StatFactorByBuffPtr>::iterator, CX2StatFactorByBuffPtr>( m_vecStatFactorArray[eChangeType_], BuffIdentity_ );

		if ( m_vecStatFactorArray[eChangeType_].end() != vItr )
		{
			(*vItr)->SetValue( fValue_ );
			m_bitsetMustChange.set( eChangeType_ );
			SetMustChangeResultValue( true );
			return true;
		}
	}

	return false;
}

/** @function : EraseStatFactor
	@brief :������ Identity�� ��ġ�ϴ� ���� ã�Ƽ� ȿ�� ����
	@param : ����, ���� ��ġ�� ����Ÿ��(eChangeType_), � ������ ���� ��ȭ���� ����(BuffIdentity_)
*/
void CX2AdditionalStatByBuff::EraseStatFactor( const BUFF_CHANGE_TYPE eChangeType_, const KBuffIdentity& BuffIdentity_ )
{
	UINT uiIndex = eChangeType_;
	if ( uiIndex < ARRAY_SIZE( m_vecStatFactorArray ) )
	{
		vector<CX2StatFactorByBuffPtr>::iterator vItr
			= FindSameBuffIdentity<vector<CX2StatFactorByBuffPtr>::iterator, CX2StatFactorByBuffPtr>( m_vecStatFactorArray[uiIndex], BuffIdentity_ );

		if ( m_vecStatFactorArray[uiIndex].end() != vItr )
		{
			m_vecStatFactorArray[uiIndex].erase( vItr );
			m_bitsetMustChange.set( eChangeType_ );
			SetMustChangeResultValue( true );
		}
	}				
}
#ifdef VERIFY_STAT_BY_BUFF
bool CX2AdditionalStatByBuff::Verify()
{
	if ( m_fBasicStatValue.Verify() &&
		m_fPassiveSum.Verify() &&
		m_fPassiveMultiple.Verify() &&
		m_fOptionValue.Verify() &&
		m_fResultValue.Verify() &&
		VerifyStatFactor() &&
		VerifyArrayResult() )
		return true;
	else
		return false;
}

bool CX2AdditionalStatByBuff::VerifyStatFactor()
{
	for ( int i = 0; i < BCT_END; i++ )
	{
		BOOST_FOREACH( CX2StatFactorByBuffPtr& ptrStatFactor, m_vecStatFactorArray[i] )
		{
			if ( NULL != ptrStatFactor && false == ptrStatFactor->Verify() )
				return false;
		}
	}

	return true;
}

bool CX2AdditionalStatByBuff::VerifyArrayResult()
{
	for ( int i = 0; i < BCT_END; i++ )
	{
		if ( false == m_fArrayResultValueByChangeType[i].Verify() )
			return false;
	}

	return true;
}

#endif // VERIFY_STAT_BY_BUFF

/** @function : PlayEffectSetImpactPointByBuff
	@brief : Ÿ�� ��ġ�� ������ ����Ʈ�� ���
	@param : ����Ʈ���� �����Ű�� ��������(pGameUnit_), Ÿ����ġ(vImpactPos_)
*/
void CX2EffectSetImpactPointByBuff::PlayEffectSetImpactPointByBuff( CX2GameUnit* pGameUnit_, const D3DXVECTOR3& vImpactPos_ )
{
	if ( NULL != g_pX2Game )
	{
#ifdef SERV_NEW_DEFENCE_DUNGEON // ���볯¥ : 2013-04-09
		if ( m_fNowCoolTime >= m_fCreateGap )		/// ���� ������ ������ ��, ���� ó��
		{
			g_pX2Game->GetEffectSet()->PlayEffectSet( m_wstrEffectSetName, pGameUnit_, 
				NULL, ( 0.0f < pGameUnit_->GetRemainHyperModeTime() ), pGameUnit_->GetPowerRate(), -1.f, D3DXVECTOR3( 1.0f, 1.0f, 1.0f ), 
				true, vImpactPos_ );
			PlaySound( pGameUnit_ );

			m_fNowCoolTime = 0.f;		/// ���� ���� �ʱ�ȭ
		}
#else // SERV_NEW_DEFENCE_DUNGEON
		g_pX2Game->GetEffectSet()->PlayEffectSet( m_wstrEffectSetName, pGameUnit_, 
			NULL, ( 0.0f < pGameUnit_->GetRemainHyperModeTime() ), pGameUnit_->GetPowerRate(), -1.f, D3DXVECTOR3( 1.0f, 1.0f, 1.0f ), 
			true, vImpactPos_ );
		PlaySound( pGameUnit_ );
#endif // SERV_NEW_DEFENCE_DUNGEON
	}
}

/** @function : PlayEffectSetHittedByBuff
	@brief : �ǰݽ� ������ ����Ʈ�� ���
	@param : ����Ʈ���� �����Ű�� ��������(pGameUnit_)
*/
void CX2EffectSetImpactPointByBuff::PlayEffectSetHittedByBuff( CX2GameUnit* pGameUnit_ )
{
	if ( NULL != g_pX2Game )
	{
#ifdef SERV_NEW_DEFENCE_DUNGEON // ���볯¥ : 2013-04-09
		if ( m_fNowCoolTime >= m_fCreateGap )		/// ���� ������ ������ ��, ���� ó��
		{
			g_pX2Game->GetEffectSet()->PlayEffectSet( m_wstrEffectSetName, pGameUnit_,
			NULL, ( 0.0f < pGameUnit_->GetRemainHyperModeTime() ), pGameUnit_->GetPowerRate() );
			PlaySound( pGameUnit_ );

			m_fNowCoolTime = 0.f;	/// ���� ���� �ʱ�ȭ
		}
#else // SERV_NEW_DEFENCE_DUNGEON
		g_pX2Game->GetEffectSet()->PlayEffectSet( m_wstrEffectSetName, pGameUnit_,
			NULL, ( 0.0f < pGameUnit_->GetRemainHyperModeTime() ), pGameUnit_->GetPowerRate() );
		PlaySound( pGameUnit_ );
#endif // SERV_NEW_DEFENCE_DUNGEON
	}
}

/** @function : PlaySound
	@brief : ���� ���
	@param : ���带 �����Ű�� ��������(pGameUnit_)
*/
void CX2EffectSetImpactPointByBuff::PlaySound( CX2GameUnit* pGameUnit_ )
{
	if ( !m_wstrSoundName.empty() )
		pGameUnit_->PlaySound( m_wstrSoundName.c_str() );
}

#ifdef SERV_NEW_DEFENCE_DUNGEON // ���볯¥ : 2013-04-09

/** @function : PlayEffectSetNormalAttackByBuff
	@brief : �Ϲ� ���� Ÿ�ݽ� ����Ʈ�� ����
	@param : ����Ʈ���� �����Ű�� ��������(pGameUnit_), Ÿ����ġ(vImpactPos_)
*/
void CX2EffectSetImpactPointByBuff::PlayEffectSetNormalAttackByBuff( CX2GameUnit* pGameUnit_, const CX2DamageManager::ATTACK_TYPE eAttackType )
{
	if ( NULL != g_pX2Game )
	{
		if ( m_fNowCoolTime >= m_fCreateGap && CX2DamageManager::AT_NORMAL == eAttackType )		/// ���� ������ ������, �Ϲ� �����϶� ���� ó��
		{
			g_pX2Game->GetEffectSet()->PlayEffectSet( m_wstrEffectSetName, pGameUnit_, 
				NULL, ( 0.0f < pGameUnit_->GetRemainHyperModeTime() ), pGameUnit_->GetPowerRate(), -1.f, D3DXVECTOR3( 1.0f, 1.0f, 1.0f ) );
			PlaySound( pGameUnit_ );

			m_fNowCoolTime = 0.f;		/// ���� ���� �ʱ�ȭ
		}
	}
}

/** @function : ProcessCoolTime
	@brief : ��Ÿ�� ���� �Լ�
	@param : �����Ӵ� �ð�
*/
void CX2EffectSetImpactPointByBuff::ProcessCoolTime( float fElapsedTime )
{
	if ( m_fNowCoolTime < m_fCreateGap )
		m_fNowCoolTime += fElapsedTime;
}
#endif // SERV_NEW_DEFENCE_DUNGEON

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

/** @function : CX2CustomFunctionByBuffPhantomSword
	@brief : CX2CustomFunctionByBuffPhantomSword �� ������
*/
CX2CustomFunctionByBuffPhantomSword::CX2CustomFunctionByBuffPhantomSword( const KBuffIdentity& BuffIdentity_, CX2GameUnit* pGameUnit_ ) 
	: CX2CustomFunctionByBuff( BuffIdentity_ )
	, MAGIC_PHANTOM_SWORD_DELAY( 0.2f )
	, MAGIC_PHANTOM_DAMAGE_MULTIPLIER( 1.2f )
	, MAGIC_PHANTOM_STOP_TIME_DEF( 0.033f )
	, MAGIC_MAX_COUNT_CATCH_UP( 2 )
	, m_fThirdBladeTime( 0.0f )
	, m_pDamageEffectThirdBlade( NULL )
	, m_hThirdBladeCenterMesh( INVALID_MESH_INSTANCE_HANDLE )
	, m_hThirdBladeSheathMesh( INVALID_MESH_INSTANCE_HANDLE )
	, m_wstrPhantomBladeDamageDataName()
	, m_iPhantomBladeAttackTimeIndex( -1 )
	, m_vPhantomBladeBackSpeed( 0.0f, 0.0f )
	, m_PhantomBladeReActType( CX2DamageManager::RT_NO_DAMAGE )
{
	InitializeDamageEffectThridBlade( pGameUnit_ );
	InitializeMeshThirdBaldeCenter( pGameUnit_ );	
}



/** @function : ~CX2CustomFunctionByBuffPhantomSword
	@brief : CX2CustomFunctionByBuffPhantomSword�� �Ҹ���
*/
/*virtual*/ CX2CustomFunctionByBuffPhantomSword::~CX2CustomFunctionByBuffPhantomSword()
{
	if ( NULL != g_pX2Game )
	{
		g_pX2Game->GetMajorXMeshPlayer()->DestroyInstance( m_hThirdBladeCenterMesh );
		g_pX2Game->GetDamageEffect()->DestroyInstance( m_pDamageEffectThirdBlade );
		m_pDamageEffectThirdBlade = NULL;
	}

	m_ThirdBladeMatrixQueue.swap( std::queue<ThirdBladeMatrixPtr>() );
	m_ThirdBladeDamageDataList.swap( std::list<ThirdBladeDamageDataPtr>() );
}

/** @function : FunctionInCommonStateStart
	@brief : CommonStateStart ���� ����� �Լ�
	@param : ��������(pGameUnit_), ��ƸŴ���(luaManager_)
*/
/*virtual*/ void CX2CustomFunctionByBuffPhantomSword::FunctionInCommonStateStart( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ )
{
	LUA_GET_VALUE(	luaManager_, "PHANTOM_BLADE_DAMAGE_DATA_NAME",		m_wstrPhantomBladeDamageDataName,	L"" );
	LUA_GET_VALUE(	luaManager_, "PHANTOM_BLADE_ATTACK_TIME_INDEX",		m_iPhantomBladeAttackTimeIndex,		-1 );

	LUA_GET_VALUE(	luaManager_, "PHANTOM_BLADE_BACK_SPEED_X",			m_vPhantomBladeBackSpeed.x,			0.f );
	LUA_GET_VALUE(	luaManager_, "PHANTOM_BLADE_BACK_SPEED_Y",			m_vPhantomBladeBackSpeed.y,			0.f );

	LUA_GET_VALUE_ENUM( luaManager_, "PHANTOM_BLADE_REACT_TYPE",		m_PhantomBladeReActType,			CX2DamageManager::REACT_TYPE,		CX2DamageManager::RT_NO_DAMAGE );

	
	if ( pGameUnit_->GetSizeOfAttackTimeList() > 0 && -1 != m_iPhantomBladeAttackTimeIndex )
	{
		if ( NULL != m_pDamageEffectThirdBlade )
		{
			CKTDGXMeshPlayer::CXMeshInstance* pMeshInst = m_pDamageEffectThirdBlade->GetMainEffect();
			if( NULL != pMeshInst )			
			{
				CreateAndInsertThirdBladeData( pGameUnit_, luaManager_ );
				if ( m_ThirdBladeDamageDataList.size() == 1 )	/// kimhc // �̰� �� �̷��� ������..?
					MoveDataDataToDamageEffect( pGameUnit_ );
			}	// if( NULL != pMeshInst )			
		}	// if ( NULL != m_pDamageEffectThirdBlade )
	}
}

/** @function : FunctionInCommonFrameMove
	@brief : CommonFrameMove ���� ����� �Լ�
	@param : ��������(pGameUnit_), ��ƸŴ���(luaManager_)
*/
/*virtual*/ void CX2CustomFunctionByBuffPhantomSword::FunctionInCommonFrameMove( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ )
{
	if ( NULL != m_pDamageEffectThirdBlade )
	{
		CKTDGXMeshPlayer::CXMeshInstance* pMeshInst = m_pDamageEffectThirdBlade->GetMainEffect();
		CKTDGXMeshPlayer::CXMeshInstance* pMeshInstCenter = g_pX2Game->GetMajorXMeshPlayer()->GetMeshInstance( m_hThirdBladeCenterMesh );

		if ( NULL != pMeshInst && NULL != pMeshInstCenter )
		{
			m_fThirdBladeTime = pMeshInst->GetGlobalTime();
			ThirdBladeMatrixPtr	ptrThirdBladeMatrix( new ThirdBladeMatrix );
			if ( NULL != ptrThirdBladeMatrix )
			{
				const D3DXMATRIX* pMatrixBone =	pGameUnit_->GetCombineMatrixFromBoneName( L"Dummy1_Rhand" );
				ptrThirdBladeMatrix->PastCombineMatrix = *pMatrixBone;
				ptrThirdBladeMatrix->fPastTime = static_cast<float>( g_pKTDXApp->GetTime() ); 
				m_ThirdBladeMatrixQueue.push( ptrThirdBladeMatrix );

				const float fDelayedTime = static_cast<float>( g_pKTDXApp->GetTime() ) - MAGIC_PHANTOM_SWORD_DELAY;
				ThirdBladeMatrixPtr ptrFirstThirdBladeMatrix = m_ThirdBladeMatrixQueue.front();
				
				for ( int i = 0; i < MAGIC_MAX_COUNT_CATCH_UP; ++i )
				{
					if ( ptrFirstThirdBladeMatrix->fPastTime > fDelayedTime )
						break;
					
					m_ThirdBladeMatrixQueue.pop();
					if ( !m_ThirdBladeMatrixQueue.empty() )
						ptrFirstThirdBladeMatrix = m_ThirdBladeMatrixQueue.front();
				}
				pMeshInst->SetUseDXMatrix( ptrFirstThirdBladeMatrix->PastCombineMatrix );
				pMeshInstCenter->SetUseDXMatrix( ptrFirstThirdBladeMatrix->PastCombineMatrix );
				
				const D3DXVECTOR3& vPos = pGameUnit_->GetPos();
				pMeshInstCenter->SetPos( vPos );
				pMeshInst->SetPos( vPos );
			}
		}
		else
			m_fThirdBladeTime = 0;

		if ( !m_ThirdBladeDamageDataList.empty() )
		{
			ThirdBladeDamageDataPtr ptrThirdBladeDamageData = m_ThirdBladeDamageDataList.front();

			if ( ptrThirdBladeDamageData->m_vAttackTime.y < m_fThirdBladeTime  )		
			{
				m_ThirdBladeDamageDataList.pop_front();
				if ( !m_ThirdBladeDamageDataList.empty() )
				{
					if ( NULL != m_pDamageEffectThirdBlade )
						MoveDataDataToDamageEffect( pGameUnit_ );
				}
			}
		}
	}
}

/** @function : FunctionInCommonStateEnd
	@brief : CommonStateEnd ���� ����� �Լ�
	@param : ��������(pGameUnit_), ��ƸŴ���(luaManager_)
*/
/*virtual*/ void CX2CustomFunctionByBuffPhantomSword::FunctionInCommonStateEnd( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ )
{
	if ( pGameUnit_->GetSizeOfAttackTimeList() > 0 )
	{
		if ( NULL != m_pDamageEffectThirdBlade )
		{
			CKTDGXMeshPlayer::CXMeshInstance* pMeshInst = m_pDamageEffectThirdBlade->GetMainEffect();
			if ( NULL != pMeshInst && !m_ThirdBladeDamageDataList.empty() )
			{
				ThirdBladeDamageDataPtr ptrThirdBladeDamageDataBack	= m_ThirdBladeDamageDataList.back();
				const float fInterruptedAttackTime = m_fThirdBladeTime + MAGIC_PHANTOM_SWORD_DELAY;

				if ( fInterruptedAttackTime < ptrThirdBladeDamageDataBack->m_vAttackTime.y )
					ptrThirdBladeDamageDataBack->m_vAttackTime.y = fInterruptedAttackTime;

				if ( m_ThirdBladeDamageDataList.size() == 1 )
				{
					ThirdBladeDamageDataPtr ptrThirdBladeDamageDataFront 
						= m_ThirdBladeDamageDataList.front();
					m_pDamageEffectThirdBlade->SetAttackTime( ptrThirdBladeDamageDataFront->m_vAttackTime );
				}
			}
		}
	}
}

/** @function : InitializeDamageEffectThridBlade
	@brief : ���Ҽҵ��� ������ ����Ʈ ����
	@param : ���Ҽҵ带 ����ϴ� ��������(pGameUnit_)
*/
void CX2CustomFunctionByBuffPhantomSword::InitializeDamageEffectThridBlade( CX2GameUnit* pGameUnit_ )
{
	m_pDamageEffectThirdBlade
		= g_pX2Game->GetDamageEffect()->CreateInstance( pGameUnit_, L"THIRD_BLADE", 
		pGameUnit_->GetPowerRate(), pGameUnit_->GetPos(), pGameUnit_->GetRotateDegree(), 
		pGameUnit_->GetRotateDegree(),
		pGameUnit_->GetLandPos().y );	

	if ( NULL != m_pDamageEffectThirdBlade )
	{
		CKTDGXMeshPlayer::CXMeshInstance* pMeshInst = m_pDamageEffectThirdBlade->GetMainEffect();
		if ( NULL != pMeshInst )
		{
			pMeshInst->SetBoundingRadius( 0.0f );

			const D3DXMATRIX* pMatrixBone =	pGameUnit_->GetCombineMatrixFromBoneName( L"Dummy1_Rhand" );
			if ( NULL != pMatrixBone )
			{
				pMeshInst->SetUseDXMatrix( *pMatrixBone );
				pMeshInst->OnFrameMove( g_pKTDXApp->GetTime(), 0.01f );
			}
		}
	}
}

/** @function : InitializeMeshThirdBaldeCenter
	@brief : ���Ҽҵ��� �޽� ����
	@param : ���Ҽҵ带 ����ϴ� ��������(pGameUnit_)
*/
void CX2CustomFunctionByBuffPhantomSword::InitializeMeshThirdBaldeCenter( CX2GameUnit* pGameUnit_ )
{
	CKTDGXMeshPlayer::CXMeshInstance* pMeshInstCenter 
		= g_pX2Game->GetMajorXMeshPlayer()->CreateInstance( pGameUnit_,  L"ThirdBladeCenter",
		pGameUnit_->GetPos(), pGameUnit_->GetRotateDegree(), pGameUnit_->GetRotateDegree() );
	if ( NULL != pMeshInstCenter )
	{
		m_hThirdBladeCenterMesh = pMeshInstCenter->GetHandle();
		pMeshInstCenter->SetBoundingRadius( 0.0f );

		const D3DXMATRIX* pMatrixBone =	pGameUnit_->GetCombineMatrixFromBoneName( L"Dummy1_Rhand" );
		if ( NULL != pMatrixBone )
		{
			pMeshInstCenter->SetUseDXMatrix( *pMatrixBone );
			pMeshInstCenter->OnFrameMove( g_pKTDXApp->GetTime(), 0.01f );
		}
	}
}

/** @function : CreateAndInsertThirdBladeData
	@brief : ���Ҽҵ��� ������ ����Ÿ ���� �� queue�� push�ϴ� �Լ�
	@param : ���Ҽҵ带 ����ϴ� ��������(pGameUnit_), ��ƸŴ���(luaManager_)
*/
void CX2CustomFunctionByBuffPhantomSword::CreateAndInsertThirdBladeData( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ )
{
	const D3DXVECTOR2* const pAttackTime 
		= pGameUnit_->GetAttackTimeListByIndex( m_iPhantomBladeAttackTimeIndex );
	if ( NULL != pAttackTime )
	{
		ThirdBladeDamageDataPtr ptrThirdBladeDamageData( new ThirdBladeDamageData );
		if ( NULL != ptrThirdBladeDamageData )
		{
			ptrThirdBladeDamageData->m_DamageData.SimpleInit();
			ptrThirdBladeDamageData->m_DamageData.attackerType			= CX2DamageManager::AT_UNIT;
			ptrThirdBladeDamageData->m_DamageData.optrAttackerGameUnit	= pGameUnit_;
			ptrThirdBladeDamageData->m_DamageData.pAttackerEffect		= NULL;

			if ( m_wstrPhantomBladeDamageDataName.empty() )
				g_pData->GetDamageManager()->SetDamageDataFromLUA( &(ptrThirdBladeDamageData->m_DamageData), luaManager_, L"DAMAGE_DATA" );
			else
				g_pData->GetDamageManager()->SetDamageDataFromLUA( &(ptrThirdBladeDamageData->m_DamageData), luaManager_, m_wstrPhantomBladeDamageDataName.c_str() );
			
			const D3DXVECTOR2 vDelayedAttackTime = *pAttackTime / pGameUnit_->GetPlaySpeed();

			ptrThirdBladeDamageData->m_vAttackTime.x				= m_fThirdBladeTime + vDelayedAttackTime.x + MAGIC_PHANTOM_SWORD_DELAY; 
			ptrThirdBladeDamageData->m_vAttackTime.y				= m_fThirdBladeTime + vDelayedAttackTime.y + MAGIC_PHANTOM_SWORD_DELAY; 
			ptrThirdBladeDamageData->m_DamageData.damage.fPhysic	*= MAGIC_PHANTOM_DAMAGE_MULTIPLIER;
			ptrThirdBladeDamageData->m_DamageData.damage.fMagic		*= MAGIC_PHANTOM_DAMAGE_MULTIPLIER;
			ptrThirdBladeDamageData->m_DamageData.fForceDownValue	= 0.f;
			ptrThirdBladeDamageData->m_DamageData.fHitAddMP			= 0.f;
			ptrThirdBladeDamageData->m_DamageData.fStopTimeDef		+= MAGIC_PHANTOM_STOP_TIME_DEF;
			ptrThirdBladeDamageData->m_DamageData.backSpeed			= m_vPhantomBladeBackSpeed;
			ptrThirdBladeDamageData->m_DamageData.bArrangedFly		= true;
			ptrThirdBladeDamageData->m_DamageData.attackType		= CX2DamageManager::AT_SPECIAL;
			ptrThirdBladeDamageData->m_DamageData.optrAttackerGameUnit	= pGameUnit_;

			if ( CX2DamageManager::RT_NO_DAMAGE != m_PhantomBladeReActType )
				ptrThirdBladeDamageData->m_DamageData.reActType = m_PhantomBladeReActType;

			m_ThirdBladeDamageDataList.push_back( ptrThirdBladeDamageData );
		}
	}
}

/** @function : MoveDataDataToDamageEffect
	@brief : ������ ������Ʈ���� �о���� ������ �����͸� ����������Ʈ�� ����
	@param : ���Ҽҵ带 ������� ��������(pGameUnit_)
*/
void CX2CustomFunctionByBuffPhantomSword::MoveDataDataToDamageEffect( CX2GameUnit* pGameUnit_ )
{
	/// m_ThirdBladeDamageDataList�� empty �˻�, m_pDamageEffectThirdBlade NULL üũ�� caller���� ������
	ThirdBladeDamageDataPtr ptrThirdBladeDamageData = m_ThirdBladeDamageDataList.front();
	m_pDamageEffectThirdBlade->SetAttackTime( ptrThirdBladeDamageData->m_vAttackTime );

	CX2DamageManager::DamageData* pDamageData = m_pDamageEffectThirdBlade->GetDamageData();
	*pDamageData = ptrThirdBladeDamageData->m_DamageData;
	pDamageData->attackerType = CX2DamageManager::AT_EFFECT;
	pDamageData->pAttackerEffect = m_pDamageEffectThirdBlade;
	m_pDamageEffectThirdBlade->SetFollowLineRight( pGameUnit_->GetIsRight() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

// CX2CustomFunctionByBuffMagicalMakeUp::CX2CustomFunctionByBuffMagicalMakeUp( const KBuffIdentity& BuffIdentity_, CX2GameUnit* pGameUnit_ )
// 	: CX2CustomFunctionByBuff( BuffIdentity_ ), MAGICAL_MAKE_UP_WEAPON_ID( 40930 )
// {
// 	CX2Unit* pUnit = pGameUnit_->GetUnit();
// 	if ( NULL != pUnit )
// 	{
// 		m_ptrEquipTransform = CX2ItemManager::CreateDummyEquip( MAGICAL_MAKE_UP_WEAPON_ID, pUnit, pGameUnit_->GetSkinAnimPtr() );
// 		m_ptrEquipTransform->SetManagerObject( this );
// 		m_ptrEquipTransform->SetShowObject(false);
// 	}
// }
// 
// CX2CustomFunctionByBuffMagicalMakeUp::~CX2CustomFunctionByBuffMagicalMakeUp()
// {
// 	/// �� ���ָ��� Ư�� ������ ������ �� ��������� �ϴ� ��(ex: �����ø���ũ���� �������̶�� �÷��� ��...)
// //	pGameUnit_->UpSetSpecificValueByBuffTempletID( m_DataBuffIdentity.m_eBuffTempletID );
// }
// 
// void CX2CustomFunctionByBuffMagicalMakeUp::FunctionInCommonFrameMove( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ )
// {
// // 	if ( NULL != m_ptrEquipTransform )
// // 	{
// // 		m_ptrEquipTransform->SetRenderParam( m_fTime, m_fElapsedTime * m_AdvanceTimeCount, m_pXSkinAnim->GetRenderParam() );
// // 		m_ptrEquipTransform->OnFrameMove( m_fTime, m_fElapsedTime * m_AdvanceTimeCount );
// // 	}	
// }

/*virtual*/ void CX2CustomFunctionByBuffBigBangStream::FunctionInCommonStateStart( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ )
{
	m_vecAnimationTime.resize( 0 );
	if ( luaManager_.BeginTable( "BIG_BANG_STREAM" ) )
	{
		int iIndex = 1;
		float fValue = 0.0f;
		while ( luaManager_.GetValue( iIndex, fValue ) )
		{
			m_vecAnimationTime.push_back( fValue );
			++iIndex;
		}

		/// ū�Ϳ� ���� ���� ������ ����
		std::sort( m_vecAnimationTime.begin(), m_vecAnimationTime.end(), std::greater<float>() );
		m_vStateStartPos = pGameUnit_->GetPos();	/// ������Ʈ�� ���ۉ��� ���� ���� ��ġ�� ����

		luaManager_.EndTable();
	}
}

/*virtual*/ void CX2CustomFunctionByBuffBigBangStream::DoExtraAttack( CX2GameUnit* pGameUnit_ )
{
	const D3DXVECTOR3& vBonePos = pGameUnit_->GetBonePos( L"Bip01" );
	
	CX2DamageEffect::CEffect* pDamageEffect 
		= g_pX2Game->GetDamageEffect()->CreateInstance( 
		pGameUnit_, L"HAND_GRENADE_0", pGameUnit_->GetPowerRate(), vBonePos,
		pGameUnit_->GetRotateDegree(), pGameUnit_->GetRotateDegree(),  pGameUnit_->GetLandPos().y );
	g_pX2Game->AddRemoveDamageEffect( pDamageEffect );	
	
	CKTDGXMeshPlayer::CXMeshInstance *pMesh = pDamageEffect->GetMainEffect();
	if ( NULL != pMesh )
	{
#ifdef FIX_BIGBANG_STREAM_BUFF
		D3DXVECTOR3 vDir = pGameUnit_->GetDirVector() * 150.f;

		float fDirX = ( (vDir.x + abs(vDir.z)) * 0.3f ) * 10.f + 100.f;
		const float fDirY = (vDir.y * 10.f) + 100.f;
#else
		const D3DXVECTOR3& vPos = pGameUnit_->GetPos();
		D3DXVECTOR3 vDir = vPos - m_vStateStartPos;
		float fDirX = ( (vDir.x + vDir.z) * 0.5f ) * 10.f + 100.f;
		const float fDirY = (vDir.y * 10.f) + 100.f;

		if ( pGameUnit_->GetIsRight() == false )
			fDirX *= -1.f;
#endif // FIX_BIGBANG_STREAM_BUFF

		pMesh->SetVelocity( D3DXVECTOR3( fDirX, fDirY, 0.f) );
	}
}

/*virtual*/ void CX2CustomFunctionByBuffBigBangStream::FunctionInCommonFrameMove( CX2GameUnit* pGameUnit_, KLuaManager& luaManager_ )
{
	if ( !m_vecAnimationTime.empty() && pGameUnit_->GetSkinAnimPtr() )
	{
		const float fNowAnimationTime = pGameUnit_->GetSkinAnimPtr()->GetNowAnimationTime();

		while ( !m_vecAnimationTime.empty() && m_vecAnimationTime.back() <= fNowAnimationTime )
		{
			DoExtraAttack( pGameUnit_ );
			m_vecAnimationTime.pop_back();
		}		
	}
}

/*virtual*/ void CX2CustomFunctionByBuffMarkOfCommand::FunctionInDamageReact( CX2GameUnit* pGameUnit_, IN CX2DamageManager::DamageData& damageData_ )
{
	DoMarkOfCommand( pGameUnit_, damageData_ );
}

void CX2CustomFunctionByBuffMarkOfCommand::DoMarkOfCommand( CX2GameUnit* pGameUnit_, IN CX2DamageManager::DamageData& damageData_ )
{
#ifdef SERV_ADD_CHUNG_SHELLING_GUARDIAN
if( damageData_.m_fHpPercentUp <= 0.f &&
	damageData_.reActType != CX2DamageManager::RT_CREATE_DAMAGE_EFFECT &&
	damageData_.reActType != CX2DamageManager::RT_NO_DAMAGE &&
	damageData_.reActResult != CX2DamageManager::RT_DUMMY_DAMAGE &&
	damageData_.reActResult != CX2DamageManager::RT_SENSOR &&
	damageData_.hitType != CX2DamageManager::HT_GROUND_HIT &&
	damageData_.m_eDamageTrigger != CX2DamageManager::DTT_MARK_OF_COMMANDER &&
	damageData_.m_bNoDamage == false )
{
	if ( null != damageData_.optrAttackerGameUnit &&
		 m_checkElapsedTime.CheckAndResetElapsedTime() )
	{
		CX2GameUnit* pAttackerGameUnit = damageData_.optrAttackerGameUnit.GetObservable();

		D3DXVECTOR3 vDirectVectorZeroY = pAttackerGameUnit->GetDirVector();
		vDirectVectorZeroY.y	= 0.f;
		const float fConstUnitHeight = 1080.f;
		const float fShellPowerRate = damageData_.damage.GetAllDamage() * damageData_.m_fDungeonRate;

		for ( UINT uiIndex = 0; uiIndex < m_uiDamageEffectCount; ++uiIndex )
		{
			float fRandReservTime = 0.5f;
			D3DXVECTOR3 vRotateDegree = pAttackerGameUnit->GetRotateDegree();

			if ( 0 < uiIndex )
			{
				fRandReservTime += pAttackerGameUnit->GetRandomFloat( uiIndex ) * 0.3f * uiIndex;
				fRandReservTime += pAttackerGameUnit->GetRandomFloat( pGameUnit_->GetUnitIndex() ) * 0.3f;
				vRotateDegree.z += ( pAttackerGameUnit->GetRandomFloat( pGameUnit_->GetUnitIndex() + uiIndex) * 10.f - 5.f );
			}

			D3DXVECTOR3 vShellStartPos = pGameUnit_->GetPos();
			vShellStartPos.y += fConstUnitHeight;			

			if ( pAttackerGameUnit->GetIsRight() )
				vShellStartPos += ( m_fBasicPosX[uiIndex] * vDirectVectorZeroY );
			else
				vShellStartPos -= ( m_fBasicPosX[uiIndex] * vDirectVectorZeroY );

			/// �޸� �������
			const WCHAR* pwszDamageEffectName
				= ( ( 0.0f < m_fMemoRate && pAttackerGameUnit->GetRandomFloat( uiIndex + 3 ) <= m_fMemoRate ) ?
				L"CHUNG_MARK_OF_COMMANDER_SHELL_MEMO" : L"CHUNG_MARK_OF_COMMANDER_SHELL" );
#ifdef UPGRADE_SKILL_SYSTEM_2013 // û ��ų ����, ������
			g_pX2Game->GetDamageEffect()->CreateInstance( pAttackerGameUnit, pwszDamageEffectName, 
			fShellPowerRate * m_fAddPowerRate, vShellStartPos, vRotateDegree, vRotateDegree,  
			pAttackerGameUnit->GetLandPos().y, true, fRandReservTime );						

#else // UPGRADE_SKILL_SYSTEM_2013 // û ��ų ����, ������
			g_pX2Game->GetDamageEffect()->CreateInstance( pAttackerGameUnit, pwszDamageEffectName, 
				fShellPowerRate, vShellStartPos, vRotateDegree, vRotateDegree,  
				pAttackerGameUnit->GetLandPos().y, true, fRandReservTime );				
#endif // UPGRADE_SKILL_SYSTEM_2013 // û ��ų ����, ������								
		}
		
	}
}
#endif SERV_ADD_CHUNG_SHELLING_GUARDIAN

}
