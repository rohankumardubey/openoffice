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


#ifndef _NUMRULE_HXX
#define _NUMRULE_HXX

#include <bf_svtools/bf_solar.h>


#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif
#ifndef _SV_GEN_HXX //autogen wg. Size
#include <tools/gen.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX //autogen
#include <bf_svx/svxenum.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _CALBCK_HXX
#include <calbck.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx> 		// Fuer die inline-ASSERTs
#endif
#ifndef _SWERROR_H
#include <error.h>			// Fuer die inline-ASSERTs
#endif
#ifndef _SVX_NUMITEM_HXX
#include <bf_svx/numitem.hxx>
#endif
class Font; 
namespace binfilter {


class SvxBrushItem;
class SvxNumRule;
class SwCharFmt;
class SwDoc;
class SwFmtVertOrient;
class SwNodeNum;
class SwTxtNode;

extern char __FAR_DATA sOutlineStr[];	// SWG-Filter

inline BYTE GetRealLevel( const BYTE nLvl )
{
	return nLvl & (NO_NUMLEVEL - 1);
}
const sal_Unicode cBulletChar	= 0x2022;	// Charakter fuer Aufzaehlungen

class SwNumFmt : public SvxNumberFormat, public SwClient
{
	SwFmtVertOrient* pVertOrient;

	void UpdateNumNodes( SwDoc* pDoc );
    virtual void NotifyGraphicArrived();
public:
	SwNumFmt();
	SwNumFmt( const SwNumFmt& );
	SwNumFmt( const SvxNumberFormat&, SwDoc* pDoc);

	virtual ~SwNumFmt();

	SwNumFmt& operator=( const SwNumFmt& );
	BOOL operator==( const SwNumFmt& ) const;
	BOOL operator!=( const SwNumFmt& r ) const { return !(*this == r); }

	const Graphic* GetGraphic() const;

	SwCharFmt* GetCharFmt() const { return (SwCharFmt*)pRegisteredIn; }
	void SetCharFmt( SwCharFmt* );
	virtual void Modify( SfxPoolItem* pOld, SfxPoolItem* pNew );

	virtual void			SetCharFmtName(const String& rSet);
	virtual const String&	GetCharFmtName()const;

	virtual void	SetGraphicBrush( const SvxBrushItem* pBrushItem, const Size* pSize = 0, const SvxFrameVertOrient* pOrient = 0);

	virtual void				SetVertOrient(SvxFrameVertOrient eSet);
	virtual SvxFrameVertOrient 	GetVertOrient() const;
    const SwFmtVertOrient*      GetGraphicOrientation() const;
};

enum SwNumRuleType { OUTLINE_RULE = 0, NUM_RULE = 1, RULE_END = 2 };
class SwNumRule
{
	friend void _FinitCore();

	static SwNumFmt* aBaseFmts [ RULE_END ][ MAXLEVEL ];
	static USHORT aDefNumIndents[ MAXLEVEL ];
	static USHORT nRefCount;
	static Font* pDefBulletFont;
	static char* pDefOutlineName;

	SwNumFmt* aFmts[ MAXLEVEL ];

	String sName;
	SwNumRuleType eRuleType;
	USHORT nPoolFmtId;		// Id-fuer "automatich" erzeugte NumRules
	USHORT nPoolHelpId;		// HelpId fuer diese Pool-Vorlage
	BYTE nPoolHlpFileId; 	// FilePos ans Doc auf die Vorlagen-Hilfen
	BOOL bAutoRuleFlag : 1;
	BOOL bInvalidRuleFlag : 1;
	BOOL bContinusNum : 1;	// Fortlaufende Numerierung - ohne Ebenen
	BOOL bAbsSpaces : 1;	// die Ebenen repraesentieren absol. Einzuege

	static void _MakeDefBulletFont();

public:
	SwNumRule( const String& rNm, SwNumRuleType = NUM_RULE,
				BOOL bAutoFlg = TRUE );

	SwNumRule( const SwNumRule& );
	~SwNumRule();

	SwNumRule& operator=( const SwNumRule& );
	BOOL operator==( const SwNumRule& ) const;
	BOOL operator!=( const SwNumRule& r ) const { return !(*this == r); }

	inline const SwNumFmt* GetNumFmt( USHORT i ) const;
	inline const SwNumFmt& Get( USHORT i ) const;
	void Set( USHORT i, const SwNumFmt* );
	void Set( USHORT i, const SwNumFmt& );
	String MakeNumString( const SwNodeNum&, BOOL bInclStrings = TRUE,
							BOOL bOnlyArabic = FALSE ) const;

	inline sal_Unicode GetBulletChar( const SwNodeNum& ) const;
	inline const Font* GetBulletFont( const SwNodeNum& ) const;
	static inline const Font& GetDefBulletFont();

	static char* GetOutlineRuleName() { return pDefOutlineName; }

	static inline USHORT GetNumIndent( BYTE nLvl );
	static inline USHORT GetBullIndent( BYTE nLvl );

	SwNumRuleType GetRuleType() const 			{ return eRuleType; }
	void SetRuleType( SwNumRuleType eNew ) 		{ eRuleType = eNew;
												  bInvalidRuleFlag = TRUE; }

	// eine Art Copy-Constructor, damit die Num-Formate auch an den
	// richtigen CharFormaten eines Dokumentes haengen !!
	// (Kopiert die NumFormate und returnt sich selbst)
	SwNumRule& CopyNumRule( SwDoc*, const SwNumRule& );

	// testet ob die CharFormate aus dem angegeben Doc sind und kopiert
	// die gegebenfalls
	void CheckCharFmts( SwDoc* pDoc );

	// test ob der Einzug von dieser Numerierung kommt.
	BOOL IsRuleLSpace( SwTxtNode& rNd ) const;

	const String& GetName() const 		{ return sName; }
	void SetName( const String& rNm )	{ sName = rNm; }

	BOOL IsAutoRule() const 			{ return bAutoRuleFlag; }
	void SetAutoRule( BOOL bFlag )		{ bAutoRuleFlag = bFlag; }

	BOOL IsInvalidRule() const 			{ return bInvalidRuleFlag; }
	void SetInvalidRule( BOOL bFlag )	{ bInvalidRuleFlag = bFlag; }

	BOOL IsContinusNum() const 			{ return bContinusNum; }
	void SetContinusNum( BOOL bFlag )	{ bContinusNum = bFlag; }

	BOOL IsAbsSpaces() const 			{ return bAbsSpaces; }
	void SetAbsSpaces( BOOL bFlag )		{ bAbsSpaces = bFlag; }

	// erfragen und setzen der Poolvorlagen-Id's
	USHORT GetPoolFmtId() const			{ return nPoolFmtId; }
	void SetPoolFmtId( USHORT nId ) 	{ nPoolFmtId = nId; }

	// erfragen und setzen der Hilfe-Id's fuer die Document-Vorlagen
	USHORT GetPoolHelpId() const 		{ return nPoolHelpId; }
	void SetPoolHelpId( USHORT nId ) 	{ nPoolHelpId = nId; }
	BYTE GetPoolHlpFileId() const 		{ return nPoolHlpFileId; }
	void SetPoolHlpFileId( BYTE nId ) 	{ nPoolHlpFileId = nId; }

	/**  
        #109308# Sets adjustment in all formats of the numbering rule. 

        @param eNum adjustment to be set
    */
    void SetNumAdjust(SvxAdjust eNum);

	void		SetSvxRule(const SvxNumRule&, SwDoc* pDoc);
	SvxNumRule	MakeSvxNumRule() const;
};


class SwNodeNum
{
	USHORT nLevelVal[ MAXLEVEL ];		// Nummern aller Levels
	USHORT nSetValue;					// vorgegeben Nummer
	BYTE nMyLevel;						// akt. Level
	BOOL bStartNum;						// Numerierung neu starten

public:
	inline SwNodeNum( BYTE nLevel = NO_NUM, USHORT nSetVal = USHRT_MAX );
	inline SwNodeNum& operator=( const SwNodeNum& rCpy );

	BOOL operator==( const SwNodeNum& ) const;

	BYTE GetLevel() const 					{ return nMyLevel; }
	void SetLevel( BYTE nVal )  			{ nMyLevel = nVal; }

	BOOL IsStart() const					{ return bStartNum; }
	void SetStart( BOOL bFlag = TRUE ) 		{ bStartNum = bFlag; }

	USHORT GetSetValue() const 				{ return nSetValue; }
	void SetSetValue( USHORT nVal )  		{ nSetValue = nVal; }

	const USHORT* GetLevelVal() const 		{ return nLevelVal; }
		  USHORT* GetLevelVal() 	 		{ return nLevelVal; }
};




// ------------ inline Methoden ----------------------------

inline const SwNumFmt& SwNumRule::Get( USHORT i ) const
{
	ASSERT_ID( i < MAXLEVEL && eRuleType < RULE_END, ERR_NUMLEVEL);
	return aFmts[ i ] ? *aFmts[ i ]
					  : *aBaseFmts[ eRuleType ][ i ];
}

inline const SwNumFmt* SwNumRule::GetNumFmt( USHORT i ) const
{
	ASSERT_ID( i < MAXLEVEL && eRuleType < RULE_END, ERR_NUMLEVEL);
	return aFmts[ i ];
}
inline const Font& SwNumRule::GetDefBulletFont()
{
	if( !pDefBulletFont )
		SwNumRule::_MakeDefBulletFont();
	return *pDefBulletFont;
}

inline USHORT SwNumRule::GetNumIndent( BYTE nLvl )
{
	ASSERT( MAXLEVEL > nLvl, "NumLevel is out of range" );
	return aDefNumIndents[ nLvl ];
}
inline USHORT SwNumRule::GetBullIndent( BYTE nLvl )
{
	ASSERT( MAXLEVEL > nLvl, "NumLevel is out of range" );
	return aDefNumIndents[ nLvl ];
}

inline sal_Unicode SwNumRule::GetBulletChar( const SwNodeNum& rNum ) const
{
	return Get( rNum.GetLevel() & ~NO_NUMLEVEL ).GetBulletChar();
}
inline const Font* SwNumRule::GetBulletFont( const SwNodeNum& rNum ) const
{
	return Get( rNum.GetLevel() & ~NO_NUMLEVEL ).GetBulletFont();
}



SwNodeNum::SwNodeNum( BYTE nLevel, USHORT nSetVal )
	: nSetValue( nSetVal ), nMyLevel( nLevel ), bStartNum( FALSE )
{
	memset( nLevelVal, 0, sizeof( nLevelVal ) );
}

inline SwNodeNum& SwNodeNum::operator=( const SwNodeNum& rCpy )
{
	nSetValue = rCpy.nSetValue;
	nMyLevel = rCpy.nMyLevel;
	bStartNum = rCpy.bStartNum;

	memcpy( nLevelVal, rCpy.nLevelVal, sizeof( nLevelVal ) );
	return *this;
}


} //namespace binfilter
#endif	// _NUMRULE_HXX
