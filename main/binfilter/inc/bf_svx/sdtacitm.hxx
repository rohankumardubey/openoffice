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


#ifndef sdtacitm_hxx
#define sdtacitm_hxx

#ifndef _SFXINTITEM_HXX //autogen
#include <bf_svtools/intitem.hxx>
#endif

#ifndef _SVDDEF_HXX //autogen
#include <bf_svx/svddef.hxx>
#endif
namespace binfilter {

// Anzahl der Durchlaeufe. 0=Endlos.
class SdrTextAniCountItem: public SfxUInt16Item {
public:
	SdrTextAniCountItem(UINT16 nVal=0): SfxUInt16Item(SDRATTR_TEXT_ANICOUNT,nVal) {}
	SdrTextAniCountItem(SvStream& rIn): SfxUInt16Item(SDRATTR_TEXT_ANICOUNT,rIn)  {}
};

}//end of namespace binfilter
#endif
