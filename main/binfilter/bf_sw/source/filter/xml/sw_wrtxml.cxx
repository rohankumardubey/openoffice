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

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HPP_
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COMPHELPER_GENERICPROPERTYSET_HXX_
#include <comphelper/genericpropertyset.hxx>
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef _XMLGRHLP_HXX
#include <bf_svx/xmlgrhlp.hxx>
#endif
#ifndef _XMLEOHLP_HXX
#include <bf_svx/xmleohlp.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SAVEOPT_HXX
#include <bf_svtools/saveopt.hxx>
#endif

#ifndef _SFXDOCFILE_HXX //autogen wg. SfxMedium
#include <bf_sfx2/docfile.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _PAM_HXX //autogen wg. SwPaM
#include <pam.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#ifndef _DOCSTAT_HXX //autogen wg. SwDoc
#include <docstat.hxx>
#endif
#ifndef _DOCSH_HXX //autogen wg. SwDoc
#include <docsh.hxx>
#endif

#ifndef _ERRHDL_HXX //autogen wg. ASSERT
#include <errhdl.hxx>
#endif
#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _WRTXML_HXX
#include <wrtxml.hxx>
#endif
#ifndef _XMLEXP_HXX
#include <xmlexp.hxx>
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#endif
namespace binfilter {
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

#define LOGFILE_AUTHOR "mb93740"

SwXMLWriter::SwXMLWriter()
{
}


__EXPORT SwXMLWriter::~SwXMLWriter()
{
}


sal_uInt32 SwXMLWriter::_Write()
{
	// Get service factory
	Reference< lang::XMultiServiceFactory > xServiceFactory =
			::legacy_binfilters::getLegacyProcessServiceFactory();
	ASSERT( xServiceFactory.is(),
			"SwXMLWriter::Write: got no service manager" );
	if( !xServiceFactory.is() )
		return ERR_SWG_WRITE_ERROR;

	// Get data sink ...
	Reference< io::XOutputStream > xOut;
	SvStorageStreamRef xDocStream;
	Reference< document::XGraphicObjectResolver > xGraphicResolver;
	SvXMLGraphicHelper *pGraphicHelper = 0;
	Reference< document::XEmbeddedObjectResolver > xObjectResolver;
	SvXMLEmbeddedObjectHelper *pObjectHelper = 0;

	ASSERT( pStg, "Where is my storage?" );
	pGraphicHelper = SvXMLGraphicHelper::Create( *pStg,
												 GRAPHICHELPER_MODE_WRITE,
												 sal_False );
	xGraphicResolver = pGraphicHelper;

	SvPersist *pPersist = pDoc->GetPersist();
	if( pPersist )
	{
		pObjectHelper = SvXMLEmbeddedObjectHelper::Create(
										 *pStg, *pPersist,
										 EMBEDDEDOBJECTHELPER_MODE_WRITE,
										 sal_False );
		xObjectResolver = pObjectHelper;
	}

	// create and prepare the XPropertySet that gets passed through
	// the components, and the XStatusIndicator that shows progress to
	// the user.

	// create XPropertySet with three properties for status indicator
	::comphelper::PropertyMapEntry aInfoMap[] =
	{
		{ "ProgressRange", sizeof("ProgressRange")-1, 0,
			  &::getCppuType((sal_Int32*)0),
			  beans::PropertyAttribute::MAYBEVOID, 0},
		{ "ProgressMax", sizeof("ProgressMax")-1, 0,
			  &::getCppuType((sal_Int32*)0),
			  beans::PropertyAttribute::MAYBEVOID, 0},
		{ "ProgressCurrent", sizeof("ProgressCurrent")-1, 0,
			  &::getCppuType((sal_Int32*)0),
			  beans::PropertyAttribute::MAYBEVOID, 0},
		{ "WrittenNumberStyles", sizeof("WrittenNumberStyles")-1, 0,
			  &::getCppuType((uno::Sequence<sal_Int32> *)0),
			  beans::PropertyAttribute::MAYBEVOID, 0},
		{ "UsePrettyPrinting", sizeof("UsePrettyPrinting")-1, 0,
			  &::getBooleanCppuType(),
			  beans::PropertyAttribute::MAYBEVOID, 0},
		{ "ShowChanges", sizeof("ShowChanges")-1, 0,
			  &::getBooleanCppuType(),
			  beans::PropertyAttribute::MAYBEVOID, 0 },
		{ NULL, 0, 0, NULL, 0, 0 }
	};
	uno::Reference< beans::XPropertySet > xInfoSet(
				::comphelper::GenericPropertySet_CreateInstance(
							new comphelper::PropertySetInfo( aInfoMap ) ) );

	// create XStatusIndicator
	uno::Reference<task::XStatusIndicator> xStatusIndicator;

	uno::Any aAny;
	if (bShowProgress)
	{
		try
		{
			uno::Reference<frame::XModel> xModel( pDoc->GetDocShell()->GetModel());
			if (xModel.is())
			{
				uno::Reference<frame::XController> xController(
					xModel->getCurrentController());
				if( xController.is())
				{
					uno::Reference<frame::XFrame> xFrame( xController->getFrame());
					if( xFrame.is())
					{
						uno::Reference<task::XStatusIndicatorFactory> xFactory(
							xFrame, uno::UNO_QUERY );
						if( xFactory.is())
						{
							xStatusIndicator =
								xFactory->createStatusIndicator();
						}
					}
				}
			}
		}
		catch( const RuntimeException& )
		{
			xStatusIndicator = 0;
		}

		// set progress range and start status indicator
		sal_Int32 nProgressRange(1000000);
		if (xStatusIndicator.is())
		{
			xStatusIndicator->start(SW_RESSTR( STR_STATSTR_SWGWRITE),
									nProgressRange);
		}
		aAny <<= nProgressRange;
		OUString sProgressRange(RTL_CONSTASCII_USTRINGPARAM("ProgressRange"));
		xInfoSet->setPropertyValue(sProgressRange, aAny);

		aAny <<= XML_PROGRESS_REF_NOT_SET;
		OUString sProgressMax(RTL_CONSTASCII_USTRINGPARAM("ProgressMax"));
		xInfoSet->setPropertyValue(sProgressMax, aAny);
	}
	SvtSaveOptions aSaveOpt;
	OUString sUsePrettyPrinting(RTL_CONSTASCII_USTRINGPARAM("UsePrettyPrinting"));
	sal_Bool bUsePrettyPrinting( aSaveOpt.IsPrettyPrinting() );
	aAny.setValue( &bUsePrettyPrinting, ::getBooleanCppuType() );
	xInfoSet->setPropertyValue( sUsePrettyPrinting, aAny );

    // save show redline mode ...
	OUString sShowChanges(RTL_CONSTASCII_USTRINGPARAM("ShowChanges"));
    sal_uInt16 nRedlineMode = pDoc->GetRedlineMode();
    sal_Bool bShowChanges( IsShowChanges( nRedlineMode ) );
    aAny.setValue( &bShowChanges, ::getBooleanCppuType() );
    xInfoSet->setPropertyValue( sShowChanges, aAny );
    // ... and hide redlines for export
    nRedlineMode &= ~REDLINE_SHOW_MASK;
    nRedlineMode |= REDLINE_SHOW_INSERT;
    pDoc->SetRedlineMode( nRedlineMode );


	// filter arguments
	// - graphics + object resolver for styles + content
	// - status indicator
	// - info property set
	// - else empty
	sal_Int32 nArgs = 1;
	if( xStatusIndicator.is() )
		nArgs++;

	Sequence < Any > aEmptyArgs( nArgs );
	Any *pArgs = aEmptyArgs.getArray();
	if( xStatusIndicator.is() )
		*pArgs++ <<= xStatusIndicator;
	*pArgs++ <<= xInfoSet;

	if( xGraphicResolver.is() )
		nArgs++;
	if( xObjectResolver.is() )
		nArgs++;

	Sequence < Any > aFilterArgs( nArgs );
	pArgs = aFilterArgs.getArray();
	if( xGraphicResolver.is() )
		*pArgs++ <<= xGraphicResolver;
	if( xObjectResolver.is() )
		*pArgs++ <<= xObjectResolver;
	if( xStatusIndicator.is() )
		*pArgs++ <<= xStatusIndicator;
	*pArgs++ <<= xInfoSet;

	//Get model
	Reference< lang::XComponent > xModelComp(
		pDoc->GetDocShell()->GetModel(), UNO_QUERY );
	ASSERT( xModelComp.is(), "XMLWriter::Write: got no model" );
	if( !xModelComp.is() )
		return ERR_SWG_WRITE_ERROR;

	PutNumFmtFontsInAttrPool();
	PutEditEngFontsInAttrPool();

	// properties
	Sequence < PropertyValue > aProps( pOrigFileName ? 1 : 0 );
	if( pOrigFileName )
	{
		PropertyValue *pProps = aProps.getArray();
		pProps->Name = OUString( RTL_CONSTASCII_USTRINGPARAM("FileName") );
		(pProps++)->Value <<= OUString( *pOrigFileName  );
	}

	// export sub streams for package, else full stream into a file
	sal_Bool bWarn = sal_False, bErr = sal_False;
	String sWarnFile, sErrFile;

	if( !bOrganizerMode && !bBlock &&
		SFX_CREATE_MODE_EMBEDDED != pDoc->GetDocShell()->GetCreateMode() )
	{
		if( !WriteThroughComponent(
				xModelComp, "meta.xml", xServiceFactory,
				"com.sun.star.comp.Writer.XMLMetaExporter",
				aEmptyArgs, aProps, sal_True ) )
		{
			bWarn = sal_True;
			sWarnFile = String( RTL_CONSTASCII_STRINGPARAM("meta.xml"),
								RTL_TEXTENCODING_ASCII_US );
		}
	}

	if( !WriteThroughComponent(
			xModelComp, "styles.xml", xServiceFactory,
			"com.sun.star.comp.Writer.XMLStylesExporter",
			aFilterArgs, aProps, sal_False ) )
	{
		bErr = sal_True;
		sErrFile = String( RTL_CONSTASCII_STRINGPARAM("styles.xml"),
						   RTL_TEXTENCODING_ASCII_US );
	}

	if( !bErr )
	{
		if( !bBlock )
		{
			if( !WriteThroughComponent(
				xModelComp, "settings.xml", xServiceFactory,
				"com.sun.star.comp.Writer.XMLSettingsExporter",
				aEmptyArgs, aProps, sal_False ) )
			{
				if( !bWarn )
				{
					bWarn = sal_True;
					sWarnFile = String( RTL_CONSTASCII_STRINGPARAM("settings.xml"),
										RTL_TEXTENCODING_ASCII_US );
				}
			}
		}
	}

	if( !bOrganizerMode && !bErr )
	{
		if( !WriteThroughComponent(
				xModelComp, "content.xml", xServiceFactory,
				"com.sun.star.comp.Writer.XMLContentExporter",
				aFilterArgs, aProps, sal_False ) )
		{
			bErr = sal_True;
			sErrFile = String( RTL_CONSTASCII_STRINGPARAM("content.xml"),
							   RTL_TEXTENCODING_ASCII_US );
		}
	}

	if( pDoc->GetRootFrm() && pDoc->GetDocStat().nPage > 1 &&
		!(bOrganizerMode || bBlock || bErr) )
	{
//			DBG_ASSERT( !pDoc->GetDocStat().bModified,
//						"doc stat is modified!" );
		OUString sStreamName( RTL_CONSTASCII_USTRINGPARAM("layout-cache") );
		SvStorageStreamRef xStrm =	pStg->OpenStream( sStreamName,
							   STREAM_WRITE | STREAM_SHARE_DENYWRITE );
		DBG_ASSERT(xStrm.Is(), "Can't create output stream in package!");
		if( xStrm.Is() )
		{
			xStrm->SetSize( 0 );
			String aPropName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("MediaType") ) );
			OUString aMime( RTL_CONSTASCII_USTRINGPARAM("appication/binary") );
			uno::Any aAny;
			aAny <<= aMime;
			xStrm->SetProperty( aPropName, aAny );
			xStrm->SetBufferSize( 16*1024 );
			pDoc->WriteLayoutCache( *xStrm );
			xStrm->Commit();
		}
	}

	if( pGraphicHelper )
		SvXMLGraphicHelper::Destroy( pGraphicHelper );
	xGraphicResolver = 0;

	if( pObjectHelper )
		SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );
	xObjectResolver = 0;

    // restore redline mode
    aAny = xInfoSet->getPropertyValue( sShowChanges );
    nRedlineMode = pDoc->GetRedlineMode();
    nRedlineMode &= ~REDLINE_SHOW_MASK;
    nRedlineMode |= REDLINE_SHOW_INSERT;
    if ( *(sal_Bool*)aAny.getValue() )
        nRedlineMode |= REDLINE_SHOW_DELETE;
    pDoc->SetRedlineMode( nRedlineMode );

	if (xStatusIndicator.is())
	{
		xStatusIndicator->end();
	}

	if( bErr )
	{
		if( sErrFile.Len() )
			return *new StringErrorInfo( ERR_WRITE_ERROR_FILE, sErrFile,
										 ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
		else
			return ERR_SWG_WRITE_ERROR;
	}
	else if( bWarn )
	{
		if( sWarnFile.Len() )
			return *new StringErrorInfo( WARN_WRITE_ERROR_FILE, sWarnFile,
										 ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
		else
			return WARN_SWG_FEATURES_LOST;
	}

	return 0;
}

ULONG SwXMLWriter::WriteStorage()
{
	return _Write();
}

ULONG SwXMLWriter::Write( SwPaM& rPaM, SfxMedium& rMed,
							   const String* pFileName )
{
	return IsStgWriter()
			? ((StgWriter *)this)->Write( rPaM, *rMed.GetOutputStorage( sal_True ), pFileName )
			: ((Writer *)this)->Write( rPaM, *rMed.GetOutStream(), pFileName );
}

sal_Bool SwXMLWriter::WriteThroughComponent(
	const Reference<XComponent> & xComponent,
	const sal_Char* pStreamName,
	const Reference<lang::XMultiServiceFactory> & rFactory,
	const sal_Char* pServiceName,
	const Sequence<Any> & rArguments,
	const Sequence<beans::PropertyValue> & rMediaDesc,
	sal_Bool bPlainStream )
{
	DBG_ASSERT( NULL != pStg, "Need storage!" );
	DBG_ASSERT( NULL != pStreamName, "Need stream name!" );
	DBG_ASSERT( NULL != pServiceName, "Need service name!" );

    RTL_LOGFILE_TRACE_AUTHOR1( "sw", LOGFILE_AUTHOR,
                               "SwXMLWriter::WriteThroughComponent : stream %s",
                               pStreamName );

	Reference< io::XOutputStream > xOutputStream;
	SvStorageStreamRef xDocStream;

	// open stream
	OUString sStreamName = OUString::createFromAscii( pStreamName );
	xDocStream = pStg->OpenStream( sStreamName,
								   STREAM_WRITE | STREAM_SHARE_DENYWRITE );
	DBG_ASSERT(xDocStream.Is(), "Can't create output stream in package!");
	if (! xDocStream.Is())
		return sal_False;

	xDocStream->SetSize( 0 );

	String aPropName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("MediaType") ) );
	OUString aMime( RTL_CONSTASCII_USTRINGPARAM("text/xml") );
	uno::Any aAny;
	aAny <<= aMime;
	xDocStream->SetProperty( aPropName, aAny );

	if( bPlainStream )
	{
		OUString aPropName( RTL_CONSTASCII_USTRINGPARAM("Compressed") );
		sal_Bool bFalse = sal_False;
		aAny.setValue( &bFalse, ::getBooleanCppuType() );
		xDocStream->SetProperty( aPropName, aAny );
	}
	else
	{
		OUString aPropName( RTL_CONSTASCII_USTRINGPARAM("Encrypted") );
		sal_Bool bTrue = sal_True;
		aAny.setValue( &bTrue, ::getBooleanCppuType() );
		xDocStream->SetProperty( aPropName, aAny );
	}


	// set buffer and create outputstream
	xDocStream->SetBufferSize( 16*1024 );
	xOutputStream = new ::utl::OOutputStreamWrapper( *xDocStream );

	// write the stuff
	sal_Bool bRet = WriteThroughComponent(
		xOutputStream, xComponent, rFactory,
		pServiceName, rArguments, rMediaDesc );

	// finally, commit stream.
	if( bRet )
		xDocStream->Commit();

	return bRet;

}

sal_Bool SwXMLWriter::WriteThroughComponent(
	const Reference<io::XOutputStream> & xOutputStream,
	const Reference<XComponent> & xComponent,
	const Reference<XMultiServiceFactory> & rFactory,
	const sal_Char* pServiceName,
	const Sequence<Any> & rArguments,
	const Sequence<PropertyValue> & rMediaDesc )
{
	ASSERT( xOutputStream.is(), "I really need an output stream!" );
	ASSERT( xComponent.is(), "Need component!" );
	ASSERT( NULL != pServiceName, "Need component name!" );

    RTL_LOGFILE_CONTEXT_AUTHOR( aFilterLog, "sw", LOGFILE_AUTHOR,
                                "SwXMLWriter::WriteThroughComponent" );

	// get component
	Reference< io::XActiveDataSource > xSaxWriter(
		rFactory->createInstance(
			String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(
				"com.sun.star.xml.sax.Writer")) ),
		UNO_QUERY );
	ASSERT( xSaxWriter.is(), "can't instantiate XML writer" );
	if(!xSaxWriter.is())
		return sal_False;

    RTL_LOGFILE_CONTEXT_TRACE( aFilterLog, "SAX-Writer created" );

	// connect XML writer to output stream
	xSaxWriter->setOutputStream( xOutputStream );

	// prepare arguments (prepend doc handler to given arguments)
	Reference<xml::sax::XDocumentHandler> xDocHandler( xSaxWriter,UNO_QUERY);
	Sequence<Any> aArgs( 1 + rArguments.getLength() );
	aArgs[0] <<= xDocHandler;
	for(sal_Int32 i = 0; i < rArguments.getLength(); i++)
		aArgs[i+1] = rArguments[i];

	// get filter component
	Reference< document::XExporter > xExporter(
		rFactory->createInstanceWithArguments(
			OUString::createFromAscii(pServiceName), aArgs), UNO_QUERY);
	ASSERT( xExporter.is(),
			"can't instantiate export filter component" );
	if( !xExporter.is() )
		return sal_False;
    RTL_LOGFILE_CONTEXT_TRACE1( aFilterLog, "%s instantiated.", pServiceName );

	// set block mode (if appropriate)
	if( bBlock )
	{
		Reference<XUnoTunnel> xFilterTunnel( xExporter, UNO_QUERY );
		if (xFilterTunnel.is())
		{
			SwXMLExport *pFilter = (SwXMLExport *)xFilterTunnel->getSomething(
											SwXMLExport::getUnoTunnelId() );
			if (NULL != pFilter)
				pFilter->setBlockMode();
		}
	}


	// connect model and filter
	xExporter->setSourceDocument( xComponent );

	// filter!
    RTL_LOGFILE_CONTEXT_TRACE( aFilterLog, "call filter()" );
	Reference<XFilter> xFilter( xExporter, UNO_QUERY );
	return xFilter->filter( rMediaDesc );
}


// -----------------------------------------------------------------------

void GetXMLWriter( const String& rName, WriterRef& xRet )
{
	xRet = new SwXMLWriter();
}

// -----------------------------------------------------------------------
}
