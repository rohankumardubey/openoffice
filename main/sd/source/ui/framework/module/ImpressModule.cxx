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



#include "precompiled_sd.hxx"

#include "framework/ImpressModule.hxx"

#include "framework/FrameworkHelper.hxx"
#include "ViewTabBarModule.hxx"
#include "CenterViewFocusModule.hxx"
#include "SlideSorterModule.hxx"
#include "ToolPanelModule.hxx"
#include "ToolBarModule.hxx"
#include "ShellStackGuard.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd { namespace framework {


void ImpressModule::Initialize (Reference<frame::XController>& rxController)
{
    new CenterViewFocusModule(rxController);
    new ViewTabBarModule(
        rxController,
        FrameworkHelper::CreateResourceId(
            FrameworkHelper::msViewTabBarURL,
            FrameworkHelper::msCenterPaneURL));
    new SlideSorterModule(
        rxController,
        FrameworkHelper::msLeftImpressPaneURL);
    ToolPanelModule::Initialize(rxController);
    new ToolBarModule(rxController);
    new ShellStackGuard(rxController);
}


} } // end of namespace sd::framework
