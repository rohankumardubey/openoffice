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



#ifndef _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX
#define _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include <bf_xmloff/xmlictxt.hxx>
#endif

#ifndef _XMLOFF_XMLEVENT_HXX
#include <bf_xmloff/xmlevent.hxx>
#endif

#include <map>
#include <vector>

namespace com { namespace sun { namespace star {
	namespace xml { namespace sax {	class XAttributeList; } }
	namespace beans { struct PropertyValue;	}
	namespace container { class XNameReplace; }
	namespace document { class XEventsSupplier; }
} } }
namespace rtl {	class OUString; }
namespace binfilter {

typedef ::std::pair<
			::rtl::OUString,
			::com::sun::star::uno::Sequence<
				::com::sun::star::beans::PropertyValue> > EventNameValuesPair;

typedef ::std::vector< EventNameValuesPair > EventsVector;

/**
 * Import <script:events> element. 
 * 
 * The import context usually sets the events immediatly at the event
 * XNameReplace. If none was given on construction, it operates in 
 * delayed mode: All events are collected and may then be set
 * with the setEvents() method.  
 */
class XMLEventsImportContext : public SvXMLImportContext
{
protected:
	// the event XNameReplace; may be empty
	::com::sun::star::uno::Reference< 
		::com::sun::star::container::XNameReplace> xEvents;

	// if no XNameReplace is given, use this vector to collect events
	EventsVector aCollectEvents;

public:

	TYPEINFO();

	XMLEventsImportContext(
		SvXMLImport& rImport, 
		sal_uInt16 nPrfx,
		const ::rtl::OUString& rLocalName);

	XMLEventsImportContext(
		SvXMLImport& rImport, 
		sal_uInt16 nPrfx,
		const ::rtl::OUString& rLocalName,
		const ::com::sun::star::uno::Reference< 
			::com::sun::star::document::XEventsSupplier> & xEventsSupplier);

	~XMLEventsImportContext();

	void AddEventValues(
		const ::rtl::OUString& rEventName,
		const ::com::sun::star::uno::Sequence< 
			::com::sun::star::beans::PropertyValue> & rValues);

    /// if the import operates in delayed mode, you can use this method
    /// to set all events that have been read on the XEventsSupplier
	void SetEvents(
		const ::com::sun::star::uno::Reference< 
			::com::sun::star::document::XEventsSupplier> & xEventsSupplier);

    /// if the import operates in delayed mode, you can use this method
    /// to set all events that have been read on the XNameReplace
	void SetEvents(
		const ::com::sun::star::uno::Reference< 
			::com::sun::star::container::XNameReplace> & xNameRepl);

    /// if the import operates indelayed mode, you can use this method
    /// to obtain the value sequence for a specific event
    sal_Bool GetEventSequence(
        const ::rtl::OUString& rName,
        ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue> & rSequence );

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
};

}//end of namespace binfilter
#endif
