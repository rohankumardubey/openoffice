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



#ifndef _XMLOFF_FORMLAYEREXPORT_HXX_
#define _XMLOFF_FORMLAYEREXPORT_HXX_

#ifndef _VOS_REFERNCE_HXX_
#include <vos/refernce.hxx>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _XMLOFF_XMLEXPPR_HXX
#include <bf_xmloff/xmlexppr.hxx>
#endif

namespace com { namespace sun { namespace star { namespace awt {
	class XControlModel;
} } } }
namespace binfilter {

class SvXMLExport;
//.........................................................................
namespace xmloff
{
//.........................................................................

	class OFormLayerXMLExport_Impl;
	class OFormsRootExport;

	//=====================================================================
	//= OFormLayerXMLExport
	//=====================================================================
	/** provides functionallity for exporting a complete form layer.
	*/
	class OFormLayerXMLExport
				:public ::vos::OReference
	{
	protected:
		/// our export context
		SvXMLExport&				m_rContext;
		// impl class
		OFormLayerXMLExport_Impl*	m_pImpl;

	protected:
		~OFormLayerXMLExport();

	public:
		OFormLayerXMLExport(SvXMLExport& _rContext);

		/** initializes some internal structures for fast access to the given page

			<p>This method has to be called before you use getControlId for controls on the given page.
			This way a performance optimization can be done for faster access to the control ids</p>

			@return
				<TRUE/> if the page has been examined before. If <FALSE/> is returned, this is a serious error.

			@see getControlId
			@see examine
		*/
		sal_Bool seekPage(
			const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& _rxDrawPage);

		/** get the id for the given control

			<p>The page the control belongs to must have been examined and sought to.</p>

			@param _rxControl
				the control which's id should be retrieved. Must not be <NULL/>.

			@see examineForms
			@see seekPage

		*/
		::rtl::OUString getControlId(
			const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControl);

		/** retrieves the style name for the control's number style.

			<p>For performance reasons, this method is allowed to be called for any controls, even those which
			do not have a number style. In this case, an empty string is returned.</p>

			@param _rxControl
				the control which's id should be retrieved. Must not be <NULL/>.

			@see examineForms
			@see seekPage
		*/
		::rtl::OUString getControlNumberStyle(
			const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControl );

		/** examines the forms collection given.

			<p>This method will collect all form layer related data of the given draw page</p>

			@param _rxDrawPage
				the draw page to examine. The object will be queried for a <type scope="com.sun.star.form">XFormsSupplier</type>
				interface to obtain the forms container.
		*/
		void examineForms(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& _rxDrawPage);

		/** exports the structure of a form layer

			<p>This method does not export styles (e.g. font properties of the controls), or any references
			external instances may have to the control models contained in the form layer (e.g. uno control
			shapes in the drawing layer may have such references)</p>

			<p>No top level element describing the whole collection is inserted. I.e. if within your document, you
			expect the the forms collection to be stored like
				<listing>
					&lt;Forms&gt;
						....	// all the forms stuff here
					&lt;/Forms&gt;
				</listing>
			you have to start the Forms element yourself.</p>

			@param	_rxDrawPage
				the draw page to examine. The object will be queried for a <type scope="com.sun.star.form">XFormsSupplier</type>
				interface to obtain the forms container.
		*/
		void exportForms(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& _rxDrawPage);

		/** exports the automatic controls number styles
		*/
		void exportAutoControlNumberStyles();

		/** exports the auto-styles collected during the examineForms calls
		*/
		void exportAutoStyles();

		/** exclude the given control (model) from export.

			<p>If your document contains form controls which are not to be exported for whatever reason,
			you need to announce the models of these controls (can be retrieved from XControlShape::getControl)
			to the form layer exporter.<br/>
			Of course you have to do this before calling <member>exportForms</member></p>
		*/
		void excludeFromExport( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > _rxControl );
	};

	//=========================================================================
	//= OOfficeFormsExport
	//=========================================================================
	/// export helper for the office::forms element
	class OOfficeFormsExport
	{
	private:
		OFormsRootExport*	m_pImpl;

	public:
		OOfficeFormsExport( SvXMLExport& _rExp );
		~OOfficeFormsExport();
	};

//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter
#endif // _XMLOFF_FORMLAYEREXPORT_HXX_

