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


#ifndef INCLUDED_I18NUTIL_X_RTL_USTRING_H
#define INCLUDED_I18NUTIL_X_RTL_USTRING_H

#ifndef _RTL_STRING_HXX_
#include <rtl/strbuf.hxx>
#endif
#include <rtl/memory.h>
#include <rtl/alloc.h>


/**
 * Allocates a new <code>rtl_uString</code> which can hold nLen + 1 characters.
 * The reference count is 0. The characters of room is not cleared.
 * This method is similar to rtl_uString_new_WithLength in rtl/ustring.h, but 
 * can allocate a new string more efficiently. You need to "acquire" by such as 
 * OUString( rtl_uString * value ) if you intend to use it for a while.
 * @param	[output] newStr
 * @param	[input]  nLen
 */
inline void SAL_CALL x_rtl_uString_new_WithLength( rtl_uString ** newStr, sal_Int32 nLen, sal_Int32 _refCount = 0 )
{
  *newStr = (rtl_uString*) rtl_allocateMemory ( sizeof(rtl_uString) + sizeof(sal_Unicode) * nLen);
  (*newStr)->refCount = _refCount;
  (*newStr)->length = nLen;

  // rtl_uString is defined in rtl/ustring.h as below:
  //typedef struct _rtl_uString
  //{
  //    sal_Int32		refCount;
  //	sal_Int32		length;
  //	sal_Unicode 	buffer[1];
  //} rtl_uString;
}	

inline rtl_uString * SAL_CALL x_rtl_uString_new_WithLength( sal_Int32 nLen, sal_Int32 _refCount = 0 )
{
  rtl_uString *newStr = (rtl_uString*) rtl_allocateMemory ( sizeof(rtl_uString) + sizeof(sal_Unicode) * nLen);
  newStr->refCount = _refCount;
  newStr->length = nLen;
  return newStr;
}	

/**
 * Release <code>rtl_uString</code> regardless its reference count.
 */
inline void SAL_CALL x_rtl_uString_release( rtl_uString * value )
{
  rtl_freeMemory(value);
}


#endif // #ifndef _I18N_X_RTL_USTRING_H_
