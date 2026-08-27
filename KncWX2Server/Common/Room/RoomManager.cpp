#include "RoomManager.h"

ImplInstanceKeeping( KRoomManager );
NiImplementRootRTTI( KRoomManager );
//{{ Iruha : 2026-08-27 // VS2010 port: ImplementException(x) is a real macro (KncException.h),
// but this file never includes that header and KRoomManager never uses DeclareException, so
// this call was never actually defined here. VC7.1 silently parsed it as an implicit-int
// prototype (harmless, unused); VC10 makes that a hard error (C4430). Same class of issue as
// ImplementDBThread above.
//ImplementException( KRoomManager );
//}}

KRoomManager::KRoomManager()
{
}

KRoomManager::~KRoomManager()
{
}

ImplToStringW( KRoomManager )
{
	return stm_;
}

void KRoomManager::ShutDown()
{
	End();

	ReleaseKObj();
}