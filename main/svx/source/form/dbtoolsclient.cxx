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
#include "precompiled_svx.hxx"
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include "svx/dbtoolsclient.hxx"
#include <osl/diagnose.h>
#include <connectivity/formattedcolumnvalue.hxx>

//........................................................................
namespace svxform
{
//........................................................................

	using namespace ::connectivity::simple;
	using namespace ::com::sun::star::sdbc;
	using namespace ::com::sun::star::lang;
	using namespace ::com::sun::star::util;
	using namespace ::com::sun::star::uno;
	using namespace ::com::sun::star::beans;
	using namespace ::com::sun::star::sdb;
	using namespace ::com::sun::star::container;

	//====================================================================
	//= ODbtoolsClient
	//====================================================================
	::osl::Mutex	ODbtoolsClient::s_aMutex;
	sal_Int32		ODbtoolsClient::s_nClients = 0;
	oslModule		ODbtoolsClient::s_hDbtoolsModule = NULL;
	createDataAccessToolsFactoryFunction
					ODbtoolsClient::s_pFactoryCreationFunc = NULL;

	//--------------------------------------------------------------------
	ODbtoolsClient::ODbtoolsClient()
	{
		m_bCreateAlready = sal_False;
	}

	//--------------------------------------------------------------------
	bool ODbtoolsClient::ensureLoaded() const
	{
		if ( !m_bCreateAlready )
        {
		    m_bCreateAlready = true;

            registerClient();
		    if ( s_pFactoryCreationFunc )
		    {	// loading the lib succeeded
			    void* pUntypedFactory = (*s_pFactoryCreationFunc)();
			    IDataAccessToolsFactory* pDBTFactory = static_cast< IDataAccessToolsFactory* >( pUntypedFactory );
			    OSL_ENSURE( pDBTFactory, "ODbtoolsClient::ODbtoolsClient: no factory returned!" );
			    if ( pDBTFactory )
			    {
				    m_xDataAccessFactory = pDBTFactory;
				    // by definition, the factory was aquired once
				    m_xDataAccessFactory->release();
			    }
		    }
        }
		return m_xDataAccessFactory.is();
	}

	//--------------------------------------------------------------------
	ODbtoolsClient::~ODbtoolsClient()
	{
		// clear the factory _before_ revoking the client
		// (the revocation may unload the DBT lib)
		m_xDataAccessFactory = NULL;
		// revoke the client
		if ( m_bCreateAlready )
			revokeClient();
	}

	//--------------------------------------------------------------------
    extern "C" { static void SAL_CALL thisModule() {} }

	void ODbtoolsClient::registerClient() 
	{
		::osl::MutexGuard aGuard(s_aMutex);
		if (1 == ++s_nClients)
		{
			OSL_ENSURE(NULL == s_hDbtoolsModule, "ODbtoolsClient::registerClient: inconsistence: already have a module!");
			OSL_ENSURE(NULL == s_pFactoryCreationFunc, "ODbtoolsClient::registerClient: inconsistence: already have a factory function!");

			const ::rtl::OUString sModuleName = ::rtl::OUString::createFromAscii(
				SVLIBRARY( "dbtools" )
			);

			// load the dbtools library
			s_hDbtoolsModule = osl_loadModuleRelative(
                &thisModule, sModuleName.pData, 0);
			OSL_ENSURE(NULL != s_hDbtoolsModule, "ODbtoolsClient::registerClient: could not load the dbtools library!");
			if (NULL != s_hDbtoolsModule)
			{
				// get the symbol for the method creating the factory
				const ::rtl::OUString sFactoryCreationFunc = ::rtl::OUString::createFromAscii("createDataAccessToolsFactory");
				//  reinterpret_cast<createDataAccessToolsFactoryFunction>
				s_pFactoryCreationFunc = (createDataAccessToolsFactoryFunction)(
					osl_getFunctionSymbol(s_hDbtoolsModule, sFactoryCreationFunc.pData));

				if (NULL == s_pFactoryCreationFunc)
				{	// did not find the symbol
					OSL_ENSURE(sal_False, "ODbtoolsClient::registerClient: could not find the symbol for creating the factory!");
					osl_unloadModule(s_hDbtoolsModule);
					s_hDbtoolsModule = NULL;
				}
			}
		}
	}

	//--------------------------------------------------------------------
	void ODbtoolsClient::revokeClient()
	{
		::osl::MutexGuard aGuard(s_aMutex);
		if (0 == --s_nClients)
		{
			s_pFactoryCreationFunc = NULL;
			if (s_hDbtoolsModule)
				osl_unloadModule(s_hDbtoolsModule);
			s_hDbtoolsModule = NULL;
		}

		OSL_ENSURE(s_nClients >= 0,"Illegall call of revokeClient()");
	}

	//====================================================================
	//= OStaticDataAccessTools
	//====================================================================
	//--------------------------------------------------------------------
	OStaticDataAccessTools::OStaticDataAccessTools()
	{
	}

	//--------------------------------------------------------------------
	//add by BerryJia for fixing Bug97420 Time:2002-9-12-11:00(PRC time)
	bool OStaticDataAccessTools::ensureLoaded() const
	{
        if ( !ODbtoolsClient::ensureLoaded() )
            return false;
	 	m_xDataAccessTools = getFactory()->getDataAccessTools();
        return m_xDataAccessTools.is();
	}

	//--------------------------------------------------------------------
	Reference< XNumberFormatsSupplier > OStaticDataAccessTools::getNumberFormats(const Reference< XConnection>& _rxConn, sal_Bool _bAllowDefault) const
	{
		Reference< XNumberFormatsSupplier > xReturn;
		if ( ensureLoaded() )
			xReturn = m_xDataAccessTools->getNumberFormats(_rxConn, _bAllowDefault);
		return xReturn;
	}

	//--------------------------------------------------------------------
	sal_Int32 OStaticDataAccessTools::getDefaultNumberFormat( const Reference< XPropertySet >& _xColumn, const Reference< XNumberFormatTypes >& _xTypes, const Locale& _rLocale )
    {
        sal_Int32 nReturn = 0;
		if ( ensureLoaded() )
			nReturn = m_xDataAccessTools->getDefaultNumberFormat( _xColumn, _xTypes, _rLocale );
		return nReturn;
    }

	//--------------------------------------------------------------------
	Reference< XConnection> OStaticDataAccessTools::getConnection_withFeedback(const ::rtl::OUString& _rDataSourceName,
		const ::rtl::OUString& _rUser, const ::rtl::OUString& _rPwd, const Reference< XMultiServiceFactory>& _rxFactory) const
			SAL_THROW ( (SQLException) )
	{
		Reference< XConnection > xReturn;
		if ( ensureLoaded() )
			xReturn = m_xDataAccessTools->getConnection_withFeedback(_rDataSourceName, _rUser, _rPwd, _rxFactory);
		return xReturn;
	}

	//--------------------------------------------------------------------
	Reference< XConnection > OStaticDataAccessTools::connectRowset( const Reference< XRowSet >& _rxRowSet,
        const Reference< XMultiServiceFactory >& _rxFactory, sal_Bool _bSetAsActiveConnection ) const
        SAL_THROW ( ( SQLException, WrappedTargetException, RuntimeException ) )
	{
		Reference< XConnection > xReturn;
		if ( ensureLoaded() )
			xReturn = m_xDataAccessTools->connectRowset( _rxRowSet, _rxFactory, _bSetAsActiveConnection );
		return xReturn;
	}

	//--------------------------------------------------------------------
	Reference< XConnection > OStaticDataAccessTools::getRowSetConnection(const Reference< XRowSet >& _rxRowSet) const SAL_THROW ( (RuntimeException) )
	{
		Reference< XConnection > xReturn;
		if ( ensureLoaded() )
			xReturn = m_xDataAccessTools->getRowSetConnection(_rxRowSet);
		return xReturn;
	}

	//--------------------------------------------------------------------
	void OStaticDataAccessTools::TransferFormComponentProperties(const Reference< XPropertySet>& _rxOld,
		const Reference< XPropertySet>& _rxNew, const Locale& _rLocale) const
	{
		if ( ensureLoaded() )
			m_xDataAccessTools->TransferFormComponentProperties(_rxOld, _rxNew, _rLocale);
	}

	//--------------------------------------------------------------------
	::rtl::OUString OStaticDataAccessTools::quoteName(const ::rtl::OUString& _rQuote, const ::rtl::OUString& _rName) const
	{
		::rtl::OUString sReturn;
		if ( ensureLoaded() )
			sReturn = m_xDataAccessTools->quoteName(_rQuote, _rName);
		return sReturn;
	}

	// ------------------------------------------------
	::rtl::OUString OStaticDataAccessTools::composeTableNameForSelect( const Reference< XConnection >& _rxConnection, const Reference< XPropertySet>& _xTable ) const
    {
		::rtl::OUString sReturn;
		if ( ensureLoaded() )
			sReturn = m_xDataAccessTools->composeTableNameForSelect( _rxConnection, _xTable );
        return sReturn;
    }

	//--------------------------------------------------------------------
	SQLContext OStaticDataAccessTools::prependContextInfo(SQLException& _rException, const Reference< XInterface >& _rxContext,
		const ::rtl::OUString& _rContextDescription, const ::rtl::OUString& _rContextDetails) const
	{
		SQLContext aReturn;
		if ( ensureLoaded() )
			aReturn = m_xDataAccessTools->prependContextInfo(_rException, _rxContext, _rContextDescription, _rContextDetails);
		return aReturn;
	}

	//----------------------------------------------------------------
	Reference< XDataSource > OStaticDataAccessTools::getDataSource( const ::rtl::OUString& _rsRegisteredName, const Reference< XMultiServiceFactory>& _rxFactory ) const
	{
		Reference< XDataSource > xReturn;
		if ( ensureLoaded() )
			xReturn = m_xDataAccessTools->getDataSource(_rsRegisteredName,_rxFactory);
		return xReturn;
	}

	//----------------------------------------------------------------
	sal_Bool OStaticDataAccessTools::canInsert(const Reference< XPropertySet>& _rxCursorSet) const
	{
		sal_Bool bRet = sal_False;
		if ( ensureLoaded() )
			bRet = m_xDataAccessTools->canInsert( _rxCursorSet );
		return bRet;
	}

	//----------------------------------------------------------------
	sal_Bool OStaticDataAccessTools::canUpdate(const Reference< XPropertySet>& _rxCursorSet) const
	{
		sal_Bool bRet = sal_False;
		if ( ensureLoaded() )
			bRet = m_xDataAccessTools->canUpdate( _rxCursorSet );
		return bRet;
	}

	//----------------------------------------------------------------
	sal_Bool OStaticDataAccessTools::canDelete(const Reference< XPropertySet>& _rxCursorSet) const
	{
		sal_Bool bRet = sal_False;
		if ( ensureLoaded() )
			bRet = m_xDataAccessTools->canDelete( _rxCursorSet );
		return bRet;
	}

	//----------------------------------------------------------------
	Reference< XNameAccess > OStaticDataAccessTools::getFieldsByCommandDescriptor( const Reference< XConnection >& _rxConnection,
		const sal_Int32 _nCommandType, const ::rtl::OUString& _rCommand,
			Reference< XComponent >& _rxKeepFieldsAlive, ::dbtools::SQLExceptionInfo* _pErrorInfo ) SAL_THROW( ( ) )
	{
		Reference< XNameAccess > aFields;
		if ( ensureLoaded() )
			aFields = m_xDataAccessTools->getFieldsByCommandDescriptor( _rxConnection, _nCommandType,
				_rCommand, _rxKeepFieldsAlive, _pErrorInfo );

		return aFields;
	}

	//----------------------------------------------------------------
	Sequence< ::rtl::OUString > OStaticDataAccessTools::getFieldNamesByCommandDescriptor(
		const Reference< XConnection >& _rxConnection, const sal_Int32 _nCommandType,
		const ::rtl::OUString& _rCommand, ::dbtools::SQLExceptionInfo* _pErrorInfo ) SAL_THROW( ( ) )
	{
		Sequence< ::rtl::OUString > aNames;
		if ( ensureLoaded() )
			aNames = m_xDataAccessTools->getFieldNamesByCommandDescriptor( _rxConnection, _nCommandType,
				_rCommand, _pErrorInfo );
		return aNames;
	}

	//----------------------------------------------------------------
    bool OStaticDataAccessTools::isEmbeddedInDatabase( const Reference< XInterface >& _rxComponent, Reference< XConnection >& _rxActualConnection )
    {
        bool bReturn = false;
		if ( ensureLoaded() )
			bReturn = m_xDataAccessTools->isEmbeddedInDatabase( _rxComponent, _rxActualConnection );
        return bReturn;
    }

	//----------------------------------------------------------------
    bool OStaticDataAccessTools::isEmbeddedInDatabase( const Reference< XInterface >& _rxComponent )
    {
        bool bReturn = false;
		if ( ensureLoaded() )
        {
            Reference< XConnection > xDummy;
			bReturn = m_xDataAccessTools->isEmbeddedInDatabase( _rxComponent, xDummy );
        }
        return bReturn;
    }

    //====================================================================
	//= DBToolsObjectFactory
	//====================================================================
	//----------------------------------------------------------------
    DBToolsObjectFactory::DBToolsObjectFactory()
    {
    }

	//----------------------------------------------------------------
    DBToolsObjectFactory::~DBToolsObjectFactory()
    {
    }

	//----------------------------------------------------------------
    ::std::auto_ptr< ::dbtools::FormattedColumnValue > DBToolsObjectFactory::createFormattedColumnValue(
        const ::comphelper::ComponentContext& _rContext, const Reference< XRowSet >& _rxRowSet, const Reference< XPropertySet >& _rxColumn )
    {
        ::std::auto_ptr< ::dbtools::FormattedColumnValue > pValue;
		if ( ensureLoaded() )
            pValue = getFactory()->createFormattedColumnValue( _rContext, _rxRowSet, _rxColumn );
        return pValue;
    }

//........................................................................
}	// namespace svxform
//........................................................................


