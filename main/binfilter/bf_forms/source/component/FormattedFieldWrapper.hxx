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



#ifndef _FRM_FORMATTED_FIELD_WRAPPER_HXX_
#define _FRM_FORMATTED_FIELD_WRAPPER_HXX_

#ifndef _FORMS_FORMCOMPONENT_HXX_
#include "FormComponent.hxx"
#endif
namespace binfilter {

//.........................................................................
namespace frm
{

class OEditModel;
//==================================================================
//= OFormattedFieldWrapper
//==================================================================
typedef ::cppu::WeakAggImplHelper3	<	::com::sun::star::io::XPersistObject
									,	::com::sun::star::lang::XServiceInfo
									,	::com::sun::star::util::XCloneable
									>	OFormattedFieldWrapper_Base;

class OFormattedFieldWrapper : public OFormattedFieldWrapper_Base
{
	::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>	m_xServiceFactory;

protected:
	::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation> 		m_xAggregate;

	OEditModel*		m_pEditPart;
		// if we act as formatted this is used to write the EditModel part
	::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject> 	m_xFormattedPart;
		// if we act as formatted, this is the PersistObject interface of our aggregate, used
		// to read and write the FormattedModel part

	OFormattedFieldWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory, sal_Bool _bActAsFormatted);
		// if _bActAsFormatted is sal_False, the state is undetermined until somebody calls ::read or does
		// anything which requires a living aggregate
	OFormattedFieldWrapper( const OFormattedFieldWrapper* _pCloneSource );
	virtual ~OFormattedFieldWrapper();

	friend InterfaceRef SAL_CALL OFormattedFieldWrapper_CreateInstance_ForceFormatted(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
	friend InterfaceRef SAL_CALL OFormattedFieldWrapper_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

public:
	// UNO
	DECLARE_UNO3_AGG_DEFAULTS(OFormattedFieldWrapper, OWeakAggObject);
	virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
	virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

	// XPersistObject
	virtual ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);
	virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
	virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

	// XCloneable
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException);

protected:
	/// ensure we're in a defined state, which means a FormattedModel _OR_ an EditModel
	void ensureAggregate();
};

//.........................................................................
}
//.........................................................................

}//end of namespace binfilter
#endif // _FRM_FORMATTED_FIELD_WRAPPER_HXX_

