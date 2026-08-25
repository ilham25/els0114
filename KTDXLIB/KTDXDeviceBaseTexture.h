#pragma once
#include "ktdxdevice.h"


class CKTDXDeviceBaseTexture : public CKTDXDevice
{
public:
	CKTDXDeviceBaseTexture( wstring deviceID, DEVICETYPE eDeviceType );
	virtual ~CKTDXDeviceBaseTexture(void);

	virtual void SetDeviceTexture( int stage = 0 ) = 0;
};

