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


#ifndef _SCH_XMLPARAGRAPHCONTEXT_HXX_
#define _SCH_XMLPARAGRAPHCONTEXT_HXX_

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif
#ifndef _RTL_USTRBUF_HXX_
#include "rtl/ustrbuf.hxx"
#endif
namespace com { namespace sun { namespace star { namespace xml { namespace sax {
		class XAttributeList;
}}}}}
namespace binfilter {

class SchXMLImport;


class SchXMLParagraphContext : public SvXMLImportContext
{
private:
	::rtl::OUString& mrText;
    ::rtl::OUStringBuffer maBuffer;

public:
	SchXMLParagraphContext( SvXMLImport& rImport,
							const ::rtl::OUString& rLocalName,
							::rtl::OUString& rText );
	virtual ~SchXMLParagraphContext();	
	virtual void EndElement();
    
	virtual SvXMLImportContext* CreateChildContext(
		USHORT nPrefix,
		const ::rtl::OUString& rLocalName,
		const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

	virtual void Characters( const ::rtl::OUString& rChars );
};

}//end of namespace binfilter
#endif	// _SCH_XMLPARAGRAPHCONTEXT_HXX_
