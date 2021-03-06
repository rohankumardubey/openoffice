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



#ifndef _SC_VIEWSETTINGSSEQUENCEDEFINES_HXX
#define _SC_VIEWSETTINGSSEQUENCEDEFINES_HXX

// this are the defines for the position of the settings in the
// ViewSettingsSequence
namespace binfilter {

#define SC_VIEWSETTINGS_COUNT				23

#define SC_VIEW_ID							0
#define SC_TABLE_VIEWSETTINGS				1
#define SC_ACTIVE_TABLE						2
#define SC_HORIZONTAL_SCROLL_BAR_WIDTH		3
#define SC_ZOOM_TYPE						4
#define SC_ZOOM_VALUE						5
#define SC_PAGE_VIEW_ZOOM_VALUE				6
#define SC_PAGE_BREAK_PREVIEW				7
#define SC_SHOWZERO							8
#define SC_SHOWNOTES						9
#define SC_SHOWGRID							10
#define SC_GRIDCOLOR						11
#define SC_SHOWPAGEBR						12
#define SC_COLROWHDR						13
#define SC_SHEETTABS						14
#define SC_OUTLSYMB							15
#define SC_SNAPTORASTER						16
#define SC_RASTERVIS						17
#define SC_RASTERRESX						18
#define SC_RASTERRESY						19
#define SC_RASTERSUBX						20
#define SC_RASTERSUBY						21
#define SC_RASTERSYNC						22


// this are the defines for the position of the settings in the
// TableViewSettingsSequence

#define SC_TABLE_VIEWSETTINGS_COUNT			11

#define SC_CURSOR_X							0
#define SC_CURSOR_Y							1
#define SC_HORIZONTAL_SPLIT_MODE			2
#define SC_VERTICAL_SPLIT_MODE				3
#define SC_HORIZONTAL_SPLIT_POSITION		4
#define SC_VERTICAL_SPLIT_POSITION			5
#define SC_ACTIVE_SPLIT_RANGE				6
#define SC_POSITION_LEFT					7
#define SC_POSITION_RIGHT					8
#define SC_POSITION_TOP						9
#define SC_POSITION_BOTTOM					10

#define SC_CURSORPOSITIONX					"CursorPositionX"
#define SC_CURSORPOSITIONY					"CursorPositionY"
#define SC_HORIZONTALSPLITMODE				"HorizontalSplitMode"
#define SC_VERTICALSPLITMODE				"VerticalSplitMode"
#define SC_HORIZONTALSPLITPOSITION			"HorizontalSplitPosition"
#define SC_VERTICALSPLITPOSITION			"VerticalSplitPosition"
#define SC_ACTIVESPLITRANGE					"ActiveSplitRange"
#define SC_POSITIONLEFT						"PositionLeft"
#define SC_POSITIONRIGHT					"PositionRight"
#define SC_POSITIONTOP						"PositionTop"
#define SC_POSITIONBOTTOM					"PositionBottom"

#define SC_TABLES							"Tables"
#define SC_ACTIVETABLE						"ActiveTable"
#define SC_HORIZONTALSCROLLBARWIDTH			"HorizontalScrollbarWidth"
#define SC_ZOOMTYPE							"ZoomType"
#define SC_ZOOMVALUE						"ZoomValue"
#define SC_PAGEVIEWZOOMVALUE				"PageViewZoomValue"
#define SC_SHOWPAGEBREAKPREVIEW					"ShowPageBreakPreview"
#define SC_VIEWID							"ViewId"
#define SC_VIEW								"View"

} //namespace binfilter
#endif
