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


#ifndef	_DRAWFONT_HXX
#define	_DRAWFONT_HXX

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_SHORTS
#define _SVSTDARR_BYTES
#define _SVSTDARR_USHORTS
#define _SVSTDARR_XUB_STRLEN
#include <bf_svtools/svstdarr.hxx>
#endif
#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#include "errhdl.hxx"
#ifndef _FRACT_HXX
#include <tools/fract.hxx>
#endif

#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif
class OutputDevice;
class Point;
class Size;
class Font;
namespace binfilter {

class SwWrongList;
class SwFont;

class ViewShell;
class SwTxtNode;
class SwAttrHandler;
class SwTxtFrm;
class SwUnderlineFont;

/*************************************************************************
 *				  class SwScanner
 * Hilfsklasse, die beim Spellen die Worte im gewuenschten Bereich
 * nacheinander zur Verfuegung stellt.
 *************************************************************************/

class SwScanner
{
    XubString aWord;
    const SwWrongList* pWrong;
    const SwTxtNode& rNode;
    xub_StrLen nEndPos;
    xub_StrLen nBegin;
    xub_StrLen nLen;
    LanguageType aCurrLang;
    USHORT nWordType;
    BOOL bReverse;
    BOOL bStart;
    BOOL bIsOnlineSpell;

public:
    SwScanner( const SwTxtNode& rNd, const SwWrongList* pWrng, USHORT nWordType,
               xub_StrLen nStart, xub_StrLen nEnde, BOOL bRev, BOOL bOS );

    // This next word function tries to find the language for the next word
    // It should currently _not_ be used for spell checking, and works only for
    // ! bReverse
    BOOL NextWord();

    const XubString& GetWord() const    { return aWord; }

    xub_StrLen GetBegin() const         { return nBegin; }
    xub_StrLen GetEnd() const           { return nBegin + nLen; }
    xub_StrLen GetLen() const           { return nLen; }
};

/*************************************************************************
 *						class SwScriptInfo
 *
 * encapsultes information about script changes
 *************************************************************************/

class SwScriptInfo
{
private:
    SvXub_StrLens aScriptChg;
    SvBytes aScriptType;
    SvXub_StrLens aDirChg;
    SvBytes aDirType;
    SvXub_StrLens aKashida;
    SvXub_StrLens aCompChg;
    SvXub_StrLens aCompLen;
    SvBytes aCompType;
    xub_StrLen nInvalidityPos;
    BYTE nDefaultDir;

    void UpdateBidiInfo( const String& rTxt );

public:
    enum CompType { KANA, SPECIAL_LEFT, SPECIAL_RIGHT, NONE };

    inline SwScriptInfo() : nInvalidityPos( 0 ), nDefaultDir( 0 ) {};

    // determines script changes
    void InitScriptInfo( const SwTxtNode& rNode, sal_Bool bRTL );

    // set/get position from which data is invalid
    inline void SetInvalidity( const xub_StrLen nPos );
    inline xub_StrLen GetInvalidity() const { return nInvalidityPos; };

    // get default direction for paragraph
    inline BYTE GetDefaultDir() const { return nDefaultDir; };

    // array operations, nCnt refers to array position
    inline USHORT CountScriptChg() const;
    inline xub_StrLen GetScriptChg( const USHORT nCnt ) const;
    inline BYTE GetScriptType( const USHORT nCnt ) const;

    inline USHORT CountDirChg() const;
    inline xub_StrLen GetDirChg( const USHORT nCnt ) const;
    inline BYTE GetDirType( const USHORT nCnt ) const;

    inline USHORT CountKashida() const;
    inline xub_StrLen GetKashida( const USHORT nCnt ) const;

    inline USHORT CountCompChg() const;
    inline xub_StrLen GetCompStart( const USHORT nCnt ) const;
    inline xub_StrLen GetCompLen( const USHORT nCnt ) const;
    inline BYTE GetCompType( const USHORT nCnt ) const;

    // "high" level operations, nPos refers to string position
    xub_StrLen NextScriptChg( const xub_StrLen nPos ) const;
    BYTE ScriptType( const xub_StrLen nPos ) const;

    // Returns the position of the next direction level change.
    // If bLevel is set, the position of the next level which is smaller
    // than the level at position nPos is returned. This is required to
    // obtain the end of a SwBidiPortion
    xub_StrLen NextDirChg( const xub_StrLen nPos,
                           const BYTE* pLevel = 0 ) const;
    BYTE DirType( const xub_StrLen nPos ) const;


    // examines the range [ nStart, nStart + nEnd ] if there are kanas
    // returns start index of kana entry in array, otherwise USHRT_MAX
    USHORT HasKana( xub_StrLen nStart, const xub_StrLen nEnd ) const;

    // modifies the kerning array according to a given compress value
    long Compress( sal_Int32* pKernArray, xub_StrLen nIdx, xub_StrLen nLen,
                   const USHORT nCompress, const USHORT nFontHeight,
                   Point* pPoint = NULL ) const;
};

inline void SwScriptInfo::SetInvalidity( const xub_StrLen nPos )
{
    if ( nPos < nInvalidityPos )
        nInvalidityPos = nPos;
};
inline USHORT SwScriptInfo::CountScriptChg() const { return aScriptChg.Count(); }
inline xub_StrLen SwScriptInfo::GetScriptChg( const USHORT nCnt ) const
{
    ASSERT( nCnt < aScriptChg.Count(),"No ScriptChange today!");
    return aScriptChg[ nCnt ];
}
inline BYTE SwScriptInfo::GetScriptType( const xub_StrLen nCnt ) const
{
    ASSERT( nCnt < aScriptChg.Count(),"No ScriptType today!");
    return aScriptType[ nCnt ];
}

inline USHORT SwScriptInfo::CountDirChg() const { return aDirChg.Count(); }
inline xub_StrLen SwScriptInfo::GetDirChg( const USHORT nCnt ) const
{
    ASSERT( nCnt < aDirChg.Count(),"No DirChange today!");
    return aDirChg[ nCnt ];
}
inline BYTE SwScriptInfo::GetDirType( const xub_StrLen nCnt ) const
{
    ASSERT( nCnt < aDirChg.Count(),"No DirType today!");
    return aDirType[ nCnt ];
}

inline USHORT SwScriptInfo::CountKashida() const { return aKashida.Count(); }
inline xub_StrLen SwScriptInfo::GetKashida( const USHORT nCnt ) const
{
    ASSERT( nCnt < aKashida.Count(),"No Kashidas today!");
    return aKashida[ nCnt ];
}

inline USHORT SwScriptInfo::CountCompChg() const { return aCompChg.Count(); };
inline xub_StrLen SwScriptInfo::GetCompStart( const USHORT nCnt ) const
{
    ASSERT( nCnt < aCompChg.Count(),"No CompressionStart today!");
    return aCompChg[ nCnt ];
}
inline xub_StrLen SwScriptInfo::GetCompLen( const USHORT nCnt ) const
{
    ASSERT( nCnt < aCompChg.Count(),"No CompressionLen today!");
    return aCompLen[ nCnt ];
}

inline BYTE SwScriptInfo::GetCompType( const USHORT nCnt ) const
{
    ASSERT( nCnt < aCompChg.Count(),"No CompressionType today!");
    return aCompType[ nCnt ];
}

/*************************************************************************
 *                      class SwDrawTextInfo
 *
 * encapsulates information for drawing text
 *************************************************************************/

class SwDrawTextInfo
{
    const SwTxtFrm* pFrm;
	OutputDevice* pOut;
	ViewShell* pSh;
    const SwScriptInfo* pScriptInfo;
	const Point* pPos;
	const XubString* pText;
	const SwWrongList* pWrong;
	const Size* pSize;
	SwFont *pFnt;
    SwUnderlineFont* pUnderFnt;
	xub_StrLen* pHyphPos;
//   Fraction aZoom;
	long nLeft;
	long nRight;
	long nKanaDiff;
	xub_StrLen nIdx;
	xub_StrLen nLen;
    xub_StrLen nOfst;
	USHORT nWidth;
	USHORT nAscent;
    USHORT nCompress;
	short nSperren;
	short nKern;
	short nSpace;
    BYTE nCursorBidiLevel;
	BOOL bBullet : 1;
	BOOL bUpper : 1;		// Fuer Kapitaelchen: Grossbuchstaben-Flag
	BOOL bDrawSpace : 1;	// Fuer Kapitaelchen: Unter/Durchstreichung
	BOOL bGreyWave	: 1;	// Graue Wellenlinie beim extended TextInput
    BOOL bSpaceStop : 1;    // For underlining we need to know, if a portion
                            // is right in front of a hole portion or a
                            // fix margin portion.
    BOOL bSnapToGrid : 1;   // Does paragraph snap to grid?
    BOOL bIgnoreFrmRTL : 1; // Paint text as if text has LTR direction, used for
                            // line numbering
    BOOL bPosMatchesBounds :1;  // GetCrsrOfst should not return the next
                                // position if screen position is inside second
                                // have of bound rect, used for Accessibility

    SwDrawTextInfo();       // nicht zulaessig
public:
#ifdef DBG_UTIL
	BOOL bOut 	: 1;	// In der Non-Product wird der Zugriff auf die Member
	BOOL bPos 	: 1;	// durch diese Flags ueberwacht.
	BOOL bText	: 1;	// Wird ein Member gesetzt, so wird das entsprechende
	BOOL bWrong	: 1;	// Flag gesetzt.
	BOOL bSize	: 1;	// Wird ein Member ausgelesen, so wird ASSERTet, dass
	BOOL bFnt	: 1;	// dieser zuvor gesetzt worden ist.
	BOOL bIdx	: 1;
	BOOL bLen	: 1;
	BOOL bWidth	: 1;
	BOOL bAscent: 1;
	BOOL bSperr : 1;
	BOOL bKern	: 1;
	BOOL bSpace	: 1;
	BOOL bBull	: 1;
	BOOL bSpec	: 1;
	BOOL bUppr	: 1;
	BOOL bDrawSp: 1;
	BOOL bGreyWv: 1;
    BOOL bLeft  : 1;
	BOOL bRight : 1;
	BOOL bKana	: 1;
	BOOL bOfst	: 1;
	BOOL bHyph	: 1;
#endif

    SwDrawTextInfo( ViewShell *pS, OutputDevice &rO, const SwScriptInfo* pSI,
					const XubString &rSt, xub_StrLen nI, xub_StrLen nL,
                    USHORT nW = 0, BOOL bB = FALSE )
    {
        pFrm = NULL;
        pSh = pS;
        pOut = &rO;
        pScriptInfo = pSI;
        pText = &rSt;
        nIdx = nI;
        nLen = nL;
        nKern = 0;
        nCompress = 0;
        nWidth = nW;
        nCursorBidiLevel = 0;
        bBullet = bB;
        pUnderFnt = 0;
        bGreyWave = FALSE;
        bSpaceStop = FALSE;
        bSnapToGrid = FALSE;
        bIgnoreFrmRTL = FALSE;
        bPosMatchesBounds = FALSE;

#ifdef DBG_UTIL
        bOut = bText = bIdx = bLen = bWidth = bKern = bBull = bSpec =
        bGreyWv = TRUE;

        bPos = bWrong = bSize = bFnt = bAscent = bSpace = bUppr =
        bDrawSp = bLeft = bRight = bKana = bOfst = bHyph = FALSE;
#endif
	}

    const SwTxtFrm* GetFrm() const
    {
        return pFrm;
    }

    void SetFrm( const SwTxtFrm* pNewFrm )
    {
        pFrm = pNewFrm;
    }

    ViewShell *GetShell() const
    {
        return pSh;
    }

    OutputDevice& GetOut() const
    {
		ASSERT( bOut, "DrawTextInfo: Undefined Outputdevice" );
		return *pOut;
	}

    OutputDevice *GetpOut() const
    {
		ASSERT( bOut, "DrawTextInfo: Undefined Outputdevice" );
		return pOut;
	}

    const SwScriptInfo* GetScriptInfo() const
    {
        return pScriptInfo;
    }

    const Point &GetPos() const
    {
		ASSERT( bPos, "DrawTextInfo: Undefined Position" );
		return *pPos;
	}

    xub_StrLen *GetHyphPos() const
    {
		ASSERT( bHyph, "DrawTextInfo: Undefined Hyph Position" );
		return pHyphPos;
	}

    const XubString &GetText() const
    {
		ASSERT( bText, "DrawTextInfo: Undefined String" );
		return *pText;
	}

    const SwWrongList* GetWrong() const
    {
		ASSERT( bWrong, "DrawTextInfo: Undefined WrongList" );
		return pWrong;
	}

    const Size &GetSize() const
    {
		ASSERT( bSize, "DrawTextInfo: Undefined Size" );
		return *pSize;
	}

    SwFont* GetFont() const
    {
		ASSERT( bFnt, "DrawTextInfo: Undefined Font" );
		return pFnt;
	}

    SwUnderlineFont* GetUnderFnt() const
    {
        ASSERT( bSpec, "DrawTextInfo: Undefined Underlinefont" );
        return pUnderFnt;
	}

    xub_StrLen GetIdx() const
    {
		ASSERT( bIdx, "DrawTextInfo: Undefined Index" );
		return nIdx;
	}

    xub_StrLen GetLen() const
    {
		ASSERT( bLen, "DrawTextInfo: Undefined Length" );
		return nLen;
	}

    xub_StrLen GetOfst() const
    {
        ASSERT( bOfst, "DrawTextInfo: Undefined Offset" );
		return nOfst;
	}

    xub_StrLen GetEnd() const
    {
		ASSERT( bIdx, "DrawTextInfo: Undefined Index" );
		ASSERT( bLen, "DrawTextInfo: Undefined Length" );
		return nIdx + nLen;
	}

    long GetLeft() const
    {
		ASSERT( bLeft, "DrawTextInfo: Undefined left range" );
		return nLeft;
	}

    long GetRight() const
    {
		ASSERT( bRight, "DrawTextInfo: Undefined right range" );
		return nRight;
	}

    long GetKanaDiff() const
    {
		ASSERT( bKana, "DrawTextInfo: Undefined kana difference" );
		return nKanaDiff;
	}

    USHORT GetWidth() const
    {
		ASSERT( bWidth, "DrawTextInfo: Undefined Width" );
		return nWidth;
	}

    USHORT GetAscent() const
    {
		ASSERT( bAscent, "DrawTextInfo: Undefined Ascent" );
		return nAscent;
	}

    USHORT GetKanaComp() const
    {
        return nCompress;
	}

    short GetSperren() const
    {
		ASSERT( bSperr, "DrawTextInfo: Undefined >Sperren<" );
		return nSperren;
	}

    short GetKern() const
    {
		ASSERT( bKern, "DrawTextInfo: Undefined Kerning" );
		return nKern;
	}

    short GetSpace() const
    {
		ASSERT( bSpace, "DrawTextInfo: Undefined Spacing" );
		return nSpace;
	}

    BYTE GetCursorBidiLevel() const
    {
        return nCursorBidiLevel;
    }

    BOOL GetBullet() const
    {
		ASSERT( bBull, "DrawTextInfo: Undefined Bulletflag" );
		return bBullet;
	}

    BOOL GetUpper() const
    {
		ASSERT( bUppr, "DrawTextInfo: Undefined Upperflag" );
		return bUpper;
	}

    BOOL GetDrawSpace() const
    {
		ASSERT( bDrawSp, "DrawTextInfo: Undefined DrawSpaceflag" );
		return bDrawSpace;
	}

    BOOL GetGreyWave() const
    {
		ASSERT( bGreyWv, "DrawTextInfo: Undefined GreyWave" );
		return bGreyWave;
	}

    BOOL IsSpaceStop() const
    {
        return bSpaceStop;
	}

    BOOL SnapToGrid() const
    {
        return bSnapToGrid;
    }

    BOOL IsIgnoreFrmRTL() const
    {
        return bIgnoreFrmRTL;
    }

    BOOL IsPosMatchesBounds() const
    {
        return bPosMatchesBounds;
    }

    void SetOut( OutputDevice &rNew )
    {
        pOut = &rNew;
#ifdef DBG_UTIL
		bOut = TRUE;
#endif
	}

    void SetPos( const Point &rNew )
    {
        pPos = &rNew;
#ifdef DBG_UTIL
		bPos = TRUE;
#endif
	}

    void SetHyphPos( xub_StrLen *pNew )
    {
        pHyphPos = pNew;
#ifdef DBG_UTIL
		bHyph = TRUE;
#endif
	}

    void SetText( const XubString &rNew )
    {
        pText = &rNew;
#ifdef DBG_UTIL
		bText = TRUE;
#endif
	}

    void SetWrong( const SwWrongList* pNew )
    {
        pWrong = pNew;
#ifdef DBG_UTIL
		bWrong = TRUE;
#endif
	}

    void SetSize( const Size &rNew )
    {
        pSize = &rNew;
#ifdef DBG_UTIL
		bSize = TRUE;
#endif
	}

    void SetFont( SwFont* pNew )
    {
        pFnt = pNew;
#ifdef DBG_UTIL
		bFnt = TRUE;
#endif
	}

    void SetIdx( xub_StrLen nNew )
    {
        nIdx = nNew;
#ifdef DBG_UTIL
		bIdx = TRUE;
#endif
	}

    void SetLen( xub_StrLen nNew )
    {
        nLen = nNew;
#ifdef DBG_UTIL
		bLen = TRUE;
#endif
	}

    void SetOfst( xub_StrLen nNew )
    {
        nOfst = nNew;
#ifdef DBG_UTIL
		bOfst = TRUE;
#endif
	}

    void SetLeft( long nNew )
    {
        nLeft = nNew;
#ifdef DBG_UTIL
		bLeft = TRUE;
#endif
	}

    void SetRight( long nNew )
    {
        nRight = nNew;
#ifdef DBG_UTIL
		bRight = TRUE;
#endif
	}

    void SetKanaDiff( long nNew )
    {
        nKanaDiff = nNew;
#ifdef DBG_UTIL
		bKana = TRUE;
#endif
	}

    void SetWidth( USHORT nNew )
    {
        nWidth = nNew;
#ifdef DBG_UTIL
		bWidth = TRUE;
#endif
	}

    void SetAscent( USHORT nNew )
    {
        nAscent = nNew;
#ifdef DBG_UTIL
		bAscent = TRUE;
#endif
	}

    void SetKern( short nNew )
    {
        nKern = nNew;
#ifdef DBG_UTIL
		bKern = TRUE;
#endif
	}

    void SetSperren( short nNew )
    {
        nSperren = nNew;
#ifdef DBG_UTIL
		bSperr = TRUE;
#endif
	}

    void SetSpace( short nNew )
    {
		if( nNew < 0 )
		{
			SetSperren( -nNew );
			nSpace = 0;
		}
		else
		{
			nSpace = nNew;
			SetSperren( 0 );
		}
#ifdef DBG_UTIL
		bSpace = TRUE;
#endif
	}

    void SetCursorBidiLevel( BYTE nNew )
    {
        nCursorBidiLevel = nNew;
    }

    void SetKanaComp( short nNew )
    {
		nCompress = nNew;
	}

    void SetBullet( BOOL bNew )
    {
        bBullet = bNew;
#ifdef DBG_UTIL
		bBull = TRUE;
#endif
	}

    void SetUnderFnt( SwUnderlineFont* pFnt )
    {
        pUnderFnt = pFnt;
#ifdef DBG_UTIL
		bSpec = TRUE;
#endif
	}

    void SetUpper( BOOL bNew )
    {
        bUpper = bNew;
#ifdef DBG_UTIL
		bUppr = TRUE;
#endif
	}

    void SetDrawSpace( BOOL bNew )
    {
        bDrawSpace = bNew;
#ifdef DBG_UTIL
		bDrawSp = TRUE;
#endif
	}

    void SetGreyWave( BOOL bNew )
    {
        bGreyWave = bNew;
#ifdef DBG_UTIL
		bGreyWv = TRUE;
#endif
	}

    void SetSpaceStop( BOOL bNew )
    {
        bSpaceStop = bNew;
    }

    void SetSnapToGrid( BOOL bNew )
    {
        bSnapToGrid = bNew;
    }

    void SetIgnoreFrmRTL( BOOL bNew )
    {
        bIgnoreFrmRTL = bNew;
    }

    void SetPosMatchesBounds( BOOL bNew )
    {
        bPosMatchesBounds = bNew;
    }


    // sets a new color at the output device if necessary
    // if a font is passed as argument, the change if made to the font
    // otherwise the font at the output device is changed
    // returns if the font has been changed
};

} //namespace binfilter
#endif
