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

#ifndef _SVX_ULSPITEM_HXX //autogen
#include <bf_svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <bf_svx/lrspitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <bf_svx/fhgtitem.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>			// fuer GetAttrPool
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
namespace binfilter {


/*N*/ TYPEINIT1( SwTxtFmtColl, SwFmtColl );
/*N*/ TYPEINIT1( SwGrfFmtColl, SwFmtColl );
/*N*/ TYPEINIT1( SwConditionTxtFmtColl, SwTxtFmtColl );
/*N*/ TYPEINIT1( SwCollCondition, SwClient );

/*N*/ SV_IMPL_PTRARR( SwFmtCollConditions, SwCollConditionPtr );


/*
 * SwTxtFmtColl  TXT
 */

/*N*/ void SwTxtFmtColl::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
/*N*/ {
/*N*/ 	if( GetDoc()->IsInDtor() )
/*N*/ 	{
/*N*/ 		SwFmtColl::Modify( pOld, pNew );
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	int bNewParent = FALSE;
/*N*/ 	SvxULSpaceItem *pNewULSpace = 0, *pOldULSpace = 0;
/*N*/ 	SvxLRSpaceItem *pNewLRSpace = 0, *pOldLRSpace = 0;
/*N*/ 	SvxFontHeightItem* aFontSizeArr[3] = {0,0,0};
/*N*/ 
/*N*/ 	SwAttrSetChg *pNewChgSet = 0,  *pOldChgSet = 0;
/*N*/ 
/*N*/ 	switch( pOld ? pOld->Which() : pNew ? pNew->Which() : 0 )
/*N*/ 	{
/*N*/ 	case RES_ATTRSET_CHG:
/*N*/ 		// nur neu berechnen, wenn nicht wir der "Versender" sind !!!
/*N*/ 		pNewChgSet = (SwAttrSetChg*)pNew;
/*N*/ 		pOldChgSet = (SwAttrSetChg*)pOld;
/*N*/ 		pNewChgSet->GetChgSet()->GetItemState(
/*N*/ 			RES_LR_SPACE, FALSE, (const SfxPoolItem**)&pNewLRSpace );
/*N*/ 		pNewChgSet->GetChgSet()->GetItemState(
/*N*/ 			RES_UL_SPACE, FALSE, (const SfxPoolItem**)&pNewULSpace );
/*N*/ 		pNewChgSet->GetChgSet()->GetItemState( RES_CHRATR_FONTSIZE,
/*N*/ 						FALSE, (const SfxPoolItem**)&(aFontSizeArr[0]) );
/*N*/ 		pNewChgSet->GetChgSet()->GetItemState( RES_CHRATR_CJK_FONTSIZE,
/*N*/ 						FALSE, (const SfxPoolItem**)&(aFontSizeArr[1]) );
/*N*/ 		pNewChgSet->GetChgSet()->GetItemState( RES_CHRATR_CTL_FONTSIZE,
/*N*/ 						FALSE, (const SfxPoolItem**)&(aFontSizeArr[2]) );
/*N*/ 		break;
/*N*/ 
/*N*/ 	case RES_FMT_CHG:
/*N*/ 		if( GetAttrSet().GetParent() )
/*N*/ 		{
/*N*/ 			const SfxItemSet* pParent = GetAttrSet().GetParent();
/*N*/ 			pNewLRSpace = (SvxLRSpaceItem*)&pParent->Get( RES_LR_SPACE );
/*N*/ 			pNewULSpace = (SvxULSpaceItem*)&pParent->Get( RES_UL_SPACE );
/*N*/ 			aFontSizeArr[0] = (SvxFontHeightItem*)&pParent->Get( RES_CHRATR_FONTSIZE );
/*N*/ 			aFontSizeArr[1] = (SvxFontHeightItem*)&pParent->Get( RES_CHRATR_CJK_FONTSIZE );
/*N*/ 			aFontSizeArr[2] = (SvxFontHeightItem*)&pParent->Get( RES_CHRATR_CTL_FONTSIZE );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 
/*?*/ 	case RES_LR_SPACE:
/*?*/ 		pNewLRSpace = (SvxLRSpaceItem*)pNew;
/*?*/ 		break;
/*?*/ 	case RES_UL_SPACE:
/*?*/ 		pNewULSpace = (SvxULSpaceItem*)pNew;
/*?*/ 		break;
/*?*/ 	case RES_CHRATR_FONTSIZE:
/*?*/ 		aFontSizeArr[0] = (SvxFontHeightItem*)pNew;
/*?*/ 		break;
/*?*/ 	case RES_CHRATR_CJK_FONTSIZE:
/*?*/ 		aFontSizeArr[1] = (SvxFontHeightItem*)pNew;
/*?*/ 		break;
/*?*/ 	case RES_CHRATR_CTL_FONTSIZE:
/*?*/ 		aFontSizeArr[2] = (SvxFontHeightItem*)pNew;
/*?*/ 		break;
/*?*/ 	}
/*?*/ 
/*N*/ 	int bWeiter = TRUE;
/*N*/ 
/*N*/ 	// dann pruefe doch mal gegen die eigenen Attribute
/*N*/ 	if( pNewLRSpace && SFX_ITEM_SET == GetItemState( RES_LR_SPACE, FALSE,
/*N*/ 										(const SfxPoolItem**)&pOldLRSpace ))
/*N*/ 	{
/*N*/ 		int bChg = FALSE;
/*N*/ 		if( pOldLRSpace != pNewLRSpace )	// verhinder Rekursion (SetAttr!!)
/*N*/ 		{
/*N*/ 			SvxLRSpaceItem aNew( *pOldLRSpace );
/*N*/ 			// wir hatten eine relative Angabe -> neu berechnen
/*N*/ 			if( 100 != aNew.GetPropLeft() )
/*N*/ 			{
/*N*/ 				long nTmp = aNew.GetLeft();		// alten zum Vergleichen
/*N*/ 				aNew.SetLeft( pNewLRSpace->GetLeft(), aNew.GetPropLeft() );
/*N*/ 				bChg |= nTmp != aNew.GetLeft();
/*N*/ 			}
/*N*/ 			// wir hatten eine relative Angabe -> neu berechnen
/*N*/ 			if( 100 != aNew.GetPropRight() )
/*N*/ 			{
/*N*/ 				long nTmp = aNew.GetRight();		// alten zum Vergleichen
/*N*/ 				aNew.SetRight( pNewLRSpace->GetRight(), aNew.GetPropRight() );
/*N*/ 				bChg |= nTmp != aNew.GetRight();
/*N*/ 			}
/*N*/ 			// wir hatten eine relative Angabe -> neu berechnen
/*N*/ 			if( 100 != aNew.GetPropTxtFirstLineOfst() )
/*N*/ 			{
/*N*/ 				short nTmp = aNew.GetTxtFirstLineOfst();		// alten zum Vergleichen
/*N*/ 				aNew.SetTxtFirstLineOfst( pNewLRSpace->GetTxtFirstLineOfst(),
/*N*/ 											aNew.GetPropTxtFirstLineOfst() );
/*N*/ 				bChg |= nTmp != aNew.GetTxtFirstLineOfst();
/*N*/ 			}
/*N*/ 			if( bChg )
/*N*/ 			{
/*N*/ 				SetAttr( aNew );
/*N*/ 				bWeiter = 0 != pOldChgSet || bNewParent;
/*N*/ 			}
/*N*/ 			// bei uns absolut gesetzt -> nicht weiter propagieren, es sei
/*N*/ 			// denn es wird bei uns gesetzt!
/*N*/ 			else if( pNewChgSet )
/*?*/ 				bWeiter = pNewChgSet->GetTheChgdSet() == &GetAttrSet();
/*N*/ 		}
/*N*/ 
/*N*/ #ifndef NUM_RELSPACE
/*N*/ 		if( !bChg && pOldLRSpace == pNewLRSpace &&
/*N*/ 			// pNewChgSet->GetTheChgdSet() == &aSet &&
/*N*/ 			GetOutlineLevel() < MAXLEVEL )
/*N*/ 		{
/*N*/ 			// dann muss der Wert in die OutlineRule uebertragen werden
/*N*/ 			GetDoc()->SetOutlineLSpace( GetOutlineLevel(),
/*N*/ 										pOldLRSpace->GetTxtFirstLineOfst(),
/*N*/ 										pOldLRSpace->GetTxtLeft() );
/*N*/ 		}
/*N*/ #endif
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pNewULSpace && SFX_ITEM_SET == GetItemState(
/*N*/ 			RES_UL_SPACE, FALSE, (const SfxPoolItem**)&pOldULSpace ) &&
/*N*/ 		pOldULSpace != pNewULSpace )	// verhinder Rekursion (SetAttr!!)
/*N*/ 	{
/*N*/ 		SvxULSpaceItem aNew( *pOldULSpace );
/*N*/ 		int bChg = FALSE;
/*N*/ 		// wir hatten eine relative Angabe -> neu berechnen
/*N*/ 		if( 100 != aNew.GetPropUpper() )
/*N*/ 		{
/*?*/ 			USHORT nTmp = aNew.GetUpper();		// alten zum Vergleichen
/*?*/ 			aNew.SetUpper( pNewULSpace->GetUpper(), aNew.GetPropUpper() );
/*?*/ 			bChg |= nTmp != aNew.GetUpper();
/*N*/ 		}
/*N*/ 		// wir hatten eine relative Angabe -> neu berechnen
/*N*/ 		if( 100 != aNew.GetPropLower() )
/*N*/ 		{
/*?*/ 			USHORT nTmp = aNew.GetLower();		// alten zum Vergleichen
/*?*/ 			aNew.SetLower( pNewULSpace->GetLower(), aNew.GetPropLower() );
/*?*/ 			bChg |= nTmp != aNew.GetLower();
/*N*/ 		}
/*N*/ 		if( bChg )
/*N*/ 		{
/*?*/ 			SetAttr( aNew );
/*?*/ 			bWeiter = 0 != pOldChgSet || bNewParent;
/*N*/ 		}
		// bei uns absolut gesetzt -> nicht weiter propagieren, es sei
		// denn es wird bei uns gesetzt!
/*N*/ 		else if( pNewChgSet )
/*?*/ 			bWeiter = pNewChgSet->GetTheChgdSet() == &GetAttrSet();
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 	for( int nC = 0, nArrLen = sizeof(aFontSizeArr) / sizeof( aFontSizeArr[0]);
/*N*/ 			nC < nArrLen; ++nC )
/*N*/ 	{
/*N*/ 		SvxFontHeightItem *pFSize = aFontSizeArr[ nC ], *pOldFSize;
/*N*/ 		if( pFSize && SFX_ITEM_SET == GetItemState(
/*N*/ 			pFSize->Which(), FALSE, (const SfxPoolItem**)&pOldFSize ) &&
/*N*/ 			// verhinder Rekursion (SetAttr!!)
/*N*/ 			pFSize != pOldFSize )
/*N*/ 		{
/*N*/ 			if( 100 == pOldFSize->GetProp() &&
/*N*/ 				SFX_MAPUNIT_RELATIVE == pOldFSize->GetPropUnit() )
/*N*/ 			{
/*N*/ 				// bei uns absolut gesetzt -> nicht weiter propagieren, es sei
/*N*/ 				// denn es wird bei uns gesetzt!
/*N*/ 				if( pNewChgSet )
/*?*/ 					bWeiter = pNewChgSet->GetTheChgdSet() == &GetAttrSet();
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				// wir hatten eine relative Angabe -> neu berechnen
/*N*/ 				UINT32 nTmp = pOldFSize->GetHeight();		// alten zum Vergleichen
/*N*/ 				SvxFontHeightItem aNew;
/*N*/ 				aNew.SetWhich( pFSize->Which() );
/*N*/ 				aNew.SetHeight( pFSize->GetHeight(), pOldFSize->GetProp(),
/*N*/ 								pOldFSize->GetPropUnit() );
/*N*/ 				if( nTmp != aNew.GetHeight() )
/*N*/ 				{
/*?*/ 					SetAttr( aNew );
/*?*/ 					bWeiter = 0 != pOldChgSet || bNewParent;
/*N*/ 				}
/*N*/ 				// bei uns absolut gesetzt -> nicht weiter propagieren, es sei
/*N*/ 				// denn es wird bei uns gesetzt!
/*N*/ 				else if( pNewChgSet )
/*?*/ 					bWeiter = pNewChgSet->GetTheChgdSet() == &GetAttrSet();
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( bWeiter )
/*N*/ 		SwFmtColl::Modify( pOld, pNew );
/*N*/ }


//FEATURE::CONDCOLL

/*N*/ SwCollCondition::SwCollCondition( SwTxtFmtColl* pColl, ULONG nMasterCond,
/*N*/ 								ULONG nSubCond )
/*N*/ 	: SwClient( pColl ), nCondition( nMasterCond )
/*N*/ {
/*N*/ 	aSubCondition.nSubCondition = nSubCond;
/*N*/ }






/*N*/ SwCollCondition::~SwCollCondition()
/*N*/ {
/*N*/ 	if( USRFLD_EXPRESSION & nCondition )
/*?*/ 		delete aSubCondition.pFldExpression;
/*N*/ }


/*N*/  int SwCollCondition::operator==( const SwCollCondition& rCmp ) const
/*N*/  {
DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	int nRet = 0;
/*N*/ }


/*N*/ void SwCollCondition::SetCondition( ULONG nCond, ULONG nSubCond )
/*N*/ {
/*N*/ 	if( USRFLD_EXPRESSION & nCondition )
/*?*/ 		delete aSubCondition.pFldExpression;
/*N*/ 	nCondition = nCond;
/*N*/ 	aSubCondition.nSubCondition = nSubCond;
/*N*/ }


/*N*/ SwConditionTxtFmtColl::~SwConditionTxtFmtColl()
/*N*/ {
/*N*/ }

#ifdef USED
	// zum "abfischen" von Aenderungen
#endif


/*N*/ const SwCollCondition* SwConditionTxtFmtColl::HasCondition(
/*N*/ 						const SwCollCondition& rCond ) const
/*N*/ {
/*N*/ 	const SwCollCondition* pFnd = 0;
		USHORT n=0;
/*N*/ 	for( n = 0; n < aCondColls.Count(); ++n )
/*?*/ 		if( *( pFnd = aCondColls[ n ]) == rCond )
/*?*/ 			break;
/*N*/ 
/*N*/ 	return n < aCondColls.Count() ? pFnd : 0;
/*N*/ }


/*N*/ void SwConditionTxtFmtColl::InsertCondition( const SwCollCondition& rCond )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	for( USHORT n = 0; n < aCondColls.Count(); ++n )
/*N*/ }




//FEATURE::CONDCOLL




}
