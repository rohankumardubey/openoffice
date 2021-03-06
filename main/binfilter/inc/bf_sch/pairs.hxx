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


#ifndef _SVX_SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif

#ifndef _SFXDEFS_HXX //autogen
#include <bf_sfx2/sfxdefs.hxx>
#endif
#ifndef _XDEF_HXX //autogen
#include <bf_svx/xdef.hxx>
#endif
#ifndef _SVDDEF_HXX //autogen
#include <bf_svx/svddef.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <bf_svx/eeitem.hxx>
#endif

#ifndef _SFXMODULE_HXX //autogen
#include <bf_sfx2/module.hxx>
#endif
#ifndef _TOOLS_RESMGR_HXX //autogen
#include <tools/resmgr.hxx>
#endif



#include "schattr.hxx"
#include "app.hrc" // z.Z. fuer SID_TEXTBREAK
namespace binfilter {


const USHORT nTitleWhichPairs[] =
{
	SCHATTR_TEXT_ORIENT, SCHATTR_TEXT_ORIENT,       //     4          sch/schattr.hxx
	SCHATTR_TEXT_DEGREES,SCHATTR_TEXT_DEGREES,      //    53          sch/schattr.hxx
    SCHATTR_USER_DEFINED_ATTR, SCHATTR_USER_DEFINED_ATTR, //     100  sch/schattr.hxx
	XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  bf_svx/xdef.hxx
	XATTR_FILL_FIRST, XATTR_FILL_LAST,              //  1018 -  1046  bf_svx/xdef.hxx
	SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,      //  1067 -  1078  bf_svx/svddef.hxx
	EE_ITEMS_START, EE_ITEMS_END,                   //  3994 -  4037  bf_svx/eeitem.hxx
	0
};

const USHORT nAxisWhichPairs[] =
{
	XATTR_LINE_FIRST, XATTR_LINE_LAST,							//  1000 -  1016  bf_svx/xdef.hxx
	EE_ITEMS_START, EE_ITEMS_END,								//  3994 -  4037  bf_svx/eeitem.hxx
	SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_VALUE,	// 10585 - 10585  bf_svx/svxids.hrc
	SID_ATTR_NUMBERFORMAT_SOURCE, SID_ATTR_NUMBERFORMAT_SOURCE, // 11432          bf_svx/svxids.hrc
	SCHATTR_AXISTYPE, SCHATTR_AXISTYPE,							//    39          sch/schattr.hxx
	SCHATTR_TEXT_START, SCHATTR_TEXT_END,						//     4 -     6  sch/schattr.hxx
	SCHATTR_TEXT_DEGREES,SCHATTR_TEXT_DEGREES,      			//    53          sch/schattr.hxx
	SCHATTR_TEXT_OVERLAP, SCHATTR_TEXT_OVERLAP,					//    54          sch/schattr.hxx
	SCHATTR_AXIS_START, SCHATTR_AXIS_END,						//    70 -    95  sch/schattr.hxx
    SCHATTR_USER_DEFINED_ATTR, SCHATTR_USER_DEFINED_ATTR,       //   100          sch/schattr.hxx
	SID_TEXTBREAK, SID_TEXTBREAK,								// 30587          sch/app.hrc
	0
};
const USHORT nAllAxisWhichPairs[] =
{
	XATTR_LINE_FIRST, XATTR_LINE_LAST,				//  1000 -  1016  bf_svx/xdef.hxx
	EE_ITEMS_START, EE_ITEMS_END,					//  3994 -  4037  bf_svx/eeitem.hxx
	SCHATTR_TEXT_ORIENT, SCHATTR_TEXT_ORIENT,       //    4           sch/schattr.hxx
	SCHATTR_TEXT_DEGREES,SCHATTR_TEXT_DEGREES,		//    53          sch/schattr.hxx
	SCHATTR_TEXT_OVERLAP, SCHATTR_TEXT_OVERLAP,		//    54          sch/schattr.hxx
	SCHATTR_AXIS_SHOWDESCR, SCHATTR_AXIS_SHOWDESCR,	//    85          sch/schattr.hxx
	SID_TEXTBREAK, SID_TEXTBREAK,					// 30587          sch/app.hrc
	0
};

const USHORT nGridWhichPairs[] =
{
	XATTR_LINE_FIRST, XATTR_LINE_LAST,				//  1000 -  1016  bf_svx/xdef.hxx
    SCHATTR_USER_DEFINED_ATTR, SCHATTR_USER_DEFINED_ATTR, //     100  sch/schattr.hxx
    0
};

const USHORT nChartWhichPairs[] =
{
	SCHATTR_STYLE_START,SCHATTR_STYLE_END,			//    59 -    68  sch/schattr.hxx
    SCHATTR_USER_DEFINED_ATTR, SCHATTR_USER_DEFINED_ATTR, //     100  sch/schattr.hxx
	0
};
const USHORT nDiagramAreaWhichPairs[] =
{
	XATTR_LINE_FIRST, XATTR_LINE_LAST,				//  1000 -  1016  bf_svx/xdef.hxx
	XATTR_FILL_FIRST, XATTR_FILL_LAST,				//  1018 -  1046  bf_svx/xdef.hxx
    SCHATTR_USER_DEFINED_ATTR, SCHATTR_USER_DEFINED_ATTR, //     100  sch/schattr.hxx
	0
};
const USHORT nAreaAndChartWhichPairs[] = //Diese Pairs umfassen Chart und Area-Whichpairs!!!
{
	XATTR_LINE_FIRST, XATTR_LINE_LAST,				//  1000 -  1016  bf_svx/xdef.hxx
	XATTR_FILL_FIRST, XATTR_FILL_LAST,				//  1018 -  1046  bf_svx/xdef.hxx
	SCHATTR_STYLE_START,SCHATTR_STYLE_END,			//    59 -    68  sch/schattr.hxx
    SCHATTR_USER_DEFINED_ATTR, SCHATTR_USER_DEFINED_ATTR, //     100  sch/schattr.hxx
	0
};
const USHORT nLegendWhichPairs[] =
{
	XATTR_LINE_FIRST, XATTR_LINE_LAST,				//  1000 -  1016  bf_svx/xdef.hxx
	XATTR_FILL_FIRST, XATTR_FILL_LAST,				//  1018 -  1046  bf_svx/xdef.hxx
	SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,      //  1067 -  1078  bf_svx/svddef.hxx
	EE_ITEMS_START, EE_ITEMS_END,					//  3994 -  4037  bf_svx/eeitem.hxx
	SCHATTR_LEGEND_START, SCHATTR_LEGEND_END,		//     3 -     3  sch/schattr.hxx
    SCHATTR_USER_DEFINED_ATTR, SCHATTR_USER_DEFINED_ATTR, //     100  sch/schattr.hxx
	0
};

#define CHART_ROW_WHICHPAIRS 	\
	XATTR_LINE_FIRST, XATTR_LINE_LAST,				/*  1000 -  1016  bf_svx/xdef.hxx	 */	\
	XATTR_FILL_FIRST, XATTR_FILL_LAST,				/*  1018 -  1046  bf_svx/xdef.hxx	 */	\
	EE_ITEMS_START, EE_ITEMS_END,					/*  3994 -  4037  bf_svx/eeitem.hxx */	\
	SCHATTR_DATADESCR_START, SCHATTR_DATADESCR_END,	/*     1 -     2  sch/schattr.hxx*/	\
    SCHATTR_DUMMY0, SCHATTR_DUMMY0,					/*	  40          sch/schattr.hxx*/	\
    SCHATTR_DUMMY1, SCHATTR_DUMMY1,					/*	  41          sch/schattr.hxx*/	\
    SCHATTR_STAT_START, SCHATTR_STAT_END,			/*    45 -    52  sch/schattr.hxx*/	\
    SCHATTR_STYLE_START,SCHATTR_STYLE_END,			/*    59 -    68  sch/schattr.hxx*/	\
	SCHATTR_AXIS,SCHATTR_AXIS,						/*    69          sch/schattr.hxx*/	\
	SCHATTR_SYMBOL_BRUSH,SCHATTR_SYMBOL_BRUSH,		/*    96          sch/schattr.hxx*/	\
	SCHATTR_SYMBOL_SIZE,SCHATTR_USER_DEFINED_ATTR,  /*    99 -   100  sch/schattr.hxx*/	\
	SDRATTR_3D_FIRST, SDRATTR_3D_LAST				/*  1244 -  1334  bf_svx/svddef.hxx */

const USHORT nRowWhichPairs[] =
{
	CHART_ROW_WHICHPAIRS,0
};

const USHORT nAreaWhichPairs[] =
{
	XATTR_LINE_FIRST, XATTR_LINE_LAST,				//  1000 -  1016  bf_svx/xdef.hxx
	XATTR_FILL_FIRST, XATTR_FILL_LAST,				//  1000 -  1016  bf_svx/xdef.hxx
	SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,		//  1067 -  1078  bf_svx/svddef.hxx
    SCHATTR_USER_DEFINED_ATTR, SCHATTR_USER_DEFINED_ATTR, //     100  sch/schattr.hxx
	0
};

const USHORT nTextWhichPairs[] =
{
	EE_ITEMS_START, EE_ITEMS_END,					//  3994 -  4037  bf_svx/eeitem.hxx
	SCHATTR_TEXT_ORIENT, SCHATTR_TEXT_ORIENT,		//     4          sch/schattr.hxx
	SCHATTR_TEXT_DEGREES,SCHATTR_TEXT_DEGREES,		//    53          sch/schattr.hxx
    SCHATTR_USER_DEFINED_ATTR, SCHATTR_USER_DEFINED_ATTR, //     100  sch/schattr.hxx
	0
};

const USHORT nTextOrientWhichPairs[] =
{
	EE_ITEMS_START, EE_ITEMS_END,					//  3994 -  4037  bf_svx/eeitem.hxx
	SCHATTR_TEXT_ORIENT, SCHATTR_TEXT_ORIENT,		//     4          sch/schattr.hxx
	SCHATTR_TEXT_DEGREES,SCHATTR_TEXT_DEGREES,		//    53          sch/schattr.hxx
	0
};

const USHORT nStatWhichPairs[]=
{
	SCHATTR_STAT_START, SCHATTR_STAT_END,			//    45 -    52  sch/schattr.hxx
		0
};


} //namespace binfilter
