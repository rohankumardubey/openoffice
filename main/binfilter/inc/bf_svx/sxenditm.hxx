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


#ifndef _SXENDITM_HXX
#define _SXENDITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <bf_svx/svddef.hxx>
#endif

#ifndef _SDMETITM_HXX
#include <bf_svx/sdmetitm.hxx>
#endif
namespace binfilter {

class SdrEdgeNode1HorzDistItem: public SdrMetricItem {
public:
	SdrEdgeNode1HorzDistItem(long nVal=0): SdrMetricItem(SDRATTR_EDGENODE1HORZDIST,nVal)  {}
	SdrEdgeNode1HorzDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGENODE1HORZDIST,rIn) {}
};

class SdrEdgeNode1VertDistItem: public SdrMetricItem {
public:
	SdrEdgeNode1VertDistItem(long nVal=0): SdrMetricItem(SDRATTR_EDGENODE1VERTDIST,nVal)  {}
	SdrEdgeNode1VertDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGENODE1VERTDIST,rIn) {}
};

class SdrEdgeNode2HorzDistItem: public SdrMetricItem {
public:
	SdrEdgeNode2HorzDistItem(long nVal=0): SdrMetricItem(SDRATTR_EDGENODE2HORZDIST,nVal)  {}
	SdrEdgeNode2HorzDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGENODE2HORZDIST,rIn) {}
};

class SdrEdgeNode2VertDistItem: public SdrMetricItem {
public:
	SdrEdgeNode2VertDistItem(long nVal=0): SdrMetricItem(SDRATTR_EDGENODE2VERTDIST,nVal)  {}
	SdrEdgeNode2VertDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGENODE2VERTDIST,rIn) {}
};

class SdrEdgeNode1GlueDistItem: public SdrMetricItem {
public:
	SdrEdgeNode1GlueDistItem(long nVal=0): SdrMetricItem(SDRATTR_EDGENODE1GLUEDIST,nVal)  {}
	SdrEdgeNode1GlueDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGENODE1GLUEDIST,rIn) {}
};

class SdrEdgeNode2GlueDistItem: public SdrMetricItem {
public:
	SdrEdgeNode2GlueDistItem(long nVal=0): SdrMetricItem(SDRATTR_EDGENODE2GLUEDIST,nVal)  {}
	SdrEdgeNode2GlueDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGENODE2GLUEDIST,rIn) {}
};

}//end of namespace binfilter
#endif
