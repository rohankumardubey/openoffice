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



#include <bf_svtools/syslocale.hxx>

#define _ZFORLIST_DECLARE_TABLE

#include <bf_svtools/zformat.hxx>
#include <bf_svtools/numuno.hxx>
#include <rtl/math.hxx>
#include <i18npool/mslangid.hxx>
#include <tools/debug.hxx>

// #110680#
//#include <comphelper/processfactory.hxx>

#include "xmlnumfi.hxx"
#include "xmlnmspe.hxx"
#include "xmlimp.hxx"
#include "xmluconv.hxx"
#include "nmspmap.hxx"

namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

//-------------------------------------------------------------------------

struct SvXMLNumFmtEntry
{
	::rtl::OUString	aName;
	sal_uInt32		nKey;
	sal_Bool		bRemoveAfterUse;

	SvXMLNumFmtEntry( const ::rtl::OUString& rN, sal_uInt32 nK, sal_Bool bR ) :
		aName(rN), nKey(nK), bRemoveAfterUse(bR) {}
};

typedef SvXMLNumFmtEntry* SvXMLNumFmtEntryPtr;
SV_DECL_PTRARR_DEL( SvXMLNumFmtEntryArr, SvXMLNumFmtEntryPtr, 4, 4 )//STRIP007;

struct SvXMLEmbeddedElement
{
	sal_Int32		nFormatPos;
	::rtl::OUString	aText;

	SvXMLEmbeddedElement( sal_Int32 nFP, const ::rtl::OUString& rT ) :
		nFormatPos(nFP), aText(rT) {}

	//	comparison operators for PTRARR sorting - sorted by position
	BOOL operator ==( const SvXMLEmbeddedElement& r ) const	{ return nFormatPos == r.nFormatPos; }
	BOOL operator < ( const SvXMLEmbeddedElement& r ) const	{ return nFormatPos <  r.nFormatPos; }
};

typedef SvXMLEmbeddedElement* SvXMLEmbeddedElementPtr;
SV_DECL_PTRARR_SORT_DEL( SvXMLEmbeddedElementArr, SvXMLEmbeddedElementPtr, 0, 4 )//STRIP007 ;

//-------------------------------------------------------------------------

class SvXMLNumImpData
{
	SvNumberFormatter*	pFormatter;
	SvXMLTokenMap*		pStylesElemTokenMap;
	SvXMLTokenMap*		pStyleElemTokenMap;
	SvXMLTokenMap*		pStyleAttrTokenMap;
	SvXMLTokenMap*		pStyleElemAttrTokenMap;
	LocaleDataWrapper*	pLocaleData;
	SvXMLNumFmtEntryArr	aNameEntries;

	// #110680#
	::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxServiceFactory;

public:
	// #110680#
	// SvXMLNumImpData( SvNumberFormatter* pFmt );
	SvXMLNumImpData( 
		SvNumberFormatter* pFmt, 
		const uno::Reference<lang::XMultiServiceFactory>& xServiceFactory );
	~SvXMLNumImpData();

	SvNumberFormatter*		GetNumberFormatter() const	{ return pFormatter; }
	const SvXMLTokenMap&	GetStylesElemTokenMap();
	const SvXMLTokenMap&	GetStyleElemTokenMap();
	const SvXMLTokenMap&	GetStyleAttrTokenMap();
	const SvXMLTokenMap&	GetStyleElemAttrTokenMap();
	const LocaleDataWrapper&	GetLocaleData( LanguageType nLang );
	sal_uInt32				GetKeyForName( const ::rtl::OUString& rName );
	void					AddKey( sal_uInt32 nKey, const ::rtl::OUString& rName, sal_Bool bRemoveAfterUse );
	void					SetUsed( sal_uInt32 nKey );
	void					RemoveVolatileFormats();
};


struct SvXMLNumberInfo
{
	sal_Int32	nDecimals;
	sal_Int32	nInteger;
	sal_Int32	nExpDigits;
	sal_Int32	nNumerDigits;
	sal_Int32	nDenomDigits;
	sal_Bool	bGrouping;
	sal_Bool	bDecReplace;
	sal_Bool	bVarDecimals;
	double		fDisplayFactor;
	SvXMLEmbeddedElementArr	aEmbeddedElements;

	SvXMLNumberInfo()
	{
		nDecimals = nInteger = nExpDigits = nNumerDigits = nDenomDigits = -1;
		bGrouping = bDecReplace = bVarDecimals = sal_False;
		fDisplayFactor = 1.0;
	}
};

class SvXMLNumFmtElementContext : public SvXMLImportContext
{
	SvXMLNumFormatContext&	rParent;
	sal_uInt16				nType;
	::rtl::OUStringBuffer		aContent;
	SvXMLNumberInfo			aNumInfo;
	LanguageType			nElementLang;
	sal_Bool				bLong;
	sal_Bool				bTextual;
	::rtl::OUString			sCalendar;

public:
				SvXMLNumFmtElementContext( SvXMLImport& rImport, USHORT nPrfx,
									const ::rtl::OUString& rLName,
									SvXMLNumFormatContext& rParentContext, sal_uInt16 nNewType,
									const ::com::sun::star::uno::Reference<
										::com::sun::star::xml::sax::XAttributeList>& xAttrList );
	virtual		~SvXMLNumFmtElementContext();

	virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
									const ::rtl::OUString& rLocalName,
									const ::com::sun::star::uno::Reference<
									  	::com::sun::star::xml::sax::XAttributeList>& xAttrList );
	virtual void Characters( const ::rtl::OUString& rChars );
	virtual void EndElement();

	void	AddEmbeddedElement( sal_Int32 nFormatPos, const ::rtl::OUString& rContent );
};


class SvXMLNumFmtEmbeddedTextContext : public SvXMLImportContext
{
	SvXMLNumFmtElementContext&	rParent;
	::rtl::OUStringBuffer			aContent;
	sal_Int32					nTextPosition;

public:
				SvXMLNumFmtEmbeddedTextContext( SvXMLImport& rImport, USHORT nPrfx,
									const ::rtl::OUString& rLName,
									SvXMLNumFmtElementContext& rParentContext,
									const ::com::sun::star::uno::Reference<
										::com::sun::star::xml::sax::XAttributeList>& xAttrList );
	virtual		~SvXMLNumFmtEmbeddedTextContext();

	virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
									const ::rtl::OUString& rLocalName,
									const ::com::sun::star::uno::Reference<
									  	::com::sun::star::xml::sax::XAttributeList>& xAttrList );
	virtual void Characters( const ::rtl::OUString& rChars );
	virtual void EndElement();
};


class SvXMLNumFmtMapContext : public SvXMLImportContext
{
	SvXMLNumFormatContext&	rParent;
	::rtl::OUString			sCondition;
	::rtl::OUString			sName;

public:
				SvXMLNumFmtMapContext( SvXMLImport& rImport, USHORT nPrfx,
									const ::rtl::OUString& rLName,
									SvXMLNumFormatContext& rParentContext,
									const ::com::sun::star::uno::Reference<
										::com::sun::star::xml::sax::XAttributeList>& xAttrList );
	virtual		~SvXMLNumFmtMapContext();

	virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
									const ::rtl::OUString& rLocalName,
									const ::com::sun::star::uno::Reference<
									  	::com::sun::star::xml::sax::XAttributeList>& xAttrList );
	virtual void Characters( const ::rtl::OUString& rChars );
	virtual void EndElement();
};


class SvXMLNumFmtPropContext : public SvXMLImportContext
{
	SvXMLNumFormatContext&	rParent;
	Color					aColor;
	sal_Bool				bColSet;

public:
				SvXMLNumFmtPropContext( SvXMLImport& rImport, USHORT nPrfx,
									const ::rtl::OUString& rLName,
									SvXMLNumFormatContext& rParentContext,
									const ::com::sun::star::uno::Reference<
										::com::sun::star::xml::sax::XAttributeList>& xAttrList );
	virtual		~SvXMLNumFmtPropContext();

	virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
									const ::rtl::OUString& rLocalName,
									const ::com::sun::star::uno::Reference<
									  	::com::sun::star::xml::sax::XAttributeList>& xAttrList );
	virtual void Characters( const ::rtl::OUString& rChars );
	virtual void EndElement();
};


//-------------------------------------------------------------------------

enum SvXMLStyleTokens
{
	XML_TOK_STYLE_TEXT,
	XML_TOK_STYLE_NUMBER,
	XML_TOK_STYLE_SCIENTIFIC_NUMBER,
	XML_TOK_STYLE_FRACTION,
	XML_TOK_STYLE_CURRENCY_SYMBOL,
	XML_TOK_STYLE_DAY,
	XML_TOK_STYLE_MONTH,
	XML_TOK_STYLE_YEAR,
	XML_TOK_STYLE_ERA,
	XML_TOK_STYLE_DAY_OF_WEEK,
	XML_TOK_STYLE_WEEK_OF_YEAR,
	XML_TOK_STYLE_QUARTER,
	XML_TOK_STYLE_HOURS,
	XML_TOK_STYLE_AM_PM,
	XML_TOK_STYLE_MINUTES,
	XML_TOK_STYLE_SECONDS,
	XML_TOK_STYLE_BOOLEAN,
	XML_TOK_STYLE_TEXT_CONTENT,
	XML_TOK_STYLE_PROPERTIES,
	XML_TOK_STYLE_MAP
};

enum SvXMLStyleAttrTokens
{
	XML_TOK_STYLE_ATTR_NAME,
	XML_TOK_STYLE_ATTR_LANGUAGE,
	XML_TOK_STYLE_ATTR_COUNTRY,
	XML_TOK_STYLE_ATTR_TITLE,
	XML_TOK_STYLE_ATTR_AUTOMATIC_ORDER,
	XML_TOK_STYLE_ATTR_FORMAT_SOURCE,
	XML_TOK_STYLE_ATTR_TRUNCATE_ON_OVERFLOW,
	XML_TOK_STYLE_ATTR_VOLATILE,
    XML_TOK_STYLE_ATTR_TRANSL_FORMAT,
    XML_TOK_STYLE_ATTR_TRANSL_LANGUAGE,
    XML_TOK_STYLE_ATTR_TRANSL_COUNTRY,
    XML_TOK_STYLE_ATTR_TRANSL_STYLE
};

enum SvXMLStyleElemAttrTokens
{
	XML_TOK_ELEM_ATTR_DECIMAL_PLACES,
	XML_TOK_ELEM_ATTR_MIN_INTEGER_DIGITS,
	XML_TOK_ELEM_ATTR_GROUPING,
	XML_TOK_ELEM_ATTR_DISPLAY_FACTOR,
	XML_TOK_ELEM_ATTR_DECIMAL_REPLACEMENT,
	XML_TOK_ELEM_ATTR_MIN_EXPONENT_DIGITS,
	XML_TOK_ELEM_ATTR_MIN_NUMERATOR_DIGITS,
	XML_TOK_ELEM_ATTR_MIN_DENOMINATOR_DIGITS,
	XML_TOK_ELEM_ATTR_LANGUAGE,
	XML_TOK_ELEM_ATTR_COUNTRY,
	XML_TOK_ELEM_ATTR_STYLE,
	XML_TOK_ELEM_ATTR_TEXTUAL,
	XML_TOK_ELEM_ATTR_CALENDAR
};

//-------------------------------------------------------------------------

//
//	standard colors
//

#define XML_NUMF_COLORCOUNT		10

static ColorData aNumFmtStdColors[XML_NUMF_COLORCOUNT] =
{
	COL_BLACK,
	COL_LIGHTBLUE,
	COL_LIGHTGREEN,
	COL_LIGHTCYAN,
	COL_LIGHTRED,
	COL_LIGHTMAGENTA,
	COL_BROWN,
	COL_GRAY,
	COL_YELLOW,
	COL_WHITE
};

//
//	token maps
//

static __FAR_DATA SvXMLTokenMapEntry aStylesElemMap[] =
{
	//	style elements
	{ XML_NAMESPACE_NUMBER, XML_NUMBER_STYLE, 	   XML_TOK_STYLES_NUMBER_STYLE		},
	{ XML_NAMESPACE_NUMBER, XML_CURRENCY_STYLE,    XML_TOK_STYLES_CURRENCY_STYLE	},
	{ XML_NAMESPACE_NUMBER, XML_PERCENTAGE_STYLE,  XML_TOK_STYLES_PERCENTAGE_STYLE	},
	{ XML_NAMESPACE_NUMBER, XML_DATE_STYLE, 	   XML_TOK_STYLES_DATE_STYLE		},
	{ XML_NAMESPACE_NUMBER, XML_TIME_STYLE, 	   XML_TOK_STYLES_TIME_STYLE		},
	{ XML_NAMESPACE_NUMBER, XML_BOOLEAN_STYLE,     XML_TOK_STYLES_BOOLEAN_STYLE		},
	{ XML_NAMESPACE_NUMBER, XML_TEXT_STYLE, 	   XML_TOK_STYLES_TEXT_STYLE		},
	XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aStyleElemMap[] =
{
	//	elements in a style
	{ XML_NAMESPACE_NUMBER, XML_TEXT,				XML_TOK_STYLE_TEXT				},
	{ XML_NAMESPACE_NUMBER, XML_NUMBER,		 	    XML_TOK_STYLE_NUMBER			},
	{ XML_NAMESPACE_NUMBER, XML_SCIENTIFIC_NUMBER,	XML_TOK_STYLE_SCIENTIFIC_NUMBER	},
	{ XML_NAMESPACE_NUMBER, XML_FRACTION,			XML_TOK_STYLE_FRACTION			},
	{ XML_NAMESPACE_NUMBER, XML_CURRENCY_SYMBOL,	XML_TOK_STYLE_CURRENCY_SYMBOL	},
	{ XML_NAMESPACE_NUMBER, XML_DAY,				XML_TOK_STYLE_DAY				},
	{ XML_NAMESPACE_NUMBER, XML_MONTH,				XML_TOK_STYLE_MONTH				},
	{ XML_NAMESPACE_NUMBER, XML_YEAR,				XML_TOK_STYLE_YEAR				},
	{ XML_NAMESPACE_NUMBER, XML_ERA,				XML_TOK_STYLE_ERA				},
	{ XML_NAMESPACE_NUMBER, XML_DAY_OF_WEEK,		XML_TOK_STYLE_DAY_OF_WEEK		},
	{ XML_NAMESPACE_NUMBER, XML_WEEK_OF_YEAR,		XML_TOK_STYLE_WEEK_OF_YEAR		},
	{ XML_NAMESPACE_NUMBER, XML_QUARTER,			XML_TOK_STYLE_QUARTER			},
	{ XML_NAMESPACE_NUMBER, XML_HOURS,				XML_TOK_STYLE_HOURS				},
	{ XML_NAMESPACE_NUMBER, XML_AM_PM,				XML_TOK_STYLE_AM_PM				},
	{ XML_NAMESPACE_NUMBER, XML_MINUTES,			XML_TOK_STYLE_MINUTES			},
	{ XML_NAMESPACE_NUMBER, XML_SECONDS,			XML_TOK_STYLE_SECONDS			},
	{ XML_NAMESPACE_NUMBER, XML_BOOLEAN,			XML_TOK_STYLE_BOOLEAN			},
	{ XML_NAMESPACE_NUMBER, XML_TEXT_CONTENT,		XML_TOK_STYLE_TEXT_CONTENT		},
	{ XML_NAMESPACE_STYLE,  XML_PROPERTIES,		    XML_TOK_STYLE_PROPERTIES		},
	{ XML_NAMESPACE_STYLE,  XML_MAP,				XML_TOK_STYLE_MAP				},
	XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aStyleAttrMap[] =
{
	//	attributes for a style
	{ XML_NAMESPACE_STYLE,  XML_NAME,			 	   XML_TOK_STYLE_ATTR_NAME					},
	{ XML_NAMESPACE_NUMBER, XML_LANGUAGE,		 	   XML_TOK_STYLE_ATTR_LANGUAGE				},
	{ XML_NAMESPACE_NUMBER, XML_COUNTRY,		 	   XML_TOK_STYLE_ATTR_COUNTRY				},
	{ XML_NAMESPACE_NUMBER, XML_TITLE,			 	   XML_TOK_STYLE_ATTR_TITLE					},
	{ XML_NAMESPACE_NUMBER, XML_AUTOMATIC_ORDER, 	   XML_TOK_STYLE_ATTR_AUTOMATIC_ORDER		},
	{ XML_NAMESPACE_NUMBER, XML_FORMAT_SOURCE, 	       XML_TOK_STYLE_ATTR_FORMAT_SOURCE			},
	{ XML_NAMESPACE_NUMBER, XML_TRUNCATE_ON_OVERFLOW,  XML_TOK_STYLE_ATTR_TRUNCATE_ON_OVERFLOW	},
	{ XML_NAMESPACE_STYLE,  XML_VOLATILE,		 	   XML_TOK_STYLE_ATTR_VOLATILE				},
    { XML_NAMESPACE_NUMBER, XML_TRANSLITERATION_FORMAT,     XML_TOK_STYLE_ATTR_TRANSL_FORMAT    },
    { XML_NAMESPACE_NUMBER, XML_TRANSLITERATION_LANGUAGE,   XML_TOK_STYLE_ATTR_TRANSL_LANGUAGE  },
    { XML_NAMESPACE_NUMBER, XML_TRANSLITERATION_COUNTRY,    XML_TOK_STYLE_ATTR_TRANSL_COUNTRY   },
    { XML_NAMESPACE_NUMBER, XML_TRANSLITERATION_STYLE,      XML_TOK_STYLE_ATTR_TRANSL_STYLE     },
	XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aStyleElemAttrMap[] =
{
	//	attributes for an element within a style
	{ XML_NAMESPACE_NUMBER, XML_DECIMAL_PLACES,		     XML_TOK_ELEM_ATTR_DECIMAL_PLACES		},
	{ XML_NAMESPACE_NUMBER, XML_MIN_INTEGER_DIGITS,      XML_TOK_ELEM_ATTR_MIN_INTEGER_DIGITS	},
	{ XML_NAMESPACE_NUMBER, XML_GROUPING,			 	 XML_TOK_ELEM_ATTR_GROUPING				},
	{ XML_NAMESPACE_NUMBER, XML_DISPLAY_FACTOR,		 	 XML_TOK_ELEM_ATTR_DISPLAY_FACTOR		},
	{ XML_NAMESPACE_NUMBER, XML_DECIMAL_REPLACEMENT,     XML_TOK_ELEM_ATTR_DECIMAL_REPLACEMENT	},
	{ XML_NAMESPACE_NUMBER, XML_MIN_EXPONENT_DIGITS,     XML_TOK_ELEM_ATTR_MIN_EXPONENT_DIGITS	},
	{ XML_NAMESPACE_NUMBER, XML_MIN_NUMERATOR_DIGITS,    XML_TOK_ELEM_ATTR_MIN_NUMERATOR_DIGITS	},
	{ XML_NAMESPACE_NUMBER, XML_MIN_DENOMINATOR_DIGITS,  XML_TOK_ELEM_ATTR_MIN_DENOMINATOR_DIGITS },
	{ XML_NAMESPACE_NUMBER, XML_LANGUAGE,			 	 XML_TOK_ELEM_ATTR_LANGUAGE				},
	{ XML_NAMESPACE_NUMBER, XML_COUNTRY,			 	 XML_TOK_ELEM_ATTR_COUNTRY				},
	{ XML_NAMESPACE_NUMBER, XML_STYLE,				 	 XML_TOK_ELEM_ATTR_STYLE				},
	{ XML_NAMESPACE_NUMBER, XML_TEXTUAL,			 	 XML_TOK_ELEM_ATTR_TEXTUAL				},
	{ XML_NAMESPACE_NUMBER, XML_CALENDAR,			 	 XML_TOK_ELEM_ATTR_CALENDAR				},
	XML_TOKEN_MAP_END
};

// maps for SvXMLUnitConverter::convertEnum

static __FAR_DATA SvXMLEnumMapEntry aStyleValueMap[] =
{
	{ XML_SHORT,            sal_False	},
	{ XML_LONG,             sal_True	},
	{ XML_TOKEN_INVALID,    0 }
};

static __FAR_DATA SvXMLEnumMapEntry aFormatSourceMap[] =
{
	{ XML_FIXED,	        sal_False },
	{ XML_LANGUAGE,         sal_True  },
	{ XML_TOKEN_INVALID,    0 }
};

//-------------------------------------------------------------------------

struct SvXMLDefaultDateFormat
{
	NfIndexTableOffset			eFormat;
	SvXMLDateElementAttributes	eDOW;
	SvXMLDateElementAttributes	eDay;
	SvXMLDateElementAttributes	eMonth;
	SvXMLDateElementAttributes	eYear;
	SvXMLDateElementAttributes	eHours;
	SvXMLDateElementAttributes	eMins;
	SvXMLDateElementAttributes	eSecs;
	sal_Bool					bSystem;
};

static __FAR_DATA SvXMLDefaultDateFormat aDefaultDateFormats[] =
{
	// format							day-of-week		day				month				year			hours			minutes			seconds			format-source

	{ NF_DATE_SYSTEM_SHORT,				XML_DEA_NONE,	XML_DEA_ANY,	XML_DEA_ANY,		XML_DEA_ANY,	XML_DEA_NONE,	XML_DEA_NONE,	XML_DEA_NONE,	sal_True },
	{ NF_DATE_SYSTEM_LONG,				XML_DEA_ANY,	XML_DEA_ANY,	XML_DEA_ANY,		XML_DEA_ANY,	XML_DEA_NONE,	XML_DEA_NONE,	XML_DEA_NONE,	sal_True },
	{ NF_DATE_SYS_MMYY,					XML_DEA_NONE,	XML_DEA_NONE,	XML_DEA_LONG,		XML_DEA_SHORT,	XML_DEA_NONE,	XML_DEA_NONE,	XML_DEA_NONE,	sal_False },
	{ NF_DATE_SYS_DDMMM,				XML_DEA_NONE,	XML_DEA_LONG,	XML_DEA_TEXTSHORT,	XML_DEA_NONE,	XML_DEA_NONE,	XML_DEA_NONE,	XML_DEA_NONE,	sal_False },
	{ NF_DATE_SYS_DDMMYYYY,				XML_DEA_NONE,	XML_DEA_LONG,	XML_DEA_LONG,		XML_DEA_LONG,	XML_DEA_NONE,	XML_DEA_NONE,	XML_DEA_NONE,	sal_False },
	{ NF_DATE_SYS_DDMMYY,				XML_DEA_NONE,	XML_DEA_LONG,	XML_DEA_LONG,		XML_DEA_SHORT,	XML_DEA_NONE,	XML_DEA_NONE,	XML_DEA_NONE,	sal_False },
	{ NF_DATE_SYS_DMMMYY,				XML_DEA_NONE,	XML_DEA_SHORT,	XML_DEA_TEXTSHORT,	XML_DEA_SHORT,	XML_DEA_NONE,	XML_DEA_NONE,	XML_DEA_NONE,	sal_False },
	{ NF_DATE_SYS_DMMMYYYY,				XML_DEA_NONE,	XML_DEA_SHORT,	XML_DEA_TEXTSHORT,	XML_DEA_LONG,	XML_DEA_NONE,	XML_DEA_NONE,	XML_DEA_NONE,	sal_False },
	{ NF_DATE_SYS_DMMMMYYYY,			XML_DEA_NONE,	XML_DEA_SHORT,	XML_DEA_TEXTLONG,	XML_DEA_LONG,	XML_DEA_NONE,	XML_DEA_NONE,	XML_DEA_NONE,	sal_False },
	{ NF_DATE_SYS_NNDMMMYY,				XML_DEA_SHORT,	XML_DEA_SHORT,	XML_DEA_TEXTSHORT,	XML_DEA_SHORT,	XML_DEA_NONE,	XML_DEA_NONE,	XML_DEA_NONE,	sal_False },
	{ NF_DATE_SYS_NNDMMMMYYYY,			XML_DEA_SHORT,	XML_DEA_SHORT,	XML_DEA_TEXTLONG,	XML_DEA_LONG,	XML_DEA_NONE,	XML_DEA_NONE,	XML_DEA_NONE,	sal_False },
	{ NF_DATE_SYS_NNNNDMMMMYYYY,		XML_DEA_LONG,	XML_DEA_SHORT,	XML_DEA_TEXTLONG,	XML_DEA_LONG,	XML_DEA_NONE,	XML_DEA_NONE,	XML_DEA_NONE,	sal_False },
	{ NF_DATETIME_SYSTEM_SHORT_HHMM,	XML_DEA_NONE,	XML_DEA_ANY,	XML_DEA_ANY,		XML_DEA_ANY,	XML_DEA_ANY,	XML_DEA_ANY,	XML_DEA_NONE,	sal_True },
	{ NF_DATETIME_SYS_DDMMYYYY_HHMMSS,	XML_DEA_NONE,	XML_DEA_ANY,	XML_DEA_ANY,		XML_DEA_ANY,	XML_DEA_ANY,	XML_DEA_ANY,	XML_DEA_ANY,	sal_False }
};

//-------------------------------------------------------------------------

SV_IMPL_PTRARR( SvXMLNumFmtEntryArr, SvXMLNumFmtEntryPtr );
SV_IMPL_OP_PTRARR_SORT( SvXMLEmbeddedElementArr, SvXMLEmbeddedElementPtr );

//-------------------------------------------------------------------------

//
//	SvXMLNumImpData
//

// #110680#
// SvXMLNumImpData::SvXMLNumImpData( SvNumberFormatter* pFmt ) :
SvXMLNumImpData::SvXMLNumImpData( 
	SvNumberFormatter* pFmt, 
	const uno::Reference<lang::XMultiServiceFactory>& xServiceFactory ) 
:	pFormatter(pFmt),
	pStylesElemTokenMap(NULL),
	pStyleElemTokenMap(NULL),
	pStyleAttrTokenMap(NULL),
	pStyleElemAttrTokenMap(NULL),
	pLocaleData(NULL),

	// #110680#
	mxServiceFactory(xServiceFactory)
{
	DBG_ASSERT( mxServiceFactory.is(), "got no service manager" );
}

SvXMLNumImpData::~SvXMLNumImpData()
{
	delete pStylesElemTokenMap;
	delete pStyleElemTokenMap;
	delete pStyleAttrTokenMap;
	delete pStyleElemAttrTokenMap;
	delete pLocaleData;
}

sal_uInt32 SvXMLNumImpData::GetKeyForName( const ::rtl::OUString& rName )
{
	USHORT nCount = aNameEntries.Count();
	for (USHORT i=0; i<nCount; i++)
	{
		const SvXMLNumFmtEntry* pObj = aNameEntries[i];
		if ( pObj->aName == rName )
			return pObj->nKey;				// found
	}
	return NUMBERFORMAT_ENTRY_NOT_FOUND;
}

void SvXMLNumImpData::AddKey( sal_uInt32 nKey, const ::rtl::OUString& rName, sal_Bool bRemoveAfterUse )
{
	if ( bRemoveAfterUse )
	{
		//	if there is already an entry for this key without the bRemoveAfterUse flag,
		//	clear the flag for this entry, too

		USHORT nCount = aNameEntries.Count();
		for (USHORT i=0; i<nCount; i++)
		{
			SvXMLNumFmtEntry* pObj = aNameEntries[i];
			if ( pObj->nKey == nKey && !pObj->bRemoveAfterUse )
			{
				bRemoveAfterUse = sal_False;		// clear flag for new entry
				break;
			}
		}
	}
	else
	{
		//	call SetUsed to clear the bRemoveAfterUse flag for other entries for this key
		SetUsed( nKey );
	}

	SvXMLNumFmtEntry* pObj = new SvXMLNumFmtEntry( rName, nKey, bRemoveAfterUse );
	aNameEntries.Insert( pObj, aNameEntries.Count() );
}

void SvXMLNumImpData::SetUsed( sal_uInt32 nKey )
{
	USHORT nCount = aNameEntries.Count();
	for (USHORT i=0; i<nCount; i++)
	{
		SvXMLNumFmtEntry* pObj = aNameEntries[i];
		if ( pObj->nKey == nKey )
		{
			pObj->bRemoveAfterUse = sal_False;		// used -> don't remove

			//	continue searching - there may be several entries for the same key
			//	(with different names), the format must not be deleted if any one of
			//	them is used
		}
	}
}

void SvXMLNumImpData::RemoveVolatileFormats()
{
	//	remove temporary (volatile) formats from NumberFormatter
	//	called at the end of each import (styles and content), so volatile formats
	//	from styles can't be used in content

	if ( !pFormatter )
		return;

	USHORT nCount = aNameEntries.Count();
	for (USHORT i=0; i<nCount; i++)
	{
		const SvXMLNumFmtEntry* pObj = aNameEntries[i];
		if ( pObj->bRemoveAfterUse )
        {
            const SvNumberformat* pFormat = pFormatter->GetEntry(pObj->nKey);
            if (pFormat && (pFormat->GetType() & NUMBERFORMAT_DEFINED))
			    pFormatter->DeleteEntry( pObj->nKey );
        }
	}
}

const SvXMLTokenMap& SvXMLNumImpData::GetStylesElemTokenMap()
{
	if( !pStylesElemTokenMap )
		pStylesElemTokenMap = new SvXMLTokenMap( aStylesElemMap );
	return *pStylesElemTokenMap;
}

const SvXMLTokenMap& SvXMLNumImpData::GetStyleElemTokenMap()
{
	if( !pStyleElemTokenMap )
		pStyleElemTokenMap = new SvXMLTokenMap( aStyleElemMap );
	return *pStyleElemTokenMap;
}

const SvXMLTokenMap& SvXMLNumImpData::GetStyleAttrTokenMap()
{
	if( !pStyleAttrTokenMap )
		pStyleAttrTokenMap = new SvXMLTokenMap( aStyleAttrMap );
	return *pStyleAttrTokenMap;
}

const SvXMLTokenMap& SvXMLNumImpData::GetStyleElemAttrTokenMap()
{
	if( !pStyleElemAttrTokenMap )
		pStyleElemAttrTokenMap = new SvXMLTokenMap( aStyleElemAttrMap );
	return *pStyleElemAttrTokenMap;
}

const LocaleDataWrapper& SvXMLNumImpData::GetLocaleData( LanguageType nLang )
{
	if ( !pLocaleData )
		// #110680#
		//pLocaleData = new LocaleDataWrapper(
		//	(pFormatter ? pFormatter->GetServiceManager() :
		//	::comphelper::getProcessServiceFactory()),
		//	MsLangId::convertLanguageToLocale( nLang ) );
		pLocaleData = new LocaleDataWrapper(
			(pFormatter ? pFormatter->GetServiceManager() :
			mxServiceFactory),
			MsLangId::convertLanguageToLocale( nLang ) );
	else
		pLocaleData->setLocale( MsLangId::convertLanguageToLocale( nLang ) );
	return *pLocaleData;
}

//-------------------------------------------------------------------------

//
//	SvXMLNumFmtMapContext
//

SvXMLNumFmtMapContext::SvXMLNumFmtMapContext( SvXMLImport& rImport,
									USHORT nPrfx, const ::rtl::OUString& rLName,
									SvXMLNumFormatContext& rParentContext,
									const uno::Reference<xml::sax::XAttributeList>& xAttrList ) :
	SvXMLImportContext( rImport, nPrfx, rLName ),
	rParent( rParentContext )
{
	sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
	for( sal_Int16 i=0; i < nAttrCount; i++ )
	{
		OUString sAttrName = xAttrList->getNameByIndex( i );
		OUString sValue = xAttrList->getValueByIndex( i );
		OUString aLocalName;
		sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
		if ( nPrefix == XML_NAMESPACE_STYLE )
		{
			if ( IsXMLToken( aLocalName, XML_CONDITION) )
				sCondition = sValue;
			else if ( IsXMLToken( aLocalName, XML_APPLY_STYLE_NAME) )
				sName = sValue;
		}
	}
}

SvXMLNumFmtMapContext::~SvXMLNumFmtMapContext()
{
}

SvXMLImportContext* SvXMLNumFmtMapContext::CreateChildContext(
									USHORT nPrfx, const ::rtl::OUString& rLName,
									const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
	// no elements supported - use default context
	return new SvXMLImportContext( GetImport(), nPrfx, rLName );
}

void SvXMLNumFmtMapContext::Characters( const ::rtl::OUString& rChars )
{
}

void SvXMLNumFmtMapContext::EndElement()
{
	rParent.AddCondition( sCondition, sName );
}

//-------------------------------------------------------------------------

//
//	SvXMLNumFmtPropContext
//

SvXMLNumFmtPropContext::SvXMLNumFmtPropContext( SvXMLImport& rImport,
									USHORT nPrfx, const ::rtl::OUString& rLName,
									SvXMLNumFormatContext& rParentContext,
									const uno::Reference<xml::sax::XAttributeList>& xAttrList ) :
	SvXMLImportContext( rImport, nPrfx, rLName ),
	rParent( rParentContext ),
	bColSet( sal_False )
{
	sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
	for( sal_Int16 i=0; i < nAttrCount; i++ )
	{
		OUString sAttrName = xAttrList->getNameByIndex( i );
		OUString sValue = xAttrList->getValueByIndex( i );
		OUString aLocalName;
		sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
		if ( nPrefix == XML_NAMESPACE_FO && IsXMLToken( aLocalName, XML_COLOR ) )
			bColSet = SvXMLUnitConverter::convertColor( aColor, sValue );
	}
}

SvXMLNumFmtPropContext::~SvXMLNumFmtPropContext()
{
}

SvXMLImportContext* SvXMLNumFmtPropContext::CreateChildContext(
									USHORT nPrfx, const ::rtl::OUString& rLName,
									const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
	// no elements supported - use default context
	return new SvXMLImportContext( GetImport(), nPrfx, rLName );
}

void SvXMLNumFmtPropContext::Characters( const ::rtl::OUString& rChars )
{
}

void SvXMLNumFmtPropContext::EndElement()
{
	if (bColSet)
		rParent.AddColor( aColor );
}

//-------------------------------------------------------------------------

//
//	SvXMLNumFmtEmbeddedTextContext
//

SvXMLNumFmtEmbeddedTextContext::SvXMLNumFmtEmbeddedTextContext( SvXMLImport& rImport,
									USHORT nPrfx, const ::rtl::OUString& rLName,
									SvXMLNumFmtElementContext& rParentContext,
									const uno::Reference<xml::sax::XAttributeList>& xAttrList ) :
	SvXMLImportContext( rImport, nPrfx, rLName ),
	rParent( rParentContext ),
	nTextPosition( 0 )
{
	sal_Int32 nAttrVal;

	sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
	for( sal_Int16 i=0; i < nAttrCount; i++ )
	{
		OUString sAttrName = xAttrList->getNameByIndex( i );
		OUString sValue = xAttrList->getValueByIndex( i );
		OUString aLocalName;
		sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
		if ( nPrefix == XML_NAMESPACE_NUMBER && IsXMLToken( aLocalName, XML_POSITION ) )
		{
			if ( SvXMLUnitConverter::convertNumber( nAttrVal, sValue, 0 ) )
				nTextPosition = nAttrVal;
		}
	}
}

SvXMLNumFmtEmbeddedTextContext::~SvXMLNumFmtEmbeddedTextContext()
{
}

SvXMLImportContext* SvXMLNumFmtEmbeddedTextContext::CreateChildContext(
									USHORT nPrfx, const ::rtl::OUString& rLName,
									const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
	// no elements supported - use default context
	return new SvXMLImportContext( GetImport(), nPrfx, rLName );
}

void SvXMLNumFmtEmbeddedTextContext::Characters( const ::rtl::OUString& rChars )
{
	aContent.append( rChars );
}

void SvXMLNumFmtEmbeddedTextContext::EndElement()
{
	rParent.AddEmbeddedElement( nTextPosition, aContent.makeStringAndClear() );
}

//-------------------------------------------------------------------------

sal_Bool lcl_ValidChar( sal_Unicode cChar, sal_uInt16 nFormatType )
{
	//	see ImpSvNumberformatScan::Next_Symbol
	if ( cChar == ' ' ||
		 cChar == '-' ||
		 cChar == '/' ||
		 cChar == '.' ||
		 cChar == ',' ||
		 cChar == ':' ||
		 cChar == '\'' )
		return sal_True;	// for all format types

	//	percent sign must be used without quotes for percentage styles only
	if ( nFormatType == XML_TOK_STYLES_PERCENTAGE_STYLE && cChar == '%' )
		return sal_True;

	//	don't put quotes around single parentheses (often used for negative numbers)
	if ( ( nFormatType == XML_TOK_STYLES_NUMBER_STYLE ||
		   nFormatType == XML_TOK_STYLES_CURRENCY_STYLE ||
		   nFormatType == XML_TOK_STYLES_PERCENTAGE_STYLE ) &&
		 ( cChar == '(' || cChar == ')' ) )
		return sal_True;

	return sal_False;
}

void lcl_EnquoteIfNecessary( ::rtl::OUStringBuffer& rContent, sal_uInt16 nFormatType )
{
	sal_Bool bQuote = sal_True;
	sal_Int32 nLength = rContent.getLength();
	
	if ( ( nLength == 1 &&
			lcl_ValidChar( rContent.charAt(0), nFormatType ) ) ||
		 ( nLength == 2 &&
		 	lcl_ValidChar( rContent.charAt(0), nFormatType ) &&
		 	rContent.charAt(1) == ' ' ) )
	{
		//	don't quote single separator characters like space or percent,
		//	or separator characters followed by space (used in date formats)
		bQuote = sal_False;
	}
	else if ( nFormatType == XML_TOK_STYLES_PERCENTAGE_STYLE && nLength > 1 )
	{
		//	the percent character in percentage styles must be left out of quoting
		//	(one occurence is enough even if there are several percent characters in the string)

		::rtl::OUString aString( rContent.getStr() );
		sal_Int32 nPos = aString.indexOf( (sal_Unicode) '%' );
		if ( nPos >= 0 )
		{
			if ( nPos + 1 < nLength )
			{
				if ( nPos + 2 == nLength && lcl_ValidChar( rContent.charAt(nPos + 1), nFormatType ) )
				{
					//	single character that doesn't need quoting
				}
				else
				{
					//	quote text behind percent character
					rContent.insert( nPos + 1, (sal_Unicode) '"' );
					rContent.append( (sal_Unicode) '"' );
				}
			}
			if ( nPos > 0 )
			{
				if ( nPos == 1 && lcl_ValidChar( rContent.charAt(0), nFormatType ) )
				{
					//	single character that doesn't need quoting
				}
				else
				{
					//	quote text before percent character
					rContent.insert( nPos, (sal_Unicode) '"' );
					rContent.insert( 0, (sal_Unicode) '"' );
				}
			}
			bQuote = sal_False;
		}
		// else: normal quoting (below)
	}

	if ( bQuote )
	{
        // #i55469# quotes in the string itself have to be escaped
        rtl::OUString aString( rContent.getStr() );
        bool bEscape = ( aString.indexOf( (sal_Unicode) '"' ) >= 0 );
        if ( bEscape )
        {
            // A quote is turned into "\"" - a quote to end quoted text, an escaped quote,
            // and a quote to resume quoting.
            rtl::OUString aInsert( rtl::OUString::createFromAscii( "\"\\\"" ) );

            sal_Int32 nPos = 0;
            while ( nPos < rContent.getLength() )
            {
                if ( rContent.charAt( nPos ) == (sal_Unicode) '"' )
                {
                    rContent.insert( nPos, aInsert );
                    nPos += aInsert.getLength();
                }
                ++nPos;
            }
        }

		//	quote string literals
		rContent.insert( 0, (sal_Unicode) '"' );
		rContent.append( (sal_Unicode) '"' );

        // remove redundant double quotes at start or end
        if ( bEscape )
        {
            if ( rContent.getLength() > 2 &&
                 rContent.charAt(0) == (sal_Unicode) '"' &&
                 rContent.charAt(1) == (sal_Unicode) '"' )
            {
                String aTrimmed( rContent.makeStringAndClear().copy(2) );
                rContent = rtl::OUStringBuffer( aTrimmed );
            }

            sal_Int32 nLen = rContent.getLength();
            if ( nLen > 2 &&
                 rContent.charAt(nLen-1) == (sal_Unicode) '"' &&
                 rContent.charAt(nLen-2) == (sal_Unicode) '"' )
            {
                String aTrimmed( rContent.makeStringAndClear().copy( 0, nLen - 2 ) );
                rContent = rtl::OUStringBuffer( aTrimmed );
            }
        }
	}
}

//
//	SvXMLNumFmtElementContext
//

SvXMLNumFmtElementContext::SvXMLNumFmtElementContext( SvXMLImport& rImport,
									USHORT nPrfx, const ::rtl::OUString& rLName,
									SvXMLNumFormatContext& rParentContext, sal_uInt16 nNewType,
									const uno::Reference<xml::sax::XAttributeList>& xAttrList ) :
	SvXMLImportContext( rImport, nPrfx, rLName ),
	rParent( rParentContext ),
	nType( nNewType ),
	nElementLang( LANGUAGE_SYSTEM ),
	bLong( FALSE ),
	bTextual( FALSE )
{
	OUString sLanguage, sCountry;
	sal_Int32 nAttrVal;
	sal_Bool bAttrBool;
	sal_uInt16 nAttrEnum;
	double fAttrDouble;

	sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
	for( sal_Int16 i=0; i < nAttrCount; i++ )
	{
		OUString sAttrName = xAttrList->getNameByIndex( i );
		OUString sValue = xAttrList->getValueByIndex( i );
		OUString aLocalName;
		sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

		const SvXMLTokenMap& rTokenMap = rParent.GetData()->GetStyleElemAttrTokenMap();
		sal_uInt16 nToken = rTokenMap.Get( nPrefix, aLocalName );

		switch (nToken)
		{
			case XML_TOK_ELEM_ATTR_DECIMAL_PLACES:
				if ( SvXMLUnitConverter::convertNumber( nAttrVal, sValue, 0 ) )
					aNumInfo.nDecimals = nAttrVal;
				break;
			case XML_TOK_ELEM_ATTR_MIN_INTEGER_DIGITS:
				if ( SvXMLUnitConverter::convertNumber( nAttrVal, sValue, 0 ) )
					aNumInfo.nInteger = nAttrVal;
				break;
			case XML_TOK_ELEM_ATTR_GROUPING:
				if ( SvXMLUnitConverter::convertBool( bAttrBool, sValue ) )
					aNumInfo.bGrouping = bAttrBool;
				break;
			case XML_TOK_ELEM_ATTR_DISPLAY_FACTOR:
				if ( SvXMLUnitConverter::convertDouble( fAttrDouble, sValue ) )
					aNumInfo.fDisplayFactor = fAttrDouble;
				break;
			case XML_TOK_ELEM_ATTR_DECIMAL_REPLACEMENT:
				if ( sValue.getLength() > 0 )
					aNumInfo.bDecReplace = sal_True;	// only a default string is supported
				else
					aNumInfo.bVarDecimals = sal_True;	// empty replacement string: variable decimals
				break;
			case XML_TOK_ELEM_ATTR_MIN_EXPONENT_DIGITS:
				if ( SvXMLUnitConverter::convertNumber( nAttrVal, sValue, 0 ) )
					aNumInfo.nExpDigits = nAttrVal;
				break;
			case XML_TOK_ELEM_ATTR_MIN_NUMERATOR_DIGITS:
				if ( SvXMLUnitConverter::convertNumber( nAttrVal, sValue, 0 ) )
					aNumInfo.nNumerDigits = nAttrVal;
				break;
			case XML_TOK_ELEM_ATTR_MIN_DENOMINATOR_DIGITS:
				if ( SvXMLUnitConverter::convertNumber( nAttrVal, sValue, 0 ) )
					aNumInfo.nDenomDigits = nAttrVal;
				break;
			case XML_TOK_ELEM_ATTR_LANGUAGE:
				sLanguage = sValue;
				break;
			case XML_TOK_ELEM_ATTR_COUNTRY:
				sCountry = sValue;
				break;
			case XML_TOK_ELEM_ATTR_STYLE:
				if ( SvXMLUnitConverter::convertEnum( nAttrEnum, sValue, aStyleValueMap ) )
					bLong = (sal_Bool) nAttrEnum;
				break;
			case XML_TOK_ELEM_ATTR_TEXTUAL:
				if ( SvXMLUnitConverter::convertBool( bAttrBool, sValue ) )
					bTextual = bAttrBool;
				break;
			case XML_TOK_ELEM_ATTR_CALENDAR:
				sCalendar = sValue;
				break;
		}
	}

	if ( sLanguage.getLength() || sCountry.getLength() )
	{
		nElementLang = MsLangId::convertIsoNamesToLanguage( sLanguage, sCountry );
		if ( nElementLang == LANGUAGE_DONTKNOW )
			nElementLang = LANGUAGE_SYSTEM;			//! error handling for invalid locales?
	}
}

SvXMLNumFmtElementContext::~SvXMLNumFmtElementContext()
{
}

SvXMLImportContext* SvXMLNumFmtElementContext::CreateChildContext(
									USHORT nPrfx, const ::rtl::OUString& rLName,
									const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
	//	only number:number supports number:embedded-text child element

	if ( nType == XML_TOK_STYLE_NUMBER &&
		 nPrfx == XML_NAMESPACE_NUMBER && IsXMLToken( rLName, XML_EMBEDDED_TEXT ) )
	{
		return new SvXMLNumFmtEmbeddedTextContext( GetImport(), nPrfx, rLName, *this, xAttrList );
	}
	else
		return new SvXMLImportContext( GetImport(), nPrfx, rLName );
}

void SvXMLNumFmtElementContext::Characters( const ::rtl::OUString& rChars )
{
	aContent.append( rChars );
}

void SvXMLNumFmtElementContext::AddEmbeddedElement( sal_Int32 nFormatPos, const ::rtl::OUString& rContent )
{
	if ( rContent.getLength() )
	{
		SvXMLEmbeddedElement* pObj = new SvXMLEmbeddedElement( nFormatPos, rContent );
		if ( !aNumInfo.aEmbeddedElements.Insert( pObj ) )
		{
			//	there's already an element at this position - append text to existing element

			delete pObj;
			USHORT nElementCount = aNumInfo.aEmbeddedElements.Count();
			for (USHORT i=0; i<nElementCount; i++)
			{
				pObj = aNumInfo.aEmbeddedElements[i];
				if ( pObj->nFormatPos == nFormatPos )
				{
					pObj->aText += rContent;
					break;
				}
			}
		}
	}
}

void SvXMLNumFmtElementContext::EndElement()
{
	sal_Bool bEffLong = bLong;
	switch (nType)
	{
		case XML_TOK_STYLE_TEXT:
			if ( rParent.HasLongDoW() &&
					rParent.GetLocaleData().getLongDateDayOfWeekSep() ==
						String( aContent.getStr() ) )
			{
				//	skip separator constant after long day of week
				//	(NF_KEY_NNNN contains the separator)

				if ( rParent.ReplaceNfKeyword( NF_KEY_NNN, NF_KEY_NNNN ) )
				{
					//!aContent.setLength(0);		//! doesn't work, #76293#
					aContent = OUStringBuffer();
				}

				rParent.SetHasLongDoW( sal_False );		// only once
			}
			if ( aContent.getLength() )
			{
				lcl_EnquoteIfNecessary( aContent, rParent.GetType() );
				rParent.AddToCode( aContent.makeStringAndClear() );
			}
			break;

		case XML_TOK_STYLE_NUMBER:
			rParent.AddNumber( aNumInfo );
			break;

		case XML_TOK_STYLE_CURRENCY_SYMBOL:
			rParent.AddCurrency( aContent.makeStringAndClear(), nElementLang );
			break;

		case XML_TOK_STYLE_TEXT_CONTENT:
			rParent.AddToCode( OUString::valueOf((sal_Unicode)'@') );
			break;
		case XML_TOK_STYLE_BOOLEAN:
			// ignored - only default boolean format is supported
			break;

		case XML_TOK_STYLE_DAY:
			rParent.UpdateCalendar( sCalendar );
			rParent.AddNfKeyword( bEffLong ? NF_KEY_DD : NF_KEY_D );
			break;
		case XML_TOK_STYLE_MONTH:
			rParent.UpdateCalendar( sCalendar );
			rParent.AddNfKeyword( bTextual ? ( bEffLong ? NF_KEY_MMMM : NF_KEY_MMM ) :
											 ( bEffLong ? NF_KEY_MM : NF_KEY_M ) );
			break;
		case XML_TOK_STYLE_YEAR:
			rParent.UpdateCalendar( sCalendar );
			// Y after G (era) is replaced by E
			if ( rParent.HasEra() )
				rParent.AddNfKeyword( bEffLong ? NF_KEY_EEC : NF_KEY_EC );
			else
				rParent.AddNfKeyword( bEffLong ? NF_KEY_YYYY : NF_KEY_YY );
			break;
		case XML_TOK_STYLE_ERA:
			rParent.UpdateCalendar( sCalendar );
#if 0
//! I18N doesn't provide SYSTEM or extended date information yet
			if ( rParent.IsFromSystem() )
				bEffLong = SvXMLNumFmtDefaults::IsSystemLongEra( rParent.GetInternational(), bLong );
#endif
			rParent.AddNfKeyword( bEffLong ? NF_KEY_GGG : NF_KEY_G );
			//	HasEra flag is set
			break;
		case XML_TOK_STYLE_DAY_OF_WEEK:
			rParent.UpdateCalendar( sCalendar );
			rParent.AddNfKeyword( bEffLong ? NF_KEY_NNNN : NF_KEY_NN );
			break;
		case XML_TOK_STYLE_WEEK_OF_YEAR:
			rParent.UpdateCalendar( sCalendar );
			rParent.AddNfKeyword( NF_KEY_WW );
			break;
		case XML_TOK_STYLE_QUARTER:
			rParent.UpdateCalendar( sCalendar );
			rParent.AddNfKeyword( bEffLong ? NF_KEY_QQ : NF_KEY_Q );
			break;
		case XML_TOK_STYLE_HOURS:
			rParent.AddNfKeyword( bEffLong ? NF_KEY_HH : NF_KEY_H );
			break;
		case XML_TOK_STYLE_AM_PM:
			//!	short/long?
			rParent.AddNfKeyword( NF_KEY_AMPM );
			break;
		case XML_TOK_STYLE_MINUTES:
			rParent.AddNfKeyword( bEffLong ? NF_KEY_MMI : NF_KEY_MI );
			break;
		case XML_TOK_STYLE_SECONDS:
			rParent.AddNfKeyword( bEffLong ? NF_KEY_SS : NF_KEY_S );
			if ( aNumInfo.nDecimals > 0 )
			{
				//	manually add the decimal places
				const String& rSep = rParent.GetLocaleData().getNumDecimalSep();
				for ( xub_StrLen j=0; j<rSep.Len(); j++ )
				{
					rParent.AddToCode( OUString::valueOf( rSep.GetChar(j) ) );
				}
				for (sal_Int32 i=0; i<aNumInfo.nDecimals; i++)
					rParent.AddToCode( OUString::valueOf((sal_Unicode)'0') );
			}
			break;

		case XML_TOK_STYLE_FRACTION:
			{
				aNumInfo.nDecimals = 0;
				rParent.AddNumber( aNumInfo );		// number without decimals

				//!	build string and add at once

				sal_Int32 i;
				rParent.AddToCode( OUString::valueOf((sal_Unicode)' ') );
				for (i=0; i<aNumInfo.nNumerDigits; i++)
					rParent.AddToCode( OUString::valueOf((sal_Unicode)'?') );
				rParent.AddToCode( OUString::valueOf((sal_Unicode)'/') );
				for (i=0; i<aNumInfo.nDenomDigits; i++)
					rParent.AddToCode( OUString::valueOf((sal_Unicode)'?') );
			}
			break;

		case XML_TOK_STYLE_SCIENTIFIC_NUMBER:
			{
				rParent.AddNumber( aNumInfo );		// simple number

				rParent.AddToCode( OUString::createFromAscii( "E+" ) );
				for (sal_Int32 i=0; i<aNumInfo.nExpDigits; i++)
					rParent.AddToCode( OUString::valueOf((sal_Unicode)'0') );
			}
			break;

		default:
			DBG_ERROR("invalid element ID");
	}
}

//-------------------------------------------------------------------------

sal_uInt16 SvXMLNumFmtDefaults::GetDefaultDateFormat( SvXMLDateElementAttributes eDOW,
				SvXMLDateElementAttributes eDay, SvXMLDateElementAttributes eMonth,
				SvXMLDateElementAttributes eYear, SvXMLDateElementAttributes eHours,
				SvXMLDateElementAttributes eMins, SvXMLDateElementAttributes eSecs,
				sal_Bool bSystem )
{
	const sal_uInt16 nCount = sizeof(aDefaultDateFormats) / sizeof(SvXMLDefaultDateFormat);
	for (sal_uInt16 nPos=0; nPos<nCount; nPos++)
	{
		const SvXMLDefaultDateFormat& rEntry = aDefaultDateFormats[nPos];
		if ( bSystem == rEntry.bSystem &&
			( eDOW   == rEntry.eDOW   || ( rEntry.eDOW   == XML_DEA_ANY && eDOW   != XML_DEA_NONE ) ) &&
			( eDay   == rEntry.eDay   || ( rEntry.eDay   == XML_DEA_ANY && eDay   != XML_DEA_NONE ) ) &&
			( eMonth == rEntry.eMonth || ( rEntry.eMonth == XML_DEA_ANY && eMonth != XML_DEA_NONE ) ) &&
			( eYear  == rEntry.eYear  || ( rEntry.eYear  == XML_DEA_ANY && eYear  != XML_DEA_NONE ) ) &&
			( eHours == rEntry.eHours || ( rEntry.eHours == XML_DEA_ANY && eHours != XML_DEA_NONE ) ) &&
			( eMins  == rEntry.eMins  || ( rEntry.eMins  == XML_DEA_ANY && eMins  != XML_DEA_NONE ) ) &&
			( eSecs  == rEntry.eSecs  || ( rEntry.eSecs  == XML_DEA_ANY && eSecs  != XML_DEA_NONE ) ) )
		{
			return rEntry.eFormat;
		}
	}

	return NF_INDEX_TABLE_ENTRIES;	// invalid
}

//-------------------------------------------------------------------------

//
//	SvXMLNumFormatContext
//

SvXMLNumFormatContext::SvXMLNumFormatContext( SvXMLImport& rImport,
									USHORT nPrfx, const ::rtl::OUString& rLName,
									SvXMLNumImpData* pNewData, sal_uInt16 nNewType,
									const uno::Reference<xml::sax::XAttributeList>& xAttrList,
									SvXMLStylesContext& rStyles ) :
	SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList ),
	pData( pNewData ),
	pStyles( &rStyles ),
	aMyConditions(),
	nType( nNewType ),
	nKey(-1),
	nFormatLang( LANGUAGE_SYSTEM ),
	bAutoOrder( FALSE ),
	bFromSystem( FALSE ),
	bTruncate( TRUE ),
	bAutoDec( FALSE ),
	bAutoInt( FALSE ),
	bHasExtraText( FALSE ),
	bHasLongDoW( FALSE ),
	bHasEra( FALSE ),
	bHasDateTime( FALSE ),
	bRemoveAfterUse( sal_False ),
	eDateDOW( XML_DEA_NONE ),
	eDateDay( XML_DEA_NONE ),
	eDateMonth( XML_DEA_NONE ),
	eDateYear( XML_DEA_NONE ),
	eDateHours( XML_DEA_NONE ),
	eDateMins( XML_DEA_NONE ),
	eDateSecs( XML_DEA_NONE ),
	bDateNoDefault( sal_False )
{
	OUString sLanguage, sCountry;
    ::com::sun::star::i18n::NativeNumberXmlAttributes aNatNumAttr;
	sal_Bool bAttrBool;
	sal_uInt16 nAttrEnum;

	sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
	for( sal_Int16 i=0; i < nAttrCount; i++ )
	{
		OUString sAttrName = xAttrList->getNameByIndex( i );
		OUString sValue = xAttrList->getValueByIndex( i );
		OUString aLocalName;
		sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

		const SvXMLTokenMap& rTokenMap = pData->GetStyleAttrTokenMap();
		sal_uInt16 nToken = rTokenMap.Get( nPrefix, aLocalName );
		switch (nToken)
		{
			case XML_TOK_STYLE_ATTR_NAME:
//				aName = sValue;
				break;
			case XML_TOK_STYLE_ATTR_LANGUAGE:
				sLanguage = sValue;
				break;
			case XML_TOK_STYLE_ATTR_COUNTRY:
				sCountry = sValue;
				break;
			case XML_TOK_STYLE_ATTR_TITLE:
				sFormatTitle = sValue;
				break;
			case XML_TOK_STYLE_ATTR_AUTOMATIC_ORDER:
				if ( SvXMLUnitConverter::convertBool( bAttrBool, sValue ) )
					bAutoOrder = bAttrBool;
				break;
			case XML_TOK_STYLE_ATTR_FORMAT_SOURCE:
				if ( SvXMLUnitConverter::convertEnum( nAttrEnum, sValue, aFormatSourceMap ) )
					bFromSystem = (sal_Bool) nAttrEnum;
				break;
			case XML_TOK_STYLE_ATTR_TRUNCATE_ON_OVERFLOW:
				if ( SvXMLUnitConverter::convertBool( bAttrBool, sValue ) )
					bTruncate = bAttrBool;
				break;
			case XML_TOK_STYLE_ATTR_VOLATILE:
				//	volatile formats can be removed after importing
				//	if not used in other styles
				if ( SvXMLUnitConverter::convertBool( bAttrBool, sValue ) )
					bRemoveAfterUse = bAttrBool;
				break;
            case XML_TOK_STYLE_ATTR_TRANSL_FORMAT:
                aNatNumAttr.Format = sValue;
                break;
            case XML_TOK_STYLE_ATTR_TRANSL_LANGUAGE:
                aNatNumAttr.Locale.Language = sValue;
                break;
            case XML_TOK_STYLE_ATTR_TRANSL_COUNTRY:
                aNatNumAttr.Locale.Country = sValue;
                break;
            case XML_TOK_STYLE_ATTR_TRANSL_STYLE:
                aNatNumAttr.Style = sValue;
                break;
		}
	}

	if ( sLanguage.getLength() || sCountry.getLength() )
	{
		nFormatLang = MsLangId::convertIsoNamesToLanguage( sLanguage, sCountry );
		if ( nFormatLang == LANGUAGE_DONTKNOW )
			nFormatLang = LANGUAGE_SYSTEM;			//! error handling for invalid locales?
	}

    if ( aNatNumAttr.Format.getLength() )
    {
        SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
        if ( pFormatter )
        {
            sal_Int32 nNatNum = pFormatter->GetNatNum()->convertFromXmlAttributes( aNatNumAttr );
            aFormatCode.appendAscii( RTL_CONSTASCII_STRINGPARAM( "[NatNum" ) );
            aFormatCode.append( nNatNum, 10 );

		    LanguageType eLang = MsLangId::convertIsoNamesToLanguage( 
                    aNatNumAttr.Locale.Language, aNatNumAttr.Locale.Country );
            if ( eLang == LANGUAGE_DONTKNOW )
                eLang = LANGUAGE_SYSTEM;            //! error handling for invalid locales?
            if ( eLang != nFormatLang && eLang != LANGUAGE_SYSTEM )
            {
                aFormatCode.appendAscii( RTL_CONSTASCII_STRINGPARAM( "][$-" ) );
                // language code in upper hex:
                aFormatCode.append( String::CreateFromInt32( sal_Int32( eLang ), 16 ).ToUpperAscii() );
            }
            aFormatCode.append( sal_Unicode(']') );
        }
    }
}

SvXMLNumFormatContext::SvXMLNumFormatContext( SvXMLImport& rImport,
									USHORT nPrfx, const ::rtl::OUString& rLName,
									const uno::Reference<xml::sax::XAttributeList>& xAttrList,
									const sal_Int32 nTempKey,
									SvXMLStylesContext& rStyles ) :
	SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList, XML_STYLE_FAMILY_DATA_STYLE ),
	pData( NULL ),
	pStyles( &rStyles ),
	aMyConditions(),
	nType( 0 ),
	nKey(nTempKey),
	nFormatLang( LANGUAGE_SYSTEM ),
	bAutoOrder( FALSE ),
	bFromSystem( FALSE ),
	bTruncate( TRUE ),
	bAutoDec( FALSE ),
	bAutoInt( FALSE ),
	bHasExtraText( FALSE ),
	bHasLongDoW( FALSE ),
	bHasEra( FALSE ),
	bHasDateTime( FALSE ),
	bRemoveAfterUse( sal_False ),
	eDateDOW( XML_DEA_NONE ),
	eDateDay( XML_DEA_NONE ),
	eDateMonth( XML_DEA_NONE ),
	eDateYear( XML_DEA_NONE ),
	eDateHours( XML_DEA_NONE ),
	eDateMins( XML_DEA_NONE ),
	eDateSecs( XML_DEA_NONE ),
	bDateNoDefault( sal_False )
{
	SetAttribute(XML_NAMESPACE_STYLE, GetXMLToken(XML_NAME), rLName);
}

SvXMLNumFormatContext::~SvXMLNumFormatContext()
{
}

SvXMLImportContext* SvXMLNumFormatContext::CreateChildContext(
									USHORT nPrfx, const ::rtl::OUString& rLName,
									const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
	SvXMLImportContext* pContext = NULL;

	const SvXMLTokenMap& rTokenMap = pData->GetStyleElemTokenMap();
	sal_uInt16 nToken = rTokenMap.Get( nPrfx, rLName );
	switch (nToken)
	{
		case XML_TOK_STYLE_TEXT:
		case XML_TOK_STYLE_NUMBER:
		case XML_TOK_STYLE_SCIENTIFIC_NUMBER:
		case XML_TOK_STYLE_FRACTION:
		case XML_TOK_STYLE_CURRENCY_SYMBOL:
		case XML_TOK_STYLE_DAY:
		case XML_TOK_STYLE_MONTH:
		case XML_TOK_STYLE_YEAR:
		case XML_TOK_STYLE_ERA:
		case XML_TOK_STYLE_DAY_OF_WEEK:
		case XML_TOK_STYLE_WEEK_OF_YEAR:
		case XML_TOK_STYLE_QUARTER:
		case XML_TOK_STYLE_HOURS:
		case XML_TOK_STYLE_AM_PM:
		case XML_TOK_STYLE_MINUTES:
		case XML_TOK_STYLE_SECONDS:
		case XML_TOK_STYLE_BOOLEAN:
		case XML_TOK_STYLE_TEXT_CONTENT:
			pContext = new SvXMLNumFmtElementContext( GetImport(), nPrfx, rLName,
														*this, nToken, xAttrList );
			break;

		case XML_TOK_STYLE_PROPERTIES:
			pContext = new SvXMLNumFmtPropContext( GetImport(), nPrfx, rLName,
														*this, xAttrList );
			break;
		case XML_TOK_STYLE_MAP:
			{
				//	SvXMLNumFmtMapContext::EndElement adds to aMyConditions,
				//	so there's no need for an extra flag
				pContext = new SvXMLNumFmtMapContext( GetImport(), nPrfx, rLName,
															*this, xAttrList );
			}
			break;
	}

	if( !pContext )
		pContext = new SvXMLImportContext( GetImport(), nPrfx, rLName );
	return pContext;
}

sal_Int32 SvXMLNumFormatContext::GetKey()
{
	if (nKey > -1)
	{
		if (bRemoveAfterUse)
		{
			//	format is used -> don't remove
			bRemoveAfterUse = sal_False;
			if (pData)
				pData->SetUsed(nKey);

			//	Add to import's list of keys now - CreateAndInsert didn't add
			//	the style if bRemoveAfterUse was set.
			GetImport().AddNumberStyle( nKey, GetName() );
		}
		return nKey;
	}
	else
	{
		// reset bRemoveAfterUse before CreateAndInsert, so AddKey is called without bRemoveAfterUse set
		bRemoveAfterUse = sal_False;
		CreateAndInsert(sal_True);
		return nKey;
	}
}

sal_Int32 SvXMLNumFormatContext::PrivateGetKey()
{
	//	used for map elements in CreateAndInsert - don't reset bRemoveAfterUse flag

	if (nKey > -1)
		return nKey;
	else
	{
		CreateAndInsert(sal_True);
		return nKey;
	}
}

void SvXMLNumFormatContext::GetFormat( ::rtl::OUString& rFormatString, lang::Locale& rLocale)
{
	//#95893#; remember the created FormatString and Locales
	if (!sFormatString.getLength() &&
		!aLocale.Language.getLength() &&
		!aLocale.Country.getLength())
	{
		if (aMyConditions.size())
		{
			::rtl::OUString sFormat;
			lang::Locale aLoc;
			for (sal_uInt32 i = 0; i < aMyConditions.size(); i++)
			{
				SvXMLNumFormatContext* pStyle = (SvXMLNumFormatContext *)pStyles->FindStyleChildContext(
					XML_STYLE_FAMILY_DATA_STYLE, aMyConditions[i].sMapName, sal_False);
				if (pStyle)
				{
					pStyle->GetFormat(sFormat, aLoc);
					AddCondition(i, sFormat, pStyle->GetLocaleData());
				}
			}
		}

		if ( !aFormatCode.getLength() )
		{
			//	insert empty format as empty string (with quotes)
			//	#93901# this check has to be done before inserting the conditions
			aFormatCode.appendAscii("\"\"");	// ""
		}
		
		aFormatCode.insert( 0, aConditions.makeStringAndClear() );
		sFormatString = aFormatCode.makeStringAndClear();
        MsLangId::convertLanguageToLocale(nFormatLang, aLocale);
	}
	rLocale = aLocale;
	rFormatString = sFormatString;
}

void SvXMLNumFormatContext::CreateAndInsert(sal_Bool bOverwrite)
{
	if (!(nKey > -1))
	{
		SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
		if (!pFormatter)
		{
			DBG_ERROR("no number formatter");
			return;
		}

		sal_uInt32 nIndex = NUMBERFORMAT_ENTRY_NOT_FOUND;

		for (sal_uInt32 i = 0; i < aMyConditions.size(); i++)
		{
			SvXMLNumFormatContext* pStyle = (SvXMLNumFormatContext *)pStyles->FindStyleChildContext(
				XML_STYLE_FAMILY_DATA_STYLE, aMyConditions[i].sMapName, sal_False);
			if (pStyle)
			{
				if ((pStyle->PrivateGetKey() > -1))		// don't reset pStyle's bRemoveAfterUse flag
					AddCondition(i);
			}
		}

		if ( !aFormatCode.getLength() )
		{
			//	insert empty format as empty string (with quotes)
			//	#93901# this check has to be done before inserting the conditions
			aFormatCode.appendAscii("\"\"");	// ""
		}

		aFormatCode.insert( 0, aConditions.makeStringAndClear() );
		OUString sFormat = aFormatCode.makeStringAndClear();

		//	test special cases

		if ( bAutoDec )			// automatic decimal places
		{
			//	#99391# adjust only if the format contains no text elements, no conditions
			//	and no color definition (detected by the '[' at the start)

			if ( nType == XML_TOK_STYLES_NUMBER_STYLE && !bHasExtraText &&
					aMyConditions.size() == 0 && sFormat.toChar() != (sal_Unicode)'[' )
				nIndex = pFormatter->GetStandardIndex( nFormatLang );
		}
		if ( bAutoInt )			// automatic integer digits
		{
			//!	only if two decimal places was set?

			if ( nType == XML_TOK_STYLES_NUMBER_STYLE && !bHasExtraText &&
					aMyConditions.size() == 0 && sFormat.toChar() != (sal_Unicode)'[' )
				nIndex = pFormatter->GetFormatIndex( NF_NUMBER_SYSTEM, nFormatLang );
		}

		//	boolean is always the builtin boolean format
		//	(no other boolean formats are implemented)
		if ( nType == XML_TOK_STYLES_BOOLEAN_STYLE )
			nIndex = pFormatter->GetFormatIndex( NF_BOOLEAN, nFormatLang );

		//	check for default date formats
		if ( nType == XML_TOK_STYLES_DATE_STYLE && bAutoOrder && !bDateNoDefault )
		{
			NfIndexTableOffset eFormat = (NfIndexTableOffset) SvXMLNumFmtDefaults::GetDefaultDateFormat(
				eDateDOW, eDateDay, eDateMonth, eDateYear,
				eDateHours, eDateMins, eDateSecs, bFromSystem );
			if ( eFormat < NF_INDEX_TABLE_ENTRIES )
			{
				//	#109651# if a date format has the automatic-order attribute and
				//	contains exactly the elements of one of the default date formats,
				//	use that default format, with the element order and separators
				//	from the current locale settings

				nIndex = pFormatter->GetFormatIndex( eFormat, nFormatLang );
			}
		}

		if ( nIndex == NUMBERFORMAT_ENTRY_NOT_FOUND && sFormat.getLength() )
		{
			//	insert by format string

			String aFormatStr( sFormat );
			nIndex = pFormatter->GetEntryKey( aFormatStr, nFormatLang );
			if ( nIndex == NUMBERFORMAT_ENTRY_NOT_FOUND )
			{
				xub_StrLen	nErrPos	= 0;
				short		nType	= 0;
				sal_Bool bOk = pFormatter->PutEntry( aFormatStr, nErrPos, nType, nIndex, nFormatLang );
				if ( !bOk && nErrPos == 0 && aFormatStr != String(sFormat) )
				{
					//	if the string was modified by PutEntry, look for an existing format
					//	with the modified string
					nIndex = pFormatter->GetEntryKey( aFormatStr, nFormatLang );
					if ( nIndex != NUMBERFORMAT_ENTRY_NOT_FOUND )
						bOk = sal_True;
				}
				if (!bOk)
					nIndex = NUMBERFORMAT_ENTRY_NOT_FOUND;
			}
		}

#if 0
//! I18N doesn't provide SYSTEM or extended date information yet
		if ( nIndex != NUMBERFORMAT_ENTRY_NOT_FOUND && !bFromSystem )
		{
			//	instead of automatic date format, use fixed formats if bFromSystem is not set
			//!	prevent use of automatic formats in other cases, force user-defined format?

			sal_uInt32 nNewIndex = nIndex;

			NfIndexTableOffset eOffset = pFormatter->GetIndexTableOffset( nIndex );
			if ( eOffset == NF_DATE_SYSTEM_SHORT )
			{
				const International& rInt = pData->GetInternational( nFormatLang );
				if ( rInt.IsDateDayLeadingZero() && rInt.IsDateMonthLeadingZero() )
				{
					if ( rInt.IsDateCentury() )
						nNewIndex = pFormatter->GetFormatIndex( NF_DATE_SYS_DDMMYYYY, nFormatLang );
					else
						nNewIndex = pFormatter->GetFormatIndex( NF_DATE_SYS_DDMMYY, nFormatLang );
				}
			}
			else if ( eOffset == NF_DATE_SYSTEM_LONG )
			{
				const International& rInt = pData->GetInternational( nFormatLang );
				if ( !rInt.IsLongDateDayLeadingZero() )
				{
					sal_Bool bCentury = rInt.IsLongDateCentury();
					MonthFormat eMonth = rInt.GetLongDateMonthFormat();
					if ( eMonth == MONTH_LONG && bCentury )
					{
						if ( rInt.GetLongDateDayOfWeekFormat() == DAYOFWEEK_LONG )
							nNewIndex = pFormatter->GetFormatIndex( NF_DATE_SYS_NNNNDMMMMYYYY, nFormatLang );
						else
							nNewIndex = pFormatter->GetFormatIndex( NF_DATE_SYS_NNDMMMMYYYY, nFormatLang );
					}
					else if ( eMonth == MONTH_SHORT && !bCentury )
						nNewIndex = pFormatter->GetFormatIndex( NF_DATE_SYS_NNDMMMYY, nFormatLang );
				}
			}

			if ( nNewIndex != nIndex )
			{
				//	verify the fixed format really matches the format string
				//	(not the case with some formats from locale data)

				const SvNumberformat* pFixedFormat = pFormatter->GetEntry( nNewIndex );
				if ( pFixedFormat && pFixedFormat->GetFormatstring() == String(sFormat) )
					nIndex = nNewIndex;
			}
		}
#endif

		if ( nIndex != NUMBERFORMAT_ENTRY_NOT_FOUND && !bAutoOrder )
		{
			//	use fixed-order formats instead of SYS... if bAutoOrder is false
			//	(only if the format strings are equal for the locale)

			NfIndexTableOffset eOffset = pFormatter->GetIndexTableOffset( nIndex );
			if ( eOffset == NF_DATE_SYS_DMMMYYYY )
			{
				sal_uInt32 nNewIndex = pFormatter->GetFormatIndex( NF_DATE_DIN_DMMMYYYY, nFormatLang );
				const SvNumberformat* pOldEntry = pFormatter->GetEntry( nIndex );
				const SvNumberformat* pNewEntry = pFormatter->GetEntry( nNewIndex );
				if ( pOldEntry && pNewEntry && pOldEntry->GetFormatstring() == pNewEntry->GetFormatstring() )
					nIndex = nNewIndex;
			}
			else if ( eOffset == NF_DATE_SYS_DMMMMYYYY )
			{
				sal_uInt32 nNewIndex = pFormatter->GetFormatIndex( NF_DATE_DIN_DMMMMYYYY, nFormatLang );
				const SvNumberformat* pOldEntry = pFormatter->GetEntry( nIndex );
				const SvNumberformat* pNewEntry = pFormatter->GetEntry( nNewIndex );
				if ( pOldEntry && pNewEntry && pOldEntry->GetFormatstring() == pNewEntry->GetFormatstring() )
					nIndex = nNewIndex;
			}
		}

		if ((nIndex != NUMBERFORMAT_ENTRY_NOT_FOUND) && sFormatTitle.getLength())
		{
			SvNumberformat* pFormat = const_cast<SvNumberformat*>(pFormatter->GetEntry( nIndex ));
			if (pFormat)
			{
				String sTitle (sFormatTitle);
				pFormat->SetComment(sTitle);
			}
		}

		if ( nIndex == NUMBERFORMAT_ENTRY_NOT_FOUND )
		{
			DBG_ERROR("invalid number format");
			nIndex = pFormatter->GetStandardIndex( nFormatLang );
		}

		pData->AddKey( nIndex, GetName(), bRemoveAfterUse );
		nKey = nIndex;

		//	Add to import's list of keys (shared between styles and content import)
		//	only if not volatile - formats are removed from NumberFormatter at the
		//	end of each import (in SvXMLNumFmtHelper dtor).
		//	If bRemoveAfterUse is reset later in GetKey, AddNumberStyle is called there.

		if (!bRemoveAfterUse)
			GetImport().AddNumberStyle( nKey, GetName() );

	#if 0
		ByteString aByte( String(sFormatName), gsl_getSystemTextEncoding() );
		aByte.Append( " | " );
		aByte.Append(ByteString( String(sFormat), gsl_getSystemTextEncoding() ));
		aByte.Append( " | " );
		aByte.Append(ByteString::CreateFromInt32( nIndex ));

	//	DBG_ERROR(aByte.GetBuffer());
		int xxx=42;
	#endif
	}
}

void SvXMLNumFormatContext::Finish( sal_Bool bOverwrite )
{
	SvXMLStyleContext::Finish( bOverwrite );
//	AddCondition();
}

const LocaleDataWrapper& SvXMLNumFormatContext::GetLocaleData() const
{
	return pData->GetLocaleData( nFormatLang );
}

void SvXMLNumFormatContext::AddToCode( const ::rtl::OUString& rString )
{
	aFormatCode.append( rString );
	bHasExtraText = sal_True;
}

void SvXMLNumFormatContext::AddNumber( const SvXMLNumberInfo& rInfo )
{
	SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
	if (!pFormatter)
		return;

	//	store special conditions
	bAutoDec = ( rInfo.nDecimals < 0 );
	bAutoInt = ( rInfo.nInteger < 0 );

	sal_uInt16 nPrec = 0;
	sal_uInt16 nLeading = 0;
	if ( rInfo.nDecimals >= 0 )						//	< 0 : Default
		nPrec = (sal_uInt16) rInfo.nDecimals;
	if ( rInfo.nInteger >= 0 )						//	< 0 : Default
		nLeading = (sal_uInt16) rInfo.nInteger;

	if ( bAutoDec )
	{
		if ( nType == XML_TOK_STYLES_CURRENCY_STYLE )
		{
			//	for currency formats, "automatic decimals" is used for the automatic
			//	currency format with (fixed) decimals from the locale settings

			const LocaleDataWrapper& rLoc = pData->GetLocaleData( nFormatLang );
			nPrec = rLoc.getCurrDigits();
		}
		else
		{
			//	for other types, "automatic decimals" means dynamic determination of
			//	decimals, as achieved with the "general" keyword

	        aFormatCode.append( pFormatter->GetStandardName( nFormatLang ) );
	        return;
		}
	}
	if ( bAutoInt )
	{
		//!...
	}

	sal_uInt16 nGenPrec = nPrec;
	if ( rInfo.bDecReplace || rInfo.bVarDecimals )
		nGenPrec = 0;				// generate format without decimals...

	sal_Bool bGrouping = rInfo.bGrouping;
	USHORT nEmbeddedCount = rInfo.aEmbeddedElements.Count();
	if ( nEmbeddedCount )
		bGrouping = sal_False;		// grouping and embedded characters can't be used together

	String aNumStr;
	sal_uInt32 nStdIndex = pFormatter->GetStandardIndex( nFormatLang );
	pFormatter->GenerateFormat( aNumStr, nStdIndex, nFormatLang,
								bGrouping, sal_False, nGenPrec, nLeading );

	if ( nEmbeddedCount )
	{
		//	insert embedded strings into number string
		//	only the integer part is supported
		//	nZeroPos is the string position where format position 0 is inserted

	    xub_StrLen nZeroPos = aNumStr.Search( pData->GetLocaleData( nFormatLang ).getNumDecimalSep() );
	    if ( nZeroPos == STRING_NOTFOUND )
	    	nZeroPos = aNumStr.Len();

		//	aEmbeddedElements is sorted - last entry has the largest position (leftmost)
		const SvXMLEmbeddedElement* pLastObj = rInfo.aEmbeddedElements[nEmbeddedCount - 1];
		sal_Int32 nLastFormatPos = pLastObj->nFormatPos;
		if ( nLastFormatPos >= nZeroPos )
		{
			//	add '#' characters so all embedded texts are really embedded in digits
			//	(there always has to be a digit before the leftmost embedded text)

			xub_StrLen nAddCount = (xub_StrLen)nLastFormatPos + 1 - nZeroPos;
			String aDigitStr;
			aDigitStr.Fill( nAddCount, (sal_Unicode)'#' );
			aNumStr.Insert( aDigitStr, 0 );
			nZeroPos += nAddCount;
		}

		//	aEmbeddedElements is sorted with ascending positions - loop is from right to left
		for (USHORT nElement = 0; nElement < nEmbeddedCount; nElement++)
		{
			const SvXMLEmbeddedElement* pObj = rInfo.aEmbeddedElements[nElement];
			sal_Int32 nFormatPos = pObj->nFormatPos;
			sal_Int32 nInsertPos = nZeroPos - nFormatPos;
			if ( nFormatPos >= 0 && nInsertPos >= 0 )
			{
				::rtl::OUStringBuffer aContent( pObj->aText );
				//	#107805# always quote embedded strings - even space would otherwise
				//	be recognized as thousands separator in French.
				aContent.insert( 0, (sal_Unicode) '"' );
				aContent.append( (sal_Unicode) '"' );

				aNumStr.Insert( String( aContent.makeStringAndClear() ), (xub_StrLen)nInsertPos );
			}
		}
	}

	aFormatCode.append( aNumStr );

	if ( ( rInfo.bDecReplace || rInfo.bVarDecimals ) && nPrec )		// add decimal replacement (dashes)
	{
		//	add dashes for explicit decimal replacement, # for variable decimals
		sal_Unicode cAdd = rInfo.bDecReplace ? '-' : '#';

		aFormatCode.append( pData->GetLocaleData( nFormatLang ).getNumDecimalSep() );
		for ( sal_uInt16 i=0; i<nPrec; i++)
			aFormatCode.append( cAdd );
	}

	//	add extra thousands separators for display factor

	if ( rInfo.fDisplayFactor != 1.0 && rInfo.fDisplayFactor > 0.0 )
	{
		//	test for 1.0 is just for optimization - nSepCount would be 0

		//	one separator for each factor of 1000
		sal_Int32 nSepCount = (sal_Int32) ::rtl::math::round( log10(rInfo.fDisplayFactor) / 3.0 );
		if ( nSepCount > 0 )
		{
			OUString aSep = pData->GetLocaleData( nFormatLang ).getNumThousandSep();
			for ( sal_Int32 i=0; i<nSepCount; i++ )
				aFormatCode.append( aSep );
		}
	}
}

void SvXMLNumFormatContext::AddCurrency( const ::rtl::OUString& rContent, LanguageType nLang )
{
	sal_Bool bAutomatic = sal_False;
	OUString aSymbol = rContent;
	if ( aSymbol.getLength() == 0 )
	{
		//	get currency symbol for language

		//aSymbol = pData->GetLocaleData( nFormatLang ).getCurrSymbol();

		SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
		if ( pFormatter )
		{
			pFormatter->ChangeIntl( nFormatLang );
			String sCurString, sDummy;
			pFormatter->GetCompatibilityCurrency( sCurString, sDummy );
			aSymbol = sCurString;

			bAutomatic = sal_True;
		}
	}
	else if ( nLang == LANGUAGE_SYSTEM && aSymbol.compareToAscii("CCC") == 0 )
	{
		//	"CCC" is used for automatic long symbol
		bAutomatic = sal_True;
	}

	if ( bAutomatic )
	{
		//	remove unnecessary quotes before automatic symbol (formats like "-(0DM)")
		//	otherwise the currency symbol isn't recognized (#94048#)

		sal_Int32 nLength = aFormatCode.getLength();
		if ( nLength > 1 && aFormatCode.charAt( nLength-1 ) == '"' )
		{
			//	find start of quoted string
			//	When SvXMLNumFmtElementContext::EndElement creates escaped quotes,
			//	they must be handled here, too.

			sal_Int32 nFirst = nLength - 2;
			while ( nFirst >= 0 && aFormatCode.charAt( nFirst ) != '"' )
				--nFirst;
			if ( nFirst >= 0 )
			{
				//	remove both quotes from aFormatCode
				::rtl::OUString aOld = aFormatCode.makeStringAndClear();
				if ( nFirst > 0 )
					aFormatCode.append( aOld.copy( 0, nFirst ) );
				if ( nLength > nFirst + 2 )
					aFormatCode.append( aOld.copy( nFirst + 1, nLength - nFirst - 2 ) );
			}
		}
	}

	if (!bAutomatic)
		aFormatCode.appendAscii( "[$" );			// intro for "new" currency symbols

	aFormatCode.append( aSymbol );

	if (!bAutomatic)
	{
		if ( nLang != LANGUAGE_SYSTEM )
		{
			//	'-' sign and language code in hex:
			aFormatCode.append( (sal_Unicode) '-' );
			aFormatCode.append( String::CreateFromInt32( sal_Int32( nLang ), 16 ).ToUpperAscii() );
		}

		aFormatCode.append( (sal_Unicode) ']' );	// end of "new" currency symbol
	}
}

void SvXMLNumFormatContext::AddNfKeyword( sal_uInt16 nIndex )
{
	SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
	if (!pFormatter)
		return;

	if ( nIndex == NF_KEY_G || nIndex == NF_KEY_GG || nIndex == NF_KEY_GGG )
		bHasEra = sal_True;

	if ( nIndex == NF_KEY_NNNN )
	{
		nIndex = NF_KEY_NNN;
		bHasLongDoW = sal_True;			// to remove string constant with separator
	}

	String sKeyword = pFormatter->GetKeyword( nFormatLang, nIndex );

	if ( nIndex == NF_KEY_H  || nIndex == NF_KEY_HH  ||
		 nIndex == NF_KEY_MI || nIndex == NF_KEY_MMI ||
		 nIndex == NF_KEY_S  || nIndex == NF_KEY_SS )
	{
		if ( !bTruncate && !bHasDateTime )
		{
			//	with truncate-on-overflow = false, add "[]" to first time part

			sKeyword.Insert( (sal_Unicode) '[', 0 );
			sKeyword.Append( (sal_Unicode) ']' );
		}
		bHasDateTime = sal_True;
	}

	aFormatCode.append( sKeyword );

	//	collect the date elements that the format contains, to recognize default date formats
	switch ( nIndex )
	{
		case NF_KEY_NN:		eDateDOW = XML_DEA_SHORT;		break;
		case NF_KEY_NNN:
		case NF_KEY_NNNN:	eDateDOW = XML_DEA_LONG;		break;
		case NF_KEY_D:		eDateDay = XML_DEA_SHORT;		break;
		case NF_KEY_DD:		eDateDay = XML_DEA_LONG;		break;
		case NF_KEY_M:		eDateMonth = XML_DEA_SHORT;		break;
		case NF_KEY_MM:		eDateMonth = XML_DEA_LONG;		break;
		case NF_KEY_MMM:	eDateMonth = XML_DEA_TEXTSHORT;	break;
		case NF_KEY_MMMM:	eDateMonth = XML_DEA_TEXTLONG;	break;
		case NF_KEY_YY:		eDateYear = XML_DEA_SHORT;		break;
		case NF_KEY_YYYY:	eDateYear = XML_DEA_LONG;		break;
		case NF_KEY_H:		eDateHours = XML_DEA_SHORT;		break;
		case NF_KEY_HH:		eDateHours = XML_DEA_LONG;		break;
		case NF_KEY_MI:		eDateMins = XML_DEA_SHORT;		break;
		case NF_KEY_MMI:	eDateMins = XML_DEA_LONG;		break;
		case NF_KEY_S:		eDateSecs = XML_DEA_SHORT;		break;
		case NF_KEY_SS:		eDateSecs = XML_DEA_LONG;		break;
		case NF_KEY_AP:
		case NF_KEY_AMPM:	break;			// AM/PM may or may not be in date/time formats -> ignore by itself
		default:
			bDateNoDefault = sal_True;		// any other element -> no default format
	}
}

sal_Bool lcl_IsAtEnd( ::rtl::OUStringBuffer& rBuffer, const String& rToken )
{
    sal_Int32 nBufLen = rBuffer.getLength();
    xub_StrLen nTokLen = rToken.Len();

    if ( nTokLen > nBufLen )
    	return sal_False;

	sal_Int32 nStartPos = nTokLen - nBufLen;
	for ( xub_StrLen nTokPos = 0; nTokPos < nTokLen; nTokPos++ )
		if ( rToken.GetChar( nTokPos ) != rBuffer.charAt( nStartPos + nTokPos ) )
			return sal_False;

	return sal_True;
}

sal_Bool SvXMLNumFormatContext::ReplaceNfKeyword( sal_uInt16 nOld, sal_uInt16 nNew )
{
	//	replaces one keyword with another if it is found at the end of the code

	SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
	if (!pFormatter)
		return sal_False;

	String sOldStr = pFormatter->GetKeyword( nFormatLang, nOld );
	if ( lcl_IsAtEnd( aFormatCode, sOldStr ) )
	{
		// remove old keyword
		aFormatCode.setLength( aFormatCode.getLength() - sOldStr.Len() );

		// add new keyword
		String sNewStr = pFormatter->GetKeyword( nFormatLang, nNew );
		aFormatCode.append( sNewStr );

		return sal_True;	// changed
	}
	return sal_False;		// not found
}

void SvXMLNumFormatContext::AddCondition( const sal_Int32 nIndex )
{
	::rtl::OUString rApplyName = aMyConditions[nIndex].sMapName;
	::rtl::OUString rCondition = aMyConditions[nIndex].sCondition;
	SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
	sal_uInt32 nKey = pData->GetKeyForName( rApplyName );
	OUString sValue = OUString::createFromAscii( "value()" );		//! define constant
	sal_Int32 nValLen = sValue.getLength();

	if ( pFormatter && nKey != NUMBERFORMAT_ENTRY_NOT_FOUND &&
			rCondition.copy( 0, nValLen ) == sValue )
	{
		//!	test for valid conditions
		//!	test for default conditions

		OUString sRealCond = rCondition.copy( nValLen, rCondition.getLength() - nValLen );
		sal_Bool bDefaultCond = sal_False;

		//!	collect all conditions first and adjust default to >=0, >0 or <0 depending on count
		//!	allow blanks in conditions
		sal_Bool bFirstCond = ( aConditions.getLength() == 0 );
		if ( bFirstCond && aMyConditions.size() == 1 && sRealCond.compareToAscii( ">=0" ) == 0 )
			bDefaultCond = sal_True;

		if ( nType == XML_TOK_STYLES_TEXT_STYLE && nIndex == 2 )
		{
			//	The third condition in a number format with a text part can only be
			//	"all other numbers", the condition string must be empty.
			bDefaultCond = sal_True;
		}

		if (!bDefaultCond)
		{
            sal_Int32 nPos = sRealCond.indexOf( '.' );
            if ( nPos >= 0 )
            {   // #i8026# #103991# localize decimal separator
                const String& rDecSep = GetLocaleData().getNumDecimalSep();
                if ( rDecSep.Len() > 1 || rDecSep.GetChar(0) != '.' )
                    sRealCond = sRealCond.replaceAt( nPos, 1, rDecSep );
            }
			aConditions.append( (sal_Unicode) '[' );
			aConditions.append( sRealCond );
			aConditions.append( (sal_Unicode) ']' );
		}

		const SvNumberformat* pFormat = pFormatter->GetEntry(nKey);
		if ( pFormat )
			aConditions.append( OUString( pFormat->GetFormatstring() ) );

		aConditions.append( (sal_Unicode) ';' );
	}
}

void SvXMLNumFormatContext::AddCondition( const sal_Int32 nIndex, const ::rtl::OUString& rFormat, const LocaleDataWrapper& rData )
{
	::rtl::OUString rCondition = aMyConditions[nIndex].sCondition;
	OUString sValue = OUString::createFromAscii( "value()" );		//! define constant
	sal_Int32 nValLen = sValue.getLength();

	if ( rCondition.copy( 0, nValLen ) == sValue )
	{
		//!	test for valid conditions
		//!	test for default conditions

		OUString sRealCond = rCondition.copy( nValLen, rCondition.getLength() - nValLen );
		sal_Bool bDefaultCond = sal_False;

		//!	collect all conditions first and adjust default to >=0, >0 or <0 depending on count
		//!	allow blanks in conditions
		sal_Bool bFirstCond = ( aConditions.getLength() == 0 );
		if ( bFirstCond && aMyConditions.size() == 1 && sRealCond.compareToAscii( ">=0" ) == 0 )
			bDefaultCond = sal_True;

		if ( nType == XML_TOK_STYLES_TEXT_STYLE && nIndex == 2 )
		{
			//	The third condition in a number format with a text part can only be
			//	"all other numbers", the condition string must be empty.
			bDefaultCond = sal_True;
		}

		if (!bDefaultCond)
		{
            sal_Int32 nPos = sRealCond.indexOf( '.' );
            if ( nPos >= 0 )
            {   // #i8026# #103991# localize decimal separator
                const String& rDecSep = rData.getNumDecimalSep();
                if ( rDecSep.Len() > 1 || rDecSep.GetChar(0) != '.' )
                    sRealCond = sRealCond.replaceAt( nPos, 1, rDecSep );
            }
			aConditions.append( (sal_Unicode) '[' );
			aConditions.append( sRealCond );
			aConditions.append( (sal_Unicode) ']' );
		}

		aConditions.append( rFormat );

		aConditions.append( (sal_Unicode) ';' );
	}
}

void SvXMLNumFormatContext::AddCondition( const ::rtl::OUString& rCondition, const ::rtl::OUString& rApplyName )
{
	MyCondition aCondition;
	aCondition.sCondition = rCondition;
	aCondition.sMapName = rApplyName;
	aMyConditions.push_back(aCondition);
}

void SvXMLNumFormatContext::AddColor( const Color& rColor )
{
	SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
	if (!pFormatter)
		return;

	OUStringBuffer aColName;
	for ( sal_uInt16 i=0; i<XML_NUMF_COLORCOUNT; i++ )
		if ( rColor == aNumFmtStdColors[i] )
		{
			aColName = OUString( pFormatter->GetKeyword( nFormatLang, NF_KEY_FIRSTCOLOR + i ) );
			break;
		}

	if ( aColName.getLength() )
	{
		aColName.insert( 0, (sal_Unicode) '[' );
		aColName.append( (sal_Unicode) ']' );
		aFormatCode.insert( 0, aColName.makeStringAndClear() );
	}
}

void SvXMLNumFormatContext::UpdateCalendar( const ::rtl::OUString& rNewCalendar )
{
	if ( rNewCalendar != sCalendar )
	{
		sCalendar = rNewCalendar;
		if ( sCalendar.getLength() )
		{
			aFormatCode.appendAscii( "[~" );			// intro for calendar code
			aFormatCode.append( sCalendar );
			aFormatCode.append( (sal_Unicode) ']' );	// end of "new" currency symbolcalendar code
		}
	}
}

sal_Bool SvXMLNumFormatContext::IsSystemLanguage()
{
    return nFormatLang == LANGUAGE_SYSTEM;
}

//-------------------------------------------------------------------------

//
//	SvXMLNumFmtHelper
//

// #110680#
//SvXMLNumFmtHelper::SvXMLNumFmtHelper(
//						const uno::Reference<util::XNumberFormatsSupplier>& rSupp )
SvXMLNumFmtHelper::SvXMLNumFmtHelper(
	const uno::Reference<util::XNumberFormatsSupplier>& rSupp,
	const uno::Reference<lang::XMultiServiceFactory>& xServiceFactory )
:	mxServiceFactory(xServiceFactory)
{
	DBG_ASSERT( mxServiceFactory.is(), "got no service manager" );

	SvNumberFormatter* pFormatter = NULL;
	SvNumberFormatsSupplierObj* pObj =
					SvNumberFormatsSupplierObj::getImplementation( rSupp );
	if (pObj)
		pFormatter = pObj->GetNumberFormatter();

	// #110680#
	// pData = new SvXMLNumImpData( pFormatter );
	pData = new SvXMLNumImpData( pFormatter, mxServiceFactory );
}

// #110680#
// SvXMLNumFmtHelper::SvXMLNumFmtHelper( SvNumberFormatter* pNumberFormatter )
SvXMLNumFmtHelper::SvXMLNumFmtHelper( 
	SvNumberFormatter* pNumberFormatter,
	const uno::Reference<lang::XMultiServiceFactory>& xServiceFactory )
:	mxServiceFactory(xServiceFactory)
{
	DBG_ASSERT( mxServiceFactory.is(), "got no service manager" );

	// #110680#
	// pData = new SvXMLNumImpData( pNumberFormatter );
	pData = new SvXMLNumImpData( pNumberFormatter, mxServiceFactory );
}

SvXMLNumFmtHelper::~SvXMLNumFmtHelper()
{
	//	remove temporary (volatile) formats from NumberFormatter
	pData->RemoveVolatileFormats();

	delete pData;
}

SvXMLStyleContext*	SvXMLNumFmtHelper::CreateChildContext( SvXMLImport& rImport,
				USHORT nPrefix, const OUString& rLocalName,
				const uno::Reference<xml::sax::XAttributeList>& xAttrList,
				SvXMLStylesContext& rStyles )
{
	SvXMLStyleContext* pContext = NULL;

	const SvXMLTokenMap& rTokenMap = pData->GetStylesElemTokenMap();
	sal_uInt16 nToken = rTokenMap.Get( nPrefix, rLocalName );
	switch (nToken)
	{
		case XML_TOK_STYLES_NUMBER_STYLE:
		case XML_TOK_STYLES_CURRENCY_STYLE:
		case XML_TOK_STYLES_PERCENTAGE_STYLE:
		case XML_TOK_STYLES_DATE_STYLE:
		case XML_TOK_STYLES_TIME_STYLE:
		case XML_TOK_STYLES_BOOLEAN_STYLE:
		case XML_TOK_STYLES_TEXT_STYLE:
			pContext = new SvXMLNumFormatContext( rImport, nPrefix, rLocalName,
													pData, nToken, xAttrList, rStyles );
			break;
	}

	// return NULL if not a data style, caller must handle other elements
	return pContext;
}

const SvXMLTokenMap& SvXMLNumFmtHelper::GetStylesElemTokenMap()
{
	return pData->GetStylesElemTokenMap();
}

/*sal_uInt32 SvXMLNumFmtHelper::GetKeyForName( const ::rtl::OUString& rName )
{
	return pData->GetKeyForName( rName );
}*/


}//end of namespace binfilter
