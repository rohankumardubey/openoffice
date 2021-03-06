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



#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#define _XMLOFF_PROGRESSBARHELPER_HXX

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif
namespace binfilter {

#define XML_PROGRESSRANGE	"ProgressRange"
#define XML_PROGRESSMAX		"ProgressMax"
#define XML_PROGRESSCURRENT	"ProgressCurrent"
#define XML_PROGRESSREPEAT  "ProgressRepeat"

class ProgressBarHelper
{
			::com::sun::star::uno::Reference < ::com::sun::star::task::XStatusIndicator > 	xStatusIndicator;
			sal_Int32																		nRange;
			sal_Int32																		nReference;
			sal_Int32																		nValue;
			double																			fOldPercent;
			sal_Bool																		bStrict;
            // #96469#; if the value goes over the Range the progressbar starts again
            sal_Bool                                                                        bRepeat;

#ifdef DBG_UTIL
			sal_Bool																		bFailure;
#endif
public:
			ProgressBarHelper(const ::com::sun::star::uno::Reference < ::com::sun::star::task::XStatusIndicator>& xStatusIndicator,
								const sal_Bool bStrict);
			~ProgressBarHelper();

			void SetText(::rtl::OUString& rText) { if (xStatusIndicator.is()) xStatusIndicator->setText(rText); }
			void SetRange(sal_Int32 nValue) { nRange = nValue; }
			void SetReference(sal_Int32 nValue) { nReference = nValue; }
			void SetValue(sal_Int32 nValue);
            void SetRepeat(sal_Bool bValue) { bRepeat = bValue; }
			inline void Increment(sal_Int32 nInc = 1) { SetValue( nValue+nInc ); }
            void End() { if (xStatusIndicator.is()) xStatusIndicator->end(); }

			// set the new reference and returns the new value which gives the
			// Progress Bar the sam position as before
			sal_Int32 ChangeReference(sal_Int32 nNewReference);

			sal_Int32 GetReference() { return nReference; }
			sal_Int32 GetValue() { return nValue; }
            sal_Bool GetRepeat() { return bRepeat; }
};

}//end of namespace binfilter
#endif

