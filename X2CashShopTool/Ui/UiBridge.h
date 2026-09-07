//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-07
// Description: X2CashShopTool - the two managed/native bridge pieces the
// windows share: NativeBridge, which is every string and pixel conversion
// that crosses the /clr line, and IconProvider, the bounded managed-Bitmap
// cache above CIconStore's own.
//
// Phase 4 wrote both of these inside MainForm.h. Phase 5's picker needs
// exactly the same two, so they moved here rather than being included
// through MainForm.h - a header that also declares MainForm, which
// includes the picker. The bodies are phase 4's, unchanged; only the file
// they live in is new. See CASH_SHOP_TOOL_PLAN.md.
//
// No third-party header is reachable from here, and nothing in this file
// calls Lua: it is included from the /clr Ui project (plan sections 6/7).
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <msclr/marshal.h>
#include <msclr/marshal_cppstd.h>
#include <string>
#include <vector>

#include "../Core/DdsDecode.h"
#include "../Core/IconStore.h"

using namespace System;
using namespace System::Windows::Forms;

namespace X2CashShopTool
{
	//////////////////////////////////////////////////////////////////////
	// The two conversions that cross the managed/native line, in one
	// place. Main.cpp's own helpers forward to these rather than keeping
	// a second copy of the same twenty lines.

	ref class NativeBridge abstract sealed
	{
	public:
		// Every narrow string crossing this boundary is UTF-8 - item names
		// included, because that is what the scripts hold and what
		// KLuaManager decodes with (luaLib/KLuaManager.h:803).
		static String^ Utf8( const std::string& str )
		{
			if( str.empty() )
				return String::Empty;

			array<Byte>^ abBytes = gcnew array<Byte>( (int) str.size() );
			System::Runtime::InteropServices::Marshal::Copy(
				IntPtr( (void*) str.data() ), abBytes, 0, (int) str.size() );

			return System::Text::Encoding::UTF8->GetString( abBytes );
		}

		// CIconStore::Get hands back a BORROWED pointer, valid only until
		// the next call to Get - so the copy happens here, immediately.
		// Format32bppArgb's scanline layout is B,G,R,A on a little-endian
		// machine, which is exactly what SDecodedImage holds, so this is a
		// row-by-row memcpy and not a per-pixel shuffle.
		static System::Drawing::Bitmap^ ToBitmap( const SDecodedImage* pImage )
		{
			if( NULL == pImage || pImage->IsEmpty() )
				return nullptr;

			System::Drawing::Bitmap^ kBitmap = gcnew System::Drawing::Bitmap(
				pImage->iWidth, pImage->iHeight,
				System::Drawing::Imaging::PixelFormat::Format32bppArgb );

			System::Drawing::Imaging::BitmapData^ kData = kBitmap->LockBits(
				System::Drawing::Rectangle( 0, 0, pImage->iWidth, pImage->iHeight ),
				System::Drawing::Imaging::ImageLockMode::WriteOnly,
				System::Drawing::Imaging::PixelFormat::Format32bppArgb );

			const unsigned char*	pSrc		= &pImage->vecBGRA[0];
			const size_t			uSrcStride	= pImage->Stride();
			unsigned char*			pDst		= (unsigned char*) kData->Scan0.ToPointer();

			for( int y = 0; y < pImage->iHeight; ++y )
				::memcpy( pDst + (size_t) y * kData->Stride, pSrc + (size_t) y * uSrcStride, uSrcStride );

			kBitmap->UnlockBits( kData );
			return kBitmap;
		}

		// The one shipped shop image that is not a DDS at all (a PNG under
		// a .dds name). Shown anyway - it is a perfectly good picture and
		// the fallback would be a lie about it.
		static System::Drawing::Bitmap^ FromRawBytes( const std::vector<char>& vecBytes )
		{
			if( vecBytes.empty() )
				return nullptr;

			try
			{
				array<Byte>^ abBytes = gcnew array<Byte>( (int) vecBytes.size() );
				System::Runtime::InteropServices::Marshal::Copy(
					IntPtr( (void*) &vecBytes[0] ), abBytes, 0, (int) vecBytes.size() );

				System::IO::MemoryStream^ kStream = gcnew System::IO::MemoryStream( abBytes, false );
				return gcnew System::Drawing::Bitmap( kStream );
			}
			catch( Exception^ )
			{
				return nullptr;
			}
		}
	};

	//////////////////////////////////////////////////////////////////////
	// Decoded icons, as managed Bitmaps, bounded.
	//
	// CIconStore already has its own 16 MB LRU over the decoded BGRA, but
	// a Bitmap is a GDI+ object and turning one back into a Bitmap on
	// every repaint would churn a megabyte of handles per scroll. This is
	// the second, thinner cache: name -> Bitmap, FIFO, capped. FIFO rather
	// than LRU because the access pattern is a scrolling list, where the
	// oldest entry really is the least likely to come back.
	ref class IconProvider
	{
	public:
		IconProvider( CIconStore* pStore, int iCapacity )
		{
			m_pStore	= pStore;
			m_iCapacity	= iCapacity;
			m_kMap		= gcnew System::Collections::Generic::Dictionary<String^, System::Drawing::Bitmap^>(
								StringComparer::OrdinalIgnoreCase );
			m_kOrder	= gcnew System::Collections::Generic::Queue<String^>();
		}

		// Never null unless the install is broken: an unresolvable name
		// falls back exactly where the client falls back
		// (X2Lib/X2SlotItem.cpp:255-265).
		System::Drawing::Bitmap^ Get( String^ sShopImage )
		{
			String^ sKey = ( nullptr == sShopImage ) ? String::Empty : sShopImage;

			System::Drawing::Bitmap^ kFound = nullptr;
			if( m_kMap->TryGetValue( sKey, kFound ) )
				return kFound;

			System::Drawing::Bitmap^ kBitmap = Decode( sKey );

			if( m_kMap->Count >= m_iCapacity && m_kOrder->Count > 0 )
			{
				String^ sOldest = m_kOrder->Dequeue();

				System::Drawing::Bitmap^ kEvicted = nullptr;
				if( m_kMap->TryGetValue( sOldest, kEvicted ) )
				{
					m_kMap->Remove( sOldest );

					// Safe to dispose: the Ui is single-threaded and no
					// paint is in flight while this runs.
					if( nullptr != kEvicted )
						delete kEvicted;
				}
			}

			m_kMap->Add( sKey, kBitmap );
			m_kOrder->Enqueue( sKey );

			return kBitmap;
		}

	private:
		System::Drawing::Bitmap^ Decode( String^ sShopImage )
		{
			if( NULL == m_pStore )
				return nullptr;

			const std::string strName = msclr::interop::marshal_as<std::string>( sShopImage );

			SIconLookup kLookup;
			const SDecodedImage* pImage = m_pStore->Get( strName.c_str(), kLookup );

			// A real picture stored under a .dds name - one shipped file
			// is a PNG. DdsError_NotDds is kept apart from _Corrupt for
			// exactly this, so GDI+ can be given the undecoded bytes.
			if( IconOutcome_FallbackUndecodable == kLookup.eOutcome && DdsError_NotDds == kLookup.eError )
			{
				std::vector<char>	vecRaw;
				std::string			strIgnored;

				if( m_pStore->ReadRaw( strName.c_str(), vecRaw, strIgnored ) )
				{
					System::Drawing::Bitmap^ kRaw = NativeBridge::FromRawBytes( vecRaw );
					if( nullptr != kRaw )
						return kRaw;
				}
			}

			return NativeBridge::ToBitmap( pImage );
		}

		CIconStore*	m_pStore;
		int			m_iCapacity;

		System::Collections::Generic::Dictionary<String^, System::Drawing::Bitmap^>^	m_kMap;
		System::Collections::Generic::Queue<String^>^								m_kOrder;
	};
}
