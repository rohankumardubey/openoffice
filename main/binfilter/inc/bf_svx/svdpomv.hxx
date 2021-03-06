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



#error dieser Header entfaellt nun!

#ifndef _SVDPOMV_HXX
#define _SVDPOMV_HXX

#ifndef _SVDMRKV_HXX
#include "svdmrkv.hxx"
#endif
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@   @@@@  @@  @@  @@  @@   @@  @@@@  @@@@@  @@  @@  @@ @@ @@ @@@@@ @@   @@
//  @@  @@ @@  @@ @@  @@  @@  @@@ @@@ @@  @@ @@  @@ @@ @@   @@ @@ @@ @@    @@ @ @@
//  @@@@@  @@  @@ @@   @@@@   @@@@@@@ @@@@@@ @@@@@  @@@@    @@@@@ @@ @@@@  @@@@@@@
//  @@     @@  @@ @@    @@    @@ @ @@ @@  @@ @@  @@ @@ @@    @@@  @@ @@    @@@ @@@
//  @@      @@@@  @@@@@ @@    @@   @@ @@  @@ @@  @@ @@  @@    @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrPolyMarkView: public SdrMarkView {
private:
#ifndef _SVDRAW_HXX
	void ImpClearVars();
#endif
public:
	SdrPolyMarkView(SdrModel* pModel1, OutputDevice* pOut);
	SdrPolyMarkView(SdrModel* pModel1, ExtOutputDevice* pXOut);
	~SdrPolyMarkView();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif //_SVDPOMV_HXX

