#include "StdAfx.h"
#include ".\ktdxlog.h"

CKTDXLog g_CKTDXLog;

CKTDXLog::CKTDXLog(void)
{
	m_LastErrorLog = "";
}

CKTDXLog::~CKTDXLog(void)
{
	m_StateLog.clear();
	m_ErrorLog.clear();

	for(UINT i=0; i<m_queLastErrorLog.size(); ++i)
		m_queLastErrorLog.pop();	
}

void CKTDXLog::PushStateLog( const WCHAR* statLog )
{
	string logstr;
	ConvertWCHARToChar( logstr, statLog );
	PushStateLog( logstr.c_str() );
}

void CKTDXLog::PushErrorLog( int errNum, const WCHAR* errorLog )
{
	string logstr;
	ConvertWCHARToChar( logstr, errorLog );
	PushErrorLog( errNum, logstr.c_str() );
}

void CKTDXLog::PushStateLog( const char* statLog )
{
	time_t t0 = time(0);

#ifdef	CONVERSION_VS
	struct tm t;
	localtime_s( &t, &t0);
	stringstream strstm;
	strstm << 1900+t.tm_year << "/" << 1+t.tm_mon << "/" << t.tm_mday << ", " << t.tm_hour << ":" << t.tm_min << ":" << t.tm_sec << " > " << statLog;
#else	CONVERSION_VS
	tm* t = localtime(&t0);
	stringstream strstm;
	strstm << 1900+t->tm_year << "/" << 1+t->tm_mon << "/" << t->tm_mday << ", " << t->tm_hour << ":" << t->tm_min << ":" << t->tm_sec << " > " << statLog;
#endif	CONVERSION_VS
	
	m_StateLog.push_back( strstm.str().c_str() );
}

void CKTDXLog::PushErrorLog( int errNum )
{
	time_t t0 = time(0);
#ifdef	CONVERSION_VS
	struct tm t;
	localtime_s( &t, &t0);
	stringstream strstm;
	strstm << 1900+t.tm_year << "/" << 1+t.tm_mon << "/" << t.tm_mday << ", " << t.tm_hour << ":" << t.tm_min << ":" << t.tm_sec << " > " << "ErrorEnum(" << errNum << ")";
#else	CONVERSION_VS
	tm* t = localtime(&t0);
	stringstream strstm;
	strstm << 1900+t->tm_year << "/" << 1+t->tm_mon << "/" << t->tm_mday << ", " << t->tm_hour << ":" << t->tm_min << ":" << t->tm_sec << " > " << "ErrorEnum(" << errNum << ")";
#endif	CONVERSION_VS

	m_ErrorLog.push_back( strstm.str().c_str() );
}

void CKTDXLog::PushErrorLog( int errNum, const char* errorLog )
{
	time_t t0 = time(0);
#ifdef	CONVERSION_VS
	struct tm t;
	localtime_s( &t, &t0);
	stringstream strstm;
	strstm << 1900+t.tm_year << "/" << 1+t.tm_mon << "/" << t.tm_mday << ", " << t.tm_hour << ":" << t.tm_min << ":" << t.tm_sec << " > " << "ErrorEnum(" << errNum << "),\t" << "ErrorMsg(\"" << errorLog << "\")";
#else	CONVERSION_VS
	tm* t = localtime(&t0);
	stringstream strstm;
	strstm << 1900+t->tm_year << "/" << 1+t->tm_mon << "/" << t->tm_mday << ", " << t->tm_hour << ":" << t->tm_min << ":" << t->tm_sec << " > " << "ErrorEnum(" << errNum << "),\t" << "ErrorMsg(\"" << errorLog << "\")";
#endif	CONVERSION_VS

	m_ErrorLog.push_back( strstm.str().c_str() );
}

void CKTDXLog::SetLastErrorLog( const WCHAR* statLog )
{ 
	ConvertWCHARToChar( m_LastErrorLog, statLog ); 
}

void CKTDXLog::StateLogToFile( const char* fileName )
{
	for( int i = 0; i < (int)m_StateLog.size(); i++ )
	{
		KLOG(fileName) << m_StateLog[i].c_str() << fileout;
	}
	KLOG(fileName) << "_____LOG_END____" << fileout;
	KLOG(fileName) << "\n\n\n\n\n\n\n" << fileout;
}

void CKTDXLog::ErrorLogToFile( const char* fileName, const char* systemInfo )
{
	for( int i = 0; i < (int)m_ErrorLog.size(); i++ )
	{
		KLOG(fileName) << m_ErrorLog[i].c_str() << fileout;
	}

	KLOG(fileName) << "LastErrorLog : " << m_LastErrorLog.c_str() << fileout;
#if 0 
	KLOG(fileName) << "\n" << fileout;
	for(UINT i=0; i<m_queLastErrorLog.size(); ++i)
	{
		KLOG(fileName) << "LastErrorLog(" << i << ") : " << DeQueLastErrorLog() << fileout;
	}	
#endif
	KLOG(fileName) << "_____LOG_END____" << fileout;

	if( systemInfo != NULL )
	{
		KLOG(fileName) << "------INFO------" << fileout;
		KLOG(fileName) << systemInfo << fileout;
	}
	
	
	KLOG(fileName) << "\n\n\n\n\n\n\n" << fileout;


}

#if 0
void CKTDXLog::EnQueLastErrorLog( const WCHAR* errorLog )
{
	if(m_queLastErrorLog.size() > 5)
	{
		m_queLastErrorLog.pop();		
	}

	time_t t0 = time(0);

#ifdef	CONVERSION_VS
	struct tm t;
	localtime_s( &t, &t0);
	wstringstream wstrstm;
	wstrstm << 1900+t.tm_year << "/" << 1+t.tm_mon << "/" << t.tm_mday << ", " << t.tm_hour << ":" << t.tm_min << ":" << t.tm_sec << " > " << "ErrorMsg(\"" << errorLog << "\")";
#else	CONVERSION_VS
	tm* t = localtime(&t0);
	wstringstream wstrstm;
	wstrstm << 1900+t->tm_year << "/" << 1+t->tm_mon << "/" << t->tm_mday << ", " << t->tm_hour << ":" << t->tm_min << ":" << t->tm_sec << " > " << "ErrorMsg(\"" << errorLog << "\")";
#endif	CONVERSION_VS

	m_queLastErrorLog.push( wstrstm.str().c_str() );
}

const char* CKTDXLog::DeQueLastErrorLog()
{
	if(m_queLastErrorLog.size() > 0)
	{
		string strErrorLog = "";
		wstring wstrErrorLog = m_queLastErrorLog.back();
        m_queLastErrorLog.pop();


		ConvertWCHARToChar( strErrorLog, wstrErrorLog.c_str() ); 

		return strErrorLog.c_str();
	}

	return NULL;
}
#endif


