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



#ifndef _FORMS_LIMITED_FORMATS_HXX_
#define _FORMS_LIMITED_FORMATS_HXX_

#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_ 
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_ 
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#include <com/sun/star/beans/XFastPropertySet.hpp>
namespace binfilter {

//.........................................................................
namespace frm
{
//.........................................................................

	//=====================================================================
	//= OLimitedFormats
	//=====================================================================
	/** maintains translation tables format key <-> enum value
		<p>Used for controls which provide a limited number for (standard) formats, which
		should be available as format keys.</p>
	*/
	class OLimitedFormats
	{
	private:
		static sal_Int32	s_nInstanceCount;
		static ::osl::Mutex	s_aMutex;
		static ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >
							s_xStandardFormats;

	protected:
		sal_Int32			m_nFormatEnumPropertyHandle;
		const sal_Int16		m_nTableId;
		::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet >
							m_xAggregate;

	protected:
		/** ctor
			<p>The class id is used to determine the translation table to use. All instances which
			pass the same value here share one table.</p>
		*/
		OLimitedFormats(
			const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
			const sal_Int16 _nClassId
			);
		~OLimitedFormats();

	protected:
		void setAggregateSet(
			const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet >& _rxAggregate,
			sal_Int32 _nOriginalPropertyHandle
		);

	protected:
		void		getFormatKeyPropertyValue( ::com::sun::star::uno::Any& _rValue ) const;
		sal_Bool	convertFormatKeyPropertyValue(
						::com::sun::star::uno::Any& _rConvertedValue,
						::com::sun::star::uno::Any& _rOldValue,
				const	::com::sun::star::uno::Any& _rNewValue
			);
		void		setFormatKeyPropertyValue( const ::com::sun::star::uno::Any& _rNewValue );
		// setFormatKeyPropertyValue should only be called with a value got from convertFormatKeyPropertyValue!

		::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >
					getFormatsSupplier() const { return s_xStandardFormats; }

	private:	
		void acquireSupplier(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);
		void releaseSupplier();

		static void ensureTableInitialized(const sal_Int16 _nTableId);
		static void clearTable(const sal_Int16 _nTableId);
	};

//.........................................................................
}	// namespace frm
//.........................................................................

}//end of namespace binfilter
#endif // _FORMS_LIMITED_FORMATS_HXX_

