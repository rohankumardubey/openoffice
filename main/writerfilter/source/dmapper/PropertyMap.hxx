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


#ifndef INCLUDED_DMAPPER_PROPERTYMAP_HXX
#define INCLUDED_DMAPPER_PROPERTYMAP_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HXX_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Any.h>
#include <PropertyIds.hxx>
#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>

#include <resourcemodel/TagLogger.hxx>

namespace com{namespace sun{namespace star{
    namespace beans{
    struct PropertyValue;
    }
    namespace container{
        class XNameAccess;
        class XNameContainer;
    }
    namespace lang{
        class XMultiServiceFactory;
    }
    namespace text{
        class XTextRange;
        class XTextColumns;
        class XFootnote;
    }
    namespace table{
        struct BorderLine;
    }
}}}

namespace writerfilter {
namespace dmapper{
class DomainMapper_Impl;

enum BorderPosition
{
    BORDER_LEFT,
    BORDER_RIGHT,
    BORDER_TOP,
    BORDER_BOTTOM
};
/*-- 15.06.2006 08:22:33---------------------------------------------------

  -----------------------------------------------------------------------*/
struct PropertyDefinition
{
    PropertyIds eId;
    bool        bIsTextProperty;

    PropertyDefinition( PropertyIds _eId, bool _bIsTextProperty ) :
        eId( _eId ),
        bIsTextProperty( _bIsTextProperty ){}

    bool    operator== (const PropertyDefinition& rDef) const
            {   return rDef.eId == eId; }        
    bool    operator< (const PropertyDefinition& rDef) const
            {   return eId < rDef.eId; } 
};    
typedef std::map < PropertyDefinition, ::com::sun::star::uno::Any > _PropertyMap;
class PropertyMap : public _PropertyMap
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   m_aValues;
    //marks context as footnote context - ::text( ) events contain either the footnote character or can be ignored
    //depending on sprmCSymbol
    sal_Unicode                                                                 m_cFootnoteSymbol; // 0 == invalid
    sal_Int32                                                                   m_nFootnoteFontId; // negative values are invalid ids
    ::rtl::OUString                                                             m_sFootnoteFontName;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XFootnote >       m_xFootnote;

protected:
    void Invalidate()
    {
        if(m_aValues.getLength())
            m_aValues.realloc( 0 );
    }
    
public:
    PropertyMap();
    virtual ~PropertyMap();

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetPropertyValues();
    bool hasEmptyPropertyValues() const {return !m_aValues.getLength();}
    /** Add property, usually overwrites already available attributes. It shouldn't overwrite in case of default attributes
     */
    void Insert( PropertyIds eId, bool bIsTextProperty, const ::com::sun::star::uno::Any& rAny, bool bOverwrite = true );
    using _PropertyMap::insert;
    void insert(const boost::shared_ptr<PropertyMap> pMap, bool bOverwrite = true);

    const ::com::sun::star::uno::Reference< ::com::sun::star::text::XFootnote>&  GetFootnote() const;
    void SetFootnote( ::com::sun::star::uno::Reference< ::com::sun::star::text::XFootnote> xF ) { m_xFootnote = xF; }

    sal_Unicode GetFootnoteSymbol() const { return m_cFootnoteSymbol;}
    void        SetFootnoteSymbol(sal_Unicode cSet) { m_cFootnoteSymbol = cSet;}
    
    sal_Int32   GetFootnoteFontId() const { return m_nFootnoteFontId;}
    void        SetFootnoteFontId(sal_Int32 nSet) { m_nFootnoteFontId = nSet;}

    const ::rtl::OUString&      GetFootnoteFontName() const { return m_sFootnoteFontName;}
    void                        SetFootnoteFontName( const ::rtl::OUString& rSet ) { m_sFootnoteFontName = rSet;}

    virtual void insertTableProperties( const PropertyMap* );
    
    virtual XMLTag::Pointer_t toTag() const;    
};
typedef boost::shared_ptr<PropertyMap>  PropertyMapPtr;

/*-- 24.07.2006 08:26:33---------------------------------------------------

  -----------------------------------------------------------------------*/
class SectionPropertyMap : public PropertyMap
{
    //--> debug
    sal_Int32 nSectionNumber;
    //<-- debug
    //'temporarily' the section page settings are imported as page styles
    // empty strings mark page settings as not yet imported

    bool                                                                        m_bIsFirstSection;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >      m_xStartingRange;

    ::rtl::OUString                                                             m_sFirstPageStyleName;
    ::rtl::OUString                                                             m_sFollowPageStyleName;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_aFirstPageStyle;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_aFollowPageStyle;

    ::com::sun::star::table::BorderLine*    m_pBorderLines[4];
    sal_Int32                               m_nBorderDistances[4];
    sal_Int32                               m_nBorderParams;

    bool                                    m_bTitlePage;
    sal_Int16                               m_nColumnCount;
    sal_Int32                               m_nColumnDistance;
    ::std::vector< sal_Int32 >              m_aColWidth;
    ::std::vector< sal_Int32 >              m_aColDistance;

    bool                                    m_bSeparatorLineIsOn;
    bool                                    m_bEvenlySpaced;
    bool                                    m_bIsLandscape;

    bool                                    m_bPageNoRestart;
    sal_Int32                               m_nPageNumber;
    sal_Int32                               m_nBreakType;
    sal_Int32                               m_nPaperBin;
    sal_Int32                               m_nFirstPaperBin;

    sal_Int32                               m_nLeftMargin;
    sal_Int32                               m_nRightMargin;
    sal_Int32                               m_nTopMargin;
    sal_Int32                               m_nBottomMargin;
    sal_Int32                               m_nHeaderTop;
    sal_Int32                               m_nHeaderBottom;

    sal_Int32                               m_nDzaGutter;
    bool                                    m_bGutterRTL;
    bool                                    m_bSFBiDi;

    sal_Int32                               m_nGridType;
    sal_Int32                               m_nGridLinePitch;
    sal_Int32                               m_nDxtCharSpace;

    //line numbering
    sal_Int32                               m_nLnnMod;
    sal_Int32                               m_nLnc;
    sal_Int32                               m_ndxaLnn;
    sal_Int32                               m_nLnnMin;

    void _ApplyProperties( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xStyle );
    ::com::sun::star::uno::Reference< com::sun::star::text::XTextColumns > ApplyColumnProperties(
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xFollowPageStyle );
    void CopyLastHeaderFooter( bool bFirstPage, DomainMapper_Impl& rDM_Impl );
    void PrepareHeaderFooterProperties( bool bFirstPage );
    bool HasHeader( bool bFirstPage ) const;
    bool HasFooter( bool bFirstPage ) const;

    void SetBorderDistance( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xStyle, 
        PropertyIds eMarginId, PropertyIds eDistId, sal_Int32 nDistance, sal_Int32 nOffsetFrom );

public:
        explicit SectionPropertyMap(bool bIsFirstSection);
        ~SectionPropertyMap();

    enum PageType
    {
        PAGE_FIRST,
        PAGE_LEFT,
        PAGE_RIGHT
    };

    void SetStart( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange )
    {
        m_xStartingRange = xRange;
    }

    const ::rtl::OUString&  GetPageStyleName( bool bFirst );
    void                    SetPageStyleName( bool bFirst, const ::rtl::OUString& rName);

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > GetPageStyle(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xStyles,
            const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >& xTextFactory,
            bool bFirst );

    void SetBorder( BorderPosition ePos, sal_Int32 nLineDistance, const ::com::sun::star::table::BorderLine& rBorderLine );
    void SetBorderParams( sal_Int32 nSet ) { m_nBorderParams = nSet; }

    void SetColumnCount( sal_Int16 nCount ) { m_nColumnCount = nCount; }
    void SetColumnDistance( sal_Int32 nDist ) { m_nColumnDistance = nDist; }
    void AppendColumnWidth( sal_Int32 nWidth ) { m_aColWidth.push_back( nWidth ); }
    void AppendColumnSpacing( sal_Int32 nDist ) {m_aColDistance.push_back( nDist ); }

    void SetTitlePage( bool bSet ) { m_bTitlePage = bSet; }
    void SetSeparatorLine( bool bSet ) { m_bSeparatorLineIsOn = bSet; }
    void SetEvenlySpaced( bool bSet ) {    m_bEvenlySpaced = bSet; }
    void SetLandscape( bool bSet ) { m_bIsLandscape = bSet; }
    void SetPageNoRestart( bool bSet ) { m_bPageNoRestart = bSet; }
    void SetPageNumber( sal_Int32 nSet ) { m_nPageNumber = nSet; }
    void SetBreakType( sal_Int32 nSet ) { m_nBreakType = nSet; }
    void SetPaperBin( sal_Int32 nSet );
    void SetFirstPaperBin( sal_Int32 nSet );

    void SetLeftMargin(    sal_Int32 nSet ) { m_nLeftMargin = nSet; }
    void SetRightMargin( sal_Int32 nSet ) { m_nRightMargin = nSet; }
    void SetTopMargin(    sal_Int32 nSet ) { m_nTopMargin = nSet; }
    void SetBottomMargin( sal_Int32 nSet ) { m_nBottomMargin = nSet; }
    void SetHeaderTop(    sal_Int32 nSet ) { m_nHeaderTop = nSet; }
    void SetHeaderBottom( sal_Int32 nSet ) { m_nHeaderBottom = nSet; }

    void SetGutterRTL( bool bSet ) { m_bGutterRTL = bSet;}
    void SetDzaGutter( sal_Int32 nSet ) {m_nDzaGutter = nSet; }
    void SetSFBiDi( bool bSet ) { m_bSFBiDi = bSet;}

    void SetGridType(sal_Int32 nSet) { m_nGridType = nSet; }
    void SetGridLinePitch( sal_Int32 nSet ) { m_nGridLinePitch = nSet; }
    void SetDxtCharSpace( sal_Int32 nSet ) { m_nDxtCharSpace = nSet; }

    void SetLnnMod( sal_Int32 nValue ) { m_nLnnMod = nValue; }
    void SetLnc(    sal_Int32 nValue ) { m_nLnc    = nValue; }
    void SetdxaLnn( sal_Int32 nValue ) { m_ndxaLnn  = nValue; }
    void SetLnnMin( sal_Int32 nValue ) { m_nLnnMin = nValue; }

    //determine which style gets the borders
    void ApplyBorderToPageStyles(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xStyles,
            const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >& xTextFactory,
            sal_Int32 nValue );

    void CloseSectionGroup( DomainMapper_Impl& rDM_Impl );
};
typedef boost::shared_ptr<SectionPropertyMap> SectionPropertyMapPtr;

/*-- 28.12.2007 08:17:34---------------------------------------------------

  -----------------------------------------------------------------------*/
class ParagraphProperties
{
    bool                    m_bFrameMode;
    sal_Int32               m_nDropCap; //drop, margin ST_DropCap
    sal_Int32               m_nLines; //number of lines of the drop cap
    sal_Int32               m_w;    //width
    sal_Int32               m_h;    //height
    sal_Int32               m_nWrap;   // from ST_Wrap around, auto, none, notBeside, through, tight
    sal_Int32               m_hAnchor; // page, from ST_HAnchor  margin, page, text
    sal_Int32               m_vAnchor; // around from ST_VAnchor margin, page, text
    sal_Int32               m_x; //x-position
    bool                    m_bxValid;
    sal_Int32               m_y; //y-position
    bool                    m_byValid;
    sal_Int32               m_hSpace; //frame padding h
    sal_Int32               m_vSpace; //frame padding v
    sal_Int32               m_hRule; //  from ST_HeightRule exact, atLeast, auto
    sal_Int32               m_xAlign; // from ST_XAlign center, inside, left, outside, right
    sal_Int32               m_yAlign; // from ST_YAlign bottom, center, inline, inside, outside, top
    bool                    m_bAnchorLock;

    sal_Int8                m_nDropCapLength; //number of characters

    ::rtl::OUString         m_sParaStyleName;

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >      m_xStartingRange; //start of a frame
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >      m_xEndingRange; //end of the frame

public: 
    ParagraphProperties();
    ParagraphProperties(const ParagraphProperties&); 
    ~ParagraphProperties();

    int operator==(const ParagraphProperties&); //does not compare the starting/ending range, m_sParaStyleName and m_nDropCapLength
    
    void    SetFrameMode() { m_bFrameMode = true; }
    bool    IsFrameMode()const { return m_bFrameMode; }

    void SetDropCap( sal_Int32 nSet ) { m_nDropCap = nSet; }
    sal_Int32 GetDropCap()const { return m_nDropCap; }
    
    void SetLines( sal_Int32 nSet ) { m_nLines = nSet; }
    sal_Int32 GetLines() const { return m_nLines; }
    
    void Setw( sal_Int32 nSet ) { m_w = nSet; }
    sal_Int32 Getw() const { return m_w; }
    
    void Seth( sal_Int32 nSet ) { m_h = nSet; }
    sal_Int32 Geth() const { return m_h; }
    
    void SetWrap( sal_Int32 nSet ) { m_nWrap = nSet; }
    sal_Int32 GetWrap() const { return m_nWrap; }
    
    void SethAnchor( sal_Int32 nSet ) { m_hAnchor = nSet; }
    sal_Int32 GethAnchor() const { return m_hAnchor;}

    void SetvAnchor( sal_Int32 nSet ) { m_vAnchor = nSet; }
    sal_Int32 GetvAnchor() const { return m_vAnchor; }

    void Setx( sal_Int32 nSet ) { m_x = nSet; m_bxValid = true;}
    sal_Int32 Getx() const { return m_x; }
    bool IsxValid() const {return m_bxValid;}
    
    void Sety( sal_Int32 nSet ) { m_y = nSet; m_byValid = true;}
    sal_Int32 Gety()const { return m_y; }
    bool IsyValid() const {return m_byValid;}
    
    void SethSpace( sal_Int32 nSet ) { m_hSpace = nSet; }
    sal_Int32 GethSpace()const { return m_hSpace; }
    
    void SetvSpace( sal_Int32 nSet ) { m_vSpace = nSet; }
    sal_Int32 GetvSpace()const { return m_vSpace; }
    
    void SethRule( sal_Int32 nSet ) { m_hRule = nSet; }
    sal_Int32 GethRule() const  { return m_hRule; }
    
    void SetxAlign( sal_Int32 nSet ) { m_xAlign = nSet; }
    sal_Int32 GetxAlign()const { return m_xAlign; }
    
    void SetyAlign( sal_Int32 nSet ) { m_yAlign = nSet; }
    sal_Int32 GetyAlign()const { return m_yAlign; }
    
    void SetAnchorLock( bool bSet ) {m_bAnchorLock = bSet; }

    sal_Int8    GetDropCapLength() const { return m_nDropCapLength;}
    void        SetDropCapLength(sal_Int8 nSet) { m_nDropCapLength = nSet;}

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > GetStartingRange() const { return m_xStartingRange; }
    void SetStartingRange( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > xSet ) { m_xStartingRange = xSet; }

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > GetEndingRange() const { return m_xEndingRange; }
    void SetEndingRange( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > xSet ) { m_xEndingRange = xSet; }

    void                    SetParaStyleName( const ::rtl::OUString& rSet ) { m_sParaStyleName = rSet;}
    const ::rtl::OUString&  GetParaStyleName() const { return m_sParaStyleName;}


};
typedef boost::shared_ptr<ParagraphProperties>  ParagraphPropertiesPtr;
/*-- 14.06.2007 12:12:34---------------------------------------------------
    property map of a stylesheet 
  -----------------------------------------------------------------------*/

#define WW_OUTLINE_MAX  sal_Int16( 9 )

class StyleSheetPropertyMap : public PropertyMap, public ParagraphProperties

{
    //special table style properties
//    sal_Int32               mnCT_Spacing_after;
    sal_Int32               mnCT_Spacing_line;
    sal_Int32               mnCT_Spacing_lineRule;

    ::rtl::OUString         msCT_Fonts_ascii;
    bool                    mbCT_TrPrBase_tblHeader;
    sal_Int32               mnCT_TrPrBase_jc;
    sal_Int32               mnCT_TcPrBase_vAlign;

    sal_Int32               mnCT_TblWidth_w;
    sal_Int32               mnCT_TblWidth_type;

//    bool                    mbCT_Spacing_afterSet;
    bool                    mbCT_Spacing_lineSet;
    bool                    mbCT_Spacing_lineRuleSet;

    bool                    mbCT_TrPrBase_tblHeaderSet;
    bool                    mbCT_TrPrBase_jcSet;
    bool                    mbCT_TcPrBase_vAlignSet;

    bool                    mbCT_TblWidth_wSet;
    bool                    mbCT_TblWidth_typeSet;

    sal_Int32               mnListId;
    sal_Int16               mnListLevel;

    sal_Int16               mnOutlineLevel;
public: 
    explicit StyleSheetPropertyMap();
    ~StyleSheetPropertyMap();

//    void SetCT_Spacing_after(      sal_Int32 nSet )              
//        {mnCT_Spacing_after = nSet;    mbCT_Spacing_afterSet = true;        }
    void SetCT_Spacing_line(       sal_Int32 nSet )              
        {mnCT_Spacing_line = nSet;     mbCT_Spacing_lineSet = true;         }
    void SetCT_Spacing_lineRule(   sal_Int32  nSet )             
        {mnCT_Spacing_lineRule = nSet; mbCT_Spacing_lineRuleSet = true;     }

    void SetCT_Fonts_ascii(  const ::rtl::OUString& rSet )       
        {msCT_Fonts_ascii = rSet;          }
    void SetCT_TrPrBase_tblHeader( bool bSet )                   
        {mbCT_TrPrBase_tblHeader = bSet; mbCT_TrPrBase_tblHeaderSet = true; }
    void SetCT_TrPrBase_jc(        sal_Int32 nSet )              
        {mnCT_TrPrBase_jc = nSet;        mbCT_TrPrBase_jcSet = true;     }
    void SetCT_TcPrBase_vAlign(    sal_Int32 nSet )              
        {mnCT_TcPrBase_vAlign = nSet;    mbCT_TcPrBase_vAlignSet = true; }

    void SetCT_TblWidth_w( sal_Int32 nSet )              
        { mnCT_TblWidth_w = nSet;    mbCT_TblWidth_wSet = true; }
    void SetCT_TblWidth_type( sal_Int32 nSet )              
        {mnCT_TblWidth_type = nSet;    mbCT_TblWidth_typeSet = true; }

//    bool GetCT_Spacing_after(   sal_Int32& rToFill) const          
//    {
//        if( mbCT_Spacing_afterSet )
//            rToFill = mnCT_Spacing_after;       
//        return mbCT_Spacing_afterSet;   
//    }
    bool GetCT_Spacing_line(    sal_Int32& rToFill) const          
    {
        if( mbCT_Spacing_lineSet )
            rToFill = mnCT_Spacing_line;        
        return mbCT_Spacing_lineSet;    
    }
    bool GetCT_Spacing_lineRule(sal_Int32& rToFill) const          
    {
        if( mbCT_Spacing_lineRuleSet )
            rToFill = mnCT_Spacing_lineRule;    
        return mbCT_Spacing_lineRuleSet;
    }

    bool GetCT_Fonts_ascii(::rtl::OUString& rToFill) const         
    {
        if( msCT_Fonts_ascii.getLength() > 0 )
            rToFill = msCT_Fonts_ascii;         
        return msCT_Fonts_ascii.getLength() > 0; 
    }
    bool GetCT_TrPrBase_tblHeader(bool& rToFill) const             
    {
        if( mbCT_TrPrBase_tblHeaderSet )      
            rToFill = mbCT_TrPrBase_tblHeader;  
        return mbCT_TrPrBase_tblHeaderSet; 
    }
    bool GetCT_TrPrBase_jc(     sal_Int32& rToFill)const           
    {
        if( mbCT_TrPrBase_jcSet )
            rToFill = mnCT_TrPrBase_jc;         
        return mbCT_TrPrBase_jcSet;        
    }
    bool GetCT_TcPrBase_vAlign( sal_Int32& rToFill)const           
    {
        if( mbCT_TcPrBase_vAlignSet )      
            rToFill = mnCT_TcPrBase_vAlign;     
        return mbCT_TcPrBase_vAlignSet;   
    }
    sal_Int32   GetListId() const               { return mnListId; }
    void        SetListId(sal_Int32 nId)        { mnListId = nId; }

    sal_Int16   GetListLevel() const            { return mnListLevel; }
    void        SetListLevel(sal_Int16 nLevel)  { mnListLevel = nLevel; }
    
    sal_Int16   GetOutlineLevel() const            { return mnOutlineLevel; }
    void        SetOutlineLevel(sal_Int16 nLevel)  
    { 
        if ( nLevel < WW_OUTLINE_MAX )
            mnOutlineLevel = nLevel; 
    }
};
/*-- 27.12.2007 12:38:06---------------------------------------------------

  -----------------------------------------------------------------------*/
class ParagraphPropertyMap : public PropertyMap, public ParagraphProperties
{
public:     
    explicit ParagraphPropertyMap();
    ~ParagraphPropertyMap();

};
/*-- 15.02.2008 16:06:52---------------------------------------------------

  -----------------------------------------------------------------------*/
class TablePropertyMap : public PropertyMap
{
public:    
    enum TablePropertyMapTarget
    {
        TablePropertyMapTarget_START,
        CELL_MAR_LEFT = TablePropertyMapTarget_START,
        CELL_MAR_RIGHT,
        CELL_MAR_TOP,
        CELL_MAR_BOTTOM,
        TABLE_WIDTH,
        GAP_HALF,
        LEFT_MARGIN,
        HORI_ORIENT,
        TablePropertyMapTarget_MAX
    };
private:
    struct ValidValue 
    {
        sal_Int32   nValue;
        bool        bValid;
        ValidValue() : 
            nValue( 0 ), 
            bValid( false ){}
    };
    ValidValue m_aValidValues[TablePropertyMapTarget_MAX];

public:     
    explicit TablePropertyMap();
    ~TablePropertyMap();

    bool    getValue( TablePropertyMapTarget eWhich, sal_Int32& nFill );
    void    setValue( TablePropertyMapTarget eWhich, sal_Int32 nSet );

    virtual void insertTableProperties( const PropertyMap* );
};
typedef boost::shared_ptr<TablePropertyMap>  TablePropertyMapPtr;
} //namespace dmapper
} //namespace writerfilter
#endif
