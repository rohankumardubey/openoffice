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


#ifndef _SVX_DLG_CTRL_HXX
#define _SVX_DLG_CTRL_HXX

// include ---------------------------------------------------------------

#include <svtools/ctrlbox.hxx>
#include <sfx2/tabdlg.hxx>
#include "svx/svxdllapi.h"
#include <svx/rectenum.hxx>
#include <vcl/graph.hxx>
#ifndef _XTABLE_HXX
class XBitmapEntry;
class XBitmapList;
class XColorEntry;
class XColorTable;
class XDash;
class XDashEntry;
class XDashList;
class XGradient;
class XGradientEntry;
class XGradientList;
class XHatch;
class XHatchEntry;
class XHatchList;
class XLineEndEntry;
class XLineEndList;
class XFillAttrSetItem;
#endif

class XOBitmap;
class XOutdevItemPool;

namespace com { namespace sun { namespace star { namespace awt {
	struct Point;
} } } }

/*************************************************************************
|*
|* Von SfxTabPage abgeleitet, um vom Control ueber virtuelle Methode
|* benachrichtigt werden zu koennen.
|*
\************************************************************************/
class SvxTabPage : public SfxTabPage
{

public:
	SvxTabPage( Window* pParent, ResId Id, const SfxItemSet& rInAttrs  ) :
		SfxTabPage( pParent, Id, rInAttrs ) {}

	virtual void PointChanged( Window* pWindow, RECT_POINT eRP ) = 0;
};

/*************************************************************************
|*
|*	Control zur Darstellung und Auswahl der Eckpunkte (und Mittelpunkt)
|*	eines Objekts
|*
\************************************************************************/
typedef sal_uInt16 CTL_STATE;
#define CS_NOHORZ	1		// no horizontal input information is used
#define CS_NOVERT	2		// no vertikal input information is used

class SvxRectCtlAccessibleContext;

class SVX_DLLPUBLIC SvxRectCtl : public Control
{
private:
	SVX_DLLPRIVATE void				InitSettings( sal_Bool bForeground, sal_Bool bBackground );
	SVX_DLLPRIVATE void				InitRectBitmap( void );
	SVX_DLLPRIVATE Bitmap&          GetRectBitmap( void );
    SVX_DLLPRIVATE void             Resize_Impl();

protected:
	SvxRectCtlAccessibleContext*	pAccContext;
	sal_uInt16							nBorderWidth;
	sal_uInt16							nRadius;
	Size							aSize;
	Point							aPtLT, aPtMT, aPtRT;
	Point							aPtLM, aPtMM, aPtRM;
	Point							aPtLB, aPtMB, aPtRB;
	Point							aPtNew;
	RECT_POINT						eRP, eDefRP;
	CTL_STYLE						eCS;
	Bitmap*							pBitmap;
	CTL_STATE						m_nState;

	// #103516# Added a possibility to completely disable this control
	sal_Bool						mbCompleteDisable;

	RECT_POINT			GetRPFromPoint( Point ) const;
	Point				GetPointFromRP( RECT_POINT ) const;
	void				SetFocusRect( const Rectangle* pRect = NULL );		// pRect == NULL -> calculate rectangle in method
	Point				SetActualRPWithoutInvalidate( RECT_POINT eNewRP );	// returns the last point

	virtual void		GetFocus();
	virtual void		LoseFocus();

	Point				GetApproxLogPtFromPixPt( const Point& rRoughPixelPoint ) const;
public:
	SvxRectCtl( Window* pParent, const ResId& rResId, RECT_POINT eRpt = RP_MM,
				sal_uInt16 nBorder = 200, sal_uInt16 nCircle = 80, CTL_STYLE eStyle = CS_RECT );
	virtual ~SvxRectCtl();

	virtual void 		Paint( const Rectangle& rRect );
	virtual void 		MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        KeyInput( const KeyEvent& rKeyEvt );
	virtual void		StateChanged( StateChangedType nStateChange );
	virtual void		DataChanged( const DataChangedEvent& rDCEvt );
    virtual void        Resize();

	void				Reset();
	RECT_POINT			GetActualRP() const;
	void				SetActualRP( RECT_POINT eNewRP );

	void				SetState( CTL_STATE nState );

	sal_uInt8				GetNumOfChilds( void ) const;	// returns number of usable radio buttons

	Rectangle			CalculateFocusRectangle( void ) const;
	Rectangle			CalculateFocusRectangle( RECT_POINT eRectPoint ) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

	RECT_POINT			GetApproxRPFromPixPt( const ::com::sun::star::awt::Point& rPixelPoint ) const;

	// #103516# Added a possibility to completely disable this control
	sal_Bool IsCompletelyDisabled() const { return mbCompleteDisable; }
	void DoCompletelyDisable(sal_Bool bNew);
};

/*************************************************************************
|*
|*	Control zur Darstellung und Auswahl des Winkels der Eckpunkte
|*	eines Objekts
|*
\************************************************************************/
class SvxAngleCtl : public SvxRectCtl
{
private:
	void	Initialize();

protected:
	Font	aFont;
	Size	aFontSize;
	sal_Bool	bPositive;

public:
			SvxAngleCtl( Window* pParent, const ResId& rResId );
			SvxAngleCtl( Window* pParent, const ResId& rResId, Size aSize );

	void	ChangeMetric()
				{ bPositive = !bPositive; }
	virtual void Paint( const Rectangle& rRect );
};

/*************************************************************************
|*
|*	Preview-Control zur Darstellung von Bitmaps
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxBitmapCtl
{
protected:
	Size			aSize;
	sal_uInt16			nLines;
	Color			aPixelColor, aBackgroundColor;
	const sal_uInt16*	pBmpArray;

public:
			SvxBitmapCtl( Window* pParent, const Size& rSize );
			~SvxBitmapCtl();

	XOBitmap	GetXBitmap();

	void	SetBmpArray( const sal_uInt16* pPixel ) { pBmpArray = pPixel; }
	void	SetLines( sal_uInt16 nLns ) { nLines = nLns; }
	void	SetPixelColor( Color aColor ) { aPixelColor = aColor; }
	void	SetBackgroundColor( Color aColor ) { aBackgroundColor = aColor; }
};

/*************************************************************************
|*
|*	Control zum Editieren von Bitmaps
|*
\************************************************************************/
class SVX_DLLPUBLIC SvxPixelCtl : public Control
{
private:
    using OutputDevice::SetLineColor;

protected:
	sal_uInt16		nLines, nSquares;
	Color		aPixelColor;
	Color		aBackgroundColor;
	Color		aLineColor;
	Size		aRectSize;
	sal_uInt16* 	pPixel;
	sal_Bool		bPaintable;

	void	ChangePixel( sal_uInt16 nPixel );

public:
			SvxPixelCtl( Window* pParent, const ResId& rResId,
						sal_uInt16 nNumber = 8 );
			~SvxPixelCtl();

	virtual void Paint( const Rectangle& rRect );
	virtual void MouseButtonDown( const MouseEvent& rMEvt );

	void	SetXBitmap( const XOBitmap& rXOBitmap );

	void	SetPixelColor( const Color& rCol ) { aPixelColor = rCol; }
	void	SetBackgroundColor( const Color& rCol ) { aBackgroundColor = rCol; }
	void	SetLineColor( const Color& rCol ) { aLineColor = rCol; }

	sal_uInt16	GetLineCount() const { return nLines; }
	Color	GetPixelColor() const { return aPixelColor; }
	Color	GetBackgroundColor() const { return aBackgroundColor; }

	sal_uInt16	GetBitmapPixel( const sal_uInt16 nPixelNumber );
	sal_uInt16* GetBitmapPixelPtr() { return pPixel; }

	void	SetPaintable( sal_Bool bTmp ) { bPaintable = bTmp; }
	void	Reset();
};

/*************************************************************************
|*
|* ColorLB kann mit Farben und Namen gefuellt werden
|*
\************************************************************************/
class SVX_DLLPUBLIC ColorLB : public ColorListBox
{

public:
		 ColorLB( Window* pParent, ResId Id ) : ColorListBox( pParent, Id ) {}
		 ColorLB( Window* pParent, WinBits aWB ) : ColorListBox( pParent, aWB ) {}

	virtual void Fill( const XColorTable* pTab );

	void Append( XColorEntry* pEntry, Bitmap* pBmp = NULL );
	void Modify( XColorEntry* pEntry, sal_uInt16 nPos, Bitmap* pBmp = NULL );
};

/*************************************************************************
|*
|* HatchingLB
|*
\************************************************************************/
class SVX_DLLPUBLIC HatchingLB : public ListBox
{

public:
		 HatchingLB( Window* pParent, ResId Id, sal_Bool bUserDraw = sal_True );
		 HatchingLB( Window* pParent, WinBits aWB, sal_Bool bUserDraw = sal_True );

	virtual void Fill( const XHatchList* pList );
	virtual void UserDraw( const UserDrawEvent& rUDEvt );

	void	Append( XHatchEntry* pEntry, Bitmap* pBmp = NULL );
	void	Modify( XHatchEntry* pEntry, sal_uInt16 nPos, Bitmap* pBmp = NULL );
	void	SelectEntryByList( const XHatchList* pList, const String& rStr,
						const XHatch& rXHatch, sal_uInt16 nDist = 0 );

private:
	XHatchList*		mpList;
	sal_Bool			mbUserDraw;
};

/*************************************************************************
|*
|* GradientLB
|*
\************************************************************************/
class SVX_DLLPUBLIC GradientLB : public ListBox
{
public:
	GradientLB( Window* pParent, ResId Id, sal_Bool bUserDraw = sal_True );
	GradientLB( Window* pParent, WinBits aWB, sal_Bool bUserDraw = sal_True );

	virtual void Fill( const XGradientList* pList );
	virtual void UserDraw( const UserDrawEvent& rUDEvt );

	void	Append( XGradientEntry* pEntry, Bitmap* pBmp = NULL );
	void	Modify( XGradientEntry* pEntry, sal_uInt16 nPos, Bitmap* pBmp = NULL );
	void	SelectEntryByList( const XGradientList* pList, const String& rStr,
						const XGradient& rXGradient, sal_uInt16 nDist = 0 );

private:
	XGradientList* mpList;
	sal_Bool			mbUserDraw;
};

/*************************************************************************
|*
|* BitmapLB
|*
\************************************************************************/
class SVX_DLLPUBLIC BitmapLB : public ListBox
{
public:
		 BitmapLB( Window* pParent, ResId Id, sal_Bool bUserDraw = sal_True );

	virtual void Fill( const XBitmapList* pList );
	virtual void UserDraw( const UserDrawEvent& rUDEvt );

	void	Append( XBitmapEntry* pEntry, Bitmap* pBmp = NULL );
	void	Modify( XBitmapEntry* pEntry, sal_uInt16 nPos, Bitmap* pBmp = NULL );
	void	SelectEntryByList( const XBitmapList* pList, const String& rStr,
						const Bitmap& rBmp);

private:
	VirtualDevice	aVD;
	Bitmap			aBitmap;

	XBitmapList*	mpList;
	sal_Bool			mbUserDraw;

	SVX_DLLPRIVATE void SetVirtualDevice();
};

/*************************************************************************
|*
|* FillAttrLB vereint alle Fuellattribute in einer ListBox
|*
\************************************************************************/
class FillAttrLB : public ColorListBox
{
private:
	VirtualDevice	aVD;
	Bitmap			aBitmap;

	void SetVirtualDevice();

public:
		 FillAttrLB( Window* pParent, ResId Id );
		 FillAttrLB( Window* pParent, WinBits aWB );

	virtual void Fill( const XColorTable* pTab );
	virtual void Fill( const XHatchList* pList );
	virtual void Fill( const XGradientList* pList );
	virtual void Fill( const XBitmapList* pList );

	void	SelectEntryByList( const XBitmapList* pList, const String& rStr,
						const Bitmap& rBmp);
};

/*************************************************************************
|*
|* FillTypeLB
|*
\************************************************************************/
class FillTypeLB : public ListBox
{

public:
		 FillTypeLB( Window* pParent, ResId Id ) : ListBox( pParent, Id ) {}
		 FillTypeLB( Window* pParent, WinBits aWB ) : ListBox( pParent, aWB ) {}

	virtual void Fill();
};

/*************************************************************************
|*
|* LineLB
|*
\************************************************************************/
class SVX_DLLPUBLIC LineLB : public ListBox
{

public:
		 LineLB( Window* pParent, ResId Id ) : ListBox( pParent, Id ) {}
		 LineLB( Window* pParent, WinBits aWB ) : ListBox( pParent, aWB ) {}

	virtual void Fill( const XDashList* pList );

	void Append( XDashEntry* pEntry, Bitmap* pBmp = NULL );
	void Modify( XDashEntry* pEntry, sal_uInt16 nPos, Bitmap* pBmp = NULL );
	void SelectEntryByList( const XDashList* pList, const String& rStr,
							const XDash& rDash, sal_uInt16 nDist = 0 );
	void FillStyles();
};

/*************************************************************************
|*
|* LineEndsLB
|*
\************************************************************************/
class SVX_DLLPUBLIC LineEndLB : public ListBox
{

public:
		 LineEndLB( Window* pParent, ResId Id ) : ListBox( pParent, Id ) {}
		 LineEndLB( Window* pParent, WinBits aWB ) : ListBox( pParent, aWB ) {}

	virtual void Fill( const XLineEndList* pList, sal_Bool bStart = sal_True );

	void	Append( XLineEndEntry* pEntry, Bitmap* pBmp = NULL,
					sal_Bool bStart = sal_True );
	void	Modify( XLineEndEntry* pEntry, sal_uInt16 nPos, Bitmap* pBmp = NULL,
					sal_Bool bStart = sal_True );
};

//////////////////////////////////////////////////////////////////////////////

class SdrObject;
class SdrModel;

class SvxPreviewBase : public Control
{
private:
	SdrModel*										mpModel;
    VirtualDevice*                                  mpBufferDevice;

protected:
	void InitSettings(bool bForeground, bool bBackground);

    // prepare buffered paint
    void LocalPrePaint();

    // end and output buffered paint
    void LocalPostPaint();

public:
	SvxPreviewBase( Window* pParent, const ResId& rResId );
	virtual ~SvxPreviewBase();

    // change support
	virtual void StateChanged(StateChangedType nStateChange);
	virtual void DataChanged(const DataChangedEvent& rDCEvt);

    // dada read access
    SdrModel& getModel() const { return *mpModel; }
    OutputDevice& getBufferDevice() const { return *mpBufferDevice; }
};

/*************************************************************************
|*
|* SvxLinePreview
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxXLinePreview : public SvxPreviewBase
{
private:
	SdrObject*										mpLineObjA;
	SdrObject*										mpLineObjB;
	SdrObject*										mpLineObjC;
	
	//#58425# Symbole auf einer Linie (z.B. StarChart)
	Graphic*										mpGraphic;
	sal_Bool										mbWithSymbol;
	Size											maSymbolSize;

public:
	SvxXLinePreview( Window* pParent, const ResId& rResId );
	virtual ~SvxXLinePreview();

	void SetLineAttributes(const SfxItemSet& rItemSet);

	void ShowSymbol( sal_Bool b ) { mbWithSymbol = b; };
	void SetSymbol( Graphic* p, const Size& s );
	void ResizeSymbol( const Size& s );

	virtual void Paint( const Rectangle& rRect );
};

/*************************************************************************
|*
|* SvxXRectPreview
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxXRectPreview : public SvxPreviewBase
{
private:
	SdrObject*										mpRectangleObject;

public:
	SvxXRectPreview( Window* pParent, const ResId& rResId );
	virtual ~SvxXRectPreview();

	void SetAttributes(const SfxItemSet& rItemSet);

	virtual void	Paint( const Rectangle& rRect );
};

/*************************************************************************
|*
|* SvxXShadowPreview
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxXShadowPreview : public SvxPreviewBase
{
private:
	SdrObject*										mpRectangleObject;
	SdrObject*										mpRectangleShadow;

public:
	SvxXShadowPreview( Window* pParent, const ResId& rResId );
	virtual ~SvxXShadowPreview();

	void SetRectangleAttributes(const SfxItemSet& rItemSet);
	void SetShadowAttributes(const SfxItemSet& rItemSet);
	void SetShadowPosition(const Point& rPos);
	
	virtual void	Paint( const Rectangle& rRect );
};

#endif // _SVX_DLG_CTRL_HXX

