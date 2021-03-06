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



#ifndef _XMLOFF_XMLSECTIONIMPORTCONTEXT_HXX_
#define _XMLOFF_XMLSECTIONIMPORTCONTEXT_HXX_

#ifndef _XMLOFF_XMLICTXT_HXX 
#include "xmlictxt.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_ 
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

namespace com { namespace sun { namespace star {
	namespace text { class XTextRange;	}
	namespace beans { class XPropertySet; }
	namespace xml { namespace sax { class XAttributeList; } }
} } }
namespace rtl {	class OUString; }
namespace binfilter {
class XMLTextImportHelper;


/**
 * Import text sections.
 *
 * This context may *also* be used for index header sections. The 
 * differentiates its behaviour based on GetLocalName().
 */
class XMLSectionImportContext : public SvXMLImportContext
{
	/// start position; ranges aquired via getStart(),getEnd() don't move
	::com::sun::star::uno::Reference<
		::com::sun::star::text::XTextRange> xStartRange;

	/// end position
	::com::sun::star::uno::Reference<
		::com::sun::star::text::XTextRange> xEndRange;

	/// TextSection (as XPropertySet) for passing down to data source elements
	::com::sun::star::uno::Reference<
		::com::sun::star::beans::XPropertySet> xSectionPropertySet;

	const ::rtl::OUString sTextSection;
	const ::rtl::OUString sIndexHeaderSection;
	const ::rtl::OUString sCondition;
	const ::rtl::OUString sIsVisible;
	const ::rtl::OUString sProtectionKey;
	const ::rtl::OUString sIsProtected;
	const ::rtl::OUString sIsCurrentlyVisible;
	const ::rtl::OUString sEmpty;

	::rtl::OUString sStyleName;
	::rtl::OUString sName;
	::rtl::OUString sCond;
	::com::sun::star::uno::Sequence<sal_Int8> aSequence;
	sal_Bool bProtect;
	sal_Bool bCondOK;
	sal_Bool bIsVisible;
	sal_Bool bValid;
	sal_Bool bSequenceOK;
    sal_Bool bIsCurrentlyVisible;
    sal_Bool bIsCurrentlyVisibleOK;

	sal_Bool bHasContent;

public:

	TYPEINFO();

	XMLSectionImportContext(
		SvXMLImport& rImport, 
		sal_uInt16 nPrfx,
		const ::rtl::OUString& rLocalName );

	~XMLSectionImportContext();

protected:

	virtual void StartElement(
		const ::com::sun::star::uno::Reference< 
			::com::sun::star::xml::sax::XAttributeList> & xAttrList);

	virtual void EndElement();

	virtual SvXMLImportContext *CreateChildContext( 
		sal_uInt16 nPrefix,
		const ::rtl::OUString& rLocalName,
		const ::com::sun::star::uno::Reference< 
			::com::sun::star::xml::sax::XAttributeList> & xAttrList );

	void ProcessAttributes(
		const ::com::sun::star::uno::Reference< 
			::com::sun::star::xml::sax::XAttributeList> & xAttrList );
};

}//end of namespace binfilter
#endif
