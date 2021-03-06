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



#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include "filtuno.hxx"
#include "miscuno.hxx"
#include "unoguard.hxx"
#include "imoptdlg.hxx"
#include "docsh.hxx"
#include "globstr.hrc"
namespace binfilter {

using namespace ::com::sun::star;

//------------------------------------------------------------------------

#define SCFILTEROPTIONSOBJ_SERVICE		"com.sun.star.ui.dialogs.FilterOptionsDialog"
#define SCFILTEROPTIONSOBJ_IMPLNAME		"com.sun.star.comp.Calc.FilterOptionsDialog"

SC_SIMPLE_SERVICE_INFO( ScFilterOptionsObj, SCFILTEROPTIONSOBJ_IMPLNAME, SCFILTEROPTIONSOBJ_SERVICE )

#define SC_UNONAME_FILENAME         "URL"
#define SC_UNONAME_FILTERNAME		"FilterName"
#define SC_UNONAME_FILTEROPTIONS	"FilterOptions"
#define SC_UNONAME_INPUTSTREAM		"InputStream"

//------------------------------------------------------------------------

ScFilterOptionsObj::ScFilterOptionsObj() :
	bExport( sal_False )
{
}

ScFilterOptionsObj::~ScFilterOptionsObj()
{
}

// stuff for exService_...

uno::Reference<uno::XInterface>	SAL_CALL ScFilterOptionsObj_CreateInstance(
						const uno::Reference<lang::XMultiServiceFactory>& )
{
	ScUnoGuard aGuard;
	SC_DLL()->Load();		// load module

	return (::cppu::OWeakObject*) new ScFilterOptionsObj;
}

::rtl::OUString ScFilterOptionsObj::getImplementationName_Static()
{
	return ::rtl::OUString::createFromAscii( SCFILTEROPTIONSOBJ_IMPLNAME );
}

uno::Sequence< ::rtl::OUString> ScFilterOptionsObj::getSupportedServiceNames_Static()
{
	uno::Sequence< ::rtl::OUString> aRet(1);
	::rtl::OUString* pArray = aRet.getArray();
	pArray[0] = ::rtl::OUString::createFromAscii( SCFILTEROPTIONSOBJ_SERVICE );
	return aRet;
}

// XPropertyAccess

uno::Sequence<beans::PropertyValue> SAL_CALL ScFilterOptionsObj::getPropertyValues() throw(uno::RuntimeException)
{
	uno::Sequence<beans::PropertyValue> aRet(1);
	beans::PropertyValue* pArray = aRet.getArray();

	pArray[0].Name = ::rtl::OUString::createFromAscii( SC_UNONAME_FILTEROPTIONS );
	pArray[0].Value <<= aFilterOptions;

	return aRet;
}

void SAL_CALL ScFilterOptionsObj::setPropertyValues( const uno::Sequence<beans::PropertyValue>& aProps )
					throw(beans::UnknownPropertyException, beans::PropertyVetoException,
							lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
	const beans::PropertyValue* pPropArray = aProps.getConstArray();
	long nPropCount = aProps.getLength();
	for (long i = 0; i < nPropCount; i++)
	{
		const beans::PropertyValue& rProp = pPropArray[i];
		String aPropName = rProp.Name;

		if ( aPropName.EqualsAscii( SC_UNONAME_FILENAME ) )
			rProp.Value >>= aFileName;
		else if ( aPropName.EqualsAscii( SC_UNONAME_FILTERNAME ) )
			rProp.Value >>= aFilterName;
		else if ( aPropName.EqualsAscii( SC_UNONAME_FILTEROPTIONS ) )
			rProp.Value >>= aFilterOptions;
		else if ( aPropName.EqualsAscii( SC_UNONAME_INPUTSTREAM ) )
			rProp.Value >>= xInputStream;
	}
}

// XExecutableDialog

void SAL_CALL ScFilterOptionsObj::setTitle( const ::rtl::OUString& aTitle ) throw(uno::RuntimeException)
{
	// not used
}

sal_Int16 SAL_CALL ScFilterOptionsObj::execute() throw(uno::RuntimeException)
{
	sal_Int16 nRet = ui::dialogs::ExecutableDialogResults::CANCEL;

	String aFilterString( aFilterName );
	if ( !bExport && aFilterString == ScDocShell::GetAsciiFilterName() )
	{
		//	ascii import is special...

		INetURLObject aURL( aFileName );
		String aExt = aURL.getExtension();
		String aPrivDatName = aURL.getName();
		sal_Unicode cAsciiDel;
		if (aExt.EqualsIgnoreCaseAscii("CSV"))
			cAsciiDel = ',';
		else
			cAsciiDel = '\t';

		SvStream* pInStream = NULL;
		if ( xInputStream.is() )
			pInStream = ::utl::UcbStreamHelper::CreateStream( xInputStream );

		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ScImportAsciiDlg* pDlg = new ScImportAsciiDlg( NULL, aPrivDatName, pInStream, cAsciiDel );
		delete pInStream;
	}
	else
	{
		sal_Bool bMultiByte = sal_True;
		sal_Bool bDBEnc     = sal_False;
		sal_Bool bAscii     = sal_False;

		sal_Unicode cStrDel = '"';
		sal_Unicode cAsciiDel = ';';
		rtl_TextEncoding eEncoding = RTL_TEXTENCODING_DONTKNOW;

		String aTitle;

		if ( aFilterString == ScDocShell::GetAsciiFilterName() )
		{
			//	ascii export (import is handled above)

			INetURLObject aURL( aFileName );
			String aExt = aURL.getExtension();
			if (aExt.EqualsIgnoreCaseAscii("CSV"))
				cAsciiDel = ',';
			else
				cAsciiDel = '\t';

			aTitle = ScGlobal::GetRscString( STR_EXPORT_ASCII );
			bAscii = sal_True;
		}
		else if ( aFilterString == ScDocShell::GetLotusFilterName() )
		{
			//	lotus is only imported
			DBG_ASSERT( !bExport, "Filter Options for Lotus Export is not implemented" );

			aTitle = ScGlobal::GetRscString( STR_IMPORT_LOTUS );
			eEncoding = RTL_TEXTENCODING_IBM_437;
		}
		else if ( aFilterString == ScDocShell::GetDBaseFilterName() )
		{
			if ( bExport )
			{
				//	dBase export
				aTitle = ScGlobal::GetRscString( STR_EXPORT_DBF );
			}
			else
			{
				//	dBase import
				aTitle = ScGlobal::GetRscString( STR_IMPORT_DBF );
			}
			// common for dBase import/export
			eEncoding = RTL_TEXTENCODING_IBM_850;
			bMultiByte = sal_False;
			bDBEnc = sal_True;
		}
		else if ( aFilterString == ScDocShell::GetDifFilterName() )
		{
			if ( bExport )
			{
				//	DIF export
				aTitle = ScGlobal::GetRscString( STR_EXPORT_DIF );
			}
			else
			{
				//	DIF import
				aTitle = ScGlobal::GetRscString( STR_IMPORT_DIF );
			}
			// common for DIF import/export
			eEncoding = RTL_TEXTENCODING_MS_1252;
		}

		ScImportOptions aOptions( cAsciiDel, cStrDel, eEncoding);
		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ScImportOptionsDlg* pDlg = new ScImportOptionsDlg( NULL, bAscii,
	}

	xInputStream.clear();	// don't hold the stream longer than necessary

	return nRet;
}

// XImporter

void SAL_CALL ScFilterOptionsObj::setTargetDocument( const uno::Reference<lang::XComponent>& xDoc )
							throw(lang::IllegalArgumentException, uno::RuntimeException)
{
	bExport = sal_False;
}

// XExporter

void SAL_CALL ScFilterOptionsObj::setSourceDocument( const uno::Reference<lang::XComponent>& xDoc )
							throw(lang::IllegalArgumentException, uno::RuntimeException)
{
	bExport = sal_True;
}

}
