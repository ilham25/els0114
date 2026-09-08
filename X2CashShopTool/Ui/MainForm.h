//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phases 4, 5 and 6 - the main window. Tabs and
// sub-tabs from CashShopCategory.lua as phase 1 parsed it, the product
// grid over cash_product, Add / Edit / Delete, the wallet, the Problems
// group holding the rows the game will not show, and phase 6's details
// pane and technical-details toggle.
// See CASH_SHOP_TOOL_PLAN.md, phases 4, 5 and 6.
//
// Everything here is managed. The native side - the save file, the item
// catalog, the icon store and the label store - is reached only through
// the four pointers the constructor is handed, which Main.cpp owns and
// outlives the window. No Lua call is ever made from this file, so no C++
// exception from luaconf.h's throw can unwind a managed frame (plan
// sections 6 and 7).
//
// Two rules this window is built around, both of them the client's:
//
//   * category and quantity are narrowed to a signed char on the wire, so
//     1..127 is enforced and never clamped - see EditProductForm.h, where
//     the refusal lives;
//   * the game reads cash_product ONCE per process, so an edit made here
//     shows up in the game only after the game is restarted. The window
//     says that in as many words rather than letting a correct write look
//     like a failed one. The file:line for it is in the details pane, not
//     in the header - phase 6 moved it, because the rule is what the
//     reader needs and the citation is for whoever doubts it.
//
// PHASE 6's RULE, and the one a later tidy-up is most likely to undo:
// nothing in the DEFAULT view spells a value the way a header spells it.
// No CSC_*, no IT_*, no EP_*, no file:line. Every label comes from
// LabelBridge, which carries the text and its origin together, and the
// View -> Technical details toggle brings phase 5's presentation back
// verbatim rather than deleting it - the dense view is this tool's
// diagnostic value and is what made the 388 invisible rows individually
// reachable in the first place.
//
// Numbers are NOT labels and never get replaced by one: product_no,
// item_id, category, quantity and price are what gets written to the save,
// so a label sits beside them and the details pane always shows them
// whether the toggle is on or off.
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <msclr/marshal.h>
#include <msclr/marshal_cppstd.h>
#include <string>
#include <vector>

#include "../Core/CashDb.h"
#include "../Core/DdsDecode.h"
#include "../Core/IconStore.h"
#include "../Core/IndexCache.h"		// DefaultSettingsPath / ReadSettingBool / WriteSettingBool
#include "../Core/ItemIndex.h"
#include "../Core/Labels.h"

#include "UiBridge.h"
#include "EditProductForm.h"
#include "ItemPickerForm.h"

using namespace System;
using namespace System::Windows::Forms;

namespace X2CashShopTool
{
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

		// True = the item is a package component declared with bShowItem
		// false, so GetAllCashItemList filters it out of the shop AFTER
		// the catalog packet has carried it (X2ItemManager.cpp:1868 and
		// :2866-2880). Phase 5 found this; it is the second of the two
		// silent-drop rules and, unlike the missing-templet one, it is
		// invisible even in offline_server.log.
		bool	PackageHidden;

		// What the item IS, off the catalog row - phase 6 keeps these so
		// the grid can say "Costume - Shoes - Rare" instead of repeating
		// the product's own numbers back at the reader.
		int		ItemType;
		int		ItemGrade;
		int		EquipPosition;
		bool	IsFashion;

		// The two presentations of the second line, built once at load.
		// Plain is the default view; Tech is phase 5's line verbatim.
		String^	PlainLine;
		String^	TechLine;

		// "Costumes / Costume weapon" and "CSC_FASHION / CSSC_FASHION_WEAPON".
		String^	CategoryPlain;
		String^	CategoryTech;

		ProductRow()
		: ProductNo( 0 ), ItemID( 0 ), CategoryNo( 0 ), Quantity( 1 ), Price( 1 ), IsEvent( 0 )
		, Name( String::Empty ), ShopImage( String::Empty )
		, HasTemplet( false ), TabIdx( -1 ), PackageHidden( false )
		, ItemType( 0 ), ItemGrade( 0 ), EquipPosition( 0 ), IsFashion( false )
		, PlainLine( String::Empty ), TechLine( String::Empty )
		, CategoryPlain( String::Empty ), CategoryTech( String::Empty )
		{}

		// True when the game will not show this row, for any of the three
		// reasons the tool knows about. Drives the Problems group.
		bool IsProblem( int iMin, int iMax )
		{
			return ( false == HasTemplet )
				|| ( TabIdx < 0 )
				|| PackageHidden
				|| CategoryNo < iMin || CategoryNo > iMax
				|| Quantity   < iMin || Quantity   > iMax;
		}
	};

	//////////////////////////////////////////////////////////////////////
	// The grid. One owner-drawn panel painting only the visible rows -
	// the same discipline phase 2's icon wall settled on, and the one
	// phase 5's picker over 48,754 items needed. A Control per row is a
	// window handle per row.

	ref class ProductGridPanel : public Panel
	{
	public:
		ProductGridPanel( IconProvider^ kIcons, LabelBridge^ kLabels )
		{
			m_kIcons	= kIcons;
			m_kLabels	= kLabels;
			m_kRows		= gcnew System::Collections::Generic::List<ProductRow^>();
			m_iSelected	= -1;
			m_iRowHeight= 72;
			m_bTechnical= false;

			SetStyle( ControlStyles::OptimizedDoubleBuffer
				| ControlStyles::AllPaintingInWmPaint
				| ControlStyles::UserPaint
				| ControlStyles::Selectable, true );

			AutoScroll	= true;
			TabStop		= true;
			BackColor	= System::Drawing::Color::FromArgb( 32, 32, 36 );

			// Segoe UI for the words, Consolas ONLY where digits have to
			// line up into a column. Phase 5 used Consolas for both, which
			// is why every list read like a log file.
			m_kNameFont		= gcnew System::Drawing::Font( "Segoe UI", 9.5f, System::Drawing::FontStyle::Bold );
			m_kMetaFont		= gcnew System::Drawing::Font( "Segoe UI", 8.25f );
			m_kTechFont		= gcnew System::Drawing::Font( "Consolas", 8.0f );
			m_kNumberFont	= gcnew System::Drawing::Font( "Consolas", 10.0f, System::Drawing::FontStyle::Bold );
			m_kBadgeFont	= gcnew System::Drawing::Font( "Segoe UI", 7.0f );

			m_kLinePen		= gcnew System::Drawing::Pen( System::Drawing::Color::FromArgb( 52, 52, 58 ), 1.0f );
			m_kSelectBrush	= gcnew System::Drawing::SolidBrush( System::Drawing::Color::FromArgb( 48, 66, 96 ) );
			m_kDroppedBrush	= gcnew System::Drawing::SolidBrush( System::Drawing::Color::FromArgb( 60, 34, 34 ) );

			// One StringFormat per alignment, made once: the ellipsis is
			// what keeps a long item name from running under the price
			// column when the window is at its 900 px minimum.
			m_kRight = gcnew System::Drawing::StringFormat();
			m_kRight->Alignment	= System::Drawing::StringAlignment::Far;
			m_kRight->Trimming	= System::Drawing::StringTrimming::None;
			m_kRight->FormatFlags = System::Drawing::StringFormatFlags::NoWrap;

			m_kClip = gcnew System::Drawing::StringFormat();
			m_kClip->Trimming		= System::Drawing::StringTrimming::EllipsisCharacter;
			m_kClip->FormatFlags	= System::Drawing::StringFormatFlags::NoWrap;

			m_kToolTip = gcnew ToolTip();
			m_kToolTip->InitialDelay	= 400;
			m_kToolTip->AutoPopDelay	= 20000;
			m_iHoverIndex = -1;
		}

		event EventHandler^	SelectionChanged;
		event EventHandler^	EditRequested;
		event EventHandler^	DeleteRequested;
		event EventHandler^	AddRequested;

		void SetTechnical( bool bTechnical )
		{
			m_bTechnical = bTechnical;
			Invalidate();
		}

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
				g->DrawString( "Nothing here.", m_kNameFont,
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

			case Keys::Insert:
				AddRequested( this, EventArgs::Empty );
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

		// The hover tooltip keeps every number, in both views: it is the
		// cheapest place to check a product_no against the save and it
		// costs nothing to leave there.
		String^ Describe( ProductRow^ kRow )
		{
			return String::Format(
				"{0}\r\n\r\nproduct {1}   item {2}\r\ncategory {3}  ({4})"
				"\r\nquantity {5}   price {6} {7}{8}{9}",
				kRow->Name, kRow->ProductNo, kRow->ItemID,
				kRow->CategoryNo, kRow->CategoryPlain,
				kRow->Quantity, kRow->Price, m_kLabels->Currency,
				( 0 != kRow->IsEvent ) ? "\r\nmarked as an event product" : "",
				kRow->HasTemplet
					? ( kRow->PackageHidden
						? "\r\n\r\nTHE SHOP WILL NOT SHOW THIS ROW: the item is declared a package"
						  " component that is not sold on its own."
						: "" )
					: "\r\n\r\nTHE GAME WILL DROP THIS ROW: it has no item of that id." );
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

			//////////////////////////////////////////////////////////////
			// The words. Clipped to whatever is left after the number
			// column, so a long name ellipsizes instead of running under
			// the price at the 900 px minimum width.

			const int	iTextLeft	= 80;
			const int	iRight		= ClientSize.Width - 12;
			const float	fTextWidth	= (float) Math::Max( 60, iRight - 140 - iTextLeft );

			g->DrawString( kRow->Name, m_kNameFont,
				kRow->HasTemplet ? System::Drawing::Brushes::Gainsboro : System::Drawing::Brushes::LightCoral,
				System::Drawing::RectangleF( (float) iTextLeft, (float)( iY + 6 ), fTextWidth, 20.0f ),
				m_kClip );

			g->DrawString(
				m_bTechnical ? kRow->TechLine : kRow->PlainLine,
				m_bTechnical ? m_kTechFont : m_kMetaFont,
				System::Drawing::Brushes::Silver,
				System::Drawing::RectangleF( (float) iTextLeft, (float)( iY + 28 ), fTextWidth, 20.0f ),
				m_kClip );

			// The third line is only ever a warning, so it stays in the
			// same place and in the same two colours in both views.
			String^ sNote = nullptr;
			System::Drawing::Brush^ kNoteBrush = System::Drawing::Brushes::Goldenrod;

			if( false == kRow->HasTemplet )
			{
				sNote		= "the game drops this: there is no item with that id";
				kNoteBrush	= System::Drawing::Brushes::IndianRed;
			}
			else if( kRow->TabIdx < 0 )
			{
				sNote = "no tab in the shop shows this category";
			}
			else if( kRow->PackageHidden )
			{
				sNote = "the shop hides this item: it is a package component, not sold on its own";
			}

			if( nullptr != sNote )
			{
				g->DrawString( sNote, m_kMetaFont, kNoteBrush,
					System::Drawing::RectangleF( (float) iTextLeft, (float)( iY + 47 ), fTextWidth, 20.0f ),
					m_kClip );
			}

			//////////////////////////////////////////////////////////////
			// The numbers, right-aligned so a column of prices reads as a
			// column, and in the game's own currency word rather than
			// phase 5's ED - which was simply wrong (STR_ID_34).

			System::Drawing::RectangleF kPrice( (float)( iRight - 130 ), (float)( iY + 10 ), 130.0f, 20.0f );
			g->DrawString( String::Format( "{0:N0} {1}", kRow->Price, m_kLabels->Currency ),
				m_kNumberFont, System::Drawing::Brushes::Khaki, kPrice, m_kRight );

			System::Drawing::RectangleF kQty( (float)( iRight - 130 ), (float)( iY + 33 ), 130.0f, 20.0f );
			g->DrawString( String::Format( "x{0}", kRow->Quantity ), m_kTechFont,
				System::Drawing::Brushes::Silver, kQty, m_kRight );

			// A SMALL MARK, not the loudest thing on the row. m_bEvent
			// appears nowhere in X2Lib or KTDXLIB outside Offline/ - the
			// emulator sets it (X2OfflineCashShop.cpp:172) and no client
			// code reads it - so phase 5's orange badge was giving the
			// one field with no in-game effect top billing.
			if( 0 != kRow->IsEvent )
			{
				System::Drawing::RectangleF kEvent( (float)( iRight - 130 ), (float)( iY + 50 ), 130.0f, 16.0f );
				g->DrawString( "event", m_kBadgeFont,
					System::Drawing::Brushes::DarkKhaki, kEvent, m_kRight );
			}
		}

		IconProvider^												m_kIcons;
		LabelBridge^												m_kLabels;
		System::Collections::Generic::List<ProductRow^>^				m_kRows;
		int															m_iSelected;
		int															m_iRowHeight;
		int															m_iHoverIndex;
		bool														m_bTechnical;

		System::Drawing::Font^			m_kNameFont;
		System::Drawing::Font^			m_kMetaFont;
		System::Drawing::Font^			m_kTechFont;
		System::Drawing::Font^			m_kNumberFont;
		System::Drawing::Font^			m_kBadgeFont;
		System::Drawing::Pen^			m_kLinePen;
		System::Drawing::SolidBrush^	m_kSelectBrush;
		System::Drawing::SolidBrush^	m_kDroppedBrush;
		System::Drawing::StringFormat^	m_kRight;
		System::Drawing::StringFormat^	m_kClip;
		ToolTip^						m_kToolTip;
	};

	//////////////////////////////////////////////////////////////////////
	// What the left-hand lists hold. Plain carriers with a ToString, so
	// the ListBoxes need no owner-draw of their own. Each holds BOTH
	// presentations, so the technical toggle is a repaint of the lists and
	// not a reload of the save.

	ref class TabEntry
	{
	public:
		int		TabIdx;			// -1 = every product, -2 = the Problems group
		String^	Plain;
		String^	Tech;
		bool	Technical;

		virtual String^ ToString() override	{ return Technical ? Tech : Plain; }
	};

	// Which rows a sub-list selection keeps. The five beyond Category are
	// the diagnostics: they exist so a row that is invisible in the game
	// is reachable in the tool, which is the whole reason the tool can say
	// anything useful about a catalog nobody chose.
	enum class SubFilterKind
	{
		Everything	= 0,
		Category,
		WholeTab,
		AnyProblem,		// phase 6: the union of the four below, so the group has one count
		Orphaned,		// category matches no CashShopCategory.lua pair
		Dropped,		// item has no client templet
		OutOfRange,		// category or quantity outside 1..127
		PackageHidden,	// the item is a package component the shop filters out (phase 5)
	};

	ref class SubEntry
	{
	public:
		SubFilterKind	Kind;
		int				BillingNo;
		int				TabIdx;
		String^			Plain;
		String^			Tech;
		bool			Technical;

		virtual String^ ToString() override	{ return Technical ? Tech : Plain; }
	};

	//////////////////////////////////////////////////////////////////////

	ref class MainForm : public Form
	{
	public:
		// pDb must already be open read-write, and pCatalog is phase 1's
		// item catalog with SetKnownItems already handed over. pLabels may
		// have failed to load its strings, in which case every label falls
		// back to this tool's wording and the status bar says so once.
		// Main.cpp owns all four and outlives the window.
		MainForm( CCashDb* pDb, CIconStore* pIcons, const SExtractResult* pCatalog,
					const CLabelStore* pLabels, bool bTechnical )
		{
			m_pDb		= pDb;
			m_pCatalog	= pCatalog;

			m_kLabels	= gcnew LabelBridge( pCatalog, pLabels );
			m_kIcons	= gcnew IconProvider( pIcons, 1500 );
			m_kAllRows	= gcnew System::Collections::Generic::List<ProductRow^>();

			m_bTechnical		= bTechnical;

			// Built on the first Add, not here: it marshals 48,754 items
			// and most sessions open this window to change a price.
			m_kCatalogView		= nullptr;
			m_bPackageDataKnown	= ( NULL != pCatalog ) && pCatalog->bPackageDataRan;

			Text			= "X2CashShopTool - the offline cash shop";
			ClientSize		= System::Drawing::Size( 1180, 760 );
			MinimumSize		= System::Drawing::Size( 900, 560 );
			StartPosition	= FormStartPosition::CenterScreen;
			BackColor		= System::Drawing::Color::FromArgb( 32, 32, 36 );
			ForeColor		= System::Drawing::Color::Gainsboro;

			BuildCategoryMaps();
			BuildChrome();

			ReloadFromDb( 0 );

			// Said once, at the end of the load, so it is the line left on
			// screen rather than being scrolled off by the load report.
			if( false == m_kLabels->HasStrings )
				Say( m_kLabels->Degraded );
		}

		//////////////////////////////////////////////////////////////////
		// Phase 6's exit test 3, as a MEASUREMENT.
		//
		// "Resize to the 900x560 minimum and to maximized: nothing
		// overlaps, nothing is clipped, the grid and both lists still
		// scroll to their last row." Phase 5's scroll-extent probe is the
		// precedent for not trusting that by eye - and for the trap that a
		// probe on an UNSHOWN form measures nothing, because a Panel with
		// AutoScroll has no scrollbars and no laid-out children until it
		// is on a visible form.
		//
		// So: shown off-screen at the given client size, then every
		// container in the tree checked for two things a hand-rolled
		// layout gets wrong and a TableLayoutPanel should not -
		//
		//   * two visible siblings whose bounds intersect;
		//   * a visible child sticking out of its parent's client area,
		//     or squeezed to zero in either direction.
		//
		// Returns the number of complaints and prints each one, so a clean
		// run is a number rather than an impression.
		int MeasureLayout( int iWidth, int iHeight, int% iRowsOut, int% iCanvasOut, int% iReachedOut )
		{
			StartPosition	= FormStartPosition::Manual;
			Location		= System::Drawing::Point( -32000, -32000 );

			Show();
			Application::DoEvents();

			ClientSize = System::Drawing::Size( iWidth, iHeight );
			Application::DoEvents();

			int iComplaints = CheckContainer( this, "MainForm" );

			//////////////////////////////////////////////////////////////
			// The grid still has to reach its last row at this size.

			const int iCanvas = m_kGrid->AutoScrollMinSize.Height;

			m_kGrid->AutoScrollPosition = System::Drawing::Point( 0, iCanvas );
			Application::DoEvents();

			// AutoScrollPosition reads back NEGATED - the WinForms oddity
			// phase 5 documented.
			const int iReached = -m_kGrid->AutoScrollPosition.Y;

			iRowsOut	= m_kGrid->RowCount();
			iCanvasOut	= iCanvas;
			iReachedOut	= iReached + m_kGrid->ClientSize.Height;

			m_kGrid->AutoScrollPosition = System::Drawing::Point( 0, 0 );

			Hide();
			return iComplaints;
		}

	private:
		int CheckContainer( Control^ kParent, String^ sPath )
		{
			int iComplaints = 0;

			System::Collections::Generic::List<Control^>^ kKids =
				gcnew System::Collections::Generic::List<Control^>();

			for each( Control^ kChild in kParent->Controls )
			{
				if( kChild->Visible )
					kKids->Add( kChild );
			}

			System::Drawing::Rectangle kInside = kParent->ClientRectangle;

			for( int i = 0; i < kKids->Count; ++i )
			{
				Control^ kChild = kKids[i];
				String^ sChild = String::Format( "{0} / {1}", sPath, Describe( kChild ) );

				if( kChild->Width <= 0 || kChild->Height <= 0 )
				{
					Console::WriteLine( "    COLLAPSED  {0}  is {1}x{2}", sChild, kChild->Width, kChild->Height );
					++iComplaints;
				}
				else if( false == kInside.Contains( kChild->Bounds ) )
				{
					// A scrolled container legitimately holds children
					// outside its client area; nothing in this window
					// does, so it is reported rather than excused.
					Console::WriteLine( "    CLIPPED    {0}  {1}  outside  {2}",
						sChild, kChild->Bounds.ToString(), kInside.ToString() );
					++iComplaints;
				}

				for( int j = i + 1; j < kKids->Count; ++j )
				{
					if( kChild->Bounds.IntersectsWith( kKids[j]->Bounds ) )
					{
						Console::WriteLine( "    OVERLAP    {0}  {1}   and   {2}  {3}",
							Describe( kChild ), kChild->Bounds.ToString(),
							Describe( kKids[j] ), kKids[j]->Bounds.ToString() );
						++iComplaints;
					}
				}

				iComplaints += CheckContainer( kChild, sChild );
			}

			return iComplaints;
		}

		static String^ Describe( Control^ kControl )
		{
			String^ sText = kControl->Text;
			if( sText->Length > 22 )
				sText = sText->Substring( 0, 22 );

			sText = sText->Replace( "\r\n", " " );

			return String::Format( "{0}({1})", kControl->GetType()->Name, sText );
		}

		//////////////////////////////////////////////////////////////////
		// Layout.
		//
		// Phase 5 laid this out with fixed Bounds plus a hand-rolled
		// LayoutHeader() re-running on every Resize, which worked and was
		// one control away from overlapping at any moment. Phase 6 uses
		// TableLayoutPanels instead: the only pixel numbers left are
		// column and row sizes, and nothing is positioned relative to
		// anything else.
		//
		// Docked controls are laid out from the LAST entry in Controls to
		// the first, so the Fill control has to be added FIRST or it takes
		// the whole form and the edges land on top of it.

		void BuildChrome()
		{
			m_kFont		= gcnew System::Drawing::Font( "Segoe UI", 9.0f );
			m_kSmall	= gcnew System::Drawing::Font( "Segoe UI", 8.25f );
			m_kFixed	= gcnew System::Drawing::Font( "Consolas", 8.5f );

			// BuildStatus BEFORE BuildMain: it is what creates
			// m_kFilterLabel now, and ApplyFilter writes to that during
			// the ReloadFromDb the constructor ends with.
			Control^ kStatus	= BuildStatus();
			Control^ kMain		= BuildMain();
			Control^ kHeader	= BuildHeader();
			Control^ kMenu		= BuildMenu();

			Controls->Add( kMain );
			Controls->Add( kStatus );
			Controls->Add( kHeader );
			Controls->Add( kMenu );

			MainMenuStrip = m_kMenu;
		}

		//////////////////////////////////////////////////////////////////
		// The menu, which is the only new chrome phase 6 adds. The
		// technical toggle lives here rather than as a checkbox in the
		// header because it is a view mode, not a filter, and putting it
		// beside the filters would suggest it changed which rows are
		// shown.

		Control^ BuildMenu()
		{
			m_kMenu = gcnew MenuStrip();
			m_kMenu->Dock		= DockStyle::Top;
			m_kMenu->BackColor	= System::Drawing::Color::FromArgb( 24, 24, 28 );
			m_kMenu->ForeColor	= System::Drawing::Color::Gainsboro;
			m_kMenu->Renderer	= gcnew ToolStripProfessionalRenderer();
			m_kMenu->Font		= m_kFont;

			ToolStripMenuItem^ kView = gcnew ToolStripMenuItem( "&View" );

			m_kTechnicalItem = gcnew ToolStripMenuItem( "&Technical details" );
			m_kTechnicalItem->CheckOnClick	= true;
			m_kTechnicalItem->Checked		= m_bTechnical;
			m_kTechnicalItem->ShortcutKeys	= (Keys)( Keys::Control | Keys::T );
			m_kTechnicalItem->ToolTipText	= "Put the script's own names and the row's numbers back on screen.";
			m_kTechnicalItem->CheckedChanged += gcnew EventHandler( this, &MainForm::OnTechnicalToggled );
			kView->DropDownItems->Add( m_kTechnicalItem );

			kView->DropDownItems->Add( gcnew ToolStripSeparator() );

			ToolStripMenuItem^ kRefresh = gcnew ToolStripMenuItem( "&Reload from the save" );
			kRefresh->ShortcutKeys	= Keys::F5;
			kRefresh->Click			+= gcnew EventHandler( this, &MainForm::OnRefresh );
			kView->DropDownItems->Add( kRefresh );

			ToolStripMenuItem^ kHelp = gcnew ToolStripMenuItem( "&Help" );

			ToolStripMenuItem^ kKeys = gcnew ToolStripMenuItem( "&Keyboard shortcuts..." );
			kKeys->Click += gcnew EventHandler( this, &MainForm::OnShowKeys );
			kHelp->DropDownItems->Add( kKeys );

			ToolStripMenuItem^ kAbout = gcnew ToolStripMenuItem( "&Where these words come from..." );
			kAbout->Click += gcnew EventHandler( this, &MainForm::OnShowAbout );
			kHelp->DropDownItems->Add( kAbout );

			m_kMenu->Items->Add( kView );
			m_kMenu->Items->Add( kHelp );

			return m_kMenu;
		}

		//////////////////////////////////////////////////////////////////
		// The header. Phase 5 spent 96 pixels and three lines of goldenrod
		// prose here, two of them a citation. One line survives - the one
		// that is a rule the reader has to know - and the citation moved
		// into the details pane and the About box.

		Control^ BuildHeader()
		{
			TableLayoutPanel^ kHeader = gcnew TableLayoutPanel();
			kHeader->Dock			= DockStyle::Top;
			kHeader->Height			= 36;
			kHeader->BackColor		= System::Drawing::Color::FromArgb( 24, 24, 28 );
			kHeader->ColumnCount	= 4;
			kHeader->RowCount		= 1;
			kHeader->Padding		= System::Windows::Forms::Padding( 10, 5, 10, 5 );

			kHeader->ColumnStyles->Add( gcnew ColumnStyle( SizeType::Percent,  100.0f ) );
			kHeader->ColumnStyles->Add( gcnew ColumnStyle( SizeType::AutoSize ) );
			kHeader->ColumnStyles->Add( gcnew ColumnStyle( SizeType::Absolute, 120.0f ) );
			kHeader->ColumnStyles->Add( gcnew ColumnStyle( SizeType::Absolute, 62.0f ) );

			Label^ kNotice = gcnew Label();
			kNotice->Dock		= DockStyle::Fill;
			kNotice->TextAlign	= System::Drawing::ContentAlignment::MiddleLeft;
			kNotice->Font		= m_kFont;
			kNotice->ForeColor	= System::Drawing::Color::Goldenrod;
			kNotice->AutoEllipsis = true;
			kNotice->Text		= "Restart the game to see anything you change here.";
			kHeader->Controls->Add( kNotice, 0, 0 );

			m_kWalletLabel = gcnew Label();
			m_kWalletLabel->AutoSize	= true;
			m_kWalletLabel->Dock		= DockStyle::Fill;
			m_kWalletLabel->TextAlign	= System::Drawing::ContentAlignment::MiddleRight;
			m_kWalletLabel->Font		= m_kFont;
			m_kWalletLabel->ForeColor	= System::Drawing::Color::Gainsboro;
			m_kWalletLabel->Margin		= System::Windows::Forms::Padding( 0, 0, 8, 0 );
			kHeader->Controls->Add( m_kWalletLabel, 1, 0 );

			m_kWalletBox = gcnew TextBox();
			m_kWalletBox->Dock			= DockStyle::Fill;
			m_kWalletBox->Font			= m_kFixed;
			m_kWalletBox->BackColor		= System::Drawing::Color::FromArgb( 32, 32, 36 );
			m_kWalletBox->ForeColor		= System::Drawing::Color::Gainsboro;
			m_kWalletBox->BorderStyle	= ::BorderStyle::FixedSingle;
			m_kWalletBox->TextAlign		= HorizontalAlignment::Right;
			kHeader->Controls->Add( m_kWalletBox, 2, 0 );

			m_kWalletButton = gcnew Button();
			m_kWalletButton->Dock		= DockStyle::Fill;
			m_kWalletButton->Text		= "Set";
			m_kWalletButton->Font		= m_kFont;
			m_kWalletButton->FlatStyle	= ::FlatStyle::Flat;
			m_kWalletButton->ForeColor	= System::Drawing::Color::Gainsboro;
			m_kWalletButton->Margin		= System::Windows::Forms::Padding( 4, 0, 0, 0 );
			m_kWalletButton->Click		+= gcnew EventHandler( this, &MainForm::OnSetWallet );
			kHeader->Controls->Add( m_kWalletButton, 3, 0 );

			return kHeader;
		}

		Control^ BuildStatus()
		{
			TableLayoutPanel^ kBar = gcnew TableLayoutPanel();
			kBar->Dock			= DockStyle::Bottom;
			kBar->Height		= 46;
			kBar->BackColor		= System::Drawing::Color::FromArgb( 20, 20, 24 );
			kBar->ColumnCount	= 2;
			kBar->RowCount		= 1;

			kBar->ColumnStyles->Add( gcnew ColumnStyle( SizeType::Percent,  100.0f ) );
			kBar->ColumnStyles->Add( gcnew ColumnStyle( SizeType::Absolute, 150.0f ) );
			kBar->RowStyles->Add( gcnew RowStyle( SizeType::Percent, 100.0f ) );

			m_kStatus = gcnew Label();
			m_kStatus->Dock			= DockStyle::Fill;
			m_kStatus->Margin		= System::Windows::Forms::Padding( 0 );
			m_kStatus->Padding		= System::Windows::Forms::Padding( 12, 5, 6, 5 );
			m_kStatus->Font			= m_kSmall;
			m_kStatus->ForeColor	= System::Drawing::Color::FromArgb( 170, 200, 170 );
			kBar->Controls->Add( m_kStatus, 0, 0 );

			// The row count moved here from the button row. It belongs
			// next to the audit line - that is where the reader already
			// is - and taking it out of the button row is half of what
			// makes that row impossible to clip.
			m_kFilterLabel = gcnew Label();
			m_kFilterLabel->Dock		= DockStyle::Fill;
			m_kFilterLabel->Margin		= System::Windows::Forms::Padding( 0 );
			m_kFilterLabel->Padding		= System::Windows::Forms::Padding( 0, 5, 12, 5 );
			m_kFilterLabel->TextAlign	= System::Drawing::ContentAlignment::TopRight;
			m_kFilterLabel->Font		= m_kSmall;
			m_kFilterLabel->AutoEllipsis= true;
			m_kFilterLabel->ForeColor	= System::Drawing::Color::FromArgb( 160, 160, 168 );
			kBar->Controls->Add( m_kFilterLabel, 1, 0 );

			return kBar;
		}

		//////////////////////////////////////////////////////////////////
		// Three columns: the tab lists, the grid, the details pane. The
		// two edge columns are Absolute and the middle one Percent, so a
		// maximized window gives every extra pixel to the grid and the
		// 900 px minimum still leaves it 360.

		Control^ BuildMain()
		{
			TableLayoutPanel^ kMain = gcnew TableLayoutPanel();
			kMain->Dock			= DockStyle::Fill;
			kMain->ColumnCount	= 3;
			kMain->RowCount		= 1;

			kMain->ColumnStyles->Add( gcnew ColumnStyle( SizeType::Absolute, 240.0f ) );
			kMain->ColumnStyles->Add( gcnew ColumnStyle( SizeType::Percent,  100.0f ) );
			kMain->ColumnStyles->Add( gcnew ColumnStyle( SizeType::Absolute, 300.0f ) );
			kMain->RowStyles->Add( gcnew RowStyle( SizeType::Percent, 100.0f ) );

			kMain->Controls->Add( BuildLists(),		0, 0 );
			kMain->Controls->Add( BuildGrid(),		1, 0 );
			kMain->Controls->Add( BuildDetails(),	2, 0 );

			return kMain;
		}

		Control^ BuildLists()
		{
			TableLayoutPanel^ kLeft = gcnew TableLayoutPanel();
			kLeft->Dock			= DockStyle::Fill;
			kLeft->Margin		= System::Windows::Forms::Padding( 0 );
			kLeft->BackColor	= System::Drawing::Color::FromArgb( 24, 24, 28 );
			kLeft->ColumnCount	= 1;
			kLeft->RowCount		= 4;

			kLeft->ColumnStyles->Add( gcnew ColumnStyle( SizeType::Percent, 100.0f ) );
			kLeft->RowStyles->Add( gcnew RowStyle( SizeType::Absolute,  22.0f ) );
			kLeft->RowStyles->Add( gcnew RowStyle( SizeType::Absolute, 200.0f ) );
			kLeft->RowStyles->Add( gcnew RowStyle( SizeType::Absolute,  22.0f ) );
			kLeft->RowStyles->Add( gcnew RowStyle( SizeType::Percent,  100.0f ) );

			// The headings name the THING, not the file it was parsed
			// from. Phase 5's read "tab  (CashShopCategory.lua)" and
			// "sub-category  ->  cash_product.category".
			kLeft->Controls->Add( MakeHeading( "Shop tabs" ), 0, 0 );

			m_kTabList = MakeList();
			m_kTabList->SelectedIndexChanged += gcnew EventHandler( this, &MainForm::OnTabChanged );
			kLeft->Controls->Add( m_kTabList, 0, 1 );

			m_kSubHeading = MakeHeading( "Inside this tab" );
			kLeft->Controls->Add( m_kSubHeading, 0, 2 );

			m_kSubList = MakeList();
			m_kSubList->SelectedIndexChanged += gcnew EventHandler( this, &MainForm::OnSubChanged );
			kLeft->Controls->Add( m_kSubList, 0, 3 );

			return kLeft;
		}

		Control^ BuildGrid()
		{
			Panel^ kMiddle = gcnew Panel();
			kMiddle->Dock	= DockStyle::Fill;
			kMiddle->Margin	= System::Windows::Forms::Padding( 0 );

			m_kGrid = gcnew ProductGridPanel( m_kIcons, m_kLabels );
			m_kGrid->Dock = DockStyle::Fill;
			m_kGrid->SetTechnical( m_bTechnical );
			m_kGrid->SelectionChanged	+= gcnew EventHandler( this, &MainForm::OnSelectionChanged );
			m_kGrid->EditRequested		+= gcnew EventHandler( this, &MainForm::OnEdit );
			m_kGrid->DeleteRequested	+= gcnew EventHandler( this, &MainForm::OnDelete );
			m_kGrid->AddRequested		+= gcnew EventHandler( this, &MainForm::OnAdd );

			//////////////////////////////////////////////////////////////
			// The buttons. Clean captions; the key hints are in tooltips
			// and in the status line, and the keys themselves keep working
			// - IsInputKey, Selectable and the Focus() on mouse-down in
			// ProductGridPanel are all load-bearing and phases 4 and 5
			// both had to discover them.
			//
			// A WRAPPING FlowLayoutPanel, and that is not a style choice.
			// The first draft of this row was a TableLayoutPanel with four
			// absolute columns, and --layout-test caught it: at the
			// 900x560 minimum the grid column is 360 px wide, four
			// buttons plus their margins want 402, and the last button
			// and the row count were simply cut off. Absolute columns in a
			// container narrower than their sum clip in silence. A
			// FlowLayoutPanel that wraps and auto-sizes cannot: it takes a
			// second line instead, and the row grows to fit.

			FlowLayoutPanel^ kButtons = gcnew FlowLayoutPanel();
			kButtons->Dock			= DockStyle::Bottom;
			kButtons->AutoSize		= true;
			kButtons->AutoSizeMode	= System::Windows::Forms::AutoSizeMode::GrowAndShrink;
			kButtons->WrapContents	= true;
			kButtons->BackColor		= System::Drawing::Color::FromArgb( 24, 24, 28 );
			kButtons->Padding		= System::Windows::Forms::Padding( 10, 8, 10, 8 );

			m_kEditButton	= MakeButton( "Edit",			"Change the price, quantity, tab or event mark.  (Enter, or double-click)",	kButtons );
			m_kDeleteButton	= MakeButton( "Delete",			"Take this product out of the shop.  (Del)",									kButtons );
			Button^ kAdd	= MakeButton( "Add product",	"Put another item on sale.  (Ins)",											kButtons );
			Button^ kRefresh= MakeButton( "Reload",			"Read the save again, in case something else changed it.  (F5)",				kButtons );

			m_kEditButton->Click	+= gcnew EventHandler( this, &MainForm::OnEdit );
			m_kDeleteButton->Click	+= gcnew EventHandler( this, &MainForm::OnDelete );
			kAdd->Click				+= gcnew EventHandler( this, &MainForm::OnAdd );
			kRefresh->Click			+= gcnew EventHandler( this, &MainForm::OnRefresh );

			kMiddle->Controls->Add( m_kGrid );
			kMiddle->Controls->Add( kButtons );

			return kMiddle;
		}

		//////////////////////////////////////////////////////////////////
		// The details pane: the SINGLE place every demoted technical fact
		// lives, and the reason the default view can afford to be plain.
		// Read-only multiline TextBoxes rather than Labels, for two
		// reasons - a TextBox scrolls instead of clipping, and a
		// product_no you can select is a product_no you can paste into a
		// sqlite3 query.

		Control^ BuildDetails()
		{
			TableLayoutPanel^ kRight = gcnew TableLayoutPanel();
			kRight->Dock		= DockStyle::Fill;
			kRight->Margin		= System::Windows::Forms::Padding( 0 );
			kRight->BackColor	= System::Drawing::Color::FromArgb( 24, 24, 28 );
			kRight->ColumnCount	= 1;
			kRight->RowCount	= 3;

			kRight->ColumnStyles->Add( gcnew ColumnStyle( SizeType::Percent, 100.0f ) );
			kRight->RowStyles->Add( gcnew RowStyle( SizeType::Absolute,  22.0f ) );
			kRight->RowStyles->Add( gcnew RowStyle( SizeType::Absolute, 116.0f ) );
			kRight->RowStyles->Add( gcnew RowStyle( SizeType::Percent,  100.0f ) );

			kRight->Controls->Add( MakeHeading( "The selected product" ), 0, 0 );

			// Consolas here, and ONLY here plus the grid's number column:
			// these are the values that get written to the save, and they
			// are worth aligning.
			m_kDetailNumbers = MakeReadOnlyBox( m_kFixed );
			kRight->Controls->Add( m_kDetailNumbers, 0, 1 );

			m_kDetailProse = MakeReadOnlyBox( m_kSmall );
			kRight->Controls->Add( m_kDetailProse, 0, 2 );

			return kRight;
		}

		TextBox^ MakeReadOnlyBox( System::Drawing::Font^ kFont )
		{
			TextBox^ kBox = gcnew TextBox();
			kBox->Dock			= DockStyle::Fill;
			kBox->Multiline		= true;
			kBox->ReadOnly		= true;
			kBox->WordWrap		= true;
			kBox->ScrollBars	= ScrollBars::Vertical;
			kBox->BorderStyle	= ::BorderStyle::None;
			kBox->BackColor		= System::Drawing::Color::FromArgb( 24, 24, 28 );
			kBox->ForeColor		= System::Drawing::Color::FromArgb( 190, 190, 196 );
			kBox->Font			= kFont;
			kBox->Margin		= System::Windows::Forms::Padding( 10, 2, 6, 6 );
			kBox->TabStop		= false;

			return kBox;
		}

		ListBox^ MakeList()
		{
			ListBox^ kList = gcnew ListBox();
			kList->Dock			= DockStyle::Fill;
			kList->Margin		= System::Windows::Forms::Padding( 0 );
			kList->Font			= m_kFont;
			kList->BackColor	= System::Drawing::Color::FromArgb( 24, 24, 28 );
			kList->ForeColor	= System::Drawing::Color::Gainsboro;
			kList->BorderStyle	= ::BorderStyle::None;
			kList->IntegralHeight = false;

			return kList;
		}

		Label^ MakeHeading( String^ sText )
		{
			Label^ kLabel = gcnew Label();
			kLabel->Dock		= DockStyle::Fill;
			kLabel->Margin		= System::Windows::Forms::Padding( 0 );
			kLabel->Padding		= System::Windows::Forms::Padding( 10, 4, 0, 0 );
			kLabel->Font		= gcnew System::Drawing::Font( "Segoe UI", 8.25f, System::Drawing::FontStyle::Bold );
			kLabel->ForeColor	= System::Drawing::Color::FromArgb( 140, 140, 150 );
			kLabel->BackColor	= System::Drawing::Color::FromArgb( 18, 18, 22 );
			kLabel->Text		= sText;
			return kLabel;
		}

		Button^ MakeButton( String^ sText, String^ sTip, Control^ kParent )
		{
			Button^ kButton = gcnew Button();
			kButton->AutoSize	= true;
			kButton->AutoSizeMode = System::Windows::Forms::AutoSizeMode::GrowAndShrink;
			kButton->MinimumSize= System::Drawing::Size( 80, 28 );
			kButton->Padding	= System::Windows::Forms::Padding( 8, 0, 8, 0 );
			kButton->Margin		= System::Windows::Forms::Padding( 0, 0, 8, 0 );
			kButton->Text		= sText;
			kButton->Font		= m_kFont;
			kButton->FlatStyle	= ::FlatStyle::Flat;
			kButton->ForeColor	= System::Drawing::Color::Gainsboro;

			if( nullptr == m_kTips )
			{
				m_kTips = gcnew ToolTip();
				m_kTips->InitialDelay	= 350;
				m_kTips->AutoPopDelay	= 15000;
			}

			m_kTips->SetToolTip( kButton, sTip );

			kParent->Controls->Add( kButton );
			return kButton;
		}

		//////////////////////////////////////////////////////////////////
		// The technical toggle. It changes only how things are SPELLED -
		// never which rows are shown - so it repaints and rebuilds the two
		// lists and touches neither the save nor the filter.

		void OnTechnicalToggled( Object^ /*kSender*/, EventArgs^ /*kArgs*/ )
		{
			m_bTechnical = m_kTechnicalItem->Checked;

			// DefaultSettingsPath() is called here rather than kept as a
			// String^ member: it hands back exactly the std::wstring
			// WriteSettingBool wants, and a managed copy would only have
			// to be marshalled straight back.
			WriteSettingBool( DefaultSettingsPath(), L"technical", m_bTechnical );

			m_kGrid->SetTechnical( m_bTechnical );

			RestyleLists();

			ProductRow^ kSelected = m_kGrid->Selected();
			ShowDetails( kSelected );

			Say( m_bTechnical
				? "Technical details on: the script's own names and the row's numbers are back."
				: "Technical details off." );
		}

		// Re-spelling the lists in place. Setting Technical on each entry
		// and forcing the ListBox to re-call ToString is cheaper and less
		// error-prone than rebuilding both lists and then trying to put
		// the selection back where it was.
		void RestyleLists()
		{
			RestyleOne( m_kTabList );
			RestyleOne( m_kSubList );

			m_kSubHeading->Text = m_bTechnical
				? "sub-category  ->  cash_product.category"
				: "Inside this tab";
		}

		void RestyleOne( ListBox^ kList )
		{
			for( int i = 0; i < kList->Items->Count; ++i )
			{
				TabEntry^ kTab = dynamic_cast<TabEntry^>( kList->Items[i] );
				if( nullptr != kTab )
					kTab->Technical = m_bTechnical;

				SubEntry^ kSub = dynamic_cast<SubEntry^>( kList->Items[i] );
				if( nullptr != kSub )
					kSub->Technical = m_bTechnical;
			}

			// A ListBox caches the string it got from ToString, so the
			// items have to be handed back to it for the new spelling to
			// appear at all.
			const int iSelected = kList->SelectedIndex;

			m_bSuspendFilter = true;

			array<Object^>^ akItems = gcnew array<Object^>( kList->Items->Count );
			kList->Items->CopyTo( akItems, 0 );

			kList->BeginUpdate();
			kList->Items->Clear();
			kList->Items->AddRange( akItems );
			kList->EndUpdate();

			if( iSelected >= 0 && iSelected < kList->Items->Count )
				kList->SelectedIndex = iSelected;

			m_bSuspendFilter = false;
		}

		//////////////////////////////////////////////////////////////////
		// Help.

		void OnShowKeys( Object^ /*kSender*/, EventArgs^ /*kArgs*/ )
		{
			MessageBox::Show( this,
				"In the product list:\r\n"
				"    Enter, or double-click     edit the highlighted product\r\n"
				"    Del                        delete it\r\n"
				"    Ins                        add a new product\r\n"
				"    arrows, Page Up/Down, Home/End    move about\r\n"
				"\r\n"
				"Anywhere:\r\n"
				"    F5          read the save again\r\n"
				"    Ctrl+T      technical details on or off\r\n"
				"\r\n"
				"In the item picker, Enter uses the highlighted item. There is no\r\n"
				"form-wide default button there on purpose, so Enter in the search\r\n"
				"box does not commit whatever happened to be selected.",
				"Keyboard shortcuts", MessageBoxButtons::OK, MessageBoxIcon::Information );
		}

		// Where the words come from, in the tool rather than in the plan,
		// because rule 1 of phase 6 is that this tool's own wording is
		// never presented as the game's - and that is only true if it is
		// auditable from inside the tool.
		void OnShowAbout( Object^ /*kSender*/, EventArgs^ /*kArgs*/ )
		{
			String^ sStrings = m_kLabels->HasStrings
				? String::Format( "General.ess: {0:N0} string(s) read from the game directory.",
					m_kLabels->StringCount )
				: String::Format( "General.ess: NOT READ.\r\n{0}", m_kLabels->Degraded );

			MessageBox::Show( this, String::Format(
				"Item and product names are the game's own, out of the shipped scripts.\r\n"
				"\r\n"
				"{0}\r\n"
				"\r\n"
				"Equip slots, item grades, the Costume flag and the {1} currency are the\r\n"
				"game's own localized strings, reached through the client's own switches.\r\n"
				"Item types come from a mapping in the client that this build compiles out.\r\n"
				"The shop's tab and sub-tab names are this tool's wording: the captions the\r\n"
				"game paints there are pictures, not text, so there is nothing to quote.\r\n"
				"\r\n"
				"Run the tool with --labels to print every label, every value, and where\r\n"
				"each one came from - including everything this tool worded itself.\r\n"
				"\r\n"
				"An edit shows up in the game only after the game is restarted, because\r\n"
				"CX2OfflineCashShop::EnsureLoaded reads cash_product once per process\r\n"
				"(X2Lib/Offline/X2OfflineCashShop.h:98).",
				sStrings, m_kLabels->Currency ),
				"Where these words come from", MessageBoxButtons::OK, MessageBoxIcon::Information );
		}

		//////////////////////////////////////////////////////////////////
		// The tab table, turned into something the lists can show.

		void BuildCategoryMaps()
		{
			m_kCatPlain	= gcnew System::Collections::Generic::Dictionary<int, String^>();
			m_kCatTech	= gcnew System::Collections::Generic::Dictionary<int, String^>();
			m_kCatTab	= gcnew System::Collections::Generic::Dictionary<int, int>();
			m_kTabPlain	= gcnew System::Collections::Generic::Dictionary<int, String^>();
			m_kTabTech	= gcnew System::Collections::Generic::Dictionary<int, String^>();
			m_kSubPlain	= gcnew System::Collections::Generic::Dictionary<int, String^>();
			m_kSubTech	= gcnew System::Collections::Generic::Dictionary<int, String^>();

			if( NULL == m_pCatalog )
				return;

			for( size_t u = 0; u != m_pCatalog->vecCategories.size(); ++u )
			{
				const SCashCategoryRow& kRow = m_pCatalog->vecCategories[u];

				// The CSC_*/CSSC_* names phase 4 reversed out of Enum.lua,
				// and the plain-language labels phase 6 pairs with them.
				String^ sTabTech = NativeBridge::Utf8( kRow.strTabName );
				String^ sSubTech = NativeBridge::Utf8( kRow.strSubName );

				LabelInfo^ kTabLabel = m_kLabels->CashCategory( kRow.strTabName );
				LabelInfo^ kSubLabel = m_kLabels->CashSubCategory( kRow.strSubName );

				String^ sTabPlain = kTabLabel->Text;
				String^ sSubPlain = kSubLabel->Text;

				if( String::IsNullOrEmpty( sTabTech ) )
					sTabTech = String::Format( "tab {0}", kRow.iTabIdx );
				if( String::IsNullOrEmpty( sSubTech ) )
					sSubTech = String::Format( "sub {0}", kRow.iCsscEnum );

				if( String::IsNullOrEmpty( sTabPlain ) )
					sTabPlain = sTabTech;
				if( String::IsNullOrEmpty( sSubPlain ) )
					sSubPlain = sSubTech;

				if( false == m_kCatPlain->ContainsKey( kRow.iBillingCategoryNo ) )
				{
					m_kCatPlain->Add( kRow.iBillingCategoryNo, String::Format( "{0} / {1}", sTabPlain, sSubPlain ) );
					m_kCatTech->Add(  kRow.iBillingCategoryNo, String::Format( "{0} / {1}", sTabTech,  sSubTech  ) );
					m_kCatTab->Add(   kRow.iBillingCategoryNo, kRow.iTabIdx );
					m_kSubPlain->Add( kRow.iBillingCategoryNo, sSubPlain );
					m_kSubTech->Add(  kRow.iBillingCategoryNo, sSubTech );
				}

				if( false == m_kTabPlain->ContainsKey( kRow.iTabIdx ) )
				{
					m_kTabPlain->Add( kRow.iTabIdx, sTabPlain );
					m_kTabTech->Add(  kRow.iTabIdx, sTabTech );
				}
			}
		}

		void BuildTabList()
		{
			m_bSuspendFilter = true;

			m_kTabList->Items->Clear();

			m_kTabList->Items->Add( MakeTab( -1,
				String::Format( "Everything  ({0})", m_kAllRows->Count ),
				String::Format( "All  ({0} row(s), every category)", m_kAllRows->Count ) ) );

			if( NULL != m_pCatalog )
			{
				int iLastTab = -0x7fffffff;

				for( size_t u = 0; u != m_pCatalog->vecCategories.size(); ++u )
				{
					const SCashCategoryRow& kRow = m_pCatalog->vecCategories[u];
					if( kRow.iTabIdx == iLastTab )
						continue;

					iLastTab = kRow.iTabIdx;

					String^ sPlain = nullptr;
					String^ sTech = nullptr;
					m_kTabPlain->TryGetValue( kRow.iTabIdx, sPlain );
					m_kTabTech->TryGetValue( kRow.iTabIdx, sTech );

					const int iCount = CountInTab( kRow.iTabIdx );

					m_kTabList->Items->Add( MakeTab( kRow.iTabIdx,
						String::Format( "{0}  ({1})", ( nullptr != sPlain ) ? sPlain : kRow.iTabIdx.ToString(), iCount ),
						String::Format( "{0}  ({1} row(s))", ( nullptr != sTech ) ? sTech : kRow.iTabIdx.ToString(), iCount ) ) );
				}
			}

			// PHASE 6 moved these out of the "All" tab's sub-list. Phase 5
			// mixed the four diagnostic buckets in with the real
			// sub-categories, so repricing a product navigated past
			// "dropped - no item templet" every time. They are their own
			// group now, and its count is visible without selecting
			// anything - which is the more useful half of the change.
			const int iProblems = CountWhere( SubFilterKind::AnyProblem, 0 );

			m_kTabList->Items->Add( MakeTab( -2,
				String::Format( "Problems  ({0})", iProblems ),
				String::Format( "Problems  ({0} row(s) the game will not show properly)", iProblems ) ) );

			m_bSuspendFilter = false;

			if( m_kTabList->Items->Count > 0 )
				m_kTabList->SelectedIndex = 0;
		}

		TabEntry^ MakeTab( int iTabIdx, String^ sPlain, String^ sTech )
		{
			TabEntry^ kEntry = gcnew TabEntry();
			kEntry->TabIdx		= iTabIdx;
			kEntry->Plain		= sPlain;
			kEntry->Tech		= sTech;
			kEntry->Technical	= m_bTechnical;
			return kEntry;
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

			if( -2 == iTabIdx )
			{
				//////////////////////////////////////////////////////////
				// The Problems group. Each of these is a row the game will
				// not show, or will show wrongly, and this list is the
				// only place any of them is visible - the 388 dropped rows
				// existed as one number in one log line until phase 4.

				AddSub( SubFilterKind::AnyProblem, 0, -2,
					String::Format( "Everything with a problem  ({0})", CountWhere( SubFilterKind::AnyProblem, 0 ) ),
					String::Format( "any problem  ({0})", CountWhere( SubFilterKind::AnyProblem, 0 ) ) );

				AddSub( SubFilterKind::Dropped, 0, -2,
					String::Format( "The game has no such item  ({0})", CountWhere( SubFilterKind::Dropped, 0 ) ),
					String::Format( "dropped - no item templet  ({0})", CountWhere( SubFilterKind::Dropped, 0 ) ) );

				AddSub( SubFilterKind::Orphaned, 0, -2,
					String::Format( "No tab shows it  ({0})", CountWhere( SubFilterKind::Orphaned, 0 ) ),
					String::Format( "orphaned - no tab shows it  ({0})", CountWhere( SubFilterKind::Orphaned, 0 ) ) );

				AddSub( SubFilterKind::OutOfRange, 0, -2,
					String::Format( "Tab or quantity out of range  ({0})", CountWhere( SubFilterKind::OutOfRange, 0 ) ),
					String::Format( "out of range - not 1..127  ({0})", CountWhere( SubFilterKind::OutOfRange, 0 ) ) );

				// Kept OUT of the dropped bucket on purpose: these rows
				// reach the client and are filtered by the shop, so they
				// are absent from offline_server.log's dropped count as
				// well.
				const int iHidden = CountWhere( SubFilterKind::PackageHidden, 0 );

				AddSub( SubFilterKind::PackageHidden, 0, -2,
					m_bPackageDataKnown
						? String::Format( "The shop hides it: package component  ({0})", iHidden )
						: String::Format( "Package rule UNKNOWN - the script did not run  ({0})", iHidden ),
					m_bPackageDataKnown
						? String::Format( "hidden - package component, bShowItem false  ({0})", iHidden )
						: String::Format( "hidden - package rule UNKNOWN, PackageItemData.lua did not run  ({0})", iHidden ) );
			}
			else if( iTabIdx < 0 )
			{
				AddSub( SubFilterKind::Everything, 0, -1,
					String::Format( "Every product in the shop  ({0})", m_kAllRows->Count ),
					String::Format( "everything  ({0})", m_kAllRows->Count ) );
			}
			else
			{
				const int iInTab = CountInTab( iTabIdx );

				AddSub( SubFilterKind::WholeTab, 0, iTabIdx,
					String::Format( "The whole tab  ({0})", iInTab ),
					String::Format( "all of this tab  ({0})", iInTab ) );

				for( size_t u = 0; NULL != m_pCatalog && u != m_pCatalog->vecCategories.size(); ++u )
				{
					const SCashCategoryRow& kRow = m_pCatalog->vecCategories[u];
					if( kRow.iTabIdx != iTabIdx )
						continue;

					String^ sPlain = nullptr;
					String^ sTech = nullptr;
					m_kSubPlain->TryGetValue( kRow.iBillingCategoryNo, sPlain );
					m_kSubTech->TryGetValue( kRow.iBillingCategoryNo, sTech );

					const int iCount = CountWhere( SubFilterKind::Category, kRow.iBillingCategoryNo );

					// The category number stays beside the label in BOTH
					// views. It is what gets written to the save, and rule
					// 2 of this phase is that a label never replaces a
					// number that is the contract.
					AddSub( SubFilterKind::Category, kRow.iBillingCategoryNo, iTabIdx,
						String::Format( "{0}  ->  {1}   ({2})",
							( nullptr != sPlain ) ? sPlain : kRow.iCsscEnum.ToString(),
							kRow.iBillingCategoryNo, iCount ),
						String::Format( "{0,-34} -> {1,3}  ({2})",
							( nullptr != sTech ) ? sTech : kRow.iCsscEnum.ToString(),
							kRow.iBillingCategoryNo, iCount ) );
				}
			}

			m_bSuspendFilter = false;

			if( m_kSubList->Items->Count > 0 )
				m_kSubList->SelectedIndex = 0;
		}

		void AddSub( SubFilterKind eKind, int iBillingNo, int iTabIdx, String^ sPlain, String^ sTech )
		{
			SubEntry^ kEntry = gcnew SubEntry();
			kEntry->Kind		= eKind;
			kEntry->BillingNo	= iBillingNo;
			kEntry->TabIdx		= iTabIdx;
			kEntry->Plain		= sPlain;
			kEntry->Tech		= sTech;
			kEntry->Technical	= m_bTechnical;
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
			case SubFilterKind::PackageHidden:	return kRow->PackageHidden;
			case SubFilterKind::AnyProblem:		return kRow->IsProblem( CASH_FIELD_MIN, CASH_FIELD_MAX );

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

			m_kFilterLabel->Text = String::Format( "showing {0:N0} of {1:N0}", kKept->Count, m_kAllRows->Count );
		}

		//////////////////////////////////////////////////////////////////
		// Reading the save.

		void ReloadFromDb( int iKeepProductNo )
		{
			std::string strError;

			std::vector<SCashProductRow> vecProducts;
			if( false == m_pDb->LoadProducts( vecProducts, strError ) )
			{
				Say( String::Format( "ERROR reading the catalog: {0}", NativeBridge::Utf8( strError ) ) );
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

					kRow->ItemType		= pItem->iItemType;
					kRow->ItemGrade		= pItem->iItemGrade;
					kRow->EquipPosition	= pItem->iEquipPosition;
					kRow->IsFashion		= ( 0 != pItem->iIsFashion );

					kRow->PackageHidden	= IsHiddenPackageItem(
						m_pCatalog->vecHiddenPackageItems, kSrc.iItemID );
				}
				else
				{
					kRow->Name			= String::Format( "item {0} - the game has no such item", kSrc.iItemID );
					kRow->ShopImage		= String::Empty;
					kRow->HasTemplet	= false;
				}

				int iTabIdx = -1;
				m_kCatTab->TryGetValue( kSrc.iCategoryNo, iTabIdx );
				kRow->TabIdx = m_kCatTab->ContainsKey( kSrc.iCategoryNo ) ? iTabIdx : -1;

				String^ sPlain = nullptr;
				String^ sTech = nullptr;
				kRow->CategoryPlain = m_kCatPlain->TryGetValue( kSrc.iCategoryNo, sPlain ) ? sPlain : "no tab shows this";
				kRow->CategoryTech  = m_kCatTech->TryGetValue( kSrc.iCategoryNo, sTech )   ? sTech  : "(no tab shows this)";

				BuildRowLines( kRow );

				m_kAllRows->Add( kRow );
			}

			int iWallet = 0;
			if( m_pDb->GetWallet( iWallet, strError ) )
				m_kWalletBox->Text = iWallet.ToString();

			m_kWalletLabel->Text = String::Format( "{0} to spend", m_kLabels->Currency );

			// iKept is what the catalog PACKET will carry, which is the
			// number offline_server.log prints and the number phase 3
			// checked against it. The package-hidden count is a further,
			// client-side filter on top of that and is reported apart from
			// it rather than folded in - see SExtractResult.
			m_sSaveLine = String::Format(
				"{0}\r\nsave format {1}\r\n{2:N0} product(s), {3:N0} the game will show, {4:N0} it will drop"
				"\r\n{5:N0} in no tab, in {6} categor(ies) that match none\r\n{7} hidden by the package rule",
				msclr::interop::marshal_as<String^>( m_pDb->Path() ),
				m_pDb->UserVersion(), kReport.iRows, kReport.iKept, kReport.iDropped,
				kReport.iOrphanRows, kReport.iOrphanCategories,
				m_bPackageDataKnown ? CountWhere( SubFilterKind::PackageHidden, 0 ).ToString() : "an unknown number of" );

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
				"{0:N0} product(s) loaded across {1} shop tab(s) and {2} sub-categor(ies)."
				"   {3:N0} of them the game will not show.   Enter edits, Ins adds, Del deletes.",
				m_kAllRows->Count, m_kTabList->Items->Count - 2,
				(int) m_pCatalog->vecCategories.size(), kReport.iDropped ) );
		}

		// The second line of a grid row, in both spellings, built once per
		// load rather than per repaint.
		//
		// The plain one is what the item IS - "Costume - Shoes - Rare" -
		// because the row already carries the product's numbers in the
		// details pane and repeating them here was the single densest
		// thing in phase 5's window.
		void BuildRowLines( ProductRow^ kRow )
		{
			kRow->TechLine = String::Format( "product {0}   item {1}   category {2}  {3}",
				kRow->ProductNo, kRow->ItemID, kRow->CategoryNo, kRow->CategoryTech );

			if( false == kRow->HasTemplet )
			{
				kRow->PlainLine = String::Format( "product {0}  -  in {1}", kRow->ProductNo, kRow->CategoryPlain );
				return;
			}

			System::Collections::Generic::List<String^>^ kParts =
				gcnew System::Collections::Generic::List<String^>();

			// The game's own word for m_bFashion, STR_ID_251. The tool
			// said "fashion" until this phase.
			if( kRow->IsFashion )
				kParts->Add( m_kLabels->Costume );

			LabelInfo^ kSlot = m_kLabels->EquipPosition( kRow->EquipPosition );
			LabelInfo^ kType = m_kLabels->ItemType( kRow->ItemType );
			LabelInfo^ kGrade = m_kLabels->ItemGrade( kRow->ItemGrade );

			// An unequippable item's slot says "not equipped", which is
			// true and is noise on 5,862 rows - so the item TYPE carries
			// the line for those and the slot carries it for the rest.
			if( 0 != kRow->EquipPosition && false == String::IsNullOrEmpty( kSlot->Text ) )
				kParts->Add( kSlot->Text );
			else if( false == String::IsNullOrEmpty( kType->Text ) )
				kParts->Add( kType->Text );

			if( false == String::IsNullOrEmpty( kGrade->Text ) )
				kParts->Add( kGrade->Text );

			kRow->PlainLine = String::Join( "   -   ", kParts->ToArray() );
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
		// The details pane. EVERY demoted fact lands here, and it is
		// visible with the technical toggle OFF - which is what makes the
		// plain default view safe rather than lossy.

		void ShowDetails( ProductRow^ kRow )
		{
			if( nullptr == kRow )
			{
				m_kDetailNumbers->Text	= "nothing selected";
				m_kDetailProse->Text	= String::Format( "This save\r\n{0}", m_sSaveLine );
				return;
			}

			m_kDetailNumbers->Text = String::Format(
				"product_no   {0}\r\nitem_id      {1}\r\ncategory     {2}\r\nquantity     {3}\r\nprice        {4}\r\nis_event     {5}",
				kRow->ProductNo, kRow->ItemID, kRow->CategoryNo,
				kRow->Quantity, kRow->Price, kRow->IsEvent );

			System::Text::StringBuilder^ kText = gcnew System::Text::StringBuilder();

			kText->AppendLine( kRow->Name );
			kText->AppendLine();

			kText->AppendLine( String::Format( "price   {0:N0} {1}   for {2}",
				kRow->Price, m_kLabels->Currency,
				( 1 == kRow->Quantity ) ? "one" : kRow->Quantity.ToString() ) );

			kText->AppendLine( String::Format( "tab     {0}   (category {1})",
				kRow->CategoryPlain, kRow->CategoryNo ) );

			kText->AppendLine( String::Format( "script  {0}", kRow->CategoryTech ) );
			kText->AppendLine( String::Format( "icon    {0}",
				String::IsNullOrEmpty( kRow->ShopImage ) ? "none - the shop draws its no-image icon" : kRow->ShopImage ) );

			if( kRow->HasTemplet )
			{
				kText->AppendLine();
				AppendLabel( kText, "type ", m_kLabels->ItemType( kRow->ItemType ) );
				AppendLabel( kText, "slot ", m_kLabels->EquipPosition( kRow->EquipPosition ) );
				AppendLabel( kText, "grade", m_kLabels->ItemGrade( kRow->ItemGrade ) );

				if( kRow->IsFashion )
				{
					kText->AppendLine( String::Format( "{0}   [{1}]   {2}",
						m_kLabels->Costume, m_kLabels->CostumeOrigin,
						"what the client itself prints for the item's costume flag" ) );
				}
			}

			//////////////////////////////////////////////////////////////
			// Whichever of the two silent-drop rules applies, with the
			// citation - which is what the header used to carry.

			if( false == kRow->HasTemplet )
			{
				kText->AppendLine();
				kText->AppendLine( "THE GAME WILL DROP THIS ROW. There is no item with that id, so the shop "
					"discards the row at load with nothing but a line in offline_server.log "
					"(X2Lib/Offline/X2OfflineCashShop.cpp:85-89)." );
			}
			else if( kRow->PackageHidden )
			{
				kText->AppendLine();
				kText->AppendLine( "THE SHOP WILL NOT SHOW THIS ROW. PackageItemData.lua declares this item a "
					"package component that is not sold on its own, and GetAllCashItemList drops it "
					"AFTER the catalog has already carried it (X2Lib/X2ItemManager.cpp:1868 and :2866-2880). "
					"Nothing in offline_server.log reports this one." );
			}
			else if( kRow->TabIdx < 0 )
			{
				kText->AppendLine();
				kText->AppendLine( "NO TAB SHOWS THIS ROW. Its category matches no pair in "
					"CashShopCategory.lua, so the row is valid and unreachable." );
			}

			if( kRow->CategoryNo < CASH_FIELD_MIN || kRow->CategoryNo > CASH_FIELD_MAX
				|| kRow->Quantity < CASH_FIELD_MIN || kRow->Quantity > CASH_FIELD_MAX )
			{
				kText->AppendLine();
				kText->AppendLine( String::Format( "OUT OF RANGE. Both category and quantity are narrowed to a "
					"signed char on the wire, so only {0}..{1} survives the trip "
					"(X2Lib/Offline/X2OfflineCashShop.cpp:166).",
					(int) CASH_FIELD_MIN, (int) CASH_FIELD_MAX ) );
			}

			if( 0 != kRow->IsEvent )
			{
				kText->AppendLine();
				kText->AppendLine( "Marked as an event product. Nothing in the game reads that flag: the "
					"offline server sets it and no client code looks at it." );
			}

			kText->AppendLine();
			kText->AppendLine( "An edit shows up in the game only after the game is restarted - the shop "
				"reads the catalog once per process (X2Lib/Offline/X2OfflineCashShop.h:98)." );

			kText->AppendLine();
			kText->AppendLine( "This save" );
			kText->AppendLine( m_sSaveLine );

			m_kDetailProse->Text = kText->ToString();
		}

		// "slot   Shoes   [ESS]   the client's own EQIP_POSITION switch..."
		// The origin tag is the point: it is what keeps this tool's own
		// wording from reading like the game's.
		void AppendLabel( System::Text::StringBuilder^ kText, String^ sField, LabelInfo^ kLabel )
		{
			if( nullptr == kLabel || String::IsNullOrEmpty( kLabel->Text ) )
				return;

			kText->AppendLine( String::Format( "{0}   {1}   [{2}]{3}{4}",
				sField, kLabel->Text, kLabel->OriginTag,
				String::IsNullOrEmpty( kLabel->Script ) ? String::Empty : String::Format( "   {0}", kLabel->Script ),
				( kLabel->StringID >= 0 ) ? String::Format( "   STR_ID_{0}", kLabel->StringID ) : String::Empty ) );
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
			ProductRow^ kRow = m_kGrid->Selected();

			const bool bHave = ( nullptr != kRow );
			m_kEditButton->Enabled		= bHave;
			m_kDeleteButton->Enabled	= bHave;

			ShowDetails( kRow );
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
			Say( "read the save again." );
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
				CategoryChoices(), m_pDb, false, PackageWarning( kRow->PackageHidden ),
				m_kLabels->Currency, m_bTechnical );

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

		//////////////////////////////////////////////////////////////////
		// Phase 5 - insert.
		//
		// Two dialogs in sequence, deliberately: the picker answers "which
		// item", the field dialog answers "on what terms". Splitting them
		// is what lets the field dialog stay the same one the edit path
		// uses, so the 1..127 rule and the refusal text exist once.

		String^ PackageWarning( bool bHidden )
		{
			if( false == bHidden )
				return String::Empty;

			return "THE SHOP WILL NOT SHOW THIS ITEM: it is declared a package component that is "
				"not sold on its own, and the shop drops those after the catalog has carried them.";
		}

		// Built on first use and kept. Marshalling 48,754 items costs real
		// milliseconds and most sessions never open the picker at all, so
		// paying it at startup would slow down the common case to speed up
		// the rare one.
		ItemCatalogView^ CatalogView()
		{
			if( nullptr == m_kCatalogView )
			{
				Say( "building the item list - 48,754 items, once per run..." );

				// DoEvents is here only so that line gets painted before
				// an 80 ms pause - and DISABLING THE FORM FIRST is what
				// makes it safe, because DoEvents pumps input: a second
				// click on Add would otherwise re-enter this handler and
				// start a second picker while the first was still
				// building. Restored in the finally, so an exception in
				// the build cannot leave the window dead.
				Enabled = false;
				Cursor = Cursors::WaitCursor;

				try
				{
					Application::DoEvents();
					m_kCatalogView = gcnew ItemCatalogView( m_pCatalog, m_kLabels );
				}
				finally
				{
					Cursor = Cursors::Default;
					Enabled = true;
				}

				Say( String::Format( "item list built: {0:N0} item(s) in {1} ms, {2:N0} KB.",
					m_kCatalogView->All->Length, m_kCatalogView->BuildMs,
					m_kCatalogView->ManagedBytes / 1024 ) );
			}

			// Refreshed every open: an insert made a moment ago changes
			// which items are already sold, and a stale "already sold"
			// badge is worse than none.
			System::Collections::Generic::HashSet<int>^ kSold =
				gcnew System::Collections::Generic::HashSet<int>();

			for each( ProductRow^ kRow in m_kAllRows )
				kSold->Add( kRow->ItemID );

			m_kCatalogView->MarkSold( kSold );

			return m_kCatalogView;
		}

		// The category the new product should default to: whatever the
		// current sub-tab is showing, so adding to a tab you are looking
		// at needs no thought. 0 on the Everything and Problems groups,
		// which the dialog shows as an empty combo and Validate then
		// refuses - the right outcome, because there is no sensible guess
		// and a silently-chosen category is how a product lands in a tab
		// that makes no sense.
		int CurrentCategory()
		{
			SubEntry^ kSub = safe_cast<SubEntry^>( m_kSubList->SelectedItem );

			if( nullptr != kSub && SubFilterKind::Category == kSub->Kind )
				return kSub->BillingNo;

			// A whole-tab selection has no single category; take the first
			// of that tab's, which is the one its sub-list shows first.
			const int iTabIdx = ( nullptr != kSub ) ? kSub->TabIdx : -1;

			if( iTabIdx >= 0 && NULL != m_pCatalog )
			{
				for( size_t u = 0; u != m_pCatalog->vecCategories.size(); ++u )
				{
					if( m_pCatalog->vecCategories[u].iTabIdx == iTabIdx )
						return m_pCatalog->vecCategories[u].iBillingCategoryNo;
				}
			}

			return 0;
		}

		void OnAdd( Object^ /*kSender*/, EventArgs^ /*kArgs*/ )
		{
			ItemPickerForm^ kPicker = gcnew ItemPickerForm( CatalogView(), m_kIcons, m_kLabels, m_bTechnical );

			if( ::DialogResult::OK != kPicker->ShowDialog( this ) )
			{
				Say( "nothing added." );
				return;
			}

			// A COPY, for the same reason the edit path takes one: the grid
			// repaints behind the modal dialog and a repaint that evicts
			// disposes the Bitmap the dialog is still showing.
			System::Drawing::Bitmap^ kIcon = m_kIcons->Get( kPicker->ItemShopImage );
			if( nullptr != kIcon )
				kIcon = gcnew System::Drawing::Bitmap( kIcon );

			EditProductForm^ kDialog = gcnew EditProductForm(
				0, kPicker->ItemID, kPicker->ItemName, kPicker->ItemShopImage, kIcon,
				CurrentCategory(), 1, 1, false,
				CategoryChoices(), m_pDb, true, PackageWarning( kPicker->HiddenPackage ),
				m_kLabels->Currency, m_bTechnical );

			if( ::DialogResult::OK != kDialog->ShowDialog( this ) )
			{
				Say( String::Format( "item {0} ({1}) not added.", kPicker->ItemID, kPicker->ItemName ) );
				return;
			}

			SCashProductRow kWrite;
			kWrite.iProductNo	= 0;			// allocated as max+1 inside Insert's transaction
			kWrite.iItemID		= kPicker->ItemID;
			kWrite.iCategoryNo	= kDialog->Category;
			kWrite.iQuantity	= kDialog->Quantity;
			kWrite.iPrice		= kDialog->Price;
			kWrite.iIsEvent		= kDialog->IsEvent ? 1 : 0;

			std::string strError;
			if( false == m_pDb->Insert( kWrite, strError ) )
			{
				Say( String::Format( "REFUSED - item {0} was not added: {1}",
					kPicker->ItemID, NativeBridge::Utf8( strError ) ) );
				return;
			}

			// Reload FIRST: ReloadFromDb ends by announcing itself, and
			// the write is the thing the reader needs left on screen.
			ReloadFromDb( kWrite.iProductNo );

			SayWithBackup( String::Format(
				"product {0} added: item {1} ({2}), category {3}, qty {4}, price {5}, event {6}."
				"{7}   Restart the game to see it.",
				kWrite.iProductNo, kWrite.iItemID, kPicker->ItemName,
				kWrite.iCategoryNo, kWrite.iQuantity, kWrite.iPrice, kWrite.iIsEvent,
				kPicker->HiddenPackage
					? "   THE SHOP WILL NOT SHOW IT: it is a package component, not sold on its own."
					: "" ) );
		}

		void OnDelete( Object^ /*kSender*/, EventArgs^ /*kArgs*/ )
		{
			ProductRow^ kRow = m_kGrid->Selected();
			if( nullptr == kRow )
				return;

			String^ sAsk = String::Format(
				"Delete product {0}?\r\n\r\n{1}\r\nitem {2}\r\ntab {3}  (category {4})\r\n"
				"quantity {5}, price {6} {7}"
				"\r\n\r\nThis writes to the save. A backup of the whole save is taken into db_backup\\ "
				"before the first write of this session.",
				kRow->ProductNo, kRow->Name, kRow->ItemID, kRow->CategoryPlain, kRow->CategoryNo,
				kRow->Quantity, kRow->Price, m_kLabels->Currency );

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
				Say( "REFUSED - the amount must be a number." );
				return;
			}

			std::string strError;
			if( false == m_pDb->SetWallet( iValue, strError ) )
			{
				Say( String::Format( "REFUSED - the amount was not changed: {0}",
					NativeBridge::Utf8( strError ) ) );

				int iCurrent = 0;
				std::string strIgnored;
				if( m_pDb->GetWallet( iCurrent, strIgnored ) )
					m_kWalletBox->Text = iCurrent.ToString();

				return;
			}

			SayWithBackup( String::Format( "{0} to spend set to {1:N0}."
				"   The shop compares against it but never takes anything off it.",
				m_kLabels->Currency, iValue ) );
		}

		// "11  Costumes / Costume weapon" per legal billing category, for
		// the edit dialog's combo, in whichever spelling is current. The
		// NUMBER IS FIRST in both, and that is load-bearing: the dialog
		// parses it back out of free text, and free text is what makes
		// "type 128 and watch it be refused" a testable claim.
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
				const bool bFound = m_bTechnical
					? m_kCatTech->TryGetValue( kRow.iBillingCategoryNo, sLabel )
					: m_kCatPlain->TryGetValue( kRow.iBillingCategoryNo, sLabel );

				kChoices->Add( String::Format( "{0}  {1}", kRow.iBillingCategoryNo,
					bFound ? sLabel : String::Empty ) );
			}

			return kChoices;
		}

		//////////////////////////////////////////////////////////////////

		CCashDb*				m_pDb;
		const SExtractResult*	m_pCatalog;

		LabelBridge^										m_kLabels;
		IconProvider^										m_kIcons;
		System::Collections::Generic::List<ProductRow^>^		m_kAllRows;

		// The picker's catalog, built on first Add. Null until then.
		ItemCatalogView^	m_kCatalogView;

		// False = PackageItemData.lua did not run, so "0 hidden rows"
		// means nobody could tell rather than that there are none. The
		// report says which.
		bool				m_bPackageDataKnown;

		// Phase 6. Persisted in X2CashShopTool.ini beside the index cache
		// - NOT in ItemIndex.db, which is a cache that gets thrown away
		// and rebuilt whenever an archive changes.
		bool				m_bTechnical;

		// The save's own summary, shown in the details pane rather than in
		// the header. Rebuilt on every load.
		String^				m_sSaveLine;

		System::Collections::Generic::Dictionary<int, String^>^	m_kCatPlain;
		System::Collections::Generic::Dictionary<int, String^>^	m_kCatTech;
		System::Collections::Generic::Dictionary<int, int>^		m_kCatTab;
		System::Collections::Generic::Dictionary<int, String^>^	m_kTabPlain;
		System::Collections::Generic::Dictionary<int, String^>^	m_kTabTech;
		System::Collections::Generic::Dictionary<int, String^>^	m_kSubPlain;
		System::Collections::Generic::Dictionary<int, String^>^	m_kSubTech;

		ProductGridPanel^	m_kGrid;
		ListBox^			m_kTabList;
		ListBox^			m_kSubList;
		Label^				m_kSubHeading;
		Label^				m_kStatus;
		Label^				m_kFilterLabel;
		TextBox^			m_kDetailNumbers;
		TextBox^			m_kDetailProse;
		TextBox^			m_kWalletBox;
		Button^				m_kWalletButton;
		Button^				m_kEditButton;
		Button^				m_kDeleteButton;
		Label^				m_kWalletLabel;
		MenuStrip^			m_kMenu;
		ToolStripMenuItem^	m_kTechnicalItem;
		ToolTip^			m_kTips;

		System::Drawing::Font^	m_kFont;
		System::Drawing::Font^	m_kSmall;
		System::Drawing::Font^	m_kFixed;

		// Set while the lists are being rebuilt, so the SelectedIndex
		// changes that rebuilding causes do not each trigger a filter pass
		// over 2,360 rows.
		bool	m_bSuspendFilter;
	};
}
