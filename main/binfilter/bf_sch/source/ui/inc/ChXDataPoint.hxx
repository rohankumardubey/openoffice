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



#ifndef _SCH_DATA_POINT_HXX
#define _SCH_DATA_POINT_HXX

#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XSHAPEDESCRIPTOR_HPP__
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

// header for SvxItemPropertySet
#ifndef SVX_UNOPROV_HXX
#include <bf_svx/unoprov.hxx>
#endif
namespace binfilter {

class ChartModel;

class ChXDataPoint : public cppu::WeakImplHelper5<
	::com::sun::star::beans::XPropertySet,
	::com::sun::star::beans::XPropertyState,
	::com::sun::star::drawing::XShapeDescriptor,
	::com::sun::star::lang::XServiceInfo,
	::com::sun::star::lang::XUnoTunnel >
{
private:
	SvxItemPropertySet maPropSet;
	ChartModel* mpModel;

	sal_Int32	mnCol, mnRow;

	::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getStatisticsProperties( const sal_Int32 nId ) const;
    void AddDataPointAttr( SfxItemSet& rOutAttributes );

public:
	ChXDataPoint( sal_Int32 _Col, sal_Int32 _Row, ChartModel* _Model = NULL );
	virtual ~ChXDataPoint();

	sal_Int32 GetCol() const		{ return mnCol; }
	sal_Int32 GetRow() const		{ return mnRow; }

	static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();

	// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
		throw( ::com::sun::star::uno::RuntimeException );

	// XPropertySet
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
		throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName,
											const ::com::sun::star::uno::Any& aValue )
		throw( ::com::sun::star::beans::UnknownPropertyException,
			   ::com::sun::star::beans::PropertyVetoException,
			   ::com::sun::star::lang::IllegalArgumentException,
			   ::com::sun::star::lang::WrappedTargetException,
			   ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName )
		throw( ::com::sun::star::beans::UnknownPropertyException,
			   ::com::sun::star::lang::WrappedTargetException,
			   ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
				const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
		throw( ::com::sun::star::beans::UnknownPropertyException,
			   ::com::sun::star::lang::WrappedTargetException,
			   ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
				const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener )
		throw( ::com::sun::star::beans::UnknownPropertyException,
			   ::com::sun::star::lang::WrappedTargetException,
			   ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName,
				const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
		throw( ::com::sun::star::beans::UnknownPropertyException,
			   ::com::sun::star::lang::WrappedTargetException,
			   ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
				const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
		throw( ::com::sun::star::beans::UnknownPropertyException,
			   ::com::sun::star::lang::WrappedTargetException,
			   ::com::sun::star::uno::RuntimeException );

	// XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName )
		throw( ::com::sun::star::beans::UnknownPropertyException,
			   ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates(
		const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName )
		throw( ::com::sun::star::beans::UnknownPropertyException,
			   ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName )
		throw( ::com::sun::star::beans::UnknownPropertyException,
			   ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName )
		throw( ::com::sun::star::beans::UnknownPropertyException,
			   ::com::sun::star::lang::WrappedTargetException,
			   ::com::sun::star::uno::RuntimeException );

	// XShapeDescriptor
    virtual ::rtl::OUString SAL_CALL getShapeType() throw( ::com::sun::star::uno::RuntimeException );

	// XServiceInfo
	virtual ::rtl::OUString SAL_CALL getImplementationName()
		throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
		throw( ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
		throw( ::com::sun::star::uno::RuntimeException );

	// XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier )
		throw( ::com::sun::star::uno::RuntimeException );
};

} //namespace binfilter
#endif	// _SCH_DATA_POINT_HXX

