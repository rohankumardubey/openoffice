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


#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIMERANGE_HPP_
#include <com/sun/star/util/DateTimeRange.hpp>
#endif

#include <unotools/intlwrapper.hxx>
#include <comphelper/processfactory.hxx>

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/datetime.hxx>
#include <rtl/math.hxx>

#include <vcl/svapp.hxx>

#include <bf_svtools/dtritem.hxx>

namespace binfilter
{

DBG_NAME( SfxDateTimeRangeItem )
TYPEINIT1( SfxDateTimeRangeItem, SfxPoolItem );

// -----------------------------------------------------------------------

SfxDateTimeRangeItem::SfxDateTimeRangeItem( USHORT which, const DateTime& rStartDT,
											const DateTime& rEndDT ) :
	SfxPoolItem( which ),
	aStartDateTime( rStartDT ),
	aEndDateTime( rEndDT )
{
	DBG_CTOR( SfxDateTimeRangeItem, 0 );
}

// -----------------------------------------------------------------------

SfxDateTimeRangeItem::SfxDateTimeRangeItem( const SfxDateTimeRangeItem& rItem ) :
	SfxPoolItem( rItem ),
	aStartDateTime( rItem.aStartDateTime ),
	aEndDateTime( rItem.aEndDateTime )
{
	DBG_CTOR( SfxDateTimeRangeItem, 0 );
}

// -----------------------------------------------------------------------

int SfxDateTimeRangeItem::operator==( const SfxPoolItem& rItem ) const
{
	DBG_CHKTHIS( SfxDateTimeRangeItem, 0 );
	DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );

	return  ((SfxDateTimeRangeItem&)rItem ).aStartDateTime == aStartDateTime &&
			((SfxDateTimeRangeItem&)rItem ).aEndDateTime == aEndDateTime;
}

// -----------------------------------------------------------------------

int SfxDateTimeRangeItem::Compare( const SfxPoolItem& rItem ) const
{
	DBG_CHKTHIS( SfxDateTimeRangeItem, 0 );
	DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );

	double fThisRange = aEndDateTime - aStartDateTime;
	double fRange = ((const SfxDateTimeRangeItem&)rItem).aEndDateTime -
		((const SfxDateTimeRangeItem&)rItem).aStartDateTime;

	if ( ::rtl::math::approxEqual( fRange, fThisRange ) )
		return 0;
	else if ( fRange < fThisRange )
		return -1;
	else
		return 1;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxDateTimeRangeItem::Create( SvStream& rStream, USHORT ) const
{
	DBG_CHKTHIS(SfxDateTimeRangeItem, 0);

	sal_uInt32 nStartDate, nEndDate;
	sal_Int32 nStartTime, nEndTime;

	rStream >> nStartDate;
	rStream >> nStartTime;

	rStream >> nEndDate;
	rStream >> nEndTime;

	DateTime aStartDT, aEndDT;

	aStartDT.SetDate( nStartDate );
	aStartDT.SetTime( nStartTime );

	aEndDT.SetDate( nEndDate );
	aEndDT.SetTime( nEndTime );

	return new SfxDateTimeRangeItem( Which(), aStartDT, aEndDT );
}

// -----------------------------------------------------------------------

SvStream& SfxDateTimeRangeItem::Store( SvStream& rStream, USHORT ) const
{
	DBG_CHKTHIS( SfxDateTimeRangeItem, 0 );

	rStream << aStartDateTime.GetDate();
	rStream << aStartDateTime.GetTime();

	rStream << aEndDateTime.GetDate();
	rStream << aEndDateTime.GetTime();

	return rStream;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxDateTimeRangeItem::Clone( SfxItemPool* ) const
{
	DBG_CHKTHIS( SfxDateTimeRangeItem, 0 );

	return new SfxDateTimeRangeItem( *this );
}
// -----------------------------------------------------------------------

SfxItemPresentation SfxDateTimeRangeItem::GetPresentation
(
	SfxItemPresentation 	/*ePresentation*/,
	SfxMapUnit				/*eCoreMetric*/,
	SfxMapUnit				/*ePresentationMetric*/,
	XubString& 				rText,
    const ::IntlWrapper *   pIntlWrapper
)	const
{
	DBG_CHKTHIS(SfxDateTimeRangeItem, 0);
	DateTime aRange(aEndDateTime - (const Time&)aStartDateTime);
    if (pIntlWrapper)
	{
        rText = pIntlWrapper->getLocaleData()->getDate(aRange);
		rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
        rText += pIntlWrapper->getLocaleData()->getTime(aRange);
	}
	else
	{
		DBG_WARNING("SfxDateTimeRangeItem::GetPresentation():"
                     " Using default en_US IntlWrapper");
        const ::IntlWrapper aIntlWrapper(
            ::comphelper::getProcessServiceFactory(), LANGUAGE_ENGLISH_US );
        rText = aIntlWrapper.getLocaleData()->getDate(aRange);
		rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
        rText += aIntlWrapper.getLocaleData()->getTime(aRange);
	}
	return SFX_ITEM_PRESENTATION_NAMELESS;
}

//----------------------------------------------------------------------------
// virtual
BOOL SfxDateTimeRangeItem::PutValue( const com::sun::star::uno::Any& rVal,BYTE )
{
	com::sun::star::util::DateTimeRange aValue;
	if ( rVal >>= aValue )
	{
		aStartDateTime = DateTime( Date( aValue.StartDay,
			  						     aValue.StartMonth,
			  						     aValue.StartYear ),
							  	   Time( aValue.StartHours,
			  							 aValue.StartMinutes,
			  							 aValue.StartSeconds,
			  							 aValue.StartHundredthSeconds ) );
		aEndDateTime   = DateTime( Date( aValue.EndDay,
			  						     aValue.EndMonth,
			  						     aValue.EndYear ),
							  	   Time( aValue.EndHours,
			  							 aValue.EndMinutes,
			  							 aValue.EndSeconds,
			  							 aValue.EndHundredthSeconds ) );
		return TRUE;
	}

	DBG_ERROR( "SfxDateTimeRangeItem::PutValue - Wrong type!" );
	return FALSE;
}

//----------------------------------------------------------------------------
// virtual
BOOL SfxDateTimeRangeItem::QueryValue( com::sun::star::uno::Any& rVal,
							 	       BYTE nMemberId ) const
{
	nMemberId &= ~CONVERT_TWIPS;
	com::sun::star::util::DateTimeRange aValue( aStartDateTime.Get100Sec(),
    									   		aStartDateTime.GetSec(),
   										   		aStartDateTime.GetMin(),
    									   		aStartDateTime.GetHour(),
    									   		aStartDateTime.GetDay(),
    									   		aStartDateTime.GetMonth(),
    									   		aStartDateTime.GetYear(),
												aEndDateTime.Get100Sec(),
    									   		aEndDateTime.GetSec(),
   										   		aEndDateTime.GetMin(),
    									   		aEndDateTime.GetHour(),
    									   		aEndDateTime.GetDay(),
    									   		aEndDateTime.GetMonth(),
    									   		aEndDateTime.GetYear() );
	rVal <<= aValue;
	return TRUE;
}


}
