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



#ifndef _XMLOFF_FORMS_EVENTIMPORT_HXX_
#include "eventimport.hxx"
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _XMLOFF_FORMS_STRINGS_HXX_
#include "strings.hxx"
#endif
namespace binfilter {

//.........................................................................
namespace xmloff
{
//.........................................................................

	using namespace ::com::sun::star::uno;
	using namespace ::com::sun::star::beans;
	using namespace ::com::sun::star::script;
	using namespace ::com::sun::star::container;

	//=====================================================================
	//= OFormEventsImportContext
	//=====================================================================
	//---------------------------------------------------------------------
	OFormEventsImportContext::OFormEventsImportContext(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName, IEventAttacher& _rEventAttacher)
		:XMLEventsImportContext(_rImport, _nPrefix, _rLocalName)
		,m_rEventAttacher(_rEventAttacher)
	{
	}

	//---------------------------------------------------------------------
	void OFormEventsImportContext::EndElement()
	{
		Sequence< ScriptEventDescriptor > aTranslated(aCollectEvents.size());
		ScriptEventDescriptor* pTranslated = aTranslated.getArray();

		// loop through the collected events and translate them
		const PropertyValue* pEventDescription;
		const PropertyValue* pEventDescriptionEnd;
		sal_Int32 nSeparatorPos = -1;
		for	(	EventsVector::const_iterator aEvent = aCollectEvents.begin();
				aEvent != aCollectEvents.end();
				++aEvent, ++pTranslated
			)
		{
			// the name of the event is built from ListenerType::EventMethod
			nSeparatorPos = aEvent->first.indexOf(EVENT_NAME_SEPARATOR);
			OSL_ENSURE(-1 != nSeparatorPos, "OFormEventsImportContext::EndElement: invalid (unrecognized) event name!");
			pTranslated->ListenerType = aEvent->first.copy(0, nSeparatorPos);
			pTranslated->EventMethod = aEvent->first.copy(nSeparatorPos + EVENT_NAME_SEPARATOR.length);

			::rtl::OUString sLibrary;

			// the local macro name and the event type are specified as properties
			pEventDescription		=						aEvent->second.getConstArray();
			pEventDescriptionEnd	=	pEventDescription + aEvent->second.getLength();
			for (;pEventDescription != pEventDescriptionEnd; ++pEventDescription)
			{
				if (0 == pEventDescription->Name.compareToAscii(EVENT_LOCALMACRONAME))
					pEventDescription->Value >>= pTranslated->ScriptCode;
				else if (0 == pEventDescription->Name.compareToAscii(EVENT_TYPE))
					pEventDescription->Value >>= pTranslated->ScriptType;
				else if ( 0 == pEventDescription->Name.compareToAscii( EVENT_LIBRARY ) )
					pEventDescription->Value >>= sLibrary;
			}

			if ( 0 == pTranslated->ScriptType.compareToAscii( EVENT_STARBASIC ) )
			{
				if ( 0 == sLibrary.compareToAscii( EVENT_STAROFFICE ) )
					sLibrary = EVENT_APPLICATION;

				if ( sLibrary.getLength() )
				{
					// for StarBasic, the library is prepended
					sal_Unicode cLibSeparator = ':';
					sLibrary += ::rtl::OUString( &cLibSeparator, 1 );
				}
				sLibrary += pTranslated->ScriptCode;
				pTranslated->ScriptCode = sLibrary;
			}
		}

		// register the events
		m_rEventAttacher.registerEvents(aTranslated);

		XMLEventsImportContext::EndElement();
	}

	//=====================================================================
	//= ODefaultEventAttacherManager
	//=====================================================================
	//-------------------------------------------------------------------------
	void ODefaultEventAttacherManager::registerEvents(const Reference< XPropertySet >& _rxElement,
		const Sequence< ScriptEventDescriptor >& _rEvents)
	{
		OSL_ENSURE(m_aEvents.end() == m_aEvents.find(_rxElement),
			"ODefaultEventAttacherManager::registerEvents: already have events for this object!");
		// for the moment, only remember the script events
		m_aEvents[_rxElement] = _rEvents;
	}

	//-------------------------------------------------------------------------
	void ODefaultEventAttacherManager::setEvents(const Reference< XIndexAccess >& _rxContainer)
	{
		Reference< XEventAttacherManager > xEventManager(_rxContainer, UNO_QUERY);
		if (!xEventManager.is())
		{
			OSL_ENSURE(sal_False, "ODefaultEventAttacherManager::setEvents: invalid argument!");
			return;
		}

		// loop through all elements
		sal_Int32 nCount = _rxContainer->getCount();
		Reference< XPropertySet > xCurrent;
		ConstMapPropertySet2ScriptSequenceIterator aRegisteredEventsPos;
		for (sal_Int32 i=0; i<nCount; ++i)
		{
			::cppu::extractInterface(xCurrent, _rxContainer->getByIndex(i));
			if (xCurrent.is())
			{
				aRegisteredEventsPos = m_aEvents.find(xCurrent);
				if (m_aEvents.end() != aRegisteredEventsPos)
					xEventManager->registerScriptEvents(i, aRegisteredEventsPos->second);
			}
		}
	}

//.........................................................................
}	// namespace xmloff
//.........................................................................


}//end of namespace binfilter
