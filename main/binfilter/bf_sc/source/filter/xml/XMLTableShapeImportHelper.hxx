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



#ifndef _SC_XMLTABLESHAPEIMPORTHELPER_HXX
#define _SC_XMLTABLESHAPEIMPORTHELPER_HXX

#ifndef _XMLOFF_SHAPEIMPORT_HXX_
#include <bf_xmloff/shapeimport.hxx>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
namespace binfilter {

class ScXMLImport;

class XMLTableShapeImportHelper : public XMLShapeImportHelper
{
	::com::sun::star::table::CellAddress aStartCell;
	sal_Bool bOnTable : 1;

public:

	XMLTableShapeImportHelper( ScXMLImport& rImp, SvXMLImportPropertyMapper *pImpMapper=0 );
	~XMLTableShapeImportHelper();

    void SetLayer(::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape>& rShape, sal_Int16 nLayerID, const ::rtl::OUString& sType) const;
    virtual void finishShape(::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rShape,
			const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
			::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rShapes);


	void SetCell (const ::com::sun::star::table::CellAddress& rAddress) { aStartCell = rAddress; }
	void SetOnTable (const sal_Bool bTempOnTable) { bOnTable = bTempOnTable; }
};


} //namespace binfilter
#endif
