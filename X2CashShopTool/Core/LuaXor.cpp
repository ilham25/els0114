//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 1 - in-place XOR implementation.
//////////////////////////////////////////////////////////////////////////
#include "LuaXor.h"

#ifdef _MANAGED
#error "This TU compiled managed. /clr leaked in - see CppBuild.targets:733-738"
#endif

#include <string.h>

namespace
{
	// KTDXLIB/KTDX.h:388-390. Carried here as literals rather than by
	// including KTDX.h, which is the engine hub and would drag in the world.
	const unsigned int XOR_KEYS[3] =
	{
		0xc6f8aa02u,	// XOR_KEY0
		0x2647abdcu,	// XOR_KEY1
		0x9800bbefu,	// XOR_KEY2
	};
}

void XorLuaScript( void* pData, size_t uSize )
{
	if( NULL == pData || 0 == uSize )
		return;

	unsigned char* const	pBytes	= (unsigned char*) pData;
	const size_t			uWhole	= uSize / 4;

	for( size_t u = 0; u != uWhole; ++u )
	{
		unsigned int uValue;
		::memcpy( &uValue, pBytes + u * 4, 4 );
		uValue ^= XOR_KEYS[u % 3];
		::memcpy( pBytes + u * 4, &uValue, 4 );
	}

	// The tail. XORDecrypt reads only the remaining 1-3 bytes into its int,
	// XORs the whole int against the key the rotation has reached, and then
	// appends just those 1-3 bytes (KTDXCommonFunc.h:672-687). On a
	// little-endian target that is exactly the low 1-3 bytes of the key
	// applied to the tail, which is what this does - without depending on
	// the uninitialised upper bytes the original reads.
	const size_t uTail = uSize - uWhole * 4;
	if( uTail > 0 )
	{
		const unsigned char* const pKey = (const unsigned char*) &XOR_KEYS[uWhole % 3];
		for( size_t u = 0; u != uTail; ++u )
			pBytes[uWhole * 4 + u] ^= pKey[u];
	}
}
