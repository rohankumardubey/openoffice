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


#ifndef _SWTYPES_HXX
#define _SWTYPES_HXX

#include <bf_svtools/bf_solar.h>
#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif
#include <limits.h> 	//fuer LONG_MAX

#ifdef PM20
#include <stdlib.h>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

// wenn das hier geaendert wird, dann auch im globals.hrc aendern!!!
//#define SW_FILEFORMAT_40 SOFFICE_FILEFORMAT_40

namespace com{namespace sun{namespace star{
	namespace linguistic2{
		class XDictionaryList;
		class XSpellChecker1;
		class XHyphenator;
		class XThesaurus;
	}
	namespace beans{
		class XPropertySet;
	}
}}}
namespace utl{
	class TransliterationWrapper;
}
class Size;
class MapMode;
class ResMgr;
class UniString;
class ByteString;
class Graphic;
class OutputDevice;
class CharClass;
class LocaleDataWrapper;
class CollatorWrapper;
namespace binfilter {


class SwPathFinder;


typedef long SwTwips;
#define INVALID_TWIPS	LONG_MAX
#define TWIPS_MAX		(LONG_MAX - 1)

#define MM50   283	// 1/2 cm in TWIPS

#define MINFLY 23	//Minimalgroesse fuer FlyFrms
#define MINLAY 23	//Minimalgroesse anderer Frms

// Default-Spaltenabstand zweier Textspalten entspricht 0.3 cm
#define DEF_GUTTER_WIDTH (MM50 / 5 * 3)

//Minimale Distance (Abstand zum Text) fuer das BorderAttribut, damit
//die aligned'en Linien nicht geplaettet werden.
//28 Twips == 0,4mm
#define MIN_BORDER_DIST 28

	/* minimaler Dokmentrand */
const SwTwips lMinBorder = 1134;

//Die Wiesenbreite links neben und ueber dem Dokument.
//Die halbe Wiesenbreite ist der Abstand zwischen den Seiten.
#define DOCUMENTBORDER 568L

// Konstante Strings
extern UniString aEmptyStr;			// ""
extern ByteString aEmptyByteStr;	// ""
extern UniString aDotStr;			// '.'

//Zum Einfuegen von Beschriftungen (wie bzw. wo soll eingefuegt werden).
//Hier weil ein eigenes hxx nicht lohnt und es sonst nirgendwo so recht
//hinpasst.
enum SwLabelType
{
	LTYPE_TABLE,	//Beschriftung einer Tabelle
	LTYPE_OBJECT,	//Beschriftung von Grafik oder OLE
	LTYPE_FLY,		//Beschriftung eines (Text-)Rahmens
	LTYPE_DRAW		//Beschriftung eines Zeichen-Objektes
};


const BYTE OLD_MAXLEVEL = 5;
const BYTE MAXLEVEL = 10;		//Ehemals numrule.hxx
const BYTE NO_NUM 		= 200;  //Ehemals numrule.hxx
const BYTE NO_NUMBERING = 201;  //Ehemals numrule.hxx
const BYTE NO_INIT 		= 202;  //Ehemals numrule.hxx

// fuer Absaetze mit NO_NUM aber auf unterschiedlichen Leveln
// DAMIT entfaellt das NO_NUM !!!!
const BYTE NO_NUMLEVEL  = 0x20;	// wird mit den Levels verodert


/*
 * Nette Funktionen als MACRO oder inline
 */

/* ein KiloByte sind 1024 Byte */
#define KB 1024

#define SET_CURR_SHELL( shell ) CurrShell aCurr( shell )

// pPathFinder wird von der UI initialisiert. Die Klasse liefert alle
// benoetigten Pfade.
extern SwPathFinder *pPathFinder;

// Werte fuer die Einzuege an der Nummerierung und BulletListe
// (fuer die weiteren Ebenen sind die Werte mit den Ebenen+1 zu
//	multiplizieren; Ebenen 0..4 !!!)
const USHORT lBullIndent = 567 / 2;
const short lBullFirstLineOffset = -567 / 2;
const USHORT lNumIndent = 567 / 2;
const short lNumFirstLineOffset = -567 / 2;

// Anzahl der SystemField-Types vom SwDoc
#define INIT_FLDTYPES	32
// Anzahl der vordefinierten Seq-Feldtypen. Es handelt sich dabei
// immer um die letzen Felder vor INIT_FLDTYPES
#define INIT_SEQ_FLDTYPES	4

//Die ehemaligen Rendevouz-Ids leben weiter:
//Es gibt Ids fuer die Anker (SwFmtAnchor) und ein paar weitere die nur fuer
//Schnittstellen Bedeutung haben (SwDoc).
enum RndStdIds
{
	FLY_AT_CNTNT,		//Absatzgebundener Rahmen
	FLY_IN_CNTNT,		//Zeichengebundener Rahmen
	FLY_PAGE,			//Seitengebundener Rahmen
	FLY_AT_FLY,			//Rahmengebundener Rahmen ( LAYER_IMPL )
	FLY_AUTO_CNTNT,		//Automatisch positionierter, absatzgebundener Rahmen
						//Der Rest wird nur fuer SS benutzt.
	RND_STD_HEADER,
	RND_STD_FOOTER,
	RND_STD_HEADERL,
	RND_STD_HEADERR,
	RND_STD_FOOTERL,
	RND_STD_FOOTERR,

	RND_DRAW_OBJECT		// ein Draw-Object !! nur fuer die SwDoc-Schnittstelle!
};


extern ResMgr* pSwResMgr;			// steht in swapp0.cxx
#define SW_RES(i)		ResId(i,*pSwResMgr)
#define SW_RESSTR(i)	UniString(ResId(i,*pSwResMgr))

com::sun::star::lang::Locale	CreateLocale( LanguageType eLanguage );

::com::sun::star::uno::Reference<
	::com::sun::star::linguistic2::XHyphenator >	GetHyphenator();
::com::sun::star::uno::Reference<
	::com::sun::star::linguistic2::XDictionaryList >	GetDictionaryList();
::com::sun::star::uno::Reference<
	::com::sun::star::beans::XPropertySet >			GetLinguPropertySet();

// reutns the twip size of this graphic
Size GetGraphicSizeTwip( const Graphic&, OutputDevice* pOutDev );


// Seperator fuer Sprunge im Dokument auf verschiedene Inhalttype
const sal_Unicode cMarkSeperator = '|';
extern const sal_Char* __FAR_DATA pMarkToTable;				// Strings stehen
extern const sal_Char* __FAR_DATA pMarkToFrame;             // im Init.cxx
extern const sal_Char* __FAR_DATA pMarkToRegion;
extern const sal_Char* __FAR_DATA pMarkToOutline;
extern const sal_Char* __FAR_DATA pMarkToText;
extern const sal_Char* __FAR_DATA pMarkToGraphic;
extern const sal_Char* __FAR_DATA pMarkToOLE;

#ifndef DB_DELIM							// Ist in OFA definiert!!!
#define DB_DELIM ((sal_Unicode)0xff)		// Datenbank <-> Tabellen-Trenner
#endif


enum SetAttrMode
{
	SETATTR_DEFAULT			= 0x0000,	// default
	SETATTR_DONTEXPAND 		= 0x0001,	// TextAttribute nicht weiter expand.
	SETATTR_DONTREPLACE		= 0x0002,	// kein anderes TextAttrib verdraengen

	SETATTR_NOTXTATRCHR		= 0x0004,	// bei Attr ohne Ende kein 0xFF einfuegen
	SETATTR_NOHINTADJUST	= 0x0008, 	// keine Zusammenfassung von Bereichen.
	SETATTR_NOFORMATATTR	= 0x0010,	// nicht zum FormatAttribut umwandeln
	SETATTR_DONTCHGNUMRULE  = 0x0020, 	// nicht die NumRule veraendern
	SETATTR_APICALL			= 0x0040	// called from API (all UI related
										// functionality will be disabled)
};

//Umrechnung Twip<-> 1/100 mm fuer UNO

#define TWIP_TO_MM100(TWIP) 	((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)	((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))

#define SW_ISPRINTABLE( c ) ( c >= ' ' && 127 != c )

#ifndef SW_CONSTASCII_DECL
#define SW_CONSTASCII_DECL( n, s ) n[sizeof(s)]
#endif
#ifndef SW_CONSTASCII_DEF
#define SW_CONSTASCII_DEF( n, s ) n[sizeof(s)] = s
#endif


#define CHAR_HARDBLANK		((sal_Unicode)0x00A0)
#define CHAR_HARDHYPHEN		((sal_Unicode)0x2011)
#define CHAR_SOFTHYPHEN		((sal_Unicode)0x00AD)

// returns the APP - CharClass instance - used for all ToUpper/ToLower/...
CharClass& GetAppCharClass();
LocaleDataWrapper& GetAppLocaleData();

ULONG GetAppLanguage();


#if 0
// I18N doesn't get this right, can't specify more than one to ignore
#define SW_COLLATOR_IGNORES ( \
	::com::sun::star::i18n::CollatorOptions::CollatorOptions_IGNORE_CASE | \
	::com::sun::star::i18n::CollatorOptions::CollatorOptions_IGNORE_KANA | \
	::com::sun::star::i18n::CollatorOptions::CollatorOptions_IGNORE_WIDTH )
#else
#define SW_COLLATOR_IGNORES ( \
	::com::sun::star::i18n::CollatorOptions::CollatorOptions_IGNORE_CASE )
#endif

CollatorWrapper& GetAppCollator();
CollatorWrapper& GetAppCaseCollator();

const ::utl::TransliterationWrapper& GetAppCmpStrIgnore();


} //namespace binfilter
#endif
