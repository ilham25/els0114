#pragma once

class CKTDXLog
{
	public:
		CKTDXLog(void);
		~CKTDXLog(void);

		void PushStateLog( const WCHAR* statLog );
		void PushErrorLog( int errNum, const WCHAR* errorLog );

		void PushStateLog( const char* statLog );
		void PushErrorLog( int errNum );
		void PushErrorLog( int errNum, const char* errorLog );

		__forceinline void SetLastErrorLog( const char* statLog ) { m_LastErrorLog = statLog; }
		void SetLastErrorLog( const WCHAR* statLog );

		void StateLogToFile( const char* fileName );
		void ErrorLogToFile( const char* fileName, const char* systemInfo = NULL );
        
        int GetStateLogSize() { return m_StateLog.size(); }
        int GetErrorLogSize() { return m_ErrorLog.size(); }
        const char *GetStateLogStr(int inx) { return m_StateLog[inx].c_str(); }
        const char *GetErrorLogStr(int inx) { return m_ErrorLog[inx].c_str(); }
        const char *GetLastErrorLog() { return m_LastErrorLog.c_str(); }

#if 0
		// 사용하지 않으므로 제거한다.
		void EnQueLastErrorLog( const WCHAR* errorLog );
		const char *DeQueLastErrorLog();
#endif

	private:
		vector<string>		m_StateLog;
		vector<string>		m_ErrorLog;
		string				m_LastErrorLog;

		std::queue<wstring>	m_queLastErrorLog;
};

extern CKTDXLog g_CKTDXLog;
