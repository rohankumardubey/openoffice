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



#ifndef _SVX_XFTSFIT_HXX
#define _SVX_XFTSFIT_HXX

#include "svx/svxdllapi.h"

/*************************************************************************
|*
|* FormText-StdFormItem
|*
\************************************************************************/

class SVX_DLLPUBLIC XFormTextStdFormItem : public SfxEnumItem
{
public:
							TYPEINFO();
							XFormTextStdFormItem(XFormTextStdForm = XFTFORM_NONE);
							XFormTextStdFormItem(SvStream& rIn);
	virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
	virtual SfxPoolItem*    Create(SvStream& rIn, sal_uInt16 nVer) const;
	virtual sal_uInt16          GetValueCount() const;
	XFormTextStdForm		GetValue() const { return (XFormTextStdForm) SfxEnumItem::GetValue(); }
	// #FontWork#
	virtual	sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
	virtual	sal_Bool		PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

#endif

