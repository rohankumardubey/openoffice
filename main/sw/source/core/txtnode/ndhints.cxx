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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include "txatbase.hxx"
#include "ndhints.hxx"
#include <txtatr.hxx>

#ifndef PRODUCT
#include <pam.hxx>
#endif


_SV_IMPL_SORTAR_ALG( SwpHtStart, SwTxtAttr* )
_SV_IMPL_SORTAR_ALG( SwpHtEnd, SwTxtAttr* )

#ifdef NIE

void DumpHints( const SwpHtStart &rHtStart,
				const SwpHtEnd &rHtEnd )
{
#ifdef DBG_UTIL
	aDbstream << "DumpHints:" << endl;
	(aDbstream << "\tStarts:" ).WriteNumber(rHtStart.Count()) << endl;
	for( sal_uInt16 i = 0; i < rHtStart.Count(); ++i )
	{
		const SwTxtAttr *pHt = rHtStart[i];
		((((aDbstream << '\t').WriteNumber( i )<< " [").WriteNumber( pHt->Which() )
			<< ']' << '\t').WriteNumber( long( pHt ) )
				  << '\t').WriteNumber( *pHt->GetStart() );
		if( pHt->GetEnd() )
			(aDbstream << " -> " ).WriteNumber( *pHt->GetEnd() );
		aDbstream << endl;
	}
	(aDbstream << "\tEnds:").WriteNumber( rHtEnd.Count() )<< endl;
	for( i = 0; i < rHtEnd.Count(); ++i )
	{
		const SwTxtAttr *pHt = rHtEnd[i];
		(((aDbstream << '\t').WriteNumber( i )<< " [").WriteNumber( pHt->Which() )
			<< ']' << '\t' ).WriteNumber( long( pHt ) );
		if( pHt->GetEnd() )
			(aDbstream << '\t').WriteNumber( *pHt->GetEnd() )<< " <- ";
		aDbstream.WriteNumber( *pHt->GetStart() )<< endl;
	}
	aDbstream << endl;
#endif
}
#else
inline void DumpHints(const SwpHtStart &, const SwpHtEnd &) { }
#endif

/*************************************************************************
 *                        inline IsEqual()
 *************************************************************************/

inline sal_Bool IsEqual( const SwTxtAttr &rHt1, const SwTxtAttr &rHt2 )
{
	return (long)(&rHt1) == (long)(&rHt2);
}

/*************************************************************************
 *                      IsLessStart()
 *************************************************************************/

// SV_IMPL_OP_PTRARR_SORT( SwpHtStart, SwTxtAttr* )
// kein SV_IMPL_PTRARR_SORT( name,ArrElement )
// unser SEEK_PTR_TO_OBJECT_NOTL( name,ArrElement )

// Sortierreihenfolge: Start, Ende (umgekehrt!), Which-Wert (umgekehrt!),
// 					   als letztes die Adresse selbst

static sal_Bool lcl_IsLessStart( const SwTxtAttr &rHt1, const SwTxtAttr &rHt2 )
{
	if ( *rHt1.GetStart() == *rHt2.GetStart() )
	{
        const xub_StrLen nHt1 = *rHt1.GetAnyEnd();
        const xub_StrLen nHt2 = *rHt2.GetAnyEnd();
		if ( nHt1 == nHt2 )
		{
            const sal_uInt16 nWhich1 = rHt1.Which();
            const sal_uInt16 nWhich2 = rHt2.Which();
            if ( nWhich1 == nWhich2 )
            {
                if ( RES_TXTATR_CHARFMT == nWhich1 )
                {
                    const sal_uInt16 nS1 = static_cast<const SwTxtCharFmt&>(rHt1).GetSortNumber();
                    const sal_uInt16 nS2 = static_cast<const SwTxtCharFmt&>(rHt2).GetSortNumber();
                    ASSERT( nS1 != nS2, "AUTOSTYLES: lcl_IsLessStart trouble" )
                    if ( nS1 != nS2 ) // robust
                        return nS1 < nS2;
                }

                return (long)&rHt1 < (long)&rHt2;
            }
            // order is important! for requirements see hintids.hxx
            return ( nWhich1 > nWhich2 );
        }
        return ( nHt1 > nHt2 );
	}
	return ( *rHt1.GetStart() < *rHt2.GetStart() );
}

/*************************************************************************
 *                      inline IsLessEnd()
 *************************************************************************/

// Zuerst nach Ende danach nach Ptr
static sal_Bool lcl_IsLessEnd( const SwTxtAttr &rHt1, const SwTxtAttr &rHt2 )
{
    const xub_StrLen nHt1 = *rHt1.GetAnyEnd();
    const xub_StrLen nHt2 = *rHt2.GetAnyEnd();
	if ( nHt1 == nHt2 )
	{
		if ( *rHt1.GetStart() == *rHt2.GetStart() )
		{
            const sal_uInt16 nWhich1 = rHt1.Which();
            const sal_uInt16 nWhich2 = rHt2.Which();
            if ( nWhich1 == nWhich2 )
            {
                if ( RES_TXTATR_CHARFMT == nWhich1 )
                {
                    const sal_uInt16 nS1 = static_cast<const SwTxtCharFmt&>(rHt1).GetSortNumber();
                    const sal_uInt16 nS2 = static_cast<const SwTxtCharFmt&>(rHt2).GetSortNumber();
                    ASSERT( nS1 != nS2, "AUTOSTYLES: lcl_IsLessEnd trouble" )
                    if ( nS1 != nS2 ) // robust
                        return nS1 > nS2;
                }

                return (long)&rHt1 > (long)&rHt2;
            }
            // order is important! for requirements see hintids.hxx
            return ( nWhich1 < nWhich2 );
        }
		else
			return ( *rHt1.GetStart() > *rHt2.GetStart() );
	}
	return ( nHt1 < nHt2 );
}

/*************************************************************************
 *                      SwpHtStart::Seek_Entry()
 *************************************************************************/

sal_Bool SwpHtStart::Seek_Entry( const SwTxtAttr *pElement, sal_uInt16 *pPos ) const
{
	sal_uInt16 nOben = Count(), nMitte, nUnten = 0;
	if( nOben > 0 )
	{
		nOben--;
		while( nUnten <= nOben )
		{
			nMitte = nUnten + ( nOben - nUnten ) / 2;
			const SwTxtAttr *pMitte = (*this)[nMitte];
			if( IsEqual( *pMitte, *pElement ) )
			{
				*pPos = nMitte;
				return sal_True;
			}
			else
				if( lcl_IsLessStart( *pMitte, *pElement ) )
					nUnten = nMitte + 1;
				else
					if( nMitte == 0 )
					{
						*pPos = nUnten;
						return sal_False;
					}
					else
						nOben = nMitte - 1;
		}
	}
	*pPos = nUnten;
	return sal_False;
}

/*************************************************************************
 *                      SwpHtEnd::Seek_Entry()
 *************************************************************************/

sal_Bool SwpHtEnd::Seek_Entry( const SwTxtAttr *pElement, sal_uInt16 *pPos ) const
{
	sal_uInt16 nOben = Count(), nMitte, nUnten = 0;
	if( nOben > 0 )
	{
		nOben--;
		while( nUnten <= nOben )
		{
			nMitte = nUnten + ( nOben - nUnten ) / 2;
			const SwTxtAttr *pMitte = (*this)[nMitte];
			if( IsEqual( *pMitte, *pElement ) )
			{
				*pPos = nMitte;
				return sal_True;
			}
			else
				if( lcl_IsLessEnd( *pMitte, *pElement ) )
					nUnten = nMitte + 1;
				else
					if( nMitte == 0 )
					{
						*pPos = nUnten;
						return sal_False;
					}
					else
						nOben = nMitte - 1;
		}
	}
	*pPos = nUnten;
	return sal_False;
}

/*************************************************************************
 *                      class SwpHintsArr
 *************************************************************************/

void SwpHintsArray::Insert( const SwTxtAttr *pHt )
{
    Resort();
#ifdef DBG_UTIL
    sal_uInt16 nPos;
    ASSERT(!m_HintStarts.Seek_Entry( pHt, &nPos ),
            "Insert: hint already in HtStart");
    ASSERT(!m_HintEnds.Seek_Entry( pHt, &nPos ),
            "Insert: hint already in HtEnd");
#endif
    m_HintStarts.Insert( pHt );
    m_HintEnds.Insert( pHt );
#ifdef DBG_UTIL
#ifdef NIE
    (aDbstream << "Insert: " ).WriteNumber( long( pHt ) ) << endl;
    DumpHints( m_HintStarts, m_HintEnds );
#endif
#endif
}

void SwpHintsArray::DeleteAtPos( const sal_uInt16 nPos )
{
    // optimization: nPos is the position in the Starts array
    const SwTxtAttr *pHt = m_HintStarts[ nPos ];
    m_HintStarts.Remove( nPos );

    Resort();

    sal_uInt16 nEndPos;
    m_HintEnds.Seek_Entry( pHt, &nEndPos );
    m_HintEnds.Remove( nEndPos );
#ifdef DBG_UTIL
#ifdef NIE
    (aDbstream << "DeleteAtPos: " ).WriteNumber( long( pHt ) ) << endl;
    DumpHints( m_HintStarts, m_HintEnds );
#endif
#endif
}

#ifdef DBG_UTIL

/*************************************************************************
 *                      SwpHintsArray::Check()
 *************************************************************************/


#define CHECK_ERR(cond, text) \
        if(!(cond)) \
        { \
            ASSERT(!this, text); \
            DumpHints(m_HintStarts, m_HintEnds); \
            return !(const_cast<SwpHintsArray*>(this))->Resort(); \
        }

bool SwpHintsArray::Check() const
{
	// 1) gleiche Anzahl in beiden Arrays
    CHECK_ERR( m_HintStarts.Count() == m_HintEnds.Count(),
        "HintsCheck: wrong sizes" );
	xub_StrLen nLastStart = 0;
	xub_StrLen nLastEnd   = 0;

	const SwTxtAttr *pLastStart = 0;
	const SwTxtAttr *pLastEnd = 0;

	for( sal_uInt16 i = 0; i < Count(); ++i )
	{
		// --- Start-Kontrolle ---

		// 2a) gueltiger Pointer? vgl. DELETEFF
        const SwTxtAttr *pHt = m_HintStarts[i];
		CHECK_ERR( 0xFF != *(unsigned char*)pHt, "HintsCheck: start ptr was deleted" );

		// 3a) Stimmt die Start-Sortierung?
		xub_StrLen nIdx = *pHt->GetStart();
		CHECK_ERR( nIdx >= nLastStart, "HintsCheck: starts are unsorted" );

		// 4a) IsLessStart-Konsistenz
		if( pLastStart )
			CHECK_ERR( lcl_IsLessStart( *pLastStart, *pHt ), "HintsCheck: IsLastStart" );

		nLastStart = nIdx;
		pLastStart = pHt;

		// --- End-Kontrolle ---

		// 2b) gueltiger Pointer? vgl. DELETEFF
        const SwTxtAttr *pHtEnd = m_HintEnds[i];
		CHECK_ERR( 0xFF != *(unsigned char*)pHtEnd, "HintsCheck: end ptr was deleted" );

		// 3b) Stimmt die End-Sortierung?
		nIdx = *pHtEnd->GetAnyEnd();
		CHECK_ERR( nIdx >= nLastEnd, "HintsCheck: ends are unsorted" );
		nLastEnd = nIdx;

		// 4b) IsLessEnd-Konsistenz
		if( pLastEnd )
			CHECK_ERR( lcl_IsLessEnd( *pLastEnd, *pHtEnd ), "HintsCheck: IsLastEnd" );

		nLastEnd = nIdx;
		pLastEnd = pHtEnd;

		// --- Ueberkreuzungen ---

		// 5) gleiche Pointer in beiden Arrays
        if( !m_HintStarts.Seek_Entry( pHt, &nIdx ) )
            nIdx = STRING_LEN;

        CHECK_ERR( STRING_LEN != nIdx, "HintsCheck: no GetStartOf" );

		// 6) gleiche Pointer in beiden Arrays
        if( !m_HintEnds.Seek_Entry( pHt, &nIdx ) )
            nIdx = STRING_LEN;

        CHECK_ERR( STRING_LEN != nIdx, "HintsCheck: no GetEndOf" );

        // 7a) character attributes in array?
        sal_uInt16 nWhich = pHt->Which();
        CHECK_ERR( !isCHRATR(nWhich),
                   "HintsCheck: Character attribute in start array" );

        // 7b) character attributes in array?
        nWhich = pHtEnd->Which();
        CHECK_ERR( !isCHRATR(nWhich),
                   "HintsCheck: Character attribute in end array" );

        // 8) style portion check
#if OSL_DEBUG_LEVEL > 1
        const SwTxtAttr* pHtThis = m_HintStarts[i];
        const SwTxtAttr* pHtLast = i > 0 ? m_HintStarts[i-1] : 0;
        CHECK_ERR( 0 == i ||
                    ( RES_TXTATR_CHARFMT != pHtLast->Which() && RES_TXTATR_AUTOFMT != pHtLast->Which() ) ||
                    ( RES_TXTATR_CHARFMT != pHtThis->Which() && RES_TXTATR_AUTOFMT != pHtThis->Which() ) ||
                    ( *pHtThis->GetStart() >= *pHtLast->GetEnd() ) ||
                    (   (   (   (*pHtThis->GetStart() == *pHtLast->GetStart())
                            &&  (*pHtThis->GetEnd()   == *pHtLast->GetEnd())
                            ) // same range
                        ||  (*pHtThis->GetStart() == *pHtThis->GetEnd())
                        )
                    &&  (   (pHtThis->Which() != RES_TXTATR_AUTOFMT)
                        ||  (pHtLast->Which() != RES_TXTATR_AUTOFMT)
                        ) // never two AUTOFMT on same range
                    ),
                   "HintsCheck: Portion inconsistency. "
                   "This can be temporarily ok during undo operations" );

        // 9) nesting portion check
        if (pHtThis->IsNesting())
        {
            for ( sal_uInt16 j = 0; j < Count(); ++j )
            {
                SwTxtAttr const * const pOther( m_HintStarts[j] );
                if ( pOther->IsNesting() &&  (i != j) )
                {
                    SwComparePosition cmp = ComparePosition(
                        *pHtThis->GetStart(), *pHtThis->GetEnd(),
                        *pOther->GetStart(), *pOther->GetEnd());
                    CHECK_ERR( (POS_OVERLAP_BEFORE != cmp) &&
                               (POS_OVERLAP_BEHIND != cmp),
                        "HintsCheck: overlapping nesting hints!!!" );
                }
            }
        }

        // 10) dummy char check (unfortunately cannot check SwTxtNode::m_Text)
        if (pHtThis->HasDummyChar())
        {
            for ( sal_uInt16 j = 0; j < i; ++j )
            {
                SwTxtAttr const * const pOther( m_HintStarts[j] );
                if (pOther->HasDummyChar())
                {
                    CHECK_ERR( (*pOther->GetStart() != *pHtThis->GetStart()),
                        "HintsCheck: multiple hints claim same CH_TXTATR!");
                }
            }
        }
#endif
    }
    return true;
}

#endif      /* PRODUCT */

/*************************************************************************
 *                          SwpHintsArray::Resort()
 *************************************************************************/

// Resort() wird vor jedem Insert und Delete gerufen.
// Wenn Textmasse geloescht wird, so werden die Indizes in
// ndtxt.cxx angepasst. Leider erfolgt noch keine Neusortierung
// auf gleichen Positionen.

bool SwpHintsArray::Resort()
{
    bool bResort = false;
	const SwTxtAttr *pLast = 0;
	sal_uInt16 i;

    for ( i = 0; i < m_HintStarts.Count(); ++i )
    {
        const SwTxtAttr *pHt = m_HintStarts[i];
		if( pLast && !lcl_IsLessStart( *pLast, *pHt ) )
		{
#ifdef NIE
#ifdef DBG_UTIL
//            ASSERT( bResort, "!Resort/Start: correcting hints-array" );
			aDbstream << "Resort: Starts" << endl;
            DumpHints( m_HintStarts, m_HintEnds );
#endif
#endif
            m_HintStarts.Remove( i );
            m_HintStarts.Insert( pHt );
            pHt = m_HintStarts[i];
			if ( pHt != pLast )
				--i;
            bResort = true;
        }
		pLast = pHt;
	}

	pLast = 0;
    for ( i = 0; i < m_HintEnds.Count(); ++i )
    {
        const SwTxtAttr *pHt = m_HintEnds[i];
		if( pLast && !lcl_IsLessEnd( *pLast, *pHt ) )
		{
#ifdef NIE
#ifdef DBG_UTIL
			aDbstream << "Resort: Ends" << endl;
            DumpHints( m_HintStarts, m_HintEnds );
#endif
#endif
            m_HintEnds.Remove( i );
            m_HintEnds.Insert( pHt );
            pHt = m_HintEnds[i]; // normalerweise == pLast
			// Wenn die Unordnung etwas groesser ist (24200),
			// muessen wir Position i erneut vergleichen.
			if ( pLast != pHt )
				--i;
            bResort = true;
        }
		pLast = pHt;
	}
#ifdef DBG_UTIL
#ifdef NIE
	aDbstream << "Resorted:" << endl;
    DumpHints( m_HintStarts, m_HintEnds );
#endif
#endif
	return bResort;
}


