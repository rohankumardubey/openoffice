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



#ifndef _SVP_PSPGRAPHICS_HXX
#define _SVP_PSPGRAPHICS_HXX


#include "vcl/fontmanager.hxx"

#include "sallayout.hxx"
#include "salgdi.hxx"

namespace psp { struct JobData; class PrinterGfx; }

class ServerFont;
class ImplDevFontAttributes;
class SalInfoPrinter;

class PspGraphics : public SalGraphics
{
    psp::JobData*				m_pJobData;
    psp::PrinterGfx*			m_pPrinterGfx;
    String*						m_pPhoneNr;
    bool						m_bSwallowFaxNo;
    String						m_aPhoneCollection;
    bool						m_bPhoneCollectionActive;

    ServerFont*					m_pServerFont[ MAX_FALLBACK ];
    bool						m_bFontVertical;
    SalInfoPrinter*             m_pInfoPrinter;

protected:
    virtual bool drawAlphaBitmap( const SalTwoRect&, const SalBitmap& rSourceBitmap, const SalBitmap& rAlphaBitmap );
    virtual bool drawAlphaRect( long nX, long nY, long nWidth, long nHeight, sal_uInt8 nTransparency );

public:
    PspGraphics( psp::JobData* pJob, psp::PrinterGfx* pGfx, String* pPhone, bool bSwallow, SalInfoPrinter* pInfoPrinter )
            : m_pJobData( pJob ),
              m_pPrinterGfx( pGfx ),
              m_pPhoneNr( pPhone ),
              m_bSwallowFaxNo( bSwallow ),
              m_bPhoneCollectionActive( false ),
              m_bFontVertical( false ),
              m_pInfoPrinter( pInfoPrinter )
    { for( int i = 0; i < MAX_FALLBACK; i++ ) m_pServerFont[i] = 0; }
    virtual ~PspGraphics();

    // helper methods for sharing with X11SalGraphics
    static const void* DoGetEmbedFontData( psp::fontID aFont, const sal_Ucs* pUnicodes, sal_Int32* pWidths, FontSubsetInfo& rInfo, long* pDataLen );
    static void DoFreeEmbedFontData( const void* pData, long nLen );
    static const Ucs2SIntMap* DoGetFontEncodingVector( psp::fontID aFont, const Ucs2OStrMap** pNonEncoded );
    static void DoGetGlyphWidths( psp::fontID aFont,
                                  bool bVertical,
                                  Int32Vector& rWidths,
                                  Ucs2UIntMap& rUnicodeEnc );
    static ImplDevFontAttributes Info2DevFontAttributes( const psp::FastPrintFontInfo& );
    static void AnnounceFonts( ImplDevFontList*, const psp::FastPrintFontInfo& );
    static FontWidth	ToFontWidth (psp::width::type eWidth);
    static FontWeight	ToFontWeight (psp::weight::type eWeight);
    static FontPitch	ToFontPitch (psp::pitch::type ePitch);
    static FontItalic	ToFontItalic (psp::italic::type eItalic);
    static FontFamily	ToFontFamily (psp::family::type eFamily);

    // overload all pure virtual methods
    virtual void			GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY );
    virtual sal_uInt16			GetBitCount();
    virtual long			GetGraphicsWidth() const;

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
    virtual sal_uInt16          SetFont( ImplFontSelectData*, int nFallbackLevel );
    virtual void			GetFontMetric( ImplFontMetricData*, int nFallbackLevel );
    virtual sal_uLong			GetKernPairs( sal_uLong nPairs, ImplKernPairData* pKernPairs );
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
    virtual void			drawPolyLine( sal_uLong nPoints, const SalPoint* pPtAry );
    virtual void			drawPolygon( sal_uLong nPoints, const SalPoint* pPtAry );
    virtual bool            drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double fTransparency );
    virtual bool            drawPolyLine( 
        const ::basegfx::B2DPolygon&, 
        double fTransparency, 
        const ::basegfx::B2DVector& rLineWidths, 
        basegfx::B2DLineJoin,
        com::sun::star::drawing::LineCap);
    virtual void			drawPolyPolygon( sal_uInt32 nPoly,
                                             const sal_uInt32* pPoints,
                                             PCONSTSALPOINT* pPtAry );
    virtual sal_Bool		drawPolyLineBezier( sal_uLong nPoints,
                                                const SalPoint* pPtAry,
                                                const sal_uInt8* pFlgAry );
    virtual sal_Bool		drawPolygonBezier( sal_uLong nPoints,
                                               const SalPoint* pPtAry,
                                               const sal_uInt8* pFlgAry );
    virtual sal_Bool		drawPolyPolygonBezier( sal_uInt32 nPoly,
                                                   const sal_uInt32* pPoints,
                                                   const SalPoint* const* pPtAry,
                                                   const sal_uInt8* const* pFlgAry );

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
                                        const SalBitmap& rTransparentBitmap );
    virtual void			drawMask( const SalTwoRect* pPosAry,
                                      const SalBitmap& rSalBitmap,
                                      SalColor nMaskColor );
    virtual SalBitmap*		getBitmap( long nX, long nY, long nWidth, long nHeight );
    virtual SalColor		getPixel( long nX, long nY );
    virtual void			invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags );
    virtual void			invert( sal_uLong nPoints, const SalPoint* pPtAry, SalInvert nFlags );

    virtual sal_Bool			drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize );
    virtual bool            filterText( const String& rOrigText, String& rNewText, xub_StrLen nIndex, xub_StrLen& rLen, xub_StrLen& rCutStart, xub_StrLen& rCutStop );
    
    virtual SystemGraphicsData 		GetGraphicsData() const;
    virtual SystemFontData          GetSysFontData( int nFallbacklevel ) const;
};

#endif // _SVP_PSPGRAPHICS_HXX

