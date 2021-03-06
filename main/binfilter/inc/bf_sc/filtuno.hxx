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



#ifndef SC_FILTUNO_HXX
#define SC_FILTUNO_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XIMPORTER_HPP_
#include <com/sun/star/document/XImporter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif


namespace com { namespace sun { namespace star { namespace io {
	class XInputStream;
} } } }
namespace binfilter {

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
	ScFilterOptionsObj_CreateInstance(
		const ::com::sun::star::uno::Reference<
			::com::sun::star::lang::XMultiServiceFactory >& );


class ScFilterOptionsObj : public ::cppu::WeakImplHelper5<
							::com::sun::star::beans::XPropertyAccess,
							::com::sun::star::ui::dialogs::XExecutableDialog,
							::com::sun::star::document::XImporter,
							::com::sun::star::document::XExporter,
							::com::sun::star::lang::XServiceInfo >
{
private:
	::rtl::OUString		aFileName;
	::rtl::OUString		aFilterName;
	::rtl::OUString		aFilterOptions;
	::com::sun::star::uno::Reference<
		::com::sun::star::io::XInputStream > xInputStream;
	sal_Bool			bExport;

public:
							ScFilterOptionsObj();
	virtual					~ScFilterOptionsObj();

	static ::rtl::OUString	getImplementationName_Static();
	static ::com::sun::star::uno::Sequence< ::rtl::OUString> getSupportedServiceNames_Static();

							// XPropertyAccess
	virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
							SAL_CALL getPropertyValues() throw (::com::sun::star::uno::RuntimeException);
	virtual void SAL_CALL	setPropertyValues( const ::com::sun::star::uno::Sequence<
									::com::sun::star::beans::PropertyValue >& aProps )
								throw (::com::sun::star::beans::UnknownPropertyException,
										::com::sun::star::beans::PropertyVetoException,
										::com::sun::star::lang::IllegalArgumentException,
										::com::sun::star::lang::WrappedTargetException,
										::com::sun::star::uno::RuntimeException);

							// XExecutableDialog
	virtual void SAL_CALL	setTitle( const ::rtl::OUString& aTitle )
								throw (::com::sun::star::uno::RuntimeException);
	virtual sal_Int16 SAL_CALL execute() throw (::com::sun::star::uno::RuntimeException);

							// XImporter
	virtual void SAL_CALL	setTargetDocument( const ::com::sun::star::uno::Reference<
									::com::sun::star::lang::XComponent >& xDoc )
								throw (::com::sun::star::lang::IllegalArgumentException,
										::com::sun::star::uno::RuntimeException);

							// XExporter
	virtual void SAL_CALL	setSourceDocument( const ::com::sun::star::uno::Reference<
									::com::sun::star::lang::XComponent >& xDoc )
								throw (::com::sun::star::lang::IllegalArgumentException,
										::com::sun::star::uno::RuntimeException);

							// XServiceInfo
	virtual ::rtl::OUString SAL_CALL getImplementationName()
								throw(::com::sun::star::uno::RuntimeException);
	virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
								throw(::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
								throw(::com::sun::star::uno::RuntimeException);
};


} //namespace binfilter
#endif

