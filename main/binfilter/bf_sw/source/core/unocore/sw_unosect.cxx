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




#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _COM_SUN_STAR_TEXT_SECTIONFILELINK_HPP_
#include <com/sun/star/text/SectionFileLink.hpp>
#endif

#include <cmdid.h>
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <bf_svtools/urihelper.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <bf_svx/brshitem.hxx>
#endif
#ifndef _SVX_XMLCNITEM_HXX
#include <bf_svx/xmlcnitm.hxx>
#endif
#ifndef _LINKMGR_HXX
#include <bf_so3/linkmgr.hxx>
#endif
#ifndef _LNKBASE_HXX
#include <bf_so3/lnkbase.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOREDLINE_HXX
#include <unoredline.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif
#ifndef _SECTION_HXX //autogen
#include <section.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _HINTS_HXX //autogen
#include <hints.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _UNOIDX_HXX
#include <unoidx.hxx>
#endif
#ifndef _DOCTXM_HXX
#include <doctxm.hxx>
#endif
#ifndef _FMTFTNTX_HXX
#include <fmtftntx.hxx>
#endif
#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <bf_svx/frmdiritem.hxx>
#endif
/* #109700# */
#ifndef _SVX_LRSPITEM_HXX
#include <bf_svx/lrspitem.hxx>
#endif
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::rtl;

/******************************************************************
 *
 ******************************************************************/
TYPEINIT1(SwXTextSection, SwClient);
struct SwTextSectionProperties_Impl
{

    String  sCondition;
    String  sLinkFileName;
    String  sSectionFilter;
    String  sSectionRegion;
    uno::Sequence<sal_Int8> aPassword;

    SwFmtCol*   pColItem;
    SvxBrushItem* pBrushItem;
    SwFmtFtnAtTxtEnd* pFtnItem;
    SwFmtEndAtTxtEnd* pEndItem;
	SvXMLAttrContainerItem *pXMLAttr;
    SwFmtNoBalancedColumns *pNoBalanceItem;
    SvxFrameDirectionItem *pFrameDirItem;
    SvxLRSpaceItem *pLRSpaceItem; // #109700#
    sal_Bool    bDDE;
    sal_Bool    bHidden;
    sal_Bool    bCondHidden;
    sal_Bool    bProtect;
    sal_Bool    bUpdateType;

    SwTextSectionProperties_Impl() :
        bDDE(0),
        bHidden(0),
        bProtect(0),
        bCondHidden(0),
        pColItem(0),
        pBrushItem(0),
        pFtnItem(0),
        pEndItem(0),
		pXMLAttr(0),
        pNoBalanceItem(0),
        pFrameDirItem(0),
        pLRSpaceItem(0), // #109700#
        bUpdateType(sal_True){}

    ~SwTextSectionProperties_Impl()
    {
        delete pColItem;
        delete pBrushItem;
        delete pFtnItem;
        delete pEndItem;
		delete pXMLAttr;
        delete pNoBalanceItem;
        delete pFrameDirItem;
        delete pLRSpaceItem; // #109700#
    }
};
/* -----------------------------11.07.00 12:10--------------------------------

 ---------------------------------------------------------------------------*/
SwXTextSection* SwXTextSection::GetImplementation(Reference< XInterface> xRef )
{
    uno::Reference<lang::XUnoTunnel> xTunnel( xRef, uno::UNO_QUERY);
    if(xTunnel.is())
        return (SwXTextSection*)xTunnel->getSomething(SwXTextSection::getUnoTunnelId());
    return 0;
}

/* -----------------------------13.03.00 12:15--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXTextSection::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXTextSection::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}
/*-- 10.12.98 14:47:05---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextSection::SwXTextSection(SwSectionFmt* pFmt, BOOL bIndexHeader) :
        SwClient(pFmt),
        aLstnrCntnr( (text::XTextContent*)this),
        aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_SECTION)),
//          _pMap(aSwMapProvider.getPropertyMap(PROPERTY_MAP_SECTION)),
        m_bIsDescriptor(pFmt == 0),
        m_bIndexHeader(bIndexHeader),
        pProps(pFmt ? 0 : new SwTextSectionProperties_Impl())
{

}
/*-- 10.12.98 14:47:07---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextSection::~SwXTextSection()
{
    delete pProps;
}
/*-- 10.12.98 14:47:08---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextSection >  SwXTextSection::getParentSection(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextSection >  aRef;
    SwSectionFmt*  pSectFmt = GetFmt();
    if(pSectFmt)
    {
        SwSectionFmt* pParentFmt = pSectFmt->GetParent();
        if(pParentFmt)
        {
            SwXTextSection* pxSect = (SwXTextSection*)SwClientIter(*pParentFmt).
                                                        First(TYPE(SwXTextSection));
            if(pxSect)
                aRef = pxSect;
            else
                aRef = new SwXTextSection(pParentFmt);
        }
    }
    else
        throw uno::RuntimeException();
    return aRef;
}
/*-- 10.12.98 14:47:08---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< uno::Reference< text::XTextSection >  > SwXTextSection::getChildSections(void)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Sequence<uno::Reference< text::XTextSection > > aSeq;
    SwSectionFmt*  pSectFmt = GetFmt();
    if(pSectFmt)
    {
        SwSections aChildren;
        pSectFmt->GetChildSections(aChildren, SORTSECT_NOT, sal_False);
        aSeq.realloc(aChildren.Count());
        uno::Reference< text::XTextSection > * pArray = aSeq.getArray();
        for(sal_uInt16 i = 0; i < aChildren.Count(); i++)
        {
            SwSectionFmt* pChild = aChildren.GetObject(i)->GetFmt();
            SwXTextSection* pxSect = (SwXTextSection*)SwClientIter(*pChild).
                                                        First(TYPE(SwXTextSection));
            if(pxSect)
                pArray[i] = pxSect;
            else
                pArray[i] = new SwXTextSection(pChild);
        }
    }
    return aSeq;

}
/* -----------------18.02.99 13:31-------------------
 *
 * --------------------------------------------------*/
void SwXTextSection::attachToRange(const uno::Reference< text::XTextRange > & xTextRange)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    if(!m_bIsDescriptor)
        throw uno::RuntimeException();

    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                SwXTextRange::getUnoTunnelId());
        pCursor = (OTextCursorHelper*)xRangeTunnel->getSomething(
                                OTextCursorHelper::getUnoTunnelId());
    }

    SwDoc* pDoc = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ? (SwDoc*)pCursor->GetDoc() : 0;
    if(pDoc)
    {
        SwUnoInternalPaM aPam(*pDoc);
        //das muss jetzt sal_True liefern
        SwXTextRange::XTextRangeToSwPaM(aPam, xTextRange);
        UnoActionContext aCont(pDoc);

        sal_Bool bRet = sal_False;

        SwSection* pRet = 0;
        if(!m_sName.Len())
            m_sName =  C2S("TextSection");
        SectionType eType =  pProps->bDDE ? DDE_LINK_SECTION :
            pProps->sLinkFileName.Len() || pProps->sSectionRegion.Len() ?  FILE_LINK_SECTION :
                                                                CONTENT_SECTION;
        // index header section?
        if(m_bIndexHeader)
        {
            // caller wants an index header section, but will only
            // give him one if a) we are inside an index, and b) said
            // index doesn't yet have a header section.
            const SwTOXBase* pBase = aPam.GetDoc()->GetCurTOX(
                                                    *aPam.Start() );

            // are we inside an index?
            if (pBase)
            {
                // get all child sections
                SwSections aSectionsArr;
                ((SwTOXBaseSection*)pBase)->GetFmt()->
                    GetChildSections(aSectionsArr);

                // and search for current header section
                sal_uInt16 nCount = aSectionsArr.Count();
                sal_Bool bHeaderPresent = sal_False;
                for(sal_uInt16 i = 0; i < nCount; i++)
                {
                    bHeaderPresent |=
                        (aSectionsArr[i]->GetType() == TOX_HEADER_SECTION);
                }
                if (! bHeaderPresent)
                {
                    eType = TOX_HEADER_SECTION;
                }
            }
        }

        SwSection aSect(eType, pDoc->GetUniqueSectionName(&m_sName));
        aSect.SetCondition(pProps->sCondition);
        String sLinkName(pProps->sLinkFileName);
        sLinkName += ::binfilter::cTokenSeperator;
        sLinkName += pProps->sSectionFilter;
        sLinkName += ::binfilter::cTokenSeperator;
        sLinkName += pProps->sSectionRegion;
        aSect.SetLinkFileName(sLinkName);

        aSect.SetHidden(pProps->bHidden);
        aSect.SetProtect(pProps->bProtect);
        SfxItemSet aSet(pDoc->GetAttrPool(),
                    RES_COL, RES_COL,
                    RES_BACKGROUND, RES_BACKGROUND,
                    RES_FTN_AT_TXTEND, RES_FRAMEDIR,
                    RES_LR_SPACE, RES_LR_SPACE, // #109700#
					RES_UNKNOWNATR_CONTAINER,RES_UNKNOWNATR_CONTAINER,
                    0);
            if(pProps->pBrushItem)
                aSet.Put(*pProps->pBrushItem);
            if(pProps->pColItem)
                aSet.Put(*pProps->pColItem);
            if(pProps->pFtnItem)
                aSet.Put(*pProps->pFtnItem);
            if(pProps->pEndItem)
                aSet.Put(*pProps->pEndItem);
            if(pProps->pXMLAttr)
                aSet.Put(*pProps->pXMLAttr);
            if(pProps->pNoBalanceItem)
                aSet.Put(*pProps->pNoBalanceItem);
            if(pProps->pFrameDirItem)
                aSet.Put(*pProps->pFrameDirItem);
            /* #109700# */
            if(pProps->pLRSpaceItem)
                aSet.Put(*pProps->pLRSpaceItem);

        // section password
        if (pProps->aPassword.getLength() > 0)
            aSect.SetPasswd(pProps->aPassword);

        pRet = pDoc->Insert( aPam, aSect, aSet.Count() ? &aSet : 0 );
        pRet->GetFmt()->Add(this);

        // #97450# XML import must hide sections depending on their old
        //         condition status
        if( pProps->sCondition.Len() != 0 )
            pRet->SetCondHidden(pProps->bCondHidden);

        // set update type if DDE link (and connect, if necessary)
        if (pProps->bDDE)
        {
            if (! pRet->IsConnected())
            {
                pRet->CreateLink(CREATE_CONNECT);
            }
            pRet->SetUpdateType(pProps->bUpdateType ? ::binfilter::LINKUPDATE_ALWAYS :
                                ::binfilter::LINKUPDATE_ONCALL);
        }

        DELETEZ(pProps);
        m_bIsDescriptor = sal_False;
    }
    else
        throw lang::IllegalArgumentException();
}
/*-- 10.12.98 14:47:09---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::attach(const uno::Reference< text::XTextRange > & xTextRange)
                    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
	vos::OGuard aGuard(Application::GetSolarMutex());
    attachToRange( xTextRange );
}

/*-- 10.12.98 14:47:09---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXTextSection::getAnchor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextRange >  xRet;
    SwSectionFmt*  pSectFmt = GetFmt();
    if(pSectFmt)
    {
        const SwSection* pSect;
        const SwNodeIndex* pIdx;
        if( 0 != ( pSect = pSectFmt->GetSection() ) &&
            0 != ( pIdx = pSectFmt->GetCntnt().GetCntntIdx() ) &&
            pIdx->GetNode().GetNodes().IsDocNodes() )
        {
            SwPaM aPaM(*pIdx);
            aPaM.Move( fnMoveForward, fnGoCntnt );

			const SwEndNode* pEndNode = pIdx->GetNode().EndOfSectionNode();
			SwPaM aEnd(*pEndNode);
            aEnd.Move( fnMoveBackward, fnGoCntnt );
            xRet = SwXTextRange::CreateTextRangeFromPosition(pSectFmt->GetDoc(),
                *aPaM.Start(), aEnd.Start());
        }
    }
    return xRet;
}
/*-- 10.12.98 14:47:09---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::dispose(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwSectionFmt*  pSectFmt = GetFmt();
    if(pSectFmt)
        pSectFmt->GetDoc()->DelSectionFmt( pSectFmt );
    else
        throw uno::RuntimeException();
}
/*-- 10.12.98 14:47:10---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::addEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/*-- 10.12.98 14:47:10---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}
/*-- 10.12.98 14:47:11---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo >  SwXTextSection::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = aPropSet.getPropertySetInfo();
    return aRef;
}
/* -----------------------------12.02.01 10:29--------------------------------

 ---------------------------------------------------------------------------*/
struct SwSectItemSet_Impl
{

    SfxItemSet* pItemSet;
    SwSectItemSet_Impl() :
        pItemSet(0){}
    ~SwSectItemSet_Impl()
        {delete pItemSet;}
};
/* -----------------------------12.02.01 10:45--------------------------------

 ---------------------------------------------------------------------------*/
void SwXTextSection::setPropertyValues(
    const Sequence< ::rtl::OUString >& rPropertyNames,
    const Sequence< Any >& rValues )
        throw(PropertyVetoException, lang::IllegalArgumentException,
                        lang::WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwSectionFmt*   pFmt = GetFmt();
    if(rPropertyNames.getLength() != rValues.getLength())
        throw IllegalArgumentException();
    if(pFmt || m_bIsDescriptor)
    {
        SwSection   aSection(CONTENT_SECTION, aEmptyStr);
        SwSection* pSect = pFmt ? pFmt->GetSection() : 0;
        if(pFmt)
            aSection = *pSect;
        const OUString* pPropertyNames = rPropertyNames.getConstArray();
        const Any* pValues = rValues.getConstArray();
        SwSectItemSet_Impl aItemSet;

        sal_Bool bLinkModeChanged = sal_False;
        sal_Bool bLinkMode;
        for(sal_Int16 nProperty = 0; nProperty < rPropertyNames.getLength(); nProperty++)
        {
            const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                    aPropSet.getPropertyMap(), pPropertyNames[nProperty]);
            if(pMap)
            {
			    if ( pMap->nFlags & PropertyAttribute::READONLY)
                    throw PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + pPropertyNames[nProperty], static_cast < cppu::OWeakObject * > ( this ) );
                switch(pMap->nWID)
                {
                    case WID_SECT_CONDITION:
                    {
                        OUString uTmp;
                        pValues[nProperty] >>= uTmp;
                        if(m_bIsDescriptor)
                            pProps->sCondition = String(uTmp);
                        else
                            aSection.SetCondition(uTmp);
                    }
                    break;
                    case WID_SECT_DDE_TYPE      :
                    case WID_SECT_DDE_FILE      :
                    case WID_SECT_DDE_ELEMENT   :
                    {
                        OUString uTmp;
                        pValues[nProperty] >>= uTmp;
                        String sTmp(uTmp);
                        if(m_bIsDescriptor)
                        {
                            if(!pProps->bDDE)
                            {
                                pProps->sLinkFileName = ::binfilter::cTokenSeperator;
                                pProps->sLinkFileName += ::binfilter::cTokenSeperator;
                                pProps->bDDE = sal_True;
                            }
                            pProps->sLinkFileName.SetToken(pMap->nWID - WID_SECT_DDE_TYPE,::binfilter::cTokenSeperator,sTmp);
                        }
                        else
                        {
                            String sLinkFileName(aSection.GetLinkFileName());
                            if(aSection.GetType() != DDE_LINK_SECTION)
                            {
                                sLinkFileName = ::binfilter::cTokenSeperator;
                                sLinkFileName += ::binfilter::cTokenSeperator;
                                aSection.SetType(DDE_LINK_SECTION);
                            }
                            sLinkFileName.SetToken(pMap->nWID - WID_SECT_DDE_TYPE,::binfilter::cTokenSeperator, sTmp);
                            aSection.SetLinkFileName(sLinkFileName);
                        }
                    }
                    break;
                    case WID_SECT_DDE_AUTOUPDATE:
                    {
                        sal_Bool bVal = *(sal_Bool*)pValues[nProperty].getValue();
                        if(m_bIsDescriptor)
                        {
                            pProps->bUpdateType = bVal;
                        }
                        else
                        {
                            bLinkModeChanged = sal_True;
                            bLinkMode = bVal;
                        }
                    }
                    break;
                    case WID_SECT_LINK     :
                    {
                        text::SectionFileLink aLink;
                        if(pValues[nProperty] >>= aLink)
                        {
                            if(m_bIsDescriptor)
                            {
                                pProps->bDDE = sal_False;
                                pProps->sLinkFileName = String(aLink.FileURL);
                                pProps->sSectionFilter = String(aLink.FilterName);
                            }
                            else
                            {
                                if(aSection.GetType() != FILE_LINK_SECTION &&
                                    aLink.FileURL.getLength())
                                    aSection.SetType(FILE_LINK_SECTION);
                                String sFileName;
                                if(aLink.FileURL.getLength())
                                    sFileName += ::binfilter::StaticBaseUrl::SmartRelToAbs( aLink.FileURL);
                                sFileName += ::binfilter::cTokenSeperator;
                                sFileName += String(aLink.FilterName);
                                sFileName += ::binfilter::cTokenSeperator;
                                sFileName += aSection.GetLinkFileName().GetToken( 2, ::binfilter::cTokenSeperator );
                                aSection.SetLinkFileName(sFileName);
                                if(sFileName.Len() < 3)
                                    aSection.SetType(CONTENT_SECTION);
                            }
                        }
                        else
                            throw lang::IllegalArgumentException();
                    }
                    break;
                    case WID_SECT_REGION :
                    {
                        OUString uTmp;
                        pValues[nProperty] >>= uTmp;
                        String sLink(uTmp);
                        if(m_bIsDescriptor)
                        {
                            pProps->bDDE = sal_False;
                            pProps->sSectionRegion = sLink;
                        }
                        else
                        {
                            if(aSection.GetType() != FILE_LINK_SECTION &&
                                    sLink.Len())
                                    aSection.SetType(FILE_LINK_SECTION);
                            String sSectLink(aSection.GetLinkFileName());
                            while( 3 < sSectLink.GetTokenCount( ::binfilter::cTokenSeperator ))
                            {
                                sSectLink += ::binfilter::cTokenSeperator;
                            }
                            sSectLink.SetToken(2, ::binfilter::cTokenSeperator, sLink);
                            aSection.SetLinkFileName(sSectLink);
                            if(sSectLink.Len() < 3)
                                aSection.SetType(CONTENT_SECTION);
                        }
                    }
                    break;
                    case WID_SECT_VISIBLE   :
                    {
                        sal_Bool bVal = *(sal_Bool*)pValues[nProperty].getValue();
                        if(m_bIsDescriptor)
                            pProps->bHidden = !bVal;
                        else
                            aSection.SetHidden(!bVal);
                    }
                    break;
                    case WID_SECT_CURRENTLY_VISIBLE:
                    {
                        sal_Bool bVal = *(sal_Bool*)pValues[nProperty].getValue();
                        if(m_bIsDescriptor)
                            pProps->bCondHidden = !bVal;
                        else
                            if( aSection.GetCondition().Len() != 0 )
                                aSection.SetCondHidden(!bVal);
                    }
                    break;
                    case WID_SECT_PROTECTED:
                    {
                        sal_Bool bVal = *(sal_Bool*)pValues[nProperty].getValue();
                        if(m_bIsDescriptor)
                            pProps->bProtect = bVal;
                        else
                            aSection.SetProtect(bVal);
                    }
                    break;
                    case WID_SECT_PASSWORD:
                    {
                        uno::Sequence<sal_Int8> aSeq;
                        pValues[nProperty] >>= aSeq;
                        if (m_bIsDescriptor)
                            pProps->aPassword = aSeq;
                        else
                            aSection.SetPasswd(aSeq);
                    }
                    break;
                    default:
                        if(pFmt)
                        {
                            const SfxItemSet& rOldAttrSet = pFmt->GetAttrSet();
                            aItemSet.pItemSet = new SfxItemSet(*rOldAttrSet.GetPool(),
                                                        pMap->nWID, pMap->nWID, 0);
                            aItemSet.pItemSet->Put(rOldAttrSet);
                            aPropSet.setPropertyValue(*pMap, pValues[nProperty], *aItemSet.pItemSet);
                        }
                        else
                        {
                            SfxPoolItem* pPutItem = 0;
                            if(RES_COL == pMap->nWID)
                            {
                                if(!pProps->pColItem)
                                    pProps->pColItem = new SwFmtCol;
                                    pPutItem = pProps->pColItem;
                            }
                            else if(RES_BACKGROUND == pMap->nWID)
                            {
                                if(!pProps->pBrushItem)
                                    pProps->pBrushItem = new SvxBrushItem;
                                pPutItem = pProps->pBrushItem;
                            }
                            else if(RES_FTN_AT_TXTEND == pMap->nWID)
                            {
                                if(!pProps->pFtnItem)
                                    pProps->pFtnItem = new SwFmtFtnAtTxtEnd;
                                pPutItem = pProps->pFtnItem;
                            }
                            else if(RES_END_AT_TXTEND == pMap->nWID)
                            {
                                if(!pProps->pEndItem)
                                    pProps->pEndItem = new SwFmtEndAtTxtEnd;
                                pPutItem = pProps->pEndItem;
                            }
                            else if(RES_UNKNOWNATR_CONTAINER== pMap->nWID)
                            {
                                if(!pProps->pXMLAttr)
                                    pProps->pXMLAttr= new SvXMLAttrContainerItem( RES_UNKNOWNATR_CONTAINER );
                                pPutItem = pProps->pXMLAttr;
                            }
                            else if(RES_COLUMNBALANCE== pMap->nWID)
                            {
                                if(!pProps->pNoBalanceItem)
                                    pProps->pNoBalanceItem= new SwFmtNoBalancedColumns( RES_COLUMNBALANCE );
                                pPutItem = pProps->pNoBalanceItem;
                            }
                            else if(RES_FRAMEDIR == pMap->nWID)
                            {
                                if(!pProps->pFrameDirItem)
                                    pProps->pFrameDirItem = new SvxFrameDirectionItem( FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR );
                                pPutItem = pProps->pFrameDirItem;
                            }
                            else if(RES_LR_SPACE == pMap->nWID)
                            {
                                // #109700#
                                if(!pProps->pLRSpaceItem)
                                    pProps->pLRSpaceItem = new SvxLRSpaceItem( RES_LR_SPACE );
                                pPutItem = pProps->pLRSpaceItem;
                            }
                            if(pPutItem)
                                pPutItem->PutValue(pValues[nProperty], pMap->nMemberId);
                        }

                }
            }
            else
        		throw RuntimeException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pPropertyNames[nProperty], static_cast < cppu::OWeakObject * > ( this ) );

        }
        if(pFmt)
        {
            SwDoc* pDoc = pFmt->GetDoc();
            const SwSectionFmts& rFmts = pDoc->GetSections();
            UnoActionContext aContext(pDoc);
            for( sal_uInt16 i = 0; i < rFmts.Count(); i++ )
            {
                if(rFmts[i]->GetSection()->GetName() == pSect->GetName())
                {
                    // --> OD 2006-07-14 #b6448029#
                    // always prevent update of an existing link in the section
                    pDoc->ChgSection( i, aSection, aItemSet.pItemSet, sal_True );
                    // <--

                    {
                        // temporarily remove actions to allow cursor update
                        UnoActionRemoveContext aRemoveContext( pDoc );
                    }

                    SwSection* pSect = pFmt->GetSection();
                    if( bLinkModeChanged && pSect->GetType() == DDE_LINK_SECTION)
                    {
                        // set update type; needs an established link
                        if(!pSect->IsConnected())
                        {
                            pSect->CreateLink(CREATE_CONNECT);
                        }
                        pSect->SetUpdateType(bLinkMode ? ::binfilter::LINKUPDATE_ALWAYS
                                                : ::binfilter::LINKUPDATE_ONCALL);
                    }
                    // section found and processed: break from loop
                    break;
                }
            }
        }
    }
    else
        throw uno::RuntimeException();

}
/*-- 10.12.98 14:47:11---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::setPropertyValue(
    const OUString& rPropertyName, const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException,
        lang::IllegalArgumentException,     lang::WrappedTargetException,
        uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Sequence< ::rtl::OUString > aPropertyNames(1);
    aPropertyNames.getArray()[0] = rPropertyName;
    Sequence< Any > aValues(1);
    aValues.getArray()[0] = aValue;
    setPropertyValues(aPropertyNames, aValues);
}
/* -----------------------------12.02.01 10:43--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< Any > SwXTextSection::getPropertyValues(
    const Sequence< ::rtl::OUString >& rPropertyNames )
        throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Sequence< Any > aRet(rPropertyNames.getLength());
    Any* pRet = aRet.getArray();
    SwSectionFmt*   pFmt = GetFmt();
    if(pFmt||m_bIsDescriptor)
    {
        SwSection* pSect = pFmt ? pFmt->GetSection() : 0;
        const OUString* pPropertyNames = rPropertyNames.getConstArray();
        for(sal_Int32 nProperty = 0; nProperty < rPropertyNames.getLength(); nProperty++)
        {
            const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                aPropSet.getPropertyMap(), pPropertyNames[nProperty]);
            if(pMap)
            {
                switch(pMap->nWID)
                {
                    case WID_SECT_CONDITION:
                    {
                        OUString uTmp(
                            m_bIsDescriptor ? pProps->sCondition : pSect->GetCondition());
                        pRet[nProperty] <<= uTmp;
                    }
                    break;
                    case WID_SECT_DDE_TYPE      :
                    case WID_SECT_DDE_FILE      :
                    case WID_SECT_DDE_ELEMENT   :
                    {
                        String sRet;
                        if(m_bIsDescriptor)
                        {
                            if(pProps->bDDE)
                                sRet = pProps->sLinkFileName;
                        }
                        else if( DDE_LINK_SECTION == pSect->GetType() )
                        {
                            sRet = pSect->GetLinkFileName();
                        }
                        sRet = sRet.GetToken(pMap->nWID - WID_SECT_DDE_TYPE, ::binfilter::cTokenSeperator);
                        pRet[nProperty] <<= OUString(sRet);
                    }
                    break;
                    case WID_SECT_DDE_AUTOUPDATE:
                    {
                        // GetUpdateType() returns .._ALWAYS or .._ONCALL
						if ( pSect->IsLinkType() && pSect->IsConnected() )
						{
							sal_Bool bTemp =
								(pSect->GetUpdateType() == ::binfilter::LINKUPDATE_ALWAYS);
							pRet[nProperty].setValue( &bTemp, ::getCppuBooleanType());
						}
                    }
                    break;
                    case WID_SECT_LINK     :
                    {
                        text::SectionFileLink aLink;
                        if(m_bIsDescriptor)
                        {
                            if(!pProps->bDDE)
                            {
                                aLink.FileURL = pProps->sLinkFileName;
                                aLink.FilterName = pProps->sSectionFilter;
                            }
                        }
                        else if( FILE_LINK_SECTION == pSect->GetType() )
                        {
                            String sRet( pSect->GetLinkFileName() );
                            aLink.FileURL = sRet.GetToken(0, ::binfilter::cTokenSeperator );
                            aLink.FilterName = sRet.GetToken(1, ::binfilter::cTokenSeperator );
                        }
                        pRet[nProperty].setValue(&aLink, ::getCppuType((text::SectionFileLink*)0));
                    }
                    break;
                    case WID_SECT_REGION :
                    {
                        String sRet;
                        if(m_bIsDescriptor)
                        {
                            sRet = pProps->sSectionRegion;
                        }
                        else if( FILE_LINK_SECTION == pSect->GetType() )
                            sRet = pSect->GetLinkFileName().GetToken(2, ::binfilter::cTokenSeperator);
                        pRet[nProperty] <<= OUString(sRet);
                    }
                    break;
                    case WID_SECT_VISIBLE   :
                    {
                        sal_Bool bTemp = m_bIsDescriptor ? !pProps->bHidden : !pSect->IsHidden();
                        pRet[nProperty].setValue( &bTemp, ::getCppuBooleanType());
                    }
                    break;
                    case WID_SECT_CURRENTLY_VISIBLE:
                    {
                        sal_Bool bTmp = m_bIsDescriptor ? !pProps->bCondHidden : !pSect->IsCondHidden();
                        pRet[nProperty].setValue( &bTmp, ::getCppuBooleanType());
                    }
                    break;
                    case WID_SECT_PROTECTED:
                    {
                        sal_Bool bTemp = m_bIsDescriptor ? pProps->bProtect : pSect->IsProtect();
                        pRet[nProperty].setValue( &bTemp, ::getCppuBooleanType());
                    }
                    break;
                    case  FN_PARAM_LINK_DISPLAY_NAME:
                    {
                        if(pFmt)
                            pRet[nProperty] <<= OUString(pFmt->GetSection()->GetName());
                    }
                    break;
                    case WID_SECT_DOCUMENT_INDEX:
                    {
                        // search enclosing index
                        SwSection* pEnclosingSection = pSect;
                        while ( (pEnclosingSection != NULL) &&
                                (TOX_CONTENT_SECTION !=
                                pEnclosingSection->GetType()) )
                        {
                            pEnclosingSection = pEnclosingSection->GetParent();
                        }
                        if (pEnclosingSection)
                        {
                            // convert section to TOXBase and get SwXDocumentIndex
                            SwTOXBaseSection* pTOXBaseSect =
                                PTR_CAST(SwTOXBaseSection, pEnclosingSection);
                            Reference<XDocumentIndex> xIndex =
                                SwXDocumentIndexes::GetObject(pTOXBaseSect);
                            pRet[nProperty] <<= xIndex;
                        }
                        // else: no enclosing index found -> empty return value
                    }
                    break;
                    case WID_SECT_IS_GLOBAL_DOC_SECTION:
                    {
                        sal_Bool bRet = (NULL == pFmt) ? sal_False :
                                        (NULL != pFmt->GetGlobalDocSection());
                        pRet[nProperty].setValue( &bRet, ::getCppuBooleanType());
                    }
                    break;
                    case  FN_UNO_ANCHOR_TYPES:
                    case  FN_UNO_TEXT_WRAP:
                    case  FN_UNO_ANCHOR_TYPE:
                        SwXParagraph::getDefaultTextContentValue(pRet[nProperty], OUString(), pMap->nWID);
                    break;
                    case FN_UNO_REDLINE_NODE_START:
                    case FN_UNO_REDLINE_NODE_END:
                    {
                        SwNode* pSectNode = pFmt->GetSectionNode();
                        if(FN_UNO_REDLINE_NODE_END == pMap->nWID)
                            pSectNode = pSectNode->EndOfSectionNode();
                        const SwRedlineTbl& rRedTbl = pFmt->GetDoc()->GetRedlineTbl();
                        for(USHORT nRed = 0; nRed < rRedTbl.Count(); nRed++)
                        {
                            const SwRedline* pRedline = rRedTbl[nRed];
                            const SwNode* pRedPointNode = pRedline->GetNode(TRUE);
                            const SwNode* pRedMarkNode = pRedline->GetNode(FALSE);
                            if(pRedPointNode == pSectNode || pRedMarkNode == pSectNode)
                            {
                                const SwNode* pStartOfRedline = SwNodeIndex(*pRedPointNode) <= SwNodeIndex(*pRedMarkNode) ?
                                    pRedPointNode : pRedMarkNode;
                                BOOL bIsStart = pStartOfRedline == pSectNode;
                                pRet[nProperty] <<= SwXRedlinePortion::CreateRedlineProperties(*pRedline, bIsStart);
                                break;
                            }
                        }
                    }
                    break;
                    case WID_SECT_PASSWORD:
                    {
                        pRet[nProperty] <<= m_bIsDescriptor ? pProps->aPassword : pSect->GetPasswd();
                    }
                    break;
                    default:
                        if(pFmt)
                            pRet[nProperty] = aPropSet.getPropertyValue(*pMap, pFmt->GetAttrSet());
                        else
                        {
                            const SfxPoolItem* pQueryItem = 0;
                            if(RES_COL == pMap->nWID)
                            {
                                if(!pProps->pColItem)
                                    pProps->pColItem = new SwFmtCol;
                                    pQueryItem = pProps->pColItem;
                            }
                            else if(RES_BACKGROUND == pMap->nWID)
                            {
                                if(!pProps->pBrushItem)
                                    pProps->pBrushItem = new SvxBrushItem;
                                pQueryItem = pProps->pBrushItem;
                            }
                            else if(RES_FTN_AT_TXTEND == pMap->nWID)
                            {
                                if(!pProps->pFtnItem)
                                    pProps->pFtnItem = new SwFmtFtnAtTxtEnd;
                                pQueryItem = pProps->pFtnItem;
                            }
                            else if(RES_END_AT_TXTEND == pMap->nWID)
                            {
                                if(!pProps->pEndItem)
                                    pProps->pEndItem = new SwFmtEndAtTxtEnd;
                                pQueryItem = pProps->pEndItem;
                            }
                            else if(RES_UNKNOWNATR_CONTAINER== pMap->nWID)
                            {
                                if(!pProps->pXMLAttr)
                                    pProps->pXMLAttr= new SvXMLAttrContainerItem ;
                                pQueryItem = pProps->pXMLAttr;
                            }
                            else if(RES_COLUMNBALANCE== pMap->nWID)
                            {
                                if(!pProps->pNoBalanceItem)
                                    pProps->pNoBalanceItem= new SwFmtNoBalancedColumns;
                                pQueryItem = pProps->pNoBalanceItem;
                            }
                            else if(RES_FRAMEDIR == pMap->nWID)
                            {
                                if(!pProps->pFrameDirItem)
                                    pProps->pFrameDirItem = new SvxFrameDirectionItem;
                                pQueryItem = pProps->pFrameDirItem;
                            }
                            /* -> #109700# */
                            else if(RES_LR_SPACE == pMap->nWID)
                            {
                                if(!pProps->pLRSpaceItem)
                                    pProps->pLRSpaceItem = new SvxLRSpaceItem;
                                pQueryItem = pProps->pLRSpaceItem;
                            }
                            /* <- #109700# */
                            if(pQueryItem)
                                pQueryItem->QueryValue(pRet[nProperty], pMap->nMemberId);
                        }
                }
            }
            else
		        throw RuntimeException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pPropertyNames[nProperty], static_cast < cppu::OWeakObject * > ( this ) );
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 10.12.98 14:47:12---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextSection::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Sequence< ::rtl::OUString > aPropertyNames(1);
    aPropertyNames.getArray()[0] = rPropertyName;
    return getPropertyValues(aPropertyNames).getConstArray()[0];
}
/* -----------------------------12.02.01 10:30--------------------------------

 ---------------------------------------------------------------------------*/
void SwXTextSection::addPropertiesChangeListener(
    const Sequence< ::rtl::OUString >& aPropertyNames,
    const Reference< XPropertiesChangeListener >& xListener ) throw(RuntimeException)
{
    DBG_WARNING("not implemented");
}
/* -----------------------------12.02.01 10:30--------------------------------

 ---------------------------------------------------------------------------*/
void SwXTextSection::removePropertiesChangeListener(
    const Reference< XPropertiesChangeListener >& xListener )
        throw(RuntimeException)
{
    DBG_WARNING("not implemented");
}
/* -----------------------------12.02.01 10:30--------------------------------

 ---------------------------------------------------------------------------*/
void SwXTextSection::firePropertiesChangeEvent(
    const Sequence< ::rtl::OUString >& aPropertyNames,
    const Reference< XPropertiesChangeListener >& xListener )
        throw(RuntimeException)
{
    DBG_WARNING("not implemented");
}
/*-- 10.12.98 14:47:13---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::addPropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 10.12.98 14:47:13---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::removePropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 10.12.98 14:47:14---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 10.12.98 14:47:14---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

/*-- 08.11.00 10:47:55---------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyState SwXTextSection::getPropertyState( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Sequence< OUString > aNames(1);
    aNames.getArray()[0] = rPropertyName;
    return getPropertyStates(aNames).getConstArray()[0];
}
/*-- 08.11.00 10:47:55---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< PropertyState > SwXTextSection::getPropertyStates(
    const Sequence< OUString >& rPropertyNames )
        throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Sequence< PropertyState > aStates(rPropertyNames.getLength());
    SwSectionFmt*   pFmt = GetFmt();
    if(pFmt||m_bIsDescriptor)
    {
        PropertyState* pStates = aStates.getArray();
        const OUString* pNames = rPropertyNames.getConstArray();
        for(sal_Int32 i = 0; i < rPropertyNames.getLength(); i++)
        {
            pStates[i] = PropertyState_DEFAULT_VALUE;
            const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                aPropSet.getPropertyMap(), pNames[i]);
            if(!pMap)
		        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pNames[i], static_cast < cppu::OWeakObject * > ( this ) );
            switch(pMap->nWID)
            {
                case WID_SECT_CONDITION:
                case WID_SECT_DDE_TYPE      :
                case WID_SECT_DDE_FILE      :
                case WID_SECT_DDE_ELEMENT   :
                case WID_SECT_DDE_AUTOUPDATE:
                case WID_SECT_LINK     :
                case WID_SECT_REGION :
                case WID_SECT_VISIBLE   :
                case WID_SECT_PROTECTED:
                case  FN_PARAM_LINK_DISPLAY_NAME:
                case  FN_UNO_ANCHOR_TYPES:
                case  FN_UNO_TEXT_WRAP:
                case  FN_UNO_ANCHOR_TYPE:
                    pStates[i] = PropertyState_DIRECT_VALUE;
                break;
                default:
                    if(pFmt)
                        pStates[i] = aPropSet.getPropertyState(pNames[i], pFmt->GetAttrSet());
                    else
                    {
                        const SfxPoolItem* pQueryItem = 0;
                        if(RES_COL == pMap->nWID)
                        {
                            if(!pProps->pColItem)
                                pStates[i] = PropertyState_DEFAULT_VALUE;
                            else
                                pStates[i] = PropertyState_DIRECT_VALUE;
                        }
                        else //if(RES_BACKGROUND == pMap->nWID)
                        {
                            if(!pProps->pBrushItem)
                                pStates[i] = PropertyState_DEFAULT_VALUE;
                            else
                                pStates[i] = PropertyState_DIRECT_VALUE;
                        }
                    }
            }
        }
    }
    else
        throw RuntimeException();
    return aStates;
}
/*-- 08.11.00 10:47:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::setPropertyToDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwSectionFmt*   pFmt = GetFmt();
    if(pFmt||m_bIsDescriptor)
    {
        SwSection   aSection(CONTENT_SECTION, aEmptyStr);
        SwSection* pSect = pFmt ? pFmt->GetSection() : 0;
        if(pFmt)
            aSection = *pSect;
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                aPropSet.getPropertyMap(), rPropertyName);
        if(!pMap)
        	throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
		if ( pMap->nFlags & PropertyAttribute::READONLY)
            throw PropertyVetoException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
        SfxItemSet* pNewAttrSet = 0;
        switch(pMap->nWID)
        {
            case WID_SECT_CONDITION:
            {
                if(m_bIsDescriptor)
                    pProps->sCondition = aEmptyStr;
                else
                    aSection.SetCondition(aEmptyStr);
            }
            break;
            case WID_SECT_DDE_TYPE      :
            case WID_SECT_DDE_FILE      :
            case WID_SECT_DDE_ELEMENT   :
            case WID_SECT_LINK     :
            case WID_SECT_REGION :
                aSection.SetType(CONTENT_SECTION);
            break;
            case WID_SECT_DDE_AUTOUPDATE:
                aSection.SetUpdateType(::binfilter::LINKUPDATE_ALWAYS);
            break;
            case WID_SECT_VISIBLE   :
            {
                if(m_bIsDescriptor)
                    pProps->bHidden = FALSE;
                else
                    aSection.SetHidden(FALSE);
            }
            break;
            case WID_SECT_PROTECTED:
            {
                if(m_bIsDescriptor)
                    pProps->bProtect = FALSE;
                else
                    aSection.SetProtect(FALSE);
            }
            break;
            case  FN_UNO_ANCHOR_TYPES:
            case  FN_UNO_TEXT_WRAP:
            case  FN_UNO_ANCHOR_TYPE:
            break;
            default:
                if(pMap->nWID <= SFX_WHICH_MAX)
                {
                    if(pFmt)
                    {
                        const SfxItemSet& rOldAttrSet = pFmt->GetAttrSet();
                        pNewAttrSet = new SfxItemSet(*rOldAttrSet.GetPool(),
                                                    pMap->nWID, pMap->nWID, 0);
                        pNewAttrSet->ClearItem(pMap->nWID);
                    }
                    else
                    {
                        if(RES_COL == pMap->nWID)
                            DELETEZ(pProps->pColItem);
                        else if(RES_BACKGROUND == pMap->nWID)
                            DELETEZ(pProps->pBrushItem);
                    }
                }
        }
        if(pFmt)
        {
            SwDoc* pDoc = pFmt->GetDoc();
            const SwSectionFmts& rFmts = pDoc->GetSections();
            UnoActionContext aContext(pDoc);
            for( sal_uInt16 i = 0; i < rFmts.Count(); i++ )
            {
                if(rFmts[i]->GetSection()->GetName() == pSect->GetName())
                {
                    // --> OD 2006-07-14 #b6448029#
                    // always prevent update of an existing link in the section
                    pDoc->ChgSection( i, aSection, pNewAttrSet, sal_True );
                    // <--

                    {
                        // temporarily remove actions to allow cursor update
                        UnoActionRemoveContext aRemoveContext( pDoc );
                    }

                    break;
                }
            }
            delete pNewAttrSet;
        }
    }
    else
        throw RuntimeException();
}
/*-- 08.11.00 10:47:56---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXTextSection::getPropertyDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    SwSectionFmt*   pFmt = GetFmt();
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                            aPropSet.getPropertyMap(), rPropertyName);
	if (!pMap)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    switch(pMap->nWID)
    {
        case WID_SECT_CONDITION:
        case WID_SECT_DDE_TYPE      :
        case WID_SECT_DDE_FILE      :
        case WID_SECT_DDE_ELEMENT   :
        case WID_SECT_REGION :
        case FN_PARAM_LINK_DISPLAY_NAME:
            aRet <<= OUString();
        break;
        case WID_SECT_LINK     :
            aRet <<= SectionFileLink();
        break;
        case WID_SECT_DDE_AUTOUPDATE:
        case WID_SECT_VISIBLE   :
        {
            sal_Bool bTemp = TRUE;
            aRet.setValue( &bTemp, ::getCppuBooleanType());
        }
        break;
        case WID_SECT_PROTECTED:
        {
            sal_Bool bTemp = FALSE;
            aRet.setValue( &bTemp, ::getCppuBooleanType());
        }
        break;
        case  FN_UNO_ANCHOR_TYPES:
        case  FN_UNO_TEXT_WRAP:
        case  FN_UNO_ANCHOR_TYPE:
            SwXParagraph::getDefaultTextContentValue(aRet, OUString(), pMap->nWID);
        break;
        default:
        if(pFmt && pMap->nWID <= SFX_WHICH_MAX)
        {
            SwDoc* pDoc = pFmt->GetDoc();
            const SfxPoolItem& rDefItem =
                pDoc->GetAttrPool().GetDefaultItem(pMap->nWID);
            rDefItem.QueryValue(aRet, pMap->nMemberId);
        }
    }
    return aRet;
}
/*-- 10.12.98 14:47:15---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXTextSection::getName(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    String sRet;
    const SwSectionFmt* pFmt = GetFmt();
    if(pFmt)
        sRet = pFmt->GetSection()->GetName();
    else if(m_bIsDescriptor)
        sRet = m_sName;
    else
        throw uno::RuntimeException();
    return sRet;
}
/*-- 10.12.98 14:47:16---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::setName(const OUString& rName) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwSectionFmt*   pFmt = GetFmt();
    if(pFmt)
    {
        SwSection   aSection(CONTENT_SECTION, aEmptyStr);
        SwSection* pSect = pFmt->GetSection();
        aSection = *pSect;
        String sNewName(rName);
        aSection.SetName(sNewName);

        const SwSectionFmts& rFmts = pFmt->GetDoc()->GetSections();
        sal_uInt16 nApplyPos = USHRT_MAX;
        for( sal_uInt16 i = 0; i < rFmts.Count(); i++ )
        {
            if(rFmts[i]->GetSection() == pSect)
                nApplyPos = i;
            else if(sNewName == rFmts[i]->GetSection()->GetName())
                throw uno::RuntimeException();
        }
        if(nApplyPos != USHRT_MAX)
        {
            {
                UnoActionContext aContext(pFmt->GetDoc());
                pFmt->GetDoc()->ChgSection( nApplyPos, aSection);
            }
            {
                // temporarily remove actions to allow cursor update
                UnoActionRemoveContext aRemoveContext( pFmt->GetDoc() );
            }
        }
    }
    else if(m_bIsDescriptor)
        m_sName = String(rName);
    else
        throw uno::RuntimeException();
}
/* -----------------02.11.99 11:30-------------------

 --------------------------------------------------*/
OUString SwXTextSection::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXTextSection");
}
/* -----------------02.11.99 11:30-------------------

 --------------------------------------------------*/
sal_Bool SwXTextSection::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return !rServiceName.compareToAscii("com.sun.star.text.TextSection") ||
                !rServiceName.compareToAscii("com.sun.star.document.LinkTarget") ||
                    !rServiceName.compareToAscii("com.sun.star.text.TextContent");
}
/* -----------------02.11.99 11:30-------------------

 --------------------------------------------------*/
uno::Sequence< OUString > SwXTextSection::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(3);
    OUString* pArr = aRet.getArray();
    pArr[0] = C2U("com.sun.star.text.TextSection");
    pArr[1] = C2U("com.sun.star.document.LinkTarget");
    pArr[2] = C2U("com.sun.star.text.TextContent");
    return aRet;
}

/*-- 10.12.98 14:42:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    if(pOld && pOld->Which() == RES_REMOVE_UNO_OBJECT &&
        (void*)GetRegisteredIn() == ((SwPtrMsgPoolItem *)pOld)->pObject )
            ((SwModify*)GetRegisteredIn())->Remove(this);
    else
        ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        aLstnrCntnr.Disposing();
}

}
