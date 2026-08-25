#include "StdAfx.h"
#include ".\ktdxenumtolua.h"

CKTDXEnumToLua::CKTDXEnumToLua(void)
{
}

CKTDXEnumToLua::~CKTDXEnumToLua(void)
{
}

bool CKTDXEnumToLua::LoadEnumToLua( KLuaManager* pLuaManager, const WCHAR* pEnumFileName )
{
	KGCMassFileManager::CMassFile::MASSFILE_MEMBERFILEINFO_POINTER Info;
	Info = g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LoadDataFile( pEnumFileName );
	if( Info == NULL )
	{
		string strFileName;
		ConvertWCHARToChar( strFileName, pEnumFileName );
		ErrorLogMsg( KEM_ERROR103, strFileName.c_str() );
		return false;
	}

	if( g_pKTDXApp->GetLuaBinder()->DoMemory( Info->pRealData, Info->size ) == E_FAIL )
	{
		string strFileName;
		ConvertWCHARToChar( strFileName, pEnumFileName );
		ErrorLogMsg( KEM_ERROR104, strFileName.c_str() );
		return false;
	}

	return true;
}









