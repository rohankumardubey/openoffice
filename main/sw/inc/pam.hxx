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


#ifndef _PAM_HXX
#define _PAM_HXX

#include <stddef.h>			// fuer MemPool
#include <tools/gen.hxx>
#include <tools/mempool.hxx>
#include <cshtyp.hxx>		// fuer die Funktions-Definitionen
#include <ring.hxx>			// Superklasse
#include <index.hxx>		// fuer SwIndex
#include <ndindex.hxx>		// fuer SwNodeIndex
#include "swdllapi.h"

class SwFmt;
class SfxPoolItem;
class SfxItemSet;
class SwDoc;
class SwNode;
class SwCntntNode;
class SwPaM;

namespace com { namespace sun { namespace star { namespace util {
	struct SearchOptions;
} } } }

namespace utl {
	class TextSearch;
}

struct SW_DLLPUBLIC SwPosition
{
	SwNodeIndex nNode;
	SwIndex nContent;

    SwPosition( const SwNodeIndex &rNode, const SwIndex &rCntnt );
    explicit SwPosition( const SwNodeIndex &rNode );
    explicit SwPosition( const SwNode& rNode );
    explicit SwPosition( SwCntntNode& rNode, const xub_StrLen nOffset = 0 );

	SwPosition( const SwPosition & );
	SwPosition &operator=(const SwPosition &);

    // #111827#
    /**
       Returns the document this position is in.

       @return the document this position is in.
    */
    SwDoc * GetDoc() const;

	sal_Bool operator < (const SwPosition &) const;
	sal_Bool operator >	(const SwPosition &) const;
	sal_Bool operator <=(const SwPosition &) const;
	sal_Bool operator >=(const SwPosition &) const;
	sal_Bool operator ==(const SwPosition &) const;
	sal_Bool operator !=(const SwPosition &) const;
};


// das Ergebnis eines Positions Vergleiches
enum SwComparePosition {
	POS_BEFORE,				// Pos1 liegt vor Pos2
	POS_BEHIND,				// Pos1 liegt hinter Pos2
	POS_INSIDE,				// Pos1 liegt vollstaendig in Pos2
	POS_OUTSIDE,			// Pos2 liegt vollstaendig in Pos1
	POS_EQUAL,				// Pos1 ist genauso gross wie Pos2
	POS_OVERLAP_BEFORE,		// Pos1 ueberlappt Pos2 am Anfang
	POS_OVERLAP_BEHIND,		// Pos1 ueberlappt Pos2 am Ende
	POS_COLLIDE_START,		// Pos1 Start stoesst an Pos2 Ende
	POS_COLLIDE_END			// Pos1 End stoesst an Pos2 Start
};
SwComparePosition ComparePosition(
			const SwPosition& rStt1, const SwPosition& rEnd1,
			const SwPosition& rStt2, const SwPosition& rEnd2 );

SwComparePosition ComparePosition(
			const unsigned long nStt1, const unsigned long nEnd1,
			const unsigned long nStt2, const unsigned long nEnd2 );


// SwPointAndMark / SwPaM
struct SwMoveFnCollection;
typedef SwMoveFnCollection* SwMoveFn;
SW_DLLPUBLIC extern SwMoveFn fnMoveForward; // SwPam::Move()/Find() default argument.
SW_DLLPUBLIC extern SwMoveFn fnMoveBackward;

typedef sal_Bool (*SwGoInDoc)( SwPaM& rPam, SwMoveFn fnMove );
SW_DLLPUBLIC extern SwGoInDoc fnGoDoc;
extern SwGoInDoc fnGoSection;
SW_DLLPUBLIC extern SwGoInDoc fnGoNode;
SW_DLLPUBLIC extern SwGoInDoc fnGoCntnt; // SwPam::Move() default argument.
extern SwGoInDoc fnGoCntntCells;
extern SwGoInDoc fnGoCntntSkipHidden;
extern SwGoInDoc fnGoCntntCellsSkipHidden;

void _InitPam();

class SW_DLLPUBLIC SwPaM : public Ring
{
    SwPosition   m_Bound1;
    SwPosition   m_Bound2;
    SwPosition * m_pPoint; // points at either m_Bound1 or m_Bound2
    SwPosition * m_pMark;  // points at either m_Bound1 or m_Bound2
    bool m_bIsInFrontOfLabel;

	SwPaM* MakeRegion( SwMoveFn fnMove, const SwPaM * pOrigRg = 0 );

public:
	SwPaM( const SwPosition& rPos, SwPaM* pRing = 0 );
	SwPaM( const SwPosition& rMk, const SwPosition& rPt, SwPaM* pRing = 0 );
	SwPaM( const SwNodeIndex& rMk, const SwNodeIndex& rPt,
		   long nMkOffset = 0, long nPtOffset = 0, SwPaM* pRing = 0 );
	SwPaM( const SwNode& rMk, const SwNode& rPt,
		   long nMkOffset = 0, long nPtOffset = 0, SwPaM* pRing = 0 );
	SwPaM(	const SwNodeIndex& rMk, xub_StrLen nMkCntnt,
			const SwNodeIndex& rPt, xub_StrLen nPtCntnt, SwPaM* pRing = 0 );
	SwPaM(	const SwNode& rMk, xub_StrLen nMkCntnt,
			const SwNode& rPt, xub_StrLen nPtCntnt, SwPaM* pRing = 0 );
	SwPaM( const SwNode& rNd, xub_StrLen nCntnt = 0, SwPaM* pRing = 0 );
	SwPaM( const SwNodeIndex& rNd, xub_StrLen nCntnt = 0, SwPaM* pRing = 0 );
	virtual ~SwPaM();

	// @@@ semantic: no copy ctor.
	SwPaM( SwPaM & );
	// @@@ semantic: no copy assignment for super class Ring.
	SwPaM& operator=( const SwPaM & );

	// Bewegen des Cursors
	sal_Bool Move( SwMoveFn fnMove = fnMoveForward,
					SwGoInDoc fnGo = fnGoCntnt );

	// Suchen
	sal_uInt8 Find(	const com::sun::star::util::SearchOptions& rSearchOpt,
				sal_Bool bSearchInNotes,
				utl::TextSearch& rSTxt,
				SwMoveFn fnMove = fnMoveForward,
				const SwPaM *pPam =0, sal_Bool bInReadOnly = sal_False);
	sal_Bool Find(	const SwFmt& rFmt,
				SwMoveFn fnMove = fnMoveForward,
				const SwPaM *pPam =0, sal_Bool bInReadOnly = sal_False);
	sal_Bool Find(	const SfxPoolItem& rAttr, sal_Bool bValue = sal_True,
				SwMoveFn fnMove = fnMoveForward,
				const SwPaM *pPam =0, sal_Bool bInReadOnly = sal_False );
	sal_Bool Find(	const SfxItemSet& rAttr, sal_Bool bNoColls,
				SwMoveFn fnMove,
				const SwPaM *pPam, sal_Bool bInReadOnly, sal_Bool bMoveFirst );

	bool DoSearch( const com::sun::star::util::SearchOptions& rSearchOpt, utl::TextSearch& rSTxt,
					SwMoveFn fnMove, sal_Bool bSrchForward, sal_Bool bRegSearch, sal_Bool bChkEmptyPara, sal_Bool bChkParaEnd,
					xub_StrLen &nStart, xub_StrLen &nEnde,xub_StrLen nTxtLen,SwNode* pNode, SwPaM* pPam);

    inline bool IsInFrontOfLabel() const        { return m_bIsInFrontOfLabel; }
    inline void _SetInFrontOfLabel( bool bNew ) { m_bIsInFrontOfLabel = bNew; }

    virtual void SetMark();

    void DeleteMark()
    {
        if (m_pMark != m_pPoint)
        {
            // clear the mark position; this helps if mark's SwIndex is
            // registered at some node, and that node is then deleted
            *m_pMark = SwPosition( SwNodeIndex( GetNode()->GetNodes() ) );
            m_pMark = m_pPoint;
        }
    }
#ifndef DBG_UTIL

    void Exchange()
    {
        if (m_pPoint != m_pMark)
        {
            SwPosition *pTmp = m_pPoint;
            m_pPoint = m_pMark;
            m_pMark = pTmp;
        }
    }
#else
    void Exchange();
#endif

    /** A PaM marks a selection if Point and Mark are distinct positions.
        @return     true iff the PaM spans a selection
     */
    bool HasMark() const { return m_pPoint == m_pMark ? false : true; }

    const SwPosition *GetPoint() const { return m_pPoint; }
          SwPosition *GetPoint()       { return m_pPoint; }
    const SwPosition *GetMark()  const { return m_pMark; }
          SwPosition *GetMark()        { return m_pMark; }

    const SwPosition *Start() const
                { return (*m_pPoint) <= (*m_pMark) ? m_pPoint : m_pMark; }
          SwPosition *Start()
                { return (*m_pPoint) <= (*m_pMark) ? m_pPoint : m_pMark; }

    const SwPosition *End()   const
                { return (*m_pPoint) >  (*m_pMark) ? m_pPoint : m_pMark; }
          SwPosition *End()
                { return (*m_pPoint) >  (*m_pMark) ? m_pPoint : m_pMark; }

    /// @return current Node at Point/Mark
    SwNode    * GetNode      ( bool bPoint = true ) const
    {
        return &( bPoint ? m_pPoint->nNode : m_pMark->nNode ).GetNode();
    }

    /// @return current ContentNode at Point/Mark
    SwCntntNode* GetCntntNode( bool bPoint = true ) const
    {
        return GetNode(bPoint)->GetCntntNode();
    }

    /**
       Normalizes PaM, i.e. sort point and mark.

       @param bPointFirst sal_True: If the point is behind the mark then swap.
                          sal_False: If the mark is behind the point then swap.
    */
    SwPaM & Normalize(sal_Bool bPointFirst = sal_True);

    /// @return the document (SwDoc) at which the PaM is registered
    SwDoc* GetDoc() const   { return m_pPoint->nNode.GetNode().GetDoc(); }

          SwPosition& GetBound( bool bOne = true )
                            { return bOne ? m_Bound1 : m_Bound2; }
    const SwPosition& GetBound( bool bOne = true ) const
                            { return bOne ? m_Bound1 : m_Bound2; }

	// erfrage die Seitennummer auf der der Cursor steht
	sal_uInt16 GetPageNum( sal_Bool bAtPoint = sal_True, const Point* pLayPos = 0 );

	// steht in etwas geschuetztem oder in die Selektion umspannt
	// etwas geschuetztes.
    sal_Bool HasReadonlySel( bool bFormView ) const;

    sal_Bool ContainsPosition(const SwPosition & rPos)
    { return *Start() <= rPos && rPos <= *End(); }

    static sal_Bool Overlap(const SwPaM & a, const SwPaM & b);
    
    static sal_Bool LessThan(const SwPaM & a, const SwPaM & b);

	DECL_FIXEDMEMPOOL_NEWDEL(SwPaM);

    String GetTxt() const;
    void InvalidatePaM();
};


sal_Bool CheckNodesRange( const SwNodeIndex&, const SwNodeIndex&, sal_Bool );
sal_Bool GoInCntnt( SwPaM & rPam, SwMoveFn fnMove );


#endif	// _PAM_HXX
