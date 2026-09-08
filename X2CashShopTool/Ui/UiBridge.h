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
#include "../Core/ItemIndex.h"
#include "../Core/Labels.h"

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

	//////////////////////////////////////////////////////////////////////
	// Phase 6 - one resolved label, and the managed face of CLabelStore.
	//
	// Every window in this tool paints enum values, and phase 5's answer
	// was to paint the enumerator: IT_DEFENCE, EP_DEFENCE_FOOT,
	// CSSC_FASHION_WEAPON. This is where a value stops being spelled the
	// way a header spells it.
	//
	// A LabelInfo carries the text AND its origin, together, because the
	// one rule the label table exists to enforce is that this tool's own
	// wording is never presented as the game's. The details pane shows
	// Origin for the selected row and --labels prints the whole set.
	//
	// Script is the SCRIPT's own name for the value - IT_DEFENCE - kept so
	// the technical view can put it back and so the details pane can show
	// both at once. It is never the primary label.

	ref class LabelInfo
	{
	public:
		String^	Text;			// what to paint
		String^	Script;			// the Enum.lua name, or empty when it has none
		String^	OriginTag;		// ESS / CODE / SCRIPT / TOOL
		String^	OriginText;		// "the game's own text", ...
		String^	Note;			// the switch it came from, or why it is the tool's
		String^	EssRaw;			// the .ess row untouched; differs from Text only for grades
		String^	ScriptControl;	// the studio's control name for a sub-tab, or empty
		int		StringID;		// the STR_ID_* row, or -1

		LabelInfo()
		: Text( String::Empty ), Script( String::Empty )
		, OriginTag( "-" ), OriginText( "nothing" ), Note( String::Empty )
		, EssRaw( String::Empty ), ScriptControl( String::Empty ), StringID( -1 )
		{}

		// "Rare" by default; "Rare  (IG_RARE)" in the technical view, so
		// the toggle adds the enumerator back rather than swapping one
		// vocabulary for the other.
		String^ Show( bool bTechnical )
		{
			if( false == bTechnical || String::IsNullOrEmpty( Script ) )
				return Text;

			return String::Format( "{0}  ({1})", Text, Script );
		}
	};

	//////////////////////////////////////////////////////////////////////
	// Value -> LabelInfo, cached.
	//
	// CLabelStore is keyed on the SCRIPT's own enum name rather than on an
	// ordinal - deliberately, so nothing transcribes a number that could
	// drift from Enum.lua - which means every lookup is a name resolution
	// followed by a table scan. Painting one grid row would do six of
	// them, so each answer is built once and kept. There are at most a few
	// dozen distinct values per enum.

	ref class LabelBridge
	{
	public:
		LabelBridge( const SExtractResult* pCatalog, const CLabelStore* pLabels )
		{
			m_pCatalog	= pCatalog;
			m_pLabels	= pLabels;

			m_kCache = gcnew System::Collections::Generic::Dictionary<String^, LabelInfo^>();

			HasStrings	= ( NULL != pLabels ) && pLabels->HasStrings();
			Degraded	= ( NULL != pLabels ) ? NativeBridge::Utf8( pLabels->Degraded() ) : String::Empty;
			StringCount	= ( NULL != pLabels ) ? (int) pLabels->StringCount() : 0;

			SLabel kCurrency	= ( NULL != pLabels ) ? pLabels->Currency() : SLabel();
			SLabel kCostume		= ( NULL != pLabels ) ? pLabels->Costume()  : SLabel();

			Currency		= NativeBridge::Utf8( kCurrency.strText );
			CurrencyOrigin	= gcnew String( LabelOriginTag( kCurrency.eOrigin ) );
			Costume			= NativeBridge::Utf8( kCostume.strText );
			CostumeOrigin	= gcnew String( LabelOriginTag( kCostume.eOrigin ) );

			// Never empty even on a broken install: CLabelStore falls back
			// to the constant it read out of the repo and says so in the
			// origin, which is the honest version of a fallback.
			if( String::IsNullOrEmpty( Currency ) )	Currency = "K-Ching";
			if( String::IsNullOrEmpty( Costume ) )	Costume  = "Costume";
		}

		property bool		HasStrings;
		property String^	Degraded;
		property int		StringCount;

		// STR_ID_34, what the shop appends to every price
		// (X2Lib/X2CashShop.cpp:9194). The tool said ED until phase 6.
		property String^	Currency;
		property String^	CurrencyOrigin;

		// STR_ID_251, the game's word for m_bFashion
		// (X2Lib/X2ItemSlotManager.cpp:1270-1274).
		property String^	Costume;
		property String^	CostumeOrigin;

		LabelInfo^ ItemType( int iValue )
			{ return ByValue( LabelEnum_ItemType, EnumTableItemType(), iValue ); }

		LabelInfo^ EquipPosition( int iValue )
			{ return ByValue( LabelEnum_EquipPosition, EnumTableEquipPosition(), iValue ); }

		LabelInfo^ ItemGrade( int iValue )
			{ return ByValue( LabelEnum_ItemGrade, EnumTableItemGrade(), iValue ); }

		// The two cash-shop enums arrive already named: phase 4 reversed
		// the CSC_* and CSSC_* names out of Enum.lua into SCashCategoryRow,
		// so there is nothing to look up by value here.
		LabelInfo^ CashCategory( const std::string& strEnumName )
			{ return ByName( LabelEnum_CashCategory, strEnumName, -1 ); }

		LabelInfo^ CashSubCategory( const std::string& strEnumName )
			{ return ByName( LabelEnum_CashSubCategory, strEnumName, -1 ); }

	private:
		LabelInfo^ ByValue( ELabelEnum eEnum, const char* pszTable, int iValue )
		{
			String^ sKey = String::Format( "{0}/{1}", (int) eEnum, iValue );

			LabelInfo^ kFound = nullptr;
			if( m_kCache->TryGetValue( sKey, kFound ) )
				return kFound;

			// Empty when the value is in the item data and not in
			// Enum.lua, which CLabelStore turns into "value <n>" rather
			// than into a blank.
			const std::string strName = ( NULL != m_pCatalog )
				? LookupEnumRowName( m_pCatalog->vecEnumNames, pszTable, iValue )
				: std::string();

			LabelInfo^ kInfo = Build( eEnum, strName, iValue );

			m_kCache->Add( sKey, kInfo );
			return kInfo;
		}

		LabelInfo^ ByName( ELabelEnum eEnum, const std::string& strEnumName, int iValue )
		{
			String^ sKey = String::Format( "{0}/n/{1}", (int) eEnum, NativeBridge::Utf8( strEnumName ) );

			LabelInfo^ kFound = nullptr;
			if( m_kCache->TryGetValue( sKey, kFound ) )
				return kFound;

			LabelInfo^ kInfo = Build( eEnum, strEnumName, iValue );

			m_kCache->Add( sKey, kInfo );
			return kInfo;
		}

		LabelInfo^ Build( ELabelEnum eEnum, const std::string& strEnumName, int iValue )
		{
			LabelInfo^ kInfo = gcnew LabelInfo();

			if( NULL == m_pLabels )
			{
				kInfo->Text		= String::Format( "value {0}", iValue );
				kInfo->Script	= NativeBridge::Utf8( strEnumName );
				return kInfo;
			}

			const SLabel kLabel = m_pLabels->Label( eEnum, strEnumName.c_str(), iValue );

			kInfo->Text			= NativeBridge::Utf8( kLabel.strText );
			kInfo->Script		= NativeBridge::Utf8( strEnumName );
			kInfo->OriginTag	= gcnew String( LabelOriginTag( kLabel.eOrigin ) );
			kInfo->OriginText	= gcnew String( LabelOriginName( kLabel.eOrigin ) );
			kInfo->Note			= NativeBridge::Utf8( kLabel.strNote );
			kInfo->EssRaw		= NativeBridge::Utf8( kLabel.strEssRaw );
			kInfo->ScriptControl= NativeBridge::Utf8( kLabel.strScriptName );
			kInfo->StringID		= kLabel.iStringID;

			return kInfo;
		}

		const SExtractResult*	m_pCatalog;
		const CLabelStore*		m_pLabels;

		System::Collections::Generic::Dictionary<String^, LabelInfo^>^	m_kCache;
	};
}
