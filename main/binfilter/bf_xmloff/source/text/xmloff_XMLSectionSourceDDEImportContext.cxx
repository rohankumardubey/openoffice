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



#ifndef _XMLOFF_XMLSECTIONSOURCEDDEIMPORTCONTEXT_HXX_
#include "XMLSectionSourceDDEImportContext.hxx"
#endif




#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif


#ifndef _XMLOFF_NMSPMAP_HXX 
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif


#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_ 
#include <com/sun/star/uno/Reference.h>
#endif



#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
namespace binfilter {

using ::rtl::OUString;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XMultiPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::binfilter::xmloff::token;

const sal_Char sAPI_DDECommandFile[] = "DDECommandFile";
const sal_Char sAPI_DDECommandType[] = "DDECommandType";
const sal_Char sAPI_DDECommandElement[] = "DDECommandElement";
const sal_Char sAPI_IsAutomaticUpdate[] = "IsAutomaticUpdate";


TYPEINIT1(XMLSectionSourceDDEImportContext, SvXMLImportContext);

XMLSectionSourceDDEImportContext::XMLSectionSourceDDEImportContext(
	SvXMLImport& rImport, 
	sal_uInt16 nPrfx,
	const OUString& rLocalName,
	Reference<XPropertySet> & rSectPropSet) :
		SvXMLImportContext(rImport, nPrfx, rLocalName),
		rSectionPropertySet(rSectPropSet),
		sDdeCommandFile(RTL_CONSTASCII_USTRINGPARAM(sAPI_DDECommandFile)),
		sDdeCommandType(RTL_CONSTASCII_USTRINGPARAM(sAPI_DDECommandType)),
	   sDdeCommandElement(RTL_CONSTASCII_USTRINGPARAM(sAPI_DDECommandElement)),
		sIsAutomaticUpdate(RTL_CONSTASCII_USTRINGPARAM(sAPI_IsAutomaticUpdate))
{
}

XMLSectionSourceDDEImportContext::~XMLSectionSourceDDEImportContext()
{
}

enum XMLSectionSourceDDEToken 
{
	XML_TOK_SECTION_DDE_APPLICATION,
	XML_TOK_SECTION_DDE_TOPIC,
	XML_TOK_SECTION_DDE_ITEM,
	XML_TOK_SECTION_IS_AUTOMATIC_UPDATE
};

static __FAR_DATA SvXMLTokenMapEntry aSectionSourceDDETokenMap[] =
{
	{ XML_NAMESPACE_OFFICE, XML_DDE_APPLICATION, 
		  XML_TOK_SECTION_DDE_APPLICATION },
	{ XML_NAMESPACE_OFFICE, XML_DDE_TOPIC, XML_TOK_SECTION_DDE_TOPIC },
	{ XML_NAMESPACE_OFFICE, XML_DDE_ITEM, XML_TOK_SECTION_DDE_ITEM },
	{ XML_NAMESPACE_OFFICE, XML_AUTOMATIC_UPDATE, 
		  XML_TOK_SECTION_IS_AUTOMATIC_UPDATE },
	XML_TOKEN_MAP_END
};


void XMLSectionSourceDDEImportContext::StartElement(
	const Reference<XAttributeList> & xAttrList)
{
	SvXMLTokenMap aTokenMap(aSectionSourceDDETokenMap);
	OUString sApplication;
	OUString sTopic;
	OUString sItem;
	sal_Bool bAutomaticUpdate = sal_False;
	
	sal_Int16 nLength = xAttrList->getLength();
	for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
	{
		OUString sLocalName;
		sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
			GetKeyByAttrName( xAttrList->getNameByIndex(nAttr), 
							  &sLocalName );

		switch (aTokenMap.Get(nPrefix, sLocalName))
		{
			case XML_TOK_SECTION_DDE_APPLICATION:
				sApplication = xAttrList->getValueByIndex(nAttr);
				break;
			case XML_TOK_SECTION_DDE_TOPIC:
				sTopic = xAttrList->getValueByIndex(nAttr);
				break;
			case XML_TOK_SECTION_DDE_ITEM:
				sItem = xAttrList->getValueByIndex(nAttr);
				break;
			case XML_TOK_SECTION_IS_AUTOMATIC_UPDATE:
			{
				sal_Bool bTmp;
				if (SvXMLUnitConverter::convertBool(
					bTmp, xAttrList->getValueByIndex(nAttr)))
				{
					bAutomaticUpdate = bTmp;
				}
				break;
			}
			default:
				; // ignore
				break;
		}
	}

	// DDE not supported on all platforms; query property first
	if (rSectionPropertySet->getPropertySetInfo()->
		hasPropertyByName(sDdeCommandFile))
	{
		// use multi property set to force single update of connection #83654#
		Sequence<OUString> aNames(4);
		Sequence<Any> aValues(4);

		aValues[0] <<= sApplication;
		aNames[0] = sDdeCommandFile;

		aValues[1] <<= sTopic;
		aNames[1] = sDdeCommandType;

		aValues[2] <<= sItem;
		aNames[2] = sDdeCommandElement;

		aValues[3].setValue(&bAutomaticUpdate, ::getBooleanCppuType());
		aNames[3] = sIsAutomaticUpdate;

		Reference<XMultiPropertySet> rMultiPropSet(rSectionPropertySet, 
												   UNO_QUERY);
		DBG_ASSERT(rMultiPropSet.is(), "we'd really like a XMultiPropertySet");
		if (rMultiPropSet.is())
			rMultiPropSet->setPropertyValues(aNames, aValues);
		// else: ignore
	}
}

void XMLSectionSourceDDEImportContext::EndElement()
{
	// nothing to be done!
}

SvXMLImportContext* XMLSectionSourceDDEImportContext::CreateChildContext( 
	sal_uInt16 nPrefix,
	const OUString& rLocalName,
	const Reference<XAttributeList> & xAttrList )
{
	// ignore -> default context
	return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}
}//end of namespace binfilter
