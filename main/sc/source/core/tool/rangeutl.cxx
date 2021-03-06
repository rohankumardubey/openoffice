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

#include <tools/debug.hxx>

#include "rangeutl.hxx"
#include "document.hxx"
#include "global.hxx"
#include "dbcolect.hxx"
#include "rangenam.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "convuno.hxx"
#include "externalrefmgr.hxx"
#include "compiler.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::formula::FormulaGrammar;
using namespace ::com::sun::star;

//------------------------------------------------------------------------

sal_Bool ScRangeUtil::MakeArea( const String&	rAreaStr,
							ScArea&			rArea,
							ScDocument*		pDoc,
							SCTAB			nTab,
							ScAddress::Details const & rDetails ) const
{
	// Eingabe in rAreaStr: "$Tabelle1.$A1:$D17"

	// BROKEN BROKEN BROKEN
	// but it is only used in the consolidate dialog.  Ignore for now.

	sal_Bool		nSuccess	= sal_False;
	sal_uInt16		nPointPos	= rAreaStr.Search('.');
	sal_uInt16		nColonPos	= rAreaStr.Search(':');
	String		aStrArea( rAreaStr );
	ScRefAddress	startPos;
	ScRefAddress	endPos;

	if ( nColonPos == STRING_NOTFOUND )
		if ( nPointPos != STRING_NOTFOUND )
		{
			aStrArea += ':';
			aStrArea += rAreaStr.Copy( nPointPos+1 ); // '.' nicht mitkopieren
		}

	nSuccess = ConvertDoubleRef( pDoc, aStrArea, nTab, startPos, endPos, rDetails );

	if ( nSuccess )
		rArea = ScArea( startPos.Tab(),
						startPos.Col(),	startPos.Row(),
						endPos.Col(),	endPos.Row() );

	return nSuccess;
}

//------------------------------------------------------------------------

void ScRangeUtil::CutPosString( const String&	theAreaStr,
								String&			thePosStr ) const
{
	String	aPosStr;
	// BROKEN BROKEN BROKEN
	// but it is only used in the consolidate dialog.  Ignore for now.

	sal_uInt16	nColonPos = theAreaStr.Search(':');

	if ( nColonPos != STRING_NOTFOUND )
		aPosStr = theAreaStr.Copy( 0, nColonPos ); // ':' nicht mitkopieren
	else
		aPosStr = theAreaStr;

	thePosStr = aPosStr;
}

//------------------------------------------------------------------------

sal_Bool ScRangeUtil::IsAbsTabArea( const String& 	rAreaStr,
								ScDocument*		pDoc,
								ScArea***		pppAreas,
								sal_uInt16*			pAreaCount,
                                sal_Bool            /* bAcceptCellRef */,
								ScAddress::Details const & rDetails ) const
{
	DBG_ASSERT( pDoc, "Kein Dokument uebergeben!" );
	if ( !pDoc )
		return sal_False;

	// BROKEN BROKEN BROKEN
	// but it is only used in the consolidate dialog.  Ignore for now.

	/*
	 * Erwartet wird ein String der Form
	 *		"$Tabelle1.$A$1:$Tabelle3.$D$17"
	 * Wenn bAcceptCellRef == sal_True ist, wird auch ein String der Form
	 *		"$Tabelle1.$A$1"
	 * akzeptiert.
	 *
	 * als Ergebnis wird ein ScArea-Array angelegt,
	 * welches ueber ppAreas bekannt gegeben wird und auch
	 * wieder geloescht werden muss!
	 */

	sal_Bool	bStrOk = sal_False;
	String	aTempAreaStr(rAreaStr);
	String	aStartPosStr;
	String	aEndPosStr;

	if ( STRING_NOTFOUND == aTempAreaStr.Search(':') )
	{
		aTempAreaStr.Append(':');
		aTempAreaStr.Append(rAreaStr);
	}

	sal_uInt16	 nColonPos = aTempAreaStr.Search(':');

	if (   STRING_NOTFOUND != nColonPos
		&& STRING_NOTFOUND != aTempAreaStr.Search('.') )
	{
		ScRefAddress	aStartPos;
		ScRefAddress	aEndPos;

		aStartPosStr = aTempAreaStr.Copy( 0,		   nColonPos  );
		aEndPosStr	 = aTempAreaStr.Copy( nColonPos+1, STRING_LEN );

		if ( ConvertSingleRef( pDoc, aStartPosStr, 0, aStartPos, rDetails ) )
		{
			if ( ConvertSingleRef( pDoc, aEndPosStr, aStartPos.Tab(), aEndPos, rDetails ) )
			{
				aStartPos.SetRelCol( sal_False );
				aStartPos.SetRelRow( sal_False );
				aStartPos.SetRelTab( sal_False );
				aEndPos.SetRelCol( sal_False );
				aEndPos.SetRelRow( sal_False );
				aEndPos.SetRelTab( sal_False );

				bStrOk = sal_True;

				if ( pppAreas && pAreaCount ) // Array zurueckgegeben?
				{
					SCTAB		nStartTab	= aStartPos.Tab();
					SCTAB		nEndTab		= aEndPos.Tab();
					sal_uInt16		nTabCount	= static_cast<sal_uInt16>(nEndTab-nStartTab+1);
					ScArea** 	theAreas	= new ScArea*[nTabCount];
					SCTAB		nTab		= 0;
					sal_uInt16		i			= 0;
					ScArea		theArea( 0, aStartPos.Col(), aStartPos.Row(),
											aEndPos.Col(), aEndPos.Row() );

					nTab = nStartTab;
					for ( i=0; i<nTabCount; i++ )
					{
						theAreas[i] = new ScArea( theArea );
						theAreas[i]->nTab = nTab;
						nTab++;
					}
					*pppAreas   = theAreas;
					*pAreaCount = nTabCount;
				}
			}
		}
	}

	return bStrOk;
}

//------------------------------------------------------------------------

sal_Bool ScRangeUtil::IsAbsArea( const String&	rAreaStr,
							 ScDocument*	pDoc,
							 SCTAB			nTab,
							 String*		pCompleteStr,
							 ScRefAddress*	pStartPos,
							 ScRefAddress*	pEndPos,
							 ScAddress::Details const & rDetails ) const
{
	sal_Bool		bIsAbsArea = sal_False;
	ScRefAddress	startPos;
	ScRefAddress	endPos;

	bIsAbsArea = ConvertDoubleRef( pDoc, rAreaStr, nTab, startPos, endPos, rDetails );

	if ( bIsAbsArea )
	{
		startPos.SetRelCol( sal_False );
		startPos.SetRelRow( sal_False );
		startPos.SetRelTab( sal_False );
		endPos  .SetRelCol( sal_False );
		endPos  .SetRelRow( sal_False );
		endPos  .SetRelTab( sal_False );

		if ( pCompleteStr )
		{
			*pCompleteStr  = startPos.GetRefString( pDoc, MAXTAB+1, rDetails );
			*pCompleteStr += ':';
			*pCompleteStr += endPos  .GetRefString( pDoc, nTab, rDetails );
		}

		if ( pStartPos && pEndPos )
		{
			*pStartPos = startPos;
			*pEndPos   = endPos;
		}
	}

	return bIsAbsArea;
}

//------------------------------------------------------------------------

sal_Bool ScRangeUtil::IsAbsPos( const String&	rPosStr,
							ScDocument*		pDoc,
							SCTAB			nTab,
							String*			pCompleteStr,
							ScRefAddress*	pPosTripel,
							ScAddress::Details const & rDetails ) const
{
	sal_Bool		bIsAbsPos = sal_False;
	ScRefAddress	thePos;

	bIsAbsPos = ConvertSingleRef( pDoc, rPosStr, nTab, thePos, rDetails ); 
	thePos.SetRelCol( sal_False );
	thePos.SetRelRow( sal_False );
	thePos.SetRelTab( sal_False );

	if ( bIsAbsPos )
	{
		if ( pPosTripel )
			*pPosTripel = thePos;
		if ( pCompleteStr )
			*pCompleteStr = thePos.GetRefString( pDoc, MAXTAB+1, rDetails );
	}

	return bIsAbsPos;
}

//------------------------------------------------------------------------

sal_Bool ScRangeUtil::MakeRangeFromName	(
	const String&	rName,
	ScDocument*		pDoc,
	SCTAB			nCurTab,
	ScRange&		rRange,
	RutlNameScope 	eScope,
	ScAddress::Details const & rDetails ) const
{
	sal_Bool bResult=sal_False;
	ScRangeUtil		aRangeUtil;
    SCTAB nTab = 0;
    SCCOL nColStart = 0;
    SCCOL nColEnd = 0;
    SCROW nRowStart = 0;
    SCROW nRowEnd = 0;

	if( eScope==RUTL_NAMES )
	{
		ScRangeName& rRangeNames = *(pDoc->GetRangeName());
		sal_uInt16		 nAt		 = 0;

		if ( rRangeNames.SearchName( rName, nAt ) )
		{
			ScRangeData* pData = rRangeNames[nAt];
			String		 aStrArea;
			ScRefAddress	 aStartPos;
			ScRefAddress	 aEndPos;

			pData->GetSymbol( aStrArea );

			if ( IsAbsArea( aStrArea, pDoc, nCurTab,
							NULL, &aStartPos, &aEndPos, rDetails ) )
			{
				nTab	   = aStartPos.Tab();
				nColStart  = aStartPos.Col();
				nRowStart  = aStartPos.Row();
				nColEnd    = aEndPos.Col();
				nRowEnd    = aEndPos.Row();
				bResult	   = sal_True;
			}
			else
			{
				CutPosString( aStrArea, aStrArea );

				if ( IsAbsPos( aStrArea, pDoc, nCurTab,
										  NULL, &aStartPos, rDetails ) )
				{
					nTab	   = aStartPos.Tab();
					nColStart  = nColEnd = aStartPos.Col();
					nRowStart  = nRowEnd = aStartPos.Row();
					bResult	   = sal_True;
				}
			}
		}
	}
	else if( eScope==RUTL_DBASE )
	{
		ScDBCollection&	rDbNames = *(pDoc->GetDBCollection());
		sal_uInt16		 	nAt = 0;

		if ( rDbNames.SearchName( rName, nAt ) )
		{
			ScDBData* pData = rDbNames[nAt];

			pData->GetArea( nTab, nColStart, nRowStart,
								  nColEnd,	 nRowEnd );
			bResult = sal_True;
		}
	}
	else
	{
		DBG_ERROR( "ScRangeUtil::MakeRangeFromName" );
	}

	if( bResult )
	{
		rRange = ScRange( nColStart, nRowStart, nTab, nColEnd, nRowEnd, nTab );
	}

	return bResult;
}

//========================================================================

void ScRangeStringConverter::AssignString(
		OUString& rString,
		const OUString& rNewStr,
        sal_Bool bAppendStr,
        sal_Unicode cSeperator)
{
	if( bAppendStr )
	{
		if( rNewStr.getLength() )
		{
			if( rString.getLength() )
                rString += rtl::OUString(cSeperator);
			rString += rNewStr;
		}
	}
	else
		rString = rNewStr;
}

sal_Int32 ScRangeStringConverter::IndexOf(
		const OUString& rString,
		sal_Unicode cSearchChar,
		sal_Int32 nOffset,
		sal_Unicode cQuote )
{
	sal_Int32		nLength		= rString.getLength();
	sal_Int32		nIndex		= nOffset;
	sal_Bool		bQuoted		= sal_False;
	sal_Bool		bExitLoop	= sal_False;

	while( !bExitLoop && (nIndex < nLength) )
	{
		sal_Unicode cCode = rString[ nIndex ];
		bExitLoop = (cCode == cSearchChar) && !bQuoted;
		bQuoted = (bQuoted != (cCode == cQuote));
		if( !bExitLoop )
			nIndex++;
	}
	return (nIndex < nLength) ? nIndex : -1;
}

sal_Int32 ScRangeStringConverter::IndexOfDifferent(
		const OUString& rString,
		sal_Unicode cSearchChar,
		sal_Int32 nOffset )
{
	sal_Int32		nLength		= rString.getLength();
	sal_Int32		nIndex		= nOffset;
	sal_Bool		bExitLoop	= sal_False;

	while( !bExitLoop && (nIndex < nLength) )
	{
		bExitLoop = (rString[ nIndex ] != cSearchChar);
		if( !bExitLoop )
			nIndex++;
	}
	return (nIndex < nLength) ? nIndex : -1;
}

void ScRangeStringConverter::GetTokenByOffset(
		OUString& rToken,
		const OUString& rString,
		sal_Int32& nOffset,
        sal_Unicode cSeperator,
		sal_Unicode cQuote)
{
	sal_Int32 nLength = rString.getLength();
	if( nOffset >= nLength )
	{
		rToken = OUString();
		nOffset = -1;
	}
	else
	{
		sal_Int32 nTokenEnd = IndexOf( rString, cSeperator, nOffset, cQuote );
		if( nTokenEnd < 0 )
			nTokenEnd = nLength;
		rToken = rString.copy( nOffset, nTokenEnd - nOffset );

		sal_Int32 nNextBegin = IndexOfDifferent( rString, cSeperator, nTokenEnd );
		nOffset = (nNextBegin < 0) ? nLength : nNextBegin;
	}
}

void ScRangeStringConverter::AppendTableName(OUStringBuffer& rBuf, const OUString& rTabName, sal_Unicode /* cQuote */)
{
    // quote character is always "'"
    String aQuotedTab(rTabName);
    ScCompiler::CheckTabQuotes(aQuotedTab, ::formula::FormulaGrammar::CONV_OOO);
    rBuf.append(aQuotedTab);
}

sal_Int32 ScRangeStringConverter::GetTokenCount( const OUString& rString, sal_Unicode cSeperator, sal_Unicode cQuote )
{
	OUString	sToken;
	sal_Int32	nCount = 0;
	sal_Int32	nOffset = 0;
	while( nOffset >= 0 )
	{
		GetTokenByOffset( sToken, rString, nOffset, cQuote, cSeperator );
		if( nOffset >= 0 )
			nCount++;
	}
	return nCount;
}

//___________________________________________________________________

sal_Bool ScRangeStringConverter::GetAddressFromString(
		ScAddress& rAddress,
		const OUString& rAddressStr,
		const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
		sal_Int32& nOffset,
        sal_Unicode cSeperator,
        sal_Unicode cQuote )
{
	OUString sToken;
	GetTokenByOffset( sToken, rAddressStr, nOffset, cSeperator, cQuote );
	if( nOffset >= 0 )
    {
        if ((rAddress.Parse( sToken, const_cast<ScDocument*>(pDocument), eConv ) & SCA_VALID) == SCA_VALID)
            return true;
    }
	return sal_False;
}

sal_Bool ScRangeStringConverter::GetRangeFromString(
		ScRange& rRange,
		const OUString& rRangeStr,
		const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
		sal_Int32& nOffset,
        sal_Unicode cSeperator,
        sal_Unicode cQuote )
{
	OUString sToken;
	sal_Bool bResult(sal_False);
	GetTokenByOffset( sToken, rRangeStr, nOffset, cSeperator, cQuote );
	if( nOffset >= 0 )
	{
        sal_Int32 nIndex = IndexOf( sToken, ':', 0, cQuote );
        String aUIString(sToken);

        if( nIndex < 0 )
        {
            if ( aUIString.GetChar(0) == (sal_Unicode) '.' )
                aUIString.Erase( 0, 1 );
            bResult = ((rRange.aStart.Parse( aUIString, const_cast<ScDocument*> (pDocument), eConv) & SCA_VALID) == SCA_VALID);
            rRange.aEnd = rRange.aStart;
        }
        else
        {
            if ( aUIString.GetChar(0) == (sal_Unicode) '.' )
            {
                aUIString.Erase( 0, 1 );
                --nIndex;
            }

            if ( nIndex < aUIString.Len() - 1 &&
                    aUIString.GetChar((xub_StrLen)nIndex + 1) == (sal_Unicode) '.' )
                aUIString.Erase( (xub_StrLen)nIndex + 1, 1 );

            bResult = ((rRange.Parse(aUIString, const_cast<ScDocument*> (pDocument), eConv) & SCA_VALID) == SCA_VALID);

            // #i77703# chart ranges in the file format contain both sheet names, even for an external reference sheet.
            // This isn't parsed by ScRange, so try to parse the two Addresses then.
            if (!bResult)
            {    
                bResult = ((rRange.aStart.Parse( aUIString.Copy(0, (xub_StrLen)nIndex), const_cast<ScDocument*>(pDocument),
                                eConv) & SCA_VALID) == SCA_VALID) &&
                          ((rRange.aEnd.Parse( aUIString.Copy((xub_StrLen)nIndex+1), const_cast<ScDocument*>(pDocument),
                                eConv) & SCA_VALID) == SCA_VALID);
            }
        }
    }
	return bResult;
}

sal_Bool ScRangeStringConverter::GetRangeListFromString(
		ScRangeList& rRangeList,
		const OUString& rRangeListStr,
		const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Unicode cSeperator,
        sal_Unicode cQuote )
{
    sal_Bool bRet = sal_True;
	DBG_ASSERT( rRangeListStr.getLength(), "ScXMLConverter::GetRangeListFromString - empty string!" );
	sal_Int32 nOffset = 0;
	while( nOffset >= 0 )
	{
		ScRange* pRange = new ScRange;
		if( GetRangeFromString( *pRange, rRangeListStr, pDocument, eConv, nOffset, cSeperator, cQuote ) && (nOffset >= 0) )
			rRangeList.Insert( pRange, LIST_APPEND );
        else if (nOffset > -1)
            bRet = sal_False;
	}
    return bRet;
}


//___________________________________________________________________

sal_Bool ScRangeStringConverter::GetAreaFromString(
		ScArea& rArea,
		const OUString& rRangeStr,
		const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
		sal_Int32& nOffset,
        sal_Unicode cSeperator,
        sal_Unicode cQuote )
{
	ScRange aScRange;
	sal_Bool bResult(sal_False);
	if( GetRangeFromString( aScRange, rRangeStr, pDocument, eConv, nOffset, cSeperator, cQuote ) && (nOffset >= 0) )
	{
		rArea.nTab = aScRange.aStart.Tab();
		rArea.nColStart = aScRange.aStart.Col();
		rArea.nRowStart = aScRange.aStart.Row();
		rArea.nColEnd = aScRange.aEnd.Col();
		rArea.nRowEnd = aScRange.aEnd.Row();
		bResult = sal_True;
	}
	return bResult;
}


//___________________________________________________________________

sal_Bool ScRangeStringConverter::GetAddressFromString(
		table::CellAddress& rAddress,
		const OUString& rAddressStr,
		const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
		sal_Int32& nOffset,
        sal_Unicode cSeperator,
        sal_Unicode cQuote )
{
	ScAddress aScAddress;
	sal_Bool bResult(sal_False);
	if( GetAddressFromString( aScAddress, rAddressStr, pDocument, eConv, nOffset, cSeperator, cQuote ) && (nOffset >= 0) )
	{
		ScUnoConversion::FillApiAddress( rAddress, aScAddress );
		bResult = sal_True;
	}
	return bResult;
}

sal_Bool ScRangeStringConverter::GetRangeFromString(
		table::CellRangeAddress& rRange,
		const OUString& rRangeStr,
		const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
		sal_Int32& nOffset,
        sal_Unicode cSeperator,
        sal_Unicode cQuote )
{
	ScRange aScRange;
	sal_Bool bResult(sal_False);
	if( GetRangeFromString( aScRange, rRangeStr, pDocument, eConv, nOffset, cSeperator, cQuote ) && (nOffset >= 0) )
	{
		ScUnoConversion::FillApiRange( rRange, aScRange );
		bResult = sal_True;
	}
	return bResult;
}

sal_Bool ScRangeStringConverter::GetRangeListFromString(
		uno::Sequence< table::CellRangeAddress >& rRangeSeq,
		const OUString& rRangeListStr,
		const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Unicode cSeperator,
        sal_Unicode cQuote )
{
    sal_Bool bRet = sal_True;
	DBG_ASSERT( rRangeListStr.getLength(), "ScXMLConverter::GetRangeListFromString - empty string!" );
	table::CellRangeAddress aRange;
	sal_Int32 nOffset = 0;
	while( nOffset >= 0 )
	{
		if( GetRangeFromString( aRange, rRangeListStr, pDocument, eConv, nOffset, cSeperator, cQuote ) && (nOffset >= 0) )
		{
			rRangeSeq.realloc( rRangeSeq.getLength() + 1 );
			rRangeSeq[ rRangeSeq.getLength() - 1 ] = aRange;
		}
        else
            bRet = sal_False;
	}
    return bRet;
}


//___________________________________________________________________

void ScRangeStringConverter::GetStringFromAddress(
		OUString& rString,
		const ScAddress& rAddress,
		const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Unicode cSeperator,
		sal_Bool bAppendStr,
		sal_uInt16 nFormatFlags )
{
	if (pDocument && pDocument->HasTable(rAddress.Tab()))
	{
		String sAddress;
		rAddress.Format( sAddress, nFormatFlags, (ScDocument*) pDocument, eConv );
		AssignString( rString, sAddress, bAppendStr, cSeperator );
	}
}

void ScRangeStringConverter::GetStringFromRange(
		OUString& rString,
		const ScRange& rRange,
		const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Unicode cSeperator,
		sal_Bool bAppendStr,
		sal_uInt16 nFormatFlags )
{
	if (pDocument && pDocument->HasTable(rRange.aStart.Tab()))
	{
		ScAddress aStartAddress( rRange.aStart );
		ScAddress aEndAddress( rRange.aEnd );
		String sStartAddress;
		String sEndAddress;
		aStartAddress.Format( sStartAddress, nFormatFlags, (ScDocument*) pDocument, eConv );
		aEndAddress.Format( sEndAddress, nFormatFlags, (ScDocument*) pDocument, eConv );
		OUString sOUStartAddress( sStartAddress );
        sOUStartAddress += OUString(':');
		sOUStartAddress += OUString( sEndAddress );
		AssignString( rString, sOUStartAddress, bAppendStr, cSeperator );
	}
}

void ScRangeStringConverter::GetStringFromRangeList(
		OUString& rString,
		const ScRangeList* pRangeList,
		const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Unicode cSeperator,
		sal_uInt16 nFormatFlags )
{
	OUString sRangeListStr;
	if( pRangeList )
	{
		sal_Int32 nCount = pRangeList->Count();
		for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
		{
			const ScRange* pRange = pRangeList->GetObject( nIndex );
			if( pRange )
				GetStringFromRange( sRangeListStr, *pRange, pDocument, eConv, cSeperator, sal_True, nFormatFlags );
		}
	}
	rString = sRangeListStr;
}


//___________________________________________________________________

void ScRangeStringConverter::GetStringFromArea(
		OUString& rString,
		const ScArea& rArea,
		const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Unicode cSeperator,
		sal_Bool bAppendStr,
		sal_uInt16 nFormatFlags )
{
	ScRange aRange( rArea.nColStart, rArea.nRowStart, rArea.nTab, rArea.nColEnd, rArea.nRowEnd, rArea.nTab );
	GetStringFromRange( rString, aRange, pDocument, eConv, cSeperator, bAppendStr, nFormatFlags );
}


//___________________________________________________________________

void ScRangeStringConverter::GetStringFromAddress(
		OUString& rString,
		const table::CellAddress& rAddress,
		const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Unicode cSeperator,
		sal_Bool bAppendStr,
		sal_uInt16 nFormatFlags )
{
	ScAddress aScAddress( static_cast<SCCOL>(rAddress.Column), static_cast<SCROW>(rAddress.Row), rAddress.Sheet );
	GetStringFromAddress( rString, aScAddress, pDocument, eConv, cSeperator, bAppendStr, nFormatFlags );
}

void ScRangeStringConverter::GetStringFromRange(
		OUString& rString,
		const table::CellRangeAddress& rRange,
		const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Unicode cSeperator,
		sal_Bool bAppendStr,
		sal_uInt16 nFormatFlags )
{
	ScRange aScRange( static_cast<SCCOL>(rRange.StartColumn), static_cast<SCROW>(rRange.StartRow), rRange.Sheet,
		static_cast<SCCOL>(rRange.EndColumn), static_cast<SCROW>(rRange.EndRow), rRange.Sheet );
	GetStringFromRange( rString, aScRange, pDocument, eConv, cSeperator, bAppendStr, nFormatFlags );
}

void ScRangeStringConverter::GetStringFromRangeList(
		OUString& rString,
		const uno::Sequence< table::CellRangeAddress >& rRangeSeq,
		const ScDocument* pDocument,
        FormulaGrammar::AddressConvention eConv,
        sal_Unicode cSeperator,
		sal_uInt16 nFormatFlags )
{
	OUString sRangeListStr;
	sal_Int32 nCount = rRangeSeq.getLength();
	for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
	{
		const table::CellRangeAddress& rRange = rRangeSeq[ nIndex ];
		GetStringFromRange( sRangeListStr, rRange, pDocument, eConv, cSeperator, sal_True, nFormatFlags );
	}
	rString = sRangeListStr;
}

static void lcl_appendCellAddress(
    rtl::OUStringBuffer& rBuf, ScDocument* pDoc, const ScAddress& rCell, 
    const ScAddress::ExternalInfo& rExtInfo)
{
    if (rExtInfo.mbExternal)
    {
        ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
        const String* pFilePath = pRefMgr->getExternalFileName(rExtInfo.mnFileId, true);
        if (!pFilePath)
            return;

        sal_Unicode cQuote = '\'';
        rBuf.append(cQuote);
        rBuf.append(*pFilePath);
        rBuf.append(cQuote);
        rBuf.append(sal_Unicode('#'));
        rBuf.append(sal_Unicode('$'));
        ScRangeStringConverter::AppendTableName(rBuf, rExtInfo.maTabName);
        rBuf.append(sal_Unicode('.'));

        String aAddr;
        rCell.Format(aAddr, SCA_ABS, NULL, ::formula::FormulaGrammar::CONV_OOO);
        rBuf.append(aAddr);
    }
    else
    {
        String aAddr;
        rCell.Format(aAddr, SCA_ABS_3D, pDoc, ::formula::FormulaGrammar::CONV_OOO);
        rBuf.append(aAddr);
    }
}

static void lcl_appendCellRangeAddress(
    rtl::OUStringBuffer& rBuf, ScDocument* pDoc, const ScAddress& rCell1, const ScAddress& rCell2,
    const ScAddress::ExternalInfo& rExtInfo1, const ScAddress::ExternalInfo& rExtInfo2)
{
    if (rExtInfo1.mbExternal)
    {
        DBG_ASSERT(rExtInfo2.mbExternal, "2nd address is not external!?");
        DBG_ASSERT(rExtInfo1.mnFileId == rExtInfo2.mnFileId, "File IDs do not match between 1st and 2nd addresses.");

        ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
        const String* pFilePath = pRefMgr->getExternalFileName(rExtInfo1.mnFileId, true);
        if (!pFilePath)
            return;

        sal_Unicode cQuote = '\'';
        rBuf.append(cQuote);
        rBuf.append(*pFilePath);
        rBuf.append(cQuote);
        rBuf.append(sal_Unicode('#'));
        rBuf.append(sal_Unicode('$'));
        ScRangeStringConverter::AppendTableName(rBuf, rExtInfo1.maTabName);
        rBuf.append(sal_Unicode('.'));

        String aAddr;
        rCell1.Format(aAddr, SCA_ABS, NULL, ::formula::FormulaGrammar::CONV_OOO);
        rBuf.append(aAddr);

        rBuf.appendAscii(":");

        if (rExtInfo1.maTabName != rExtInfo2.maTabName)
        {
            rBuf.append(sal_Unicode('$'));
            ScRangeStringConverter::AppendTableName(rBuf, rExtInfo2.maTabName);
            rBuf.append(sal_Unicode('.'));
        }

        rCell2.Format(aAddr, SCA_ABS, NULL, ::formula::FormulaGrammar::CONV_OOO);
        rBuf.append(aAddr);
    }
    else
    {
        ScRange aRange;
        aRange.aStart = rCell1;
        aRange.aEnd   = rCell2;
        String aAddr;
        aRange.Format(aAddr, SCR_ABS_3D, pDoc, ::formula::FormulaGrammar::CONV_OOO);
        rBuf.append(aAddr);
    }
}

void ScRangeStringConverter::GetStringFromXMLRangeString( OUString& rString, const OUString& rXMLRange, ScDocument* pDoc )
{
    const sal_Unicode cSep = ' ';
    const sal_Unicode cQuote = '\'';

    OUStringBuffer aRetStr;
    sal_Int32 nOffset = 0;
    bool bFirst = true;

    while (nOffset >= 0)
    {
        OUString aToken;
        GetTokenByOffset(aToken, rXMLRange, nOffset, cSep, cQuote);
        if (nOffset < 0)
            break;

        sal_Int32 nSepPos = IndexOf(aToken, ':', 0, cQuote);
        if (nSepPos >= 0)
        {
            // Cell range
            OUString aBeginCell = aToken.copy(0, nSepPos);
            OUString aEndCell   = aToken.copy(nSepPos+1);

            if (!aBeginCell.getLength() || !aEndCell.getLength())
                // both cell addresses must exist for this to work.
                continue;

            sal_Int32 nEndCellDotPos = aEndCell.indexOf('.');
            if (nEndCellDotPos <= 0)
            {
                // initialize buffer with table name...
                sal_Int32 nDotPos = IndexOf(aBeginCell, sal_Unicode('.'), 0, cQuote);
                OUStringBuffer aBuf = aBeginCell.copy(0, nDotPos);

                if (nEndCellDotPos == 0)
                {    
                    // workaround for old syntax (probably pre-chart2 age?)
                    // e.g. Sheet1.A1:.B2
                    aBuf.append(aEndCell);
                }
                else if (nEndCellDotPos < 0)
                {
                    // sheet name in the end cell is omitted (e.g. Sheet2.A1:B2).
                    aBuf.append(sal_Unicode('.'));
                    aBuf.append(aEndCell);
                }
                aEndCell = aBuf.makeStringAndClear();
            }
                
            ScAddress::ExternalInfo aExtInfo1, aExtInfo2;
            ScAddress aCell1, aCell2;
            rtl::OUString aBuf;
            sal_uInt16 nRet = aCell1.Parse(aBeginCell, pDoc, FormulaGrammar::CONV_OOO, &aExtInfo1);
            if ((nRet & SCA_VALID) != SCA_VALID)
                // first cell is invalid.
                continue;

            nRet = aCell2.Parse(aEndCell, pDoc, FormulaGrammar::CONV_OOO, &aExtInfo2);
            if ((nRet & SCA_VALID) != SCA_VALID)
                // second cell is invalid.
                continue;

            if (aExtInfo1.mnFileId != aExtInfo2.mnFileId || aExtInfo1.mbExternal != aExtInfo2.mbExternal)
                // external info inconsistency.
                continue;

            // All looks good!

            if (bFirst)
                bFirst = false;
            else
                aRetStr.appendAscii(";");

            lcl_appendCellRangeAddress(aRetStr, pDoc, aCell1, aCell2, aExtInfo1, aExtInfo2);
        }
        else
        {
            // Chart always saves ranges using CONV_OOO convention.
            ScAddress::ExternalInfo aExtInfo;
            ScAddress aCell;
            sal_uInt16 nRet = aCell.Parse(aToken, pDoc, ::formula::FormulaGrammar::CONV_OOO, &aExtInfo);
            if ((nRet & SCA_VALID) != SCA_VALID)
                continue;

            // Looks good!

            if (bFirst)
                bFirst = false;
            else
                aRetStr.appendAscii(";");

            lcl_appendCellAddress(aRetStr, pDoc, aCell, aExtInfo);
        }
    }

    rString = aRetStr.makeStringAndClear();
}

//========================================================================

ScArea::ScArea( SCTAB tab,
				SCCOL colStart, SCROW rowStart,
				SCCOL colEnd,   SCROW rowEnd ) :
		nTab	 ( tab ),
		nColStart( colStart ),	nRowStart( rowStart ),
		nColEnd	 ( colEnd ),	nRowEnd  ( rowEnd )
{
}

//------------------------------------------------------------------------

ScArea::ScArea( const ScArea& r ) :
		nTab	 ( r.nTab ),
		nColStart( r.nColStart ),	nRowStart( r.nRowStart ),
		nColEnd  ( r.nColEnd ),		nRowEnd  ( r.nRowEnd )
{
}

//------------------------------------------------------------------------

ScArea& ScArea::operator=( const ScArea& r )
{
	nTab		= r.nTab;
	nColStart	= r.nColStart;
	nRowStart	= r.nRowStart;
	nColEnd		= r.nColEnd;
	nRowEnd		= r.nRowEnd;
	return *this;
}

//------------------------------------------------------------------------

sal_Bool ScArea::operator==( const ScArea& r ) const
{
	return (   (nTab		== r.nTab)
			&& (nColStart	== r.nColStart)
			&& (nRowStart	== r.nRowStart)
			&& (nColEnd		== r.nColEnd)
			&& (nRowEnd		== r.nRowEnd) );
}

//------------------------------------------------------------------------

ScAreaNameIterator::ScAreaNameIterator( ScDocument* pDoc ) :
	aStrNoName( ScGlobal::GetRscString(STR_DB_NONAME) )
{
	pRangeName = pDoc->GetRangeName();
	pDBCollection = pDoc->GetDBCollection();
	nPos = 0;
	bFirstPass = sal_True;
}

sal_Bool ScAreaNameIterator::Next( String& rName, ScRange& rRange )
{
	for (;;)
	{
		if ( bFirstPass )									// erst Bereichsnamen
		{
			if ( pRangeName && nPos < pRangeName->GetCount() )
			{
				ScRangeData* pData = (*pRangeName)[nPos++];
				if ( pData && pData->IsValidReference(rRange) )
				{
					rName = pData->GetName();
					return sal_True;							// gefunden
				}
			}
			else
			{
				bFirstPass = sal_False;
				nPos = 0;
			}
		}
		if ( !bFirstPass )									// dann DB-Bereiche
		{
			if ( pDBCollection && nPos < pDBCollection->GetCount() )
			{
				ScDBData* pData = (*pDBCollection)[nPos++];
				if (pData && pData->GetName() != aStrNoName)
				{
					pData->GetArea( rRange );
					rName = pData->GetName();
					return sal_True;							// gefunden
				}
			}
			else
				return sal_False;								// gibt nichts mehr
		}
	}
}




