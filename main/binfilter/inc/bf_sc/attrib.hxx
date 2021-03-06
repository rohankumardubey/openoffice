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



#ifndef SC_SCATTR_HXX
#define SC_SCATTR_HXX

#include <bf_svtools/bf_solar.h>


#ifndef _SFXINTITEM_HXX //autogen
#include <bf_svtools/intitem.hxx>
#endif

#ifndef _SFXENUMITEM_HXX //autogen
#include <bf_svtools/eitem.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
namespace binfilter {

//------------------------------------------------------------------------

										// Flags fuer durch Merge verdeckte Zellen
										// und Control fuer Auto-Filter
#define SC_MF_HOR				1
#define SC_MF_VER				2
#define SC_MF_AUTO				4
#define SC_MF_BUTTON			8
#define SC_MF_SCENARIO			16

#define SC_MF_ALL				31


class EditTextObject;
class SvxBorderLine;

//------------------------------------------------------------------------

class ScMergeAttr: public SfxPoolItem
{
	INT16       nColMerge;
	INT16       nRowMerge;
public:
				TYPEINFO();
				ScMergeAttr();
				ScMergeAttr( INT16 nCol, INT16 nRow = 0);
				ScMergeAttr( const ScMergeAttr& );
				~ScMergeAttr();


	virtual int             operator==( const SfxPoolItem& ) const;
	virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
	virtual SfxPoolItem*    Create( SvStream& rStream, USHORT nVer ) const;
	virtual SvStream&       Store( SvStream& rStream, USHORT nVer ) const;

			INT16           GetColMerge() const {return nColMerge; }
			INT16           GetRowMerge() const {return nRowMerge; }

			BOOL			IsMerged() const { return nColMerge>1 || nRowMerge>1; }

	inline  ScMergeAttr& operator=(const ScMergeAttr& rMerge)
			{
				nColMerge = rMerge.nColMerge;
				nRowMerge = rMerge.nRowMerge;
				return *this;
			}
};

//------------------------------------------------------------------------

class ScMergeFlagAttr: public SfxInt16Item
{
public:
			ScMergeFlagAttr();
			ScMergeFlagAttr(INT16 nFlags);
			~ScMergeFlagAttr();

	BOOL	IsHorOverlapped() const		{ return ( GetValue() & SC_MF_HOR ) != 0;  }
	BOOL	IsVerOverlapped() const		{ return ( GetValue() & SC_MF_VER ) != 0;  }
	BOOL	IsOverlapped() const		{ return ( GetValue() & ( SC_MF_HOR | SC_MF_VER ) ) != 0; }

	BOOL	HasAutoFilter() const		{ return ( GetValue() & SC_MF_AUTO ) != 0; }
	BOOL	HasButton() const			{ return ( GetValue() & SC_MF_BUTTON ) != 0; }

	BOOL	IsScenario() const			{ return ( GetValue() & SC_MF_SCENARIO ) != 0; }
};

//------------------------------------------------------------------------
class ScProtectionAttr: public SfxPoolItem
{
	BOOL        bProtection;    // Zelle schuetzen
	BOOL        bHideFormula;   // Formel nicht Anzeigen
	BOOL        bHideCell;      // Zelle nicht Anzeigen
	BOOL        bHidePrint;     // Zelle nicht Ausdrucken
public:
							TYPEINFO();
							ScProtectionAttr();
							ScProtectionAttr(   BOOL bProtect,
												BOOL bHFormula = FALSE,
												BOOL bHCell = FALSE,
												BOOL bHPrint = FALSE);
							ScProtectionAttr( const ScProtectionAttr& );
							~ScProtectionAttr();


	virtual int             operator==( const SfxPoolItem& ) const;
	virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
	virtual SfxPoolItem*    Create( SvStream& rStream, USHORT nVer ) const;
	virtual SvStream&       Store( SvStream& rStream, USHORT nVer ) const;

	virtual	BOOL			QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
	virtual	BOOL			PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

			BOOL            GetProtection() const { return bProtection; }
			BOOL            GetHideFormula() const { return bHideFormula; }
			BOOL            GetHideCell() const { return bHideCell; }
			BOOL            GetHidePrint() const { return bHidePrint; }
	inline  ScProtectionAttr& operator=(const ScProtectionAttr& rProtection)
			{
				bProtection = rProtection.bProtection;
				bHideFormula = rProtection.bHideFormula;
				bHideCell = rProtection.bHideCell;
				bHidePrint = rProtection.bHidePrint;
				return *this;
			}
};


//----------------------------------------------------------------------------
// ScRangeItem: verwaltet einen Tabellenbereich

#define SCR_INVALID		0x01
#define SCR_ALLTABS		0x02
#define SCR_TONEWTAB	0x04

class ScRangeItem : public SfxPoolItem
{
public:
			TYPEINFO();

			inline	ScRangeItem( const USHORT nWhich );
			inline	ScRangeItem( const USHORT   nWhich,
								 const ScRange& rRange,
								 const USHORT 	nNewFlags = 0 );
			inline	ScRangeItem( const ScRangeItem& rCpy );

	inline ScRangeItem& operator=( const ScRangeItem &rCpy );

	// "pure virtual Methoden" vom SfxPoolItem
	virtual int 				operator==( const SfxPoolItem& ) const;
	virtual SfxPoolItem*		Clone( SfxItemPool *pPool = 0 ) const;
	virtual USHORT				GetVersion( USHORT nFileVersion ) const;
	virtual SfxPoolItem*		Create(SvStream &, USHORT) const;
	virtual SvStream&			Store( SvStream& rStream, USHORT nVer ) const;

	const ScRange&	GetRange() const 				{ return aRange;  }
	void			SetRange( const ScRange& rNew )	{ aRange = rNew; }

	USHORT			GetFlags() const 				{ return nFlags;  }
	void			SetFlags( USHORT nNew )	 		{ nFlags = nNew; }

private:
	ScRange aRange;
	USHORT	nFlags;
};

inline ScRangeItem::ScRangeItem( const USHORT nWhich )
	:	SfxPoolItem( nWhich ), nFlags( SCR_INVALID ) // == ungueltige Area
{
}

inline ScRangeItem::ScRangeItem( const USHORT	nWhich,
								 const ScRange& rRange,
								 const USHORT	nNew )
	: SfxPoolItem( nWhich ), aRange( rRange ), nFlags( nNew )
{
}

inline ScRangeItem::ScRangeItem( const ScRangeItem& rCpy )
	: SfxPoolItem( rCpy.Which() ), aRange( rCpy.aRange ), nFlags( rCpy.nFlags )
{}

inline ScRangeItem& ScRangeItem::operator=( const ScRangeItem &rCpy )
{
	aRange = rCpy.aRange;
	return *this;
}

//----------------------------------------------------------------------------
// ScTableListItem: verwaltet eine Liste von Tabellen
//----------------------------------------------------------------------------
class ScTableListItem : public SfxPoolItem
{
public:
	TYPEINFO();

	inline	ScTableListItem( const USHORT nWhich );
			ScTableListItem( const ScTableListItem& rCpy );
			ScTableListItem( const USHORT nWhich, const List& rList );
			~ScTableListItem();


	// "pure virtual Methoden" vom SfxPoolItem
	virtual int 				operator==( const SfxPoolItem& ) const;
	virtual SfxPoolItem*		Clone( SfxItemPool *pPool = 0 ) const;
	virtual SfxPoolItem*		Create(SvStream &, USHORT) const;
	virtual SvStream&			Store( SvStream& rStream, USHORT nVer ) const;

	void	SetTableList( const List& aList );

public:
	USHORT  nCount;
	USHORT* pTabArr;
};

inline ScTableListItem::ScTableListItem( const USHORT nWhich )
	: SfxPoolItem(nWhich), nCount(0), pTabArr(NULL)
{}

//----------------------------------------------------------------------------
// Seitenformat-Item: Kopf-/Fusszeileninhalte

#define SC_HF_LEFTAREA   1
#define SC_HF_CENTERAREA 2
#define SC_HF_RIGHTAREA  3

class ScPageHFItem : public SfxPoolItem
{
	EditTextObject* pLeftArea;
	EditTextObject* pCenterArea;
	EditTextObject* pRightArea;

public:
				TYPEINFO();
				ScPageHFItem( USHORT nWhich );
				ScPageHFItem( const ScPageHFItem& rItem );
				~ScPageHFItem();

	virtual String          GetValueText() const;
	virtual int             operator==( const SfxPoolItem& ) const;
	virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

	virtual SfxPoolItem*    Create( SvStream& rStream, USHORT nVer ) const;
	virtual SvStream&       Store( SvStream& rStream, USHORT nVer ) const;

	virtual USHORT			GetVersion( USHORT nFileVersion ) const;

	virtual	BOOL			QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
	virtual	BOOL			PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

	const EditTextObject* GetLeftArea() const		{ return pLeftArea; }
	const EditTextObject* GetCenterArea() const		{ return pCenterArea; }
	const EditTextObject* GetRightArea() const		{ return pRightArea; }

	void SetLeftArea( const EditTextObject& rNew );
	void SetCenterArea( const EditTextObject& rNew );
	void SetRightArea( const EditTextObject& rNew );

	//Set mit Uebereignung der Pointer, nArea siehe defines oben
	void SetArea( EditTextObject *pNew, int nArea );
};


//----------------------------------------------------------------------------
// Seitenformat-Item: Kopf-/Fusszeileninhalte

class ScViewObjectModeItem: public SfxEnumItem
{
public:
				TYPEINFO();

				ScViewObjectModeItem( USHORT nWhich );
				ScViewObjectModeItem( USHORT nWhich, ScVObjMode eMode );
				~ScViewObjectModeItem();

	virtual USHORT				GetValueCount() const;
	virtual SfxPoolItem*		Clone( SfxItemPool *pPool = 0 ) const;
	virtual SfxPoolItem*		Create(SvStream &, USHORT) const;
	virtual USHORT				GetVersion( USHORT nFileVersion ) const;
};

//----------------------------------------------------------------------------
//

class ScDoubleItem : public SfxPoolItem
{
public:
				TYPEINFO();
				ScDoubleItem( USHORT nWhich, double nVal=0 );
				ScDoubleItem( const ScDoubleItem& rItem );
				~ScDoubleItem();

	virtual String          GetValueText() const;
	virtual int             operator==( const SfxPoolItem& ) const;
	virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

	virtual SfxPoolItem*    Create( SvStream& rStream, USHORT nVer ) const;
	virtual SvStream&       Store( SvStream& rStream, USHORT nVer ) const;

	double GetValue() const		{ return nValue; }

	void SetValue( const double nVal ) { nValue = nVal;}

private:
	double	nValue;
};


} //namespace binfilter
#endif

