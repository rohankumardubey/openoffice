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


#ifndef _NDHINTS_HXX
#define _NDHINTS_HXX


#include <svl/svarray.hxx>
#include <tools/mempool.hxx>

#include "swtypes.hxx"


class SwTxtNode;
class SwRegHistory;                 // steht im RolBck.hxx
class SwTxtAttr;
class SwTxtAttrNesting;

class SfxPoolItem;
class SfxItemSet;
class SwDoc;

typedef enum {
    COPY = true,
    NEW  = false,
} CopyOrNew_t;

// if COPY then pTxtNode must be given!
SW_DLLPRIVATE SwTxtAttr *
MakeTxtAttr( SwDoc & rDoc, SfxPoolItem & rNew,
        xub_StrLen const nStt, xub_StrLen const nEnd,
        CopyOrNew_t const bIsCopy = NEW, SwTxtNode *const pTxtNode = 0);
SW_DLLPRIVATE SwTxtAttr *
MakeTxtAttr( SwDoc & rDoc, const SfxItemSet & rSet,
        xub_StrLen nStt, xub_StrLen nEnd );

// create redline dummy text hint that must not be inserted into hints array
SW_DLLPRIVATE SwTxtAttr*
MakeRedlineTxtAttr( SwDoc & rDoc, SfxPoolItem& rAttr );


/*
 * Ableitung der Klasse SwpHints ueber den Umweg ueber SwpHts, da
 * lediglich die Klasse SwTxtNode Attribute einfuegen und
 * loeschen koennen soll. Anderen Klassen wie den Frames steht
 * lediglich ein lesender Zugriff ueber den Index-Operator zur
 * Verfuegung.
 * Groesse beim Anlegen gleich 1, weil nur dann ein Array erzeug wird, wenn
 * auch ein Hint eingefuegt wird.
 */

/*************************************************************************
 *                      class SwpHtStart/End
 *************************************************************************/

SV_DECL_PTRARR_SORT(SwpHtStart,SwTxtAttr*,1,1)
SV_DECL_PTRARR_SORT(SwpHtEnd,SwTxtAttr*,1,1)

/*************************************************************************
 *                      class SwpHintsArr
 *************************************************************************/

/// the Hints array
class SwpHintsArray
{

protected:
    SwpHtStart m_HintStarts;
    SwpHtEnd   m_HintEnds;

    //FIXME: why are the non-const methods public?
public:
    void Insert( const SwTxtAttr *pHt );
    void DeleteAtPos( const sal_uInt16 nPosInStart );
    bool Resort();
    SwTxtAttr * Cut( const sal_uInt16 nPosInStart );

    inline const SwTxtAttr * GetStart( const sal_uInt16 nPos ) const
        { return m_HintStarts[nPos]; }
    inline const SwTxtAttr * GetEnd  ( const sal_uInt16 nPos ) const
        { return m_HintEnds  [nPos]; }
    inline       SwTxtAttr * GetStart( const sal_uInt16 nPos )
        { return m_HintStarts[nPos]; }
    inline       SwTxtAttr * GetEnd  ( const sal_uInt16 nPos )
        { return m_HintEnds  [nPos]; }

    inline sal_uInt16 GetEndCount()   const { return m_HintEnds  .Count(); }
    inline sal_uInt16 GetStartCount() const { return m_HintStarts.Count(); }

    inline sal_uInt16 GetStartOf( const SwTxtAttr *pHt ) const;
    inline sal_uInt16 GetPos( const SwTxtAttr *pHt ) const
        { return m_HintStarts.GetPos( pHt ); }

    inline SwTxtAttr * GetTextHint( const sal_uInt16 nIdx )
        { return GetStart(nIdx); }
    inline const SwTxtAttr * operator[]( const sal_uInt16 nIdx ) const
        { return m_HintStarts[nIdx]; }
    inline sal_uInt16 Count() const { return m_HintStarts.Count(); }

#ifdef DBG_UTIL
    bool Check() const;
#endif
};

/*************************************************************************
 *                      class SwpHints
 *************************************************************************/

// public interface
class SwpHints : public SwpHintsArray
{
private:
    SwRegHistory* m_pHistory;   // for Undo

    bool m_bFontChange          : 1;  // font change
    // true: the Node is in Split and Frames are moved
    bool m_bInSplitNode         : 1;
    // m_bHasHiddenParaField is invalid, call CalcHiddenParaField()
    bool m_bCalcHiddenParaField : 1;
    bool m_bHasHiddenParaField  : 1;  // HiddenParaFld
    bool m_bFootnote            : 1;  // footnotes
    bool m_bDDEFields           : 1;  // the TextNode has DDE fields

    // records a new attibute in m_pHistory.
    void NoteInHistory( SwTxtAttr *pAttr, const bool bNew = false );

    void CalcFlags( );

    // Delete methods may only be called by the TextNode!
    // Because the TextNode also guarantees removal of the Character for
    // attributes without an end.
    friend class SwTxtNode;
    void DeleteAtPos( const sal_uInt16 nPos );
    // Delete the given Hint. The Hint must actually be in the array!
    void Delete( SwTxtAttr* pTxtHt );

    inline void SetInSplitNode(bool bInSplit) { m_bInSplitNode = bInSplit; }
    inline void SetCalcHiddenParaField() { m_bCalcHiddenParaField = true; }
    inline void SetHiddenParaField( const bool bNew )
        { m_bHasHiddenParaField = bNew; }
    inline bool HasHiddenParaField() const
    {
        if ( m_bCalcHiddenParaField )
        {
            (const_cast<SwpHints*>(this))->CalcHiddenParaField();
        }
        return m_bHasHiddenParaField;
    }

    void InsertNesting(SwTxtAttrNesting & rNewHint);
    bool TryInsertNesting(SwTxtNode & rNode, SwTxtAttrNesting & rNewHint);
    void BuildPortions( SwTxtNode& rNode, SwTxtAttr& rNewHint,
            const SetAttrMode nMode );
    bool MergePortions( SwTxtNode& rNode );

public:
    SwpHints();

    inline bool CanBeDeleted() const    { return !Count(); }

    // register a History, which receives all attribute changes (for Undo)
    void Register( SwRegHistory* pHist ) { m_pHistory = pHist; }
    // deregister the currently registered History
    void DeRegister() { Register(0); }
    SwRegHistory* GetHistory() const    { return m_pHistory; }

    /// try to insert the hint
    /// @return true iff hint successfully inserted
    bool TryInsertHint( SwTxtAttr * const pHint, SwTxtNode & rNode,
            const SetAttrMode nMode = nsSetAttrMode::SETATTR_DEFAULT );

    inline bool HasFtn() const          { return m_bFootnote; }
    inline bool IsInSplitNode() const   { return m_bInSplitNode; }

    // calc current value of m_bHasHiddenParaField, returns true iff changed
    bool CalcHiddenParaField();

    DECL_FIXEDMEMPOOL_NEWDEL(SwpHints)
};

// Ausgabeoperator fuer die Texthints
SvStream &operator<<(SvStream &aS, const SwpHints &rHints); //$ ostream

/*************************************************************************
 *                         Inline Implementations
 *************************************************************************/

inline sal_uInt16 SwpHintsArray::GetStartOf( const SwTxtAttr *pHt ) const
{
    sal_uInt16 nPos;
    if ( !m_HintStarts.Seek_Entry( pHt, &nPos ) )
    {
        nPos = USHRT_MAX;
    }
    return nPos;
}

inline SwTxtAttr *SwpHintsArray::Cut( const sal_uInt16 nPosInStart )
{
    SwTxtAttr *pHt = GetTextHint(nPosInStart);
    DeleteAtPos( nPosInStart );
    return pHt;
}


#endif
