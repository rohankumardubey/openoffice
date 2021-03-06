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


#ifndef _SFXMINSTACK_HXX
#define _SFXMINSTACK_HXX

#include <bf_svtools/bf_solar.h>

//ASDBG #ifndef _SFXMINARRAY_HXX
namespace binfilter {

#define DECL_OBJSTACK( ARR, T, nI, nG ) \
DECL_OBJARRAY( ARR##arr_, T, nI, nG ); \
class ARR: private ARR##arr_ \
{ \
public: \
	ARR( BYTE nInitSize = nI, BYTE nGrowSize = nG ): \
		ARR##arr_( nInitSize, nGrowSize ) \
    {} \
\
	ARR( const ARR& rOrig ): \
		ARR##arr_( rOrig ) \
    {} \
\
	USHORT		Count() const { return ARR##arr_::Count(); } \
	void		Push( const T& rElem ) { Append( rElem ); } \
	const T& Top( USHORT nLevel = 0 ) const \
                { return (*this)[Count()-nLevel-1]; } \
	const T& Bottom() const { return (*this)[0]; } \
	T		 Pop(); \
	void		Clear() { ARR##arr_::Clear(); } \
	BOOL		Contains( const T& rItem ) const \
				{ return ARR##arr_::Contains( rItem ); } \
}

#define IMPL_OBJSTACK( ARR, T ) \
IMPL_OBJARRAY( ARR##arr_, T ); \
\
T ARR::Pop() \
{	T aRet = (*this)[Count()-1]; \
    Remove( Count()-1, 1 ); \
    return aRet; \
}
//STRIP008 the following "DECL_PTRARRAY( ARR##arr_, T, nI, nG );\" should be "DECL_PTRARRAY( ARR##arr_, T, nI, nG )\"
#define DECL_PTRSTACK( ARR, T, nI, nG ) \
DECL_PTRARRAY( ARR##arr_, T, nI, nG ) \
class ARR: private ARR##arr_ \
{ \
public: \
	ARR( BYTE nInitSize = nI, BYTE nGrowSize = nG ): \
		ARR##arr_( nInitSize, nGrowSize ) \
    {} \
\
	ARR( const ARR& rOrig ): \
		ARR##arr_( rOrig ) \
    {} \
\
	USHORT		Count() const { return ARR##arr_::Count(); } \
	void		Push( T rElem ) { Append( rElem ); } \
    BOOL        Replace( T rOldElem, T rNewElem ) \
                { return ARR##arr_::Replace( rOldElem, rNewElem ); } \
	T			Top( USHORT nLevel = 0 ) const \
                { return (*this)[Count()-nLevel-1]; } \
	T			Bottom() const { return (*this)[0]; } \
	T			Pop() \
				{	T aRet = (*this)[Count()-1]; \
                    Remove( Count()-1, 1 ); \
                    return aRet; \
                } \
	T*		 operator*() \
                { return &(*this)[Count()-1]; } \
	void		Clear() { ARR##arr_::Clear(); } \
	BOOL		Contains( const T pItem ) const \
				{ return ARR##arr_::Contains( pItem ); } \
}

}//end of namespace binfilter
#endif

