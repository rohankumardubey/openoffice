/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <tools/table.hxx>
#include <tools/debug.hxx>
#include <tools/rc.h>

#include <vcl/decoview.hxx>
#include <vcl/lstbox.h>
#include <vcl/button.hxx>
#include <vcl/event.hxx>
#include <vcl/combobox.hxx>

#include <svdata.hxx>
#include <subedit.hxx>
#include <ilstbox.hxx>
#include <controldata.hxx>

// =======================================================================

inline sal_uLong ImplCreateKey( sal_uInt16 nPos )
{
	// Key = Pos+1, wegen Pos 0
	return nPos+1;
}

// -----------------------------------------------------------------------

static void lcl_GetSelectedEntries( Table& rSelectedPos, const XubString& rText, xub_Unicode cTokenSep, const ImplEntryList* pEntryList )
{
	for( xub_StrLen n = rText.GetTokenCount( cTokenSep ); n; )
	{
		XubString aToken = rText.GetToken( --n, cTokenSep );
		aToken.EraseLeadingAndTrailingChars( ' ' );
		sal_uInt16 nPos = pEntryList->FindEntry( aToken );
		if ( nPos != LISTBOX_ENTRY_NOTFOUND )
			rSelectedPos.Insert( ImplCreateKey( nPos ), (void*)sal_IntPtr(1L) );
	}
}

// =======================================================================

ComboBox::ComboBox( WindowType nType ) :
	Edit( nType )
{
	ImplInitComboBoxData();
}

// -----------------------------------------------------------------------

ComboBox::ComboBox( Window* pParent, WinBits nStyle ) :
	Edit( WINDOW_COMBOBOX )
{
	ImplInitComboBoxData();
	ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

ComboBox::ComboBox( Window* pParent, const ResId& rResId ) :
	Edit( WINDOW_COMBOBOX )
{
	ImplInitComboBoxData();
	rResId.SetRT( RSC_COMBOBOX );
	WinBits nStyle = ImplInitRes( rResId );
	ImplInit( pParent, nStyle );
	ImplLoadRes( rResId );

	if ( !(nStyle & WB_HIDE ) )
		Show();
}

// -----------------------------------------------------------------------

ComboBox::~ComboBox()
{
	SetSubEdit( NULL );
	delete mpSubEdit;

	delete mpImplLB;
	mpImplLB = NULL;

	delete mpFloatWin;
	delete mpBtn;
}

// -----------------------------------------------------------------------

void ComboBox::ImplInitComboBoxData()
{
	mpSubEdit			= NULL;
	mpBtn				= NULL;
	mpImplLB			= NULL;
	mpFloatWin			= NULL;

	mnDDHeight			= 0;
	mbDDAutoSize		= sal_True;
	mbSyntheticModify	= sal_False;
	mbMatchCase 		= sal_False;
	mcMultiSep			= ';';
}

// -----------------------------------------------------------------------

void ComboBox::ImplCalcEditHeight()
{
	sal_Int32 nLeft, nTop, nRight, nBottom;
	GetBorder( nLeft, nTop, nRight, nBottom );
	mnDDHeight = (sal_uInt16)(mpSubEdit->GetTextHeight() + nTop + nBottom + 4);
	if ( !IsDropDownBox() )
		mnDDHeight += 4;

    Rectangle aCtrlRegion( Point( 0, 0 ), Size( 10, 10 ) );
    Rectangle aBoundRegion, aContentRegion;
    ImplControlValue aControlValue;
    ControlType aType = IsDropDownBox() ? CTRL_COMBOBOX : CTRL_EDITBOX;
    if( GetNativeControlRegion( aType, PART_ENTIRE_CONTROL,
                                aCtrlRegion,
                                CTRL_STATE_ENABLED,
                                aControlValue, rtl::OUString(),
                                aBoundRegion, aContentRegion ) )
    {
        const long nNCHeight = aBoundRegion.GetHeight();
        if( mnDDHeight < nNCHeight )
            mnDDHeight = sal::static_int_cast<sal_uInt16>( nNCHeight );
    }
}

// -----------------------------------------------------------------------

void ComboBox::ImplInit( Window* pParent, WinBits nStyle )
{
	ImplInitStyle( nStyle );

	sal_Bool bNoBorder = ( nStyle & WB_NOBORDER ) ? sal_True : sal_False;
	if ( !(nStyle & WB_DROPDOWN) )
	{
		nStyle &= ~WB_BORDER;
		nStyle |= WB_NOBORDER;
	}
	else
	{
		if ( !bNoBorder )
			nStyle |= WB_BORDER;
	}

	Edit::ImplInit( pParent, nStyle );
	SetBackground();

	// DropDown ?
	WinBits nEditStyle = nStyle & ( WB_LEFT | WB_RIGHT | WB_CENTER );
	WinBits nListStyle = nStyle;
	if( nStyle & WB_DROPDOWN )
	{
		mpFloatWin = new ImplListBoxFloatingWindow( this );
		mpFloatWin->SetAutoWidth( sal_True );
		mpFloatWin->SetPopupModeEndHdl( LINK( this, ComboBox, ImplPopupModeEndHdl ) );

		mpBtn = new ImplBtn( this, WB_NOLIGHTBORDER | WB_RECTSTYLE );
		ImplInitDropDownButton( mpBtn );
		mpBtn->SetMBDownHdl( LINK( this, ComboBox, ImplClickBtnHdl ) );
		mpBtn->Show();

		nEditStyle |= WB_NOBORDER;
		nListStyle &= ~WB_BORDER;
		nListStyle |= WB_NOBORDER;
	}
	else
	{
		if ( !bNoBorder )
		{
			nEditStyle |= WB_BORDER;
			nListStyle &= ~WB_NOBORDER;
			nListStyle |= WB_BORDER;
		}
	}

	mpSubEdit = new Edit( this, nEditStyle );
	mpSubEdit->EnableRTL( sal_False );
	SetSubEdit( mpSubEdit );
	mpSubEdit->SetPosPixel( Point() );
	EnableAutocomplete( sal_True );
	mpSubEdit->Show();

	Window* pLBParent = this;
	if ( mpFloatWin )
		pLBParent = mpFloatWin;
	mpImplLB = new ImplListBox( pLBParent, nListStyle|WB_SIMPLEMODE );
	mpImplLB->SetPosPixel( Point() );
	mpImplLB->SetSelectHdl( LINK( this, ComboBox, ImplSelectHdl ) );
	mpImplLB->SetCancelHdl( LINK( this, ComboBox, ImplCancelHdl ) );
	mpImplLB->SetDoubleClickHdl( LINK( this, ComboBox, ImplDoubleClickHdl ) );
	mpImplLB->SetUserDrawHdl( LINK( this, ComboBox, ImplUserDrawHdl ) );
	mpImplLB->SetSelectionChangedHdl( LINK( this, ComboBox, ImplSelectionChangedHdl ) );
	mpImplLB->Show();

	if ( mpFloatWin )
		mpFloatWin->SetImplListBox( mpImplLB );
	else
		mpImplLB->GetMainWindow()->AllowGrabFocus( sal_True );

	ImplCalcEditHeight();

	SetCompoundControl( sal_True );
}

// -----------------------------------------------------------------------

WinBits ComboBox::ImplInitStyle( WinBits nStyle )
{
	if ( !(nStyle & WB_NOTABSTOP) )
		nStyle |= WB_TABSTOP;
	if ( !(nStyle & WB_NOGROUP) )
		nStyle |= WB_GROUP;
	return nStyle;
}

// -----------------------------------------------------------------------

void ComboBox::ImplLoadRes( const ResId& rResId )
{
	Edit::ImplLoadRes( rResId );

	sal_uLong nNumber = ReadLongRes();

	if( nNumber )
	{
		for( sal_uInt16 i = 0; i < nNumber; i++ )
		{
			InsertEntry( ReadStringRes(), LISTBOX_APPEND );
		}
	}
}

// -----------------------------------------------------------------------

void ComboBox::EnableAutocomplete( sal_Bool bEnable, sal_Bool bMatchCase )
{
	mbMatchCase = bMatchCase;

	if ( bEnable )
		mpSubEdit->SetAutocompleteHdl( LINK( this, ComboBox, ImplAutocompleteHdl ) );
	else
		mpSubEdit->SetAutocompleteHdl( Link() );
}

// -----------------------------------------------------------------------

sal_Bool ComboBox::IsAutocompleteEnabled() const
{
	return mpSubEdit->GetAutocompleteHdl().IsSet();
}

// -----------------------------------------------------------------------

IMPL_LINK( ComboBox, ImplClickBtnHdl, void*, EMPTYARG )
{
    ImplCallEventListeners( VCLEVENT_DROPDOWN_PRE_OPEN );
	mpSubEdit->GrabFocus();
	if ( !mpImplLB->GetEntryList()->GetMRUCount() )
		ImplUpdateFloatSelection();
	else
		mpImplLB->SelectEntry( 0 , sal_True );
	mpBtn->SetPressed( sal_True );
    SetSelection( Selection( 0, SELECTION_MAX ) );
	mpFloatWin->StartFloat( sal_True );
    ImplCallEventListeners( VCLEVENT_DROPDOWN_OPEN );

    ImplClearLayoutData();
    if( mpImplLB )
        mpImplLB->GetMainWindow()->ImplClearLayoutData();

	return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ComboBox, ImplPopupModeEndHdl, void*, EMPTYARG )
{
    if( mpFloatWin->IsPopupModeCanceled() )
    {
        if ( !mpImplLB->GetEntryList()->IsEntryPosSelected( mpFloatWin->GetPopupModeStartSaveSelection() ) )
        {
            mpImplLB->SelectEntry( mpFloatWin->GetPopupModeStartSaveSelection(), sal_True );
            sal_Bool bTravelSelect = mpImplLB->IsTravelSelect();
            mpImplLB->SetTravelSelect( sal_True );
            Select();
            mpImplLB->SetTravelSelect( bTravelSelect );
        }
    }

    ImplClearLayoutData();
    if( mpImplLB )
        mpImplLB->GetMainWindow()->ImplClearLayoutData();

	mpBtn->SetPressed( sal_False );
    ImplCallEventListeners( VCLEVENT_DROPDOWN_CLOSE );
	return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ComboBox, ImplAutocompleteHdl, Edit*, pEdit )
{
	Selection			aSel = pEdit->GetSelection();
	AutocompleteAction	eAction = pEdit->GetAutocompleteAction();

    /* If there is no current selection do not auto complete on
       Tab/Shift-Tab since then we would not cycle to the next field.
    */
	if ( aSel.Len() ||
		 ((eAction != AUTOCOMPLETE_TABFORWARD) && (eAction != AUTOCOMPLETE_TABBACKWARD)) )
	{
		XubString	aFullText = pEdit->GetText();
		XubString	aStartText = aFullText.Copy( 0, (xub_StrLen)aSel.Max() );
		sal_uInt16		nStart = mpImplLB->GetCurrentPos();

		if ( nStart == LISTBOX_ENTRY_NOTFOUND )
			nStart = 0;

		sal_Bool bForward = sal_True;
		if ( eAction == AUTOCOMPLETE_TABFORWARD )
			nStart++;
		else if ( eAction == AUTOCOMPLETE_TABBACKWARD )
		{
			bForward = sal_False;
			nStart = nStart ? nStart - 1 : mpImplLB->GetEntryList()->GetEntryCount()-1;
		}
        
        sal_uInt16 nPos = LISTBOX_ENTRY_NOTFOUND;
        if( ! mbMatchCase )
        {
			// Try match case insensitive from current position
			nPos = mpImplLB->GetEntryList()->FindMatchingEntry( aStartText, nStart, bForward, sal_True );
            if ( nPos == LISTBOX_ENTRY_NOTFOUND )
                // Try match case insensitive, but from start
                nPos = mpImplLB->GetEntryList()->FindMatchingEntry( aStartText, bForward ? 0 : (mpImplLB->GetEntryList()->GetEntryCount()-1), bForward, sal_True );
        }
            
		if ( nPos == LISTBOX_ENTRY_NOTFOUND )
            // Try match full from current position
            nPos = mpImplLB->GetEntryList()->FindMatchingEntry( aStartText, nStart, bForward, sal_False );
		if ( nPos == LISTBOX_ENTRY_NOTFOUND )
			//  Match full, but from start
			nPos = mpImplLB->GetEntryList()->FindMatchingEntry( aStartText, bForward ? 0 : (mpImplLB->GetEntryList()->GetEntryCount()-1), bForward, sal_False );

		if ( nPos != LISTBOX_ENTRY_NOTFOUND )
		{
			XubString aText = mpImplLB->GetEntryList()->GetEntryText( nPos );
			Selection aSelection( aText.Len(), aStartText.Len() );
			pEdit->SetText( aText, aSelection );
		}
	}

	return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ComboBox, ImplSelectHdl, void*, EMPTYARG )
{
	sal_Bool bPopup = IsInDropDown();
	sal_Bool bCallSelect = sal_False;
	if ( mpImplLB->IsSelectionChanged() || bPopup )
	{
		XubString aText;
		if ( IsMultiSelectionEnabled() )
		{
			aText = mpSubEdit->GetText();

			// Alle Eintraege entfernen, zu denen es einen Entry gibt, der aber nicht selektiert ist.
			xub_StrLen nIndex = 0;
			while ( nIndex != STRING_NOTFOUND )
			{
				xub_StrLen	nPrevIndex = nIndex;
				XubString	aToken = aText.GetToken( 0, mcMultiSep, nIndex );
				xub_StrLen	nTokenLen = aToken.Len();
				aToken.EraseLeadingAndTrailingChars( ' ' );
				sal_uInt16		nP = mpImplLB->GetEntryList()->FindEntry( aToken );
				if ( (nP != LISTBOX_ENTRY_NOTFOUND) && (!mpImplLB->GetEntryList()->IsEntryPosSelected( nP )) )
				{
					aText.Erase( nPrevIndex, nTokenLen );
					nIndex = sal::static_int_cast<xub_StrLen>(nIndex - nTokenLen);
					if ( (nPrevIndex < aText.Len()) && (aText.GetChar( nPrevIndex ) == mcMultiSep) )
					{
						aText.Erase( nPrevIndex, 1 );
						nIndex--;
					}
				}
				aText.EraseLeadingAndTrailingChars( ' ' );
			}

			// Fehlende Eintraege anhaengen...
			Table aSelInText;
			lcl_GetSelectedEntries( aSelInText, aText, mcMultiSep, mpImplLB->GetEntryList() );
			sal_uInt16 nSelectedEntries = mpImplLB->GetEntryList()->GetSelectEntryCount();
			for ( sal_uInt16 n = 0; n < nSelectedEntries; n++ )
			{
				sal_uInt16 nP = mpImplLB->GetEntryList()->GetSelectEntryPos( n );
				if ( !aSelInText.IsKeyValid( ImplCreateKey( nP ) ) )
				{
					if ( aText.Len() && (aText.GetChar( aText.Len()-1 ) != mcMultiSep) )
						aText += mcMultiSep;
					if ( aText.Len() )
						aText += ' ';   // etwas auflockern
					aText += mpImplLB->GetEntryList()->GetEntryText( nP );
					aText += mcMultiSep;
				}
			}
			if ( aText.Len() && (aText.GetChar( aText.Len()-1 ) == mcMultiSep) )
				aText.Erase( aText.Len()-1, 1 );
		}
		else
		{
			aText = mpImplLB->GetEntryList()->GetSelectEntry( 0 );
		}

		mpSubEdit->SetText( aText );

		Selection aNewSelection( 0, aText.Len() );
		if ( IsMultiSelectionEnabled() )
			aNewSelection.Min() = aText.Len();
		mpSubEdit->SetSelection( aNewSelection );

		bCallSelect = sal_True;
	}

	// #84652# Call GrabFocus and EndPopupMode before calling Select/Modify, but after changing the text

	if ( bPopup && !mpImplLB->IsTravelSelect() &&
		( !IsMultiSelectionEnabled() || !mpImplLB->GetSelectModifier() ) )
	{
		mpFloatWin->EndPopupMode();
		GrabFocus();
	}

	if ( bCallSelect )
	{
		mpSubEdit->SetModifyFlag();
		mbSyntheticModify = sal_True;
		Modify();
		mbSyntheticModify = sal_False;
		Select();
	}

	return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ComboBox, ImplCancelHdl, void*, EMPTYARG )
{
	if( IsInDropDown() )
		mpFloatWin->EndPopupMode();

	return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( ComboBox, ImplSelectionChangedHdl, void*, n )
{
	if ( !mpImplLB->IsTrackingSelect() )
	{
		sal_uInt16 nChanged = (sal_uInt16)(sal_uLong)n;
		if ( !mpSubEdit->IsReadOnly() && mpImplLB->GetEntryList()->IsEntryPosSelected( nChanged ) )
			mpSubEdit->SetText( mpImplLB->GetEntryList()->GetEntryText( nChanged ) );
	}
	return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( ComboBox, ImplDoubleClickHdl, void*, EMPTYARG )
{
	DoubleClick();
	return 0;
}

// -----------------------------------------------------------------------

void ComboBox::ToggleDropDown()
{
    if( IsDropDownBox() )
    {
        if( mpFloatWin->IsInPopupMode() )
            mpFloatWin->EndPopupMode();
        else
        {
            mpSubEdit->GrabFocus();
            if ( !mpImplLB->GetEntryList()->GetMRUCount() )
                ImplUpdateFloatSelection();
            else
                mpImplLB->SelectEntry( 0 , sal_True );
            ImplCallEventListeners( VCLEVENT_DROPDOWN_PRE_OPEN );
            mpBtn->SetPressed( sal_True );
            SetSelection( Selection( 0, SELECTION_MAX ) );
            mpFloatWin->StartFloat( sal_True );
            ImplCallEventListeners( VCLEVENT_DROPDOWN_OPEN );
        }
    }
}

// -----------------------------------------------------------------------

void ComboBox::Select()
{
    ImplCallEventListenersAndHandler( VCLEVENT_COMBOBOX_SELECT, maSelectHdl, this );
}

// -----------------------------------------------------------------------

void ComboBox::DoubleClick()
{
    ImplCallEventListenersAndHandler( VCLEVENT_COMBOBOX_DOUBLECLICK, maDoubleClickHdl, this );
}

// -----------------------------------------------------------------------

void ComboBox::EnableAutoSize( sal_Bool bAuto )
{
	mbDDAutoSize = bAuto;
	if ( mpFloatWin )
	{
		if ( bAuto && !mpFloatWin->GetDropDownLineCount() )
			mpFloatWin->SetDropDownLineCount( 5 );
		else if ( !bAuto )
			mpFloatWin->SetDropDownLineCount( 0 );
	}
}

// -----------------------------------------------------------------------

void ComboBox::EnableDDAutoWidth( sal_Bool b )
{
    if ( mpFloatWin )
        mpFloatWin->SetAutoWidth( b );
}

 // -----------------------------------------------------------------------

sal_Bool ComboBox::IsDDAutoWidthEnabled() const
{
    return mpFloatWin ? mpFloatWin->IsAutoWidth() : sal_False;
}


// -----------------------------------------------------------------------

void ComboBox::SetDropDownLineCount( sal_uInt16 nLines )
{
	if ( mpFloatWin )
		mpFloatWin->SetDropDownLineCount( nLines );
}

// -----------------------------------------------------------------------

sal_uInt16 ComboBox::GetDropDownLineCount() const
{
	sal_uInt16 nLines = 0;
	if ( mpFloatWin )
		nLines = mpFloatWin->GetDropDownLineCount();
	return nLines;
}

// -----------------------------------------------------------------------

void ComboBox::SetPosSizePixel( long nX, long nY, long nWidth, long nHeight,
								sal_uInt16 nFlags )
{
	if( IsDropDownBox() && ( nFlags & WINDOW_POSSIZE_SIZE ) )
	{
		Size aPrefSz = mpFloatWin->GetPrefSize();
		if ( ( nFlags & WINDOW_POSSIZE_HEIGHT ) && ( nHeight >= 2*mnDDHeight ) )
			aPrefSz.Height() = nHeight-mnDDHeight;
		if ( nFlags & WINDOW_POSSIZE_WIDTH )
			aPrefSz.Width() = nWidth;
		mpFloatWin->SetPrefSize( aPrefSz );

		if ( IsAutoSizeEnabled() && ! (nFlags & WINDOW_POSSIZE_DROPDOWN) )
			nHeight = mnDDHeight;
	}

	Edit::SetPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
}

// -----------------------------------------------------------------------

void ComboBox::Resize()
{
    Control::Resize();

	Size aOutSz = GetOutputSizePixel();
	if( IsDropDownBox() )
	{
		long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
		long	nTop = 0;
		long	nBottom = aOutSz.Height();

        Window *pBorder = GetWindow( WINDOW_BORDER );
		ImplControlValue aControlValue;
		Point aPoint;
		Rectangle aContent, aBound;

        // use the full extent of the control
		Rectangle aArea( aPoint, pBorder->GetOutputSizePixel() );

		if ( GetNativeControlRegion(CTRL_COMBOBOX, PART_BUTTON_DOWN,
				aArea, 0, aControlValue, rtl::OUString(), aBound, aContent) )
		{
            // convert back from border space to local coordinates
            aPoint = pBorder->ScreenToOutputPixel( OutputToScreenPixel( aPoint ) );
            aContent.Move(-aPoint.X(), -aPoint.Y());

		    mpBtn->SetPosSizePixel( aContent.Left(), nTop, aContent.getWidth(), (nBottom-nTop) );

            // adjust the size of the edit field
            if ( GetNativeControlRegion(CTRL_COMBOBOX, PART_SUB_EDIT,
                        aArea, 0, aControlValue, rtl::OUString(), aBound, aContent) )
            {
                // convert back from border space to local coordinates
                aContent.Move(-aPoint.X(), -aPoint.Y());

                // use the themes drop down size
                mpSubEdit->SetPosSizePixel( aContent.TopLeft(), aContent.GetSize() );
            }
            else
            {
                // use the themes drop down size for the button
                aOutSz.Width() -= aContent.getWidth();
                mpSubEdit->SetSizePixel( aOutSz );
            }
		}
		else
        {
            nSBWidth = CalcZoom( nSBWidth );
		    mpSubEdit->SetPosSizePixel( Point( 0, 0 ), Size( aOutSz.Width() - nSBWidth, aOutSz.Height() ) );
		    mpBtn->SetPosSizePixel( aOutSz.Width() - nSBWidth, nTop, nSBWidth, (nBottom-nTop) );
        }
	}
	else
	{
		mpSubEdit->SetSizePixel( Size( aOutSz.Width(), mnDDHeight ) );
		mpImplLB->SetPosSizePixel( 0, mnDDHeight, aOutSz.Width(), aOutSz.Height() - mnDDHeight );
		if ( GetText().Len() )
			ImplUpdateFloatSelection();
	}

	// FloatingWindow-Groesse auch im unsichtbare Zustand auf Stand halten,
	// weil KEY_PGUP/DOWN ausgewertet wird...
	if ( mpFloatWin )
		mpFloatWin->SetSizePixel( mpFloatWin->CalcFloatSize() );
}

// -----------------------------------------------------------------------

void ComboBox::FillLayoutData() const
{
    mpControlData->mpLayoutData = new vcl::ControlLayoutData();
    AppendLayoutData( *mpSubEdit );
    mpSubEdit->SetLayoutDataParent( this );
    Control* pMainWindow = mpImplLB->GetMainWindow();
    if( mpFloatWin )
    {
        // dropdown mode
        if( mpFloatWin->IsReallyVisible() )
        {
            AppendLayoutData( *pMainWindow );
            pMainWindow->SetLayoutDataParent( this );
        }
    }
    else
    {
        AppendLayoutData( *pMainWindow );
        pMainWindow->SetLayoutDataParent( this );
    }
}

// -----------------------------------------------------------------------

void ComboBox::StateChanged( StateChangedType nType )
{
	Edit::StateChanged( nType );

	if ( nType == STATE_CHANGE_READONLY )
	{
		mpImplLB->SetReadOnly( IsReadOnly() );
		if ( mpBtn )
			mpBtn->Enable( IsEnabled() && !IsReadOnly() );
	}
	else if ( nType == STATE_CHANGE_ENABLE )
	{
		mpSubEdit->Enable( IsEnabled() );
		mpImplLB->Enable( IsEnabled() && !IsReadOnly() );
		if ( mpBtn )
			mpBtn->Enable( IsEnabled() && !IsReadOnly() );
		Invalidate();
	}
	else if( nType == STATE_CHANGE_UPDATEMODE )
	{
		mpImplLB->SetUpdateMode( IsUpdateMode() );
	}
	else if ( nType == STATE_CHANGE_ZOOM )
	{
		mpImplLB->SetZoom( GetZoom() );
		mpSubEdit->SetZoom( GetZoom() );
		ImplCalcEditHeight();
		Resize();
	}
	else if ( nType == STATE_CHANGE_CONTROLFONT )
	{
		mpImplLB->SetControlFont( GetControlFont() );
		mpSubEdit->SetControlFont( GetControlFont() );
		ImplCalcEditHeight();
		Resize();
	}
	else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
	{
		mpImplLB->SetControlForeground( GetControlForeground() );
		mpSubEdit->SetControlForeground( GetControlForeground() );
	}
	else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
	{
		mpImplLB->SetControlBackground( GetControlBackground() );
		mpSubEdit->SetControlBackground( GetControlBackground() );
	}
	else if ( nType == STATE_CHANGE_STYLE )
	{
		SetStyle( ImplInitStyle( GetStyle() ) );
		mpImplLB->GetMainWindow()->EnableSort( ( GetStyle() & WB_SORT ) ? sal_True : sal_False );
	}
    else if( nType == STATE_CHANGE_MIRRORING )
    {
        if( mpBtn )
        {
            mpBtn->EnableRTL( IsRTLEnabled() );
            ImplInitDropDownButton( mpBtn );
        }
        mpSubEdit->StateChanged( STATE_CHANGE_MIRRORING );
        mpImplLB->EnableRTL( IsRTLEnabled() );
        Resize();
    }
}

// -----------------------------------------------------------------------

void ComboBox::DataChanged( const DataChangedEvent& rDCEvt )
{
	Control::DataChanged( rDCEvt );

	if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
		 (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
		 ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
		  (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
	{
		if ( mpBtn )
		{
			mpBtn->SetSettings( GetSettings() );
			ImplInitDropDownButton( mpBtn );
		}
		Resize();
		mpImplLB->Resize(); // Wird nicht durch ComboBox::Resize() gerufen, wenn sich die ImplLB nicht aendert.
        SetBackground();    // due to a hack in Window::UpdateSettings the background must be reset 
                            // otherwise it will overpaint NWF drawn comboboxes
	}
}

// -----------------------------------------------------------------------

long ComboBox::PreNotify( NotifyEvent& rNEvt )
{

	return Edit::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long ComboBox::Notify( NotifyEvent& rNEvt )
{
	long nDone = 0;
	if( ( rNEvt.GetType() == EVENT_KEYINPUT ) && ( rNEvt.GetWindow() == mpSubEdit )
			&& !IsReadOnly() )
	{
		KeyEvent aKeyEvt = *rNEvt.GetKeyEvent();
		sal_uInt16	 nKeyCode = aKeyEvt.GetKeyCode().GetCode();
		switch( nKeyCode )
		{
			case KEY_UP:
			case KEY_DOWN:
			case KEY_PAGEUP:
			case KEY_PAGEDOWN:
			{
				ImplUpdateFloatSelection();
				if( ( nKeyCode == KEY_DOWN ) && mpFloatWin && !mpFloatWin->IsInPopupMode() && aKeyEvt.GetKeyCode().IsMod2() )
				{
                    ImplCallEventListeners( VCLEVENT_DROPDOWN_PRE_OPEN );
					mpBtn->SetPressed( sal_True );
					if ( mpImplLB->GetEntryList()->GetMRUCount() )
						mpImplLB->SelectEntry( 0 , sal_True );
                    SetSelection( Selection( 0, SELECTION_MAX ) );
					mpFloatWin->StartFloat( sal_False );
                    ImplCallEventListeners( VCLEVENT_DROPDOWN_OPEN );
					nDone = 1;
				}
				else if( ( nKeyCode == KEY_UP ) && mpFloatWin && mpFloatWin->IsInPopupMode() && aKeyEvt.GetKeyCode().IsMod2() )
				{
					mpFloatWin->EndPopupMode();
					nDone = 1;
				}
				else
                {
					nDone = mpImplLB->ProcessKeyInput( aKeyEvt );
                }
			}
			break;

			case KEY_RETURN:
			{
				if( ( rNEvt.GetWindow() == mpSubEdit ) && IsInDropDown() )
				{
					mpImplLB->ProcessKeyInput( aKeyEvt );
					nDone = 1;
				}
			}
			break;
		}
	}
	else if ( (rNEvt.GetType() == EVENT_LOSEFOCUS) && mpFloatWin )
	{
		if( mpFloatWin->HasChildPathFocus() )
			mpSubEdit->GrabFocus();
		else if ( mpFloatWin->IsInPopupMode() && !HasChildPathFocus( sal_True ) )
			mpFloatWin->EndPopupMode();
	}
	else if( (rNEvt.GetType() == EVENT_COMMAND) &&
			 (rNEvt.GetCommandEvent()->GetCommand() == COMMAND_WHEEL) &&
			 (rNEvt.GetWindow() == mpSubEdit) )
	{
        sal_uInt16 nWheelBehavior( GetSettings().GetMouseSettings().GetWheelBehavior() );
        if  (   ( nWheelBehavior == MOUSE_WHEEL_ALWAYS )
            ||  (   ( nWheelBehavior == MOUSE_WHEEL_FOCUS_ONLY )
                &&  HasChildPathFocus()
                )
            )
        {
            nDone = mpImplLB->HandleWheelAsCursorTravel( *rNEvt.GetCommandEvent() );
        }
        else
        {
            nDone = 0;  // don't eat this event, let the default handling happen (i.e. scroll the context)
        }
	}
    else if( ( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN ) && ( rNEvt.GetWindow() == mpImplLB->GetMainWindow() ) )
	{
		mpSubEdit->GrabFocus();
	}

	return nDone ? nDone : Edit::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void ComboBox::SetText( const XubString& rStr )
{
    ImplCallEventListeners( VCLEVENT_COMBOBOX_SETTEXT );

	Edit::SetText( rStr );
	ImplUpdateFloatSelection();
}

// -----------------------------------------------------------------------

void ComboBox::SetText( const XubString& rStr, const Selection& rNewSelection )
{
    ImplCallEventListeners( VCLEVENT_COMBOBOX_SETTEXT );

	Edit::SetText( rStr, rNewSelection );
	ImplUpdateFloatSelection();
}

// -----------------------------------------------------------------------

void ComboBox::Modify()
{
	if ( !mbSyntheticModify )
		ImplUpdateFloatSelection();

	Edit::Modify();
}

// -----------------------------------------------------------------------

void ComboBox::ImplUpdateFloatSelection()
{
	// Text in der ListBox in den sichtbaren Bereich bringen
	mpImplLB->SetCallSelectionChangedHdl( sal_False );
	if ( !IsMultiSelectionEnabled() )
	{
		XubString	aSearchStr( mpSubEdit->GetText() );
		sal_uInt16		nSelect = LISTBOX_ENTRY_NOTFOUND;
		sal_Bool		bSelect = sal_True;

		if ( mpImplLB->GetCurrentPos() != LISTBOX_ENTRY_NOTFOUND )
		{
			XubString aCurrent = mpImplLB->GetEntryList()->GetEntryText( mpImplLB->GetCurrentPos() );
			if ( aCurrent == aSearchStr )
				nSelect = mpImplLB->GetCurrentPos();
		}

		if ( nSelect == LISTBOX_ENTRY_NOTFOUND )
			nSelect = mpImplLB->GetEntryList()->FindEntry( aSearchStr );
		if ( nSelect == LISTBOX_ENTRY_NOTFOUND )
		{
			nSelect = mpImplLB->GetEntryList()->FindMatchingEntry( aSearchStr );
			bSelect = sal_False;
		}

		if( nSelect != LISTBOX_ENTRY_NOTFOUND )
		{
			if ( !mpImplLB->IsVisible( nSelect ) )
				mpImplLB->ShowProminentEntry( nSelect );
			mpImplLB->SelectEntry( nSelect, bSelect );
		}
		else
		{
			nSelect = mpImplLB->GetEntryList()->GetSelectEntryPos( 0 );
			if( nSelect != LISTBOX_ENTRY_NOTFOUND )
				mpImplLB->SelectEntry( nSelect, sal_False );
			mpImplLB->ResetCurrentPos();
		}
	}
	else
	{
		Table aSelInText;
		lcl_GetSelectedEntries( aSelInText, mpSubEdit->GetText(), mcMultiSep, mpImplLB->GetEntryList() );
		for ( sal_uInt16 n = 0; n < mpImplLB->GetEntryList()->GetEntryCount(); n++ )
			mpImplLB->SelectEntry( n, aSelInText.IsKeyValid( ImplCreateKey( n ) ) );
	}
	mpImplLB->SetCallSelectionChangedHdl( sal_True );
}

// -----------------------------------------------------------------------

sal_uInt16 ComboBox::InsertEntry( const XubString& rStr, sal_uInt16 nPos )
{
	sal_uInt16 nRealPos = mpImplLB->InsertEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount(), rStr );
	nRealPos = sal::static_int_cast<sal_uInt16>(nRealPos - mpImplLB->GetEntryList()->GetMRUCount());
    CallEventListeners( VCLEVENT_COMBOBOX_ITEMADDED, (void*) sal_IntPtr(nRealPos) );
	return nRealPos;
}

// -----------------------------------------------------------------------

sal_uInt16 ComboBox::InsertEntry( const XubString& rStr, const Image& rImage, sal_uInt16 nPos )
{
	sal_uInt16 nRealPos = mpImplLB->InsertEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount(), rStr, rImage );
	nRealPos = sal::static_int_cast<sal_uInt16>(nRealPos - mpImplLB->GetEntryList()->GetMRUCount());
    CallEventListeners( VCLEVENT_COMBOBOX_ITEMADDED, (void*) sal_IntPtr(nRealPos) );
	return nRealPos;
}

// -----------------------------------------------------------------------

void ComboBox::RemoveEntry( const XubString& rStr )
{
	RemoveEntry( GetEntryPos( rStr ) );
}

// -----------------------------------------------------------------------

void ComboBox::RemoveEntry( sal_uInt16 nPos )
{
	mpImplLB->RemoveEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
    CallEventListeners( VCLEVENT_COMBOBOX_ITEMREMOVED, (void*) sal_IntPtr(nPos) );
}

// -----------------------------------------------------------------------

void ComboBox::Clear()
{
	mpImplLB->Clear();
    CallEventListeners( VCLEVENT_COMBOBOX_ITEMREMOVED, (void*) sal_IntPtr(-1) );
}
// -----------------------------------------------------------------------

Image ComboBox::GetEntryImage( sal_uInt16 nPos ) const
{
    if ( mpImplLB->GetEntryList()->HasEntryImage( nPos ) )
        return mpImplLB->GetEntryList()->GetEntryImage( nPos );
    return Image();
}

// -----------------------------------------------------------------------

sal_uInt16 ComboBox::GetEntryPos( const XubString& rStr ) const
{
	sal_uInt16 nPos = mpImplLB->GetEntryList()->FindEntry( rStr );
	if ( nPos != LISTBOX_ENTRY_NOTFOUND )
		nPos = sal::static_int_cast<sal_uInt16>(nPos - mpImplLB->GetEntryList()->GetMRUCount());
	return nPos;
}

// -----------------------------------------------------------------------

sal_uInt16 ComboBox::GetEntryPos( const void* pData ) const
{
	sal_uInt16 nPos = mpImplLB->GetEntryList()->FindEntry( pData );
	if ( nPos != LISTBOX_ENTRY_NOTFOUND )
		nPos = sal::static_int_cast<sal_uInt16>(nPos - mpImplLB->GetEntryList()->GetMRUCount());
	return nPos;
}

// -----------------------------------------------------------------------

XubString ComboBox::GetEntry( sal_uInt16 nPos ) const
{
	return mpImplLB->GetEntryList()->GetEntryText( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

// -----------------------------------------------------------------------

sal_uInt16 ComboBox::GetEntryCount() const
{
	return mpImplLB->GetEntryList()->GetEntryCount() - mpImplLB->GetEntryList()->GetMRUCount();
}

// -----------------------------------------------------------------------

sal_Bool ComboBox::IsTravelSelect() const
{
	return mpImplLB->IsTravelSelect();
}

// -----------------------------------------------------------------------

sal_Bool ComboBox::IsInDropDown() const
{
	return mpFloatWin && mpFloatWin->IsInPopupMode();
}

// -----------------------------------------------------------------------

void ComboBox::EnableMultiSelection( sal_Bool bMulti )
{
	mpImplLB->EnableMultiSelection( bMulti, sal_False );
	mpImplLB->SetMultiSelectionSimpleMode( sal_True );
}

// -----------------------------------------------------------------------

sal_Bool ComboBox::IsMultiSelectionEnabled() const
{
	return mpImplLB->IsMultiSelectionEnabled();
}

// -----------------------------------------------------------------------

long ComboBox::CalcWindowSizePixel( sal_uInt16 nLines ) const
{
	return mpImplLB->GetEntryHeight() * nLines;
}

// -----------------------------------------------------------------------

Size ComboBox::GetOptimalSize(WindowSizeType eType) const
{
    switch (eType) {
    case WINDOWSIZE_MINIMUM:
        return CalcMinimumSize();
    default:
        return Edit::GetOptimalSize( eType );
    }
}

// -----------------------------------------------------------------------

Size ComboBox::CalcMinimumSize() const
{
	Size aSz;
	if ( !IsDropDownBox() )
	{
		aSz = mpImplLB->CalcSize( mpImplLB->GetEntryList()->GetEntryCount() );
		aSz.Height() += mnDDHeight;
	}
	else
	{
		aSz.Height() = mpImplLB->CalcSize( 1 ).Height();
		aSz.Width() = mpImplLB->GetMaxEntryWidth();
		aSz.Width() += GetSettings().GetStyleSettings().GetScrollBarSize();
	}

	aSz = CalcWindowSize( aSz );
	return aSz;
}

// -----------------------------------------------------------------------

Size ComboBox::CalcAdjustedSize( const Size& rPrefSize ) const
{
	Size aSz = rPrefSize;
	sal_Int32 nLeft, nTop, nRight, nBottom;
	((Window*)this)->GetBorder( nLeft, nTop, nRight, nBottom );
	aSz.Height() -= nTop+nBottom;
	if ( !IsDropDownBox() )
	{
		long nEntryHeight = CalcSize( 1, 1 ).Height();
		long nLines = aSz.Height() / nEntryHeight;
		if ( nLines < 1 )
			nLines = 1;
		aSz.Height() = nLines * nEntryHeight;
		aSz.Height() += mnDDHeight;
	}
	else
	{
		aSz.Height() = mnDDHeight;
	}
	aSz.Height() += nTop+nBottom;

	aSz = CalcWindowSize( aSz );
	return aSz;
}

// -----------------------------------------------------------------------

Size ComboBox::CalcSize( sal_uInt16 nColumns, sal_uInt16 nLines ) const
{
	// ggf. werden ScrollBars eingeblendet
	Size aMinSz = CalcMinimumSize();
	Size aSz;

	// Hoehe
	if ( nLines )
	{
		if ( !IsDropDownBox() )
			aSz.Height() = mpImplLB->CalcSize( nLines ).Height() + mnDDHeight;
		else
			aSz.Height() = mnDDHeight;
	}
	else
		aSz.Height() = aMinSz.Height();

	// Breite
	if ( nColumns )
		aSz.Width() = nColumns * GetTextWidth( UniString( 'X' ) );
	else
		aSz.Width() = aMinSz.Width();

	if ( IsDropDownBox() )
		aSz.Width() += GetSettings().GetStyleSettings().GetScrollBarSize();

	if ( !IsDropDownBox() )
	{
		if ( aSz.Width() < aMinSz.Width() )
			aSz.Height() += GetSettings().GetStyleSettings().GetScrollBarSize();
		if ( aSz.Height() < aMinSz.Height() )
			aSz.Width() += GetSettings().GetStyleSettings().GetScrollBarSize();
	}

	aSz = CalcWindowSize( aSz );
	return aSz;
}

// -----------------------------------------------------------------------

void ComboBox::GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const
{
	long nCharWidth = GetTextWidth( UniString( 'x' ) );
	if ( !IsDropDownBox() )
	{
		Size aOutSz = mpImplLB->GetMainWindow()->GetOutputSizePixel();
		rnCols = (sal_uInt16)(aOutSz.Width()/nCharWidth);
		rnLines = (sal_uInt16)(aOutSz.Height()/mpImplLB->GetEntryHeight());
	}
	else
	{
		Size aOutSz = mpSubEdit->GetOutputSizePixel();
		rnCols = (sal_uInt16)(aOutSz.Width()/nCharWidth);
		rnLines = 1;
	}
}

// -----------------------------------------------------------------------

void ComboBox::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags )
{
	mpImplLB->GetMainWindow()->ImplInitSettings( sal_True, sal_True, sal_True );

	Point aPos = pDev->LogicToPixel( rPos );
	Size aSize = pDev->LogicToPixel( rSize );
	Font aFont = mpImplLB->GetMainWindow()->GetDrawPixelFont( pDev );
	OutDevType eOutDevType = pDev->GetOutDevType();

	pDev->Push();
	pDev->SetMapMode();
	pDev->SetFont( aFont );
	pDev->SetTextFillColor();

	// Border/Background
	pDev->SetLineColor();
	pDev->SetFillColor();
	sal_Bool bBorder = !(nFlags & WINDOW_DRAW_NOBORDER ) && (GetStyle() & WB_BORDER);
	sal_Bool bBackground = !(nFlags & WINDOW_DRAW_NOBACKGROUND) && IsControlBackground();
	if ( bBorder || bBackground )
	{
		Rectangle aRect( aPos, aSize );
		// aRect.Top() += nEditHeight;
		if ( bBorder )
		{
            ImplDrawFrame( pDev, aRect );
		}
		if ( bBackground )
		{
			pDev->SetFillColor( GetControlBackground() );
			pDev->DrawRect( aRect );
		}
	}

	// Inhalt
	if ( !IsDropDownBox() )
	{
	    long        nOnePixel = GetDrawPixel( pDev, 1 );
		long        nTextHeight = pDev->GetTextHeight();
		long        nEditHeight = nTextHeight + 6*nOnePixel;
        sal_uInt16      nTextStyle = TEXT_DRAW_VCENTER;

		// First, draw the edit part
        mpSubEdit->Draw( pDev, aPos, Size( aSize.Width(), nEditHeight ), nFlags );

        // Second, draw the listbox
        if ( GetStyle() & WB_CENTER )
            nTextStyle |= TEXT_DRAW_CENTER;
        else if ( GetStyle() & WB_RIGHT )
            nTextStyle |= TEXT_DRAW_RIGHT;
        else
            nTextStyle |= TEXT_DRAW_LEFT;

		if ( ( nFlags & WINDOW_DRAW_MONO ) || ( eOutDevType == OUTDEV_PRINTER ) )
		{
			pDev->SetTextColor( Color( COL_BLACK ) );
		}
		else
		{
			if ( !(nFlags & WINDOW_DRAW_NODISABLE ) && !IsEnabled() )
			{
				const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
				pDev->SetTextColor( rStyleSettings.GetDisableColor() );
			}
			else
			{
				pDev->SetTextColor( GetTextColor() );
			}
		}

		Rectangle aClip( aPos, aSize );
		pDev->IntersectClipRegion( aClip );
		sal_uInt16 nLines = (sal_uInt16) ( (aSize.Height()-nEditHeight) / nTextHeight );
		if ( !nLines )
			nLines = 1;
		sal_uInt16 nTEntry = IsReallyVisible() ? mpImplLB->GetTopEntry() : 0;
		
        Rectangle aTextRect( aPos, aSize );
        
        aTextRect.Left() += 3*nOnePixel;
        aTextRect.Right() -= 3*nOnePixel;
        aTextRect.Top() += nEditHeight + nOnePixel;
        aTextRect.Bottom() = aTextRect.Top() + nTextHeight;

        // the drawing starts here
        for ( sal_uInt16 n = 0; n < nLines; n++ )
        {
			pDev->DrawText( aTextRect, mpImplLB->GetEntryList()->GetEntryText( n+nTEntry ), nTextStyle );
            aTextRect.Top() += nTextHeight;
            aTextRect.Bottom() += nTextHeight;
        }
	}

	pDev->Pop();
	
    // Call Edit::Draw after restoring the MapMode...
	if ( IsDropDownBox() )
	{
		mpSubEdit->Draw( pDev, rPos, rSize, nFlags );
		// DD-Button ?
	}
	
}

// -----------------------------------------------------------------------

IMPL_LINK( ComboBox, ImplUserDrawHdl, UserDrawEvent*, pEvent )
{
	UserDraw( *pEvent );
	return 1;
}

// -----------------------------------------------------------------------

void ComboBox::UserDraw( const UserDrawEvent& )
{
}

// -----------------------------------------------------------------------

void ComboBox::SetUserItemSize( const Size& rSz )
{
	mpImplLB->GetMainWindow()->SetUserItemSize( rSz );
}

// -----------------------------------------------------------------------

const Size& ComboBox::GetUserItemSize() const
{
	return mpImplLB->GetMainWindow()->GetUserItemSize();
}

// -----------------------------------------------------------------------

void ComboBox::EnableUserDraw( sal_Bool bUserDraw )
{
	mpImplLB->GetMainWindow()->EnableUserDraw( bUserDraw );
}

// -----------------------------------------------------------------------

sal_Bool ComboBox::IsUserDrawEnabled() const
{
	return mpImplLB->GetMainWindow()->IsUserDrawEnabled();
}

// -----------------------------------------------------------------------

void ComboBox::DrawEntry( const UserDrawEvent& rEvt, sal_Bool bDrawImage, sal_Bool bDrawText, sal_Bool bDrawTextAtImagePos )
{
	DBG_ASSERT( rEvt.GetDevice() == mpImplLB->GetMainWindow(), "DrawEntry?!" );
	mpImplLB->GetMainWindow()->DrawEntry( rEvt.GetItemId(), bDrawImage, bDrawText, bDrawTextAtImagePos );
}

// -----------------------------------------------------------------------

void ComboBox::SetSeparatorPos( sal_uInt16 n )
{
	mpImplLB->SetSeparatorPos( n );
}

// -----------------------------------------------------------------------

void ComboBox::SetSeparatorPos()
{
	mpImplLB->SetSeparatorPos( LISTBOX_ENTRY_NOTFOUND );
}

// -----------------------------------------------------------------------

sal_uInt16 ComboBox::GetSeparatorPos() const
{
	return mpImplLB->GetSeparatorPos();
}

// -----------------------------------------------------------------------

void ComboBox::SetMRUEntries( const XubString& rEntries, xub_Unicode cSep )
{
	mpImplLB->SetMRUEntries( rEntries, cSep );
}

// -----------------------------------------------------------------------

XubString ComboBox::GetMRUEntries( xub_Unicode cSep ) const
{
	return mpImplLB->GetMRUEntries( cSep );
}

// -----------------------------------------------------------------------

void ComboBox::SetMaxMRUCount( sal_uInt16 n )
{
	mpImplLB->SetMaxMRUCount( n );
}

// -----------------------------------------------------------------------

sal_uInt16 ComboBox::GetMaxMRUCount() const
{
	return mpImplLB->GetMaxMRUCount();
}

// -----------------------------------------------------------------------

sal_uInt16 ComboBox::GetDisplayLineCount() const
{
    return mpImplLB->GetDisplayLineCount();
}

// -----------------------------------------------------------------------

void ComboBox::SetEntryData( sal_uInt16 nPos, void* pNewData )
{
	mpImplLB->SetEntryData( nPos + mpImplLB->GetEntryList()->GetMRUCount(), pNewData );
}

// -----------------------------------------------------------------------

void* ComboBox::GetEntryData( sal_uInt16 nPos ) const
{
	return mpImplLB->GetEntryList()->GetEntryData( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

// -----------------------------------------------------------------------

void ComboBox::SetTopEntry( sal_uInt16 nPos )
{
	mpImplLB->SetTopEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

// -----------------------------------------------------------------------

void ComboBox::ShowProminentEntry( sal_uInt16 nPos )
{
	mpImplLB->ShowProminentEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

// -----------------------------------------------------------------------

sal_uInt16 ComboBox::GetTopEntry() const
{
	sal_uInt16 nPos = GetEntryCount() ? mpImplLB->GetTopEntry() : LISTBOX_ENTRY_NOTFOUND;
	if ( nPos < mpImplLB->GetEntryList()->GetMRUCount() )
		nPos = 0;
	return nPos;
}

// -----------------------------------------------------------------------

void ComboBox::SetProminentEntryType( ProminentEntry eType )
{
    mpImplLB->SetProminentEntryType( eType );
}

// -----------------------------------------------------------------------

ProminentEntry ComboBox::GetProminentEntryType() const
{
    return mpImplLB->GetProminentEntryType();
}

// -----------------------------------------------------------------------

Rectangle ComboBox::GetDropDownPosSizePixel() const
{
    return mpFloatWin ? mpFloatWin->GetWindowExtentsRelative( const_cast<ComboBox*>(this) ) : Rectangle();
}

// -----------------------------------------------------------------------

Rectangle ComboBox::GetListPosSizePixel() const
{
    return mpFloatWin ? Rectangle() : mpImplLB->GetMainWindow()->GetWindowExtentsRelative( const_cast<ComboBox*>(this) );
}

// -----------------------------------------------------------------------

const Wallpaper& ComboBox::GetDisplayBackground() const
{
    if( ! mpSubEdit->IsBackground() )
        return Control::GetDisplayBackground();

    const Wallpaper& rBack = mpSubEdit->GetBackground();
    if( ! rBack.IsBitmap() &&
        ! rBack.IsGradient() &&
        rBack.GetColor().GetColor() == COL_TRANSPARENT
        )
        return Control::GetDisplayBackground();
    return rBack;
}
// -----------------------------------------------------------------------------
sal_uInt16 ComboBox::GetSelectEntryCount() const
{
	return mpImplLB->GetEntryList()->GetSelectEntryCount();
}
// -----------------------------------------------------------------------------
sal_uInt16 ComboBox::GetSelectEntryPos( sal_uInt16 nIndex ) const
{
	sal_uInt16 nPos = mpImplLB->GetEntryList()->GetSelectEntryPos( nIndex );
	if ( nPos != LISTBOX_ENTRY_NOTFOUND )
	{
		if ( nPos < mpImplLB->GetEntryList()->GetMRUCount() )
			nPos = mpImplLB->GetEntryList()->FindEntry( mpImplLB->GetEntryList()->GetEntryText( nPos ) );
		nPos = sal::static_int_cast<sal_uInt16>(nPos - mpImplLB->GetEntryList()->GetMRUCount());
	}
	return nPos;
}
// -----------------------------------------------------------------------------
sal_Bool ComboBox::IsEntryPosSelected( sal_uInt16 nPos ) const
{
	return mpImplLB->GetEntryList()->IsEntryPosSelected( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}
// -----------------------------------------------------------------------------
void ComboBox::SelectEntryPos( sal_uInt16 nPos, sal_Bool bSelect)
{
	if ( nPos < mpImplLB->GetEntryList()->GetEntryCount() )
		mpImplLB->SelectEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount(), bSelect );
}
// -----------------------------------------------------------------------------
void ComboBox::SetNoSelection()
{
	mpImplLB->SetNoSelection();
	mpSubEdit->SetText( String() );
}
// -----------------------------------------------------------------------------
Rectangle ComboBox::GetBoundingRectangle( sal_uInt16 nItem ) const
{
    Rectangle aRect = mpImplLB->GetMainWindow()->GetBoundingRectangle( nItem );
    Rectangle aOffset = mpImplLB->GetMainWindow()->GetWindowExtentsRelative( (Window*)this );
    aRect.Move( aOffset.TopLeft().X(), aOffset.TopLeft().Y() );
    return aRect;
}
// -----------------------------------------------------------------------------

void ComboBox::SetBorderStyle( sal_uInt16 nBorderStyle )
{
	Window::SetBorderStyle( nBorderStyle );
	if ( !IsDropDownBox() )
	{
		mpSubEdit->SetBorderStyle( nBorderStyle );
		mpImplLB->SetBorderStyle( nBorderStyle );
	}
}
// -----------------------------------------------------------------------------

long ComboBox::GetIndexForPoint( const Point& rPoint, sal_uInt16& rPos ) const
{
    if( !HasLayoutData() )
        FillLayoutData();

    // check whether rPoint fits at all
    long nIndex = Control::GetIndexForPoint( rPoint );
    if( nIndex != -1 )
    {
        // point must be either in main list window
        // or in impl window (dropdown case)
        ImplListBoxWindow* pMain = mpImplLB->GetMainWindow();
    
        // convert coordinates to ImplListBoxWindow pixel coordinate space
        Point aConvPoint = LogicToPixel( rPoint );
        aConvPoint = OutputToAbsoluteScreenPixel( aConvPoint );
        aConvPoint = pMain->AbsoluteScreenToOutputPixel( aConvPoint );
        aConvPoint = pMain->PixelToLogic( aConvPoint );

        // try to find entry
        sal_uInt16 nEntry = pMain->GetEntryPosForPoint( aConvPoint );
        if( nEntry == LISTBOX_ENTRY_NOTFOUND )
            nIndex = -1;
        else
            rPos = nEntry;
    }

    // get line relative index
    if( nIndex != -1 )
        nIndex = ToRelativeLineIndex( nIndex );

    return nIndex;
}
