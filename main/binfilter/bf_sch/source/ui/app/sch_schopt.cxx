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



#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
// header for SvStream
// header for SAL_STATIC_CAST
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#include "schopt.hxx"

#include "schresid.hxx"
#include "strings.hrc"
namespace binfilter {

using namespace ::com::sun::star;

#define SCH_OPTIONS_VERSION_001		USHORT( 1 )


// --------------------
// class SchColorTable
// --------------------
/*N*/ SchColorTable::SchColorTable()
/*N*/ {
/*N*/ }


/*N*/ BOOL SchColorTable::Insert( ULONG nKey, XColorEntry* pEntry )
/*N*/ {
/*N*/ 	return Table::Insert( nKey, (void*)pEntry );
/*N*/ }



/*N*/ void SchColorTable::ClearAndDestroy()
/*N*/ {
/*N*/ 	for( ULONG i = Count(); i; )
/*N*/ 		delete Get( --i );
/*N*/ 	Clear();
/*N*/ }

/*N*/ XColorEntry* SchColorTable::Get( ULONG nKey ) const
/*N*/ {
/*N*/ 	return SAL_STATIC_CAST( XColorEntry*, Table::Get( nKey ) );
/*N*/ }

/*N*/ Color SchColorTable::GetColor( ULONG nKey ) const
/*N*/ {
/*N*/ 	XColorEntry* pEntry = Get( nKey );
/*N*/ 	if( pEntry )
/*N*/ 		return pEntry->GetColor();
/*N*/ 
/*N*/ 	return COL_BLACK;
/*N*/ }

/*N*/ ColorData SchColorTable::GetColorData( ULONG nKey ) const
/*N*/ {
/*N*/ 	return GetColor( nKey ).GetRGBColor();
/*N*/ }


// ====================
// class SchOptions
// ====================

/*N*/ SchOptions::SchOptions() :
/*N*/ 		::utl::ConfigItem( ::rtl::OUString::createFromAscii( "Office.Chart" )),
/*N*/ 		mbIsInitialized( FALSE )
/*N*/ {
/*N*/ 	maPropertyNames.realloc( 1 );
/*N*/ 	maPropertyNames[ 0 ] = ::rtl::OUString::createFromAscii( "DefaultColor/Series" );
/*N*/ }

/*N*/ SchOptions::~SchOptions()
/*N*/ {
/*N*/ 	maDefColors.ClearAndDestroy();
/*N*/ }

/*N*/ const SchColorTable& SchOptions::GetDefaultColors()
/*N*/ {
/*N*/ 	if( ! mbIsInitialized )
/*N*/ 	{
/*N*/ 		mbIsInitialized = RetrieveOptions();
/*N*/ 	}
/*N*/ 
/*N*/ 	return maDefColors;
/*N*/ }


/*N*/ BOOL SchOptions::RetrieveOptions()
/*N*/ {
/*N*/ 	// get sequence containing all properties
/*N*/ 	
/*N*/ 	uno::Sequence< ::rtl::OUString > aNames = GetPropertyNames();
/*N*/ 	uno::Sequence< uno::Any > aProperties( aNames.getLength());
/*N*/ 	aProperties = GetProperties( aNames );
/*N*/ 
/*N*/ 	if( aProperties.getLength() == aNames.getLength())
/*N*/ 	{
/*N*/ 		// 1. default colors for series
/*N*/ 		maDefColors.ClearAndDestroy();
/*N*/ 		uno::Sequence< sal_Int64 > aColorSeq;
/*N*/ 		aProperties[ 0 ] >>= aColorSeq;
/*N*/ 
/*N*/ 		sal_Int32 nCount = aColorSeq.getLength();
/*N*/ 		Color aCol;
/*N*/ 
/*N*/ 		// create strings for entry names
/*N*/ 		String aResName( SchResId( STR_DIAGRAM_ROW ));
/*N*/ 		String aPrefix, aPostfix, aName;
/*N*/ 		xub_StrLen nPos = aResName.SearchAscii( "$(ROW)" );
/*N*/ 		if( nPos != STRING_NOTFOUND )
/*N*/ 		{
/*N*/ 			aPrefix = String( aResName, 0, nPos );
/*N*/ 			aPostfix = String( aResName, nPos + sizeof( "$(ROW)" ) - 1, STRING_LEN );
/*N*/ 		}
/*N*/ 		else
/*?*/ 			aPrefix = aResName;
/*N*/ 
/*N*/ 		// set color values
/*N*/ 		for( sal_Int32 i=0; i < nCount; i++ )
/*N*/ 		{
/*N*/ 			aCol.SetColor( SAL_STATIC_CAST( ColorData, aColorSeq[ i ] ));
/*N*/ 
/*N*/ 			aName = aPrefix;
/*N*/ 			aName.Append( String::CreateFromInt32( i + 1 ));
/*N*/ 			aName.Append( aPostfix );
/*N*/ 
/*N*/ 			maDefColors.Insert( i, new XColorEntry( aCol, aName ) );
/*N*/ 		}
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

void SchOptions::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames ) {}
void SchOptions::Commit() {}


// --------------------
// class SchColorTableItem
// --------------------







}
