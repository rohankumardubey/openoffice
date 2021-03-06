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



#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSTATES_HPP_ 
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_TEXTMASTERPAGECONTEXT_HXX_
#include "XMLTextMasterPageContext.hxx"
#endif
#ifndef _XMLOFF_TEXTHEADERFOOTERCONTEXT_HXX_
#include "XMLTextHeaderFooterContext.hxx"
#endif
#ifndef _XMLOFF_PAGEMASTERIMPORTCONTEXT_HXX
#include "PageMasterImportContext.hxx"
#endif
namespace binfilter {


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
//using namespace ::com::sun::star::text;
using namespace ::binfilter::xmloff::token;

Reference < XStyle > XMLTextMasterPageContext::Create()
{
	Reference < XStyle > xNewStyle;

	Reference< XMultiServiceFactory > xFactory( GetImport().GetModel(),
													UNO_QUERY );
	if( xFactory.is() )
	{
		Reference < XInterface > xIfc =
			xFactory->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM(
									"com.sun.star.style.PageStyle")) );
		if( xIfc.is() )
			xNewStyle = Reference < XStyle >( xIfc, UNO_QUERY );
	}

	return xNewStyle;
}
TYPEINIT1( XMLTextMasterPageContext, SvXMLStyleContext );

XMLTextMasterPageContext::XMLTextMasterPageContext( SvXMLImport& rImport,
		sal_uInt16 nPrfx, const OUString& rLName,
		const Reference< XAttributeList > & xAttrList,
		sal_Bool bOverwrite ) :
	SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList, XML_STYLE_FAMILY_MASTER_PAGE ),
	sIsPhysical( RTL_CONSTASCII_USTRINGPARAM( "IsPhysical" ) ),
	sFollowStyle( RTL_CONSTASCII_USTRINGPARAM( "FollowStyle" ) ),
	sPageStyleLayout( RTL_CONSTASCII_USTRINGPARAM( "PageStyleLayout" ) ),
	sPageMasterName(),
	bInsertHeader( sal_False ),
	bInsertFooter( sal_False ),
	bInsertHeaderLeft( sal_False ),
	bInsertFooterLeft( sal_False ),
	bHeaderInserted( sal_False ),
	bFooterInserted( sal_False ),
	bHeaderLeftInserted( sal_False ),
	bFooterLeftInserted( sal_False )
{
	OUString sName;
	sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
	for( sal_Int16 i=0; i < nAttrCount; i++ )
	{
		const OUString& rAttrName = xAttrList->getNameByIndex( i );
		OUString aLocalName;
		sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,	&aLocalName );
		if( XML_NAMESPACE_STYLE == nPrefix )
		{
			if( IsXMLToken( aLocalName, XML_NAME ) )
			{
				sName = xAttrList->getValueByIndex( i );
			}
			else if( IsXMLToken( aLocalName, XML_NEXT_STYLE_NAME ) )
			{
				sFollow = xAttrList->getValueByIndex( i );
			}
			else if( IsXMLToken( aLocalName, XML_PAGE_MASTER_NAME ) )
			{
				sPageMasterName = xAttrList->getValueByIndex( i );
			}
		}
	}

	if( 0 == sName.getLength() )
		return;

	Reference < XNameContainer > xPageStyles =
			GetImport().GetTextImport()->GetPageStyles();
	if( !xPageStyles.is() )
		return;

	Any aAny;
	sal_Bool bNew = sal_False;
	if( xPageStyles->hasByName( sName ) )
	{
		aAny = xPageStyles->getByName( sName );
		aAny >>= xStyle;
	}
	else
	{
		xStyle = Create();
		if( !xStyle.is() )
			return;

		aAny <<= xStyle;
		xPageStyles->insertByName( sName, aAny );
		bNew = sal_True;
	}

	Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
	Reference< XPropertySetInfo > xPropSetInfo =
				xPropSet->getPropertySetInfo();
	if( !bNew && xPropSetInfo->hasPropertyByName( sIsPhysical ) )
	{
		aAny = xPropSet->getPropertyValue( sIsPhysical );
		bNew = !*(sal_Bool *)aAny.getValue();
	}
	SetNew( bNew );

	if( bOverwrite || bNew )
	{
		Reference < XMultiPropertyStates > xMultiStates( xPropSet,
														 UNO_QUERY );
		OSL_ENSURE( xMultiStates.is(), 
					"text page style does not support multi property set" );
		if( xMultiStates.is() )
			xMultiStates->setAllPropertiesToDefault();

		bInsertHeader = bInsertFooter = sal_True;
		bInsertHeaderLeft = bInsertFooterLeft = sal_True;
	}
}

XMLTextMasterPageContext::~XMLTextMasterPageContext()
{
}

SvXMLImportContext *XMLTextMasterPageContext::CreateChildContext(
		sal_uInt16 nPrefix,
		const OUString& rLocalName,
		const Reference< XAttributeList > & xAttrList )
{
	SvXMLImportContext *pContext = 0;

	const SvXMLTokenMap& rTokenMap =
		GetImport().GetTextImport()->GetTextMasterPageElemTokenMap();

	sal_Bool bInsert = sal_False, bFooter = sal_False, bLeft = sal_False;
	switch( rTokenMap.Get( nPrefix, rLocalName ) )
	{
	case XML_TOK_TEXT_MP_HEADER:
		if( bInsertHeader && !bHeaderInserted )
		{
			bInsert = sal_True;
			bHeaderInserted = sal_True;
		}
		break;
	case XML_TOK_TEXT_MP_FOOTER:
		if( bInsertFooter && !bFooterInserted )
		{
			bInsert = bFooter = sal_True;
			bFooterInserted = sal_True;
		}
		break;
	case XML_TOK_TEXT_MP_HEADER_LEFT:
		if( bInsertHeaderLeft && bHeaderInserted && !bHeaderLeftInserted )
			bInsert = bLeft = sal_True;
		break;
	case XML_TOK_TEXT_MP_FOOTER_LEFT:
		if( bInsertFooterLeft && bFooterInserted && !bFooterLeftInserted )
			bInsert = bFooter = bLeft = sal_True;
		break;
	}

	if( bInsert && xStyle.is() )
	{
		pContext = CreateHeaderFooterContext( nPrefix, rLocalName,
													xAttrList,
													bFooter, bLeft );
	}
	else
	{
		pContext = SvXMLStyleContext::CreateChildContext( nPrefix, rLocalName,
														  xAttrList );
	}

	return pContext;
}

SvXMLImportContext *XMLTextMasterPageContext::CreateHeaderFooterContext(
			sal_uInt16 nPrefix,
			const ::rtl::OUString& rLocalName,
			const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
			const sal_Bool bFooter,
			const sal_Bool bLeft )
{
	Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
	return new XMLTextHeaderFooterContext( GetImport(),
												nPrefix, rLocalName,
												xAttrList,
												xPropSet,
												bFooter, bLeft );
}

void XMLTextMasterPageContext::Finish( sal_Bool bOverwrite )
{
	if( xStyle.is() && (IsNew() || bOverwrite) )
	{
		Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
		if ( sPageMasterName.getLength() )
		{
			XMLPropStyleContext* pStyle =
				GetImport().GetTextImport()->FindPageMaster( sPageMasterName );
			if (pStyle)
			{
				pStyle->FillPropertySet(xPropSet);
			}
		}

		Reference < XNameContainer > xPageStyles =
			GetImport().GetTextImport()->GetPageStyles();
		if( !xPageStyles.is() )
			return;

		if( !sFollow.getLength() || !xPageStyles->hasByName( sFollow ) )
			sFollow = xStyle->getName();

		Reference< XPropertySetInfo > xPropSetInfo =
			xPropSet->getPropertySetInfo();
		if( xPropSetInfo->hasPropertyByName( sFollowStyle ) )
		{
			Any aAny = xPropSet->getPropertyValue( sFollowStyle );
			OUString sCurrFollow;
			aAny >>= sCurrFollow;
			if( sCurrFollow != sFollow )
			{
				aAny <<= sFollow;
				xPropSet->setPropertyValue( sFollowStyle, aAny );
			}
		}
	}
}
}//end of namespace binfilter
