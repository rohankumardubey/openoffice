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


#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <frmfmt.hxx>
#include <ndnotxt.hxx>
#include <flyfrm.hxx>
#include <cntfrm.hxx>
#include <fmtcntnt.hxx>
#include <ndindex.hxx>
#include "fesh.hxx"
#include <hints.hxx>
#include "accmap.hxx"
#include "accframebase.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::rtl::OUString;

sal_Bool SwAccessibleFrameBase::IsSelected()
{
	sal_Bool bRet = sal_False;

    DBG_ASSERT( GetMap(), "no map?" );
	const ViewShell *pVSh = GetMap()->GetShell();
    DBG_ASSERT( pVSh, "no shell?" );
	if( pVSh->ISA( SwFEShell ) )
	{
		const SwFEShell *pFESh = static_cast< const SwFEShell * >( pVSh );
		const SwFrm *pFlyFrm = pFESh->GetCurrFlyFrm();
		if( pFlyFrm == GetFrm() )
			bRet = sal_True;
	}

	return bRet;
}

void SwAccessibleFrameBase::GetStates(
		::utl::AccessibleStateSetHelper& rStateSet )
{
	SwAccessibleContext::GetStates( rStateSet );

	const ViewShell *pVSh = GetMap()->GetShell();
    DBG_ASSERT( pVSh, "no shell?" );
	sal_Bool bSelectable =  pVSh->ISA( SwFEShell );

	// SELECTABLE
	if( bSelectable )
		rStateSet.AddState( AccessibleStateType::SELECTABLE );

	// FOCUSABLE
	if( bSelectable )
		rStateSet.AddState( AccessibleStateType::FOCUSABLE );

	// SELECTED and FOCUSED
	if( IsSelected() )
	{
		rStateSet.AddState( AccessibleStateType::SELECTED );
		ASSERT( bIsSelected, "bSelected out of sync" );
		::vos::ORef < SwAccessibleContext > xThis( this );
		GetMap()->SetCursorContext( xThis );

		Window *pWin = GetWindow();
		if( pWin && pWin->HasFocus() )
			rStateSet.AddState( AccessibleStateType::FOCUSED );
	}
}


sal_uInt8 SwAccessibleFrameBase::GetNodeType( const SwFlyFrm *pFlyFrm )
{
	sal_uInt8 nType = ND_TEXTNODE;
	if( pFlyFrm->Lower() )
	{
 		if( pFlyFrm->Lower()->IsNoTxtFrm() )
		{
			const SwCntntFrm *pCntFrm =
				static_cast<const SwCntntFrm *>( pFlyFrm->Lower() );
			nType = pCntFrm->GetNode()->GetNodeType();
		}
	}
	else
	{
		const SwFrmFmt *pFrmFmt = pFlyFrm->GetFmt();
		const SwFmtCntnt& rCntnt = pFrmFmt->GetCntnt();
		const SwNodeIndex *pNdIdx = rCntnt.GetCntntIdx();
		if( pNdIdx )
		{
			const SwCntntNode *pCNd =
				(pNdIdx->GetNodes())[pNdIdx->GetIndex()+1]->GetCntntNode();
			if( pCNd )
				nType = pCNd->GetNodeType();
		}
	}

	return nType;
}

SwAccessibleFrameBase::SwAccessibleFrameBase(
        SwAccessibleMap* pInitMap,
        sal_Int16 nInitRole,
        const SwFlyFrm* pFlyFrm  ) :
    SwAccessibleContext( pInitMap, nInitRole, pFlyFrm ),
	bIsSelected( sal_False )
{
	vos::OGuard aGuard(Application::GetSolarMutex());

	const SwFrmFmt *pFrmFmt = pFlyFrm->GetFmt();
	const_cast< SwFrmFmt * >( pFrmFmt )->Add( this );

	SetName( pFrmFmt->GetName() );

	bIsSelected = IsSelected();
}

void SwAccessibleFrameBase::_InvalidateCursorPos()
{
	sal_Bool bNewSelected = IsSelected();
	sal_Bool bOldSelected;

	{
		vos::OGuard aGuard( aMutex );
		bOldSelected = bIsSelected;
		bIsSelected = bNewSelected;
	}

	if( bNewSelected )
	{
		// remember that object as the one that has the caret. This is
		// neccessary to notify that object if the cursor leaves it.
		::vos::ORef < SwAccessibleContext > xThis( this );
		GetMap()->SetCursorContext( xThis );
	}

	if( bOldSelected != bNewSelected )
	{
		Window *pWin = GetWindow();
		if( pWin && pWin->HasFocus() && bNewSelected )
			FireStateChangedEvent( AccessibleStateType::FOCUSED, bNewSelected );
		FireStateChangedEvent( AccessibleStateType::SELECTED, bNewSelected );
		if( pWin && pWin->HasFocus() && !bNewSelected )
			FireStateChangedEvent( AccessibleStateType::FOCUSED, bNewSelected );

		uno::Reference< XAccessible > xParent( GetWeakParent() );
		if( xParent.is() )
		{
			SwAccessibleContext *pAcc =
				static_cast <SwAccessibleContext *>( xParent.get() );

			AccessibleEventObject aEvent;
			aEvent.EventId = AccessibleEventId::SELECTION_CHANGED;
			pAcc->FireAccessibleEvent( aEvent );
		}
	}
}

void SwAccessibleFrameBase::_InvalidateFocus()
{
	Window *pWin = GetWindow();
	if( pWin )
	{
		sal_Bool bSelected;

		{
			vos::OGuard aGuard( aMutex );
			bSelected = bIsSelected;
		}
		ASSERT( bSelected, "focus object should be selected" );

		FireStateChangedEvent( AccessibleStateType::FOCUSED,
							   pWin->HasFocus() && bSelected );
	}
}

sal_Bool SwAccessibleFrameBase::HasCursor()
{
	vos::OGuard aGuard( aMutex );
	return bIsSelected;
}


SwAccessibleFrameBase::~SwAccessibleFrameBase()
{
}

void SwAccessibleFrameBase::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
	sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0 ;
	const SwFlyFrm *pFlyFrm = static_cast< const SwFlyFrm * >( GetFrm() );
	switch( nWhich )
	{
	case RES_NAME_CHANGED:
		if(  pFlyFrm )
		{
			const SwFrmFmt *pFrmFmt = pFlyFrm->GetFmt();
			ASSERT( pFrmFmt == GetRegisteredIn(), "invalid frame" );

			OUString sOldName( GetName() );
			ASSERT( !pOld ||
					static_cast < const SwStringMsgPoolItem * >( pOld )->GetString() == String( sOldName ),
					"invalid old name" );

			const String& rNewName = pFrmFmt->GetName();
			SetName( rNewName );
			ASSERT( !pNew ||
					static_cast < const SwStringMsgPoolItem * >( pNew )->GetString() == rNewName,
					"invalid new name" );

			if( sOldName != GetName() )
			{
				AccessibleEventObject aEvent;
				aEvent.EventId = AccessibleEventId::NAME_CHANGED;
				aEvent.OldValue <<= sOldName;
				aEvent.NewValue <<= GetName();
				FireAccessibleEvent( aEvent );
			}
		}
		break;
	case RES_OBJECTDYING:
        // mba: it seems that this class intentionally does not call code in base class SwClient
		if( GetRegisteredIn() ==
				static_cast< SwModify *>( static_cast< const SwPtrMsgPoolItem * >( pOld )->pObject ) )
			GetRegisteredInNonConst()->Remove( this );
		break;

	case RES_FMT_CHG:
		if( static_cast< const SwFmtChg * >(pNew)->pChangedFmt == GetRegisteredIn() &&
			static_cast< const SwFmtChg * >(pOld)->pChangedFmt->IsFmtInDTOR() )
			GetRegisteredInNonConst()->Remove( this );
		break;

	default:
        // mba: former call to base class method removed as it is meant to handle only RES_OBJECTDYING
		break;
	}
}

void SwAccessibleFrameBase::Dispose( sal_Bool bRecursive )
{
	vos::OGuard aGuard(Application::GetSolarMutex());

	if( GetRegisteredIn() )
		GetRegisteredInNonConst()->Remove( this );

	SwAccessibleContext::Dispose( bRecursive );
}
