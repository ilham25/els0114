#include "RoomManager.h"

ImplInstanceKeeping( KRoomManager );
NiImplementRootRTTI( KRoomManager );
ImplementException( KRoomManager );

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