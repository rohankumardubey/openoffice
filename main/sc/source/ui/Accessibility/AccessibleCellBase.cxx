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


#include "AccessibleCellBase.hxx"
#include "attrib.hxx"
#include "scitems.hxx"
#include "miscuno.hxx"
#include "document.hxx"
#include "docfunc.hxx"
#include "cell.hxx"
#include "unoguard.hxx"
#include "scresid.hxx"
#ifndef SC_SC_HRC
#include "sc.hrc"
#endif
#include "unonames.hxx"

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <tools/debug.hxx>
#include <editeng/brshitem.hxx>
#include <rtl/uuid.h>
#include <comphelper/sequence.hxx>
#include <sfx2/objsh.hxx>

#include <float.h>

using namespace	::com::sun::star;
using namespace	::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessibleCellBase::ScAccessibleCellBase(
        const uno::Reference<XAccessible>& rxParent,
		ScDocument* pDoc,
		const ScAddress& rCellAddress,
		sal_Int32 nIndex)
	:
	ScAccessibleContextBase(rxParent, AccessibleRole::TABLE_CELL),
	maCellAddress(rCellAddress),
	mpDoc(pDoc),
	mnIndex(nIndex)
{
}

ScAccessibleCellBase::~ScAccessibleCellBase()
{
}

	//=====  XAccessibleComponent  ============================================

sal_Bool SAL_CALL ScAccessibleCellBase::isVisible(  )
		throw (uno::RuntimeException)
{
 	ScUnoGuard aGuard;
    IsObjectValid();
	// test whether the cell is hidden (column/row - hidden/filtered)
	sal_Bool bVisible(sal_True);
	if (mpDoc)
	{
        bool bColHidden = mpDoc->ColHidden(maCellAddress.Col(), maCellAddress.Tab());
        bool bRowHidden = mpDoc->RowHidden(maCellAddress.Row(), maCellAddress.Tab());
        bool bColFiltered = mpDoc->ColFiltered(maCellAddress.Col(), maCellAddress.Tab());
        bool bRowFiltered = mpDoc->RowFiltered(maCellAddress.Row(), maCellAddress.Tab());

        if (bColHidden || bColFiltered || bRowHidden || bRowFiltered)
			bVisible = sal_False;
	}
	return bVisible;
}

sal_Int32 SAL_CALL ScAccessibleCellBase::getForeground()
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    sal_Int32 nColor(0);
    if (mpDoc)
    {
        SfxObjectShell* pObjSh = mpDoc->GetDocumentShell();
        if ( pObjSh )
        {
            uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( pObjSh->GetModel(), uno::UNO_QUERY );
            if ( xSpreadDoc.is() )
            {
                uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
                uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
                if ( xIndex.is() )
                {
                    uno::Any aTable = xIndex->getByIndex(maCellAddress.Tab());
                    uno::Reference<sheet::XSpreadsheet> xTable;
                    if (aTable>>=xTable)
                    {
                        uno::Reference<table::XCell> xCell = xTable->getCellByPosition(maCellAddress.Col(), maCellAddress.Row());
                        if (xCell.is())
                        {
                            uno::Reference<beans::XPropertySet> xCellProps(xCell, uno::UNO_QUERY);
                            if (xCellProps.is())
                            {
                                uno::Any aAny = xCellProps->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_CCOLOR)));
                                aAny >>= nColor;
                            }
                        }
                    }
                }
            }
        }
    }
    return nColor;
}

sal_Int32 SAL_CALL ScAccessibleCellBase::getBackground()
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    sal_Int32 nColor(0);

    if (mpDoc)
    {
        SfxObjectShell* pObjSh = mpDoc->GetDocumentShell();
        if ( pObjSh )
        {
            uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( pObjSh->GetModel(), uno::UNO_QUERY );
            if ( xSpreadDoc.is() )
            {
                uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
                uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
                if ( xIndex.is() )
                {
                    uno::Any aTable = xIndex->getByIndex(maCellAddress.Tab());
                    uno::Reference<sheet::XSpreadsheet> xTable;
                    if (aTable>>=xTable)
                    {
                        uno::Reference<table::XCell> xCell = xTable->getCellByPosition(maCellAddress.Col(), maCellAddress.Row());
                        if (xCell.is())
                        {
                            uno::Reference<beans::XPropertySet> xCellProps(xCell, uno::UNO_QUERY);
                            if (xCellProps.is())
                            {
                                uno::Any aAny = xCellProps->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_CELLBACK)));
                                aAny >>= nColor;
                            }
                        }
                    }
                }
            }
        }
    }

    return nColor;
}

	//=====  XInterface  =====================================================

uno::Any SAL_CALL ScAccessibleCellBase::queryInterface( uno::Type const & rType )
	throw (uno::RuntimeException)
{
	uno::Any aAny (ScAccessibleCellBaseImpl::queryInterface(rType));
	return aAny.hasValue() ? aAny : ScAccessibleContextBase::queryInterface(rType);
}

void SAL_CALL ScAccessibleCellBase::acquire()
	throw ()
{
	ScAccessibleContextBase::acquire();
}

void SAL_CALL ScAccessibleCellBase::release()
	throw ()
{
	ScAccessibleContextBase::release();
}

	//=====  XAccessibleContext  ==============================================

sal_Int32
	ScAccessibleCellBase::getAccessibleIndexInParent(void)
        throw (uno::RuntimeException)
{
	ScUnoGuard aGuard;
    IsObjectValid();
	return mnIndex;
}

::rtl::OUString SAL_CALL
    ScAccessibleCellBase::createAccessibleDescription(void)
    throw (uno::RuntimeException)
{
	rtl::OUString sDescription = String(ScResId(STR_ACC_CELL_DESCR));

	return sDescription;
}

::rtl::OUString SAL_CALL
    ScAccessibleCellBase::createAccessibleName(void)
    throw (uno::RuntimeException)
{
	String sName( ScResId(STR_ACC_CELL_NAME) );
	String sAddress;
	// Document not needed, because only the cell address, but not the tablename is needed
	// always us OOO notation
	maCellAddress.Format( sAddress, SCA_VALID, NULL );
	sName.SearchAndReplaceAscii("%1", sAddress);
    /*  #i65103# ZoomText merges cell address and contents, e.g. if value 2 is
        contained in cell A1, ZT reads "cell A twelve" instead of "cell A1 - 2".
        Simple solution: Append a space character to the cell address. */
    sName.Append( ' ' );
    return rtl::OUString(sName);
}

	//=====  XAccessibleValue  ================================================

uno::Any SAL_CALL
	ScAccessibleCellBase::getCurrentValue(  )
	throw (uno::RuntimeException)
{
 	ScUnoGuard aGuard;
    IsObjectValid();
	uno::Any aAny;
	if (mpDoc)
		aAny <<= mpDoc->GetValue(maCellAddress);

	return aAny;
}

sal_Bool SAL_CALL
	ScAccessibleCellBase::setCurrentValue( const uno::Any& aNumber )
	throw (uno::RuntimeException)
{
 	ScUnoGuard aGuard;
    IsObjectValid();
	double fValue = 0;
	sal_Bool bResult(sal_False);
	if((aNumber >>= fValue) && mpDoc && mpDoc->GetDocumentShell())
	{
		uno::Reference<XAccessibleStateSet> xParentStates;
		if (getAccessibleParent().is())
		{
			uno::Reference<XAccessibleContext> xParentContext = getAccessibleParent()->getAccessibleContext();
			xParentStates = xParentContext->getAccessibleStateSet();
		}
		if (IsEditable(xParentStates))
		{
			ScDocShell* pDocShell = (ScDocShell*) mpDoc->GetDocumentShell();
			ScDocFunc aFunc(*pDocShell);
			bResult = aFunc.PutCell( maCellAddress, new ScValueCell(fValue), sal_True );
		}
	}
	return bResult;
}

uno::Any SAL_CALL
	ScAccessibleCellBase::getMaximumValue(  )
	throw (uno::RuntimeException)
{
	uno::Any aAny;
	aAny <<= DBL_MAX;

	return aAny;
}

uno::Any SAL_CALL
	ScAccessibleCellBase::getMinimumValue(  )
	throw (uno::RuntimeException)
{
	uno::Any aAny;
	aAny <<= -DBL_MAX;

	return aAny;
}

	//=====  XServiceInfo  ====================================================

::rtl::OUString SAL_CALL ScAccessibleCellBase::getImplementationName(void)
        throw (uno::RuntimeException)
{
	return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("ScAccessibleCellBase"));
}

	//=====  XTypeProvider  ===================================================

uno::Sequence< uno::Type > SAL_CALL ScAccessibleCellBase::getTypes()
		throw (uno::RuntimeException)
{
	return comphelper::concatSequences(ScAccessibleCellBaseImpl::getTypes(), ScAccessibleContextBase::getTypes());
}

uno::Sequence<sal_Int8> SAL_CALL
	ScAccessibleCellBase::getImplementationId(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
	static uno::Sequence<sal_Int8> aId;
	if (aId.getLength() == 0)
	{
		aId.realloc (16);
		rtl_createUuid (reinterpret_cast<sal_uInt8 *>(aId.getArray()), 0, sal_True);
	}
	return aId;
}

sal_Bool ScAccessibleCellBase::IsEditable(
	const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
	sal_Bool bEditable(sal_False);
	if (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::EDITABLE))
		bEditable = sal_True;
	return bEditable;
}
