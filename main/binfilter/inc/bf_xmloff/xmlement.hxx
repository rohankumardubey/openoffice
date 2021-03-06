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



#ifndef _XMLOFF_XMLEMENT_HXX
#define _XMLOFF_XMLEMENT_HXX

#ifndef _SAL_TYPES_H 
#include <sal/types.h>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <bf_xmloff/xmltoken.hxx>
#endif
namespace binfilter {

/** Map an XMLTokenEnum to a sal_uInt16 value.
 * To be used with SvXMLUnitConverter::convertEnum(...) 
 */
struct SvXMLEnumMapEntry
{
	::binfilter::xmloff::token::XMLTokenEnum   eToken;
	sal_uInt16                      nValue;
};

#define ENUM_STRING_MAP_ENTRY(name,tok) { name, sizeof(name)-1, tok }

#define ENUM_STRING_MAP_END()           { NULL, 0, 0 }

/** Map a const sal_Char* (with length) to a sal_uInt16 value.
 * To be used with SvXMLUnitConverter::convertEnum(...) 
 */
struct SvXMLEnumStringMapEntry
{
	const sal_Char *    pName;
    sal_Int32           nNameLength;
	sal_uInt16          nValue;
};

}//end of namespace binfilter
#endif	//  _XMLOFF_XMLEMENT_HXX

