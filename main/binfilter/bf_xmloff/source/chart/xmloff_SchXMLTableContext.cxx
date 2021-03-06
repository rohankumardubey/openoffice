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



#include "SchXMLTableContext.hxx"
#include "SchXMLParagraphContext.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _COM_SUN_STAR_CHART_XCHARTDATAARRAY_HPP_
#include <com/sun/star/chart/XChartDataArray.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTSERIESADDRESS_HPP_
#include <com/sun/star/chart/ChartSeriesAddress.hpp>
#endif
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

// ----------------------------------------
// class SchXMLTableContext
// ----------------------------------------

SchXMLTableContext::SchXMLTableContext( SchXMLImportHelper& rImpHelper,
										SvXMLImport& rImport,
										const ::rtl::OUString& rLName,
										SchXMLTable& aTable ) :
		SvXMLImportContext( rImport, XML_NAMESPACE_TABLE, rLName ),
		mrImportHelper( rImpHelper ),
		mrTable( aTable )
{
	mrTable.nColumnIndex = -1;
	mrTable.nMaxColumnIndex = -1;
	mrTable.nRowIndex = -1;
	mrTable.aData.clear();
}

SchXMLTableContext::~SchXMLTableContext()
{
}

SvXMLImportContext *SchXMLTableContext::CreateChildContext(
	USHORT nPrefix,
	const ::rtl::OUString& rLocalName,
	const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
	SvXMLImportContext* pContext = 0;
	const SvXMLTokenMap& rTokenMap = mrImportHelper.GetTableElemTokenMap();

	switch( rTokenMap.Get( nPrefix, rLocalName ))
	{
		case XML_TOK_TABLE_HEADER_COLS:
		case XML_TOK_TABLE_COLUMNS:
			pContext = new SchXMLTableColumnsContext( mrImportHelper, GetImport(), rLocalName, mrTable );
			break;

		case XML_TOK_TABLE_COLUMN:
			pContext = new SchXMLTableColumnContext( mrImportHelper, GetImport(), rLocalName, mrTable );
			break;

		case XML_TOK_TABLE_HEADER_ROWS:
		case XML_TOK_TABLE_ROWS:
			pContext = new SchXMLTableRowsContext( mrImportHelper, GetImport(), rLocalName, mrTable );
			break;

		case XML_TOK_TABLE_ROW:
			pContext = new SchXMLTableRowContext( mrImportHelper, GetImport(), rLocalName, mrTable );
			break;

		default:
			pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
	}

	return pContext;
}

// ========================================
// classes for columns
// ========================================

// ----------------------------------------
// class SchXMLTableColumnsContext
// ----------------------------------------

SchXMLTableColumnsContext::SchXMLTableColumnsContext(
	SchXMLImportHelper& rImpHelper,
	SvXMLImport& rImport,
	const ::rtl::OUString& rLocalName,
	SchXMLTable& aTable ) :
		SvXMLImportContext( rImport, XML_NAMESPACE_TABLE, rLocalName ),
		mrImportHelper( rImpHelper ),
		mrTable( aTable )
{
}

SchXMLTableColumnsContext::~SchXMLTableColumnsContext()
{
}

SvXMLImportContext* SchXMLTableColumnsContext::CreateChildContext(
	USHORT nPrefix,
	const ::rtl::OUString& rLocalName,
	const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
	SvXMLImportContext* pContext = 0;

	if( nPrefix == XML_NAMESPACE_TABLE &&
		IsXMLToken( rLocalName, XML_TABLE_COLUMN ) )
	{
		pContext = new SchXMLTableColumnContext( mrImportHelper, GetImport(), rLocalName, mrTable );
	}
	else
		pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

	return pContext;
}

// ----------------------------------------
// class SchXMLTableColumnContext
// ----------------------------------------

SchXMLTableColumnContext::SchXMLTableColumnContext(
	SchXMLImportHelper& rImpHelper,
	SvXMLImport& rImport,
	const ::rtl::OUString& rLocalName,
	SchXMLTable& aTable ) :
		SvXMLImportContext( rImport, XML_NAMESPACE_TABLE, rLocalName ),
		mrImportHelper( rImpHelper ),
		mrTable( aTable )
{
}

void SchXMLTableColumnContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
	// get number-columns-repeated attribute
	sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
	::rtl::OUString aValue;

	for( sal_Int16 i = 0; i < nAttrCount; i++ )
	{
		::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
		::rtl::OUString aLocalName;
		USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

		if( nPrefix == XML_NAMESPACE_TABLE &&
			IsXMLToken( aLocalName, XML_NUMBER_COLUMNS_REPEATED ) )
		{
			aValue = xAttrList->getValueByIndex( i );
			break;	 // we only need this attribute
		}
	}

	if( aValue.getLength())
	{
		sal_Int32 nRepeated = aValue.toInt32();
		mrTable.nNumberOfColsEstimate += nRepeated;
	}
	else
	{
		mrTable.nNumberOfColsEstimate++;
	}
}

SchXMLTableColumnContext::~SchXMLTableColumnContext()
{
}

// ========================================
// classes for rows
// ========================================

// ----------------------------------------
// class SchXMLTableRowsContext
// ----------------------------------------

SchXMLTableRowsContext::SchXMLTableRowsContext(
	SchXMLImportHelper& rImpHelper,
	SvXMLImport& rImport,
	const ::rtl::OUString& rLocalName,
	SchXMLTable& aTable ) :
		SvXMLImportContext( rImport, XML_NAMESPACE_TABLE, rLocalName ),
		mrImportHelper( rImpHelper ),
		mrTable( aTable )
{
}

SchXMLTableRowsContext::~SchXMLTableRowsContext()
{
}

SvXMLImportContext* SchXMLTableRowsContext::CreateChildContext(
	USHORT nPrefix,
	const ::rtl::OUString& rLocalName,
	const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
	SvXMLImportContext* pContext = 0;

	if( nPrefix == XML_NAMESPACE_TABLE &&
        IsXMLToken( rLocalName, XML_TABLE_ROW ) )
	{
		pContext = new SchXMLTableRowContext( mrImportHelper, GetImport(), rLocalName, mrTable );
	}
	else
	{
		pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
	}

	return pContext;
}

// ----------------------------------------
// class SchXMLTableRowContext
// ----------------------------------------

SchXMLTableRowContext::SchXMLTableRowContext(
	SchXMLImportHelper& rImpHelper,
	SvXMLImport& rImport,
	const ::rtl::OUString& rLocalName,
	SchXMLTable& aTable ) :
		SvXMLImportContext( rImport, XML_NAMESPACE_TABLE, rLocalName ),
		mrImportHelper( rImpHelper ),
		mrTable( aTable )
{
	mrTable.nColumnIndex = -1;
	mrTable.nRowIndex++;

	std::vector< SchXMLCell > aNewRow;
	aNewRow.reserve( mrTable.nNumberOfColsEstimate );
	while( mrTable.aData.size() <= (unsigned long)mrTable.nRowIndex )
		mrTable.aData.push_back( aNewRow );
}

SchXMLTableRowContext::~SchXMLTableRowContext()
{
}

SvXMLImportContext* SchXMLTableRowContext::CreateChildContext(
	USHORT nPrefix,
	const ::rtl::OUString& rLocalName,
	const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
	SvXMLImportContext* pContext = 0;

	// <table:table-cell> element
	if( nPrefix == XML_NAMESPACE_TABLE &&
        IsXMLToken(rLocalName, XML_TABLE_CELL ) )
	{
		pContext = new SchXMLTableCellContext( mrImportHelper, GetImport(), rLocalName, mrTable );
	}
	else
	{
		pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
	}

	return pContext;
}


// ========================================
// classes for cells and their content
// ========================================

// ----------------------------------------
// class SchXMLTableCellContext
// ----------------------------------------

SchXMLTableCellContext::SchXMLTableCellContext(
	SchXMLImportHelper& rImpHelper,
	SvXMLImport& rImport,
	const ::rtl::OUString& rLocalName,
	SchXMLTable& aTable ) :
		SvXMLImportContext( rImport, XML_NAMESPACE_TABLE, rLocalName ),
		mrImportHelper( rImpHelper ),
		mrTable( aTable )
{
}

SchXMLTableCellContext::~SchXMLTableCellContext()
{
}

void SchXMLTableCellContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
	sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
	::rtl::OUString aValue;
	::rtl::OUString aLocalName;
	::rtl::OUString aCellContent;	
	SchXMLCellType eValueType  = SCH_CELL_TYPE_UNKNOWN;
	const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetCellAttrTokenMap();

	for( sal_Int16 i = 0; i < nAttrCount; i++ )
	{
		::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
		USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

		switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
		{
			case XML_TOK_CELL_VAL_TYPE:
				aValue = xAttrList->getValueByIndex( i );
				if( IsXMLToken( aValue, XML_FLOAT ) )
					eValueType = SCH_CELL_TYPE_FLOAT;
				else if( IsXMLToken( aValue, XML_STRING ) )
					eValueType = SCH_CELL_TYPE_STRING;
				break;

			case XML_TOK_CELL_VALUE:
				aCellContent = xAttrList->getValueByIndex( i );
				break;
		}
	}

	mbReadPara = sal_True;
	SchXMLCell aCell;
	aCell.eType = eValueType;

	if( eValueType == SCH_CELL_TYPE_FLOAT )
	{
		double fData;
		// the result may be false if a NaN is read, but that's ok
		SvXMLUnitConverter::convertDouble( fData, aCellContent );

		aCell.fValue = fData;
		// dont read following <text:p> element
		mbReadPara = sal_False;
	}

	mrTable.aData[ mrTable.nRowIndex ].push_back( aCell );
	mrTable.nColumnIndex++;
	if( mrTable.nMaxColumnIndex < mrTable.nColumnIndex )
		mrTable.nMaxColumnIndex = mrTable.nColumnIndex;
}

SvXMLImportContext* SchXMLTableCellContext::CreateChildContext(
	USHORT nPrefix,
	const ::rtl::OUString& rLocalName,
	const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
	SvXMLImportContext* pContext = 0;

	// <text:p> element
	if( mbReadPara &&
		nPrefix == XML_NAMESPACE_TEXT &&
        IsXMLToken( rLocalName, XML_P ) )
	{
		// we have to read a string here (not a float)
		pContext = new SchXMLParagraphContext( GetImport(), rLocalName, maCellContent );
	}
	else
	{
		pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
	}

	return pContext;
}

void SchXMLTableCellContext::EndElement()
{
	if( mbReadPara && maCellContent.getLength())
		mrTable.aData[ mrTable.nRowIndex ][ mrTable.nColumnIndex ].aString = maCellContent;
}

// ========================================

// just interpret the table in a linear way with no references used
// (this is just a workaround for clipboard handling in EA2)
void SchXMLTableHelper::applyTableSimple(
	const SchXMLTable& rTable,
	uno::Reference< chart::XChartDocument > xChartDoc )
{
	// interpret table like this:
	//
	//  series ----+---\
	//             |   |
	//  categories |   |
	//        |    |   |
	//        V    V   V
	//        A    B   C  ...
	//   1         x   x        <--- labels
	//   2    x    0   0
	//   3    x    0   0
	//  ...
	if( xChartDoc.is())
	{
		uno::Reference< chart::XChartDataArray > xData( xChartDoc->getData(), uno::UNO_QUERY );
		if( xData.is())
		{
            // get NaN
            double fSolarNaN;
            ::rtl::math::setNan( &fSolarNaN );
            double fNaN = fSolarNaN;
            sal_Bool bConvertNaN = sal_False;

            uno::Reference< chart::XChartData > xChartData( xData, uno::UNO_QUERY );
			if( xChartData.is())
            {
                fNaN = xChartData->getNotANumber();
                bConvertNaN = ( ! ::rtl::math::isNan( fNaN ));
            }

            sal_Int32 nRowCount = rTable.aData.size();
			sal_Int32 nColumnCount = 0;
			sal_Int32 nCol = 0, nRow = 0;
			if( nRowCount )
				nColumnCount = rTable.aData[ 0 ].size();

			uno::Sequence< ::rtl::OUString > aCategories( nRowCount - 1 );
			uno::Sequence< ::rtl::OUString > aLabels( nColumnCount - 1 );
			uno::Sequence< uno::Sequence< double > > aData( nRowCount - 1 );
			for( nRow = 0; nRow < nRowCount - 1; nRow++ )
				aData[ nRow ].realloc( nColumnCount - 1 );

			// set labels
			::std::vector< ::std::vector< SchXMLCell > >::const_iterator iRow = rTable.aData.begin();
			for( nCol = 1; nCol < nColumnCount; nCol++ )
			{
				aLabels[ nCol - 1 ] = (*iRow)[ nCol ].aString;
			}
			xData->setColumnDescriptions( aLabels );
				
            double fVal;
            const sal_Bool bConstConvertNan = bConvertNaN;
			for( ++iRow, nRow = 0; iRow != rTable.aData.end(); iRow++, nRow++ )
			{
				aCategories[ nRow ] = (*iRow)[ 0 ].aString;
				for( nCol = 1; nCol < nColumnCount; nCol++ )
                {
                    fVal = (*iRow)[ nCol ].fValue;
                    if( bConstConvertNan &&
                        ::rtl::math::isNan( fVal ))
                        aData[ nRow ][ nCol - 1 ] = fNaN;
                    else
                        aData[ nRow ][ nCol - 1 ] = fVal;
                }
			}
			xData->setRowDescriptions( aCategories );
			xData->setData( aData );
		}
	}
}

}//end of namespace binfilter
