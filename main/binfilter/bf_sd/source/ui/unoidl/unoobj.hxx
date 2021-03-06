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


#ifndef _UNOOBJ_HXX
#define _UNOOBJ_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_ 
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_ 
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_ 
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _SVDPOOL_HXX //autogen
#include <bf_svx/svdpool.hxx>
#endif

#ifndef _SVX_UNOMASTER_HXX
#include <bf_svx/unomaster.hxx>
#endif

#include <bf_svx/unoipset.hxx>

#include <cppuhelper/implbase2.hxx>
struct SvEventDescription;
namespace binfilter {

class SdrObject;
class SdXImpressDocument;
class SdAnimationInfo;

class SdXShape : public SvxShapeMaster,
				 public ::com::sun::star::document::XEventsSupplier
{
	friend class SdUnoEventsAccess;

private:
	SvxShape* mpShape;
	SvxItemPropertySet	maPropSet;
	const SfxItemPropertyMap* mpMap;
	SdXImpressDocument* mpModel;

	void SetStyleSheet( const ::com::sun::star::uno::Any& rAny ) throw( ::com::sun::star::lang::IllegalArgumentException );
	::com::sun::star::uno::Any GetStyleSheet() const throw( ::com::sun::star::beans::UnknownPropertyException  );

	// Intern
	SdAnimationInfo* GetAnimationInfo( sal_Bool bCreate = sal_False ) const throw();
	sal_Bool IsPresObj() const throw();
	void SetPresObj( sal_Bool bPresObj ) throw();

	sal_Bool IsEmptyPresObj() const throw();
	void SetEmptyPresObj( sal_Bool bEmpty ) throw();

	sal_Bool IsMasterDepend() const throw();
	void SetMasterDepend( sal_Bool bDepend ) throw();

	SdrObject* GetSdrObject() const throw();
	sal_Int32 GetPresentationOrderPos() const throw();
	void SetPresentationOrderPos( sal_Int32 nPos ) throw();

	::com::sun::star::uno::Sequence< sal_Int8 >* mpImplementationId;

public:
	SdXShape(SvxShape* pShape, SdXImpressDocument* pModel) throw();
	virtual ~SdXShape() throw();

	virtual sal_Bool queryAggregation( const ::com::sun::star::uno::Type & rType, ::com::sun::star::uno::Any& aAny );
	virtual void dispose();

	// XInterface
	virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
	virtual void SAL_CALL acquire() throw();
	virtual void SAL_CALL release() throw();

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

	//XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException); 

    //XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

	// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

	// XEventsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw(::com::sun::star::uno::RuntimeException);
};

const SvEventDescription* ImplGetSupportedMacroItems();

} //namespace binfilter
#endif


