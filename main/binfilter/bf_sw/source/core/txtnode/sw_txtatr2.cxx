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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _TXTINET_HXX //autogen
#include <txtinet.hxx>
#endif
#ifndef _TXTATR_HXX //autogen
#include <txtatr.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>        // SwCharFmt, SwTxtNode
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>        // SwCharFmt, SwUpdateAttr
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>		// RES_POOLCHR_INET_...
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>			// SwDoc
#endif
namespace binfilter {


/*************************************************************************
 *						class SwTxtHardBlank
 *************************************************************************/



/*************************************************************************
 *						class SwTxtCharFmt
 *************************************************************************/

/*N*/ SwTxtCharFmt::SwTxtCharFmt( const SwFmtCharFmt& rAttr,
/*N*/ 					xub_StrLen nStart, xub_StrLen nEnd )
/*N*/ 	: SwTxtAttrEnd( rAttr, nStart, nEnd ),
/*N*/ 	pMyTxtNd( 0 )
/*N*/ {
/*N*/ 	((SwFmtCharFmt&)rAttr).pTxtAttr = this;
/*N*/ 	SetCharFmtAttr( TRUE );
/*N*/ }

/*N*/ SwTxtCharFmt::~SwTxtCharFmt( )
/*N*/ {
/*N*/ }

/*N*/ void SwTxtCharFmt::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
/*N*/ {
/*N*/ 	USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
/*N*/ #ifdef DBG_UTIL
/*N*/ 	if ( (nWhich<RES_CHRATR_BEGIN || nWhich>RES_CHRATR_END)
/*N*/ 			&& (nWhich!=RES_OBJECTDYING)
/*N*/ 			&& (nWhich!=RES_ATTRSET_CHG)
/*N*/ 			&& (nWhich!=RES_FMT_CHG) )
/*N*/ 		ASSERT(!this, "SwTxtCharFmt::Modify(): unbekanntes Modify!");
/*N*/ #endif
/*N*/ 
/*N*/ 	if( pMyTxtNd )
/*N*/ 	{
/*N*/ 		SwUpdateAttr aUpdateAttr( *GetStart(), *GetEnd(), nWhich );
/*N*/ 		pMyTxtNd->SwCntntNode::Modify( &aUpdateAttr, &aUpdateAttr );
/*N*/ 	}
/*N*/ }

	// erfrage vom Modify Informationen
/*N*/ BOOL SwTxtCharFmt::GetInfo( SfxPoolItem& rInfo ) const
/*N*/ {
/*N*/ 	if( RES_AUTOFMT_DOCNODE != rInfo.Which() || !pMyTxtNd ||
/*N*/ 		&pMyTxtNd->GetNodes() != ((SwAutoFmtGetDocNode&)rInfo).pNodes )
/*N*/ 		return TRUE;
/*N*/ 
/*N*/ 	((SwAutoFmtGetDocNode&)rInfo).pCntntNode = pMyTxtNd;
/*N*/ 	return FALSE;
/*N*/ }

/*************************************************************************
 *						class SwTxtINetFmt
 *************************************************************************/

/*N*/ SwTxtINetFmt::SwTxtINetFmt( const SwFmtINetFmt& rAttr,
/*N*/ 							xub_StrLen nStart, xub_StrLen nEnd )
/*N*/ 	: SwTxtAttrEnd( rAttr, nStart, nEnd ),
/*N*/ 	SwClient( 0 ),
/*N*/     pMyTxtNd( 0 )
/*N*/ {
/*N*/ 	bValidVis = FALSE;
/*N*/ 	((SwFmtINetFmt&)rAttr).pTxtAttr  = this;
/*N*/ 	SetCharFmtAttr( TRUE );
/*N*/ }

/*N*/ SwTxtINetFmt::~SwTxtINetFmt( )
/*N*/ {
/*N*/ }

/*N*/ SwCharFmt* SwTxtINetFmt::GetCharFmt()
/*N*/ {
/*N*/ 	const SwFmtINetFmt& rFmt = SwTxtAttrEnd::GetINetFmt();
/*N*/ 	SwCharFmt* pRet = NULL;
/*N*/ 
/*N*/ 	if( rFmt.GetValue().Len() )
/*N*/ 	{
/*N*/ 		const SwDoc* pDoc = GetTxtNode().GetDoc();
/*N*/ 		if( !IsValidVis() )
/*N*/ 		{
/*N*/ 			SetVisited( pDoc->IsVisitedURL( rFmt.GetValue() ) );
/*N*/ 			SetValidVis( TRUE );
/*N*/ 		}
/*N*/ 		USHORT nId;
/*N*/ 		const String& rStr = IsVisited() ? rFmt.GetVisitedFmt()
/*N*/ 										   : rFmt.GetINetFmt();
/*N*/ 		if( rStr.Len() )
/*N*/ 			nId = IsVisited() ? rFmt.GetVisitedFmtId() : rFmt.GetINetFmtId();
/*N*/ 		else
/*N*/ 			nId = IsVisited() ? RES_POOLCHR_INET_VISIT : RES_POOLCHR_INET_NORMAL;
/*N*/ 
/*N*/ 		// JP 10.02.2000, Bug 72806: dont modify the doc for getting the
/*N*/ 		//		correct charstyle.
/*N*/ 		BOOL bResetMod = !pDoc->IsModified();
/*N*/ 		Link aOle2Lnk;
/*N*/ 		if( bResetMod )
/*N*/ 		{
/*N*/ 			aOle2Lnk = pDoc->GetOle2Link();
/*N*/ 			((SwDoc*)pDoc)->SetOle2Link( Link() );
/*N*/ 		}
/*N*/ 
/*N*/ 		pRet = IsPoolUserFmt( nId )
/*N*/ 				? ((SwDoc*)pDoc)->FindCharFmtByName( rStr )
/*N*/ 				: ((SwDoc*)pDoc)->GetCharFmtFromPool( nId );
/*N*/ 
/*N*/ 		if( bResetMod )
/*N*/ 		{
/*N*/ 			((SwDoc*)pDoc)->ResetModified();
/*N*/ 			((SwDoc*)pDoc)->SetOle2Link( aOle2Lnk );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pRet )
/*N*/ 		pRet->Add( this );
/*N*/ 	else if( GetRegisteredIn() )
/*N*/ 		pRegisteredIn->Remove( this );
/*N*/ 
/*N*/ 	return pRet;
/*N*/ }

/*N*/ void SwTxtINetFmt::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
/*N*/ {
/*N*/ 	USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
/*N*/ #ifdef DBG_UTIL
/*N*/ 	if ( (nWhich<RES_CHRATR_BEGIN || nWhich>RES_CHRATR_END)
/*N*/ 			&& (nWhich!=RES_OBJECTDYING)
/*N*/ 			&& (nWhich!=RES_ATTRSET_CHG)
/*N*/ 			&& (nWhich!=RES_FMT_CHG) )
/*N*/ 		ASSERT(!this, "SwTxtCharFmt::Modify(): unbekanntes Modify!");
/*N*/ #endif
/*N*/ 
/*N*/ 	if( pMyTxtNd )
/*N*/ 	{
/*N*/ 		SwUpdateAttr aUpdateAttr( *GetStart(), *GetEnd(), nWhich );
/*N*/ 		pMyTxtNd->SwCntntNode::Modify( &aUpdateAttr, &aUpdateAttr );
/*N*/ 	}
/*N*/ }

	// erfrage vom Modify Informationen
/*N*/ BOOL SwTxtINetFmt::GetInfo( SfxPoolItem& rInfo ) const
/*N*/ {
/*N*/ 	if( RES_AUTOFMT_DOCNODE != rInfo.Which() || !pMyTxtNd ||
/*N*/ 		&pMyTxtNd->GetNodes() != ((SwAutoFmtGetDocNode&)rInfo).pNodes )
/*N*/ 		return TRUE;
/*N*/ 
/*N*/ 	((SwAutoFmtGetDocNode&)rInfo).pCntntNode = pMyTxtNd;
/*N*/ 	return FALSE;
/*N*/ }


// ATT_XNLCONTAINERITEM ******************************




// ******************************






// ******************************


}
