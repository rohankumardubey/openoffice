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



#include <bf_svtools/useroptions.hxx>

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX
#include <bf_svtools/smplhint.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif
#include <rtl/logfile.hxx>
#include "itemholder2.hxx"

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

namespace binfilter
{

// class SvtUserOptions_Impl ---------------------------------------------

class SvtUserOptions_Impl : public utl::ConfigItem, public SfxBroadcaster
{
private:
    String          m_aCompany;
	String			m_aFirstName;
	String			m_aLastName;
	String			m_aID;
	String			m_aStreet;
	String			m_aCity;
	String			m_aState;
	String			m_aZip;
	String			m_aCountry;
	String			m_aPosition;
	String			m_aTitle;
	String			m_aTelephoneHome;
	String			m_aTelephoneWork;
	String			m_aFax;
	String			m_aEmail;
	String			m_aCustomerNumber;
    String          m_aFathersName;
    String          m_aApartment;

	String			m_aEmptyString;
	String			m_aFullName;
	String			m_aLocale;

	sal_Bool		m_bIsROCompany;
	sal_Bool		m_bIsROFirstName;
	sal_Bool		m_bIsROLastName;
	sal_Bool		m_bIsROID;
	sal_Bool		m_bIsROStreet;
	sal_Bool		m_bIsROCity;
	sal_Bool		m_bIsROState;
	sal_Bool		m_bIsROZip;
	sal_Bool		m_bIsROCountry;
	sal_Bool		m_bIsROPosition;
	sal_Bool		m_bIsROTitle;
	sal_Bool		m_bIsROTelephoneHome;
	sal_Bool		m_bIsROTelephoneWork;
	sal_Bool		m_bIsROFax;
	sal_Bool		m_bIsROEmail;
	sal_Bool		m_bIsROCustomerNumber;
    sal_Bool        m_bIsROFathersName;
    sal_Bool        m_bIsROApartment;

	typedef String SvtUserOptions_Impl:: *StrPtr;

	void			InitFullName();
    void            Load();

    static void     InitUserPropertyNames();
public:
	SvtUserOptions_Impl();
    ~SvtUserOptions_Impl();

	virtual void 	Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
	virtual void	Commit();

	// get the user token
    const String&   GetCompany() const { return m_aCompany; }
    const String&   GetFirstName() const { return m_aFirstName; }
    const String&   GetLastName() const { return m_aLastName; }
    const String&   GetID() const { return m_aID; }
    const String&   GetStreet() const { return m_aStreet; }
    const String&   GetCity() const { return m_aCity; }
    const String&   GetState() const { return m_aState; }
    const String&   GetZip() const { return m_aZip; }
    const String&   GetCountry() const { return m_aCountry; }
    const String&   GetPosition() const { return m_aPosition; }
    const String&   GetTitle() const { return m_aTitle; }
    const String&   GetTelephoneHome() const { return m_aTelephoneHome; }
    const String&   GetTelephoneWork() const { return m_aTelephoneWork; }
    const String&   GetFax() const { return m_aFax; }
    const String&   GetEmail() const { return m_aEmail; }
    const String&   GetCustomerNumber() const { return m_aCustomerNumber; }
    const String&   GetFathersName() const { return m_aFathersName; }
    const String&   GetApartment() const { return m_aApartment; }

    const String&   GetFullName();
    const String&   GetLocale() const { return m_aLocale; }
};

// global ----------------------------------------------------------------

static SvtUserOptions_Impl*	pOptions = NULL;
static sal_Int32			nRefCount = 0;

#define READONLY_DEFAULT	sal_False

// functions -------------------------------------------------------------

namespace
{
    struct PropertyNames
        : public rtl::Static< Sequence< rtl::OUString >, PropertyNames> {};
}

// -----------------------------------------------------------------------

void SvtUserOptions_Impl::InitUserPropertyNames()
{
	static const char* aPropNames[] =
	{
		"Data/l",						// USER_OPT_CITY
		"Data/o",						// USER_OPT_COMPANY
		"Data/c",						// USER_OPT_COUNTRY
		"Data/mail",					// USER_OPT_EMAIL
		"Data/facsimiletelephonenumber",// USER_OPT_FAX
		"Data/givenname",				// USER_OPT_FIRSTNAME
		"Data/sn",						// USER_OPT_LASTNAME
		"Data/position",				// USER_OPT_POSITION
		"Data/st",						// USER_OPT_STATE
		"Data/street",					// USER_OPT_STREET
		"Data/homephone",				// USER_OPT_TELEPHONEHOME
		"Data/telephonenumber",			// USER_OPT_TELEPHONEWORK
		"Data/title",					// USER_OPT_TITLE
		"Data/initials",				// USER_OPT_ID
        "Data/postalcode",              // USER_OPT_ZIP
        "Data/fathersname",             // USER_OPT_FATHERSNAME
        "Data/apartment"                // USER_OPT_APARTMENT
	};
	const int nCount = sizeof( aPropNames ) / sizeof( const char* );
    Sequence< rtl::OUString > &rPropertyNames = PropertyNames::get();
    rPropertyNames.realloc(nCount);
    OUString* pNames = rPropertyNames.getArray();
	for ( int i = 0; i < nCount; i++ )
		pNames[i] = OUString::createFromAscii( aPropNames[i] );
}

// -----------------------------------------------------------------------

void SvtUserOptions_Impl::InitFullName()
{
	m_aFullName = GetFirstName();
	m_aFullName.EraseLeadingAndTrailingChars();
	if ( m_aFullName.Len() )
		m_aFullName += ' ';
	m_aFullName += GetLastName();
	m_aFullName.EraseTrailingChars();
}

// -----------------------------------------------------------------------
SvtUserOptions_Impl::SvtUserOptions_Impl() :

	ConfigItem( OUString::createFromAscii("UserProfile") ),

	m_bIsROCompany( READONLY_DEFAULT ),
	m_bIsROFirstName( READONLY_DEFAULT ),
	m_bIsROLastName( READONLY_DEFAULT ),
	m_bIsROID( READONLY_DEFAULT ),
	m_bIsROStreet( READONLY_DEFAULT ),
	m_bIsROCity( READONLY_DEFAULT ),
	m_bIsROState( READONLY_DEFAULT ),
	m_bIsROZip( READONLY_DEFAULT ),
	m_bIsROCountry( READONLY_DEFAULT ),
	m_bIsROPosition( READONLY_DEFAULT ),
	m_bIsROTitle( READONLY_DEFAULT ),
	m_bIsROTelephoneHome( READONLY_DEFAULT ),
	m_bIsROTelephoneWork( READONLY_DEFAULT ),
	m_bIsROFax( READONLY_DEFAULT ),
	m_bIsROEmail( READONLY_DEFAULT ),
    m_bIsROCustomerNumber( READONLY_DEFAULT ),
    m_bIsROFathersName( READONLY_DEFAULT ),
    m_bIsROApartment( READONLY_DEFAULT )
{
    InitUserPropertyNames();
    EnableNotification( PropertyNames::get() );
    Load();
    Any aAny = ConfigManager::GetConfigManager()->GetDirectConfigProperty( ConfigManager::LOCALE );
	OUString aLocale;
	if ( aAny >>= aLocale )
		m_aLocale = String( aLocale );
	else
	{
		DBG_ERRORFILE( "SvtUserOptions_Impl::SvtUserOptions_Impl(): no locale found" );
	}
}
// -----------------------------------------------------------------------

SvtUserOptions_Impl::~SvtUserOptions_Impl()
{
}

// -----------------------------------------------------------------------
void SvtUserOptions_Impl::Load()
{
    Sequence< rtl::OUString > &rPropertyNames = PropertyNames::get();
    Sequence< Any > seqValues = GetProperties( rPropertyNames );
    Sequence< sal_Bool > seqRO = GetReadOnlyStates( rPropertyNames );
	const Any* pValues = seqValues.getConstArray();
    DBG_ASSERT( seqValues.getLength() == rPropertyNames.getLength(), "GetProperties failed" );
    if ( seqValues.getLength() == rPropertyNames.getLength() )
	{
		OUString aTempStr;

        for ( int nProp = 0; nProp < rPropertyNames.getLength(); nProp++ )
		{
			if ( pValues[nProp].hasValue() )
			{
				if ( pValues[nProp] >>= aTempStr )
				{
					String* pToken = NULL;
					sal_Bool* pBool = NULL;

					switch ( nProp )
					{
						case USER_OPT_COMPANY:
							pToken = &m_aCompany; pBool = &m_bIsROCompany; break;
						case USER_OPT_FIRSTNAME:
							pToken = &m_aFirstName; pBool = &m_bIsROFirstName; break;
						case USER_OPT_LASTNAME:
							pToken = &m_aLastName; pBool = &m_bIsROLastName; break;
						case USER_OPT_ID:
							pToken = &m_aID; pBool = &m_bIsROID; break;
						case USER_OPT_STREET:
							pToken = &m_aStreet; pBool = &m_bIsROStreet; break;
						case USER_OPT_CITY:
							pToken = &m_aCity; pBool = &m_bIsROCity; break;
						case USER_OPT_STATE:
							pToken = &m_aState; pBool = &m_bIsROState; break;
						case USER_OPT_ZIP:
							pToken = &m_aZip; pBool = &m_bIsROZip; break;
						case USER_OPT_COUNTRY:
							pToken = &m_aCountry; pBool = &m_bIsROCountry; break;
						case USER_OPT_POSITION:
							pToken = &m_aPosition; pBool = &m_bIsROPosition; break;
						case USER_OPT_TITLE:
							pToken = &m_aTitle; pBool = &m_bIsROTitle; break;
						case USER_OPT_TELEPHONEHOME:
							pToken = &m_aTelephoneHome; pBool = &m_bIsROTelephoneHome; break;
						case USER_OPT_TELEPHONEWORK:
							pToken = &m_aTelephoneWork; pBool = &m_bIsROTelephoneWork; break;
						case USER_OPT_FAX:
							pToken = &m_aFax; pBool = &m_bIsROFax; break;
						case USER_OPT_EMAIL:
							pToken = &m_aEmail; pBool = &m_bIsROEmail; break;
                        case USER_OPT_FATHERSNAME:
                            pToken = &m_aFathersName; pBool = &m_bIsROFathersName;  break;
                        case USER_OPT_APARTMENT:
                            pToken = &m_aApartment; pBool = &m_bIsROApartment;    break;
                        default:
							DBG_ERRORFILE( "invalid index to load a user token" );
					}

					if ( pToken )
						*pToken = String( aTempStr );
					if ( pBool )
						*pBool = seqRO[nProp];
				}
				else
				{
					DBG_ERRORFILE( "Wrong any type" );
				}
			}
		}
	}
    InitFullName();
}
// -----------------------------------------------------------------------

void SvtUserOptions_Impl::Commit()
{
    Sequence< rtl::OUString > &rPropertyNames = PropertyNames::get();
    sal_Int32 nOrgCount = rPropertyNames.getLength();

    Sequence< OUString > seqNames( nOrgCount );
    Sequence< Any > seqValues( nOrgCount );
    sal_Int32 nRealCount = 0;

	OUString aTempStr;

	for ( int nProp = 0; nProp < nOrgCount; nProp++ )
	{
		sal_Bool* pbReadonly = NULL;

		switch ( nProp )
		{
			case USER_OPT_COMPANY:
				aTempStr = OUString( m_aCompany ); pbReadonly = &m_bIsROCompany; break;
			case USER_OPT_FIRSTNAME:
				aTempStr = OUString( m_aFirstName ); pbReadonly = &m_bIsROFirstName; break;
			case USER_OPT_LASTNAME:
				aTempStr = OUString( m_aLastName );	pbReadonly = &m_bIsROLastName; break;
			case USER_OPT_ID:
				aTempStr = OUString( m_aID ); pbReadonly = &m_bIsROID; break;
			case USER_OPT_STREET:
				aTempStr = OUString( m_aStreet ); pbReadonly = &m_bIsROStreet; break;
			case USER_OPT_CITY:
				aTempStr = OUString( m_aCity );	pbReadonly = &m_bIsROCity; break;
			case USER_OPT_STATE:
				aTempStr = OUString( m_aState ); pbReadonly = &m_bIsROState; break;
			case USER_OPT_ZIP:
				aTempStr = OUString( m_aZip ); pbReadonly = &m_bIsROZip; break;
			case USER_OPT_COUNTRY:
				aTempStr = OUString( m_aCountry ); pbReadonly = &m_bIsROCountry; break;
			case USER_OPT_POSITION:
				aTempStr = OUString( m_aPosition );	pbReadonly = &m_bIsROPosition; break;
			case USER_OPT_TITLE:
				aTempStr = OUString( m_aTitle ); pbReadonly = &m_bIsROTitle; break;
			case USER_OPT_TELEPHONEHOME:
				aTempStr = OUString( m_aTelephoneHome ); pbReadonly = &m_bIsROTelephoneHome; break;
			case USER_OPT_TELEPHONEWORK:
				aTempStr = OUString( m_aTelephoneWork ); pbReadonly = &m_bIsROTelephoneWork; break;
			case USER_OPT_FAX:
				aTempStr = OUString( m_aFax ); pbReadonly = &m_bIsROFax; break;
			case USER_OPT_EMAIL:
				aTempStr = OUString( m_aEmail ); pbReadonly = &m_bIsROEmail; break;
            case USER_OPT_FATHERSNAME:
                aTempStr = OUString( m_aFathersName ); pbReadonly = &m_bIsROFathersName;  break;
            case USER_OPT_APARTMENT:
                aTempStr = OUString( m_aApartment ); pbReadonly = &m_bIsROApartment;    break;
            default:
				DBG_ERRORFILE( "invalid index to save a user token" );
		}

		if ( pbReadonly && !(*pbReadonly) )
		{
            seqValues[nRealCount] <<= aTempStr;
            seqNames[nRealCount] = rPropertyNames[nProp];
            ++nRealCount;
		}
	}

	// Set properties in configuration.
    seqNames.realloc( nRealCount );
    seqValues.realloc( nRealCount );
    PutProperties( seqNames, seqValues );
    //broadcast changes
    Broadcast(SfxSimpleHint(SFX_HINT_USER_OPTIONS_CHANGED));
}

// -----------------------------------------------------------------------

const String& SvtUserOptions_Impl::GetFullName()
{
	if ( IsModified() )
		InitFullName();
	return m_aFullName;
}

// -----------------------------------------------------------------------

void SvtUserOptions_Impl::Notify( const Sequence<rtl::OUString>& )
{
    Load();
    Broadcast(SfxSimpleHint(SFX_HINT_USER_OPTIONS_CHANGED));
}

// class SvtUserOptions --------------------------------------------------

SvtUserOptions::SvtUserOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( GetInitMutex() );

	if ( !pOptions )
    {
        RTL_LOGFILE_CONTEXT(aLog, "svtools ( ??? ) ::SvtUserOptions_Impl::ctor()");
        pOptions = new SvtUserOptions_Impl;

        ItemHolder2::holdConfigItem(E_USEROPTIONS);
    }
    ++nRefCount;
    pImp = pOptions;
    StartListening( *pImp);
}

// -----------------------------------------------------------------------

SvtUserOptions::~SvtUserOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( GetInitMutex() );

	if ( !--nRefCount )
	{
		if ( pOptions->IsModified() )
			pOptions->Commit();
        DELETEZ( pOptions );
	}
}

// -----------------------------------------------------------------------

::osl::Mutex& SvtUserOptions::GetInitMutex()
{
	// Initialize static mutex only for one time!
    static ::osl::Mutex* pMutex = NULL;
	// If these method first called (Mutex not already exist!) ...
    if ( pMutex == NULL )
    {
		// ... we must create a new one. Protect follow code with the global mutex -
		// It must be - we create a static variable!
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
		// We must check our pointer again -
		// because another instance of our class will be faster then this instance!
        if ( pMutex == NULL )
        {
			// Create the new mutex and set it for return on static variable.
            static ::osl::Mutex aMutex;
            pMutex = &aMutex;
        }
    }
	// Return new created or already existing mutex object.
    return *pMutex;
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetCompany() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
	return pImp->GetCompany();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetFirstName() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
	return pImp->GetFirstName();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetLastName() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
	return pImp->GetLastName();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetID() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
	return pImp->GetID();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetStreet() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
	return pImp->GetStreet();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetCity() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
	return pImp->GetCity();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetState() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
	return pImp->GetState();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetZip() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
	return pImp->GetZip();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetCountry() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
	return pImp->GetCountry();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetPosition() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
	return pImp->GetPosition();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetTitle() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
	return pImp->GetTitle();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetTelephoneHome() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
	return pImp->GetTelephoneHome();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetTelephoneWork() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
	return pImp->GetTelephoneWork();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetFax() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
	return pImp->GetFax();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetEmail() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
	return pImp->GetEmail();
}

// -----------------------------------------------------------------------

const String& SvtUserOptions::GetFullName() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
	return pImp->GetFullName();
}

/* -----------------07.07.2003 09:30-----------------

 --------------------------------------------------*/
void SvtUserOptions::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    vos::OGuard aVclGuard( Application::GetSolarMutex() );
    Broadcast( rHint );
}

}
