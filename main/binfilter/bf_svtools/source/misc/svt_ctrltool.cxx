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



#define CTRLTOOL_CXX

#include <string.h>
#include <tools/debug.hxx>
#include <i18npool/mslangid.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <svtools.hrc>
#include <svtdata.hxx>
#include <ctrltool.hxx>

namespace binfilter
{

class ImplFontListFontInfo : public FontInfo
{
	friend class FontList;

private:
	OutputDevice*			mpDevice;
	ImplFontListFontInfo*	mpNext;

public:
							ImplFontListFontInfo( const FontInfo& rInfo,
												  OutputDevice* pDev ) :
								FontInfo( rInfo )
							{
								mpDevice = pDev;
							}

	OutputDevice*			GetDevice() const { return mpDevice; }
};

class ImplFontListNameInfo
{
	friend class FontList;

private:
	XubString				maSearchName;
	ImplFontListFontInfo*	mpFirst;
	USHORT					mnType;

							ImplFontListNameInfo( const XubString& rSearchName ) :
								maSearchName( rSearchName )
							{}

	const XubString&		GetSearchName() const { return maSearchName; }
};

static StringCompare ImplCompareFontInfo( ImplFontListFontInfo* pInfo1,
										  ImplFontListFontInfo* pInfo2 )
{
	if ( pInfo1->GetWeight() < pInfo2->GetWeight() )
		return COMPARE_LESS;
	else if ( pInfo1->GetWeight() > pInfo2->GetWeight() )
		return COMPARE_GREATER;

	if ( pInfo1->GetItalic() < pInfo2->GetItalic() )
		return COMPARE_LESS;
	else if ( pInfo1->GetItalic() > pInfo2->GetItalic() )
		return COMPARE_GREATER;

	return pInfo1->GetStyleName().CompareTo( pInfo2->GetStyleName() );
}

static void ImplMakeSearchString( XubString& rStr )
{
	rStr.ToLowerAscii();
}

// -----------------------------------------------------------------------

static void ImplMakeSearchStringFromName( XubString& rStr )
{
	rStr = rStr.GetToken( 0, ';' );
	ImplMakeSearchString( rStr );
}

// -----------------------------------------------------------------------

ImplFontListNameInfo* FontList::ImplFind( const XubString& rSearchName, ULONG* pIndex ) const
{
	// Wenn kein Eintrag in der Liste oder der Eintrag groesser ist als
	// der Letzte, dann hinten dranhaengen. Wir vergleichen erst mit dem
	// letzten Eintrag, da die Liste von VCL auch sortiert zurueckkommt
	// und somit die Wahrscheinlichkeit das hinten angehaengt werden muss
	// sehr gross ist.
	StringCompare eComp;
	ULONG nCnt = Count();
	if ( !nCnt )
	{
		if ( pIndex )
			*pIndex = LIST_APPEND;
		return NULL;
	}
	else
	{
		ImplFontListNameInfo* pCmpData = (ImplFontListNameInfo*)List::GetObject( nCnt-1 );
		eComp = rSearchName.CompareTo( pCmpData->maSearchName );
		if ( eComp == COMPARE_GREATER )
		{
			if ( pIndex )
				*pIndex = LIST_APPEND;
			return NULL;
		}
		else if ( eComp == COMPARE_EQUAL )
			return pCmpData;
	}

	// Fonts in der Liste suchen
	ImplFontListNameInfo*	pCompareData;
	ImplFontListNameInfo*	pFoundData = NULL;
	ULONG					nLow = 0;
	ULONG					nHigh = nCnt-1;
	ULONG					nMid;

	do
	{
		nMid = (nLow + nHigh) / 2;
		pCompareData = (ImplFontListNameInfo*)List::GetObject( nMid );
		eComp = rSearchName.CompareTo( pCompareData->maSearchName );
		if ( eComp == COMPARE_LESS )
		{
			if ( !nMid )
				break;
			nHigh = nMid-1;
		}
		else
		{
			if ( eComp == COMPARE_GREATER )
				nLow = nMid + 1;
			else
			{
				pFoundData = pCompareData;
				break;
			}
		}
	}
	while ( nLow <= nHigh );

	if ( pIndex )
	{
		eComp = rSearchName.CompareTo( pCompareData->maSearchName );
		if ( eComp == COMPARE_GREATER )
			*pIndex = (nMid+1);
		else
			*pIndex = nMid;
	}

	return pFoundData;
}

// -----------------------------------------------------------------------

ImplFontListNameInfo* FontList::ImplFindByName( const XubString& rStr ) const
{
	XubString aSearchName = rStr;
	ImplMakeSearchStringFromName( aSearchName );
	return ImplFind( aSearchName, NULL );
}

// -----------------------------------------------------------------------

void FontList::ImplInsertFonts( OutputDevice* pDevice, BOOL bAll,
								BOOL bInsertData )
{
	rtl_TextEncoding eSystemEncoding = gsl_getSystemTextEncoding();

	USHORT nType;
	if ( pDevice->GetOutDevType() != OUTDEV_PRINTER )
		nType = FONTLIST_FONTNAMETYPE_SCREEN;
	else
		nType = FONTLIST_FONTNAMETYPE_PRINTER;

	// Alle Fonts vom Device abfragen
	int n = pDevice->GetDevFontCount();
	USHORT	i;
	for( i = 0; i < n; i++ )
	{
		FontInfo aFontInfo = pDevice->GetDevFont( i );

		// Wenn keine Raster-Schriften angezeigt werden sollen,
		// dann diese ignorieren
		if ( !bAll && (aFontInfo.GetType() == TYPE_RASTER) )
			continue;

		XubString				aSearchName = aFontInfo.GetName();
		ImplFontListNameInfo*	pData;
		ULONG					nIndex;
		ImplMakeSearchString( aSearchName );
		pData = ImplFind( aSearchName, &nIndex );

		if ( !pData )
		{
			if ( bInsertData )
			{
				ImplFontListFontInfo* pNewInfo = new ImplFontListFontInfo( aFontInfo, pDevice );
				pData = new ImplFontListNameInfo( aSearchName );
				pData->mpFirst		= pNewInfo;
				pNewInfo->mpNext	= NULL;
				pData->mnType		= 0;
				Insert( (void*)pData, nIndex );
			}
		}
		else
		{
			if ( bInsertData )
			{
				BOOL					bInsert = TRUE;
				ImplFontListFontInfo*	pPrev = NULL;
				ImplFontListFontInfo*	pTemp = pData->mpFirst;
				ImplFontListFontInfo*	pNewInfo = new ImplFontListFontInfo( aFontInfo, pDevice );
				while ( pTemp )
				{
					StringCompare eComp = ImplCompareFontInfo( pNewInfo, pTemp );
					if ( (eComp == COMPARE_LESS) || (eComp == COMPARE_EQUAL) )
					{
						if ( eComp == COMPARE_EQUAL )
						{
							// Overwrite charset, because charset should match
							// with the system charset
							if ( (pTemp->GetCharSet() != eSystemEncoding) &&
								 (pNewInfo->GetCharSet() == eSystemEncoding) )
							{
								ImplFontListFontInfo* pTemp2 = pTemp->mpNext;
								*((FontInfo*)pTemp) = *((FontInfo*)pNewInfo);
								pTemp->mpNext = pTemp2;
							}
							delete pNewInfo;
							bInsert = FALSE;
						}

						break;
					}

					pPrev = pTemp;
					pTemp = pTemp->mpNext;
				}

				if ( bInsert )
				{
					pNewInfo->mpNext = pTemp;
					if ( pPrev )
						pPrev->mpNext = pNewInfo;
					else
						pData->mpFirst = pNewInfo;
				}
			}
		}

		if ( pData )
		{
			pData->mnType |= nType;
			if ( aFontInfo.GetType() != TYPE_RASTER )
				pData->mnType |= FONTLIST_FONTNAMETYPE_SCALABLE;
		}
	}
}

// =======================================================================

FontList::FontList( OutputDevice* pDevice, OutputDevice* pDevice2, BOOL bAll ) :
	List( 4096, sal::static_int_cast< USHORT >(pDevice->GetDevFontCount()), 32 )
{
	// Variablen initialisieren
	mpDev = pDevice;
	mpDev2 = pDevice2;
	mpSizeAry = NULL;

	// Stylenamen festlegen
	maLight 		= XubString( SvtResId( STR_SVT_STYLE_LIGHT ) );
	maLightItalic	= XubString( SvtResId( STR_SVT_STYLE_LIGHT_ITALIC ) );
	maNormal		= XubString( SvtResId( STR_SVT_STYLE_NORMAL ) );
	maNormalItalic	= XubString( SvtResId( STR_SVT_STYLE_NORMAL_ITALIC ) );
	maBold			= XubString( SvtResId( STR_SVT_STYLE_BOLD ) );
	maBoldItalic	= XubString( SvtResId( STR_SVT_STYLE_BOLD_ITALIC ) );
	maBlack 		= XubString( SvtResId( STR_SVT_STYLE_BLACK ) );
	maBlackItalic	= XubString( SvtResId( STR_SVT_STYLE_BLACK_ITALIC ) );

	ImplInsertFonts( pDevice, bAll, TRUE );

	// Gegebenenfalls muessen wir mit den Bildschirmfonts vergleichen,
	// damit dort die eigentlich doppelten auf Equal mappen koennen
	BOOL bCompareWindow = FALSE;
	if ( !pDevice2 && (pDevice->GetOutDevType() == OUTDEV_PRINTER) )
	{
		bCompareWindow = TRUE;
		pDevice2 = Application::GetDefaultDevice();
	}

	if ( pDevice2 &&
		 (pDevice2->GetOutDevType() != pDevice->GetOutDevType()) )
		ImplInsertFonts( pDevice2, bAll, !bCompareWindow );
}

// -----------------------------------------------------------------------

FontList::~FontList()
{
	// Gegebenenfalls SizeArray loeschen
	if ( mpSizeAry )
		delete[] mpSizeAry;

	// FontInfos loeschen
	ImplFontListNameInfo* pData = (ImplFontListNameInfo*)First();
	while ( pData )
	{
		ImplFontListFontInfo* pTemp;
		ImplFontListFontInfo* pInfo = pData->mpFirst;
		while ( pInfo )
		{
			pTemp = pInfo->mpNext;
			delete pInfo;
			pInfo = pTemp;
		}
		ImplFontListNameInfo* pNext = (ImplFontListNameInfo*)Next();
		delete pData;
		pData = pNext;
	}
}

// -----------------------------------------------------------------------

FontInfo FontList::Get( const XubString& rName,
						FontWeight eWeight, FontItalic eItalic ) const
{
	ImplFontListNameInfo* pData = ImplFindByName( rName );
	ImplFontListFontInfo* pFontInfo = NULL;
	ImplFontListFontInfo* pFontNameInfo = NULL;
	if ( pData )
	{
		ImplFontListFontInfo* pSearchInfo = pData->mpFirst;
		pFontNameInfo = pSearchInfo;
		while ( pSearchInfo )
		{
			if ( (eWeight == pSearchInfo->GetWeight()) &&
				 (eItalic == pSearchInfo->GetItalic()) )
			{
				pFontInfo = pSearchInfo;
				break;
			}

			pSearchInfo = pSearchInfo->mpNext;
		}
	}

	// Konnten die Daten nicht gefunden werden, dann muessen bestimmte
	// Attribute nachgebildet werden
	FontInfo aInfo;
	if ( !pFontInfo )
	{
		// Falls der Fontname stimmt, uebernehmen wir soviel wie moeglich
		if ( pFontNameInfo )
		{
			aInfo = *pFontNameInfo;
			aInfo.SetStyleName( XubString() );
		}

		aInfo.SetWeight( eWeight );
		aInfo.SetItalic( eItalic );
	}
	else
		aInfo = *pFontInfo;

	// set Fontname to keep FontAlias
	aInfo.SetName( rName );

	return aInfo;
}

//------------------------------------------------------------------------

}
