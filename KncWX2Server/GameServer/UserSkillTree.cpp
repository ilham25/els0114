#include ".\userskilltree.h"
#include "X2Data/XSLSkillTree.h"

#include "NetError.h"


#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013 // ���볯¥: 2013-06-27

KUserSkillTree::KUserSkillTree(void) :
//{{ Iruha : 2026-08-25 // Skill Slot B open by default, seed the permanent-sentinel end date
#ifdef SERV_IRUHADEV_SKILL_SLOT_B_FREE
m_wstrSkillSlotBEndDate( L"2049-12-31 23:59:00" ),
m_tSkillSlotBEndDate( CTime( 2049, 12, 31, 23, 59, 0 ) ),
#else
m_wstrSkillSlotBEndDate( L"" ),
m_tSkillSlotBEndDate( 0 ),
#endif SERV_IRUHADEV_SKILL_SLOT_B_FREE
//}}
m_iCSPoint( 0 ),
m_iMaxCSPoint( 0 ), 
m_wstrCSPointEndDate( L"" ),
m_tCSPointEndDate( 0 ),
m_iUnitClass( 0 ),
//{{ 2010. 03. 22  ������	����� ��Ʈ
m_cSkillNoteMaxPageNum( 0 )
{
	for( int i = 0; i < MAX_SKILL_SLOT; ++i )
	{
		m_aiSkillSlot[i] = 0;
	}
}

KUserSkillTree::~KUserSkillTree(void)
{
}

void KUserSkillTree::Reset( bool bResetSkillTree, bool bResetEquippedSkill, bool bResetUnsealedSkill, bool bResetCashSkillPoint, bool bResetSkillNote )
{
	if( true == bResetEquippedSkill )
	{
		for( int i = 0; i < MAX_SKILL_SLOT; ++i )
		{
			m_aiSkillSlot[i] = 0;
		}
	}

	if( true == bResetSkillTree )
	{
		m_mapSkillTree.clear();
	}

	if( true == bResetUnsealedSkill )
	{
		m_setUnsealedSkillID.clear();
	}

	if( true == bResetCashSkillPoint )
	{
		m_iCSPoint = 0;
		m_iMaxCSPoint = 0;
		KncUtil::ConvertStringToCTime( std::wstring( L"2000-01-01 00:00:00" ), m_tCSPointEndDate );
	}

	//{{ 2010. 03. 22  ������	����� ��Ʈ
	if( true == bResetSkillNote )
	{
		m_cSkillNoteMaxPageNum = 0;
		m_mapSkillNote.clear();
	}
}

void KUserSkillTree::InitSkill( IN std::vector<KUserSkillData>& vecSkillList, IN int aSkillSlot[], IN std::wstring& wstrSkillSlotBEndDate, IN std::vector<short int>& vecUnsealedSkillList, IN int iUnitClass )
{
	m_iUnitClass = iUnitClass;

	m_mapSkillTree.clear();

	for( UINT i=0; i<vecSkillList.size(); i++ )
	{
		const KUserSkillData& userSkillData = vecSkillList[i];

		SkillDataMap::iterator mit;
		mit = m_mapSkillTree.find( (int)userSkillData.m_iSkillID );
		if( mit != m_mapSkillTree.end() )
		{
			START_LOG( cerr, L"�ߺ��� ��ų�� �����ϰ� ����." )
				<< BUILD_LOG( userSkillData.m_iSkillID )
				<< BUILD_LOG( userSkillData.m_cSkillLevel )
				<< BUILD_LOG( userSkillData.m_cSkillCSPoint )
				<< END_LOG;

			continue;
		}

		int iSkillLevel = (int) userSkillData.m_cSkillLevel;

		// ��ų�� �ְ��������� ���� ��ų�� �ִٸ� �ְ������� ����
		const CXSLSkillTree::SkillTreeTemplet* pSkillTreeTemplet = SiCXSLSkillTree()->GetSkillTreeTemplet( m_iUnitClass, userSkillData.m_iSkillID );
		if( NULL != pSkillTreeTemplet )
		{
			if( (int) userSkillData.m_cSkillLevel > pSkillTreeTemplet->m_iMasterSkillLevel )
			{
				START_LOG( cerr, L"�ְ��������� ���� ������ ��ų�� DB�� ����" )
					<< BUILD_LOG( m_iUnitClass )
					<< BUILD_LOG( userSkillData.m_iSkillID )
					<< BUILD_LOG( userSkillData.m_cSkillLevel )
					<< BUILD_LOG( pSkillTreeTemplet->m_iMasterSkillLevel )
					<< BUILD_LOG( userSkillData.m_cSkillCSPoint )
					<< END_LOG;

				iSkillLevel = pSkillTreeTemplet->m_iMasterSkillLevel;
			}
		}


		// �̹� �����ϸ� ���Ե��� �ʰ� �������� ������ ���Եȴ�.
		m_mapSkillTree[ userSkillData.m_iSkillID ] = UserSkillData( iSkillLevel, (int)userSkillData.m_cSkillCSPoint );
	}

	if( !aSkillSlot )
	{
		START_LOG( cerr, L"��ų ������ NULL" )
			<< END_LOG;

		return;
	}

	// ���� ��ų
	for( int i = 0; i < MAX_SKILL_SLOT; i++ )
	{
		m_aiSkillSlot[i] = aSkillSlot[i];
	}

	//  ���������� ��ų ���
	m_setUnsealedSkillID.clear();
	for( UINT i=0; i<vecUnsealedSkillList.size(); i++ )
	{
		m_setUnsealedSkillID.insert( (int) vecUnsealedSkillList[i] );
	}


	//{{ 2011. 01. 06  ��μ�  ��ų����ü���� üũ(�κ��丮-�Ⱓ��) ��� ����
	SetSkillSolotBEndDate( wstrSkillSlotBEndDate );
}


//{{ 2010. 03. 22  ������	����� ��Ʈ
void KUserSkillTree::InitSkillNote( IN char cSkillNoteMaxPageNum, IN const std::map< char, int >& mapSkillNote )
{
	m_mapSkillNote.clear();

	// ������Ʈ	
	m_cSkillNoteMaxPageNum = cSkillNoteMaxPageNum;
	m_mapSkillNote = mapSkillNote;
}

int KUserSkillTree::GetSkillLevel( IN int iSkillID )
{
	SkillDataMap::iterator mit;
	mit = m_mapSkillTree.find( iSkillID );
	if( mit != m_mapSkillTree.end() )
	{
		const UserSkillData& userSkillData = mit->second;
		return userSkillData.m_iSkillLevel;
	}

	return 0;
}

bool KUserSkillTree::GetSkillLevelAndCSP( IN int iSkillID, OUT int& iSkillLevel, OUT int& iSkillCSPoint )
{
	SkillDataMap::iterator mit;
	mit = m_mapSkillTree.find( iSkillID );
	if( mit != m_mapSkillTree.end() )
	{
		const UserSkillData& userSkillData = mit->second;
		iSkillLevel		= userSkillData.m_iSkillLevel;
		iSkillCSPoint	= userSkillData.m_iSkillCSPoint;
		return true;
	}
	else
	{
		iSkillLevel		= 0;
		iSkillCSPoint	= 0;
	}

	return false;
}


// �����Ͱ� ������ ���� �����ϰ�, ������ �߰��Ѵ�
bool KUserSkillTree::SetSkillLevelAndCSP( int iSkillID, int iSkillLevel, int iSkillCSPoint )
{
	SET_ERROR( NET_OK );

	if( iSkillLevel <= 0 )
	{
		SET_ERROR( ERR_SKILL_20 );

		return false;
	}

	const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( iSkillID );
	if( NULL == pSkillTemplet )
	{
		SET_ERROR( ERR_SKILL_19 );
		return false;
	}

	if( iSkillLevel > SiCXSLSkillTree()->GetMaxSkillLevel( m_iUnitClass, iSkillID ) )
	{
		SET_ERROR( ERR_SKILL_21 );

		return false;
	}

	SkillDataMap::iterator mit;
	mit = m_mapSkillTree.find( iSkillID );
	if( mit != m_mapSkillTree.end() )
	{
		UserSkillData& userSkillData = mit->second;
		userSkillData.m_iSkillLevel		= iSkillLevel;
		userSkillData.m_iSkillCSPoint	= iSkillCSPoint;
	}
	else 
	{
		m_mapSkillTree[ iSkillID ] = UserSkillData( iSkillLevel, iSkillCSPoint );
	}

	return true;
}

bool KUserSkillTree::IsExist( IN int iSkillID )
{
	SkillDataMap::iterator mit;
	mit = m_mapSkillTree.find( iSkillID );
	if( mit != m_mapSkillTree.end() )
	{
		return true;
	}

	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
	if( iSkillID == CXSLSkillTree::SI_ETC_WS_COMMON_LOVE ) // 6001
	{
		return true;
	}

	return false;
}

bool KUserSkillTree::ChangeSkillSlot( int iSlotID, int iSkillID )
{
	SET_ERROR( NET_OK );


	if( iSlotID < 0 || iSlotID >= MAX_SKILL_SLOT )
	{
		SET_ERROR( ERR_SKILL_09 );

		return false;
	}



	if( 0 == iSkillID )
	{
		m_aiSkillSlot[iSlotID] = 0;

		return true;
	}



	if( iSkillID > 0 && IsExist( iSkillID ) == false )
	{
		SET_ERROR( ERR_SKILL_10 );

		return false;
	}




	if( IsSkillSlotB( iSlotID )  &&  iSkillID != 0 )
	{
		// [����] �κ��丮�� ��ų����B Ȯ�� �������� �ִ��� �˻縦 �ұ�? ����?

		CTime tCurrentTime = CTime::GetCurrentTime();
		if( tCurrentTime > m_tSkillSlotBEndDate )
		{
			SET_ERROR( ERR_SKILL_13 );

			return false;
		}
	}

	m_aiSkillSlot[iSlotID] = iSkillID;

	return true;
}

//{{ 2012. 12. 3	�ڼ���	��ų ���� ü���� ��Ŷ ����
bool KUserSkillTree::ChangeSkillSlot( IN const KEGS_CHANGE_SKILL_SLOT_REQ& kPacket_, OUT KEGS_CHANGE_SKILL_SLOT_ACK& kPacket )
{
	SET_ERROR( NET_OK );

	const iSlotID	= kPacket_.m_iSlotID;
	const iSkillID	= kPacket_.m_iSkillID;
	const iSlotID2	= GetSlotID( kPacket_.m_iSkillID );
	const iSkillID2	= GetSkillID( kPacket_.m_iSlotID );

	if( ( iSlotID < 0 ) || ( iSlotID >= MAX_SKILL_SLOT ) )
	{
		SET_ERROR( ERR_SKILL_09 );
		return false;
	}

	if( 0 == iSkillID )
	{
		m_aiSkillSlot[iSlotID] = 0;
	}
	else
	{
		if( iSkillID > 0 && IsExist( iSkillID ) == false )
		{
			SET_ERROR( ERR_SKILL_10 );
			return false;
		}

		if( IsSkillSlotB( iSlotID ) )
		{
			// [����] �κ��丮�� ��ų����B Ȯ�� �������� �ִ��� �˻縦 �ұ�? ����?

			CTime tCurrentTime = CTime::GetCurrentTime();
			if( tCurrentTime > m_tSkillSlotBEndDate )
			{
				SET_ERROR( ERR_SKILL_13 );
				return false;
			}
		}
	}

	if( ( 0 <= iSlotID2 ) && ( iSlotID2 < MAX_SKILL_SLOT ) )
	{
		if( 0 == iSkillID2 )
		{
			m_aiSkillSlot[iSlotID2] = 0;
		}
		else if( ( iSkillID2 > 0 ) && ( IsExist( iSkillID2 ) == false ) )
		{
			SET_ERROR( ERR_SKILL_10 );
			return false;
		}
		else
		{
			if( IsSkillSlotB( iSlotID2 ) )
			{
				// [����] �κ��丮�� ��ų����B Ȯ�� �������� �ִ��� �˻縦 �ұ�? ����?

				CTime tCurrentTime = CTime::GetCurrentTime();
				if( tCurrentTime > m_tSkillSlotBEndDate )
				{
					SET_ERROR( ERR_SKILL_13 );
					return false;
				}
			}
			m_aiSkillSlot[iSlotID2] = iSkillID2;
		}
	}

	m_aiSkillSlot[iSlotID] = iSkillID;

	kPacket.m_iSlotID	= iSlotID;
	kPacket.m_iSkillID	= iSkillID;
	kPacket.m_iSlotID2	= iSlotID2;
	kPacket.m_iSkillID2	= iSkillID2;

	return true;
}

void KUserSkillTree::GetSkillSlot( OUT std::vector<int>& vecSkillID )
{
	vecSkillID.resize(0);

	for( int i = 0; i < MAX_SKILL_SLOT; ++i )
	{
		vecSkillID.push_back( m_aiSkillSlot[i] );
	}
}




void KUserSkillTree::GetSkillSlot( OUT std::vector<KSkillData>& vecSkillSlot )
{
	vecSkillSlot.resize(0);

	for( int i = 0; i < MAX_SKILL_SLOT; ++i )
	{
		KSkillData kSkillData;
		kSkillData.m_iSkillID = m_aiSkillSlot[i];
		kSkillData.m_cSkillLevel = (char) GetSkillLevel( kSkillData.m_iSkillID );
		vecSkillSlot.push_back( kSkillData );
	}
}


void KUserSkillTree::GetSkillSlot( OUT KSkillData aSkillSlot[] )
{
	if( !aSkillSlot )
	{
		START_LOG( cerr, L"��ų ������ NULL" )
			<< END_LOG;

		return;
	}

	for( int i = 0; i < MAX_SKILL_SLOT; ++i )
	{
		aSkillSlot[i].m_iSkillID = m_aiSkillSlot[i];
		aSkillSlot[i].m_cSkillLevel = (char) GetSkillLevel( aSkillSlot[i].m_iSkillID );
	}
}

//{{ 2012. 12. 3	�ڼ���	��ų ���� ü���� ��Ŷ ����
int KUserSkillTree::GetSkillID( int iSlotID )
{
	if( iSlotID < 0 || iSlotID >= MAX_SKILL_SLOT )
	{
		return 0;
	}

	return m_aiSkillSlot[iSlotID];
}

int KUserSkillTree::GetSlotID( int iSkillID )
{
	if( IsExist( iSkillID ) == false )
	{
		return -1;
	}

	for( int i=0; i < MAX_SKILL_SLOT; ++i )
	{
		if( m_aiSkillSlot[i] == iSkillID )
		{
			return i;
		}
	}

	return -1;
}


// �нú� ��ų ID�� ���� ������ �������ش�. ����, PVP���ӿ��� �� ���ֿ��� �ٸ� ������ ��ų ������ �˷��ֱ� ���ؼ�
void KUserSkillTree::GetPassiveSkillData( OUT std::vector<KSkillData>& vecSkillSlot )
{
	vecSkillSlot.resize(0);

	SkillDataMap::iterator it;
	for( it = m_mapSkillTree.begin(); it != m_mapSkillTree.end(); it++ )
	{
		KSkillData kSkillData;
		kSkillData.m_iSkillID		= it->first;
		kSkillData.m_cSkillLevel	= (UCHAR) it->second.m_iSkillLevel;
		vecSkillSlot.push_back( kSkillData );
	}
}

void KUserSkillTree::CalcUsedSPointAndCSPoint( OUT int& iSPoint, OUT int& iCSPoint )
{
	iSPoint = 0;
	iCSPoint = 0;

	for( SkillDataMap::iterator mit = m_mapSkillTree.begin(); mit != m_mapSkillTree.end(); ++mit )
	{
		const int iSkillID = (int)mit->first;
		const UserSkillData& userSkillData = mit->second;

		const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( mit->first );
		if( pSkillTemplet == NULL )
		{
			START_LOG( cerr, L"�������� �ʴ� ��ų ���� �Դϴ�." )
				<< BUILD_LOG( mit->first )
				<< END_LOG;
			continue;
		}

		if( userSkillData.m_iSkillLevel > 0 )
		{
			for( int i = 0 ; i < userSkillData.m_iSkillLevel ; ++i )
			{
				if( i == 0 )
				{
					if( SiCXSLSkillTree()->IsUnitTypeDefaultSkill( iSkillID ) == true )
						continue;

					iSPoint += pSkillTemplet->m_iRequireLearnSkillPoint;
				}
				else
				{
					iSPoint += pSkillTemplet->m_iRequireUpgradeSkillPoint;
				}				
			}

			iSPoint		= iSPoint - userSkillData.m_iSkillCSPoint;
			iCSPoint	+= userSkillData.m_iSkillCSPoint;
		}
		else if( userSkillData.m_iSkillLevel == 0 )
		{
			if( userSkillData.m_iSkillCSPoint > 0 )
			{
				START_LOG( cwarn, L"��ų ������ 0�����ε� SkillCSP�� 0���� ŭ!" )
					<< BUILD_LOG( iSkillID )
					<< BUILD_LOG( userSkillData.m_iSkillLevel )
					<< BUILD_LOG( userSkillData.m_iSkillCSPoint )
					<< END_LOG;
			}
		}
		else
		{
			START_LOG( cwarn, L"��ų ������ 0���� �۴�!" )
				<< BUILD_LOG( iSkillID )
				<< BUILD_LOG( userSkillData.m_iSkillLevel )
				<< BUILD_LOG( userSkillData.m_iSkillCSPoint )
				<< END_LOG;
		}
	}
}

bool KUserSkillTree::IsCashSkillPointExpired()
{
	if( 0 == m_iMaxCSPoint )
		return true;

	return false;
}

// ���� ����� ��ų ����Ʈ�� ��길 ���ش�(ĳ�� ��ų ����Ʈ ����)
// @iRetrievedSPoint: ������ SP ��ġ
void KUserSkillTree::CalcExpireCashSkillPoint( OUT int& iRetrievedSPoint, OUT std::vector<KUserSkillData>& vecModifiedUserSkillData )
{
	iRetrievedSPoint = 0;
	vecModifiedUserSkillData.clear();

	for( SkillDataMap::iterator mit = m_mapSkillTree.begin(); mit != m_mapSkillTree.end(); ++mit )
	{
		const int iSkillID = mit->first;
		const UserSkillData& userSkillData = mit->second;

		const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( iSkillID );
		if( pSkillTemplet == NULL )
			continue;

		bool isDefaultSkill = SiCXSLSkillTree()->IsUnitTypeDefaultSkill( iSkillID );

		// ���ڵ� ĳ�� ����Ʈ�� �������� ó������
		if( userSkillData.m_iSkillCSPoint <= 0 )
			continue;
		
		// csp �� ������ ��ų ���� ����(������ �� ��ų ���� ����)
		// ��ų ������ ����� �Ͽ� �ǵ��� ������ ������
		int iSkillLevel = userSkillData.m_iSkillLevel;
		int iSkillCSP = userSkillData.m_iSkillCSPoint;
		int iRollBackLevelStep = 0;
		int iReturnSP = 0;		// ��ų�� �ش� �������� �ø��� ���� �ʿ��� ��ų
		for(  ; 0 < iSkillLevel ; --iSkillLevel )
		{
			// �������� ����Ʈ
			if( iSkillLevel == 1 )
			{
				if( isDefaultSkill == false )
				{
					iReturnSP += pSkillTemplet->m_iRequireLearnSkillPoint;
					iSkillCSP -= pSkillTemplet->m_iRequireLearnSkillPoint;
				}
			}
			// ���׷��̵� �ϱ� ���� ����Ʈ
			else
			{
				iReturnSP += pSkillTemplet->m_iRequireUpgradeSkillPoint;
				iSkillCSP -= pSkillTemplet->m_iRequireUpgradeSkillPoint;
			}

			++iRollBackLevelStep;

			// csp �� 0 ���϶�� ���̻� ����
			if( iSkillCSP <= 0 )
			{
				break;
			}
		}
	
		// �������� �ʱ�ȭ�Ǵ� ��ų�ΰ�? 
		if( userSkillData.m_iSkillLevel <= iRollBackLevelStep )
		{
			// Learn point �� ������ ���Ѵ�.	// upgrade point �� �����Ͼ� �Ѵ�.
			for( int i = 0 ; i < userSkillData.m_iSkillLevel ; ++i )
			{
				// �������� ����Ʈ
				if( i == 0 )
				{
					if( isDefaultSkill == false )
					{
						iRetrievedSPoint += pSkillTemplet->m_iRequireLearnSkillPoint;
					}
				}
				// ���׷��̵� �ϱ� ���� ����Ʈ
				else
				{
					iRetrievedSPoint += pSkillTemplet->m_iRequireUpgradeSkillPoint;
				}
			}
		}
		else
		{
			// upgrade point �� �����ϸ� �ȴ�
			iRetrievedSPoint += pSkillTemplet->m_iRequireUpgradeSkillPoint * iRollBackLevelStep;
		}

		// ��ų ������ �ǵ����� �۾�
		UCHAR uNewSkillLevel = userSkillData.m_iSkillLevel - iRollBackLevelStep;
		if( uNewSkillLevel == 0 )
		{
			if( isDefaultSkill == true )
			{
				uNewSkillLevel = 1;
			}
		}
		else if( uNewSkillLevel < 0 )
		{
			uNewSkillLevel = 0;
		}
		
		vecModifiedUserSkillData.push_back( KUserSkillData( (short) iSkillID, uNewSkillLevel, 0 ) );
	}

	// ��ųƮ���� �Ҹ��ߴ� CSP �� �����ִ� CSP �� ���ؼ� ���Ž��� CSP�� ���� ������ SP ��ġ�� ���ȴ�
	iRetrievedSPoint += m_iCSPoint - m_iMaxCSPoint;
}

// ������ cash skill point�� ȹ���� ��ų�� �ǵ�����.
void KUserSkillTree::ExpireCashSkillPoint()
{
	for( SkillDataMap::iterator mit = m_mapSkillTree.begin(); mit != m_mapSkillTree.end(); ++mit )
	{
		int iSkillID = mit->first;
		UserSkillData& userSkillData = mit->second;

		const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( iSkillID );
		if( pSkillTemplet == NULL )
			continue;

		bool isDefaultSkill = SiCXSLSkillTree()->IsUnitTypeDefaultSkill( iSkillID );

		// ���ڵ� ĳ�� ����Ʈ�� �������� ó������
		if( userSkillData.m_iSkillCSPoint <= 0 )
			continue;

		// csp �� ������ ��ų ���� ����(������ �� ��ų ���� ����)
		// ��ų ������ ����� �Ͽ� �ǵ��� ������ ������
		int iSkillLevel = userSkillData.m_iSkillLevel;
		int iSkillCSP = userSkillData.m_iSkillCSPoint;
		int iRollBackLevelStep = 0;
		int iReturnSP = 0;		// ��ų�� �ش� �������� �ø��� ���� �ʿ��� ��ų
		for(  ; 0 < iSkillLevel ; --iSkillLevel )
		{
			// �������� ����Ʈ
			if( iSkillLevel == 1 )
			{
				if( isDefaultSkill == false )
				{
					iReturnSP += pSkillTemplet->m_iRequireLearnSkillPoint;
					iSkillCSP -= pSkillTemplet->m_iRequireLearnSkillPoint;
				}
			}
			// ���׷��̵� �ϱ� ���� ����Ʈ
			else
			{
				iReturnSP += pSkillTemplet->m_iRequireUpgradeSkillPoint;
				iSkillCSP -= pSkillTemplet->m_iRequireUpgradeSkillPoint;
			}

			++iRollBackLevelStep;

			// csp �� 0 ���϶�� ���̻� ����
			if( iSkillCSP <= 0 )
			{
				break;
			}
		}

		// ��ų ������ �ǵ����� �۾�
		int iNewSkillLevel = userSkillData.m_iSkillLevel - iRollBackLevelStep;
		if( iNewSkillLevel == 0 )
		{
			if( isDefaultSkill == true )
			{
				iNewSkillLevel = 1;
			}
		}
		else if( iNewSkillLevel < 0 )
		{
			iNewSkillLevel = 0;
		}

		userSkillData.m_iSkillLevel		= iNewSkillLevel;
		userSkillData.m_iSkillCSPoint	= 0;
	}

	// ������ ��ų �߿��� skill level�� 0������ ��ų�� ������ Ż����Ų��.
	for( int i=0; i<MAX_SKILL_SLOT; i++ )
	{
		if( GetSkillLevel( m_aiSkillSlot[i] ) <= 0 )
		{
			ChangeSkillSlot( i, 0 );
		}
	}

	m_iMaxCSPoint = 0;
	m_iCSPoint = 0;
}

bool KUserSkillTree::IsSkillUnsealed( int iSkillID )
{
	std::set< int >::iterator it = m_setUnsealedSkillID.find( iSkillID );
	if( it != m_setUnsealedSkillID.end() )
	{
		return true;
	}

	return false;
}

//{{ 2009. 8. 4  ������		��ų��������
bool KUserSkillTree::SkillUnseal( int iSkillID )
{
	if( IsSkillUnsealed( iSkillID ) )
	{
		START_LOG( cerr, L"�̹� ���������� ��ų�Դϴ�. �Ͼ���� �ȵǴ� ����! �˻������ٵ�.." )
			<< BUILD_LOG( iSkillID )
			<< END_LOG;

		return false;
	}

	m_setUnsealedSkillID.insert( iSkillID );
	return true;
}

bool KUserSkillTree::SetCSPointEndDate( std::wstring wstrEndDate )
{
	if( true == wstrEndDate.empty() )
		return false;

	if( !KncUtil::ConvertStringToCTime( wstrEndDate, m_tCSPointEndDate ) )
		return false;

	m_wstrCSPointEndDate = wstrEndDate;

	return true;
}

void KUserSkillTree::ExpandSkillSlotB( std::wstring& wstrSkillSlotBEndDate )
{	
//{{ Iruha : 2026-08-25 // Skill Slot B open by default - keep the permanent sentinel, don't shorten it and don't clear the equipped B slots
#ifdef SERV_IRUHADEV_SKILL_SLOT_B_FREE
	m_wstrSkillSlotBEndDate = L"2049-12-31 23:59:00";
	KncUtil::ConvertStringToCTime( m_wstrSkillSlotBEndDate, m_tSkillSlotBEndDate );
#else
	m_wstrSkillSlotBEndDate = wstrSkillSlotBEndDate;

	if( !KncUtil::ConvertStringToCTime( wstrSkillSlotBEndDate, m_tSkillSlotBEndDate ) )
	{
		START_LOG( cerr, L"���ڿ� �ð� ��ȯ ����." )
			<< BUILD_LOG( wstrSkillSlotBEndDate )
			<< END_LOG;

		// ���� �ð����� ����
		m_wstrSkillSlotBEndDate = L"2000-01-01 00:00:00";
		KncUtil::ConvertStringToCTime( m_wstrSkillSlotBEndDate, m_tSkillSlotBEndDate );
	}

	for( int i = SKILL_SLOT_B1; i < MAX_SKILL_SLOT; ++i )
	{
		m_aiSkillSlot[i] = 0;
	}
#endif SERV_IRUHADEV_SKILL_SLOT_B_FREE
//}}
}

KUserSkillTree::SKILL_SLOT_B_EXPIRATION_STATE KUserSkillTree::GetSkillSlotBExpirationState()
{
	// note!! ��ų ���� Ȯ�� ������ ���Ⱓ�� 1�� �̻� �������� ���� ���������� �����Ѵ�.
	const CTimeSpan MAGIC_PERMANENT_TIME_SPAN = CTimeSpan( 365, 0, 0, 0 );

	CTime tCurrentTime = CTime::GetCurrentTime();
	if( tCurrentTime >= m_tSkillSlotBEndDate )
	{
		return SSBES_EXPIRED;
	}
	else
	{
		CTimeSpan expirationTimeLeft = m_tSkillSlotBEndDate - tCurrentTime;

		if( expirationTimeLeft > MAGIC_PERMANENT_TIME_SPAN )
		{
			return SSBES_PERMANENT;
		}
		else
		{
			return SSBES_NOT_EXPIRED;
		}
	}
}

void KUserSkillTree::ExpireSkillSlotB()
{
	// ���� �ð��� ���Ͽ� ����ð��� ��
	CTime tCurrentTime = CTime::GetCurrentTime();
	if( tCurrentTime >= m_tSkillSlotBEndDate )
	{
		// ���� ȣ��Ǵ� �κ��̱� ������ �ڵ� ����ȭ (for�� ����)
		m_aiSkillSlot[SKILL_SLOT_B1] = 0;
		m_aiSkillSlot[SKILL_SLOT_B2] = 0;
		m_aiSkillSlot[SKILL_SLOT_B3] = 0;
		m_aiSkillSlot[SKILL_SLOT_B4] = 0;
	}
}

bool KUserSkillTree::IsMyUnitClassSkill( int iSkillID )
{
	if( SiCXSLSkillTree()->GetMasterSkillLevel( m_iUnitClass, iSkillID ) > 0 )
		return true;

	return false;
}

bool KUserSkillTree::IsAllPrecedingSkillLearned( int iSkillID, std::map< int, KGetSkillInfo >& mapSkillList )
{
	const CXSLSkillTree::SkillTreeTemplet* pSkillTreeTemplet = SiCXSLSkillTree()->GetSkillTreeTemplet( m_iUnitClass, iSkillID );
	if( NULL == pSkillTreeTemplet )
	{
		START_LOG( cerr, L"��ųƮ�� ���ø��� ����_preceding" )
			<< BUILD_LOG( m_iUnitClass )
			<< BUILD_LOG( iSkillID )
			<< END_LOG;

		return false;
	}

	if( pSkillTreeTemplet->m_iPrecedingSkill > 0 )
	{
		if( IsSkillLearned( pSkillTreeTemplet->m_iPrecedingSkill ) == false )
		{
			std::map< int, KGetSkillInfo >::iterator mit = mapSkillList.find( pSkillTreeTemplet->m_iPrecedingSkill );
			if( mit != mapSkillList.end() )
			{
				return true;
			}
		}
	}
	
	return true;
}



bool KUserSkillTree::IsAllFollowingSkillLevelZero( int iSkillID )
{
	const CXSLSkillTree::SkillTreeTemplet* pSkillTreeTemplet = SiCXSLSkillTree()->GetSkillTreeTemplet( m_iUnitClass, iSkillID );
	if( NULL == pSkillTreeTemplet )
	{
		START_LOG( cerr, L"��ųƮ�� ���ø��� ����_following" )
			<< BUILD_LOG( m_iUnitClass )
			<< BUILD_LOG( iSkillID )
			<< END_LOG;

		return false;
	}

	BOOST_TEST_FOREACH( const int, iFollowingSkillID, pSkillTreeTemplet->m_vecFollowingSkill )
	{
		if( GetSkillLevel( iFollowingSkillID ) > 0 )
		{
			return false;
		}
	}

	return true;
}

//{{ 2010. 03. 22  ������	����� ��Ʈ
void KUserSkillTree::GetSkillNote( OUT std::vector< int >& vecSkillNote )
{
	vecSkillNote.clear();

	std::map< char, int >::const_iterator mit;
	for( mit = m_mapSkillNote.begin(); mit != m_mapSkillNote.end(); ++mit )
	{
		vecSkillNote.push_back( mit->second );
	}
}

bool KUserSkillTree::GetExpandSkillNotePage( IN u_char ucLevel, OUT char& cPageNum )
{
	cPageNum = 0;

	const u_char ucCheckNum = ucLevel / 10; // 10�� �ڸ��� ���ϱ� ���� 10���� ����
	switch( ucCheckNum )
	{
	case 0:
	case 1:
		return false;

	case 2:
		cPageNum = 1;
		return true;

	case 3:
		cPageNum = 2;
		return true;

	case 4:
		cPageNum = 3;
		return true;

	case 5:
		cPageNum = 4;
		return true;

		//{{ 2011. 07. 13	������	���� Ȯ��
	case 6:
		cPageNum = 5;
		return true;
	}

	return false;
}

bool KUserSkillTree::IsExistSkillNotePage( IN char cPageNum )
{
	if( m_cSkillNoteMaxPageNum > cPageNum )
		return true;

	return false;
}

bool KUserSkillTree::IsExistSkillNoteMemoID( IN int iSkillNoteMemoID )
{
	std::map< char, int >::const_iterator mit;
	for( mit = m_mapSkillNote.begin(); mit != m_mapSkillNote.end(); ++mit )
	{
		if( mit->second == iSkillNoteMemoID )
			return true;
	}

	return false;
}

void KUserSkillTree::UpdateSkillNoteMemo( IN char cPageNum, IN int iMemoID )
{
	std::map< char, int >::iterator mit;
	mit = m_mapSkillNote.find( cPageNum );
	if( mit == m_mapSkillNote.end() )
	{
		m_mapSkillNote.insert( std::make_pair( cPageNum, iMemoID ) );
	}
	else
	{
		mit->second = iMemoID;
	}
}

//{{ 2011. 01. 06  ��μ�  ��ų����ü���� üũ(�κ��丮-�Ⱓ��) ��� ����
void KUserSkillTree::SetSkillSolotBEndDate( IN std::wstring& wstrSkillSlotBEndDate )
{
//{{ Iruha : 2026-08-25 // Skill Slot B open by default - ignore the incoming end date, keep it permanent
#ifdef SERV_IRUHADEV_SKILL_SLOT_B_FREE
	m_wstrSkillSlotBEndDate = L"2049-12-31 23:59:00";
	KncUtil::ConvertStringToCTime( m_wstrSkillSlotBEndDate, m_tSkillSlotBEndDate );
#else
	// ��ų ���� B
	m_wstrSkillSlotBEndDate = wstrSkillSlotBEndDate;

	if( true == wstrSkillSlotBEndDate.empty() )
	{
		// ���� �ð����� ����
		m_wstrSkillSlotBEndDate = L"2000-01-01 00:00:00";
		KncUtil::ConvertStringToCTime( m_wstrSkillSlotBEndDate, m_tSkillSlotBEndDate );
	}
	else if( !KncUtil::ConvertStringToCTime( wstrSkillSlotBEndDate, m_tSkillSlotBEndDate ) )
	{
		START_LOG( cerr, L"���ڿ� �ð� ��ȯ ����." )
			<< BUILD_LOG( wstrSkillSlotBEndDate )
			<< END_LOG;

		// ���� �ð����� ����
		m_wstrSkillSlotBEndDate = L"2000-01-01 00:00:00";
		KncUtil::ConvertStringToCTime( m_wstrSkillSlotBEndDate, m_tSkillSlotBEndDate );
	}
#endif SERV_IRUHADEV_SKILL_SLOT_B_FREE
//}}
}

//{{ 2011. 11. 21  ��μ�	���� ���� ������
void KUserSkillTree::GetUnSealedSkillList( OUT std::vector< short >& vecUnsealedSkillID )
{
	std::set< int >::iterator sit = m_setUnsealedSkillID.begin();
	for( ; sit != m_setUnsealedSkillID.end() ; ++sit )
	{
		vecUnsealedSkillID.push_back( static_cast<short>(*sit) );
	}
}

void KUserSkillTree::SetClassChangeSkill( IN std::map< int, int >& mapSkill )
{
	std::set< int >::iterator sit;

	std::map< int, int >::iterator mit = mapSkill.begin();
	for(  ; mit != mapSkill.end() ; ++mit )
	{
		int itempfirst = mit->first;
		sit = m_setUnsealedSkillID.find( mit->first );
		if( sit != m_setUnsealedSkillID.end() )
		{
			m_setUnsealedSkillID.erase( sit );
			m_setUnsealedSkillID.insert( mit->second );
		}
		else
		{
			START_LOG( cerr, L"�̷� ��ų�� ������ ���� �ʴٰ�?! �ִٰ� �ؼ� ����Ѱ��ݾ�!!" )
				<< BUILD_LOG( mit->first )
				<< BUILD_LOG( mit->second )
				<< END_LOG;
		}
	}
}

void KUserSkillTree::SetClassChangeMemo( IN std::map< int, int >& mapMemo )
{
	std::map< char, int >::iterator mymit;

	std::map< int, int >::iterator mit = mapMemo.begin();
	for(  ; mit != mapMemo.end() ; ++mit )
	{
		mymit = m_mapSkillNote.begin();
		for( ; mymit != m_mapSkillNote.end() ; ++mymit )
		{
			if( mymit->second == mit->first )
			{
				mymit->second = mit->second;
			}
		}
	}
}

void KUserSkillTree::CheckAddSkillStat_BaseHP( IN const KStat& kStat, IN OUT KStat& kModifiedBaseStatBySkill )
{
	const int iSkillLevel = GetSkillLevel( (int) CXSLSkillTree::SI_P_ES_POWERFUL_VITAL );
	if( 0 < iSkillLevel )
	{
		const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( (int)CXSLSkillTree::SI_P_ES_POWERFUL_VITAL );
		if( NULL != pSkillTemplet )
		{
			float fRate = pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_MAX_HP_REL, iSkillLevel );
			kModifiedBaseStatBySkill.m_iBaseHP += static_cast<int>( kStat.m_iBaseHP * CXSLSkillTree::CalulateIncreaseingRate( fRate ) );
		}
	}

	//{{ kimhc // 2011.1.14 // û 1�� ����, ǻ��������� ��� ����
	const int iSkillLevelGuardMastery = GetSkillLevel( static_cast<int>( CXSLSkillTree::SI_P_CFG_GUARD_MASTERY ) );
	if ( 0 < iSkillLevelGuardMastery )
	{
		const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( static_cast<int>( CXSLSkillTree::SI_P_CFG_GUARD_MASTERY ) );
		if ( NULL != pSkillTemplet )
		{
			float fRate = pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_MAX_HP_REL, iSkillLevelGuardMastery );
			kModifiedBaseStatBySkill.m_iBaseHP += static_cast<int>( kStat.m_iBaseHP * CXSLSkillTree::CalulateIncreaseingRate( fRate ) );
		}
	}

	// ȣ�Ű���
	{
		const int iSkillLevel = GetSkillLevel( static_cast<int>( CXSLSkillTree::SI_P_ASD_SELF_PROTECTION_FORTITUDE ) );
		if ( 0 < iSkillLevel )
		{
			const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( static_cast<int>( CXSLSkillTree::SI_P_ASD_SELF_PROTECTION_FORTITUDE ) );
			if ( NULL != pSkillTemplet )
			{
				float fRate = pSkillTemplet->GetSkillAbilityValue( CXSLSkillTree::SA_MAX_HP_REL, iSkillLevel );
				kModifiedBaseStatBySkill.m_iBaseHP += static_cast<int>( kStat.m_iBaseHP * CXSLSkillTree::CalulateIncreaseingRate( fRate ) );
			}
		}
	}
}

void KUserSkillTree::GetSkillStat( KStat& kStat )
{
	kStat.Init();


	SkillDataMap::iterator mit;
	for( mit = m_mapSkillTree.begin(); mit != m_mapSkillTree.end(); ++mit )
	{
		const UserSkillData& userSkillData = mit->second;
		if( userSkillData.m_iSkillLevel <= 0 )
			continue;

		const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( mit->first );

		if( pSkillTemplet == NULL )
		{
			START_LOG( cerr, L"��ų ���ø� ����Ʈ �̻�.!" )
				<< BUILD_LOG( mit->first )
				<< BUILD_LOG( userSkillData.m_iSkillLevel )
				<< END_LOG;

			continue;
		}

		if( userSkillData.m_iSkillLevel <= 0 )
			continue;

		if( userSkillData.m_iSkillLevel > CXSLSkillTree::SkillTemplet::SLV_MAX_LEVEL )
			continue;

		switch( pSkillTemplet->m_eType )
		{
		case CXSLSkillTree::ST_PASSIVE_PHYSIC_ATTACK: 
		case CXSLSkillTree::ST_PASSIVE_MAGIC_ATTACK:  
		case CXSLSkillTree::ST_PASSIVE_MAGIC_DEFENCE: 
		case CXSLSkillTree::ST_PASSIVE_PHYSIC_DEFENCE:
			{
				if( pSkillTemplet->m_vecStat.size() <= 0)
				{
					START_LOG( cerr, L"[�׽�Ʈ] �̷��� ������ �ȵȴ�!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" )
						<< END_LOG;
					continue;
				}

				CXSLStat::Stat kSkillStat = pSkillTemplet->m_vecStat[userSkillData.m_iSkillLevel - 1];

				kStat.m_iBaseHP			+= (int)kSkillStat.m_fBaseHP;
				kStat.m_iAtkPhysic		+= (int)kSkillStat.m_fAtkPhysic;
				kStat.m_iAtkMagic		+= (int)kSkillStat.m_fAtkMagic;
				kStat.m_iDefPhysic		+= (int)kSkillStat.m_fDefPhysic;
				kStat.m_iDefMagic		+= (int)kSkillStat.m_fDefMagic;
			} break;
		}
	}
}

bool KUserSkillTree::IsMasterSkillLevel( IN int iSkillID )
{
	const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( iSkillID );
	if( NULL == pSkillTemplet )
	{
		START_LOG( cerr, L"��ų�� �ְ� �������� Ȯ���Ϸ��µ� ��ų ���ø��� NULL" )
			<< BUILD_LOG( iSkillID )
			<< END_LOG;

		return false;
	}

	SkillDataMap::iterator mit;
	mit = m_mapSkillTree.find( iSkillID );
	if( mit != m_mapSkillTree.end() )
	{
		const UserSkillData& userSkillData = mit->second;
		int iMasterSkillLevel = SiCXSLSkillTree()->GetMasterSkillLevel( m_iUnitClass, iSkillID );
		if( userSkillData.m_iSkillLevel > iMasterSkillLevel )
		{
			START_LOG( cerr, L"��ų�� �ְ� �������� Ȯ���Ϸ��µ� ��ų ������ �ְ��������� ����" )
				<< BUILD_LOG( m_iUnitClass )
				<< BUILD_LOG( iSkillID )
				<< BUILD_LOG( userSkillData.m_iSkillLevel )
				<< BUILD_LOG( iMasterSkillLevel )
				<< END_LOG;

			return true;
		}
		else if( userSkillData.m_iSkillLevel == iMasterSkillLevel )
		{
			return true;
		}
		else 
		{
			return false;
		}
	}

	return false;
}

bool KUserSkillTree::IsSkillLearned( IN int iSkillID )
{
	const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( iSkillID );
	if( NULL == pSkillTemplet )
	{
		START_LOG( cerr, L"��ų�� �ְ� �������� Ȯ���Ϸ��µ� ��ų ���ø��� NULL" )
			<< BUILD_LOG( iSkillID )
			<< END_LOG;

		return false;
	}

	SkillDataMap::iterator mit;
	mit = m_mapSkillTree.find( iSkillID );
	if( mit != m_mapSkillTree.end() )
	{
		const UserSkillData& userSkillData = mit->second;
		if( userSkillData.m_iSkillLevel > 0 )
		{
			return true;
		}
	}

	return false;
}

bool KUserSkillTree::GetNecessarySkillPoint( IN OUT std::map< int, KGetSkillInfo >& mapSkillList, IN int& iTotalSP, IN int& iTotalCSP )
{
	// ��ü �ʿ� sp �� ���ϱ�
	iTotalSP = 0;
	// ��ü �ʿ� csp �� ���ϱ�
	iTotalCSP = 0;

	std::map< int, KGetSkillInfo>::iterator mit = mapSkillList.begin();
	for( ; mit != mapSkillList.end() ; ++mit )
	{
		const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( mit->first );
		if( NULL == pSkillTemplet )
		{
			START_LOG( cerr, L"��ų ���ø��� NULL" )
				<< BUILD_LOG( mit->first )
				<< END_LOG;

			return false;
		}

		SkillDataMap::iterator mitMySkill;
		mitMySkill = m_mapSkillTree.find( mit->first );
		if( mitMySkill != m_mapSkillTree.end() )
		{
			const UserSkillData& userSkillData = mitMySkill->second;
			if( userSkillData.m_iSkillLevel > 0 )
			{
				// �̹� �����ߴ�, ��ȭ�� �ʿ��� ����Ʈ�� ���
				int iUpgradeLevel = mit->second.m_iSkillLevel - userSkillData.m_iSkillLevel;
				if( iUpgradeLevel < 0 )
				{
					START_LOG( cerr, L"��ų ������ �̻��ϴ�" )
						<< BUILD_LOG( mit->first )
						<< BUILD_LOG( iUpgradeLevel )
						<< END_LOG;

					return false;
				}

				// ĳ�� ��ų�� ��� ���� �ƴ϶��
				if( IsCashSkillPointExpired() == true )
				{
					iTotalSP += pSkillTemplet->m_iRequireUpgradeSkillPoint * iUpgradeLevel;
				}
				else
				{
					mit->second.m_iSpendSkillCSPoint = pSkillTemplet->m_iRequireUpgradeSkillPoint * iUpgradeLevel;
					iTotalSP += mit->second.m_iSpendSkillCSPoint;
				}
			}
			else
			{
				// �̽��� ��ų, ����� ��ȭ�� �ʿ��� ����Ʈ ���
				int iUpgradeLevel = mit->second.m_iSkillLevel - 1; // ��ų ���� ���� ����
				if( iUpgradeLevel < 0 )
				{
					START_LOG( cerr, L"��ų ������ �̻��ϴ�" )
						<< BUILD_LOG( mit->first )
						<< BUILD_LOG( iUpgradeLevel )
						<< END_LOG;

					return false;
				}
				
				// ĳ�� ��ų�� ��� ���� �ƴ϶��
				if( IsCashSkillPointExpired() == true )
				{
					iTotalSP += pSkillTemplet->m_iRequireLearnSkillPoint + pSkillTemplet->m_iRequireUpgradeSkillPoint * iUpgradeLevel;
				}
				else
				{
					mit->second.m_iSpendSkillCSPoint = pSkillTemplet->m_iRequireLearnSkillPoint+ pSkillTemplet->m_iRequireUpgradeSkillPoint * iUpgradeLevel;
					iTotalSP += mit->second.m_iSpendSkillCSPoint;
				}
			}
		}
		else
		{
			// �̽��� ��ų, ����� ��ȭ�� �ʿ��� ����Ʈ ���
			int iUpgradeLevel = mit->second.m_iSkillLevel - 1; // ��ų ���� ���� ����
			if( iUpgradeLevel < 0 )
			{
				START_LOG( cerr, L"��ų ������ �̻��ϴ�" )
					<< BUILD_LOG( mit->first )
					<< BUILD_LOG( iUpgradeLevel )
					<< END_LOG;

				return false;
			}

			// ĳ�� ��ų�� ��� ���� �ƴ϶��
			if( IsCashSkillPointExpired() == true )
			{
				iTotalSP += pSkillTemplet->m_iRequireLearnSkillPoint + pSkillTemplet->m_iRequireUpgradeSkillPoint * iUpgradeLevel;
			}
			else
			{
				mit->second.m_iSpendSkillCSPoint = pSkillTemplet->m_iRequireLearnSkillPoint+ pSkillTemplet->m_iRequireUpgradeSkillPoint * iUpgradeLevel;
				iTotalSP += mit->second.m_iSpendSkillCSPoint;
			}
		}
	}

	// �Ҹ�Ǵ� sp �� �ִ�
	if( iTotalSP > 0 )
	{
		// ĳ�� ��ų�� ��� ���̶��
		if( IsCashSkillPointExpired() == false )
		{
			if( GetCSPoint() > 0 )
			{
				iTotalSP -= GetCSPoint();
				iTotalCSP = GetCSPoint();
			}
		}
	}

	return true;
}

void KUserSkillTree::GetTierSkillList( IN int iTier, OUT std::vector< int >& vecTierSkillList, OUT bool& bDefaultSkillTire )
{
	vecTierSkillList.clear();

	SkillDataMap::iterator mitMySkill;
	mitMySkill = m_mapSkillTree.begin();
	for( ; mitMySkill != m_mapSkillTree.end() ; ++mitMySkill )
	{
		const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( mitMySkill->first );
		if( NULL == pSkillTemplet )
		{
			START_LOG( cerr, L"��ų ���ø��� NULL" )
				<< BUILD_LOG( mitMySkill->first )
				<< END_LOG;

			continue;
		}

		const CXSLSkillTree::SkillTreeTemplet* pSkillTreeTemplet = SiCXSLSkillTree()->GetSkillTreeTemplet( m_iUnitClass, mitMySkill->first );
		if( NULL == pSkillTreeTemplet )
		{
			START_LOG( cerr, L"��ų Ʈ�� ���ø��� NULL" )
				<< BUILD_LOG( mitMySkill->first )
				<< END_LOG;

			continue;
		}

		if( pSkillTreeTemplet->m_iTier == iTier )
		{
			if( SiCXSLSkillTree()->IsUnitTypeDefaultSkill( mitMySkill->first ) == true )
			{
				bDefaultSkillTire = true;
			}

			if( mitMySkill->second.m_iSkillLevel > 0 )
			{
				vecTierSkillList.push_back( mitMySkill->first );
			}			
		}
	}
}

void KUserSkillTree::GetHaveSkillList( OUT std::map< int, int >& mapHaveSkill )
{
	SkillDataMap::iterator mitMySkill;
	mitMySkill = m_mapSkillTree.begin();
	for( ; mitMySkill != m_mapSkillTree.end() ; ++mitMySkill )
	{
		mapHaveSkill.insert( std::make_pair( mitMySkill->first, mitMySkill->second.m_iSkillLevel ) );
	}
}

void KUserSkillTree::ResetSkill( IN int iSkillID, IN bool bDefaultSkill )
{
	SkillDataMap::iterator mitMySkill;
	mitMySkill = m_mapSkillTree.find( iSkillID );
	if( mitMySkill != m_mapSkillTree.end() )
	{
		if( bDefaultSkill == true )
		{
			mitMySkill->second.m_iSkillLevel = 1;
			mitMySkill->second.m_iSkillCSPoint = 0;
		}
		else
		{
			m_mapSkillTree.erase( mitMySkill );
		}
	}
}

bool KUserSkillTree::CheckGetNewSkill( IN std::map< int, KGetSkillInfo >& mapGetSkillList, IN int iUnitClass, IN int iLevel, OUT KEGS_GET_SKILL_ACK& kPacket )
{
	// ������ ��ų �˻� ����
	std::map< int, KGetSkillInfo >::iterator mit = mapGetSkillList.begin();
	for( ; mit != mapGetSkillList.end() ; ++mit )
	{
		// 1. ���� ��ų �ΰ�?
		const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( mit->first );
		if( pSkillTemplet == NULL )
		{
			kPacket.m_iOK = NetError::ERR_SKILL_01;
			return false;
		}

		const CXSLSkillTree::SkillTreeTemplet* pSkillTreeTemplet = SiCXSLSkillTree()->GetSkillTreeTemplet( iUnitClass, mit->first );
		if( pSkillTreeTemplet == NULL )
		{
			kPacket.m_iOK = NetError::ERR_SKILL_01;
			return false;
		}

		// 2. ��ų ���� �˻�
		if( IsMasterSkillLevel( mit->first ) == true )
		{
			kPacket.m_iOK = NetError::ERR_SKILL_00;
			return false;
		}

		// ��� ��ų�� ���� �˻�
		int iMasterSkillLevel = SiCXSLSkillTree()->GetMasterSkillLevel( m_iUnitClass, mit->first );
		if( mit->second.m_iSkillLevel > iMasterSkillLevel )
		{
			kPacket.m_iOK = NetError::ERR_SKILL_00;
			return false;
		}

		// ��� ��ų�� �ʿ� ���� �˻�
		if( (int)pSkillTemplet->m_vecRequireCharactorLevel.size() >= mit->second.m_iSkillLevel )
		{
			if( pSkillTemplet->m_vecRequireCharactorLevel[mit->second.m_iSkillLevel-1] > iLevel )
			{
				kPacket.m_iOK = NetError::ERR_SKILL_03;
				return false;
			}
		}

		// 3. ���� ��ų �˻�
		if( pSkillTemplet->m_bBornSealed == true )
		{
			if( IsSkillUnsealed( mit->first ) == false )
			{
				kPacket.m_iOK = NetError::ERR_SKILL_15;
				return false;
			}
		}

		// 4. ���� ���� Unit Class �˻�
		if( IsMyUnitClassSkill( mit->first ) == false )
		{
			START_LOG( cerr, L"�ڽ��� Ŭ������ �ٸ� ��ų�� ȹ���Ϸ���.!" )
				<< BUILD_LOG( iUnitClass )
				<< BUILD_LOG( mit->first )
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_SKILL_16;
			return false;
		}

		// 5. ���ེų�� ���� �˻�
		if( IsAllPrecedingSkillLearned( mit->first, mapGetSkillList ) == false )
		{
			kPacket.m_iOK = NetError::ERR_SKILL_07;
			return false;
		}

		// 6. 2������ ��ų�� ��� ���� tier ��ų ���� ���� �˻�
#ifndef SERV_IRUHADEV_SKILLTREE_NO_LOCK
		if( pSkillTreeTemplet->m_iColumn == 0 || pSkillTreeTemplet->m_iColumn == 1 )
		{
			// ��� ��ų �߿� ���� 2������ ��ų�� �ִ��� Ȯ��
			std::vector< int > vecTierSkillList;
			vecTierSkillList.clear();
			bool bDefaultSkillTire = false;
			GetTierSkillList( pSkillTreeTemplet->m_iTier, vecTierSkillList, bDefaultSkillTire ); 

			// �⺻ ��ų�� �ƴϾ�� �Ѵ�.
			if( bDefaultSkillTire == false )
			{
				BOOST_TEST_FOREACH( int, iTierSkillID, vecTierSkillList )
				{
					// ���� ��ų�̶�� �Ѿ
					if( iTierSkillID == mit->first )
						continue;

					const CXSLSkillTree::SkillTreeTemplet* pTierSkillTreeTemplet = SiCXSLSkillTree()->GetSkillTreeTemplet( iUnitClass, iTierSkillID );
					if( NULL == pTierSkillTreeTemplet )
					{
						kPacket.m_iOK = NetError::ERR_SKILL_01;
						return false;
					}

					// ���� Ƽ���� ��ų�ε� 2������ ��ų�� ��� ���� �ִ�.
					if( pTierSkillTreeTemplet->m_iColumn == 0 || pTierSkillTreeTemplet->m_iColumn == 1 )
					{
						kPacket.m_iOK = NetError::ERR_SKILL_29;
						return false;
					}
				}
			}
			
			// ��� ��ų �߿� ���� 2������ ��ų�� �ִ��� Ȯ��
			std::map< int, KGetSkillInfo >::iterator mitOther = mapGetSkillList.begin();
			for( ; mitOther != mapGetSkillList.end() ; ++mitOther )
			{
				// ���� ��ų�̶�� �н�
				if( mitOther->first == mit->first )
					continue;

				const CXSLSkillTree::SkillTreeTemplet* pOtherSkillTreeTemplet = SiCXSLSkillTree()->GetSkillTreeTemplet( iUnitClass, mitOther->first );
				if( pOtherSkillTreeTemplet == NULL )
				{
					kPacket.m_iOK = NetError::ERR_SKILL_01;
					return false;
				}

				// ���� tier
				if( pOtherSkillTreeTemplet->m_iTier == pSkillTreeTemplet->m_iTier )
				{
					// 2������ ��ų�̴�.
					if( pOtherSkillTreeTemplet->m_iColumn == 0 || pOtherSkillTreeTemplet->m_iColumn == 1 )
					{
						kPacket.m_iOK = NetError::ERR_SKILL_29;
						return false;
					}
				}
			}
		}
#endif SERV_IRUHADEV_SKILLTREE_NO_LOCK
	}

	return true;
}

bool KUserSkillTree::CheckResetSkill( IN KEGS_RESET_SKILL_REQ& kPacket_, IN int iUnitClass, IN int iLevel, OUT int& iOK, OUT bool& bSKillInitLevel )
{
	// 0������ ���� ���ΰ�?
	bSKillInitLevel = true;

	// ���� ��� SkillTemplet
	const CXSLSkillTree::SkillTemplet* pDelSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( kPacket_.m_iSkillID );
	if( pDelSkillTemplet == NULL )
	{
		iOK = NetError::ERR_SKILL_01;
		return false;
	}

	// ���� ��� SkillTreeTemplet
	const CXSLSkillTree::SkillTreeTemplet* pDelSkillTreeTemplet = SiCXSLSkillTree()->GetSkillTreeTemplet( iUnitClass, kPacket_.m_iSkillID );
	if( pDelSkillTreeTemplet == NULL )
	{
		iOK = NetError::ERR_SKILL_01;
		return false;
	}

	// ��ų ������ �������� �˻�
	if( GetSkillLevel( kPacket_.m_iSkillID ) < 1 )
	{
		iOK = NetError::ERR_SKILL_20;
		return false;
	}

	// �⺻ ��ų�̸� 0�� ��Ű�� ���Ѵ�
	bool isDefaultSkill = SiCXSLSkillTree()->IsUnitTypeDefaultSkill( kPacket_.m_iSkillID );

	// ���� ��ų�̸� 0�� ��Ű�� ���Ѵ�
	bool bAllFollowingSkillLevelZero = IsAllFollowingSkillLevelZero( kPacket_.m_iSkillID );


	if( isDefaultSkill == true || bAllFollowingSkillLevelZero == false )
	{
		bSKillInitLevel = false;
	}

	return true;
}

#else	// SERV_UPGRADE_SKILL_SYSTEM_2013
/*
KUserSkillTree::KUserSkillTree(void) :
m_wstrSkillSlotBEndDate( L"" ),
m_tSkillSlotBEndDate( 0 ),
m_iCSPoint( 0 ),
m_iMaxCSPoint( 0 ), 
m_wstrCSPointEndDate( L"" ),
m_tCSPointEndDate( 0 ),
m_iUnitClass( 0 ),
//{{ 2010. 03. 22  ������	����� ��Ʈ
#ifdef SERV_SKILL_NOTE	
m_cSkillNoteMaxPageNum( 0 )
#endif SERV_SKILL_NOTE
//}}
{
	for( int i = 0; i < MAX_SKILL_SLOT; ++i )
	{
		m_aiSkillSlot[i] = 0;
	}
}

KUserSkillTree::~KUserSkillTree(void)
{
}

void KUserSkillTree::Reset( bool bResetSkillTree, bool bResetEquippedSkill, bool bResetUnsealedSkill, bool bResetCashSkillPoint, bool bResetSkillNote )
{
	if( true == bResetEquippedSkill )
	{
		for( int i = 0; i < MAX_SKILL_SLOT; ++i )
		{
			m_aiSkillSlot[i] = 0;
		}
	}

	if( true == bResetSkillTree )
	{
		m_mapSkillTree.clear();
	}

	if( true == bResetUnsealedSkill )
	{
		m_setUnsealedSkillID.clear();
	}

	//if( true == bResetSkillSlotB )
	//{
	//	m_bEnabledSkillSlotB = false;
	//}

	if( true == bResetCashSkillPoint )
	{
		m_iCSPoint = 0;
		m_iMaxCSPoint = 0;
		KncUtil::ConvertStringToCTime( std::wstring( L"2000-01-01 00:00:00" ), m_tCSPointEndDate );
	}

	//{{ 2010. 03. 22  ������	����� ��Ʈ
#ifdef SERV_SKILL_NOTE	
	if( true == bResetSkillNote )
	{
		m_cSkillNoteMaxPageNum = 0;
		m_mapSkillNote.clear();
	}
#endif SERV_SKILL_NOTE
	//}}
}

void KUserSkillTree::InitSkill( IN std::vector<KUserSkillData>& vecSkillList, IN int aSkillSlot[], IN std::wstring& wstrSkillSlotBEndDate, IN std::vector<short int>& vecUnsealedSkillList, IN int iUnitClass )
{
	m_iUnitClass = iUnitClass;

	m_mapSkillTree.clear();

	for( UINT i=0; i<vecSkillList.size(); i++ )
	{
		const KUserSkillData& userSkillData = vecSkillList[i];
		
		SkillDataMap::iterator mit;
		mit = m_mapSkillTree.find( (int)userSkillData.m_iSkillID );
		if( mit != m_mapSkillTree.end() )
		{
			START_LOG( cerr, L"�ߺ��� ��ų�� �����ϰ� ����." )
				<< BUILD_LOG( userSkillData.m_iSkillID )
				<< BUILD_LOG( userSkillData.m_cSkillLevel )
				<< BUILD_LOG( userSkillData.m_cSkillCSPoint )
				<< END_LOG;

			continue;
		}


		int iSkillLevel = (int) userSkillData.m_cSkillLevel;


		// ��ų�� �ְ��������� ���� ��ų�� �ִٸ� �ְ������� ����
		const CXSLSkillTree::SkillTreeTemplet* pSkillTreeTemplet = SiCXSLSkillTree()->GetSkillTreeTemplet( m_iUnitClass, userSkillData.m_iSkillID );
		if( NULL != pSkillTreeTemplet )
		{
			if( (int) userSkillData.m_cSkillLevel > pSkillTreeTemplet->m_iMaxSkillLevel )
			{
				START_LOG( cerr, L"�ְ��������� ���� ������ ��ų�� DB�� ����" )
					<< BUILD_LOG( m_iUnitClass )
					<< BUILD_LOG( userSkillData.m_iSkillID )
					<< BUILD_LOG( userSkillData.m_cSkillLevel )
					<< BUILD_LOG( pSkillTreeTemplet->m_iMaxSkillLevel )
					<< BUILD_LOG( userSkillData.m_cSkillCSPoint )
					<< END_LOG;

				iSkillLevel = pSkillTreeTemplet->m_iMaxSkillLevel;
			}
		}
		

		// �̹� �����ϸ� ���Ե��� �ʰ� �������� ������ ���Եȴ�.
		m_mapSkillTree[ userSkillData.m_iSkillID ] = UserSkillData( iSkillLevel, (int)userSkillData.m_cSkillCSPoint );
	}
	
	if( !aSkillSlot )
	{
		START_LOG( cerr, L"��ų ������ NULL" )
			<< END_LOG;

		return;
	}

	// ���� ��ų
	for( int i = 0; i < MAX_SKILL_SLOT; i++ )
	{
		m_aiSkillSlot[i] = aSkillSlot[i];
	}

	//  ���������� ��ų ���
	m_setUnsealedSkillID.clear();
	for( UINT i=0; i<vecUnsealedSkillList.size(); i++ )
	{
		m_setUnsealedSkillID.insert( (int) vecUnsealedSkillList[i] );
	}

	
	//{{ 2011. 01. 06  ��μ�  ��ų����ü���� üũ(�κ��丮-�Ⱓ��) ��� ����
#ifdef SERV_SKILL_SLOT_CHANGE_INVENTORY
	
	SetSkillSolotBEndDate( wstrSkillSlotBEndDate );
    
#else
	// ��ų ���� B
	m_wstrSkillSlotBEndDate = wstrSkillSlotBEndDate;

	if( true == wstrSkillSlotBEndDate.empty() )
	{
		// ���� �ð����� ����
		m_wstrSkillSlotBEndDate = L"2000-01-01 00:00:00";
		KncUtil::ConvertStringToCTime( m_wstrSkillSlotBEndDate, m_tSkillSlotBEndDate );
	}
	else if( !KncUtil::ConvertStringToCTime( wstrSkillSlotBEndDate, m_tSkillSlotBEndDate ) )
	{
		START_LOG( cerr, L"���ڿ� �ð� ��ȯ ����." )
			<< BUILD_LOG( wstrSkillSlotBEndDate )
			<< END_LOG;

		// ���� �ð����� ����
		m_wstrSkillSlotBEndDate = L"2000-01-01 00:00:00";
		KncUtil::ConvertStringToCTime( m_wstrSkillSlotBEndDate, m_tSkillSlotBEndDate );
	}
#endif SERV_SKILL_SLOT_CHANGE_INVENTORY
	//}}
}


//{{ 2010. 03. 22  ������	����� ��Ʈ
#ifdef SERV_SKILL_NOTE

void KUserSkillTree::InitSkillNote( IN char cSkillNoteMaxPageNum, IN const std::map< char, int >& mapSkillNote )
{
	m_mapSkillNote.clear();

	// ������Ʈ	
	m_cSkillNoteMaxPageNum = cSkillNoteMaxPageNum;
    m_mapSkillNote = mapSkillNote;
}

#endif SERV_SKILL_NOTE
//}}


int KUserSkillTree::GetSkillLevel( IN int iSkillID )
{
	SkillDataMap::iterator mit;
	mit = m_mapSkillTree.find( iSkillID );
	if( mit != m_mapSkillTree.end() )
	{
		const UserSkillData& userSkillData = mit->second;
		return userSkillData.m_iSkillLevel;
	}

	return 0;
}

bool KUserSkillTree::GetSkillLevelAndCSP( IN int iSkillID, OUT int& iSkillLevel, OUT int& iSkillCSPoint )
{
	SkillDataMap::iterator mit;
	mit = m_mapSkillTree.find( iSkillID );
	if( mit != m_mapSkillTree.end() )
	{
		const UserSkillData& userSkillData = mit->second;
		iSkillLevel		= userSkillData.m_iSkillLevel;
		iSkillCSPoint	= userSkillData.m_iSkillCSPoint;
		return true;
	}
	else
	{
		iSkillLevel		= 0;
		iSkillCSPoint	= 0;
	}

	return false;
}


// �����Ͱ� ������ ���� �����ϰ�, ������ �߰��Ѵ�
bool KUserSkillTree::SetSkillLevelAndCSP( int iSkillID, int iSkillLevel, int iSkillCSPoint )
{
	SET_ERROR( NET_OK );


	int iCheckSkillLevel = iSkillLevel;
	if( 0 == iSkillLevel )
	{
		iCheckSkillLevel = 1;
	}
	else if( iSkillLevel < 0 )
	{
		SET_ERROR( ERR_SKILL_20 );

		return false;
	}
	

	const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( iSkillID, iCheckSkillLevel );
	if( NULL == pSkillTemplet )
	{
		SET_ERROR( ERR_SKILL_19 );

		return false;
	}

	if( iSkillLevel < iSkillCSPoint )
	{
		SET_ERROR( ERR_SKILL_20 );

		return false;
	}


	if( iSkillLevel > SiCXSLSkillTree()->GetMaxSkillLevel( m_iUnitClass, iSkillID ) )
	{
		SET_ERROR( ERR_SKILL_21 );

		return false;
	}


	SkillDataMap::iterator mit;
	mit = m_mapSkillTree.find( iSkillID );
	if( mit != m_mapSkillTree.end() )
	{
		UserSkillData& userSkillData = mit->second;
		userSkillData.m_iSkillLevel		= iSkillLevel;
		userSkillData.m_iSkillCSPoint	= iSkillCSPoint;
	}
	else 
	{
		m_mapSkillTree[ iSkillID ] = UserSkillData( iSkillLevel, iSkillCSPoint );
	}

	return true;
}

bool KUserSkillTree::IsExist( IN int iSkillID )
{
	SkillDataMap::iterator mit;
	mit = m_mapSkillTree.find( iSkillID );
	if( mit != m_mapSkillTree.end() )
	{
		return true;
	}

	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
	if( iSkillID == CXSLSkillTree::SI_ETC_WS_COMMON_LOVE ) // 6001
	{
		return true;
	}
#endif SERV_RELATIONSHIP_SYSTEM
	//}

	return false;
}

bool KUserSkillTree::ChangeSkillSlot( int iSlotID, int iSkillID )
{
	SET_ERROR( NET_OK );


	if( iSlotID < 0 || iSlotID >= MAX_SKILL_SLOT )
	{
		SET_ERROR( ERR_SKILL_09 );

		return false;
	}

	
	
	if( 0 == iSkillID )
	{
		m_aiSkillSlot[iSlotID] = 0;

		return true;
	}



	if( iSkillID > 0 && IsExist( iSkillID ) == false )
	{
		SET_ERROR( ERR_SKILL_10 );

		return false;
	}




	if( IsSkillSlotB( iSlotID )  &&  iSkillID != 0 )
	{
		// [����] �κ��丮�� ��ų����B Ȯ�� �������� �ִ��� �˻縦 �ұ�? ����?

		CTime tCurrentTime = CTime::GetCurrentTime();
		if( tCurrentTime > m_tSkillSlotBEndDate )
		{
			SET_ERROR( ERR_SKILL_13 );

			return false;
		}
	}

	m_aiSkillSlot[iSlotID] = iSkillID;
	//m_aiSkillSlot[iSlotID].m_cSkillLevel = (UCHAR) GetSkillLevel( iSkillID );


	return true;
}

//{{ 2012. 12. 3	�ڼ���	��ų ���� ü���� ��Ŷ ����
#ifdef SERV_SKILL_SLOT_CHANGE_PACKET_INTEGRATE
bool KUserSkillTree::ChangeSkillSlot( IN const KEGS_CHANGE_SKILL_SLOT_REQ& kPacket_, OUT KEGS_CHANGE_SKILL_SLOT_ACK& kPacket )
{
	SET_ERROR( NET_OK );

	const iSlotID	= kPacket_.m_iSlotID;
	const iSkillID	= kPacket_.m_iSkillID;
	const iSlotID2	= GetSlotID( kPacket_.m_iSkillID );
	const iSkillID2	= GetSkillID( kPacket_.m_iSlotID );

	if( ( iSlotID < 0 ) || ( iSlotID >= MAX_SKILL_SLOT ) )
	{
		SET_ERROR( ERR_SKILL_09 );
		return false;
	}

	if( 0 == iSkillID )
	{
		m_aiSkillSlot[iSlotID] = 0;
	}
	else
	{
		if( iSkillID > 0 && IsExist( iSkillID ) == false )
		{
			SET_ERROR( ERR_SKILL_10 );
			return false;
		}

		if( IsSkillSlotB( iSlotID ) )
		{
			// [����] �κ��丮�� ��ų����B Ȯ�� �������� �ִ��� �˻縦 �ұ�? ����?

			CTime tCurrentTime = CTime::GetCurrentTime();
			if( tCurrentTime > m_tSkillSlotBEndDate )
			{
				SET_ERROR( ERR_SKILL_13 );
				return false;
			}
		}
	}

	if( ( 0 <= iSlotID2 ) && ( iSlotID2 < MAX_SKILL_SLOT ) )
	{
		if( 0 == iSkillID2 )
		{
			m_aiSkillSlot[iSlotID2] = 0;
		}
		else if( ( iSkillID2 > 0 ) && ( IsExist( iSkillID2 ) == false ) )
		{
			SET_ERROR( ERR_SKILL_10 );
			return false;
		}
		else
		{
			if( IsSkillSlotB( iSlotID2 ) )
			{
				// [����] �κ��丮�� ��ų����B Ȯ�� �������� �ִ��� �˻縦 �ұ�? ����?

				CTime tCurrentTime = CTime::GetCurrentTime();
				if( tCurrentTime > m_tSkillSlotBEndDate )
				{
					SET_ERROR( ERR_SKILL_13 );
					return false;
				}
			}
			m_aiSkillSlot[iSlotID2] = iSkillID2;
		}
	}

	m_aiSkillSlot[iSlotID] = iSkillID;

	kPacket.m_iSlotID	= iSlotID;
	kPacket.m_iSkillID	= iSkillID;
	kPacket.m_iSlotID2	= iSlotID2;
	kPacket.m_iSkillID2	= iSkillID2;

	return true;
}
#endif SERV_SKILL_SLOT_CHANGE_PACKET_INTEGRATE
//}}

void KUserSkillTree::GetSkillSlot( OUT std::vector<int>& vecSkillID )
{
	vecSkillID.resize(0);

	for( int i = 0; i < MAX_SKILL_SLOT; ++i )
	{
		vecSkillID.push_back( m_aiSkillSlot[i] );
	}
}




void KUserSkillTree::GetSkillSlot( OUT std::vector<KSkillData>& vecSkillSlot )
{
	vecSkillSlot.resize(0);

	for( int i = 0; i < MAX_SKILL_SLOT; ++i )
	{
		KSkillData kSkillData;
		kSkillData.m_iSkillID = m_aiSkillSlot[i];
		kSkillData.m_cSkillLevel = (char) GetSkillLevel( kSkillData.m_iSkillID );
		vecSkillSlot.push_back( kSkillData );
	}
}


void KUserSkillTree::GetSkillSlot( OUT KSkillData aSkillSlot[] )
{
	if( !aSkillSlot )
	{
		START_LOG( cerr, L"��ų ������ NULL" )
			<< END_LOG;

		return;
	}

	for( int i = 0; i < MAX_SKILL_SLOT; ++i )
	{
		aSkillSlot[i].m_iSkillID = m_aiSkillSlot[i];
		aSkillSlot[i].m_cSkillLevel = (char) GetSkillLevel( aSkillSlot[i].m_iSkillID );
	}
}

//{{ 2012. 12. 3	�ڼ���	��ų ���� ü���� ��Ŷ ����
#ifdef SERV_SKILL_SLOT_CHANGE_PACKET_INTEGRATE
int KUserSkillTree::GetSkillID( int iSlotID )
{
	if( iSlotID < 0 || iSlotID >= MAX_SKILL_SLOT )
	{
		return 0;
	}

	return m_aiSkillSlot[iSlotID];
}

int KUserSkillTree::GetSlotID( int iSkillID )
{
	if( IsExist( iSkillID ) == false )
	{
		return -1;
	}

	for( int i=0; i < MAX_SKILL_SLOT; ++i )
	{
		if( m_aiSkillSlot[i] == iSkillID )
		{
			return i;
		}
	}

	return -1;
}
#endif SERV_SKILL_SLOT_CHANGE_PACKET_INTEGRATE
//}}

// �нú� ��ų ID�� ���� ������ �������ش�. ����, PVP���ӿ��� �� ���ֿ��� �ٸ� ������ ��ų ������ �˷��ֱ� ���ؼ�
void KUserSkillTree::GetPassiveSkillData( OUT std::vector<KSkillData>& vecSkillSlot )
{
	vecSkillSlot.resize(0);

	SkillDataMap::iterator it;
	for( it = m_mapSkillTree.begin(); it != m_mapSkillTree.end(); it++ )
	{
		KSkillData kSkillData;
		kSkillData.m_iSkillID		= it->first;
		kSkillData.m_cSkillLevel	= (UCHAR) it->second.m_iSkillLevel;
		vecSkillSlot.push_back( kSkillData );
	}
}

void KUserSkillTree::CalcUsedSPointAndCSPoint( OUT int& iSPoint, OUT int& iCSPoint )
{
	iSPoint = 0;
	iCSPoint = 0;

	for( SkillDataMap::iterator mit = m_mapSkillTree.begin(); mit != m_mapSkillTree.end(); ++mit )
	{
		const int iSkillID = (int)mit->first;
		const UserSkillData& userSkillData = mit->second;


		if( userSkillData.m_iSkillLevel > 0 )
		{
			if( userSkillData.m_iSkillCSPoint > userSkillData.m_iSkillLevel )
			{
				START_LOG( cerr, L"��ų ���� < ��ų CSP" )
					<< BUILD_LOG( iSkillID )
					<< BUILD_LOG( userSkillData.m_iSkillLevel )
					<< BUILD_LOG( userSkillData.m_iSkillCSPoint )
					<< END_LOG;
			}

			iSPoint		+= userSkillData.m_iSkillLevel - userSkillData.m_iSkillCSPoint;
			iCSPoint	+= userSkillData.m_iSkillCSPoint;
		}
		else if( userSkillData.m_iSkillLevel == 0 )
		{
			if( userSkillData.m_iSkillCSPoint > 0 )
			{
				START_LOG( cwarn, L"��ų ������ 0�����ε� SkillCSP�� 0���� ŭ!" )
					<< BUILD_LOG( iSkillID )
					<< BUILD_LOG( userSkillData.m_iSkillLevel )
					<< BUILD_LOG( userSkillData.m_iSkillCSPoint )
					<< END_LOG;
			}
		}
		else
		{
			START_LOG( cwarn, L"��ų ������ 0���� �۴�!" )
				<< BUILD_LOG( iSkillID )
				<< BUILD_LOG( userSkillData.m_iSkillLevel )
				<< BUILD_LOG( userSkillData.m_iSkillCSPoint )
				<< END_LOG;
		}
	}
}

// tier���� �Ҹ�� ���� SP+CSP�� ������ش�
void KUserSkillTree::CalcCumulativeUsedSPointOnEachTier( OUT std::vector< int >& vecTierSPoint )
{
	int iMaxTierIndex = 0;
	for( SkillDataMap::iterator mit = m_mapSkillTree.begin(); mit != m_mapSkillTree.end(); ++mit )
	{
		const UserSkillData& userSkillData = mit->second;
	
		// ��ų �������� ��ȿ���� �˻�
		if( userSkillData.m_iSkillLevel <= 0 )
			continue;

		const CXSLSkillTree::SkillTreeTemplet* pSkillTreeTemplet = SiCXSLSkillTree()->GetSkillTreeTemplet( m_iUnitClass, mit->first );
		if( pSkillTreeTemplet == NULL )
			continue;

		if( iMaxTierIndex < pSkillTreeTemplet->m_iTier )
			iMaxTierIndex = pSkillTreeTemplet->m_iTier;
	}

	vecTierSPoint.resize(0);
	for( int i = 0; i <= iMaxTierIndex; ++i )
	{
		vecTierSPoint.push_back( 0 );
	}

	for( SkillDataMap::iterator mit = m_mapSkillTree.begin(); mit != m_mapSkillTree.end(); ++mit )
	{
		const UserSkillData& userSkillData = mit->second;
	
		// ��ų �������� ��ȿ���� �˻�
		if( userSkillData.m_iSkillLevel <= 0 )
			continue;

		const CXSLSkillTree::SkillTreeTemplet* pSkillTreeTemplet = SiCXSLSkillTree()->GetSkillTreeTemplet( m_iUnitClass, mit->first );
		if( pSkillTreeTemplet == NULL )
			continue;

		//{{ 2010. 8. 17	������	��ų �ǵ����� ���� ����
#ifdef SERV_RESET_SKILL_BUG_FIX
		// ���� �� Tier���� SPoint ���Ҷ� CSP�� �����ϰ� ���մϴ�!
		vecTierSPoint[ pSkillTreeTemplet->m_iTier ] += ( userSkillData.m_iSkillLevel - userSkillData.m_iSkillCSPoint );
#else
		vecTierSPoint[ pSkillTreeTemplet->m_iTier ] += userSkillData.m_iSkillLevel;
#endif SERV_RESET_SKILL_BUG_FIX
		//}}
	}

	int iCumulativeTierSPoint = 0;
	for( UINT ui = 0; ui < vecTierSPoint.size(); ++ui )
	{
		iCumulativeTierSPoint += vecTierSPoint[ui];
		vecTierSPoint[ui] = iCumulativeTierSPoint;
	}
}

bool KUserSkillTree::IsCashSkillPointExpired()
{
	if( 0 == m_iMaxCSPoint )
		return true;
	
	return false;
}



// cash skill point�� ���� ��ų�� ������ �����ϰ� ������ ��ų ����Ʈ�� ��길 ���ش�
// @iRetrievedSPoint: ������ SP ��ġ
void KUserSkillTree::CalcExpireCashSkillPoint( OUT int& iRetrievedSPoint, OUT std::vector<KUserSkillData>& vecModifiedUserSkillData )
{
	iRetrievedSPoint = 0;
	vecModifiedUserSkillData.clear();
	
	for( SkillDataMap::iterator mit = m_mapSkillTree.begin(); mit != m_mapSkillTree.end(); ++mit )
	{
		const int iSkillID = mit->first;
		const UserSkillData& userSkillData = mit->second;
		
		// ĳ�� ��ų�� �� ����� �ִٸ�?
		if( userSkillData.m_iSkillCSPoint > 0 )
		{
			iRetrievedSPoint	+= userSkillData.m_iSkillCSPoint;
			UCHAR uNewSkillLevel = (UCHAR) ( userSkillData.m_iSkillLevel - userSkillData.m_iSkillCSPoint );

			// �⺻��ų ������ 0�� �Ǵ� ��� ���� �α׸� ����� 1�� ����� �ش�
			if( SiCXSLSkillTree()->IsUnitTypeDefaultSkill( iSkillID ) == true )
			{
				if( uNewSkillLevel <= 0 )
				{
					START_LOG( cerr, L"CSP �Ⱓ����ÿ� �⺻��ų ������ 0������ ������ �ٲٷ��� ����߽��ϴ�!!" )
						<< BUILD_LOG( iSkillID )
						<< BUILD_LOG( userSkillData.m_iSkillLevel )
						<< BUILD_LOG( userSkillData.m_iSkillCSPoint )
						<< END_LOG;

					uNewSkillLevel = 1;
				}
			}

			vecModifiedUserSkillData.push_back( KUserSkillData( (short) iSkillID, uNewSkillLevel, 0 ) );
		}
	}

	// ��ųƮ���� �Ҹ��ߴ� CSP �� �����ִ� CSP �� ���ؼ� ���Ž��� CSP�� ���� ������ SP ��ġ�� ���ȴ�
	iRetrievedSPoint += m_iCSPoint - m_iMaxCSPoint;
}


// ������ cash skill point�� ȹ���� ��ų�� �ǵ�����.
void KUserSkillTree::ExpireCashSkillPoint()
{
	for( SkillDataMap::iterator mit = m_mapSkillTree.begin(); mit != m_mapSkillTree.end(); ++mit )
	{
		int iSkillID = mit->first;
		UserSkillData& userSkillData = mit->second;

		if( userSkillData.m_iSkillCSPoint > 0 )
		{
			int iNewSkillLevel = userSkillData.m_iSkillLevel - userSkillData.m_iSkillCSPoint;
						
			// �⺻��ų ������ 0�� �Ǵ� ��� ���� �α׸� ����� 1�� ����� �ش�
			if( true == SiCXSLSkillTree()->IsUnitTypeDefaultSkill( iSkillID ) )
			{
				if( iNewSkillLevel <= 0 )
				{
					START_LOG( cerr, L"CSP �Ⱓ����ÿ� �⺻��ų ������ 0������ ������ �ٲٷ��� �߽��ϴ�!!" )
						<< BUILD_LOG( iSkillID )
						<< BUILD_LOG( userSkillData.m_iSkillLevel )
						<< BUILD_LOG( userSkillData.m_iSkillCSPoint )
						<< END_LOG;

					iNewSkillLevel = 1;
				}
			}
			
			userSkillData.m_iSkillLevel		= iNewSkillLevel;
			userSkillData.m_iSkillCSPoint	= 0;
		}
	}

	// ������ ��ų �߿��� skill level�� 0������ ��ų�� ������ Ż����Ų��.
	for( int i=0; i<MAX_SKILL_SLOT; i++ )
	{
		if( GetSkillLevel( m_aiSkillSlot[i] ) <= 0 )
		{
			ChangeSkillSlot( i, 0 );
		}
	}

	m_iMaxCSPoint = 0;
	m_iCSPoint = 0;
}








bool KUserSkillTree::IsSkillUnsealed( int iSkillID )
{
	std::set< int >::iterator it = m_setUnsealedSkillID.find( iSkillID );
	if( it != m_setUnsealedSkillID.end() )
	{
		return true;
	}

	return false;
}

//{{ 2009. 8. 4  ������		��ų��������
bool KUserSkillTree::SkillUnseal( int iSkillID )
{
	if( IsSkillUnsealed( iSkillID ) )
	{
		START_LOG( cerr, L"�̹� ���������� ��ų�Դϴ�. �Ͼ���� �ȵǴ� ����! �˻������ٵ�.." )
			<< BUILD_LOG( iSkillID )
			<< END_LOG;

		return false;
	}

	m_setUnsealedSkillID.insert( iSkillID );
	return true;
}
//}}


bool KUserSkillTree::SetCSPointEndDate( std::wstring wstrEndDate )
{
	if( true == wstrEndDate.empty() )
		return false;

	if( !KncUtil::ConvertStringToCTime( wstrEndDate, m_tCSPointEndDate ) )
		return false;

	m_wstrCSPointEndDate = wstrEndDate;

	return true;
}



void KUserSkillTree::ExpandSkillSlotB( std::wstring& wstrSkillSlotBEndDate )
{	
	m_wstrSkillSlotBEndDate = wstrSkillSlotBEndDate;

	if( !KncUtil::ConvertStringToCTime( wstrSkillSlotBEndDate, m_tSkillSlotBEndDate ) )
	{
		START_LOG( cerr, L"���ڿ� �ð� ��ȯ ����." )
			<< BUILD_LOG( wstrSkillSlotBEndDate )
			<< END_LOG;

		// ���� �ð����� ����
		m_wstrSkillSlotBEndDate = L"2000-01-01 00:00:00";
		KncUtil::ConvertStringToCTime( m_wstrSkillSlotBEndDate, m_tSkillSlotBEndDate );
	}

	for( int i = SKILL_SLOT_B1; i < MAX_SKILL_SLOT; ++i )
	{
		m_aiSkillSlot[i] = 0;
	}
}




KUserSkillTree::SKILL_SLOT_B_EXPIRATION_STATE KUserSkillTree::GetSkillSlotBExpirationState()
{
	// note!! ��ų ���� Ȯ�� ������ ���Ⱓ�� 1�� �̻� �������� ���� ���������� �����Ѵ�.
	const CTimeSpan MAGIC_PERMANENT_TIME_SPAN = CTimeSpan( 365, 0, 0, 0 );

	CTime tCurrentTime = CTime::GetCurrentTime();
	if( tCurrentTime >= m_tSkillSlotBEndDate )
	{
		return SSBES_EXPIRED;
	}
	else
	{
		CTimeSpan expirationTimeLeft = m_tSkillSlotBEndDate - tCurrentTime;

		if( expirationTimeLeft > MAGIC_PERMANENT_TIME_SPAN )
		{
			return SSBES_PERMANENT;
		}
		else
		{
			return SSBES_NOT_EXPIRED;
		}
	}
}



void KUserSkillTree::ExpireSkillSlotB()
{
	// ���� �ð��� ���Ͽ� ����ð��� ��
	CTime tCurrentTime = CTime::GetCurrentTime();
	if( tCurrentTime >= m_tSkillSlotBEndDate )
	{
		// ���� ȣ��Ǵ� �κ��̱� ������ �ڵ� ����ȭ (for�� ����)
		m_aiSkillSlot[SKILL_SLOT_B1] = 0;
		m_aiSkillSlot[SKILL_SLOT_B2] = 0;
		m_aiSkillSlot[SKILL_SLOT_B3] = 0;
		m_aiSkillSlot[SKILL_SLOT_B4] = 0;
	}
}




bool KUserSkillTree::IsMyUnitClassSkill( int iSkillID )
{
	if( SiCXSLSkillTree()->GetMaxSkillLevel( m_iUnitClass, iSkillID ) > 0 )
		return true;

	return false;
}



bool KUserSkillTree::IsAllPrecedingSkillMaxLevel( int iSkillID )
{
	const CXSLSkillTree::SkillTreeTemplet* pSkillTreeTemplet = SiCXSLSkillTree()->GetSkillTreeTemplet( m_iUnitClass, iSkillID );
	if( NULL == pSkillTreeTemplet )
	{
		START_LOG( cerr, L"��ųƮ�� ���ø��� ����_preceding" )
			<< BUILD_LOG( m_iUnitClass )
			<< BUILD_LOG( iSkillID )
			<< END_LOG;

		return false;
	}


	for( UINT i=0; i<pSkillTreeTemplet->m_vecPrecedingSkill.size(); i++ )
	{
		int iPrecedingSkillID = pSkillTreeTemplet->m_vecPrecedingSkill[i];
		if( false == IsMaxSkillLevel( iPrecedingSkillID ) )
		{
			return false;
		}
	}

	return true;
}



bool KUserSkillTree::IsAllFollowingSkillLevelZero( int iSkillID )
{
	const CXSLSkillTree::SkillTreeTemplet* pSkillTreeTemplet = SiCXSLSkillTree()->GetSkillTreeTemplet( m_iUnitClass, iSkillID );
	if( NULL == pSkillTreeTemplet )
	{
		START_LOG( cerr, L"��ųƮ�� ���ø��� ����_following" )
			<< BUILD_LOG( m_iUnitClass )
			<< BUILD_LOG( iSkillID )
			<< END_LOG;

		return false;
	}

	BOOST_TEST_FOREACH( const int, iFollowingSkillID, pSkillTreeTemplet->m_vecFollowingSkill )
	{
		if( GetSkillLevel( iFollowingSkillID ) > 0 )
		{
			return false;
		}
	}

	return true;
}


// �� ��ų�� ���⿡ ����� ��ŭ SP�� �Ҹ��ߴ���
bool KUserSkillTree::IsTierOpened( int iSkillID )
{
	const CXSLSkillTree::SkillTreeTemplet* pSkillTreeTemplet = SiCXSLSkillTree()->GetSkillTreeTemplet( m_iUnitClass, iSkillID );
	if( NULL == pSkillTreeTemplet )
	{
		START_LOG( cerr, L"��ųƮ�� ���ø��� ����_IsTierOpened" )
			<< BUILD_LOG( m_iUnitClass )
			<< BUILD_LOG( iSkillID )
			<< END_LOG;

		return false;
	}


	const int MAGIC_SKILL_POINT_PER_TIER = 5;
	int iUsedSPoint = 0;
	int iUsedCSPoint = 0;
	CalcUsedSPointAndCSPoint( iUsedSPoint, iUsedCSPoint );
	if( iUsedSPoint + iUsedCSPoint < pSkillTreeTemplet->m_iTier * MAGIC_SKILL_POINT_PER_TIER )
	{
		return false;
	}

	return true;
}


//{{ 2010. 03. 22  ������	����� ��Ʈ
#ifdef SERV_SKILL_NOTE

void KUserSkillTree::GetSkillNote( OUT std::vector< int >& vecSkillNote )
{
	vecSkillNote.clear();

	std::map< char, int >::const_iterator mit;
	for( mit = m_mapSkillNote.begin(); mit != m_mapSkillNote.end(); ++mit )
	{
		vecSkillNote.push_back( mit->second );
	}
}

bool KUserSkillTree::GetExpandSkillNotePage( IN u_char ucLevel, OUT char& cPageNum )
{
	cPageNum = 0;

	const u_char ucCheckNum = ucLevel / 10; // 10�� �ڸ��� ���ϱ� ���� 10���� ����
	switch( ucCheckNum )
	{
	case 0:
	case 1:
		return false;

	case 2:
		cPageNum = 1;
		return true;

	case 3:
		cPageNum = 2;
		return true;

	case 4:
		cPageNum = 3;
		return true;

	case 5:
		cPageNum = 4;
		return true;
		//{{ 2011. 07. 13	������	���� Ȯ��
#ifdef SERV_EXPAND_60_LIMIT_LEVEL
	case 6:
		cPageNum = 5;
		return true;
#endif SERV_EXPAND_60_LIMIT_LEVEL
		//}}
	}

	return false;
}

bool KUserSkillTree::IsExistSkillNotePage( IN char cPageNum )
{
    if( m_cSkillNoteMaxPageNum > cPageNum )
		return true;

	return false;
}

bool KUserSkillTree::IsExistSkillNoteMemoID( IN int iSkillNoteMemoID )
{
	std::map< char, int >::const_iterator mit;
	for( mit = m_mapSkillNote.begin(); mit != m_mapSkillNote.end(); ++mit )
	{
        if( mit->second == iSkillNoteMemoID )
			return true;
	}

	return false;
}

void KUserSkillTree::UpdateSkillNoteMemo( IN char cPageNum, IN int iMemoID )
{
	std::map< char, int >::iterator mit;
	mit = m_mapSkillNote.find( cPageNum );
	if( mit == m_mapSkillNote.end() )
	{
		m_mapSkillNote.insert( std::make_pair( cPageNum, iMemoID ) );
	}
	else
	{
        mit->second = iMemoID;
	}
}

#endif SERV_SKILL_NOTE
//}}

//{{ 2011. 01. 06  ��μ�  ��ų����ü���� üũ(�κ��丮-�Ⱓ��) ��� ����
#ifdef SERV_SKILL_SLOT_CHANGE_INVENTORY
void KUserSkillTree::SetSkillSolotBEndDate( IN std::wstring& wstrSkillSlotBEndDate )
{
	// ��ų ���� B
	m_wstrSkillSlotBEndDate = wstrSkillSlotBEndDate;

	if( true == wstrSkillSlotBEndDate.empty() )
	{
		// ���� �ð����� ����
		m_wstrSkillSlotBEndDate = L"2000-01-01 00:00:00";
		KncUtil::ConvertStringToCTime( m_wstrSkillSlotBEndDate, m_tSkillSlotBEndDate );
	}
	else if( !KncUtil::ConvertStringToCTime( wstrSkillSlotBEndDate, m_tSkillSlotBEndDate ) )
	{
		START_LOG( cerr, L"���ڿ� �ð� ��ȯ ����." )
			<< BUILD_LOG( wstrSkillSlotBEndDate )
			<< END_LOG;

		// ���� �ð����� ����
		m_wstrSkillSlotBEndDate = L"2000-01-01 00:00:00";
		KncUtil::ConvertStringToCTime( m_wstrSkillSlotBEndDate, m_tSkillSlotBEndDate );
	}
}
#endif SERV_SKILL_SLOT_CHANGE_INVENTORY
//}}

//{{ 2011. 11. 21  ��μ�	���� ���� ������
#ifdef SERV_UNIT_CLASS_CHANGE_ITEM
void KUserSkillTree::GetUnSealedSkillList( OUT std::vector< short >& vecUnsealedSkillID )
{
	std::set< int >::iterator sit = m_setUnsealedSkillID.begin();
	for( ; sit != m_setUnsealedSkillID.end() ; ++sit )
	{
		vecUnsealedSkillID.push_back( static_cast<short>(*sit) );
	}
}

void KUserSkillTree::SetClassChangeSkill( IN std::map< int, int >& mapSkill )
{
	std::set< int >::iterator sit;

	std::map< int, int >::iterator mit = mapSkill.begin();
	for(  ; mit != mapSkill.end() ; ++mit )
	{
		int itempfirst = mit->first;
		sit = m_setUnsealedSkillID.find( mit->first );
		if( sit != m_setUnsealedSkillID.end() )
		{
			m_setUnsealedSkillID.erase( sit );
			m_setUnsealedSkillID.insert( mit->second );
		}
		else
		{
			START_LOG( cerr, L"�̷� ��ų�� ������ ���� �ʴٰ�?! �ִٰ� �ؼ� ����Ѱ��ݾ�!!" )
				<< BUILD_LOG( mit->first )
				<< BUILD_LOG( mit->second )
				<< END_LOG;
		}
	}
}

void KUserSkillTree::SetClassChangeMemo( IN std::map< int, int >& mapMemo )
{
	std::map< char, int >::iterator mymit;
	
	std::map< int, int >::iterator mit = mapMemo.begin();
	for(  ; mit != mapMemo.end() ; ++mit )
	{
		mymit = m_mapSkillNote.begin();
		for( ; mymit != m_mapSkillNote.end() ; ++mymit )
		{
			if( mymit->second == mit->first )
			{
				mymit->second = mit->second;
			}
		}
	}
}
#endif SERV_UNIT_CLASS_CHANGE_ITEM
//}}

void KUserSkillTree::GetSkillStat( KStat& kStat )
{
	kStat.Init();


	SkillDataMap::iterator mit;
	for( mit = m_mapSkillTree.begin(); mit != m_mapSkillTree.end(); ++mit )
	{
		const UserSkillData& userSkillData = mit->second;
		if( userSkillData.m_iSkillLevel <= 0 )
			continue;

		const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( mit->first, userSkillData.m_iSkillLevel );

		if( pSkillTemplet == NULL )
		{
			START_LOG( cerr, L"��ų ���ø� ����Ʈ �̻�.!" )
				<< BUILD_LOG( mit->first )
				<< BUILD_LOG( userSkillData.m_iSkillLevel )
				<< END_LOG;

			continue;
		}

		switch( pSkillTemplet->m_eType )
		{
		case CXSLSkillTree::ST_PASSIVE_PHYSIC_ATTACK: 
		case CXSLSkillTree::ST_PASSIVE_MAGIC_ATTACK:  
		case CXSLSkillTree::ST_PASSIVE_MAGIC_DEFENCE: 
		case CXSLSkillTree::ST_PASSIVE_PHYSIC_DEFENCE:
			{
				kStat.m_iBaseHP			+= (int)pSkillTemplet->m_Stat.m_fBaseHP;
				kStat.m_iAtkPhysic		+= (int)pSkillTemplet->m_Stat.m_fAtkPhysic;
				kStat.m_iAtkMagic		+= (int)pSkillTemplet->m_Stat.m_fAtkMagic;
				kStat.m_iDefPhysic		+= (int)pSkillTemplet->m_Stat.m_fDefPhysic;
				kStat.m_iDefMagic		+= (int)pSkillTemplet->m_Stat.m_fDefMagic;
			} break;
		}
	}
}

bool KUserSkillTree::IsMaxSkillLevel( IN int iSkillID )
{
	const CXSLSkillTree::SkillTemplet* pSkillTemplet = SiCXSLSkillTree()->GetSkillTemplet( iSkillID, 1 );
	if( NULL == pSkillTemplet )
	{
		START_LOG( cerr, L"��ų�� �ְ� �������� Ȯ���Ϸ��µ� ��ų ���ø��� NULL" )
			<< BUILD_LOG( iSkillID )
			<< END_LOG;

		return false;
	}

	SkillDataMap::iterator mit;
	mit = m_mapSkillTree.find( iSkillID );
	if( mit != m_mapSkillTree.end() )
	{
		const UserSkillData& userSkillData = mit->second;
		int iMaxSkillLevel = SiCXSLSkillTree()->GetMaxSkillLevel( m_iUnitClass, iSkillID );
		if( userSkillData.m_iSkillLevel > iMaxSkillLevel )
		{
			START_LOG( cerr, L"��ų�� �ְ� �������� Ȯ���Ϸ��µ� ��ų ������ �ְ��������� ����" )
				<< BUILD_LOG( m_iUnitClass )
				<< BUILD_LOG( iSkillID )
				<< BUILD_LOG( userSkillData.m_iSkillLevel )
				<< BUILD_LOG( iMaxSkillLevel )
				<< END_LOG;

			return true;
		}
		else if( userSkillData.m_iSkillLevel == iMaxSkillLevel )
		{
			return true;
		}
		else 
		{
			return false;
		}
	}

	return false;
}
*/
#endif	// SERV_UPGRADE_SKILL_SYSTEM_2013

