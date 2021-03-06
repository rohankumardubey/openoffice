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



#ifndef _XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX
#define _XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX


#ifndef _COM_SUN_STAR_UCB_XANYCOMPARE_HPP_ 
#include <com/sun/star/ucb/XAnyCompare.hpp>
#endif

namespace com { namespace sun { namespace star { namespace container {
	class XIndexReplace; } } } }
namespace rtl { class OUString; }
namespace binfilter {

class XMLTextListAutoStylePool_Impl;
class XMLTextListAutoStylePoolNames_Impl;
class XMLTextListAutoStylePoolEntry_Impl;
class SvXMLExport;

class XMLTextListAutoStylePool
{
	SvXMLExport& rExport;

	const ::rtl::OUString sPrefix;

	XMLTextListAutoStylePool_Impl *pPool;
	XMLTextListAutoStylePoolNames_Impl *pNames;
	sal_uInt32 nName;

	/** this is an optional NumRule compare component for applications where
		the NumRules don't have names */
	::com::sun::star::uno::Reference< ::com::sun::star::ucb::XAnyCompare > mxNumRuleCompare;

	sal_uInt32 Find( XMLTextListAutoStylePoolEntry_Impl* pEntry ) const;
public:

	XMLTextListAutoStylePool( SvXMLExport& rExport );
	~XMLTextListAutoStylePool();

	void RegisterName( const ::rtl::OUString& rName );

	::rtl::OUString Add(
			const ::com::sun::star::uno::Reference <
				::com::sun::star::container::XIndexReplace > & rNumRules );

	::rtl::OUString Find(
			const ::com::sun::star::uno::Reference <
				::com::sun::star::container::XIndexReplace > & rNumRules ) const;
	::rtl::OUString Find( const ::rtl::OUString& rInternalName ) const;

	void exportXML() const; 
};

}//end of namespace binfilter
#endif	//  _XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX
