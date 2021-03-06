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

#ifndef _SVXLINKMGR_HXX
#include <bf_svx/linkmgr.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWSERV_HXX
#include <swserv.hxx>
#endif
#ifndef _SWBASLNK_HXX
#include <swbaslnk.hxx>
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif

#ifndef _SWERROR_H
#include <swerror.h>
#endif
namespace binfilter {

/*N*/ SV_IMPL_REF( SwServerObject )

/*N*/ SwServerObject::~SwServerObject()
/*N*/ {
/*N*/ }










/*N*/ BOOL SwServerObject::IsLinkInServer( const SwBaseLink* pChkLnk ) const
/*N*/ {
/*N*/ 	ULONG nSttNd = 0, nEndNd = 0;
/*N*/ 	xub_StrLen nStt, nEnd;
/*N*/ 	const SwNode* pNd = 0;
/*N*/ 	const SwNodes* pNds = 0;
/*N*/ 
/*N*/ 	switch( eType )
/*N*/ 	{
/*?*/ 	case BOOKMARK_SERVER:
/*?*/ 		if( CNTNT_TYPE.pBkmk->GetOtherPos() )
/*?*/ 		{
/*?*/ 			SwBookmark& rBkmk = *CNTNT_TYPE.pBkmk;
/*?*/ 			const SwPosition* pStt = &rBkmk.GetPos(),
/*?*/ 							* pEnd = rBkmk.GetOtherPos();
/*?*/ 			if( *pStt > *pEnd )
/*?*/ 			{
/*?*/ 				const SwPosition* pTmp = pStt;
/*?*/ 				pStt = pEnd;
/*?*/ 				pEnd = pTmp;
/*?*/ 			}
/*?*/ 
/*?*/ 			nSttNd = pStt->nNode.GetIndex();
/*?*/ 			nStt = pStt->nContent.GetIndex();
/*?*/ 			nEndNd = pEnd->nNode.GetIndex();
/*?*/ 			nEnd = pEnd->nContent.GetIndex();
/*?*/ 			pNds = &pStt->nNode.GetNodes();
/*?*/ 		}
/*?*/ 		break;
/*?*/ 
/*?*/ 	case TABLE_SERVER: 		pNd = CNTNT_TYPE.pTblNd;	break;
/*?*/ 	case SECTION_SERVER:	pNd = CNTNT_TYPE.pSectNd;	break;
/*?*/ 
/*N*/ 	case SECTION_SERVER+1:
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pNd )
/*N*/ 	{
/*N*/ 		nSttNd = pNd->GetIndex();
/*N*/ 		nEndNd = pNd->EndOfSectionIndex();
/*N*/ 		nStt = 0, nEnd = USHRT_MAX;
/*N*/ 		pNds = &pNd->GetNodes();
/*N*/ 	}
/*N*/ 
/*N*/ 	if( nSttNd && nEndNd )
/*N*/ 	{
/*N*/ 		// LinkManager besorgen:
/*N*/ 		const ::binfilter::SvBaseLinks& rLnks = pNds->GetDoc()->GetLinkManager().GetLinks();
/*N*/ 
/*N*/ // um Rekursionen zu Verhindern: ServerType umsetzen!
/*N*/ SwServerObject::ServerModes eSave = eType;
/*N*/ if( !pChkLnk )
/*N*/ // sowas sollte man nicht tun, wer weiss schon, wie gross ein enum ist
/*N*/ // ICC nimmt keinen int
/*N*/ // #41723#
/*N*/ //	*((int*)&eType) = SECTION_SERVER+1;
/*N*/ 	((SwServerObject*)this)->eType = NONE_SERVER;
/*N*/ 		for( USHORT n = rLnks.Count(); n; )
/*N*/ 		{
/*N*/ 			const ::binfilter::SvBaseLink* pLnk = &(*rLnks[ --n ]);
/*N*/ 			if( pLnk && OBJECT_CLIENT_GRF != pLnk->GetObjType() &&
/*N*/ 				pLnk->ISA( SwBaseLink ) &&
/*N*/ 				!((SwBaseLink*)pLnk)->IsNoDataFlag() &&
/*N*/ 				((SwBaseLink*)pLnk)->IsInRange( nSttNd, nEndNd, nStt, nEnd ))
/*N*/ 			{
/*N*/ 				if( pChkLnk )
/*N*/ 				{
/*N*/ 					if( pLnk == pChkLnk ||
/*N*/ 						((SwBaseLink*)pLnk)->IsRecursion( pChkLnk ) )
/*N*/ 						return TRUE;
/*N*/ 				}
/*N*/ 				else if( ((SwBaseLink*)pLnk)->IsRecursion( (SwBaseLink*)pLnk ) )
/*N*/ 					((SwBaseLink*)pLnk)->SetNoDataFlag();
/*N*/ 			}
/*N*/ 		}
/*N*/ if( !pChkLnk )
/*N*/ 	//	*((int*)&eType) = eSave;
/*?*/ 	((SwServerObject*)this)->eType = eSave;
/*N*/ 	}
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }

/*  */


/*N*/ SwDataChanged::SwDataChanged( const SwPaM& rPam, USHORT nTyp )
/*N*/ 	: pPam( &rPam ), nType( nTyp ), pDoc( rPam.GetDoc() ), pPos( 0 )
/*N*/ {
/*N*/ 	nNode = rPam.GetPoint()->nNode.GetIndex();
/*N*/ 	nCntnt = rPam.GetPoint()->nContent.GetIndex();
/*N*/ }
 

/*N*/ SwDataChanged::SwDataChanged( SwDoc* pDoc, const SwPosition& rPos, USHORT nTyp )
/*N*/ 	: pPam( 0 ), nType( nTyp ), pDoc( pDoc ), pPos( &rPos )
/*N*/ {
/*N*/ 	nNode = rPos.nNode.GetIndex();
/*N*/ 	nCntnt = rPos.nContent.GetIndex();
/*N*/ }
/*N*/ 

/*N*/ SwDataChanged::~SwDataChanged()
/*N*/ {
/*N*/ 	// JP 09.04.96: nur wenn das Layout vorhanden ist ( also waehrend der
/*N*/ 	//				Eingabe)
/*N*/ 	if( pDoc->GetRootFrm() )
/*N*/ 	{
/*N*/ 		const ::binfilter::SvLinkSources& rServers = pDoc->GetLinkManager().GetServers();
/*N*/ 
/*N*/ 		for( USHORT nCnt = rServers.Count(); nCnt; )
/*N*/ 		{
/*N*/ 			::binfilter::SvLinkSourceRef refObj( rServers[ --nCnt ] );
/*N*/ 			// noch jemand am Object interessiert ?
/*?*/ 			if( refObj->HasDataLinks() && refObj->ISA( SwServerObject ))
/*?*/ 			{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	SwServerObject& rObj = *(SwServerObject*)&refObj;
/*?*/ 			}
/*?*/ 
/*?*/ 			// sollte jetzt gar keine Verbindung mehr bestehen
/*?*/ 			if( !refObj->HasDataLinks() )
/*?*/ 			{
/*?*/ 				// dann raus aus der Liste (Object bleibt aber bestehen!)
/*?*/ 				// falls es noch da ist !!
/*?*/ 				if( nCnt < rServers.Count() && &refObj == rServers[ nCnt ] )
/*?*/ 					pDoc->GetLinkManager().RemoveServer( nCnt, 1 );
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ }




}
