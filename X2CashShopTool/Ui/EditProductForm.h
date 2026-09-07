//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phases 4 and 5 - the field dialog for one
// row of cash_product, in either of two modes.
// See CASH_SHOP_TOOL_PLAN.md, "Phase 4 - The main window" and "Phase 5 -
// Insert, with the virtualized picker".
//
// Category, quantity, price and the event flag. The ITEM is shown and not
// edited: an existing product's item cannot be swapped here, and a new
// product's item was chosen in the picker before this dialog opened. A
// free-text item id box would have been a way to create exactly the row
// the client silently drops, which is the failure the tool exists to
// expose.
//
// Phase 5 added bIsInsert. It changes the captions and, load-bearingly,
// which Validate overload runs: an insert has no product_no yet, so
// Validate must be told not to demand one. This is one dialog with two
// modes rather than two dialogs because the four fields, their limits and
// the reason for those limits are identical, and a second copy of them
// would be a second place for the 1..127 rule to drift.
//
// EVERY NUMERIC FIELD IS A PLAIN TEXTBOX, and that is the point of this
// file rather than an accident of laziness. NumericUpDown silently clamps
// a value to its Minimum/Maximum, which would turn "quantity 128" into
// "quantity 127" without a word - and the whole reason CASH_FIELD_MAX
// exists is that 128 reaches the client as -128 and buys nothing. The
// phase's exit test says the limits must be REFUSED, not clamped, so the
// text is parsed, handed to CCashDb::Validate, and the refusal is shown
// with the reason Validate gives. Do not "improve" these into spinners.
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>

#include "../Core/CashDb.h"

using namespace System;
using namespace System::Windows::Forms;

namespace X2CashShopTool
{
	// Assembly-private on purpose: its constructor takes a native
	// CCashDb*, and a public member exposing a native type is C4692.
	ref class EditProductForm : public Form
	{
	public:
		EditProductForm( int iProductNo, int iItemID, String^ sItemName, String^ sShopImage,
						 System::Drawing::Bitmap^ kIcon,
						 int iCategory, int iQuantity, int iPrice, bool bIsEvent,
						 System::Collections::Generic::List<String^>^ kCategoryChoices,
						 const CCashDb* pDb, bool bIsInsert, String^ sExtraNote )
		{
			m_pDb			= pDb;
			m_iProductNo	= iProductNo;
			m_iItemID		= iItemID;
			m_bIsInsert		= bIsInsert;

			Category	= iCategory;
			Quantity	= iQuantity;
			Price		= iPrice;
			IsEvent		= bIsEvent;

			Text = bIsInsert
				? String::Format( "Add product - item {0}", iItemID )
				: String::Format( "Edit product {0}", iProductNo );
			ClientSize		= System::Drawing::Size( 560, 300 );
			FormBorderStyle	= ::FormBorderStyle::FixedDialog;
			StartPosition	= FormStartPosition::CenterParent;
			MinimizeBox		= false;
			MaximizeBox		= false;
			BackColor		= System::Drawing::Color::FromArgb( 40, 40, 44 );
			ForeColor		= System::Drawing::Color::Gainsboro;

			System::Drawing::Font^ kFont		= gcnew System::Drawing::Font( "Segoe UI", 9.0f );
			System::Drawing::Font^ kFixedFont	= gcnew System::Drawing::Font( "Consolas", 8.5f );

			//////////////////////////////////////////////////////////////
			// What is being edited, shown and not editable.

			PictureBox^ kPicture = gcnew PictureBox();
			kPicture->Bounds		= System::Drawing::Rectangle( 14, 14, 64, 64 );
			kPicture->SizeMode		= PictureBoxSizeMode::Normal;
			kPicture->BackColor		= System::Drawing::Color::FromArgb( 24, 24, 28 );
			kPicture->Image			= kIcon;
			Controls->Add( kPicture );

			Label^ kItem = gcnew Label();
			kItem->Bounds		= System::Drawing::Rectangle( 90, 14, 456, 66 );
			kItem->Font			= kFixedFont;
			kItem->ForeColor	= System::Drawing::Color::Gainsboro;
			kItem->Text			= String::Format(
				"item {0}   {1}\r\nm_ShopImage: {2}\r\n{3}",
				iItemID, sItemName, ( String::IsNullOrEmpty( sShopImage ) ? "(none)" : sShopImage ),
				String::IsNullOrEmpty( sExtraNote )
					? ( bIsInsert
						? "product_no is allocated as max+1 when this is saved"
						: String::Format( "product_no {0}", iProductNo ) )
					: sExtraNote );

			// Amber when the caller had something to warn about - today
			// that is only the shop's hidden-package rule, which makes an
			// otherwise perfect row invisible in the game.
			if( false == String::IsNullOrEmpty( sExtraNote ) )
				kItem->ForeColor = System::Drawing::Color::Goldenrod;

			Controls->Add( kItem );

			//////////////////////////////////////////////////////////////
			// The four editable fields.

			int iY = 94;

			Controls->Add( MakeLabel( "category", 14, iY + 3, kFont ) );

			// Editable, NOT DropDownList. A list would make the legal
			// values easy and 128 impossible to type - and 128 is exactly
			// what the exit test has to be able to type in order to see it
			// refused. The list is the convenience; the free text is the
			// contract.
			m_kCategory = gcnew ComboBox();
			m_kCategory->Bounds			= System::Drawing::Rectangle( 100, iY, 320, 24 );
			m_kCategory->DropDownStyle	= ComboBoxStyle::DropDown;
			m_kCategory->Font			= kFixedFont;
			m_kCategory->FlatStyle		= ::FlatStyle::Flat;

			for each( String^ sChoice in kCategoryChoices )
				m_kCategory->Items->Add( sChoice );

			m_kCategory->Text = ChoiceFor( kCategoryChoices, iCategory );
			Controls->Add( m_kCategory );

			Controls->Add( MakeHint( "1..127", 430, iY + 3, kFixedFont ) );

			iY += 34;
			Controls->Add( MakeLabel( "quantity", 14, iY + 3, kFont ) );
			m_kQuantity = MakeBox( iQuantity.ToString(), 100, iY, 120, kFixedFont );
			Controls->Add( m_kQuantity );
			Controls->Add( MakeHint( "1..127", 430, iY + 3, kFixedFont ) );

			iY += 34;
			Controls->Add( MakeLabel( "price", 14, iY + 3, kFont ) );
			m_kPrice = MakeBox( iPrice.ToString(), 100, iY, 120, kFixedFont );
			Controls->Add( m_kPrice );
			Controls->Add( MakeHint( "0 or more; not narrowed on the wire", 430, iY + 3, kFixedFont ) );

			iY += 34;
			m_kEvent = gcnew CheckBox();
			m_kEvent->Bounds	= System::Drawing::Rectangle( 100, iY, 200, 22 );
			m_kEvent->Font		= kFont;
			m_kEvent->ForeColor	= System::Drawing::Color::Gainsboro;
			m_kEvent->Text		= "is_event";
			m_kEvent->Checked	= bIsEvent;
			Controls->Add( m_kEvent );

			//////////////////////////////////////////////////////////////

			Label^ kWhy = gcnew Label();
			kWhy->Bounds	= System::Drawing::Rectangle( 14, iY + 32, 532, 32 );
			kWhy->Font		= gcnew System::Drawing::Font( "Segoe UI", 8.0f );
			kWhy->ForeColor	= System::Drawing::Color::FromArgb( 170, 170, 178 );
			kWhy->Text		= "category and quantity are narrowed to a signed char on the wire "
				"(X2OfflineCashShop.cpp:181 and :185), so 128 arrives as -128 and 0 is rewritten to 1. "
				"Out-of-range values are refused here, never clamped.";
			Controls->Add( kWhy );

			Button^ kOk = gcnew Button();
			kOk->Bounds		= System::Drawing::Rectangle( 350, 258, 90, 28 );
			kOk->Text		= bIsInsert ? "Add" : "Save";
			kOk->Font		= kFont;
			kOk->FlatStyle	= ::FlatStyle::Flat;
			kOk->ForeColor	= System::Drawing::Color::Gainsboro;
			kOk->Click		+= gcnew EventHandler( this, &EditProductForm::OnOk );
			Controls->Add( kOk );

			Button^ kCancel = gcnew Button();
			kCancel->Bounds			= System::Drawing::Rectangle( 450, 258, 90, 28 );
			kCancel->Text			= "Cancel";
			kCancel->Font			= kFont;
			kCancel->FlatStyle		= ::FlatStyle::Flat;
			kCancel->ForeColor		= System::Drawing::Color::Gainsboro;
			kCancel->DialogResult	= ::DialogResult::Cancel;
			Controls->Add( kCancel );

			AcceptButton = kOk;
			CancelButton = kCancel;
		}

		// What the dialog was left holding, valid when it returned OK.
		property int	Category;
		property int	Quantity;
		property int	Price;
		property bool	IsEvent;

	private:
		Label^ MakeLabel( String^ sText, int iX, int iY, System::Drawing::Font^ kFont )
		{
			Label^ kLabel = gcnew Label();
			kLabel->Bounds		= System::Drawing::Rectangle( iX, iY, 80, 20 );
			kLabel->Font		= kFont;
			kLabel->ForeColor	= System::Drawing::Color::Gainsboro;
			kLabel->Text		= sText;
			return kLabel;
		}

		Label^ MakeHint( String^ sText, int iX, int iY, System::Drawing::Font^ kFont )
		{
			Label^ kLabel = gcnew Label();
			kLabel->Bounds		= System::Drawing::Rectangle( iX, iY, 120, 34 );
			kLabel->Font		= kFont;
			kLabel->ForeColor	= System::Drawing::Color::FromArgb( 150, 150, 158 );
			kLabel->Text		= sText;
			return kLabel;
		}

		TextBox^ MakeBox( String^ sText, int iX, int iY, int iWidth, System::Drawing::Font^ kFont )
		{
			TextBox^ kBox = gcnew TextBox();
			kBox->Bounds		= System::Drawing::Rectangle( iX, iY, iWidth, 24 );
			kBox->Font			= kFont;
			kBox->BackColor		= System::Drawing::Color::FromArgb( 24, 24, 28 );
			kBox->ForeColor		= System::Drawing::Color::Gainsboro;
			kBox->BorderStyle	= ::BorderStyle::FixedSingle;
			kBox->Text			= sText;
			return kBox;
		}

		// The combo shows "11  CSC_FASHION / CSSC_FASHION_WEAPON", so the
		// number is the leading token whether it was picked from the list
		// or typed by hand. Anything that is not a number at the front is
		// an error the caller reports, never a silent 0.
		static bool ParseLeading( String^ sText, int% iOut )
		{
			iOut = 0;

			if( nullptr == sText )
				return false;

			String^ sTrimmed = sText->Trim();
			int iEnd = 0;
			while( iEnd < sTrimmed->Length && Char::IsDigit( sTrimmed[iEnd] ) )
				++iEnd;

			if( 0 == iEnd )
				return false;

			return Int32::TryParse( sTrimmed->Substring( 0, iEnd ), iOut );
		}

		static String^ ChoiceFor( System::Collections::Generic::List<String^>^ kChoices, int iValue )
		{
			for each( String^ sChoice in kChoices )
			{
				int iParsed = 0;
				if( ParseLeading( sChoice, iParsed ) && iParsed == iValue )
					return sChoice;
			}

			// A category that matches no tab is still a real value in the
			// table, so it is shown as itself rather than snapped to
			// something legal.
			return iValue.ToString();
		}

		void Complain( String^ sMessage, Control^ kFocus )
		{
			MessageBox::Show( this, sMessage, "Refused",
				MessageBoxButtons::OK, MessageBoxIcon::Warning );

			if( nullptr != kFocus )
				kFocus->Focus();
		}

		void OnOk( Object^ /*kSender*/, EventArgs^ /*kArgs*/ )
		{
			int iCategory = 0;
			int iQuantity = 0;
			int iPrice    = 0;

			if( false == ParseLeading( m_kCategory->Text, iCategory ) )
			{
				Complain( "category must be a number.", m_kCategory );
				return;
			}

			if( false == Int32::TryParse( m_kQuantity->Text->Trim(), iQuantity ) )
			{
				Complain( "quantity must be a number.", m_kQuantity );
				return;
			}

			if( false == Int32::TryParse( m_kPrice->Text->Trim(), iPrice ) )
			{
				Complain( "price must be a number.", m_kPrice );
				return;
			}

			// The same Validate the write path calls, so a value refused
			// here is refused for the reason the database layer gives and
			// not for a second opinion written in the Ui.
			SCashProductRow kRow;
			kRow.iProductNo		= m_iProductNo;
			kRow.iItemID		= m_iItemID;
			kRow.iCategoryNo	= iCategory;
			kRow.iQuantity		= iQuantity;
			kRow.iPrice			= iPrice;
			kRow.iIsEvent		= m_kEvent->Checked ? 1 : 0;

			// bIsInsert, not a literal false: an insert has no product_no
			// yet and Validate refuses a non-positive one on the edit path.
			std::string strError;
			if( NULL != m_pDb && false == m_pDb->Validate( kRow, m_bIsInsert, strError ) )
			{
				array<Byte>^ abBytes = gcnew array<Byte>( (int) strError.size() );
				if( abBytes->Length > 0 )
				{
					System::Runtime::InteropServices::Marshal::Copy(
						IntPtr( (void*) strError.data() ), abBytes, 0, abBytes->Length );
				}

				Complain( System::Text::Encoding::UTF8->GetString( abBytes ), nullptr );
				return;
			}

			Category	= iCategory;
			Quantity	= iQuantity;
			Price		= iPrice;
			IsEvent		= ( 0 != kRow.iIsEvent );

			DialogResult = ::DialogResult::OK;
			Close();
		}

		const CCashDb*	m_pDb;
		int				m_iProductNo;
		int				m_iItemID;
		bool			m_bIsInsert;

		ComboBox^	m_kCategory;
		TextBox^	m_kQuantity;
		TextBox^	m_kPrice;
		CheckBox^	m_kEvent;
	};
}
