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

#include <tools/list.hxx>
#include <tools/debug.hxx>
#include <tools/rc.h>

#include <vcl/event.hxx>
#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/status.hxx>
#include <vcl/virdev.hxx>

#include <svdata.hxx>
#include <window.h>

// =======================================================================

#define STATUSBAR_OFFSET_X		STATUSBAR_OFFSET
#define STATUSBAR_OFFSET_Y		2
#define STATUSBAR_OFFSET_TEXTY	3

#define STATUSBAR_PRGS_OFFSET	3
#define STATUSBAR_PRGS_COUNT	100
#define STATUSBAR_PRGS_MIN		5

// -----------------------------------------------------------------------

class StatusBar::ImplData
{
public:
    ImplData();
    ~ImplData();

    VirtualDevice*		mpVirDev;
    long                mnItemBorderWidth;
    bool                mbTopBorder:1;
    bool                mbDrawItemFrames:1;
};

StatusBar::ImplData::ImplData()
{
    mpVirDev = NULL;
    mbTopBorder = false;
    mbDrawItemFrames = false;
    mnItemBorderWidth = 0;
}

StatusBar::ImplData::~ImplData()
{
}

struct ImplStatusItem
{
	sal_uInt16				mnId;
	StatusBarItemBits	mnBits;
	long				mnWidth;
	long				mnOffset;
	long				mnExtraWidth;
	long				mnX;
	XubString			maText;
	XubString			maHelpText;
	XubString			maQuickHelpText;
	rtl::OString		maHelpId;
	void*				mpUserData;
	sal_Bool				mbVisible;
	XubString			maAccessibleName;
    XubString           maCommand;
};

DECLARE_LIST( ImplStatusItemList, ImplStatusItem* )

// =======================================================================

inline long ImplCalcProgessWidth( sal_uInt16 nMax, long nSize )
{
	return ((nMax*(nSize+(nSize/2)))-(nSize/2)+(STATUSBAR_PRGS_OFFSET*2));
}

// -----------------------------------------------------------------------

static Point ImplGetItemTextPos( const Size& rRectSize, const Size& rTextSize,
								 sal_uInt16 nStyle )
{
	long nX;
	long nY;
    long delta = (rTextSize.Height()/4) + 1;
    if( delta + rTextSize.Width() > rRectSize.Width() )
        delta = 0;

	if ( nStyle & SIB_LEFT )
		nX = delta;
	else if ( nStyle & SIB_RIGHT )
		nX = rRectSize.Width()-rTextSize.Width()-delta;
	else // SIB_CENTER
		nX = (rRectSize.Width()-rTextSize.Width())/2;
	nY = (rRectSize.Height()-rTextSize.Height())/2 + 1;
	return Point( nX, nY );
}

// -----------------------------------------------------------------------

sal_Bool StatusBar::ImplIsItemUpdate()
{
	if ( !mbProgressMode && mbVisibleItems && IsReallyVisible() && IsUpdateMode() )
		return sal_True;
	else
		return sal_False;
}

// -----------------------------------------------------------------------

void StatusBar::ImplInit( Window* pParent, WinBits nStyle )
{
    mpImplData = new ImplData;

	// Default ist RightAlign
	if ( !(nStyle & (WB_LEFT | WB_RIGHT)) )
		nStyle |= WB_RIGHT;

	Window::ImplInit( pParent, nStyle & ~WB_BORDER, NULL );

	// WinBits merken
	mpItemList		= new ImplStatusItemList;
	mpImplData->mpVirDev		= new VirtualDevice( *this );
	mnCurItemId 	= 0;
	mbFormat		= sal_True;
	mbVisibleItems	= sal_True;
	mbProgressMode	= sal_False;
	mbInUserDraw	= sal_False;
	mbBottomBorder	= sal_False;
    mnItemsWidth    = STATUSBAR_OFFSET_X;
	mnDX			= 0;
	mnDY			= 0;
	mnCalcHeight	= 0;
	mnItemY 		= STATUSBAR_OFFSET_Y;
	mnTextY 		= STATUSBAR_OFFSET_TEXTY;

	ImplInitSettings( sal_True, sal_True, sal_True );
	SetLineColor();

    SetOutputSizePixel( CalcWindowSizePixel() );
}

// -----------------------------------------------------------------------

StatusBar::StatusBar( Window* pParent, WinBits nStyle ) :
	Window( WINDOW_STATUSBAR )
{
	ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

StatusBar::StatusBar( Window* pParent, const ResId& rResId ) :
	Window( WINDOW_STATUSBAR )
{
	rResId.SetRT( RSC_STATUSBAR );
	WinBits nStyle = ImplInitRes( rResId );
	ImplInit( pParent, nStyle );
	ImplLoadRes( rResId );

	if ( !(nStyle & WB_HIDE) )
		Show();
}

// -----------------------------------------------------------------------

StatusBar::~StatusBar()
{
	// Alle Items loeschen
	ImplStatusItem* pItem = mpItemList->First();
	while ( pItem )
	{
		delete pItem;
		pItem = mpItemList->Next();
	}

	delete mpItemList;

	// VirtualDevice loeschen
	delete mpImplData->mpVirDev;

    delete mpImplData;
}

// -----------------------------------------------------------------------

void StatusBar::ImplInitSettings( sal_Bool bFont,
								  sal_Bool bForeground, sal_Bool bBackground )
{
	const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

	if ( bFont )
	{
		Font aFont = rStyleSettings.GetToolFont();
		if ( IsControlFont() )
			aFont.Merge( GetControlFont() );
		SetZoomedPointFont( aFont );
	}

	if ( bForeground || bFont )
	{
		Color aColor;
		if ( IsControlForeground() )
			aColor = GetControlForeground();
		else if ( GetStyle() & WB_3DLOOK )
			aColor = rStyleSettings.GetButtonTextColor();
		else
			aColor = rStyleSettings.GetWindowTextColor();
		SetTextColor( aColor );
		SetTextFillColor();

		mpImplData->mpVirDev->SetFont( GetFont() );
		mpImplData->mpVirDev->SetTextColor( GetTextColor() );
		mpImplData->mpVirDev->SetTextAlign( GetTextAlign() );
		mpImplData->mpVirDev->SetTextFillColor();
	}

	if ( bBackground )
	{
		Color aColor;
		if ( IsControlBackground() )
			aColor = GetControlBackground();
		else if ( GetStyle() & WB_3DLOOK )
			aColor = rStyleSettings.GetFaceColor();
		else
			aColor = rStyleSettings.GetWindowColor();
        SetBackground( aColor );
        mpImplData->mpVirDev->SetBackground( GetBackground() );
        
        // NWF background
        if( ! IsControlBackground() &&
              IsNativeControlSupported( CTRL_WINDOW_BACKGROUND, PART_BACKGROUND_WINDOW ) )
        {
            ImplGetWindowImpl()->mnNativeBackground = PART_BACKGROUND_WINDOW;
            EnableChildTransparentMode( sal_True );
        }
	}
}

// -----------------------------------------------------------------------

void StatusBar::ImplFormat()
{
	ImplStatusItem* pItem;
	long			nExtraWidth;
	long			nExtraWidth2;
	long			nX;
	sal_uInt16			nAutoSizeItems = 0;

	// Breiten zusammenrechnen
	mnItemsWidth = STATUSBAR_OFFSET_X;
	long nOffset = 0;
	pItem = mpItemList->First();
	while ( pItem )
	{
		if ( pItem->mbVisible )
		{
			if ( pItem->mnBits & SIB_AUTOSIZE )
				nAutoSizeItems++;

			mnItemsWidth += pItem->mnWidth + nOffset;
			nOffset = pItem->mnOffset;
		}

		pItem = mpItemList->Next();
	}

	if ( GetStyle() & WB_RIGHT )
	{
		// Bei rechtsbuendiger Ausrichtung wird kein AutoSize ausgewertet,
		// da wir links den Text anzeigen, der mit SetText gesetzt wird
		nX				= mnDX - mnItemsWidth;
		nExtraWidth 	= 0;
		nExtraWidth2	= 0;
	}
	else
	{
		mnItemsWidth += STATUSBAR_OFFSET_X;

		// Bei linksbuendiger Ausrichtung muessen wir gegebenenfalls noch
		// AutoSize auswerten
		if ( nAutoSizeItems && (mnDX > (mnItemsWidth - STATUSBAR_OFFSET)) )
		{
			nExtraWidth  = (mnDX - mnItemsWidth - 1) / nAutoSizeItems;
			nExtraWidth2 = (mnDX - mnItemsWidth - 1) % nAutoSizeItems;
		}
		else
		{
			nExtraWidth  = 0;
			nExtraWidth2 = 0;
		}
		nX = STATUSBAR_OFFSET_X;
		if( ImplHasMirroredGraphics() && IsRTLEnabled() )
		    nX += ImplGetSVData()->maNWFData.mnStatusBarLowerRightOffset;
	}

	pItem = mpItemList->First();
	while ( pItem )
	{
		if ( pItem->mbVisible )
		{
			if ( pItem->mnBits & SIB_AUTOSIZE )
			{
				pItem->mnExtraWidth = nExtraWidth;
				if ( nExtraWidth2 )
				{
					pItem->mnExtraWidth++;
					nExtraWidth2--;
				}
			}
			else
				pItem->mnExtraWidth = 0;

			pItem->mnX = nX;
			nX += pItem->mnWidth + pItem->mnExtraWidth + pItem->mnOffset;
		}

		pItem = mpItemList->Next();
	}

	mbFormat = sal_False;
}

// -----------------------------------------------------------------------

Rectangle StatusBar::ImplGetItemRectPos( sal_uInt16 nPos ) const
{
    Rectangle		aRect;
    ImplStatusItem* pItem;
    pItem = mpItemList->GetObject( nPos );
    if ( pItem )
    {
        if ( pItem->mbVisible )
        {
            aRect.Left()   = pItem->mnX;
            aRect.Right()  = aRect.Left() + pItem->mnWidth + pItem->mnExtraWidth;
            aRect.Top()    = mnItemY;
            aRect.Bottom() = mnCalcHeight - STATUSBAR_OFFSET_Y;
            if( IsTopBorder() )
                aRect.Bottom()+=2;
        }
    }

    return aRect;
}

// -----------------------------------------------------------------------

sal_uInt16 StatusBar::ImplGetFirstVisiblePos() const
{
    ImplStatusItem* pItem;
    
    for( sal_uInt16 nPos = 0; nPos < mpItemList->Count(); nPos++ )
    {
        pItem = mpItemList->GetObject( nPos );
        if ( pItem )
        {
            if ( pItem->mbVisible )
                return nPos;
        }
    }
    
    return ~0;
}

// -----------------------------------------------------------------------

void StatusBar::ImplDrawText( sal_Bool bOffScreen, long nOldTextWidth )
{
	// Das ueberschreiben der Item-Box verhindern
	Rectangle aTextRect;
	aTextRect.Left() = STATUSBAR_OFFSET_X+1;
	aTextRect.Top()  = mnTextY;
	if ( mbVisibleItems && (GetStyle() & WB_RIGHT) )
		aTextRect.Right() = mnDX - mnItemsWidth - 1;
	else
		aTextRect.Right() = mnDX - 1;
	if ( aTextRect.Right() > aTextRect.Left() )
	{
		// Position ermitteln
		XubString aStr = GetText();
		sal_uInt16 nPos = aStr.Search( _LF );
		if ( nPos != STRING_NOTFOUND )
			aStr.Erase( nPos );

		aTextRect.Bottom() = aTextRect.Top()+GetTextHeight()+1;

		if ( bOffScreen )
		{
			long nMaxWidth = Max( nOldTextWidth, GetTextWidth( aStr ) );
			Size aVirDevSize( nMaxWidth, aTextRect.GetHeight() );
			mpImplData->mpVirDev->SetOutputSizePixel( aVirDevSize );
			Rectangle aTempRect = aTextRect;
			aTempRect.SetPos( Point( 0, 0 ) );
			mpImplData->mpVirDev->DrawText( aTempRect, aStr, TEXT_DRAW_LEFT | TEXT_DRAW_TOP | TEXT_DRAW_CLIP | TEXT_DRAW_ENDELLIPSIS );
			DrawOutDev( aTextRect.TopLeft(), aVirDevSize, Point(), aVirDevSize, *mpImplData->mpVirDev );
		}
		else
			DrawText( aTextRect, aStr, TEXT_DRAW_LEFT | TEXT_DRAW_TOP | TEXT_DRAW_CLIP | TEXT_DRAW_ENDELLIPSIS );
	}
}

// -----------------------------------------------------------------------

void StatusBar::ImplDrawItem( sal_Bool bOffScreen, sal_uInt16 nPos, sal_Bool bDrawText, sal_Bool bDrawFrame )
{
	Rectangle aRect = ImplGetItemRectPos( nPos );

	if ( aRect.IsEmpty() )
		return;

	// Ausgabebereich berechnen
	ImplStatusItem* 	pItem = mpItemList->GetObject( nPos );
    long nW = mpImplData->mnItemBorderWidth + 1;
	Rectangle			aTextRect( aRect.Left()+nW, aRect.Top()+nW,
								   aRect.Right()-nW, aRect.Bottom()-nW );
	Size				aTextRectSize( aTextRect.GetSize() );

	if ( bOffScreen )
		mpImplData->mpVirDev->SetOutputSizePixel( aTextRectSize );
	else
	{
		Region aRegion( aTextRect );
		SetClipRegion( aRegion );
	}

	// Text ausgeben
	if ( bDrawText )
	{
		Size	aTextSize( GetTextWidth( pItem->maText ), GetTextHeight() );
		Point	aTextPos = ImplGetItemTextPos( aTextRectSize, aTextSize, pItem->mnBits );
		if ( bOffScreen )
			mpImplData->mpVirDev->DrawText( aTextPos, pItem->maText );
		else
		{
			aTextPos.X() += aTextRect.Left();
			aTextPos.Y() += aTextRect.Top();
			DrawText( aTextPos, pItem->maText );
		}
	}

	// Gegebenenfalls auch DrawItem aufrufen
	if ( pItem->mnBits & SIB_USERDRAW )
	{
		if ( bOffScreen )
		{
			mbInUserDraw = sal_True;
            mpImplData->mpVirDev->EnableRTL( IsRTLEnabled() );
			UserDrawEvent aODEvt( mpImplData->mpVirDev, Rectangle( Point(), aTextRectSize ), pItem->mnId );
			UserDraw( aODEvt );
            mpImplData->mpVirDev->EnableRTL( sal_False );
			mbInUserDraw = sal_False;
		}
		else
		{
			UserDrawEvent aODEvt( this, aTextRect, pItem->mnId );
			UserDraw( aODEvt );
		}
	}

	if ( bOffScreen )
		DrawOutDev( aTextRect.TopLeft(), aTextRectSize, Point(), aTextRectSize, *mpImplData->mpVirDev );
	else
		SetClipRegion();

	// Frame ausgeben
	if ( bDrawFrame )
    {
        if( mpImplData->mbDrawItemFrames )
        {
            if( !(pItem->mnBits & SIB_FLAT) )
            {
                sal_uInt16 nStyle;
                
                if ( pItem->mnBits & SIB_IN )
                    nStyle = FRAME_DRAW_IN;
                else
                    nStyle = FRAME_DRAW_OUT;
                
                DecorationView aDecoView( this );
                aDecoView.DrawFrame( aRect, nStyle );
            }
        }
        else if( nPos != ImplGetFirstVisiblePos() )
        {
            // draw separator
            Point aFrom( aRect.TopLeft() );
            aFrom.X()--;
            aFrom.Y()++;
            Point aTo( aRect.BottomLeft() );
            aTo.X()--;
            aTo.Y()--;

            DecorationView aDecoView( this );
            aDecoView.DrawSeparator( aFrom, aTo );
        }
    }

	if ( !ImplIsRecordLayout() )
		ImplCallEventListeners( VCLEVENT_STATUSBAR_DRAWITEM, (void*) sal_IntPtr(pItem->mnId) );
}

// -----------------------------------------------------------------------

void DrawProgress( Window* pWindow, const Point& rPos,
				   long nOffset, long nPrgsWidth, long nPrgsHeight,
				   sal_uInt16 nPercent1, sal_uInt16 nPercent2, sal_uInt16 nPercentCount,
                   const Rectangle& rFramePosSize
                   )
{
    if( pWindow->IsNativeControlSupported( CTRL_PROGRESS, PART_ENTIRE_CONTROL ) )
    {
        bool bNeedErase = ImplGetSVData()->maNWFData.mbProgressNeedsErase;
        
        long nFullWidth = (nPrgsWidth + nOffset) * (10000 / nPercentCount);
        long nPerc = (nPercent2 > 10000) ? 10000 : nPercent2;
        ImplControlValue aValue( nFullWidth * (long)nPerc / 10000 );
        Rectangle aDrawRect( rPos, Size( nFullWidth, nPrgsHeight ) );
        Rectangle aControlRegion( aDrawRect );
        if( bNeedErase )
        {
            Window* pEraseWindow = pWindow;
            while( pEraseWindow->IsPaintTransparent()                         &&
                   ! pEraseWindow->ImplGetWindowImpl()->mbFrame )
            {
                pEraseWindow = pEraseWindow->ImplGetWindowImpl()->mpParent;
            }
            if( pEraseWindow == pWindow )
                // restore background of pWindow
                pEraseWindow->Erase( rFramePosSize );
            else
            {
                // restore transparent background
                Point aTL( pWindow->OutputToAbsoluteScreenPixel( rFramePosSize.TopLeft() ) );
                aTL = pEraseWindow->AbsoluteScreenToOutputPixel( aTL );
                Rectangle aRect( aTL, rFramePosSize.GetSize() );
                pEraseWindow->Invalidate( aRect, INVALIDATE_NOCHILDREN     |
                                                 INVALIDATE_NOCLIPCHILDREN |
                                                 INVALIDATE_TRANSPARENT );
                pEraseWindow->Update();
            }
            pWindow->Push( PUSH_CLIPREGION );
            pWindow->IntersectClipRegion( rFramePosSize );
        }
        sal_Bool bNativeOK = pWindow->DrawNativeControl( CTRL_PROGRESS, PART_ENTIRE_CONTROL, aControlRegion,
                                                     CTRL_STATE_ENABLED, aValue, rtl::OUString() );
        if( bNeedErase )
            pWindow->Pop();
        if( bNativeOK )
        {
            pWindow->Flush();
            return;
        }
    }
    
	// Werte vorberechnen
	sal_uInt16 nPerc1 = nPercent1 / nPercentCount;
	sal_uInt16 nPerc2 = nPercent2 / nPercentCount;

	if ( nPerc1 > nPerc2 )
	{
		// Support progress that can also decrease
		
		// Rechteck berechnen
		long		nDX = nPrgsWidth + nOffset;
		long		nLeft = rPos.X()+((nPerc1-1)*nDX);
		Rectangle	aRect( nLeft, rPos.Y(), nLeft+nPrgsWidth, rPos.Y()+nPrgsHeight );

		do
		{
			pWindow->Erase( aRect );
			aRect.Left()  -= nDX;
			aRect.Right() -= nDX;
			nPerc1--;
		}
		while ( nPerc1 > nPerc2 );

		pWindow->Flush();
	}	
	else if ( nPerc1 < nPerc2 )
	{
		// Percent-Rechtecke malen
		// Wenn Percent2 ueber 100%, Werte anpassen
		if ( nPercent2 > 10000 )
		{
			nPerc2 = 10000 / nPercentCount;
			if ( nPerc1 >= nPerc2 )
				nPerc1 = nPerc2-1;
		}

		// Rechteck berechnen
		long		nDX = nPrgsWidth + nOffset;
		long		nLeft = rPos.X()+(nPerc1*nDX);
		Rectangle	aRect( nLeft, rPos.Y(), nLeft+nPrgsWidth, rPos.Y()+nPrgsHeight );

		do
		{
			pWindow->DrawRect( aRect );
			aRect.Left()  += nDX;
			aRect.Right() += nDX;
			nPerc1++;
		}
		while ( nPerc1 < nPerc2 );

		// Bei mehr als 100%, lassen wir das Rechteck blinken
		if ( nPercent2 > 10000 )
		{
			// an/aus-Status festlegen
			if ( ((nPercent2 / nPercentCount) & 0x01) == (nPercentCount & 0x01) )
			{
				aRect.Left()  -= nDX;
				aRect.Right() -= nDX;
				pWindow->Erase( aRect );
			}
		}

		pWindow->Flush();
	}
}

// -----------------------------------------------------------------------

void StatusBar::ImplDrawProgress( sal_Bool bPaint,
								  sal_uInt16 nPercent1, sal_uInt16 nPercent2 )
{
    bool bNative = IsNativeControlSupported( CTRL_PROGRESS, PART_ENTIRE_CONTROL );
	// bPaint: draw text also, else only update progress
	if ( bPaint )
	{
		DrawText( maPrgsTxtPos, maPrgsTxt );
        if( ! bNative )
        {
            DecorationView aDecoView( this );
            aDecoView.DrawFrame( maPrgsFrameRect, FRAME_DRAW_IN );
        }
	}

	Point aPos( maPrgsFrameRect.Left()+STATUSBAR_PRGS_OFFSET,
				maPrgsFrameRect.Top()+STATUSBAR_PRGS_OFFSET );
    long nPrgsHeight = mnPrgsSize;
    if( bNative )
    {
        aPos = maPrgsFrameRect.TopLeft();
        nPrgsHeight = maPrgsFrameRect.GetHeight();
    }
	DrawProgress( this, aPos, mnPrgsSize/2, mnPrgsSize, nPrgsHeight,
				  nPercent1*100, nPercent2*100, mnPercentCount, maPrgsFrameRect );
}

// -----------------------------------------------------------------------

void StatusBar::ImplCalcProgressRect()
{
	// calculate text size
	Size aPrgsTxtSize( GetTextWidth( maPrgsTxt ), GetTextHeight() );
	maPrgsTxtPos.X()	= STATUSBAR_OFFSET_X+1;
    
	// calculate progress frame
	maPrgsFrameRect.Left()		= maPrgsTxtPos.X()+aPrgsTxtSize.Width()+STATUSBAR_OFFSET;
	maPrgsFrameRect.Top()		= mnItemY;
	maPrgsFrameRect.Bottom()	= mnCalcHeight - STATUSBAR_OFFSET_Y;
    if( IsTopBorder() )
        maPrgsFrameRect.Bottom()+=2;

	// calculate size of progress rects
	mnPrgsSize = maPrgsFrameRect.Bottom()-maPrgsFrameRect.Top()-(STATUSBAR_PRGS_OFFSET*2);
	sal_uInt16 nMaxPercent = STATUSBAR_PRGS_COUNT;

	long nMaxWidth = mnDX-STATUSBAR_OFFSET-1;

	// make smaller if there are too many rects
	while ( maPrgsFrameRect.Left()+ImplCalcProgessWidth( nMaxPercent, mnPrgsSize ) > nMaxWidth )
	{
		nMaxPercent--;
		if ( nMaxPercent <= STATUSBAR_PRGS_MIN )
			break;
	}
	maPrgsFrameRect.Right() = maPrgsFrameRect.Left() + ImplCalcProgessWidth( nMaxPercent, mnPrgsSize );

	// save the divisor for later
	mnPercentCount = 10000 / nMaxPercent;
    sal_Bool bNativeOK = sal_False;
    if( IsNativeControlSupported( CTRL_PROGRESS, PART_ENTIRE_CONTROL ) )
    {
        ImplControlValue aValue;
        Rectangle aControlRegion( Rectangle( (const Point&)Point(), maPrgsFrameRect.GetSize() ) );
        Rectangle aNativeControlRegion, aNativeContentRegion;
        if( (bNativeOK = GetNativeControlRegion( CTRL_PROGRESS, PART_ENTIRE_CONTROL, aControlRegion,
                                                 CTRL_STATE_ENABLED, aValue, rtl::OUString(),
                                                 aNativeControlRegion, aNativeContentRegion ) ) != sal_False )
        {
            long nProgressHeight = aNativeControlRegion.GetHeight();
            if( nProgressHeight > maPrgsFrameRect.GetHeight() )
            {
                long nDelta = nProgressHeight - maPrgsFrameRect.GetHeight();
                maPrgsFrameRect.Top() -= (nDelta - nDelta/2);
                maPrgsFrameRect.Bottom() += nDelta/2;
            }
            maPrgsTxtPos.Y() = maPrgsFrameRect.Top() + (nProgressHeight - GetTextHeight())/2;
        }
    }
    if( ! bNativeOK )
        maPrgsTxtPos.Y()	= mnTextY;
}

// -----------------------------------------------------------------------

void StatusBar::MouseButtonDown( const MouseEvent& rMEvt )
{
	// Nur bei linker Maustaste ToolBox ausloesen
	if ( rMEvt.IsLeft() )
	{
		if ( mbVisibleItems )
		{
			Point  aMousePos = rMEvt.GetPosPixel();
			sal_uInt16 i = 0;

			// Item suchen, das geklickt wurde
			ImplStatusItem* pItem = mpItemList->First();
			while ( pItem )
			{
				// Ist es dieses Item
				if ( ImplGetItemRectPos( i ).IsInside( aMousePos ) )
				{
					mnCurItemId = pItem->mnId;
					if ( rMEvt.GetClicks() == 2 )
						DoubleClick();
					else
						Click();
					mnCurItemId = 0;

					// Item wurde gefunden
					return;
				}

				i++;
				pItem = mpItemList->Next();
			}
		}

		// Kein Item, dann nur Click oder DoubleClick
		if ( rMEvt.GetClicks() == 2 )
			DoubleClick();
		else
			Click();
	}
}

// -----------------------------------------------------------------------

void StatusBar::Paint( const Rectangle& )
{
	if ( mbFormat )
		ImplFormat();

	sal_uInt16 nItemCount = (sal_uInt16)mpItemList->Count();

	if ( mbProgressMode )
		ImplDrawProgress( sal_True, 0, mnPercent );
	else
	{
		// Text zeichen
		if ( !mbVisibleItems || (GetStyle() & WB_RIGHT) )
			ImplDrawText( sal_False, 0 );

		// Items zeichnen
		if ( mbVisibleItems )
		{
			// Items zeichnen
			for ( sal_uInt16 i = 0; i < nItemCount; i++ )
				ImplDrawItem( sal_False, i, sal_True, sal_True );
		}
	}

    // draw borders
    if( IsTopBorder() )
    {
	    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
	    SetLineColor( rStyleSettings.GetShadowColor() );
	    DrawLine( Point( 0, 0 ), Point( mnDX-1, 0 ) );
	    SetLineColor( rStyleSettings.GetLightColor() );
	    DrawLine( Point( 0, 1 ), Point( mnDX-1, 1 ) );
    }

	if ( IsBottomBorder() )
	{
		const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
		SetLineColor( rStyleSettings.GetShadowColor() );
		DrawLine( Point( 0, mnDY-2 ), Point( mnDX-1, mnDY-2 ) );
		SetLineColor( rStyleSettings.GetLightColor() );
		DrawLine( Point( 0, mnDY-1 ), Point( mnDX-1, mnDY-1 ) );
	}
}

// -----------------------------------------------------------------------

void StatusBar::Move()
{
	Window::Move();
}

// -----------------------------------------------------------------------

void StatusBar::Resize()
{
	// Breite und Hoehe abfragen und merken
	Size aSize = GetOutputSizePixel();
	mnDX = aSize.Width() - ImplGetSVData()->maNWFData.mnStatusBarLowerRightOffset;
	mnDY = aSize.Height();
	mnCalcHeight = mnDY;
    // subtract border
    if( IsTopBorder() )
        mnCalcHeight -= 2;
	if ( IsBottomBorder() )
		mnCalcHeight -= 2;

    mnItemY = STATUSBAR_OFFSET_Y;
    if( IsTopBorder() )
        mnItemY += 2;
    mnTextY = (mnCalcHeight-GetTextHeight())/2;
    if( IsTopBorder() )
        mnTextY += 2;

	// Formatierung neu ausloesen
	mbFormat = sal_True;

	if ( mbProgressMode )
		ImplCalcProgressRect();

	Invalidate();
}

// -----------------------------------------------------------------------

void StatusBar::RequestHelp( const HelpEvent& rHEvt )
{
	// no keyboard help in status bar
	if( rHEvt.KeyboardActivated() )
		return;

	sal_uInt16 nItemId = GetItemId( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );

	if ( nItemId )
	{
		Rectangle aItemRect = GetItemRect( nItemId );
		Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
		aItemRect.Left()   = aPt.X();
		aItemRect.Top()    = aPt.Y();
		aPt = OutputToScreenPixel( aItemRect.BottomRight() );
		aItemRect.Right()  = aPt.X();
		aItemRect.Bottom() = aPt.Y();

		if ( rHEvt.GetMode() & HELPMODE_BALLOON )
		{
			XubString aStr = GetHelpText( nItemId );
			Help::ShowBalloon( this, aItemRect.Center(), aItemRect, aStr );
			return;
		}
		else if ( rHEvt.GetMode() & HELPMODE_QUICK )
		{
			XubString	aStr = GetQuickHelpText( nItemId );
            // Show quickhelp if available
            if( aStr.Len() )
            {
			    Help::ShowQuickHelp( this, aItemRect, aStr );
                return;
            }
			aStr = GetItemText( nItemId );
			// show a quick help if item text doesn't fit
			if ( GetTextWidth( aStr ) > aItemRect.GetWidth() )
			{
				Help::ShowQuickHelp( this, aItemRect, aStr );
				return;
			}
		}
		else if ( rHEvt.GetMode() & HELPMODE_EXTENDED )
		{
            String aCommand = GetItemCommand( nItemId );
            rtl::OString aHelpId( GetHelpId( nItemId ) );

            if ( aCommand.Len() || aHelpId.getLength() )
            {
                // Wenn eine Hilfe existiert, dann ausloesen
                Help* pHelp = Application::GetHelp();
                if ( pHelp )
                {
                    if ( aCommand.Len() )
                        pHelp->Start( aCommand, this );
                    else if ( aHelpId.getLength() )
                        pHelp->Start( rtl::OStringToOUString( aHelpId, RTL_TEXTENCODING_UTF8 ), this );
                }
                return;
            }
		}
	}

	Window::RequestHelp( rHEvt );
}

// -----------------------------------------------------------------------

void StatusBar::StateChanged( StateChangedType nType )
{
	Window::StateChanged( nType );

	if ( nType == STATE_CHANGE_INITSHOW )
		ImplFormat();
	else if ( nType == STATE_CHANGE_UPDATEMODE )
		Invalidate();
	else if ( (nType == STATE_CHANGE_ZOOM) ||
			  (nType == STATE_CHANGE_CONTROLFONT) )
	{
		mbFormat = sal_True;
		ImplInitSettings( sal_True, sal_False, sal_False );
		Invalidate();
	}
	else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
	{
		ImplInitSettings( sal_False, sal_True, sal_False );
		Invalidate();
	}
	else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
	{
		ImplInitSettings( sal_False, sal_False, sal_True );
		Invalidate();
	}
}

// -----------------------------------------------------------------------

void StatusBar::DataChanged( const DataChangedEvent& rDCEvt )
{
	Window::DataChanged( rDCEvt );

	if ( (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
		 (rDCEvt.GetType() == DATACHANGED_FONTS) ||
		 (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
		 ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
		  (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
	{
		mbFormat = sal_True;
		ImplInitSettings( sal_True, sal_True, sal_True );
        ImplStatusItem* pItem = mpItemList->First();
        long nFudge = GetTextHeight() / 4;
        while ( pItem )
        {
            long nWidth = GetTextWidth( pItem->maText ) + nFudge;
            if( nWidth > pItem->mnWidth + STATUSBAR_OFFSET )
                pItem->mnWidth = nWidth + STATUSBAR_OFFSET;
            pItem = mpItemList->Next();
        }
        Size aSize = GetSizePixel();
        // do not disturb current width, since
        // CalcWindowSizePixel calculates a minimum width
        aSize.Height() = CalcWindowSizePixel().Height();
        SetSizePixel( aSize );
		Invalidate();
	}
}

// -----------------------------------------------------------------------

void StatusBar::Click()
{
    ImplCallEventListeners( VCLEVENT_STATUSBAR_CLICK );
	maClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void StatusBar::DoubleClick()
{
    ImplCallEventListeners( VCLEVENT_STATUSBAR_DOUBLECLICK );
	maDoubleClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void StatusBar::UserDraw( const UserDrawEvent& )
{
}

// -----------------------------------------------------------------------

void StatusBar::InsertItem( sal_uInt16 nItemId, sal_uLong nWidth,
							StatusBarItemBits nBits,
							long nOffset, sal_uInt16 nPos )
{
	DBG_ASSERT( nItemId, "StatusBar::InsertItem(): ItemId == 0" );
	DBG_ASSERT( GetItemPos( nItemId ) == STATUSBAR_ITEM_NOTFOUND,
				"StatusBar::InsertItem(): ItemId already exists" );

	// IN und CENTER sind Default
	if ( !(nBits & (SIB_IN | SIB_OUT | SIB_FLAT)) )
		nBits |= SIB_IN;
	if ( !(nBits & (SIB_LEFT | SIB_RIGHT | SIB_CENTER)) )
		nBits |= SIB_CENTER;

	// Item anlegen
    long nFudge = GetTextHeight()/4;
	ImplStatusItem* pItem	= new ImplStatusItem;
	pItem->mnId 			= nItemId;
	pItem->mnBits			= nBits;
	pItem->mnWidth			= (long)nWidth+nFudge+STATUSBAR_OFFSET;
	pItem->mnOffset 		= nOffset;
	pItem->mpUserData		= 0;
	pItem->mbVisible		= sal_True;

	// Item in die Liste einfuegen
	mpItemList->Insert( pItem, nPos );

	mbFormat = sal_True;
	if ( ImplIsItemUpdate() )
		Invalidate();

	ImplCallEventListeners( VCLEVENT_STATUSBAR_ITEMADDED, (void*) sal_IntPtr(nItemId) );
}

// -----------------------------------------------------------------------

void StatusBar::RemoveItem( sal_uInt16 nItemId )
{
	sal_uInt16 nPos = GetItemPos( nItemId );
	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
	{
		ImplStatusItem* pItem = mpItemList->Remove( nPos );
		delete pItem;

		mbFormat = sal_True;
		if ( ImplIsItemUpdate() )
			Invalidate();

		ImplCallEventListeners( VCLEVENT_STATUSBAR_ITEMREMOVED, (void*) sal_IntPtr(nItemId) );
	}
}

// -----------------------------------------------------------------------

void StatusBar::ShowItem( sal_uInt16 nItemId )
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
	{
		ImplStatusItem* pItem = mpItemList->GetObject( nPos );
		if ( !pItem->mbVisible )
		{
			pItem->mbVisible = sal_True;

			mbFormat = sal_True;
			if ( ImplIsItemUpdate() )
				Invalidate();

			ImplCallEventListeners( VCLEVENT_STATUSBAR_SHOWITEM, (void*) sal_IntPtr(nItemId) );
		}
	}
}

// -----------------------------------------------------------------------

void StatusBar::HideItem( sal_uInt16 nItemId )
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
	{
		ImplStatusItem* pItem = mpItemList->GetObject( nPos );
		if ( pItem->mbVisible )
		{
			pItem->mbVisible = sal_False;

			mbFormat = sal_True;
			if ( ImplIsItemUpdate() )
				Invalidate();

			ImplCallEventListeners( VCLEVENT_STATUSBAR_HIDEITEM, (void*) sal_IntPtr(nItemId) );
		}
	}
}

// -----------------------------------------------------------------------

sal_Bool StatusBar::IsItemVisible( sal_uInt16 nItemId ) const
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
		return mpItemList->GetObject( nPos )->mbVisible;
	else
		return sal_False;
}

// -----------------------------------------------------------------------

void StatusBar::ShowItems()
{
	if ( !mbVisibleItems )
	{
		mbVisibleItems = sal_True;
		if ( !mbProgressMode )
			Invalidate();

		ImplCallEventListeners( VCLEVENT_STATUSBAR_SHOWALLITEMS );
	}
}

// -----------------------------------------------------------------------

void StatusBar::HideItems()
{
	if ( mbVisibleItems )
	{
		mbVisibleItems = sal_False;
		if ( !mbProgressMode )
			Invalidate();

		ImplCallEventListeners( VCLEVENT_STATUSBAR_HIDEALLITEMS );
	}
}

// -----------------------------------------------------------------------

void StatusBar::CopyItems( const StatusBar& rStatusBar )
{
	// Alle Items entfernen
	ImplStatusItem* pItem = mpItemList->First();
	while ( pItem )
	{
		delete pItem;
		pItem = mpItemList->Next();
	}

	// Items aus der Liste loeschen
	mpItemList->Clear();

	// Items kopieren
	sal_uLong i = 0;
	pItem = rStatusBar.mpItemList->GetObject( i );
	while ( pItem )
	{
		mpItemList->Insert( new ImplStatusItem( *pItem ), LIST_APPEND );
		i++;
		pItem = rStatusBar.mpItemList->GetObject( i );
	}

	mbFormat = sal_True;
	if ( ImplIsItemUpdate() )
		Invalidate();
}

// -----------------------------------------------------------------------

void StatusBar::Clear()
{
	// Alle Item loeschen
	ImplStatusItem* pItem = mpItemList->First();
	while ( pItem )
	{
		delete pItem;
		pItem = mpItemList->Next();
	}

	// Items aus der Liste loeschen
	mpItemList->Clear();

	mbFormat = sal_True;
	if ( ImplIsItemUpdate() )
		Invalidate();

	ImplCallEventListeners( VCLEVENT_STATUSBAR_ALLITEMSREMOVED );
}

// -----------------------------------------------------------------------

sal_uInt16 StatusBar::GetItemCount() const
{
	return (sal_uInt16)mpItemList->Count();
}

// -----------------------------------------------------------------------

sal_uInt16 StatusBar::GetItemId( sal_uInt16 nPos ) const
{
	ImplStatusItem* pItem = mpItemList->GetObject( nPos );
	if ( pItem )
		return pItem->mnId;
	else
		return 0;
}

// -----------------------------------------------------------------------

sal_uInt16 StatusBar::GetItemPos( sal_uInt16 nItemId ) const
{
	ImplStatusItem* pItem = mpItemList->First();
	while ( pItem )
	{
		if ( pItem->mnId == nItemId )
			return (sal_uInt16)mpItemList->GetCurPos();

		pItem = mpItemList->Next();
	}

	return STATUSBAR_ITEM_NOTFOUND;
}

// -----------------------------------------------------------------------

sal_uInt16 StatusBar::GetItemId( const Point& rPos ) const
{
	if ( AreItemsVisible() && !mbFormat )
	{
		sal_uInt16 nItemCount = GetItemCount();
		sal_uInt16 nPos;
		for ( nPos = 0; nPos < nItemCount; nPos++ )
		{
			// Rechteck holen
			Rectangle aRect = ImplGetItemRectPos( nPos );
			if ( aRect.IsInside( rPos ) )
				return mpItemList->GetObject( nPos )->mnId;
		}
	}

	return 0;
}

// -----------------------------------------------------------------------

Rectangle StatusBar::GetItemRect( sal_uInt16 nItemId ) const
{
	Rectangle aRect;

	if ( AreItemsVisible() && !mbFormat )
	{
		sal_uInt16 nPos = GetItemPos( nItemId );
		if ( nPos != STATUSBAR_ITEM_NOTFOUND )
		{
			// Rechteck holen und Rahmen abziehen
			aRect = ImplGetItemRectPos( nPos );
            long nW = mpImplData->mnItemBorderWidth+1;
            aRect.Top() += nW-1;
            aRect.Bottom() -= nW-1;
			aRect.Left() += nW;
			aRect.Right() -= nW;
			return aRect;
		}
	}

	return aRect;
}

// -----------------------------------------------------------------------

Point StatusBar::GetItemTextPos( sal_uInt16 nItemId ) const
{
	if ( !mbFormat )
	{
		sal_uInt16 nPos = GetItemPos( nItemId );
		if ( nPos != STATUSBAR_ITEM_NOTFOUND )
		{
			// Rechteck holen
			ImplStatusItem* pItem = mpItemList->GetObject( nPos );
			Rectangle aRect = ImplGetItemRectPos( nPos );
            long nW = mpImplData->mnItemBorderWidth + 1;
            Rectangle			aTextRect( aRect.Left()+nW, aRect.Top()+nW,
                                           aRect.Right()-nW, aRect.Bottom()-nW );
			Point aPos = ImplGetItemTextPos( aTextRect.GetSize(),
											 Size( GetTextWidth( pItem->maText ), GetTextHeight() ),
											 pItem->mnBits );
			if ( !mbInUserDraw )
			{
				aPos.X() += aTextRect.Left();
				aPos.Y() += aTextRect.Top();
			}
			return aPos;
		}
	}

	return Point();
}

// -----------------------------------------------------------------------

sal_uLong StatusBar::GetItemWidth( sal_uInt16 nItemId ) const
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
		return mpItemList->GetObject( nPos )->mnWidth;
	else
		return 0;
}

// -----------------------------------------------------------------------

StatusBarItemBits StatusBar::GetItemBits( sal_uInt16 nItemId ) const
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
		return mpItemList->GetObject( nPos )->mnBits;
	else
		return 0;
}

// -----------------------------------------------------------------------

long StatusBar::GetItemOffset( sal_uInt16 nItemId ) const
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
		return mpItemList->GetObject( nPos )->mnOffset;
	else
		return 0;
}

// -----------------------------------------------------------------------

void StatusBar::SetItemText( sal_uInt16 nItemId, const XubString& rText )
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
	{
		ImplStatusItem* pItem = mpItemList->GetObject( nPos );

		if ( pItem->maText != rText )
		{
			pItem->maText = rText;

            // adjust item width - see also DataChanged()
            long nFudge = GetTextHeight()/4;
            long nWidth = GetTextWidth( pItem->maText ) + nFudge;
            if( (nWidth > pItem->mnWidth + STATUSBAR_OFFSET) || 
                ((nWidth < pItem->mnWidth) && (mnDX - STATUSBAR_OFFSET) < mnItemsWidth  ))
            {
                pItem->mnWidth = nWidth + STATUSBAR_OFFSET;
                ImplFormat();
                Invalidate();
            }

			// Item neu Zeichen, wenn StatusBar sichtbar und
			// UpdateMode gesetzt ist
			if ( pItem->mbVisible && !mbFormat && ImplIsItemUpdate() )
			{
				Update();
				ImplDrawItem( sal_True, nPos, sal_True, sal_False );
				Flush();
			}
		}
	}
}

// -----------------------------------------------------------------------

const XubString& StatusBar::GetItemText( sal_uInt16 nItemId ) const
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
		return mpItemList->GetObject( nPos )->maText;
	else
		return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void StatusBar::SetItemCommand( sal_uInt16 nItemId, const XubString& rCommand )
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
	{
		ImplStatusItem* pItem = mpItemList->GetObject( nPos );

		if ( pItem->maCommand != rCommand )
			pItem->maCommand = rCommand;
	}
}

// -----------------------------------------------------------------------

const XubString& StatusBar::GetItemCommand( sal_uInt16 nItemId )
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
		return mpItemList->GetObject( nPos )->maCommand;
	else
		return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void StatusBar::SetItemData( sal_uInt16 nItemId, void* pNewData )
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
	{
		ImplStatusItem* pItem = mpItemList->GetObject( nPos );
		pItem->mpUserData = pNewData;

		// Wenn es ein User-Item ist, DrawItem-Aufrufen
		if ( (pItem->mnBits & SIB_USERDRAW) && pItem->mbVisible &&
			 !mbFormat && ImplIsItemUpdate() )
		{
			Update();
			ImplDrawItem( sal_True, nPos, sal_False, sal_False );
			Flush();
		}
	}
}

// -----------------------------------------------------------------------

void* StatusBar::GetItemData( sal_uInt16 nItemId ) const
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
		return mpItemList->GetObject( nPos )->mpUserData;
	else
		return NULL;
}

// -----------------------------------------------------------------------

void StatusBar::SetHelpText( sal_uInt16 nItemId, const XubString& rText )
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
		mpItemList->GetObject( nPos )->maHelpText = rText;
}

// -----------------------------------------------------------------------

const XubString& StatusBar::GetHelpText( sal_uInt16 nItemId ) const
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
	{
		ImplStatusItem* pItem = mpItemList->GetObject( nPos );
		if ( !pItem->maHelpText.Len() && ( pItem->maHelpId.getLength() || pItem->maCommand.Len() ))
		{
			Help* pHelp = Application::GetHelp();
			if ( pHelp )
            {
				if ( pItem->maCommand.Len() )
                    pItem->maHelpText = pHelp->GetHelpText( pItem->maCommand, this );
                if ( !pItem->maHelpText.Len() && pItem->maHelpId.getLength() )
                    pItem->maHelpText = pHelp->GetHelpText( rtl::OStringToOUString( pItem->maHelpId, RTL_TEXTENCODING_UTF8 ), this );
            }
		}

		return pItem->maHelpText;
	}
	else
		return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void StatusBar::SetQuickHelpText( sal_uInt16 nItemId, const XubString& rText )
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
		mpItemList->GetObject( nPos )->maQuickHelpText = rText;
}

// -----------------------------------------------------------------------

const XubString& StatusBar::GetQuickHelpText( sal_uInt16 nItemId ) const
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
	{
		ImplStatusItem* pItem = mpItemList->GetObject( nPos );
		return pItem->maQuickHelpText;
	}
	else
		return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void StatusBar::SetHelpId( sal_uInt16 nItemId, const rtl::OString& rHelpId )
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
		mpItemList->GetObject( nPos )->maHelpId = rHelpId;
}

// -----------------------------------------------------------------------

rtl::OString StatusBar::GetHelpId( sal_uInt16 nItemId ) const
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	rtl::OString aRet;
	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
    {
		ImplStatusItem* pItem = mpItemList->GetObject( nPos );
        if ( pItem->maHelpId.getLength() )
            aRet = pItem->maHelpId;
        else
            aRet = ::rtl::OUStringToOString( pItem->maCommand, RTL_TEXTENCODING_UTF8 );        
    }

	return aRet;
}

// -----------------------------------------------------------------------

void StatusBar::ImplCalcBorder( )
{
	mnCalcHeight = mnDY;
    // subtract border
    if( IsTopBorder() )
    {
        mnCalcHeight -= 2;
        mnTextY += 2;
        mnItemY += 2;
    }
	if ( IsBottomBorder() )
		mnCalcHeight -= 2;
	mbFormat = sal_True;
	Invalidate();
}

void StatusBar::SetBottomBorder( sal_Bool bBottomBorder )
{
	if ( mbBottomBorder != bBottomBorder )
	{
		mbBottomBorder = bBottomBorder;
        ImplCalcBorder();
	}
}

void StatusBar::SetTopBorder( sal_Bool bTopBorder )
{
    if ( mpImplData->mbTopBorder != static_cast<bool>(bTopBorder) )
    {
	    mpImplData->mbTopBorder = static_cast<bool>(bTopBorder);
        ImplCalcBorder();
    }
}

sal_Bool StatusBar::IsTopBorder() const
{
    return mpImplData->mbTopBorder;
}

// -----------------------------------------------------------------------

void StatusBar::StartProgressMode( const XubString& rText )
{
	DBG_ASSERT( !mbProgressMode, "StatusBar::StartProgressMode(): progress mode is active" );

	mbProgressMode	= sal_True;
	mnPercent		= 0;
	maPrgsTxt		= rText;

	// Groessen berechnen
	ImplCalcProgressRect();

	// Paint ausloesen (dort wird der Text und der Frame gemalt)
	const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
	Color aPrgsColor = rStyleSettings.GetHighlightColor();
	if ( aPrgsColor == rStyleSettings.GetFaceColor() )
		aPrgsColor = rStyleSettings.GetDarkShadowColor();
	SetLineColor();
	SetFillColor( aPrgsColor );
	if ( IsReallyVisible() )
	{
		Invalidate();
		Update();
		Flush();
	}
}

// -----------------------------------------------------------------------

void StatusBar::SetProgressValue( sal_uInt16 nNewPercent )
{
	DBG_ASSERT( mbProgressMode, "StatusBar::SetProgressValue(): no progrss mode" );
	DBG_ASSERTWARNING( nNewPercent <= 100, "StatusBar::SetProgressValue(): nPercent > 100" );

	if ( mbProgressMode
	&&   IsReallyVisible()
	&&   (!mnPercent || (mnPercent != nNewPercent)) )
	{
		Update();
		SetLineColor();
		ImplDrawProgress( sal_False, mnPercent, nNewPercent );
		Flush();
	}
	mnPercent = nNewPercent;
}

// -----------------------------------------------------------------------

void StatusBar::EndProgressMode()
{
	DBG_ASSERT( mbProgressMode, "StatusBar::EndProgressMode(): no progress mode" );

	mbProgressMode = sal_False;
	maPrgsTxt.Erase();

	// Paint neu ausloesen um StatusBar wieder herzustellen
	SetFillColor( GetSettings().GetStyleSettings().GetFaceColor() );
	if ( IsReallyVisible() )
	{
		Invalidate();
		Update();
		Flush();
	}
}

// -----------------------------------------------------------------------

void StatusBar::ResetProgressMode()
{
	if ( mbProgressMode )
	{
		mnPercent = 0;		
		maPrgsTxt.Erase();
		if ( IsReallyVisible() )
		{
			Invalidate();
			Update();
			Flush();
		}
	}
}

// -----------------------------------------------------------------------

void StatusBar::SetText( const XubString& rText )
{
	if ( (!mbVisibleItems || (GetStyle() & WB_RIGHT)) && !mbProgressMode &&
		 IsReallyVisible() && IsUpdateMode() )
	{
		if ( mbFormat  )
		{
			Invalidate();
			Window::SetText( rText );
		}
		else
		{
			Update();
			long nOldTextWidth = GetTextWidth( GetText() );
			Window::SetText( rText );
			ImplDrawText( sal_True, nOldTextWidth );
			Flush();
		}
	}
	else if ( mbProgressMode )
	{
		maPrgsTxt = rText;
		if ( IsReallyVisible() )
		{
			Invalidate();
			Update();
			Flush();
		}
	}
	else
		Window::SetText( rText );
}

// -----------------------------------------------------------------------

Size StatusBar::CalcWindowSizePixel() const
{
	sal_uLong	i = 0;
	sal_uLong	nCount = mpItemList->Count();
	long	nOffset = 0;
	long	nCalcWidth = (STATUSBAR_OFFSET_X*2);
	long	nCalcHeight;

	while ( i < nCount )
	{
		ImplStatusItem* pItem = mpItemList->GetObject( i );
		nCalcWidth += pItem->mnWidth + nOffset;
		nOffset = pItem->mnOffset;
		i++;
	}
    
    long nMinHeight = GetTextHeight();
    const long nBarTextOffset = STATUSBAR_OFFSET_TEXTY*2;
    long nProgressHeight = nMinHeight + nBarTextOffset;
    // FIXME: IsNativeControlSupported and GetNativeControlRegion should be const ?
    StatusBar* pThis = const_cast<StatusBar*>( this );
    if( pThis->IsNativeControlSupported( CTRL_PROGRESS, PART_ENTIRE_CONTROL ) )
    {
        ImplControlValue aValue;
        Rectangle aControlRegion( (const Point&)Point(), Size( nCalcWidth, nMinHeight ) );
        Rectangle aNativeControlRegion, aNativeContentRegion;
        if( pThis->GetNativeControlRegion( CTRL_PROGRESS, PART_ENTIRE_CONTROL, aControlRegion,
                                           CTRL_STATE_ENABLED, aValue, rtl::OUString(),
                                           aNativeControlRegion, aNativeContentRegion ) )
        {
            nProgressHeight = aNativeControlRegion.GetHeight();
        }
    }
    
    if( mpImplData->mbDrawItemFrames &&
        pThis->IsNativeControlSupported( CTRL_FRAME, PART_BORDER ) )
    {
        ImplControlValue aControlValue( FRAME_DRAW_NODRAW );
        Rectangle aBound, aContent;
        Rectangle aNatRgn( Point( 0, 0 ), Size( 150, 50 ) );
        if(	pThis->GetNativeControlRegion(CTRL_FRAME, PART_BORDER,
            aNatRgn, 0, aControlValue, rtl::OUString(), aBound, aContent) )
        {
            mpImplData->mnItemBorderWidth =
                ( aBound.GetHeight() - aContent.GetHeight() ) / 2;
        }
    }

	nCalcHeight = nMinHeight+nBarTextOffset + 2*mpImplData->mnItemBorderWidth;
    if( nCalcHeight < nProgressHeight+2 )
        nCalcHeight = nProgressHeight+2;
    
    // add border
    if( IsTopBorder() )
        nCalcHeight += 2;
	if ( IsBottomBorder() )
		nCalcHeight += 2;

	return Size( nCalcWidth, nCalcHeight );
}


// -----------------------------------------------------------------------

void StatusBar::SetAccessibleName( sal_uInt16 nItemId, const XubString& rName )
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
	{
		ImplStatusItem* pItem = mpItemList->GetObject( nPos );

		if ( pItem->maAccessibleName != rName )
		{
			pItem->maAccessibleName = rName;
			ImplCallEventListeners( VCLEVENT_STATUSBAR_NAMECHANGED, (void*) sal_IntPtr(pItem->mnId) );
		}
	}
}

// -----------------------------------------------------------------------

const XubString& StatusBar::GetAccessibleName( sal_uInt16 nItemId ) const
{
	sal_uInt16 nPos = GetItemPos( nItemId );

	if ( nPos != STATUSBAR_ITEM_NOTFOUND )
		return mpItemList->GetObject( nPos )->maAccessibleName;
	else
		return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------
