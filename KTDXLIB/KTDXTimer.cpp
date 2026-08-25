#include "StdAfx.h"
#include ".\ktdxtimer.h"



void CKTDXCheckElapsedTime::OnFrameMove()
{
	m_fSumOfElapsedTime += g_pKTDXApp->GetElapsedTime();
}
