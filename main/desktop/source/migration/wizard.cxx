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
#include "precompiled_desktop.hxx"

#include <migration.hxx>
#include "wizard.hxx"
#include "wizard.hrc"
#include "pages.hxx"
#include "app.hxx"

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/bootstrap.hxx>

#include <comphelper/processfactory.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/datetime.hxx>
#include <osl/file.hxx>
#include <osl/time.h>
#include <osl/module.hxx>
#include <unotools/bootstrap.hxx>
#include <vcl/msgbox.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>

using namespace svt;
using namespace rtl;
using namespace osl;
using namespace utl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::container;

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

namespace desktop
{

const FirstStartWizard::WizardState FirstStartWizard::STATE_WELCOME      = 0;
const FirstStartWizard::WizardState FirstStartWizard::STATE_LICENSE      = 1;
const FirstStartWizard::WizardState FirstStartWizard::STATE_MIGRATION    = 2;
const FirstStartWizard::WizardState FirstStartWizard::STATE_USER         = 3;
const FirstStartWizard::WizardState FirstStartWizard::STATE_UPDATE_CHECK = 4;
const FirstStartWizard::WizardState FirstStartWizard::STATE_REGISTRATION = 5;

static sal_Int32 getBuildId()
{
    ::rtl::OUString aDefault;
    ::rtl::OUString aBuildIdData = utl::Bootstrap::getBuildIdData( aDefault );
    sal_Int32 nBuildId( 0 );
    sal_Int32 nIndex1 = aBuildIdData.indexOf(':');
    sal_Int32 nIndex2 = aBuildIdData.indexOf(')');
    if (( nIndex1 > 0 ) && ( nIndex2 > 0 ) && ( nIndex2-1 > nIndex1+1 ))
    {
        ::rtl::OUString aBuildId = aBuildIdData.copy( nIndex1+1, nIndex2-nIndex1-1 );
        nBuildId = aBuildId.toInt32();
    }
    return nBuildId;
}

WizardResId::WizardResId( sal_uInt16 nId ) :
	ResId( nId, *FirstStartWizard::GetResManager() )
{
}

ResMgr *FirstStartWizard::pResMgr = 0;

ResMgr *FirstStartWizard::GetResManager()
{
    if ( !FirstStartWizard::pResMgr )
    {
        String aMgrName = String::CreateFromAscii( "dkt" );
        FirstStartWizard::pResMgr = ResMgr::CreateResMgr( OUStringToOString( aMgrName, RTL_TEXTENCODING_UTF8 ));
    }
    return FirstStartWizard::pResMgr;
}

FirstStartWizard::FirstStartWizard( Window* pParent, sal_Bool bLicenseNeedsAcceptance, const rtl::OUString &rLicensePath )
    :RoadmapWizard( pParent, WizardResId(DLG_FIRSTSTART_WIZARD),
        WZB_NEXT|WZB_PREVIOUS|WZB_FINISH|WZB_CANCEL|WZB_HELP)
    ,m_bOverride(sal_False)
    ,m_aDefaultPath(0)
	,m_aMigrationPath(0)
    ,m_bDone(sal_False)
	,m_bLicenseNeedsAcceptance( bLicenseNeedsAcceptance )
    ,m_bLicenseWasAccepted(sal_False)
    ,m_bAutomaticUpdChk(sal_True)
    ,m_aThrobber(this, WizardResId(CTRL_THROBBER))
    ,m_aLicensePath( rLicensePath )
{
    FreeResource();
    // ---
//	enableState(STATE_USER, sal_False);
//  enableState(STATE_REGISTRATION, sal_False);

    Size aTPSize(TP_WIDTH, TP_HEIGHT);
    SetPageSizePixel(LogicToPixel(aTPSize, MAP_APPFONT));

    //set help id
    m_pPrevPage->SetHelpId(HID_FIRSTSTART_PREV);
    m_pNextPage->SetHelpId(HID_FIRSTSTART_NEXT);
    m_pCancel->SetHelpId(HID_FIRSTSTART_CANCEL);
    m_pFinish->SetHelpId(HID_FIRSTSTART_FINISH);
    // m_pHelp->SetUniqueId(UID_FIRSTSTART_HELP);
    m_pHelp->Hide();
    m_pHelp->Disable();
    
    // save button lables
    m_sNext = m_pNextPage->GetText();
    m_sCancel = m_pCancel->GetText();

    // save cancel click handler
    m_lnkCancel = m_pCancel->GetClickHdl();

    m_aDefaultPath = defineWizardPagesDependingFromContext();
	activatePath(m_aDefaultPath, sal_True);

    ActivatePage();

    // set text of finish putton:
    m_pFinish->SetText(String(WizardResId(STR_FINISH)));
    // disable "finish button"
    enableButtons(WZB_FINISH, sal_False);
    defaultButton(WZB_NEXT);
}

void FirstStartWizard::DisableButtonsWhileMigration()
{
    enableButtons(0xff, sal_False);
}

::svt::RoadmapWizardTypes::PathId FirstStartWizard::defineWizardPagesDependingFromContext()
{
	::svt::RoadmapWizardTypes::PathId aDefaultPath = 0;

    sal_Bool bPage_Welcome      = sal_True;
    sal_Bool bPage_Migration    = sal_True;
    sal_Bool bPage_User         = sal_True;
    sal_Bool bPage_UpdateCheck  = sal_True;

    bPage_Migration   = Migration::checkMigration();
    bPage_UpdateCheck = showOnlineUpdatePage();

    WizardPath aPath;
    if (bPage_Welcome)
        aPath.push_back(STATE_WELCOME);
    if (bPage_Migration)
        aPath.push_back(STATE_MIGRATION);
    if (bPage_User)
        aPath.push_back(STATE_USER);
    if (bPage_UpdateCheck)
        aPath.push_back(STATE_UPDATE_CHECK);
    
    declarePath(aDefaultPath, aPath);

	// a) If license must be accepted by the user, all direct links
	//    to wizard tab pages must be disabled. Because such pages
	//	  should be accessible only in case license was accepted !
	// b) But if no license should be shown at all ...
	//    such direct links can be enabled by default.
	sal_Bool bAllowDirectLink = true;

    if (bPage_User)
        enableState(STATE_USER, bAllowDirectLink);
    if (bPage_UpdateCheck)
        enableState(STATE_UPDATE_CHECK, bAllowDirectLink);
    if (bPage_Migration)
        enableState(STATE_MIGRATION, bAllowDirectLink);

	return aDefaultPath;
}

// catch F1 and disable help
long FirstStartWizard::PreNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyCode& rKey = rNEvt.GetKeyEvent()->GetKeyCode();
        if( rKey.GetCode() == KEY_F1 && ! rKey.GetModifier() )
            return sal_True;
    }
    return RoadmapWizard::PreNotify(rNEvt);        
}


void FirstStartWizard::enterState(WizardState _nState)
{
    RoadmapWizard::enterState(_nState);
    // default state
    // all on
    enableButtons(0xff, sal_True);
    // finish off
    enableButtons(WZB_FINISH, sal_False);
    // default text
    m_pCancel->SetText(m_sCancel);
    m_pCancel->SetClickHdl(m_lnkCancel);
    m_pNextPage->SetText(m_sNext);

    // default
    defaultButton(WZB_NEXT);

    // specialized state
    switch (_nState)
    {     
    case STATE_WELCOME:
        enableButtons(WZB_PREVIOUS, sal_False);
        break;
    case STATE_LICENSE:
        m_pCancel->SetText(String(WizardResId(STR_LICENSE_DECLINE)));
        m_pNextPage->SetText(String(WizardResId(STR_LICENSE_ACCEPT)));
        enableButtons(WZB_NEXT, sal_False);
        // attach warning dialog to cancel/decline button        
        m_pCancel->SetClickHdl( LINK(this, FirstStartWizard, DeclineHdl) );
        break;
    case STATE_REGISTRATION:
        enableButtons(WZB_NEXT, sal_False); 
        enableButtons(WZB_FINISH, sal_True);
        defaultButton(WZB_FINISH);
        break;
    }

    // focus
        
}

IMPL_LINK( FirstStartWizard, DeclineHdl, PushButton *, EMPTYARG )
{
    QueryBox aBox(this, WizardResId(QB_ASK_DECLINE));
    sal_Int32 ret = aBox.Execute();
    if ( ret == BUTTON_OK || ret == BUTTON_YES)
    {
        Close();
        return sal_False;
    }
    else
        return sal_True;
}


TabPage* FirstStartWizard::createPage(WizardState _nState)
{
    TabPage *pTabPage = 0;
    switch (_nState)
    {
    case STATE_WELCOME:
        pTabPage = new WelcomePage(this, WizardResId(TP_WELCOME), m_bLicenseNeedsAcceptance);
        break;
    case STATE_MIGRATION:
        pTabPage = new MigrationPage(this, WizardResId(TP_MIGRATION), m_aThrobber);
        break;
    case STATE_USER:
        pTabPage = new UserPage(this, WizardResId(TP_USER));
        break;
    case STATE_UPDATE_CHECK:
        pTabPage = new UpdateCheckPage(this, WizardResId(TP_UPDATE_CHECK));
        break;
    }
    pTabPage->Show();

    return pTabPage;
}

String FirstStartWizard::getStateDisplayName( WizardState _nState ) const
{
    String sName;
    switch(_nState)
    {
    case STATE_WELCOME:
        sName = String(WizardResId(STR_STATE_WELCOME));
        break;
    case STATE_MIGRATION:
        sName = String(WizardResId(STR_STATE_MIGRATION));
        break;
    case STATE_USER:
        sName = String(WizardResId(STR_STATE_USER));
        break;
    case STATE_UPDATE_CHECK:
        sName = String(WizardResId(STR_STATE_UPDATE_CHECK));
        break;
    }
    return sName;
}

sal_Bool FirstStartWizard::prepareLeaveCurrentState( CommitPageReason _eReason )
{
    // the license acceptance is handled here, because it needs to change the state
    // of the roadmap wizard which the page implementation does not know.
    if (
		(_eReason              == eTravelForward) &&
		(getCurrentState()     == STATE_LICENSE ) &&
		(m_bLicenseWasAccepted == sal_False     )
	   )
    {
		if (Migration::checkMigration())
            enableState(FirstStartWizard::STATE_MIGRATION, sal_True);
        if ( showOnlineUpdatePage() )
            enableState(FirstStartWizard::STATE_UPDATE_CHECK, sal_True);
		enableState(FirstStartWizard::STATE_USER, sal_True);
        enableState(FirstStartWizard::STATE_REGISTRATION, sal_True);

        storeAcceptDate();
        m_bLicenseWasAccepted = sal_True;        
    }

    return svt::RoadmapWizard::prepareLeaveCurrentState(_eReason);
}

sal_Bool FirstStartWizard::leaveState(WizardState)
{    
    if (( getCurrentState() == STATE_MIGRATION ) && m_bLicenseWasAccepted )
    {
        // Store accept date and patch level now as it has been 
        // overwritten by the migration process!
        storeAcceptDate();
        setPatchLevel();
    }

    return sal_True;
}

sal_Bool FirstStartWizard::onFinish()
{
    return svt::RoadmapWizard::onFinish();
}

short FirstStartWizard::Execute()
{
    return svt::RoadmapWizard::Execute();
}

static OUString _makeDateTimeString (const DateTime& aDateTime, sal_Bool bUTC = sal_False)
{
    OStringBuffer aDateTimeString;
    aDateTimeString.append((sal_Int32)aDateTime.GetYear());
    aDateTimeString.append("-");
    if (aDateTime.GetMonth()<10) aDateTimeString.append("0");
    aDateTimeString.append((sal_Int32)aDateTime.GetMonth());
    aDateTimeString.append("-");
    if (aDateTime.GetDay()<10) aDateTimeString.append("0");
    aDateTimeString.append((sal_Int32)aDateTime.GetDay());
    aDateTimeString.append("T");
    if (aDateTime.GetHour()<10) aDateTimeString.append("0");
    aDateTimeString.append((sal_Int32)aDateTime.GetHour());
    aDateTimeString.append(":");
    if (aDateTime.GetMin()<10) aDateTimeString.append("0");
    aDateTimeString.append((sal_Int32)aDateTime.GetMin());
    aDateTimeString.append(":");
    if (aDateTime.GetSec()<10) aDateTimeString.append("0");
    aDateTimeString.append((sal_Int32)aDateTime.GetSec());
    if (bUTC) aDateTimeString.append("Z");

    return OStringToOUString(aDateTimeString.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US);
}

static OUString _getCurrentDateString()
{
    OUString aString;
    return _makeDateTimeString(DateTime());   
}


static const OUString sConfigSrvc( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationProvider" ) );
static const OUString sAccessSrvc( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationUpdateAccess" ) );
static const OUString sReadSrvc  ( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationAccess" ) );

void FirstStartWizard::storeAcceptDate()
{

    try {
        Reference < XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
        // get configuration provider
        Reference< XMultiServiceFactory > theConfigProvider = Reference< XMultiServiceFactory >(
        xFactory->createInstance(sConfigSrvc), UNO_QUERY_THROW);
        Sequence< Any > theArgs(1);
        NamedValue v(OUString::createFromAscii("NodePath"), 
            makeAny(OUString::createFromAscii("org.openoffice.Setup/Office")));
        theArgs[0] <<= v;
        Reference< XPropertySet > pset = Reference< XPropertySet >(
            theConfigProvider->createInstanceWithArguments(sAccessSrvc, theArgs), UNO_QUERY_THROW);
        Any result = pset->getPropertyValue(OUString::createFromAscii("LicenseAcceptDate"));

        OUString aAcceptDate = _getCurrentDateString();
        pset->setPropertyValue(OUString::createFromAscii("LicenseAcceptDate"), makeAny(aAcceptDate));
        Reference< XChangesBatch >(pset, UNO_QUERY_THROW)->commitChanges();

		// since the license is accepted the local user registry can be cleaned if required
		cleanOldOfficeRegKeys();
    } catch (const Exception&)
    {
    }

}

void FirstStartWizard::setPatchLevel()
{
    try {
        Reference < XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
        // get configuration provider
        Reference< XMultiServiceFactory > theConfigProvider = Reference< XMultiServiceFactory >(
        xFactory->createInstance(sConfigSrvc), UNO_QUERY_THROW);
        Sequence< Any > theArgs(1);
        NamedValue v(OUString::createFromAscii("NodePath"), 
            makeAny(OUString::createFromAscii("org.openoffice.Office.Common/Help/Registration")));
        theArgs[0] <<= v;
        Reference< XPropertySet > pset = Reference< XPropertySet >(
            theConfigProvider->createInstanceWithArguments(sAccessSrvc, theArgs), UNO_QUERY_THROW);
        Any result = pset->getPropertyValue(OUString::createFromAscii("ReminderDate"));

        OUString aPatchLevel( RTL_CONSTASCII_USTRINGPARAM( "Patch" ));
        aPatchLevel += OUString::valueOf( getBuildId(), 10 );
        pset->setPropertyValue(OUString::createFromAscii("ReminderDate"), makeAny(aPatchLevel));
        Reference< XChangesBatch >(pset, UNO_QUERY_THROW)->commitChanges();
    } catch (const Exception&)
    {
    }
}

#ifdef WNT
typedef int ( __stdcall * CleanCurUserRegProc ) ( wchar_t* );
#endif

void FirstStartWizard::cleanOldOfficeRegKeys()
{
#ifdef WNT
	// after the wizard is completed clean OOo1.1.x entries in the current user registry if required
	// issue i47658

    OUString aBaseLocationPath;
    OUString aSharedLocationPath;
	OUString aInstallMode;

	::utl::Bootstrap::PathStatus aBaseLocateResult = 
        ::utl::Bootstrap::locateBaseInstallation( aBaseLocationPath );
    ::utl::Bootstrap::PathStatus aSharedLocateResult = 
        ::utl::Bootstrap::locateSharedData( aSharedLocationPath );
    aInstallMode = ::utl::Bootstrap::getAllUsersValue( ::rtl::OUString() );

	// TODO: replace the checking for install mode
    if ( aBaseLocateResult == ::utl::Bootstrap::PATH_EXISTS && aSharedLocateResult == ::utl::Bootstrap::PATH_EXISTS
	  && aInstallMode.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "1" ) ) ) )
    {
		::rtl::OUString aDeregCompletePath =
					aBaseLocationPath + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/program/regcleanold.dll" ) );
		::rtl::OUString aExecCompletePath =
					aSharedLocationPath + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/regdeinstall/userdeinst.exe" ) );

		osl::Module aCleanModule( aDeregCompletePath );
		CleanCurUserRegProc pNativeProc = ( CleanCurUserRegProc )(
					aCleanModule.getFunctionSymbol(
						::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CleanCurUserOldSystemRegistry" ) ) ) );

		if( pNativeProc!=NULL )
		{
			::rtl::OUString aExecCompleteSysPath;
			if ( osl::File::getSystemPathFromFileURL( aExecCompletePath, aExecCompleteSysPath ) == FileBase::E_None
			  && aExecCompleteSysPath.getLength() )
			{
				( *pNativeProc )( (wchar_t*)( aExecCompleteSysPath.getStr() ) );
			}
		}
	}
#endif
}

sal_Bool FirstStartWizard::showOnlineUpdatePage()
{
    try {
        Reference < XNameReplace > xUpdateAccess;
        Reference < XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );

        xUpdateAccess = Reference < XNameReplace >(
            xFactory->createInstance( UNISTRING( "com.sun.star.setup.UpdateCheckConfig" ) ), UNO_QUERY_THROW );   

        if ( xUpdateAccess.is() )
        {
            sal_Bool bAutoUpdChk = sal_False;
            Any result = xUpdateAccess->getByName( UNISTRING( "AutoCheckEnabled" ) );
            result >>= bAutoUpdChk;
            if ( bAutoUpdChk == sal_False )
                return sal_True;
            else
                return sal_False;
        }
    } catch (const Exception&)
    {
    }
    return sal_False;
}

}
