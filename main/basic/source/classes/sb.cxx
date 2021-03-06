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
#include "precompiled_basic.hxx"

#include <stdio.h>

#include "sb.hxx"
#include <tools/rcid.h>
#include <tools/config.hxx>
#include <tools/stream.hxx>
#ifndef __RSC //autogen
#include <tools/errinf.hxx>
#endif
#include <basic/sbx.hxx>
#include <tools/list.hxx>
#include <tools/shl.hxx>
#include <tools/rc.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/processfactory.hxx>
#include "sbunoobj.hxx"
#include "sbjsmeth.hxx"
#include "sbjsmod.hxx"
#include "sbintern.hxx"
#include "disas.hxx"
#include "runtime.hxx"
#include <basic/sbuno.hxx>
#include <basic/sbobjmod.hxx>
#include "stdobj.hxx"
#include "filefmt.hxx"
#include "sb.hrc"
#include <basrid.hxx>
#include <vos/mutex.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include "errobject.hxx"
#include <map>
#include <hash_map>

#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/ModuleInfo.hpp>
using namespace ::com::sun::star::script;

// #pragma SW_SEGMENT_CLASS( SBASIC, SBASIC_CODE )

SV_IMPL_VARARR(SbTextPortions,SbTextPortion)

TYPEINIT1(StarBASIC,SbxObject)

#define RTLNAME "@SBRTL"
//  i#i68894#
using namespace ::com::sun::star;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::lang::XMultiServiceFactory;

const static String aThisComponent( RTL_CONSTASCII_USTRINGPARAM("ThisComponent") );
const static String aVBAHook( RTL_CONSTASCII_USTRINGPARAM( "VBAGlobals" ) );

// ============================================================================

class DocBasicItem : public ::cppu::WeakImplHelper1< util::XCloseListener >
{
public:
    explicit DocBasicItem( StarBASIC& rDocBasic );
    virtual ~DocBasicItem();

    inline const SbxObjectRef& getClassModules() const { return mxClassModules; }
    inline bool isDocClosed() const { return mbDocClosed; }

	void clearDependingVarsOnDelete( StarBASIC& rDeletedBasic );

    void startListening();
    void stopListening();

    virtual void SAL_CALL queryClosing( const lang::EventObject& rSource, sal_Bool bGetsOwnership ) throw (util::CloseVetoException, uno::RuntimeException);
    virtual void SAL_CALL notifyClosing( const lang::EventObject& rSource ) throw (uno::RuntimeException);
    virtual void SAL_CALL disposing( const lang::EventObject& rSource ) throw (uno::RuntimeException);

private:
    StarBASIC&      mrDocBasic;
    SbxObjectRef    mxClassModules;
    bool            mbDocClosed;
    bool            mbDisposed;
};

// ----------------------------------------------------------------------------

DocBasicItem::DocBasicItem( StarBASIC& rDocBasic ) :
    mrDocBasic( rDocBasic ),
    mxClassModules( new SbxObject( String() ) ),
    mbDocClosed( false ),
    mbDisposed( false )
{
}

DocBasicItem::~DocBasicItem()
{
    stopListening();
}

void DocBasicItem::clearDependingVarsOnDelete( StarBASIC& rDeletedBasic )
{
    mrDocBasic.implClearDependingVarsOnDelete( &rDeletedBasic );
}

void DocBasicItem::startListening()
{
    Any aThisComp;
    mrDocBasic.GetUNOConstant( "ThisComponent", aThisComp );
    Reference< util::XCloseBroadcaster > xCloseBC( aThisComp, UNO_QUERY );
    mbDisposed = !xCloseBC.is();
    if( xCloseBC.is() )
        try { xCloseBC->addCloseListener( this ); } catch( uno::Exception& ) {}
}

void DocBasicItem::stopListening()
{
    if( mbDisposed ) return;
    mbDisposed = true;
    Any aThisComp;
    mrDocBasic.GetUNOConstant( "ThisComponent", aThisComp );
    Reference< util::XCloseBroadcaster > xCloseBC( aThisComp, UNO_QUERY );
    if( xCloseBC.is() )
        try { xCloseBC->removeCloseListener( this ); } catch( uno::Exception& ) {}
}

void SAL_CALL DocBasicItem::queryClosing( const lang::EventObject& /*rSource*/, sal_Bool /*bGetsOwnership*/ ) throw (util::CloseVetoException, uno::RuntimeException)
{
}

void SAL_CALL DocBasicItem::notifyClosing( const lang::EventObject& /*rEvent*/ ) throw (uno::RuntimeException)
{
    stopListening();
    mbDocClosed = true;
}

void SAL_CALL DocBasicItem::disposing( const lang::EventObject& /*rEvent*/ ) throw (uno::RuntimeException)
{
    stopListening();
}

// ----------------------------------------------------------------------------

namespace {

typedef ::rtl::Reference< DocBasicItem > DocBasicItemRef;
typedef std::map< const StarBASIC*, DocBasicItemRef > DocBasicItemMap;
static DocBasicItemMap GaDocBasicItems;

const DocBasicItem* lclFindDocBasicItem( const StarBASIC* pDocBasic )
{
    DocBasicItemMap::iterator it = GaDocBasicItems.find( pDocBasic );
    return (it != GaDocBasicItems.end()) ? it->second.get() : 0;
}

void lclInsertDocBasicItem( StarBASIC& rDocBasic )
{
    DocBasicItemRef& rxDocBasicItem = GaDocBasicItems[ &rDocBasic ];
    rxDocBasicItem.set( new DocBasicItem( rDocBasic ) );
    rxDocBasicItem->startListening();
}

void lclRemoveDocBasicItem( StarBASIC& rDocBasic )
{
    DocBasicItemMap::iterator it = GaDocBasicItems.find( &rDocBasic );
    if( it != GaDocBasicItems.end() )
    {
        it->second->stopListening();
        GaDocBasicItems.erase( it );
    }
    DocBasicItemMap::iterator it_end = GaDocBasicItems.end();
    for( it = GaDocBasicItems.begin(); it != it_end; ++it )
        it->second->clearDependingVarsOnDelete( rDocBasic );
}

StarBASIC* lclGetDocBasicForModule( SbModule* pModule )
{
    StarBASIC* pRetBasic = NULL;
    SbxObject* pCurParent = pModule;
    while( pCurParent->GetParent() != NULL )
    {
        pCurParent = pCurParent->GetParent();
        StarBASIC* pDocBasic = PTR_CAST( StarBASIC, pCurParent );
        if( pDocBasic != NULL && pDocBasic->IsDocBasic() )
        {
            pRetBasic = pDocBasic;
            break;
        }
    }
    return pRetBasic;
}

} // namespace

// ============================================================================

SbxObject* StarBASIC::getVBAGlobals( )
{
	if ( !pVBAGlobals )
	{	
		Any aThisDoc;
		if ( GetUNOConstant("ThisComponent", aThisDoc) )
		{
			Reference< XMultiServiceFactory > xDocFac( aThisDoc, UNO_QUERY );
                        if ( xDocFac.is() )
			{
				try
				{
					xDocFac->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.VBAGlobals" ) ) );
				}
				catch( Exception& )
				{
					// Ignore
				}
			}
		}
		pVBAGlobals = (SbUnoObject*)Find( aVBAHook , SbxCLASS_DONTCARE );
	}
	return pVBAGlobals;
}

//  i#i68894#
SbxVariable* StarBASIC::VBAFind( const String& rName, SbxClassType t )
{
	if( rName == aThisComponent )
		return NULL;
	// rename to init globals
	if ( getVBAGlobals( ) )
		return pVBAGlobals->Find( rName, t );
	return NULL;

}
// Create array for conversion SFX <-> VB error code
struct SFX_VB_ErrorItem
{
	sal_uInt16  nErrorVB;
	SbError nErrorSFX;
};

const SFX_VB_ErrorItem __FAR_DATA SFX_VB_ErrorTab[] =
{
	{ 1, SbERR_BASIC_EXCEPTION },  // #87844 Map exception to error code 1
	{ 2, SbERR_SYNTAX },
	{ 3, SbERR_NO_GOSUB },
	{ 4, SbERR_REDO_FROM_START },
	{ 5, SbERR_BAD_ARGUMENT },
	{ 6, SbERR_MATH_OVERFLOW },
	{ 7, SbERR_NO_MEMORY },
	{ 8, SbERR_ALREADY_DIM },
	{ 9, SbERR_OUT_OF_RANGE },
	{ 10, SbERR_DUPLICATE_DEF },
	{ 11, SbERR_ZERODIV },
	{ 12, SbERR_VAR_UNDEFINED },
	{ 13, SbERR_CONVERSION },
	{ 14, SbERR_BAD_PARAMETER },
	{ 18, SbERR_USER_ABORT },
	{ 20, SbERR_BAD_RESUME },
	{ 28, SbERR_STACK_OVERFLOW },
	{ 35, SbERR_PROC_UNDEFINED },
	{ 48, SbERR_BAD_DLL_LOAD },
	{ 49, SbERR_BAD_DLL_CALL },
	{ 51, SbERR_INTERNAL_ERROR },
	{ 52, SbERR_BAD_CHANNEL },
	{ 53, SbERR_FILE_NOT_FOUND },
	{ 54, SbERR_BAD_FILE_MODE },
	{ 55, SbERR_FILE_ALREADY_OPEN },
	{ 57, SbERR_IO_ERROR },
	{ 58, SbERR_FILE_EXISTS },
	{ 59, SbERR_BAD_RECORD_LENGTH },
	{ 61, SbERR_DISK_FULL },
	{ 62, SbERR_READ_PAST_EOF },
	{ 63, SbERR_BAD_RECORD_NUMBER },
	{ 67, SbERR_TOO_MANY_FILES },
	{ 68, SbERR_NO_DEVICE },
	{ 70, SbERR_ACCESS_DENIED },
	{ 71, SbERR_NOT_READY },
	{ 73, SbERR_NOT_IMPLEMENTED },
	{ 74, SbERR_DIFFERENT_DRIVE },
	{ 75, SbERR_ACCESS_ERROR },
	{ 76, SbERR_PATH_NOT_FOUND },
	{ 91, SbERR_NO_OBJECT },
	{ 93, SbERR_BAD_PATTERN },
	{ 94, SBERR_IS_NULL },
	{ 250, SbERR_DDE_ERROR },
	{ 280, SbERR_DDE_WAITINGACK },
	{ 281, SbERR_DDE_OUTOFCHANNELS },
	{ 282, SbERR_DDE_NO_RESPONSE },
	{ 283, SbERR_DDE_MULT_RESPONSES },
	{ 284, SbERR_DDE_CHANNEL_LOCKED },
	{ 285, SbERR_DDE_NOTPROCESSED },
	{ 286, SbERR_DDE_TIMEOUT },
	{ 287, SbERR_DDE_USER_INTERRUPT },
	{ 288, SbERR_DDE_BUSY },
	{ 289, SbERR_DDE_NO_DATA },
	{ 290, SbERR_DDE_WRONG_DATA_FORMAT },
	{ 291, SbERR_DDE_PARTNER_QUIT },
	{ 292, SbERR_DDE_CONV_CLOSED },
	{ 293, SbERR_DDE_NO_CHANNEL },
	{ 294, SbERR_DDE_INVALID_LINK },
	{ 295, SbERR_DDE_QUEUE_OVERFLOW },
	{ 296, SbERR_DDE_LINK_ALREADY_EST },
	{ 297, SbERR_DDE_LINK_INV_TOPIC },
	{ 298, SbERR_DDE_DLL_NOT_FOUND },
	{ 323, SbERR_CANNOT_LOAD },
	{ 341, SbERR_BAD_INDEX },
	{ 366, SbERR_NO_ACTIVE_OBJECT },
	{ 380, SbERR_BAD_PROP_VALUE },
	{ 382, SbERR_PROP_READONLY },
	{ 394, SbERR_PROP_WRITEONLY },
	{ 420, SbERR_INVALID_OBJECT },
	{ 423, SbERR_NO_METHOD },
	{ 424, SbERR_NEEDS_OBJECT },
	{ 425, SbERR_INVALID_USAGE_OBJECT },
	{ 430, SbERR_NO_OLE },
	{ 438, SbERR_BAD_METHOD },
	{ 440, SbERR_OLE_ERROR },
	{ 445, SbERR_BAD_ACTION },
	{ 446, SbERR_NO_NAMED_ARGS },
	{ 447, SbERR_BAD_LOCALE },
	{ 448, SbERR_NAMED_NOT_FOUND },
	{ 449, SbERR_NOT_OPTIONAL },
	{ 450, SbERR_WRONG_ARGS },
	{ 451, SbERR_NOT_A_COLL },
	{ 452, SbERR_BAD_ORDINAL },
	{ 453, SbERR_DLLPROC_NOT_FOUND },
	{ 460, SbERR_BAD_CLIPBD_FORMAT },
	{ 951, SbERR_UNEXPECTED },
	{ 952, SbERR_EXPECTED },
	{ 953, SbERR_SYMBOL_EXPECTED },
	{ 954, SbERR_VAR_EXPECTED },
	{ 955, SbERR_LABEL_EXPECTED },
	{ 956, SbERR_LVALUE_EXPECTED },
	{ 957, SbERR_VAR_DEFINED },
	{ 958, SbERR_PROC_DEFINED },
	{ 959, SbERR_LABEL_DEFINED },
	{ 960, SbERR_UNDEF_VAR },
	{ 961, SbERR_UNDEF_ARRAY },
	{ 962, SbERR_UNDEF_PROC },
	{ 963, SbERR_UNDEF_LABEL },
	{ 964, SbERR_UNDEF_TYPE },
	{ 965, SbERR_BAD_EXIT },
	{ 966, SbERR_BAD_BLOCK },
	{ 967, SbERR_BAD_BRACKETS },
	{ 968, SbERR_BAD_DECLARATION },
	{ 969, SbERR_BAD_PARAMETERS },
	{ 970, SbERR_BAD_CHAR_IN_NUMBER },
	{ 971, SbERR_MUST_HAVE_DIMS },
	{ 972, SbERR_NO_IF },
	{ 973, SbERR_NOT_IN_SUBR },
	{ 974, SbERR_NOT_IN_MAIN },
	{ 975, SbERR_WRONG_DIMS },
	{ 976, SbERR_BAD_OPTION },
	{ 977, SbERR_CONSTANT_REDECLARED },
	{ 978, SbERR_PROG_TOO_LARGE },
	{ 979, SbERR_NO_STRINGS_ARRAYS },
	{ 1000, SbERR_PROPERTY_NOT_FOUND },
	{ 1001, SbERR_METHOD_NOT_FOUND },
	{ 1002, SbERR_ARG_MISSING },
	{ 1003, SbERR_BAD_NUMBER_OF_ARGS },
	{ 1004, SbERR_METHOD_FAILED },
	{ 1005, SbERR_SETPROP_FAILED },
	{ 1006, SbERR_GETPROP_FAILED },
	{ 1007, SbERR_BASIC_COMPAT },
	{ 0xFFFF, 0xFFFFFFFFL }		// End mark
};

// The StarBASIC factory is a hack. When a SbModule is created, its pointer
// is saved and given to the following SbProperties/SbMethods. This restores
// the Modul-relationshop. But it works only when a modul is loaded.
// Can cause troubles with separately loaded properties!

SbxBase* SbiFactory::Create( sal_uInt16 nSbxId, sal_uInt32 nCreator )
{
	if( nCreator ==  SBXCR_SBX )
	{
		String aEmpty;
		switch( nSbxId )
		{
			case SBXID_BASIC:
				return new StarBASIC( NULL );
			case SBXID_BASICMOD:
				return new SbModule( aEmpty );
			case SBXID_BASICPROP:
				return new SbProperty( aEmpty, SbxVARIANT, NULL );
			case SBXID_BASICMETHOD:
				return new SbMethod( aEmpty, SbxVARIANT, NULL );
			case SBXID_JSCRIPTMOD:
				return new SbJScriptModule( aEmpty );
			case SBXID_JSCRIPTMETH:
				return new SbJScriptMethod( aEmpty, SbxVARIANT, NULL );
		}
	}
	return NULL;
}

SbxObject* SbiFactory::CreateObject( const String& rClass )
{
	if( rClass.EqualsIgnoreCaseAscii( "StarBASIC" ) )
		return new StarBASIC( NULL );
	else
	if( rClass.EqualsIgnoreCaseAscii( "StarBASICModule" ) )
	{
		String aEmpty;
		return new SbModule( aEmpty );
	}
	else
	if( rClass.EqualsIgnoreCaseAscii( "Collection" ) )
	{
		String aCollectionName( RTL_CONSTASCII_USTRINGPARAM("Collection") );
		return new BasicCollection( aCollectionName );
	}
	else
    if( rClass.EqualsIgnoreCaseAscii( "FileSystemObject" ) )
    {
        try
        {
            Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory(), UNO_SET_THROW );
            ::rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.FileSystemObject" ) );
            Reference< XInterface > xInterface( xFactory->createInstance( aServiceName ), UNO_SET_THROW );
            return new SbUnoObject( aServiceName, uno::makeAny( xInterface ) );
        }
        catch( Exception& )
        {}
    }

    return NULL;
}


// Factory class to create OLE objects
class SbOLEFactory : public SbxFactory
{
public:
	virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 = SBXCR_SBX );
	virtual SbxObject* CreateObject( const String& );
};

SbxBase* SbOLEFactory::Create( sal_uInt16, sal_uInt32 )
{
	// Not supported
	return NULL;
}

SbUnoObject* createOLEObject_Impl( const String& aType );	// sbunoobj.cxx

SbxObject* SbOLEFactory::CreateObject( const String& rClassName )
{
	SbxObject* pRet = createOLEObject_Impl( rClassName );
	return pRet;
}


//========================================================================
// SbFormFactory, show user forms by: dim as new <user form name>

class SbFormFactory : public SbxFactory
{
public:
	virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 = SBXCR_SBX );
	virtual SbxObject* CreateObject( const String& );
};

SbxBase* SbFormFactory::Create( sal_uInt16, sal_uInt32 )
{
	// Not supported
	return NULL;
}

SbxObject* SbFormFactory::CreateObject( const String& rClassName )
{
	if( SbModule* pMod = pMOD )
	{
		if( SbxVariable* pVar = pMod->Find( rClassName, SbxCLASS_OBJECT ) )
		{
			if( SbUserFormModule* pFormModule = PTR_CAST( SbUserFormModule, pVar->GetObject() ) )
			{
			    bool bInitState = pFormModule->getInitState();
				if( bInitState )
				{
					// Not the first instantiate, reset
					bool bTriggerTerminateEvent = false;
					pFormModule->ResetApiObj( bTriggerTerminateEvent );
					pFormModule->setInitState( false );
				}
				else
				{
					pFormModule->Load();
				}
				return pFormModule->CreateInstance();
			}
		}
	}
	return 0;
}


//========================================================================
// SbTypeFactory

SbxObject* cloneTypeObjectImpl( const SbxObject& rTypeObj )
{
	SbxObject* pRet = new SbxObject( rTypeObj );
	pRet->PutObject( pRet );

	// Copy the properties, not only the reference to them
	SbxArray* pProps = pRet->GetProperties();
	sal_uInt32 nCount = pProps->Count32();
	for( sal_uInt32 i = 0 ; i < nCount ; i++ )
	{
		SbxVariable* pVar = pProps->Get32( i );
		SbxProperty* pProp = PTR_CAST( SbxProperty, pVar );
		if( pProp )
		{
			SbxProperty* pNewProp = new SbxProperty( *pProp );
			SbxDataType eVarType = pVar->GetType();
			if( eVarType & SbxARRAY )
			{
				SbxBase* pParObj = pVar->GetObject();
				SbxDimArray* pSource = PTR_CAST(SbxDimArray,pParObj);
				SbxDimArray* pDest = new SbxDimArray( pVar->GetType() );
				sal_Int32 lb = 0;
				sal_Int32 ub = 0;

				pDest->setHasFixedSize( pSource->hasFixedSize() );
				if ( pSource->GetDims() && pSource->hasFixedSize() )
				{
					for ( sal_Int32 j = 1 ; j <= pSource->GetDims(); ++j )
					{
						pSource->GetDim32( (sal_Int32)j, lb, ub );
						pDest->AddDim32( lb, ub );
					}
				}
				else 
					pDest->unoAddDim( 0, -1 ); // variant array

				sal_uInt16 nSavFlags = pVar->GetFlags();
				pNewProp->ResetFlag( SBX_FIXED );
				// need to reset the FIXED flag 
				// when calling PutObject ( because the type will not match Object ) 	
				pNewProp->PutObject( pDest );
				pNewProp->SetFlags( nSavFlags );
			}
			if( eVarType == SbxOBJECT )
			{
				SbxBase* pObjBase = pVar->GetObject();
				SbxObject* pSrcObj = PTR_CAST(SbxObject,pObjBase);
				SbxObject* pDestObj = NULL;
				if( pSrcObj != NULL )
					pDestObj = cloneTypeObjectImpl( *pSrcObj );
				pNewProp->PutObject( pDestObj );
			}
			pProps->PutDirect( pNewProp, i );
		}
	}
	return pRet;
}

// Factory class to create user defined objects (type command)
class SbTypeFactory : public SbxFactory
{
public:
	virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 = SBXCR_SBX );
	virtual SbxObject* CreateObject( const String& );
};

SbxBase* SbTypeFactory::Create( sal_uInt16, sal_uInt32 )
{
	// Not supported
	return NULL;
}

SbxObject* SbTypeFactory::CreateObject( const String& rClassName )
{
	SbxObject* pRet = NULL;
	SbModule* pMod = pMOD;
	if( pMod )
	{
		const SbxObject* pObj = pMod->FindType( rClassName );
		if( pObj )
			pRet = cloneTypeObjectImpl( *pObj );
	}
	return pRet;
}

SbxObject* createUserTypeImpl( const String& rClassName )
{
	SbxObject* pRetObj = pTYPEFAC->CreateObject( rClassName );
	return pRetObj;
}


TYPEINIT1(SbClassModuleObject,SbModule)

SbClassModuleObject::SbClassModuleObject( SbModule* pClassModule )
	: SbModule( pClassModule->GetName() )
	, mpClassModule( pClassModule )
	, mbInitializeEventDone( false )
{
	aOUSource = pClassModule->aOUSource;
	aComment = pClassModule->aComment;
	pImage = pClassModule->pImage;
	pBreaks = pClassModule->pBreaks;

	SetClassName( pClassModule->GetName() );

	// Allow search only internally
	ResetFlag( SBX_GBLSEARCH );

	// Copy the methods from original class module
	SbxArray* pClassMethods = pClassModule->GetMethods();
	sal_uInt32 nMethodCount = pClassMethods->Count32();
	sal_uInt32 i;
	for( i = 0 ; i < nMethodCount ; i++ )
	{
		SbxVariable* pVar = pClassMethods->Get32( i );

		// Exclude SbIfaceMapperMethod to copy them in a second step
		SbIfaceMapperMethod* pIfaceMethod = PTR_CAST( SbIfaceMapperMethod, pVar );
		if( !pIfaceMethod )
		{
			SbMethod* pMethod = PTR_CAST(SbMethod, pVar );
			if( pMethod )
			{
				sal_uInt16 nFlags_ = pMethod->GetFlags();
				pMethod->SetFlag( SBX_NO_BROADCAST );
				SbMethod* pNewMethod = new SbMethod( *pMethod );
				pNewMethod->ResetFlag( SBX_NO_BROADCAST );
				pMethod->SetFlags( nFlags_ );
				pNewMethod->pMod = this;
				pNewMethod->SetParent( this );
				pMethods->PutDirect( pNewMethod, i );
				StartListening( pNewMethod->GetBroadcaster(), sal_True );
			}
		}
	}

	// Copy SbIfaceMapperMethod in a second step to ensure that
	// the corresponding base methods have already been copied
	for( i = 0 ; i < nMethodCount ; i++ )
	{
		SbxVariable* pVar = pClassMethods->Get32( i );

		SbIfaceMapperMethod* pIfaceMethod = PTR_CAST( SbIfaceMapperMethod, pVar );
		if( pIfaceMethod )
		{
			SbMethod* pImplMethod = pIfaceMethod->getImplMethod();
			if( !pImplMethod )
			{
				DBG_ERROR( "No ImplMethod" );
				continue;
			}

			// Search for own copy of ImplMethod
			String aImplMethodName = pImplMethod->GetName();
			SbxVariable* p = pMethods->Find( aImplMethodName, SbxCLASS_METHOD );
			SbMethod* pImplMethodCopy = p ? PTR_CAST(SbMethod,p) : NULL;
			if( !pImplMethodCopy )
			{
				DBG_ERROR( "Found no ImplMethod copy" );
				continue;
			}
			SbIfaceMapperMethod* pNewIfaceMethod =
				new SbIfaceMapperMethod( pIfaceMethod->GetName(), pImplMethodCopy );
			pMethods->PutDirect( pNewIfaceMethod, i );
		}
	}

	// Copy the properties from original class module
	SbxArray* pClassProps = pClassModule->GetProperties();
	sal_uInt32 nPropertyCount = pClassProps->Count32();
	for( i = 0 ; i < nPropertyCount ; i++ )
	{
		SbxVariable* pVar = pClassProps->Get32( i );
		SbProcedureProperty* pProcedureProp = PTR_CAST( SbProcedureProperty, pVar );
		if( pProcedureProp )
		{
			sal_uInt16 nFlags_ = pProcedureProp->GetFlags();
			pProcedureProp->SetFlag( SBX_NO_BROADCAST );
			SbProcedureProperty* pNewProp = new SbProcedureProperty
				( pProcedureProp->GetName(), pProcedureProp->GetType() );
				// ( pProcedureProp->GetName(), pProcedureProp->GetType(), this );
			pNewProp->SetFlags( nFlags_ ); // Copy flags
			pNewProp->ResetFlag( SBX_NO_BROADCAST ); // except the Broadcast if it was set
			pProcedureProp->SetFlags( nFlags_ );
			pProps->PutDirect( pNewProp, i );
			StartListening( pNewProp->GetBroadcaster(), sal_True );
		}
		else
		{
			SbxProperty* pProp = PTR_CAST( SbxProperty, pVar );
			if( pProp )
			{
				sal_uInt16 nFlags_ = pProp->GetFlags();
				pProp->SetFlag( SBX_NO_BROADCAST );
				SbxProperty* pNewProp = new SbxProperty( *pProp );

				// Special handling for modules instances and collections, they need
				// to be instantiated, otherwise all refer to the same base object
				SbxDataType eVarType = pProp->GetType();
				if( eVarType == SbxOBJECT )
				{
					SbxBase* pObjBase = pProp->GetObject();
					SbxObject* pObj = PTR_CAST(SbxObject,pObjBase);
					if( pObj != NULL )
					{
						String aObjClass = pObj->GetClassName();

						SbClassModuleObject* pClassModuleObj = PTR_CAST(SbClassModuleObject,pObjBase);
						if( pClassModuleObj != NULL )
						{
							SbModule* pLclClassModule = pClassModuleObj->getClassModule();
							SbClassModuleObject* pNewObj = new SbClassModuleObject( pLclClassModule );
							pNewObj->SetName( pProp->GetName() );
							pNewObj->SetParent( pLclClassModule->pParent );
							pNewProp->PutObject( pNewObj );
						}
						else if( aObjClass.EqualsIgnoreCaseAscii( "Collection" ) )
						{
							String aCollectionName( RTL_CONSTASCII_USTRINGPARAM("Collection") );
							BasicCollection* pNewCollection = new BasicCollection( aCollectionName );
							pNewCollection->SetName( pProp->GetName() );
							pNewCollection->SetParent( pClassModule->pParent );
							pNewProp->PutObject( pNewCollection );
						}
					}
				}

				pNewProp->ResetFlag( SBX_NO_BROADCAST );
				pNewProp->SetParent( this );
				pProps->PutDirect( pNewProp, i );
				pProp->SetFlags( nFlags_ );
			}
		}
	}
	SetModuleType( ModuleType::CLASS );
	mbVBACompat = pClassModule->mbVBACompat;
}

SbClassModuleObject::~SbClassModuleObject()
{
    // do not trigger termination event when document is already closed
	if( StarBASIC::IsRunning() )
        if( StarBASIC* pDocBasic = lclGetDocBasicForModule( this ) )
            if( const DocBasicItem* pDocBasicItem = lclFindDocBasicItem( pDocBasic ) )
                if( !pDocBasicItem->isDocClosed() )
                    triggerTerminateEvent();

	// Must be deleted by base class dtor because this data
	// is not owned by the SbClassModuleObject object
	pImage = NULL;
	pBreaks = NULL;
}

void SbClassModuleObject::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
						   const SfxHint& rHint, const TypeId& rHintType )
{
	handleProcedureProperties( rBC, rHint );
}

SbxVariable* SbClassModuleObject::Find( const XubString& rName, SbxClassType t )
{
	SbxVariable* pRes = SbxObject::Find( rName, t );
	if( pRes )
	{
		triggerInitializeEvent();

		SbIfaceMapperMethod* pIfaceMapperMethod = PTR_CAST(SbIfaceMapperMethod,pRes);
		if( pIfaceMapperMethod )
		{
			pRes = pIfaceMapperMethod->getImplMethod();
			pRes->SetFlag( SBX_EXTFOUND );
		}
	}
	return pRes;
}

void SbClassModuleObject::triggerInitializeEvent( void )
{
	static String aInitMethodName( RTL_CONSTASCII_USTRINGPARAM("Class_Initialize") );

	if( mbInitializeEventDone )
		return;

	mbInitializeEventDone = true;

	// Search method
	SbxVariable* pMeth = SbxObject::Find( aInitMethodName, SbxCLASS_METHOD );
	if( pMeth )
	{
		SbxValues aVals;
		pMeth->Get( aVals );
	}
}

void SbClassModuleObject::triggerTerminateEvent( void )
{
	static String aTermMethodName( RTL_CONSTASCII_USTRINGPARAM("Class_Terminate") );

	if( !mbInitializeEventDone || GetSbData()->bRunInit )
		return;

	// Search method
	SbxVariable* pMeth = SbxObject::Find( aTermMethodName, SbxCLASS_METHOD );
	if( pMeth )
	{
		SbxValues aVals;
		pMeth->Get( aVals );
	}
}


SbClassData::SbClassData( void )
{
	mxIfaces = new SbxArray();
}

void SbClassData::clear( void )
{
	mxIfaces->Clear(); 
	maRequiredTypes.clear();
}

SbClassFactory::SbClassFactory( void )
{
	String aDummyName;
	xClassModules = new SbxObject( aDummyName );
}

SbClassFactory::~SbClassFactory()
{}

void SbClassFactory::AddClassModule( SbModule* pClassModule )
{
	SbxObjectRef xToUseClassModules = xClassModules;

    if( StarBASIC* pDocBasic = lclGetDocBasicForModule( pClassModule ) )
        if( const DocBasicItem* pDocBasicItem = lclFindDocBasicItem( pDocBasic ) )
            xToUseClassModules = pDocBasicItem->getClassModules();

	SbxObject* pParent = pClassModule->GetParent();
	xToUseClassModules->Insert( pClassModule );
	pClassModule->SetParent( pParent );
}

void SbClassFactory::RemoveClassModule( SbModule* pClassModule )
{
	xClassModules->Remove( pClassModule );
}

SbxBase* SbClassFactory::Create( sal_uInt16, sal_uInt32 )
{
	// Not supported
	return NULL;
}

SbxObject* SbClassFactory::CreateObject( const String& rClassName )
{
	SbxObjectRef xToUseClassModules = xClassModules;

    if( SbModule* pMod = pMOD )
        if( StarBASIC* pDocBasic = lclGetDocBasicForModule( pMod ) )
            if( const DocBasicItem* pDocBasicItem = lclFindDocBasicItem( pDocBasic ) )
                xToUseClassModules = pDocBasicItem->getClassModules();

	SbxVariable* pVar = xToUseClassModules->Find( rClassName, SbxCLASS_OBJECT );
	SbxObject* pRet = NULL;
	if( pVar )
	{
		SbModule* pVarMod = (SbModule*)pVar;
		pRet = new SbClassModuleObject( pVarMod );
	}
	return pRet;
}

SbModule* SbClassFactory::FindClass( const String& rClassName )
{
	SbxVariable* pVar = xClassModules->Find( rClassName, SbxCLASS_DONTCARE );
	SbModule* pMod = pVar ? (SbModule*)pVar : NULL;
	return pMod;
}

StarBASIC::StarBASIC( StarBASIC* p, sal_Bool bIsDocBasic  )
	: SbxObject( String( RTL_CONSTASCII_USTRINGPARAM("StarBASIC") ) ), bDocBasic( bIsDocBasic )
{
	SetParent( p );
	pLibInfo = NULL;
	bNoRtl = bBreak = sal_False;
	bVBAEnabled = sal_False;
	pModules = new SbxArray;

	if( !GetSbData()->nInst++ )
	{
		pSBFAC = new SbiFactory;
		AddFactory( pSBFAC );
		pTYPEFAC = new SbTypeFactory;
		AddFactory( pTYPEFAC );
		pCLASSFAC = new SbClassFactory;
		AddFactory( pCLASSFAC );
		pOLEFAC = new SbOLEFactory;
		AddFactory( pOLEFAC );
		pFORMFAC = new SbFormFactory;
		AddFactory( pFORMFAC );
		pUNOFAC = new SbUnoFactory;
		AddFactory( pUNOFAC );
	}
	pRtl = new SbiStdObject( String( RTL_CONSTASCII_USTRINGPARAM(RTLNAME) ), this );
    // Search via StarBasic is always global
	SetFlag( SBX_GBLSEARCH );
	pVBAGlobals = NULL;
	bQuit = sal_False;

	if( bDocBasic )
        lclInsertDocBasicItem( *this );
}

// #51727 Override SetModified so that the modified state
// is not given to the parent
void StarBASIC::SetModified( sal_Bool b )
{
    SbxBase::SetModified( b );
}

extern void lcl_closeTraceFile();

StarBASIC::~StarBASIC()
{
#ifdef DBG_TRACE_BASIC
    lcl_closeTraceFile();
#endif

	// Needs to be first action as it can trigger events
	disposeComVariablesForBasic( this );

	if( !--GetSbData()->nInst )
	{
		RemoveFactory( pSBFAC );
		delete pSBFAC; pSBFAC = NULL;
		RemoveFactory( pUNOFAC );
		delete pUNOFAC; pUNOFAC = NULL;
		RemoveFactory( pTYPEFAC );
		delete pTYPEFAC; pTYPEFAC = NULL;
		RemoveFactory( pCLASSFAC );
		delete pCLASSFAC; pCLASSFAC = NULL;
		RemoveFactory( pOLEFAC );
		delete pOLEFAC; pOLEFAC = NULL;
		RemoveFactory( pFORMFAC );
		delete pFORMFAC; pFORMFAC = NULL;

#ifdef DBG_UTIL
	// There is no need to clean SbiData at program end,
	// but we dislike MLK's at Purify
	// TODO: Where else???
	SbiGlobals** pp = (SbiGlobals**) ::GetAppData( SHL_SBC );
	SbiGlobals* p = *pp;
	if( p )
	{
		delete p;
		*pp = 0;
	}
#endif
	}
	else if( bDocBasic )
	{
	    SbxError eOld = SbxBase::GetError();

        lclRemoveDocBasicItem( *this );

		SbxBase::ResetError();
		if( eOld != SbxERR_OK )
			SbxBase::SetError( eOld );
	}

    // #100326 Set Parent NULL in registered listeners
    if( xUnoListeners.Is() )
    {
        sal_uInt16 uCount = xUnoListeners->Count();
	    for( sal_uInt16 i = 0 ; i < uCount ; i++ )
	    {
		    SbxVariable* pListenerObj = xUnoListeners->Get( i );
            pListenerObj->SetParent( NULL );
        }
        xUnoListeners = NULL;
    }

    clearUnoMethodsForBasic( this );
}

// Override new() operator, so that everyone can create a new instance
void* StarBASIC::operator new( size_t n )
{
	if( n < sizeof( StarBASIC ) )
	{
//		DBG_ASSERT( sal_False, "Warnung: inkompatibler BASIC-Stand!" );
		n = sizeof( StarBASIC );
	}
	return ::operator new( n );
}

void StarBASIC::operator delete( void* p )
{
	::operator delete( p );
}

void StarBASIC::implClearDependingVarsOnDelete( StarBASIC* pDeletedBasic )
{
	if( this != pDeletedBasic )
	{
		for( sal_uInt16 i = 0; i < pModules->Count(); i++ )
		{
			SbModule* p = (SbModule*)pModules->Get( i );
			p->ClearVarsDependingOnDeletedBasic( pDeletedBasic );
		}
	}

	for( sal_uInt16 nObj = 0; nObj < pObjs->Count(); nObj++ )
	{
		SbxVariable* pVar = pObjs->Get( nObj );
		StarBASIC* pBasic = PTR_CAST(StarBASIC,pVar);
		if( pBasic && pBasic != pDeletedBasic )
			pBasic->implClearDependingVarsOnDelete( pDeletedBasic );
	}
}


/**************************************************************************
*
*    Creation/Managment of modules
*
**************************************************************************/

SbModule* StarBASIC::MakeModule( const String& rName, const String& rSrc )
{
	return MakeModule32( rName, rSrc );
}

SbModule* StarBASIC::MakeModule32( const String& rName, const ::rtl::OUString& rSrc )
{
    ModuleInfo mInfo;
    mInfo.ModuleType = ModuleType::NORMAL;
    return MakeModule32(  rName, mInfo, rSrc );
}
SbModule* StarBASIC::MakeModule32( const String& rName, const ModuleInfo& mInfo, const rtl::OUString& rSrc )
{
    
    OSL_TRACE("create module %s type mInfo %d", rtl::OUStringToOString( rName, RTL_TEXTENCODING_UTF8 ).getStr(), mInfo.ModuleType );
    SbModule* p = NULL;
    switch ( mInfo.ModuleType )
    {
        case ModuleType::DOCUMENT:
            // In theory we should be able to create Object modules
            // in ordinary basic ( in vba mode thought these are create
            // by the application/basic and not by the user )
            p = new SbObjModule( rName, mInfo, isVBAEnabled() );
            break;
        case ModuleType::CLASS:
            p = new SbModule( rName, isVBAEnabled() );
			p->SetModuleType( ModuleType::CLASS );
	    break;
        case ModuleType::FORM:
            p = new SbUserFormModule( rName, mInfo, isVBAEnabled() );
	    break;
        default:
            p = new SbModule( rName, isVBAEnabled() );
            
    }
	p->SetSource32( rSrc );
	p->SetParent( this );
	pModules->Insert( p, pModules->Count() );
	SetModified( sal_True );
	return p;
}

void StarBASIC::Insert( SbxVariable* pVar )
{
	if( pVar->IsA( TYPE(SbModule) ) )
	{
		pModules->Insert( pVar, pModules->Count() );
		pVar->SetParent( this );
		StartListening( pVar->GetBroadcaster(), sal_True );
	}
	else
	{
		sal_Bool bWasModified = IsModified();
		SbxObject::Insert( pVar );
		if( !bWasModified && pVar->IsSet( SBX_DONTSTORE ) )
			SetModified( sal_False );
	}
}

void StarBASIC::Remove( SbxVariable* pVar )
{
	if( pVar->IsA( TYPE(SbModule) ) )
	{
        // #87540 Can be last reference!
        SbxVariableRef xVar = pVar;
		pModules->Remove( pVar );
		pVar->SetParent( 0 );
		EndListening( pVar->GetBroadcaster() );
	}
	else
		SbxObject::Remove( pVar );
}

sal_Bool StarBASIC::Compile( SbModule* pMod )
{
	return pMod ? pMod->Compile() : sal_False;
}

sal_Bool StarBASIC::Disassemble( SbModule* pMod, String& rText )
{
	rText.Erase();
	if( pMod )
		pMod->Disassemble( rText );
	return sal_Bool( rText.Len() != 0 );
}

void StarBASIC::Clear()
{
	while( pModules->Count() )
		pModules->Remove( pModules->Count() - 1 );
}

SbModule* StarBASIC::FindModule( const String& rName )
{
	for( sal_uInt16 i = 0; i < pModules->Count(); i++ )
	{
		SbModule* p = (SbModule*) pModules->Get( i );
		if( p->GetName().EqualsIgnoreCaseAscii( rName ) )
			return p;
	}
	return NULL;
}


struct ClassModuleRunInitItem
{
	SbModule*		m_pModule;
	bool			m_bProcessing;
	bool			m_bRunInitDone;
	//ModuleVector	m_vModulesDependingOnThisModule;

	ClassModuleRunInitItem( void )
		: m_pModule( NULL )
		, m_bProcessing( false )
		, m_bRunInitDone( false )
	{}
	ClassModuleRunInitItem( SbModule* pModule )
		: m_pModule( pModule )
		, m_bProcessing( false )
		, m_bRunInitDone( false )
	{}
};

// Derive from has_map type instead of typedef
// to allow forward declaration in sbmod.hxx
class ModuleInitDependencyMap : public
	std::hash_map< ::rtl::OUString, ClassModuleRunInitItem,
		::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > >
{};

void SbModule::implProcessModuleRunInit( ModuleInitDependencyMap& rMap, ClassModuleRunInitItem& rItem )
{
	rItem.m_bProcessing = true;

	//bool bAnyDependencies = true;
	SbModule* pModule = rItem.m_pModule;
	if( pModule->pClassData != NULL )
	{
		StringVector& rReqTypes = pModule->pClassData->maRequiredTypes;
		if( rReqTypes.size() > 0 )
		{
			for( StringVector::iterator it = rReqTypes.begin() ; it != rReqTypes.end() ; ++it )
			{
				String& rStr = *it;

				// Is required type a class module?
				ModuleInitDependencyMap::iterator itFind = rMap.find( rStr );
				if( itFind != rMap.end() )
				{
					ClassModuleRunInitItem& rParentItem = itFind->second;
					if( rParentItem.m_bProcessing )
					{
						// TODO: raise error?
						DBG_ERROR( "Cyclic module dependency detected" );
						continue;
					}

					if( !rParentItem.m_bRunInitDone )
						implProcessModuleRunInit( rMap, rParentItem );
				}
			}
		}
	}

	pModule->RunInit();
	rItem.m_bRunInitDone = true;
	rItem.m_bProcessing = false;
}

// Run Init-Code of all modules (including inserted libraries)
void StarBASIC::InitAllModules( StarBASIC* pBasicNotToInit )
{
	::vos::OGuard guard( Application::GetSolarMutex() );

    // Init own modules
	for ( sal_uInt16 nMod = 0; nMod < pModules->Count(); nMod++ )
	{
		SbModule* pModule = (SbModule*)pModules->Get( nMod );
		if(	!pModule->IsCompiled() )
			pModule->Compile();
	}
	// compile modules first then RunInit ( otherwise there is 
	// can be order dependency, e.g. classmodule A has a member
	// of of type classmodule B and classmodule B hasn't been compiled yet )

	// Consider required types to init in right order. Class modules
	// that are required by other modules have to be initialized first.
	ModuleInitDependencyMap aMIDMap;
	for ( sal_uInt16 nMod = 0; nMod < pModules->Count(); nMod++ )
	{
		SbModule* pModule = (SbModule*)pModules->Get( nMod );
		String aModuleName = pModule->GetName();
		if( pModule->isProxyModule() )
			aMIDMap[aModuleName] = ClassModuleRunInitItem( pModule );
	}

	ModuleInitDependencyMap::iterator it;
	for( it = aMIDMap.begin() ; it != aMIDMap.end(); ++it )
	{
		ClassModuleRunInitItem& rItem = it->second;
		SbModule::implProcessModuleRunInit( aMIDMap, rItem );
	}

	// Call RunInit on standard modules
	for ( sal_uInt16 nMod = 0; nMod < pModules->Count(); nMod++ )
	{
		SbModule* pModule = (SbModule*)pModules->Get( nMod );
		if( !pModule->isProxyModule() )
			pModule->RunInit();
	}

    // Check all objects if they are BASIC,
    // if yes initialize
	for ( sal_uInt16 nObj = 0; nObj < pObjs->Count(); nObj++ )
	{
		SbxVariable* pVar = pObjs->Get( nObj );
		StarBASIC* pBasic = PTR_CAST(StarBASIC,pVar);
		if( pBasic && pBasic != pBasicNotToInit )
			pBasic->InitAllModules();
	}
}

// #88329 Put modules back to not initialised state to
// force reinitialisation at next start
void StarBASIC::DeInitAllModules( void )
{
    // Deinit own modules
	for ( sal_uInt16 nMod = 0; nMod < pModules->Count(); nMod++ )
	{
		SbModule* pModule = (SbModule*)pModules->Get( nMod );
        if( pModule->pImage && !pModule->isProxyModule() && !pModule->ISA(SbObjModule) )
		    pModule->pImage->bInit = false;
	}

	for ( sal_uInt16 nObj = 0; nObj < pObjs->Count(); nObj++ )
	{
		SbxVariable* pVar = pObjs->Get( nObj );
		StarBASIC* pBasic = PTR_CAST(StarBASIC,pVar);
		if( pBasic )
			pBasic->DeInitAllModules();
	}
}

// #43011 For TestTool, to delete global vars
void StarBASIC::ClearGlobalVars( void )
{
	SbxArrayRef xProps( GetProperties() );
	sal_uInt16 nPropCount = xProps->Count();
	for ( sal_uInt16 nProp = 0 ; nProp < nPropCount ; ++nProp )
	{
		SbxBase* pVar = xProps->Get( nProp );
		pVar->Clear();
	}
	SetModified( sal_True );
}

// This implementation at first searches within the runtime library,
// then it looks for an element within one module. This moudle can be
// a public var or an entrypoint. If it is not found and we look for a
// method and a module with the given name is found the search continues
// for entrypoint "Main".
// If this fails again a conventional search over objects is performend.
SbxVariable* StarBASIC::Find( const String& rName, SbxClassType t )
{
	static String aMainStr( RTL_CONSTASCII_USTRINGPARAM("Main") );

	SbxVariable* pRes = NULL;
	SbModule* pNamed = NULL;
	// "Extended" search in Runtime Lib
	// but only if SbiRuntime has not set the flag
	if( !bNoRtl )
	{
		if( t == SbxCLASS_DONTCARE || t == SbxCLASS_OBJECT )
		{
			if( rName.EqualsIgnoreCaseAscii( RTLNAME ) )
				pRes = pRtl;
		}
		if( !pRes )
			pRes = ((SbiStdObject*) (SbxObject*) pRtl)->Find( rName, t );
		if( pRes )
			pRes->SetFlag( SBX_EXTFOUND );
	}
	// Search module
	if( !pRes )
	  for( sal_uInt16 i = 0; i < pModules->Count(); i++ )
	{
		SbModule* p = (SbModule*) pModules->Get( i );
		if( p->IsVisible() )
		{
			// Remember modul fpr Main() call
			// or is the name equal?!?
			if( p->GetName().EqualsIgnoreCaseAscii( rName ) )
			{
				if( t == SbxCLASS_OBJECT || t == SbxCLASS_DONTCARE )
				{
					pRes = p; break;
				}
				pNamed = p;
			}
			// Only variables qualified by the Module Name e.g. Sheet1.foo
			// should work for Documant && Class type Modules
			sal_Int32 nType = p->GetModuleType();
			if ( nType == ModuleType::DOCUMENT || nType == ModuleType::FORM )
				continue;

			// otherwise check if the element is available
			// unset GBLSEARCH-Flag (due to Rekursion)
			sal_uInt16 nGblFlag = p->GetFlags() & SBX_GBLSEARCH;
			p->ResetFlag( SBX_GBLSEARCH );
			pRes = p->Find( rName, t );
			p->SetFlag( nGblFlag );
			if( pRes )
				break;
		}
	}
	if( !pRes && pNamed && ( t == SbxCLASS_METHOD || t == SbxCLASS_DONTCARE ) &&
		!pNamed->GetName().EqualsIgnoreCaseAscii( aMainStr ) )
			pRes = pNamed->Find( aMainStr, SbxCLASS_METHOD );
	if( !pRes )
		pRes = SbxObject::Find( rName, t );
	return pRes;
}

sal_Bool StarBASIC::Call( const String& rName, SbxArray* pParam )
{
	sal_Bool bRes = SbxObject::Call( rName, pParam );
	if( !bRes )
	{
		SbxError eErr = SbxBase::GetError();
		SbxBase::ResetError();
		if( eErr != SbxERR_OK )
			RTError( (SbError)eErr, 0, 0, 0 );
	}
	return bRes;
}

// Find method via name (e.g. query via BASIC IDE)
SbxBase* StarBASIC::FindSBXInCurrentScope( const String& rName )
{
	if( !pINST )
		return NULL;
	if( !pINST->pRun )
		return NULL;
	return pINST->pRun->FindElementExtern( rName );
}

// Preserve old interface
SbxVariable* StarBASIC::FindVarInCurrentScopy
( const String& rName, sal_uInt16& rStatus )
{
	rStatus = 1;              // Presumption: nothing found
	SbxVariable* pVar = NULL;
	SbxBase* pSbx = FindSBXInCurrentScope( rName );
	if( pSbx )
	{
		if( !pSbx->ISA(SbxMethod) && !pSbx->ISA(SbxObject) )
			pVar = PTR_CAST(SbxVariable,pSbx);
	}
	if( pVar )
		rStatus = 0;      // We found something
	return pVar;
}

void StarBASIC::QuitAndExitApplication()
{
    Stop();
    bQuit = sal_True;
}

void StarBASIC::Stop()
{
	SbiInstance* p = pINST;
	while( p )
	{
		p->Stop();
		p = p->pNext;
	}
}

sal_Bool StarBASIC::IsRunning()
{
	return sal_Bool( pINST != NULL );
}

/**************************************************************************
*
*    Object factories and others
*
**************************************************************************/

// Activation of an object. There is no need to access active objects
// with name via BASIC. If NULL is given, everything is activated.
void StarBASIC::ActivateObject( const String* pName, sal_Bool bActivate )
{
	if( pName )
	{
		SbxObject* p = (SbxObject*) SbxObject::Find( *pName, SbxCLASS_OBJECT );
		if( p )
		{
			if( bActivate )
				p->SetFlag( SBX_EXTSEARCH );
			else
				p->ResetFlag( SBX_EXTSEARCH );
		}
	}
	else
	{
		for( sal_uInt16 i = 0; i < GetObjects()->Count(); i++ )
		{
			SbxObject* p = (SbxObject*) GetObjects()->Get( i );
			if( bActivate )
				p->SetFlag( SBX_EXTSEARCH );
			else
				p->ResetFlag( SBX_EXTSEARCH );
		}
	}
}

/**************************************************************************
*
*    Debugging and error handling
*
**************************************************************************/

SbMethod* StarBASIC::GetActiveMethod( sal_uInt16 nLevel )
{
	if( pINST )
		return pINST->GetCaller( nLevel );
	else
		return NULL;
}

SbModule* StarBASIC::GetActiveModule()
{
	if( pINST && !IsCompilerError() )
		return pINST->GetActiveModule();
	else
		return pCMOD;
}

sal_uInt16 StarBASIC::BreakPoint( sal_uInt16 l, sal_uInt16 c1, sal_uInt16 c2 )
{
	SetErrorData( 0, l, c1, c2 );
	bBreak = sal_True;
	if( GetSbData()->aBreakHdl.IsSet() )
		return (sal_uInt16) GetSbData()->aBreakHdl.Call( this );
	else
		return BreakHdl();
}

sal_uInt16 StarBASIC::StepPoint( sal_uInt16 l, sal_uInt16 c1, sal_uInt16 c2 )
{
	SetErrorData( 0, l, c1, c2 );
	bBreak = sal_False;
	if( GetSbData()->aBreakHdl.IsSet() )
		return (sal_uInt16) GetSbData()->aBreakHdl.Call( this );
	else
		return BreakHdl();
}

sal_uInt16 __EXPORT StarBASIC::BreakHdl()
{
	return (sal_uInt16) ( aBreakHdl.IsSet()
		? aBreakHdl.Call( this ) : SbDEBUG_CONTINUE );
}

// Calls for error handler and break handler
sal_uInt16 StarBASIC::GetLine()		{ return GetSbData()->nLine; }
sal_uInt16 StarBASIC::GetCol1()		{ return GetSbData()->nCol1; }
sal_uInt16 StarBASIC::GetCol2()		{ return GetSbData()->nCol2; }

// Specific to error handler
SbError StarBASIC::GetErrorCode()		{ return GetSbData()->nCode; }
const String& StarBASIC::GetErrorText()	{ return GetSbData()->aErrMsg; }
sal_Bool StarBASIC::IsCompilerError()		{ return GetSbData()->bCompiler; }
void StarBASIC::SetGlobalLanguageMode( SbLanguageMode eLanguageMode )
{
	GetSbData()->eLanguageMode = eLanguageMode;
}
SbLanguageMode StarBASIC::GetGlobalLanguageMode()
{
	return GetSbData()->eLanguageMode;
}
// Local settings
SbLanguageMode StarBASIC::GetLanguageMode()
{
	// Use global settings?
	if( eLanguageMode == SB_LANG_GLOBAL )
		return GetSbData()->eLanguageMode;
	else
		return eLanguageMode;
}

// AB: 29.3.96
// Das Mapping zwischen alten und neuen Fehlercodes erfolgt, indem die Tabelle
// SFX_VB_ErrorTab[] durchsucht wird. Dies ist zwar nicht besonders performant,
// verbraucht aber viel weniger Speicher als entsprechende switch-Bloecke.
// Die Umrechnung von Fehlercodes muss nicht schnell sein, daher auch keine
// binaere Suche bei VB-Error -> SFX-Error.

// Neue Fehler-Codes auf alte, Sbx-Kompatible zurueckmappen
sal_uInt16 StarBASIC::GetVBErrorCode( SbError nError )
{
	sal_uInt16 nRet = 0;

	if( SbiRuntime::isVBAEnabled() )
	{
		switch( nError )
		{
			case SbERR_BASIC_ARRAY_FIX:
				return 10;
			case SbERR_BASIC_STRING_OVERFLOW:
				return 14;
			case SbERR_BASIC_EXPR_TOO_COMPLEX:
				return 16;
			case SbERR_BASIC_OPER_NOT_PERFORM:
				return 17;
			case SbERR_BASIC_TOO_MANY_DLL:
				return 47;
			case SbERR_BASIC_LOOP_NOT_INIT:
				return 92;
			default:
				nRet = 0;
		}
	}
	
	// Suchschleife
	const SFX_VB_ErrorItem* pErrItem;
	sal_uInt16 nIndex = 0;
	do
	{
		pErrItem = SFX_VB_ErrorTab + nIndex;
		if( pErrItem->nErrorSFX == nError )
		{
			nRet = pErrItem->nErrorVB;
			break;
		}
		nIndex++;
	}
	while( pErrItem->nErrorVB != 0xFFFF );		// bis End-Marke
	return nRet;
}

SbError	StarBASIC::GetSfxFromVBError( sal_uInt16 nError )
{
	SbError nRet = 0L;

	if( SbiRuntime::isVBAEnabled() )
	{
		switch( nError )
		{
			case 1:
			case 2:
			case 4:
			case 8:
			case 12:
			case 73:
				return 0L;
			case 10:
				return SbERR_BASIC_ARRAY_FIX;
			case 14:
				return SbERR_BASIC_STRING_OVERFLOW;
			case 16:
				return SbERR_BASIC_EXPR_TOO_COMPLEX;
			case 17:
				return SbERR_BASIC_OPER_NOT_PERFORM;
			case 47:
				return SbERR_BASIC_TOO_MANY_DLL;
			case 92:
				return SbERR_BASIC_LOOP_NOT_INIT;
			default:
				nRet = 0L;
		}
	}
	const SFX_VB_ErrorItem* pErrItem;
	sal_uInt16 nIndex = 0;
	do
	{
		pErrItem = SFX_VB_ErrorTab + nIndex;
		if( pErrItem->nErrorVB == nError )
		{
			nRet = pErrItem->nErrorSFX;
			break;
		}
		else if( pErrItem->nErrorVB > nError )
			break;				// kann nicht mehr gefunden werden

		nIndex++;
	}
	while( pErrItem->nErrorVB != 0xFFFF );		// bis End-Marke
	return nRet;
}

// Error- / Break-Daten setzen
void StarBASIC::SetErrorData
( SbError nCode, sal_uInt16 nLine, sal_uInt16 nCol1, sal_uInt16 nCol2 )
{
	SbiGlobals& aGlobals = *GetSbData();
	aGlobals.nCode = nCode;
	aGlobals.nLine = nLine;
	aGlobals.nCol1 = nCol1;
	aGlobals.nCol2 = nCol2;
}

//----------------------------------------------------------------
// Hilfsklasse zum Zugriff auf String SubResourcen einer Resource.
// Quelle: sfx2\source\doc\docfile.cxx (TLX)
struct BasicStringList_Impl : private Resource
{
	ResId aResId;

	BasicStringList_Impl( ResId& rErrIdP,  sal_uInt16 nId)
		: Resource( rErrIdP ),aResId(nId, *rErrIdP.GetResMgr() ){}
	~BasicStringList_Impl() { FreeResource(); }

	String GetString(){ return String( aResId ); }
	sal_Bool IsErrorTextAvailable( void )
		{ return IsAvailableRes(aResId.SetRT(RSC_STRING)); }
};
//----------------------------------------------------------------

// #60175 Flag, das bei Basic-Fehlern das Anziehen der SFX-Resourcen verhindert
static sal_Bool bStaticSuppressSfxResource = sal_False;

void StarBASIC::StaticSuppressSfxResource( sal_Bool bSuppress )
{
	bStaticSuppressSfxResource = bSuppress;
}

// Hack for #83750, use bStaticSuppressSfxResource as setup flag
sal_Bool runsInSetup( void )
{
	return bStaticSuppressSfxResource;
}


void StarBASIC::MakeErrorText( SbError nId, const String& aMsg )
{
	vos::OGuard aSolarGuard( Application::GetSolarMutex() );

	if( bStaticSuppressSfxResource )
	{
		GetSbData()->aErrMsg = String( RTL_CONSTASCII_USTRINGPARAM("No resource: Error message not available") );
		return;
	}

	sal_uInt16 nOldID = GetVBErrorCode( nId );

	// Hilfsklasse instanzieren
	BasResId aId( RID_BASIC_START );
	BasicStringList_Impl aMyStringList( aId, sal_uInt16(nId & ERRCODE_RES_MASK) );

	if( aMyStringList.IsErrorTextAvailable() )
	{
		// Merge Message mit Zusatztext
		String aMsg1 = aMyStringList.GetString();
		// Argument-Platzhalter durch %s ersetzen
		String aSrgStr( RTL_CONSTASCII_USTRINGPARAM("$(ARG1)") );
		sal_uInt16 nResult = aMsg1.Search( aSrgStr );

		if( nResult != STRING_NOTFOUND )
		{
			aMsg1.Erase( nResult, aSrgStr.Len() );
			aMsg1.Insert( aMsg, nResult );
		}
		GetSbData()->aErrMsg = aMsg1;
	}
	else if( nOldID != 0 )
	{
		String aStdMsg( RTL_CONSTASCII_USTRINGPARAM("Fehler ") );
		aStdMsg += String::CreateFromInt32( nOldID);
		aStdMsg += String( RTL_CONSTASCII_USTRINGPARAM(": Kein Fehlertext verfuegbar!") );
		GetSbData()->aErrMsg = aStdMsg;
	}
	else
		GetSbData()->aErrMsg = String::EmptyString();

}

sal_Bool StarBASIC::CError
	( SbError code, const String& rMsg, sal_uInt16 l, sal_uInt16 c1, sal_uInt16 c2 )
{
	vos::OGuard aSolarGuard( Application::GetSolarMutex() );

	// Compiler-Fehler waehrend der Laufzeit -> Programm anhalten
	if( IsRunning() )
	{
		// #109018 Check if running Basic is affected
		StarBASIC* pStartedBasic = pINST->GetBasic();
		if( pStartedBasic != this )
			return sal_False;

		Stop();
	}

	// Flag setzen, damit GlobalRunInit den Fehler mitbekommt
	GetSbData()->bGlobalInitErr = sal_True;

	// Fehlertext basteln
	MakeErrorText( code, rMsg );

	// Umsetzung des Codes fuer String-Transport in SFX-Error
	if( rMsg.Len() )
		code = (sal_uIntPtr)*new StringErrorInfo( code, String(rMsg) );

	SetErrorData( code, l, c1, c2 );
	GetSbData()->bCompiler = sal_True;
	sal_Bool bRet;
	if( GetSbData()->aErrHdl.IsSet() )
		bRet = (sal_Bool) GetSbData()->aErrHdl.Call( this );
	else
		bRet = ErrorHdl();
	GetSbData()->bCompiler = sal_False;		// nur sal_True fuer Error-Handler
	return bRet;
}

sal_Bool StarBASIC::RTError
	( SbError code, sal_uInt16 l, sal_uInt16 c1, sal_uInt16 c2 )
{
	return RTError( code, String(), l, c1, c2 );
}

sal_Bool StarBASIC::RTError( SbError code, const String& rMsg, sal_uInt16 l, sal_uInt16 c1, sal_uInt16 c2 )
{
	vos::OGuard aSolarGuard( Application::GetSolarMutex() );

	SbError c = code;
	if( (c & ERRCODE_CLASS_MASK) == ERRCODE_CLASS_COMPILER )
		c = 0;
	MakeErrorText( c, rMsg );

	// Umsetzung des Codes fuer String-Transport in SFX-Error
	if( rMsg.Len() )
	{
		// very confusing, even though MakeErrorText sets up the error text
		// seems that this is not used ( if rMsg already has content )
		// In the case of VBA MakeErrorText also formats the error to be alittle more 
		// like vba ( adds an error number etc )
		if ( SbiRuntime::isVBAEnabled() && ( code == SbERR_BASIC_COMPAT ) )
		{
			String aTmp = '\'';
			aTmp += String::CreateFromInt32( SbxErrObject::getUnoErrObject()->getNumber() );
			aTmp += String( RTL_CONSTASCII_USTRINGPARAM("\'\n") );
			aTmp +=  GetSbData()->aErrMsg.Len() ? GetSbData()->aErrMsg : rMsg;
			code = (sal_uIntPtr)*new StringErrorInfo( code, aTmp );
		}
		else
			code = (sal_uIntPtr)*new StringErrorInfo( code, String(rMsg) );
	}

	SetErrorData( code, l, c1, c2 );
	if( GetSbData()->aErrHdl.IsSet() )
		return (sal_Bool) GetSbData()->aErrHdl.Call( this );
	else
		return ErrorHdl();
}

void StarBASIC::Error( SbError n )
{
	Error( n, String() );
}

void StarBASIC::Error( SbError n, const String& rMsg )
{
	if( pINST )
		pINST->Error( n, rMsg );
}

void StarBASIC::FatalError( SbError n )
{
	if( pINST )
		pINST->FatalError( n );
}

void StarBASIC::FatalError( SbError _errCode, const String& _details )
{
	if( pINST )
		pINST->FatalError( _errCode, _details );
}

SbError StarBASIC::GetErrBasic()
{
	if( pINST )
		return pINST->GetErr();
	else
		return 0;
}

// #66536 Zusatz-Message fuer RTL-Funktion Error zugreifbar machen
String StarBASIC::GetErrorMsg()
{
	if( pINST )
		return pINST->GetErrorMsg();
	else
		return String();
}

sal_uInt16 StarBASIC::GetErl()
{
	if( pINST )
		return pINST->GetErl();
	else
		return 0;
}

sal_Bool __EXPORT StarBASIC::ErrorHdl()
{
	return (sal_Bool) ( aErrorHdl.IsSet()
		? aErrorHdl.Call( this ) : sal_False );
}

Link StarBASIC::GetGlobalErrorHdl()
{
	return GetSbData()->aErrHdl;
}

void StarBASIC::SetGlobalErrorHdl( const Link& rLink )
{
	GetSbData()->aErrHdl = rLink;
}


Link StarBASIC::GetGlobalBreakHdl()
{
	return GetSbData()->aBreakHdl;
}

void StarBASIC::SetGlobalBreakHdl( const Link& rLink )
{
	GetSbData()->aBreakHdl = rLink;
}

SbxArrayRef StarBASIC::getUnoListeners( void )
{
    if( !xUnoListeners.Is() )
        xUnoListeners = new SbxArray();
    return xUnoListeners;
}


/**************************************************************************
*
*	Laden und Speichern
*
**************************************************************************/

sal_Bool StarBASIC::LoadData( SvStream& r, sal_uInt16 nVer )
{
	if( !SbxObject::LoadData( r, nVer ) )
		return sal_False;

    // #95459 Delete dialogs, otherwise endless recursion
    // in SbxVarable::GetType() if dialogs are accessed
    sal_uInt16 nObjCount = pObjs->Count();
    SbxVariable** ppDeleteTab = new SbxVariable*[ nObjCount ];
	sal_uInt16 nObj;

	for( nObj = 0 ; nObj < nObjCount ; nObj++ )
	{
		SbxVariable* pVar = pObjs->Get( nObj );
		StarBASIC* pBasic = PTR_CAST( StarBASIC, pVar );
        ppDeleteTab[nObj] = pBasic ? NULL : pVar;
	}
	for( nObj = 0 ; nObj < nObjCount ; nObj++ )
	{
        SbxVariable* pVar = ppDeleteTab[nObj];
        if( pVar )
            pObjs->Remove( pVar );
    }
    delete[] ppDeleteTab;

	sal_uInt16 nMod;
	pModules->Clear();
	r >> nMod;
	for( sal_uInt16 i = 0; i < nMod; i++ )
	{
		SbModule* pMod = (SbModule*) SbxBase::Load( r );
		if( !pMod )
			return sal_False;
		else if( pMod->ISA(SbJScriptModule) )
		{
			// Ref zuweisen, damit pMod deleted wird
			SbModuleRef xRef = pMod;
		}
		else
		{
			pMod->SetParent( this );
			pModules->Put( pMod, i );
		}
	}
	// HACK fuer SFX-Mist!
	SbxVariable* p = Find( String( RTL_CONSTASCII_USTRINGPARAM("FALSE") ), SbxCLASS_PROPERTY );
	if( p )
		Remove( p );
	p = Find( String( RTL_CONSTASCII_USTRINGPARAM("TRUE") ), SbxCLASS_PROPERTY );
	if( p )
		Remove( p );
	// Ende des Hacks!
	// Suche ueber StarBASIC ist immer global
	DBG_ASSERT( IsSet( SBX_GBLSEARCH ), "Basic ohne GBLSEARCH geladen" );
	SetFlag( SBX_GBLSEARCH );
	return sal_True;
}

sal_Bool StarBASIC::StoreData( SvStream& r ) const
{
	if( !SbxObject::StoreData( r ) )
		return sal_False;
	r << (sal_uInt16) pModules->Count();
	for( sal_uInt16 i = 0; i < pModules->Count(); i++ )
	{
		SbModule* p = (SbModule*) pModules->Get( i );
		if( !p->Store( r ) )
			return sal_False;
	}
	return sal_True;
}

sal_Bool StarBASIC::LoadOldModules( SvStream& )
{
	return sal_False;
}

bool StarBASIC::GetUNOConstant( const sal_Char* _pAsciiName, ::com::sun::star::uno::Any& aOut )
{
    bool bRes = false;
    ::rtl::OUString sVarName( ::rtl::OUString::createFromAscii( _pAsciiName ) );
    SbUnoObject* pGlobs = dynamic_cast<SbUnoObject*>( Find( sVarName, SbxCLASS_DONTCARE ) );
    if ( pGlobs )
    {
        aOut = pGlobs->getUnoAny();
        bRes = true;
    }
    return bRes;
}

Reference< frame::XModel > StarBASIC::GetModelFromBasic( SbxObject* pBasic )
{
    OSL_PRECOND( pBasic != NULL, "getModelFromBasic: illegal call!" );
    if ( !pBasic )
        return NULL;

    // look for the ThisComponent variable, first in the parent (which
    // might be the document's Basic), then in the parent's parent (which might be
    // the application Basic)
    const ::rtl::OUString sThisComponent( RTL_CONSTASCII_USTRINGPARAM( "ThisComponent" ) );
    SbxVariable* pThisComponent = NULL;

    SbxObject* pLookup = pBasic->GetParent();
    while ( pLookup && !pThisComponent )
    {
        pThisComponent = pLookup->Find( sThisComponent, SbxCLASS_OBJECT );
        pLookup = pLookup->GetParent();
    }
    if ( !pThisComponent )
    {
        OSL_TRACE("Failed to get ThisComponent");
            // the application Basic, at the latest, should have this variable
        return NULL;
    }

    Any aThisComponentAny( sbxToUnoValue( pThisComponent ) );
    Reference< frame::XModel > xModel( aThisComponentAny, UNO_QUERY );
    if ( !xModel.is() )
    {
        // it's no XModel. Okay, ThisComponent nowadays is allowed to be a controller.
        Reference< frame::XController > xController( aThisComponentAny, UNO_QUERY );
        if ( xController.is() )
            xModel = xController->getModel();
    }

    if ( !xModel.is() )
        return NULL;

#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE("Have model ThisComponent points to url %s",
        ::rtl::OUStringToOString( xModel->getURL(),
            RTL_TEXTENCODING_ASCII_US ).pData->buffer );
#endif

    return xModel;
}


//========================================================================
// #118116 Implementation Collection object

TYPEINIT1(BasicCollection,SbxObject)

static const char pCountStr[]	= "Count";
static const char pAddStr[]		= "Add";
static const char pItemStr[]	= "Item";
static const char pRemoveStr[]	= "Remove";
static sal_uInt16 nCountHash = 0, nAddHash, nItemHash, nRemoveHash;

SbxInfoRef BasicCollection::xAddInfo = NULL;
SbxInfoRef BasicCollection::xItemInfo = NULL;

BasicCollection::BasicCollection( const XubString& rClass )
			 : SbxObject( rClass )
{
	if( !nCountHash )
	{
		nCountHash  = MakeHashCode( String::CreateFromAscii( pCountStr ) );
		nAddHash    = MakeHashCode( String::CreateFromAscii( pAddStr ) );
		nItemHash   = MakeHashCode( String::CreateFromAscii( pItemStr ) );
		nRemoveHash = MakeHashCode( String::CreateFromAscii( pRemoveStr ) );
	}
	Initialize();
	
}

BasicCollection::~BasicCollection()
{}

void BasicCollection::Clear()
{
	SbxObject::Clear();
	Initialize();
}

void BasicCollection::Initialize()
{
	xItemArray = new SbxArray();
	SetType( SbxOBJECT );
	SetFlag( SBX_FIXED );
	ResetFlag( SBX_WRITE );
	SbxVariable* p;
	p = Make( String::CreateFromAscii( pCountStr ), SbxCLASS_PROPERTY, SbxINTEGER );
	p->ResetFlag( SBX_WRITE );
	p->SetFlag( SBX_DONTSTORE );
	p = Make( String::CreateFromAscii( pAddStr ), SbxCLASS_METHOD, SbxEMPTY );
	p->SetFlag( SBX_DONTSTORE );
	p = Make( String::CreateFromAscii( pItemStr ), SbxCLASS_METHOD, SbxVARIANT );
	p->SetFlag( SBX_DONTSTORE );
	p = Make( String::CreateFromAscii( pRemoveStr ), SbxCLASS_METHOD, SbxEMPTY );
	p->SetFlag( SBX_DONTSTORE );
	if ( !xAddInfo.Is() ) 
	{
		xAddInfo = new SbxInfo;
		xAddInfo->AddParam(  String( RTL_CONSTASCII_USTRINGPARAM("Item") ), SbxVARIANT, SBX_READ );	
		xAddInfo->AddParam(  String( RTL_CONSTASCII_USTRINGPARAM("Key") ), SbxVARIANT, SBX_READ | SBX_OPTIONAL );	
		xAddInfo->AddParam(  String( RTL_CONSTASCII_USTRINGPARAM("Before") ), SbxVARIANT, SBX_READ | SBX_OPTIONAL );	
		xAddInfo->AddParam(  String( RTL_CONSTASCII_USTRINGPARAM("After") ), SbxVARIANT, SBX_READ | SBX_OPTIONAL );	
	}
	if ( !xItemInfo.Is() ) 
	{
		xItemInfo = new SbxInfo;
		xItemInfo->AddParam(  String( RTL_CONSTASCII_USTRINGPARAM("Index") ), SbxVARIANT, SBX_READ | SBX_OPTIONAL);	
	}
}

SbxVariable* BasicCollection::Find( const XubString& rName, SbxClassType t )
{
	SbxVariable* pFind = SbxObject::Find( rName, t );
	return pFind;
}

void BasicCollection::SFX_NOTIFY( SfxBroadcaster& rCst, const TypeId& rId1,
								const SfxHint& rHint, const TypeId& rId2 )
{
	const SbxHint* p = PTR_CAST(SbxHint,&rHint);
	if( p )
	{
		sal_uIntPtr nId = p->GetId();
		sal_Bool bRead  = sal_Bool( nId == SBX_HINT_DATAWANTED );
		sal_Bool bWrite = sal_Bool( nId == SBX_HINT_DATACHANGED );
		sal_Bool bRequestInfo = sal_Bool( nId == SBX_HINT_INFOWANTED );
		SbxVariable* pVar = p->GetVar();
		SbxArray* pArg = pVar->GetParameters();
		XubString aVarName( pVar->GetName() );
		if( bRead || bWrite )
		{
			if( pVar->GetHashCode() == nCountHash
				  && aVarName.EqualsIgnoreCaseAscii( pCountStr ) )
				pVar->PutLong( xItemArray->Count32() );
			else if( pVar->GetHashCode() == nAddHash
				  && aVarName.EqualsIgnoreCaseAscii( pAddStr ) )
				CollAdd( pArg );
			else if( pVar->GetHashCode() == nItemHash
				  && aVarName.EqualsIgnoreCaseAscii( pItemStr ) )
				CollItem( pArg );
			else if( pVar->GetHashCode() == nRemoveHash
				  && aVarName.EqualsIgnoreCaseAscii( pRemoveStr ) )
				CollRemove( pArg );
			else
				SbxObject::SFX_NOTIFY( rCst, rId1, rHint, rId2 );
			return;
		}
		else if ( bRequestInfo )
		{
			if( pVar->GetHashCode() == nAddHash
				  && aVarName.EqualsIgnoreCaseAscii( pAddStr ) )
				pVar->SetInfo( xAddInfo );
			else if( pVar->GetHashCode() == nItemHash
				  && aVarName.EqualsIgnoreCaseAscii( pItemStr ) )
				pVar->SetInfo( xItemInfo );
		}
	}
	SbxObject::SFX_NOTIFY( rCst, rId1, rHint, rId2 );
}

sal_Int32 BasicCollection::implGetIndex( SbxVariable* pIndexVar )
{
	sal_Int32 nIndex = -1;
	if( pIndexVar->GetType() == SbxSTRING )
		nIndex = implGetIndexForName( pIndexVar->GetString() );
	else
		nIndex = pIndexVar->GetLong() - 1;
	return nIndex;
}

sal_Int32 BasicCollection::implGetIndexForName( const String& rName )
{
	sal_Int32 nIndex = -1;
	sal_Int32 nCount = xItemArray->Count32();
	sal_Int32 nNameHash = MakeHashCode( rName );
	for( sal_Int32 i = 0 ; i < nCount ; i++ )
	{
		SbxVariable* pVar = xItemArray->Get32( i );
		if( pVar->GetHashCode() == nNameHash &&
			pVar->GetName().EqualsIgnoreCaseAscii( rName ) )
		{
			nIndex = i;
			break;
		}
	}
	return nIndex;
}

void BasicCollection::CollAdd( SbxArray* pPar_ )
{
	sal_uInt16 nCount = pPar_->Count();
	if( nCount < 2 || nCount > 5 )
	{
		SetError( SbxERR_WRONG_ARGS );
		return;
	}

	SbxVariable* pItem = pPar_->Get(1);
	if( pItem )
	{
		int nNextIndex;
		if( nCount < 4 )
		{
			nNextIndex = xItemArray->Count();
		}
		else
		{
			SbxVariable* pBefore = pPar_->Get(3);
			if( nCount == 5 )
			{
				if( !( pBefore->IsErr() || ( pBefore->GetType() == SbxEMPTY ) ) )
				{
					SetError( SbERR_BAD_ARGUMENT );
					return;
				}
				SbxVariable* pAfter = pPar_->Get(4);
				sal_Int32 nAfterIndex = implGetIndex( pAfter );
				if( nAfterIndex == -1 )
				{
					SetError( SbERR_BAD_ARGUMENT );
					return;
				}
				nNextIndex = nAfterIndex + 1;
			}
			else // if( nCount == 4 )
			{
				sal_Int32 nBeforeIndex = implGetIndex( pBefore );
				if( nBeforeIndex == -1 )
				{
					SetError( SbERR_BAD_ARGUMENT );
					return;
				}
				nNextIndex = nBeforeIndex;
			}
		}

		SbxVariableRef pNewItem = new SbxVariable( *pItem );
		if( nCount >= 3 )
		{
			SbxVariable* pKey = pPar_->Get(2);
			if( !( pKey->IsErr() || ( pKey->GetType() == SbxEMPTY ) ) )
			{
				if( pKey->GetType() != SbxSTRING )
				{
					SetError( SbERR_BAD_ARGUMENT );
					return;
				}
				String aKey = pKey->GetString();
				if( implGetIndexForName( aKey ) != -1 )
				{
					SetError( SbERR_BAD_ARGUMENT );
					return;
				}
				pNewItem->SetName( aKey );
			}
		}
		pNewItem->SetFlag( SBX_READWRITE );
		xItemArray->Insert32( pNewItem, nNextIndex );
	}
	else
	{
		SetError( SbERR_BAD_ARGUMENT );
		return;
	}
}

void BasicCollection::CollItem( SbxArray* pPar_ )
{
	if( pPar_->Count() != 2 )
	{
		SetError( SbxERR_WRONG_ARGS );
		return;
	}
	SbxVariable* pRes = NULL;
	SbxVariable* p = pPar_->Get( 1 );
	sal_Int32 nIndex = implGetIndex( p );
	if( nIndex >= 0 && nIndex < (sal_Int32)xItemArray->Count32() )
		pRes = xItemArray->Get32( nIndex );
	if( !pRes )
		SetError( SbERR_BAD_ARGUMENT );
	else
		*(pPar_->Get(0)) = *pRes;
}

void BasicCollection::CollRemove( SbxArray* pPar_ )
{
	if( pPar_ == NULL || pPar_->Count() != 2 )
	{
		SetError( SbxERR_WRONG_ARGS );
		return;
	}

	SbxVariable* p = pPar_->Get( 1 );
	sal_Int32 nIndex = implGetIndex( p );
	if( nIndex >= 0 && nIndex < (sal_Int32)xItemArray->Count32() )
	{
		xItemArray->Remove32( nIndex );

		// Correct for stack if necessary
		SbiInstance* pInst = pINST;
		SbiRuntime* pRT = pInst ? pInst->pRun : NULL;
		if( pRT )
		{
			SbiForStack* pStack = pRT->FindForStackItemForCollection( this );
			if( pStack != NULL )
			{
				if( pStack->nCurCollectionIndex >= nIndex )
					--pStack->nCurCollectionIndex;
			}
		}
	}
	else
		SetError( SbERR_BAD_ARGUMENT );
}

