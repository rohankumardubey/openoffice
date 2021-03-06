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
#include "precompiled_dbaccess.hxx"
#include "statusbarontroller.hxx"
#ifndef _DBU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif

extern "C" void SAL_CALL createRegistryInfo_OStatusbarController()
{
	static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OStatusbarController> aAutoRegistration;
}
namespace dbaui
{
	using namespace svt;
	using namespace com::sun::star::uno;
	using namespace com::sun::star::beans;
	using namespace com::sun::star::lang;
	using namespace ::com::sun::star::frame;
	using namespace ::com::sun::star::util;

	IMPLEMENT_SERVICE_INFO1_STATIC(OStatusbarController,"com.sun.star.sdb.ApplicationStatusbarController","com.sun.star.frame.StatusbarController")
	IMPLEMENT_FORWARD_XINTERFACE2(OStatusbarController,StatusbarController,OStatusbarController_BASE)
}
