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
#include "precompiled_i18npool.hxx"

// prevent internal compiler error with MSVC6SP3
#include <utility>

#include <i18nutil/oneToOneMapping.hxx>
#define TRANSLITERATION_IterationMark_ja_JP
#include <transliteration_Ignore.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

OneToOneMappingTable_t ignoreIterationMark_ja_JP_mappingTable[] = {
    MAKE_PAIR( 0x3046, 0x3094 ),  // HIRAGANA LETTER U --> HIRAGANA LETTER VU
    MAKE_PAIR( 0x304B, 0x304C ),  // HIRAGANA LETTER KA --> HIRAGANA LETTER GA
    MAKE_PAIR( 0x304D, 0x304E ),  // HIRAGANA LETTER KI --> HIRAGANA LETTER GI
    MAKE_PAIR( 0x304F, 0x3050 ),  // HIRAGANA LETTER KU --> HIRAGANA LETTER GU
    MAKE_PAIR( 0x3051, 0x3052 ),  // HIRAGANA LETTER KE --> HIRAGANA LETTER GE
    MAKE_PAIR( 0x3053, 0x3054 ),  // HIRAGANA LETTER KO --> HIRAGANA LETTER GO
    MAKE_PAIR( 0x3055, 0x3056 ),  // HIRAGANA LETTER SA --> HIRAGANA LETTER ZA
    MAKE_PAIR( 0x3057, 0x3058 ),  // HIRAGANA LETTER SI --> HIRAGANA LETTER ZI
    MAKE_PAIR( 0x3059, 0x305A ),  // HIRAGANA LETTER SU --> HIRAGANA LETTER ZU
    MAKE_PAIR( 0x305B, 0x305C ),  // HIRAGANA LETTER SE --> HIRAGANA LETTER ZE
    MAKE_PAIR( 0x305D, 0x305E ),  // HIRAGANA LETTER SO --> HIRAGANA LETTER ZO
    MAKE_PAIR( 0x305F, 0x3060 ),  // HIRAGANA LETTER TA --> HIRAGANA LETTER DA
    MAKE_PAIR( 0x3061, 0x3062 ),  // HIRAGANA LETTER TI --> HIRAGANA LETTER DI
    MAKE_PAIR( 0x3064, 0x3065 ),  // HIRAGANA LETTER TU --> HIRAGANA LETTER DU
    MAKE_PAIR( 0x3066, 0x3067 ),  // HIRAGANA LETTER TE --> HIRAGANA LETTER DE
    MAKE_PAIR( 0x3068, 0x3069 ),  // HIRAGANA LETTER TO --> HIRAGANA LETTER DO
    MAKE_PAIR( 0x306F, 0x3070 ),  // HIRAGANA LETTER HA --> HIRAGANA LETTER BA
    MAKE_PAIR( 0x3072, 0x3073 ),  // HIRAGANA LETTER HI --> HIRAGANA LETTER BI
    MAKE_PAIR( 0x3075, 0x3076 ),  // HIRAGANA LETTER HU --> HIRAGANA LETTER BU
    MAKE_PAIR( 0x3078, 0x3079 ),  // HIRAGANA LETTER HE --> HIRAGANA LETTER BE
    MAKE_PAIR( 0x307B, 0x307C ),  // HIRAGANA LETTER HO --> HIRAGANA LETTER BO
    MAKE_PAIR( 0x309D, 0x309E ),  // HIRAGANA ITERATION MARK --> HIRAGANA VOICED ITERATION MARK
    MAKE_PAIR( 0x30A6, 0x30F4 ),  // KATAKANA LETTER U --> KATAKANA LETTER VU
    MAKE_PAIR( 0x30AB, 0x30AC ),  // KATAKANA LETTER KA --> KATAKANA LETTER GA
    MAKE_PAIR( 0x30AD, 0x30AE ),  // KATAKANA LETTER KI --> KATAKANA LETTER GI
    MAKE_PAIR( 0x30AF, 0x30B0 ),  // KATAKANA LETTER KU --> KATAKANA LETTER GU
    MAKE_PAIR( 0x30B1, 0x30B2 ),  // KATAKANA LETTER KE --> KATAKANA LETTER GE
    MAKE_PAIR( 0x30B3, 0x30B4 ),  // KATAKANA LETTER KO --> KATAKANA LETTER GO
    MAKE_PAIR( 0x30B5, 0x30B6 ),  // KATAKANA LETTER SA --> KATAKANA LETTER ZA
    MAKE_PAIR( 0x30B7, 0x30B8 ),  // KATAKANA LETTER SI --> KATAKANA LETTER ZI
    MAKE_PAIR( 0x30B9, 0x30BA ),  // KATAKANA LETTER SU --> KATAKANA LETTER ZU
    MAKE_PAIR( 0x30BB, 0x30BC ),  // KATAKANA LETTER SE --> KATAKANA LETTER ZE
    MAKE_PAIR( 0x30BD, 0x30BE ),  // KATAKANA LETTER SO --> KATAKANA LETTER ZO
    MAKE_PAIR( 0x30BF, 0x30C0 ),  // KATAKANA LETTER TA --> KATAKANA LETTER DA
    MAKE_PAIR( 0x30C1, 0x30C2 ),  // KATAKANA LETTER TI --> KATAKANA LETTER DI
    MAKE_PAIR( 0x30C4, 0x30C5 ),  // KATAKANA LETTER TU --> KATAKANA LETTER DU
    MAKE_PAIR( 0x30C6, 0x30C7 ),  // KATAKANA LETTER TE --> KATAKANA LETTER DE
    MAKE_PAIR( 0x30C8, 0x30C9 ),  // KATAKANA LETTER TO --> KATAKANA LETTER DO
    MAKE_PAIR( 0x30CF, 0x30D0 ),  // KATAKANA LETTER HA --> KATAKANA LETTER BA
    MAKE_PAIR( 0x30D2, 0x30D3 ),  // KATAKANA LETTER HI --> KATAKANA LETTER BI
    MAKE_PAIR( 0x30D5, 0x30D6 ),  // KATAKANA LETTER HU --> KATAKANA LETTER BU
    MAKE_PAIR( 0x30D8, 0x30D9 ),  // KATAKANA LETTER HE --> KATAKANA LETTER BE
    MAKE_PAIR( 0x30DB, 0x30DC ),  // KATAKANA LETTER HO --> KATAKANA LETTER BO
    MAKE_PAIR( 0x30EF, 0x30F7 ),  // KATAKANA LETTER WA --> KATAKANA LETTER VA
    MAKE_PAIR( 0x30F0, 0x30F8 ),  // KATAKANA LETTER WI --> KATAKANA LETTER VI
    MAKE_PAIR( 0x30F1, 0x30F9 ),  // KATAKANA LETTER WE --> KATAKANA LETTER VE
    MAKE_PAIR( 0x30F2, 0x30FA ),  // KATAKANA LETTER WO --> KATAKANA LETTER VO
    MAKE_PAIR( 0x30FD, 0x30FE )   // KATAKANA ITERATION MARK --> KATAKANA VOICED ITERATION MARK
};


OUString SAL_CALL 
ignoreIterationMark_ja_JP::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
    oneToOneMapping aTable(ignoreIterationMark_ja_JP_mappingTable, sizeof(ignoreIterationMark_ja_JP_mappingTable));

    // Create a string buffer which can hold nCount + 1 characters.
    // The reference count is 0 now.
    rtl_uString * newStr = x_rtl_uString_new_WithLength( nCount ); // defined in x_rtl_ustring.h  
    sal_Unicode * dst = newStr->buffer;
    const sal_Unicode * src = inStr.getStr() + startPos;

    sal_Int32 * p = 0;
    sal_Int32 position = 0;
    if (useOffset) {
        // Allocate nCount length to offset argument.
        offset.realloc( nCount );
        p = offset.getArray();
        position = startPos;
    }

    // 
    sal_Unicode previousChar = *src ++;
    sal_Unicode currentChar;

    // Conversion
    while (-- nCount > 0) {
        currentChar = *src ++;

        switch ( currentChar ) {
            case 0x30fd: // KATAKANA ITERATION MARK
            case 0x309d: // HIRAGANA ITERATION MARK
            case 0x3005: // IDEOGRAPHIC ITERATION MARK
                currentChar = previousChar;
                break;
            case 0x30fe: // KATAKANA VOICED ITERATION MARK
            case 0x309e: // HIRAGANA VOICED ITERATION MARK
                currentChar = aTable[ previousChar ];
                break;
        }
        if (useOffset)
            *p ++ = position ++;
        *dst ++ = previousChar;
        previousChar = currentChar;
    }

    if (nCount == 0) {
        if (useOffset)
            *p = position;
        *dst ++ = previousChar;
    }

    *dst = (sal_Unicode) 0;

    newStr->length = sal_Int32(dst - newStr->buffer);
    if (useOffset)
        offset.realloc(newStr->length);
    return OUString( newStr ); // defined in rtl/usrting. The reference count is increased from 0 to 1.

}

} } } }
