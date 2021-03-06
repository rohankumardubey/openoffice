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


#ifndef _WRTASC_HXX
#define _WRTASC_HXX

#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _WRT_FN_HXX
#include <wrt_fn.hxx>
#endif
namespace binfilter {

extern SwNodeFnTab aASCNodeFnTab;


// der ASC-Writer

class SwASCWriter : public Writer
{
	String sLineEnd;

	virtual ULONG WriteStream();

public:
	SwASCWriter( const String& rFilterName );
	virtual ~SwASCWriter();

	const String& GetLineEnd() const      { return sLineEnd; }
};


} //namespace binfilter
#endif	//  _WRTASC_HXX
