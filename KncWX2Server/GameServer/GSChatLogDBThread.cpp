#include <winsock2.h>
#include <iomanip>
#include <sstream>

#include <dbg/dbg.hpp>

#include "GSChatLogDBThread.h"
#include "GameServer.h"
#include "NetError.h"

#ifdef SERV_RECORD_CHAT

// [����!] DO_QUERY() ���� �������ڼ� ��Ȯ�ϰ� ���� ������ ���� ũ���� ���ϴ�!

//{{ Iruha : 2026-08-27 // VS2010 port: ImplementDBThread is undefined everywhere in this tree.
// VC7.1 silently parsed it as an implicit-int prototype (harmless, unused); VC10 makes that
// a hard error (C4430). DeclareDBThread already implements the constructor inline, so this
// pairing macro never did anything. Most sibling *DBThread.cpp files already comment it out.
//ImplementDBThread( KGSChatLogDBThread );
//}}
ImplPfID( KGSChatLogDBThread, PI_GS_CHAT_LOG_DB );

#define CLASS_TYPE KGSChatLogDBThread

IMPL_PROFILER_DUMP( KGSChatLogDBThread )
{
	for( unsigned int ui = 0; ui < vecDump.size(); ++ui )
	{
		unsigned int iAvg = 0;
		if( vecDump[ui].m_iQueryCount > 0 )	iAvg = vecDump[ui].m_iTotalTime / vecDump[ui].m_iQueryCount;		

		DO_QUERY_NO_PROFILE( L"exec dbo.up_insert_querystats", L"N\'%s\', %d, %d, %d, %d, %d, %d",
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


KGSChatLogDBThread::~KGSChatLogDBThread(void)
{
}



void KGSChatLogDBThread::ProcessEvent( const KEventPtr& spEvent_ )
{
	switch( spEvent_->m_usEventID )
	{
		CASE( DCHATLOG_RECORD_CHAT_NOT );


	default:
		START_LOG( cerr, L"�̺�Ʈ �ڵ鷯�� ���ǵ��� �ʾ���. " << spEvent_->GetIDStr() );
	}

	//////////////////////////////////////////////////////////////////////////
#ifdef SP_PROFILER
	CALL_PROFILER_DUMP( SP_PROFILER_DUMP_TIME );
#endif SP_PROFILER
	//////////////////////////////////////////////////////////////////////////
}

//{{ 2010.11.26	��ȿ��	SQL_Injection ���� �ذ� 
#include <boost/algorithm/string/replace.hpp>
//}}

IMPL_ON_FUNC( DCHATLOG_RECORD_CHAT_NOT )
{
	// ���� ���� ���Ӽ��ο� �� �˸� //
	// ���� �� ������ �α׷� �����. 

	int iOK = NetError::ERR_ODBC_01;

	//{{ 2010.11.26	��ȿ��	SQL_Injection ���� �ذ� 
	std::wstring wstr_tmp = kPacket_.m_wstrChatText;
    boost::replace_all( wstr_tmp, L"'",L"''");
	
	DO_QUERY( L"exec dbo.up_insert_chat", L"%d, %d, %d, N\'%s\', %d", 
		% kPacket_.m_iAccountUID 
		% kPacket_.m_iUnitUID 
		% kPacket_.m_iServerGroupID
		% wstr_tmp	// ���� //
		% kPacket_.m_iChatType
		);
	//}}

	if( m_kODBC.BeginFetch() )
	{
		FETCH_DATA( iOK );
		m_kODBC.EndFetch();
	}


end_proc:
	if( iOK != NetError::NET_OK )
	{
		START_LOG( cerr, L"ä�� �α� ��� ����.!" )
			<< BUILD_LOG( iOK )
			<< BUILD_LOG( kPacket_.m_iAccountUID )
			<< BUILD_LOG( kPacket_.m_iUnitUID )
			<< BUILD_LOG( kPacket_.m_iServerGroupID )
			<< BUILD_LOG( kPacket_.m_wstrChatText )
			<< BUILD_LOG( kPacket_.m_iChatType )
			<< END_LOG;
	}
}

#endif SERV_RECORD_CHAT