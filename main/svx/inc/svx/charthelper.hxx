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

#ifndef CHARTHELPER_HXX
#define CHARTHELPER_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <basegfx/range/b2drange.hxx>
#include <svx/svxdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

namespace svt { class EmbeddedObjectRef; }

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC ChartHelper
{
public:
    // test if given reference is a chart
    static bool IsChart(const svt::EmbeddedObjectRef& xObjRef);

    // try to access rXModel in case of a chart to to get the chart content
    // as sequence of primitives. Return range of primitives (chart size) in rRange;
    // it will be used to embed the chart to the SdrObject transformation. This
    // allows to define possible distances between chart and SDrObject bounds here
    static drawinglayer::primitive2d::Primitive2DSequence tryToGetChartContentAsPrimitive2DSequence(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rXModel,
        basegfx::B2DRange& rRange);
};

//////////////////////////////////////////////////////////////////////////////

#endif //CHARTHELPER_HXX
