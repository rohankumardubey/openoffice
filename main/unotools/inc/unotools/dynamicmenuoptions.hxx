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


#ifndef INCLUDED_unotools_DYNAMICMENUOPTIONS_HXX
#define INCLUDED_unotools_DYNAMICMENUOPTIONS_HXX

//_________________________________________________________________________________________________________________
//	includes
//_________________________________________________________________________________________________________________

#include "unotools/unotoolsdllapi.h"
#include <sal/types.h>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <unotools/options.hxx>

//_________________________________________________________________________________________________________________
//	types, enums, ...
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
	@descr			The method GetList() returns a list of property values.
					Use follow defines to seperate values by names.
*//*-*************************************************************************************************************/
#define DYNAMICMENU_PROPERTYNAME_URL                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL"             ))
#define DYNAMICMENU_PROPERTYNAME_TITLE                  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Title"           ))
#define DYNAMICMENU_PROPERTYNAME_IMAGEIDENTIFIER        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ImageIdentifier" ))
#define DYNAMICMENU_PROPERTYNAME_TARGETNAME             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TargetName"      ))

/*-************************************************************************************************************//**
    @descr          You can use these enum values to specify right menu if you call our interface methods.
*//*-*************************************************************************************************************/
enum EDynamicMenuType
{
    E_NEWMENU       =   0,
    E_WIZARDMENU    =   1,
    E_HELPBOOKMARKS =   2
};
//_________________________________________________________________________________________________________________
//	forward declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
	@short			forward declaration to our private date container implementation
	@descr			We use these class as internal member to support small memory requirements.
					You can create the container if it is neccessary. The class which use these mechanism
					is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class SvtDynamicMenuOptions_Impl;

//_________________________________________________________________________________________________________________
//	declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          collect informations about dynamic menus
    @descr          Make it possible to configure dynamic menu structures of menus like "new" or "wizard".

	@implements		-
	@base			-

	@devstatus		ready to use
*//*-*************************************************************************************************************/

class UNOTOOLS_DLLPUBLIC SvtDynamicMenuOptions: public utl::detail::Options
{
	//-------------------------------------------------------------------------------------------------------------
	//	public methods
	//-------------------------------------------------------------------------------------------------------------

	public:

		//---------------------------------------------------------------------------------------------------------
		//	constructor / destructor
		//---------------------------------------------------------------------------------------------------------

		/*-****************************************************************************************************//**
			@short		standard constructor and destructor
			@descr		This will initialize an instance with default values.
						We implement these class with a refcount mechanism! Every instance of this class increase it
						at create and decrease it at delete time - but all instances use the same data container!
						He is implemented as a static member ...

			@seealso	member m_nRefCount
			@seealso	member m_pDataContainer

			@param		-
			@return		-

			@onerror	-
		*//*-*****************************************************************************************************/

         SvtDynamicMenuOptions();
        virtual ~SvtDynamicMenuOptions();

		//---------------------------------------------------------------------------------------------------------
		//	interface
		//---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
			@short		clear complete sepcified list
            @descr      Call this methods to clear the whole list.
                        To fill it again use AppendItem().

			@seealso	-

            @param      "eMenu" select right menu to clear.
			@return		-

			@onerror	-
		*//*-*****************************************************************************************************/

        void Clear( EDynamicMenuType eMenu );

		/*-****************************************************************************************************//**
            @short      return complete specified list
            @descr      Call it to get all entries of an dynamic menu.
                        We return a list of all nodes with his names and properties.

			@seealso	-

            @param      "eMenu" select right menu.
            @return     A list of menu items is returned.

            @onerror    We return an empty list.
		*//*-*****************************************************************************************************/

        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > GetMenu( EDynamicMenuType eMenu ) const;

		/*-****************************************************************************************************//**
            @short      append a new item to specified menu
            @descr      You can append items to a menu only - removing isn't allowed for a special item!
                        We support a nothing or all mechanism only! Clear all or append something ...

			@seealso	method Clear()

            @param      "eMenu"             select right menu.
            @param      "sURL"              URL for dispatch
            @param      "sTitle"            label of menu entry
            @param      "sImageIdentifier"  icon identifier
            @param      "sTargetName"       target for dispatch
			@return		-

			@onerror	-
		*//*-*****************************************************************************************************/

        void AppendItem(            EDynamicMenuType    eMenu            ,
                            const   ::rtl::OUString&    sURL             ,
                            const   ::rtl::OUString&    sTitle           ,
                            const   ::rtl::OUString&    sImageIdentifier ,
                            const   ::rtl::OUString&    sTargetName      );

	//-------------------------------------------------------------------------------------------------------------
	//	private methods
	//-------------------------------------------------------------------------------------------------------------

	private:

		/*-****************************************************************************************************//**
			@short		return a reference to a static mutex
			@descr		These class is partially threadsafe (for de-/initialization only).
						All access methods are'nt safe!
						We create a static mutex only for one ime and use at different times.

			@seealso	-

			@param		-
			@return		A reference to a static mutex member.

			@onerror	-
		*//*-*****************************************************************************************************/

        UNOTOOLS_DLLPRIVATE static ::osl::Mutex& GetOwnStaticMutex();

	//-------------------------------------------------------------------------------------------------------------
	//	private member
	//-------------------------------------------------------------------------------------------------------------

	private:

		/*Attention

			Don't initialize these static member in these header!
			a) Double dfined symbols will be detected ...
			b) and unresolved externals exist at linking time.
			Do it in your source only.
		 */

        static SvtDynamicMenuOptions_Impl* m_pDataContainer    ;   /// impl. data container as dynamic pointer for smaller memory requirements!
        static sal_Int32             m_nRefCount         ;   /// internal ref count mechanism

};      // class SvtDynamicMenuOptions

#endif  // #ifndef INCLUDED_unotools_DYNAMICMENUOPTIONS_HXX
