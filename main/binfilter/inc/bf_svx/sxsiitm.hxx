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


#ifndef _SXSIITM_HXX
#define _SXSIITM_HXX

#ifndef _SXFIITM_HXX
#include <bf_svx/sxfiitm.hxx>
#endif
namespace binfilter {

class SdrScaleItem: public SdrFractionItem {
public:
	TYPEINFO();
	SdrScaleItem(USHORT nId=0): SdrFractionItem(nId,Fraction(1,1)) {}
	SdrScaleItem(USHORT nId, const Fraction& rVal): SdrFractionItem(nId,rVal) {}
	SdrScaleItem(USHORT nId, SvStream& rIn): SdrFractionItem(nId,rIn) {}
	virtual SfxPoolItem*     Create(SvStream&, USHORT nVer) const;
	virtual SfxPoolItem*     Clone(SfxItemPool *pPool=NULL) const;
};

}//end of namespace binfilter
#endif
