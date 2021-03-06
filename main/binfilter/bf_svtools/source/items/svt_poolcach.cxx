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


#include <limits.h>

#ifndef GCC
#endif

#include <bf_svtools/itempool.hxx>
#include <bf_svtools/itemset.hxx>
#include "poolcach.hxx"

namespace binfilter
{

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxItemPoolCache)


//------------------------------------------------------------------------

struct SfxItemModifyImpl
{
	const SfxSetItem  *pOrigItem;
	SfxSetItem		  *pPoolItem;
};

SV_DECL_VARARR( SfxItemModifyArr_Impl, SfxItemModifyImpl, 8, 8 )
SV_IMPL_VARARR( SfxItemModifyArr_Impl, SfxItemModifyImpl);

//------------------------------------------------------------------------

SfxItemPoolCache::SfxItemPoolCache( SfxItemPool *pItemPool,
									const SfxItemSet *pPutSet ):
	pPool(pItemPool),
	pCache(new SfxItemModifyArr_Impl),
	pSetToPut( pPutSet ),
	pItemToPut( 0 )
{
	DBG_CTOR(SfxItemPoolCache, 0);
	DBG_ASSERT(pItemPool, "kein Pool angegeben");
}

//------------------------------------------------------------------------

SfxItemPoolCache::~SfxItemPoolCache()
{
	DBG_DTOR(SfxItemPoolCache, 0);
	for ( USHORT nPos = 0; nPos < pCache->Count(); ++nPos ) {
		pPool->Remove( *(*pCache)[nPos].pPoolItem );
		pPool->Remove( *(*pCache)[nPos].pOrigItem );
	}
	delete pCache; pCache = 0;

	if ( pItemToPut )
		pPool->Remove( *pItemToPut );
}

//------------------------------------------------------------------------

const SfxSetItem& SfxItemPoolCache::ApplyTo( const SfxSetItem &rOrigItem, BOOL bNew )
{
	DBG_CHKTHIS(SfxItemPoolCache, 0);
	DBG_ASSERT( pPool == rOrigItem.GetItemSet().GetPool(), "invalid Pool" );
	DBG_ASSERT( IsDefaultItem( &rOrigItem ) || IsPooledItem( &rOrigItem ),
				"original not in pool" );

	// Suchen, ob diese Transformations schon einmal vorkam
	for ( USHORT nPos = 0; nPos < pCache->Count(); ++nPos )
	{
		SfxItemModifyImpl &rMapEntry = (*pCache)[nPos];
		if ( rMapEntry.pOrigItem == &rOrigItem )
		{
			// aendert sich ueberhaupt etwas?
			if ( rMapEntry.pPoolItem != &rOrigItem )
			{
				rMapEntry.pPoolItem->AddRef(2); // einen davon fuer den Cache
				if ( bNew )
					pPool->Put( rOrigItem );	//! AddRef??
			}
			return *rMapEntry.pPoolItem;
		}
	}

	// die neue Attributierung in einem neuen Set eintragen
	SfxSetItem *pNewItem = (SfxSetItem *)rOrigItem.Clone();
	if ( pItemToPut )
	{
		pNewItem->GetItemSet().PutDirect( *pItemToPut );
		DBG_ASSERT( &pNewItem->GetItemSet().Get( pItemToPut->Which() ) == pItemToPut,
					"wrong item in temporary set" );
	}
	else
		pNewItem->GetItemSet().Put( *pSetToPut );
	const SfxSetItem* pNewPoolItem = (const SfxSetItem*) &pPool->Put( *pNewItem );
	DBG_ASSERT( pNewPoolItem != pNewItem, "Pool: rein == raus?" );
	delete pNewItem;

	// Refernzzaehler anpassen, je einen davon fuer den Cache
	pNewPoolItem->AddRef( pNewPoolItem != &rOrigItem ? 2 : 1 );
	if ( bNew )
		pPool->Put( rOrigItem );	//! AddRef??

	// die Transformation im Cache eintragen
	SfxItemModifyImpl aModify;
	aModify.pOrigItem = &rOrigItem;
	aModify.pPoolItem = (SfxSetItem*) pNewPoolItem;
	pCache->Insert( aModify, pCache->Count() );

	DBG_ASSERT( !pItemToPut ||
				&pNewPoolItem->GetItemSet().Get( pItemToPut->Which() ) == pItemToPut,
				"wrong item in resulting set" );

	return *pNewPoolItem;
}

}
