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



#ifndef _XMLOFF_XMLEMBEDDEDOBJECTEXPORTFILTER_HXX
#include "XMLEmbeddedObjectExportFilter.hxx"
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;


XMLEmbeddedObjectExportFilter::XMLEmbeddedObjectExportFilter(
		const Reference< XDocumentHandler > & rHandler ) throw() :
	xHandler( rHandler ),
	xExtHandler( rHandler, UNO_QUERY )
{
}

XMLEmbeddedObjectExportFilter::~XMLEmbeddedObjectExportFilter () throw()
{
}


void SAL_CALL XMLEmbeddedObjectExportFilter::startDocument( void )
	throw( SAXException, RuntimeException )
{
	// do nothing, filter this
}

void SAL_CALL XMLEmbeddedObjectExportFilter::endDocument( void )
	throw( SAXException, RuntimeException)
{
	// do nothing, filter this
}

void SAL_CALL XMLEmbeddedObjectExportFilter::startElement(
		const OUString& rName,
		const Reference< XAttributeList >& xAttrList )
	throw(SAXException, RuntimeException)
{
	xHandler->startElement( rName, xAttrList );
}

void SAL_CALL XMLEmbeddedObjectExportFilter::endElement( const OUString& rName )
	throw(SAXException, RuntimeException)
{
	xHandler->endElement( rName );
}

void SAL_CALL XMLEmbeddedObjectExportFilter::characters( const OUString& rChars )
	throw(SAXException, RuntimeException)
{
	xHandler->characters( rChars );
}

void SAL_CALL XMLEmbeddedObjectExportFilter::ignorableWhitespace(
		const OUString& rWhitespaces )
	throw(SAXException, RuntimeException)
{
	xHandler->ignorableWhitespace( rWhitespaces );
}

void SAL_CALL XMLEmbeddedObjectExportFilter::processingInstruction(
		const OUString& rTarget,
	    const OUString& rData )
	throw(SAXException, RuntimeException)
{
	xHandler->processingInstruction( rTarget, rData );
}

void SAL_CALL XMLEmbeddedObjectExportFilter::setDocumentLocator(
		const Reference< XLocator >& rLocator )
	throw(SAXException, RuntimeException)
{
	xHandler->setDocumentLocator( rLocator );
}

// XExtendedDocumentHandler
void SAL_CALL XMLEmbeddedObjectExportFilter::startCDATA( void )
	throw(SAXException, RuntimeException)
{
	if( xExtHandler.is() )
		xExtHandler->startCDATA();
}

void SAL_CALL XMLEmbeddedObjectExportFilter::endCDATA( void )
	throw(RuntimeException)
{
	if( xExtHandler.is() )
		xExtHandler->endCDATA();
}

void SAL_CALL XMLEmbeddedObjectExportFilter::comment( const OUString& rComment )
	throw(SAXException, RuntimeException)
{
	if( xExtHandler.is() )
		xExtHandler->comment( rComment );
}

void SAL_CALL XMLEmbeddedObjectExportFilter::allowLineBreak( void )
	throw(SAXException, RuntimeException)
{
	if( xExtHandler.is() )
		xExtHandler->allowLineBreak();
}

void SAL_CALL XMLEmbeddedObjectExportFilter::unknown( const OUString& rString )
	throw(SAXException, RuntimeException)
{
	if( xExtHandler.is() )
		xExtHandler->unknown( rString );
}

// XInitialize
void SAL_CALL XMLEmbeddedObjectExportFilter::initialize(
		const Sequence< Any >& aArguments )
	throw(Exception, RuntimeException)
{
	const sal_Int32 nAnyCount = aArguments.getLength();
	const Any* pAny = aArguments.getConstArray();

	for( sal_Int32 nIndex = 0; nIndex < nAnyCount; nIndex++, pAny++ )
	{
		if( pAny->getValueType() ==
				::getCppuType((const Reference< XDocumentHandler >*)0))
		{
			*pAny >>= xHandler;
			*pAny >>= xExtHandler;
		}
	}
}

// XServiceInfo
OUString SAL_CALL XMLEmbeddedObjectExportFilter::getImplementationName()
	throw(RuntimeException)
{
	OUString aStr;
	return aStr;
}

sal_Bool SAL_CALL XMLEmbeddedObjectExportFilter::supportsService( const OUString& ServiceName )
	throw(RuntimeException)
{
	return sal_False;
}

Sequence< OUString > SAL_CALL XMLEmbeddedObjectExportFilter::getSupportedServiceNames(  )
	throw(RuntimeException)
{
	Sequence< OUString > aSeq;
	return aSeq;
}
}//end of namespace binfilter
