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
#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif

#include <hintids.hxx>
#include <regionsw.hxx>
#include <svl/urihelper.hxx>
#include <svl/PasswordHelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <sfx2/passwd.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/request.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/docinsert.hxx>
#include <sfx2/filedlghelper.hxx>
#include <editeng/sizeitem.hxx>
#include <svtools/htmlcfg.hxx>

#include <comphelper/storagehelper.hxx>
#include <uitool.hxx>
#include <IMark.hxx>
#include <section.hxx>
#include <docary.hxx>
#include <doc.hxx>						// fuers SwSectionFmt-Array
#include <basesh.hxx>
#include <wdocsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <swundo.hxx>               	// fuer Undo-Ids
#include <column.hxx>
#include <fmtfsize.hxx>
#include <swunodef.hxx>
#include <shellio.hxx>

#include <helpid.h>
#include <cmdid.h>
#include <regionsw.hrc>
#include <comcore.hrc>
#include <globals.hrc>
#include <sfx2/bindings.hxx>
#include <svx/htmlmode.hxx>
#include <svx/dlgutil.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxdlg.hxx>
#include <svx/flagsdef.hxx>

using namespace ::com::sun::star;


// sw/inc/docary.hxx
SV_IMPL_PTRARR( SwSectionFmts, SwSectionFmtPtr )

#define FILE_NAME_LENGTH 17

static void   lcl_ReadSections( SfxMedium& rMedium, ComboBox& rBox );

void lcl_FillList( SwWrtShell& rSh, ComboBox& rSubRegions, ComboBox* pAvailNames, const SwSectionFmt* pNewFmt )
{
    const SwSectionFmt* pFmt;
    if( !pNewFmt )
    {
        sal_uInt16 nCount = rSh.GetSectionFmtCount();
        for(sal_uInt16 i=0;i<nCount;i++)
        {
            SectionType eTmpType;
            if( !(pFmt = &rSh.GetSectionFmt(i))->GetParent() &&
                    pFmt->IsInNodesArr() &&
                    (eTmpType = pFmt->GetSection()->GetType()) != TOX_CONTENT_SECTION
                    && TOX_HEADER_SECTION != eTmpType )
            {
                    String* pString =
                        new String(pFmt->GetSection()->GetSectionName());
                    if(pAvailNames)
                        pAvailNames->InsertEntry(*pString);
                    rSubRegions.InsertEntry(*pString);
                    lcl_FillList( rSh, rSubRegions, pAvailNames, pFmt );
            }
        }
    }
    else
    {
        SwSections aTmpArr;
        sal_uInt16 nCnt = pNewFmt->GetChildSections(aTmpArr,SORTSECT_POS);
        if( nCnt )
        {
            SectionType eTmpType;
            for( sal_uInt16 n = 0; n < nCnt; ++n )
                if( (pFmt = aTmpArr[n]->GetFmt())->IsInNodesArr()&&
                    (eTmpType = pFmt->GetSection()->GetType()) != TOX_CONTENT_SECTION
                    && TOX_HEADER_SECTION != eTmpType )
                {
                    String* pString =
                        new String(pFmt->GetSection()->GetSectionName());
                    if(pAvailNames)
                        pAvailNames->InsertEntry(*pString);
                    rSubRegions.InsertEntry(*pString);
                    lcl_FillList( rSh, rSubRegions, pAvailNames, pFmt );
                }
        }
    }
}

void lcl_FillSubRegionList( SwWrtShell& rSh, ComboBox& rSubRegions, ComboBox* pAvailNames )
{
    lcl_FillList( rSh, rSubRegions, pAvailNames, 0 );
    IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
    for( IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getMarksBegin();
        ppMark != pMarkAccess->getMarksEnd();
        ppMark++)
    {
        const ::sw::mark::IMark* pBkmk = ppMark->get(); 
        if( pBkmk->IsExpanded() )
            rSubRegions.InsertEntry( pBkmk->GetName() );
    }
}

/* -----------------25.06.99 15:38-------------------

 --------------------------------------------------*/
class SwTestPasswdDlg : public SfxPasswordDialog
{
public:
		SwTestPasswdDlg(Window* pParent) :
		SfxPasswordDialog(pParent)
		{
			SetHelpId(HID_DLG_PASSWD_SECTION);
		}
};

/*----------------------------------------------------------------------------
 Beschreibung: User Data Klasse fuer Bereichsinformationen
----------------------------------------------------------------------------*/

class SectRepr
{
private:
    SwSectionData           m_SectionData;
    SwFmtCol                m_Col;
    SvxBrushItem            m_Brush;
    SwFmtFtnAtTxtEnd        m_FtnNtAtEnd;
    SwFmtEndAtTxtEnd        m_EndNtAtEnd;
    SwFmtNoBalancedColumns  m_Balance;
    SvxFrameDirectionItem   m_FrmDirItem;
    SvxLRSpaceItem          m_LRSpaceItem;
    sal_uInt16                  m_nArrPos;
    // zeigt an, ob evtl. Textinhalt im Bereich ist
    bool                    m_bContent  : 1;
    // fuer Multiselektion erst markieren, dann mit der TreeListBox arbeiten!
    bool                    m_bSelected : 1;
    uno::Sequence<sal_Int8> m_TempPasswd;

public:
    SectRepr(sal_uInt16 nPos, SwSection& rSect);
    bool    operator==(SectRepr& rSectRef) const
            { return m_nArrPos == rSectRef.GetArrPos(); }

    bool    operator< (SectRepr& rSectRef) const
            { return m_nArrPos <  rSectRef.GetArrPos(); }

    SwSectionData &     GetSectionData()        { return m_SectionData; }
    SwSectionData const&GetSectionData() const  { return m_SectionData; }
    SwFmtCol&               GetCol()            { return m_Col; }
    SvxBrushItem&           GetBackground()     { return m_Brush; }
    SwFmtFtnAtTxtEnd&       GetFtnNtAtEnd()     { return m_FtnNtAtEnd; }
    SwFmtEndAtTxtEnd&       GetEndNtAtEnd()     { return m_EndNtAtEnd; }
    SwFmtNoBalancedColumns& GetBalance()        { return m_Balance; }
    SvxFrameDirectionItem&  GetFrmDir()         { return m_FrmDirItem; }
    SvxLRSpaceItem&         GetLRSpace()        { return m_LRSpaceItem; }

    sal_uInt16              GetArrPos() const { return m_nArrPos; }
    String              GetFile() const;
    String              GetSubRegion() const;
    void                SetFile(String const& rFile);
    void                SetFilter(String const& rFilter);
    void                SetSubRegion(String const& rSubRegion);

    bool                IsContent() { return m_bContent; }
    void                SetContent(bool const bValue) { m_bContent = bValue; }

    void                SetSelected() { m_bSelected = true; }
    bool                IsSelected() const { return m_bSelected; }

    uno::Sequence<sal_Int8> & GetTempPasswd() { return m_TempPasswd; }
    void SetTempPasswd(const uno::Sequence<sal_Int8> & rPasswd)
        { m_TempPasswd = rPasswd; }
};


SV_IMPL_OP_PTRARR_SORT( SectReprArr, SectReprPtr )

SectRepr::SectRepr( sal_uInt16 nPos, SwSection& rSect )
    : m_SectionData( rSect )
    , m_Brush( RES_BACKGROUND )
    , m_FrmDirItem( FRMDIR_ENVIRONMENT, RES_FRAMEDIR )
    , m_LRSpaceItem( RES_LR_SPACE )
    , m_nArrPos(nPos)
    , m_bContent(m_SectionData.GetLinkFileName().Len() == 0)
    , m_bSelected(false)
{
	SwSectionFmt *pFmt = rSect.GetFmt();
	if( pFmt )
	{
        m_Col = pFmt->GetCol();
        m_Brush = pFmt->GetBackground();
        m_FtnNtAtEnd = pFmt->GetFtnAtTxtEnd();
        m_EndNtAtEnd = pFmt->GetEndAtTxtEnd();
        m_Balance.SetValue(pFmt->GetBalancedColumns().GetValue());
        m_FrmDirItem = pFmt->GetFrmDir();
        m_LRSpaceItem = pFmt->GetLRSpace();
	}
}

void SectRepr::SetFile( const String& rFile )
{
	String sNewFile( INetURLObject::decode( rFile, INET_HEX_ESCAPE,
						   				INetURLObject::DECODE_UNAMBIGUOUS,
										RTL_TEXTENCODING_UTF8 ));
    String sOldFileName( m_SectionData.GetLinkFileName() );
    String sSub( sOldFileName.GetToken( 2, sfx2::cTokenSeperator ) );

	if( rFile.Len() || sSub.Len() )
	{
        sNewFile += sfx2::cTokenSeperator;
		if( rFile.Len() ) // Filter nur mit FileName
            sNewFile += sOldFileName.GetToken( 1, sfx2::cTokenSeperator );

        sNewFile += sfx2::cTokenSeperator;
		sNewFile +=	sSub;
	}

    m_SectionData.SetLinkFileName( sNewFile );

	if( rFile.Len() || sSub.Len() )
    {
        m_SectionData.SetType( FILE_LINK_SECTION );
    }
    else
    {
        m_SectionData.SetType( CONTENT_SECTION );
    }
}


void SectRepr::SetFilter( const String& rFilter )
{
	String sNewFile;
    String sOldFileName( m_SectionData.GetLinkFileName() );
    String sFile( sOldFileName.GetToken( 0, sfx2::cTokenSeperator ) );
    String sSub( sOldFileName.GetToken( 2, sfx2::cTokenSeperator ) );

	if( sFile.Len() )
        (((( sNewFile = sFile ) += sfx2::cTokenSeperator ) += rFilter )
                                += sfx2::cTokenSeperator ) += sSub;
	else if( sSub.Len() )
        (( sNewFile = sfx2::cTokenSeperator ) += sfx2::cTokenSeperator ) += sSub;

    m_SectionData.SetLinkFileName( sNewFile );

	if( sNewFile.Len() )
    {
        m_SectionData.SetType( FILE_LINK_SECTION );
    }
}

void SectRepr::SetSubRegion(const String& rSubRegion)
{
	String sNewFile;
    String sOldFileName( m_SectionData.GetLinkFileName() );
    String sFilter( sOldFileName.GetToken( 1, sfx2::cTokenSeperator ) );
    sOldFileName = sOldFileName.GetToken( 0, sfx2::cTokenSeperator );

	if( rSubRegion.Len() || sOldFileName.Len() )
        (((( sNewFile = sOldFileName ) += sfx2::cTokenSeperator ) += sFilter )
                                       += sfx2::cTokenSeperator ) += rSubRegion;

    m_SectionData.SetLinkFileName( sNewFile );

	if( rSubRegion.Len() || sOldFileName.Len() )
    {
        m_SectionData.SetType( FILE_LINK_SECTION );
    }
    else
    {
        m_SectionData.SetType( CONTENT_SECTION );
    }
}


String SectRepr::GetFile() const
{
    String sLinkFile( m_SectionData.GetLinkFileName() );
	if( sLinkFile.Len() )
	{
        if (DDE_LINK_SECTION == m_SectionData.GetType())
        {
            sal_uInt16 n = sLinkFile.SearchAndReplace( sfx2::cTokenSeperator, ' ' );
            sLinkFile.SearchAndReplace( sfx2::cTokenSeperator, ' ',  n );
		}
		else
            sLinkFile = INetURLObject::decode( sLinkFile.GetToken( 0,
                                               sfx2::cTokenSeperator ),
										INET_HEX_ESCAPE,
						   				INetURLObject::DECODE_UNAMBIGUOUS,
										RTL_TEXTENCODING_UTF8 );
	}
	return sLinkFile;
}


String SectRepr::GetSubRegion() const
{
    String sLinkFile( m_SectionData.GetLinkFileName() );
	if( sLinkFile.Len() )
        sLinkFile = sLinkFile.GetToken( 2, sfx2::cTokenSeperator );
	return sLinkFile;
}



/*----------------------------------------------------------------------------
 Beschreibung: Dialog Bearbeiten Bereiche
----------------------------------------------------------------------------*/

//---------------------------------------------------------------------

SwEditRegionDlg::SwEditRegionDlg( Window* pParent, SwWrtShell& rWrtSh )
	: SfxModalDialog( pParent, SW_RES(MD_EDIT_REGION) ),
    aNameFL             ( this, SW_RES( FL_NAME ) ),
    aCurName            ( this, SW_RES( ED_RANAME ) ),
    aTree               ( this, SW_RES( TLB_SECTION )),
    aLinkFL             ( this, SW_RES( FL_LINK ) ),
    aFileCB             ( this, SW_RES( CB_FILE ) ),
	aDDECB              ( this, SW_RES( CB_DDE ) ) ,
    aFileNameFT         ( this, SW_RES( FT_FILE ) ) ,
	aDDECommandFT       ( this, SW_RES( FT_DDE ) ) ,
	aFileNameED         ( this, SW_RES( ED_FILE ) ),
    aFilePB             ( this, SW_RES( PB_FILE ) ),
    aSubRegionFT        ( this, SW_RES( FT_SUBREG ) ) ,
	aSubRegionED        ( this, SW_RES( LB_SUBREG ) ) ,
    bSubRegionsFilled( false ),

    aProtectFL          ( this, SW_RES( FL_PROTECT ) ),
    aProtectCB          ( this, SW_RES( CB_PROTECT ) ),
	aPasswdCB			( this, SW_RES( CB_PASSWD ) ),
    aPasswdPB           ( this, SW_RES( PB_PASSWD ) ),

    aHideFL             ( this, SW_RES( FL_HIDE ) ),
    aHideCB             ( this, SW_RES( CB_HIDE ) ),
    aConditionFT        ( this, SW_RES( FT_CONDITION ) ),
    aConditionED        ( this, SW_RES( ED_CONDITION ) ),

    // --> FME 2004-06-22 #114856# edit in readonly sections
    aPropertiesFL       ( this, SW_RES( FL_PROPERTIES ) ),
    aEditInReadonlyCB   ( this, SW_RES( CB_EDIT_IN_READONLY ) ),
    // <--

    aOK                 ( this, SW_RES( PB_OK ) ),
    aCancel             ( this, SW_RES( PB_CANCEL ) ),
	aOptionsPB  		( this, SW_RES( PB_OPTIONS ) ),
	aDismiss			( this, SW_RES( CB_DISMISS ) ),
	aHelp               ( this, SW_RES( PB_HELP ) ),

    aImageIL            (       SW_RES(IL_BITMAPS)),
    aImageILH           (       SW_RES(ILH_BITMAPS)),

    rSh( rWrtSh ),
    pAktEntry( 0 ),
    m_pDocInserter        ( NULL ),
    m_pOldDefDlgParent    ( NULL ),
    bDontCheckPasswd    ( sal_True)
{
	FreeResource();

    bWeb = 0 != PTR_CAST( SwWebDocShell, rSh.GetView().GetDocShell() );

	aTree.SetSelectHdl		( LINK( this, SwEditRegionDlg, GetFirstEntryHdl));
	aTree.SetDeselectHdl	( LINK( this, SwEditRegionDlg, DeselectHdl));
	aCurName.SetModifyHdl	( LINK( this, SwEditRegionDlg, NameEditHdl));
	aConditionED.SetModifyHdl( LINK( this, SwEditRegionDlg, ConditionEditHdl));
	aOK.SetClickHdl			( LINK( this, SwEditRegionDlg, OkHdl));
	aPasswdCB.SetClickHdl	( LINK( this, SwEditRegionDlg, ChangePasswdHdl));
    aPasswdPB.SetClickHdl   ( LINK( this, SwEditRegionDlg, ChangePasswdHdl));
	aHideCB.SetClickHdl		( LINK( this, SwEditRegionDlg, ChangeHideHdl));
    // --> FME 2004-06-22 #114856# edit in readonly sections
    aEditInReadonlyCB.SetClickHdl ( LINK( this, SwEditRegionDlg, ChangeEditInReadonlyHdl));
    // <--

    aOptionsPB.Show();
	aOptionsPB.SetClickHdl	( LINK( this, SwEditRegionDlg, OptionsHdl));
	aProtectCB.SetClickHdl	( LINK( this, SwEditRegionDlg, ChangeProtectHdl));
	aDismiss.SetClickHdl	( LINK( this, SwEditRegionDlg, ChangeDismissHdl));
	aFileCB.SetClickHdl     ( LINK( this, SwEditRegionDlg, UseFileHdl ));
	aFilePB.SetClickHdl     ( LINK( this, SwEditRegionDlg, FileSearchHdl ));
	aFileNameED.SetModifyHdl( LINK( this, SwEditRegionDlg, FileNameHdl ));
	aSubRegionED.SetModifyHdl( LINK( this, SwEditRegionDlg, FileNameHdl ));
    aSubRegionED.AddEventListener( LINK( this, SwEditRegionDlg, SubRegionEventHdl ));
    aSubRegionED.EnableAutocomplete( sal_True, sal_True );

	aTree.SetHelpId(HID_REGION_TREE);
	aTree.SetSelectionMode( MULTIPLE_SELECTION );
	aTree.SetStyle(aTree.GetStyle()|WB_HASBUTTONSATROOT|WB_CLIPCHILDREN|WB_HSCROLL);
	aTree.SetSpaceBetweenEntries(0);

	if(bWeb)
	{
        aConditionFT         .Hide();
        aConditionED    .Hide();
		aPasswdCB		.Hide();
		aHideCB			.Hide();

		aDDECB              .Hide();
		aDDECommandFT       .Hide();
	}

	aDDECB.SetClickHdl		( LINK( this, SwEditRegionDlg, DDEHdl ));

    //Ermitteln der vorhandenen Bereiche
	pCurrSect = rSh.GetCurrSection();
	RecurseList( 0, 0 );
	//falls der Cursor nicht in einem Bereich steht,
	//wird immer der erste selektiert
	if( !aTree.FirstSelected() && aTree.First() )
		aTree.Select( aTree.First() );
	aTree.Show();
    bDontCheckPasswd = sal_False;

	aPasswdPB.SetAccessibleRelationMemberOf(&aProtectFL);
	aPasswdPB.SetAccessibleRelationLabeledBy(&aPasswdCB);
    aSubRegionED.SetAccessibleName(aSubRegionFT.GetText());
}
/* -----------------------------26.04.01 14:56--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwEditRegionDlg::CheckPasswd(CheckBox* pBox)
{
    if(bDontCheckPasswd)
        return sal_True;
    sal_Bool bRet = sal_True;
    SvLBoxEntry* pEntry = aTree.FirstSelected();
    while( pEntry )
	{
		SectReprPtr pRepr = (SectReprPtr)pEntry->GetUserData();
        if (!pRepr->GetTempPasswd().getLength()
            && pRepr->GetSectionData().GetPassword().getLength())
        {
            SwTestPasswdDlg aPasswdDlg(this);
            bRet = sal_False;
            if (aPasswdDlg.Execute())
            {
                String sNewPasswd( aPasswdDlg.GetPassword() );
                UNO_NMSPC::Sequence <sal_Int8 > aNewPasswd;
                SvPasswordHelper::GetHashPassword( aNewPasswd, sNewPasswd );
                if (SvPasswordHelper::CompareHashPassword(
                        pRepr->GetSectionData().GetPassword(), sNewPasswd))
                {
                    pRepr->SetTempPasswd(aNewPasswd);
                    bRet = sal_True;
                }
                else
                {
                    InfoBox(this, SW_RES(REG_WRONG_PASSWORD)).Execute();
                }
            }
        }
        pEntry = aTree.NextSelected(pEntry);
    }
    if(!bRet && pBox)
    {
        //reset old button state
        if(pBox->IsTriStateEnabled())
            pBox->SetState(pBox->IsChecked() ? STATE_NOCHECK : STATE_DONTKNOW);
        else
            pBox->Check(!pBox->IsChecked());
    }

    return bRet;
}
/*---------------------------------------------------------------------
	Beschreibung: Durchsuchen nach Child-Sections, rekursiv
---------------------------------------------------------------------*/

void SwEditRegionDlg::RecurseList( const SwSectionFmt* pFmt, SvLBoxEntry* pEntry )
{
    SwSection* pSect = 0;
	SvLBoxEntry* pSelEntry = 0;

	if (!pFmt)
	{
		sal_uInt16 nCount=rSh.GetSectionFmtCount();
		for ( sal_uInt16 n=0; n < nCount; n++ )
		{
			SectionType eTmpType;
			if( !( pFmt = &rSh.GetSectionFmt(n))->GetParent() &&
				pFmt->IsInNodesArr() &&
				(eTmpType = pFmt->GetSection()->GetType()) != TOX_CONTENT_SECTION
				&& TOX_HEADER_SECTION != eTmpType )
			{
				SectRepr* pSectRepr = new SectRepr( n,
											*(pSect=pFmt->GetSection()) );
                Image aImg = BuildBitmap( pSect->IsProtect(),pSect->IsHidden(), sal_False);
                pEntry = aTree.InsertEntry(pSect->GetSectionName(), aImg, aImg);
                Image aHCImg = BuildBitmap( pSect->IsProtect(),pSect->IsHidden(), sal_True);
                aTree.SetExpandedEntryBmp(pEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);
                aTree.SetCollapsedEntryBmp(pEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);
                pEntry->SetUserData(pSectRepr);
				RecurseList( pFmt, pEntry );
				if (pEntry->HasChilds())
					aTree.Expand(pEntry);
				if (pCurrSect==pSect)
					aTree.Select(pEntry);
			}
		}
	}
	else
	{
		SwSections aTmpArr;
		SvLBoxEntry* pNEntry;
		sal_uInt16 nCnt = pFmt->GetChildSections(aTmpArr,SORTSECT_POS);
		if( nCnt )
		{
			for( sal_uInt16 n = 0; n < nCnt; ++n )
			{
				SectionType eTmpType;
                pFmt = aTmpArr[n]->GetFmt();
				if( pFmt->IsInNodesArr() &&
					(eTmpType = pFmt->GetSection()->GetType()) != TOX_CONTENT_SECTION
					&& TOX_HEADER_SECTION != eTmpType )
				{
					pSect=aTmpArr[n];
					SectRepr* pSectRepr=new SectRepr(
									FindArrPos( pSect->GetFmt() ), *pSect );
                    Image aImage = BuildBitmap( pSect->IsProtect(),
                                            pSect->IsHidden(), sal_False);
                    pNEntry = aTree.InsertEntry(
                        pSect->GetSectionName(), aImage, aImage, pEntry);
                    Image aHCImg = BuildBitmap( pSect->IsProtect(),pSect->IsHidden(), sal_True);
                    aTree.SetExpandedEntryBmp(pEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);
                    aTree.SetCollapsedEntryBmp(pEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);
                    pNEntry->SetUserData(pSectRepr);
					RecurseList( aTmpArr[n]->GetFmt(), pNEntry );
					if( pNEntry->HasChilds())
						aTree.Expand(pNEntry);
					if (pCurrSect==pSect)
						pSelEntry = pNEntry;
				}
			}
		}
	}
	if(0 != pSelEntry)
	{
		aTree.MakeVisible(pSelEntry);
		aTree.Select(pSelEntry);
	}
}
/*---------------------------------------------------------------------

---------------------------------------------------------------------*/

sal_uInt16 SwEditRegionDlg::FindArrPos(const SwSectionFmt* pFmt )
{
	sal_uInt16 nCount=rSh.GetSectionFmtCount();
	for (sal_uInt16 i=0;i<nCount;i++)
		if (pFmt==&rSh.GetSectionFmt(i))
			return i;

	DBG_ERROR(  "SectionFormat nicht in der Liste" );
	return USHRT_MAX;
}
/*---------------------------------------------------------------------
 Beschreibung:
---------------------------------------------------------------------*/

SwEditRegionDlg::~SwEditRegionDlg( )
{
	SvLBoxEntry* pEntry = aTree.First();
	while( pEntry )
	{
		delete (SectRepr*)pEntry->GetUserData();
		pEntry = aTree.Next( pEntry );
	}

	aSectReprArr.DeleteAndDestroy( 0, aSectReprArr.Count() );
    delete m_pDocInserter;
}
/* -----------------------------09.10.2001 15:41------------------------------

 ---------------------------------------------------------------------------*/
void    SwEditRegionDlg::SelectSection(const String& rSectionName)
{
    SvLBoxEntry* pEntry = aTree.First();
    while(pEntry)
    {
        SectReprPtr pRepr = (SectReprPtr)pEntry->GetUserData();
        if (pRepr->GetSectionData().GetSectionName() == rSectionName)
            break;
        pEntry = aTree.Next(pEntry);
    }
    if(pEntry)
    {
        aTree.SelectAll( sal_False);
        aTree.Select(pEntry);
        aTree.MakeVisible(pEntry);
    }
}
/*---------------------------------------------------------------------
	Beschreibung: 	Selektierte Eintrag in der TreeListBox wird im
					Edit-Fenster angezeigt
					Bei Multiselektion werden einige Controls disabled
---------------------------------------------------------------------*/

IMPL_LINK( SwEditRegionDlg, GetFirstEntryHdl, SvTreeListBox *, pBox )
{
    bDontCheckPasswd = sal_True;
    SvLBoxEntry* pEntry=pBox->FirstSelected();
	aHideCB		.Enable(sal_True);
    // --> FME 2004-06-22 #114856# edit in readonly sections
    aEditInReadonlyCB.Enable(sal_True);
    // <--
    aProtectCB  .Enable(sal_True);
	aFileCB		.Enable(sal_True);
    UNO_NMSPC::Sequence <sal_Int8> aCurPasswd;
	if( 1 < pBox->GetSelectionCount() )
	{
		aHideCB.EnableTriState( sal_True );
		aProtectCB.EnableTriState( sal_True );
        // --> FME 2004-06-22 #114856# edit in readonly sections
        aEditInReadonlyCB.EnableTriState ( sal_True );
        // <--
        aFileCB.EnableTriState( sal_True );

        bool bHiddenValid       = true;
        bool bProtectValid      = true;
        bool bConditionValid    = true;
        // --> FME 2004-06-22 #114856# edit in readonly sections
        bool bEditInReadonlyValid = true;
        bool bEditInReadonly    = true;
        // <--
        bool bHidden            = true;
        bool bProtect           = true;
		String sCondition;
		sal_Bool bFirst 			= sal_True;
		sal_Bool bFileValid 		= sal_True;
		sal_Bool bFile 				= sal_True;
        sal_Bool bPasswdValid       = sal_True;

		while( pEntry )
		{
			SectRepr* pRepr=(SectRepr*) pEntry->GetUserData();
            SwSectionData const& rData( pRepr->GetSectionData() );
			if(bFirst)
			{
                sCondition      = rData.GetCondition();
                bHidden         = rData.IsHidden();
                bProtect        = rData.IsProtectFlag();
                // --> FME 2004-06-22 #114856# edit in readonly sections
                bEditInReadonly = rData.IsEditInReadonlyFlag();
                // <--
                bFile           = (rData.GetType() != CONTENT_SECTION);
                aCurPasswd      = rData.GetPassword();
			}
			else
			{
                String sTemp(rData.GetCondition());
				if(sCondition != sTemp)
					bConditionValid = sal_False;
                bHiddenValid      = (bHidden == rData.IsHidden());
                bProtectValid     = (bProtect == rData.IsProtectFlag());
                // --> FME 2004-06-22 #114856# edit in readonly sections
                bEditInReadonlyValid =
                    (bEditInReadonly == rData.IsEditInReadonlyFlag());
                // <--
                bFileValid        = (bFile ==
                    (rData.GetType() != CONTENT_SECTION));
                bPasswdValid      = (aCurPasswd == rData.GetPassword());
			}
			pEntry = pBox->NextSelected(pEntry);
			bFirst = sal_False;
		}

		aHideCB.SetState( !bHiddenValid ? STATE_DONTKNOW :
					bHidden ? STATE_CHECK : STATE_NOCHECK);
		aProtectCB.SetState( !bProtectValid ? STATE_DONTKNOW :
					bProtect ? STATE_CHECK : STATE_NOCHECK);
        // --> FME 2004-06-22 #114856# edit in readonly sections
        aEditInReadonlyCB.SetState( !bEditInReadonlyValid ? STATE_DONTKNOW :
                    bEditInReadonly ? STATE_CHECK : STATE_NOCHECK);
        // <--
        aFileCB.SetState(!bFileValid ? STATE_DONTKNOW :
					bFile ? STATE_CHECK : STATE_NOCHECK);

		if(bConditionValid)
			aConditionED.SetText(sCondition);
		else
		{
//			aConditionED.SetText(aEmptyStr);
            aConditionFT.Enable(sal_False);
            aConditionED.Enable(sal_False);
		}

		aFilePB.Enable(sal_False);
		aFileNameFT	.Enable(sal_False);
		aFileNameED	.Enable(sal_False);
		aSubRegionFT.Enable(sal_False);
		aSubRegionED.Enable(sal_False);
//        aNameFT     .Enable(sal_False);
		aCurName	.Enable(sal_False);
		aOptionsPB	.Enable(sal_False);
		aDDECB   			.Enable(sal_False);
		aDDECommandFT       .Enable(sal_False);
        sal_Bool bPasswdEnabled = aProtectCB.GetState() == STATE_CHECK;
        aPasswdCB.Enable(bPasswdEnabled);
        aPasswdPB.Enable(bPasswdEnabled);
        if(!bPasswdValid)
        {
            pEntry = pBox->FirstSelected();
            pBox->SelectAll( sal_False );
            pBox->Select( pEntry );
            GetFirstEntryHdl(pBox);
            return 0;
        }
        else
            aPasswdCB.Check(aCurPasswd.getLength() > 0);
	}
	else if (pEntry )
	{
//        aNameFT     .Enable(sal_True);
		aCurName	.Enable(sal_True);
		aOptionsPB	.Enable(sal_True);
		SectRepr* pRepr=(SectRepr*) pEntry->GetUserData();
        SwSectionData const& rData( pRepr->GetSectionData() );
        aConditionED.SetText(rData.GetCondition());
		aHideCB.Enable();
        aHideCB.SetState((rData.IsHidden()) ? STATE_CHECK : STATE_NOCHECK);
		sal_Bool bHide = STATE_CHECK == aHideCB.GetState();
        aConditionED.Enable(bHide);
        aConditionFT.Enable(bHide);
        aPasswdCB.Check(rData.GetPassword().getLength() > 0);

		aOK.Enable();
		aPasswdCB.Enable();
		aCurName.SetText(pBox->GetEntryText(pEntry));
		aCurName.Enable();
		aDismiss.Enable();
		String aFile = pRepr->GetFile();
		String sSub = pRepr->GetSubRegion();
        bSubRegionsFilled = false;
        aSubRegionED.Clear();
		if(aFile.Len()||sSub.Len())
		{
			aFileCB.Check(sal_True);
			aFileNameED.SetText(aFile);
			aSubRegionED.SetText(sSub);
            aDDECB.Check(rData.GetType() == DDE_LINK_SECTION);
		}
		else
		{
			aFileCB.Check(sal_False);
			aFileNameED.SetText(aFile);
			aDDECB.Enable(sal_False);
			aDDECB.Check(sal_False);
		}
		UseFileHdl(&aFileCB);
		DDEHdl( &aDDECB );
        aProtectCB.SetState((rData.IsProtectFlag())
                ? STATE_CHECK : STATE_NOCHECK);
		aProtectCB.Enable();

        // --> FME 2004-06-22 #114856# edit in readonly sections
        aEditInReadonlyCB.SetState((rData.IsEditInReadonlyFlag())
                ? STATE_CHECK : STATE_NOCHECK);
        aEditInReadonlyCB.Enable();
        // <--

        sal_Bool bPasswdEnabled = aProtectCB.IsChecked();
        aPasswdCB.Enable(bPasswdEnabled);
        aPasswdPB.Enable(bPasswdEnabled);
	}
    bDontCheckPasswd = sal_False;
	return 0;
}
/*-----------------28.06.97 09:19-------------------

--------------------------------------------------*/
IMPL_LINK( SwEditRegionDlg, DeselectHdl, SvTreeListBox *, pBox )
{
	if( !pBox->GetSelectionCount() )
	{
		aHideCB		.Enable(sal_False);
		aProtectCB	.Enable(sal_False);
        // --> FME 2004-06-22 #114856# edit in readonly sections
        aEditInReadonlyCB.Enable(sal_False);
        // <--
        aPasswdCB   .Enable(sal_False);
        aPasswdCB   .Enable(sal_False);
        aConditionFT     .Enable(sal_False);
        aConditionED.Enable(sal_False);
		aFileCB		.Enable(sal_False);
		aFilePB		.Enable(sal_False);
		aFileNameFT  .Enable(sal_False);
		aFileNameED  .Enable(sal_False);
		aSubRegionFT .Enable(sal_False);
		aSubRegionED .Enable(sal_False);
//        aNameFT      .Enable(sal_False);
		aCurName	 .Enable(sal_False);
		aDDECB   			.Enable(sal_False);
		aDDECommandFT       .Enable(sal_False);

		UseFileHdl(&aFileCB);
		DDEHdl( &aDDECB );
	}
	return 0;
}

/*---------------------------------------------------------------------
	Beschreibung:	Im OkHdl werden die veraenderten Einstellungen
					uebernommen und aufgehobene Bereiche geloescht
---------------------------------------------------------------------*/

IMPL_LINK( SwEditRegionDlg, OkHdl, CheckBox *, EMPTYARG )
{
	// JP 13.03.96:
	// temp. Array weil sich waehrend des aendern eines Bereiches die
	// Position innerhalb des "Core-Arrays" verschieben kann:
	//	- bei gelinkten Bereichen, wenn sie weitere SubBereiche haben oder
	//	  neu erhalten.
	// JP 30.05.97: StartUndo darf natuerlich auch erst nach dem Kopieren
	//				der Formate erfolgen (ClearRedo!)

	const SwSectionFmts& rDocFmts = rSh.GetDoc()->GetSections();
	SwSectionFmts aOrigArray( 0, 5 );
	aOrigArray.Insert( &rDocFmts, 0 );

	rSh.StartAllAction();
	rSh.StartUndo();
	rSh.ResetSelect( 0,sal_False );
	SvLBoxEntry* pEntry = aTree.First();

	while( pEntry )
	{
		SectReprPtr pRepr = (SectReprPtr) pEntry->GetUserData();
		SwSectionFmt* pFmt = aOrigArray[ pRepr->GetArrPos() ];
        if (!pRepr->GetSectionData().IsProtectFlag())
        {
            pRepr->GetSectionData().SetPassword(uno::Sequence<sal_Int8 >());
        }
        sal_uInt16 nNewPos = rDocFmts.GetPos( pFmt );
		if( USHRT_MAX != nNewPos )
		{
			SfxItemSet* pSet = pFmt->GetAttrSet().Clone( sal_False );
			if( pFmt->GetCol() != pRepr->GetCol() )
				pSet->Put( pRepr->GetCol() );

			if( pFmt->GetBackground(sal_False) != pRepr->GetBackground() )
				pSet->Put( pRepr->GetBackground() );

			if( pFmt->GetFtnAtTxtEnd(sal_False) != pRepr->GetFtnNtAtEnd() )
				pSet->Put( pRepr->GetFtnNtAtEnd() );

			if( pFmt->GetEndAtTxtEnd(sal_False) != pRepr->GetEndNtAtEnd() )
				pSet->Put( pRepr->GetEndNtAtEnd() );

			if( pFmt->GetBalancedColumns() != pRepr->GetBalance() )
				pSet->Put( pRepr->GetBalance() );

            if( pFmt->GetFrmDir() != pRepr->GetFrmDir() )
                pSet->Put( pRepr->GetFrmDir() );

            if( pFmt->GetLRSpace() != pRepr->GetLRSpace())
                pSet->Put( pRepr->GetLRSpace());

            rSh.UpdateSection( nNewPos, pRepr->GetSectionData(),
							pSet->Count() ? pSet : 0 );
			delete pSet;
		}
		pEntry = aTree.Next( pEntry );
	}

	for(sal_uInt16 i = aSectReprArr.Count(); i; )
	{
		SwSectionFmt* pFmt = aOrigArray[ aSectReprArr[ --i ]->GetArrPos() ];
		sal_uInt16 nNewPos = rDocFmts.GetPos( pFmt );
		if( USHRT_MAX != nNewPos )
			rSh.DelSectionFmt( nNewPos );
	}
//    rSh.ChgSectionPasswd(aNewPasswd);

	aOrigArray.Remove( 0, aOrigArray.Count() );

	//JP 21.05.97: EndDialog muss vor Ende der EndAction gerufen werden,
	//				sonst kann es ScrollFehler geben.
	EndDialog(RET_OK);

	rSh.EndUndo();
	rSh.EndAllAction();

	return 0;
}
/*---------------------------------------------------------------------
 Beschreibung: Toggle protect
---------------------------------------------------------------------*/

IMPL_LINK( SwEditRegionDlg, ChangeProtectHdl, TriStateBox *, pBox )
{
    if(!CheckPasswd(pBox))
        return 0;
    pBox->EnableTriState( sal_False );
	SvLBoxEntry* pEntry=aTree.FirstSelected();
	DBG_ASSERT(pEntry,"kein Entry gefunden");
    sal_Bool bCheck = STATE_CHECK == pBox->GetState();
    while( pEntry )
	{
		SectReprPtr pRepr = (SectReprPtr) pEntry->GetUserData();
        pRepr->GetSectionData().SetProtectFlag(bCheck);
        Image aImage = BuildBitmap( bCheck,
                                    STATE_CHECK == aHideCB.GetState(), sal_False);
        aTree.SetExpandedEntryBmp(pEntry, aImage, BMP_COLOR_NORMAL);
        aTree.SetCollapsedEntryBmp(pEntry, aImage, BMP_COLOR_NORMAL);
        Image aHCImg = BuildBitmap( bCheck, STATE_CHECK == aHideCB.GetState(), sal_True);
        aTree.SetExpandedEntryBmp(pEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);
        aTree.SetCollapsedEntryBmp(pEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);
        pEntry = aTree.NextSelected(pEntry);
	}
    aPasswdCB.Enable(bCheck);
    aPasswdPB.Enable(bCheck);
    return 0;
}
/*---------------------------------------------------------------------
 Beschreibung: Toggle hide
---------------------------------------------------------------------*/

IMPL_LINK( SwEditRegionDlg, ChangeHideHdl, TriStateBox *, pBox )
{
    if(!CheckPasswd(pBox))
        return 0;
    pBox->EnableTriState( sal_False );
	SvLBoxEntry* pEntry=aTree.FirstSelected();
	DBG_ASSERT(pEntry,"kein Entry gefunden");
	while( pEntry )
	{
		SectReprPtr pRepr = (SectReprPtr) pEntry->GetUserData();
        pRepr->GetSectionData().SetHidden(STATE_CHECK == pBox->GetState());
        Image aImage = BuildBitmap(STATE_CHECK == aProtectCB.GetState(),
                                    STATE_CHECK == pBox->GetState(), sal_False);
        aTree.SetExpandedEntryBmp(pEntry, aImage, BMP_COLOR_NORMAL);
        aTree.SetCollapsedEntryBmp(pEntry, aImage, BMP_COLOR_NORMAL);
        Image aHCImg = BuildBitmap( STATE_CHECK == aProtectCB.GetState(),
                                    STATE_CHECK == pBox->GetState(), sal_True);
        aTree.SetExpandedEntryBmp(pEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);
        aTree.SetCollapsedEntryBmp(pEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);

		pEntry = aTree.NextSelected(pEntry);
	}

	sal_Bool bHide = STATE_CHECK == pBox->GetState();
    aConditionED.Enable(bHide);
    aConditionFT.Enable(bHide);
    return 0;
}

/*---------------------------------------------------------------------
 Beschreibung: Toggle edit in readonly
---------------------------------------------------------------------*/

IMPL_LINK( SwEditRegionDlg, ChangeEditInReadonlyHdl, TriStateBox *, pBox )
{
    if(!CheckPasswd(pBox))
        return 0;
    pBox->EnableTriState( sal_False );
    SvLBoxEntry* pEntry=aTree.FirstSelected();
    DBG_ASSERT(pEntry,"kein Entry gefunden");
    while( pEntry )
    {
        SectReprPtr pRepr = (SectReprPtr) pEntry->GetUserData();
        pRepr->GetSectionData().SetEditInReadonlyFlag(
                STATE_CHECK == pBox->GetState());
        pEntry = aTree.NextSelected(pEntry);
    }

    return 0;
}

/*---------------------------------------------------------------------
 Beschreibung: selektierten Bereich aufheben
---------------------------------------------------------------------*/

IMPL_LINK( SwEditRegionDlg, ChangeDismissHdl, CheckBox *, EMPTYARG )
{
    if(!CheckPasswd())
        return 0;
    SvLBoxEntry* pEntry = aTree.FirstSelected();
	SvLBoxEntry* pChild;
	SvLBoxEntry* pParent;
	//zuerst alle selektierten markieren
	while(pEntry)
	{
		const SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
		pSectRepr->SetSelected();
		pEntry = aTree.NextSelected(pEntry);
	}
	pEntry = aTree.FirstSelected();
	// dann loeschen
	while(pEntry)
	{
		const SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
		SvLBoxEntry* pRemove = 0;
		sal_Bool bRestart = sal_False;
		if(pSectRepr->IsSelected())
		{
			aSectReprArr.Insert( pSectRepr );
			while( (pChild = aTree.FirstChild(pEntry) )!= 0 )
			{
				//durch das Umhaengen muss wieder am Anfang aufgesetzt werden
				bRestart = sal_True;
				pParent=aTree.GetParent(pEntry);
				aTree.GetModel()->Move(pChild, pParent, aTree.GetModel()->GetRelPos(pEntry));
			}
			pRemove = pEntry;
		}
		if(bRestart)
			pEntry = aTree.First();
		else
			pEntry = aTree.Next(pEntry);
		if(pRemove)
			aTree.GetModel()->Remove( pRemove );
	}

	if ( (pEntry=aTree.FirstSelected()) == 0 )
	{
        aConditionFT.        Enable(sal_False);
        aConditionED.   Enable(sal_False);
		aDismiss.		Enable(sal_False);
		aCurName.		Enable(sal_False);
		aProtectCB.		Enable(sal_False);
		aPasswdCB.		Enable(sal_False);
		aHideCB.		Enable(sal_False);
        // --> FME 2004-06-22 #114856# edit in readonly sections
        aEditInReadonlyCB.Enable(sal_False);
        aEditInReadonlyCB.SetState(STATE_NOCHECK);
        // <--
        aProtectCB.     SetState(STATE_NOCHECK);
		aPasswdCB.		Check(sal_False);
		aHideCB.		SetState(STATE_NOCHECK);
		aFileCB.		Check(sal_False);
		//sonst liegt der Focus auf dem HelpButton
		aOK.GrabFocus();
		UseFileHdl(&aFileCB);
	}
	return 0;
}
/*---------------------------------------------------------------------
 Beschreibung: CheckBox mit Datei verknuepfen?
---------------------------------------------------------------------*/

IMPL_LINK( SwEditRegionDlg, UseFileHdl, CheckBox *, pBox )
{
    if(!CheckPasswd(pBox))
        return 0;
    SvLBoxEntry* pEntry = aTree.FirstSelected();
	pBox->EnableTriState(sal_False);
	sal_Bool bMulti = 1 < aTree.GetSelectionCount();
	sal_Bool bFile = pBox->IsChecked();
	if(pEntry)
	{
		while(pEntry)
		{
			const SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
			sal_Bool bContent = pSectRepr->IsContent();
			if( pBox->IsChecked() && bContent && rSh.HasSelection() )
			{
				if( RET_NO == QueryBox( this, SW_RES(QB_CONNECT) ).Execute() )
					pBox->Check( sal_False );
			}
			if( bFile )
				pSectRepr->SetContent(sal_False);
			else
			{
				pSectRepr->SetFile(aEmptyStr);
				pSectRepr->SetSubRegion(aEmptyStr);
                pSectRepr->GetSectionData().SetLinkFilePassword(aEmptyStr);
			}

			pEntry = aTree.NextSelected(pEntry);
		}
		aFileNameFT.Enable(bFile && ! bMulti);
		aFileNameED.Enable(bFile && ! bMulti);
		aFilePB.Enable(bFile && ! bMulti);
		aSubRegionED.Enable(bFile && ! bMulti);
		aSubRegionFT.Enable(bFile && ! bMulti);
		aDDECommandFT.Enable(bFile && ! bMulti);
		aDDECB.Enable(bFile && ! bMulti);
		if( bFile )
		{
			aProtectCB.SetState(STATE_CHECK);
			aFileNameED.GrabFocus();

		}
		else
		{
			aDDECB.Check(sal_False);
			DDEHdl(&aDDECB);
//			aFileNameED.SetText(aEmptyStr);
			aSubRegionED.SetText(aEmptyStr);
		}
	}
	else
	{
		pBox->Check(sal_False);
		pBox->Enable(sal_False);
		aFilePB.Enable(sal_False);
		aFileNameED.Enable(sal_False);
		aFileNameFT.Enable(sal_False);
		aSubRegionED.Enable(sal_False);
		aSubRegionFT.Enable(sal_False);
		aDDECB.Check(sal_False);
		aDDECB.Enable(sal_False);
		aDDECommandFT.Enable(sal_False);
	}
	return 0;
}

/*---------------------------------------------------------------------
	Beschreibung: Dialog Datei einfuegen rufen
---------------------------------------------------------------------*/

IMPL_LINK( SwEditRegionDlg, FileSearchHdl, PushButton *, EMPTYARG )
{
    if(!CheckPasswd(0))
        return 0;

    m_pOldDefDlgParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );
    if ( m_pDocInserter )
        delete m_pDocInserter;
    m_pDocInserter = new ::sfx2::DocumentInserter( 0, String::CreateFromAscii("swriter") );
    m_pDocInserter->StartExecuteModal( LINK( this, SwEditRegionDlg, DlgClosedHdl ) );
    return 0;
}

/*---------------------------------------------------------------------
	Beschreibung:
---------------------------------------------------------------------*/

IMPL_LINK( SwEditRegionDlg, OptionsHdl, PushButton *, EMPTYARG )
{
    if(!CheckPasswd())
        return 0;
    SvLBoxEntry* pEntry = aTree.FirstSelected();

	if(pEntry)
	{
		SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
		SfxItemSet aSet(rSh.GetView().GetPool(),
							RES_COL, RES_COL,
                            RES_COLUMNBALANCE, RES_FRAMEDIR,
							RES_BACKGROUND, RES_BACKGROUND,
							RES_FRM_SIZE, RES_FRM_SIZE,
							SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE,
							RES_LR_SPACE, RES_LR_SPACE,
							RES_FTN_AT_TXTEND, RES_END_AT_TXTEND,
                            0);

		aSet.Put( pSectRepr->GetCol() );
		aSet.Put( pSectRepr->GetBackground() );
		aSet.Put( pSectRepr->GetFtnNtAtEnd() );
		aSet.Put( pSectRepr->GetEndNtAtEnd() );
		aSet.Put( pSectRepr->GetBalance() );
        aSet.Put( pSectRepr->GetFrmDir() );
        aSet.Put( pSectRepr->GetLRSpace() );

		const SwSectionFmts& rDocFmts = rSh.GetDoc()->GetSections();
		SwSectionFmts aOrigArray( 0, 5 );
		aOrigArray.Insert( &rDocFmts, 0 );

		SwSectionFmt* pFmt = aOrigArray[pSectRepr->GetArrPos()];
		long nWidth = rSh.GetSectionWidth(*pFmt);
		aOrigArray.Remove( 0, aOrigArray.Count() );
		if (!nWidth)
			nWidth = USHRT_MAX;

		aSet.Put(SwFmtFrmSize(ATT_VAR_SIZE, nWidth));
		aSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(nWidth, nWidth)));

		SwSectionPropertyTabDialog aTabDlg(this, aSet, rSh);
		if(RET_OK == aTabDlg.Execute())
		{
			const SfxItemSet* pOutSet = aTabDlg.GetOutputItemSet();
			if( pOutSet && pOutSet->Count() )
			{
				const SfxPoolItem *pColItem, *pBrushItem,
                                  *pFtnItem, *pEndItem, *pBalanceItem,
                                  *pFrmDirItem, *pLRSpaceItem;
				SfxItemState eColState = pOutSet->GetItemState(
										RES_COL, sal_False, &pColItem );
				SfxItemState eBrushState = pOutSet->GetItemState(
										RES_BACKGROUND, sal_False, &pBrushItem );
				SfxItemState eFtnState = pOutSet->GetItemState(
										RES_FTN_AT_TXTEND, sal_False, &pFtnItem );
				SfxItemState eEndState = pOutSet->GetItemState(
										RES_END_AT_TXTEND, sal_False, &pEndItem );
				SfxItemState eBalanceState = pOutSet->GetItemState(
										RES_COLUMNBALANCE, sal_False, &pBalanceItem );
                SfxItemState eFrmDirState = pOutSet->GetItemState(
                                        RES_FRAMEDIR, sal_False, &pFrmDirItem );
                SfxItemState eLRState = pOutSet->GetItemState(
                                        RES_LR_SPACE, sal_False, &pLRSpaceItem);

				if( SFX_ITEM_SET == eColState ||
					SFX_ITEM_SET == eBrushState ||
					SFX_ITEM_SET == eFtnState ||
					SFX_ITEM_SET == eEndState ||
                    SFX_ITEM_SET == eBalanceState||
                    SFX_ITEM_SET == eFrmDirState||
                    SFX_ITEM_SET == eLRState)
				{
                    SvLBoxEntry* pSelEntry = aTree.FirstSelected();
                    while( pSelEntry )
					{
                        SectReprPtr pRepr = (SectReprPtr)pSelEntry->GetUserData();
						if( SFX_ITEM_SET == eColState )
							pRepr->GetCol() = *(SwFmtCol*)pColItem;
						if( SFX_ITEM_SET == eBrushState )
							pRepr->GetBackground() = *(SvxBrushItem*)pBrushItem;
						if( SFX_ITEM_SET == eFtnState )
							pRepr->GetFtnNtAtEnd() = *(SwFmtFtnAtTxtEnd*)pFtnItem;
						if( SFX_ITEM_SET == eEndState )
							pRepr->GetEndNtAtEnd() = *(SwFmtEndAtTxtEnd*)pEndItem;
						if( SFX_ITEM_SET == eBalanceState )
							pRepr->GetBalance().SetValue(((SwFmtNoBalancedColumns*)pBalanceItem)->GetValue());
                        if( SFX_ITEM_SET == eFrmDirState )
                            pRepr->GetFrmDir().SetValue(((SvxFrameDirectionItem*)pFrmDirItem)->GetValue());
                        if( SFX_ITEM_SET == eLRState )
                            pRepr->GetLRSpace() = *(SvxLRSpaceItem*)pLRSpaceItem;

                        pSelEntry = aTree.NextSelected(pSelEntry);
					}
				}
			}
		}
	}

	return 0;
}

/*---------------------------------------------------------------------
	Beschreibung:  	Uebernahme des Dateinamen oder
					des verknuepften Bereichs
---------------------------------------------------------------------*/

IMPL_LINK( SwEditRegionDlg, FileNameHdl, Edit *, pEdit )
{
    Selection aSelect = pEdit->GetSelection();
    if(!CheckPasswd())
        return 0;
    pEdit->SetSelection(aSelect);
    SvLBoxEntry* pEntry=aTree.FirstSelected();
	DBG_ASSERT(pEntry,"kein Entry gefunden");
	SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
	if(pEdit == &aFileNameED)
	{
        bSubRegionsFilled = false;
        aSubRegionED.Clear();
		if( aDDECB.IsChecked() )
		{
			String sLink( pEdit->GetText() );
			sal_uInt16 nPos = 0;
			while( STRING_NOTFOUND != (nPos = sLink.SearchAscii( "  ", nPos )) )
				sLink.Erase( nPos--, 1 );

            nPos = sLink.SearchAndReplace( ' ', sfx2::cTokenSeperator );
            sLink.SearchAndReplace( ' ', sfx2::cTokenSeperator, nPos );

            pSectRepr->GetSectionData().SetLinkFileName( sLink );
            pSectRepr->GetSectionData().SetType( DDE_LINK_SECTION );
        }
		else
		{
			String sTmp(pEdit->GetText());
			if(sTmp.Len())
            {
                SfxMedium* pMedium = rSh.GetView().GetDocShell()->GetMedium();
                INetURLObject aAbs;
                if( pMedium )
                    aAbs = pMedium->GetURLObject();
                sTmp = URIHelper::SmartRel2Abs(
                    aAbs, sTmp, URIHelper::GetMaybeFileHdl() );
            }
			pSectRepr->SetFile( sTmp );
            pSectRepr->GetSectionData().SetLinkFilePassword( aEmptyStr );
		}
	}
	else
	{
		pSectRepr->SetSubRegion( pEdit->GetText() );
	}
	return 0;
}
/*---------------------------------------------------------------------
	Beschreibung:
---------------------------------------------------------------------*/

IMPL_LINK( SwEditRegionDlg, DDEHdl, CheckBox*, pBox )
{
    if(!CheckPasswd(pBox))
        return 0;
    SvLBoxEntry* pEntry=aTree.FirstSelected();
	if(pEntry)
	{
		sal_Bool bFile = aFileCB.IsChecked();
		SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
        SwSectionData & rData( pSectRepr->GetSectionData() );
		sal_Bool bDDE = pBox->IsChecked();
		if(bDDE)
		{
			aFileNameFT.Hide();
			aDDECommandFT.Enable();
			aDDECommandFT.Show();
			aSubRegionFT.Hide();
			aSubRegionED.Hide();
            if (FILE_LINK_SECTION == rData.GetType())
			{
				pSectRepr->SetFile(aEmptyStr);
				aFileNameED.SetText(aEmptyStr);
                rData.SetLinkFilePassword( aEmptyStr );
            }
            rData.SetType(DDE_LINK_SECTION);
			aFileNameED.SetAccessibleName(aDDECommandFT.GetText());
		}
		else
		{
			aDDECommandFT.Hide();
			aFileNameFT.Enable(bFile);
			aFileNameFT.Show();
			aSubRegionED.Show();
			aSubRegionFT.Show();
			aSubRegionED.Enable(bFile);
			aSubRegionFT.Enable(bFile);
			aSubRegionED.Enable(bFile);
            if (DDE_LINK_SECTION == rData.GetType())
			{
                rData.SetType(FILE_LINK_SECTION);
				pSectRepr->SetFile(aEmptyStr);
                rData.SetLinkFilePassword( aEmptyStr );
				aFileNameED.SetText(aEmptyStr);
			}
			aFileNameED.SetAccessibleName(aFileNameFT.GetText());
		}
		aFilePB.Enable(bFile && !bDDE);
	}
	return 0;
}
/*---------------------------------------------------------------------

---------------------------------------------------------------------*/

IMPL_LINK( SwEditRegionDlg, ChangePasswdHdl, Button *, pBox )
{
    sal_Bool bChange = pBox == &aPasswdPB;
    if(!CheckPasswd(0))
    {
        if(!bChange)
            aPasswdCB.Check(!aPasswdCB.IsChecked());
        return 0;
    }
    SvLBoxEntry* pEntry=aTree.FirstSelected();
    sal_Bool bSet = bChange ? bChange : aPasswdCB.IsChecked();
    DBG_ASSERT(pEntry,"kein Entry gefunden");
    while( pEntry )
	{
		SectReprPtr pRepr = (SectReprPtr)pEntry->GetUserData();
        if(bSet)
        {
            if(!pRepr->GetTempPasswd().getLength() || bChange)
            {
                SwTestPasswdDlg aPasswdDlg(this);
                aPasswdDlg.ShowExtras(SHOWEXTRAS_CONFIRM);
                if(RET_OK == aPasswdDlg.Execute())
                {
                    String sNewPasswd( aPasswdDlg.GetPassword() );
                    if( aPasswdDlg.GetConfirm() == sNewPasswd )
                    {
                        SvPasswordHelper::GetHashPassword( pRepr->GetTempPasswd(), sNewPasswd );
                    }
                    else
                    {
                        InfoBox(pBox, SW_RES(REG_WRONG_PASSWD_REPEAT)).Execute();
                        ChangePasswdHdl(pBox);
                        break;
                    }
                }
                else
                {
                    if(!bChange)
                        aPasswdCB.Check(sal_False);
                    break;
                }
            }
            pRepr->GetSectionData().SetPassword(pRepr->GetTempPasswd());
        }
        else
        {
            pRepr->GetSectionData().SetPassword(uno::Sequence<sal_Int8 >());
        }
		pEntry = aTree.NextSelected(pEntry);
	}
    return 0;
}
/*---------------------------------------------------------------------
	Beschreibung:	Aktueller Bereichsname wird sofort beim editieren
					in die TreeListBox eingetragen, mit leerem String
					kein Ok()
---------------------------------------------------------------------*/

IMPL_LINK( SwEditRegionDlg, NameEditHdl, Edit *, EMPTYARG )
{
    if(!CheckPasswd(0))
        return 0;
    SvLBoxEntry* pEntry=aTree.FirstSelected();
	DBG_ASSERT(pEntry,"kein Entry gefunden");
	if (pEntry)
	{
		String	aName = aCurName.GetText();
		aTree.SetEntryText(pEntry,aName);
        SectReprPtr pRepr = (SectReprPtr) pEntry->GetUserData();
        pRepr->GetSectionData().SetSectionName(aName);

		aOK.Enable(aName.Len() != 0);
	}
	return 0;
}
/*---------------------------------------------------------------------

---------------------------------------------------------------------*/

IMPL_LINK( SwEditRegionDlg, ConditionEditHdl, Edit *, pEdit )
{
    Selection aSelect = pEdit->GetSelection();
    if(!CheckPasswd(0))
        return 0;
    pEdit->SetSelection(aSelect);
    SvLBoxEntry* pEntry = aTree.FirstSelected();
	DBG_ASSERT(pEntry,"kein Entry gefunden");
	while( pEntry )
	{
		SectReprPtr pRepr = (SectReprPtr)pEntry->GetUserData();
        pRepr->GetSectionData().SetCondition(pEdit->GetText());
		pEntry = aTree.NextSelected(pEntry);
	}
	return 0;
}

IMPL_LINK( SwEditRegionDlg, DlgClosedHdl, sfx2::FileDialogHelper *, _pFileDlg )
{
    String sFileName, sFilterName, sPassword;
    if ( _pFileDlg->GetError() == ERRCODE_NONE )
    {
        SfxMedium* pMedium = m_pDocInserter->CreateMedium();
        if ( pMedium )
        {
            sFileName = pMedium->GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
            sFilterName = pMedium->GetFilter()->GetFilterName();
            const SfxPoolItem* pItem;
            if ( SFX_ITEM_SET == pMedium->GetItemSet()->GetItemState( SID_PASSWORD, sal_False, &pItem ) )
                sPassword = ( (SfxStringItem*)pItem )->GetValue();
            ::lcl_ReadSections( *pMedium, aSubRegionED );
            delete pMedium;
        }
    }

    SvLBoxEntry* pEntry = aTree.FirstSelected();
    DBG_ASSERT( pEntry, "no entry found" );
    if ( pEntry )
    {
        SectReprPtr pSectRepr = (SectRepr*)pEntry->GetUserData();
        pSectRepr->SetFile( sFileName );
        pSectRepr->SetFilter( sFilterName );
        pSectRepr->GetSectionData().SetLinkFilePassword(sPassword);
        aFileNameED.SetText( pSectRepr->GetFile() );
    }

    Application::SetDefDialogParent( m_pOldDefDlgParent );
    return 0;
}
/*-- 03.09.2009 16:24:18---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK( SwEditRegionDlg, SubRegionEventHdl, VclWindowEvent *, pEvent )
{
    if( !bSubRegionsFilled && pEvent && pEvent->GetId() == VCLEVENT_DROPDOWN_PRE_OPEN )
    {
        //if necessary fill the names bookmarks/sections/tables now 
        
        rtl::OUString sFileName = aFileNameED.GetText();
        if(sFileName.getLength())
        {
            SfxMedium* pMedium = rSh.GetView().GetDocShell()->GetMedium();
            INetURLObject aAbs;
            if( pMedium )
                aAbs = pMedium->GetURLObject();
            sFileName = URIHelper::SmartRel2Abs(
                    aAbs, sFileName, URIHelper::GetMaybeFileHdl() );

            //load file and set the shell
            SfxMedium aMedium( sFileName, STREAM_STD_READ );
            sFileName = aMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
            ::lcl_ReadSections( aMedium, aSubRegionED );
        }    
        else
            lcl_FillSubRegionList( rSh, aSubRegionED, 0 );
        bSubRegionsFilled = true;
    }    
    return 0;
}

/* -----------------------------08.05.2002 15:00------------------------------

 ---------------------------------------------------------------------------*/
Image SwEditRegionDlg::BuildBitmap(sal_Bool bProtect,sal_Bool bHidden, sal_Bool bHighContrast)
{
    ImageList& rImgLst = bHighContrast ? aImageILH : aImageIL;
    return rImgLst.GetImage((!bHidden+(bProtect<<1)) + 1);
}

/*--------------------------------------------------------------------
	Beschreibung:	Hilfsfunktion - Bereichsnamen aus dem Medium lesen
 --------------------------------------------------------------------*/

static void lcl_ReadSections( SfxMedium& rMedium, ComboBox& rBox )
{
	rBox.Clear();
    uno::Reference < embed::XStorage > xStg;
    if( rMedium.IsStorage() && (xStg = rMedium.GetStorage()).is() )
	{
        SvStrings aArr( 10, 10 );
        sal_uInt32 nFormat = SotStorage::GetFormatID( xStg );
        if ( nFormat == SOT_FORMATSTR_ID_STARWRITER_60 || nFormat == SOT_FORMATSTR_ID_STARWRITERGLOB_60 ||
            nFormat == SOT_FORMATSTR_ID_STARWRITER_8 || nFormat == SOT_FORMATSTR_ID_STARWRITERGLOB_8)
            SwGetReaderXML()->GetSectionList( rMedium, aArr );

		for( sal_uInt16 n = 0; n < aArr.Count(); ++n )
			rBox.InsertEntry( *aArr[ n ] );

        aArr.DeleteAndDestroy(0, aArr.Count());
	}
}
/* -----------------21.05.99 10:16-------------------
 *
 * --------------------------------------------------*/
SwInsertSectionTabDialog::SwInsertSectionTabDialog(
			Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh) :
	SfxTabDialog( pParent, SW_RES(DLG_INSERT_SECTION), &rSet ),
    rWrtSh(rSh)
    , m_pSectionData(0)
{
	String sInsert(SW_RES(ST_INSERT));
	GetOKButton().SetText(sInsert);
	FreeResource();
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");
	AddTabPage(TP_INSERT_SECTION, SwInsertSectionTabPage::Create, 0);
	AddTabPage(TP_COLUMN,	SwColumnPage::Create,  	 0);
    AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0);
	AddTabPage(TP_SECTION_FTNENDNOTES, SwSectionFtnEndTabPage::Create, 0);
    AddTabPage(TP_SECTION_INDENTS, SwSectionIndentTabPage::Create, 0);

	SvxHtmlOptions* pHtmlOpt = SvxHtmlOptions::Get();
	long nHtmlMode = pHtmlOpt->GetExportMode();

	sal_Bool bWeb = 0 != PTR_CAST( SwWebDocShell, rSh.GetView().GetDocShell() );
	if(bWeb)
	{
		RemoveTabPage(TP_SECTION_FTNENDNOTES);
        RemoveTabPage(TP_SECTION_INDENTS);
        if( HTML_CFG_NS40 != nHtmlMode && HTML_CFG_WRITER != nHtmlMode)
			RemoveTabPage(TP_COLUMN);
	}
	SetCurPageId(TP_INSERT_SECTION);
}
/* -----------------21.05.99 10:17-------------------
 *
 * --------------------------------------------------*/
SwInsertSectionTabDialog::~SwInsertSectionTabDialog()
{
}
/* -----------------21.05.99 10:23-------------------
 *
 * --------------------------------------------------*/
void SwInsertSectionTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
	if(TP_INSERT_SECTION == nId)
		((SwInsertSectionTabPage&)rPage).SetWrtShell(rWrtSh);
	else if( TP_BACKGROUND == nId  )
    {
			SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
			aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_SHOW_SELECTOR));
			rPage.PageCreated(aSet);
	}
	else if( TP_COLUMN == nId )
	{
		const SwFmtFrmSize& rSize = (const SwFmtFrmSize&)GetInputSetImpl()->Get(RES_FRM_SIZE);
		((SwColumnPage&)rPage).SetPageWidth(rSize.GetWidth());
		((SwColumnPage&)rPage).ShowBalance(sal_True);
        ((SwColumnPage&)rPage).SetInSection(sal_True);
	}
    else if(TP_SECTION_INDENTS == nId)
        ((SwSectionIndentTabPage&)rPage).SetWrtShell(rWrtSh);
}
/* -----------------21.05.99 13:08-------------------
 *
 * --------------------------------------------------*/

void SwInsertSectionTabDialog::SetSectionData(SwSectionData const& rSect)
{
    m_pSectionData.reset( new SwSectionData(rSect) );
}
/* -----------------21.05.99 13:10-------------------
 *
 * --------------------------------------------------*/
short	SwInsertSectionTabDialog::Ok()
{
	short nRet = SfxTabDialog::Ok();
    DBG_ASSERT(m_pSectionData.get(),
            "SwInsertSectionTabDialog: no SectionData?");
    const SfxItemSet* pOutputItemSet = GetOutputItemSet();
    rWrtSh.InsertSection(*m_pSectionData, pOutputItemSet);
    SfxViewFrame* pViewFrm = rWrtSh.GetView().GetViewFrame();
    uno::Reference< frame::XDispatchRecorder > xRecorder =
            pViewFrm->GetBindings().GetRecorder();
    if ( xRecorder.is() )
    {
        SfxRequest aRequest( pViewFrm, FN_INSERT_REGION);
        const SfxPoolItem* pCol;
        if(SFX_ITEM_SET == pOutputItemSet->GetItemState(RES_COL, sal_False, &pCol))
        {
            aRequest.AppendItem(SfxUInt16Item(SID_ATTR_COLUMNS,
                ((const SwFmtCol*)pCol)->GetColumns().Count()));
        }
        aRequest.AppendItem(SfxStringItem( FN_PARAM_REGION_NAME,
                    m_pSectionData->GetSectionName()));
        aRequest.AppendItem(SfxStringItem( FN_PARAM_REGION_CONDITION,
                    m_pSectionData->GetCondition()));
        aRequest.AppendItem(SfxBoolItem( FN_PARAM_REGION_HIDDEN,
                    m_pSectionData->IsHidden()));
        aRequest.AppendItem(SfxBoolItem( FN_PARAM_REGION_PROTECT,
                    m_pSectionData->IsProtectFlag()));
        // --> FME 2004-06-22 #114856# edit in readonly sections
        aRequest.AppendItem(SfxBoolItem( FN_PARAM_REGION_EDIT_IN_READONLY,
                    m_pSectionData->IsEditInReadonlyFlag()));
        // <--

        String sLinkFileName( m_pSectionData->GetLinkFileName() );
        aRequest.AppendItem(SfxStringItem( FN_PARAM_1, sLinkFileName.GetToken( 0, sfx2::cTokenSeperator )));
        aRequest.AppendItem(SfxStringItem( FN_PARAM_2, sLinkFileName.GetToken( 1, sfx2::cTokenSeperator )));
        aRequest.AppendItem(SfxStringItem( FN_PARAM_3, sLinkFileName.GetToken( 2, sfx2::cTokenSeperator )));
        aRequest.Done();
    }
    return nRet;
}

/* -----------------21.05.99 10:31-------------------
 *
 * --------------------------------------------------*/
SwInsertSectionTabPage::SwInsertSectionTabPage(
							Window *pParent, const SfxItemSet &rAttrSet) :
	SfxTabPage( pParent, SW_RES(TP_INSERT_SECTION), rAttrSet ),
    aNameFL       ( this, SW_RES( FL_NAME ) ),
    aCurName            ( this, SW_RES( ED_RNAME ) ),
    aLinkFL             ( this, SW_RES( FL_LINK ) ),
    aFileCB             ( this, SW_RES( CB_FILE ) ),
	aDDECB              ( this, SW_RES( CB_DDE ) ) ,
	aDDECommandFT       ( this, SW_RES( FT_DDE ) ) ,
    aFileNameFT         ( this, SW_RES( FT_FILE ) ) ,
	aFileNameED         ( this, SW_RES( ED_FILE ) ),
    aFilePB             ( this, SW_RES( PB_FILE ) ),
    aSubRegionFT        ( this, SW_RES( FT_SUBREG ) ) ,
	aSubRegionED        ( this, SW_RES( LB_SUBREG ) ) ,

    aProtectFL          ( this, SW_RES( FL_PROTECT ) ),
    aProtectCB          ( this, SW_RES( CB_PROTECT ) ),
    aPasswdCB           ( this, SW_RES( CB_PASSWD ) ),
    aPasswdPB           ( this, SW_RES( PB_PASSWD ) ),

    aHideFL             ( this, SW_RES( FL_HIDE ) ),
    aHideCB             ( this, SW_RES( CB_HIDE ) ),
    aConditionFT             ( this, SW_RES( FT_CONDITION ) ),
    aConditionED        ( this, SW_RES( ED_CONDITION ) ),
    // --> FME 2004-06-22 #114856# edit in readonly sections
    aPropertiesFL       ( this, SW_RES( FL_PROPERTIES ) ),
    aEditInReadonlyCB   ( this, SW_RES( CB_EDIT_IN_READONLY ) ),
    // <--

    m_pWrtSh(0),
    m_pDocInserter(NULL),
    m_pOldDefDlgParent(NULL)
{
	FreeResource();

	aProtectCB.SetClickHdl	( LINK( this, SwInsertSectionTabPage, ChangeProtectHdl));
    aPasswdCB.SetClickHdl   ( LINK( this, SwInsertSectionTabPage, ChangePasswdHdl));
    aPasswdPB.SetClickHdl   ( LINK( this, SwInsertSectionTabPage, ChangePasswdHdl));
    aHideCB.SetClickHdl     ( LINK( this, SwInsertSectionTabPage, ChangeHideHdl));
    // --> FME 2004-06-22 #114856# edit in readonly sections
    aEditInReadonlyCB.SetClickHdl       ( LINK( this, SwInsertSectionTabPage, ChangeEditInReadonlyHdl));
    // <--
    aFileCB.SetClickHdl     ( LINK( this, SwInsertSectionTabPage, UseFileHdl ));
	aFilePB.SetClickHdl     ( LINK( this, SwInsertSectionTabPage, FileSearchHdl ));
	aCurName.SetModifyHdl	( LINK( this, SwInsertSectionTabPage, NameEditHdl));
	aDDECB.SetClickHdl		( LINK( this, SwInsertSectionTabPage, DDEHdl ));
    ChangeProtectHdl(&aProtectCB);
	aPasswdPB.SetAccessibleRelationMemberOf(&aProtectFL);
    aSubRegionED.EnableAutocomplete( sal_True, sal_True );
}
/* -----------------21.05.99 10:31-------------------
 *
 * --------------------------------------------------*/
SwInsertSectionTabPage::~SwInsertSectionTabPage()
{
    delete m_pDocInserter;
}

void	SwInsertSectionTabPage::SetWrtShell(SwWrtShell& rSh)
{
    m_pWrtSh = &rSh;

    sal_Bool bWeb = 0 != PTR_CAST(SwWebDocShell, m_pWrtSh->GetView().GetDocShell());
	if(bWeb)
	{
		aHideCB    		.Hide();
		aConditionED    .Hide();
        aConditionFT    .Hide();
		aDDECB           .Hide();
		aDDECommandFT    .Hide();
	}

    lcl_FillSubRegionList( *m_pWrtSh, aSubRegionED, &aCurName );

    SwSectionData *const pSectionData =
        static_cast<SwInsertSectionTabDialog*>(GetTabDialog())
            ->GetSectionData();
    if (pSectionData) // something set?
    {
        aCurName.SetText(
            rSh.GetUniqueSectionName(& pSectionData->GetSectionName()));
        aProtectCB.Check( 0 != pSectionData->IsProtectFlag() );
        m_sFileName = pSectionData->GetLinkFileName();
        m_sFilePasswd = pSectionData->GetLinkFilePassword();
        aFileCB.Check( 0 != m_sFileName.Len() );
        aFileNameED.SetText( m_sFileName );
		UseFileHdl( &aFileCB );
	}
	else
	{
		aCurName.SetText( rSh.GetUniqueSectionName() );
	}
}
/* -----------------21.05.99 10:32-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwInsertSectionTabPage::FillItemSet( SfxItemSet& )
{
    SwSectionData aSection(CONTENT_SECTION, aCurName.GetText());
    aSection.SetCondition(aConditionED.GetText());
    sal_Bool bProtected = aProtectCB.IsChecked();
    aSection.SetProtectFlag(bProtected);
	aSection.SetHidden(aHideCB.IsChecked());
    // --> FME 2004-06-22 #114856# edit in readonly sections
    aSection.SetEditInReadonlyFlag(aEditInReadonlyCB.IsChecked());
    // <--
    if(bProtected)
    {
        aSection.SetPassword(m_aNewPasswd);
    }
	String sFileName = aFileNameED.GetText();
	String sSubRegion = aSubRegionED.GetText();
	sal_Bool bDDe = aDDECB.IsChecked();
	if(aFileCB.IsChecked() && (sFileName.Len() || sSubRegion.Len() || bDDe))
	{
		String aLinkFile;
		if( bDDe )
		{
			aLinkFile = sFileName;

			sal_uInt16 nPos = 0;
			while( STRING_NOTFOUND != (nPos = aLinkFile.SearchAscii( "  ", nPos )) )
				aLinkFile.Erase( nPos--, 1 );

            nPos = aLinkFile.SearchAndReplace( ' ', sfx2::cTokenSeperator );
            aLinkFile.SearchAndReplace( ' ', sfx2::cTokenSeperator, nPos );
		}
		else
		{
			if(sFileName.Len())
			{
                SfxMedium* pMedium = m_pWrtSh->GetView().GetDocShell()->GetMedium();
                INetURLObject aAbs;
                if( pMedium )
                    aAbs = pMedium->GetURLObject();
                aLinkFile = URIHelper::SmartRel2Abs(
                    aAbs, sFileName, URIHelper::GetMaybeFileHdl() );
                aSection.SetLinkFilePassword( m_sFilePasswd );
			}

            aLinkFile += sfx2::cTokenSeperator;
            aLinkFile += m_sFilterName;
            aLinkFile += sfx2::cTokenSeperator;
			aLinkFile += sSubRegion;
        }

		aSection.SetLinkFileName(aLinkFile);
		if(aLinkFile.Len())
		{
			aSection.SetType( aDDECB.IsChecked() ?
									DDE_LINK_SECTION :
										FILE_LINK_SECTION);
		}
	}
    ((SwInsertSectionTabDialog*)GetTabDialog())->SetSectionData(aSection);
	return sal_True;
}
/* -----------------21.05.99 10:32-------------------
 *
 * --------------------------------------------------*/
void SwInsertSectionTabPage::Reset( const SfxItemSet& )
{
}
/* -----------------21.05.99 11:22-------------------
 *
 * --------------------------------------------------*/
SfxTabPage*	SwInsertSectionTabPage::Create( Window* pParent,
								const SfxItemSet& rAttrSet)
{
	return new SwInsertSectionTabPage(pParent, rAttrSet);
}
/*---------------------------------------------------------------------

---------------------------------------------------------------------*/

IMPL_LINK( SwInsertSectionTabPage, ChangeHideHdl, CheckBox *, pBox )
{
	sal_Bool bHide = pBox->IsChecked();
    aConditionED.Enable(bHide);
    aConditionFT.Enable(bHide);
	return 0;
}
/*---------------------------------------------------------------------

---------------------------------------------------------------------*/

IMPL_LINK( SwInsertSectionTabPage, ChangeEditInReadonlyHdl, CheckBox *, EMPTYARG )
{
    return 0;
}
/*---------------------------------------------------------------------

---------------------------------------------------------------------*/

IMPL_LINK( SwInsertSectionTabPage, ChangeProtectHdl, CheckBox *, pBox )
{
    sal_Bool bCheck = pBox->IsChecked();
    aPasswdCB.Enable(bCheck);
    aPasswdPB.Enable(bCheck);
    return 0;
}
/* -----------------------------26.04.01 14:50--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK( SwInsertSectionTabPage, ChangePasswdHdl, Button *, pButton )
{
    sal_Bool bChange = pButton == &aPasswdPB;
    sal_Bool bSet = bChange ? bChange : aPasswdCB.IsChecked();
    if(bSet)
    {
        if(!m_aNewPasswd.getLength() || bChange)
        {
            SwTestPasswdDlg aPasswdDlg(this);
            aPasswdDlg.ShowExtras(SHOWEXTRAS_CONFIRM);
            if(RET_OK == aPasswdDlg.Execute())
            {
                String sNewPasswd( aPasswdDlg.GetPassword() );
                if( aPasswdDlg.GetConfirm() == sNewPasswd )
                {
                    SvPasswordHelper::GetHashPassword( m_aNewPasswd, sNewPasswd );
                }
                else
                {
                    InfoBox(pButton, SW_RES(REG_WRONG_PASSWD_REPEAT)).Execute();
                }
            }
            else if(!bChange)
                aPasswdCB.Check(sal_False);
        }
    }
    else
        m_aNewPasswd.realloc(0);
    return 0;
}
/*---------------------------------------------------------------------

---------------------------------------------------------------------*/

IMPL_LINK_INLINE_START( SwInsertSectionTabPage, NameEditHdl, Edit *, EMPTYARG )
{
	String	aName=aCurName.GetText();
	GetTabDialog()->GetOKButton().Enable(aName.Len() && aCurName.GetEntryPos( aName ) == USHRT_MAX);
	return 0;
}
IMPL_LINK_INLINE_END( SwInsertSectionTabPage, NameEditHdl, Edit *, EMPTYARG )

/*---------------------------------------------------------------------

---------------------------------------------------------------------*/

IMPL_LINK( SwInsertSectionTabPage, UseFileHdl, CheckBox *, pBox )
{
	if( pBox->IsChecked() )
	{
        if( m_pWrtSh->HasSelection() &&
			RET_NO == QueryBox( this, SW_RES(QB_CONNECT) ).Execute() )
			pBox->Check( sal_False );
	}

	sal_Bool bFile = pBox->IsChecked();
	aFileNameFT.Enable(bFile);
	aFileNameED.Enable(bFile);
	aFilePB.Enable(bFile);
	aSubRegionFT.Enable(bFile);
	aSubRegionED.Enable(bFile);
	aDDECommandFT.Enable(bFile);
	aDDECB.Enable(bFile);
	if( bFile )
	{
//		aFileNameED.SetText( aFileName );
		aFileNameED.GrabFocus();
		aProtectCB.Check( sal_True );
	}
	else
	{
		aDDECB.Check(sal_False);
		DDEHdl(&aDDECB);
//		aFileNameED.SetText(aEmptyStr);
	}
	return 0;
}

/*---------------------------------------------------------------------

---------------------------------------------------------------------*/

IMPL_LINK( SwInsertSectionTabPage, FileSearchHdl, PushButton *, EMPTYARG )
{
    m_pOldDefDlgParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );
    if ( m_pDocInserter )
        delete m_pDocInserter;
    m_pDocInserter = new ::sfx2::DocumentInserter( 0, String::CreateFromAscii("swriter") );
    m_pDocInserter->StartExecuteModal( LINK( this, SwInsertSectionTabPage, DlgClosedHdl ) );
    return 0;
}

/*---------------------------------------------------------------------

---------------------------------------------------------------------*/

IMPL_LINK( SwInsertSectionTabPage, DDEHdl, CheckBox*, pBox )
{
	sal_Bool bDDE = pBox->IsChecked();
	sal_Bool bFile = aFileCB.IsChecked();
	aFilePB.Enable(!bDDE && bFile);
	if(bDDE)
	{
		aFileNameFT.Hide();
		aDDECommandFT.Enable(bDDE);
		aDDECommandFT.Show();
		aSubRegionFT.Hide();
		aSubRegionED.Hide();
		aFileNameED.SetAccessibleName(aDDECommandFT.GetText());
	}
	else
	{
		aDDECommandFT.Hide();
		aFileNameFT.Enable(bFile);
		aFileNameFT.Show();
		aSubRegionFT.Show();
		aSubRegionED.Show();
		aSubRegionED.Enable(bFile);
		aFileNameED.SetAccessibleName(aFileNameFT.GetText());
	}
	return 0;
}

IMPL_LINK( SwInsertSectionTabPage, DlgClosedHdl, sfx2::FileDialogHelper *, _pFileDlg )
{
    if ( _pFileDlg->GetError() == ERRCODE_NONE )
    {
        SfxMedium* pMedium = m_pDocInserter->CreateMedium();
        if ( pMedium )
        {
            m_sFileName = pMedium->GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
            m_sFilterName = pMedium->GetFilter()->GetFilterName();
            const SfxPoolItem* pItem;
            if ( SFX_ITEM_SET == pMedium->GetItemSet()->GetItemState( SID_PASSWORD, sal_False, &pItem ) )
                m_sFilePasswd = ( (SfxStringItem*)pItem )->GetValue();
            aFileNameED.SetText( INetURLObject::decode(
                m_sFileName, INET_HEX_ESCAPE, INetURLObject::DECODE_UNAMBIGUOUS, RTL_TEXTENCODING_UTF8 ) );
            ::lcl_ReadSections( *pMedium, aSubRegionED );
            delete pMedium;
        }
    }
    else
        m_sFilterName = m_sFilePasswd = aEmptyStr;

    Application::SetDefDialogParent( m_pOldDefDlgParent );
    return 0;
}

// --------------------------------------------------------------

// Numerierungsformat Umsetzung:
// ListBox	- Format			- Enum-Wert
// 0 		- A, B, C, ...		- 0
// 1		- a, b, c, ...		- 1
// 2		- I, II, III, ... 	- 2
// 3        - i, ii, iii, ...	- 3
// 4        - 1, 2, 3, ...		- 4
// 5        - A, .., AA, .., 	- 9
// 6        - a, .., aa, .., 	- 10

inline sal_uInt16 GetNumPos( sal_uInt16 n )
{
	return SVX_NUM_ARABIC < n ? n - 4 : n;
}

inline SvxExtNumType GetNumType( sal_uInt16 n )
{
	return (SvxExtNumType)(4 < n ? n + 4 : n );
}

SwSectionFtnEndTabPage::SwSectionFtnEndTabPage( Window *pParent,
												const SfxItemSet &rAttrSet)
	: SfxTabPage( pParent, SW_RES( TP_SECTION_FTNENDNOTES ), rAttrSet ),
    aFtnFL              ( this, SW_RES( FL_FTN ) ),
	aFtnNtAtTextEndCB   ( this, SW_RES( CB_FTN_AT_TXTEND ) ),

    aFtnNtNumCB         ( this, SW_RES( CB_FTN_NUM ) ),
    aFtnOffsetLbl       ( this, SW_RES( FT_FTN_OFFSET   )),
    aFtnOffsetFld       ( this, SW_RES( FLD_FTN_OFFSET   )),

    aFtnNtNumFmtCB      ( this, SW_RES( CB_FTN_NUM_FMT ) ),
    aFtnPrefixFT        ( this, SW_RES( FT_FTN_PREFIX   )),
    aFtnPrefixED        ( this, SW_RES( ED_FTN_PREFIX    )),
    aFtnNumViewBox      ( this, SW_RES( LB_FTN_NUMVIEW  ), INSERT_NUM_EXTENDED_TYPES),
	aFtnSuffixFT		( this, SW_RES( FT_FTN_SUFFIX    )),
	aFtnSuffixED		( this, SW_RES( ED_FTN_SUFFIX    )),

    aEndFL              ( this, SW_RES( FL_END ) ),
	aEndNtAtTextEndCB   ( this, SW_RES( CB_END_AT_TXTEND )),

    aEndNtNumCB         ( this, SW_RES( CB_END_NUM )),
    aEndOffsetLbl       ( this, SW_RES( FT_END_OFFSET   )),
    aEndOffsetFld       ( this, SW_RES( FLD_END_OFFSET   )),

    aEndNtNumFmtCB      ( this, SW_RES( CB_END_NUM_FMT ) ),
    aEndPrefixFT        ( this, SW_RES( FT_END_PREFIX   )),
    aEndPrefixED        ( this, SW_RES( ED_END_PREFIX    )),
    aEndNumViewBox      ( this, SW_RES( LB_END_NUMVIEW  ), INSERT_NUM_EXTENDED_TYPES),
	aEndSuffixFT		( this, SW_RES( FT_END_SUFFIX    )),
	aEndSuffixED		( this, SW_RES( ED_END_SUFFIX    ))
{
	FreeResource();

	Link aLk( LINK( this, SwSectionFtnEndTabPage, FootEndHdl));
	aFtnNtAtTextEndCB.SetClickHdl( aLk );
	aFtnNtNumCB.SetClickHdl( aLk );
	aEndNtAtTextEndCB.SetClickHdl( aLk );
	aEndNtNumCB.SetClickHdl( aLk );
	aFtnNtNumFmtCB.SetClickHdl( aLk );
	aEndNtNumFmtCB.SetClickHdl( aLk );
}

SwSectionFtnEndTabPage::~SwSectionFtnEndTabPage()
{
}

sal_Bool SwSectionFtnEndTabPage::FillItemSet( SfxItemSet& rSet )
{
	SwFmtFtnAtTxtEnd aFtn( aFtnNtAtTextEndCB.IsChecked()
							? ( aFtnNtNumCB.IsChecked()
								? ( aFtnNtNumFmtCB.IsChecked()
									? FTNEND_ATTXTEND_OWNNUMANDFMT
									: FTNEND_ATTXTEND_OWNNUMSEQ )
								: FTNEND_ATTXTEND )
							: FTNEND_ATPGORDOCEND );

	switch( aFtn.GetValue() )
	{
	case FTNEND_ATTXTEND_OWNNUMANDFMT:
		aFtn.SetNumType( aFtnNumViewBox.GetSelectedNumberingType() );
		aFtn.SetPrefix( aFtnPrefixED.GetText() );
		aFtn.SetSuffix( aFtnSuffixED.GetText() );
		// no break;

	case FTNEND_ATTXTEND_OWNNUMSEQ:
        aFtn.SetOffset( static_cast< sal_uInt16 >( aFtnOffsetFld.GetValue()-1 ) );
		// no break;
	}

	SwFmtEndAtTxtEnd aEnd( aEndNtAtTextEndCB.IsChecked()
							? ( aEndNtNumCB.IsChecked()
								? ( aEndNtNumFmtCB.IsChecked()
									? FTNEND_ATTXTEND_OWNNUMANDFMT
									: FTNEND_ATTXTEND_OWNNUMSEQ )
								: FTNEND_ATTXTEND )
							: FTNEND_ATPGORDOCEND );

	switch( aEnd.GetValue() )
	{
	case FTNEND_ATTXTEND_OWNNUMANDFMT:
        aEnd.SetNumType( aEndNumViewBox.GetSelectedNumberingType() );
		aEnd.SetPrefix( aEndPrefixED.GetText() );
		aEnd.SetSuffix( aEndSuffixED.GetText() );
		// no break;

	case FTNEND_ATTXTEND_OWNNUMSEQ:
        aEnd.SetOffset( static_cast< sal_uInt16 >( aEndOffsetFld.GetValue()-1 ) );
		// no break;
	}

	rSet.Put( aFtn );
	rSet.Put( aEnd );

	return sal_True;
}

void SwSectionFtnEndTabPage::ResetState( sal_Bool bFtn,
									const SwFmtFtnEndAtTxtEnd& rAttr )
{
	CheckBox *pNtAtTextEndCB, *pNtNumCB, *pNtNumFmtCB;
	FixedText*pPrefixFT, *pSuffixFT;
	Edit *pPrefixED, *pSuffixED;
	SwNumberingTypeListBox *pNumViewBox;
	FixedText* pOffsetTxt;
	NumericField *pOffsetFld;

	if( bFtn )
	{
		pNtAtTextEndCB = &aFtnNtAtTextEndCB;
		pNtNumCB = &aFtnNtNumCB;
		pNtNumFmtCB = &aFtnNtNumFmtCB;
		pPrefixFT = &aFtnPrefixFT;
		pPrefixED = &aFtnPrefixED;
		pSuffixFT = &aFtnSuffixFT;
		pSuffixED = &aFtnSuffixED;
		pNumViewBox = &aFtnNumViewBox;
		pOffsetTxt = &aFtnOffsetLbl;
		pOffsetFld = &aFtnOffsetFld;
	}
	else
	{
		pNtAtTextEndCB = &aEndNtAtTextEndCB;
		pNtNumCB = &aEndNtNumCB;
		pNtNumFmtCB = &aEndNtNumFmtCB;
		pPrefixFT = &aEndPrefixFT;
		pPrefixED = &aEndPrefixED;
		pSuffixFT = &aEndSuffixFT;
		pSuffixED = &aEndSuffixED;
		pNumViewBox = &aEndNumViewBox;
		pOffsetTxt = &aEndOffsetLbl;
		pOffsetFld = &aEndOffsetFld;
	}

	sal_uInt16 eState = rAttr.GetValue();
	switch( eState )
	{
	// case FTNEND_ATPGORDOCEND:
	case FTNEND_ATTXTEND_OWNNUMANDFMT:
		pNtNumFmtCB->SetState( STATE_CHECK );
		// no break;

	case FTNEND_ATTXTEND_OWNNUMSEQ:
		pNtNumCB->SetState( STATE_CHECK );
		// no break;

	case FTNEND_ATTXTEND:
		pNtAtTextEndCB->SetState( STATE_CHECK );
		// no break;
	}

	pNumViewBox->SelectNumberingType( rAttr.GetNumType() );
	pOffsetFld->SetValue( rAttr.GetOffset() + 1 );
	pPrefixED->SetText( rAttr.GetPrefix() );
	pSuffixED->SetText( rAttr.GetSuffix() );

	switch( eState )
	{
	case FTNEND_ATPGORDOCEND:
		pNtNumCB->Enable( sal_False );
		// no break;

	case FTNEND_ATTXTEND:
		pNtNumFmtCB->Enable( sal_False );
		pOffsetFld->Enable( sal_False );
		pOffsetTxt->Enable( sal_False );
		// no break;

	case FTNEND_ATTXTEND_OWNNUMSEQ:
		pNumViewBox->Enable( sal_False );
		pPrefixFT->Enable( sal_False );
		pPrefixED->Enable( sal_False );
		pSuffixFT->Enable( sal_False );
		pSuffixED->Enable( sal_False );
		// no break;
	}
}

void SwSectionFtnEndTabPage::Reset( const SfxItemSet& rSet )
{
	ResetState( sal_True, (const SwFmtFtnAtTxtEnd&)rSet.Get(
									RES_FTN_AT_TXTEND, sal_False ));
	ResetState( sal_False, (const SwFmtEndAtTxtEnd&)rSet.Get(
									RES_END_AT_TXTEND, sal_False ));
}

SfxTabPage*	SwSectionFtnEndTabPage::Create( Window* pParent,
								const SfxItemSet& rAttrSet)
{
	return new SwSectionFtnEndTabPage(pParent, rAttrSet);
}

IMPL_LINK( SwSectionFtnEndTabPage, FootEndHdl, CheckBox *, pBox )
{
//	pBox->EnableTriState( sal_False );
	sal_Bool bFoot = &aFtnNtAtTextEndCB == pBox || &aFtnNtNumCB == pBox ||
					&aFtnNtNumFmtCB == pBox ;

	CheckBox *pNumBox, *pNumFmtBox, *pEndBox;
	SwNumberingTypeListBox* pNumViewBox;
	FixedText* pOffsetTxt;
	NumericField *pOffsetFld;
	FixedText*pPrefixFT, *pSuffixFT;
	Edit *pPrefixED, *pSuffixED;

	if( bFoot )
	{
		pEndBox = &aFtnNtAtTextEndCB;
		pNumBox = &aFtnNtNumCB;
		pNumFmtBox = &aFtnNtNumFmtCB;
		pNumViewBox = &aFtnNumViewBox;
		pOffsetTxt = &aFtnOffsetLbl;
		pOffsetFld = &aFtnOffsetFld;
		pPrefixFT = &aFtnPrefixFT;
		pSuffixFT = &aFtnSuffixFT;
		pPrefixED = &aFtnPrefixED;
		pSuffixED = &aFtnSuffixED;
	}
	else
	{
		pEndBox = &aEndNtAtTextEndCB;
		pNumBox = &aEndNtNumCB;
		pNumFmtBox = &aEndNtNumFmtCB;
		pNumViewBox = &aEndNumViewBox;
		pOffsetTxt = &aEndOffsetLbl;
		pOffsetFld = &aEndOffsetFld;
		pPrefixFT = &aEndPrefixFT;
		pSuffixFT = &aEndSuffixFT;
		pPrefixED = &aEndPrefixED;
		pSuffixED = &aEndSuffixED;
	}

	sal_Bool bEnableAtEnd = STATE_CHECK == pEndBox->GetState();
	sal_Bool bEnableNum = bEnableAtEnd && STATE_CHECK == pNumBox->GetState();
	sal_Bool bEnableNumFmt = bEnableNum && STATE_CHECK == pNumFmtBox->GetState();

	pNumBox->Enable( bEnableAtEnd );
	pOffsetTxt->Enable( bEnableNum );
	pOffsetFld->Enable( bEnableNum );
	pNumFmtBox->Enable( bEnableNum );
	pNumViewBox->Enable( bEnableNumFmt );
	pPrefixED->Enable( bEnableNumFmt );
	pSuffixED->Enable( bEnableNumFmt );
	pPrefixFT->Enable( bEnableNumFmt );
	pSuffixFT->Enable( bEnableNumFmt );

	return 0;
}

/* -----------------21.05.99 13:59-------------------
 *
 * --------------------------------------------------*/
SwSectionPropertyTabDialog::SwSectionPropertyTabDialog(
	Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh) :
    SfxTabDialog(pParent, SW_RES(DLG_SECTION_PROPERTIES), &rSet),
    rWrtSh(rSh)
{
	FreeResource();
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");
	AddTabPage(TP_COLUMN,	SwColumnPage::Create,  	 0);
    AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );
	AddTabPage(TP_SECTION_FTNENDNOTES, SwSectionFtnEndTabPage::Create, 0);
    AddTabPage(TP_SECTION_INDENTS, SwSectionIndentTabPage::Create, 0);

	SvxHtmlOptions* pHtmlOpt = SvxHtmlOptions::Get();
	long nHtmlMode = pHtmlOpt->GetExportMode();
	sal_Bool bWeb = 0 != PTR_CAST( SwWebDocShell, rSh.GetView().GetDocShell() );
	if(bWeb)
	{
		RemoveTabPage(TP_SECTION_FTNENDNOTES);
        RemoveTabPage(TP_SECTION_INDENTS);
        if( HTML_CFG_NS40 != nHtmlMode && HTML_CFG_WRITER != nHtmlMode)
			RemoveTabPage(TP_COLUMN);
	}
}
/* -----------------21.05.99 13:59-------------------
 *
 * --------------------------------------------------*/
SwSectionPropertyTabDialog::~SwSectionPropertyTabDialog()
{
}
/* -----------------21.05.99 13:59-------------------
 *
 * --------------------------------------------------*/
void SwSectionPropertyTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
	if( TP_BACKGROUND == nId  )
    {
			SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
			aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_SHOW_SELECTOR));
			rPage.PageCreated(aSet);
	}
	else if( TP_COLUMN == nId )
    {
		((SwColumnPage&)rPage).ShowBalance(sal_True);
        ((SwColumnPage&)rPage).SetInSection(sal_True);
    }
    else if(TP_SECTION_INDENTS == nId)
        ((SwSectionIndentTabPage&)rPage).SetWrtShell(rWrtSh);
}
/*-- 13.06.2003 09:59:08---------------------------------------------------

  -----------------------------------------------------------------------*/
SwSectionIndentTabPage::SwSectionIndentTabPage( Window *pParent, const SfxItemSet &rAttrSet ) :
    SfxTabPage(pParent, SW_RES(TP_SECTION_INDENTS), rAttrSet),
    aIndentFL(this,     SW_RES(FL_INDENT     )),
    aBeforeFT(this,     SW_RES(FT_BEFORE     )),
    aBeforeMF(this,     SW_RES(MF_BEFORE     )),
    aAfterFT(this,      SW_RES(FT_AFTER      )),
    aAfterMF(this,      SW_RES(MF_AFTER      )),
    aPreviewWin(this,   SW_RES(WIN_PREVIEW   ))
{
    FreeResource();
    Link aLk = LINK(this, SwSectionIndentTabPage, IndentModifyHdl);
    aBeforeMF.SetModifyHdl(aLk);
    aAfterMF.SetModifyHdl(aLk);
    aPreviewWin.SetAccessibleName(aIndentFL.GetText());
}
/*-- 13.06.2003 09:59:23---------------------------------------------------

  -----------------------------------------------------------------------*/
SwSectionIndentTabPage::~SwSectionIndentTabPage()
{
}
/*-- 13.06.2003 09:59:23---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwSectionIndentTabPage::FillItemSet( SfxItemSet& rSet)
{
    if(aBeforeMF.IsValueModified() ||
            aAfterMF.IsValueModified())
    {
        SvxLRSpaceItem aLRSpace(
                static_cast< long >(aBeforeMF.Denormalize(aBeforeMF.GetValue(FUNIT_TWIP))) ,
                static_cast< long >(aAfterMF.Denormalize(aAfterMF.GetValue(FUNIT_TWIP))), 0, 0, RES_LR_SPACE);
        rSet.Put(aLRSpace);
    }
    return sal_True;
}
/*-- 13.06.2003 09:59:24---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwSectionIndentTabPage::Reset( const SfxItemSet& rSet)
{
    //this page doesn't show up in HTML mode
    FieldUnit aMetric = ::GetDfltMetric(sal_False);
    SetMetric(aBeforeMF, aMetric);
    SetMetric(aAfterMF , aMetric);

    SfxItemState eItemState = rSet.GetItemState( RES_LR_SPACE );
    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        const SvxLRSpaceItem& rSpace =
            (const SvxLRSpaceItem&)rSet.Get( RES_LR_SPACE );

        aBeforeMF.SetValue( aBeforeMF.Normalize(rSpace.GetLeft()), FUNIT_TWIP );
        aAfterMF.SetValue( aAfterMF.Normalize(rSpace.GetRight()), FUNIT_TWIP );
    }
    else
    {
        aBeforeMF.SetEmptyFieldValue();
        aAfterMF.SetEmptyFieldValue();
    }
    aBeforeMF.SaveValue();
    aAfterMF.SaveValue();
    IndentModifyHdl(0);
}
/*-- 13.06.2003 09:59:24---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxTabPage*  SwSectionIndentTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet)
{
    return new SwSectionIndentTabPage(pParent, rAttrSet);
}
/* -----------------13.06.2003 13:57-----------------

 --------------------------------------------------*/
void SwSectionIndentTabPage::SetWrtShell(SwWrtShell& rSh)
{
    //set sensible values at the preview
    aPreviewWin.SetAdjust(SVX_ADJUST_BLOCK);
    aPreviewWin.SetLastLine(SVX_ADJUST_BLOCK);
    const SwRect& rPageRect = rSh.GetAnyCurRect( RECT_PAGE, 0 );
    Size aPageSize(rPageRect.Width(), rPageRect.Height());
    aPreviewWin.SetSize(aPageSize);
}
/* -----------------13.06.2003 14:02-----------------

 --------------------------------------------------*/
IMPL_LINK(SwSectionIndentTabPage, IndentModifyHdl, MetricField*, EMPTYARG)
{
    aPreviewWin.SetLeftMargin( static_cast< long >(aBeforeMF.Denormalize(aBeforeMF.GetValue(FUNIT_TWIP))) );
    aPreviewWin.SetRightMargin( static_cast< long >(aAfterMF.Denormalize(aAfterMF.GetValue(FUNIT_TWIP))) );
    aPreviewWin.Draw(sal_True);
    return 0;
}

