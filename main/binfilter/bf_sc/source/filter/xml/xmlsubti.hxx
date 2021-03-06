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


#ifndef SC_XMLSUBTI_HXX
#define SC_XMLSUBTI_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <bf_xmloff/xmlictxt.hxx>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <bf_xmloff/xmlimp.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEET_HPP_
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_XCELLRANGE_HPP_
#include <com/sun/star/table/XCellRange.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLRANGEADDRESS_HPP_
#include <com/sun/star/table/CellRangeAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif
#include <list>

#ifndef _SC_XMLTABLESHAPERESIZER_HXX
#include "XMLTableShapeResizer.hxx"
#endif
namespace binfilter {

class ScXMLImport;

const int nDefaultRowCount = 20;
const int nDefaultColCount = 20;
const int nDefaultTabCount = 10;

typedef std::vector<sal_Int32> ScMysalIntVec;
typedef std::list<sal_Int32> ScMysalIntList;

class ScMyTableData
{
private:
	::com::sun::star::table::CellAddress	aTableCellPos;
	ScMysalIntVec	  					nColsPerCol;
	ScMysalIntVec				  		nRealCols;
	ScMysalIntVec	  	 				nRowsPerRow;
	ScMysalIntVec	  					nRealRows;
	sal_Int32							nSpannedCols;
	sal_Int32							nColCount;
	sal_Int32							nSubTableSpanned;
	ScMysalIntList						nChangedCols;
public:
										ScMyTableData(sal_Int16 nSheet = -1, sal_Int32 nCol = -1, sal_Int32 nRow = -1);
										~ScMyTableData();
	::com::sun::star::table::CellAddress 	GetCellPos() const { return aTableCellPos; }
	sal_Int32							GetRow() const { return aTableCellPos.Row; }
	sal_Int32							GetColumn() const { return aTableCellPos.Column; }
	void								AddRow();
	void								AddColumn();
	void								SetFirstColumn() { aTableCellPos.Column = -1; }
	sal_Int32                           GetColsPerCol(const sal_Int32 nIndex) const { return nColsPerCol[nIndex]; }
	void								SetColsPerCol(const sal_Int32 nIndex, sal_Int32 nValue = 1) { nColsPerCol[nIndex] = nValue; }
	sal_Int32							GetRealCols(const sal_Int32 nIndex, const sal_Bool bIsNormal = sal_True) const;
	void 								SetRealCols(const sal_Int32 nIndex, const sal_Int32 nValue) { nRealCols[nIndex] = nValue; }
	sal_Int32							GetRowsPerRow(const sal_Int32 nIndex) const { return nRowsPerRow[nIndex]; }
	void								SetRowsPerRow(const sal_Int32 nIndex, const sal_Int32 nValue = 1) { nRowsPerRow[nIndex] = nValue; }
	sal_Int32							GetRealRows(const sal_Int32 nIndex) const { return nIndex < 0 ? 0 : nRealRows[nIndex]; }
	void								SetRealRows(const sal_Int32 nIndex, const sal_Int32 nValue) { nRealRows[nIndex] = nValue; }
	sal_Int32							GetSpannedCols() const { return nSpannedCols; }
	void								SetSpannedCols(const sal_Int32 nTempSpannedCols) { nSpannedCols = nTempSpannedCols; }
	sal_Int32							GetColCount() const { return nColCount; }
	void								SetColCount(const sal_Int32 nTempColCount) { nColCount = nTempColCount; }
	sal_Int32							GetSubTableSpanned() const { return nSubTableSpanned; }
	void								SetSubTableSpanned(const sal_Int32 nValue) { nSubTableSpanned = nValue; }
	sal_Int32							GetChangedCols(const sal_Int32 nFromIndex, const sal_Int32 nToIndex) const;
	void								SetChangedCols(const sal_Int32 nValue);
};

//*******************************************************************************************************************************

class ScMyTables
{
private:
    typedef std::list< ::com::sun::star::table::CellRangeAddress>          ScMyMatrixRangeList;

	ScXMLImport&						rImport;

	ScMyShapeResizer                    aResizeShapes;

	::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet > xCurrentSheet;
	::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > xCurrentCellRange;
	::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > xDrawPage;
	::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShapes > xShapes;
	::rtl::OUString						sCurrentSheetName;
	::rtl::OUString						sPassword;
	std::vector<ScMyTableData*>			aTableVec;
    ScMyMatrixRangeList                 aMatrixRangeList;
	::com::sun::star::table::CellAddress	aRealCellPos;
	sal_Int32							nCurrentColStylePos;
	sal_Int16							nCurrentDrawPage;
	sal_Int16							nCurrentXShapes;
	sal_Int16							nTableCount;
	sal_Int16							nCurrentSheet;
	sal_Bool							bProtection : 1;

	sal_Bool 							IsMerged (const ::com::sun::star::uno::Reference < ::com::sun::star::table::XCellRange>& xCellRange,
												const sal_Int32 nCol, const sal_Int32 nRow,
												::com::sun::star::table::CellRangeAddress& aCellAddress) const;
	void								UnMerge();
	void								DoMerge(sal_Int32 nCount = -1);
	void								InsertRow();
	void								NewRow();
	void								InsertColumn();
	void								NewColumn(sal_Bool bIsCovered);
public:
										ScMyTables(ScXMLImport& rImport);
										~ScMyTables();
	void   								NewSheet(const ::rtl::OUString& sTableName, const ::rtl::OUString& sStyleName,
												const sal_Bool bProtection, const ::rtl::OUString& sPassword);
	void								AddRow();
	void								SetRowStyle(const ::rtl::OUString& rCellStyleName);
	void								AddColumn(sal_Bool bIsCovered);
	void								NewTable(sal_Int32 nTempSpannedCols);
	void								UpdateRowHeights();
	void								ResizeShapes() { aResizeShapes.ResizeShapes(); }
	void								DeleteTable();
	::com::sun::star::table::CellAddress	GetRealCellPos();
	void								AddColCount(sal_Int32 nTempColCount);
	void								AddColStyle(const sal_Int32 nRepeat, const ::rtl::OUString& rCellStyleName);
	::rtl::OUString						GetCurrentSheetName() const { return sCurrentSheetName; }
	sal_Int16							GetCurrentSheet() const { return nCurrentSheet; }
	sal_Int32							GetCurrentColumn() const { return aTableVec[nTableCount - 1]->GetColCount(); }
	sal_Int32							GetCurrentRow() const { return aTableVec[nTableCount - 1]->GetRow(); }
	::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet >
										GetCurrentXSheet()	{ return xCurrentSheet; }
	::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >
										GetCurrentXCellRange()	{ return xCurrentCellRange; }
	::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >
										GetCurrentXDrawPage();
	::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
										GetCurrentXShapes();
	sal_Bool							HasDrawPage();
	sal_Bool							HasXShapes();
	void								AddShape(::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShape>& rShape,
												::rtl::OUString* pRangeList,
												::com::sun::star::table::CellAddress& rStartAddress,
												::com::sun::star::table::CellAddress& rEndAddress,
												sal_Int32 nEndX, sal_Int32 nEndY);

    void                                AddMatrixRange(sal_uInt32 nStartColumn, sal_uInt32 nStartRow, sal_uInt32 nEndColumn, sal_uInt32 nEndRow);
    sal_Bool                            IsPartOfMatrix(sal_uInt32 nColumn, sal_uInt32 nRow);
};

} //namespace binfilter
#endif
