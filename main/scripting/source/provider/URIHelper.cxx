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
#include "precompiled_scripting.hxx"

#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#include <rtl/ustrbuf.hxx>
#include "URIHelper.hxx"

#define PRTSTR(x) ::rtl::OUStringToOString(x, RTL_TEXTENCODING_ASCII_US).pData->buffer

namespace func_provider
{

using ::rtl::OUString;
namespace uno = ::com::sun::star::uno;
namespace ucb = ::com::sun::star::ucb;
namespace lang = ::com::sun::star::lang;
namespace uri = ::com::sun::star::uri;
namespace script = ::com::sun::star::script;
 
static const char SHARE[] = "share";
static const char SHARE_URI[] =
    "vnd.sun.star.expand:${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE( "bootstrap") "::BaseInstallation}";

static const char SHARE_UNO_PACKAGES[] = "share:uno_packages";
static const char SHARE_UNO_PACKAGES_URI[] = 
    "vnd.sun.star.expand:$UNO_SHARED_PACKAGES_CACHE";

static const char USER[] = "user";
static const char USER_URI[] =
    "vnd.sun.star.expand:${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}";

static const char USER_UNO_PACKAGES[] = "user:uno_packages";
static const char USER_UNO_PACKAGES_DIR[] =
    "/user/uno_packages/cache";

static const char DOCUMENT[] = "document";
static const char TDOC_SCHEME[] = "vnd.sun.star.tdoc";

ScriptingFrameworkURIHelper::ScriptingFrameworkURIHelper(
    const uno::Reference< uno::XComponentContext >& xContext)
        throw( uno::RuntimeException )
{
    try
    {
        m_xSimpleFileAccess = uno::Reference< ucb::XSimpleFileAccess >(
            xContext->getServiceManager()->createInstanceWithContext(
                OUString::createFromAscii(
                    "com.sun.star.ucb.SimpleFileAccess"),
                xContext), uno::UNO_QUERY_THROW);
    }
    catch (uno::Exception&)
    {
        OSL_ENSURE(false,
            "Scripting Framework error initialising XSimpleFileAccess");
    }

    try
    {
        m_xUriReferenceFactory = uno::Reference< uri::XUriReferenceFactory >(
            xContext->getServiceManager()->createInstanceWithContext(
                OUString::createFromAscii(
                    "com.sun.star.uri.UriReferenceFactory"),
            xContext ), uno::UNO_QUERY_THROW );
    }
    catch (uno::Exception&)
    {
        OSL_ENSURE(false,
            "Scripting Framework error initialising XUriReferenceFactory");
    }
}

ScriptingFrameworkURIHelper::~ScriptingFrameworkURIHelper()
{
    // currently does nothing
}

void SAL_CALL
ScriptingFrameworkURIHelper::initialize(
    const uno::Sequence < uno::Any >& args )
throw ( uno::Exception, uno::RuntimeException )
{
    if ( args.getLength() != 2 ||
         args[0].getValueType() != ::getCppuType((const OUString*)NULL) || 
         args[1].getValueType() != ::getCppuType((const OUString*)NULL) )
    {
        throw uno::RuntimeException( OUString::createFromAscii(
            "ScriptingFrameworkURIHelper got invalid argument list" ),
                uno::Reference< uno::XInterface >() );
    }

    if ( (args[0] >>= m_sLanguage) == sal_False ||
         (args[1] >>= m_sLocation) == sal_False )
    {
        throw uno::RuntimeException( OUString::createFromAscii(
            "ScriptingFrameworkURIHelper error parsing args" ),
                uno::Reference< uno::XInterface >() );
    }

    SCRIPTS_PART = OUString::createFromAscii( "/Scripts/" );
    SCRIPTS_PART = SCRIPTS_PART.concat( m_sLanguage.toAsciiLowerCase() );

    if ( !initBaseURI() )
    {
        throw uno::RuntimeException( OUString::createFromAscii(
            "ScriptingFrameworkURIHelper cannot find script directory"),
                uno::Reference< uno::XInterface >() );
    }
}

bool
ScriptingFrameworkURIHelper::initBaseURI()
{
    OUString uri, test;
    bool bAppendScriptsPart = false;

    if ( m_sLocation.equalsAscii(USER))
    {
        test = OUString::createFromAscii(USER);
        uri = OUString::createFromAscii(USER_URI);
        bAppendScriptsPart = true;
    }
    else if ( m_sLocation.equalsAscii(USER_UNO_PACKAGES))
    {
        test = OUString::createFromAscii("uno_packages");
        uri = OUString::createFromAscii(USER_URI);
        uri = uri.concat(OUString::createFromAscii(USER_UNO_PACKAGES_DIR));
    }
    else if (m_sLocation.equalsAscii(SHARE))
    {
        test = OUString::createFromAscii(SHARE);
        uri = OUString::createFromAscii(SHARE_URI);
        bAppendScriptsPart = true;
    }
    else if (m_sLocation.equalsAscii(SHARE_UNO_PACKAGES))
    {
        test = OUString::createFromAscii("uno_packages");
        uri = OUString::createFromAscii(SHARE_UNO_PACKAGES_URI);
    }
    else if (m_sLocation.indexOf(OUString::createFromAscii(TDOC_SCHEME)) == 0)
    {
        m_sBaseURI = m_sLocation.concat( SCRIPTS_PART );
        m_sLocation = OUString::createFromAscii( DOCUMENT );
        return true;
    }
    else
    {
        return false;
    }

    if ( !m_xSimpleFileAccess->exists( uri ) ||
         !m_xSimpleFileAccess->isFolder( uri ) )
    {
        return false;
    }

    uno::Sequence< OUString > children =
        m_xSimpleFileAccess->getFolderContents( uri, true );

    for ( sal_Int32 i = 0; i < children.getLength(); i++ )
    {
        OUString child = children[i];
        sal_Int32 idx = child.lastIndexOf(test);

        // OSL_TRACE("Trying: %s", PRTSTR(child));
        // OSL_TRACE("idx=%d, testlen=%d, children=%d",
        //     idx, test.getLength(), child.getLength());

        if ( idx != -1 && (idx + test.getLength()) == child.getLength() )
        {
            // OSL_TRACE("FOUND PATH: %s", PRTSTR(child));
            if ( bAppendScriptsPart )
            {
                m_sBaseURI = child.concat( SCRIPTS_PART );
            }
            else
            {
                m_sBaseURI = child;
            }
            return true;
        }
    }
    return false;
}

OUString
ScriptingFrameworkURIHelper::getLanguagePart(const OUString& rStorageURI)
{
    OUString result;

    sal_Int32 idx = rStorageURI.indexOf(m_sBaseURI);
    sal_Int32 len = m_sBaseURI.getLength() + 1;

    if ( idx != -1 )
    {
        result = rStorageURI.copy(idx + len);
        result = result.replace('/', '|');
    }
    return result;
}

OUString
ScriptingFrameworkURIHelper::getLanguagePath(const OUString& rLanguagePart)
{
    OUString result;
    result = rLanguagePart.replace('|', '/');
    return result;
}

OUString SAL_CALL
ScriptingFrameworkURIHelper::getScriptURI(const OUString& rStorageURI)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    ::rtl::OUStringBuffer buf(120);

    buf.appendAscii("vnd.sun.star.script:");
    buf.append(getLanguagePart(rStorageURI));
    buf.appendAscii("?language=");
    buf.append(m_sLanguage);
    buf.appendAscii("&location=");
    buf.append(m_sLocation);

    return buf.makeStringAndClear();
}

OUString SAL_CALL
ScriptingFrameworkURIHelper::getStorageURI(const OUString& rScriptURI)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    OUString sLanguagePart;
    try
    {
        uno::Reference < uri::XVndSunStarScriptUrl > xURI(
            m_xUriReferenceFactory->parse( rScriptURI ), uno::UNO_QUERY_THROW );
        sLanguagePart = xURI->getName();
    }
    catch ( uno::Exception& )
    {
        throw lang::IllegalArgumentException(
            OUString::createFromAscii( "Script URI not valid" ),
                uno::Reference< uno::XInterface >(), 1 );
    }

    ::rtl::OUStringBuffer buf(120);
    buf.append(m_sBaseURI);
    buf.append(OUString::createFromAscii("/"));
    buf.append(getLanguagePath(sLanguagePart));

    OUString result = buf.makeStringAndClear();

    return result;
}

OUString SAL_CALL
ScriptingFrameworkURIHelper::getRootStorageURI()
    throw( uno::RuntimeException )
{
    return m_sBaseURI;
}

OUString SAL_CALL
ScriptingFrameworkURIHelper::getImplementationName()
    throw( uno::RuntimeException )
{
    return OUString::createFromAscii(
        "com.sun.star.script.provider.ScriptURIHelper" );
}

sal_Bool SAL_CALL
ScriptingFrameworkURIHelper::supportsService( const OUString& serviceName )
    throw( uno::RuntimeException )
{
    OUString m_sServiceName = OUString::createFromAscii(
        "com.sun.star.script.provider.ScriptURIHelper" );

    if ( serviceName.equals( m_sServiceName ) )
    {
        return sal_True;
    }
    return sal_False;
}

uno::Sequence< ::rtl::OUString > SAL_CALL
ScriptingFrameworkURIHelper::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    ::rtl::OUString serviceNameList[] = { 
        ::rtl::OUString::createFromAscii(
            "com.sun.star.script.provider.ScriptURIHelper" ) };

    uno::Sequence< ::rtl::OUString > serviceNames = uno::Sequence <
        ::rtl::OUString > ( serviceNameList, 1 );

    return serviceNames;
}
}
