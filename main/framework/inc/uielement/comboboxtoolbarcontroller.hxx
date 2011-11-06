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



#ifndef __FRAMEWORK_UIELEMENT_COMBOBOXTOOLBARCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_COMBOBOXTOOLBARCONTROLLER_HXX_

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/ControlCommand.hpp>

//_________________________________________________________________________________________________________________
//	includes of other projects
//_________________________________________________________________________________________________________________

//#include <vcl/toolbox.hxx>
//#include <vcl/combobox.hxx>
#include <uielement/complextoolbarcontroller.hxx>

namespace framework
{

class ToolBar;
class ComboBoxControl;

class IComboBoxListener
{
    public:
        virtual void Select() = 0;
        virtual void DoubleClick() = 0;
        virtual void Modify() = 0;
        virtual void KeyInput( const KeyEvent& rKEvt ) = 0;
	    virtual void GetFocus() = 0;
	    virtual void LoseFocus() = 0;
        virtual long PreNotify( NotifyEvent& rNEvt ) = 0;
};

class ComboboxToolbarController : public IComboBoxListener,
                                  public ComplexToolbarController
                                  
{
    public:
        ComboboxToolbarController( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager, 
                                   const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                   ToolBox*     pToolBar,
                                   sal_uInt16       nID,
                                   sal_Int32    nWidth,
                                   const rtl::OUString& aCommand );
        virtual ~ComboboxToolbarController();

        // XComponent
        virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );

        // IComboBoxListener
        virtual void Select();
        virtual void DoubleClick();
        virtual void Modify();
        virtual void KeyInput( const KeyEvent& rKEvt );
	    virtual void GetFocus();
	    virtual void LoseFocus();
        virtual long PreNotify( NotifyEvent& rNEvt );

    protected:
        virtual void executeControlCommand( const ::com::sun::star::frame::ControlCommand& rControlCommand );
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> getExecuteArgs(sal_Int16 KeyModifier) const;

    private:
        ComboBoxControl*    m_pComboBox;
};

}

#endif // __FRAMEWORK_UIELEMENT_COMBOBOXTOOLBARCONTROLLER_HXX_
