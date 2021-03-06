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



#ifndef _XMLOFF_EVENTIMPORTHELPER_HXX
#define _XMLOFF_EVENTIMPORTHELPER_HXX


#ifndef _XMLOFF_XMLEVENT_HXX
#include "xmlevent.hxx"
#endif

#include <map>
#include <list>


namespace com { namespace sun { namespace star {
	namespace xml { namespace sax {	class XAttributeList; } }
} } }
namespace rtl {	class OUString; }
namespace binfilter {
class XMLEventContextFactory;
class XMLEventsImportContext;
struct XMLEventNameTranslation;

typedef ::std::map< ::rtl::OUString, XMLEventContextFactory* > FactoryMap;
typedef ::std::map< ::rtl::OUString, ::rtl::OUString > NameMap;
typedef ::std::list< NameMap* > NameMapList;


/**
 * Helps the XMLEventsImportContext.
 * 
 * This class stores
 * a) the translation from XML event names to API event names, and
 * b) a mapping from script language names to XMLEventContextFactory objects
 *    (that handle particular languages).
 *
 * Event name translation tables may be added, i.e. they will be joined 
 * together. If different translations are needed (i.e., if the same XML name
 * needs to be translated to different API names in different contexts), then
 * translation tables may be saved on a translation table stack.
 */
class XMLEventImportHelper
{
	/// map of XMLEventContextFactory objects
	FactoryMap aFactoryMap;

	/// map from XML to API names
	NameMap* pEventNameMap;

	/// stack of previous aEventNameMap
	NameMapList aEventNameMapList;

public:
	XMLEventImportHelper();

	~XMLEventImportHelper();

	/// register a handler for a particular language type
	void RegisterFactory( const ::rtl::OUString& rLanguage,
						  XMLEventContextFactory* aFactory );

	/// add event name translation to the internal table
	void AddTranslationTable( const XMLEventNameTranslation* pTransTable );

	/// save the old translation table on a stack and install an empty table
	void PushTranslationTable();

	/// recover the top-most previously saved translation table
	void PopTranslationTable();

	/// create an appropriate import context for a particular event
	SvXMLImportContext* CreateContext(
		SvXMLImport& rImport,
		sal_uInt16 nPrefix,
		const ::rtl::OUString& rLocalName,
		const ::com::sun::star::uno::Reference< 
			::com::sun::star::xml::sax::XAttributeList> & xAttrList,
		XMLEventsImportContext* rEvents,
		const ::rtl::OUString& rXmlEventName,
		const ::rtl::OUString& rLanguage);

};

}//end of namespace binfilter
#endif
