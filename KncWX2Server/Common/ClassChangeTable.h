#pragma once

#include "ServerDefine.h"
#include "RefreshSingleton.h"
#include <map>
#include <vector>
#include <set>	// Iruha : 2026-08-27 // VS2010 port: was pulled in transitively under VC7.1; needed explicitly now
//#include "ServerDefine.h"


//{{ 2011. 11. 21  ��μ�	���� ���� ������
#ifdef SERV_UNIT_CLASS_CHANGE_ITEM

class KClassChangeTable
{
	DeclareRefreshSingleton( KClassChangeTable );
	DeclareLuaScriptParser;
	DeclDumpToLogFile;
	DeclDump;
	DeclToStringW;

public:
	KClassChangeTable();
	~KClassChangeTable();

// ��ũ��Ʈ �Ľ� �Լ�
	// ������ �׷�id ���
	void SetClassGroupID_Lua( IN int iClassGroup, IN int iUnitClass );

	// ������ų ����
	void SetClassChange_Skill_Lua( IN int iSkillGroup, IN int iSkill, IN int iUnitClass );
	
	// �����޸� ����
	void SetClassChange_Memo_Lua( IN int iMemoGroup, IN int iMemo, IN int iUnitClass );
	
	// �κ��丮 �� ����, ĳ���޸� ����
	void SetClassChange_InventoryItem_Lua( IN int iItemGroup, IN int iItem, IN int iUnitClass );
	
	// ���� �� ����Ʈ ����
	void SetClassChange_Quest_Lua( IN int iQuestGroup, IN int iQuest, IN int iUnitClass );
	
	// ���� �ؾ� �� ����Ʈ ����
	void SetClassChange_DelQuest_Lua( IN int iQuest );

//////////////////////////////////////////////////////////////////////////
	bool GetClassGroupID( IN int& iClass, OUT int& iClassGroupID );
	bool GetSkillGroupID( IN int& iSkill, OUT int& iSkillGroupID );
	bool GetMemoGroupID( IN int& iMemo, OUT int& iMemoGroupID );
	bool GetItemGroupID( IN int& iItem, OUT int& iItemGroupID );
	bool GetQuestGroupID( IN int& iQuest, OUT int& iQuestGroupID );

	bool GetUnitClassList( IN int& iClassGroupID, OUT std::vector< int >& vecClassList );
	bool GetClassChange_NewSkill( IN int& iNewUnitClass, IN int& iOldSkill, OUT int& iNewSkill );
	bool GetClassChange_NewMemo( IN int& iNewUnitClass, IN int& iOldMemo, OUT int& iNewMemo );
	bool GetClassChange_NewItem( IN int& iNewUnitClass, IN int& iOldItem, OUT int& iNewItem );
	bool GetClassChange_NewQuest( IN int& iNewUnitClass, IN int& iOldQuest, OUT int& iNewQuest );

	bool GetClassQuestList( IN int& iUnitClass, OUT std::map< int, int >& mapQuestList );

	bool CompareUnitClass( IN int& iNowUnitClass, IN int& iNewUnitClass );

	void GetClassChangeDeleteQuest( OUT std::set<int>& setQuest )	{	setQuest = m_setClassChangeDeleteQuest;	}

private:
	// ClassChange ������ ������ ��Ƶ�
	std::map< int, int >								m_mapClassGroupID;			// std::map< iClass, iClassGroup >
	std::map< int, std::vector< int > >					m_mapUnitClassGroupID;		// std::map< iClassGroup, std::vector< iClass > >

	// ������ų ����
	std::map< int, int >								m_mapSkillGroupID;			// std::map< iSkill, iClassGroup >
	std::map< int, std::map< int, int > >				m_mapClassChangeSkill;		// std::map< iSkillGroup, std::map< iUnitClass, iSkill >>

	// �����޸� ����
	std::map< int, int >								m_mapMemoGroupID;			// std::map< iMemo, iClassGroup >
	std::map< int, std::map< int, int > >				m_mapClassChangeMemo;		// std::map< iMemoGroup, std::map< iUnitClass, iMemo >>

	// �κ��丮 �� item ����
	std::map< int, int >								m_mapItemGroupID;			// std::map< iItem, iClassGroup >
	std::map< int, std::map< int, int > >				m_mapClassChangeItem;		// std::map< iItemGroup, std::map< iUnitClass, iItem >>

	// ���� �� ����Ʈ ����
	std::map< int, int >								m_mapQuestGroupID;			// std::map< iQuest, iClassGroup >
	std::map< int, std::map< int, int > >				m_mapClassChangeQuest;		// std::map< iQuestGroup, std::map< iUnitClass, iQuest >>
	std::map< int, std::map< int, int > >				m_mapClassChangeQuestList;	// std::map< iUnitClass, std::map< iQuest, iQuestGroup >>

	// ���� �ؾ� �� ����Ʈ ����
	std::set< int >										m_setClassChangeDeleteQuest; // ������ �ʿ��� ����Ʈ
	
};

DefRefreshSingletonInline( KClassChangeTable );

#endif SERV_UNIT_CLASS_CHANGE_ITEM
//}}