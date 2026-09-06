//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 2 - DDS decoder implementation.
// DXT1/3/5 block decode plus mask-driven uncompressed unpack.
//////////////////////////////////////////////////////////////////////////
#include "DdsDecode.h"

// Native-only, like every other file in Core. Nothing here touches a
// third-party library, but the canary is what proves the split holds -
// see CASH_SHOP_TOOL_PLAN.md section 6 and the phase 0 notes.
#ifdef _MANAGED
#error "This TU compiled managed. /clr leaked in - see CppBuild.targets:733-738"
#endif

#include <stdio.h>
#include <string.h>

namespace
{
	//////////////////////////////////////////////////////////////////////
	// The DDS header, by offset. 128 bytes total: 4 magic + 124 DDSURFACEDESC2,
	// with the DDPIXELFORMAT nested at 76.

	const size_t	DDS_HEADER_SIZE		= 128;

	const size_t	OFF_MAGIC			= 0;
	const size_t	OFF_HEIGHT			= 12;
	const size_t	OFF_WIDTH			= 16;
	const size_t	OFF_PF_FLAGS		= 80;
	const size_t	OFF_PF_FOURCC		= 84;
	const size_t	OFF_PF_RGBBITCOUNT	= 88;
	const size_t	OFF_PF_RMASK		= 92;
	const size_t	OFF_PF_GMASK		= 96;
	const size_t	OFF_PF_BMASK		= 100;
	const size_t	OFF_PF_AMASK		= 104;

	const unsigned int	DDPF_ALPHAPIXELS	= 0x00000001;
	const unsigned int	DDPF_FOURCC			= 0x00000004;
	const unsigned int	DDPF_RGB			= 0x00000040;

	// A tool that only ever shows item icons has no business inflating a
	// 4096x4096 surface into 64 MB because a header said so.
	const int	MAX_DIMENSION	= 2048;

	unsigned int ReadU32( const unsigned char* p, size_t uOffset )
	{
		unsigned int uValue = 0;
		::memcpy( &uValue, p + uOffset, sizeof( unsigned int ) );
		return uValue;
	}

	//////////////////////////////////////////////////////////////////////
	// 565 -> 888. Bit replication, which is what every DXT decoder and
	// every GPU does: 5 bits of 31 map onto 8 bits of 255 exactly when the
	// top bits are repeated into the bottom ones.

	void Expand565( unsigned short usColor, unsigned char* pBGR )
	{
		const unsigned int uR = ( usColor >> 11 ) & 0x1F;
		const unsigned int uG = ( usColor >> 5 )  & 0x3F;
		const unsigned int uB = ( usColor )       & 0x1F;

		pBGR[0] = (unsigned char)( ( uB << 3 ) | ( uB >> 2 ) );
		pBGR[1] = (unsigned char)( ( uG << 2 ) | ( uG >> 4 ) );
		pBGR[2] = (unsigned char)( ( uR << 3 ) | ( uR >> 2 ) );
	}

	// The four colours of one DXT colour block, as BGR triples.
	//
	// bOneBitAlpha is true only for DXT1: there, c0 <= c1 selects a
	// three-colour mode whose fourth entry is transparent black. DXT3 and
	// DXT5 carry their alpha separately and therefore always use the
	// four-colour interpolation regardless of how c0 and c1 compare - a
	// detail that is easy to get wrong and shows up as blocky dark edges
	// on exactly the icons that have real alpha.
	void BuildColorTable( const unsigned char* pBlock, bool bOneBitAlpha,
							unsigned char acBGR[4][3], unsigned char acAlpha[4] )
	{
		unsigned short usC0 = 0;
		unsigned short usC1 = 0;
		::memcpy( &usC0, pBlock + 0, sizeof( unsigned short ) );
		::memcpy( &usC1, pBlock + 2, sizeof( unsigned short ) );

		Expand565( usC0, acBGR[0] );
		Expand565( usC1, acBGR[1] );

		acAlpha[0] = 255;
		acAlpha[1] = 255;
		acAlpha[2] = 255;
		acAlpha[3] = 255;

		if( false == bOneBitAlpha || usC0 > usC1 )
		{
			for( int i = 0; i < 3; ++i )
			{
				acBGR[2][i] = (unsigned char)( ( 2 * (int) acBGR[0][i] + (int) acBGR[1][i] ) / 3 );
				acBGR[3][i] = (unsigned char)( ( (int) acBGR[0][i] + 2 * (int) acBGR[1][i] ) / 3 );
			}
		}
		else
		{
			for( int i = 0; i < 3; ++i )
			{
				acBGR[2][i] = (unsigned char)( ( (int) acBGR[0][i] + (int) acBGR[1][i] ) / 2 );
				acBGR[3][i] = 0;
			}

			acAlpha[3] = 0;		// the transparent entry of the 3-colour mode
		}
	}

	// Writes one 4x4 block into the surface, clipping at the right and
	// bottom edges. pacAlpha16 is NULL for DXT1 (alpha comes from the
	// colour table) and otherwise points at 16 already-decoded alpha bytes
	// in block-local row-major order.
	void EmitBlock( unsigned char* pSurface, int iWidth, int iHeight, size_t uStride,
					int iBlockX, int iBlockY,
					const unsigned char acBGR[4][3], const unsigned char acAlpha[4],
					unsigned int uIndices, const unsigned char* pacAlpha16 )
	{
		for( int y = 0; y < 4; ++y )
		{
			const int iPixelY = iBlockY * 4 + y;
			if( iPixelY >= iHeight )
				break;

			for( int x = 0; x < 4; ++x )
			{
				const int iPixelX = iBlockX * 4 + x;
				if( iPixelX >= iWidth )
					continue;

				const int iLocal	= y * 4 + x;
				const int iIndex	= (int)( ( uIndices >> ( 2 * iLocal ) ) & 0x3 );

				unsigned char* pOut = pSurface + (size_t) iPixelY * uStride + (size_t) iPixelX * 4;

				pOut[0] = acBGR[iIndex][0];
				pOut[1] = acBGR[iIndex][1];
				pOut[2] = acBGR[iIndex][2];
				pOut[3] = ( NULL != pacAlpha16 ) ? pacAlpha16[iLocal] : acAlpha[iIndex];
			}
		}
	}

	//////////////////////////////////////////////////////////////////////

	bool DecodeDxt( const unsigned char* pData, size_t uSize, int iWidth, int iHeight,
					EDdsFormat eFormat, SDecodedImage& kOut, std::string* pstrDetail )
	{
		const int	iBlocksX	= ( iWidth  + 3 ) / 4;
		const int	iBlocksY	= ( iHeight + 3 ) / 4;
		const size_t uBlockBytes	= ( DdsFormat_DXT1 == eFormat ) ? 8u : 16u;
		const size_t uNeeded		= (size_t) iBlocksX * (size_t) iBlocksY * uBlockBytes;

		if( uSize < uNeeded )
		{
			if( NULL != pstrDetail )
			{
				char szMsg[128];
				::sprintf_s( szMsg, sizeof( szMsg ), "%s surface wants %u byte(s), file holds %u",
					DdsFormatName( eFormat ), (unsigned) uNeeded, (unsigned) uSize );
				*pstrDetail = szMsg;
			}
			return false;
		}

		const size_t uStride = (size_t) iWidth * 4;
		kOut.vecBGRA.assign( uStride * (size_t) iHeight, 0 );

		unsigned char* const pSurface = &kOut.vecBGRA[0];

		for( int by = 0; by < iBlocksY; ++by )
		{
			for( int bx = 0; bx < iBlocksX; ++bx )
			{
				const unsigned char* pBlock =
					pData + ( (size_t) by * iBlocksX + (size_t) bx ) * uBlockBytes;

				unsigned char	acAlpha16[16];
				bool			bHaveAlpha16 = false;

				const unsigned char* pColorBlock = pBlock;

				if( DdsFormat_DXT3 == eFormat )
				{
					// 8 bytes of 4-bit alpha, two pixels per byte, low
					// nibble first, then the colour block.
					for( int i = 0; i < 8; ++i )
					{
						const unsigned char cByte = pBlock[i];
						const unsigned int uLow  = ( cByte & 0x0F );
						const unsigned int uHigh = ( cByte >> 4 );

						// 4 bits of 15 onto 8 bits of 255 is nibble
						// replication, same principle as Expand565.
						acAlpha16[i * 2 + 0] = (unsigned char)( ( uLow  << 4 ) | uLow );
						acAlpha16[i * 2 + 1] = (unsigned char)( ( uHigh << 4 ) | uHigh );
					}

					bHaveAlpha16	= true;
					pColorBlock		= pBlock + 8;
				}
				else if( DdsFormat_DXT5 == eFormat )
				{
					const int iA0 = pBlock[0];
					const int iA1 = pBlock[1];

					int aiPalette[8];
					aiPalette[0] = iA0;
					aiPalette[1] = iA1;

					if( iA0 > iA1 )
					{
						for( int i = 1; i <= 6; ++i )
							aiPalette[i + 1] = ( ( 7 - i ) * iA0 + i * iA1 ) / 7;
					}
					else
					{
						for( int i = 1; i <= 4; ++i )
							aiPalette[i + 1] = ( ( 5 - i ) * iA0 + i * iA1 ) / 5;

						aiPalette[6] = 0;
						aiPalette[7] = 255;
					}

					// Six bytes of 3-bit indices: a 48-bit little-endian
					// field, read as two 24-bit halves so nothing needs a
					// 64-bit shift.
					for( int iHalf = 0; iHalf < 2; ++iHalf )
					{
						unsigned int uBits = (unsigned int) pBlock[2 + iHalf * 3]
										| ( (unsigned int) pBlock[3 + iHalf * 3] << 8 )
										| ( (unsigned int) pBlock[4 + iHalf * 3] << 16 );

						for( int i = 0; i < 8; ++i )
						{
							const int iIndex = (int)( ( uBits >> ( 3 * i ) ) & 0x7 );
							acAlpha16[iHalf * 8 + i] = (unsigned char) aiPalette[iIndex];
						}
					}

					bHaveAlpha16	= true;
					pColorBlock		= pBlock + 8;
				}

				unsigned char	acBGR[4][3];
				unsigned char	acAlpha[4];
				BuildColorTable( pColorBlock, ( DdsFormat_DXT1 == eFormat ), acBGR, acAlpha );

				unsigned int uIndices = 0;
				::memcpy( &uIndices, pColorBlock + 4, sizeof( unsigned int ) );

				EmitBlock( pSurface, iWidth, iHeight, uStride, bx, by,
					acBGR, acAlpha, uIndices, bHaveAlpha16 ? acAlpha16 : NULL );
			}
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////
	// Uncompressed DDPF_RGB. Driven by the channel masks in the header
	// rather than by assuming B,G,R,A byte order, because the 245 such
	// files in this install are a mix of 24- and 32-bit and there is no
	// reason to hardcode what the header already states.

	int MaskShift( unsigned int uMask )
	{
		if( 0 == uMask )
			return 0;

		int iShift = 0;
		while( 0 == ( uMask & 1u ) )
		{
			uMask >>= 1;
			++iShift;
		}

		return iShift;
	}

	unsigned int MaskMax( unsigned int uMask, int iShift )
	{
		return ( uMask >> iShift );
	}

	unsigned char ScaleChannel( unsigned int uValue, unsigned int uMax )
	{
		if( 0 == uMax )
			return 255;

		return (unsigned char)( ( uValue * 255u + uMax / 2u ) / uMax );
	}

	bool DecodeUncompressed( const unsigned char* pData, size_t uSize,
							int iWidth, int iHeight, unsigned int uBitCount,
							unsigned int uRMask, unsigned int uGMask,
							unsigned int uBMask, unsigned int uAMask,
							SDecodedImage& kOut, EDdsError* peError, std::string* pstrDetail )
	{
		if( 16 != uBitCount && 24 != uBitCount && 32 != uBitCount )
		{
			if( NULL != peError )	*peError = DdsError_UnsupportedFormat;
			if( NULL != pstrDetail )
			{
				char szMsg[96];
				::sprintf_s( szMsg, sizeof( szMsg ), "uncompressed %u bpp is not handled", uBitCount );
				*pstrDetail = szMsg;
			}
			return false;
		}

		const size_t uBytesPerPixel	= (size_t)( uBitCount / 8 );
		const size_t uSrcStride		= uBytesPerPixel * (size_t) iWidth;
		const size_t uNeeded		= uSrcStride * (size_t) iHeight;

		if( uSize < uNeeded )
		{
			if( NULL != peError )	*peError = DdsError_Corrupt;
			if( NULL != pstrDetail )
			{
				char szMsg[128];
				::sprintf_s( szMsg, sizeof( szMsg ), "uncompressed surface wants %u byte(s), file holds %u",
					(unsigned) uNeeded, (unsigned) uSize );
				*pstrDetail = szMsg;
			}
			return false;
		}

		const int iRShift = MaskShift( uRMask );
		const int iGShift = MaskShift( uGMask );
		const int iBShift = MaskShift( uBMask );
		const int iAShift = MaskShift( uAMask );

		const unsigned int uRMax = MaskMax( uRMask, iRShift );
		const unsigned int uGMax = MaskMax( uGMask, iGShift );
		const unsigned int uBMax = MaskMax( uBMask, iBShift );
		const unsigned int uAMax = MaskMax( uAMask, iAShift );

		const size_t uStride = (size_t) iWidth * 4;
		kOut.vecBGRA.assign( uStride * (size_t) iHeight, 0 );

		for( int y = 0; y < iHeight; ++y )
		{
			const unsigned char*	pSrc = pData + (size_t) y * uSrcStride;
			unsigned char*			pDst = &kOut.vecBGRA[0] + (size_t) y * uStride;

			for( int x = 0; x < iWidth; ++x )
			{
				unsigned int uPixel = 0;
				for( size_t u = 0; u != uBytesPerPixel; ++u )
					uPixel |= ( (unsigned int) pSrc[u] ) << ( 8 * u );

				pDst[0] = ScaleChannel( ( uPixel & uBMask ) >> iBShift, uBMax );
				pDst[1] = ScaleChannel( ( uPixel & uGMask ) >> iGShift, uGMax );
				pDst[2] = ScaleChannel( ( uPixel & uRMask ) >> iRShift, uRMax );
				pDst[3] = ( 0 != uAMask )
					? ScaleChannel( ( uPixel & uAMask ) >> iAShift, uAMax )
					: (unsigned char) 255;

				pSrc += uBytesPerPixel;
				pDst += 4;
			}
		}

		return true;
	}
}

//////////////////////////////////////////////////////////////////////////

const char* DdsFormatName( EDdsFormat eFormat )
{
	switch( eFormat )
	{
	case DdsFormat_DXT1:			return "DXT1";
	case DdsFormat_DXT3:			return "DXT3";
	case DdsFormat_DXT5:			return "DXT5";
	case DdsFormat_Uncompressed:	return "uncompressed";
	default:						break;
	}

	return "none";
}

bool DecodeDds( const void* pData, size_t uSize, SDecodedImage& kOut,
				EDdsError* peError, std::string* pstrDetail )
{
	kOut.Clear();

	if( NULL != peError )		*peError = DdsError_None;
	if( NULL != pstrDetail )	pstrDetail->clear();

	const unsigned char* const p = (const unsigned char*) pData;

	if( NULL == p || uSize < DDS_HEADER_SIZE )
	{
		if( NULL != peError )		*peError = DdsError_NotDds;
		if( NULL != pstrDetail )	*pstrDetail = "shorter than a DDS header";
		return false;
	}

	if( 0 != ::memcmp( p + OFF_MAGIC, "DDS ", 4 ) )
	{
		if( NULL != peError )	*peError = DdsError_NotDds;
		if( NULL != pstrDetail )
		{
			char szMsg[96];
			::sprintf_s( szMsg, sizeof( szMsg ), "magic is %02X %02X %02X %02X, not 'DDS '",
				p[0], p[1], p[2], p[3] );
			*pstrDetail = szMsg;
		}
		return false;
	}

	const int iHeight	= (int) ReadU32( p, OFF_HEIGHT );
	const int iWidth	= (int) ReadU32( p, OFF_WIDTH );

	if( iWidth <= 0 || iHeight <= 0 || iWidth > MAX_DIMENSION || iHeight > MAX_DIMENSION )
	{
		if( NULL != peError )	*peError = DdsError_Corrupt;
		if( NULL != pstrDetail )
		{
			char szMsg[96];
			::sprintf_s( szMsg, sizeof( szMsg ), "implausible dimensions %dx%d", iWidth, iHeight );
			*pstrDetail = szMsg;
		}
		return false;
	}

	const unsigned int uPfFlags	= ReadU32( p, OFF_PF_FLAGS );
	const unsigned int uFourCC	= ReadU32( p, OFF_PF_FOURCC );

	// Only mip 0 is wanted, and it is the first surface in the file, so
	// everything after it is simply not read (plan section 4 - these are
	// drawn at native size and never scaled down).
	const unsigned char* const	pSurface	= p + DDS_HEADER_SIZE;
	const size_t				uSurfaceSize	= uSize - DDS_HEADER_SIZE;

	kOut.iWidth	= iWidth;
	kOut.iHeight	= iHeight;

	if( 0 != ( uPfFlags & DDPF_FOURCC ) )
	{
		EDdsFormat eFormat = DdsFormat_None;

		if( 0 == ::memcmp( &uFourCC, "DXT1", 4 ) )			eFormat = DdsFormat_DXT1;
		else if( 0 == ::memcmp( &uFourCC, "DXT3", 4 ) )		eFormat = DdsFormat_DXT3;
		else if( 0 == ::memcmp( &uFourCC, "DXT5", 4 ) )		eFormat = DdsFormat_DXT5;

		if( DdsFormat_None == eFormat )
		{
			kOut.Clear();
			if( NULL != peError )	*peError = DdsError_UnsupportedFormat;
			if( NULL != pstrDetail )
			{
				char szMsg[96];
				const char* pszCC = (const char*) &uFourCC;
				::sprintf_s( szMsg, sizeof( szMsg ), "FourCC '%c%c%c%c' is not handled",
					pszCC[0], pszCC[1], pszCC[2], pszCC[3] );
				*pstrDetail = szMsg;
			}
			return false;
		}

		kOut.eFormat = eFormat;

		if( false == DecodeDxt( pSurface, uSurfaceSize, iWidth, iHeight, eFormat, kOut, pstrDetail ) )
		{
			kOut.Clear();
			if( NULL != peError )	*peError = DdsError_Corrupt;
			return false;
		}

		return true;
	}

	if( 0 != ( uPfFlags & DDPF_RGB ) )
	{
		const unsigned int uBitCount	= ReadU32( p, OFF_PF_RGBBITCOUNT );
		const unsigned int uRMask		= ReadU32( p, OFF_PF_RMASK );
		const unsigned int uGMask		= ReadU32( p, OFF_PF_GMASK );
		const unsigned int uBMask		= ReadU32( p, OFF_PF_BMASK );
		const unsigned int uAMask		= ( 0 != ( uPfFlags & DDPF_ALPHAPIXELS ) )
											? ReadU32( p, OFF_PF_AMASK ) : 0u;

		kOut.eFormat = DdsFormat_Uncompressed;

		if( false == DecodeUncompressed( pSurface, uSurfaceSize, iWidth, iHeight,
				uBitCount, uRMask, uGMask, uBMask, uAMask, kOut, peError, pstrDetail ) )
		{
			kOut.Clear();
			return false;
		}

		return true;
	}

	kOut.Clear();
	if( NULL != peError )	*peError = DdsError_UnsupportedFormat;
	if( NULL != pstrDetail )
	{
		char szMsg[96];
		::sprintf_s( szMsg, sizeof( szMsg ), "pixel format flags 0x%08X are neither FOURCC nor RGB", uPfFlags );
		*pstrDetail = szMsg;
	}

	return false;
}
