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



#ifndef _SV_SALGDI_H
#define _SV_SALGDI_H

// -=-= includes -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "tools/fract.hxx"

#include "vcl/salgtype.hxx"
#include "vcl/vclenum.hxx"

#include "salstd.hxx"
#include "salgdi.hxx"
#include "sallayout.hxx"
#include "vclpluginapi.h"

#include <deque>

// -=-= forwards -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct  ImplFontMetricData;
struct  ImplFontSelectData;
class   SalBitmap;
class   SalColormap;
class   SalDisplay;
class   SalFrame;
class   X11SalVirtualDevice;
class   SalPolyLine;
class   PspSalPrinter;
class   PspSalInfoPrinter;
class   ServerFont;
class   ImplLayoutArgs;
class   X11FontLayout;
class   ServerFontLayout;

namespace basegfx {
    class B2DTrapezoid;
}

// -=-= SalGraphicsData =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

class CairoFontsCache
{
private:
    static int mnRefCount;
    typedef std::deque< std::pair<void *, void*> > LRUFonts;
    static LRUFonts maLRUFonts;
public:
    CairoFontsCache();
    static void  CacheFont(void *pFont, void *pId);
    static void* FindCachedFont(void *pId);
    ~CairoFontsCache();
};

class VCLPLUG_GEN_PUBLIC X11SalGraphics : public SalGraphics
{
    friend class            X11FontLayout;
    friend class            ServerFontLayout;
protected:
    SalFrame*				m_pFrame; // the SalFrame which created this Graphics or NULL
    X11SalVirtualDevice*	m_pVDev;  // the SalVirtualDevice which created this Graphics or NULL

    const SalColormap*      m_pColormap;
    SalColormap    *m_pDeleteColormap;
    Drawable        hDrawable_;     // use
    int             m_nScreen;
    void*           m_pRenderFormat;
    XID             m_aRenderPicture;
    CairoFontsCache m_aCairoFontsCache;

    XLIB_Region     pPaintRegion_;
    XLIB_Region     mpClipRegion;

    GC              pPenGC_;        // Pen attributes
    SalColor        nPenColor_;
    Pixel           nPenPixel_;

    GC              pFontGC_;       // Font attributes
    ServerFont*             mpServerFont[ MAX_FALLBACK ];

    SalColor        nTextColor_;
    Pixel           nTextPixel_;
    sal_Bool            bFontVertical_;

    sal_Bool            bDisableGraphite_;

    GC              pBrushGC_;      // Brush attributes
    SalColor        nBrushColor_;
    Pixel           nBrushPixel_;
    Pixmap          hBrush_;        // Dither

    GC				pMonoGC_;
    GC				pCopyGC_;
    GC				pMaskGC_;
    GC				pInvertGC_;
    GC				pInvert50GC_;
    GC				pStippleGC_;
    GC				pTrackingGC_;

    sal_Bool			bWindow_ : 1;		// is Window
    sal_Bool			bPrinter_ : 1;		// is Printer
    sal_Bool			bVirDev_ : 1;		// is VirDev
    sal_Bool			bPenGC_ : 1;		// is Pen GC valid
    sal_Bool			bFontGC_ : 1;		// is Font GC valid
    sal_Bool			bBrushGC_ : 1;		// is Brush GC valid
    sal_Bool			bMonoGC_ : 1;		// is Mono GC valid
    sal_Bool			bCopyGC_ : 1;		// is Copy GC valid
    sal_Bool			bInvertGC_ : 1;		// is Invert GC valid
    sal_Bool			bInvert50GC_ : 1;	// is Invert50 GC valid
    sal_Bool			bStippleGC_ : 1;	// is Stipple GC valid
    sal_Bool            bTrackingGC_ : 1;   // is Tracking GC valid
    bool			bXORMode_ : 1;		// is ROP XOR Mode set
    sal_Bool			bDitherBrush_ : 1;	// is solid or tile

    using SalGraphics::SetClipRegion;
    void			SetClipRegion( GC          pGC,
                                   XLIB_Region pXReg = NULL ) const;

    GC				GetTrackingGC();
    GC				GetInvertGC();
    GC				GetInvert50GC();
    GC				CreateGC( Drawable      hDrawable,
                              unsigned long nMask = GCGraphicsExposures );
    GC				SelectPen();
    GC				SelectBrush();
    void			DrawLines( sal_uIntPtr              nPoints,
                               const SalPolyLine &rPoints,
                               GC				  pGC,
                               bool bClose
                               );
    sal_Bool			GetDitherPixmap ( SalColor nSalColor );

    inline	GC				GetMonoGC( Pixmap hPixmap );
    inline	GC				GetCopyGC();
    inline	GC				GetStippleGC();

    int 			Clip      ( XLIB_Region   pRegion,
                                int          &nX,
                                int          &nY,
                                unsigned int &nDX,
                                unsigned int &nDY,
                                int          &nSrcX,
                                int          &nSrcY ) const;
    int				Clip      ( int          &nX,
                                int          &nY,
                                unsigned int &nDX,
                                unsigned int &nDY,
                                int          &nSrcX,
                                int          &nSrcY ) const;
    GC				SetMask   ( int          &nX,
                                int          &nY,
                                unsigned int &nDX,
                                unsigned int &nDY,
                                int          &nSrcX,
                                int          &nSrcY,
                                Pixmap        hClipMask );
    using SalGraphics::DrawBitmap;
    void            DrawBitmap( const SalTwoRect *pPosAry,
                                SalGraphics      *pThis,
                                const SalBitmap  &rSalBitmap,
                                const SalBitmap  &rTransparentBitmap,
                                SalColor          nTransparentColor );

    GC                      GetFontGC();
    bool                    setFont( const ImplFontSelectData* pEntry, int nFallbackLevel );

    void                    drawMaskedBitmap( const SalTwoRect* pPosAry,
                                              const SalBitmap& rSalBitmap,
                                              const SalBitmap& rTransparentBitmap );

protected:
    void                    DrawPrinterString( const SalLayout& );

    void                    DrawServerFontString( const ServerFontLayout& );
    void                    DrawServerSimpleFontString( const ServerFontLayout& );
    void                    DrawServerAAFontString( const ServerFontLayout& );
    bool                    DrawServerAAForcedString( const ServerFontLayout& );
    void                    DrawCairoAAFontString( const ServerFontLayout& );
    
    void freeResources();
public:
                            X11SalGraphics();
    virtual				~X11SalGraphics();

            void            Init( SalFrame *pFrame, Drawable aDrawable, int nScreen );
            void            Init( X11SalVirtualDevice *pVirtualDevice, SalColormap* pColormap = NULL, bool bDeleteColormap = false );
            void            Init( class ImplSalPrinterData *pPrinter );
            void            DeInit();

    inline  const SalDisplay*   GetDisplay() const;
    inline  Display*            GetXDisplay() const;
    inline  const SalVisual&    GetVisual() const;
    inline  Drawable        GetDrawable() const { return hDrawable_; }
    void                    SetDrawable( Drawable d, int nScreen );
    XID                     GetXRenderPicture();
    void*                   GetXRenderFormat() const { return m_pRenderFormat; }
    inline  void            SetXRenderFormat( void* pRenderFormat ) { m_pRenderFormat = pRenderFormat; }
    inline  const SalColormap&    GetColormap() const { return *m_pColormap; }
    using SalGraphics::GetPixel;
    inline  Pixel           GetPixel( SalColor nSalColor ) const;

    int                     GetScreenNumber() const { return m_nScreen; }

    // overload all pure virtual methods
    virtual void			GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY );
    virtual sal_uInt16			GetBitCount();
    virtual long			GetGraphicsWidth() const;
    virtual long			GetGraphicsHeight() const;

    virtual void			ResetClipRegion();
    virtual bool            setClipRegion( const Region& );

    virtual void			SetLineColor();
    virtual void			SetLineColor( SalColor nSalColor );
    virtual void			SetFillColor();

    virtual void          	SetFillColor( SalColor nSalColor );

    virtual void			SetXORMode( bool bSet, bool );

    virtual void			SetROPLineColor( SalROPColor nROPColor );
    virtual void			SetROPFillColor( SalROPColor nROPColor );

    virtual void			SetTextColor( SalColor nSalColor );
    virtual sal_uInt16			SetFont( ImplFontSelectData*, int nFallbackLevel );
    virtual void			GetFontMetric( ImplFontMetricData*, int nFallbackLevel );
    virtual sal_uLong			GetKernPairs( sal_uLong nMaxPairs, ImplKernPairData* );
    virtual const ImplFontCharMap* GetImplFontCharMap() const;
    virtual void			GetDevFontList( ImplDevFontList* );
    virtual void			GetDevFontSubstList( OutputDevice* );
    virtual bool			AddTempDevFont( ImplDevFontList*, const String& rFileURL, const String& rFontName );
    virtual sal_Bool			CreateFontSubset( const rtl::OUString& rToFile,
                                              const ImplFontData*,
                                              sal_Int32* pGlyphIDs,
                                              sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo
                                              );
    virtual const Ucs2SIntMap* GetFontEncodingVector( const ImplFontData*, const Ucs2OStrMap** ppNonEncoded );
    virtual const void*	GetEmbedFontData( const ImplFontData*,
                                          const sal_Ucs* pUnicodes,
                                          sal_Int32* pWidths,
                                          FontSubsetInfo& rInfo,
                                          long* pDataLen );
    virtual void			FreeEmbedFontData( const void* pData, long nDataLen );
    virtual void            GetGlyphWidths( const ImplFontData*,
                                            bool bVertical,
                                            Int32Vector& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc );
    virtual sal_Bool			GetGlyphBoundRect( long nIndex, Rectangle& );
    virtual sal_Bool			GetGlyphOutline( long nIndex, ::basegfx::B2DPolyPolygon& );
    virtual SalLayout*		GetTextLayout( ImplLayoutArgs&, int nFallbackLevel );
    virtual void			DrawServerFontLayout( const ServerFontLayout& );
    virtual bool            supportsOperation( OutDevSupportType ) const;
    virtual void			drawPixel( long nX, long nY );
    virtual void			drawPixel( long nX, long nY, SalColor nSalColor );
    virtual void			drawLine( long nX1, long nY1, long nX2, long nY2 );
    virtual void			drawRect( long nX, long nY, long nWidth, long nHeight );
    void                    drawPolyLine( sal_uIntPtr nPoints, const SalPoint* pPtAry, bool bClose );
    virtual void			drawPolyLine( sal_uIntPtr nPoints, const SalPoint* pPtAry );
    virtual void			drawPolygon( sal_uIntPtr nPoints, const SalPoint* pPtAry );
    virtual void			drawPolyPolygon( sal_uInt32 nPoly,
                                             const sal_uInt32* pPoints,
                                             PCONSTSALPOINT* pPtAry );
    virtual bool			drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double fTransparency );
    virtual bool			drawPolyLine( 
        const ::basegfx::B2DPolygon&, 
        double fTransparency, 
        const ::basegfx::B2DVector& rLineWidth, 
        basegfx::B2DLineJoin,
        com::sun::star::drawing::LineCap);
    virtual bool			drawFilledTrapezoids( const ::basegfx::B2DTrapezoid*, int nTrapCount, double fTransparency );

#if 1 // TODO: remove these obselete methods
    virtual sal_Bool		drawPolyLineBezier( sal_uIntPtr nPoints,
                                                const SalPoint* pPtAry,
                                                const sal_uInt8* pFlgAry );
    virtual sal_Bool		drawPolygonBezier( sal_uIntPtr nPoints,
                                               const SalPoint* pPtAry,
                                               const sal_uInt8* pFlgAry );
    virtual sal_Bool		drawPolyPolygonBezier( sal_uInt32 nPoly,
                                                   const sal_uInt32* pPoints,
                                                   const SalPoint* const* pPtAry,
                                                   const sal_uInt8* const* pFlgAry );
#endif

    virtual void			copyArea( long nDestX,
                                      long nDestY,
                                      long nSrcX,
                                      long nSrcY,
                                      long nSrcWidth,
                                      long nSrcHeight,
                                      sal_uInt16 nFlags );
    virtual void			copyBits( const SalTwoRect* pPosAry,
                                      SalGraphics* pSrcGraphics );
    virtual void			drawBitmap( const SalTwoRect* pPosAry,
                                        const SalBitmap& rSalBitmap );
    virtual void			drawBitmap( const SalTwoRect* pPosAry,
                                        const SalBitmap& rSalBitmap,
                                        SalColor nTransparentColor );
    virtual void			drawBitmap( const SalTwoRect* pPosAry,
                                        const SalBitmap& rSalBitmap,
                                        const SalBitmap& rMaskBitmap );
    virtual void			drawMask( const SalTwoRect* pPosAry,
                                      const SalBitmap& rSalBitmap,
                                      SalColor nMaskColor );
    virtual SalBitmap*		getBitmap( long nX, long nY, long nWidth, long nHeight );
    virtual SalColor		getPixel( long nX, long nY );
    virtual void			invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags );
    virtual void			invert( sal_uIntPtr nPoints, const SalPoint* pPtAry, SalInvert nFlags );

    virtual sal_Bool			drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uIntPtr nSize );

    virtual bool 			drawAlphaBitmap( const SalTwoRect&,
                                             const SalBitmap& rSourceBitmap,
                                             const SalBitmap& rAlphaBitmap );

    virtual bool		    drawAlphaRect( long nX, long nY, long nWidth, 
                                           long nHeight, sal_uInt8 nTransparency );

    virtual SystemGraphicsData GetGraphicsData() const;
    virtual SystemFontData     GetSysFontData( int nFallbacklevel ) const;

    /*  use to handle GraphicsExpose/NoExpose after XCopyArea & friends
     *  if pFrame is not NULL, corresponding Paint events are generated
     *  and dispatched to pFrame
     *
     *  it is imperative to eat up graphics exposes even in case you don't need
     *  them because the next one using XCopyArea can depend on them
     */
    void YieldGraphicsExpose();

    // do XCopyArea or XGet/PutImage depending on screen numbers
    // signature is like XCopyArea with screen numbers added
    static void CopyScreenArea( Display* pDisplay,
                                Drawable aSrc, int nScreenSrc, int nSrcDepth,
                                Drawable aDest, int nScreenDest, int nDestDepth,
                                GC aDestGC,
                                int src_x, int src_y,
                                unsigned int w, unsigned int h,
                                int dest_x, int dest_y );
    static void releaseGlyphPeer();
};


inline const SalDisplay *X11SalGraphics::GetDisplay() const
{ return GetColormap().GetDisplay(); }

inline const SalVisual& X11SalGraphics::GetVisual() const
{ return GetColormap().GetVisual(); }

inline Display *X11SalGraphics::GetXDisplay() const
{ return GetColormap().GetXDisplay(); }

inline Pixel X11SalGraphics::GetPixel( SalColor nSalColor ) const
{ return GetColormap().GetPixel( nSalColor ); }


// -=-= Shortcuts =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifdef DBG_UTIL
#define stderr0( s )			fprintf( stderr, s )
#else
#define stderr0( s )			;
#endif

#endif // _SV_SALGDI_H

