#include "ClassChangeTable.h"
#include "X2Data/XSLUnit.h"

//////////////////////////////////////////////////////////////////////////
//{{ 2011. 11. 21  ��μ�	���� ���� ������
#ifdef SERV_UNIT_CLASS_CHANGE_ITEM
//////////////////////////////////////////////////////////////////////////

ImplementRefreshSingleton( KClassChangeTable );
KClassChangeTable::KClassChangeTable(void)
{
}

KClassChangeTable::~KClassChangeTable(void)
{
}

ImplToStringW( KClassChangeTable )
{
	/*
	stm_	<< L"----------[ World Mission Manager ]----------" << std::endl
		<< TOSTRINGW( m_bActive )
		<< BUILD_LOG( m_iTotalCCU )
		<< BUILD_LOG( m_iMissionTime )
		<< BUILD_LOG( m_iNextDOT )
		<< BUILD_LOG( m_usWorldBuffDurationTime )
		<< BUILD_LOG( m_usWorldBuffDurationTime )
		<< BUILD_LOG( m_iCrystalTargetCount )
		<< BUILD_LOG( m_iProtectedCrystalCount )
		<< BUILD_LOG( m_fCountPerWorldCrystal )
		<< BUILD_LOG( m_iWorldCrystalCount )
		<< BUILD_LOG( m_fNormalPlayRate )
		<< BUILD_LOG( m_fHardPlayRate )
		<< BUILD_LOG( m_fExpertPlayRate )
		<< BUILD_LOG( m_fNormalSuccessRate )
		<< BUILD_LOG( m_fHardSuccessRate )
		<< BUILD_LOG( m_fExpertSuccessRate )
		<< TOSTRINGW( m_kTimerDOT.elapsed() )
		;
	*/

	return stm_;
}

ImplementLuaScriptParser( KClassChangeTable )
{

	lua_tinker::class_add<KClassChangeTable>( GetLuaState(), "KClassChangeTable" );
	lua_tinker::class_def<KClassChangeTable>( GetLuaState(), "dump",									&KClassChangeTable::Dump );

	lua_tinker::class_def<KClassChangeTable>( GetLuaState(), "SetClassGroupID"	,						&KClassChangeTable::SetClassGroupID_Lua );
	lua_tinker::class_def<KClassChangeTable>( GetLuaState(), "SetClassChange_Skill",					&KClassChangeTable::SetClassChange_Skill_Lua );
	lua_tinker::class_def<KClassChangeTable>( GetLuaState(), "SetClassChange_Memo",						&KClassChangeTable::SetClassChange_Memo_Lua );
	lua_tinker::class_def<KClassChangeTable>( GetLuaState(), "SetClassChange_InventoryItem",			&KClassChangeTable::SetClassChange_InventoryItem_Lua );
	lua_tinker::class_def<KClassChangeTable>( GetLuaState(), "SetClassChange_Quest",					&KClassChangeTable::SetClassChange_Quest_Lua );
	lua_tinker::class_def<KClassChangeTable>( GetLuaState(), "SetClassChange_DelQuest",					&KClassChangeTable::SetClassChange_DelQuest_Lua );

	lua_tinker::decl( GetLuaState(), "ClassChangeTable", this );
}

// ������ �׷�id ���
void KClassChangeTable::SetClassGroupID_Lua( IN int iClassGroup, IN int iUnitClass )
{
	if( iClassGroup <= 0 || iUnitClass <= CXSLUnit::UC_NONE || iUnitClass >= CXSLUnit::UC_MAX )
	{
		START_LOG( cerr, L"���� ���濡 �ʿ��� �������� ��ũ��Ʈ �Ľ� ����!" )
			<< BUILD_LOG( iClassGroup )
			<< BUILD_LOG( iUnitClass )
			<< END_LOG;
		return;
	}

	std::map< int, int >::iterator mitClassGroupID = m_mapClassGroupID.find( iUnitClass );
	if( mitClassGroupID != m_mapClassGroupID.end() )
	{
		START_LOG( cerr, L"���� ���濡 �ʿ��� �������� �ߺ� �Ľ�!" )
			<< BUILD_LOG( iClassGroup )
			<< BUILD_LOG( iUnitClass )
			<< END_LOG;
		return;
	}

	m_mapClassGroupID.insert( std::make_pair( iUnitClass, iClassGroup ) );

	std::map< int, std::vector< int > >::iterator mitGroupID = m_mapUnitClassGroupID.find( iClassGroup );
	if( mitGroupID == m_mapUnitClassGroupID.end() )
	{
		std::vector< int > vecTemp;
		vecTemp.push_back( iUnitClass );

		m_mapUnitClassGroupID.insert( std::make_pair( iClassGroup, vecTemp ) );
	}
	else
	{
		std::vector< int >::iterator vit = mitGroupID->second.begin();
		if( vit == mitGroupID->second.end() )
		{
			START_LOG( cerr, L"ClassGroupID �� �����Ͱ� �ִٰ� �ߴµ�....����ִ�. �ٽ� ����!" )
				<< BUILD_LOG( iClassGroup )
				<< BUILD_LOG( iUnitClass )
				<< END_LOG;

			mitGroupID->second.push_back( iUnitClass );
		}
		else
		{
			bool bRet = true;
			//{{ Iruha : 2026-08-27 // VS2010 port: `vit != NULL` compared a vector iterator to
			// NULL, which VC7.1's raw-pointer-backed iterator allowed; VC10's checked iterator
			// type has no such comparison. The check was vacuous inside this loop (vit is
			// always a valid, non-end iterator here), so it's dropped rather than replaced.
			for( ; vit != mitGroupID->second.end() ; ++vit )
			{
				if( (*vit) == iUnitClass )
				{
					START_LOG( cerr, L"ClassGroupID �� �����Ͱ� �ִٰ� �ߴµ�....�ߺ� �Ǿ� �ִ�!" )
						<< BUILD_LOG( iClassGroup )
						<< BUILD_LOG( iUnitClass )
						<< BUILD_LOG( (*vit) )
						<< END_LOG;

					bRet = false;
					break;
				}
			}
			//}}

			if( bRet == true )
			{
				mitGroupID->second.push_back( iUnitClass );
			}
		}
	}
}

// ��ų
void KClassChangeTable::SetClassChange_Skill_Lua( IN int iSkillGroup, IN int iSkill, IN int iUnitClass )
{
	if( iSkillGroup <= 0 || iSkill <= 0 || iUnitClass <= CXSLUnit::UC_NONE || iUnitClass >= CXSLUnit::UC_MAX )
	{
		START_LOG( cerr, L"���� ���濡 �ʿ��� ��ų���� ��ũ��Ʈ �Ľ� ����!" )
			<< BUILD_LOG( iSkillGroup )
			<< BUILD_LOG( iSkill )
			<< BUILD_LOG( iUnitClass )
			<< END_LOG;
		return;
	}

	// ��ų�� �׷�id ����
	std::map< int, int >::iterator mitSkillGroupID = m_mapSkillGroupID.find( iSkill );
	if( mitSkillGroupID != m_mapSkillGroupID.end() )
	{
		START_LOG( clog, L"��ų �� �׷� ���̵� �ߺ� �Ľ�!!! ���� ������ ���� ������ ��ų�� ���� �� �����Ƿ� ������ �ƴϴ�" )
			<< BUILD_LOG( iSkillGroup )
			<< BUILD_LOG( iSkill )
			<< BUILD_LOG( iUnitClass )
			<< END_LOG;
	}
	else
	{
		m_mapSkillGroupID.insert( std::make_pair( iSkill, iSkillGroup ) );
	}

	// ���� ��ų ����� ���� �� �����̳ʿ� ����
	std::map< int, std::map< int, int > >::iterator mit = m_mapClassChangeSkill.find( iSkillGroup );
	if( mit == m_mapClassChangeSkill.end() )
	{
		std::map< int, int > mapTemp;
		mapTemp.insert( std::make_pair( iUnitClass, iSkill ) );

		m_mapClassChangeSkill.insert( std::make_pair( iSkillGroup, mapTemp) );
	}
	else
	{
		std::map< int, int >::iterator mitGroup = mit->second.find( iUnitClass );
		if( mitGroup != mit->second.end() )
		{
			START_LOG( clog, L"���� ���濡 �ʿ��� ��ų���� �ߺ� �Ľ�!" )
				<< BUILD_LOG( iSkillGroup )
				<< BUILD_LOG( iSkill )
				<< BUILD_LOG( iUnitClass )
				<< END_LOG;
			return;
		}

		mit->second.insert( std::make_pair( iUnitClass, iSkill ) );
	}
}

// �޸�
void KClassChangeTable::SetClassChange_Memo_Lua( IN int iMemoGroup, IN int iMemo, IN int iUnitClass )
{
	if( iMemoGroup < 0 || iMemo <= 0 || iUnitClass <= CXSLUnit::UC_NONE || iUnitClass >= CXSLUnit::UC_MAX )
	{
		START_LOG( cerr, L"���� ���濡 �ʿ��� �޸���� ��ũ��Ʈ �Ľ� ����!" )
			<< BUILD_LOG( iMemoGroup )
			<< BUILD_LOG( iMemo )
			<< BUILD_LOG( iUnitClass )
			<< END_LOG;
		return;
	}

	// �޸� �׷�id ����
	std::map< int, int >::iterator mitMemoGroupID = m_mapMemoGroupID.find( iMemo );
	if( mitMemoGroupID != m_mapMemoGroupID.end() )
	{
		START_LOG( clog, L"�޸� �� �׷� ���̵� �ߺ� �Ľ�!!!" )
			<< BUILD_LOG( iMemoGroup )
			<< BUILD_LOG( iMemo )
			<< BUILD_LOG( iUnitClass )
			<< END_LOG;
	}
	else
	{
		m_mapMemoGroupID.insert( std::make_pair( iMemo, iMemoGroup ) );
	}

	// ���� �޸� ����� ���� �� �����̳ʿ� ����
	std::map< int, std::map< int, int > >::iterator mit = m_mapClassChangeMemo.find( iMemoGroup );
	if( mit == m_mapClassChangeMemo.end() )
	{
		std::map< int, int > mapTemp;
		mapTemp.insert( std::make_pair( iUnitClass, iMemo ) );

		m_mapClassChangeMemo.insert( std::make_pair( iMemoGroup, mapTemp) );
	}
	else
	{
		std::map< int, int >::iterator mitGroup = mit->second.find( iUnitClass );
		if( mitGroup != mit->second.end() )
		{
			START_LOG( cerr, L"���� ���濡 �ʿ��� �޸���� �ߺ� �Ľ�!" )
				<< BUILD_LOG( iMemoGroup )
				<< BUILD_LOG( iMemo )
				<< BUILD_LOG( iUnitClass )
				<< END_LOG;
			return;
		}

		mit->second.insert( std::make_pair( iUnitClass, iMemo ) );
	}
}

// ���� �� ĳ�� ������
void KClassChangeTable::SetClassChange_InventoryItem_Lua( IN int iItemGroup, IN int iItem, IN int iUnitClass )
{
	if( iItemGroup < 0 || iItem <= 0 || iUnitClass <= CXSLUnit::UC_NONE || iUnitClass >= CXSLUnit::UC_MAX )
	{
		START_LOG( cerr, L"���� ���濡 �ʿ��� �޸���� ��ũ��Ʈ �Ľ� ����!" )
			<< BUILD_LOG( iItemGroup )
			<< BUILD_LOG( iItem )
			<< BUILD_LOG( iUnitClass )
			<< END_LOG;
		return;
	}

	// ��ų�� �׷�id ����
	std::map< int, int >::iterator mitItemGroupID = m_mapItemGroupID.find( iItem );
	if( mitItemGroupID != m_mapItemGroupID.end() )
	{
		START_LOG( clog, L"������ �� �׷� ���̵� �ߺ� �Ľ�!!!" )
			<< BUILD_LOG( iItemGroup )
			<< BUILD_LOG( iItem )
			<< BUILD_LOG( iUnitClass )
			<< END_LOG;
	}
	else
	{
		m_mapItemGroupID.insert( std::make_pair( iItem, iItemGroup ) );
	}

	// ���� �޸� ����� ���� �� �����̳ʿ� ����
	std::map< int, std::map< int, int > >::iterator mit = m_mapClassChangeItem.find( iItemGroup );
	if( mit == m_mapClassChangeItem.end() )
	{
		std::map< int, int > mapTemp;
		mapTemp.insert( std::make_pair( iUnitClass, iItem ) );

		m_mapClassChangeItem.insert( std::make_pair( iItemGroup, mapTemp) );
	}
	else
	{
		std::map< int, int >::iterator mitGroup = mit->second.find( iUnitClass );
		if( mitGroup != mit->second.end() )
		{
			START_LOG( cerr, L"���� ���濡 �ʿ��� �޸���� �ߺ� �Ľ�!" )
				<< BUILD_LOG( iItemGroup )
				<< BUILD_LOG( iItem )
				<< BUILD_LOG( iUnitClass )
				<< END_LOG;
			return;
		}

		mit->second.insert( std::make_pair( iUnitClass, iItem ) );
	}
}

// ����Ʈ
void KClassChangeTable::SetClassChange_Quest_Lua( IN int iQuestGroup, IN int iQuest, IN int iUnitClass )
{
	if( iQuestGroup <= 0 || iQuest <= 0 || iUnitClass <= CXSLUnit::UC_NONE || iUnitClass >= CXSLUnit::UC_MAX )
	{
		START_LOG( cerr, L"���� ���濡 �ʿ��� �޸���� ��ũ��Ʈ �Ľ� ����!" )
			<< BUILD_LOG( iQuestGroup )
			<< BUILD_LOG( iQuest )
			<< BUILD_LOG( iUnitClass )
			<< END_LOG;
		return;
	}

	// ��ų�� �׷�id ����
	std::map< int, int >::iterator mitQuestGroupID = m_mapQuestGroupID.find( iQuest );
	if( mitQuestGroupID != m_mapQuestGroupID.end() )
	{
		START_LOG( clog, L"��ų �� �׷� ���̵� �ߺ� �Ľ�!!!" )
			<< BUILD_LOG( iQuestGroup )
			<< BUILD_LOG( iQuest )
			<< BUILD_LOG( iUnitClass )
			<< END_LOG;
	}
	else
	{
		m_mapQuestGroupID.insert( std::make_pair( iQuest, iQuestGroup ) );
	}

	// ���� �޸� ����� ���� �� �����̳ʿ� ����
	std::map< int, std::map< int, int > >::iterator mit = m_mapClassChangeQuest.find( iQuestGroup );
	if( mit == m_mapClassChangeQuest.end() )
	{
		std::map< int, int > mapTemp;
		mapTemp.insert( std::make_pair( iUnitClass, iQuest ) );

		m_mapClassChangeQuest.insert( std::make_pair( iQuestGroup, mapTemp) );
	}
	else
	{
		std::map< int, int >::iterator mitGroup = mit->second.find( iUnitClass );
		if( mitGroup != mit->second.end() )
		{
			START_LOG( clog, L"���� ���濡 �ʿ��� �޸���� �ߺ� �Ľ�!" )
				<< BUILD_LOG( iQuestGroup )
				<< BUILD_LOG( iQuest )
				<< BUILD_LOG( iUnitClass )
				<< END_LOG;
			return;
		}

		mit->second.insert( std::make_pair( iUnitClass, iQuest ) );
	}

	// UnitClass ���� �����͸� �߰� ����
	mit = m_mapClassChangeQuestList.find( iUnitClass );
	if( mit == m_mapClassChangeQuestList.end() )
	{
		std::map< int, int > mapTemp;
		mapTemp.insert( std::make_pair( iQuest, iQuestGroup ) );

		m_mapClassChangeQuestList.insert( std::make_pair( iUnitClass, mapTemp) );
	}
	else
	{
		std::map< int, int >::iterator mitGroup = mit->second.find( iQuest );
		if( mitGroup != mit->second.end() )
		{
			START_LOG( cerr, L"���� ���濡 �ʿ��� �޸���� �ߺ� �Ľ�!" )
				<< BUILD_LOG( iQuestGroup )
				<< BUILD_LOG( iQuest )
				<< BUILD_LOG( iUnitClass )
				<< END_LOG;
			return;
		}

		mit->second.insert( std::make_pair( iQuest, iQuestGroup ) );
	}
}

void KClassChangeTable::SetClassChange_DelQuest_Lua( IN int iQuest )
{
	if( iQuest <= 0 )
	{
		START_LOG( cerr, L"�߸��� Quest �Դϴ�." )
			<< BUILD_LOG( iQuest )
			<< END_LOG;
	}

	std::set<int>::iterator sit = m_setClassChangeDeleteQuest.find( iQuest );
	if( sit != m_setClassChangeDeleteQuest.end() )
	{
		START_LOG( cerr, L"�ߺ��� Quest �Դϴ�." )
			<< BUILD_LOG( iQuest )
			<< END_LOG;
		return;
	}

	m_setClassChangeDeleteQuest.insert( iQuest );
}

bool KClassChangeTable::GetClassGroupID( IN int& iClass, OUT int& iClassGroupID )
{
	iClassGroupID = 0;

	std::map< int, int >::iterator mit = m_mapClassGroupID.find( iClass );
	if( mit == m_mapClassGroupID.end() )
	{
		START_LOG( clog, L"�ش� iClass�� �ش��ϴ� iClassGroupID �� �������� �ʴ´�!" )
			<< BUILD_LOG( iClass )
			<< BUILD_LOG( iClassGroupID )
			<< END_LOG;
		return false;
	}

	iClassGroupID = mit->second;
	return true;
}

bool KClassChangeTable::GetSkillGroupID( IN int& iSkill, OUT int& iSkillGroupID )
{
	iSkillGroupID = 0;

	std::map< int, int >::iterator mit = m_mapSkillGroupID.find( iSkill );
	if( mit == m_mapSkillGroupID.end() )
	{
		START_LOG( clog, L"�ش� iSkill�� �ش��ϴ� iClassGroupID �� �������� �ʴ´�!" )
			<< BUILD_LOG( iSkill )
			<< BUILD_LOG( iSkillGroupID )
			<< END_LOG;
		return false;
	}

	iSkillGroupID = mit->second;
	return true;
}

bool KClassChangeTable::GetMemoGroupID( IN int& iMemo, OUT int& iMemoGroupID )
{
	iMemoGroupID = -1;

	std::map< int, int >::iterator mit = m_mapMemoGroupID.find( iMemo );
	if( mit == m_mapMemoGroupID.end() )
	{
		START_LOG( clog, L"�ش� iMemo�� �ش��ϴ� iClassGroupID �� �������� �ʴ´�!" )
			<< BUILD_LOG( iMemo )
			<< BUILD_LOG( iMemoGroupID )
			<< END_LOG;
		return false;
	}

	iMemoGroupID = mit->second;
	return true;
}

bool KClassChangeTable::GetItemGroupID( IN int& iItem, OUT int& iItemGroupID )
{
	iItemGroupID = -1;

	std::map< int, int >::iterator mit = m_mapItemGroupID.find( iItem );
	if( mit == m_mapItemGroupID.end() )
	{
		START_LOG( clog, L"�ش� iItem�� �ش��ϴ� iClassGroupID �� �������� �ʴ´�!" )
			<< BUILD_LOG( iItem )
			<< BUILD_LOG( iItemGroupID )
			<< END_LOG;
		return false;
	}

	iItemGroupID = mit->second;
	return true;
}

bool KClassChangeTable::GetQuestGroupID( IN int& iQuest, OUT int& iQuestGroupID )
{
	iQuestGroupID = 0;

	std::map< int, int >::iterator mit = m_mapQuestGroupID.find( iQuest );
	if( mit == m_mapQuestGroupID.end() )
	{
		START_LOG( clog, L"�ش� iQuest�� �ش��ϴ� iClassGroupID �� �������� �ʴ´�!" )
			<< BUILD_LOG( iQuest )
			<< BUILD_LOG( iQuestGroupID )
			<< END_LOG;
		return false;
	}

	iQuestGroupID = mit->second;
	return true;
}

bool KClassChangeTable::GetUnitClassList( IN int& iClassGroupID, OUT std::vector< int >& vecClassList )
{
	vecClassList.clear();

	std::map< int, std::vector< int > >::iterator mit = m_mapUnitClassGroupID.find( iClassGroupID );
	if( mit == m_mapUnitClassGroupID.end() )
	{
		START_LOG( clog, L"�ش� iClassGroupID�� �ش��ϴ� ClassList �� �������� �ʴ´�!" )
			<< BUILD_LOG( iClassGroupID )
			<< END_LOG;
		return false;
	}

	vecClassList = mit->second;
	return true;
}

bool KClassChangeTable::GetClassChange_NewSkill( IN int& iNewUnitClass, IN int& iOldSkill, OUT int& iNewSkill )
{
	iNewSkill = 0;

	int iOldSkillGroupID = 0;

	if( GetSkillGroupID( iOldSkill, iOldSkillGroupID ) == false )
	{
		START_LOG( cerr, L"�ش� iSkill�� �ش��ϴ� iSkillGroupID �� �������� �ʴ´�!" )
			<< BUILD_LOG( iNewUnitClass )
			<< BUILD_LOG( iOldSkill )
			<< BUILD_LOG( iNewSkill )
			<< END_LOG;
		return false;
	}

	std::map< int, std::map< int, int > >::const_iterator mit = m_mapClassChangeSkill.find( iOldSkillGroupID );
	if( mit == m_mapClassChangeSkill.end() )
	{
		START_LOG( cerr, L"�ش� iSkill�� �ش��ϴ� SkillList �� �������� �ʴ´�!" )
			<< BUILD_LOG( iNewUnitClass )
			<< BUILD_LOG( iOldSkill )
			<< BUILD_LOG( iNewSkill )
			<< END_LOG;

		return false;
	}

	std::map< int, int >::const_iterator mitList = mit->second.find( iNewUnitClass );
	if( mitList == mit->second.end() )
	{
		START_LOG( clog, L"�ش� iNewUnitClass�� �ش��ϴ� NewSkill �� �������� �ʴ´�!" )
			<< BUILD_LOG( iNewUnitClass )
			<< BUILD_LOG( iOldSkill )
			<< BUILD_LOG( iNewSkill )
			<< END_LOG;

		return false;
	}

	iNewSkill = mitList->second;

	return true;
}

bool KClassChangeTable::GetClassChange_NewMemo( IN int& iNewUnitClass, IN int& iOldMemo, OUT int& iNewMemo )
{
	iNewMemo = 0;

	int iOldMemoGroupID = 0;

	if( GetMemoGroupID( iOldMemo, iOldMemoGroupID ) == false )
	{
		START_LOG( clog, L"�ش� iClass�� �ش��ϴ� iClassGroupID �� �������� �ʴ´�!" )
			<< BUILD_LOG( iNewUnitClass )
			<< BUILD_LOG( iOldMemo )
			<< BUILD_LOG( iNewMemo )
			<< END_LOG;
		return false;
	}

	std::map< int, std::map< int, int > >::const_iterator mit = m_mapClassChangeMemo.find( iOldMemoGroupID );
	if( mit == m_mapClassChangeMemo.end() )
	{
		START_LOG( cerr, L"�ش� iMemo�� �ش��ϴ� MemoList �� �������� �ʴ´�!" )
			<< BUILD_LOG( iNewUnitClass )
			<< BUILD_LOG( iOldMemo )
			<< BUILD_LOG( iNewMemo )
			<< END_LOG;

		return false;
	}

	std::map< int, int >::const_iterator mitList = mit->second.find( iNewUnitClass );
	if( mitList == mit->second.end() )
	{
		START_LOG( clog, L"�ش� iNewUnitClass�� �ش��ϴ� NewMemo �� �������� �ʴ´�!" )
			<< BUILD_LOG( iNewUnitClass )
			<< BUILD_LOG( iOldMemo )
			<< BUILD_LOG( iNewMemo )
			<< END_LOG;

		return false;
	}

	iNewMemo = mitList->second;

	return true;
}

bool KClassChangeTable::GetClassChange_NewItem( IN int& iNewUnitClass, IN int& iOldItem, OUT int& iNewItem )
{
	iNewItem = 0;

	int iOldItemGroupID = 0;

	if( GetItemGroupID( iOldItem, iOldItemGroupID ) == false )
	{
		START_LOG( clog, L"�ش� iClass�� �ش��ϴ� iClassGroupID �� �������� �ʴ´�!" )
			<< BUILD_LOG( iNewUnitClass )
			<< BUILD_LOG( iOldItem )
			<< BUILD_LOG( iNewItem )
			<< END_LOG;
		return false;
	}

	std::map< int, std::map< int, int > >::const_iterator mit = m_mapClassChangeItem.find( iOldItemGroupID );
	if( mit == m_mapClassChangeItem.end() )
	{
		START_LOG( cerr, L"�ش� iItem�� �ش��ϴ� ItemList �� �������� �ʴ´�!" )
			<< BUILD_LOG( iNewUnitClass )
			<< BUILD_LOG( iOldItem )
			<< BUILD_LOG( iNewItem )
			<< END_LOG;

		return false;
	}

	std::map< int, int >::const_iterator mitList = mit->second.find( iNewUnitClass );
	if( mitList == mit->second.end() )
	{
		START_LOG( clog, L"�ش� iNewUnitClass�� �ش��ϴ� NewItem �� �������� �ʴ´�!" )
			<< BUILD_LOG( iNewUnitClass )
			<< BUILD_LOG( iOldItem )
			<< BUILD_LOG( iNewItem )
			<< END_LOG;

		return false;
	}

	iNewItem = mitList->second;

	return true;
}

bool KClassChangeTable::GetClassChange_NewQuest( IN int& iNewUnitClass, IN int& iOldQuest, OUT int& iNewQuest )
{
	iNewQuest = 0;

	int iOldQuestGroupID = 0;

	if( GetQuestGroupID( iOldQuest, iOldQuestGroupID ) == false )
	{
		START_LOG( clog, L"�ش� iClass�� �ش��ϴ� iClassGroupID �� �������� �ʴ´�!" )
			<< BUILD_LOG( iNewUnitClass )
			<< BUILD_LOG( iOldQuest )
			<< BUILD_LOG( iNewQuest )
			<< END_LOG;
		return false;
	}

	std::map< int, std::map< int, int > >::const_iterator mit = m_mapClassChangeQuest.find( iOldQuestGroupID );
	if( mit == m_mapClassChangeQuest.end() )
	{
		START_LOG( cerr, L"�ش� iQuest�� �ش��ϴ� QuestList �� �������� �ʴ´�!" )
			<< BUILD_LOG( iNewUnitClass )
			<< BUILD_LOG( iOldQuest )
			<< BUILD_LOG( iNewQuest )
			<< END_LOG;

		return false;
	}

	std::map< int, int >::const_iterator mitList = mit->second.find( iNewUnitClass );
	if( mitList == mit->second.end() )
	{
		START_LOG( clog, L"�ش� iNewUnitClass�� �ش��ϴ� NewQuest �� �������� �ʴ´�!" )
			<< BUILD_LOG( iNewUnitClass )
			<< BUILD_LOG( iOldQuest )
			<< BUILD_LOG( iNewQuest )
			<< END_LOG;

		return false;
	}

	iNewQuest = mitList->second;

	return true;
}

bool KClassChangeTable::GetClassQuestList( IN int& iUnitClass, OUT std::map< int, int >& mapQuestList )
{
	mapQuestList.clear();

	if( iUnitClass <= CXSLUnit::UC_NONE || iUnitClass >= CXSLUnit::UC_MAX )
	{
		START_LOG( cerr, L"�������� �ʴ� UnitClass �Դϴ�." )
			<< BUILD_LOG( iUnitClass )
			<< END_LOG;
		return false;
	}

	std::map< int, std::map< int, int > >::const_iterator mit = m_mapClassChangeQuestList.find( iUnitClass );
	if( mit == m_mapClassChangeQuestList.end() )
	{
		START_LOG( cerr, L"�����Ͱ� ����? �������� �ʴ� UnitClass �ΰ�? �ƴϸ� ��ũ��Ʈ�� �����ֳ�?" )
			<< BUILD_LOG( iUnitClass )
			<< END_LOG;
		return false;
	}

	mapQuestList = mit->second;

	return true;
}

bool KClassChangeTable::CompareUnitClass( IN int& iNowUnitClass, IN int& iNewUnitClass )
{
	int iNowGroupID = 0;
	int iNewGroupID = 0;
	if( GetClassGroupID( iNowUnitClass, iNowGroupID ) == false )
	{
		START_LOG( cerr, L"UnitGroupID �� �������� �ʴ� iNowGroupID �̴�." )
			<< BUILD_LOG( iNowUnitClass )
			<< BUILD_LOG( iNewUnitClass )
			<< BUILD_LOG( iNowGroupID )
			<< BUILD_LOG( iNewGroupID )
			<< END_LOG;

		return false;
	}
	if( GetClassGroupID( iNewUnitClass, iNewGroupID ) == false )
	{
		START_LOG( cerr, L"UnitGroupID �� �������� �ʴ� iNewGroupID �̴�." )
			<< BUILD_LOG( iNowUnitClass )
			<< BUILD_LOG( iNewUnitClass )
			<< BUILD_LOG( iNowGroupID )
			<< BUILD_LOG( iNewGroupID )
			<< END_LOG;

		return false;
	}	
	
	//////////////////////////////////////////////////////////////////////////
	
	if( iNewUnitClass == CXSLUnit::UC_ELSWORD_SHEATH_KNIGHT && 
		iNowGroupID != iNewGroupID &&
		CXSLUnit::GetUnitClassBaseGrade( static_cast<CXSLUnit::UNIT_CLASS>(iNowUnitClass) ) == CXSLUnit::UC_ELSWORD_SWORDMAN )
	{
		return true;
	}
	
	if( iNewUnitClass == CXSLUnit::UC_ARME_BATTLE_MAGICIAN && 
		iNowGroupID != iNewGroupID &&
		CXSLUnit::GetUnitClassBaseGrade( static_cast<CXSLUnit::UNIT_CLASS>(iNowUnitClass) ) == CXSLUnit::UC_ARME_VIOLET_MAGE )
	{
		return true;
	}

	if( iNewUnitClass == CXSLUnit::UC_LIRE_TRAPPING_RANGER && 
		iNowGroupID != iNewGroupID &&
		CXSLUnit::GetUnitClassBaseGrade( static_cast<CXSLUnit::UNIT_CLASS>(iNowUnitClass) ) == CXSLUnit::UC_LIRE_ELVEN_RANGER )
	{
		return true;
	}

	if( iNewUnitClass == CXSLUnit::UC_RAVEN_WEAPON_TAKER && 
		iNowGroupID != iNewGroupID &&
		CXSLUnit::GetUnitClassBaseGrade( static_cast<CXSLUnit::UNIT_CLASS>(iNowUnitClass) ) == CXSLUnit::UC_RAVEN_FIGHTER )
	{
		return true;
	}

	if( iNewUnitClass == CXSLUnit::UC_EVE_ELECTRA && 
		iNowGroupID != iNewGroupID &&
		CXSLUnit::GetUnitClassBaseGrade( static_cast<CXSLUnit::UNIT_CLASS>(iNowUnitClass) ) == CXSLUnit::UC_EVE_NASOD )
	{
		return true;
	}

	if( iNewUnitClass == CXSLUnit::UC_CHUNG_SHELLING_GUARDIAN && 
		iNowGroupID != iNewGroupID &&
		CXSLUnit::GetUnitClassBaseGrade( static_cast<CXSLUnit::UNIT_CLASS>(iNowUnitClass) ) == CXSLUnit::UC_CHUNG_IRON_CANNON )
	{
		return true;
	}
	//////////////////////////////////////////////////////////////////////////

	return (iNowGroupID == iNewGroupID);
}





#endif SERV_UNIT_CLASS_CHANGE_ITEM
//}}
