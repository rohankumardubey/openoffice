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



#ifndef _DTRITEM_HXX
#define _DTRITEM_HXX

#include <bf_svtools/bf_solar.h>

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif

#include <bf_svtools/poolitem.hxx>

class SvStream;

namespace binfilter {

DBG_NAMEEX(SfxDateTimeRangeItem)

// class SfxDateTimeRangeItem -------------------------------------------------

class SfxDateTimeRangeItem : public SfxPoolItem
{
private:
	DateTime				aStartDateTime;
	DateTime				aEndDateTime;

public:
			TYPEINFO();

			SfxDateTimeRangeItem( const SfxDateTimeRangeItem& rCpy );
			SfxDateTimeRangeItem( USHORT nWhich, const DateTime& rStartDT,
								  const DateTime& rEndDT );

			~SfxDateTimeRangeItem()
				{ DBG_DTOR(SfxDateTimeRangeItem, 0); }

	virtual	int				operator==( const SfxPoolItem& )			const;
    using SfxPoolItem::Compare;
	virtual int				Compare( const SfxPoolItem &rWith )			const;
	virtual SfxPoolItem*	Create( SvStream&, USHORT nItemVersion )	const;
	virtual SvStream&		Store( SvStream&, USHORT nItemVersion )		const;
	virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 )				const;
	virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
									SfxMapUnit eCoreMetric,
									SfxMapUnit ePresMetric,
									XubString &rText,
                                    const ::IntlWrapper * pIntlWrapper = 0 )
		const;

	virtual	BOOL 			PutValue  ( const ::com::sun::star::uno::Any& rVal,
						     			BYTE nMemberId = 0 );
	virtual	BOOL 			QueryValue( ::com::sun::star::uno::Any& rVal,
							 			BYTE nMemberId = 0 ) const;

	const DateTime&			GetStartDateTime()		const { return aStartDateTime; }
	const DateTime&			GetEndDateTime()		const { return aEndDateTime; }

	void					SetStartDateTime( const DateTime& rDT )
							{ DBG_ASSERT( GetRefCount() == 0, "SetDateTime() with pooled item" );
							  aStartDateTime = rDT; }

	void					SetEndDateTime( const DateTime& rDT )
							{ DBG_ASSERT( GetRefCount() == 0, "SetDateTime() with pooled item" );
							  aEndDateTime = rDT; }
};

}

#endif

