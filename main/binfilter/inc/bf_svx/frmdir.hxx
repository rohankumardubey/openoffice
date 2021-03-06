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



#ifndef _SVX_FRMDIR_HXX
#define _SVX_FRMDIR_HXX

// ----------------------------------------------------------------------------
namespace binfilter {

/** Defines possible text directions in frames. */
enum SvxFrameDirection
{
    /** Horizontal, from left to right, from top to bottom
        (typical for western languages). */
    FRMDIR_HORI_LEFT_TOP,

    /** Horizontal, from right to left, from top to bottom
        (typical for ararbic/hebrew languages). */
    FRMDIR_HORI_RIGHT_TOP,

    /** Vertical, from top to bottom, from right to left
        (typical for asian languages). */
    FRMDIR_VERT_TOP_RIGHT,

    /** Vertical, from top to bottom, from left to right
        (typical for mongol language). */
    FRMDIR_VERT_TOP_LEFT,

    /** Use the value from the environment, can only be used in frames. */
    FRMDIR_ENVIRONMENT
};

// ----------------------------------------------------------------------------

}//end of namespace binfilter
#endif // #ifndef _SVX_FRMDIR_HXX

