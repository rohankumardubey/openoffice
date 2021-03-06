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



#ifdef _MSC_VER
#pragma hdrstop
#endif

#define ITEMID_UNDERLINE	0
#define ITEMID_WEIGHT       0
#define ITEMID_ESCAPEMENT   0
#define ITEMID_CHARSETCOLOR 0
#define ITEMID_COLOR 		0

//#ifndef _SYSTEM_HXX //autogen
//#include <vcl/system.hxx>
//#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SFX_DOCFILE_HXX
#include <bf_sfx2/docfile.hxx>
#endif
// fuer die Sort-String-Arrays aus dem SVMEM.HXX
#define _SVSTDARR_STRINGSISORTDTOR
#define _SVSTDARR_STRINGSDTOR

#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_UNICODETYPE_HDL_
#include <com/sun/star/i18n/UnicodeType.hdl>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svxids.hrc>
#endif

#include "escpitem.hxx"
#include "svxacorr.hxx"

#ifndef _SVX_HELPID_HRC
#include <helpid.hrc>
#endif

#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <bf_xmloff/xmltoken.hxx>
#endif
namespace binfilter {

using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;
using namespace ::rtl;
using namespace ::utl;

const int C_NONE 				= 0x00;
const int C_FULL_STOP 			= 0x01;
const int C_EXCLAMATION_MARK	= 0x02;
const int C_QUESTION_MARK		= 0x04;

static const sal_Char pImplWrdStt_ExcptLstStr[]    = "WordExceptList";
static const sal_Char pImplCplStt_ExcptLstStr[]    = "SentenceExceptList";
static const sal_Char pImplAutocorr_ListStr[]      = "DocumentList";
static const sal_Char pXMLImplWrdStt_ExcptLstStr[] = "WordExceptList.xml";
static const sal_Char pXMLImplCplStt_ExcptLstStr[] = "SentenceExceptList.xml";
static const sal_Char pXMLImplAutocorr_ListStr[]   = "DocumentList.xml";

static const sal_Char
	/* auch bei diesen Anfaengen - Klammern auf und alle Arten von Anf.Zei. */
	sImplSttSkipChars[]	= "\"\'([{\x83\x84\x89\x91\x92\x93\x94",
	/* auch bei diesen Ende - Klammern auf und alle Arten von Anf.Zei. */
	sImplEndSkipChars[]	= "\"\')]}\x83\x84\x89\x91\x92\x93\x94";

// diese Zeichen sind in Worten erlaubt: (fuer FnCptlSttSntnc)
static const sal_Char sImplWordChars[] = "-'";

void EncryptBlockName_Imp( String& rName );
void DecryptBlockName_Imp( String& rName );


// FileVersions Nummern fuer die Ersetzungs-/Ausnahmelisten getrennt
#define WORDLIST_VERSION_358	1
#define EXEPTLIST_VERSION_358	0



/*N*/ typedef SvxAutoCorrectLanguageLists* SvxAutoCorrectLanguageListsPtr;
/*N*/ DECLARE_TABLE( SvxAutoCorrLanguageTable_Impl,  SvxAutoCorrectLanguageListsPtr)//STRIP008 ;

/*N*/ DECLARE_TABLE( SvxAutoCorrLastFileAskTable_Impl, long )//STRIP008 ;









	// wird nach dem austauschen der Zeichen von den Funktionen
	//	- FnCptlSttWrd
	// 	- FnCptlSttSntnc
	// gerufen. Dann koennen die Worte ggfs. in die Ausnahmelisten
	// aufgenommen werden.



/*N*/ static USHORT GetAppLang()
/*N*/ {
/*N*/ 	return Application::GetSettings().GetLanguage();
/*N*/ }





/* -----------------18.11.98 15:28-------------------
 *
 * --------------------------------------------------*/
/*N*/ void lcl_ClearTable(SvxAutoCorrLanguageTable_Impl& rLangTable)
/*N*/ {
/*N*/ 	SvxAutoCorrectLanguageListsPtr pLists = rLangTable.Last();
/*N*/ 	while(pLists)
/*N*/ 	{
/*?*/ 		delete pLists;
/*?*/ 		pLists = rLangTable.Prev();
/*N*/ 	}
/*N*/ 	rLangTable.Clear();
/*N*/ }

/* -----------------19.11.98 10:15-------------------
 *
 * --------------------------------------------------*/
/*N*/ long SvxAutoCorrect::GetDefaultFlags()
/*N*/ {
/*N*/ 	long nRet = Autocorrect
/*N*/ 					| CptlSttSntnc
/*N*/ 					| CptlSttWrd
/*N*/ 					| ChgFractionSymbol
/*N*/ 					| ChgOrdinalNumber
/*N*/ 					| ChgToEnEmDash
/*N*/ 					| ChgWeightUnderl
/*N*/ 					| SetINetAttr
/*N*/ 					| ChgQuotes
/*N*/ 					| SaveWordCplSttLst
/*N*/ 					| SaveWordWrdSttLst;
/*N*/ 	LanguageType eLang = GetAppLang();
/*N*/ 	switch( eLang )
/*N*/ 	{
/*N*/ 	case LANGUAGE_ENGLISH:
/*N*/ 	case LANGUAGE_ENGLISH_US:
/*N*/ 	case LANGUAGE_ENGLISH_UK:
/*N*/ 	case LANGUAGE_ENGLISH_AUS:
/*N*/ 	case LANGUAGE_ENGLISH_CAN:
/*N*/ 	case LANGUAGE_ENGLISH_NZ:
/*N*/ 	case LANGUAGE_ENGLISH_EIRE:
/*N*/ 	case LANGUAGE_ENGLISH_SAFRICA:
/*N*/ 	case LANGUAGE_ENGLISH_JAMAICA:
/*N*/ 	case LANGUAGE_ENGLISH_CARRIBEAN:
/*N*/ 		nRet &= ~(ChgQuotes|ChgSglQuotes);
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }


/*N*/ SvxAutoCorrect::SvxAutoCorrect( const String& rShareAutocorrFile,
/*N*/ 								const String& rUserAutocorrFile )
/*N*/ 	: sShareAutoCorrFile( rShareAutocorrFile ),
/*N*/ 	sUserAutoCorrFile( rUserAutocorrFile ),
/*N*/ 	cStartSQuote( 0 ), cEndSQuote( 0 ), cStartDQuote( 0 ), cEndDQuote( 0 ),
/*N*/ 	pLangTable( new SvxAutoCorrLanguageTable_Impl ),
/*N*/ 	pLastFileTable( new SvxAutoCorrLastFileAskTable_Impl ),
/*N*/ 	pCharClass( 0 )
/*N*/ {
/*N*/ 	nFlags = SvxAutoCorrect::GetDefaultFlags();
/*N*/ 
/*N*/ 	c1Div2 = ByteString::ConvertToUnicode( '\xBD', RTL_TEXTENCODING_MS_1252 );
/*N*/ 	c1Div4 = ByteString::ConvertToUnicode( '\xBC', RTL_TEXTENCODING_MS_1252 );
/*N*/ 	c3Div4 = ByteString::ConvertToUnicode( '\xBE', RTL_TEXTENCODING_MS_1252 );
/*N*/ 	cEmDash = ByteString::ConvertToUnicode( '\x97', RTL_TEXTENCODING_MS_1252 );
/*N*/ 	cEnDash = ByteString::ConvertToUnicode( '\x96', RTL_TEXTENCODING_MS_1252 );
/*N*/ }

/*N*/ SvxAutoCorrect::SvxAutoCorrect( const SvxAutoCorrect& rCpy )
/*N*/ 	: nFlags( rCpy.nFlags & ~(ChgWordLstLoad|CplSttLstLoad|WrdSttLstLoad)),
/*N*/ 	aSwFlags( rCpy.aSwFlags ),
/* Die Sprachentabelle wird neu aufgebaut, da sie im Dtor von rCpy abgeraeumt wird!
 */
/*N*/ 	sShareAutoCorrFile( rCpy.sShareAutoCorrFile ),
/*N*/ 	sUserAutoCorrFile( rCpy.sUserAutoCorrFile ),
/*N*/ 	cStartSQuote( rCpy.cStartSQuote ), cEndSQuote( rCpy.cEndSQuote ),
/*N*/ 	cStartDQuote( rCpy.cStartDQuote ), cEndDQuote( rCpy.cEndDQuote ),
/*N*/ 	c1Div2( rCpy.c1Div2 ), c1Div4( rCpy.c1Div4 ), c3Div4( rCpy.c3Div4 ),
/*N*/ 	cEmDash( rCpy.cEmDash ), cEnDash( rCpy.cEnDash ),
/*N*/ 	pLangTable( new SvxAutoCorrLanguageTable_Impl ),
/*N*/ 	pLastFileTable( new SvxAutoCorrLastFileAskTable_Impl ),
/*N*/ 	pCharClass( 0 )
/*N*/ {
/*N*/ }


/*N*/ SvxAutoCorrect::~SvxAutoCorrect()
/*N*/ {
/*N*/ 	lcl_ClearTable(*pLangTable);
/*N*/ 	delete pLangTable;
/*N*/ 	delete pLastFileTable;
/*N*/ 	delete pCharClass;
/*N*/ }


/*N*/ void SvxAutoCorrect::SetAutoCorrFlag( long nFlag, BOOL bOn )
/*N*/ {
/*N*/ 	long nOld = nFlags;
/*N*/ 	nFlags = bOn ? nFlags | nFlag
/*N*/ 				 : nFlags & ~nFlag;
/*N*/ 
/*N*/ 	if( !bOn )
/*N*/ 	{
/*N*/ 		if( (nOld & CptlSttSntnc) != (nFlags & CptlSttSntnc) )
/*?*/ 			nFlags &= ~CplSttLstLoad;
/*N*/ 		if( (nOld & CptlSttWrd) != (nFlags & CptlSttWrd) )
/*?*/ 			nFlags &= ~WrdSttLstLoad;
/*N*/ 		if( (nOld & Autocorrect) != (nFlags & Autocorrect) )
/*?*/ 			nFlags &= ~ChgWordLstLoad;
/*N*/ 	}
/*N*/ }


	// Zwei Grossbuchstaben am Wort-Anfang ??












//The method below is renamed from _GetQuote to GetQuote by BerryJia for Bug95846 Time:2002-8-13 15:50







	// fuegt ein einzelnes Wort hinzu. Die Liste wird sofort
	// in die Datei geschrieben!


	// fuegt ein einzelnes Wort hinzu. Die Liste wird sofort
	// in die Datei geschrieben!











	//	- loesche einen Eintrag


	//	- return den Ersetzungstext (nur fuer SWG-Format, alle anderen
	//		koennen aus der Wortliste herausgeholt werden!)

	//	- Text mit Attributierung (kann nur der SWG - SWG-Format!)




/* This code is copied from SwXMLTextBlocks::GeneratePackageName */



/* -----------------18.11.98 16:00-------------------
 *
 * --------------------------------------------------*/


// suche das oder die Worte in der ErsetzungsTabelle
/* -----------------18.11.98 13:46-------------------
 *
 * --------------------------------------------------*/
/* -----------------18.11.98 14:28-------------------
 *
 * --------------------------------------------------*/
/* -----------------18.11.98 14:49-------------------
 *
 * --------------------------------------------------*/

/* -----------------20.11.98 11:53-------------------
 *
 * --------------------------------------------------*/

/* -----------------18.11.98 11:16-------------------
 *
 * --------------------------------------------------*/
/*N*/ SvxAutoCorrectLanguageLists::SvxAutoCorrectLanguageLists(
/*N*/ 				SvxAutoCorrect& rParent,
/*N*/ 				const String& rShareAutoCorrectFile,
/*N*/ 				const String& rUserAutoCorrectFile,
/*N*/ 				LanguageType eLang)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

/* -----------------18.11.98 11:16-------------------
 *
 * --------------------------------------------------*/
/*N*/ SvxAutoCorrectLanguageLists::~SvxAutoCorrectLanguageLists()
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/

/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/


/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
/* -----------------18.11.98 15:20-------------------
 *
 * --------------------------------------------------*/

/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/

/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/

/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/


/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/

/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
	//	- Text mit Attributierung (kann nur der SWG - SWG-Format!)

/* -----------------18.11.98 11:26-------------------
 *
 * --------------------------------------------------*/
	//	- loesche einen Eintrag
}
