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


#ifndef _SVX_PBINITEM_HXX
#define _SVX_PBINITEM_HXX

// include ---------------------------------------------------------------

#ifndef _SFXINTITEM_HXX //autogen
#include <bf_svtools/intitem.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif
namespace binfilter {

// define ----------------------------------------------------------------

#define PAPERBIN_PRINTER_SETTINGS	((BYTE)0xFF)

// class SvxPaperBinItem -------------------------------------------------

/*	[Beschreibung]

	Dieses Item beschreibt die Auswahl eines Papierschachts des Drucker.
*/

class SvxPaperBinItem : public SfxByteItem
{
public:
	TYPEINFO();

	inline SvxPaperBinItem( const USHORT nId = ITEMID_PAPERBIN,
							const BYTE nTray = PAPERBIN_PRINTER_SETTINGS );
	inline SvxPaperBinItem &operator=( const SvxPaperBinItem &rCpy );

	// "pure virtual Methoden" vom SfxPoolItem
	virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
	virtual SfxPoolItem*    Create( SvStream &, USHORT ) const;
	virtual SvStream&		Store( SvStream &, USHORT nItemVersion ) const;
};

inline SvxPaperBinItem::SvxPaperBinItem( const USHORT nId, const BYTE nT )
	: SfxByteItem( nId, nT )
{}

inline SvxPaperBinItem &SvxPaperBinItem::operator=(
	const SvxPaperBinItem &rCpy )
{
	SetValue( rCpy.GetValue() );
	return *this;
}

}//end of namespace binfilter
#endif

