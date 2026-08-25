#include ".\kautopath.h"

//{{ 2009. 7. 13  최육사	하드코딩 제거
static std::vector< std::string > g_vecAddPath; // cpp파일에 선언하여 외부접근차단

void KAutoPath::AddPath( const std::string& strPath )
{
	g_vecAddPath.push_back( strPath );
}
//}}

KAutoPath::KAutoPath()
{
	char szCurDir[MAX_PATH] = "";
	std::string strCurDir;
	//char cFolder;

	// 실행경로는 기본적으로 넣어준다!
	GetCurrentDirectoryA( MAX_PATH, szCurDir );
	//cFolder = szCurDir[0];


	strCurDir = szCurDir;
	strCurDir += "\\";
	m_vecDirectory.push_back( strCurDir );
	InitDirectory( strCurDir );

	//{{ 2009. 7. 13  최육사	하드코딩 제거

	// 그외에 추가 경로도 설정한다!
	std::vector< std::string >::const_iterator vit;
	for( vit = g_vecAddPath.begin(); vit != g_vecAddPath.end(); ++vit )
	{
		strcpy( szCurDir, vit->c_str() );
		//szCurDir[0] = cFolder;
		strCurDir = szCurDir;

		// 등록된 추가 경로는 기본으로 넣자!
		std::string strAddDir = strCurDir;
		strAddDir += "\\";
		m_vecDirectory.push_back( strAddDir );

		InitDirectory( strCurDir );
	}
	//}}
}

KAutoPath::~KAutoPath(void)
{
	m_vecDirectory.clear();
}

bool KAutoPath::GetPullPath( std::string& strFileName )
{
	FILE* file;
	std::string strName;

	for( int i = 0; i < (int)m_vecDirectory.size(); ++i )
	{
		strName = m_vecDirectory[i] + strFileName;

		file = fopen( strName.c_str(), "rb" );
		if( file != NULL )
		{
			strFileName = strName;
			break;
		}
	}

	if( file == NULL )
	{
		return false;
	}

	fclose( file );
	return true;
}

bool KAutoPath::GetPullPath( std::wstring& wstrFileName )
{
	char strFileName[255] = {0,};
	std::string strName;

	WideCharToMultiByte( CP_ACP, 0, wstrFileName.c_str(), -1, strFileName, 255, NULL, NULL );

	FILE* file;

	for( int i = 0; i < (int)m_vecDirectory.size(); ++i )
	{
		strName = m_vecDirectory[i] + strFileName;

		file = fopen( strName.c_str(), "rb" );
		if( file != NULL )
		{
			WCHAR wstrTemp[256] = L"";
			::MultiByteToWideChar( CP_ACP, 0, strName.c_str(), -1, wstrTemp, 255 );
			wstrFileName = wstrTemp;
			break;
		}
	}

	if( file == NULL )
	{
		return false;
	}

	fclose( file );
	return true;
}

void KAutoPath::InitDirectory( std::string strCurDir )
{
	HANDLE				hSearch;
	WIN32_FIND_DATAA	fd;
	char				szSearchPath[256];

	strcpy( szSearchPath, strCurDir.c_str() );
	strcat( szSearchPath, "\\*.*" );

	hSearch = FindFirstFileA( szSearchPath, &fd );

	if( hSearch == INVALID_HANDLE_VALUE )
		return;

	do
	{
		if( strcmp( fd.cFileName, "." ) && strcmp( fd.cFileName, ".." ) )
		{
			if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				char	szNewSearchDir[256];
				std::string	strDir;

				strcpy( szNewSearchDir, strCurDir.c_str() );
				strcat( szNewSearchDir, "\\" );
				strcat( szNewSearchDir, fd.cFileName );

				strDir = szNewSearchDir;
				strDir += "\\";

				m_vecDirectory.push_back( strDir );

				InitDirectory( szNewSearchDir );
			}
		}

	}while( FindNextFileA( hSearch, &fd ) );

	FindClose( hSearch );
}