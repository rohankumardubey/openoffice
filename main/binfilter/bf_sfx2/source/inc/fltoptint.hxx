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



#include <com/sun/star/document/XInteractionFilterOptions.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <comphelper/interaction.hxx>

namespace binfilter {
 
class FilterOptionsContinuation : public comphelper::OInteraction< ::com::sun::star::document::XInteractionFilterOptions >
{
	::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > rProperties;

public:
    virtual void SAL_CALL setFilterOptions( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rProp ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getFilterOptions(  ) throw (::com::sun::star::uno::RuntimeException);
};

class RequestFilterOptions : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionRequest >
{
    ::com::sun::star::uno::Any m_aRequest;
		
    ::com::sun::star::uno::Sequence< 
					::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > 
				> m_lContinuations;
		
    comphelper::OInteractionAbort*  m_pAbort;
		
    FilterOptionsContinuation*	m_pOptions;

public:
    RequestFilterOptions( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > rModel,
							  ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > rProperties );
	
    sal_Bool    isAbort() { return m_pAbort->wasSelected(); }
		
	::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > getFilterOptions()
	{
		return m_pOptions->getFilterOptions();
	}
		
    virtual ::com::sun::star::uno::Any SAL_CALL getRequest() 
		throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< 
				::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > 
			> SAL_CALL getContinuations() 
		throw( ::com::sun::star::uno::RuntimeException );
};  
}//end of namespace binfilter
