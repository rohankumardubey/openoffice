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



#ifndef _SVX_UNOFIELD_HXX
#define _SVX_UNOFIELD_HXX

#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_ 
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_ 
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_ 
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELD_HPP_
#include <com/sun/star/text/XTextField.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_ 
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif

#include <comphelper/servicehelper.hxx>

#include <bf_svx/mutxhelp.hxx>
namespace binfilter {
class SfxItemPropertySet;

// ids usable for the constructor
#define ID_DATEFIELD		0
#define ID_URLFIELD			1
#define ID_PAGEFIELD		2
#define ID_PAGESFIELD		3
#define ID_TIMEFIELD		4
#define ID_FILEFIELD		5
#define ID_TABLEFIELD		6
#define ID_EXT_TIMEFIELD	7
#define ID_EXT_FILEFIELD	8
#define ID_AUTHORFIELD		9
#define ID_MEASUREFIELD		10
#define ID_EXT_DATEFIELD	11
#define ID_UNKNOWN			12		// this must be the last entry

class SvxUnoFieldData_Impl;
class SvxFieldData;

class SvxUnoTextField : public SvxMutexHelper,
						public ::cppu::OComponentHelper,
						public ::com::sun::star::text::XTextField,
						public ::com::sun::star::beans::XPropertySet,
						public ::com::sun::star::lang::XServiceInfo,
						public ::com::sun::star::lang::XUnoTunnel
{
private:
	::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > mxAnchor;
	SfxItemPropertySet*	mpPropSet;
	sal_Int32				mnServiceId;
	SvxUnoFieldData_Impl*	mpImpl;

protected:
	::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > maTypeSequence;

public:
	SvxUnoTextField( sal_Int32 nServiceId ) throw();
	SvxUnoTextField( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > xAnchor, const ::rtl::OUString& rPresentation, const SvxFieldData* pFieldData ) throw();
	virtual	~SvxUnoTextField() throw();

	// Intern
	virtual sal_Int32 GetFieldId( const SvxFieldData* pFieldData ) const throw();
	SvxFieldData* CreateFieldData() const throw();
	void SetAnchor( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > xAnchor ) { mxAnchor = xAnchor; }

	UNO3_GETIMPLEMENTATION_DECL( SvxUnoTextField )

	// ::com::sun::star::uno::XInterface
	virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
	virtual void SAL_CALL acquire() throw();
	virtual void SAL_CALL release() throw();

	// ::com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

	// XTextField
    virtual ::rtl::OUString SAL_CALL getPresentation( sal_Bool bShowCommand ) throw(::com::sun::star::uno::RuntimeException);

	// XTextContent
    virtual void SAL_CALL attach( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xTextRange ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getAnchor(  ) throw(::com::sun::star::uno::RuntimeException);

	// ::com::sun::star::lang::XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);

	// XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

	// OComponentHelper
	virtual void SAL_CALL disposing();

	// XServiceInfo
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
};

}//end of namespace binfilter
#endif

