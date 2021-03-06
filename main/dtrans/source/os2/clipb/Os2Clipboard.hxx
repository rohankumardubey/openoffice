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



#ifndef _OS2CLIPBOARD_HXX_
#define _OS2CLIPBOARD_HXX_

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <cppuhelper/compbase4.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/datatransfer/clipboard/RenderingCapabilities.hpp>
#include "Os2Transferable.hxx"

// the service names
#define OS2_CLIPBOARD_SERVICE_NAME "com.sun.star.datatransfer.clipboard.SystemClipboard"

// the implementation names
#define OS2_CLIPBOARD_IMPL_NAME "com.sun.star.datatransfer.clipboard.Os2Clipboard"

// the registry key names
#define OS2_CLIPBOARD_REGKEY_NAME "/com.sun.star.datatransfer.clipboard.Os2Clipboard/UNO/SERVICES/com.sun.star.datatransfer.clipboard.SystemClipboard"

namespace os2 {

class Os2Clipboard : 
	//public cppu::WeakComponentImplHelper3< ::com::sun::star::datatransfer::clipboard::XClipboardEx, ::com::sun::star::datatransfer::clipboard::XClipboardNotifier, ::com::sun::star::lang::XServiceInfo >
    public ::cppu::WeakComponentImplHelper4 < \
    ::com::sun::star::datatransfer::clipboard::XClipboardEx, \
    ::com::sun::star::datatransfer::clipboard::XClipboardNotifier, \
    ::com::sun::star::lang::XServiceInfo, \
    ::com::sun::star::lang::XInitialization >
{

public:
	Os2Clipboard();
	~Os2Clipboard();

	/*
	 * XInitialization
	 */
	virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
		throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

	/*
	 * XServiceInfo
	 */
	virtual ::rtl::OUString SAL_CALL getImplementationName()
		throw(::com::sun::star::uno::RuntimeException);
	virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) 
		throw(::com::sun::star::uno::RuntimeException);

	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() 
		throw(::com::sun::star::uno::RuntimeException);

	/*
	 * XClipboard
	 */
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > SAL_CALL getContents() 
		throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL setContents( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTransferable, const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner ) 
		throw( ::com::sun::star::uno::RuntimeException );
	virtual ::rtl::OUString SAL_CALL getName() 
		throw( ::com::sun::star::uno::RuntimeException );

	/*
	 * XClipboardEx
	 */
	virtual sal_Int8 SAL_CALL getRenderingCapabilities()
		throw( ::com::sun::star::uno::RuntimeException );
		
	/*
	 * XClipboardNotifier
	 */
	virtual void SAL_CALL addClipboardListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener ) 
		throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL removeClipboardListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener ) 
		throw( ::com::sun::star::uno::RuntimeException );
	void SAL_CALL notifyAllClipboardListener( );

public:
	sal_Bool m_bInSetClipboardData;

private:
	HAB	hAB;
	HWND	hObjWnd;
	ULONG	hText, hBitmap;	// handles to previous clipboard data

	::osl::Mutex m_aMutex;
	::rtl::OUString m_aName;
	
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > m_aContents;
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner > m_aOwner;
	
	sal_Bool m_bInitialized;

};

} // namespace Os2

// ------------------------------------------------------------------------

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL Os2Clipboard_getSupportedServiceNames();
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Os2Clipboard_createInstance( 
	const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xMultiServiceFactory);

#endif

