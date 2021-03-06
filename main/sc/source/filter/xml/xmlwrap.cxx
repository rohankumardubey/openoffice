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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <rsc/rscsfx.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <tools/debug.hxx>
#include <vos/xception.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>
#include <svx/xmlgrhlp.hxx>
#include <svtools/sfxecode.hxx>
#include <sfx2/frame.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <sfx2/sfxsids.hrc>
#include <tools/urlobj.hxx>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/xml/sax/XErrorHandler.hpp>
#include <com/sun/star/xml/sax/XEntityResolver.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XDTDHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/extract.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>

#include <svx/xmleohlp.hxx>
#include <rtl/logfile.hxx>
#include <unotools/saveopt.hxx>

#include "document.hxx"
#include "xmlwrap.hxx"
#include "xmlimprt.hxx"
#include "xmlexprt.hxx"
#include "global.hxx"
#include "globstr.hrc"
#include "scerrors.hxx"
#include "XMLExportSharedData.hxx"
#include "docuno.hxx"
#include "sheetdata.hxx"
#include "XMLCodeNameProvider.hxx"

#define MAP_LEN(x) x, sizeof(x) - 1

using namespace com::sun::star;
using ::rtl::OUString;

// -----------------------------------------------------------------------

ScXMLImportWrapper::ScXMLImportWrapper(ScDocument& rD, SfxMedium* pM, const uno::Reference < embed::XStorage >& xStor ) :
	rDoc(rD),
	pMedium(pM),
    xStorage(xStor)
{
    DBG_ASSERT( pMedium || xStorage.is(), "ScXMLImportWrapper: Medium or Storage must be set" );
}

//UNUSED2008-05  uno::Reference <task::XStatusIndicator> ScXMLImportWrapper::GetStatusIndicator(
//UNUSED2008-05          uno::Reference < frame::XModel> & rModel)
//UNUSED2008-05  {
//UNUSED2008-05      DBG_ERROR( "The status indicator from medium must be used!" );
//UNUSED2008-05
//UNUSED2008-05      uno::Reference<task::XStatusIndicator> xStatusIndicator;
//UNUSED2008-05
//UNUSED2008-05      if (rModel.is())
//UNUSED2008-05      {
//UNUSED2008-05          uno::Reference<frame::XController> xController( rModel->getCurrentController());
//UNUSED2008-05          if ( xController.is())
//UNUSED2008-05          {
//UNUSED2008-05              uno::Reference<task::XStatusIndicatorFactory> xFactory( xController->getFrame(), uno::UNO_QUERY );
//UNUSED2008-05              if ( xFactory.is())
//UNUSED2008-05              {
//UNUSED2008-05                  try
//UNUSED2008-05                  {
//UNUSED2008-05                      xStatusIndicator.set(xFactory->createStatusIndicator());
//UNUSED2008-05                  }
//UNUSED2008-05                  catch ( lang::DisposedException e )
//UNUSED2008-05                  {
//UNUSED2008-05                      DBG_ERROR("Exception while trying to get a Status Indicator");
//UNUSED2008-05                  }
//UNUSED2008-05              }
//UNUSED2008-05          }
//UNUSED2008-05      }
//UNUSED2008-05      return xStatusIndicator;
//UNUSED2008-05  }

uno::Reference <task::XStatusIndicator> ScXMLImportWrapper::GetStatusIndicator()
{
	uno::Reference<task::XStatusIndicator> xStatusIndicator;
	if (pMedium)
	{
		SfxItemSet* pSet = pMedium->GetItemSet();
		if (pSet)
		{
			const SfxUnoAnyItem* pItem = static_cast<const SfxUnoAnyItem*>(pSet->GetItem(SID_PROGRESS_STATUSBAR_CONTROL));
			if (pItem)
                xStatusIndicator.set(pItem->GetValue(), uno::UNO_QUERY);
		}
	}
	return xStatusIndicator;
}

sal_uInt32 ScXMLImportWrapper::ImportFromComponent(uno::Reference<lang::XMultiServiceFactory>& xServiceFactory,
	uno::Reference<frame::XModel>& xModel, uno::Reference<uno::XInterface>& xXMLParser,
	xml::sax::InputSource& aParserInput,
	const rtl::OUString& sComponentName, const rtl::OUString& sDocName,
	const rtl::OUString& sOldDocName, uno::Sequence<uno::Any>& aArgs,
	sal_Bool bMustBeSuccessfull)
{
    uno::Reference < io::XStream > xDocStream;
    if ( !xStorage.is() && pMedium )
        xStorage = pMedium->GetStorage();

	// Get data source ...

//	uno::Reference< uno::XInterface > xPipe;
//	uno::Reference< io::XActiveDataSource > xSource;

	sal_Bool bEncrypted = sal_False;
    rtl::OUString sStream(sDocName);
    if( xStorage.is() )
	{
		try
		{
        	uno::Reference < container::XNameAccess > xAccess( xStorage, uno::UNO_QUERY );
        	if ( xAccess->hasByName(sDocName) && xStorage->isStreamElement( sDocName) )
            	xDocStream = xStorage->openStreamElement( sDocName, embed::ElementModes::READ );
        	else if (sOldDocName.getLength() && xAccess->hasByName(sOldDocName) && xStorage->isStreamElement( sOldDocName) )
			{
            	xDocStream = xStorage->openStreamElement( sOldDocName, embed::ElementModes::READ );
            	sStream = sOldDocName;
			}
			else
				return sal_False;

        	aParserInput.aInputStream = xDocStream->getInputStream();
        	uno::Reference < beans::XPropertySet > xSet( xDocStream, uno::UNO_QUERY );

        	uno::Any aAny = xSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Encrypted") ) );
        	aAny >>= bEncrypted;
		}
		catch( packages::WrongPasswordException& )
		{
			return ERRCODE_SFX_WRONGPASSWORD;
		}
		catch( packages::zip::ZipIOException& )
		{
			return ERRCODE_IO_BROKENPACKAGE;
		}
		catch( uno::Exception& )
		{
			return SCERR_IMPORT_UNKNOWN;
		}
	}
    // #99667#; no longer necessary
/*	else if ( pMedium )
	{
		// if there is a medium and if this medium has a load environment,
		// we get an active data source from the medium.
		pMedium->GetInStream()->Seek( 0 );
		xSource = pMedium->GetDataSource();
		DBG_ASSERT( xSource.is(), "got no data source from medium" );
		if( !xSource.is() )
			return sal_False;

		// get a pipe for connecting the data source to the parser
		xPipe = xServiceFactory->createInstance(
				OUString::createFromAscii("com.sun.star.io.Pipe") );
		DBG_ASSERT( xPipe.is(),
				"XMLReader::Read: com.sun.star.io.Pipe service missing" );
		if( !xPipe.is() )
			return sal_False;

		// connect pipe's output stream to the data source
		uno::Reference<io::XOutputStream> xPipeOutput( xPipe, uno::UNO_QUERY );
		xSource->setOutputStream( xPipeOutput );

		aParserInput.aInputStream =
			uno::Reference< io::XInputStream >( xPipe, uno::UNO_QUERY );
	}*/
	else
		return SCERR_IMPORT_UNKNOWN;

	// set Base URL
	uno::Reference< beans::XPropertySet > xInfoSet;
	if( aArgs.getLength() > 0 )
		aArgs.getConstArray()[0] >>= xInfoSet;
	DBG_ASSERT( xInfoSet.is(), "missing property set" );
	if( xInfoSet.is() )
	{
        rtl::OUString sPropName( RTL_CONSTASCII_USTRINGPARAM("StreamName") );
        xInfoSet->setPropertyValue( sPropName, uno::makeAny( sStream ) );
	}

    sal_uInt32 nReturn(0);
    rDoc.SetRangeOverflowType(0);   // is modified by the importer if limits are exceeded

	uno::Reference<xml::sax::XDocumentHandler> xDocHandler(
		xServiceFactory->createInstanceWithArguments(
			sComponentName, aArgs ),
		uno::UNO_QUERY );
	DBG_ASSERT( xDocHandler.is(), "can't get Calc importer" );
	uno::Reference<document::XImporter> xImporter( xDocHandler, uno::UNO_QUERY );
	uno::Reference<lang::XComponent> xComponent( xModel, uno::UNO_QUERY );
	if (xImporter.is())
		xImporter->setTargetDocument( xComponent );

	// connect parser and filter
	uno::Reference<xml::sax::XParser> xParser( xXMLParser, uno::UNO_QUERY );
	xParser->setDocumentHandler( xDocHandler );

	// parse
/*	if( xSource.is() )
	{
		uno::Reference<io::XActiveDataControl> xSourceControl( xSource, uno::UNO_QUERY );
		if( xSourceControl.is() )
			xSourceControl->start();
	}*/

	try
	{
		xParser->parseStream( aParserInput );
	}
	catch( xml::sax::SAXParseException& r )
	{
        // sax parser sends wrapped exceptions,
        // try to find the original one
        xml::sax::SAXException aSaxEx = *(xml::sax::SAXException*)(&r);
        sal_Bool bTryChild = sal_True;

        while( bTryChild )
        {
            xml::sax::SAXException aTmp;
            if ( aSaxEx.WrappedException >>= aTmp )
                aSaxEx = aTmp;
            else
                bTryChild = sal_False;
        }

        packages::zip::ZipIOException aBrokenPackage;
        if ( aSaxEx.WrappedException >>= aBrokenPackage )
            return ERRCODE_IO_BROKENPACKAGE;
		else if( bEncrypted )
			nReturn = ERRCODE_SFX_WRONGPASSWORD;
        else
        {

#ifdef DBG_UTIL
		    ByteString aError( "SAX parse exception catched while importing:\n" );
		    aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
		    DBG_ERROR( aError.GetBuffer() );
#endif

		    String sErr( String::CreateFromInt32( r.LineNumber ));
		    sErr += ',';
		    sErr += String::CreateFromInt32( r.ColumnNumber );

		    if( sDocName.getLength() )
		    {
			    nReturn = *new TwoStringErrorInfo(
							    (bMustBeSuccessfull ? SCERR_IMPORT_FILE_ROWCOL
										   	 	    : SCWARN_IMPORT_FILE_ROWCOL),
					    	    sDocName, sErr,
							    ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
		    }
		    else
		    {
			    DBG_ASSERT( bMustBeSuccessfull, "Warnings are not supported" );
			    nReturn = *new StringErrorInfo( SCERR_IMPORT_FORMAT_ROWCOL, sErr,
							     ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
		    }
        }
	}
	catch( xml::sax::SAXException& r )
	{
        packages::zip::ZipIOException aBrokenPackage;
        if ( r.WrappedException >>= aBrokenPackage )
            return ERRCODE_IO_BROKENPACKAGE;
		else if( bEncrypted )
			nReturn = ERRCODE_SFX_WRONGPASSWORD;
        else
        {

#ifdef DBG_UTIL
		    ByteString aError( "SAX exception catched while importing:\n" );
		    aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
		    DBG_ERROR( aError.GetBuffer() );
#endif
            (void)r;    // avoid warning in product version

    		nReturn = SCERR_IMPORT_FORMAT;
        }
	}
	catch( packages::zip::ZipIOException& r )
	{
#ifdef DBG_UTIL
		ByteString aError( "Zip exception catched while importing:\n" );
		aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
		DBG_ERROR( aError.GetBuffer() );
#endif
        (void)r;    // avoid warning in product version

		nReturn = ERRCODE_IO_BROKENPACKAGE;
	}
	catch( io::IOException& r )
	{
#ifdef DBG_UTIL
		ByteString aError( "IO exception catched while importing:\n" );
		aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
		DBG_ERROR( aError.GetBuffer() );
#endif
        (void)r;    // avoid warning in product version

		nReturn = SCERR_IMPORT_OPEN;
	}
	catch( uno::Exception& r )
	{
#ifdef DBG_UTIL
		ByteString aError( "uno exception catched while importing:\n" );
		aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
		DBG_ERROR( aError.GetBuffer() );
#endif
        (void)r;    // avoid warning in product version

		nReturn = SCERR_IMPORT_UNKNOWN;
	}

    // #i31130# Can't use getImplementation here to get the ScXMLImport from xDocHandler,
    // because when OOo 1.x files are loaded, xDocHandler is the OOo2OasisTransformer.
    // So the overflow warning ErrorCode is now stored in the document.
    // Export works differently, there getImplementation still works.

    if (rDoc.HasRangeOverflow() && !nReturn)
        nReturn = rDoc.GetRangeOverflowType();

	// free the component
	xParser->setDocumentHandler( NULL );

	// success!
	return nReturn;
}

sal_Bool ScXMLImportWrapper::Import(sal_Bool bStylesOnly, ErrCode& nError)
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "sb99857", "ScXMLImportWrapper::Import" );

	uno::Reference<lang::XMultiServiceFactory> xServiceFactory =
										comphelper::getProcessServiceFactory();
	DBG_ASSERT( xServiceFactory.is(), "got no service manager" );
	if( !xServiceFactory.is() )
		return sal_False;

	xml::sax::InputSource aParserInput;
	if (pMedium)
		aParserInput.sSystemId = OUString(pMedium->GetName());

    if ( !xStorage.is() && pMedium )
        xStorage = pMedium->GetStorage();

	// get parser
	uno::Reference<uno::XInterface> xXMLParser(
		xServiceFactory->createInstance(
			OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser" )) ));
	DBG_ASSERT( xXMLParser.is(), "com.sun.star.xml.sax.Parser service missing" );
	if( !xXMLParser.is() )
		return sal_False;

	// get filter
	SfxObjectShell* pObjSh = rDoc.GetDocumentShell();
	if ( pObjSh )
	{
		rtl::OUString sEmpty;
		uno::Reference<frame::XModel> xModel(pObjSh->GetModel());

		/** property map for export info set */
		comphelper::PropertyMapEntry aImportInfoMap[] =
		{
			{ MAP_LEN( "ProgressRange" ), 0, &::getCppuType((sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
			{ MAP_LEN( "ProgressMax" ), 0, &::getCppuType((sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
			{ MAP_LEN( "ProgressCurrent" ), 0, &::getCppuType((sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
			{ MAP_LEN( "NumberStyles" ), 0, &::getCppuType((uno::Reference<container::XNameAccess> *)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "PrivateData" ), 0, &::getCppuType( (uno::Reference<uno::XInterface> *)0 ), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { MAP_LEN( "BaseURI" ), 0, &::getCppuType( (rtl::OUString *)0 ), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { MAP_LEN( "StreamRelPath" ), 0, &::getCppuType( (rtl::OUString *)0 ), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { MAP_LEN( "StreamName" ), 0, &::getCppuType( (rtl::OUString *)0 ), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
			{ MAP_LEN( "BuildId" ), 0, &::getCppuType( (OUString *)0 ), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
			{ MAP_LEN( "VBACompatibilityMode" ), 0, &::getBooleanCppuType(), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
			{ MAP_LEN( "ScriptConfiguration" ), 0, &::getCppuType((uno::Reference<container::XNameAccess> *)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "OrganizerMode" ), 0, &::getBooleanCppuType(),
                ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },

			{ NULL, 0, 0, NULL, 0, 0 }
		};
		uno::Reference< beans::XPropertySet > xInfoSet( comphelper::GenericPropertySet_CreateInstance( new comphelper::PropertySetInfo( aImportInfoMap ) ) );

		// ---- get BuildId from parent container if available

		uno::Reference< container::XChild > xChild( xModel, uno::UNO_QUERY );
		if( xChild.is() )
		{
			uno::Reference< beans::XPropertySet > xParentSet( xChild->getParent(), uno::UNO_QUERY );
			if( xParentSet.is() )
			{
				uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xParentSet->getPropertySetInfo() );
				OUString sPropName( RTL_CONSTASCII_USTRINGPARAM("BuildId" ) );
				if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(sPropName) )
				{
					xInfoSet->setPropertyValue( sPropName, xParentSet->getPropertyValue(sPropName) );
				}
			}
		}

		// -------------------------------------

		uno::Reference<task::XStatusIndicator> xStatusIndicator(GetStatusIndicator());
		if (xStatusIndicator.is())
		{
			sal_Int32 nProgressRange(1000000);
			xStatusIndicator->start(rtl::OUString(ScGlobal::GetRscString(STR_LOAD_DOC)), nProgressRange);
            xInfoSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ProgressRange")), uno::makeAny(nProgressRange));
		}

        // Set base URI
		OSL_ENSURE( pMedium, "There is no medium to get MediaDescriptor from!\n" );
        ::rtl::OUString aBaseURL = pMedium ? pMedium->GetBaseURL() : ::rtl::OUString();
        rtl::OUString sPropName( RTL_CONSTASCII_USTRINGPARAM("BaseURI") );
	    xInfoSet->setPropertyValue( sPropName, uno::makeAny( aBaseURL ) );

		// TODO/LATER: do not do it for embedded links
	    if( SFX_CREATE_MODE_EMBEDDED == pObjSh->GetCreateMode() )
	    {
            OUString aName;
			if ( pMedium && pMedium->GetItemSet() )
			{
				const SfxStringItem* pDocHierarchItem = static_cast<const SfxStringItem*>(
                	pMedium->GetItemSet()->GetItem(SID_DOC_HIERARCHICALNAME) );
				if ( pDocHierarchItem )
					aName = pDocHierarchItem->GetValue();
			}
            else
                aName = ::rtl::OUString::createFromAscii( "dummyObjectName" );

		    if( aName.getLength() )
		    {
                sPropName = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StreamRelPath"));
                xInfoSet->setPropertyValue( sPropName, uno::makeAny( aName ) );
		    }
	    }

        if (bStylesOnly)
        {
            ::rtl::OUString const sOrganizerMode(
                RTL_CONSTASCII_USTRINGPARAM("OrganizerMode"));
            xInfoSet->setPropertyValue(sOrganizerMode, uno::makeAny(sal_True));
        }

    	sal_Bool bOasis = ( SotStorage::GetVersion( xStorage ) > SOFFICE_FILEFORMAT_60 );

        // #i103539#: always read meta.xml for generator
		sal_uInt32 nMetaRetval(0);
        uno::Sequence<uno::Any> aMetaArgs(1);
        uno::Any* pMetaArgs = aMetaArgs.getArray();
        pMetaArgs[0] <<= xInfoSet;

        RTL_LOGFILE_CONTEXT_TRACE( aLog, "meta import start" );

        nMetaRetval = ImportFromComponent(
            xServiceFactory, xModel, xXMLParser, aParserInput,
            bOasis ? rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLOasisMetaImporter"))
                   : rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLMetaImporter")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("meta.xml")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Meta.xml")), aMetaArgs,
            sal_False);

        RTL_LOGFILE_CONTEXT_TRACE( aLog, "meta import end" );

		SvXMLGraphicHelper* pGraphicHelper = NULL;
		uno::Reference< document::XGraphicObjectResolver > xGrfContainer;

		uno::Reference< document::XEmbeddedObjectResolver > xObjectResolver;
		SvXMLEmbeddedObjectHelper *pObjectHelper = NULL;

        if( xStorage.is() )
		{
            pGraphicHelper = SvXMLGraphicHelper::Create( xStorage, GRAPHICHELPER_MODE_READ );
			xGrfContainer = pGraphicHelper;

            if( pObjSh )
			{
                pObjectHelper = SvXMLEmbeddedObjectHelper::Create(xStorage, *pObjSh, EMBEDDEDOBJECTHELPER_MODE_READ, sal_False );
				xObjectResolver = pObjectHelper;
			}
		}
		uno::Sequence<uno::Any> aStylesArgs(4);
		uno::Any* pStylesArgs = aStylesArgs.getArray();
		pStylesArgs[0] <<= xInfoSet;
		pStylesArgs[1] <<= xGrfContainer;
		pStylesArgs[2] <<= xStatusIndicator;
		pStylesArgs[3] <<= xObjectResolver;

		sal_uInt32 nSettingsRetval(0);
		if (!bStylesOnly)
		{
			//	Settings must be loaded first because of the printer setting,
			//	which is needed in the page styles (paper tray).

		    uno::Sequence<uno::Any> aSettingsArgs(1);
		    uno::Any* pSettingsArgs = aSettingsArgs.getArray();
		    pSettingsArgs[0] <<= xInfoSet;

			RTL_LOGFILE_CONTEXT_TRACE( aLog, "settings import start" );

			nSettingsRetval = ImportFromComponent(xServiceFactory, xModel, xXMLParser, aParserInput,
				bOasis ? rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLOasisSettingsImporter"))
                       : rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLSettingsImporter")),
				rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("settings.xml")),
				sEmpty, aSettingsArgs, sal_False);

			RTL_LOGFILE_CONTEXT_TRACE( aLog, "settings import end" );
		}

		sal_uInt32 nStylesRetval(0);
		{
		    RTL_LOGFILE_CONTEXT_TRACE( aLog, "styles import start" );

			nStylesRetval = ImportFromComponent(xServiceFactory, xModel, xXMLParser, aParserInput,
				bOasis ? rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLOasisStylesImporter"))
                       : rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLStylesImporter")),
				rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("styles.xml")),
				sEmpty, aStylesArgs, sal_True);

			RTL_LOGFILE_CONTEXT_TRACE( aLog, "styles import end" );
		}

		sal_uInt32 nDocRetval(0);
		if (!bStylesOnly)
		{
			uno::Sequence<uno::Any> aDocArgs(4);
			uno::Any* pDocArgs = aDocArgs.getArray();
			pDocArgs[0] <<= xInfoSet;
			pDocArgs[1] <<= xGrfContainer;
			pDocArgs[2] <<= xStatusIndicator;
			pDocArgs[3] <<= xObjectResolver;

			RTL_LOGFILE_CONTEXT_TRACE( aLog, "content import start" );

			nDocRetval = ImportFromComponent(xServiceFactory, xModel, xXMLParser, aParserInput,
				bOasis ? rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLOasisContentImporter"))
                       : rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLContentImporter")),
				rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("content.xml")),
				rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Content.xml")), aDocArgs,
				sal_True);

			RTL_LOGFILE_CONTEXT_TRACE( aLog, "content import end" );
		}
		if( pGraphicHelper )
			SvXMLGraphicHelper::Destroy( pGraphicHelper );

		if( pObjectHelper )
			SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );

		if (xStatusIndicator.is())
			xStatusIndicator->end();

		sal_Bool bRet(sal_False);
		if (bStylesOnly)
		{
            if (nStylesRetval)
                nError = nStylesRetval;
            else
				bRet = sal_True;
		}
		else
		{
			if (nDocRetval)
            {
                nError = nDocRetval;
                if (nDocRetval == SCWARN_IMPORT_RANGE_OVERFLOW ||
                    nDocRetval == SCWARN_IMPORT_ROW_OVERFLOW ||
                    nDocRetval == SCWARN_IMPORT_COLUMN_OVERFLOW ||
                    nDocRetval == SCWARN_IMPORT_SHEET_OVERFLOW)
                    bRet = sal_True;
            }
			else if (nStylesRetval)
                nError = nStylesRetval;
			else if (nMetaRetval)
                nError = nMetaRetval;
			else if (nSettingsRetval)
                nError = nSettingsRetval;
			else
				bRet = sal_True;
		}

		// set BuildId on XModel for later OLE object loading
		if( xInfoSet.is() )
		{
			uno::Reference< beans::XPropertySet > xModelSet( xModel, uno::UNO_QUERY );
			if( xModelSet.is() )
			{
				uno::Reference< beans::XPropertySetInfo > xModelSetInfo( xModelSet->getPropertySetInfo() );
				OUString sBuildPropName( RTL_CONSTASCII_USTRINGPARAM("BuildId" ) );
				if( xModelSetInfo.is() && xModelSetInfo->hasPropertyByName(sBuildPropName) )
				{
					xModelSet->setPropertyValue( sBuildPropName, xInfoSet->getPropertyValue(sBuildPropName) );
				}
			}

			// Set Code Names
			uno::Any aAny = xInfoSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("ScriptConfiguration") ));
			uno::Reference <container::XNameAccess> xCodeNameAccess;
			if( aAny >>= xCodeNameAccess )
				XMLCodeNameProvider::set( xCodeNameAccess, &rDoc );

            // VBA compatibility
            bool bVBACompat = false;
            if ( (xInfoSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("VBACompatibilityMode"))) >>= bVBACompat) && bVBACompat )
            {
                /*  Set library container to VBA compatibility mode, this
                    forces loading the Basic project, which in turn creates the
                    VBA Globals object and does all related initialization. */
                if ( xModelSet.is() ) try
                {
                    uno::Reference< script::vba::XVBACompatibility > xVBACompat( xModelSet->getPropertyValue(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "BasicLibraries" ) ) ), uno::UNO_QUERY_THROW );
                    xVBACompat->setVBACompatibilityMode( sal_True );
                }
                catch( uno::Exception& )
                {
                }
            }
		}

		// Don't test bStylesRetval and bMetaRetval, because it could be an older file which not contain such streams
		return bRet;//!bStylesOnly ? bDocRetval : bStylesRetval;
	}
	return sal_False;
}

bool lcl_HasValidStream(ScDocument& rDoc)
{
    SfxObjectShell* pObjSh = rDoc.GetDocumentShell();
    if ( pObjSh->IsDocShared() )
        return false;                       // never copy stream from shared file

    // don't read remote file again
    // (could instead re-use medium directly in that case)
    SfxMedium* pSrcMed = rDoc.GetDocumentShell()->GetMedium();
    if ( !pSrcMed || pSrcMed->IsRemote() )
        return false;

    SCTAB nTabCount = rDoc.GetTableCount();
    for (SCTAB nTab=0; nTab<nTabCount; ++nTab)
        if (rDoc.IsStreamValid(nTab))
            return true;
    return false;
}

sal_Bool ScXMLImportWrapper::ExportToComponent(uno::Reference<lang::XMultiServiceFactory>& xServiceFactory,
	uno::Reference<frame::XModel>& xModel, uno::Reference<uno::XInterface>& xWriter,
	uno::Sequence<beans::PropertyValue>& aDescriptor, const rtl::OUString& sName,
	const rtl::OUString& sMediaType, const rtl::OUString& sComponentName,
	const sal_Bool bPlainText, uno::Sequence<uno::Any>& aArgs, ScMySharedData*& pSharedData)
{
	sal_Bool bRet(sal_False);
	uno::Reference<io::XOutputStream> xOut;
    uno::Reference<io::XStream> xStream;

    if ( !xStorage.is() && pMedium )
        xStorage = pMedium->GetOutputStorage();

    if( xStorage.is() )
	{
		// #96807#; trunc stream before use, because it could be an existing stream
		// and the new content could be shorter than the old content. In this case
		// would not all be over written by the new content and the xml file
		// would not be valid.
        xStream = xStorage->openStreamElement( sName, embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
        uno::Reference < beans::XPropertySet > xSet( xStream, uno::UNO_QUERY );
        if (xSet.is())
        {
		    xSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MediaType")), uno::makeAny(sMediaType));
            OUString aUseCommonPassPropName( RTL_CONSTASCII_USTRINGPARAM("UseCommonStoragePasswordEncryption") );
		    if (bPlainText)
                xSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Compressed")), uno::makeAny(sal_False));

            // even plain stream should be encrypted in encrypted documents
            xSet->setPropertyValue( aUseCommonPassPropName, uno::makeAny(sal_True) );
    	}

        xOut = xStream->getOutputStream();
	}
    // #99667#; no longer necessary
/*	else if ( pMedium )
	{
		xOut = pMedium->GetDataSink();
	}*/

	// set Base URL
	uno::Reference< beans::XPropertySet > xInfoSet;
	if( aArgs.getLength() > 0 )
		aArgs.getConstArray()[0] >>= xInfoSet;
	DBG_ASSERT( xInfoSet.is(), "missing property set" );
	if( xInfoSet.is() )
	{
        rtl::OUString sPropName( RTL_CONSTASCII_USTRINGPARAM("StreamName") );
        xInfoSet->setPropertyValue( sPropName, uno::makeAny( sName ) );
	}

    uno::Reference<io::XActiveDataSource> xSrc( xWriter, uno::UNO_QUERY );
	xSrc->setOutputStream( xOut );

	uno::Reference<document::XFilter> xFilter(
		xServiceFactory->createInstanceWithArguments( sComponentName , aArgs ),
			uno::UNO_QUERY );
	DBG_ASSERT( xFilter.is(), "can't get exporter" );
	uno::Reference<document::XExporter> xExporter( xFilter, uno::UNO_QUERY );
	uno::Reference<lang::XComponent> xComponent( xModel, uno::UNO_QUERY );
	if (xExporter.is())
		xExporter->setSourceDocument( xComponent );

	if ( xFilter.is() )
	{
		ScXMLExport* pExport = static_cast<ScXMLExport*>(SvXMLExport::getImplementation(xFilter));
		pExport->SetSharedData(pSharedData);

        // if there are sheets to copy, get the source stream
        if ( sName.equalsAscii("content.xml") && lcl_HasValidStream(rDoc) &&
             ( pExport->getExportFlags() & EXPORT_OASIS ) )
        {
            // old stream is still in this file's storage - open read-only

            // #i106854# use the document's storage directly, without a temporary SfxMedium
            uno::Reference<embed::XStorage> xTmpStorage = rDoc.GetDocumentShell()->GetStorage();
            uno::Reference<io::XStream> xSrcStream;
            uno::Reference<io::XInputStream> xSrcInput;

            // #i108978# If an embedded object is saved and no events are notified, don't use the stream
            // because without the ...DONE events, stream positions aren't updated.
            ScSheetSaveData* pSheetData = ScModelObj::getImplementation(xModel)->GetSheetSaveData();
            if (pSheetData && pSheetData->IsInSupportedSave())
            {
                try
                {
                    if (xTmpStorage.is())
                        xSrcStream = xTmpStorage->openStreamElement( sName, embed::ElementModes::READ );
                    if (xSrcStream.is())
                        xSrcInput = xSrcStream->getInputStream();
                }
                catch (uno::Exception&)
                {
                    // stream not available (for example, password protected) - save normally (xSrcInput is null)
                }
            }

            pExport->SetSourceStream( xSrcInput );
    		bRet = xFilter->filter( aDescriptor );
            pExport->SetSourceStream( uno::Reference<io::XInputStream>() );

            // If there was an error, reset all stream flags, so the next save attempt will use normal saving.
            // #i110692# For embedded objects, the stream may be unavailable for one save operation (m_pAntiImpl)
            // and become available again later. But after saving normally once, the stream positions aren't
            // valid anymore, so the flags also have to be reset if the stream wasn't available.
            if ( !bRet || !xSrcInput.is() )
            {
                SCTAB nTabCount = rDoc.GetTableCount();
                for (SCTAB nTab=0; nTab<nTabCount; nTab++)
                    if (rDoc.IsStreamValid(nTab))
                        rDoc.SetStreamValid(nTab, sal_False);
            }
        }
        else
    		bRet = xFilter->filter( aDescriptor );

		pSharedData = pExport->GetSharedData();

        //stream is closed by SAX parser
        //if (xOut.is())
        //    xOut->closeOutput();
	}
	return bRet;
}

sal_Bool ScXMLImportWrapper::Export(sal_Bool bStylesOnly)
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "sb99857", "ScXMLImportWrapper::Export" );

	uno::Reference<lang::XMultiServiceFactory> xServiceFactory(comphelper::getProcessServiceFactory());
	DBG_ASSERT( xServiceFactory.is(), "got no service manager" );
	if( !xServiceFactory.is() )
		return sal_False;

	uno::Reference<uno::XInterface> xWriter(xServiceFactory->createInstance(
			OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Writer" )) ));
	DBG_ASSERT( xWriter.is(), "com.sun.star.xml.sax.Writer service missing" );
	if(!xWriter.is())
		return sal_False;

    if ( !xStorage.is() && pMedium )
        xStorage = pMedium->GetOutputStorage();

	uno::Reference<xml::sax::XDocumentHandler> xHandler( xWriter, uno::UNO_QUERY );

	OUString sFileName;
	OUString sTextMediaType(RTL_CONSTASCII_USTRINGPARAM("text/xml"));
	if (pMedium)
		sFileName = pMedium->GetName();
	SfxObjectShell* pObjSh = rDoc.GetDocumentShell();
	uno::Sequence<beans::PropertyValue> aDescriptor(1);
	beans::PropertyValue* pProps = aDescriptor.getArray();
	pProps[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "FileName" ) );
	pProps[0].Value <<= sFileName;

	/** property map for export info set */
	comphelper::PropertyMapEntry aExportInfoMap[] =
	{
		{ MAP_LEN( "ProgressRange" ), 0, &::getCppuType((sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
		{ MAP_LEN( "ProgressMax" ), 0, &::getCppuType((sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
		{ MAP_LEN( "ProgressCurrent" ), 0, &::getCppuType((sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
		{ MAP_LEN( "WrittenNumberStyles" ), 0, &::getCppuType((uno::Sequence<sal_Int32>*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
		{ MAP_LEN( "UsePrettyPrinting" ), 0, &::getCppuType((sal_Bool*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { MAP_LEN( "BaseURI" ), 0, &::getCppuType( (rtl::OUString *)0 ), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
        { MAP_LEN( "StreamRelPath" ), 0, &::getCppuType( (rtl::OUString *)0 ), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
        { MAP_LEN( "StreamName" ), 0, &::getCppuType( (rtl::OUString *)0 ), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
        { MAP_LEN( "StyleNames" ), 0, &::getCppuType( (uno::Sequence<rtl::OUString>*)0 ), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
        { MAP_LEN( "StyleFamilies" ), 0, &::getCppuType( (uno::Sequence<sal_Int32>*)0 ), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
		{ MAP_LEN( "TargetStorage" ), 0, &embed::XStorage::static_type(), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
		{ NULL, 0, 0, NULL, 0, 0 }
	};
	uno::Reference< beans::XPropertySet > xInfoSet( comphelper::GenericPropertySet_CreateInstance( new comphelper::PropertySetInfo( aExportInfoMap ) ) );

    if ( pObjSh && xStorage.is() )
	{
		pObjSh->UpdateDocInfoForSave();		// update information

        uno::Reference<frame::XModel> xModel(pObjSh->GetModel());
		uno::Reference<task::XStatusIndicator> xStatusIndicator(GetStatusIndicator());
		sal_Int32 nProgressRange(1000000);
		if(xStatusIndicator.is())
			xStatusIndicator->start(rtl::OUString(ScGlobal::GetRscString(STR_SAVE_DOC)), nProgressRange);
        xInfoSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ProgressRange")), uno::makeAny(nProgressRange));

		SvtSaveOptions aSaveOpt;
		sal_Bool bUsePrettyPrinting(aSaveOpt.IsPrettyPrinting());
        xInfoSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UsePrettyPrinting")), uno::makeAny(bUsePrettyPrinting));

		const OUString sTargetStorage( RTL_CONSTASCII_USTRINGPARAM("TargetStorage") );
		xInfoSet->setPropertyValue( sTargetStorage, uno::Any( xStorage ) );

		OSL_ENSURE( pMedium, "There is no medium to get MediaDescriptor from!\n" );
        ::rtl::OUString aBaseURL = pMedium ? pMedium->GetBaseURL( true ) : ::rtl::OUString();
        rtl::OUString sPropName( RTL_CONSTASCII_USTRINGPARAM("BaseURI") );
	    xInfoSet->setPropertyValue( sPropName, uno::makeAny( aBaseURL ) );

		// TODO/LATER: do not do it for embedded links
	    if( SFX_CREATE_MODE_EMBEDDED == pObjSh->GetCreateMode() )
	    {
			OUString aName = ::rtl::OUString::createFromAscii( "dummyObjectName" );
			if ( pMedium && pMedium->GetItemSet() )
			{
				const SfxStringItem* pDocHierarchItem = static_cast<const SfxStringItem*>(
                	pMedium->GetItemSet()->GetItem(SID_DOC_HIERARCHICALNAME) );
				if ( pDocHierarchItem )
					aName = pDocHierarchItem->GetValue();
			}

		    if( aName.getLength() )
		    {
                sPropName = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StreamRelPath"));
                xInfoSet->setPropertyValue( sPropName, uno::makeAny( aName ) );
		    }
	    }

		sal_Bool bMetaRet(pObjSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED);
		sal_Bool bStylesRet (sal_False);
		sal_Bool bDocRet(sal_False);
		sal_Bool bSettingsRet(sal_False);
		ScMySharedData* pSharedData = NULL;

        sal_Bool bOasis = ( SotStorage::GetVersion( xStorage ) > SOFFICE_FILEFORMAT_60 );

		// meta export
		if (!bStylesOnly && !bMetaRet)
		{
			uno::Sequence<uno::Any> aMetaArgs(3);
			uno::Any* pMetaArgs = aMetaArgs.getArray();
			pMetaArgs[0] <<= xInfoSet;
			pMetaArgs[1] <<= xHandler;
			pMetaArgs[2] <<= xStatusIndicator;

		    RTL_LOGFILE_CONTEXT_TRACE( aLog, "meta export start" );

			bMetaRet = ExportToComponent(xServiceFactory, xModel, xWriter, aDescriptor,
				rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("meta.xml")),
				sTextMediaType,
                bOasis ? rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLOasisMetaExporter"))
                       : rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLMetaExporter")),
				sal_True, aMetaArgs, pSharedData);

			RTL_LOGFILE_CONTEXT_TRACE( aLog, "meta export end" );
		}

		uno::Reference< document::XEmbeddedObjectResolver > xObjectResolver;
		SvXMLEmbeddedObjectHelper *pObjectHelper = 0;

		uno::Reference< document::XGraphicObjectResolver > xGrfContainer;
		SvXMLGraphicHelper* pGraphicHelper = 0;

        if( xStorage.is() )
		{
            pGraphicHelper = SvXMLGraphicHelper::Create( xStorage, GRAPHICHELPER_MODE_WRITE, sal_False );
			xGrfContainer = pGraphicHelper;
		}

        if( pObjSh )
		{
            pObjectHelper = SvXMLEmbeddedObjectHelper::Create( xStorage, *pObjSh, EMBEDDEDOBJECTHELPER_MODE_WRITE, sal_False );
			xObjectResolver = pObjectHelper;
		}

		// styles export

		{
			uno::Sequence<uno::Any> aStylesArgs(5);
			uno::Any* pStylesArgs = aStylesArgs.getArray();
			pStylesArgs[0] <<= xInfoSet;
			pStylesArgs[1] <<= xGrfContainer;
			pStylesArgs[2] <<= xStatusIndicator;
			pStylesArgs[3] <<= xHandler;
			pStylesArgs[4] <<= xObjectResolver;

		    RTL_LOGFILE_CONTEXT_TRACE( aLog, "styles export start" );

			bStylesRet = ExportToComponent(xServiceFactory, xModel, xWriter, aDescriptor,
				rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("styles.xml")),
				sTextMediaType,
                bOasis ? rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLOasisStylesExporter"))
                       : rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLStylesExporter")),
				sal_False, aStylesArgs, pSharedData);

			RTL_LOGFILE_CONTEXT_TRACE( aLog, "styles export end" );
		}

		// content export

		if (!bStylesOnly)
		{
			uno::Sequence<uno::Any> aDocArgs(5);
			uno::Any* pDocArgs = aDocArgs.getArray();
			pDocArgs[0] <<= xInfoSet;
			pDocArgs[1] <<= xGrfContainer;
			pDocArgs[2] <<= xStatusIndicator;
			pDocArgs[3] <<= xHandler;
			pDocArgs[4] <<= xObjectResolver;

		    RTL_LOGFILE_CONTEXT_TRACE( aLog, "content export start" );

			bDocRet = ExportToComponent(xServiceFactory, xModel, xWriter, aDescriptor,
				rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("content.xml")),
				sTextMediaType,
                bOasis ? rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLOasisContentExporter"))
                       : rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLContentExporter")),
				sal_False, aDocArgs, pSharedData);

		    RTL_LOGFILE_CONTEXT_TRACE( aLog, "content export end" );
		}

		if( pGraphicHelper )
			SvXMLGraphicHelper::Destroy( pGraphicHelper );

		if( pObjectHelper )
			SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );

		// settings export

		if (!bStylesOnly)
		{
			uno::Sequence<uno::Any> aSettingsArgs(3);
			uno::Any* pSettingsArgs = aSettingsArgs.getArray();
			pSettingsArgs[0] <<= xInfoSet;
			pSettingsArgs[1] <<= xHandler;
			pSettingsArgs[2] <<= xStatusIndicator;

		    RTL_LOGFILE_CONTEXT_TRACE( aLog, "settings export start" );

			bSettingsRet = ExportToComponent(xServiceFactory, xModel, xWriter, aDescriptor,
				rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("settings.xml")),
				sTextMediaType,
                bOasis ? rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLOasisSettingsExporter"))
                       : rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLSettingsExporter")),
				sal_False, aSettingsArgs, pSharedData);

			RTL_LOGFILE_CONTEXT_TRACE( aLog, "settings export end" );
		}

		if (pSharedData)
			delete pSharedData;

		if (xStatusIndicator.is())
			xStatusIndicator->end();
		return bStylesRet && ((!bStylesOnly && bDocRet && bMetaRet && bSettingsRet) || bStylesOnly);
	}

	// later: give string descriptor as parameter for doc type

	return sal_False;
}



