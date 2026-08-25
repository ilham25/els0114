#include "StdAfx.h"
#include ".\x2userskilltree.h"


/** @function	CX2userSkillTree
	@brief		������, ���� ��ų �ʱ�ȭ, ���Ŭ���� �ʱ�ȭ
	*/
CX2UserSkillTree::CX2UserSkillTree(void) :
m_eSkillSlotBExpirationState( SSBES_EXPIRED ),
	m_wstrSkillSlotBEndDate( L"" ),
	m_iUnitClass( CX2Unit::UC_NONE )
{
	BOOST_STATIC_ASSERT( ARRAY_SIZE( m_aEquippedSkill ) == ARRAY_SIZE( m_aEquippedSkillSlotB ) );
	for( int i=0; i<ARRAY_SIZE( m_aEquippedSkill ); ++i )
	{
		m_aEquippedSkill[i].Init();
		m_aEquippedSkillSlotB[i].Init();
	}
#ifdef GUILD_SKILL
	m_iGuildClass = 0;
#endif GUILD_SKILL

}

/** @function	~CX2UserSkillTree
	@brief		�Ҹ���
*/
CX2UserSkillTree::~CX2UserSkillTree(void)
{
}

/** @function	OnFrameMove
	@brief		������ ���� ��, ��ų ��Ÿ�� ó��
	@param[in]	fTime : ���� �ð�
	@param[in]	fElapsedTime : ��� �ð�
*/
void CX2UserSkillTree::OnFrameMove( double fTime, float fElapsedTime )
{
	CX2GageManager* pGageManager = CX2GageManager::GetInstance();


#ifdef NEXON_QA_CHEAT_REQ
	if( NULL != g_pData->GetMyUser() && g_pData->GetMyUser()->GetAuthLevel() >= CX2User::XUAL_SPECIAL_USER && true == g_pMain->GetNoCoolTime() )
	{
		AllSkillNoCoolTime();
		g_pMain->SetNoCoolTime( false );
	}
	else
	{

#endif //NEXON_QA_CHEAT_REQ


	for( int i=0; i<ARRAY_SIZE( m_aEquippedSkill ); ++i )
	{
		SkillSlotData& skillData = m_aEquippedSkill[i];
		if( skillData.m_fCoolTimeLeft > 0.f )
		{
			skillData.m_fCoolTimeLeft -= fElapsedTime;
			if( skillData.m_fCoolTimeLeft < 0.f )
			{
				skillData.m_fCoolTimeLeft = 0.f;
			}

			pGageManager->SetMySkillCoolTimeList( i, skillData.m_fCoolTimeLeft );
		}
	}

	for( int i=0; i<ARRAY_SIZE( m_aEquippedSkillSlotB ); i++ )
	{
		SkillSlotData& skillData = m_aEquippedSkillSlotB[i];
		if( skillData.m_fCoolTimeLeft > 0.f )
		{
			skillData.m_fCoolTimeLeft -= fElapsedTime;
			if( skillData.m_fCoolTimeLeft < 0.f )
			{
				skillData.m_fCoolTimeLeft = 0.f;
			}

			pGageManager->SetMySkillCoolTimeList( i + EQUIPPED_SKILL_SLOT_COUNT, skillData.m_fCoolTimeLeft );
		}
	}	


#ifdef NEXON_QA_CHEAT_REQ
	}
#endif //NEXON_QA_CHEAT_REQ


	if( NULL != g_pData && NULL != g_pData->GetMyUser() && NULL != g_pData->GetMyUser()->GetSelectUnit() 
		&& NULL != g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() )
	{
		int iCurrentSP = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_iSPoint + g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_iCSPoint;	
		static int iOldSP = 0;

		if( iCurrentSP != iOldSP )
		{
			if( NULL != g_pData->GetPlayGuide() )
			{
				if( iCurrentSP > iOldSP )
				{	//SP�� �������� �þ��� �� SP ȹ�� ���̵� ���
					g_pData->GetPlayGuide()->SetPlayGuide(CX2PlayGuide::PGT_OBTAIN_SKILL_POINT, true, iCurrentSP);
				}
				iOldSP = iCurrentSP;
			}

			if( iCurrentSP > 0 )
			{	//SP�� 0�ʰ� �� �� SP ���� ���̵� ���

				if( NULL != g_pMain && NULL != g_pMain->GetKeyPad() )
				{
					g_pData->GetPlayGuide()->SetPlayGuide(CX2PlayGuide::PGT_HAS_SKILL_POINT, true, g_pMain->GetKeyPad()->GetKeyString( GAMEACTION_SKILL_TREE ) );
				}
			}
			else
			{
				g_pData->GetPlayGuide()->SetPlayGuide(CX2PlayGuide::PGT_HAS_SKILL_POINT, false);
			}
		}
	}

#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
	/// ��ų ���� ���� ���� ��ȿ�� �˻�
	SkillDataMap::const_iterator mit = m_mapSkillAcquired.begin();

	for ( mit; mit != m_mapSkillAcquired.end(); ++mit )
	{
		if( mit->second.m_iSkillLevel.Verify() == false ||
			mit->second.m_iSkillCSPoint.Verify() == false ||
			mit->second.m_iIncreaseSkillLevelByBuff.Verify() == false ||
			mit->second.m_iIncreaseSkillLevelBySocket.Verify() == false )
		{
			if( NULL != g_pData &&
				NULL != g_pData->GetMyUser() &&
				NULL != g_pData->GetMyUser()->GetUserData() &&
				NULL != g_pData->GetServerProtocol() &&
				g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER &&
				g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT) )
			return;
		}
	}

	SkillDataMap::const_iterator mitGuild = m_mapGuildSkillAcquired.begin();

	for ( mitGuild; mitGuild != m_mapGuildSkillAcquired.end(); ++mitGuild )
	{
		if( mitGuild->second.m_iSkillLevel.Verify() == false ||
			mitGuild->second.m_iSkillCSPoint.Verify() == false ||
			mitGuild->second.m_iIncreaseSkillLevelByBuff.Verify() == false ||
			mitGuild->second.m_iIncreaseSkillLevelBySocket.Verify() == false )
		{
			if( NULL != g_pData &&
				NULL != g_pData->GetMyUser() &&
				NULL != g_pData->GetMyUser()->GetUserData() &&
				NULL != g_pData->GetServerProtocol() &&
				g_pData->GetMyUser()->GetUserData()->hackingUserType != CX2User::HUT_AGREE_HACK_USER &&
				g_pData->GetServerProtocol()->SendID( EGS_REPORT_HACK_USER_NOT) )
			return;
		}
	}
#endif // UPGRADE_SKILL_SYSTEM_2013
}

/** @function	Reset
	@brief		�缳��, ���� ��ų(A, B), ��ų Ʈ��, ���� ��ų �ʱ�ȭ
	@param[in]	bResetSkillTree : ��ų Ʈ��
	@param[in]	bResetEquippedSkill : ���� ��ų
	@param[in]	bResetUnsealedSkill : ���� ������ ��ų
	@param[in]	bResetSkillSlotB : B ����
*/
void CX2UserSkillTree::Reset( bool bResetSkillTree, bool bResetEquippedSkill, bool bResetUnsealedSkill, bool bResetSkillSlotB )
{
	if( true == bResetEquippedSkill )
	{
		for( int i=0; i<ARRAY_SIZE( m_aEquippedSkill ); ++i )
		{
			m_aEquippedSkill[i].Init();
			m_aEquippedSkillSlotB[i].Init();
		}
	}

	if( true == bResetSkillTree )
	{
		m_mapSkillAcquired.clear();
	}

	if( true == bResetUnsealedSkill )
	{
		m_setUnsealedSkillID.clear();
	}

	if( bResetSkillSlotB )
	{
		SetSkillSlotBExpirationState( SSBES_EXPIRED );
	}
}

/** @function	Reset
	@brief		���� �Լ� ��� ��ų �ʱ�ȭ �߰�
	@param[in]	bResetSkillTree : ��ų Ʈ��
	@param[in]	bResetGuildSkillTree : ��� ��ų Ʈ��
	@param[in]	bResetEquippedSkill : ���� ��ų
	@param[in]	bResetUnsealedSkill : ���� ������ ��ų
	@param[in]	bResetSkillSlotB : B ����
*/
#ifdef GUILD_SKILL
//{{ oasis907 : ����� //// 2009-11-12 //// 
void CX2UserSkillTree::Reset( bool bResetSkillTree, bool bResetGuildSkillTree, bool bResetEquippedSkill, bool bResetUnsealedSkill, bool bResetSkillSlotB )
{
	if( true == bResetEquippedSkill )
	{
		for( int i=0; i<ARRAY_SIZE( m_aEquippedSkill ); ++i )
		{
			m_aEquippedSkill[i].Init();
			m_aEquippedSkillSlotB[i].Init();
		}
	}

	if( true == bResetSkillTree )
	{
		m_mapSkillAcquired.clear();
	}

	if( true == bResetGuildSkillTree )
	{
		m_mapGuildSkillAcquired.clear();
	}

	if( true == bResetUnsealedSkill )
	{
		m_setUnsealedSkillID.clear();
	}

	if( bResetSkillSlotB )
	{
		SetSkillSlotBExpirationState( SSBES_EXPIRED );
	}
}
//}} oasis907 : ����� //// 2009-11-12 //// 
#endif GUILD_SKILL

/** @function	ConstructSkillTreeNotMyUnit
	@brief		����, ���� ���ӿ��� �� ĳ���Ͱ� �ƴ� �ٸ� ĳ���Ͱ� � passive skill�� ������� ������ �� ���Ǵ� �Լ�
	@param[in]	vecSkillData : ��ų ����
	@param[out]	aEquippedSkill[] : A Slot
	@param[out]	aEquippedSkillSlotB[] : B Slot
*/
void CX2UserSkillTree::ConstructSkillTreeNotMyUnit( const std::vector<KSkillData>& vecSkillData, const KSkillData aEquippedSkill[], const KSkillData aEquippedSkillSlotB[] )
{
	// ������ // 
	// �߰� �� m_iIncreaseSkillLevelBySocket���� �ʱ�ȭ ��Ű�� �ʱ� ���� false�� ����
	Reset( false, false, false, false );

	for( int i=0; i<EQUIPPED_SKILL_SLOT_COUNT; i++ )
	{
		SetSkillLevelAndCSP( (CX2SkillTree::SKILL_ID) aEquippedSkill[i].m_iSkillID, (int) aEquippedSkill[i].m_cSkillLevel, 0 );
	}

	for( int i=0; i<EQUIPPED_SKILL_SLOT_COUNT; i++ )
	{
		SetSkillLevelAndCSP( (CX2SkillTree::SKILL_ID) aEquippedSkillSlotB[i].m_iSkillID, (int) aEquippedSkillSlotB[i].m_cSkillLevel, 0 );
	}

	BOOST_TEST_FOREACH( const KSkillData&, kSkillData, vecSkillData )
	{
		SetSkillLevelAndCSP( (CX2SkillTree::SKILL_ID) kSkillData.m_iSkillID, (int) kSkillData.m_cSkillLevel, 0 );
	}
}

#ifdef GUILD_SKILL
//{{ oasis907 : ����� //// 2009-11-18 //// 
/** @function	ConstructGuildSkillTreeNotMyUnit
	@brief		�����κ��� ���� ��� ��ų�� �����ϴ� �Լ�, ���߿� ���� �Լ��� ��ü X
				��� ��ų�� ������Ʈ�ϴ� �Լ� (���� ��� ������)
				����, �������ӿ��� �� ĳ���Ͱ� �ƴ� �ٸ� ĳ���Ͱ� � passive skill�� ������� ������ �� ���Ǵ� �Լ�
	@param[in]	vecGuildSkillData : ��� ��ų ����
*/
void CX2UserSkillTree::ConstructGuildSkillTreeNotMyUnit( const std::vector<KSkillData>& vecGuildSkillData )
{
	Reset( false, true, false, false, false );

	// ��� ��ų
	BOOST_TEST_FOREACH( const KSkillData&, kSkillData, vecGuildSkillData )
	{
		SetSkillLevelAndCSP( (CX2SkillTree::SKILL_ID) kSkillData.m_iSkillID, (int) kSkillData.m_cSkillLevel, 0 );
	}
}
//}} oasis907 : ����� //// 2009-11-18 //// 
#endif GUILD_SKILL

void CX2UserSkillTree::SetEquippedSkill( const int iSlotIndex_, const bool bSlotB_, const KSkillData& equippedSkillData_ )
{
	if ( bSlotB_ )
	{
		if ( iSlotIndex_ >= 0 && iSlotIndex_ < ARRAY_SIZE( m_aEquippedSkillSlotB ) )
		{
			m_aEquippedSkillSlotB[iSlotIndex_].Init();
			m_aEquippedSkillSlotB[iSlotIndex_].m_eID			= static_cast<CX2SkillTree::SKILL_ID>( equippedSkillData_.m_iSkillID );
			m_aEquippedSkillSlotB[iSlotIndex_].m_iSkillLevel	= static_cast<int>( equippedSkillData_.m_cSkillLevel );
		}
	}
	else
	{
		if ( iSlotIndex_ >= 0 && iSlotIndex_ < ARRAY_SIZE( m_aEquippedSkill ) )
		{
			m_aEquippedSkill[iSlotIndex_].Init();
			m_aEquippedSkill[iSlotIndex_].m_eID			= static_cast<CX2SkillTree::SKILL_ID>( equippedSkillData_.m_iSkillID );
			m_aEquippedSkill[iSlotIndex_].m_iSkillLevel	= static_cast<int>( equippedSkillData_.m_cSkillLevel );
		}
	}
}

/** @function	SetEquippedSkill
	@brief		���� ��ų ����
	@param[out]	aEquippedSkill[] : A Slot
	@param[out]	aEquippedSkillSlotB[] : B Slot
*/
void CX2UserSkillTree::SetEquippedSkill( const KSkillData aEquippedSkill[], const KSkillData aEquippedSkillSlotB[] )
{
	/// �ʱ�ȭ
	for( int i=0; i<ARRAY_SIZE( m_aEquippedSkill ); ++i )
	{
		m_aEquippedSkill[i].Init();
		m_aEquippedSkillSlotB[i].Init();
	}

	
	/// A ����(ID, ����)
	for( int i=0; i<ARRAY_SIZE( m_aEquippedSkill ); ++i )
	{
		m_aEquippedSkill[i].m_eID			= (CX2SkillTree::SKILL_ID) aEquippedSkill[i].m_iSkillID;
		m_aEquippedSkill[i].m_iSkillLevel	= (int) aEquippedSkill[i].m_cSkillLevel;
	}

	/// B ����(ID, ����)
	for( int i=0; i<ARRAY_SIZE( m_aEquippedSkillSlotB ); i++ )
	{
		m_aEquippedSkillSlotB[i].m_eID			= (CX2SkillTree::SKILL_ID) aEquippedSkillSlotB[i].m_iSkillID;
		m_aEquippedSkillSlotB[i].m_iSkillLevel	= (int) aEquippedSkillSlotB[i].m_cSkillLevel;
	}
}

void CX2UserSkillTree::SetEquippedSkillLevelPlusBySlotIndex( const int iEquippedSkillLevelPlus_, const int iSlotIndex_, const bool bIsSlotB_ )
{
	if ( bIsSlotB_ )
	{
		if ( iSlotIndex_ >= 0 && iSlotIndex_ < ARRAY_SIZE( m_aEquippedSkillSlotB ) )
			m_aEquippedSkillSlotB[iSlotIndex_].m_iSkillLevelPlus = iEquippedSkillLevelPlus_;
	}
	else
	{
		if ( iSlotIndex_ >= 0 && iSlotIndex_ < ARRAY_SIZE( m_aEquippedSkill ) )
			m_aEquippedSkill[iSlotIndex_].m_iSkillLevelPlus = iEquippedSkillLevelPlus_;
	}
}

/** @function	SetEquippedSkillLevelPlus
	@brief		���� ��ų ���� ����(�����ۿ� ����)
	@param[in]	aEquippedSkillLevelPlus[] : A Slot ��ų ���� ���� ��
	@param[in]	aEquippedSkillLevelPlusSlotB[] : B Slot ��ų ���� ���� ��
*/
void CX2UserSkillTree::SetEquippedSkillLevelPlus( const int aEquippedSkillLevelPlus[], const int aEquippedSkillLevelPlusSlotB[] )
{
	for( int i=0; i<ARRAY_SIZE( m_aEquippedSkill ); ++i )
	{
		m_aEquippedSkill[i].m_iSkillLevelPlus	= aEquippedSkillLevelPlus[i];
	}

	for( int i=0; i<ARRAY_SIZE( m_aEquippedSkillSlotB ); ++i )
	{
		m_aEquippedSkillSlotB[i].m_iSkillLevelPlus	= aEquippedSkillLevelPlusSlotB[i];
	}
}


/** @function	SetAcquiredSkill
	@brief		����� ��ų ����
	@param[in]	��ų ����Ʈ
*/
void CX2UserSkillTree::SetAcquiredSkill( const std::vector<KUserSkillData>& vecSkillList )
{
	m_mapSkillAcquired.clear();
	BOOST_TEST_FOREACH( const KUserSkillData&, userSkillData, vecSkillList )
	{
		SkillDataMap::iterator mit;
		mit = m_mapSkillAcquired.find( (CX2SkillTree::SKILL_ID) userSkillData.m_iSkillID );
		ASSERT( mit == m_mapSkillAcquired.end() );
		if( mit == m_mapSkillAcquired.end() )
		{
			// �̹� �����ϸ� ���Ե��� �ʰ� �������� ������ ���Եȴ�.
			m_mapSkillAcquired[ (CX2SkillTree::SKILL_ID)userSkillData.m_iSkillID ] = UserSkillData( (int)userSkillData.m_cSkillLevel, (int)userSkillData.m_cSkillCSPoint );
		}
	}
}
#ifdef GUILD_SKILL
//{{ oasis907 : ����� //// 2009-11-12 //// 
/** @function	SetAcquiredGuildSkill
	@brief		����� ��� ��ų ����
	@param[in]	��� ��ų ����Ʈ
*/
void CX2UserSkillTree::SetAcquiredGuildSkill( const std::vector<KGuildSkillData>& vecSkillList )
{
	m_mapGuildSkillAcquired.clear();
	BOOST_TEST_FOREACH( const KGuildSkillData&, userSkillData, vecSkillList )
	{
		SkillDataMap::iterator mit;
		mit = m_mapGuildSkillAcquired.find( (CX2SkillTree::SKILL_ID) userSkillData.m_iSkillID );
		ASSERT( mit == m_mapGuildSkillAcquired.end() );
		if( mit == m_mapGuildSkillAcquired.end() )
		{
			// �̹� �����ϸ� ���Ե��� �ʰ� �������� ������ ���Եȴ�.
			m_mapGuildSkillAcquired[ (CX2SkillTree::SKILL_ID)userSkillData.m_iSkillID ] = UserSkillData( (int)userSkillData.m_cSkillLevel, (int)userSkillData.m_cSkillCSPoint );
		}
	}
}
//}} oasis907 : ����� //// 2009-11-12 //// 
#endif GUILD_SKILL
/** @function	SetUnsealedSkill
	@brief		���� ������ ��ų ����
	@param[in]	���� ������ ��ų ����Ʈ
*/
void CX2UserSkillTree::SetUnsealedSkill( const std::vector<short>& vecUnsealedSkillList )
{
	//  ���������� ��ų ���
	m_setUnsealedSkillID.clear();
	for( UINT i=0; i<vecUnsealedSkillList.size(); i++ )
	{
		m_setUnsealedSkillID.insert( (CX2SkillTree::SKILL_ID) vecUnsealedSkillList[i] );
	}
}

/** @function	GetSkillLevel
	@brief		��ų ���� ���
	@param[in]	��ų ID, �߰��� ��ų ȿ��( ĳ�� ����Ʈ, ����, ���� ) ���� ����
	@return		��ų ����
*/
int CX2UserSkillTree::GetSkillLevel( CX2SkillTree::SKILL_ID eSkillID, bool bBaslcLevel /*= false*/ ) const
{
	KTDXPROFILE();

#ifdef GUILD_SKILL
	//{{ oasis907 : ����� //// 2009-11-16 //// 
	if(IsGuildSkill(eSkillID))
	{
		SkillDataMap::const_iterator mit = m_mapGuildSkillAcquired.find( eSkillID );
		if( mit != m_mapGuildSkillAcquired.end() )
		{
			const UserSkillData& userSkillData = mit->second;

		#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
			const int iSkillLevel = true == bBaslcLevel ? userSkillData.m_iSkillLevel : userSkillData.GetTotalSkillLevel();

			return min( iSkillLevel, MAX_LIMITED_SKILL_LEVEL );
		#else // UPGRADE_SKILL_SYSTEM_2013
			return userSkillData.m_iSkillLevel;
		#endif // UPGRADE_SKILL_SYSTEM_2013
		}
		return 0;
	}
	//}} oasis907 : ����� //// 2009-11-16 //// 
#endif GUILD_SKILL

	SkillDataMap::const_iterator mit = m_mapSkillAcquired.find( eSkillID );
	if( mit != m_mapSkillAcquired.end() )
	{
		const UserSkillData& userSkillData = mit->second;

	#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
		const int iSkillLevel = true == bBaslcLevel ? userSkillData.m_iSkillLevel : userSkillData.GetTotalSkillLevel();

		return min( iSkillLevel, MAX_LIMITED_SKILL_LEVEL );
	#else // UPGRADE_SKILL_SYSTEM_2013
		return userSkillData.m_iSkillLevel;
	#endif // UPGRADE_SKILL_SYSTEM_2013
	}
	return 0;
}

//{{ oasis907 : ����� //// 2009-11-18 ////
/** @function	GetUserSkillTemplet
	@brief		���� ��ų ���ø� ���
	@param[in]	��ų ID
	@return		��ų ���ø�
*/
const CX2SkillTree::SkillTemplet* CX2UserSkillTree::GetUserSkillTemplet( CX2SkillTree::SKILL_ID eSkillID) const
{
	KTDXPROFILE();

	//// passive skill ���� ����ؾ� �Ѵ�. active skill�� getequippedactiveskill()�Լ��� �����
	//ASSERT( NULL == g_pData->GetSkillTree()->GetSkillTemplet( eSkillID, 1 ) ||
	//	CX2SkillTree::ST_PASSIVE_PHYSIC_ATTACK == g_pData->GetSkillTree()->GetSkillTemplet( eSkillID, 1 )->m_eType ||
	//	CX2SkillTree::ST_PASSIVE_MAGIC_ATTACK == g_pData->GetSkillTree()->GetSkillTemplet( eSkillID, 1 )->m_eType ||
	//	CX2SkillTree::ST_PASSIVE_MAGIC_DEFENCE == g_pData->GetSkillTree()->GetSkillTemplet( eSkillID, 1 )->m_eType ||
	//	CX2SkillTree::ST_PASSIVE_PHYSIC_DEFENCE == g_pData->GetSkillTree()->GetSkillTemplet( eSkillID, 1 )->m_eType ||
	//	CX2SkillTree::ST_PASSIVE == g_pData->GetSkillTree()->GetSkillTemplet( eSkillID, 1 )->m_eType );

	SkillDataMap::const_iterator mit = m_mapSkillAcquired.find( eSkillID );

	if( mit == m_mapSkillAcquired.end() )
	{
		return NULL;
	}

	const UserSkillData& userSkillData = mit->second;
	/////////////////////////////////////////////
#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
	return g_pData->GetSkillTree()->GetSkillTemplet(eSkillID);
#else // UPGRADE_SKILL_SYSTEM_2013
	return g_pData->GetSkillTree()->GetSkillTemplet(eSkillID, userSkillData.m_iSkillLevel);
#endif // UPGRADE_SKILL_SYSTEM_2013
}
//}} oasis907 : ����� //// 2009-11-18 ////

#ifdef GUILD_SKILL
//{{ oasis907 : ����� //// 2009-11-18 ////
/** @function	GetSkillLevel
	@brief		��� ��ų ���� ���
	@param[out]	int ��� ��ų ����
*/
int CX2UserSkillTree::GetSkillLevel( CX2SkillTree::SKILL_ID eSkillID, int iGuildMemberGrade) const
{
	KTDXPROFILE();
	if(iGuildMemberGrade == CX2GuildManager::GUG_INVALID ||
		iGuildMemberGrade >= g_pData->GetGuildManager()->GetLimitGuildGrade()) // ������ ĿƮ���� ��� ��� ���
	{
		return 0;      // ����� ���� ��� ��ų ���� �ȵɶ�
	}

	SkillDataMap::const_iterator mit = m_mapGuildSkillAcquired.find( eSkillID );
	if( mit != m_mapGuildSkillAcquired.end() )
	{
		const UserSkillData& userSkillData = mit->second;
		return userSkillData.m_iSkillLevel;
	}

	return 0;
}
//}} oasis907 : ����� //// 2009-11-18 ////

//{{ oasis907 : ����� //// 2009-11-18 ////
/** @function	GetUserSkillTemplet
	@brief		��� ���� ��ų ���ø� ���
	@param[in]	��ų ID
	@param[in]	��� ��� ���
	@return		��ų ���ø�
*/
const CX2SkillTree::SkillTemplet* CX2UserSkillTree::GetUserSkillTemplet( CX2SkillTree::SKILL_ID eSkillID, int iGuildMemberGrade) const
{
	KTDXPROFILE();

	if(iGuildMemberGrade == CX2GuildManager::GUG_INVALID ||
		iGuildMemberGrade >= g_pData->GetGuildManager()->GetLimitGuildGrade())  // ������ ĿƮ���� ��� ��� ���
	{
		return NULL;	// ����� ���� ��� ��ų ���� �ȵɶ�
	}

	SkillDataMap::const_iterator mit = m_mapGuildSkillAcquired.find( eSkillID );

	if( mit == m_mapGuildSkillAcquired.end() )
	{
		return NULL;   
	}

	const UserSkillData& userSkillData = mit->second;
	/////////////////////////////////////////////

#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
	return g_pData->GetSkillTree()->GetSkillTemplet(eSkillID );
#else // UPGRADE_SKILL_SYSTEM_2013
	return g_pData->GetSkillTree()->GetSkillTemplet(eSkillID, userSkillData.m_iSkillLevel);
#endif // UPGRADE_SKILL_SYSTEM_2013
}
//}} oasis907 : ����� //// 2009-11-18 //// 
#endif GUILD_SKILL

/** @function	GetSkillLevelAndCSP
	@brief		��ų ����, ���� ����Ʈ ���
	@param[in]	��ų ID
	@param[out]	��ų ����
	@param[out]	��ų Spell Point
*/
bool CX2UserSkillTree::GetSkillLevelAndCSP( IN CX2SkillTree::SKILL_ID eSkillID, OUT int& iSkillLevel, OUT int& iSkillCSPoint )
{
#ifdef GUILD_SKILL
	//{{ oasis907 : ����� //// 2009-11-16 //// 
	if(IsGuildSkill(eSkillID))
	{
		SkillDataMap::iterator mit = m_mapGuildSkillAcquired.find( eSkillID );
		if( mit != m_mapGuildSkillAcquired.end() )
		{
			const UserSkillData& userSkillData = mit->second;
			iSkillLevel		= userSkillData.m_iSkillLevel;
			iSkillCSPoint	= userSkillData.m_iSkillCSPoint;
			return true;
		}
		return false;
	}
	//}} oasis907 : ����� //// 2009-11-16 //// 
#endif GUILD_SKILL

	SkillDataMap::iterator mit = m_mapSkillAcquired.find( eSkillID );
	if( mit != m_mapSkillAcquired.end() )
	{
		const UserSkillData& userSkillData = mit->second;
		iSkillLevel		= userSkillData.m_iSkillLevel;
		iSkillCSPoint	= userSkillData.m_iSkillCSPoint;
		return true;
	}

	return false;
}


/** @function	SetSkillLevelAndCSP
	@brief		�����Ͱ� ������ ���� �����ϰ�, ������ �߰��Ѵ�, skill level�� 0���� ������ ����
	@param[in]	��ų ID
	@param[in]	��ų ����
	@param[in]	��ų Spell Point
	@return		false : ���� ����
	@return		true : ���� �Ϸ�
*/
bool CX2UserSkillTree::SetSkillLevelAndCSP( CX2SkillTree::SKILL_ID eSkillID, int iSkillLevel, int iSkillCSPoint )
{
#ifdef GUILD_SKILL
	//{{ oasis907 : ����� //// 2009-11-16 //// 
	if(IsGuildSkill(eSkillID))
	{
#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID );
#else // UPGRADE_SKILL_SYSTEM_2013
		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID, 1 );
#endif // UPGRADE_SKILL_SYSTEM_2013
		if( NULL == pSkillTemplet )
			return false;

		if( iSkillLevel > g_pData->GetSkillTree()->GetMaxGuildSkillLevel( (int)m_iGuildClass, eSkillID ) )
			return false;

		if( iSkillLevel > 0 &&
			iSkillLevel < iSkillCSPoint )
			return false;


		SkillDataMap::iterator mit;
		mit = m_mapGuildSkillAcquired.find( eSkillID );
		if( mit != m_mapGuildSkillAcquired.end() )
		{
			if( iSkillLevel > 0 )
			{
				UserSkillData& userSkillData = mit->second;
				userSkillData.m_iSkillLevel		= iSkillLevel;
				userSkillData.m_iSkillCSPoint	= iSkillCSPoint;
			}
			else
			{
				m_mapGuildSkillAcquired.erase( mit );
			}
		}
		else 
		{
			if( iSkillLevel > 0 )
			{
				m_mapGuildSkillAcquired[ eSkillID ] = UserSkillData( iSkillLevel, iSkillCSPoint );
			}
		}
		return true;

	}
	//}} oasis907 : ����� //// 2009-11-16 //// 
#endif GUILD_SKILL

#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
	const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID );
#else // UPGRADE_SKILL_SYSTEM_2013
	const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID, 1 );
#endif // UPGRADE_SKILL_SYSTEM_2013
	if( NULL == pSkillTemplet )
		return false;

	if( iSkillLevel > g_pData->GetSkillTree()->GetMaxSkillLevel( (int)m_iUnitClass, eSkillID ) )
		return false;

#ifndef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
	if( iSkillLevel > 0 &&
		iSkillLevel < iSkillCSPoint )
		return false;
#endif // UPGRADE_SKILL_SYSTEM_2013


	SkillDataMap::iterator mit;
	mit = m_mapSkillAcquired.find( eSkillID );
	if( mit != m_mapSkillAcquired.end() )
	{
		if( iSkillLevel > 0 )
		{
			UserSkillData& userSkillData = mit->second;
			userSkillData.m_iSkillLevel		= iSkillLevel;
			userSkillData.m_iSkillCSPoint	= iSkillCSPoint;
		}
		else
		{
			m_mapSkillAcquired.erase( mit );
		}
	}
	else 
	{
		if( iSkillLevel > 0 )
		{
			m_mapSkillAcquired[ eSkillID ] = UserSkillData( iSkillLevel, iSkillCSPoint );
		}
	}
	return true;
}

/** @function	IsMaxSkillLevel
	@brief		��ų ���� Ȯ��(�ְ�ġ)
	@param[in]	��ų ID
	@return		�ְ�ġ ���� �ƴ��� ����
*/
bool CX2UserSkillTree::IsMaxSkillLevel( IN CX2SkillTree::SKILL_ID eSkillID )
{
#ifdef GUILD_SKILL
	//{{ oasis907 : ����� //// 2009-11-16 //// 
	if(IsGuildSkill(eSkillID))
	{
#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID );
#else // UPGRADE_SKILL_SYSTEM_2013
		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID, 1 );
#endif // UPGRADE_SKILL_SYSTEM_2013
		if( NULL == pSkillTemplet )
			return false;

		SkillDataMap::iterator mit;
		mit = m_mapGuildSkillAcquired.find( eSkillID );
		if( mit != m_mapGuildSkillAcquired.end() )
		{
			const UserSkillData& userSkillData = mit->second;
			if( userSkillData.m_iSkillLevel >= g_pData->GetSkillTree()->GetMaxGuildSkillLevel( (int)m_iGuildClass, eSkillID ) )
				return true;
			else 
				return false;
		}

		return false;
	}
	//}} oasis907 : ����� //// 2009-11-16 //// 
#endif GUILD_SKILL

#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
	const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID );
#else // UPGRADE_SKILL_SYSTEM_2013
	const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID, 1 );
#endif // UPGRADE_SKILL_SYSTEM_2013
	if( NULL == pSkillTemplet )
		return false;

	SkillDataMap::iterator mit;
	mit = m_mapSkillAcquired.find( eSkillID );
	if( mit != m_mapSkillAcquired.end() )
	{
		const UserSkillData& userSkillData = mit->second;
		if( userSkillData.m_iSkillLevel >= g_pData->GetSkillTree()->GetMaxSkillLevel( (int)m_iUnitClass, eSkillID ) )
			return true;
		else 
			return false;
	}

	return false;
}

//{{ kimhc // 2010.12.14 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
/** @function	SetSkillSlotInfo
	@brief		��ų ���� ���� ����(ĳ���� �߰��� ���� ����)
	@param[in]	��ų ���� ID
	@param[in]	��ų ID
*/
void CX2UserSkillTree::SetSkillSlotInfo( int iSkillSlotID, CX2SkillTree::SKILL_ID eSkillID ) 
{
	// ĳ���� �߰�
	switch( iSkillSlotID )
	{
	case CX2UserSkillTree::SKILL_SLOT_A1:
	case CX2UserSkillTree::SKILL_SLOT_A2:
	case CX2UserSkillTree::SKILL_SLOT_A3:
	case CX2UserSkillTree::SKILL_SLOT_A4:
		{
			const CX2SkillTree::SkillTemplet* pSkillTemplet = GetUserSkillTemplet( eSkillID);
			if(pSkillTemplet != NULL)
			{
#ifdef ARA_CHARACTER_BASE

	#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
				if ( NULL == g_pData ||
					 NULL == g_pData->GetMyUser() || 
					 NULL == g_pData->GetMyUser()->GetSelectUnit() || 
					 NULL == g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() )
					return;
	
				const CX2UserSkillTree& userSkillTree = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_UserSkillTree;
	
				const int iSkillTempletLevel = max( 1, userSkillTree.GetSkillLevel( pSkillTemplet->m_eID ) );	/// ��ų ����
	
				SetSkillSlot( iSkillSlotID, eSkillID, pSkillTemplet->GetSkillMPConsumptionValue( iSkillTempletLevel ), pSkillTemplet->m_usCBConsumption,
					pSkillTemplet->m_usFPConsumtion, false );
	#else // UPGRADE_SKILL_SYSTEM_2013
				SetSkillSlot( iSkillSlotID, eSkillID, pSkillTemplet->m_fMPConsumption, pSkillTemplet->m_usCBConsumption,
					pSkillTemplet->m_usFPConsumtion, false );
	#endif // UPGRADE_SKILL_SYSTEM_2013
				
#else
				SetSkillSlot( iSkillSlotID, eSkillID, pSkillTemplet->m_fMPConsumption, pSkillTemplet->m_usCBConsumption, false );
#endif
			}
			else
			{
#ifdef ARA_CHARACTER_BASE
				SetSkillSlot( iSkillSlotID, eSkillID, 0, 0, 0, false );
#else
				SetSkillSlot( iSkillSlotID, eSkillID, 0, 0, false );
#endif
			}

		} break;

	case CX2UserSkillTree::SKILL_SLOT_B1:
	case CX2UserSkillTree::SKILL_SLOT_B2:
	case CX2UserSkillTree::SKILL_SLOT_B3:
	case CX2UserSkillTree::SKILL_SLOT_B4:
		{
			const CX2SkillTree::SkillTemplet* pSkillTemplet = GetUserSkillTemplet( eSkillID);
			if(pSkillTemplet != NULL)
			{
#ifdef ARA_CHARACTER_BASE

	#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
				if ( NULL == g_pData ||
					NULL == g_pData->GetMyUser() || 
					NULL == g_pData->GetMyUser()->GetSelectUnit() || 
					NULL == g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() )
					return;

				const CX2UserSkillTree& userSkillTree = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_UserSkillTree;

				const int iSkillTempletLevel = max( 1, userSkillTree.GetSkillLevel( pSkillTemplet->m_eID ) );	/// ��ų ����

				SetSkillSlot( iSkillSlotID-(int)CX2UserSkillTree::SKILL_SLOT_B1, eSkillID, pSkillTemplet->GetSkillMPConsumptionValue( iSkillTempletLevel ), 
					pSkillTemplet->m_usCBConsumption, pSkillTemplet->m_usFPConsumtion, true );
	#else // UPGRADE_SKILL_SYSTEM_2013
				SetSkillSlot( iSkillSlotID-(int)CX2UserSkillTree::SKILL_SLOT_B1, eSkillID, pSkillTemplet->m_fMPConsumption, 
					pSkillTemplet->m_usCBConsumption, pSkillTemplet->m_usFPConsumtion, true );
	#endif // UPGRADE_SKILL_SYSTEM_2013
				
#else
				SetSkillSlot( iSkillSlotID-(int)CX2UserSkillTree::SKILL_SLOT_B1, eSkillID, pSkillTemplet->m_fMPConsumption, 
					pSkillTemplet->m_usCBConsumption, true );
#endif
			}
			else
			{
#ifdef ARA_CHARACTER_BASE
				SetSkillSlot( iSkillSlotID-(int)CX2UserSkillTree::SKILL_SLOT_B1, eSkillID, 0, 0, 0, true );
#else
				SetSkillSlot( iSkillSlotID-(int)CX2UserSkillTree::SKILL_SLOT_B1, eSkillID, 0, 0, true );
#endif
			}
		} break;
	}
}

#else	NEW_CHARACTER_CHUNG
/** @function	SetSkillSlotInfo
	@brief		��ų ���� ���� ����
	@param[in]	��ų ���� ID
	@param[in]	��ų ID
*/
void CX2UserSkillTree::SetSkillSlotInfo( int iSkillSlotID, CX2SkillTree::SKILL_ID eSkillID ) 
{
	switch( iSkillSlotID )
	{
	case CX2UserSkillTree::SKILL_SLOT_A1:
	case CX2UserSkillTree::SKILL_SLOT_A2:
	case CX2UserSkillTree::SKILL_SLOT_A3:
	case CX2UserSkillTree::SKILL_SLOT_A4:
		{
			const CX2SkillTree::SkillTemplet* pSkillTemplet = GetUserSkillTemplet( eSkillID);
			if(pSkillTemplet != NULL)
			{
				SetSkillSlot( iSkillSlotID, eSkillID, pSkillTemplet->m_fMPConsumption, false );
			}
			else
			{
				SetSkillSlot( iSkillSlotID, eSkillID, 0, false );
			}

		} break;

	case CX2UserSkillTree::SKILL_SLOT_B1:
	case CX2UserSkillTree::SKILL_SLOT_B2:
	case CX2UserSkillTree::SKILL_SLOT_B3:
	case CX2UserSkillTree::SKILL_SLOT_B4:
		{
			const CX2SkillTree::SkillTemplet* pSkillTemplet = GetUserSkillTemplet( eSkillID);
			if(pSkillTemplet != NULL)
			{
				SetSkillSlot( iSkillSlotID-(int)CX2UserSkillTree::SKILL_SLOT_B1, eSkillID, pSkillTemplet->m_fMPConsumption, true );
			}
			else
			{
				SetSkillSlot( iSkillSlotID-(int)CX2UserSkillTree::SKILL_SLOT_B1, eSkillID, 0, true );
			}
		} break;
	}
}

#endif	NEW_CHARACTER_CHUNG
//}} kimhc // 2010.12.14 //  2010-12-23 New Character CHUNG

//{{ kimhc // 2010.12.14 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG

/** @function : GetCBConsumption
	@brief : ���� �ε����� ����A,B �� ���� CB�Ҹ��� ����
	@param : const int iSlotIndex_, const bool bSlotB_
	@return : USHORT (CB �Ҹ�)
*/
USHORT CX2UserSkillTree::GetCBConsumption( const int iSlotIndex_, const bool bSlotB_ ) const
{
	if( iSlotIndex_ < 0 || iSlotIndex_ > ARRAY_SIZE( m_aEquippedSkill ) -1 )
		return 0;

	if( true == bSlotB_ )
	{
		return m_aEquippedSkillSlotB[iSlotIndex_].m_usCBConsumption;
	}
	else
	{
		return m_aEquippedSkill[iSlotIndex_].m_usCBConsumption;
	}
}

/** @function : SetCBConsumption
	@brief : ���� ���Կ� �����Ǿ��ִ� CB �Ҹ��� Set��
	@param : int iSlotIndex_, bool bSlotB_, int iCBConsumption_
*/
void CX2UserSkillTree::SetCBConsumption( int iSlotIndex_, bool bSlotB_, USHORT usCBConsumption_ )
{
	if( iSlotIndex_ < 0 || iSlotIndex_ > ARRAY_SIZE( m_aEquippedSkill ) -1 )
		return;

	if( true == bSlotB_ )
	{
		m_aEquippedSkillSlotB[iSlotIndex_].m_usCBConsumption = usCBConsumption_;
	}
	else
	{
		m_aEquippedSkill[iSlotIndex_].m_usCBConsumption = usCBConsumption_;
	}
}

#pragma region ForcePower
#ifdef ARA_CHARACTER_BASE
/** @function 	: GetFPConsumption
	@brief 		: �ش� ������ ��� ��뷮 Ȯ��
*/
USHORT CX2UserSkillTree::GetFPConsumption( const int iSlotIndex_, const bool bSlotB_ ) const
{
	if( iSlotIndex_ < 0 || iSlotIndex_ > ARRAY_SIZE( m_aEquippedSkill ) -1 )
		return 0;

	if( true == bSlotB_ )
	{
		return m_aEquippedSkillSlotB[iSlotIndex_].m_usFPConsumption;
	}
	else
	{
		return m_aEquippedSkill[iSlotIndex_].m_usFPConsumption;
	}
}

/** @function 	: SetFPConsumption
	@brief 		: �ش� ������ ��� ��뷮 ����
*/
void CX2UserSkillTree::SetFPConsumption( int iSlotIndex_, bool bSlotB_, USHORT usFPConsumption_ )
{
	if( iSlotIndex_ < 0 || iSlotIndex_ > ARRAY_SIZE( m_aEquippedSkill ) -1 )
		return;

	if( true == bSlotB_ )
	{
		m_aEquippedSkillSlotB[iSlotIndex_].m_usFPConsumption = usFPConsumption_;
	}
	else
	{
		m_aEquippedSkill[iSlotIndex_].m_usFPConsumption = usFPConsumption_;
	}
}

/** @function 	: GetFPConsumption
	@brief 		: �ش� ������ ��� ��뷮 Ȯ��
*/
USHORT CX2UserSkillTree::GetFPGainCount( const int iSlotIndex_, const bool bSlotB_ ) const
{
	if( iSlotIndex_ < 0 || iSlotIndex_ > ARRAY_SIZE( m_aEquippedSkill ) -1 )
		return 0;

	if( true == bSlotB_ )
	{
		return m_aEquippedSkillSlotB[iSlotIndex_].m_usFPGainCount;
	}
	else
	{
		return m_aEquippedSkill[iSlotIndex_].m_usFPGainCount;
	}
}

/** @function 	: SetFPConsumption
	@brief 		: �ش� ������ ��� ��뷮 ����
*/
void CX2UserSkillTree::SetFPGainCount( int iSlotIndex_, bool bSlotB_, USHORT usFPGainCount_ )
{
	if( iSlotIndex_ < 0 || iSlotIndex_ > ARRAY_SIZE( m_aEquippedSkill ) -1 )
		return;

	if( true == bSlotB_ )
	{
		m_aEquippedSkillSlotB[iSlotIndex_].m_usFPGainCount = usFPGainCount_;
	}
	else
	{
		m_aEquippedSkill[iSlotIndex_].m_usFPGainCount = usFPGainCount_;
	}
}
#endif
#pragma endregion ���


/** @function	SetSkillSlot
	@brief		��ų ���� ����
	@param[in]	���� �ε���
	@param[in]	��ų ID
	@param[in]	�Ҹ� ����
	@param[in]	�Ҹ� ĳ����
	@param[in]	�Ҹ� ���
	@param[in]	B ���� ���� �ƴ���
*/
#ifdef ARA_CHARACTER_BASE
bool CX2UserSkillTree::SetSkillSlot( int iSlotIndex_, CX2SkillTree::SKILL_ID eSkillID_, float fMPConsume_, USHORT usCBConsume_,
									 USHORT usFPConsume, bool bSlotB_ )
#else
bool CX2UserSkillTree::SetSkillSlot( int iSlotIndex_, CX2SkillTree::SKILL_ID eSkillID_, float fMPConsume_, USHORT usCBConsume_, bool bSlotB_ )
#endif
{
	if( iSlotIndex_ < 0 ||
		iSlotIndex_ > ARRAY_SIZE( m_aEquippedSkill ) - 1 )
	{
		return false;
	}

	//if( true == bSlotB &&
	//	false == GetEnabledSkillSlotB() )
	//{
	//	return false;
	//}

	if( true == bSlotB_ )
	{
		m_aEquippedSkillSlotB[iSlotIndex_].m_eID = eSkillID_;
		m_aEquippedSkillSlotB[iSlotIndex_].m_fCoolTimeLeft = 0.f;
		m_aEquippedSkillSlotB[iSlotIndex_].m_fMPConsumption = fMPConsume_;
		m_aEquippedSkillSlotB[iSlotIndex_].m_usCBConsumption = usCBConsume_;
#ifdef ARA_CHARACTER_BASE
		m_aEquippedSkillSlotB[iSlotIndex_].m_usFPConsumption = usFPConsume;
#endif
	}
	else
	{
		m_aEquippedSkill[iSlotIndex_].m_eID = eSkillID_;
		m_aEquippedSkill[iSlotIndex_].m_fCoolTimeLeft = 0.f;
		m_aEquippedSkill[iSlotIndex_].m_fMPConsumption = fMPConsume_;
		m_aEquippedSkill[iSlotIndex_].m_usCBConsumption = usCBConsume_;
#ifdef ARA_CHARACTER_BASE
		m_aEquippedSkillSlotB[iSlotIndex_].m_usFPConsumption = usFPConsume;
#endif
	}

	return true;
}


#else	NEW_CHARACTER_CHUNG
/** @function	SetSkillSlot
	@brief		��ų ���� ����
	@param[in]	���� �ε���
	@param[in]	��ų ID
	@param[in]	�Ҹ� ����
	@param[in]	B ���� ���� �ƴ���
*/
bool CX2UserSkillTree::SetSkillSlot( int iSlotIndex, CX2SkillTree::SKILL_ID eSkillID, float fMPConsumption, bool bSlotB )
{
	if( iSlotIndex < 0 ||
		iSlotIndex > ARRAY_SIZE( m_aEquippedSkill ) - 1 )
	{
		return false;
	}

	//if( true == bSlotB &&
	//	false == GetEnabledSkillSlotB() )
	//{
	//	return false;
	//}

	if( true == bSlotB )
	{
		m_aEquippedSkillSlotB[iSlotIndex].m_eID = eSkillID;
		m_aEquippedSkillSlotB[iSlotIndex].m_fCoolTimeLeft = 0.f;
		m_aEquippedSkillSlotB[iSlotIndex].m_fMPConsumption = fMPConsumption;
	}
	else
	{
		m_aEquippedSkill[iSlotIndex].m_eID = eSkillID;
		m_aEquippedSkill[iSlotIndex].m_fCoolTimeLeft = 0.f;
		m_aEquippedSkill[iSlotIndex].m_fMPConsumption = fMPConsumption;
	}

	return true;
}

#endif	NEW_CHARACTER_CHUNG
//}} kimhc // 2010.12.14 //  2010-12-23 New Character CHUNG

/** @function	GetSkillSlot
	@brief		��ų ���� ���
	@param[in]	���� �ε���
	@param[in]	B �������� �ƴ���
*/
const CX2UserSkillTree::SkillSlotData* CX2UserSkillTree::GetSkillSlot( int iSlotIndex, bool bSlotB ) const
{
	if( iSlotIndex < 0 ||
		iSlotIndex > ARRAY_SIZE( m_aEquippedSkill ) -1 )
	{
		return NULL;
	}
	
	if( iSlotIndex >= 4 )
	{
		bSlotB = true;
		iSlotIndex -= 4;
	}
	
	if( true == bSlotB )
	{
		return &m_aEquippedSkillSlotB[iSlotIndex];
	}
	else
	{
		return &m_aEquippedSkill[iSlotIndex];
	}
}

/** @function : GetSkillSlot
	@brief : ��ų���Ե������� �����͸� ��� �Լ�
	@param : ��ųID(eSkillID_)
	@return : ��ų���Ե����� ������
*/
const CX2UserSkillTree::SkillSlotData* CX2UserSkillTree::GetSkillSlot( const CX2SkillTree::SKILL_ID eSkillID_ ) const
{
	const UINT uiArraySize = ARRAY_SIZE( m_aEquippedSkill );

	for ( UINT uiIndex = 0; uiIndex < uiArraySize; ++uiIndex )
	{
		if ( eSkillID_ == m_aEquippedSkill[uiIndex].m_eID )
			return &m_aEquippedSkill[uiIndex];
		else if ( eSkillID_ == m_aEquippedSkillSlotB[uiIndex].m_eID )
			return &m_aEquippedSkillSlotB[uiIndex];			
	}

	return NULL;
}

/** @function : ChangeMpConsumptionPercent
	@brief : Mp �Ҹ��� ����
	@param : Mp �Ҹ����� ������ ��ųID, ���� ����(fMultiplier)
*/
void CX2UserSkillTree::ChangeMpConsumptionPercent( const CX2SkillTree::SKILL_ID eSkillID_, const float fMultiplier_ )
{
	const UINT uiArraySize = ARRAY_SIZE( m_aEquippedSkill );

	for ( UINT uiIndex = 0; uiIndex < uiArraySize; ++uiIndex )
	{
		if ( eSkillID_ == m_aEquippedSkill[uiIndex].m_eID )
		{
			m_aEquippedSkill[uiIndex].m_fMPConsumption *= fMultiplier_;
			break;
		}
		else if ( eSkillID_ == m_aEquippedSkillSlotB[uiIndex].m_eID )
		{
			m_aEquippedSkillSlotB[uiIndex].m_fMPConsumption *= fMultiplier_;
			break;
		}
	}
}

/** @function	SetMpConsumption
	@brief		���� �Ҹ� ����
	@param[in]	���� �ε���
	@param[in]	B ����?
	@param[in]	���� �Ҹ�
*/
void CX2UserSkillTree::SetMpConsumption( int iSlotIndex_, bool bSlotB_, float fMpConsumption_ )
{
	if( iSlotIndex_ < 0 || iSlotIndex_ > ARRAY_SIZE( m_aEquippedSkill ) -1 )
		return;

	if( true == bSlotB_ )
	{
		m_aEquippedSkillSlotB[iSlotIndex_].m_fMPConsumption = fMpConsumption_;
	}
	else
	{
		m_aEquippedSkill[iSlotIndex_].m_fMPConsumption = fMpConsumption_;
	}
}

/** @function	GetSkillStat
	@brief		��ų Stat ���
	@return		CX2Stat::Stat
*/
CX2Stat::Stat CX2UserSkillTree::GetSkillStat() const
{
	CX2Stat::Stat retStat;

	BOOST_TEST_FOREACH( const SkillDataMap::value_type&, value, m_mapSkillAcquired )
	{
		const UserSkillData& userSkillData = value.second;
		
#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( value.first );
#else // UPGRADE_SKILL_SYSTEM_2013
		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( value.first, userSkillData.m_iSkillLevel );
#endif // UPGRADE_SKILL_SYSTEM_2013
		
		if( pSkillTemplet == NULL )
			continue;

		switch( pSkillTemplet->m_eType )
		{
		case CX2SkillTree::ST_PASSIVE_PHYSIC_ATTACK: 
		case CX2SkillTree::ST_PASSIVE_MAGIC_ATTACK:  
		case CX2SkillTree::ST_PASSIVE_MAGIC_DEFENCE: 
		case CX2SkillTree::ST_PASSIVE_PHYSIC_DEFENCE:
			{
	#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
				retStat.AddStat( pSkillTemplet->GetSkillStatValue( userSkillData.m_iSkillLevel ), false );
	#else // UPGRADE_SKILL_SYSTEM_2013
				retStat.AddStat( pSkillTemplet->m_Stat, false );
	#endif // UPGRADE_SKILL_SYSTEM_2013
			} break;
		}
	}

	return retStat;
}

#ifdef GUILD_SKILL
//{{ oasis907 : ����� //// 2009-11-12 //// 
/** @function	GetGuildSkillStat
	@brief		��� ��ų ���� ���
	@return		CX2Stat:Stat
*/
CX2Stat::Stat CX2UserSkillTree::GetGuildSkillStat(int iGuildMemberGrade)
{
	CX2Stat::Stat retStat;

	// oasis907 : ����� [2009.12.8] // ��� ���
	if(iGuildMemberGrade == CX2GuildManager::GUG_INVALID ||
		iGuildMemberGrade >= g_pData->GetGuildManager()->GetLimitGuildGrade()) // ������ ĿƮ���� ��� ��� ���
	{
		return retStat;
	}

	BOOST_TEST_FOREACH( const SkillDataMap::value_type&, value, m_mapGuildSkillAcquired )
	{
		const UserSkillData& userSkillData = value.second;

	#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( value.first );

		if( pSkillTemplet == NULL )
			continue;

		switch( pSkillTemplet->m_eType )
		{
		case CX2SkillTree::ST_GUILD_PASSIVE_PHYSIC_ATTACK_MAGIC_DEFENCE: 
		case CX2SkillTree::ST_GUILD_PASSIVE_MAGIC_ATTCK_PHYSIC_DEFENCE:  
			{
				retStat.AddStat( pSkillTemplet->GetSkillStatValue( userSkillData.m_iSkillLevel ), false );
			} break;
		}
	#else // UPGRADE_SKILL_SYSTEM_2013
		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( value.first, userSkillData.m_iSkillLevel );

		if( pSkillTemplet == NULL )
			continue;

		switch( pSkillTemplet->m_eType )
		{
		case CX2SkillTree::ST_GUILD_PASSIVE_PHYSIC_ATTACK_MAGIC_DEFENCE: 
		case CX2SkillTree::ST_GUILD_PASSIVE_MAGIC_ATTCK_PHYSIC_DEFENCE:  
			{
				retStat.AddStat( pSkillTemplet->m_Stat, false );
			} break;
		}
	#endif // UPGRADE_SKILL_SYSTEM_2013
	}

	return retStat;
}
//}} oasis907 : ����� //// 2009-11-12 //// 
#endif GUILD_SKILL

/** @function	CalcUsedSPointAndCSPoint
	@brief		���� Skill Point�� ĳ�� ��ų ����Ʈ �� ���
	@param[out]	iSPont : ��ų ����Ʈ
	@param[out]	iCSPoint : ĳ�� ��ų ����Ʈ
*/
void CX2UserSkillTree::CalcUsedSPointAndCSPoint( OUT int& iSPoint, OUT int& iCSPoint )
{
#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
	if ( NULL == g_pData || NULL == g_pData->GetSkillTree() )
		return;

	iSPoint		= 0;
	iCSPoint	= 0;

	for( SkillDataMap::iterator mit = m_mapSkillAcquired.begin(); mit != m_mapSkillAcquired.end(); ++mit )
	{
#ifdef ADDED_RELATIONSHIP_SYSTEM
		/// ��ȥ ��ų�̶��, ��ų ����Ʈ ���� ����
		CX2SkillTree::SKILL_ID eSkillID = mit->first;

		if ( CX2SkillTree::SI_ETC_WS_COMMON_LOVE == eSkillID )
			continue;
#endif ADDED_RELATIONSHIP_SYSTEM
#ifndef ADDED_RELATIONSHIP_SYSTEM
		CX2SkillTree::SKILL_ID eSkillID = mit->first;
#endif ADDED_RELATIONSHIP_SYSTEM 
		
		/// ��ų ���ø� ��ȯ
		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID );

		if ( NULL != pSkillTemplet )
		{
			const UserSkillData& userSkillData = mit->second;

			/// ����� ��ų ����Ʈ ���� ( ĳ�� ��ų ����Ʈ�� ���� )
			for( int i = 0 ; i < userSkillData.m_iSkillLevel ; ++i )
			{
				if( i == 0 )	/// ������ 1�� ��
				{
					if( true == g_pData->GetSkillTree()->isDefaultSkill( eSkillID ) )	/// �⺻ ��ų�̸� ����
						continue;

					iSPoint += pSkillTemplet->m_iRequireLearnSkillPoint;	/// ���� ����Ʈ�� ����
				}
				else
				{
					iSPoint += pSkillTemplet->m_iRequireUpgradeSkillPoint;	/// ��ȭ ����Ʈ�� ����
				}
			}

			iCSPoint	+= userSkillData.m_iSkillCSPoint;	/// ĳ�� ��ų ����Ʈ�� ����
			iSPoint		-= userSkillData.m_iSkillCSPoint;	/// ����� ĳ�� ��ų ����Ʈ�� ���ܽ���, �Ϲ� ��ų ����Ʈ�� ����
			
		}
	}

#else // UPGRADE_SKILL_SYSTEM_2013

	iSPoint = 0;
	iCSPoint = 0;

	for( SkillDataMap::iterator mit = m_mapSkillAcquired.begin(); mit != m_mapSkillAcquired.end(); ++mit )
	{
#ifdef ADDED_RELATIONSHIP_SYSTEM
		/// ��ȥ ��ų�̶��, ��ų ����Ʈ ���� ����
		CX2SkillTree::SKILL_ID eSkillID = mit->first;

		if ( CX2SkillTree::SI_ETC_WS_COMMON_LOVE == eSkillID )
			continue;
#endif ADDED_RELATIONSHIP_SYSTEM

		const UserSkillData& userSkillData = mit->second;

		if( userSkillData.m_iSkillLevel > 0 )
		{
			iSPoint		+= userSkillData.m_iSkillLevel - userSkillData.m_iSkillCSPoint;
			iCSPoint	+= userSkillData.m_iSkillCSPoint;
		}
	}

#endif // UPGRADE_SKILL_SYSTEM_2013
}

/** @function	CalcCumulativeUsedSPointOnEachTier
	@brief		tier���� �Ҹ�� ���� SP+CSP�� ������ش�
	@param[out]	vecTierSPoint : 
*/
void CX2UserSkillTree::CalcCumulativeUsedSPointOnEachTier( OUT std::vector<int>& vecTierSPoint )
{

	int iMaxTierIndex = 0;
	for( SkillDataMap::iterator mit = m_mapSkillAcquired.begin(); mit != m_mapSkillAcquired.end(); ++mit )
	{
		const UserSkillData& userSkillData = mit->second;
		if( userSkillData.m_iSkillLevel > 0 )
		{
			const CX2SkillTree::SkillTreeTemplet* pSkillTreeTemplet = g_pData->GetSkillTree()->GetSkillTreeTemplet( (int)m_iUnitClass, mit->first );
			if( NULL != pSkillTreeTemplet )
			{
				if( iMaxTierIndex < pSkillTreeTemplet->m_iTier )
					iMaxTierIndex = pSkillTreeTemplet->m_iTier;
			}
		}
	}

	vecTierSPoint.resize(0);
	for( int i=0; i<= iMaxTierIndex; i++ )
	{
		vecTierSPoint.push_back( 0 );
	}


	
	for( SkillDataMap::iterator mit = m_mapSkillAcquired.begin(); mit != m_mapSkillAcquired.end(); ++mit )
	{
		const UserSkillData& userSkillData = mit->second;
		if( userSkillData.m_iSkillLevel > 0 )
		{
			const CX2SkillTree::SkillTreeTemplet* pSkillTreeTemplet = g_pData->GetSkillTree()->GetSkillTreeTemplet( (int)m_iUnitClass, mit->first );
			if( NULL != pSkillTreeTemplet )
			{
				vecTierSPoint[ pSkillTreeTemplet->m_iTier ] += userSkillData.m_iSkillLevel; 

//{{ kimhc // 2010.8.17 // ��ų �ǵ����Ⱑ �Ǹ� �ȵǴ� ��Ȳ���� �ǵ����Ⱑ �Ǿ� ���������� ��ųƮ���� �Ǵ� ������ �ذ��� ���Դϴ�.
#ifdef	SERV_RESET_SKILL_BUG_FIX
				vecTierSPoint[ pSkillTreeTemplet->m_iTier ] -= userSkillData.m_iSkillCSPoint; 
#endif	SERV_RESET_SKILL_BUG_FIX
//}} kimhc // 2010.8.17 // ��ų �ǵ����Ⱑ �Ǹ� �ȵǴ� ��Ȳ���� �ǵ����Ⱑ �Ǿ� ���������� ��ųƮ���� �Ǵ� ������ �ذ��� ���Դϴ�.
			}
		}
	}


	int iCumulativeTierSPoint = 0;
	BOOST_TEST_FOREACH( int&, iTierSPoint, vecTierSPoint )
	{
		iCumulativeTierSPoint += iTierSPoint;
		iTierSPoint = iCumulativeTierSPoint;
	}
}


#ifdef GUILD_SKILL
//{{ oasis907 : ����� //// 2009-11-12 //// 
void CX2UserSkillTree::CalcUsedGuildSPointAndCSPoint( OUT int& iSPoint, OUT int& iCSPoint )
{
	iSPoint = 0;
	iCSPoint = 0;

	for( SkillDataMap::iterator mit = m_mapGuildSkillAcquired.begin(); mit != m_mapGuildSkillAcquired.end(); ++mit )
	{
		const UserSkillData& userSkillData = mit->second;

		if( userSkillData.m_iSkillLevel > 0 )
		{
			iSPoint		+= userSkillData.m_iSkillLevel - userSkillData.m_iSkillCSPoint;
			iCSPoint	+= userSkillData.m_iSkillCSPoint;
		}

	}
}



// tier���� �Ҹ�� ���� SP+CSP�� ������ش�
void CX2UserSkillTree::CalcCumulativeUsedGuildSPointOnEachTier( OUT std::vector<int>& vecTierSPoint )
{

	int iMaxTierIndex = 0;
	for( SkillDataMap::iterator mit = m_mapGuildSkillAcquired.begin(); mit != m_mapGuildSkillAcquired.end(); ++mit )
	{
		const UserSkillData& userSkillData = mit->second;
		if( userSkillData.m_iSkillLevel > 0 )
		{
			const CX2SkillTree::SkillTreeTemplet* pSkillTreeTemplet = g_pData->GetSkillTree()->GetGuildSkillTreeTemplet( (int)m_iGuildClass, mit->first );
			if( NULL != pSkillTreeTemplet )
			{
				if( iMaxTierIndex < pSkillTreeTemplet->m_iTier )
					iMaxTierIndex = pSkillTreeTemplet->m_iTier;
			}
		}
	}

	vecTierSPoint.resize(0);
	for( int i=0; i<= iMaxTierIndex; i++ )
	{
		vecTierSPoint.push_back( 0 );
	}



	for( SkillDataMap::iterator mit = m_mapGuildSkillAcquired.begin(); mit != m_mapGuildSkillAcquired.end(); ++mit )
	{
		const UserSkillData& userSkillData = mit->second;
		if( userSkillData.m_iSkillLevel > 0 )
		{
			const CX2SkillTree::SkillTreeTemplet* pSkillTreeTemplet = g_pData->GetSkillTree()->GetGuildSkillTreeTemplet( (int)m_iGuildClass, mit->first );
			if( NULL != pSkillTreeTemplet )
			{
				vecTierSPoint[ pSkillTreeTemplet->m_iTier ] += userSkillData.m_iSkillLevel; 
				//{{ kimhc // 2010.8.17 // ��ų �ǵ����Ⱑ �Ǹ� �ȵǴ� ��Ȳ���� �ǵ����Ⱑ �Ǿ� ���������� ��ųƮ���� �Ǵ� ������ �ذ��� ���Դϴ�.
#ifdef	SERV_RESET_SKILL_BUG_FIX
				vecTierSPoint[ pSkillTreeTemplet->m_iTier ] -= userSkillData.m_iSkillCSPoint; 
#endif	SERV_RESET_SKILL_BUG_FIX
				//}} kimhc // 2010.8.17 // ��ų �ǵ����Ⱑ �Ǹ� �ȵǴ� ��Ȳ���� �ǵ����Ⱑ �Ǿ� ���������� ��ųƮ���� �Ǵ� ������ �ذ��� ���Դϴ�.
			}
		}
	}


	int iCumulativeTierSPoint = 0;
	BOOST_TEST_FOREACH( int&, iTierSPoint, vecTierSPoint )
	{
		iCumulativeTierSPoint += iTierSPoint;
		iTierSPoint = iCumulativeTierSPoint;
	}
}
//}} oasis907 : ����� //// 2009-11-12 //// 
#endif GUILD_SKILL


// cash skill point�� ���� ��ų�� ������ �����Ѵ�
#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ
void CX2UserSkillTree::ExpireCashSkillPoint( OUT const std::map<int, int> mapHaveSKill_ )
#else // UPGRADE_SKILL_SYSTEM_2013
void CX2UserSkillTree::ExpireCashSkillPoint()
#endif // UPGRADE_SKILL_SYSTEM_2013
{
#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ

	/// ��� ��ų ��� �ʱ�ȭ
	Reset( true, false, false, false );

	/// �׳�ý� ���� ������ ����� ��ų��� ����
	std::map<int, int>::const_iterator it = mapHaveSKill_.begin();

	for ( it; it != mapHaveSKill_.end(); ++it  )
	{
		const CX2SkillTree:: SKILL_ID	eSkillID	= static_cast<CX2SkillTree::SKILL_ID>( it->first );
		const int						eSkillLevel	= it->second;

		SetSkillLevelAndCSP( eSkillID, eSkillLevel, 0 );
	}

#else // UPGRADE_SKILL_SYSTEM_2013 

	for( SkillDataMap::iterator mit = m_mapSkillAcquired.begin(); mit != m_mapSkillAcquired.end(); ++mit )
	{
		UserSkillData& userSkillData = mit->second;

		if( userSkillData.m_iSkillCSPoint > 0 )
		{
			userSkillData.m_iSkillLevel		-= userSkillData.m_iSkillCSPoint;
			userSkillData.m_iSkillCSPoint	= 0;
		}
	}

#endif UPGRADE_SKILL_SYSTEM_2013


	// ������ ��ų �߿��� skill level�� 0������ ��ų�� ������ Ż����Ų��.
	for( int i=0; i<ARRAY_SIZE( m_aEquippedSkill ); ++i )
	{
		SkillSlotData& skillSlotData = m_aEquippedSkill[i];
		if( false == DoIHaveThisSkill( skillSlotData.m_eID ) )
		{
//{{ kimhc // 2010.12.14 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
#ifdef ARA_CHARACTER_BASE
			SetSkillSlot( i, CX2SkillTree::SI_NONE, 0, 0, 0, false );
#else
			SetSkillSlot( i, CX2SkillTree::SI_NONE, 0, 0, false );
#endif //ARA_CHARACTER_BASE
#else	NEW_CHARACTER_CHUNG
			SetSkillSlot( i, CX2SkillTree::SI_NONE, 0, false );
#endif	NEW_CHARACTER_CHUNG
//}} kimhc // 2010.12.14 //  2010-12-23 New Character CHUNG
		}
	}


	for( int i=0; i<ARRAY_SIZE( m_aEquippedSkillSlotB ); i++ )
	{
		SkillSlotData& skillSlotData = m_aEquippedSkillSlotB[i];
		if( false == DoIHaveThisSkill( skillSlotData.m_eID ) )
		{
			//{{ kimhc // 2010.12.14 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
#ifdef ARA_CHARACTER_BASE
			SetSkillSlot( i, CX2SkillTree::SI_NONE, 0, 0, 0, true );
#else
			SetSkillSlot( i, CX2SkillTree::SI_NONE, 0, 0, true );
#endif //ARA_CHARACTER_BASE
#else	NEW_CHARACTER_CHUNG
			SetSkillSlot( i, CX2SkillTree::SI_NONE, 0, true );
#endif	NEW_CHARACTER_CHUNG
			//}} kimhc // 2010.12.14 //  2010-12-23 New Character CHUNG
		}
	}
}
#ifdef GUILD_SKILL
//{{ oasis907 : ����� [2009.11.27] //
// cash skill point�� ���� ��ų�� ������ �����Ѵ�
void CX2UserSkillTree::ExpireGuildCashSkillPoint()
{
	for( SkillDataMap::iterator mit = m_mapGuildSkillAcquired.begin(); mit != m_mapGuildSkillAcquired.end(); ++mit )
	{
		UserSkillData& userSkillData = mit->second;

		if( userSkillData.m_iSkillCSPoint > 0 )
		{
			userSkillData.m_iSkillLevel		-= userSkillData.m_iSkillCSPoint;
			userSkillData.m_iSkillCSPoint	= 0;
		}
	}
    // oasis907 : ����� [2009.11.27] // ��� ��ų�� ��Ƽ�갡 �����Ƿ� �ʿ����
/*
	// ������ ��ų �߿��� skill level�� 0������ ��ų�� ������ Ż����Ų��.
	for( int i=0; i<ARRAY_SIZE( m_aEquippedSkill ); i++ )
	{
		SkillSlotData& skillSlotData = m_aEquippedSkill[i];
		if( false == DoIHaveThisSkill( skillSlotData.m_eID ) )
		{
			SetSkillSlot( i, CX2SkillTree::SI_NONE, 0, false );
		}
	}


	for( int i=0; i<ARRAY_SIZE( m_aEquippedSkillSlotB ); i++ )
	{
		SkillSlotData& skillSlotData = m_aEquippedSkillSlotB[i];
		if( false == DoIHaveThisSkill( skillSlotData.m_eID ) )
		{
			SetSkillSlot( i, CX2SkillTree::SI_NONE, 0, false );
		}
	}
*/
}
#endif GUILD_SKILL




//}} oasis907 : ����� [2009.11.27] //

bool CX2UserSkillTree::IsSkillUnsealed( CX2SkillTree::SKILL_ID eSkillID )
{
	std::set< CX2SkillTree::SKILL_ID >::iterator it = m_setUnsealedSkillID.find( eSkillID );
	if( it != m_setUnsealedSkillID.end() )
	{
		return true;
	}

	return false;
}



void CX2UserSkillTree::AddSkillUnsealed( CX2SkillTree::SKILL_ID eSkillID )
{
	if( true == IsSkillUnsealed( eSkillID ) )
		return;

	m_setUnsealedSkillID.insert( eSkillID );
}


bool CX2UserSkillTree::DoIHaveThisSkill( CX2SkillTree::SKILL_ID eSkillID )
{
#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����

	/// ���� ��ų ����Ʈ�� ���ڵ� ��ų���� �˻�
	if( GetSkillLevel(eSkillID) > 0 )
		return true;
	/// �ƴ϶��, ��ųâ���� ��ų ������ �ø��� �ִ� ��ų���� �˻�
	else if (	NULL != g_pData &&
				NULL != g_pData->GetUIManager() &&
				NULL != g_pData->GetSkillTree() &&
				NULL != g_pData->GetUIManager()->GetUISkillTree() &&
				true == g_pData->GetUIManager()->GetUISkillTree()->GetNowLearnSkill() )
	{
		/// �ø� ��Ͽ� �ش� ��ų�� ������, ���������� ó��
		const std::map<int, KGetSkillInfo> mapSkillInfo = g_pData->GetSkillTree()->GetMapSkillInfo();

		const std::map<int, KGetSkillInfo>::const_iterator it = mapSkillInfo.find( static_cast<int>( eSkillID ) );

		if ( it != mapSkillInfo.end() )
			return true;
	}

	return false;
#else // UPGRADE_SKILL_SYSTEM_2013

	if( GetSkillLevel(eSkillID) <= 0 )
		return false;

	return true;

#endif // UPGRADE_SKILL_SYSTEM_2013
}


bool CX2UserSkillTree::CanIResetThisSkill( CX2SkillTree::SKILL_ID eSkillID )
{
#ifdef GUILD_SKILL
//{{ oasis907 : ����� //// 2009-11-17 //// 
	if(IsGuildSkill(eSkillID))
	{
	#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
		if( true == g_pData->GetSkillTree()->IsUnitTypeDefaultSkill( eSkillID ) && GetSkillLevel( eSkillID, true ) <= 1)	// �¿� ���� : 1���������� ���� �� �ְ�..
	#else // UPGRADE_SKILL_SYSTEM_2013
		if( true == g_pData->GetSkillTree()->IsUnitTypeDefaultSkill( eSkillID ) && GetSkillLevel( eSkillID ) <= 1)	// �¿� ���� : 1���������� ���� �� �ְ�..
	#endif // UPGRADE_SKILL_SYSTEM_2013
			return false;

		int iMySkillLevel = GetSkillLevel( eSkillID );
		if( iMySkillLevel <= 0 )
			return false;

	#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID );
	#else // UPGRADE_SKILL_SYSTEM_2013
		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID, iMySkillLevel );
	#endif // UPGRADE_SKILL_SYSTEM_2013
		
		if( NULL == pSkillTemplet )
			return false;


		const CX2SkillTree::SkillTreeTemplet* pSkillTreeTemplet = g_pData->GetSkillTree()->GetGuildSkillTreeTemplet( (int)m_iGuildClass, eSkillID );
		if( NULL == pSkillTreeTemplet )
			return false;


#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
		CX2SkillTree::SKILL_ID eFollowingSkillID = static_cast<CX2SkillTree::SKILL_ID>(pSkillTreeTemplet->m_iFollowingSkill);
		if( true == DoIHaveThisSkill( eFollowingSkillID ) )
			return false;
#else // UPGRADE_SKILL_SYSTEM_2013
		for( UINT i=0; i<pSkillTreeTemplet->m_vecFollowingSkill.size(); i++ )
		{
			CX2SkillTree::SKILL_ID eFollowingSkillID = (CX2SkillTree::SKILL_ID) pSkillTreeTemplet->m_vecFollowingSkill[i];
			if( true == DoIHaveThisSkill( eFollowingSkillID ) )
				return false;
		}
#endif // UPGRADE_SKILL_SYSTEM_2013


		// ��ų �ǵ����⸦ �ϰ� �� �Ŀ� ���� tier�� ��ų�� �� ��� �Ǵ� ��� ��ų ������ �� �� ����
		std::vector<int> vecCumulativeTierSPoint;
		CalcCumulativeUsedGuildSPointOnEachTier( vecCumulativeTierSPoint );


		const int MAGIC_SKILL_POINT_PER_TIER = 5;
		for( UINT i=pSkillTreeTemplet->m_iTier+1; i<vecCumulativeTierSPoint.size(); i++ )
		{
			if( vecCumulativeTierSPoint[i-1]-1 < (int)i * MAGIC_SKILL_POINT_PER_TIER )
			{
				return false;
			}
		}

		return true;
	}
//}} oasis907 : ����� //// 2009-11-17 //// 
#endif GUILD_SKILL

#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
	if( true == g_pData->GetSkillTree()->IsUnitTypeDefaultSkill( eSkillID ) && GetSkillLevel( eSkillID, true ) <= 1)	// �¿� ���� : 1���������� ���� �� �ְ�..
#else // UPGRADE_SKILL_SYSTEM_2013
	if( true == g_pData->GetSkillTree()->IsUnitTypeDefaultSkill( eSkillID ) && GetSkillLevel( eSkillID ) <= 1)	// �¿� ���� : 1���������� ���� �� �ְ�..
#endif // UPGRADE_SKILL_SYSTEM_2013
		return false;


	int iMySkillLevel = GetSkillLevel( eSkillID );
	if( iMySkillLevel <= 0 )
		return false;

#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
	const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID );
#else // UPGRADE_SKILL_SYSTEM_2013
	const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID, iMySkillLevel );
#endif // UPGRADE_SKILL_SYSTEM_2013
	
	if( NULL == pSkillTemplet )
		return false;


	const CX2SkillTree::SkillTreeTemplet* pSkillTreeTemplet = g_pData->GetSkillTree()->GetSkillTreeTemplet( (int)m_iUnitClass, eSkillID );
	if( NULL == pSkillTreeTemplet )
		return false;


#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
		CX2SkillTree::SKILL_ID eFollowingSkillID = static_cast<CX2SkillTree::SKILL_ID>( pSkillTreeTemplet->m_iFollowingSkill );
		if( true == DoIHaveThisSkill( eFollowingSkillID ) )
			return false;
#else // UPGRADE_SKILL_SYSTEM_2013
	for( UINT i=0; i<pSkillTreeTemplet->m_vecFollowingSkill.size(); i++ )
	{
		CX2SkillTree::SKILL_ID eFollowingSkillID = (CX2SkillTree::SKILL_ID) pSkillTreeTemplet->m_vecFollowingSkill[i];
		if( true == DoIHaveThisSkill( eFollowingSkillID ) )
			return false;
	}
#endif // UPGRADE_SKILL_SYSTEM_2013



	// ��ų �ǵ����⸦ �ϰ� �� �Ŀ� ���� tier�� ��ų�� �� ��� �Ǵ� ��� ��ų ������ �� �� ����
	std::vector<int> vecCumulativeTierSPoint;
	CalcCumulativeUsedSPointOnEachTier( vecCumulativeTierSPoint );


	const int MAGIC_SKILL_POINT_PER_TIER = 5;
	for( UINT i=pSkillTreeTemplet->m_iTier+1; i<vecCumulativeTierSPoint.size(); i++ )
	{
		if( vecCumulativeTierSPoint[i-1]-1 < (int)i * MAGIC_SKILL_POINT_PER_TIER )
		{
			return false;
		}
	}

	return true;
}


bool CX2UserSkillTree::CanILearnThisSkill( CX2SkillTree::SKILL_ID eSkillID, bool bCheckSP /*= true*/ )
{
#ifdef GUILD_SKILL
//{{ oasis907 : ����� //// 2009-11-17 //// 
	if(IsGuildSkill(eSkillID))
	{
		if( NULL == g_pData ||
			NULL == g_pData->GetMyUser() ||
			NULL == g_pData->GetMyUser()->GetSelectUnit() ||
			NULL == g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() )
		{
			return false;
		}

		CX2Unit::UnitData* pUnitData = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData();

		if(bCheckSP == true)
		{
			if( pUnitData->m_iGuildSPoint + pUnitData->m_iGuildCSPoint < 1 )
				return false;
		}

		if( true == IsMaxSkillLevel( eSkillID ) )
			return false;

		int iSkillLevelToLearn = GetSkillLevel( eSkillID ) + 1;

	#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID );
	#else // UPGRADE_SKILL_SYSTEM_2013
		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID, iSkillLevelToLearn );
	#endif // UPGRADE_SKILL_SYSTEM_2013
		
		if( NULL == pSkillTemplet )
			return false;



		// ������������ ���� ��ų�� ȹ���� �� ����
		if( true == pSkillTemplet->m_bBornSealed )
		{
			if( false == IsSkillUnsealed( eSkillID ) )
			{
				return false;
			}
		}

		const CX2SkillTree::SkillTreeTemplet* pSkillTreeTemplet = g_pData->GetSkillTree()->GetGuildSkillTreeTemplet( (int)m_iGuildClass, eSkillID );
		if( NULL == pSkillTreeTemplet )
			return false;


		const int MAGIC_SKILL_POINT_PER_TIER = 5;
		int iUsedSPoint = 0;
		int iUsedCSPoint = 0;
		CalcUsedGuildSPointAndCSPoint( iUsedSPoint, iUsedCSPoint );
		if( iUsedSPoint + iUsedCSPoint < pSkillTreeTemplet->m_iTier * MAGIC_SKILL_POINT_PER_TIER )
		{
			return false;
		}



#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
		CX2SkillTree::SKILL_ID ePrecedingSkillID = static_cast<CX2SkillTree::SKILL_ID>( pSkillTreeTemplet->m_iPrecedingSkill );

		if( CX2SkillTree::SI_NONE != ePrecedingSkillID &&false == IsMaxSkillLevel( ePrecedingSkillID ) )
			return false;
#else // UPGRADE_SKILL_SYSTEM_2013
		for( UINT i=0; i<pSkillTreeTemplet->m_vecPrecedingSkill.size(); i++ )
		{
			CX2SkillTree::SKILL_ID ePrecedingSkillID = (CX2SkillTree::SKILL_ID) pSkillTreeTemplet->m_vecPrecedingSkill[i];
			if( false == IsMaxSkillLevel( ePrecedingSkillID ) )
				return false;
		}
#endif // UPGRADE_SKILL_SYSTEM_2013

		return true;

	}
//}} oasis907 : ����� //// 2009-11-17 //// 
#endif GUILD_SKILL

	if( NULL == g_pData ||
		NULL == g_pData->GetMyUser() ||
		NULL == g_pData->GetMyUser()->GetSelectUnit() ||
		NULL == g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() )
	{
		return false;
	}

	CX2Unit::UnitData* pUnitData = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData();

	if(bCheckSP == true)
	{
		if( pUnitData->m_iSPoint + pUnitData->m_iCSPoint < 1 )
			return false;
	}
	
	if( true == IsMaxSkillLevel( eSkillID ) )
		return false;

	int iSkillLevelToLearn = GetSkillLevel( eSkillID ) + 1;

#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
	const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID );
#else // UPGRADE_SKILL_SYSTEM_2013
	const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID, iSkillLevelToLearn );
#endif // UPGRADE_SKILL_SYSTEM_2013
	
	if( NULL == pSkillTemplet )
		return false;


    
	// ������������ ���� ��ų�� ȹ���� �� ����
	if( true == pSkillTemplet->m_bBornSealed )
	{
		if( false == IsSkillUnsealed( eSkillID ) )
		{
			return false;
		}
	}


	const CX2SkillTree::SkillTreeTemplet* pSkillTreeTemplet = g_pData->GetSkillTree()->GetSkillTreeTemplet( (int)m_iUnitClass, eSkillID );
	if( NULL == pSkillTreeTemplet )
		return false;


	const int MAGIC_SKILL_POINT_PER_TIER = 5;
	int iUsedSPoint = 0;
	int iUsedCSPoint = 0;
	CalcUsedSPointAndCSPoint( iUsedSPoint, iUsedCSPoint );
	if( iUsedSPoint + iUsedCSPoint < pSkillTreeTemplet->m_iTier * MAGIC_SKILL_POINT_PER_TIER )
	{
		return false;
	}



#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
	CX2SkillTree::SKILL_ID ePrecedingSkillID = static_cast<CX2SkillTree::SKILL_ID>( pSkillTreeTemplet->m_iFollowingSkill );
	if( false == IsMaxSkillLevel( ePrecedingSkillID ) )
		return false;
#else // UPGRADE_SKILL_SYSTEM_2013
	for( UINT i=0; i<pSkillTreeTemplet->m_vecPrecedingSkill.size(); i++ )
	{
		CX2SkillTree::SKILL_ID ePrecedingSkillID = (CX2SkillTree::SKILL_ID) pSkillTreeTemplet->m_vecPrecedingSkill[i];
		if( false == IsMaxSkillLevel( ePrecedingSkillID ) )
			return false;
	}
#endif // UPGRADE_SKILL_SYSTEM_2013

	return true;
}


bool CX2UserSkillTree::CanIInitSkillTree()
{
	for(SkillDataMap::iterator mit = m_mapSkillAcquired.begin(); mit != m_mapSkillAcquired.end(); ++mit)
	{
		CX2SkillTree::SKILL_ID eSkillID = mit->first;
		const UserSkillData& userSkillData = mit->second;
		if( false == g_pData->GetSkillTree()->IsUnitTypeDefaultSkill( eSkillID ) )
		{
			return true;
		}
		else if( GetSkillLevel( eSkillID ) > 1 )
		{
			return true;
		}
	}

	return false;

}
#ifdef GUILD_SKILL
//{{ oasis907 : ����� //// 2009-11-12 //// 
bool CX2UserSkillTree::CanIInitGuildSkillTree()
{
	for(SkillDataMap::iterator mit = m_mapGuildSkillAcquired.begin(); mit != m_mapGuildSkillAcquired.end(); ++mit)
	{
		CX2SkillTree::SKILL_ID eSkillID = mit->first;
		const UserSkillData& userSkillData = mit->second;
		if( false == g_pData->GetSkillTree()->IsUnitTypeDefaultSkill( eSkillID ) )
		{
			return true;
		}
		else if( GetSkillLevel( eSkillID ) > 1 )
		{
			return true;
		}
	}

	return false;

}
//}} oasis907 : ����� //// 2009-11-12 //// 
#endif GUILD_SKILL


int CX2UserSkillTree::GetSkillEquippedSlotIndex( CX2SkillTree::SKILL_ID eSkillID, bool bSlotB )
{
	if( false == bSlotB )
	{
		for( int i=0; i<ARRAY_SIZE( m_aEquippedSkill ); ++i )
		{
			if( m_aEquippedSkill[i].m_eID == eSkillID )
				return i;
		}

	}
	else
	{
		for( int i=0; i<ARRAY_SIZE( m_aEquippedSkillSlotB ); ++i )
		{
			if( m_aEquippedSkillSlotB[i].m_eID == eSkillID )
				return i;
		}
	}

	return -1;

}

bool CX2UserSkillTree::GetEnabledSkillSlotB() const
{
//{{ Iruha : 2026-08-25 // Skill Slot B open by default, no medal purchase required
#ifdef SERV_IRUHADEV_SKILL_SLOT_B_FREE
	return true;
#endif SERV_IRUHADEV_SKILL_SLOT_B_FREE
//}}

	switch( m_eSkillSlotBExpirationState )
	{
	case SSBES_EXPIRED:
		{
			return false;
		} break;

	case SSBES_NOT_EXPIRED:
	case SSBES_PERMANENT:
		{
			return true;
		} break;
	}

	return false;
}



void CX2UserSkillTree::SetSkillCoolTimeLeft( CX2SkillTree::SKILL_ID eSkillID, float fCoolTimeLeft )
{
	CX2GageManager* pGageManager = CX2GageManager::GetInstance();

	for( int i=0; i<ARRAY_SIZE( m_aEquippedSkill ); ++i )
	{
		SkillSlotData& skillSlot = m_aEquippedSkill[ i ];
				
		if( eSkillID == skillSlot.m_eID )
		{
			skillSlot.m_fCoolTimeLeft = fCoolTimeLeft;
			pGageManager->SetMySkillCoolTimeList( i, fCoolTimeLeft );
		}
	}

	if( true == GetEnabledSkillSlotB() )
	{
		for( int i=0; i<ARRAY_SIZE( m_aEquippedSkillSlotB ); ++i )
		{
			SkillSlotData& skillSlot = m_aEquippedSkillSlotB[ i ];
			
			if( eSkillID == skillSlot.m_eID )
			{
				skillSlot.m_fCoolTimeLeft = fCoolTimeLeft;
				pGageManager->SetMySkillCoolTimeList( i + 4, fCoolTimeLeft );
			}
		}
	}
}

//{{ kimhc // 2010.11.2 // �������� - ���� ������
#ifdef	NEW_SKILL_2010_11

void CX2UserSkillTree::ResetLeftSkillCoolTimeAll()
{
	// ���� a�� ���� b�� ����� ���ٰ� �����Ͽ���
	const int iSizeOfIndex = ARRAY_SIZE( m_aEquippedSkill );
	CX2GageManager* pGageManager = CX2GageManager::GetInstance();

	for ( int iCurrentIndex = 0; iCurrentIndex < iSizeOfIndex; ++iCurrentIndex )
	{
		m_aEquippedSkill[iCurrentIndex].m_fCoolTimeLeft = 0.0f;
		m_aEquippedSkillSlotB[iCurrentIndex].m_fCoolTimeLeft = 0.0f;

		pGageManager->SetMySkillCoolTimeList( iCurrentIndex, 0.0f );
		pGageManager->SetMySkillCoolTimeList( iCurrentIndex + 4, 0.0f );
	}
}

#ifdef UPGRADE_SKILL_SYSTEM_2013 /// ����ȯ

#ifdef ADDED_BBT_ALL_EQUIP_SKILL_SET_OR_RESET_COOLTIME					// ������ / 13-07-04 / ��� ���� ���� ��ų�� ��Ÿ�� �ִ�ġ�� �����ǰ� �ϴ� BBT �߰�
void CX2UserSkillTree::ResetLeftSkillCoolTimeBySkillType( const CX2SkillTree::SKILL_TYPE eSkillType, const float fCoolTime /*= 0.f*/, bool bForceSet /*= true*/, BUFF_CHANGE_TYPE eChangeType /*= BCT_RELATION_VALUE*/, const float fCoolTimeRate /* = 1.f */, const bool bIsRelativeAllSkill /* = false */ )

#else // ADDED_BBT_ALL_EQUIP_SKILL_SET_OR_RESET_COOLTIME
void CX2UserSkillTree::ResetLeftSkillCoolTimeBySkillType( const CX2SkillTree::SKILL_TYPE eSkillType, const float fCoolTime /*= 0.f*/, bool bForceSet /*= true*/, BUFF_CHANGE_TYPE eChangeType /*= BCT_RELATION_VALUE*/ )

#endif // ADDED_BBT_ALL_EQUIP_SKILL_SET_OR_RESET_COOLTIME					// ������ / 13-07-04 / ��� ���� ���� ��ų�� ��Ÿ�� �ִ�ġ�� �����ǰ� �ϴ� BBT �߰�

#else // UPGRADE_SKILL_SYSTEM_2013
void CX2UserSkillTree::ResetLeftSkillCoolTimeBySkillType( const CX2SkillTree::SKILL_TYPE eSkillType )
#endif // UPGRADE_SKILL_SYSTEM_2013
{
	// ���� a�� ���� b�� ����� ���ٰ� �����Ͽ���
	const int iSizeOfIndex = ARRAY_SIZE( m_aEquippedSkill );
	CX2GageManager* pGageManager = CX2GageManager::GetInstance();

	for ( int iCurrentIndex = 0; iCurrentIndex < iSizeOfIndex; ++iCurrentIndex )
	{
		const CX2SkillTree::SkillTemplet* pSkillTempletSlotA = GetUserSkillTemplet( m_aEquippedSkill[iCurrentIndex].m_eID );

		if ( NULL != pSkillTempletSlotA && ( eSkillType == pSkillTempletSlotA->m_eType 
#ifdef ADDED_BBT_ALL_EQUIP_SKILL_SET_OR_RESET_COOLTIME					// ������, ��� ���� ���� ��ų�� ��Ÿ�� �ִ�ġ�� �����ǰ� �ϴ� BBT �߰�
			|| bIsRelativeAllSkill == true								// ���� ������ ��ų�� ��� �����ϴ� ��ų Ÿ��  
#endif // ADDED_BBT_ALL_EQUIP_SKILL_SET_OR_RESET_COOLTIME				// ������, ��� ���� ���� ��ų�� ��Ÿ�� �ִ�ġ�� �����ǰ� �ϴ� BBT �߰�			
			) )
		{

#ifdef UPGRADE_SKILL_SYSTEM_2013 /// ����ȯ
			float fNowCoolTime = m_aEquippedSkill[iCurrentIndex].m_fCoolTimeLeft;		/// �ش� ��ų�� ���� ��Ÿ��

			switch ( eChangeType )
			{
			case BCT_FIX_VALUE:
				{
					/// ���� ��Ÿ�� ���� ���� ������ �߰ų� ���� ��Ÿ���� ������ ��Ÿ�Ӻ��� �ξ� Ŭ��, ���� ��Ÿ���� ������ ��Ÿ������ �����Ѵ�.
					if ( true == bForceSet || fNowCoolTime > fCoolTime )
					{
						m_aEquippedSkill[iCurrentIndex].m_fCoolTimeLeft = fCoolTime;
						pGageManager->SetMySkillCoolTimeList( iCurrentIndex, fCoolTime );
					}
				} break;

			case BCT_RELATION_VALUE:
				{
					fNowCoolTime += fCoolTime;
					fNowCoolTime = max( 0, fNowCoolTime );

					m_aEquippedSkill[iCurrentIndex].m_fCoolTimeLeft = fNowCoolTime;
					pGageManager->SetMySkillCoolTimeList( iCurrentIndex, fNowCoolTime );
				} break;
#ifdef ADDED_BBT_ALL_EQUIP_SKILL_SET_OR_RESET_COOLTIME					// ������, ��� ���� ���� ��ų�� ��Ÿ�� �ִ�ġ�� �����ǰ� �ϴ� BBT �߰�
			case BCT_PERCENT:
				{										
					int iSkillLevel = GetSkillLevel ( pSkillTempletSlotA->m_eID );
					float fMaxCoolTime = pSkillTempletSlotA->GetSkillCoolTimeValue( iSkillLevel );
					fNowCoolTime = fMaxCoolTime * fCoolTimeRate;

					m_aEquippedSkill[iCurrentIndex].m_fCoolTimeLeft = fNowCoolTime;
					pGageManager->SetMySkillCoolTimeList( iCurrentIndex, fNowCoolTime);
				} break;
#endif // ADDED_BBT_ALL_EQUIP_SKILL_SET_OR_RESET_COOLTIME					// ������, ��� ���� ���� ��ų�� ��Ÿ�� �ִ�ġ�� �����ǰ� �ϴ� BBT �߰�

			default:
				break;
			}
			
			
	#else // UPGRADE_SKILL_SYSTEM_2013
			m_aEquippedSkill[iCurrentIndex].m_fCoolTimeLeft = 0.0f;
			pGageManager->SetMySkillCoolTimeList( iCurrentIndex, 0.0f );
	#endif // UPGRADE_SKILL_SYSTEM_2013
			 
		}

#ifdef BALANCE_PATCH_20110303
		const CX2SkillTree::SkillTemplet* pSkillTempletSlotB = GetUserSkillTemplet( m_aEquippedSkillSlotB[iCurrentIndex].m_eID );
#else
		const CX2SkillTree::SkillTemplet* pSkillTempletSlotB = GetUserSkillTemplet( m_aEquippedSkill[iCurrentIndex].m_eID );
#endif

		if ( NULL != pSkillTempletSlotB && ( eSkillType == pSkillTempletSlotB->m_eType 
#ifdef ADDED_BBT_ALL_EQUIP_SKILL_SET_OR_RESET_COOLTIME					// ������, ��� ���� ���� ��ų�� ��Ÿ�� �ִ�ġ�� �����ǰ� �ϴ� BBT �߰�
			|| bIsRelativeAllSkill == true								// ���� ������ ��ų�� ��� �����ϴ� ��ų Ÿ��  
#endif // ADDED_BBT_ALL_EQUIP_SKILL_SET_OR_RESET_COOLTIME					// ������, ��� ���� ���� ��ų�� ��Ÿ�� �ִ�ġ�� �����ǰ� �ϴ� BBT �߰�			

			) )
		{

	#ifdef UPGRADE_SKILL_SYSTEM_2013 /// ����ȯ
			float fNowCoolTime = m_aEquippedSkillSlotB[iCurrentIndex].m_fCoolTimeLeft;	/// �ش� ��ų�� ���� ��Ÿ��

			switch( eChangeType )
			{
			case BCT_FIX_VALUE:
				{
					/// ���� ��Ÿ�� ���� ���� ������ �߰ų� ���� ��Ÿ���� ������ ��Ÿ�Ӻ��� �ξ� Ŭ��, ���� ��Ÿ���� ������ ��Ÿ������ �����Ѵ�.
					if ( true == bForceSet || fNowCoolTime > fCoolTime )
					{
						m_aEquippedSkillSlotB[iCurrentIndex].m_fCoolTimeLeft = fCoolTime;
						pGageManager->SetMySkillCoolTimeList( iCurrentIndex + 4, fCoolTime );
					}
				} break;

			case BCT_RELATION_VALUE:
				{
					fNowCoolTime += fCoolTime;
					fNowCoolTime = max( 0, fNowCoolTime );

					m_aEquippedSkillSlotB[iCurrentIndex].m_fCoolTimeLeft = fNowCoolTime;
					pGageManager->SetMySkillCoolTimeList( iCurrentIndex + 4, fNowCoolTime );
				} break;


#ifdef ADDED_BBT_ALL_EQUIP_SKILL_SET_OR_RESET_COOLTIME					// ������, ��� ���� ���� ��ų�� ��Ÿ�� �ִ�ġ�� �����ǰ� �ϴ� BBT �߰�
			case BCT_PERCENT:
				{										
					int iSkillLevel = GetSkillLevel ( pSkillTempletSlotB->m_eID );
					float fMaxCoolTime = pSkillTempletSlotB->GetSkillCoolTimeValue( iSkillLevel );
					fNowCoolTime = fMaxCoolTime * fCoolTimeRate;

					m_aEquippedSkillSlotB[iCurrentIndex].m_fCoolTimeLeft = fNowCoolTime;
					pGageManager->SetMySkillCoolTimeList( iCurrentIndex, fNowCoolTime);
				} break;
#endif // ADDED_BBT_ALL_EQUIP_SKILL_SET_OR_RESET_COOLTIME					// ������, ��� ���� ���� ��ų�� ��Ÿ�� �ִ�ġ�� �����ǰ� �ϴ� BBT �߰�


			default:
				break;
			}
			
	#else // UPGRADE_SKILL_SYSTEM_2013
			m_aEquippedSkillSlotB[iCurrentIndex].m_fCoolTimeLeft = 0.0f;
			pGageManager->SetMySkillCoolTimeList( iCurrentIndex + 4, 0.0f );
	#endif // UPGRADE_SKILL_SYSTEM_2013

		}
	}
}

#endif	NEW_SKILL_2010_11
//}} kimhc // 2010.11.2 // �������� - ���� ������

#ifdef GUILD_SKILL
//{{ oasis907 : ����� //// 2009-11-16 //// 
bool CX2UserSkillTree::IsGuildSkill( CX2SkillTree::SKILL_ID eSkillID ) const
{
#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
	const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet(eSkillID );
#else // UPGRADE_SKILL_SYSTEM_2013
	const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet(eSkillID, 1);
#endif // UPGRADE_SKILL_SYSTEM_2013
	
	if(pSkillTemplet == NULL)
	{
		return false;
	}
	if(pSkillTemplet->m_bGuildSkill)
	{
		return true;
	}
	return false;

}
//}} oasis907 : ����� //// 2009-11-16 //// 
#endif GUILD_SKILL


#ifdef SERV_SKILL_NOTE
bool CX2UserSkillTree::GetEqipSkillMemo(int iSkillMemo)
{
	for(UINT i=0; i<m_vecSkillNote.size(); ++i)
	{
		if(m_vecSkillNote[i] == iSkillMemo)
		{
			return true;
		}
	}

	return false;
}
#endif

//���� B�� ���� �� �� �������� �������� �ʴ� ���� ����
int CX2UserSkillTree::UpdateEquippedSkillLevelFromAcqureidMap( const CX2SkillTree::SKILL_ID eSkillID, const int iSlotIndex_, bool bSlotB )
{
	if ( !m_mapSkillAcquired.empty() )
	{
		SkillDataMap::const_iterator cmItr = m_mapSkillAcquired.find( eSkillID );
		if ( m_mapSkillAcquired.end() != cmItr )
		{
			if ( iSlotIndex_ >= 0 && iSlotIndex_ < ARRAY_SIZE( m_aEquippedSkill ) )
			{
				if( true == bSlotB )
				{
					m_aEquippedSkillSlotB[iSlotIndex_].m_iSkillLevel = cmItr->second.m_iSkillLevel;
					return m_aEquippedSkillSlotB[iSlotIndex_].m_iSkillLevel;
				}
				else
				{
					m_aEquippedSkill[iSlotIndex_].m_iSkillLevel = cmItr->second.m_iSkillLevel;
					return m_aEquippedSkill[iSlotIndex_].m_iSkillLevel;
				}
			}
		}
	}

	return 0;
}

void CX2UserSkillTree::InitSkillCoolTimeFromGageManager( const vector<float>& vecSkillCoolTime_ )
{
	// ���� a�� ���� b�� ����� ���ٰ� �����Ͽ���
	const int iSizeOfIndex = ARRAY_SIZE( m_aEquippedSkill );
	
	for ( UINT uiSlotIndex = 0; uiSlotIndex < vecSkillCoolTime_.size(); ++uiSlotIndex )
	{
		if ( iSizeOfIndex > uiSlotIndex )
			m_aEquippedSkill[uiSlotIndex].m_fCoolTimeLeft = vecSkillCoolTime_[uiSlotIndex];
		else
			m_aEquippedSkillSlotB[uiSlotIndex - iSizeOfIndex].m_fCoolTimeLeft = vecSkillCoolTime_[uiSlotIndex];
	}	
}

/*static*/ void CX2UserSkillTree::GetSlotIndexAndSlotB( IN const int iSkillSlotId_, OUT int& iSlotIndex_, OUT bool& bSlotB_ )
{
	// SlotB�� ù��° ���� ID int������ ����
	const int iSlotBStartId = static_cast<int>( CX2UserSkillTree::SKILL_SLOT_B1 );

	// iSkillSlotId_�� B�������� �ƴ���..
	bSlotB_ = ( iSkillSlotId_ >= iSlotBStartId ? true : false );

	// B�����̸� B������ ���� ������ ���� ���ڷ� �Ѱ��� ���̵𿡼� ���� (0~3) ������ ���� Index�� ����
	iSlotIndex_ = ( bSlotB_ == true ? iSkillSlotId_ - iSlotBStartId : iSkillSlotId_ );
}




#ifdef NEXON_QA_CHEAT_REQ
void CX2UserSkillTree::AllSkillNoCoolTime()
{
	CX2GageManager* pGageManager = CX2GageManager::GetInstance();

	if( NULL == pGageManager )
		return;

	for( int i=0; i<ARRAY_SIZE( m_aEquippedSkill ); ++i )
	{
		SkillSlotData& skillData = m_aEquippedSkill[i];
		skillData.m_fCoolTimeLeft = 0.f;

		pGageManager->SetMySkillCoolTimeList( i, skillData.m_fCoolTimeLeft );
	}

	for( int i=0; i<ARRAY_SIZE( m_aEquippedSkillSlotB ); i++ )
	{
		SkillSlotData& skillData = m_aEquippedSkillSlotB[i];
		skillData.m_fCoolTimeLeft = 0.f;

		pGageManager->SetMySkillCoolTimeList( i + EQUIPPED_SKILL_SLOT_COUNT, skillData.m_fCoolTimeLeft );
	}	
}
#endif //NEXON_QA_CHEAT_REQ

#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����

/** @function	: SetDefaultSkill
	@brief		: �� ������ �⺻ ��ų ���� �Լ� ( SkillData.lua )
*/

void CX2UserSkillTree::SetDefaultSkill( bool bCanPassIfChecked_ /* = false */ )
{
	/// �� ������ �⺻ ��ų ����
	if ( NULL == g_pData  ||
		 NULL == g_pData->GetSkillTree() ||
		 NULL == g_pData->GetMyUser()->GetSelectUnit() )
	{
		return;
	}

	const int iUnitClass = static_cast<int>( g_pData->GetMyUser()->GetSelectUnit()->GetClass() );	/// �ش� ������ Ŭ����
	const std::map<int, std::vector<int>>					mapDefaultSkill		= g_pData->GetSkillTree()->GetMapDefaultSkill();	/// �⺻ ��ų ����

	const std::map<int, std::vector<int>>::const_iterator	it					= mapDefaultSkill.find( iUnitClass );


	if ( it != mapDefaultSkill.end() )	/// �ش� Ŭ������ �⺻ ��ų ������ �ִٸ�, ����
	{
		std::vector<int> vecDefaultSkillID = it->second;
	
		BOOST_FOREACH( int iValue, vecDefaultSkillID )
		{
			if ( true == bCanPassIfChecked_ &&	
				 0 < g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_UserSkillTree.GetSkillLevel( static_cast<CX2SkillTree::SKILL_ID> ( iValue ) ) )
			{
				continue;
			}
			SetSkillLevelAndCSP( static_cast<CX2SkillTree::SKILL_ID>( iValue ), 1, 0 );
		}
	}
}

/** @function	GetIncreaseSkillLevel
	@brief		��ų ���� ���� ȿ��ġ ���
	@param[in]	��ų ID
	@return		int : ��ų ����
*/
int CX2UserSkillTree::GetIncreaseSkillLevel( CX2SkillTree::SKILL_ID eSkillID ) const
{
	KTDXPROFILE();

#ifdef GUILD_SKILL
	if(IsGuildSkill(eSkillID))
	{
		SkillDataMap::const_iterator mit = m_mapGuildSkillAcquired.find( eSkillID );
		if( mit != m_mapGuildSkillAcquired.end() )
		{
			const UserSkillData& userSkillData = mit->second;
			return userSkillData.GetIncreaseSkillLevel();
		}
		return 0;
	}
#endif GUILD_SKILL

	SkillDataMap::const_iterator mit = m_mapSkillAcquired.find( eSkillID );
	if( mit != m_mapSkillAcquired.end() )
	{
		const UserSkillData& userSkillData = mit->second;
		return userSkillData.GetIncreaseSkillLevel();
	}
	return 0;
}

#endif // UPGRADE_SKILL_SYSTEM_2013

#ifdef SERV_ARA_CHANGE_CLASS_SECOND // ����ȯ
/// ��ų ���� ID�� ���� �Ǿ��ִ� ��ų ���̵� ��ȯ
const CX2SkillTree::SKILL_ID CX2UserSkillTree::GetSkillIDByEquipSlotID( IN const int iStateID_ ) const
{
	int  iSkillSlotIndex	= -1;
	bool bSlotB				= false;

	switch ( CX2GUUser::USER_STATE_ID( iStateID_ ) )
	{
	case CX2GUUser::USI_SPECIAL_ATTACK_1:
	case CX2GUUser::USI_SPECIAL_ATTACK_HYPER_1:
		{
			iSkillSlotIndex = 0;
		} break;
	case CX2GUUser::USI_SPECIAL_ATTACK_2:
	case CX2GUUser::USI_SPECIAL_ATTACK_HYPER_2:
		{
			iSkillSlotIndex = 1;
		} break;
	case CX2GUUser::USI_SPECIAL_ATTACK_3:
	case CX2GUUser::USI_SPECIAL_ATTACK_HYPER_3:
		{
			iSkillSlotIndex = 2;
		} break;
	case CX2GUUser::USI_SPECIAL_ATTACK_4:
	case CX2GUUser::USI_SPECIAL_ATTACK_HYPER_4:
		{
			iSkillSlotIndex = 3;
		} break;

	case CX2GUUser::USI_SPECIAL_ATTACK_SLOTB_1:
	case CX2GUUser::USI_SPECIAL_ATTACK_SLOTB_HYPER_1:
		{
			iSkillSlotIndex = 0;
			bSlotB = true;
		} break;
	case CX2GUUser::USI_SPECIAL_ATTACK_SLOTB_2:
	case CX2GUUser::USI_SPECIAL_ATTACK_SLOTB_HYPER_2:
		{
			iSkillSlotIndex = 1;
			bSlotB = true;
		} break;
	case CX2GUUser::USI_SPECIAL_ATTACK_SLOTB_3:
	case CX2GUUser::USI_SPECIAL_ATTACK_SLOTB_HYPER_3:
		{
			iSkillSlotIndex = 2;
			bSlotB = true;
		} break;
	case CX2GUUser::USI_SPECIAL_ATTACK_SLOTB_4:
	case CX2GUUser::USI_SPECIAL_ATTACK_SLOTB_HYPER_4:
		{
			iSkillSlotIndex = 3;
			bSlotB = true;
		} break;
	}

	const SkillSlotData* pSkillSlotData = GetSkillSlot( iSkillSlotIndex, bSlotB );

	if ( NULL != pSkillSlotData )
		return pSkillSlotData->m_eID;

	return CX2SkillTree::SI_NONE;
}
#endif // SERV_ARA_CHANGE_CLASS_SECOND