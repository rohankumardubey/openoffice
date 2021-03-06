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



#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#include "smdll.hxx"
#include "document.hxx"

#ifndef _CPPUHELPER_FACTORY_HXX_ 
#include <cppuhelper/factory.hxx>
#endif

namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

//Math document
extern Sequence< OUString > SAL_CALL    
        SmDocument_getSupportedServiceNames() throw();
extern OUString SAL_CALL 
        SmDocument_getImplementationName() throw();
extern Reference< XInterface >SAL_CALL 
        SmDocument_createInstance(const Reference< XMultiServiceFactory > & rSMgr) throw( Exception );

//MathML import
extern Sequence< OUString > SAL_CALL
        SmXMLImport_getSupportedServiceNames() throw();
extern OUString SAL_CALL 
        SmXMLImport_getImplementationName() throw();
extern Reference< XInterface > SAL_CALL 
        SmXMLImport_createInstance(const Reference< XMultiServiceFactory > & rSMgr) throw( Exception );
extern Sequence< OUString > SAL_CALL
        SmXMLImportMeta_getSupportedServiceNames() throw();
extern OUString SAL_CALL 
        SmXMLImportMeta_getImplementationName() throw();
extern Reference< XInterface > SAL_CALL 
        SmXMLImportMeta_createInstance(const Reference< XMultiServiceFactory > & rSMgr) throw( Exception );
extern Sequence< OUString > SAL_CALL
        SmXMLImportSettings_getSupportedServiceNames() throw();
extern OUString SAL_CALL SmXMLImportSettings_getImplementationName() throw();
extern Reference< XInterface > SAL_CALL 
        SmXMLImportSettings_createInstance(const Reference< XMultiServiceFactory > & rSMgr) throw( Exception );

//MathML export
extern Sequence< OUString > SAL_CALL
        SmXMLExport_getSupportedServiceNames() throw();
extern OUString SAL_CALL 
        SmXMLExport_getImplementationName() throw();
extern Reference< XInterface > SAL_CALL 
        SmXMLExport_createInstance(const Reference< XMultiServiceFactory > & rSMgr) throw( Exception );
extern Sequence< OUString > SAL_CALL
        SmXMLExportMeta_getSupportedServiceNames() throw();
extern OUString SAL_CALL 
        SmXMLExportMeta_getImplementationName() throw();
extern Reference< XInterface > SAL_CALL 
        SmXMLExportMeta_createInstance(const Reference< XMultiServiceFactory > & rSMgr) throw( Exception );
extern Sequence< OUString > SAL_CALL
        SmXMLExportSettings_getSupportedServiceNames() throw();
extern OUString SAL_CALL 
        SmXMLExportSettings_getImplementationName() throw();
extern Reference< XInterface > SAL_CALL 
        SmXMLExportSettings_createInstance(const Reference< XMultiServiceFactory > & rSMgr) throw( Exception );
extern Sequence< OUString > SAL_CALL
        SmXMLExportContent_getSupportedServiceNames() throw();
extern OUString SAL_CALL 
        SmXMLExportContent_getImplementationName() throw();
extern Reference< XInterface > SAL_CALL 
        SmXMLExportContent_createInstance(const Reference< XMultiServiceFactory > & rSMgr) throw( Exception );


extern "C" {

void SAL_CALL component_getImplementationEnvironment(
        const  sal_Char**   ppEnvironmentTypeName,
        uno_Environment**   ppEnvironment           )
{
	*ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

void* SAL_CALL component_getFactory( const sal_Char* pImplementationName,
                                     void* pServiceManager,
                                     void* pRegistryKey )
{
	// Set default return value for this operation - if it failed.
	void* pReturn = NULL ;

	if	(
			( pImplementationName	!=	NULL ) &&
			( pServiceManager		!=	NULL )
		)
	{
		// Define variables which are used in following macros.
        Reference< XSingleServiceFactory >   xFactory                                                                                                ;
        Reference< XMultiServiceFactory >    xServiceManager( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;

		if( SmXMLImport_getImplementationName().equalsAsciiL(
			pImplementationName, strlen(pImplementationName)) )
		{
			xFactory = ::cppu::createSingleFactory( xServiceManager,
			SmXMLImport_getImplementationName(),
			SmXMLImport_createInstance,
			SmXMLImport_getSupportedServiceNames() );
		}
		else if( SmXMLExport_getImplementationName().equalsAsciiL(
			pImplementationName, strlen(pImplementationName)) )
		{
			xFactory = ::cppu::createSingleFactory( xServiceManager,
			SmXMLExport_getImplementationName(),
			SmXMLExport_createInstance,
			SmXMLExport_getSupportedServiceNames() );
		}
		else if( SmXMLImportMeta_getImplementationName().equalsAsciiL(
			pImplementationName, strlen(pImplementationName)) )
		{
			xFactory = ::cppu::createSingleFactory( xServiceManager,
			SmXMLImportMeta_getImplementationName(),
			SmXMLImportMeta_createInstance,
			SmXMLImportMeta_getSupportedServiceNames() );
		}
		else if( SmXMLExportMeta_getImplementationName().equalsAsciiL(
			pImplementationName, strlen(pImplementationName)) )
		{
			xFactory = ::cppu::createSingleFactory( xServiceManager,
			SmXMLExportMeta_getImplementationName(),
			SmXMLExportMeta_createInstance,
			SmXMLExportMeta_getSupportedServiceNames() );
		}
		else if( SmXMLImportSettings_getImplementationName().equalsAsciiL( 
			pImplementationName, strlen(pImplementationName)) )
		{
			xFactory = ::cppu::createSingleFactory( xServiceManager,
			SmXMLImportSettings_getImplementationName(),
			SmXMLImportSettings_createInstance, 
			SmXMLImportSettings_getSupportedServiceNames() );
		}
		else if( SmXMLExportSettings_getImplementationName().equalsAsciiL( 
			pImplementationName, strlen(pImplementationName)) )
		{
			xFactory = ::cppu::createSingleFactory( xServiceManager,
			SmXMLExportSettings_getImplementationName(),
			SmXMLExportSettings_createInstance, 
			SmXMLExportSettings_getSupportedServiceNames() );
		}
	    else if( SmXMLExportContent_getImplementationName().equalsAsciiL( 
			pImplementationName, strlen(pImplementationName)) )
		{
			xFactory = ::cppu::createSingleFactory( xServiceManager,
			SmXMLExportContent_getImplementationName(),
			SmXMLExportContent_createInstance, 
			SmXMLExportContent_getSupportedServiceNames() );
		}
	    else if( SmDocument_getImplementationName().equalsAsciiL( 
			pImplementationName, strlen(pImplementationName)) )
		{
			xFactory = ::cppu::createSingleFactory( xServiceManager,
			SmDocument_getImplementationName(),
			SmDocument_createInstance, 
			SmDocument_getSupportedServiceNames() );
		}


		// Factory is valid - service was found.
		if ( xFactory.is() )
		{
			xFactory->acquire();
			pReturn = xFactory.get();
		}
	}

	// Return with result of this operation.
	return pReturn ;
}
} // extern "C"



}
