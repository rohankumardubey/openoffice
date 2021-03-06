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

#ifndef GCC
#endif

//_________________________________________________________________________________________________________________
//	includes
//_________________________________________________________________________________________________________________

#include <bf_svtools/options3d.hxx>

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#include <itemholder1.hxx>

//_________________________________________________________________________________________________________________
//	namespaces
//_________________________________________________________________________________________________________________

using namespace ::utl					;
using namespace ::rtl					;
using namespace ::osl					;
using namespace ::com::sun::star::uno	;

namespace binfilter
{

//_________________________________________________________________________________________________________________
//	const
//_________________________________________________________________________________________________________________

#define	ROOTNODE_START			   		OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Common/_3D_Engine"	))
#define	DEFAULT_DITHERING				sal_True
#define	DEFAULT_OPENGL					sal_True
#define	DEFAULT_OPENGL_FASTER			sal_True
#define	DEFAULT_OPENGL_FASTER			sal_True
#define	DEFAULT_SHOWFULL				sal_False

#define	PROPERTYNAME_DITHERING			OUString(RTL_CONSTASCII_USTRINGPARAM("Dithering"		))
#define	PROPERTYNAME_OPENGL				OUString(RTL_CONSTASCII_USTRINGPARAM("OpenGL"			))
#define	PROPERTYNAME_OPENGL_FASTER		OUString(RTL_CONSTASCII_USTRINGPARAM("OpenGL_Faster"	))
#define	PROPERTYNAME_SHOWFULL			OUString(RTL_CONSTASCII_USTRINGPARAM("ShowFull"	))

#define	PROPERTYHANDLE_DITHERING		0
#define	PROPERTYHANDLE_OPENGL			1
#define	PROPERTYHANDLE_OPENGL_FASTER	2
#define	PROPERTYHANDLE_SHOWFULL			3

#define	PROPERTYCOUNT					4

class SvtOptions3D_Impl : public ConfigItem
{
public:

//---------------------------------------------------------------------------------------------------------
//	constructor / destructor
//---------------------------------------------------------------------------------------------------------

	 SvtOptions3D_Impl();
	~SvtOptions3D_Impl();

//---------------------------------------------------------------------------------------------------------
//	overloaded methods of baseclass
//---------------------------------------------------------------------------------------------------------

	virtual void Commit();
	void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& );

//-------------------------------------------------------------------------------------------------------------
//	private methods
//-------------------------------------------------------------------------------------------------------------

private:

	static Sequence< OUString > impl_GetPropertyNames();

//-------------------------------------------------------------------------------------------------------------
//	private member
//-------------------------------------------------------------------------------------------------------------

private:

		sal_Bool	m_bDithering;
		sal_Bool	m_bOpenGL;
		sal_Bool	m_bOpenGL_Faster;
		sal_Bool	m_bShowFull;
};

//_________________________________________________________________________________________________________________
//	definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//	constructor
//*****************************************************************************************************************
SvtOptions3D_Impl::SvtOptions3D_Impl() :
	ConfigItem( ROOTNODE_START	),
	m_bDithering( DEFAULT_DITHERING ),
	m_bOpenGL( DEFAULT_OPENGL ),
	m_bOpenGL_Faster( DEFAULT_OPENGL_FASTER ),
	m_bShowFull( DEFAULT_SHOWFULL )
{
	Sequence< OUString >	seqNames( impl_GetPropertyNames() );
	Sequence< Any >			seqValues	= GetProperties( seqNames )	;

	DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtOptions3D_Impl::SvtOptions3D_Impl()\nI miss some values of configuration keys!\n" );

	// Copy values from list in right order to ouer internal member.
	sal_Int32 nPropertyCount = seqValues.getLength();
	sal_Int32 nProperty	= 0;
	
	for( nProperty=0; nProperty<nPropertyCount; ++nProperty )
	{
		DBG_ASSERT( !(seqValues[nProperty].hasValue()==sal_False), "SvtOptions3D_Impl::SvtOptions3D_Impl()\nInvalid property value for property detected!\n" );

        switch( nProperty )
        {
            case PROPERTYHANDLE_DITHERING:
			{
				DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptions3D_Impl::SvtOptions3D_Impl()\nWho has changed the value type of \"Office.Common\\_3D_Engine\\Dithering\"?" );
				seqValues[nProperty] >>= m_bDithering;
			}
			break;

            case PROPERTYHANDLE_OPENGL:
			{
				DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptions3D_Impl::SvtOptions3D_Impl()\nWho has changed the value type of \"Office.Common\\_3D_Engine\\OpenGL\"?" );
				seqValues[nProperty] >>= m_bOpenGL;
			}
			break;

            case PROPERTYHANDLE_OPENGL_FASTER:
			{
				DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptions3D_Impl::SvtOptions3D_Impl()\nWho has changed the value type of \"Office.Common\\_3D_Engine\\OpenGL_Faster\"?" );
				seqValues[nProperty] >>= m_bOpenGL_Faster;
			}
			break;

            case PROPERTYHANDLE_SHOWFULL:
			{
				DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtOptions3D_Impl::SvtOptions3D_Impl()\nWho has changed the value type of \"Office.Common\\_3D_Engine\\ShowFull\"?" );
				seqValues[nProperty] >>= m_bShowFull;
			}
			break;
        }
	}
}

//*****************************************************************************************************************
//	destructor
//*****************************************************************************************************************
SvtOptions3D_Impl::~SvtOptions3D_Impl()
{
	if( IsModified() )
		Commit();
}

//*****************************************************************************************************************
//	Commit
//*****************************************************************************************************************
void SvtOptions3D_Impl::Commit()
{
	Sequence< OUString >	aSeqNames( impl_GetPropertyNames() );
	Sequence< Any >			aSeqValues( aSeqNames.getLength() );
	
	for( sal_Int32 nProperty = 0, nCount = aSeqNames.getLength(); nProperty < nCount; ++nProperty )
	{
        switch( nProperty )
        {
            case PROPERTYHANDLE_DITHERING:
				aSeqValues[nProperty] <<= m_bDithering;
			break;

            case PROPERTYHANDLE_OPENGL:
				aSeqValues[nProperty] <<= m_bOpenGL;
			break;

            case PROPERTYHANDLE_OPENGL_FASTER:
				aSeqValues[nProperty] <<= m_bOpenGL_Faster;
			break;

            case PROPERTYHANDLE_SHOWFULL:
				aSeqValues[nProperty] <<= m_bShowFull;
			break;
        }
	}

	PutProperties( aSeqNames, aSeqValues );
}

	void SvtOptions3D_Impl::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

//*****************************************************************************************************************
//     private method
//*****************************************************************************************************************
Sequence< OUString > SvtOptions3D_Impl::impl_GetPropertyNames()
{
       // Build static list of configuration key names.
       static const OUString pProperties[] =
       {
               PROPERTYNAME_DITHERING          ,
               PROPERTYNAME_OPENGL                     ,
               PROPERTYNAME_OPENGL_FASTER      ,
               PROPERTYNAME_SHOWFULL           
       };
       // Initialize return sequence with these list ...
       static const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );
       // ... and return it.
       return seqPropertyNames;
}

//*****************************************************************************************************************
//	initialize static member
//	DON'T DO IT IN YOUR HEADER!
//	see definition for further informations
//*****************************************************************************************************************
SvtOptions3D_Impl*		SvtOptions3D::m_pDataContainer	= NULL	;
sal_Int32				SvtOptions3D::m_nRefCount		= 0		;

//*****************************************************************************************************************
//	constructor
//*****************************************************************************************************************
SvtOptions3D::SvtOptions3D()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
	// Increase ouer refcount ...
	++m_nRefCount;
	// ... and initialize ouer data container only if it not already!
    if( m_pDataContainer == NULL )
	{
        m_pDataContainer = new SvtOptions3D_Impl();
        ItemHolder1::holdConfigItem(E_OPTIONS3D);
	}
}

//*****************************************************************************************************************
//	destructor
//*****************************************************************************************************************
SvtOptions3D::~SvtOptions3D()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );
	// Decrease ouer refcount.
	--m_nRefCount;
	// If last instance was deleted ...
	// we must destroy ouer static data container!
    if( m_nRefCount <= 0 )
	{
		delete m_pDataContainer;
		m_pDataContainer = NULL;
	}
}

//*****************************************************************************************************************
//	private method
//*****************************************************************************************************************
Mutex& SvtOptions3D::GetOwnStaticMutex()
{
	// Initialize static mutex only for one time!
    static Mutex* pMutex = NULL;
	// If these method first called (Mutex not already exist!) ...
    if( pMutex == NULL )
    {
		// ... we must create a new one. Protect follow code with the global mutex -
		// It must be - we create a static variable!
        MutexGuard aGuard( Mutex::getGlobalMutex() );
		// We must check our pointer again - because it can be that another instance of ouer class will be fastr then these!
        if( pMutex == NULL )
        {
			// Create the new mutex and set it for return on static variable.
            static Mutex aMutex;
            pMutex = &aMutex;
        }
    }
	// Return new created or already existing mutex object.
    return *pMutex;
}

}
