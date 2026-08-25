#pragma once

#include "fmod/inc/fmod.h"
#include "fmod/inc/fmod_errors.h"
#ifdef	X2OPTIMIZE_SOUND_BACKROUND_LOAD
#include "KGCMassFileBufferManager.h"
#endif	X2OPTIMIZE_SOUND_BACKROUND_LOAD


class CKTDXDeviceSound : public CKTDXDevice
{
public:
	CKTDXDeviceSound( FMOD_SYSTEM* pSystem, wstring fileName, bool bUse3D = true );
	~CKTDXDeviceSound(void);


	void Play( bool loop = false, bool b3DSound = true );

#ifdef CHECK_SOUND_LOADING_TIME
	void CheckPlay( bool& bCreateSound, bool loop = false, bool b3DSound = true );
#endif // CHECK_SOUND_LOADING_TIME

	void Wait();
	void Stop();

	bool IsPlaying();
	void Set3DPosition( const D3DXVECTOR3& pos );
	void SetMax3DDistance( float fMaxDistance );
	void SetVolume( float fVolume );

protected:

	bool Update3DPosition();
	virtual HRESULT _Load( bool bSkipStateCheck = false
#ifdef	X2OPTIMIZE_SOUND_BACKROUND_LOAD
				, bool bBackgroundQueueing = false
#endif	X2OPTIMIZE_SOUND_BACKROUND_LOAD		
		);

#ifdef CHECK_SOUND_LOADING_TIME
	 virtual HRESULT _CheckLoad( bool& bCreateSound,
		bool bSkipStateCheck = false
#ifdef	X2OPTIMIZE_SOUND_BACKROUND_LOAD
		, bool bBackgroundQueueing = false
#endif	X2OPTIMIZE_SOUND_BACKROUND_LOAD		
		);
#endif // CHECK_SOUND_LOADING_TIME

	virtual HRESULT _UnLoad();
#ifdef	X2OPTIMIZE_SOUND_BACKROUND_LOAD
	void			_CheckLoadFMODSound();
#endif	X2OPTIMIZE_SOUND_BACKROUND_LOAD

#ifdef CHECK_SOUND_LOADING_TIME
	void			_CheckCheckLoadFMODSound( bool& bCreateSound );
#endif // CHECK_SOUND_LOADING_TIME

protected:


	FMOD_VECTOR 		m_vFmodPos;

	FMOD_SYSTEM*		m_pSystem;
	FMOD_SOUND*			m_pSound;
	FMOD_CHANNEL*		m_pChannel;

	bool				m_b3DSound;
    float               m_fMaxDistance;

#ifdef	X2OPTIMIZE_SOUND_BACKROUND_LOAD
	KGCMassFileBufferPtr m_spMemoryBuffer;	
#endif	X2OPTIMIZE_SOUND_BACKROUND_LOAD

};
