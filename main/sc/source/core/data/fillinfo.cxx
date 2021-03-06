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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <editeng/boxitem.hxx>
#include <editeng/bolnitem.hxx>
#include <editeng/editdata.hxx>		// can be removed if table has a bLayoutRTL flag
#include <editeng/shaditem.hxx>

#include "fillinfo.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "table.hxx"
#include "attrib.hxx"
#include "attarray.hxx"
#include "markarr.hxx"
#include "markdata.hxx"
#include "patattr.hxx"
#include "poolhelp.hxx"
#include "docpool.hxx"
#include "conditio.hxx"
#include "stlpool.hxx"

// -----------------------------------------------------------------------

const sal_uInt16 ROWINFO_MAX = 1024;


enum FillInfoLinePos
	{
		FILP_TOP,
		FILP_BOTTOM,
		FILP_LEFT,
		FILP_RIGHT
	};


inline const SvxBorderLine* GetNullOrLine( const SvxBoxItem* pBox, FillInfoLinePos eWhich )
{
	if (pBox)
	{
		if (eWhich==FILP_TOP)
			return pBox->GetTop();
		else if (eWhich==FILP_BOTTOM)
			return pBox->GetBottom();
		else if (eWhich==FILP_LEFT)
			return pBox->GetLeft();
		else
			return pBox->GetRight();
	}
	else
		return NULL;
}

//	aehnlich wie in output.cxx

void lcl_GetMergeRange( SCsCOL nX, SCsROW nY, SCSIZE nArrY,
							ScDocument* pDoc, RowInfo* pRowInfo,
							SCCOL nX1, SCROW nY1, SCCOL /* nX2 */, SCROW /* nY2 */, SCTAB nTab,
							SCsCOL& rStartX, SCsROW& rStartY, SCsCOL& rEndX, SCsROW& rEndY )
{
	CellInfo* pInfo = &pRowInfo[nArrY].pCellInfo[nX+1];

	rStartX = nX;
	rStartY = nY;
	sal_Bool bHOver = pInfo->bHOverlapped;
	sal_Bool bVOver = pInfo->bVOverlapped;
    SCCOL nLastCol;
    SCROW nLastRow;

	while (bHOver)				// nY konstant
	{
		--rStartX;
        if (rStartX >= (SCsCOL) nX1 && !pDoc->ColHidden(rStartX, nTab, nLastCol))
		{
			bHOver = pRowInfo[nArrY].pCellInfo[rStartX+1].bHOverlapped;
			bVOver = pRowInfo[nArrY].pCellInfo[rStartX+1].bVOverlapped;
		}
		else
		{
			sal_uInt16 nOverlap = ((ScMergeFlagAttr*)pDoc->GetAttr(
								rStartX, rStartY, nTab, ATTR_MERGE_FLAG ))->GetValue();
			bHOver = ((nOverlap & SC_MF_HOR) != 0);
			bVOver = ((nOverlap & SC_MF_VER) != 0);
		}
	}

	while (bVOver)
	{
		--rStartY;

		if (nArrY>0)
			--nArrY;						// lokale Kopie !

		if (rStartX >= (SCsCOL) nX1 && rStartY >= (SCsROW) nY1 &&
            !pDoc->ColHidden(rStartX, nTab, nLastCol) &&
            !pDoc->RowHidden(rStartY, nTab, nLastRow) &&
			(SCsROW) pRowInfo[nArrY].nRowNo == rStartY)
		{
			bHOver = pRowInfo[nArrY].pCellInfo[rStartX+1].bHOverlapped;
			bVOver = pRowInfo[nArrY].pCellInfo[rStartX+1].bVOverlapped;
		}
		else
		{
			sal_uInt16 nOverlap = ((ScMergeFlagAttr*)pDoc->GetAttr(
								rStartX, rStartY, nTab, ATTR_MERGE_FLAG ))->GetValue();
			bHOver = ((nOverlap & SC_MF_HOR) != 0);
			bVOver = ((nOverlap & SC_MF_VER) != 0);
		}
	}

	const ScMergeAttr* pMerge;
	if (rStartX >= (SCsCOL) nX1 && rStartY >= (SCsROW) nY1 &&
        !pDoc->ColHidden(rStartX, nTab, nLastCol) &&
        !pDoc->RowHidden(rStartY, nTab, nLastRow) &&
		(SCsROW) pRowInfo[nArrY].nRowNo == rStartY)
	{
		pMerge = (const ScMergeAttr*) &pRowInfo[nArrY].pCellInfo[rStartX+1].pPatternAttr->
										GetItem(ATTR_MERGE);
	}
	else
		pMerge = (const ScMergeAttr*) pDoc->GetAttr(rStartX,rStartY,nTab,ATTR_MERGE);

	rEndX = rStartX + pMerge->GetColMerge() - 1;
	rEndY = rStartY + pMerge->GetRowMerge() - 1;
}

#define CELLINFO(x,y) pRowInfo[nArrY+y].pCellInfo[nArrX+x]

void ScDocument::FillInfo( ScTableInfo& rTabInfo, SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
							SCTAB nTab, double nScaleX, double nScaleY,
							sal_Bool bPageMode, sal_Bool bFormulaMode, const ScMarkData* pMarkData )
{
	DBG_ASSERT( pTab[nTab], "Tabelle existiert nicht" );

	sal_Bool bLayoutRTL = IsLayoutRTL( nTab );

	ScDocumentPool* pPool = xPoolHelper->GetDocPool();
	ScStyleSheetPool* pStlPool = xPoolHelper->GetStylePool();

    RowInfo* pRowInfo = rTabInfo.mpRowInfo;

	const SvxBrushItem* pDefBackground =
			(const SvxBrushItem*) &pPool->GetDefaultItem( ATTR_BACKGROUND );
	const ScMergeAttr* pDefMerge =
			(const ScMergeAttr*) &pPool->GetDefaultItem( ATTR_MERGE );
	const SvxShadowItem* pDefShadow =
			(const SvxShadowItem*) &pPool->GetDefaultItem( ATTR_SHADOW );

	SCROW nThisRow;
	SCCOL nX;
	SCROW nY;
	SCsROW nSignedY;
	SCCOL nArrX;
	SCSIZE nArrY;
	SCSIZE nArrCount;
	sal_Bool bAnyMerged = sal_False;
	sal_Bool bAnyShadow = sal_False;
	sal_Bool bAnyCondition = sal_False;

	sal_Bool bTabProtect = IsTabProtected(nTab);

												// fuer Blockmarken von zusammengefassten Zellen mit
												// versteckter erster Zeile / Spalte
	sal_Bool bPaintMarks = sal_False;
	sal_Bool bSkipMarks = sal_False;
    SCCOL nBlockStartX = 0, nBlockEndX = 0;
    SCROW nBlockEndY = 0, nBlockStartY = 0;
	if (pMarkData && pMarkData->IsMarked())
	{
		ScRange aTmpRange;
		pMarkData->GetMarkArea(aTmpRange);
		if ( nTab >= aTmpRange.aStart.Tab() && nTab <= aTmpRange.aEnd.Tab() )
		{
			nBlockStartX = aTmpRange.aStart.Col();
			nBlockStartY = aTmpRange.aStart.Row();
			nBlockEndX = aTmpRange.aEnd.Col();
			nBlockEndY = aTmpRange.aEnd.Row();
			ExtendHidden( nBlockStartX, nBlockStartY, nBlockEndX, nBlockEndY, nTab );	//? noetig ?
			if (pMarkData->IsMarkNegative())
				bSkipMarks = sal_True;
			else
				bPaintMarks = sal_True;
		}
	}

	//	zuerst nur die Eintraege fuer die ganze Spalte

	nArrY=0;
	SCROW nYExtra = nY2+1;
    sal_uInt16 nDocHeight = ScGlobal::nStdRowHeight;
    SCROW nDocHeightEndRow = -1;
	for (nSignedY=((SCsROW)nY1)-1; nSignedY<=(SCsROW)nYExtra; nSignedY++)
	{
		if (nSignedY >= 0)
			nY = (SCROW) nSignedY;
		else
			nY = MAXROW+1;			// ungueltig

        if (nY > nDocHeightEndRow)
        {
            if (ValidRow(nY))
                nDocHeight = GetRowHeight( nY, nTab, NULL, &nDocHeightEndRow );
            else
                nDocHeight = ScGlobal::nStdRowHeight;
        }

		if ( nArrY==0 || nDocHeight || nY > MAXROW )
		{
			RowInfo* pThisRowInfo = &pRowInfo[nArrY];
			pThisRowInfo->pCellInfo = NULL;					// wird unten belegt

			sal_uInt16 nHeight = (sal_uInt16) ( nDocHeight * nScaleY );
			if (!nHeight)
				nHeight = 1;

			pThisRowInfo->nRowNo		= nY;				//! Fall < 0 ?
			pThisRowInfo->nHeight 		= nHeight;
			pThisRowInfo->bEmptyBack	= sal_True;
			pThisRowInfo->bEmptyText	= sal_True;
			pThisRowInfo->bChanged		= sal_True;
			pThisRowInfo->bAutoFilter	= sal_False;
			pThisRowInfo->bPushButton	= sal_False;
			pThisRowInfo->nRotMaxCol	= SC_ROTMAX_NONE;

			++nArrY;
			if (nArrY >= ROWINFO_MAX)
			{
				DBG_ERROR("Zu grosser Bereich bei FillInfo" );
				nYExtra = nSignedY;									// Ende
				nY2 = nYExtra - 1;									// Bereich anpassen
			}
		}
		else
			if (nSignedY==(SCsROW) nYExtra)							// zusaetzliche Zeile verdeckt ?
				++nYExtra;
	}
	nArrCount = nArrY;										// incl. Dummys

	//	rotierter Text...

	//	Attribut im Dokument ueberhaupt verwendet?
	sal_Bool bAnyItem = sal_False;
	sal_uInt32 nRotCount = pPool->GetItemCount2( ATTR_ROTATE_VALUE );
	for (sal_uInt32 nItem=0; nItem<nRotCount; nItem++)
		if (pPool->GetItem2( ATTR_ROTATE_VALUE, nItem ))
		{
			bAnyItem = sal_True;
			break;
		}

	SCCOL nRotMax = nX2;
	if ( bAnyItem && HasAttrib( 0,nY1,nTab, MAXCOL,nY2+1,nTab,
								HASATTR_ROTATE | HASATTR_CONDITIONAL ) )
	{
		//!	Conditionals auch bei HASATTR_ROTATE abfragen ????

		DBG_ASSERT( nArrCount>2, "nArrCount zu klein" );
//		FindMaxRotCol( nTab, &pRowInfo[1], nArrCount-2, nX1, nX2 );
		FindMaxRotCol( nTab, &pRowInfo[1], nArrCount-1, nX1, nX2 );
		//	FindMaxRotCol setzt nRotMaxCol

		for (nArrY=0; nArrY<nArrCount; nArrY++)
			if (pRowInfo[nArrY].nRotMaxCol != SC_ROTMAX_NONE && pRowInfo[nArrY].nRotMaxCol > nRotMax)
				nRotMax = pRowInfo[nArrY].nRotMaxCol;
	}

	//	Zell-Infos erst nach dem Test auf gedrehte allozieren
	//	bis nRotMax wegen nRotateDir Flag

	for (nArrY=0; nArrY<nArrCount; nArrY++)
	{
		RowInfo* pThisRowInfo = &pRowInfo[nArrY];
		nY = pThisRowInfo->nRowNo;
		pThisRowInfo->pCellInfo = new CellInfo[ nRotMax+1+2 ];	// vom Aufrufer zu loeschen !

		for (nArrX=0; nArrX<=nRotMax+2; nArrX++)				// Zell-Infos vorbelegen
		{
			if (nArrX>0)
				nX = nArrX-1;
			else
				nX = MAXCOL+1;		// ungueltig

			CellInfo* pInfo = &pThisRowInfo->pCellInfo[nArrX];
			pInfo->bEmptyCellText = sal_True;
			pInfo->pCell = NULL;
			if (bPaintMarks)
				pInfo->bMarked = ( nX >= nBlockStartX && nX <= nBlockEndX
								&& nY >= nBlockStartY && nY <= nBlockEndY );
			else
				pInfo->bMarked = sal_False;
			pInfo->nWidth = 0;

			pInfo->nClipMark	= SC_CLIPMARK_NONE;
			pInfo->bMerged		= sal_False;
			pInfo->bHOverlapped = sal_False;
			pInfo->bVOverlapped = sal_False;
			pInfo->bAutoFilter	= sal_False;
			pInfo->bPushButton	= sal_False;
            pInfo->bPopupButton = false;
            pInfo->bFilterActive = false;
			pInfo->nRotateDir	= SC_ROTDIR_NONE;

			pInfo->bPrinted		= sal_False;					//	view-intern
			pInfo->bHideGrid	= sal_False;					//	view-intern
			pInfo->bEditEngine	= sal_False;					//	view-intern

			pInfo->pBackground  = NULL;						//! weglassen?
			pInfo->pPatternAttr = NULL;
			pInfo->pConditionSet= NULL;

            pInfo->pLinesAttr   = NULL;
            pInfo->mpTLBRLine   = NULL;
            pInfo->mpBLTRLine   = NULL;

			pInfo->pShadowAttr	  = pDefShadow;
			pInfo->pHShadowOrigin = NULL;
			pInfo->pVShadowOrigin = NULL;
		}
	}

	for (nArrX=nX2+3; nArrX<=nRotMax+2; nArrX++)			// restliche Breiten eintragen
	{
		nX = nArrX-1;
		if ( ValidCol(nX) )
		{
            if (!ColHidden(nX, nTab))
			{
				sal_uInt16 nThisWidth = (sal_uInt16) (GetColWidth( nX, nTab ) * nScaleX);
				if (!nThisWidth)
					nThisWidth = 1;

				pRowInfo[0].pCellInfo[nArrX].nWidth = nThisWidth;
			}
		}
	}

	for (nArrX=0; nArrX<=nX2+2; nArrX++)					// links & rechts + 1
	{
		nX = (nArrX>0) ? nArrX-1 : MAXCOL+1;					// negativ -> ungueltig

		if ( ValidCol(nX) )
		{
            // #i58049#, #i57939# Hidden columns must be skipped here, or their attributes
            // will disturb the output

            // TODO: Optimize this loop.
            if (!ColHidden(nX, nTab))
			{
				sal_uInt16 nThisWidth = (sal_uInt16) (GetColWidth( nX, nTab ) * nScaleX);
                if (!nThisWidth)
					nThisWidth = 1;

				pRowInfo[0].pCellInfo[nArrX].nWidth = nThisWidth;			//! dies sollte reichen

				ScColumn* pThisCol = &pTab[nTab]->aCol[nX];					// Spalten-Daten

                nArrY = 1;
                SCSIZE nUIndex;
                bool bHiddenRow = true;
                SCROW nHiddenEndRow = -1;
                (void) pThisCol->Search( nY1, nUIndex );
                while ( nUIndex < pThisCol->nCount &&
                        (nThisRow=pThisCol->pItems[nUIndex].nRow) <= nY2 )
                {
                    if (nThisRow > nHiddenEndRow)
                        bHiddenRow = RowHidden( nThisRow, nTab, nHiddenEndRow);
                    if ( !bHiddenRow )
                    {
                        while ( pRowInfo[nArrY].nRowNo < nThisRow )
                            ++nArrY;
                        DBG_ASSERT( pRowInfo[nArrY].nRowNo == nThisRow, "Zeile nicht gefunden in FillInfo" );

                        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
                        CellInfo* pInfo = &pThisRowInfo->pCellInfo[nArrX];
                        pInfo->pCell = pThisCol->pItems[nUIndex].pCell;
                        if (pInfo->pCell->GetCellType() != CELLTYPE_NOTE)
                        {
                            pThisRowInfo->bEmptyText = sal_False;                   // Zeile nicht leer
                            pInfo->bEmptyCellText = sal_False;                      // Zelle nicht leer
                        }
                        ++nArrY;
                    }
                    ++nUIndex;
                }

				if (nX+1 >= nX1)								// Attribute/Blockmarken ab nX1-1
				{
					ScAttrArray* pThisAttrArr = pThisCol->pAttrArray;		// Attribute

					nArrY = 0;
					const ScPatternAttr* pPattern;
					SCROW nCurRow=nY1;					// einzelne Zeile
					if (nCurRow>0)
						--nCurRow;						// oben 1 mehr
					else
						nArrY = 1;
					nThisRow=nCurRow;					// Ende des Bereichs
					SCSIZE  nIndex;
					(void) pThisAttrArr->Search( nCurRow, nIndex );


					do
					{
						nThisRow=pThisAttrArr->pData[nIndex].nRow;				// Ende des Bereichs
						pPattern=pThisAttrArr->pData[nIndex].pPattern;

						const SvxBrushItem* pBackground = (const SvxBrushItem*)
														&pPattern->GetItem(ATTR_BACKGROUND);
						const SvxBoxItem* pLinesAttr = (const SvxBoxItem*)
														&pPattern->GetItem(ATTR_BORDER);

                        const SvxLineItem* pTLBRLine = static_cast< const SvxLineItem* >(
                            &pPattern->GetItem( ATTR_BORDER_TLBR ) );
                        const SvxLineItem* pBLTRLine = static_cast< const SvxLineItem* >(
                            &pPattern->GetItem( ATTR_BORDER_BLTR ) );

						const SvxShadowItem* pShadowAttr = (const SvxShadowItem*)
														&pPattern->GetItem(ATTR_SHADOW);
						if (pShadowAttr != pDefShadow)
							bAnyShadow = sal_True;

						const ScMergeAttr* pMergeAttr = (const ScMergeAttr*)
												&pPattern->GetItem(ATTR_MERGE);
						sal_Bool bMerged = ( pMergeAttr != pDefMerge && *pMergeAttr != *pDefMerge );
						sal_uInt16 nOverlap = ((const ScMergeFlagAttr*) &pPattern->GetItemSet().
														Get(ATTR_MERGE_FLAG))->GetValue();
						sal_Bool bHOverlapped = ((nOverlap & SC_MF_HOR) != 0);
						sal_Bool bVOverlapped = ((nOverlap & SC_MF_VER) != 0);
						sal_Bool bAutoFilter  = ((nOverlap & SC_MF_AUTO) != 0);
						sal_Bool bPushButton  = ((nOverlap & SC_MF_BUTTON) != 0);
						sal_Bool bScenario	  = ((nOverlap & SC_MF_SCENARIO) != 0);
                        bool bPopupButton = ((nOverlap & SC_MF_BUTTON_POPUP) != 0);
                        bool bFilterActive = ((nOverlap & SC_MF_HIDDEN_MEMBER) != 0);
						if (bMerged||bHOverlapped||bVOverlapped)
							bAnyMerged = sal_True;								// intern

						sal_Bool bHidden, bHideFormula;
						if (bTabProtect)
						{
							const ScProtectionAttr& rProtAttr = (const ScProtectionAttr&)
														pPattern->GetItem(ATTR_PROTECTION);
							bHidden = rProtAttr.GetHideCell();
							bHideFormula = rProtAttr.GetHideFormula();
						}
						else
							bHidden = bHideFormula = sal_False;

						sal_uLong nConditional = ((const SfxUInt32Item&)pPattern->
												GetItem(ATTR_CONDITIONAL)).GetValue();
						const ScConditionalFormat* pCondForm = NULL;
						if ( nConditional && pCondFormList )
							pCondForm = pCondFormList->GetFormat( nConditional );

						do
						{
                            SCROW nLastHiddenRow = -1;
                            bool bRowHidden = RowHidden(nCurRow, nTab, nLastHiddenRow);
							if ( nArrY==0 || !bRowHidden )
							{
								RowInfo* pThisRowInfo = &pRowInfo[nArrY];
								if (pBackground != pDefBackground)			// Spalten-HG == Standard ?
									pThisRowInfo->bEmptyBack = sal_False;
								if (bAutoFilter)
									pThisRowInfo->bAutoFilter = sal_True;
								if (bPushButton)
									pThisRowInfo->bPushButton = sal_True;

								CellInfo* pInfo = &pThisRowInfo->pCellInfo[nArrX];
								pInfo->pBackground	= pBackground;
								pInfo->pPatternAttr	= pPattern;
								pInfo->bMerged		= bMerged;
								pInfo->bHOverlapped	= bHOverlapped;
								pInfo->bVOverlapped	= bVOverlapped;
								pInfo->bAutoFilter	= bAutoFilter;
								pInfo->bPushButton	= bPushButton;
                                pInfo->bPopupButton = bPopupButton;
                                pInfo->bFilterActive = bFilterActive;
								pInfo->pLinesAttr	= pLinesAttr;
                                pInfo->mpTLBRLine   = pTLBRLine;
                                pInfo->mpBLTRLine   = pBLTRLine;
								pInfo->pShadowAttr	= pShadowAttr;
								//	nWidth wird nicht mehr einzeln gesetzt

                                sal_Bool bEmbed = sal_False; //bIsEmbedded &&
										nTab	>= aEmbedRange.aStart.Tab() &&
										nTab    <= aEmbedRange.aEnd.Tab()   &&
										nX		>= aEmbedRange.aStart.Col() &&
										nX	    <= aEmbedRange.aEnd.Col()   &&
										nCurRow >= aEmbedRange.aStart.Row() &&
										nCurRow <= aEmbedRange.aEnd.Row();

								if (bScenario)
								{
									pInfo->pBackground = ScGlobal::GetButtonBrushItem();
									pThisRowInfo->bEmptyBack = sal_False;
								}
								else if (bEmbed)
								{
									pInfo->pBackground = ScGlobal::GetEmbeddedBrushItem();
									pThisRowInfo->bEmptyBack = sal_False;
								}

								if (bHidden || ( bFormulaMode && bHideFormula && pInfo->pCell
													&& pInfo->pCell->GetCellType()
														== CELLTYPE_FORMULA ))
									pInfo->bEmptyCellText = sal_True;

								if ( pCondForm )
								{
									String aStyle = pCondForm->GetCellStyle( pInfo->pCell,
														ScAddress( nX, nCurRow, nTab ) );
									if (aStyle.Len())
									{
										SfxStyleSheetBase* pStyleSheet =
												pStlPool->Find( aStyle, SFX_STYLE_FAMILY_PARA );
										if ( pStyleSheet )
										{
											//!	Style-Sets cachen !!!
											pInfo->pConditionSet = &pStyleSheet->GetItemSet();
											bAnyCondition = sal_True;
										}
										// if style is not there, treat like no condition
									}
								}

								++nArrY;
							}
                            else if (bRowHidden && nLastHiddenRow >= 0)
                            {
                                nCurRow = nLastHiddenRow;
                                if (nCurRow > nThisRow)
                                    nCurRow = nThisRow;
                            }
							++nCurRow;
						}
						while (nCurRow <= nThisRow && nCurRow <= nYExtra);
						++nIndex;
					}
					while ( nIndex < pThisAttrArr->nCount && nThisRow < nYExtra );


					if (pMarkData && pMarkData->IsMultiMarked())
					{
						//	Blockmarken
						const ScMarkArray* pThisMarkArr = pMarkData->GetArray()+nX;
						sal_Bool bThisMarked;
						nArrY = 1;
						nCurRow = nY1;										// einzelne Zeile
						nThisRow = nY1;										// Ende des Bereichs

                        if ( pThisMarkArr->Search( nY1, nIndex ) )
                        {
                            do
                            {
                                nThisRow=pThisMarkArr->pData[nIndex].nRow;      // Ende des Bereichs
                                bThisMarked=pThisMarkArr->pData[nIndex].bMarked;

                                do
                                {
                                    if ( !RowHidden( nCurRow,nTab ) )
                                    {
                                        if ( bThisMarked )
                                        {
                                            sal_Bool bSkip = bSkipMarks &&
                                                        nX      >= nBlockStartX &&
                                                        nX      <= nBlockEndX   &&
                                                        nCurRow >= nBlockStartY &&
                                                        nCurRow <= nBlockEndY;
                                            if (!bSkip)
                                            {
                                                RowInfo* pThisRowInfo = &pRowInfo[nArrY];
                                                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nArrX];
                                                pInfo->bMarked = sal_True;
                                            }
                                        }
                                        ++nArrY;
                                    }
                                    ++nCurRow;
                                }
                                while (nCurRow <= nThisRow && nCurRow <= nY2);
                                ++nIndex;
                            }
                            while ( nIndex < pThisMarkArr->nCount && nThisRow < nY2 );
                        }
					}
				}
				else									// vordere Spalten
				{
					for (nArrY=1; nArrY+1<nArrCount; nArrY++)
					{
						RowInfo* pThisRowInfo = &pRowInfo[nArrY];
						CellInfo* pInfo = &pThisRowInfo->pCellInfo[nArrX];

						pInfo->nWidth		= nThisWidth;			//! oder nur 0 abfragen ??
					}
				}
			}
		}
		else
			pRowInfo[0].pCellInfo[nArrX].nWidth = STD_COL_WIDTH;
		// STD_COL_WIDTH ganz links und rechts wird fuer DrawExtraShadow gebraucht
	}

	//-------------------------------------------------------------------------
	//	bedingte Formatierung auswerten

	if (bAnyCondition)
	{
		for (nArrY=0; nArrY<nArrCount; nArrY++)
		{
			for (nArrX=nX1; nArrX<=nX2+2; nArrX++)					// links und rechts einer mehr
			{
				CellInfo* pInfo = &pRowInfo[nArrY].pCellInfo[nArrX];
				const SfxItemSet* pCondSet = pInfo->pConditionSet;
				if (pCondSet)
				{
					const SfxPoolItem* pItem;

							//	Hintergrund
					if ( pCondSet->GetItemState( ATTR_BACKGROUND, sal_True, &pItem ) == SFX_ITEM_SET )
					{
						pInfo->pBackground = (const SvxBrushItem*) pItem;
						pRowInfo[nArrY].bEmptyBack = sal_False;
					}

							//	Umrandung
					if ( pCondSet->GetItemState( ATTR_BORDER, sal_True, &pItem ) == SFX_ITEM_SET )
						pInfo->pLinesAttr = (const SvxBoxItem*) pItem;

                    if ( pCondSet->GetItemState( ATTR_BORDER_TLBR, sal_True, &pItem ) == SFX_ITEM_SET )
                        pInfo->mpTLBRLine = static_cast< const SvxLineItem* >( pItem );
                    if ( pCondSet->GetItemState( ATTR_BORDER_BLTR, sal_True, &pItem ) == SFX_ITEM_SET )
                        pInfo->mpBLTRLine = static_cast< const SvxLineItem* >( pItem );

							//	Schatten
					if ( pCondSet->GetItemState( ATTR_SHADOW, sal_True, &pItem ) == SFX_ITEM_SET )
					{
						pInfo->pShadowAttr = (const SvxShadowItem*) pItem;
						bAnyShadow = sal_True;
					}
				}
			}
		}
	}

	//	bedingte Formatierung Ende
	//-------------------------------------------------------------------------

				//
				//		Daten von zusammengefassten Zellen anpassen
				//

	if (bAnyMerged)
	{
		for (nArrY=0; nArrY<nArrCount; nArrY++)
		{
			RowInfo* pThisRowInfo = &pRowInfo[nArrY];
            nSignedY = nArrY ? pThisRowInfo->nRowNo : ((SCsROW)nY1)-1;

			for (nArrX=nX1; nArrX<=nX2+2; nArrX++)					// links und rechts einer mehr
			{
                SCsCOL nSignedX = ((SCsCOL) nArrX) - 1;
				CellInfo* pInfo = &pThisRowInfo->pCellInfo[nArrX];

				if (pInfo->bMerged || pInfo->bHOverlapped || pInfo->bVOverlapped)
				{
					SCsCOL nStartX;
					SCsROW nStartY;
					SCsCOL nEndX;
					SCsROW nEndY;
					lcl_GetMergeRange( nSignedX,nSignedY, nArrY, this,pRowInfo, nX1,nY1,nX2,nY2,nTab,
										nStartX,nStartY, nEndX,nEndY );
					const ScPatternAttr* pStartPattern = GetPattern( nStartX,nStartY,nTab );
					const SfxItemSet* pStartCond = GetCondResult( nStartX,nStartY,nTab );
					const SfxPoolItem* pItem;

					// Hintergrund kopieren (oder in output.cxx)

					if ( !pStartCond || pStartCond->
									GetItemState(ATTR_BACKGROUND,sal_True,&pItem) != SFX_ITEM_SET )
						pItem = &pStartPattern->GetItem(ATTR_BACKGROUND);
					pInfo->pBackground = (const SvxBrushItem*) pItem;
					pRowInfo[nArrY].bEmptyBack = sal_False;

					// Schatten

					if ( !pStartCond || pStartCond->
									GetItemState(ATTR_SHADOW,sal_True,&pItem) != SFX_ITEM_SET )
						pItem = &pStartPattern->GetItem(ATTR_SHADOW);
					pInfo->pShadowAttr = (const SvxShadowItem*) pItem;
					if (pInfo->pShadowAttr != pDefShadow)
						bAnyShadow = sal_True;

					// Blockmarken - wieder mit Original-Merge-Werten

					sal_Bool bCellMarked = sal_False;
					if (bPaintMarks)
						bCellMarked = ( nStartX >= (SCsCOL) nBlockStartX
									&& nStartX <= (SCsCOL) nBlockEndX
									&& nStartY >= (SCsROW) nBlockStartY
									&& nStartY <= (SCsROW) nBlockEndY );
					if (pMarkData && pMarkData->IsMultiMarked() && !bCellMarked)
					{
						const ScMarkArray* pThisMarkArr = pMarkData->GetArray()+nStartX;
						SCSIZE nIndex;
                        if ( pThisMarkArr->Search( nStartY, nIndex ) )
                            bCellMarked=pThisMarkArr->pData[nIndex].bMarked;
					}

					pInfo->bMarked = bCellMarked;
				}
			}
		}
	}

	if (bAnyShadow)								// Schatten verteilen
	{
		for (nArrY=0; nArrY<nArrCount; nArrY++)
		{
			sal_Bool bTop = ( nArrY == 0 );
			sal_Bool bBottom = ( nArrY+1 == nArrCount );

			for (nArrX=nX1; nArrX<=nX2+2; nArrX++)					// links und rechts einer mehr
			{
				sal_Bool bLeft = ( nArrX == nX1 );
				sal_Bool bRight = ( nArrX == nX2+2 );

				CellInfo* pInfo = &pRowInfo[nArrY].pCellInfo[nArrX];
				const SvxShadowItem* pThisAttr = pInfo->pShadowAttr;
				SvxShadowLocation eLoc = pThisAttr ? pThisAttr->GetLocation() : SVX_SHADOW_NONE;
				if (eLoc != SVX_SHADOW_NONE)
				{
					//	oder Test auf != eLoc

					SCsCOL nDxPos = 1;
					SCsCOL nDxNeg = -1;

					while ( nArrX+nDxPos < nX2+2 && pRowInfo[0].pCellInfo[nArrX+nDxPos].nWidth == 0 )
						++nDxPos;
					while ( nArrX+nDxNeg > nX1 && pRowInfo[0].pCellInfo[nArrX+nDxNeg].nWidth == 0 )
						--nDxNeg;

					sal_Bool bLeftDiff = !bLeft &&
							CELLINFO(nDxNeg,0).pShadowAttr->GetLocation() == SVX_SHADOW_NONE;
					sal_Bool bRightDiff = !bRight &&
							CELLINFO(nDxPos,0).pShadowAttr->GetLocation() == SVX_SHADOW_NONE;
					sal_Bool bTopDiff = !bTop &&
							CELLINFO(0,-1).pShadowAttr->GetLocation() == SVX_SHADOW_NONE;
					sal_Bool bBottomDiff = !bBottom &&
							CELLINFO(0,1).pShadowAttr->GetLocation() == SVX_SHADOW_NONE;

					if ( bLayoutRTL )
					{
						switch (eLoc)
						{
							case SVX_SHADOW_BOTTOMRIGHT: eLoc = SVX_SHADOW_BOTTOMLEFT;	break;
							case SVX_SHADOW_BOTTOMLEFT:	 eLoc = SVX_SHADOW_BOTTOMRIGHT;	break;
							case SVX_SHADOW_TOPRIGHT:	 eLoc = SVX_SHADOW_TOPLEFT;		break;
							case SVX_SHADOW_TOPLEFT:	 eLoc = SVX_SHADOW_TOPRIGHT;	break;
                            default:
                            {
                                // added to avoid warnings
                            }
						}
					}

					switch (eLoc)
					{
						case SVX_SHADOW_BOTTOMRIGHT:
							if (bBottomDiff)
							{
								CELLINFO(0,1).pHShadowOrigin = pThisAttr;
								CELLINFO(0,1).eHShadowPart =
												bLeftDiff ? SC_SHADOW_HSTART : SC_SHADOW_HORIZ;
							}
							if (bRightDiff)
							{
								CELLINFO(1,0).pVShadowOrigin = pThisAttr;
								CELLINFO(1,0).eVShadowPart =
												bTopDiff ? SC_SHADOW_VSTART : SC_SHADOW_VERT;
							}
							if (bBottomDiff && bRightDiff)
							{
								CELLINFO(1,1).pHShadowOrigin = pThisAttr;
								CELLINFO(1,1).eHShadowPart = SC_SHADOW_CORNER;
							}
							break;

						case SVX_SHADOW_BOTTOMLEFT:
							if (bBottomDiff)
							{
								CELLINFO(0,1).pHShadowOrigin = pThisAttr;
								CELLINFO(0,1).eHShadowPart =
												bRightDiff ? SC_SHADOW_HSTART : SC_SHADOW_HORIZ;
							}
							if (bLeftDiff)
							{
								CELLINFO(-1,0).pVShadowOrigin = pThisAttr;
								CELLINFO(-1,0).eVShadowPart =
												bTopDiff ? SC_SHADOW_VSTART : SC_SHADOW_VERT;
							}
							if (bBottomDiff && bLeftDiff)
							{
								CELLINFO(-1,1).pHShadowOrigin = pThisAttr;
								CELLINFO(-1,1).eHShadowPart = SC_SHADOW_CORNER;
							}
							break;

						case SVX_SHADOW_TOPRIGHT:
							if (bTopDiff)
							{
								CELLINFO(0,-1).pHShadowOrigin = pThisAttr;
								CELLINFO(0,-1).eHShadowPart =
												bLeftDiff ? SC_SHADOW_HSTART : SC_SHADOW_HORIZ;
							}
							if (bRightDiff)
							{
								CELLINFO(1,0).pVShadowOrigin = pThisAttr;
								CELLINFO(1,0).eVShadowPart =
												bBottomDiff ? SC_SHADOW_VSTART : SC_SHADOW_VERT;
							}
							if (bTopDiff && bRightDiff)
							{
								CELLINFO(1,-1).pHShadowOrigin = pThisAttr;
								CELLINFO(1,-1).eHShadowPart = SC_SHADOW_CORNER;
							}
							break;

						case SVX_SHADOW_TOPLEFT:
							if (bTopDiff)
							{
								CELLINFO(0,-1).pHShadowOrigin = pThisAttr;
								CELLINFO(0,-1).eHShadowPart =
												bRightDiff ? SC_SHADOW_HSTART : SC_SHADOW_HORIZ;
							}
							if (bLeftDiff)
							{
								CELLINFO(-1,0).pVShadowOrigin = pThisAttr;
								CELLINFO(-1,0).eVShadowPart =
												bBottomDiff ? SC_SHADOW_VSTART : SC_SHADOW_VERT;
							}
							if (bTopDiff && bLeftDiff)
							{
								CELLINFO(-1,-1).pHShadowOrigin = pThisAttr;
								CELLINFO(-1,-1).eHShadowPart = SC_SHADOW_CORNER;
							}
							break;

						default:
							DBG_ERROR("falscher Shadow-Enum");
					}
				}
			}
		}
	}

    rTabInfo.mnArrCount = sal::static_int_cast<sal_uInt16>(nArrCount);
    rTabInfo.mbPageMode = bPageMode;

    // ========================================================================
    // *** create the frame border array ***

    // RowInfo structs are filled in the range [ 0 , nArrCount-1 ]
    // each RowInfo contains CellInfo structs in the range [ nX1-1 , nX2+1 ]

    size_t nColCount = nX2 - nX1 + 3;
    size_t nRowCount = nArrCount;

    svx::frame::Array& rArray = rTabInfo.maArray;
    rArray.Initialize( nColCount, nRowCount );
    rArray.SetUseDiagDoubleClipping( false );

    for( size_t nRow = 0; nRow < nRowCount; ++nRow )
    {
        sal_uInt16 nCellInfoY = static_cast< sal_uInt16 >( nRow );
        RowInfo& rThisRowInfo = pRowInfo[ nCellInfoY ];

        for( size_t nCol = 0; nCol < nColCount; ++nCol )
        {
            sal_uInt16 nCellInfoX = static_cast< sal_uInt16 >( nCol + nX1 );
            const CellInfo& rInfo = rThisRowInfo.pCellInfo[ nCellInfoX ];

            const SvxBoxItem* pBox = rInfo.pLinesAttr;
            const SvxLineItem* pTLBR = rInfo.mpTLBRLine;
            const SvxLineItem* pBLTR = rInfo.mpBLTRLine;

            size_t nFirstCol = nCol;
            size_t nFirstRow = nRow;

            // *** merged cells *** -------------------------------------------

            if( !rArray.IsMerged( nCol, nRow ) && (rInfo.bMerged || rInfo.bHOverlapped || rInfo.bVOverlapped) )
            {
                // *** insert merged range in svx::frame::Array ***

                /*  #i69369# top-left cell of a merged range may be located in
                    a hidden column or row. Use lcl_GetMergeRange() to find the
                    complete merged range, then calculate dimensions and
                    document position of the visible range. */

                // note: document columns are always one less than CellInfoX coords
                // note: document rows must be looked up in RowInfo structs

                // current column and row in document coordinates
                SCCOL nCurrDocCol = static_cast< SCCOL >( nCellInfoX - 1 );
                SCROW nCurrDocRow = static_cast< SCROW >( (nCellInfoY > 0) ? rThisRowInfo.nRowNo : (nY1 - 1) );

                // find entire merged range in document, returns signed document coordinates
                SCsCOL nFirstRealDocColS, nLastRealDocColS;
                SCsROW nFirstRealDocRowS, nLastRealDocRowS;
                lcl_GetMergeRange( static_cast< SCsCOL >( nCurrDocCol ), static_cast< SCsROW >( nCurrDocRow ),
                    nCellInfoY, this, pRowInfo, nX1,nY1,nX2,nY2,nTab,
                    nFirstRealDocColS, nFirstRealDocRowS, nLastRealDocColS, nLastRealDocRowS );

                // *complete* merged range in document coordinates
                SCCOL nFirstRealDocCol = static_cast< SCCOL >( nFirstRealDocColS );
                SCROW nFirstRealDocRow = static_cast< SCROW >( nFirstRealDocRowS );
                SCCOL nLastRealDocCol  = static_cast< SCCOL >( nLastRealDocColS );
                SCROW nLastRealDocRow  = static_cast< SCROW >( nLastRealDocRowS );

                // first visible column (nX1-1 is first processed document column)
                SCCOL nFirstDocCol = (nX1 > 0) ? ::std::max< SCCOL >( nFirstRealDocCol, nX1 - 1 ) : nFirstRealDocCol;
                sal_uInt16 nFirstCellInfoX = static_cast< sal_uInt16 >( nFirstDocCol + 1 );
                nFirstCol = static_cast< size_t >( nFirstCellInfoX - nX1 );

                // last visible column (nX2+1 is last processed document column)
                SCCOL nLastDocCol = (nX2 < MAXCOL) ? ::std::min< SCCOL >( nLastRealDocCol, nX2 + 1 ) : nLastRealDocCol;
                sal_uInt16 nLastCellInfoX = static_cast< sal_uInt16 >( nLastDocCol + 1 );
                size_t nLastCol = static_cast< size_t >( nLastCellInfoX - nX1 );

                // first visible row
                sal_uInt16 nFirstCellInfoY = nCellInfoY;
                while( ((nFirstCellInfoY > 1) && (pRowInfo[ nFirstCellInfoY - 1 ].nRowNo >= nFirstRealDocRow)) ||
                       ((nFirstCellInfoY == 1) && (static_cast< SCROW >( nY1 - 1 ) >= nFirstRealDocRow)) )
                    --nFirstCellInfoY;
                SCROW nFirstDocRow = (nFirstCellInfoY > 0) ? pRowInfo[ nFirstCellInfoY ].nRowNo : static_cast< SCROW >( nY1 - 1 );
                nFirstRow = static_cast< size_t >( nFirstCellInfoY );

                // last visible row
                sal_uInt16 nLastCellInfoY = nCellInfoY;
                while( (sal::static_int_cast<SCSIZE>(nLastCellInfoY + 1) < nArrCount) &&
                            (pRowInfo[ nLastCellInfoY + 1 ].nRowNo <= nLastRealDocRow) )
                    ++nLastCellInfoY;
                SCROW nLastDocRow = (nLastCellInfoY > 0) ? pRowInfo[ nLastCellInfoY ].nRowNo : static_cast< SCROW >( nY1 - 1 );
                size_t nLastRow = static_cast< size_t >( nLastCellInfoY );

                // insert merged range
                rArray.SetMergedRange( nFirstCol, nFirstRow, nLastCol, nLastRow );

                // *** find additional size not included in svx::frame::Array ***

                // additional space before first column
                if( nFirstCol == 0 )
                {
                    long nSize = 0;
                    for( SCCOL nDocCol = nFirstRealDocCol; nDocCol < nFirstDocCol; ++nDocCol )
                        nSize += std::max( static_cast< long >( GetColWidth( nDocCol, nTab ) * nScaleX ), 1L );
                    rArray.SetAddMergedLeftSize( nCol, nRow, nSize );
                }
                // additional space after last column
                if( nLastCol + 1 == nColCount )
                {
                    long nSize = 0;
                    for( SCCOL nDocCol = nLastDocCol + 1; nDocCol <= nLastRealDocCol; ++nDocCol )
                        nSize += std::max( static_cast< long >( GetColWidth( nDocCol, nTab ) * nScaleX ), 1L );
                    rArray.SetAddMergedRightSize( nCol, nRow, nSize );
                }
                // additional space above first row
                if( nFirstRow == 0 )
                {
                    long nSize = 0;
                    for( SCROW nDocRow = nFirstRealDocRow; nDocRow < nFirstDocRow; ++nDocRow )
                        nSize += std::max( static_cast< long >( GetRowHeight( nDocRow, nTab ) * nScaleY ), 1L );
                    rArray.SetAddMergedTopSize( nCol, nRow, nSize );
                }
                // additional space beyond last row
                if( nLastRow + 1 == nRowCount )
                {
                    long nSize = 0;
                    for( SCROW nDocRow = nLastDocRow + 1; nDocRow <= nLastRealDocRow; ++nDocRow )
                        nSize += std::max( static_cast< long >( GetRowHeight( nDocRow, nTab ) * nScaleY ), 1L );
                    rArray.SetAddMergedBottomSize( nCol, nRow, nSize );
                }

                // *** use line attributes from real origin cell ***

                if( (nFirstRealDocCol != nCurrDocCol) || (nFirstRealDocRow != nCurrDocRow) )
                {
                    if( const ScPatternAttr* pPattern = GetPattern( nFirstRealDocCol, nFirstRealDocRow, nTab ) )
                    {
                        const SfxItemSet* pCond = GetCondResult( nFirstRealDocCol, nFirstRealDocRow, nTab );
                        pBox = static_cast< const SvxBoxItem* >( &pPattern->GetItem( ATTR_BORDER, pCond ) );
                        pTLBR = static_cast< const SvxLineItem* >( &pPattern->GetItem( ATTR_BORDER_TLBR, pCond ) );
                        pBLTR = static_cast< const SvxLineItem* >( &pPattern->GetItem( ATTR_BORDER_BLTR, pCond ) );
                    }
                    else
                    {
                        pBox = 0;
                        pTLBR = pBLTR = 0;
                    }
                }
            }

            // *** borders *** ------------------------------------------------

            if( pBox )
            {
                rArray.SetCellStyleLeft(   nFirstCol, nFirstRow, svx::frame::Style( pBox->GetLeft(),   nScaleX ) );
                rArray.SetCellStyleRight(  nFirstCol, nFirstRow, svx::frame::Style( pBox->GetRight(),  nScaleX ) );
                rArray.SetCellStyleTop(    nFirstCol, nFirstRow, svx::frame::Style( pBox->GetTop(),    nScaleY ) );
                rArray.SetCellStyleBottom( nFirstCol, nFirstRow, svx::frame::Style( pBox->GetBottom(), nScaleY ) );
            }

            if( pTLBR )
                rArray.SetCellStyleTLBR( nFirstCol, nFirstRow, svx::frame::Style( pTLBR->GetLine(), nScaleY ) );
            if( rInfo.mpBLTRLine )
                rArray.SetCellStyleBLTR( nFirstCol, nFirstRow, svx::frame::Style( pBLTR->GetLine(), nScaleY ) );
        }
    }

    /*  Mirror the entire frame array.
        1st param = Mirror the vertical double line styles as well.
        2nd param = Do not swap diagonal lines.
     */
    if( bLayoutRTL )
        rArray.MirrorSelfX( true, false );
}

// ============================================================================

ScTableInfo::ScTableInfo() :
    mpRowInfo( new RowInfo[ ROWINFO_MAX ] ),
    mbPageMode( false )
{
    for( sal_uInt16 nIdx = 0; nIdx < ROWINFO_MAX; ++nIdx )
        mpRowInfo[ nIdx ].pCellInfo = 0;
}

ScTableInfo::~ScTableInfo()
{
    for( sal_uInt16 nIdx = 0; nIdx < ROWINFO_MAX; ++nIdx )
        delete [] mpRowInfo[ nIdx ].pCellInfo;
    delete [] mpRowInfo;
}

// ============================================================================

