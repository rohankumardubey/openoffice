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
#include "precompiled_sw.hxx"


#include <tools/stream.hxx>
#include <unotools/syslocale.hxx>

#include "swtypes.hxx"
#include "hintids.hxx"
#include "uitool.hxx"
#include "usrpref.hxx"
#include "crstate.hxx"
#include <linguistic/lngprops.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <unotools/localedatawrapper.hxx>

#include <unomid.h>

using namespace utl;
using namespace rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


void SwMasterUsrPref::SetUsrPref(const SwViewOption &rCopy)
{
	*((SwViewOption*)this) = rCopy;
}

SwMasterUsrPref::SwMasterUsrPref(sal_Bool bWeb) :
    eFldUpdateFlags(AUTOUPD_OFF),
    nLinkUpdateMode(0),
    bIsHScrollMetricSet(sal_False),
    bIsVScrollMetricSet(sal_False),
    nDefTab( MM50 * 4 ),
    bIsSquaredPageMode(sal_False),
    aContentConfig(bWeb, *this),
    aLayoutConfig(bWeb, *this),
    aGridConfig(bWeb, *this),
    aCursorConfig(*this),
    pWebColorConfig(bWeb ? new SwWebColorConfig(*this) : 0)
{
    MeasurementSystem eSystem = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
    eUserMetric = MEASURE_METRIC == eSystem ? FUNIT_CM : FUNIT_INCH;
    eHScrollMetric = eUserMetric;
    eVScrollMetric = eUserMetric;

    aContentConfig.Load();
	aLayoutConfig.Load();
	aGridConfig.Load();
	aCursorConfig.Load();
	if(pWebColorConfig)
		pWebColorConfig->Load();
}
/* -----------------------------13.02.01 09:48--------------------------------

 ---------------------------------------------------------------------------*/
SwMasterUsrPref::~SwMasterUsrPref()
{
	delete pWebColorConfig;
}
/*-- 28.09.00 09:55:32---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence<OUString> SwContentViewConfig::GetPropertyNames()
{
	static const char* aPropNames[] =
	{
		"Display/GraphicObject",   					// 	0
		"Display/Table",							//  1
		"Display/DrawingControl",  					//  2
		"Display/FieldCode",	   					//  3
		"Display/Note",								//  4
        "Display/PreventTips",                      //  5
        "NonprintingCharacter/MetaCharacters",     //   6
        "NonprintingCharacter/ParagraphEnd",        //  7
        "NonprintingCharacter/OptionalHyphen",      //  8
        "NonprintingCharacter/Space",               //  9
        "NonprintingCharacter/Break",               // 10
        "NonprintingCharacter/ProtectedSpace",      // 11
            "NonprintingCharacter/Tab",             // 12 //not in Writer/Web
            "NonprintingCharacter/HiddenText",      // 13
            "NonprintingCharacter/HiddenParagraph", // 14
            "NonprintingCharacter/HiddenCharacter",      // 15
            "Update/Link",                          // 16
            "Update/Field",                         // 17
            "Update/Chart"                          // 18


	};
    const int nCount = bWeb ? 12 : 19;
	Sequence<OUString> aNames(nCount);
	OUString* pNames = aNames.getArray();
	for(int i = 0; i < nCount; i++)
	{
		pNames[i] = OUString::createFromAscii(aPropNames[i]);
	}
	return aNames;
}
/*-- 28.09.00 09:55:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwContentViewConfig::SwContentViewConfig(sal_Bool bIsWeb, SwMasterUsrPref& rPar) :
    ConfigItem(bIsWeb ? C2U("Office.WriterWeb/Content") :  C2U("Office.Writer/Content")),
	rParent(rPar),
	bWeb(bIsWeb)
{
	Load();
    EnableNotification( GetPropertyNames() );
}
/*-- 28.09.00 09:55:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwContentViewConfig::~SwContentViewConfig()
{
}
/*-- 09.02.07 09:55:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwContentViewConfig::Notify( const Sequence< OUString > & /*rPropertyNames*/ )
{
    Load();
}
/*-- 28.09.00 09:55:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwContentViewConfig::Commit()
{
	Sequence<OUString> aNames = GetPropertyNames();

	Sequence<Any> aValues(aNames.getLength());
	Any* pValues = aValues.getArray();

	for(int nProp = 0; nProp < aNames.getLength(); nProp++)
	{
		sal_Bool bVal = sal_False;
		switch(nProp)
		{
			case  0: bVal = rParent.IsGraphic();	break;// "Display/GraphicObject",
			case  1: bVal = rParent.IsTable();	break;// "Display/Table",
			case  2: bVal = rParent.IsDraw();    	break;// "Display/DrawingControl",
			case  3: bVal = rParent.IsFldName(); 	break;// "Display/FieldCode",
			case  4: bVal = rParent.IsPostIts(); 	break;// "Display/Note",
            case  5: bVal = rParent.IsPreventTips(); break; // "Display/PreventTips"
            case  6: bVal = rParent.IsViewMetaChars(); break; //"NonprintingCharacter/MetaCharacters"
            case  7: bVal = rParent.IsParagraph(sal_True); break;// "NonprintingCharacter/ParagraphEnd",
            case  8: bVal = rParent.IsSoftHyph(); break;// "NonprintingCharacter/OptionalHyphen",
            case  9: bVal = rParent.IsBlank(sal_True);  break;// "NonprintingCharacter/Space",
            case 10: bVal = rParent.IsLineBreak(sal_True);break;// "NonprintingCharacter/Break",
            case 11: bVal = rParent.IsHardBlank(); break;// "NonprintingCharacter/ProtectedSpace",
            case 12: bVal = rParent.IsTab(sal_True);        break;// "NonprintingCharacter/Tab",
            case 13: bVal = rParent.IsShowHiddenField(); break;// "NonprintingCharacter/Fields: HiddenText",
            case 14: bVal = rParent.IsShowHiddenPara(); break;// "NonprintingCharacter/Fields: HiddenParagraph",
            case 15: bVal = rParent.IsShowHiddenChar(sal_True);    break;// "NonprintingCharacter/HiddenCharacter",
            case 16: pValues[nProp] <<= rParent.GetUpdateLinkMode();    break;// "Update/Link",
            case 17: bVal = rParent.IsUpdateFields(); break;// "Update/Field",
            case 18: bVal = rParent.IsUpdateCharts(); break;// "Update/Chart"
        }
        if(nProp != 16)
			pValues[nProp].setValue(&bVal, ::getBooleanCppuType());
	}
	PutProperties(aNames, aValues);
}
/*-- 28.09.00 09:55:34---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwContentViewConfig::Load()
{
	Sequence<OUString> aNames = GetPropertyNames();
	Sequence<Any> aValues = GetProperties(aNames);
	const Any* pValues = aValues.getConstArray();
	DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
	if(aValues.getLength() == aNames.getLength())
	{
		for(int nProp = 0; nProp < aNames.getLength(); nProp++)
		{
			if(pValues[nProp].hasValue())
			{
                sal_Bool bSet = nProp != 16 ? *(sal_Bool*)pValues[nProp].getValue() : sal_False;
				switch(nProp)
				{
					case  0: rParent.SetGraphic(bSet);	break;// "Display/GraphicObject",
					case  1: rParent.SetTable(bSet);	break;// "Display/Table",
					case  2: rParent.SetDraw(bSet);    	break;// "Display/DrawingControl",
					case  3: rParent.SetFldName(bSet); 	break;// "Display/FieldCode",
					case  4: rParent.SetPostIts(bSet); 	break;// "Display/Note",
                    case  5: rParent.SetPreventTips(bSet);  break;// "Display/PreventTips",
                    case  6: rParent.SetViewMetaChars(bSet); break; //"NonprintingCharacter/MetaCharacters"
                    case  7: rParent.SetParagraph(bSet); break;// "NonprintingCharacter/ParagraphEnd",
                    case  8: rParent.SetSoftHyph(bSet); break;// "NonprintingCharacter/OptionalHyphen",
                    case  9: rParent.SetBlank(bSet);    break;// "NonprintingCharacter/Space",
                    case 10: rParent.SetLineBreak(bSet);break;// "NonprintingCharacter/Break",
                    case 11: rParent.SetHardBlank(bSet); break;// "NonprintingCharacter/ProtectedSpace",
                    case 12: rParent.SetTab(bSet);      break;// "NonprintingCharacter/Tab",
                    case 13: rParent.SetShowHiddenField(bSet);   break;// "NonprintingCharacter/Fields: HiddenText",
                    case 14: rParent.SetShowHiddenPara(bSet); break;// "NonprintingCharacter/Fields: HiddenParagraph",
                    case 15: rParent.SetShowHiddenChar(bSet); break;// "NonprintingCharacter/HiddenCharacter",
                    case 16:
                    {
						sal_Int32 nSet = 0;
						pValues[nProp] >>= nSet;
                        rParent.SetUpdateLinkMode(nSet, sal_True);
					}
					break;// "Update/Link",
                    case 17: rParent.SetUpdateFields(bSet, sal_True); break;// "Update/Field",
                    case 18: rParent.SetUpdateCharts(bSet, sal_True); break;// "Update/Chart"
				}
			}
		}
	}
}
/*-- 28.09.00 09:55:34---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence<OUString> SwLayoutViewConfig::GetPropertyNames()
{
	static const char* aPropNames[] =
	{
        "Line/Guide",                           // 0
        "Line/SimpleControlPoint",              // 1
        "Line/LargeControlPoint",               // 2
        "Window/HorizontalScroll",              // 3
        "Window/VerticalScroll",                // 4
        "Window/ShowRulers",                    // 5
        "Window/HorizontalRuler",               // 6
        "Window/VerticalRuler",                 // 7
        "Window/HorizontalRulerUnit",           // 8
        "Window/VerticalRulerUnit",             // 9
        "Window/SmoothScroll",                  //10
        "Zoom/Value",                           //11
        "Zoom/Type",                            //12
        "Other/IsAlignMathObjectsToBaseline",   //13
        "Other/MeasureUnit",                    //14
        // below properties are not available in WriterWeb
        "Other/TabStop",                        //15
        "Window/IsVerticalRulerRight",          //16
        "ViewLayout/Columns",                   //17
        "ViewLayout/BookMode",                  //18
        "Other/IsSquaredPageMode"               //19
    };
    const int nCount = bWeb ? 15 : 20;
    Sequence<OUString> aNames(nCount);
	OUString* pNames = aNames.getArray();
	for(int i = 0; i < nCount; i++)
	{
		pNames[i] = OUString::createFromAscii(aPropNames[i]);
	}
	return aNames;
}
/*-- 28.09.00 09:55:34---------------------------------------------------

  -----------------------------------------------------------------------*/
SwLayoutViewConfig::SwLayoutViewConfig(sal_Bool bIsWeb, SwMasterUsrPref& rPar) :
    ConfigItem(bIsWeb ? C2U("Office.WriterWeb/Layout") :  C2U("Office.Writer/Layout"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
	rParent(rPar),
	bWeb(bIsWeb)
{
}
/*-- 28.09.00 09:55:35---------------------------------------------------

  -----------------------------------------------------------------------*/
SwLayoutViewConfig::~SwLayoutViewConfig()
{
}
/*-- 28.09.00 09:55:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwLayoutViewConfig::Commit()
{
	Sequence<OUString> aNames = GetPropertyNames();

	Sequence<Any> aValues(aNames.getLength());
	Any* pValues = aValues.getArray();

	for(int nProp = 0; nProp < aNames.getLength(); nProp++)
	{
        Any &rVal = pValues[nProp];
		switch(nProp)
		{
            case  0: rVal <<= (sal_Bool) rParent.IsCrossHair(); break;              // "Line/Guide",
            case  1: rVal <<= (sal_Bool) rParent.IsSolidMarkHdl(); break;           // "Line/SimpleControlPoint",
            case  2: rVal <<= (sal_Bool) rParent.IsBigMarkHdl(); break;             // "Line/LargeControlPoint",
            case  3: rVal <<= (sal_Bool) rParent.IsViewHScrollBar(); break;         // "Window/HorizontalScroll",
            case  4: rVal <<= (sal_Bool) rParent.IsViewVScrollBar(); break;         // "Window/VerticalScroll",
            case  5: rVal <<= (sal_Bool) rParent.IsViewAnyRuler(); break;           // "Window/ShowRulers"
            // #i14593# use IsView*Ruler(sal_True) instead of IsView*Ruler()
            // this preserves the single ruler states even if "Window/ShowRulers" is off
            case  6: rVal <<= (sal_Bool) rParent.IsViewHRuler(sal_True); break;         // "Window/HorizontalRuler",
            case  7: rVal <<= (sal_Bool) rParent.IsViewVRuler(sal_True); break;         // "Window/VerticalRuler",
            case  8:
                if(rParent.bIsHScrollMetricSet)
                    rVal <<= (sal_Int32)rParent.eHScrollMetric;                     // "Window/HorizontalRulerUnit"
            break;
            case  9:
                if(rParent.bIsVScrollMetricSet)
                    rVal <<= (sal_Int32)rParent.eVScrollMetric;                     // "Window/VerticalRulerUnit"
            break;
            case 10: rVal <<= (sal_Bool) rParent.IsSmoothScroll(); break;           // "Window/SmoothScroll",
            case 11: rVal <<= (sal_Int32)rParent.GetZoom(); break;                  // "Zoom/Value",
            case 12: rVal <<= (sal_Int32)rParent.GetZoomType(); break;              // "Zoom/Type",
            case 13: rVal <<= (sal_Bool) rParent.IsAlignMathObjectsToBaseline(); break;      // "Other/IsAlignMathObjectsToBaseline"
            case 14: rVal <<= (sal_Int32)rParent.GetMetric(); break;                // "Other/MeasureUnit",
            case 15: rVal <<= static_cast<sal_Int32>(TWIP_TO_MM100(rParent.GetDefTab())); break;// "Other/TabStop",
            case 16: rVal <<= (sal_Bool) rParent.IsVRulerRight(); break;            // "Window/IsVerticalRulerRight",
            case 17: rVal <<= (sal_Int32)rParent.GetViewLayoutColumns(); break;     // "ViewLayout/Columns",
            case 18: rVal <<= (sal_Bool) rParent.IsViewLayoutBookMode(); break;     // "ViewLayout/BookMode",
            case 19: rVal <<= (sal_Bool) rParent.IsSquaredPageMode(); break;        // "Other/IsSquaredPageMode",
        }
	}
	PutProperties(aNames, aValues);
}
/*-- 28.09.00 09:55:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwLayoutViewConfig::Load()
{
	Sequence<OUString> aNames = GetPropertyNames();
	Sequence<Any> aValues = GetProperties(aNames);
	const Any* pValues = aValues.getConstArray();
	DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
	if(aValues.getLength() == aNames.getLength())
	{
		for(int nProp = 0; nProp < aNames.getLength(); nProp++)
		{
			if(pValues[nProp].hasValue())
			{
                sal_Int32   nInt32Val   = 0;
                sal_Bool    bSet        = sal_False;
                pValues[nProp] >>= nInt32Val;
                pValues[nProp] >>= bSet;

                switch(nProp)
				{
                    case  0: rParent.SetCrossHair(bSet); break;// "Line/Guide",
                    case  1: rParent.SetSolidMarkHdl(bSet); break;// "Line/SimpleControlPoint",
                    case  2: rParent.SetBigMarkHdl(bSet); break;// "Line/LargeControlPoint",
                    case  3: rParent.SetViewHScrollBar(bSet); break;// "Window/HorizontalScroll",
                    case  4: rParent.SetViewVScrollBar(bSet); break;// "Window/VerticalScroll",
                    case  5: rParent.SetViewAnyRuler(bSet);break; // "Window/ShowRulers"
                    case  6: rParent.SetViewHRuler(bSet); break;// "Window/HorizontalRuler",
                    case  7: rParent.SetViewVRuler(bSet); break;// "Window/VerticalRuler",
                    case  8:
                    {
                        rParent.bIsHScrollMetricSet = sal_True;
                        rParent.eHScrollMetric = ((FieldUnit)nInt32Val);  // "Window/HorizontalRulerUnit"
                    }
                    break;
                    case  9:
                    {
                        rParent.bIsVScrollMetricSet = sal_True;
                        rParent.eVScrollMetric = ((FieldUnit)nInt32Val); // "Window/VerticalRulerUnit"
                    }
                    break;
                    case 10: rParent.SetSmoothScroll(bSet); break;// "Window/SmoothScroll",
                    case 11: rParent.SetZoom( static_cast< sal_uInt16 >(nInt32Val) ); break;// "Zoom/Value",
                    case 12: rParent.SetZoomType( static_cast< SvxZoomType >(nInt32Val) ); break;// "Zoom/Type",
                    case 13: rParent.SetAlignMathObjectsToBaseline(bSet); break;// "Other/IsAlignMathObjectsToBaseline"
                    case 14: rParent.SetMetric((FieldUnit)nInt32Val, sal_True); break;// "Other/MeasureUnit",
                    case 15: rParent.SetDefTab(MM100_TO_TWIP(nInt32Val), sal_True); break;// "Other/TabStop",
                    case 16: rParent.SetVRulerRight(bSet); break;// "Window/IsVerticalRulerRight",
                    case 17: rParent.SetViewLayoutColumns( static_cast<sal_uInt16>(nInt32Val) ); break;// "ViewLayout/Columns",
                    case 18: rParent.SetViewLayoutBookMode(bSet); break;// "ViewLayout/BookMode",
                    case 19: rParent.SetDefaultPageMode(bSet,sal_True); break;// "Other/IsSquaredPageMode",
                }
			}
		}
	}
}

void SwLayoutViewConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
Sequence<OUString> SwGridConfig::GetPropertyNames()
{
	static const char* aPropNames[] =
	{
		"Option/SnapToGrid",			// 0
		"Option/VisibleGrid",			// 1
		"Option/Synchronize",			// 2
		"Resolution/XAxis",				// 3
		"Resolution/YAxis",				// 4
		"Subdivision/XAxis",			// 5
		"Subdivision/YAxis"				// 6
	};
	const int nCount = 7;
	Sequence<OUString> aNames(nCount);
	OUString* pNames = aNames.getArray();
	for(int i = 0; i < nCount; i++)
	{
		pNames[i] = OUString::createFromAscii(aPropNames[i]);
	}
	return aNames;
}
/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
SwGridConfig::SwGridConfig(sal_Bool bIsWeb, SwMasterUsrPref& rPar) :
    ConfigItem(bIsWeb ? C2U("Office.WriterWeb/Grid") :  C2U("Office.Writer/Grid"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
	rParent(rPar),
	bWeb(bIsWeb)
{
}
/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
SwGridConfig::~SwGridConfig()
{
}
/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
void SwGridConfig::Commit()
{
	Sequence<OUString> aNames = GetPropertyNames();

	Sequence<Any> aValues(aNames.getLength());
	Any* pValues = aValues.getArray();

	for(int nProp = 0; nProp < aNames.getLength(); nProp++)
	{
		sal_Bool bSet;
		switch(nProp)
		{
			case  0: bSet = rParent.IsSnap(); break;// 		"Option/SnapToGrid",
			case  1: bSet = rParent.IsGridVisible(); break;//"Option/VisibleGrid",
			case  2: bSet = rParent.IsSynchronize(); break;//  "Option/Synchronize",
			case  3: pValues[nProp] <<= (sal_Int32)TWIP_TO_MM100(rParent.GetSnapSize().Width()); break;//      "Resolution/XAxis",
			case  4: pValues[nProp] <<= (sal_Int32)TWIP_TO_MM100(rParent.GetSnapSize().Height()); break;//      "Resolution/YAxis",
			case  5: pValues[nProp] <<= (sal_Int16)rParent.GetDivisionX(); break;//   "Subdivision/XAxis",
			case  6: pValues[nProp] <<= (sal_Int16)rParent.GetDivisionY(); break;//   "Subdivision/YAxis"
		}
		if(nProp < 3)
  			pValues[nProp].setValue(&bSet, ::getBooleanCppuType());
	}
	PutProperties(aNames, aValues);
}
/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
void SwGridConfig::Load()
{
	Sequence<OUString> aNames = GetPropertyNames();
	Sequence<Any> aValues = GetProperties(aNames);
	const Any* pValues = aValues.getConstArray();
	DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
	if(aValues.getLength() == aNames.getLength())
	{
		Size aSnap(rParent.GetSnapSize());
		for(int nProp = 0; nProp < aNames.getLength(); nProp++)
		{
			if(pValues[nProp].hasValue())
			{
				sal_Bool bSet = nProp < 3 ? *(sal_Bool*)pValues[nProp].getValue() : sal_False;
                sal_Int32 nSet = 0;
				if(nProp >= 3)
					pValues[nProp] >>= nSet;
				switch(nProp)
				{
					case  0: rParent.SetSnap(bSet); break;// 		"Option/SnapToGrid",
					case  1: rParent.SetGridVisible(bSet); break;//"Option/VisibleGrid",
					case  2: rParent.SetSynchronize(bSet); break;//  "Option/Synchronize",
					case  3: aSnap.Width() = MM100_TO_TWIP(nSet); break;//      "Resolution/XAxis",
					case  4: aSnap.Height() = MM100_TO_TWIP(nSet); break;//      "Resolution/YAxis",
                    case  5: rParent.SetDivisionX((short)nSet); break;//   "Subdivision/XAxis",
                    case  6: rParent.SetDivisionY((short)nSet); break;//   "Subdivision/YAxis"
				}
			}
		}
		rParent.SetSnapSize(aSnap);
	}
}

void SwGridConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
Sequence<OUString> SwCursorConfig::GetPropertyNames()
{
	static const char* aPropNames[] =
	{
		"DirectCursor/UseDirectCursor",	// 0
		"DirectCursor/Insert",			// 1
        "Option/ProtectedArea"          // 2
	};
    const int nCount = 3;
	Sequence<OUString> aNames(nCount);
	OUString* pNames = aNames.getArray();
	for(int i = 0; i < nCount; i++)
		pNames[i] = C2U(aPropNames[i]);
	return aNames;
}
/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
SwCursorConfig::SwCursorConfig(SwMasterUsrPref& rPar) :
    ConfigItem(C2U("Office.Writer/Cursor"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
	rParent(rPar)
{
}
/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
SwCursorConfig::~SwCursorConfig()
{
}
/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
void SwCursorConfig::Commit()
{
	Sequence<OUString> aNames = GetPropertyNames();

	Sequence<Any> aValues(aNames.getLength());
	Any* pValues = aValues.getArray();

	for(int nProp = 0; nProp < aNames.getLength(); nProp++)
	{
		sal_Bool bSet;
		switch(nProp)
		{
			case  0: bSet = rParent.IsShadowCursor(); 		break;//  "DirectCursor/UseDirectCursor",
			case  1: pValues[nProp] <<= (sal_Int32)rParent.GetShdwCrsrFillMode(); 	break;//  "DirectCursor/Insert",
            case  2: bSet = rParent.IsCursorInProtectedArea(); break;// "Option/ProtectedArea"
		}
        if(nProp != 1 )
  			pValues[nProp].setValue(&bSet, ::getBooleanCppuType());
	}
	PutProperties(aNames, aValues);
}
/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
void SwCursorConfig::Load()
{
	Sequence<OUString> aNames = GetPropertyNames();
	Sequence<Any> aValues = GetProperties(aNames);
	const Any* pValues = aValues.getConstArray();
	DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
	if(aValues.getLength() == aNames.getLength())
	{

		for(int nProp = 0; nProp < aNames.getLength(); nProp++)
		{
			if(pValues[nProp].hasValue())
			{
                sal_Bool bSet = sal_False;
                sal_Int32 nSet = 0;
                if(nProp != 1 )
					bSet = *(sal_Bool*)pValues[nProp].getValue();
				else
					pValues[nProp] >>= nSet;
				switch(nProp)
				{
					case  0: rParent.SetShadowCursor(bSet); 		break;//  "DirectCursor/UseDirectCursor",
                    case  1: rParent.SetShdwCrsrFillMode((sal_uInt8)nSet); break;//  "DirectCursor/Insert",
                    case  2: rParent.SetCursorInProtectedArea(bSet); break;// "Option/ProtectedArea"
				}
			}
		}

	}
}

void SwCursorConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

/*-- 28.09.00 09:55:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwWebColorConfig::SwWebColorConfig(SwMasterUsrPref& rPar) :
    ConfigItem(C2U("Office.WriterWeb/Background"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
	rParent(rPar),
	aPropNames(1)
{
	aPropNames.getArray()[0] = C2U("Color");
}
/*-- 28.09.00 09:55:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwWebColorConfig::~SwWebColorConfig()
{
}
/*-- 28.09.00 09:55:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwWebColorConfig::Commit()
{
	Sequence<Any> aValues(aPropNames.getLength());
	Any* pValues = aValues.getArray();
	for(int nProp = 0; nProp < aPropNames.getLength(); nProp++)
	{
		switch(nProp)
		{
			case  0: pValues[nProp] <<= (sal_Int32)rParent.GetRetoucheColor().GetColor();	break;// "Color",
		}
	}
	PutProperties(aPropNames, aValues);
}

void SwWebColorConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

/*-- 28.09.00 09:55:34---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwWebColorConfig::Load()
{
	Sequence<Any> aValues = GetProperties(aPropNames);
	const Any* pValues = aValues.getConstArray();
	DBG_ASSERT(aValues.getLength() == aPropNames.getLength(), "GetProperties failed");
	if(aValues.getLength() == aPropNames.getLength())
	{
		for(int nProp = 0; nProp < aPropNames.getLength(); nProp++)
		{
			if(pValues[nProp].hasValue())
			{
				switch(nProp)
				{
					case  0:
						sal_Int32 nSet = 0;
						pValues[nProp] >>= nSet; rParent.SetRetoucheColor(nSet);
					break;// "Color",
				}
			}
		}
	}
}


