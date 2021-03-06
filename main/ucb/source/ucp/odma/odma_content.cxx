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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

/**************************************************************************
								TODO
 **************************************************************************

 *************************************************************************/
#include <osl/diagnose.h>
#include "odma_contentprops.hxx"
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#include <com/sun/star/ucb/MissingInputStreamException.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/MissingPropertiesException.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include "odma_content.hxx"
#include "odma_provider.hxx"
#include "odma_resultset.hxx"
#include "odma_inputstream.hxx"
#include <ucbhelper/content.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <rtl/ref.hxx>
#include <osl/file.hxx>

using namespace com::sun::star;
using namespace odma;

//=========================================================================
//=========================================================================
//
// Content Implementation.
//
//=========================================================================
//=========================================================================

Content::Content( const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
                  ContentProvider* pProvider,
                  const uno::Reference< ucb::XContentIdentifier >& Identifier,
				  const ::rtl::Reference<ContentProperties>& _rProps)
	: ContentImplHelper( rxSMgr, pProvider, Identifier )
	,m_aProps(_rProps)
	,m_pProvider(pProvider)
	,m_pContent(NULL)
{
	OSL_ENSURE(m_aProps.is(),"No valid ContentPropeties!");
}

//=========================================================================
// virtual
Content::~Content()
{
	delete m_pContent;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

// virtual
void SAL_CALL Content::acquire() throw()
{
	ContentImplHelper::acquire();
}

//=========================================================================
// virtual
void SAL_CALL Content::release() throw()
{
	ContentImplHelper::release();
}

//=========================================================================
// virtual
uno::Any SAL_CALL Content::queryInterface( const uno::Type & rType )
    throw ( uno::RuntimeException )
{
    uno::Any aRet;

	// @@@ Add support for additional interfaces.
#if 0
  	aRet = cppu::queryInterface( rType,
                                 static_cast< yyy::Xxxxxxxxx * >( this ) );
#endif

 	return aRet.hasValue() ? aRet : ContentImplHelper::queryInterface( rType );
}

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_COMMON_IMPL( Content );

//=========================================================================
// virtual
uno::Sequence< uno::Type > SAL_CALL Content::getTypes()
    throw( uno::RuntimeException )
{
	// @@@ Add own interfaces.

    static cppu::OTypeCollection* pCollection = 0;

	if ( !pCollection )
	{
		osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
	  	if ( !pCollection )
	  	{
            static cppu::OTypeCollection aCollection(
                CPPU_TYPE_REF( lang::XTypeProvider ),
                CPPU_TYPE_REF( lang::XServiceInfo ),
                CPPU_TYPE_REF( lang::XComponent ),
                CPPU_TYPE_REF( ucb::XContent ),
                CPPU_TYPE_REF( ucb::XCommandProcessor ),
                CPPU_TYPE_REF( beans::XPropertiesChangeNotifier ),
                CPPU_TYPE_REF( ucb::XCommandInfoChangeNotifier ),
                CPPU_TYPE_REF( beans::XPropertyContainer ),
                CPPU_TYPE_REF( beans::XPropertySetInfoChangeNotifier ),
                CPPU_TYPE_REF( container::XChild ) );
	  		pCollection = &aCollection;
		}
	}

	return (*pCollection).getTypes();
}

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

// virtual
rtl::OUString SAL_CALL Content::getImplementationName()
    throw( uno::RuntimeException )
{
    // @@@ Adjust implementation name. Keep the prefix "com.sun.star.comp."!
    return rtl::OUString::createFromAscii( "com.sun.star.comp.odma.Content" );
}

//=========================================================================
// virtual
uno::Sequence< rtl::OUString > SAL_CALL Content::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
	// @@@ Adjust macro name.
    uno::Sequence< rtl::OUString > aSNS( 1 );
	aSNS.getArray()[ 0 ]
            = rtl::OUString::createFromAscii( ODMA_CONTENT_SERVICE_NAME );
	return aSNS;
}

//=========================================================================
//
// XContent methods.
//
//=========================================================================

// virtual
rtl::OUString SAL_CALL Content::getContentType()
    throw( uno::RuntimeException )
{
	// @@@ Adjust macro name ( def in odma_provider.hxx ).
    return rtl::OUString::createFromAscii( ODMA_CONTENT_TYPE );
}

//=========================================================================
//
// XCommandProcessor methods.
//
//=========================================================================

// virtual
uno::Any SAL_CALL Content::execute(
        const ucb::Command& aCommand,
        sal_Int32 /*CommandId*/,
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception,
           ucb::CommandAbortedException,
           uno::RuntimeException )
{
    uno::Any aRet;

    if ( aCommand.Name.equalsAsciiL(
			RTL_CONSTASCII_STRINGPARAM( "getPropertyValues" ) ) )
	{
		//////////////////////////////////////////////////////////////////
		// getPropertyValues
		//////////////////////////////////////////////////////////////////

        uno::Sequence< beans::Property > Properties;
		if ( !( aCommand.Argument >>= Properties ) )
		{
            OSL_ENSURE( sal_False, "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
		}

        aRet <<= getPropertyValues( Properties, Environment );
	}
    else if ( aCommand.Name.equalsAsciiL(
				RTL_CONSTASCII_STRINGPARAM( "setPropertyValues" ) ) )
    {
		//////////////////////////////////////////////////////////////////
		// setPropertyValues
		//////////////////////////////////////////////////////////////////

        uno::Sequence< beans::PropertyValue > aProperties;
		if ( !( aCommand.Argument >>= aProperties ) )
		{
            OSL_ENSURE( sal_False, "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

		if ( !aProperties.getLength() )
		{
            OSL_ENSURE( sal_False, "No properties!" );
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet <<= setPropertyValues( aProperties, Environment );
	}
    else if ( aCommand.Name.equalsAsciiL(
				RTL_CONSTASCII_STRINGPARAM( "getPropertySetInfo" ) ) )
    {
		//////////////////////////////////////////////////////////////////
		// getPropertySetInfo
		//////////////////////////////////////////////////////////////////

		// Note: Implemented by base class.
		aRet <<= getPropertySetInfo( Environment );
	}
    else if ( aCommand.Name.equalsAsciiL(
				RTL_CONSTASCII_STRINGPARAM( "getCommandInfo" ) ) )
    {
		//////////////////////////////////////////////////////////////////
		// getCommandInfo
		//////////////////////////////////////////////////////////////////

		// Note: Implemented by base class.
		aRet <<= getCommandInfo( Environment );
	}
    else if ( aCommand.Name.equalsAsciiL(
				RTL_CONSTASCII_STRINGPARAM( "open" ) ) )
    {
        ucb::OpenCommandArgument2 aOpenCommand;
      	if ( !( aCommand.Argument >>= aOpenCommand ) )
		{
            OSL_ENSURE( sal_False, "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        sal_Bool bOpenFolder =
            ( ( aOpenCommand.Mode == ucb::OpenMode::ALL ) ||
              ( aOpenCommand.Mode == ucb::OpenMode::FOLDERS ) ||
              ( aOpenCommand.Mode == ucb::OpenMode::DOCUMENTS ) );

        if ( bOpenFolder)
		{
            // open as folder - return result set

            uno::Reference< ucb::XDynamicResultSet > xSet
                            = new DynamicResultSet( m_xSMgr,
													this,
													aOpenCommand,
													Environment );
    		aRet <<= xSet;
  		}

        if ( aOpenCommand.Sink.is() )
        {
            // Open document - supply document data stream.

            // Check open mode
            if ( ( aOpenCommand.Mode
                    == ucb::OpenMode::DOCUMENT_SHARE_DENY_NONE ) ||
                 ( aOpenCommand.Mode
                    == ucb::OpenMode::DOCUMENT_SHARE_DENY_WRITE ) )
            {
                // Unsupported.
                ucbhelper::cancelCommandExecution(
                    uno::makeAny( ucb::UnsupportedOpenModeException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    sal_Int16( aOpenCommand.Mode ) ) ),
                    Environment );
                // Unreachable
            }


            rtl::OUString aURL = m_xIdentifier->getContentIdentifier();
			rtl::OUString sFileURL = openDoc();
			delete m_pContent;
			m_pContent = new ::ucbhelper::Content(sFileURL,NULL);
			if(!m_pContent->isDocument())
			{
				rtl::OUString sErrorMsg(RTL_CONSTASCII_USTRINGPARAM("File: "));
				sErrorMsg += sFileURL;
				sErrorMsg += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" could not be found."));
				ucbhelper::cancelCommandExecution(
						uno::makeAny( io::IOException(
										sErrorMsg,
										static_cast< cppu::OWeakObject * >( this )) ),
						Environment );
			}

            uno::Reference< io::XOutputStream > xOut
                = uno::Reference< io::XOutputStream >(
                    aOpenCommand.Sink, uno::UNO_QUERY );
    		if ( xOut.is() )
      		{
				// @@@ PUSH: write data into xOut
				m_pContent->openStream(xOut);
      		}
    		else
      		{
                uno::Reference< io::XActiveDataSink > xDataSink
                    = uno::Reference< io::XActiveDataSink >(
                        aOpenCommand.Sink, uno::UNO_QUERY );
      			if ( xDataSink.is() )
				{
	  				// @@@ PULL: wait for client read
					uno::Reference< io::XInputStream > xIn;
					try
					{
						xIn = m_pContent->openStream();
					}
					catch(uno::Exception&)
					{
						OSL_ENSURE(0,"Exception occured while creating the file content!");
					}
    				xDataSink->setInputStream( xIn );
				}
      			else
				{
					uno::Reference< io::XActiveDataStreamer > activeDataStreamer( aOpenCommand.Sink,uno::UNO_QUERY );
					if(activeDataStreamer.is())
					{
						activeDataStreamer->setStream(new OOdmaStream(m_pContent,getContentProvider(),m_aProps));
						m_pContent = NULL; // don't delete here because the stream is now the owner
					}
					else
					{
						// Note: aOpenCommand.Sink may contain an XStream
						//       implementation. Support for this type of
						//       sink is optional...
						ucbhelper::cancelCommandExecution(
							uno::makeAny( ucb::UnsupportedDataSinkException(
									rtl::OUString(),
									static_cast< cppu::OWeakObject * >( this ),
									aOpenCommand.Sink ) ),
							Environment );
						// Unreachable
					}
                }
	  		}
		}
	}
	else if ( aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "close" ) ) )
    {
		getContentProvider()->closeDocument(m_aProps->m_sDocumentId);
	}
	else if ( aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "delete" ) ) )
    {
		//////////////////////////////////////////////////////////////////
		// delete
		//////////////////////////////////////////////////////////////////

		// Remove own and all children's Additional Core Properties.
		removeAdditionalPropertySet( sal_True );
		// Remove own and all childrens(!) persistent data.
		if(!getContentProvider()->deleteDocument(m_aProps))
			ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
	}
    else if ( aCommand.Name.equalsAsciiL(
				RTL_CONSTASCII_STRINGPARAM( "insert" ) ) )
    {
		//////////////////////////////////////////////////////////////////
		// insert
		//////////////////////////////////////////////////////////////////

        ucb::InsertCommandArgument arg;
      	if ( !( aCommand.Argument >>= arg ) )
		{
	  		OSL_ENSURE( sal_False, "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
		}

      	insert( arg.Data, arg.ReplaceExisting, Environment );
    }
	else if( ! aCommand.Name.compareToAscii( "transfer" ) )
	{
		ucb::TransferInfo aTransferInfo;
		if( ! ( aCommand.Argument >>= aTransferInfo ) )
		{
			OSL_ENSURE( sal_False, "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
		}
		::rtl::Reference<ContentProperties> aProp = m_aProps;
		if(aProp->m_bIsFolder)
		{
			aProp = getContentProvider()->getContentPropertyWithTitle(aTransferInfo.NewTitle);
			if(!aProp.is())
				aProp = getContentProvider()->getContentPropertyWithSavedAsName(aTransferInfo.NewTitle);
			sal_Bool bError = !aProp.is();
			if(bError)
			{
				sal_Char* pExtension = NULL;
				::rtl::OString sExt;
				sal_Int32 nPos = aTransferInfo.NewTitle.lastIndexOf('.');
				if(nPos != -1)
				{
					sExt = ::rtl::OUStringToOString(aTransferInfo.NewTitle.copy(nPos+1),RTL_TEXTENCODING_ASCII_US);
					if(sExt.equalsIgnoreAsciiCase("txt"))
						pExtension = ODM_FORMAT_TEXT;
					else if(sExt.equalsIgnoreAsciiCase("rtf"))
						pExtension = ODM_FORMAT_RTF;
					else if(sExt.equalsIgnoreAsciiCase("ps"))
						pExtension = ODM_FORMAT_PS;
					else  
						pExtension = const_cast<sal_Char*>(sExt.getStr());
				}
				else
					pExtension = ODM_FORMAT_TEXT;

				sal_Char* lpszNewDocId = new sal_Char[ODM_DOCID_MAX];
				void *pData = NULL;
				DWORD dwFlags = ODM_SILENT;
				ODMSTATUS odm = NODMSaveAsEx(ContentProvider::getHandle(),
											 NULL, // means it is saved the first time
											 lpszNewDocId,
											 pExtension,
											 NULL, // no callback function here
											 pData,
											 &dwFlags);

				// check if we have to call the DMS dialog
				if(odm == ODM_E_USERINT) 
				{ 
					dwFlags = 0;
					odm = NODMSaveAsEx(ContentProvider::getHandle(),
											 NULL, // means it is saved the first time
											 lpszNewDocId,
											 pExtension,
											 NULL, // no callback function here
											 pData,
											 &dwFlags);
				}
				bError = odm != ODM_SUCCESS;
				if(!bError)
				{
					aProp = new ContentProperties();
					aProp->m_sDocumentId	= ::rtl::OString(lpszNewDocId);
					aProp->m_sContentType	= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ODMA_CONTENT_TYPE));
					aProp->m_sSavedAsName	= aTransferInfo.NewTitle;
					getContentProvider()->append(aProp);

					// now set the title
					WORD nDocInfo = ODM_NAME;
					::rtl::OUString sFileName = aTransferInfo.NewTitle;
					sal_Int32 nIndex = aTransferInfo.NewTitle.lastIndexOf( sal_Unicode('.') );
					if(nIndex != -1)
						sFileName = aTransferInfo.NewTitle.copy(0,nIndex);

					::rtl::OString sDocInfoValue = ::rtl::OUStringToOString(sFileName,RTL_TEXTENCODING_ASCII_US);
					odm = NODMSetDocInfo(	ContentProvider::getHandle(),
											lpszNewDocId,
											nDocInfo,
											const_cast<sal_Char*>(sDocInfoValue.getStr())
											);

				}
				else if ( odm == ODM_E_CANCEL)
                    NODMActivate(ContentProvider::getHandle(),
                                 ODM_DELETE,
                                 lpszNewDocId);

				delete [] lpszNewDocId;
			}
			if(bError)
				ucbhelper::cancelCommandExecution(
						uno::makeAny( lang::IllegalArgumentException(
											rtl::OUString(),
											static_cast< cppu::OWeakObject * >( this ),
											-1 ) ),
						Environment );
		}
		rtl::OUString sFileURL = ContentProvider::openDoc(aProp);

		sal_Int32 nLastIndex = sFileURL.lastIndexOf( sal_Unicode('/') );
		::ucbhelper::Content aContent(sFileURL.copy(0,nLastIndex),NULL);
		//	aTransferInfo.NameClash = ucb::NameClash::OVERWRITE;
		aTransferInfo.NewTitle = sFileURL.copy( 1 + nLastIndex );
		aContent.executeCommand(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("transfer")),uno::makeAny(aTransferInfo));
		getContentProvider()->saveDocument(aProp->m_sDocumentId);
	}
	else
	{
		//////////////////////////////////////////////////////////////////
		// Unsupported command
		//////////////////////////////////////////////////////////////////

        OSL_ENSURE( sal_False, "Content::execute - unsupported command!" );

        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::UnsupportedCommandException(
                            rtl::OUString(),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
        // Unreachable
    }

	return aRet;
}

//=========================================================================
// virtual
void SAL_CALL Content::abort( sal_Int32 /*CommandId*/ )
    throw( uno::RuntimeException )
{
	// @@@ Implement logic to abort running commands, if this makes
	//     sense for your content.
}

//=========================================================================
//
// Non-interface methods.
//
//=========================================================================

// virtual
::rtl::OUString Content::getParentURL()
{
    ::rtl::OUString sURL = m_xIdentifier->getContentIdentifier();

    // @@@ Extract URL of parent from aURL and return it...
	static ::rtl::OUString sScheme1(RTL_CONSTASCII_USTRINGPARAM(ODMA_URL_SCHEME ODMA_URL_SHORT "/"));
	static ::rtl::OUString sScheme2(RTL_CONSTASCII_USTRINGPARAM(ODMA_URL_SCHEME ODMA_URL_SHORT));
	if(sURL == sScheme1 || sURL == sScheme2)
		sURL = ::rtl::OUString();
	else
		sURL = sScheme1;

    return sURL;
}

//=========================================================================
// static
uno::Reference< sdbc::XRow > Content::getPropertyValues(
            const uno::Reference< lang::XMultiServiceFactory >& rSMgr,
            const uno::Sequence< beans::Property >& rProperties,
            const rtl::Reference<ContentProperties>& rData,
            const rtl::Reference< ::ucbhelper::ContentProviderImplHelper >& rProvider,
            const rtl::OUString& rContentId )
{
	// Note: Empty sequence means "get values of all supported properties".

    rtl::Reference< ::ucbhelper::PropertyValueSet > xRow
                                = new ::ucbhelper::PropertyValueSet( rSMgr );

	sal_Int32 nCount = rProperties.getLength();
	if ( nCount )
	{
        uno::Reference< beans::XPropertySet > xAdditionalPropSet;
		sal_Bool bTriedToGetAdditonalPropSet = sal_False;

        const beans::Property* pProps = rProperties.getConstArray();
		for ( sal_Int32 n = 0; n < nCount; ++n )
		{
            const beans::Property& rProp = pProps[ n ];

			// Process Core properties.

            if ( rProp.Name.equalsAsciiL(
					RTL_CONSTASCII_STRINGPARAM( "ContentType" ) ) )
            {
				xRow->appendString ( rProp, rData->m_sContentType );
			}
            else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
			{
				xRow->appendString ( rProp, rData->m_sTitle );
			}
            else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) ) )
			{
				xRow->appendBoolean( rProp, rData->m_bIsDocument );
			}
            else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) ) )
			{
				xRow->appendBoolean( rProp, rData->m_bIsFolder );
			}
            else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "DateCreated" ) ) )
			{
				xRow->appendTimestamp( rProp, rData->m_aDateCreated );
			}
			else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "DateModified" ) ) )
			{
				xRow->appendTimestamp( rProp, rData->m_aDateModified );
			}
			else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "IsReadOnly" ) ) )
			{
				xRow->appendBoolean( rProp, rData->m_bIsReadOnly );
			}
			else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Author" ) ) )
			{
				xRow->appendString ( rProp, rData->m_sAuthor );
			}
			else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Subject" ) ) )
			{
				xRow->appendString ( rProp, rData->m_sSubject );
			}
			else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Keywords" ) ) )
			{
				xRow->appendString ( rProp, rData->m_sKeywords );
			}
			else
			{
				// @@@ Note: If your data source supports adding/removing
				//     properties, you should implement the interface
				//     XPropertyContainer by yourself and supply your own
				//     logic here. The base class uses the service
				//     "com.sun.star.ucb.Store" to maintain Additional Core
				//     properties. But using server functionality is preferred!

				// Not a Core Property! Maybe it's an Additional Core Property?!

				if ( !bTriedToGetAdditonalPropSet && !xAdditionalPropSet.is() )
				{
					xAdditionalPropSet
                        = uno::Reference< beans::XPropertySet >(
							rProvider->getAdditionalPropertySet( rContentId,
																 sal_False ),
                            uno::UNO_QUERY );
					bTriedToGetAdditonalPropSet = sal_True;
				}

				if ( xAdditionalPropSet.is() )
				{
					if ( !xRow->appendPropertySetValue(
												xAdditionalPropSet,
												rProp ) )
					{
						// Append empty entry.
						xRow->appendVoid( rProp );
					}
				}
				else
				{
					// Append empty entry.
					xRow->appendVoid( rProp );
				}
			}
		}
	}
	else
	{
		// Append all Core Properties.
		xRow->appendString (
            beans::Property( rtl::OUString::createFromAscii( "ContentType" ),
					  -1,
                      getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
			rData->m_sContentType );
		xRow->appendString (
            beans::Property( rtl::OUString::createFromAscii( "Title" ),
					  -1,
                      getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                      beans::PropertyAttribute::BOUND ),
			rData->m_sTitle );
		xRow->appendBoolean(
            beans::Property( rtl::OUString::createFromAscii( "IsDocument" ),
					  -1,
					  getCppuBooleanType(),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
			rData->m_bIsDocument );
		xRow->appendBoolean(
            beans::Property( rtl::OUString::createFromAscii( "IsFolder" ),
					  -1,
					  getCppuBooleanType(),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
			rData->m_bIsFolder );

		// @@@ Append other properties supported directly.
		xRow->appendTimestamp(
            beans::Property( rtl::OUString::createFromAscii( "DateCreated" ),
					  -1,
					  getCppuType(static_cast< const util::DateTime * >( 0 ) ),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
			rData->m_aDateCreated );
		xRow->appendTimestamp(
            beans::Property( rtl::OUString::createFromAscii( "DateModified" ),
					  -1,
					  getCppuType(static_cast< const util::DateTime * >( 0 ) ),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
			rData->m_aDateModified );
		xRow->appendBoolean(
            beans::Property( rtl::OUString::createFromAscii( "IsReadOnly" ),
					  -1,
					  getCppuBooleanType(),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
			rData->m_bIsReadOnly );
		xRow->appendString (
            beans::Property( rtl::OUString::createFromAscii( "Author" ),
					  -1,
                      getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                      beans::PropertyAttribute::BOUND ),
			rData->m_sAuthor );
		xRow->appendString (
            beans::Property( rtl::OUString::createFromAscii( "Subject" ),
					  -1,
                      getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                      beans::PropertyAttribute::BOUND ),
			rData->m_sSubject );
		xRow->appendString (
            beans::Property( rtl::OUString::createFromAscii( "Keywords" ),
					  -1,
                      getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                      beans::PropertyAttribute::BOUND ),
			rData->m_sKeywords );

		// @@@ Note: If your data source supports adding/removing
		//     properties, you should implement the interface
		//     XPropertyContainer by yourself and supply your own
		//     logic here. The base class uses the service
		//     "com.sun.star.ucb.Store" to maintain Additional Core
		//     properties. But using server functionality is preferred!

		// Append all Additional Core Properties.

        uno::Reference< beans::XPropertySet > xSet(
			rProvider->getAdditionalPropertySet( rContentId, sal_False ),
            uno::UNO_QUERY );
		xRow->appendPropertySet( xSet );
	}

    return uno::Reference< sdbc::XRow >( xRow.get() );
}

//=========================================================================
uno::Reference< sdbc::XRow > Content::getPropertyValues(
            const uno::Sequence< beans::Property >& rProperties,
            const uno::Reference< ucb::XCommandEnvironment >& /*xEnv*/ )
{
	osl::Guard< osl::Mutex > aGuard( m_aMutex );
	return getPropertyValues( m_xSMgr,
							  rProperties,
							  m_aProps,
                              rtl::Reference<
                                ::ucbhelper::ContentProviderImplHelper >(
                                    m_xProvider.get() ),
							  m_xIdentifier->getContentIdentifier() );
}

//=========================================================================
uno::Sequence< uno::Any > Content::setPropertyValues(
            const uno::Sequence< beans::PropertyValue >& rValues,
            const uno::Reference< ucb::XCommandEnvironment >& /*xEnv*/ )
{
	osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    uno::Sequence< uno::Any > aRet( rValues.getLength() );
    uno::Sequence< beans::PropertyChangeEvent > aChanges( rValues.getLength() );
	sal_Int32 nChanged = 0;

    beans::PropertyChangeEvent aEvent;
    aEvent.Source         = static_cast< cppu::OWeakObject * >( this );
	aEvent.Further 		  = sal_False;
//	aEvent.PropertyName	  =
	aEvent.PropertyHandle = -1;
//	aEvent.OldValue		  =
//	aEvent.NewValue       =

    const beans::PropertyValue* pValues = rValues.getConstArray();
	sal_Int32 nCount = rValues.getLength();

    uno::Reference< ucb::XPersistentPropertySet > xAdditionalPropSet;
	sal_Bool bTriedToGetAdditonalPropSet = sal_False;

	for ( sal_Int32 n = 0; n < nCount; ++n )
	{
        const beans::PropertyValue& rValue = pValues[ n ];

        if ( rValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
		{
			changePropertyValue(rValue,n,m_aProps->m_sTitle,nChanged,aRet,aChanges);
		}
		else if ( rValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "Author") ) )
		{
			changePropertyValue(rValue,n,m_aProps->m_sAuthor,nChanged,aRet,aChanges);
		}
		else if ( rValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "Keywords") ) )
		{
			changePropertyValue(rValue,n,m_aProps->m_sKeywords,nChanged,aRet,aChanges);
		}
		else if ( rValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "Subject") ) )
		{
			changePropertyValue(rValue,n,m_aProps->m_sSubject,nChanged,aRet,aChanges);
		}
		else if (	rValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "ContentType" ) )	||
					rValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) )	||
					rValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) )		||
					rValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "DateCreated" ) )	||
					rValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "DateModified" ) )	||
					rValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "IsReadOnly" ) ) )
        {
			// Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                "Property is read-only!") ),
                            static_cast< cppu::OWeakObject * >( this ) );
		}
		else
		{
			// @@@ Note: If your data source supports adding/removing
			//     properties, you should implement the interface
			//     XPropertyContainer by yourself and supply your own
			//     logic here. The base class uses the service
			//     "com.sun.star.ucb.Store" to maintain Additional Core
			//     properties. But using server functionality is preferred!

			// Not a Core Property! Maybe it's an Additional Core Property?!

			if ( !bTriedToGetAdditonalPropSet && !xAdditionalPropSet.is() )
			{
				xAdditionalPropSet = getAdditionalPropertySet( sal_False );
				bTriedToGetAdditonalPropSet = sal_True;
			}

			if ( xAdditionalPropSet.is() )
			{
				try
				{
                    uno::Any aOldValue
                        = xAdditionalPropSet->getPropertyValue( rValue.Name );
					if ( aOldValue != rValue.Value )
					{
						xAdditionalPropSet->setPropertyValue(
												rValue.Name, rValue.Value );

						aEvent.PropertyName = rValue.Name;
						aEvent.OldValue		= aOldValue;
						aEvent.NewValue     = rValue.Value;

						aChanges.getArray()[ nChanged ] = aEvent;
						nChanged++;
					}
                    else
                    {
                        // Old value equals new value. No error!
                    }
				}
                catch ( beans::UnknownPropertyException const & e )
				{
                    aRet[ n ] <<= e;
				}
                catch ( lang::WrappedTargetException const & e )
				{
                    aRet[ n ] <<= e;
				}
                catch ( beans::PropertyVetoException const & e )
				{
                    aRet[ n ] <<= e;
				}
                catch ( lang::IllegalArgumentException const & e )
				{
                    aRet[ n ] <<= e;
				}
			}
            else
            {
                aRet[ n ] <<= uno::Exception(
                                rtl::OUString::createFromAscii(
                                    "No property set for storing the value!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
		}
	}

	if ( nChanged > 0 )
	{
		// @@@ Save changes.
//		storeData();

		aGuard.clear();
		aChanges.realloc( nChanged );
		notifyPropertiesChange( aChanges );
	}

    return aRet;
}

#if 0
//=========================================================================
void Content::queryChildren( ContentRefList& rChildren )
{
	// @@@ Adapt method to your URL scheme...

	// Obtain a list with a snapshot of all currently instanciated contents
	// from provider and extract the contents which are direct children
	// of this content.

	::ucbhelper::ContentRefList aAllContents;
	m_xProvider->queryExistingContents( aAllContents );

	OUString aURL = m_xIdentifier->getContentIdentifier();
	sal_Int32 nPos = aURL.lastIndexOf( '/' );

	if ( nPos != ( aURL.getLength() - 1 ) )
	{
		// No trailing slash found. Append.
		aURL += OUString::createFromAscii( "/" );
	}

	sal_Int32 nLen = aURL.getLength();

	::ucbhelper::ContentRefList::const_iterator it  = aAllContents.begin();
	::ucbhelper::ContentRefList::const_iterator end = aAllContents.end();

	while ( it != end )
	{
		::ucbhelper::ContentImplHelperRef xChild = (*it);
		OUString aChildURL = xChild->getIdentifier()->getContentIdentifier();

		// Is aURL a prefix of aChildURL?
		if ( ( aChildURL.getLength() > nLen ) &&
			 ( aChildURL.compareTo( aURL, nLen ) == 0 ) )
		{
			sal_Int32 nPos = nLen;
			nPos = aChildURL.indexOf( '/', nPos );

			if ( ( nPos == -1 ) ||
				 ( nPos == ( aChildURL.getLength() - 1 ) ) )
			{
				// No further slashes / only a final slash. It's a child!
				rChildren.push_back(
					ContentRef(
						static_cast< Content * >( xChild.get() ) ) );
			}
		}
		++it;
	}
}
#endif
//=========================================================================
void Content::insert(
        const uno::Reference< io::XInputStream > & xInputStream,
        sal_Bool bReplaceExisting,
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception )
{
	osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

	// Check, if all required properties were set.
	if ( !m_aProps->m_sTitle.getLength())
	{
        OSL_ENSURE( sal_False, "Content::insert - property value missing!" );

        uno::Sequence< rtl::OUString > aProps( 1 );
        aProps[ 0 ] = rtl::OUString::createFromAscii( "zzzz" );
        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::MissingPropertiesException(
                                rtl::OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                aProps ) ),
            Environment );
        // Unreachable
	}

    if ( !xInputStream.is() )
    {
        OSL_ENSURE( sal_False, "Content::insert - No data stream!" );

        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::MissingInputStreamException(
                            rtl::OUString(),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
        // Unreachable
    }

	// Assemble new content identifier...

    //	uno::Reference< ucb::XContentIdentifier > xId = ...;

    // Fail, if a resource with given id already exists.
    if ( !bReplaceExisting ) // && hasData( m_xIdentifier ) )
    {
		ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::UnsupportedCommandException(
                            rtl::OUString(),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
//        ucbhelper::cancelCommandExecution(
//						ucb::IOErrorCode_ALREADY_EXISTING,
//						Environment,
//						uno::makeAny(static_cast< cppu::OWeakObject * >( this ))
//                         );
        // Unreachable
    }

	//	m_xIdentifier = xId;

//  @@@
//	storeData();

	aGuard.clear();
	inserted();
}
#if 0
//=========================================================================
void Content::destroy( sal_Bool bDeletePhysical )
    throw( uno::Exception )
{
	// @@@ take care about bDeletePhysical -> trashcan support

    uno::Reference< ucb::XContent > xThis = this;

	deleted();

	osl::Guard< osl::Mutex > aGuard( m_aMutex );

	// Process instanciated children...

	ContentRefList aChildren;
	queryChildren( aChildren );

	ContentRefList::const_iterator it  = aChildren.begin();
	ContentRefList::const_iterator end = aChildren.end();

	while ( it != end )
	{
		(*it)->destroy( bDeletePhysical );
		++it;
	}
}
#endif

// -----------------------------------------------------------------------------
::rtl::OUString Content::openDoc()
{
	OSL_ENSURE(m_aProps.is(),"No valid content properties!");
	return ContentProvider::openDoc(m_aProps);
}
// -----------------------------------------------------------------------------
void Content::changePropertyValue(const beans::PropertyValue& _rValue,
								  sal_Int32 _rnCurrentPos,
								  ::rtl::OUString& _rsMemberValue,
								  sal_Int32& _rnChanged,
								  uno::Sequence< uno::Any >& _rRet,
								  uno::Sequence< beans::PropertyChangeEvent >& _rChanges) throw (beans::IllegalTypeException)
{
    rtl::OUString sNewValue;
	sal_Bool bError = sal_False;
	if ( _rValue.Value >>= sNewValue )
	{
		if ( sNewValue != _rsMemberValue )
		{
			osl::Guard< osl::Mutex > aGuard( m_aMutex );
			// first we have to check if we could change the property inside the DMS
			::rtl::OString sDocInfoValue = ::rtl::OUStringToOString(sNewValue,RTL_TEXTENCODING_ASCII_US);
			WORD nDocInfo = 0;
			if(&_rsMemberValue == &m_aProps->m_sTitle)
				nDocInfo = ODM_TITLETEXT;
			else if(&_rsMemberValue == &m_aProps->m_sAuthor)
				nDocInfo = ODM_AUTHOR;
			else if(&_rsMemberValue == &m_aProps->m_sSubject)
				nDocInfo = ODM_SUBJECT;
			else if(&_rsMemberValue == &m_aProps->m_sKeywords)
				nDocInfo = ODM_KEYWORDS;
			else
				bError = sal_True;

			if(!bError)
			{
				ODMSTATUS odm = NODMSetDocInfo(	ContentProvider::getHandle(),
												const_cast<sal_Char*>(m_aProps->m_sDocumentId.getStr()),
												nDocInfo,
												const_cast<sal_Char*>(sDocInfoValue.getStr())
												);
				if(odm == ODM_SUCCESS)
				{
					beans::PropertyChangeEvent aEvent;
					aEvent.Source			= static_cast< cppu::OWeakObject * >( this );
					aEvent.Further 			= sal_False;
					aEvent.PropertyHandle	= -1;
					aEvent.PropertyName		= _rValue.Name;
					aEvent.OldValue			= uno::makeAny( _rsMemberValue );
					aEvent.NewValue			= uno::makeAny( sNewValue );

					_rChanges.getArray()[ _rnChanged ] = aEvent;

					_rsMemberValue = sNewValue;
					++_rnChanged;
				}
			}
		}
        else
        {
            // Old value equals new value. No error!
        }
	}
    else
		bError = sal_True;

	if(bError)
    {
        _rRet[ _rnCurrentPos ] <<= beans::IllegalTypeException(
                        rtl::OUString::createFromAscii(
                            "Property value has wrong type!" ),
                        static_cast< cppu::OWeakObject * >( this ) );
    }
}
// -----------------------------------------------------------------------------

