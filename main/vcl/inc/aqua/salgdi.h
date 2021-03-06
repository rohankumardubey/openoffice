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

#include "basegfx/polygon/b2dpolypolygon.hxx"

#include "premac.h"
#include <ApplicationServices/ApplicationServices.h>
#include "postmac.h"

#include "aqua/aquavcltypes.h"

#include "outfont.hxx"
#include "salgdi.hxx"

#include <vector>

class AquaSalFrame;
class AquaSalBitmap;
class ImplDevFontAttributes;

class CGRect;

// mac specific physically available font face
class ImplMacFontData : public ImplFontData
{
public:
	ImplMacFontData( const ImplDevFontAttributes&, ATSUFontID );

    virtual ~ImplMacFontData();

    virtual ImplFontData*   Clone() const;
    virtual ImplFontEntry*  CreateFontInstance( ImplFontSelectData& ) const;
	virtual sal_IntPtr      GetFontId() const;
    
    const ImplFontCharMap*	GetImplFontCharMap() const;
	bool					HasChar( sal_uInt32 cChar ) const;

	void					ReadOs2Table() const;
	void					ReadMacCmapEncoding() const;
	bool					HasCJKSupport() const;

private:
    const ATSUFontID			mnFontId;
	mutable const ImplFontCharMap*	mpCharMap;
	mutable bool				mbOs2Read;		 // true if OS2-table related info is valid
	mutable bool				mbHasOs2Table;
	mutable bool				mbCmapEncodingRead; // true if cmap encoding of Mac font is read
	mutable bool				mbHasCJKSupport; // #i78970# CJK fonts need extra leading
};

// abstracting quartz color instead of having to use an CGFloat[] array
class RGBAColor
{
public:
	RGBAColor( SalColor );
	RGBAColor( float fRed, float fGreen, float fBlue, float fAlpha ); //NOTUSEDYET
	const float* AsArray() const	{ return &mfRed; }
	bool IsVisible() const			{ return (mfAlpha > 0); }
	void SetAlpha( float fAlpha )	{ mfAlpha = fAlpha; }
private:
	float mfRed, mfGreen, mfBlue, mfAlpha;
};

// -------------------
// - AquaSalGraphics -
// -------------------
class AquaSalGraphics : public SalGraphics
{
    friend class ATSLayout;
protected:
    AquaSalFrame*                           mpFrame;
	CGLayerRef								mxLayer;	// Quartz graphics layer
	CGContextRef		                    mrContext;	// Quartz drawing context
	class XorEmulation*						mpXorEmulation;
    int                                     mnXorMode; // 0: off 1: on 2: invert only
	int										mnWidth;
	int										mnHeight;
	int										mnBitmapDepth;	// zero unless bitmap
    /// device resolution of this graphics
    long                                    mnRealDPIX;
	long                                    mnRealDPIY;
	/// some graphics implementations (e.g. AquaSalInfoPrinter) scale
	/// everything down by a factor (see SetupPrinterGraphics for details)
	/// so we have to compensate for it with the inverse factor
    double                                  mfFakeDPIScale;

    /// path representing current clip region
    CGMutablePathRef                        mxClipPath;

    /// Drawing colors    
    /// pen color RGBA
    RGBAColor                               maLineColor;
    /// brush color RGBA
    RGBAColor                               maFillColor;

    // Device Font settings
 	const ImplMacFontData*                  mpMacFontData;
    /// ATSU style object which carries all font attributes
    ATSUStyle			                    maATSUStyle;
    /// text rotation as ATSU angle
    Fixed                                   mnATSUIRotation;
    /// workaround to prevent ATSU overflows for huge font sizes
    float                                   mfFontScale;
    /// <1.0: font is squeezed, >1.0 font is stretched, else 1.0
    float                                   mfFontStretch;
    /// allows text to be rendered without antialiasing
    bool                                    mbNonAntialiasedText;

	// Graphics types
    
    /// is this a printer graphics
	bool                                    mbPrinter;
    /// is this a virtual device graphics
	bool                                    mbVirDev;
    /// is this a window graphics
	bool                                    mbWindow;

public:
    AquaSalGraphics();	
    virtual ~AquaSalGraphics();

    bool                IsPenVisible() const	{ return maLineColor.IsVisible(); }
    bool                IsBrushVisible() const	{ return maFillColor.IsVisible(); }
    
    void                SetWindowGraphics( AquaSalFrame* pFrame );
    void                SetPrinterGraphics( CGContextRef, long nRealDPIX, long nRealDPIY, double fFakeScale );
    void                SetVirDevGraphics( CGLayerRef, CGContextRef, int nBitDepth = 0 );

    void                initResolution( NSWindow* );
    void                copyResolution( AquaSalGraphics& );
    void                updateResolution();
    
    bool                IsWindowGraphics()      const   { return mbWindow; }
    bool                IsPrinterGraphics()     const   { return mbPrinter; }
    bool                IsVirDevGraphics()      const   { return mbVirDev; }
    AquaSalFrame*       getGraphicsFrame() const { return mpFrame; }
    void                setGraphicsFrame( AquaSalFrame* pFrame ) { mpFrame = pFrame; }

    void                ImplDrawPixel( long nX, long nY, const RGBAColor& ); // helper to draw single pixels
    
    bool                CheckContext();
    void                UpdateWindow( NSRect& ); // delivered in NSView coordinates
	void				RefreshRect( const CGRect& );
	void				RefreshRect( const NSRect& );
	void				RefreshRect(float lX, float lY, float lWidth, float lHeight);

    void                SetState();
    void                UnsetState();
    // InvalidateContext does an UnsetState and sets mrContext to 0
    void                InvalidateContext();

    virtual bool        setClipRegion( const Region& );

    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    virtual void		drawPixel( long nX, long nY );
    virtual void		drawPixel( long nX, long nY, SalColor nSalColor );
    virtual void		drawLine( long nX1, long nY1, long nX2, long nY2 );
    virtual void		drawRect( long nX, long nY, long nWidth, long nHeight );
    virtual void		drawPolyLine( sal_uLong nPoints, const SalPoint* pPtAry );
    virtual void		drawPolygon( sal_uLong nPoints, const SalPoint* pPtAry );
    virtual void		drawPolyPolygon( sal_uLong nPoly, const sal_uLong* pPoints, PCONSTSALPOINT* pPtAry );
    virtual bool        drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double fTransparency );
    virtual sal_Bool	drawPolyLineBezier( sal_uLong nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry );
    virtual sal_Bool	drawPolygonBezier( sal_uLong nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry );
    virtual sal_Bool	drawPolyPolygonBezier( sal_uLong nPoly, const sal_uLong* pPoints, const SalPoint* const* pPtAry, const sal_uInt8* const* pFlgAry );
    virtual bool        drawPolyLine( 
        const ::basegfx::B2DPolygon&, 
        double fTransparency, 
        const ::basegfx::B2DVector& rLineWidths, 
        basegfx::B2DLineJoin,
        com::sun::star::drawing::LineCap eLineCap);

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void		copyArea( long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth,
                                  long nSrcHeight, sal_uInt16 nFlags );

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    virtual void		copyBits( const SalTwoRect* pPosAry, SalGraphics* pSrcGraphics );
    virtual void		drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap );
    virtual void		drawBitmap( const SalTwoRect* pPosAry,
                                    const SalBitmap& rSalBitmap,
                                    SalColor nTransparentColor );
    virtual void		drawBitmap( const SalTwoRect* pPosAry,
                                    const SalBitmap& rSalBitmap,
                                    const SalBitmap& rTransparentBitmap );
    virtual void		drawMask( const SalTwoRect* pPosAry,
                                  const SalBitmap& rSalBitmap,
                                  SalColor nMaskColor );

    virtual SalBitmap*	getBitmap( long nX, long nY, long nWidth, long nHeight );
    virtual SalColor	getPixel( long nX, long nY );

    // invert --> ClipRegion (only Windows or VirDevs)
    virtual void		invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags);
    virtual void		invert( sal_uLong nPoints, const SalPoint* pPtAry, SalInvert nFlags );

    virtual sal_Bool		drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize );

    virtual bool 			drawAlphaBitmap( const SalTwoRect&,
                                             const SalBitmap& rSourceBitmap,
                                             const SalBitmap& rAlphaBitmap );

    virtual bool		    drawAlphaRect( long nX, long nY, long nWidth,
                                           long nHeight, sal_uInt8 nTransparency );

    CGPoint*                makeCGptArray(sal_uLong nPoints, const SalPoint*  pPtAry);
    // native widget rendering methods that require mirroring
    virtual sal_Bool        hitTestNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                              const Point& aPos, sal_Bool& rIsInside );
    virtual sal_Bool        drawNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                           ControlState nState, const ImplControlValue& aValue,
                                           const rtl::OUString& aCaption );
    virtual sal_Bool        drawNativeControlText( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                                               ControlState nState, const ImplControlValue& aValue,
                                               const rtl::OUString& aCaption );
    virtual sal_Bool        getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState nState,
                                                const ImplControlValue& aValue, const rtl::OUString& aCaption,
                                                Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion );

    // get device resolution
    virtual void			GetResolution( long& rDPIX, long& rDPIY );
    // get the depth of the device
    virtual sal_uInt16			GetBitCount();
    // get the width of the device
    virtual long			GetGraphicsWidth() const;

    // set the clip region to empty
    virtual void			ResetClipRegion();

    // set the line color to transparent (= don't draw lines)
    virtual void			SetLineColor();
    // set the line color to a specific color
    virtual void			SetLineColor( SalColor nSalColor );
    // set the fill color to transparent (= don't fill)
    virtual void			SetFillColor();
    // set the fill color to a specific color, shapes will be
    // filled accordingly
    virtual void          	SetFillColor( SalColor nSalColor );
    // enable/disable XOR drawing
    virtual void			SetXORMode( bool bSet, bool bInvertOnly );
    // set line color for raster operations
    virtual void			SetROPLineColor( SalROPColor nROPColor );
    // set fill color for raster operations
    virtual void			SetROPFillColor( SalROPColor nROPColor );
    // set the text color to a specific color
    virtual void			SetTextColor( SalColor nSalColor );
    // set the font
    virtual sal_uInt16         SetFont( ImplFontSelectData*, int nFallbackLevel );
    // get the current font's etrics
    virtual void			GetFontMetric( ImplFontMetricData*, int nFallbackLevel );
    // get kernign pairs of the current font
    // return only PairCount if (pKernPairs == NULL)
    virtual sal_uLong			GetKernPairs( sal_uLong nPairs, ImplKernPairData* pKernPairs );
    // get the repertoire of the current font
    virtual const ImplFontCharMap* GetImplFontCharMap() const;
    // graphics must fill supplied font list
    virtual void			GetDevFontList( ImplDevFontList* );
    // graphics should call ImplAddDevFontSubstitute on supplied
    // OutputDevice for all its device specific preferred font substitutions
    virtual void			GetDevFontSubstList( OutputDevice* );
    virtual bool			AddTempDevFont( ImplDevFontList*, const String& rFileURL, const String& rFontName );
    // CreateFontSubset: a method to get a subset of glyhps of a font
    // inside a new valid font file
    // returns TRUE if creation of subset was successfull
    // parameters: rToFile: contains a osl file URL to write the subset to
    //             pFont: describes from which font to create a subset
    //             pGlyphIDs: the glyph ids to be extracted
    //             pEncoding: the character code corresponding to each glyph
    //             pWidths: the advance widths of the correspoding glyphs (in PS font units)
    //             nGlyphs: the number of glyphs
    //             rInfo: additional outgoing information
    // implementation note: encoding 0 with glyph id 0 should be added implicitly
    // as "undefined character"
    virtual sal_Bool			CreateFontSubset( const rtl::OUString& rToFile,
                                              const ImplFontData* pFont,
                                              long* pGlyphIDs,
                                              sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo // out parameter
                                              );

    // GetFontEncodingVector: a method to get the encoding map Unicode
	// to font encoded character; this is only used for type1 fonts and
    // may return NULL in case of unknown encoding vector
    // if ppNonEncoded is set and non encoded characters (that is type1
    // glyphs with only a name) exist it is set to the corresponding
    // map for non encoded glyphs; the encoding vector contains -1
    // as encoding for these cases
    virtual const Ucs2SIntMap* GetFontEncodingVector( const ImplFontData*, const Ucs2OStrMap** ppNonEncoded );

    // GetEmbedFontData: gets the font data for a font marked
    // embeddable by GetDevFontList or NULL in case of error
    // parameters: pFont: describes the font in question
    //             pWidths: the widths of all glyphs from char code 0 to 255
    //                      pWidths MUST support at least 256 members;
    //             rInfo: additional outgoing information
    //             pDataLen: out parameter, contains the byte length of the returned buffer
    virtual const void*	GetEmbedFontData( const ImplFontData*,
                                          const sal_Ucs* pUnicodes,
                                          sal_Int32* pWidths,
                                          FontSubsetInfo& rInfo,
                                          long* pDataLen );
    // frees the font data again
    virtual void			FreeEmbedFontData( const void* pData, long nDataLen );

    virtual void            GetGlyphWidths( const ImplFontData*,
                                            bool bVertical,
                                            Int32Vector& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc );

    virtual sal_Bool                    GetGlyphBoundRect( long nIndex, Rectangle& );
    virtual sal_Bool                    GetGlyphOutline( long nIndex, basegfx::B2DPolyPolygon& );

    virtual SalLayout*              GetTextLayout( ImplLayoutArgs&, int nFallbackLevel );
    virtual void					 DrawServerFontLayout( const ServerFontLayout& );
    virtual bool                    supportsOperation( OutDevSupportType ) const;

    // Query the platform layer for control support
    virtual sal_Bool IsNativeControlSupported( ControlType nType, ControlPart nPart );

    virtual SystemGraphicsData    GetGraphicsData() const;
    virtual SystemFontData        GetSysFontData( int /* nFallbacklevel */ ) const;

private:
    // differences between VCL, Quartz and kHiThemeOrientation coordinate systems
    // make some graphics seem to be vertically-mirrored from a VCL perspective
    bool IsFlipped() const { return mbWindow; }

	void ApplyXorContext();
    void Pattern50Fill();
    UInt32 getState( ControlState nState );
    UInt32 getTrackState( ControlState nState );
};

class XorEmulation
{
public:
					XorEmulation();
	/*final*/		~XorEmulation();

	void			SetTarget( int nWidth, int nHeight, int nBitmapDepth, CGContextRef, CGLayerRef );
	bool			UpdateTarget();
	void			Enable()			{ mbIsEnabled = true; }
	void			Disable()			{ mbIsEnabled = false; }
	bool 			IsEnabled() const	{ return mbIsEnabled; }
	CGContextRef	GetTargetContext() const { return mxTargetContext; }
	CGContextRef	GetMaskContext() const { return (mbIsEnabled ? mxMaskContext : NULL); }

private:
	CGLayerRef		mxTargetLayer;
	CGContextRef	mxTargetContext;
	CGContextRef	mxMaskContext;
	CGContextRef	mxTempContext;
	sal_uLong*			mpMaskBuffer;
	sal_uLong*			mpTempBuffer;
	int				mnBufferLongs;
	bool			mbIsEnabled;
};


// --- some trivial inlines

inline void AquaSalGraphics::RefreshRect( const CGRect& rRect )
{
	RefreshRect( rRect.origin.x, rRect.origin.y, rRect.size.width, rRect.size.height );
}

inline void AquaSalGraphics::RefreshRect( const NSRect& rRect )
{
	RefreshRect( rRect.origin.x, rRect.origin.y, rRect.size.width, rRect.size.height );
}

inline RGBAColor::RGBAColor( SalColor nSalColor )
:	mfRed( SALCOLOR_RED(nSalColor) * (1.0/255))
,	mfGreen( SALCOLOR_GREEN(nSalColor) * (1.0/255))
,	mfBlue( SALCOLOR_BLUE(nSalColor) * (1.0/255))
,	mfAlpha( 1.0 )	// opaque
{}

inline RGBAColor::RGBAColor( float fRed, float fGreen, float fBlue, float fAlpha )
:	mfRed( fRed )
,	mfGreen( fGreen )
,	mfBlue( fBlue )
,	mfAlpha( fAlpha )
{}

#endif // _SV_SALGDI_H
