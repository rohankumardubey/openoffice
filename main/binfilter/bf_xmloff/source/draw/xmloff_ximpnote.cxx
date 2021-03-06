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



#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _XIMPNOTES_HXX
#include "ximpnote.hxx"
#endif

namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////

SdXMLNotesContext::SdXMLNotesContext( SdXMLImport& rImport,
	USHORT nPrfx, const OUString& rLocalName,
	const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
	uno::Reference< drawing::XShapes >& rShapes) 
:	SdXMLGenericPageContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
{
	const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
	for(sal_Int16 i=0; i < nAttrCount; i++)
	{
		OUString sAttrName = xAttrList->getNameByIndex( i );
		OUString aLocalName;
		sal_uInt16 nPrefix = GetSdImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
		OUString sValue = xAttrList->getValueByIndex( i );
		const SvXMLTokenMap& rAttrTokenMap = GetSdImport().GetMasterPageAttrTokenMap();

		switch(rAttrTokenMap.Get(nPrefix, aLocalName))
		{
			case XML_TOK_MASTERPAGE_PAGE_MASTER_NAME:
			{
				msPageMasterName = sValue;
				break;
			}
		}
	}

	// now delete all up-to-now contained shapes from this notes page
	uno::Reference< drawing::XShape > xShape;
	while(rShapes->getCount())
	{
		rShapes->getByIndex(0L) >>= xShape;
		if(xShape.is())
			rShapes->remove(xShape);
	}

	// set page-master?
	if(msPageMasterName.getLength())
	{
		SetPageMaster( msPageMasterName );
	}
}

//////////////////////////////////////////////////////////////////////////////

SdXMLNotesContext::~SdXMLNotesContext()
{
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext *SdXMLNotesContext::CreateChildContext( USHORT nPrefix,
	const OUString& rLocalName,
	const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
	// OK, notes page is set on base class, objects can be imported on notes page
	SvXMLImportContext *pContext = 0L;

	// some special objects inside presentation:notes context
	// ...







	// call parent when no own context was created
	if(!pContext)
		pContext = SdXMLGenericPageContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

	return pContext;
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLNotesContext::EndElement()
{
	SdXMLGenericPageContext::EndElement();
}
}//end of namespace binfilter
