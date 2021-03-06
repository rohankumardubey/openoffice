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



#ifndef _XMLOFF_XMLLINENUMBERINGEXPORT_HXX_
#define _XMLOFF_XMLLINENUMBERINGEXPORT_HXX_


#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_ 
#include <com/sun/star/uno/Reference.h>
#endif
namespace binfilter {

class SvXMLExport;

/** export <text:linenumbering-configuration> and it's child elements */
class XMLLineNumberingExport 
{
	const ::rtl::OUString sCharStyleName;
	const ::rtl::OUString sCountEmptyLines;
	const ::rtl::OUString sCountLinesInFrames;
	const ::rtl::OUString sDistance;
	const ::rtl::OUString sInterval;
	const ::rtl::OUString sSeparatorText;
	const ::rtl::OUString sNumberPosition;
	const ::rtl::OUString sNumberingType;
	const ::rtl::OUString sIsOn;
	const ::rtl::OUString sRestartAtEachPage;
	const ::rtl::OUString sSeparatorInterval;

	SvXMLExport& rExport;

public:	
	XMLLineNumberingExport(SvXMLExport& rExp);

	void Export();
};

}//end of namespace binfilter
#endif
