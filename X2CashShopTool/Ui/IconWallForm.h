//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 2 - the scratch window the phase's
// exit test asks for: "a wall of ~200 decoded icons, correct colours and
// alpha, with a count of how many items resolved to a real file versus the
// fallback". See CASH_SHOP_TOOL_PLAN.md, "Phase 2 - Icons".
//
// Scratch means scratch. This is not the product grid - that is phase 4,
// built from cash_product and the parsed tab table. What this window has
// to settle is only whether the pixels coming out of DdsDecode are right,
// which is a thing no amount of reading can decide.
//
// Managed all the way through: every tile is already a Bitmap by the time
// this file sees it (Main.cpp copies the borrowed BGRA out of the store
// immediately, which is what CIconStore::Get's contract requires).
//////////////////////////////////////////////////////////////////////////
#pragma once

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace X2CashShopTool
{
	// One tile: the decoded image plus what it took to get it, so the
	// window can be read as a report and not just looked at.
	public ref class IconTile
	{
	public:
		Bitmap^		Image;
		String^		Caption;	// item id, or the group heading's label
		String^		Detail;		// tooltip: name, m_ShopImage, format, outcome
		bool		IsFallback;
		bool		IsFailure;
		String^		GroupLabel;	// non-null on the first tile of a group

		IconTile()
		: Image( nullptr ), Caption( String::Empty ), Detail( String::Empty )
		, IsFallback( false ), IsFailure( false ), GroupLabel( nullptr )
		{}
	};

	//////////////////////////////////////////////////////////////////////
	// The wall itself. One owner-drawn panel rather than 240 PictureBoxes:
	// a PictureBox per tile is 240 windows, 240 handles and a visibly slow
	// resize, and phase 5's picker will need the same paint-on-demand
	// discipline over 48,754 rows.
	public ref class IconWallPanel : public Panel
	{
	public:
		IconWallPanel( System::Collections::Generic::List<IconTile^>^ kTiles )
		: m_kTiles( kTiles )
		, m_iTileSize( 64 )
		, m_iPadding( 10 )
		, m_iCaptionHeight( 14 )
		, m_iHeadingHeight( 26 )
		{
			SetStyle( ControlStyles::OptimizedDoubleBuffer
				| ControlStyles::AllPaintingInWmPaint
				| ControlStyles::UserPaint, true );

			AutoScroll	= true;
			BackColor	= Color::FromArgb( 40, 40, 44 );

			m_kToolTip = gcnew ToolTip();
			m_kToolTip->InitialDelay	= 250;
			m_kToolTip->ReshowDelay		= 100;
			m_kToolTip->AutoPopDelay	= 20000;

			// The checkerboard is the whole point of the alpha check: a
			// 64x64 icon drawn on flat grey cannot be told apart from one
			// whose alpha channel was decoded as fully opaque.
			m_kChecker = MakeChecker();

			m_iLastLayoutWidth	= -1;
			m_iHoverIndex		= -1;

			// One font each, made once. Creating a Font inside OnPaint is
			// how an owner-drawn list starts leaking GDI handles.
			m_kHeadingFont	= gcnew System::Drawing::Font( "Segoe UI", 9.0f, FontStyle::Bold );
			m_kCaptionFont	= gcnew System::Drawing::Font( "Consolas", 7.0f );
			m_kFailureFont	= gcnew System::Drawing::Font( "Segoe UI", 20.0f, FontStyle::Bold );
			m_kNormalPen	= gcnew Pen( Color::FromArgb( 90, 90, 96 ), 1.0f );
			m_kFallbackPen	= gcnew Pen( Color::Goldenrod, 1.0f );
		}

	protected:
		virtual void OnPaint( PaintEventArgs^ e ) override
		{
			Graphics^ g = e->Graphics;
			g->InterpolationMode	= Drawing2D::InterpolationMode::NearestNeighbor;
			g->PixelOffsetMode		= Drawing2D::PixelOffsetMode::Half;

			EnsureLayout();

			for( int i = 0; i < m_kTiles->Count; ++i )
			{
				System::Drawing::Rectangle kBounds = m_akBounds[i];
				kBounds.Offset( AutoScrollPosition.X, AutoScrollPosition.Y );

				if( false == e->ClipRectangle.IntersectsWith( kBounds ) )
					continue;

				DrawTile( g, m_kTiles[i], kBounds );
			}
		}

		virtual void OnMouseMove( MouseEventArgs^ e ) override
		{
			Panel::OnMouseMove( e );

			const int iHit = HitTest( e->Location );
			if( iHit == m_iHoverIndex )
				return;

			m_iHoverIndex = iHit;
			m_kToolTip->SetToolTip( this, ( iHit >= 0 ) ? m_kTiles[iHit]->Detail : String::Empty );
		}

		virtual void OnClientSizeChanged( EventArgs^ e ) override
		{
			Panel::OnClientSizeChanged( e );
			m_iLastLayoutWidth = -1;
			Invalidate();
		}

	private:
		int HitTest( System::Drawing::Point kPoint )
		{
			if( nullptr == m_akBounds )
				return -1;

			for( int i = 0; i < m_kTiles->Count; ++i )
			{
				System::Drawing::Rectangle kBounds = m_akBounds[i];
				kBounds.Offset( AutoScrollPosition.X, AutoScrollPosition.Y );

				if( kBounds.Contains( kPoint ) )
					return i;
			}

			return -1;
		}

		Bitmap^ MakeChecker()
		{
			Bitmap^ kChecker = gcnew Bitmap( 16, 16 );
			for( int y = 0; y < 16; ++y )
			{
				for( int x = 0; x < 16; ++x )
				{
					const bool bLight = ( ( ( x / 8 ) + ( y / 8 ) ) % 2 ) == 0;
					kChecker->SetPixel( x, y, bLight
						? Color::FromArgb( 210, 210, 214 )
						: Color::FromArgb( 150, 150, 156 ) );
				}
			}

			return kChecker;
		}

		void EnsureLayout()
		{
			const int iWidth = ClientSize.Width;
			if( iWidth == m_iLastLayoutWidth )
				return;

			m_iLastLayoutWidth	= iWidth;
			m_akBounds			= gcnew array<System::Drawing::Rectangle>( m_kTiles->Count );

			const int iCell		= m_iTileSize + m_iPadding;
			const int iColumns	= Math::Max( 1, ( iWidth - m_iPadding ) / iCell );

			int iX		= 0;
			int iY		= m_iPadding;
			int iColumn	= 0;

			for( int i = 0; i < m_kTiles->Count; ++i )
			{
				// A group heading breaks the row, so the reader can see at
				// a glance which block of tiles is DXT5 and which is the
				// fallback.
				if( nullptr != m_kTiles[i]->GroupLabel )
				{
					if( iColumn != 0 )
					{
						iY += iCell + m_iCaptionHeight;
						iColumn = 0;
					}

					iY += m_iHeadingHeight;
				}

				iX = m_iPadding + iColumn * iCell;
				m_akBounds[i] = System::Drawing::Rectangle( iX, iY, m_iTileSize, m_iTileSize );

				++iColumn;
				if( iColumn >= iColumns )
				{
					iColumn = 0;
					iY += iCell + m_iCaptionHeight;
				}
			}

			AutoScrollMinSize = System::Drawing::Size( iWidth - 24, iY + iCell + m_iCaptionHeight + m_iPadding );
		}

		void DrawTile( Graphics^ g, IconTile^ kTile, System::Drawing::Rectangle kBounds )
		{
			if( nullptr != kTile->GroupLabel )
			{
				g->DrawString( kTile->GroupLabel, m_kHeadingFont,
					Brushes::Gainsboro, (float) m_iPadding, (float)( kBounds.Top - m_iHeadingHeight + 4 ) );
			}

			// Checkerboard first, then the icon over it: any pixel whose
			// alpha decoded wrongly shows up immediately as a hard square
			// where the checker should be showing through.
			System::Drawing::Drawing2D::GraphicsState^ kState = g->Save();
			g->SetClip( kBounds );
			for( int y = kBounds.Top; y < kBounds.Bottom; y += 16 )
			{
				for( int x = kBounds.Left; x < kBounds.Right; x += 16 )
					g->DrawImageUnscaled( m_kChecker, x, y );
			}
			g->Restore( kState );

			if( nullptr != kTile->Image )
			{
				g->DrawImage( kTile->Image, kBounds.Left, kBounds.Top,
					kTile->Image->Width, kTile->Image->Height );
			}
			else
			{
				g->FillRectangle( Brushes::DarkRed, kBounds );
				g->DrawString( "X", m_kFailureFont,
					Brushes::White, (float)( kBounds.Left + 16 ), (float)( kBounds.Top + 12 ) );
			}

			g->DrawRectangle( kTile->IsFallback ? m_kFallbackPen : m_kNormalPen,
				kBounds.Left, kBounds.Top, kBounds.Width - 1, kBounds.Height - 1 );

			g->DrawString( kTile->Caption, m_kCaptionFont,
				kTile->IsFallback ? Brushes::Goldenrod : Brushes::Silver,
				(float) kBounds.Left, (float) kBounds.Bottom + 1.0f );
		}

		System::Collections::Generic::List<IconTile^>^	m_kTiles;
		array<System::Drawing::Rectangle>^				m_akBounds;
		Bitmap^											m_kChecker;
		ToolTip^										m_kToolTip;

		System::Drawing::Font^	m_kHeadingFont;
		System::Drawing::Font^	m_kCaptionFont;
		System::Drawing::Font^	m_kFailureFont;
		Pen^					m_kNormalPen;
		Pen^					m_kFallbackPen;

		int	m_iTileSize;
		int	m_iPadding;
		int	m_iCaptionHeight;
		int	m_iHeadingHeight;
		int	m_iLastLayoutWidth;
		int	m_iHoverIndex;
	};

	//////////////////////////////////////////////////////////////////////

	public ref class IconWallForm : public Form
	{
	public:
		IconWallForm( System::Collections::Generic::List<IconTile^>^ kTiles, String^ sSummary )
		{
			Text			= "X2CashShopTool - phase 2 icon wall";
			ClientSize		= System::Drawing::Size( 900, 700 );
			StartPosition	= FormStartPosition::CenterScreen;
			BackColor		= Color::FromArgb( 32, 32, 36 );

			Label^ kSummary = gcnew Label();
			kSummary->Dock			= DockStyle::Top;
			kSummary->Height		= 82;
			kSummary->Padding		= System::Windows::Forms::Padding( 10, 8, 10, 8 );
			kSummary->Font			= gcnew System::Drawing::Font( "Consolas", 8.5f );
			kSummary->ForeColor		= Color::Gainsboro;
			kSummary->BackColor		= Color::FromArgb( 24, 24, 28 );
			kSummary->Text			= sSummary;

			IconWallPanel^ kWall = gcnew IconWallPanel( kTiles );
			kWall->Dock = DockStyle::Fill;

			Controls->Add( kWall );
			Controls->Add( kSummary );
		}
	};
}
