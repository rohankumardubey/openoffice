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


#ifndef _PAGEDESC_HXX
#define _PAGEDESC_HXX

#include <bf_svtools/bf_solar.h>

#ifndef _FRACT_HXX
#include <tools/fract.hxx>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>	//fuer SwTwips
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
namespace binfilter {

class SfxPoolItem;
class SwTxtFmtColl;
class SwNode;

//Separator line adjustment
enum SwFtnAdj
{
	FTNADJ_LEFT,
	FTNADJ_CENTER,
	FTNADJ_RIGHT
};

//footnote information
class SwPageFtnInfo
{
	SwTwips 	nMaxHeight;		//maximum height of the footnote area.
	ULONG		nLineWidth;		//width of separator line
	Color		aLineColor;		//color of the separator line
	Fraction    aWidth;			//percentage width of the separator line.
	SwFtnAdj	eAdj;			//line adjustment.
	SwTwips		nTopDist;		//distance between body and separator.
	SwTwips		nBottomDist;	//distance between separator and first footnote
public:
	SwTwips   	GetHeight() const 		{ return nMaxHeight; }
	ULONG 			GetLineWidth() const { return nLineWidth; }
	const Color& 	GetLineColor() const { return aLineColor;}
	const Fraction&	GetWidth() const 	{ return aWidth; }
	SwFtnAdj	GetAdj()	const 		{ return eAdj; }
	SwTwips		GetTopDist()const 		{ return nTopDist; }
	SwTwips		GetBottomDist() const 	{ return nBottomDist; }

	void SetHeight( SwTwips  nNew ) 	{ nMaxHeight = nNew; }
	void SetLineWidth(ULONG nSet  ) 	{ nLineWidth = nSet; }
	void SetLineColor(const Color& rCol )  { aLineColor = rCol;}
	void SetWidth( const Fraction &rNew){ aWidth = rNew; }
	void SetAdj   ( SwFtnAdj eNew ) 	{ eAdj = eNew; }
	void SetTopDist   ( SwTwips nNew ) 	{ nTopDist = nNew; }
	void SetBottomDist( SwTwips nNew ) 	{ nBottomDist = nNew; }

	SwPageFtnInfo();
	SwPageFtnInfo( const SwPageFtnInfo& );
	SwPageFtnInfo& operator=( const SwPageFtnInfo& );

	BOOL operator ==( const SwPageFtnInfo& ) const;
};

/*
 * Verwendung des UseOnPage (eUse) und der FrmFmt'e
 *
 *	RIGHT	- aMaster nur fuer rechte Seiten, linke  Seiten immer leer.
 *	LEFT	- aLeft fuer linke  Seiten, rechte Seiten immer leer.
 *			  aLeft ist eine Kopie des Master.
 *  ALL		- aMaster fuer rechte Seiten, aLeft fuer Linke Seiten.
 *			  aLeft ist eine Kopie des Master.
 *	MIRROR	- aMaster fuer rechte Seiten, aLeft fuer linke Seiten.
 *			  aLeft ist eine Kopie des Master, Raender sind gespiegelt.
 *
 * UI dreht auschliesslich am Master! aLeft wird beim Chg am Dokument
 * enstprechend dem eUse eingestellt.
 *
 * Damit es die Filter etwas einfacher haben werden weitere Werte im
 * eUse untergebracht:
 *
 * HEADERSHARE - Headerinhalt auf beiden Seiten gleich
 * FOOTERSHARE - Footerinhalt auf beiden Seiten gleich
 *
 * Die Werte werden bei den entsprechenden Get-/Set-Methden ausmaskiert.
 * Zugriff auf das volle eUse inclusive der Header-Footer information
 * per ReadUseOn(), WriteUseOn() (fuer Filter und CopyCTor)!
 *
 * Die FrmFormate fuer Header/Footer werden anhand der Attribute fuer
 * Header/Footer vom UI am Master eingestellt (Hoehe, Raender, Hintergrund...);
 * Header/Footer fuer die Linke Seite werden entsprechen kopiert bzw.
 * gespielt (Chg am Dokument).
 * Das jew. Attribut fuer den Inhalt wird automatisch beim Chg am
 * Dokument versorgt (entsprechen den SHARE-informationen werden Inhalte
 * erzeugt bzw. entfernt).
 *
 */

enum UseOnPage
{	PD_NONE			  = 0x0000,	//for internal use only.
	PD_LEFT			  = 0x0001,
	PD_RIGHT		  = 0x0002,
	PD_ALL			  = 0x0003,
	PD_MIRROR		  = 0x0007,
	PD_HEADERSHARE    = 0x0040,
	PD_FOOTERSHARE    = 0x0080,
	PD_NOHEADERSHARE  = 0x00BF, //for internal use only
	PD_NOFOOTERSHARE  = 0x007F  //for internal use only
};

class SwPageDesc : public SwModify
{
	friend class SwDoc;

	//nicht (mehr) implementiert.
	SwPageDesc& operator=( const SwPageDesc& );

	String		aDescName;
	SvxNumberType	aNumType;
	SwFrmFmt	aMaster;
	SwFrmFmt	aLeft;
	SwDepend	aDepend;	// wg. Registerhaltigkeit
	SwPageDesc *pFollow;
	USHORT		nRegHeight;	// Zeilenabstand und Fontascent der Vorlage
	USHORT		nRegAscent; // fuer die Registerhaltigkeit
	UseOnPage	eUse;
	BOOL		bLandscape;

	//Fussnoteninformationen
	SwPageFtnInfo aFtnInfo;

	//Wird zum Spiegeln vom Chg (Doc) gerufen.
	//Kein Abgleich an anderer Stelle.
	void Mirror();

	void ResetAllAttr( sal_Bool bLeft );

	SwPageDesc(const String&, SwFrmFmt*, SwDoc *pDc );
public:
	const String &GetName() const { return aDescName; }
		  void 	  SetName( const String& rNewName ) { aDescName = rNewName; }

	BOOL GetLandscape() const { return bLandscape; }
	void SetLandscape( BOOL bNew ) { bLandscape = bNew; }

	const SvxNumberType &GetNumType() const { return aNumType; }
		  void		 	SetNumType( const SvxNumberType& rNew ) { aNumType = rNew; }

	const SwPageFtnInfo &GetFtnInfo() const { return aFtnInfo; }
		  SwPageFtnInfo &GetFtnInfo()		{ return aFtnInfo; }
	void  SetFtnInfo( const SwPageFtnInfo &rNew ) { aFtnInfo = rNew; }

	inline BOOL IsHeaderShared() const;
	inline BOOL IsFooterShared() const;
	inline void ChgHeaderShare( BOOL bNew );
	inline void ChgFooterShare( BOOL bNew );

	inline void		 SetUseOn( UseOnPage eNew );
	inline UseOnPage GetUseOn() const;

	void	  WriteUseOn( UseOnPage eNew ) { eUse = eNew; }
	UseOnPage ReadUseOn () const { return eUse; }

		  SwFrmFmt &GetMaster() { return aMaster; }
		  SwFrmFmt &GetLeft()   { return aLeft; }
	const SwFrmFmt &GetMaster() const { return aMaster; }
	const SwFrmFmt &GetLeft()   const { return aLeft; }

	// Reset all attrs of the format but keep the ones a pagedesc
	// cannot live without.
	inline void ResetAllMasterAttr();
	inline void ResetAllLeftAttr();

	//Mit den folgenden Methoden besorgt sich das Layout ein Format
	//um eine Seite erzeugen zu koennen
	inline SwFrmFmt *GetRightFmt();
	inline const SwFrmFmt *GetRightFmt() const;
	inline SwFrmFmt *GetLeftFmt();
	inline const SwFrmFmt *GetLeftFmt() const;

	USHORT GetRegHeight() const { return nRegHeight; }
	USHORT GetRegAscent() const { return nRegAscent; }
	void SetRegHeight( USHORT nNew ){ nRegHeight = nNew; }
	void SetRegAscent( USHORT nNew ){ nRegAscent = nNew; }

	inline void SetFollow( const SwPageDesc* pNew );
	const SwPageDesc* GetFollow() const { return pFollow; }
		  SwPageDesc* GetFollow() { return pFollow; }

	void SetRegisterFmtColl( const SwTxtFmtColl* rFmt );
	const SwTxtFmtColl* GetRegisterFmtColl() const;
	virtual void Modify( SfxPoolItem *pOldValue, SfxPoolItem *pNewValue );
	void RegisterChange();

	// erfragen und setzen der PoolFormat-Id
	USHORT GetPoolFmtId() const 		{ return aMaster.GetPoolFmtId(); }
	void SetPoolFmtId( USHORT nId ) 	{ aMaster.SetPoolFmtId( nId ); }
	USHORT GetPoolHelpId() const 		{ return aMaster.GetPoolHelpId(); }
	void SetPoolHelpId( USHORT nId ) 	{ aMaster.SetPoolHelpId( nId ); }
	BYTE GetPoolHlpFileId() const 		{ return aMaster.GetPoolHlpFileId(); }
	void SetPoolHlpFileId( BYTE nId )	{ aMaster.SetPoolHlpFileId( nId ); }
	// erfrage die Attribut-Beschreibung, returnt den reingereichten String
	void GetPresentation( SfxItemPresentation ePres,
		SfxMapUnit eCoreMetric,	SfxMapUnit ePresMetric,	String &rText ) const;

		// erfrage vom Client Informationen


	//Given a SwNode return the pagedesc in use at that location.
	static const SwPageDesc* GetPageDescOfNode(const SwNode& rNd);

	SwPageDesc( const SwPageDesc& );
	~SwPageDesc();
};

inline void SwPageDesc::SetFollow( const SwPageDesc* pNew )
{
	pFollow = pNew ? (SwPageDesc*)pNew : this;
}

inline BOOL SwPageDesc::IsHeaderShared() const
{
	return eUse & PD_HEADERSHARE ? TRUE : FALSE;
}
inline BOOL SwPageDesc::IsFooterShared() const
{
	return eUse & PD_FOOTERSHARE ? TRUE : FALSE;
}
inline void SwPageDesc::ChgHeaderShare( BOOL bNew )
{
	if ( bNew )
		eUse = (UseOnPage) (eUse | PD_HEADERSHARE);
		// (USHORT&)eUse |= (USHORT)PD_HEADERSHARE;
	else
		eUse = (UseOnPage) (eUse & PD_NOHEADERSHARE);
		// (USHORT&)eUse &= (USHORT)PD_NOHEADERSHARE;
}
inline void SwPageDesc::ChgFooterShare( BOOL bNew )
{
	if ( bNew )
		eUse = (UseOnPage) (eUse | PD_FOOTERSHARE);
		// (USHORT&)eUse |= (USHORT)PD_FOOTERSHARE;
	else
		eUse = (UseOnPage) (eUse & PD_NOFOOTERSHARE);
		// (USHORT&)eUse &= (USHORT)PD_NOFOOTERSHARE;
}
inline void	SwPageDesc::SetUseOn( UseOnPage eNew )
{
	UseOnPage eTmp = PD_NONE;
	if ( eUse & PD_HEADERSHARE )
		eTmp = PD_HEADERSHARE;
		// (USHORT&)eTmp |= (USHORT)PD_HEADERSHARE;
	if ( eUse & PD_FOOTERSHARE )
		eTmp = (UseOnPage) (eTmp | PD_FOOTERSHARE);
		// (USHORT&)eTmp |= (USHORT)PD_FOOTERSHARE;
	eUse = (UseOnPage) (eTmp | eNew);
	// (USHORT&)eUse = eTmp | eNew;
}
inline UseOnPage SwPageDesc::GetUseOn() const
{
	UseOnPage eRet = eUse;
	eRet = (UseOnPage) (eRet & PD_NOHEADERSHARE);
	// (USHORT&)eRet &= (USHORT)PD_NOHEADERSHARE;
	eRet = (UseOnPage) (eRet & PD_NOFOOTERSHARE);
	// (USHORT&)eRet &= (USHORT)PD_NOFOOTERSHARE;
	return eRet;
}

inline void SwPageDesc::ResetAllMasterAttr()
{
	ResetAllAttr( sal_False );
}

inline void SwPageDesc::ResetAllLeftAttr()
{
	ResetAllAttr( sal_True );
}

inline SwFrmFmt *SwPageDesc::GetRightFmt()
{
	return PD_RIGHT & eUse ? &aMaster : 0;
}
inline const SwFrmFmt *SwPageDesc::GetRightFmt() const
{
	return PD_RIGHT & eUse ? &aMaster : 0;
}
inline SwFrmFmt *SwPageDesc::GetLeftFmt()
{
	return PD_LEFT & eUse ? &aLeft : 0;
}
inline const SwFrmFmt *SwPageDesc::GetLeftFmt() const
{
	return PD_LEFT & eUse ? &aLeft : 0;
}

} //namespace binfilter
#endif	//_PAGEDESC_HXX
