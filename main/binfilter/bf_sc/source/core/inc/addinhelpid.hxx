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



#ifndef SC_ADDINHELPID_HXX
#define SC_ADDINHELPID_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
namespace binfilter {


// ============================================================================

struct ScUnoAddInHelpId;

/** Generates help IDs for standard Calc AddIns. */
class ScUnoAddInHelpIdGenerator
{
private:
    const ScUnoAddInHelpId*     pCurrHelpIds;       /// Array of function names and help IDs.
    sal_uInt32                  nArrayCount;        /// Count of array entries.

public:
                                ScUnoAddInHelpIdGenerator( const ::rtl::OUString& rServiceName );

    /** Sets service name of the AddIn. Has to be done before requesting help IDs. */
    void                        SetServiceName( const ::rtl::OUString& rServiceName );

    /** @return  The help ID of the function with given built-in name or 0 if not found. */
    sal_uInt16                  GetHelpId( const ::rtl::OUString& rFuncName ) const;
};


// ============================================================================

} //namespace binfilter
#endif

