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



#ifndef _XMLOFF_XMLINDEXILLUSTRATIONSOURCECONTEXT_HXX_
#define _XMLOFF_XMLINDEXILLUSTRATIONSOURCECONTEXT_HXX_

#ifndef _XMLOFF_XMLINDEXTABLESOURCECONTEXT_HXX_
#include "XMLIndexTableSourceContext.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_ 
#include <com/sun/star/uno/Reference.h>
#endif


namespace com { namespace sun { namespace star {
	namespace xml { namespace sax { class XAttributeList; } }
	namespace beans { class XPropertySet; }
} } }
namespace rtl {	class OUString; }
namespace binfilter {


/**
 * Import illustration index source element;
 *
 * All logic is inherited from table source context. The only difference is
 * the different child context (illustration entry template).
 */
class XMLIndexIllustrationSourceContext : public XMLIndexTableSourceContext
{
public:

	TYPEINFO();

	XMLIndexIllustrationSourceContext(
		SvXMLImport& rImport, 
		sal_uInt16 nPrfx,
		const ::rtl::OUString& rLocalName,
		::com::sun::star::uno::Reference< 
			::com::sun::star::beans::XPropertySet> & rPropSet);

	~XMLIndexIllustrationSourceContext();

protected:

	virtual SvXMLImportContext* CreateChildContext( 
		sal_uInt16 nPrefix,
		const ::rtl::OUString& rLocalName,
		const ::com::sun::star::uno::Reference< 
			::com::sun::star::xml::sax::XAttributeList> & xAttrList );
};

}//end of namespace binfilter
#endif
