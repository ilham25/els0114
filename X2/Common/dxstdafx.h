//--------------------------------------------------------------------------------------
// File: DxStdAfx.h
//
// Desc: Standard includes and precompiled headers for DXUT
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#pragma once
#ifndef DXSDK_STDAFX_H
#define DXSDK_STDAFX_H

#ifndef WINVER
#define WINVER         0x0410
#endif
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410 
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT   0x0500 
#endif

#define _HAS_ITERATOR_DEBUGGING 0

#if defined(DEBUG) || defined(_DEBUG)
	#pragma comment( lib, "KTDXLIBD.lib" )
	#pragma comment( lib, "X2LibD.lib" )
#else
	#pragma comment( lib, "KTDXLIB.lib" )
	#pragma comment( lib, "X2Lib.lib" )
#endif


#include "X2Main.h"
//#include "vld.h"
#endif // !defined(DXSDK_STDAFX_H)
