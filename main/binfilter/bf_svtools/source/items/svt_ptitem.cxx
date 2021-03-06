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


#include <bf_svtools/ptitem.hxx>

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#include <bf_svtools/poolitem.hxx>
#include "memberid.hrc"

using namespace ::com::sun::star;

namespace binfilter
{

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxPointItem)

#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)    ((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))

// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SfxPointItem, SfxPoolItem);

// -----------------------------------------------------------------------

SfxPointItem::SfxPointItem()
{
	DBG_CTOR(SfxPointItem, 0);
}

// -----------------------------------------------------------------------

SfxPointItem::SfxPointItem( USHORT nW, const Point& rVal ) :
	SfxPoolItem( nW ),
	aVal( rVal )
{
	DBG_CTOR(SfxPointItem, 0);
}

// -----------------------------------------------------------------------

SfxPointItem::SfxPointItem( const SfxPointItem& rItem ) :
	SfxPoolItem( rItem ),
	aVal( rItem.aVal )
{
	DBG_CTOR(SfxPointItem, 0);
}

// -----------------------------------------------------------------------

SfxItemPresentation SfxPointItem::GetPresentation
(
	SfxItemPresentation 	/*ePresentation*/,
	SfxMapUnit				/*eCoreMetric*/,
	SfxMapUnit				/*ePresentationMetric*/,
	XubString& 				rText,
    const ::IntlWrapper *
)	const
{
	DBG_CHKTHIS(SfxPointItem, 0);
	rText = UniString::CreateFromInt32(aVal.X());
	rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
	rText += UniString::CreateFromInt32(aVal.Y());
	rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
	return SFX_ITEM_PRESENTATION_NAMELESS;
}

// -----------------------------------------------------------------------

int SfxPointItem::operator==( const SfxPoolItem& rItem ) const
{
	DBG_CHKTHIS(SfxPointItem, 0);
	DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
	return ((SfxPointItem&)rItem).aVal == aVal;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxPointItem::Clone(SfxItemPool *) const
{
	DBG_CHKTHIS(SfxPointItem, 0);
	return new SfxPointItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxPointItem::Create(SvStream &rStream, USHORT ) const
{
	DBG_CHKTHIS(SfxPointItem, 0);
	Point aStr;
	rStream >> aStr;
	return new SfxPointItem(Which(), aStr);
}

// -----------------------------------------------------------------------

SvStream& SfxPointItem::Store(SvStream &rStream, USHORT ) const
{
	DBG_CHKTHIS(SfxPointItem, 0);
	rStream << aVal;
	return rStream;
}

// -----------------------------------------------------------------------

BOOL SfxPointItem::QueryValue( uno::Any& rVal,
							   BYTE nMemberId ) const
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    awt::Point aTmp(aVal.X(), aVal.Y());
    if( bConvert )
    {
        aTmp.X = TWIP_TO_MM100(aTmp.X);
        aTmp.Y = TWIP_TO_MM100(aTmp.Y);
    }
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0: rVal <<= aTmp; break;
        case MID_X: rVal <<= aTmp.X; break;
        case MID_Y: rVal <<= aTmp.Y; break;
        default: DBG_ERROR("Wrong MemberId!"); return FALSE;
    }

	return TRUE;
}

// -----------------------------------------------------------------------

BOOL SfxPointItem::PutValue( const uno::Any& rVal,
							 BYTE nMemberId )
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
	BOOL bRet = FALSE;
    awt::Point aValue;
    sal_Int32 nVal = 0;
    if ( !nMemberId )
    {        
        bRet = ( rVal >>= aValue );
        if( bConvert )
        {
            aValue.X = MM100_TO_TWIP(aValue.X);
            aValue.Y = MM100_TO_TWIP(aValue.Y);
        }        
    }
    else
    {        
        bRet = ( rVal >>= nVal );
        if( bConvert )
            nVal = MM100_TO_TWIP( nVal );
    }

    if ( bRet )
    {
        switch ( nMemberId )
        {
            case 0: aVal.setX( aValue.X ); aVal.setY( aValue.Y ); break;
            case MID_X: aVal.setX( nVal ); break;
            case MID_Y: aVal.setY( nVal ); break;
            default: DBG_ERROR("Wrong MemberId!"); return FALSE;
        }
    }

	return bRet;
}

}
