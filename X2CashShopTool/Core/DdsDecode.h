//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 2 - DDS -> 32-bit BGRA. See
// CASH_SHOP_TOOL_PLAN.md section 4 ("Icons are whole files, not an atlas").
//
// The plan says the icons are "64x64 DXT1 or DXT5". Measured across the
// 15,098 distinct m_ShopImage files the catalog actually names, the real
// spread is DXT1 14,521 / DXT5 190 / uncompressed 24-bit 168 / DXT3 141 /
// uncompressed 32-bit 77, plus exactly one PNG stored under a .dds name.
// So all five DDS shapes are decoded here; the odd container is reported
// as a distinct outcome rather than being mistaken for a broken file.
//
// No D3D anywhere - the client needs a device to make a texture, a tool
// that only has to show 64x64 pixels does not (plan section 4).
//
// This header is included from the /clr Ui project, so it pulls in no
// third-party header - see the note at the top of KomArchive.h.
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <vector>

// What the surface turned out to be. Reported per decode and counted, so
// "the wall looks right" is backed by a census instead of a glance.
enum EDdsFormat
{
	DdsFormat_None = 0,
	DdsFormat_DXT1,
	DdsFormat_DXT3,
	DdsFormat_DXT5,
	DdsFormat_Uncompressed,		// DDPF_RGB, 16/24/32bpp, unpacked through its own channel masks
	DdsFormat_Count
};

const char*	DdsFormatName( EDdsFormat eFormat );

// Why a decode did not produce pixels. NotDds is kept apart from Corrupt
// on purpose: one file in the shipped set is a PNG under a .dds name, and
// that is a container the Ui can still show through GDI+, whereas Corrupt
// means the bytes really are unusable.
enum EDdsError
{
	DdsError_None = 0,
	DdsError_NotDds,			// no 'DDS ' magic - some other image container
	DdsError_UnsupportedFormat,	// a DDS this decoder does not implement (e.g. DXT2/4, cubemap, volume)
	DdsError_Corrupt			// truncated, or implausible dimensions
};

struct SDecodedImage
{
	int							iWidth;
	int							iHeight;
	EDdsFormat					eFormat;
	// Top-down, 4 bytes per pixel, B G R A in memory order - which is
	// exactly the byte layout of a GDI+ Format32bppArgb scanline on a
	// little-endian machine, so the Ui can memcpy row by row. Alpha is
	// straight, not premultiplied, as DDS stores it.
	std::vector<unsigned char>	vecBGRA;

	SDecodedImage() : iWidth( 0 ), iHeight( 0 ), eFormat( DdsFormat_None ) {}

	size_t	Stride() const	{ return (size_t) iWidth * 4; }
	bool	IsEmpty() const	{ return vecBGRA.empty(); }
	void	Clear()			{ iWidth = 0; iHeight = 0; eFormat = DdsFormat_None; vecBGRA.clear(); }
};

// Decodes mip 0 of a DDS held in memory. Later mips are ignored: the tool
// only ever draws these at their native size.
//
// Dimensions are not assumed to be multiples of four. Ten of the shipped
// shop icons are 55x55, which is 13.75 blocks wide, so the last block
// column and row are partial and must be clipped rather than written past
// the end of the surface.
bool DecodeDds( const void* pData, size_t uSize, SDecodedImage& kOut,
				EDdsError* peError, std::string* pstrDetail );
