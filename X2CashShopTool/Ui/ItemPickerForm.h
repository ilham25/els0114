//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-07
// Description: X2CashShopTool phase 5 - the item picker, over the whole
// 48,754-item catalog. See CASH_SHOP_TOOL_PLAN.md, "Phase 5 - Insert,
// with the virtualized picker".
//
// 2,360 of the client's 48,754 items are purchasable and the other
// ~46,000 are unreachable; this window is the thing that reaches them.
//
// THE LIST IS AN OWNER-DRAWN PANEL PAINTING ONLY THE VISIBLE BAND, not a
// VirtualMode ListView as the plan's decision table says. Phase 4 settled
// on the panel for the 2,360-row product grid and its own note calls it
// "the one phase 5's picker needs over 48,754 rows", and the deciding
// argument is the icons: a virtual ListView can only draw an icon out of a
// pre-populated ImageList, so 48,754 rows means either 48,754 decoded
// bitmaps up front - 780 MB of 64x64 BGRA - or an owner-draw path on top
// of virtual mode anyway. The panel gets icons on demand through the same
// IconProvider the grid uses, and only the ~20 rows on screen are ever
// touched. See the decision note in the plan.
//
// Everything here is managed and no Lua call is ever made from this file,
// so no C++ exception from luaconf.h's throw can unwind a managed frame
// (plan sections 6 and 7). The catalog is read through the const
// SExtractResult* the constructor is handed, which Main.cpp owns.
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <msclr/marshal.h>
#include <msclr/marshal_cppstd.h>
#include <string>
#include <vector>

#include "../Core/CashDb.h"
#include "../Core/IconStore.h"
#include "../Core/ItemIndex.h"

#include "UiBridge.h"

using namespace System;
using namespace System::Windows::Forms;

namespace X2CashShopTool
{
	//////////////////////////////////////////////////////////////////////
	// One item, as the picker paints it.

	ref class PickerItem
	{
	public:
		int		ItemID;
		String^	Name;
		String^	ShopImage;
		int		ItemType;
		int		ItemGrade;
		int		EquipPosition;
		bool	IsFashion;

		// The shop will not show this item even with a valid cash_product
		// row: it is declared a package component with bShowItem false, so
		// GetAllCashItemList filters it out after the catalog packet has
		// carried it (X2ItemManager.cpp:1868 and :2866-2880). This is the
		// second silent-drop rule in the client and the reason the picker
		// says so before an insert rather than after a play-test.
		bool	HiddenPackage;

		// Already sold as at least one product. Refreshed from
		// cash_product every time the picker opens, because an insert made
		// in this session changes it.
		bool	AlreadySold;

		// The enum names, resolved once at construction so painting a row
		// costs no lookup.
		String^	TypeName;
		String^	EquipName;

		// Lower-cased once. Filtering 48,754 names on every keystroke is
		// the one thing in this window that could stutter, and comparing
		// against a prepared string is what keeps it from doing so.
		String^	NameLower;
	};

	//////////////////////////////////////////////////////////////////////
	// The catalog, marshalled once.
	//
	// Built by MainForm the first time the picker is opened and kept, so
	// re-opening the picker is instant and the 48,754 marshalling calls
	// happen once per run rather than once per insert. This is where the
	// picker's whole memory cost lives - reported in the status bar when
	// it is built, because "no growth in memory" is a claim that should
	// come with a number.

	ref class ItemCatalogView
	{
	public:
		ItemCatalogView( const SExtractResult* pCatalog )
		{
			System::Diagnostics::Stopwatch^ kWatch = System::Diagnostics::Stopwatch::StartNew();

			const long long iBefore = GC::GetTotalMemory( false );

			const size_t uCount = ( NULL != pCatalog ) ? pCatalog->vecItems.size() : 0;
			m_kAll = gcnew array<PickerItem^>( (int) uCount );

			for( size_t u = 0; u != uCount; ++u )
			{
				const SItemRow& kSrc = pCatalog->vecItems[u];

				PickerItem^ kItem = gcnew PickerItem();
				kItem->ItemID		= kSrc.iItemID;
				kItem->Name			= NativeBridge::Utf8( kSrc.strName );
				kItem->ShopImage	= NativeBridge::Utf8( kSrc.strShopImage );
				kItem->ItemType		= kSrc.iItemType;
				kItem->ItemGrade	= kSrc.iItemGrade;
				kItem->EquipPosition= kSrc.iEquipPosition;
				kItem->IsFashion	= ( 0 != kSrc.iIsFashion );

				kItem->HiddenPackage = IsHiddenPackageItem(
					pCatalog->vecHiddenPackageItems, kSrc.iItemID );

				kItem->AlreadySold	= false;

				kItem->TypeName		= NativeBridge::Utf8( LookupEnumRowName(
					pCatalog->vecEnumNames, EnumTableItemType(), kSrc.iItemType ) );
				kItem->EquipName	= NativeBridge::Utf8( LookupEnumRowName(
					pCatalog->vecEnumNames, EnumTableEquipPosition(), kSrc.iEquipPosition ) );

				kItem->NameLower	= kItem->Name->ToLowerInvariant();

				m_kAll[(int) u] = kItem;
			}

			m_bPackageDataKnown	= ( NULL != pCatalog ) && pCatalog->bPackageDataRan;
			m_iHiddenPackage	= ( NULL != pCatalog ) ? (int) pCatalog->vecHiddenPackageItems.size() : 0;

			BuildChoices( pCatalog );

			kWatch->Stop();

			m_iBuildMs		= (int) kWatch->ElapsedMilliseconds;
			m_iManagedBytes	= (int)( GC::GetTotalMemory( false ) - iBefore );
		}

		property array<PickerItem^>^	All			{ array<PickerItem^>^ get() { return m_kAll; } }
		property int					BuildMs		{ int get() { return m_iBuildMs; } }
		property int					ManagedBytes{ int get() { return m_iManagedBytes; } }
		property int					HiddenPackageCount	{ int get() { return m_iHiddenPackage; } }
		property bool					PackageDataKnown	{ bool get() { return m_bPackageDataKnown; } }

		// The distinct ITEM_TYPE and EQIP_POSITION values that actually
		// occur, with the script's own names. Built from the catalog rather
		// than from the enum tables so the two filters never offer a value
		// no item has.
		property System::Collections::Generic::List<String^>^	TypeChoices
			{ System::Collections::Generic::List<String^>^ get() { return m_kTypeChoices; } }
		property System::Collections::Generic::List<int>^		TypeValues
			{ System::Collections::Generic::List<int>^ get() { return m_kTypeValues; } }
		property System::Collections::Generic::List<String^>^	EquipChoices
			{ System::Collections::Generic::List<String^>^ get() { return m_kEquipChoices; } }
		property System::Collections::Generic::List<int>^		EquipValues
			{ System::Collections::Generic::List<int>^ get() { return m_kEquipValues; } }

		// Called before every open, because an insert made in this session
		// changes which items are already sold.
		void MarkSold( System::Collections::Generic::HashSet<int>^ kSoldItemIds )
		{
			for( int i = 0; i < m_kAll->Length; ++i )
				m_kAll[i]->AlreadySold = kSoldItemIds->Contains( m_kAll[i]->ItemID );
		}

	private:
		void BuildChoices( const SExtractResult* pCatalog )
		{
			m_kTypeChoices	= gcnew System::Collections::Generic::List<String^>();
			m_kTypeValues	= gcnew System::Collections::Generic::List<int>();
			m_kEquipChoices	= gcnew System::Collections::Generic::List<String^>();
			m_kEquipValues	= gcnew System::Collections::Generic::List<int>();

			m_kTypeChoices->Add( "(any item type)" );
			m_kTypeValues->Add( -1 );
			m_kEquipChoices->Add( "(any equip slot)" );
			m_kEquipValues->Add( -1 );

			if( NULL == pCatalog )
				return;

			System::Collections::Generic::SortedDictionary<int, int>^ kTypeCount =
				gcnew System::Collections::Generic::SortedDictionary<int, int>();
			System::Collections::Generic::SortedDictionary<int, int>^ kEquipCount =
				gcnew System::Collections::Generic::SortedDictionary<int, int>();

			for( int i = 0; i < m_kAll->Length; ++i )
			{
				Bump( kTypeCount,  m_kAll[i]->ItemType );
				Bump( kEquipCount, m_kAll[i]->EquipPosition );
			}

			for each( System::Collections::Generic::KeyValuePair<int, int> kPair in kTypeCount )
			{
				String^ sName = NativeBridge::Utf8( LookupEnumRowName(
					pCatalog->vecEnumNames, EnumTableItemType(), kPair.Key ) );

				m_kTypeChoices->Add( Describe( sName, kPair.Key, kPair.Value ) );
				m_kTypeValues->Add( kPair.Key );
			}

			for each( System::Collections::Generic::KeyValuePair<int, int> kPair in kEquipCount )
			{
				String^ sName = NativeBridge::Utf8( LookupEnumRowName(
					pCatalog->vecEnumNames, EnumTableEquipPosition(), kPair.Key ) );

				m_kEquipChoices->Add( Describe( sName, kPair.Key, kPair.Value ) );
				m_kEquipValues->Add( kPair.Key );
			}
		}

		static void Bump( System::Collections::Generic::SortedDictionary<int, int>^ kMap, int iKey )
		{
			int iCount = 0;
			kMap->TryGetValue( iKey, iCount );
			kMap[iKey] = iCount + 1;
		}

		// A value Enum.lua has no name for is shown as the number alone.
		// That is information - the value is in the item data and not in
		// the enum table - and inventing a label for it would hide it.
		static String^ Describe( String^ sName, int iValue, int iCount )
		{
			if( String::IsNullOrEmpty( sName ) )
				return String::Format( "{0}  (no Enum.lua name)   {1}", iValue, iCount );

			return String::Format( "{0,-24} {1,4}   {2}", sName, iValue, iCount );
		}

		array<PickerItem^>^	m_kAll;
		int					m_iBuildMs;
		int					m_iManagedBytes;
		int					m_iHiddenPackage;
		bool				m_bPackageDataKnown;

		System::Collections::Generic::List<String^>^	m_kTypeChoices;
		System::Collections::Generic::List<int>^		m_kTypeValues;
		System::Collections::Generic::List<String^>^	m_kEquipChoices;
		System::Collections::Generic::List<int>^		m_kEquipValues;
	};

	//////////////////////////////////////////////////////////////////////
	// The list. Same shape as phase 4's ProductGridPanel and for the same
	// reason: a Control per row is a window handle per row, and 48,754
	// window handles is not a list, it is a hang.

	ref class ItemListPanel : public Panel
	{
	public:
		ItemListPanel( IconProvider^ kIcons )
		{
			m_kIcons		= kIcons;
			m_kRows			= gcnew System::Collections::Generic::List<PickerItem^>();
			m_iSelected		= -1;
			m_iRowHeight	= 44;

			SetStyle( ControlStyles::OptimizedDoubleBuffer
				| ControlStyles::AllPaintingInWmPaint
				| ControlStyles::UserPaint
				| ControlStyles::Selectable, true );

			AutoScroll	= true;
			TabStop		= true;
			BackColor	= System::Drawing::Color::FromArgb( 32, 32, 36 );

			m_kNameFont		= gcnew System::Drawing::Font( "Segoe UI", 9.0f, System::Drawing::FontStyle::Bold );
			m_kMetaFont		= gcnew System::Drawing::Font( "Consolas", 8.0f );
			m_kBadgeFont	= gcnew System::Drawing::Font( "Segoe UI", 7.0f, System::Drawing::FontStyle::Bold );

			m_kLinePen		= gcnew System::Drawing::Pen( System::Drawing::Color::FromArgb( 48, 48, 54 ), 1.0f );
			m_kSelectBrush	= gcnew System::Drawing::SolidBrush( System::Drawing::Color::FromArgb( 48, 66, 96 ) );
			m_kSoldBrush	= gcnew System::Drawing::SolidBrush( System::Drawing::Color::FromArgb( 34, 46, 34 ) );
			m_kHiddenBrush	= gcnew System::Drawing::SolidBrush( System::Drawing::Color::FromArgb( 56, 44, 24 ) );

			m_kRight = gcnew System::Drawing::StringFormat();
			m_kRight->Alignment = System::Drawing::StringAlignment::Far;
		}

		event EventHandler^	SelectionChanged;
		event EventHandler^	ItemActivated;

		void SetRows( System::Collections::Generic::List<PickerItem^>^ kRows, int iKeepItemID )
		{
			m_kRows		= kRows;
			m_iSelected	= -1;

			if( iKeepItemID > 0 )
			{
				for( int i = 0; i < m_kRows->Count; ++i )
				{
					if( m_kRows[i]->ItemID == iKeepItemID )
					{
						m_iSelected = i;
						break;
					}
				}
			}

			if( m_iSelected < 0 && m_kRows->Count > 0 )
				m_iSelected = 0;

			AutoScrollMinSize	= System::Drawing::Size( 0, m_kRows->Count * m_iRowHeight );
			AutoScrollPosition	= System::Drawing::Point( 0, 0 );

			ScrollSelectionIntoView();
			Invalidate();

			SelectionChanged( this, EventArgs::Empty );
		}

		PickerItem^ Selected()
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

			// The icons are 64x64 and drawn at 32x32, so this is a halving
			// and bilinear is both correct and cheap at twenty rows a
			// frame. NearestNeighbor on a halved DXT icon loses every
			// other pixel of the outline.
			g->InterpolationMode	= System::Drawing::Drawing2D::InterpolationMode::HighQualityBilinear;
			g->PixelOffsetMode		= System::Drawing::Drawing2D::PixelOffsetMode::Half;

			if( 0 == m_kRows->Count )
			{
				g->DrawString( "nothing matches - clear the search box or widen the filters",
					m_kNameFont, System::Drawing::Brushes::DimGray, 16.0f, 16.0f );
				return;
			}

			const int iScrollY = -AutoScrollPosition.Y;

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
				ItemActivated( this, EventArgs::Empty );
		}

		// Without this a Panel never sees the arrow keys at all - they are
		// eaten as navigation before OnKeyDown runs. Same trap phase 4's
		// grid hit.
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
				ItemActivated( this, EventArgs::Empty );
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

		void DrawRow( System::Drawing::Graphics^ g, PickerItem^ kRow, int iIndex, int iY )
		{
			System::Drawing::Rectangle kBounds( 0, iY, ClientSize.Width, m_iRowHeight );

			if( kRow->HiddenPackage )
				g->FillRectangle( m_kHiddenBrush, kBounds );
			else if( kRow->AlreadySold )
				g->FillRectangle( m_kSoldBrush, kBounds );

			if( iIndex == m_iSelected )
				g->FillRectangle( m_kSelectBrush, kBounds );

			g->DrawLine( m_kLinePen, 0, iY + m_iRowHeight - 1, ClientSize.Width, iY + m_iRowHeight - 1 );

			System::Drawing::Bitmap^ kIcon = m_kIcons->Get( kRow->ShopImage );
			if( nullptr != kIcon )
				g->DrawImage( kIcon, 5, iY + 5, 32, 32 );

			const int iTextLeft = 46;

			g->DrawString( kRow->Name, m_kNameFont, System::Drawing::Brushes::Gainsboro,
				(float) iTextLeft, (float)( iY + 3 ) );

			g->DrawString(
				String::Format( "{0}   {1}   {2}{3}",
					kRow->ItemID,
					String::IsNullOrEmpty( kRow->TypeName ) ? String::Format( "type {0}", kRow->ItemType ) : kRow->TypeName,
					String::IsNullOrEmpty( kRow->EquipName ) ? String::Format( "slot {0}", kRow->EquipPosition ) : kRow->EquipName,
					kRow->IsFashion ? "   fashion" : "" ),
				m_kMetaFont, System::Drawing::Brushes::Gray, (float) iTextLeft, (float)( iY + 23 ) );

			//////////////////////////////////////////////////////////////
			// The two things about an item that decide whether inserting
			// it will do anything, said in the row rather than in a
			// dialog after the fact.

			const int iRight = ClientSize.Width - 10;

			if( kRow->HiddenPackage )
			{
				System::Drawing::RectangleF kNote( (float)( iRight - 300 ), (float)( iY + 4 ), 300.0f, 18.0f );
				g->DrawString( "the shop hides this: package component", m_kBadgeFont,
					System::Drawing::Brushes::Goldenrod, kNote, m_kRight );
			}

			if( kRow->AlreadySold )
			{
				System::Drawing::RectangleF kNote( (float)( iRight - 300 ), (float)( iY + 22 ), 300.0f, 18.0f );
				g->DrawString( "already sold in the shop", m_kBadgeFont,
					System::Drawing::Brushes::DarkSeaGreen, kNote, m_kRight );
			}
		}

		IconProvider^												m_kIcons;
		System::Collections::Generic::List<PickerItem^>^			m_kRows;
		int															m_iSelected;
		int															m_iRowHeight;

		System::Drawing::Font^			m_kNameFont;
		System::Drawing::Font^			m_kMetaFont;
		System::Drawing::Font^			m_kBadgeFont;
		System::Drawing::Pen^			m_kLinePen;
		System::Drawing::SolidBrush^	m_kSelectBrush;
		System::Drawing::SolidBrush^	m_kSoldBrush;
		System::Drawing::SolidBrush^	m_kHiddenBrush;
		System::Drawing::StringFormat^	m_kRight;
	};

	//////////////////////////////////////////////////////////////////////

	ref class ItemPickerForm : public Form
	{
	public:
		ItemPickerForm( ItemCatalogView^ kView, IconProvider^ kIcons )
		{
			m_kView = kView;

			ItemID			= 0;
			ItemName		= String::Empty;
			ItemShopImage	= String::Empty;
			HiddenPackage	= false;

			Text			= "Pick an item to sell";
			ClientSize		= System::Drawing::Size( 940, 660 );
			MinimumSize		= System::Drawing::Size( 700, 460 );
			StartPosition	= FormStartPosition::CenterParent;
			BackColor		= System::Drawing::Color::FromArgb( 32, 32, 36 );
			ForeColor		= System::Drawing::Color::Gainsboro;
			ShowInTaskbar	= false;

			m_kFont		= gcnew System::Drawing::Font( "Segoe UI", 9.0f );
			m_kFixed	= gcnew System::Drawing::Font( "Consolas", 8.5f );

			BuildChrome( kIcons );
			ApplyFilter( 0 );

			m_kSearch->Select();
		}

		// What was picked, valid when the dialog returned OK.
		property int		ItemID;
		property String^	ItemName;
		property String^	ItemShopImage;
		property bool		HiddenPackage;

		// Runs one filter pass with the search box set to sQuery and
		// reports what it cost, without the window ever being shown.
		//
		// This exists for --picker-test, and it deliberately drives the
		// SAME ApplyFilter the keystrokes drive rather than a copy of the
		// loop: a measurement of a parallel implementation would be a
		// measurement of nothing. The exit test asks whether the picker
		// opens instantly and scrolls 48,754 rows without stutter, and
		// two thirds of that is a number.
		void MeasureFilter( String^ sQuery, int% iRowsOut, int% iMsOut )
		{
			// Set the text OUTSIDE the stopwatch: assigning it raises
			// TextChanged, which runs a pass of its own, and timing both
			// would report double. One pass is what a keystroke costs.
			m_kSearch->Text = ( nullptr == sQuery ) ? String::Empty : sQuery;

			System::Diagnostics::Stopwatch^ kWatch = System::Diagnostics::Stopwatch::StartNew();
			ApplyFilter( 0 );
			kWatch->Stop();

			iRowsOut	= m_kList->RowCount();
			iMsOut		= (int) kWatch->ElapsedMilliseconds;
		}

		// Also for --picker-test: whether the list can actually REACH its
		// last row.
		//
		// 48,754 rows at 44 pixels is a 2.1-million-pixel virtual canvas,
		// and that is the one number in this window that is large enough
		// to be worth doubting - a scroll range that silently saturates
		// would leave the last few thousand items unreachable while
		// everything above them looked perfect. Win32's SCROLLINFO takes
		// an int, so this should hold; "should" is why it is measured.
		void MeasureScrollExtent( int% iCanvasHeightOut, int% iReachedOut, int% iRowReachedOut )
		{
			// THE FORM HAS TO BE SHOWN FOR THIS TO MEAN ANYTHING. A Panel
			// with AutoScroll has no scrollbars until it is laid out on a
			// visible form, and until then assigning AutoScrollPosition is
			// simply dropped - the first version of this probe measured an
			// unshown form, got y = 0 back and reported that the end of the
			// list was unreachable. That was the measurement failing, not
			// the list.
			//
			// Shown off-screen, so a diagnostic switch does not flash a
			// window at whoever ran it.
			StartPosition	= FormStartPosition::Manual;
			Location		= System::Drawing::Point( -32000, -32000 );

			Show();
			Application::DoEvents();

			const int iCanvas = m_kList->AutoScrollMinSize.Height;

			m_kList->AutoScrollPosition = System::Drawing::Point( 0, iCanvas );
			Application::DoEvents();

			// AutoScrollPosition reads back NEGATED, which is the WinForms
			// oddity this reverses rather than reporting as a negative
			// number nobody expects.
			const int iReached = -m_kList->AutoScrollPosition.Y;

			iCanvasHeightOut	= iCanvas;
			iReachedOut			= iReached;
			iRowReachedOut		= ( iReached + m_kList->ClientSize.Height ) / 44;

			m_kList->AutoScrollPosition = System::Drawing::Point( 0, 0 );

			Hide();
		}

	private:
		//////////////////////////////////////////////////////////////////
		// Layout. Docked controls are laid out from the LAST entry in
		// Controls to the first, so the Fill control is added first.

		void BuildChrome( IconProvider^ kIcons )
		{
			m_kList = gcnew ItemListPanel( kIcons );
			m_kList->Dock				= DockStyle::Fill;
			m_kList->SelectionChanged	+= gcnew EventHandler( this, &ItemPickerForm::OnSelectionChanged );
			m_kList->ItemActivated		+= gcnew EventHandler( this, &ItemPickerForm::OnActivated );

			//////////////////////////////////////////////////////////////
			// The search row.

			Panel^ kTop = gcnew Panel();
			kTop->Dock		= DockStyle::Top;
			kTop->Height	= 76;
			kTop->BackColor	= System::Drawing::Color::FromArgb( 24, 24, 28 );

			Label^ kSearchLabel = gcnew Label();
			kSearchLabel->Bounds	= System::Drawing::Rectangle( 12, 12, 90, 20 );
			kSearchLabel->Font		= m_kFont;
			kSearchLabel->Text		= "name or id";
			kTop->Controls->Add( kSearchLabel );

			m_kSearch = gcnew TextBox();
			m_kSearch->Bounds		= System::Drawing::Rectangle( 105, 9, 300, 24 );
			m_kSearch->Font			= m_kFixed;
			m_kSearch->BackColor	= System::Drawing::Color::FromArgb( 32, 32, 36 );
			m_kSearch->ForeColor	= System::Drawing::Color::Gainsboro;
			m_kSearch->BorderStyle	= ::BorderStyle::FixedSingle;
			m_kSearch->TextChanged	+= gcnew EventHandler( this, &ItemPickerForm::OnFilterChanged );
			kTop->Controls->Add( m_kSearch );

			Label^ kHint = gcnew Label();
			kHint->Bounds	= System::Drawing::Rectangle( 415, 12, 500, 20 );
			kHint->Font		= gcnew System::Drawing::Font( "Segoe UI", 8.0f );
			kHint->ForeColor= System::Drawing::Color::FromArgb( 150, 150, 158 );
			kHint->Text		= "space-separated words must all appear in the name; digits also match an item id";
			kTop->Controls->Add( kHint );

			m_kType = MakeCombo( m_kView->TypeChoices, 105, 41, 240, kTop );
			m_kEquip= MakeCombo( m_kView->EquipChoices, 355, 41, 240, kTop );

			m_kHideSold = gcnew CheckBox();
			m_kHideSold->Bounds		= System::Drawing::Rectangle( 605, 41, 150, 24 );
			m_kHideSold->Font		= m_kFont;
			m_kHideSold->ForeColor	= System::Drawing::Color::Gainsboro;
			m_kHideSold->Text		= "hide already sold";
			m_kHideSold->CheckedChanged += gcnew EventHandler( this, &ItemPickerForm::OnFilterChanged );
			kTop->Controls->Add( m_kHideSold );

			m_kHideHidden = gcnew CheckBox();
			m_kHideHidden->Bounds		= System::Drawing::Rectangle( 760, 41, 170, 24 );
			m_kHideHidden->Font			= m_kFont;
			m_kHideHidden->ForeColor	= System::Drawing::Color::Gainsboro;
			m_kHideHidden->Text			= "hide package-hidden";
			m_kHideHidden->Enabled		= m_kView->PackageDataKnown;
			m_kHideHidden->CheckedChanged += gcnew EventHandler( this, &ItemPickerForm::OnFilterChanged );
			kTop->Controls->Add( m_kHideHidden );

			Label^ kFilterLabel = gcnew Label();
			kFilterLabel->Bounds	= System::Drawing::Rectangle( 12, 44, 90, 20 );
			kFilterLabel->Font		= m_kFont;
			kFilterLabel->Text		= "filters";
			kTop->Controls->Add( kFilterLabel );

			//////////////////////////////////////////////////////////////
			// The footer: what is selected, and the two buttons.

			Panel^ kBottom = gcnew Panel();
			kBottom->Dock		= DockStyle::Bottom;
			kBottom->Height		= 96;
			kBottom->BackColor	= System::Drawing::Color::FromArgb( 24, 24, 28 );

			m_kChosen = gcnew Label();
			m_kChosen->Bounds	= System::Drawing::Rectangle( 12, 8, 700, 54 );
			m_kChosen->Anchor	= (AnchorStyles)( AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right );
			m_kChosen->Font		= m_kFixed;
			m_kChosen->ForeColor= System::Drawing::Color::Gainsboro;
			kBottom->Controls->Add( m_kChosen );

			m_kCount = gcnew Label();
			m_kCount->Bounds	= System::Drawing::Rectangle( 12, 66, 700, 20 );
			m_kCount->Anchor	= (AnchorStyles)( AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right );
			m_kCount->Font		= gcnew System::Drawing::Font( "Segoe UI", 8.0f );
			m_kCount->ForeColor	= System::Drawing::Color::FromArgb( 160, 160, 168 );
			kBottom->Controls->Add( m_kCount );

			m_kOk = gcnew Button();
			m_kOk->Bounds		= System::Drawing::Rectangle( 0, 30, 100, 28 );
			m_kOk->Anchor		= (AnchorStyles)( AnchorStyles::Top | AnchorStyles::Right );
			m_kOk->Text			= "Use this item";
			m_kOk->Font			= m_kFont;
			m_kOk->FlatStyle	= ::FlatStyle::Flat;
			m_kOk->ForeColor	= System::Drawing::Color::Gainsboro;
			m_kOk->Click		+= gcnew EventHandler( this, &ItemPickerForm::OnActivated );
			kBottom->Controls->Add( m_kOk );

			Button^ kCancel = gcnew Button();
			kCancel->Bounds			= System::Drawing::Rectangle( 0, 30, 90, 28 );
			kCancel->Anchor			= (AnchorStyles)( AnchorStyles::Top | AnchorStyles::Right );
			kCancel->Text			= "Cancel";
			kCancel->Font			= m_kFont;
			kCancel->FlatStyle		= ::FlatStyle::Flat;
			kCancel->ForeColor		= System::Drawing::Color::Gainsboro;
			kCancel->DialogResult	= ::DialogResult::Cancel;
			kBottom->Controls->Add( kCancel );

			m_kCancel	= kCancel;
			m_kBottom	= kBottom;

			kBottom->Resize += gcnew EventHandler( this, &ItemPickerForm::OnFooterResize );

			Controls->Add( m_kList );
			Controls->Add( kBottom );
			Controls->Add( kTop );

			// AcceptButton is deliberately NOT set: Enter belongs to the
			// list, where it activates the highlighted row, and a form-wide
			// accept button would steal it while the search box has focus
			// and pick whatever happened to be selected.
			CancelButton = kCancel;

			LayoutFooter();
		}

		ComboBox^ MakeCombo( System::Collections::Generic::List<String^>^ kChoices,
							int iX, int iY, int iWidth, Control^ kParent )
		{
			ComboBox^ kCombo = gcnew ComboBox();
			kCombo->Bounds			= System::Drawing::Rectangle( iX, iY, iWidth, 24 );
			kCombo->Font			= m_kFixed;
			kCombo->FlatStyle		= ::FlatStyle::Flat;
			kCombo->DropDownStyle	= ComboBoxStyle::DropDownList;
			kCombo->DropDownHeight	= 400;

			for each( String^ sChoice in kChoices )
				kCombo->Items->Add( sChoice );

			kCombo->SelectedIndex		= 0;
			kCombo->SelectedIndexChanged += gcnew EventHandler( this, &ItemPickerForm::OnFilterChanged );

			kParent->Controls->Add( kCombo );
			return kCombo;
		}

		void OnFooterResize( Object^ /*kSender*/, EventArgs^ /*kArgs*/ )
		{
			LayoutFooter();
		}

		void LayoutFooter()
		{
			if( nullptr == m_kBottom )
				return;

			const int iRight = m_kBottom->ClientSize.Width;

			m_kOk->Left		= iRight - 210;
			m_kCancel->Left	= iRight - 100;

			m_kChosen->Width	= Math::Max( 200, iRight - 230 );
			m_kCount->Width		= Math::Max( 200, iRight - 230 );
		}

		//////////////////////////////////////////////////////////////////
		// Filtering.
		//
		// A straight pass over all 48,754 rows on every keystroke, timed
		// and reported. No debounce timer and no index: the pass is a
		// prepared-string IndexOf per row and the measured cost is printed
		// in the footer, so if it ever stops being cheap the window says
		// so rather than just feeling slow.

		void OnFilterChanged( Object^ /*kSender*/, EventArgs^ /*kArgs*/ )
		{
			PickerItem^ kSelected = m_kList->Selected();
			ApplyFilter( ( nullptr != kSelected ) ? kSelected->ItemID : 0 );
		}

		void ApplyFilter( int iKeepItemID )
		{
			System::Diagnostics::Stopwatch^ kWatch = System::Diagnostics::Stopwatch::StartNew();

			array<String^>^ akTerms	= SearchTerms();
			const int iTypeWanted	= ValueAt( m_kView->TypeValues,  m_kType->SelectedIndex );
			const int iEquipWanted	= ValueAt( m_kView->EquipValues, m_kEquip->SelectedIndex );
			const bool bHideSold	= m_kHideSold->Checked;
			const bool bHideHidden	= m_kHideHidden->Checked;

			// Non-negative when the search box holds nothing but digits,
			// in which case an item id prefix counts as a match too.
			const int iNumeric = NumericQuery();

			array<PickerItem^>^ akAll = m_kView->All;

			System::Collections::Generic::List<PickerItem^>^ kKept =
				gcnew System::Collections::Generic::List<PickerItem^>();

			for( int i = 0; i < akAll->Length; ++i )
			{
				PickerItem^ kItem = akAll[i];

				if( iTypeWanted >= 0 && kItem->ItemType != iTypeWanted )
					continue;

				if( iEquipWanted >= 0 && kItem->EquipPosition != iEquipWanted )
					continue;

				if( bHideSold && kItem->AlreadySold )
					continue;

				if( bHideHidden && kItem->HiddenPackage )
					continue;

				if( false == Matches( kItem, akTerms, iNumeric ) )
					continue;

				kKept->Add( kItem );
			}

			kWatch->Stop();

			m_kList->SetRows( kKept, iKeepItemID );

			m_kCount->Text = String::Format(
				"showing {0:N0} of {1:N0} item(s)   -   filtered in {2} ms"
				"   -   catalog marshalled once in {3} ms, {4:N0} KB{5}",
				kKept->Count, akAll->Length, kWatch->ElapsedMilliseconds,
				m_kView->BuildMs, m_kView->ManagedBytes / 1024,
				m_kView->PackageDataKnown
					? String::Format( "   -   {0:N0} item(s) the shop hides as package components",
						m_kView->HiddenPackageCount )
					: "   -   PackageItemData.lua did not run: the hidden-package rule is unknown" );
		}

		static int ValueAt( System::Collections::Generic::List<int>^ kValues, int iIndex )
		{
			if( iIndex < 0 || iIndex >= kValues->Count )
				return -1;

			return kValues[iIndex];
		}

		// Lower-cased once per pass, not once per row, and compared with
		// Ordinal against the row's prepared NameLower. Culture-aware
		// comparison over 48,754 rows is roughly two orders of magnitude
		// slower and would be the whole cost of the pass.
		array<String^>^ SearchTerms()
		{
			String^ sQuery = m_kSearch->Text->Trim();

			if( String::IsNullOrEmpty( sQuery ) )
				return gcnew array<String^>( 0 );

			return sQuery->ToLowerInvariant()->Split(
				gcnew array<wchar_t>{ L' ', L'\t' }, StringSplitOptions::RemoveEmptyEntries );
		}

		int NumericQuery()
		{
			String^ sQuery = m_kSearch->Text->Trim();

			int iValue = 0;
			if( sQuery->Length > 0 && Int32::TryParse( sQuery, iValue ) && iValue > 0 )
				return iValue;

			return -1;
		}

		// Every term must appear somewhere in the name, OR the query is a
		// number and the item id starts with it. The id branch is
		// arithmetic rather than a ToString per row: "1316" should find all
		// 1,316,xxx-style ids, and 48,754 string allocations per keystroke
		// to answer that is exactly the sort of thing that makes a list
		// feel slow for no reason.
		static bool Matches( PickerItem^ kItem, array<String^>^ akTerms, int iNumeric )
		{
			if( 0 == akTerms->Length )
				return true;

			bool bNameMatches = true;
			for( int t = 0; t < akTerms->Length; ++t )
			{
				if( kItem->NameLower->IndexOf( akTerms[t], StringComparison::Ordinal ) < 0 )
				{
					bNameMatches = false;
					break;
				}
			}

			if( bNameMatches )
				return true;

			if( iNumeric > 0 )
			{
				for( int iId = kItem->ItemID; iId > 0; iId /= 10 )
				{
					if( iId == iNumeric )
						return true;
				}
			}

			return false;
		}

		//////////////////////////////////////////////////////////////////

		void OnSelectionChanged( Object^ /*kSender*/, EventArgs^ /*kArgs*/ )
		{
			PickerItem^ kItem = m_kList->Selected();

			m_kOk->Enabled = ( nullptr != kItem );

			if( nullptr == kItem )
			{
				m_kChosen->Text = "nothing selected";
				return;
			}

			m_kChosen->Text = String::Format(
				"item {0}   {1}\r\nm_ShopImage: {2}\r\n{3}{4}",
				kItem->ItemID, kItem->Name,
				String::IsNullOrEmpty( kItem->ShopImage ) ? "(none - the shop will draw the Noimage icon)" : kItem->ShopImage,
				kItem->AlreadySold ? "already sold in the shop; a second product for the same item is legal and deliberate.  " : "",
				kItem->HiddenPackage
					? "THE SHOP WILL NOT SHOW THIS ITEM: PackageItemData.lua declares it a package component with bShowItem false."
					: "" );
		}

		void OnActivated( Object^ /*kSender*/, EventArgs^ /*kArgs*/ )
		{
			PickerItem^ kItem = m_kList->Selected();
			if( nullptr == kItem )
				return;

			ItemID			= kItem->ItemID;
			ItemName		= kItem->Name;
			ItemShopImage	= kItem->ShopImage;
			HiddenPackage	= kItem->HiddenPackage;

			DialogResult = ::DialogResult::OK;
			Close();
		}

		ItemCatalogView^	m_kView;

		ItemListPanel^	m_kList;
		TextBox^		m_kSearch;
		ComboBox^		m_kType;
		ComboBox^		m_kEquip;
		CheckBox^		m_kHideSold;
		CheckBox^		m_kHideHidden;
		Label^			m_kChosen;
		Label^			m_kCount;
		Button^			m_kOk;
		Button^			m_kCancel;
		Panel^			m_kBottom;

		System::Drawing::Font^	m_kFont;
		System::Drawing::Font^	m_kFixed;
	};
}
