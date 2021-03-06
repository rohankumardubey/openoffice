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



#ifndef _SVX_UNOVIWED_HXX
#define _SVX_UNOVIWED_HXX

#include <bf_svtools/bf_solar.h>

#ifndef _SVX_UNOEDSRC_HXX
#include <bf_svx/unoedsrc.hxx>
#endif

#include <bf_svx/editdata.hxx>
namespace binfilter {

class EditView;

/// Specialization for Calc
class SvxEditEngineViewForwarder : public SvxEditViewForwarder
{
private:
	EditView&			mrView;

public:
						SvxEditEngineViewForwarder( EditView& rView );
	virtual				~SvxEditEngineViewForwarder();

	virtual BOOL		IsValid() const;

    virtual Rectangle	GetVisArea() const;
    virtual Point		LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const;
    virtual Point		PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const;

    virtual sal_Bool	GetSelection( ESelection& rSelection ) const;
    virtual sal_Bool	SetSelection( const ESelection& rSelection );
    virtual sal_Bool	Copy();
    virtual sal_Bool	Cut();
    virtual sal_Bool	Paste();

};

}//end of namespace binfilter
#endif

