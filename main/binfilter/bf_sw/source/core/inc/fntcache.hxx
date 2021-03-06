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


#ifndef _FNTCACHE_HXX
#define _FNTCACHE_HXX

#include <bf_svtools/bf_solar.h>


#ifndef _SV_FONT_HXX //autogen
#include <vcl/font.hxx>
#endif
#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif

#include "swtypes.hxx"
#include "swcache.hxx"
/*N*/ #include <tools/debug.hxx>  //for stripping
class Printer; 
class OutputDevice; 
class FontMetric; 
namespace binfilter {

class SwFntObj;
class SwDrawTextInfo;	// DrawText
class SwScriptInfo;
class ViewShell;
class SwSubFont;

/*************************************************************************
 *                      class SwFntCache
 *************************************************************************/

class SwFntCache : public SwCache
{
public:

	inline SwFntCache() : SwCache(50,50
#ifdef DBG_UTIL
	, ByteString( RTL_CONSTASCII_STRINGPARAM(
						"Globaler Font-Cache pFntCache" ))
#endif
	) {}

	inline SwFntObj *First( ) { return (SwFntObj *)SwCache::First(); }
	inline SwFntObj *Next( SwFntObj *pFntObj)
		{ return (SwFntObj *)SwCache::Next( (SwCacheObj *)pFntObj ); }
	void Flush();
};

// Font-Cache, globale Variable, in txtinit.Cxx angelegt/zerstoert
extern SwFntCache *pFntCache;
extern SwFntObj *pLastFont;
extern BYTE *pMagicNo;
extern Color *pWaveCol;

/*************************************************************************
 *                      class SwFntObj
 *************************************************************************/

class SwFntObj : public SwCacheObj
{
	friend class SwFntAccess;
	friend void _InitCore();
	friend void _FinitCore();

	Font aFont;
	Font *pScrFont;
	Font *pPrtFont;
    OutputDevice* pPrinter;
	USHORT nLeading;
	USHORT nScrAscent;
	USHORT nPrtAscent;
	USHORT nScrHeight;
	USHORT nPrtHeight;
	USHORT nPropWidth;
	USHORT nZoom;
	BOOL bSymbol : 1;
	BOOL bPaintBlank : 1;
    void ChooseFont( ViewShell *pSh, OutputDevice *pOut );

	static long nPixWidth;
	static MapMode *pPixMap;
	static OutputDevice *pPixOut;

public:
	DECL_FIXEDMEMPOOL_NEWDEL(SwFntObj)

	SwFntObj( const SwSubFont &rFont, const void* pOwner,
			  ViewShell *pSh );

	virtual ~SwFntObj();

	inline 		 Font *GetScrFont()		{ return pScrFont; }
	inline 		 Font *GetFont()		{ return &aFont; }
	inline const Font *GetFont() const  { return &aFont; }

	inline USHORT GetLeading() const  { return nLeading; }

    void GuessLeading( const ViewShell *pSh, const FontMetric& rMet );
    USHORT GetAscent( const ViewShell *pSh, const OutputDevice *pOut );
    USHORT GetHeight( const ViewShell *pSh, const OutputDevice *pOut );

    void SetDevFont( const ViewShell *pSh, OutputDevice *pOut );
    inline OutputDevice* GetPrt() const { return pPrinter; }
	inline USHORT	GetZoom() const { return nZoom; }
	inline USHORT	GetPropWidth() const { return nPropWidth; }
	inline BOOL		IsSymbol() const { return bSymbol; }

	Size  GetTextSize( SwDrawTextInfo &rInf );

    void CreateScrFont( const ViewShell *pSh, const OutputDevice& rOut );
    void CreatePrtFont( const OutputDevice& rOut );
};

/*************************************************************************
 *                      class SwFntAccess
 *************************************************************************/


class SwFntAccess : public SwCacheAccess
{
	ViewShell *pShell;
protected:
	virtual SwCacheObj *NewObj( );

public:
	SwFntAccess( const void * &rMagic, USHORT &rIndex, const void *pOwner,
				 ViewShell *pShell,
				 BOOL bCheck = FALSE  );
    inline SwFntObj* Get() { return (SwFntObj*) SwCacheAccess::Get(); };
};


} //namespace binfilter
#endif
