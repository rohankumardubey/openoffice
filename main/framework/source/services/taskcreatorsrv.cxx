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
#include "precompiled_framework.hxx"
#include "services/taskcreatorsrv.hxx"

//_______________________________________________
// own includes
#include <helper/persistentwindowstate.hxx>
#include <helper/tagwindowasmodified.hxx>
#include <helper/titlebarupdate.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <loadenv/targethelper.hxx>
#include <services.h>

//_______________________________________________
// interface includes
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>

//_______________________________________________
// other includes
#include <svtools/colorcfg.hxx>
#include <vcl/svapp.hxx>

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#include <vcl/window.hxx>

//_______________________________________________
// namespaces

namespace framework
{

//-----------------------------------------------
const ::rtl::OUString TaskCreatorService::ARGUMENT_PARENTFRAME                   = ::rtl::OUString::createFromAscii("ParentFrame"                   ); // XFrame
const ::rtl::OUString TaskCreatorService::ARGUMENT_FRAMENAME                     = ::rtl::OUString::createFromAscii("FrameName"                     ); // OUString
const ::rtl::OUString TaskCreatorService::ARGUMENT_MAKEVISIBLE                   = ::rtl::OUString::createFromAscii("MakeVisible"                   ); // sal_Bool
const ::rtl::OUString TaskCreatorService::ARGUMENT_CREATETOPWINDOW               = ::rtl::OUString::createFromAscii("CreateTopWindow"               ); // sal_Bool
const ::rtl::OUString TaskCreatorService::ARGUMENT_POSSIZE                       = ::rtl::OUString::createFromAscii("PosSize"                       ); // Rectangle
const ::rtl::OUString TaskCreatorService::ARGUMENT_CONTAINERWINDOW               = ::rtl::OUString::createFromAscii("ContainerWindow"               ); // XWindow
const ::rtl::OUString TaskCreatorService::ARGUMENT_SUPPORTPERSISTENTWINDOWSTATE  = ::rtl::OUString::createFromAscii("SupportPersistentWindowState"  ); // sal_Bool
const ::rtl::OUString TaskCreatorService::ARGUMENT_ENABLE_TITLEBARUPDATE         = ::rtl::OUString::createFromAscii("EnableTitleBarUpdate"          ); // sal_Bool
    
//-----------------------------------------------
DEFINE_XINTERFACE_3(TaskCreatorService                                ,
                    OWeakObject                                       ,
                    DIRECT_INTERFACE(css::lang::XTypeProvider        ),
                    DIRECT_INTERFACE(css::lang::XServiceInfo         ),
                    DIRECT_INTERFACE(css::lang::XSingleServiceFactory))

//-----------------------------------------------
DEFINE_XTYPEPROVIDER_3(TaskCreatorService              ,
                       css::lang::XTypeProvider        ,
                       css::lang::XServiceInfo         ,
                       css::lang::XSingleServiceFactory)

//-----------------------------------------------
DEFINE_XSERVICEINFO_ONEINSTANCESERVICE(TaskCreatorService                ,
									   ::cppu::OWeakObject               ,
									   SERVICENAME_TASKCREATOR           ,
									   IMPLEMENTATIONNAME_FWK_TASKCREATOR)

//-----------------------------------------------
DEFINE_INIT_SERVICE(
                    TaskCreatorService,
                    {
                        /*Attention
                            I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                            to create a new instance of this class by our own supported service factory.
                            see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                        */
                    }
                   )

//-----------------------------------------------
TaskCreatorService::TaskCreatorService(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : ThreadHelpBase     (&Application::GetSolarMutex())
    , ::cppu::OWeakObject(                             )
    , m_xSMGR            (xSMGR                        )
{
}

//-----------------------------------------------
TaskCreatorService::~TaskCreatorService()
{
}

//-----------------------------------------------
css::uno::Reference< css::uno::XInterface > SAL_CALL TaskCreatorService::createInstance()
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    return createInstanceWithArguments(css::uno::Sequence< css::uno::Any >());
}

//-----------------------------------------------
css::uno::Reference< css::uno::XInterface > SAL_CALL TaskCreatorService::createInstanceWithArguments(const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    static ::rtl::OUString     DEFAULTVAL_FRAMENAME                     = ::rtl::OUString();
    static sal_Bool            DEFAULTVAL_MAKEVISIBLE                   = sal_False;
    static sal_Bool            DEFAULTVAL_CREATETOPWINDOW               = sal_True;
	static css::awt::Rectangle DEFAULTVAL_POSSIZE                       = css::awt::Rectangle(0, 0, 0, 0); // only possize=[0,0,0,0] triggers default handling of vcl !
    static sal_Bool            DEFAULTVAL_SUPPORTPERSSISTENTWINDOWSTATE = sal_False;
    static sal_Bool            DEFAULTVAL_ENABLE_TITLEBARUPDATE         = sal_True;

    ::comphelper::SequenceAsHashMap lArgs(lArguments);

	css::uno::Reference< css::frame::XFrame > xParentFrame                  = lArgs.getUnpackedValueOrDefault(TaskCreatorService::ARGUMENT_PARENTFRAME                  , css::uno::Reference< css::frame::XFrame >());
    ::rtl::OUString                           sFrameName                    = lArgs.getUnpackedValueOrDefault(TaskCreatorService::ARGUMENT_FRAMENAME                    , DEFAULTVAL_FRAMENAME                       );
    sal_Bool                                  bVisible                      = lArgs.getUnpackedValueOrDefault(TaskCreatorService::ARGUMENT_MAKEVISIBLE                  , DEFAULTVAL_MAKEVISIBLE                     );
    sal_Bool                                  bCreateTopWindow              = lArgs.getUnpackedValueOrDefault(TaskCreatorService::ARGUMENT_CREATETOPWINDOW              , DEFAULTVAL_CREATETOPWINDOW                 );
    css::awt::Rectangle                       aPosSize                      = lArgs.getUnpackedValueOrDefault(TaskCreatorService::ARGUMENT_POSSIZE                      , DEFAULTVAL_POSSIZE                         );
    css::uno::Reference< css::awt::XWindow >  xContainerWindow              = lArgs.getUnpackedValueOrDefault(TaskCreatorService::ARGUMENT_CONTAINERWINDOW              , css::uno::Reference< css::awt::XWindow >() );
    sal_Bool                                  bSupportPersistentWindowState = lArgs.getUnpackedValueOrDefault(TaskCreatorService::ARGUMENT_SUPPORTPERSISTENTWINDOWSTATE , DEFAULTVAL_SUPPORTPERSSISTENTWINDOWSTATE   );
    sal_Bool                                  bEnableTitleBarUpdate         = lArgs.getUnpackedValueOrDefault(TaskCreatorService::ARGUMENT_ENABLE_TITLEBARUPDATE        , DEFAULTVAL_ENABLE_TITLEBARUPDATE           );

    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    /* } SAFE */

    // We use FrameName property to set it as API name of the new created frame later.
    // But those frame names must be different from the set of special target names as e.g. _blank, _self etcpp !
    ::rtl::OUString sRightName = impl_filterNames(sFrameName);

    // if no external frame window was given ... create a new one.
    if ( ! xContainerWindow.is())
	{
		css::uno::Reference< css::awt::XWindow > xParentWindow;
		if (xParentFrame.is())
			xParentWindow = xParentFrame->getContainerWindow();

		// Parent has no own window ...
		// So we have to create a top level window always !
		if ( ! xParentWindow.is())
			bCreateTopWindow = sal_True;

        xContainerWindow = implts_createContainerWindow(xParentWindow, aPosSize, bCreateTopWindow);
	}
    
    //------------------->
    // HACK  #125187# + #i53630#
    // Mark all document windows as "special ones", so VCL can bind
    // special features to it. Because VCL doesnt know anything about documents ...
    // Note: Doing so it's no longer supported, that e.g. our wizards can use findFrame(_blank)
    // to create it's previes frames. They must do it manually by using WindowDescriptor+Toolkit!
    css::uno::Reference< css::frame::XDesktop > xDesktop(xParentFrame, css::uno::UNO_QUERY);
    ::sal_Bool bTopLevelDocumentWindow = (
                                            (sRightName.getLength () < 1) &&
                                            (
                                                (! xParentFrame.is() )    ||
                                                (  xDesktop.is()     )
                                            )
                                         );
    if (bTopLevelDocumentWindow)
        implts_applyDocStyleToWindow(xContainerWindow);
    //------------------->

    // create the new frame
    css::uno::Reference< css::frame::XFrame > xFrame = implts_createFrame(xParentFrame, xContainerWindow, sRightName);
    
    // special freature:
    // A special listener will restore pos/size states in case
    // a component was loaded into the frame first time.
    if (bSupportPersistentWindowState)
        implts_establishWindowStateListener(xFrame);
    
    // special feature: On Mac we need tagging the window in case
    // the underlying model was modified.
    // VCL will ignore our calls in case different platform then Mac
    // is used ...
    if (bTopLevelDocumentWindow)
        implts_establishDocModifyListener (xFrame);
    
    // special freature:
    // A special listener will update title bar (text and icon)
    // if component of frame will be changed.
    if (bEnableTitleBarUpdate)
        implts_establishTitleBarUpdate(xFrame);
    
    // Make it visible directly here ... 
    // if its required from outside.
    if (bVisible)
        xContainerWindow->setVisible(bVisible);

    return css::uno::Reference< css::uno::XInterface >(xFrame, css::uno::UNO_QUERY_THROW);
}

//-----------------------------------------------
void TaskCreatorService::implts_applyDocStyleToWindow(const css::uno::Reference< css::awt::XWindow >& xWindow) const
{
    // SYNCHRONIZED ->
    ::vos::OClearableGuard aSolarGuard(Application::GetSolarMutex());
    Window* pVCLWindow = VCLUnoHelper::GetWindow(xWindow);
    if (pVCLWindow)
        pVCLWindow->SetExtendedStyle(WB_EXT_DOCUMENT);
    aSolarGuard.clear();
    // <- SYNCHRONIZED
}

//-----------------------------------------------
css::uno::Reference< css::awt::XWindow > TaskCreatorService::implts_createContainerWindow( const css::uno::Reference< css::awt::XWindow >& xParentWindow ,
                                                                                           const css::awt::Rectangle&                      aPosSize      ,
                                                                                                 sal_Bool                                  bTopWindow    )
{
    // SAFE  ->
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SAFE
    
    // get toolkit to create task container window
    css::uno::Reference< css::awt::XToolkit > xToolkit( xSMGR->createInstance( SERVICENAME_VCLTOOLKIT ), css::uno::UNO_QUERY_THROW);

    // Check if child frames can be created realy. We need at least a valid window at the parent frame ...
    css::uno::Reference< css::awt::XWindowPeer > xParentWindowPeer;
    if ( ! bTopWindow)
    {
        if ( ! xParentWindow.is())
            bTopWindow = sal_False;
        else
            xParentWindowPeer = css::uno::Reference< css::awt::XWindowPeer >(xParentWindow, css::uno::UNO_QUERY_THROW);
    }
        
    // describe window properties.
    css::awt::WindowDescriptor aDescriptor;
    if (bTopWindow)
    {
        aDescriptor.Type                =   css::awt::WindowClass_TOP                       ;
        aDescriptor.WindowServiceName   =   DECLARE_ASCII("window")                         ;
        aDescriptor.ParentIndex         =   -1                                              ;
        aDescriptor.Parent              =   css::uno::Reference< css::awt::XWindowPeer >()  ;
        aDescriptor.Bounds              =   aPosSize                                        ;
        aDescriptor.WindowAttributes    =   css::awt::WindowAttribute::BORDER               |
                                            css::awt::WindowAttribute::MOVEABLE             |
                                            css::awt::WindowAttribute::SIZEABLE             |
                                            css::awt::WindowAttribute::CLOSEABLE            |
                                            css::awt::VclWindowPeerAttribute::CLIPCHILDREN  ;
    }
    else
    {
        aDescriptor.Type                =   css::awt::WindowClass_TOP                       ;
        aDescriptor.WindowServiceName   =   DECLARE_ASCII("dockingwindow")                  ;
        aDescriptor.ParentIndex         =   1                                               ;
        aDescriptor.Parent              =   xParentWindowPeer                               ;
        aDescriptor.Bounds              =   aPosSize                                        ;
        aDescriptor.WindowAttributes    =   css::awt::VclWindowPeerAttribute::CLIPCHILDREN  ;
    }
    
    // create a new blank container window and get access to parent container to append new created task.
    css::uno::Reference< css::awt::XWindowPeer > xPeer      = xToolkit->createWindow( aDescriptor );
    css::uno::Reference< css::awt::XWindow >     xWindow    ( xPeer, css::uno::UNO_QUERY );
    if ( ! xWindow.is())
        throw css::uno::Exception(::rtl::OUString::createFromAscii("TaskCreator service was not able to create suitable frame window."),
                                  static_cast< ::cppu::OWeakObject* >(this));
	if (bTopWindow)
		xPeer->setBackground(::svtools::ColorConfig().GetColorValue(::svtools::APPBACKGROUND).nColor);
	else
		xPeer->setBackground(0xffffffff);
    
    return xWindow;
}

//-----------------------------------------------
css::uno::Reference< css::frame::XFrame > TaskCreatorService::implts_createFrame( const css::uno::Reference< css::frame::XFrame >& xParentFrame    ,
                                                                                  const css::uno::Reference< css::awt::XWindow >&  xContainerWindow,
                                                                                  const ::rtl::OUString&                           sName           )
{
    // SAFE  ->
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SAFE
    
    // create new frame.
    css::uno::Reference< css::frame::XFrame > xNewFrame( xSMGR->createInstance( SERVICENAME_FRAME ), css::uno::UNO_QUERY_THROW );

    // Set window on frame.
    // Do it before calling any other interface methods ...
    // The new created frame must be initialized before you can do anything else there.
    xNewFrame->initialize( xContainerWindow );

    // Put frame to the frame tree.
    // Note: The property creator/parent will be set on the new putted frame automaticly ... by the parent container.
	if (xParentFrame.is())
	{
		css::uno::Reference< css::frame::XFramesSupplier > xSupplier  (xParentFrame, css::uno::UNO_QUERY_THROW);
		css::uno::Reference< css::frame::XFrames >         xContainer = xSupplier->getFrames();
		xContainer->append( xNewFrame );
	}

    // Set it's API name (if there is one from outside)
    if (sName.getLength())
        xNewFrame->setName( sName );

	return xNewFrame;
}

//-----------------------------------------------
void TaskCreatorService::implts_establishWindowStateListener( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    // SAFE  ->
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SAFE

    // Special feature: It's allowed for frames using a top level window only!
    // We must create a special listener service and couple it with the new created task frame.
    // He will restore or save the window state of it ...
    // See used classes for further informations too.
    PersistentWindowState* pPersistentStateHandler = new PersistentWindowState(xSMGR);
    css::uno::Reference< css::lang::XInitialization > xInit(static_cast< ::cppu::OWeakObject* >(pPersistentStateHandler), css::uno::UNO_QUERY_THROW);

    css::uno::Sequence< css::uno::Any > lInitData(1);
    lInitData[0] <<= xFrame;
    xInit->initialize(lInitData);
}

//-----------------------------------------------
void TaskCreatorService::implts_establishDocModifyListener( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    // SAFE  ->
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SAFE

    // Special feature: It's allowed for frames using a top level window only!
    // We must create a special listener service and couple it with the new created task frame.
    // It will tag the window as modified if the underlying model was modified ...
    TagWindowAsModified* pTag = new TagWindowAsModified(xSMGR);
    css::uno::Reference< css::lang::XInitialization > xInit(static_cast< ::cppu::OWeakObject* >(pTag), css::uno::UNO_QUERY_THROW);

    css::uno::Sequence< css::uno::Any > lInitData(1);
    lInitData[0] <<= xFrame;
    xInit->initialize(lInitData);
}

//-----------------------------------------------
void TaskCreatorService::implts_establishTitleBarUpdate( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    // SAFE  ->
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SAFE

    TitleBarUpdate* pHelper = new TitleBarUpdate (xSMGR);
    css::uno::Reference< css::lang::XInitialization > xInit(static_cast< ::cppu::OWeakObject* >(pHelper), css::uno::UNO_QUERY_THROW);

    css::uno::Sequence< css::uno::Any > lInitData(1);
    lInitData[0] <<= xFrame;
    xInit->initialize(lInitData);
}

//-----------------------------------------------
::rtl::OUString TaskCreatorService::impl_filterNames( const ::rtl::OUString& sName )
{
    ::rtl::OUString sFiltered;
    if (TargetHelper::isValidNameForFrame(sName))
        sFiltered = sName;
    return sFiltered;
}

} // namespace framework
