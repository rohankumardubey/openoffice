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
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <vcl/gdimtf.hxx>
#endif
#ifndef _IPOBJ_HXX
#include <bf_so3/ipobj.hxx>
#endif

#include <tools/poly.hxx>
#include <vcl/outdev.hxx>

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif


#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>			// fuer SwFmtChg
#endif
namespace binfilter {


/*N*/ SwNoTxtNode::SwNoTxtNode( const SwNodeIndex & rWhere,
/*N*/ 				  const BYTE nNdType,
/*N*/ 				  SwGrfFmtColl *pGrfColl,
/*N*/ 				  SwAttrSet* pAutoAttr ) :
/*N*/ 	SwCntntNode( rWhere, nNdType, pGrfColl ),
/*N*/     pContour( 0 ),
/*N*/     bAutomaticContour( FALSE ),
/*N*/ 	bContourMapModeValid( TRUE ),
/*N*/ 	bPixelContour( FALSE )
/*N*/ {
/*N*/ 	// soll eine Harte-Attributierung gesetzt werden?
/*N*/ 	if( pAutoAttr )
/*N*/ 		SetAttr( *pAutoAttr );
/*N*/ }


/*N*/ SwNoTxtNode::~SwNoTxtNode()
/*N*/ {
/*N*/ 	delete pContour;
/*N*/ }


// erzeugt fuer alle Ableitungen einen AttrSet mit Bereichen
// fuer Frame- und Grafik-Attributen
/*N*/ void SwNoTxtNode::NewAttrSet( SwAttrPool& rPool )
/*N*/ {
/*N*/ 	ASSERT( !pAttrSet, "AttrSet ist doch gesetzt" );
/*N*/ 	pAttrSet = new SwAttrSet( rPool, aNoTxtNodeSetRange );
/*N*/ 	pAttrSet->SetParent( &GetFmtColl()->GetAttrSet() );
/*N*/ }

// Dummies fuer das Laden/Speichern von persistenten Daten
// bei Grafiken und OLE-Objekten






/*N*/ void SwNoTxtNode::SetContour( const PolyPolygon *pPoly, BOOL bAutomatic )
/*N*/ {
/*N*/ 	delete pContour;
/*N*/ 	if ( pPoly )
/*?*/ 		pContour = new PolyPolygon( *pPoly );
/*N*/ 	else
/*N*/ 		pContour = 0;
/*N*/     bAutomaticContour = bAutomatic;
/*N*/ 	bContourMapModeValid = TRUE;
/*N*/ 	bPixelContour = FALSE;
/*N*/ }



/*N*/ const PolyPolygon *SwNoTxtNode::HasContour() const
/*N*/ {
/*N*/ 	if( !bContourMapModeValid )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 const MapMode aGrfMap( GetGraphic().GetPrefMapMode() );
/*N*/ 	}
/*N*/ 
/*N*/ 	return pContour;
/*N*/ }



/*N*/ BOOL SwNoTxtNode::GetContourAPI( PolyPolygon &rContour ) const
/*N*/ {
/*N*/ 	if( !pContour )
/*N*/ 		return FALSE;
/*N*/ 
/*?*/ 	rContour = *pContour;
/*?*/ 	if( bContourMapModeValid )
/*?*/ 	{
/*?*/       const MapMode aGrfMap( GetGraphic().GetPrefMapMode() );
 /*?*/      const MapMode aContourMap( MAP_100TH_MM );
 /*?*/      ASSERT( aGrfMap.GetMapUnit() != MAP_PIXEL ||
 /*?*/              aGrfMap == MapMode( MAP_PIXEL ),
 /*?*/                  "scale factor for pixel unsupported" );
 /*?*/      if( aGrfMap.GetMapUnit() != MAP_PIXEL &&
 /*?*/          aGrfMap != aContourMap )
 /*?*/      {
 /*?*/          USHORT nPolyCount = rContour.Count();
 /*?*/          for( USHORT j=0; j<nPolyCount; j++ )
 /*?*/          {
 /*?*/              Polygon& rPoly = (*pContour)[j];
 /*?*/ 
 /*?*/              USHORT nCount = rPoly.GetSize();
 /*?*/              for( USHORT i=0 ; i<nCount; i++ )
 /*?*/              {
 /*?*/                  rPoly[i] = OutputDevice::LogicToLogic( rPoly[i], aGrfMap,
 /*?*/                                                         aContourMap );
 /*?*/              }
 /*?*/          }
 /*?*/      }
/*?*/ 	}
/*?*/ 
/*?*/ 	return TRUE;
/*N*/ }

BOOL SwNoTxtNode::IsPixelContour() const
{
    BOOL bRet;
    if( bContourMapModeValid )
    {
        const MapMode aGrfMap( GetGraphic().GetPrefMapMode() );
        bRet = aGrfMap.GetMapUnit() == MAP_PIXEL;
    }
    else
    {
        bRet = bPixelContour;
    }

    return bRet;
}


Graphic SwNoTxtNode::GetGraphic() const
{
    Graphic aRet;
    if ( GetGrfNode() )
    {
        ((SwGrfNode*)this)->SwapIn( TRUE );
        aRet = ((SwGrfNode*)this)->GetGrf();
    }
    else
    {
        ASSERT( GetOLENode(), "new type of Node?" );
        SvInPlaceObjectRef xObj( ((SwOLENode*)this)->GetOLEObj().GetOleRef() );
        GDIMetaFile aMtf;
        aRet = xObj->GetGDIMetaFile( aMtf );
    }
    return aRet;
}


/*N*/ void SwNoTxtNode::SetAlternateText( const String& rTxt, sal_Bool bBroadcast )
/*N*/ {
/*N*/ 	if( bBroadcast )
/*N*/ 	{
/*?*/ 		SwStringMsgPoolItem aOld( RES_ALT_TEXT_CHANGED, aAlternateText );
/*?*/ 		SwStringMsgPoolItem aNew( RES_ALT_TEXT_CHANGED, rTxt );
/*?*/ 		aAlternateText = rTxt;
/*?*/ 		Modify( &aOld, &aNew );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aAlternateText = rTxt;
/*N*/ 	}
/*N*/ }

}
