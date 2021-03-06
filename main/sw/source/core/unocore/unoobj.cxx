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
#include "precompiled_sw.hxx"

#include <com/sun/star/table/TableSortField.hpp>

#include <osl/endian.h>
#include <rtl/ustrbuf.hxx>
#include <unotools/collatorwrapper.hxx>
#include <swtypes.hxx>
#include <hintids.hxx>
#include <cmdid.h>
#include <hints.hxx>
#include <IMark.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <istyleaccess.hxx>
#include <ndtxt.hxx>
#include <ndnotxt.hxx>
#include <unocrsr.hxx>
#include <unocrsrhelper.hxx>
#include <swundo.hxx>
#include <rootfrm.hxx>
#include <flyfrm.hxx>
#include <ftnidx.hxx>
#include <sfx2/linkmgr.hxx>
#include <docary.hxx>
#include <paratr.hxx>
#include <tools/urlobj.hxx>
#include <pam.hxx>
#include <tools/cachestr.hxx>
#include <shellio.hxx>
#include <swerror.h>
#include <swtblfmt.hxx>
#include <fmtruby.hxx>
#include <docsh.hxx>
#include <docstyle.hxx>
#include <charfmt.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <fmtpdsc.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hrc>
#include <poolfmt.hxx>
#include <edimp.hxx>
#include <fchrfmt.hxx>
#include <fmtautofmt.hxx>
#include <cntfrm.hxx>
#include <pagefrm.hxx>
#include <doctxm.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <fmtrfmrk.hxx>
#include <txtrfmrk.hxx>
#include <unotextrange.hxx>
#include <unotextcursor.hxx>
#include <unomap.hxx>
#include <unosett.hxx>
#include <unoprnms.hxx>
#include <unotbl.hxx>
#include <unodraw.hxx>
#include <unocoll.hxx>
#include <unostyle.hxx>
#include <unofield.hxx>
#include <unometa.hxx>
#include <fmtanchr.hxx>
#include <editeng/flstitem.hxx>
#include <svtools/ctrltool.hxx>
#include <flypos.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <unoidx.hxx>
#include <unoframe.hxx>
#include <fmthdft.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <fmtflcnt.hxx>
#define _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTSSORT
#include <svl/svstdarr.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/unolingu.hxx>
#include <fmtclds.hxx>
#include <dcontact.hxx>
#include <SwStyleNameMapper.hxx>
#include <crsskip.hxx>
#include <sortopt.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <memory>
#include <unoparaframeenum.hxx>
#include <unoparagraph.hxx>


using namespace ::com::sun::star;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;


/****************************************************************************
	static methods
****************************************************************************/
uno::Sequence< sal_Int8 >  CreateUnoTunnelId()
{
	static osl::Mutex aCreateMutex;
	osl::Guard<osl::Mutex> aGuard( aCreateMutex );
	uno::Sequence< sal_Int8 > aSeq( 16 );
    rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0,	sal_True );
	return aSeq;
}
/****************************************************************************
	Hilfsklassen
****************************************************************************/

/* -----------------13.05.98 12:15-------------------
 *
 * --------------------------------------------------*/
SwUnoInternalPaM::SwUnoInternalPaM(SwDoc& rDoc) :
	SwPaM(rDoc.GetNodes())
{
}
SwUnoInternalPaM::~SwUnoInternalPaM()
{
	while( GetNext() != this)
	{
		delete GetNext();
	}
}

SwUnoInternalPaM&	SwUnoInternalPaM::operator=(const SwPaM& rPaM)
{
	const SwPaM* pTmp = &rPaM;
	*GetPoint() = *rPaM.GetPoint();
	if(rPaM.HasMark())
	{
		SetMark();
		*GetMark() = *rPaM.GetMark();
	}
	else
		DeleteMark();
	while(&rPaM != (pTmp = (const SwPaM*)pTmp->GetNext()))
	{
		if(pTmp->HasMark())
			new SwPaM(*pTmp->GetMark(), *pTmp->GetPoint(), this);
		else
			new SwPaM(*pTmp->GetPoint(), this);
	}
	return *this;
}

/*-----------------09.03.98 08:29-------------------

--------------------------------------------------*/
void SwUnoCursorHelper::SelectPam(SwPaM & rPam, const bool bExpand)
{
    if (bExpand)
    {
        if (!rPam.HasMark())
        {
            rPam.SetMark();
        }
    }
    else if (rPam.HasMark())
    {
        rPam.DeleteMark();
    }
}

/* -----------------20.05.98 14:59-------------------
 *
 * --------------------------------------------------*/
void SwUnoCursorHelper::GetTextFromPam(SwPaM & rPam, OUString & rBuffer)
{
    if (!rPam.HasMark())
    {
		return;
    }
	SvCacheStream aStream( 20480 );
#ifdef OSL_BIGENDIAN
    aStream.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
#else
    aStream.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
#endif
    WriterRef xWrt;
    // TODO/MBA: looks like a BaseURL doesn't make sense here
    SwReaderWriter::GetWriter( C2S(FILTER_TEXT_DLG), String(), xWrt );
	if( xWrt.Is() )
	{
        SwWriter aWriter( aStream, rPam );
		xWrt->bASCII_NoLastLineEnd = sal_True;
		xWrt->bExportPargraphNumbering = sal_False;
		SwAsciiOptions aOpt = xWrt->GetAsciiOptions();
		aOpt.SetCharSet( RTL_TEXTENCODING_UNICODE );
		xWrt->SetAsciiOptions( aOpt );
		xWrt->bUCS2_WithStartChar = sal_False;
        // --> FME #i68522#
        const sal_Bool bOldShowProgress = xWrt->bShowProgress;
        xWrt->bShowProgress = sal_False;
        // <--

		long lLen;
		if( !IsError( aWriter.Write( xWrt ) ) &&
			0x7ffffff > (( lLen  = aStream.GetSize() )
									/ sizeof( sal_Unicode )) + 1 )
		{
			aStream << (sal_Unicode)'\0';

            long lUniLen = (lLen / sizeof( sal_Unicode ));
			::rtl::OUStringBuffer aStrBuffer( lUniLen );
			aStream.Seek( 0 );
			aStream.ResetError();
			while(lUniLen)
			{
				String sBuf;
				sal_Int32 nLocalLen = 0;
				if( lUniLen >= STRING_MAXLEN )
                {
					nLocalLen =  STRING_MAXLEN - 1;
                }
                else
                {
					nLocalLen = lUniLen;
                }
                sal_Unicode *const pStrBuf =
                    sBuf.AllocBuffer( xub_StrLen( nLocalLen + 1));
				aStream.Read( pStrBuf, 2 * nLocalLen );
				pStrBuf[ nLocalLen ] = '\0';
				aStrBuffer.append( pStrBuf, nLocalLen );
				lUniLen -= nLocalLen;
			}
			rBuffer = aStrBuffer.makeStringAndClear();
		}
        xWrt->bShowProgress = bOldShowProgress;
	}
}

/* -----------------06.07.98 07:33-------------------
 *
 * --------------------------------------------------*/
static void
lcl_setCharStyle(SwDoc *const pDoc, const uno::Any & rValue, SfxItemSet & rSet)
throw (lang::IllegalArgumentException)
{
    SwDocShell *const pDocSh = pDoc->GetDocShell();
	if(pDocSh)
	{
		OUString uStyle;
        if (!(rValue >>= uStyle))
        {
            throw lang::IllegalArgumentException();
        }
		String sStyle;
        SwStyleNameMapper::FillUIName(uStyle, sStyle,
                nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, sal_True);
        SwDocStyleSheet *const pStyle = static_cast<SwDocStyleSheet*>(
            pDocSh->GetStyleSheetPool()->Find(sStyle, SFX_STYLE_FAMILY_CHAR));
        if (!pStyle)
        {
            throw lang::IllegalArgumentException();
        }
        const SwFmtCharFmt aFmt(pStyle->GetCharFmt());
        rSet.Put(aFmt);
	}
};
/* -----------------08.06.06 10:43-------------------
 *
 * --------------------------------------------------*/
static void
lcl_setAutoStyle(IStyleAccess & rStyleAccess, const uno::Any & rValue,
        SfxItemSet & rSet, const bool bPara)
throw (lang::IllegalArgumentException)
{
    OUString uStyle;
    if (!(rValue >>= uStyle))
    {
         throw lang::IllegalArgumentException();
    }
    StylePool::SfxItemSet_Pointer_t pStyle = bPara ?
        rStyleAccess.getByName(uStyle, IStyleAccess::AUTO_STYLE_PARA ):
        rStyleAccess.getByName(uStyle, IStyleAccess::AUTO_STYLE_CHAR );
    if(pStyle.get())
    {
        SwFmtAutoFmt aFmt( (bPara)
            ? sal::static_int_cast< sal_uInt16 >(RES_AUTO_STYLE)
            : sal::static_int_cast< sal_uInt16 >(RES_TXTATR_AUTOFMT) );
        aFmt.SetStyleHandle( pStyle );
        rSet.Put(aFmt);
    }
    else
    {
         throw lang::IllegalArgumentException();
    }
};
/* -----------------30.06.98 08:46-------------------
 *
 * --------------------------------------------------*/
void
SwUnoCursorHelper::SetTxtFmtColl(const uno::Any & rAny, SwPaM & rPaM)
throw (lang::IllegalArgumentException)
{
    SwDoc *const pDoc = rPaM.GetDoc();
    SwDocShell *const pDocSh = pDoc->GetDocShell();
	if(!pDocSh)
		return;
	OUString uStyle;
	rAny >>= uStyle;
	String sStyle;
    SwStyleNameMapper::FillUIName(uStyle, sStyle,
            nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, sal_True );
    SwDocStyleSheet *const pStyle = static_cast<SwDocStyleSheet*>(
            pDocSh->GetStyleSheetPool()->Find(sStyle, SFX_STYLE_FAMILY_PARA));
    if (!pStyle)
    {
        throw lang::IllegalArgumentException();
    }

    SwTxtFmtColl *const pLocal = pStyle->GetCollection();
    UnoActionContext aAction(pDoc);
    pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
    SwPaM *pTmpCrsr = &rPaM;
    do {
        pDoc->SetTxtFmtColl(*pTmpCrsr, pLocal);
        pTmpCrsr = static_cast<SwPaM*>(pTmpCrsr->GetNext());
    } while ( pTmpCrsr != &rPaM );
    pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
}

/* -----------------06.07.98 07:38-------------------
 *
 * --------------------------------------------------*/
bool
SwUnoCursorHelper::SetPageDesc(
        const uno::Any& rValue, SwDoc & rDoc, SfxItemSet & rSet)
{
    OUString uDescName;
    if (!(rValue >>= uDescName))
    {
        return false;
    }
    ::std::auto_ptr<SwFmtPageDesc> pNewDesc;
	const SfxPoolItem* pItem;
	if(SFX_ITEM_SET == rSet.GetItemState( RES_PAGEDESC, sal_True, &pItem ) )
    {
        pNewDesc.reset(new SwFmtPageDesc(
                    *static_cast<const SwFmtPageDesc*>(pItem)));
    }
    if (!pNewDesc.get())
    {
        pNewDesc.reset(new SwFmtPageDesc());
    }
	String sDescName;
    SwStyleNameMapper::FillUIName(uDescName, sDescName,
            nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, sal_True);
    if (!pNewDesc->GetPageDesc() ||
        (pNewDesc->GetPageDesc()->GetName() != sDescName))
	{
		sal_Bool bPut = sal_False;
		if(sDescName.Len())
		{
            SwPageDesc *const pPageDesc =
                ::GetPageDescByName_Impl(rDoc, sDescName);
            if (!pPageDesc)
            {
                throw lang::IllegalArgumentException();
            }
            pNewDesc.get()->RegisterToPageDesc( *pPageDesc );
            bPut = sal_True;
		}
		if(!bPut)
		{
			rSet.ClearItem(RES_BREAK);
			rSet.Put(SwFmtPageDesc());
		}
		else
        {
			rSet.Put(*pNewDesc);
        }
	}
    return true;
}

/* -----------------30.06.98 10:29-------------------
 *
 * --------------------------------------------------*/
static void
lcl_SetNodeNumStart(SwPaM & rCrsr, uno::Any const& rValue)
{
	sal_Int16 nTmp = 1;
	rValue >>= nTmp;
	sal_uInt16 nStt = (nTmp < 0 ? USHRT_MAX : (sal_uInt16)nTmp);
	SwDoc* pDoc = rCrsr.GetDoc();
	UnoActionContext aAction(pDoc);

	if( rCrsr.GetNext() != &rCrsr )			// Mehrfachselektion ?
	{
        pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
		SwPamRanges aRangeArr( rCrsr );
		SwPaM aPam( *rCrsr.GetPoint() );
		for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
		{
		  pDoc->SetNumRuleStart(*aRangeArr.SetPam( n, aPam ).GetPoint());
		  pDoc->SetNodeNumStart(*aRangeArr.SetPam( n, aPam ).GetPoint(),
					nStt );
        }
        pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    }
    else
    {
        pDoc->SetNumRuleStart( *rCrsr.GetPoint());
		pDoc->SetNodeNumStart( *rCrsr.GetPoint(), nStt );
    }
}

static bool
lcl_setCharFmtSequence(SwPaM & rPam, uno::Any const& rValue)
{
    uno::Sequence<OUString> aCharStyles;
    if (!(rValue >>= aCharStyles))
    {
        return false;
    }

    for (sal_Int32 nStyle = 0; nStyle < aCharStyles.getLength(); nStyle++)
    {
        uno::Any aStyle;
        rPam.GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_START, NULL);
        aStyle <<= aCharStyles.getConstArray()[nStyle];
        // create a local set and apply each format directly
        SfxItemSet aSet(rPam.GetDoc()->GetAttrPool(),
                RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT);
        lcl_setCharStyle(rPam.GetDoc(), aStyle, aSet);
        // the first style should replace the current attributes,
        // all other have to be added
        SwUnoCursorHelper::SetCrsrAttr(rPam, aSet, (nStyle)
                ? nsSetAttrMode::SETATTR_DONTREPLACE
                : nsSetAttrMode::SETATTR_DEFAULT);
        rPam.GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_START, NULL);
    }
    return true;
}

static void
lcl_setDropcapCharStyle(SwPaM & rPam, SfxItemSet & rItemSet,
        uno::Any const& rValue)
{
    OUString uStyle;
    if (!(rValue >>= uStyle))
    {
        throw lang::IllegalArgumentException();
    }
    String sStyle;
    SwStyleNameMapper::FillUIName(uStyle, sStyle,
            nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, sal_True);
    SwDoc *const pDoc = rPam.GetDoc();
    //default character style must not be set as default format
    SwDocStyleSheet *const pStyle = static_cast<SwDocStyleSheet*>(
            pDoc->GetDocShell()
            ->GetStyleSheetPool()->Find(sStyle, SFX_STYLE_FAMILY_CHAR));
    if (!pStyle ||
        (static_cast<SwDocStyleSheet*>(pStyle)->GetCharFmt() ==
             pDoc->GetDfltCharFmt()))
    {
        throw lang::IllegalArgumentException();
    }
    ::std::auto_ptr<SwFmtDrop> pDrop;
    SfxPoolItem const* pItem(0);
    if (SFX_ITEM_SET ==
            rItemSet.GetItemState(RES_PARATR_DROP, sal_True, &pItem))
    {
        pDrop.reset(new SwFmtDrop(*static_cast<const SwFmtDrop*>(pItem)));
    }
    if (!pDrop.get())
    {
        pDrop.reset(new SwFmtDrop);
    }
    const rtl::Reference<SwDocStyleSheet> xStyle(new SwDocStyleSheet(*pStyle));
    pDrop->SetCharFmt(xStyle->GetCharFmt());
    rItemSet.Put(*pDrop);
}

static void
lcl_setRubyCharstyle(SfxItemSet & rItemSet, uno::Any const& rValue)
{
    OUString sTmp;
    if (!(rValue >>= sTmp))
    {
        throw lang::IllegalArgumentException();
    }

    ::std::auto_ptr<SwFmtRuby> pRuby;
    const SfxPoolItem* pItem;
    if (SFX_ITEM_SET ==
            rItemSet.GetItemState(RES_TXTATR_CJK_RUBY, sal_True, &pItem))
    {
        pRuby.reset(new SwFmtRuby(*static_cast<const SwFmtRuby*>(pItem)));
    }
    if (!pRuby.get())
    {
        pRuby.reset(new SwFmtRuby(aEmptyStr));
    }
    String sStyle;
    SwStyleNameMapper::FillUIName(sTmp, sStyle,
            nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, sal_True );
    pRuby->SetCharFmtName(sStyle);
    pRuby->SetCharFmtId(0);
    if (sStyle.Len() > 0)
    {
        const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(
                sStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
        pRuby->SetCharFmtId(nId);
    }
    rItemSet.Put(*pRuby);
}

/* -----------------17.09.98 09:44-------------------
 *
 * --------------------------------------------------*/
bool
SwUnoCursorHelper::SetCursorPropertyValue(
        SfxItemPropertySimpleEntry const& rEntry, const uno::Any& rValue,
        SwPaM & rPam, SfxItemSet & rItemSet)
throw (lang::IllegalArgumentException)
{
    if (!(rEntry.nFlags & beans::PropertyAttribute::MAYBEVOID) &&
        (rValue.getValueType() == ::getCppuVoidType()))
    {
        return false;
    }
    bool bRet = true;
    switch (rEntry.nWID)
    {
        case RES_TXTATR_CHARFMT:
            lcl_setCharStyle(rPam.GetDoc(), rValue, rItemSet);
        break;
        case RES_TXTATR_AUTOFMT:
            lcl_setAutoStyle(rPam.GetDoc()->GetIStyleAccess(),
                    rValue, rItemSet, false);
        break;
        case FN_UNO_CHARFMT_SEQUENCE:
            lcl_setCharFmtSequence(rPam, rValue);
        break;
        case FN_UNO_PARA_STYLE :
            SwUnoCursorHelper::SetTxtFmtColl(rValue, rPam);
        break;
        case RES_AUTO_STYLE:
            lcl_setAutoStyle(rPam.GetDoc()->GetIStyleAccess(),
                    rValue, rItemSet, true);
        break;
        case FN_UNO_PAGE_STYLE:
            //FIXME nothing here?
        break;
        case FN_UNO_NUM_START_VALUE:
            lcl_SetNodeNumStart( rPam, rValue );
        break;
        case FN_UNO_NUM_LEVEL:
        // --> OD 2008-07-14 #i91601#
        case FN_UNO_LIST_ID:
        // <--
        case FN_UNO_IS_NUMBER:
        {
            // multi selection is not considered
            SwTxtNode *const pTxtNd = rPam.GetNode()->GetTxtNode();
            // --> OD 2008-05-14 #refactorlists# - check on list style not needed
//                const SwNumRule* pRule = pTxtNd->GetNumRule();
//                if( FN_UNO_NUM_LEVEL == rEntry.nWID  &&  pRule != NULL )
            if (FN_UNO_NUM_LEVEL == rEntry.nWID)
            // <--
            {
                sal_Int16 nLevel = 0;
                if (rValue >>= nLevel)
                {
                    pTxtNd->SetAttrListLevel(nLevel);
                }
            }
            // --> OD 2008-07-14 #i91601#
            else if (FN_UNO_LIST_ID == rEntry.nWID)
            {
                ::rtl::OUString sListId;
                if (rValue >>= sListId)
                {
                    pTxtNd->SetListId( sListId );
                }
            }
            // <--
            else if (FN_UNO_IS_NUMBER == rEntry.nWID)
            {
                sal_Bool bIsNumber(sal_False);
                if (rValue >>= bIsNumber)
                {
                    if (!bIsNumber)
                    {
                        pTxtNd->SetCountedInList( false );
                    }
                }
            }
            //PROPERTY_MAYBEVOID!
        }
        break;
        case FN_NUMBER_NEWSTART:
        {
            sal_Bool bVal = sal_False;
            if (!(rValue >>= bVal))
            {
                throw lang::IllegalArgumentException();
            }
            rPam.GetDoc()->SetNumRuleStart(*rPam.GetPoint(), bVal);
        }
        break;
        case FN_UNO_NUM_RULES:
            SwUnoCursorHelper::setNumberingProperty(rValue, rPam);
        break;
        case RES_PARATR_DROP:
        {
            if (MID_DROPCAP_CHAR_STYLE_NAME == rEntry.nMemberId)
            {
                lcl_setDropcapCharStyle(rPam, rItemSet, rValue);
            }
            else
            {
                bRet = false;
            }
        }
        break;
        case RES_TXTATR_CJK_RUBY:
        {
            if (MID_RUBY_CHARSTYLE == rEntry.nMemberId)
            {
                lcl_setRubyCharstyle(rItemSet, rValue);
            }
            else
            {
                bRet = false;
            }
        }
        break;
        case RES_PAGEDESC:
        {
            if (MID_PAGEDESC_PAGEDESCNAME == rEntry.nMemberId)
            {
                SwUnoCursorHelper::SetPageDesc(
                        rValue, *rPam.GetDoc(), rItemSet);
            }
            else
            {
                bRet = false;
            }
        }
        break;
        default:
            bRet = false;
    }
    return bRet;
}

/* -----------------30.06.98 08:39-------------------
 *
 * --------------------------------------------------*/
SwFmtColl *
SwUnoCursorHelper::GetCurTxtFmtColl(SwPaM & rPaM, const bool bConditional)
{
	static const sal_uInt16 nMaxLookup = 1000;
	SwFmtColl *pFmt = 0;

//	if ( GetCrsrCnt() > nMaxLookup )
//		return 0;
    bool bError = false;
    SwPaM *pTmpCrsr = &rPaM;
    do
    {
        const sal_uLong nSttNd = pTmpCrsr->Start()->nNode.GetIndex();
        const sal_uLong nEndNd = pTmpCrsr->End()->nNode.GetIndex();

		if( nEndNd - nSttNd >= nMaxLookup )
		{
			pFmt = 0;
			break;
		}

        const SwNodes& rNds = rPaM.GetDoc()->GetNodes();
		for( sal_uLong n = nSttNd; n <= nEndNd; ++n )
		{
            SwTxtNode const*const pNd = rNds[ n ]->GetTxtNode();
			if( pNd )
			{
                SwFmtColl *const pNdFmt = (bConditional)
                    ? pNd->GetFmtColl() : &pNd->GetAnyFmtColl();
				if( !pFmt )
                {
					pFmt = pNdFmt;
                }
				else if( pFmt != pNdFmt )
				{
                    bError = true;
					break;
				}
			}
		}

        pTmpCrsr = static_cast<SwPaM*>(pTmpCrsr->GetNext());
    } while ( pTmpCrsr != &rPaM );
    return (bError) ? 0 : pFmt;
}

/* -----------------26.06.98 16:20-------------------
 * 	Hilfsfunktion fuer PageDesc
 * --------------------------------------------------*/
SwPageDesc*	GetPageDescByName_Impl(SwDoc& rDoc, const String& rName)
{
	SwPageDesc* pRet = 0;
	sal_uInt16 nDCount = rDoc.GetPageDescCnt();
	sal_uInt16 i;

	for( i = 0; i < nDCount; i++ )
	{
		SwPageDesc* pDsc = &rDoc._GetPageDesc( i );
		if(pDsc->GetName() == rName)
		{
			pRet = pDsc;
			break;
		}
	}
	if(!pRet)
	{
        for(i = RC_POOLPAGEDESC_BEGIN; i <= STR_POOLPAGE_LANDSCAPE; ++i)
		{
			const String aFmtName(SW_RES(i));
			if(aFmtName == rName)
			{
                pRet = rDoc.GetPageDescFromPool( static_cast< sal_uInt16 >(
                            RES_POOLPAGE_BEGIN + i - RC_POOLPAGEDESC_BEGIN) );
				break;
			}
		}
	}

	return pRet;
 }

/******************************************************************
 * SwXTextCursor
 ******************************************************************/

class SwXTextCursor::Impl
    : public SwClient
{

public:

    const SfxItemPropertySet &  m_rPropSet;
    const enum CursorType       m_eType;
    const uno::Reference< text::XText > m_xParentText;
    SwEventListenerContainer    m_ListenerContainer;
    bool                        m_bIsDisposed;

    Impl(   SwXTextCursor & rThis,
            SwDoc & rDoc,
            const enum CursorType eType,
            uno::Reference<text::XText> xParent,
            SwPosition const& rPoint, SwPosition const*const pMark)
        : SwClient(rDoc.CreateUnoCrsr(rPoint, sal_False))
        , m_rPropSet(*aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_CURSOR))
        , m_eType(eType)
        , m_xParentText(xParent)
        , m_ListenerContainer(static_cast< ::cppu::OWeakObject* >(&rThis))
        , m_bIsDisposed(false)
    {
        if (pMark)
        {
            GetCursor()->SetMark();
            *GetCursor()->GetMark() = *pMark;
        }
    }

    ~Impl() {
        // Impl owns the cursor; delete it here: SolarMutex is locked
        delete GetRegisteredIn();
    }

    SwUnoCrsr * GetCursor() {
        return (m_bIsDisposed) ? 0 :
            static_cast<SwUnoCrsr*>(const_cast<SwModify*>(GetRegisteredIn()));
    }

    SwUnoCrsr & GetCursorOrThrow() {
        SwUnoCrsr *const pUnoCursor( GetCursor() );
        if (!pUnoCursor) {
            throw uno::RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "SwXTextCursor: disposed or invalid")), 0);
        }
        return *pUnoCursor;
    }

    void Invalidate() {
        m_bIsDisposed = true;
        m_ListenerContainer.Disposing();
    }
protected:
    // SwClient
    virtual void Modify(const SfxPoolItem *pOld, const SfxPoolItem *pNew);

};

void SwXTextCursor::Impl::Modify(const SfxPoolItem *pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);

    if (!GetRegisteredIn() ||
        // if the cursor leaves its designated section, it becomes invalid
        ((pOld != NULL) && (pOld->Which() == RES_UNOCURSOR_LEAVES_SECTION)))
    {
        Invalidate();
    }
}


SwUnoCrsr const* SwXTextCursor::GetCursor() const
{
    return m_pImpl->GetCursor();
}

SwUnoCrsr * SwXTextCursor::GetCursor()
{
    return m_pImpl->GetCursor();
}

/*-- 09.12.98 14:19:01---------------------------------------------------

  -----------------------------------------------------------------------*/
SwPaM const* SwXTextCursor::GetPaM() const
{
    return m_pImpl->GetCursor();
}

SwPaM * SwXTextCursor::GetPaM()
{
    return m_pImpl->GetCursor();
}

/*-- 09.12.98 14:19:02---------------------------------------------------

  -----------------------------------------------------------------------*/
SwDoc const* SwXTextCursor::GetDoc() const
{
    return m_pImpl->GetCursor() ? m_pImpl->GetCursor()->GetDoc() : 0;
}
/* -----------------22.07.99 13:52-------------------

 --------------------------------------------------*/
SwDoc * SwXTextCursor::GetDoc()
{
    return m_pImpl->GetCursor() ? m_pImpl->GetCursor()->GetDoc() : 0;
}


/*-- 09.12.98 14:19:19---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextCursor::SwXTextCursor(
        SwDoc & rDoc,
        uno::Reference< text::XText > const& xParent,
        const enum CursorType eType,
        const SwPosition& rPos,
        SwPosition const*const pMark)
    : m_pImpl( new SwXTextCursor::Impl(*this, rDoc, eType, xParent,
                rPos, pMark ) )
{
}

/* -----------------04.03.99 09:02-------------------
 *
 * --------------------------------------------------*/
SwXTextCursor::SwXTextCursor(uno::Reference< text::XText > const& xParent,
        SwPaM const& rSourceCursor, const enum CursorType eType)
    : m_pImpl( new SwXTextCursor::Impl(*this, *rSourceCursor.GetDoc(), eType,
                xParent, *rSourceCursor.GetPoint(),
                rSourceCursor.HasMark() ? rSourceCursor.GetMark() : 0) )
{
}

/*-- 09.12.98 14:19:20---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextCursor::~SwXTextCursor()
{
}

/*-- 09.12.98 14:19:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextCursor::DeleteAndInsert(const ::rtl::OUString& rText,
        const bool bForceExpandHints)
{
    SwUnoCrsr *const pUnoCrsr = m_pImpl->GetCursor();
	if(pUnoCrsr)
	{
		// Start/EndAction
		SwDoc* pDoc = pUnoCrsr->GetDoc();
		UnoActionContext aAction(pDoc);
        const xub_StrLen nTxtLen = rText.getLength();
        pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_INSERT, NULL);
        SwCursor * pCurrent = pUnoCrsr;
        do
        {
            if (pCurrent->HasMark())
            {
                pDoc->DeleteAndJoin(*pCurrent);
            }
			if(nTxtLen)
			{
                const bool bSuccess(
                    SwUnoCursorHelper::DocInsertStringSplitCR(
                        *pDoc, *pCurrent, rText, bForceExpandHints ) );
                DBG_ASSERT( bSuccess, "Doc->Insert(Str) failed." );
                (void) bSuccess;

                SwUnoCursorHelper::SelectPam(*pUnoCrsr, true);
                pCurrent->Left(rText.getLength(),
                        CRSR_SKIP_CHARS, sal_False, sal_False);
            }
            pCurrent = static_cast<SwCursor *>(pCurrent->GetNext());
        } while (pCurrent != pUnoCrsr);
        pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_INSERT, NULL);
    }
}


enum ForceIntoMetaMode { META_CHECK_BOTH, META_INIT_START, META_INIT_END };

static sal_Bool
lcl_ForceIntoMeta(SwPaM & rCursor,
        uno::Reference<text::XText> const & xParentText,
        const enum ForceIntoMetaMode eMode)
{
    sal_Bool bRet( sal_True ); // means not forced in META_CHECK_BOTH
    SwXMeta const * const pXMeta( dynamic_cast<SwXMeta*>(xParentText.get()) );
    ASSERT(pXMeta, "no parent?");
    if (!pXMeta)
        throw uno::RuntimeException();
    SwTxtNode * pTxtNode;
    xub_StrLen nStart;
    xub_StrLen nEnd;
    const bool bSuccess( pXMeta->SetContentRange(pTxtNode, nStart, nEnd) );
    ASSERT(bSuccess, "no pam?");
    if (!bSuccess)
        throw uno::RuntimeException();
    // force the cursor back into the meta if it has moved outside
    SwPosition start(*pTxtNode, nStart);
    SwPosition end(*pTxtNode, nEnd);
    switch (eMode)
    {
        case META_INIT_START:
            *rCursor.GetPoint() = start;
            break;
        case META_INIT_END:
            *rCursor.GetPoint() = end;
            break;
        case META_CHECK_BOTH:
            if (*rCursor.Start() < start)
            {
                *rCursor.Start() = start;
                bRet = sal_False;
            }
            if (*rCursor.End() > end)
            {
                *rCursor.End() = end;
                bRet = sal_False;
            }
            break;
    }
    return bRet;
}

bool SwXTextCursor::IsAtEndOfMeta() const
{
    if (CURSOR_META == m_pImpl->m_eType)
    {
        SwUnoCrsr const * const pCursor( m_pImpl->GetCursor() );
        SwXMeta const*const pXMeta(
                dynamic_cast<SwXMeta*>(m_pImpl->m_xParentText.get()) );
        ASSERT(pXMeta, "no meta?");
        if (pCursor && pXMeta)
        {
            SwTxtNode * pTxtNode;
            xub_StrLen nStart;
            xub_StrLen nEnd;
            const bool bSuccess(
                    pXMeta->SetContentRange(pTxtNode, nStart, nEnd) );
            ASSERT(bSuccess, "no pam?");
            if (bSuccess)
            {
                const SwPosition end(*pTxtNode, nEnd);
                if (   (*pCursor->GetPoint() == end)
                    || (*pCursor->GetMark()  == end))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

/*-- 09.12.98 14:19:19---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXTextCursor::getImplementationName() throw (uno::RuntimeException)
{
	return C2U("SwXTextCursor");
}

/*-- 09.12.98 14:19:19---------------------------------------------------

  -----------------------------------------------------------------------*/
static char const*const g_ServicesTextCursor[] =
{
    "com.sun.star.text.TextCursor",
    "com.sun.star.style.CharacterProperties",
    "com.sun.star.style.CharacterPropertiesAsian",
    "com.sun.star.style.CharacterPropertiesComplex",
    "com.sun.star.style.ParagraphProperties",
    "com.sun.star.style.ParagraphPropertiesAsian",
    "com.sun.star.style.ParagraphPropertiesComplex",
    "com.sun.star.text.TextSortable",
};
static const size_t g_nServicesTextCursor(
    sizeof(g_ServicesTextCursor)/sizeof(g_ServicesTextCursor[0]));

sal_Bool SAL_CALL SwXTextCursor::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    return ::sw::SupportsServiceImpl(
            g_nServicesTextCursor, g_ServicesTextCursor, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXTextCursor::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return ::sw::GetSupportedServiceNamesImpl(
            g_nServicesTextCursor, g_ServicesTextCursor);
}

/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXTextCursor::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
	return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL
SwXTextCursor::getSomething(const uno::Sequence< sal_Int8 >& rId)
throw (uno::RuntimeException)
{
    const sal_Int64 nRet( ::sw::UnoTunnelImpl<SwXTextCursor>(rId, this) );
    return (nRet) ? nRet : OTextCursorHelper::getSomething(rId);
}

/*-- 09.12.98 14:18:12---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SwXTextCursor::collapseToStart() throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    if (rUnoCursor.HasMark())
    {
        if (*rUnoCursor.GetPoint() > *rUnoCursor.GetMark())
        {
            rUnoCursor.Exchange();
        }
        rUnoCursor.DeleteMark();
    }
}
/*-- 09.12.98 14:18:14---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SwXTextCursor::collapseToEnd() throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    if (rUnoCursor.HasMark())
    {
        if (*rUnoCursor.GetPoint() < *rUnoCursor.GetMark())
        {
            rUnoCursor.Exchange();
        }
        rUnoCursor.DeleteMark();
    }
}
/*-- 09.12.98 14:18:41---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL SwXTextCursor::isCollapsed() throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

	sal_Bool bRet = sal_True;
    SwUnoCrsr *const pUnoCrsr = m_pImpl->GetCursor();
	if(pUnoCrsr && pUnoCrsr->GetMark())
	{
		bRet = (*pUnoCrsr->GetPoint() == *pUnoCrsr->GetMark());
	}
	return bRet;
}

/*-- 09.12.98 14:18:42---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL
SwXTextCursor::goLeft(sal_Int16 nCount, sal_Bool Expand)
throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    sal_Bool bRet = rUnoCursor.Left( nCount, CRSR_SKIP_CHARS, sal_False, sal_False);
    if (CURSOR_META == m_pImpl->m_eType)
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH)
            && bRet;
    }
	return bRet;
}
/*-- 09.12.98 14:18:42---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL
SwXTextCursor::goRight(sal_Int16 nCount, sal_Bool Expand)
throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    sal_Bool bRet = rUnoCursor.Right(nCount, CRSR_SKIP_CHARS, sal_False, sal_False);
    if (CURSOR_META == m_pImpl->m_eType)
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH)
            && bRet;
    }
	return bRet;
}

/*-- 09.12.98 14:18:43---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL
SwXTextCursor::gotoStart(sal_Bool Expand) throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    if (CURSOR_BODY == m_pImpl->m_eType)
    {
        rUnoCursor.Move( fnMoveBackward, fnGoDoc );
        //check, that the cursor is not in a table
        SwTableNode * pTblNode = rUnoCursor.GetNode()->FindTableNode();
        SwCntntNode * pCNode = 0;
        while (pTblNode)
        {
            rUnoCursor.GetPoint()->nNode = *pTblNode->EndOfSectionNode();
            pCNode = GetDoc()->GetNodes().GoNext(&rUnoCursor.GetPoint()->nNode);
            pTblNode = (pCNode) ? pCNode->FindTableNode() : 0;
        }
        if (pCNode)
        {
            rUnoCursor.GetPoint()->nContent.Assign(pCNode, 0);
        }
        SwStartNode const*const pTmp =
            rUnoCursor.GetNode()->StartOfSectionNode();
        if (pTmp->IsSectionNode())
        {
            SwSectionNode const*const pSectionStartNode =
                static_cast<SwSectionNode const*>(pTmp);
            if (pSectionStartNode->GetSection().IsHiddenFlag())
            {
                pCNode = GetDoc()->GetNodes().GoNextSection(
                        &rUnoCursor.GetPoint()->nNode, sal_True, sal_False);
                if (pCNode)
                {
                    rUnoCursor.GetPoint()->nContent.Assign(pCNode, 0);
                }
            }
        }
    }
    else if (   (CURSOR_FRAME   == m_pImpl->m_eType)
            ||  (CURSOR_TBLTEXT == m_pImpl->m_eType)
            ||  (CURSOR_HEADER  == m_pImpl->m_eType)
            ||  (CURSOR_FOOTER  == m_pImpl->m_eType)
            ||  (CURSOR_FOOTNOTE== m_pImpl->m_eType)
            ||  (CURSOR_REDLINE == m_pImpl->m_eType))
    {
        rUnoCursor.MoveSection(fnSectionCurr, fnSectionStart);
    }
    else if (CURSOR_META == m_pImpl->m_eType)
    {
        lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText, META_INIT_START);
    }
}
/*-- 09.12.98 14:18:43---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL
SwXTextCursor::gotoEnd(sal_Bool Expand) throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    if (CURSOR_BODY == m_pImpl->m_eType)
    {
        rUnoCursor.Move( fnMoveForward, fnGoDoc );
    }
    else if (   (CURSOR_FRAME   == m_pImpl->m_eType)
            ||  (CURSOR_TBLTEXT == m_pImpl->m_eType)
            ||  (CURSOR_HEADER  == m_pImpl->m_eType)
            ||  (CURSOR_FOOTER  == m_pImpl->m_eType)
            ||  (CURSOR_FOOTNOTE== m_pImpl->m_eType)
            ||  (CURSOR_REDLINE == m_pImpl->m_eType))
    {
        rUnoCursor.MoveSection( fnSectionCurr, fnSectionEnd);
    }
    else if (CURSOR_META == m_pImpl->m_eType)
    {
        lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText, META_INIT_END);
    }
}

void SAL_CALL
SwXTextCursor::gotoRange(
    const uno::Reference< text::XTextRange > & xRange, sal_Bool bExpand)
throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (!xRange.is())
    {
        throw uno::RuntimeException();
    }

    SwUnoCrsr & rOwnCursor( m_pImpl->GetCursorOrThrow() );

    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange  = ::sw::UnoTunnelGetImplementation<SwXTextRange>(xRangeTunnel);
        pCursor =
            ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xRangeTunnel);
    }

    if (!pRange && !pCursor)
    {
        throw uno::RuntimeException();
    }

    SwStartNodeType eSearchNodeType = SwNormalStartNode;
    switch (m_pImpl->m_eType)
    {
        case CURSOR_FRAME:      eSearchNodeType = SwFlyStartNode;       break;
        case CURSOR_TBLTEXT:    eSearchNodeType = SwTableBoxStartNode;  break;
        case CURSOR_FOOTNOTE:   eSearchNodeType = SwFootnoteStartNode;  break;
        case CURSOR_HEADER:     eSearchNodeType = SwHeaderStartNode;    break;
        case CURSOR_FOOTER:     eSearchNodeType = SwFooterStartNode;    break;
        //case CURSOR_INVALID:
        //case CURSOR_BODY:
        default:
            ;
    }
    const SwStartNode* pOwnStartNode =
        rOwnCursor.GetNode()->FindSttNodeByType(eSearchNodeType);

    SwPaM aPam(GetDoc()->GetNodes());
    const SwPaM * pPam(0);
    if (pCursor)
    {
        pPam = pCursor->GetPaM();
    }
    else if (pRange)
    {
        if (pRange->GetPositions(aPam))
        {
            pPam = & aPam;
        }
    }

    if (!pPam)
    {
        throw uno::RuntimeException();
    }
    const SwStartNode* pTmp =
        pPam->GetNode()->FindSttNodeByType(eSearchNodeType);

    //SectionNodes ueberspringen
    while(pTmp && pTmp->IsSectionNode())
    {
        pTmp = pTmp->StartOfSectionNode();
    }
    while(pOwnStartNode && pOwnStartNode->IsSectionNode())
    {
        pOwnStartNode = pOwnStartNode->StartOfSectionNode();
    }
    if(pOwnStartNode != pTmp)
    {
        throw uno::RuntimeException();
    }

    if (CURSOR_META == m_pImpl->m_eType)
    {
        SwPaM CopyPam(*pPam->GetMark(), *pPam->GetPoint());
        const bool bNotForced( lcl_ForceIntoMeta(
                    CopyPam, m_pImpl->m_xParentText, META_CHECK_BOTH) );
        if (!bNotForced)
        {
            throw uno::RuntimeException(
                C2U("gotoRange: parameter range not contained in nesting"
                    " text content for which this cursor was created"),
                static_cast<text::XWordCursor*>(this));
        }
    }

    //jetzt muss die Selektion erweitert werden
    if(bExpand)
    {
        // der Cursor soll alles einschliessen, was bisher von ihm und dem uebergebenen
        // Range eingeschlossen wurde
        const SwPosition aOwnLeft(*rOwnCursor.Start());
        const SwPosition aOwnRight(*rOwnCursor.End());
        SwPosition const& rParamLeft  = *pPam->Start();
        SwPosition const& rParamRight = *pPam->End();

        // jetzt sind vier SwPositions da, zwei davon werden gebraucht, also welche?
        *rOwnCursor.GetPoint() = (aOwnRight > rParamRight)
            ? aOwnRight : *rOwnCursor.GetPoint() = rParamRight;
        rOwnCursor.SetMark();
        *rOwnCursor.GetMark() = (aOwnLeft < rParamLeft)
            ? aOwnLeft : *rOwnCursor.GetMark() = rParamLeft;
    }
    else
    {
        // cursor should be the given range
        *rOwnCursor.GetPoint() = *pPam->GetPoint();
        if (pPam->HasMark())
        {
            rOwnCursor.SetMark();
            *rOwnCursor.GetMark() = *pPam->GetMark();
        }
        else
        {
            rOwnCursor.DeleteMark();
        }
    }
}

/*-- 09.12.98 14:18:44---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL SwXTextCursor::isStartOfWord() throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    const sal_Bool bRet =
        rUnoCursor.IsStartWordWT( i18n::WordType::DICTIONARY_WORD );
	return bRet;
}
/*-- 09.12.98 14:18:44---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL SwXTextCursor::isEndOfWord() throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    const sal_Bool bRet =
        rUnoCursor.IsEndWordWT( i18n::WordType::DICTIONARY_WORD );
	return bRet;
}

/*-- 09.12.98 14:18:44---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL
SwXTextCursor::gotoNextWord(sal_Bool Expand) throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

	//Probleme gibt's noch mit einem Absatzanfang, an dem kein Wort beginnt.
	sal_Bool bRet = sal_False;
    // remember old position to check if cursor has moved
    // since the called functions are sometimes a bit unreliable
    // in specific cases...
    SwPosition  *const pPoint     = rUnoCursor.GetPoint();
    SwNode      *const pOldNode   = &pPoint->nNode.GetNode();
    xub_StrLen   const nOldIndex  = pPoint->nContent.GetIndex();

    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    // end of paragraph
    if (rUnoCursor.GetCntntNode() &&
            (pPoint->nContent == rUnoCursor.GetCntntNode()->Len()))
    {
        rUnoCursor.Right(1, CRSR_SKIP_CHARS, sal_False, sal_False);
    }
    else
    {
        const bool bTmp =
            rUnoCursor.GoNextWordWT( i18n::WordType::DICTIONARY_WORD );
        // if there is no next word within the current paragraph
        // try to go to the start of the next paragraph
        if (!bTmp)
        {
            rUnoCursor.MovePara(fnParaNext, fnParaStart);
        }
    }

    // return true if cursor has moved
    bRet =  (&pPoint->nNode.GetNode() != pOldNode)  ||
            (pPoint->nContent.GetIndex() != nOldIndex);
    if (bRet && (CURSOR_META == m_pImpl->m_eType))
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH);
    }

	return bRet;
}

/*-- 09.12.98 14:18:45---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL
SwXTextCursor::gotoPreviousWord(sal_Bool Expand) throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

	// hier machen Leerzeichen am Absatzanfang Probleme
	sal_Bool bRet = sal_False;
    SwPosition  *const pPoint     = rUnoCursor.GetPoint();
    SwNode      *const pOldNode   = &pPoint->nNode.GetNode();
    xub_StrLen   const nOldIndex  = pPoint->nContent.GetIndex();

    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    // start of paragraph?
    if (pPoint->nContent == 0)
    {
        rUnoCursor.Left(1, CRSR_SKIP_CHARS, sal_False, sal_False);
    }
    else
    {
        rUnoCursor.GoPrevWordWT( i18n::WordType::DICTIONARY_WORD );
        if (pPoint->nContent == 0)
        {
            rUnoCursor.Left(1, CRSR_SKIP_CHARS, sal_False, sal_False);
        }
    }

    // return true if cursor has moved
    bRet =  (&pPoint->nNode.GetNode() != pOldNode)  ||
            (pPoint->nContent.GetIndex() != nOldIndex);
    if (bRet && (CURSOR_META == m_pImpl->m_eType))
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH);
    }

	return bRet;
}

/*-- 09.12.98 14:18:45---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL
SwXTextCursor::gotoEndOfWord(sal_Bool Expand) throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

	sal_Bool bRet = sal_False;
    SwPosition  *const pPoint     = rUnoCursor.GetPoint();
    SwNode      &      rOldNode   = pPoint->nNode.GetNode();
    xub_StrLen   const nOldIndex  = pPoint->nContent.GetIndex();

    const sal_Int16 nWordType = i18n::WordType::DICTIONARY_WORD;
    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    if (!rUnoCursor.IsEndWordWT( nWordType ))
    {
        rUnoCursor.GoEndWordWT( nWordType );
    }

    // restore old cursor if we are not at the end of a word by now
    // otherwise use current one
    bRet = rUnoCursor.IsEndWordWT( nWordType );
    if (!bRet)
    {
        pPoint->nNode       = rOldNode;
        pPoint->nContent    = nOldIndex;
    }
    else if (CURSOR_META == m_pImpl->m_eType)
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH);
    }

	return bRet;
}
/*-- 09.12.98 14:18:46---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL
SwXTextCursor::gotoStartOfWord(sal_Bool Expand) throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

	sal_Bool bRet = sal_False;
    SwPosition  *const pPoint     = rUnoCursor.GetPoint();
    SwNode      &      rOldNode   = pPoint->nNode.GetNode();
    xub_StrLen   const nOldIndex  = pPoint->nContent.GetIndex();

    const sal_Int16 nWordType = i18n::WordType::DICTIONARY_WORD;
    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    if (!rUnoCursor.IsStartWordWT( nWordType ))
    {
        rUnoCursor.GoStartWordWT( nWordType );
    }

    // restore old cursor if we are not at the start of a word by now
    // otherwise use current one
    bRet = rUnoCursor.IsStartWordWT( nWordType );
    if (!bRet)
    {
        pPoint->nNode       = rOldNode;
        pPoint->nContent    = nOldIndex;
    }
    else if (CURSOR_META == m_pImpl->m_eType)
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH);
    }

	return bRet;
}

/*-- 09.12.98 14:18:46---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL
SwXTextCursor::isStartOfSentence() throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    // start of paragraph?
    sal_Bool bRet = rUnoCursor.GetPoint()->nContent == 0;
    // with mark ->no sentence start
    // (check if cursor is no selection, i.e. it does not have
    // a mark or else point and mark are identical)
    if (!bRet && (!rUnoCursor.HasMark() ||
                    *rUnoCursor.GetPoint() == *rUnoCursor.GetMark()))
    {
        SwCursor aCrsr(*rUnoCursor.GetPoint(),0,false);
        SwPosition aOrigPos = *aCrsr.GetPoint();
        aCrsr.GoSentence(SwCursor::START_SENT );
        bRet = aOrigPos == *aCrsr.GetPoint();
    }
	return bRet;
}
/*-- 09.12.98 14:18:47---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL
SwXTextCursor::isEndOfSentence() throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    // end of paragraph?
    sal_Bool bRet = rUnoCursor.GetCntntNode() &&
        (rUnoCursor.GetPoint()->nContent == rUnoCursor.GetCntntNode()->Len());
    // with mark->no sentence end
    // (check if cursor is no selection, i.e. it does not have
    // a mark or else point and mark are identical)
    if (!bRet && (!rUnoCursor.HasMark() ||
                    *rUnoCursor.GetPoint() == *rUnoCursor.GetMark()))
    {
        SwCursor aCrsr(*rUnoCursor.GetPoint(), 0, false);
        SwPosition aOrigPos = *aCrsr.GetPoint();
        aCrsr.GoSentence(SwCursor::END_SENT);
        bRet = aOrigPos == *aCrsr.GetPoint();
    }
	return bRet;
}

/*-- 09.12.98 14:18:47---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL
SwXTextCursor::gotoNextSentence(sal_Bool Expand) throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    const bool bWasEOS = isEndOfSentence();
    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    sal_Bool bRet = rUnoCursor.GoSentence(SwCursor::NEXT_SENT);
    if (!bRet)
    {
        bRet = rUnoCursor.MovePara(fnParaNext, fnParaStart);
    }

    // if at the end of the sentence (i.e. at the space after the '.')
    // advance to next word in order for GoSentence to work properly
    // next time and have isStartOfSentence return true after this call
    if (!rUnoCursor.IsStartWord())
    {
        const bool bNextWord = rUnoCursor.GoNextWord();
        if (bWasEOS && !bNextWord)
        {
            bRet = sal_False;
        }
    }
    if (CURSOR_META == m_pImpl->m_eType)
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH)
            && bRet;
    }
	return bRet;
}
/*-- 09.12.98 14:18:47---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL
SwXTextCursor::gotoPreviousSentence(sal_Bool Expand)
throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    sal_Bool bRet = rUnoCursor.GoSentence(SwCursor::PREV_SENT);
    if (!bRet)
    {
        bRet = rUnoCursor.MovePara(fnParaPrev, fnParaStart);
        if (bRet)
        {
            rUnoCursor.MovePara(fnParaCurr, fnParaEnd);
            // at the end of a paragraph move to the sentence end again
            rUnoCursor.GoSentence(SwCursor::PREV_SENT);
        }
    }
    if (CURSOR_META == m_pImpl->m_eType)
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH)
            && bRet;
    }
	return bRet;
}

/* -----------------15.10.99 08:24-------------------

 --------------------------------------------------*/
sal_Bool SAL_CALL
SwXTextCursor::gotoStartOfSentence(sal_Bool Expand)
throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

	sal_Bool bRet = sal_False;
    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    // if we're at the para start then we wont move
    // but bRet is also true if GoSentence failed but
    // the start of the sentence is reached
    bRet = SwUnoCursorHelper::IsStartOfPara(rUnoCursor)
        || rUnoCursor.GoSentence(SwCursor::START_SENT)
        || SwUnoCursorHelper::IsStartOfPara(rUnoCursor);
    if (CURSOR_META == m_pImpl->m_eType)
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH)
            && bRet;
    }
	return bRet;
}
/* -----------------15.10.99 08:24-------------------

 --------------------------------------------------*/
sal_Bool SAL_CALL
SwXTextCursor::gotoEndOfSentence(sal_Bool Expand) throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

	sal_Bool bRet = sal_False;
    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    // bRet is true if GoSentence() succeeded or if the
    // MovePara() succeeded while the end of the para is
    // not reached already
    sal_Bool bAlreadyParaEnd = SwUnoCursorHelper::IsEndOfPara(rUnoCursor);
    bRet = !bAlreadyParaEnd
            &&  (rUnoCursor.GoSentence(SwCursor::END_SENT)
                 || rUnoCursor.MovePara(fnParaCurr, fnParaEnd));
    if (CURSOR_META == m_pImpl->m_eType)
    {
        bRet = lcl_ForceIntoMeta(rUnoCursor, m_pImpl->m_xParentText,
                    META_CHECK_BOTH)
            && bRet;
    }
	return bRet;
}

/*-- 09.12.98 14:18:48---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL
SwXTextCursor::isStartOfParagraph() throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    const sal_Bool bRet = SwUnoCursorHelper::IsStartOfPara(rUnoCursor);
	return bRet;
}
/*-- 09.12.98 14:18:48---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL
SwXTextCursor::isEndOfParagraph() throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    const sal_Bool bRet = SwUnoCursorHelper::IsEndOfPara(rUnoCursor);
	return bRet;
}

/*-- 09.12.98 14:18:49---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL
SwXTextCursor::gotoStartOfParagraph(sal_Bool Expand)
throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    if (CURSOR_META == m_pImpl->m_eType)
    {
        return sal_False;
    }
    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    sal_Bool bRet = SwUnoCursorHelper::IsStartOfPara(rUnoCursor);
    if (!bRet)
    {
        bRet = rUnoCursor.MovePara(fnParaCurr, fnParaStart);
    }

	// since MovePara(fnParaCurr, fnParaStart) only returns false
	// if we were already at the start of the paragraph this function
	// should always complete successfully.
	DBG_ASSERT( bRet, "gotoStartOfParagraph failed" );
	return bRet;
}
/*-- 09.12.98 14:18:49---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL
SwXTextCursor::gotoEndOfParagraph(sal_Bool Expand) throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    if (CURSOR_META == m_pImpl->m_eType)
    {
        return sal_False;
    }
    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    sal_Bool bRet = SwUnoCursorHelper::IsEndOfPara(rUnoCursor);
    if (!bRet)
    {
        bRet = rUnoCursor.MovePara(fnParaCurr, fnParaEnd);
    }

	// since MovePara(fnParaCurr, fnParaEnd) only returns false
	// if we were already at the end of the paragraph this function
	// should always complete successfully.
	DBG_ASSERT( bRet, "gotoEndOfParagraph failed" );
	return bRet;
}

/*-- 09.12.98 14:18:50---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL
SwXTextCursor::gotoNextParagraph(sal_Bool Expand) throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    if (CURSOR_META == m_pImpl->m_eType)
    {
        return sal_False;
    }
    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    const sal_Bool bRet = rUnoCursor.MovePara(fnParaNext, fnParaStart);
	return bRet;
}
/*-- 09.12.98 14:18:50---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SAL_CALL
SwXTextCursor::gotoPreviousParagraph(sal_Bool Expand)
throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    if (CURSOR_META == m_pImpl->m_eType)
    {
        return sal_False;
    }
    SwUnoCursorHelper::SelectPam(rUnoCursor, Expand);
    const sal_Bool bRet = rUnoCursor.MovePara(fnParaPrev, fnParaStart);
	return bRet;
}

/*-- 09.12.98 14:18:50---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XText > SAL_CALL
SwXTextCursor::getText() throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

	return m_pImpl->m_xParentText;
}

/*-- 09.12.98 14:18:50---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange > SAL_CALL
SwXTextCursor::getStart() throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    uno::Reference< text::XTextRange > xRet;
    SwPaM aPam(*rUnoCursor.Start());
    const uno::Reference< text::XText >  xParent = getText();
    if (CURSOR_META == m_pImpl->m_eType)
    {
        // return cursor to prevent modifying SwXTextRange for META
        SwXTextCursor * const pXCursor(
            new SwXTextCursor(*rUnoCursor.GetDoc(), xParent, CURSOR_META,
                *rUnoCursor.GetPoint()) );
        pXCursor->gotoStart(sal_False);
        xRet = static_cast<text::XWordCursor*>(pXCursor);
    }
    else
    {
        xRet = new SwXTextRange(aPam, xParent);
    }
	return xRet;
}
/*-- 09.12.98 14:18:51---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange > SAL_CALL
SwXTextCursor::getEnd() throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    uno::Reference< text::XTextRange >  xRet;
    SwPaM aPam(*rUnoCursor.End());
    const uno::Reference< text::XText >  xParent = getText();
    if (CURSOR_META == m_pImpl->m_eType)
    {
        // return cursor to prevent modifying SwXTextRange for META
        SwXTextCursor * const pXCursor(
            new SwXTextCursor(*rUnoCursor.GetDoc(), xParent, CURSOR_META,
                *rUnoCursor.GetPoint()) );
        pXCursor->gotoEnd(sal_False);
        xRet = static_cast<text::XWordCursor*>(pXCursor);
    }
    else
    {
        xRet = new SwXTextRange(aPam, xParent);
    }
	return xRet;
}

/*-- 09.12.98 14:18:51---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SAL_CALL SwXTextCursor::getString() throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    OUString aTxt;
    SwUnoCursorHelper::GetTextFromPam(rUnoCursor, aTxt);
	return aTxt;
}
/*-- 09.12.98 14:18:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL
SwXTextCursor::setString(const OUString& aString) throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );
    (void) rUnoCursor; // just to check if valid

    const bool bForceExpandHints( (CURSOR_META != m_pImpl->m_eType)
        ? false
        : dynamic_cast<SwXMeta*>(m_pImpl->m_xParentText.get())
                ->CheckForOwnMemberMeta(*GetPaM(), true) );
    DeleteAndInsert(aString, bForceExpandHints);
}

/* -----------------------------03.05.00 12:56--------------------------------

 ---------------------------------------------------------------------------*/
uno::Any SwUnoCursorHelper::GetPropertyValue(
    SwPaM& rPaM, const SfxItemPropertySet& rPropSet,
    const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    uno::Any aAny;
    SfxItemPropertySimpleEntry const*const pEntry =
        rPropSet.getPropertyMap()->getByName(rPropertyName);

    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                + rPropertyName, static_cast<cppu::OWeakObject *>(0));
    }

    beans::PropertyState eTemp;
    const sal_Bool bDone = SwUnoCursorHelper::getCrsrPropertyValue(
            *pEntry, rPaM, &aAny, eTemp );

    if (!bDone)
    {
        SfxItemSet aSet(rPaM.GetDoc()->GetAttrPool(),
            RES_CHRATR_BEGIN, RES_FRMATR_END - 1,
            RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
            RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
            0L);
        SwUnoCursorHelper::GetCrsrAttr(rPaM, aSet);

        rPropSet.getPropertyValue(*pEntry, aSet, aAny);
    }

	return aAny;
}
/* -----------------------------03.05.00 12:57--------------------------------

 ---------------------------------------------------------------------------*/
void SwUnoCursorHelper::SetPropertyValue(
    SwPaM& rPaM, const SfxItemPropertySet& rPropSet,
    const OUString& rPropertyName,
    const uno::Any& rValue,
    const SetAttrMode nAttrMode, const bool bTableMode)
throw (beans::UnknownPropertyException, beans::PropertyVetoException,
        lang::IllegalArgumentException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    SwDoc *const pDoc = rPaM.GetDoc();
    SfxItemPropertySimpleEntry const*const pEntry =
        rPropSet.getPropertyMap()->getByName(rPropertyName);
    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                + rPropertyName,
            static_cast<cppu::OWeakObject *>(0));
    }

    if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
    {
        throw beans::PropertyVetoException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("Property is read-only: "))
                + rPropertyName,
            static_cast<cppu::OWeakObject *>(0));
    }

    SfxItemSet aItemSet( pDoc->GetAttrPool(), pEntry->nWID, pEntry->nWID );
    SwUnoCursorHelper::GetCrsrAttr( rPaM, aItemSet );

    if (!SwUnoCursorHelper::SetCursorPropertyValue(
                *pEntry, rValue, rPaM, aItemSet))
    {
        rPropSet.setPropertyValue(*pEntry, rValue, aItemSet );
    }
    SwUnoCursorHelper::SetCrsrAttr(rPaM, aItemSet, nAttrMode, bTableMode);
}

/* -----------------------------03.05.00 13:16--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< beans::PropertyState >
SwUnoCursorHelper::GetPropertyStates(
            SwPaM& rPaM, const SfxItemPropertySet& rPropSet,
            const uno::Sequence< OUString >& rPropertyNames,
            const SwGetPropertyStatesCaller eCaller)
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    const OUString* pNames = rPropertyNames.getConstArray();
    uno::Sequence< beans::PropertyState > aRet(rPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    SfxItemPropertyMap const*const pMap = rPropSet.getPropertyMap();
    ::std::auto_ptr<SfxItemSet> pSet;
    ::std::auto_ptr<SfxItemSet> pSetParent;

    for (sal_Int32 i = 0, nEnd = rPropertyNames.getLength(); i < nEnd; i++)
    {
        SfxItemPropertySimpleEntry const*const pEntry =
                pMap->getByName( pNames[i] );
        if(!pEntry)
        {
			if (pNames[i].equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_SKIP_HIDDEN_TEXT)) ||
		        pNames[i].equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_SKIP_PROTECTED_TEXT)))
			{
				pStates[i] = beans::PropertyState_DEFAULT_VALUE;
				continue;
			}
			else if (SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION_TOLERANT ==
                        eCaller)
            {
                //this values marks the element as unknown property
                pStates[i] = beans::PropertyState_MAKE_FIXED_SIZE;
                continue;
            }
            else
            {
                throw beans::UnknownPropertyException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                        + pNames[i],
                    static_cast<cppu::OWeakObject *>(0));
            }
		}
        if (((SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION == eCaller)  ||
             (SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION_TOLERANT == eCaller)) &&
            pEntry->nWID < FN_UNO_RANGE_BEGIN &&
            pEntry->nWID > FN_UNO_RANGE_END  &&
            pEntry->nWID < RES_CHRATR_BEGIN &&
            pEntry->nWID > RES_TXTATR_END )
        {
			pStates[i] = beans::PropertyState_DEFAULT_VALUE;
        }
		else
		{
            if ( pEntry->nWID >= FN_UNO_RANGE_BEGIN &&
                 pEntry->nWID <= FN_UNO_RANGE_END )
            {
                SwUnoCursorHelper::getCrsrPropertyValue(
                    *pEntry, rPaM, 0, pStates[i] );
            }
			else
			{
                if (!pSet.get())
				{
					switch ( eCaller )
					{
                        case SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION_TOLERANT:
                        case SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION:
                            pSet.reset(
                                new SfxItemSet( rPaM.GetDoc()->GetAttrPool(),
                                    RES_CHRATR_BEGIN,   RES_TXTATR_END ));
						break;
						case SW_PROPERTY_STATE_CALLER_SINGLE_VALUE_ONLY:
                            pSet.reset(
                                new SfxItemSet( rPaM.GetDoc()->GetAttrPool(),
                                    pEntry->nWID, pEntry->nWID ));
						break;
						default:
                            pSet.reset( new SfxItemSet(
                                rPaM.GetDoc()->GetAttrPool(),
                                RES_CHRATR_BEGIN, RES_FRMATR_END - 1,
								RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
								RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
                                0L ));
					}
                    // --> OD 2006-07-12 #i63870#
                    SwUnoCursorHelper::GetCrsrAttr( rPaM, *pSet );
                    // <--
				}

                pStates[i] = ( pSet->Count() )
                    ? rPropSet.getPropertyState( *pEntry, *pSet )
                    : beans::PropertyState_DEFAULT_VALUE;

				//try again to find out if a value has been inherited
				if( beans::PropertyState_DIRECT_VALUE == pStates[i] )
                {
                    if (!pSetParent.get())
                    {
                        pSetParent.reset( pSet->Clone( sal_False ) );
                        // --> OD 2006-07-12 #i63870#
                        SwUnoCursorHelper::GetCrsrAttr(
                                rPaM, *pSetParent, sal_True, sal_False );
                        // <--
                    }

                    pStates[i] = ( (pSetParent)->Count() )
                        ? rPropSet.getPropertyState( *pEntry, *pSetParent )
                        : beans::PropertyState_DEFAULT_VALUE;
                }
            }
        }
    }
	return aRet;
}
/* -----------------------------03.05.00 13:17--------------------------------

 ---------------------------------------------------------------------------*/
beans::PropertyState SwUnoCursorHelper::GetPropertyState(
    SwPaM& rPaM, const SfxItemPropertySet& rPropSet,
    const OUString& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    uno::Sequence< OUString > aStrings ( 1 );
	aStrings[0] = rPropertyName;
    uno::Sequence< beans::PropertyState > aSeq =
        GetPropertyStates(rPaM, rPropSet, aStrings,
                SW_PROPERTY_STATE_CALLER_SINGLE_VALUE_ONLY );
	return aSeq[0];
}
/* -----------------------------03.05.00 13:20--------------------------------

 ---------------------------------------------------------------------------*/
static void
lcl_SelectParaAndReset( SwPaM &rPaM, SwDoc & rDoc,
        SvUShortsSort const*const pWhichIds = 0 )
{
	// if we are reseting paragraph attributes, we need to select the full paragraph first
	SwPosition aStart = *rPaM.Start();
	SwPosition aEnd = *rPaM.End();
    ::std::auto_ptr< SwUnoCrsr > pTemp ( rDoc.CreateUnoCrsr(aStart, sal_False) );
	if(!SwUnoCursorHelper::IsStartOfPara(*pTemp))
    {
		pTemp->MovePara(fnParaCurr, fnParaStart);
    }
	pTemp->SetMark();
	*pTemp->GetPoint() = aEnd;
    SwUnoCursorHelper::SelectPam(*pTemp, true);
	if(!SwUnoCursorHelper::IsEndOfPara(*pTemp))
    {
		pTemp->MovePara(fnParaCurr, fnParaEnd);
    }
    rDoc.ResetAttrs(*pTemp, sal_True, pWhichIds);
}


void SwUnoCursorHelper::SetPropertyToDefault(
	SwPaM& rPaM, const SfxItemPropertySet& rPropSet,
	const OUString& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    SwDoc & rDoc = *rPaM.GetDoc();
    SfxItemPropertySimpleEntry const*const pEntry =
        rPropSet.getPropertyMap()->getByName(rPropertyName);
    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                + rPropertyName, static_cast<cppu::OWeakObject *>(0));
    }

    if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
    {
        throw uno::RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM(
                "setPropertyToDefault: property is read-only: "))
                + rPropertyName, 0);
    }

    if (pEntry->nWID < RES_FRMATR_END)
    {
        SvUShortsSort aWhichIds;
        aWhichIds.Insert(pEntry->nWID);
        if (pEntry->nWID < RES_PARATR_BEGIN)
        {
            rDoc.ResetAttrs(rPaM, sal_True, &aWhichIds);
        }
        else
        {
            lcl_SelectParaAndReset ( rPaM, rDoc, &aWhichIds );
        }
    }
    else
    {
        SwUnoCursorHelper::resetCrsrPropertyValue(*pEntry, rPaM);
    }
}

/* -----------------------------03.05.00 13:19--------------------------------

 ---------------------------------------------------------------------------*/
uno::Any SwUnoCursorHelper::GetPropertyDefault(
	SwPaM& rPaM, const SfxItemPropertySet& rPropSet,
	const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    SfxItemPropertySimpleEntry const*const pEntry =
        rPropSet.getPropertyMap()->getByName(rPropertyName);
    if (!pEntry)
    {
        throw beans::UnknownPropertyException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                + rPropertyName, static_cast<cppu::OWeakObject *>(0));
    }

    uno::Any aRet;
    if (pEntry->nWID < RES_FRMATR_END)
    {
        SwDoc & rDoc = *rPaM.GetDoc();
        const SfxPoolItem& rDefItem =
            rDoc.GetAttrPool().GetDefaultItem(pEntry->nWID);
        rDefItem.QueryValue(aRet, pEntry->nMemberId);
    }
	return aRet;
}

/*-- 09.12.98 14:18:54---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXTextCursor::getPropertySetInfo() throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

	static uno::Reference< beans::XPropertySetInfo >  xRef;
	if(!xRef.is())
	{
        static SfxItemPropertyMapEntry aCrsrExtMap_Impl[] =
		{
			{ SW_PROP_NAME(UNO_NAME_IS_SKIP_HIDDEN_TEXT), FN_SKIP_HIDDEN_TEXT, &::getBooleanCppuType(), PROPERTY_NONE,     0},
			{ SW_PROP_NAME(UNO_NAME_IS_SKIP_PROTECTED_TEXT), FN_SKIP_PROTECTED_TEXT, &::getBooleanCppuType(), PROPERTY_NONE,     0},
			{0,0,0,0,0,0}
		};
        const uno::Reference< beans::XPropertySetInfo >  xInfo =
            m_pImpl->m_rPropSet.getPropertySetInfo();
		// PropertySetInfo verlaengern!
		const uno::Sequence<beans::Property> aPropSeq = xInfo->getProperties();
		xRef = new SfxExtItemPropertySetInfo(
			aCrsrExtMap_Impl,
			aPropSeq );
	}
	return xRef;
}

/*-- 09.12.98 14:18:54---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL
SwXTextCursor::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue)
throw (beans::UnknownPropertyException, beans::PropertyVetoException,
        lang::IllegalArgumentException, lang::WrappedTargetException,
        uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    if (rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_IS_SKIP_HIDDEN_TEXT)))
    {
        sal_Bool bSet(sal_False);
        if (!(rValue >>= bSet))
        {
            throw lang::IllegalArgumentException();
        }
        rUnoCursor.SetSkipOverHiddenSections(bSet);
    }
    else if (rPropertyName.equalsAsciiL(
                SW_PROP_NAME(UNO_NAME_IS_SKIP_PROTECTED_TEXT)))
    {
        sal_Bool bSet(sal_False);
        if (!(rValue >>= bSet))
        {
            throw lang::IllegalArgumentException();
        }
        rUnoCursor.SetSkipOverProtectSections(bSet);
    }
    else
    {
        SwUnoCursorHelper::SetPropertyValue(rUnoCursor,
                m_pImpl->m_rPropSet, rPropertyName, rValue);
    }
}

/*-- 09.12.98 14:18:55---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SAL_CALL
SwXTextCursor::getPropertyValue(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

	uno::Any aAny;
    if (rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_IS_SKIP_HIDDEN_TEXT)))
    {
        const sal_Bool bSet = rUnoCursor.IsSkipOverHiddenSections();
        aAny <<= bSet;
    }
    else if (rPropertyName.equalsAsciiL(
                SW_PROP_NAME(UNO_NAME_IS_SKIP_PROTECTED_TEXT)))
    {
        const sal_Bool bSet = rUnoCursor.IsSkipOverProtectSections();
        aAny <<= bSet;
    }
    else
    {
        aAny = SwUnoCursorHelper::GetPropertyValue(rUnoCursor,
                m_pImpl->m_rPropSet, rPropertyName);
    }
	return aAny;
}

/*-- 09.12.98 14:18:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL
SwXTextCursor::addPropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXTextCursor::addPropertyChangeListener(): not implemented");
}

/*-- 09.12.98 14:18:57---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL
SwXTextCursor::removePropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXTextCursor::removePropertyChangeListener(): not implemented");
}

/*-- 09.12.98 14:18:57---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL
SwXTextCursor::addVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXTextCursor::addVetoableChangeListener(): not implemented");
}

/*-- 09.12.98 14:18:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL
SwXTextCursor::removeVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    OSL_ENSURE(false,
        "SwXTextCursor::removeVetoableChangeListener(): not implemented");
}

/*-- 05.03.99 11:36:11---------------------------------------------------

  -----------------------------------------------------------------------*/
beans::PropertyState SAL_CALL
SwXTextCursor::getPropertyState(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    const beans::PropertyState eRet = SwUnoCursorHelper::GetPropertyState(
            rUnoCursor, m_pImpl->m_rPropSet, rPropertyName);
    return eRet;
}
/*-- 05.03.99 11:36:11---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< beans::PropertyState > SAL_CALL
SwXTextCursor::getPropertyStates(
        const uno::Sequence< OUString >& rPropertyNames)
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    return SwUnoCursorHelper::GetPropertyStates(
            rUnoCursor, m_pImpl->m_rPropSet, rPropertyNames);
}

/*-- 05.03.99 11:36:12---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL
SwXTextCursor::setPropertyToDefault(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    // forward: need no solar mutex here
    uno::Sequence < OUString > aSequence ( &rPropertyName, 1 );
	setPropertiesToDefault ( aSequence );
}
/*-- 05.03.99 11:36:12---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SAL_CALL
SwXTextCursor::getPropertyDefault(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    // forward: need no solar mutex here
    const uno::Sequence < OUString > aSequence ( &rPropertyName, 1 );
	return getPropertyDefaults ( aSequence ).getConstArray()[0];
}

// para specific attribut ranges
static sal_uInt16 g_ParaResetableSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_PARATR_BEGIN, RES_PARATR_END-1,
    // --> OD 2008-02-25 #refactorlists#
    RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END-1,
    // <--
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// selection specific attribut ranges
static sal_uInt16 g_ResetableSetRange[] = {
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
    RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
    RES_TXTATR_CJK_RUBY, RES_TXTATR_CJK_RUBY,
    RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
    0
};

static void
lcl_EnumerateIds(sal_uInt16 const* pIdRange, SvUShortsSort & rWhichIds)
{
    while (*pIdRange)
    {
        const sal_uInt16 nStart = sal::static_int_cast<sal_uInt16>(*pIdRange++);
        const sal_uInt16 nEnd   = sal::static_int_cast<sal_uInt16>(*pIdRange++);
        for (sal_uInt16 nId = nStart + 1;  nId <= nEnd;  ++nId)
        {
            rWhichIds.Insert( nId );
        }
    }
}

void SAL_CALL
SwXTextCursor::setAllPropertiesToDefault()
throw (uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    SvUShortsSort aParaWhichIds;
    SvUShortsSort aWhichIds;
    lcl_EnumerateIds(g_ParaResetableSetRange, aParaWhichIds);
    lcl_EnumerateIds(g_ResetableSetRange, aWhichIds);
    if (aParaWhichIds.Count())
    {
        lcl_SelectParaAndReset(rUnoCursor, *rUnoCursor.GetDoc(),
            &aParaWhichIds);
    }
    if (aWhichIds.Count())
    {
        rUnoCursor.GetDoc()->ResetAttrs(rUnoCursor, sal_True, &aWhichIds);
    }
}

void SAL_CALL
SwXTextCursor::setPropertiesToDefault(
        const uno::Sequence< OUString >& rPropertyNames)
throw (beans::UnknownPropertyException, uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    const sal_Int32 nCount = rPropertyNames.getLength();
	if ( nCount )
    {
        SwDoc & rDoc = *rUnoCursor.GetDoc();
        const OUString * pNames = rPropertyNames.getConstArray();
        SvUShortsSort aWhichIds;
        SvUShortsSort aParaWhichIds;
        for (sal_Int32 i = 0; i < nCount; i++)
        {
            SfxItemPropertySimpleEntry const*const  pEntry =
                m_pImpl->m_rPropSet.getPropertyMap()->getByName( pNames[i] );
            if (!pEntry)
            {
                if (pNames[i].equalsAsciiL(
                        SW_PROP_NAME(UNO_NAME_IS_SKIP_HIDDEN_TEXT)) ||
                    pNames[i].equalsAsciiL(
                        SW_PROP_NAME(UNO_NAME_IS_SKIP_PROTECTED_TEXT)))
                {
                    continue;
                }
                throw beans::UnknownPropertyException(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                        + pNames[i],
                    static_cast<cppu::OWeakObject *>(this));
            }
            if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
            {
                throw uno::RuntimeException(
                    OUString(RTL_CONSTASCII_USTRINGPARAM(
                            "setPropertiesToDefault: property is read-only: "))
                        + pNames[i],
                    static_cast<cppu::OWeakObject *>(this));
            }

            if (pEntry->nWID < RES_FRMATR_END)
            {
                if (pEntry->nWID < RES_PARATR_BEGIN)
                {
                    aWhichIds.Insert(pEntry->nWID);
                }
                else
                {
                    aParaWhichIds.Insert(pEntry->nWID);
                }
            }
            else if (pEntry->nWID == FN_UNO_NUM_START_VALUE)
            {
                SwUnoCursorHelper::resetCrsrPropertyValue(*pEntry, rUnoCursor);
            }
        }

        if (aParaWhichIds.Count())
        {
            lcl_SelectParaAndReset(rUnoCursor, rDoc, &aParaWhichIds);
        }
        if (aWhichIds.Count())
        {
            rDoc.ResetAttrs(rUnoCursor, sal_True, &aWhichIds);
        }
    }
}

uno::Sequence< uno::Any > SAL_CALL
SwXTextCursor::getPropertyDefaults(
        const uno::Sequence< OUString >& rPropertyNames)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    const sal_Int32 nCount = rPropertyNames.getLength();
    uno::Sequence< uno::Any > aRet(nCount);
	if ( nCount )
    {
        SwDoc & rDoc = *rUnoCursor.GetDoc();
        const OUString *pNames = rPropertyNames.getConstArray();
        uno::Any *pAny = aRet.getArray();
        for (sal_Int32 i = 0; i < nCount; i++)
        {
            SfxItemPropertySimpleEntry const*const pEntry =
                m_pImpl->m_rPropSet.getPropertyMap()->getByName( pNames[i] );
            if (!pEntry)
            {
                if (pNames[i].equalsAsciiL(
                        SW_PROP_NAME(UNO_NAME_IS_SKIP_HIDDEN_TEXT)) ||
                    pNames[i].equalsAsciiL(
                        SW_PROP_NAME(UNO_NAME_IS_SKIP_PROTECTED_TEXT)))
                {
                    continue;
                }
                throw beans::UnknownPropertyException(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown property: "))
                        + pNames[i],
                    static_cast<cppu::OWeakObject *>(0));
            }
            if (pEntry->nWID < RES_FRMATR_END)
            {
                const SfxPoolItem& rDefItem =
                    rDoc.GetAttrPool().GetDefaultItem(pEntry->nWID);
                rDefItem.QueryValue(pAny[i], pEntry->nMemberId);
            }
        }
    }
	return aRet;
}

/*-- 10.03.2008 09:58:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL
SwXTextCursor::makeRedline(
    const ::rtl::OUString& rRedlineType,
    const uno::Sequence< beans::PropertyValue >& rRedlineProperties)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    SwUnoCursorHelper::makeRedline(rUnoCursor, rRedlineType, rRedlineProperties);
}

/*-- 09.12.98 14:18:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SwXTextCursor::insertDocumentFromURL(const OUString& rURL,
    const uno::Sequence< beans::PropertyValue >& rOptions)
throw (lang::IllegalArgumentException, io::IOException,
        uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    SwUnoCursorHelper::InsertFile(&rUnoCursor, rURL, rOptions);
}

/* -----------------------------15.12.00 14:01--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< beans::PropertyValue >
SwUnoCursorHelper::CreateSortDescriptor(const bool bFromTable)
{
    uno::Sequence< beans::PropertyValue > aRet(5);
    beans::PropertyValue* pArray = aRet.getArray();

    uno::Any aVal;
    aVal.setValue( &bFromTable, ::getCppuBooleanType());
    pArray[0] = beans::PropertyValue(C2U("IsSortInTable"), -1, aVal,
                    beans::PropertyState_DIRECT_VALUE);

    aVal <<= sal_Unicode(' ');
    pArray[1] = beans::PropertyValue(C2U("Delimiter"), -1, aVal,
                    beans::PropertyState_DIRECT_VALUE);

    aVal <<= (sal_Bool) sal_False;
    pArray[2] = beans::PropertyValue(C2U("IsSortColumns"), -1, aVal,
                    beans::PropertyState_DIRECT_VALUE);

    aVal <<= (sal_Int32) 3;
    pArray[3] = beans::PropertyValue(C2U("MaxSortFieldsCount"), -1, aVal,
                    beans::PropertyState_DIRECT_VALUE);

    uno::Sequence< table::TableSortField > aFields(3);
    table::TableSortField* pFields = aFields.getArray();

    lang::Locale aLang( SvxCreateLocale( LANGUAGE_SYSTEM ) );
    // get collator algorithm to be used for the locale
    uno::Sequence< OUString > aSeq(
            GetAppCollator().listCollatorAlgorithms( aLang ) );
    const sal_Int32 nLen = aSeq.getLength();
    DBG_ASSERT( nLen > 0, "list of collator algorithms is empty!");
    OUString aCollAlg;
    if (nLen > 0)
    {
        aCollAlg = aSeq.getConstArray()[0];
    }

#if OSL_DEBUG_LEVEL > 1
    const OUString *pTxt = aSeq.getConstArray();
    (void)pTxt;
#endif

    pFields[0].Field = 1;
    pFields[0].IsAscending = sal_True;
    pFields[0].IsCaseSensitive = sal_False;
    pFields[0].FieldType = table::TableSortFieldType_ALPHANUMERIC;
    pFields[0].CollatorLocale = aLang;
    pFields[0].CollatorAlgorithm = aCollAlg;

    pFields[1].Field = 1;
    pFields[1].IsAscending = sal_True;
    pFields[1].IsCaseSensitive = sal_False;
    pFields[1].FieldType = table::TableSortFieldType_ALPHANUMERIC;
    pFields[1].CollatorLocale = aLang;
    pFields[1].CollatorAlgorithm = aCollAlg;

    pFields[2].Field = 1;
    pFields[2].IsAscending = sal_True;
    pFields[2].IsCaseSensitive = sal_False;
    pFields[2].FieldType = table::TableSortFieldType_ALPHANUMERIC;
    pFields[2].CollatorLocale = aLang;
    pFields[2].CollatorAlgorithm = aCollAlg;

    aVal <<= aFields;
    pArray[4] = beans::PropertyValue(C2U("SortFields"), -1, aVal,
                    beans::PropertyState_DIRECT_VALUE);

    return aRet;
}

/*-- 09.12.98 14:18:58---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< beans::PropertyValue > SAL_CALL
SwXTextCursor::createSortDescriptor() throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    return SwUnoCursorHelper::CreateSortDescriptor(false);
}

/* -----------------------------15.12.00 14:06--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwUnoCursorHelper::ConvertSortProperties(
    const uno::Sequence< beans::PropertyValue >& rDescriptor,
    SwSortOptions& rSortOpt)
{
    sal_Bool bRet = sal_True;
    const beans::PropertyValue* pProperties = rDescriptor.getConstArray();

    rSortOpt.bTable = sal_False;
    rSortOpt.cDeli = ' ';
    rSortOpt.eDirection = SRT_COLUMNS;  //!! UI text may be contrary though !!

    SwSortKey* pKey1 = new SwSortKey;
    pKey1->nColumnId = USHRT_MAX;
    pKey1->bIsNumeric = sal_True;
    pKey1->eSortOrder = SRT_ASCENDING;

    SwSortKey* pKey2 = new SwSortKey;
    pKey2->nColumnId = USHRT_MAX;
    pKey2->bIsNumeric = sal_True;
    pKey2->eSortOrder = SRT_ASCENDING;

    SwSortKey* pKey3 = new SwSortKey;
    pKey3->nColumnId = USHRT_MAX;
    pKey3->bIsNumeric = sal_True;
    pKey3->eSortOrder = SRT_ASCENDING;
    SwSortKey* aKeys[3] = {pKey1, pKey2, pKey3};

    sal_Bool bOldSortdescriptor(sal_False);
    sal_Bool bNewSortdescriptor(sal_False);

    for (sal_Int32 n = 0; n < rDescriptor.getLength(); ++n)
    {
        uno::Any aValue( pProperties[n].Value );
//      String sPropName = pProperties[n].Name;
        const OUString& rPropName = pProperties[n].Name;

        // old and new sortdescriptor
        if (rPropName.equalsAscii("IsSortInTable"))
        {
            if (aValue.getValueType() == ::getBooleanCppuType())
            {
                rSortOpt.bTable = *(sal_Bool*)aValue.getValue();
            }
            else
            {
                bRet = sal_False;
            }
        }
        else if (rPropName.equalsAscii("Delimiter"))
        {
            sal_Unicode uChar = sal_Unicode();
            if (aValue >>= uChar)
            {
                rSortOpt.cDeli = uChar;
            }
            else
            {
                bRet = sal_False;
            }
        }
        // old sortdescriptor
        else if (rPropName.equalsAscii("SortColumns"))
        {
            bOldSortdescriptor = sal_True;
            sal_Bool bTemp(sal_False);
            if (aValue >>= bTemp)
            {
                rSortOpt.eDirection = bTemp ? SRT_COLUMNS : SRT_ROWS;
            }
            else
            {
                bRet = sal_False;
            }
        }
        else if ( rPropName.equalsAscii("IsCaseSensitive"))
        {
            bOldSortdescriptor = sal_True;
            sal_Bool bTemp(sal_False);
            if (aValue >>= bTemp)
            {
                rSortOpt.bIgnoreCase = !bTemp;
            }
            else
            {
                bRet = sal_False;
            }
        }
        else if (rPropName.equalsAscii("CollatorLocale"))
        {
            bOldSortdescriptor = sal_True;
            lang::Locale aLocale;
            if (aValue >>= aLocale)
            {
                rSortOpt.nLanguage = SvxLocaleToLanguage( aLocale );
            }
            else
            {
                bRet = sal_False;
            }
        }
        else if (rPropName.matchAsciiL("CollatorAlgorithm", 17) &&
            rPropName.getLength() == 18 &&
            (rPropName.getStr()[17] >= '0' && rPropName.getStr()[17] <= '9'))
        {
            bOldSortdescriptor = sal_True;
            sal_uInt16 nIndex = rPropName.getStr()[17];
            nIndex -= '0';
            OUString aTxt;
            if ((aValue >>= aTxt) && nIndex < 3)
            {
                aKeys[nIndex]->sSortType = aTxt;
            }
            else
            {
                bRet = sal_False;
            }
        }
        else if (rPropName.matchAsciiL("SortRowOrColumnNo", 17) &&
            rPropName.getLength() == 18 &&
            (rPropName.getStr()[17] >= '0' && rPropName.getStr()[17] <= '9'))
        {
            bOldSortdescriptor = sal_True;
            sal_uInt16 nIndex = rPropName.getStr()[17];
            nIndex -= '0';
            sal_Int16 nCol = -1;
            if (aValue.getValueType() == ::getCppuType((const sal_Int16*)0)
                && nIndex < 3)
            {
                aValue >>= nCol;
            }
            if (nCol >= 0)
            {
                aKeys[nIndex]->nColumnId = nCol;
            }
            else
            {
                bRet = sal_False;
            }
        }
        else if (0 == rPropName.indexOf(C2U("IsSortNumeric")) &&
            rPropName.getLength() == 14 &&
            (rPropName.getStr()[13] >= '0' && rPropName.getStr()[13] <= '9'))
        {
            bOldSortdescriptor = sal_True;
            sal_uInt16 nIndex = rPropName.getStr()[13];
            nIndex = nIndex - '0';
            if (aValue.getValueType() == ::getBooleanCppuType() && nIndex < 3)
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                aKeys[nIndex]->bIsNumeric = bTemp;
            }
            else
            {
                bRet = sal_False;
            }
        }
        else if (0 == rPropName.indexOf(C2U("IsSortAscending")) &&
            rPropName.getLength() == 16 &&
            (rPropName.getStr()[15] >= '0' && rPropName.getStr()[15] <= '9'))
        {
            bOldSortdescriptor = sal_True;
            sal_uInt16 nIndex = rPropName.getStr()[15];
            nIndex -= '0';
            if (aValue.getValueType() == ::getBooleanCppuType() && nIndex < 3)
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                aKeys[nIndex]->eSortOrder = (bTemp)
                    ? SRT_ASCENDING : SRT_DESCENDING;
            }
            else
            {
                bRet = sal_False;
            }
        }
        // new sortdescriptor
        else if (rPropName.equalsAscii("IsSortColumns"))
        {
            bNewSortdescriptor = sal_True;
            if (aValue.getValueType() == ::getBooleanCppuType())
            {
                sal_Bool bTemp = *(sal_Bool*)aValue.getValue();
                rSortOpt.eDirection = bTemp ? SRT_COLUMNS : SRT_ROWS;
            }
            else
            {
                bRet = sal_False;
            }
        }
        else if (rPropName.equalsAscii("SortFields"))
        {
            bNewSortdescriptor = sal_True;
            uno::Sequence < table::TableSortField > aFields;
            if (aValue >>= aFields)
            {
                sal_Int32 nCount(aFields.getLength());
                if (nCount <= 3)
                {
                    table::TableSortField* pFields = aFields.getArray();
                    for (sal_Int32 i = 0; i < nCount; ++i)
                    {
                        rSortOpt.bIgnoreCase = !pFields[i].IsCaseSensitive;
                        rSortOpt.nLanguage =
                            SvxLocaleToLanguage( pFields[i].CollatorLocale );
                        aKeys[i]->sSortType = pFields[i].CollatorAlgorithm;
                        aKeys[i]->nColumnId =
                            static_cast<sal_uInt16>(pFields[i].Field);
                        aKeys[i]->bIsNumeric = (pFields[i].FieldType ==
                                table::TableSortFieldType_NUMERIC);
                        aKeys[i]->eSortOrder = (pFields[i].IsAscending)
                            ? SRT_ASCENDING : SRT_DESCENDING;
                    }
                }
                else
                {
                    bRet = sal_False;
                }
            }
            else
            {
                bRet = sal_False;
            }
        }
    }

    if (bNewSortdescriptor && bOldSortdescriptor)
    {
        DBG_ERROR("someone tried to set the old deprecated and "
            "the new sortdescriptor");
        bRet = sal_False;
    }

    if (pKey1->nColumnId != USHRT_MAX)
    {
        rSortOpt.aKeys.C40_INSERT(SwSortKey, pKey1, rSortOpt.aKeys.Count());
    }
    if (pKey2->nColumnId != USHRT_MAX)
    {
        rSortOpt.aKeys.C40_INSERT(SwSortKey, pKey2, rSortOpt.aKeys.Count());
    }
    if (pKey3->nColumnId != USHRT_MAX)
    {
        rSortOpt.aKeys.C40_INSERT(SwSortKey, pKey3, rSortOpt.aKeys.Count());
    }

    return bRet && rSortOpt.aKeys.Count() > 0;
}

/*-- 09.12.98 14:19:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL
SwXTextCursor::sort(const uno::Sequence< beans::PropertyValue >& rDescriptor)
throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    if (rUnoCursor.HasMark())
    {
        SwSortOptions aSortOpt;
        if (!SwUnoCursorHelper::ConvertSortProperties(rDescriptor, aSortOpt))
        {
            throw uno::RuntimeException();
        }
        UnoActionContext aContext( rUnoCursor.GetDoc() );

        SwPosition & rStart = *rUnoCursor.Start();
        SwPosition & rEnd   = *rUnoCursor.End();

        SwNodeIndex aPrevIdx( rStart.nNode, -1 );
        const sal_uLong nOffset = rEnd.nNode.GetIndex() - rStart.nNode.GetIndex();
        const xub_StrLen nCntStt  = rStart.nContent.GetIndex();

        rUnoCursor.GetDoc()->SortText(rUnoCursor, aSortOpt);

        // Selektion wieder setzen
        rUnoCursor.DeleteMark();
        rUnoCursor.GetPoint()->nNode.Assign( aPrevIdx.GetNode(), +1 );
        SwCntntNode *const pCNd = rUnoCursor.GetCntntNode();
        xub_StrLen nLen = pCNd->Len();
        if (nLen > nCntStt)
        {
            nLen = nCntStt;
        }
        rUnoCursor.GetPoint()->nContent.Assign(pCNd, nLen );
        rUnoCursor.SetMark();

        rUnoCursor.GetPoint()->nNode += nOffset;
        SwCntntNode *const pCNd2 = rUnoCursor.GetCntntNode();
        rUnoCursor.GetPoint()->nContent.Assign( pCNd2, pCNd2->Len() );
    }
}

/* -----------------------------03.04.00 09:11--------------------------------

 ---------------------------------------------------------------------------*/
uno::Reference< container::XEnumeration > SAL_CALL
SwXTextCursor::createContentEnumeration(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    if (!rServiceName.equalsAscii("com.sun.star.text.TextContent"))
    {
        throw uno::RuntimeException();
    }

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    uno::Reference< container::XEnumeration > xRet =
        new SwXParaFrameEnumeration(rUnoCursor, PARAFRAME_PORTION_TEXTRANGE);
    return xRet;
}

/* -----------------------------07.03.01 14:53--------------------------------

 ---------------------------------------------------------------------------*/
uno::Reference< container::XEnumeration > SAL_CALL
SwXTextCursor::createEnumeration() throw (uno::RuntimeException)
{
    vos::OGuard g(Application::GetSolarMutex());

    SwUnoCrsr & rUnoCursor( m_pImpl->GetCursorOrThrow() );

    const uno::Reference<lang::XUnoTunnel> xTunnel(
            m_pImpl->m_xParentText, uno::UNO_QUERY);
    SwXText* pParentText = 0;
    if (xTunnel.is())
    {
        pParentText = ::sw::UnoTunnelGetImplementation<SwXText>(xTunnel);
    }
    DBG_ASSERT(pParentText, "parent is not a SwXText");
    if (!pParentText)
    {
        throw uno::RuntimeException();
    }

    ::std::auto_ptr<SwUnoCrsr> pNewCrsr(
        rUnoCursor.GetDoc()->CreateUnoCrsr(*rUnoCursor.GetPoint()) );
    if (rUnoCursor.HasMark())
    {
        pNewCrsr->SetMark();
        *pNewCrsr->GetMark() = *rUnoCursor.GetMark();
    }
    const CursorType eSetType = (CURSOR_TBLTEXT == m_pImpl->m_eType)
            ? CURSOR_SELECTION_IN_TABLE : CURSOR_SELECTION;
    SwTableNode const*const pStartNode( (CURSOR_TBLTEXT == m_pImpl->m_eType)
            ? rUnoCursor.GetPoint()->nNode.GetNode().FindTableNode()
            : 0);
    SwTable const*const pTable(
            (pStartNode) ? & pStartNode->GetTable() : 0 );
    const uno::Reference< container::XEnumeration > xRet =
        new SwXParagraphEnumeration(
                pParentText, pNewCrsr, eSetType, pStartNode, pTable);

    return xRet;
}

/* -----------------------------07.03.01 15:43--------------------------------

 ---------------------------------------------------------------------------*/
uno::Type SAL_CALL
SwXTextCursor::getElementType() throw (uno::RuntimeException)
{
    return text::XTextRange::static_type();
}

/* -----------------------------07.03.01 15:43--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SAL_CALL SwXTextCursor::hasElements() throw (uno::RuntimeException)
{
    return sal_True;
}

/* -----------------------------03.04.00 09:11--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SAL_CALL
SwXTextCursor::getAvailableServiceNames() throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString::createFromAscii("com.sun.star.text.TextContent");
    return aRet;
}

// ---------------------------------------------------------------------------
IMPLEMENT_FORWARD_REFCOUNT( SwXTextCursor,SwXTextCursor_Base )

uno::Any SAL_CALL
SwXTextCursor::queryInterface(const uno::Type& rType)
throw (uno::RuntimeException)
{
    return (rType == lang::XUnoTunnel::static_type())
        ? OTextCursorHelper::queryInterface(rType)
        : SwXTextCursor_Base::queryInterface(rType);
}

