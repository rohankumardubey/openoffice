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


#ifndef _UNOFRAME_HXX
#define _UNOFRAME_HXX

#include <bf_svtools/bf_solar.h>

#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <bf_sfx2/objsh.hxx>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEMBEDDEDOBJECTSUPPLIER_HPP_
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFRAME_HPP_
#include <com/sun/star/text/XTextFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif

#include <cppuhelper/implbase1.hxx> 
#include <cppuhelper/implbase2.hxx> 
#include <cppuhelper/implbase3.hxx> 
#include <cppuhelper/implbase6.hxx> 

namespace binfilter {

class SwDoc;
/*-----------------12.02.98 11:21-------------------

--------------------------------------------------*/
class BaseFrameProperties_Impl;
class SwXFrame : public cppu::WeakImplHelper6
<
	::com::sun::star::lang::XServiceInfo,
	::com::sun::star::beans::XPropertySet,
	::com::sun::star::beans::XPropertyState,
	::com::sun::star::drawing::XShape,
	::com::sun::star::container::XNamed,
	::com::sun::star::lang::XUnoTunnel
>,
	public SwClient
{
	SwEventListenerContainer		aLstnrCntnr;
	SfxItemPropertySet				aPropSet;
	const SfxItemPropertyMap* 		_pMap;
	SwDoc*							mpDoc;

	const FlyCntType 				eType;

	// Descriptor-interface
	BaseFrameProperties_Impl*		pProps;
	sal_Bool 						bIsDescriptor;
	String 							sName;

protected:
	::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >	mxStyleData;
	::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >	mxStyleFamily;

	virtual ~SwXFrame();
public:
	SwXFrame(FlyCntType eSet,
				const SfxItemPropertyMap* 	pMap,
				SwDoc *pDoc ); //Descriptor-If
	SwXFrame(SwFrmFmt& rFrmFmt, FlyCntType eSet,
				const SfxItemPropertyMap* 	pMap);
	

	static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

	//XUnoTunnel
	virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

	TYPEINFO();

	//XNamed
	virtual ::rtl::OUString SAL_CALL getName(void) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL setName(const ::rtl::OUString& Name_) throw( ::com::sun::star::uno::RuntimeException );

	//XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

 	//XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

   //XShape
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPosition( const ::com::sun::star::awt::Point& aPosition ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSize( const ::com::sun::star::awt::Size& aSize ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);

	//XShapeDescriptor
	virtual ::rtl::OUString SAL_CALL getShapeType(void) throw( ::com::sun::star::uno::RuntimeException );

	//Basisimplementierung
	//XComponent
    virtual void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);

	virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  SAL_CALL getAnchor(void) throw( ::com::sun::star::uno::RuntimeException );

	//XServiceInfo
	virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
	virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

	//SwClient
	virtual void 	Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

	void attachToRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange)throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    void attach( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xTextRange ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

	SwFrmFmt* 		GetFrmFmt() const
	{
		return PTR_CAST ( SwFrmFmt, GetRegisteredIn() );
	}
	FlyCntType 		GetFlyCntType()const {return eType;}

	sal_Bool 			IsDescriptor() const {return bIsDescriptor;}
	void			ResetDescriptor();
	static SdrObject *GetOrCreateSdrObject( SwFlyFrmFmt *pFmt );
};
/*-----------------20.02.98 11:28-------------------

--------------------------------------------------*/
typedef cppu::WeakImplHelper3
<
	::com::sun::star::text::XTextFrame,
	::com::sun::star::container::XEnumerationAccess,
	::com::sun::star::document::XEventsSupplier
>
SwXTextFrameBaseClass;

class SwXTextFrame : public SwXTextFrameBaseClass,
	public SwXText,
	public SwXFrame
{
	const SfxItemPropertyMap* 	_pMap;

protected:
	virtual const SwStartNode *GetStartNode() const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >         createCursor()throw(::com::sun::star::uno::RuntimeException);
	virtual ~SwXTextFrame();
public:
	SwXTextFrame(SwDoc *pDoc);
	SwXTextFrame(SwFrmFmt& rFmt);
	

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw();
    virtual void SAL_CALL release(  ) throw();

	//XTypeProvider
	virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

	//XTextFrame
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  SAL_CALL getText(void) throw( ::com::sun::star::uno::RuntimeException );

	//XText
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursor(void) throw( ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursorByRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & aTextPosition) throw( ::com::sun::star::uno::RuntimeException );

	//XEnumerationAccess - frueher XParagraphEnumerationAccess
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createEnumeration(void) throw( ::com::sun::star::uno::RuntimeException );

	//XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

	//XTextContent
    virtual void SAL_CALL attach( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xTextRange ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getAnchor(  ) throw(::com::sun::star::uno::RuntimeException);

	//XComponent
    virtual void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);

	//XServiceInfo
	virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
	virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    // XEventsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw(::com::sun::star::uno::RuntimeException);

	//XUnoTunnel
	virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XPropertySet
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    void * SAL_CALL operator new( size_t ) throw();
	void SAL_CALL operator delete( void * ) throw();
};
/*-----------------20.02.98 11:28-------------------

--------------------------------------------------*/
typedef cppu::WeakImplHelper2
<
	::com::sun::star::text::XTextContent,
	::com::sun::star::document::XEventsSupplier
>
SwXTextGraphicObjectBaseClass;
class SwXTextGraphicObject : public SwXTextGraphicObjectBaseClass,
							public SwXFrame
{
protected:
	virtual ~SwXTextGraphicObject();
public:
	SwXTextGraphicObject( SwDoc *pDoc );
	SwXTextGraphicObject(SwFrmFmt& rFmt);
	

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw();
    virtual void SAL_CALL release(  ) throw();

	//XTypeProvider
	virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

	//XTextContent
	virtual void SAL_CALL attach(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  SAL_CALL getAnchor(void) throw( ::com::sun::star::uno::RuntimeException );

	//XComponent
	virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );

	//XServiceInfo
	virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
	virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    // XEventsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw(::com::sun::star::uno::RuntimeException);
	void * SAL_CALL operator new( size_t ) throw();
	void SAL_CALL operator delete( void * ) throw();
};
/*-----------------20.02.98 11:28-------------------

--------------------------------------------------*/
typedef cppu::WeakImplHelper3
<
	::com::sun::star::text::XTextContent,
	::com::sun::star::document::XEmbeddedObjectSupplier,
	::com::sun::star::document::XEventsSupplier
>SwXTextEmbeddedObjectBaseClass;

class SwXTextEmbeddedObject : public SwXTextEmbeddedObjectBaseClass,
								public SwXFrame
{
protected:
	virtual ~SwXTextEmbeddedObject();
public:
	SwXTextEmbeddedObject( SwDoc *pDoc );
	SwXTextEmbeddedObject(SwFrmFmt& rFmt);
	

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw();
    virtual void SAL_CALL release(  ) throw();

	//XTypeProvider
	virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

	//XTextContent
	virtual void SAL_CALL attach(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  SAL_CALL getAnchor(void) throw( ::com::sun::star::uno::RuntimeException );

	//XComponent
	virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );

	//XEmbeddedObjectSupplier,
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >  SAL_CALL getEmbeddedObject(void) throw( ::com::sun::star::uno::RuntimeException );

	//XServiceInfo
	virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
	virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    // XEventsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw(::com::sun::star::uno::RuntimeException);
	void * SAL_CALL operator new( size_t ) throw();
	void SAL_CALL operator delete( void * ) throw();
};



class SwXOLEListener : public cppu::WeakImplHelper1
<
	::com::sun::star::util::XModifyListener
>,
	public SwClient
{
	SvPtrarr aFmts;
 	::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > GetModel( const SwFmt& rFmt, SwOLENode** ppNd = 0 ) const;
	SfxObjectShell* GetObjShell( const SwFmt& rFmt,
									SwOLENode** ppNd = 0 ) const;
	sal_uInt16 FindEntry( const ::com::sun::star::lang::EventObject& Source, SwOLENode** ppNd = 0 );
public:

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::util::XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException);

	sal_Bool AddOLEFmt( SwFrmFmt& rFmt );
	void Modify( SfxPoolItem*, SfxPoolItem* );
};



} //namespace binfilter
#endif

