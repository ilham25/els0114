#include <winsock2.h>
#include <iomanip>
#include <sstream>

#include <dbg/dbg.hpp>

#include "GSGameDBThread.h"
#include "GameServer.h"
#include "NetError.h"
#include "Inventory.h"      // ���� ������ ���� �� InventoryCategory �ʿ�.
#include "StatTable.h"
#include "ExpTable.h"
#include "X2Data/XSLItem.h"
#include "GSSimLayer.h"
#include "TutorialManager.h"
#include "X2Data/XSLAttribEnchantItem.h"
#include "X2Data/XSLUnit.h"
//{{ 2013. 05. 15	������	������ ����
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
	#include "X2Data/XSLSocketItem.h"
#endif SERV_NEW_ITEM_SYSTEM_2013_05
//}}

#ifdef SERV_CODE_EVENT
#include "ScriptID_Code.h"
#endif SERV_CODE_EVENT

//ImplementDBThread( KGSGameDBThread );

#define CLASS_TYPE KGSGameDBThread

IMPL_PROFILER_DUMP( KGSGameDBThread )
{
	for( unsigned int ui = 0; ui < vecDump.size(); ++ui )
	{
		unsigned int iAvg = 0;
		if( vecDump[ui].m_iQueryCount > 0 )	iAvg = vecDump[ui].m_iTotalTime / vecDump[ui].m_iQueryCount;		

		DO_QUERY_NO_PROFILE( L"exec dbo.gup_insert_querystats", L"N\'%s\', %d, %d, %d, %d, %d, %d",
			% vecDump[ui].m_wstrQuery
			% vecDump[ui].m_iMinTime
			% iAvg
			% vecDump[ui].m_iMaxTime
			% vecDump[ui].m_iOver1Sec
			% vecDump[ui].m_iQueryCount
			% vecDump[ui].m_iQueryFail
			);

		continue;

end_proc:
		START_LOG( cerr, vecDump[ui].m_wstrQuery )
			<< BUILD_LOG( vecDump[ui].m_iMinTime )
			<< BUILD_LOG( vecDump[ui].m_iMaxTime )
			<< BUILD_LOG( vecDump[ui].m_iTotalTime )
			<< BUILD_LOG( vecDump[ui].m_iQueryCount )
			<< BUILD_LOG( iAvg )
			<< BUILD_LOG( vecDump[ui].m_iOver1Sec )
			<< BUILD_LOG( vecDump[ui].m_iQueryFail )
			<< END_LOG;
	}
}

KGSGameDBThread::~KGSGameDBThread(void)
{
}

void KGSGameDBThread::ProcessEvent( const KEventPtr& spEvent_ )
{
    switch( spEvent_->m_usEventID )
    {    
        CASE( DBE_UPDATE_UNIT_INFO_REQ );
		//{{ 2012. 03. 27	�ڼ���	�Ƹ����� ���� ������ ���� ����! ( ���� ���� ǥ�� )
#ifdef SERV_EVENT_RETURN_USER_MARK
		_CASE( DBE_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ, KDBE_GET_SECOND_SECURITY_INFO_REQ_FOR_GameDB );
#else
		_CASE( DBE_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ, std::wstring );
#endif SERV_EVENT_RETURN_USER_MARK
		//}}

	   //{{ 2011. 08. 09  ��μ� (2011.08.11) Ư���� ���� ������ ������ ���Ͽ� �ű��ɸ��� ���� �� ������ ���� �̺�Ʈ
#ifdef SERV_NEW_CREATE_CHAR_EVENT
	   CASE( DBE_GAME_CREATE_UNIT_REQ );
#else
	   _CASE( DBE_GAME_CREATE_UNIT_REQ, KEGS_CREATE_UNIT_REQ );
#endif SERV_NEW_CREATE_CHAR_EVENT
	   //}}
       _CASE( DBE_GAME_DELETE_UNIT_REQ, KEGS_DELETE_UNIT_REQ );
	   //{{ 2012.02.20 ��ȿ��	ĳ���� ���� ���μ��� ���� (���� ��� �Ⱓ ����)
#ifdef SERV_UNIT_WAIT_DELETE
	   _CASE( DBE_GAME_FINAL_DELETE_UNIT_REQ, KEGS_FINAL_DELETE_UNIT_REQ );
	   _CASE( DBE_GAME_RESTORE_UNIT_REQ, KEGS_RESTORE_UNIT_REQ );
#endif SERV_UNIT_WAIT_DELETE
	   //}}

       _CASE( DBE_GAME_SELECT_UNIT_REQ, KEGS_SELECT_UNIT_REQ );
		//{{ 2009. 5. 28  ������	ä���̵�
		CASE( DBE_CHANNEL_CHANGE_GAME_SELECT_UNIT_REQ );
		//}}

	   _CASE( DBE_ADMIN_MODIFY_UNIT_LEVEL_REQ, KEGS_ADMIN_MODIFY_UNIT_LEVEL_REQ );
	   _CASE( DBE_ADMIN_CHANGE_ED_REQ, KEGS_ADMIN_CHANGE_ED_REQ );
        CASE( DBE_BUY_ED_ITEM_REQ );
        CASE( DBE_INSERT_ITEM_REQ );
		CASE( DBE_NEW_QUEST_REQ );
		CASE( DBE_GIVE_UP_QUEST_REQ );
		CASE( DBE_QUEST_COMPLETE_REQ );
		CASE( DBE_INSERT_SKILL_REQ );

		//{{ 2012. 03. 23	�ڼ���	�����ڿ� ġƮŰ ���� ����
#ifdef SERV_FIX_THE_ADMIN_CHEAT
		_CASE( DBE_ADMIN_INIT_SKILL_TREE_REQ, KDBE_INIT_SKILL_TREE_REQ );
#else
		_CASE( DBE_ADMIN_INIT_SKILL_TREE_REQ, UidType );
#endif SERV_FIX_THE_ADMIN_CHEAT
		//}}

       _CASE( DBE_SEARCH_UNIT_REQ, std::wstring );
	   _CASE( DBE_KNM_REQUEST_NEW_FRIEND_INFO_REQ, std::wstring );
	    CASE( DBE_INSERT_TRADE_ITEM_REQ );
		//{{ 2009. 2. 10  ������	���ΰŷ� ���� ����
		CASE( DBE_INSERT_TRADE_ITEM_BY_SERVER_NOT );
		//}}
       _CASE( DBE_INSERT_PURCHASED_CASH_ITEM_REQ, KDBE_INSERT_ITEM_REQ );
	    CASE( DBE_INSERT_PURCHASED_CASH_PACKAGE_REQ );
		
		CASE( DBE_OPEN_RANDOM_ITEM_REQ );
		CASE( DBE_ITEM_MANUFACTURE_REQ );

	   _CASE( DBE_CHANGE_NICK_NAME_REQ, KEGS_CHANGE_NICK_NAME_REQ );

		CASE( DBE_NEW_BLACKLIST_USER_REQ );
		CASE( DBE_DEL_BLACKLIST_USER_REQ );

		//{{ 2009. 9. 22  ������	����ĳ��		
		CASE( DBE_CHANGE_UNIT_CLASS_REQ );
		//}}

		CASE( DBE_INSERT_BUY_PERSONAL_SHOP_ITEM_REQ );

        CASE( DBE_ENCHANT_ITEM_REQ );
		//{{ 2008. 12. 21  ������	��ȭ ����
		CASE( DBE_RESTORE_ITEM_REQ );
		//}}
		CASE( DBE_RESOLVE_ITEM_REQ );
		CASE( DBE_SOCKET_ITEM_REQ );

		CASE( DBE_INIT_SKILL_TREE_REQ );
		
		CASE( DBE_RECOMMEND_USER_REQ );
		CASE( DBE_INSERT_REWARD_TO_POST_REQ );
		CASE( DBE_PREPARE_INSERT_LETTER_TO_POST_REQ );
		CASE( DBE_INSERT_LETTER_TO_POST_REQ );
		
		//{{ 2008. 9. 18  ������	������
		CASE( DBE_GET_POST_LETTER_LIST_REQ );
		CASE( DBE_READ_LETTER_NOT );
		CASE( DBE_GET_ITEM_FROM_LETTER_REQ );
		CASE( DBE_DELETE_LETTER_NOT );
		//}}		

		CASE( DBE_DEL_TUTORIAL_REQ );
		CASE( DBE_INSERT_TUTORIAL_REQ );

		CASE( DBE_RESET_SKILL_REQ );
        CASE( DBE_EXPAND_INVENTORY_SLOT_REQ );
		CASE( DBE_EXPAND_SKILL_SLOT_REQ );
		//{{ 2008. 12. 14  ������	ĳ���� ���� Ȯ��
		CASE( DBE_EXPAND_CHAR_SLOT_REQ );
		//}}
		
		CASE( DBE_GET_WISH_LIST_REQ );
		CASE( DBE_UPDATE_EVENT_TIME_REQ );
		CASE( DBE_UPDATE_INVENTORY_ITEM_POS_NOT );
		CASE( DBE_PRESENT_CASH_ITEM_CHECK_NICKNAME_REQ );

		//{{ 2008. 9. 3  ������		�Ӽ���ȭ
		CASE( DBE_ATTRIB_ENCHANT_ITEM_REQ );
		CASE( DBE_IDENTIFY_ITEM_REQ );
		//}}

		//{{ 2008. 9. 26  ������	��ü�� ��������Ʈ
		CASE( DBE_NEW_POST_BLACK_LIST_REQ );
		CASE( DBE_DEL_POST_BLACK_LIST_REQ );
		//}}

		//{{ 2008. 10. 7  ������	Ÿ��Ʋ
		CASE( DBE_INSERT_TITLE_REQ );
		//}}

		//{{ 2008. 12. 25  ������	�ο�
		CASE( DBE_ENCHANT_ATTACH_ITEM_REQ );
		//}}

		//{{ 2008. 11. 18  ������	������ ��ȯ
		CASE( DBE_ITEM_EXCHANGE_REQ );
		//}}

		//{{ 2009. 4. 8  ������		�г��� ����
		CASE( DBE_DELETE_NICK_NAME_REQ );
		//}}

		CASE( DBE_INSERT_CASH_SKILL_POINT_REQ );
		CASE( DBE_EXPIRE_CASH_SKILL_POINT_REQ );

		//{{ 2009. 8. 4  ������		���� ��ų
		CASE( DBE_UNSEAL_SKILL_REQ );
		//}}

		//{{ 2009. 5. 11  ������	�ǽð� ������
		CASE( DBE_GET_ITEM_INSERT_TO_INVENTORY_REQ );
		CASE( DBE_GET_TEMP_ITEM_REQ );
		//}}

        CASE( DBE_REQUEST_FRIEND_REQ );
        CASE( DBE_ACCEPT_FRIEND_REQ );
        CASE( DBE_DENY_FRIEND_REQ );
        CASE( DBE_BLOCK_FRIEND_REQ );
        CASE( DBE_UNBLOCK_FRIEND_REQ );
        CASE( DBE_DELETE_FRIEND_REQ );
        CASE( DBE_MOVE_FRIEND_REQ );
        CASE( DBE_MAKE_FRIEND_GROUP_REQ );
        CASE( DBE_RENAME_FRIEND_GROUP_REQ );
        CASE( DBE_DELETE_FRIEND_GROUP_REQ );
        CASE( DBE_FRIEND_MESSAGE_NOT );

		//{{ 2009. 7. 29  ������	item set cheat
		CASE( DBE_ADMIN_GET_ITEM_SET_NOT );
		//}}

		//{{ 2009. 8. 27  ������	����
		CASE( DBE_SEAL_ITEM_REQ );
		CASE( DBE_UNSEAL_ITEM_REQ );
		//}}

		//////////////////////////////////////////////////////////////////////////		
		//{{ 2009. 9. 22  ������	���
#ifdef GUILD_TEST
		
		//{{ 2012. 02. 22	�ڼ���	��� �̸� �����
#ifdef SERV_GUILD_CHANGE_NAME
#else
		CASE( DBE_CREATE_GUILD_REQ );
#endif SERV_GUILD_CHANGE_NAME
		//}}

	   _CASE( DBE_INVITE_GUILD_NICKNAME_CHECK_REQ, KELG_INVITE_GUILD_ACK );
		CASE( DBE_JOIN_GUILD_REQ );

		CASE( DBE_CHANGE_GUILD_MEMBER_GRADE_REQ );
		CASE( DBE_CHANGE_GUILD_MESSAGE_REQ );
		CASE( DBE_KICK_GUILD_MEMBER_REQ );
		CASE( DBE_CHANGE_GUILD_MEMBER_MESSAGE_REQ );

		CASE( DBE_DISBAND_GUILD_REQ );
		CASE( DBE_EXPAND_GUILD_MAX_MEMBER_REQ );

		//{{ 2009. 10. 27  ������	��巹��
		CASE( DBE_UPDATE_GUILD_EXP_REQ );	   
		//}}

#endif GUILD_TEST
		//}}
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		//{{ 2009. 11. 24  ������	��彺ų
#ifdef GUILD_SKILL_TEST

		CASE( DBE_RESET_GUILD_SKILL_REQ );
		CASE( DBE_INIT_GUILD_SKILL_REQ );

#endif GUILD_SKILL_TEST
		//}}
		//////////////////////////////////////////////////////////////////////////

		//{{ 2009. 12. 8  ������	ũ���������̺�Ʈ
		CASE( DBE_CHECK_TIME_EVENT_COMPLETE_REQ );
		//}}

#ifdef SERV_GLOBAL_BILLING
		//{{ ����� : [2010/8/19/] //	���� ��� ����
		CASE( DBE_GET_NICKNAME_BY_UNITUID_REQ );
		//}} ����� : [2010/8/19/] //	���� ��� ����
#endif // SERV_GLOBAL_BILLING

		//{{ 2010. 01. 11  ������	��õ�θ���Ʈ
		CASE( DBE_GET_RECOMMEND_USER_LIST_REQ );
		//}}

		//{{ 2010. 02. 23  ������	�� ����Ʈ �̺�Ʈ
#ifdef SERV_WEB_POINT_EVENT
		CASE( DBE_ATTENDANCE_CHECK_REQ );
	   _CASE( DBE_INCREASE_WEB_POINT_LOG_NOT, KDBE_INCREASE_WEB_POINT_ACK );	   
#endif SERV_WEB_POINT_EVENT
		//}}		
		//{{ 2010. 03. 22  ������	����� ��Ʈ
#ifdef SERV_SKILL_NOTE
		CASE( DBE_EXPAND_SKILL_NOTE_PAGE_REQ );
		CASE( DBE_REG_SKILL_NOTE_MEMO_REQ );
#endif SERV_SKILL_NOTE
		//}}
		//{{ 2010. 7. 30 ������	�� �ý���
#ifdef SERV_PET_SYSTEM
		CASE( DBE_CREATE_PET_REQ );
		CASE( DBE_SUMMON_PET_REQ );

		//{{ 2012. 02. 22	�ڼ���	�� �̸� �����
#ifdef SERV_PET_CHANGE_NAME
		CASE( DBE_CHANGE_PET_NAME_REQ );
#endif SERV_PET_CHANGE_NAME
		//}}

#endif SERV_PET_SYSTEM
		//}}

		//{{ 2010. 8. 16	������	�Ⱓ ���� ������ �̺�Ʈ
#ifdef SERV_RESET_PERIOD_EVENT
		CASE( DBE_RESET_PERIOD_ITEM_REQ );
#endif SERV_RESET_PERIOD_EVENT
		//}}
		//{{ 2011. 01. 04	������	�Ӽ� ����
#ifdef SERV_ATTRIBUTE_CHARM
		CASE( DBE_ATTRIB_ATTACH_ITEM_REQ );
#endif SERV_ATTRIBUTE_CHARM
		//}}
		//{{ 2011. 04. 14	������	�븮 ����
#ifdef SERV_PSHOP_AGENCY
		CASE( DBE_INSERT_PERIOD_PSHOP_AGENCY_REQ );
		CASE( DBE_PREPARE_REG_PSHOP_AGENCY_ITEM_REQ );
		CASE( DBE_INSERT_TO_INVENTORY_PICK_UP_FROM_PSHOP_AGENCY_REQ );
#endif SERV_PSHOP_AGENCY
		//}}
		//{{ ���� - ���� ���� ����Ʈ, ĳ�� �۾� 
#ifdef SERV_SHARING_BANK_QUEST_CASH
		CASE( DBE_SHARING_BACK_OPEN_REQ);
#endif
		//}}
#ifdef SERV_SHARING_BANK_EVENT
		CASE( DBE_SHARING_BANK_EVENT_REQ );
#endif
#ifdef	SERV_SHARING_BANK_TEST
		_CASE( DBE_GET_SHARE_BANK_REQ, KEGS_GET_SHARE_BANK_REQ );
		CASE( DBE_UPDATE_SHARE_ITEM_REQ );
		_CASE( DBE_UPDATE_SHARE_ITEM_FOR_MOVE_SLOT_REQ, KDBE_UPDATE_SHARE_ITEM_REQ );
		_CASE( DBE_UPDATE_SEAL_DATA_NOT, KDBE_SEAL_ITEM_REQ );

		CASE( DBE_CHANGE_INVENTORY_SLOT_ITEM_REQ );
#endif	SERV_SHARING_BANK_TEST

#ifdef GIANT_RESURRECTION_CASHSTONE
		CASE( DBE_REALTIME_RESURRECTION_CASHSTONE_NOT );
		CASE( DBE_RESURRECTION_CASHSTONE_NOT );
#endif //GIANT_RESURRECTION_CASHSTONE

#ifdef SERV_ADVERTISEMENT_EVENT
		CASE( DBE_INSERT_ADVERTISEMENT_EVENT_INFO_NOT );
#endif SERV_ADVERTISEMENT_EVENT

		//{{ 2011. 08. 03	������	���� ���� ���ῡ ���� ����ó��
#ifdef SERV_CLIENT_QUIT_PVP_BUG_PLAY_FIX
		CASE( DBE_QUIT_USER_PVP_RESULT_UPDATE_NOT );
#endif SERV_CLIENT_QUIT_PVP_BUG_PLAY_FIX
		//}}
		//{{ 2011. 06. 22    ��μ�    ��Ż ���� �� - NEXON ���� �ý��� ����
#ifdef SERV_NEXON_SESSION_PROTOCOL
		CASE( DBE_RETAINING_SELECT_REWARD_REQ );
#endif SERV_NEXON_SESSION_PROTOCOL
		//}} 
		//{{ 2011. 10. 14	������	������ ��� DB ������Ʈ ����
#ifdef SERV_USE_ITEM_DB_UPDATE_FIX
		CASE( DBE_USE_ITEM_IN_INVENTORY_REQ );
#endif SERV_USE_ITEM_DB_UPDATE_FIX
		//}}
		//{{ 2011. 11. 21  ��μ�	���� ���� ������
#ifdef SERV_UNIT_CLASS_CHANGE_ITEM
		CASE( DBE_BUY_UNIT_CLASS_CHANGE_REQ );
#endif SERV_UNIT_CLASS_CHANGE_ITEM
		//}}
		//{{ 2012. 04. 30	�ڼ���	������ �ֹ��� ���� �̺�Ʈ ( ������ �ߺ� üũ )
#ifdef SERV_SCROLL_OF_SAGE_CHECK_THE_LETTER_BOX
		_CASE( DBE_UPDATE_EVENT_TIME_NOT, KDBE_UPDATE_EVENT_TIME_REQ );
#endif SERV_SCROLL_OF_SAGE_CHECK_THE_LETTER_BOX
		//}}
		//{{ 2012. 05. 08	��μ�       ���� �ŷ� ���� ����
#ifdef SERV_TRADE_LOGIC_CHANGE_TRADE
		CASE( DBE_TRADE_COMPLETE_REQ );
#endif SERV_TRADE_LOGIC_CHANGE_TRADE
		//}}
		//{{ 2012. 05. 30	������	�׳�ý� �Ⱓ ġƮ
#ifdef SERV_CASH_SKILL_POINT_DATE_CHANGE
		CASE( DBE_ADMIN_CASH_SKILL_POINT_DATE_CHANGE_REQ );
#endif SERV_CASH_SKILL_POINT_DATE_CHANGE
		//}}
		//{{ 2012. 07. 25	�ڼ���	�ش� ĳ������ ��� ��ų�� �� ��� ġƮ
#ifdef SERV_ADMIN_CHEAT_GET_ALL_SKILL
		CASE( DBE_ADMIN_CHEAT_GET_ALL_SKILL_REQ );
#endif SERV_ADMIN_CHEAT_GET_ALL_SKILL
		//}}

		//{{ ���� ED �ŷ��� ED ����ȭ - ��μ�
#ifdef SERV_SEND_LETTER_BEFOR_ED_SYNC
		CASE( DBE_SYNC_ED_REQ );
#endif SERV_SEND_LETTER_BEFOR_ED_SYNC
		//}}

		//{{ 2012. 08. 14	�ڼ���	��õ���� ��ȭ �̺�Ʈ ���̵� ���� ���
#ifdef SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE
		CASE_NOPARAM( DBE_CHECK_THE_ARCHUANGEL_S_COIN_EVENT_LETTER_REQ );
#endif SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE
		//}}

		//{{ 2012. 08. 21	�ڼ���	���� ���� ����
#ifdef SERV_TRADE_LOGIC_CHANGE_LETTER
		CASE( DBE_GET_ITEM_FROM_LETTER_ARRANGE_REQ );
#endif SERV_TRADE_LOGIC_CHANGE_LETTER
		//}}

		//{{ 2012 ���� ����2 ���� ��Ī �̺�Ʈ	- ��μ�
#ifdef SERV_2012_PVP_SEASON2_EVENT
		CASE( DBE_PVP_WIN_EVENT_CHECK_REQ );
#endif SERV_2012_PVP_SEASON2_EVENT
		//}}
		//{{ 2012. 12. 14  ���� �̼� �ý��� ( �������� ����Ʈ ) - ��μ�
#ifdef SERV_ACCOUNT_MISSION_SYSTEM
		_CASE( DBE_NEW_ACCOUNT_QUEST_GAME_DB_REQ, KDBE_NEW_QUEST_REQ );
		_CASE( DBE_ACCOUNT_QUEST_COMPLETE_GAME_DB_REQ, KDBE_ACCOUNT_QUEST_COMPLETE_ACK );
#endif SERV_ACCOUNT_MISSION_SYSTEM
		//}}
		//{{ 2012. 12. 24	�ڼ���	�� ���� ���� ��� �߰�
#ifdef SERV_PET_AUTO_LOOTING
		CASE( DBE_PET_AUTO_LOOTING_NOT );
#endif SERV_PET_AUTO_LOOTING
		//}}
		//{{ 2011.05.04   �ӱԼ� �ƹ�Ÿ �ռ� �ý���
#ifdef SERV_SYNTHESIS_AVATAR
		CASE( DBE_OPEN_SYNTHESIS_ITEM_REQ );
#endif SERV_SYNTHESIS_AVATAR
		//}}

#ifdef SERV_HALLOWEEN_PUMPKIN_FAIRY_PET
		CASE( DBE_CHANGE_PET_ID_REQ );
#endif //SERV_HALLOWEEN_PUMPKIN_FAIRY_PET

		//{{ 2013. 3. 11	�ڼ���	 ���� ��ŷ �ý���
#ifdef SERV_LOCAL_RANKING_SYSTEM
		CASE( DBE_GAME_LOCAL_RANKING_USER_INFO_READ_REQ );
		CASE( DBE_GAME_LOCAL_RANKING_UNIT_INFO_READ_FOR_INQUIRY_REQ );
		CASE( DBE_GAME_LOCAL_RANKING_UNIT_INFO_READ_FOR_INCREASE_REQ );
#endif SERV_LOCAL_RANKING_SYSTEM
		//}}
		//{{ 2013. 03. 21	 ��õ�� �ý��� ���� - ��μ�
#ifdef SERV_RECOMMEND_LIST_EVENT
		CASE( DBE_RECOMMEND_USER_GET_NEXON_SN_REQ );
#endif SERV_RECOMMEND_LIST_EVENT
		//}
#ifdef SERV_ADD_WARP_BUTTON
		CASE( DBE_INSERT_WARP_VIP_REQ );
#endif // SERV_ADD_WARP_BUTTON
		//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
		_CASE( DBE_COUPLE_PROPOSE_USER_FIND_REQ, KEGS_COUPLE_PROPOSE_REQ );
		CASE( DBE_COUPLE_MAKING_SUCCESS_REQ );
		CASE( DBE_WEDDING_PROPOSE_REQ );
		_CASE( DBE_CHECK_NICK_NAME_REQ, KEGS_CHECK_NICK_NAME_REQ );
		_CASE( DBE_WEDDING_ITEM_INFO_REQ, KEGS_WEDDING_ITEM_INFO_REQ );
		CASE( DBE_WEDDING_ITEM_DELETE_REQ );
		CASE( DBE_BREAK_UP_REQ );
		CASE( DBE_ADMIN_CHANGE_COUPLE_DATE_REQ );
		CASE( DBE_CHANGE_LOVE_WORD_REQ );
		CASE( DBE_INSERT_WEDDING_REWARD_REQ );
#endif SERV_RELATIONSHIP_SYSTEM
		//}
#ifdef SERV_PERIOD_PET
		CASE( DBE_RELEASE_PET_REQ );
#endif SERV_PERIOD_PET

#ifdef	SERV_RIDING_PET_SYSTM// ���볯¥: 2013-04-21
		CASE( DBE_GET_RIDING_PET_LIST_REQ );
		CASE( DBE_CREATE_RIDING_PET_REQ );
		CASE( DBE_RELEASE_RIDING_PET_REQ );
#endif	// SERV_RIDING_PET_SYSTM

		//{{ 2013. 05. 15	������	������ ����
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
		CASE( DBE_ITEM_EVALUATE_REQ );
		CASE( DBE_RESTORE_ITEM_EVALUATE_REQ );
		CASE( DBE_ITEM_CONVERT_REQ );
#endif SERV_NEW_ITEM_SYSTEM_2013_05
		//}}

#ifdef SERV_RELATIONSHIP_SYSTEM_LAUNCHING_EVENT// �۾���¥: 2013-05-13	// �ڼ���
		CASE( DBE_SEND_LOVE_LETTER_EVENT_REQ );
#endif // SERV_RELATIONSHIP_SYSTEM_LAUNCHING_EVENT

#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013 // ���볯¥: 2013-06-27
		CASE( DBE_ADMIN_CHANGE_UNIT_CLASS_REQ );
		CASE( DBE_ADMIN_AUTO_GET_ALL_SKILL_REQ );
		CASE( DBE_ADMIN_GET_SKILL_REQ );
#endif	// SERV_UPGRADE_SKILL_SYSTEM_2013

#ifdef SERV_JUMPING_CHARACTER// �۾���¥: 2013-07-10	// �ڼ���
		CASE( DBE_JUMPING_CHARACTER_UPDATE_REQ );
#endif // SERV_JUMPING_CHARACTER
#ifdef SERV_RECRUIT_EVENT_BASE
		CASE( DBE_USE_RECRUIT_TICKET_REQ );
		CASE( DBE_REGISTER_RECRUITER_REQ );
		CASE( DBE_GET_RECRUIT_RECRUITER_LIST_REQ );
#endif SERV_RECRUIT_EVENT_BASE

#ifdef SERV_NEW_YEAR_EVENT_2014
		CASE( DBE_2013_EVENT_MISSION_COMPLETE_REQ );
		CASE( DBE_2014_EVENT_MISSION_COMPLETE_REQ );
#endif SERV_NEW_YEAR_EVENT_2014

#ifdef SERV_READY_TO_SOSUN_EVENT
		CASE( DBE_READY_TO_SOSUN_EVENT_REQ );
#endif SERV_READY_TO_SOSUN_EVENT

#ifdef SERV_GLOBAL_MISSION_MANAGER
		CASE( DBE_REGIST_GLOBAL_MISSION_CLEAR_NOT );
#endif SERV_GLOBAL_MISSION_MANAGER


    default:
        START_LOG( cerr, L"�̺�Ʈ �ڵ鷯�� ���ǵ��� �ʾ���. " << spEvent_->GetIDStr() );
    }

	//////////////////////////////////////////////////////////////////////////
#ifdef SP_PROFILER
	CALL_PROFILER_DUMP( SP_PROFILER_DUMP_TIME );
#endif SP_PROFILER
	//////////////////////////////////////////////////////////////////////////
}

//{{ 2009. 5. 28  ������	ä���̵�
//{{ 2012. 12. 10  ĳ���� ���� ��Ŷ ���� - ��μ�
#ifdef SERV_SELECT_UNIT_PACKET_DIVISION
bool KGSGameDBThread::Query_SelectUnit( IN const UidType iUserUID, IN const KEGS_SELECT_UNIT_REQ& kReq, OUT KDBE_SELECT_UNIT_ACK& kAck )
#else
bool KGSGameDBThread::Query_SelectUnit( IN const UidType iUserUID, IN const KEGS_SELECT_UNIT_REQ& kReq, OUT KEGS_SELECT_UNIT_ACK& kAck )
#endif SERV_SELECT_UNIT_PACKET_DIVISION
//}}
{
	kAck.m_kUnitInfo.Init();
	kAck.m_iOK = NetError::ERR_ODBC_01;
	std::map< UidType, KInventoryItemInfo >::iterator mit;
	//{{ 2010. 8. 2	������	�� �ý���
#ifdef SERV_PET_SYSTEM
	std::map< UidType, UidType > mapPetItemList;
#endif SERV_PET_SYSTEM
	//}}

	//{{ 2011.10.18     ��μ�    Īȣ �ߺ� ����(�Ϸ�,����) ���� ó��
#ifdef SERV_TITLE_DUPLICATE_PROCESS_REVISION
	std::vector< KTitleInfo >::iterator vitTitle;
#endif SERV_TITLE_DUPLICATE_PROCESS_REVISION
	//}}
	//{{ ����� : [2010/8/31/] //	��Ȱ�� ��� �и� �۾�
#ifdef SERV_SELECT_UNIT_NEW
	KELOG_UPDATE_STONE_NOT kResNot;
#endif	//	SERV_SELECT_UNIT_NEW
	//}} ����� : [2010/8/31/] //	��Ȱ�� ��� �и� �۾�

#ifdef SERV_ADD_WARP_BUTTON
	std::wstring wstrVipEndDate;
	std::wstring wstrVipRegDate;
#endif // SERV_ADD_WARP_BUTTON

	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
	std::vector< UidType > vecWeddingItem;
#endif SERV_RELATIONSHIP_SYSTEM
	//}

	int iOK = NetError::ERR_ODBC_01;

	// �ɸ��� �α��� ���� üũ
	//{{ 2012. 09. 26	������		���� ���� ���� ����
#ifdef SERV_POST_COPY_BUG_FIX
	DO_QUERY( L"exec dbo.gup_select_unit_new", L"%d, %d", % kReq.m_iUnitUID % iUserUID );
#else
	DO_QUERY( L"exec dbo.gup_select_unit", L"%d", % kReq.m_iUnitUID );
#endif SERV_POST_COPY_BUG_FIX
	//}}	
	if( m_kODBC.BeginFetch() )
	{
		//{{ 2011. 01. 18	������	ĳ���� ī��Ʈ ����
//#ifdef SERV_CHAR_LOG
		//{{ 2011. 03. 22	������	ĳ���� ù ���� �α�
#ifdef SERV_DAILY_CHAR_FIRST_SELECT
#ifdef SERV_SELECT_UNIT_NEW
		FETCH_DATA( iOK
			>> kAck.m_wstrUnitCreateDate
			>> kAck.m_wstrUnitLastLoginDate
			>> kResNot.m_iSupplyCnt
			>> kResNot.m_iQuantity );
#else
		FETCH_DATA( iOK
			>> kAck.m_wstrUnitCreateDate
			>> kAck.m_wstrUnitLastLoginDate );
#endif
#else
		FETCH_DATA( iOK
			>> kAck.m_wstrUnitCreateDate );
#endif SERV_DAILY_CHAR_FIRST_SELECT
		//}}		
//#else
//		FETCH_DATA( iOK );
//#endif SERV_CHAR_LOG
		//}}		
		m_kODBC.EndFetch();
	}
	else
	{
		kAck.m_iOK = NetError::ERR_SELECT_UNIT_05;

		START_LOG( cerr, L"gup_select_unit ȣ�� ����!" )
			<< BUILD_LOG( kAck.m_iOK )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;

		return false;
	}

	if( iOK != NetError::NET_OK )
	{
		//{{ 2012. 09. 26	������		���� ���� ���� ����
#ifdef SERV_POST_COPY_BUG_FIX
		switch( iOK )
		{
		case -10:	kAck.m_iOK = NetError::ERR_SELECT_UNIT_07;	break;
		default:	kAck.m_iOK = NetError::ERR_SELECT_UNIT_00;	break;
		}
#else
		kAck.m_iOK = NetError::ERR_SELECT_UNIT_00;
#endif SERV_POST_COPY_BUG_FIX
		//}}		

		START_LOG( cerr, L"gup_select_unit ȣ�� ����!" )
			<< BUILD_LOG( NetError::GetErrStr( kAck.m_iOK ) )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;

		return false;
	}

	//{{ ����� : [2010/8/31/] //	��Ȱ�� ��� �и� �۾�
#ifdef SERV_SELECT_UNIT_NEW
	// ĳ���� �α��� ���� üũ�� �Ϸ�Ǹ� Log DB�� ��Ȱ�� ���� ���� ����(����ġ���� �������� ���� ��츸 ����)
	if( kResNot.m_iQuantity < kResNot.m_iSupplyCnt )
	{
		// ���� : ���� - �� �α��� ������ ����������? �α��� ���� �������� ����
		SendToLogDB( ELOG_UPDATE_STONE_NOT, kResNot );
	}
#endif	//	SERV_SELECT_UNIT_NEW
	//}} ����� : [2010/8/31/] //	��Ȱ�� ��� �и� �۾�

	//////////////////////////////////////////////////////////////////////////	
	//{{ 2009. 10. 28  ������	�޸����� �̺�Ʈ
#ifdef SERV_COMEBACK_EVENT

	DO_QUERY( L"exec dbo.gup_select_event_unit", L"%d, %d", % iUserUID % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}

	if( iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"�޸� ���� �̺�Ʈ ���� ȣ�� ����!" )
			<< BUILD_LOG( iUserUID )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;

		// ȣ�� �����ϴ��� ĳ���� ������ �ǰ� ����!
	}

#endif SERV_COMEBACK_EVENT
	//}}
	//////////////////////////////////////////////////////////////////////////

	// ĳ���� ������ ��´�.
	DO_QUERY( L"exec dbo.gup_get_unit_info_by_unituid", L"%d", % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		//{{ 2012. 06. 11	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
		int iMapIDDummy = 0;
#endif SERV_BATTLE_FIELD_SYSTEM
		//}}
		//{{ 2011. 07. 11	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
		FETCH_DATA( kAck.m_kUnitInfo.m_nUnitUID
			>> kAck.m_kUnitInfo.m_cUnitClass
			>> kAck.m_kUnitInfo.m_iEXP
			>> kAck.m_kUnitInfo.m_ucLevel
			>> kAck.m_kUnitInfo.m_iED
			>> kAck.m_kUnitInfo.m_kStat.m_iBaseHP
			>> kAck.m_kUnitInfo.m_kStat.m_iAtkPhysic
			>> kAck.m_kUnitInfo.m_kStat.m_iAtkMagic
			>> kAck.m_kUnitInfo.m_kStat.m_iDefPhysic
			>> kAck.m_kUnitInfo.m_kStat.m_iDefMagic
			>> kAck.m_kUnitInfo.m_iSPoint
			>> kAck.m_kUnitInfo.m_wstrNickName
			>> iMapIDDummy
			>> kAck.m_kUnitInfo.m_iSpirit
			>> kAck.m_bIsSpiritUpdated
			>> kAck.m_iRecommendUnitUID
			);
#else
		FETCH_DATA( kAck.m_kUnitInfo.m_nUnitUID
			>> kAck.m_kUnitInfo.m_cUnitClass
			>> kAck.m_kUnitInfo.m_iEXP
			>> kAck.m_kUnitInfo.m_ucLevel
			>> kAck.m_kUnitInfo.m_iED
			>> kAck.m_kUnitInfo.m_iVSPoint			
			>> kAck.m_kUnitInfo.m_iVSPointMax
			>> kAck.m_kUnitInfo.m_kStat.m_iBaseHP
			>> kAck.m_kUnitInfo.m_kStat.m_iAtkPhysic
			>> kAck.m_kUnitInfo.m_kStat.m_iAtkMagic
			>> kAck.m_kUnitInfo.m_kStat.m_iDefPhysic
			>> kAck.m_kUnitInfo.m_kStat.m_iDefMagic
			>> kAck.m_kUnitInfo.m_iSPoint
			>> kAck.m_kUnitInfo.m_wstrNickName
			>> kAck.m_kUnitInfo.m_iWin				
			>> kAck.m_kUnitInfo.m_iLose
			>> kAck.m_kUnitInfo.m_nMapID
			>> kAck.m_kUnitInfo.m_iSpirit
			>> kAck.m_bIsSpiritUpdated
			>> kAck.m_iRecommendUnitUID
			);
#endif SERV_PVP_NEW_SYSTEM
		//}}

		m_kODBC.EndFetch();

		kAck.m_iOK = NetError::NET_OK;
	}
	else
	{
		kAck.m_iOK = NetError::ERR_SELECT_UNIT_00;

		START_LOG( cerr, L"gup_get_unit_info_by_unituid ȣ�� ����!" )
			<< BUILD_LOG( kAck.m_iOK )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;

		return false;
	}

#ifdef SERV_ELISIS_PREVIOUS_SIS_EVENT// �۾���¥: 2013-06-25	// �ڼ���
	if( CXSLUnit::GetUnitClassToUnitType( static_cast<CXSLUnit::UNIT_CLASS>( kAck.m_kUnitInfo.m_cUnitClass ) ) == CXSLUnit::UT_ELESIS )
	{
		kAck.m_iOK = NetError::ERR_SELECT_UNIT_01;
		return false;
	}
#endif // SERV_ELISIS_PREVIOUS_SIS_EVENT

	//////////////////////////////////////////////////////////////////////////
	//{{ 2012. 02. 02	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	// ĳ������ ������ ��ġ ���� ��������!
	DO_QUERY( L"exec dbo.P_GUnitLastPosition_GET", L"%d", % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kAck.m_kUnitInfo.m_kLastPos.m_iMapID
			>> kAck.m_kUnitInfo.m_kLastPos.m_ucLastTouchLineIndex
			>> kAck.m_kUnitInfo.m_kLastPos.m_usLastPosValue
			);
		m_kODBC.EndFetch();
	}
	else
	{
		kAck.m_iOK = NetError::ERR_SELECT_UNIT_05;

		START_LOG( cerr, L"P_GUnitLastPosition_GET ȣ�� ����!" )
			<< BUILD_LOG( NetError::GetErrStr( kAck.m_iOK ) )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;

		return false;
	}

	// ĳ������ ������ �÷��� ���°� ��������!
	// �ش� ĳ������ ������ �÷��� ���°� ����!
	DO_QUERY( L"exec dbo.P_GUnitPlayInfo_SEL", L"%d", % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		KGamePlayStatus kInfo;
		const CXSLUnit::UNIT_TYPE eUnitType = CXSLUnit::GetUnitClassToUnitType( static_cast<CXSLUnit::UNIT_CLASS>(kAck.m_kUnitInfo.m_cUnitClass) );
		switch( eUnitType )
		{
		case CXSLUnit::UT_ELSWORD:
#ifdef SERV_NEW_CHARACTER_EL
		case CXSLUnit::UT_ELESIS:
#endif // SERV_NEW_CHARACTER_EL
			kInfo.SetCharAbilType( KGamePlayStatus::CAC_WSP );
			break;

		case CXSLUnit::UT_CHUNG:
			kInfo.SetCharAbilType( KGamePlayStatus::CAC_CANNON_BALL_COUNT );
			break;

		case CXSLUnit::UT_ARA:
			kInfo.SetCharAbilType( KGamePlayStatus::CAC_FORCE_POWER );
			break;
		}

		FETCH_DATA( kInfo.m_iMaxHP
			>> kInfo.m_iCurHP
			>> kInfo.m_iMaxMP
			>> kInfo.m_iCurMP
			>> kInfo.m_iCurHyperGage
			>> kInfo.m_iCharAbilCount
			);

		m_kODBC.EndFetch();

		// ����!
		kAck.m_kGamePlayStatus.Set( kInfo );
	}
	else
	{
		kAck.m_iOK = NetError::ERR_SELECT_UNIT_05;

		START_LOG( cerr, L"P_GUnitPlayInfo_SEL ȣ�� ����!" )
			<< BUILD_LOG( kAck.m_iOK )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;
		return false;
	}

	// �� Ÿ�� ���� ���!
	if( kAck.m_kGamePlayStatus.IsEmpty() == false )
	{
		DO_QUERY( L"exec dbo.P_GCoolTime_SEL", L"%d", % kReq.m_iUnitUID );
		while( m_kODBC.Fetch() )
		{
			int iCoolTimeType = 0;
			int iSlotID = 0;
			int iCoolTime = 0;

			FETCH_DATA( iCoolTimeType
				>> iSlotID
				>> iCoolTime );

			switch( iCoolTimeType )
			{
			case KGamePlayStatus::CTT_SKILL_COOL_TIME:
				{
					kAck.m_kGamePlayStatus.AddSkillCoolTime( iSlotID, iCoolTime );
				}
				break;

			case KGamePlayStatus::CTT_QUICK_SLOT_COOL_TIME:
				{
                    kAck.m_kGamePlayStatus.AddQuickSlotCoolTime( iSlotID, iCoolTime );
				}
				break;

#ifdef	SERV_RIDING_PET_SYSTM// ���볯¥: 2013-04-21
			case KGamePlayStatus::CTT_RIDING_PET_COOL_TIME:
				{
					kAck.m_kGamePlayStatus.AddRidingPetCoolTime( iSlotID, iCoolTime );
				}
				break;
#endif	// SERV_RIDING_PET_SYSTM

			default:
				{
					START_LOG( cerr, L"���ǵ��� ���� ��Ÿ�� Ÿ���Դϴ�!" )
						<< BUILD_LOG( kReq.m_iUnitUID )
						<< BUILD_LOG( iCoolTimeType )
						<< END_LOG;
				}
				break;
			}
		}
	}
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}
	//////////////////////////////////////////////////////////////////////////	

	//{{ 2011. 07. 22	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
	// ���� ���� �޾ƿ���!
	
	//{{ 2012. 06. 25	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2
	DO_QUERY( L"exec dbo.P_GUnitPVP_Season2_GET", L"%d", % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kAck.m_kUnitInfo.m_iOfficialMatchCnt
			>> kAck.m_kUnitInfo.m_iRating
			>> kAck.m_kUnitInfo.m_iMaxRating
			>> kAck.m_kUnitInfo.m_iRPoint
			>> kAck.m_kUnitInfo.m_iAPoint
			>> kAck.m_kUnitInfo.m_bIsWinBeforeMatch
			>> kAck.m_kUnitInfo.m_cRank
			>> kAck.m_kUnitInfo.m_iWin
			>> kAck.m_kUnitInfo.m_iLose
			>> kAck.m_kUnitInfo.m_fKFactor
			>> kAck.m_kUnitInfo.m_bIsRedistributionUser
			>> kAck.m_kUnitInfo.m_iPastSeasonWin
			);
		m_kODBC.EndFetch();
	}
	else
	{
		kAck.m_iOK = NetError::ERR_SELECT_UNIT_05;

		START_LOG( cerr, L"P_GUnitPVP_Season2_GET ȣ�� ����!" )
			<< BUILD_LOG( kAck.m_iOK )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;

		return false;
	}
#else
	DO_QUERY( L"exec dbo.gup_get_unitpvp_info", L"%d", % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kAck.m_kUnitInfo.m_iOfficialMatchCnt
			>> kAck.m_kUnitInfo.m_iRating
			>> kAck.m_kUnitInfo.m_iMaxRating
			>> kAck.m_kUnitInfo.m_iRPoint
			>> kAck.m_kUnitInfo.m_iAPoint
			>> kAck.m_kUnitInfo.m_bIsWinBeforeMatch
			>> kAck.m_kUnitInfo.m_iWin
			>> kAck.m_kUnitInfo.m_iLose
			);
		m_kODBC.EndFetch();
	}
	else
	{
		kAck.m_iOK = NetError::ERR_SELECT_UNIT_05;

		START_LOG( cerr, L"gup_get_unitpvp_info ȣ�� ����!" )
			<< BUILD_LOG( kAck.m_iOK )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;

		return false;
	}
#endif SERV_2012_PVP_SEASON2
	//}}
#endif SERV_PVP_NEW_SYSTEM
	//}}

	//////////////////////////////////////////////////////////////////////////	
	//{{ 2011. 09. 23	������	�α׿��� ���� EDüũ
#ifdef SERV_LOGOUT_ED_CHECK
	// EDüũ ���� �޾ƿ���!
	DO_QUERY( L"exec dbo.gup_get_unit_lastpoint", L"%d", % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kAck.m_iRealDataED );
		m_kODBC.EndFetch();
	}
	else
	{
		kAck.m_iOK = NetError::ERR_SELECT_UNIT_05;

		START_LOG( cerr, L"gup_get_unit_lastpoint ȣ�� ����!" )
			<< BUILD_LOG( kAck.m_iOK )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;

		return false;
	}
#endif SERV_LOGOUT_ED_CHECK
	//}}
	//////////////////////////////////////////////////////////////////////////	

	// ���� Ŭ���� ������ ��´�.
	DO_QUERY( L"exec dbo.gup_get_dungeon_clear", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		KDungeonClearInfo kInfo;
		kInfo.m_bNew = false;

		FETCH_DATA( kInfo.m_iDungeonID
			>> kInfo.m_iMaxScore
			>> kInfo.m_cMaxTotalRank
			>> kInfo.m_wstrClearTime );

		// ĳ���� UID, ���� ID�� PK �̹Ƿ� �� ĳ���Ϳ� ���� ���� ID�� ��� �ٸ��� ����Ǿ� �ִ�.
		// ���� map�� insert �� �� Ű �ߺ� ���� �˻縦 �� �ʿ䰡 ����.
		kAck.m_kUnitInfo.m_mapDungeonClear.insert( std::make_pair( kInfo.m_iDungeonID, kInfo ) );
	}

	// �Ʒü� Ŭ���� ������ ��´�.	
	DO_QUERY( L"exec dbo.gup_get_tc_clear", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		KTCClearInfo kInfo;
		kInfo.m_bNew = false;

		FETCH_DATA( kInfo.m_iTCID
			>> kInfo.m_wstrClearTime );

		// ĳ���� UID, �Ʒü� ID�� PK �̹Ƿ� �� ĳ���Ϳ� ���� ���� ID�� ��� �ٸ��� ����Ǿ� �ִ�.
		// ���� map�� insert �� �� Ű �ߺ� ���� �˻縦 �� �ʿ䰡 ����.
		kAck.m_kUnitInfo.m_mapTCClear.insert( std::make_pair( kInfo.m_iTCID, kInfo ) );
	}

#ifdef SERV_LIMITED_DUNGEON_PLAY_TIMES
	DO_QUERY( L"exec dbo.P_GEventDungeonData_GET", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		KDungeonPlayInfo kInfo;
		kInfo.m_bNew = false;

		FETCH_DATA( kInfo.m_iDungeonID
			>> kInfo.m_iPlayTimes
			>> kInfo.m_iClearTimes );

		// ĳ���� UID, ���� ID�� PK �̹Ƿ� �� ĳ���Ϳ� ���� ���� ID�� ��� �ٸ��� ����Ǿ� �ִ�.
		// ���� map�� insert �� �� Ű �ߺ� ���� �˻縦 �� �ʿ䰡 ����.
		kAck.m_kUnitInfo.m_mapDungeonPlay.insert( std::make_pair( kInfo.m_iDungeonID, kInfo ) );
	}
#endif SERV_LIMITED_DUNGEON_PLAY_TIMES

	//////////////////////////////////////////////////////////////////////////	
	//{{ 2010. 9. 29	������	������ ���� �����丵
#ifdef SERV_GET_INVENTORY_REFAC
	//////////////////////////////////////////////////////////////////////////	
	if( Query_GetInventory( kReq.m_iUnitUID, kAck.m_mapInventorySlotSize, kAck.m_mapItem, kAck.m_mapPetItem ) == false )
	{
		START_LOG( cerr, L"�κ��丮 ���� ��� ����!" )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;

		kAck.m_iOK = NetError::ERR_SELECT_UNIT_05;

		START_LOG( cerr, L"Query_GetInventory() ȣ�� ����!" )
			<< BUILD_LOG( kAck.m_iOK )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;

		goto end_proc;
	}

	//////////////////////////////////////////////////////////////////////////	
#else
	//////////////////////////////////////////////////////////////////////////	
	// �κ��丮 ����� ��´�.
//	DO_QUERY( L"exec dbo.gup_get_inventory_size", L"%d", % kReq.m_iUnitUID );
//	while( m_kODBC.Fetch() )
//	{
//		int iCategoty;
//		int iNumSlot;
//
//		FETCH_DATA( iCategoty
//			>> iNumSlot );
//
//		// ĳ���� UID, ī�װ��� ID�� PK�̹Ƿ� �� ĳ���Ϳ� ���� ī�װ��� ID�� ��� �ٸ��� ����Ǿ� �ִ�.
//		// ���� map�� insert �� �� Ű �ߺ� ���� �˻縦 �� �ʿ䰡 ����.
//		kAck.m_mapInventorySlotSize.insert( std::make_pair( iCategoty, iNumSlot ) );
//	}
//
//	//////////////////////////////////////////////////////////////////////////	
//	//{{ 2010. 8. 2	������	�� �ý���
//#ifdef SERV_PET_SYSTEM
//	DO_QUERY( L"exec dbo.gup_get_item_list_pet", L"%d", % kReq.m_iUnitUID );
//	while( m_kODBC.Fetch() )
//	{
//		UidType iItemUID = 0;
//		UidType iPetUID = 0;
//
//		FETCH_DATA( iItemUID
//			>> iPetUID );
//
//		std::map< UidType, UidType >::iterator mitPet;
//		mitPet = mapPetItemList.find( iPetUID );
//		if( mitPet == mapPetItemList.end() )
//		{
//			mapPetItemList.insert( std::make_pair( iItemUID, iPetUID ) );
//		}
//		else
//		{
//			START_LOG( cerr, L"�ߺ��Ǵ� ItemUID�� �ֽ��ϴ�. ���� �� ���� ����!" )
//				<< BUILD_LOG( iItemUID )
//				<< BUILD_LOG( iPetUID )
//				<< END_LOG;
//		}
//	}
//#endif SERV_PET_SYSTEM
//	//}}
//
//	// ���� �������� ��´�.	
//	DO_QUERY( L"exec dbo.gup_get_item_list", L"%d", % kReq.m_iUnitUID );
//	while( m_kODBC.Fetch() )
//	{
//		int iEnchantLevel = 0;
//		//{{ 2011. 07. 25    ��μ�    ������ �ɼ�ID ������ ������ ����
//#ifdef SERV_ITEM_OPTION_DATA_SIZE
//		int arrSocketOption[4] = {0,0,0,0};
//#else
//		short arrSocketOption[4] = {0,0,0,0};
//#endif SERV_ITEM_OPTION_DATA_SIZE
//		//}} 
//		KInventoryItemInfo kInventoryItemInfo;
//
//		FETCH_DATA( kInventoryItemInfo.m_iItemUID
//			>> kInventoryItemInfo.m_kItemInfo.m_iItemID
//			>> kInventoryItemInfo.m_kItemInfo.m_cUsageType
//			>> kInventoryItemInfo.m_kItemInfo.m_iQuantity
//			>> kInventoryItemInfo.m_kItemInfo.m_sEndurance
//			>> kInventoryItemInfo.m_kItemInfo.m_sPeriod
//			>> kInventoryItemInfo.m_kItemInfo.m_wstrExpirationDate
//			>> iEnchantLevel
//			>> arrSocketOption[0]
//			>> arrSocketOption[1]
//			>> arrSocketOption[2]
//			>> arrSocketOption[3]
//			>> kInventoryItemInfo.m_cSlotCategory
//				>> kInventoryItemInfo.m_cSlotID );
//
//			//{{ 2008. 2. 20  ������  ��ȭ
//			kInventoryItemInfo.m_kItemInfo.m_cEnchantLevel = static_cast<char>(iEnchantLevel);
//			//}}
//
//			//{{ 2008. 3. 7  ������  ����
//			int iCheckIdx;
//			for( iCheckIdx = 3; iCheckIdx >= 0; --iCheckIdx )
//			{
//				if( arrSocketOption[iCheckIdx] != 0 )
//					break;
//			}
//
//			for( int iIdx = 0; iIdx <= iCheckIdx; ++iIdx )
//			{
//				kInventoryItemInfo.m_kItemInfo.m_vecItemSocket.push_back( arrSocketOption[iIdx] );
//			}
//			//}}
//
//			//{{ 2010. 8. 2	������	�� �ý���
//#ifdef SERV_PET_SYSTEM
//			// �� ī�װ����̸� ���� ����!
//			if( kInventoryItemInfo.m_cSlotCategory == CXSLInventory::ST_PET )
//			{
//				std::map< UidType, UidType >::const_iterator mitPet;
//				mitPet = mapPetItemList.find( kInventoryItemInfo.m_iItemUID );
//				if( mitPet == mapPetItemList.end() )
//				{
//					START_LOG( cerr, L"�� ������ �������� �ش� �������� ã�� ���ߴ�! �Ͼ�� �ȵǴ� ����!" )
//						<< BUILD_LOG( kReq.m_iUnitUID )
//						<< BUILD_LOG( kInventoryItemInfo.m_iItemUID )
//						<< END_LOG;
//					continue;
//				}
//
//				const UidType iPetUID = mitPet->second;
//
//				std::map< UidType, std::map< UidType, KInventoryItemInfo > >::iterator mitPetItem;
//				mitPetItem = kAck.m_mapPetItem.find( iPetUID );
//				if( mitPetItem == kAck.m_mapPetItem.end() )
//				{
//					std::map< UidType, KInventoryItemInfo > mapPetItem;
//					mapPetItem.insert( std::make_pair( kInventoryItemInfo.m_iItemUID, kInventoryItemInfo ) );
//					kAck.m_mapPetItem.insert( std::make_pair( iPetUID, mapPetItem ) );
//				}
//				else
//				{
//					mitPetItem->second.insert( std::make_pair( kInventoryItemInfo.m_iItemUID, kInventoryItemInfo ) );
//				}
//			}
//			// �� ī�װ��� �ƴѰ͵��� �Ϲ� �κ��丮��!
//			else
//			{
//				// ������ UID�� PK�̹Ƿ� map�� insert �� �� Ű �ߺ� ���� �˻縦 �� �ʿ䰡 ����.
//				kAck.m_mapItem.insert( std::make_pair( kInventoryItemInfo.m_iItemUID, kInventoryItemInfo ) );
//
//				//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
//#ifdef SERV_RELATIONSHIP_SYSTEM
//				// ûø��, ��ȥ �����
//				if( kInventoryItemInfo.m_kItemInfo.m_iItemID == CXSLItem::SI_WEDDING_INVITATION_ITEM || kInventoryItemInfo.m_kItemInfo.m_iItemID == CXSLItem::SI_WEDDING_RESERVATION_ITEM )
//				{
//					vecWeddingItem.push_back( kInventoryItemInfo.m_iItemUID );
//				}
//#endif SERV_RELATIONSHIP_SYSTEM
//					//}
//			}
//#else 
//			// ������ UID�� PK�̹Ƿ� map�� insert �� �� Ű �ߺ� ���� �˻縦 �� �ʿ䰡 ����.
//			kAck.m_mapItem.insert( std::make_pair( kInventoryItemInfo.m_iItemUID, kInventoryItemInfo ) );
//#endif SERV_PET_SYSTEM
//			//}}
//	}
//
//	//////////////////////////////////////////////////////////////////////////
//	// �Ӽ� ��ȭ ���
//	DO_QUERY( L"exec dbo.gup_get_attribute", L"%d", % kReq.m_iUnitUID );
//	while( m_kODBC.Fetch() )
//	{
//		UidType iItemUID = 0;
//		int iAttribEnchantSlotNo = 0;
//		int iAttribEnchantID = 0;
//
//		FETCH_DATA( iItemUID
//			>> iAttribEnchantSlotNo
//			>> iAttribEnchantID );
//
//		std::map< UidType, KInventoryItemInfo >::iterator mitAtt;
//		mitAtt = kAck.m_mapItem.find( iItemUID );
//		if( mitAtt == kAck.m_mapItem.end() )
//		{
//			//{{ 2010. 8. 2	������	�� �ý���
//#ifdef SERV_PET_SYSTEM
//			bool bFindItem = false;
//			std::map< UidType, std::map< UidType, KInventoryItemInfo > >::iterator mitPetItem;
//			for( mitPetItem = kAck.m_mapPetItem.begin(); mitPetItem != kAck.m_mapPetItem.end(); ++mitPetItem )
//			{
//				mitAtt = mitPetItem->second.find( iItemUID );
//				if( mitAtt != mitPetItem->second.end() )
//				{
//					bFindItem = true;
//					break;
//				}
//			}
//
//			if( bFindItem == false )
//			{
//				START_LOG( cerr, L"������ ����Ʈ�� �޾Ҵµ� �ش� �Ӽ��� �����鼭 itemuid�� ����?" )
//					<< BUILD_LOG( iItemUID )
//					<< BUILD_LOG( iAttribEnchantSlotNo )
//					<< BUILD_LOG( iAttribEnchantID )
//					<< END_LOG;
//				continue;
//			}
//#else
//			START_LOG( cerr, L"������ ����Ʈ�� �޾Ҵµ� �ش� �Ӽ��� �����鼭 itemuid�� ����?" )
//				<< BUILD_LOG( iItemUID )
//				<< BUILD_LOG( iAttribEnchantSlotNo )
//				<< BUILD_LOG( iAttribEnchantID )
//				<< END_LOG;
//			continue;
//#endif SERV_PET_SYSTEM
//			//}}
//		}
//
//		// �Ӽ� ��ȭ
//		switch( iAttribEnchantSlotNo )
//		{
//		case CXSLAttribEnchantItem::ESI_SLOT_1:
//			mitAtt->second.m_kItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant0 = static_cast<char>( iAttribEnchantID );
//			break;
//
//		case CXSLAttribEnchantItem::ESI_SLOT_2:
//			mitAtt->second.m_kItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant1 = static_cast<char>( iAttribEnchantID );
//			break;
//
//		case CXSLAttribEnchantItem::ESI_SLOT_3:
//			mitAtt->second.m_kItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant2 = static_cast<char>( iAttribEnchantID );
//			break;
//
//		default:
//			START_LOG( cerr, L"�Ӽ� ��ȭ ���� �ѹ��� �̻��մϴ�." )
//				<< BUILD_LOG( iItemUID )
//				<< BUILD_LOG( iAttribEnchantSlotNo )
//				<< BUILD_LOG( iAttribEnchantID )
//				<< END_LOG;
//			break;
//		}
//	}
//
//	//////////////////////////////////////////////////////////////////////////
//	// �к� ���� ���
//	//{{ 2009. 8. 28  ������	�к�
//	DO_QUERY( L"exec dbo.gup_get_item_list_seal", L"%d", % kReq.m_iUnitUID );
//	while( m_kODBC.Fetch() )
//	{
//		UidType iItemUID = 0;
//		u_char ucSealData = 0;		
//
//		FETCH_DATA( iItemUID
//			>> ucSealData );
//
//		std::map< UidType, KInventoryItemInfo >::iterator mitSeal;
//		mitSeal = kAck.m_mapItem.find( iItemUID );
//		if( mitSeal == kAck.m_mapItem.end() )
//		{
//			//{{ 2010. 8. 2	������	�� �ý���
//#ifdef SERV_PET_SYSTEM
//			bool bFindItem = false;
//			std::map< UidType, std::map< UidType, KInventoryItemInfo > >::iterator mitPetItem;
//			for( mitPetItem = kAck.m_mapPetItem.begin(); mitPetItem != kAck.m_mapPetItem.end(); ++mitPetItem )
//			{
//				mitSeal = mitPetItem->second.find( iItemUID );
//				if( mitSeal != mitPetItem->second.end() )
//				{
//					bFindItem = true;
//					break;
//				}
//			}
//
//			if( bFindItem == false )
//			{
//				START_LOG( cerr, L"������ ����Ʈ�� �޾Ҵµ� �ش� �к����� �����鼭 itemuid�� ����?" )
//					<< BUILD_LOG( iItemUID )
//					<< BUILD_LOGc( ucSealData )
//					<< END_LOG;
//				continue;
//			}
//#else
//			START_LOG( cerr, L"������ ����Ʈ�� �޾Ҵµ� �ش� �к����� �����鼭 itemuid�� ����?" )
//				<< BUILD_LOG( iItemUID )
//				<< BUILD_LOGc( ucSealData )
//				<< END_LOG;
//			continue;
//#endif SERV_PET_SYSTEM
//			//}}			
//		}
//
//		// �к� ���� ������Ʈ
//		mitSeal->second.m_kItemInfo.m_ucSealData = ucSealData;
//	}
	//}}
	//////////////////////////////////////////////////////////////////////////	
#endif SERV_GET_INVENTORY_REFAC
	//////////////////////////////////////////////////////////////////////////	
	//}}	

	//////////////////////////////////////////////////////////////////////////	
	// ���� ������ ������ �����Ѵ�.
	for( mit = kAck.m_mapItem.begin(); mit != kAck.m_mapItem.end(); ++mit )
	{
		if( mit->second.m_cSlotCategory == CXSLInventory::ST_E_EQUIP )
		{
			std::map< int, KInventoryItemInfo >::iterator mit2;
			//{{ 2011. 12. 15	������	�κ��丮 ����IDũ�� �ø���
#ifdef SERV_EXPAND_SLOT_ID_DATA_SIZE
			mit2 = kAck.m_kUnitInfo.m_mapEquippedItem.find( mit->second.m_sSlotID );
#else
			mit2 = kAck.m_kUnitInfo.m_mapEquippedItem.find( mit->second.m_cSlotID );
#endif SERV_EXPAND_SLOT_ID_DATA_SIZE
			//}}
			if( mit2 != kAck.m_kUnitInfo.m_mapEquippedItem.end() )
			{
				START_LOG( cerr, L"���� ������ �ߺ���." )
					<< BUILD_LOG( mit->second.m_iItemUID )
					//{{ 2011. 12. 15	������	�κ��丮 ����IDũ�� �ø���
#ifdef SERV_EXPAND_SLOT_ID_DATA_SIZE
					<< BUILD_LOG( mit->second.m_sSlotID )
#else
					<< BUILD_LOGc( mit->second.m_cSlotID )
#endif SERV_EXPAND_SLOT_ID_DATA_SIZE
					//}}
					<< BUILD_LOG( mit->second.m_kItemInfo.m_iItemID )
					<< END_LOG;
			}
			//{{ 2011. 12. 15	������	�κ��丮 ����IDũ�� �ø���
#ifdef SERV_EXPAND_SLOT_ID_DATA_SIZE
			kAck.m_kUnitInfo.m_mapEquippedItem.insert( std::make_pair( mit->second.m_sSlotID, mit->second) );
#else
			kAck.m_kUnitInfo.m_mapEquippedItem.insert( std::make_pair( mit->second.m_cSlotID, mit->second) );
#endif SERV_EXPAND_SLOT_ID_DATA_SIZE
			//}}
		}
	}

	//////////////////////////////////////////////////////////////////////////	
	// ��Ȱ�� �޾ƿ���.
	DO_QUERY( L"exec dbo.gup_get_resurrection_stone", L"%d", % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kAck.m_iNumResurrectionStone );

		m_kODBC.EndFetch();
	}
	else
	{
		kAck.m_iOK = NetError::ERR_SELECT_UNIT_03;

		START_LOG( cerr, L"gup_get_resurrection_stone ȣ�� ����!" )
			<< BUILD_LOG( kAck.m_iOK )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;

		return false;
	}

	//{{ 2009. 10. 14  ������	�ڵ����� ��Ȱ��
#ifdef AP_RESTONE

	DO_QUERY( L"exec dbo.gup_get_resurrection_stone_autopay", L"%d", % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kAck.m_iNumAutoPaymentResStone
			>> kAck.m_wstrAutoPayResStoneLastDate
			);

		m_kODBC.EndFetch();
	}
	else
	{
		kAck.m_iOK = NetError::ERR_SELECT_UNIT_03;

		START_LOG( cerr, L"gup_get_resurrection_stone_autopay ȣ�� ����!" )
			<< BUILD_LOG( kAck.m_iOK )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;

		return false;
	}

#endif AP_RESTONE
	//}}

#ifdef SERV_NEW_YEAR_EVENT_2014
	// ��ġ ���� ���� ����Ͽ� ������ �� �޴� ���� ������ ������ �Ӵϴ�.
	kAck.m_kUnitInfo.m_ucOldYearMissionRewardedLevel = kAck.m_kUnitInfo.m_ucLevel;

	if( kReq.m_setCodeEventScriptID.find( CEI_OLD_YEAR_EVENT_2013 ) != kReq.m_setCodeEventScriptID.end() )
	{
		DO_QUERY( L"exec dbo.P_GEventEndYear_GET", L"%d, %d", % kReq.m_iUnitUID % kAck.m_kUnitInfo.m_ucLevel );
		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kAck.m_kUnitInfo.m_ucOldYearMissionRewardedLevel );
			m_kODBC.EndFetch();
		}
	}
	if( kReq.m_setCodeEventScriptID.find( CEI_NEW_YEAR_EVENT_2014 ) != kReq.m_setCodeEventScriptID.end() )
	{
		UidType iNewYearEventUnitUID = 0;
		DO_QUERY( L"exec dbo.P_GEventNewYear_GET", L"%d, %d", % iUserUID % kReq.m_iUnitUID );
		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iNewYearEventUnitUID
				>> kAck.m_kUnitInfo.m_iNewYearMissionStepID
				);
			m_kODBC.EndFetch();
		}
		// ������ �̼��� ������ �ٸ� ĳ���Ͱ� ������ -1�� ����
		if( iNewYearEventUnitUID != 0 && iNewYearEventUnitUID != kReq.m_iUnitUID )
			kAck.m_kUnitInfo.m_iNewYearMissionStepID = -1;
	}
#endif SERV_NEW_YEAR_EVENT_2014

	//////////////////////////////////////////////////////////////////////////
	//QUEST ��� �޾ƿ���

	//������ ����Ʈ
	DO_QUERY( L"exec dbo.gup_get_quest_list", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		KQuestInstance		kQuest;
		KSubQuestInstance	kSub[5];
		kQuest.m_OwnorUnitUID = kReq.m_iUnitUID;

		FETCH_DATA( kQuest.m_iID
			>> kSub[0].m_ucClearData
			>> kSub[1].m_ucClearData
			>> kSub[2].m_ucClearData
			>> kSub[3].m_ucClearData
			>> kSub[4].m_ucClearData
			);
		for( int i = 0; i < 5; i++ )
			kQuest.m_vecSubQuestInstance.push_back( kSub[i] );

		kAck.m_vecQuest.push_back( kQuest );
	}

	//{{ 2010. 04. 02  ������	��������Ʈ
#ifdef SERV_DAILY_QUEST

	//�Ϸ� ����Ʈ(�ݺ� X)
	{
		CTime tCompleteDate;
		std::wstring wstrCompleteDate;

		DO_QUERY( L"exec dbo.gup_get_quest_complete_list", L"%d", % kReq.m_iUnitUID );
		while( m_kODBC.Fetch() )
		{
			KCompleteQuestInfo kInfo;
			kInfo.m_iCompleteCount = 1;

			FETCH_DATA( kInfo.m_iQuestID
				>> wstrCompleteDate );

			// �Ϸ� ��¥ ��ȯ
			LIF( KncUtil::ConvertStringToCTime( wstrCompleteDate, tCompleteDate ) );
			kInfo.m_tCompleteDate = tCompleteDate.GetTime();

			kAck.m_vecCompletQuest.push_back( kInfo );
		}

		//�Ϸ� ����Ʈ(�ݺ� O)
		DO_QUERY( L"exec dbo.gup_get_quest_Repeat_complete_list", L"%d", % kReq.m_iUnitUID );
		while( m_kODBC.Fetch() )
		{
			KCompleteQuestInfo kInfo;
			kInfo.m_iCompleteCount = 1;

			FETCH_DATA( kInfo.m_iQuestID
				>> wstrCompleteDate );

			// �Ϸ� ��¥ ��ȯ
			LIF( KncUtil::ConvertStringToCTime( wstrCompleteDate, tCompleteDate ) );
			kInfo.m_tCompleteDate = tCompleteDate.GetTime();

			kAck.m_vecCompletQuest.push_back( kInfo );
		}
	}

#else

	//�Ϸ� ����Ʈ(�ݺ� X)
	DO_QUERY( L"exec dbo.gup_get_quest_complete_list", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		int	iQuestID = 0;

		FETCH_DATA( iQuestID );

		kAck.m_vecCompletQuest.push_back( iQuestID );
	}

	//�Ϸ� ����Ʈ(�ݺ� O)
	DO_QUERY( L"exec dbo.gup_get_quest_Repeat_complete_list", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		int	iQuestID = 0;

		FETCH_DATA( iQuestID );

		kAck.m_vecCompletQuest.push_back( iQuestID );
	}

#endif SERV_DAILY_QUEST
	//}}

	//////////////////////////////////////////////////////////////////////////
	//TITLE MISSION ��� �޾ƿ���

	// �������� Ÿ��Ʋ
	bool bResetTitle = false;
	bool bIsEquippedTitle = false;
	
	DO_QUERY( L"exec dbo.gup_get_title_list", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		KTitleInfo kTitle;

		FETCH_DATA( kTitle.m_iTitleID
			>> kTitle.m_wstrEndDate
			>> bIsEquippedTitle );

		kAck.m_vecTitle.push_back( kTitle );

		if( bIsEquippedTitle )
		{
			//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
#ifdef SERV_TITLE_DATA_SIZE
			if( kAck.m_kUnitInfo.m_iTitleID != 0 )
#else
			if( kAck.m_kUnitInfo.m_sTitleID != 0 )
#endif SERV_TITLE_DATA_SIZE
			//}}			
			{
				START_LOG( cerr, L"�������� Īȣ�� �� �ֳ�?" )
					<< BUILD_LOG( kReq.m_iUnitUID )
					//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
#ifdef SERV_TITLE_DATA_SIZE
					<< BUILD_LOG( kAck.m_kUnitInfo.m_iTitleID )
#else
					<< BUILD_LOG( kAck.m_kUnitInfo.m_sTitleID )
#endif SERV_TITLE_DATA_SIZE
					//}}
					<< BUILD_LOG( kTitle.m_iTitleID )
					<< END_LOG;

				bResetTitle = true;
				continue;
			}

			//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
#ifdef SERV_TITLE_DATA_SIZE
			kAck.m_kUnitInfo.m_iTitleID = kTitle.m_iTitleID;
#else
			kAck.m_kUnitInfo.m_sTitleID = static_cast<short>(kTitle.m_iTitleID);
#endif SERV_TITLE_DATA_SIZE
			//}}			
		}
	}

	// �ߺ� ������ Ÿ��Ʋ�� ��������	
	if( bResetTitle )
	{
		// ��� ��������
		DO_QUERY( L"exec dbo.gup_update_title_hang", L"%d, %d, %d", % kReq.m_iUnitUID % 0 % 0 );
		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"Īȣ ���� ���� ������Ʈ ����!" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< END_LOG;
		}

		//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
#ifdef SERV_TITLE_DATA_SIZE
		DO_QUERY( L"exec dbo.gup_update_title_hang", L"%d, %d, %d", % kReq.m_iUnitUID % 0 % kAck.m_kUnitInfo.m_iTitleID );
#else
		DO_QUERY( L"exec dbo.gup_update_title_hang", L"%d, %d, %d", % kReq.m_iUnitUID % 0 % kAck.m_kUnitInfo.m_sTitleID );
#endif SERV_TITLE_DATA_SIZE
		//}}		
		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"Īȣ ���� ������Ʈ ����!" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kReq.m_iUnitUID )
				//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
#ifdef SERV_TITLE_DATA_SIZE
				<< BUILD_LOG( kAck.m_kUnitInfo.m_iTitleID )
#else
				<< BUILD_LOG( kAck.m_kUnitInfo.m_sTitleID )
#endif SERV_TITLE_DATA_SIZE
				//}}				
				<< END_LOG;
		}
	}

	// ������ �̼�
	DO_QUERY( L"exec dbo.gup_get_title_mission_list", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		KMissionInstance	kMission;
		KSubMissionInstance	kSub[5];
		kMission.m_vecSubMissionInstance.reserve( 5 ); // �̸� ������ �ֱ�

		FETCH_DATA( kMission.m_iID
			>> kSub[0].m_sClearData
			>> kSub[1].m_sClearData
			>> kSub[2].m_sClearData
			>> kSub[3].m_sClearData
			>> kSub[4].m_sClearData
			);

		//{{ 2011.10.18     ��μ�    Īȣ �ߺ� ����(�Ϸ�,����) ���� ó��
#ifdef SERV_TITLE_DUPLICATE_PROCESS_REVISION
		bool bRet = false;
		KTitleInfo kTitle;
		for( vitTitle = kAck.m_vecTitle.begin() ; vitTitle != kAck.m_vecTitle.end() ; ++vitTitle )
		{
			kTitle = (*vitTitle);

			if( kTitle.m_iTitleID == kMission.m_iID )
			{
				bRet = true;
				break;
			}
		}
		if( bRet == true )
		{
			START_LOG( clog, L"�������ε� ���� �Ϸ��� Ÿ��Ʋ �̼��� �ִ�!" )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kTitle.m_iTitleID )
				<< BUILD_LOG( kMission.m_iID );

			continue;
		}
#endif SERV_TITLE_DUPLICATE_PROCESS_REVISION
		//}}
	
		for( int i = 0; i < 5; ++i )
			kMission.m_vecSubMissionInstance.push_back( kSub[i] );

		kAck.m_vecMission.push_back( kMission );
	}

	//////////////////////////////////////////////////////////////////////////
	// ĳ�ý�ų ����Ʈ ���� ���� �޾ƿ���
	DO_QUERY( L"exec dbo.gup_get_cash_skill_point_info", L"%d", % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		short iCSPoint = 0;
		short iMaxCSPoint = 0;

		FETCH_DATA( iCSPoint
			>> iMaxCSPoint
			>> kAck.m_kUnitInfo.m_wstrCSPointEndDate );

		kAck.m_kUnitInfo.m_iCSPoint		= iCSPoint;
		kAck.m_kUnitInfo.m_iMaxCSPoint	= iMaxCSPoint;

		m_kODBC.EndFetch();
	}
	else
	{
		START_LOG( cerr, L"ĳ�ý�ų����Ʈ ���� ������ ���з� �⺻���� �־���." )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;

		kAck.m_kUnitInfo.m_iCSPoint = 0;
		kAck.m_kUnitInfo.m_iMaxCSPoint = 0;
		kAck.m_kUnitInfo.m_wstrCSPointEndDate = L"2000-01-01 00:00:00";
	}

	// ���������� ��ų��� �޾ƿ���
	DO_QUERY( L"exec dbo.gup_get_unsealed_skill_info", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		short iSkillID = 0;

		FETCH_DATA( iSkillID );

		kAck.m_vecSkillUnsealed.push_back( iSkillID );
	}

	//SKILL LIST �޾ƿ���
	DO_QUERY( L"exec dbo.gup_get_skill_list_new", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		short iSkillID = 0;
		UCHAR cSkillLevel = 0;
		UCHAR cSkillCSPoint = 0;

		FETCH_DATA( iSkillID 
			>> cSkillLevel
			>> cSkillCSPoint
			);

		kAck.m_vecSkillAcquired.push_back( KUserSkillData( iSkillID, cSkillLevel, cSkillCSPoint ) );
	}

	//SKILL SLOT �޾ƿ���
	DO_QUERY( L"exec dbo.gup_get_skill_slot_new", L"%d", % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kAck.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkill[0].m_iSkillID
			>> kAck.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkill[1].m_iSkillID
			>> kAck.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkill[2].m_iSkillID
			>> kAck.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkill[3].m_iSkillID
			);

		m_kODBC.EndFetch();
	}
	else
	{
		//kPacket.m_iOK = NetError::ERR_SELECT_UNIT_02;
		//goto end_proc;
		//��ų������ �߰��Ǵ� �Ŷ� ���� ������� �ִ� ĳ�������� ������ ������ �������
		DO_QUERY( L"exec dbo.gup_insert_skill_slot_new", L"%d, %d, %d, %d, %d",
			% kReq.m_iUnitUID
			% (int)0
			% (int)0
			% (int)0
			% (int)0
			);
		
		int iOK = 0;

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK != 0 )
		{ 
			START_LOG( cerr, L"������ �ִ� ĳ������ ��ų ���� ������ ��������" )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< END_LOG;

			return false;
		}

		START_LOG( clog, L"========= ���� ĳ���� ��ų������ ���� ���� ���� ���� ===========" )
			<< BUILD_LOG( kReq.m_iUnitUID )
			;

		kAck.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkill[0].Init();
		kAck.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkill[1].Init();
		kAck.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkill[2].Init();
		kAck.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkill[3].Init();
	}

	//SKILL SLOT B �޾ƿ���
	DO_QUERY( L"exec dbo.gup_get_skill_slot2_new", L"%d", % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kAck.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkillSlotB[0].m_iSkillID
			>> kAck.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkillSlotB[1].m_iSkillID
			>> kAck.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkillSlotB[2].m_iSkillID
			>> kAck.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkillSlotB[3].m_iSkillID
			>> kAck.m_kUnitInfo.m_UnitSkillData.m_wstrSkillSlotBEndDate
			);

		m_kODBC.EndFetch();
	}
	else
	{
		START_LOG( cerr, L"��ų ���� ���� ������ ���з� �⺻ �ʱⰪ �־���." )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;

		kAck.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkillSlotB[0].Init();
		kAck.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkillSlotB[1].Init();
		kAck.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkillSlotB[2].Init();
		kAck.m_kUnitInfo.m_UnitSkillData.m_aEquippedSkillSlotB[3].Init();
		kAck.m_kUnitInfo.m_UnitSkillData.m_wstrSkillSlotBEndDate = L"2000-01-01 00:00:00";
	}

	//{{ 2010. 03. 27  ������	����� ��Ʈ
#ifdef SERV_SKILL_NOTE

	// ����� ��Ʈ �ִ� ������ �� ���
	DO_QUERY( L"exec dbo.gup_get_notecnt", L"%d", % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kAck.m_cSkillNoteMaxPageNum );
		m_kODBC.EndFetch();
	}
	else
	{
		START_LOG( clog, L"����� ��Ʈ ��ų ���� ������ �����ϴ�. ������Ʈ ������ ���µ�.." )
			<< BUILD_LOG( kReq.m_iUnitUID );

		// 0���� �ʱ�ȭ
		kAck.m_cSkillNoteMaxPageNum = 0;
	}

	// ����� ��Ʈ �޾ƿ���
	DO_QUERY( L"exec dbo.gup_get_note", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		char cPageNum = 0;
		int iMemoID = 0;

		FETCH_DATA( cPageNum
			>> iMemoID
			);

		kAck.m_mapSkillNote.insert( std::make_pair( cPageNum, iMemoID ) );
	}

#endif SERV_SKILL_NOTE
	//}}

	//COMMUNITY OPTION �޾ƿ���
	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
	DO_QUERY( L"exec dbo.P_GDenyOption_SEL", L"%d", % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kAck.m_kDenyOptions.m_cDenyFriendShip
			>> kAck.m_kDenyOptions.m_cDenyInviteGuild
			>> kAck.m_kDenyOptions.m_cDenyParty
			>> kAck.m_kDenyOptions.m_cDenyPersonalTrade
			>> kAck.m_kDenyOptions.m_cDenyRequestCouple
			);

		m_kODBC.EndFetch();

		kAck.m_iOK = NetError::NET_OK;
	}
#else
	DO_QUERY( L"exec dbo.gup_get_community_opt_unituid", L"%d", % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kAck.m_kDenyOptions.m_cDenyFriendShip
			>> kAck.m_kDenyOptions.m_cDenyInviteGuild
			>> kAck.m_kDenyOptions.m_cDenyParty
			>> kAck.m_kDenyOptions.m_cDenyPersonalTrade
			);

		m_kODBC.EndFetch();

		kAck.m_iOK = NetError::NET_OK;
	}
#endif SERV_RELATIONSHIP_SYSTEM
	//}
	else
	{
		kAck.m_iOK = NetError::ERR_COMMUNITY_OPT_02;

		START_LOG( cerr, L"gup_get_community_opt_unituid ȣ�� ����!" )
			<< BUILD_LOG( kAck.m_iOK )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;

		return false;
	}

	//{{ 2008. 1. 31  ������  
	//BLACK LIST �޾ƿ���
	DO_QUERY( L"exec dbo.gup_select_blacklist", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		KChatBlackListUnit kBlackListUnit;

		FETCH_DATA( kBlackListUnit.m_iUnitUID
			>> kBlackListUnit.m_wstrNickName
			);

		kAck.m_vecChatBlackList.push_back( kBlackListUnit );
	}
	//}}

	//080405.hoons. ��������Ʈ �޾ƿ���.
	int iTutorialFlag = -1;
	if( kAck.m_kUnitInfo.m_ucLevel <= KTutorialManager::UNIT_LEVEL_STUDENT_MAX )
		iTutorialFlag = 0;
	else if( kAck.m_kUnitInfo.m_ucLevel >= KTutorialManager::UNIT_LEVEL_TEACHER_MIN )
		iTutorialFlag = 1;

#ifndef SERV_NO_DISCIPLE
	if( iTutorialFlag != -1 ) // ���� �̰ų�, ������ ���ǿ��� ȣ���ϵ���...11~19 ���� ĳ���ʹ� ȣ�� �ʿ伺 X // ��μ� // 2013-07-05
	{
		DO_QUERY( L"exec dbo.gup_get_tutor", L"%d, %d", % iTutorialFlag % kReq.m_iUnitUID );
		while( m_kODBC.Fetch() )
		{
			KTutorialDBUnitInfo kInfo;
			FETCH_DATA( kInfo.m_kTutorialUnitInfo.m_iUnitUID
				>> kInfo.m_kTutorialUnitInfo.m_ucLevel
				>> kInfo.m_kTutorialUnitInfo.m_wstrNickName
				>> kInfo.m_wstrLastDate
				);

			kAck.m_vecTutorialDBUnitInfo.push_back( kInfo );
		}
	}
#endif

	// ���� �ð� �̺�Ʈ ���� ��� : SP�̸��� 30min���� �Ǿ������� �����δ� ���� �ð� �̺�Ʈ �����Դϴ�.
	DO_QUERY( L"exec dbo.gup_select_30min", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		KConnectTimeEventInfo kConnectTimeEvent;

		FETCH_DATA( kConnectTimeEvent.m_iEventUID
			>> kConnectTimeEvent.m_wstrEventTime
			);

		kAck.m_vecConnectTimeEvent.push_back( kConnectTimeEvent );
	}
	//}}

#ifdef SERV_TIME_EVENT_ONLY_CURRENT_USER_CHAR
	// �ű� ������
	DO_QUERY( L"exec dbo.P_GEvent_CombackUser_CHK",  L"%d, %d", % iUserUID % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kAck.m_iNewUnitE );

		//START_LOG(clog2, L"�輮��_����ĳ���� ���� �̺�Ʈ. m_bNewUnit ���� �ű� ������?? ���!")
		//	<< BUILD_LOG( iUserUID)
		//	<< BUILD_LOG( kReq.m_iUnitUID )
		//	<< BUILD_LOG( kAck.m_iNewUnitE)
		//	<< END_LOG;
		 
		m_kODBC.EndFetch();
	}
#endif SERV_TIME_EVENT_ONLY_CURRENT_USER_CHAR


#ifdef SERV_ADVERTISEMENT_EVENT
	int iEventUID = 0;

	DO_QUERY( L"exec dbo.P_GAdvertisementEvent_SEL", L"%d", % iUserUID );
	while( m_kODBC.Fetch() )
	{
		FETCH_DATA( iEventUID );

		kAck.m_vecAdvertisementEvent.push_back( iEventUID );
	}
#endif SERV_ADVERTISEMENT_EVENT

#ifdef SERV_RECRUIT_EVENT_BASE
	DO_QUERY( L"exec dbo.gup_get_recommend_newuid", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		KRecommendUserInfo kInfo;

		FETCH_DATA( kInfo.m_iUnitUID
			>> kInfo.m_ucLevel
			>> kInfo.m_cUnitClass
			>> kInfo.m_wstrNickName );

		kAck.m_vecRecruiterUnitInfo.push_back( kInfo );

		START_LOG( clog, L"��õ�� ��� ȹ�� �Ϸ�" )
			<< BUILD_LOG( kInfo.m_iUnitUID )
			<< BUILD_LOG( kInfo.m_ucLevel )
			<< BUILD_LOG( kInfo.m_cUnitClass )
			<< BUILD_LOG( kInfo.m_wstrNickName )
			<< END_LOG;
	}

	DO_QUERY( L"exec dbo.gup_get_recommend_olduid", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		KRecommendUserInfo kInfo;

		FETCH_DATA( kInfo.m_iUnitUID
			>> kInfo.m_ucLevel
			>> kInfo.m_cUnitClass
			>> kInfo.m_wstrNickName );

		kAck.m_vecRecruitUnitInfo.push_back( kInfo );

		START_LOG( clog, L"����õ�� ��� ȹ�� �Ϸ�" )
			<< BUILD_LOG( kInfo.m_iUnitUID )
			<< BUILD_LOG( kInfo.m_ucLevel )
			<< BUILD_LOG( kInfo.m_cUnitClass )
			<< BUILD_LOG( kInfo.m_wstrNickName )
			<< END_LOG;
	}
#endif SERV_RECRUIT_EVENT_BASE

#ifdef SERV_CUSTOM_CONNECT_EVENT
	DO_QUERY( L"exec dbo.P_CustomEvent_CHK", L"%d, %d", % iUserUID % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kAck.m_iCustomEventID );

		m_kODBC.EndFetch();

		START_LOG( clog, L"P_CustomEvent_CHK sp ���� ���" )
			<< BUILD_LOG( kAck.m_iCustomEventID )
			<< END_LOG;
	}
	else
	{
		START_LOG( clog, L"P_CustomEvent_CHK sp ȣ�� ����!" )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;
		kAck.m_iCustomEventID = 0;
	}
#endif SERV_CUSTOM_CONNECT_EVENT

#ifdef SERV_CHINA_SPIRIT_EVENT
	DO_QUERY( L"exec dbo.P_GEventChinaTicketSpirit_GET", L"%d", % kReq.m_iUnitUID );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA(	kAck.m_kUnitInfo.m_arrChinaSpirit[0]
			>> kAck.m_kUnitInfo.m_arrChinaSpirit[1]
			>> kAck.m_kUnitInfo.m_arrChinaSpirit[2]
			>> kAck.m_kUnitInfo.m_arrChinaSpirit[3]
			>> kAck.m_kUnitInfo.m_arrChinaSpirit[4]
			>> kAck.m_kUnitInfo.m_arrChinaSpirit[5]
			);
		m_kODBC.EndFetch();
	}
	else
	{
		kAck.m_kUnitInfo.m_arrChinaSpirit[0] = 0;
		kAck.m_kUnitInfo.m_arrChinaSpirit[1] = 0;
		kAck.m_kUnitInfo.m_arrChinaSpirit[2] = 0;
		kAck.m_kUnitInfo.m_arrChinaSpirit[3] = 0;
		kAck.m_kUnitInfo.m_arrChinaSpirit[4] = 0;
		kAck.m_kUnitInfo.m_arrChinaSpirit[5] = 0;
	}
#endif //SERV_CHINA_SPIRIT_EVENT

	//{{ 2012. 04. 11	�ڼ���	( ���� ���� ǥ�� )
#ifdef SERV_EVENT_RETURN_USER_MARK_SCRIPT
#else
		//////////////////////////////////////////////////////////////////////////	
		//{{ 2010. 06. 11  ������	�������� ���ӽð� �̺�Ʈ
	#ifdef SERV_ACC_TIME_EVENT

		DO_QUERY( L"exec dbo.gup_select_event_account_nor", L"%d", % iUserUID );
		while( m_kODBC.Fetch() )
		{
			KConnectTimeEventInfo kConnectTimeEvent;

			FETCH_DATA( kConnectTimeEvent.m_iEventUID
				>> kConnectTimeEvent.m_wstrEventTime
				);

			kAck.m_vecConnectTimeEvent.push_back( kConnectTimeEvent );
		}
	#endif SERV_ACC_TIME_EVENT
		//}}
		//////////////////////////////////////////////////////////////////////////
#endif SERV_EVENT_RETURN_USER_MARK_SCRIPT
	//}}

	//////////////////////////////////////////////////////////////////////////	
	//{{ 2009. 12. 7  ������	�����ð��̺�Ʈ
#ifdef CUMULATIVE_TIME_EVENT

	DO_QUERY( L"exec dbo.gup_get_remaintime", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		KCumulativeTimeEventInfo kCumulativeTimeEvent;

		FETCH_DATA( kCumulativeTimeEvent.m_iEventUID
			>> kCumulativeTimeEvent.m_iCumulativeTime
			);

		kAck.m_vecCumulativeTimeEvent.push_back( kCumulativeTimeEvent );
	}

	//////////////////////////////////////////////////////////////////////////	
	//{{ 2010. 06. 11  ������	�������� ���ӽð� �̺�Ʈ
#ifdef SERV_ACC_TIME_EVENT

	DO_QUERY( L"exec dbo.gup_select_event_account_acc", L"%d", % iUserUID );
	while( m_kODBC.Fetch() )
	{
		KCumulativeTimeEventInfo kCumulativeTimeEvent;

		FETCH_DATA( kCumulativeTimeEvent.m_iEventUID
			>> kCumulativeTimeEvent.m_iCumulativeTime
			);

		kAck.m_vecCumulativeTimeEvent.push_back( kCumulativeTimeEvent );
	}
#endif SERV_ACC_TIME_EVENT
	//}}
	//////////////////////////////////////////////////////////////////////////

#endif CUMULATIVE_TIME_EVENT
	//}}
	//////////////////////////////////////////////////////////////////////////	


#ifdef SERV_GLOBAL_FRIEND
	// ģ�� ��� ��� ����
	DO_QUERY( L"exec dbo.gup_friend_get_list", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		KFriendInfo kFriendInfo;
		FETCH_DATA( kFriendInfo.m_iUnitUID
			>> kFriendInfo.m_wstrNickName
			>> kFriendInfo.m_cGroupID
			>> kFriendInfo.m_cStatus
			);
		kFriendInfo.m_cPosition = KFriendInfo::FP_OFF_LINE;

		kAck.m_kMessengerInfo.m_mapFriendInfo.insert( std::make_pair( kFriendInfo.m_iUnitUID, kFriendInfo ) );
	}

	// ģ�� �׷� ������
	DO_QUERY( L"exec dbo.gup_friend_get_group_list", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		char cGroup;
		std::wstring wstrGroupName;
		FETCH_DATA( cGroup
			>> wstrGroupName
			);

		kAck.m_kMessengerInfo.m_mapGroup.insert( std::make_pair( cGroup, wstrGroupName ) );
	}

	// ģ�� �޼��� ������
	DO_QUERY( L"exec dbo.gup_friend_get_message", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		KFriendMessageInfo kFriendMessageInfo;
		FETCH_DATA( kFriendMessageInfo.m_iUnitUID
			>> kFriendMessageInfo.m_wstrNickName
			>> kFriendMessageInfo.m_cMessageType
			>> kFriendMessageInfo.m_wstrMessage
			>> kFriendMessageInfo.m_wstrRegDate
			);

		kAck.m_kMessengerInfo.m_vecFriendMessageInfo.push_back( kFriendMessageInfo );
	}
#endif //SERV_GLOBAL_FRIEND


	//{{ 2009. 7. 7  ������		��ŷ����
	DO_QUERY( L"exec dbo.gup_get_rank_myrecord", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		CTime tRegDate;
		std::wstring wstrRegDate;
		KHenirRankingInfo kInfo;

		FETCH_DATA( kInfo.m_iStageCount
			>> kInfo.m_ulPlayTime
			>> kInfo.m_ucLevel
			>> kInfo.m_cUnitClass
			>> wstrRegDate
			);

		// RegDate ��ȯ
		KncUtil::ConvertStringToCTime( wstrRegDate, tRegDate );
		kInfo.m_tRegDate = tRegDate.GetTime();
		kInfo.m_iUnitUID = kReq.m_iUnitUID;
		kInfo.m_wstrNickName = kAck.m_kUnitInfo.m_wstrNickName;		

		kAck.m_vecHenirRanking.push_back( kInfo );
	}
	//}}

	//{{ 2009. 9. 25  ������	���
#ifdef GUILD_TEST

	// ��� ���� ���
	DO_QUERY( L"exec dbo.gup_get_my_guild_info", L"%d", % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kAck.m_kUnitInfo.m_kUserGuildInfo.m_iGuildUID
			>> kAck.m_kUnitInfo.m_kUserGuildInfo.m_wstrGuildName );

		m_kODBC.EndFetch();

		kAck.m_iOK = NetError::NET_OK;
	}
	
	// ���� ��� �� ���� ����Ʈ ���� ���
	if( kAck.m_kUnitInfo.m_kUserGuildInfo.m_iGuildUID > 0 )
	{
		DO_QUERY( L"exec dbo.gup_get_guild_member_grade", L"%d", % kReq.m_iUnitUID );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kAck.m_kUnitInfo.m_kUserGuildInfo.m_ucMemberShipGrade
				>> kAck.m_kUnitInfo.m_kUserGuildInfo.m_iHonorPoint );

			m_kODBC.EndFetch();
		}

		if( kAck.m_kUnitInfo.m_kUserGuildInfo.m_ucMemberShipGrade == -1  ||  
			kAck.m_kUnitInfo.m_kUserGuildInfo.m_iHonorPoint == -1 )
		{
			START_LOG( cerr, L"���� ��� �� ���� ����Ʈ ���� ��� ����!" )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< END_LOG;

			kAck.m_kUnitInfo.m_kUserGuildInfo.m_ucMemberShipGrade = 0;
			kAck.m_kUnitInfo.m_kUserGuildInfo.m_iHonorPoint = 0;
		}
	}

#endif GUILD_TEST
	//}}

	//{{ 2010. 7. 21  ������	�� �ý���
#ifdef SERV_PET_SYSTEM

	kAck.m_iSummonedPetUID = 0;

	// �� ���� ���!
	DO_QUERY( L"exec dbo.gup_get_pet_list", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		bool bIsSummoned = false;
		KPetInfo kInfo;

#ifdef SERV_PERIOD_PET
		// SERV_PET_AUTO_LOOTING, SERV_PETID_DATA_TYPE_CHANGE ���� �� �������� �ϳ��� ����
		FETCH_DATA( kInfo.m_iPetUID
			>> kInfo.m_iPetID
			>> kInfo.m_wstrPetName
			>> kInfo.m_cEvolutionStep
			>> kInfo.m_sSatiety	
			>> kInfo.m_iIntimacy
			>> kInfo.m_sExtroversion
			>> kInfo.m_sEmotion
			>> kInfo.m_wstrRegDate
			>> bIsSummoned
			>> kInfo.m_bAutoFeed
			>> kInfo.m_wstrLastFeedDate
			>> kInfo.m_wstrLastSummonDate
			>> kInfo.m_bAutoLooting
			>> kInfo.m_wstrDestroyDate
			);
#else SERV_PERIOD_PET

		//{{ 2012. 12. 24	�ڼ���	�� ���� ���� ��� �߰�
#ifdef SERV_PET_AUTO_LOOTING
#ifdef SERV_PETID_DATA_TYPE_CHANGE //2013.07.02
		FETCH_DATA( kInfo.m_iPetUID
			>> kInfo.m_iPetID
			>> kInfo.m_wstrPetName
			>> kInfo.m_cEvolutionStep
			>> kInfo.m_sSatiety	
			>> kInfo.m_iIntimacy
			>> kInfo.m_sExtroversion
			>> kInfo.m_sEmotion
			>> kInfo.m_wstrRegDate
			>> bIsSummoned
			>> kInfo.m_bAutoFeed
			>> kInfo.m_wstrLastFeedDate
			>> kInfo.m_wstrLastSummonDate
			>> kInfo.m_bAutoLooting
			);
#else //SERV_PETID_DATA_TYPE_CHANGE
		FETCH_DATA( kInfo.m_iPetUID
			>> kInfo.m_cPetID
			>> kInfo.m_wstrPetName
			>> kInfo.m_cEvolutionStep
			>> kInfo.m_sSatiety	
			>> kInfo.m_iIntimacy
			>> kInfo.m_sExtroversion
			>> kInfo.m_sEmotion
			>> kInfo.m_wstrRegDate
			>> bIsSummoned
			>> kInfo.m_bAutoFeed
			>> kInfo.m_wstrLastFeedDate
			>> kInfo.m_wstrLastSummonDate
			>> kInfo.m_bAutoLooting
			);
#endif //SERV_PETID_DATA_TYPE_CHANGE
#else
		FETCH_DATA( kInfo.m_iPetUID
			>> kInfo.m_cPetID
			>> kInfo.m_wstrPetName
			>> kInfo.m_cEvolutionStep
			>> kInfo.m_sSatiety	
			>> kInfo.m_iIntimacy
			>> kInfo.m_sExtroversion
			>> kInfo.m_sEmotion
			>> kInfo.m_wstrRegDate
			>> bIsSummoned
			>> kInfo.m_bAutoFeed
			>> kInfo.m_wstrLastFeedDate
			>> kInfo.m_wstrLastSummonDate
			);
#endif SERV_PET_AUTO_LOOTING
		//}}

#endif SERV_PERIOD_PET

		if( bIsSummoned )
		{
			if( kAck.m_iSummonedPetUID != 0 )
			{
				START_LOG( cerr, L"��? ��ȯ�� ���� ���ֳ�!? ������ ���� ����!" )
					<< BUILD_LOG( kReq.m_iUnitUID )
					<< BUILD_LOG( kAck.m_iSummonedPetUID )
					<< BUILD_LOG( kInfo.m_iPetUID )
					<< END_LOG;
			}

#ifdef SERV_PERIOD_PET
			CTime tDestroyDate;
			if( kInfo.m_wstrDestroyDate != L"" &&
				KncUtil::ConvertStringToCTime( kInfo.m_wstrDestroyDate, tDestroyDate ) == true &&
				tDestroyDate <= CTime::GetCurrentTime() )
			{
				// ���ᰡ �� ���̶�� �������� �ʽ��ϴ�.
				kAck.m_iSummonedPetUID = 0;
			}
			else
#endif SERV_PERIOD_PET
			kAck.m_iSummonedPetUID = kInfo.m_iPetUID;
		}

		kAck.m_vecPetList.push_back( kInfo );
	}

#endif SERV_PET_SYSTEM
	//}}

	//{{ 2011. 01. 17	������	ĳ���� ī��Ʈ ����
#ifdef SERV_CHAR_LOG
	// ĳ���� ī��Ʈ ���� ���!
	DO_QUERY( L"exec dbo.gup_get_Character_Count", L"%d", % kReq.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		int iLogType = 0;
		int iCountValue = 0;

		FETCH_DATA( iLogType
			>> iCountValue
			);

		kAck.m_mapCharGameCount.insert( std::make_pair( iLogType, iCountValue ) );
	}
#endif SERV_CHAR_LOG
	//}}

	//{{ 2011. 04. 18	������	�븮 ����
#ifdef SERV_PSHOP_AGENCY
	// �븮 ���� ���� ���
	DO_QUERY( L"exec dbo.gup_get_PShop_info_UnitUID", L"%d", % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kAck.m_kPShopAgencyInfo.m_wstrAgencyExpirationDate
			>> kAck.m_kPShopAgencyInfo.m_bIsPShopOpen
            );

		m_kODBC.EndFetch();
	}
	else
	{
		kAck.m_kPShopAgencyInfo.m_wstrAgencyExpirationDate = L"2000-01-01 00:00:00";
		kAck.m_kPShopAgencyInfo.m_bIsPShopOpen = false;
	}
#endif SERV_PSHOP_AGENCY
	//}}

	//{{ 2011. 08. 12   ��μ�      ��ϸ� ���� 
#ifdef SERV_NEW_HENIR_TEST
	// ��ϸ� ���� ���� Ƚ�� ��� (SP ���ο��� 1�� Ƚ�� �ʱ�ȭ)
	{
		CTime tNow = CTime::GetCurrentTime();
		std::wstring wstrNow = tNow.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
		DO_QUERY( L"exec dbo.gup_get_henir_reward_cnt", L"%d, N\'%s\'", % kReq.m_iUnitUID % wstrNow );
		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK
				>> kAck.m_PacketHenirRewardCount.m_iNormal
				>> kAck.m_PacketHenirRewardCount.m_iPremium
				>> kAck.m_PacketHenirRewardCount.m_iEvent
				);

			m_kODBC.EndFetch();
		}
	}

	switch( iOK )
	{
	case NetError::NET_OK:
		{
			START_LOG( clog, L"��ϸ� ���� Ƚ�� ��� ����" )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kAck.m_PacketHenirRewardCount.m_iNormal )
				<< BUILD_LOG( kAck.m_PacketHenirRewardCount.m_iPremium )
				<< BUILD_LOG( kAck.m_PacketHenirRewardCount.m_iEvent );
		}break;
	case -1:
		{
			START_LOG( cerr, L"��ϸ� ���� Ƚ�� ��� ���� - GUnit�� ���� ���� or ������ ����" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kAck.m_PacketHenirRewardCount.m_iNormal )
				<< BUILD_LOG( kAck.m_PacketHenirRewardCount.m_iPremium )
				<< BUILD_LOG( kAck.m_PacketHenirRewardCount.m_iEvent );
			kAck.m_iOK = NetError::ERR_SELECT_UNIT_05;
			return false;
		}break;
	case -2:
		{
			START_LOG( cerr, L"��ϸ� ���� Ƚ�� �ʱ�ȭ ���� - �ʱ�ȭ ������Ʈ ����" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kAck.m_PacketHenirRewardCount.m_iNormal )
				<< BUILD_LOG( kAck.m_PacketHenirRewardCount.m_iPremium )
				<< BUILD_LOG( kAck.m_PacketHenirRewardCount.m_iEvent );
			kAck.m_iOK = NetError::ERR_SELECT_UNIT_05;
			return false;
		}break;
	case -3:
		{
			START_LOG( cerr, L"��ϸ� ���� Ƚ�� ��� ���� - ��ϸ� ���̺��� �����Ͱ� ���ų� ������" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kAck.m_PacketHenirRewardCount.m_iNormal )
				<< BUILD_LOG( kAck.m_PacketHenirRewardCount.m_iPremium )
				<< BUILD_LOG( kAck.m_PacketHenirRewardCount.m_iEvent );
			kAck.m_iOK = NetError::ERR_SELECT_UNIT_05;
			return false;
		}break;
	default:
		{
			START_LOG( cerr, L"��ϸ� ���� Ƚ�� ��� ���� - �̻��� ����?!" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kAck.m_PacketHenirRewardCount.m_iNormal )
				<< BUILD_LOG( kAck.m_PacketHenirRewardCount.m_iPremium )
				<< BUILD_LOG( kAck.m_PacketHenirRewardCount.m_iEvent );
			kAck.m_iOK = NetError::ERR_SELECT_UNIT_05;
			return false;
		}break;
	}	
#endif SERV_NEW_HENIR_TEST
	//}}

	//{{ 2012 ���� ����2 ���� ��Ī �̺�Ʈ	- ��μ�
#ifdef SERV_2012_PVP_SEASON2_EVENT
	if( kAck.m_iOK == NetError::NET_OK )
	{
		DO_QUERY( L"exec dbo.P_GEvent_Unit_GET ", L"%d", % kReq.m_iUnitUID );
		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kAck.m_iPvpEventIndex
					 >> kAck.m_wstrLastPvpEventDate );

			m_kODBC.EndFetch();
		}
	}
#endif SERV_2012_PVP_SEASON2_EVENT
	//}}


#ifdef SERV_ADD_WARP_BUTTON
	//////////////////////////////////////////////////////////////////////////
	// �ں� �ͽ������� VIP Ƽ�� ���� ����
	int iPeriod = 0;

	DO_QUERY( L"exec dbo.P_GVIPTicket_SEL", L"%d", % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( wstrVipEndDate
			>> wstrVipRegDate
			>> iPeriod );

		m_kODBC.EndFetch();

		CTime ctVipEndDate;
		if( KncUtil::ConvertStringToCTime( wstrVipEndDate, ctVipEndDate ) == true )
			kAck.m_kUnitInfo.m_trWarpVipEndData = ctVipEndDate.GetTime();
		else
			kAck.m_kUnitInfo.m_trWarpVipEndData = 0LL;

		START_LOG( clog, L"[TEST] VIP ���� �� �����Դ��� üũ" )
			<< BUILD_LOG( wstrVipEndDate.c_str() )
			<< BUILD_LOG( wstrVipRegDate.c_str() )
			<< BUILD_LOG( iPeriod )
			<< END_LOG;
	}
	else
	{
		START_LOG( clog, L"�ں� �ͽ������� VIP ���� �������� ����!! VIP ���� �� ���� ���� ������." )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;

		kAck.m_kUnitInfo.m_trWarpVipEndData = 0LL;
	}
#endif // SERV_ADD_WARP_BUTTON

	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
	if( kAck.m_iOK == NetError::NET_OK )
	{
		std::wstring wstrWeddingDate;
		std::wstring wstrLastRewardDate;

		// �ο� ���� ������ DB ���� �޾ƿ;� �Ѵ�.
		DO_QUERY( L"exec dbo.P_GCouple_Info_SEL", L"%d", % kReq.m_iUnitUID );
		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kAck.m_kRelationshipInfo.m_iOtherUnitUID
				>> kAck.m_kRelationshipInfo.m_wstrOtherNickName
				>> kAck.m_kRelationshipInfo.m_cRelationshipType
				>> wstrWeddingDate
				>> kAck.m_kRelationshipInfo.m_wstrLoveWord
				>> wstrLastRewardDate
				>> kAck.m_kRelationshipInfo.m_cRewardTitleType	);

			m_kODBC.EndFetch();
		}

		if( kAck.m_kRelationshipInfo.m_cRelationshipType == SEnum::RT_SOLO )
		{
			kAck.m_kRelationshipInfo.m_iOtherUnitUID = 0;
			kAck.m_kRelationshipInfo.m_wstrOtherNickName.clear();
		}
		else
		{
			CTime tWeddingDate;
			KncUtil::ConvertStringToCTime( wstrWeddingDate, tWeddingDate );
			kAck.m_kRelationshipInfo.m_tDate = tWeddingDate.GetTime();

			CTime tLastRewardDate;
			KncUtil::ConvertStringToCTime( wstrLastRewardDate, tLastRewardDate );
			kAck.m_kRelationshipInfo.m_tLastReward = tLastRewardDate.GetTime();
		}
	}
#endif SERV_RELATIONSHIP_SYSTEM
	//}

#ifdef SERV_GROW_UP_SOCKET
	kAck.m_kUnitInfo.m_iEventQuestClearCount = 0;
	kAck.m_kUnitInfo.m_iExchangeCount = 0;

	DO_QUERY( L"exec dbo.P_GEventTradeCount_GET", L"%d", % kReq.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kAck.m_kUnitInfo.m_iExchangeCount );

		m_kODBC.EndFetch();

		kAck.m_iOK = NetError::NET_OK;
	}
	else
	{
		kAck.m_iOK = NetError::NET_OK;

		START_LOG( clog, L"P_GEventTradeCount_GET ȣ�� ����! ���� �ϸ� �ȵǴµ�??" )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;
	}	
#endif SERV_GROW_UP_SOCKET

#ifdef SERV_GATE_OF_DARKNESS_SUPPORT_EVENT
	if( kReq.m_setCodeEventScriptID.find( CEI_GATE_OF_DARKNESS_SUPPORT_EVENT ) != kReq.m_setCodeEventScriptID.end() )
	{
		DO_QUERY( L"exec dbo.P_GEvent_GateOfDarkness_SEL", L"%d", % kReq.m_iUnitUID );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kAck.m_iGateOfDarknessSupportEventTime );

			m_kODBC.EndFetch();
		}
	}
	else
	{
		kAck.m_iGateOfDarknessSupportEventTime = 0;
	}

#endif SERV_GATE_OF_DARKNESS_SUPPORT_EVENT
	// ĳ���� ���� ����!
	kAck.m_iOK = NetError::NET_OK;
	return true;

end_proc:
	kAck.m_iOK = NetError::ERR_SELECT_UNIT_05;

	START_LOG( cerr, L"�ɸ��� ���� ������ ���� ����!" )
		<< BUILD_LOG( kAck.m_iOK )
		<< BUILD_LOG( kReq.m_iUnitUID )
		<< END_LOG;

	return false;
}
//}}

//{{ 2010. 9. 29	������	������ ���� �����丵
#ifdef SERV_GET_INVENTORY_REFAC

bool KGSGameDBThread::Query_GetInventory( IN const UidType iUnitUID, 
										  OUT std::map< int, int >& mapInventorySlotSizeResult,
										  OUT std::map< UidType, KInventoryItemInfo >& mapItemResult,
										  OUT std::map< UidType, std::map< UidType, KInventoryItemInfo > >& mapPetItemResult
										  )
{
	mapInventorySlotSizeResult.clear();
	mapItemResult.clear();
	mapPetItemResult.clear();

	typedef std::pair< int, int >	KAttribInfo;

	std::map< UidType, UidType >	mapPetItemList;	 // �� ������ ����
	std::map< UidType, u_char >		mapSealItemList; // �к� ������ ����	
	std::map< UidType, std::vector< KAttribInfo > > mapAttribItemList; // �Ӽ� ������ ����
	std::map< UidType, std::vector< KAttribInfo > >::iterator mitAttrib;
	//{{ 2013. 05. 24	������	������ ����
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
	std::map< UidType, std::vector< int > > mapRandomSocketList;
	std::map< UidType, std::vector< int > >::iterator mitRS;
#endif SERV_NEW_ITEM_SYSTEM_2013_05
	//}}

	//////////////////////////////////////////////////////////////////////////	
	// �κ��丮 ����� ��´�.
	DO_QUERY( L"exec dbo.gup_get_inventory_size", L"%d", % iUnitUID );
	while( m_kODBC.Fetch() )
	{
		int iCategoty;
		int iNumSlot;

		FETCH_DATA( iCategoty
			>> iNumSlot );

		// ĳ���� UID, ī�װ��� ID�� PK�̹Ƿ� �� ĳ���Ϳ� ���� ī�װ��� ID�� ��� �ٸ��� ����Ǿ� �ִ�.
		// ���� map�� insert �� �� Ű �ߺ� ���� �˻縦 �� �ʿ䰡 ����.
		mapInventorySlotSizeResult.insert( std::make_pair( iCategoty, iNumSlot ) );
	}

	//////////////////////////////////////////////////////////////////////////
	// �� ������ ���� ���
	DO_QUERY( L"exec dbo.gup_get_item_list_pet", L"%d", % iUnitUID );
	while( m_kODBC.Fetch() )
	{
		UidType iItemUID = 0;
		UidType iPetUID = 0;

		FETCH_DATA( iItemUID
			>> iPetUID );

		std::map< UidType, UidType >::iterator mitPet;
		//{{ 2012. 12. 17	������	iItemUID�� iPetUID�� �߸� �������� ������.
		mitPet = mapPetItemList.find( iItemUID );
		//}}
		if( mitPet == mapPetItemList.end() )
		{
			mapPetItemList.insert( std::make_pair( iItemUID, iPetUID ) );
		}
		else
		{
			START_LOG( cerr, L"�ߺ��Ǵ� ItemUID�� �ֽ��ϴ�. ���� �� ���� ����!" )
				<< BUILD_LOG( iItemUID )
				<< BUILD_LOG( iPetUID )
				<< END_LOG;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// �Ӽ� ��ȭ ���� ���
	DO_QUERY( L"exec dbo.gup_get_attribute", L"%d", % iUnitUID );
	while( m_kODBC.Fetch() )
	{
		UidType iItemUID = 0;
		int iAttribEnchantSlotNo = 0;
		int iAttribEnchantID = 0;

		FETCH_DATA( iItemUID
			>> iAttribEnchantSlotNo
			>> iAttribEnchantID );

		KAttribInfo kAttribInfo( iAttribEnchantSlotNo, iAttribEnchantID );

		mitAttrib = mapAttribItemList.find( iItemUID );
		if( mitAttrib == mapAttribItemList.end() )
		{
			std::vector< KAttribInfo > vecAttribInfo;
			vecAttribInfo.push_back( kAttribInfo );
			mapAttribItemList.insert( std::make_pair( iItemUID, vecAttribInfo ) );
		}
		else
		{
            mitAttrib->second.push_back( kAttribInfo );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// �к� ���� ���
	//{{ 2009. 8. 28  ������	�к�
	DO_QUERY( L"exec dbo.gup_get_item_list_seal", L"%d", % iUnitUID );
	while( m_kODBC.Fetch() )
	{
		UidType iItemUID = 0;
		u_char ucSealData = 0;

		FETCH_DATA( iItemUID
			>> ucSealData );

		mapSealItemList.insert( std::make_pair( iItemUID, ucSealData ) );
	}
	//}}

	//{{ 2013. 05. 24	������	������ ����
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
	// ���� ���� ���� ���
	DO_QUERY( L"exec dbo.P_GItemSocket_Random_SEL", L"%d", % iUnitUID );
	while( m_kODBC.Fetch() )
	{
		UidType iItemUID = 0;
		int arrRandomSocketOption[CXSLSocketItem::RSC_MAX] = {0,};

		FETCH_DATA( iItemUID
			>> arrRandomSocketOption[0]
			>> arrRandomSocketOption[1]
			>> arrRandomSocketOption[2]
			>> arrRandomSocketOption[3]
			>> arrRandomSocketOption[4]
			);

		// 2-1. ���� ���� ���� ������Ʈ
		//{{ Iruha : 2026-08-27 // VS2010 port: iCheckIdx is used after the loop, which VC7.1's
		// non-conformant /Zc:forScope- tolerated; VC10 requires the loop variable to outlive
		// the loop, so it's hoisted here.
		int iCheckIdx;
		for( iCheckIdx = CXSLSocketItem::RSC_MAX - 1; iCheckIdx >= 0; --iCheckIdx )
		{
			if( arrRandomSocketOption[iCheckIdx] != 0 )
				break;
		}
		//}}

		std::vector< int > vecRandomSocket;
		for( int iIdx = 0; iIdx <= iCheckIdx; ++iIdx )
		{
			vecRandomSocket.push_back( arrRandomSocketOption[iIdx] );
		}

		// itemuid���� �ߺ����� �ʴ°��� db���� �����ؾ��Ѵ�.
		mapRandomSocketList.insert( std::make_pair( iItemUID, vecRandomSocket ) );
	}
#endif SERV_NEW_ITEM_SYSTEM_2013_05
	//}}

	//////////////////////////////////////////////////////////////////////////	
	// ���� �������� ��´�.	
	//{{ 2013. 05. 21	������	������ ����
	DO_QUERY( L"exec dbo.gup_get_item_list", L"%d", % iUnitUID );
	while( m_kODBC.Fetch() )
	{
		int iEnchantLevel = 0;
		//{{ 2011. 07. 25    ��μ�    ������ �ɼ�ID ������ ������ ����
//#ifdef SERV_ITEM_OPTION_DATA_SIZE
		int arrSocketOption[4] = {0,0,0,0};
//#else
//		short arrSocketOption[4] = {0,0,0,0};
//#endif SERV_ITEM_OPTION_DATA_SIZE
		//}} 
		
		KInventoryItemInfo kInventoryItemInfo;

			//{{ 2011. 12. 15	������	�κ��丮 ����IDũ�� �ø���
//#ifdef SERV_EXPAND_SLOT_ID_DATA_SIZE
		//{{ 2013. 05. 21	������	������ ����
		FETCH_DATA( kInventoryItemInfo.m_iItemUID
			>> kInventoryItemInfo.m_kItemInfo.m_iItemID
			>> kInventoryItemInfo.m_kItemInfo.m_cUsageType
			>> kInventoryItemInfo.m_kItemInfo.m_iQuantity
			>> kInventoryItemInfo.m_kItemInfo.m_sEndurance
			>> kInventoryItemInfo.m_kItemInfo.m_sPeriod
			>> kInventoryItemInfo.m_kItemInfo.m_wstrExpirationDate
			>> iEnchantLevel
			>> arrSocketOption[0]
			>> arrSocketOption[1]
			>> arrSocketOption[2]
			>> arrSocketOption[3]
			>> kInventoryItemInfo.m_cSlotCategory
			>> kInventoryItemInfo.m_sSlotID );
//#else
//		FETCH_DATA( kInventoryItemInfo.m_iItemUID
//			>> kInventoryItemInfo.m_kItemInfo.m_iItemID
//			>> kInventoryItemInfo.m_kItemInfo.m_cUsageType
//			>> kInventoryItemInfo.m_kItemInfo.m_iQuantity
//			>> kInventoryItemInfo.m_kItemInfo.m_sEndurance
//			>> kInventoryItemInfo.m_kItemInfo.m_sPeriod
//			>> kInventoryItemInfo.m_kItemInfo.m_wstrExpirationDate
//			>> iEnchantLevel
//			>> arrSocketOption[0]
//			>> arrSocketOption[1]
//			>> arrSocketOption[2]
//			>> arrSocketOption[3]
//			>> kInventoryItemInfo.m_cSlotCategory
//			>> kInventoryItemInfo.m_cSlotID );
//#endif SERV_EXPAND_SLOT_ID_DATA_SIZE
			//}}

		// 1. ��ȭ ���� ������Ʈ
		kInventoryItemInfo.m_kItemInfo.m_cEnchantLevel = static_cast<char>(iEnchantLevel);

		// 2. ���� ���� ������Ʈ
		//{{ Iruha : 2026-08-27 // VS2010 port: iCheckIdx is used after the loop, which VC7.1's
		// non-conformant /Zc:forScope- tolerated; VC10 requires the loop variable to outlive
		// the loop, so it's hoisted here.
		int iCheckIdx;
		for( iCheckIdx = 3; iCheckIdx >= 0; --iCheckIdx )
		{
			if( arrSocketOption[iCheckIdx] != 0 )
				break;
		}
		//}}

		for( int iIdx = 0; iIdx <= iCheckIdx; ++iIdx )
		{
			kInventoryItemInfo.m_kItemInfo.m_vecItemSocket.push_back( arrSocketOption[iIdx] );
		}

		//{{ 2013. 05. 21	������	������ ����
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
		// 2-1. ���� ���� ���� ������Ʈ
		std::map< UidType, std::vector< int > >::const_iterator mitCRS;
		mitCRS = mapRandomSocketList.find( kInventoryItemInfo.m_iItemUID );
		if( mitCRS != mapRandomSocketList.end() )
		{
			kInventoryItemInfo.m_kItemInfo.m_vecRandomSocket = mitCRS->second;
		}
#endif SERV_NEW_ITEM_SYSTEM_2013_05
		//}}

		// 3. �к� ���� ������Ʈ
		std::map< UidType, u_char >::const_iterator mitSeal;
		mitSeal = mapSealItemList.find( kInventoryItemInfo.m_iItemUID );
		if( mitSeal != mapSealItemList.end() )
		{
			kInventoryItemInfo.m_kItemInfo.m_ucSealData = mitSeal->second;
		}

		// 4. �Ӽ� ���� ������Ʈ
		mitAttrib = mapAttribItemList.find( kInventoryItemInfo.m_iItemUID );
		if( mitAttrib != mapAttribItemList.end() )
		{
			BOOST_TEST_FOREACH( const KAttribInfo&, kAttribInfo, mitAttrib->second )
			{
				// �Ӽ� ��ȭ
				switch( kAttribInfo.first )
				{
				case CXSLAttribEnchantItem::ESI_SLOT_1:
					kInventoryItemInfo.m_kItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant0 = static_cast<char>( kAttribInfo.second );
					break;

				case CXSLAttribEnchantItem::ESI_SLOT_2:
					kInventoryItemInfo.m_kItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant1 = static_cast<char>( kAttribInfo.second );
					break;

				case CXSLAttribEnchantItem::ESI_SLOT_3:
					kInventoryItemInfo.m_kItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant2 = static_cast<char>( kAttribInfo.second );
					break;

				default:
					START_LOG( cerr, L"�Ӽ� ��ȭ ���� �ѹ��� �̻��մϴ�." )
						<< BUILD_LOG( kInventoryItemInfo.m_iItemUID )
						<< BUILD_LOG( kAttribInfo.first )
						<< BUILD_LOG( kAttribInfo.second )
						<< END_LOG;
					break;
				}
			}
		}
		
		//////////////////////////////////////////////////////////////////////////		
		// �� ī�װ����̸� ���� ����!
		if( kInventoryItemInfo.m_cSlotCategory == CXSLInventory::ST_PET )
		{
			std::map< UidType, UidType >::const_iterator mitPet;
			mitPet = mapPetItemList.find( kInventoryItemInfo.m_iItemUID );
			if( mitPet == mapPetItemList.end() )
			{
				START_LOG( cerr, L"�� ������ �������� �ش� �������� ã�� ���ߴ�! �Ͼ�� �ȵǴ� ����!" )
					<< BUILD_LOG( iUnitUID )
					<< BUILD_LOG( kInventoryItemInfo.m_iItemUID )
					<< END_LOG;
				continue;
			}

			const UidType iPetUID = mitPet->second;

			std::map< UidType, std::map< UidType, KInventoryItemInfo > >::iterator mitPetItem;
			mitPetItem = mapPetItemResult.find( iPetUID );
			if( mitPetItem == mapPetItemResult.end() )
			{
				std::map< UidType, KInventoryItemInfo > mapPetItem;
				mapPetItem.insert( std::make_pair( kInventoryItemInfo.m_iItemUID, kInventoryItemInfo ) );
				mapPetItemResult.insert( std::make_pair( iPetUID, mapPetItem ) );
			}
			else
			{
				mitPetItem->second.insert( std::make_pair( kInventoryItemInfo.m_iItemUID, kInventoryItemInfo ) );
			}
		}
		//////////////////////////////////////////////////////////////////////////		
		// �� ī�װ��� �ƴѰ͵��� �Ϲ� �κ��丮��!
		else
		{
			// ������ UID�� PK�̹Ƿ� map�� insert �� �� Ű �ߺ� ���� �˻縦 �� �ʿ䰡 ����.
			mapItemResult.insert( std::make_pair( kInventoryItemInfo.m_iItemUID, kInventoryItemInfo ) );
		}
	}

	return true;

end_proc:
	START_LOG( cerr, L"ĳ���� �κ��丮 ��� ����!" )
		<< BUILD_LOG( iUnitUID )
		<< END_LOG;
    return false;
}

#endif SERV_GET_INVENTORY_REFAC
//}}

//{{ 2010. 9. 8	������	������ ȹ�� ����
//////////////////////////////////////////////////////////////////////////
#ifdef SERV_GET_ITEM_REASON
//////////////////////////////////////////////////////////////////////////

bool KGSGameDBThread::Query_InsertItemList( IN const SEnum::GET_ITEM_REASON eGetItemReason, IN const UidType iUnitUID, IN const std::vector< KItemInfo >& vecItemInfo, OUT std::map< UidType, KItemInfo >& mapInsertedItemInfo, IN const bool bUnSeal /*= true*/ )
{
	if( SEnum::IsValidGetItemReason( eGetItemReason ) == false )
	{
		START_LOG( cerr, L"��ȿ�������� ������ ���� �����Դϴ�!" )
			<< BUILD_LOG( eGetItemReason )
			<< BUILD_LOG( iUnitUID )
			<< BUILD_LOG( vecItemInfo.size() )
			<< END_LOG;
	}

	BOOST_TEST_FOREACH( const KItemInfo&, kNewItemInfo, vecItemInfo )
	{
		KItemInfo kNewItemInfoResult = kNewItemInfo; // ���纻 ����

		//////////////////////////////////////////////////////////////////////////
		// ������ ����
		UidType iItemUID = 0;
		int iOK = NetError::ERR_ODBC_01;

		//{{ 2011. 08. 31	������	������ ���� ����
#ifdef SERV_NEW_ITEM_REASON_DB_LOG
		DO_QUERY( L"exec dbo.gup_insert_item", L"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
			% iUnitUID
			% kNewItemInfo.m_iItemID
			% static_cast<int>(kNewItemInfo.m_cUsageType)
			% kNewItemInfo.m_iQuantity
			% kNewItemInfo.m_sEndurance
			% kNewItemInfo.m_sPeriod
			% static_cast<int>(kNewItemInfo.m_cEnchantLevel)
			% static_cast<int>(kNewItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant0 )
			% static_cast<int>(kNewItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant1 )
			% static_cast<int>(kNewItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant2 )
			% static_cast<int>(eGetItemReason)
			);
#else
		DO_QUERY( L"exec dbo.gup_insert_item", L"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
			% iUnitUID
			% kNewItemInfo.m_iItemID
			% static_cast<int>(kNewItemInfo.m_cUsageType)
			% kNewItemInfo.m_iQuantity
			% kNewItemInfo.m_sEndurance
			% kNewItemInfo.m_sPeriod
			% static_cast<int>(kNewItemInfo.m_cEnchantLevel)
			% static_cast<int>(kNewItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant0 )
			% static_cast<int>(kNewItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant1 )
			% static_cast<int>(kNewItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant2 )
			);
#endif SERV_NEW_ITEM_REASON_DB_LOG
		//}}		

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK
				>> iItemUID
				>> kNewItemInfoResult.m_wstrExpirationDate );

			m_kODBC.EndFetch();
		}

		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"������ ���� ����." )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( iUnitUID )
				<< BUILD_LOG( kNewItemInfo.m_iItemID )
				<< BUILD_LOGc( kNewItemInfo.m_cUsageType )
				<< BUILD_LOG( kNewItemInfo.m_iQuantity )
				<< BUILD_LOG( kNewItemInfo.m_sEndurance )
				<< BUILD_LOG( kNewItemInfo.m_sPeriod )
				<< BUILD_LOGc( kNewItemInfo.m_cEnchantLevel )
				<< BUILD_LOGc( kNewItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant0 )
				<< BUILD_LOGc( kNewItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant1 )
				<< BUILD_LOGc( kNewItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant2 )
				<< END_LOG;
			goto end_proc;
		}

		//////////////////////////////////////////////////////////////////////////		
		//{{ 2009. 8. 27  ������	�к�
		if( kNewItemInfo.m_ucSealData > 0 )
		{
			//{{ 2009. 9. 2  ������		�к�
			if( kNewItemInfoResult.IsSealedItem()  &&  bUnSeal )
			{
				kNewItemInfoResult.UnsealItem(); // �к� ����
			}
			//}}

			DO_QUERY( L"exec dbo.gup_update_item_seal", L"%d, %d", % iItemUID % static_cast<int>(kNewItemInfoResult.m_ucSealData) );

			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( iOK );
				m_kODBC.EndFetch();
			}

			if( iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"�к� ���� ������Ʈ ����." )
					<< BUILD_LOG( iOK )
					<< BUILD_LOG( iItemUID )
					<< BUILD_LOGc( kNewItemInfoResult.m_ucSealData )
					<< END_LOG;
				goto end_proc;
			}
		}
		//}}

		//////////////////////////////////////////////////////////////////////////		
		// ���� ������ �����Ѵٸ� ���� DB�� insert����!
		if( kNewItemInfo.m_vecItemSocket.empty() == false )
		{
			bool bSocketExist = false;
			int arrSocketInfo[4] = {0,0,0,0}; // DB ���̺��� �������̺� ����
			//{{ 2011. 07. 25    ��μ�    ������ �ɼ�ID ������ ������ ����
#ifdef SERV_ITEM_OPTION_DATA_SIZE
			std::vector< int >::const_iterator vitSocket = kNewItemInfo.m_vecItemSocket.begin();
#else
			std::vector< short >::const_iterator vitSocket = kNewItemInfo.m_vecItemSocket.begin();
#endif SERV_ITEM_OPTION_DATA_SIZE
//}} 
			for( int iIdx = 0; iIdx < 4; ++iIdx )
			{
				if( vitSocket == kNewItemInfo.m_vecItemSocket.end() )
					continue;

				if( *vitSocket > 0 )
				{
					bSocketExist = true;
				}
				arrSocketInfo[iIdx] = *vitSocket;
				++vitSocket;
			}

			if( bSocketExist )
			{
				DO_QUERY( L"exec dbo.gup_update_Socket", L"%d, %d, %d, %d, %d, %d",
					% iUnitUID					// @iUnitUID bigint
					% iItemUID		            // @iItemUID bigint
					% arrSocketInfo[0]			// @iSoket1 smallint
					% arrSocketInfo[1]			// @iSoket2 smallint
					% arrSocketInfo[2]			// @iSoket3 smallint
					% arrSocketInfo[3]			// @iSoket4 smallint
					);

				if( m_kODBC.BeginFetch() )
				{
					FETCH_DATA( iOK );
					m_kODBC.EndFetch();
				}

				if( iOK != NetError::NET_OK )
				{
					START_LOG( cerr, L"������ ���� ���� ������Ʈ ����." )
						<< BUILD_LOG( iOK )
						<< BUILD_LOG( iUnitUID )
						<< BUILD_LOG( iItemUID )
						<< BUILD_LOG( arrSocketInfo[0] )
						<< BUILD_LOG( arrSocketInfo[1] )
						<< BUILD_LOG( arrSocketInfo[2] )
						<< BUILD_LOG( arrSocketInfo[3] )
						<< END_LOG;
				}
			}
		}

		//{{ 2013. 05. 21	������	������ ����
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
		//////////////////////////////////////////////////////////////////////////		
		// ���� ���� ������ �����Ѵٸ� ���� DB�� insert����!
		if( kNewItemInfo.m_vecRandomSocket.empty() == false )
		{
			bool bSocketExist = false;
			int arrSocketInfo[CXSLSocketItem::RSC_MAX] = {0,}; // DB ���̺��� �������̺� ����
			
			std::vector< int >::const_iterator vitSocket = kNewItemInfo.m_vecRandomSocket.begin();
			for( int iIdx = 0; iIdx < CXSLSocketItem::RSC_MAX; ++iIdx )
			{
				if( vitSocket == kNewItemInfo.m_vecRandomSocket.end() )
					continue;

				if( *vitSocket > 0 )
				{
					bSocketExist = true;
				}
				arrSocketInfo[iIdx] = *vitSocket;
				++vitSocket;
			}

			if( bSocketExist )
			{
				DO_QUERY( L"exec dbo.P_GItemSocket_Random_INT_UPD", L"%d, %d, %d, %d, %d, %d, %d",
					% iUnitUID					// @iUnitUID bigint
					% iItemUID		            // @iItemUID bigint
					% arrSocketInfo[0]			// @iSoket1 smallint
					% arrSocketInfo[1]			// @iSoket2 smallint
					% arrSocketInfo[2]			// @iSoket3 smallint
					% arrSocketInfo[3]			// @iSoket4 smallint
					% arrSocketInfo[4]			// @iSoket5 smallint
					);

				if( m_kODBC.BeginFetch() )
				{
					FETCH_DATA( iOK );
					m_kODBC.EndFetch();
				}

				if( iOK != NetError::NET_OK )
				{
					START_LOG( cerr, L"���� ���� ���� ������Ʈ ����." )
						<< BUILD_LOG( iOK )
						<< BUILD_LOG( iUnitUID )
						<< BUILD_LOG( iItemUID )
						<< BUILD_LOG( arrSocketInfo[0] )
						<< BUILD_LOG( arrSocketInfo[1] )
						<< BUILD_LOG( arrSocketInfo[2] )
						<< BUILD_LOG( arrSocketInfo[3] )
						<< BUILD_LOG( arrSocketInfo[4] )
						<< END_LOG;
				}
			}
		}
#endif SERV_NEW_ITEM_SYSTEM_2013_05
		//}}

		// �����Ǵ� ������ ������ ��� DB�� insert�Ͽ����Ƿ� �����̳ʿ��� ����!
		mapInsertedItemInfo.insert( std::make_pair( iItemUID, kNewItemInfoResult ) );
	}

	return true;

end_proc:
	return false;
}

//////////////////////////////////////////////////////////////////////////
//#else
////////////////////////////////////////////////////////////////////////////
//
//bool KGSGameDBThread::Query_InsertItemList( IN const UidType iUnitUID, IN const std::vector< KItemInfo >& vecItemInfo, OUT std::map< UidType, KItemInfo >& mapInsertedItemInfo, IN const bool bUnSeal /*= true*/ )
//{
//	std::vector< KItemInfo >::const_iterator vit;
//	for( vit = vecItemInfo.begin(); vit != vecItemInfo.end(); ++vit )
//	{
//		UidType iItemUID;
//		KItemInfo kItemInfo = *vit;
//		//{{ 2009. 9. 2  ������		�к�
//		if( kItemInfo.IsSealedItem()  &&  bUnSeal )
//		{
//			kItemInfo.m_ucSealData -= 100; // �к� ����
//		}
//		//}}
//
//		//////////////////////////////////////////////////////////////////////////
//		// ������ ����
//		int iOK = NetError::ERR_ODBC_01;
//
//		DO_QUERY( L"exec dbo.gup_insert_item", L"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
//			% iUnitUID
//			% vit->m_iItemID
//			% static_cast<int>(vit->m_cUsageType)
//			% vit->m_iQuantity
//			% vit->m_sEndurance
//			% vit->m_sPeriod
//			% static_cast<int>(vit->m_cEnchantLevel) 
//			% static_cast<int>(vit->m_kAttribEnchantInfo.m_cAttribEnchant0 ) 
//			% static_cast<int>(vit->m_kAttribEnchantInfo.m_cAttribEnchant1 )
//			//{{ 2009. 11. 9  ������	Ʈ���üӼ�
//			% static_cast<int>(vit->m_kAttribEnchantInfo.m_cAttribEnchant2 )
//			//}}
//			);
//
//		if( m_kODBC.BeginFetch() )
//		{
//			FETCH_DATA( iOK
//				>> iItemUID
//				>> kItemInfo.m_wstrExpirationDate );
//
//			m_kODBC.EndFetch();
//		}
//
//		if( iOK == NetError::NET_OK )
//		{
//			// ������ UID �� �����ϹǷ� �ߺ� �˻� ���� map�� key�� ���� �� �ִ�.
//			mapInsertedItemInfo.insert( std::make_pair( iItemUID, kItemInfo ) );
//		}
//		else
//		{
//			START_LOG( cerr, L"������ ���� ����." )
//				<< BUILD_LOG( iOK )
//				<< BUILD_LOG( iUnitUID )
//				<< BUILD_LOG( vit->m_iItemID )
//				<< BUILD_LOGc( vit->m_cUsageType )
//				<< BUILD_LOG( vit->m_iQuantity )
//				<< BUILD_LOG( vit->m_sEndurance )
//				<< BUILD_LOG( vit->m_sPeriod )
//				<< BUILD_LOGc( vit->m_cEnchantLevel )
//				<< BUILD_LOGc( vit->m_kAttribEnchantInfo.m_cAttribEnchant0 )
//				<< BUILD_LOGc( vit->m_kAttribEnchantInfo.m_cAttribEnchant1 )
//				//{{ 2009. 11. 9  ������	Ʈ���üӼ�
//				<< BUILD_LOGc( vit->m_kAttribEnchantInfo.m_cAttribEnchant2 )
//				//}}
//				<< END_LOG;
//			goto end_proc;
//		}
//
//		//////////////////////////////////////////////////////////////////////////		
//		//{{ 2009. 8. 27  ������	�к�
//		if( vit->m_ucSealData > 0 )
//		{
//			DO_QUERY( L"exec dbo.gup_update_item_seal", L"%d, %d", % iItemUID % static_cast<int>(kItemInfo.m_ucSealData) );
//
//			if( m_kODBC.BeginFetch() )
//			{
//				FETCH_DATA( iOK );
//				m_kODBC.EndFetch();
//			}
//
//			if( iOK != NetError::NET_OK )
//			{
//				START_LOG( cerr, L"�к� ���� ������Ʈ ����." )
//					<< BUILD_LOG( iOK )
//					<< BUILD_LOG( iItemUID )
//					<< BUILD_LOGc( kItemInfo.m_ucSealData )
//					<< END_LOG;
//			}
//		}
//		//}}
//
//		//////////////////////////////////////////////////////////////////////////		
//		// ������ ��������� ���� ����������..
//		if( vit->m_vecItemSocket.empty() )
//			continue;
//
//		bool bSocketExist = false;
//		int arrSocketInfo[4] = {0,0,0,0}; // DB ���̺��� �������̺� ����
//		std::vector< short >::const_iterator vitSocket = vit->m_vecItemSocket.begin();
//		for( int iIdx = 0; iIdx < 4; ++iIdx )
//		{
//			if( vitSocket == vit->m_vecItemSocket.end() )
//				continue;
//
//			if( *vitSocket > 0 )
//			{
//				bSocketExist = true;
//			}
//			arrSocketInfo[iIdx] = *vitSocket;
//			++vitSocket;
//		}
//
//		if( bSocketExist )
//		{
//			DO_QUERY( L"exec dbo.gup_update_Socket", L"%d, %d, %d, %d, %d, %d",
//				% iUnitUID					// @iUnitUID bigint
//				% iItemUID		            // @iItemUID bigint
//				% arrSocketInfo[0]			// @iSoket1 smallint
//				% arrSocketInfo[1]			// @iSoket2 smallint
//				% arrSocketInfo[2]			// @iSoket3 smallint
//				% arrSocketInfo[3]			// @iSoket4 smallint
//				);
//
//			if( m_kODBC.BeginFetch() )
//			{
//				FETCH_DATA( iOK );
//				m_kODBC.EndFetch();
//			}
//
//			if( iOK != NetError::NET_OK )
//			{
//				START_LOG( cerr, L"������ ���� ���� ������Ʈ ����." )
//					<< BUILD_LOG( iOK )
//					<< BUILD_LOG( iUnitUID )
//					<< BUILD_LOG( iItemUID )
//					<< BUILD_LOG( arrSocketInfo[0] )
//					<< BUILD_LOG( arrSocketInfo[1] )
//					<< BUILD_LOG( arrSocketInfo[2] )
//					<< BUILD_LOG( arrSocketInfo[3] )
//					<< END_LOG;
//			}
//		}
//	}
//
//	return true;
//
//end_proc:
//	return false;
//}

//////////////////////////////////////////////////////////////////////////
#endif SERV_GET_ITEM_REASON
//////////////////////////////////////////////////////////////////////////
//}}

void KGSGameDBThread::Query_UpdateSealItem( IN const std::set< int >& setSealItem, IN OUT std::map< UidType, KItemInfo >& mapInsertedItemInfo )
{
	// �к� ������ ����Ʈ�� ������ �ƹ� ó�� ����
	if( setSealItem.empty() )
		return;

	std::map< UidType, KItemInfo >::iterator mit;
	for( mit = mapInsertedItemInfo.begin(); mit != mapInsertedItemInfo.end(); ++mit )
	{
		// �к� ������� �˻�
		if( setSealItem.find( mit->second.m_iItemID ) == setSealItem.end() )
			continue;

		// �Ⱓ�� �������̸� ���� ����
		if( mit->second.m_sPeriod > 0 )
			continue;

		// �к�ó���Ѵ�!
		mit->second.m_ucSealData = 100;

		int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_update_item_seal", L"%d, %d", % mit->first % static_cast<int>(mit->second.m_ucSealData) );
		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

end_proc:
		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"���� DB ������Ʈ ����." )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( mit->first )
				<< BUILD_LOGc( mit->second.m_ucSealData )
				<< END_LOG;

			// �к� DB������Ʈ ���������� �ѹ�
			mit->second.m_ucSealData = 0;
		}
	}
}

void KGSGameDBThread::Query_UpdateDungeonClear( UidType iUnitUID, std::map< int, KDungeonClearInfo >& mapDungeonClearInfo, std::vector< int >& vecFailed )
{
    vecFailed.clear();
    std::map< int, KDungeonClearInfo >::iterator mit = mapDungeonClearInfo.begin();

    while( mit != mapDungeonClearInfo.end() )
    {
        int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_insert_dungeon_clear", L"%d, %d, %d, %d, N\'%s\'",
			% iUnitUID
			% mit->second.m_iDungeonID
			% mit->second.m_iMaxScore
			% (int)mit->second.m_cMaxTotalRank
			% mit->second.m_wstrClearTime
			);

        if( m_kODBC.BeginFetch() )
        {
            FETCH_DATA( iOK );
            m_kODBC.EndFetch();
        }

        if( iOK != NetError::NET_OK )
        {
            START_LOG( cerr, L"���� Ŭ���� ���� ������Ʈ ����." )
                << BUILD_LOG( iOK )
                << END_LOG;

            goto end_proc;
        }

        mit = mapDungeonClearInfo.erase( mit );
        continue;

end_proc:
        mit++;
    }

    for( mit = mapDungeonClearInfo.begin(); mit != mapDungeonClearInfo.end(); mit++ )
    {
        vecFailed.push_back( mit->first );
    }
    return;
}

#ifdef SERV_LIMITED_DUNGEON_PLAY_TIMES
void KGSGameDBThread::Query_UpdateDungeonPlay( IN UidType iUnitUID, IN std::map< int, KDungeonPlayInfo >& mapDungeonPlayInfo )
{
	std::map< int, KDungeonPlayInfo >::iterator mitDungeonPlayInfo = mapDungeonPlayInfo.begin();

	while( mitDungeonPlayInfo != mapDungeonPlayInfo.end() )
	{
		int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.P_GEventDungeonData_SET", L"%d, %d, %d, %d",
			% iUnitUID
			% mitDungeonPlayInfo->second.m_iDungeonID
			% mitDungeonPlayInfo->second.m_iPlayTimes
			% mitDungeonPlayInfo->second.m_iClearTimes
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"���� �÷��� ���� ������Ʈ ����." )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( mitDungeonPlayInfo->second.m_iDungeonID )
				<< BUILD_LOG( mitDungeonPlayInfo->second.m_iPlayTimes )
				<< BUILD_LOG( mitDungeonPlayInfo->second.m_iClearTimes )
				<< END_LOG;

			goto end_proc;
		}

end_proc:
		mitDungeonPlayInfo++;
	}

}
#endif SERV_LIMITED_DUNGEON_PLAY_TIMES

void KGSGameDBThread::Query_UpdateTCClear( UidType iUnitUID, std::map< int, KTCClearInfo >& mapTCClearInfo, std::vector< int >& vecFailed )
{
    vecFailed.clear();
	std::map< int, KTCClearInfo >::iterator mit = mapTCClearInfo.begin();

	while( mit != mapTCClearInfo.end() )
	{
        int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_insert_tc_clear", L"%d, %d, N\'%s\'",
			% iUnitUID
			% mit->second.m_iTCID
			% mit->second.m_wstrClearTime
			);	

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"�Ʒü� Ŭ���� ���� ������Ʈ ����." )
				<< BUILD_LOG( iOK )
				<< END_LOG;

			goto end_proc;
		}

		mit = mapTCClearInfo.erase( mit );
		continue;

end_proc:
		mit++;
	}

    for( mit = mapTCClearInfo.begin(); mit != mapTCClearInfo.end(); mit++ )
    {
        vecFailed.push_back( mit->first );
    }
	return;
}

bool KGSGameDBThread::Query_UpdateItemQuantity( UidType iUnitUID, std::map< UidType, int >& mapUpdated, std::map< UidType, int >& mapFailed, bool& bUpdateFailed )
{
	//{{ 2009. 11. 17  ������	DB��������	
	bool bQuerySuccess = true;
	//}}
	bUpdateFailed = false;
    mapFailed.clear();

    std::map< UidType, int >::iterator mit = mapUpdated.begin();
    while( mit != mapUpdated.end() )
    {
        int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_update_item", L"%d, %d, %d, %d", 
			% iUnitUID 
			% mit->first 
			% CXSLItem::PT_QUANTITY 
			% mit->second
			);

// 		START_LOG( cerr, L"������Ʈ ���� Ȯ������" )
// 			<< BUILD_LOG( iUnitUID )
// 			<< BUILD_LOG( mit->first  )
// 			<< BUILD_LOG( mit->second  )
// 			<< END_LOG;

        if( m_kODBC.BeginFetch() )
        {
            FETCH_DATA( iOK );
            m_kODBC.EndFetch();

            if( iOK != NetError::NET_OK )
            {
                START_LOG( cerr, L"������ ���� ������Ʈ ����." )
                    << BUILD_LOG( iOK )
					<< BUILD_LOG( iUnitUID )
					<< BUILD_LOG( mit->first )
					<< BUILD_LOG( mit->second )
                    << END_LOG;

				//{{ 2008. 10. 23  ������	���а� �����ѹ� ��ƺ���~!
				if( iOK == -5 )
				{
					START_LOG( cout, L"���а� ������Ʈ ���� �߻�!" )
						<< BUILD_LOG( iOK )
						<< BUILD_LOG( iUnitUID )
						<< BUILD_LOG( mit->first )
						<< BUILD_LOG( mit->second );

					bUpdateFailed = true;
				}
				//}}
            }
        }
        else
        {
            START_LOG( cerr, L"������ ���� ������Ʈ�� BeginFetch ����." );
        }

end_proc:
        if( iOK != NetError::NET_OK  &&  iOK != -5 )
        {
			//{{ 2009. 11. 17  ������	DB��������
			bQuerySuccess = false;
			//}}

			mapFailed.insert( std::make_pair( mit->first, mit->second ) );
        }
        mit++;
    }

	return bQuerySuccess;
}

bool KGSGameDBThread::Query_UpdateItemEndurance( IN UidType iUnitUID, IN std::map< UidType, int >& mapUpdated, OUT std::map< UidType, int >& mapFailed )
{
	//{{ 2009. 11. 17  ������	DB��������
	bool bQuerySuccess = true;
	//}}

	mapFailed.clear();

	std::map< UidType, int >::iterator mit = mapUpdated.begin();
	while( mit != mapUpdated.end() )
	{
		int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_update_item", L"%d, %d, %d, %d", 
			% iUnitUID
			% mit->first 
			% CXSLItem::PT_ENDURANCE 
			% mit->second 
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();

			if( iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"������ ���� ������Ʈ ����." )
					<< BUILD_LOG( iOK )
					<< BUILD_LOG( iUnitUID )
					<< BUILD_LOG( mit->first )
					<< BUILD_LOG( mit->second )
					<< END_LOG;
			}
		}
		else
		{
			START_LOG( cerr, L"������ ������ ������Ʈ�� BeginFetch ����." );
		}

end_proc:
		if( iOK != NetError::NET_OK )
		{
			//{{ 2009. 11. 17  ������	DB��������
			bQuerySuccess = false;
			//}}

			mapFailed.insert( std::make_pair( mit->first, mit->second ) );
		}
		mit++;
	}

	return bQuerySuccess;
}

//void KGSGameDBThread::UpdateItemPosition( IN std::map< UidType, std::pair< int, int > >& mapUpdated, OUT std::map< UidType, std::pair< int, int > >& mapFailed, IN bool bFinal )
//{
//	mapFailed.clear();
//
//	std::map< UidType, std::pair< int, int > >::iterator mit = mapUpdated.begin();
//	while( mit != mapUpdated.end() )
//	{
//		int iOK = NetError::ERR_UNKNOWN;
//		DO_QUERY( ( L"exec dbo.gup_update_item_position %d, %d, %d" ) % mit->first % mit->second.first % mit->second.second );
//
//		if( m_kODBC.BeginFetch() )
//		{
//			FETCH_DATA( iOK );
//			m_kODBC.EndFetch();
//
//			if( iOK != NetError::NET_OK )
//			{
//				START_LOG( cerr, L"������ ��ġ ������Ʈ ����." )
//					<< BUILD_LOG( iOK )
//					<< END_LOG;
//			}
//		}
//		else
//		{
//			START_LOG( cerr, L"������ ��ġ ������Ʈ�� BeginFetch ����." );
//		}
//
//end_proc:
//		if( iOK != NetError::NET_OK )
//		{
//			mapFailed.insert( std::make_pair( mit->first, mit->second ) );
//
//			//{{ 2008. 7. 8  ������  �κ� ī�װ��� ���� ã��
//			START_LOG( cerr, L"�κ� �����ϱ�?!" )
//				<< BUILD_LOG( bFinal );
//			//}}	
//		}
//		mit++;
//	}
//}

//{{ 2010. 8. 3	������	�� �ý���
#ifdef SERV_PET_SYSTEM

void KGSGameDBThread::Query_UpdateItemPosition( IN UidType iUnitUID, IN std::map< UidType, KItemPosition >& mapUpdated, OUT std::map< UidType, KItemPosition >& mapFailed )
{
	mapFailed.clear();

	std::map< UidType, KItemPosition >::iterator mit = mapUpdated.begin();
	while( mit != mapUpdated.end() )
	{
		int iOK = NetError::ERR_ODBC_01;

		// �� �κ��丮�� ������ �ִ� �������� üũ!
		if( mit->second.m_iPetUID == KItemPosition::IPE_INVALID_PET_UID )
		{
			DO_QUERY( L"exec dbo.gup_update_item_position", L"%d, %d, %d",
				% mit->first
				% mit->second.m_iSlotCategory
				% mit->second.m_iSlotID
				);
		}
		else
		{
			DO_QUERY( L"exec dbo.gup_update_item_position_pet", L"%d, %d, %d, %d, %d", 
				% iUnitUID
				% mit->first
				% mit->second.m_iPetUID
				% mit->second.m_iSlotCategory
				% mit->second.m_iSlotID
				);
		}

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();

			if( iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"������ ��ġ ������Ʈ ����." )
					<< BUILD_LOG( iOK )
					<< BUILD_LOG( iUnitUID )
					<< BUILD_LOG( mit->first )
					<< BUILD_LOG( mit->second.m_iPetUID )
					<< BUILD_LOG( mit->second.m_iSlotCategory )
					<< BUILD_LOG( mit->second.m_iSlotID )
					<< END_LOG;
			}
		}
		else
		{
			START_LOG( cerr, L"������ ��ġ ������Ʈ�� BeginFetch ����." );
		}

end_proc:
		if( iOK == NetError::NET_OK )
		{
			mapFailed.insert( std::make_pair( mit->first, mit->second ) );
		}
		mit++;
	}
}

#else

void KGSGameDBThread::Query_UpdateItemPosition( IN UidType iUnitUID, IN std::map< UidType, std::pair< int, int > >& mapUpdated, OUT std::map< UidType, std::pair< int, int > >& mapFailed )
{
	mapFailed.clear();

	std::map< UidType, std::pair< int, int > >::iterator mit = mapUpdated.begin();
	while( mit != mapUpdated.end() )
	{
		int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_update_item_position", L"%d, %d, %d", % mit->first % mit->second.first % mit->second.second );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();

			if( iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"������ ��ġ ������Ʈ ����." )
					<< BUILD_LOG( iOK )
					<< END_LOG;
			}
		}
		else
		{
			START_LOG( cerr, L"������ ��ġ ������Ʈ�� BeginFetch ����." );
		}

end_proc:
		if( iOK == NetError::NET_OK )
		{
			mapFailed.insert( std::make_pair( mit->first, mit->second ) );
		}
		mit++;
	}
}

#endif SERV_PET_SYSTEM
//}}

//{{ 2009. 12. 15  ������	������ ��������
void KGSGameDBThread::Query_DeleteItem( IN std::vector< KDeletedItemInfo >& vecDeleted, OUT std::vector< KDeletedItemInfo >& vecFailed )
{
    vecFailed.clear();

    std::vector< KDeletedItemInfo >::iterator vit = vecDeleted.begin();
    while( vit != vecDeleted.end() )
    {
        int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_delete_item", L"%d, %d", % vit->m_iItemUID % (int)vit->m_ucDeleteReason );
        
// 		START_LOG( cerr, L"���� ���� Ȯ������" )
// 			<< BUILD_LOG( vit->m_iItemUID )
// 			<< END_LOG;

        if( m_kODBC.BeginFetch() )
        {
            FETCH_DATA( iOK );
            m_kODBC.EndFetch();

            if( iOK != NetError::NET_OK )
            {
                START_LOG( cerr, L"������ ���� ����." )
                    << BUILD_LOG( iOK )
					<< BUILD_LOG( vit->m_iItemUID )
					<< BUILD_LOGc( vit->m_ucDeleteReason )
                    << END_LOG;
            }
        }
        else
        {
            START_LOG( cerr, L"������ ���� �� BeginFetch ����." );
        }
end_proc:
        if( iOK != NetError::NET_OK )
        {
            vecFailed.push_back( *vit );
        }
        ++vit;
    }
}
//}}

//{{ 2009. 10. 28  ������	��巹��
bool KGSGameDBThread::Query_GetGuildMemberGrade( IN UidType iUnitUID, OUT u_char& ucGuildMemberGrade )
{
	ucGuildMemberGrade = 0;
	int iHonorPointDummy = 0;

	DO_QUERY( L"exec dbo.gup_get_guild_member_grade", L"%d", % iUnitUID );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( ucGuildMemberGrade
			>> iHonorPointDummy );

		m_kODBC.EndFetch();
	}

	if( ucGuildMemberGrade == -1 )
	{
		START_LOG( cwarn, L"���� ��� ������ �������� ����." )
			<< BUILD_LOG( iUnitUID )
			<< END_LOG;

		goto end_proc;
	}

	return true;

end_proc:
	return false;
}
//}}

#ifdef GIANT_RESURRECTION_CASHSTONE
//{{ //2011.12.19 lygan_������ // �߱��� ĳ�������� ���ԵǴ� ��Ȱ�� �������� �۾�(�繫 �ɻ��)
int KGSGameDBThread::Query_UpdateResurrection_stone( IN const UidType iUnitUID, IN const int iNumResurrectionStone, OUT int& iACKNumResurrectionStone)
{
	int iOK = NetError::ERR_ODBC_01;
	DO_QUERY( L"exec dbo.gup_update_resurrection_stone", L"%d, %d", % iUnitUID % iNumResurrectionStone );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}

	if( iOK == NetError::NET_OK )
	{
		iACKNumResurrectionStone = 0;
	}
	else
	{
		START_LOG( cerr, L"��Ȱ�� ������Ʈ ����." )
			<< BUILD_LOG( iOK )
			<< END_LOG;
		goto end_proc;
	}

	return iOK;

end_proc:
	return iOK;
}

//}}
#endif //GIANT_RESURRECTION_CASHSTONE

//{{ 2012. 10. 23	������		DB������Ʈ �ڵ� �����丵
#ifdef SERV_DB_UPDATE_UNIT_INFO_REFACTORING
bool KGSGameDBThread::Query_UpdateUnitInfo( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq, OUT KDBE_UPDATE_UNIT_INFO_ACK& kAck )
{
	int iOK = NetError::ERR_ODBC_01;

	int iDummpMapID = 0;

	//{{ 2011. 10. 26	������	DB��ŷ Ʈ��
#ifdef SERV_DB_HACKING_ED_UPDATE_TRAP
	DO_QUERY( L"exec dbo.gup_update_ui_5", L"%d, %d, %d, %d, %d, %d, %d, %d",
		% kReq.m_iUnitUID
		% kReq.m_iEXP
		% kReq.m_iLevel
		% kReq.m_iED
		% kReq.m_iSPoint
		% iDummpMapID
		% kReq.m_iSpirit
		% kReq.m_bIsSpiritUpdated
		);
#else
	//{{ 2011. 07. 22	������	���� ����
	//#ifdef SERV_PVP_NEW_SYSTEM
	DO_QUERY( L"exec dbo.gup_update_unit_info", L"%d, %d, %d, %d, %d, %d, %d, %d",
		% kReq.m_iUnitUID
		% kReq.m_iEXP
		% kReq.m_iLevel
		% kReq.m_iED
		% kReq.m_iSPoint		
		% kReq.m_iMapID
		% kReq.m_iSpirit
		% kReq.m_bIsSpiritUpdated
		);
	//#else
	//	DO_QUERY( L"exec dbo.gup_update_unit_info", L"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
	//		% kPacket_.m_iUnitUID
	//		% kPacket_.m_iEXP
	//		% kPacket_.m_iLevel
	//		% kPacket_.m_iED
	//		% kPacket_.m_iVSPoint
	//		% kPacket_.m_iVSPointMax
	//		% kPacket_.m_iSPoint
	//		% kPacket_.m_iWin
	//		% kPacket_.m_iLose
	//		% kPacket_.m_iMapID
	//		% kPacket_.m_iSpirit
	//		% kPacket_.m_bIsSpiritUpdated
	//		);
	//#endif SERV_PVP_NEW_SYSTEM
	//}}
#endif SERV_DB_HACKING_ED_UPDATE_TRAP
	//}}	

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}

#ifdef SERV_CHINA_SPIRIT_EVENT
	if( iOK == NetError::NET_OK )
	{
		DO_QUERY( L"exec dbo.P_GEventChinaTicketSpirit_SET", L"%d, %d, %d, %d, %d, %d, %d", 
			% kReq.m_iUnitUID
			% kReq.m_arrChinaSpirit[0]
			% kReq.m_arrChinaSpirit[1]
			% kReq.m_arrChinaSpirit[2]
			% kReq.m_arrChinaSpirit[3]
			% kReq.m_arrChinaSpirit[4]
			% kReq.m_arrChinaSpirit[5]
			);
	}
#endif //SERV_CHINA_SPIRIT_EVENT

end_proc:
	if( iOK == NetError::NET_OK )
	{
		kAck.m_iEXP = 0;
		kAck.m_iED = 0;
		//{{ 2011. 07. 22	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
#else
		kAck.m_iVSPoint = 0;
		kAck.m_iVSPointMax = 0;
#endif SERV_PVP_NEW_SYSTEM
		//}}
		return true;
	}
	else
	{
		START_LOG( cerr, L"ĳ���� �⺻ ���� ������Ʈ ����." )
			<< BUILD_LOG( iOK )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< BUILD_LOG( kReq.m_iEXP )
			<< BUILD_LOG( kReq.m_iLevel )
			<< BUILD_LOG( kReq.m_iED )
			<< BUILD_LOG( kReq.m_iSPoint )
			<< BUILD_LOG( iDummpMapID )
			<< BUILD_LOG( kReq.m_iSpirit )
			<< BUILD_LOG( kReq.m_bIsSpiritUpdated )
			<< END_LOG;
		return false;
	}
}

bool KGSGameDBThread::Query_UpdatePvpInfo( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq, OUT KDBE_UPDATE_UNIT_INFO_ACK& kAck )
{
	int iOK = NetError::ERR_ODBC_01;

	//{{ 2011. 07. 22	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM

	//{{ 2012. 06. 22	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2
	DO_QUERY( L"exec dbo.P_GUnitPVP_Season2_UPT",
		L"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
		% kReq.m_iUnitUID
		% kReq.m_iOfficialMatchCnt
		% kReq.m_iRating
		% kReq.m_iMaxRating
		% kReq.m_iRPoint
		% kReq.m_iAPoint
		% kReq.m_bIsWinBeforeMatch
		% (int)kReq.m_cEmblemEnum
		% kReq.m_iWin
		% kReq.m_iLose
		% kReq.m_fKFactor
		);
#else
	DO_QUERY( L"exec dbo.gup_update_unitpvp"
		, L"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
		% kReq.m_iUnitUID
		% kReq.m_iOfficialMatchCnt
		% kReq.m_iRating
		% kReq.m_iMaxRating
		% kReq.m_iRPoint
		% kReq.m_iAPoint
		% kReq.m_bIsWinBeforeMatch
		% (int)kReq.m_cEmblemEnum
		% kReq.m_iWin
		% kReq.m_iLose
		);
#endif SERV_2012_PVP_SEASON2
	//}}

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}

end_proc:
	if( iOK == NetError::NET_OK )
	{
		kAck.m_iOfficialMatchCnt = 0;
		kAck.m_iRating = 0;
		kAck.m_iMaxRating = 0;
		kAck.m_iRPoint = 0;
		kAck.m_iAPoint = 0;
		kAck.m_iWin = 0;
		kAck.m_iLose = 0;
		//{{ 2012. 06. 22	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2
		kAck.m_fKFactor = 0.f;
#endif SERV_2012_PVP_SEASON2
		//}}
		return true;
	}
	else
	{
		START_LOG( cerr, L"���� ���� ���� ������Ʈ ����." )
			<< BUILD_LOG( iOK )
			//{{ 2012. 06. 22	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< BUILD_LOG( kReq.m_iOfficialMatchCnt )
			<< BUILD_LOG( kReq.m_iRating )
			<< BUILD_LOG( kReq.m_iMaxRating )
			<< BUILD_LOG( kReq.m_iRPoint )
			<< BUILD_LOG( kReq.m_iAPoint )
			<< BUILD_LOG( kReq.m_bIsWinBeforeMatch )
			<< BUILD_LOGc( kReq.m_cEmblemEnum )
			<< BUILD_LOG( kReq.m_iWin )
			<< BUILD_LOG( kReq.m_iLose )
			<< BUILD_LOG( kReq.m_fKFactor )
#endif SERV_2012_PVP_SEASON2
			//}}
			<< END_LOG;
		return false;
	}
#endif SERV_PVP_NEW_SYSTEM
	//}}
}

bool KGSGameDBThread::Query_UpdatePvpInfo( IN const KDBE_QUIT_USER_PVP_RESULT_UPDATE_NOT& kNot )
{
	int iOK = NetError::ERR_ODBC_01;

	//{{ 2011. 07. 22	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM

	//{{ 2012. 06. 22	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2
	DO_QUERY( L"exec dbo.P_GUnitPVP_Season2_UPT",
		L"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
		% kNot.m_iUnitUID
		% kNot.m_iOfficialMatchCnt
		% kNot.m_iRating
		% kNot.m_iMaxRating
		% kNot.m_iRPoint
		% kNot.m_iAPoint
		% kNot.m_bIsWinBeforeMatch
		% (int)kNot.m_cEmblemEnum
		% kNot.m_iWin
		% kNot.m_iLose
		% kNot.m_fKFactor
		);
#else
	DO_QUERY( L"exec dbo.gup_update_unitpvp"
		, L"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
		% kNot.m_iUnitUID
		% kNot.m_iOfficialMatchCnt
		% kNot.m_iRating
		% kNot.m_iMaxRating
		% kNot.m_iRPoint
		% kNot.m_iAPoint
		% kNot.m_bIsWinBeforeMatch
		% (int)kNot.m_cEmblemEnum
		% kNot.m_iWin
		% kNot.m_iLose
		);
#endif SERV_2012_PVP_SEASON2
	//}}

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}

end_proc:
	if( iOK == NetError::NET_OK )
	{
		return true;
	}
	else
	{
		START_LOG( cerr, L"���� ���� ���� ������Ʈ ����." )
			<< BUILD_LOG( iOK )
			//{{ 2012. 06. 22	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2
			<< BUILD_LOG( kNot.m_iUnitUID )
			<< BUILD_LOG( kNot.m_iOfficialMatchCnt )
			<< BUILD_LOG( kNot.m_iRating )
			<< BUILD_LOG( kNot.m_iMaxRating )
			<< BUILD_LOG( kNot.m_iRPoint )
			<< BUILD_LOG( kNot.m_iAPoint )
			<< BUILD_LOG( kNot.m_bIsWinBeforeMatch )
			<< BUILD_LOGc( kNot.m_cEmblemEnum )
			<< BUILD_LOG( kNot.m_iWin )
			<< BUILD_LOG( kNot.m_iLose )
			<< BUILD_LOG( kNot.m_fKFactor )
#endif SERV_2012_PVP_SEASON2
			//}}
			<< END_LOG;
		return false;
	}
#endif SERV_PVP_NEW_SYSTEM
	//}}
}

bool KGSGameDBThread::Query_UpdateLastGamePoint( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq )
{
	int iOK = NetError::ERR_ODBC_01;
	
	// �ش� ĳ������ ������ ��ġ ���� ����!
	DO_QUERY( L"exec dbo.gup_update_unit_lastpoint", L"%d, %d", % kReq.m_iUnitUID % kReq.m_iRealDataED );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}

end_proc:
	if( iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"�α� ���� EDüũ ���� ������Ʈ ����." )
			<< BUILD_LOG( iOK )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< BUILD_LOG( kReq.m_iRealDataED )
			<< END_LOG;
		return false;
	}

	return true;
}

bool KGSGameDBThread::Query_UpdateResurrectionStone( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq, OUT KDBE_UPDATE_UNIT_INFO_ACK& kAck )
{
	// �߱��� ��� ��Ȱ�� ��ȭ���� ��� ���ν��� ȣ����
#ifndef GIANT_RESURRECTION_CASHSTONE
	// ��Ȱ�� ��ȭ���� ������ ���ν��� ȣ�����. (�߱�����)
	if( kReq.m_iNumResurrectionStone != 0 )
#endif //GIANT_RESURRECTION_CASHSTONE
	{
		int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_update_resurrection_stone", L"%d, %d", % kReq.m_iUnitUID % kReq.m_iNumResurrectionStone );
		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK == NetError::NET_OK )
		{
			kAck.m_iNumResurrectionStone = 0;
		}
		else
		{
			START_LOG( cerr, L"��Ȱ�� ������Ʈ ����." )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kReq.m_iNumResurrectionStone )
				<< END_LOG;
			goto end_proc;
		}
	}

	//{{ 2009. 10. 14  ������	�ڵ����� ��Ȱ��
#ifdef AP_RESTONE
	if( kReq.m_iNumAutoPaymentResStone != 0 )
	{
		int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_update_resurrection_stone_autopay", L"%d, %d", % kReq.m_iUnitUID % kReq.m_iNumAutoPaymentResStone );
		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK == NetError::NET_OK )
		{
			kAck.m_iNumAutoPaymentResStone = 0;
		}
		else
		{
			START_LOG( cerr, L"�ڵ����� ��Ȱ�� ������Ʈ ����." )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kReq.m_iNumAutoPaymentResStone )
				<< END_LOG;
			goto end_proc;
		}
	}
#endif AP_RESTONE
	//}}

	return true;

end_proc:
	START_LOG( cerr, L"��Ȱ�� ���� ������Ʈ ����!" )
		<< BUILD_LOG( kReq.m_iUnitUID )
		<< BUILD_LOG( kReq.m_iNumResurrectionStone )
		<< BUILD_LOG( kReq.m_iNumAutoPaymentResStone )
		<< END_LOG;
	return false;
}

bool KGSGameDBThread::Query_UpdateQuestInfo( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq )
{
	//QUEST UPDATE
	BOOST_TEST_FOREACH( const KQuestUpdate&, kQuestInfo, kReq.m_vecQuestData )
	{
		if( kQuestInfo.m_vecClearData.size() != 5 )
		{
			START_LOG( cerr, L"����Ʈ ���� ������ 5���� �ƴϴ�! ���� �Ͼ���� �ȵǴ� ����!" )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kQuestInfo.m_iQuestID )
				<< BUILD_LOG( kQuestInfo.m_vecClearData.size() )
				<< END_LOG;
			continue;
		}

		int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_update_quest", L"%d, %d, %d, %d, %d, %d, %d",
			% kReq.m_iUnitUID
			% kQuestInfo.m_iQuestID
			% (int)kQuestInfo.m_vecClearData[0]
			% (int)kQuestInfo.m_vecClearData[1]
			% (int)kQuestInfo.m_vecClearData[2]
			% (int)kQuestInfo.m_vecClearData[3]
			% (int)kQuestInfo.m_vecClearData[4]
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"����Ʈ ���� ���� ������Ʈ ����!" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kQuestInfo.m_iQuestID )
				<< BUILD_LOGc( kQuestInfo.m_vecClearData[0] )
				<< BUILD_LOGc( kQuestInfo.m_vecClearData[1] )
				<< BUILD_LOGc( kQuestInfo.m_vecClearData[2] )
				<< BUILD_LOGc( kQuestInfo.m_vecClearData[3] )
				<< BUILD_LOGc( kQuestInfo.m_vecClearData[4] )
				<< END_LOG;
		}
	}

	//{{ 2010. 02. 09  ������	���� �̺�Ʈ ����Ʈ
#ifdef SERV_DAY_QUEST

	// ����� ���� ����Ʈ
	BOOST_TEST_FOREACH( const int, iGiveUpQuestID, kReq.m_vecGiveUpQuestList )
	{
		int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_delete_quest", L"%d, %d", % kReq.m_iUnitUID % iGiveUpQuestID );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"����Ʈ ���� ���� ������Ʈ ����!" )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( iGiveUpQuestID )
				<< END_LOG;
		}
	}

#endif SERV_DAY_QUEST
	//}}
	return true;

end_proc:
	START_LOG( cerr, L"����Ʈ ���� ������Ʈ ����!" )
		<< BUILD_LOG( kReq.m_iUnitUID )
		<< END_LOG;
	return false;
}

bool KGSGameDBThread::Query_UpdateTitleInfo( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq )
{	
	//TITLE MISSION UPDATE
	BOOST_TEST_FOREACH( const KMissionUpdate&, kMission, kReq.m_vecMissionData )
	{
		if( kMission.m_vecClearData.size() != 5 )
		{
			START_LOG( cerr, L"Īȣ Ŭ���� ������ 5���� �ƴϴ�! ���� �Ͼ���� �ȵǴ� ����!" )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kMission.m_iTitleID )
				<< BUILD_LOG( kMission.m_vecClearData.size() )
				<< END_LOG;
			continue;
		}

		int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_update_title_mission", L"%d, %d, %d, %d, %d, %d, %d",
			% kReq.m_iUnitUID
			% kMission.m_iTitleID
			% kMission.m_vecClearData[0]
			% kMission.m_vecClearData[1]
			% kMission.m_vecClearData[2]
			% kMission.m_vecClearData[3]
			% kMission.m_vecClearData[4]
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"�̼� ���� ������Ʈ ����!" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kMission.m_iTitleID )
				<< BUILD_LOGc( kMission.m_vecClearData[0] )
				<< BUILD_LOGc( kMission.m_vecClearData[1] )
				<< BUILD_LOGc( kMission.m_vecClearData[2] )
				<< BUILD_LOGc( kMission.m_vecClearData[3] )
				<< BUILD_LOGc( kMission.m_vecClearData[4] )
				<< END_LOG;
			goto end_proc;
		}
	}

	//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
//#ifdef SERV_TITLE_DATA_SIZE

	// �������� Ÿ��Ʋ ������Ʈ
	if( kReq.m_iInitTitleID != kReq.m_iEquippedTitleID )
	{
		int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_update_title_hang", L"%d, %d, %d",
			% kReq.m_iUnitUID
			% kReq.m_iInitTitleID
			% kReq.m_iEquippedTitleID
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"Īȣ ���� ���� ������Ʈ ����!" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kReq.m_iInitTitleID )
				<< BUILD_LOG( kReq.m_iEquippedTitleID )
				<< END_LOG;
			goto end_proc;
		}
	}

//#else
//
//	// �������� Ÿ��Ʋ ������Ʈ
//	if( kPacket_.m_sInitTitleID != kPacket_.m_sEquippedTitleID )
//	{
//		DO_QUERY( L"exec dbo.gup_update_title_hang", L"%d, %d, %d",
//			% kPacket_.m_iUnitUID
//			% kPacket_.m_sInitTitleID
//			% kPacket_.m_sEquippedTitleID
//			);
//
//		if( m_kODBC.BeginFetch() )
//		{
//			FETCH_DATA( iOK );
//			m_kODBC.EndFetch();
//		}
//
//		if( iOK != NetError::NET_OK )
//		{
//			START_LOG( cerr, L"Īȣ ���� ���� ������Ʈ ����!" )
//				<< BUILD_LOG( iOK )
//				<< BUILD_LOG( kPacket_.m_iUnitUID )
//				<< BUILD_LOG( kPacket_.m_sInitTitleID )
//				<< BUILD_LOG( kPacket_.m_sEquippedTitleID )
//				<< END_LOG;
//
//			kPacket.m_iOK = iOK;
//		}
//	}
//
//#endif SERV_TITLE_DATA_SIZE
	//}}
	return true;

end_proc:
	START_LOG( cerr, L"Īȣ ���� ������Ʈ ����!" )
		<< BUILD_LOG( kReq.m_iUnitUID )
		<< END_LOG;
    return false;
}

bool KGSGameDBThread::Query_UpdateSkillSlotInfo( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq )
{
	if( kReq.m_vecSkillSlot.size() < 4 )
	{
		START_LOG( cerr, L"��ų ���� ������ �̻��մϴ�!" )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< BUILD_LOG( kReq.m_vecSkillSlot.size() )
			<< END_LOG;
		return false;
	}

	//SKILL SLOT UPDATE
	{
		int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_insert_skill_slot_new", L"%d, %d, %d, %d, %d",
			% kReq.m_iUnitUID
			% kReq.m_vecSkillSlot[0]
			% kReq.m_vecSkillSlot[1]
			% kReq.m_vecSkillSlot[2]
			% kReq.m_vecSkillSlot[3]
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"��ų ���� ���� ������Ʈ ����!" )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kReq.m_vecSkillSlot[0] )
				<< BUILD_LOG( kReq.m_vecSkillSlot[1] )
				<< BUILD_LOG( kReq.m_vecSkillSlot[2] )
				<< BUILD_LOG( kReq.m_vecSkillSlot[3] )
				<< END_LOG;
			goto end_proc;
		}
	}
	
	//SKILL SLOT B UPDATE
	if( kReq.m_vecSkillSlot.size() >= 8 )
	{
		int iOK = NetError::ERR_ODBC_01;

		// [���� ����]
		DO_QUERY( L"exec dbo.gup_insert_skill_slot2_new", L"%d, %d, %d, %d, %d",
			% kReq.m_iUnitUID
			% kReq.m_vecSkillSlot[4]
			% kReq.m_vecSkillSlot[5]
			% kReq.m_vecSkillSlot[6]
			% kReq.m_vecSkillSlot[7]
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"��ų ����B ���� ������Ʈ ����!" )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kReq.m_vecSkillSlot[4] )
				<< BUILD_LOG( kReq.m_vecSkillSlot[5] )
				<< BUILD_LOG( kReq.m_vecSkillSlot[6] )
				<< BUILD_LOG( kReq.m_vecSkillSlot[7] )
				<< END_LOG;
			goto end_proc;
		}
	}
	
	return true;

end_proc:
	START_LOG( cerr, L"��ų ���� ���� ������Ʈ ����!" )
		<< BUILD_LOG( kReq.m_iUnitUID )
		<< END_LOG;
	return false;
}

bool KGSGameDBThread::Query_UpdateCommunityOption( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq )
{
	int iOK = NetError::ERR_ODBC_01;

	//COMMUNITY OPTION UPDATE
	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
	DO_QUERY( L"exec dbo.P_GDenyOption_UPD", L"%d, %d, %d, %d, %d, %d",
		% kReq.m_iUnitUID
		% (int)kReq.m_kDenyOptions.m_cDenyFriendShip
		% (int)kReq.m_kDenyOptions.m_cDenyInviteGuild
		% (int)kReq.m_kDenyOptions.m_cDenyParty
		% (int)kReq.m_kDenyOptions.m_cDenyPersonalTrade
		% (int)kReq.m_kDenyOptions.m_cDenyRequestCouple
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}
#else
// �ؿ��� SP �̸� ����
	DO_QUERY( L"exec dbo.gup_update_community_opt", L"%d, %d, %d, %d, %d",
		% kReq.m_iUnitUID
		% (int)kReq.m_kDenyOptions.m_cDenyFriendShip
		% (int)kReq.m_kDenyOptions.m_cDenyInviteGuild
		% (int)kReq.m_kDenyOptions.m_cDenyParty
		% (int)kReq.m_kDenyOptions.m_cDenyPersonalTrade
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}
#endif SERV_RELATIONSHIP_SYSTEM
	//}

end_proc:
	if( iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"Ŀ�´�Ƽ �ɼ� ���� ������Ʈ ����!" )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< BUILD_LOGc( kReq.m_kDenyOptions.m_cDenyFriendShip )
			<< BUILD_LOGc( kReq.m_kDenyOptions.m_cDenyInviteGuild )
			<< BUILD_LOGc( kReq.m_kDenyOptions.m_cDenyParty )
			<< BUILD_LOGc( kReq.m_kDenyOptions.m_cDenyPersonalTrade )
			//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
			<< BUILD_LOGc( kReq.m_kDenyOptions.m_cDenyRequestCouple )
#endif SERV_RELATIONSHIP_SYSTEM
			//}
			<< END_LOG;
		return false;
	}

	return true;
}

bool KGSGameDBThread::Query_UpdateWishList( IN const UidType iUserUID, IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq )
{	
	std::map< int, int >::const_iterator mitWishList;
	for( mitWishList = kReq.m_mapWishList.begin(); mitWishList != kReq.m_mapWishList.end(); ++mitWishList )
	{
		int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_update_WishList", L"%d, %d, %d", % iUserUID % mitWishList->first % mitWishList->second );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

end_proc:
		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"��ٱ��� DB������Ʈ ����!" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( iUserUID )
				<< BUILD_LOG( mitWishList->first )
				<< BUILD_LOG( mitWishList->second )
				<< END_LOG;
		}
	}

	return true;
}

bool KGSGameDBThread::Query_UpdateRankingInfo( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq )
{
	BOOST_TEST_FOREACH( const KHenirRankingInfo&, kRankingInfo, kReq.m_vecHenirRanking )
	{
		// RegDate ��ȯ
		CTime tRegDate = CTime( kRankingInfo.m_tRegDate );
		std::wstring wstrRegDate = ( CStringW )( tRegDate.Format( _T( "%Y-%m-%d %H:%M:%S" ) ) );

		int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_insert_rank_myrecord", L"%d, %d, %d, %d, %d, N\'%s\'",
			% kReq.m_iUnitUID
			% kRankingInfo.m_iStageCount
			% kRankingInfo.m_ulPlayTime
			% (int)kRankingInfo.m_ucLevel
			% (int)kRankingInfo.m_cUnitClass
			% wstrRegDate
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

end_proc:
		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"��ϸ� ��ŷ ���� DB������Ʈ ����!" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kRankingInfo.m_iStageCount )
				<< BUILD_LOG( kRankingInfo.m_ulPlayTime )
				<< BUILD_LOG( (int)kRankingInfo.m_ucLevel )
				<< BUILD_LOG( (int)kRankingInfo.m_cUnitClass )
				<< BUILD_LOG( wstrRegDate )
				<< END_LOG;
			return false;
		}

		// ��ŷ������ �ϳ��ۿ� �ȵ���ִ�.
		break;
	}

	return true;
}

bool KGSGameDBThread::Query_UpdateGuildInfo( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq )
{
	//{{ 2009. 10. 7  ������	���
//#ifdef GUILD_TEST

	// ��� ���� ������Ʈ
	if( kReq.m_iGuildUID == 0 )
		return true;

	// ������ ������Ʈ��� �α׾ƿ� ó��
	if( kReq.m_bFinal )
	{
		int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_update_guild_lastdate", L"%d, %d",
			% kReq.m_iUnitUID
			% kReq.m_iGuildUID
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"��� �α׾ƿ� ó�� ����!" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kReq.m_iGuildUID )
				<< END_LOG;
		}
	}

	{
		int iOK = NetError::ERR_ODBC_01;

		// ��� ���� ����Ʈ ������Ʈ
		DO_QUERY( L"exec dbo.gup_update_guild_member_exp", L"%d, %d, %d",
			% kReq.m_iUnitUID
			% kReq.m_iGuildUID
			% kReq.m_iHonorPoint
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"���� ���� ����Ʈ ������Ʈ ����." )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kReq.m_iGuildUID )
				<< BUILD_LOG( kReq.m_iHonorPoint )
				<< END_LOG;
		}
	}

//#endif GUILD_TEST
	//}}

	return true;

end_proc:
	START_LOG( cerr, L"��� ���� ������Ʈ ����." )
		<< BUILD_LOG( kReq.m_iUnitUID )
		<< BUILD_LOG( kReq.m_iGuildUID )
		<< END_LOG;
	return false;
}

bool KGSGameDBThread::Query_UpdateConnectTimeEventInfo( IN const UidType iUserUID, IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq )
{	
	BOOST_TEST_FOREACH( const KCumulativeTimeEventInfo&, kEventInfo, kReq.m_vecUpdateEventTime )
	{
		//{{ 2010. 06. 15  ������	�������� ���ӽð� �̺�Ʈ
#ifdef SERV_ACC_TIME_EVENT
		if( kEventInfo.m_bAccountEvent )
		{
			int iOK = NetError::ERR_ODBC_01;

			DO_QUERY( L"exec dbo.gup_update_remaintime_account", L"%d, %d, %d", % iUserUID % kEventInfo.m_iEventUID % kEventInfo.m_iCumulativeTime );

			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( iOK );
				m_kODBC.EndFetch();
			}

			if( iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"�̺�Ʈ Ÿ�� DB������Ʈ ����?" )
					<< BUILD_LOG( iOK )
					<< BUILD_LOG( iUserUID )
					<< BUILD_LOG( kEventInfo.m_iEventUID )
					<< BUILD_LOG( kEventInfo.m_iCumulativeTime )
					<< END_LOG;
			}
		}
		else
#endif SERV_ACC_TIME_EVENT
		//}}
		{
			int iOK = NetError::ERR_ODBC_01;

			DO_QUERY( L"exec dbo.gup_update_remaintime", L"%d, %d, %d", % kReq.m_iUnitUID % kEventInfo.m_iEventUID % kEventInfo.m_iCumulativeTime );

			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( iOK );
				m_kODBC.EndFetch();
			}

			if( iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"�̺�Ʈ Ÿ�� DB������Ʈ ����?" )
					<< BUILD_LOG( iOK )
					<< BUILD_LOG( kReq.m_iUnitUID )
					<< BUILD_LOG( kEventInfo.m_iEventUID )
					<< BUILD_LOG( kEventInfo.m_iCumulativeTime )
					<< END_LOG;
			}
		}
	}

	return true;

end_proc:
	START_LOG( cerr, L"���� �ð� �̺�Ʈ DB������Ʈ ����" )
		<< BUILD_LOG( iUserUID )
		<< BUILD_LOG( kReq.m_iUnitUID )
		<< END_LOG;
	return false;
}

bool KGSGameDBThread::Query_UpdatePetInfo( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq, OUT KDBE_UPDATE_UNIT_INFO_ACK& kAck )
{	
	BOOST_TEST_FOREACH( const KPetInfo&, kPet, kReq.m_vecPet )
	{
		int iOK = NetError::ERR_ODBC_01;

		// �� ���� ������Ʈ
		DO_QUERY( L"exec dbo.gup_update_pet_info", L"%d, %d, %d, %d, %d, %d, %d, N\'%s\', N\'%s\'", 
			% kPet.m_iPetUID
			% (int)kPet.m_cEvolutionStep
			% kPet.m_sSatiety
			% kPet.m_iIntimacy
			% kPet.m_sExtroversion
			% kPet.m_sEmotion
			% kPet.m_bAutoFeed
			% kPet.m_wstrLastFeedDate
			% kPet.m_wstrLastSummonDate
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

end_proc:
		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"�� ���� ������Ʈ ����!" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kPet.m_iPetUID )
				<< BUILD_LOGc( kPet.m_cEvolutionStep )
				<< BUILD_LOG( kPet.m_sSatiety )
				<< BUILD_LOG( kPet.m_iIntimacy )
				<< BUILD_LOG( kPet.m_sExtroversion )
				<< BUILD_LOG( kPet.m_sEmotion )
				<< BUILD_LOG( kPet.m_bAutoFeed )
				<< BUILD_LOG( kPet.m_wstrLastFeedDate )
				<< BUILD_LOG( kPet.m_wstrLastSummonDate )
				<< END_LOG;

			// DB������Ʈ�� ���� �Ѵٸ� �ѹ� �����͸� ����!
			kAck.m_vecPet.push_back( kPet );
		}
	}

	return true;
}

bool KGSGameDBThread::Query_UpdateGameCountInfo( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq )
{
	std::map< int, int >::const_iterator mitCGC;
	for( mitCGC = kReq.m_mapCharGameCount.begin(); mitCGC != kReq.m_mapCharGameCount.end(); ++mitCGC )
	{
		int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_update_Character_Count", L"%d, %d, %d", 
			% kReq.m_iUnitUID
			% mitCGC->first
			% mitCGC->second
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

end_proc:
		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"ĳ���� ���� ī��Ʈ ���� ������Ʈ!" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( mitCGC->first )
				<< BUILD_LOG( mitCGC->second )
				<< END_LOG;
		}
	}

	return true;
}

bool KGSGameDBThread::Query_UpdateHenirRewardCount( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq )
{
	int iOK = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec dbo.gup_update_henir_reward_cnt", L"%d, %d, %d, %d", 
		% kReq.m_iUnitUID 
		% kReq.m_kHenirRewardCnt.m_iNormal 
		% kReq.m_kHenirRewardCnt.m_iPremium 
		% kReq.m_kHenirRewardCnt.m_iEvent 
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}

end_proc:
	if( iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"ĳ���� ���� ī��Ʈ ���� ������Ʈ!" )
			<< BUILD_LOG( iOK )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< BUILD_LOG( kReq.m_kHenirRewardCnt.m_iNormal )
			<< BUILD_LOG( kReq.m_kHenirRewardCnt.m_iPremium )
			<< BUILD_LOG( kReq.m_kHenirRewardCnt.m_iEvent )
			<< END_LOG;
		return false;
	}

	return true;
}

//{{ 2012. 10. 23	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
bool KGSGameDBThread::Query_UpdateLastPosition( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq )
{
	int iOK = NetError::ERR_ODBC_01;
	
	// ������ ��ġ ����
	DO_QUERY( L"exec dbo.P_GUnitLastPosition_UPT", L"%d, %d, %d, %d",
		% kReq.m_iUnitUID
		% kReq.m_kLastPos.m_iMapID
		% (int)kReq.m_kLastPos.m_ucLastTouchLineIndex
		% kReq.m_kLastPos.m_usLastPosValue
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}

end_proc:
	if( iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"���� ���� ��ġ ���� ������Ʈ ����." )
			<< BUILD_LOG( iOK )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< BUILD_LOG( kReq.m_kLastPos.m_iMapID )
			<< BUILD_LOGc( kReq.m_kLastPos.m_ucLastTouchLineIndex )
			<< BUILD_LOG( kReq.m_kLastPos.m_usLastPosValue )
			<< END_LOG;
		return false;
	}

	return true;
}

bool KGSGameDBThread::Query_UpdateGamePlayStatus( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq )
{	
	if( kReq.m_kGamePlayStatus.IsEmpty() )
		return true;

    KGamePlayStatus kPlayStatus;
	if( kReq.m_kGamePlayStatus.Get( kPlayStatus ) == false )
	{
		START_LOG( cerr, L"���� �÷��� ���� ������ ����? GSUser���� ������ �޾ƿ��� �ʳ�?" )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< END_LOG;
		return false;
	}

	{
		int iOK = NetError::ERR_ODBC_01;

		// ĳ���� �⺻ ���� ����
		DO_QUERY( L"exec dbo.P_GUnitPlayInfo_INS", L"%d, %d, %d, %d, %d, %d, %d",
			% kReq.m_iUnitUID
			% kPlayStatus.m_iMaxHP
			% kPlayStatus.m_iCurHP
			% kPlayStatus.m_iMaxMP
			% kPlayStatus.m_iCurMP
			% kPlayStatus.m_iCurHyperGage
			% kPlayStatus.m_iCharAbilCount
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"ĳ���� �÷��� ���� ���� ����!" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kPlayStatus.m_iMaxHP )
				<< BUILD_LOG( kPlayStatus.m_iCurHP )
				<< BUILD_LOG( kPlayStatus.m_iMaxMP )
				<< BUILD_LOG( kPlayStatus.m_iCurMP )
				<< BUILD_LOG( kPlayStatus.m_iCurHyperGage )
				<< BUILD_LOG( kPlayStatus.m_iCharAbilCount )
				<< END_LOG;
		}
	}

	// ��ų ��Ÿ�� ������ ����!
#ifdef SERV_CHANGE_SKILL_COOL_TIME_DB_SP 	// ���볯¥: 2013-05-02
	{
		int arrySlotID[8] = {-1,};
		int arryCoolTime[8] = {0,};
        for( int index = 0 ; index < 8 ; ++index )
		{
			arrySlotID[index] = -1;
			arryCoolTime[index] = 0;
		}

		std::map< int, int >::const_iterator mit = kPlayStatus.m_mapSkillCoolTime.begin();
		for( int index = 0 ; mit != kPlayStatus.m_mapSkillCoolTime.end(); ++mit, ++index )
		{
			if( index >= 8 )
			{
				// �迭 ������ ���� �̹Ƿ� Ȯ�� �ʿ�
				START_LOG( cerr, L"��ų �迭 index �� ���� �Ǿ����ϴ�." )
					<< BUILD_LOG( kReq.m_iUnitUID )
					<< BUILD_LOG( index )
					<< BUILD_LOG( mit->first )
					<< BUILD_LOG( mit->second )
					<< END_LOG;
				continue;;
			}

			arrySlotID[index] = mit->first;
			arryCoolTime[index] = mit->second;
		}

		int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.P_GCoolTime_INS_New", 
								L"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
			% kReq.m_iUnitUID
			% KGamePlayStatus::CTT_SKILL_COOL_TIME
			% arrySlotID[0]
			% arryCoolTime[0]
			% arrySlotID[1]
			% arryCoolTime[1]
			% arrySlotID[2]
			% arryCoolTime[2]
			% arrySlotID[3]
			% arryCoolTime[3]
			% arrySlotID[4]
			% arryCoolTime[4]
			% arrySlotID[5]
			% arryCoolTime[5]
			% arrySlotID[6]
			% arryCoolTime[6]
			% arrySlotID[7]
			% arryCoolTime[7]
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"ĳ���� ��ų ��Ÿ�� ���� ���� ����!" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( arrySlotID[0] )
				<< BUILD_LOG( arryCoolTime[0] )
				<< BUILD_LOG( arrySlotID[1] )
				<< BUILD_LOG( arryCoolTime[1] )
				<< BUILD_LOG( arrySlotID[2] )
				<< BUILD_LOG( arryCoolTime[2] )
				<< BUILD_LOG( arrySlotID[3] )
				<< BUILD_LOG( arryCoolTime[3] )
				<< BUILD_LOG( arrySlotID[4] )
				<< BUILD_LOG( arryCoolTime[4] )
				<< BUILD_LOG( arrySlotID[5] )
				<< BUILD_LOG( arryCoolTime[5] )
				<< BUILD_LOG( arrySlotID[6] )
				<< BUILD_LOG( arryCoolTime[6] )
				<< BUILD_LOG( arrySlotID[7] )
				<< BUILD_LOG( arryCoolTime[7] )
				<< END_LOG;
		}
	}

#else		// SERV_CHANGE_SKILL_COOL_TIME_DB_SP

	// ��ų ��Ÿ�� ������ ����!
	{
		std::map< int, int >::const_iterator mit;
		for( mit = kPlayStatus.m_mapSkillCoolTime.begin(); mit != kPlayStatus.m_mapSkillCoolTime.end(); ++mit )
		{
			int iOK = NetError::ERR_ODBC_01;

			DO_QUERY( L"exec dbo.P_GCoolTime_INS", L"%d, %d, %d, %d",
				% kReq.m_iUnitUID
				% KGamePlayStatus::CTT_SKILL_COOL_TIME
				% mit->first
				% mit->second
				);

			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( iOK );
				m_kODBC.EndFetch();
			}

			if( iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"ĳ���� �÷��� ���� ���� ����!" )
					<< BUILD_LOG( iOK )
					<< BUILD_LOG( kReq.m_iUnitUID )
					<< BUILD_LOG( mit->first )
					<< BUILD_LOG( mit->second )
					<< END_LOG;
			}
		}
	}
#endif	// SERV_CHANGE_SKILL_COOL_TIME_DB_SP

#ifdef	SERV_CHANGE_QUICK_SLOT_COOL_TIME_DB_SP 	// ���볯¥: 2013-06-20
	// ������ ��Ÿ�� ������ ����!
	{
		int arrySlotID[CXSLItem::CIG_MAX] = {-1,};
		int arryCoolTime[CXSLItem::CIG_MAX] = {0,};
		for( int index = 0 ; index < CXSLItem::CIG_MAX ; ++index )
		{
			std::map< int, int >::iterator mit;
			mit = kPlayStatus.m_mapQuickSlotCoolTime.find( index );
			if( mit != kPlayStatus.m_mapQuickSlotCoolTime.end() )
			{
				arrySlotID[index] = mit->first;
				arryCoolTime[index] = mit->second;
			}
			else
			{
				arrySlotID[index] = -1;
				arryCoolTime[index] = 0;
			}
		}

		// 10 ���� ��� ȣ��!
		for( int inum = 0 ; inum < CXSLItem::CIG_MAX ; inum += 10 )
		{
			int arryTempSlotID[10] = {-1,};
			int arryTempCoolTime[10] = {0,};

			arryTempSlotID[0]		= ( inum < CXSLItem::CIG_MAX )		? arrySlotID[inum]		: -1;
			arryTempCoolTime[0]		= ( inum < CXSLItem::CIG_MAX )		? arryCoolTime[inum]	: 0; 
			arryTempSlotID[1]		= ( inum+1 < CXSLItem::CIG_MAX )	? arrySlotID[inum+1]	: -1;
			arryTempCoolTime[1]		= ( inum+1 < CXSLItem::CIG_MAX )	? arryCoolTime[inum+1]	: 0; 
			arryTempSlotID[2]		= ( inum+2 < CXSLItem::CIG_MAX )	? arrySlotID[inum+2]	: -1;
			arryTempCoolTime[2]		= ( inum+2 < CXSLItem::CIG_MAX )	? arryCoolTime[inum+2]	: 0; 
			arryTempSlotID[3]		= ( inum+3 < CXSLItem::CIG_MAX )	? arrySlotID[inum+3]	: -1;
			arryTempCoolTime[3]		= ( inum+3 < CXSLItem::CIG_MAX )	? arryCoolTime[inum+3]	: 0; 
			arryTempSlotID[4]		= ( inum+4 < CXSLItem::CIG_MAX )	? arrySlotID[inum+4]	: -1;
			arryTempCoolTime[4]		= ( inum+4 < CXSLItem::CIG_MAX )	? arryCoolTime[inum+4]	: 0; 
			arryTempSlotID[5]		= ( inum+5 < CXSLItem::CIG_MAX )	? arrySlotID[inum+5]	: -1;
			arryTempCoolTime[5]		= ( inum+5 < CXSLItem::CIG_MAX )	? arryCoolTime[inum+5]	: 0; 
			arryTempSlotID[6]		= ( inum+6 < CXSLItem::CIG_MAX )	? arrySlotID[inum+6]	: -1;
			arryTempCoolTime[6]		= ( inum+6 < CXSLItem::CIG_MAX )	? arryCoolTime[inum+6]	: 0; 
			arryTempSlotID[7]		= ( inum+7 < CXSLItem::CIG_MAX )	? arrySlotID[inum+7]	: -1;
			arryTempCoolTime[7]		= ( inum+7 < CXSLItem::CIG_MAX )	? arryCoolTime[inum+7]	: 0; 
			arryTempSlotID[8]		= ( inum+8 < CXSLItem::CIG_MAX )	? arrySlotID[inum+8]	: -1;
			arryTempCoolTime[8]		= ( inum+8 < CXSLItem::CIG_MAX )	? arryCoolTime[inum+8]	: 0; 
			arryTempSlotID[9]		= ( inum+9 < CXSLItem::CIG_MAX )	? arrySlotID[inum+9]	: -1;
			arryTempCoolTime[9]		= ( inum+9 < CXSLItem::CIG_MAX )	? arryCoolTime[inum+9]	: 0; 

			int iOK = NetError::ERR_ODBC_01;
			DO_QUERY( L"exec dbo.P_GCoolTime_INS_Quick", 
				L"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
				% kReq.m_iUnitUID
				% KGamePlayStatus::CTT_QUICK_SLOT_COOL_TIME
				% arryTempSlotID[0]
				% arryTempCoolTime[0]
				% arryTempSlotID[1]
				% arryTempCoolTime[1]
				% arryTempSlotID[2]
				% arryTempCoolTime[2]
				% arryTempSlotID[3]
				% arryTempCoolTime[3]
				% arryTempSlotID[4]
				% arryTempCoolTime[4]
				% arryTempSlotID[5]
				% arryTempCoolTime[5]
				% arryTempSlotID[6]
				% arryTempCoolTime[6]
				% arryTempSlotID[7]
				% arryTempCoolTime[7]
				% arryTempSlotID[8]
				% arryTempCoolTime[8]
				% arryTempSlotID[9]
				% arryTempCoolTime[9]
				);

				if( m_kODBC.BeginFetch() )
				{
					FETCH_DATA( iOK );
					m_kODBC.EndFetch();
				}

				if( iOK != NetError::NET_OK )
				{
					START_LOG( cerr, L"ĳ���� �� ���� ��Ÿ�� ���� ���� ����!" )
						<< BUILD_LOG( iOK )
						<< BUILD_LOG( kReq.m_iUnitUID )
						<< BUILD_LOG( arryTempSlotID[0] )
						<< BUILD_LOG( arryTempCoolTime[0] )
						<< BUILD_LOG( arryTempSlotID[1] )
						<< BUILD_LOG( arryTempCoolTime[1] )
						<< BUILD_LOG( arryTempSlotID[2] )
						<< BUILD_LOG( arryTempCoolTime[2] )
						<< BUILD_LOG( arryTempSlotID[3] )
						<< BUILD_LOG( arryTempCoolTime[3] )
						<< BUILD_LOG( arryTempSlotID[4] )
						<< BUILD_LOG( arryTempCoolTime[4] )
						<< BUILD_LOG( arryTempSlotID[5] )
						<< BUILD_LOG( arryTempCoolTime[5] )
						<< BUILD_LOG( arryTempSlotID[6] )
						<< BUILD_LOG( arryTempCoolTime[6] )
						<< BUILD_LOG( arryTempSlotID[7] )
						<< BUILD_LOG( arryTempCoolTime[7] )
						<< BUILD_LOG( arryTempSlotID[8] )
						<< BUILD_LOG( arryTempCoolTime[8] )
						<< BUILD_LOG( arryTempSlotID[9] )
						<< BUILD_LOG( arryTempCoolTime[9] )
						<< END_LOG;
				}
		}

	}
#else	// SERV_CHANGE_QUICK_SLOT_COOL_TIME_DB_SP

// ������ ��Ÿ�� ������ ����!
	{
		std::map< int, int >::const_iterator mit;
		for( mit = kPlayStatus.m_mapQuickSlotCoolTime.begin(); mit != kPlayStatus.m_mapQuickSlotCoolTime.end(); ++mit )
		{
			int iOK = NetError::ERR_ODBC_01;

			DO_QUERY( L"exec dbo.P_GCoolTime_INS", L"%d, %d, %d, %d",
				% kReq.m_iUnitUID
				% KGamePlayStatus::CTT_QUICK_SLOT_COOL_TIME
				% mit->first
				% mit->second
				);

			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( iOK );
				m_kODBC.EndFetch();
			}

			if( iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"ĳ���� �÷��� ���� ���� ����!" )
					<< BUILD_LOG( iOK )
					<< BUILD_LOG( kReq.m_iUnitUID )
					<< BUILD_LOG( mit->first )
					<< BUILD_LOG( mit->second )
					<< END_LOG;
			}
		}
	}

#endif	// SERV_CHANGE_QUICK_SLOT_COOL_TIME_DB_SP


#ifdef	SERV_RIDING_PET_SYSTM// ���볯¥: 2013-04-21
	// ������ ��Ÿ�� ������ ����!
	{
		std::map< int, int >::const_iterator it;
		for( it = kPlayStatus.m_mapRidingPetCoolTime.begin(); it != kPlayStatus.m_mapRidingPetCoolTime.end(); ++it )
		{
			int iOK = NetError::ERR_ODBC_01;

			DO_QUERY( L"exec dbo.P_GCoolTime_INS", L"%d, %d, %d, %d",
				% kReq.m_iUnitUID
				% KGamePlayStatus::CTT_RIDING_PET_COOL_TIME
				% it->first
				% it->second
				);

			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( iOK );
				m_kODBC.EndFetch();
			}

			if( iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"ĳ���� �÷��� ���� ���� ����!" )
					<< BUILD_LOG( iOK )
					<< BUILD_LOG( kReq.m_iUnitUID )
					<< BUILD_LOG( it->first )
					<< BUILD_LOG( it->second )
					<< END_LOG;
			}
		}
	}
#endif	// SERV_RIDING_PET_SYSTM

	return true;

end_proc:
	START_LOG( cerr, L"ĳ���� �÷��� ���� ���� ����!" )
		<< BUILD_LOG( kReq.m_iUnitUID )
		<< END_LOG;
	return false;
}
#endif SERV_BATTLE_FIELD_SYSTEM
//}}

//{{ 2012. 05. 6	�ڼ���	����� �� ����
#ifdef SERV_REFORM_THE_GATE_OF_DARKNESS
bool KGSGameDBThread::Query_UpdateBuffEffect( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq )
{	
	BOOST_TEST_FOREACH( const KRecordBuffInfo&, kRecordBuffInfo, kReq.m_vecRecordBuffInfo )
	{
		int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.P_GBuffeffect_UPD", L"%d, %d, N\'%s\'",
			% kReq.m_iUnitUID
			% kRecordBuffInfo.m_iBuffID
			% kRecordBuffInfo.m_wstrStartTime
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

end_proc:
		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"���� ���� ������Ʈ ����!" )
				<< BUILD_LOG( kReq.m_iUnitUID )
				<< BUILD_LOG( kRecordBuffInfo.m_iBuffID )
				<< BUILD_LOG( kRecordBuffInfo.m_wstrStartTime )
				<< END_LOG;
		}
	}

	return true;
}
#endif SERV_REFORM_THE_GATE_OF_DARKNESS
//}}

//{{ 2013. 3. 17	�ڼ���	 ���� ��ŷ �ý���
#ifdef SERV_LOCAL_RANKING_SYSTEM
bool KGSGameDBThread::Query_UpdateLocalRankingInfo( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq, OUT KDBE_UPDATE_UNIT_INFO_ACK& kAck )
{	
	int iOK = NetError::ERR_ODBC_01;

	if( ( 0 < kReq.m_iChangedLocalRankingSpirit ) || ( 0 < kReq.m_iChangedLocalRankingAP ) )
	{
		DO_QUERY( L"exec dbo.P_GFriendSystem_UnitInfo_INS", L"%d, %d, %d",
			% kReq.m_iUnitUID
			% kReq.m_iChangedLocalRankingSpirit
			% kReq.m_iChangedLocalRankingAP
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}
	}

end_proc:
	if( iOK == NetError::NET_OK )
	{
		kAck.m_iChangedLocalRankingSpirit	= 0;
		kAck.m_iChangedLocalRankingAP		= 0;
	}

	return true;
}
#endif SERV_LOCAL_RANKING_SYSTEM
//}}

#ifdef	SERV_RIDING_PET_SYSTM// ���볯¥: 2013-04-21
bool KGSGameDBThread::Query_UpdateRidingPetInfo( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq, OUT KDBE_UPDATE_UNIT_INFO_ACK& kAck )
{
	BOOST_TEST_FOREACH( const KRidingPetInfo&, kInfo, kReq.m_vecRidingPetList )
	{
		int iOK = NetError::ERR_ODBC_01;

		USHORT usStamina = static_cast<USHORT>( kInfo.m_fStamina );
		CTime tLastUnSummonDate( kInfo.m_iLastUnSummonDate );
		std::wstring wstrLastUnSummonDate = tLastUnSummonDate.Format( _T( "%Y-%m-%d %H:%M:%S" ) );

		DO_QUERY( L"exec dbo.P_GRiding_UPD", L"%d, %d, %d, N\'%s\'",
			% kInfo.m_iRidingPetUID
			% kReq.m_iUnitUID
			% usStamina
			% wstrLastUnSummonDate
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK != NetError::NET_OK )
		{
			kAck.m_vecRidingPetUID.push_back( kInfo.m_iRidingPetUID );
		}
	}

end_proc:
	return true;
}
#endif	// SERV_RIDING_PET_SYSTM

#ifdef SERV_GATE_OF_DARKNESS_SUPPORT_EVENT
bool KGSGameDBThread::Query_UpdateGateOfDarknessSupportEventTime( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq )
{
	int iOK = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec dbo.P_GEvent_GateOfDarkness_SET", L"%d, %d", 
		% kReq.m_iUnitUID
		% kReq.m_iGateOfDarknessSupportEventTime
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}

end_proc:
	if( iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"����� �� ���� �̺�Ʈ ���� �ð� ���� ����!" )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< BUILD_LOG( kReq.m_iGateOfDarknessSupportEventTime )
			<< END_LOG;
	}

	return true;
}
#endif SERV_GATE_OF_DARKNESS_SUPPORT_EVENT
//////////////////////////////////////////////////////////////////////////
#endif SERV_DB_UPDATE_UNIT_INFO_REFACTORING
//////////////////////////////////////////////////////////////////////////
//}}


//{{ 2013. 07. 08	������	���� ����
#ifdef SERV_SHARING_BANK_TEST
bool KGSGameDBThread::Query_UpdateShareItem( IN const KDBE_UPDATE_SHARE_ITEM_REQ& kReq, OUT KDBE_UPDATE_SHARE_ITEM_ACK& kAck )
{
	kAck.m_iOK					= NetError::NET_OK;
	kAck.m_bReload				= kReq.m_bReload;
	kAck.m_wstrReloadNickname	= kReq.m_wstrReloadNickname;
	kAck.m_vecInventorySlotItem = kReq.m_vecInventorySlotItem;

	std::map< UidType, KUpdateShareItemInfo >::const_iterator mit;	//	������, ���� ������Ʈ
	KItemPositionUpdate::const_iterator mit2;						//	���� ��ġ����
	std::map< UidType, KTradeShareItemInfo >::const_iterator mit3;	//	In&Out	

	//	1. ������ ������Ʈ
	for( mit = kReq.m_mapUpdateEndurance.begin(); mit != kReq.m_mapUpdateEndurance.end(); ++mit )
	{
		DO_QUERY( L"exec dbo.gup_update_item", L"%d, %d, %d, %d", 
			% mit->second.m_iUnitUID
			% mit->first 
			% mit->second.m_iUsageType
			% mit->second.m_iValue
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kAck.m_iOK );
			m_kODBC.EndFetch();

			if( kAck.m_iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"���� ���� : ������ ���� ������Ʈ ����." )
					<< BUILD_LOG( kAck.m_iOK )
					<< BUILD_LOG( mit->second.m_iUnitUID )
					<< BUILD_LOG( mit->first )
					<< BUILD_LOG( mit->second.m_iValue )
					<< END_LOG;
			}
		}
		else
		{
			START_LOG( cerr, L"���� ���� : ������ ������ ������Ʈ�� BeginFetch ����." );
		}
	}

	//	2. ���� ������Ʈ
	for( mit = kReq.m_mapUpdateQuantity.begin(); mit != kReq.m_mapUpdateQuantity.end(); ++mit )
	{
		DO_QUERY( L"exec dbo.gup_update_item", L"%d, %d, %d, %d", 
			% mit->second.m_iUnitUID
			% mit->first 
			% mit->second.m_iUsageType
			% mit->second.m_iValue
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kAck.m_iOK );
			m_kODBC.EndFetch();

			if( kAck.m_iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"���� ���� : ������ ���� ������Ʈ ����." )
					<< BUILD_LOG( kAck.m_iOK )
					<< BUILD_LOG( mit->second.m_iUnitUID )
					<< BUILD_LOG( mit->first )
					<< BUILD_LOG( mit->second.m_iValue )
					<< END_LOG;

				//{{ 2008. 10. 23  ������	���а� �����ѹ� ��ƺ���~!
				if( kAck.m_iOK == -5 )
				{
					START_LOG( cout, L"���� ���� : ���а� ������Ʈ ���� �߻�!" )
						<< BUILD_LOG( kAck.m_iOK )
						<< BUILD_LOG( mit->second.m_iUnitUID )
						<< BUILD_LOG( mit->first )
						<< BUILD_LOG( mit->second.m_iValue )
						<< END_LOG;

				}
				//}}
			}
		}
		else
		{
			START_LOG( cerr, L"���� ���� : ������ ���� ������Ʈ�� BeginFetch ����." );
		}
	}

	//	3. ���� ��ġ ����
	for( mit2 = kReq.m_mapUpdatePosition.begin(); mit2 != kReq.m_mapUpdatePosition.end(); ++mit2 )
	{
		const UidType iItemUID	= mit2->first;
		const int iCategory		= mit2->second.m_iSlotCategory;
		const int iSlotID		= mit2->second.m_iSlotID;

		DO_QUERY( L"exec dbo.gup_update_item_position", L"%d, %d, %d",
			% iItemUID
			% iCategory
			% iSlotID
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kAck.m_iOK );
			m_kODBC.EndFetch();

			if( kAck.m_iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"���� ���� : ������ ��ġ ������Ʈ ����." )
					<< BUILD_LOG( kAck.m_iOK )
					<< BUILD_LOG( iItemUID )
					<< BUILD_LOG( iCategory )
					<< BUILD_LOG( iSlotID )
					<< END_LOG;
			}
		}
		else
		{
			START_LOG( cerr, L"���� ���� : ������ ��ġ ������Ʈ BeginFetch ����." );
		}
	}

	//	4. �κ��丮 -> ��������
	for( mit3 = kReq.m_mapInItem.begin(); mit3 != kReq.m_mapInItem.end(); ++mit3 )
	{
		KTradeShareItemInfo kShareInfo = mit3->second;

#ifdef SERV_SHARE_BANK_ITEM_EVALUATE_FIX
		DO_QUERY( L"exec dbo.P_GItem_Trade_BankItem_New", L"%d, %d, %d, %d, %d, %d, %d",
			% kShareInfo.m_iFromUnitUID
			% kShareInfo.m_iToUnitUID
			% kShareInfo.m_iItemUID
			% static_cast<int>(kShareInfo.m_cInventoryCategory)
			% static_cast<int>(kShareInfo.m_sSlotID)
			% static_cast<int>(kShareInfo.m_ucSealCnt)
			% static_cast<int>(kShareInfo.m_ucDeleteReason)
			);
#else //SERV_SHARE_BANK_ITEM_EVALUATE_FIX
		DO_QUERY( L"exec dbo.P_GItem_Trade_BankItem", L"%d, %d, %d, %d, %d, %d, %d",
			% kShareInfo.m_iFromUnitUID
			% kShareInfo.m_iToUnitUID
			% kShareInfo.m_iItemUID
			% static_cast<int>(kShareInfo.m_cInventoryCategory)
			% static_cast<int>(kShareInfo.m_sSlotID)
			% static_cast<int>(kShareInfo.m_ucSealCnt)
			% static_cast<int>(kShareInfo.m_ucDeleteReason)
			);
#endif //SERV_SHARE_BANK_ITEM_EVALUATE_FIX

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kAck.m_iOK
				>> kShareInfo.m_iNewItemUID
				>> kShareInfo.m_wstrRegDate );

			m_kODBC.EndFetch();

			if( kAck.m_iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"���� ���� : ������������ ������ �ֱ� ����" )
					<< BUILD_LOG( kAck.m_iOK )
					<< BUILD_LOG( kShareInfo.m_iFromUnitUID )
					<< BUILD_LOG( kShareInfo.m_iToUnitUID )
					<< BUILD_LOG( kShareInfo.m_iItemUID )
					<< BUILD_LOG( kShareInfo.m_iItemID )
					<< BUILD_LOGc( kShareInfo.m_cInventoryCategory)
					<< BUILD_LOG( kShareInfo.m_sSlotID )
					<< BUILD_LOG( kShareInfo.m_ucSealCnt )
					<< BUILD_LOG( kShareInfo.m_ucDeleteReason )
					<< END_LOG;
			}
			else
			{
				kAck.m_vecTradeShareItemResult.push_back( kShareInfo );
			}
		}
		else
		{
			START_LOG( cerr, L"���� ���� : ������������ ������ �ֱ� BeginFetch ����." );
		}
	}

	//	5. �������� -> �κ��丮
	for( mit3 = kReq.m_mapOutItem.begin(); mit3 != kReq.m_mapOutItem.end(); ++mit3 )
	{
		KTradeShareItemInfo kShareInfo = mit3->second;

#ifdef SERV_SHARE_BANK_ITEM_EVALUATE_FIX
		DO_QUERY( L"exec dbo.P_GItem_Trade_BankItem_New", L"%d, %d, %d, %d, %d, %d, %d",
			% kShareInfo.m_iFromUnitUID
			% kShareInfo.m_iToUnitUID
			% kShareInfo.m_iItemUID
			% static_cast<int>(kShareInfo.m_cInventoryCategory)
			% static_cast<int>(kShareInfo.m_sSlotID)
			% static_cast<int>(kShareInfo.m_ucSealCnt)
			% static_cast<int>(kShareInfo.m_ucDeleteReason)
			);
#else //SERV_SHARE_BANK_ITEM_EVALUATE_FIX
		DO_QUERY( L"exec dbo.P_GItem_Trade_BankItem", L"%d, %d, %d, %d, %d, %d, %d",
			% kShareInfo.m_iFromUnitUID
			% kShareInfo.m_iToUnitUID
			% kShareInfo.m_iItemUID
			% static_cast<int>(kShareInfo.m_cInventoryCategory)
			% static_cast<int>(kShareInfo.m_sSlotID)
			% static_cast<int>(kShareInfo.m_ucSealCnt)
			% static_cast<int>(kShareInfo.m_ucDeleteReason)
			);
#endif //SERV_SHARE_BANK_ITEM_EVALUATE_FIX

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kAck.m_iOK
				>> kShareInfo.m_iNewItemUID
				>> kShareInfo.m_wstrRegDate );

			m_kODBC.EndFetch();

			if( kAck.m_iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"���� ���� : �������࿡�� ������ ������ ����" )
					<< BUILD_LOG( kAck.m_iOK )
					<< BUILD_LOG( kShareInfo.m_iFromUnitUID )
					<< BUILD_LOG( kShareInfo.m_iToUnitUID )
					<< BUILD_LOG( kShareInfo.m_iItemUID )
					<< BUILD_LOG( kShareInfo.m_iItemID )
					<< BUILD_LOGc( kShareInfo.m_cInventoryCategory)
					<< BUILD_LOG( kShareInfo.m_sSlotID )
					<< BUILD_LOG( kShareInfo.m_ucSealCnt )
					<< BUILD_LOG( kShareInfo.m_ucDeleteReason )
					<< END_LOG;
			}
			else
			{
				kAck.m_vecTradeShareItemResult.push_back( kShareInfo );
			}
		}
		else
		{
			START_LOG( cerr, L"���� ���� : �������࿡�� ������ ������ BeginFetch ����." );
		}
	}

	//	6. ������ ������ ������Ʈ
	BOOST_TEST_FOREACH( const UidType, iItemUID, kReq.m_vecDeletedItem )
	{
		DO_QUERY( L"exec dbo.gup_delete_item", L"%d, %d", % iItemUID % static_cast<int>(KDeletedItemInfo::DR_ZERO_QUANTITY_SHARE_BANK) );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kAck.m_iOK );
			m_kODBC.EndFetch();

			if( kAck.m_iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"���� ���� : ������ ���� ����." )
					<< BUILD_LOG( kAck.m_iOK )
					<< BUILD_LOG( iItemUID )
					<< END_LOG;
			}
		}
		else
		{
			START_LOG( cerr, L"���� ���� : �������࿡�� ������ ������ BeginFetch ����." );
		}
	}

	return true;

end_proc:
	return false;
}
#endif SERV_SHARING_BANK_TEST

void KGSGameDBThread::SendToServer( unsigned short usEventID )
{
    SendToServer( usEventID, char() );
}

void KGSGameDBThread::SendToUser( UidType nTo, unsigned short usEventID )
{
    SendToUser( nTo, usEventID, char() );
}

//{{ 2012. 10. 23	������		DB������Ʈ �ڵ� �����丵
//////////////////////////////////////////////////////////////////////////
#ifdef SERV_DB_UPDATE_UNIT_INFO_REFACTORING
//////////////////////////////////////////////////////////////////////////

IMPL_ON_FUNC( DBE_UPDATE_UNIT_INFO_REQ )
{
	KDBE_UPDATE_UNIT_INFO_ACK kPacket;
	kPacket.m_iEventID				= kPacket_.m_iEventID;
	kPacket.m_iEXP				    = kPacket_.m_iEXP;
	kPacket.m_iED				    = kPacket_.m_iED;
	//{{ 2011. 07. 11	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
	kPacket.m_iOfficialMatchCnt		= kPacket_.m_iOfficialMatchCnt;
	kPacket.m_iRating				= kPacket_.m_iRating;
	kPacket.m_iMaxRating			= kPacket_.m_iMaxRating;
	kPacket.m_iRPoint				= kPacket_.m_iRPoint;
	kPacket.m_iAPoint				= kPacket_.m_iAPoint;
	kPacket.m_iWin					= kPacket_.m_iWin;
	kPacket.m_iLose					= kPacket_.m_iLose;

	//{{ 2012. 06. 22	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2
	kPacket.m_fKFactor				= kPacket_.m_fKFactor;
#endif SERV_2012_PVP_SEASON2
	//}}

#else
	kPacket.m_iVSPoint			    = kPacket_.m_iVSPoint;
	kPacket.m_iVSPointMax           = kPacket_.m_iVSPointMax;
#endif SERV_PVP_NEW_SYSTEM
	//}}
	std::map< int, KDungeonClearInfo >::iterator mitDungeonClear;
	for( mitDungeonClear = kPacket_.m_mapDungeonClear.begin(); mitDungeonClear != kPacket_.m_mapDungeonClear.end(); mitDungeonClear++ )
	{
		kPacket.m_vecDungeonClear.push_back( mitDungeonClear->first );
	}
	std::map< int, KTCClearInfo >::iterator mitTCClear;
	for( mitTCClear = kPacket_.m_mapTCClear.begin(); mitTCClear != kPacket_.m_mapTCClear.end(); mitTCClear++ )
	{
		kPacket.m_vecTCClear.push_back( mitTCClear->first );
	}
	kPacket.m_kItemQuantityUpdate = kPacket_.m_kItemQuantityUpdate;
	kPacket.m_kItemEnduranceUpdate = kPacket_.m_kItemEnduranceUpdate;
	kPacket.m_kItemPositionUpdate = kPacket_.m_kItemPositionUpdate;
	kPacket.m_iNumResurrectionStone = kPacket_.m_iNumResurrectionStone;
	//{{ 2009. 10. 14  ������	�ڵ����� ��Ȱ��
#ifdef AP_RESTONE
	kPacket.m_iNumAutoPaymentResStone = kPacket_.m_iNumAutoPaymentResStone;
#endif AP_RESTONE
	//}}
	//{{ 2013. 3. 17	�ڼ���	 ���� ��ŷ �ý���
#ifdef SERV_LOCAL_RANKING_SYSTEM
	kPacket.m_iChangedLocalRankingSpirit	= kPacket_.m_iChangedLocalRankingSpirit;
	kPacket.m_iChangedLocalRankingAP		= kPacket_.m_iChangedLocalRankingAP;
#endif SERV_LOCAL_RANKING_SYSTEM
	//}}

	// ĳ���� �⺻ ���� ������Ʈ
	LIF( Query_UpdateUnitInfo( kPacket_, kPacket ) );

	// ĳ���� ���� ���� ������Ʈ
	LIF( Query_UpdatePvpInfo( kPacket_, kPacket ) );

	//{{ 2011. 09. 23	������	�α׿��� ���� EDüũ
#ifdef SERV_LOGOUT_ED_CHECK
	LIF( Query_UpdateLastGamePoint( kPacket_ ) );
#endif SERV_LOGOUT_ED_CHECK
	//}}

	//{{ 2012. 10. 23	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	// ������ ��ġ ���� ����
	LIF( Query_UpdateLastPosition( kPacket_ ) );

	// ĳ���� �÷��� ���� ����
	LIF( Query_UpdateGamePlayStatus( kPacket_ ) );
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}

	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
	LIF( Query_UpdateLastLogOffDate( kPacket_ ) );
#endif SERV_RELATIONSHIP_SYSTEM
	//}

	bool bUpdateFailed = false;
	Query_UpdateDungeonClear( kPacket_.m_iUnitUID, kPacket_.m_mapDungeonClear, kPacket.m_vecDungeonClear );
	Query_UpdateTCClear( kPacket_.m_iUnitUID, kPacket_.m_mapTCClear, kPacket.m_vecTCClear );
#ifdef SERV_LIMITED_DUNGEON_PLAY_TIMES
	Query_UpdateDungeonPlay( kPacket_.m_iUnitUID, kPacket_.m_mapDungeonPlay );
#endif SERV_LIMITED_DUNGEON_PLAY_TIMES
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	LIF( Query_UpdateItemEndurance( kPacket_.m_iUnitUID, kPacket_.m_kItemEnduranceUpdate, kPacket.m_kItemEnduranceUpdate ) );
	Query_UpdateItemPosition( kPacket_.m_iUnitUID, kPacket_.m_kItemPositionUpdate, kPacket.m_kItemPositionUpdate );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_bFinal );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	// ��Ȱ�� ���� ������Ʈ
	LIF( Query_UpdateResurrectionStone( kPacket_, kPacket ) );

	// ����Ʈ ���� ������Ʈ
	LIF( Query_UpdateQuestInfo( kPacket_ ) );

	// Īȣ ���� ������Ʈ
	LIF( Query_UpdateTitleInfo( kPacket_ ) );

	// ��ų ���� ������Ʈ
	LIF( Query_UpdateSkillSlotInfo( kPacket_ ) );

	// Ŀ�´�Ƽ �ɼ� ���� ������Ʈ
	LIF( Query_UpdateCommunityOption( kPacket_ ) );

	// �� ����Ʈ ������Ʈ
	LIF( Query_UpdateWishList( FIRST_SENDER_UID, kPacket_ ) );

	// ��ϸ� �ð� ��ŷ ���� ������Ʈ
	LIF( Query_UpdateRankingInfo( kPacket_ ) );
	
	// ��� ���� ������Ʈ
	LIF( Query_UpdateGuildInfo( kPacket_ ) );

	//{{ 2009. 12. 8  ������	�̺�Ʈ����
#ifdef CUMULATIVE_TIME_EVENT
	// �̺�Ʈ ���� ������Ʈ
	LIF( Query_UpdateConnectTimeEventInfo( FIRST_SENDER_UID, kPacket_ ) );
#endif CUMULATIVE_TIME_EVENT
	//}}
	
	//{{ 2010. 8. 4	������	�� �ý���
#ifdef SERV_PET_SYSTEM
	// �� ���� ������Ʈ
	LIF( Query_UpdatePetInfo( kPacket_, kPacket ) );
#endif SERV_PET_SYSTEM
	//}}

	//{{ 2011. 01. 17	������	ĳ���� ī��Ʈ ����
#ifdef SERV_CHAR_LOG
	// ���� ī��Ʈ ������Ʈ
	LIF( Query_UpdateGameCountInfo( kPacket_ ) );
#endif SERV_CHAR_LOG
	//}}

	//{{ 2011. 08. 12   ��μ�      ��ϸ� ���� 
#ifdef SERV_NEW_HENIR_TEST
	LIF( Query_UpdateHenirRewardCount( kPacket_ ) );
#endif SERV_NEW_HENIR_TEST
	//}}

	//{{ 2012. 05. 6	�ڼ���	����� �� ����
#ifdef SERV_REFORM_THE_GATE_OF_DARKNESS
	LIF( Query_UpdateBuffEffect( kPacket_ ) );
#endif SERV_REFORM_THE_GATE_OF_DARKNESS
	//}}

	//{{ 2013. 3. 17	�ڼ���	 ���� ��ŷ �ý���
#ifdef SERV_LOCAL_RANKING_SYSTEM
	LIF( Query_UpdateLocalRankingInfo( kPacket_, kPacket ) );
#endif SERV_LOCAL_RANKING_SYSTEM
	//}}

#ifdef	SERV_RIDING_PET_SYSTM// ���볯¥: 2013-04-21
	LIF( Query_UpdateRidingPetInfo( kPacket_, kPacket ) );
#endif	// SERV_RIDING_PET_SYSTM

#ifdef SERV_GATE_OF_DARKNESS_SUPPORT_EVENT
	LIF( Query_UpdateGateOfDarknessSupportEventTime( kPacket_ ) );
#endif SERV_GATE_OF_DARKNESS_SUPPORT_EVENT
	if( kPacket_.m_bFinal  &&  kPacket_.m_iEventID == 0 )
		return;

	SendToUser( LAST_SENDER_UID, DBE_UPDATE_UNIT_INFO_ACK, kPacket );
}

//////////////////////////////////////////////////////////////////////////
#else
//////////////////////////////////////////////////////////////////////////

//IMPL_ON_FUNC( DBE_UPDATE_UNIT_INFO_REQ )
//{
//	KDBE_UPDATE_UNIT_INFO_ACK kPacket;
//	//{{ 2009. 3. 17  ������	ä���̵�
//	kPacket.m_iEventID				= kPacket_.m_iEventID;
//	//}}
//	kPacket.m_iEXP				    = kPacket_.m_iEXP;
//	kPacket.m_iED				    = kPacket_.m_iED;
//	//{{ 2011. 07. 11	������	���� ����
//#ifdef SERV_PVP_NEW_SYSTEM
//	kPacket.m_iOfficialMatchCnt		= kPacket_.m_iOfficialMatchCnt;
//	kPacket.m_iRating				= kPacket_.m_iRating;
//	kPacket.m_iMaxRating			= kPacket_.m_iMaxRating;
//	kPacket.m_iRPoint				= kPacket_.m_iRPoint;
//	kPacket.m_iAPoint				= kPacket_.m_iAPoint;
//	kPacket.m_iWin					= kPacket_.m_iWin;
//	kPacket.m_iLose					= kPacket_.m_iLose;
//
//	//{{ 2012. 06. 22	�ڼ���	2012 ���� ����2
//#ifdef SERV_2012_PVP_SEASON2
//	kPacket.m_fKFactor				= kPacket_.m_fKFactor;
//#endif SERV_2012_PVP_SEASON2
//	//}}
//
//#else
//	kPacket.m_iVSPoint			    = kPacket_.m_iVSPoint;
//	kPacket.m_iVSPointMax           = kPacket_.m_iVSPointMax;
//#endif SERV_PVP_NEW_SYSTEM
//	//}}
//	std::map< int, KDungeonClearInfo >::iterator mitDungeonClear;
//	for( mitDungeonClear = kPacket_.m_mapDungeonClear.begin(); mitDungeonClear != kPacket_.m_mapDungeonClear.end(); mitDungeonClear++ )
//	{
//		kPacket.m_vecDungeonClear.push_back( mitDungeonClear->first );
//	}
//	std::map< int, KTCClearInfo >::iterator mitTCClear;
//	for( mitTCClear = kPacket_.m_mapTCClear.begin(); mitTCClear != kPacket_.m_mapTCClear.end(); mitTCClear++ )
//	{
//		kPacket.m_vecTCClear.push_back( mitTCClear->first );
//	}
//	kPacket.m_kItemQuantityUpdate = kPacket_.m_kItemQuantityUpdate;
//	kPacket.m_kItemEnduranceUpdate = kPacket_.m_kItemEnduranceUpdate;
//	kPacket.m_kItemPositionUpdate = kPacket_.m_kItemPositionUpdate;
//	kPacket.m_iNumResurrectionStone = kPacket_.m_iNumResurrectionStone;
//	//{{ 2009. 10. 14  ������	�ڵ����� ��Ȱ��
//#ifdef AP_RESTONE
//	kPacket.m_iNumAutoPaymentResStone = kPacket_.m_iNumAutoPaymentResStone;
//#endif AP_RESTONE
//	//}}	
//
//	int iOK = NetError::ERR_ODBC_01;
//	//{{ 2012. 06. 11	������	��Ʋ�ʵ� �ý���
//#ifdef SERV_BATTLE_FIELD_SYSTEM
//	int iMapIDDummy = 0;
//#endif SERV_BATTLE_FIELD_SYSTEM
//	//}}
//
//	//{{ 2011. 10. 26	������	DB��ŷ Ʈ��
//#ifdef SERV_DB_HACKING_ED_UPDATE_TRAP
//	DO_QUERY( L"exec dbo.gup_update_ui_5", L"%d, %d, %d, %d, %d, %d, %d, %d",
//		% kPacket_.m_iUnitUID
//		% kPacket_.m_iEXP
//		% kPacket_.m_iLevel
//		% kPacket_.m_iED
//		% kPacket_.m_iSPoint		
//		% iMapIDDummy
//		% kPacket_.m_iSpirit
//		% kPacket_.m_bIsSpiritUpdated
//		);
//#else
//	//{{ 2011. 07. 22	������	���� ����
//	//#ifdef SERV_PVP_NEW_SYSTEM
//	DO_QUERY( L"exec dbo.gup_update_unit_info", L"%d, %d, %d, %d, %d, %d, %d, %d",
//		% kPacket_.m_iUnitUID
//		% kPacket_.m_iEXP
//		% kPacket_.m_iLevel
//		% kPacket_.m_iED
//		% kPacket_.m_iSPoint		
//		% kPacket_.m_iMapID
//		% kPacket_.m_iSpirit
//		% kPacket_.m_bIsSpiritUpdated
//		);
//	//#else
//	//	DO_QUERY( L"exec dbo.gup_update_unit_info", L"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
//	//		% kPacket_.m_iUnitUID
//	//		% kPacket_.m_iEXP
//	//		% kPacket_.m_iLevel
//	//		% kPacket_.m_iED
//	//		% kPacket_.m_iVSPoint
//	//		% kPacket_.m_iVSPointMax
//	//		% kPacket_.m_iSPoint
//	//		% kPacket_.m_iWin
//	//		% kPacket_.m_iLose
//	//		% kPacket_.m_iMapID
//	//		% kPacket_.m_iSpirit
//	//		% kPacket_.m_bIsSpiritUpdated
//	//		);
//	//#endif SERV_PVP_NEW_SYSTEM
//	//}}
//#endif SERV_DB_HACKING_ED_UPDATE_TRAP
//	//}}	
//
//	if( m_kODBC.BeginFetch() )
//	{
//		FETCH_DATA( iOK );
//		m_kODBC.EndFetch();
//	}
//
//	if( iOK == NetError::NET_OK )
//	{
//		kPacket.m_iEXP = 0;
//		kPacket.m_iED = 0;
//		//{{ 2011. 07. 22	������	���� ����
//#ifdef SERV_PVP_NEW_SYSTEM
//#else
//		kPacket.m_iVSPoint = 0;
//		kPacket.m_iVSPointMax = 0;
//#endif SERV_PVP_NEW_SYSTEM
//		//}}
//    }
//    else
//    {
//        START_LOG( cerr, L"���� ���� ������Ʈ ����." )
//            << BUILD_LOG( iOK )
//			<< BUILD_LOG( kPacket_.m_iUnitUID )
//			<< BUILD_LOG( kPacket_.m_iEXP )
//			<< BUILD_LOG( kPacket_.m_iLevel )
//			<< BUILD_LOG( kPacket_.m_iED )
//			<< BUILD_LOG( kPacket_.m_iSPoint )
//			<< BUILD_LOG( kPacket_.m_iSpirit )
//			<< BUILD_LOG( kPacket_.m_bIsSpiritUpdated )
//            << END_LOG;
//
//		kPacket.m_iOK = iOK;
//    }
//
//	//////////////////////////////////////////////////////////////////////////
//	//{{ 2012. 02. 02	������	��Ʋ�ʵ� �ý���
//#ifdef SERV_BATTLE_FIELD_SYSTEM
//	DO_QUERY( L"exec dbo.P_GUnitLastPosition_UPT", L"%d, %d, %d, %d",
//		% kPacket_.m_iUnitUID
//		% kPacket_.m_kLastPos.m_iMapID
//		% (int)kPacket_.m_kLastPos.m_ucLastTouchLineIndex
//		% kPacket_.m_kLastPos.m_usLastPosValue
//		);
//
//	if( m_kODBC.BeginFetch() )
//	{
//		FETCH_DATA( iOK );
//		m_kODBC.EndFetch();
//	}
//
//	if( iOK != NetError::NET_OK )
//	{
//		START_LOG( cerr, L"���� ���� ��ġ ���� ������Ʈ ����." )
//			<< BUILD_LOG( iOK )
//			<< BUILD_LOG( kPacket_.m_iUnitUID )
//			<< BUILD_LOG( kPacket_.m_kLastPos.m_iMapID )
//			<< BUILD_LOGc( kPacket_.m_kLastPos.m_ucLastTouchLineIndex )
//			<< BUILD_LOG( kPacket_.m_kLastPos.m_usLastPosValue )
//			<< END_LOG;
//
//		kPacket.m_iOK = iOK;
//	}
//#endif SERV_BATTLE_FIELD_SYSTEM
//	//}}
//	//////////////////////////////////////////////////////////////////////////	
//
//	//{{ 2011. 07. 22	������	���� ����
//#ifdef SERV_PVP_NEW_SYSTEM
//
//	//{{ 2012. 06. 22	�ڼ���	2012 ���� ����2
//#ifdef SERV_2012_PVP_SEASON2
//	DO_QUERY( L"exec dbo.P_GUnitPVP_Season2_UPT",
//		L"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
//		% kPacket_.m_iUnitUID
//		% kPacket_.m_iOfficialMatchCnt
//		% kPacket_.m_iRating
//		% kPacket_.m_iMaxRating
//		% kPacket_.m_iRPoint
//		% kPacket_.m_iAPoint
//		% kPacket_.m_bIsWinBeforeMatch
//		% (int)kPacket_.m_cEmblemEnum
//		% kPacket_.m_iWin
//		% kPacket_.m_iLose
//		% kPacket_.m_fKFactor
//		);
//#else
//	DO_QUERY( L"exec dbo.gup_update_unitpvp"
//		, L"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
//		% kPacket_.m_iUnitUID
//		% kPacket_.m_iOfficialMatchCnt
//		% kPacket_.m_iRating
//		% kPacket_.m_iMaxRating
//		% kPacket_.m_iRPoint
//		% kPacket_.m_iAPoint
//		% kPacket_.m_bIsWinBeforeMatch
//		% (int)kPacket_.m_cEmblemEnum
//		% kPacket_.m_iWin
//		% kPacket_.m_iLose
//		);
//#endif SERV_2012_PVP_SEASON2
//	//}}
//
//	if( m_kODBC.BeginFetch() )
//	{
//		FETCH_DATA( iOK );
//		m_kODBC.EndFetch();
//	}
//
//	if( iOK == NetError::NET_OK )
//	{
//		kPacket.m_iOfficialMatchCnt = 0;
//		kPacket.m_iRating = 0;
//		kPacket.m_iMaxRating = 0;
//		kPacket.m_iRPoint = 0;
//		kPacket.m_iAPoint = 0;
//		kPacket.m_iWin = 0;
//		kPacket.m_iLose = 0;
//		//{{ 2012. 06. 22	�ڼ���	2012 ���� ����2
//#ifdef SERV_2012_PVP_SEASON2
//		kPacket.m_fKFactor = 0.f;
//#endif SERV_2012_PVP_SEASON2
//		//}}
//	}
//	else
//	{
//		START_LOG( cerr, L"���� ���� ���� ������Ʈ ����." )
//			<< BUILD_LOG( iOK )
//			//{{ 2012. 06. 22	�ڼ���	2012 ���� ����2
//#ifdef SERV_2012_PVP_SEASON2
//			<< BUILD_LOG( kPacket_.m_iUnitUID )
//			<< BUILD_LOG( kPacket_.m_iOfficialMatchCnt )
//			<< BUILD_LOG( kPacket_.m_iRating )
//			<< BUILD_LOG( kPacket_.m_iMaxRating )
//			<< BUILD_LOG( kPacket_.m_iRPoint )
//			<< BUILD_LOG( kPacket_.m_iAPoint )
//			<< BUILD_LOG( kPacket_.m_bIsWinBeforeMatch )
//			<< BUILD_LOGc( kPacket_.m_cEmblemEnum )
//			<< BUILD_LOG( kPacket_.m_iWin )
//			<< BUILD_LOG( kPacket_.m_iLose )
//			<< BUILD_LOG( kPacket_.m_fKFactor )
//#endif SERV_2012_PVP_SEASON2
//			//}}
//			<< END_LOG;
//
//		kPacket.m_iOK = iOK;
//	}
//#endif SERV_PVP_NEW_SYSTEM
//	//}}
//
//	//////////////////////////////////////////////////////////////////////////	
//	//{{ 2011. 09. 23	������	�α׿��� ���� EDüũ
//#ifdef SERV_LOGOUT_ED_CHECK
//	// �ش� ĳ������ ������ ��ġ ���� ����!
//	DO_QUERY( L"exec dbo.gup_update_unit_lastpoint", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iRealDataED );
//
//	if( m_kODBC.BeginFetch() )
//	{
//		FETCH_DATA( iOK );
//		m_kODBC.EndFetch();
//	}
//
//	if( iOK != NetError::NET_OK )
//	{
//		START_LOG( cerr, L"�α� ���� EDüũ ���� ������Ʈ ����." )
//			<< BUILD_LOG( iOK )
//			<< BUILD_LOG( kPacket_.m_iUnitUID )
//			<< BUILD_LOG( kPacket_.m_iRealDataED )
//			<< END_LOG;
//
//		kPacket.m_iOK = iOK;
//	}
//
//	// �ش� ĳ������ ������ �÷��� ���°� ����!
//	if( kPacket_.m_kGamePlayStatus.IsEmpty() == false )
//	{
//		KGamePlayStatus kPlayStatus;
//		LIF( kPacket_.m_kGamePlayStatus.Get( kPlayStatus ) );
//
//		// ĳ���� �⺻ ���� ����
//		DO_QUERY( L"exec dbo.P_GUnitPlayInfo_INS", L"%d, %d, %d, %d, %d, %d, %d",
//			% kPacket_.m_iUnitUID
//			% kPlayStatus.m_iMaxHP
//			% kPlayStatus.m_iCurHP
//			% kPlayStatus.m_iMaxMP
//			% kPlayStatus.m_iCurMP
//			% kPlayStatus.m_iCurHyperGage
//			% kPlayStatus.m_iCharAbilCount
//			);
//
//		if( m_kODBC.BeginFetch() )
//		{
//			FETCH_DATA( iOK );
//			m_kODBC.EndFetch();
//		}
//
//		if( iOK != NetError::NET_OK )
//		{
//			START_LOG( cerr, L"ĳ���� �÷��� ���� ���� ����!" )
//				<< BUILD_LOG( iOK )
//				<< BUILD_LOG( kPacket_.m_iUnitUID )
//				<< BUILD_LOG( kPlayStatus.m_iMaxHP )
//				<< BUILD_LOG( kPlayStatus.m_iCurHP )
//				<< BUILD_LOG( kPlayStatus.m_iMaxMP )
//				<< BUILD_LOG( kPlayStatus.m_iCurMP )
//				<< BUILD_LOG( kPlayStatus.m_iCurHyperGage )
//				<< BUILD_LOG( kPlayStatus.m_iCharAbilCount )
//				<< END_LOG;
//
//			kPacket.m_iOK = iOK;
//		}
//
//		// ��ų ��Ÿ�� ������ ����!
//		{
//			std::map< int, int >::const_iterator mit;
//			for( mit = kPlayStatus.m_mapSkillCoolTime.begin(); mit != kPlayStatus.m_mapSkillCoolTime.end(); ++mit )
//			{
//				DO_QUERY( L"exec dbo.P_GCoolTime_INS", L"%d, %d, %d, %d",
//					% kPacket_.m_iUnitUID
//					% KGamePlayStatus::CTT_SKILL_COOL_TIME
//					% mit->first
//					% mit->second
//					);
//
//				if( m_kODBC.BeginFetch() )
//				{
//					FETCH_DATA( iOK );
//					m_kODBC.EndFetch();
//				}
//
//				if( iOK != NetError::NET_OK )
//				{
//					START_LOG( cerr, L"ĳ���� �÷��� ���� ���� ����!" )
//						<< BUILD_LOG( iOK )
//						<< BUILD_LOG( kPacket_.m_iUnitUID )
//						<< BUILD_LOG( mit->first )
//						<< BUILD_LOG( mit->second )
//						<< END_LOG;
//
//					kPacket.m_iOK = iOK;
//				}
//			}
//		}
//
//		// ������ ��Ÿ�� ������ ����!
//		{
//			std::map< int, int >::const_iterator mit;
//			for( mit = kPlayStatus.m_mapQuickSlotCoolTime.begin(); mit != kPlayStatus.m_mapQuickSlotCoolTime.end(); ++mit )
//			{
//				DO_QUERY( L"exec dbo.P_GCoolTime_INS", L"%d, %d, %d, %d",
//					% kPacket_.m_iUnitUID
//					% KGamePlayStatus::CTT_QUICK_SLOT_COOL_TIME
//					% mit->first
//					% mit->second
//					);
//
//				if( m_kODBC.BeginFetch() )
//				{
//					FETCH_DATA( iOK );
//					m_kODBC.EndFetch();
//				}
//
//				if( iOK != NetError::NET_OK )
//				{
//					START_LOG( cerr, L"ĳ���� �÷��� ���� ���� ����!" )
//						<< BUILD_LOG( iOK )
//						<< BUILD_LOG( kPacket_.m_iUnitUID )
//						<< BUILD_LOG( mit->first )
//						<< BUILD_LOG( mit->second )
//						<< END_LOG;
//
//					kPacket.m_iOK = iOK;
//				}
//			}
//		}
//	}
//
//#endif SERV_LOGOUT_ED_CHECK
//	//}}
//	//////////////////////////////////////////////////////////////////////////	
//
//	bool bUpdateFailed = false;
//	Query_UpdateDungeonClear( kPacket_.m_iUnitUID, kPacket_.m_mapDungeonClear, kPacket.m_vecDungeonClear );
//	Query_UpdateTCClear( kPacket_.m_iUnitUID, kPacket_.m_mapTCClear, kPacket.m_vecTCClear );
//	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
//	LIF( Query_UpdateItemEndurance( kPacket_.m_iUnitUID, kPacket_.m_kItemEnduranceUpdate, kPacket.m_kItemEnduranceUpdate ) );
//	Query_UpdateItemPosition( kPacket_.m_iUnitUID, kPacket_.m_kItemPositionUpdate, kPacket.m_kItemPositionUpdate );
//	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );
//
//	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
//	if( bUpdateFailed )
//	{
//		START_LOG( cout, L"���а� ������Ʈ ����!" )
//			<< BUILD_LOG( kPacket_.m_iUnitUID )
//			<< BUILD_LOG( kPacket_.m_bFinal );
//
//		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
//		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
//		{
//			START_LOG( cout, L"������ ����" )
//				<< BUILD_LOG( mitQC->first )
//				<< BUILD_LOG( mitQC->second );
//		}
//	}
//	//}}
//
//	//////////////////////////////////////////////////////////////////////////	
//	// ��Ȱ�� ��ȭ���� ������ ���ν��� ȣ�����.
//	if( kPacket_.m_iNumResurrectionStone != 0 )
//	{
//		DO_QUERY( L"exec dbo.gup_update_resurrection_stone", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iNumResurrectionStone );
//		if( m_kODBC.BeginFetch() )
//		{
//			FETCH_DATA( iOK );
//			m_kODBC.EndFetch();
//		}
//
//		if( iOK == NetError::NET_OK )
//		{
//			kPacket.m_iNumResurrectionStone = 0;
//		}
//		else
//		{
//			START_LOG( cerr, L"��Ȱ�� ������Ʈ ����." )
//				<< BUILD_LOG( iOK )
//				<< END_LOG;
//
//			kPacket.m_iOK = iOK;
//		}
//	}
//
//	//{{ 2009. 10. 14  ������	�ڵ����� ��Ȱ��
//#ifdef AP_RESTONE
//	if( kPacket_.m_iNumAutoPaymentResStone != 0 )
//	{
//		DO_QUERY( L"exec dbo.gup_update_resurrection_stone_autopay", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iNumAutoPaymentResStone );
//		if( m_kODBC.BeginFetch() )
//		{
//			FETCH_DATA( iOK );
//			m_kODBC.EndFetch();
//		}
//
//		if( iOK == NetError::NET_OK )
//		{
//			kPacket.m_iNumAutoPaymentResStone = 0;
//		}
//		else
//		{
//			START_LOG( cerr, L"�ڵ����� ��Ȱ�� ������Ʈ ����." )
//				<< BUILD_LOG( iOK )
//				<< END_LOG;
//
//			kPacket.m_iOK = iOK;
//		}
//	}
//#endif AP_RESTONE
//	//}}
//
//	//QUEST UPDATE
//	for( int i = 0; i < (int)kPacket_.m_vecQuestData.size(); i++ )
//	{
//		KQuestUpdate kQuest = kPacket_.m_vecQuestData[i];
//
//		DO_QUERY( L"exec dbo.gup_update_quest", L"%d, %d, %d, %d, %d, %d, %d",
//			% kPacket_.m_iUnitUID
//			% kQuest.m_iQuestID
//			% (int)kQuest.m_vecClearData[0]
//			% (int)kQuest.m_vecClearData[1]
//			% (int)kQuest.m_vecClearData[2]
//			% (int)kQuest.m_vecClearData[3]
//			% (int)kQuest.m_vecClearData[4]
//			);
//
//		if( m_kODBC.BeginFetch() )
//		{
//			FETCH_DATA( iOK );
//			m_kODBC.EndFetch();
//		}
//
//		if( iOK != NetError::NET_OK )
//		{
//			kPacket.m_iOK = iOK;
//		}
//	}
//
//	//{{ 2010. 02. 09  ������	���� �̺�Ʈ ����Ʈ
//#ifdef SERV_DAY_QUEST
//
//	// ����� ���� ����Ʈ
//	for( u_int ui = 0; ui < kPacket_.m_vecGiveUpQuestList.size(); ++ui )
//	{
//		const int& iGiveUpQuestID = kPacket_.m_vecGiveUpQuestList[ui];
//
//		DO_QUERY( L"exec dbo.gup_delete_quest", L"%d, %d", % kPacket_.m_iUnitUID % iGiveUpQuestID );
//
//		if( m_kODBC.BeginFetch() )
//		{
//			FETCH_DATA( iOK );
//			m_kODBC.EndFetch();
//		}
//
//		if( iOK != NetError::NET_OK )
//		{
//			kPacket.m_iOK = iOK;
//		}
//	}
//
//#endif SERV_DAY_QUEST
//	//}}
//
//	////{{ 2008. 10. 7  ������	Ÿ��Ʋ
//	//TITLE MISSION UPDATE
//	for( u_int i = 0; i < kPacket_.m_vecMissionData.size(); i++ )
//	{
//		KMissionUpdate kMission = kPacket_.m_vecMissionData[i];
//
//		DO_QUERY( L"exec dbo.gup_update_title_mission", L"%d, %d, %d, %d, %d, %d, %d",
//			% kPacket_.m_iUnitUID
//			% kMission.m_iTitleID
//			% kMission.m_vecClearData[0]
//			% kMission.m_vecClearData[1]
//			% kMission.m_vecClearData[2]
//			% kMission.m_vecClearData[3]
//			% kMission.m_vecClearData[4]
//			);
//
//		if( m_kODBC.BeginFetch() )
//		{
//			FETCH_DATA( iOK );
//			m_kODBC.EndFetch();
//		}
//
//		if( iOK != NetError::NET_OK )
//		{
//			START_LOG( cerr, L"�̼� ���� ������Ʈ ����!" )
//				<< BUILD_LOG( iOK )
//				<< BUILD_LOG( kPacket_.m_iUnitUID )
//				<< BUILD_LOG( kMission.m_iTitleID )
//				<< END_LOG;
//
//			kPacket.m_iOK = iOK;
//		}
//	}
//
//	//{{ 2010. 11. 17	������	Īȣ ������ ũ�� ���̱�
//#ifdef SERV_TITLE_DATA_SIZE
//
//	// �������� Ÿ��Ʋ ������Ʈ
//	if( kPacket_.m_iInitTitleID != kPacket_.m_iEquippedTitleID )
//	{
//		DO_QUERY( L"exec dbo.gup_update_title_hang", L"%d, %d, %d",
//			% kPacket_.m_iUnitUID
//			% kPacket_.m_iInitTitleID
//			% kPacket_.m_iEquippedTitleID
//			);
//
//		if( m_kODBC.BeginFetch() )
//		{
//			FETCH_DATA( iOK );
//			m_kODBC.EndFetch();
//		}
//
//		if( iOK != NetError::NET_OK )
//		{
//			START_LOG( cerr, L"Īȣ ���� ���� ������Ʈ ����!" )
//				<< BUILD_LOG( iOK )
//				<< BUILD_LOG( kPacket_.m_iUnitUID )
//				<< BUILD_LOG( kPacket_.m_iInitTitleID )
//				<< BUILD_LOG( kPacket_.m_iEquippedTitleID )
//				<< END_LOG;
//
//			kPacket.m_iOK = iOK;
//		}
//	}
//
//#else
//
//	// �������� Ÿ��Ʋ ������Ʈ
//	if( kPacket_.m_sInitTitleID != kPacket_.m_sEquippedTitleID )
//	{
//		DO_QUERY( L"exec dbo.gup_update_title_hang", L"%d, %d, %d",
//			% kPacket_.m_iUnitUID
//			% kPacket_.m_sInitTitleID
//			% kPacket_.m_sEquippedTitleID
//			);
//
//		if( m_kODBC.BeginFetch() )
//		{
//			FETCH_DATA( iOK );
//			m_kODBC.EndFetch();
//		}
//
//		if( iOK != NetError::NET_OK )
//		{
//			START_LOG( cerr, L"Īȣ ���� ���� ������Ʈ ����!" )
//				<< BUILD_LOG( iOK )
//				<< BUILD_LOG( kPacket_.m_iUnitUID )
//				<< BUILD_LOG( kPacket_.m_sInitTitleID )
//				<< BUILD_LOG( kPacket_.m_sEquippedTitleID )
//				<< END_LOG;
//
//			kPacket.m_iOK = iOK;
//		}
//	}
//
//#endif SERV_TITLE_DATA_SIZE
//	//}}
//
//
//	//}}
//
//	//SKILL SLOT UPDATE
//	DO_QUERY( L"exec dbo.gup_insert_skill_slot_new", L"%d, %d, %d, %d, %d",
//		% kPacket_.m_iUnitUID
//		% kPacket_.m_vecSkillSlot[0]
//		% kPacket_.m_vecSkillSlot[1]
//		% kPacket_.m_vecSkillSlot[2]
//		% kPacket_.m_vecSkillSlot[3]
//		);
//
//	if( m_kODBC.BeginFetch() )
//	{
//		FETCH_DATA( iOK );
//		m_kODBC.EndFetch();
//	}
//
//	if( iOK != NetError::NET_OK )
//	{
//		kPacket.m_iOK = iOK;
//	}
//
//	//SKILL SLOT B UPDATE
//	if( kPacket_.m_vecSkillSlot.size() >= 8 )
//	{
//		// [���� ����]
//		DO_QUERY( L"exec dbo.gup_insert_skill_slot2_new", L"%d, %d, %d, %d, %d",
//			% kPacket_.m_iUnitUID
//			% kPacket_.m_vecSkillSlot[4]
//			% kPacket_.m_vecSkillSlot[5]
//			% kPacket_.m_vecSkillSlot[6]
//			% kPacket_.m_vecSkillSlot[7]
//			);
//
//		if( m_kODBC.BeginFetch() )
//		{
//			FETCH_DATA( iOK );
//			m_kODBC.EndFetch();
//		}
//
//		if( iOK != NetError::NET_OK )
//		{
//			kPacket.m_iOK = iOK;
//		}
//	}
//
//	//COMMUNITY OPTION UPDATE
//	DO_QUERY( L"exec dbo.gup_update_community_opt", L"%d, %d, %d, %d, %d",
//		% kPacket_.m_iUnitUID
//		% (int)kPacket_.m_kDenyOptions.m_cDenyFriendShip
//		% (int)kPacket_.m_kDenyOptions.m_cDenyInviteGuild
//		% (int)kPacket_.m_kDenyOptions.m_cDenyParty
//		% (int)kPacket_.m_kDenyOptions.m_cDenyPersonalTrade
//		);
//
//	if( m_kODBC.BeginFetch() )
//	{
//		FETCH_DATA( iOK );
//		m_kODBC.EndFetch();
//	}
//
//	if( iOK != NetError::NET_OK )
//	{
//		kPacket.m_iOK = iOK;
//	}
//
//	//{{ 2008. 5. 5  ������  ��ٱ��� ������Ʈ
//	{
//		std::map< int, int >::const_iterator mitWishList;
//		for( mitWishList = kPacket_.m_mapWishList.begin(); mitWishList != kPacket_.m_mapWishList.end(); ++mitWishList )
//		{
//			DO_QUERY( L"exec dbo.gup_update_WishList", L"%d, %d, %d", % FIRST_SENDER_UID % mitWishList->first % mitWishList->second );
//
//			if( m_kODBC.BeginFetch() )
//			{
//				FETCH_DATA( iOK );
//				m_kODBC.EndFetch();
//			}
//
//			if( iOK != NetError::NET_OK )
//			{
//				START_LOG( cerr, L"��ٱ��� DB������Ʈ ����!" )
//					<< BUILD_LOG( iOK )
//					<< BUILD_LOG( FIRST_SENDER_UID )
//					<< BUILD_LOG( mitWishList->first )
//					<< BUILD_LOG( mitWishList->second )
//					<< END_LOG;
//
//				// [����] �����ϴ��� ���� �ѹ��� �ʿ�� ����. �߿䵵�� ���� ����..
//				kPacket.m_iOK = iOK;
//			}
//		}
//	}
//	//}}
//
//	//{{ 2009. 7. 7  ������		��ŷ ����
//	{
//		std::vector< KHenirRankingInfo >::const_iterator vit = kPacket_.m_vecHenirRanking.begin();
//		if( vit != kPacket_.m_vecHenirRanking.end() )
//		{
//			const KHenirRankingInfo& kRankingInfo = *vit;
//
//			// RegDate ��ȯ
//			CTime tRegDate = CTime( kRankingInfo.m_tRegDate );
//			std::wstring wstrRegDate = ( CStringW )( tRegDate.Format( _T( "%Y-%m-%d %H:%M:%S" ) ) );
//
//			DO_QUERY( L"exec dbo.gup_insert_rank_myrecord", L"%d, %d, %d, %d, %d, N\'%s\'",
//				% kPacket_.m_iUnitUID
//				% kRankingInfo.m_iStageCount
//				% kRankingInfo.m_ulPlayTime
//				% (int)kRankingInfo.m_ucLevel
//				% (int)kRankingInfo.m_cUnitClass
//				% wstrRegDate
//				);
//
//			if( m_kODBC.BeginFetch() )
//			{
//				FETCH_DATA( iOK );
//				m_kODBC.EndFetch();
//			}
//
//			if( iOK != NetError::NET_OK )
//			{
//				START_LOG( cerr, L"�����̵� ��ŷ DB������Ʈ ����!" )
//					<< BUILD_LOG( iOK )
//					<< BUILD_LOG( kPacket_.m_iUnitUID )
//					<< BUILD_LOG( kRankingInfo.m_iStageCount )
//					<< BUILD_LOG( kRankingInfo.m_ulPlayTime )
//					<< BUILD_LOG( (int)kRankingInfo.m_ucLevel )
//					<< BUILD_LOG( (int)kRankingInfo.m_cUnitClass )
//					<< BUILD_LOG( wstrRegDate )
//					<< END_LOG;
//
//				// ������������ ���� ����ó����?
//				kPacket.m_iOK = iOK;
//			}
//		}
//	}
//	//}}
//
//	//{{ 2009. 10. 7  ������	���
//#ifdef GUILD_TEST
//
//	// ��� ���� ������Ʈ
//	if( kPacket_.m_iGuildUID > 0 )
//	{
//		// ������ ������Ʈ��� �α׾ƿ� ó��
//		if( kPacket_.m_bFinal )
//		{
//			DO_QUERY( L"exec dbo.gup_update_guild_lastdate", L"%d, %d",
//				% kPacket_.m_iUnitUID
//				% kPacket_.m_iGuildUID
//				);
//
//			if( m_kODBC.BeginFetch() )
//			{
//				FETCH_DATA( iOK );
//				m_kODBC.EndFetch();
//			}
//
//			if( iOK != NetError::NET_OK )
//			{
//				kPacket.m_iOK = iOK;
//
//				START_LOG( cerr, L"��� �α׾ƿ� ó�� ����!" )
//					<< BUILD_LOG( iOK )
//					<< BUILD_LOG( kPacket_.m_iUnitUID )
//					<< BUILD_LOG( kPacket_.m_iGuildUID )
//					<< END_LOG;
//			}
//		}
//
//		//////////////////////////////////////////////////////////////////////////
//		//{{ 2009. 10. 28  ������	��巹��
//		DO_QUERY( L"exec dbo.gup_update_guild_member_exp", L"%d, %d, %d",
//			% kPacket_.m_iUnitUID
//			% kPacket_.m_iGuildUID
//			% kPacket_.m_iHonorPoint
//			);
//
//		if( m_kODBC.BeginFetch() )
//		{
//			FETCH_DATA( iOK );
//			m_kODBC.EndFetch();
//		}
//
//		if( iOK != NetError::NET_OK )
//		{
//			kPacket.m_iOK = iOK;
//
//			START_LOG( cerr, L"���� ���� ����Ʈ ������Ʈ ����." )
//				<< BUILD_LOG( iOK )
//				<< BUILD_LOG( kPacket_.m_iUnitUID )
//				<< BUILD_LOG( kPacket_.m_iGuildUID )
//				<< BUILD_LOG( kPacket_.m_iHonorPoint )
//				<< END_LOG;
//		}
//		//}}
//		//////////////////////////////////////////////////////////////////////////		
//	}
//
//#endif GUILD_TEST
//	//}}
//
//	//{{ 2009. 12. 8  ������	�̺�Ʈ����
//#ifdef CUMULATIVE_TIME_EVENT
//
//	{
//		std::vector< KCumulativeTimeEventInfo >::const_iterator vitCT;
//		for( vitCT = kPacket_.m_vecUpdateEventTime.begin(); vitCT != kPacket_.m_vecUpdateEventTime.end(); ++vitCT )
//		{
//			//{{ 2010. 06. 15  ������	�������� ���ӽð� �̺�Ʈ
//#ifdef SERV_ACC_TIME_EVENT
//			if( vitCT->m_bAccountEvent )
//			{
//				DO_QUERY( L"exec dbo.gup_update_remaintime_account", L"%d, %d, %d", % LAST_SENDER_UID % vitCT->m_iEventUID % vitCT->m_iCumulativeTime );
//
//				if( m_kODBC.BeginFetch() )
//				{
//					FETCH_DATA( iOK );
//					m_kODBC.EndFetch();
//				}
//
//				if( iOK != NetError::NET_OK )
//				{
//					kPacket.m_iOK = iOK;
//
//					START_LOG( cerr, L"�̺�Ʈ Ÿ�� DB������Ʈ ����?" )
//						<< BUILD_LOG( iOK )
//						<< BUILD_LOG( LAST_SENDER_UID )
//						<< BUILD_LOG( vitCT->m_iEventUID )
//						<< BUILD_LOG( vitCT->m_iCumulativeTime )
//						<< END_LOG;
//				}
//			}
//			else
//#endif SERV_ACC_TIME_EVENT
//			//}}
//			{
//				DO_QUERY( L"exec dbo.gup_update_remaintime", L"%d, %d, %d", % kPacket_.m_iUnitUID % vitCT->m_iEventUID % vitCT->m_iCumulativeTime );
//
//				if( m_kODBC.BeginFetch() )
//				{
//					FETCH_DATA( iOK );
//					m_kODBC.EndFetch();
//				}
//
//				if( iOK != NetError::NET_OK )
//				{
//					kPacket.m_iOK = iOK;
//
//					START_LOG( cerr, L"�̺�Ʈ Ÿ�� DB������Ʈ ����?" )
//						<< BUILD_LOG( iOK )
//						<< BUILD_LOG( kPacket_.m_iUnitUID )
//						<< BUILD_LOG( vitCT->m_iEventUID )
//						<< BUILD_LOG( vitCT->m_iCumulativeTime )
//						<< END_LOG;
//				}
//			}
//		}
//	}	
//
//#endif CUMULATIVE_TIME_EVENT
//	//}}
//
//	//{{ 2010. 8. 4	������	�� �ý���
//#ifdef SERV_PET_SYSTEM
//
//	// �� ���� ������Ʈ	
//	BOOST_TEST_FOREACH( const KPetInfo&, kPet, kPacket_.m_vecPet )
//	{
//		DO_QUERY( L"exec dbo.gup_update_pet_info", L"%d, %d, %d, %d, %d, %d, %d, N\'%s\', N\'%s\'", 
//			% kPet.m_iPetUID
//			% (int)kPet.m_cEvolutionStep
//			% kPet.m_sSatiety
//			% kPet.m_iIntimacy
//			% kPet.m_sExtroversion
//			% kPet.m_sEmotion
//			% kPet.m_bAutoFeed
//			% kPet.m_wstrLastFeedDate
//			% kPet.m_wstrLastSummonDate
//			);
//
//		if( m_kODBC.BeginFetch() )
//		{
//			FETCH_DATA( iOK );
//			m_kODBC.EndFetch();
//		}
//
//		if( iOK != NetError::NET_OK )
//		{
//			kPacket.m_iOK = iOK;
//
//			START_LOG( cerr, L"�� ���� ������Ʈ ����!" )
//				<< BUILD_LOG( kPacket.m_iOK )
//				<< BUILD_LOG( kPet.m_iPetUID )
//				<< BUILD_LOG( kPet.m_sSatiety )
//				<< BUILD_LOG( kPet.m_iIntimacy )
//				<< BUILD_LOG( kPet.m_sExtroversion )
//				<< BUILD_LOG( kPet.m_sEmotion )
//				<< BUILD_LOG( kPet.m_bAutoFeed )
//				<< BUILD_LOG( kPet.m_wstrLastFeedDate )
//				<< BUILD_LOG( kPet.m_wstrLastSummonDate )
//				<< END_LOG;
//
//			// DB������Ʈ�� ���� �Ѵٸ� �ѹ� �����͸� ����!
//			kPacket.m_vecPet.push_back( kPet );
//		}
//	}
//
//#endif SERV_PET_SYSTEM
//	//}}
//
//	//{{ 2011. 01. 17	������	ĳ���� ī��Ʈ ����
//#ifdef SERV_CHAR_LOG
//	{
//		std::map< int, int >::const_iterator mitCGC;
//		for( mitCGC = kPacket_.m_mapCharGameCount.begin(); mitCGC != kPacket_.m_mapCharGameCount.end(); ++mitCGC )
//		{
//			DO_QUERY( L"exec dbo.gup_update_Character_Count", L"%d, %d, %d", 
//				% kPacket_.m_iUnitUID
//				% mitCGC->first
//				% mitCGC->second
//				);
//
//			if( m_kODBC.BeginFetch() )
//			{
//				FETCH_DATA( iOK );
//				m_kODBC.EndFetch();
//			}
//
//			if( iOK != NetError::NET_OK )
//			{
//				kPacket.m_iOK = iOK;
//
//				START_LOG( cerr, L"ĳ���� ���� ī��Ʈ ���� ������Ʈ!" )
//					<< BUILD_LOG( kPacket.m_iOK )
//					<< BUILD_LOG( kPacket_.m_iUnitUID )
//					<< BUILD_LOG( mitCGC->first )
//					<< BUILD_LOG( mitCGC->second )
//					<< END_LOG;
//			}
//		}
//	}
//#endif SERV_CHAR_LOG
//	//}}
//	//{{ 2011. 08. 12   ��μ�      ��ϸ� ���� 
//#ifdef SERV_NEW_HENIR_TEST
//	DO_QUERY( L"exec dbo.gup_update_henir_reward_cnt", L"%d, %d, %d, %d", 
//		% kPacket_.m_iUnitUID 
//		% kPacket_.m_kHenirRewardCnt.m_iNormal 
//		% kPacket_.m_kHenirRewardCnt.m_iPremium 
//		% kPacket_.m_kHenirRewardCnt.m_iEvent 
//		);
//
//	if( m_kODBC.BeginFetch() )
//	{
//		FETCH_DATA( iOK );
//		m_kODBC.EndFetch();
//	}
//
//	if( iOK != NetError::NET_OK )
//	{
//		kPacket.m_iOK = iOK;
//
//		START_LOG( cerr, L"ĳ���� ���� ī��Ʈ ���� ������Ʈ!" )
//			<< BUILD_LOG( kPacket.m_iOK )
//			<< BUILD_LOG( kPacket_.m_iUnitUID )
//			<< BUILD_LOG( kPacket_.m_kHenirRewardCnt.m_iNormal )
//			<< BUILD_LOG( kPacket_.m_kHenirRewardCnt.m_iPremium )
//			<< BUILD_LOG( kPacket_.m_kHenirRewardCnt.m_iEvent )
//			<< END_LOG;
//	}
//#endif SERV_NEW_HENIR_TEST
//	//}}
//
//	//{{ 2012. 05. 6	�ڼ���	����� �� ����
//#ifdef SERV_REFORM_THE_GATE_OF_DARKNESS
//	BOOST_TEST_FOREACH( const KRecordBuffInfo&, kRecordBuffInfo, kPacket_.m_vecRecordBuffInfo )
//	{
//		DO_QUERY( L"exec dbo.P_GBuffeffect_UPD", L"%d, %d, N\'%s\'",
//			% kPacket_.m_iUnitUID
//			% kRecordBuffInfo.m_iBuffID
//			% kRecordBuffInfo.m_wstrStartTime
//			);
//
//		if( m_kODBC.BeginFetch() )
//		{
//			FETCH_DATA( iOK );
//			m_kODBC.EndFetch();
//		}
//
//		if( iOK != NetError::NET_OK )
//		{
//			START_LOG( cerr, L"���� ���� ������Ʈ ����!" )
//				<< BUILD_LOG( kPacket_.m_iUnitUID )
//				<< BUILD_LOG( kRecordBuffInfo.m_iBuffID )
//				<< BUILD_LOG( kRecordBuffInfo.m_wstrStartTime )
//				<< END_LOG;
//		}
//	}
//#endif SERV_REFORM_THE_GATE_OF_DARKNESS
//	//}}
//
//end_proc:
//
//    if( kPacket_.m_bFinal  &&  kPacket_.m_iEventID == 0 )
//		return;
//
//	SendToUser( LAST_SENDER_UID, DBE_UPDATE_UNIT_INFO_ACK, kPacket );
//}

//////////////////////////////////////////////////////////////////////////
#endif SERV_DB_UPDATE_UNIT_INFO_REFACTORING
//////////////////////////////////////////////////////////////////////////
//}}

//{{ 2012. 04. 05	�ڼ���	( ���� ���� ǥ�� )
#ifdef SERV_EVENT_RETURN_USER_MARK_SCRIPT
_IMPL_ON_FUNC( DBE_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ, KDBE_GET_SECOND_SECURITY_INFO_REQ_FOR_GameDB )
#else
//{{ 2012. 03. 27	�ڼ���	�Ƹ����� ���� ������ ���� ����! ( ���� ���� ǥ�� )
#ifdef SERV_EVENT_RETURN_USER_MARK
_IMPL_ON_FUNC( DBE_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ, KDBE_GET_SECOND_SECURITY_INFO_REQ_FOR_GameDB )
#else
_IMPL_ON_FUNC( DBE_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ, std::wstring )
#endif SERV_EVENT_RETURN_USER_MARK
//}}
#endif SERV_EVENT_RETURN_USER_MARK_SCRIPT
//}}
{
	KEGS_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

#ifdef SERV_UNIT_WAIT_DELETE //2012.03.07 lygan_������ // ĳ���� ������ �������� ���ڿ��� �ƴ� ���ڷ� ��ȯ�ϱ� ����
	std::wstring wstrDelAbleDate  = L"";
	std::wstring wstrRestoreAbleDate  = L"";
	CTime tDelAbleDateTime;
	CTime tRestoreAbleDateTime;
#endif SERV_UNIT_WAIT_DELETE
	std::vector< KUnitInfo >::iterator vit;

	//{{ 2012. 04. 12	�ڼ���	( ���� ���� ǥ�� )
#ifdef SERV_EVENT_RETURN_USER_MARK_SCRIPT

	kPacket.m_wstrLastConnectDate = kPacket_.m_wstrLastConnectDate;

	DO_QUERY( L"exec dbo.P_GCriterion_Event_SEL", L"%d", % kPacket_.m_iUserUID );
	while( m_kODBC.Fetch() )
	{
		int iCriterionEventUID;

		FETCH_DATA( iCriterionEventUID );

		kPacket.m_vecCriterionEventUID.push_back( iCriterionEventUID );
	}

	DO_QUERY( L"exec dbo.gup_get_user_info", L"%d, N\'%s\'", % LAST_SENDER_UID % kPacket_.m_wstrName.c_str() );
#else
		//{{ 2012. 03. 27	�ڼ���	�Ƹ����� ���� ������ ���� ����! ( ���� ���� ǥ�� )
	#ifdef SERV_EVENT_RETURN_USER_MARK
		kPacket.m_bEventMark = kPacket_.m_bEventMark;
		DO_QUERY( L"exec dbo.gup_get_user_info", L"%d, N\'%s\'", % LAST_SENDER_UID % kPacket_.m_wstrName.c_str() );
	#else
		DO_QUERY( L"exec dbo.gup_get_user_info", L"%d, N\'%s\'", % LAST_SENDER_UID % kPacket_.c_str() );
	#endif SERV_EVENT_RETURN_USER_MARK
		//}}
#endif SERV_EVENT_RETURN_USER_MARK_SCRIPT
		//}}
	
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_nUnitSlot );

		m_kODBC.EndFetch();
	}

	//{{ 2012. 09. 11	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2
	DO_QUERY( L"exec dbo.P_GUnit_Userinfo_GET", L"%d", % LAST_SENDER_UID );
#else
	DO_QUERY( L"exec dbo.gup_get_unit_info_by_useruid", L"%d", % LAST_SENDER_UID );
#endif SERV_2012_PVP_SEASON2
	//}}
    while( m_kODBC.Fetch() )
    {
        KUnitInfo kUnitInfo;
		//{{ 2012. 06. 11	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
		int iMapIDDummy = 0;
#endif SERV_BATTLE_FIELD_SYSTEM
		//}}

		//{{ 2011. 07. 11	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
		//{{ 2012. 09. 11	�ڼ���	2012 ���� ����2
#ifdef SERV_2012_PVP_SEASON2		
		
		//{{ 2012.02.20 ��ȿ��	ĳ���� ���� ���μ��� ���� (���� ��� �Ⱓ ����)
#ifdef SERV_UNIT_WAIT_DELETE
		FETCH_DATA( kUnitInfo.m_nUnitUID
			>> kUnitInfo.m_cUnitClass
			>> kUnitInfo.m_iEXP
			>> kUnitInfo.m_ucLevel
			>> kUnitInfo.m_iED
			>> kUnitInfo.m_iOfficialMatchCnt
			>> kUnitInfo.m_cRank
			>> kUnitInfo.m_kStat.m_iBaseHP
			>> kUnitInfo.m_kStat.m_iAtkPhysic
			>> kUnitInfo.m_kStat.m_iAtkMagic
			>> kUnitInfo.m_kStat.m_iDefPhysic
			>> kUnitInfo.m_kStat.m_iDefMagic
			>> kUnitInfo.m_iSPoint
			>> kUnitInfo.m_wstrNickName
			>> iMapIDDummy
			>> kUnitInfo.m_wstrLastDate
			>> kUnitInfo.m_bDeleted
			>> wstrDelAbleDate
			>> wstrRestoreAbleDate
			);
#else SERV_UNIT_WAIT_DELETE
		FETCH_DATA( kUnitInfo.m_nUnitUID
			>> kUnitInfo.m_cUnitClass
			>> kUnitInfo.m_iEXP
			>> kUnitInfo.m_ucLevel
			>> kUnitInfo.m_iED
			>> kUnitInfo.m_iOfficialMatchCnt
			>> kUnitInfo.m_cRank
			>> kUnitInfo.m_kStat.m_iBaseHP
			>> kUnitInfo.m_kStat.m_iAtkPhysic
			>> kUnitInfo.m_kStat.m_iAtkMagic
			>> kUnitInfo.m_kStat.m_iDefPhysic
			>> kUnitInfo.m_kStat.m_iDefMagic
			>> kUnitInfo.m_iSPoint
			>> kUnitInfo.m_wstrNickName
			>> iMapIDDummy
		);
#endif SERV_UNIT_WAIT_DELETE
		//}}		
		
		// GetPvpRankForClient()
		// ĳ���� ����Ʈ�� �о� ������ ������ ���� ������ ä���� ���� �ʰ� DB���� �ٷ� ��ũ ������ �о�� ������
		// ��ġ ��Ŀ�� ���� ��ȯ ������ �̰����� �ٷ� �����Ͽ���. ( �о ��ũ ������ ������ ��ϵ��� �ʴ´� )
		if( kUnitInfo.m_iOfficialMatchCnt < 10 )
		{
			kUnitInfo.m_cRank = static_cast<char>( CXSLUnit::PVPRANK_RANK_ARRANGE );
		}
#else
		//{{ 2012.02.20 ��ȿ��	ĳ���� ���� ���μ��� ���� (���� ��� �Ⱓ ����)
#ifdef SERV_UNIT_WAIT_DELETE
		FETCH_DATA( kUnitInfo.m_nUnitUID
			>> kUnitInfo.m_cUnitClass
			>> kUnitInfo.m_iEXP
			>> kUnitInfo.m_ucLevel
			>> kUnitInfo.m_iED
			>> kUnitInfo.m_iRating
			>> kUnitInfo.m_kStat.m_iBaseHP
			>> kUnitInfo.m_kStat.m_iAtkPhysic
			>> kUnitInfo.m_kStat.m_iAtkMagic
			>> kUnitInfo.m_kStat.m_iDefPhysic
			>> kUnitInfo.m_kStat.m_iDefMagic
			>> kUnitInfo.m_iSPoint
			>> kUnitInfo.m_wstrNickName
			>> iMapIDDummy
			>> kUnitInfo.m_wstrLastDate
			>> kUnitInfo.m_bDeleted
			>> wstrDelAbleDate
			>> wstrRestoreAbleDate
			);
#else SERV_UNIT_WAIT_DELETE
		FETCH_DATA( kUnitInfo.m_nUnitUID
			>> kUnitInfo.m_cUnitClass
			>> kUnitInfo.m_iEXP
			>> kUnitInfo.m_ucLevel
			>> kUnitInfo.m_iED
			>> kUnitInfo.m_iRating
			>> kUnitInfo.m_kStat.m_iBaseHP
			>> kUnitInfo.m_kStat.m_iAtkPhysic
			>> kUnitInfo.m_kStat.m_iAtkMagic
			>> kUnitInfo.m_kStat.m_iDefPhysic
			>> kUnitInfo.m_kStat.m_iDefMagic
			>> kUnitInfo.m_iSPoint
			>> kUnitInfo.m_wstrNickName
			>> iMapIDDummy
			);
#endif SERV_UNIT_WAIT_DELETE
		//}}
#endif SERV_2012_PVP_SEASON2
//}}
#else
		FETCH_DATA( kUnitInfo.m_nUnitUID
			>> kUnitInfo.m_cUnitClass
			>> kUnitInfo.m_iEXP
			>> kUnitInfo.m_ucLevel
			>> kUnitInfo.m_iED
			>> kUnitInfo.m_iVSPoint
			>> kUnitInfo.m_iVSPointMax
			>> kUnitInfo.m_kStat.m_iBaseHP
			>> kUnitInfo.m_kStat.m_iAtkPhysic
			>> kUnitInfo.m_kStat.m_iAtkMagic
			>> kUnitInfo.m_kStat.m_iDefPhysic
			>> kUnitInfo.m_kStat.m_iDefMagic
			>> kUnitInfo.m_iSPoint
			>> kUnitInfo.m_wstrNickName
			>> kUnitInfo.m_iWin
			>> kUnitInfo.m_iLose
			>> kUnitInfo.m_nMapID
			);
#endif SERV_PVP_NEW_SYSTEM
		//}}        

#ifdef SERV_UNIT_WAIT_DELETE
		//{{ // 2012.03.06 lygan_������ // ���� ���ڿ��ð��� �״�� �޴� �κ��� �������� ���ڷ� �����ؼ� Ŭ��� ���� ������ ����
		if( KncUtil::ConvertStringToCTime( wstrDelAbleDate, tDelAbleDateTime ) == true )
			kUnitInfo.m_trDelAbleDate = tDelAbleDateTime.GetTime();
		else
			kUnitInfo.m_trDelAbleDate = 0LL;

		//{{ // 2012.03.06 lygan_������ // ���� ���ڿ��ð��� �״�� �޴� �κ��� �������� ���ڷ� �����ؼ� Ŭ��� ���� ������ ����
		if( KncUtil::ConvertStringToCTime( wstrRestoreAbleDate, tRestoreAbleDateTime ) == true )
			kUnitInfo.m_trRestoreAbleDate = tRestoreAbleDateTime.GetTime();
		else
			kUnitInfo.m_trRestoreAbleDate = 0LL;
		//}}
#endif SERV_UNIT_WAIT_DELETE

		//////////////////////////////////////////////////////////////////////////
		if( CXSLUnit::IsValidUnitClass( static_cast<CXSLUnit::UNIT_CLASS>(kUnitInfo.m_cUnitClass) ) == false )
		{
			START_LOG( cout, L"[�˸�] ��ϵ��� ���� ���� Ŭ�����Դϴ�!" )
				<< BUILD_LOG( kUnitInfo.m_nUnitUID )
				<< BUILD_LOG( kUnitInfo.m_wstrNickName )
				<< BUILD_LOGc( kUnitInfo.m_cUnitClass );
			continue;
		}
		
		//////////////////////////////////////////////////////////////////////////		

		kPacket.m_vecUnitInfo.push_back( kUnitInfo );
	}

	// ���� ������ ����
	for( vit = kPacket.m_vecUnitInfo.begin(); vit != kPacket.m_vecUnitInfo.end(); ++vit )
	{
		std::map< UidType, int > mapItemUIDSlot;
		mapItemUIDSlot.clear();

		//{{ 2013. 05. 24	������	������ ����
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
		// ���� ���� ���� ���
		std::map< UidType, std::vector< int > > mapRandomSocketList;

		DO_QUERY( L"exec dbo.P_GItemSocket_Random_SEL", L"%d", % vit->m_nUnitUID );
		while( m_kODBC.Fetch() )
		{
			UidType iItemUID = 0;
			int arrRandomSocketOption[CXSLSocketItem::RSC_MAX] = {0,};

			FETCH_DATA( iItemUID
				>> arrRandomSocketOption[0]
				>> arrRandomSocketOption[1]
				>> arrRandomSocketOption[2]
				>> arrRandomSocketOption[3]
				>> arrRandomSocketOption[4]
				);

				// 2-1. ���� ���� ���� ������Ʈ
				//{{ Iruha : 2026-08-27 // VS2010 port: iCheckIdx is used after the loop, which
				// VC7.1's non-conformant /Zc:forScope- tolerated; VC10 requires the loop
				// variable to outlive the loop, so it's hoisted here.
				int iCheckIdx;
				for( iCheckIdx = CXSLSocketItem::RSC_MAX - 1; iCheckIdx >= 0; --iCheckIdx )
				{
					if( arrRandomSocketOption[iCheckIdx] != 0 )
						break;
				}
				//}}

				std::vector< int > vecRandomSocket;
				for( int iIdx = 0; iIdx <= iCheckIdx; ++iIdx )
				{
					vecRandomSocket.push_back( arrRandomSocketOption[iIdx] );
				}

				// itemuid���� �ߺ����� �ʴ°��� db���� �����ؾ��Ѵ�.
				mapRandomSocketList.insert( std::make_pair( iItemUID, vecRandomSocket ) );
		}
#endif SERV_NEW_ITEM_SYSTEM_2013_05
		//}}

		// ���� �������� ��´�.
		DO_QUERY( L"exec dbo.gup_get_equipped_item_list", L"%d", % vit->m_nUnitUID );
		while( m_kODBC.Fetch() )
		{
			int iEnchantLevel = 0;
			//{{ 2011. 07. 25    ��μ�    ������ �ɼ�ID ������ ������ ����
//#ifdef SERV_ITEM_OPTION_DATA_SIZE
			int arrSocketOption[4] = {0,0,0,0};
//#else
//			short arrSocketOption[4] = {0,0,0,0};
//#endif SERV_ITEM_OPTION_DATA_SIZE
			//}} 
			KInventoryItemInfo kInventoryItemInfo;

			//{{ 2011. 12. 15	������	�κ��丮 ����IDũ�� �ø���
//#ifdef SERV_EXPAND_SLOT_ID_DATA_SIZE
			FETCH_DATA( kInventoryItemInfo.m_iItemUID
				>> kInventoryItemInfo.m_kItemInfo.m_iItemID
				>> kInventoryItemInfo.m_kItemInfo.m_cUsageType
				>> kInventoryItemInfo.m_kItemInfo.m_iQuantity
				>> kInventoryItemInfo.m_kItemInfo.m_sEndurance
				>> kInventoryItemInfo.m_kItemInfo.m_sPeriod
				>> kInventoryItemInfo.m_kItemInfo.m_wstrExpirationDate
				>> iEnchantLevel
				>> arrSocketOption[0]
				>> arrSocketOption[1]
				>> arrSocketOption[2]
				>> arrSocketOption[3]
				>> kInventoryItemInfo.m_cSlotCategory
				>> kInventoryItemInfo.m_sSlotID );
//#else
//			FETCH_DATA( kInventoryItemInfo.m_iItemUID
//				>> kInventoryItemInfo.m_kItemInfo.m_iItemID
//				>> kInventoryItemInfo.m_kItemInfo.m_cUsageType
//				>> kInventoryItemInfo.m_kItemInfo.m_iQuantity
//				>> kInventoryItemInfo.m_kItemInfo.m_sEndurance
//				>> kInventoryItemInfo.m_kItemInfo.m_sPeriod
//				>> kInventoryItemInfo.m_kItemInfo.m_wstrExpirationDate
//				>> iEnchantLevel
//				>> arrSocketOption[0]
//				>> arrSocketOption[1]
//				>> arrSocketOption[2]
//				>> arrSocketOption[3]
//				>> kInventoryItemInfo.m_cSlotCategory
//				>> kInventoryItemInfo.m_cSlotID );
//#endif SERV_EXPAND_SLOT_ID_DATA_SIZE
			//}}

			//{{ 2008. 2. 20  ������  ��ȭ
			kInventoryItemInfo.m_kItemInfo.m_cEnchantLevel = static_cast<char>(iEnchantLevel);
			//}}

			//{{ 2008. 3. 7  ������  ����
			//{{ Iruha : 2026-08-27 // VS2010 port: iCheckIdx is used after the loop, which
			// VC7.1's non-conformant /Zc:forScope- tolerated; VC10 requires the loop
			// variable to outlive the loop, so it's hoisted here.
			int iCheckIdx;
			for( iCheckIdx = 3; iCheckIdx >= 0; --iCheckIdx )
			{
				if( arrSocketOption[iCheckIdx] != 0 )
					break;
			}
			//}}

			for( int iIdx = 0; iIdx <= iCheckIdx; ++iIdx )
			{
				kInventoryItemInfo.m_kItemInfo.m_vecItemSocket.push_back( arrSocketOption[iIdx] );
			}
			//}}

			//{{ 2013. 05. 21	������	������ ����
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
			// 2-1. ���� ���� ���� ������Ʈ
			std::map< UidType, std::vector< int > >::const_iterator mitCRS;
			mitCRS = mapRandomSocketList.find( kInventoryItemInfo.m_iItemUID );
			if( mitCRS != mapRandomSocketList.end() )
			{
				kInventoryItemInfo.m_kItemInfo.m_vecRandomSocket = mitCRS->second;
			}
#endif SERV_NEW_ITEM_SYSTEM_2013_05
			//}}

            std::map< int, KInventoryItemInfo >::iterator mit;
			//{{ 2011. 12. 15	������	�κ��丮 ����IDũ�� �ø���
//#ifdef SERV_EXPAND_SLOT_ID_DATA_SIZE
			mit = vit->m_mapEquippedItem.find( kInventoryItemInfo.m_sSlotID );
//#else
//			mit = vit->m_mapEquippedItem.find( kInventoryItemInfo.m_cSlotID );
//#endif SERV_EXPAND_SLOT_ID_DATA_SIZE
//			//}}
            if( mit != vit->m_mapEquippedItem.end() )
            {
                START_LOG( cerr, L"���� ������ ������ �ߺ���." )
                    << BUILD_LOG( kInventoryItemInfo.m_iItemUID )
					//{{ 2011. 12. 15	������	�κ��丮 ����IDũ�� �ø���
//#ifdef SERV_EXPAND_SLOT_ID_DATA_SIZE
					<< BUILD_LOG( kInventoryItemInfo.m_sSlotID )
//#else
//					<< BUILD_LOGc( kInventoryItemInfo.m_cSlotID )
//#endif SERV_EXPAND_SLOT_ID_DATA_SIZE
					//}}
                    << BUILD_LOG( kInventoryItemInfo.m_kItemInfo.m_iItemID )
                    << END_LOG;
            }
			//{{ 2008. 9. 26  ������	�Ӽ���ȭ ĳ���� ����Ʈ ǥ��
			//{{ 2011. 12. 15	������	�κ��丮 ����IDũ�� �ø���
//#ifdef SERV_EXPAND_SLOT_ID_DATA_SIZE
			vit->m_mapEquippedItem.insert( std::make_pair( kInventoryItemInfo.m_sSlotID, kInventoryItemInfo ) );

			mapItemUIDSlot.insert( std::make_pair( kInventoryItemInfo.m_iItemUID, kInventoryItemInfo.m_sSlotID ) );
//#else
//			vit->m_mapEquippedItem.insert( std::make_pair( kInventoryItemInfo.m_cSlotID, kInventoryItemInfo ) );
//
//			mapItemUIDSlot.insert( std::make_pair( kInventoryItemInfo.m_iItemUID, kInventoryItemInfo.m_cSlotID ) );
//#endif SERV_EXPAND_SLOT_ID_DATA_SIZE
			//}}
        }

		//////////////////////////////////////////////////////////////////////////
		// �Ӽ� ��ȭ ���
		DO_QUERY( L"exec dbo.gup_get_attribute", L"%d", % vit->m_nUnitUID );
		while( m_kODBC.Fetch() )
		{
			UidType iItemUID = 0;
			int iAttribEnchantSlotNo = 0;
			int iAttribEnchantID = 0;

			FETCH_DATA( iItemUID
				>> iAttribEnchantSlotNo
				>> iAttribEnchantID );

			std::map< UidType, int >::const_iterator mitSlot;
			mitSlot = mapItemUIDSlot.find( iItemUID );
			if( mitSlot == mapItemUIDSlot.end() )
				continue;            

			std::map< int, KInventoryItemInfo >::iterator mitAtt;
			mitAtt = vit->m_mapEquippedItem.find( mitSlot->second );
			if( mitAtt == vit->m_mapEquippedItem.end() )
				continue;

			// �Ӽ� ��ȭ
			switch( iAttribEnchantSlotNo )
			{
			case CXSLAttribEnchantItem::ESI_SLOT_1:
				mitAtt->second.m_kItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant0 = static_cast<char>( iAttribEnchantID );
				break;

			case CXSLAttribEnchantItem::ESI_SLOT_2:
				mitAtt->second.m_kItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant1 = static_cast<char>( iAttribEnchantID );
				break;

			case CXSLAttribEnchantItem::ESI_SLOT_3:
				mitAtt->second.m_kItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant2 = static_cast<char>( iAttribEnchantID );
				break;

			default:
				START_LOG( cerr, L"�Ӽ� ��ȭ ���� �ѹ��� �̻��մϴ�." )
					<< BUILD_LOG( iItemUID )
					<< BUILD_LOG( iAttribEnchantSlotNo )
					<< BUILD_LOG( iAttribEnchantID )
					<< END_LOG;
				break;
			}
		}
		//////////////////////////////////////////////////////////////////////////

		//{{ 2009. 9. 25  ������	���
#ifdef GUILD_TEST

		// ��� ���� ���
		DO_QUERY( L"exec dbo.gup_get_my_guild_info", L"%d", % vit->m_nUnitUID );
		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( vit->m_kUserGuildInfo.m_iGuildUID
				>> vit->m_kUserGuildInfo.m_wstrGuildName );

			m_kODBC.EndFetch();
		}

#endif GUILD_TEST
		//}}
    }

#ifdef SERV_SHARING_BANK_QUEST_CASH
	// ���� : ĳ���� �Ⱥ��̵��� �ϱ� ���ؼ�, ���� ���� �ߴ��� üũ�Ѵ�.

	//1. ���� ���� Ȯ�� SP  ȣ��
	DO_QUERY( L"exec dbo.P_GBankShare_CHK", L"%d",
		% LAST_SENDER_UID
		);

	while( m_kODBC.Fetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
	}

	//2. ���� ���� ������ ��Ŷ�� ����.
	if(kPacket.m_iOK != NetError::NET_OK)
	{
		START_LOG(clog2, L"����α�:��������� Ȱ��ȭ �Ǿ� ���� �ʴ�.")
			<< BUILD_LOG(LAST_SENDER_UID)
			<< BUILD_LOG(kPacket.m_iOK)
			<< END_LOG;

		kPacket.m_bSharingBank = false;
	}
	else
	{
		kPacket.m_bSharingBank = true;
	}
#endif

#ifdef SERV_JUMPING_CHARACTER// �۾���¥: 2013-07-12	// �ڼ���
	{
		KDBE_JUMPING_CHARACTER_INFO_NOT kPacket;

		DO_QUERY( L"exec dbo.P_GEvent_Jumping_SEL", L"%d", % LAST_SENDER_UID );

		while( m_kODBC.Fetch() )
		{
			UidType			iUnitUID;
			std::wstring	wstrRegDate;

			FETCH_DATA( iUnitUID
				>> wstrRegDate
				);

			kPacket.m_mapJumpingInfo.insert( std::map< UidType, std::wstring >::value_type( iUnitUID, wstrRegDate ) );
		}

		SendToUser( LAST_SENDER_UID, DBE_JUMPING_CHARACTER_INFO_NOT, kPacket );
	}
#endif // SERV_JUMPING_CHARACTER

	kPacket.m_iOK = NetError::NET_OK;

end_proc:
    LOG_SUCCESS( kPacket.m_iOK == NetError::NET_OK )
        << BUILD_LOG( LAST_SENDER_UID );

    SendToUser( LAST_SENDER_UID, DBE_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK, kPacket );
}

//{{ 2011. 08. 09  ��μ� (2011.08.11) Ư���� ���� ������ ������ ���Ͽ� �ű��ɸ��� ���� �� ������ ���� �̺�Ʈ
#ifdef SERV_NEW_CREATE_CHAR_EVENT
IMPL_ON_FUNC( DBE_GAME_CREATE_UNIT_REQ )
#else
_IMPL_ON_FUNC( DBE_GAME_CREATE_UNIT_REQ, KEGS_CREATE_UNIT_REQ )
#endif SERV_NEW_CREATE_CHAR_EVENT
//}}
{
    KEGS_CREATE_UNIT_ACK kPacket;
    kPacket.m_iOK = NetError::ERR_ODBC_01;

    START_LOG( clog, L"���� ����" )
        << BUILD_LOG( kPacket_.m_wstrNickName )
        << BUILD_LOG( kPacket_.m_iClass );

#ifdef SERV_NICK_NAME_DOUBLE_CHECK
	if( kPacket_.m_wstrNickName.size() > 1 )
	{
		for ( int i = 0; i < (int)kPacket_.m_wstrNickName.size(); i++ )
		{
			WCHAR tempChar = kPacket_.m_wstrNickName[i];

			if ( !((tempChar >= 'a' && tempChar <= 'z') || (tempChar >= 'A' && tempChar <= 'Z') || (tempChar >= '0' && tempChar <= '9')))
			{
				START_LOG( cerr, L"[ERROR] �г��ӿ� �߸��� ���� ����!" )
					<< BUILD_LOG( kPacket_.m_wstrNickName )
					<< END_LOG;
			
				kPacket.m_iOK = NetError::ERR_CREATE_UNIT_08;
				kPacket.m_kUnitInfo.m_wstrNickName = kPacket_.m_wstrNickName;
				kPacket.m_kUnitInfo.m_cUnitClass = kPacket_.m_iClass;
				goto end_proc;
			}
		}
	}
	else
	{
		START_LOG( cerr, L"[ERROR] �г��� ���̰� �߸��Ǿ���!" )
			<< BUILD_LOG( kPacket_.m_wstrNickName )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_CREATE_UNIT_04;
		kPacket.m_kUnitInfo.m_wstrNickName = kPacket_.m_wstrNickName;
		kPacket.m_kUnitInfo.m_cUnitClass = kPacket_.m_iClass;
		goto end_proc;
	}
#endif //SERV_NICK_NAME_DOUBLE_CHECK

    DO_QUERY( L"exec dbo.gup_create_unit", L"%d, N\'%s\', %d",
				% LAST_SENDER_UID
				% kPacket_.m_wstrNickName
				% kPacket_.m_iClass
				// ������ �ʴ� ���� �б⸸ �ϹǷ� �� �����忡�� �����ص� ��������..
				//% GetKGameServer()->GetServerGroupID()
				);


    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( kPacket.m_iOK
                 >> kPacket.m_kUnitInfo.m_nUnitUID
				 //{{ 2008. 4. 8  ������  ������ �г��� ��� ���� ��¥
				 >> kPacket.m_wstrEnableDate
				 //}}
				 );

        m_kODBC.EndFetch();
    }	

    if( kPacket.m_iOK != NetError::NET_OK )
    {
        switch( kPacket.m_iOK )
        {
        case -1:    kPacket.m_iOK = NetError::ERR_CREATE_UNIT_00;   break;
        case -2:    kPacket.m_iOK = NetError::ERR_CREATE_UNIT_01;   break;
        case -3:    kPacket.m_iOK = NetError::ERR_CREATE_UNIT_05;   break;
        case -10:   kPacket.m_iOK = NetError::ERR_CREATE_UNIT_02;   break;
		case -222:  kPacket.m_iOK = NetError::ERR_CREATE_UNIT_06;	break;
		case -23:   kPacket.m_iOK = NetError::ERR_CREATE_UNIT_09;	break;
		default:
			START_LOG( cerr, L"gup_create_unit: ���ǵ��� ���� ������Դϴ�." )
				<< BUILD_LOG( kPacket.m_iOK )
				<< END_LOG;
			break;
        }

#ifdef SERV_GUARANTEE_UNIQUENESS_OF_NAME_CN
		kPacket.m_bReqGiant_DeleteName = true;
		kPacket.m_wstrNickName = kPacket_.m_wstrNickName;
#endif //SERV_GUARANTEE_UNIQUENESS_OF_NAME_CN

        goto end_proc;
    }

    kPacket.m_iOK = NetError::ERR_ODBC_01;

    DO_QUERY( L"exec dbo.gup_get_unit_info_by_unituid", L"%d", % kPacket.m_kUnitInfo.m_nUnitUID );
    if( m_kODBC.BeginFetch() )
    {
		bool bIsSpiritUpdated = false;
		UidType iRecommendUnitUID = 0;
		//{{ 2012. 06. 11	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
		int iMapIDDummy = 0;
#endif SERV_BATTLE_FIELD_SYSTEM
		//}}
		//{{ 2011. 07. 11	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
		FETCH_DATA( kPacket.m_kUnitInfo.m_nUnitUID
			>> kPacket.m_kUnitInfo.m_cUnitClass
			>> kPacket.m_kUnitInfo.m_iEXP
			>> kPacket.m_kUnitInfo.m_ucLevel
			>> kPacket.m_kUnitInfo.m_iED
			>> kPacket.m_kUnitInfo.m_kStat.m_iBaseHP
			>> kPacket.m_kUnitInfo.m_kStat.m_iAtkPhysic
			>> kPacket.m_kUnitInfo.m_kStat.m_iAtkMagic
			>> kPacket.m_kUnitInfo.m_kStat.m_iDefPhysic
			>> kPacket.m_kUnitInfo.m_kStat.m_iDefMagic
			>> kPacket.m_kUnitInfo.m_iSPoint
			>> kPacket.m_kUnitInfo.m_wstrNickName
			>> iMapIDDummy
			>> kPacket.m_kUnitInfo.m_iSpirit
			>> bIsSpiritUpdated
			>> iRecommendUnitUID
			);
#else
		FETCH_DATA( kPacket.m_kUnitInfo.m_nUnitUID
			>> kPacket.m_kUnitInfo.m_cUnitClass
			>> kPacket.m_kUnitInfo.m_iEXP
			>> kPacket.m_kUnitInfo.m_ucLevel
			>> kPacket.m_kUnitInfo.m_iED
			>> kPacket.m_kUnitInfo.m_iVSPoint
			>> kPacket.m_kUnitInfo.m_iVSPointMax
			>> kPacket.m_kUnitInfo.m_kStat.m_iBaseHP
			>> kPacket.m_kUnitInfo.m_kStat.m_iAtkPhysic
			>> kPacket.m_kUnitInfo.m_kStat.m_iAtkMagic
			>> kPacket.m_kUnitInfo.m_kStat.m_iDefPhysic
			>> kPacket.m_kUnitInfo.m_kStat.m_iDefMagic
			>> kPacket.m_kUnitInfo.m_iSPoint
			>> kPacket.m_kUnitInfo.m_wstrNickName
			>> kPacket.m_kUnitInfo.m_iWin
			>> kPacket.m_kUnitInfo.m_iLose
			>> kPacket.m_kUnitInfo.m_nMapID
			);
#endif SERV_PVP_NEW_SYSTEM
		//}}        

        m_kODBC.EndFetch();

		kPacket.m_iOK = NetError::NET_OK;
    }
	else
	{
		kPacket.m_iOK = NetError::ERR_CREATE_UNIT_03;

		goto end_proc;
	}

#ifdef SERV_2012_PVP_SEASON2// �۾���¥: 2013-07-01	// �ڼ���
	DO_QUERY( L"exec dbo.P_GUnitPVP_Season2_GET", L"%d", % kPacket.m_kUnitInfo.m_nUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_kUnitInfo.m_iOfficialMatchCnt
			>> kPacket.m_kUnitInfo.m_iRating
			>> kPacket.m_kUnitInfo.m_iMaxRating
			>> kPacket.m_kUnitInfo.m_iRPoint
			>> kPacket.m_kUnitInfo.m_iAPoint
			>> kPacket.m_kUnitInfo.m_bIsWinBeforeMatch
			>> kPacket.m_kUnitInfo.m_cRank
			>> kPacket.m_kUnitInfo.m_iWin
			>> kPacket.m_kUnitInfo.m_iLose
			>> kPacket.m_kUnitInfo.m_fKFactor
			>> kPacket.m_kUnitInfo.m_bIsRedistributionUser
			>> kPacket.m_kUnitInfo.m_iPastSeasonWin
			);
		m_kODBC.EndFetch();
	}
	else
	{
		START_LOG( cerr, L"P_GUnitPVP_Season2_GET ȣ�� ����!" )
			<< BUILD_LOG( kPacket.m_kUnitInfo.m_nUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_CREATE_UNIT_03;
		goto end_proc;
	}
#endif // SERV_2012_PVP_SEASON2

	//{{ 2009. 3. 31  ������	ĳ���� ������ ���θ�� ������
	int iOK = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec dbo.gup_create_unit_set_promotion", L"%d", % kPacket.m_kUnitInfo.m_nUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK	);
		m_kODBC.EndFetch();
	}
	else
	{
		START_LOG( cerr, L"ĳ���� ������ �������� ���� ����!" )
			<< BUILD_LOG( kPacket.m_kUnitInfo.m_nUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_CREATE_UNIT_03;
		goto end_proc;
	}

	if( iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"ĳ���� ������ �������� ���� ����!" )
			<< BUILD_LOG( kPacket.m_kUnitInfo.m_nUnitUID )
			<< BUILD_LOG( iOK )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_CREATE_UNIT_03;
		goto end_proc;
	}
	//}}

	//////////////////////////////////////////////////////////////////////////	
	//{{ 2011. 08. 09  ��μ� (2011.08.11) Ư���� ���� ������ ������ ���Ͽ� �ű��ɸ��� ���� �� ������ ���� �̺�Ʈ
//#ifdef SERV_NEW_CREATE_CHAR_EVENT
//	DO_QUERY( L"exec dbo.gup_insert_event_unit", L"%d, %d, N\'%s\'", % LAST_SENDER_UID % kPacket.m_kUnitInfo.m_nUnitUID % kPacket_.m_wstrRegDate );
//	if( m_kODBC.BeginFetch() )
//	{
//		FETCH_DATA( iOK );
//		m_kODBC.EndFetch();
//	}
//
//	switch( iOK )
//	{
//	case 0:
//		{
//			START_LOG( clog, L"ĳ���� ������ ������ ���� ����!!" )
//				<< BUILD_LOG( iOK )
//				<< BUILD_LOG( LAST_SENDER_UID )
//				<< BUILD_LOG( kPacket.m_kUnitInfo.m_nUnitUID )
//				<< BUILD_LOG( kPacket_.m_wstrRegDate )
//				<< END_LOG;
//
//		}break;
//	case -1:
//		{
//			START_LOG( cerr, L"���� �������� üũ�ߴµ� �ű԰� �ƴ� ��(�ѹ�ƴ�/�׳� ����)" )
//				<< BUILD_LOG( iOK )
//				<< BUILD_LOG( LAST_SENDER_UID )
//				<< BUILD_LOG( kPacket.m_kUnitInfo.m_nUnitUID )
//				<< BUILD_LOG( kPacket_.m_wstrRegDate )
//				<< END_LOG;
//		}break;
//	case -2:
//		{
//			START_LOG( cerr, L"ť�� �������� �Ⱥ���������" )
//				<< BUILD_LOG( iOK )
//				<< BUILD_LOG( LAST_SENDER_UID )
//				<< BUILD_LOG( kPacket.m_kUnitInfo.m_nUnitUID )
//				<< BUILD_LOG( kPacket_.m_wstrRegDate )
//				<< END_LOG;
//		}break;
//	case -3:
//		{
//			START_LOG( cerr, L"Īȣ ���� �ȵ�����" )
//				<< BUILD_LOG( iOK )
//				<< BUILD_LOG( LAST_SENDER_UID )
//				<< BUILD_LOG( kPacket.m_kUnitInfo.m_nUnitUID )
//				<< BUILD_LOG( kPacket_.m_wstrRegDate )
//				<< END_LOG;
//		}break;
//	case -4:
//		{
//			START_LOG( cerr, L"ť�� Īȣ ���� ĳ�� ��� ���������� " )
//				<< BUILD_LOG( iOK )
//				<< BUILD_LOG( LAST_SENDER_UID )
//				<< BUILD_LOG( kPacket.m_kUnitInfo.m_nUnitUID )
//				<< BUILD_LOG( kPacket_.m_wstrRegDate )
//				<< END_LOG;
//		}break;
//	default:
//		{
//			START_LOG( cerr, L"���� ���� ���Դϴ�." )
//				<< BUILD_LOG( iOK )
//				<< BUILD_LOG( LAST_SENDER_UID )
//				<< BUILD_LOG( kPacket.m_kUnitInfo.m_nUnitUID )
//				<< BUILD_LOG( kPacket_.m_wstrRegDate )
//				<< END_LOG;
//		}break;
//	}
//#endif SERV_NEW_CREATE_CHAR_EVENT
	//}}
	//////////////////////////////////////////////////////////////////////////

end_proc:

    SendToUser( LAST_SENDER_UID, DBE_GAME_CREATE_UNIT_ACK, kPacket );
}

_IMPL_ON_FUNC( DBE_GAME_DELETE_UNIT_REQ, KEGS_DELETE_UNIT_REQ )
{
    KEGS_DELETE_UNIT_ACK kPacket;
    kPacket.m_iOK = NetError::ERR_ODBC_01;
    kPacket.m_iUnitUID = kPacket_.m_iUnitUID;
#ifdef GUILD_TEST
	kPacket.m_iGuildUID = 0;	
	std::wstring wstrGuildName;
#endif GUILD_TEST

	//{{ 2012.02.20 ��ȿ��	ĳ���� ���� ���μ��� ���� (���� ��� �Ⱓ ����)
#ifdef SERV_UNIT_WAIT_DELETE
	//{{	//2012.03.05 lygan_������ // ĳ���� ���� �Ҷ� ���� ������ ������ �� �˾� ���� ����
	std::wstring wstrDelDate  = L"";
	std::wstring wstrDelAbleDate  = L"";
	std::wstring wstrRestoreAbleDate  = L"";
	std::wstring wstrReDelAbleDate = L"";
	CTime tReDelAbleDateTime;
	CTime tDelAbleDateTime;

	//}}//2012.03.05 lygan_������ // ĳ���� ���� �Ҷ� ���� ������ ������ �� �˾� ���� ����
	DO_QUERY( L"exec dbo.P_GUnitRestore_ReDelAbleDate_CHK", L"%d, %d",	% kPacket_.m_iUnitUID
		% kPacket_.m_iUserUID);
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK 
			>> wstrReDelAbleDate
			);

		m_kODBC.EndFetch();
	}
	else
	{
		START_LOG( cerr, L"ĳ���� ���� ���� ���� üũ ��� ���� �� ����??" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iUserUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_DELETE_UNIT_00;

		goto end_proc;
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"ĳ���� ���� ���� ���� üũ ����!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iUserUID )
			<< END_LOG;

		if ( kPacket.m_iOK == -1 )
		{
			kPacket.m_iOK = NetError::ERR_DELETE_UNIT_00;
		}
		else if( kPacket.m_iOK == -22 )
		{
			kPacket.m_iOK = NetError::ERR_DELETE_UNIT_02;

			//{{ // 2012.03.06 lygan_������ // ���� ���ڿ��ð��� �״�� �޴� �κ��� �������� ���ڷ� �����ؼ� Ŭ��� ���� ������ ����
			if( KncUtil::ConvertStringToCTime( wstrReDelAbleDate, tReDelAbleDateTime ) == true )
				kPacket.m_tReDelAbleDate = tReDelAbleDateTime.GetTime();
			else
				kPacket.m_tReDelAbleDate = 0LL;
			//}}
		}
		else
		{
			kPacket.m_iOK = NetError::ERR_UNKNOWN;
		}

		goto end_proc;
	}


#endif SERV_UNIT_WAIT_DELETE
	//}}

#ifdef SERV_RECRUIT_EVENT_BASE
	DO_QUERY( L"exec dbo.gup_delete_recommend", L"%d", % kPacket_.m_iUnitUID );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );

		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"��õ�� ���̺����� ĳ���� ���� ����!" )
			<< BUILD_LOG( NetError::ERR_RECOMMEND_USER_06 )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;
	}
#endif SERV_RECRUIT_EVENT_BASE

	//{{ 2011. 02. 23	������	ĳ���� �α�
#ifdef SERV_CHAR_LOG
	// ĳ���� ������ ��´�.
	DO_QUERY( L"exec dbo.gup_get_unit_info_by_unituid", L"%d", % kPacket_.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		KUnitInfo kUnitInfo;
		bool bIsSpiritUpdated = false;
		UidType iRecommendUnitUID = 0;
		//{{ 2012. 06. 11	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
		int iMapIDDummy = 0;
#endif SERV_BATTLE_FIELD_SYSTEM
		//}}

		//{{ 2011. 07. 11	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
		FETCH_DATA( kUnitInfo.m_nUnitUID
			>> kPacket.m_cUnitClass
			>> kUnitInfo.m_iEXP
			>> kPacket.m_ucLevel
			>> kUnitInfo.m_iED
			>> kUnitInfo.m_kStat.m_iBaseHP
			>> kUnitInfo.m_kStat.m_iAtkPhysic
			>> kUnitInfo.m_kStat.m_iAtkMagic
			>> kUnitInfo.m_kStat.m_iDefPhysic
			>> kUnitInfo.m_kStat.m_iDefMagic
			>> kUnitInfo.m_iSPoint
			>> kPacket.m_wstrNickName
			>> iMapIDDummy
			>> kUnitInfo.m_iSpirit
			>> bIsSpiritUpdated
			>> iRecommendUnitUID
			);
#else
		FETCH_DATA( kUnitInfo.m_nUnitUID
			>> kPacket.m_cUnitClass
			>> kUnitInfo.m_iEXP
			>> kPacket.m_ucLevel
			>> kUnitInfo.m_iED
			>> kUnitInfo.m_iVSPoint			
			>> kUnitInfo.m_iVSPointMax
			>> kUnitInfo.m_kStat.m_iBaseHP
			>> kUnitInfo.m_kStat.m_iAtkPhysic
			>> kUnitInfo.m_kStat.m_iAtkMagic
			>> kUnitInfo.m_kStat.m_iDefPhysic
			>> kUnitInfo.m_kStat.m_iDefMagic
			>> kUnitInfo.m_iSPoint
			>> kPacket.m_wstrNickName
			>> kUnitInfo.m_iWin
			>> kUnitInfo.m_iLose
			>> kUnitInfo.m_nMapID
			>> kUnitInfo.m_iSpirit
			>> bIsSpiritUpdated
			>> iRecommendUnitUID
			);
#endif SERV_PVP_NEW_SYSTEM
		//}}

		m_kODBC.EndFetch();
	}
	else
	{
		kPacket.m_iOK = NetError::ERR_SELECT_UNIT_00;
		goto end_proc;
	}
#endif SERV_CHAR_LOG
	//}}


#ifdef SERV_UNIT_WAIT_DELETE //2012.06.08 lygan_������ // �г��� ���� ��� ���� ĳ���ʹ� ������ ���ϰ� ó�� �ϱ� ����
	if(kPacket.m_wstrNickName.length() != 0)
	{
		if(kPacket.m_wstrNickName.find(L"_") != -1)
		{
			kPacket.m_iOK = NetError::ERR_DELETE_UNIT_05;
			goto end_proc;
		}
	}
#endif //SERV_UNIT_WAIT_DELETE

	//{{ 2009. 10. 8  ������	���
#ifdef GUILD_TEST
	// ��� ���� ���
	DO_QUERY( L"exec dbo.gup_get_my_guild_info", L"%d", % kPacket_.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iGuildUID
			>> wstrGuildName );

		m_kODBC.EndFetch();
	}

	if( kPacket.m_iGuildUID > 0 )
	{
		//////////////////////////////////////////////////////////////////////////
		// ��� ������ ��� �˻�
		u_char ucGuildMemberShipGrade = 0;
		
		if( Query_GetGuildMemberGrade( kPacket_.m_iUnitUID, ucGuildMemberShipGrade ) == false )
		{
			START_LOG( cerr, L"���� ��� ���� ��� ����. GuildUID�� ��ȿ�ѵ� DB���� ��������� ����? ������ ���¿���!" )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_GUILD_19; // ��忡 ���� ������ �ƴմϴ�.
			goto end_proc;
		}

		// ��� �����ʹ� ĳ���� ������ �Ҽ� ����!
		if( ucGuildMemberShipGrade == SEnum::GUG_MASTER )
		{
			kPacket.m_iOK = NetError::ERR_GUILD_35;
			goto end_proc;
		}

		//////////////////////////////////////////////////////////////////////////
		// ��� Ż�� ó��
		DO_QUERY( L"exec dbo.gup_delete_guild_member", L"%d, %d", 
			% kPacket_.m_iUnitUID
			% kPacket.m_iGuildUID
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"��� Ż�� ����." )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( kPacket.m_iGuildUID )
				<< END_LOG;

			switch( kPacket.m_iOK )
			{
			case -1: kPacket.m_iOK = NetError::ERR_GUILD_19; break; // �ش� ��忡 ���� ������ �ƴմϴ�.
			case -2: kPacket.m_iOK = NetError::ERR_GUILD_26; break; // Ʈ������ ����
			default: kPacket.m_iOK = NetError::ERR_UNKNOWN; break; // ��� Ż�� ����
			}

			goto end_proc;
		}
	}
#endif GUILD_TEST
	//}}
	//{{ 2012.02.20 ��ȿ��	ĳ���� ���� ���μ��� ���� (���� ��� �Ⱓ ����)
#ifdef SERV_UNIT_WAIT_DELETE
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	DO_QUERY( L"exec dbo.P_GUnitWaitDelete_INT", L"%d, %d", % kPacket_.m_iUnitUID 
															% kPacket_.m_iUserUID);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );

		m_kODBC.EndFetch();
	}


	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"ĳ���� ���� ��� ó�� ����!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		if( kPacket.m_iOK == -1 )
		{
			kPacket.m_iOK = NetError::ERR_DELETE_UNIT_01;
		}
		else if ( kPacket.m_iOK == -11 || kPacket.m_iOK == -12 )
		{
			// -11 : ĳ���� ���� ���� ����, -12 : ���� ���� ���� ����
			// ���� ���忡���� DB������ ������
			kPacket.m_iOK = NetError::ERR_ODBC_01;
		} 
		else if ( kPacket.m_iOK == -22 )
		{
			kPacket.m_iOK = NetError::ERR_DELETE_UNIT_02;
		}
	}


	//{{ //2012.03.05 lygan_������ // ĳ���� ���� �Ҷ� ���� ������ ������ �� �˾� ���� ����
	DO_QUERY( L"exec dbo.P_GUnitWaitDelete_SEL", L"%d",	% kPacket_.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( wstrDelDate 
			>> wstrDelAbleDate
			>> wstrRestoreAbleDate
			);

		m_kODBC.EndFetch();
	}

	//{{ // 2012.03.06 lygan_������ // ���� ���ڿ��ð��� �״�� �޴� �κ��� �������� ���ڷ� �����ؼ� Ŭ��� ���� ������ ����
	if( KncUtil::ConvertStringToCTime( wstrDelAbleDate, tDelAbleDateTime ) == true )
		kPacket.m_tDelAbleDate = tDelAbleDateTime.GetTime();
	else
		kPacket.m_tDelAbleDate = 0LL;
	//}}

	//}} //2012.03.05 lygan_������ // ĳ���� ���� �Ҷ� ���� ������ ������ �� �˾� ���� ����
#else SERV_UNIT_WAIT_DELETE
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	//{{ 2012. 02. 21	��μ�	ĳ���� ���� �� ��� Ż�� ����ó�� ����
#ifdef SERV_UNIT_DELETE_EXCEPTION_MODIFY
	DO_QUERY( L"exec dbo.gup_delete_unit", L"%d, %d", % kPacket_.m_iUserUID % kPacket_.m_iUnitUID );
#else
	DO_QUERY( L"exec dbo.gup_delete_unit", L"%d", % kPacket_.m_iUnitUID );
#endif SERV_UNIT_DELETE_EXCEPTION_MODIFY
	//}}

    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( kPacket.m_iOK );

        m_kODBC.EndFetch();
    }

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"ĳ���� ���� ����!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		switch( kPacket.m_iOK )
		{
		case -1: kPacket.m_iOK = NetError::ERR_DELETE_UNIT_00; break; // �ش� ĳ���͸� ã�� �� �����ϴ�.
			//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
		case -2: kPacket.m_iOK = NetError::ERR_DELETE_UNIT_06; break; // �ο� ���¿����� ���� �� �� �����ϴ�.
#endif SERV_RELATIONSHIP_SYSTEM
			//}
		default: kPacket.m_iOK = NetError::ERR_DELETE_UNIT_05; break; // ĳ���� ���� ����
		}
	}
#endif SERV_UNIT_WAIT_DELETE
	//}}

end_proc:
    SendToUser( LAST_SENDER_UID, DBE_GAME_DELETE_UNIT_ACK, kPacket );
}

//{{ 2012.02.20 ��ȿ��	ĳ���� ���� ���μ��� ���� (���� ��� �Ⱓ ����)
#ifdef SERV_UNIT_WAIT_DELETE
_IMPL_ON_FUNC( DBE_GAME_FINAL_DELETE_UNIT_REQ, KEGS_FINAL_DELETE_UNIT_REQ )
{
	KEGS_FINAL_DELETE_UNIT_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iUnitUID = kPacket_.m_iUnitUID;

	//{{ 2012. 06. 11	������	��Ʋ�ʵ� �ý���
#ifdef SERV_BATTLE_FIELD_SYSTEM
	int iMapIDDummy = 0;
#endif SERV_BATTLE_FIELD_SYSTEM
	//}}

	//{{ 2011. 02. 23	������	ĳ���� �α�
#ifdef SERV_CHAR_LOG
	// ĳ���� ������ ��´�.
	DO_QUERY( L"exec dbo.gup_get_unit_info_by_unituid", L"%d", % kPacket_.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		KUnitInfo kUnitInfo;
		bool bIsSpiritUpdated = false;
		UidType iRecommendUnitUID = 0;

		FETCH_DATA( kUnitInfo.m_nUnitUID
			>> kPacket.m_cUnitClass
			>> kUnitInfo.m_iEXP
			>> kPacket.m_ucLevel
			>> kUnitInfo.m_iED
			>> kUnitInfo.m_kStat.m_iBaseHP
			>> kUnitInfo.m_kStat.m_iAtkPhysic
			>> kUnitInfo.m_kStat.m_iAtkMagic
			>> kUnitInfo.m_kStat.m_iDefPhysic
			>> kUnitInfo.m_kStat.m_iDefMagic
			>> kUnitInfo.m_iSPoint
			>> kPacket.m_wstrNickName
			>> iMapIDDummy
			>> kUnitInfo.m_iSpirit				
			>> bIsSpiritUpdated
			>> iRecommendUnitUID			
			);

		m_kODBC.EndFetch();
	}
	else
	{
		kPacket.m_iOK = NetError::ERR_SELECT_UNIT_00;
		goto end_proc;
	}
#endif SERV_CHAR_LOG
	//}}

	// ��� ������ �̹� ���� �Ǿ��ٰ� �� //

	kPacket.m_iOK = NetError::ERR_ODBC_01;
	DO_QUERY( L"exec dbo.gup_delete_unit", L"%d, %d", % kPacket_.m_iUserUID
		% kPacket_.m_iUnitUID);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );

		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"ĳ���� ���� ���� ó�� ����!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		if( kPacket.m_iOK == -21 || kPacket.m_iOK == - 23)
		{
			kPacket.m_iOK = NetError::ERR_DELETE_UNIT_04;
		}

		else if ( kPacket.m_iOK == -22 )
		{
			kPacket.m_iOK = NetError::ERR_DELETE_UNIT_03;

		}
		else if (	kPacket.m_iOK == -24 || kPacket.m_iOK == -11 || 
			kPacket.m_iOK == -13 || kPacket.m_iOK == -15 )
		{
			kPacket.m_iOK = NetError::ERR_ODBC_01;
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_GAME_FINAL_DELETE_UNIT_ACK, kPacket );

}

_IMPL_ON_FUNC( DBE_GAME_RESTORE_UNIT_REQ, KEGS_RESTORE_UNIT_REQ )
{
	KEGS_RESTORE_UNIT_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iUnitUID = kPacket_.m_iUnitUID;

	std::wstring wstrRestoreAbleDate  = L"";
	CTime tRestoreAbleDateTime;

	kPacket.m_iOK = NetError::ERR_ODBC_01;
	DO_QUERY( L"exec dbo.P_GUnitRestore_SET", L"%d, %d",% kPacket_.m_iUnitUID
		% kPacket_.m_iUserUID);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK 
			>> wstrRestoreAbleDate
			);

		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"ĳ���� ���� ó�� ����!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iUserUID )
			<< END_LOG;

		if( kPacket.m_iOK == -31 || kPacket.m_iOK == -33)
		{
			kPacket.m_iOK = NetError::ERR_RESTORE_UNIT_01;
		}

		else if ( kPacket.m_iOK == -32 )
		{
			kPacket.m_iOK = NetError::ERR_RESTORE_UNIT_02;
			//{{ // 2012.03.06 lygan_������ // ���� ���ڿ��ð��� �״�� �޴� �κ��� �������� ���ڷ� �����ؼ� Ŭ��� ���� ������ ����
			if( KncUtil::ConvertStringToCTime( wstrRestoreAbleDate, tRestoreAbleDateTime ) == true )
				kPacket.m_tRestoreAbleDate = tRestoreAbleDateTime.GetTime();
			else
				kPacket.m_tRestoreAbleDate = 0LL;
			//}}

		}
		else if ( kPacket.m_iOK == -34 || kPacket.m_iOK == -11 )
		{
			kPacket.m_iOK = NetError::ERR_ODBC_01;
		}
		else
		{
			kPacket.m_iOK = NetError::ERR_UNKNOWN;
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_GAME_RESTORE_UNIT_ACK, kPacket );
}
#endif SERV_UNIT_WAIT_DELETE
//}}

_IMPL_ON_FUNC( DBE_GAME_SELECT_UNIT_REQ, KEGS_SELECT_UNIT_REQ )
{
	//{{ 2012. 12. 10  ĳ���� ���� ��Ŷ ���� - ��μ�
#ifdef SERV_SELECT_UNIT_PACKET_DIVISION
	KDBE_SELECT_UNIT_ACK kPacket;
#else
	KEGS_SELECT_UNIT_ACK kPacket;
#endif SERV_SELECT_UNIT_PACKET_DIVISION
	//}}

	// ĳ���� ����
	Query_SelectUnit( LAST_SENDER_UID, kPacket_, kPacket );

#ifdef SERV_ELISIS_PREVIOUS_SIS_EVENT// �۾���¥: 2013-06-25	// �ڼ���
	if( ( kPacket.m_iOK != NetError::ERR_SELECT_UNIT_01 )
		|| ( CXSLUnit::GetUnitClassToUnitType( static_cast<CXSLUnit::UNIT_CLASS>( kPacket.m_kUnitInfo.m_cUnitClass ) ) != CXSLUnit::UT_ELESIS )
		)
	{
#endif // SERV_ELISIS_PREVIOUS_SIS_EVENT
    LOG_SUCCESS( kPacket.m_iOK == NetError::NET_OK )
		<< BUILD_LOG( NetError::GetErrStr( kPacket.m_iOK ) )
        << BUILD_LOG( LAST_SENDER_UID )
        << BUILD_LOG( kPacket_.m_iUnitUID );
#ifdef SERV_ELISIS_PREVIOUS_SIS_EVENT// �۾���¥: 2013-06-25	// �ڼ���
	}
#endif // SERV_ELISIS_PREVIOUS_SIS_EVENT

    SendToUser( LAST_SENDER_UID, DBE_GAME_SELECT_UNIT_ACK, kPacket );
}

//{{ 2009. 5. 28  ������	ä���̵�
IMPL_ON_FUNC( DBE_CHANNEL_CHANGE_GAME_SELECT_UNIT_REQ )
{
	KDBE_CHANNEL_CHANGE_GAME_SELECT_UNIT_ACK kPacket;
	kPacket.m_kConnectAck = kPacket_.m_kConnectAck;
	kPacket.m_kVerifyAccountAck = kPacket_.m_kVerifyAccountAck;
	kPacket.m_kChangeUserInfo = kPacket_.m_kChangeUserInfo;

	// ĳ���� ����
	Query_SelectUnit( LAST_SENDER_UID, kPacket_.m_kSelectUnitReq, kPacket.m_kSelectUnitAck );

	LOG_SUCCESS( kPacket.m_kSelectUnitAck.m_iOK == NetError::NET_OK )
		<< BUILD_LOG( NetError::GetErrStr( kPacket.m_kSelectUnitAck.m_iOK ) )
		<< BUILD_LOG( LAST_SENDER_UID )
		<< BUILD_LOG( kPacket_.m_kSelectUnitReq.m_iUnitUID );

	SendToUser( LAST_SENDER_UID, DBE_CHANNEL_CHANGE_GAME_SELECT_UNIT_ACK, kPacket );
}
//}}

_IMPL_ON_FUNC( DBE_ADMIN_MODIFY_UNIT_LEVEL_REQ, KEGS_ADMIN_MODIFY_UNIT_LEVEL_REQ )
{
    KEGS_ADMIN_MODIFY_UNIT_LEVEL_ACK kPacket;
    kPacket.m_iOK = NetError::ERR_ADMIN_COMMAND_00;
	kPacket.m_kUnitInfo.m_ucLevel = (UCHAR)kPacket_.m_iLevel;
	kPacket.m_kUnitInfo.m_iEXP = kPacket_.m_iEXP;
	
	DO_QUERY( L"exec dbo.gup_get_unit_uid", L"N\'%s\'", % kPacket_.m_wstrUnitNickName );

	UidType	unitUID;
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( unitUID );
		m_kODBC.EndFetch();

		kPacket.m_kUnitInfo.m_nUnitUID = unitUID;
	}
	else
	{
		kPacket.m_iOK = NetError::ERR_ADMIN_COMMAND_01;
		goto end_proc;
	}

	//{{ 2011. 07. 22	������	��� ���� SP���
#ifdef SERV_ALL_DB_QUERY_USE_SP
	DO_QUERY( L"exec dbo.gup_get_unitclass_by_unituid", L"%d", % unitUID );
#else
	//DO_QUERY_NO_PROFILE( L"SELECT UnitClass FROM dbo.GUnit( NOLOCK )", L"WHERE UnitUID = %d", % unitUID );
#endif SERV_ALL_DB_QUERY_USE_SP
	//}}    

    int iClass;
    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( iClass );
        m_kODBC.EndFetch();
    }
    else
    {
        kPacket.m_iOK = NetError::ERR_ADMIN_COMMAND_01;
        goto end_proc;
    }

	DO_QUERY( L"exec dbo.gup_admin_update_unit_info", L"%d, %d, %d",
        % unitUID
        % kPacket_.m_iEXP
        % kPacket_.m_iLevel
        );

    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( kPacket.m_iOK );
        m_kODBC.EndFetch();
    }

end_proc:
    SendToUser( LAST_SENDER_UID, DBE_ADMIN_MODIFY_UNIT_LEVEL_ACK, kPacket );
}

_IMPL_ON_FUNC( DBE_ADMIN_CHANGE_ED_REQ, KEGS_ADMIN_CHANGE_ED_REQ )
{
	KEGS_ADMIN_CHANGE_ED_ACK kPacket;
	kPacket.m_iOK = NetError::NET_OK;

	DO_QUERY( L"exec dbo.gup_get_unit_uid", L"N\'%s\'", % kPacket_.m_wstrUnitNickName );

	UidType	unitUID;

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( unitUID );
		m_kODBC.EndFetch();
	}
	else
	{
		kPacket.m_iOK = NetError::ERR_ADMIN_COMMAND_04;
		goto end_proc;
	}

	DO_QUERY( L"exec dbo.gup_admin_update_unit_gamepoint", L"%d, %d",
		% unitUID
		% kPacket_.m_nED
		);

	if( m_kODBC.BeginFetch() )
	{
		m_kODBC.EndFetch();
	}
	else
	{
		kPacket.m_iOK = NetError::ERR_ADMIN_COMMAND_04;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_ADMIN_CHANGE_ED_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_BUY_ED_ITEM_REQ )
{
    KDBE_BUY_ED_ITEM_ACK kPacket;
    kPacket.m_iED = 0;
	//{{ 2011. 07. 11	������	���� ����
#ifdef SERV_PVP_NEW_SYSTEM
	kPacket.m_iAPoint = 0;
#else
	kPacket.m_iVSPoint = 0;
#endif SERV_PVP_NEW_SYSTEM
	//}}    
    kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;

	bool bUpdateFailed = false;
    LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
    LIF( Query_UpdateItemEndurance( kPacket_.m_iUnitUID, kPacket_.m_kItemEnduranceUpdate, kPacket.m_kItemEnduranceUpdate ) );
    Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	// DB�� ������ ED�������� ���� �غ� ����!
	std::vector< KItemInfo > vecInsertItemList;

	// ���� ������ �����̳ʿ� �Ű� ����!
	BOOST_TEST_FOREACH( const KBuyGPItemInfo&, kBuyGPItemInfo, kPacket_.m_vecBuyGPItemInfo )
    {
		KItemInfo kItemInfo;
		kItemInfo.m_iItemID		= kBuyGPItemInfo.m_iItemID;
		kItemInfo.m_cUsageType	= kBuyGPItemInfo.m_cPeriodType;
		kItemInfo.m_iQuantity	= kBuyGPItemInfo.m_iQuantity;
		kItemInfo.m_sEndurance	= kBuyGPItemInfo.m_sEndurance;
		kItemInfo.m_sPeriod		= kBuyGPItemInfo.m_sPeriod;
		vecInsertItemList.push_back( kItemInfo );
    }

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
#ifdef SERV_GET_ITEM_REASON_BY_CHEAT
	int iGetItemReason = ( true == kPacket_.m_bCheat ) ? SEnum::GIR_GET_ITEM_BY_CHEAT : SEnum::GIR_BUY_ED_ITEM;

	if( Query_InsertItemList( static_cast<SEnum::GET_ITEM_REASON>( iGetItemReason ), kPacket_.m_iUnitUID, vecInsertItemList, kPacket.m_mapItemInfo ) == false )
#else
	if( Query_InsertItemList( SEnum::GIR_BUY_ED_ITEM, kPacket_.m_iUnitUID, vecInsertItemList, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON_BY_CHEAT
#else
	if( Query_InsertItemList( kPacket_.m_iUnitUID, vecInsertItemList, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
	//}}
	{
		START_LOG( cerr, L"������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;
	}

    SendToUser( LAST_SENDER_UID, DBE_BUY_ED_ITEM_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_INSERT_ITEM_REQ )
{
    KDBE_INSERT_ITEM_ACK kPacket;
	kPacket.m_bOutRoom = kPacket_.m_bOutRoom;
    kPacket.m_mapInsertedItem = kPacket_.m_mapInsertedItem;
    kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;
	//{{ 2011. 05. 04  ��μ�	���� Ŭ����� ������ ���� ���� �߰�
#ifdef SERV_DUNGEON_CLEAR_PAYMENT_ITEM
	kPacket.m_mapGetItem = kPacket_.m_mapGetItem;
#endif SERV_DUNGEON_CLEAR_PAYMENT_ITEM
	//}}

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	LIF( Query_UpdateItemEndurance( kPacket_.m_iUnitUID, kPacket_.m_kItemEnduranceUpdate, kPacket.m_kItemEnduranceUpdate ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
	if( Query_InsertItemList( static_cast<SEnum::GET_ITEM_REASON>(kPacket_.m_cGetItemReason), kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#else
	if( Query_InsertItemList( kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
	//}}	
	{
		START_LOG( cerr, L"������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;
	}

    SendToUser( LAST_SENDER_UID, DBE_INSERT_ITEM_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_NEW_QUEST_REQ )
{
	KDBE_NEW_QUEST_ACK kPacket;
	kPacket.m_iOK		= NetError::ERR_ODBC_01;
	kPacket.m_iQuestID	= kPacket_.m_iQuestID;
	//{{ 2010. 10. 27	������	����Ʈ ���� �߰�
#ifdef SERV_QUEST_CLEAR_EXPAND
	kPacket.m_mapInsertedItem			= kPacket_.m_mapInsertedItem;
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;

	//////////////////////////////////////////////////////////////////////////
	// ������ ����
	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_UnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_UnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
	if( Query_InsertItemList( SEnum::GIR_QUEST_REWARD, kPacket_.m_UnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#else
	if( Query_InsertItemList( kPacket_.m_UnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
		//}}
	{
		START_LOG( cerr, L"����Ʈ ���� ���� ������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_UnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_UNKNOWN;
		goto end_proc;
	}
#endif SERV_QUEST_CLEAR_EXPAND
	//}}

	//////////////////////////////////////////////////////////////////////////
	// ����Ʈ ����
	DO_QUERY( L"exec dbo.gup_create_quest", L"%d, %d", % kPacket_.m_UnitUID % kPacket_.m_iQuestID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"����Ʈ DB ���� ����." )
			<< BUILD_LOG( LAST_SENDER_UID )
			<< BUILD_LOG( kPacket.m_iQuestID )
			<< BUILD_LOG( kPacket.m_iOK )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_QUEST_10;
		goto end_proc;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_NEW_QUEST_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_GIVE_UP_QUEST_REQ )
{
	KDBE_GIVE_UP_QUEST_ACK kPacket;
	kPacket.m_iOK		= NetError::ERR_ODBC_01;
	kPacket.m_iQuestID	= kPacket_.m_iQuestID;

	DO_QUERY( L"exec dbo.gup_delete_quest", L"%d, %d", % kPacket_.m_UnitUID % kPacket_.m_iQuestID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"����Ʈ DB ���� ����." )
			<< BUILD_LOG( LAST_SENDER_UID )
			<< BUILD_LOG( kPacket.m_iQuestID )
			<< BUILD_LOG( kPacket.m_iOK )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_QUEST_10;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_GIVE_UP_QUEST_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_QUEST_COMPLETE_REQ )
{
	KDBE_QUEST_COMPLETE_ACK kPacket;
	std::vector< KItemInfo >::iterator vit;
	//{{ 2012. 04. 17	������	����Ʈ ���� ���� ����
#ifdef SERV_ENTER_FIELD_QUEST_CLEAR
	kPacket.m_bAutoComplete	= kPacket_.m_bAutoComplete;
#endif SERV_ENTER_FIELD_QUEST_CLEAR
	//}}
	kPacket.m_bIsNew		= kPacket_.m_bIsNew;
	kPacket.m_iOK			= NetError::ERR_ODBC_01;
    kPacket.m_mapInsertedItem = kPacket_.m_mapInsertedItem;
    kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_bIsChangeJob = kPacket_.m_bIsChangeJob;
	kPacket.m_cChangeUnitClass = kPacket_.m_cChangeUnitClass;	

#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013 // ���볯¥: 2013-06-27
	kPacket.m_iNewDefaultSkill1 = kPacket_.m_iNewDefaultSkill1;
	kPacket.m_iNewDefaultSkill2 = kPacket_.m_iNewDefaultSkill2;	
#endif	// SERV_UPGRADE_SKILL_SYSTEM_2013

	//{{ 2010. 04. 02  ������	��������Ʈ
#ifdef SERV_DAILY_QUEST
	
	kPacket.m_kCompleteQuestInfo.m_iQuestID = kPacket_.m_iQuestID;
	std::wstring wstrCompleteDate;

	DO_QUERY( L"exec dbo.gup_complete_quest", L"%d, %d, %d", % kPacket_.m_UnitUID % kPacket_.m_iQuestID % kPacket_.m_bIsRepeat );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK
			>> wstrCompleteDate
			);

		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"����Ʈ �Ϸ� DB ������Ʈ ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_UnitUID )
			<< BUILD_LOG( kPacket_.m_iQuestID )
			<< BUILD_LOG( kPacket_.m_bIsRepeat )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_QUEST_10;
		goto end_proc;
	}
	else
	{
		CTime tCompleteDate;
		LIF( KncUtil::ConvertStringToCTime( wstrCompleteDate, tCompleteDate ) );

		// ����Ʈ �Ϸ� ������Ʈ ����!
		kPacket.m_kCompleteQuestInfo.m_iQuestID = kPacket_.m_iQuestID;
		kPacket.m_kCompleteQuestInfo.m_iCompleteCount = 1;
		kPacket.m_kCompleteQuestInfo.m_tCompleteDate = tCompleteDate.GetTime();
	}

#else

	kPacket.m_iQuestID = kPacket_.m_iQuestID;

	DO_QUERY( L"exec dbo.gup_complete_quest", L"%d, %d, %d", % kPacket_.m_UnitUID % kPacket_.m_iQuestID % kPacket_.m_bIsRepeat );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"����Ʈ �Ϸ� DB ������Ʈ ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_UnitUID )
			<< BUILD_LOG( kPacket_.m_iQuestID )
			<< BUILD_LOG( kPacket_.m_bIsRepeat )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_QUEST_10;
		goto end_proc;
	}	

#endif SERV_DAILY_QUEST
	//}}

	//item uid �޾ƿ���
	if( kPacket_.m_bIsNew == true )
	{
		bool bUpdateFailed = false;
		LIF( Query_UpdateItemQuantity( kPacket_.m_UnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
		Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

		//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
		if( bUpdateFailed )
		{
			START_LOG( cout, L"���а� ������Ʈ ����!" )
				<< BUILD_LOG( kPacket_.m_UnitUID );

			std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
			for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
			{
				START_LOG( cout, L"������ ����" )
					<< BUILD_LOG( mitQC->first )
					<< BUILD_LOG( mitQC->second );
			}
		}
		//}}

		//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
		if( Query_InsertItemList( SEnum::GIR_QUEST_REWARD, kPacket_.m_UnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#else
		if( Query_InsertItemList( kPacket_.m_UnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
		//}}
		{
			START_LOG( cerr, L"����Ʈ ���� ������ ������Ʈ ����!" )
				<< BUILD_LOG( kPacket_.m_UnitUID )
				<< END_LOG;
			goto end_proc;
		}
	}

	//��������Ʈ ���� ���ó��..
	if( kPacket_.m_bIsChangeJob == true )
	{
#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013 // ���볯¥: 2013-06-27
		DO_QUERY( L"exec P_GUnit_UPD_UnitClass", L"%d, %d, %d, %d", % kPacket_.m_UnitUID % (int)kPacket_.m_cChangeUnitClass % kPacket_.m_iNewDefaultSkill1 % kPacket_.m_iNewDefaultSkill2 );
#else	// SERV_UPGRADE_SKILL_SYSTEM_2013
/*
		DO_QUERY( L"exec gup_update_unit_class", L"%d, %d", % kPacket_.m_UnitUID % (int)kPacket_.m_cChangeUnitClass );
*/
#endif	// SERV_UPGRADE_SKILL_SYSTEM_2013
		
		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}
		else
		{
			kPacket.m_iOK = NetError::ERR_QUEST_10;

			START_LOG( cerr, L"DB ó�� ���з� ���� ���� ����Ʈ�Ϸ� ��������.!" )
				<< BUILD_LOG( LAST_SENDER_UID )
				<< BUILD_LOG( NetError::GetErrStr( kPacket.m_iOK ) )
				<< END_LOG;

			goto end_proc;
		}

        if( kPacket.m_iOK != NetError::NET_OK )
		{
			kPacket.m_iOK = NetError::ERR_QUEST_10;

			START_LOG( cerr, L"���� ����Ʈ�Ϸ� ��������.!" )
				<< BUILD_LOG( LAST_SENDER_UID )
				<< BUILD_LOG( NetError::GetErrStr( kPacket.m_iOK ) )
				<< END_LOG;

			goto end_proc;
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_QUEST_COMPLETE_ACK, kPacket );
}

#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013 // ���볯¥: 2013-06-27
IMPL_ON_FUNC( DBE_INSERT_SKILL_REQ )
{
	KDBE_INSERT_SKILL_ACK kPacket;
	kPacket.m_iOK						= NetError::ERR_ODBC_01;
	kPacket.m_mapSkillList				= kPacket_.m_mapSkillList;
	kPacket.m_iCSPoint					= kPacket_.m_iCSPoint;
	kPacket.m_iTotalSpendSkillPoint		= kPacket_.m_iTotalSpendSkillPoint;
	kPacket.m_iBeforCSPoint		= kPacket_.m_iBeforCSPoint;
    
	// ĳ�� ��ų ����Ʈ ������ ����
	if( kPacket_.m_iCSPoint >= 0 )
	{
		DO_QUERY( L"exec dbo.gup_update_cash_skill_point_info", L"%d, %d",
			% kPacket_.m_iUnitUID
			% kPacket_.m_iCSPoint
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}	

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"failed to update cash skill point when insert skill!" )
				<< BUILD_LOG( LAST_SENDER_UID )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( kPacket_.m_iCSPoint )
				<< BUILD_LOG( kPacket.m_iOK )
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_SKILL_24;

			goto end_proc;	
		}
	}

	// ��ų ȹ�� ������ ����
	{
		std::map< int, KGetSkillInfo >::iterator mit = kPacket_.m_mapSkillList.begin();
		for( ; mit != kPacket_.m_mapSkillList.end() ; ++mit )
		{
			int iOK = NetError::NET_OK;

			DO_QUERY( L"exec dbo.gup_update_skill_info_new", L"%d, %d, %d, %d",
				% kPacket_.m_iUnitUID 
				% mit->second.m_iSkillID
				% mit->second.m_iSkillLevel
				% mit->second.m_iSpendSkillCSPoint );

			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( iOK );
				m_kODBC.EndFetch();
			}

			if( iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"failed to insert skill!" )
					<< BUILD_LOG( LAST_SENDER_UID )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( mit->second.m_iSkillID )
					<< BUILD_LOG( mit->second.m_iSkillLevel )
					<< BUILD_LOG( mit->second.m_iSpendSkillCSPoint )
					<< BUILD_LOG( iOK )
					<< END_LOG;

				kPacket.m_iOK = NetError::ERR_SKILL_00;

				goto end_proc;
			}
		}
	}

	kPacket.m_iOK = NetError::NET_OK;

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_INSERT_SKILL_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_INIT_SKILL_TREE_REQ )
{
	KDBE_INIT_SKILL_TREE_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iItemUID			= kPacket_.m_iItemUID;
	kPacket.m_iSPoint 			= kPacket_.m_iSPoint;
	kPacket.m_iCSPoint 			= kPacket_.m_iCSPoint;
	kPacket.m_iDefaultSkillID1	= kPacket_.m_iDefaultSkillID1;
	kPacket.m_iDefaultSkillID2	= kPacket_.m_iDefaultSkillID2;
	kPacket.m_iDefaultSkillID3	= kPacket_.m_iDefaultSkillID3;
	kPacket.m_iDefaultSkillID4	= kPacket_.m_iDefaultSkillID4;
	kPacket.m_iDefaultSkillID5	= kPacket_.m_iDefaultSkillID5;
	kPacket.m_iDefaultSkillID6	= kPacket_.m_iDefaultSkillID6;
	kPacket.m_iBeforSPoint	= kPacket_.m_iBeforSPoint;
	kPacket.m_iBeforCSPoint	= kPacket_.m_iBeforCSPoint;

	DO_QUERY( L"exec dbo.P_GSkill_New_All_DEL", L"%d, %d, %d, %d, %d, %d, %d, %d, %d",
		% kPacket_.m_iUnitUID
		% kPacket_.m_iSPoint
		% kPacket_.m_iCSPoint
		% kPacket_.m_iDefaultSkillID1
		% kPacket_.m_iDefaultSkillID2
		% kPacket_.m_iDefaultSkillID3
		% kPacket_.m_iDefaultSkillID4
		% kPacket_.m_iDefaultSkillID5
		% kPacket_.m_iDefaultSkillID6
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"DB ��ų�ʱ�ȭ ����.!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iItemUID )
			<< BUILD_LOG( kPacket_.m_iSPoint )
			<< BUILD_LOG( kPacket_.m_iCSPoint )
			<< BUILD_LOG( kPacket_.m_iDefaultSkillID1 )
			<< BUILD_LOG( kPacket_.m_iDefaultSkillID2 )
			<< BUILD_LOG( kPacket_.m_iDefaultSkillID3 )
			<< BUILD_LOG( kPacket_.m_iDefaultSkillID4 )
			<< BUILD_LOG( kPacket_.m_iDefaultSkillID5 )
			<< BUILD_LOG( kPacket_.m_iDefaultSkillID6 )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_SKILL_12;

		goto end_proc;
	}

	kPacket.m_iOK = NetError::NET_OK;

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_INIT_SKILL_TREE_ACK, kPacket );
}

//{{ 2012. 03. 23	�ڼ���	�����ڿ� ġƮŰ ���� ����
_IMPL_ON_FUNC( DBE_ADMIN_INIT_SKILL_TREE_REQ, KDBE_INIT_SKILL_TREE_REQ )
{
	KEGS_ADMIN_INIT_SKILL_TREE_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iSPoint = kPacket_.m_iSPoint;
	kPacket.m_iCSPoint = kPacket_.m_iCSPoint;

	//{{ 2012. 03. 23	�ڼ���	�����ڿ� ġƮŰ ���� ����
	DO_QUERY( L"exec dbo.P_GSkill_New_All_DEL", L"%d, %d, %d, %d, %d, %d, %d, %d, %d",
		% kPacket_.m_iUnitUID
		% kPacket_.m_iSPoint
		% kPacket_.m_iCSPoint
		% kPacket_.m_iDefaultSkillID1
		% kPacket_.m_iDefaultSkillID2
		% kPacket_.m_iDefaultSkillID3
		% kPacket_.m_iDefaultSkillID4
		% kPacket_.m_iDefaultSkillID5
		% kPacket_.m_iDefaultSkillID6 );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_ADMIN_INIT_SKILL_TREE_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_ADMIN_CHANGE_UNIT_CLASS_REQ )
{
	KDBE_ADMIN_CHANGE_UNIT_CLASS_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_cUnitClass = kPacket_.m_cUnitClass;

	//{{ 2012. 03. 23	�ڼ���	�����ڿ� ġƮŰ ���� ����
	DO_QUERY( L"exec dbo.P_GUnit_UPD_UnitClass_Admin", L"%d, %d, %d, %d, %d, %d, %d, %d",
		% kPacket_.m_iUnitUID
		% static_cast<int>(kPacket_.m_cUnitClass)
		% kPacket_.m_iNewDefaultSkill1
		% kPacket_.m_iNewDefaultSkill2
		% kPacket_.m_iNewDefaultSkill3
		% kPacket_.m_iNewDefaultSkill4
		% kPacket_.m_iNewDefaultSkill5
		% kPacket_.m_iNewDefaultSkill6 );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_ADMIN_CHANGE_UNIT_CLASS_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_ADMIN_AUTO_GET_ALL_SKILL_REQ )
{
	KDBE_ADMIN_AUTO_GET_ALL_SKILL_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_cUnitClass = kPacket_.m_cUnitClass;
	kPacket.m_iSPoint = kPacket_.m_iSPoint;
	kPacket.m_iCSPoint = kPacket_.m_iCSPoint;
	kPacket.m_mapGetSkillList = kPacket_.m_mapGetSkillList;
	kPacket.m_vecUnsealedSkillID = kPacket_.m_vecUnsealedSkillID;
	kPacket.m_iNewDefaultSkill1 = kPacket_.m_iNewDefaultSkill1;
	kPacket.m_iNewDefaultSkill2 = kPacket_.m_iNewDefaultSkill2;
	kPacket.m_iNewDefaultSkill3 = kPacket_.m_iNewDefaultSkill3;
	kPacket.m_iNewDefaultSkill4 = kPacket_.m_iNewDefaultSkill4;
	kPacket.m_iNewDefaultSkill5 = kPacket_.m_iNewDefaultSkill5;
	kPacket.m_iNewDefaultSkill6 = kPacket_.m_iNewDefaultSkill6;

	//{{ 2012. 03. 23	�ڼ���	�����ڿ� ġƮŰ ���� ����
	DO_QUERY( L"exec dbo.P_GUnit_UPD_UnitClass_Admin", L"%d, %d, %d, %d, %d, %d, %d, %d",
		% kPacket_.m_iUnitUID
		% static_cast<int>(kPacket_.m_cUnitClass)
		% kPacket_.m_iNewDefaultSkill1
		% kPacket_.m_iNewDefaultSkill2
		% kPacket_.m_iNewDefaultSkill3
		% kPacket_.m_iNewDefaultSkill4
		% kPacket_.m_iNewDefaultSkill5
		% kPacket_.m_iNewDefaultSkill6 );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"ġƮŰ ��ų �ʱ�ȭ ����" )
			<< BUILD_LOG( LAST_SENDER_UID )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket.m_iOK )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_SKILL_00;
		goto end_proc;
	}

	{
		std::map< int, int >::iterator mit = kPacket_.m_mapGetSkillList.begin();
		for( ; mit != kPacket_.m_mapGetSkillList.end() ; ++mit )
		{
			DO_QUERY( L"exec dbo.gup_update_skill_info_new", L"%d, %d, %d, %d",
				% kPacket_.m_iUnitUID 
				% mit->first
				% mit->second
				% 0 );

			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( kPacket.m_iOK );
				m_kODBC.EndFetch();
			}

			if( kPacket.m_iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"failed to insert skill!" )
					<< BUILD_LOG( LAST_SENDER_UID )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( mit->first )
					<< BUILD_LOG( mit->second )
					<< BUILD_LOG( kPacket.m_iOK )
					<< END_LOG;

				kPacket.m_iOK = NetError::ERR_SKILL_00;

				goto end_proc;
			}
		}

		BOOST_TEST_FOREACH( short, iSkillID, kPacket_.m_vecUnsealedSkillID )
		{
			DO_QUERY( L"exec dbo.gup_insert_unsealed_skill", L"%d, %d", % kPacket_.m_iUnitUID % (int)iSkillID );
			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( kPacket.m_iOK );
				m_kODBC.EndFetch();
			}

			if( kPacket.m_iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"failed to insert unsealed skill!" )
					<< BUILD_LOG( LAST_SENDER_UID )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( iSkillID )
					<< BUILD_LOG( kPacket.m_iOK )
					<< END_LOG;

				kPacket.m_iOK = NetError::ERR_SKILL_00;

				goto end_proc;
			}
		}
	}
	

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_ADMIN_AUTO_GET_ALL_SKILL_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_ADMIN_GET_SKILL_REQ )
{
	KDBE_ADMIN_GET_SKILL_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_00;
	kPacket.m_iSkillID = kPacket_.m_iSkillID;
	kPacket.m_iSkillLevel = kPacket_.m_iSkillLevel;
	kPacket.m_iCSPoint = kPacket_.m_iCSPoint;
	kPacket.m_bUnsealed = kPacket_.m_bUnsealed;

	// ��ų ȹ�� ������ ����
	{
		DO_QUERY( L"exec dbo.gup_update_skill_info_new", L"%d, %d, %d, %d",
			% kPacket_.m_iUnitUID 
			% kPacket_.m_iSkillID
			% kPacket_.m_iSkillLevel
			% kPacket_.m_iCSPoint );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"failed to insert skill!" )
				<< BUILD_LOG( LAST_SENDER_UID )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( kPacket_.m_iSkillID )
				<< BUILD_LOG( kPacket.m_iOK )
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_SKILL_00;

			goto end_proc;
		}

		if( kPacket_.m_bUnsealed == true )
		{
			DO_QUERY( L"exec dbo.gup_insert_unsealed_skill", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iSkillID );
			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( kPacket.m_iOK );
				m_kODBC.EndFetch();
			}

			if( kPacket.m_iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"failed to insert unsealed skill!" )
					<< BUILD_LOG( LAST_SENDER_UID )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( kPacket_.m_iSkillID )
					<< BUILD_LOG( kPacket.m_iOK )
					<< END_LOG;

				kPacket.m_iOK = NetError::ERR_SKILL_00;

				goto end_proc;
			}
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_ADMIN_GET_SKILL_ACK, kPacket );
}

#else	// SERV_UPGRADE_SKILL_SYSTEM_2013
/*
IMPL_ON_FUNC( DBE_INSERT_SKILL_REQ )
{
	KDBE_INSERT_SKILL_ACK kPacket;
	kPacket.m_iOK				= NetError::ERR_ODBC_01;
	kPacket.m_iSkillID			= kPacket_.m_iSkillID;
	kPacket.m_iSkillLevel		= kPacket_.m_iSkillLevel;
	kPacket.m_iSkillCSPoint		= kPacket_.m_iSkillCSPoint;
	kPacket.m_iCSPoint			= kPacket_.m_iCSPoint;

	// ĳ�� ��ų ����Ʈ ������ ����
	if( kPacket_.m_iCSPoint >= 0 )
	{
		DO_QUERY( L"exec dbo.gup_update_cash_skill_point_info", L"%d, %d",
			% kPacket_.m_iUnitUID
			% kPacket_.m_iCSPoint
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}	

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"failed to update cash skill point when insert skill!" )
				<< BUILD_LOG( LAST_SENDER_UID )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( kPacket_.m_iCSPoint )
				<< BUILD_LOG( kPacket_.m_iSkillID )
				<< BUILD_LOG( kPacket_.m_iSkillLevel )
				<< BUILD_LOG( kPacket_.m_iSkillCSPoint )
				<< BUILD_LOG( kPacket.m_iOK )
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_SKILL_24;

			goto end_proc;	
		}
	}

	// ��ų ȹ�� ������ ����
	DO_QUERY( L"exec dbo.gup_update_skill_info_new", L"%d, %d, %d, %d",
		% kPacket_.m_iUnitUID 
		% kPacket_.m_iSkillID
		% kPacket_.m_iSkillLevel
		% kPacket_.m_iSkillCSPoint );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}	

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"failed to insert skill!" )
			<< BUILD_LOG( LAST_SENDER_UID )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iSkillID )
			<< BUILD_LOG( kPacket_.m_iSkillLevel )
			<< BUILD_LOG( kPacket_.m_iSkillCSPoint )
			<< BUILD_LOG( kPacket.m_iOK )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_SKILL_00;

		goto end_proc;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_INSERT_SKILL_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_INIT_SKILL_TREE_REQ )
{
	KDBE_INIT_SKILL_TREE_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iItemUID			= kPacket_.m_iItemUID;
	kPacket.m_iSPoint 			= kPacket_.m_iSPoint;
	kPacket.m_iCSPoint 			= kPacket_.m_iCSPoint;
	kPacket.m_iDefaultSkillID	= kPacket_.m_iDefaultSkillID;
	kPacket.m_iDefaultSkillID2	= kPacket_.m_iDefaultSkillID2;

	DO_QUERY( L"exec dbo.gup_delete_all_skill_new", L"%d, %d, %d, %d, %d",
		% kPacket_.m_iUnitUID
		% kPacket_.m_iSPoint
		% kPacket_.m_iCSPoint
		% kPacket_.m_iDefaultSkillID
		% kPacket_.m_iDefaultSkillID2
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"DB ��ų�ʱ�ȭ ����.!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iItemUID )
			<< BUILD_LOG( kPacket_.m_iSPoint )
			<< BUILD_LOG( kPacket_.m_iCSPoint )
			<< BUILD_LOG( kPacket_.m_iDefaultSkillID )
			<< BUILD_LOG( kPacket_.m_iDefaultSkillID2 )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_SKILL_12;

		goto end_proc;
	}

	kPacket.m_iOK = NetError::NET_OK;

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_INIT_SKILL_TREE_ACK, kPacket );
}

//{{ 2012. 03. 23	�ڼ���	�����ڿ� ġƮŰ ���� ����
#ifdef SERV_FIX_THE_ADMIN_CHEAT
	_IMPL_ON_FUNC( DBE_ADMIN_INIT_SKILL_TREE_REQ, KDBE_INIT_SKILL_TREE_REQ )
#else
	_IMPL_ON_FUNC( DBE_ADMIN_INIT_SKILL_TREE_REQ, UidType )
#endif SERV_FIX_THE_ADMIN_CHEAT
//}}
{
	KEGS_ADMIN_INIT_SKILL_TREE_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	//{{ 2012. 03. 23	�ڼ���	�����ڿ� ġƮŰ ���� ����
#ifdef SERV_FIX_THE_ADMIN_CHEAT
	DO_QUERY( L"exec dbo.gup_delete_all_skill_new", L"%d, %d, %d, %d, %d",
		% kPacket_.m_iUnitUID
		% kPacket_.m_iSPoint
		% kPacket_.m_iCSPoint
		% kPacket_.m_iDefaultSkillID
		% kPacket_.m_iDefaultSkillID2 );
#else
	DO_QUERY( L"exec dbo.gup_delete_all_skill_new", L"%d, %d, %d, %d, %d",
		% kPacket_
		%1000
		%0
		%0
		%0 );
#endif SERV_FIX_THE_ADMIN_CHEAT
	//}}

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_ADMIN_INIT_SKILL_TREE_ACK, kPacket );
}
*/
#endif	// SERV_UPGRADE_SKILL_SYSTEM_2013


_IMPL_ON_FUNC( DBE_SEARCH_UNIT_REQ, std::wstring )
{
    KPacketOK kPacket;
    kPacket.m_iOK = NetError::ERR_ODBC_01;

    DO_QUERY( L"exec dbo.gup_get_unit_uid", L"N\'%s\'", % kPacket_ );

    UidType iUnitUID = 0;
    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( iUnitUID );
        m_kODBC.EndFetch();
    }

    if( iUnitUID != 0 )
    {
        kPacket.m_iOK = NetError::ERR_SEARCH_UNIT_03;
    }
    else
    {
        kPacket.m_iOK = NetError::ERR_SEARCH_UNIT_04;
    }

end_proc:
    SendToUser( LAST_SENDER_UID, DBE_SEARCH_UNIT_ACK, kPacket );
}

_IMPL_ON_FUNC( DBE_KNM_REQUEST_NEW_FRIEND_INFO_REQ, std::wstring )
{	
	KEGS_KNM_REQUEST_NEW_FRIEND_INFO_ACK kPacket;
	kPacket.m_iOK				= NetError::ERR_ODBC_01;
	kPacket.m_wstrUnitNickName	= kPacket_;

	KDenyOptions kDeny;

	DO_QUERY( L"exec dbo.gup_get_unit_uid", L"N\'%s\'", % kPacket_ );

	UidType iUnitUID = 0;
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iUnitUID );
		m_kODBC.EndFetch();
	}

	if( iUnitUID != 0 )
	{
		kPacket.m_iOK = NetError::NET_OK;
	}
	else
	{
		kPacket.m_iOK = NetError::ERR_KNM_02;

		goto end_proc;
	}

	//{{ 2009. 4. 1  ������		ģ�� �ʴ� ���� �ɼ��� ã�ƺ���	
	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
	DO_QUERY( L"exec dbo.P_GDenyOption_SEL", L"%d", % iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kDeny.m_cDenyFriendShip
			>> kDeny.m_cDenyInviteGuild
			>> kDeny.m_cDenyParty
			>> kDeny.m_cDenyPersonalTrade
			>> kDeny.m_cDenyRequestCouple
			);

		m_kODBC.EndFetch();

		// ģ�� �ʴ� �ź� �ɼ�
		if( kDeny.m_cDenyFriendShip == KDenyOptions::DOS_ON )
		{
			kPacket.m_iOK = NetError::ERR_KNM_06;
			goto end_proc;
		}

		kPacket.m_iOK = NetError::NET_OK;
	}
#else
	DO_QUERY( L"exec dbo.gup_get_community_opt_unituid", L"%d", % iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kDeny.m_cDenyFriendShip
			>> kDeny.m_cDenyInviteGuild
			>> kDeny.m_cDenyParty
			>> kDeny.m_cDenyPersonalTrade
			);

		m_kODBC.EndFetch();

		// ģ�� �ʴ� �ź� �ɼ�
		if( kDeny.m_cDenyFriendShip == KDenyOptions::DOS_ON )
		{
			kPacket.m_iOK = NetError::ERR_KNM_06;
			goto end_proc;
		}

		kPacket.m_iOK = NetError::NET_OK;
	}
#endif SERV_RELATIONSHIP_SYSTEM
	//}
	else
	{
		kPacket.m_iOK = NetError::ERR_COMMUNITY_OPT_02;
		goto end_proc;
	}
	//}}

	// �ؽ� �޽����� CharacterSN.
	DO_QUERY( L"exec dbo.gup_get_MessengerSN", L"%d", % iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		int iTemp = 0;
		FETCH_DATA( iTemp );

		kPacket.m_uiKNMSerialNum = static_cast<u_int>(iTemp);

		m_kODBC.EndFetch();
	}
	else
	{
		kPacket.m_iOK = NetError::ERR_KNM_02;
		goto end_proc;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_KNM_REQUEST_NEW_FRIEND_INFO_ACK, kPacket );
}


IMPL_ON_FUNC( DBE_INSERT_TRADE_ITEM_REQ )
{
	KDBE_INSERT_TRADE_ITEM_ACK kPacket;
	kPacket.m_mapInsertedItem = kPacket_.m_mapInsertedItem;
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
	if( Query_InsertItemList( SEnum::GIR_TRADE, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#else
	if( Query_InsertItemList( kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
	//}}
	{
		START_LOG( cerr, L"������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;		
	}

	SendToUser( LAST_SENDER_UID, DBE_INSERT_TRADE_ITEM_ACK, kPacket );
}

//{{ 2009. 2. 10  ������	���ΰŷ� ���� ����
IMPL_ON_FUNC( DBE_INSERT_TRADE_ITEM_BY_SERVER_NOT )
{
	std::map< UidType, KItemInfo > mapDummy;
	KItemQuantityUpdate kDummy;
	std::map< UidType, int >::const_iterator mit;
	std::vector< KDeletedItemInfo >::const_iterator vit;
	std::vector< KItemInfo >::const_iterator vitIT;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kDummy.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kDummy.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
	if( Query_InsertItemList( SEnum::GIR_TRADE_EXCEPTION, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, mapDummy ) == false )
#else
	if( Query_InsertItemList( kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, mapDummy ) == false )
#endif SERV_GET_ITEM_REASON
	//}}
	{
		START_LOG( cerr, L"������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;
	}

	int iOK = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec dbo.gup_admin_update_unit_gamepoint", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iED );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}

	if( iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"���ΰŷ��� ����� ���� ���� ���� ED������Ʈ ����!" )
			<< BUILD_LOG( iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iED )
			<< END_LOG;
	}

	//////////////////////////////////////////////////////////////////////////
	START_LOG( cout, L"���ΰŷ� �Ǵ� ���λ����� ����� ���� ���� ���� �ŷ������� ������Ʈ!" )
		<< BUILD_LOG( kPacket_.m_iUnitUID )
		<< BUILD_LOG( kPacket_.m_iED );

	mit = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
	for( ; mit != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mit )
	{
		START_LOG( cout, L"���� ����" )
			<< BUILD_LOG( mit->first )
			<< BUILD_LOG( mit->second );
	}

	vit = kPacket_.m_kItemQuantityUpdate.m_vecDeleted.begin();
	for( ; vit != kPacket_.m_kItemQuantityUpdate.m_vecDeleted.end(); ++vit )
	{
		START_LOG( cout, L"������ ����" )
			<< BUILD_LOG( vit->m_iItemUID );
	}
    
	vitIT = kPacket_.m_vecItemInfo.begin();
	for( ; vitIT != kPacket_.m_vecItemInfo.end(); ++vitIT )
	{
		START_LOG( cout, L"������ ����" )
			<< BUILD_LOG( vitIT->m_iItemID )
			<< BUILD_LOGc( vitIT->m_cUsageType )
			<< BUILD_LOG( vitIT->m_iQuantity )
			<< BUILD_LOG( vitIT->m_sEndurance )
			<< BUILD_LOGc( vitIT->m_cEnchantLevel )
			<< BUILD_LOGc( vitIT->m_kAttribEnchantInfo.m_cAttribEnchant0 )
			<< BUILD_LOGc( vitIT->m_kAttribEnchantInfo.m_cAttribEnchant1 )
			<< BUILD_LOG( vitIT->m_sPeriod )
			<< BUILD_LOG( vitIT->m_wstrExpirationDate );

		//{{ 2011. 07. 25    ��μ�    ������ �ɼ�ID ������ ������ ����
#ifdef SERV_ITEM_OPTION_DATA_SIZE
		std::vector< int >::const_iterator vitS = vitIT->m_vecItemSocket.begin();
#else
		std::vector< short >::const_iterator vitS = vitIT->m_vecItemSocket.begin();
#endif SERV_ITEM_OPTION_DATA_SIZE
		//}} 
		for( ; vitS != vitIT->m_vecItemSocket.begin(); ++vitS )
		{
			START_LOG( cout, L"������ ���� : ����" )
				<< BUILD_LOG( *vitS );
		}
	}
	//////////////////////////////////////////////////////////////////////////

end_proc:
	return;
}
//}}

_IMPL_ON_FUNC( DBE_INSERT_PURCHASED_CASH_ITEM_REQ, KDBE_INSERT_ITEM_REQ )
{
    KDBE_INSERT_ITEM_ACK kPacket;
    kPacket.m_mapInsertedItem = kPacket_.m_mapInsertedItem;
    kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;

	bool bUpdateFailed = false;
    LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
    LIF( Query_UpdateItemEndurance( kPacket_.m_iUnitUID, kPacket_.m_kItemEnduranceUpdate, kPacket.m_kItemEnduranceUpdate ) );
    Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
	if( Query_InsertItemList( SEnum::GIR_BUY_CASH_ITEM, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#else
	if( Query_InsertItemList( kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
	//}}
	{
		START_LOG( cerr, L"������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;
	}

	//{{ 2009. 9. 2  ������		�к�
	Query_UpdateSealItem( kPacket_.m_setSealCashItem, kPacket.m_mapItemInfo );
	//}}

    SendToUser( LAST_SENDER_UID, DBE_INSERT_PURCHASED_CASH_ITEM_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_INSERT_PURCHASED_CASH_PACKAGE_REQ )
{
	KDBE_INSERT_PURCHASED_CASH_PACKAGE_ACK kPacket;
	kPacket.m_ulProductNo = kPacket_.m_ulProductNo;
	kPacket.m_mapInsertedItem = kPacket_.m_mapInsertedItem;
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	LIF( Query_UpdateItemEndurance( kPacket_.m_iUnitUID, kPacket_.m_kItemEnduranceUpdate, kPacket.m_kItemEnduranceUpdate ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
	if( Query_InsertItemList( SEnum::GIR_BUY_CASH_PACKAGE, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#else
	if( Query_InsertItemList( kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
	//}}
	{
		START_LOG( cerr, L"������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;
	}

	//{{ 2009. 9. 2  ������		�к�
	Query_UpdateSealItem( kPacket_.m_setSealCashItem, kPacket.m_mapItemInfo );
	//}}

	SendToUser( LAST_SENDER_UID, DBE_INSERT_PURCHASED_CASH_PACKAGE_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_OPEN_RANDOM_ITEM_REQ )
{
	KDBE_OPEN_RANDOM_ITEM_ACK kPacket;
    kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_mapInsertedItem			= kPacket_.m_mapInsertedItem;
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
	//{{ 2013. 06. 04	������	������ ����
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05	
	// ť�� ���� ���� ������� �к� �ǵ��� ����
	if( Query_InsertItemList( SEnum::GIR_RANDOM_ITEM, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo, false ) == false )
#else
	if( Query_InsertItemList( SEnum::GIR_RANDOM_ITEM, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_NEW_ITEM_SYSTEM_2013_05
	//}}	
#else
	if( Query_InsertItemList( kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
	//}}
	{
		START_LOG( cerr, L"���� ť�� ������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_RANDOM_ITEM_06;
	}
	else
	{
		kPacket.m_iOK = NetError::NET_OK;

		//{{ 2010. 7. 26  ������	�к� ���� ť��
#ifdef SERV_SEALED_RANDOM_ITEM
		Query_UpdateSealItem( kPacket_.m_setSealRandomItem, kPacket.m_mapItemInfo );
#endif SERV_SEALED_RANDOM_ITEM
		//}}
	}

	//{{ 2011.12.28 ��μ�   ���� ť�� ���½� ��Ȱ�� ǥ�� ���� ����
#ifdef SERV_OPEN_RANDOM_CUBE_VIEW_ERROR
	kPacket.m_iNumResurrectionStone	= kPacket_.m_iNumResurrectionStone;
#endif SERV_OPEN_RANDOM_CUBE_VIEW_ERROR
	//}}

	//{{ 2012. 11. 26 ť�� ED ���� ���� ��� �߰� - ��μ�
#ifdef SERV_CUBE_OPEN_ED_CONDITION
	kPacket.m_iSpendED = kPacket_.m_iSpendED;
#endif SERV_CUBE_OPEN_ED_CONDITION
	//}

	SendToUser( LAST_SENDER_UID, DBE_OPEN_RANDOM_ITEM_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_ITEM_MANUFACTURE_REQ )
{
	KDBE_ITEM_MANUFACTURE_ACK kPacket;
    kPacket.m_iOK                       = NetError::ERR_ODBC_01;
	kPacket.m_mapInsertedItem			= kPacket_.m_mapInsertedItem;
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;

	bool bUpdateFailed = false;
    LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
    Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
	if( Query_InsertItemList( SEnum::GIR_MANUFACTURE, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#else
	if( Query_InsertItemList( kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
	//}}
	{
		START_LOG( cerr, L"������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_MANUFACTURE_11;
	}
	else
	{
		kPacket.m_iOK = NetError::NET_OK;
		//{{ 2012. 02. 07	�ڼ���	������ �� �۾� ( g_pManufactureItemManager:AddManufactureResultGroupWithRate �� �ʱ� ���� ���� ���� �߰� )
#ifdef SERV_ADD_SEALED_ITEM_SIGN

		std::set< int > setSealItem;

		for( size_t i=0; i < kPacket_.m_vecItemInfo.size(); ++i )
		{
			if( kPacket_.m_vecItemInfo[i].IsSealedItem() == true )   // ���� �������̶�� ��
			{
				setSealItem.insert( kPacket_.m_vecItemInfo[i].m_iItemID );
			}
		}
		Query_UpdateSealItem( setSealItem, kPacket.m_mapItemInfo );

#endif SERV_ADD_SEALED_ITEM_SIGN
		//}}
	}

	SendToUser( LAST_SENDER_UID, DBE_ITEM_MANUFACTURE_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_RESOLVE_ITEM_REQ )
{
	KDBE_RESOLVE_ITEM_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_bJackpot					= kPacket_.m_bJackpot;
	kPacket.m_mapInsertedItem			= kPacket_.m_mapInsertedItem;
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
	if( Query_InsertItemList( SEnum::GIR_RESOLVE_ITEM, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#else
	if( Query_InsertItemList( kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
	//}}
	{
		START_LOG( cerr, L"������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_RESOLVE_ITEM_05;
	}
	else
	{
		kPacket.m_iOK = NetError::NET_OK;
	}

	SendToUser( LAST_SENDER_UID, DBE_RESOLVE_ITEM_ACK, kPacket );
}

_IMPL_ON_FUNC( DBE_CHANGE_NICK_NAME_REQ, KEGS_CHANGE_NICK_NAME_REQ )
{
    KEGS_CHANGE_NICK_NAME_ACK kPacket;
    kPacket.m_iOK = NetError::ERR_ODBC_01;
    kPacket.m_iUnitUID = kPacket_.m_iUnitUID;
	kPacket.m_bCheckOnly = kPacket_.m_bCheckOnly;

	// ��밡���� �г������� üũ�� �ϱ�
	if( kPacket_.m_bCheckOnly )
	{
		DO_QUERY( L"exec dbo.gup_check_nickname", L"N\'%s\'", % kPacket_.m_wstrNickName );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}
		else
		{
			kPacket.m_iOK = NetError::ERR_RESTORE_NICK_NAME_04;
			goto end_proc;
		}

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			switch( kPacket.m_iOK )
			{
			case -1:
				kPacket.m_iOK = NetError::ERR_RESTORE_NICK_NAME_02;
				break;
			case -2:
				kPacket.m_iOK = NetError::ERR_RESTORE_NICK_NAME_03;
				break;			

			default:
				{
					START_LOG( cerr, L"���ǵ��� ���� ����! sp�� �ٲ���?" )
						<< BUILD_LOG( kPacket.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( kPacket_.m_wstrNickName )
						<< END_LOG;

					kPacket.m_iOK = NetError::ERR_RESTORE_NICK_NAME_04;
				}
			}

			goto end_proc;
		}
	}
	// ������ �г��� �����ϱ�
	else
	{
		DO_QUERY( L"exec dbo.gup_restore_nickname", L"%d, N\'%s\'", % kPacket_.m_iUnitUID % kPacket_.m_wstrNickName );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}
		else
		{
			kPacket.m_iOK = NetError::ERR_RESTORE_NICK_NAME_04;
			goto end_proc;
		}

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			switch( kPacket.m_iOK )
			{
			case -1:
				kPacket.m_iOK = NetError::ERR_RESTORE_NICK_NAME_00;
				break;
			case -2:
				kPacket.m_iOK = NetError::ERR_RESTORE_NICK_NAME_01;
				break;
			case -3:
				kPacket.m_iOK = NetError::ERR_RESTORE_NICK_NAME_02;
				break;
			case -4:
				kPacket.m_iOK = NetError::ERR_RESTORE_NICK_NAME_03;
				break;
			case -5:
				kPacket.m_iOK = NetError::ERR_RESTORE_NICK_NAME_04;
				break;

			default:
				{
					START_LOG( cerr, L"���ǵ��� ���� ����! sp�� �ٲ���?" )
						<< BUILD_LOG( kPacket.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( kPacket_.m_wstrNickName )
						<< END_LOG;

					kPacket.m_iOK = NetError::ERR_RESTORE_NICK_NAME_04;
				}
			}

			goto end_proc;
		}
		else
		{
			kPacket.m_wstrNickName = kPacket_.m_wstrNickName;
		}
	}    

end_proc:
    SendToUser( LAST_SENDER_UID, DBE_CHANGE_NICK_NAME_ACK, kPacket );
}

//{{ 2007. 8. 9  ������  ��������Ʈ ��Ŷ �ڵ鷯 - GSGameDBThread
IMPL_ON_FUNC( DBE_NEW_BLACKLIST_USER_REQ )
{
	KEGS_NEW_BLACKLIST_USER_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_kChatBlackListUnit.m_iUnitUID = kPacket_.m_iBlackListUnitUID;

	// ��������Ʈ ���� ����
	DO_QUERY( L"exec dbo.gup_insert_blacklist", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iBlackListUnitUID );
	
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK
				 >> kPacket.m_kChatBlackListUnit.m_wstrNickName;
			);
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		switch( kPacket.m_iOK )
		{
		case -1:    kPacket.m_iOK = NetError::ERR_BLACKLIST_03;   break;
		case -2:    kPacket.m_iOK = NetError::ERR_BLACKLIST_01;   break;
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_NEW_BLACKLIST_USER_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_DEL_BLACKLIST_USER_REQ )
{
	KDBE_DEL_BLACKLIST_USER_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iBlackListUnitUID = kPacket_.m_iBlackListUnitUID;

	DO_QUERY( L"exec dbo.gup_delete_blacklist", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iBlackListUnitUID );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		switch( kPacket.m_iOK )
		{
		case -1:    kPacket.m_iOK = NetError::ERR_BLACKLIST_02;   break;
		case -2:    kPacket.m_iOK = NetError::ERR_BLACKLIST_04;   break;
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_DEL_BLACKLIST_USER_ACK, kPacket );
}
//}}

//{{ 2009. 9. 22  ������	����ĳ��
IMPL_ON_FUNC( DBE_CHANGE_UNIT_CLASS_REQ )
{
	KDBE_CHANGE_UNIT_CLASS_ACK kPacket;
	kPacket.m_iOK		 = NetError::ERR_ODBC_01;
	kPacket.m_usEventID  = kPacket_.m_usEventID;
	kPacket.m_cUnitClass = kPacket_.m_cUnitClass;
	//{{ 2012. 07. 12	��μ�       ������ ���� �̺�Ʈ
#ifdef SERV_BUY_SECOND_JOB_CHANGE_ITEM_EVENT
	kPacket.m_iItemID = kPacket_.m_iItemID;
#endif SERV_BUY_SECOND_JOB_CHANGE_ITEM_EVENT
	//}}

#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013 // ���볯¥: 2013-06-27
	kPacket.m_iNewDefaultSkill1 = kPacket_.m_iNewDefaultSkill1;
	kPacket.m_iNewDefaultSkill2 = kPacket_.m_iNewDefaultSkill2;

	DO_QUERY( L"exec P_GUnit_UPD_UnitClass", L"%d, %d, %d, %d", % kPacket_.m_iUnitUID % (int)kPacket_.m_cUnitClass % kPacket_.m_iNewDefaultSkill1 % kPacket_.m_iNewDefaultSkill2 );
#else	// SERV_UPGRADE_SKILL_SYSTEM_2013
/*
	DO_QUERY( L"exec gup_update_unit_class", L"%d, %d", % kPacket_.m_iUnitUID % (int)kPacket_.m_cUnitClass );
*/
#endif	// SERV_UPGRADE_SKILL_SYSTEM_2013

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK )
	{
		switch( kPacket.m_iOK )
		{
		case -1:    kPacket.m_iOK = NetError::ERR_ADMIN_COMMAND_04;   break;
		case -2:    kPacket.m_iOK = NetError::ERR_ADMIN_COMMAND_05;   break;
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_CHANGE_UNIT_CLASS_ACK, kPacket );
}
//}}

IMPL_ON_FUNC( DBE_INSERT_BUY_PERSONAL_SHOP_ITEM_REQ )
{
	KDBE_INSERT_BUY_PERSONAL_SHOP_ITEM_ACK kPacket;
	kPacket.m_mapInsertedItem = kPacket_.m_mapInsertedItem;
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );

// 	START_LOG( cerr, L"���� ���� ȣ�� �ϳ�" )
// 		<< BUILD_LOG( kPacket_.m_iUnitUID )
// 		<< END_LOG;

	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
	if( Query_InsertItemList( SEnum::GIR_PERSONAL_SHOP, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#else
	if( Query_InsertItemList( kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
	//}}
	{
		START_LOG( cerr, L"������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;
	}

	SendToUser( LAST_SENDER_UID, DBE_INSERT_BUY_PERSONAL_SHOP_ITEM_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_ENCHANT_ITEM_REQ )
{
    KDBE_ENCHANT_ITEM_ACK kPacket;
    kPacket.m_iEnchantResult		  = kPacket_.m_iEnchantResult;
    kPacket.m_iItemUID				  = kPacket_.m_iItemUID;
	kPacket.m_iItemID				  = kPacket_.m_iItemID;
    kPacket.m_iLevelAfterEnchant	  = kPacket_.m_iLevelAfterEnchant;
    kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec gup_update_Enchant", L"%d, %d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iItemUID % kPacket_.m_iLevelAfterEnchant );

    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( kPacket.m_iOK );
        m_kODBC.EndFetch();
    }

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		std::wstring wstrEnchantResult = NetError::GetErrStr( kPacket_.m_iEnchantResult );

		START_LOG( cerr, L"��ȭ ���� ������Ʈ ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iItemUID )
			<< BUILD_LOG( kPacket_.m_iLevelAfterEnchant )
			<< BUILD_LOG( wstrEnchantResult )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_ENCHANT_ITEM_13;
		goto end_proc;
	}

	bool bUpdateFailed = false;
    LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

end_proc:
    SendToUser( LAST_SENDER_UID, DBE_ENCHANT_ITEM_ACK, kPacket );
}

//{{ 2008. 12. 21  ������	��ȭ ����
IMPL_ON_FUNC( DBE_RESTORE_ITEM_REQ )
{
	KDBE_RESTORE_ITEM_ACK kPacket;
	kPacket.m_iItemUID				  = kPacket_.m_iItemUID;
	kPacket.m_iLevelAfterEnchant	  = kPacket_.m_iLevelAfterEnchant;
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec gup_update_Enchant", L"%d, %d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iItemUID % kPacket_.m_iLevelAfterEnchant );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"���Ұ� ������ ���� ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iItemUID )
			<< BUILD_LOG( kPacket_.m_iLevelAfterEnchant )	
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_ENCHANT_ITEM_13;
		goto end_proc;
	}

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_RESTORE_ITEM_ACK, kPacket );
}
//}}

//{{ 2008. 3. 6  ������  ����
IMPL_ON_FUNC( DBE_SOCKET_ITEM_REQ )
{
	KDBE_SOCKET_ITEM_ACK kPacket;
	kPacket.m_vecSocketInfo = kPacket_.m_vecSocketInfo;
	//{{ 2010. 04. 15  ������	���� ����
#ifdef SERV_SOCKET_NEW
#else
	kPacket.m_iSocketResult = kPacket_.m_iSocketResult;
	kPacket.m_iItemID		= kPacket_.m_iItemID;
#endif SERV_SOCKET_NEW
	//}}
	kPacket.m_iItemUID		= kPacket_.m_iItemUID;
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;
	//{{ 2011. 01. 26	������	����Ʈ Ŭ���� ���� �߰�
#ifdef SERV_QUEST_CLEAR_EXPAND
	kPacket.m_iSocketUseCount = kPacket_.m_iSocketUseCount;
#endif SERV_QUEST_CLEAR_EXPAND
	//}}
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	int arrSocketInfo[4] = {0,0,0,0}; // DB ���̺��� �������̺� ����
	//{{ 2011. 07. 25    ��μ�    ������ �ɼ�ID ������ ������ ����
#ifdef SERV_ITEM_OPTION_DATA_SIZE
	std::vector< int >::const_iterator vit = kPacket.m_vecSocketInfo.begin();
#else
	std::vector< short >::const_iterator vit = kPacket.m_vecSocketInfo.begin();
#endif SERV_ITEM_OPTION_DATA_SIZE
	//}} 
	for( int iIdx = 0; iIdx < 4; iIdx++ )
	{		
		if( vit == kPacket.m_vecSocketInfo.end() )
			continue;

		arrSocketInfo[iIdx] = *vit;
		++vit;
	}

	DO_QUERY( L"exec dbo.gup_update_Socket", L"%d, %d, %d, %d, %d, %d",
		% kPacket_.m_iUnitUID		// @iUnitUID bigint
		% kPacket_.m_iItemUID		// @iItemUID bigint
		% arrSocketInfo[0]			// @iSoket1 smallint
		% arrSocketInfo[1]			// @iSoket2 smallint
		% arrSocketInfo[2]			// @iSoket3 smallint
		% arrSocketInfo[3]			// @iSoket4 smallint
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"������ ���� ���� ������Ʈ ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iItemUID )
			<< BUILD_LOG( arrSocketInfo[0] )
			<< BUILD_LOG( arrSocketInfo[1] )
			<< BUILD_LOG( arrSocketInfo[2] )
			<< BUILD_LOG( arrSocketInfo[3] )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_SOCKET_ITEM_10;
	}

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( LAST_SENDER_UID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_SOCKET_ITEM_ACK, kPacket );	
}
//}}

//{{ 2008. 3. 28  ������  ��õ / ���� ����
IMPL_ON_FUNC( DBE_RECOMMEND_USER_REQ )
{
	KDBE_RECOMMEND_USER_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iRecommendedUnitUID = 0;

	//{{ 2013. 03. 21	 ��õ�� �ý��� ���� - ��μ�
#ifdef SERV_RECOMMEND_LIST_EVENT
	// ����
	DO_QUERY( L"exec dbo.P_GRecommend_INS", L"%d, N\'%s\'",
		% kPacket_.m_iUnitUID
		% kPacket_.m_wstrNickName
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK
			>> kPacket.m_iRecommendedUnitUID );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		switch( kPacket.m_iOK )
		{
		case -1:    kPacket.m_iOK = NetError::ERR_RECOMMEND_USER_00;   break;
		case -2:    kPacket.m_iOK = NetError::ERR_RECOMMEND_USER_01;   break;
		case -3:	kPacket.m_iOK = NetError::ERR_RECOMMEND_USER_07;   break;		// �� �̻� ��õ �� �� ���� �����Դϴ�.
			//}}
		default:
			{
				START_LOG( cerr, L"��õ�� ����� �����Ͽ����ϴ�!" )
					<< BUILD_LOG( kPacket.m_iOK )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( kPacket_.m_wstrNickName )
					<< END_LOG;

				kPacket.m_iOK = NetError::ERR_ODBC_01;
			}
		}
	}
#else
	// ����
	DO_QUERY( L"exec dbo.gup_insert_recommend", L"%d, N\'%s\'",
		% kPacket_.m_iUnitUID
		% kPacket_.m_wstrNickName
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK
			>> kPacket.m_iRecommendedUnitUID );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		switch( kPacket.m_iOK )
		{
		case -1:    kPacket.m_iOK = NetError::ERR_RECOMMEND_USER_00;   break;
			//{{ 2008. 5. 16  ������  ü��ID ����
		case -3:	kPacket.m_iOK = NetError::ERR_GUEST_USER_01;	   break;
			//}}
		default:
			{
				START_LOG( cerr, L"��õ�� ����� �����Ͽ����ϴ�!" )
					<< BUILD_LOG( kPacket.m_iOK )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( kPacket_.m_wstrNickName )
					<< END_LOG;

				kPacket.m_iOK = NetError::ERR_ODBC_01;
			}
		}
	}
#endif SERV_RECOMMEND_LIST_EVENT
	//}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_RECOMMEND_USER_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_INSERT_REWARD_TO_POST_REQ )
{
	KDBE_INSERT_REWARD_TO_POST_ACK kPacket;
	kPacket.m_iOK					  	   = NetError::ERR_ODBC_01;
	kPacket.m_iRewardLetter.m_iFromUnitUID = kPacket_.m_iFromUnitUID;
	kPacket.m_iRewardLetter.m_iToUnitUID   = kPacket_.m_iToUnitUID;
	kPacket.m_iRewardLetter.m_cScriptType  = kPacket_.m_iRewardType;
	kPacket.m_iRewardLetter.m_iScriptIndex = kPacket_.m_iRewardID;
	kPacket.m_iRewardLetter.m_iQuantity	   = kPacket_.m_sQuantity;
	kPacket.m_iRewardLetter.m_wstrMessage  = kPacket_.m_wstrMessage;

	// ����
	DO_QUERY( L"exec dbo.gup_insert_post_item", L"%d, %d, %d, %d, %d, N\'%s\'",
		% kPacket_.m_iFromUnitUID
		% kPacket_.m_iToUnitUID
		% kPacket_.m_sQuantity
		% kPacket_.m_iRewardType
		% kPacket_.m_iRewardID
		% kPacket_.m_wstrMessage
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK
			>> kPacket.m_iRewardLetter.m_iPostNo
			>> kPacket.m_iRewardLetter.m_wstrRegDate );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		switch( kPacket.m_iOK )
		{
		case -1:    kPacket.m_iOK = NetError::ERR_REWARD_TO_POST_00;   break;
		}

		goto end_proc;
	}	

	//{{ 2010. 02. 24  ������	�� ����Ʈ �̺�Ʈ
#ifdef SERV_WEB_POINT_EVENT

	// �� ����Ʈ �̺�Ʈ ���� ���� �α�
	if( kPacket_.m_iRewardType == KPostItemInfo::LT_WEB_POINT_EVENT )
	{
		int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_insert_event_webpost", L"%d, %d", % kPacket_.m_iToUnitUID % kPacket.m_iRewardLetter.m_iPostNo );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"�� ����Ʈ �̺�Ʈ ���� ���� �α� ���� ����" )				
				<< BUILD_LOG( kPacket_.m_iToUnitUID )
				<< BUILD_LOG( kPacket.m_iRewardLetter.m_iPostNo )
				<< END_LOG;
		}
	}

#endif SERV_WEB_POINT_EVENT
	//}}

	//{{ 2013. 01. 21  ��Ը� �̺�Ʈ ���� �� ���� ���� ĳ���Ϳ��� �����ϴ� �ý��� - ��μ�
#ifdef SERV_FIRST_SELECT_UNIT_REWARD_SYSTEM
	kPacket.m_iDBIndex = kPacket_.m_iDBIndex;
#endif SERV_FIRST_SELECT_UNIT_REWARD_SYSTEM
	//}

end_proc:
	if( kPacket_.m_bGameServerEvent )
	{
		SendToServer( DBE_INSERT_REWARD_TO_POST_ACK, kPacket );
	}
	else
	{
		SendToUser( LAST_SENDER_UID, DBE_INSERT_REWARD_TO_POST_ACK, kPacket );
	}
}
//}}

//{{ 2008. 9. 18  ������	���� ����
IMPL_ON_FUNC( DBE_PREPARE_INSERT_LETTER_TO_POST_REQ )
{
	KDBE_PREPARE_INSERT_LETTER_TO_POST_ACK kPacket;
	kPacket.m_iOK			  = NetError::ERR_ODBC_01;

	//{{ 2012. 08. 23	�ڼ���	���� ���� ����
#ifdef SERV_TRADE_LOGIC_CHANGE_LETTER
	// ED ����ȭ
	int iOK = NetError::ERR_ODBC_00;
	DO_QUERY( L"exec dbo.P_GUnit_UPT_POST_GP", L"%d, %d", % kPacket_.m_iFromUnitUID % kPacket_.m_iIncrementED );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}

	if( iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"ED ����ȭ ����!!" )
			<< BUILD_LOG( LAST_SENDER_UID )
			<< BUILD_LOG( kPacket_.m_iFromUnitUID )
			<< BUILD_LOG( kPacket_.m_iIncrementED )
			<< BUILD_LOG( iOK )
			<< END_LOG;

		// ���� �ߴٸ� ED�� ������� �������� �Ѵ�.
		kPacket.m_iIncrementED = kPacket_.m_iIncrementED;
	}

	// ������ ����ȭ
	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iFromUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iFromUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}

	if( ( kPacket.m_iIncrementED != 0 ) ||
		( kPacket.m_kItemQuantityUpdate.m_mapQuantityChange.empty() != true ) ||
		( kPacket.m_kItemQuantityUpdate.m_vecDeleted.empty() != true ) )
	{
		// �۾� �� ����ȭ ���� ����!!
		kPacket.m_iOK = NetError::ERR_POST_LETTER_03;
		goto end_proc;
	}
#endif SERV_TRADE_LOGIC_CHANGE_LETTER
	//}}

	kPacket.m_iItemUID		  = kPacket_.m_iItemUID;
	kPacket.m_wstrToNickName  = kPacket_.m_wstrToNickName;
	kPacket.m_kSendLetter	  = kPacket_.m_kSendLetter;
	kPacket.m_iSendLetterCost = kPacket_.m_iSendLetterCost;

	//////////////////////////////////////////////////////////////////////////
	// ������ ÷�� �ֱ� ���� ���� üũ	
	DO_QUERY( L"exec dbo.gup_check_insert_post_item", L"%d, N\'%s\'", % kPacket_.m_iFromUnitUID % kPacket_.m_wstrToNickName );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		switch( kPacket.m_iOK )
		{
		case -99: kPacket.m_iOK = NetError::ERR_POST_LETTER_10; break;
		case -98: kPacket.m_iOK = NetError::ERR_POST_LETTER_13; break;
		case -97: kPacket.m_iOK = NetError::ERR_POST_LETTER_15; break;
		case -96: kPacket.m_iOK = NetError::ERR_POST_LETTER_17; break;
			//{{ 2012. 09. 26	������		���� ���� ���� ����
#ifdef SERV_POST_COPY_BUG_FIX
		case -100: kPacket.m_iOK = NetError::ERR_POST_LETTER_20; break;
#endif SERV_POST_COPY_BUG_FIX
			//}}

		default:
			START_LOG( cerr, L"���ǵ��� ���� ����~!" )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( kPacket_.m_iFromUnitUID )
				<< BUILD_LOG( kPacket_.m_wstrToNickName )
				<< END_LOG;
			break;
		}
	}
#ifdef	SERV_SHARING_BANK_TEST
	else
	{
		if( kPacket_.m_iItemUID > 0 )
		{
			DO_QUERY( L"exec dbo.P_GItem_Check", L"%d, %d", % kPacket_.m_iItemUID % kPacket_.m_iFromUnitUID );

			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( kPacket.m_iOK  );
				m_kODBC.EndFetch();
			}
		}

		if(kPacket.m_iOK == -1)
			kPacket.m_iOK = NetError::ERR_POST_LETTER_06;
	}
#endif	SERV_SHARING_BANK_TEST

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_PREPARE_INSERT_LETTER_TO_POST_ACK, kPacket );
}

//{{ 2012. 08. 20	�ڼ���	���� ���� ����
//////////////////////////////////////////////////////////////////////////
#ifdef SERV_TRADE_LOGIC_CHANGE_LETTER
//////////////////////////////////////////////////////////////////////////
IMPL_ON_FUNC( DBE_INSERT_LETTER_TO_POST_REQ )
{
	KDBE_INSERT_LETTER_TO_POST_ACK kPacket;
	//{{ 2011. 07. 25    ��μ�    ������ �ɼ�ID ������ ������ ����
#ifdef SERV_ITEM_OPTION_DATA_SIZE
	std::vector< int >::const_iterator vit;
#else
	std::vector< short >::const_iterator vit;
#endif SERV_ITEM_OPTION_DATA_SIZE
	//}} 
	kPacket.m_iOK					  = NetError::ERR_ODBC_01;
	kPacket.m_wstrToNickName		  = kPacket_.m_wstrToNickName;	
	kPacket.m_kSendLetter			  = kPacket_.m_kSendLetter;
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_iSendLetterCost		  = kPacket_.m_iSendLetterCost;
	
	if( kPacket_.m_kSendLetter.IsSystemLetter() != true )
	{
		const byte	iTradeType		= 0;	// ���� ������
		int			iED				= -kPacket_.m_iSendLetterCost;
		UidType		iItemUID		= 0;
		int			iQuantity		= 0;
		byte		iPostItemType	= 3;	// ÷�� ����
		byte		iUsageType		= static_cast<byte>( CXSLItem::PT_INFINITY );

		if( 0 < kPacket_.m_kSendLetter.m_iScriptIndex )
		{
			if( kPacket_.m_kSendLetter.m_iScriptIndex == CXSLItem::EDI_BRONZE_ED )
			{
				iPostItemType	= 2;	// ED ÷��
				iED				-= kPacket_.m_kSendLetter.m_iQuantity;
				iQuantity		= kPacket_.m_kSendLetter.m_iQuantity;
			}
			else
			{
				iPostItemType = 1;	// ������ ÷��
				iItemUID  = kPacket_.m_kSendLetter.m_iItemUID;
				iQuantity = kPacket_.m_kSendLetter.m_iQuantity;
				iUsageType= kPacket_.m_kSendLetter.m_iUsageType;
			}
		}

		DO_QUERY( L"exec dbo.P_GItem_Total_Post",
			L"%d, %d, N\'%s\', %d, %d, %d, %d, %d, N\'%s\', N\'%s\', %d",
			% iTradeType
			% kPacket_.m_iFromUnitUID
			% kPacket_.m_wstrToNickName
			% iED
			% iItemUID
			% iQuantity
			% iUsageType
			% iPostItemType
			% kPacket_.m_kSendLetter.m_wstrTitle
			% kPacket_.m_kSendLetter.m_wstrMessage
			% kPacket_.m_kSendLetter.m_iPostNo
			);

		UidType iGarbageValue;

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK
				>> kPacket.m_kSendLetter.m_iPostNo
				>> kPacket.m_kSendLetter.m_iToUnitUID
				>> kPacket.m_kSendLetter.m_wstrRegDate
				>> iGarbageValue
				);
			m_kODBC.EndFetch();
		}

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"���� ���� ����!! ���� �Ͼ���� �ȵ�!!!!! ���� �̽� �߻�!")
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( iTradeType )
				<< BUILD_LOG( kPacket_.m_iFromUnitUID )
				<< BUILD_LOG( kPacket_.m_wstrToNickName )
				<< BUILD_LOG( iED )
				<< BUILD_LOG( iItemUID )
				<< BUILD_LOG( iQuantity )
				<< BUILD_LOG( iUsageType )
				<< BUILD_LOG( iPostItemType )
				<< BUILD_LOG( kPacket_.m_kSendLetter.m_wstrTitle )
				<< BUILD_LOG( kPacket_.m_kSendLetter.m_wstrMessage )
				<< BUILD_LOG( kPacket_.m_kSendLetter.m_iPostNo )
				<< END_LOG;

			kPacket.m_kItemQuantityUpdate = kPacket_.m_kItemQuantityUpdate;
			goto end_proc;
		}
	}
	else
	{
		// 1. �����۵��� �����..
		bool bUpdateFailed = false;
		LIF( Query_UpdateItemQuantity( kPacket_.m_iFromUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
		Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );	

		//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
		if( bUpdateFailed )
		{
			START_LOG( cout, L"���а� ������Ʈ ����!" )
				<< BUILD_LOG( kPacket_.m_iFromUnitUID );

			std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
			for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
			{
				START_LOG( cout, L"������ ����" )
					<< BUILD_LOG( mitQC->first )
					<< BUILD_LOG( mitQC->second );
			}
		}
		//}}

		int arrSocketInfo[4] = {0,0,0,0}; // DB ���̺��� �������̺� ����
		vit = kPacket_.m_kSendLetter.m_vecItemSocket.begin();
		for( int iIdx = 0; iIdx < 4; iIdx++ )
		{		
			if( vit == kPacket_.m_kSendLetter.m_vecItemSocket.end() )
				continue;

			arrSocketInfo[iIdx] = *vit;
			++vit;
		}
		
		//{{ 2013. 05. 28	������	������ ����
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05

		int arrRandomSocketInfo[CXSLSocketItem::RSC_MAX] = {0,}; // DB ���̺��� �������̺� ����

		std::vector< int >::const_iterator vitSocket = kPacket_.m_kSendLetter.m_vecRandomSocket.begin();
		for( int iIdx = 0; iIdx < CXSLSocketItem::RSC_MAX; ++iIdx )
		{
			if( vitSocket == kPacket_.m_kSendLetter.m_vecRandomSocket.end() )
				continue;

			arrRandomSocketInfo[iIdx] = *vitSocket;
			++vitSocket;
		}

		// 2. ��ü���� ������ �ִ´�..
		DO_QUERY( L"exec dbo.P_GPost_INS",
			L"%d, N\'%s\', %d, %d, %d, N\'%s\', N\'%s\', %d, %d, %d, "
			L"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
			% kPacket_.m_iFromUnitUID					// @FromUnitUID bigint
			% kPacket_.m_wstrToNickName					// @ToUnitNickName nvarchar(16)
			% kPacket_.m_kSendLetter.m_iQuantity		// @Quantity  int
			% static_cast<int>(kPacket_.m_kSendLetter.m_cScriptType) // @ScType   tinyint
			% kPacket_.m_kSendLetter.m_iScriptIndex		// @ScIndex  int
			% kPacket_.m_kSendLetter.m_wstrTitle		// @Title   nvarchar(38)
			% kPacket_.m_kSendLetter.m_wstrMessage		// @Message  nvarchar(600)
			% static_cast<int>(kPacket_.m_kSendLetter.m_cEnchantLevel)	// @ELevel   tinyint
			% arrSocketInfo[0]							// @Socket1  smallint
			% arrSocketInfo[1]							// @Socket2  smallint
			% arrSocketInfo[2]							// @Socket3  smallint
			% arrSocketInfo[3]							// @Socket4  smallint
			% static_cast<int>(kPacket_.m_kSendLetter.m_kAttribEnchantInfo.m_cAttribEnchant0 )	// @Attribute1  tinyint
			% static_cast<int>(kPacket_.m_kSendLetter.m_kAttribEnchantInfo.m_cAttribEnchant1 )	// @Attribute2  tinyint
			% static_cast<int>(kPacket_.m_kSendLetter.m_kAttribEnchantInfo.m_cAttribEnchant2 )	// @Attribute3  tinyint
			% static_cast<int>(kPacket_.m_kSendLetter.m_ucSealData)
			% arrRandomSocketInfo[0]					// @Socket1R
			% arrRandomSocketInfo[1]					// @Socket2R
			% arrRandomSocketInfo[2]					// @Socket3R
			% arrRandomSocketInfo[3]					// @Socket4R
			% arrRandomSocketInfo[4]					// @Socket5R
			);
#else
		DO_QUERY( L"exec dbo.gup_insert_post_item_new",
			L"%d, N\'%s\', %d, %d, %d, N\'%s\', N\'%s\', %d, %d, %d, "
			L"%d, %d, %d, %d, %d, %d",
			% kPacket_.m_iFromUnitUID					// @FromUnitUID bigint
			% kPacket_.m_wstrToNickName					// @ToUnitNickName nvarchar(16)
			% kPacket_.m_kSendLetter.m_iQuantity		// @Quantity  int
			% static_cast<int>(kPacket_.m_kSendLetter.m_cScriptType) // @ScType   tinyint
			% kPacket_.m_kSendLetter.m_iScriptIndex		// @ScIndex  int
			% kPacket_.m_kSendLetter.m_wstrTitle		// @Title   nvarchar(38)
			% kPacket_.m_kSendLetter.m_wstrMessage		// @Message  nvarchar(600)
			% static_cast<int>(kPacket_.m_kSendLetter.m_cEnchantLevel)	// @ELevel   tinyint
			% arrSocketInfo[0]							// @Socket1  smallint
			% arrSocketInfo[1]							// @Socket2  smallint
			% arrSocketInfo[2]							// @Socket3  smallint
			% arrSocketInfo[3]							// @Socket4  smallint
			% static_cast<int>(kPacket_.m_kSendLetter.m_kAttribEnchantInfo.m_cAttribEnchant0 )	// @Attribute1  tinyint
			% static_cast<int>(kPacket_.m_kSendLetter.m_kAttribEnchantInfo.m_cAttribEnchant1 )	// @Attribute2  tinyint
			% static_cast<int>(kPacket_.m_kSendLetter.m_kAttribEnchantInfo.m_cAttribEnchant2 )	// @Attribute3  tinyint
			% static_cast<int>(kPacket_.m_kSendLetter.m_ucSealData)
			);
#endif SERV_NEW_ITEM_SYSTEM_2013_05
		//}}		

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK
				>> kPacket.m_kSendLetter.m_iPostNo
				>> kPacket.m_kSendLetter.m_iToUnitUID
				>> kPacket.m_kSendLetter.m_wstrRegDate );
			m_kODBC.EndFetch();
		}

		if( kPacket.m_iOK != NetError::NET_OK )
		{	
			START_LOG( cerr, L"���� ���� DB isnert����! �Ͼ�� �ȵǴ� ����!!" )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( kPacket_.m_iFromUnitUID )
				<< BUILD_LOG( kPacket_.m_wstrToNickName	)
				<< BUILD_LOG( kPacket_.m_kSendLetter.m_iQuantity )
				<< BUILD_LOGc( kPacket_.m_kSendLetter.m_cScriptType )
				<< BUILD_LOG( kPacket_.m_kSendLetter.m_iScriptIndex )
				<< BUILD_LOG( kPacket_.m_kSendLetter.m_wstrTitle )
				<< BUILD_LOG( kPacket_.m_kSendLetter.m_wstrMessage )
				<< BUILD_LOGc( kPacket_.m_kSendLetter.m_cEnchantLevel )
				<< BUILD_LOG( arrSocketInfo[0] )
				<< BUILD_LOG( arrSocketInfo[1] )
				<< BUILD_LOG( arrSocketInfo[2] )
				<< BUILD_LOG( arrSocketInfo[3] )
				<< BUILD_LOGc( kPacket_.m_kSendLetter.m_kAttribEnchantInfo.m_cAttribEnchant0 )
				<< BUILD_LOGc( kPacket_.m_kSendLetter.m_kAttribEnchantInfo.m_cAttribEnchant1 )
				//{{ 2009. 11. 9  ������	Ʈ���üӼ�
				<< BUILD_LOGc( kPacket_.m_kSendLetter.m_kAttribEnchantInfo.m_cAttribEnchant2 )
				//}}
				//{{ 2009. 8. 27  ������	�к�
				<< BUILD_LOGc( kPacket_.m_kSendLetter.m_ucSealData )
				//}}
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_POST_LETTER_03;
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_INSERT_LETTER_TO_POST_ACK, kPacket );
}
//}}
//////////////////////////////////////////////////////////////////////////
#else
//////////////////////////////////////////////////////////////////////////
IMPL_ON_FUNC( DBE_INSERT_LETTER_TO_POST_REQ )
{
	KDBE_INSERT_LETTER_TO_POST_ACK kPacket;
	//{{ 2011. 07. 25    ��μ�    ������ �ɼ�ID ������ ������ ����
#ifdef SERV_ITEM_OPTION_DATA_SIZE
	std::vector< int >::const_iterator vit;
#else
	std::vector< short >::const_iterator vit;
#endif SERV_ITEM_OPTION_DATA_SIZE
	//}} 
	kPacket.m_iOK					  = NetError::ERR_ODBC_01;
	kPacket.m_wstrToNickName		  = kPacket_.m_wstrToNickName;	
	kPacket.m_kSendLetter			  = kPacket_.m_kSendLetter;
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_iSendLetterCost		  = kPacket_.m_iSendLetterCost;

	// 1. �����۵��� �����..
	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iFromUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );	

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iFromUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	int arrSocketInfo[4] = {0,0,0,0}; // DB ���̺��� �������̺� ����
	vit = kPacket_.m_kSendLetter.m_vecItemSocket.begin();
	for( int iIdx = 0; iIdx < 4; iIdx++ )
	{		
		if( vit == kPacket_.m_kSendLetter.m_vecItemSocket.end() )
			continue;

		arrSocketInfo[iIdx] = *vit;
		++vit;
	}
	
	//{{ 2013. 05. 28	������	������ ����
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05

	int arrRandomSocketInfo[CXSLSocketItem::RSC_MAX] = {0,}; // DB ���̺��� �������̺� ����

	std::vector< int >::const_iterator vitSocket = kPacket_.m_kSendLetter.m_vecRandomSocket.begin();
	for( int iIdx = 0; iIdx < CXSLSocketItem::RSC_MAX; ++iIdx )
	{
		if( vitSocket == kPacket_.m_kSendLetter.m_vecRandomSocket.end() )
			continue;

		arrRandomSocketInfo[iIdx] = *vitSocket;
		++vitSocket;
	}

	// 2. ��ü���� ������ �ִ´�..
	DO_QUERY( L"exec dbo.P_GPost_INS",
		L"%d, N\'%s\', %d, %d, %d, N\'%s\', N\'%s\', %d, %d, %d, "
		L"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
		% kPacket_.m_iFromUnitUID					// @FromUnitUID bigint
		% kPacket_.m_wstrToNickName					// @ToUnitNickName nvarchar(16)
		% kPacket_.m_kSendLetter.m_iQuantity		// @Quantity  int
		% static_cast<int>(kPacket_.m_kSendLetter.m_cScriptType) // @ScType   tinyint
		% kPacket_.m_kSendLetter.m_iScriptIndex		// @ScIndex  int
		% kPacket_.m_kSendLetter.m_wstrTitle		// @Title   nvarchar(38)
		% kPacket_.m_kSendLetter.m_wstrMessage		// @Message  nvarchar(600)
		% static_cast<int>(kPacket_.m_kSendLetter.m_cEnchantLevel)	// @ELevel   tinyint
		% arrSocketInfo[0]							// @Socket1  smallint
		% arrSocketInfo[1]							// @Socket2  smallint
		% arrSocketInfo[2]							// @Socket3  smallint
		% arrSocketInfo[3]							// @Socket4  smallint
		% static_cast<int>(kPacket_.m_kSendLetter.m_kAttribEnchantInfo.m_cAttribEnchant0 )	// @Attribute1  tinyint
		% static_cast<int>(kPacket_.m_kSendLetter.m_kAttribEnchantInfo.m_cAttribEnchant1 )	// @Attribute2  tinyint
		% static_cast<int>(kPacket_.m_kSendLetter.m_kAttribEnchantInfo.m_cAttribEnchant2 )	// @Attribute3  tinyint
		% static_cast<int>(kPacket_.m_kSendLetter.m_ucSealData)
		% arrRandomSocketInfo[0]					// @Socket1R
		% arrRandomSocketInfo[1]					// @Socket2R
		% arrRandomSocketInfo[2]					// @Socket3R
		% arrRandomSocketInfo[3]					// @Socket4R
		% arrRandomSocketInfo[4]					// @Socket5R
		);
#else
	DO_QUERY( L"exec dbo.gup_insert_post_item_new",
		L"%d, N\'%s\', %d, %d, %d, N\'%s\', N\'%s\', %d, %d, %d, "
		L"%d, %d, %d, %d, %d, %d",
		% kPacket_.m_iFromUnitUID					// @FromUnitUID bigint
		% kPacket_.m_wstrToNickName					// @ToUnitNickName nvarchar(16)
		% kPacket_.m_kSendLetter.m_iQuantity		// @Quantity  int
		% static_cast<int>(kPacket_.m_kSendLetter.m_cScriptType) // @ScType   tinyint
		% kPacket_.m_kSendLetter.m_iScriptIndex		// @ScIndex  int
		% kPacket_.m_kSendLetter.m_wstrTitle		// @Title   nvarchar(38)
		% kPacket_.m_kSendLetter.m_wstrMessage		// @Message  nvarchar(600)
		% static_cast<int>(kPacket_.m_kSendLetter.m_cEnchantLevel)	// @ELevel   tinyint
		% arrSocketInfo[0]							// @Socket1  smallint
		% arrSocketInfo[1]							// @Socket2  smallint
		% arrSocketInfo[2]							// @Socket3  smallint
		% arrSocketInfo[3]							// @Socket4  smallint
		% static_cast<int>(kPacket_.m_kSendLetter.m_kAttribEnchantInfo.m_cAttribEnchant0 )	// @Attribute1  tinyint
		% static_cast<int>(kPacket_.m_kSendLetter.m_kAttribEnchantInfo.m_cAttribEnchant1 )	// @Attribute2  tinyint
		% static_cast<int>(kPacket_.m_kSendLetter.m_kAttribEnchantInfo.m_cAttribEnchant2 )	// @Attribute3  tinyint
		% static_cast<int>(kPacket_.m_kSendLetter.m_ucSealData)
		);
#endif SERV_NEW_ITEM_SYSTEM_2013_05
	//}}	

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK
			>> kPacket.m_kSendLetter.m_iPostNo
			>> kPacket.m_kSendLetter.m_iToUnitUID
			>> kPacket.m_kSendLetter.m_wstrRegDate );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{	
		START_LOG( cerr, L"���� ���� DB isnert����! �Ͼ�� �ȵǴ� ����!!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iFromUnitUID )
			<< BUILD_LOG( kPacket_.m_wstrToNickName	)
			<< BUILD_LOG( kPacket_.m_kSendLetter.m_iQuantity )
			<< BUILD_LOGc( kPacket_.m_kSendLetter.m_cScriptType )
			<< BUILD_LOG( kPacket_.m_kSendLetter.m_iScriptIndex )
			<< BUILD_LOG( kPacket_.m_kSendLetter.m_wstrTitle )
			<< BUILD_LOG( kPacket_.m_kSendLetter.m_wstrMessage )
			<< BUILD_LOGc( kPacket_.m_kSendLetter.m_cEnchantLevel )
			<< BUILD_LOG( arrSocketInfo[0] )
			<< BUILD_LOG( arrSocketInfo[1] )
			<< BUILD_LOG( arrSocketInfo[2] )
			<< BUILD_LOG( arrSocketInfo[3] )
			<< BUILD_LOGc( kPacket_.m_kSendLetter.m_kAttribEnchantInfo.m_cAttribEnchant0 )
			<< BUILD_LOGc( kPacket_.m_kSendLetter.m_kAttribEnchantInfo.m_cAttribEnchant1 )
			//{{ 2009. 11. 9  ������	Ʈ���üӼ�
			<< BUILD_LOGc( kPacket_.m_kSendLetter.m_kAttribEnchantInfo.m_cAttribEnchant2 )
			//}}
			//{{ 2009. 8. 27  ������	�к�
			<< BUILD_LOGc( kPacket_.m_kSendLetter.m_ucSealData )
			//}}
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_POST_LETTER_03;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_INSERT_LETTER_TO_POST_ACK, kPacket );
}
//////////////////////////////////////////////////////////////////////////
#endif SERV_TRADE_LOGIC_CHANGE_LETTER
//}}

//{{ 2008. 9. 18  ������	������
IMPL_ON_FUNC( DBE_GET_POST_LETTER_LIST_REQ )
{
	KDBE_GET_POST_LETTER_LIST_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	//{{ 2012. 11. 08	�ڼ���	�������� �����
#ifdef SERV_ELIOS_INVESTIGATIONS
	kPacket.m_bIsChannelChange				= kPacket_.m_bIsChannelChange;
	kPacket.m_bEliosInvestigationsReward	= kPacket_.m_bEliosInvestigationsReward;
#endif SERV_ELIOS_INVESTIGATIONS
	//}}

	// ����
	//{{ 2013. 05. 28	������	������ ����
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
	DO_QUERY( L"exec dbo.P_Gpost_SEL", L"%d", % kPacket_.m_iUnitUID );
#else
	DO_QUERY( L"exec dbo.gup_get_post_item_new", L"%d", % kPacket_.m_iUnitUID );
#endif SERV_NEW_ITEM_SYSTEM_2013_05
	//}}	

	while( m_kODBC.Fetch() )
	{
		//{{ 2011. 07. 25    ��μ�    ������ �ɼ�ID ������ ������ ����
//#ifdef SERV_ITEM_OPTION_DATA_SIZE
		int arrSocketOption[4] = {0,0,0,0};
//#else
//		short arrSocketOption[4] = {0,0,0,0};
//#endif SERV_ITEM_OPTION_DATA_SIZE
		//}} 
		//{{ 2013. 05. 28	������	������ ����
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
		int arrRandomSocketOption[CXSLSocketItem::RSC_MAX] = {0,};
#endif SERV_NEW_ITEM_SYSTEM_2013_05
		//}}

		KPostItemInfo kPostItemInfo;
		bool bReceived = false;

		//{{ 2013. 05. 28	������	������ ����
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
		FETCH_DATA( kPostItemInfo.m_iPostNo
			>> kPostItemInfo.m_wstrFromNickName			
			>> kPostItemInfo.m_iToUnitUID
			>> kPostItemInfo.m_iQuantity
			>> kPostItemInfo.m_cScriptType
			>> kPostItemInfo.m_iScriptIndex
			>> kPostItemInfo.m_wstrRegDate
			>> bReceived
			>> kPostItemInfo.m_bRead
			>> kPostItemInfo.m_wstrTitle
			>> kPostItemInfo.m_wstrMessage
			>> kPostItemInfo.m_cEnchantLevel
			>> arrSocketOption[0]
			>> arrSocketOption[1]
			>> arrSocketOption[2]
			>> arrSocketOption[3]
			>> kPostItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant0
			>> kPostItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant1
			>> kPostItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant2
			>> arrRandomSocketOption[0]
			>> arrRandomSocketOption[1]
			>> arrRandomSocketOption[2]
			>> arrRandomSocketOption[3]
			>> arrRandomSocketOption[4]
			>> kPostItemInfo.m_ucSealData
			>> kPostItemInfo.m_iFromUnitUID
			);
#else
		FETCH_DATA( kPostItemInfo.m_iPostNo
			>> kPostItemInfo.m_wstrFromNickName
			>> kPostItemInfo.m_iToUnitUID
			>> kPostItemInfo.m_iQuantity
			>> kPostItemInfo.m_cScriptType
			>> kPostItemInfo.m_iScriptIndex
			>> kPostItemInfo.m_wstrRegDate
			>> bReceived
			>> kPostItemInfo.m_bRead
			>> kPostItemInfo.m_wstrTitle
			>> kPostItemInfo.m_wstrMessage
			>> kPostItemInfo.m_cEnchantLevel
			>> arrSocketOption[0]
			>> arrSocketOption[1]
			>> arrSocketOption[2]
			>> arrSocketOption[3]
			>> kPostItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant0
			>> kPostItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant1
			>> kPostItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant2
			>> kPostItemInfo.m_ucSealData
			>> kPostItemInfo.m_iFromUnitUID
			);
#endif SERV_NEW_ITEM_SYSTEM_2013_05
		//}}	

		// ÷�θ� �̹� �޾Ҵٸ� �ʱ�ȭ ó��
		if( bReceived )
		{
			kPostItemInfo.m_iScriptIndex = 0;
			kPostItemInfo.m_iQuantity	 = 0;
		}
		else
		{
			//{{ Iruha : 2026-08-27 // VS2010 port: iCheckIdx is used after the loop, which
			// VC7.1's non-conformant /Zc:forScope- tolerated; VC10 requires the loop
			// variable to outlive the loop, so it's hoisted here.
			int iCheckIdx;
			for( iCheckIdx = 3; iCheckIdx >= 0; --iCheckIdx )
			{
				if( arrSocketOption[iCheckIdx] != 0 )
					break;
			}
			//}}

			for( int iIdx = 0; iIdx <= iCheckIdx; ++iIdx )
			{
				kPostItemInfo.m_vecItemSocket.push_back( arrSocketOption[iIdx] );
			}

			//{{ 2013. 05. 28	������	������ ����
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
			// 2-1. ���� ���� ���� ������Ʈ
			//{{ Iruha : 2026-08-27 // VS2010 port: same for-scope hoist as above
			for( iCheckIdx = CXSLSocketItem::RSC_MAX - 1; iCheckIdx >= 0; --iCheckIdx )
			{
				if( arrRandomSocketOption[iCheckIdx] != 0 )
					break;
			}
			//}}

			for( int iIdx = 0; iIdx <= iCheckIdx; ++iIdx )
			{
				kPostItemInfo.m_vecRandomSocket.push_back( arrRandomSocketOption[iIdx] );
			}
#endif SERV_NEW_ITEM_SYSTEM_2013_05
			//}}
		}

		// ��ü�� ���� �ֱ�
		kPacket.m_vecPostItem.push_back( kPostItemInfo );
	}

	// ��������Ʈ
	DO_QUERY( L"exec dbo.gup_get_post_blacklist", L"%d", % kPacket_.m_iUnitUID );

	while( m_kODBC.Fetch() )
	{
		std::wstring wstrNickName;
		FETCH_DATA( wstrNickName );

		// ��ü�� ���� �ֱ�
		kPacket.m_vecBlackList.push_back( wstrNickName );
	}

	kPacket.m_iOK = NetError::NET_OK;

end_proc:
	LOG_SUCCESS( kPacket.m_iOK == NetError::NET_OK )
		<< BUILD_LOG( LAST_SENDER_UID )
		<< END_LOG;

	SendToUser( LAST_SENDER_UID, DBE_GET_POST_LETTER_LIST_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_READ_LETTER_NOT )
{
	int iOK = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec dbo.gup_update_read_post_letter", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iPostNo );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}

	if( iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"���� �б� DB������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iPostNo )
			<< BUILD_LOG( iOK )
			<< END_LOG;
	}

end_proc:
	return;
}

//{{ 2012. 08. 20	�ڼ���	���� ���� ����
#ifdef SERV_TRADE_LOGIC_CHANGE_LETTER
IMPL_ON_FUNC( DBE_GET_ITEM_FROM_LETTER_REQ )
{
	KDBE_GET_ITEM_FROM_LETTER_ACK kPacket;
	kPacket.m_iOK			  = NetError::ERR_ODBC_01;
	kPacket.m_iPostNo		  = kPacket_.m_iPostNo;
	kPacket.m_bSystemLetter	  = kPacket_.m_bSystemLetter;
	kPacket.m_mapInsertedItem = kPacket_.m_mapInsertedItem;
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_iED			  = kPacket_.m_iED;

	// ED ����ȭ
	int iOK = NetError::ERR_ODBC_00;
	DO_QUERY( L"exec dbo.P_GUnit_UPT_POST_GP", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iIncrementED );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}

	if( iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"ED ����ȭ ����!!" )
			<< BUILD_LOG( LAST_SENDER_UID )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iIncrementED )
			<< BUILD_LOG( iOK )
			<< END_LOG;

		// ���� �ߴٸ� ED�� ������� �������� �Ѵ�.
		kPacket.m_iIncrementED = kPacket_.m_iIncrementED;
	}

	// ������ ����ȭ
	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}

	if( ( kPacket.m_iIncrementED != 0 ) ||
		( kPacket.m_kItemQuantityUpdate.m_mapQuantityChange.empty() != true ) ||
		( kPacket.m_kItemQuantityUpdate.m_vecDeleted.empty() != true ) )
	{
		// �۾� �� ����ȭ ���� ����!!
		kPacket.m_iOK = NetError::ERR_POST_LETTER_06;
		goto end_proc;
	}

	if( kPacket_.m_bSystemLetter != true )
	{
		kPacket.m_vecUpdatedInventorySlot.clear();
		if( 0 < kPacket_.m_iED )
		{
			const byte	iTradeType		= 1;				// ���� �ޱ�
			int			iED				= 0;
			UidType		iItemUID		= 0;
			int			iQuantity		= 0;
			byte		iPostItemType	= 3;				// ÷�� ����
			byte		iUsageType		= static_cast<byte>( CXSLItem::PT_INFINITY );

			DO_QUERY( L"exec dbo.P_GItem_Total_Post",
				L"%d, %d, N\'%s\', %d, %d, %d, %d, %d, N\'%s\', N\'%s\', %d",
				% iTradeType
				% kPacket_.m_iUnitUID
				% L""
				% iED
				% iItemUID
				% iQuantity
				% iUsageType
				% iPostItemType
				% L""
				% L""
				% kPacket_.m_iPostNo
				);

			int				iGarbagePostNo;
			UidType			iGarbageUnitUID;
			std::wstring	wstrGarbageRegDate;

			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( kPacket.m_iOK
					>> iGarbagePostNo
					>> iGarbageUnitUID
					>> wstrGarbageRegDate
					>> iItemUID
					);
				m_kODBC.EndFetch();
			}

			if( kPacket.m_iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"������ �������� ����!!")
					<< BUILD_LOG( kPacket.m_iOK )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( kPacket_.m_iPostNo )
					<< END_LOG;
				goto end_proc;
			}
		}
		else
		{
			if( kPacket_.m_vecItemInfo.empty() == true )
			{
                START_LOG( cerr, L"���⼭ ���� ������ 0���ϸ��� ����. �Ͼ���� �ȵǴ� ����!" )
					<< BUILD_LOG( kPacket_.m_vecItemInfo.size() )
					<< END_LOG;

				kPacket.m_iOK = NetError::ERR_UNKNOWN;
				goto end_proc;
			}

			BOOST_TEST_FOREACH( const KItemInfo&, kItem, kPacket_.m_vecItemInfo )
			{
				const byte	iTradeType		= 1;				// ���� �ޱ�
				int			iED				= 0;
				UidType		iItemUID		= 0;
				int			iQuantity		= 0;
				byte		iPostItemType	= 3;				// ÷�� ����
				byte		iUsageType		= static_cast<byte>( kItem.m_cUsageType );

				DO_QUERY( L"exec dbo.P_GItem_Total_Post",
					L"%d, %d, N\'%s\', %d, %d, %d, %d, %d, N\'%s\', N\'%s\', %d",
					% iTradeType
					% kPacket_.m_iUnitUID
					% L""
					% iED
					% iItemUID
					% iQuantity
					% iUsageType
					% iPostItemType
					% L""
					% L""
					% kPacket_.m_iPostNo
					);

				int				iGarbagePostNo;
				UidType			iGarbageUnitUID;
				std::wstring	wstrGarbageRegDate;

				if( m_kODBC.BeginFetch() )
				{
					FETCH_DATA( kPacket.m_iOK
						>> iGarbagePostNo
						>> iGarbageUnitUID
						>> wstrGarbageRegDate
						>> iItemUID
						);
					m_kODBC.EndFetch();
				}

				if( kPacket.m_iOK != NetError::NET_OK )
				{
					START_LOG( cerr, L"������ �������� ����!!")
						<< BUILD_LOG( kPacket.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( kPacket_.m_iPostNo )
						<< END_LOG;
					goto end_proc;
				}

				KInventoryItemInfo kInventoryItemInfo;
				kInventoryItemInfo.m_iItemUID	= iItemUID;
				kInventoryItemInfo.m_kItemInfo	= kItem;
				kPacket.m_vecUpdatedInventorySlot.push_back( kInventoryItemInfo );
				break;
			}
		}
	}
	else
	{
		// �ý��� ������� �����..
		DO_QUERY( L"exec dbo.P_GPost_DEL_System", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iPostNo );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"���� ÷�ι� �޾ư��� DB������Ʈ ����!" )
				<< BUILD_LOG( kPacket_.m_iPostNo )
				<< BUILD_LOG( kPacket.m_iOK )
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_POST_LETTER_06;
			goto end_proc;
		}

		//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
		if( Query_InsertItemList( SEnum::GIR_FROM_LETTER, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#else
		if( Query_InsertItemList( kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
			//}}
		{
			START_LOG( cerr, L"������ ������Ʈ ����!" )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( kPacket_.m_iPostNo )
				<< END_LOG;
		}
	}

end_proc:
SendToUser( LAST_SENDER_UID, DBE_GET_ITEM_FROM_LETTER_ACK, kPacket );
}

#else	// SERV_TRADE_LOGIC_CHANGE_LETTER

IMPL_ON_FUNC( DBE_GET_ITEM_FROM_LETTER_REQ )
{
	//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
	KDBE_GET_ITEM_FROM_LETTER_ACK kPacket;
	kPacket.m_iOK			  = NetError::ERR_ODBC_01;
	kPacket.m_iPostNo		  = kPacket_.m_iPostNo;
	kPacket.m_cLetterType	  = kPacket_.m_cLetterType;
	kPacket.m_mapInsertedItem = kPacket_.m_mapInsertedItem;
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_iED			  = kPacket_.m_iED;

	CTime tCurr = CTime::GetCurrentTime();
	CTime tWedding;
	std::wstring wstrWeddingDate;

	switch( kPacket_.m_cLetterType )
	{
	case KPostItemInfo::LT_POST_OFFICE:
		{
			// ���� ������� ÷��üũ�� �Ѵ�..
			DO_QUERY( L"exec dbo.gup_update_get_post_item", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iPostNo );
			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( kPacket.m_iOK );
				m_kODBC.EndFetch();
			}
		}
		break;
	case KPostItemInfo::LT_WEDDING_INVITATION:
	case KPostItemInfo::LT_WEDDING_RESERVE:
		{
			bool bExistWeddingUID = true;
			DO_QUERY( L"exec dbo.P_GPost_DEL_Wedding", L"%d, %d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iPostNo % bExistWeddingUID );
			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( kPacket.m_iOK
					     >> kPacket.m_iWeddingUID
						 >> wstrWeddingDate );
				m_kODBC.EndFetch();
			}

			if( kPacket.m_iOK != NetError::NET_OK )
			{
				kPacket.m_iWeddingUID = 0;
				START_LOG( cerr, L"���� ���� �����ε�...��ȥ����UID �� ���� ���ߴ�" )
					<< BUILD_LOG( kPacket_.m_iPostNo )
					<< BUILD_LOG( kPacket.m_iOK )
					<< END_LOG;
			}
			else
			{
				KncUtil::ConvertStringToCTime( wstrWeddingDate, tWedding );
			}
		}
		break;
	case KPostItemInfo::LT_WEDDING_REWARD:
		{
			bool bExistWeddingUID = false;
			DO_QUERY( L"exec dbo.P_GPost_DEL_Wedding", L"%d, %d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iPostNo % bExistWeddingUID );
			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( kPacket.m_iOK
					>> kPacket.m_iWeddingUID
					>> wstrWeddingDate );
				m_kODBC.EndFetch();
			}

			// �����Ƿ� ������ �ʱ�ȭ
			kPacket.m_iWeddingUID = 0;

			if( kPacket.m_iOK != NetError::NET_OK )
			{
				kPacket.m_iWeddingUID = 0;
				START_LOG( cerr, L"���� ���� �����ε�..." )
					<< BUILD_LOG( kPacket_.m_iPostNo )
					<< BUILD_LOG( kPacket.m_iOK )
					<< END_LOG;
			}
		}
		break;
	default:
		{
			// �ý��� ������� �����..
			DO_QUERY( L"exec dbo.P_GPost_DEL_System", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iPostNo );
			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( kPacket.m_iOK );
				m_kODBC.EndFetch();
			}
		}
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"���� ÷�ι� �޾ư��� DB������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iPostNo )
			<< BUILD_LOG( kPacket.m_iOK )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_POST_LETTER_06;
		goto end_proc;
	}

	if( kPacket_.m_cLetterType == KPostItemInfo::LT_WEDDING_INVITATION || kPacket_.m_cLetterType == KPostItemInfo::LT_WEDDING_RESERVE )
	{
		std::vector< KItemInfo >::iterator vit = kPacket_.m_vecItemInfo.begin();
		if( vit != kPacket_.m_vecItemInfo.end() )
		{
			CTimeSpan tSpan(tWedding.GetTime() - tCurr.GetTime());
			vit->m_sPeriod	=(short)(tSpan.GetTotalHours() / 24 ) + 1;
		}
	}
	
	if( Query_InsertItemList( SEnum::GIR_FROM_LETTER, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
	{
		START_LOG( cerr, L"������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iPostNo )
			<< END_LOG;
	}
	
	if( kPacket_.m_cLetterType == KPostItemInfo::LT_WEDDING_INVITATION || kPacket_.m_cLetterType == KPostItemInfo::LT_WEDDING_RESERVE )
	{
		std::map< UidType, KItemInfo >::iterator mit = kPacket.m_mapItemInfo.begin();
		if( mit != kPacket.m_mapItemInfo.end() )
		{
			kPacket.m_iWeddingItemUID = mit->first;
			int iOK = NetError::ERR_ODBC_01;
			DO_QUERY( L"exec dbo.P_GCouple_WeddingInvitation_Wedding_INT", L"%d, %d", % mit->first % kPacket.m_iWeddingUID );
			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( iOK );
				m_kODBC.EndFetch();
			}

			if( iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"ûø�� ������ ��� ����!" )
					<< BUILD_LOG( mit->first )
					<< BUILD_LOG( kPacket.m_iWeddingUID )
					<< END_LOG;

				kPacket.m_iWeddingItemUID = 0;
			}
		}	
	}

#else //  SERV_RELATIONSHIP_SYSTEM
	KDBE_GET_ITEM_FROM_LETTER_ACK kPacket;
	kPacket.m_iOK			  = NetError::ERR_ODBC_01;
	kPacket.m_iPostNo		  = kPacket_.m_iPostNo;
	kPacket.m_bSystemLetter	  = kPacket_.m_bSystemLetter;
	kPacket.m_mapInsertedItem = kPacket_.m_mapInsertedItem;
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_iED			  = kPacket_.m_iED;

	if( kPacket_.m_bSystemLetter )
	{
		// �ý��� ������� �����..
		DO_QUERY( L"exec dbo.P_GPost_DEL_System", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iPostNo );
	}
	else
	{
		// ���� ������� ÷��üũ�� �Ѵ�..
		DO_QUERY( L"exec dbo.gup_update_get_post_item", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iPostNo );
	}

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"���� ÷�ι� �޾ư��� DB������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iPostNo )
			<< BUILD_LOG( kPacket.m_iOK )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_POST_LETTER_06;
		goto end_proc;
	}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
	if( Query_InsertItemList( SEnum::GIR_FROM_LETTER, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#else
	if( Query_InsertItemList( kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
		//}}
	{
		START_LOG( cerr, L"������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iPostNo )
			<< END_LOG;
	}
#endif SERV_RELATIONSHIP_SYSTEM
	//}
	
end_proc:
	SendToUser( LAST_SENDER_UID, DBE_GET_ITEM_FROM_LETTER_ACK, kPacket );
}

#endif SERV_TRADE_LOGIC_CHANGE_LETTER
//}}
IMPL_ON_FUNC( DBE_DELETE_LETTER_NOT )
{
	int iOK = NetError::ERR_ODBC_01;

	std::vector< UidType >::const_iterator vit;
	for( vit = kPacket_.m_vecPostNo.begin(); vit != kPacket_.m_vecPostNo.end(); ++vit )
	{
		DO_QUERY( L"exec dbo.gup_delete_post_item_by_PostNo", L"%d, %d", % kPacket_.m_iUnitUID % *vit );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

end_proc:
		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"���� ���� DB������Ʈ ����!" )
				<< BUILD_LOG( *vit )
				<< BUILD_LOG( iOK )
				<< END_LOG;
		}
	}
}
//}}

IMPL_ON_FUNC( DBE_DEL_TUTORIAL_REQ )
{
	KDBE_DEL_TUTORIAL_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iTeacherUID = kPacket_.m_iTeacherUID;
	kPacket.m_iStudentUID = kPacket_.m_iStudentUID;
	kPacket.m_cReason = kPacket_.m_cReason;

	DO_QUERY( L"exec dbo.gup_delete_Tutor", L"%d, %d",
				% kPacket_.m_iTeacherUID
				% kPacket_.m_iStudentUID
				);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );

		m_kODBC.EndFetch();

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"��������Ʈ ������ ����.!" )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( kPacket_.m_iTeacherUID )
				<< BUILD_LOG( kPacket_.m_iStudentUID )
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_TUTORIAL_00;
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_DEL_TUTORIAL_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_INSERT_TUTORIAL_REQ )
{
	KDBE_INSERT_TUTORIAL_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iTeacherUID = kPacket_.m_iTeacherUID;
	kPacket.m_kStudentUnitInfo = kPacket_.m_kStudentUnitInfo;

	DO_QUERY( L"exec dbo.gup_insert_tutor", L"%d, %d",
		% kPacket_.m_iTeacherUID
		% kPacket_.m_kStudentUnitInfo.m_iUnitUID
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );

		m_kODBC.EndFetch();

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			switch( kPacket.m_iOK )
			{
			case -2:
				kPacket.m_iOK = NetError::ERR_TUTORIAL_04;
				break;
			case -3:
				kPacket.m_iOK = NetError::ERR_TUTORIAL_05;
				break;
			}

			START_LOG( cerr, L"�������� �α� DBó�� ����.!" )
				<< BUILD_LOG( kPacket.m_iTeacherUID )
				<< BUILD_LOG( kPacket.m_kStudentUnitInfo.m_iUnitUID )
				<< BUILD_LOG( NetError::GetErrStr( kPacket.m_iOK ) )
				<< END_LOG;
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_INSERT_TUTORIAL_ACK, kPacket );
}

#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013 // ���볯¥: 2013-06-27

IMPL_ON_FUNC( DBE_RESET_SKILL_REQ )
{
	KDBE_RESET_SKILL_ACK kPacket;	
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iDelSkillID				= kPacket_.m_iDelSkillID;
	kPacket.m_iDelSkillLevel			= kPacket_.m_iDelSkillLevel;
	kPacket.m_iCSPoint					= kPacket_.m_iCSPoint;
	kPacket.m_iBeforSPoint				= kPacket_.m_iBeforSPoint;
	kPacket.m_iBeforCSPoint				= kPacket_.m_iBeforCSPoint;
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	if( !kPacket.m_kItemQuantityUpdate.m_mapQuantityChange.empty() || !kPacket.m_kItemQuantityUpdate.m_vecDeleted.empty() )
	{
		START_LOG( cerr, L"��ų �ʱ�ȭ ������ DB���� �������� Ȥ�� ���� ����." )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;
		kPacket.m_iOK = NetError::ERR_RESET_SKILL_02;
		goto end_proc;
	}

	// ��ų �ʱ�ȭ
	DO_QUERY( L"exec dbo.gup_update_skill_info_new", L"%d, %d, %d, %d",
		% kPacket_.m_iUnitUID 
		% kPacket_.m_iDelSkillID 
		% kPacket_.m_iDelSkillLevel
		% 0
		);


	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );

		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"��ų �ʱ�ȭ DB���� ���� ��ȯ." )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iDelSkillID )
			<< BUILD_LOG( kPacket.m_iOK )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_RESET_SKILL_03;
		goto end_proc;
	}
	
	// ĳ�� ��ų ����Ʈ ���� ����
	if( kPacket_.m_iCSPoint >= 0 )
	{
		DO_QUERY( L"exec dbo.gup_update_cash_skill_point_info", L"%d, %d",
			% kPacket_.m_iUnitUID
			% kPacket_.m_iCSPoint
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}	

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"failed to update cash skill point when reset skill!" )
				<< BUILD_LOG( LAST_SENDER_UID )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( kPacket_.m_iCSPoint )
				<< BUILD_LOG( kPacket_.m_iDelSkillID )
				<< BUILD_LOG( kPacket_.m_iDelSkillLevel )
				<< BUILD_LOG( kPacket.m_iOK )
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_SKILL_24;

			goto end_proc;	
		}
	}

end_proc:
	LOG_SUCCESS( kPacket.m_iOK == NetError::NET_OK )
		<< BUILD_LOG( LAST_SENDER_UID )
		<< BUILD_LOG( kPacket_.m_iUnitUID )
		<< BUILD_LOG( kPacket_.m_iDelSkillID )
		<< BUILD_LOG( kPacket_.m_iDelSkillLevel )
		<< BUILD_LOG( kPacket_.m_iCSPoint )
		<< BUILD_LOG( kPacket.m_iOK );

	SendToUser( LAST_SENDER_UID, DBE_RESET_SKILL_ACK, kPacket );
}

#else	// SERV_UPGRADE_SKILL_SYSTEM_2013
/*
IMPL_ON_FUNC( DBE_RESET_SKILL_REQ )
{
	KDBE_RESET_SKILL_ACK kPacket;	
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iSkillID					= kPacket_.m_iSkillID;
	kPacket.m_iSkillLevel				= kPacket_.m_iSkillLevel;
	kPacket.m_iSkillCSPoint				= kPacket_.m_iSkillCSPoint;
	kPacket.m_iCSPoint					= kPacket_.m_iCSPoint;
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	if( !kPacket.m_kItemQuantityUpdate.m_mapQuantityChange.empty() || !kPacket.m_kItemQuantityUpdate.m_vecDeleted.empty() )
	{
		START_LOG( cerr, L"��ų �ʱ�ȭ ������ DB���� �������� Ȥ�� ���� ����." )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;
		kPacket.m_iOK = NetError::ERR_RESET_SKILL_02;
		goto end_proc;
	}

	// ��ų �ʱ�ȭ
	DO_QUERY( L"exec dbo.gup_update_skill_info_new", L"%d, %d, %d, %d",
		% kPacket_.m_iUnitUID 
		% kPacket_.m_iSkillID 
		% kPacket_.m_iSkillLevel
		% kPacket_.m_iSkillCSPoint
		);


	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );

		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"��ų �ʱ�ȭ DB���� ���� ��ȯ." )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iSkillID )
			<< BUILD_LOG( kPacket.m_iOK )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_RESET_SKILL_03;
		goto end_proc;
	}

	// ĳ�� ��ų ����Ʈ ���� ����
	if( kPacket_.m_iCSPoint >= 0 )
	{
		DO_QUERY( L"exec dbo.gup_update_cash_skill_point_info", L"%d, %d",
			% kPacket_.m_iUnitUID
			% kPacket_.m_iCSPoint
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}	

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"failed to update cash skill point when reset skill!" )
				<< BUILD_LOG( LAST_SENDER_UID )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( kPacket_.m_iCSPoint )
				<< BUILD_LOG( kPacket_.m_iSkillID )
				<< BUILD_LOG( kPacket_.m_iSkillLevel )
				<< BUILD_LOG( kPacket_.m_iSkillCSPoint )
				<< BUILD_LOG( kPacket.m_iOK )
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_SKILL_24;

			goto end_proc;	
		}
	}



end_proc:
	LOG_SUCCESS( kPacket.m_iOK == NetError::NET_OK )
		<< BUILD_LOG( LAST_SENDER_UID )
		<< BUILD_LOG( kPacket_.m_iUnitUID )
		<< BUILD_LOG( kPacket_.m_iSkillID )
		<< BUILD_LOG( kPacket_.m_iSkillLevel )
		<< BUILD_LOG( kPacket_.m_iSkillCSPoint )
		<< BUILD_LOG( kPacket_.m_iCSPoint )
		//<< BUILD_LOG( kPacket_.m_iRetrievedSPoint )
		//<< BUILD_LOG( kPacket_.m_iRetrievedCSPoint )
		<< BUILD_LOG( kPacket.m_iOK );

	SendToUser( LAST_SENDER_UID, DBE_RESET_SKILL_ACK, kPacket );
}
*/
#endif	// SERV_UPGRADE_SKILL_SYSTEM_2013

IMPL_ON_FUNC( DBE_EXPAND_INVENTORY_SLOT_REQ )
{
    KDBE_EXPAND_INVENTORY_SLOT_ACK kPacket;
	kPacket.m_usEventID = kPacket_.m_usEventID;

    std::map< int, int >::const_iterator mit;
    for( mit = kPacket_.m_mapExpandedSlot.begin(); mit != kPacket_.m_mapExpandedSlot.end(); ++mit )
    {
//#ifdef SERV_REFORM_INVENTORY_INT	// �ؿ��� �ּ� ó��
		//{{ 2012. 12. 26	�ڼ���	�κ��丮 ���� �׽�Ʈ	- ����� ( Merged by �ڼ��� )
//#ifdef SERV_REFORM_INVENTORY_TEST
//		if( mit->first == CXSLInventory::ST_BANK )
//		{
//			DO_QUERY( L"exec dbo.gup_insert_inventory_size", L"%d, %d, %d", % kPacket_.m_iUnitUID % mit->first % mit->second );
//		}
//		else
//		{
//			DO_QUERY( L"exec dbo.P_GItemInventorySize_INS", L"%d, %d, %d", % kPacket_.m_iUnitUID % mit->first % mit->second );
//		}
//#else
        DO_QUERY( L"exec dbo.gup_insert_inventory_size", L"%d, %d, %d", % kPacket_.m_iUnitUID % mit->first % mit->second );
//#endif SERV_REFORM_INVENTORY_TEST
		//}}
//#endif SERV_REFORM_INVENTORY_INT

		int iOK = NetError::ERR_ODBC_01;

        if( m_kODBC.BeginFetch() )
        {
            FETCH_DATA( iOK );
            m_kODBC.EndFetch();
        }

        if( iOK == NetError::NET_OK )
        {
            kPacket.m_mapExpandedSlot.insert( std::make_pair( mit->first, mit->second ) );
        }
        else
        {
            START_LOG( cerr, L"�κ��丮 Ȯ�� ����." )
                << BUILD_LOG( iOK )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( mit->first )
				<< BUILD_LOG( mit->second )
                << END_LOG;
        }
    }

end_proc:
    SendToUser( LAST_SENDER_UID, DBE_EXPAND_INVENTORY_SLOT_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_EXPAND_SKILL_SLOT_REQ )
{
	KDBE_EXPAND_SKILL_SLOT_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iPeriodExpire = kPacket_.m_iPeriodExpire;
	//{{ 2011. 11. 30	������	��Ű�� ��ǰ �߰�
#ifdef SERV_ADD_PACKAGE_PRODUCT
	kPacket.m_usEventID = kPacket_.m_usEventID;
#endif SERV_ADD_PACKAGE_PRODUCT
	//}}

	DO_QUERY( L"exec dbo.gup_insert_skill_slotB_new", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iPeriodExpire );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK 
			>> kPacket.m_wstrSkillSlotBEndDate );

		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"��ų����B Ȯ�� ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iPeriodExpire )			
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_SKILL_14;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_EXPAND_SKILL_SLOT_ACK, kPacket );
}

//{{ 2008. 12. 14  ������	ĳ���� ���� Ȯ��
IMPL_ON_FUNC( DBE_EXPAND_CHAR_SLOT_REQ )
{
	KDBE_EXPAND_CHAR_SLOT_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec dbo.gup_update_ussize", L"%d, %d, %d", % LAST_SENDER_UID % kPacket_.m_iExpandSlotSize % kPacket_.m_iCharSlotMax );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK 
			>> kPacket.m_iCharSlotSize );

		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"ĳ���� ���� Ȯ�� ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( LAST_SENDER_UID )			
			<< BUILD_LOG( kPacket_.m_iExpandSlotSize )
			<< BUILD_LOG( kPacket_.m_iCharSlotMax )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_BUY_CASH_ITEM_31;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_EXPAND_CHAR_SLOT_ACK, kPacket );
}
//}}

//{{ 2008. 5. 5  ������  ��ٱ��� ���
IMPL_ON_FUNC( DBE_GET_WISH_LIST_REQ )
{
	KDBE_GET_WISH_LIST_ACK kPacket;
	int iSlotID = 0;
	int iItemID = 0;

	DO_QUERY( L"exec dbo.gup_get_WishList", L"%d", % kPacket_.m_iUserUID );
	while( m_kODBC.Fetch() )
	{
		FETCH_DATA( iSlotID
				 >> iItemID
				 );
		
		kPacket.m_mapWishList.insert( std::make_pair( iSlotID, iItemID ) );
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_GET_WISH_LIST_ACK, kPacket );
}
//}}

//{{ 2008. 5. 19  ������  �̺�Ʈ Ÿ�� ������Ʈ
IMPL_ON_FUNC( DBE_UPDATE_EVENT_TIME_REQ )
{
	KDBE_UPDATE_EVENT_TIME_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	{
		//////////////////////////////////////////////////////////////////////////
		// ���� �ð� �̺�Ʈ

		std::vector< KConnectTimeEventInfo >::const_iterator vit;
		for( vit = kPacket_.m_vecConnectTimeEvent.begin(); vit != kPacket_.m_vecConnectTimeEvent.end(); ++vit )
		{
			//{{ 2010. 06. 11  ������	�������� ���ӽð� �̺�Ʈ
#ifdef SERV_ACC_TIME_EVENT
			if( vit->m_bAccountEvent )
			{
				DO_QUERY( L"exec dbo.gup_update_event_account_nor", L"%d, %d, N\'%s\'", % LAST_SENDER_UID % vit->m_iEventUID % vit->m_wstrEventTime );

				if( m_kODBC.BeginFetch() )
				{
					FETCH_DATA( kPacket.m_iOK );
					m_kODBC.EndFetch();
				}

				if( kPacket.m_iOK == NetError::NET_OK )
				{
					kPacket.m_setConnectTimeEvent.insert( vit->m_iEventUID );
					
					//{{ 2012. 08. 14	�ڼ���	��õ���� ��ȭ �̺�Ʈ ���̵� ���� ���
#ifdef SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE
					if( vit->m_iScriptID == 573 )
					{
						CTime tCurrentTime = CTime::GetCurrentTime();
						std::wstring wstrCurrentTime = tCurrentTime.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
						DO_QUERY( L"exec dbo.P_GEvent_AngelCoin_INT", L"%d, %d, N\'%s\'", % LAST_SENDER_UID % kPacket_.m_iUnitUID % wstrCurrentTime );

						int iResult = -1;
						if( m_kODBC.BeginFetch() )
						{
							FETCH_DATA( iResult );
							m_kODBC.EndFetch();
						}
						
						if( iResult != 0 )
						{
							START_LOG( cerr, L"��õ���� ��ȭ �̺�Ʈ Insert BeginFetch() ����")
								<< BUILD_LOG( LAST_SENDER_UID )
								<< BUILD_LOG( kPacket_.m_iUnitUID )
								<< BUILD_LOG( wstrCurrentTime )
								<< END_LOG;
						}
					}
#endif SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE
					//}}
				}
				else
				{
					START_LOG( cerr, L"�̺�Ʈ Ÿ�� DB������Ʈ ����?" )
						<< BUILD_LOG( kPacket.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( vit->m_iEventUID )
						<< BUILD_LOG( vit->m_wstrEventTime )
						<< END_LOG;
				}
			}
			else
#endif SERV_ACC_TIME_EVENT
			//}}
			{
				DO_QUERY( L"exec dbo.gup_update_30min", L"%d, %d, N\'%s\'", % kPacket_.m_iUnitUID % vit->m_iEventUID % vit->m_wstrEventTime );

				if( m_kODBC.BeginFetch() )
				{
					FETCH_DATA( kPacket.m_iOK );
					m_kODBC.EndFetch();
				}

				if( kPacket.m_iOK == NetError::NET_OK )
				{
					kPacket.m_setConnectTimeEvent.insert( vit->m_iEventUID );
				}
				else
				{
					START_LOG( cerr, L"�̺�Ʈ Ÿ�� DB������Ʈ ����?" )
						<< BUILD_LOG( kPacket.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( vit->m_iEventUID )
						<< BUILD_LOG( vit->m_wstrEventTime )
						<< END_LOG;
				}
			}			
		}

		//////////////////////////////////////////////////////////////////////////		
	}
	

	//{{ 2009. 12. 7  ������	�����ð��̺�Ʈ
#ifdef CUMULATIVE_TIME_EVENT

	{
		//////////////////////////////////////////////////////////////////////////
		// ���� �ð� �̺�Ʈ

		//{{ 2013. 1. 8	�ڼ���	���� �̺�Ʈ�� �ݺ� ��� �߰�
#ifdef SERV_REPEAT_CUMULATIVE_REWARD_ITEM_EVENT
#else
		CTime tUpdateTime = CTime::GetCurrentTime();
		tUpdateTime += CTimeSpan( 18250, 0, 0, 0 ); // �ѹ��� ��� �ϹǷ� 50���� ���Ѵ�.
		std::wstring wstrUpdateDate = ( CStringW )( tUpdateTime.Format( _T( "%Y-%m-%d %H:%M:%S" ) ) );
#endif SERV_REPEAT_CUMULATIVE_REWARD_ITEM_EVENT
		//}}

		std::vector< KCumulativeTimeEventInfo >::const_iterator vitCT;
		for( vitCT = kPacket_.m_vecCumulativeTimeEvent.begin(); vitCT != kPacket_.m_vecCumulativeTimeEvent.end(); ++vitCT )
		{
			//{{ 2010. 06. 11  ������	�������� ���ӽð� �̺�Ʈ
#ifdef SERV_ACC_TIME_EVENT
			if( vitCT->m_bAccountEvent )
			{
				// �Ϸ� ������ ������Ʈ ����!
				//{{ 2013. 1. 8	�ڼ���	���� �̺�Ʈ�� �ݺ� ��� �߰�
#ifdef SERV_REPEAT_CUMULATIVE_REWARD_ITEM_EVENT
				DO_QUERY( L"exec dbo.gup_update_event_account_nor", L"%d, %d, N\'%s\'", % LAST_SENDER_UID % vitCT->m_iEventUID % vitCT->m_wstrEventTime );
#else
				DO_QUERY( L"exec dbo.gup_update_event_account_nor", L"%d, %d, N\'%s\'", % LAST_SENDER_UID % vitCT->m_iEventUID % wstrUpdateDate );
#endif SERV_REPEAT_CUMULATIVE_REWARD_ITEM_EVENT
				//}}

				if( m_kODBC.BeginFetch() )
				{
					FETCH_DATA( kPacket.m_iOK );
					m_kODBC.EndFetch();
				}

				if( kPacket.m_iOK == NetError::NET_OK )
				{
					kPacket.m_setCumulativeTimeEvent.insert( vitCT->m_iEventUID );
				}
				else
				{
					START_LOG( cerr, L"�̺�Ʈ Ÿ�� DB������Ʈ ����?" )
						<< BUILD_LOG( kPacket.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( vitCT->m_iEventUID )
						//{{ 2013. 1. 8	�ڼ���	���� �̺�Ʈ�� �ݺ� ��� �߰�
#ifdef SERV_REPEAT_CUMULATIVE_REWARD_ITEM_EVENT
						<< BUILD_LOG( vitCT->m_wstrEventTime )
#else
						<< BUILD_LOG( wstrUpdateDate )
#endif SERV_REPEAT_CUMULATIVE_REWARD_ITEM_EVENT
						//}}
						<< END_LOG;
				}
			}
			else
#endif SERV_ACC_TIME_EVENT
			//}}
			{
				// �Ϸ� ������ ������Ʈ ����!
				//{{ 2013. 1. 8	�ڼ���	���� �̺�Ʈ�� �ݺ� ��� �߰�
#ifdef SERV_REPEAT_CUMULATIVE_REWARD_ITEM_EVENT
				DO_QUERY( L"exec dbo.gup_update_30min", L"%d, %d, N\'%s\'", % kPacket_.m_iUnitUID % vitCT->m_iEventUID % vitCT->m_wstrEventTime );
#else
				DO_QUERY( L"exec dbo.gup_update_30min", L"%d, %d, N\'%s\'", % kPacket_.m_iUnitUID % vitCT->m_iEventUID % wstrUpdateDate );
#endif SERV_REPEAT_CUMULATIVE_REWARD_ITEM_EVENT
				//}}

				if( m_kODBC.BeginFetch() )
				{
					FETCH_DATA( kPacket.m_iOK );
					m_kODBC.EndFetch();
				}

				if( kPacket.m_iOK == NetError::NET_OK )
				{
					kPacket.m_setCumulativeTimeEvent.insert( vitCT->m_iEventUID );
				}
				else
				{
					START_LOG( cerr, L"�̺�Ʈ Ÿ�� DB������Ʈ ����?" )
						<< BUILD_LOG( kPacket.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( vitCT->m_iEventUID )
						//{{ 2013. 1. 8	�ڼ���	���� �̺�Ʈ�� �ݺ� ��� �߰�
#ifdef SERV_REPEAT_CUMULATIVE_REWARD_ITEM_EVENT
						<< BUILD_LOG( vitCT->m_wstrEventTime )
#else
						<< BUILD_LOG( wstrUpdateDate )
#endif SERV_REPEAT_CUMULATIVE_REWARD_ITEM_EVENT
						//}}
						<< END_LOG;
				}
			}			
		}
	}	

#endif CUMULATIVE_TIME_EVENT
	//}}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_UPDATE_EVENT_TIME_ACK, kPacket );
}
//}}

//{{ 2008. 5. 28  ������  �߸��� �κ� ī�װ���
IMPL_ON_FUNC( DBE_UPDATE_INVENTORY_ITEM_POS_NOT )
{
	//{{ 2010. 8. 3	������	�� �ý���
#ifdef SERV_PET_SYSTEM
	std::map< UidType, KItemPosition > mapFailedPos;
#else
	std::map< UidType, std::pair< int, int > > mapFailedPos;
#endif SERV_PET_SYSTEM
	//}}
	
	Query_UpdateItemPosition( kPacket_.m_iUnitUID, kPacket_.m_kItemPositionUpdate, mapFailedPos );
}
//}}

//{{ 2008. 6. 20  ������  �����ϱ�
IMPL_ON_FUNC( DBE_PRESENT_CASH_ITEM_CHECK_NICKNAME_REQ )
{
	KDBE_PRESENT_CASH_ITEM_CHECK_NICKNAME_ACK kPacket;
	kPacket.m_iOK				 = NetError::ERR_ODBC_01;
	kPacket.m_wstrMessage		 = kPacket_.m_wstrMessage;
	kPacket.m_vecPurchaseReqInfo = kPacket_.m_vecPurchaseReqInfo;

#ifdef SERV_GLOBAL_BILLING
	kPacket.m_wstrReceiverNickName = kPacket_.m_wstrReceiverNickName;
#endif // SERV_GLOBAL_BILLING

#ifdef SERV_SUPPORT_SEVERAL_CASH_TYPES
	kPacket.m_iUseCashType		 = kPacket_.m_iUseCashType;
#endif //SERV_SUPPORT_SEVERAL_CASH_TYPES

#ifdef SERV_NEXON_COUPON_SYSTEM// �۾���¥: 2013-07-29	// �ڼ���
	kPacket.m_bUseCoupon		= kPacket_.m_bUseCoupon;
#endif // SERV_NEXON_COUPON_SYSTEM	

	// SERV_GLOBAL_BILLING - SP ���� �ؾ� ��
	DO_QUERY( L"exec dbo.gup_get_userid_by_nickname", L"N\'%s\'", % kPacket_.m_wstrReceiverNickName );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK
			>> kPacket.m_wstrUserID
			>> kPacket.m_iReceiverUserUID	// SERV_GLOBAL_BILLING
			>> kPacket.m_iReceiverUnitUID	// SERV_GLOBAL_BILLING
			);

		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		switch( kPacket.m_iOK )
		{
		case -1:    kPacket.m_iOK = NetError::ERR_BUY_CASH_ITEM_22;   break;
		}
	}
	else
	{
		DO_QUERY( L"exec dbo.gup_get_unitclass_level", L"N\'%s\'", % kPacket_.m_wstrReceiverNickName );
		if( m_kODBC.BeginFetch() )
		{
			int iOK = -1;
			FETCH_DATA( iOK
				>> kPacket.m_iReceiverUnitClass
				>> kPacket.m_cReceiverLevel
				);
			m_kODBC.EndFetch();
		}
		else
		{
			kPacket.m_cReceiverLevel = -1;
			kPacket.m_iReceiverUnitClass = -1;
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_PRESENT_CASH_ITEM_CHECK_NICKNAME_ACK, kPacket );
}
//}}

//{{ 2008. 9. 3  ������		�Ӽ���ȭ
IMPL_ON_FUNC( DBE_ATTRIB_ENCHANT_ITEM_REQ )
{
	KDBE_ATTRIB_ENCHANT_ITEM_ACK kPacket;
	kPacket.m_iItemUID = kPacket_.m_iItemUID;
	kPacket.m_iItemID = kPacket_.m_iItemID;
	kPacket.m_cAttribEnchantSlotNo = kPacket_.m_cAttribEnchantSlotNo;
	kPacket.m_cAttribEnchantID = kPacket_.m_cAttribEnchantID;
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec dbo.gup_update_Attribute", L"%d, %d, %d, %d",
		% kPacket_.m_iUnitUID
		% kPacket_.m_iItemUID
		% static_cast<int>(kPacket_.m_cAttribEnchantSlotNo) 
		% static_cast<int>(kPacket_.m_cAttribEnchantID)
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"�Ӽ� ��ȭ ���� ������Ʈ ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iItemUID )
			<< BUILD_LOGc( kPacket_.m_cAttribEnchantSlotNo )
			<< BUILD_LOGc( kPacket_.m_cAttribEnchantID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_ATTRIB_ENCHANT_01;
	}

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_ATTRIB_ENCHANT_ITEM_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_IDENTIFY_ITEM_REQ )
{
	KDBE_IDENTIFY_ITEM_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_mapInsertedItem			= kPacket_.m_mapInsertedItem;
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
	if( Query_InsertItemList( SEnum::GIR_IDENTIFY_ITEM, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#else
	if( Query_InsertItemList( kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
	//}}	
	{
		START_LOG( cerr, L"������ ������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_IDENTIFY_04;
	}
	else
	{
		kPacket.m_iOK = NetError::NET_OK;
	}

	SendToUser( LAST_SENDER_UID, DBE_IDENTIFY_ITEM_ACK, kPacket );
}
//}}

//{{ 2008. 9. 26  ������	��ü�� ��������Ʈ
IMPL_ON_FUNC( DBE_NEW_POST_BLACK_LIST_REQ )
{
	KDBE_NEW_POST_BLACK_LIST_ACK kPacket;
	kPacket.m_wstrNickName = kPacket_.m_wstrNickName;
	
	DO_QUERY( L"exec dbo.gup_insert_post_blacklist", L"%d, N\'%s\'",
		% kPacket_.m_iUnitUID
		% kPacket_.m_wstrNickName
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		switch( kPacket.m_iOK )
		{
		case -1: kPacket.m_iOK = NetError::ERR_POST_BLACK_LIST_00; break;
		case -2: kPacket.m_iOK = NetError::ERR_POST_BLACK_LIST_01; break;
		
		default: kPacket.m_iOK = NetError::ERR_POST_BLACK_LIST_02; break;
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_NEW_POST_BLACK_LIST_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_DEL_POST_BLACK_LIST_REQ )
{
	KDBE_DEL_POST_BLACK_LIST_ACK kPacket;
	kPacket.m_wstrNickName = kPacket_.m_wstrNickName;	
	
	DO_QUERY( L"exec dbo.gup_delete_post_blacklist", L"%d, N\'%s\'",
		% kPacket_.m_iUnitUID
		% kPacket_.m_wstrNickName
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		switch( kPacket.m_iOK )
		{
		case -1: kPacket.m_iOK = NetError::ERR_POST_BLACK_LIST_00; break;
		case -2: kPacket.m_iOK = NetError::ERR_POST_BLACK_LIST_03; break;

		default: kPacket.m_iOK = NetError::ERR_POST_BLACK_LIST_04; break;
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_DEL_POST_BLACK_LIST_ACK, kPacket );
}
//}}

//{{ 2008. 10. 7  ������	Ÿ��Ʋ
IMPL_ON_FUNC( DBE_INSERT_TITLE_REQ )
{
	KDBE_INSERT_TITLE_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iUnitUID = kPacket_.m_iUnitUID;
	kPacket.m_iTitleID = kPacket_.m_iTitleID;

	//{{ 2011. 04. 27	������	Īȣ ȹ�� ������ ����
#ifdef SERV_TITLE_ITEM_NEW
	if( kPacket_.m_bExpandPeriod )
	{
		DO_QUERY( L"exec dbo.gup_update_title", L"%d, %d, %d",
			% kPacket_.m_iUnitUID
			% kPacket_.m_iTitleID
			% kPacket_.m_sPeriod
			);
	}
	else
#endif SERV_TITLE_ITEM_NEW
	//}}
	{
		DO_QUERY( L"exec dbo.gup_insert_Title", L"%d, %d, %d",
			% kPacket_.m_iUnitUID
			% kPacket_.m_iTitleID
			% kPacket_.m_sPeriod
			);
	}

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK
			>> kPacket.m_wstrEndDate );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
        START_LOG( cerr, L"Īȣ ������Ʈ�� �����ߴ�? �Ͼ�� ���� ����." )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iTitleID )
			<< BUILD_LOG( kPacket_.m_sPeriod )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_TITLE_04;
	}
	
end_proc:
	if( kPacket_.m_bGameServerEvent )
	{
		SendToServer( DBE_INSERT_TITLE_ACK, kPacket );
	}
	else
	{
		SendToUser( LAST_SENDER_UID, DBE_INSERT_TITLE_ACK, kPacket );
	}
}
//}}

//{{ 2008. 12. 25  ������	�ο�
IMPL_ON_FUNC( DBE_ENCHANT_ATTACH_ITEM_REQ )
{
	KDBE_ENCHANT_ATTACH_ITEM_ACK kPacket;
	kPacket.m_iItemUID				  = kPacket_.m_iItemUID;
	kPacket.m_iLevelAfterEnchant	  = kPacket_.m_iLevelAfterEnchant;
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec gup_update_Enchant", L"%d, %d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iItemUID % kPacket_.m_iLevelAfterEnchant );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"��ȭ ���� �ο� ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iItemUID )
			<< BUILD_LOG( kPacket_.m_iLevelAfterEnchant )	
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_ATTACH_ITEM_03;
		goto end_proc;
	}

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_ENCHANT_ATTACH_ITEM_ACK, kPacket );
}
//}}

//{{ 2008. 11. 18  ������	������ ��ȯ
IMPL_ON_FUNC( DBE_ITEM_EXCHANGE_REQ )
{
	KDBE_ITEM_EXCHANGE_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;	
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;	

	//{{ 2013. 02. 19   ��ȯ �α� �߰� - ��μ�
#ifdef SERV_EXCHANGE_LOG
	kPacket.m_iSourceItemID			= kPacket_.m_iSourceItemID;
	kPacket.m_iSourceItemQuantity	= kPacket_.m_iSourceItemQuantity;
	kPacket.m_cExchangeType			= kPacket_.m_cExchangeType;
#endif SERV_EXCHANGE_LOG
	//}

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
	if( Query_InsertItemList( SEnum::GIR_EXCHANGE_ITEM, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#else
	if( Query_InsertItemList( kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
	//}}
	{
		START_LOG( cerr, L"������ ��ȯ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_ITEM_EXCHANGE_04;
	}
	else
	{
		kPacket.m_iOK = NetError::NET_OK;
	}

	//{{ 2012. 03. 05	��μ�	������ȯ
#ifdef SERV_RANDOM_EXCHANGE_RESULT_VIEW
	kPacket.m_mapResultItem = kPacket_.m_mapResultItem;
#endif SERV_RANDOM_EXCHANGE_RESULT_VIEW
	//}}

#ifdef SERV_2013_JUNGCHU_TITLE
	kPacket.m_b12TimesRewarded = false;
#endif SERV_2013_JUNGCHU_TITLE

#if defined( SERV_GROW_UP_SOCKET ) || defined( SERV_2013_JUNGCHU_TITLE )
	if( ( kPacket.m_iOK == NetError::NET_OK ) && kPacket_.m_iSourceItemID == EXCHANGE_ITEM_ID_FOR_GROW_UP )
	{
		DO_QUERY( L"exec dbo.P_GEventTradeCount_UPD", L"%d", % kPacket_.m_iUnitUID );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iExchangeCount );
			m_kODBC.EndFetch();

#ifdef SERV_2013_JUNGCHU_TITLE
			if( kPacket.m_iExchangeCount == 12 )
				kPacket.m_b12TimesRewarded = true;
#endif SERV_2013_JUNGCHU_TITLE
		}
		else
		{
			START_LOG( clog, L"P_GEventTradeCount_UPD ȣ�� ����! ���� �ϸ� �ȵǴµ�??" )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< END_LOG;
		}
	}
end_proc:
#endif SERV_GROW_UP_SOCKET

	//{{ 2012. 08. 14	�ڼ���	��õ���� ��ȭ ��ȯ �α�
#ifdef SERV_ARCHUANGEL_S_COIN_EVENT_LOG
	if( ( kPacket.m_iOK == NetError::NET_OK ) && ( kPacket_.m_vecDestItem.empty() == false ) )
	{
		bool bChecker = false;
		if( 1 < kPacket_.m_vecDestItem.size() )
		{
			START_LOG( cerr, L"��õ���� ��ȭ ��ȯ ��� �������� ������ �������Դϴ�.")
				<< END_LOG;
			bChecker = true;
		}

		CTime tCurrentTime = CTime::GetCurrentTime();
		std::wstring wstrCurrentTime = tCurrentTime.Format( _T( "%Y-%m-%d %H:%M:%S" ) );

		BOOST_TEST_FOREACH( const int, iItemID, kPacket_.m_vecDestItem )
		{
			if( bChecker == true )
			{
				START_LOG( cerr, iItemID )
					<< END_LOG;
			}

			DO_QUERY( L"exec dbo.P_GEvent_AngelCoin_Log_INT", L"%d, %d, %d, N\'%s\'", % LAST_SENDER_UID % kPacket_.m_iUnitUID % iItemID % wstrCurrentTime );

			int iResult = -2;
			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( iResult );
				m_kODBC.EndFetch();
			}

			if( iResult != NetError::NET_OK )
			{
				START_LOG( cerr, L"��õ���� ��ȭ ��ȯ �α� ���� ����")
					<< BUILD_LOG( LAST_SENDER_UID )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( iItemID )
					<< BUILD_LOG( wstrCurrentTime )
					<< END_LOG;
			}
		}
	}

end_proc:
#endif SERV_ARCHUANGEL_S_COIN_EVENT_LOG
	//}}

	SendToUser( LAST_SENDER_UID, DBE_ITEM_EXCHANGE_ACK, kPacket );
}
//}}

//{{ 2009. 4. 8  ������		�г��� ����
IMPL_ON_FUNC( DBE_DELETE_NICK_NAME_REQ )
{
	KDBE_DELETE_NICK_NAME_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec dbo.gup_change_nickname", L"%d, N\'%s\'", % kPacket_.m_iUnitUID % kPacket_.m_wstrNickName );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}
	else
	{
		kPacket.m_iOK = NetError::ERR_CHANGE_NICK_NAME_00;
		goto end_proc;
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		switch( kPacket.m_iOK )
		{
		case -1:
		case -2:
			{
				START_LOG( cerr, L"�г��� ���� ����!" )
					<< BUILD_LOG( kPacket.m_iOK )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( kPacket_.m_wstrNickName )
					<< END_LOG;

				kPacket.m_iOK = NetError::ERR_CHANGE_NICK_NAME_00;
			}
			break;

		default:
			{
				START_LOG( cerr, L"���ǵ��� ���� ����! sp�� �ٲ���?" )
					<< BUILD_LOG( kPacket.m_iOK )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( kPacket_.m_wstrNickName )
					<< END_LOG;

				kPacket.m_iOK = NetError::ERR_CHANGE_NICK_NAME_00;
			}
		}
		goto end_proc;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_DELETE_NICK_NAME_ACK, kPacket );
}
//}}

IMPL_ON_FUNC( DBE_INSERT_CASH_SKILL_POINT_REQ )
{
	KDBE_INSERT_CASH_SKILL_POINT_ACK kPacket;
	kPacket.m_iOK					= NetError::ERR_ODBC_01;
	kPacket.m_iCSPoint				= kPacket_.m_iCSPoint;
	kPacket.m_iPeriod				= kPacket_.m_iPeriod;
	kPacket.m_bUpdateEndDateOnly	= kPacket_.m_bUpdateEndDateOnly;
	//{{ 2010. 12. 8	������	�̺�Ʈ�� �׳�ý� �ູ
#ifdef SERV_EVENT_CASH_SKILL_POINT_ITEM
	kPacket.m_iSkillPointItemID		= kPacket_.m_iSkillPointItemID;
#endif SERV_EVENT_CASH_SKILL_POINT_ITEM
	//}}


	if( true == kPacket_.m_bUpdateEndDateOnly )
	{
		DO_QUERY( L"exec dbo.gup_update_cash_skill_period", L"%d, %d",
			% kPacket_.m_iUnitUID
			% kPacket_.m_iPeriod
			);
	}
	else
	{
		DO_QUERY( L"exec dbo.gup_insert_cash_skill_point_info", L"%d, %d, %d",
			% kPacket_.m_iUnitUID
			% kPacket_.m_iCSPoint
			% kPacket_.m_iPeriod
			);
	}

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK
			>> kPacket.m_wstrEndDate );
		m_kODBC.EndFetch();
	}	

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"failed to insert cash skill point!" )
			<< BUILD_LOG( LAST_SENDER_UID )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iCSPoint )
			<< BUILD_LOG( kPacket_.m_iPeriod )
			<< BUILD_LOG( kPacket.m_iOK )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_SKILL_23;

		goto end_proc;	
	}


end_proc:
	LOG_SUCCESS( kPacket.m_iOK == NetError::NET_OK )
		<< BUILD_LOG( LAST_SENDER_UID )
		<< END_LOG;

	SendToUser( LAST_SENDER_UID, DBE_INSERT_CASH_SKILL_POINT_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_EXPIRE_CASH_SKILL_POINT_REQ )
{
	KDBE_EXPIRE_CASH_SKILL_POINT_ACK kPacket;
	kPacket.m_iOK					= NetError::ERR_ODBC_01;
	kPacket.m_iRetrievedSPoint		= kPacket_.m_iRetrievedSPoint;

	// expire ��Ű�� ���� ���� ��� ��ųƮ���� ���纻�� log�� �����, �׸��� MAX_CSP�� 0�� �ٲ۴�.
	DO_QUERY( L"exec dbo.gup_insert_BeforeSkillList", L"%d", % kPacket_.m_iUnitUID );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"failed to log current skill tree before reset cash skill point!!" )
			<< BUILD_LOG( LAST_SENDER_UID )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iRetrievedSPoint )
			<< BUILD_LOG( kPacket.m_iOK )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_SKILL_25;
		goto end_proc;	
	}

	// �α׸� ����ٸ� ������ cash skill point ������ �ʱ�ȭ �Ѵ�
	DO_QUERY( L"exec dbo.gup_update_cash_skill_point_info", L"%d, %d", % kPacket_.m_iUnitUID % 0 ); // Cash skill point

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"failed to reset cash skill point!" )
			<< BUILD_LOG( LAST_SENDER_UID )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iRetrievedSPoint )
			<< BUILD_LOG( kPacket.m_iOK )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_SKILL_25;
		goto end_proc;	
	}

	// note!! 300�� �̻� skill������ �ٲ�� ��찡 ������? �α׸� �����.
	// ������ ���� ȣ���� �����ؼ� ���� �α�?
	if( kPacket_.m_vecUserSkillData.size() > 300 )
	{
		START_LOG( cerr, L"too many skill update on cash skill point expiration!" )
			<< BUILD_LOG( (int) kPacket_.m_vecUserSkillData.size() )
			<< END_LOG;
	}

	for( UINT i = 0; i < kPacket_.m_vecUserSkillData.size(); ++i )
	{
		const KUserSkillData& userSkillData = kPacket_.m_vecUserSkillData[i];

		DO_QUERY( L"exec dbo.gup_update_skill_info_new", L"%d, %d, %d, %d",
			% kPacket_.m_iUnitUID 
			% userSkillData.m_iSkillID
			% userSkillData.m_cSkillLevel
			% userSkillData.m_cSkillCSPoint );

		int iResult = NetError::ERR_UNKNOWN;
		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iResult );
			m_kODBC.EndFetch();
		}
		
		if( iResult != NetError::NET_OK )
		{
			START_LOG( cerr, L"failed to update skill on cash skill point expire!" )
				<< BUILD_LOG( LAST_SENDER_UID )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( kPacket_.m_iRetrievedSPoint )
				<< BUILD_LOG( userSkillData.m_iSkillID )
				<< BUILD_LOG( userSkillData.m_cSkillLevel )
				<< BUILD_LOG( userSkillData.m_cSkillCSPoint )
				<< BUILD_LOG( kPacket.m_iOK )
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_SKILL_22;
		}
	}

end_proc:
	LOG_SUCCESS( kPacket.m_iOK == NetError::NET_OK )
		<< BUILD_LOG( LAST_SENDER_UID )
		<< END_LOG;

	SendToUser( LAST_SENDER_UID, DBE_EXPIRE_CASH_SKILL_POINT_ACK, kPacket );
}

//{{ 2009. 8. 4  ������		���� ��ų
IMPL_ON_FUNC( DBE_UNSEAL_SKILL_REQ )
{
	KDBE_UNSEAL_SKILL_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iSkillID = kPacket_.m_iSkillID;

	DO_QUERY( L"exec dbo.gup_insert_unsealed_skill", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iSkillID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"���ν�ų �������� ����!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iSkillID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_SKILL_27;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_UNSEAL_SKILL_ACK, kPacket );
}
//}}

//{{ 2009. 5. 11  ������	�ǽð� ������
IMPL_ON_FUNC( DBE_GET_ITEM_INSERT_TO_INVENTORY_REQ )
{
	KDBE_GET_ITEM_INSERT_TO_INVENTORY_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_kBroadInfo = kPacket_.m_kBroadInfo;
	kPacket.m_mapInsertItem = kPacket_.m_mapInsertItem;
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
	if( Query_InsertItemList( SEnum::GIR_DUNGEON_DROP, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#else
	if( Query_InsertItemList( kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
	//}}	
	{
		START_LOG( cerr, L"�ǽð� �����۾����� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_ITEM_10;
	}
	else
	{
		kPacket.m_iOK = NetError::NET_OK;

		//{{ 2009. 11. 18  ������	Ư���ð�����̺�Ʈ
		std::set< int > setSealItem;
		std::vector< KItemInfo >::const_iterator vit;
		for( vit = kPacket_.m_vecItemInfo.begin(); vit != kPacket_.m_vecItemInfo.end(); ++vit )
		{
			if( vit->IsSealedItem() )
			{
				setSealItem.insert( vit->m_iItemID );
			}
		}

		Query_UpdateSealItem( setSealItem, kPacket.m_mapItemInfo );
		//}}
	}

	SendToUser( LAST_SENDER_UID, DBE_GET_ITEM_INSERT_TO_INVENTORY_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_GET_TEMP_ITEM_REQ )
{
	KDBE_GET_TEMP_ITEM_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iTempItemUID = kPacket_.m_iTempItemUID;
	kPacket.m_mapInsertItem = kPacket_.m_mapInsertItem;
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
	if( Query_InsertItemList( SEnum::GIR_TEMP_INVENTORY, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#else
	if( Query_InsertItemList( kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
	//}}
	{
		START_LOG( cerr, L"�ǽð� �����۾����� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_ITEM_10;
	}
	else
	{
		kPacket.m_iOK = NetError::NET_OK;

		//{{ 2009. 11. 18  ������	Ư���ð�����̺�Ʈ
		std::set< int > setSealItem;
		std::vector< KItemInfo >::const_iterator vit;
		for( vit = kPacket_.m_vecItemInfo.begin(); vit != kPacket_.m_vecItemInfo.end(); ++vit )
		{
            if( vit->IsSealedItem() )
			{
				setSealItem.insert( vit->m_iItemID );
			}
		}

		Query_UpdateSealItem( setSealItem, kPacket.m_mapItemInfo );
		//}}
	}

	SendToUser( LAST_SENDER_UID, DBE_GET_TEMP_ITEM_ACK, kPacket );
}
//}}

IMPL_ON_FUNC( DBE_REQUEST_FRIEND_REQ )
{
    KDBE_REQUEST_FRIEND_ACK kPacket;
    kPacket.m_iOK = NetError::ERR_ODBC_01;
    kPacket.m_wstrNickName = kPacket_.m_wstrNickName;
    kPacket.m_wstrMessage = kPacket_.m_wstrMessage;

    DO_QUERY( L"exec dbo.gup_friend_request", L"%d, %d, N\'%s\', %d, %d",
		% LAST_SENDER_UID
		% kPacket_.m_iUnitUID
		% kPacket_.m_wstrNickName
		% KFriendInfo::FS_REQUESTED
		% KFriendInfo::FS_WAITING
		);

    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( kPacket.m_iOK
            >> kPacket.m_iUnitUID );

        m_kODBC.EndFetch();
    }

    switch( kPacket.m_iOK )
    {
    case NetError::NET_OK:
        break;
#ifdef SERV_REQUEST_FRIEND_NO_NICKNAME_ERROR_FIX
	case -1:	// �ӱԼ� �Ϻ� �߰� �г��� �������� ���� �� 
		kPacket.m_iOK = NetError::ERR_SEARCH_UNIT_04;
		break;
#endif SERV_REQUEST_FRIEND_NO_NICKNAME_ERROR_FIX
    case -5:
        kPacket.m_iOK = NetError::ERR_MESSENGER_13;
        break;
    case -6:
        kPacket.m_iOK = NetError::ERR_MESSENGER_12;
        break;
    case -7:
        kPacket.m_iOK = NetError::ERR_MESSENGER_14;
        break;
    case -8:
        kPacket.m_iOK = NetError::ERR_MESSENGER_15;
        break;
    default:
		{
			START_LOG( cerr, L"���� ���� ���� ���� ���ϰ�" )
				<< BUILD_LOG( LAST_SENDER_UID )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( kPacket.m_iUnitUID )
				<< END_LOG;
			kPacket.m_iOK = NetError::ERR_MESSENGER_10;
		}
        break;
    }

    if( kPacket.m_iOK != NetError::NET_OK )
    {
        START_LOG( cerr, L"ģ�� ��û ����" )
            << BUILD_LOG( LAST_SENDER_UID )
            << BUILD_LOG( kPacket.m_iOK )
            << BUILD_LOG( kPacket.m_iUnitUID )
            << END_LOG;
    }

end_proc:
    SendToUser( LAST_SENDER_UID, DBE_REQUEST_FRIEND_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_ACCEPT_FRIEND_REQ )
{
    KDBE_ACCEPT_FRIEND_ACK kPacket;
    kPacket.m_iOK = NetError::ERR_ODBC_01;
    kPacket.m_iUnitUID = kPacket_.m_iFriendUnitUID;

    DO_QUERY( L"exec dbo.gup_friend_accept", L"%d, %d, %d, %d, %d",
		% kPacket_.m_iUnitUID
		% kPacket_.m_iFriendUnitUID
		% KFriendInfo::FS_WAITING
		% KFriendInfo::FS_REQUESTED
		% KFriendInfo::FS_NORMAL
		);

    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( kPacket.m_iOK );
        m_kODBC.EndFetch();
    }

    if( kPacket.m_iOK != NetError::NET_OK )
    {
        START_LOG( cerr, L"ģ�� ���� ����." )
            << BUILD_LOG( kPacket.m_iOK )
            << BUILD_LOG( LAST_SENDER_UID )
            << BUILD_LOG( kPacket_.m_iUnitUID )
            << BUILD_LOG( kPacket_.m_iFriendUnitUID )
            << END_LOG;

        kPacket.m_iOK = NetError::ERR_MESSENGER_10;
    }

end_proc:
    SendToUser( LAST_SENDER_UID, DBE_ACCEPT_FRIEND_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_DENY_FRIEND_REQ )
{
    KDBE_DENY_FRIEND_ACK kPacket;
    kPacket.m_iOK = NetError::ERR_ODBC_01;
    kPacket.m_iUnitUID = kPacket_.m_iFriendUnitUID;

    DO_QUERY( L"exec dbo.gup_friend_delete", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iFriendUnitUID );
    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( kPacket.m_iOK );
        m_kODBC.EndFetch();
    }

    if( kPacket.m_iOK != NetError::NET_OK )
    {
        START_LOG( cerr, L"ģ�� ���� ����." )
            << BUILD_LOG( kPacket.m_iOK )
            << BUILD_LOG( LAST_SENDER_UID )
            << BUILD_LOG( kPacket_.m_iUnitUID )
            << BUILD_LOG( kPacket_.m_iFriendUnitUID )
            << END_LOG;

        kPacket.m_iOK = NetError::ERR_MESSENGER_10;
    }

end_proc:
    SendToUser( LAST_SENDER_UID, DBE_DENY_FRIEND_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_BLOCK_FRIEND_REQ )
{
    KDBE_BLOCK_FRIEND_ACK kPacket;
    kPacket.m_iOK = NetError::ERR_ODBC_01;
    kPacket.m_iUnitUID = kPacket_.m_iFriendUnitUID;

    DO_QUERY( L"exec dbo.gup_friend_block_unblock", L"%d, %d, %d, %d",
		% kPacket_.m_iUnitUID
		% kPacket_.m_iFriendUnitUID
		% KFriendInfo::FS_NORMAL
		% KFriendInfo::FS_BLOCKED
		);

    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( kPacket.m_iOK );
        m_kODBC.EndFetch();
    }

    if( kPacket.m_iOK != NetError::NET_OK )
    {
        START_LOG( cerr, L"ģ�� ���� ����." )
            << BUILD_LOG( kPacket.m_iOK )
            << BUILD_LOG( LAST_SENDER_UID )
            << BUILD_LOG( kPacket_.m_iUnitUID )
            << BUILD_LOG( kPacket_.m_iFriendUnitUID )
            << END_LOG;

        kPacket.m_iOK = NetError::ERR_MESSENGER_10;
    }

end_proc:
    SendToUser( LAST_SENDER_UID, DBE_BLOCK_FRIEND_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_UNBLOCK_FRIEND_REQ )
{
    KDBE_UNBLOCK_FRIEND_ACK kPacket;
    kPacket.m_iOK = NetError::ERR_ODBC_01;
    kPacket.m_iUnitUID = kPacket_.m_iFriendUnitUID;

    DO_QUERY( L"exec dbo.gup_friend_block_unblock", L"%d, %d, %d, %d",
		% kPacket_.m_iUnitUID
		% kPacket_.m_iFriendUnitUID
		% KFriendInfo::FS_BLOCKED
		% KFriendInfo::FS_NORMAL
		);

    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( kPacket.m_iOK );
        m_kODBC.EndFetch();
    }

    if( kPacket.m_iOK != NetError::NET_OK )
    {
        START_LOG( cerr, L"ģ�� ���� ���� ����." )
            << BUILD_LOG( kPacket.m_iOK )
            << BUILD_LOG( LAST_SENDER_UID )
            << BUILD_LOG( kPacket_.m_iUnitUID )
            << BUILD_LOG( kPacket_.m_iFriendUnitUID )
            << END_LOG;

        kPacket.m_iOK = NetError::ERR_MESSENGER_10;
    }

end_proc:
    SendToUser( LAST_SENDER_UID, DBE_UNBLOCK_FRIEND_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_DELETE_FRIEND_REQ )
{
    KDBE_DELETE_FRIEND_ACK kPacket;
    kPacket.m_iOK = NetError::ERR_ODBC_01;
    kPacket.m_iUnitUID = kPacket_.m_iFriendUnitUID;

    DO_QUERY( L"exec dbo.gup_friend_delete", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iFriendUnitUID );
    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( kPacket.m_iOK );
        m_kODBC.EndFetch();
    }

    if( kPacket.m_iOK != NetError::NET_OK )
    {
        START_LOG( cerr, L"ģ�� ���� ����." )
            << BUILD_LOG( kPacket.m_iOK )
            << BUILD_LOG( LAST_SENDER_UID )
            << BUILD_LOG( kPacket_.m_iUnitUID )
            << BUILD_LOG( kPacket_.m_iFriendUnitUID )
            << END_LOG;

        kPacket.m_iOK = NetError::ERR_MESSENGER_10;
    }

end_proc:
    SendToUser( LAST_SENDER_UID, DBE_DELETE_FRIEND_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_MOVE_FRIEND_REQ )
{
    KDBE_MOVE_FRIEND_ACK kPacket;
    kPacket.m_iOK = NetError::ERR_ODBC_01;
    kPacket.m_iUnitUID = kPacket_.m_iFriendUnitUID;
    kPacket.m_cTargetGroupID = kPacket_.m_cTargetGroupID;

    DO_QUERY( L"exec dbo.gup_friend_move", L"%d, %d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iFriendUnitUID % ( int )kPacket_.m_cTargetGroupID );
    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( kPacket.m_iOK );
        m_kODBC.EndFetch();
    }

    if( kPacket.m_iOK != NetError::NET_OK )
    {
        START_LOG( cerr, L"ģ�� �׷� �̵� ����." )
            << BUILD_LOG( kPacket.m_iOK )
            << BUILD_LOG( LAST_SENDER_UID )
            << BUILD_LOG( kPacket_.m_iUnitUID )
            << BUILD_LOG( kPacket_.m_iFriendUnitUID )
            << BUILD_LOGc( kPacket_.m_cTargetGroupID )
            << END_LOG;

        kPacket.m_iOK = NetError::ERR_MESSENGER_10;
    }

end_proc:
    SendToUser( LAST_SENDER_UID, DBE_MOVE_FRIEND_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_MAKE_FRIEND_GROUP_REQ )
{
    KDBE_MAKE_FRIEND_GROUP_ACK kPacket;
    kPacket.m_iOK = NetError::ERR_ODBC_01;
    kPacket.m_cGroupID = kPacket_.m_cGroupID;
    kPacket.m_wstrGroupName = kPacket_.m_wstrGroupName;

    DO_QUERY( L"exec dbo.gup_friend_group_make", L"%d, %d, N\'%s\'", % kPacket_.m_iUnitUID % ( int )kPacket_.m_cGroupID % kPacket_.m_wstrGroupName );
    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( kPacket.m_iOK );
        m_kODBC.EndFetch();
    }

    if( kPacket.m_iOK != NetError::NET_OK )
    {
        START_LOG( cerr, L"ģ�� �׷� ����� ����." )
            << BUILD_LOG( kPacket.m_iOK )
            << BUILD_LOG( LAST_SENDER_UID )
            << BUILD_LOG( kPacket_.m_iUnitUID )
            << BUILD_LOGc( kPacket_.m_cGroupID )
            << BUILD_LOG( kPacket_.m_wstrGroupName )
            << END_LOG;

        kPacket.m_iOK = NetError::ERR_MESSENGER_10;
    }

end_proc:
    SendToUser( LAST_SENDER_UID, DBE_MAKE_FRIEND_GROUP_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_RENAME_FRIEND_GROUP_REQ )
{
    KDBE_MAKE_FRIEND_GROUP_ACK kPacket;
    kPacket.m_iOK = NetError::ERR_ODBC_01;
    kPacket.m_cGroupID = kPacket_.m_cGroupID;
    kPacket.m_wstrGroupName = kPacket_.m_wstrGroupName;

    DO_QUERY( L"exec dbo.gup_friend_group_rename", L"%d, %d, N\'%s\'", % kPacket_.m_iUnitUID % ( int )kPacket_.m_cGroupID % kPacket_.m_wstrGroupName );
    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( kPacket.m_iOK );
        m_kODBC.EndFetch();
    }

    if( kPacket.m_iOK != NetError::NET_OK )
    {
        START_LOG( cerr, L"ģ�� �׷� �̸� ���� ����." )
            << BUILD_LOG( kPacket.m_iOK )
            << BUILD_LOG( LAST_SENDER_UID )
            << BUILD_LOG( kPacket_.m_iUnitUID )
            << BUILD_LOGc( kPacket_.m_cGroupID )
            << BUILD_LOG( kPacket_.m_wstrGroupName )
            << END_LOG;

        kPacket.m_iOK = NetError::ERR_MESSENGER_10;
    }

end_proc:
    SendToUser( LAST_SENDER_UID, DBE_RENAME_FRIEND_GROUP_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_DELETE_FRIEND_GROUP_REQ )
{
    KDBE_DELETE_FRIEND_GROUP_ACK kPacket;
    kPacket.m_iOK = NetError::ERR_ODBC_01;
    kPacket.m_cGroupID = kPacket_.m_cGroupID;

    DO_QUERY( L"exec dbo.gup_friend_group_delete", L"%d, %d", % kPacket_.m_iUnitUID % ( int )kPacket_.m_cGroupID );
    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( kPacket.m_iOK );
        m_kODBC.EndFetch();
    }

    if( kPacket.m_iOK != NetError::NET_OK )
    {
        START_LOG( cerr, L"ģ�� �׷� ���� ����." )
            << BUILD_LOG( kPacket.m_iOK )
            << BUILD_LOG( LAST_SENDER_UID )
            << BUILD_LOG( kPacket_.m_iUnitUID )
            << BUILD_LOGc( kPacket_.m_cGroupID )
            << END_LOG;

        kPacket.m_iOK = NetError::ERR_MESSENGER_10;
    }

end_proc:
    SendToUser( LAST_SENDER_UID, DBE_DELETE_FRIEND_GROUP_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_FRIEND_MESSAGE_NOT )
{
    int iOK = NetError::ERR_ODBC_01;

    DO_QUERY( L"exec dbo.gup_friend_send_message", L"%d, N\'%s\', %d, N\'%s\'", % kPacket_.m_iSenderUnitUID % kPacket_.m_wstrReceiverNickName % ( int )kPacket_.m_cMessageType % kPacket_.m_wstrMessage );
    if( m_kODBC.BeginFetch() )
    {
        FETCH_DATA( iOK );
        m_kODBC.EndFetch();
    }

    if( iOK != NetError::NET_OK )
    {
        START_LOG( cerr, L"ģ�� �޼��� ����� ����." )
            << BUILD_LOG( iOK )
            << BUILD_LOG( kPacket_.m_iSenderUnitUID )
            << BUILD_LOG( kPacket_.m_wstrReceiverNickName )
            << BUILD_LOGc( kPacket_.m_cMessageType )
            << BUILD_LOG( kPacket_.m_wstrMessage )
            << END_LOG;
    }

end_proc:
    return;
}

//{{ 2009. 7. 29  ������	item set cheat
IMPL_ON_FUNC( DBE_ADMIN_GET_ITEM_SET_NOT )
{
	int iOK = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec dbo.gup_insert_patchitem", L"N\'%s\'", % kPacket_.m_wstrNickName );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}

	if( iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"������ ��Ʈ ġƮ ����!" )
			<< BUILD_LOG( iOK )
			<< BUILD_LOG( kPacket_.m_wstrNickName )
			<< END_LOG;
	}

end_proc:
	return;
}
//}}

//{{ 2009. 8. 27  ������	����
IMPL_ON_FUNC( DBE_SEAL_ITEM_REQ )
{
	KDBE_SEAL_ITEM_ACK kPacket;
	kPacket.m_ucSealResult			  = kPacket_.m_ucSealResult;	
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec dbo.gup_update_item_seal", L"%d, %d", % kPacket_.m_iItemUID % kPacket_.m_ucSealResult );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"���� DB ������Ʈ ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iItemUID )
			<< BUILD_LOGc( kPacket_.m_ucSealResult )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_SEAL_ITEM_16;
		goto end_proc;
	}

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iItemUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_SEAL_ITEM_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_UNSEAL_ITEM_REQ )
{
	KDBE_UNSEAL_ITEM_ACK kPacket;
	kPacket.m_ucSealResult			  = kPacket_.m_ucSealResult;	
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec dbo.gup_update_item_seal", L"%d, %d", % kPacket_.m_iItemUID % kPacket_.m_ucSealResult );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"�������� DB ������Ʈ ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iItemUID )
			<< BUILD_LOGc( kPacket_.m_ucSealResult )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_SEAL_ITEM_16;
		goto end_proc;
	}

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iItemUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_UNSEAL_ITEM_ACK, kPacket );
}
//}}

//////////////////////////////////////////////////////////////////////////
//{{ 2009. 9. 22  ������	���
#ifdef GUILD_TEST

//{{ 2012. 02. 22	�ڼ���	��� �̸� �����
#ifdef SERV_GUILD_CHANGE_NAME
#else
IMPL_ON_FUNC( DBE_CREATE_GUILD_REQ )
{
	KDBE_CREATE_GUILD_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iItemUID = kPacket_.m_iItemUID;

	// 1. ��� â�� �������� üũ
	DO_QUERY( L"exec dbo.gup_create_guild_check", L"%d, N\'%s\'", % kPacket_.m_iUnitUID % kPacket_.m_wstrGuildName );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( clog, L"��� ������ �� ���°����� üũ��!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_wstrGuildName );

		switch( kPacket.m_iOK )
		{
		case -1: kPacket.m_iOK = NetError::ERR_GUILD_00; break; // �̹� ���� ��忡 ������
		case -2: kPacket.m_iOK = NetError::ERR_GUILD_01; break; // ��� �̸� �ߺ�
			//{{ 2009. 10. 26  ������	���
		case -3: kPacket.m_iOK = NetError::ERR_GUILD_42; break; // �� ��� �̸��� 7�ϰ� ����� �� �����ϴ�.
			//}}
		default: kPacket.m_iOK = NetError::ERR_UNKNOWN; break;
		}

#ifdef SERV_GUARANTEE_UNIQUENESS_OF_NAME_CN
		kPacket.m_bReqGiant_DeleteName = true; 
		kPacket.m_wstrGuildName = kPacket_.m_wstrGuildName;
#endif //SERV_GUARANTEE_UNIQUENESS_OF_NAME_CN
		goto end_proc;
	}

	// 2. ��� ����
#ifdef SERV_CREATE_GUILD_EVENT
	DO_QUERY( L"exec dbo.gup_create_guild", L"%d, N\'%s\', %d, N\'%s\'", 
		% kPacket_.m_iUnitUID 
		% kPacket_.m_wstrGuildName
		% 30
		% kPacket_.m_wstrGuildMessage
		); // �ʱ��ο� 30������
#else //SERV_CREATE_GUILD_EVENT
	DO_QUERY( L"exec dbo.gup_create_guild", L"%d, N\'%s\', %d, N\'%s\'", 
		% kPacket_.m_iUnitUID 
		% kPacket_.m_wstrGuildName
		% 20
		% kPacket_.m_wstrGuildMessage
		);
#endif //SERV_CREATE_GUILD_EVENT

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK
			>> kPacket.m_kCreatedGuildInfo.m_iGuildUID
			>> kPacket.m_kCreatedGuildInfo.m_wstrFoundingDay );

		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"��� ���� ����!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_wstrGuildName )
			<< END_LOG;

		switch( kPacket.m_iOK )
		{
		case -1: kPacket.m_iOK = NetError::ERR_GUILD_04; break; // ��� â�� ����
		case -2: kPacket.m_iOK = NetError::ERR_GUILD_05; break; // �����ڰ� ���� ���� �� ����			
		default: kPacket.m_iOK = NetError::ERR_UNKNOWN; break;
		}

#ifdef SERV_GUARANTEE_UNIQUENESS_OF_NAME_CN
		kPacket.m_bReqGiant_DeleteName = true; 
		kPacket.m_wstrGuildName = kPacket_.m_wstrGuildName;
#endif //SERV_GUARANTEE_UNIQUENESS_OF_NAME_CN
		goto end_proc;
	}
	else
	{
		// â�� �����ϸ� �ʱ� ��� ���� ����
		kPacket.m_kCreatedGuildInfo.m_wstrGuildName		= kPacket_.m_wstrGuildName;
#ifdef SERV_CREATE_GUILD_EVENT
		kPacket.m_kCreatedGuildInfo.m_usMaxNumMember	= 30;
#else //SERV_CREATE_GUILD_EVENT
		kPacket.m_kCreatedGuildInfo.m_usMaxNumMember	= 20;
#endif //SERV_CREATE_GUILD_EVENT
		kPacket.m_kCreatedGuildInfo.m_ucGuildLevel		= 1;
		kPacket.m_kCreatedGuildInfo.m_iGuildEXP			= 0;
		kPacket.m_kCreatedGuildInfo.m_wstrGuildMessage	= kPacket_.m_wstrGuildMessage;
	}

	//{{ 2009. 12. 3  ������	��彺ų
	// 3. ��� ��ų ����Ʈ
#ifdef GUILD_SKILL_TEST

	DO_QUERY( L"exec dbo.gup_update_guild_skill_point", L"%d, %d",
		% kPacket.m_kCreatedGuildInfo.m_iGuildUID
		% 1
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"��� ���� ���Ŀ� ��ų ����Ʈ 1����Ʈ insert�� �����Ͽ���. ������ ���¿���!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket.m_kCreatedGuildInfo.m_iGuildUID )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		switch( kPacket.m_iOK )
		{
		case -1: kPacket.m_iOK = NetError::ERR_GUILD_06; break; // ���������ʴ� ���
		default: kPacket.m_iOK = NetError::ERR_UNKNOWN; break;
		}

		goto end_proc;
	}
	else
	{
		// ��� ��ų ����Ʈ �ʱⰪ�� 1����Ʈ��.
		kPacket.m_kGuildSkillInfo.m_iGuildSPoint = 1;
	}

#endif GUILD_SKILL_TEST
	//}}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_CREATE_GUILD_ACK, kPacket );
}

#endif SERV_GUILD_CHANGE_NAME
//}}
_IMPL_ON_FUNC( DBE_INVITE_GUILD_NICKNAME_CHECK_REQ, KELG_INVITE_GUILD_ACK )
{
	int iOK = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec dbo.gup_get_unit_uid", L"N\'%s\'", % kPacket_.m_wstrReceiverNickName );

	UidType iUnitUID = 0;
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iUnitUID );
		m_kODBC.EndFetch();
	}

	if( iUnitUID != 0 )
	{
		kPacket_.m_iOK = NetError::ERR_GUILD_08; // ���� ���������� �����Դϴ�.
	}
	else
	{
		kPacket_.m_iOK = NetError::ERR_SEARCH_UNIT_04; // �������� �ʴ� �г����Դϴ�.
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_INVITE_GUILD_NICKNAME_CHECK_ACK, kPacket_ );
}

IMPL_ON_FUNC( DBE_JOIN_GUILD_REQ )
{
	KDBE_JOIN_GUILD_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iGuildUID = kPacket_.m_iGuildUID;
	kPacket.m_kJoinGuildMember = kPacket_.m_kJoinGuildMember;	

	//////////////////////////////////////////////////////////////////////////
	// ��� ����
	DO_QUERY( L"exec dbo.gup_create_guild_member", L"%d, %d, N\'%s\'", 
		% kPacket_.m_kJoinGuildMember.m_iUnitUID
		% kPacket_.m_iGuildUID
		% L""
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}
	
	if( kPacket.m_iOK != NetError::NET_OK )
	{
		if( kPacket.m_iOK == -3 )
		{
			START_LOG( cwarn, L"��� �ִ� ���� �ο� ����." )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( kPacket_.m_iGuildUID )
				<< BUILD_LOG( kPacket_.m_kJoinGuildMember.m_iUnitUID )
				<< END_LOG;
		}
		else
		{
			START_LOG( cerr, L"��� ���� ����." )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( kPacket_.m_iGuildUID )
				<< BUILD_LOG( kPacket_.m_kJoinGuildMember.m_iUnitUID )
				<< END_LOG;
		}

		switch( kPacket.m_iOK )
		{
		case -1: kPacket.m_iOK = NetError::ERR_GUILD_14; break; // �̹� ��忡 ���ԵǾ�����
		case -2: kPacket.m_iOK = NetError::ERR_GUILD_18; break; // ��� ���� ����
		case -3: kPacket.m_iOK = NetError::ERR_GUILD_18; break; // ��� ���� �ִ� �ο� ����
		default: kPacket.m_iOK = NetError::ERR_UNKNOWN; break;
		}
	}

end_proc:	
	SendToServer( DBE_JOIN_GUILD_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_CHANGE_GUILD_MEMBER_GRADE_REQ )
{
	KDBE_CHANGE_GUILD_MEMBER_GRADE_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iGuildUID = kPacket_.m_iGuildUID;
	kPacket.m_iTargetUnitUID = kPacket_.m_iTargetUnitUID;
	kPacket.m_wstrTargetNickName = kPacket_.m_wstrTargetNickName;
	kPacket.m_ucMemberShipGrade = kPacket_.m_ucMemberShipGrade;	
	kPacket.m_bChangeGuildMaster = kPacket_.m_bChangeGuildMaster; // �ݵ�� �Ѱ�����ϴ� ��!
	
	//////////////////////////////////////////////////////////////////////////
	// ��� ���� ��û!
	
	if( kPacket_.m_bChangeGuildMaster )
	{
		// ��� ������ ����
		DO_QUERY( L"exec dbo.gup_update_guild_master", L"%d, %d, %d, %d, %d",
			% kPacket_.m_iUnitUID
			% kPacket_.m_iTargetUnitUID 
			% (int)SEnum::GUG_NORMAL_USER
			% (int)SEnum::GUG_MASTER
			% kPacket_.m_iGuildUID
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"��帶���� ���� ����." )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( kPacket_.m_iTargetUnitUID )
				<< BUILD_LOG( kPacket_.m_iGuildUID )
				<< END_LOG;

			switch( kPacket.m_iOK )
			{
			case -1: kPacket.m_iOK = NetError::ERR_GUILD_19; break; // �ش� ��忡 ���� ������ �ƴմϴ�.
			case -2: kPacket.m_iOK = NetError::ERR_GUILD_21; break; // ���� �渶 ���º�ȯ ����
			case -3: kPacket.m_iOK = NetError::ERR_GUILD_21; break; // �� �渶 ���º�ȯ ����
			default: kPacket.m_iOK = NetError::ERR_UNKNOWN; break; // ���� ��� ���� ����
			}
		}
	}
	else
	{
		// ��� ��� ��� ����
		DO_QUERY( L"exec dbo.gup_update_guild_member_grade", L"%d, %d, %d",
			% kPacket_.m_iTargetUnitUID 
			% kPacket_.m_iGuildUID
			% kPacket_.m_ucMemberShipGrade 
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"���� ��� ���� ����." )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( kPacket_.m_iGuildUID )
				<< BUILD_LOG( kPacket_.m_iTargetUnitUID )
				<< BUILD_LOGc( kPacket_.m_ucMemberShipGrade )
				<< END_LOG;

			switch( kPacket.m_iOK )
			{
			case -1: kPacket.m_iOK = NetError::ERR_GUILD_19; break; // �ش� ��忡 ���� ������ �ƴմϴ�.
			case -2: kPacket.m_iOK = NetError::ERR_GUILD_21; break; // Ʈ������ ����
			default: kPacket.m_iOK = NetError::ERR_UNKNOWN; break; // ���� ��� ���� ����
			}
		}
	}
	
end_proc:
	SendToUser( LAST_SENDER_UID, DBE_CHANGE_GUILD_MEMBER_GRADE_ACK, kPacket );

	if( kPacket.m_iOK == NetError::NET_OK )
	{
		// ��� ������ �����ϸ� ��� �Ŵ����� ������ ������ ������Ʈ �Ѵ�.
		SendToLoginServer( ELG_UPDATE_CHANGE_GUILD_MEMBER_GRADE_NOT, kPacket );
	}
}

IMPL_ON_FUNC( DBE_CHANGE_GUILD_MESSAGE_REQ )
{
	KDBE_CHANGE_GUILD_MESSAGE_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iGuildUID = kPacket_.m_iGuildUID;
	kPacket.m_wstrMessage = kPacket_.m_wstrMessage;

	//////////////////////////////////////////////////////////////////////////
	// 1. ��� �޽��� ������ ��û�� ������ ��������� �ִ��� �˾ƺ���!
	u_char ucGuildMemberShipGrade = 0;	

	if( Query_GetGuildMemberGrade( kPacket_.m_iUnitUID, ucGuildMemberShipGrade ) == false )
	{
		START_LOG( cerr, L"���� ��� ���� ��� ����." )			
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_GUILD_19; // ��忡 ���� ������ �ƴմϴ�.
		goto end_proc;
	}

	// ��� �����Ϳ� ��� �����ڸ� ��� �޽��� ��������� �ֽ��ϴ�.
	if( ucGuildMemberShipGrade != SEnum::GUG_MASTER  &&
		ucGuildMemberShipGrade != SEnum::GUG_SYSOP )
	{
		kPacket.m_iOK = NetError::ERR_GUILD_22; // ��� �޽��� ���� ������ �����ϴ�.
		goto end_proc;
	}

	//////////////////////////////////////////////////////////////////////////
	// 2. ��� �޽��� ���� ��û!

	DO_QUERY( L"exec dbo.gup_update_guild_message", L"%d, %d, N\'%s\'", 
		% kPacket_.m_iUnitUID
		% kPacket_.m_iGuildUID
		% kPacket_.m_wstrMessage
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"��� �޽��� ���� ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iGuildUID )
			<< BUILD_LOG( kPacket_.m_wstrMessage )
			<< END_LOG;

		switch( kPacket.m_iOK )
		{
		case -1: kPacket.m_iOK = NetError::ERR_GUILD_19; break; // �ش� ��忡 ���� ������ �ƴմϴ�.
		case -2: kPacket.m_iOK = NetError::ERR_GUILD_23; break; // Ʈ������ ����
		default: kPacket.m_iOK = NetError::ERR_UNKNOWN; break; // ��� �޽��� ���� ����
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_CHANGE_GUILD_MESSAGE_ACK, kPacket );
	
	if( kPacket.m_iOK == NetError::NET_OK )
	{
		// ��� �޽��� ������ �����ϸ� ��� �Ŵ����� ������ ������ ������Ʈ �Ѵ�.
		SendToLoginServer( ELG_UPDATE_CHANGE_GUILD_MESSAGE_NOT, kPacket );
	}
}

IMPL_ON_FUNC( DBE_KICK_GUILD_MEMBER_REQ )
{
	KDBE_KICK_GUILD_MEMBER_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iUnitUID = kPacket_.m_iUnitUID;
	kPacket.m_iGuildUID = kPacket_.m_iGuildUID;
	kPacket.m_iTargetUnitUID = kPacket_.m_iTargetUnitUID;

	//////////////////////////////////////////////////////////////////////////
	// 1. �������ڰ� ���� �����ϴ°Ŷ�� ���� ��û�� ������ ������ �ִ��� �˾ƺ���!
	u_char ucGuildMemberShipGrade = 0;
	
	if( Query_GetGuildMemberGrade( kPacket_.m_iUnitUID, ucGuildMemberShipGrade ) == false )
	{
		START_LOG( cerr, L"���� ��� ���� ��� ����." )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_GUILD_19; // ��忡 ���� ������ �ƴմϴ�.
		goto end_proc;
	}
	
	// �������� �����ϴ°Ŷ��?
	if( kPacket_.m_iUnitUID != kPacket_.m_iTargetUnitUID )
	{
		u_char ucTargetMemberShipGrade = 0;

		if( Query_GetGuildMemberGrade( kPacket_.m_iTargetUnitUID, ucTargetMemberShipGrade ) == false )
		{
			START_LOG( cerr, L"���� ��� ���� ��� ����." )
				<< BUILD_LOG( kPacket_.m_iTargetUnitUID )
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_GUILD_19; // ��忡 ���� ������ �ƴմϴ�.
			goto end_proc;
		}

		// ��� �����͸� ���� ��Ű���� �Ŷ��?
		if( ucTargetMemberShipGrade == SEnum::GUG_MASTER )
		{
			kPacket.m_iOK = NetError::ERR_GUILD_25; // ��� �����ʹ� ���� ���� �� �����ϴ�!
			goto end_proc;
		}
			
		// ������ ���� ������ �ִ°�?
		switch( ucGuildMemberShipGrade )
		{
		case SEnum::GUG_MASTER:			
			break;

		case SEnum::GUG_SYSOP:
			{
				// �����ڰ� �ٸ� �����ڸ� ���� ��ų�� ����.
				if( ucTargetMemberShipGrade == SEnum::GUG_SYSOP )
				{
					kPacket.m_iOK = NetError::ERR_GUILD_24; // ���� ���� ������ �����ϴ�.
					goto end_proc;
				}
			}
			break;

		default:
			{
				kPacket.m_iOK = NetError::ERR_GUILD_24; // ���� ���� ������ �����ϴ�.
				goto end_proc;
			}
			break;
		}
	}
	else
	{
		// ��� �����Ͱ� Ż���Ϸ��� �Ŷ��?
		if( ucGuildMemberShipGrade == SEnum::GUG_MASTER )
		{
			kPacket.m_iOK = NetError::ERR_GUILD_25; // ��� �����ʹ� Ż������ ����Ҽ� �����ϴ�!
			goto end_proc;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// 2. Ż������ ���� ����Ʈ �ʱ�ȭ
	
	DO_QUERY( L"exec dbo.gup_update_guild_member_exp", L"%d, %d, %d",
		% kPacket_.m_iUnitUID
		% kPacket_.m_iGuildUID
		% 0
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"���� ���� ����Ʈ �ʱ�ȭ ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iGuildUID )
			<< END_LOG;

		switch( kPacket.m_iOK )
		{
		case -1: kPacket.m_iOK = NetError::ERR_GUILD_06; break; // ������ �ƴϰų�, �߸��� ���ѹ�
		case -2: kPacket.m_iOK = NetError::ERR_GUILD_29; break; // Ʈ������ ����
		default: kPacket.m_iOK = NetError::ERR_UNKNOWN; break;
		}

		goto end_proc;
	}

	//////////////////////////////////////////////////////////////////////////	
	// 3. ���� ���� �Ǵ� ���Ż�� ��û!

	DO_QUERY( L"exec dbo.gup_delete_guild_member", L"%d, %d", 
		% kPacket_.m_iTargetUnitUID
		% kPacket_.m_iGuildUID
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"��� Ż�� ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iTargetUnitUID )
			<< BUILD_LOG( kPacket_.m_iGuildUID )
			<< END_LOG;

		switch( kPacket.m_iOK )
		{
		case -1: kPacket.m_iOK = NetError::ERR_GUILD_19; break; // �ش� ��忡 ���� ������ �ƴմϴ�.
		case -2: kPacket.m_iOK = NetError::ERR_GUILD_26; break; // Ʈ������ ����
		default: kPacket.m_iOK = NetError::ERR_UNKNOWN; break; // ��� Ż�� ����
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_KICK_GUILD_MEMBER_ACK, kPacket );
	
	if( kPacket.m_iOK == NetError::NET_OK )
	{
		// ���� Ż�� �����ϸ� ��� �Ŵ����� ������ ������ ������Ʈ �Ѵ�.
		SendToLoginServer( ELG_UPDATE_KICK_GUILD_MEMBER_NOT, kPacket );
	}
}

IMPL_ON_FUNC( DBE_CHANGE_GUILD_MEMBER_MESSAGE_REQ )
{
	KDBE_CHANGE_GUILD_MEMBER_MESSAGE_ACK kPacket;
    kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iUnitUID = kPacket_.m_iUnitUID;
	kPacket.m_iTargetUnitUID = kPacket_.m_iTargetUnitUID;
	kPacket.m_iGuildUID = kPacket_.m_iGuildUID;
	kPacket.m_wstrMessage = kPacket_.m_wstrMessage;

	// �ٸ� ������ �޽����� �����ϴ°��̶��!
	if( kPacket_.m_iUnitUID != kPacket_.m_iTargetUnitUID )
	{
		// ��û���� ����� ����!
		u_char ucGuildMemberShipGrade = 0;

		if( Query_GetGuildMemberGrade( kPacket_.m_iUnitUID, ucGuildMemberShipGrade ) == false )
		{
			START_LOG( cerr, L"���� ��� ���� ��� ����." )				
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_GUILD_19; // ��忡 ���� ������ �ƴմϴ�.
			goto end_proc;
		}

		// ������� ����� ����!
		u_char ucTargerGrade = 0;

		if( Query_GetGuildMemberGrade( kPacket_.m_iTargetUnitUID, ucTargerGrade ) == false )
		{
			START_LOG( cerr, L"���� ��� ���� ��� ����." )				
				<< BUILD_LOG( kPacket_.m_iTargetUnitUID )
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_GUILD_19; // ��忡 ���� ������ �ƴմϴ�.
			goto end_proc;
		}

		// ��޺� ���� ����
		switch( ucTargerGrade )
		{
		case SEnum::GUG_MASTER:
			{
				START_LOG( cerr, L"�ٸ� �������� ��帶������ �λ縻�� ���뺯���Ҽ�����!" )
					<< BUILD_LOG( kPacket_.m_iGuildUID )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( kPacket_.m_iTargetUnitUID )
					<< END_LOG;

                kPacket.m_iOK = NetError::ERR_GUILD_28;
				goto end_proc;
			}
			break;

		case SEnum::GUG_SYSOP:
			{
				if( ucGuildMemberShipGrade != SEnum::GUG_MASTER )
				{
					kPacket.m_iOK = NetError::ERR_GUILD_28; // ��� �������� �λ縻�� ��帶���͸� ���������մϴ�.
					goto end_proc;
				}
			}
			break;

		case SEnum::GUG_OLD_USER:
		case SEnum::GUG_VIP_USER:
		case SEnum::GUG_NORMAL_USER:
		case SEnum::GUG_NEW_USER:
			{
				if( ucGuildMemberShipGrade != SEnum::GUG_MASTER  &&
					ucGuildMemberShipGrade != SEnum::GUG_SYSOP )
				{
					kPacket.m_iOK = NetError::ERR_GUILD_28; // �Ϲ� ����� �λ縻�� ��帶���Ϳ� �������ڸ� ���������մϴ�.
					goto end_proc;
				}
			}
			break;

		default:
			{
				START_LOG( cerr, L"���� ��� ���� ��� ����." )
					<< BUILD_LOG( kPacket.m_iOK )
					<< BUILD_LOG( kPacket_.m_iTargetUnitUID )
					<< END_LOG;

				kPacket.m_iOK = NetError::ERR_GUILD_19; // ��忡 ���� ������ �ƴմϴ�.
				goto end_proc;
			}
		}
	}

	// ���� �λ縻 ����
	DO_QUERY( L"exec dbo.gup_update_guild_member_message", L"%d, %d, N\'%s\'", 
		% kPacket_.m_iTargetUnitUID
		% kPacket_.m_iGuildUID
		% kPacket_.m_wstrMessage
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"��� Ż�� ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iTargetUnitUID )
			<< BUILD_LOG( kPacket_.m_iGuildUID )
			<< BUILD_LOG( kPacket_.m_wstrMessage )
			<< END_LOG;

		switch( kPacket.m_iOK )
		{
		case -1: kPacket.m_iOK = NetError::ERR_GUILD_19; break; // �ش� ��忡 ���� ������ �ƴմϴ�.
		case -2: kPacket.m_iOK = NetError::ERR_GUILD_29; break; // Ʈ������ ����
		default: kPacket.m_iOK = NetError::ERR_UNKNOWN; break; // ���� �λ縻 ���� ����
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_CHANGE_GUILD_MEMBER_MESSAGE_ACK, kPacket );
	
	if( kPacket.m_iOK == NetError::NET_OK )
	{
		// ���� �λ縻 ������ �����ϸ� ��� �Ŵ����� ������ ������ ������Ʈ �Ѵ�.
		SendToLoginServer( ELG_UPDATE_CHANGE_GUILD_MEMBER_MESSAGE_NOT, kPacket );
	}
}

IMPL_ON_FUNC( DBE_DISBAND_GUILD_REQ )
{
	KDBE_DISBAND_GUILD_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iGuildUID = kPacket_.m_iGuildUID;

	//////////////////////////////////////////////////////////////////////////
	// 1. �������ڰ� ���� �����ϴ°Ŷ�� ���� ��û�� ������ ������ �ִ��� �˾ƺ���!
	u_char ucGuildMemberShipGrade = 0;

	if( Query_GetGuildMemberGrade( kPacket_.m_iUnitUID, ucGuildMemberShipGrade ) == false )
	{
		START_LOG( cerr, L"���� ��� ���� ��� ����." )			
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_GUILD_19; // ��忡 ���� ������ �ƴմϴ�.
		goto end_proc;
	}

	// ��� �����͸� ����ػ��� �Ҽ��ִ�
	if( ucGuildMemberShipGrade != SEnum::GUG_MASTER )
	{
		START_LOG( cerr, L"��� �����Ͱ� �ƴѵ� ����ػ��� �Ϸ���!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( ucGuildMemberShipGrade )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_GUILD_30; // ��帶���Ͱ� �ƴմϴ�.
		goto end_proc;
	}

	//////////////////////////////////////////////////////////////////////////	
	// 2. ��� �ػ� ��û

	//��� �ػ�
	DO_QUERY( L"exec dbo.gup_delete_guild", L"%d, %d", 
		% kPacket_.m_iUnitUID
		% kPacket_.m_iGuildUID
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"��� Ż�� ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )	
			<< BUILD_LOG( kPacket_.m_iGuildUID )
			<< END_LOG;

		switch( kPacket.m_iOK )
		{
		case -1: kPacket.m_iOK = NetError::ERR_GUILD_19; break; // �ش� ������ �ش� ������ �ƴ�
		case -2: kPacket.m_iOK = NetError::ERR_GUILD_31; break; // ����� ������ 1�� �ʰ�
		case -3: kPacket.m_iOK = NetError::ERR_GUILD_32; break; // ���� ���� ����
		case -4: kPacket.m_iOK = NetError::ERR_GUILD_32; break; // ��� ������ ������Ʈ ����
		case -5: kPacket.m_iOK = NetError::ERR_GUILD_32; break; // ��� ���� �α� ����� ����
		default: kPacket.m_iOK = NetError::ERR_UNKNOWN; break; // ��� �ػ� ����
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_DISBAND_GUILD_ACK, kPacket );
	
	if( kPacket.m_iOK == NetError::NET_OK )
	{
		// ����ػ��� �����ϸ� ���Ŵ����� �ش� ��带 ���췯 ����!
		SendToLoginServer( ELG_UPDATE_DISBAND_GUILD_NOT, kPacket );
	}	
}

IMPL_ON_FUNC( DBE_EXPAND_GUILD_MAX_MEMBER_REQ )
{
	KGuildInfo kGuildInfo;
	KDBE_EXPAND_GUILD_MAX_MEMBER_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iGuildUID = kPacket_.m_iGuildUID;

	//////////////////////////////////////////////////////////////////////////
	// ��� ���������� Ȯ��
	u_char ucGuildMemberShipGrade = 0;

	if( Query_GetGuildMemberGrade( kPacket_.m_iUnitUID, ucGuildMemberShipGrade ) == false )
	{
		START_LOG( cerr, L"���� ��� ���� ��� ����." )			
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_GUILD_19; // ��忡 ���� ������ �ƴմϴ�.
		goto end_proc;
	}

	// ��� ������, ��� �����͸� ����ο� Ȯ���� �� �� �ִ�.
	if( ucGuildMemberShipGrade != SEnum::GUG_MASTER  &&
		ucGuildMemberShipGrade != SEnum::GUG_SYSOP )
	{
		START_LOG( cerr, L"��� ������ �Ǵ� ��� �����ڰ� �ƴѵ� ��� �ο� Ȯ���� �Ϸ���!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( ucGuildMemberShipGrade )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_GUILD_40;
		goto end_proc;
	}

	//////////////////////////////////////////////////////////////////////////
	// ��� Ȯ�� ������ �ο����� Ȯ��

	DO_QUERY( L"exec dbo.gup_get_guild_info", L"%d", % kPacket_.m_iGuildUID );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK
			>> kGuildInfo.m_wstrGuildName
			>> kGuildInfo.m_usMaxNumMember
			>> kGuildInfo.m_ucGuildLevel
			>> kGuildInfo.m_iGuildEXP
			>> kGuildInfo.m_wstrGuildMessage
			>> kGuildInfo.m_wstrFoundingDay );

		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"��� ���� ��� ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iGuildUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_GUILD_06;
		goto end_proc;
	}
	
	if( kGuildInfo.m_usMaxNumMember >= KGuildInfo::GUILD_MAX_MEMBER_LIMIT )
	{
		START_LOG( cerr, L"��� �ο��� ���̻� Ȯ���� �� �����ϴ�." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iGuildUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_GUILD_41;
		goto end_proc;
	}
	else
	{
		// �ο� Ȯ��!
		kPacket.m_usMaxNumMember = kGuildInfo.m_usMaxNumMember + 10;
		if( kPacket.m_usMaxNumMember > KGuildInfo::GUILD_MAX_MEMBER_LIMIT )
			kPacket.m_usMaxNumMember = KGuildInfo::GUILD_MAX_MEMBER_LIMIT;
	}

	//////////////////////////////////////////////////////////////////////////
	// ��� �ο� Ȯ��

	DO_QUERY( L"exec dbo.gup_update_guild_maxno", L"%d, %d", % kPacket_.m_iGuildUID % kPacket.m_usMaxNumMember );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"��� �ο� Ȯ�� ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iGuildUID )
			<< BUILD_LOG( kPacket.m_usMaxNumMember )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_GUILD_39;
	}

end_proc:
	SendToServer( DBE_EXPAND_GUILD_MAX_MEMBER_ACK, kPacket );
}

//{{ 2009. 10. 27  ������	��巹��
IMPL_ON_FUNC( DBE_UPDATE_GUILD_EXP_REQ )
{
	KDBE_UPDATE_GUILD_EXP_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iUnitUID = kPacket_.m_iUnitUID;
	kPacket.m_iGuildUID = kPacket_.m_iGuildUID;

	KGuildInfo kGuildInfo;

	//////////////////////////////////////////////////////////////////////////
	// ���� ����Ʈ ������Ʈ
	DO_QUERY( L"exec dbo.gup_update_guild_member_exp", L"%d, %d, %d",
		% kPacket_.m_iUnitUID
		% kPacket_.m_iGuildUID
		% kPacket_.m_iHonorPoint
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"���� ���� ����Ʈ ������Ʈ ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iGuildUID )
			<< BUILD_LOG( kPacket_.m_iHonorPoint )
			<< END_LOG;

		switch( kPacket.m_iOK )
		{
		case -1: kPacket.m_iOK = NetError::ERR_GUILD_06; break; // ������ �ƴϰų�, �߸��� ���ѹ�
		case -2: kPacket.m_iOK = NetError::ERR_GUILD_29; break; // Ʈ������ ����
		default: kPacket.m_iOK = NetError::ERR_UNKNOWN; break;
		}

		goto end_proc;
	}

	//////////////////////////////////////////////////////////////////////////
	// ��� EXP ���	

	DO_QUERY( L"exec dbo.gup_get_guild_info", L"%d", % kPacket_.m_iGuildUID );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK
			>> kGuildInfo.m_wstrGuildName
			>> kGuildInfo.m_usMaxNumMember
			>> kGuildInfo.m_ucGuildLevel
			>> kGuildInfo.m_iGuildEXP
			>> kGuildInfo.m_wstrGuildMessage
			>> kGuildInfo.m_wstrFoundingDay );

		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"��� ���� ��� ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iGuildUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_GUILD_06;
		goto end_proc;
	}

	//////////////////////////////////////////////////////////////////////////
	// ��� ���� ���� üũ
	if( kGuildInfo.m_iGuildEXP >= kPacket_.m_iGuildLimitEXP )
	{
		START_LOG( clog, L"���̻� ��� ����ġ�� ȹ���� �� �����ϴ�." )
			<< BUILD_LOG( kGuildInfo.m_iGuildEXP )
			<< BUILD_LOG( kPacket_.m_iGuildLimitEXP );
		
		kPacket.m_iOK = NetError::ERR_GUILD_43;
		goto end_proc;
	}
	else
	{
		// ������ ����ġ
		kPacket.m_iGuildEXP = kGuildInfo.m_iGuildEXP + kPacket_.m_iGuildEXP;
		
		if( kPacket.m_iGuildEXP > kPacket_.m_iGuildLimitEXP )
		{
			kPacket.m_iGuildEXP = kPacket_.m_iGuildLimitEXP;

			// ���а� �ٽ� ���
			kPacket_.m_iGuildEXP = kPacket_.m_iGuildLimitEXP - kGuildInfo.m_iGuildEXP;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// ��� EXP ������Ʈ
	DO_QUERY( L"exec dbo.gup_update_guild_exp", L"%d, %d, %d",
		% kPacket_.m_iUnitUID
		% kPacket_.m_iGuildUID
		% kPacket_.m_iGuildEXP
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"��� EXP ������Ʈ ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iGuildUID )
			<< BUILD_LOG( kPacket.m_iGuildEXP )
			<< END_LOG;

		switch( kPacket.m_iOK )
		{
		case -1: kPacket.m_iOK = NetError::ERR_GUILD_06; break; // ������ �ƴϰų�, �߸��� ���ѹ�
		case -2: kPacket.m_iOK = NetError::ERR_GUILD_29; break; // Ʈ������ ����
		default: kPacket.m_iOK = NetError::ERR_UNKNOWN; break;
		}

		goto end_proc;
	}

end_proc:
	SendToServer( DBE_UPDATE_GUILD_EXP_ACK, kPacket );
}
//}}

#endif GUILD_TEST
//}}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//{{ 2009. 11. 24  ������	��彺ų
#ifdef GUILD_SKILL_TEST

IMPL_ON_FUNC( DBE_RESET_GUILD_SKILL_REQ )
{
	KDBE_RESET_GUILD_SKILL_ACK kPacket;
	kPacket.m_iOK						= NetError::ERR_ODBC_01;
	kPacket.m_iGuildUID					= kPacket_.m_iGuildUID;
	kPacket.m_iGuildSkillID				= kPacket_.m_iGuildSkillID;
	kPacket.m_iGuildSkillLevel			= kPacket_.m_iGuildSkillLevel;
	kPacket.m_iGuildSkillCSPoint		= kPacket_.m_iGuildSkillCSPoint;
	kPacket.m_iGuildSPoint				= kPacket_.m_iGuildSPoint;
	kPacket.m_iGuildCSPoint				= kPacket_.m_iGuildCSPoint;		
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	if( !kPacket.m_kItemQuantityUpdate.m_mapQuantityChange.empty()  ||  !kPacket.m_kItemQuantityUpdate.m_vecDeleted.empty() )
	{
		START_LOG( cerr, L"��� ��ų �ʱ�ȭ ������ DB���� �������� Ȥ�� ���� ����." )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_GUILD_SKILL_08;
		goto end_proc;
	}

	// ��ų �ʱ�ȭ
	DO_QUERY( L"exec dbo.gup_update_guild_skill_info_new", L"%d, %d, %d, %d",
		% kPacket_.m_iGuildUID
		% kPacket_.m_iGuildSkillID 
		% kPacket_.m_iGuildSkillLevel
		% kPacket_.m_iGuildSkillCSPoint
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"��� ��ų �ǵ����� DB���� ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iGuildUID )
			<< BUILD_LOG( kPacket_.m_iGuildSkillID )
			<< BUILD_LOG( kPacket_.m_iGuildSkillLevel )
			<< BUILD_LOG( kPacket_.m_iGuildSkillCSPoint )
			<< END_LOG;

		switch( kPacket.m_iOK )
		{
		case -1: kPacket.m_iOK = NetError::ERR_GUILD_06;		break; // ���������ʴ� ���
		case -2: kPacket.m_iOK = NetError::ERR_GUILD_SKILL_10;	break;
		case -3: kPacket.m_iOK = NetError::ERR_GUILD_SKILL_10;	break;
		default: kPacket.m_iOK = NetError::ERR_UNKNOWN;			break;
		}
		
		goto end_proc;
	}

	// ��� ĳ�� ��ų ����Ʈ ���� ����
	if( kPacket_.m_iGuildCSPoint >= 0 )
	{
		DO_QUERY( L"exec dbo.gup_update_guild_cash_skill_point_info", L"%d, %d, %d",
			% kPacket_.m_iGuildUID
			% kPacket_.m_iGuildCSPoint
			% kPacket_.m_iMaxGuildCSPoint
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}	

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"��� ��ų �ǵ������� ���Ŀ� �Ⱓ�� ��ų ����Ʈ DB������Ʈ �ϴ� ����!" )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( LAST_SENDER_UID )
				<< BUILD_LOG( kPacket_.m_iGuildUID )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( kPacket_.m_iGuildCSPoint )
				<< BUILD_LOG( kPacket_.m_iMaxGuildCSPoint )
				<< END_LOG;

			switch( kPacket.m_iOK )
			{
			case -1: kPacket.m_iOK = NetError::ERR_GUILD_06; break; // ���������ʴ� ���
			default: kPacket.m_iOK = NetError::ERR_UNKNOWN; break;
			}

			goto end_proc;
		}
	}
	// ��� ��ų ����Ʈ ���� ����
	else
	{
		DO_QUERY( L"exec dbo.gup_update_guild_skill_point", L"%d, %d",
			% kPacket_.m_iGuildUID
			% kPacket_.m_iGuildSPoint
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}	

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"��� ��ų �ǵ������� ���Ŀ� �ý�ų ����Ʈ DB������Ʈ �ϴ� ����!" )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( LAST_SENDER_UID )
				<< BUILD_LOG( kPacket_.m_iGuildUID )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( kPacket_.m_iGuildSPoint )
				<< END_LOG;

			switch( kPacket.m_iOK )
			{
			case -1: kPacket.m_iOK = NetError::ERR_GUILD_06; break; // ���������ʴ� ���
			default: kPacket.m_iOK = NetError::ERR_UNKNOWN; break;
			}

			goto end_proc;
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_RESET_GUILD_SKILL_ACK, kPacket );

	if( kPacket.m_iOK == NetError::NET_OK )
	{
		SendToLoginServer( ELG_RESET_GUILD_SKILL_DEL_ITEM_ACK, kPacket );
	}
}

IMPL_ON_FUNC( DBE_INIT_GUILD_SKILL_REQ )
{
	KDBE_INIT_GUILD_SKILL_ACK kPacket;
	kPacket.m_iOK				= NetError::ERR_ODBC_01;
	kPacket.m_iGuildUID			= kPacket_.m_iGuildUID;
	kPacket.m_iGuildSPoint		= kPacket_.m_iGuildSPoint;
	kPacket.m_iGuildCSPoint		= kPacket_.m_iGuildCSPoint;
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	if( !kPacket.m_kItemQuantityUpdate.m_mapQuantityChange.empty()  ||  !kPacket.m_kItemQuantityUpdate.m_vecDeleted.empty() )
	{
		START_LOG( cerr, L"��� ��ų �ʱ�ȭ ������ DB���� �������� Ȥ�� ���� ����." )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_GUILD_SKILL_12;
		goto end_proc;
	}

	// ��� ��ų �ʱ�ȭ
	DO_QUERY( L"exec dbo.gup_delete_all_guild_skill", L"%d, %d, %d",
		% kPacket_.m_iGuildUID
		% kPacket_.m_iGuildSPoint
		% kPacket_.m_iGuildCSPoint
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"��� ��ų �ʱ�ȭ DB���� ����.!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iGuildUID )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iItemUID )
			<< BUILD_LOG( kPacket_.m_iGuildSPoint )
			<< BUILD_LOG( kPacket_.m_iGuildCSPoint )
			<< END_LOG;

		switch( kPacket.m_iOK )
		{
		case -1: kPacket.m_iOK = NetError::ERR_GUILD_06; break; // ���������ʴ� ���
		case -2: kPacket.m_iOK = NetError::ERR_GUILD_SKILL_14; break;
		case -3: kPacket.m_iOK = NetError::ERR_GUILD_SKILL_14; break;
		default: kPacket.m_iOK = NetError::ERR_UNKNOWN; break;
		}

		goto end_proc;
	}

	kPacket.m_iOK = NetError::NET_OK;

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_INIT_GUILD_SKILL_ACK, kPacket );

	if( kPacket.m_iOK == NetError::NET_OK )
	{
		SendToLoginServer( ELG_INIT_GUILD_SKILL_DEL_ITEM_ACK, kPacket );
	}
}

#endif GUILD_SKILL_TEST
//}}
//////////////////////////////////////////////////////////////////////////

//{{ 2009. 12. 8  ������	ũ���������̺�Ʈ
IMPL_ON_FUNC( DBE_CHECK_TIME_EVENT_COMPLETE_REQ )
{
	KDBE_CHECK_TIME_EVENT_COMPLETE_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;

	//////////////////////////////////////////////////////////////////////////	
	// �ϴ� ������� ����!
	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
	if( Query_InsertItemList( SEnum::GIR_XMAS_EVENT, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#else
	if( Query_InsertItemList( kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
	//}}
	{
		START_LOG( cerr, L"�̺�Ʈ ���� ������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_CUMULATIVE_TIME_EVENT_02;
	}
	else
	{
		kPacket.m_iOK = NetError::NET_OK;
	}

	CTime tUpdateTime = CTime::GetCurrentTime();
	tUpdateTime += CTimeSpan( 18250, 0, 0, 0 ); // �ѹ��� ��� �ϹǷ� 50���� ���Ѵ�.
	std::wstring wstrUpdateDate = ( CStringW )( tUpdateTime.Format( _T( "%Y-%m-%d %H:%M:%S" ) ) );

	std::vector< KCumulativeTimeEventInfo >::const_iterator vit;
	for( vit = kPacket_.m_vecCompletedEvent.begin(); vit != kPacket_.m_vecCompletedEvent.end(); ++vit )
	{
		//////////////////////////////////////////////////////////////////////////
		// �ٽ� �̺�Ʈ �����Ҽ� ������ �Ϸ� ������Ʈ ���� ���� �̺�Ʈ �ٽ� ���� ���ϸ� �Ϸ� ������Ʈ ����!
		bool bRestartEvent = false;
		int iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.gup_insert_xmas_event", L"%d, %d", % kPacket_.m_iUnitUID % vit->m_iEventUID );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK <= 0 )
		{
			START_LOG( cerr, L"�̺�Ʈ Ÿ�� DB������Ʈ ����?" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( vit->m_iEventUID )
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_CUMULATIVE_TIME_EVENT_02;
			continue;
		}
		else
		{
			kPacket.m_iRewardCount = iOK;
			if( kPacket.m_iRewardCount >= 5 )
			{
				bRestartEvent = false;
			}
			else
			{
				bRestartEvent = true;
			}

			kPacket.m_iOK = NetError::NET_OK;
		}

		//////////////////////////////////////////////////////////////////////////
		// �̺�Ʈ ���� �α׸� �������!

		if( bRestartEvent == false )
		{
			DO_QUERY( L"exec dbo.gup_update_30min", L"%d, %d, N\'%s\'", % kPacket_.m_iUnitUID % vit->m_iEventUID % wstrUpdateDate );

			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( kPacket.m_iOK );
				m_kODBC.EndFetch();
			}

			if( kPacket.m_iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"�̺�Ʈ Ÿ�� DB������Ʈ ����?" )
					<< BUILD_LOG( kPacket.m_iOK )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( vit->m_iEventUID )
					<< BUILD_LOG( wstrUpdateDate )
					<< END_LOG;

				kPacket.m_iOK = NetError::ERR_CUMULATIVE_TIME_EVENT_02;
				continue;
			}
		}
		else
		{
			// �̺�Ʈ ��� �ð� �ʱ�ȭ
			DO_QUERY( L"exec dbo.gup_update_remaintime", L"%d, %d, %d", % kPacket_.m_iUnitUID % vit->m_iEventUID % 0 );

			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( iOK );
				m_kODBC.EndFetch();
			}

			if( iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"�̺�Ʈ Ÿ�� DB������Ʈ ����?" )
					<< BUILD_LOG( iOK )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( vit->m_iEventUID )					
					<< END_LOG;

				kPacket.m_iOK = NetError::ERR_CUMULATIVE_TIME_EVENT_02;
				continue;
			}
			else
			{
				// ������� �̺�Ʈ�� ����!
				kPacket.m_vecRestartEvent.push_back( vit->m_iEventUID );
			}
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_CHECK_TIME_EVENT_COMPLETE_ACK, kPacket );
}
//}}

#ifdef SERV_GLOBAL_BILLING
//{{ ����� : [2010/8/19/] //	���� ��� ����
IMPL_ON_FUNC( DBE_GET_NICKNAME_BY_UNITUID_REQ )
{
	KDBE_GET_NICKNAME_BY_UNITUID_ACK kPacket;
	kPacket.m_vecBillOrderInfo = kPacket_.m_vecBillOrderInfo;
#ifdef SERV_GLOBAL_CASH_PACKAGE
	kPacket.m_vecPackageInfo = kPacket_.m_vecPackageInfo;
	kPacket.m_vecSubPackageInfo = kPacket_.m_vecSubPackageInfo;
	kPacket.m_vecSubPackageTrans = kPacket_.m_vecSubPackageTrans;
#endif //SERV_GLOBAL_CASH_PACKAGE

	std::vector< UidType >::iterator vit;
	for( vit = kPacket_.m_vecUID.begin(); vit != kPacket_.m_vecUID.end(); vit++ )
	{
		std::wstring wstrNickName;
		UidType	fromUnitUID = *vit;

		DO_QUERY( L"exec dbo.gup_get_nickname", L"%d", % fromUnitUID );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( wstrNickName );
			m_kODBC.EndFetch();
		}

		START_LOG( clog2, L"[�׽�Ʈ�α�]")
			<< BUILD_LOG( fromUnitUID )
			<< BUILD_LOG( wstrNickName )
			<< END_LOG;

		kPacket.m_mapNickName.insert( std::make_pair< UidType, std::wstring >( fromUnitUID, wstrNickName ) );
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_GET_NICKNAME_BY_UNITUID_ACK, kPacket );
}
//}} ����� : [2010/8/19/] //	���� ��� ����
#endif // SERV_GLOBAL_BILLING


//{{ 2010. 01. 11  ������	��õ�θ���Ʈ
IMPL_ON_FUNC( DBE_GET_RECOMMEND_USER_LIST_REQ )
{
	KDBE_GET_RECOMMEND_USER_LIST_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	// ��õ�� ����Ʈ
	DO_QUERY( L"exec dbo.gup_get_recommend_list_me", L"%d", % kPacket_.m_iUnitUID );

	while( m_kODBC.Fetch() )
	{
		KRecommendUserInfo kInfo;
		char cUnitClass = 0;

		FETCH_DATA( kInfo.m_iUnitUID
			>> kInfo.m_ucLevel
			>> cUnitClass
			>> kInfo.m_wstrNickName
			>> kInfo.m_wstrRecommendDate );
		
		kPacket.m_vecRecommendUserList.push_back( kInfo );
	}

	kPacket.m_iOK = NetError::NET_OK;

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_GET_RECOMMEND_USER_LIST_ACK, kPacket );
}
//}}

//{{ 2010. 02. 23  ������	�� ����Ʈ �̺�Ʈ
#ifdef SERV_WEB_POINT_EVENT

IMPL_ON_FUNC( DBE_ATTENDANCE_CHECK_REQ )
{
	KDBE_ATTENDANCE_CHECK_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	// �⼮ üũ
	DO_QUERY( L"exec dbo.gup_insert_event_attendance", L"%d", % kPacket_.m_iUnitUID );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		if( kPacket.m_iOK != -1 )
		{
			START_LOG( cerr, L"�⼮ üũ ����!" )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< END_LOG;
		}

		switch( kPacket.m_iOK )
		{
		case -1: kPacket.m_iOK = NetError::ERR_WEB_POINT_EVENT_00; break;
		case -2: kPacket.m_iOK = NetError::ERR_WEB_POINT_EVENT_02; break;
		default: kPacket.m_iOK = NetError::ERR_UNKNOWN; break;
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_ATTENDANCE_CHECK_ACK, kPacket );
}

_IMPL_ON_FUNC( DBE_INCREASE_WEB_POINT_LOG_NOT, KDBE_INCREASE_WEB_POINT_ACK )
{	
	int iOK = NetError::ERR_ODBC_01;

	// �� ����Ʈ ȹ�� �α�
	DO_QUERY( L"exec dbo.gup_insert_event_webpoint", L"%d, %d, %d", % kPacket_.m_iUnitUID % (int)kPacket_.m_cPointType % kPacket_.m_iIncreasePoint );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}

	if( iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"�� ����Ʈ ȹ�� �α� ����!" )
			<< BUILD_LOG( iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;
	}

end_proc:
	return;
}

#endif SERV_WEB_POINT_EVENT
//}}

//{{ 2010. 03. 22  ������	����� ��Ʈ
#ifdef SERV_SKILL_NOTE

IMPL_ON_FUNC( DBE_EXPAND_SKILL_NOTE_PAGE_REQ )
{
	KDBE_EXPAND_SKILL_NOTE_PAGE_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_cExpandedMaxPageNum = kPacket_.m_cExpandedMaxPageNum;

	// ����� ��Ʈ ������ Ȯ��
	DO_QUERY( L"exec dbo.gup_insert_notecnt", L"%d, %d", % kPacket_.m_iUnitUID % (int)kPacket_.m_cExpandedMaxPageNum );
	
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}
	
	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"����� ��Ʈ ������ Ȯ�� ����!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOGc( kPacket_.m_cExpandedMaxPageNum )
			<< END_LOG;
	
		kPacket.m_iOK = NetError::ERR_SKILL_NOTE_05;
	}
	
end_proc:
	SendToUser( LAST_SENDER_UID, DBE_EXPAND_SKILL_NOTE_PAGE_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_REG_SKILL_NOTE_MEMO_REQ )
{
	KEGS_REG_SKILL_NOTE_MEMO_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_cSkillNotePageNum = kPacket_.m_cPageNum;
	kPacket.m_iMemoID = kPacket_.m_iMemoID;
	kPacket.m_vecInventorySlotInfo = kPacket_.m_vecInventorySlotInfo;

	// ����� ��Ʈ �޸� ���
	DO_QUERY( L"exec dbo.gup_insert_note", L"%d, %d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iMemoID % (int)kPacket_.m_cPageNum );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"����� ��Ʈ �޸� ��� ����!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iMemoID )
			<< BUILD_LOGc( kPacket_.m_cPageNum )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_SKILL_NOTE_03;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_REG_SKILL_NOTE_MEMO_ACK, kPacket );
}

#endif SERV_SKILL_NOTE
//}}

//{{ 2010. 7. 30 ������	�� �ý���
#ifdef SERV_PET_SYSTEM

IMPL_ON_FUNC( DBE_CREATE_PET_REQ )
{
	KDBE_CREATE_PET_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_kPetInfo = kPacket_.m_kPetInfo;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		BOOST_MAP_CONST_FOREACH( UidType, int, kItem, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( kItem.first )
				<< BUILD_LOG( kItem.second );
		}
	}
	//}}

	// �� ����
#ifdef SERV_PERIOD_PET
	// SERV_PET_AUTO_LOOTING, SERV_PETID_DATA_TYPE_CHANGE ���� �� �������� �ϳ��� ����
	DO_QUERY( L"exec dbo.gup_create_pet", L"%d, %d, N\'%s\', %d, %d, %d, %d, %d, %d", 
		% kPacket_.m_iUnitUID
		% kPacket_.m_kPetInfo.m_iPetID					// SERV_PETID_DATA_TYPE_CHANGE
		% kPacket_.m_kPetInfo.m_wstrPetName
		% (int)kPacket_.m_kPetInfo.m_cEvolutionStep
		% kPacket_.m_kPetInfo.m_sSatiety
		% kPacket_.m_kPetInfo.m_iIntimacy
		% kPacket_.m_kPetInfo.m_sExtroversion
		% kPacket_.m_kPetInfo.m_sEmotion
		% kPacket_.m_sPeriod
		);
#else SERV_PERIOD_PET

#ifdef SERV_PETID_DATA_TYPE_CHANGE //2013.07.02
	DO_QUERY( L"exec dbo.gup_create_pet", L"%d, %d, N\'%s\', %d, %d, %d, %d, %d", 
		% kPacket_.m_iUnitUID
		% kPacket_.m_kPetInfo.m_iPetID					// SERV_PETID_DATA_TYPE_CHANGE
		% kPacket_.m_kPetInfo.m_wstrPetName
		% (int)kPacket_.m_kPetInfo.m_cEvolutionStep
		% kPacket_.m_kPetInfo.m_sSatiety
		% kPacket_.m_kPetInfo.m_iIntimacy
		% kPacket_.m_kPetInfo.m_sExtroversion
		% kPacket_.m_kPetInfo.m_sEmotion
		);	
#else //SERV_PETID_DATA_TYPE_CHANGE
	DO_QUERY( L"exec dbo.gup_create_pet", L"%d, %d, N\'%s\', %d, %d, %d, %d, %d", 
		% kPacket_.m_iUnitUID
		% (int)kPacket_.m_kPetInfo.m_cPetID
		% kPacket_.m_kPetInfo.m_wstrPetName
		% (int)kPacket_.m_kPetInfo.m_cEvolutionStep
		% kPacket_.m_kPetInfo.m_sSatiety
		% kPacket_.m_kPetInfo.m_iIntimacy
		% kPacket_.m_kPetInfo.m_sExtroversion
		% kPacket_.m_kPetInfo.m_sEmotion
		);
#endif //SERV_PETID_DATA_TYPE_CHANGE

#endif SERV_PERIOD_PET

	if( m_kODBC.BeginFetch() )
	{
#ifdef SERV_PERIOD_PET
		FETCH_DATA( kPacket.m_iOK
			>> kPacket.m_kPetInfo.m_iPetUID
			>> kPacket.m_kPetInfo.m_wstrDestroyDate
			);
#else SERV_PERIOD_PET
		FETCH_DATA( kPacket.m_iOK
			>> kPacket.m_kPetInfo.m_iPetUID
			);
#endif SERV_PERIOD_PET

		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"�� ���� ����!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
#ifdef SERV_PETID_DATA_TYPE_CHANGE //2013.07.02
			<< BUILD_LOGc( kPacket_.m_kPetInfo.m_iPetID )
#else //SERV_PETID_DATA_TYPE_CHANGE
			<< BUILD_LOGc( kPacket_.m_kPetInfo.m_cPetID )
#endif //SERV_PETID_DATA_TYPE_CHANGE
#ifndef SERV_PRIVACY_AGREEMENT
			<< BUILD_LOG( kPacket_.m_kPetInfo.m_wstrPetName )
#endif SERV_PRIVACY_AGREEMENT
			<< BUILD_LOGc( kPacket_.m_kPetInfo.m_cEvolutionStep )
			<< BUILD_LOG( kPacket_.m_kPetInfo.m_sSatiety )
			<< BUILD_LOG( kPacket_.m_kPetInfo.m_iIntimacy )
			<< BUILD_LOG( kPacket_.m_kPetInfo.m_sExtroversion )
			<< BUILD_LOG( kPacket_.m_kPetInfo.m_sEmotion )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_PET_00;
		goto end_proc;
	}

#ifdef SERV_FREE_AUTO_LOOTING
	if( kPacket.m_kPetInfo.m_bFreeAutoLooting == true )
	{
		kPacket.m_iOK = NetError::ERR_ODBC_01;

		DO_QUERY( L"exec dbo.P_GPet_Info_PickUP_UPT", L"%d", % kPacket.m_kPetInfo.m_iPetUID );
		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"���� ���� ���� ��� DB ������Ʈ�� ���� �Ͽ����ϴ�." )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( kPacket.m_kPetInfo.m_iPetUID )
				<< END_LOG;
			kPacket.m_kPetInfo.m_bFreeAutoLooting = false;
			goto end_proc;
		}
	}
#endif SERV_FREE_AUTO_LOOTING

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_CREATE_PET_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_SUMMON_PET_REQ )
{
	KDBE_SUMMON_PET_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iBeforeSummonPetUID = kPacket_.m_iBeforeSummonPetUID;
	kPacket.m_iSummonPetUID = kPacket_.m_iSummonPetUID;

	//////////////////////////////////////////////////////////////////////////
	// �κ��丮 ���� ������ ������Ʈ ( ��ȯ �����Ǵ� ���� �κ� ������ ���� �Ȱ� ) 

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );	
	LIF( Query_UpdateItemEndurance( kPacket_.m_iUnitUID, kPacket_.m_kItemEnduranceUpdate, kPacket.m_kItemEnduranceUpdate ) );
	Query_UpdateItemPosition( kPacket_.m_iUnitUID, kPacket_.m_kItemPositionUpdate, kPacket.m_kItemPositionUpdate );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		BOOST_MAP_CONST_FOREACH( UidType, int, kItem, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( kItem.first )
				<< BUILD_LOG( kItem.second );
		}
	}
	//}}
    
	//////////////////////////////////////////////////////////////////////////
	// ��ȯ ����!
	if( kPacket_.m_iBeforeSummonPetUID > 0 )
	{
		// �� ��ȯ ����
		DO_QUERY( L"exec dbo.gup_update_pet_call", L"%d, %d", % kPacket_.m_iBeforeSummonPetUID % 0 );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"�� ��ȯ ���� ����!" )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( kPacket_.m_iBeforeSummonPetUID )
				<< END_LOG;

			// �� ��ȯ ���� �����ϸ� ���⼭ ���ư���!
			kPacket.m_iOK = NetError::ERR_PET_08;
			goto end_proc;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// �� ��ȯ!
	if( kPacket_.m_iSummonPetUID > 0 )
	{
		DO_QUERY( L"exec dbo.gup_update_pet_call", L"%d, %d", % kPacket_.m_iSummonPetUID % 1 );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"�� ��ȯ ����!" )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( kPacket_.m_iSummonPetUID )
				<< END_LOG;

			// �� ��ȯ �����ϸ� ���⼭ ���ư���!
			kPacket.m_iOK = NetError::ERR_PET_04;
			goto end_proc;
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_SUMMON_PET_ACK, kPacket );
}

//{{ 2012. 02. 22	�ڼ���	�� �̸� �����
#ifdef SERV_PET_CHANGE_NAME
IMPL_ON_FUNC( DBE_CHANGE_PET_NAME_REQ )
{
	KDBE_CHANGE_PET_NAME_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iItemUID = kPacket_.m_iItemUID;
	kPacket.m_kPetInfo = kPacket_.m_kPetInfo;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		BOOST_MAP_CONST_FOREACH( UidType, int, kItem, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( kItem.first )
				<< BUILD_LOG( kItem.second );
		}
	}
	//}}

	if( kPacket.m_kItemQuantityUpdate.m_mapQuantityChange.find( kPacket_.m_iItemUID ) != kPacket.m_kItemQuantityUpdate.m_mapQuantityChange.end() )
	{
		START_LOG( cerr, L"DBE_CHANGE_GUILD_NAME_REQ: ���� ���� ����" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iItemUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_ITEM_18;
		goto end_proc;
	}
	else
	{
		BOOST_TEST_FOREACH( const KDeletedItemInfo&, kDeletedItemInfo, kPacket.m_kItemQuantityUpdate.m_vecDeleted )
		{
			if( kDeletedItemInfo.m_iItemUID == kPacket_.m_iItemUID )
			{
				START_LOG( cerr, L"DBE_CHANGE_GUILD_NAME_REQ: ������ ���� ����" )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( kPacket_.m_iItemUID )
					<< END_LOG;

				kPacket.m_iOK = NetError::ERR_ITEM_18;
				goto end_proc;
			}
		}
	}

	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;

	// ���̸� ����
	DO_QUERY( L"exec dbo.P_GPet_Info_UPT", L"%d, N\'%s\'", 
		% kPacket_.m_kPetInfo.m_iPetUID
		% kPacket_.m_kPetInfo.m_wstrPetName
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );

		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"�� �̸� ���� ����! ( ������ ���� �̽� �߻� )" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOGc( kPacket_.m_kPetInfo.m_iPetUID )
#ifdef SERV_PETID_DATA_TYPE_CHANGE //2013.07.02
			<< BUILD_LOGc( kPacket_.m_kPetInfo.m_iPetID )
#else //SERV_PETID_DATA_TYPE_CHANGE
			<< BUILD_LOGc( kPacket_.m_kPetInfo.m_cPetID )
#endif //SERV_PETID_DATA_TYPE_CHANGE
			<< BUILD_LOG( kPacket_.m_kPetInfo.m_wstrPetName )
			<< BUILD_LOGc( kPacket_.m_kPetInfo.m_cEvolutionStep )
			<< BUILD_LOG( kPacket_.m_kPetInfo.m_sSatiety )
			<< BUILD_LOG( kPacket_.m_kPetInfo.m_iIntimacy )
			<< BUILD_LOG( kPacket_.m_kPetInfo.m_sExtroversion )
			<< BUILD_LOG( kPacket_.m_kPetInfo.m_sEmotion )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_PET_19;
		goto end_proc;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_CHANGE_PET_NAME_ACK, kPacket );
}
#endif SERV_PET_CHANGE_NAME
//}}

#endif SERV_PET_SYSTEM
//}}

//{{ 2010. 8. 16	������	�Ⱓ ���� ������ �̺�Ʈ
#ifdef SERV_RESET_PERIOD_EVENT

IMPL_ON_FUNC( DBE_RESET_PERIOD_ITEM_REQ )
{
	KDBE_RESET_PERIOD_ITEM_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	std::map< UidType, short >::const_iterator mit;
	for( mit = kPacket_.m_mapExpandPeriodItemList.begin(); mit != kPacket_.m_mapExpandPeriodItemList.end(); ++mit )
	{
		KItemPeriodInfo kInfo;

		DO_QUERY( L"exec dbo.gup_update_item_period", L"%d, %d, %d", % kPacket_.m_iUnitUID % mit->first % mit->second );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK
				>> kInfo.m_wstrExpirationDate );

			m_kODBC.EndFetch();
		}

		if( kPacket.m_iOK == NetError::NET_OK )
		{
			kInfo.m_iItemUID = mit->first;
			kInfo.m_sPeriod = mit->second;
			kPacket.m_vecItemPeriodInfo.push_back( kInfo );
		}
		else
		{
			START_LOG( cerr, L"�Ⱓ �ʱ�ȭ ������ DB������Ʈ ����!" )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( mit->first )
				<< BUILD_LOG( mit->second )
				<< END_LOG;
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_RESET_PERIOD_ITEM_ACK, kPacket );
}

#endif SERV_RESET_PERIOD_EVENT
//}}

//{{ 2011. 01. 04	������	�Ӽ� ����
#ifdef SERV_ATTRIBUTE_CHARM

IMPL_ON_FUNC( DBE_ATTRIB_ATTACH_ITEM_REQ )
{
	KDBE_ATTRIB_ATTACH_ITEM_ACK kPacket;
	kPacket.m_iItemUID					= kPacket_.m_iItemUID;
	kPacket.m_kAttribEnchantInfo		= kPacket_.m_kAttribEnchantInfo;
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	
	{
		// ù��° ����
		DO_QUERY( L"exec dbo.gup_update_Attribute", L"%d, %d, %d, %d",
			% kPacket_.m_iUnitUID
			% kPacket_.m_iItemUID
			% CXSLAttribEnchantItem::ESI_SLOT_1
			% static_cast<int>(kPacket_.m_kAttribEnchantInfo.m_cAttribEnchant0)
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"0�� ���� �Ӽ� �ο� ������Ʈ ����." )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( kPacket_.m_iItemUID )
				<< BUILD_LOGc( kPacket_.m_kAttribEnchantInfo.m_cAttribEnchant0 )
				<< BUILD_LOGc( kPacket_.m_kAttribEnchantInfo.m_cAttribEnchant1 )
				<< BUILD_LOGc( kPacket_.m_kAttribEnchantInfo.m_cAttribEnchant2 )
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_UNKNOWN;
		}
	}

	{
		// �ι�° ����
		DO_QUERY( L"exec dbo.gup_update_Attribute", L"%d, %d, %d, %d",
			% kPacket_.m_iUnitUID
			% kPacket_.m_iItemUID
			% CXSLAttribEnchantItem::ESI_SLOT_2
			% static_cast<int>(kPacket_.m_kAttribEnchantInfo.m_cAttribEnchant1)
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"1�� ���� �Ӽ� �ο� ������Ʈ ����." )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( kPacket_.m_iItemUID )
				<< BUILD_LOGc( kPacket_.m_kAttribEnchantInfo.m_cAttribEnchant0 )
				<< BUILD_LOGc( kPacket_.m_kAttribEnchantInfo.m_cAttribEnchant1 )
				<< BUILD_LOGc( kPacket_.m_kAttribEnchantInfo.m_cAttribEnchant2 )
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_UNKNOWN;
		}
	}

	{
		// ����° ����
		DO_QUERY( L"exec dbo.gup_update_Attribute", L"%d, %d, %d, %d",
			% kPacket_.m_iUnitUID
			% kPacket_.m_iItemUID
			% CXSLAttribEnchantItem::ESI_SLOT_3
			% static_cast<int>(kPacket_.m_kAttribEnchantInfo.m_cAttribEnchant2)
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"2�� ���� �Ӽ� �ο� ������Ʈ ����." )
				<< BUILD_LOG( kPacket.m_iOK )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( kPacket_.m_iItemUID )
				<< BUILD_LOGc( kPacket_.m_kAttribEnchantInfo.m_cAttribEnchant0 )
				<< BUILD_LOGc( kPacket_.m_kAttribEnchantInfo.m_cAttribEnchant1 )
				<< BUILD_LOGc( kPacket_.m_kAttribEnchantInfo.m_cAttribEnchant2 )
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_UNKNOWN;
		}
	}

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_ATTRIB_ATTACH_ITEM_ACK, kPacket );
}

#endif SERV_ATTRIBUTE_CHARM
//}}

//{{ 2011. 04. 14	������	�븮 ����
#ifdef SERV_PSHOP_AGENCY

IMPL_ON_FUNC( DBE_INSERT_PERIOD_PSHOP_AGENCY_REQ )
{
	KDBE_INSERT_PERIOD_PSHOP_AGENCY_ACK kPacket;
	kPacket.m_usEventID = kPacket_.m_usEventID;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	// �븮 ���� ����
	DO_QUERY( L"exec dbo.gup_insert_PShopinfo", L"%d, %d, %d", 
		% LAST_SENDER_UID
		% kPacket_.m_iUnitUID
		% kPacket_.m_sAgencyPeriod
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK
			>> kPacket.m_wstrAgencyExpirationDate
			);

		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"�븮 ���� �Ⱓ ���� ����!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_sAgencyPeriod )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_PERSONAL_SHOP_43;
		goto end_proc;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_INSERT_PERIOD_PSHOP_AGENCY_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_PREPARE_REG_PSHOP_AGENCY_ITEM_REQ )
{
	KDBE_PREPARE_REG_PSHOP_AGENCY_ITEM_ACK kPacket;
	kPacket.m_iOK = NetError::NET_OK;
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_wstrPersonalShopName		= kPacket_.m_wstrPersonalShopName;
	kPacket.m_vecSellItemInfo			= kPacket_.m_vecSellItemInfo;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	SendToUser( LAST_SENDER_UID, DBE_PREPARE_REG_PSHOP_AGENCY_ITEM_ACK, kPacket );
}

//{{ 2012. 05. 31	��μ�       �븮 ���� �ŷ� ���� ����
#ifdef SERV_TRADE_LOGIC_CHANGE_AGENCY_SHOP
IMPL_ON_FUNC( DBE_INSERT_TO_INVENTORY_PICK_UP_FROM_PSHOP_AGENCY_REQ )
{
	KDBE_INSERT_TO_INVENTORY_PICK_UP_FROM_PSHOP_AGENCY_ACK kPacket;
	kPacket.m_iOK = NetError::NET_OK;
	kPacket.m_mapInsertItem				= kPacket_.m_mapInsertItem;
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_vecSellItemInfo			= kPacket_.m_vecSellItemInfo;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );

	// 	START_LOG( cerr, L"���� ���� ȣ�� �ϳ�" )
	// 		<< BUILD_LOG( kPacket_.m_iUnitUID )
	// 		<< END_LOG;

	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
	if( Query_InsertItemList( SEnum::GIR_PICK_UP_PSHOP, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#else
	if( Query_InsertItemList( kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
		//}}
	{
		START_LOG( cerr, L"������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;
	}
	//}}

	SendToUser( LAST_SENDER_UID, DBE_INSERT_TO_INVENTORY_PICK_UP_FROM_PSHOP_AGENCY_ACK, kPacket );
}
#else
IMPL_ON_FUNC( DBE_INSERT_TO_INVENTORY_PICK_UP_FROM_PSHOP_AGENCY_REQ )
{
	KDBE_INSERT_TO_INVENTORY_PICK_UP_FROM_PSHOP_AGENCY_ACK kPacket;
	kPacket.m_mapInsertItem				= kPacket_.m_mapInsertItem;
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_vecSellItemInfo			= kPacket_.m_vecSellItemInfo;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
	if( Query_InsertItemList( SEnum::GIR_PICK_UP_PSHOP, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo, false ) == false )
#else
	if( Query_InsertItemList( kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo, false ) == false )
#endif SERV_GET_ITEM_REASON
	//}}
	{
		START_LOG( cerr, L"�븮 ���� Pick Up ������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_PERSONAL_SHOP_47;
	}
	else
	{
		kPacket.m_iOK = NetError::NET_OK;
	}

	SendToUser( LAST_SENDER_UID, DBE_INSERT_TO_INVENTORY_PICK_UP_FROM_PSHOP_AGENCY_ACK, kPacket );
}
#endif SERV_TRADE_LOGIC_CHANGE_AGENCY_SHOP
//}}

#endif SERV_PSHOP_AGENCY
//}}


#ifdef	SERV_SHARING_BANK_TEST

_IMPL_ON_FUNC( DBE_GET_SHARE_BANK_REQ, KEGS_GET_SHARE_BANK_REQ )
{
	KEGS_GET_SHARE_BANK_ACK kAck;
	kAck.m_iOK = NetError::ERR_ODBC_01;

#ifdef SERV_NEW_UNIT_TRADE_LIMIT
			kAck.m_wstrNickName = kPacket_.m_wstrNickName;
#else // SERV_NEW_UNIT_TRADE_LIMIT
		kAck.m_wstrNickName = kPacket_;
#endif // SERV_NEW_UNIT_TRADE_LIMIT

	//	UnitUID ���
	DO_QUERY( L"exec dbo.gup_get_unit_uid", L"N\'%s\'", % kAck.m_wstrNickName );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kAck.m_iUnitUID );
		m_kODBC.EndFetch();
	}

	if( kAck.m_iUnitUID != 0 )
	{
		kAck.m_iOK = NetError::NET_OK;
	}
	else
	{
		kAck.m_iOK = NetError::ERR_KNM_02;
		goto end_proc;
	}

#ifdef SERV_NEW_UNIT_TRADE_LIMIT
	if( kPacket_.m_iNewUnitTradeBlockDay != 0 )
	{
		std::wstring strUnitCreateDate;

		DO_QUERY( L"exec dbo.gup_get_select_unit", L"N\'%d\'", % kAck.m_iUnitUID );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kAck.m_iOK 
				>> strUnitCreateDate
				);
			m_kODBC.EndFetch();
		}

		if( kAck.m_iOK != NetError::NET_OK )
		{
			kAck.m_iOK = NetError::ERR_KNM_02;
			goto end_proc;
		}

		CTime tReleaseTradeBlockTime;
		LIF( KncUtil::ConvertStringToCTime( strUnitCreateDate, tReleaseTradeBlockTime ) );
		tReleaseTradeBlockTime += CTimeSpan( kPacket_.m_iNewUnitTradeBlockDay, 0, 0, 0 );
		CTime tCurTime	 = CTime::GetCurrentTime();

		if( tCurTime < tReleaseTradeBlockTime  )
		{
			kAck.m_iOK = NetError::ERR_TRADE_BLOCK_UNIT_00;
			goto end_proc;
		}
	}

	if( kPacket_.m_iNewUnitTradeBlockUnitClass != 0 )
	{
		int iClass;
		DO_QUERY( L"exec dbo.gup_get_unitclass_by_unituid", L"%d", % kAck.m_iUnitUID );
		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iClass );
			m_kODBC.EndFetch();
		}
		else
		{
			kAck.m_iOK = NetError::ERR_ADMIN_COMMAND_01;
			goto end_proc;
		}

		if( iClass < kPacket_.m_iNewUnitTradeBlockUnitClass )
		{
			kAck.m_iOK = NetError::ERR_TRADE_BLOCK_UNIT_00;
			goto end_proc;
		}
	}
#endif SERV_NEW_UNIT_TRADE_LIMIT

	//	���� ũ�� ���
	DO_QUERY( L"exec dbo.P_GItemInventorySize_SEL_Bank", L"%d", % kAck.m_iUnitUID );

	int iBankSize = 0;
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iBankSize );
		m_kODBC.EndFetch();
	}

	kAck.m_iBankSize = iBankSize + CXSLInventory::MPB_NORMAL;

	if( kAck.m_iBankSize <= 0 )
	{
		kAck.m_iOK = NetError::ERR_TRADE_09;
		goto end_proc;
	}

	//	��������� ������
#ifdef SERV_SHARE_BANK_ITEM_EVALUATE_FIX // �輮�� // ���볯¥ : 2013-08-28
	DO_QUERY( L"exec dbo.P_GItem_GET_BankItem_New", L"%d", % kAck.m_iUnitUID );
#else //SERV_SHARE_BANK_ITEM_EVALUATE_FIX
	DO_QUERY( L"exec dbo.P_GItem_GET_BankItem", L"%d", % kAck.m_iUnitUID );
#endif ////SERV_SHARE_BANK_ITEM_EVALUATE_FIX
	while( m_kODBC.Fetch() )
	{
		int iEnchantLevel = 0;
#ifdef	SERV_ITEM_OPTION_DATA_SIZE
		int arrSocketOption[4] = {0,0,0,0};
#else	
		short arrSocketOption[4] = {0,0,0,0};
#endif	SERV_ITEM_OPTION_DATA_SIZE
		KInventoryItemInfo kInventoryItemInfo;


#ifdef SERV_SHARE_BANK_ITEM_EVALUATE_FIX // �輮�� // ���볯¥ : 2013-08-28
		int arrRandomSocketOption[5] = {0};

		FETCH_DATA( kInventoryItemInfo.m_iItemUID
			>> kInventoryItemInfo.m_kItemInfo.m_iItemID
			>> kInventoryItemInfo.m_kItemInfo.m_cUsageType
			>> kInventoryItemInfo.m_kItemInfo.m_iQuantity
			>> kInventoryItemInfo.m_kItemInfo.m_sEndurance
			>> kInventoryItemInfo.m_kItemInfo.m_sPeriod
			>> kInventoryItemInfo.m_kItemInfo.m_wstrExpirationDate
			>> iEnchantLevel
			>> arrSocketOption[0]
			>> arrSocketOption[1]
			>> arrSocketOption[2]
			>> arrSocketOption[3]
			>> arrRandomSocketOption[0]
			>> arrRandomSocketOption[1]
			>> arrRandomSocketOption[2]
			>> arrRandomSocketOption[3]
			>> arrRandomSocketOption[4]
			>> kInventoryItemInfo.m_cSlotCategory
			>> kInventoryItemInfo.m_sSlotID );


			int iCheckRandomIdx;
			for( iCheckRandomIdx = 4; iCheckRandomIdx >= 0; --iCheckRandomIdx )
			{
				if( arrRandomSocketOption[iCheckRandomIdx] != 0 )
					break;
			}

			for( int iIdx = 0; iIdx <= iCheckRandomIdx; ++iIdx )
			{
				kInventoryItemInfo.m_kItemInfo.m_vecRandomSocket.push_back( arrRandomSocketOption[iIdx] );
			}

#else //SERV_SHARE_BANK_ITEM_EVALUATE_FIX

		FETCH_DATA( kInventoryItemInfo.m_iItemUID
			>> kInventoryItemInfo.m_kItemInfo.m_iItemID
			>> kInventoryItemInfo.m_kItemInfo.m_cUsageType
			>> kInventoryItemInfo.m_kItemInfo.m_iQuantity
			>> kInventoryItemInfo.m_kItemInfo.m_sEndurance
			>> kInventoryItemInfo.m_kItemInfo.m_sPeriod
			>> kInventoryItemInfo.m_kItemInfo.m_wstrExpirationDate
			>> iEnchantLevel
			>> arrSocketOption[0]
			>> arrSocketOption[1]
			>> arrSocketOption[2]
			>> arrSocketOption[3]
			>> kInventoryItemInfo.m_cSlotCategory
			>> kInventoryItemInfo.m_sSlotID );

			/*
			START_LOG(cout2, L"����α�:Ÿ�� �� Ȯ�� �غ��ô�.")
				<< BUILD_LOG(kInventoryItemInfo.m_iItemUID)
				<< BUILD_LOG(kInventoryItemInfo.m_kItemInfo.m_iItemID)
				<< BUILD_LOG(kInventoryItemInfo.m_kItemInfo.m_iQuantity)
				<< BUILD_LOG(kInventoryItemInfo.m_kItemInfo.m_sEndurance)
				<< BUILD_LOG(kInventoryItemInfo.m_kItemInfo.m_sPeriod)
				<< BUILD_LOGc( kInventoryItemInfo.m_kItemInfo.m_cUsageType )
				<< END_LOG;
				*/
#endif //SERV_SHARE_BANK_ITEM_EVALUATE_FIX

			// 1. ��ȭ ���� ������Ʈ
			kInventoryItemInfo.m_kItemInfo.m_cEnchantLevel = static_cast<char>(iEnchantLevel);

			// 2. ���� ���� ������Ʈ
			int iCheckIdx;
			for( iCheckIdx = 3; iCheckIdx >= 0; --iCheckIdx )
			{
				if( arrSocketOption[iCheckIdx] != 0 )
					break;
			}

			for( int iIdx = 0; iIdx <= iCheckIdx; ++iIdx )
			{
				kInventoryItemInfo.m_kItemInfo.m_vecItemSocket.push_back( arrSocketOption[iIdx] );
			}

			if( kInventoryItemInfo.m_cSlotCategory != CXSLInventory::ST_BANK )
			{
				START_LOG( cerr, L"���� ���� : ���� ī�װ����� �ƴѵ� ����" )
					<< BUILD_LOG( kAck.m_iUnitUID )
					<< BUILD_LOG( kInventoryItemInfo.m_iItemUID )
					<< BUILD_LOG( kInventoryItemInfo.m_cSlotCategory )
					<< END_LOG;

				continue;
			}
			// Type�� ������������ �ٲ��ش�
			kInventoryItemInfo.m_cSlotCategory = CXSLInventory::ST_SHARE_BANK;

			// ������ UID�� PK�̹Ƿ� map�� insert �� �� Ű �ߺ� ���� �˻縦 �� �ʿ䰡 ����.
			kAck.m_mapItem.insert( std::make_pair( kInventoryItemInfo.m_iItemUID, kInventoryItemInfo ) );
	}

	//	Attribute ���� ������
	DO_QUERY( L"exec dbo.P_GItemAttribute_SEL_BankItem", L"%d", % kAck.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		UidType iItemUID = 0;
		int iAttribEnchantSlotNo = 0;
		int iAttribEnchantID = 0;
		std::map< UidType, KInventoryItemInfo >::iterator mitItem;

		FETCH_DATA( iItemUID
			>> iAttribEnchantSlotNo
			>> iAttribEnchantID );


		mitItem = kAck.m_mapItem.find( iItemUID );
		if( mitItem != kAck.m_mapItem.end() )
		{
			switch( iAttribEnchantSlotNo )
			{
			case CXSLAttribEnchantItem::ESI_SLOT_1:
				mitItem->second.m_kItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant0 = static_cast<char>( iAttribEnchantID );
				break;

			case CXSLAttribEnchantItem::ESI_SLOT_2:
				mitItem->second.m_kItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant1 = static_cast<char>( iAttribEnchantID );
				break;

			case CXSLAttribEnchantItem::ESI_SLOT_3:
				mitItem->second.m_kItemInfo.m_kAttribEnchantInfo.m_cAttribEnchant2 = static_cast<char>( iAttribEnchantID );
				break;

			default:
				START_LOG( cerr, L"���� ���� : �Ӽ� ��ȭ ���� �ѹ��� �̻��մϴ�." )
					<< BUILD_LOG( kAck.m_iUnitUID )
					<< BUILD_LOG( iItemUID )
					<< BUILD_LOG( iAttribEnchantSlotNo )
					<< BUILD_LOG( iAttribEnchantID )
					<< END_LOG;
				break;
			}
		}
		else
		{
			START_LOG( cerr, L"���� ���� : �Ӽ������� �ִµ� �������� ����?" )
				<< BUILD_LOG( kAck.m_iUnitUID )
				<< BUILD_LOG( iItemUID )
				<< BUILD_LOG( iAttribEnchantSlotNo )
				<< BUILD_LOG( iAttribEnchantID )
				<< END_LOG;
		}
	}

	//	���� ���� ������
	DO_QUERY( L"exec dbo.P_GItemSeal_SEL_BankItem", L"%d", % kAck.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		UidType iItemUID = 0;
		int iSealCnt = 0;
		std::map< UidType, KInventoryItemInfo >::iterator mitItem;

		FETCH_DATA( iItemUID
			>> iSealCnt );

		mitItem = kAck.m_mapItem.find( iItemUID );
		if( mitItem != kAck.m_mapItem.end() )
		{
			mitItem->second.m_kItemInfo.m_ucSealData = iSealCnt;
		}
		else
		{
			START_LOG( cerr, L"���� ���� : ���������� �ִµ� �������� ����?" )
				<< BUILD_LOG( kAck.m_iUnitUID )
				<< BUILD_LOG( iItemUID )
				<< BUILD_LOG( iSealCnt )
				<< END_LOG;
		}
	}

end_proc:

	SendToUser( LAST_SENDER_UID, DBE_GET_SHARE_BANK_ACK, kAck );
}

IMPL_ON_FUNC( DBE_UPDATE_SHARE_ITEM_REQ )
{
	KDBE_UPDATE_SHARE_ITEM_ACK kAck;

	if( Query_UpdateShareItem( kPacket_, kAck ) == false )
	{
		START_LOG( cerr, L"���� ���� ������ ���� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;
	}
	
	SendToUser( LAST_SENDER_UID, DBE_UPDATE_SHARE_ITEM_ACK, kAck );
}

_IMPL_ON_FUNC( DBE_UPDATE_SHARE_ITEM_FOR_MOVE_SLOT_REQ, KDBE_UPDATE_SHARE_ITEM_REQ )
{
	KDBE_UPDATE_SHARE_ITEM_ACK kAck;

	if( Query_UpdateShareItem( kPacket_, kAck ) == false )
	{
		START_LOG( cerr, L"���� ���� ������ ���� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;
	}

	SendToUser( LAST_SENDER_UID, DBE_UPDATE_SHARE_ITEM_FOR_MOVE_SLOT_ACK, kAck );
}

_IMPL_ON_FUNC( DBE_UPDATE_SEAL_DATA_NOT, KDBE_SEAL_ITEM_REQ )
{
	KDBE_SEAL_ITEM_ACK kPacket;
	kPacket.m_ucSealResult			  = kPacket_.m_ucSealResult;	
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec dbo.gup_update_item_seal", L"%d, %d", % kPacket_.m_iItemUID % kPacket_.m_ucSealResult );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"���� DB ������Ʈ ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iItemUID )
			<< BUILD_LOGc( kPacket_.m_ucSealResult )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_SEAL_ITEM_16;
		goto end_proc;
	}

end_proc:
	return;
}

IMPL_ON_FUNC( DBE_CHANGE_INVENTORY_SLOT_ITEM_REQ )
{
	KDBE_CHANGE_INVENTORY_SLOT_ITEM_ACK kPacket;
	kPacket.m_cFromSlotType = kPacket_.m_cFromSlotType;
	kPacket.m_iFromSlotID = kPacket_.m_iFromSlotID;
	kPacket.m_cToSlotType = kPacket_.m_cToSlotType;
	kPacket.m_iToSlotID = kPacket_.m_iToSlotID;
	kPacket.m_kItemQuantityUpdate = kPacket_.m_kItemQuantityUpdate;
	kPacket.m_kItemEnduranceUpdate = kPacket_.m_kItemEnduranceUpdate;
	kPacket.m_kItemPositionUpdate = kPacket_.m_kItemPositionUpdate;

	//////////////////////////////////////////////////////////////////////////
	//START_LOG( cout, L"[�׽�Ʈ�α�] ���� ���� ���� �����ϱ� ���� �̸� DB������Ʈ ������ ó������!" )		
	//	<< BUILD_LOG( kPacket_.m_iUnitUID )
	//	<< BUILD_LOG( kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.size() )
	//	<< BUILD_LOG( kPacket_.m_kItemEnduranceUpdate.size() )
	//	<< BUILD_LOG( kPacket_.m_kItemPositionUpdate.size() )
	//	<< BUILD_LOG( kPacket_.m_kItemQuantityUpdate.m_vecDeleted.size() );
	//////////////////////////////////////////////////////////////////////////

	// ���� ���� ���� �̵��� �ϱ� ���� �̸� �κ��丮�� ��� ���� ������ DB�� ������!
	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	LIF( Query_UpdateItemEndurance( kPacket_.m_iUnitUID, kPacket_.m_kItemEnduranceUpdate, kPacket.m_kItemEnduranceUpdate ) );
	Query_UpdateItemPosition( kPacket_.m_iUnitUID, kPacket_.m_kItemPositionUpdate, kPacket.m_kItemPositionUpdate );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	SendToUser( LAST_SENDER_UID, DBE_CHANGE_INVENTORY_SLOT_ITEM_ACK, kPacket );
}

#endif	SERV_SHARING_BANK_TEST

#ifdef GIANT_RESURRECTION_CASHSTONE
IMPL_ON_FUNC( DBE_REALTIME_RESURRECTION_CASHSTONE_NOT )
{

	int idumyData;
	Query_UpdateResurrection_stone(kPacket_.m_iUnitUID, kPacket_.m_iNumResurrection_CashStone, idumyData );


	//end_proc:
	return;
}
IMPL_ON_FUNC( DBE_RESURRECTION_CASHSTONE_NOT )
{


	int iRet = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec dbo.gup_update_resurrection_CashStone", L"%d, %d", 
											% kPacket_.m_iUnitUID 
											% kPacket_.m_iNumResurrection_CashStone );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iRet );

		m_kODBC.EndFetch();
	}
	else
	{
		goto end_proc;
	}


end_proc:
	LOG_SUCCESS( iRet == NetError::NET_OK )
		<< BUILD_LOG( iRet )
		<< BUILD_LOG( kPacket_.m_iUnitUID  )
		<< BUILD_LOG( kPacket_.m_iNumResurrection_CashStone  )
		<< END_LOG;

	return;

}
#endif //GIANT_RESURRECTION_CASHSTONE


#ifdef SERV_SHARING_BANK_EVENT
IMPL_ON_FUNC( DBE_SHARING_BANK_EVENT_REQ )
{
	int iOK = NetError::ERR_ODBC_01;
	// sp�� ȣ���ؼ� �̺�Ʈ ������ �ֵ��� �մϴ�~
	DO_QUERY( L"exec dbo.P_GBankShare_GET_Event", L"%d, %d, %d", % kPacket_.m_iUserUID % kPacket_.m_iUnitUID % kPacket_.m_iItemID);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}

	if( iOK != NetError::NET_OK )
	{
		START_LOG( clog2, L"�������� �̺�Ʈ SP ����!" )
			<< BUILD_LOG( iOK )
			<< BUILD_LOG( kPacket_.m_iUserUID )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iItemID )
			<< END_LOG;
	}
	else
	{
		START_LOG( clog2, L"����α�:�������� �̺�Ʈ SP ����!" )
			<< BUILD_LOG( iOK )
			<< BUILD_LOG( kPacket_.m_iUserUID )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iItemID )
			<< END_LOG;

		SendToUser(LAST_SENDER_UID, DBE_SHARING_BANK_EVENT_ACK);
	}



end_proc:
	return;
}
#endif

//{{ ���� - ���� ���� ����Ʈ, ĳ�� �۾� 
#ifdef SERV_SHARING_BANK_QUEST_CASH
IMPL_ON_FUNC( DBE_SHARING_BACK_OPEN_REQ )
{
	KDBE_SHARING_BACK_OPEN_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	// ���� ���� ���� �ߴٰ� ��� �˸���
	// �ش� sp�� ȣ��

	DO_QUERY( L"exec dbo.P_GBankShare_INS", L"%d, %d, %d", % kPacket_.m_iUserUID % kPacket_.m_iUnitUID % kPacket_.m_iOpenType);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"�������� �ձ� SP ����!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUserUID )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( static_cast<int>(kPacket_.m_iOpenType) )
			<< END_LOG;
	}

	kPacket.m_iOpenType = kPacket_.m_iOpenType;
	SendToUser( LAST_SENDER_UID, DBE_SHARING_BACK_OPEN_ACK, kPacket );
end_proc:
	return;
}
#endif SERV_SHARING_BANK_QUEST_CASH
//}}

#ifdef SERV_ADVERTISEMENT_EVENT
IMPL_ON_FUNC( DBE_INSERT_ADVERTISEMENT_EVENT_INFO_NOT )
{
	int iOK = NetError::ERR_ODBC_01;

	BOOST_TEST_FOREACH( const int&, iEventUID, kPacket_.m_vecCompletedEvent )
	{
		DO_QUERY( L"exec dbo.P_GAdvertisementEvent_INT", L"%d, %d",
			% kPacket_.m_iUserUID
			% iEventUID
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"�ǽð� ���� �̺�Ʈ ���� ��� ����" )
				<< BUILD_LOG( iOK )
				<< END_LOG;
		}
	}
end_proc:
	return;
}
#endif SERV_ADVERTISEMENT_EVENT

//{{ 2011. 08. 03	������	���� ���� ���ῡ ���� ����ó��
#ifdef SERV_CLIENT_QUIT_PVP_BUG_PLAY_FIX
IMPL_ON_FUNC( DBE_QUIT_USER_PVP_RESULT_UPDATE_NOT )
{
	LIF( Query_UpdatePvpInfo( kPacket_ ) );
}
#endif SERV_CLIENT_QUIT_PVP_BUG_PLAY_FIX
//}}

//{{ 2011. 06. 22    ��μ�    ��Ż ���� �� - NEXON ���� �ý��� ����
#ifdef SERV_NEXON_SESSION_PROTOCOL
IMPL_ON_FUNC( DBE_RETAINING_SELECT_REWARD_REQ )
{
	KDBE_RETAINING_SELECT_REWARD_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;	
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;	

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	
	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cerr, L"��Ż���� ���� ���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	if( Query_InsertItemList( SEnum::GIR_RETAINING_ITEM, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
	{
		START_LOG( cerr, L"��Ż���� ���� ������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_ITEM_06;
	}
	else
	{
		kPacket.m_iOK = NetError::NET_OK;
	}

	SendToUser( LAST_SENDER_UID, DBE_RETAINING_SELECT_REWARD_ACK, kPacket );
}
#endif SERV_NEXON_SESSION_PROTOCOL
//}} 

//{{ 2011. 10. 14	������	������ ��� DB ������Ʈ ����
#ifdef SERV_USE_ITEM_DB_UPDATE_FIX
IMPL_ON_FUNC( DBE_USE_ITEM_IN_INVENTORY_REQ )
{
	KDBE_USE_ITEM_IN_INVENTORY_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iUsedItemID = kPacket_.m_iUsedItemID;
#ifdef SERV_GOLD_TICKET
	kPacket.m_iItemUID = kPacket_.m_iItemUID;
#endif //SERV_GOLD_TICKET
	kPacket.m_iWarpPointMapID = kPacket_.m_iWarpPointMapID;
	//{{ 2012. 10. 31	�ڼ���	�ں� �ͽ������� Ƽ�� �߰�
#ifdef SERV_ADD_COBO_EXPRESS_TICKET
	kPacket.m_iED	= kPacket_.m_iED;
#endif SERV_ADD_COBO_EXPRESS_TICKET
	//}}
	//{{ 2012. 12. 24	�ڼ���	�� ���� ���� ��� �߰�
//#ifdef SERV_PET_AUTO_LOOTING
	kPacket.m_iTempCode	= kPacket_.m_iTempCode;
//#endif SERV_PET_AUTO_LOOTING
	//}}

	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	SendToUser( LAST_SENDER_UID, DBE_USE_ITEM_IN_INVENTORY_ACK, kPacket );
}
#endif SERV_USE_ITEM_DB_UPDATE_FIX
//}}

//{{ 2011. 11. 21  ��μ�	���� ���� ������
#ifdef SERV_UNIT_CLASS_CHANGE_ITEM
IMPL_ON_FUNC( DBE_BUY_UNIT_CLASS_CHANGE_REQ )
{
	kPacket_.m_iOK = NetError::ERR_ODBC_00;

	std::map< int, int >::const_iterator mitSkill = kPacket_.m_mapChangeSkill.begin();
	std::map< int, int >::const_iterator mitMemo = kPacket_.m_mapChangeMemo.begin();
	std::map< UidType, int >::const_iterator mitItem = kPacket_.m_mapChangeItem.begin();
	std::map< int, int >::const_iterator mitCompleteQuest = kPacket_.m_mapChangeCompleteQuest.begin();
	std::map< int, int >::const_iterator mitInProgressQuest = kPacket_.m_mapChangeInProgressQuest.begin();

	// Ŭ���� ����
	DO_QUERY( L"exec dbo.P_GUnit_UPT_UnitClassChange", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iNewUnitClass ); // �ܼ� UnitClass �� ����
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket_.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket_.m_iOK != NetError::NET_OK )
	{
		switch( kPacket_.m_iOK )
		{
		case -1:
			{
				START_LOG( cerr, L"���� ���� ������ ����! UnitClass ���� ���� - �ش� ������ ���� ���" )
					<< BUILD_LOG( kPacket_.m_iOK )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( kPacket_.m_iNewUnitClass )
					<< END_LOG;
			}break;
		case -2:
			{
				START_LOG( cerr, L"���� ���� ������ ����! UnitClass ���� ���� - ������Ʈ ����" )
					<< BUILD_LOG( kPacket_.m_iOK )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( kPacket_.m_iNewUnitClass )
					<< END_LOG;
			}break;
		default:
			{
				START_LOG( cerr, L"���� ���� ������ ����! UnitClass ���� ����" )
					<< BUILD_LOG( kPacket_.m_iOK )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( kPacket_.m_iNewUnitClass )
					<< END_LOG;
			}break;
		}
	}

#ifdef SERV_UNLIMITED_SECOND_CHANGE_JOB
	// ������ ������� ���� �α� �����.
	if( kPacket_.m_iOK == NetError::NET_OK && kPacket_.m_bUnlimitedSecondJobItem == true )
	{
		KDBE_UNLIMITED_SECOND_CHANGE_JOB_NOT	kUSCjobNot;
		kUSCjobNot.m_iUnitUID			=	kPacket_.m_iUnitUID;
		kUSCjobNot.m_iOldUnitClass		=	kPacket_.m_iOldUnitClass;
		kUSCjobNot.m_iNewUnitClass		=	kPacket_.m_iNewUnitClass;

		SendToLogDB( DBE_UNLIMITED_SECOND_CHANGE_JOB_NOT, kUSCjobNot );
	}
#endif SERV_UNLIMITED_SECOND_CHANGE_JOB

	// ��ų �ʱ�ȭ
#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013 // ���볯¥: 2013-06-27
	DO_QUERY( L"exec dbo.P_GSkill_New_All_DEL_UnitClassChange", L"%d, %d, %d, %d, %d, %d, %d, %d, %d",
		% kPacket_.m_iUnitUID
		% kPacket_.m_iSPoint
		% kPacket_.m_iCSPoint
		% kPacket_.m_iDefaultSkillID1
		% kPacket_.m_iDefaultSkillID2
		% kPacket_.m_iDefaultSkillID3
		% kPacket_.m_iDefaultSkillID4
		% kPacket_.m_iDefaultSkillID5
		% kPacket_.m_iDefaultSkillID6
		);
#else	// SERV_UPGRADE_SKILL_SYSTEM_2013
/*
	DO_QUERY( L"exec dbo.P_GSkill_New_DEL_UnitClassChange", L"%d, %d, %d, %d, %d",
	% kPacket_.m_iUnitUID
	% kPacket_.m_iSPoint
	% kPacket_.m_iCSPoint
	% kPacket_.m_iDefaultSkillID
	% kPacket_.m_iDefaultSkillID2
	);
*/
#endif	// SERV_UPGRADE_SKILL_SYSTEM_2013

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket_.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket_.m_iOK != NetError::NET_OK )
	{
		switch( kPacket_.m_iOK )
		{
		case -1:
			{
				START_LOG( cerr, L"DB ��ų�ʱ�ȭ ����.! - ���� ��ų ����� ����" )
					<< BUILD_LOG( kPacket_.m_iOK )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( kPacket_.m_iSPoint )
					<< BUILD_LOG( kPacket_.m_iCSPoint )
#ifdef SERV_UPGRADE_SKILL_SYSTEM_2013 // ���볯¥: 2013-06-27
					<< BUILD_LOG( kPacket_.m_iDefaultSkillID1 )
					<< BUILD_LOG( kPacket_.m_iDefaultSkillID2 )
					<< BUILD_LOG( kPacket_.m_iDefaultSkillID3 )
					<< BUILD_LOG( kPacket_.m_iDefaultSkillID4 )
					<< BUILD_LOG( kPacket_.m_iDefaultSkillID5 )
					<< BUILD_LOG( kPacket_.m_iDefaultSkillID6 )
#else	// SERV_UPGRADE_SKILL_SYSTEM_2013
/*
					<< BUILD_LOG( kPacket_.m_iDefaultSkillID )
					<< BUILD_LOG( kPacket_.m_iDefaultSkillID2 )
*/
#endif	// SERV_UPGRADE_SKILL_SYSTEM_2013
					<< END_LOG;
			}break;
		case -2:
			{
				START_LOG( cerr, L"DB ��ų�ʱ�ȭ ����.!- ��ų ����Ʈ ���� ����" )
					<< BUILD_LOG( kPacket_.m_iOK )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( kPacket_.m_iSPoint )
					<< BUILD_LOG( kPacket_.m_iCSPoint )
					<< END_LOG;
			}break;
		case -3:
			{
				START_LOG( cerr, L"DB ��ų�ʱ�ȭ ����.!- ĳ�� ��ų ����Ʈ ���� ����" )
					<< BUILD_LOG( kPacket_.m_iOK )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( kPacket_.m_iSPoint )
					<< BUILD_LOG( kPacket_.m_iCSPoint )
					<< END_LOG;
			}break;
		case -4:
			{
				START_LOG( cerr, L"DB ��ų�ʱ�ȭ ����.!- ��ų ����A �ʱ�ȭ ����" )
					<< BUILD_LOG( kPacket_.m_iOK )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( kPacket_.m_iSPoint )
					<< BUILD_LOG( kPacket_.m_iCSPoint )
					<< END_LOG;
			}break;
		case -5:
			{
				START_LOG( cerr, L"DB ��ų�ʱ�ȭ ����.!- ��ų ����B �ʱ�ȭ ����" )
					<< BUILD_LOG( kPacket_.m_iOK )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( kPacket_.m_iSPoint )
					<< BUILD_LOG( kPacket_.m_iCSPoint )
					<< END_LOG;
			}break;
		case -6:
			{
				START_LOG( cerr, L"DB ��ų�ʱ�ȭ ����.! - �⺻ ��ų ���� 1�� ����" )
					<< BUILD_LOG( kPacket_.m_iOK )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( kPacket_.m_iSPoint )
					<< BUILD_LOG( kPacket_.m_iCSPoint )
					<< END_LOG;
			}break;
		default:
			{
				START_LOG( cerr, L"DB ��ų�ʱ�ȭ ����.!" )
					<< BUILD_LOG( kPacket_.m_iOK )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( kPacket_.m_iSPoint )
					<< BUILD_LOG( kPacket_.m_iCSPoint )
					<< END_LOG;
			}break;
		}
	}

	// ��ų ���� ����
	for( ; mitSkill != kPacket_.m_mapChangeSkill.end() ; ++mitSkill )
	{
		DO_QUERY( L"exec dbo.P_GSkill_Unsealed_UPT_UnitClassChange", L"%d, %d, %d", % kPacket_.m_iUnitUID % mitSkill->first % mitSkill->second );
		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket_.m_iOK );
			m_kODBC.EndFetch();
		}

		if( kPacket_.m_iOK != NetError::NET_OK )
		{
			switch( kPacket_.m_iOK )
			{
			case -1:
				{
					START_LOG( cerr, L"���� ���� ������ ����! Skill ���� ���� - �ش� ������ ���� ���" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( mitSkill->first )
						<< BUILD_LOG( mitSkill->second )
						<< END_LOG;
				}break;
			case -2:
				{
					START_LOG( cerr, L"���� ���� ������ ����! Skill ���� ���� - �ش� ĳ�� ��ų�� ������������ ���� ���" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( mitSkill->first )
						<< BUILD_LOG( mitSkill->second )
						<< END_LOG;
				}break;
			default:
				{
					START_LOG( cerr, L"���� ���� ������ ����! Skill ���� ����" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( mitSkill->first )
						<< BUILD_LOG( mitSkill->second )
						<< END_LOG;
				}break;
			}
		}
	}
	
	// ������ �޸� ���� ����
	for( ; mitMemo != kPacket_.m_mapChangeMemo.end() ; ++mitMemo )
	{
		DO_QUERY( L"exec dbo.P_GNote_UPT_UnitClassChange", L"%d, %d, %d", % kPacket_.m_iUnitUID % mitMemo->first % mitMemo->second );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket_.m_iOK );
			m_kODBC.EndFetch();
		}

		if( kPacket_.m_iOK != NetError::NET_OK )
		{
			switch( kPacket_.m_iOK )
			{
			case -1:
				{
					START_LOG( cerr, L"���� ���� ������ ����! Memo ���� ���� - �ش� ������ ���� ���" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( mitMemo->first )
						<< BUILD_LOG( mitMemo->second )
						<< END_LOG;
				}break;
			case -2:
				{
					START_LOG( cerr, L"���� ���� ������ ����! Memo ���� ���� - �ش� �޸� ����� ��Ʈ�� ��ϵ��� ���� ���" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( mitMemo->first )
						<< BUILD_LOG( mitMemo->second )
						<< END_LOG;
				}break;
			default:
				{
					START_LOG( cerr, L"���� ���� ������ ����! Memo ���� ����" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( mitMemo->first )
						<< BUILD_LOG( mitMemo->second )
						<< END_LOG;
				}break;
			}
		}
	}

	// �κ��丮 ĳ�� & ����Ʈ ������ ���� ����
	for( ; mitItem != kPacket_.m_mapChangeItem.end() ; ++mitItem )
	{
		DO_QUERY( L"exec dbo.P_GItem_UPT_UnitClassChange", L"%d, %d, %d", % kPacket_.m_iUnitUID % mitItem->first % mitItem->second );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket_.m_iOK );
			m_kODBC.EndFetch();
		}

		if( kPacket_.m_iOK != NetError::NET_OK )
		{
			switch( kPacket_.m_iOK )
			{
			case -1:
				{
					START_LOG( cerr, L"���� ���� ������ ����! Item ���� ���� - �ش� ������ ���� ���" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( mitItem->first )
						<< BUILD_LOG( mitItem->second )
						<< END_LOG;
				}break;
			case -2:
				{
					START_LOG( cerr, L"���� ���� ������ ����! Item ���� ���� - �ش� �������� �κ��丮�� ���� ���" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( mitItem->first )
						<< BUILD_LOG( mitItem->second )
						<< END_LOG;
				}break;
			default:
				{
					START_LOG( cerr, L"���� ���� ������ ����! Item ���� ����" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( mitItem->first )
						<< BUILD_LOG( mitItem->second )
						<< END_LOG;
				}break;
			}
		}
	}

	// ����Ʈ ����
	for( u_short unSize = 0 ; unSize < kPacket_.m_vecDeleteCompleteQuest.size() ; ++unSize )
	{
		DO_QUERY( L"exec dbo.P_GQuests_Complete_DEL_UnitClassChange", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_vecDeleteCompleteQuest[unSize]  );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket_.m_iOK );
			m_kODBC.EndFetch();
		}

		if( kPacket_.m_iOK != NetError::NET_OK )
		{
			switch( kPacket_.m_iOK )
			{
			case -1:
				{
					START_LOG( cerr, L"���� ���� Quest ���� ���� - �ش� ������ ���� ���" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( kPacket_.m_vecDeleteCompleteQuest[unSize] )
						<< END_LOG;
				}break;
			case -2:
				{
					START_LOG( cerr, L"���� ���� Quest ���� ���� - ����Ʈ�� �Ϸ���� ���� ���" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( kPacket_.m_vecDeleteCompleteQuest[unSize] )
						<< END_LOG;
				}break;
			default:
				{
					START_LOG( cerr, L"���� ���� Quest ���� ����" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( kPacket_.m_vecDeleteCompleteQuest[unSize] )
						<< END_LOG;
				}break;
			}
		}
	}

	// ����Ʈ ����
	for( u_short unSize = 0 ; unSize < kPacket_.m_vecDeleteInProgressQuest.size() ; ++unSize )
	{
		DO_QUERY( L"exec dbo.P_GQuests_DEL_UnitClassChange", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_vecDeleteInProgressQuest[unSize]  );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket_.m_iOK );
			m_kODBC.EndFetch();
		}

		if( kPacket_.m_iOK != NetError::NET_OK )
		{
			switch( kPacket_.m_iOK )
			{
			case -1:
				{
					START_LOG( cerr, L"���� ���� ���� �� Quest ���� ���� - �ش� ������ ���� ���" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( kPacket_.m_vecDeleteInProgressQuest[unSize] )
						<< END_LOG;
				}break;
			case -2:
				{
					START_LOG( cerr, L"���� ���� ���� �� Quest ���� ���� - ����Ʈ�� �Ϸ���� ���� ���" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( kPacket_.m_vecDeleteInProgressQuest[unSize] )
						<< END_LOG;
				}break;
			default:
				{
					START_LOG( cerr, L"���� ���� ���� �� Quest ���� ����" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( kPacket_.m_vecDeleteInProgressQuest[unSize] )
						<< END_LOG;
				}break;
			}
		}
	}

	// ����Ʈ ���� ����
	for( ; mitCompleteQuest != kPacket_.m_mapChangeCompleteQuest.end() ; ++mitCompleteQuest )
	{
		DO_QUERY( L"exec dbo.P_GQuests_Complete_UPT_UnitClassChange", L"%d, %d, %d", % kPacket_.m_iUnitUID % mitCompleteQuest->first % mitCompleteQuest->second );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket_.m_iOK );
			m_kODBC.EndFetch();
		}

		if( kPacket_.m_iOK != NetError::NET_OK )
		{
			switch( kPacket_.m_iOK )
			{
			case -1:
				{
					START_LOG( cerr, L"���� ���� ������ ����! �Ϸ� Quest ���� ���� - �ش� ������ ���� ���" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( mitCompleteQuest->first )
						<< BUILD_LOG( mitCompleteQuest->second )
						<< END_LOG;
				}break;
			case -2:
				{
					START_LOG( cerr, L"���� ���� ������ ����! �Ϸ� Quest ���� ���� - �ش� ����Ʈ�� �Ϸ���� ���� ���" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( mitCompleteQuest->first )
						<< BUILD_LOG( mitCompleteQuest->second )
						<< END_LOG;
				}break;
			default:
				{
					START_LOG( cerr, L"���� ���� ������ ����! �Ϸ� Quest ���� ����" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( mitCompleteQuest->first )
						<< BUILD_LOG( mitCompleteQuest->second )
						<< END_LOG;
				}break;
			}
		}
	}

	for( ; mitInProgressQuest != kPacket_.m_mapChangeInProgressQuest.end() ; ++mitInProgressQuest )
	{
		DO_QUERY( L"exec dbo.P_GQuests_UPT_UnitClassChange", L"%d, %d, %d", 
			% kPacket_.m_iUnitUID 
			% mitInProgressQuest->first 
			% mitInProgressQuest->second
			); //�Ϸ�� ���� �� ���� ����

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket_.m_iOK );
			m_kODBC.EndFetch();
		}

		if( kPacket_.m_iOK != NetError::NET_OK )
		{
			switch( kPacket_.m_iOK )
			{
			case -1:
				{
					START_LOG( cerr, L"���� ���� ������ ����! ���� Quest ���� ���� - �ش� ������ ���� ���" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( mitInProgressQuest->first )
						<< BUILD_LOG( mitInProgressQuest->second )
						<< END_LOG;
				}break;
			case -2:
				{
					START_LOG( cerr, L"���� ���� ������ ����! ���� Quest ���� ���� - �ش� ����Ʈ�� ���������� ���� ��� " )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( mitInProgressQuest->first )
						<< BUILD_LOG( mitInProgressQuest->second )
						<< END_LOG;
				}break;
			default:
				{
					START_LOG( cerr, L"���� ���� ������ ����! ���� Quest ���� ����" )
						<< BUILD_LOG( kPacket_.m_iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( mitInProgressQuest->first )
						<< BUILD_LOG( mitInProgressQuest->second )
						<< END_LOG;
				}break;
			}
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_BUY_UNIT_CLASS_CHANGE_ACK, kPacket_ );
}
#endif SERV_UNIT_CLASS_CHANGE_ITEM
//}}


//{{ 2012. 04. 30	�ڼ���	������ �ֹ��� ���� �̺�Ʈ ( ������ �ߺ� üũ )
#ifdef SERV_SCROLL_OF_SAGE_CHECK_THE_LETTER_BOX
_IMPL_ON_FUNC( DBE_UPDATE_EVENT_TIME_NOT, KDBE_UPDATE_EVENT_TIME_REQ )
{
	int iOK = NetError::ERR_ODBC_01;

	{
		//////////////////////////////////////////////////////////////////////////
		// ���� �ð� �̺�Ʈ

		std::vector< KConnectTimeEventInfo >::const_iterator vit;
		for( vit = kPacket_.m_vecConnectTimeEvent.begin(); vit != kPacket_.m_vecConnectTimeEvent.end(); ++vit )
		{
			//{{ 2010. 06. 11  ������	�������� ���ӽð� �̺�Ʈ
#ifdef SERV_ACC_TIME_EVENT
			if( vit->m_bAccountEvent )
			{
				DO_QUERY( L"exec dbo.gup_update_event_account_nor", L"%d, %d, N\'%s\'", % LAST_SENDER_UID % vit->m_iEventUID % vit->m_wstrEventTime );

				if( m_kODBC.BeginFetch() )
				{
					FETCH_DATA( iOK );
					m_kODBC.EndFetch();
				}

				if( iOK != NetError::NET_OK )
				{
					START_LOG( cerr, L"�̺�Ʈ Ÿ�� DB������Ʈ ����?" )
						<< BUILD_LOG( iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( vit->m_iEventUID )
						<< BUILD_LOG( vit->m_wstrEventTime )
						<< END_LOG;
				}
			}
			else
#endif SERV_ACC_TIME_EVENT
				//}}
			{
				DO_QUERY( L"exec dbo.gup_update_30min", L"%d, %d, N\'%s\'", % kPacket_.m_iUnitUID % vit->m_iEventUID % vit->m_wstrEventTime );

				if( m_kODBC.BeginFetch() )
				{
					FETCH_DATA( iOK );
					m_kODBC.EndFetch();
				}

				if( iOK != NetError::NET_OK )
				{
					START_LOG( cerr, L"�̺�Ʈ Ÿ�� DB������Ʈ ����?" )
						<< BUILD_LOG( iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( vit->m_iEventUID )
						<< BUILD_LOG( vit->m_wstrEventTime )
						<< END_LOG;
				}
			}			
		}

		//////////////////////////////////////////////////////////////////////////		
	}


	//{{ 2009. 12. 7  ������	�����ð��̺�Ʈ
#ifdef CUMULATIVE_TIME_EVENT

	{
		//////////////////////////////////////////////////////////////////////////
		// ���� �ð� �̺�Ʈ

		CTime tUpdateTime = CTime::GetCurrentTime();
		tUpdateTime += CTimeSpan( 18250, 0, 0, 0 ); // �ѹ��� ��� �ϹǷ� 50���� ���Ѵ�.
		std::wstring wstrUpdateDate = ( CStringW )( tUpdateTime.Format( _T( "%Y-%m-%d %H:%M:%S" ) ) );

		std::vector< KCumulativeTimeEventInfo >::const_iterator vitCT;
		for( vitCT = kPacket_.m_vecCumulativeTimeEvent.begin(); vitCT != kPacket_.m_vecCumulativeTimeEvent.end(); ++vitCT )
		{
			//{{ 2010. 06. 11  ������	�������� ���ӽð� �̺�Ʈ
#ifdef SERV_ACC_TIME_EVENT
			if( vitCT->m_bAccountEvent )
			{
				// �Ϸ� ������ ������Ʈ ����!
				DO_QUERY( L"exec dbo.gup_update_event_account_nor", L"%d, %d, N\'%s\'", % LAST_SENDER_UID % vitCT->m_iEventUID % wstrUpdateDate );

				if( m_kODBC.BeginFetch() )
				{
					FETCH_DATA( iOK );
					m_kODBC.EndFetch();
				}

				if( iOK != NetError::NET_OK )
				{
					START_LOG( cerr, L"�̺�Ʈ Ÿ�� DB������Ʈ ����?" )
						<< BUILD_LOG( iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( vitCT->m_iEventUID )
						<< BUILD_LOG( wstrUpdateDate )
						<< END_LOG;
				}
			}
			else
#endif SERV_ACC_TIME_EVENT
				//}}
			{
				// �Ϸ� ������ ������Ʈ ����!
				DO_QUERY( L"exec dbo.gup_update_30min", L"%d, %d, N\'%s\'", % kPacket_.m_iUnitUID % vitCT->m_iEventUID % wstrUpdateDate );

				if( m_kODBC.BeginFetch() )
				{
					FETCH_DATA( iOK );
					m_kODBC.EndFetch();
				}

				if( iOK != NetError::NET_OK )
				{
					START_LOG( cerr, L"�̺�Ʈ Ÿ�� DB������Ʈ ����?" )
						<< BUILD_LOG( iOK )
						<< BUILD_LOG( kPacket_.m_iUnitUID )
						<< BUILD_LOG( vitCT->m_iEventUID )
						<< BUILD_LOG( wstrUpdateDate )
						<< END_LOG;
				}
			}
		}
	}

#endif CUMULATIVE_TIME_EVENT
	//}}

end_proc:
	return;
}
#endif SERV_SCROLL_OF_SAGE_CHECK_THE_LETTER_BOX
//}}

//{{ 2012. 05. 08	��μ�       ���� �ŷ� ���� ����
#ifdef SERV_TRADE_LOGIC_CHANGE_TRADE
IMPL_ON_FUNC( DBE_TRADE_COMPLETE_REQ )
{
	KDBE_TRADE_COMPLETE_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iUnitUID = kPacket_.m_iUnitUID;
	kPacket.m_iED = kPacket_.m_iED;
	kPacket.m_vecInventorySlotInfo = kPacket_.m_vecInventorySlotInfo;

	bool bUpdateFailed = false;

	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	SendToUser( LAST_SENDER_UID, DBE_TRADE_COMPLETE_ACK, kPacket );
}
#endif SERV_TRADE_LOGIC_CHANGE_TRADE
//}}

//{{ 2012. 07. 24	�ڼ���	�ش� ĳ������ ��� ��ų�� �� ��� ġƮ
#ifdef SERV_ADMIN_CHEAT_GET_ALL_SKILL
IMPL_ON_FUNC( DBE_ADMIN_CHEAT_GET_ALL_SKILL_REQ )
{
	KDBE_ADMIN_CHEAT_GET_ALL_SKILL_ACK kPacket;
	kPacket.m_iOK = NetError::NET_OK;

	// ���ε� ��ų ����
	if( kPacket_.m_vecUnSealedSkill.empty() == false )
	{
		BOOST_TEST_FOREACH( short, iSkillID, kPacket_.m_vecUnSealedSkill )
		{
			int iOK = NetError::NET_OK;
			DO_QUERY( L"exec dbo.gup_insert_unsealed_skill", L"%d, %d", % kPacket_.m_iUnitUID % iSkillID );
			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( iOK );
				m_kODBC.EndFetch();
			}

			// ������ �����ϸ� �ش� ��ų�� ���� �ʴ´�.
			if( iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"���ν�ų �������� ����!" )
					<< BUILD_LOG( iOK )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< BUILD_LOG( iSkillID )
					<< END_LOG;
				
				kPacket_.m_mapSkillInfo.erase( iSkillID );
				kPacket.m_iOK = NetError::ERR_ODBC_01;
				continue;
			}
			kPacket.m_vecUnSealedSkill.push_back( iSkillID );
		}
	}

	// ���� ��ų ���� �� ���
	for( std::map<int, KAdminCheatSkill>::iterator it=kPacket_.m_mapSkillInfo.begin(); it != kPacket_.m_mapSkillInfo.end(); ++it )
	{
		int iOK = NetError::NET_OK;
		const int iSkillID		= it->first;
		const int iSkillLevel	= it->second.m_iSkillLevel;
		const int iSkillCSPoint	= it->second.m_iSkillCSPoint;

		DO_QUERY( L"exec dbo.gup_update_skill_info_new", L"%d, %d, %d, %d",
			% kPacket_.m_iUnitUID 
			% iSkillID
			% iSkillLevel
			% iSkillCSPoint );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"failed to insert skill!" )
				<< BUILD_LOG( LAST_SENDER_UID )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( iSkillID )
				<< BUILD_LOG( iSkillLevel )
				<< BUILD_LOG( iSkillCSPoint )
				<< BUILD_LOG( iOK )
				<< END_LOG;

			kPacket.m_iOK = NetError::ERR_ODBC_01;
			continue;
		}

		kPacket.m_mapSkillInfo.insert( std::map<int, KAdminCheatSkill>::value_type( it->first, it->second ) );
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_ADMIN_CHEAT_GET_ALL_SKILL_ACK, kPacket );
}
#endif SERV_ADMIN_CHEAT_GET_ALL_SKILL
//}}

//{{ 2012. 05. 30	������	�׳�ý� �Ⱓ ġƮ
#ifdef SERV_CASH_SKILL_POINT_DATE_CHANGE
IMPL_ON_FUNC( DBE_ADMIN_CASH_SKILL_POINT_DATE_CHANGE_REQ )
{
	KEGS_ADMIN_CASH_SKILL_POINT_DATE_CHANGE_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_00;

	// �Ⱓ ���� ġƮ
	DO_QUERY( L"exec dbo.P_GSkill_Cash_UPD_C", L"%d, N\'%s\'", % kPacket_.m_iUnitUID % kPacket_.m_wstrModifyDate );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}
	
	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"ġƮ ���� ȣ�� ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_wstrModifyDate )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_UNKNOWN;
	}
	else
	{
		kPacket.m_wstrCspEndDate = kPacket_.m_wstrModifyDate;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_ADMIN_CASH_SKILL_POINT_DATE_CHANGE_ACK, kPacket );
}
#endif SERV_CASH_SKILL_POINT_DATE_CHANGE
//}}

//{{ ���� ED �ŷ��� ED ����ȭ - ��μ�
#ifdef SERV_SEND_LETTER_BEFOR_ED_SYNC
IMPL_ON_FUNC( DBE_SYNC_ED_REQ )
{
	KDBE_SYNC_ED_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_00;
	kPacket.m_kInfo = kPacket_;

	int iTotalChangeED = kPacket.m_kInfo.m_iChangeED - kPacket.m_kInfo.m_iED - kPacket.m_kInfo.m_iSendLetterCost;

	DO_QUERY( L"exec dbo.P_GUnit_UPT_POST_GP", L"%d, %d", % kPacket.m_kInfo.m_iFromUnitUID % iTotalChangeED );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	// ������ �����ϸ� �ش� ��ų�� ���� �ʴ´�.
	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"ED ����ȭ ����!!" )
			<< BUILD_LOG( LAST_SENDER_UID )
			<< BUILD_LOG( kPacket.m_kInfo.m_iFromUnitUID )
			<< BUILD_LOG( iTotalChangeED )
			<< BUILD_LOG( kPacket.m_iOK )
			<< END_LOG;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_SYNC_ED_ACK, kPacket );
}
#endif SERV_SEND_LETTER_BEFOR_ED_SYNC
//}}

//{{ 2012. 08. 14	�ڼ���	��õ���� ��ȭ �̺�Ʈ ���̵� ���� ���
#ifdef SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE
IMPL_ON_FUNC_NOPARAM( DBE_CHECK_THE_ARCHUANGEL_S_COIN_EVENT_LETTER_REQ )
{
	KDBE_CHECK_THE_ARCHUANGEL_S_COIN_EVENT_LETTER_ACK kPacket;

	CTime tCurrentTime = CTime::GetCurrentTime();
	CTime tMore( tCurrentTime.GetYear(), tCurrentTime.GetMonth(), tCurrentTime.GetDay(), 0, 0, 0 );
	CTime tUnder( tCurrentTime.GetYear(), tCurrentTime.GetMonth(), tCurrentTime.GetDay() + 1, 0, 0, 0 );

	std::wstring wstrMore = tMore.Format( _T("%Y-%m-%d %H:%M:%S") );
	std::wstring wstrUnder = tUnder.Format( _T("%Y-%m-%d %H:%M:%S") );

	DO_QUERY( L"exec dbo.P_GEvent_AngelCoin_GET", L"%d, N\'%s\', N\'%s\'", % FIRST_SENDER_UID % wstrMore % wstrUnder );

	// �α� �����
	int iResult = -1;
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iResult );
		m_kODBC.EndFetch();
	}

	if( iResult == 0 )
	{
		kPacket.m_bReceiveTheLetter = true;
	}
	else
	{
		kPacket.m_bReceiveTheLetter = false;
		if( iResult == -1 )	
		{
			START_LOG( cerr, L"��õ���� ��ȭ �̺�Ʈ BeginFetch() ����")
				<< END_LOG;
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_CHECK_THE_ARCHUANGEL_S_COIN_EVENT_LETTER_ACK, kPacket );
}
#endif SERV_ARCHUANGEL_S_COIN_EVENT_GUIDE
//}}

//{{ 2012. 08. 21	�ڼ���	���� ���� ����
#ifdef SERV_TRADE_LOGIC_CHANGE_LETTER
IMPL_ON_FUNC( DBE_GET_ITEM_FROM_LETTER_ARRANGE_REQ )
{
	bool bUpdateFailed = false;
	KItemQuantityUpdate kFailed;

	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kFailed.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kFailed.m_vecDeleted );

	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}

	kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange	= kFailed.m_mapQuantityChange;
	kPacket_.m_kItemQuantityUpdate.m_vecDeleted			= kFailed.m_vecDeleted;

	BOOST_TEST_FOREACH( KInventoryItemInfo, kItem, kPacket_.m_vecMoveItemSlotInfo )
	{
		int iOK;
		DO_QUERY( L"exec dbo.gup_update_item_position", L"%d, %d, %d", % kItem.m_iItemUID % static_cast<byte>( kItem.m_cSlotCategory ) % kItem.m_sSlotID );
		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();

			if( iOK != NetError::NET_OK )
			{
				START_LOG( cerr, L"������ ��ġ ������Ʈ ����." )
					<< BUILD_LOG( iOK )
					<< END_LOG;
			}
		}
		else
		{
			START_LOG( cerr, L"������ ��ġ ������Ʈ�� BeginFetch ����." );
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_GET_ITEM_FROM_LETTER_ARRANGE_ACK, kPacket_ );
}
#endif SERV_TRADE_LOGIC_CHANGE_LETTER
//}}

//{{ 2012 ���� ����2 ���� ��Ī �̺�Ʈ	- ��μ�
#ifdef SERV_2012_PVP_SEASON2_EVENT
IMPL_ON_FUNC( DBE_PVP_WIN_EVENT_CHECK_REQ )
{
	KDBE_PVP_WIN_EVENT_CHECK_ACK kPacket;
	kPacket.m_iOK					  	 = NetError::ERR_ODBC_01;
	kPacket.m_kRewardAck.m_iOK			 = NetError::ERR_ODBC_01;
	kPacket.m_kRewardAck.m_iRewardLetter.m_iFromUnitUID = kPacket_.m_iUnitUID;
	kPacket.m_kRewardAck.m_iRewardLetter.m_iToUnitUID   = kPacket_.m_iUnitUID;
	kPacket.m_kRewardAck.m_iRewardLetter.m_cScriptType  = KPostItemInfo::LT_EVENT;
	kPacket.m_kRewardAck.m_iRewardLetter.m_iScriptIndex = 10407;
	kPacket.m_iIndex = kPacket_.m_iIndex;
	kPacket.m_wstrRegDate = kPacket_.m_wstrRegDate;

	int iPvpEventIndex = 0;
	std::wstring wstrLastPvpEventDate;

	// ���� �� �ִ� �������� Ȯ���Ѵ�.
	DO_QUERY( L"exec dbo.P_GEvent_Unit_GET ", L"%d", % kPacket_.m_iUnitUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iPvpEventIndex
			>> wstrLastPvpEventDate );

		m_kODBC.EndFetch();
	}

	// ���� ���� �ִ�.
	if( iPvpEventIndex != 0 )
	{
		// ���� ��¥�ε� ���� �ε������ �̹� �̺�Ʈ�� ���� ���� ����̴�.
		CTime tCurr;
		CTime tDBRegDate;
		KncUtil::ConvertStringToCTime( kPacket_.m_wstrRegDate, tCurr );
		KncUtil::ConvertStringToCTime( wstrLastPvpEventDate, tDBRegDate );

		if( tCurr.GetDay() == tDBRegDate.GetDay() && kPacket_.m_iIndex == iPvpEventIndex )
		{
			START_LOG( cerr, L"���� �̺�Ʈ ������ ���� ����ڰ� �ƴմϴ�." )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( kPacket_.m_wstrRegDate )
				<< BUILD_LOG( iPvpEventIndex )
				<< BUILD_LOG( wstrLastPvpEventDate )
				<< END_LOG;

			goto end_proc;
		}
	}
	
	// ����
	DO_QUERY( L"exec dbo.gup_insert_post_item", L"%d, %d, %d, %d, %d, N\'%s\'",
		% kPacket_.m_iUnitUID
		% kPacket_.m_iUnitUID
		% 0
		% KPostItemInfo::LT_EVENT
		% 10407
		% kPacket.m_kRewardAck.m_iRewardLetter.m_wstrMessage
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_kRewardAck.m_iOK
				 >> kPacket.m_kRewardAck.m_iRewardLetter.m_iPostNo
				 >> kPacket.m_kRewardAck.m_iRewardLetter.m_wstrRegDate );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_kRewardAck.m_iOK == NetError::NET_OK )
	{
		DO_QUERY( L"exec dbo.P_GEvent_Unit_INT", L"%d, %d, N\'%s\'",
			% kPacket_.m_iUnitUID
			% kPacket_.m_iIndex
			% kPacket_.m_wstrRegDate
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );
			m_kODBC.EndFetch();
		}

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"���� �̺�Ʈ ������ ���� ���� �α� ��� ����" )
				<< BUILD_LOG( kPacket_.m_iUnitUID )
				<< BUILD_LOG( kPacket.m_iOK )
				<< END_LOG;
		}
	}	

end_proc:

	// �α� ��� ����
	SendToUser( LAST_SENDER_UID, DBE_PVP_WIN_EVENT_CHECK_ACK, kPacket );

	// ���� �߼� ����
	if( kPacket.m_kRewardAck.m_iOK == NetError::NET_OK )
	{
		SendToUser( LAST_SENDER_UID, DBE_INSERT_REWARD_TO_POST_ACK, kPacket.m_kRewardAck );
	}
}
#endif SERV_2012_PVP_SEASON2_EVENT
//}}

//{{ 2012. 12. 14  ���� �̼� �ý��� ( �������� ����Ʈ ) - ��μ�
#ifdef SERV_ACCOUNT_MISSION_SYSTEM
_IMPL_ON_FUNC( DBE_NEW_ACCOUNT_QUEST_GAME_DB_REQ, KDBE_NEW_QUEST_REQ )
{
	KDBE_NEW_QUEST_ACK kPacket;
	kPacket.m_iOK		= NetError::ERR_ODBC_01;
	kPacket.m_iQuestID	= kPacket_.m_iQuestID;
	kPacket.m_mapInsertedItem			= kPacket_.m_mapInsertedItem;
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;

	//////////////////////////////////////////////////////////////////////////
	// ������ ����
	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_UnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_UnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
	if( Query_InsertItemList( SEnum::GIR_QUEST_REWARD, kPacket_.m_UnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
	{
		START_LOG( cerr, L"����Ʈ ���� ���� ������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_UnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_UNKNOWN;
		goto end_proc;
	}
	//////////////////////////////////////////////////////////////////////////

	kPacket.m_iOK		= NetError::NET_OK;
end_proc:
	SendToUser( LAST_SENDER_UID, DBE_NEW_ACCOUNT_QUEST_GAME_DB_ACK, kPacket );
}

_IMPL_ON_FUNC( DBE_ACCOUNT_QUEST_COMPLETE_GAME_DB_REQ, KDBE_ACCOUNT_QUEST_COMPLETE_ACK )
{
	KDBE_QUEST_COMPLETE_ACK kPacket;

	kPacket.m_bAutoComplete				= kPacket_.m_kQuestReq.m_bAutoComplete;
	kPacket.m_bIsNew					= kPacket_.m_kQuestReq.m_bIsNew;
	kPacket.m_iOK						= NetError::ERR_ODBC_01;
	kPacket.m_mapInsertedItem			= kPacket_.m_kQuestReq.m_mapInsertedItem;
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_kQuestReq.m_vecUpdatedInventorySlot;
	kPacket.m_bIsChangeJob				= kPacket_.m_kQuestReq.m_bIsChangeJob;
	kPacket.m_cChangeUnitClass			= kPacket_.m_kQuestReq.m_cChangeUnitClass;	
	kPacket.m_kCompleteQuestInfo.m_iQuestID = kPacket_.m_kQuestReq.m_iQuestID;
	kPacket.m_kCompleteQuestInfo.m_iCompleteCount = 1;
	kPacket.m_kCompleteQuestInfo.m_tCompleteDate = kPacket_.m_tCompleteTime;

	//item uid �޾ƿ���
	if( kPacket_.m_kQuestReq.m_bIsNew == true )
	{
		bool bUpdateFailed = false;
		LIF( Query_UpdateItemQuantity( kPacket_.m_kQuestReq.m_UnitUID, kPacket_.m_kQuestReq.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
		Query_DeleteItem( kPacket_.m_kQuestReq.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

		//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
		if( bUpdateFailed )
		{
			START_LOG( cout, L"���а� ������Ʈ ����!" )
				<< BUILD_LOG( kPacket_.m_kQuestReq.m_UnitUID );

			std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kQuestReq.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
			for( ; mitQC != kPacket_.m_kQuestReq.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
			{
				START_LOG( cout, L"������ ����" )
					<< BUILD_LOG( mitQC->first )
					<< BUILD_LOG( mitQC->second );
			}
		}
		//}}

		if( Query_InsertItemList( SEnum::GIR_QUEST_REWARD, kPacket_.m_kQuestReq.m_UnitUID, kPacket_.m_kQuestReq.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
		{
			START_LOG( cerr, L"����Ʈ ���� ������ ������Ʈ ����!" )
				<< BUILD_LOG( kPacket_.m_kQuestReq.m_UnitUID )
				<< END_LOG;
			goto end_proc;
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_ACCOUNT_QUEST_COMPLETE_GAME_DB_ACK, kPacket );
};
#endif SERV_ACCOUNT_MISSION_SYSTEM
//}}

//{{ 2012. 12. 24	�ڼ���	�� ���� ���� ��� �߰�
#ifdef SERV_PET_AUTO_LOOTING
IMPL_ON_FUNC( DBE_PET_AUTO_LOOTING_NOT )
{
	int iOK = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec dbo.P_GPet_Info_PickUP_UPT", L"%d", % kPacket_.m_iPetUID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}

	if( iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"���� ���� ���� ��� DB ������Ʈ�� ���� �Ͽ����ϴ�." )
			<< BUILD_LOG( kPacket_.m_iPetUID )
			<< END_LOG;
	}
end_proc:
	return;
}
#endif SERV_PET_AUTO_LOOTING
//}}

//{{ 2011.05.04   �ӱԼ� �ƹ�Ÿ �ռ� �ý���
#ifdef SERV_SYNTHESIS_AVATAR
IMPL_ON_FUNC( DBE_OPEN_SYNTHESIS_ITEM_REQ )
{
	KDBE_OPEN_SYNTHESIS_ITEM_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_mapInsertedItem			= kPacket_.m_mapInsertedItem;
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_mapSynthesisData			= kPacket_.m_mapSynthesisData;
	kPacket.m_iKeyData					= kPacket_.m_iKeyData;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	if( Query_InsertItemList( SEnum::GIR_SYNTHESIS_ITEM, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
	{
		START_LOG( cerr, L"������ ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_RANDOM_ITEM_06;
	}
	else
	{
		kPacket.m_iOK = NetError::NET_OK;
	}

	//{{ 2009. 9. 2  ������		�к�
	Query_UpdateSealItem( kPacket_.m_setSealCashItem, kPacket.m_mapItemInfo );
	//}}

	SendToUser( LAST_SENDER_UID, DBE_OPEN_SYNTHESIS_ITEM_ACK, kPacket );
}
#endif SERV_SYNTHESIS_AVATAR
//}}

#ifdef SERV_HALLOWEEN_PUMPKIN_FAIRY_PET
IMPL_ON_FUNC( DBE_CHANGE_PET_ID_REQ )
{
	KDBE_CHANGE_PET_ID_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	// �� ID ����
	DO_QUERY( L"exec dbo.P_GPet_Info_UPD_ChangePet", L"%d, %d", % kPacket_.m_iPetUID % kPacket_.m_iAfterPetID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"���� ���� ��� ������ ���� DB ������Ʈ�� ���� �Ͽ����ϴ�." )
			<< BUILD_LOG( kPacket_.m_iPetUID )
			<< END_LOG;
	}

	kPacket.m_iPetUID = kPacket_.m_iPetUID;
	kPacket.m_iBeforePetID = kPacket_.m_iBeforePetID;
	kPacket.m_iAfterPetID = kPacket_.m_iAfterPetID;

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_CHANGE_PET_ID_ACK, kPacket );
	return;
}
#endif //SERV_HALLOWEEN_PUMPKIN_FAIRY_PET

#ifdef SERV_ADD_WARP_BUTTON
IMPL_ON_FUNC( DBE_INSERT_WARP_VIP_REQ )
{
	KDBE_INSERT_WARP_VIP_ACK kAck;
	kAck.m_iOK = NetError::ERR_ODBC_01;
	kAck.m_wstrEndDate = L"2000-01-01 00:00:00";
	
	// VIP �Ⱓ ���Ž��� �ݴϴ�.
	DO_QUERY( L"exec dbo.P_GVIPTicket_SET", L" %d, %d", % kPacket_.m_iUnitUID % kPacket_.m_iPeriod );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kAck.m_iOK
			>> kAck.m_wstrEndDate );

		m_kODBC.EndFetch();
	}
	
	START_LOG( clog, L"[TEST] VIP �Ⱓ DB�� ������Ʈ �߽��ϴ�." )
		<< BUILD_LOG( kPacket_.m_iUnitUID )
		<< BUILD_LOG( kPacket_.m_iPeriod )
		<< BUILD_LOG( kAck.m_wstrEndDate.c_str() )
		<< END_LOG;

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_INSERT_WARP_VIP_ACK, kAck );
}
#endif // SERV_ADD_WARP_BUTTON

//{{ 2013. 3. 11	�ڼ���	 ���� ��ŷ �ý���
#ifdef SERV_LOCAL_RANKING_SYSTEM
IMPL_ON_FUNC( DBE_GAME_LOCAL_RANKING_USER_INFO_READ_REQ )
{
	KDBE_ACCOUNT_LOCAL_RANKING_USER_INFO_READ_ACK kPacket;

	// ���� ����
	DO_QUERY( L"exec dbo.P_GFriendSystem_UnitInfo_GET", L"%d", % kPacket_.m_iUnitUID );

	if( m_kODBC.BeginFetch() )
	{
		kPacket.m_iOK = NetError::ERR_ODBC_01;

		FETCH_DATA( kPacket.m_iOK
			>> kPacket.m_iUID
			>> kPacket.m_kUnitInfo.m_wstrNickName
			>> kPacket.m_kUnitInfo.m_ucLevel
			>> kPacket.m_kUnitInfo.m_cUnitClass
			>> kPacket.m_kUnitInfo.m_iPoint[SEnum::LRMTC_SPIRIT]
			>> kPacket.m_kUnitInfo.m_iPoint[SEnum::LRMTC_AP]
			);
		m_kODBC.EndFetch();

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			SendToUser( LAST_SENDER_UID, DBE_ACCOUNT_LOCAL_RANKING_USER_INFO_READ_ACK, kPacket );
			return;
		}

		kPacket.m_kUserInfo.m_iUserUID	= kPacket_.m_iUserUID;
		kPacket.m_kUnitInfo.m_iUnitUID	= kPacket_.m_iUnitUID;
		kPacket.m_bFiltered				= kPacket_.m_bFiltered;
	}

end_proc:
	KncSend( PI_GS_GAME_DB, 0, PI_ACCOUNT_DB, 0, anTrace_, DBE_ACCOUNT_LOCAL_RANKING_USER_INFO_READ_REQ, kPacket );
}

IMPL_ON_FUNC( DBE_GAME_LOCAL_RANKING_UNIT_INFO_READ_FOR_INQUIRY_REQ )
{
	KDBE_ACCOUNT_LOCAL_RANKING_UNIT_INFO_READ_FOR_INQUIRY_REQ kPacket;

	// ���� ����
	DO_QUERY( L"exec dbo.P_GFriendSystem_UnitInfo_GET", L"%d", % kPacket_.m_iUnitUID );

	if( m_kODBC.BeginFetch() )
	{
		kPacket.m_iOK = NetError::ERR_ODBC_01;

		FETCH_DATA( kPacket.m_iOK
			>> kPacket.m_iUID
			>> kPacket.m_kUnitInfo.m_wstrNickName
			>> kPacket.m_kUnitInfo.m_ucLevel
			>> kPacket.m_kUnitInfo.m_cUnitClass
			>> kPacket.m_kUnitInfo.m_iPoint[SEnum::LRMTC_SPIRIT]
			>> kPacket.m_kUnitInfo.m_iPoint[SEnum::LRMTC_AP]
			);
		m_kODBC.EndFetch();

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			SendToUser( LAST_SENDER_UID, DBE_ACCOUNT_LOCAL_RANKING_UNIT_INFO_READ_FOR_INQUIRY_ACK, kPacket );
			return;
		}

		kPacket.m_kUserInfo.m_iUserUID	= kPacket_.m_iUserUID;
		kPacket.m_kUnitInfo.m_iUnitUID	= kPacket_.m_iUnitUID;

		kPacket.m_nViewPage				= kPacket_.m_nViewPage;
		kPacket.m_cMainTabIndex			= kPacket_.m_cMainTabIndex;
		kPacket.m_cSubTabIndex			= kPacket_.m_cSubTabIndex;
		kPacket.m_byteFilter			= kPacket_.m_byteFilter;
#ifdef SERV_LOCAL_RANKING_SYSTEM_STATISTICS_LOG// �۾���¥: 2013-05-15	// �ڼ���
		kPacket.m_bRankingButtonClick	= kPacket_.m_bRankingButtonClick;
#endif // SERV_LOCAL_RANKING_SYSTEM_STATISTICS_LOG
	}

end_proc:
	KncSend( PI_GS_GAME_DB, 0, PI_ACCOUNT_DB, 0, anTrace_, DBE_ACCOUNT_LOCAL_RANKING_UNIT_INFO_READ_FOR_INQUIRY_REQ, kPacket );
}

IMPL_ON_FUNC( DBE_GAME_LOCAL_RANKING_UNIT_INFO_READ_FOR_INCREASE_REQ )
{
	KDBE_ACCOUNT_LOCAL_RANKING_UNIT_INFO_READ_FOR_INCREASE_ACK kPacket;

	// ���� ����
	DO_QUERY( L"exec dbo.P_GFriendSystem_UnitInfo_GET", L"%d", % kPacket_.m_iUnitUID );

	if( m_kODBC.BeginFetch() )
	{
		kPacket.m_iOK = NetError::ERR_ODBC_01;

		FETCH_DATA( kPacket.m_iOK
			>> kPacket.m_iUID
			>> kPacket.m_kUnitInfo.m_wstrNickName
			>> kPacket.m_kUnitInfo.m_ucLevel
			>> kPacket.m_kUnitInfo.m_cUnitClass
			>> kPacket.m_kUnitInfo.m_iPoint[SEnum::LRMTC_SPIRIT]
			>> kPacket.m_kUnitInfo.m_iPoint[SEnum::LRMTC_AP]
			);
		m_kODBC.EndFetch();

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			SendToUser( LAST_SENDER_UID, DBE_ACCOUNT_LOCAL_RANKING_UNIT_INFO_READ_FOR_INCREASE_ACK, kPacket );
			return;
		}

		kPacket.m_kUserInfo.m_iUserUID	= kPacket_.m_iUserUID;
		kPacket.m_kUnitInfo.m_iUnitUID	= kPacket_.m_iUnitUID;

		kPacket.m_iIncreasedSpirit	= kPacket_.m_iIncreasedSpirit;
		kPacket.m_iIncreasedAP		= kPacket_.m_iIncreasedAP;
		kPacket.m_bForce			= kPacket_.m_bForce;
	}

end_proc:
	KncSend( PI_GS_GAME_DB, 0, PI_ACCOUNT_DB, 0, anTrace_, DBE_ACCOUNT_LOCAL_RANKING_UNIT_INFO_READ_FOR_INCREASE_REQ, kPacket );
}
#endif SERV_LOCAL_RANKING_SYSTEM
//}}

//{{ 2013. 03. 21	 ��õ�� �ý��� ���� - ��μ�
#ifdef SERV_RECOMMEND_LIST_EVENT
IMPL_ON_FUNC( DBE_RECOMMEND_USER_GET_NEXON_SN_REQ )
{
	KDBE_RECOMMEND_USER_GET_NEXON_SN_ACK kAck;
	kAck.m_iRecommendUserNexonSN = kPacket_.m_iRecommendUserNexonSN;
	kAck.m_iRecommendUnitUID = kPacket_.m_iRecommendUnitUID;	
	kAck.m_wstrRecommendedUserNickName = kPacket_.m_wstrRecommendedUserNickName;

	DO_QUERY( L"exec dbo.P_GUnitNickName_MUser_OID_GET", L" N\'%s\'", % kPacket_.m_wstrRecommendedUserNickName );

	if( m_kODBC.BeginFetch() )
	{
#ifdef SERV_RECOMMEND_LIST_EVENT_2013_07	// ���볯¥: 2013-07-04
		FETCH_DATA( kAck.m_iRecommendedUserNexonSN 
			>> kAck.m_iRecommendedUserChannelCode  );
#else	// SERV_RECOMMEND_LIST_EVENT_2013_07
		FETCH_DATA( kAck.m_iRecommendedUserNexonSN );
#endif SERV_RECOMMEND_LIST_EVENT_2013_07

		m_kODBC.EndFetch();
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_RECOMMEND_USER_GET_NEXON_SN_ACK, kAck );
}
#endif SERV_RECOMMEND_LIST_EVENT
//}

#ifdef	SERV_RIDING_PET_SYSTM// ���볯¥: 2013-04-21
IMPL_ON_FUNC( DBE_GET_RIDING_PET_LIST_REQ )
{
	KDBE_GET_RIDING_PET_LIST_ACK kPacket;
	kPacket.m_iOK					= NetError::ERR_ODBC_01;
	kPacket.m_iAfterWorkStorageKey	= kPacket_.m_iAfterWorkStorageKey;

	DO_QUERY( L"exec dbo.P_GRiding_SEL", L"%d", % kPacket_.m_iUnitUID );

	while( m_kODBC.Fetch() == true )
	{
		CTime tLastUnSummonDate;
		std::wstring wstrLastUnSummonDate;

		KRidingPetInfo	kInfo;
		USHORT			usStamina;

		FETCH_DATA( kInfo.m_iRidingPetUID
			>> kInfo.m_usRindingPetID
			>> usStamina
			>> kInfo.m_wstrDestroyDate
			>> wstrLastUnSummonDate
			);

		wstrLastUnSummonDate		= wstrLastUnSummonDate.substr( 0, 19 );
		kInfo.m_fStamina			= usStamina;
		kInfo.m_iLastUnSummonDate	= ( KncUtil::ConvertStringToCTime( wstrLastUnSummonDate, tLastUnSummonDate ) == true ) ? tLastUnSummonDate.GetTime() : 0;
		kPacket.m_vecRidingPetList.push_back( kInfo );
	}

	kPacket.m_iOK = NetError::NET_OK;

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_GET_RIDING_PET_LIST_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_CREATE_RIDING_PET_REQ )
{
	KDBE_CREATE_RIDING_PET_ACK kPacket;
	kPacket.m_iOK								= NetError::ERR_ODBC_01;
	kPacket.m_kCreatedPetInfo.m_usRindingPetID	= kPacket_.m_usCreatePetID;
	kPacket.m_vecUpdatedInventorySlot			= kPacket_.m_vecUpdatedInventorySlot;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		BOOST_MAP_CONST_FOREACH( UidType, int, kItem, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( kItem.first )
				<< BUILD_LOG( kItem.second );
		}
	}
	//}}

	// ���̵� �� ����
	DO_QUERY( L"exec dbo.P_GRiding_INT", L"%d, %d, %d", 
		% kPacket_.m_iUnitUID
		% kPacket_.m_usCreatePetID
		% kPacket_.m_sPeriod
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK
			>> kPacket.m_kCreatedPetInfo.m_iRidingPetUID
			>> kPacket.m_kCreatedPetInfo.m_wstrDestroyDate
			);

		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"���̵� �� ���� ����!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_usCreatePetID )
			<< BUILD_LOG( kPacket_.m_sPeriod )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_RIDING_PET_05;
		goto end_proc;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_CREATE_RIDING_PET_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_RELEASE_RIDING_PET_REQ )
{
	KEGS_RELEASE_RIDING_PET_ACK kPacket;
	kPacket.m_iOK			= NetError::ERR_ODBC_01;
	kPacket.m_iRidingPetUID	= kPacket_.m_iRidingPetUID;
	
	DO_QUERY( L"exec dbo.P_GRiding_UPD_Deleted", L"%d, %d",
		% kPacket_.m_iRidingPetUID
		% kPacket_.m_iUnitUID
		);

	if( m_kODBC.BeginFetch() == true )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"���̵� �� �����ֱ� ����!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket.m_iRidingPetUID )
			<< END_LOG;
		
		kPacket.m_iOK = NetError::ERR_RIDING_PET_11;	// ���̵� �� �����ֱ⿡ �����Ͽ����ϴ�.
		goto end_proc;
	}

#ifdef SERV_EVENT_RIDING_WITH_SUB_QUEST
	// ���� ����Ʈ�� �������̸�
	if(kPacket_.m_bEventQuest == true)
	{
		// �̺�Ʈ ������ ����!
		KDBE_INSERT_REWARD_TO_POST_REQ kPacketToDBPVPEVENT;
		kPacketToDBPVPEVENT.m_iFromUnitUID = kPacket_.m_iUnitUID;
		kPacketToDBPVPEVENT.m_iToUnitUID   = kPacket_.m_iUnitUID;
		kPacketToDBPVPEVENT.m_iRewardType  = KPostItemInfo::LT_EVENT;
		kPacketToDBPVPEVENT.m_iRewardID	   = _CONST_AEVENT_RIDING_WITH_SUB_QUEST::iHasEventQuestRewardID; 

		// DBE_INSERT_REWARD_TO_POST_REQ, kPacketToDBPVPEVENT); // �̺κ� ����
		KDBE_INSERT_REWARD_TO_POST_ACK kPacket;
		kPacket.m_iOK					  	   = NetError::ERR_ODBC_01;
		kPacket.m_iRewardLetter.m_iFromUnitUID = kPacketToDBPVPEVENT.m_iFromUnitUID;
		kPacket.m_iRewardLetter.m_iToUnitUID   = kPacketToDBPVPEVENT.m_iToUnitUID;
		kPacket.m_iRewardLetter.m_cScriptType  = kPacketToDBPVPEVENT.m_iRewardType;
		kPacket.m_iRewardLetter.m_iScriptIndex = kPacketToDBPVPEVENT.m_iRewardID;
		kPacket.m_iRewardLetter.m_iQuantity	   = kPacketToDBPVPEVENT.m_sQuantity;
		kPacket.m_iRewardLetter.m_wstrMessage  = kPacketToDBPVPEVENT.m_wstrMessage;

		// ����
		DO_QUERY( L"exec dbo.gup_insert_post_item", L"%d, %d, %d, %d, %d, N\'%s\'",
			% kPacketToDBPVPEVENT.m_iFromUnitUID
			% kPacketToDBPVPEVENT.m_iToUnitUID
			% kPacketToDBPVPEVENT.m_sQuantity
			% kPacketToDBPVPEVENT.m_iRewardType
			% kPacketToDBPVPEVENT.m_iRewardID
			% kPacketToDBPVPEVENT.m_wstrMessage
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK
				>> kPacket.m_iRewardLetter.m_iPostNo
				>> kPacket.m_iRewardLetter.m_wstrRegDate );
			m_kODBC.EndFetch();
		}

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			switch( kPacket.m_iOK )
			{
			default:
				kPacket.m_iOK = NetError::ERR_REWARD_TO_POST_00;
				break;
			}
		}	

		if( kPacketToDBPVPEVENT.m_bGameServerEvent )
		{
			SendToServer( DBE_INSERT_REWARD_TO_POST_ACK, kPacket );
		}
		else
		{
			SendToUser( LAST_SENDER_UID, DBE_INSERT_REWARD_TO_POST_ACK, kPacket );
		}		
	}
#endif //SERV_EVENT_RIDING_WITH_SUB_QUEST

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_RELEASE_RIDING_PET_ACK, kPacket );
}
#endif	// SERV_RIDING_PET_SYSTM

//{{ 2013. 04. 01	 �ο� �ý��� - ��μ�
#ifdef SERV_RELATIONSHIP_SYSTEM
_IMPL_ON_FUNC( DBE_COUPLE_PROPOSE_USER_FIND_REQ, KEGS_COUPLE_PROPOSE_REQ )
{
	KDBE_COUPLE_PROPOSE_USER_FIND_ACK kAck;
	kAck.m_iOK = NetError::ERR_ODBC_01;
	kAck.m_wstrOtherNickName = kPacket_.m_wstrUnitName;

	DO_QUERY( L"exec dbo.P_GCouple_Info_SEL_Check", L"N\'%s\'", % kPacket_.m_wstrUnitName );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kAck.m_iOK
				 >> kAck.m_iOtherUserUID
				 >> kAck.m_iOtherUnitUID
				 >> kAck.m_cUnitClass
				 >> kAck.m_cRelationshipType
				 );

		m_kODBC.EndFetch();
	}

	// ���� ���� �ʴ� �г���
	if( kAck.m_iOK == -1 )
	{
		kAck.m_iOK = NetError::ERR_RELATIONSHIP_04;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_COUPLE_PROPOSE_USER_FIND_ACK, kAck );
};

IMPL_ON_FUNC( DBE_COUPLE_MAKING_SUCCESS_REQ )
{
	KDBE_COUPLE_MAKING_SUCCESS_ACK kAck;
	kAck.m_iOK = NetError::ERR_ODBC_01;
	kAck.m_iRequestUnitUID = kPacket_.m_iRequestUnitUID;
	kAck.m_wstrRequestUnitName = kPacket_.m_wstrRequestUnitName;
	kAck.m_iAcceptUserUID = kPacket_.m_iAcceptUserUID;
	kAck.m_ucAcceptUnitLevel = kPacket_.m_ucAcceptUnitLevel;
	kAck.m_cAcceptUnitClass = kPacket_.m_cAcceptUnitClass;
	kAck.m_iAcceptUnitUID = kPacket_.m_iAcceptUnitUID;
	kAck.m_wstrAcceptUnitName = kPacket_.m_wstrAcceptUnitName;
	kAck.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;

	Query_DeleteItem( kPacket_.m_vecDeleted, kAck.m_kItemQuantityUpdate.m_vecDeleted );

	// ���� �ð� ����
	CTime tNow = CTime::GetCurrentTime();

	// ������ ��� �����ؾ����� �ο� ���� ���
	if( kAck.m_kItemQuantityUpdate.m_vecDeleted.empty() == true )
	{		
		std::wstring wstrNow = tNow.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
		// ��Ī�� �ʱⰪ - ĳ�ø� ������� ������ �������
		std::wstring wstrLoveWord;

		DO_QUERY( L"exec dbo.P_GCouple_Info_INT_UPD", L"%d, %d, %d, N\'%s\', N\'%s\', %d", 
				% 1
				% kAck.m_iRequestUnitUID
				% kAck.m_iAcceptUnitUID
				% wstrNow
				% wstrLoveWord
				% 0 );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kAck.m_iOK );

			m_kODBC.EndFetch();
		}
	}
	else
	{
		kAck.m_iOK = NetError::ERR_RELATIONSHIP_11;
	}

	if( kAck.m_iOK == NetError::NET_OK )
	{
		kAck.m_tDate = tNow.GetTime();

		START_LOG( clog, L"Ŀ�� �α� ����!" )
			<< BUILD_LOG( kAck.m_iOK )
			<< BUILD_LOG( kAck.m_wstrRequestUnitName )
			<< BUILD_LOG( kAck.m_wstrAcceptUnitName )
			<< BUILD_LOG( static_cast<int>(kAck.m_kItemQuantityUpdate.m_vecDeleted.size()) )
			<< BUILD_LOG( static_cast<int>(kPacket_.m_vecDeleted.size()) )
			<< END_LOG;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_COUPLE_MAKING_SUCCESS_ACK, kAck );
}

IMPL_ON_FUNC( DBE_WEDDING_PROPOSE_REQ )
{
	KDBE_WEDDING_PROPOSE_ACK kAck;
	kAck.m_iOK = NetError::ERR_ODBC_01;
	
	kAck.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;
	kAck.m_wstrWeddingDate = kPacket_.m_wstrWeddingDate;;
	kAck.m_iManUID = kPacket_.m_iManUID;;
	kAck.m_iGirlUID = kPacket_.m_iGirlUID;;
	kAck.m_cWeddingHallType = kPacket_.m_cWeddingHallType;;
	kAck.m_cOfficiantNPC = kPacket_.m_cOfficiantNPC;;
	kAck.m_wstrWeddingMsg = kPacket_.m_wstrWeddingMsg;;

	// ������ ����
	bool bUpdateFailed = false;
	Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kAck.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kAck.m_kItemQuantityUpdate.m_vecDeleted );

	// ���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}

	// ������ ��� �����ؾ����� �ο� ���� ���
	if( kAck.m_kItemQuantityUpdate.m_vecDeleted.empty() == true && bUpdateFailed == false )
	{
		// ���� �ð� ����
		CTime tNow = CTime::GetCurrentTime();
		std::wstring wstrNow = tNow.Format( _T( "%Y-%m-%d %H:%M:%S" ) );	

		DO_QUERY( L"exec dbo.P_GCouple_WeddingInfo_INT", L"%d, %d, %d, %d, N\'%s\', N\'%s\', N\'%s\'", 
			% kPacket_.m_iManUID
			% kPacket_.m_iGirlUID
			% static_cast<int>(kPacket_.m_cWeddingHallType)
			% static_cast<int>(kPacket_.m_cOfficiantNPC)
			% wstrNow
			% kPacket_.m_wstrWeddingDate
			% kPacket_.m_wstrWeddingMsg );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kAck.m_iOK
					 >> kAck.m_iWeddingUID );

			m_kODBC.EndFetch();
		}

		kAck.m_iOK = NetError::NET_OK;


	}
	else
	{
		kAck.m_iOK = NetError::ERR_RELATIONSHIP_20;
	}

	if( kAck.m_iOK == NetError::NET_OK )
	{
		if( Quety_WeddingLetter( LAST_SENDER_UID, kPacket_, kAck ) == false )
		{
			// ���� ���� �ߴ�!
			START_LOG( cerr, L"��ȥ ���� ���� �߼� ����!!" )
				<< END_LOG;

			kAck.m_iOK = NetError::ERR_RELATIONSHIP_20;
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_WEDDING_PROPOSE_ACK, kAck );
}

bool KGSGameDBThread::Quety_WeddingLetter( IN UidType iSendUserUID, IN KDBE_WEDDING_PROPOSE_REQ& kPacket_, IN OUT KDBE_WEDDING_PROPOSE_ACK& kAck )
{
	//////////////////////////////////////////////////////////////////////////
	// ���� �߼�
	int iOK = NetError::ERR_ODBC_01;

	std::wstring wstReserveTemp;

	char cPostTypeInvitation = KPostItemInfo::LT_WEDDING_INVITATION;
	int iPostInvitationItem = CXSLItem::SI_WEDDING_INVITATION_ITEM;

	int	iPostNo = 0;
	//////////////////////////////////////////////////////////////////////////
	// �Ŷ�
	{
		KPostItemInfo kInfo;
		kInfo.m_iFromUnitUID = kPacket_.m_iManUID;
		kInfo.m_iToUnitUID   = kPacket_.m_iManUID;
		kInfo.m_cScriptType  = KPostItemInfo::LT_WEDDING_RESERVE;
		kInfo.m_iScriptIndex = CXSLItem::SI_WEDDING_RESERVATION_ITEM;
		kInfo.m_iQuantity	   = 1;

		DO_QUERY( L"exec dbo.gup_insert_post_item", L"%d, %d, %d, %d, %d, N\'%s\'",
			% kInfo.m_iFromUnitUID
			% kInfo.m_iToUnitUID
			% kInfo.m_iQuantity
			% static_cast<int>(kInfo.m_cScriptType)
			% kInfo.m_iScriptIndex
			% kInfo.m_wstrMessage
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK
				>> kInfo.m_iPostNo
				>> kInfo.m_wstrRegDate );
			m_kODBC.EndFetch();
		}

		if( iOK == NetError::NET_OK )
		{
			KDBE_INSERT_REWARD_TO_POST_ACK kPacket;
			kPacket.m_iOK = iOK;
			kPacket.m_iRewardLetter = kInfo;
			SendToUser( iSendUserUID, DBE_INSERT_REWARD_TO_POST_ACK, kPacket );

			iOK = NetError::ERR_ODBC_01;

			// Gpost_Wedding �� ���� �߰�
			DO_QUERY( L"exec dbo.P_Gpost_Wedding_INT", L"%d, %d",
				% kInfo.m_iPostNo
				% kAck.m_iWeddingUID );

			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( iOK );
				m_kODBC.EndFetch();
			}

			if( iOK == NetError::NET_OK )
			{
				START_LOG( cerr, L"������ Wedding ���� insert ����" )
					<< BUILD_LOG( iOK )
					<< BUILD_LOG( kInfo.m_iPostNo )
					<< BUILD_LOG( kAck.m_iWeddingUID )
					<< END_LOG;

				kInfo.m_iFromUnitUID = kPacket_.m_iManUID;
				kInfo.m_iToUnitUID   = kPacket_.m_iManUID;
				kInfo.m_cScriptType  = KPostItemInfo::LT_WEDDING_REWARD;
				kInfo.m_iScriptIndex = CXSLItem::SI_WEDDING_DRESS_ITEM;
				kInfo.m_iQuantity	   = 1;

				DO_QUERY( L"exec dbo.gup_insert_post_item", L"%d, %d, %d, %d, %d, N\'%s\'",
					% kInfo.m_iFromUnitUID
					% kInfo.m_iToUnitUID
					% kInfo.m_iQuantity
					% static_cast<int>(kInfo.m_cScriptType)
					% kInfo.m_iScriptIndex
					% kInfo.m_wstrMessage
					);

				if( m_kODBC.BeginFetch() )
				{
					FETCH_DATA( iOK
						>> kInfo.m_iPostNo
						>> kInfo.m_wstrRegDate );
					m_kODBC.EndFetch();
				}

				if( iOK == NetError::NET_OK )
				{
					KDBE_INSERT_REWARD_TO_POST_ACK kPacket;
					kPacket.m_iOK = iOK;
					kPacket.m_iRewardLetter = kInfo;
					SendToUser( iSendUserUID, DBE_INSERT_REWARD_TO_POST_ACK, kPacket );
				}
			}
		}
		else
		{
			START_LOG( cerr, L"��ȥ�� ���� ���� �߼� ����" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kInfo.m_iFromUnitUID )
				<< BUILD_LOG( kInfo.m_iToUnitUID )
				<< BUILD_LOG( kInfo.m_iQuantity )
				<< BUILD_LOGc( kInfo.m_cScriptType )
				<< BUILD_LOG( kInfo.m_iScriptIndex )
				<< END_LOG;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	// �ź�
	{
		KPostItemInfo kInfo;
		kInfo.m_iFromUnitUID = kPacket_.m_iGirlUID;
		kInfo.m_iToUnitUID   = kPacket_.m_iGirlUID;
		kInfo.m_cScriptType  = KPostItemInfo::LT_WEDDING_RESERVE;
		kInfo.m_iScriptIndex = CXSLItem::SI_WEDDING_RESERVATION_ITEM;
		kInfo.m_iQuantity	   = 1;

		DO_QUERY( L"exec dbo.gup_insert_post_item", L"%d, %d, %d, %d, %d, N\'%s\'",
			% kInfo.m_iFromUnitUID
			% kInfo.m_iToUnitUID
			% kInfo.m_iQuantity
			% static_cast<int>(kInfo.m_cScriptType)
			% kInfo.m_iScriptIndex
			% kInfo.m_wstrMessage
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK
				>> kInfo.m_iPostNo
				>> kInfo.m_wstrRegDate );
			m_kODBC.EndFetch();
		}

		if( iOK == NetError::NET_OK )
		{
			KDBE_INSERT_REWARD_TO_POST_ACK kPacket;
			kPacket.m_iOK = iOK;
			kPacket.m_iRewardLetter = kInfo;
			SendToUser( iSendUserUID, DBE_INSERT_REWARD_TO_POST_ACK, kPacket );

			iOK = NetError::ERR_ODBC_01;

			// Gpost_Wedding �� ���� �߰�
			DO_QUERY( L"exec dbo.P_Gpost_Wedding_INT", L"%d, %d",
				% kInfo.m_iPostNo
				% kAck.m_iWeddingUID );

			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( iOK );
				m_kODBC.EndFetch();
			}

			if( iOK == NetError::NET_OK )
			{
				START_LOG( cerr, L"������ Wedding ���� insert ����" )
					<< BUILD_LOG( iOK )
					<< BUILD_LOG( kInfo.m_iPostNo )
					<< BUILD_LOG( kAck.m_iWeddingUID )
					<< END_LOG;

				kInfo.m_iFromUnitUID = kPacket_.m_iGirlUID;
				kInfo.m_iToUnitUID   = kPacket_.m_iGirlUID;
				kInfo.m_cScriptType  = KPostItemInfo::LT_WEDDING_REWARD;
				kInfo.m_iScriptIndex = CXSLItem::SI_WEDDING_DRESS_ITEM;
				kInfo.m_iQuantity	   = 1;

				DO_QUERY( L"exec dbo.gup_insert_post_item", L"%d, %d, %d, %d, %d, N\'%s\'",
					% kInfo.m_iFromUnitUID
					% kInfo.m_iToUnitUID
					% kInfo.m_iQuantity
					% static_cast<int>(kInfo.m_cScriptType)
					% kInfo.m_iScriptIndex
					% kInfo.m_wstrMessage
					);

				if( m_kODBC.BeginFetch() )
				{
					FETCH_DATA( iOK
						>> kInfo.m_iPostNo
						>> kInfo.m_wstrRegDate );
					m_kODBC.EndFetch();
				}

				if( iOK == NetError::NET_OK )
				{
					KDBE_INSERT_REWARD_TO_POST_ACK kPacket;
					kPacket.m_iOK = iOK;
					kPacket.m_iRewardLetter = kInfo;
					SendToUser( iSendUserUID, DBE_INSERT_REWARD_TO_POST_ACK, kPacket );
				}
			}
		}
		else
		{
			START_LOG( cerr, L"��ȥ�� ���� ���� �߼� ����" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kInfo.m_iFromUnitUID )
				<< BUILD_LOG( kInfo.m_iToUnitUID )
				<< BUILD_LOG( kInfo.m_iQuantity )
				<< BUILD_LOGc( kInfo.m_cScriptType )
				<< BUILD_LOG( kInfo.m_iScriptIndex )
				<< END_LOG;
		}
	}
	// �ϰ�
	BOOST_TEST_FOREACH( UidType, iRecvUID, kPacket_.m_vecInviteUnitList )
	{
		KPostItemInfo kInfo;
		kInfo.m_iFromUnitUID = kPacket_.m_iManUID;
		kInfo.m_iToUnitUID   = iRecvUID;
		kInfo.m_cScriptType  = KPostItemInfo::LT_WEDDING_INVITATION;
		kInfo.m_iScriptIndex = CXSLItem::SI_WEDDING_INVITATION_ITEM;
		kInfo.m_iQuantity	   = 1;
		kInfo.m_wstrFromNickName = kPacket_.m_wstrWeddingNickName;
		kInfo.m_wstrMessage = kPacket_.m_wstrWeddingMsg;

		DO_QUERY( L"exec dbo.gup_insert_post_item", L"%d, %d, %d, %d, %d, N\'%s\'",
			% kInfo.m_iFromUnitUID
			% kInfo.m_iToUnitUID
			% kInfo.m_iQuantity
			% static_cast<int>(kInfo.m_cScriptType)
			% kInfo.m_iScriptIndex
			% kInfo.m_wstrMessage
			);

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK
				>> kInfo.m_iPostNo
				>> kInfo.m_wstrRegDate );
			m_kODBC.EndFetch();
		}

		if( iOK == NetError::NET_OK )
		{
			KDBE_INSERT_REWARD_TO_POST_ACK kPacket;
			kPacket.m_iOK = iOK;
			kPacket.m_iRewardLetter = kInfo;
			SendToUser( iSendUserUID, DBE_INSERT_REWARD_TO_POST_ACK, kPacket );

			iOK = NetError::ERR_ODBC_01;

			// Gpost_Wedding �� ���� �߰�
			DO_QUERY( L"exec dbo.P_Gpost_Wedding_INT", L"%d, %d",
				% kInfo.m_iPostNo
				% kAck.m_iWeddingUID );

			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( iOK );
				m_kODBC.EndFetch();
			}

			if( iOK == NetError::NET_OK )
			{
				START_LOG( cerr, L"������ Wedding ���� insert ����" )
					<< BUILD_LOG( iOK )
					<< BUILD_LOG( kInfo.m_iPostNo )
					<< BUILD_LOG( kAck.m_iWeddingUID )
					<< END_LOG;
			}
		}
		else
		{
			START_LOG( cerr, L"��ȥ�� ���� ���� �߼� ����" )
				<< BUILD_LOG( iOK )
				<< BUILD_LOG( kInfo.m_iFromUnitUID )
				<< BUILD_LOG( kInfo.m_iToUnitUID )
				<< BUILD_LOG( kInfo.m_iQuantity )
				<< BUILD_LOGc( kInfo.m_cScriptType )
				<< BUILD_LOG( kInfo.m_iScriptIndex )
				<< END_LOG;
		}
	}

	return true;

end_proc:
	return false;
}

bool KGSGameDBThread::Query_UpdateLastLogOffDate( IN const KDBE_UPDATE_UNIT_INFO_REQ& kReq )
{
	if( kReq.m_cUnitRelationshipType == SEnum::RT_SOLO )
	{
		return true;
	}

	int iOK = NetError::ERR_ODBC_01;
	CTime tCurr = CTime::GetCurrentTime();
	std::wstring wstrLastDate;
	wstrLastDate = tCurr.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
	// ������ ��ġ ����
	DO_QUERY( L"exec dbo.P_GCouple_Info_UPD_LastLogOffDate", L"%d, N\'%s\'",
		% kReq.m_iUnitUID
		% wstrLastDate
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}

end_proc:
	if( iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"���� ������ �α׿��� �ð� ������Ʈ ����." )
			<< BUILD_LOG( iOK )
			<< BUILD_LOG( kReq.m_iUnitUID )
			<< BUILD_LOG( wstrLastDate )
			<< END_LOG;
		return false;
	}

	return true;
}

_IMPL_ON_FUNC( DBE_CHECK_NICK_NAME_REQ, KEGS_CHECK_NICK_NAME_REQ )
{
	KEGS_CHECK_NICK_NAME_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_wstrNickName = kPacket_.m_wstrNickName;

	DO_QUERY( L"exec dbo.gup_get_unit_name", L"N\'%s\'", % kPacket_.m_wstrNickName );

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iUnitUID );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iUnitUID > 0 )
	{
		kPacket.m_iOK = NetError::NET_OK;
	}
	else
	{
		kPacket.m_iOK = NetError::ERR_RELATIONSHIP_04;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_CHECK_NICK_NAME_ACK, kPacket );
}

_IMPL_ON_FUNC( DBE_WEDDING_ITEM_INFO_REQ, KEGS_WEDDING_ITEM_INFO_REQ )
{
	KEGS_WEDDING_ITEM_INFO_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	bool bAllFind = true;

	BOOST_TEST_FOREACH( UidType, iItemUID, kPacket_.m_vecWeddingItemList )
	{
		int iWeddingUID = 0;
		DO_QUERY( L"exec dbo.P_GCouple_WeddingInvitation_SEL", L"%d", % iItemUID );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iWeddingUID );
			m_kODBC.EndFetch();
		}

		if( iWeddingUID > 0 )
		{
			KWeddingItemInfo kInfo;
			kInfo.m_iItemUID = iItemUID;
			kInfo.m_iWeddingUID = iWeddingUID;
			kPacket.m_mapWeddingItemInfo.insert( std::make_pair( kInfo.m_iItemUID, kInfo ) );
		}
		else
		{
			START_LOG( cerr, L"�����ۿ� ��Ī�� ���� ������ �����ϴ�." )
				<< BUILD_LOG( LAST_SENDER_UID )
				<< BUILD_LOG( iItemUID )
				<< END_LOG;

			bAllFind = false;
			kPacket.m_iOK = NetError::ERR_RELATIONSHIP_24;
			break;
		}
	}

	if( bAllFind == true )
	{
		kPacket.m_iOK = NetError::NET_OK;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_WEDDING_ITEM_INFO_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_WEDDING_ITEM_DELETE_REQ )
{
	KDBE_WEDDING_ITEM_DELETE_ACK kPacket;
	kPacket.m_iOK = NetError::NET_OK;
	kPacket.m_mapWeddingItemInfo = kPacket_.m_mapWeddingItemInfo;
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;

	Query_DeleteItem( kPacket_.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	SendToUser( LAST_SENDER_UID, DBE_WEDDING_ITEM_DELETE_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_BREAK_UP_REQ )
{
	KDBE_BREAK_UP_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iCost = kPacket_.m_iCost;
	kPacket.m_iMyUnitUID = kPacket_.m_iMyUnitUID;
	kPacket.m_iLoverUnitUID = kPacket_.m_iLoverUnitUID;

	CTime tNow = CTime::GetCurrentTime();
	std::wstring wstrNow = tNow.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
	std::wstring wstrTemp;
	// Ÿ���� 3 �̸� �̺�
	DO_QUERY( L"exec dbo.P_GCouple_Info_INT_UPD", L"%d, %d, %d, N\'%s\', N\'%s\', %d", 
		% 3	
		% kPacket.m_iMyUnitUID
		% kPacket.m_iLoverUnitUID
		% wstrNow
		% wstrTemp
		% 0	);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"������� ����!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iMyUnitUID )
			<< BUILD_LOG( kPacket_.m_iLoverUnitUID )
			<< BUILD_LOG( kPacket_.m_iCost )
			<< END_LOG;

		goto end_proc;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_BREAK_UP_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_ADMIN_CHANGE_COUPLE_DATE_REQ )
{
	KEGS_ADMIN_CHANGE_COUPLE_DATE_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	
	DO_QUERY( L"exec dbo.P_GCouple_Info_UPD_RegDate", L"%d, %d, %f", % kPacket_.m_iUnitUID % kPacket_.m_iLoverUnitUID % kPacket_.m_fAddRate );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"Ŀ�� ��¥ ���� ���� ġƮŰ ����" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iLoverUnitUID )
			<< BUILD_LOG( kPacket_.m_fAddRate )
			<< END_LOG;
	
		goto end_proc;
	}

	kPacket.m_iOK = NetError::NET_OK;

	START_LOG( cout, L"Ŀ�� ��¥ ���� ����!" )
		<< BUILD_LOG( kPacket.m_iOK )
		<< BUILD_LOG( kPacket_.m_iUnitUID )
		<< BUILD_LOG( kPacket_.m_iLoverUnitUID )
		<< BUILD_LOG( kPacket_.m_fAddRate )
		<< END_LOG;

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_ADMIN_CHANGE_COUPLE_DATE_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_CHANGE_LOVE_WORD_REQ )
{
	KDBE_CHANGE_LOVE_WORD_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iUseUnitUID = kPacket_.m_iUseUnitUID;
    kPacket.m_iLoverUnitUID = kPacket_.m_iLoverUnitUID;
	kPacket.m_wstrNewLoveWord = kPacket_.m_wstrNewLoveWord;
	kPacket.m_vecUpdatedInventorySlot = kPacket_.m_vecUpdatedInventorySlot;

	// ������ ����
	bool bUpdateFailed = false;
	Query_UpdateItemQuantity( kPacket_.m_iUseUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	// ���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUseUnitUID )
			<< END_LOG;

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}


	CTime tNow = CTime::GetCurrentTime();
	std::wstring wstrNow = tNow.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
	std::wstring wstrTemp;
	// Ÿ���� 3 �̸� �̺�
	DO_QUERY( L"exec dbo.P_GCouple_Info_INT_UPD", L"%d, %d, %d, N\'%s\', N\'%s\', %d", 
		% 4	
		% kPacket_.m_iUseUnitUID
		% kPacket_.m_iLoverUnitUID
		% wstrNow
		% kPacket_.m_wstrNewLoveWord
		% 0	);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"��Ī ���� ����!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUseUnitUID )
			<< BUILD_LOG( kPacket_.m_iLoverUnitUID )
			<< BUILD_LOG( kPacket_.m_wstrNewLoveWord )
			<< END_LOG;

		goto end_proc;
	}

	kPacket.m_iOK = NetError::NET_OK;

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_CHANGE_LOVE_WORD_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_INSERT_WEDDING_REWARD_REQ )
{
	KDBE_INSERT_WEDDING_REWARD_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_bTitleReward = kPacket_.m_bTitleReward;
	kPacket.m_iTitleRewardStep = kPacket_.m_iTitleRewardStep;

	// ���� �߼� sp
	KPostItemInfo kInfo;
	kInfo.m_iFromUnitUID = kPacket_.kPostReq.m_iFromUnitUID;
	kInfo.m_iToUnitUID   = kPacket_.kPostReq.m_iToUnitUID;
	kInfo.m_cScriptType  = kPacket_.kPostReq.m_iRewardType;
	kInfo.m_iScriptIndex = kPacket_.kPostReq.m_iRewardID;
	kInfo.m_iQuantity	   = 1;

	DO_QUERY( L"exec dbo.gup_insert_post_item", L"%d, %d, %d, %d, %d, N\'%s\'",
		% kInfo.m_iFromUnitUID
		% kInfo.m_iToUnitUID
		% kInfo.m_iQuantity
		% static_cast<int>(kInfo.m_cScriptType)
		% kInfo.m_iScriptIndex
		% kInfo.m_wstrMessage
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK
			>> kInfo.m_iPostNo
			>> kInfo.m_wstrRegDate );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK == NetError::NET_OK )
	{
		// ���� �߼� ��� ����
		KDBE_INSERT_REWARD_TO_POST_ACK kPacket;
		kPacket.m_iOK = NetError::NET_OK;
		kPacket.m_iRewardLetter = kInfo;
		SendToUser( LAST_SENDER_UID, DBE_INSERT_REWARD_TO_POST_ACK, kPacket );

		// DB �ο� ���� ����
		int iType = 0;
		if( kPacket_.m_bTitleReward == true )
		{
			iType = 6;	// Ÿ��Ʋ ���� ���� ������Ʈ
		}
		else
		{
			iType = 5;	// �Ϲ� ����
		}

		CTime tCurr = CTime::GetCurrentTime();
		std::wstring wstrNow = tCurr.Format( _T( "%Y-%m-%d %H:%M:%S" ) );
		std::wstring wstrLoveWord;

		kPacket.m_iOK = NetError::ERR_ODBC_01;
		DO_QUERY( L"exec dbo.P_GCouple_Info_INT_UPD", L"%d, %d, %d, N\'%s\', N\'%s\', %d", 
			% iType
			% kPacket_.m_iUnitUID
			% kPacket_.m_iLoverUnitUID
			% wstrNow
			% wstrLoveWord
			% kPacket_.m_iTitleRewardStep );

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( kPacket.m_iOK );

			m_kODBC.EndFetch();
		}

		if( kPacket.m_iOK != NetError::NET_OK )
		{
			START_LOG( cerr, L"�ο� ���� ���� ����!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iLoverUnitUID )
			<< BUILD_LOG( iType )
			<< BUILD_LOG( kPacket_.m_iTitleRewardStep )
			<< END_LOG;
		}
	}
end_proc:
	SendToUser( LAST_SENDER_UID, DBE_INSERT_WEDDING_REWARD_ACK, kPacket );
}

#endif SERV_RELATIONSHIP_SYSTEM
//}

//{{ 2013. 05. 15	������	������ ����
#ifdef SERV_NEW_ITEM_SYSTEM_2013_05
IMPL_ON_FUNC( DBE_ITEM_EVALUATE_REQ )
{
	KDBE_ITEM_EVALUATE_ACK kPacket;
	kPacket.m_vecRandomSocket = kPacket_.m_vecRandomSocket;
	kPacket.m_iItemUID = kPacket_.m_iItemUID;
	kPacket.m_iCost = kPacket_.m_iCost;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	int arrSocketInfo[CXSLSocketItem::RSC_MAX] = {0,}; // DB ���̺��� �������̺� ����	

	std::vector< int >::const_iterator vit = kPacket.m_vecRandomSocket.begin();
	for( int iIdx = 0; iIdx < CXSLSocketItem::RSC_MAX; ++iIdx )
	{		
		if( vit == kPacket.m_vecRandomSocket.end() )
			continue;

		arrSocketInfo[iIdx] = *vit;
		++vit;
	}

	// ���� ���� ������Ʈ
	DO_QUERY( L"exec dbo.P_GItemSocket_Random_INT_UPD", L"%d, %d, %d, %d, %d, %d, %d",
		% kPacket_.m_iUnitUID		// @iUnitUID bigint
		% kPacket_.m_iItemUID		// @iItemUID bigint
		% arrSocketInfo[0]			// @iSoket1 smallint
		% arrSocketInfo[1]			// @iSoket2 smallint
		% arrSocketInfo[2]			// @iSoket3 smallint
		% arrSocketInfo[3]			// @iSoket4 smallint
		% arrSocketInfo[4]			// @iSoket4 smallint
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"������ ���� ���� ������Ʈ ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iItemUID )
			<< BUILD_LOG( arrSocketInfo[0] )
			<< BUILD_LOG( arrSocketInfo[1] )
			<< BUILD_LOG( arrSocketInfo[2] )
			<< BUILD_LOG( arrSocketInfo[3] )
			<< BUILD_LOG( arrSocketInfo[4] )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_EVALUATE_ITEM_04;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_ITEM_EVALUATE_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_RESTORE_ITEM_EVALUATE_REQ )
{
	KDBE_RESTORE_ITEM_EVALUATE_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iTargetItemUID = kPacket_.m_iTargetItemUID;
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	int arrSocketInfo[CXSLSocketItem::RSC_MAX] = {0,}; // DB ���̺��� �������̺� ����	

	// ���� ���� ������Ʈ
	DO_QUERY( L"exec dbo.P_GItemSocket_Random_INT_UPD", L"%d, %d, %d, %d, %d, %d, %d",
		% kPacket_.m_iUnitUID		// @iUnitUID bigint
		% kPacket_.m_iTargetItemUID	// @iItemUID bigint
		% arrSocketInfo[0]			// @iSoket1 smallint
		% arrSocketInfo[1]			// @iSoket2 smallint
		% arrSocketInfo[2]			// @iSoket3 smallint
		% arrSocketInfo[3]			// @iSoket4 smallint
		% arrSocketInfo[4]			// @iSoket4 smallint
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"������ ���� ���� ������Ʈ ����." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iTargetItemUID )
			<< BUILD_LOG( arrSocketInfo[0] )
			<< BUILD_LOG( arrSocketInfo[1] )
			<< BUILD_LOG( arrSocketInfo[2] )
			<< BUILD_LOG( arrSocketInfo[3] )
			<< BUILD_LOG( arrSocketInfo[4] )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_EVALUATE_ITEM_04;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_RESTORE_ITEM_EVALUATE_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_ITEM_CONVERT_REQ )
{
	KDBE_ITEM_CONVERT_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_iCommissionED = kPacket_.m_iCommissionED;
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	if( Query_InsertItemList( SEnum::GIR_RANDOM_ITEM, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo, false ) == false )
	{
		START_LOG( cerr, L"������ ��ȯ�� ������ DB �߰� ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_RANDOM_ITEM_06;
	}
	else
	{
		kPacket.m_iOK = NetError::NET_OK;
	}

	SendToUser( LAST_SENDER_UID, DBE_ITEM_CONVERT_ACK, kPacket );
}
#endif SERV_NEW_ITEM_SYSTEM_2013_05
//}}

#ifdef SERV_RELATIONSHIP_SYSTEM_LAUNCHING_EVENT// �۾���¥: 2013-05-13	// �ڼ���
IMPL_ON_FUNC( DBE_SEND_LOVE_LETTER_EVENT_REQ )
{
	KDBE_SEND_LOVE_LETTER_EVENT_ACK kPacket;
	kPacket.m_iOK						= NetError::ERR_ODBC_01;
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;

	// ������ ����
	bool bUpdateFailed = false;
	Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	// ���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}

	// ������ �����ؾ����� ���� ���� ���
	if( ( kPacket.m_kItemQuantityUpdate.m_vecDeleted.empty() == true ) &&
		( bUpdateFailed == false )
		)
	{
		DO_QUERY( L"exec dbo.P_GEvent_Letter_INT", L"%d, N\'%s\'",
			% kPacket_.m_iUnitUID
			% kPacket_.m_wstrMessage
			);

		if( m_kODBC.BeginFetch() )
		{
			bool bIsFirstSend;
			FETCH_DATA( kPacket.m_iOK
				>> bIsFirstSend
				);
			m_kODBC.EndFetch();

			if( kPacket.m_iOK == NetError::NET_OK )
			{
				if( bIsFirstSend == true )
				{
					kPacket.m_iOK = NetError::ERR_REQUEST_OF_LOVE_01;
				}
			}
			else
			{
				START_LOG( cerr, L"�Ƿ��� ���� ���� DB ���� ����" )
					<< BUILD_LOG( kPacket_.m_iUnitUID )
					<< END_LOG;
			}
		}
	}
	else
	{
		kPacket.m_iOK = NetError::ERR_REQUEST_OF_LOVE_04;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_SEND_LOVE_LETTER_EVENT_ACK, kPacket );
}
#endif // SERV_RELATIONSHIP_SYSTEM_LAUNCHING_EVENT

#ifdef SERV_JUMPING_CHARACTER// �۾���¥: 2013-07-10	// �ڼ���
IMPL_ON_FUNC( DBE_JUMPING_CHARACTER_UPDATE_REQ )
{
	// ���� ���� SP�� ������ �Ѵ�.

	KDBE_JUMPING_CHARACTER_UPDATE_ACK kPacket;
	kPacket.m_iUnitUID = kPacket_.m_iUnitUID;

	// ���� ���
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	DO_QUERY( L"exec dbo.P_GEvent_Jumping_INT", L"%d, %d, N\'%s\'",
		% LAST_SENDER_UID
		% kPacket_.m_iUnitUID
		% kPacket_.m_wstrRegDate
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		goto end_proc;
	}

	kPacket.m_iOK					= NetError::NET_OK;
	kPacket.m_iUnitUID				= kPacket_.m_iUnitUID;
	kPacket.m_iLevel				= kPacket_.m_iLevel;
	kPacket.m_cExpandedMaxPageNum	= -1;
	kPacket.m_iClass				= -1;

	// 1. ����� ��Ʈ ������ ó��
	if( 0 <= kPacket_.m_cExpandedMaxPageNum )
	{
		DO_QUERY( L"exec dbo.gup_insert_notecnt", L"%d, %d",
			% kPacket_.m_iUnitUID
			% static_cast<int>( kPacket_.m_cExpandedMaxPageNum )
			);

		int iOK = NetError::ERR_ODBC_01;

		if( m_kODBC.BeginFetch() )
		{
			FETCH_DATA( iOK );
			m_kODBC.EndFetch();
		}

		if( iOK == NetError::NET_OK )
		{
			kPacket.m_cExpandedMaxPageNum = kPacket_.m_cExpandedMaxPageNum;

			// �α�
			DO_QUERY( L"exec dbo.P_GJumpingLog_INT", L"%d, %d, %d, N\'%s\'",
				% LAST_SENDER_UID
				% kPacket_.m_iUnitUID
				% 1
				% kPacket_.m_wstrRegDate
				);

			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( iOK );
				m_kODBC.EndFetch();
			}
		}
	}

	// 2. Class �� �⺻ ��ų ó��
	{
		int iOK = NetError::NET_OK;
		const int iMax = static_cast<int>( kPacket_.m_vecNewDefaultSkill.size() / 2 );
		for( int i = 0; i < iMax; ++i )
		{
			const int iNewDefaultSkill1 = kPacket_.m_vecNewDefaultSkill[i * 2];
			const int iNewDefaultSkill2 = kPacket_.m_vecNewDefaultSkill[i * 2 + 1];

			DO_QUERY( L"exec P_GUnit_UPD_UnitClass", L"%d, %d, %d, %d",
				% kPacket_.m_iUnitUID
				% kPacket_.m_iClass
				% iNewDefaultSkill1
				% iNewDefaultSkill2
				);

			int iTemp = NetError::ERR_ODBC_01;

			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( iTemp );
				m_kODBC.EndFetch();
			}

			if( iTemp == NetError::NET_OK )
			{
				kPacket.m_iClass = kPacket_.m_iClass;
				kPacket.m_vecNewDefaultSkill.push_back( iNewDefaultSkill1 );
				kPacket.m_vecNewDefaultSkill.push_back( iNewDefaultSkill2 );
			}
			else
			{
				iOK = iTemp;
			}
		}

		if( iOK == NetError::NET_OK )
		{
			// �α�
			DO_QUERY( L"exec dbo.P_GJumpingLog_INT", L"%d, %d, %d, N\'%s\'",
				% LAST_SENDER_UID
				% kPacket_.m_iUnitUID
				% 2
				% kPacket_.m_wstrRegDate
				);

			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( iOK );
				m_kODBC.EndFetch();
			}
		}
	}

	// 3. ���� ��ų ó��
	{
		int iOK = NetError::NET_OK;
		BOOST_TEST_FOREACH( short, iSkillID, kPacket_.m_vecUnSealedSkill )
		{
			DO_QUERY( L"exec dbo.gup_insert_unsealed_skill", L"%d, %d",
				% kPacket_.m_iUnitUID
				% iSkillID
				);

			int iTemp = NetError::ERR_ODBC_01;
			
			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( iTemp );
				m_kODBC.EndFetch();
			}

			if( iTemp == NetError::NET_OK )
			{
				kPacket.m_vecUnSealedSkill.push_back( iSkillID );
			}
			else
			{
				iOK = iTemp;
			}
		}

		if( iOK == NetError::NET_OK )
		{
			// �α�
			DO_QUERY( L"exec dbo.P_GJumpingLog_INT", L"%d, %d, %d, N\'%s\'",
				% LAST_SENDER_UID
				% kPacket_.m_iUnitUID
				% 3
				% kPacket_.m_wstrRegDate
				);

			if( m_kODBC.BeginFetch() )
			{
				FETCH_DATA( iOK );
				m_kODBC.EndFetch();
			}
		}
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_JUMPING_CHARACTER_UPDATE_ACK, kPacket );
}
#endif // SERV_JUMPING_CHARACTER

#ifdef SERV_PERIOD_PET
IMPL_ON_FUNC( DBE_RELEASE_PET_REQ )
{
	KEGS_RELEASE_PET_ACK kPacket;
	kPacket.m_iOK			= NetError::ERR_ODBC_01;
	kPacket.m_iPetUID		= kPacket_.m_iPetUID;

	DO_QUERY( L"exec dbo.P_GPet_Info_DEL", L"%d",
		% kPacket_.m_iPetUID
		);

	if( m_kODBC.BeginFetch() == true )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"�� �����ֱ� ����!" )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iPetUID )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_RIDING_PET_11;	// �� �����ֱ⿡ �����Ͽ����ϴ�.
		goto end_proc;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_RELEASE_PET_ACK, kPacket );
}
#endif SERV_PERIOD_PET

#ifdef SERV_RECRUIT_EVENT_BASE
IMPL_ON_FUNC( DBE_USE_RECRUIT_TICKET_REQ )
{
	KDBE_USE_RECRUIT_TICKET_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	CTime tRegDate;
	std::wstring wstrRegDate;
	CTime tEventDate;
	KncUtil::ConvertStringToCTime( STRING_RECRUIT_EVENT_DATE, tEventDate );

	DO_QUERY( L"exec dbo.gup_get_recommend_regdate", L"%d",
		% kPacket_.m_iUnitUID
		);
	if( m_kODBC.BeginFetch() )
	{
		kPacket.m_iOK = NetError::NET_OK;

		FETCH_DATA( wstrRegDate );
		m_kODBC.EndFetch();

		KncUtil::ConvertStringToCTime( wstrRegDate, tRegDate );
	}

	if( kPacket.m_iOK != NetError::NET_OK ||
		tRegDate < tEventDate )
	{
		START_LOG( cerr, L"��õ���� ����� �ƴմϴ�." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_RECOMMEND_USER_02;
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_USE_RECRUIT_TICKET_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_REGISTER_RECRUITER_REQ )
{
	KDBE_REGISTER_RECRUITER_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	CTime tRegDate;
	std::wstring wstrRegDate;
	CTime tEventDate;
	KncUtil::ConvertStringToCTime( STRING_RECRUIT_EVENT_DATE, tEventDate );

	// ��õ�� ��¥ �˻�
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	DO_QUERY( L"exec dbo.gup_get_recommend_nickname", L"%d",
		% kPacket_.m_wstrNickname
		);
	if( m_kODBC.BeginFetch() )
	{
		kPacket.m_iOK = NetError::NET_OK;

		FETCH_DATA( wstrRegDate );
		m_kODBC.EndFetch();

		KncUtil::ConvertStringToCTime( wstrRegDate, tRegDate );
	}
	if( kPacket.m_iOK != NetError::NET_OK ||
		tRegDate > tEventDate )
	{
		START_LOG( cerr, L"��õ ������ ������ �ƴմϴ�." )
			<< BUILD_LOG( kPacket.m_iOK )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_RECOMMEND_USER_04;
		goto end_proc;
	}

	// ����
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	DO_QUERY( L"exec dbo.gup_insert_recommend_new", L"%d, N\'%s\'",
		% kPacket_.m_iUnitUID
		% kPacket_.m_wstrNickname
		);

	if( m_kODBC.BeginFetch() )
	{
		// ���߿� ��� �ҷ����� sp�� ���Ƿ� iRecruiterUnitUID �� ���⼱ ���� �޾ƿ� �ʿ䰡 ���� ���̱� �ߴ�.
		UidType iRecruiterUnitUID;
		FETCH_DATA( kPacket.m_iOK
			>> iRecruiterUnitUID
			);
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
	{
		switch( kPacket.m_iOK )
		{
		case -1:
			kPacket.m_iOK = NetError::ERR_RECOMMEND_USER_00;
			break;
		case -3:
			kPacket.m_iOK = NetError::ERR_RECOMMEND_USER_03;
			break;
		case -4:
			kPacket.m_iOK = NetError::ERR_RECOMMEND_USER_01;
			break;
		default:
			kPacket.m_iOK = NetError::ERR_ODBC_01;
			break;
		}

		goto end_proc;
	}

	// ����õ���� ��õ�� ��Ͽ� �߰��� �� �ֱ� ���� �κ�
	DO_QUERY( L"exec dbo.gup_get_recommend_newuid", L"%d", % kPacket_.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		FETCH_DATA( kPacket.m_kRecruiterUnitInfo.m_iUnitUID
			>> kPacket.m_kRecruiterUnitInfo.m_ucLevel
			>> kPacket.m_kRecruiterUnitInfo.m_cUnitClass
			>> kPacket.m_kRecruiterUnitInfo.m_wstrNickName );
	}

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_REGISTER_RECRUITER_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_GET_RECRUIT_RECRUITER_LIST_REQ )
{
	KEGS_GET_RECRUIT_RECRUITER_LIST_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec dbo.gup_get_recommend_newuid", L"%d", % kPacket_.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		KRecommendUserInfo kInfo;
		kInfo.m_wstrRecommendDate = L"";

		FETCH_DATA( kInfo.m_iUnitUID
			>> kInfo.m_ucLevel
			>> kInfo.m_cUnitClass
			>> kInfo.m_wstrNickName );

		kPacket.m_vecRecruiterUnitInfo.push_back( kInfo );
	}

	DO_QUERY( L"exec dbo.gup_get_recommend_olduid", L"%d", % kPacket_.m_iUnitUID );
	while( m_kODBC.Fetch() )
	{
		KRecommendUserInfo kInfo;
		kInfo.m_wstrRecommendDate = L"";

		FETCH_DATA( kInfo.m_iUnitUID
			>> kInfo.m_ucLevel
			>> kInfo.m_cUnitClass
			>> kInfo.m_wstrNickName );

		kPacket.m_vecRecruitUnitInfo.push_back( kInfo );
	}

	kPacket.m_iOK = NetError::NET_OK;

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_GET_RECRUIT_RECRUITER_LIST_ACK, kPacket );
}


#endif SERV_RECRUIT_EVENT_BASE

#ifdef SERV_NEW_YEAR_EVENT_2014
IMPL_ON_FUNC( DBE_2013_EVENT_MISSION_COMPLETE_REQ )
{
	KDBE_2013_EVENT_MISSION_COMPLETE_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_ucLevel = kPacket_.m_ucLevel;

	DO_QUERY( L"exec dbo.P_GEventEndYear_UPD", L"%d, %d", % kPacket_.m_iUnitUID % kPacket_.m_ucLevel );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
		kPacket.m_iOK = NetError::ERR_QUEST_09;

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_2013_EVENT_MISSION_COMPLETE_ACK, kPacket );
}

IMPL_ON_FUNC( DBE_2014_EVENT_MISSION_COMPLETE_REQ )
{
	KEGS_2014_EVENT_MISSION_COMPLETE_ACK kPacket;

	DO_QUERY( L"exec dbo.P_GEventNewYear_SET", L"%d, %d, %d", % kPacket_.m_iUserUID % kPacket_.m_iUnitUID % kPacket_.m_iNewYearMissionStepID );
	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( kPacket.m_iOK );
		m_kODBC.EndFetch();
	}

	if( kPacket.m_iOK != NetError::NET_OK )
		kPacket.m_iOK = NetError::ERR_QUEST_09;

end_proc:
	SendToUser( LAST_SENDER_UID, DBE_2014_EVENT_MISSION_COMPLETE_ACK, kPacket );
}
#endif SERV_NEW_YEAR_EVENT_2014

#ifdef SERV_READY_TO_SOSUN_EVENT
IMPL_ON_FUNC( DBE_READY_TO_SOSUN_EVENT_REQ )
{
	KDBE_READY_TO_SOSUN_EVENT_ACK kPacket;
	kPacket.m_iOK = NetError::ERR_ODBC_01;
	kPacket.m_mapInsertedItem			= kPacket_.m_mapInsertedItem;
	kPacket.m_vecUpdatedInventorySlot	= kPacket_.m_vecUpdatedInventorySlot;
	kPacket.m_iFirstUnitClass			= kPacket_.m_iFirstUnitClass;

	bool bUpdateFailed = false;
	LIF( Query_UpdateItemQuantity( kPacket_.m_iUnitUID, kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange, kPacket.m_kItemQuantityUpdate.m_mapQuantityChange, bUpdateFailed ) );
	Query_DeleteItem( kPacket_.m_kItemQuantityUpdate.m_vecDeleted, kPacket.m_kItemQuantityUpdate.m_vecDeleted );

	//{{ 2008. 10. 23  ������	���а� ������Ʈ ����
	if( bUpdateFailed )
	{
		START_LOG( cout, L"���а� ������Ʈ ����!" )
			<< BUILD_LOG( kPacket_.m_iUnitUID );

		std::map< UidType, int >::const_iterator mitQC = kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.begin();
		for( ; mitQC != kPacket_.m_kItemQuantityUpdate.m_mapQuantityChange.end(); ++mitQC )
		{
			START_LOG( cout, L"������ ����" )
				<< BUILD_LOG( mitQC->first )
				<< BUILD_LOG( mitQC->second );
		}
	}
	//}}

	//{{ 2010. 9. 8	������	������ ȹ�� ����
#ifdef SERV_GET_ITEM_REASON
	if( Query_InsertItemList( SEnum::GIR_ENCHANTMENT_EXTRACTION_ITEM, kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#else
	if( Query_InsertItemList( kPacket_.m_iUnitUID, kPacket_.m_vecItemInfo, kPacket.m_mapItemInfo ) == false )
#endif SERV_GET_ITEM_REASON
		//}}
	{
		START_LOG( cerr, L"�ƶ� 1�� ���� �̸� ���߱� �̺�Ʈ ���� ������ ȹ�� ����" )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< END_LOG;

		kPacket.m_iOK = NetError::ERR_RANDOM_ITEM_06;
	}
	else
	{
		kPacket.m_iOK = NetError::NET_OK;
	}

	SendToUser( LAST_SENDER_UID, DBE_READY_TO_SOSUN_EVENT_ACK, kPacket );
}
#endif SERV_READY_TO_SOSUN_EVENT

#ifdef SERV_GLOBAL_MISSION_MANAGER
IMPL_ON_FUNC( DBE_REGIST_GLOBAL_MISSION_CLEAR_NOT )
{
	int iOK = NetError::ERR_ODBC_01;

	DO_QUERY( L"exec dbo.P_GEvent_GlobalMission_INS", L"N\'%s\', %d",
		% kPacket_.m_wstrGlobalMissionDay 
		% kPacket_.m_iUnitUID
		);

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}

end_proc:
	return;
}
#endif SERV_GLOBAL_MISSION_MANAGER
