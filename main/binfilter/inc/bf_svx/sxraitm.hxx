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


#ifndef _SXRAITM_HXX
#define _SXRAITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <bf_svx/svddef.hxx>
#endif

#ifndef _SDANGITM_HXX
#include <bf_svx/sdangitm.hxx>
#endif
namespace binfilter {

//------------------------------
// class SdrRotateAngleItem
//------------------------------
class SdrRotateAngleItem: public SdrAngleItem {
public:
	SdrRotateAngleItem(long nAngle=0): SdrAngleItem(SDRATTR_ROTATEANGLE,nAngle) {}
	SdrRotateAngleItem(SvStream& rIn): SdrAngleItem(SDRATTR_ROTATEANGLE,rIn)    {}
};

}//end of namespace binfilter
#endif
