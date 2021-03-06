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


#include <hintids.hxx>
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <sfx2/request.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/brshitem.hxx>
#include <numrule.hxx>

#include "cmdid.h"
#include "wrtsh.hxx"
#include "view.hxx"
#include "viewopt.hxx"
#include "wdocsh.hxx"
#include "textsh.hxx"
#include "uiitems.hxx"
#include "swabstdlg.hxx"
#include <globals.hrc>
#include <sfx2/tabdlg.hxx>

void SwTextShell::ExecEnterNum(SfxRequest &rReq)
{
	//wg. Aufzeichnung schon vor dem evtl. Shellwechsel
	switch(rReq.GetSlot())
	{
	case FN_NUM_NUMBERING_ON:
	{
        SFX_REQUEST_ARG( rReq, pItem, SfxBoolItem, FN_PARAM_1 , sal_False );
        sal_Bool bMode = !GetShell().HasNumber(); // #i29560#
        if ( pItem )
            bMode = pItem->GetValue();
        else
            rReq.AppendItem( SfxBoolItem( FN_PARAM_1, bMode ) );

        if ( bMode != (GetShell().HasNumber()) ) // #i29560#
        {
            rReq.Done();
            if( bMode )
                GetShell().NumOn();
            else
                GetShell().NumOrBulletOff(); // #i29560#
        }
	}
	break;
	case FN_NUM_BULLET_ON:
	{
        SFX_REQUEST_ARG( rReq, pItem, SfxBoolItem, FN_PARAM_1 , sal_False );
        sal_Bool bMode = !GetShell().HasBullet(); // #i29560#
        if ( pItem )
            bMode = pItem->GetValue();
        else
            rReq.AppendItem( SfxBoolItem( FN_PARAM_1, bMode ) );

        if ( bMode != (GetShell().HasBullet()) ) // #i29560#
        {
            rReq.Done();
            if( bMode )
                GetShell().BulletOn();
            else
                GetShell().NumOrBulletOff(); // #i29560#
        }
	}
	break;
	case FN_NUMBER_BULLETS:
	{
        // --> OD 2008-02-29 #refactorlists#
//        // per default sal_True, damit die Schleife im Dialog richtig arbeitet!
//        sal_Bool bHasChild = sal_True;
        // <--
		SfxItemSet aSet(GetPool(),
				SID_HTML_MODE, SID_HTML_MODE,
				SID_ATTR_NUMBERING_RULE, SID_PARAM_CUR_NUM_LEVEL,
				0 );
		SwDocShell* pDocSh = GetView().GetDocShell();
		sal_Bool bHtml = 0 != PTR_CAST(SwWebDocShell, pDocSh);
		const SwNumRule* pCurRule = GetShell().GetCurNumRule();
		if( pCurRule )
		{
			SvxNumRule aRule = pCurRule->MakeSvxNumRule();

			//convert type of linked bitmaps from SVX_NUM_BITMAP to (SVX_NUM_BITMAP|LINK_TOKEN)
			for(sal_uInt16 i = 0; i < aRule.GetLevelCount(); i++)
			{
				SvxNumberFormat aFmt(aRule.GetLevel(i));
				if(SVX_NUM_BITMAP == aFmt.GetNumberingType())
				{
					const SvxBrushItem* pBrush = aFmt.GetBrush();
					const String* pLinkStr;
					if(pBrush &&
						0 != (pLinkStr = pBrush->GetGraphicLink()) &&
							pLinkStr->Len())
						aFmt.SetNumberingType(SvxExtNumType(SVX_NUM_BITMAP|LINK_TOKEN));
					aRule.SetLevel(i, aFmt, aRule.Get(i) != 0);
				}
			}
			if(bHtml)
				aRule.SetFeatureFlag(NUM_ENABLE_EMBEDDED_BMP, sal_False);

			aSet.Put(SvxNumBulletItem(aRule));
            // --> OD 2008-02-29 #refactorlists# - removed <bHasChild>
            ASSERT( GetShell().GetNumLevel() < MAXLEVEL,
                    "<SwTextShell::ExecEnterNum()> - numbered node without valid list level. Serious defect -> please inform OD." );
            sal_uInt16 nLevel = GetShell().GetNumLevel();
            // <--
			if( nLevel < MAXLEVEL )
			{
				nLevel = 1<<nLevel;
				aSet.Put( SfxUInt16Item( SID_PARAM_CUR_NUM_LEVEL, nLevel ));
			}
		}
		else
		{
            // --> OD 2008-02-11 #newlistlevelattrs#
            SwNumRule aRule( GetShell().GetUniqueNumRuleName(),
                             // --> OD 2008-06-06 #i89178#
                             numfunc::GetDefaultPositionAndSpaceMode() );
                             // <--
            // <--
			SvxNumRule aSvxRule = aRule.MakeSvxNumRule();
            const bool bRightToLeft = GetShell().IsInRightToLeftText( 0 );

            if( bHtml || bRightToLeft )
			{
                for( sal_uInt8 n = 0; n < MAXLEVEL; ++n )
				{
					SvxNumberFormat aFmt( aSvxRule.GetLevel( n ) );
                    if ( n && bHtml )
                    {
                        // 1/2" fuer HTML
                        aFmt.SetLSpace(720);
                        aFmt.SetAbsLSpace(n * 720);
                    }
                    // --> FME 2005-01-21 #i38904#  Default alignment for
                    // numbering/bullet should be rtl in rtl paragraph:
                    if ( bRightToLeft )
                    {
                        aFmt.SetNumAdjust( SVX_ADJUST_RIGHT );
                    }
                    // <--
                    aSvxRule.SetLevel( n, aFmt, sal_False );
				}
				aSvxRule.SetFeatureFlag(NUM_ENABLE_EMBEDDED_BMP, sal_False);
			}
			aSet.Put(SvxNumBulletItem(aSvxRule));
		}

		aSet.Put( SfxBoolItem( SID_PARAM_NUM_PRESET,sal_False ));

		// vor dem Dialog wird der HtmlMode an der DocShell versenkt
		pDocSh->PutItem(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(pDocSh)));

		SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");
        SfxAbstractTabDialog* pDlg = pFact->CreateSwTabDialog( DLG_SVXTEST_NUM_BULLET,
														GetView().GetWindow(), &aSet, GetShell());
        DBG_ASSERT(pDlg, "Dialogdiet fail!");
		sal_uInt16 nRet = pDlg->Execute();
		const SfxPoolItem* pItem;
		if( RET_OK == nRet )
		{
			if( SFX_ITEM_SET == pDlg->GetOutputItemSet()->GetItemState( SID_ATTR_NUMBERING_RULE, sal_False, &pItem ))
			{
				rReq.AppendItem(*pItem);
				rReq.Done();
				SvxNumRule* pSetRule = ((SvxNumBulletItem*)pItem)->GetNumRule();
				pSetRule->UnLinkGraphics();
                // --> OD 2008-02-11 #newlistlevelattrs#
				SwNumRule aSetRule( pCurRule
										? pCurRule->GetName()
                                        : GetShell().GetUniqueNumRuleName(),
                                    // --> OD 2008-06-06 #i89178#
                                    numfunc::GetDefaultPositionAndSpaceMode() );
                                    // <--
                // <--
				aSetRule.SetSvxRule( *pSetRule, GetShell().GetDoc());
				aSetRule.SetAutoRule( sal_True );
                // --> OD 2008-03-17 #refactorlists#
                // No start of new list, if an existing list style is edited.
                // Otherwise start a new list.
                const bool bCreateList = (pCurRule == 0);
                GetShell().SetCurNumRule( aSetRule, bCreateList );
                // <--
			}
			// wenn der Dialog mit OK verlassen wurde, aber nichts ausgewaehlt
			// wurde dann muss die Numerierung zumindest eingeschaltet werden,
			// wenn sie das noch nicht ist
			else if( !pCurRule && SFX_ITEM_SET == aSet.GetItemState( SID_ATTR_NUMBERING_RULE, sal_False, &pItem ))
			{
				rReq.AppendItem( *pItem );
				rReq.Done();
				SvxNumRule* pSetRule = ((SvxNumBulletItem*)pItem)->GetNumRule();
                // --> OD 2008-02-11 #newlistlevelattrs#
                SwNumRule aSetRule( GetShell().GetUniqueNumRuleName(),
                                    // --> OD 2008-06-06 #i89178#
                                    numfunc::GetDefaultPositionAndSpaceMode() );
                                    // <--
                // <--
				aSetRule.SetSvxRule(*pSetRule, GetShell().GetDoc());
				aSetRule.SetAutoRule( sal_True );
                // --> OD 2008-03-17 #refactorlists#
                // start new list
                GetShell().SetCurNumRule( aSetRule, true );
                // <--
			}
		}
		else if(RET_USER == nRet)
			GetShell().DelNumRules();

		delete pDlg;
	}
	break;
	default:
		ASSERT(sal_False,  falscher Dispatcher);
		return;
	}
}


