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
#include "precompiled_sc.hxx"
#include "xecontent.hxx"

#include <list>
#include <algorithm>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/XAreaLinks.hpp>
#include <com/sun/star/sheet/XAreaLink.hpp>
#include <sfx2/objsh.hxx>
#include <tools/urlobj.hxx>
#include <svl/itemset.hxx>
#include <formula/grammar.hxx>
#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include "document.hxx"
#include "validat.hxx"
#include "unonames.hxx"
#include "convuno.hxx"
#include "rangenam.hxx"
#include "tokenarray.hxx"
#include "stlpool.hxx"
#include "patattr.hxx"
#include "fapihelper.hxx"
#include "xehelper.hxx"
#include "xestyle.hxx"
#include "xename.hxx"

using namespace ::oox;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::table::CellRangeAddress;
using ::com::sun::star::sheet::XAreaLinks;
using ::com::sun::star::sheet::XAreaLink;
using ::rtl::OString;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// Shared string table ========================================================

// 1 = SST hash table statistics prompt
#define EXC_INCL_SST_STATISTICS 0

// ----------------------------------------------------------------------------

/** A single string entry in the hash table. */
struct XclExpHashEntry
{
    const XclExpString* mpString;       /// Pointer to the string (no ownership).
    sal_uInt32          mnSstIndex;     /// The SST index of this string.
    inline explicit     XclExpHashEntry( const XclExpString* pString = 0, sal_uInt32 nSstIndex = 0 ) :
                            mpString( pString ), mnSstIndex( nSstIndex ) {}
};

/** Function object for strict weak ordering. */
struct XclExpHashEntrySWO
{
    inline bool         operator()( const XclExpHashEntry& rLeft, const XclExpHashEntry& rRight ) const
                            { return *rLeft.mpString < *rRight.mpString; }
};

// ----------------------------------------------------------------------------

/** Implementation of the SST export.
    @descr  Stores all passed strings in a hash table and prevents repeated
    insertion of equal strings. */
class XclExpSstImpl
{
public:
    explicit            XclExpSstImpl();

    /** Inserts the passed string, if not already inserted, and returns the unique SST index. */
    sal_uInt32          Insert( XclExpStringRef xString );

    /** Writes the complete SST and EXTSST records. */
    void                Save( XclExpStream& rStrm );
    void                SaveXml( XclExpXmlStream& rStrm );

private:
    typedef ::std::list< XclExpStringRef >      XclExpStringList;
    typedef ::std::vector< XclExpHashEntry >    XclExpHashVec;
    typedef ::std::vector< XclExpHashVec >      XclExpHashTab;

    XclExpStringList    maStringList;   /// List of unique strings (in SST ID order).
    XclExpHashTab       maHashTab;      /// Hashed table that manages string pointers.
    sal_uInt32          mnTotal;        /// Total count of strings (including doubles).
    sal_uInt32          mnSize;         /// Size of the SST (count of unique strings).
};

// ----------------------------------------------------------------------------

const sal_uInt32 EXC_SST_HASHTABLE_SIZE = 2048;

XclExpSstImpl::XclExpSstImpl() :
    maHashTab( EXC_SST_HASHTABLE_SIZE ),
    mnTotal( 0 ),
    mnSize( 0 )
{
}

sal_uInt32 XclExpSstImpl::Insert( XclExpStringRef xString )
{
    DBG_ASSERT( xString.get(), "XclExpSstImpl::Insert - empty pointer not allowed" );
    if( !xString.get() )
        xString.reset( new XclExpString );

    ++mnTotal;
    sal_uInt32 nSstIndex = 0;

    // calculate hash value in range [0,EXC_SST_HASHTABLE_SIZE)
    sal_uInt16 nHash = xString->GetHash();
    (nHash ^= (nHash / EXC_SST_HASHTABLE_SIZE)) %= EXC_SST_HASHTABLE_SIZE;

    XclExpHashVec& rVec = maHashTab[ nHash ];
    XclExpHashEntry aEntry( xString.get(), mnSize );
    XclExpHashVec::iterator aIt = ::std::lower_bound( rVec.begin(), rVec.end(), aEntry, XclExpHashEntrySWO() );
    if( (aIt == rVec.end()) || (*aIt->mpString != *xString) )
    {
        nSstIndex = mnSize;
        maStringList.push_back( xString );
        rVec.insert( aIt, aEntry );
        ++mnSize;
    }
    else
    {
        nSstIndex = aIt->mnSstIndex;
    }

    return nSstIndex;
}

void XclExpSstImpl::Save( XclExpStream& rStrm )
{
    if( maStringList.empty() )
        return;

#if (OSL_DEBUG_LEVEL > 1) && EXC_INCL_SST_STATISTICS
    { // own scope for the statistics
#define APPENDINT( value ) Append( ByteString::CreateFromInt32( value ) )
        ScfUInt32Vec aVec;
        size_t nPerBucket = mnSize / EXC_SST_HASHTABLE_SIZE + 1, nEff = 0;
        for( XclExpHashTab::const_iterator aTIt = maHashTab.begin(), aTEnd = maHashTab.end(); aTIt != aTEnd; ++aTIt )
        {
            size_t nSize = aTIt->size();
            if( nSize >= aVec.size() ) aVec.resize( nSize + 1, 0 );
            ++aVec[ nSize ];
            if( nSize > nPerBucket ) nEff += nSize - nPerBucket;
        }
        ByteString aStr( "SST HASHING STATISTICS\n\n" );
        aStr.Append( "Total count:\t" ).APPENDINT( mnTotal ).Append( " strings\n" );
        aStr.Append( "Reduced to:\t" ).APPENDINT( mnSize ).Append( " strings (" );
        aStr.APPENDINT( 100 * mnSize / mnTotal ).Append( "%)\n" );
        aStr.Append( "Effectivity:\t\t" ).APPENDINT( 100 - 100 * nEff / mnSize );
        aStr.Append( "% (best: " ).APPENDINT( nPerBucket ).Append( " strings per bucket)\n" );
        aStr.Append( "\t\tCount of buckets\nBucket size\ttotal\tmax\tTotal strings\n" );
        for( size_t nIx = 0, nSize = aVec.size(), nInc = 1; nIx < nSize; nIx += nInc )
        {
            if( (nIx == 10) || (nIx == 100) || (nIx == 1000) ) nInc = nIx;
            size_t nMaxIx = ::std::min( nIx + nInc, nSize ), nCount = 0, nMaxCount = 0, nStrings = 0;
            for( size_t nSubIx = nIx; nSubIx < nMaxIx; ++nSubIx )
            {
                nCount += aVec[ nSubIx ];
                if( aVec[ nSubIx ] > nMaxCount ) nMaxCount = aVec[ nSubIx ];
                nStrings += nSubIx * aVec[ nSubIx ];
            }
            if( nMaxCount )
            {
                aStr.APPENDINT( nIx );
                if( nMaxIx - nIx > 1 ) aStr.Append( '-' ).APPENDINT( nMaxIx - 1 );
                aStr.Append( "\t\t" ).APPENDINT( nCount ).Append( '\t' ).APPENDINT( nMaxCount );
                aStr.Append( '\t' ).APPENDINT( nStrings ).Append( '\n' );
            }
        }
        DBG_ERRORFILE( aStr.GetBuffer() );
#undef APPENDINT
    }
#endif

    SvMemoryStream aExtSst( 8192 );

    sal_uInt32 nBucket = mnSize;
    while( nBucket > 0x0100 )
        nBucket /= 2;

    sal_uInt16 nPerBucket = llimit_cast< sal_uInt16 >( nBucket, 8 );
    sal_uInt16 nBucketIndex = 0;

    // *** write the SST record ***

    rStrm.StartRecord( EXC_ID_SST, 8 );

    rStrm << mnTotal << mnSize;
    for( XclExpStringList::const_iterator aIt = maStringList.begin(), aEnd = maStringList.end(); aIt != aEnd; ++aIt )
    {
        if( !nBucketIndex )
        {
            // write bucket info before string to get correct record position
            sal_uInt32 nStrmPos = static_cast< sal_uInt32 >( rStrm.GetSvStreamPos() );
            sal_uInt16 nRecPos = rStrm.GetRawRecPos() + 4;
            aExtSst << nStrmPos             // stream position
                    << nRecPos              // position from start of SST or CONTINUE
                    << sal_uInt16( 0 );     // reserved
        }

        rStrm << **aIt;

        if( ++nBucketIndex == nPerBucket )
            nBucketIndex = 0;
    }

    rStrm.EndRecord();

    // *** write the EXTSST record ***

    rStrm.StartRecord( EXC_ID_EXTSST, 0 );

    rStrm << nPerBucket;
    rStrm.SetSliceSize( 8 );    // size of one bucket info
    aExtSst.Seek( STREAM_SEEK_TO_BEGIN );
    rStrm.CopyFromStream( aExtSst );

    rStrm.EndRecord();
}

void XclExpSstImpl::SaveXml( XclExpXmlStream& rStrm )
{
    if( maStringList.empty() )
        return;

    sax_fastparser::FSHelperPtr pSst = rStrm.CreateOutputStream( 
            OUString::createFromAscii( "xl/sharedStrings.xml" ),
            OUString::createFromAscii( "sharedStrings.xml" ),
            rStrm.GetCurrentStream()->getOutputStream(),
            "application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml",
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings" );
    rStrm.PushStream( pSst );

    pSst->startElement( XML_sst,
            XML_xmlns, "http://schemas.openxmlformats.org/spreadsheetml/2006/main", 
            XML_count, OString::valueOf( (sal_Int32) mnTotal ).getStr(),
            XML_uniqueCount, OString::valueOf( (sal_Int32) mnSize ).getStr(),
            FSEND );

    for( XclExpStringList::const_iterator aIt = maStringList.begin(), aEnd = maStringList.end(); aIt != aEnd; ++aIt )
    {
        pSst->startElement( XML_si, FSEND );
        (*aIt)->WriteXml( rStrm );
        pSst->endElement( XML_si );
    }

    pSst->endElement( XML_sst );

    rStrm.PopStream();
}

// ----------------------------------------------------------------------------

XclExpSst::XclExpSst() :
    mxImpl( new XclExpSstImpl )
{
}

XclExpSst::~XclExpSst()
{
}

sal_uInt32 XclExpSst::Insert( XclExpStringRef xString )
{
    return mxImpl->Insert( xString );
}

void XclExpSst::Save( XclExpStream& rStrm )
{
    mxImpl->Save( rStrm );
}

void XclExpSst::SaveXml( XclExpXmlStream& rStrm )
{
    mxImpl->SaveXml( rStrm );
}

// Merged cells ===============================================================

XclExpMergedcells::XclExpMergedcells( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
}

void XclExpMergedcells::AppendRange( const ScRange& rRange, sal_uInt32 nBaseXFId )
{
    if( GetBiff() == EXC_BIFF8 )
    {
        maMergedRanges.Append( rRange );
        maBaseXFIds.push_back( nBaseXFId );
    }
}

sal_uInt32 XclExpMergedcells::GetBaseXFId( const ScAddress& rPos ) const
{
    DBG_ASSERT( maBaseXFIds.size() == maMergedRanges.Count(), "XclExpMergedcells::GetBaseXFId - invalid lists" );
    ScfUInt32Vec::const_iterator aIt = maBaseXFIds.begin();
    ScRangeList& rNCRanges = const_cast< ScRangeList& >( maMergedRanges );
    for( const ScRange* pScRange = rNCRanges.First(); pScRange; pScRange = rNCRanges.Next(), ++aIt )
        if( pScRange->In( rPos ) )
            return *aIt;
    return EXC_XFID_NOTFOUND;
}

void XclExpMergedcells::Save( XclExpStream& rStrm )
{
    if( GetBiff() == EXC_BIFF8 )
    {
        XclRangeList aXclRanges;
        GetAddressConverter().ConvertRangeList( aXclRanges, maMergedRanges, true );
        size_t nFirstRange = 0;
        size_t nRemainingRanges = aXclRanges.size();
        while( nRemainingRanges > 0 )
        {
            size_t nRangeCount = ::std::min< size_t >( nRemainingRanges, EXC_MERGEDCELLS_MAXCOUNT );
            rStrm.StartRecord( EXC_ID_MERGEDCELLS, 2 + 8 * nRangeCount );
            aXclRanges.WriteSubList( rStrm, nFirstRange, nRangeCount );
            rStrm.EndRecord();
            nFirstRange += nRangeCount;
            nRemainingRanges -= nRangeCount;
        }
    }
}

void XclExpMergedcells::SaveXml( XclExpXmlStream& rStrm )
{
    sal_uLong nCount = maMergedRanges.Count();
    if( !nCount )
        return;
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_mergeCells,
            XML_count,  OString::valueOf( (sal_Int32) nCount ).getStr(),
            FSEND );
    for( sal_uLong i = 0; i < nCount; ++i )
    {
        if( const ScRange* pRange = maMergedRanges.GetObject( i ) )
        {
            rWorksheet->singleElement( XML_mergeCell,
                    XML_ref,    XclXmlUtils::ToOString( *pRange ).getStr(),
                    FSEND );
        }
    }
    rWorksheet->endElement( XML_mergeCells );
}

// Hyperlinks =================================================================

XclExpHyperlink::XclExpHyperlink( const XclExpRoot& rRoot, const SvxURLField& rUrlField, const ScAddress& rScPos ) :
    XclExpRecord( EXC_ID_HLINK ),
    maScPos( rScPos ),
    mxVarData( new SvMemoryStream ),
    mnFlags( 0 )
{
    const String& rUrl = rUrlField.GetURL();
    const String& rRepr = rUrlField.GetRepresentation();
    INetURLObject aUrlObj( rUrl );
    const INetProtocol eProtocol = aUrlObj.GetProtocol();
    bool bWithRepr = rRepr.Len() > 0;
    XclExpStream aXclStrm( *mxVarData, rRoot );         // using in raw write mode.

    // description
    if( bWithRepr )
    {
        XclExpString aDescr( rRepr, EXC_STR_FORCEUNICODE, 255 );
        aXclStrm << sal_uInt32( aDescr.Len() + 1 );     // string length + 1 trailing zero word
        aDescr.WriteBuffer( aXclStrm );                 // NO flags
        aXclStrm << sal_uInt16( 0 );

        mnFlags |= EXC_HLINK_DESCR;
        mxRepr.reset( new String( rRepr ) );
    }

    // file link or URL
    if( eProtocol == INET_PROT_FILE )
    {
        sal_uInt16 nLevel;
        bool bRel;
        String aFileName( BuildFileName( nLevel, bRel, rUrl, rRoot ) );

        if( !bRel )
            mnFlags |= EXC_HLINK_ABS;
        mnFlags |= EXC_HLINK_BODY;

        ByteString aAsciiLink( aFileName, rRoot.GetTextEncoding() );
        XclExpString aLink( aFileName, EXC_STR_FORCEUNICODE, 255 );
        aXclStrm    << XclTools::maGuidFileMoniker
                    << nLevel
                    << sal_uInt32( aAsciiLink.Len() + 1 );      // string length + 1 trailing zero byte
        aXclStrm.Write( aAsciiLink.GetBuffer(), aAsciiLink.Len() );
        aXclStrm    << sal_uInt8( 0 )
                    << sal_uInt32( 0xDEADFFFF );
        aXclStrm.WriteZeroBytes( 20 );
        aXclStrm    << sal_uInt32( aLink.GetBufferSize() + 6 )
                    << sal_uInt32( aLink.GetBufferSize() )      // byte count, not string length
                    << sal_uInt16( 0x0003 );
        aLink.WriteBuffer( aXclStrm );                          // NO flags

        if( !mxRepr.get() )
            mxRepr.reset( new String( aFileName ) );

        msTarget = XclXmlUtils::ToOUString( aLink );
    }
    else if( eProtocol != INET_PROT_NOT_VALID )
    {
        XclExpString aUrl( aUrlObj.GetURLNoMark(), EXC_STR_FORCEUNICODE, 255 );
        aXclStrm    << XclTools::maGuidUrlMoniker
                    << sal_uInt32( aUrl.GetBufferSize() + 2 );  // byte count + 1 trailing zero word
        aUrl.WriteBuffer( aXclStrm );                           // NO flags
        aXclStrm    << sal_uInt16( 0 );

        mnFlags |= EXC_HLINK_BODY | EXC_HLINK_ABS;
        if( !mxRepr.get() )
            mxRepr.reset( new String( rUrl ) );

        msTarget = XclXmlUtils::ToOUString( aUrl );
    }
    else if( rUrl.GetChar( 0 ) == '#' )     // hack for #89066#
    {
        String aTextMark( rUrl.Copy( 1 ) );
        aTextMark.SearchAndReplace( '.', '!' );
        mxTextMark.reset( new XclExpString( aTextMark, EXC_STR_FORCEUNICODE, 255 ) );
    }

    // text mark
    if( !mxTextMark.get() && aUrlObj.HasMark() )
        mxTextMark.reset( new XclExpString( aUrlObj.GetMark(), EXC_STR_FORCEUNICODE, 255 ) );

    if( mxTextMark.get() )
    {
        aXclStrm    << sal_uInt32( mxTextMark->Len() + 1 );  // string length + 1 trailing zero word
        mxTextMark->WriteBuffer( aXclStrm );                 // NO flags
        aXclStrm    << sal_uInt16( 0 );

        mnFlags |= EXC_HLINK_MARK;
    }

    SetRecSize( 32 + mxVarData->Tell() );
}

XclExpHyperlink::~XclExpHyperlink()
{
}

String XclExpHyperlink::BuildFileName(
        sal_uInt16& rnLevel, bool& rbRel, const String& rUrl, const XclExpRoot& rRoot ) const
{
    String aDosName( INetURLObject( rUrl ).getFSysPath( INetURLObject::FSYS_DOS ) );
    rnLevel = 0;
    rbRel = rRoot.IsRelUrl();

    if( rbRel )
    {
        // try to convert to relative file name
        String aTmpName( aDosName );
        aDosName = INetURLObject::GetRelURL( rRoot.GetBasePath(), rUrl,
            INetURLObject::WAS_ENCODED, INetURLObject::DECODE_WITH_CHARSET );

        if( aDosName.SearchAscii( INET_FILE_SCHEME ) == 0 )
        {
            // not converted to rel -> back to old, return absolute flag
            aDosName = aTmpName;
            rbRel = false;
        }
        else if( aDosName.SearchAscii( "./" ) == 0 )
        {
            aDosName.Erase( 0, 2 );
        }
        else
        {
            while( aDosName.SearchAndReplaceAscii( "../", EMPTY_STRING ) == 0 )
                ++rnLevel;
        }
    }
    return aDosName;
}

void XclExpHyperlink::WriteBody( XclExpStream& rStrm )
{
    sal_uInt16 nXclCol = static_cast< sal_uInt16 >( maScPos.Col() );
    sal_uInt16 nXclRow = static_cast< sal_uInt16 >( maScPos.Row() );
    mxVarData->Seek( STREAM_SEEK_TO_BEGIN );

    rStrm   << nXclRow << nXclRow << nXclCol << nXclCol
            << XclTools::maGuidStdLink
            << sal_uInt32( 2 )
            << mnFlags;
    rStrm.CopyFromStream( *mxVarData );
}

void XclExpHyperlink::SaveXml( XclExpXmlStream& rStrm )
{
    OUString sId = rStrm.addRelation( rStrm.GetCurrentStream()->getOutputStream(),
            XclXmlUtils::ToOUString( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink" ),
            msTarget,
            XclXmlUtils::ToOUString( "External" ) );
    rStrm.GetCurrentStream()->singleElement( XML_hyperlink,
            XML_ref,                XclXmlUtils::ToOString( maScPos ).getStr(),
            FSNS( XML_r, XML_id ),  XclXmlUtils::ToOString( sId ).getStr(),
            XML_location,           mxTextMark.get() != NULL 
                                        ? XclXmlUtils::ToOString( *mxTextMark ).getStr() 
                                        : NULL,
            // OOXTODO: XML_tooltip,    from record HLinkTooltip 800h wzTooltip
            XML_display,            XclXmlUtils::ToOString( *mxRepr ).getStr(),
            FSEND );
}

// Label ranges ===============================================================

XclExpLabelranges::XclExpLabelranges( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
    SCTAB nScTab = GetCurrScTab();
    // row label ranges
    FillRangeList( maRowRanges, rRoot.GetDoc().GetRowNameRangesRef(), nScTab );
    // row labels only over 1 column (restriction of Excel97/2000/XP)
    for( ScRange* pScRange = maRowRanges.First(); pScRange; pScRange = maRowRanges.Next() )
        if( pScRange->aStart.Col() != pScRange->aEnd.Col() )
            pScRange->aEnd.SetCol( pScRange->aStart.Col() );
    // col label ranges
    FillRangeList( maColRanges, rRoot.GetDoc().GetColNameRangesRef(), nScTab );
}

void XclExpLabelranges::FillRangeList( ScRangeList& rScRanges,
        ScRangePairListRef xLabelRangesRef, SCTAB nScTab )
{
    for( const ScRangePair* pRangePair = xLabelRangesRef->First(); pRangePair; pRangePair = xLabelRangesRef->Next() )
    {
        const ScRange& rScRange = pRangePair->GetRange( 0 );
        if( rScRange.aStart.Tab() == nScTab )
            rScRanges.Append( rScRange );
    }
}

void XclExpLabelranges::Save( XclExpStream& rStrm )
{
    XclExpAddressConverter& rAddrConv = GetAddressConverter();
    XclRangeList aRowXclRanges, aColXclRanges;
    rAddrConv.ConvertRangeList( aRowXclRanges, maRowRanges, false );
    rAddrConv.ConvertRangeList( aColXclRanges, maColRanges, false );
    if( !aRowXclRanges.empty() || !aColXclRanges.empty() )
    {
        rStrm.StartRecord( EXC_ID_LABELRANGES, 4 + 8 * (aRowXclRanges.size() + aColXclRanges.size()) );
        rStrm << aRowXclRanges << aColXclRanges;
        rStrm.EndRecord();
    }
}

// Conditional formatting  ====================================================

/** Represents a CF record that contains one condition of a conditional format. */
class XclExpCFImpl : protected XclExpRoot
{
public:
    explicit            XclExpCFImpl( const XclExpRoot& rRoot, const ScCondFormatEntry& rFormatEntry );

    /** Writes the body of the CF record. */
    void                WriteBody( XclExpStream& rStrm );

private:
    const ScCondFormatEntry& mrFormatEntry; /// Calc conditional format entry.
    XclFontData         maFontData;         /// Font formatting attributes.
    XclExpCellBorder    maBorder;           /// Border formatting attributes.
    XclExpCellArea      maArea;             /// Pattern formatting attributes.
    XclTokenArrayRef    mxTokArr1;          /// Formula for first condition.
    XclTokenArrayRef    mxTokArr2;          /// Formula for second condition.
    sal_uInt32          mnFontColorId;      /// Font color ID.
    sal_uInt8           mnType;             /// Type of the condition (cell/formula).
    sal_uInt8           mnOperator;         /// Comparison operator for cell type.
    bool                mbFontUsed;         /// true = Any font attribute used.
    bool                mbHeightUsed;       /// true = Font height used.
    bool                mbWeightUsed;       /// true = Font weight used.
    bool                mbColorUsed;        /// true = Font color used.
    bool                mbUnderlUsed;       /// true = Font underline type used.
    bool                mbItalicUsed;       /// true = Font posture used.
    bool                mbStrikeUsed;       /// true = Font strikeout used.
    bool                mbBorderUsed;       /// true = Border attribute used.
    bool                mbPattUsed;         /// true = Pattern attribute used.
};

// ----------------------------------------------------------------------------

XclExpCFImpl::XclExpCFImpl( const XclExpRoot& rRoot, const ScCondFormatEntry& rFormatEntry ) :
    XclExpRoot( rRoot ),
    mrFormatEntry( rFormatEntry ),
    mnFontColorId( 0 ),
    mnType( EXC_CF_TYPE_CELL ),
    mnOperator( EXC_CF_CMP_NONE ),
    mbFontUsed( false ),
    mbHeightUsed( false ),
    mbWeightUsed( false ),
    mbColorUsed( false ),
    mbUnderlUsed( false ),
    mbItalicUsed( false ),
    mbStrikeUsed( false ),
    mbBorderUsed( false ),
    mbPattUsed( false )
{
    /*  Get formatting attributes here, and not in WriteBody(). This is needed to
        correctly insert all colors into the palette. */

    if( SfxStyleSheetBase* pStyleSheet = GetDoc().GetStyleSheetPool()->Find( mrFormatEntry.GetStyle(), SFX_STYLE_FAMILY_PARA ) )
    {
        const SfxItemSet& rItemSet = pStyleSheet->GetItemSet();

        // font
        mbHeightUsed = ScfTools::CheckItem( rItemSet, ATTR_FONT_HEIGHT,     true );
        mbWeightUsed = ScfTools::CheckItem( rItemSet, ATTR_FONT_WEIGHT,     true );
        mbColorUsed  = ScfTools::CheckItem( rItemSet, ATTR_FONT_COLOR,      true );
        mbUnderlUsed = ScfTools::CheckItem( rItemSet, ATTR_FONT_UNDERLINE,  true );
        mbItalicUsed = ScfTools::CheckItem( rItemSet, ATTR_FONT_POSTURE,    true );
        mbStrikeUsed = ScfTools::CheckItem( rItemSet, ATTR_FONT_CROSSEDOUT, true );
        mbFontUsed = mbHeightUsed || mbWeightUsed || mbColorUsed || mbUnderlUsed || mbItalicUsed || mbStrikeUsed;
        if( mbFontUsed )
        {
            Font aFont;
            ScPatternAttr::GetFont( aFont, rItemSet, SC_AUTOCOL_RAW );
            maFontData.FillFromVclFont( aFont );
            mnFontColorId = GetPalette().InsertColor( maFontData.maColor, EXC_COLOR_CELLTEXT );
        }

        // border
        mbBorderUsed = ScfTools::CheckItem( rItemSet, ATTR_BORDER, true );
        if( mbBorderUsed )
            maBorder.FillFromItemSet( rItemSet, GetPalette(), GetBiff() );

        // pattern
        mbPattUsed = ScfTools::CheckItem( rItemSet, ATTR_BACKGROUND, true );
        if( mbPattUsed )
            maArea.FillFromItemSet( rItemSet, GetPalette(), GetBiff() );
    }

    // *** mode and comparison operator ***

    bool bFmla2 = false;
    switch( rFormatEntry.GetOperation() )
    {
        case SC_COND_NONE:          mnType = EXC_CF_TYPE_NONE;                              break;
        case SC_COND_BETWEEN:       mnOperator = EXC_CF_CMP_BETWEEN;        bFmla2 = true;  break;
        case SC_COND_NOTBETWEEN:    mnOperator = EXC_CF_CMP_NOT_BETWEEN;    bFmla2 = true;  break;
        case SC_COND_EQUAL:         mnOperator = EXC_CF_CMP_EQUAL;                          break;
        case SC_COND_NOTEQUAL:      mnOperator = EXC_CF_CMP_NOT_EQUAL;                      break;
        case SC_COND_GREATER:       mnOperator = EXC_CF_CMP_GREATER;                        break;
        case SC_COND_LESS:          mnOperator = EXC_CF_CMP_LESS;                           break;
        case SC_COND_EQGREATER:     mnOperator = EXC_CF_CMP_GREATER_EQUAL;                  break;
        case SC_COND_EQLESS:        mnOperator = EXC_CF_CMP_LESS_EQUAL;                     break;
        case SC_COND_DIRECT:        mnType = EXC_CF_TYPE_FMLA;                              break;
        default:                    mnType = EXC_CF_TYPE_NONE;
            DBG_ERRORFILE( "XclExpCF::WriteBody - unknown condition type" );
    }

    // *** formulas ***

    XclExpFormulaCompiler& rFmlaComp = GetFormulaCompiler();

    ::std::auto_ptr< ScTokenArray > xScTokArr( mrFormatEntry.CreateTokenArry( 0 ) );
    mxTokArr1 = rFmlaComp.CreateFormula( EXC_FMLATYPE_CONDFMT, *xScTokArr );

    if( bFmla2 )
    {
        xScTokArr.reset( mrFormatEntry.CreateTokenArry( 1 ) );
        mxTokArr2 = rFmlaComp.CreateFormula( EXC_FMLATYPE_CONDFMT, *xScTokArr );
    }
}

void XclExpCFImpl::WriteBody( XclExpStream& rStrm )
{
    // *** mode and comparison operator ***

    rStrm << mnType << mnOperator;

    // *** formula sizes ***

    sal_uInt16 nFmlaSize1 = mxTokArr1.get() ? mxTokArr1->GetSize() : 0;
    sal_uInt16 nFmlaSize2 = mxTokArr2.get() ? mxTokArr2->GetSize() : 0;
    rStrm << nFmlaSize1 << nFmlaSize2;

    // *** formatting blocks ***

    if( mbFontUsed || mbBorderUsed || mbPattUsed )
    {
        sal_uInt32 nFlags = EXC_CF_ALLDEFAULT;

        ::set_flag( nFlags, EXC_CF_BLOCK_FONT,   mbFontUsed );
        ::set_flag( nFlags, EXC_CF_BLOCK_BORDER, mbBorderUsed );
        ::set_flag( nFlags, EXC_CF_BLOCK_AREA,   mbPattUsed );

        // attributes used -> set flags to 0.
        ::set_flag( nFlags, EXC_CF_BORDER_ALL, !mbBorderUsed );
        ::set_flag( nFlags, EXC_CF_AREA_ALL,   !mbPattUsed );

        rStrm << nFlags << sal_uInt16( 0 );

        if( mbFontUsed )
        {
            // font height, 0xFFFFFFFF indicates unused
            sal_uInt32 nHeight = mbHeightUsed ? maFontData.mnHeight : 0xFFFFFFFF;
            // font style: italic and strikeout
            sal_uInt32 nStyle = 0;
            ::set_flag( nStyle, EXC_CF_FONT_STYLE,     maFontData.mbItalic );
            ::set_flag( nStyle, EXC_CF_FONT_STRIKEOUT, maFontData.mbStrikeout );
            // font color, 0xFFFFFFFF indicates unused
            sal_uInt32 nColor = mbColorUsed ? GetPalette().GetColorIndex( mnFontColorId ) : 0xFFFFFFFF;
            // font used flags for italic, weight, and strikeout -> 0 = used, 1 = default
            sal_uInt32 nFontFlags1 = EXC_CF_FONT_ALLDEFAULT;
            ::set_flag( nFontFlags1, EXC_CF_FONT_STYLE, !(mbItalicUsed || mbWeightUsed) );
            ::set_flag( nFontFlags1, EXC_CF_FONT_STRIKEOUT, !mbStrikeUsed );
            // font used flag for underline -> 0 = used, 1 = default
            sal_uInt32 nFontFlags3 = mbUnderlUsed ? 0 : EXC_CF_FONT_UNDERL;

            rStrm.WriteZeroBytesToRecord( 64 );
            rStrm   << nHeight
                    << nStyle
                    << maFontData.mnWeight
                    << EXC_FONTESC_NONE
                    << maFontData.mnUnderline;
            rStrm.WriteZeroBytesToRecord( 3 );
            rStrm   << nColor
                    << sal_uInt32( 0 )
                    << nFontFlags1
                    << EXC_CF_FONT_ESCAPEM      // escapement never used -> set the flag
                    << nFontFlags3;
            rStrm.WriteZeroBytesToRecord( 16 );
            rStrm   << sal_uInt16( 1 );         // must be 1
        }

        if( mbBorderUsed )
        {
            sal_uInt16 nLineStyle = 0;
            sal_uInt32 nLineColor = 0;
            maBorder.SetFinalColors( GetPalette() );
            maBorder.FillToCF8( nLineStyle, nLineColor );
            rStrm << nLineStyle << nLineColor << sal_uInt16( 0 );
        }

        if( mbPattUsed )
        {
            sal_uInt16 nPattern = 0, nColor = 0;
            maArea.SetFinalColors( GetPalette() );
            maArea.FillToCF8( nPattern, nColor );
            rStrm << nPattern << nColor;
        }
    }
    else
    {
        // no data blocks at all
        rStrm << sal_uInt32( 0 ) << sal_uInt16( 0 );
    }

    // *** formulas ***

    if( mxTokArr1.get() )
        mxTokArr1->WriteArray( rStrm );
    if( mxTokArr2.get() )
        mxTokArr2->WriteArray( rStrm );
}

// ----------------------------------------------------------------------------

XclExpCF::XclExpCF( const XclExpRoot& rRoot, const ScCondFormatEntry& rFormatEntry ) :
    XclExpRecord( EXC_ID_CF ),
    XclExpRoot( rRoot ),
    mxImpl( new XclExpCFImpl( rRoot, rFormatEntry ) )
{
}

XclExpCF::~XclExpCF()
{
}

void XclExpCF::WriteBody( XclExpStream& rStrm )
{
    mxImpl->WriteBody( rStrm );
}

// ----------------------------------------------------------------------------

XclExpCondfmt::XclExpCondfmt( const XclExpRoot& rRoot, const ScConditionalFormat& rCondFormat ) :
    XclExpRecord( EXC_ID_CONDFMT ),
    XclExpRoot( rRoot )
{
    ScRangeList aScRanges;
    GetDoc().FindConditionalFormat( rCondFormat.GetKey(), aScRanges, GetCurrScTab() );
    GetAddressConverter().ConvertRangeList( maXclRanges, aScRanges, true );
    if( !maXclRanges.empty() )
    {
        for( sal_uInt16 nIndex = 0, nCount = rCondFormat.Count(); nIndex < nCount; ++nIndex )
            if( const ScCondFormatEntry* pEntry = rCondFormat.GetEntry( nIndex ) )
                maCFList.AppendNewRecord( new XclExpCF( GetRoot(), *pEntry ) );
        aScRanges.Format( msSeqRef, SCA_VALID, NULL, formula::FormulaGrammar::CONV_XL_A1 );
    }
}

XclExpCondfmt::~XclExpCondfmt()
{
}

bool XclExpCondfmt::IsValid() const
{
    return !maCFList.IsEmpty() && !maXclRanges.empty();
}

void XclExpCondfmt::Save( XclExpStream& rStrm )
{
    if( IsValid() )
    {
        XclExpRecord::Save( rStrm );
        maCFList.Save( rStrm );
    }
}

void XclExpCondfmt::WriteBody( XclExpStream& rStrm )
{
    DBG_ASSERT( !maCFList.IsEmpty(), "XclExpCondfmt::WriteBody - no CF records to write" );
    DBG_ASSERT( !maXclRanges.empty(), "XclExpCondfmt::WriteBody - no cell ranges found" );

    rStrm   << static_cast< sal_uInt16 >( maCFList.GetSize() )
            << sal_uInt16( 1 )
            << maXclRanges.GetEnclosingRange()
            << maXclRanges;
}

void XclExpCondfmt::SaveXml( XclExpXmlStream& rStrm )
{
    if( !IsValid() )
        return;
    
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_conditionalFormatting,
            XML_sqref, XclXmlUtils::ToOString( msSeqRef ).getStr(),
            // OOXTODO: XML_pivot,
            FSEND );
    maCFList.SaveXml( rStrm );
    // OOXTODO: XML_extLst
    rWorksheet->endElement( XML_conditionalFormatting );
}

// ----------------------------------------------------------------------------

XclExpCondFormatBuffer::XclExpCondFormatBuffer( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
    if( const ScConditionalFormatList* pCondFmtList = GetDoc().GetCondFormList() )
    {
        if( const ScConditionalFormatPtr* ppCondFmt = pCondFmtList->GetData() )
        {
            const ScConditionalFormatPtr* ppCondEnd = ppCondFmt + pCondFmtList->Count();
            for( ; ppCondFmt < ppCondEnd; ++ppCondFmt )
            {
                if( *ppCondFmt )
                {
                    XclExpCondfmtList::RecordRefType xCondfmtRec( new XclExpCondfmt( GetRoot(), **ppCondFmt ) );
                    if( xCondfmtRec->IsValid() )
                        maCondfmtList.AppendRecord( xCondfmtRec );
                }
            }
        }
    }
}

void XclExpCondFormatBuffer::Save( XclExpStream& rStrm )
{
    maCondfmtList.Save( rStrm );
}

void XclExpCondFormatBuffer::SaveXml( XclExpXmlStream& rStrm )
{
    maCondfmtList.SaveXml( rStrm );
}

// Validation =================================================================

namespace {

/** Writes a formula for the DV record. */
void lclWriteDvFormula( XclExpStream& rStrm, const XclTokenArray* pXclTokArr )
{
    sal_uInt16 nFmlaSize = pXclTokArr ? pXclTokArr->GetSize() : 0;
    rStrm << nFmlaSize << sal_uInt16( 0 );
    if( pXclTokArr )
        pXclTokArr->WriteArray( rStrm );
}

/** Writes a formula for the DV record, based on a single string. */
void lclWriteDvFormula( XclExpStream& rStrm, const XclExpString& rString )
{
    // fake a formula with a single tStr token
    rStrm   << static_cast< sal_uInt16 >( rString.GetSize() + 1 )
            << sal_uInt16( 0 )
            << EXC_TOKID_STR
            << rString;
}

const char* lcl_GetValidationType( sal_uInt32 nFlags )
{
    switch( nFlags & EXC_DV_MODE_MASK )
    {
        case EXC_DV_MODE_ANY:       return "none";
        case EXC_DV_MODE_WHOLE:     return "whole";
        case EXC_DV_MODE_DECIMAL:   return "decimal";
        case EXC_DV_MODE_LIST:      return "list";
        case EXC_DV_MODE_DATE:      return "date";
        case EXC_DV_MODE_TIME:      return "time";
        case EXC_DV_MODE_TEXTLEN:   return "textLength";
        case EXC_DV_MODE_CUSTOM:    return "custom";
    }
    return NULL;
}

const char* lcl_GetOperatorType( sal_uInt32 nFlags )
{
    switch( nFlags & EXC_DV_COND_MASK )
    {
        case EXC_DV_COND_BETWEEN:       return "between";
        case EXC_DV_COND_NOTBETWEEN:    return "notBetween";
        case EXC_DV_COND_EQUAL:         return "equal";
        case EXC_DV_COND_NOTEQUAL:      return "notEqual";
        case EXC_DV_COND_GREATER:       return "greaterThan";
        case EXC_DV_COND_LESS:          return "lessThan";
        case EXC_DV_COND_EQGREATER:     return "greaterThanOrEqual";
        case EXC_DV_COND_EQLESS:        return "lessThanOrEqual";
    }
    return NULL;
}

} // namespace

// ----------------------------------------------------------------------------

XclExpDV::XclExpDV( const XclExpRoot& rRoot, sal_uLong nScHandle ) :
    XclExpRecord( EXC_ID_DV ),
    XclExpRoot( rRoot ),
    mnFlags( 0 ),
    mnScHandle( nScHandle )
{
    if( const ScValidationData* pValData = GetDoc().GetValidationEntry( mnScHandle ) )
    {
        // prompt box - empty string represented by single NUL character
        String aTitle, aText;
        bool bShowPrompt = (pValData->GetInput( aTitle, aText ) == sal_True);
        if( aTitle.Len() )
            maPromptTitle.Assign( aTitle );
        else
            maPromptTitle.Assign( '\0' );
        if( aText.Len() )
            maPromptText.Assign( aText );
        else
            maPromptText.Assign( '\0' );

        // error box - empty string represented by single NUL character
        ScValidErrorStyle eScErrorStyle;
        bool bShowError = (pValData->GetErrMsg( aTitle, aText, eScErrorStyle ) == sal_True);
        if( aTitle.Len() )
            maErrorTitle.Assign( aTitle );
        else
            maErrorTitle.Assign( '\0' );
        if( aText.Len() )
            maErrorText.Assign( aText );
        else
            maErrorText.Assign( '\0' );

        // flags
        switch( pValData->GetDataMode() )
        {
            case SC_VALID_ANY:      mnFlags |= EXC_DV_MODE_ANY;         break;
            case SC_VALID_WHOLE:    mnFlags |= EXC_DV_MODE_WHOLE;       break;
            case SC_VALID_DECIMAL:  mnFlags |= EXC_DV_MODE_DECIMAL;     break;
            case SC_VALID_LIST:     mnFlags |= EXC_DV_MODE_LIST;        break;
            case SC_VALID_DATE:     mnFlags |= EXC_DV_MODE_DATE;        break;
            case SC_VALID_TIME:     mnFlags |= EXC_DV_MODE_TIME;        break;
            case SC_VALID_TEXTLEN:  mnFlags |= EXC_DV_MODE_TEXTLEN;     break;
            case SC_VALID_CUSTOM:   mnFlags |= EXC_DV_MODE_CUSTOM;      break;
            default:                DBG_ERRORFILE( "XclExpDV::XclExpDV - unknown mode" );
        }

        switch( pValData->GetOperation() )
        {
            case SC_COND_NONE:
            case SC_COND_EQUAL:         mnFlags |= EXC_DV_COND_EQUAL;       break;
            case SC_COND_LESS:          mnFlags |= EXC_DV_COND_LESS;        break;
            case SC_COND_GREATER:       mnFlags |= EXC_DV_COND_GREATER;     break;
            case SC_COND_EQLESS:        mnFlags |= EXC_DV_COND_EQLESS;      break;
            case SC_COND_EQGREATER:     mnFlags |= EXC_DV_COND_EQGREATER;   break;
            case SC_COND_NOTEQUAL:      mnFlags |= EXC_DV_COND_NOTEQUAL;    break;
            case SC_COND_BETWEEN:       mnFlags |= EXC_DV_COND_BETWEEN;     break;
            case SC_COND_NOTBETWEEN:    mnFlags |= EXC_DV_COND_NOTBETWEEN;  break;
            default:                    DBG_ERRORFILE( "XclExpDV::XclExpDV - unknown condition" );
        }
        switch( eScErrorStyle )
        {
            case SC_VALERR_STOP:        mnFlags |= EXC_DV_ERROR_STOP;       break;
            case SC_VALERR_WARNING:     mnFlags |= EXC_DV_ERROR_WARNING;    break;
            case SC_VALERR_INFO:        mnFlags |= EXC_DV_ERROR_INFO;       break;
            case SC_VALERR_MACRO:
                // #111781# set INFO for validity with macro call, delete title
                mnFlags |= EXC_DV_ERROR_INFO;
                maErrorTitle.Assign( '\0' );    // contains macro name
            break;
            default:                    DBG_ERRORFILE( "XclExpDV::XclExpDV - unknown error style" );
        }
        ::set_flag( mnFlags, EXC_DV_IGNOREBLANK, pValData->IsIgnoreBlank() );
        ::set_flag( mnFlags, EXC_DV_SUPPRESSDROPDOWN, pValData->GetListType() == ValidListType::INVISIBLE );
        ::set_flag( mnFlags, EXC_DV_SHOWPROMPT, bShowPrompt );
        ::set_flag( mnFlags, EXC_DV_SHOWERROR, bShowError );

        // formulas
        XclExpFormulaCompiler& rFmlaComp = GetFormulaCompiler();
        ::std::auto_ptr< ScTokenArray > xScTokArr;

        // first formula
        xScTokArr.reset( pValData->CreateTokenArry( 0 ) );
        if( xScTokArr.get() )
        {
            if( pValData->GetDataMode() == SC_VALID_LIST )
            {
                String aString;
                if( XclTokenArrayHelper::GetStringList( aString, *xScTokArr, '\n' ) )
                {
                    OUStringBuffer sFormulaBuf;
                    sFormulaBuf.append( (sal_Unicode) '"' );
                    /*  Formula is a list of string tokens -> build the Excel string.
                        Data validity is BIFF8 only (important for the XclExpString object).
                        Excel uses the NUL character as string list separator. */
                    mxString1.reset( new XclExpString( EXC_STR_8BITLENGTH ) );
                    xub_StrLen nTokenCnt = aString.GetTokenCount( '\n' );
                    xub_StrLen nStringIx = 0;
                    for( xub_StrLen nToken = 0; nToken < nTokenCnt; ++nToken )
                    {
                        String aToken( aString.GetToken( 0, '\n', nStringIx ) );
                        if( nToken > 0 )
                        {
                            mxString1->Append( '\0' );
                            sFormulaBuf.append( (sal_Unicode) ',' );
                        }
                        mxString1->Append( aToken );
                        sFormulaBuf.append( XclXmlUtils::ToOUString( aToken ) );
                    }
                    ::set_flag( mnFlags, EXC_DV_STRINGLIST );

                    sFormulaBuf.append( (sal_Unicode) '"' );
                    msFormula1 = sFormulaBuf.makeStringAndClear();
                }
                else
                {
                    /*  All other formulas in validation are stored like conditional
                        formatting formulas (with tRefN/tAreaN tokens as value or
                        array class). But NOT the cell references and defined names
                        in list validation - they are stored as reference class
                        tokens... Example:
                        1) Cell must be equal to A1 -> formula is =A1 -> writes tRefNV token
                        2) List is taken from A1    -> formula is =A1 -> writes tRefNR token
                        Formula compiler supports this by offering two different functions
                        CreateDataValFormula() and CreateListValFormula(). */
                    mxTokArr1 = rFmlaComp.CreateFormula( EXC_FMLATYPE_LISTVAL, *xScTokArr );
                    msFormula1 = XclXmlUtils::ToOUString( GetDoc(), pValData->GetSrcPos(), xScTokArr.get() );
                }
            }
            else
            {
                // no list validation -> convert the formula
                mxTokArr1 = rFmlaComp.CreateFormula( EXC_FMLATYPE_DATAVAL, *xScTokArr );
                msFormula1 = XclXmlUtils::ToOUString( GetDoc(), pValData->GetSrcPos(), xScTokArr.get() );
            }
        }

        // second formula
        xScTokArr.reset( pValData->CreateTokenArry( 1 ) );
        if( xScTokArr.get() )
        {
            mxTokArr2 = rFmlaComp.CreateFormula( EXC_FMLATYPE_DATAVAL, *xScTokArr );
            msFormula2 = XclXmlUtils::ToOUString( GetDoc(), pValData->GetSrcPos(), xScTokArr.get() );
        }
    }
    else
    {
        DBG_ERRORFILE( "XclExpDV::XclExpDV - missing core data" );
        mnScHandle = ULONG_MAX;
    }
}

XclExpDV::~XclExpDV()
{
}

void XclExpDV::InsertCellRange( const ScRange& rRange )
{
    maScRanges.Join( rRange );
}

bool XclExpDV::Finalize()
{
    GetAddressConverter().ConvertRangeList( maXclRanges, maScRanges, true );
    return (mnScHandle != ULONG_MAX) && !maXclRanges.empty();
}

void XclExpDV::WriteBody( XclExpStream& rStrm )
{
    // flags and strings
    rStrm << mnFlags << maPromptTitle << maErrorTitle << maPromptText << maErrorText;
    // condition formulas
    if( mxString1.get() )
        lclWriteDvFormula( rStrm, *mxString1 );
    else
        lclWriteDvFormula( rStrm, mxTokArr1.get() );
    lclWriteDvFormula( rStrm, mxTokArr2.get() );
    // cell ranges
    rStrm << maXclRanges;
}

void XclExpDV::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_dataValidation,
            XML_allowBlank,         XclXmlUtils::ToPsz( ::get_flag( mnFlags, EXC_DV_IGNOREBLANK ) ),
            XML_error,              XESTRING_TO_PSZ( maErrorText ),
            // OOXTODO: XML_errorStyle, 
            XML_errorTitle,         XESTRING_TO_PSZ( maErrorTitle ),
            // OOXTODO: XML_imeMode,
            XML_operator,           lcl_GetOperatorType( mnFlags ),
            XML_prompt,             XESTRING_TO_PSZ( maPromptText ),
            XML_promptTitle,        XESTRING_TO_PSZ( maPromptTitle ),
            XML_showDropDown,       XclXmlUtils::ToPsz( ! ::get_flag( mnFlags, EXC_DV_SUPPRESSDROPDOWN ) ),
            XML_showErrorMessage,   XclXmlUtils::ToPsz( ::get_flag( mnFlags, EXC_DV_SHOWERROR ) ),
            XML_showInputMessage,   XclXmlUtils::ToPsz( ::get_flag( mnFlags, EXC_DV_SHOWPROMPT ) ),
            XML_sqref,              XclXmlUtils::ToOString( maScRanges ).getStr(),
            XML_type,               lcl_GetValidationType( mnFlags ),
            FSEND );
    if( msFormula1.getLength() )
    {
        rWorksheet->startElement( XML_formula1, FSEND );
        rWorksheet->writeEscaped( msFormula1 );
        rWorksheet->endElement( XML_formula1 );
    }
    if( msFormula2.getLength() )
    {
        rWorksheet->startElement( XML_formula2, FSEND );
        rWorksheet->writeEscaped( msFormula2 );
        rWorksheet->endElement( XML_formula2 );
    }
    rWorksheet->endElement( XML_dataValidation );
}

// ----------------------------------------------------------------------------

XclExpDval::XclExpDval( const XclExpRoot& rRoot ) :
    XclExpRecord( EXC_ID_DVAL, 18 ),
    XclExpRoot( rRoot )
{
}

XclExpDval::~XclExpDval()
{
}

void XclExpDval::InsertCellRange( const ScRange& rRange, sal_uLong nScHandle )
{
    if( GetBiff() == EXC_BIFF8 )
    {
        XclExpDV& rDVRec = SearchOrCreateDv( nScHandle );
        rDVRec.InsertCellRange( rRange );
    }
}

void XclExpDval::Save( XclExpStream& rStrm )
{
    // check all records
    size_t nPos = maDVList.GetSize();
    while( nPos )
    {
        --nPos;     // backwards to keep nPos valid
        XclExpDVRef xDVRec = maDVList.GetRecord( nPos );
        if( !xDVRec->Finalize() )
            maDVList.RemoveRecord( nPos );
    }

    // write the DVAL and the DV's
    if( !maDVList.IsEmpty() )
    {
        XclExpRecord::Save( rStrm );
        maDVList.Save( rStrm );
    }
}

void XclExpDval::SaveXml( XclExpXmlStream& rStrm )
{
    if( maDVList.IsEmpty() )
        return;

    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_dataValidations, 
            XML_count, OString::valueOf( (sal_Int32) maDVList.GetSize() ).getStr(),
            // OOXTODO: XML_disablePrompts,
            // OOXTODO: XML_xWindow,
            // OOXTODO: XML_yWindow,
            FSEND );
    maDVList.SaveXml( rStrm );
    rWorksheet->endElement( XML_dataValidations );
}

XclExpDV& XclExpDval::SearchOrCreateDv( sal_uLong nScHandle )
{
    // test last found record
    if( mxLastFoundDV.get() && (mxLastFoundDV->GetScHandle() == nScHandle) )
        return *mxLastFoundDV;

    // binary search
    size_t nCurrPos = 0;
    if( !maDVList.IsEmpty() )
    {
        size_t nFirstPos = 0;
        size_t nLastPos = maDVList.GetSize() - 1;
        bool bLoop = true;
        sal_uLong nCurrScHandle = ::std::numeric_limits< sal_uLong >::max();
        while( (nFirstPos <= nLastPos) && bLoop )
        {
            nCurrPos = (nFirstPos + nLastPos) / 2;
            mxLastFoundDV = maDVList.GetRecord( nCurrPos );
            nCurrScHandle = mxLastFoundDV->GetScHandle();
            if( nCurrScHandle == nScHandle )
                bLoop = false;
            else if( nCurrScHandle < nScHandle )
                nFirstPos = nCurrPos + 1;
            else if( nCurrPos )
                nLastPos = nCurrPos - 1;
            else    // special case for nLastPos = -1
                bLoop = false;
        }
        if( nCurrScHandle == nScHandle )
            return *mxLastFoundDV;
        else if( nCurrScHandle < nScHandle )
            ++nCurrPos;
    }

    // create new DV record
    mxLastFoundDV.reset( new XclExpDV( *this, nScHandle ) );
    maDVList.InsertRecord( mxLastFoundDV, nCurrPos );
    return *mxLastFoundDV;
}

void XclExpDval::WriteBody( XclExpStream& rStrm )
{
    rStrm.WriteZeroBytes( 10 );
    rStrm << EXC_DVAL_NOOBJ << static_cast< sal_uInt32 >( maDVList.GetSize() );
}

// Web Queries ================================================================

XclExpWebQuery::XclExpWebQuery(
		const String& rRangeName,
        const String& rUrl,
		const String& rSource,
		sal_Int32 nRefrSecs ) :
    maDestRange( rRangeName ),
    maUrl( rUrl ),
    // refresh delay time: seconds -> minutes
    mnRefresh( ulimit_cast< sal_Int16 >( (nRefrSecs + 59L) / 60L ) ),
    mbEntireDoc( false )
{
	// comma separated list of HTML table names or indexes
	xub_StrLen nTokenCnt = rSource.GetTokenCount( ';' );
	String aNewTables, aAppendTable;
	xub_StrLen nStringIx = 0;
    bool bExitLoop = false;
    for( xub_StrLen nToken = 0; (nToken < nTokenCnt) && !bExitLoop; ++nToken )
	{
		String aToken( rSource.GetToken( 0, ';', nStringIx ) );
        mbEntireDoc = ScfTools::IsHTMLDocName( aToken );
        bExitLoop = mbEntireDoc || ScfTools::IsHTMLTablesName( aToken );
        if( !bExitLoop && ScfTools::GetHTMLNameFromName( aToken, aAppendTable ) )
            ScGlobal::AddToken( aNewTables, aAppendTable, ',' );
	}

	if( !bExitLoop )	// neither HTML_all nor HTML_tables found
	{
		if( aNewTables.Len() )
            mxQryTables.reset( new XclExpString( aNewTables ) );
		else
            mbEntireDoc = true;
	}
}

XclExpWebQuery::~XclExpWebQuery()
{
}

void XclExpWebQuery::Save( XclExpStream& rStrm )
{
    DBG_ASSERT( !mbEntireDoc || !mxQryTables.get(), "XclExpWebQuery::Save - illegal mode" );
    sal_uInt16 nFlags;

	// QSI record
    rStrm.StartRecord( EXC_ID_QSI, 10 + maDestRange.GetSize() );
    rStrm   << EXC_QSI_DEFAULTFLAGS
            << sal_uInt16( 0x0010 )
            << sal_uInt16( 0x0012 )
            << sal_uInt32( 0x00000000 )
            << maDestRange;
	rStrm.EndRecord();

	// PARAMQRY record
    nFlags = 0;
    ::insert_value( nFlags, EXC_PQRYTYPE_WEBQUERY, 0, 3 );
    ::set_flag( nFlags, EXC_PQRY_WEBQUERY );
    ::set_flag( nFlags, EXC_PQRY_TABLES, !mbEntireDoc );
    rStrm.StartRecord( EXC_ID_PQRY, 12 );
    rStrm   << nFlags
            << sal_uInt16( 0x0000 )
            << sal_uInt16( 0x0001 );
    rStrm.WriteZeroBytes( 6 );
	rStrm.EndRecord();

    // WQSTRING record
    rStrm.StartRecord( EXC_ID_WQSTRING, maUrl.GetSize() );
    rStrm << maUrl;
	rStrm.EndRecord();

	// unknown record 0x0802
    rStrm.StartRecord( EXC_ID_0802, 16 + maDestRange.GetSize() );
    rStrm   << EXC_ID_0802;             // repeated record id ?!?
    rStrm.WriteZeroBytes( 6 );
    rStrm   << sal_uInt16( 0x0003 )
            << sal_uInt32( 0x00000000 )
            << sal_uInt16( 0x0010 )
            << maDestRange;
	rStrm.EndRecord();

	// WEBQRYSETTINGS record
    nFlags = mxQryTables.get() ? EXC_WQSETT_SPECTABLES : EXC_WQSETT_ALL;
    rStrm.StartRecord( EXC_ID_WQSETT, 28 );
    rStrm   << EXC_ID_WQSETT            // repeated record id ?!?
            << sal_uInt16( 0x0000 )
            << sal_uInt16( 0x0004 )
            << sal_uInt16( 0x0000 )
            << EXC_WQSETT_DEFAULTFLAGS
            << nFlags;
    rStrm.WriteZeroBytes( 10 );
    rStrm   << mnRefresh                // refresh delay in minutes
            << EXC_WQSETT_FORMATFULL
            << sal_uInt16( 0x0000 );
	rStrm.EndRecord();

	// WEBQRYTABLES record
    if( mxQryTables.get() )
	{
        rStrm.StartRecord( EXC_ID_WQTABLES, 4 + mxQryTables->GetSize() );
        rStrm   << EXC_ID_WQTABLES          // repeated record id ?!?
                << sal_uInt16( 0x0000 )
                << *mxQryTables;            // comma separated list of source tables
		rStrm.EndRecord();
	}
}

// ----------------------------------------------------------------------------

XclExpWebQueryBuffer::XclExpWebQueryBuffer( const XclExpRoot& rRoot )
{
    SCTAB nScTab = rRoot.GetCurrScTab();
    SfxObjectShell* pShell = rRoot.GetDocShell();
    if( !pShell ) return;
    ScfPropertySet aModelProp( pShell->GetModel() );
    if( !aModelProp.Is() ) return;

    Reference< XAreaLinks > xAreaLinks;
    aModelProp.GetProperty( xAreaLinks, CREATE_OUSTRING( SC_UNO_AREALINKS ) );
    Reference< XIndexAccess > xLinksIA( xAreaLinks, UNO_QUERY );
    if( !xLinksIA.is() ) return;

    for( sal_Int32 nIndex = 0, nCount = xLinksIA->getCount(); nIndex < nCount; ++nIndex )
	{
        Reference< XAreaLink > xAreaLink( xLinksIA->getByIndex( nIndex ), UNO_QUERY );
        if( xAreaLink.is() )
		{
            CellRangeAddress aDestRange( xAreaLink->getDestArea() );
            if( static_cast< SCTAB >( aDestRange.Sheet ) == nScTab )
			{
                ScfPropertySet aLinkProp( xAreaLink );
                OUString aFilter;
                if( aLinkProp.GetProperty( aFilter, CREATE_OUSTRING( SC_UNONAME_FILTER ) ) &&
                    (aFilter == CREATE_OUSTRING( EXC_WEBQRY_FILTER )) )
                {
                    // get properties
                    OUString /*aFilterOpt,*/ aUrl;
                    sal_Int32 nRefresh = 0;

//                  aLinkProp.GetProperty( aFilterOpt, CREATE_OUSTRING( SC_UNONAME_FILTOPT ) );
                    aLinkProp.GetProperty( aUrl, CREATE_OUSTRING( SC_UNONAME_LINKURL ) );
                    aLinkProp.GetProperty( nRefresh, CREATE_OUSTRING( SC_UNONAME_REFDELAY ) );

                    String aAbsDoc( ScGlobal::GetAbsDocName( aUrl, pShell ) );
                    INetURLObject aUrlObj( aAbsDoc );
                    String aWebQueryUrl( aUrlObj.getFSysPath( INetURLObject::FSYS_DOS ) );
                    if( !aWebQueryUrl.Len() )
                        aWebQueryUrl = aAbsDoc;

                    // find range or create a new range
                    String aRangeName;
                    ScRange aScDestRange;
                    ScUnoConversion::FillScRange( aScDestRange, aDestRange );
                    if( const ScRangeData* pRangeData = rRoot.GetNamedRanges().GetRangeAtBlock( aScDestRange ) )
                    {
                        aRangeName = pRangeData->GetName();
                    }
                    else
                    {
                        XclExpFormulaCompiler& rFmlaComp = rRoot.GetFormulaCompiler();
                        XclExpNameManager& rNameMgr = rRoot.GetNameManager();

                        // create a new unique defined name containing the range
                        XclTokenArrayRef xTokArr = rFmlaComp.CreateFormula( EXC_FMLATYPE_WQUERY, aScDestRange );
                        sal_uInt16 nNameIdx = rNameMgr.InsertUniqueName( aUrlObj.getBase(), xTokArr, nScTab );
                        aRangeName = rNameMgr.GetOrigName( nNameIdx );
                    }

                    // create and store the web query record
                    if( aRangeName.Len() )
                        AppendNewRecord( new XclExpWebQuery(
                            aRangeName, aWebQueryUrl, xAreaLink->getSourceArea(), nRefresh ) );
				}
			}
		}
	}
}

// ============================================================================

