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



#include "oox/core/xmlfilterbase.hxx"
#include "oox/export/shapes.hxx"
#include "oox/export/utils.hxx"

#include <cstdio>
#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <vcl/cvtgrf.hxx>
#include <unotools/fontcvt.hxx>
#include <vcl/graph.hxx>
#include <vcl/outdev.hxx>
#include <svtools/grfmgr.hxx>
#include <rtl/strbuf.hxx>
#include <sfx2/app.hxx>
#include <svl/languageoptions.hxx>
#include <svx/escherex.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svxenum.hxx>
#include <svx/unoapi.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::i18n;
using ::com::sun::star::beans::PropertyState;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertyState;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::drawing::FillStyle;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::text::XSimpleText;
using ::com::sun::star::text::XText;
using ::com::sun::star::text::XTextContent;
using ::com::sun::star::text::XTextField;
using ::com::sun::star::text::XTextRange;
using ::rtl::OString;
using ::rtl::OStringBuffer;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::sax_fastparser::FSHelperPtr;

DBG(extern void dump_pset(Reference< XPropertySet > rXPropSet));

#define IDS(x) (OString(#x " ") + OString::valueOf( mnShapeIdMax++ )).getStr()

struct CustomShapeTypeTranslationTable
{
    const char* sOOo;
    const char* sMSO;
};

static const CustomShapeTypeTranslationTable pCustomShapeTypeTranslationTable[] = 
{
    // { "non-primitive", mso_sptMin },
    { "rectangle", "rect" },
    { "round-rectangle", "roundRect" },
    { "ellipse", "ellipse" },
    { "diamond", "diamond" },
    { "isosceles-triangle", "triangle" },
    { "right-triangle", "rtTriangle" },
    { "parallelogram", "parallelogram" },
    { "trapezoid", "trapezoid" },
    { "hexagon", "hexagon" },
    { "octagon", "octagon" },
    { "cross", "plus" },
    { "star5", "star5" },
    { "right-arrow", "rightArrow" },
    // { "mso-spt14", mso_sptThickArrow },
    { "pentagon-right", "homePlate" },
    { "cube", "cube" },
    // { "mso-spt17", mso_sptBalloon },
    // { "mso-spt18", mso_sptSeal },
    { "mso-spt19", "arc" },
    { "mso-spt20", "line" },
    { "mso-spt21", "plaque" },
    { "can", "can" },
    { "ring", "donut" },
    { "mso-spt24", "textSimple" },
    { "mso-spt25", "textOctagon" },
    { "mso-spt26", "textHexagon" },
    { "mso-spt27", "textCurve" },
    { "mso-spt28", "textWave" },
    { "mso-spt29", "textRing" },
    { "mso-spt30", "textOnCurve" },
    { "mso-spt31", "textOnRing" },
    { "mso-spt32", "straightConnector1" },
    { "mso-spt33", "bentConnector2" },
    { "mso-spt34", "bentConnector3" },
    { "mso-spt35", "bentConnector4" },
    { "mso-spt36", "bentConnector5" },
    { "mso-spt37", "curvedConnector2" },
    { "mso-spt38", "curvedConnector3" },
    { "mso-spt39", "curvedConnector4" },
    { "mso-spt40", "curvedConnector5" },
    { "mso-spt41", "callout1" },
    { "mso-spt42", "callout2" },
    { "mso-spt43", "callout3" },
    { "mso-spt44", "accentCallout1" },
    { "mso-spt45", "accentCallout2" },
    { "mso-spt46", "accentCallout3" },
    { "line-callout-1", "borderCallout1" },
    { "line-callout-2", "borderCallout2" },
    { "line-callout-3", "borderCallout3" },
    { "mso-spt49", "accentBorderCallout90" },
    { "mso-spt50", "accentBorderCallout1" },
    { "mso-spt51", "accentBorderCallout2" },
    { "mso-spt52", "accentBorderCallout3" },
    { "mso-spt53", "ribbon" },
    { "mso-spt54", "ribbon2" },
    { "chevron", "chevron" },
    { "pentagon", "pentagon" },
    { "forbidden", "noSmoking" },
    { "star8", "seal8" },
    { "mso-spt59", "seal16" },
    { "mso-spt60", "seal32" },
    { "rectangular-callout", "wedgeRectCallout" },
    { "round-rectangular-callout", "wedgeRoundRectCallout" },
    { "round-callout", "wedgeEllipseCallout" },
    { "mso-spt64", "wave" },
    { "paper", "foldedCorner" },
    { "left-arrow", "leftArrow" },
    { "down-arrow", "downArrow" },
    { "up-arrow", "upArrow" },
    { "left-right-arrow", "leftRightArrow" },
    { "up-down-arrow", "upDownArrow" },
    { "mso-spt71", "irregularSeal1" },
    { "bang", "irregularSeal2" },
    { "lightning", "lightningBolt" },
    { "heart", "heart" },
    { "mso-spt75", "pictureFrame" },
    { "quad-arrow", "quadArrow" },
    { "left-arrow-callout", "leftArrowCallout" },
    { "right-arrow-callout", "rightArrowCallout" },
    { "up-arrow-callout", "upArrowCallout" },
    { "down-arrow-callout", "downArrowCallout" },
    { "left-right-arrow-callout", "leftRightArrowCallout" },
    { "up-down-arrow-callout", "upDownArrowCallout" },
    { "quad-arrow-callout", "quadArrowCallout" },
    { "quad-bevel", "bevel" },
    { "left-bracket", "leftBracket" },
    { "right-bracket", "rightBracket" },
    { "left-brace", "leftBrace" },
    { "right-brace", "rightBrace" },
    { "mso-spt89", "leftUpArrow" },
    { "mso-spt90", "bentUpArrow" },
    { "mso-spt91", "bentArrow" },
    { "star24", "seal24" },
    { "striped-right-arrow", "stripedRightArrow" },
    { "notched-right-arrow", "notchedRightArrow" },
    { "block-arc", "blockArc" },
    { "smiley", "smileyFace" },
    { "vertical-scroll", "verticalScroll" },
    { "horizontal-scroll", "horizontalScroll" },
    { "circular-arrow", "circularArrow" },
    { "mso-spt100", "pie" }, // looks like MSO_SPT is wrong here
    { "mso-spt101", "uturnArrow" },
    { "mso-spt102", "curvedRightArrow" },
    { "mso-spt103", "curvedLeftArrow" },
    { "mso-spt104", "curvedUpArrow" },
    { "mso-spt105", "curvedDownArrow" },
    { "cloud-callout", "cloudCallout" },
    { "mso-spt107", "ellipseRibbon" },
    { "mso-spt108", "ellipseRibbon2" },
    { "flowchart-process", "flowChartProcess" },
    { "flowchart-decision", "flowChartDecision" },
    { "flowchart-data", "flowChartInputOutput" },
    { "flowchart-predefined-process", "flowChartPredefinedProcess" },
    { "flowchart-internal-storage", "flowChartInternalStorage" },
    { "flowchart-document", "flowChartDocument" },
    { "flowchart-multidocument", "flowChartMultidocument" },
    { "flowchart-terminator", "flowChartTerminator" },
    { "flowchart-preparation", "flowChartPreparation" },
    { "flowchart-manual-input", "flowChartManualInput" },
    { "flowchart-manual-operation", "flowChartManualOperation" },
    { "flowchart-connector", "flowChartConnector" },
    { "flowchart-card", "flowChartPunchedCard" },
    { "flowchart-punched-tape", "flowChartPunchedTape" },
    { "flowchart-summing-junction", "flowChartSummingJunction" },
    { "flowchart-or", "flowChartOr" },
    { "flowchart-collate", "flowChartCollate" },
    { "flowchart-sort", "flowChartSort" },
    { "flowchart-extract", "flowChartExtract" },
    { "flowchart-merge", "flowChartMerge" },
    { "mso-spt129", "flowChartOfflineStorage" },
    { "flowchart-stored-data", "flowChartOnlineStorage" },
    { "flowchart-sequential-access", "flowChartMagneticTape" },
    { "flowchart-magnetic-disk", "flowChartMagneticDisk" },
    { "flowchart-direct-access-storage", "flowChartMagneticDrum" },
    { "flowchart-display", "flowChartDisplay" },
    { "flowchart-delay", "flowChartDelay" },
    { "fontwork-plain-text", "textPlainText" },
    { "fontwork-stop", "textStop" },
    { "fontwork-triangle-up", "textTriangle" },
    { "fontwork-triangle-down", "textTriangleInverted" },
    { "fontwork-chevron-up", "textChevron" },
    { "fontwork-chevron-down", "textChevronInverted" },
    { "mso-spt142", "textRingInside" },
    { "mso-spt143", "textRingOutside" },
    { "fontwork-arch-up-curve", "textArchUpCurve" },
    { "fontwork-arch-down-curve", "textArchDownCurve" },
    { "fontwork-circle-curve", "textCircleCurve" },
    { "fontwork-open-circle-curve", "textButtonCurve" },
    { "fontwork-arch-up-pour", "textArchUpPour" },
    { "fontwork-arch-down-pour", "textArchDownPour" },
    { "fontwork-circle-pour", "textCirclePour" },
    { "fontwork-open-circle-pour", "textButtonPour" },
    { "fontwork-curve-up", "textCurveUp" },
    { "fontwork-curve-down", "textCurveDown" },
    { "fontwork-fade-up-and-right", "textCascadeUp" },
    { "fontwork-fade-up-and-left", "textCascadeDown" },
    { "fontwork-wave", "textWave1" },
    { "mso-spt157", "textWave2" },
    { "mso-spt158", "textWave3" },
    { "mso-spt159", "textWave4" },
    { "fontwork-inflate", "textInflate" },
    { "mso-spt161", "textDeflate" },
    { "mso-spt162", "textInflateBottom" },
    { "mso-spt163", "textDeflateBottom" },
    { "mso-spt164", "textInflateTop" },
    { "mso-spt165", "textDeflateTop" },
    { "mso-spt166", "textDeflateInflate" },
    { "mso-spt167", "textDeflateInflateDeflate" },
    { "fontwork-fade-right", "textFadeRight" },
    { "fontwork-fade-left", "textFadeLeft" },
    { "fontwork-fade-up", "textFadeUp" },
    { "fontwork-fade-down", "textFadeDown" },
    { "fontwork-slant-up", "textSlantUp" },
    { "fontwork-slant-down", "textSlantDown" },
    { "mso-spt174", "textCanUp" },
    { "mso-spt175", "textCanDown" },
    { "flowchart-alternate-process", "flowChartAlternateProcess" },
    { "flowchart-off-page-connector", "flowChartOffpageConnector" },
    { "mso-spt178", "callout90" },
    { "mso-spt179", "accentCallout90" },
    { "mso-spt180", "borderCallout90" },
    { "mso-spt182", "leftRightUpArrow" },
    { "sun", "sun" },
    { "moon", "moon" },
    { "bracket-pair", "bracketPair" },
    { "brace-pair", "bracePair" },
    { "star4", "seal4" },
    { "mso-spt188", "doubleWave" },
    { "mso-spt189", "actionButtonBlank" },
    { "mso-spt190", "actionButtonHome" },
    { "mso-spt191", "actionButtonHelp" },
    { "mso-spt192", "actionButtonInformation" },
    { "mso-spt193", "actionButtonForwardNext" },
    { "mso-spt194", "actionButtonBackPrevious" },
    { "mso-spt195", "actionButtonEnd" },
    { "mso-spt196", "actionButtonBeginning" },
    { "mso-spt197", "actionButtonReturn" },
    { "mso-spt198", "actionButtonDocument" },
    { "mso-spt199", "actionButtonSound" },
    { "mso-spt200", "actionButtonMovie" },
    { "mso-spt201", "hostControl" },
    { "mso-spt202", "rect" }
};

struct StringCheck
{
    bool operator()( const char* s1, const char* s2 ) const
    {
        return strcmp( s1, s2 ) == 0;
    }
};

typedef std::hash_map< const char*, const char*, std::hash<const char*>, StringCheck> CustomShapeTypeTranslationHashMap;
static CustomShapeTypeTranslationHashMap* pCustomShapeTypeTranslationHashMap = NULL;

static const char* lcl_GetPresetGeometry( const char* sShapeType )
{
    const char* sPresetGeo;

    if( pCustomShapeTypeTranslationHashMap == NULL )
    {
        pCustomShapeTypeTranslationHashMap = new CustomShapeTypeTranslationHashMap ();
        for( unsigned int i = 0; i < sizeof( pCustomShapeTypeTranslationTable )/sizeof( CustomShapeTypeTranslationTable ); i ++ )
        {
            (*pCustomShapeTypeTranslationHashMap)[ pCustomShapeTypeTranslationTable[ i ].sOOo ] = pCustomShapeTypeTranslationTable[ i ].sMSO;
            //DBG(printf("type OOo: %s MSO: %s\n", pCustomShapeTypeTranslationTable[ i ].sOOo, pCustomShapeTypeTranslationTable[ i ].sMSO));
        }
    }

    sPresetGeo = (*pCustomShapeTypeTranslationHashMap)[ sShapeType ];

    if( sPresetGeo == NULL )
        sPresetGeo = "rect";

    return sPresetGeo;
}

namespace oox { namespace drawingml {

#define GETA(propName) \
    GetProperty( rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( #propName ) ) )

#define GETAD(propName) \
    ( GetPropertyAndState( rXPropSet, rXPropState, String( RTL_CONSTASCII_USTRINGPARAM( #propName ) ), eState ) && eState == beans::PropertyState_DIRECT_VALUE )

#define GET(variable, propName) \
    if ( GETA(propName) ) \
        mAny >>= variable;

ShapeExport::ShapeExport( sal_Int32 nXmlNamespace, FSHelperPtr pFS, ::oox::core::XmlFilterBase* pFB, DocumentType eDocumentType )
    : DrawingML( pFS, pFB, eDocumentType )
    , mnXmlNamespace( nXmlNamespace )
    , mnShapeIdMax( 1 )
    , mnPictureIdMax( 1 )
    , maFraction( 1, 576 )
    , maMapModeSrc( MAP_100TH_MM )
    , maMapModeDest( MAP_INCH, Point(), maFraction, maFraction )
{
}

sal_Int32 ShapeExport::GetXmlNamespace() const
{
    return mnXmlNamespace;
}

ShapeExport& ShapeExport::SetXmlNamespace( sal_Int32 nXmlNamespace )
{
    mnXmlNamespace = nXmlNamespace;
    return *this;
}

awt::Size ShapeExport::MapSize( const awt::Size& rSize ) const
{
    Size aRetSize( OutputDevice::LogicToLogic( Size( rSize.Width, rSize.Height ), maMapModeSrc, maMapModeDest ) );

    if ( !aRetSize.Width() )
        aRetSize.Width()++;
    if ( !aRetSize.Height() )
        aRetSize.Height()++;
    return awt::Size( aRetSize.Width(), aRetSize.Height() );
}

sal_Bool ShapeExport::NonEmptyText( Reference< XShape > xShape )
{
    Reference< XSimpleText > xText( xShape, UNO_QUERY );

    return ( xText.is() && xText->getString().getLength() );
}

ShapeExport& ShapeExport::WriteBezierShape( Reference< XShape > xShape, sal_Bool bClosed )
{
    DBG(printf("write open bezier shape\n"));

    FSHelperPtr pFS = GetFS();
    pFS->startElementNS( mnXmlNamespace, XML_sp, FSEND );

    PolyPolygon aPolyPolygon = EscherPropertyContainer::GetPolyPolygon( xShape );
    Rectangle aRect( aPolyPolygon.GetBoundRect() );
    awt::Size size = MapSize( awt::Size( aRect.GetWidth(), aRect.GetHeight() ) );

    DBG(printf("poly count %d\nsize: %d x %d", aPolyPolygon.Count(), int( size.Width ), int( size.Height )));

    // non visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_nvSpPr, FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id, I32S( GetNewShapeID( xShape ) ),
                          XML_name, IDS( Freeform ),
                          FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr, FSEND );
    WriteNonVisualProperties( xShape );
    pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteTransformation( aRect );
    WritePolyPolygon( aPolyPolygon );
    Reference< XPropertySet > xProps( xShape, UNO_QUERY );
    if( xProps.is() ) {
        if( bClosed )
            WriteFill( xProps );
        WriteOutline( xProps );
    }

    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape );

    pFS->endElementNS( mnXmlNamespace, XML_sp );

    return *this;
}

ShapeExport& ShapeExport::WriteClosedBezierShape( Reference< XShape > xShape )
{
    return WriteBezierShape( xShape, TRUE );
}

ShapeExport& ShapeExport::WriteOpenBezierShape( Reference< XShape > xShape )
{
    return WriteBezierShape( xShape, FALSE );
}

ShapeExport& ShapeExport::WriteCustomShape( Reference< XShape > xShape )
{
    DBG(printf("write custom shape\n"));

    Reference< XPropertySet > rXPropSet( xShape, UNO_QUERY );
    SdrObjCustomShape* pShape = (SdrObjCustomShape*) GetSdrObjectFromXShape( xShape );
    sal_Bool bIsDefaultObject = EscherPropertyContainer::IsDefaultObject( pShape );
    sal_Bool bPredefinedHandlesUsed = TRUE;
    OUString sShapeType;
    sal_uInt32 nMirrorFlags = 0;
    MSO_SPT eShapeType = EscherPropertyContainer::GetCustomShapeType( xShape, nMirrorFlags, sShapeType );
    const char* sPresetShape = lcl_GetPresetGeometry( USS( sShapeType ) );
    DBG(printf("custom shape type: %s ==> %s\n", USS( sShapeType ), sPresetShape));
    Sequence< PropertyValue > aGeometrySeq;
    sal_Int32 nAdjustmentValuesIndex = -1;
    sal_Int32 nAdjustmentsWhichNeedsToBeConverted = 0;

    if( GETA( CustomShapeGeometry ) ) {
        DBG(printf("got custom shape geometry\n"));
        if( mAny >>= aGeometrySeq ) {

            DBG(printf("got custom shape geometry sequence\n"));
            for( int i = 0; i < aGeometrySeq.getLength(); i++ ) {
                const PropertyValue& rProp = aGeometrySeq[ i ];
                DBG(printf("geometry property: %s\n", USS( rProp.Name )));

                if( rProp.Name.equalsAscii( "AdjustmentValues" ))
                    nAdjustmentValuesIndex = i;
                else if( rProp.Name.equalsAscii( "Handles" )) {
                    if( !bIsDefaultObject )
                        bPredefinedHandlesUsed = FALSE;
                    // TODO: update nAdjustmentsWhichNeedsToBeConverted here
                }
            }
        }
    }

    FSHelperPtr pFS = GetFS();
    pFS->startElementNS( mnXmlNamespace, XML_sp, FSEND );

    // non visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_nvSpPr, FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id, I32S( GetNewShapeID( xShape ) ),
                          XML_name, IDS( CustomShape ),
                          FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr, FSEND );
    WriteNonVisualProperties( xShape );
    pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteShapeTransformation( xShape );
    if( nAdjustmentValuesIndex != -1 )
        WritePresetShape( sPresetShape, eShapeType, bPredefinedHandlesUsed, nAdjustmentsWhichNeedsToBeConverted, aGeometrySeq[ nAdjustmentValuesIndex ] );
    else
        WritePresetShape( sPresetShape );
    if( rXPropSet.is() )
    {
        WriteFill( rXPropSet );
        WriteOutline( rXPropSet );
    }

    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape );

    pFS->endElementNS( mnXmlNamespace, XML_sp );

    return *this;
}

ShapeExport& ShapeExport::WriteEllipseShape( Reference< XShape > xShape )
{
    DBG(printf("write ellipse shape\n"));

    FSHelperPtr pFS = GetFS();

    pFS->startElementNS( mnXmlNamespace, XML_sp, FSEND );

    // TODO: arc, section, cut, connector

    // non visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_nvSpPr, FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id, I32S( GetNewShapeID( xShape ) ),
                          XML_name, IDS( Ellipse ),
                          FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr, FSEND );
    WriteNonVisualProperties( xShape );
    pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteShapeTransformation( xShape );
    WritePresetShape( "ellipse" );
    Reference< XPropertySet > xProps( xShape, UNO_QUERY );
    if( xProps.is() )
    {
        WriteFill( xProps );
        WriteOutline( xProps );
    }
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape );

    pFS->endElementNS( mnXmlNamespace, XML_sp );

    return *this;
}

ShapeExport& ShapeExport::WriteFill( Reference< XPropertySet > xPropSet )
{
    FillStyle aFillStyle( FillStyle_NONE );
    xPropSet->getPropertyValue( S( "FillStyle" ) ) >>= aFillStyle;

    if( aFillStyle == FillStyle_BITMAP )
    {
        //DBG(printf ("FillStyle_BITMAP properties\n"));
        //DBG(dump_pset(rXPropSet));
    }

    if( aFillStyle == FillStyle_NONE ||
        aFillStyle == FillStyle_HATCH ) 
        return *this;
    
    switch( aFillStyle )
    {
    case ::com::sun::star::drawing::FillStyle_SOLID :
        WriteSolidFill( xPropSet );
        break;
    case ::com::sun::star::drawing::FillStyle_GRADIENT :
        WriteGradientFill( xPropSet );
        break;
    case ::com::sun::star::drawing::FillStyle_BITMAP :
        WriteBlipFill( xPropSet, S( "FillBitmapURL" ) );
        break;
    default:
        ;
    }

    return *this;
}

ShapeExport& ShapeExport::WriteGraphicObjectShape( Reference< XShape > xShape )
{
    DBG(printf("write graphic object shape\n"));

    if( NonEmptyText( xShape ) )
    {
        WriteTextShape( xShape );

        //DBG(dump_pset(mXPropSet));

        return *this;
    }

    DBG(printf("graphicObject without text\n"));

    OUString sGraphicURL;
    Reference< XPropertySet > xShapeProps( xShape, UNO_QUERY );
    if( !xShapeProps.is() || !( xShapeProps->getPropertyValue( S( "GraphicURL" ) ) >>= sGraphicURL ) )
    {
        DBG(printf("no graphic URL found\n"));
        return *this;
    }

    FSHelperPtr pFS = GetFS();

    pFS->startElementNS( mnXmlNamespace, XML_pic, FSEND );
               
    pFS->startElementNS( mnXmlNamespace, XML_nvPicPr, FSEND );

    OUString sName, sDescr;
    bool bHaveName = xShapeProps->getPropertyValue( S( "Name" ) ) >>= sName;
    bool bHaveDesc = xShapeProps->getPropertyValue( S( "Description" ) ) >>= sDescr;

    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id,     I32S( GetNewShapeID( xShape ) ),
                          XML_name,   bHaveName ? USS( sName ) : (OString("Picture ") + OString::valueOf( mnPictureIdMax++ )).getStr(),
                          XML_descr,  bHaveDesc ? USS( sDescr ) : NULL,
                          FSEND );
    // OOXTODO: //cNvPr children: XML_extLst, XML_hlinkClick, XML_hlinkHover

    pFS->singleElementNS( mnXmlNamespace, XML_cNvPicPr,
                          // OOXTODO: XML_preferRelativeSize
                          FSEND );

    WriteNonVisualProperties( xShape );

    pFS->endElementNS( mnXmlNamespace, XML_nvPicPr );

    pFS->startElementNS( mnXmlNamespace, XML_blipFill, FSEND );
    
    WriteBlip( sGraphicURL );

    bool bStretch = false;
    if( ( xShapeProps->getPropertyValue( S( "FillBitmapStretch" ) ) >>= bStretch ) && bStretch )
    {
        WriteStretch();
    }

    pFS->endElementNS( mnXmlNamespace, XML_blipFill );

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteShapeTransformation( xShape );
    WritePresetShape( "rect" );
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    pFS->endElementNS( mnXmlNamespace, XML_pic );

    return *this;
}

ShapeExport& ShapeExport::WriteConnectorShape( Reference< XShape > xShape )
{
    sal_Bool bFlipH = false;
    sal_Bool bFlipV = false;

    DBG(printf("write connector shape\n"));

    FSHelperPtr pFS = GetFS();

    const char* sGeometry = "line";
    Reference< XPropertySet > rXPropSet( xShape, UNO_QUERY );
    Reference< XPropertyState > rXPropState( xShape, UNO_QUERY );
    awt::Point aStartPoint, aEndPoint;
    Reference< XShape > rXShapeA;
    Reference< XShape > rXShapeB;
    PropertyState eState;
    ConnectorType eConnectorType;
    if( GETAD( EdgeKind ) ) {
        mAny >>= eConnectorType;

        switch( eConnectorType ) {
            case ConnectorType_CURVE:
                sGeometry = "curvedConnector3";
                break;
            case ConnectorType_STANDARD:
                sGeometry = "bentConnector3";
                break;
            default:
            case ConnectorType_LINE:
            case ConnectorType_LINES:
                sGeometry = "straightConnector1";
                break;
        }

        if( GETAD( EdgeStartPoint ) ) {
            mAny >>= aStartPoint;
            if( GETAD( EdgeEndPoint ) ) {
                mAny >>= aEndPoint;
            }
        }
        GET( rXShapeA, EdgeStartConnection );
        GET( rXShapeB, EdgeEndConnection );
    }
    EscherConnectorListEntry aConnectorEntry( xShape, aStartPoint, rXShapeA, aEndPoint, rXShapeB );

    Rectangle aRect( Point( aStartPoint.X, aStartPoint.Y ), Point( aEndPoint.X, aEndPoint.Y ) );
    if( aRect.getWidth() < 0 ) {
        bFlipH = TRUE;
        aRect.setX( aEndPoint.X );
        aRect.setWidth( aStartPoint.X - aEndPoint.X );
    }

    if( aRect.getHeight() < 0 ) {
        bFlipV = TRUE;
        aRect.setY( aEndPoint.Y );
        aRect.setHeight( aStartPoint.Y - aEndPoint.Y );
    }

    pFS->startElementNS( mnXmlNamespace, XML_cxnSp, FSEND );

    // non visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_nvCxnSpPr, FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id, I32S( GetNewShapeID( xShape ) ),
                          XML_name, IDS( Line ),
                          FSEND );
    // non visual connector shape drawing properties
    pFS->startElementNS( mnXmlNamespace, XML_cNvCxnSpPr, FSEND );
    WriteConnectorConnections( aConnectorEntry, GetShapeID( rXShapeA ), GetShapeID( rXShapeB ) );
    pFS->endElementNS( mnXmlNamespace, XML_cNvCxnSpPr );
    pFS->singleElementNS( mnXmlNamespace, XML_nvPr, FSEND );
    pFS->endElementNS( mnXmlNamespace, XML_nvCxnSpPr );

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteTransformation( aRect, bFlipH, bFlipV );
    // TODO: write adjustments (ppt export doesn't work well there either)
    WritePresetShape( sGeometry );
    Reference< XPropertySet > xShapeProps( xShape, UNO_QUERY );
    if( xShapeProps.is() )
        WriteOutline( xShapeProps );
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape );

    pFS->endElementNS( mnXmlNamespace, XML_cxnSp );

    return *this;
}

ShapeExport& ShapeExport::WriteLineShape( Reference< XShape > xShape )
{
    sal_Bool bFlipH = false;
    sal_Bool bFlipV = false;

    DBG(printf("write line shape\n"));

    FSHelperPtr pFS = GetFS();

    pFS->startElementNS( mnXmlNamespace, XML_sp, FSEND );

    PolyPolygon aPolyPolygon = EscherPropertyContainer::GetPolyPolygon( xShape );
    if( aPolyPolygon.Count() == 1 && aPolyPolygon[ 0 ].GetSize() == 2)
    {
        const Polygon& rPoly = aPolyPolygon[ 0 ];

        bFlipH = ( rPoly[ 0 ].X() > rPoly[ 1 ].X() );
        bFlipV = ( rPoly[ 0 ].Y() > rPoly[ 1 ].Y() );
    }

    // non visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_nvSpPr, FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id, I32S( GetNewShapeID( xShape ) ),
                          XML_name, IDS( Line ),
                          FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr, FSEND );
    WriteNonVisualProperties( xShape );
    pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteShapeTransformation( xShape, bFlipH, bFlipV );
    WritePresetShape( "line" );
    Reference< XPropertySet > xShapeProps( xShape, UNO_QUERY );
    if( xShapeProps.is() )
        WriteOutline( xShapeProps );
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape );

    pFS->endElementNS( mnXmlNamespace, XML_sp );

    return *this;
}

ShapeExport& ShapeExport::WriteNonVisualDrawingProperties( Reference< XShape > xShape, const char* pName )
{
    GetFS()->singleElementNS( mnXmlNamespace, XML_cNvPr,
                              XML_id, I32S( GetNewShapeID( xShape ) ),
                              XML_name, pName,
                              FSEND );

    return *this;
}

ShapeExport& ShapeExport::WriteNonVisualProperties( Reference< XShape > )
{
    // Override to generate //nvPr elements.
    return *this;
}

ShapeExport& ShapeExport::WriteRectangleShape( Reference< XShape > xShape )
{
    DBG(printf("write rectangle shape\n"));

    FSHelperPtr pFS = GetFS();

    pFS->startElementNS( mnXmlNamespace, XML_sp, FSEND );

    sal_Int32 nRadius = 0;

    Reference< XPropertySet > xShapeProps( xShape, UNO_QUERY );
    if( xShapeProps.is() )
    {
        xShapeProps->getPropertyValue( S( "CornerRadius" ) ) >>= nRadius;
    }

    if( nRadius )
    {
        nRadius = MapSize( awt::Size( nRadius, 0 ) ).Width;
    }

    // non visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_nvSpPr, FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id, I32S( GetNewShapeID( xShape ) ),
                          XML_name, IDS( Rectangle ),
                          FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr, FSEND );
    WriteNonVisualProperties( xShape );
    pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteShapeTransformation( xShape );
    WritePresetShape( "rect" );
    Reference< XPropertySet > xProps( xShape, UNO_QUERY );
    if( xProps.is() )
    {
        WriteFill( xProps );
        WriteOutline( xProps );
    }
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape );

    pFS->endElementNS( mnXmlNamespace, XML_sp );

    return *this;
}

typedef ShapeExport& (ShapeExport::*ShapeConverter)( Reference< XShape > );
typedef std::hash_map< const char*, ShapeConverter, std::hash<const char*>, StringCheck> NameToConvertMapType;

static const NameToConvertMapType& lcl_GetConverters()
{
    static bool shape_map_inited = false;
    static NameToConvertMapType shape_converters;
    if( shape_map_inited )
    {
        return shape_converters;
    }

    shape_converters[ "com.sun.star.drawing.ClosedBezierShape" ]        = &ShapeExport::WriteClosedBezierShape;
    shape_converters[ "com.sun.star.drawing.ConnectorShape" ]           = &ShapeExport::WriteConnectorShape;
    shape_converters[ "com.sun.star.drawing.CustomShape" ]              = &ShapeExport::WriteCustomShape;
    shape_converters[ "com.sun.star.drawing.EllipseShape" ]             = &ShapeExport::WriteEllipseShape;
    shape_converters[ "com.sun.star.drawing.GraphicObjectShape" ]       = &ShapeExport::WriteGraphicObjectShape;
    shape_converters[ "com.sun.star.drawing.LineShape" ]                = &ShapeExport::WriteLineShape;
    shape_converters[ "com.sun.star.drawing.OpenBezierShape" ]          = &ShapeExport::WriteOpenBezierShape;
    shape_converters[ "com.sun.star.drawing.RectangleShape" ]           = &ShapeExport::WriteRectangleShape;
    shape_converters[ "com.sun.star.drawing.TextShape" ]                = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.DateTimeShape" ]       = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.FooterShape" ]         = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.HeaderShape" ]         = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.NotesShape" ]          = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.OutlinerShape" ]       = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.SlideNumberShape" ]    = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.TitleTextShape" ]      = &ShapeExport::WriteTextShape;
    shape_map_inited = true;

    return shape_converters;
}

ShapeExport& ShapeExport::WriteShape( Reference< XShape > xShape )
{
    OUString sShapeType = xShape->getShapeType();
    DBG( printf( "write shape: %s\n", USS( sShapeType ) ) );
    NameToConvertMapType::const_iterator aConverter = lcl_GetConverters().find( USS( sShapeType ) );
    if( aConverter == lcl_GetConverters().end() )
    {
        DBG( printf( "unknown shape\n" ) );
        return WriteUnknownShape( xShape );
    }
    (this->*(aConverter->second))( xShape );

    return *this;
}

ShapeExport& ShapeExport::WriteTextBox( Reference< XShape > xShape )
{
    if( NonEmptyText( xShape ) )
    {
        FSHelperPtr pFS = GetFS();

        pFS->startElementNS( mnXmlNamespace, XML_txBody, FSEND );
        WriteText( xShape );
        pFS->endElementNS( mnXmlNamespace, XML_txBody );
    }

    return *this;
}

ShapeExport& ShapeExport::WriteTextShape( Reference< XShape > xShape )
{
    FSHelperPtr pFS = GetFS();

    pFS->startElementNS( mnXmlNamespace, XML_sp, FSEND );

    // non visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_nvSpPr, FSEND );
    WriteNonVisualDrawingProperties( xShape, IDS( TextShape ) );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr, XML_txBox, "1", FSEND );
    WriteNonVisualProperties( xShape );
    pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteShapeTransformation( xShape );
    WritePresetShape( "rect" );
    WriteBlipFill( Reference< XPropertySet >(xShape, UNO_QUERY ), S( "GraphicURL" ) );
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    WriteTextBox( xShape );

    pFS->endElementNS( mnXmlNamespace, XML_sp );

    return *this;
}

ShapeExport& ShapeExport::WriteUnknownShape( Reference< XShape > )
{
    // Override this method to do something useful.
    return *this;
}

size_t ShapeExport::ShapeHash::operator()( const ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShape > rXShape ) const
{
    return maHashFunction( USS( rXShape->getShapeType() ) );
}

sal_Int32 ShapeExport::GetNewShapeID( const Reference< XShape > rXShape )
{
    sal_Int32 nID = GetFB()->GetUniqueId();

    maShapeMap[ rXShape ] = nID;

    return nID;
}

sal_Int32 ShapeExport::GetShapeID( const Reference< XShape > rXShape )
{
    ShapeHashMap::const_iterator aIter = maShapeMap.find( rXShape );

    if( aIter == maShapeMap.end() )
        return -1;

    return aIter->second;
}

} }
