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



#ifndef _XMLITMAP_HXX
#define _XMLITMAP_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _REF_HXX
#include <tools/ref.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <bf_xmloff/xmltoken.hxx>
#endif

namespace rtl { class OUString; } 
namespace binfilter {

#define MID_FLAG_MASK					0x0000ffff

// this flags are used in the item mapper for import and export

#define MID_FLAG_SPECIAL_ITEM_IMPORT	0x80000000
#define MID_FLAG_NO_ITEM_IMPORT			0x40000000
#define MID_FLAG_SPECIAL_ITEM_EXPORT	0x20000000
#define MID_FLAG_NO_ITEM_EXPORT			0x10000000
#define MID_FLAG_SPECIAL_ITEM			0xa0000000 // both import and export
#define MID_FLAG_NO_ITEM				0x50000000 // both import and export
#define MID_FLAG_ELEMENT_ITEM_IMPORT	0x08000000
#define MID_FLAG_ELEMENT_ITEM_EXPORT	0x04000000
#define MID_FLAG_ELEMENT_ITEM			0x0c000000  // both import and export 

// ---

struct SvXMLItemMapEntry
{
	sal_uInt16 nNameSpace;		// declares the Namespace in wich this item
								// exists
	enum ::binfilter::xmloff::token::XMLTokenEnum eLocalName;
                                // the local name for the item inside 
                                // the Namespace (as an XMLTokenEnum)
	sal_uInt32 nWhichId;		// the WichId to identify the item
								// in the pool
	sal_uInt32 nMemberId;		// the memberid specifies wich part
								// of the item should be imported or
								// exported with this Namespace
								// and localName
};

// ---

class SvXMLItemMapEntries_impl;

/** this class manages an array of SvXMLItemMapEntry. It is
	used for optimizing the static array on startup of import
	or export */
class SvXMLItemMapEntries : public SvRefBase
{
protected:
	SvXMLItemMapEntries_impl* mpImpl;

public:
	SvXMLItemMapEntries( SvXMLItemMapEntry* pEntrys );
	virtual ~SvXMLItemMapEntries();

	SvXMLItemMapEntry* getByName( sal_uInt16 nNameSpace,
								  const ::rtl::OUString& rString,
								  SvXMLItemMapEntry* pStartAt = NULL ) const;
	SvXMLItemMapEntry* getByIndex( sal_uInt16 nIndex ) const;

	sal_uInt16 getCount() const;
};

SV_DECL_REF( SvXMLItemMapEntries )
SV_IMPL_REF( SvXMLItemMapEntries )


} //namespace binfilter
#endif	//  _XMLITMAP_HXX
