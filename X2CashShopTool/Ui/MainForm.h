//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 4 - the main window. Tabs and
// sub-tabs from CashShopCategory.lua as phase 1 parsed it, the product
// grid over cash_product, Edit and Delete, the wallet, and the "All"
// pseudo-tab with its report of categories no tab reaches.
// See CASH_SHOP_TOOL_PLAN.md, "Phase 4 - The main window".
//
// Everything here is managed. The native side - the save file, the item
// catalog and the icon store - is reached only through the three pointers
// the constructor is handed, which Main.cpp owns and outlives the window.
// No Lua call is ever made from this file, so no C++ exception from
// luaconf.h's throw can unwind a managed frame (plan sections 6 and 7).
//
// Two rules this window is built around, both of them the client's:
//
//   * category and quantity are narrowed to a signed char on the wire
//     (X2OfflineCashShop.cpp:181 and :185), so 1..127 is enforced and
//     never clamped - see EditProductForm.h, where the refusal lives;
//   * CX2OfflineCashShop::EnsureLoaded reads cash_product ONCE per
//     process (X2OfflineCashShop.h:98), so an edit made here shows up in
//     the game only after the game is restarted. The window says that in
//     as many words rather than letting a correct write look like a
//     failed one.
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <msclr/marshal.h>
#include <msclr/marshal_cppstd.h>
#include <string>
#include <vector>

#include "../Core/CashDb.h"
#include "../Core/DdsDecode.h"
#include "../Core/IconStore.h"
#include "../Core/ItemIndex.h"

#include "EditProductForm.h"

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
	// One row of cash_product, joined against the item catalog and the
	// tab table so the grid can paint without asking anything again.

	ref class ProductRow
	{
	public:
		int	ProductNo;
		int	ItemID;
		int	CategoryNo;
		int	Quantity;
		int	Price;
		int	IsEvent;

		String^	Name;
		String^	ShopImage;

		// False = the client will DROP this row at load with nothing but a
		// log line (X2OfflineCashShop.cpp:85-89). 388 of the 2,360 shipped
		// rows are in this state, and until this window there was no way
		// to see which.
		bool	HasTemplet;

		// -1 = the category matches no pair in CashShopCategory.lua, so no
		// tab in the game shows it however valid the row otherwise is.
		int		TabIdx;
		String^	CategoryLabel;

		ProductRow()
		: ProductNo( 0 ), ItemID( 0 ), CategoryNo( 0 ), Quantity( 1 ), Price( 1 ), IsEvent( 0 )
		, Name( String::Empty ), ShopImage( String::Empty )
		, HasTemplet( false ), TabIdx( -1 ), CategoryLabel( String::Empty )
		{}
	};

	//////////////////////////////////////////////////////////////////////
	// The grid. One owner-drawn panel painting only the visible rows -
	// the same discipline phase 2's icon wall settled on, and the one
	// phase 5's picker over 48,754 items will need. A Control per row is
	// a window handle per row.

	ref class ProductGridPanel : public Panel
	{
	public:
		ProductGridPanel( IconProvider^ kIcons )
		{
			m_kIcons	= kIcons;
			m_kRows		= gcnew System::Collections::Generic::List<ProductRow^>();
			m_iSelected	= -1;
			m_iRowHeight= 72;

			SetStyle( ControlStyles::OptimizedDoubleBuffer
				| ControlStyles::AllPaintingInWmPaint
				| ControlStyles::UserPaint
				| ControlStyles::Selectable, true );

			AutoScroll	= true;
			TabStop		= true;
			BackColor	= System::Drawing::Color::FromArgb( 32, 32, 36 );

			m_kNameFont		= gcnew System::Drawing::Font( "Segoe UI", 9.5f, System::Drawing::FontStyle::Bold );
			m_kMetaFont		= gcnew System::Drawing::Font( "Consolas", 8.0f );
			m_kNumberFont	= gcnew System::Drawing::Font( "Consolas", 10.0f, System::Drawing::FontStyle::Bold );
			m_kBadgeFont	= gcnew System::Drawing::Font( "Segoe UI", 7.0f, System::Drawing::FontStyle::Bold );

			m_kLinePen		= gcnew System::Drawing::Pen( System::Drawing::Color::FromArgb( 52, 52, 58 ), 1.0f );
			m_kSelectBrush	= gcnew System::Drawing::SolidBrush( System::Drawing::Color::FromArgb( 48, 66, 96 ) );
			m_kDroppedBrush	= gcnew System::Drawing::SolidBrush( System::Drawing::Color::FromArgb( 60, 34, 34 ) );
			m_kEventBrush	= gcnew System::Drawing::SolidBrush( System::Drawing::Color::FromArgb( 150, 96, 24 ) );

			m_kRight = gcnew System::Drawing::StringFormat();
			m_kRight->Alignment = System::Drawing::StringAlignment::Far;

			m_kToolTip = gcnew ToolTip();
			m_kToolTip->InitialDelay	= 400;
			m_kToolTip->AutoPopDelay	= 20000;
			m_iHoverIndex = -1;
		}

		event EventHandler^	SelectionChanged;
		event EventHandler^	EditRequested;
		event EventHandler^	DeleteRequested;

		void SetRows( System::Collections::Generic::List<ProductRow^>^ kRows, int iKeepProductNo )
		{
			m_kRows		= kRows;
			m_iSelected	= -1;

			// A refresh after an edit should leave the cursor where it
			// was, not at the top of the list.
			if( iKeepProductNo > 0 )
			{
				for( int i = 0; i < m_kRows->Count; ++i )
				{
					if( m_kRows[i]->ProductNo == iKeepProductNo )
					{
						m_iSelected = i;
						break;
					}
				}
			}

			if( m_iSelected < 0 && m_kRows->Count > 0 )
				m_iSelected = 0;

			AutoScrollMinSize = System::Drawing::Size( 0, m_kRows->Count * m_iRowHeight );
			ScrollSelectionIntoView();
			Invalidate();

			SelectionChanged( this, EventArgs::Empty );
		}

		ProductRow^ Selected()
		{
			if( m_iSelected < 0 || m_iSelected >= m_kRows->Count )
				return nullptr;

			return m_kRows[m_iSelected];
		}

		int RowCount()	{ return m_kRows->Count; }

	protected:
		virtual void OnPaint( PaintEventArgs^ e ) override
		{
			System::Drawing::Graphics^ g = e->Graphics;
			g->InterpolationMode	= System::Drawing::Drawing2D::InterpolationMode::NearestNeighbor;
			g->PixelOffsetMode		= System::Drawing::Drawing2D::PixelOffsetMode::Half;

			if( 0 == m_kRows->Count )
			{
				g->DrawString( "no products in this tab", m_kNameFont,
					System::Drawing::Brushes::DimGray, 16.0f, 16.0f );
				return;
			}

			const int iScrollY = -AutoScrollPosition.Y;

			// Only the visible band is painted. At 2,360 rows the
			// difference between this and painting the lot is the
			// difference between a scroll and a slideshow.
			int iFirst = ( e->ClipRectangle.Top + iScrollY ) / m_iRowHeight;
			int iLast  = ( e->ClipRectangle.Bottom + iScrollY ) / m_iRowHeight;

			iFirst = Math::Max( 0, iFirst );
			iLast  = Math::Min( m_kRows->Count - 1, iLast );

			for( int i = iFirst; i <= iLast; ++i )
				DrawRow( g, m_kRows[i], i, i * m_iRowHeight - iScrollY );
		}

		virtual void OnMouseDown( MouseEventArgs^ e ) override
		{
			Panel::OnMouseDown( e );
			Focus();

			const int iHit = HitTest( e->Y );
			if( iHit < 0 || iHit == m_iSelected )
				return;

			m_iSelected = iHit;
			Invalidate();
			SelectionChanged( this, EventArgs::Empty );
		}

		virtual void OnMouseDoubleClick( MouseEventArgs^ e ) override
		{
			Panel::OnMouseDoubleClick( e );

			if( HitTest( e->Y ) >= 0 )
				EditRequested( this, EventArgs::Empty );
		}

		virtual void OnMouseMove( MouseEventArgs^ e ) override
		{
			Panel::OnMouseMove( e );

			const int iHit = HitTest( e->Y );
			if( iHit == m_iHoverIndex )
				return;

			m_iHoverIndex = iHit;
			m_kToolTip->SetToolTip( this, ( iHit >= 0 ) ? Describe( m_kRows[iHit] ) : String::Empty );
		}

		// Without this a Panel never sees the arrow keys at all - they are
		// eaten as navigation before OnKeyDown runs.
		virtual bool IsInputKey( Keys keyData ) override
		{
			switch( keyData )
			{
			case Keys::Up:
			case Keys::Down:
			case Keys::PageUp:
			case Keys::PageDown:
			case Keys::Home:
			case Keys::End:
				return true;
			}

			return Panel::IsInputKey( keyData );
		}

		virtual void OnKeyDown( KeyEventArgs^ e ) override
		{
			Panel::OnKeyDown( e );

			if( 0 == m_kRows->Count )
				return;

			const int iPage = Math::Max( 1, ClientSize.Height / m_iRowHeight );
			int iNext = m_iSelected;

			switch( e->KeyCode )
			{
			case Keys::Up:			iNext = m_iSelected - 1;		break;
			case Keys::Down:		iNext = m_iSelected + 1;		break;
			case Keys::PageUp:		iNext = m_iSelected - iPage;	break;
			case Keys::PageDown:	iNext = m_iSelected + iPage;	break;
			case Keys::Home:		iNext = 0;						break;
			case Keys::End:			iNext = m_kRows->Count - 1;		break;

			case Keys::Enter:
				EditRequested( this, EventArgs::Empty );
				return;

			case Keys::Delete:
				DeleteRequested( this, EventArgs::Empty );
				return;

			default:
				return;
			}

			iNext = Math::Max( 0, Math::Min( m_kRows->Count - 1, iNext ) );
			if( iNext == m_iSelected )
				return;

			m_iSelected = iNext;
			ScrollSelectionIntoView();
			Invalidate();
			SelectionChanged( this, EventArgs::Empty );
		}

		virtual void OnClientSizeChanged( EventArgs^ e ) override
		{
			Panel::OnClientSizeChanged( e );
			Invalidate();
		}

	private:
		int HitTest( int iMouseY )
		{
			const int iIndex = ( iMouseY - AutoScrollPosition.Y ) / m_iRowHeight;
			return ( iIndex >= 0 && iIndex < m_kRows->Count ) ? iIndex : -1;
		}

		void ScrollSelectionIntoView()
		{
			if( m_iSelected < 0 )
				return;

			const int iTop		= m_iSelected * m_iRowHeight;
			const int iScrollY	= -AutoScrollPosition.Y;

			if( iTop < iScrollY )
				AutoScrollPosition = System::Drawing::Point( 0, iTop );
			else if( iTop + m_iRowHeight > iScrollY + ClientSize.Height )
				AutoScrollPosition = System::Drawing::Point( 0, iTop + m_iRowHeight - ClientSize.Height );
		}

		String^ Describe( ProductRow^ kRow )
		{
			return String::Format(
				"product_no {0}\r\nitem {1}  {2}\r\nm_ShopImage: {3}\r\ncategory {4}  {5}\r\n"
				"quantity {6}   price {7}   is_event {8}{9}",
				kRow->ProductNo, kRow->ItemID, kRow->Name,
				String::IsNullOrEmpty( kRow->ShopImage ) ? "(none)" : kRow->ShopImage,
				kRow->CategoryNo, kRow->CategoryLabel,
				kRow->Quantity, kRow->Price, kRow->IsEvent,
				kRow->HasTemplet ? "" : "\r\n\r\nTHE CLIENT WILL DROP THIS ROW: no item templet for that item id." );
		}

		void DrawRow( System::Drawing::Graphics^ g, ProductRow^ kRow, int iIndex, int iY )
		{
			System::Drawing::Rectangle kBounds( 0, iY, ClientSize.Width, m_iRowHeight );

			if( false == kRow->HasTemplet )
				g->FillRectangle( m_kDroppedBrush, kBounds );

			if( iIndex == m_iSelected )
				g->FillRectangle( m_kSelectBrush, kBounds );

			g->DrawLine( m_kLinePen, 0, iY + m_iRowHeight - 1, ClientSize.Width, iY + m_iRowHeight - 1 );

			System::Drawing::Bitmap^ kIcon = m_kIcons->Get( kRow->ShopImage );
			if( nullptr != kIcon )
				g->DrawImage( kIcon, 6, iY + 4, kIcon->Width, kIcon->Height );

			const int iTextLeft = 80;

			g->DrawString( kRow->Name, m_kNameFont,
				kRow->HasTemplet ? System::Drawing::Brushes::Gainsboro : System::Drawing::Brushes::LightCoral,
				(float) iTextLeft, (float)( iY + 8 ) );

			g->DrawString(
				String::Format( "product {0}   item {1}   category {2}  {3}",
					kRow->ProductNo, kRow->ItemID, kRow->CategoryNo, kRow->CategoryLabel ),
				m_kMetaFont, System::Drawing::Brushes::Gray, (float) iTextLeft, (float)( iY + 30 ) );

			if( false == kRow->HasTemplet )
			{
				g->DrawString( "the client drops this row - no item templet", m_kMetaFont,
					System::Drawing::Brushes::IndianRed, (float) iTextLeft, (float)( iY + 47 ) );
			}
			else if( kRow->TabIdx < 0 )
			{
				g->DrawString( "no tab in CashShopCategory.lua shows this category", m_kMetaFont,
					System::Drawing::Brushes::Goldenrod, (float) iTextLeft, (float)( iY + 47 ) );
			}

			//////////////////////////////////////////////////////////////
			// The numbers, right-aligned so a column of prices reads as a
			// column.

			const int iRight = ClientSize.Width - 12;

			System::Drawing::RectangleF kPrice( (float)( iRight - 130 ), (float)( iY + 10 ), 130.0f, 20.0f );
			g->DrawString( String::Format( "{0}  ED", kRow->Price ), m_kNumberFont,
				System::Drawing::Brushes::Khaki, kPrice, m_kRight );

			System::Drawing::RectangleF kQty( (float)( iRight - 130 ), (float)( iY + 32 ), 130.0f, 20.0f );
			g->DrawString( String::Format( "x{0}", kRow->Quantity ), m_kMetaFont,
				System::Drawing::Brushes::Silver, kQty, m_kRight );

			if( 0 != kRow->IsEvent )
			{
				System::Drawing::Rectangle kBadge( iRight - 58, iY + 48, 58, 15 );
				g->FillRectangle( m_kEventBrush, kBadge );
				g->DrawString( "EVENT", m_kBadgeFont, System::Drawing::Brushes::White,
					(float)( kBadge.Left + 7 ), (float)( kBadge.Top + 1 ) );
			}
		}

		IconProvider^												m_kIcons;
		System::Collections::Generic::List<ProductRow^>^				m_kRows;
		int															m_iSelected;
		int															m_iRowHeight;
		int															m_iHoverIndex;

		System::Drawing::Font^			m_kNameFont;
		System::Drawing::Font^			m_kMetaFont;
		System::Drawing::Font^			m_kNumberFont;
		System::Drawing::Font^			m_kBadgeFont;
		System::Drawing::Pen^			m_kLinePen;
		System::Drawing::SolidBrush^	m_kSelectBrush;
		System::Drawing::SolidBrush^	m_kDroppedBrush;
		System::Drawing::SolidBrush^	m_kEventBrush;
		System::Drawing::StringFormat^	m_kRight;
		ToolTip^						m_kToolTip;
	};

	//////////////////////////////////////////////////////////////////////
	// What the left-hand lists hold. Plain carriers with a ToString, so
	// the ListBoxes need no owner-draw of their own.

	ref class TabEntry
	{
	public:
		int		TabIdx;			// -1 = the "All" pseudo-tab
		String^	Label;

		virtual String^ ToString() override	{ return Label; }
	};

	// Which rows a sub-list selection keeps. The four beyond Category are
	// the "All" tab's report: they exist so a row that is invisible in the
	// game is reachable in the tool, which is the whole reason the tool
	// can say anything useful about a catalog nobody chose.
	enum class SubFilterKind
	{
		Everything	= 0,
		Category,
		WholeTab,
		Orphaned,		// category matches no CashShopCategory.lua pair
		Dropped,		// item has no client templet
		OutOfRange,		// category or quantity outside 1..127
	};

	ref class SubEntry
	{
	public:
		SubFilterKind	Kind;
		int				BillingNo;
		int				TabIdx;
		String^			Label;

		virtual String^ ToString() override	{ return Label; }
	};

	//////////////////////////////////////////////////////////////////////

	ref class MainForm : public Form
	{
	public:
		// pDb must already be open read-write, and pCatalog is phase 1's
		// item catalog with SetKnownItems already handed over. Main.cpp
		// owns all three and outlives the window.
		MainForm( CCashDb* pDb, CIconStore* pIcons, const SExtractResult* pCatalog )
		{
			m_pDb		= pDb;
			m_pCatalog	= pCatalog;

			m_kIcons	= gcnew IconProvider( pIcons, 1500 );
			m_kAllRows	= gcnew System::Collections::Generic::List<ProductRow^>();

			Text			= "X2CashShopTool - offline cash shop catalog";
			ClientSize		= System::Drawing::Size( 1180, 760 );
			MinimumSize		= System::Drawing::Size( 900, 560 );
			StartPosition	= FormStartPosition::CenterScreen;
			BackColor		= System::Drawing::Color::FromArgb( 32, 32, 36 );
			ForeColor		= System::Drawing::Color::Gainsboro;

			BuildCategoryMaps();
			BuildChrome();

			ReloadFromDb( 0 );
		}

	private:
		//////////////////////////////////////////////////////////////////
		// Layout.
		//
		// Docked controls are laid out from the LAST entry in Controls to
		// the first, so the Fill control has to be added FIRST or it takes
		// the whole form and the edges land on top of it.

		void BuildChrome()
		{
			m_kFont		= gcnew System::Drawing::Font( "Segoe UI", 9.0f );
			m_kFixed	= gcnew System::Drawing::Font( "Consolas", 8.5f );

			Panel^ kRight = gcnew Panel();
			kRight->Dock = DockStyle::Fill;

			m_kGrid = gcnew ProductGridPanel( m_kIcons );
			m_kGrid->Dock = DockStyle::Fill;
			m_kGrid->SelectionChanged	+= gcnew EventHandler( this, &MainForm::OnSelectionChanged );
			m_kGrid->EditRequested		+= gcnew EventHandler( this, &MainForm::OnEdit );
			m_kGrid->DeleteRequested	+= gcnew EventHandler( this, &MainForm::OnDelete );

			Panel^ kButtons = gcnew Panel();
			kButtons->Dock		= DockStyle::Bottom;
			kButtons->Height	= 44;
			kButtons->BackColor	= System::Drawing::Color::FromArgb( 24, 24, 28 );

			m_kEditButton	= MakeButton( "Edit  (Enter)",			10, 8, 130, kButtons );
			m_kDeleteButton	= MakeButton( "Delete  (Del)",			150, 8, 130, kButtons );
			Button^ kRefresh = MakeButton( "Refresh from the save",	290, 8, 170, kButtons );

			m_kEditButton->Click	+= gcnew EventHandler( this, &MainForm::OnEdit );
			m_kDeleteButton->Click	+= gcnew EventHandler( this, &MainForm::OnDelete );
			kRefresh->Click			+= gcnew EventHandler( this, &MainForm::OnRefresh );

			m_kFilterLabel = gcnew Label();
			m_kFilterLabel->Bounds		= System::Drawing::Rectangle( 480, 12, 640, 20 );
			m_kFilterLabel->Font		= m_kFixed;
			m_kFilterLabel->ForeColor	= System::Drawing::Color::FromArgb( 160, 160, 168 );
			kButtons->Controls->Add( m_kFilterLabel );

			kRight->Controls->Add( m_kGrid );
			kRight->Controls->Add( kButtons );

			//////////////////////////////////////////////////////////////

			Panel^ kLeft = gcnew Panel();
			kLeft->Dock			= DockStyle::Left;
			kLeft->Width		= 300;
			kLeft->BackColor	= System::Drawing::Color::FromArgb( 24, 24, 28 );

			m_kSubList = gcnew ListBox();
			m_kSubList->Dock					= DockStyle::Fill;
			m_kSubList->Font					= m_kFixed;
			m_kSubList->BackColor				= System::Drawing::Color::FromArgb( 24, 24, 28 );
			m_kSubList->ForeColor				= System::Drawing::Color::Gainsboro;
			m_kSubList->BorderStyle				= ::BorderStyle::None;
			m_kSubList->IntegralHeight			= false;
			m_kSubList->SelectedIndexChanged	+= gcnew EventHandler( this, &MainForm::OnSubChanged );

			Label^ kSubHead = MakeHeading( "sub-category  ->  cash_product.category" );

			m_kTabList = gcnew ListBox();
			m_kTabList->Dock					= DockStyle::Top;
			m_kTabList->Height					= 180;
			m_kTabList->Font					= m_kFixed;
			m_kTabList->BackColor				= System::Drawing::Color::FromArgb( 24, 24, 28 );
			m_kTabList->ForeColor				= System::Drawing::Color::Gainsboro;
			m_kTabList->BorderStyle				= ::BorderStyle::None;
			m_kTabList->IntegralHeight			= false;
			m_kTabList->SelectedIndexChanged	+= gcnew EventHandler( this, &MainForm::OnTabChanged );

			Label^ kTabHead = MakeHeading( "tab  (CashShopCategory.lua)" );

			kLeft->Controls->Add( m_kSubList );
			kLeft->Controls->Add( kSubHead );
			kLeft->Controls->Add( m_kTabList );
			kLeft->Controls->Add( kTabHead );

			//////////////////////////////////////////////////////////////

			Panel^ kHeader = gcnew Panel();
			kHeader->Dock		= DockStyle::Top;
			kHeader->Height		= 96;
			kHeader->BackColor	= System::Drawing::Color::FromArgb( 24, 24, 28 );

			m_kSaveLabel = gcnew Label();
			m_kSaveLabel->Bounds	= System::Drawing::Rectangle( 12, 10, 800, 34 );
			m_kSaveLabel->Anchor	= (AnchorStyles)( AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right );
			m_kSaveLabel->Font		= m_kFixed;
			m_kSaveLabel->ForeColor	= System::Drawing::Color::Gainsboro;
			kHeader->Controls->Add( m_kSaveLabel );

			Label^ kNotice = gcnew Label();
			kNotice->Bounds		= System::Drawing::Rectangle( 12, 50, 800, 40 );
			kNotice->Anchor		= (AnchorStyles)( AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right );
			kNotice->Font		= gcnew System::Drawing::Font( "Segoe UI", 8.0f );
			kNotice->ForeColor	= System::Drawing::Color::Goldenrod;
			kNotice->Text		= "The game reads cash_product once per process (CX2OfflineCashShop::EnsureLoaded, "
				"X2OfflineCashShop.h:98), so RESTART X2_offline.exe to see an edit. "
				"A change that looks not to have worked is almost always this.";
			kHeader->Controls->Add( kNotice );

			m_kWalletLabel = gcnew Label();
			m_kWalletLabel->Bounds		= System::Drawing::Rectangle( 0, 14, 140, 20 );
			m_kWalletLabel->Anchor		= (AnchorStyles)( AnchorStyles::Top | AnchorStyles::Right );
			m_kWalletLabel->Font		= m_kFont;
			m_kWalletLabel->ForeColor	= System::Drawing::Color::Gainsboro;
			m_kWalletLabel->Text		= "wallet (cash_start)";
			kHeader->Controls->Add( m_kWalletLabel );

			m_kWalletBox = gcnew TextBox();
			m_kWalletBox->Bounds		= System::Drawing::Rectangle( 0, 38, 130, 24 );
			m_kWalletBox->Anchor		= (AnchorStyles)( AnchorStyles::Top | AnchorStyles::Right );
			m_kWalletBox->Font			= m_kFixed;
			m_kWalletBox->BackColor		= System::Drawing::Color::FromArgb( 32, 32, 36 );
			m_kWalletBox->ForeColor		= System::Drawing::Color::Gainsboro;
			m_kWalletBox->BorderStyle	= ::BorderStyle::FixedSingle;
			kHeader->Controls->Add( m_kWalletBox );

			m_kWalletButton = gcnew Button();
			m_kWalletButton->Bounds		= System::Drawing::Rectangle( 0, 38, 60, 24 );
			m_kWalletButton->Anchor		= (AnchorStyles)( AnchorStyles::Top | AnchorStyles::Right );
			m_kWalletButton->Text		= "Set";
			m_kWalletButton->Font		= m_kFont;
			m_kWalletButton->FlatStyle	= ::FlatStyle::Flat;
			m_kWalletButton->ForeColor	= System::Drawing::Color::Gainsboro;
			m_kWalletButton->Click		+= gcnew EventHandler( this, &MainForm::OnSetWallet );
			kHeader->Controls->Add( m_kWalletButton );

			// Anchored to the right edge, so their x is set from the
			// header's real width rather than guessed.
			kHeader->Resize += gcnew EventHandler( this, &MainForm::OnHeaderResize );
			m_kHeader = kHeader;

			//////////////////////////////////////////////////////////////

			m_kStatus = gcnew Label();
			m_kStatus->Dock			= DockStyle::Bottom;
			m_kStatus->Height		= 52;
			m_kStatus->Padding		= System::Windows::Forms::Padding( 12, 6, 12, 6 );
			m_kStatus->Font			= m_kFixed;
			m_kStatus->BackColor	= System::Drawing::Color::FromArgb( 20, 20, 24 );
			m_kStatus->ForeColor	= System::Drawing::Color::FromArgb( 170, 200, 170 );

			Controls->Add( kRight );
			Controls->Add( kLeft );
			Controls->Add( kHeader );
			Controls->Add( m_kStatus );

			LayoutHeader();
		}

		Label^ MakeHeading( String^ sText )
		{
			Label^ kLabel = gcnew Label();
			kLabel->Dock		= DockStyle::Top;
			kLabel->Height		= 22;
			kLabel->Padding		= System::Windows::Forms::Padding( 8, 4, 0, 0 );
			kLabel->Font		= gcnew System::Drawing::Font( "Segoe UI", 8.0f, System::Drawing::FontStyle::Bold );
			kLabel->ForeColor	= System::Drawing::Color::FromArgb( 140, 140, 150 );
			kLabel->BackColor	= System::Drawing::Color::FromArgb( 18, 18, 22 );
			kLabel->Text		= sText;
			return kLabel;
		}

		Button^ MakeButton( String^ sText, int iX, int iY, int iWidth, Control^ kParent )
		{
			Button^ kButton = gcnew Button();
			kButton->Bounds		= System::Drawing::Rectangle( iX, iY, iWidth, 28 );
			kButton->Text		= sText;
			kButton->Font		= m_kFont;
			kButton->FlatStyle	= ::FlatStyle::Flat;
			kButton->ForeColor	= System::Drawing::Color::Gainsboro;
			kParent->Controls->Add( kButton );
			return kButton;
		}

		void OnHeaderResize( Object^ /*kSender*/, EventArgs^ /*kArgs*/ )
		{
			LayoutHeader();
		}

		void LayoutHeader()
		{
			if( nullptr == m_kHeader )
				return;

			const int iRight = m_kHeader->ClientSize.Width;

			m_kSaveLabel->Width		= Math::Max( 200, iRight - 250 );

			m_kWalletLabel->Left	= iRight - 210;
			m_kWalletBox->Left		= iRight - 210;
			m_kWalletButton->Left	= iRight - 72;
		}

		//////////////////////////////////////////////////////////////////
		// The tab table, turned into something the lists can show.

		void BuildCategoryMaps()
		{
			m_kCatLabel	= gcnew System::Collections::Generic::Dictionary<int, String^>();
			m_kCatTab	= gcnew System::Collections::Generic::Dictionary<int, int>();
			m_kTabName	= gcnew System::Collections::Generic::Dictionary<int, String^>();

			if( NULL == m_pCatalog )
				return;

			for( size_t u = 0; u != m_pCatalog->vecCategories.size(); ++u )
			{
				const SCashCategoryRow& kRow = m_pCatalog->vecCategories[u];

				String^ sTab = NativeBridge::Utf8( kRow.strTabName );
				String^ sSub = NativeBridge::Utf8( kRow.strSubName );

				if( String::IsNullOrEmpty( sTab ) )
					sTab = String::Format( "tab {0}", kRow.iTabIdx );
				if( String::IsNullOrEmpty( sSub ) )
					sSub = String::Format( "sub {0}", kRow.iCsscEnum );

				if( false == m_kCatLabel->ContainsKey( kRow.iBillingCategoryNo ) )
				{
					m_kCatLabel->Add( kRow.iBillingCategoryNo, String::Format( "{0} / {1}", sTab, sSub ) );
					m_kCatTab->Add( kRow.iBillingCategoryNo, kRow.iTabIdx );
				}

				if( false == m_kTabName->ContainsKey( kRow.iTabIdx ) )
					m_kTabName->Add( kRow.iTabIdx, sTab );
			}
		}

		void BuildTabList()
		{
			m_bSuspendFilter = true;

			m_kTabList->Items->Clear();

			TabEntry^ kAll = gcnew TabEntry();
			kAll->TabIdx	= -1;
			kAll->Label		= String::Format( "All  ({0} row(s), every category)", m_kAllRows->Count );
			m_kTabList->Items->Add( kAll );

			if( NULL != m_pCatalog )
			{
				int iLastTab = -0x7fffffff;

				for( size_t u = 0; u != m_pCatalog->vecCategories.size(); ++u )
				{
					const SCashCategoryRow& kRow = m_pCatalog->vecCategories[u];
					if( kRow.iTabIdx == iLastTab )
						continue;

					iLastTab = kRow.iTabIdx;

					String^ sName = nullptr;
					m_kTabName->TryGetValue( kRow.iTabIdx, sName );

					TabEntry^ kEntry = gcnew TabEntry();
					kEntry->TabIdx	= kRow.iTabIdx;
					kEntry->Label	= String::Format( "{0}  ({1} row(s))",
						( nullptr != sName ) ? sName : kRow.iTabIdx.ToString(),
						CountInTab( kRow.iTabIdx ) );

					m_kTabList->Items->Add( kEntry );
				}
			}

			m_bSuspendFilter = false;

			if( m_kTabList->Items->Count > 0 )
				m_kTabList->SelectedIndex = 0;
		}

		int CountInTab( int iTabIdx )
		{
			int iCount = 0;
			for each( ProductRow^ kRow in m_kAllRows )
			{
				if( kRow->TabIdx == iTabIdx )
					++iCount;
			}
			return iCount;
		}

		int CountWhere( SubFilterKind eKind, int iBillingNo )
		{
			int iCount = 0;
			for each( ProductRow^ kRow in m_kAllRows )
			{
				if( Keep( kRow, eKind, iBillingNo, 0 ) )
					++iCount;
			}
			return iCount;
		}

		void BuildSubList()
		{
			m_bSuspendFilter = true;
			m_kSubList->Items->Clear();

			TabEntry^ kTab = safe_cast<TabEntry^>( m_kTabList->SelectedItem );
			const int iTabIdx = ( nullptr != kTab ) ? kTab->TabIdx : -1;

			if( iTabIdx < 0 )
			{
				// The "All" pseudo-tab, and its report. Each of the last
				// three is a row the game will not show, or will show
				// wrongly, and the only place they are visible.
				AddSub( SubFilterKind::Everything, 0, -1,
					String::Format( "everything  ({0})", m_kAllRows->Count ) );

				AddSub( SubFilterKind::Orphaned, 0, -1,
					String::Format( "orphaned - no tab shows it  ({0})",
						CountWhere( SubFilterKind::Orphaned, 0 ) ) );

				AddSub( SubFilterKind::Dropped, 0, -1,
					String::Format( "dropped - no item templet  ({0})",
						CountWhere( SubFilterKind::Dropped, 0 ) ) );

				AddSub( SubFilterKind::OutOfRange, 0, -1,
					String::Format( "out of range - not 1..127  ({0})",
						CountWhere( SubFilterKind::OutOfRange, 0 ) ) );
			}
			else
			{
				AddSub( SubFilterKind::WholeTab, 0, iTabIdx,
					String::Format( "all of this tab  ({0})", CountInTab( iTabIdx ) ) );

				for( size_t u = 0; NULL != m_pCatalog && u != m_pCatalog->vecCategories.size(); ++u )
				{
					const SCashCategoryRow& kRow = m_pCatalog->vecCategories[u];
					if( kRow.iTabIdx != iTabIdx )
						continue;

					String^ sSub = NativeBridge::Utf8( kRow.strSubName );
					if( String::IsNullOrEmpty( sSub ) )
						sSub = String::Format( "sub {0}", kRow.iCsscEnum );

					AddSub( SubFilterKind::Category, kRow.iBillingCategoryNo, iTabIdx,
						String::Format( "{0,-34} -> {1,3}  ({2})",
							sSub, kRow.iBillingCategoryNo,
							CountWhere( SubFilterKind::Category, kRow.iBillingCategoryNo ) ) );
				}
			}

			m_bSuspendFilter = false;

			if( m_kSubList->Items->Count > 0 )
				m_kSubList->SelectedIndex = 0;
		}

		void AddSub( SubFilterKind eKind, int iBillingNo, int iTabIdx, String^ sLabel )
		{
			SubEntry^ kEntry = gcnew SubEntry();
			kEntry->Kind		= eKind;
			kEntry->BillingNo	= iBillingNo;
			kEntry->TabIdx		= iTabIdx;
			kEntry->Label		= sLabel;
			m_kSubList->Items->Add( kEntry );
		}

		bool Keep( ProductRow^ kRow, SubFilterKind eKind, int iBillingNo, int iTabIdx )
		{
			switch( eKind )
			{
			case SubFilterKind::Everything:		return true;
			case SubFilterKind::Category:		return kRow->CategoryNo == iBillingNo;
			case SubFilterKind::WholeTab:		return kRow->TabIdx == iTabIdx;
			case SubFilterKind::Orphaned:		return kRow->TabIdx < 0;
			case SubFilterKind::Dropped:		return false == kRow->HasTemplet;

			case SubFilterKind::OutOfRange:
				return kRow->CategoryNo < CASH_FIELD_MIN || kRow->CategoryNo > CASH_FIELD_MAX
					|| kRow->Quantity   < CASH_FIELD_MIN || kRow->Quantity   > CASH_FIELD_MAX;
			}

			return true;
		}

		void ApplyFilter( int iKeepProductNo )
		{
			if( m_bSuspendFilter )
				return;

			SubEntry^ kSub = safe_cast<SubEntry^>( m_kSubList->SelectedItem );

			System::Collections::Generic::List<ProductRow^>^ kKept =
				gcnew System::Collections::Generic::List<ProductRow^>();

			if( nullptr == kSub )
			{
				kKept->AddRange( m_kAllRows );
			}
			else
			{
				for each( ProductRow^ kRow in m_kAllRows )
				{
					if( Keep( kRow, kSub->Kind, kSub->BillingNo, kSub->TabIdx ) )
						kKept->Add( kRow );
				}
			}

			m_kGrid->SetRows( kKept, iKeepProductNo );

			m_kFilterLabel->Text = String::Format( "showing {0} of {1} row(s){2}",
				kKept->Count, m_kAllRows->Count,
				( nullptr != kSub ) ? String::Format( "   -   {0}", kSub->Label->Trim() ) : String::Empty );
		}

		//////////////////////////////////////////////////////////////////
		// Reading the save.

		void ReloadFromDb( int iKeepProductNo )
		{
			std::string strError;

			std::vector<SCashProductRow> vecProducts;
			if( false == m_pDb->LoadProducts( vecProducts, strError ) )
			{
				Say( String::Format( "ERROR reading cash_product: {0}", NativeBridge::Utf8( strError ) ) );
				return;
			}

			// The same join the --db report makes, so the counts this
			// window shows and the counts phase 3 printed are one number
			// produced once.
			SCashCatalogReport kReport;
			BuildCashCatalogReport( vecProducts, m_pCatalog->vecItems, m_pCatalog->vecCategories, kReport );

			m_kAllRows = gcnew System::Collections::Generic::List<ProductRow^>();

			for( size_t u = 0; u != vecProducts.size(); ++u )
			{
				const SCashProductRow& kSrc = vecProducts[u];

				ProductRow^ kRow = gcnew ProductRow();
				kRow->ProductNo		= kSrc.iProductNo;
				kRow->ItemID		= kSrc.iItemID;
				kRow->CategoryNo	= kSrc.iCategoryNo;
				kRow->Quantity		= kSrc.iQuantity;
				kRow->Price			= kSrc.iPrice;
				kRow->IsEvent		= kSrc.iIsEvent;

				const SItemRow* pItem = FindItem( kSrc.iItemID );
				if( NULL != pItem )
				{
					kRow->Name			= NativeBridge::Utf8( pItem->strName );
					kRow->ShopImage		= NativeBridge::Utf8( pItem->strShopImage );
					kRow->HasTemplet	= true;
				}
				else
				{
					kRow->Name			= String::Format( "(item {0} - no client templet)", kSrc.iItemID );
					kRow->ShopImage		= String::Empty;
					kRow->HasTemplet	= false;
				}

				int iTabIdx = -1;
				m_kCatTab->TryGetValue( kSrc.iCategoryNo, iTabIdx );
				kRow->TabIdx = m_kCatTab->ContainsKey( kSrc.iCategoryNo ) ? iTabIdx : -1;

				String^ sLabel = nullptr;
				kRow->CategoryLabel = m_kCatLabel->TryGetValue( kSrc.iCategoryNo, sLabel )
					? sLabel : "(no tab shows this)";

				m_kAllRows->Add( kRow );
			}

			int iWallet = 0;
			if( m_pDb->GetWallet( iWallet, strError ) )
				m_kWalletBox->Text = iWallet.ToString();

			m_kSaveLabel->Text = String::Format(
				"{0}\r\nuser_version {1}   {2} product(s)   {3} the game will show   {4} dropped   {5} orphaned in {6} categor(ies)",
				msclr::interop::marshal_as<String^>( m_pDb->Path() ),
				m_pDb->UserVersion(), kReport.iRows, kReport.iKept, kReport.iDropped,
				kReport.iOrphanRows, kReport.iOrphanCategories );

			// Rebuilt every reload so the per-tab counts follow an insert
			// or a delete rather than going stale the first time one runs.
			const int iTab = m_kTabList->SelectedIndex;
			const int iSub = m_kSubList->SelectedIndex;

			BuildTabList();

			if( iTab > 0 && iTab < m_kTabList->Items->Count )
				m_kTabList->SelectedIndex = iTab;
			if( iSub > 0 && iSub < m_kSubList->Items->Count )
				m_kSubList->SelectedIndex = iSub;

			ApplyFilter( iKeepProductNo );

			// The load announces itself for the same reason every write
			// does: this window has a console behind it and that console
			// is the audit log. It is also the only thing that would say
			// so if the tab lists came back empty.
			Say( String::Format(
				"{0} product(s) loaded, {1} tab(s) from CashShopCategory.lua, {2} sub-categor(ies)."
				"   {3} of them the game will not show.",
				m_kAllRows->Count, m_kTabList->Items->Count - 1,
				(int) m_pCatalog->vecCategories.size(), kReport.iDropped ) );
		}

		const SItemRow* FindItem( int iItemID )
		{
			// The catalog is sorted by item id (ExtractItemCatalog and
			// CIndexCache::Load both guarantee it), so this is a binary
			// search rather than 48,754 comparisons per row.
			size_t uLow		= 0;
			size_t uHigh	= m_pCatalog->vecItems.size();

			while( uLow < uHigh )
			{
				const size_t uMid = uLow + ( uHigh - uLow ) / 2;

				if( m_pCatalog->vecItems[uMid].iItemID < iItemID )
					uLow = uMid + 1;
				else
					uHigh = uMid;
			}

			if( uLow < m_pCatalog->vecItems.size() && m_pCatalog->vecItems[uLow].iItemID == iItemID )
				return &m_pCatalog->vecItems[uLow];

			return NULL;
		}

		//////////////////////////////////////////////////////////////////
		// Actions. Every one of them writes its result to the status bar
		// AND to stdout: the console this tool keeps is its audit log, and
		// a tool that writes into the only copy of the character should
		// leave a trail that survives the window being closed.

		void Say( String^ sText )
		{
			m_kStatus->Text = sText;
			Console::WriteLine( sText );
		}

		void SayWithBackup( String^ sText )
		{
			String^ sBackup = String::Empty;

			if( m_pDb->BackupTaken() && false == m_pDb->BackupFiles().empty() )
			{
				sBackup = String::Format( "\r\nbackup this session: {0}",
					msclr::interop::marshal_as<String^>( m_pDb->BackupFiles()[0] ) );
			}

			Say( sText + sBackup );
		}

		void OnSelectionChanged( Object^ /*kSender*/, EventArgs^ /*kArgs*/ )
		{
			const bool bHave = ( nullptr != m_kGrid->Selected() );
			m_kEditButton->Enabled		= bHave;
			m_kDeleteButton->Enabled	= bHave;
		}

		void OnTabChanged( Object^ /*kSender*/, EventArgs^ /*kArgs*/ )
		{
			if( m_bSuspendFilter )
				return;

			BuildSubList();
			ApplyFilter( 0 );
		}

		void OnSubChanged( Object^ /*kSender*/, EventArgs^ /*kArgs*/ )
		{
			ApplyFilter( 0 );
		}

		void OnRefresh( Object^ /*kSender*/, EventArgs^ /*kArgs*/ )
		{
			ProductRow^ kSelected = m_kGrid->Selected();
			ReloadFromDb( ( nullptr != kSelected ) ? kSelected->ProductNo : 0 );
			Say( "reloaded from the save." );
		}

		void OnEdit( Object^ /*kSender*/, EventArgs^ /*kArgs*/ )
		{
			ProductRow^ kRow = m_kGrid->Selected();
			if( nullptr == kRow )
				return;

			// A COPY, not the cached bitmap. The grid repaints behind a
			// modal dialog, each repaint calls IconProvider::Get, and a
			// Get that evicts disposes the Bitmap it drops - which would
			// be the one the dialog's PictureBox is still holding.
			System::Drawing::Bitmap^ kIcon = m_kIcons->Get( kRow->ShopImage );
			if( nullptr != kIcon )
				kIcon = gcnew System::Drawing::Bitmap( kIcon );

			EditProductForm^ kDialog = gcnew EditProductForm(
				kRow->ProductNo, kRow->ItemID, kRow->Name, kRow->ShopImage, kIcon,
				kRow->CategoryNo, kRow->Quantity, kRow->Price, ( 0 != kRow->IsEvent ),
				CategoryChoices(), m_pDb );

			if( ::DialogResult::OK != kDialog->ShowDialog( this ) )
			{
				Say( String::Format( "product {0} left unchanged.", kRow->ProductNo ) );
				return;
			}

			SCashProductRow kWrite;
			kWrite.iProductNo	= kRow->ProductNo;
			kWrite.iItemID		= kRow->ItemID;
			kWrite.iCategoryNo	= kDialog->Category;
			kWrite.iQuantity	= kDialog->Quantity;
			kWrite.iPrice		= kDialog->Price;
			kWrite.iIsEvent		= kDialog->IsEvent ? 1 : 0;

			String^ sBefore = String::Format( "category {0}, qty {1}, price {2}, event {3}",
				kRow->CategoryNo, kRow->Quantity, kRow->Price, kRow->IsEvent );

			std::string strError;
			if( false == m_pDb->Update( kWrite, strError ) )
			{
				Say( String::Format( "REFUSED - product {0} not changed: {1}",
					kRow->ProductNo, NativeBridge::Utf8( strError ) ) );
				return;
			}

			// Reload FIRST: ReloadFromDb ends by announcing itself, and
			// the write is the thing the reader needs left on screen.
			ReloadFromDb( kRow->ProductNo );

			SayWithBackup( String::Format(
				"product {0} (item {1}) updated: {2}  ->  category {3}, qty {4}, price {5}, event {6}."
				"   Restart the game to see it.",
				kRow->ProductNo, kRow->ItemID, sBefore,
				kWrite.iCategoryNo, kWrite.iQuantity, kWrite.iPrice, kWrite.iIsEvent ) );
		}

		void OnDelete( Object^ /*kSender*/, EventArgs^ /*kArgs*/ )
		{
			ProductRow^ kRow = m_kGrid->Selected();
			if( nullptr == kRow )
				return;

			String^ sAsk = String::Format(
				"Delete product {0}?\r\n\r\nitem {1}  {2}\r\ncategory {3}  {4}\r\nquantity {5}, price {6}"
				"\r\n\r\nThis writes to els_db.sql. A backup of the whole WAL set is taken into db_backup\\ "
				"before the first write of this session.",
				kRow->ProductNo, kRow->ItemID, kRow->Name, kRow->CategoryNo, kRow->CategoryLabel,
				kRow->Quantity, kRow->Price );

			if( ::DialogResult::Yes != MessageBox::Show( this, sAsk, "Delete product",
					MessageBoxButtons::YesNo, MessageBoxIcon::Warning, MessageBoxDefaultButton::Button2 ) )
			{
				Say( String::Format( "product {0} kept.", kRow->ProductNo ) );
				return;
			}

			std::string strError;
			if( false == m_pDb->Delete( kRow->ProductNo, strError ) )
			{
				Say( String::Format( "REFUSED - product {0} not deleted: {1}",
					kRow->ProductNo, NativeBridge::Utf8( strError ) ) );
				return;
			}

			ReloadFromDb( 0 );

			SayWithBackup( String::Format( "product {0} (item {1}, {2}) deleted.   Restart the game to see it.",
				kRow->ProductNo, kRow->ItemID, kRow->Name ) );
		}

		void OnSetWallet( Object^ /*kSender*/, EventArgs^ /*kArgs*/ )
		{
			int iValue = 0;
			if( false == Int32::TryParse( m_kWalletBox->Text->Trim(), iValue ) )
			{
				Say( "REFUSED - the wallet must be a number." );
				return;
			}

			std::string strError;
			if( false == m_pDb->SetWallet( iValue, strError ) )
			{
				Say( String::Format( "REFUSED - the wallet was not changed: {0}",
					NativeBridge::Utf8( strError ) ) );

				int iCurrent = 0;
				std::string strIgnored;
				if( m_pDb->GetWallet( iCurrent, strIgnored ) )
					m_kWalletBox->Text = iCurrent.ToString();

				return;
			}

			SayWithBackup( String::Format( "settings.cash_start set to {0}."
				"   The shop compares against it but never deducts it.", iValue ) );
		}

		// "11  CSC_FASHION / CSSC_FASHION_WEAPON" per legal billing
		// category, for the edit dialog's combo. The number is first so
		// the dialog can parse it back out of free text.
		System::Collections::Generic::List<String^>^ CategoryChoices()
		{
			System::Collections::Generic::List<String^>^ kChoices =
				gcnew System::Collections::Generic::List<String^>();

			if( NULL == m_pCatalog )
				return kChoices;

			for( size_t u = 0; u != m_pCatalog->vecCategories.size(); ++u )
			{
				const SCashCategoryRow& kRow = m_pCatalog->vecCategories[u];

				String^ sLabel = nullptr;
				m_kCatLabel->TryGetValue( kRow.iBillingCategoryNo, sLabel );

				kChoices->Add( String::Format( "{0}  {1}", kRow.iBillingCategoryNo,
					( nullptr != sLabel ) ? sLabel : String::Empty ) );
			}

			return kChoices;
		}

		//////////////////////////////////////////////////////////////////

		CCashDb*				m_pDb;
		const SExtractResult*	m_pCatalog;

		IconProvider^										m_kIcons;
		System::Collections::Generic::List<ProductRow^>^		m_kAllRows;

		System::Collections::Generic::Dictionary<int, String^>^	m_kCatLabel;
		System::Collections::Generic::Dictionary<int, int>^		m_kCatTab;
		System::Collections::Generic::Dictionary<int, String^>^	m_kTabName;

		ProductGridPanel^	m_kGrid;
		ListBox^			m_kTabList;
		ListBox^			m_kSubList;
		Label^				m_kSaveLabel;
		Label^				m_kStatus;
		Label^				m_kFilterLabel;
		TextBox^			m_kWalletBox;
		Button^				m_kWalletButton;
		Button^				m_kEditButton;
		Button^				m_kDeleteButton;
		Label^				m_kWalletLabel;
		Panel^				m_kHeader;

		System::Drawing::Font^	m_kFont;
		System::Drawing::Font^	m_kFixed;

		// Set while the lists are being rebuilt, so the SelectedIndex
		// changes that rebuilding causes do not each trigger a filter pass
		// over 2,360 rows.
		bool	m_bSuspendFilter;
	};
}
