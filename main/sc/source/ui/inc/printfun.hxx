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



#ifndef SC_PRINTFUN_HXX
#define SC_PRINTFUN_HXX


#include "pagepar.hxx"
#include "editutil.hxx"

#ifndef _PRINT_HXX //autogen
#include <vcl/print.hxx>
#endif

class SfxPrinter;
class SfxProgress;
class ScDocShell;
class ScDocument;
class ScViewData;
class SfxItemSet;
class ScPageHFItem;
class EditTextObject;
class MultiSelection;
class ScHeaderEditEngine;
class ScPageBreakData;
class ScPreviewLocationData;
class ScPrintOptions;
class SvxBoxItem;
class SvxBrushItem;
class SvxShadowItem;
class FmFormView;

#define RANGENO_NORANGE				USHRT_MAX

#define PRINT_HEADER_WIDTH			(1.0 * TWIPS_PER_CM)
#define PRINT_HEADER_HEIGHT 		(12.8 * TWIPS_PER_POINT)
#define PRINT_HEADER_FONTHEIGHT 	200


											//	Einstellungen fuer Kopf-/Fusszeilen
struct ScPrintHFParam
{
	sal_Bool				bEnable;
	sal_Bool				bDynamic;
	sal_Bool				bShared;
	long				nHeight;			//	insgesamt (Hoehe+Abstand+Rahmen)
	long				nManHeight;			//	eingestellte Groesse (Min. bei dynamisch)
	sal_uInt16				nDistance;
	sal_uInt16				nLeft;				//	Raender
	sal_uInt16				nRight;
	const ScPageHFItem* pLeft;
	const ScPageHFItem* pRight;
	const SvxBoxItem*	pBorder;
	const SvxBrushItem* pBack;
	const SvxShadowItem* pShadow;
};


// "Ersatz" fuer SV-JobSetup:

class ScJobSetup
{
public:
	ScJobSetup( SfxPrinter* pPrinter );

	Size		aUserSize;
	MapMode		aUserMapMode;
	Paper	ePaper;
	Orientation eOrientation;
	sal_uInt16		nPaperBin;
};

struct ScPrintState							//	Variablen aus ScPrintFunc retten
{
	SCTAB	nPrintTab;
	SCCOL	nStartCol;
	SCROW	nStartRow;
	SCCOL	nEndCol;
	SCROW	nEndRow;
	sal_uInt16	nZoom;
	size_t	nPagesX;
	size_t	nPagesY;
	long	nTabPages;
	long	nTotalPages;
	long	nPageStart;
	long	nDocPages;
};

class ScPageRowEntry
{
private:
	SCROW	nStartRow;
	SCROW	nEndRow;
	size_t	nPagesX;
	sal_Bool*	pHidden;
	//!		Anzahl wirklich sichtbarer cachen???

public:
			ScPageRowEntry()	{ nStartRow = nEndRow = 0; nPagesX = 0; pHidden = NULL; }
			~ScPageRowEntry()	{ delete[] pHidden; }

			ScPageRowEntry(const ScPageRowEntry& r);
	const ScPageRowEntry& operator=(const ScPageRowEntry& r);

	SCROW	GetStartRow() const		{ return nStartRow; }
	SCROW	GetEndRow() const		{ return nEndRow; }
	size_t	GetPagesX() const		{ return nPagesX; }
	void	SetStartRow(SCROW n)	{ nStartRow = n; }
	void	SetEndRow(SCROW n)		{ nEndRow = n; }

	void	SetPagesX(size_t nNew);
	void	SetHidden(size_t nX);
	sal_Bool	IsHidden(size_t nX) const;

	size_t	CountVisible() const;
};

class ScPrintFunc
{
private:
	ScDocShell* 		pDocShell;
	ScDocument* 		pDoc;
	SfxPrinter* 		pPrinter;
	OutputDevice*		pDev;
	FmFormView*			pDrawView;

	MapMode				aOldPrinterMode;	//	MapMode vor dem Aufruf

	Point				aSrcOffset;			//	Papier-1/100 mm
	Point				aOffset;			//	mit Faktor aus Seitenformat skaliert
	sal_uInt16				nManualZoom;		//	Zoom in Preview (Prozent)
	sal_Bool				bClearWin;			//	Ausgabe vorher loeschen
	sal_Bool				bUseStyleColor;
	sal_Bool				bIsRender;

	SCTAB				nPrintTab;
	long				nPageStart;			//	Offset fuer erste Seite
	long				nDocPages;			//	Seiten im Dokument

	const ScRange*		pUserArea;			//	Selektion, wenn im Dialog eingestellt

	const SfxItemSet*	pParamSet;			//	eingestellte Vorlage
	sal_Bool				bState;				//	aus State-struct erzeugt

											//	Parameter aus Vorlage:
	sal_uInt16				nLeftMargin;
	sal_uInt16				nTopMargin;
	sal_uInt16				nRightMargin;
	sal_uInt16				nBottomMargin;
	sal_Bool				bCenterHor;
	sal_Bool				bCenterVer;
	sal_Bool				bLandscape;
	sal_Bool				bSourceRangeValid;

	sal_uInt16				nPageUsage;
	Size				aPageSize;			//	Drucker-Twips
	const SvxBoxItem*	pBorderItem;
	const SvxBrushItem* pBackgroundItem;
	const SvxShadowItem* pShadowItem;

	ScRange				aLastSourceRange;
	ScPrintHFParam		aHdr;
	ScPrintHFParam		aFtr;
	ScPageTableParam	aTableParam;
	ScPageAreaParam 	aAreaParam;

											//	berechnete Werte:
	sal_uInt16				nZoom;
	sal_Bool				bPrintCurrentTable;
	sal_Bool				bMultiArea;
	long				nTabPages;
	long				nTotalPages;

	Rectangle			aPageRect;			//	Dokument-Twips

	MapMode 			aLogicMode; 		//	in DoPrint gesetzt
	MapMode 			aOffsetMode;
	MapMode 			aTwipMode;
	double				nScaleX;
	double				nScaleY;

	SCCOL				nRepeatStartCol;
	SCCOL				nRepeatEndCol;
	SCROW				nRepeatStartRow;
	SCROW				nRepeatEndRow;

	SCCOL				nStartCol;
	SCROW				nStartRow;
	SCCOL				nEndCol;
	SCROW				nEndRow;

	SCCOL*              pPageEndX;			// Seitenaufteilung
	SCROW*              pPageEndY;
	ScPageRowEntry*		pPageRows;
	size_t				nPagesX;
	size_t				nPagesY;
	size_t				nTotalY;

	ScHeaderEditEngine*	pEditEngine;
	SfxItemSet* 		pEditDefaults;

	ScHeaderFieldData	aFieldData;

	List				aNotePosList;		//	Reihenfolge der Notizen

	ScPageBreakData*	pPageData;			// zum Eintragen der Umbrueche etc.

public:
					ScPrintFunc( ScDocShell* pShell, SfxPrinter* pNewPrinter, SCTAB nTab,
								 long nPage = 0, long nDocP = 0,
								 const ScRange* pArea = NULL,
								 const ScPrintOptions* pOptions = NULL,
								 ScPageBreakData* pData = NULL );

					// ctors for device other than printer - for preview and pdf:

					ScPrintFunc( OutputDevice* pOutDev, ScDocShell* pShell, SCTAB nTab,
								 long nPage = 0, long nDocP = 0,
								 const ScRange* pArea = NULL,
								 const ScPrintOptions* pOptions = NULL );

					ScPrintFunc( OutputDevice* pOutDev, ScDocShell* pShell,
								 const ScPrintState& rState,
								 const ScPrintOptions* pOptions );

					~ScPrintFunc();

	static void 	DrawToDev( ScDocument* pDoc, OutputDevice* pDev, double nPrintFactor,
								const Rectangle& rBound, ScViewData* pViewData, sal_Bool bMetaFile );

	void			SetDrawView( FmFormView* pNew );

	void			SetOffset( const Point& rOfs );
	void			SetManualZoom( sal_uInt16 nNewZoom );
	void			SetDateTime( const Date& rDate, const Time& rTime );

	void			SetClearFlag( sal_Bool bFlag );
	void			SetUseStyleColor( sal_Bool bFlag );
	void			SetRenderFlag( sal_Bool bFlag );

    void            SetExclusivelyDrawOleAndDrawObjects();//for printing selected objects without surrounding cell contents

	sal_Bool			UpdatePages();

	void			ApplyPrintSettings();		// aus DoPrint() schon gerufen
	long			DoPrint( const MultiSelection& rPageRanges,
                                long nStartPage, long nDisplayStart, sal_Bool bDoPrint,
                                ScPreviewLocationData* pLocationData );

					//	Werte abfragen - sofort

	Size			GetPageSize() const { return aPageSize; }
	Size			GetDataSize() const;
	void			GetScaleData( Size& rPhysSize, long& rDocHdr, long& rDocFtr );
	long			GetFirstPageNo() const	{ return aTableParam.nFirstPageNo; }

					//	letzte Werte abfragen - nach DoPrint !!!

	double			GetScaleX() const { return nScaleX; }
	double			GetScaleY() const { return nScaleY; }
	long			GetTotalPages() const { return nTotalPages; }
	sal_uInt16			GetZoom() const { return nZoom; }

	void			ResetBreaks( SCTAB nTab );

	void			GetPrintState( ScPrintState& rState );
	sal_Bool			GetLastSourceRange( ScRange& rRange ) const;
    sal_uInt16          GetLeftMargin() const{return nLeftMargin;}
    sal_uInt16          GetRightMargin() const{return nRightMargin;}
    sal_uInt16          GetTopMargin() const{return nTopMargin;}
    sal_uInt16          GetBottomMargin() const{return nBottomMargin;}
    void            SetLeftMargin(sal_uInt16 nRulerLeftDistance){ nLeftMargin = nRulerLeftDistance; }
    void            SetRightMargin(sal_uInt16 nRulerRightDistance){ nRightMargin = nRulerRightDistance; }
    void            SetTopMargin(sal_uInt16 nRulerTopDistance){ nTopMargin = nRulerTopDistance; }
    void            SetBottomMargin(sal_uInt16 nRulerBottomDistance){ nBottomMargin = nRulerBottomDistance; }
    ScPrintHFParam  GetHeader(){return aHdr;}
    ScPrintHFParam  GetFooter(){return aFtr;}

private:
	void			Construct( const ScPrintOptions* pOptions );
	void			InitParam( const ScPrintOptions* pOptions );
	void			CalcZoom( sal_uInt16 nRangeNo );
	void			CalcPages();
	long			CountPages();
	long			CountNotePages();

	sal_Bool			AdjustPrintArea( sal_Bool bNew );

	Size			GetDocPageSize();

	long			TextHeight( const EditTextObject* pObject );
	void			MakeEditEngine();
	void			UpdateHFHeight( ScPrintHFParam& rParam );

	void			InitModes();

	sal_Bool			IsLeft( long nPageNo );
	sal_Bool			IsMirror( long nPageNo );
	void			ReplaceFields( long nPageNo );		// aendert Text in pEditEngine
	void			MakeTableString();				 	// setzt aTableStr

	void			PrintPage( long nPageNo,
									SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
									sal_Bool bDoPrint, ScPreviewLocationData* pLocationData );
	void			PrintArea( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
									long nScrX, long nScrY,
									sal_Bool bShLeft, sal_Bool bShTop, sal_Bool bShRight, sal_Bool bShBottom );
	void			LocateArea( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
									long nScrX, long nScrY, sal_Bool bRepCol, sal_Bool bRepRow,
									ScPreviewLocationData& rLocationData );
	void			PrintColHdr( SCCOL nX1, SCCOL nX2, long nScrX, long nScrY );
	void			PrintRowHdr( SCROW nY1, SCROW nY2, long nScrX, long nScrY );
	void			LocateColHdr( SCCOL nX1, SCCOL nX2, long nScrX, long nScrY,
								sal_Bool bRepCol, ScPreviewLocationData& rLocationData );
	void			LocateRowHdr( SCROW nY1, SCROW nY2, long nScrX, long nScrY,
								sal_Bool bRepRow, ScPreviewLocationData& rLocationData );
	void			PrintHF( long nPageNo, sal_Bool bHeader, long nStartY,
									sal_Bool bDoPrint, ScPreviewLocationData* pLocationData );

	long			PrintNotes( long nPageNo, long nNoteStart, sal_Bool bDoPrint, ScPreviewLocationData* pLocationData );
	long			DoNotes( long nNoteStart, sal_Bool bDoPrint, ScPreviewLocationData* pLocationData );

	void			DrawBorder( long nScrX, long nScrY, long nScrW, long nScrH,
									const SvxBoxItem* pBorderData,
									const SvxBrushItem* pBackground,
									const SvxShadowItem* pShadow );

	void			FillPageData();
};



#endif

