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



#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#ifndef _SC_XMLCALCULATIONSETTINGSCONTEXT_HXX
#include "XMLCalculationSettingsContext.hxx"
#endif
#ifndef SC_XMLIMPRT_HXX
#include "xmlimprt.hxx"
#endif
#ifndef SC_UNONAMES_HXX
#include "unonames.hxx"
#endif
#ifndef SC_DOCOPTIO_HXX
#include "docoptio.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <bf_xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <bf_xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <bf_xmloff/nmspmap.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETDOCUMENT_HPP_
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
namespace binfilter {

using namespace ::com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLCalculationSettingsContext::ScXMLCalculationSettingsContext( ScXMLImport& rImport,
									  USHORT nPrfx,
									  const ::rtl::OUString& rLName,
									  const ::com::sun::star::uno::Reference<
									  ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
	SvXMLImportContext( rImport, nPrfx, rLName ),
	fIterationEpsilon(0.001),
	nIterationCount(100),
	nYear2000(1930),
	bIsIterationEnabled(sal_False),
	bCalcAsShown(sal_False),
	bIgnoreCase(sal_False),
	bLookUpLabels(sal_True),
	bMatchWholeCell(sal_True),
	bUseRegularExpressions(sal_True)
{
	aNullDate.Day = 30;
	aNullDate.Month = 12;
	aNullDate.Year = 1899;
	sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
	for( sal_Int16 i=0; i < nAttrCount; i++ )
	{
		::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
		::rtl::OUString aLocalName;
		sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
											sAttrName, &aLocalName );
		::rtl::OUString sValue = xAttrList->getValueByIndex( i );

		if (nPrefix == XML_NAMESPACE_TABLE)
		{
			if (IsXMLToken(aLocalName, XML_CASE_SENSITIVE))
			{
				if (IsXMLToken(sValue, XML_FALSE))
					bIgnoreCase = sal_True;
			}
			else if (IsXMLToken(aLocalName, XML_PRECISION_AS_SHOWN))
			{
				if (IsXMLToken(sValue, XML_TRUE))
					bCalcAsShown = sal_True;
			}
			else if (IsXMLToken(aLocalName, XML_SEARCH_CRITERIA_MUST_APPLY_TO_WHOLE_CELL))
			{
				if (IsXMLToken(sValue, XML_FALSE))
					bMatchWholeCell = sal_False;
			}
			else if (IsXMLToken(aLocalName, XML_AUTOMATIC_FIND_LABELS))
			{
				if (IsXMLToken(sValue, XML_FALSE))
					bLookUpLabels = sal_False;
			}
			else if (IsXMLToken(aLocalName, XML_NULL_YEAR))
			{
				sal_Int32 nTemp;
				GetScImport().GetMM100UnitConverter().convertNumber(nTemp, sValue);
				nYear2000 = static_cast<sal_uInt16>(nTemp);
			}
			else if (IsXMLToken(aLocalName, XML_USE_REGULAR_EXPRESSIONS))
			{
				if (IsXMLToken(sValue, XML_FALSE))
					bUseRegularExpressions = sal_False;
			}
		}
	}
}

ScXMLCalculationSettingsContext::~ScXMLCalculationSettingsContext()
{
}

SvXMLImportContext *ScXMLCalculationSettingsContext::CreateChildContext( USHORT nPrefix,
											const ::rtl::OUString& rLName,
											const ::com::sun::star::uno::Reference<
									  	::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
	SvXMLImportContext *pContext = 0;

	if (nPrefix == XML_NAMESPACE_TABLE)
	{
		if (IsXMLToken(rLName, XML_NULL_DATE))
			pContext = new ScXMLNullDateContext(GetScImport(), nPrefix, rLName, xAttrList, this);
		else if (IsXMLToken(rLName, XML_ITERATION))
			pContext = new ScXMLIterationContext(GetScImport(), nPrefix, rLName, xAttrList, this);
	}

	if( !pContext )
		pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

	return pContext;
}

void ScXMLCalculationSettingsContext::EndElement()
{
	if (GetScImport().GetModel().is())
	{
		uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc (GetScImport().GetModel(), uno::UNO_QUERY);
		if (xSpreadDoc.is())
		{
			uno::Reference <beans::XPropertySet> xPropertySet (xSpreadDoc, uno::UNO_QUERY);
			if (xPropertySet.is())
			{
				uno::Any aAny = ::cppu::bool2any( bCalcAsShown );
				xPropertySet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_CALCASSHOWN)), aAny );
				aAny = ::cppu::bool2any( bIgnoreCase );
				xPropertySet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_IGNORECASE)), aAny );
				aAny = ::cppu::bool2any( bLookUpLabels );
				xPropertySet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_LOOKUPLABELS)), aAny );
				aAny = ::cppu::bool2any( bMatchWholeCell );
				xPropertySet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_MATCHWHOLE)), aAny );
				aAny = ::cppu::bool2any( bUseRegularExpressions );
				xPropertySet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_REGEXENABLED)), aAny );
				aAny = ::cppu::bool2any( bIsIterationEnabled );
				xPropertySet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ITERENABLED)), aAny );
				aAny <<= nIterationCount;
				xPropertySet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ITERCOUNT)), aAny);
				aAny <<= fIterationEpsilon;
				xPropertySet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ITEREPSILON)), aAny);
				aAny <<= aNullDate;
				xPropertySet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_NULLDATE)), aAny);
				if (GetScImport().GetDocument())
				{
					GetScImport().LockSolarMutex();
					ScDocOptions aDocOptions (GetScImport().GetDocument()->GetDocOptions());
					aDocOptions.SetYear2000(nYear2000);
					GetScImport().GetDocument()->SetDocOptions(aDocOptions);
					GetScImport().UnlockSolarMutex();
				}
			}
		}
	}
}

ScXMLNullDateContext::ScXMLNullDateContext( ScXMLImport& rImport,
									  USHORT nPrfx,
									  const ::rtl::OUString& rLName,
									  const ::com::sun::star::uno::Reference<
									  ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
									  ScXMLCalculationSettingsContext* pCalcSet) :
	SvXMLImportContext( rImport, nPrfx, rLName )
{
	sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
	for( sal_Int16 i=0; i < nAttrCount; i++ )
	{
		::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
		::rtl::OUString aLocalName;
		sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
											sAttrName, &aLocalName );
		::rtl::OUString sValue = xAttrList->getValueByIndex( i );

		if (nPrefix == XML_NAMESPACE_TABLE && IsXMLToken(aLocalName, XML_DATE_VALUE))
		{
			util::DateTime aDateTime;
			GetScImport().GetMM100UnitConverter().convertDateTime(aDateTime, sValue);
			util::Date aDate;
			aDate.Day = aDateTime.Day;
			aDate.Month = aDateTime.Month;
			aDate.Year = aDateTime.Year;
			pCalcSet->SetNullDate(aDate);
		}
	}
}

ScXMLNullDateContext::~ScXMLNullDateContext()
{
}

SvXMLImportContext *ScXMLNullDateContext::CreateChildContext( USHORT nPrefix,
											const ::rtl::OUString& rLName,
											const ::com::sun::star::uno::Reference<
									  	::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
	SvXMLImportContext *pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

	return pContext;
}

void ScXMLNullDateContext::EndElement()
{
}

ScXMLIterationContext::ScXMLIterationContext( ScXMLImport& rImport,
									  USHORT nPrfx,
									  const ::rtl::OUString& rLName,
									  const ::com::sun::star::uno::Reference<
									  ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
									  ScXMLCalculationSettingsContext* pCalcSet) :
	SvXMLImportContext( rImport, nPrfx, rLName )
{
	sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
	for( sal_Int16 i=0; i < nAttrCount; i++ )
	{
		::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
		::rtl::OUString aLocalName;
		sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
											sAttrName, &aLocalName );
		::rtl::OUString sValue = xAttrList->getValueByIndex( i );

		if (nPrefix == XML_NAMESPACE_TABLE)
		{
			if (IsXMLToken(aLocalName, XML_STATUS))
			{
				if (IsXMLToken(sValue, XML_ENABLE))
					pCalcSet->SetIterationStatus(sal_True);
			}
			else if (IsXMLToken(aLocalName, XML_STEPS))
			{
				sal_Int32 nSteps;
				GetScImport().GetMM100UnitConverter().convertNumber(nSteps, sValue);
				pCalcSet->SetIterationCount(nSteps);
			}
			else if (IsXMLToken(aLocalName, XML_MAXIMUM_DIFFERENCE))
			{
				double fDif;
				GetScImport().GetMM100UnitConverter().convertDouble(fDif, sValue);
				pCalcSet->SetIterationEpsilon(fDif);
			}
		}
	}
}

ScXMLIterationContext::~ScXMLIterationContext()
{
}

SvXMLImportContext *ScXMLIterationContext::CreateChildContext( USHORT nPrefix,
											const ::rtl::OUString& rLName,
											const ::com::sun::star::uno::Reference<
									  	::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
	SvXMLImportContext *pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

	return pContext;
}

void ScXMLIterationContext::EndElement()
{
}
}
