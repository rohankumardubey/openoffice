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



// INCLUDE ---------------------------------------------------------------

#include "dpobject.hxx"
#include "dptabsrc.hxx"
#include "dpsave.hxx"
#include "dpdimsave.hxx"
#include "dpoutput.hxx"
#include "dpshttab.hxx"
#include "dpsdbtab.hxx"
#include "dpgroup.hxx"
#include "document.hxx"
#include "rechead.hxx"
#include "pivot.hxx"		// PIVOT_DATA_FIELD
#include "dapiuno.hxx"		// ScDataPilotConversion
#include "miscuno.hxx"
#include "scerrors.hxx"
#include "refupdat.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "attrib.hxx"
#include "scitems.hxx"
#include "unonames.hxx"
// Wang Xu Ming -- 2009-8-17
// DataPilot Migration - Cache&&Performance
#include "dpglobal.hxx"
#include "globstr.hrc"
// End Comments
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceType.hpp>
#include <com/sun/star/sheet/DataPilotTableHeaderData.hpp>
#include <com/sun/star/sheet/DataPilotTablePositionData.hpp>
#include <com/sun/star/sheet/DataPilotTablePositionType.hpp>
#include <com/sun/star/sheet/DimensionFlags.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/sheet/XDrillDownDataSupplier.hpp>

#include <comphelper/processfactory.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <svl/zforlist.hxx>		// IsNumberFormat

#include <vector>
#include <stdio.h>

using namespace com::sun::star;
using ::std::vector;
using ::boost::shared_ptr;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::lang::XComponent;
using ::com::sun::star::sheet::DataPilotTableHeaderData;
using ::com::sun::star::sheet::DataPilotTablePositionData;
using ::com::sun::star::beans::XPropertySet;
using ::rtl::OUString;


// -----------------------------------------------------------------------

#define SCDPSOURCE_SERVICE	"com.sun.star.sheet.DataPilotSource"

// -----------------------------------------------------------------------

// incompatible versions of data pilot files
#define SC_DP_VERSION_CURRENT	6

// type of source data
#define SC_DP_SOURCE_SHEET		0
#define SC_DP_SOURCE_DATABASE	1
#define SC_DP_SOURCE_SERVICE	2

// -----------------------------------------------------------------------

//!	move to a header file
#define DP_PROP_COLUMNGRAND			"ColumnGrand"
#define DP_PROP_FUNCTION			"Function"
#define DP_PROP_IGNOREEMPTY			"IgnoreEmptyRows"
#define DP_PROP_ISDATALAYOUT		"IsDataLayoutDimension"
//#define DP_PROP_ISVISIBLE			"IsVisible"
#define DP_PROP_ORIENTATION			"Orientation"
#define DP_PROP_ORIGINAL			"Original"
#define DP_PROP_POSITION			"Position"
#define DP_PROP_REPEATIFEMPTY		"RepeatIfEmpty"
#define DP_PROP_ROWGRAND			"RowGrand"
#define DP_PROP_SHOWDETAILS			"ShowDetails"
#define DP_PROP_SHOWEMPTY			"ShowEmpty"
#define DP_PROP_SUBTOTALS			"SubTotals"
#define DP_PROP_USEDHIERARCHY		"UsedHierarchy"

// -----------------------------------------------------------------------

sal_uInt16 lcl_GetDataGetOrientation( const uno::Reference<sheet::XDimensionsSupplier>& xSource )
{
	long nRet = sheet::DataPilotFieldOrientation_HIDDEN;
	if ( xSource.is() )
	{
		uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
		uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
		long nIntCount = xIntDims->getCount();
		sal_Bool bFound = sal_False;
		for (long nIntDim=0; nIntDim<nIntCount && !bFound; nIntDim++)
		{
			uno::Reference<uno::XInterface> xIntDim =
				ScUnoHelpFunctions::AnyToInterface( xIntDims->getByIndex(nIntDim) );
			uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
			if ( xDimProp.is() )
			{
				bFound = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
					rtl::OUString::createFromAscii(DP_PROP_ISDATALAYOUT) );
				//!	error checking -- is "IsDataLayoutDimension" property required??
				if (bFound)
					nRet = ScUnoHelpFunctions::GetEnumProperty(
							xDimProp, rtl::OUString::createFromAscii(DP_PROP_ORIENTATION),
							sheet::DataPilotFieldOrientation_HIDDEN );
			}
		}
	}
    return static_cast< sal_uInt16 >( nRet );
}

// -----------------------------------------------------------------------

ScDPObject::ScDPObject( ScDocument* pD ) :
	pDoc( pD ),
	pSaveData( NULL ),
	pSheetDesc( NULL ),
	pImpDesc( NULL ),
	pServDesc( NULL ),
    mpTableData(static_cast<ScDPTableData*>(NULL)),
	pOutput( NULL ),
	bSettingsChanged( sal_False ),
	bAlive( sal_False ),
	bAllowMove( sal_False ),
	nHeaderRows( 0 ),
    mbHeaderLayout(false),
	bRefresh( sal_False ), // Wang Xu Ming - DataPilot migration
    mnCacheId( -1 ), // Wang Xu Ming - DataPilot migration
    mbCreatingTableData( false )
{
}

ScDPObject::ScDPObject(const ScDPObject& r) :
    ScDataObject(),
	pDoc( r.pDoc ),
	pSaveData( NULL ),
	aTableName( r.aTableName ),
	aTableTag( r.aTableTag ),
	aOutRange( r.aOutRange ),
	pSheetDesc( NULL ),
	pImpDesc( NULL ),
	pServDesc( NULL ),
    mpTableData(static_cast<ScDPTableData*>(NULL)), 
	pOutput( NULL ),
	bSettingsChanged( sal_False ),
	bAlive( sal_False ),
	bAllowMove( sal_False ),
	nHeaderRows( r.nHeaderRows ),
    mbHeaderLayout( r.mbHeaderLayout ),
	bRefresh( r.bRefresh ), // Wang Xu Ming - DataPilot migration
    mnCacheId ( r.mnCacheId ), // Wang Xu Ming - DataPilot migration
    mbCreatingTableData( false )
{
	if (r.pSaveData)
		pSaveData = new ScDPSaveData(*r.pSaveData);
	if (r.pSheetDesc)
		pSheetDesc = new ScSheetSourceDesc(*r.pSheetDesc);
	if (r.pImpDesc)
		pImpDesc = new ScImportSourceDesc(*r.pImpDesc);
	if (r.pServDesc)
		pServDesc = new ScDPServiceDesc(*r.pServDesc);
	// xSource (and pOutput) is not copied
}

ScDPObject::~ScDPObject()
{
	delete pOutput;
	delete pSaveData;
	delete pSheetDesc;
	delete pImpDesc;
	delete pServDesc;
	mnCacheId = -1; // Wang Xu Ming - DataPilot migration
    InvalidateSource();
}

ScDataObject* ScDPObject::Clone() const
{
	return new ScDPObject(*this);
}

void ScDPObject::SetAlive(sal_Bool bSet)
{
	bAlive = bSet;
}

void ScDPObject::SetAllowMove(sal_Bool bSet)
{
	bAllowMove = bSet;
}

void ScDPObject::SetSaveData(const ScDPSaveData& rData)
{
    if ( pSaveData != &rData )      // API implementation modifies the original SaveData object
    {
        delete pSaveData;
        pSaveData = new ScDPSaveData( rData );
        // Wang Xu Ming -- 2009-8-17
        // DataPilot Migration - Cache&&Performance
        if ( rData.GetCacheId() >= 0 )
            mnCacheId = rData.GetCacheId();
        else if ( mnCacheId >= 0 )
            pSaveData->SetCacheId( mnCacheId );
        // End Comments
    }

	InvalidateData();		// re-init source from SaveData
}

void ScDPObject::SetHeaderLayout (bool bUseGrid)
{
    mbHeaderLayout = bUseGrid;
}

bool ScDPObject::GetHeaderLayout() const
{
    return mbHeaderLayout;
}

void ScDPObject::SetOutRange(const ScRange& rRange)
{
	aOutRange = rRange;

	if ( pOutput )
		pOutput->SetPosition( rRange.aStart );
}

void ScDPObject::SetSheetDesc(const ScSheetSourceDesc& rDesc, bool bFromRefUpdate)
{
	if ( pSheetDesc && rDesc == *pSheetDesc )
		return;				// nothing to do

	DELETEZ( pImpDesc );
	DELETEZ( pServDesc );

    delete pSheetDesc;
	pSheetDesc = new ScSheetSourceDesc(rDesc);

	//	make valid QueryParam

	pSheetDesc->aQueryParam.nCol1 = pSheetDesc->aSourceRange.aStart.Col();
	pSheetDesc->aQueryParam.nRow1 = pSheetDesc->aSourceRange.aStart.Row();
	pSheetDesc->aQueryParam.nCol2 = pSheetDesc->aSourceRange.aEnd.Col();
	pSheetDesc->aQueryParam.nRow2 = pSheetDesc->aSourceRange.aEnd.Row();;
	pSheetDesc->aQueryParam.bHasHeader = sal_True;

	InvalidateSource();		// new source must be created
    if (!bFromRefUpdate)
        SetCacheId( -1 );   // #i116504# don't use the same cache ID for a different range (except reference update)
}

void ScDPObject::SetImportDesc(const ScImportSourceDesc& rDesc)
{
	if ( pImpDesc && rDesc == *pImpDesc )
		return;				// nothing to do

	DELETEZ( pSheetDesc );
	DELETEZ( pServDesc );

	delete pImpDesc;
	pImpDesc = new ScImportSourceDesc(rDesc);

	InvalidateSource();		// new source must be created
    SetCacheId( -1 );
}

void ScDPObject::SetServiceData(const ScDPServiceDesc& rDesc)
{
	if ( pServDesc && rDesc == *pServDesc )
		return;				// nothing to do

	DELETEZ( pSheetDesc );
	DELETEZ( pImpDesc );

	delete pServDesc;
	pServDesc = new ScDPServiceDesc(rDesc);

	InvalidateSource();		// new source must be created
}

void ScDPObject::WriteSourceDataTo( ScDPObject& rDest ) const
{
	if ( pSheetDesc )
		rDest.SetSheetDesc( *pSheetDesc );
	else if ( pImpDesc )
		rDest.SetImportDesc( *pImpDesc );
	else if ( pServDesc )
		rDest.SetServiceData( *pServDesc );

	//	name/tag are not source data, but needed along with source data

	rDest.aTableName = aTableName;
	rDest.aTableTag  = aTableTag;
}

void ScDPObject::WriteTempDataTo( ScDPObject& rDest ) const
{
	rDest.nHeaderRows = nHeaderRows;
}

sal_Bool ScDPObject::IsSheetData() const
{
	return ( pSheetDesc != NULL );
}

void ScDPObject::SetName(const String& rNew)
{
	aTableName = rNew;
}

void ScDPObject::SetTag(const String& rNew)
{
	aTableTag = rNew;
}

bool ScDPObject::IsDataDescriptionCell(const ScAddress& rPos)
{
    if (!pSaveData)
        return false;

    long nDataDimCount = pSaveData->GetDataDimensionCount();
    if (nDataDimCount != 1)
        // There has to be exactly one data dimension for the description to 
        // appear at top-left corner.
        return false;

    CreateOutput();
    ScRange aTabRange = pOutput->GetOutputRange(sheet::DataPilotOutputRangeType::TABLE);
    return (rPos == aTabRange.aStart);
}

uno::Reference<sheet::XDimensionsSupplier> ScDPObject::GetSource()
{
	CreateObjects();
	return xSource;
}

void ScDPObject::CreateOutput()
{
	CreateObjects();
	if (!pOutput)
	{
        sal_Bool bFilterButton = IsSheetData() && pSaveData && pSaveData->GetFilterButton();
        pOutput = new ScDPOutput( pDoc, xSource, aOutRange.aStart, bFilterButton );
        pOutput->SetHeaderLayout ( mbHeaderLayout );

		long nOldRows = nHeaderRows;
		nHeaderRows = pOutput->GetHeaderRows();

		if ( bAllowMove && nHeaderRows != nOldRows )
		{
			long nDiff = nOldRows - nHeaderRows;
			if ( nOldRows == 0 )
				--nDiff;
			if ( nHeaderRows == 0 )
				++nDiff;

			long nNewRow = aOutRange.aStart.Row() + nDiff;
			if ( nNewRow < 0 )
				nNewRow = 0;

			ScAddress aStart( aOutRange.aStart );
            aStart.SetRow(nNewRow);
			pOutput->SetPosition( aStart );

			//!	modify aOutRange?

			bAllowMove = sal_False;		// use only once
		}
	}
}

ScDPTableData* ScDPObject::GetTableData()
{
    if (!mpTableData && !mbCreatingTableData)
    {
        // #i117239# While filling the cache, mpTableData is still null.
        // Prevent nested calls from GetPivotData and similar functions.
        mbCreatingTableData = true;

        shared_ptr<ScDPTableData> pData;
        if ( pImpDesc )
        {
            // database data
            pData.reset(new ScDatabaseDPData(pDoc, *pImpDesc, GetCacheId()));
        }
        else
        {
            // cell data
            if (!pSheetDesc)
            {
                DBG_ERROR("no source descriptor");
                pSheetDesc = new ScSheetSourceDesc;     // dummy defaults
            }
            // Wang Xu Ming -- 2009-8-17
            // DataPilot Migration - Cache&&Performance
            pData.reset(new ScSheetDPData(pDoc, *pSheetDesc, GetCacheId()));
            // End Comments
        }

        // grouping (for cell or database data)
        if ( pSaveData && pSaveData->GetExistingDimensionData() )
        {
            shared_ptr<ScDPGroupTableData> pGroupData(new ScDPGroupTableData(pData, pDoc));
            pSaveData->GetExistingDimensionData()->WriteToData(*pGroupData);
            pData = pGroupData;
        }

        // Wang Xu Ming -- 2009-8-17
        // DataPilot Migration - Cache&&Performance
        if ( pData )
           SetCacheId( pData->GetCacheId());        // resets mpTableData
        // End Comments

        mpTableData = pData;                        // after SetCacheId

        mbCreatingTableData = false;
    }

    return mpTableData.get();
}

void ScDPObject::CreateObjects()
{
    // if groups are involved, create a new source with the ScDPGroupTableData
    if ( bSettingsChanged && pSaveData && pSaveData->GetExistingDimensionData() )
        InvalidateSource();

    if (!xSource.is())
    {
        //!	cache DPSource and/or Output?

        DBG_ASSERT( bAlive, "CreateObjects on non-inserted DPObject" );

        DELETEZ( pOutput );		// not valid when xSource is changed

        if ( pServDesc )
        {
            xSource = CreateSource( *pServDesc );
        }

        if ( !xSource.is() )    // database or sheet data, or error in CreateSource
		{
			DBG_ASSERT( !pServDesc, "DPSource could not be created" );
            ScDPTableData* pData = GetTableData();

            if ( pData )    // nested GetTableData calls may return NULL
            {
                ScDPSource* pSource = new ScDPSource( pData );
                xSource = pSource;

                if ( pSaveData && bRefresh )
                {
                    pSaveData->Refresh( xSource );
                    bRefresh = sal_False;
                }
            }
		}
        if ( xSource.is() && pSaveData )
            pSaveData->WriteToSource( xSource );
    }
    else if (bSettingsChanged)
    {
        DELETEZ( pOutput );		// not valid when xSource is changed

        uno::Reference<util::XRefreshable> xRef( xSource, uno::UNO_QUERY );
        if (xRef.is())
        {
            try
            {
                xRef->refresh();
            }
            catch(uno::Exception&)
            {
                DBG_ERROR("exception in refresh");
            }
        }

        if (pSaveData)
            pSaveData->WriteToSource( xSource );
    }
	bSettingsChanged = sal_False;
}

void ScDPObject::InvalidateData()
{
	bSettingsChanged = sal_True;
}

void ScDPObject::InvalidateSource()
{
    Reference< XComponent > xObjectComp( xSource, UNO_QUERY );
    if ( xObjectComp.is() )
    {
        try
        {
            xObjectComp->dispose();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    xSource = NULL;
    mpTableData.reset();
}

ScRange ScDPObject::GetNewOutputRange( sal_Bool& rOverflow )
{
	CreateOutput();				// create xSource and pOutput if not already done

	rOverflow = pOutput->HasError();		// range overflow or exception from source
	if ( rOverflow )
		return ScRange( aOutRange.aStart );
	else
	{
		//	don't store the result in aOutRange, because nothing has been output yet
		return pOutput->GetOutputRange();
	}
}

void ScDPObject::Output( const ScAddress& rPos )
{
	//	clear old output area
	pDoc->DeleteAreaTab( aOutRange.aStart.Col(), aOutRange.aStart.Row(),
						 aOutRange.aEnd.Col(),   aOutRange.aEnd.Row(),
						 aOutRange.aStart.Tab(), IDF_ALL );
	pDoc->RemoveFlagsTab( aOutRange.aStart.Col(), aOutRange.aStart.Row(),
						  aOutRange.aEnd.Col(),   aOutRange.aEnd.Row(),
						  aOutRange.aStart.Tab(), SC_MF_AUTO );

	CreateOutput();				// create xSource and pOutput if not already done

    pOutput->SetPosition( rPos );

	pOutput->Output();

	//	aOutRange is always the range that was last output to the document
	aOutRange = pOutput->GetOutputRange();
    const ScAddress& s = aOutRange.aStart;
    const ScAddress& e = aOutRange.aEnd;
    pDoc->ApplyFlagsTab(s.Col(), s.Row(), e.Col(), e.Row(), s.Tab(), SC_MF_DP_TABLE);
}

const ScRange ScDPObject::GetOutputRangeByType( sal_Int32 nType )
{
    CreateOutput();

    if (pOutput->HasError())
        return ScRange(aOutRange.aStart);

    return pOutput->GetOutputRange(nType);
}

sal_Bool lcl_HasButton( ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab )
{
	return ((const ScMergeFlagAttr*)pDoc->GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ))->HasButton();
}

void ScDPObject::RefreshAfterLoad()
{
	// apply drop-down attribute, initialize nHeaderRows, without accessing the source
	// (button attribute must be present)

	// simple test: block of button cells at the top, followed by an empty cell

	SCCOL nFirstCol = aOutRange.aStart.Col();
	SCROW nFirstRow = aOutRange.aStart.Row();
	SCTAB nTab = aOutRange.aStart.Tab();

	SCROW nInitial = 0;
	SCROW nOutRows = aOutRange.aEnd.Row() + 1 - aOutRange.aStart.Row();
	while ( nInitial + 1 < nOutRows && lcl_HasButton( pDoc, nFirstCol, nFirstRow + nInitial, nTab ) )
		++nInitial;

	if ( nInitial + 1 < nOutRows &&
		pDoc->IsBlockEmpty( nTab, nFirstCol, nFirstRow + nInitial, nFirstCol, nFirstRow + nInitial ) &&
		aOutRange.aEnd.Col() > nFirstCol )
	{
		sal_Bool bFilterButton = IsSheetData();			// when available, filter button setting must be checked here

		SCROW nSkip = bFilterButton ? 1 : 0;
		for (SCROW nPos=nSkip; nPos<nInitial; nPos++)
			pDoc->ApplyAttr( nFirstCol + 1, nFirstRow + nPos, nTab, ScMergeFlagAttr(SC_MF_AUTO) );

		nHeaderRows = nInitial;
	}
	else
		nHeaderRows = 0;		// nothing found, no drop-down lists
}

void ScDPObject::BuildAllDimensionMembers()
{
    if (!pSaveData)
        return;

    // #i111857# don't always create empty mpTableData for external service.
    // #163781# Initialize all members from xSource instead.
    if (pServDesc)
    {
        pSaveData->BuildAllDimensionMembersFromSource( this );
        return;
    }

    pSaveData->BuildAllDimensionMembers(GetTableData());
}

bool ScDPObject::GetMemberNames( sal_Int32 nDim, Sequence<OUString>& rNames )
{
    vector<ScDPLabelData::Member> aMembers;
    if (!GetMembers(nDim, GetUsedHierarchy(nDim), aMembers))
        return false;

    size_t n = aMembers.size();
    rNames.realloc(n);
    for (size_t i = 0; i < n; ++i)
        rNames[i] = aMembers[i].maName;

    return true;
}

bool ScDPObject::GetMembers( sal_Int32 nDim, sal_Int32 nHier, vector<ScDPLabelData::Member>& rMembers )
{
    Reference< container::XNameAccess > xMembersNA;
    if (!GetMembersNA( nDim, nHier, xMembersNA ))
        return false;

    Reference<container::XIndexAccess> xMembersIA( new ScNameToIndexAccess(xMembersNA) );
    sal_Int32 nCount = xMembersIA->getCount();
    vector<ScDPLabelData::Member> aMembers;
    aMembers.reserve(nCount);

    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        Reference<container::XNamed> xMember(xMembersIA->getByIndex(i), UNO_QUERY);
        ScDPLabelData::Member aMem;

        if (xMember.is())
            aMem.maName = xMember->getName();

        Reference<beans::XPropertySet> xMemProp(xMember, UNO_QUERY);
        if (xMemProp.is())
        {
            aMem.mbVisible     = ScUnoHelpFunctions::GetBoolProperty(xMemProp, OUString::createFromAscii(SC_UNO_ISVISIBL));
            aMem.mbShowDetails = ScUnoHelpFunctions::GetBoolProperty(xMemProp, OUString::createFromAscii(SC_UNO_SHOWDETA));

            aMem.maLayoutName = ScUnoHelpFunctions::GetStringProperty(
                xMemProp, OUString::createFromAscii(SC_UNO_LAYOUTNAME), OUString());
        }

        aMembers.push_back(aMem);
    }
    rMembers.swap(aMembers);
    return true;
}

void ScDPObject::UpdateReference( UpdateRefMode eUpdateRefMode,
									 const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
	// Output area

	SCCOL nCol1 = aOutRange.aStart.Col();
	SCROW nRow1 = aOutRange.aStart.Row();
	SCTAB nTab1 = aOutRange.aStart.Tab();
	SCCOL nCol2 = aOutRange.aEnd.Col();
	SCROW nRow2 = aOutRange.aEnd.Row();
	SCTAB nTab2 = aOutRange.aEnd.Tab();

	ScRefUpdateRes eRes =
		ScRefUpdate::Update( pDoc, eUpdateRefMode,
			rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
			rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aEnd.Tab(), nDx, nDy, nDz,
			nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
	if ( eRes != UR_NOTHING )
		SetOutRange( ScRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 ) );

	// sheet source data

	if ( pSheetDesc )
	{
		nCol1 = pSheetDesc->aSourceRange.aStart.Col();
		nRow1 = pSheetDesc->aSourceRange.aStart.Row();
		nTab1 = pSheetDesc->aSourceRange.aStart.Tab();
		nCol2 = pSheetDesc->aSourceRange.aEnd.Col();
		nRow2 = pSheetDesc->aSourceRange.aEnd.Row();
		nTab2 = pSheetDesc->aSourceRange.aEnd.Tab();

		eRes = ScRefUpdate::Update( pDoc, eUpdateRefMode,
				rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
				rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aEnd.Tab(), nDx, nDy, nDz,
				nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
		if ( eRes != UR_NOTHING )
		{
			ScSheetSourceDesc aNewDesc;
			aNewDesc.aSourceRange = ScRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );

			SCsCOL nDiffX = nCol1 - (SCsCOL) pSheetDesc->aSourceRange.aStart.Col();
			SCsROW nDiffY = nRow1 - (SCsROW) pSheetDesc->aSourceRange.aStart.Row();

			aNewDesc.aQueryParam = pSheetDesc->aQueryParam;
            aNewDesc.aQueryParam.nCol1 = sal::static_int_cast<SCCOL>( aNewDesc.aQueryParam.nCol1 + nDiffX );
            aNewDesc.aQueryParam.nCol2 = sal::static_int_cast<SCCOL>( aNewDesc.aQueryParam.nCol2 + nDiffX );
			aNewDesc.aQueryParam.nRow1 += nDiffY;	//! used?
			aNewDesc.aQueryParam.nRow2 += nDiffY;	//! used?
			SCSIZE nEC = aNewDesc.aQueryParam.GetEntryCount();
			for (SCSIZE i=0; i<nEC; i++)
				if (aNewDesc.aQueryParam.GetEntry(i).bDoQuery)
					aNewDesc.aQueryParam.GetEntry(i).nField += nDiffX;

            SetSheetDesc( aNewDesc, true );     // allocates new pSheetDesc
		}
	}
}

sal_Bool ScDPObject::RefsEqual( const ScDPObject& r ) const
{
	if ( aOutRange != r.aOutRange )
		return sal_False;

	if ( pSheetDesc && r.pSheetDesc )
	{
		if ( pSheetDesc->aSourceRange != r.pSheetDesc->aSourceRange )
			return sal_False;
	}
	else if ( pSheetDesc || r.pSheetDesc )
	{
		DBG_ERROR("RefsEqual: SheetDesc set at only one object");
		return sal_False;
	}

	return sal_True;
}

void ScDPObject::WriteRefsTo( ScDPObject& r ) const
{
	r.SetOutRange( aOutRange );
	if ( pSheetDesc )
        r.SetSheetDesc( *pSheetDesc, true );
}

void ScDPObject::GetPositionData(const ScAddress& rPos, DataPilotTablePositionData& rPosData)
{
    CreateOutput();
    pOutput->GetPositionData(rPos, rPosData);
}

bool ScDPObject::GetDataFieldPositionData(
    const ScAddress& rPos, Sequence<sheet::DataPilotFieldFilter>& rFilters)
{
    CreateOutput();

    vector<sheet::DataPilotFieldFilter> aFilters;
    if (!pOutput->GetDataResultPositionData(aFilters, rPos))
        return false;

    sal_Int32 n = static_cast<sal_Int32>(aFilters.size());
    rFilters.realloc(n);
    for (sal_Int32 i = 0; i < n; ++i)
        rFilters[i] = aFilters[i];

    return true;
}

void ScDPObject::GetDrillDownData(const ScAddress& rPos, Sequence< Sequence<Any> >& rTableData)
{
    CreateOutput();

    Reference<sheet::XDrillDownDataSupplier> xDrillDownData(xSource, UNO_QUERY);
    if (!xDrillDownData.is())
        return;

    Sequence<sheet::DataPilotFieldFilter> filters;
    if (!GetDataFieldPositionData(rPos, filters))
        return;

    rTableData = xDrillDownData->getDrillDownData(filters);
}

bool ScDPObject::IsDimNameInUse(const OUString& rName) const
{
    if (!xSource.is())
        return false;

    Reference<container::XNameAccess> xDims = xSource->getDimensions();
    Sequence<OUString> aDimNames = xDims->getElementNames();
    sal_Int32 n = aDimNames.getLength();
    for (sal_Int32 i = 0; i < n; ++i)
    {
        const OUString& rDimName = aDimNames[i];
        if (rDimName.equalsIgnoreAsciiCase(rName))
            return true;

        Reference<beans::XPropertySet> xPropSet(xDims->getByName(rDimName), UNO_QUERY);
        if (!xPropSet.is())
            continue;

        OUString aLayoutName = ScUnoHelpFunctions::GetStringProperty(
            xPropSet, OUString::createFromAscii(SC_UNO_LAYOUTNAME), OUString());
        if (aLayoutName.equalsIgnoreAsciiCase(rName))
            return true;
    }
    return false;
}

String ScDPObject::GetDimName( long nDim, sal_Bool& rIsDataLayout, sal_Int32* pFlags )
{
	rIsDataLayout = sal_False;
	String aRet;

	if ( xSource.is() )
	{
		uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
		uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
		long nDimCount = xDims->getCount();
		if ( nDim < nDimCount )
		{
			uno::Reference<uno::XInterface> xIntDim =
				ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
			uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
			uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
			if ( xDimName.is() && xDimProp.is() )
			{
				sal_Bool bData = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
								rtl::OUString::createFromAscii(DP_PROP_ISDATALAYOUT) );
				//!	error checking -- is "IsDataLayoutDimension" property required??

				rtl::OUString aName;
				try
				{
					aName = xDimName->getName();
				}
				catch(uno::Exception&)
				{
				}
				if ( bData )
					rIsDataLayout = sal_True;
				else
					aRet = String( aName );

                if (pFlags)
                    *pFlags = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                                rtl::OUString::createFromAscii(SC_UNO_FLAGS), 0 );
			}
		}
	}

	return aRet;
}

sal_Bool ScDPObject::IsDuplicated( long nDim )
{
    sal_Bool bDuplicated = sal_False;
    if ( xSource.is() )
    {
        uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
        uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
        long nDimCount = xDims->getCount();
        if ( nDim < nDimCount )
        {
            uno::Reference<uno::XInterface> xIntDim =
                ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
            uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
            if ( xDimProp.is() )
            {
                try
                {
                    uno::Any aOrigAny = xDimProp->getPropertyValue(
                                rtl::OUString::createFromAscii(DP_PROP_ORIGINAL) );
                    uno::Reference<uno::XInterface> xIntOrig;
                    if ( (aOrigAny >>= xIntOrig) && xIntOrig.is() )
                        bDuplicated = sal_True;
                }
                catch(uno::Exception&)
                {
                }
            }
        }
    }
    return bDuplicated;
}

long ScDPObject::GetDimCount()
{
    long nRet = 0;
    if ( xSource.is() )
    {
        try
        {
            uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
            if ( xDimsName.is() )
                nRet = xDimsName->getElementNames().getLength();
        }
        catch(uno::Exception&)
        {
        }
    }
    return nRet;
}

void ScDPObject::FillPageList( TypedScStrCollection& rStrings, long nField )
{
	//!	merge members access with ToggleDetails?

	//!	convert field index to dimension index?

	DBG_ASSERT( xSource.is(), "no source" );
	if ( !xSource.is() ) return;

	uno::Reference<container::XNamed> xDim;
	uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
	uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
	long nIntCount = xIntDims->getCount();
	if ( nField < nIntCount )
	{
		uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface(
									xIntDims->getByIndex(nField) );
		xDim = uno::Reference<container::XNamed>( xIntDim, uno::UNO_QUERY );
	}
	DBG_ASSERT( xDim.is(), "dimension not found" );
	if ( !xDim.is() ) return;

	uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
	long nHierarchy = ScUnoHelpFunctions::GetLongProperty( xDimProp,
							rtl::OUString::createFromAscii(DP_PROP_USEDHIERARCHY) );
	long nLevel = 0;

	long nHierCount = 0;
	uno::Reference<container::XIndexAccess> xHiers;
	uno::Reference<sheet::XHierarchiesSupplier> xHierSupp( xDim, uno::UNO_QUERY );
	if ( xHierSupp.is() )
	{
		uno::Reference<container::XNameAccess> xHiersName = xHierSupp->getHierarchies();
		xHiers = new ScNameToIndexAccess( xHiersName );
		nHierCount = xHiers->getCount();
	}
	uno::Reference<uno::XInterface> xHier;
	if ( nHierarchy < nHierCount )
		xHier = ScUnoHelpFunctions::AnyToInterface( xHiers->getByIndex(nHierarchy) );
	DBG_ASSERT( xHier.is(), "hierarchy not found" );
	if ( !xHier.is() ) return;

	long nLevCount = 0;
	uno::Reference<container::XIndexAccess> xLevels;
	uno::Reference<sheet::XLevelsSupplier> xLevSupp( xHier, uno::UNO_QUERY );
	if ( xLevSupp.is() )
	{
		uno::Reference<container::XNameAccess> xLevsName = xLevSupp->getLevels();
		xLevels = new ScNameToIndexAccess( xLevsName );
		nLevCount = xLevels->getCount();
	}
	uno::Reference<uno::XInterface> xLevel;
	if ( nLevel < nLevCount )
		xLevel = ScUnoHelpFunctions::AnyToInterface( xLevels->getByIndex(nLevel) );
	DBG_ASSERT( xLevel.is(), "level not found" );
	if ( !xLevel.is() ) return;

	uno::Reference<container::XNameAccess> xMembers;
	uno::Reference<sheet::XMembersSupplier> xMbrSupp( xLevel, uno::UNO_QUERY );
	if ( xMbrSupp.is() )
		xMembers = xMbrSupp->getMembers();
	DBG_ASSERT( xMembers.is(), "members not found" );
	if ( !xMembers.is() ) return;

	uno::Sequence<rtl::OUString> aNames = xMembers->getElementNames();
	long nNameCount = aNames.getLength();
	const rtl::OUString* pNameArr = aNames.getConstArray();
	for (long nPos = 0; nPos < nNameCount; ++nPos)
	{
        // Make sure to insert only visible members.
        Reference<XPropertySet> xPropSet(xMembers->getByName(pNameArr[nPos]), UNO_QUERY);
        sal_Bool bVisible = false;
        if (xPropSet.is())
        {
            Any any = xPropSet->getPropertyValue(OUString::createFromAscii(SC_UNO_ISVISIBL));
            any >>= bVisible;
        }

        if (bVisible)
        {
            // use the order from getElementNames
            TypedStrData* pData = new TypedStrData( pNameArr[nPos] );
            if ( !rStrings.AtInsert( rStrings.GetCount(), pData ) )
                delete pData;
        }
    }

	//	add "-all-" entry to the top (unsorted)
	TypedStrData* pAllData = new TypedStrData( String( ScResId( SCSTR_ALL ) ) );	//! separate string? (also output)
	if ( !rStrings.AtInsert( 0, pAllData ) )
		delete pAllData;
}

void ScDPObject::GetHeaderPositionData(const ScAddress& rPos, DataPilotTableHeaderData& rData)
{
    using namespace ::com::sun::star::sheet::DataPilotTablePositionType;

	CreateOutput();				// create xSource and pOutput if not already done

    // Reset member values to invalid state.
    rData.Dimension = rData.Hierarchy = rData.Level = -1;
    rData.Flags = 0;

    DataPilotTablePositionData aPosData;
    pOutput->GetPositionData(rPos, aPosData);
    const sal_Int32 nPosType = aPosData.PositionType;
    if (nPosType == COLUMN_HEADER || nPosType == ROW_HEADER)
        aPosData.PositionData >>= rData;
}

// Returns sal_True on success and stores the result in rTarget
sal_Bool ScDPObject::GetPivotData( ScDPGetPivotDataField& rTarget,
                               const std::vector< ScDPGetPivotDataField >& rFilters )
{
    // #i117239# Exit with an error if called from creating the cache for this object
    // (don't create an empty pOutput object)
    if (mbCreatingTableData)
        return sal_False;

    CreateOutput();             // create xSource and pOutput if not already done

    return pOutput->GetPivotData( rTarget, rFilters );
}

sal_Bool ScDPObject::IsFilterButton( const ScAddress& rPos )
{
	CreateOutput();				// create xSource and pOutput if not already done

	return pOutput->IsFilterButton( rPos );
}

long ScDPObject::GetHeaderDim( const ScAddress& rPos, sal_uInt16& rOrient )
{
	CreateOutput();				// create xSource and pOutput if not already done

	return pOutput->GetHeaderDim( rPos, rOrient );
}

sal_Bool ScDPObject::GetHeaderDrag( const ScAddress& rPos, sal_Bool bMouseLeft, sal_Bool bMouseTop, long nDragDim,
								Rectangle& rPosRect, sal_uInt16& rOrient, long& rDimPos )
{
	CreateOutput();				// create xSource and pOutput if not already done

	return pOutput->GetHeaderDrag( rPos, bMouseLeft, bMouseTop, nDragDim, rPosRect, rOrient, rDimPos );
}

void ScDPObject::GetMemberResultNames( ScStrCollection& rNames, long nDimension )
{
    CreateOutput();             // create xSource and pOutput if not already done

    pOutput->GetMemberResultNames( rNames, nDimension );    // used only with table data -> level not needed
}

bool lcl_Dequote( const String& rSource, xub_StrLen nStartPos, xub_StrLen& rEndPos, String& rResult )
{
    // nStartPos has to point to opening quote

    bool bRet = false;
    const sal_Unicode cQuote = '\'';

    if ( rSource.GetChar(nStartPos) == cQuote )
    {
        rtl::OUStringBuffer aBuffer;
        xub_StrLen nPos = nStartPos + 1;
        const xub_StrLen nLen = rSource.Len();

        while ( nPos < nLen )
        {
            const sal_Unicode cNext = rSource.GetChar(nPos);
            if ( cNext == cQuote )
            {
                if ( nPos+1 < nLen && rSource.GetChar(nPos+1) == cQuote )
                {
                    // double quote is used for an embedded quote
                    aBuffer.append( cNext );    // append one quote
                    ++nPos;                     // skip the next one
                }
                else
                {
                    // end of quoted string
                    rResult = aBuffer.makeStringAndClear();
                    rEndPos = nPos + 1;         // behind closing quote
                    return true;
                }
            }
            else
                aBuffer.append( cNext );
            
            ++nPos;
        }
        // no closing quote before the end of the string -> error (bRet still false)
    }

    return bRet;
}

struct ScGetPivotDataFunctionEntry
{
    const sal_Char*         pName;
    sheet::GeneralFunction  eFunc;
};

bool lcl_ParseFunction( const String& rList, xub_StrLen nStartPos, xub_StrLen& rEndPos, sheet::GeneralFunction& rFunc )
{
    static const ScGetPivotDataFunctionEntry aFunctions[] =
    {
        // our names
        { "Sum",        sheet::GeneralFunction_SUM       },
        { "Count",      sheet::GeneralFunction_COUNT     },
        { "Average",    sheet::GeneralFunction_AVERAGE   },
        { "Max",        sheet::GeneralFunction_MAX       },
        { "Min",        sheet::GeneralFunction_MIN       },
        { "Product",    sheet::GeneralFunction_PRODUCT   },
        { "CountNums",  sheet::GeneralFunction_COUNTNUMS },
        { "StDev",      sheet::GeneralFunction_STDEV     },
        { "StDevp",     sheet::GeneralFunction_STDEVP    },
        { "Var",        sheet::GeneralFunction_VAR       },
        { "VarP",       sheet::GeneralFunction_VARP      },
        // compatibility names
        { "Count Nums", sheet::GeneralFunction_COUNTNUMS },
        { "StdDev",     sheet::GeneralFunction_STDEV     },
        { "StdDevp",    sheet::GeneralFunction_STDEVP    }
	};

    const xub_StrLen nListLen = rList.Len();
    while ( nStartPos < nListLen && rList.GetChar(nStartPos) == ' ' )
        ++nStartPos;

    bool bParsed = false;
    bool bFound = false;
    String aFuncStr;
    xub_StrLen nFuncEnd = 0;
    if ( nStartPos < nListLen && rList.GetChar(nStartPos) == '\'' )
        bParsed = lcl_Dequote( rList, nStartPos, nFuncEnd, aFuncStr );
    else
    {
        nFuncEnd = rList.Search( static_cast<sal_Unicode>(']'), nStartPos );
        if ( nFuncEnd != STRING_NOTFOUND )
        {
            aFuncStr = rList.Copy( nStartPos, nFuncEnd - nStartPos );
            bParsed = true;
        }
    }

    if ( bParsed )
    {
        aFuncStr.EraseLeadingAndTrailingChars( ' ' );

        const sal_Int32 nFuncCount = sizeof(aFunctions) / sizeof(aFunctions[0]);
        for ( sal_Int32 nFunc=0; nFunc<nFuncCount && !bFound; nFunc++ )
        {
            if ( aFuncStr.EqualsIgnoreCaseAscii( aFunctions[nFunc].pName ) )
            {
                rFunc = aFunctions[nFunc].eFunc;
                bFound = true;

                while ( nFuncEnd < nListLen && rList.GetChar(nFuncEnd) == ' ' )
                    ++nFuncEnd;
                rEndPos = nFuncEnd;
            }
        }
    }

    return bFound;
}

bool lcl_IsAtStart( const String& rList, const String& rSearch, sal_Int32& rMatched,
                    bool bAllowBracket, sheet::GeneralFunction* pFunc )
{
    sal_Int32 nMatchList = 0;
    sal_Int32 nMatchSearch = 0;
    sal_Unicode cFirst = rList.GetChar(0);
    if ( cFirst == '\'' || cFirst == '[' )
    {
        // quoted string or string in brackets must match completely

        String aDequoted;
        xub_StrLen nQuoteEnd = 0;
        bool bParsed = false;

        if ( cFirst == '\'' )
            bParsed = lcl_Dequote( rList, 0, nQuoteEnd, aDequoted );
        else if ( cFirst == '[' )
        {
            // skip spaces after the opening bracket

            xub_StrLen nStartPos = 1;
            const xub_StrLen nListLen = rList.Len();
            while ( nStartPos < nListLen && rList.GetChar(nStartPos) == ' ' )
                ++nStartPos;

            if ( rList.GetChar(nStartPos) == '\'' )         // quoted within the brackets?
            {
                if ( lcl_Dequote( rList, nStartPos, nQuoteEnd, aDequoted ) )
                {
                    // after the quoted string, there must be the closing bracket, optionally preceded by spaces,
                    // and/or a function name
                    while ( nQuoteEnd < nListLen && rList.GetChar(nQuoteEnd) == ' ' )
                        ++nQuoteEnd;

                    // semicolon separates function name
                    if ( nQuoteEnd < nListLen && rList.GetChar(nQuoteEnd) == ';' && pFunc )
                    {
                        xub_StrLen nFuncEnd = 0;
                        if ( lcl_ParseFunction( rList, nQuoteEnd + 1, nFuncEnd, *pFunc ) )
                            nQuoteEnd = nFuncEnd;
                    }
                    if ( nQuoteEnd < nListLen && rList.GetChar(nQuoteEnd) == ']' )
                    {
                        ++nQuoteEnd;        // include the closing bracket for the matched length
                        bParsed = true;
                    }
                }
            }
            else
            {
                // implicit quoting to the closing bracket

                xub_StrLen nClosePos = rList.Search( static_cast<sal_Unicode>(']'), nStartPos );
                if ( nClosePos != STRING_NOTFOUND )
                {
                    xub_StrLen nNameEnd = nClosePos;
                    xub_StrLen nSemiPos = rList.Search( static_cast<sal_Unicode>(';'), nStartPos );
                    if ( nSemiPos != STRING_NOTFOUND && nSemiPos < nClosePos && pFunc )
                    {
                        xub_StrLen nFuncEnd = 0;
                        if ( lcl_ParseFunction( rList, nSemiPos + 1, nFuncEnd, *pFunc ) )
                            nNameEnd = nSemiPos;
                    }

                    aDequoted = rList.Copy( nStartPos, nNameEnd - nStartPos );
                    aDequoted.EraseTrailingChars( ' ' );        // spaces before the closing bracket or semicolon
                    nQuoteEnd = nClosePos + 1;
                    bParsed = true;
                }
            }
        }

        if ( bParsed && ScGlobal::GetpTransliteration()->isEqual( aDequoted, rSearch ) )
        {
            nMatchList = nQuoteEnd;             // match count in the list string, including quotes
            nMatchSearch = rSearch.Len();
        }
    }
    else
    {
        // otherwise look for search string at the start of rList
        ScGlobal::GetpTransliteration()->equals( rList, 0, rList.Len(), nMatchList,
                                            rSearch, 0, rSearch.Len(), nMatchSearch );
    }

    if ( nMatchSearch == rSearch.Len() )
    {
        // search string is at start of rList - look for following space or end of string

        bool bValid = false;
        if ( sal::static_int_cast<xub_StrLen>(nMatchList) >= rList.Len() )
            bValid = true;
        else
        {
            sal_Unicode cNext = rList.GetChar(sal::static_int_cast<xub_StrLen>(nMatchList));
            if ( cNext == ' ' || ( bAllowBracket && cNext == '[' ) )
                bValid = true;
        }
              
        if ( bValid )
        {
            rMatched = nMatchList;
            return true;
        }
    }

    return false;
}

sal_Bool ScDPObject::ParseFilters( ScDPGetPivotDataField& rTarget,
                               std::vector< ScDPGetPivotDataField >& rFilters,
                               const String& rFilterList )
{
    // parse the string rFilterList into parameters for GetPivotData

    CreateObjects();            // create xSource if not already done

    std::vector<String> aDataNames;     // data fields (source name)
    std::vector<String> aGivenNames;    // data fields (compound name)
    std::vector<String> aFieldNames;    // column/row/data fields
    std::vector< uno::Sequence<rtl::OUString> > aFieldValues;

    //
    // get all the field and item names
    //

    uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
    uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
    sal_Int32 nDimCount = xIntDims->getCount();
    for ( sal_Int32 nDim = 0; nDim<nDimCount; nDim++ )
    {
        uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface( xIntDims->getByIndex(nDim) );
        uno::Reference<container::XNamed> xDim( xIntDim, uno::UNO_QUERY );
        uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
        uno::Reference<sheet::XHierarchiesSupplier> xDimSupp( xDim, uno::UNO_QUERY );
        sal_Bool bDataLayout = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
                            rtl::OUString::createFromAscii(DP_PROP_ISDATALAYOUT) );
        sal_Int32 nOrient = ScUnoHelpFunctions::GetEnumProperty(
                            xDimProp, rtl::OUString::createFromAscii(DP_PROP_ORIENTATION),
                            sheet::DataPilotFieldOrientation_HIDDEN );
        if ( !bDataLayout )
        {
            if ( nOrient == sheet::DataPilotFieldOrientation_DATA )
            {
                String aSourceName;
                String aGivenName;
                ScDPOutput::GetDataDimensionNames( aSourceName, aGivenName, xIntDim );
                aDataNames.push_back( aSourceName );
                aGivenNames.push_back( aGivenName );
            }
            else if ( nOrient != sheet::DataPilotFieldOrientation_HIDDEN )
            {
                // get level names, as in ScDPOutput

                uno::Reference<container::XIndexAccess> xHiers = new ScNameToIndexAccess( xDimSupp->getHierarchies() );
                sal_Int32 nHierarchy = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                                                    rtl::OUString::createFromAscii(DP_PROP_USEDHIERARCHY) );
                if ( nHierarchy >= xHiers->getCount() )
                    nHierarchy = 0;

                uno::Reference<uno::XInterface> xHier = ScUnoHelpFunctions::AnyToInterface(
                                                    xHiers->getByIndex(nHierarchy) );
                uno::Reference<sheet::XLevelsSupplier> xHierSupp( xHier, uno::UNO_QUERY );
                if ( xHierSupp.is() )
                {
                    uno::Reference<container::XIndexAccess> xLevels = new ScNameToIndexAccess( xHierSupp->getLevels() );
                    sal_Int32 nLevCount = xLevels->getCount();
                    for (sal_Int32 nLev=0; nLev<nLevCount; nLev++)
                    {
                        uno::Reference<uno::XInterface> xLevel = ScUnoHelpFunctions::AnyToInterface(
                                                            xLevels->getByIndex(nLev) );
                        uno::Reference<container::XNamed> xLevNam( xLevel, uno::UNO_QUERY );
                        uno::Reference<sheet::XMembersSupplier> xLevSupp( xLevel, uno::UNO_QUERY );
                        if ( xLevNam.is() && xLevSupp.is() )
                        {
                            uno::Reference<container::XNameAccess> xMembers = xLevSupp->getMembers();

                            String aFieldName( xLevNam->getName() );
                            uno::Sequence<rtl::OUString> aMemberNames( xMembers->getElementNames() );

                            aFieldNames.push_back( aFieldName );
                            aFieldValues.push_back( aMemberNames );
                        }
                    }
                }
            }
        }
    }

    //
    // compare and build filters
    //

    SCSIZE nDataFields = aDataNames.size();
    SCSIZE nFieldCount = aFieldNames.size();
    DBG_ASSERT( aGivenNames.size() == nDataFields && aFieldValues.size() == nFieldCount, "wrong count" );

    bool bError = false;
    bool bHasData = false;
    String aRemaining( rFilterList );
    aRemaining.EraseLeadingAndTrailingChars( ' ' );
    while ( aRemaining.Len() && !bError )
    {
        bool bUsed = false;

        // look for data field name

        for ( SCSIZE nDataPos=0; nDataPos<nDataFields && !bUsed; nDataPos++ )
        {
            String aFound;
            sal_Int32 nMatched = 0;
            if ( lcl_IsAtStart( aRemaining, aDataNames[nDataPos], nMatched, false, NULL ) )
                aFound = aDataNames[nDataPos];
            else if ( lcl_IsAtStart( aRemaining, aGivenNames[nDataPos], nMatched, false, NULL ) )
                aFound = aGivenNames[nDataPos];

            if ( aFound.Len() )
            {
                rTarget.maFieldName = aFound;
                aRemaining.Erase( 0, sal::static_int_cast<xub_StrLen>(nMatched) );
                bHasData = true;
                bUsed = true;
            }
        }

        // look for field name

        String aSpecField;
        bool bHasFieldName = false;
        if ( !bUsed )
        {
            sal_Int32 nMatched = 0;
            for ( SCSIZE nField=0; nField<nFieldCount && !bHasFieldName; nField++ )
            {
                if ( lcl_IsAtStart( aRemaining, aFieldNames[nField], nMatched, true, NULL ) )
                {
                    aSpecField = aFieldNames[nField];
                    aRemaining.Erase( 0, sal::static_int_cast<xub_StrLen>(nMatched) );
                    aRemaining.EraseLeadingChars( ' ' );

                    // field name has to be followed by item name in brackets
                    if ( aRemaining.GetChar(0) == '[' )
                    {
                        bHasFieldName = true;
                        // bUsed remains false - still need the item
                    }
                    else
                    {
                        bUsed = true;
                        bError = true;
                    }
                }
            }
        }

        // look for field item

        if ( !bUsed )
        {
            bool bItemFound = false;
            sal_Int32 nMatched = 0;
            String aFoundName;
            String aFoundValue;
            sheet::GeneralFunction eFunc = sheet::GeneralFunction_NONE;
            sheet::GeneralFunction eFoundFunc = sheet::GeneralFunction_NONE;

            for ( SCSIZE nField=0; nField<nFieldCount; nField++ )
            {
                // If a field name is given, look in that field only, otherwise in all fields.
                // aSpecField is initialized from aFieldNames array, so exact comparison can be used.
                if ( !bHasFieldName || aFieldNames[nField] == aSpecField )
                {
                    const uno::Sequence<rtl::OUString>& rItems = aFieldValues[nField];
                    sal_Int32 nItemCount = rItems.getLength();
                    const rtl::OUString* pItemArr = rItems.getConstArray();
                    for ( sal_Int32 nItem=0; nItem<nItemCount; nItem++ )
                    {
                        if ( lcl_IsAtStart( aRemaining, pItemArr[nItem], nMatched, false, &eFunc ) )
                        {
                            if ( bItemFound )
                                bError = true;      // duplicate (also across fields)
                            else
                            {
                                aFoundName = aFieldNames[nField];
                                aFoundValue = pItemArr[nItem];
                                eFoundFunc = eFunc;
                                bItemFound = true;
                                bUsed = true;
                            }
                        }
                    }
                }
            }

            if ( bItemFound && !bError )
            {
                ScDPGetPivotDataField aField;
                aField.maFieldName = aFoundName;
                aField.meFunction = eFoundFunc;
                aField.mbValIsStr = true;
                aField.maValStr = aFoundValue;
                aField.mnValNum = 0.0;
                rFilters.push_back( aField );

                aRemaining.Erase( 0, sal::static_int_cast<xub_StrLen>(nMatched) );
            }
        }

        if ( !bUsed )
            bError = true;

        aRemaining.EraseLeadingChars( ' ' );        // remove any number of spaces between entries
    }

    if ( !bError && !bHasData && aDataNames.size() == 1 )
    {
        // if there's only one data field, its name need not be specified
        rTarget.maFieldName = aDataNames[0];
        bHasData = true;
    }

    return bHasData && !bError;
}

void ScDPObject::ToggleDetails(const DataPilotTableHeaderData& rElemDesc, ScDPObject* pDestObj)
{
	CreateObjects();			// create xSource if not already done

	//	find dimension name

	uno::Reference<container::XNamed> xDim;
	uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
	uno::Reference<container::XIndexAccess> xIntDims = new ScNameToIndexAccess( xDimsName );
	long nIntCount = xIntDims->getCount();
	if ( rElemDesc.Dimension < nIntCount )
	{
		uno::Reference<uno::XInterface> xIntDim = ScUnoHelpFunctions::AnyToInterface(
									xIntDims->getByIndex(rElemDesc.Dimension) );
		xDim = uno::Reference<container::XNamed>( xIntDim, uno::UNO_QUERY );
	}
	DBG_ASSERT( xDim.is(), "dimension not found" );
	if ( !xDim.is() ) return;
	String aDimName = xDim->getName();

	uno::Reference<beans::XPropertySet> xDimProp( xDim, uno::UNO_QUERY );
	sal_Bool bDataLayout = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
						rtl::OUString::createFromAscii(DP_PROP_ISDATALAYOUT) );
	if (bDataLayout)
	{
		//	the elements of the data layout dimension can't be found by their names
		//	-> don't change anything
		return;
	}

	//	query old state

	long nHierCount = 0;
	uno::Reference<container::XIndexAccess> xHiers;
	uno::Reference<sheet::XHierarchiesSupplier> xHierSupp( xDim, uno::UNO_QUERY );
	if ( xHierSupp.is() )
	{
		uno::Reference<container::XNameAccess> xHiersName = xHierSupp->getHierarchies();
		xHiers = new ScNameToIndexAccess( xHiersName );
		nHierCount = xHiers->getCount();
	}
	uno::Reference<uno::XInterface> xHier;
	if ( rElemDesc.Hierarchy < nHierCount )
		xHier = ScUnoHelpFunctions::AnyToInterface( xHiers->getByIndex(rElemDesc.Hierarchy) );
	DBG_ASSERT( xHier.is(), "hierarchy not found" );
	if ( !xHier.is() ) return;

	long nLevCount = 0;
	uno::Reference<container::XIndexAccess> xLevels;
	uno::Reference<sheet::XLevelsSupplier> xLevSupp( xHier, uno::UNO_QUERY );
	if ( xLevSupp.is() )
	{
		uno::Reference<container::XNameAccess> xLevsName = xLevSupp->getLevels();
		xLevels = new ScNameToIndexAccess( xLevsName );
		nLevCount = xLevels->getCount();
	}
	uno::Reference<uno::XInterface> xLevel;
	if ( rElemDesc.Level < nLevCount )
		xLevel = ScUnoHelpFunctions::AnyToInterface( xLevels->getByIndex(rElemDesc.Level) );
	DBG_ASSERT( xLevel.is(), "level not found" );
	if ( !xLevel.is() ) return;

	uno::Reference<container::XNameAccess> xMembers;
	uno::Reference<sheet::XMembersSupplier> xMbrSupp( xLevel, uno::UNO_QUERY );
	if ( xMbrSupp.is() )
		xMembers = xMbrSupp->getMembers();

	sal_Bool bFound = sal_False;
	sal_Bool bShowDetails = sal_True;

	if ( xMembers.is() )
	{
		if ( xMembers->hasByName(rElemDesc.MemberName) )
		{
			uno::Reference<uno::XInterface> xMemberInt = ScUnoHelpFunctions::AnyToInterface(
											xMembers->getByName(rElemDesc.MemberName) );
			uno::Reference<beans::XPropertySet> xMbrProp( xMemberInt, uno::UNO_QUERY );
			if ( xMbrProp.is() )
			{
				bShowDetails = ScUnoHelpFunctions::GetBoolProperty( xMbrProp,
									rtl::OUString::createFromAscii(DP_PROP_SHOWDETAILS) );
				//! don't set bFound if property is unknown?
				bFound = sal_True;
			}
		}
	}

	DBG_ASSERT( bFound, "member not found" );

	//!	use Hierarchy and Level in SaveData !!!!

	//	modify pDestObj if set, this object otherwise
	ScDPSaveData* pModifyData = pDestObj ? ( pDestObj->pSaveData ) : pSaveData;
	DBG_ASSERT( pModifyData, "no data?" );
	if ( pModifyData )
	{
        const String aName = rElemDesc.MemberName;
		pModifyData->GetDimensionByName(aDimName)->
			GetMemberByName(aName)->SetShowDetails( !bShowDetails );	// toggle

		if ( pDestObj )
			pDestObj->InvalidateData();		// re-init source from SaveData
		else
			InvalidateData();				// re-init source from SaveData
	}
}

long lcl_FindName( const rtl::OUString& rString, const uno::Reference<container::XNameAccess>& xCollection )
{
	if ( xCollection.is() )
	{
		uno::Sequence<rtl::OUString> aSeq = xCollection->getElementNames();
		long nCount = aSeq.getLength();
		const rtl::OUString* pArr = aSeq.getConstArray();
		for (long nPos=0; nPos<nCount; nPos++)
			if ( pArr[nPos] == rString )
				return nPos;
	}
	return -1;		// not found
}

sal_uInt16 lcl_FirstSubTotal( const uno::Reference<beans::XPropertySet>& xDimProp )		// PIVOT_FUNC mask
{
	uno::Reference<sheet::XHierarchiesSupplier> xDimSupp( xDimProp, uno::UNO_QUERY );
	if ( xDimProp.is() && xDimSupp.is() )
	{
		uno::Reference<container::XIndexAccess> xHiers = new ScNameToIndexAccess( xDimSupp->getHierarchies() );
		long nHierarchy = ScUnoHelpFunctions::GetLongProperty( xDimProp,
								rtl::OUString::createFromAscii(DP_PROP_USEDHIERARCHY) );
		if ( nHierarchy >= xHiers->getCount() )
			nHierarchy = 0;

		uno::Reference<uno::XInterface> xHier = ScUnoHelpFunctions::AnyToInterface(
									xHiers->getByIndex(nHierarchy) );
		uno::Reference<sheet::XLevelsSupplier> xHierSupp( xHier, uno::UNO_QUERY );
		if ( xHierSupp.is() )
		{
			uno::Reference<container::XIndexAccess> xLevels = new ScNameToIndexAccess( xHierSupp->getLevels() );
			uno::Reference<uno::XInterface> xLevel =
				ScUnoHelpFunctions::AnyToInterface( xLevels->getByIndex( 0 ) );
			uno::Reference<beans::XPropertySet> xLevProp( xLevel, uno::UNO_QUERY );
			if ( xLevProp.is() )
			{
				uno::Any aSubAny;
				try
				{
					aSubAny = xLevProp->getPropertyValue(
							rtl::OUString::createFromAscii(DP_PROP_SUBTOTALS) );
				}
				catch(uno::Exception&)
				{
				}
				uno::Sequence<sheet::GeneralFunction> aSeq;
				if ( aSubAny >>= aSeq )
				{
					sal_uInt16 nMask = 0;
					const sheet::GeneralFunction* pArray = aSeq.getConstArray();
					long nCount = aSeq.getLength();
					for (long i=0; i<nCount; i++)
						nMask |= ScDataPilotConversion::FunctionBit(pArray[i]);
					return nMask;
				}
			}
		}
	}

	DBG_ERROR("FirstSubTotal: NULL");
	return 0;
}

sal_uInt16 lcl_CountBits( sal_uInt16 nBits )
{
	if (!nBits) return 0;

	sal_uInt16 nCount = 0;
	sal_uInt16 nMask = 1;
	for (sal_uInt16 i=0; i<16; i++)
	{
		if ( nBits & nMask )
			++nCount;
		nMask <<= 1;
	}
	return nCount;
}

void lcl_FillOldFields( ScPivotFieldVector& rFields,
							const uno::Reference<sheet::XDimensionsSupplier>& xSource,
							sal_uInt16 nOrient, SCCOL nColAdd, bool bAddData )
{
	bool bDataFound = false;
	rFields.clear();

	//!	merge multiple occurences (data field with different functions)
	//!	force data field in one dimension

    std::vector< long > aPos;

	uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
	uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
	long nDimCount = xDims->getCount();
    for (long nDim=0; nDim < nDimCount; nDim++)
	{
		uno::Reference<uno::XInterface> xIntDim =
			ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
		uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );
		long nDimOrient = ScUnoHelpFunctions::GetEnumProperty(
							xDimProp, rtl::OUString::createFromAscii(DP_PROP_ORIENTATION),
							sheet::DataPilotFieldOrientation_HIDDEN );
		if ( xDimProp.is() && nDimOrient == nOrient )
		{
			sal_uInt16 nMask = 0;
			if ( nOrient == sheet::DataPilotFieldOrientation_DATA )
			{
				sheet::GeneralFunction eFunc = (sheet::GeneralFunction)ScUnoHelpFunctions::GetEnumProperty(
											xDimProp, rtl::OUString::createFromAscii(DP_PROP_FUNCTION),
											sheet::GeneralFunction_NONE );
				if ( eFunc == sheet::GeneralFunction_AUTO )
				{
					//!	test for numeric data
					eFunc = sheet::GeneralFunction_SUM;
				}
				nMask = ScDataPilotConversion::FunctionBit(eFunc);
			}
			else
				nMask = lcl_FirstSubTotal( xDimProp );		// from first hierarchy

			sal_Bool bDataLayout = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
									rtl::OUString::createFromAscii(DP_PROP_ISDATALAYOUT) );
			uno::Any aOrigAny;
			try
			{
				aOrigAny = xDimProp->getPropertyValue(
								rtl::OUString::createFromAscii(DP_PROP_ORIGINAL) );
			}
			catch(uno::Exception&)
			{
			}

			long nDupSource = -1;
			uno::Reference<uno::XInterface> xIntOrig = ScUnoHelpFunctions::AnyToInterface( aOrigAny );
			if ( xIntOrig.is() )
			{
				uno::Reference<container::XNamed> xNameOrig( xIntOrig, uno::UNO_QUERY );
				if ( xNameOrig.is() )
					nDupSource = lcl_FindName( xNameOrig->getName(), xDimsName );
			}

			bool bDupUsed = false;
			if ( nDupSource >= 0 )
			{
				//	add function bit to previous entry

				SCsCOL nCompCol;
				if ( bDataLayout )
					nCompCol = PIVOT_DATA_FIELD;
				else
					nCompCol = static_cast<SCsCOL>(nDupSource)+nColAdd;		//! seek source column from name

				for (ScPivotFieldVector::iterator aIt = rFields.begin(), aEnd = rFields.end(); (aIt != aEnd) && !bDupUsed; ++aIt)
					if ( aIt->nCol == nCompCol )
					{
						//	add to previous column only if new bits aren't already set there
						if ( ( aIt->nFuncMask & nMask ) == 0 )
						{
							aIt->nFuncMask |= nMask;
							aIt->nFuncCount = lcl_CountBits( aIt->nFuncMask );
							bDupUsed = true;
						}
					}
			}

			if ( !bDupUsed )		// also for duplicated dim if original has different orientation
			{
                rFields.resize( rFields.size() + 1 );
                ScPivotField& rField = rFields.back();

				if ( bDataLayout )
				{
					rField.nCol = PIVOT_DATA_FIELD;
					bDataFound = true;
				}
				else if ( nDupSource >= 0 )		// if source was not found (different orientation)
					rField.nCol = static_cast<SCsCOL>(nDupSource)+nColAdd;		//! seek from name
				else
					rField.nCol = static_cast<SCsCOL>(nDim)+nColAdd;	//! seek source column from name

				rField.nFuncMask = nMask;
				rField.nFuncCount = lcl_CountBits( nMask );
				
                aPos.push_back( ScUnoHelpFunctions::GetLongProperty( xDimProp,
									rtl::OUString::createFromAscii(DP_PROP_POSITION) ) );

                try
                {
                    if( nOrient == sheet::DataPilotFieldOrientation_DATA )
                        xDimProp->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_REFVALUE ) ) )
                            >>= rFields.back().maFieldRef;
                }
                catch( uno::Exception& )
                {
                }
			}
		}
	}

	//	sort by getPosition() value
    size_t nSize = aPos.size();
	for (size_t i=0; i+1<nSize; i++)
	{
		for (size_t j=0; j+i+1<nSize; j++)
            if ( aPos[j+1] < aPos[j] )
			{
                std::swap( aPos[j], aPos[j+1] );
                std::swap( rFields[j], rFields[j+1] );
			}
	}

	if ( bAddData && !bDataFound )
	{
        rFields.resize( rFields.size() + 1 );
        ScPivotField& rField = rFields.back();
		rField.nCol = PIVOT_DATA_FIELD;
		rField.nFuncMask = 0;
		rField.nFuncCount = 0;
	}
}

sal_Bool ScDPObject::FillOldParam(ScPivotParam& rParam) const
{
	((ScDPObject*)this)->CreateObjects();		// xSource is needed for field numbers

	rParam.nCol = aOutRange.aStart.Col();
	rParam.nRow = aOutRange.aStart.Row();
	rParam.nTab = aOutRange.aStart.Tab();
	// ppLabelArr / nLabels is not changed

	SCCOL nColAdd = 0;
	bool bAddData = ( lcl_GetDataGetOrientation( xSource ) == sheet::DataPilotFieldOrientation_HIDDEN );
    lcl_FillOldFields( rParam.maPageArr, xSource, sheet::DataPilotFieldOrientation_PAGE,   nColAdd, false );
	lcl_FillOldFields( rParam.maColArr,  xSource, sheet::DataPilotFieldOrientation_COLUMN, nColAdd, bAddData );
	lcl_FillOldFields( rParam.maRowArr,  xSource, sheet::DataPilotFieldOrientation_ROW,    nColAdd, false );
	lcl_FillOldFields( rParam.maDataArr, xSource, sheet::DataPilotFieldOrientation_DATA,   nColAdd, false );

	uno::Reference<beans::XPropertySet> xProp( xSource, uno::UNO_QUERY );
	if (xProp.is())
	{
		try
		{
			rParam.bMakeTotalCol = ScUnoHelpFunctions::GetBoolProperty( xProp,
						rtl::OUString::createFromAscii(DP_PROP_COLUMNGRAND), sal_True );
			rParam.bMakeTotalRow = ScUnoHelpFunctions::GetBoolProperty( xProp,
						rtl::OUString::createFromAscii(DP_PROP_ROWGRAND), sal_True );

			// following properties may be missing for external sources
			rParam.bIgnoreEmptyRows = ScUnoHelpFunctions::GetBoolProperty( xProp,
						rtl::OUString::createFromAscii(DP_PROP_IGNOREEMPTY) );
			rParam.bDetectCategories = ScUnoHelpFunctions::GetBoolProperty( xProp,
						rtl::OUString::createFromAscii(DP_PROP_REPEATIFEMPTY) );
		}
		catch(uno::Exception&)
		{
			// no error
		}
	}
	return sal_True;
}

void lcl_FillLabelData( ScDPLabelData& rData, const uno::Reference< beans::XPropertySet >& xDimProp )
{
	uno::Reference<sheet::XHierarchiesSupplier> xDimSupp( xDimProp, uno::UNO_QUERY );
	if ( xDimProp.is() && xDimSupp.is() )
	{
		uno::Reference<container::XIndexAccess> xHiers = new ScNameToIndexAccess( xDimSupp->getHierarchies() );
		long nHierarchy = ScUnoHelpFunctions::GetLongProperty( xDimProp,
								rtl::OUString::createFromAscii(DP_PROP_USEDHIERARCHY) );
		if ( nHierarchy >= xHiers->getCount() )
			nHierarchy = 0;
        rData.mnUsedHier = nHierarchy;

		uno::Reference<uno::XInterface> xHier = ScUnoHelpFunctions::AnyToInterface(
									xHiers->getByIndex(nHierarchy) );

		uno::Reference<sheet::XLevelsSupplier> xHierSupp( xHier, uno::UNO_QUERY );
		if ( xHierSupp.is() )
		{
			uno::Reference<container::XIndexAccess> xLevels = new ScNameToIndexAccess( xHierSupp->getLevels() );
			uno::Reference<uno::XInterface> xLevel =
				ScUnoHelpFunctions::AnyToInterface( xLevels->getByIndex( 0 ) );
			uno::Reference<beans::XPropertySet> xLevProp( xLevel, uno::UNO_QUERY );
			if ( xLevProp.is() )
            {
                rData.mbShowAll = ScUnoHelpFunctions::GetBoolProperty( xLevProp,
									rtl::OUString::createFromAscii(DP_PROP_SHOWEMPTY) );

                try
                {
                    xLevProp->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_SORTING ) ) )
                        >>= rData.maSortInfo;
                    xLevProp->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_LAYOUT ) ) )
                        >>= rData.maLayoutInfo;
                    xLevProp->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_AUTOSHOW ) ) )
                        >>= rData.maShowInfo;
                }
                catch(uno::Exception&)
                {
                }
            }
		}
	}
}

sal_Bool ScDPObject::FillLabelData(ScPivotParam& rParam)
{
    rParam.maLabelArray.clear();

	((ScDPObject*)this)->CreateObjects();

	uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
	uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
	long nDimCount = xDims->getCount();
	if ( nDimCount > MAX_LABELS )
		nDimCount = MAX_LABELS;
	if (!nDimCount)
		return sal_False;

	for (long nDim=0; nDim < nDimCount; nDim++)
	{
		String aFieldName;
		uno::Reference<uno::XInterface> xIntDim =
			ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
		uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
		uno::Reference<beans::XPropertySet> xDimProp( xIntDim, uno::UNO_QUERY );

		if ( xDimName.is() && xDimProp.is() )
		{
			sal_Bool bDuplicated = sal_False;
			sal_Bool bData = ScUnoHelpFunctions::GetBoolProperty( xDimProp,
							rtl::OUString::createFromAscii(DP_PROP_ISDATALAYOUT) );
			//!	error checking -- is "IsDataLayoutDimension" property required??

			try
			{
				aFieldName = String( xDimName->getName() );

				uno::Any aOrigAny = xDimProp->getPropertyValue(
							rtl::OUString::createFromAscii(DP_PROP_ORIGINAL) );
				uno::Reference<uno::XInterface> xIntOrig;
				if ( (aOrigAny >>= xIntOrig) && xIntOrig.is() )
					bDuplicated = sal_True;
			}
			catch(uno::Exception&)
			{
			}

            OUString aLayoutName = ScUnoHelpFunctions::GetStringProperty(
                xDimProp, OUString::createFromAscii(SC_UNO_LAYOUTNAME), OUString());

			if ( aFieldName.Len() && !bData && !bDuplicated )
			{
                SCsCOL nCol = static_cast< SCsCOL >( nDim );           //! ???
                bool bIsValue = true;                               //! check

                ScDPLabelData aNewLabel(aFieldName, nCol, bIsValue);
                aNewLabel.maLayoutName = aLayoutName;
                GetHierarchies(nDim, aNewLabel.maHiers);
                GetMembers(nDim, GetUsedHierarchy(nDim), aNewLabel.maMembers);
                lcl_FillLabelData(aNewLabel, xDimProp);
                aNewLabel.mnFlags = ScUnoHelpFunctions::GetLongProperty( xDimProp,
                                        rtl::OUString::createFromAscii(SC_UNO_FLAGS), 0 );
                rParam.maLabelArray.push_back(aNewLabel);
			}
		}
	}

	return sal_True;
}

sal_Bool ScDPObject::GetHierarchiesNA( sal_Int32 nDim, uno::Reference< container::XNameAccess >& xHiers )
{
    sal_Bool bRet = sal_False;
    uno::Reference<container::XNameAccess> xDimsName( GetSource()->getDimensions() );
    uno::Reference<container::XIndexAccess> xIntDims(new ScNameToIndexAccess( xDimsName ));
    if( xIntDims.is() )
    {
        uno::Reference<sheet::XHierarchiesSupplier> xHierSup(xIntDims->getByIndex( nDim ), uno::UNO_QUERY);
        if (xHierSup.is())
        {
            xHiers.set( xHierSup->getHierarchies() );
            bRet = xHiers.is();
        }
    }
    return bRet;
}

sal_Bool ScDPObject::GetHierarchies( sal_Int32 nDim, uno::Sequence< rtl::OUString >& rHiers )
{
    sal_Bool bRet = sal_False;
    uno::Reference< container::XNameAccess > xHiersNA;
    if( GetHierarchiesNA( nDim, xHiersNA ) )
    {
        rHiers = xHiersNA->getElementNames();
        bRet = sal_True;
    }
    return bRet;
}

sal_Int32 ScDPObject::GetUsedHierarchy( sal_Int32 nDim )
{
    sal_Int32 nHier = 0;
    uno::Reference<container::XNameAccess> xDimsName( GetSource()->getDimensions() );
    uno::Reference<container::XIndexAccess> xIntDims(new ScNameToIndexAccess( xDimsName ));
    uno::Reference<beans::XPropertySet> xDim(xIntDims->getByIndex( nDim ), uno::UNO_QUERY);
    if (xDim.is())
        nHier = ScUnoHelpFunctions::GetLongProperty( xDim, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNO_USEDHIER ) ) );
    return nHier;
}

sal_Bool ScDPObject::GetMembersNA( sal_Int32 nDim, uno::Reference< container::XNameAccess >& xMembers )
{
    return GetMembersNA( nDim, GetUsedHierarchy( nDim ), xMembers );
}

sal_Bool ScDPObject::GetMembersNA( sal_Int32 nDim, sal_Int32 nHier, uno::Reference< container::XNameAccess >& xMembers )
{
    sal_Bool bRet = sal_False;
    uno::Reference<container::XNameAccess> xDimsName( GetSource()->getDimensions() );
    uno::Reference<container::XIndexAccess> xIntDims(new ScNameToIndexAccess( xDimsName ));
    uno::Reference<beans::XPropertySet> xDim(xIntDims->getByIndex( nDim ), uno::UNO_QUERY);
    if (xDim.is())
    {
        uno::Reference<sheet::XHierarchiesSupplier> xHierSup(xDim, uno::UNO_QUERY);
        if (xHierSup.is())
        {
            uno::Reference<container::XIndexAccess> xHiers(new ScNameToIndexAccess(xHierSup->getHierarchies()));
            uno::Reference<sheet::XLevelsSupplier> xLevSupp( xHiers->getByIndex(nHier), uno::UNO_QUERY );
            if ( xLevSupp.is() )
            {
                uno::Reference<container::XIndexAccess> xLevels(new ScNameToIndexAccess( xLevSupp->getLevels()));
                if (xLevels.is())
                {
                    sal_Int32 nLevCount = xLevels->getCount();
                    if (nLevCount > 0)
                    {
                        uno::Reference<sheet::XMembersSupplier> xMembSupp( xLevels->getByIndex(0), uno::UNO_QUERY );
                        if ( xMembSupp.is() )
                        {
                            xMembers.set(xMembSupp->getMembers());
                            bRet = sal_True;
                        }
                    }
                }
            }
        }
    }
    return bRet;
}

//------------------------------------------------------------------------
//	convert old pivot tables into new datapilot tables

String lcl_GetDimName( const uno::Reference<sheet::XDimensionsSupplier>& xSource, long nDim )
{
	rtl::OUString aName;
	if ( xSource.is() )
	{
		uno::Reference<container::XNameAccess> xDimsName = xSource->getDimensions();
		uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess( xDimsName );
		long nDimCount = xDims->getCount();
		if ( nDim < nDimCount )
		{
			uno::Reference<uno::XInterface> xIntDim =
				ScUnoHelpFunctions::AnyToInterface( xDims->getByIndex(nDim) );
			uno::Reference<container::XNamed> xDimName( xIntDim, uno::UNO_QUERY );
			if (xDimName.is())
			{
				try
				{
					aName = xDimName->getName();
				}
				catch(uno::Exception&)
				{
				}
			}
		}
	}
	return aName;
}

// static
void ScDPObject::ConvertOrientation( ScDPSaveData& rSaveData,
							const ScPivotFieldVector& rFields, sal_uInt16 nOrient,
							ScDocument* pDoc, SCROW nRow, SCTAB nTab,
							const uno::Reference<sheet::XDimensionsSupplier>& xSource,
							bool bOldDefaults,
							const ScPivotFieldVector* pRefColFields,
                            const ScPivotFieldVector* pRefRowFields,
                            const ScPivotFieldVector* pRefPageFields )
{
	//	pDoc or xSource must be set
	DBG_ASSERT( pDoc || xSource.is(), "missing string source" );

	String aDocStr;
	ScDPSaveDimension* pDim;

	for (ScPivotFieldVector::const_iterator aIt = rFields.begin(), aEnd = rFields.end(); aIt != aEnd; ++aIt)
	{
		SCCOL nCol = aIt->nCol;
		sal_uInt16 nFuncs = aIt->nFuncMask;
        const sheet::DataPilotFieldReference& rFieldRef = aIt->maFieldRef;

		if ( nCol == PIVOT_DATA_FIELD )
			pDim = rSaveData.GetDataLayoutDimension();
		else
		{
			if ( pDoc )
				pDoc->GetString( nCol, nRow, nTab, aDocStr );
			else
				aDocStr = lcl_GetDimName( xSource, nCol );	// cols must start at 0

			if ( aDocStr.Len() )
				pDim = rSaveData.GetDimensionByName(aDocStr);
			else
				pDim = NULL;
		}

		if ( pDim )
		{
			if ( nOrient == sheet::DataPilotFieldOrientation_DATA )		// set summary function
			{
				//	generate an individual entry for each function
				bool bFirst = true;

                //  if a dimension is used for column/row/page and data,
				//	use duplicated dimensions for all data occurrences
				if (pRefColFields)
                    for (ScPivotFieldVector::const_iterator aRefIt = pRefColFields->begin(), aRefEnd = pRefColFields->end(); bFirst && (aRefIt != aRefEnd); ++aRefIt)
						if (aRefIt->nCol == nCol)
							bFirst = false;
				if (pRefRowFields)
                    for (ScPivotFieldVector::const_iterator aRefIt = pRefRowFields->begin(), aRefEnd = pRefRowFields->end(); bFirst && (aRefIt != aRefEnd); ++aRefIt)
						if (aRefIt->nCol == nCol)
							bFirst = false;
                if (pRefPageFields)
                    for (ScPivotFieldVector::const_iterator aRefIt = pRefPageFields->begin(), aRefEnd = pRefPageFields->end(); bFirst && (aRefIt != aRefEnd); ++aRefIt)
						if (aRefIt->nCol == nCol)
							bFirst = false;

				//	if set via api, a data column may occur several times
				//	(if the function hasn't been changed yet) -> also look for duplicate data column
                for (ScPivotFieldVector::const_iterator aRefIt = rFields.begin(); bFirst && (aRefIt != aIt); ++aRefIt)
					if (aRefIt->nCol == nCol)
						bFirst = false;

				sal_uInt16 nMask = 1;
				for (sal_uInt16 nBit=0; nBit<16; nBit++)
				{
					if ( nFuncs & nMask )
					{
						sheet::GeneralFunction eFunc = ScDataPilotConversion::FirstFunc( nMask );
                        ScDPSaveDimension* pCurrDim = bFirst ? pDim : rSaveData.DuplicateDimension(pDim->GetName());
                        pCurrDim->SetOrientation( nOrient );
                        pCurrDim->SetFunction( sal::static_int_cast<sal_uInt16>(eFunc) );

                        if( rFieldRef.ReferenceType == sheet::DataPilotFieldReferenceType::NONE )
                            pCurrDim->SetReferenceValue( 0 );
                        else
                            pCurrDim->SetReferenceValue( &rFieldRef );

                        bFirst = false;
					}
					nMask *= 2;
				}
			}
			else											// set SubTotals
			{
				pDim->SetOrientation( nOrient );

				sal_uInt16 nFuncArray[16];
				sal_uInt16 nFuncCount = 0;
				sal_uInt16 nMask = 1;
				for (sal_uInt16 nBit=0; nBit<16; nBit++)
				{
					if ( nFuncs & nMask )
                        nFuncArray[nFuncCount++] = sal::static_int_cast<sal_uInt16>(ScDataPilotConversion::FirstFunc( nMask ));
					nMask *= 2;
				}
				pDim->SetSubTotals( nFuncCount, nFuncArray );

				//	ShowEmpty was implicit in old tables,
				//	must be set for data layout dimension (not accessible in dialog)
				if ( bOldDefaults || nCol == PIVOT_DATA_FIELD )
					pDim->SetShowEmpty( sal_True );
			}
		}
	}
}

// static
bool ScDPObject::IsOrientationAllowed( sal_uInt16 nOrient, sal_Int32 nDimFlags )
{
    bool bAllowed = true;
    switch (nOrient)
    {
        case sheet::DataPilotFieldOrientation_PAGE:
            bAllowed = ( nDimFlags & sheet::DimensionFlags::NO_PAGE_ORIENTATION ) == 0;
            break;
        case sheet::DataPilotFieldOrientation_COLUMN:
            bAllowed = ( nDimFlags & sheet::DimensionFlags::NO_COLUMN_ORIENTATION ) == 0;
            break;
        case sheet::DataPilotFieldOrientation_ROW:
            bAllowed = ( nDimFlags & sheet::DimensionFlags::NO_ROW_ORIENTATION ) == 0;
            break;
        case sheet::DataPilotFieldOrientation_DATA:
            bAllowed = ( nDimFlags & sheet::DimensionFlags::NO_DATA_ORIENTATION ) == 0;
            break;
        default:
            {
                // allowed to remove from previous orientation
            }
    }
    return bAllowed;
}

// -----------------------------------------------------------------------

//	static
sal_Bool ScDPObject::HasRegisteredSources()
{
	sal_Bool bFound = sal_False;

	uno::Reference<lang::XMultiServiceFactory> xManager = comphelper::getProcessServiceFactory();
	uno::Reference<container::XContentEnumerationAccess> xEnAc( xManager, uno::UNO_QUERY );
	if ( xEnAc.is() )
	{
		uno::Reference<container::XEnumeration> xEnum = xEnAc->createContentEnumeration(
										rtl::OUString::createFromAscii( SCDPSOURCE_SERVICE ) );
		if ( xEnum.is() && xEnum->hasMoreElements() )
			bFound = sal_True;
	}

	return bFound;
}

//	static
uno::Sequence<rtl::OUString> ScDPObject::GetRegisteredSources()
{
	long nCount = 0;
	uno::Sequence<rtl::OUString> aSeq(0);

	//	use implementation names...

	uno::Reference<lang::XMultiServiceFactory> xManager = comphelper::getProcessServiceFactory();
	uno::Reference<container::XContentEnumerationAccess> xEnAc( xManager, uno::UNO_QUERY );
	if ( xEnAc.is() )
	{
		uno::Reference<container::XEnumeration> xEnum = xEnAc->createContentEnumeration(
										rtl::OUString::createFromAscii( SCDPSOURCE_SERVICE ) );
		if ( xEnum.is() )
		{
			while ( xEnum->hasMoreElements() )
			{
				uno::Any aAddInAny = xEnum->nextElement();
//				if ( aAddInAny.getReflection()->getTypeClass() == TypeClass_INTERFACE )
				{
					uno::Reference<uno::XInterface> xIntFac;
					aAddInAny >>= xIntFac;
					if ( xIntFac.is() )
					{
						uno::Reference<lang::XServiceInfo> xInfo( xIntFac, uno::UNO_QUERY );
						if ( xInfo.is() )
						{
							rtl::OUString sName = xInfo->getImplementationName();

							aSeq.realloc( nCount+1 );
							aSeq.getArray()[nCount] = sName;
							++nCount;
						}
					}
				}
			}
		}
	}

	return aSeq;
}

// use getContext from addincol.cxx
uno::Reference<uno::XComponentContext> getContext(uno::Reference<lang::XMultiServiceFactory> xMSF);

//	static
uno::Reference<sheet::XDimensionsSupplier> ScDPObject::CreateSource( const ScDPServiceDesc& rDesc )
{
	rtl::OUString aImplName = rDesc.aServiceName;
	uno::Reference<sheet::XDimensionsSupplier> xRet = NULL;

	uno::Reference<lang::XMultiServiceFactory> xManager = comphelper::getProcessServiceFactory();
	uno::Reference<container::XContentEnumerationAccess> xEnAc( xManager, uno::UNO_QUERY );
	if ( xEnAc.is() )
	{
		uno::Reference<container::XEnumeration> xEnum = xEnAc->createContentEnumeration(
										rtl::OUString::createFromAscii( SCDPSOURCE_SERVICE ) );
		if ( xEnum.is() )
		{
			while ( xEnum->hasMoreElements() && !xRet.is() )
			{
				uno::Any aAddInAny = xEnum->nextElement();
//				if ( aAddInAny.getReflection()->getTypeClass() == TypeClass_INTERFACE )
				{
					uno::Reference<uno::XInterface> xIntFac;
					aAddInAny >>= xIntFac;
					if ( xIntFac.is() )
					{
						uno::Reference<lang::XServiceInfo> xInfo( xIntFac, uno::UNO_QUERY );
						if ( xInfo.is() && xInfo->getImplementationName() == aImplName )
						{
							try
							{
                                // #i113160# try XSingleComponentFactory in addition to (old) XSingleServiceFactory,
                                // passing the context to the component (see ScUnoAddInCollection::Initialize)

                                uno::Reference<uno::XInterface> xInterface;
                                uno::Reference<uno::XComponentContext> xCtx = getContext(xManager);
                                uno::Reference<lang::XSingleComponentFactory> xCFac( xIntFac, uno::UNO_QUERY );
                                if (xCtx.is() && xCFac.is())
                                    xInterface = xCFac->createInstanceWithContext(xCtx);

                                if (!xInterface.is())
                                {
                                    uno::Reference<lang::XSingleServiceFactory> xFac( xIntFac, uno::UNO_QUERY );
                                    if ( xFac.is() )
                                        xInterface = xFac->createInstance();
                                }

								uno::Reference<lang::XInitialization> xInit( xInterface, uno::UNO_QUERY );
								if (xInit.is())
								{
									//	initialize
									uno::Sequence<uno::Any> aSeq(4);
									uno::Any* pArray = aSeq.getArray();
									pArray[0] <<= rtl::OUString( rDesc.aParSource );
									pArray[1] <<= rtl::OUString( rDesc.aParName );
									pArray[2] <<= rtl::OUString( rDesc.aParUser );
									pArray[3] <<= rtl::OUString( rDesc.aParPass );
									xInit->initialize( aSeq );
								}
								xRet = uno::Reference<sheet::XDimensionsSupplier>( xInterface, uno::UNO_QUERY );
							}
							catch(uno::Exception&)
							{
							}
						}
					}
				}
			}
		}
	}

	return xRet;
}

// ----------------------------------------------------------------------------

ScDPCollection::ScDPCollection(ScDocument* pDocument) :
	pDoc( pDocument )
{
}

ScDPCollection::ScDPCollection(const ScDPCollection& r) :
	ScCollection(r),
    pDoc(r.pDoc)
{
}

ScDPCollection::~ScDPCollection()
{
}

ScDataObject* ScDPCollection::Clone() const
{
	return new ScDPCollection(*this);
}

void ScDPCollection::DeleteOnTab( SCTAB nTab )
{
    sal_uInt16 nPos = 0;
    while ( nPos < nCount )
    {
        // look for output positions on the deleted sheet
        if ( static_cast<const ScDPObject*>(At(nPos))->GetOutRange().aStart.Tab() == nTab )
            AtFree(nPos);
        else
            ++nPos;
    }
}

void ScDPCollection::UpdateReference( UpdateRefMode eUpdateRefMode,
										 const ScRange& r, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
	for (sal_uInt16 i=0; i<nCount; i++)
		((ScDPObject*)At(i))->UpdateReference( eUpdateRefMode, r, nDx, nDy, nDz );
}

sal_Bool ScDPCollection::RefsEqual( const ScDPCollection& r ) const
{
	if ( nCount != r.nCount )
		return sal_False;

	for (sal_uInt16 i=0; i<nCount; i++)
		if ( ! ((const ScDPObject*)At(i))->RefsEqual( *((const ScDPObject*)r.At(i)) ) )
			return sal_False;

	return sal_True;	// all equal
}

void ScDPCollection::WriteRefsTo( ScDPCollection& r ) const
{
	if ( nCount == r.nCount )
	{
		//!	assert equal names?
		for (sal_uInt16 i=0; i<nCount; i++)
			((const ScDPObject*)At(i))->WriteRefsTo( *((ScDPObject*)r.At(i)) );
	}
	else
    {
        // #i8180# If data pilot tables were deleted with their sheet,
        // this collection contains extra entries that must be restored.
        // Matching objects are found by their names.

        DBG_ASSERT( nCount >= r.nCount, "WriteRefsTo: missing entries in document" );
        for (sal_uInt16 nSourcePos=0; nSourcePos<nCount; nSourcePos++)
        {
            const ScDPObject* pSourceObj = static_cast<const ScDPObject*>(At(nSourcePos));
            String aName = pSourceObj->GetName();
            bool bFound = false;
            for (sal_uInt16 nDestPos=0; nDestPos<r.nCount && !bFound; nDestPos++)
            {
                ScDPObject* pDestObj = static_cast<ScDPObject*>(r.At(nDestPos));
                if ( pDestObj->GetName() == aName )
                {
                    pSourceObj->WriteRefsTo( *pDestObj );     // found object, copy refs
                    bFound = true;
                }
            }
            if ( !bFound )
            {
                // none found, re-insert deleted object (see ScUndoDataPilot::Undo)

                ScDPObject* pDestObj = new ScDPObject( *pSourceObj );
                pDestObj->SetAlive(sal_True);
                if ( !r.InsertNewTable(pDestObj) )
                {
                    DBG_ERROR("cannot insert DPObject");
                    DELETEZ( pDestObj );
                }
            }
        }
        DBG_ASSERT( nCount == r.nCount, "WriteRefsTo: couldn't restore all entries" );
    }
}

ScDPObject* ScDPCollection::GetByName(const String& rName) const
{
    for (sal_uInt16 i=0; i<nCount; i++)
        if (static_cast<const ScDPObject*>(pItems[i])->GetName() == rName)
            return static_cast<ScDPObject*>(pItems[i]);
    return NULL;
}

String ScDPCollection::CreateNewName( sal_uInt16 nMin ) const
{
	String aBase( RTL_CONSTASCII_USTRINGPARAM( "Pivot" ) );
	//!	from Resource?

	for (sal_uInt16 nAdd=0; nAdd<=nCount; nAdd++)	//	nCount+1 tries
	{
		String aNewName = aBase;
		aNewName += String::CreateFromInt32( nMin + nAdd );
		sal_Bool bFound = sal_False;
		for (sal_uInt16 i=0; i<nCount && !bFound; i++)
			if (((const ScDPObject*)pItems[i])->GetName() == aNewName)
				bFound = sal_True;
		if (!bFound)
			return aNewName;			// found unused Name
	}
	return String();					// should not happen
}



// Wang Xu Ming -- 2009-8-17
// DataPilot Migration - Cache&&Performance
long ScDPObject::GetCacheId() const
{ 
    if ( GetSaveData() )
        return GetSaveData()->GetCacheId();
    else 
        return mnCacheId;
}
sal_uLong ScDPObject::RefreshCache()
{
    if ( pServDesc )
    {
        // cache table isn't used for external service - do nothing, no error
        return 0;
    }

    CreateObjects();
    sal_uLong nErrId = 0; 
    if ( pSheetDesc) 
        nErrId =  pSheetDesc->CheckValidate( pDoc );
    if ( nErrId == 0 )
    {
        long nOldId = GetCacheId();
        long nNewId = pDoc->GetNewDPObjectCacheId();
        if ( nOldId >= 0 )
            pDoc->RemoveDPObjectCache( nOldId );

        ScDPTableDataCache* pCache  = NULL;
        if ( pSheetDesc )
            pCache = pSheetDesc->CreateCache( pDoc, nNewId );
        else if ( pImpDesc )
            pCache = pImpDesc->CreateCache( pDoc, nNewId );

        if ( pCache == NULL )
        {
            //cache failed
            DBG_ASSERT( pCache , " pCache == NULL" );
            return STR_ERR_DATAPILOTSOURCE;
        }  

        nNewId = pCache->GetId();

        bRefresh = sal_True;
        ScDPCollection* pDPCollection = pDoc->GetDPCollection();
        sal_uInt16 nCount = pDPCollection->GetCount();
        for (sal_uInt16 i=0; i<nCount; i++)
        { //set new cache id
            if ( (*pDPCollection)[i]->GetCacheId() == nOldId  )
            {
                (*pDPCollection)[i]->SetCacheId( nNewId );
                (*pDPCollection)[i]->SetRefresh();

            }
        }      
        DBG_ASSERT( GetCacheId() >= 0, " GetCacheId() >= 0 " );
    }
    return nErrId;
}
void ScDPObject::SetCacheId( long nCacheId )
{
    if ( GetCacheId() != nCacheId )
    {
        InvalidateSource();
        if ( GetSaveData() )
            GetSaveData()->SetCacheId( nCacheId );

        mnCacheId = nCacheId;
    }
}
const ScDPTableDataCache* ScDPObject::GetCache() const
{
    return pDoc->GetDPObjectCache( GetCacheId() );
}
// End Comments

void ScDPCollection::FreeTable(ScDPObject* pDPObj)
{
    const ScRange& rOutRange = pDPObj->GetOutRange();
    const ScAddress& s = rOutRange.aStart;
    const ScAddress& e = rOutRange.aEnd;
    pDoc->RemoveFlagsTab(s.Col(), s.Row(), e.Col(), e.Row(), s.Tab(), SC_MF_DP_TABLE);
    Free(pDPObj);
}

bool ScDPCollection::InsertNewTable(ScDPObject* pDPObj)
{
    bool bSuccess = Insert(pDPObj);
    if (bSuccess)
    {
        const ScRange& rOutRange = pDPObj->GetOutRange();
        const ScAddress& s = rOutRange.aStart;
        const ScAddress& e = rOutRange.aEnd;
        pDoc->ApplyFlagsTab(s.Col(), s.Row(), e.Col(), e.Row(), s.Tab(), SC_MF_DP_TABLE);
    }
    return bSuccess;
}

bool ScDPCollection::HasDPTable(SCCOL nCol, SCROW nRow, SCTAB nTab) const
{
    const ScMergeFlagAttr* pMergeAttr = static_cast<const ScMergeFlagAttr*>(
            pDoc->GetAttr(nCol, nRow, nTab, ATTR_MERGE_FLAG));

    if (!pMergeAttr)
        return false;

    return pMergeAttr->HasDPTable();
}


