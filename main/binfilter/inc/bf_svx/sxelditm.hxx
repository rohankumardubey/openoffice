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


#ifndef _SXELDITM_HXX
#define _SXELDITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <bf_svx/svddef.hxx>
#endif

#ifndef _SDMETITM_HXX
#include <bf_svx/sdmetitm.hxx>
#endif
namespace binfilter {

class SdrEdgeLineDeltaAnzItem: public SfxUInt16Item {
public:
	SdrEdgeLineDeltaAnzItem(UINT16 nVal=0): SfxUInt16Item(SDRATTR_EDGELINEDELTAANZ,nVal) {}
	SdrEdgeLineDeltaAnzItem(SvStream& rIn): SfxUInt16Item(SDRATTR_EDGELINEDELTAANZ,rIn)  {}
};

class SdrEdgeLine1DeltaItem: public SdrMetricItem {
public:
	SdrEdgeLine1DeltaItem(long nVal=0): SdrMetricItem(SDRATTR_EDGELINE1DELTA,nVal)  {}
	SdrEdgeLine1DeltaItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGELINE1DELTA,rIn) {}
};

class SdrEdgeLine2DeltaItem: public SdrMetricItem {
public:
	SdrEdgeLine2DeltaItem(long nVal=0): SdrMetricItem(SDRATTR_EDGELINE2DELTA,nVal)  {}
	SdrEdgeLine2DeltaItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGELINE2DELTA,rIn) {}
};

class SdrEdgeLine3DeltaItem: public SdrMetricItem {
public:
	SdrEdgeLine3DeltaItem(long nVal=0): SdrMetricItem(SDRATTR_EDGELINE3DELTA,nVal)  {}
	SdrEdgeLine3DeltaItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGELINE3DELTA,rIn) {}
};

}//end of namespace binfilter
#endif
