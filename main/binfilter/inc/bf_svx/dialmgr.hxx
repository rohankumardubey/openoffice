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


#ifndef _SVX_DIALMGR_HXX
#define _SVX_DIALMGR_HXX

// include ---------------------------------------------------------------

class ResMgr;
namespace binfilter {

class GraphicFilter;
// forward ---------------------------------------------------------------


// struct DialogsResMgr --------------------------------------------------

struct DialogsResMgr
{
	DialogsResMgr();
	~DialogsResMgr();

	ResMgr*		pResMgr;

			// impl. steht impgrf.cxx !!
	GraphicFilter* GetGrfFilter_Impl();

private:
	// fuers LoadGraphic und Konsorten
	GraphicFilter* pGrapicFilter;
};

#define DIALOG_MGR()	*(*(DialogsResMgr**)GetAppData(BF_SHL_SVX))->pResMgr
#define SVX_RES(i)		ResId(i,DIALOG_MGR())
#define SVX_RESSTR(i)	UniString(ResId(i,DIALOG_MGR()))
#define SVX_RESSSTR(i)	String(ResId(i,DIALOG_MGR()))

}//end of namespace binfilter
#endif

