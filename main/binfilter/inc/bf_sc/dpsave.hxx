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



#ifndef SC_DPSAVE_HXX
#define SC_DPSAVE_HXX

#include <bf_svtools/bf_solar.h>

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _LIST_HXX //autogen wg. List
#include <tools/list.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XDIMENSIONSSUPPLIER_HPP_
#include <com/sun/star/sheet/XDimensionsSupplier.hpp>
#endif

class SvStream;

// --------------------------------------------------------------------
//
//	classes to save Data Pilot settings
//

namespace binfilter {

class ScDPSaveMember
{
private:
	String		aName;
	USHORT		nVisibleMode;
	USHORT		nShowDetailsMode;

public:
							ScDPSaveMember(const String& rName);
							ScDPSaveMember(const ScDPSaveMember& r);
							ScDPSaveMember(SvStream& rStream);
							~ScDPSaveMember();

	BOOL		 			operator== ( const ScDPSaveMember& r ) const;

	const String&			GetName()	{ return aName; }
	void					SetIsVisible(BOOL bSet);
	BOOL					GetIsVisible() { return BOOL(nVisibleMode); }
	void					SetShowDetails(BOOL bSet);
	BOOL					GetShowDetails() { return BOOL(nShowDetailsMode); }

	void					WriteToSource( const ::com::sun::star::uno::Reference<
											::com::sun::star::uno::XInterface>& xMember );

	void					Store( SvStream& rStream ) const;
};


class ScDPSaveDimension
{
private:
	String		aName;
	String*		pLayoutName;		// alternative name for layout, not used (yet)
	BOOL		bIsDataLayout;
	BOOL		bDupFlag;
	USHORT		nOrientation;
	USHORT		nFunction;			// enum GeneralFunction, for data dimensions
	long		nUsedHierarchy;
	USHORT		nShowEmptyMode;		//!	at level
	BOOL		bSubTotalDefault;	//!	at level
	long		nSubTotalCount;
	USHORT*		pSubTotalFuncs;		// enum GeneralFunction
	List		aMemberList;

public:
							ScDPSaveDimension(const String& rName, BOOL bDataLayout);
							ScDPSaveDimension(const ScDPSaveDimension& r);
							ScDPSaveDimension(SvStream& rStream);
							~ScDPSaveDimension();

	BOOL		 			operator== ( const ScDPSaveDimension& r ) const;

	const List&				GetMembers() { return aMemberList; }
	void					AddMember(ScDPSaveMember* pMember) { aMemberList.Insert(pMember, LIST_APPEND); };

	void					SetDupFlag(BOOL bSet)	{ bDupFlag = bSet; }
	BOOL					GetDupFlag() const		{ return bDupFlag; }

	const String&			GetName() const			{ return aName; }
	BOOL					IsDataLayout() const	{ return bIsDataLayout; }

	void					SetOrientation(USHORT nNew);
	void					SetSubTotals(long nCount, const USHORT* pFuncs);
	long					GetSubTotalsCount() { return nSubTotalCount; }
	USHORT					GetSubTotalFunc(long nIndex) { return pSubTotalFuncs[nIndex]; }
	void					SetShowEmpty(BOOL bSet);
	BOOL					GetShowEmpty() { return BOOL(nShowEmptyMode); }
	void					SetFunction(USHORT nNew);		// enum GeneralFunction
	USHORT					GetFunction() { return nFunction; }
	void					SetUsedHierarchy(long nNew);
	long					GetUsedHierarchy() { return nUsedHierarchy; }
	void					SetLayoutName(const String* pName);
	const String&			GetLayoutName() const;
	BOOL					HasLayoutName() const;
	void					ResetLayoutName();

	USHORT					GetOrientation() const	{ return nOrientation; }


	void					WriteToSource( const ::com::sun::star::uno::Reference<
											::com::sun::star::uno::XInterface>& xDim );

	void					Store( SvStream& rStream ) const;
};


class ScDPSaveData
{
private:
	List		aDimList;
	USHORT		nColumnGrandMode;
	USHORT		nRowGrandMode;
	USHORT		nIgnoreEmptyMode;
	USHORT		nRepeatEmptyMode;

public:
							ScDPSaveData();
							ScDPSaveData(const ScDPSaveData& r);
							~ScDPSaveData();

	ScDPSaveData& 			operator= ( const ScDPSaveData& r );

	BOOL		 			operator== ( const ScDPSaveData& r ) const;

	const List&				GetDimensions() const { return aDimList; }
	void					AddDimension(ScDPSaveDimension* pDim) { aDimList.Insert(pDim, LIST_APPEND); }

	ScDPSaveDimension*		GetDimensionByName(const String& rName);
	ScDPSaveDimension*		GetDataLayoutDimension();

	ScDPSaveDimension*		DuplicateDimension(const String& rName);

	ScDPSaveDimension*		GetExistingDimensionByName(const String& rName);

	void					SetColumnGrand( BOOL bSet );
	BOOL					GetColumnGrand() const { return BOOL(nColumnGrandMode); }
	void					SetRowGrand( BOOL bSet );
	BOOL					GetRowGrand() const { return BOOL(nRowGrandMode); }
	void					SetIgnoreEmptyRows( BOOL bSet );
	BOOL					GetIgnoreEmptyRows() const { return BOOL(nIgnoreEmptyMode); }
	void					SetRepeatIfEmpty( BOOL bSet );
	BOOL					GetRepeatIfEmpty() const { return BOOL(nRepeatEmptyMode); }

	void					WriteToSource( const ::com::sun::star::uno::Reference<
											::com::sun::star::sheet::XDimensionsSupplier>& xSource );

	void					Store( SvStream& rStream ) const;
	void					Load( SvStream& rStream );

};


} //namespace binfilter
#endif

