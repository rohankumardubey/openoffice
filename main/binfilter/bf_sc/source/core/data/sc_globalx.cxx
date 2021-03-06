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




#include "callform.hxx"
#include "global.hxx"

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX 
#include <unotools/localfilehelper.hxx>
#endif

#include <tools/debug.hxx>
#include <bf_svtools/pathoptions.hxx>

#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>

#ifdef _MSC_VER
#pragma hdrstop
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
namespace binfilter {


// static
/*N*/ void ScGlobal::InitAddIns()
/*N*/ {
/*N*/ 	// multi paths separated by semicolons
/*N*/ 	SvtPathOptions aPathOpt;
/*N*/ 	String aMultiPath = aPathOpt.GetAddinPath();
/*N*/ 	if ( aMultiPath.Len() > 0 )
/*N*/ 	{
/*N*/ 		xub_StrLen nTokens = aMultiPath.GetTokenCount( ';' );
/*N*/ 		xub_StrLen nIndex = 0;
/*N*/ 		for ( xub_StrLen j=0; j<nTokens; j++ )
/*N*/ 		{
/*N*/ 			String aPath( aMultiPath.GetToken( 0, ';', nIndex ) );
/*N*/ 			if ( aPath.Len() > 0 )
/*N*/ 			{
/*N*/ 				//	use LocalFileHelper to convert the path to a URL that always points
/*N*/ 				//	to the file on the server
/*N*/ 				String aUrl;
/*N*/ 				if ( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aPath, aUrl ) )
/*N*/ 					aPath = aUrl;
/*N*/ 
/*N*/ 				INetURLObject aObj;
/*N*/ 				aObj.SetSmartURL( aPath );
/*N*/ 				aObj.setFinalSlash();
/*N*/ 				try
/*N*/ 				{
/*N*/ 					::ucbhelper::Content aCnt( aObj.GetMainURL(INetURLObject::NO_DECODE),
/*N*/ 						Reference< XCommandEnvironment > () );
/*N*/ 					Reference< sdbc::XResultSet > xResultSet;
/*N*/ 					Sequence< ::rtl::OUString > aProps;
/*N*/ 					try
/*N*/ 					{
/*N*/ 						xResultSet = aCnt.createCursor(
/*N*/ 							aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY );
/*N*/ 					}
/*N*/ 					catch ( Exception& )
/*N*/ 					{
/*N*/ 						// ucb may throw different exceptions on failure now
/*N*/ 						// no assertion if AddIn directory doesn't exist
/*N*/ 					}
/*N*/ 
/*N*/     				if ( xResultSet.is() )
/*N*/     				{
/*N*/ 						Reference< sdbc::XRow > xRow( xResultSet, UNO_QUERY );
/*N*/ 						Reference< XContentAccess >
/*N*/ 							xContentAccess( xResultSet, UNO_QUERY );
/*N*/ 						try
/*N*/ 						{
/*N*/ 							if ( xResultSet->first() )
/*N*/ 							{
/*N*/ 								do
/*?*/ 								{
/*?*/ 									::rtl::OUString aId( xContentAccess->queryContentIdentifierString() );
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 							InitExternalFunc( aId );
/*?*/ 								}
/*?*/ 								while ( xResultSet->next() );
/*?*/ 							}
/*N*/ 						}
/*N*/     					catch ( Exception& )
/*N*/     					{
/*N*/         					DBG_ERRORFILE( "ResultSetException catched!" );
/*N*/     					}
/*N*/     				}
/*N*/ 				}
/*N*/ 				catch ( Exception& )
/*N*/ 				{
/*N*/     				DBG_ERRORFILE( "Exception catched!" );
/*N*/ 				}
/*N*/ 				catch ( ... )
/*N*/ 				{
/*N*/ 
/*N*/ 					DBG_ERRORFILE( "unexpected exception caught!" );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }



}
