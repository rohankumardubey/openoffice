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



#ifndef _SVX_XLNSTWIT_HXX
#define _SVX_XLNSTWIT_HXX

#ifndef _SFXMETRICITEM_HXX //autogen
#include <bf_svtools/metitem.hxx>
#endif
namespace binfilter {

#define	LINE_START_WIDTH_DEFAULT		(200)

//-----------------------------
// class XLineStartWidthItem
//-----------------------------
class XLineStartWidthItem : public SfxMetricItem
{
public:
			TYPEINFO();
			XLineStartWidthItem(long nWidth = LINE_START_WIDTH_DEFAULT);
			XLineStartWidthItem(SvStream& rIn);
	virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
	virtual SfxPoolItem*    Create(SvStream& rIn, USHORT nVer) const;

	virtual	sal_Bool        	 QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
	virtual	sal_Bool			 PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );


};

}//end of namespace binfilter
#endif
