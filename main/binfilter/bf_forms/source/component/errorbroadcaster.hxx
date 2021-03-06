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



#ifndef FORMS_ERRORBROADCASTER_HXX
#define FORMS_ERRORBROADCASTER_HXX

#ifndef _CPPUHELPER_IMPLBASE1_HXX_ 
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLERRORBROADCASTER_HPP_ 
#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_ 
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLEXCEPTION_HPP_ 
#include <com/sun/star/sdbc/SQLException.hpp>
#endif
namespace binfilter {

//.........................................................................
namespace frm
{
//.........................................................................

	//=====================================================================
	//= OErrorBroadcaster
	//=====================================================================
	typedef ::cppu::ImplHelper1	<	::com::sun::star::sdb::XSQLErrorBroadcaster
								>	OErrorBroadcaster_BASE;

	class OErrorBroadcaster : public OErrorBroadcaster_BASE
	{
	private:
		::cppu::OBroadcastHelper&			m_rBHelper;
		::cppu::OInterfaceContainerHelper	m_aErrorListeners;

	protected:
		OErrorBroadcaster( ::cppu::OBroadcastHelper& _rBHelper );
		~OErrorBroadcaster( );

		void SAL_CALL disposing();

		void SAL_CALL onError( const ::com::sun::star::sdbc::SQLException& _rException, const ::rtl::OUString& _rContextDescription );
		void SAL_CALL onError( const ::com::sun::star::sdb::SQLErrorEvent& _rException );

	protected:
	// XSQLErrorBroadcaster
		virtual void SAL_CALL addSQLErrorListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLErrorListener >& _rListener ) throw (::com::sun::star::uno::RuntimeException);
		virtual void SAL_CALL removeSQLErrorListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLErrorListener >& _rListener ) throw (::com::sun::star::uno::RuntimeException);
	};

//.........................................................................
}	// namespace frm
//.........................................................................

}//end of namespace binfilter
#endif // FORMS_ERRORBROADCASTER_HXX

