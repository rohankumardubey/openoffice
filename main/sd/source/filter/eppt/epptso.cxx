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
#include "precompiled_sd.hxx"
#include <osl/endian.h>
#include <eppt.hxx>
#include "epptdef.hxx"
#ifndef _PptEscherEx_HXX
#include "escherex.hxx"
#endif
#include <tools/poly.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/gradient.hxx>
#include <vcl/gfxlink.hxx>
#include <tools/stream.hxx>
#include <sot/storage.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gradient.hxx>
#include <sfx2/app.hxx>
#include <svl/languageoptions.hxx>
//#ifndef _SVX_XIT_HXX
//#include <svx/xit.hxx>
//#endif
#include <editeng/svxenum.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdoashp.hxx>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/awt/XFont.hpp>
#ifndef _COM_SUN_STAR_AWT_XFONTWEIGHT_HPP_
#include <com/sun/star/awt/FontWeight.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_PP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/FlagSequence.hpp>
#include <com/sun/star/drawing/PolygonFlags.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/XScriptTypeDetector.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/ScriptDirection.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <vcl/cvtgrf.hxx>
#include <tools/urlobj.hxx>
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _CPPUHELPER_PROPTYPEHLP_HXX_
#include <cppuhelper/proptypehlp.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX_
#include <ucbhelper/content.hxx>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX_
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/unohlp.hxx>
#endif
#include <rtl/crc.h>
#include <sot/clsids.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <com/sun/star/text/FontRelief.hpp>
#include <editeng/frmdiritem.hxx>
/*
#include <editeng/outliner.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdmodel.hxx>
*/
#include <svtools/fltcall.hxx>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <set>

//#include <svx/xbtmpit.hxx>

#include "i18npool/mslangid.hxx"

#include <vos/xception.hxx>
using namespace vos;

using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////////////////////////////

#define ANSI_CHARSET            0
#define DEFAULT_CHARSET         1
#define SYMBOL_CHARSET          2
#define SHIFTJIS_CHARSET        128
#define HANGEUL_CHARSET         129
#define CHINESEBIG5_CHARSET     136
#define OEM_CHARSET             255

////////////////////////////////////////////////////////////////////////////////////////////////////

/* Font Families */
#define FF_DONTCARE             0x00
#define FF_ROMAN                0x10
#define FF_SWISS                0x20
#define FF_MODERN               0x30
#define FF_SCRIPT               0x40
#define FF_DECORATIVE           0x50

////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEFAULT_PITCH           0x00
#define FIXED_PITCH             0x01
#define VARIABLE_PITCH          0x02

// ---------------------------------------------------------------------------------------------

com::sun::star::uno::Reference< com::sun::star::i18n::XBreakIterator > xPPTBreakIter;
com::sun::star::uno::Reference< com::sun::star::i18n::XScriptTypeDetector > xScriptTypeDetector;

PPTExBulletProvider::PPTExBulletProvider()
{
    pGraphicProv = new EscherGraphicProvider( _E_GRAPH_PROV_USE_INSTANCES  | _E_GRAPH_PROV_DO_NOT_ROTATE_METAFILES );
}

PPTExBulletProvider::~PPTExBulletProvider()
{
    delete pGraphicProv;
}

sal_uInt16 PPTExBulletProvider::GetId( const ByteString& rUniqueId, Size& rGraphicSize )
{
    sal_uInt16 nRetValue = 0xffff;
    sal_uInt32 nId = 0;

    if ( rUniqueId.Len() )
    {
		Rectangle       aRect;
        GraphicObject   aGraphicObject( rUniqueId );
        Graphic         aMappedGraphic, aGraphic( aGraphicObject.GetGraphic() );
        Size            aPrefSize( aGraphic.GetPrefSize() );
        BitmapEx        aBmpEx( aGraphic.GetBitmapEx() );

		if ( rGraphicSize.Width() && rGraphicSize.Height() )
		{
			double          fQ1 = ( (double)aPrefSize.Width() / (double)aPrefSize.Height() );
			double          fQ2 = ( (double)rGraphicSize.Width() / (double)rGraphicSize.Height() );
			double          fXScale = 1;
			double          fYScale = 1;

			if ( fQ1 > fQ2 )
				fYScale = fQ1 / fQ2;
			else if ( fQ1 < fQ2 )
				fXScale = fQ2 / fQ1;

			if ( ( fXScale != 1.0 ) || ( fYScale != 1.0 ) )
			{
				aBmpEx.Scale( fXScale, fYScale );
				Size aNewSize( (sal_Int32)((double)rGraphicSize.Width() / fXScale + 0.5 ),
								(sal_Int32)((double)rGraphicSize.Height() / fYScale + 0.5 ) );

				rGraphicSize = aNewSize;

				aMappedGraphic = Graphic( aBmpEx );
				aGraphicObject = GraphicObject( aMappedGraphic );
			}
		}

		nId = pGraphicProv->GetBlibID( aBuExPictureStream, aGraphicObject.GetUniqueID(), aRect, NULL, NULL );

        if ( nId && ( nId < 0x10000 ) )
            nRetValue = (sal_uInt16)nId - 1;
    }
    return nRetValue;
}

// ---------------------------------------------------------------------------------------------

GroupTable::GroupTable() :
    mnCurrentGroupEntry ( 0 ),
    mnMaxGroupEntry     ( 0 ),
    mnGroupsClosed      ( 0 ),
    mpGroupEntry        ( NULL )
{
    ImplResizeGroupTable( 32 );
}

// ---------------------------------------------------------------------------------------------

GroupTable::~GroupTable()
{
    for ( sal_uInt32 i = 0; i < mnCurrentGroupEntry; delete mpGroupEntry[ i++ ] ) ;
    delete[] mpGroupEntry;
}

// ---------------------------------------------------------------------------------------------

void GroupTable::ImplResizeGroupTable( sal_uInt32 nEntrys )
{
    if ( nEntrys > mnMaxGroupEntry )
    {
        mnMaxGroupEntry         = nEntrys;
        GroupEntry** pTemp = new GroupEntry*[ nEntrys ];
        for ( sal_uInt32 i = 0; i < mnCurrentGroupEntry; i++ )
            pTemp[ i ] = mpGroupEntry[ i ];
        if ( mpGroupEntry )
            delete[] mpGroupEntry;
        mpGroupEntry = pTemp;
    }
}

// ---------------------------------------------------------------------------------------------

sal_Bool GroupTable::EnterGroup( ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rXIndexAccessRef )
{
    sal_Bool bRet = sal_False;
    if ( rXIndexAccessRef.is() )
    {
        GroupEntry* pNewGroup = new GroupEntry( rXIndexAccessRef );
        if ( pNewGroup->mnCount )
        {
            if ( mnMaxGroupEntry == mnCurrentGroupEntry )
                ImplResizeGroupTable( mnMaxGroupEntry + 8 );
            mpGroupEntry[ mnCurrentGroupEntry++ ] = pNewGroup;
            bRet = sal_True;
        }
        else
            delete pNewGroup;
    }
    return bRet;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 GroupTable::GetGroupsClosed()
{
    sal_uInt32 nRet = mnGroupsClosed;
    mnGroupsClosed = 0;
    return nRet;
}

// ---------------------------------------------------------------------------------------------

void GroupTable::ClearGroupTable()
{
    for ( sal_uInt32 i = 0; i < mnCurrentGroupEntry; i++, delete mpGroupEntry[ i ] ) ;
    mnCurrentGroupEntry = 0;
}

// ---------------------------------------------------------------------------------------------

void GroupTable::ResetGroupTable( sal_uInt32 nCount )
{
    ClearGroupTable();
    mpGroupEntry[ mnCurrentGroupEntry++ ] = new GroupEntry( nCount );
}

// ---------------------------------------------------------------------------------------------

sal_Bool GroupTable::GetNextGroupEntry()
{
    while ( mnCurrentGroupEntry )
    {
        mnIndex = mpGroupEntry[ mnCurrentGroupEntry - 1 ]->mnCurrentPos++;

        if ( mpGroupEntry[ mnCurrentGroupEntry - 1 ]->mnCount > mnIndex )
            return sal_True;

        delete ( mpGroupEntry[ --mnCurrentGroupEntry ] );

        if ( mnCurrentGroupEntry )
            mnGroupsClosed++;
    }
    return sal_False;
}

// ---------------------------------------------------------------------------------------------

FontCollectionEntry::~FontCollectionEntry()
{
}

// ---------------------------------------------------------------------------------------------

void FontCollectionEntry::ImplInit( const String& rName )
{
    String aSubstName( GetSubsFontName( rName, SUBSFONT_ONLYONE | SUBSFONT_MS ) );
    if ( aSubstName.Len() )
    {
        Name = aSubstName;
        bIsConverted = sal_True;
    }
    else
    {
        Name = rName;
        bIsConverted = sal_False;
    }
}

FontCollection::~FontCollection()
{
    for( void* pStr = List::First(); pStr; pStr = List::Next() )
        delete (FontCollectionEntry*)pStr;
    delete pVDev;
	xPPTBreakIter = NULL;
	xScriptTypeDetector = NULL;
}

FontCollection::FontCollection() :
    pVDev ( NULL )
{
	com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
		xMSF = ::comphelper::getProcessServiceFactory();
	com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
		xInterface = xMSF->createInstance( rtl::OUString::createFromAscii( "com.sun.star.i18n.BreakIterator" ) );
	if ( xInterface.is() )
		xPPTBreakIter = com::sun::star::uno::Reference< com::sun::star::i18n::XBreakIterator >
			( xInterface, com::sun::star::uno::UNO_QUERY );

	xInterface = xMSF->createInstance( rtl::OUString::createFromAscii( "com.sun.star.i18n.ScriptTypeDetector" ) );
	if ( xInterface.is() )
		xScriptTypeDetector = com::sun::star::uno::Reference< com::sun::star::i18n::XScriptTypeDetector >
			( xInterface, com::sun::star::uno::UNO_QUERY );
}

short FontCollection::GetScriptDirection( const String& rString ) const
{
	short nRet = com::sun::star::i18n::ScriptDirection::NEUTRAL;
	if ( xScriptTypeDetector.is() )
	{
		const rtl::OUString sT( rString );
		nRet = xScriptTypeDetector->getScriptDirection( sT, 0, com::sun::star::i18n::ScriptDirection::NEUTRAL );
	}
	return nRet;
}

sal_uInt32 FontCollection::GetId( FontCollectionEntry& rEntry )
{
    if( rEntry.Name.Len() )
    {
        const sal_uInt32 nFonts = GetCount();

        for( sal_uInt32 i = 0; i < nFonts; i++ )
        {
            const FontCollectionEntry* pEntry = GetById( i );
            if( pEntry->Name == rEntry.Name )
                return i;
        }
		Font aFont;
		aFont.SetCharSet( rEntry.CharSet );
		aFont.SetName( rEntry.Original );
//		aFont.SetFamily( rEntry.Family );
//		aFont.SetPitch( rEntry.Pitch );
        aFont.SetHeight( 100 );

        if ( !pVDev )
            pVDev = new VirtualDevice;

        pVDev->SetFont( aFont );
		FontMetric aMetric( pVDev->GetFontMetric() );

        sal_uInt16 nTxtHeight = (sal_uInt16)aMetric.GetAscent() + (sal_uInt16)aMetric.GetDescent();

        if ( nTxtHeight )
        {
		    double fScaling = (double)nTxtHeight / 120.0;
            if ( ( fScaling > 0.50 ) && ( fScaling < 1.5 ) )
                rEntry.Scaling = fScaling;
        }

        List::Insert( new FontCollectionEntry( rEntry ), LIST_APPEND );
        return nFonts;
    }
    return 0;
}

const FontCollectionEntry* FontCollection::GetById( sal_uInt32 nId )
{
    return (FontCollectionEntry*)List::GetObject( nId );
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplVBAInfoContainer( SvStream* pStrm )
{
    sal_uInt32 nSize = 28;
    if ( pStrm )
    {
        *pStrm << (sal_uInt32)( 0x1f | ( EPP_VBAInfo << 16 ) )
               << (sal_uInt32)( nSize - 8 )
               << (sal_uInt32)( 2 | ( EPP_VBAInfoAtom << 16 ) )
               << (sal_uInt32)12;
        mpPptEscherEx->InsertPersistOffset( EPP_Persist_VBAInfoAtom, pStrm->Tell() );
        *pStrm << (sal_uInt32)0
               << (sal_uInt32)0
               << (sal_uInt32)1;
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplSlideViewInfoContainer( sal_uInt32 nInstance, SvStream* pStrm )
{
    sal_uInt32 nSize = 111;
    if ( pStrm )
    {
        sal_uInt8 bShowGuides = 0;
        sal_uInt8 bSnapToGrid = 1;
        sal_uInt8 bSnapToShape = 0;

        sal_Int32 nScaling = 85;
        sal_Int32 nMasterCoordinate = 0xdda;
        sal_Int32 nXOrigin = -780;
        sal_Int32 nYOrigin = -84;

        sal_Int32 nPosition1 = 0x870;
        sal_Int32 nPosition2 = 0xb40;

        if ( nInstance )
        {
            bShowGuides = 1;
            nScaling = 0x3b;
            nMasterCoordinate = 0xf0c;
            nXOrigin = -1752;
            nYOrigin = -72;
            nPosition1 = 0xb40;
            nPosition2 = 0x870;
        }
        *pStrm << (sal_uInt32)( 0xf | ( EPP_SlideViewInfo << 16 ) | ( nInstance << 4 ) )
               << (sal_uInt32)( nSize - 8 )
               << (sal_uInt32)( EPP_SlideViewInfoAtom << 16 ) << (sal_uInt32)3
               << bShowGuides << bSnapToGrid << bSnapToShape
               << (sal_uInt32)( EPP_ViewInfoAtom << 16 ) << (sal_uInt32)52
               << nScaling << (sal_Int32)100 << nScaling << (sal_Int32)100  // scaling atom - Keeps the current scale
               << nScaling << (sal_Int32)100 << nScaling << (sal_Int32)100  // scaling atom - Keeps the previous scale
               << (sal_Int32)0x17ac << nMasterCoordinate// Origin - Keeps the origin in master coordinates
               << nXOrigin << nYOrigin              // Origin
               << (sal_uInt8)1                          // Bool1 varScale - Set if zoom to fit is set
               << (sal_uInt8)0                          // bool1 draftMode - Not used
               << (sal_uInt16)0                         // padword
               << (sal_uInt32)( ( 7 << 4 ) | ( EPP_GuideAtom << 16 ) ) << (sal_uInt32)8
               << (sal_uInt32)0     // Type of the guide. If the guide is horizontal this value is zero. If it's vertical, it's one.
               << nPosition1    // Position of the guide in master coordinates. X coordinate if it's vertical, and Y coordinate if it's horizontal.
               << (sal_uInt32)( ( 7 << 4 ) | ( EPP_GuideAtom << 16 ) ) << (sal_uInt32)8
               << (sal_Int32)1      // Type of the guide. If the guide is horizontal this value is zero. If it's vertical, it's one.
               << nPosition2;   // Position of the guide in master coordinates. X coordinate if it's vertical, and Y coordinate if it's horizontal.
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplOutlineViewInfoContainer( SvStream* pStrm )
{
    sal_uInt32 nSize = 68;
    if ( pStrm )
    {
        *pStrm << (sal_uInt32)( 0xf | ( EPP_OutlineViewInfo << 16 ) ) << (sal_uInt32)( nSize - 8 )
               << (sal_uInt32)( EPP_ViewInfoAtom << 16 ) << (sal_uInt32)52
               << (sal_Int32)170 << (sal_Int32)200 << (sal_Int32)170 << (sal_Int32)200  // scaling atom - Keeps the current scale
               << (sal_Int32)170 << (sal_Int32)200 << (sal_Int32)170 << (sal_Int32)200  // scaling atom - Keeps the previous scale
               << (sal_Int32)0x17ac << 0xdda    // Origin - Keeps the origin in master coordinates
               << (sal_Int32)-780 << (sal_Int32)-84 // Origin
               << (sal_uInt8)1                  // bool1 varScale - Set if zoom to fit is set
               << (sal_uInt8)0                  // bool1 draftMode - Not used
               << (sal_uInt16)0;                // padword
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplProgBinaryTag( SvStream* pStrm )
{
    sal_uInt32 nPictureStreamSize, nOutlineStreamSize, nSize = 8;

    nPictureStreamSize = aBuExPictureStream.Tell();
    if ( nPictureStreamSize )
        nSize += nPictureStreamSize + 8;

    nOutlineStreamSize = aBuExOutlineStream.Tell();
    if ( nOutlineStreamSize )
        nSize += nOutlineStreamSize + 8;

    if ( pStrm )
    {
        *pStrm << (sal_uInt32)( EPP_BinaryTagData << 16 ) << (sal_uInt32)( nSize - 8 );
        if ( nPictureStreamSize )
        {
            *pStrm << (sal_uInt32)( 0xf | ( EPP_PST_ExtendedBuGraContainer << 16 ) ) << nPictureStreamSize;
            pStrm->Write( aBuExPictureStream.GetData(), nPictureStreamSize );
        }
        if ( nOutlineStreamSize )
        {
            *pStrm << (sal_uInt32)( 0xf | ( EPP_PST_ExtendedPresRuleContainer << 16 ) ) << nOutlineStreamSize;
            pStrm->Write( aBuExOutlineStream.GetData(), nOutlineStreamSize );
        }
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplProgBinaryTagContainer( SvStream* pStrm, SvMemoryStream* pBinTagStrm )
{
    sal_uInt32 nSize = 8 + 8 + 14;
    if ( pStrm )
    {
        *pStrm << (sal_uInt32)( 0xf | ( EPP_ProgBinaryTag << 16 ) ) << (sal_uInt32)0
               << (sal_uInt32)( EPP_CString << 16 ) << (sal_uInt32)14
               << (sal_uInt32)0x5f005f << (sal_uInt32)0x50005f
               << (sal_uInt32)0x540050 << (sal_uInt16)0x39;
    }
    if ( pBinTagStrm )
    {
        sal_uInt32 nLen = pBinTagStrm->Tell();
        nSize += nLen + 8;
        *pStrm << (sal_uInt32)( EPP_BinaryTagData << 16 ) << nLen;
        pStrm->Write( pBinTagStrm->GetData(), nLen );
    }
    else
        nSize += ImplProgBinaryTag( pStrm );

    if ( pStrm )
    {
        pStrm->SeekRel( - ( (sal_Int32)nSize - 4 ) );
        *pStrm << (sal_uInt32)( nSize - 8 );
        pStrm->SeekRel( nSize - 8 );
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplProgTagContainer( SvStream* pStrm, SvMemoryStream* pBinTagStrm )
{
    sal_uInt32 nSize = 0;
    if ( aBuExPictureStream.Tell() || aBuExOutlineStream.Tell() || pBinTagStrm )
    {
        nSize = 8;
        if ( pStrm )
        {
            *pStrm << (sal_uInt32)( 0xf | ( EPP_ProgTags << 16 ) ) << (sal_uInt32)0;
        }
        nSize += ImplProgBinaryTagContainer( pStrm, pBinTagStrm );
        if ( pStrm )
        {
            pStrm->SeekRel( - ( (sal_Int32)nSize - 4 ) );
            *pStrm << (sal_uInt32)( nSize - 8 );
            pStrm->SeekRel( nSize - 8 );
        }
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplDocumentListContainer( SvStream* pStrm )
{
    sal_uInt32 nSize = 8;
    if ( pStrm )
    {
        *pStrm << (sal_uInt32)( ( EPP_List << 16 ) | 0xf ) << (sal_uInt32)0;
    }

    nSize += ImplVBAInfoContainer( pStrm );
    nSize += ImplSlideViewInfoContainer( 0, pStrm );
    nSize += ImplOutlineViewInfoContainer( pStrm );
    nSize += ImplSlideViewInfoContainer( 1, pStrm );
    nSize += ImplProgTagContainer( pStrm );

    if ( pStrm )
    {
        pStrm->SeekRel( - ( (sal_Int32)nSize - 4 ) );
        *pStrm << (sal_uInt32)( nSize - 8 );
        pStrm->SeekRel( nSize - 8 );
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplMasterSlideListContainer( SvStream* pStrm )
{
    sal_uInt32 i, nSize = 28 * mnMasterPages + 8;
    if ( pStrm )
    {
        *pStrm << (sal_uInt32)( 0x1f | ( EPP_SlideListWithText << 16 ) ) << (sal_uInt32)( nSize - 8 );

        for ( i = 0; i < mnMasterPages; i++ )
        {
            *pStrm << (sal_uInt32)( EPP_SlidePersistAtom << 16 ) << (sal_uInt32)20;
            mpPptEscherEx->InsertPersistOffset( EPP_MAINMASTER_PERSIST_KEY | i, pStrm->Tell() );
            *pStrm << (sal_uInt32)0                 // psrReference - logical reference to the slide persist object ( EPP_MAINMASTER_PERSIST_KEY )
                   << (sal_uInt32)0                 // flags - only bit 3 used, if set then slide contains shapes other than placeholders
                   << (sal_Int32)0                  // numberTexts - number of placeholder texts stored with the persist object. Allows to display outline view without loading the slide persist objects
                   << (sal_Int32)( 0x80000000 | i ) // slideId - Unique slide identifier, used for OLE link monikers for example
                   << (sal_uInt32)0;                // reserved, usualy 0
        }
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplInsertBookmarkURL( const String& rBookmarkURL, const sal_uInt32 nType,
	const String& rStringVer0, const String& rStringVer1, const String& rStringVer2, const String& rStringVer3 )
{
    sal_uInt32 nHyperId = ++mnExEmbed;

	rtl::OUString sBookmarkURL( rBookmarkURL );
	INetURLObject aBaseURI( maBaseURI );
	INetURLObject aBookmarkURI( rBookmarkURL );
	if( aBaseURI.GetProtocol() == aBookmarkURI.GetProtocol() )
	{
		rtl::OUString aRelUrl( INetURLObject::GetRelURL( maBaseURI, rBookmarkURL, 
			INetURLObject::WAS_ENCODED, INetURLObject::DECODE_TO_IURI, RTL_TEXTENCODING_UTF8, INetURLObject::FSYS_DETECT ) );
		if ( aRelUrl.getLength() )
			sBookmarkURL = aRelUrl;
	}
    maHyperlink.Insert( new EPPTHyperlink( sBookmarkURL, nType ), LIST_APPEND );

    *mpExEmbed  << (sal_uInt16)0xf
                << (sal_uInt16)EPP_ExHyperlink
                << (sal_uInt32)0;
	sal_uInt32 nHyperSize, nHyperStart = mpExEmbed->Tell();
    *mpExEmbed  << (sal_uInt16)0
                << (sal_uInt16)EPP_ExHyperlinkAtom
                << (sal_uInt32)4
                << nHyperId;

	sal_uInt16 i, nStringLen;
	nStringLen = rStringVer0.Len();
	if ( nStringLen )
	{
		*mpExEmbed << (sal_uInt32)( EPP_CString << 16 ) << (sal_uInt32)( nStringLen * 2 );
		for ( i = 0; i < nStringLen; i++ )
		{
			*mpExEmbed << rStringVer0.GetChar( i );
		}
	}
	nStringLen = rStringVer1.Len();
	if ( nStringLen )
	{
		*mpExEmbed << (sal_uInt32)( ( EPP_CString << 16 ) | 0x10 ) << (sal_uInt32)( nStringLen * 2 );
		for ( i = 0; i < nStringLen; i++ )
		{
			*mpExEmbed << rStringVer1.GetChar( i );
		}
	}
	nStringLen = rStringVer2.Len();
	if ( nStringLen )
	{
		*mpExEmbed << (sal_uInt32)( ( EPP_CString << 16 ) | 0x20 ) << (sal_uInt32)( nStringLen * 2 );
		for ( i = 0; i < nStringLen; i++ )
		{
			*mpExEmbed << rStringVer2.GetChar( i );
		}
	}
	nStringLen = rStringVer3.Len();
	if ( nStringLen )
	{
		*mpExEmbed << (sal_uInt32)( ( EPP_CString << 16 ) | 0x30 ) << (sal_uInt32)( nStringLen * 2 );
		for ( i = 0; i < nStringLen; i++ )
		{
			*mpExEmbed << rStringVer3.GetChar( i );
		}
	}
	nHyperSize = mpExEmbed->Tell() - nHyperStart;
	mpExEmbed->SeekRel( - ( (sal_Int32)nHyperSize + 4 ) );
	*mpExEmbed  << nHyperSize;
	mpExEmbed->SeekRel( nHyperSize );
	return nHyperId;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCloseDocument()
{
    sal_uInt32 nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_Document );
    if ( nOfs )
    {
        mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_CurrentPos, mpStrm->Tell() );
        mpStrm->Seek( nOfs );

		// creating the TxMasterStyleAtom
		SvMemoryStream aTxMasterStyleAtomStrm( 0x200, 0x200 );
		{
			EscherExAtom aTxMasterStyleAtom( aTxMasterStyleAtomStrm, EPP_TxMasterStyleAtom, EPP_TEXTTYPE_Other );
			aTxMasterStyleAtomStrm << (sal_uInt16)5;		// paragraph count
			sal_uInt16 nLev;
			sal_Bool bFirst = sal_True;
			for ( nLev = 0; nLev < 5; nLev++ )
			{
				mpStyleSheet->mpParaSheet[ EPP_TEXTTYPE_Other ]->Write( aTxMasterStyleAtomStrm, mpPptEscherEx, nLev, bFirst, sal_False, mXPagePropSet );
				mpStyleSheet->mpCharSheet[ EPP_TEXTTYPE_Other ]->Write( aTxMasterStyleAtomStrm, mpPptEscherEx, nLev, bFirst, sal_False, mXPagePropSet );
				bFirst = sal_False;
			}
        }

        mpExEmbed->Seek( STREAM_SEEK_TO_END );
        sal_uInt32 nExEmbedSize = mpExEmbed->Tell();

        // nEnviroment : Gesamtgroesse des Environment Containers
        sal_uInt32 nEnvironment = maFontCollection.GetCount() * 76      // 68 bytes pro Fontenityatom und je 8 Bytes fuer die Header
                                + 8                                     // 1 FontCollection Container
                                + 20                                    // SrKinsoku Container
                                + 18                                    // 1 TxSiStyleAtom
                                + aTxMasterStyleAtomStrm.Tell()			// 1 TxMasterStyleAtom;
                                + mpStyleSheet->SizeOfTxCFStyleAtom();

        sal_uInt32 nBytesToInsert = nEnvironment + 8;

        if ( nExEmbedSize )
            nBytesToInsert += nExEmbedSize + 8 + 12;

        nBytesToInsert += maSoundCollection.GetSize();
        nBytesToInsert += mpPptEscherEx->DrawingGroupContainerSize();
        nBytesToInsert += ImplMasterSlideListContainer( NULL );
        nBytesToInsert += ImplDocumentListContainer( NULL );

        // nBytes im Stream einfuegen, und abhaengige Container anpassen
        mpPptEscherEx->InsertAtCurrentPos( nBytesToInsert, false );

        // CREATE HYPERLINK CONTAINER
        if ( nExEmbedSize )
        {
            *mpStrm << (sal_uInt16)0xf
                    << (sal_uInt16)EPP_ExObjList
                    << (sal_uInt32)( nExEmbedSize + 12 )
                    << (sal_uInt16)0
                    << (sal_uInt16)EPP_ExObjListAtom
                    << (sal_uInt32)4
                    << (sal_uInt32)mnExEmbed;
            mpPptEscherEx->InsertPersistOffset( EPP_Persist_ExObj, mpStrm->Tell() );
            mpStrm->Write( mpExEmbed->GetData(), nExEmbedSize );
        }

        // CREATE ENVIRONMENT
        *mpStrm << (sal_uInt16)0xf << (sal_uInt16)EPP_Environment << (sal_uInt32)nEnvironment;

        // Open Container ( EPP_SrKinsoku )
        *mpStrm << (sal_uInt16)0x2f << (sal_uInt16)EPP_SrKinsoku << (sal_uInt32)12;
        mpPptEscherEx->AddAtom( 4, EPP_SrKinsokuAtom, 0, 3 );
        *mpStrm << (sal_Int32)0;                        // SrKinsoku Level 0

        // Open Container ( EPP_FontCollection )
        *mpStrm << (sal_uInt16)0xf << (sal_uInt16)EPP_FontCollection << (sal_uInt32)maFontCollection.GetCount() * 76;

        for ( sal_uInt32 i = 0; i < maFontCollection.GetCount(); i++ )
        {
            mpPptEscherEx->AddAtom( 68, EPP_FontEnityAtom, 0, i );
            const FontCollectionEntry* pDesc = maFontCollection.GetById( i );
            sal_uInt32 nFontLen = pDesc->Name.Len();
            if ( nFontLen > 31 )
                nFontLen = 31;
            for ( sal_uInt16 n = 0; n < 32; n++ )
            {
                sal_Unicode nUniCode = 0;
                if ( n < nFontLen )
                    nUniCode = pDesc->Name.GetChar( n );
                *mpStrm << nUniCode;
            }
            sal_uInt8   lfCharSet = ANSI_CHARSET;
            sal_uInt8   lfClipPrecision = 0;
            sal_uInt8   lfQuality = 6;
            sal_uInt8   lfPitchAndFamily = 0;

            if ( pDesc->CharSet == RTL_TEXTENCODING_SYMBOL )
                lfCharSet = SYMBOL_CHARSET;

            switch( pDesc->Family )
            {
                case ::com::sun::star::awt::FontFamily::ROMAN :
                    lfPitchAndFamily |= FF_ROMAN;
                break;

                case ::com::sun::star::awt::FontFamily::SWISS :
                    lfPitchAndFamily |= FF_SWISS;
                break;

                case ::com::sun::star::awt::FontFamily::MODERN :
                    lfPitchAndFamily |= FF_MODERN;
                break;

                case ::com::sun::star::awt::FontFamily::SCRIPT:
                    lfPitchAndFamily |= FF_SCRIPT;
                break;

                case ::com::sun::star::awt::FontFamily::DECORATIVE:
                     lfPitchAndFamily |= FF_DECORATIVE;
                break;

                default:
                    lfPitchAndFamily |= FAMILY_DONTKNOW;
                break;
            }
            switch( pDesc->Pitch )
            {
                case ::com::sun::star::awt::FontPitch::FIXED:
                    lfPitchAndFamily |= FIXED_PITCH;
                break;

                default:
                    lfPitchAndFamily |= DEFAULT_PITCH;
                break;
            }
            *mpStrm << lfCharSet
                    << lfClipPrecision
                    << lfQuality
                    << lfPitchAndFamily;
        }
        mpStyleSheet->WriteTxCFStyleAtom( *mpStrm );        // create style that is used for new standard objects
        mpPptEscherEx->AddAtom( 10, EPP_TxSIStyleAtom );
        *mpStrm << (sal_uInt32)7                        // ?
                << (sal_Int16)2                         // ?
                << (sal_uInt8)9                         // ?
                << (sal_uInt8)8                         // ?
                << (sal_Int16)0;                        // ?

		mpStrm->Write( aTxMasterStyleAtomStrm.GetData(), aTxMasterStyleAtomStrm.Tell() );
		maSoundCollection.Write( *mpStrm );
        mpPptEscherEx->WriteDrawingGroupContainer( *mpStrm );
        ImplMasterSlideListContainer( mpStrm );
        ImplDocumentListContainer( mpStrm );

        sal_uInt32 nOldPos = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_CurrentPos );
        if ( nOldPos )
        {
            mpStrm->Seek( nOldPos );
            return sal_True;
        }
    }
    return sal_False;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PropValue::GetPropertyValue(
    ::com::sun::star::uno::Any& rAny,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
            const String& rString,
                    sal_Bool bTestPropertyAvailability )
{
    sal_Bool bRetValue = sal_True;
    if ( bTestPropertyAvailability )
    {
        bRetValue = sal_False;
        try
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                aXPropSetInfo( rXPropSet->getPropertySetInfo() );
            if ( aXPropSetInfo.is() )
                bRetValue = aXPropSetInfo->hasPropertyByName( rString );
        }
        catch( ::com::sun::star::uno::Exception& )
        {
            bRetValue = sal_False;
        }
    }
    if ( bRetValue )
    {
        try
        {
            rAny = rXPropSet->getPropertyValue( rString );
            if ( !rAny.hasValue() )
                bRetValue = sal_False;
        }
        catch( ::com::sun::star::uno::Exception& )
        {
            bRetValue = sal_False;
        }
    }
    return bRetValue;
}

// ---------------------------------------------------------------------------------------------

::com::sun::star::beans::PropertyState PropValue::GetPropertyState(
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
        const String& rPropertyName )
{
    ::com::sun::star::beans::PropertyState eRetValue = ::com::sun::star::beans::PropertyState_AMBIGUOUS_VALUE;
    try
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState > aXPropState
                ( rXPropSet, ::com::sun::star::uno::UNO_QUERY );
        if ( aXPropState.is() )
            eRetValue = aXPropState->getPropertyState( rPropertyName );
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        //...
    }
    return eRetValue;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PropValue::ImplGetPropertyValue( const String& rString )
{
    return GetPropertyValue( mAny, mXPropSet, rString );
}

// ---------------------------------------------------------------------------------------------

sal_Bool PropValue::ImplGetPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & aXPropSet, const String& rString )
{
    return GetPropertyValue( mAny, aXPropSet, rString );
}

// ---------------------------------------------------------------------------------------------

sal_Bool PropStateValue::ImplGetPropertyValue( const String& rString, sal_Bool bGetPropertyState )
{
    ePropState = ::com::sun::star::beans::PropertyState_AMBIGUOUS_VALUE;
    sal_Bool bRetValue = sal_True;
#ifdef UNX
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
            aXPropSetInfo( mXPropSet->getPropertySetInfo() );
        if ( !aXPropSetInfo.is() )
            return sal_False;
#endif
    try
    {
        mAny = mXPropSet->getPropertyValue( rString );
        if ( !mAny.hasValue() )
            bRetValue = sal_False;
        else if ( bGetPropertyState )
            ePropState = mXPropState->getPropertyState( rString );
        else
            ePropState = ::com::sun::star::beans::PropertyState_DIRECT_VALUE;
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        bRetValue = sal_False;
    }
    return bRetValue;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplInitSOIface()
{
    while( sal_True )
    {
        mXDrawPagesSupplier = ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XDrawPagesSupplier >
                ( mXModel, ::com::sun::star::uno::UNO_QUERY );
        if ( !mXDrawPagesSupplier.is() )
            break;

        mXMasterPagesSupplier = ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XMasterPagesSupplier >
                ( mXModel, ::com::sun::star::uno::UNO_QUERY );
        if ( !mXMasterPagesSupplier.is() )
            break;
        mXDrawPages = mXMasterPagesSupplier->getMasterPages();
        if ( !mXDrawPages.is() )
            break;
        mnMasterPages = mXDrawPages->getCount();
        mXDrawPages = mXDrawPagesSupplier->getDrawPages();
        if( !mXDrawPages.is() )
            break;
        mnPages =  mXDrawPages->getCount();
        if ( !ImplGetPageByIndex( 0, NORMAL ) )
            break;

        return sal_True;
    }
    return sal_False;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplSetCurrentStyleSheet( sal_uInt32 nPageNum )
{
	sal_Bool bRet = sal_False;
	if ( nPageNum >= maStyleSheetList.size() )
		nPageNum = 0;
	else
		bRet = sal_True;
	mpStyleSheet = maStyleSheetList[ nPageNum ];
	return bRet;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplGetPageByIndex( sal_uInt32 nIndex, PageType ePageType )
{
    while( sal_True )
    {
        if ( ePageType != meLatestPageType )
        {
            switch( ePageType )
            {
                case NORMAL :
                case NOTICE :
                {
                    mXDrawPages = mXDrawPagesSupplier->getDrawPages();
                    if( !mXDrawPages.is() )
                        return sal_False;
                }
                break;

                case MASTER :
                {
                    mXDrawPages = mXMasterPagesSupplier->getMasterPages();
                    if( !mXDrawPages.is() )
                        return sal_False;
                }
                break;
				default:
					break;
            }
            meLatestPageType = ePageType;
        }
        ::com::sun::star::uno::Any aAny( mXDrawPages->getByIndex( nIndex ) );
        aAny >>= mXDrawPage;
        if ( !mXDrawPage.is() )
            break;
        if ( ePageType == NOTICE )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XPresentationPage >
                aXPresentationPage( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );
            if ( !aXPresentationPage.is() )
                break;
            mXDrawPage = aXPresentationPage->getNotesPage();
            if ( !mXDrawPage.is() )
                break;
        }
        mXPagePropSet = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
            ( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );
        if ( !mXPagePropSet.is() )
            break;

        mXShapes = ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XShapes >
                ( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );
        if ( !mXShapes.is() )
            break;

		/* try to get the "real" background PropertySet. If the normal page is not supporting this property, it is
		   taken the property from the master */
		sal_Bool bHasBackground = GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Background" ) ), sal_True );
		if ( bHasBackground )
			bHasBackground = ( aAny >>= mXBackgroundPropSet );
		if ( !bHasBackground )
		{
		    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XMasterPageTarget >
				aXMasterPageTarget( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );
		    if ( aXMasterPageTarget.is() )
			{
				::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > aXMasterDrawPage;
				aXMasterDrawPage = aXMasterPageTarget->getMasterPage();
				if ( aXMasterDrawPage.is() )
				{
					::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXMasterPagePropSet;
					aXMasterPagePropSet = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
						( aXMasterDrawPage, ::com::sun::star::uno::UNO_QUERY );
					if ( aXMasterPagePropSet.is() )
					{
						sal_Bool bBackground = GetPropertyValue( aAny, aXMasterPagePropSet,
								String( RTL_CONSTASCII_USTRINGPARAM( "Background" ) ) );
						if ( bBackground )
						{
							aAny >>= mXBackgroundPropSet;
						}
					}
				}
			}
		}
        return sal_True;
    }
    return sal_False;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplGetShapeByIndex( sal_uInt32 nIndex, sal_Bool bGroup )
{
    while(sal_True)
    {
        if (  ( bGroup == sal_False ) || ( GetCurrentGroupLevel() == 0 ) )
        {
            ::com::sun::star::uno::Any aAny( mXShapes->getByIndex( nIndex ) );
            aAny >>= mXShape;
        }
        else
        {
            ::com::sun::star::uno::Any aAny( GetCurrentGroupAccess()->getByIndex( GetCurrentGroupIndex() ) );
            aAny >>= mXShape;
        }
        if ( !mXShape.is() )
            break;

        ::com::sun::star::uno::Any aAny( mXShape->queryInterface( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >*) 0 ) ));
        aAny >>= mXPropSet;

        if ( !mXPropSet.is() )
            break;
        maPosition = ImplMapPoint( mXShape->getPosition() );
        maSize = ImplMapSize( mXShape->getSize() );
        maRect = Rectangle( Point( maPosition.X, maPosition.Y ), Size( maSize.Width, maSize.Height ) );
        mType = ByteString( String( mXShape->getShapeType() ), RTL_TEXTENCODING_UTF8 );
        mType.Erase( 0, 13 );                                   // "com.sun.star." entfernen
        sal_uInt16 nPos = mType.Search( (const char*)"Shape" );
        mType.Erase( nPos, 5 );

        mbPresObj = mbEmptyPresObj = sal_False;
        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsPresentationObject" ) ) ) )
            mAny >>= mbPresObj;

        if ( mbPresObj && ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsEmptyPresentationObject" ) ) ) )
            mAny >>= mbEmptyPresObj;

        mnAngle = ( PropValue::GetPropertyValue( aAny,
            mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "RotateAngle" ) ), sal_True ) )
                ? *((sal_Int32*)aAny.getValue() )
                : 0;

        return sal_True;
    }
    return sal_False;
}

//  -----------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplGetMasterIndex( PageType ePageType )
{
    sal_uInt32 nRetValue = 0;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XMasterPageTarget >
        aXMasterPageTarget( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );

    if ( aXMasterPageTarget.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >
            aXDrawPage = aXMasterPageTarget->getMasterPage();
        if ( aXDrawPage.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                aXPropertySet( aXDrawPage, ::com::sun::star::uno::UNO_QUERY );

            if ( aXPropertySet.is() )
            {
                if ( ImplGetPropertyValue( aXPropertySet, String( RTL_CONSTASCII_USTRINGPARAM( "Number" ) ) ) )
                    nRetValue |= *(sal_Int16*)mAny.getValue();
                if ( nRetValue & 0xffff )           // ueberlauf vermeiden
                    nRetValue--;
            }
        }
    }
    if ( ePageType == NOTICE )
        nRetValue += mnMasterPages;
    return nRetValue;
}

//  -----------------------------------------------------------------------

sal_Bool PPTWriter::ImplGetStyleSheets()
{
    int             nInstance, nLevel;
    sal_Bool        bRetValue = sal_False;
	sal_uInt32		nPageNum;

	for ( nPageNum = 0; nPageNum < mnMasterPages; nPageNum++ )
	{
		::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed >
			aXNamed;

		::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
			aXNameAccess;

		::com::sun::star::uno::Reference< ::com::sun::star::style::XStyleFamiliesSupplier >
			aXStyleFamiliesSupplier( mXModel, ::com::sun::star::uno::UNO_QUERY );

		::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
			aXPropSet( mXModel, ::com::sun::star::uno::UNO_QUERY );

		sal_uInt16 nDefaultTab = ( aXPropSet.is() && ImplGetPropertyValue( aXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TabStop" ) ) ) )
			? (sal_uInt16)( *(sal_Int32*)mAny.getValue() / 4.40972 )
			: 1250;

		maStyleSheetList.push_back( new PPTExStyleSheet( nDefaultTab, (PPTExBulletProvider&)*this ) );
		ImplSetCurrentStyleSheet( nPageNum );
		if ( ImplGetPageByIndex( nPageNum, MASTER ) )
			aXNamed = ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed >
						( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );

		if ( aXStyleFamiliesSupplier.is() )
			aXNameAccess = aXStyleFamiliesSupplier->getStyleFamilies();

		bRetValue = aXNamed.is() && aXNameAccess.is() && aXStyleFamiliesSupplier.is();
		if  ( bRetValue )
		{
			for ( nInstance = EPP_TEXTTYPE_Title; nInstance <= EPP_TEXTTYPE_CenterTitle; nInstance++ )
			{
				String aStyle;
				String aFamily;
				switch ( nInstance )
				{
					case EPP_TEXTTYPE_CenterTitle :
					case EPP_TEXTTYPE_Title :
					{
						aStyle = String( RTL_CONSTASCII_USTRINGPARAM( "title" ) );
						aFamily = aXNamed->getName();
					}
					break;
					case EPP_TEXTTYPE_Body :
					{
						aStyle = String( RTL_CONSTASCII_USTRINGPARAM( "outline1" ) );      // SD_LT_SEPARATOR
						aFamily = aXNamed->getName();
					}
					break;
					case EPP_TEXTTYPE_Other :
					{
						aStyle = String( RTL_CONSTASCII_USTRINGPARAM( "standard" ) );
						aFamily = String( RTL_CONSTASCII_USTRINGPARAM( "graphics" ) );
					}
					break;
					case EPP_TEXTTYPE_CenterBody :
					{
						aStyle = String( RTL_CONSTASCII_USTRINGPARAM( "subtitle" ) );
						aFamily = aXNamed->getName();
					}
					break;
				}
				if ( aStyle.Len() && aFamily.Len() )
				{
					try
					{
						::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >xNameAccess;
						if ( aXNameAccess->hasByName( aFamily ) )
						{
							::com::sun::star::uno::Any aAny( aXNameAccess->getByName( aFamily ) );
							if( aAny.getValue() && ::cppu::extractInterface( xNameAccess, aAny ) )
							{
								::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > aXFamily;
								if ( aAny >>= aXFamily )
								{
									if ( aXFamily->hasByName( aStyle ) )
									{
										::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle > xStyle;
										aAny = aXFamily->getByName( aStyle );
										if( aAny.getValue() && ::cppu::extractInterface( xStyle, aAny ) )
										{
											::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle > aXStyle;
											aAny >>= aXStyle;
											::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
												xPropSet( aXStyle, ::com::sun::star::uno::UNO_QUERY );
											if( xPropSet.is() )
												mpStyleSheet->SetStyleSheet( xPropSet, maFontCollection, nInstance, 0 );
											for ( nLevel = 1; nLevel < 5; nLevel++ )
											{
												if ( nInstance == EPP_TEXTTYPE_Body )
												{
													sal_Unicode cTemp = aStyle.GetChar( aStyle.Len() - 1 );
													aStyle.SetChar( aStyle.Len() - 1, ++cTemp );
													if ( aXFamily->hasByName( aStyle ) )
													{
														aXFamily->getByName( aStyle ) >>= xStyle;
														if( xStyle.is() )
														{
															::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
																xPropertySet( xStyle, ::com::sun::star::uno::UNO_QUERY );
															if ( xPropertySet.is() )
																mpStyleSheet->SetStyleSheet( xPropertySet, maFontCollection, nInstance, nLevel );
														}
													}
												}
												else
													mpStyleSheet->SetStyleSheet( xPropSet, maFontCollection, nInstance, nLevel );
											}
										}
									}
								}
							}
						}
					}
					catch( ::com::sun::star::uno::Exception& )
					{
					//
					}
				}
			}
			for ( ; nInstance <= EPP_TEXTTYPE_QuarterBody; nInstance++ )
			{

			}
		}
	}
    return bRetValue;
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplWriteParagraphs( SvStream& rOut, TextObj& rTextObj )
{
    sal_Bool            bFirstParagraph = sal_True;
    sal_uInt32          nCharCount;
    sal_uInt32          nPropertyFlags = 0;
    sal_uInt16          nDepth = 0;
    sal_Int16           nLineSpacing;
    int                 nInstance = rTextObj.GetInstance();

    for ( ParagraphObj* pPara = rTextObj.First() ; pPara; pPara = rTextObj.Next(), bFirstParagraph = sal_False )
    {
        PortionObj* pPortion = (PortionObj*)pPara->First();
        nCharCount = pPara->Count();

        nDepth = pPara->nDepth;
        if ( nDepth > 4)
            nDepth = 4;

        if ( ( pPara->meTextAdjust == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
            ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, ParaAttr_Adjust, pPara->mnTextAdjust ) ) )
            nPropertyFlags |= 0x00000800;
        nLineSpacing = pPara->mnLineSpacing;

        const FontCollectionEntry* pDesc = maFontCollection.GetById( pPortion->mnFont );
        sal_Int16 nNormalSpacing = 100;
        if ( !mbFontIndependentLineSpacing && pDesc )
        {
            double fN = 100.0;
            fN *= pDesc->Scaling;
            nNormalSpacing = (sal_Int16)( fN + 0.5 );
        }
        if ( !mbFontIndependentLineSpacing && bFirstParagraph && ( nLineSpacing > nNormalSpacing ) )	// sj: i28747, no replacement for fixed linespacing
        {
            nLineSpacing = nNormalSpacing;
            nPropertyFlags |= 0x00001000;
        }
        else
        {
            if ( nLineSpacing > 0 )
            {
                if ( !mbFontIndependentLineSpacing && pDesc )
                     nLineSpacing = (sal_Int16)( (double)nLineSpacing * pDesc->Scaling + 0.5 );
            }
            else
            {
                if ( pPortion && pPortion->mnCharHeight > (sal_uInt16)( ((double)-nLineSpacing) * 0.001 * 72.0 / 2.54 ) ) // 1/100mm to point
                    nLineSpacing = nNormalSpacing;
                else
                    nLineSpacing = (sal_Int16)( (double)nLineSpacing / 4.40972 );
            }
            if ( ( pPara->meLineSpacing == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, ParaAttr_LineFeed, nLineSpacing ) ) )
                nPropertyFlags |= 0x00001000;
        }
        if ( ( pPara->meLineSpacingTop == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
            ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, ParaAttr_UpperDist, pPara->mnLineSpacingTop ) ) )
            nPropertyFlags |= 0x00002000;
        if ( ( pPara->meLineSpacingBottom == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
            ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, ParaAttr_LowerDist, pPara->mnLineSpacingBottom ) ) )
            nPropertyFlags |= 0x00004000;
        if ( ( pPara->meForbiddenRules == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
            ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, ParaAttr_UpperDist, pPara->mbForbiddenRules ) ) )
            nPropertyFlags |= 0x00020000;
        if ( ( pPara->meParagraphPunctation == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
            ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, ParaAttr_UpperDist, pPara->mbParagraphPunctation ) ) )
            nPropertyFlags |= 0x00080000;
		if ( ( pPara->meBiDi == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
			( mpStyleSheet->IsHardAttribute( nInstance, nDepth, ParaAttr_BiDi, pPara->mnBiDi ) ) )
			nPropertyFlags |= 0x00200000;


        sal_Int32 nBuRealSize = pPara->nBulletRealSize;
        sal_Int16 nBulletFlags = pPara->nBulletFlags;

        if ( pPara->bExtendedParameters )
            nPropertyFlags |= pPara->nParaFlags;
        else
        {
            nPropertyFlags |= 1;            // turn off bullet explicit
            nBulletFlags = 0;
        }
        FontCollectionEntry aFontDescEntry( pPara->aFontDesc.Name, pPara->aFontDesc.Family, pPara->aFontDesc.Pitch, pPara->aFontDesc.CharSet );
        sal_uInt16  nFontId = (sal_uInt16)maFontCollection.GetId( aFontDescEntry );

        rOut << nCharCount
             << nDepth                          // Level
             << (sal_uInt32)nPropertyFlags;     // Paragraph Attribut Set

        if ( nPropertyFlags & 0xf )
            rOut << nBulletFlags;
        if ( nPropertyFlags & 0x80 )
            rOut << (sal_uInt16)( pPara->cBulletId );
        if ( nPropertyFlags & 0x10 )
            rOut << nFontId;
        if ( nPropertyFlags & 0x40 )
            rOut << (sal_Int16)nBuRealSize;
        if ( nPropertyFlags & 0x20 )
        {
            sal_uInt32 nBulletColor = pPara->nBulletColor;
            if ( nBulletColor == COL_AUTO )
            {
                sal_Bool bIsDark = sal_False;
                ::com::sun::star::uno::Any aAny;
                if ( PropValue::GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsBackgroundDark" ) ), sal_True ) )
                    aAny >>= bIsDark;
                nBulletColor = bIsDark ? 0xffffff : 0x000000;
            }
            nBulletColor &= 0xffffff;
            nBulletColor |= 0xfe000000;
            rOut << nBulletColor;
        }
        if ( nPropertyFlags & 0x00000800 )
            rOut << (sal_uInt16)( pPara->mnTextAdjust );
        if ( nPropertyFlags & 0x00001000 )
            rOut << (sal_uInt16)( nLineSpacing );
        if ( nPropertyFlags & 0x00002000 )
            rOut << (sal_uInt16)( pPara->mnLineSpacingTop );
        if ( nPropertyFlags & 0x00004000 )
            rOut << (sal_uInt16)( pPara->mnLineSpacingBottom );
        if ( nPropertyFlags & 0x000e0000 )
        {
            sal_uInt16 nAsianSettings = 0;
            if ( pPara->mbForbiddenRules )
                nAsianSettings |= 1;
            if ( pPara->mbParagraphPunctation )
                nAsianSettings |= 4;
            rOut << nAsianSettings;
        }
		if ( nPropertyFlags & 0x200000 )
			rOut << pPara->mnBiDi;
    }
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplWritePortions( SvStream& rOut, TextObj& rTextObj )
{
    sal_uInt32  nPropertyFlags, i = 0;
    int nInstance = rTextObj.GetInstance();

    for ( ParagraphObj* pPara = rTextObj.First(); pPara; pPara = rTextObj.Next(), i++ )
    {
        for ( PortionObj* pPortion = (PortionObj*)pPara->First(); pPortion; pPortion = (PortionObj*)pPara->Next() )
        {
            nPropertyFlags = 0;
            sal_uInt32 nCharAttr = pPortion->mnCharAttr;
            sal_uInt32 nCharColor = pPortion->mnCharColor;

            if ( nCharColor == COL_AUTO )   // nCharColor depends to the background color
            {
                sal_Bool bIsDark = sal_False;
                ::com::sun::star::uno::Any aAny;
                if ( PropValue::GetPropertyValue( aAny, mXPagePropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsBackgroundDark" ) ), sal_True ) )
                    aAny >>= bIsDark;
                nCharColor = bIsDark ? 0xffffff : 0x000000;
            }

            nCharColor &= 0xffffff;

			/* the portion is using the embossed or engraved attribute, which we want to map to the relief feature of PPT.
			Because the relief feature of PPT is dependent to the background color, such a mapping can not always be used. */
			if ( nCharAttr & 0x200 )
			{
				sal_uInt32 nBackgroundColor = 0xffffff;

				if ( !nCharColor )			// special threatment for
					nCharColor = 0xffffff;	// black fontcolor

				::com::sun::star::uno::Any aAny;
				::com::sun::star::drawing::FillStyle aFS( ::com::sun::star::drawing::FillStyle_NONE );
				if ( PropValue::GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillStyle" ) ) ) )
					aAny >>= aFS;
				switch( aFS )
				{
					case ::com::sun::star::drawing::FillStyle_GRADIENT :
					{
						Point aEmptyPoint = Point();
						Rectangle aRect( aEmptyPoint, Size( 28000, 21000 ) );
                        EscherPropertyContainer aPropOpt( mpPptEscherEx->GetGraphicProvider(), mpPicStrm, aRect );
						aPropOpt.CreateGradientProperties( mXPropSet );
						aPropOpt.GetOpt( ESCHER_Prop_fillColor, nBackgroundColor );
					}
					break;
					case ::com::sun::star::drawing::FillStyle_SOLID :
					{
						if ( PropValue::GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillColor" ) ) ) )
							nBackgroundColor = mpPptEscherEx->GetColor( *((sal_uInt32*)aAny.getValue()) );
					}
					break;
					case ::com::sun::star::drawing::FillStyle_NONE :
					{
						::com::sun::star::uno::Any aBackAny;
						::com::sun::star::drawing::FillStyle aBackFS( ::com::sun::star::drawing::FillStyle_NONE );
						if ( PropValue::GetPropertyValue( aBackAny, mXBackgroundPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillStyle" ) ) ) )
							aBackAny >>= aBackFS;
						switch( aBackFS )
						{
							case ::com::sun::star::drawing::FillStyle_GRADIENT :
							{
								Point aEmptyPoint = Point();
								Rectangle aRect( aEmptyPoint, Size( 28000, 21000 ) );
                                EscherPropertyContainer aPropOpt( mpPptEscherEx->GetGraphicProvider(), mpPicStrm, aRect );
								aPropOpt.CreateGradientProperties( mXBackgroundPropSet );
								aPropOpt.GetOpt( ESCHER_Prop_fillColor, nBackgroundColor );
							}
							break;
							case ::com::sun::star::drawing::FillStyle_SOLID :
							{
								if ( PropValue::GetPropertyValue( aAny, mXBackgroundPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillColor" ) ) ) )
									nBackgroundColor = mpPptEscherEx->GetColor( *((sal_uInt32*)aAny.getValue()) );
							}
							break;
							default:
								break;
						}
					}
					break;
					default:
						break;
				}

				sal_Int32 nB = nBackgroundColor & 0xff;
				nB += (sal_uInt8)( nBackgroundColor >> 8  );
				nB += (sal_uInt8)( nBackgroundColor >> 16 );
				// if the background color is nearly black, relief can't been used, because the text would not be visible
				if ( nB < 0x60 || ( nBackgroundColor != nCharColor ) )
				{
					nCharAttr &=~ 0x200;

					// now check if the text is part of a group, and if the previous object has the same color than the fontcolor
					// ( and if fillcolor is not available the background color ), it is sometimes
					// not possible to export the 'embossed' flag
					if ( ( GetCurrentGroupLevel() > 0 ) && ( GetCurrentGroupIndex() >= 1 ) )
					{
						::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape	> aGroupedShape( GetCurrentGroupAccess()->getByIndex( GetCurrentGroupIndex() - 1 ), uno::UNO_QUERY );
						if( aGroupedShape.is() )
						{
							::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aPropSetOfNextShape
								( aGroupedShape, ::com::sun::star::uno::UNO_QUERY );
							if ( aPropSetOfNextShape.is() )
							{
								if ( PropValue::GetPropertyValue( aAny, aPropSetOfNextShape,
													String( RTL_CONSTASCII_USTRINGPARAM( "FillColor" ) ), sal_True ) )
								{
									if ( nCharColor == mpPptEscherEx->GetColor( *((sal_uInt32*)aAny.getValue()) ) )
									{
										nCharAttr |= 0x200;
									}
								}
							}
						}
					}
				}
			}
            nCharColor |= 0xfe000000;
            if ( nInstance == 4 )                       // special handling for normal textobjects:
                nPropertyFlags |= nCharAttr & 0x217;    // not all attributes ar inherited
            else
            {
                if ( /* ( pPortion->mnCharAttrHard & 1 ) || */
                    ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_Bold, nCharAttr ) ) )
                    nPropertyFlags |= 1;
                if ( /* ( pPortion->mnCharAttrHard & 2 ) || */
                    ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_Italic, nCharAttr ) ) )
                    nPropertyFlags |= 2;
                if ( /* ( pPortion->mnCharAttrHard & 4 ) || */
                    ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_Underline, nCharAttr ) ) )
                    nPropertyFlags |= 4;
                if ( /* ( pPortion->mnCharAttrHard & 0x10 ) || */
                    ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_Shadow, nCharAttr ) ) )
                    nPropertyFlags |= 0x10;
                if ( /* ( pPortion->mnCharAttrHard & 0x200 ) || */
                    ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_Embossed, nCharAttr ) ) )
                    nPropertyFlags |= 512;
            }
            if ( rTextObj.HasExtendedBullets() )
            {
                nPropertyFlags |= ( i & 0x3f ) << 10 ;
                nCharAttr  |= ( i & 0x3f ) << 10;
            }
            if ( ( pPortion->meFontName == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_Font, pPortion->mnFont ) ) )
                nPropertyFlags |= 0x00010000;
            if ( ( pPortion->meAsianOrComplexFont == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_AsianOrComplexFont, pPortion->mnAsianOrComplexFont ) ) )
                nPropertyFlags |= 0x00200000;
            if ( ( pPortion->meCharHeight == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_FontHeight, pPortion->mnCharHeight ) ) )
                nPropertyFlags |= 0x00020000;
            if ( ( pPortion->meCharColor == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_FontColor, nCharColor & 0xffffff ) ) )
                nPropertyFlags |= 0x00040000;
            if ( ( pPortion->meCharEscapement == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_Escapement, pPortion->mnCharEscapement ) ) )
                nPropertyFlags |= 0x00080000;

            sal_uInt32 nCharCount = pPortion->Count();

            rOut << nCharCount
                 << nPropertyFlags;          //PropertyFlags

            if ( nPropertyFlags & 0xffff )
                rOut << (sal_uInt16)( nCharAttr );
            if ( nPropertyFlags & 0x00010000 )
                rOut << pPortion->mnFont;
            if ( nPropertyFlags & 0x00200000 )
                rOut << pPortion->mnAsianOrComplexFont;
            if ( nPropertyFlags & 0x00020000 )
                rOut << (sal_uInt16)( pPortion->mnCharHeight );
            if ( nPropertyFlags & 0x00040000 )
                rOut << (sal_uInt32)nCharColor;
            if ( nPropertyFlags & 0x00080000 )
                rOut << pPortion->mnCharEscapement;
        }
    }
}

//  ----------------------------------------------------------------------------------------
//  laedt und konvertiert text aus shape, ergebnis ist mnTextSize gespeichert;
sal_Bool PPTWriter::ImplGetText()
{
    mnTextSize = 0;
	mbFontIndependentLineSpacing = sal_False;
    mXText = ::com::sun::star::uno::Reference<
        ::com::sun::star::text::XSimpleText >
            ( mXShape, ::com::sun::star::uno::UNO_QUERY );

    if ( mXText.is() )
	{
        mnTextSize = mXText->getString().getLength();
		::com::sun::star::uno::Any aAny;
		if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FontIndependentLineSpacing" ) ) ), sal_True )
			aAny >>= mbFontIndependentLineSpacing;
	}
    return ( mnTextSize != 0 );
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplFlipBoundingBox( EscherPropertyContainer& rPropOpt )
{
    if ( mnAngle < 0 )
        mnAngle = ( 36000 + mnAngle ) % 36000;
    else
        mnAngle = ( 36000 - ( mnAngle % 36000 ) );

    double  fCos = cos( (double)mnAngle * F_PI18000 );
    double  fSin = sin( (double)mnAngle * F_PI18000 );

    double  fWidthHalf = maRect.GetWidth() / 2;
    double  fHeightHalf = maRect.GetHeight() / 2;

    double  fXDiff = fCos * fWidthHalf + fSin * (-fHeightHalf);
    double  fYDiff = - ( fSin * fWidthHalf - fCos * ( -fHeightHalf ) );

    maRect.Move( (sal_Int32)( -( fWidthHalf - fXDiff ) ), (sal_Int32)(  - ( fHeightHalf + fYDiff ) ) );
    mnAngle *= 655;
    mnAngle += 0x8000;
    mnAngle &=~0xffff;                                  // nAngle auf volle Gradzahl runden
    rPropOpt.AddOpt( ESCHER_Prop_Rotation, mnAngle );

    if ( ( mnAngle >= ( 45 << 16 ) && mnAngle < ( 135 << 16 ) ) ||
            ( mnAngle >= ( 225 << 16 ) && mnAngle < ( 315 << 16 ) ) )
    {
        // In diesen beiden Bereichen steht in PPT gemeinerweise die
        // BoundingBox bereits senkrecht. Daher muss diese VOR
        // DER ROTATION flachgelegt werden.
        ::com::sun::star::awt::Point
            aTopLeft( (sal_Int32)( maRect.Left() + fWidthHalf - fHeightHalf ), (sal_Int32)( maRect.Top() + fHeightHalf - fWidthHalf ) );
        Size    aNewSize( maRect.GetHeight(), maRect.GetWidth() );
        maRect = Rectangle( Point( aTopLeft.X, aTopLeft.Y ), aNewSize );
    }
}

//  -----------------------------------------------------------------------

struct FieldEntry
{
    sal_uInt32  nFieldType;
    sal_uInt32  nFieldStartPos;
    sal_uInt32  nFieldEndPos;
	String		aRepresentation;
    String      aFieldUrl;

    FieldEntry( sal_uInt32 nType, sal_uInt32 nStart, sal_uInt32 nEnd )
    {
        nFieldType = nType;
        nFieldStartPos = nStart;
        nFieldEndPos = nEnd;
    }
};

//  -----------------------------------------------------------------------

PortionObj::PortionObj( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                FontCollection& rFontCollection ) :
    mnCharAttrHard      ( 0 ),
    mnCharAttr          ( 0 ),
	mnFont              ( 0 ),
	mnAsianOrComplexFont( 0xffff ),
    mnTextSize          ( 0 ),
	mbLastPortion       ( sal_True ),
    mpText              ( NULL ),
    mpFieldEntry        ( NULL )
{
    mXPropSet = rXPropSet;

    ImplGetPortionValues( rFontCollection, sal_False );
}

PortionObj::PortionObj( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & rXTextRange,
                            sal_Bool bLast, FontCollection& rFontCollection ) :
    mnCharAttrHard          ( 0 ),
    mnCharAttr              ( 0 ),
    mnFont                  ( 0 ),
    mnAsianOrComplexFont    ( 0xffff ),
	mbLastPortion           ( bLast ),
    mpText                  ( NULL ),
    mpFieldEntry            ( NULL )
{
    String aString( rXTextRange->getString() );
    String aURL;
	sal_Bool bRTL_endingParen = sal_False;

    mnTextSize = aString.Len();
    if ( bLast )
        mnTextSize++;

    if ( mnTextSize )
    {
        mpFieldEntry = NULL;
        sal_uInt32 nFieldType = 0;

        mXPropSet = ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >
                ( rXTextRange, ::com::sun::star::uno::UNO_QUERY );
        mXPropState = ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertyState >
                ( rXTextRange, ::com::sun::star::uno::UNO_QUERY );

        sal_Bool bPropSetsValid = ( mXPropSet.is() && mXPropState.is() );
        if ( bPropSetsValid )
            nFieldType = ImplGetTextField( rXTextRange, mXPropSet, aURL );
        if ( nFieldType )
        {
            mpFieldEntry = new FieldEntry( nFieldType, 0, mnTextSize );
            if ( ( nFieldType >> 28 == 4 ) )
			{
				mpFieldEntry->aRepresentation = aString;
                mpFieldEntry->aFieldUrl = aURL;
			}
        }
        sal_Bool bSymbol = sal_False;

        if ( bPropSetsValid && ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontCharSet" ) ), sal_False ) )
        {
            sal_Int16 nCharset;
            mAny >>= nCharset;
            if ( nCharset == ::com::sun::star::awt::CharSet::SYMBOL )
                bSymbol = sal_True;
        }
        if ( mpFieldEntry && ( nFieldType & 0x800000 ) )    // placeholder ?
        {
            mnTextSize = 1;
            if ( bLast )
                mnTextSize++;
            mpText = new sal_uInt16[ mnTextSize ];
            mpText[ 0 ] = 0x2a;
        }
        else
        {
            const sal_Unicode* pText = aString.GetBuffer();
			// For i39516 - a closing parenthesis that ends an RTL string is displayed backwards by PPT
			// Solution: add a Unicode Right-to-Left Mark, following the method described in i18024
			if ( bLast && pText[ aString.Len() - 1 ] == sal_Unicode(')') && rFontCollection.GetScriptDirection( aString ) == com::sun::star::i18n::ScriptDirection::RIGHT_TO_LEFT )
			{
				mnTextSize++;
				bRTL_endingParen = sal_True;
			}
            mpText = new sal_uInt16[ mnTextSize ];
            sal_uInt16 nChar;
            for ( int i = 0; i < aString.Len(); i++ )
            {
                nChar = (sal_uInt16)pText[ i ];
                if ( nChar == 0xa )
                    nChar++;
                else if ( !bSymbol )
                {
                    switch ( nChar )
                    {
                        // Currency
                        case 128:   nChar = 0x20AC; break;
                        // Punctuation and other
                        case 130:   nChar = 0x201A; break;// SINGLE LOW-9 QUOTATION MARK
                        case 131:   nChar = 0x0192; break;// LATIN SMALL LETTER F WITH HOOK
                        case 132:   nChar = 0x201E; break;// DOUBLE LOW-9 QUOTATION MARK
                                                              // LOW DOUBLE PRIME QUOTATION MARK
                        case 133:   nChar = 0x2026; break;// HORIZONTAL ELLIPSES
                        case 134:   nChar = 0x2020; break;// DAGGER
                        case 135:   nChar = 0x2021; break;// DOUBLE DAGGER
                        case 136:   nChar = 0x02C6; break;// MODIFIER LETTER CIRCUMFLEX ACCENT
                        case 137:   nChar = 0x2030; break;// PER MILLE SIGN
                        case 138:   nChar = 0x0160; break;// LATIN CAPITAL LETTER S WITH CARON
                        case 139:   nChar = 0x2039; break;// SINGLE LEFT-POINTING ANGLE QUOTATION MARK
                        case 140:   nChar = 0x0152; break;// LATIN CAPITAL LIGATURE OE
                        case 142:   nChar = 0x017D; break;// LATIN CAPITAL LETTER Z WITH CARON
                        case 145:   nChar = 0x2018; break;// LEFT SINGLE QUOTATION MARK
                                                              // MODIFIER LETTER TURNED COMMA
                        case 146:   nChar = 0x2019; break;// RIGHT SINGLE QUOTATION MARK
                                                              // MODIFIER LETTER APOSTROPHE
                        case 147:   nChar = 0x201C; break;// LEFT DOUBLE QUOTATION MARK
                                                              // REVERSED DOUBLE PRIME QUOTATION MARK
                        case 148:   nChar = 0x201D; break;// RIGHT DOUBLE QUOTATION MARK
                                                              // REVERSED DOUBLE PRIME QUOTATION MARK
                        case 149:   nChar = 0x2022; break;// BULLET
                        case 150:   nChar = 0x2013; break;// EN DASH
                        case 151:   nChar = 0x2014; break;// EM DASH
                        case 152:   nChar = 0x02DC; break;// SMALL TILDE
                        case 153:   nChar = 0x2122; break;// TRADE MARK SIGN
                        case 154:   nChar = 0x0161; break;// LATIN SMALL LETTER S WITH CARON
                        case 155:   nChar = 0x203A; break;// SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
                        case 156:   nChar = 0x0153; break;// LATIN SMALL LIGATURE OE
                        case 158:   nChar = 0x017E; break;// LATIN SMALL LETTER Z WITH CARON
                        case 159:   nChar = 0x0178; break;// LATIN CAPITAL LETTER Y WITH DIAERESIS
//                      case 222:   nChar = 0x00B6; break;// PILCROW SIGN / PARAGRAPH SIGN
                    }
                }
                mpText[ i ] = nChar;
            }
        }
		if ( bRTL_endingParen )
            mpText[ mnTextSize - 2 ] = 0x200F; // Unicode Right-to-Left mark

        if ( bLast )
            mpText[ mnTextSize - 1 ] = 0xd;

        if ( bPropSetsValid )
            ImplGetPortionValues( rFontCollection, sal_True );
    }
}

PortionObj::PortionObj( const PortionObj& rPortionObj )
: PropStateValue( rPortionObj )
{
    ImplConstruct( rPortionObj );
}

PortionObj::~PortionObj()
{
    ImplClear();
}

void PortionObj::Write( SvStream* pStrm, sal_Bool bLast )
{
    sal_uInt32 nCount = mnTextSize;
    if ( bLast && mbLastPortion )
        nCount--;
    for ( sal_uInt32 i = 0; i < nCount; i++ )
        *pStrm << (sal_uInt16)mpText[ i ];
}

void PortionObj::ImplGetPortionValues( FontCollection& rFontCollection, sal_Bool bGetPropStateValue )
{

    sal_Bool bOk = ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontName" ) ), bGetPropStateValue );
    meFontName = ePropState;
    if ( bOk )
    {
        FontCollectionEntry aFontDesc( *(::rtl::OUString*)mAny.getValue() );
        sal_uInt32  nCount = rFontCollection.GetCount();
        mnFont = (sal_uInt16)rFontCollection.GetId( aFontDesc );
        if ( mnFont == nCount )
        {
            FontCollectionEntry& rFontDesc = rFontCollection.GetLast();
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontCharSet" ) ), sal_False ) )
                mAny >>= rFontDesc.CharSet;
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontFamily" ) ), sal_False ) )
                mAny >>= rFontDesc.Family;
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontPitch" ) ), sal_False ) )
                mAny >>= rFontDesc.Pitch;
        }
    }

	sal_Int16 nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( Application::GetSettings().GetLanguage() );
	if ( mpText && mnTextSize && xPPTBreakIter.is() )
	{
		rtl::OUString sT( mpText, mnTextSize );
		nScriptType = xPPTBreakIter->getScriptType( sT, 0 );
	}
	if ( nScriptType != com::sun::star::i18n::ScriptType::COMPLEX )
	{
		bOk = ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontNameAsian" ) ), bGetPropStateValue );
		meAsianOrComplexFont = ePropState;
		if ( bOk )
		{
			FontCollectionEntry aFontDesc( *(::rtl::OUString*)mAny.getValue() );
			sal_uInt32  nCount = rFontCollection.GetCount();
			mnAsianOrComplexFont = (sal_uInt16)rFontCollection.GetId( aFontDesc );
			if ( mnAsianOrComplexFont == nCount )
			{
				FontCollectionEntry& rFontDesc = rFontCollection.GetLast();
				if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontCharSetAsian" ) ), sal_False ) )
					mAny >>= rFontDesc.CharSet;
				if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontFamilyAsian" ) ), sal_False ) )
					mAny >>= rFontDesc.Family;
				if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontPitchAsian" ) ), sal_False ) )
					mAny >>= rFontDesc.Pitch;
			}
		}
	}
	else
	{
		bOk = ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontNameComplex" ) ), bGetPropStateValue );
		meAsianOrComplexFont = ePropState;
		if ( bOk )
		{
			FontCollectionEntry aFontDesc( *(::rtl::OUString*)mAny.getValue() );
			sal_uInt32  nCount = rFontCollection.GetCount();
			mnAsianOrComplexFont = (sal_uInt16)rFontCollection.GetId( aFontDesc );
			if ( mnAsianOrComplexFont == nCount )
			{
				FontCollectionEntry& rFontDesc = rFontCollection.GetLast();
				if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontCharSetComplex" ) ), sal_False ) )
					mAny >>= rFontDesc.CharSet;
				if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontFamilyComplex" ) ), sal_False ) )
					mAny >>= rFontDesc.Family;
				if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontPitchComplex" ) ), sal_False ) )
					mAny >>= rFontDesc.Pitch;
			}
		}
	}

	rtl::OUString aCharHeightName, aCharWeightName, aCharLocaleName, aCharPostureName;
	switch( nScriptType )
	{
		case com::sun::star::i18n::ScriptType::ASIAN :
		{
			aCharHeightName  = String( RTL_CONSTASCII_USTRINGPARAM( "CharHeightAsian" ) );
			aCharWeightName  = String( RTL_CONSTASCII_USTRINGPARAM( "CharWeightAsian" ) );
			aCharLocaleName  = String( RTL_CONSTASCII_USTRINGPARAM( "CharLocaleAsian" ) );
			aCharPostureName = String( RTL_CONSTASCII_USTRINGPARAM( "CharPostureAsian" ) );
			break;
		}
		case com::sun::star::i18n::ScriptType::COMPLEX :
		{
			aCharHeightName  = String( RTL_CONSTASCII_USTRINGPARAM( "CharHeightComplex" ) );
			aCharWeightName	 = String( RTL_CONSTASCII_USTRINGPARAM( "CharWeightComplex" ) );
			aCharLocaleName  = String( RTL_CONSTASCII_USTRINGPARAM( "CharLocaleComplex" ) );
			aCharPostureName = String( RTL_CONSTASCII_USTRINGPARAM( "CharPostureComplex" ) );
			break;
		}
		default:
		{
			aCharHeightName  = String( RTL_CONSTASCII_USTRINGPARAM( "CharHeight" ) );
			aCharWeightName  = String( RTL_CONSTASCII_USTRINGPARAM( "CharWeight" ) );
			aCharLocaleName  = String( RTL_CONSTASCII_USTRINGPARAM( "CharLocale" ) );
			aCharPostureName = String( RTL_CONSTASCII_USTRINGPARAM( "CharPosture" ) );
			break;
		}
	}

    mnCharHeight = 24;
	if ( GetPropertyValue( mAny, mXPropSet, aCharHeightName, sal_False ) )
	{
		float fVal(0.0);
		if ( mAny >>= fVal )
		{
	        mnCharHeight = (sal_uInt16)( fVal + 0.5 );
			meCharHeight = GetPropertyState( mXPropSet, aCharHeightName );
		}
	}
	if ( GetPropertyValue( mAny, mXPropSet, aCharWeightName, sal_False ) )
	{
		float fFloat(0.0);
		if ( mAny >>= fFloat )
		{
			if ( fFloat >= ::com::sun::star::awt::FontWeight::SEMIBOLD )
				mnCharAttr |= 1;
			if ( GetPropertyState( mXPropSet, aCharWeightName ) == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
				mnCharAttrHard |= 1;
		}
	}
	if ( GetPropertyValue( mAny, mXPropSet, aCharLocaleName, sal_False ) )
	{
		com::sun::star::lang::Locale eLocale;
		if ( mAny >>= eLocale )
			meCharLocale = eLocale;
	}
	if ( GetPropertyValue( mAny, mXPropSet, aCharPostureName, sal_False ) )
	{
		::com::sun::star::awt::FontSlant aFS;
		if ( mAny >>= aFS )
		{
			switch( aFS )
			{
				case ::com::sun::star::awt::FontSlant_OBLIQUE :
				case ::com::sun::star::awt::FontSlant_ITALIC :
					mnCharAttr |= 2;
					break;
				default:
					break;
			}
			if ( GetPropertyState( mXPropSet, aCharPostureName ) == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
				mnCharAttrHard |= 2;
		}
	}

	if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharUnderline" ) ), bGetPropStateValue ) )
    {
        sal_Int16 nVal(0);
        mAny >>= nVal;
        switch ( nVal )
        {
            case ::com::sun::star::awt::FontUnderline::SINGLE :
            case ::com::sun::star::awt::FontUnderline::DOUBLE :
            case ::com::sun::star::awt::FontUnderline::DOTTED :
                mnCharAttr |= 4;
        }
    }
    if ( ePropState == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        mnCharAttrHard |= 4;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharShadowed" ) ), bGetPropStateValue ) )
    {
        sal_Bool bBool(sal_False);
        mAny >>= bBool;
        if ( bBool )
            mnCharAttr |= 0x10;
    }
    if ( ePropState == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        mnCharAttrHard |= 16;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharRelief" ) ), bGetPropStateValue ) )
    {
        sal_Int16 nVal(0);
        mAny >>= nVal;
        if ( nVal != ::com::sun::star::text::FontRelief::NONE )
            mnCharAttr |= 512;
    }
    if ( ePropState == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        mnCharAttrHard |= 512;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharColor" ) ), bGetPropStateValue ) )
    {
        sal_uInt32 nSOColor = *( (sal_uInt32*)mAny.getValue() );
        mnCharColor = nSOColor & 0xff00ff00;                            // green and hibyte
        mnCharColor |= (sal_uInt8)( nSOColor ) << 16;                   // red and blue is switched
        mnCharColor |= (sal_uInt8)( nSOColor >> 16 );
    }
    meCharColor = ePropState;

    mnCharEscapement = 0;
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharEscapement" ) ), bGetPropStateValue ) )
    {
        mAny >>= mnCharEscapement;
        if ( mnCharEscapement > 100 )
            mnCharEscapement = 33;
        else if ( mnCharEscapement < -100 )
            mnCharEscapement = -33;
    }
    meCharEscapement = ePropState;
}

void PortionObj::ImplClear()
{
    delete (FieldEntry*)mpFieldEntry;
    delete[] mpText;
}

void PortionObj::ImplConstruct( const PortionObj& rPortionObj )
{
    mbLastPortion = rPortionObj.mbLastPortion;
    mnTextSize = rPortionObj.mnTextSize;
    mnCharColor = rPortionObj.mnCharColor;
    mnCharEscapement = rPortionObj.mnCharEscapement;
    mnCharAttr = rPortionObj.mnCharAttr;
    mnCharHeight = rPortionObj.mnCharHeight;
    mnFont = rPortionObj.mnFont;
    mnAsianOrComplexFont = rPortionObj.mnAsianOrComplexFont;

    if ( rPortionObj.mpText )
    {
        mpText = new sal_uInt16[ mnTextSize ];
        memcpy( mpText, rPortionObj.mpText, mnTextSize << 1 );
    }
    else
        mpText = NULL;

    if ( rPortionObj.mpFieldEntry )
        mpFieldEntry = new FieldEntry( *( rPortionObj.mpFieldEntry ) );
    else
        mpFieldEntry = NULL;
}

sal_uInt32 PortionObj::ImplCalculateTextPositions( sal_uInt32 nCurrentTextPosition )
{
    if ( mpFieldEntry && ( !mpFieldEntry->nFieldStartPos ) )
    {
        mpFieldEntry->nFieldStartPos += nCurrentTextPosition;
        mpFieldEntry->nFieldEndPos += nCurrentTextPosition;
    }
    return mnTextSize;
}

//  -----------------------------------------------------------------------
// Rueckgabe:                           0 = kein TextField
//  bit28->31   text field type :
//                                      1 = Date
//                                      2 = Time
//                                      3 = SlideNumber
//                                      4 = Url
//										5 = DateTime
//										6 = header
//										7 = footer
//  bit24->27   text field sub type	(optional)
//     23->     PPT Textfield needs a placeholder

sal_uInt32 PortionObj::ImplGetTextField( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & ,
	const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet, String& rURL )
{
    sal_uInt32 nRetValue = 0;
    sal_Int32 nFormat;
	::com::sun::star::uno::Any aAny;
	if ( GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextPortionType" ) ), sal_True ) )
	{
		String  aTextFieldType( *(::rtl::OUString*)aAny.getValue() );
		if ( aTextFieldType == String( RTL_CONSTASCII_USTRINGPARAM( "TextField" ) ) )
		{
			if ( GetPropertyValue( aAny, rXPropSet, aTextFieldType, sal_True ) )
			{
				::com::sun::star::uno::Reference< ::com::sun::star::text::XTextField > aXTextField;
				if ( aAny >>= aXTextField )
				{
					if ( aXTextField.is() )
					{
						::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
							xFieldPropSet( aXTextField, ::com::sun::star::uno::UNO_QUERY );
						if ( xFieldPropSet.is() )
						{
							String aFieldKind( aXTextField->getPresentation( sal_True ) );
							if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Date" ) ) )
							{
								if ( GetPropertyValue( aAny, xFieldPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsFix" ) ) ), sal_True )
								{
									sal_Bool bBool;
									aAny >>= bBool;
									if ( !bBool )  // Fixed DateFields gibt es in PPT nicht
									{
										if ( GetPropertyValue( aAny, xFieldPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Format" ) ) ), sal_True )
										{
											nFormat = *(sal_Int32*)aAny.getValue();
											switch ( nFormat )
											{
												default:
												case 5 :
												case 4 :
												case 2 : nFormat = 0; break;
												case 8 :
												case 9 :
												case 3 : nFormat = 1; break;
												case 7 :
												case 6 : nFormat = 2; break;
											}
											nRetValue |= ( ( ( 1 << 4 ) | nFormat ) << 24 ) | 0x800000;
										}
									}
								}
							}
							else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "URL" ) ) )
							{
								if ( GetPropertyValue( aAny, xFieldPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "URL" ) ) ), sal_True )
									rURL = String( *(::rtl::OUString*)aAny.getValue() );
								nRetValue = 4 << 28;
							}
							else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Page" ) ) )
							{
								nRetValue = 3 << 28 | 0x800000;
							}
							else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Pages" ) ) )
							{

							}
							else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Time" ) ) )
							{
								if ( GetPropertyValue( aAny, xFieldPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsFix" ) ) ), sal_True )
								{
									sal_Bool bBool;
									aAny >>= bBool;
									if ( !bBool )
									{
										if ( GetPropertyValue( aAny, xFieldPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsFix" ) ) ), sal_True )
										{
											nFormat = *(sal_Int32*)aAny.getValue();
											nRetValue |= ( ( ( 2 << 4 ) | nFormat ) << 24 ) | 0x800000;
										}
									}
								}
							}
							else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "File" ) ) )
							{

							}
							else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Table" ) ) )
							{

							}
							else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "ExtTime" ) ) )
							{
								if ( GetPropertyValue( aAny, xFieldPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsFix" ) ) ), sal_True )
								{
									sal_Bool bBool;
									aAny >>= bBool;
									if ( !bBool )
									{
										if ( GetPropertyValue( aAny, xFieldPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Format" ) ) ), sal_True )
										{
											nFormat = *(sal_Int32*)aAny.getValue();
											switch ( nFormat )
											{
												default:
												case 6 :
												case 7 :
												case 8 :
												case 2 : nFormat = 12; break;
												case 3 : nFormat = 9; break;
												case 5 :
												case 4 : nFormat = 10; break;

											}
											nRetValue |= ( ( ( 2 << 4 ) | nFormat ) << 24 ) | 0x800000;
										}
									}
								}
							}
							else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "ExtFile" ) ) )
							{

							}
							else if ( aFieldKind ==  String( RTL_CONSTASCII_USTRINGPARAM( "Author" ) ) )
							{

							}
							else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "DateTime" ) ) )
							{
								nRetValue = 5 << 28 | 0x800000;
							}
							else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Header" ) ) )
							{
								nRetValue = 6 << 28 | 0x800000;
							}
							else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Footer" ) ) )
							{
								nRetValue = 7 << 28 | 0x800000;
							}
						}
					}
				}
			}
		}
	}
    return nRetValue;
}

PortionObj& PortionObj::operator=( const PortionObj& rPortionObj )
{
    if ( this != &rPortionObj )
    {
        ImplClear();
        ImplConstruct( rPortionObj );
    }
    return *this;
}

//  -----------------------------------------------------------------------

ParagraphObj::ParagraphObj( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                PPTExBulletProvider& rProv ) :
    maMapModeSrc        ( MAP_100TH_MM ),
    maMapModeDest       ( MAP_INCH, Point(), Fraction( 1, 576 ), Fraction( 1, 576 ) )
{
    mXPropSet = rXPropSet;

    bExtendedParameters = sal_False;

    nDepth = 0;
    nBulletFlags = 0;
    nParaFlags = 0;

    ImplGetParagraphValues( rProv, sal_False );
}

    ParagraphObj::ParagraphObj( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > & rXTextContent,
                    ParaFlags aParaFlags, FontCollection& rFontCollection, PPTExBulletProvider& rProv ) :
    maMapModeSrc        ( MAP_100TH_MM ),
    maMapModeDest       ( MAP_INCH, Point(), Fraction( 1, 576 ), Fraction( 1, 576 ) ),
    mbFirstParagraph    ( aParaFlags.bFirstParagraph ),
    mbLastParagraph     ( aParaFlags.bLastParagraph )
{
    bExtendedParameters = sal_False;

    nDepth = 0;
    nBulletFlags = 0;
    nParaFlags = 0;

    mXPropSet = ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
            ( rXTextContent, ::com::sun::star::uno::UNO_QUERY );

    mXPropState = ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertyState >
            ( rXTextContent, ::com::sun::star::uno::UNO_QUERY );

    if ( mXPropSet.is() && mXPropState.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess >
            aXTextPortionEA( rXTextContent, ::com::sun::star::uno::UNO_QUERY );
        if ( aXTextPortionEA.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >
                aXTextPortionE( aXTextPortionEA->createEnumeration() );
            if ( aXTextPortionE.is() )
            {
                while ( aXTextPortionE->hasMoreElements() )
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > aXCursorText;
                    ::com::sun::star::uno::Any aAny( aXTextPortionE->nextElement() );
                    if ( aAny >>= aXCursorText )
                    {
                        PortionObj* pPortionObj = new PortionObj( aXCursorText, !aXTextPortionE->hasMoreElements(), rFontCollection );
                        if ( pPortionObj->Count() )
                            Insert( pPortionObj, LIST_APPEND );
                        else
                            delete pPortionObj;
                    }
                }
            }
        }
        ImplGetParagraphValues( rProv, sal_True );//
    }
}

ParagraphObj::ParagraphObj( const ParagraphObj& rObj )
: List()
, PropStateValue()
, SOParagraph()
{
    ImplConstruct( rObj );
}

ParagraphObj::~ParagraphObj()
{
    ImplClear();
}

void ParagraphObj::Write( SvStream* pStrm )
{
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        ((PortionObj*)pPtr)->Write( pStrm, mbLastParagraph );
}

void ParagraphObj::ImplClear()
{
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        delete (PortionObj*)pPtr;
}

void ParagraphObj::CalculateGraphicBulletSize( sal_uInt16 nFontHeight )
{
    if ( ( (SvxExtNumType)nNumberingType == SVX_NUM_BITMAP ) && ( nBulletId != 0xffff ) )
    {
        // calculate the bulletrealsize for this grafik
        if ( aBuGraSize.Width() && aBuGraSize.Height() )
        {
            double fCharHeight = nFontHeight;
            double fLen = aBuGraSize.Height();
            fCharHeight = fCharHeight * 0.2540;
            double fQuo = fLen / fCharHeight;
            nBulletRealSize = (sal_Int16)( fQuo + 0.5 );
            if ( (sal_uInt16)nBulletRealSize > 400 )
                nBulletRealSize = 400;
        }
    }
}

// from sw/source/filter/ww8/wrtw8num.cxx for default bullets to export to MS intact
static void lcl_SubstituteBullet(String& rNumStr, rtl_TextEncoding& rChrSet, String& rFontName)
{
	sal_Unicode cChar = rNumStr.GetChar(0);
	StarSymbolToMSMultiFont *pConvert = CreateStarSymbolToMSMultiFont();
	String sFont = pConvert->ConvertChar(cChar);
	delete pConvert;
	if (sFont.Len())
	{
		rNumStr = static_cast< sal_Unicode >(cChar | 0xF000);
		rFontName = sFont;
		rChrSet = RTL_TEXTENCODING_SYMBOL;
	}
	else if ( (rNumStr.GetChar(0) < 0xE000 || rNumStr.GetChar(0) > 0xF8FF) )
	{
		/*
		Ok we can't fit into a known windows unicode font, but
		we are not in the private area, so we are a
		standardized symbol, so turn off the symbol bit and
		let words own font substitution kick in
		*/
		rChrSet = RTL_TEXTENCODING_UNICODE;
		rFontName = ::GetFontToken(rFontName, 0);
	}
	else
	{
		/*
		Well we don't have an available substition, and we're
		in our private area, so give up and show a standard
		bullet symbol
		*/
		rFontName.AssignAscii(RTL_CONSTASCII_STRINGPARAM("Wingdings"));
		rNumStr = static_cast< sal_Unicode >(0x6C);
     }
}

void ParagraphObj::ImplGetNumberingLevel( PPTExBulletProvider& rBuProv, sal_Int16 nNumberingDepth, sal_Bool bIsBullet, sal_Bool bGetPropStateValue )
{
	::com::sun::star::uno::Any aAny;
	if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "ParaLeftMargin" ) ) ) )
	{
		sal_Int32 nVal;
        if ( aAny >>= nVal )
			nTextOfs = static_cast< sal_Int16 >( nVal / ( 2540.0 / 576 ) + 0.5 ) ;
	}
    if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "ParaFirstLineIndent" ) ) ) )
	{
        if ( aAny >>= nBulletOfs )
			nBulletOfs = static_cast< sal_Int32 >( nBulletOfs / ( 2540.0 / 576 ) + 0.5 );
	}
	if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "NumberingIsNumber" ) ) ) )
		aAny >>= bNumberingIsNumber;

	::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace > aXIndexReplace;

    if ( bIsBullet && ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "NumberingRules" ) ), bGetPropStateValue ) )
    {
        if ( ( mAny >>= aXIndexReplace ) && nNumberingDepth < aXIndexReplace->getCount() )
        {
            mAny <<= aXIndexReplace->getByIndex( nNumberingDepth );
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>
                aPropertySequence( *( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>*)mAny.getValue() );

            const ::com::sun::star::beans::PropertyValue* pPropValue = aPropertySequence.getArray();

            sal_Int32 nPropertyCount = aPropertySequence.getLength();
            if ( nPropertyCount )
            {
                bExtendedParameters = sal_True;
                nBulletRealSize = 100;
                nMappedNumType = 0;

                String aGraphicURL;
                for ( sal_Int32 i = 0; i < nPropertyCount; i++ )
                {
                    const void* pValue = pPropValue[ i ].Value.getValue();
                    if ( pValue )
                    {
                        ::rtl::OUString aPropName( pPropValue[ i ].Name );
                        if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "NumberingType" ) ) )
                            nNumberingType = *( (sal_Int16*)pValue );
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Adjust" ) ) )
                            nHorzAdjust = *( (sal_Int16*)pValue );
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "BulletChar" ) ) )
                        {
                            String aString( *( (String*)pValue ) );
                            if ( aString.Len() )
                                cBulletId = aString.GetChar( 0 );
                        }
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "BulletFont" ) ) )
                        {
                            aFontDesc = *( (::com::sun::star::awt::FontDescriptor*)pValue );

                            // Our numbullet dialog has set the wrong textencoding for our "StarSymbol" font,
                            // instead of a Unicode encoding the encoding RTL_TEXTENCODING_SYMBOL was used.
                            // Because there might exist a lot of damaged documemts I added this two lines
                            // which fixes the bullet problem for the export.
                            if ( aFontDesc.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "StarSymbol" ) ) )
                                aFontDesc.CharSet = RTL_TEXTENCODING_MS_1252;

                        }
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "GraphicURL" ) ) )
                            aGraphicURL = ( *(::rtl::OUString*)pValue );
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "GraphicSize" ) ) )
                        {
                            if ( pPropValue[ i ].Value.getValueType() == ::getCppuType( (::com::sun::star::awt::Size*)0) )
                            {
                                // don't cast awt::Size to Size as on 64-bits they are not the same.
                                ::com::sun::star::awt::Size aSize;
                                pPropValue[ i ].Value >>= aSize;
                                aBuGraSize.nA = aSize.Width;
                                aBuGraSize.nB = aSize.Height;
                            }
                        }
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "StartWith" ) ) )
                            nStartWith = *( (sal_Int16*)pValue );
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "LeftMargin" ) ) )
                            nTextOfs = nTextOfs + static_cast< sal_Int16 >( *( (sal_Int32*)pValue ) / ( 2540.0 / 576 ) );
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FirstLineOffset" ) ) )
                            nBulletOfs += (sal_Int16)( *( (sal_Int32*)pValue ) / ( 2540.0 / 576 ) );
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "BulletColor" ) ) )
                        {
                            sal_uInt32 nSOColor = *( (sal_uInt32*)pValue );
                            nBulletColor = nSOColor & 0xff00ff00;                       // green and hibyte
                            nBulletColor |= (sal_uInt8)( nSOColor ) << 16;              // red
                            nBulletColor |= (sal_uInt8)( nSOColor >> 16 ) | 0xfe000000; // blue
                        }
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "BulletRelSize" ) ) )
                        {
                            nBulletRealSize = *( (sal_Int16*)pValue );
                            nParaFlags |= 0x40;
                            nBulletFlags |= 8;
                        }
                        else if ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Prefix" ) ) )
                            sPrefix = ( *(::rtl::OUString*)pValue );
                        else if  ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Suffix" ) ) )
                            sSuffix = ( *(::rtl::OUString*)pValue );
#ifdef DBG_UTIL
                        else if ( ! (
                                ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "SymbolTextDistance" ) ) )
                            ||  ( aPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Graphic" ) ) ) ) )
                        {
                            DBG_ERROR( "Unbekanntes Property" );
                        }
#endif
                    }
                }

                if ( aGraphicURL.Len() )
                {
					if ( aBuGraSize.Width() && aBuGraSize.Height() )
					{
						xub_StrLen nIndex = aGraphicURL.Search( (sal_Unicode)':', 0 );
						if ( nIndex != STRING_NOTFOUND )
						{
							nIndex++;
							if ( aGraphicURL.Len() > nIndex  )
							{
								ByteString aUniqueId( aGraphicURL, nIndex, aGraphicURL.Len() - nIndex, RTL_TEXTENCODING_UTF8 );
								if ( aUniqueId.Len() )
								{
									nBulletId = rBuProv.GetId( aUniqueId, aBuGraSize );
									if ( nBulletId != 0xffff )
										bExtendedBulletsUsed = sal_True;
								}
							}
						}
					}
					else
					{
						nNumberingType = SVX_NUM_NUMBER_NONE;
					}
                }

                PortionObj* pPortion = (PortionObj*)First();
                CalculateGraphicBulletSize( ( pPortion ) ? pPortion->mnCharHeight : 24 );

                switch( (SvxExtNumType)nNumberingType )
                {
                    case SVX_NUM_NUMBER_NONE : nParaFlags |= 0xf; break;

                    case SVX_NUM_CHAR_SPECIAL :                           // Bullet
                    {
						if ( aFontDesc.Name.equalsIgnoreAsciiCaseAscii("starsymbol") ||
							aFontDesc.Name.equalsIgnoreAsciiCaseAscii("opensymbol") )
						{
							String sFontName = aFontDesc.Name;
							String sNumStr = cBulletId;
							rtl_TextEncoding eChrSet = aFontDesc.CharSet;
                            lcl_SubstituteBullet(sNumStr,eChrSet,sFontName);
							aFontDesc.Name = sFontName;
							cBulletId = sNumStr.GetChar( 0 );
							aFontDesc.CharSet = eChrSet;
						 }

                        if ( aFontDesc.Name.getLength() )
                        {
/*
                            if ( aFontDesc.CharSet != ::com::sun::star::awt::CharSet::SYMBOL )
                            {
                                switch ( cBulletId )
                                {
                                    // Currency
                                    case 128:   cBulletId = 0x20AC; break;
                                    // Punctuation and other
                                    case 130:   cBulletId = 0x201A; break;// SINGLE LOW-9 QUOTATION MARK
                                    case 131:   cBulletId = 0x0192; break;// LATIN SMALL LETTER F WITH HOOK
                                    case 132:   cBulletId = 0x201E; break;// DOUBLE LOW-9 QUOTATION MARK
                                                                          // LOW DOUBLE PRIME QUOTATION MARK
                                    case 133:   cBulletId = 0x2026; break;// HORIZONTAL ELLIPSES
                                    case 134:   cBulletId = 0x2020; break;// DAGGER
                                    case 135:   cBulletId = 0x2021; break;// DOUBLE DAGGER
                                    case 136:   cBulletId = 0x02C6; break;// MODIFIER LETTER CIRCUMFLEX ACCENT
                                    case 137:   cBulletId = 0x2030; break;// PER MILLE SIGN
                                    case 138:   cBulletId = 0x0160; break;// LATIN CAPITAL LETTER S WITH CARON
                                    case 139:   cBulletId = 0x2039; break;// SINGLE LEFT-POINTING ANGLE QUOTATION MARK
                                    case 140:   cBulletId = 0x0152; break;// LATIN CAPITAL LIGATURE OE
                                    case 142:   cBulletId = 0x017D; break;// LATIN CAPITAL LETTER Z WITH CARON
                                    case 145:   cBulletId = 0x2018; break;// LEFT SINGLE QUOTATION MARK
                                                                          // MODIFIER LETTER TURNED COMMA
                                    case 146:   cBulletId = 0x2019; break;// RIGHT SINGLE QUOTATION MARK
                                                                          // MODIFIER LETTER APOSTROPHE
                                    case 147:   cBulletId = 0x201C; break;// LEFT DOUBLE QUOTATION MARK
                                                                          // REVERSED DOUBLE PRIME QUOTATION MARK
                                    case 148:   cBulletId = 0x201D; break;// RIGHT DOUBLE QUOTATION MARK
                                                                          // REVERSED DOUBLE PRIME QUOTATION MARK
                                    case 149:   cBulletId = 0x2022; break;// BULLET
                                    case 150:   cBulletId = 0x2013; break;// EN DASH
                                    case 151:   cBulletId = 0x2014; break;// EM DASH
                                    case 152:   cBulletId = 0x02DC; break;// SMALL TILDE
                                    case 153:   cBulletId = 0x2122; break;// TRADE MARK SIGN
                                    case 154:   cBulletId = 0x0161; break;// LATIN SMALL LETTER S WITH CARON
                                    case 155:   cBulletId = 0x203A; break;// SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
                                    case 156:   cBulletId = 0x0153; break;// LATIN SMALL LIGATURE OE
                                    case 158:   cBulletId = 0x017E; break;// LATIN SMALL LETTER Z WITH CARON
                                    case 159:   cBulletId = 0x0178; break;// LATIN CAPITAL LETTER Y WITH DIAERESIS
//                                  case 222:   cBulletId = 0x00B6; break;// PILCROW SIGN / PARAGRAPH SIGN
                                }
                            }
*/
                            nParaFlags |= 0x90; // wir geben den Font und den Charset vor
                        }
                    }
                    case SVX_NUM_CHARS_UPPER_LETTER :       // zaehlt von a-z, aa - az, ba - bz, ...
                    case SVX_NUM_CHARS_LOWER_LETTER :
                    case SVX_NUM_ROMAN_UPPER :
                    case SVX_NUM_ROMAN_LOWER :
                    case SVX_NUM_ARABIC :
                    case SVX_NUM_PAGEDESC :                 // Numerierung aus der Seitenvorlage
                    case SVX_NUM_BITMAP :
                    case SVX_NUM_CHARS_UPPER_LETTER_N :     // zaehlt von  a-z, aa-zz, aaa-zzz
                    case SVX_NUM_CHARS_LOWER_LETTER_N :
                    {
                        if ( nNumberingType != SVX_NUM_CHAR_SPECIAL )
                        {
                            bExtendedBulletsUsed = sal_True;
                            if ( nNumberingDepth & 1 )
                                cBulletId = 0x2013;         // defaulting bullet characters for ppt97
                            else if ( nNumberingDepth == 4 )
                                cBulletId = 0xbb;
                            else
                                cBulletId = 0x2022;

                            switch( (SvxExtNumType)nNumberingType )
                            {
                                case SVX_NUM_CHARS_UPPER_LETTER :
                                case SVX_NUM_CHARS_UPPER_LETTER_N :
                                {
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
                                            nMappedNumType = 0xa0001;   // (A)
                                        else
                                            nMappedNumType = 0xb0001;   // A)
                                    }
                                    else
                                        nMappedNumType = 0x10001;       // A.
                                }
                                break;
                                case SVX_NUM_CHARS_LOWER_LETTER :
                                case SVX_NUM_CHARS_LOWER_LETTER_N :
                                {
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
                                            nMappedNumType = 0x80001;   // (a)
                                        else
                                            nMappedNumType = 0x90001;   // a)
                                    }
                                    else
                                        nMappedNumType = 0x00001;       // a.
                                }
                                break;
                                case SVX_NUM_ROMAN_UPPER :
                                {
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
                                            nMappedNumType = 0xe0001;   // (I)
                                        else
                                            nMappedNumType = 0xf0001;   // I)
                                    }
                                    else
                                        nMappedNumType = 0x70001;       // I.
                                }
                                break;
                                case SVX_NUM_ROMAN_LOWER :
                                {
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
                                            nMappedNumType = 0x40001;   // (i)
                                        else
                                            nMappedNumType = 0x50001;   // i)
                                    }
                                    else
                                        nMappedNumType = 0x60001;       // i.
                                }
                                break;
                                case SVX_NUM_ARABIC :
                                {
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
                                            nMappedNumType = 0xc0001;   // (1)
                                        else
                                            nMappedNumType = 0x20001;   // 1)
                                    }
                                    else
                                    {
                                        if ( ! ( sSuffix.Len() + sPrefix.Len() ) )
                                            nMappedNumType = 0xd0001;   // 1
                                        else
                                            nMappedNumType = 0x30001;   // 1.
                                    }
                                }
                                break;
								default:
									break;
                            }
                        }
                        nParaFlags |= 0x2f;
                        nBulletFlags |= 6;
                        if ( mbIsBullet && bNumberingIsNumber )
                            nBulletFlags |= 1;
                    }
                }
            }
        }
    }
    nBulletOfs = nTextOfs + nBulletOfs;
    if ( nBulletOfs < 0 )
        nBulletOfs = 0;
}

void ParagraphObj::ImplGetParagraphValues( PPTExBulletProvider& rBuProv, sal_Bool bGetPropStateValue )
{
    static String sNumberingLevel   ( RTL_CONSTASCII_USTRINGPARAM( "NumberingLevel" ) );

	::com::sun::star::uno::Any aAny;
	if ( GetPropertyValue( aAny, mXPropSet, sNumberingLevel, sal_True ) )
    {
        if ( bGetPropStateValue )
            meBullet = GetPropertyState( mXPropSet, sNumberingLevel );
        nDepth = *( (sal_Int16*)aAny.getValue() );

		if ( nDepth < 0 )
		{
			mbIsBullet = sal_False;
			nDepth = 0;
		}
		else
		{
			if ( nDepth > 4 )
				nDepth = 4;
			mbIsBullet = sal_True;
		}
    }
    else
	{
        nDepth = 0;
		mbIsBullet = sal_False;
	}
    ImplGetNumberingLevel( rBuProv, nDepth, mbIsBullet, bGetPropStateValue );

	if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaTabStops" ) ), bGetPropStateValue ) )
        maTabStop = *( ::com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop>*)mAny.getValue();
    sal_Int16 eTextAdjust( ::com::sun::star::style::ParagraphAdjust_LEFT );
    if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "ParaAdjust" ) ), bGetPropStateValue ) )
        aAny >>= eTextAdjust;
    switch ( (::com::sun::star::style::ParagraphAdjust)eTextAdjust )
    {
        case ::com::sun::star::style::ParagraphAdjust_CENTER :
            mnTextAdjust = 1;
        break;
        case ::com::sun::star::style::ParagraphAdjust_RIGHT :
            mnTextAdjust = 2;
        break;
        case ::com::sun::star::style::ParagraphAdjust_BLOCK :
            mnTextAdjust = 3;
        break;
        default :
        case ::com::sun::star::style::ParagraphAdjust_LEFT :
            mnTextAdjust = 0;
        break;
    }
    meTextAdjust = ePropState;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaLineSpacing" ) ), bGetPropStateValue ) )
    {
        ::com::sun::star::style::LineSpacing aLineSpacing
            = *( (::com::sun::star::style::LineSpacing*)mAny.getValue() );
        switch ( aLineSpacing.Mode )
        {
            case ::com::sun::star::style::LineSpacingMode::MINIMUM :
            case ::com::sun::star::style::LineSpacingMode::LEADING :
            case ::com::sun::star::style::LineSpacingMode::FIX :
                mnLineSpacing = (sal_Int16)(-( aLineSpacing.Height ) );
            break;

            case ::com::sun::star::style::LineSpacingMode::PROP :
            default:
                mnLineSpacing = (sal_Int16)( aLineSpacing.Height );
            break;
        }
    }
    meLineSpacing = ePropState;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaBottomMargin" ) ), bGetPropStateValue ) )
    {
        double fSpacing = *( (sal_uInt32*)mAny.getValue() ) + ( 2540.0 / 576.0 ) - 1;
        mnLineSpacingBottom = (sal_Int16)(-( fSpacing * 576.0 / 2540.0 ) );
    }
    meLineSpacingBottom = ePropState;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaTopMargin" ) ), bGetPropStateValue ) )
    {
        double fSpacing = *( (sal_uInt32*)mAny.getValue() ) + ( 2540.0 / 576.0 ) - 1;
        mnLineSpacingTop = (sal_Int16)(-( fSpacing * 576.0 / 2540.0 ) );
    }
    meLineSpacingTop = ePropState;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaIsForbiddenRules" ) ), bGetPropStateValue ) )
        mAny >>= mbForbiddenRules;
    meForbiddenRules = ePropState;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaIsHangingPunctuation" ) ), bGetPropStateValue ) )
        mAny >>= mbParagraphPunctation;
    meParagraphPunctation = ePropState;

	mnBiDi = 0;
	if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "WritingMode" ) ), bGetPropStateValue ) )
	{
		sal_Int16 nWritingMode;
		mAny >>= nWritingMode;

		SvxFrameDirection eWritingMode( (SvxFrameDirection)nWritingMode );
		if ( ( eWritingMode == FRMDIR_HORI_RIGHT_TOP )
			|| ( eWritingMode == FRMDIR_VERT_TOP_RIGHT ) )
		{
			mnBiDi = 1;
		}
	}
	meBiDi = ePropState;
}

void ParagraphObj::ImplConstruct( const ParagraphObj& rParagraphObj )
{
    mnTextSize = rParagraphObj.mnTextSize;
    mnTextAdjust = rParagraphObj.mnTextAdjust;
    mnLineSpacing = rParagraphObj.mnLineSpacing;
    mnLineSpacingTop = rParagraphObj.mnLineSpacingTop;
    mnLineSpacingBottom = rParagraphObj.mnLineSpacingBottom;
    mbFirstParagraph = rParagraphObj.mbFirstParagraph;
    mbLastParagraph = rParagraphObj.mbLastParagraph;
    mbParagraphPunctation = rParagraphObj.mbParagraphPunctation;
    mbForbiddenRules = rParagraphObj.mbForbiddenRules;
    mnBiDi = rParagraphObj.mnBiDi;

    ParagraphObj& rOther = const_cast<ParagraphObj&>(rParagraphObj);
    for ( const void* pPtr = rOther.First(); pPtr; pPtr = rOther.Next() )
        Insert( new PortionObj( *(const PortionObj*)pPtr ), LIST_APPEND );

    maTabStop = rParagraphObj.maTabStop;
    bExtendedParameters = rParagraphObj.bExtendedParameters;
    nParaFlags = rParagraphObj.nParaFlags;
    nBulletFlags = rParagraphObj.nBulletFlags;
    sPrefix = rParagraphObj.sPrefix;
    sSuffix = rParagraphObj.sSuffix;
    sGraphicUrl = rParagraphObj.sGraphicUrl;            // String auf eine Graphic
    aBuGraSize = rParagraphObj.aBuGraSize;
    nNumberingType = rParagraphObj.nNumberingType;      // in wirlichkeit ist dies ein SvxEnum
    nHorzAdjust = rParagraphObj.nHorzAdjust;
    nBulletColor = rParagraphObj.nBulletColor;
    nBulletOfs = rParagraphObj.nBulletOfs;
    nStartWith = rParagraphObj.nStartWith;              // Start der nummerierung
    nTextOfs = rParagraphObj.nTextOfs;
    nBulletRealSize = rParagraphObj.nBulletRealSize;    // GroessenVerhaeltnis in Proz
    nDepth = rParagraphObj.nDepth;                      // aktuelle tiefe
    cBulletId = rParagraphObj.cBulletId;                // wenn Numbering Type == CharSpecial
    aFontDesc = rParagraphObj.aFontDesc;

    bExtendedBulletsUsed = rParagraphObj.bExtendedBulletsUsed;
    nBulletId = rParagraphObj.nBulletId;
}

sal_uInt32 ParagraphObj::ImplCalculateTextPositions( sal_uInt32 nCurrentTextPosition )
{
    mnTextSize = 0;
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        mnTextSize += ((PortionObj*)pPtr)->ImplCalculateTextPositions( nCurrentTextPosition + mnTextSize );
    return mnTextSize;
}

ParagraphObj& ParagraphObj::operator=( const ParagraphObj& rParagraphObj )
{
    if ( this != &rParagraphObj )
    {
        ImplClear();
        ImplConstruct( rParagraphObj );
    }
    return *this;
}

//  -----------------------------------------------------------------------

ImplTextObj::ImplTextObj( int nInstance )
{
    mnRefCount = 1;
    mnTextSize = 0;
    mnInstance = nInstance;
    mpList = new List;
    mbHasExtendedBullets = sal_False;
	mbFixedCellHeightUsed = sal_False;
}

ImplTextObj::~ImplTextObj()
{
    for ( ParagraphObj* pPtr = (ParagraphObj*)mpList->First(); pPtr; pPtr = (ParagraphObj*)mpList->Next() )
        delete pPtr;
    delete mpList;
}

TextObj::TextObj( ::com::sun::star::uno::Reference< ::com::sun::star::text::XSimpleText > & rXTextRef,
            int nInstance, FontCollection& rFontCollection, PPTExBulletProvider& rProv )
{
    mpImplTextObj = new ImplTextObj( nInstance );

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess >
        aXTextParagraphEA( rXTextRef, ::com::sun::star::uno::UNO_QUERY );

    if ( aXTextParagraphEA.is()  )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >
            aXTextParagraphE( aXTextParagraphEA->createEnumeration() );
        if ( aXTextParagraphE.is() )
        {
            ParaFlags aParaFlags;
            while ( aXTextParagraphE->hasMoreElements() )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > aXParagraph;
                ::com::sun::star::uno::Any aAny( aXTextParagraphE->nextElement() );
                if ( aAny >>= aXParagraph )
                {
                    if ( !aXTextParagraphE->hasMoreElements() )
                        aParaFlags.bLastParagraph = sal_True;
                    ParagraphObj* pPara = new ParagraphObj( aXParagraph, aParaFlags, rFontCollection, rProv );
                    mpImplTextObj->mbHasExtendedBullets |= pPara->bExtendedBulletsUsed;
                    mpImplTextObj->mpList->Insert( pPara, LIST_APPEND );
                    aParaFlags.bFirstParagraph = sal_False;
                }
            }
        }
    }
    ImplCalculateTextPositions();
}

TextObj::TextObj( const TextObj& rTextObj )
{
    mpImplTextObj = rTextObj.mpImplTextObj;
    mpImplTextObj->mnRefCount++;
}

TextObj::~TextObj()
{
    if ( ! ( --mpImplTextObj->mnRefCount ) )
        delete mpImplTextObj;
}

void TextObj::Write( SvStream* pStrm )
{
    sal_uInt32 nSize, nPos = pStrm->Tell();
    *pStrm << (sal_uInt32)( EPP_TextCharsAtom << 16 ) << (sal_uInt32)0;
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        ((ParagraphObj*)pPtr)->Write( pStrm );
    nSize = pStrm->Tell() - nPos;
    pStrm->SeekRel( - ( (sal_Int32)nSize - 4 ) );
    *pStrm << (sal_uInt32)( nSize - 8 );
    pStrm->SeekRel( nSize - 8 );
}

void TextObj::ImplCalculateTextPositions()
{
    mpImplTextObj->mnTextSize = 0;
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        mpImplTextObj->mnTextSize += ((ParagraphObj*)pPtr)->ImplCalculateTextPositions( mpImplTextObj->mnTextSize );
}

TextObj& TextObj::operator=( TextObj& rTextObj )
{
    if ( this != &rTextObj )
    {
        if ( ! ( --mpImplTextObj->mnRefCount ) )
            delete mpImplTextObj;
        mpImplTextObj = rTextObj.mpImplTextObj;
        mpImplTextObj->mnRefCount++;
    }
    return *this;
}

void TextObj::WriteTextSpecInfo( SvStream* pStrm )
{
	sal_uInt32 nCharactersLeft( Count() );
	if ( nCharactersLeft >= 1 )
	{
		EscherExAtom aAnimationInfoAtom( *pStrm, EPP_TextSpecInfoAtom, 0, 0 );
		for ( ParagraphObj* pPtr = static_cast < ParagraphObj * >( First() ); nCharactersLeft && pPtr; pPtr = static_cast< ParagraphObj* >( Next() ) )
		{
			for ( PortionObj* pPortion = static_cast< PortionObj* >( pPtr->First() ); nCharactersLeft && pPortion; pPortion = static_cast< PortionObj* >( pPtr->Next() ) )
			{
				sal_Int32 nPortionSize = pPortion->mnTextSize >= nCharactersLeft ? nCharactersLeft : pPortion->mnTextSize;
				sal_Int32 nFlags = 7;
				nCharactersLeft -= nPortionSize;
				*pStrm  << static_cast< sal_uInt32 >( nPortionSize )
						<< nFlags
						<< static_cast< sal_Int16 >( 1 )	// spellinfo -> needs rechecking
						<< static_cast< sal_Int16 >( MsLangId::convertLocaleToLanguageWithFallback( pPortion->meCharLocale ) )
						<< static_cast< sal_Int16 >( 0 );	// alt language
			}
		}
		if ( nCharactersLeft )
			*pStrm << nCharactersLeft << static_cast< sal_Int32 >( 1 ) << static_cast< sal_Int16 >( 1 );

	}
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplAdjustFirstLineLineSpacing( TextObj& rTextObj, EscherPropertyContainer& rPropOpt )
{
	if ( !mbFontIndependentLineSpacing )
	{
		ParagraphObj* pPara = rTextObj.First();
		if ( pPara )
		{
			PortionObj* pPortion = (PortionObj*)pPara->First();
			if ( pPortion )
			{
				sal_Int16 nLineSpacing = pPara->mnLineSpacing;
				const FontCollectionEntry* pDesc = maFontCollection.GetById( pPortion->mnFont );
				if ( pDesc )
					 nLineSpacing = (sal_Int16)( (double)nLineSpacing * pDesc->Scaling + 0.5 );

				if ( ( nLineSpacing > 0 ) && ( nLineSpacing < 100 ) )
				{
	/*
					if ( rxText.is() )
					{
						::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape( rxText, ::com::sun::star::uno::UNO_QUERY );
						if ( xShape.is() )
						{
							SdrObject* pObj = GetSdrObjectFromXShape( mXShape );
							if ( pObj )
							{
								const OutlinerParaObject* pParaObj = pObj->GetOutlinerParaObject();
								if ( pParaObj )
								{
									SdrModel* pModel = pObj->GetModel();
									if ( pModel )
									{
										Outliner aOutliner( &pModel->GetItemPool(), pParaObj->GetOutlinerMode() );
										aOutliner.SetText( *pParaObj );
										sal_uLong nTextHeight = aOutliner.GetLineHeight( 0, 0 );
										if ( nTextHeight )
										{
										}
									}
								}
							}
						}
					}
	*/
					double fCharHeight = pPortion->mnCharHeight;
					fCharHeight *= 2540 / 72;
					fCharHeight *= 100 - nLineSpacing;
					fCharHeight /= 100;

					sal_uInt32 nUpperDistance = 0;
					rPropOpt.GetOpt( ESCHER_Prop_dyTextTop, nUpperDistance );
					nUpperDistance += static_cast< sal_uInt32 >( fCharHeight * 360.0 );
					rPropOpt.AddOpt( ESCHER_Prop_dyTextTop, nUpperDistance );
				}
			}
		}
	}
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplWriteTextStyleAtom( SvStream& rOut, int nTextInstance, sal_uInt32 nAtomInstance,
	TextRuleEntry* pTextRule, SvStream& rExtBuStr, EscherPropertyContainer* pPropOpt )
{
    PPTExParaSheet& rParaSheet = mpStyleSheet->GetParaSheet( nTextInstance );

    rOut << (sal_uInt32)( ( EPP_TextHeaderAtom << 16 ) | ( nAtomInstance << 4 ) ) << (sal_uInt32)4
         << nTextInstance;

	if ( mbEmptyPresObj )
        mnTextSize = 0;
    if ( !mbEmptyPresObj )
    {
        ParagraphObj* pPara;
        TextObj aTextObj( mXText, nTextInstance, maFontCollection, (PPTExBulletProvider&)*this );

        // leaving out EPP_TextCharsAtom w/o text - still write out
        // attribute info though
        if ( mnTextSize )
            aTextObj.Write( &rOut );

		if ( pPropOpt )
			ImplAdjustFirstLineLineSpacing( aTextObj, *pPropOpt );

		sal_uInt32 nSize, nPos = rOut.Tell();

        rOut << (sal_uInt32)( EPP_StyleTextPropAtom << 16 ) << (sal_uInt32)0;
        ImplWriteParagraphs( rOut, aTextObj );
        ImplWritePortions( rOut, aTextObj );
        nSize = rOut.Tell() - nPos;
        rOut.SeekRel( - ( (sal_Int32)nSize - 4 ) );
        rOut << (sal_uInt32)( nSize - 8 );
        rOut.SeekRel( nSize - 8 );

        for ( pPara = aTextObj.First(); pPara; pPara = aTextObj.Next() )
        {
            for ( PortionObj* pPortion = (PortionObj*)pPara->First(); pPortion; pPortion = (PortionObj*)pPara->Next() )
            {
                if ( pPortion->mpFieldEntry )
                {
                    const FieldEntry* pFieldEntry = pPortion->mpFieldEntry;

                    switch ( pFieldEntry->nFieldType >> 28 )
                    {
                        case 1 :
                        case 2 :
                        {
                            rOut << (sal_uInt32)( EPP_DateTimeMCAtom << 16 ) << (sal_uInt32)8
                                 << (sal_uInt32)( pFieldEntry->nFieldStartPos )			// TxtOffset auf TxtField;
                                 << (sal_uInt8)( pFieldEntry->nFieldType & 0xff )		// Type
                                 << (sal_uInt8)0 << (sal_uInt16)0;						// PadBytes
                        }
                        break;
                        case 3 :
                        {
                            rOut << (sal_uInt32)( EPP_SlideNumberMCAtom << 16 ) << (sal_uInt32 ) 4
                                 << (sal_uInt32)( pFieldEntry->nFieldStartPos );
                        }
                        break;
                        case 4 :
                        {
							sal_uInt32 nPageIndex = 0;
							String aPageUrl;
							String aEmpty;
							String aFile( pFieldEntry->aFieldUrl );
							INetURLObject aUrl( pFieldEntry->aFieldUrl );
							if ( INET_PROT_FILE == aUrl.GetProtocol() )
								aFile = aUrl.PathToFileName();
							else if ( pFieldEntry->aFieldUrl.GetChar( 0 ) == '#' )
							{
								String aPage( INetURLObject::decode( pFieldEntry->aFieldUrl, '%', INetURLObject::DECODE_WITH_CHARSET ) );
								aPage.Erase( 0, 1 );
								for ( String* pStr = (String*)maSlideNameList.First(); pStr; pStr = (String*)maSlideNameList.Next(), nPageIndex++ )
								{
									if ( *pStr == aPage )
									{
										aPageUrl = UniString::CreateFromInt32( 256 + nPageIndex );
										aPageUrl.Append( String( RTL_CONSTASCII_USTRINGPARAM( "," ) ) );
										aPageUrl.Append( String::CreateFromInt32( nPageIndex + 1 ) );
										aPageUrl.Append( String( RTL_CONSTASCII_USTRINGPARAM( ",Slide " ) ) );
										aPageUrl.Append( String::CreateFromInt32( nPageIndex + 1 ) );
									}
								}
							}
							sal_uInt32 nHyperId;
							if ( aPageUrl.Len() )
								nHyperId = ImplInsertBookmarkURL( aPageUrl, 1 | ( nPageIndex << 8 ) | ( 1 << 31 ), pFieldEntry->aRepresentation, aEmpty, aEmpty, aPageUrl );
							else
								nHyperId = ImplInsertBookmarkURL( pFieldEntry->aFieldUrl, 2 | ( nHyperId << 8 ), aFile, pFieldEntry->aFieldUrl, aEmpty, aEmpty );

                            rOut << (sal_uInt32)( ( EPP_InteractiveInfo << 16 ) | 0xf ) << (sal_uInt32)24
                                 << (sal_uInt32)( EPP_InteractiveInfoAtom << 16 ) << (sal_uInt32)16
                                 << (sal_uInt32)0									// soundref
                                 << nHyperId										// hyperlink id
                                 << (sal_uInt8)4									// hyperlink action
                                 << (sal_uInt8)0                                    // ole verb
                                 << (sal_uInt8)0                                    // jump
                                 << (sal_uInt8)0                                    // flags
                                 << (sal_uInt8)8                                    // hyperlink type ?
                                 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0
                                 << (sal_uInt32)( EPP_TxInteractiveInfoAtom << 16 ) << (sal_uInt32)8
                                 << (sal_uInt32)( pFieldEntry->nFieldStartPos )
                                 << (sal_uInt32)( pFieldEntry->nFieldEndPos );
                        }
						break;
						case 5 :
						{
                            rOut << (sal_uInt32)( EPP_GenericDateMCAtom << 16 ) << (sal_uInt32)4
                                 << (sal_uInt32)( pFieldEntry->nFieldStartPos );
						}
						break;
						case 6 :
						{
                            rOut << (sal_uInt32)( EPP_HeaderMCAtom << 16 ) << (sal_uInt32 ) 4
                                 << (sal_uInt32)( pFieldEntry->nFieldStartPos );
						}
						break;
						case 7 :
						{
                            rOut << (sal_uInt32)( EPP_FooterMCAtom << 16 ) << (sal_uInt32 ) 4
                                 << (sal_uInt32)( pFieldEntry->nFieldStartPos );
						}
						break;
                        default:
                        break;
                    }
                }
            }
        }

		aTextObj.WriteTextSpecInfo( &rOut );

        // Star Office Default TabSizes schreiben ( wenn noetig )
        pPara = aTextObj.First();
        if ( pPara )
        {
            sal_uInt32  nParaFlags = 0x1f;
            sal_Int16   nDepth, nMask, nNumberingRule[ 10 ];
            sal_uInt32  nTextOfs = pPara->nTextOfs;
            sal_uInt32  nTabs = pPara->maTabStop.getLength();
            const ::com::sun::star::style::TabStop* pTabStop = ( const ::com::sun::star::style::TabStop* )pPara->maTabStop.getConstArray();

            for ( ; pPara; pPara = aTextObj.Next() )
            {
                if ( pPara->bExtendedParameters )
                {
                    nDepth = pPara->nDepth;
                    if ( nDepth < 5 )
                    {
                        nMask = 1 << nDepth;
                        if ( nParaFlags & nMask )
                        {
                            nParaFlags &=~ nMask;
                            if ( ( rParaSheet.maParaLevel[ nDepth ].mnTextOfs != pPara->nTextOfs ) ||
                                ( rParaSheet.maParaLevel[ nDepth ].mnBulletOfs != pPara->nBulletOfs ) )
                            {
                                nParaFlags |= nMask << 16;
                                nNumberingRule[ nDepth << 1 ] = pPara->nTextOfs;
                                nNumberingRule[ ( nDepth << 1 ) + 1 ] = (sal_Int16)pPara->nBulletOfs;
                            }
                        }
                    }
                }
            }
            nParaFlags >>= 16;

            sal_uInt32  nDefaultTabSize = ImplMapSize( ::com::sun::star::awt::Size( 2011, 1 ) ).Width;
            sal_uInt32  nDefaultTabs = abs( maRect.GetWidth() ) / nDefaultTabSize;
            if ( nTabs )
                nDefaultTabs -= (sal_Int32)( ( ( pTabStop[ nTabs - 1 ].Position / 4.40972 ) + nTextOfs ) / nDefaultTabSize );
            if ( (sal_Int32)nDefaultTabs < 0 )
                nDefaultTabs = 0;

            sal_uInt32 nTabCount = nTabs + nDefaultTabs;
            sal_uInt32 i, nTextRulerAtomFlags = 0;

            if ( nTabCount )
                nTextRulerAtomFlags |= 4;
            if ( nParaFlags )
                nTextRulerAtomFlags |= ( ( nParaFlags << 3 ) | ( nParaFlags << 8 ) );

            if ( nTextRulerAtomFlags )
            {
                SvStream* pRuleOut = &rOut;
                if ( pTextRule )
                    pRuleOut = pTextRule->pOut = new SvMemoryStream( 0x100, 0x100 );

                sal_uInt32 nRulePos = pRuleOut->Tell();
                *pRuleOut << (sal_uInt32)( EPP_TextRulerAtom << 16 ) << (sal_uInt32)0;
                *pRuleOut << nTextRulerAtomFlags;
                if ( nTextRulerAtomFlags & 4 )
                {
                    *pRuleOut << (sal_uInt16)nTabCount;
                    for ( i = 0; i < nTabs; i++ )
                    {
                        sal_uInt16 nPosition = (sal_uInt16)( ( pTabStop[ i ].Position / 4.40972 ) + nTextOfs );
                        sal_uInt16 nType;
                        switch ( pTabStop[ i ].Alignment )
                        {
                            case ::com::sun::star::style::TabAlign_DECIMAL :    nType = 3; break;
                            case ::com::sun::star::style::TabAlign_RIGHT :      nType = 2; break;
                            case ::com::sun::star::style::TabAlign_CENTER :     nType = 1; break;

                            case ::com::sun::star::style::TabAlign_LEFT :
                            default:                                            nType = 0;
                        };
                        *pRuleOut << nPosition
                                  << nType;
                    }

                    sal_uInt32 nWidth = 1;
                    if ( nTabs )
                        nWidth += (sal_Int32)( ( ( pTabStop[ nTabs - 1 ].Position / 4.40972 + nTextOfs ) / nDefaultTabSize ) );
                    nWidth *= nDefaultTabSize;
                    for ( i = 0; i < nDefaultTabs; i++, nWidth += nDefaultTabSize )
                        *pRuleOut << nWidth;
                }
                for ( i = 0; i < 5; i++ )
                {
                    if ( nTextRulerAtomFlags & ( 8 << i ) )
                        *pRuleOut << nNumberingRule[ i << 1 ];
                    if ( nTextRulerAtomFlags & ( 256 << i ) )
                        *pRuleOut << nNumberingRule[ ( i << 1 ) + 1 ];
                }
                sal_uInt32 nBufSize = pRuleOut->Tell() - nRulePos;
                pRuleOut->SeekRel( - ( (sal_Int32)nBufSize - 4 ) );
                *pRuleOut << (sal_uInt32)( nBufSize - 8 );
                pRuleOut->SeekRel( nBufSize - 8 );
            }
        }
        if ( aTextObj.HasExtendedBullets() )
        {
            ParagraphObj* pBulletPara = aTextObj.First();
            if ( pBulletPara )
            {
                sal_uInt32  nBulletFlags = 0;
                sal_uInt32  nNumberingType = 0, nPos2 = rExtBuStr.Tell();

                rExtBuStr << (sal_uInt32)( EPP_PST_ExtendedParagraphAtom << 16 ) << (sal_uInt32)0;

                for ( ; pBulletPara; pBulletPara = aTextObj.Next() )
                {
                    nBulletFlags = 0;
                    sal_uInt16 nBulletId = pBulletPara->nBulletId;
                    if ( pBulletPara->bExtendedBulletsUsed )
                    {
                        nBulletFlags = 0x800000;
                        if ( pBulletPara->nNumberingType != SVX_NUM_BITMAP )
                            nBulletFlags = 0x3000000;
                    }
                    rExtBuStr << (sal_uInt32)nBulletFlags;

                    if ( nBulletFlags & 0x800000 )
                        rExtBuStr << nBulletId;
                    if ( nBulletFlags & 0x1000000 )
                    {
                        switch( pBulletPara->nNumberingType )
                        {
                            case SVX_NUM_NUMBER_NONE :
                            case SVX_NUM_CHAR_SPECIAL :
                                nNumberingType = 0;
                            break;
                            case SVX_NUM_CHARS_UPPER_LETTER :
                            case SVX_NUM_CHARS_UPPER_LETTER_N :
                            case SVX_NUM_CHARS_LOWER_LETTER :
                            case SVX_NUM_CHARS_LOWER_LETTER_N :
                            case SVX_NUM_ROMAN_UPPER :
                            case SVX_NUM_ROMAN_LOWER :
                            case SVX_NUM_ARABIC :
                                nNumberingType = pBulletPara->nMappedNumType;
                            break;

    //                      case SVX_NUM_PAGEDESC :
                            case SVX_NUM_BITMAP :
                                nNumberingType = 0;
                            break;

                        }
                        rExtBuStr << (sal_uInt32)nNumberingType;
                    }
                    if ( nBulletFlags & 0x2000000 )
                        rExtBuStr << (sal_uInt16)pBulletPara->nStartWith;
                    rExtBuStr << (sal_uInt32)0 << (sal_uInt32)0;
                }
                sal_uInt32 nBulletSize = ( rExtBuStr.Tell() - nPos2 ) - 8;
                rExtBuStr.SeekRel( - ( (sal_Int32)nBulletSize + 4 ) );
                rExtBuStr << nBulletSize;
                rExtBuStr.SeekRel( nBulletSize );
            }
        }
    }
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplWriteObjectEffect( SvStream& rSt,
    ::com::sun::star::presentation::AnimationEffect eAe,
    ::com::sun::star::presentation::AnimationEffect eTe,
    sal_uInt16 nOrder )
{
	EscherExContainer aAnimationInfo( rSt, EPP_AnimationInfo );
	EscherExAtom aAnimationInfoAtom( rSt, EPP_AnimationInfoAtom, 0, 1 );
	sal_uInt32  nDimColor = 0x7000000;  // color to use for dimming
	sal_uInt32  nFlags = 0x4400;        // set of flags that determine type of build
	sal_uInt32  nSoundRef = 0;          // 0 if storage is from clipboard. Otherwise index(ID) in SoundCollection list.
	sal_uInt32  nDelayTime = 0;         // delay before playing object
	sal_uInt16  nSlideCount = 1;        // number of slides to play object
	sal_uInt8   nBuildType = 1;         // type of build
	sal_uInt8   nFlyMethod = 0;         // animation effect( fly, zoom, appear, etc )
	sal_uInt8   nFlyDirection = 0;      // Animation direction( left, right, up, down, etc )
	sal_uInt8   nAfterEffect = 0;       // what to do after build
	sal_uInt8   nSubEffect = 0;         // build by word or letter
	sal_uInt8   nOleVerb = 0;           // Determines object's class (sound, video, other)

	if ( eAe == ::com::sun::star::presentation::AnimationEffect_NONE )
	{
		nBuildType = 0;
		eAe = eTe;
	}
	switch ( eAe )
	{
		case ::com::sun::star::presentation::AnimationEffect_NONE :
		break;
		case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_LEFT :
		{
			nFlyDirection = 2;
			nFlyMethod = 10;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_TOP :
		{
			nFlyDirection = 3;
			nFlyMethod = 10;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_RIGHT :
		{
			nFlyDirection = 0;
			nFlyMethod = 10;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_BOTTOM :
		{
			nFlyDirection = 1;
			nFlyMethod = 10;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_FADE_TO_CENTER :
		{
			nFlyDirection = 1;
			nFlyMethod = 11;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_CENTER :
		{
			nFlyDirection = 0;
			nFlyMethod = 11;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_LEFT :
		{
			nFlyDirection = 0;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_TOP :
		{
			nFlyDirection = 1;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_RIGHT :
		{
			nFlyDirection = 2;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_BOTTOM :
		{
			nFlyDirection = 3;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_VERTICAL_STRIPES :
		{
			nFlyDirection = 0;
			nFlyMethod = 2;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_STRIPES :
		{
			nFlyDirection = 1;
			nFlyMethod = 2;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_CLOCKWISE :
		{
			nFlyDirection = 1;
			nFlyMethod = 3;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_COUNTERCLOCKWISE :
		{
			nFlyDirection = 0;
			nFlyMethod = 3;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_UPPERLEFT :
		{
			nFlyDirection = 7;
			nFlyMethod = 9;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_UPPERRIGHT :
		{
			nFlyDirection = 6;
			nFlyMethod = 9;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_LOWERLEFT :
		{
			nFlyDirection = 5;
			nFlyMethod = 9;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_LOWERRIGHT :
		{
			nFlyDirection = 4;
			nFlyMethod = 9;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_CLOSE_VERTICAL :
		{
			nFlyDirection = 1;
			nFlyMethod = 13;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_CLOSE_HORIZONTAL :
		{
			nFlyDirection = 3;
			nFlyMethod = 13;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_OPEN_VERTICAL :
		{
			nFlyDirection = 0;
			nFlyMethod = 13;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_OPEN_HORIZONTAL :
		{
			nFlyDirection = 2;
			nFlyMethod = 13;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_PATH :
		{
			nFlyDirection = 28;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_LEFT :
		{
			nFlyDirection = 0;
			nFlyMethod = 1;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_TOP :
		{
			nFlyDirection = 0;
			nFlyMethod = 1;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_RIGHT :
		{
			nFlyDirection = 0;
			nFlyMethod = 1;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_BOTTOM :
		{
			nFlyDirection = 0;
			nFlyMethod = 1;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_SPIRALIN_LEFT :
		case ::com::sun::star::presentation::AnimationEffect_SPIRALIN_RIGHT :
		case ::com::sun::star::presentation::AnimationEffect_SPIRALOUT_LEFT :
		case ::com::sun::star::presentation::AnimationEffect_SPIRALOUT_RIGHT :
		{
			nFlyDirection = 0x1c;
			nFlyMethod = 0xc;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_DISSOLVE :
		{
			nFlyDirection = 0;
			nFlyMethod = 5;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_WAVYLINE_FROM_LEFT :
		{
			nFlyDirection = 2;
			nFlyMethod = 10;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_WAVYLINE_FROM_TOP :
		{
			nFlyDirection = 3;
			nFlyMethod = 10;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_WAVYLINE_FROM_RIGHT :
		{
			nFlyDirection = 0;
			nFlyMethod = 10;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_WAVYLINE_FROM_BOTTOM :
		{
			nFlyDirection = 1;
			nFlyMethod = 10;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_RANDOM :
		{
			nFlyDirection = 0;
			nFlyMethod = 1;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_VERTICAL_LINES :
		{
			nFlyDirection = 1;
			nFlyMethod = 8;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_LINES :
		{
			nFlyDirection = 0;
			nFlyMethod = 8;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_LEFT :
		{
			nFlyDirection = 2;
			nFlyMethod = 10;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_TOP :
		{
			nFlyDirection = 3;
			nFlyMethod = 10;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_RIGHT :
		{
			nFlyDirection = 0;
			nFlyMethod = 10;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_BOTTOM :
		{
			nFlyDirection = 1;
			nFlyMethod = 10;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_UPPERLEFT :
		{
			nFlyDirection = 7;
			nFlyMethod = 9;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_UPPERRIGHT :
		{
			nFlyDirection = 6;
			nFlyMethod = 9;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_LOWERLEFT :
		{
			nFlyDirection = 5;
			nFlyMethod = 9;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_LOWERRIGHT :
		{
			nFlyDirection = 4;
			nFlyMethod = 9;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_APPEAR :
		break;
		case ::com::sun::star::presentation::AnimationEffect_HIDE :
		{
			nFlyDirection = 0;
			nFlyMethod = 1;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_UPPERLEFT :
		{
			nFlyDirection = 4;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_UPPERRIGHT :
		{
			nFlyDirection = 5;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_LOWERRIGHT :
		{
			nFlyDirection = 7;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_LOWERLEFT :
		{
			nFlyDirection = 6;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_UPPERLEFT :
		case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_UPPERRIGHT :
		case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_LOWERRIGHT :
		case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_LOWERLEFT :
			nAfterEffect |= 2;
		break;
		case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_LEFT :
		case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_UPPERLEFT :
		{
			nFlyDirection = 8;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_TOP :
		case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_UPPERRIGHT :
		{
			nFlyDirection = 11;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_RIGHT :
		case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_LOWERRIGHT :
		{
			nFlyDirection = 10;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_BOTTOM :
		case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_LOWERLEFT :
		{
			nFlyDirection = 9;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_TO_LEFT :
		case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_TO_UPPERLEFT :
		case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_TO_TOP :
		case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_TO_UPPERRIGHT :
		case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_TO_RIGHT :
		case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_TO_LOWERRIGHT :
		case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_TO_BOTTOM :
		case ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_TO_LOWERLEFT :
			nAfterEffect |= 2;
		break;
		case ::com::sun::star::presentation::AnimationEffect_VERTICAL_CHECKERBOARD :
		{
			nFlyDirection = 1;
			nFlyMethod = 3;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_CHECKERBOARD :
		{
			nFlyDirection = 0;
			nFlyMethod = 3;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_ROTATE :
		case ::com::sun::star::presentation::AnimationEffect_VERTICAL_ROTATE :
		{
			nFlyDirection = 27;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_STRETCH :
		case ::com::sun::star::presentation::AnimationEffect_VERTICAL_STRETCH :
		{
			nFlyDirection = 22;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_LEFT :
		case ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_UPPERLEFT :
		{
			nFlyDirection = 23;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_TOP :
		case ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_UPPERRIGHT :
		{
			nFlyDirection = 24;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_RIGHT :
		case ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_LOWERRIGHT :
		{
			nFlyDirection = 25;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_BOTTOM :
		case ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_LOWERLEFT :
		{
			nFlyDirection = 26;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN :
		{
			nFlyDirection = 16;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_SMALL :
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_SPIRAL :
		{
			nFlyDirection = 17;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT :
		{
			nFlyDirection = 18;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_SMALL :
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_SPIRAL :
		{
			nFlyDirection = 19;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_FROM_LEFT :
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_FROM_UPPERLEFT :
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_FROM_TOP :
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_FROM_UPPERRIGHT :
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_FROM_RIGHT :
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_FROM_LOWERRIGHT :
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_FROM_BOTTOM :
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_FROM_LOWERLEFT :
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_IN_FROM_CENTER :
		{
			nFlyDirection = 16;
			nFlyMethod = 12;
		}
		break;
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_FROM_LEFT :
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_FROM_UPPERLEFT :
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_FROM_TOP :
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_FROM_UPPERRIGHT :
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_FROM_RIGHT :
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_FROM_LOWERRIGHT :
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_FROM_BOTTOM :
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_FROM_LOWERLEFT :
		case ::com::sun::star::presentation::AnimationEffect_ZOOM_OUT_FROM_CENTER :
			nAfterEffect |= 2;
			break;
		default:
			break;
	}
	if ( mnDiaMode >= 1 )
		nFlags |= 4;
	if ( eTe != ::com::sun::star::presentation::AnimationEffect_NONE )
		nBuildType = 2;
	if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "SoundOn" ) ) ) )
	{
		sal_Bool bBool;
		mAny >>= bBool;
		if ( bBool )
		{
			if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Sound" ) ) ) )
			{
				nSoundRef = maSoundCollection.GetId( *(::rtl::OUString*)mAny.getValue() );
				if ( nSoundRef )
					nFlags |= 0x10;
			}
		}
	}
	sal_Bool bDimHide = sal_False;
	sal_Bool bDimPrevious = sal_False;
	if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "DimHide" ) ) ) )
		mAny >>= bDimHide;
	if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "DimPrevious" ) ) ) )
		mAny >>= bDimPrevious;
	if ( bDimPrevious )
		nAfterEffect |= 1;
	if ( bDimHide )
		nAfterEffect |= 2;
	if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "DimColor" ) ) ) )
		nDimColor = mpPptEscherEx->GetColor( *((sal_uInt32*)mAny.getValue()) ) | 0xfe000000;

	rSt << nDimColor << nFlags << nSoundRef << nDelayTime
		<< nOrder                                   // order of build ( 1.. )
		<< nSlideCount << nBuildType << nFlyMethod << nFlyDirection
		<< nAfterEffect << nSubEffect << nOleVerb
		<< (sal_uInt16)0;                               // PadWord
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplWriteClickAction( SvStream& rSt, ::com::sun::star::presentation::ClickAction eCa, sal_Bool bMediaClickAction )
{
    sal_uInt32 nSoundRef = 0;   // a reference to a sound in the sound collection, or NULL.
    sal_uInt32 nHyperLinkID = 0;// a persistent unique identifier to an external hyperlink object (only valid when action == HyperlinkAction).
    sal_uInt8   nAction = 0;     // Action See Action Table
    sal_uInt8   nOleVerb = 0;    // OleVerb Only valid when action == OLEAction. OLE verb to use, 0 = first verb, 1 = second verb, etc.
    sal_uInt8   nJump = 0;       // Jump See Jump Table
    sal_uInt8   nFlags = 0;      // Bit 1: Animated. If 1, then button is animated
                            // Bit 2: Stop sound. If 1, then stop current sound when button is pressed.
                            // Bit 3: CustomShowReturn. If 1, and this is a jump to custom show, then return to this slide after custom show.
    sal_uInt8   nHyperLinkType = 0;// HyperlinkType a value from the LinkTo enum, such as LT_URL (only valid when action == HyperlinkAction).

    String  aFile;

    /*
        Action Table:       Action Value
        NoAction            0
        MacroAction         1
        RunProgramAction    2
        JumpAction          3
        HyperlinkAction     4
        OLEAction           5
        MediaAction         6
        CustomShowAction    7

        Jump Table:     Jump Value
        NoJump          0
        NextSlide,      1
        PreviousSlide,  2
        FirstSlide,     3
        LastSlide,      4
        LastSlideViewed 5
        EndShow         6
    */

	if ( bMediaClickAction )
		nAction = 6;
	else switch( eCa )
    {
        case ::com::sun::star::presentation::ClickAction_STOPPRESENTATION :
            nJump += 2;
        case ::com::sun::star::presentation::ClickAction_LASTPAGE :
            nJump++;
        case ::com::sun::star::presentation::ClickAction_FIRSTPAGE :
            nJump++;
        case ::com::sun::star::presentation::ClickAction_PREVPAGE :
            nJump++;
        case ::com::sun::star::presentation::ClickAction_NEXTPAGE :
        {
            nJump++;
            nAction = 3;
        }
        break;
        case ::com::sun::star::presentation::ClickAction_SOUND :
        {
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Bookmark" ) ) ) )
                nSoundRef = maSoundCollection.GetId( *(::rtl::OUString*)mAny.getValue() );
        }
        break;
        case ::com::sun::star::presentation::ClickAction_PROGRAM :
        {
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Bookmark" ) ) ) )
            {
                INetURLObject aUrl( *(::rtl::OUString*)mAny.getValue() );
                if ( INET_PROT_FILE == aUrl.GetProtocol() )
                {
                    aFile = aUrl.PathToFileName();
                    nAction = 2;
                }
            }
        }
        break;

        case ::com::sun::star::presentation::ClickAction_BOOKMARK :
        {
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Bookmark" ) ) ) )
            {
                String  aBookmark( *(::rtl::OUString*)mAny.getValue() );
                sal_uInt32 nIndex = 0;
                for ( String* pStr = (String*)maSlideNameList.First(); pStr; pStr = (String*)maSlideNameList.Next(), nIndex++ )
                {
                    if ( *pStr == aBookmark )
                    {
                        // Bookmark ist ein link zu einer Dokumentseite
                        nAction = 4;
                        nHyperLinkType = 7;

						String aEmpty;
						String aHyperString = UniString::CreateFromInt32( 256 + nIndex );
                        aHyperString.Append( String( RTL_CONSTASCII_USTRINGPARAM( "," ) ) );
                        aHyperString.Append( String::CreateFromInt32( nIndex + 1 ) );
                        aHyperString.Append( String( RTL_CONSTASCII_USTRINGPARAM( ",Slide " ) ) );
                        aHyperString.Append( String::CreateFromInt32( nIndex + 1 ) );
						nHyperLinkID = ImplInsertBookmarkURL( aHyperString, 1 | ( nIndex << 8 ) | ( 1 << 31 ), aBookmark, aEmpty, aEmpty, aHyperString );
                    }
                }
            }
        }
        break;

        case ::com::sun::star::presentation::ClickAction_DOCUMENT :
		{
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Bookmark" ) ) ) )
            {
                String aBookmark( *(::rtl::OUString*)mAny.getValue() );
				if ( aBookmark.Len() )
				{
					nAction = 4;
					nHyperLinkType = 8;

					String aEmpty;
					String aBookmarkFile( aBookmark );
		            INetURLObject aUrl( aBookmark );
			        if ( INET_PROT_FILE == aUrl.GetProtocol() )
					    aBookmarkFile = aUrl.PathToFileName();
					nHyperLinkID = ImplInsertBookmarkURL( aBookmark, (sal_uInt32)(2 | ( 1 << 31 )), aBookmarkFile, aBookmark, aEmpty, aEmpty );
				}
			}
		}
		break;

        case ::com::sun::star::presentation::ClickAction_INVISIBLE :
        case ::com::sun::star::presentation::ClickAction_VERB :
        case ::com::sun::star::presentation::ClickAction_VANISH :
        case ::com::sun::star::presentation::ClickAction_MACRO :
        default :
        break;
    }

    sal_uInt32 nContainerSize = 24;
    if ( nAction == 2 )
        nContainerSize += ( aFile.Len() * 2 ) + 8;
    rSt << (sal_uInt32)( ( EPP_InteractiveInfo << 16 ) | 0xf ) << (sal_uInt32)nContainerSize
        << (sal_uInt32)( EPP_InteractiveInfoAtom << 16 ) << (sal_uInt32)16
        << nSoundRef
        << nHyperLinkID
        << nAction
        << nOleVerb
        << nJump
        << nFlags
        << (sal_uInt32)nHyperLinkType;

    if ( nAction == 2 )		// run program Action
    {
        sal_uInt16 i, nLen = aFile.Len();
        rSt << (sal_uInt32)( ( EPP_CString << 16 ) | 0x20 ) << (sal_uInt32)( nLen * 2 );
        for ( i = 0; i < nLen; i++ )
            rSt << aFile.GetChar( i );
    }

    rSt << (sal_uInt32)( ( EPP_InteractiveInfo << 16 ) | 0x1f ) << (sal_uInt32)24   // Mouse Over Action
        << (sal_uInt32)( EPP_InteractiveInfo << 16 ) << (sal_uInt32)16;
    for ( int i = 0; i < 4; i++, rSt << (sal_uInt32)0 ) ;
}

//  -----------------------------------------------------------------------

sal_Bool PPTWriter::ImplGetEffect( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rPropSet,
                                ::com::sun::star::presentation::AnimationEffect& eEffect,
                                ::com::sun::star::presentation::AnimationEffect& eTextEffect,
                                sal_Bool& bIsSound )
{
    ::com::sun::star::uno::Any aAny;
    if ( GetPropertyValue( aAny, rPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Effect" ) ) ) )
        aAny >>= eEffect;
    else
        eEffect = ::com::sun::star::presentation::AnimationEffect_NONE;

    if ( GetPropertyValue( aAny, rPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TextEffect" ) ) ) )
        aAny >>= eTextEffect;
    else
        eTextEffect = ::com::sun::star::presentation::AnimationEffect_NONE;
    if ( GetPropertyValue( aAny, rPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "SoundOn" ) ) ) )
        aAny >>= bIsSound;
    else
        bIsSound = sal_False;

    sal_Bool bHasEffect = ( ( eEffect != ::com::sun::star::presentation::AnimationEffect_NONE )
                        || ( eTextEffect != ::com::sun::star::presentation::AnimationEffect_NONE )
                        || bIsSound );
    return bHasEffect;
};

//  -----------------------------------------------------------------------

sal_Bool PPTWriter::ImplCreatePresentationPlaceholder( const sal_Bool bMasterPage, const PageType /* ePageType */,
														const sal_uInt32 nStyleInstance, const sal_uInt8 nPlaceHolderId )
{
	sal_Bool bRet = ImplGetText();
	if ( bRet && bMasterPage )
	{
		mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
        sal_uInt32 nPresShapeID = mpPptEscherEx->GenerateShapeId();
		mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, 0xa00, nPresShapeID );// Flags: HaveAnchor | HasSpt
		EscherPropertyContainer aPropOpt;
		aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x50001 );
		aPropOpt.AddOpt( ESCHER_Prop_lTxid, mnTxId += 0x60 );
		aPropOpt.AddOpt( ESCHER_Prop_AnchorText, ESCHER_AnchorMiddle );
		aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x110001 );
		aPropOpt.AddOpt( ESCHER_Prop_lineColor, 0x8000001 );
		aPropOpt.AddOpt( ESCHER_Prop_shadowColor, 0x8000002 );
		aPropOpt.CreateFillProperties( mXPropSet, sal_True );
		sal_uInt32 nLineFlags = 0x90001;
		if ( aPropOpt.GetOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags ) )
			nLineFlags |= 0x10001;  // draw dashed line if no line
		aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags );

		SvMemoryStream  aExtBu( 0x200, 0x200 );
		SvMemoryStream	aClientTextBox( 0x200, 0x200 );
        ImplWriteTextStyleAtom( aClientTextBox, nStyleInstance, 0, NULL, aExtBu, &aPropOpt );

		aPropOpt.CreateTextProperties( mXPropSet, mnTxId += 0x60, sal_False, sal_True );
		aPropOpt.CreateShapeProperties( mXShape );
		aPropOpt.Commit( *mpStrm );
		mpPptEscherEx->AddAtom( 8, ESCHER_ClientAnchor );
		*mpStrm << (sal_Int16)maRect.Top() << (sal_Int16)maRect.Left() << (sal_Int16)maRect.Right() << (sal_Int16)maRect.Bottom();      // oben, links, rechts, unten ????
		mpPptEscherEx->OpenContainer( ESCHER_ClientData );
		mpPptEscherEx->AddAtom( 8, EPP_OEPlaceholderAtom );
		*mpStrm << (sal_uInt32)0                // PlacementID
				<< (sal_uInt8)nPlaceHolderId	// PlaceHolderID
				<< (sal_uInt8)0					// Size of PlaceHolder ( 0 = FULL, 1 = HALF, 2 = QUARTER )
				<< (sal_uInt16)0;               // padword
		mpPptEscherEx->CloseContainer();		// ESCHER_ClientData
/*
		if ( aExtBu.Tell() )
        {
            if ( !pClientData )
                pClientData = new SvMemoryStream( 0x200, 0x200 );
            ImplProgTagContainer( pClientData, &aExtBu );
        }
*/
        if ( aClientTextBox.Tell() )
        {
            *mpStrm << (sal_uInt32)( ( ESCHER_ClientTextbox << 16 ) | 0xf )
                    << (sal_uInt32)aClientTextBox.Tell();

            mpStrm->Write( aClientTextBox.GetData(), aClientTextBox.Tell() );
        }
		mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer
	}
	else
		bRet = sal_False;
	return bRet;
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplCreateShape( sal_uInt32 nType, sal_uInt32 nFlags, EscherSolverContainer& rSolver )
{
    sal_uInt32 nId = mpPptEscherEx->GenerateShapeId();
    mpPptEscherEx->AddShape( nType, nFlags, nId );
    rSolver.AddShape( mXShape, nId );
}

void PPTWriter::ImplCreateTextShape( EscherPropertyContainer& rPropOpt, EscherSolverContainer& rSolver, sal_Bool bFill )
{
    mnTextStyle = EPP_TEXTSTYLE_TEXT;
    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
    ImplCreateShape( ESCHER_ShpInst_TextBox, 0xa00, rSolver );
    if ( bFill )
        rPropOpt.CreateFillProperties( mXPropSet, sal_True );
	if ( ImplGetText() )
		rPropOpt.CreateTextProperties( mXPropSet, mnTxId += 0x60, sal_False, sal_True );
}

void PPTWriter::ImplWritePage( const PHLayout& rLayout, EscherSolverContainer& aSolverContainer, PageType ePageType, sal_Bool bMasterPage, int nPageNumber )
{
    sal_uInt32  nInstance, nGroups, nShapes, nShapeCount, nPer, nLastPer, nIndices, nGroupLevel = 0, nOlePictureId;
    sal_uInt16  nEffectCount;
    ::com::sun::star::awt::Point   aTextRefPoint;

    ResetGroupTable( nShapes = mXShapes->getCount() );

    nIndices = nInstance = nLastPer = nShapeCount = nEffectCount = 0;

    sal_Bool bIsTitlePossible = sal_True;           // bei mehr als einem title geht powerpoint in die knie

    sal_uInt32  nOutlinerCount = 0;             // die gliederungsobjekte muessen dem layout entsprechen,
    sal_uInt32  nPrevTextStyle = 0;                // es darf nicht mehr als zwei geben

    nOlePictureId = 0;

    sal_Bool bAdditionalText = sal_False;

	sal_Bool bSecOutl = sal_False;
	sal_uInt32 nPObjects = 0;

    SvMemoryStream* pClientTextBox = NULL;
    SvMemoryStream* pClientData = NULL;

    while( GetNextGroupEntry() )
    {
        nShapeCount++;

        nPer = ( 5 * nShapeCount ) / nShapes;
        if ( nPer != nLastPer )
        {
            nLastPer = nPer;
            sal_uInt32 nValue = mnPagesWritten * 5 + nPer;
            if ( nValue > mnStatMaxValue )
                nValue = mnStatMaxValue;
            if ( mbStatusIndicator && ( nValue > mnLatestStatValue ) )
            {
                mXStatusIndicator->setValue( nValue );
                mnLatestStatValue = nValue;
            }
        }
        nGroups = GetGroupsClosed();
        for ( sal_uInt32 i = 0; i < nGroups; i++, mpPptEscherEx->LeaveGroup() ) ;

        if ( ImplGetShapeByIndex( GetCurrentGroupIndex(), sal_True ) )
        {
            sal_Bool bIsSound;
			sal_Bool bMediaClickAction = sal_False;
            ::com::sun::star::presentation::AnimationEffect eAe;
            ::com::sun::star::presentation::AnimationEffect eTe;

            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "PresentationOrder" ) ) ) )
                nEffectCount = *(sal_uInt16*)mAny.getValue();

            sal_Bool bEffect = ImplGetEffect( mXPropSet, eAe, eTe, bIsSound );
			::com::sun::star::presentation::ClickAction eCa = ::com::sun::star::presentation::ClickAction_NONE;
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "OnClick" ) ) ) )
                mAny >>= eCa;

            sal_Bool bGroup = mType == "drawing.Group";
            sal_Bool bOpenBezier   = mType == "drawing.OpenBezier";
            sal_Bool bClosedBezier = mType == "drawing.ClosedBezier";
            sal_Bool bPolyPolygon  = mType == "drawing.PolyPolygon";
            sal_Bool bPolyLine = mType == "drawing.PolyLine";

			List        aAdjustmentList;
            Rectangle   aPolyBoundRect;

            const ::com::sun::star::awt::Size   aSize100thmm( mXShape->getSize() );
            const ::com::sun::star::awt::Point  aPoint100thmm( mXShape->getPosition() );
            Rectangle   aRect100thmm( Point( aPoint100thmm.X, aPoint100thmm.Y ), Size( aSize100thmm.Width, aSize100thmm.Height ) );
            EscherPropertyContainer aPropOpt( mpPptEscherEx->GetGraphicProvider(), mpPicStrm, aRect100thmm );

            if ( bGroup )
            {
                SvMemoryStream* pTmp = NULL;
                ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >
                    aXIndexAccess( mXShape, ::com::sun::star::uno::UNO_QUERY );
                if ( EnterGroup( aXIndexAccess ) )
                {
                    if ( bEffect && !mbUseNewAnimations )
                    {
                        pTmp = new SvMemoryStream( 0x200, 0x200 );
						ImplWriteObjectEffect( *pTmp, eAe, eTe, ++nEffectCount );
                    }
					if ( eCa != ::com::sun::star::presentation::ClickAction_NONE )
					{
						if ( !pTmp )
							pTmp = new SvMemoryStream( 0x200, 0x200 );
						ImplWriteClickAction( *pTmp, eCa, bMediaClickAction );
					}
                    sal_uInt32 nShapeId = mpPptEscherEx->EnterGroup( &maRect, pTmp );
					aSolverContainer.AddShape( mXShape, nShapeId );
                    delete pTmp;
                }
            }
            else
            {
                sal_Bool bIsFontwork = sal_False;
				sal_Bool bIsHatching = sal_False;
                ::com::sun::star::uno::Any aAny;
				::com::sun::star::drawing::FillStyle eFS;
                if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsFontwork" ) ), sal_True ) )
                    aAny >>= bIsFontwork;
				if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillStyle" ) ), sal_True ) )
				{
					aAny >>= eFS;
					bIsHatching = eFS == ::com::sun::star::drawing::FillStyle_HATCH;
				}
                if ( bIsHatching || bIsFontwork || ( mType == "drawing.Measure" ) || ( mType == "drawing.Caption" ) )
                {
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "BoundRect" ) ) ) )
                    {
                        ::com::sun::star::awt::Rectangle aRect( *(::com::sun::star::awt::Rectangle*)mAny.getValue() );
                        maPosition = ImplMapPoint( ::com::sun::star::awt::Point( aRect.X, aRect.Y ) );
                        maSize = ImplMapSize( ::com::sun::star::awt::Size( aRect.Width, aRect.Height ) );
                        maRect = Rectangle( Point( maPosition.X, maPosition.Y ), Size( maSize.Width, maSize.Height ) );
                    }
					mType = "drawing.dontknow";
                }
            }
            sal_uInt8 nPlaceHolderAtom = EPP_PLACEHOLDER_NONE;

            mnTextSize = 0;
            mnTextStyle = EPP_TEXTSTYLE_NORMAL;

            if ( mType == "drawing.Custom" )
            {
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
				sal_uInt32 nMirrorFlags;
				rtl::OUString sCustomShapeType;
				MSO_SPT eShapeType = aPropOpt.GetCustomShapeType( mXShape, nMirrorFlags, sCustomShapeType );
				if ( sCustomShapeType.equalsAscii( "col-502ad400" ) || sCustomShapeType.equalsAscii( "col-60da8460" ) )
				{	// sj: creating metafile for customshapes that can't be saved to ms format properly
					ImplCreateShape( ESCHER_ShpInst_PictureFrame, 0xa00, aSolverContainer );
					if ( aPropOpt.CreateGraphicProperties( mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "MetaFile" ) ), sal_False ) )
					{
						aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
						SdrObject* pObj = GetSdrObjectFromXShape( mXShape );
						if ( pObj )
						{
							Rectangle aBound = pObj->GetCurrentBoundRect();
							maPosition = ImplMapPoint( ::com::sun::star::awt::Point( aBound.Left(), aBound.Top() ) );
					        maSize = ImplMapSize( ::com::sun::star::awt::Size ( aBound.GetWidth(), aBound.GetHeight() ) );
							maRect = Rectangle( Point( maPosition.X, maPosition.Y ), Size( maSize.Width, maSize.Height ) );
							mnAngle = 0;
						}
					}
				}
				else
				{
	                ImplCreateShape( eShapeType, nMirrorFlags | 0xa00, aSolverContainer );
					aPropOpt.CreateCustomShapeProperties( eShapeType, mXShape );
					aPropOpt.CreateFillProperties( mXPropSet, sal_True );
					if ( ImplGetText() )
					{
						if ( !aPropOpt.IsFontWork() )
							aPropOpt.CreateTextProperties( mXPropSet, mnTxId += 0x60, sal_True, sal_True );
					}
				}
            }
            else if ( mType == "drawing.Rectangle" )
            {
                sal_Int32 nRadius = 0;
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CornerRadius" ) ) ) )
                {
                    mAny >>= nRadius;
                    nRadius = ImplMapSize( ::com::sun::star::awt::Size( nRadius, 0 ) ).Width;
                }
                if ( nRadius )
                {
                    ImplCreateShape( ESCHER_ShpInst_RoundRectangle, 0xa00, aSolverContainer ); // Flags: Connector | HasSpt
                    sal_Int32 nLenght = maRect.GetWidth();
                    if ( nLenght > maRect.GetHeight() )
                        nLenght = maRect.GetHeight();
                    nLenght >>= 1;
                    if ( nRadius >= nLenght )
                        nRadius = 0x2a30;                           // 0x2a30 ist PPTs maximum radius
                    else
                        nRadius = ( 0x2a30 * nRadius ) / nLenght;
                    aPropOpt.AddOpt( ESCHER_Prop_adjustValue, nRadius );
                }
                else
                {
                    ImplCreateShape( ESCHER_ShpInst_Rectangle, 0xa00, aSolverContainer );          // Flags: Connector | HasSpt
                }
                aPropOpt.CreateFillProperties( mXPropSet, sal_True );
				if ( ImplGetText() )
					aPropOpt.CreateTextProperties( mXPropSet, mnTxId += 0x60, sal_False, sal_False );
            }
            else if ( mType == "drawing.Ellipse" )
            {
                ::com::sun::star::drawing::CircleKind  eCircleKind( ::com::sun::star::drawing::CircleKind_FULL );
                PolyStyle ePolyKind = POLY_CHORD;
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CircleKind" ) ) ) )
                {
                    mAny >>= eCircleKind;
                    switch ( eCircleKind )
                    {
                        case ::com::sun::star::drawing::CircleKind_SECTION :
                        {
                            ePolyKind = POLY_PIE;
                        }
                        break;
                        case ::com::sun::star::drawing::CircleKind_ARC :
                        {
                            ePolyKind = POLY_ARC;
                        }
                        break;

                        case ::com::sun::star::drawing::CircleKind_CUT :
                        {
                            ePolyKind = POLY_CHORD;
                        }
                        break;

                        default:
                            eCircleKind = ::com::sun::star::drawing::CircleKind_FULL;
                    }
                }
                if ( eCircleKind == ::com::sun::star::drawing::CircleKind_FULL )
                {
                    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                    ImplCreateShape( ESCHER_ShpInst_Ellipse, 0xa00, aSolverContainer );            // Flags: Connector | HasSpt
                    aPropOpt.CreateFillProperties( mXPropSet, sal_True );
					if ( ImplGetText() )
						aPropOpt.CreateTextProperties( mXPropSet, mnTxId += 0x60, sal_False, sal_False );
				}
                else
                {
                    sal_Int32 nStartAngle, nEndAngle;
                    if ( !ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CircleStartAngle" ) ) ) )
                        continue;
                    nStartAngle = *( (sal_Int32*)mAny.getValue() );
                    if( !ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CircleEndAngle" ) ) ) )
                        continue;
                    nEndAngle = *( (sal_Int32*)mAny.getValue() );
                    ::com::sun::star::awt::Point aPoint( mXShape->getPosition() );
                    ::com::sun::star::awt::Size  aSize( mXShape->getSize() );
                    ::com::sun::star::awt::Point aStart, aEnd, aCenter;
                    Rectangle aRect( Point( aPoint.X, aPoint.Y ), Size( aSize.Width, aSize.Height ) );
                    aStart.X = (sal_Int32)( ( cos( (double)( nStartAngle * F_PI18000 ) ) * 100.0 ) );
                    aStart.Y = - (sal_Int32)( ( sin( (double)( nStartAngle * F_PI18000 ) ) * 100.0 ) );
                    aEnd.X = (sal_Int32)( ( cos( (double)( nEndAngle * F_PI18000 ) ) * 100.0 ) );
                    aEnd.Y = - (sal_Int32)( ( sin( (double)( nEndAngle * F_PI18000 ) ) * 100.0 ) );
                    aCenter.X = aPoint.X + ( aSize.Width / 2 );
                    aCenter.Y = aPoint.Y + ( aSize.Height / 2 );
                    aStart.X += aCenter.X;
                    aStart.Y += aCenter.Y;
                    aEnd.X += aCenter.X;
                    aEnd.Y += aCenter.Y;
                    Polygon aPolygon( aRect, Point( aStart.X, aStart.Y ), Point( aEnd.X, aEnd.Y ), ePolyKind );
					sal_Bool bNeedText = sal_True;
					if ( mnAngle )
					{
						aPolygon.Rotate( aRect.TopLeft(), (sal_uInt16)( mnAngle / 10 ) );
						if ( ImplGetText() )
						{
							mpPptEscherEx->EnterGroup( 0,0 );
							nGroupLevel = mpPptEscherEx->GetGroupLevel();
							bNeedText = sal_False;
							bAdditionalText = sal_True;
							mnTextSize = 0;
						}
						mnAngle = 0;
					}
                    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                    ImplCreateShape( ESCHER_ShpInst_NotPrimitive, 0xa00, aSolverContainer );       // Flags: Connector | HasSpt
                    ::com::sun::star::awt::Rectangle aNewRect;
                    switch ( ePolyKind )
                    {
                        case POLY_PIE :
                        case POLY_CHORD :
                        {
                            if ( aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_POLYPOLYGON, sal_False, aNewRect, &aPolygon ) )
                                aPropOpt.CreateFillProperties( mXPropSet, sal_True );
                        }
                        break;

                        case POLY_ARC :
                        {
                            if ( aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_POLYLINE, sal_False, aNewRect, &aPolygon ) )
                                aPropOpt.CreateLineProperties( mXPropSet, sal_False );
                        }
                        break;
                    }
                    maRect = ImplMapRectangle( aNewRect );
                    maPosition = ::com::sun::star::awt::Point( maRect.Left(), maRect.Top() );
                    maSize = ::com::sun::star::awt::Size( maRect.GetWidth(), maRect.GetHeight() );
					if ( bNeedText && ImplGetText() )
						aPropOpt.CreateTextProperties( mXPropSet, mnTxId += 0x60, sal_False, sal_False );
                }
            }
            else if ( mType == "drawing.Control" )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XControlShape  >
                    aXControlShape( mXShape, ::com::sun::star::uno::UNO_QUERY );
                if ( !aXControlShape.is() )
                    continue;
                ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >
                    aXControlModel( aXControlShape->getControl() );
                if ( !aXControlModel.is() )
                    continue;

				sal_Int64 nAspect = ::com::sun::star::embed::Aspects::MSOLE_CONTENT;
				try
				{
					// try to get the aspect when available
					::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
						xShapeProps( mXShape, ::com::sun::star::uno::UNO_QUERY_THROW );
					xShapeProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Aspect" ) ) ) >>= nAspect;
				}
				catch( ::com::sun::star::uno::Exception& )
				{}

                *mpExEmbed  << (sal_uInt32)( 0xf | ( EPP_ExControl << 16 ) )
                            << (sal_uInt32)0;               // Size of this container

                sal_uInt32 nSize, nOldPos = mpExEmbed->Tell();

                sal_uInt32 nPageId = nPageNumber;
                if ( ePageType == MASTER )
                    nPageId |= 0x80000000;
                else
                    nPageId += 0x100;
                *mpExEmbed  << (sal_uInt32)( EPP_ExControlAtom << 16 )
                            << (sal_uInt32)4
                            << nPageId;
                PPTExOleObjEntry* pEntry = new PPTExOleObjEntry( OCX_CONTROL, mpExEmbed->Tell() );
                pEntry->xControlModel = aXControlModel;
                maExOleObj.Insert( pEntry );

                mnExEmbed++;

                *mpExEmbed  << (sal_uInt32)( 1 | ( EPP_ExOleObjAtom << 16 ) )
                            << (sal_uInt32)24
                            << (sal_uInt32)nAspect
                            << (sal_uInt32)2
                            << (sal_uInt32)mnExEmbed
                            << (sal_uInt32)0
                            << (sal_uInt32)4    // index to the persist table
                            << (sal_uInt32)0x0012de00;


                ::com::sun::star::awt::Size     aSize;
                String          aControlName;
                SvStorageRef    xTemp( new SvStorage( new SvMemoryStream(), sal_True ) );
                if ( SvxMSConvertOCXControls::WriteOCXStream( xTemp, aXControlModel, aSize, aControlName ) )
                {
                    String  aUserName( xTemp->GetUserName() );
                    String  aOleIdentifier;
                    if ( aUserName.Len() )
                    {
                        SvStorageStreamRef xCompObj = xTemp->OpenSotStream(
                            String( RTL_CONSTASCII_USTRINGPARAM( "\1CompObj" ) ),
                                STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYALL );
                        xCompObj->Seek( STREAM_SEEK_TO_END );
                        sal_uInt32  nStreamLen = xCompObj->Tell();
                        xCompObj->Seek( 0 );
                        sal_Int16   nVersion, nByteOrder;
                        sal_Int32   nWinVersion, nVal, nStringLen;
                        *xCompObj   >> nVersion
                                    >> nByteOrder
                                    >> nWinVersion
                                    >> nVal;
                        xCompObj->SeekRel( 16 );    // skipping clsid
                        *xCompObj   >> nStringLen;
                        if ( ( xCompObj->Tell() + nStringLen ) < nStreamLen )
                        {
                            xCompObj->SeekRel( nStringLen );        // now skipping the UserName;
                            *xCompObj >> nStringLen;
                            if ( ( xCompObj->Tell() + nStringLen ) < nStreamLen )
                            {
                                xCompObj->SeekRel( nStringLen );    // now skipping the clipboard formatname
                                *xCompObj   >> nStringLen;
                                if ( ( nStringLen > 1 ) && ( ( xCompObj->Tell() + nStringLen ) < nStreamLen ) )
                                {   // i think that the OleIdentifier will follow
                                    ByteString aTemp;
                                    sal_Char* p = aTemp.AllocBuffer( (sal_uInt16)(nStringLen - 1) );
                                    xCompObj->Read( p, nStringLen - 1 );
                                    aOleIdentifier = String( aTemp, gsl_getSystemTextEncoding() );
                                }
                            }
                        }
                    }
                    if ( aControlName.Len() )
                        PPTWriter::WriteCString( *mpExEmbed, aControlName, 1 );
                    if ( aOleIdentifier.Len() )
                        PPTWriter::WriteCString( *mpExEmbed, aOleIdentifier, 2 );
                    if ( aUserName.Len() )
                        PPTWriter::WriteCString( *mpExEmbed, aUserName, 3 );
                }
                nSize = mpExEmbed->Tell() - nOldPos;
                mpExEmbed->Seek( nOldPos - 4 );
                *mpExEmbed << nSize;
                mpExEmbed->Seek( STREAM_SEEK_TO_END );
                nOlePictureId = mnExEmbed;

                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                sal_uInt32 nSpFlags = SHAPEFLAG_HAVESPT | SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_OLESHAPE;
                ImplCreateShape( ESCHER_ShpInst_HostControl, nSpFlags, aSolverContainer );
                if ( aPropOpt.CreateGraphicProperties( mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "MetaFile" ) ), sal_False  ) )
                    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
                aPropOpt.AddOpt( ESCHER_Prop_pictureId, mnExEmbed );
                aPropOpt.AddOpt( ESCHER_Prop_pictureActive, 0x10000 );

                if ( aControlName.Len() )
                {
                    sal_uInt16 i, nBufSize;
                    nBufSize = ( aControlName.Len() + 1 ) << 1;
                    sal_uInt8* pBuf = new sal_uInt8[ nBufSize ];
                    sal_uInt8* pTmp = pBuf;
                    for ( i = 0; i < aControlName.Len(); i++ )
                    {
                        sal_Unicode nUnicode = aControlName.GetChar( i );
                        *pTmp++ = (sal_uInt8)nUnicode;
                        *pTmp++ = (sal_uInt8)( nUnicode >> 8 );
                    }
                    *pTmp++ = 0;
                    *pTmp = 0;
                    aPropOpt.AddOpt( ESCHER_Prop_wzName, sal_True, nBufSize, pBuf, nBufSize );
                }
            }
            else if ( mType == "drawing.Connector" )
            {
                sal_uInt16 nSpType, nSpFlags;
                ::com::sun::star::awt::Rectangle aNewRect;
                if ( aPropOpt.CreateConnectorProperties( mXShape, aSolverContainer, aNewRect, nSpType, nSpFlags ) == sal_False )
                    continue;

                maRect = ImplMapRectangle( aNewRect );
                maPosition = ::com::sun::star::awt::Point( maRect.Left(), maRect.Top() );
                maSize = ::com::sun::star::awt::Size( maRect.GetWidth(), maRect.GetHeight() );

                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ImplCreateShape( nSpType, nSpFlags, aSolverContainer );
            }
            else if ( mType == "drawing.Measure" )
            {
                continue;
            }
            else if ( mType == "drawing.Line" )
            {
                ::com::sun::star::awt::Rectangle aNewRect;
                aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_LINE, sal_False, aNewRect, NULL );
                maRect = ImplMapRectangle( aNewRect );
                maPosition = ::com::sun::star::awt::Point( maRect.Left(), maRect.Top() );
                maSize = ::com::sun::star::awt::Size( maRect.GetWidth(), maRect.GetHeight() );
                if ( ImplGetText() )
                {
                    aTextRefPoint = ::com::sun::star::awt::Point( maRect.Left(), maRect.Top() );
                    mnTextSize = 0;
                    bAdditionalText = sal_True;
                    mpPptEscherEx->EnterGroup( &maRect,0 );
                }
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                sal_uInt32 nFlags = 0xa00;                                  // Flags: Connector | HasSpt
                if ( maRect.Top() > maRect.Bottom() )
                    nFlags |= 0x80;                                         // Flags: VertMirror
                if ( maRect.Left() > maRect.Right() )
                    nFlags |= 0x40;                                         // Flags: HorzMirror

                ImplCreateShape( ESCHER_ShpInst_Line, nFlags, aSolverContainer );
                aPropOpt.AddOpt( ESCHER_Prop_shapePath, ESCHER_ShapeComplex );
                aPropOpt.CreateLineProperties( mXPropSet, sal_False );
                mnAngle = 0;
            }
            else if ( bPolyPolygon )
            {
                if ( ImplGetText() )
                {
                    mpPptEscherEx->EnterGroup( 0,0 );
                    nGroupLevel = mpPptEscherEx->GetGroupLevel();
                    bAdditionalText = sal_True;
                    mnTextSize = 0;
                }
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ImplCreateShape( ESCHER_ShpInst_NotPrimitive, 0xa00, aSolverContainer );            // Flags: Connector | HasSpt
                ::com::sun::star::awt::Rectangle aNewRect;
                aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_POLYPOLYGON, sal_False, aNewRect, NULL );
                maRect = ImplMapRectangle( aNewRect );
                maPosition = ::com::sun::star::awt::Point( maRect.Left(), maRect.Top() );
                maSize = ::com::sun::star::awt::Size( maRect.GetWidth(), maRect.GetHeight() );
                aPropOpt.CreateFillProperties( mXPropSet, sal_True );
                mnAngle = 0;
            }
            else if ( bPolyLine )
            {
                if ( ImplGetText() )
                {
                    mpPptEscherEx->EnterGroup( 0,0 );
                    nGroupLevel = mpPptEscherEx->GetGroupLevel();
                    bAdditionalText = sal_True;
                    mnTextSize = 0;
                }
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ImplCreateShape( ESCHER_ShpInst_NotPrimitive, 0xa00, aSolverContainer );            // Flags: Connector | HasSpt
                ::com::sun::star::awt::Rectangle aNewRect;
                aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_POLYLINE, sal_False, aNewRect, NULL );
                maRect = ImplMapRectangle( aNewRect );
                maPosition = ::com::sun::star::awt::Point( maRect.Left(), maRect.Top() );
                maSize = ::com::sun::star::awt::Size( maRect.GetWidth(), maRect.GetHeight() );
                aPropOpt.CreateLineProperties( mXPropSet, sal_False );
                mnAngle = 0;
            }
            else if ( bOpenBezier )
            {
                if ( ImplGetText() )
                {
                    mpPptEscherEx->EnterGroup( 0,0 );
                    nGroupLevel = mpPptEscherEx->GetGroupLevel();
                    bAdditionalText = sal_True;
                    mnTextSize = 0;
                }
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ImplCreateShape( ESCHER_ShpInst_NotPrimitive, 0xa00, aSolverContainer );            // Flags: Connector | HasSpt
                ::com::sun::star::awt::Rectangle aNewRect;
                aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_POLYLINE, sal_True, aNewRect, NULL );
                maRect = ImplMapRectangle( aNewRect );
                maPosition = ::com::sun::star::awt::Point( maRect.Left(), maRect.Top() );
                maSize = ::com::sun::star::awt::Size( maRect.GetWidth(), maRect.GetHeight() );
                aPropOpt.CreateLineProperties( mXPropSet, sal_False );
                mnAngle = 0;
            }
            else if ( bClosedBezier )
            {
                if ( ImplGetText() )
                {
                    mpPptEscherEx->EnterGroup( 0,0 );
                    nGroupLevel = mpPptEscherEx->GetGroupLevel();
                    bAdditionalText = sal_True;
                    mnTextSize = 0;
                }
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ImplCreateShape( ESCHER_ShpInst_NotPrimitive, 0xa00, aSolverContainer );            // Flags: Connector | HasSpt
                ::com::sun::star::awt::Rectangle aNewRect;
                aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_POLYPOLYGON, sal_True, aNewRect, NULL );
                maRect = ImplMapRectangle( aNewRect );
                maPosition = ::com::sun::star::awt::Point( maRect.Left(), maRect.Top() );
                maSize = ::com::sun::star::awt::Size( maRect.GetWidth(), maRect.GetHeight() );
                aPropOpt.CreateFillProperties( mXPropSet, sal_True );
                mnAngle = 0;
            }
            else if ( ( mType == "drawing.GraphicObject" ) || ( mType == "presentation.GraphicObject" ) )
            {
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );

                // ein GraphicObject kann auch ein ClickMe Element sein
                if ( mbEmptyPresObj && ( ePageType == NORMAL ) )
                {
                    nPlaceHolderAtom = rLayout.nUsedObjectPlaceHolder;
                    ImplCreateShape( ESCHER_ShpInst_Rectangle, 0x220, aSolverContainer );           // Flags: HaveAnchor | HaveMaster
                    aPropOpt.AddOpt( ESCHER_Prop_lTxid, mnTxId += 0x60 );
                    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x10001 );
                    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x10001 );
                    aPropOpt.AddOpt( ESCHER_Prop_hspMaster, mnShapeMasterBody );
                }
                else
                {
                    mXText = ::com::sun::star::uno::Reference<
                        ::com::sun::star::text::XSimpleText >
                            ( mXShape, ::com::sun::star::uno::UNO_QUERY );

                    if ( mXText.is() )
                        mnTextSize = mXText->getString().getLength();

                    if ( mnTextSize )                                       // graphic object oder Flachenfuellung
                    {
						/* SJ #i34951#: because M. documents are not allowing GraphicObjects containing text, we
						have to create a simpe Rectangle with fill bitmap instead (while not allowing BitmapMode_Repeat).
						*/
                        ImplCreateShape( ESCHER_ShpInst_Rectangle, 0xa00, aSolverContainer );       // Flags: Connector | HasSpt
                        if ( aPropOpt.CreateGraphicProperties( mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "GraphicURL" ) ), sal_True, sal_True, sal_False ) )
                        {
                            aPropOpt.AddOpt( ESCHER_Prop_WrapText, ESCHER_WrapNone );
                            aPropOpt.AddOpt( ESCHER_Prop_AnchorText, ESCHER_AnchorMiddle );
                            aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x140014 );
                            aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0x8000000 );
                            aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x80000 );
                            if ( ImplGetText() )
								aPropOpt.CreateTextProperties( mXPropSet, mnTxId += 0x60, sal_False, sal_False );
                        }
                    }
                    else
                    {
                        ImplCreateShape( ESCHER_ShpInst_PictureFrame, 0xa00, aSolverContainer );
                        if ( aPropOpt.CreateGraphicProperties( mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "GraphicURL" ) ), sal_False, sal_True ) )
                            aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
                    }
                }
            }
            else if ( ( mType == "drawing.Text" ) || ( mType == "presentation.Notes" ) )
            {
                if ( ( ePageType == NOTICE ) && mbPresObj )
                {
					if ( ImplCreatePresentationPlaceholder( bMasterPage, ePageType, EPP_TEXTTYPE_Notes, EPP_PLACEHOLDER_MASTERNOTESBODYIMAGE ) )
						continue;
					else
                        nPlaceHolderAtom = EPP_PLACEHOLDER_NOTESBODY;
                }
                ImplCreateTextShape( aPropOpt, aSolverContainer, sal_True );
            }
            else if ( mType == "presentation.TitleText" )
            {
                if ( mbPresObj )
                {
                    if ( ( ePageType == NOTICE ) && mbEmptyPresObj )
                    {
                        mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                        nPlaceHolderAtom = EPP_PLACEHOLDER_MASTERNOTESBODYIMAGE;
                        ImplCreateShape( ESCHER_ShpInst_Rectangle, 0x200, aSolverContainer );
                        aPropOpt.CreateLineProperties( mXPropSet, sal_False );
                        aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x10001 );
                    }
                    else if ( rLayout.bTitlePossible && bIsTitlePossible )
                    {
                        bIsTitlePossible = sal_False;

						ImplGetText();
						TextObj aTextObj( mXText, EPP_TEXTTYPE_Title, maFontCollection, (PPTExBulletProvider&)*this );
                        if ( ePageType == MASTER )
						{
							if ( mnTextSize )
							{
								::rtl::OUString aUString( mXText->getString() );
								sal_uInt16 nChar;

								mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                                mnShapeMasterTitle = mpPptEscherEx->GenerateShapeId();
								mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, 0xa00, mnShapeMasterTitle );// Flags: HaveAnchor | HasSpt
								EscherPropertyContainer aPropertyOptions;
								aPropertyOptions.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x50001 );
								aPropertyOptions.AddOpt( ESCHER_Prop_lTxid, mnTxId += 0x60 );
								aPropertyOptions.AddOpt( ESCHER_Prop_AnchorText, ESCHER_AnchorMiddle );
//								aPropertyOptions.AddOpt( ESCHER_Prop_fillColor, nFillColor );
//								aPropertyOptions.AddOpt( ESCHER_Prop_fillBackColor, nFillBackColor );
								aPropertyOptions.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x110001 );
								aPropertyOptions.AddOpt( ESCHER_Prop_lineColor, 0x8000001 );
								aPropertyOptions.AddOpt( ESCHER_Prop_shadowColor, 0x8000002 );
								aPropertyOptions.CreateFillProperties( mXPropSet, sal_True );
								sal_uInt32 nLineFlags = 0x90001;
								if ( aPropertyOptions.GetOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags ) )
									nLineFlags |= 0x10001;  // draw dashed line if no line
								aPropertyOptions.AddOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags );
								aPropertyOptions.CreateTextProperties( mXPropSet, mnTxId += 0x60, sal_False, sal_True );
								ImplAdjustFirstLineLineSpacing( aTextObj, aPropOpt );
								aPropertyOptions.Commit( *mpStrm );
								mpPptEscherEx->AddAtom( 8, ESCHER_ClientAnchor );
								*mpStrm << (sal_Int16)maRect.Top() << (sal_Int16)maRect.Left() << (sal_Int16)maRect.Right() << (sal_Int16)maRect.Bottom();      // oben, links, rechts, unten ????
								mpPptEscherEx->OpenContainer( ESCHER_ClientData );
								mpPptEscherEx->AddAtom( 8, EPP_OEPlaceholderAtom );
								*mpStrm << (sal_uInt32)0                                                        // PlacementID
										<< (sal_uInt8)EPP_PLACEHOLDER_MASTERTITLE                               // PlaceHolderID
										<< (sal_uInt8)0                                                         // Size of PlaceHolder ( 0 = FULL, 1 = HALF, 2 = QUARTER )
										<< (sal_uInt16)0;                                                       // padword
								mpPptEscherEx->CloseContainer();    // ESCHER_ClientData
								mpPptEscherEx->OpenContainer( ESCHER_ClientTextbox );
								mpPptEscherEx->AddAtom( 4, EPP_TextHeaderAtom );
								*mpStrm << (sal_uInt32)EPP_TEXTTYPE_Title;
								mpPptEscherEx->AddAtom( mnTextSize << 1, EPP_TextCharsAtom );
								const sal_Unicode* pString = aUString;
								for ( sal_uInt32 i = 0; i < mnTextSize; i++ )
								{
									nChar = pString[ i ];       // 0xa -> 0xb weicher Zeilenumbruch
									if ( nChar == 0xa )
										nChar++;                // 0xd -> 0xd harter Zeilenumbruch
									*mpStrm << nChar;
								}
								mpPptEscherEx->AddAtom( 6, EPP_BaseTextPropAtom );
								*mpStrm << (sal_uInt32)( mnTextSize + 1 ) << (sal_uInt16)0;
								mpPptEscherEx->AddAtom( 10, EPP_TextSpecInfoAtom );
								*mpStrm << (sal_uInt32)( mnTextSize + 1 ) << (sal_uInt32)1 << (sal_uInt16)0;
								mpPptEscherEx->CloseContainer();    // ESCHER_ClientTextBox
								mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer
							}
							continue;
						}
						else
						{
							mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
							mnTextStyle = EPP_TEXTSTYLE_TITLE;
							nPlaceHolderAtom = rLayout.nTypeOfTitle;
							ImplCreateShape( ESCHER_ShpInst_Rectangle, 0x220, aSolverContainer );          // Flags: HaveAnchor | HaveMaster
							aPropOpt.AddOpt( ESCHER_Prop_hspMaster, mnShapeMasterTitle );
							aPropOpt.CreateFillProperties( mXPropSet, sal_True );
							aPropOpt.CreateTextProperties( mXPropSet, mnTxId += 0x60, sal_False, sal_True );
							ImplAdjustFirstLineLineSpacing( aTextObj, aPropOpt );
							if ( mbEmptyPresObj )
							{
								sal_uInt32 nNoLineDrawDash = 0;
								aPropOpt.GetOpt( ESCHER_Prop_fNoLineDrawDash, nNoLineDrawDash );
								nNoLineDrawDash |= 0x10001;
								aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, nNoLineDrawDash );
							}
						}
                    }
                    else
                        mbPresObj = sal_False;
                }
                if ( !mbPresObj )
                {
                    mType = "drawing.Text";
					ImplCreateTextShape( aPropOpt, aSolverContainer, sal_True );
                }
            }
            else if ( ( mType == "presentation.Outliner" ) || ( mType == "presentation.Subtitle" ) )
            {
                if ( mbPresObj )
                {
                    nOutlinerCount++;
                    if ( (rLayout.bOutlinerPossible && ( nOutlinerCount == 1 )) ||
                         (( rLayout.bSecOutlinerPossible && ( nOutlinerCount == 2 ) ) && ( nPrevTextStyle == EPP_TEXTSTYLE_BODY ))
                       )
                    {
						ImplGetText();
						TextObj aTextObj( mXText, EPP_TEXTTYPE_Body, maFontCollection, (PPTExBulletProvider&)*this );
                        if ( ePageType == MASTER )
                        {
                            nPrevTextStyle = EPP_TEXTSTYLE_TITLE;
							if ( mnTextSize )
							{
								mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                                mnShapeMasterBody = mpPptEscherEx->GenerateShapeId();
								mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle, 0xa00, mnShapeMasterBody );  // Flags: HaveAnchor | HasSpt
								EscherPropertyContainer aPropOpt2;
								aPropOpt2.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x50001 );
								aPropOpt2.AddOpt( ESCHER_Prop_lTxid, mnTxId += 0x60 );
//								aPropOpt2.AddOpt( ESCHER_Prop_fillColor, nFillColor );
//								aPropOpt2.AddOpt( ESCHER_Prop_fillBackColor, nFillBackColor );
								aPropOpt2.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x110001 );
								aPropOpt2.AddOpt( ESCHER_Prop_lineColor, 0x8000001 );
								aPropOpt2.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90001 );
								aPropOpt2.AddOpt( ESCHER_Prop_shadowColor, 0x8000002 );
								aPropOpt2.CreateFillProperties( mXPropSet, sal_True );
								sal_uInt32 nLineFlags = 0x90001;
								if ( aPropOpt2.GetOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags ) )
									nLineFlags |= 0x10001;  // draw dashed line if no line
								aPropOpt2.AddOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags );
								aPropOpt2.CreateTextProperties( mXPropSet, mnTxId += 0x60, sal_False, sal_True );
								ImplAdjustFirstLineLineSpacing( aTextObj, aPropOpt2 );
								aPropOpt2.Commit( *mpStrm );
								mpPptEscherEx->AddAtom( 8, ESCHER_ClientAnchor );
								*mpStrm << (sal_Int16)maRect.Top() << (sal_Int16)maRect.Left() << (sal_Int16)maRect.Right() << (sal_Int16)maRect.Bottom();  // oben, links, rechts, unten ????
								mpPptEscherEx->OpenContainer( ESCHER_ClientData );
								mpPptEscherEx->AddAtom( 8, EPP_OEPlaceholderAtom );
								*mpStrm << (sal_uInt32)1                                                        // PlacementID
										<< (sal_uInt8)EPP_PLACEHOLDER_MASTERBODY                                    // PlaceHolderID
										<< (sal_uInt8)0                                                         // Size of PlaceHolder ( 0 = FULL, 1 = HALF, 2 = QUARTER )
										<< (sal_uInt16)0;                                                       // padword
								mpPptEscherEx->CloseContainer();    // ESCHER_ClientData
								mpPptEscherEx->OpenContainer( ESCHER_ClientTextbox );       // printf
								mpPptEscherEx->AddAtom( 4, EPP_TextHeaderAtom );
								*mpStrm << (sal_uInt32)EPP_TEXTTYPE_Body;
								mnTextSize = aTextObj.Count();
								aTextObj.Write( mpStrm );
								mpPptEscherEx->BeginAtom();
								for ( ParagraphObj* pPara = aTextObj.First() ; pPara; pPara = aTextObj.Next() )
								{
									sal_uInt32 nCharCount = pPara->Count();
									sal_uInt16 nDepth = pPara->nDepth;
									if ( nDepth > 4)
										nDepth = 4;

									*mpStrm << nCharCount
											<< nDepth;
								}
								mpPptEscherEx->EndAtom( EPP_BaseTextPropAtom );
								mpPptEscherEx->AddAtom( 10, EPP_TextSpecInfoAtom );
								*mpStrm << (sal_uInt32)( mnTextSize ) << (sal_uInt32)1 << (sal_uInt16)0;

								mpPptEscherEx->CloseContainer();    // ESCHER_ClientTextBox
								mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer
							}
                            continue;
                        }
						else
						{
							mnTextStyle = EPP_TEXTSTYLE_BODY;
							nPlaceHolderAtom = rLayout.nTypeOfOutliner;
							mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
							ImplCreateShape( ESCHER_ShpInst_Rectangle, 0x220, aSolverContainer );          // Flags: HaveAnchor | HaveMaster
							aPropOpt.AddOpt( ESCHER_Prop_hspMaster, mnShapeMasterBody );
							aPropOpt.CreateFillProperties( mXPropSet, sal_True );
							aPropOpt.CreateTextProperties( mXPropSet, mnTxId += 0x60, sal_False, sal_True );
							ImplAdjustFirstLineLineSpacing( aTextObj, aPropOpt );
							if ( mbEmptyPresObj )
							{
								sal_uInt32 nNoLineDrawDash = 0;
								aPropOpt.GetOpt( ESCHER_Prop_fNoLineDrawDash, nNoLineDrawDash );
								nNoLineDrawDash |= 0x10001;
								aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, nNoLineDrawDash );
							}
						}
                    }
                    else
                        mbPresObj = sal_False;
                }
                if ( !mbPresObj )
                {
                    mType = "drawing.Text";
	                ImplCreateTextShape( aPropOpt, aSolverContainer, sal_True );
                }
            }
            else if ( ( mType == "drawing.Page" ) || ( mType == "presentation.Page" ) )
            {
                if ( ( ePageType == NOTICE ) && mbPresObj )
                {
					if ( ImplCreatePresentationPlaceholder( bMasterPage, ePageType, EPP_TEXTTYPE_Notes, EPP_PLACEHOLDER_MASTERNOTESSLIDEIMAGE ) )
						continue;
					else
                        nPlaceHolderAtom = EPP_PLACEHOLDER_NOTESSLIDEIMAGE;
                }
                ImplCreateTextShape( aPropOpt, aSolverContainer, sal_True );
            }
            else if ( mType == "drawing.Frame" )
            {
                continue;
            }
            else if ( ( mType == "drawing.OLE2" ) || ( mType == "presentation.OLE2" )
                        || ( mType == "presentation.Chart" ) || ( mType == "presentation.Calc" )
                            || ( mType == "presentation.OrgChart" ) )
            {
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                if ( mbEmptyPresObj && ( ePageType == NORMAL ) )
                {
                    nPlaceHolderAtom = rLayout.nUsedObjectPlaceHolder;
                    ImplCreateShape( ESCHER_ShpInst_Rectangle, 0x220, aSolverContainer );              // Flags: HaveAnchor | HaveMaster
                    aPropOpt.AddOpt( ESCHER_Prop_lTxid, mnTxId += 0x60 );
                    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x10001 );
                    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x10001 );
                    aPropOpt.AddOpt( ESCHER_Prop_hspMaster, mnShapeMasterBody );
                }
                else
                {
					*mpExEmbed  << (sal_uInt32)( 0xf | ( EPP_ExEmbed << 16 ) )
								<< (sal_uInt32)0;               // Size of this container

					sal_uInt32 nSize, nOldPos = mpExEmbed->Tell();

					*mpExEmbed  << (sal_uInt32)( EPP_ExEmbedAtom << 16 )
								<< (sal_uInt32)8
								<< (sal_uInt32)0    // follow colorscheme : 0->do not follow
													//                      1->follow collorscheme
													//                      2->follow text and background scheme
								<< (sal_uInt8)1     // (bool)set if embedded server can not be locked
								<< (sal_uInt8)0     // (bool)do not need to send dimension
								<< (sal_uInt8)0     // (bool)is object a world table
								<< (sal_uInt8)0;    // pad byte

					PPTExOleObjEntry* pE = new PPTExOleObjEntry( NORMAL_OLE_OBJECT, mpExEmbed->Tell() );
					pE->xShape = mXShape;
					maExOleObj.Insert( pE );

					mnExEmbed++;

					sal_Int64 nAspect = ::com::sun::star::embed::Aspects::MSOLE_CONTENT;
					try
					{
						// try to get the aspect when available
						::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
							xShapeProps( mXShape, ::com::sun::star::uno::UNO_QUERY_THROW );
						xShapeProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Aspect" ) ) ) >>= nAspect;
					}
					catch( ::com::sun::star::uno::Exception& )
					{}

					*mpExEmbed  << (sal_uInt32)( 1 | ( EPP_ExOleObjAtom << 16 ) )
								<< (sal_uInt32)24
								<< (sal_uInt32)nAspect		// Aspect
								<< (sal_uInt32)0
								<< (sal_uInt32)mnExEmbed    // index to the persist table
								<< (sal_uInt32)0            // subtype
								<< (sal_uInt32)0
								<< (sal_uInt32)0x0012b600;

//					PPTWriter::WriteCString( *mpExEmbed, "Photo Editor Photo", 1 );
//                  PPTWriter::WriteCString( *mpExEmbed, "MSPhotoEd.3", 2 );
//                  PPTWriter::WriteCString( *mpExEmbed, "Microsoft Photo Editor 3.0 Photo", 3 );

					nSize = mpExEmbed->Tell() - nOldPos;
					mpExEmbed->Seek( nOldPos - 4 );
					*mpExEmbed << nSize;
					mpExEmbed->Seek( STREAM_SEEK_TO_END );
					nOlePictureId = mnExEmbed;

                    sal_uInt32 nSpFlags = 0xa00;
                    if ( nOlePictureId )
                        nSpFlags |= 0x10;
                    ImplCreateShape( ESCHER_ShpInst_PictureFrame, nSpFlags, aSolverContainer );
                    if ( aPropOpt.CreateOLEGraphicProperties( mXShape ) )
                        aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
                    if ( nOlePictureId )
                        aPropOpt.AddOpt( ESCHER_Prop_pictureId, nOlePictureId );
                }
            }
			else if ( mType == "presentation.Header" )
			{
				if ( ImplCreatePresentationPlaceholder( bMasterPage, ePageType, EPP_TEXTTYPE_Other, EPP_PLACEHOLDER_MASTERHEADER ) )
					continue;
				else
				{
					mbPresObj = sal_False;
                    mType = "drawing.Text";
	                ImplCreateTextShape( aPropOpt, aSolverContainer, sal_True );
                }
			}
			else if ( mType == "presentation.Footer" )
			{
				if ( ImplCreatePresentationPlaceholder( bMasterPage, ePageType, EPP_TEXTTYPE_Other, EPP_PLACEHOLDER_MASTERFOOTER ) )
					continue;
				else
				{
					mbPresObj = sal_False;
                    mType = "drawing.Text";
	                ImplCreateTextShape( aPropOpt, aSolverContainer, sal_True );
                }
			}
			else if ( mType == "presentation.DateTime" )
			{
				if ( ImplCreatePresentationPlaceholder( bMasterPage, ePageType, EPP_TEXTTYPE_Other, EPP_PLACEHOLDER_MASTERDATE ) )
					continue;
				else
				{
					mbPresObj = sal_False;
                    mType = "drawing.Text";
	                ImplCreateTextShape( aPropOpt, aSolverContainer, sal_True );
                }
			}
			else if ( mType == "presentation.SlideNumber" )
			{
				if ( ImplCreatePresentationPlaceholder( bMasterPage, ePageType, EPP_TEXTTYPE_Other, EPP_PLACEHOLDER_MASTERSLIDENUMBER ) )
					continue;
				else
				{
					mbPresObj = sal_False;
                    mType = "drawing.Text";
	                ImplCreateTextShape( aPropOpt, aSolverContainer, sal_True );
                }
			}
            else if ( ( (sal_Char)'3' == mType.GetChar( 8 ) ) && ( (char)'D' == mType.GetChar( 9 ) ) )  // drawing.3D
            {
                // SceneObject, CubeObject, SphereObject, LatheObject, ExtrudeObject, PolygonObject
                if ( !ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Bitmap" ) ) ) )
                    continue;

                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ImplCreateShape( ESCHER_ShpInst_PictureFrame, 0xa00, aSolverContainer );

                if ( aPropOpt.CreateGraphicProperties( mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Bitmap" ) ), sal_False ) )
                    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
            }
			else if ( mType == "drawing.Media" )
			{
				mnAngle = 0;
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ImplCreateShape( ESCHER_ShpInst_PictureFrame, 0xa00, aSolverContainer );

				::com::sun::star::uno::Any aAny;
				if ( PropValue::GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "MediaURL" ) ), sal_True ) )
				{
					rtl::OUString aMediaURL;
					if ( (aAny >>= aMediaURL ) &&  aMediaURL.getLength() )
					{
						// SJ: creating the Media RefObj
						sal_uInt32 nRefId = ++mnExEmbed;

						*mpExEmbed  << (sal_uInt16)0xf
									<< (sal_uInt16)EPP_ExMCIMovie		// PPT_PST_ExAviMovie
									<< (sal_uInt32)0;
						sal_uInt32 nSize, nStart = mpExEmbed->Tell();
						*mpExEmbed  << (sal_uInt16)0
									<< (sal_uInt16)EPP_ExObjRefAtom
									<< (sal_uInt32)4
									<< nRefId;
						*mpExEmbed  << (sal_uInt16)0xf
									<< (sal_uInt16)EPP_ExVideo
									<< (sal_uInt32)0;

						*mpExEmbed	<< (sal_uInt16)0
									<< (sal_uInt16)EPP_ExMediaAtom
									<< (sal_uInt32)8
									<< nRefId
									<< (sal_uInt16)0
									<< (sal_uInt16)0x435;


						sal_uInt16 i, nStringLen = (sal_uInt16)aMediaURL.getLength();
						*mpExEmbed << (sal_uInt32)( EPP_CString << 16 ) << (sal_uInt32)( nStringLen * 2 );
						for ( i = 0; i < nStringLen; i++ )
						{
							sal_Unicode nChar = aMediaURL[ i ];
							*mpExEmbed << nChar;
						}
						nSize = mpExEmbed->Tell() - nStart;
						mpExEmbed->SeekRel( - ( (sal_Int32)nSize + 4 ) );
						*mpExEmbed << nSize;	// size of PPT_PST_ExMCIMovie
						mpExEmbed->SeekRel( 0x10 );
						nSize -= 20;
						*mpExEmbed << nSize;	// PPT_PST_ExMediaAtom
						mpExEmbed->SeekRel( nSize );

	                    if ( !pClientData )
		                    pClientData = new SvMemoryStream( 0x200, 0x200 );
						*pClientData << (sal_uInt16)0
									 << (sal_uInt16)EPP_ExObjRefAtom
									 << (sal_uInt32)4
									 << nRefId;
					}
				}
			}
			else if ( (mType == "drawing.Table") || (mType == "presentation.Table") )
			{
				SvMemoryStream* pTmp = NULL;
				if ( bEffect && !mbUseNewAnimations )
				{
					pTmp = new SvMemoryStream( 0x200, 0x200 );
					ImplWriteObjectEffect( *pTmp, eAe, eTe, ++nEffectCount );
				}
				if ( eCa != ::com::sun::star::presentation::ClickAction_NONE )
				{
					if ( !pTmp )
						pTmp = new SvMemoryStream( 0x200, 0x200 );
					ImplWriteClickAction( *pTmp, eCa, bMediaClickAction );
				}
				ImplCreateTable( mXShape, aSolverContainer, aPropOpt );
				continue;
			}
            else if ( mType == "drawing.dontknow" )
            {
                mnAngle = 0;
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ImplCreateShape( ESCHER_ShpInst_PictureFrame, 0xa00, aSolverContainer );
                if ( aPropOpt.CreateGraphicProperties( mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "MetaFile" ) ), sal_False ) )
                    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
            }
            else
            {
                continue;
            }
            sal_Int32 nPlacementID = -1;

            sal_Bool bClientData = ( bEffect || ( eCa != ::com::sun::star::presentation::ClickAction_NONE ) ||
                                        nPlaceHolderAtom || nOlePictureId );
            if ( bClientData )
            {
                if ( nPlaceHolderAtom )
                {
                    if ( ( mnTextStyle == EPP_TEXTSTYLE_TITLE ) || ( mnTextStyle == EPP_TEXTSTYLE_BODY ) )
                        nPlacementID = nIndices++;
                    else
                    {
                        switch ( nPlaceHolderAtom )
                        {
                            default :
                            {
                                if ( nPlaceHolderAtom < 19 )
                                    break;
                            }
                            case EPP_PLACEHOLDER_NOTESBODY :
                            case EPP_PLACEHOLDER_MASTERDATE :
                            case EPP_PLACEHOLDER_NOTESSLIDEIMAGE :
                            case EPP_PLACEHOLDER_MASTERNOTESBODYIMAGE :
                                nPlacementID = nIndices++;
                        }
                    }
                    if ( !pClientData )
                        pClientData = new SvMemoryStream( 0x200, 0x200 );

                    *pClientData << (sal_uInt32)( EPP_OEPlaceholderAtom << 16 ) << (sal_uInt32)8
                                 << nPlacementID                // PlacementID
                                 << (sal_uInt8)nPlaceHolderAtom // PlaceHolderID
                                 << (sal_uInt8)0                // Size of PlaceHolder ( 0 = FULL, 1 = HALF, 2 = QUARTER )
                                 << (sal_uInt16)0;              // padword
                }
                if ( nOlePictureId )
                {
                    if ( !pClientData )
                        pClientData = new SvMemoryStream( 0x200, 0x200 );

                    *pClientData << (sal_uInt32)( EPP_ExObjRefAtom << 16 ) << (sal_uInt32)4
                                 << nOlePictureId;
                    nOlePictureId = 0;
                }
                if ( bEffect )
                {
                    if ( !pClientData )
                        pClientData = new SvMemoryStream( 0x200, 0x200 );

                    // check if it is sensible to replace the object effect with text effect,
                    // because in Impress there is the possibility to use a compound effect,
                    // e.g. the object effect is an AnimationEffect_FADE_FROM_LEFT and the
                    // text effect is a AnimationEffect_FADE_FROM_TOP, in PowerPoint there
                    // can be used only one effect
                    if ( mnTextSize && ( eTe != ::com::sun::star::presentation::AnimationEffect_NONE )
                        && ( eAe != ::com::sun::star::presentation::AnimationEffect_NONE )
                            && ( eTe != eAe ) )
                    {
                        sal_uInt32 nFillStyleFlags, nLineStyleFlags;
                        if ( aPropOpt.GetOpt( ESCHER_Prop_fNoFillHitTest, nFillStyleFlags )
                            && aPropOpt.GetOpt( ESCHER_Prop_fNoLineDrawDash, nLineStyleFlags ) )
                        {
                            // there is no fillstyle and also no linestyle
                            if ( ! ( ( nFillStyleFlags & 0x10 ) + ( nLineStyleFlags & 9 ) ) )
                                eAe = eTe;
                        }
                    }
					if ( !mbUseNewAnimations  )
						ImplWriteObjectEffect( *pClientData, eAe, eTe, ++nEffectCount );
                }

                if ( eCa != ::com::sun::star::presentation::ClickAction_NONE )
                {
                    if ( !pClientData )
                        pClientData = new SvMemoryStream( 0x200, 0x200 );
                    ImplWriteClickAction( *pClientData, eCa, bMediaClickAction );
                }
            }
            if ( ( mnTextStyle == EPP_TEXTSTYLE_TITLE ) || ( mnTextStyle == EPP_TEXTSTYLE_BODY ) )
            {
                if ( !pClientTextBox )
                    pClientTextBox = new SvMemoryStream( 0x200, 0x200 );

                if ( mbEmptyPresObj == sal_False )
                {
                    if ( ( ePageType == NORMAL ) && ( bMasterPage == sal_False ) )
                    {
						sal_uInt32 nTextType = EPP_TEXTTYPE_Body;
						if ( mnTextStyle == EPP_TEXTSTYLE_BODY )
						{				
							if ( bSecOutl )
								nTextType = EPP_TEXTTYPE_HalfBody;
							else if ( mType == "presentation.Subtitle" )
								nTextType = EPP_TEXTTYPE_CenterBody;
							bSecOutl = sal_True;
						}
						else
							nTextType = EPP_TEXTTYPE_Title;

                        TextRuleEntry aTextRule( nPageNumber );
						SvMemoryStream aExtBu( 0x200, 0x200 );
						ImplGetText();
                        ImplWriteTextStyleAtom( *pClientTextBox, nTextType, nPObjects, &aTextRule, aExtBu, NULL );
                        ImplWriteExtParaHeader( aExtBu, nPObjects++, nTextType, nPageNumber + 0x100 );
						SvMemoryStream* pOut = aTextRule.pOut;
                        if ( pOut )
                        {
                            pClientTextBox->Write( pOut->GetData(), pOut->Tell() );
                            delete pOut, aTextRule.pOut = NULL;
                        }
						if ( aExtBu.Tell() )
						{
							if ( !pClientData )
								pClientData = new SvMemoryStream( 0x200, 0x200 );
							ImplProgTagContainer( pClientData, &aExtBu );
						}
                    }
                }
            }
            else
            {
				if ( !aPropOpt.IsFontWork() )
				{
					if ( mnTextSize || ( nPlaceHolderAtom == EPP_PLACEHOLDER_MASTERDATE ) || ( nPlaceHolderAtom == EPP_PLACEHOLDER_NOTESBODY ) )
					{
						int nInstance2;
						if ( ( nPlaceHolderAtom == EPP_PLACEHOLDER_MASTERDATE ) || ( nPlaceHolderAtom == EPP_PLACEHOLDER_NOTESBODY ) )
							nInstance2 = 2;
						else
							nInstance2 = EPP_TEXTTYPE_Other;     // Text in a Shape

						if ( !pClientTextBox )
							pClientTextBox = new SvMemoryStream( 0x200, 0x200 );

						SvMemoryStream  aExtBu( 0x200, 0x200 );
						ImplWriteTextStyleAtom( *pClientTextBox, nInstance2, 0, NULL, aExtBu, &aPropOpt );
						if ( aExtBu.Tell() )
						{
							if ( !pClientData )
								pClientData = new SvMemoryStream( 0x200, 0x200 );
							ImplProgTagContainer( pClientData, &aExtBu );
						}
					}
					else if ( nPlaceHolderAtom >= 19 )
					{
						if ( !pClientTextBox )
							pClientTextBox = new SvMemoryStream( 12 );

						*pClientTextBox << (sal_uInt32)( EPP_TextHeaderAtom << 16 ) << (sal_uInt32)4
										<< (sal_uInt32)7;
					}
				}
            }

			aPropOpt.CreateShadowProperties( mXPropSet );
            maRect.Justify();
            if ( mnAngle )
                ImplFlipBoundingBox( aPropOpt );
            aPropOpt.CreateShapeProperties( mXShape );
			aPropOpt.Commit( *mpStrm );
			if ( GetCurrentGroupLevel() > 0 )		
		        mpPptEscherEx->AddChildAnchor( maRect );
			else
				mpPptEscherEx->AddClientAnchor( maRect );

			if ( pClientData )
            {
                *mpStrm << (sal_uInt32)( ( ESCHER_ClientData << 16 ) | 0xf )
                        << (sal_uInt32)pClientData->Tell();

                mpStrm->Write( pClientData->GetData(), pClientData->Tell() );
                delete pClientData, pClientData = NULL;
            }
            if ( pClientTextBox )
            {
                *mpStrm << (sal_uInt32)( ( ESCHER_ClientTextbox << 16 ) | 0xf )
                        << (sal_uInt32)pClientTextBox->Tell();

                mpStrm->Write( pClientTextBox->GetData(), pClientTextBox->Tell() );
                delete pClientTextBox, pClientTextBox = NULL;
            }
            mpPptEscherEx->CloseContainer();      // ESCHER_SpContainer
        }
        nPrevTextStyle = mnTextStyle;

        if ( bAdditionalText )
        {
            bAdditionalText = sal_False;

            ::com::sun::star::uno::Any  aAny;
            EscherPropertyContainer     aPropOpt;
            mnAngle = ( PropValue::GetPropertyValue( aAny,
                mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "RotateAngle" ) ), sal_True ) )
                    ? *((sal_Int32*)aAny.getValue() )
                    : 0;

            aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90000 );
            aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100000 );
            if ( mType == "drawing.Line" )
            {
                double fDist = hypot( maRect.GetWidth(), maRect.GetHeight() );
                maRect = Rectangle( Point( aTextRefPoint.X, aTextRefPoint.Y ),
                                        Point( (sal_Int32)( aTextRefPoint.X + fDist ), aTextRefPoint.Y - 1 ) );
                ImplCreateTextShape( aPropOpt, aSolverContainer, sal_False );
                aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, 0x60006 );        // Size Shape To Fit Text
                if ( mnAngle < 0 )
                    mnAngle = ( 36000 + mnAngle ) % 36000;
                if ( mnAngle )
                    ImplFlipBoundingBox( aPropOpt );
            }
            else
            {
                ImplCreateTextShape( aPropOpt, aSolverContainer, sal_False );
                if ( mnAngle < 0 )
                    mnAngle = ( 36000 + mnAngle ) % 36000;
                else
                    mnAngle = ( 36000 - ( mnAngle % 36000 ) );

                mnAngle *= 655;
                mnAngle += 0x8000;
                mnAngle &=~0xffff;  // nAngle auf volle Gradzahl runden
                aPropOpt.AddOpt( ESCHER_Prop_Rotation, mnAngle );
                mpPptEscherEx->SetGroupSnapRect( nGroupLevel, maRect );
                mpPptEscherEx->SetGroupLogicRect( nGroupLevel, maRect );
            }
            if ( !pClientTextBox )
                pClientTextBox = new SvMemoryStream( 0x200, 0x200 );

            SvMemoryStream  aExtBu( 0x200, 0x200 );
            ImplWriteTextStyleAtom( *pClientTextBox, EPP_TEXTTYPE_Other, 0, NULL, aExtBu, &aPropOpt );

            aPropOpt.CreateShapeProperties( mXShape );
            aPropOpt.Commit( *mpStrm );
			if ( GetCurrentGroupLevel() > 0 )		
		        mpPptEscherEx->AddChildAnchor( maRect );
			else
				mpPptEscherEx->AddClientAnchor( maRect );

            *mpStrm << (sal_uInt32)( ( ESCHER_ClientTextbox << 16 ) | 0xf )
                    << (sal_uInt32)pClientTextBox->Tell();

            mpStrm->Write( pClientTextBox->GetData(), pClientTextBox->Tell() );
            delete pClientTextBox, pClientTextBox = NULL;

            mpPptEscherEx->CloseContainer();  // ESCHER_SpContainer
            mpPptEscherEx->LeaveGroup();
        }
    }
    ClearGroupTable();                              // gruppierungen wegschreiben, sofern noch irgendwelche offen sind, was eigendlich nicht sein sollte
    nGroups = GetGroupsClosed();
    for ( sal_uInt32 i = 0; i < nGroups; i++, mpPptEscherEx->LeaveGroup() ) ;
    mnPagesWritten++;
}

//  -----------------------------------------------------------------------

::com::sun::star::awt::Point PPTWriter::ImplMapPoint( const ::com::sun::star::awt::Point& rPoint )
{
    Point aRet( OutputDevice::LogicToLogic( Point( rPoint.X, rPoint.Y ), maMapModeSrc, maMapModeDest ) );
    return ::com::sun::star::awt::Point( aRet.X(), aRet.Y() );
}

//  -----------------------------------------------------------------------

::com::sun::star::awt::Size PPTWriter::ImplMapSize( const ::com::sun::star::awt::Size& rSize )
{
    Size aRetSize( OutputDevice::LogicToLogic( Size( rSize.Width, rSize.Height ), maMapModeSrc, maMapModeDest ) );

    if ( !aRetSize.Width() )
        aRetSize.Width()++;
    if ( !aRetSize.Height() )
        aRetSize.Height()++;
    return ::com::sun::star::awt::Size( aRetSize.Width(), aRetSize.Height() );
}

//  -----------------------------------------------------------------------

Rectangle PPTWriter::ImplMapRectangle( const ::com::sun::star::awt::Rectangle& rRect )
{
    ::com::sun::star::awt::Point    aPoint( rRect.X, rRect.Y );
    ::com::sun::star::awt::Size     aSize( rRect.Width, rRect.Height );
    ::com::sun::star::awt::Point    aP( ImplMapPoint( aPoint ) );
    ::com::sun::star::awt::Size     aS( ImplMapSize( aSize ) );
    return Rectangle( Point( aP.X, aP.Y ), Size( aS.Width, aS.Height ) );
}

//  -----------------------------------------------------------------------

struct CellBorder
{
	sal_Int32						mnPos;		// specifies the distance to the top/left position of the table
	sal_Int32						mnLength;
	table::BorderLine				maCellBorder;

	CellBorder() : mnPos ( 0 ), mnLength( 0 ){};
};

void PPTWriter::ImplCreateCellBorder( const CellBorder* pCellBorder, sal_Int32 nX1, sal_Int32 nY1, sal_Int32 nX2, sal_Int32 nY2 )
{
	sal_Int32 nLineWidth = pCellBorder->maCellBorder.OuterLineWidth + pCellBorder->maCellBorder.InnerLineWidth;
	if ( nLineWidth )
	{
		mnAngle = 0;
		mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
		EscherPropertyContainer aPropOptSp;

        sal_uInt32 nId = mpPptEscherEx->GenerateShapeId();
		mpPptEscherEx->AddShape( ESCHER_ShpInst_Line, 0xa02, nId );
		aPropOptSp.AddOpt( ESCHER_Prop_shapePath, ESCHER_ShapeComplex );
		aPropOptSp.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0xa0008 );
		aPropOptSp.AddOpt( ESCHER_Prop_fshadowObscured, 0x20000 );

		sal_uInt32 nBorderColor = pCellBorder->maCellBorder.Color & 0xff00;					// green
		nBorderColor |= static_cast< sal_uInt8 >( pCellBorder->maCellBorder.Color ) << 16;	// red
		nBorderColor |= static_cast< sal_uInt8 >( pCellBorder->maCellBorder.Color >> 16 );	// blue
		aPropOptSp.AddOpt( ESCHER_Prop_lineColor, nBorderColor );

		aPropOptSp.AddOpt( ESCHER_Prop_lineWidth, nLineWidth * 360 );
		aPropOptSp.AddOpt( ESCHER_Prop_fc3DLightFace, 0x80000 );
		aPropOptSp.Commit( *mpStrm );
		mpPptEscherEx->AddAtom( 16, ESCHER_ChildAnchor );
		*mpStrm 	<< nX1
					<< nY1
					<< nX2
					<< nY2;
		mpPptEscherEx->CloseContainer();
	}
}

void PPTWriter::WriteCString( SvStream& rSt, const String& rString, sal_uInt32 nInstance )
{
    sal_uInt32 i, nLen = rString.Len();
    if ( nLen )
    {
        rSt << (sal_uInt32)( ( nInstance << 4 ) | ( EPP_CString << 16 ) )
            << (sal_uInt32)( nLen << 1 );
        for ( i = 0; i < nLen; i++ )
            rSt << rString.GetChar( (sal_uInt16)i );
    }
}

void PPTWriter::ImplCreateTable( uno::Reference< drawing::XShape >& rXShape, EscherSolverContainer& aSolverContainer,
								EscherPropertyContainer& aPropOpt )
{
	mpPptEscherEx->OpenContainer( ESCHER_SpgrContainer );
	mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
	mpPptEscherEx->AddAtom( 16, ESCHER_Spgr, 1 );
	*mpStrm		<< (sal_Int32)maRect.Left()	// Bounding box fuer die Gruppierten shapes an die sie attached werden
				<< (sal_Int32)maRect.Top()
				<< (sal_Int32)maRect.Right()
				<< (sal_Int32)maRect.Bottom();

    sal_uInt32 nShapeId = mpPptEscherEx->GenerateShapeId();
	mpPptEscherEx->AddShape( ESCHER_ShpInst_Min, 0x201, nShapeId );		// Flags: Group | Patriarch
	aSolverContainer.AddShape( rXShape, nShapeId );
	EscherPropertyContainer aPropOpt2;
	try
	{
		static const rtl::OUString	sModel( RTL_CONSTASCII_USTRINGPARAM ( "Model" ) );
		static const rtl::OUString sWidth( RTL_CONSTASCII_USTRINGPARAM ( "Width" ) );
		static const rtl::OUString sHeight( RTL_CONSTASCII_USTRINGPARAM ( "Height" ) );

		uno::Reference< table::XTable > xTable;
		if ( mXPropSet->getPropertyValue( sModel ) >>= xTable )
		{
			uno::Reference< table::XColumnRowRange > xColumnRowRange( xTable, uno::UNO_QUERY_THROW );
			uno::Reference< container::XIndexAccess > xColumns( xColumnRowRange->getColumns(), uno::UNO_QUERY_THROW );
			uno::Reference< container::XIndexAccess > xRows( xColumnRowRange->getRows(), uno::UNO_QUERY_THROW );
			sal_uInt16 nRowCount = static_cast< sal_uInt16 >( xRows->getCount() );
			sal_uInt16 nColumnCount = static_cast< sal_uInt16 >( xColumns->getCount() );

			std::vector< std::pair< sal_Int32, sal_Int32 > > aColumns;
			std::vector< std::pair< sal_Int32, sal_Int32 > > aRows;

			awt::Point aPosition( ImplMapPoint( rXShape->getPosition() ) );
			sal_uInt32 nPosition = aPosition.X;
			for ( sal_Int32 x = 0; x < nColumnCount; x++ )
			{
				uno::Reference< beans::XPropertySet > xPropSet( xColumns->getByIndex( x ), uno::UNO_QUERY_THROW );
				awt::Size aS( 0, 0 );
				xPropSet->getPropertyValue( sWidth ) >>= aS.Width;
				awt::Size aM( ImplMapSize( aS ) );
				aColumns.push_back( std::pair< sal_Int32, sal_Int32 >( nPosition, aM.Width ) );
				nPosition += aM.Width;
			}

			nPosition = aPosition.Y;
			for ( sal_Int32 y = 0; y < nRowCount; y++ )
			{
				uno::Reference< beans::XPropertySet > xPropSet( xRows->getByIndex( y ), uno::UNO_QUERY_THROW );
				awt::Size aS( 0, 0 );
				xPropSet->getPropertyValue( sHeight ) >>= aS.Height;
				awt::Size aM( ImplMapSize( aS ) );
				aRows.push_back( std::pair< sal_Int32, sal_Int32 >( nPosition, aM.Height ) );
				nPosition += aM.Height;
			}

			if ( nRowCount )
			{
				SvMemoryStream aMemStrm;
				aMemStrm.ObjectOwnsMemory( sal_False );
				aMemStrm << nRowCount
						 << nRowCount
						 << (sal_uInt16)4;

				std::vector< std::pair< sal_Int32, sal_Int32 > >::const_iterator aIter( aRows.begin() );
				while( aIter != aRows.end() )
					aMemStrm << (*aIter++).second;

				aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x1000100 );
				aPropOpt2.AddOpt( ESCHER_Prop_tableProperties, 1 );
				aPropOpt2.AddOpt( ESCHER_Prop_tableRowProperties, sal_True, aMemStrm.Tell(), static_cast< sal_uInt8* >( const_cast< void* >( aMemStrm.GetData() ) ), aMemStrm.Tell() );
				aPropOpt.CreateShapeProperties( rXShape );
				aPropOpt.Commit( *mpStrm );
				aPropOpt2.Commit( *mpStrm, 3, ESCHER_UDefProp );
				if ( GetCurrentGroupLevel() > 0 )		
					mpPptEscherEx->AddChildAnchor( maRect );
				else
					mpPptEscherEx->AddClientAnchor( maRect );
				mpPptEscherEx->CloseContainer();


				uno::Reference< table::XCellRange > xCellRange( xTable, uno::UNO_QUERY_THROW );
				for( sal_Int32 nRow = 0; nRow < xRows->getCount(); nRow++ )
				{
					for( sal_Int32 nColumn = 0; nColumn < xColumns->getCount(); nColumn++ )
					{
						uno::Reference< table::XMergeableCell > xCell( xCellRange->getCellByPosition( nColumn, nRow ), uno::UNO_QUERY_THROW );
						if ( !xCell->isMerged() )
						{
							sal_Int32 nLeft   = aColumns[ nColumn ].first;
							sal_Int32 nTop    = aRows[ nRow ].first;
							sal_Int32 nRight  = nLeft + aColumns[ nColumn ].second;
							sal_Int32 nBottom = nTop + aRows[ nRow ].second;

							for ( sal_Int32 nColumnSpan = 1; nColumnSpan < xCell->getColumnSpan(); nColumnSpan++ )
							{
								sal_uInt32 nC = nColumnSpan + nColumn;
								if ( nC < aColumns.size() )
									nRight += aColumns[ nC ].second;
								else
									nRight = maRect.Right();
							}
							for ( sal_Int32 nRowSpan = 1; nRowSpan < xCell->getRowSpan(); nRowSpan++ )
							{
								sal_uInt32 nR = nRowSpan + nRow;
								if ( nR < aColumns.size() )
									nBottom += aRows[ nR ].second;
								else
									nBottom = maRect.Bottom();
							}

							mbFontIndependentLineSpacing = sal_False;
							mXPropSet = uno::Reference< beans::XPropertySet >( xCell, uno::UNO_QUERY_THROW );
							mXText = uno::Reference< text::XSimpleText >( xCell, uno::UNO_QUERY_THROW );
							mnTextSize = mXText->getString().getLength();

							::com::sun::star::uno::Any aAny;
							if ( GetPropertyValue( aAny, mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FontIndependentLineSpacing" ) ) ), sal_True )
								aAny >>= mbFontIndependentLineSpacing;

							EscherPropertyContainer aPropOptSp;
							mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
							ImplCreateShape( ESCHER_ShpInst_Rectangle, 0xa02, aSolverContainer );          // Flags: Connector | HasSpt | Child
							aPropOptSp.CreateFillProperties( mXPropSet, sal_True );
							aPropOptSp.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90000 );
                            aPropOptSp.CreateTextProperties( mXPropSet, mnTxId += 0x60, sal_False, sal_True );
							aPropOptSp.AddOpt( ESCHER_Prop_WrapText, ESCHER_WrapSquare );

                            SvMemoryStream aClientTextBox( 0x200, 0x200 );
                            SvMemoryStream  aExtBu( 0x200, 0x200 );

                            ImplWriteTextStyleAtom( aClientTextBox, EPP_TEXTTYPE_Other, 0, NULL, aExtBu, &aPropOptSp );

                            aPropOptSp.Commit( *mpStrm );
                            mpPptEscherEx->AddAtom( 16, ESCHER_ChildAnchor );
                            *mpStrm 	<< nLeft
                                        << nTop
                                        << nRight
                                        << nBottom;

                            *mpStrm << (sal_uInt32)( ( ESCHER_ClientTextbox << 16 ) | 0xf )
                                    << (sal_uInt32)aClientTextBox.Tell();

                            mpStrm->Write( aClientTextBox.GetData(), aClientTextBox.Tell() );
							mpPptEscherEx->CloseContainer();
						}
					}
				}

				static const rtl::OUString sTopBorder( String( RTL_CONSTASCII_USTRINGPARAM( "TopBorder" ) ) );
				static const rtl::OUString sBottomBorder( String( RTL_CONSTASCII_USTRINGPARAM( "BottomBorder" ) ) );
				static const rtl::OUString sLeftBorder( String( RTL_CONSTASCII_USTRINGPARAM( "LeftBorder" ) ) );
				static const rtl::OUString sRightBorder( String( RTL_CONSTASCII_USTRINGPARAM( "RightBorder" ) ) );	
				static const rtl::OUString	sDiagonalTLBR( RTL_CONSTASCII_USTRINGPARAM ( "DiagonalTLBR" ) );
				static const rtl::OUString	sDiagonalBLTR( RTL_CONSTASCII_USTRINGPARAM ( "DiagonalBLTR" ) );

				// creating horz lines
				sal_Int32 nYPos = ImplMapPoint( rXShape->getPosition() ).Y;
				for( sal_Int32 nLine = 0; nLine < ( xRows->getCount() + 1 ); nLine++ )
				{
					sal_Int32 nXPos = ImplMapPoint( rXShape->getPosition() ).X;
					std::vector< CellBorder > vCellBorders;
					for( sal_Int32 nColumn = 0; nColumn < xColumns->getCount(); nColumn++ )
					{
						uno::Reference< beans::XPropertySet > xPropSet( xColumns->getByIndex( nColumn ), uno::UNO_QUERY_THROW );
						awt::Size aS( 0, 0 );
						xPropSet->getPropertyValue( sWidth ) >>= aS.Width;
						awt::Size aM( ImplMapSize( aS ) );

						CellBorder aCellBorder;
						aCellBorder.mnPos = nXPos;
						aCellBorder.mnLength = aM.Width;
						if ( nLine < xRows->getCount() )
						{	// top border
							uno::Reference< table::XMergeableCell > xCell( xCellRange->getCellByPosition( nColumn, nLine ), uno::UNO_QUERY_THROW );
							uno::Reference< beans::XPropertySet > xPropSet2( xCell, uno::UNO_QUERY_THROW );
							table::BorderLine aBorderLine;
							if ( xPropSet2->getPropertyValue( sTopBorder ) >>= aBorderLine )
								aCellBorder.maCellBorder = aBorderLine;
						}
						if ( nLine )
						{	// bottom border
							uno::Reference< table::XMergeableCell > xCell( xCellRange->getCellByPosition( nColumn, nLine - 1 ), uno::UNO_QUERY_THROW );
							uno::Reference< beans::XPropertySet > xPropSet2( xCell, uno::UNO_QUERY_THROW );
							table::BorderLine aBorderLine;
							if ( xPropSet2->getPropertyValue( sBottomBorder ) >>= aBorderLine )
								aCellBorder.maCellBorder = aBorderLine;
						}
						vCellBorders.push_back( aCellBorder );
						nXPos += aM.Width;
					}
					std::vector< CellBorder >::const_iterator aCellBorderIter( vCellBorders.begin() );
					while( aCellBorderIter != vCellBorders.end() )
					{
						ImplCreateCellBorder( &*aCellBorderIter, aCellBorderIter->mnPos, nYPos,
							static_cast< sal_Int32 >( aCellBorderIter->mnPos + aCellBorderIter->mnLength ), nYPos );
						aCellBorderIter++;
					}
					if ( nLine < xRows->getCount() )
					{
						uno::Reference< beans::XPropertySet > xPropSet( xRows->getByIndex( nLine ), uno::UNO_QUERY_THROW );
						awt::Size aS( 0, 0 );
						xPropSet->getPropertyValue( sHeight ) >>= aS.Height;
						awt::Size aM( ImplMapSize( aS ) );
						nYPos += aM.Height;
					}
				}

				// creating vertical lines
				sal_Int32 nXPos = ImplMapPoint( rXShape->getPosition() ).X;
				for( sal_Int32 nLine = 0; nLine < ( xColumns->getCount() + 1 ); nLine++ )
				{
					nYPos = ImplMapPoint( rXShape->getPosition() ).Y;
					std::vector< CellBorder > vCellBorders;
					for( sal_Int32 nRow = 0; nRow < xRows->getCount(); nRow++ )
					{
						uno::Reference< beans::XPropertySet > xPropSet( xRows->getByIndex( nRow ), uno::UNO_QUERY_THROW );
						awt::Size aS( 0, 0 );
						xPropSet->getPropertyValue( sHeight ) >>= aS.Height;
						awt::Size aM( ImplMapSize( aS ) );

						CellBorder aCellBorder;
						aCellBorder.mnPos = nYPos;
						aCellBorder.mnLength = aM.Height;
						if ( nLine < xColumns->getCount() )
						{	// left border
							uno::Reference< table::XMergeableCell > xCell( xCellRange->getCellByPosition( nLine, nRow ), uno::UNO_QUERY_THROW );
							uno::Reference< beans::XPropertySet > xCellSet( xCell, uno::UNO_QUERY_THROW );
							table::BorderLine aBorderLine;
							if ( xCellSet->getPropertyValue( sLeftBorder ) >>= aBorderLine )
								aCellBorder.maCellBorder = aBorderLine;
						}
						if ( nLine )
						{	// right border
							uno::Reference< table::XMergeableCell > xCell( xCellRange->getCellByPosition( nLine - 1, nRow ), uno::UNO_QUERY_THROW );
							uno::Reference< beans::XPropertySet > xCellSet( xCell, uno::UNO_QUERY_THROW );
							table::BorderLine aBorderLine;
							if ( xCellSet->getPropertyValue( sRightBorder ) >>= aBorderLine )
								aCellBorder.maCellBorder = aBorderLine;
						}
						vCellBorders.push_back( aCellBorder );
						nYPos += aM.Height;
					}
					std::vector< CellBorder >::const_iterator aCellBorderIter( vCellBorders.begin() );
					while( aCellBorderIter != vCellBorders.end() )
					{
						ImplCreateCellBorder( &*aCellBorderIter, nXPos, aCellBorderIter->mnPos,
							nXPos, static_cast< sal_Int32 >( aCellBorderIter->mnPos + aCellBorderIter->mnLength ) );
						aCellBorderIter++;
					}
					if ( nLine < xColumns->getCount() )
					{
						uno::Reference< beans::XPropertySet > xPropSet( xColumns->getByIndex( nLine ), uno::UNO_QUERY_THROW );
						awt::Size aS( 0, 0 );
						xPropSet->getPropertyValue( sWidth ) >>= aS.Width;
						awt::Size aM( ImplMapSize( aS ) );
						nXPos += aM.Width;
					}
				}
			}
		}
	}
	catch( uno::Exception& )
	{
	}
	mpPptEscherEx->CloseContainer();
}
