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



#ifndef _XMLOFF_XMLNUMI_HXX
#define _XMLOFF_XMLNUMI_HXX

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif

#include <bf_xmloff/xmlstyle.hxx>

#ifndef _COM_SUN_STAR_STYLE_NUMBERINGTYPE_HPP_
#include <com/sun/star/style/NumberingType.hpp>
#endif

namespace com { namespace sun { namespace star { namespace frame { class XModel; } } } }
namespace binfilter {
class SvI18NMap;
class SvxXMLListStyle_Impl;

class SvxXMLListStyleContext : public SvXMLStyleContext
{
	const ::rtl::OUString		sIsPhysical;
	const ::rtl::OUString		sNumberingRules;
	const ::rtl::OUString		sName;
	const ::rtl::OUString		sIsContinuousNumbering;
	const ::rtl::OUString		sIsNumbering;

	::com::sun::star::uno::Reference <
		::com::sun::star::container::XIndexReplace > xNumRules;

	SvxXMLListStyle_Impl		*pLevelStyles;

	sal_Int16					nLevels;
	sal_Bool					bConsecutive : 1;
	sal_Bool					bOutline : 1;

protected:

	virtual void SetAttribute( sal_uInt16 nPrefixKey,
							   const ::rtl::OUString& rLocalName,
							   const ::rtl::OUString& rValue );

public:

	TYPEINFO();

	SvxXMLListStyleContext(
			SvXMLImport& rImport, sal_uInt16 nPrfx,
			const ::rtl::OUString& rLName,
			const ::com::sun::star::uno::Reference<
					::com::sun::star::xml::sax::XAttributeList >& xAttrList,
			sal_Bool bOutl=sal_False );

	virtual ~SvxXMLListStyleContext();

	virtual SvXMLImportContext *CreateChildContext(
			sal_uInt16 nPrefix,
			const ::rtl::OUString& rLocalName,
			const ::com::sun::star::uno::Reference<
					::com::sun::star::xml::sax::XAttributeList >& xAttrList );

	void FillUnoNumRule(
			const ::com::sun::star::uno::Reference<
					::com::sun::star::container::XIndexReplace > & rNumRule,
			const SvI18NMap *pI18NMap ) const;

	const ::com::sun::star::uno::Reference <
		::com::sun::star::container::XIndexReplace >& GetNumRules() const
		{ return xNumRules; }
	sal_Bool IsOutline() const { return bOutline; }
	sal_Bool IsConsecutive() const { return bConsecutive; }
	sal_Int16 GetLevels() const { return nLevels; }

	static ::com::sun::star::uno::Reference <
		::com::sun::star::container::XIndexReplace >
	CreateNumRule(
		const ::com::sun::star::uno::Reference <
			::com::sun::star::frame::XModel > & rModel );

	static void SetDefaultStyle(
		const ::com::sun::star::uno::Reference <
			::com::sun::star::container::XIndexReplace > & rNumRule,
		sal_Int16 nLevel,
		sal_Bool bOrdered );

	virtual void CreateAndInsertLate( sal_Bool bOverwrite );

	void CreateAndInsertAuto() const;
};

}//end of namespace binfilter
#endif	//  _XMLOFF_XMLNUMI_HXX

