#pragma once

struct COLLECT_SERVER_PACKET
{
	unsigned short	m_iEventID;
	unsigned short	m_usType;
	KSerBuffer*		m_pSerBuff;
	std::string		m_strHackInfo;
	bool			m_bSend;
	bool			m_bExit;
};

class CX2CollectHackInfo :
	public KJohnThread
{
public:
	CX2CollectHackInfo(void);
	virtual ~CX2CollectHackInfo(void);

	virtual bool BeginThread();
	virtual void EndThread(DWORD dwTimeout_ = INFINITE);
	virtual DWORD RunThread();

private:
	bool ConnectCollectServer();
	void SendCSImg( int iCount, std::string &strFileName, bool bExit );
	void SendCSEvent( unsigned short iEventId, unsigned short usType, std::string &strHackInfo, bool bExit );
	void ReceiveCSEvent( COLLECT_SERVER_PACKET &spEvent );
	void DoProcessEvent();
	void LoadHackImg( std::string &strFileName );

public:
	void ClearQueue();
	void EnQueue( unsigned short iEventID, KSerBuffer *pBuff, bool bSend, unsigned short usType, std::string strHackInfo, bool bExit = false );
	bool UIServerEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

public:
	bool										m_bSendPacket;

private:
	CRITICAL_SECTION		                    m_csEventQueue;
	std::queue<COLLECT_SERVER_PACKET>           m_queEvent;
	CX2ServerProtocol*							m_pCollectServer;
	char*										m_pHackImgBuf;
	long										m_lTotalSize;
	int											m_iImageOffset;
	bool										m_bRunThread;
};

