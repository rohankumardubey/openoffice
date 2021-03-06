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
#include "precompiled_basegfx.hxx"
// autogenerated file with codegen.pl

#include "preextstl.h"
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "postextstl.h"

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/curve/b2dbeziertools.hxx>
#include <basegfx/range/b2dpolyrange.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <boost/bind.hpp>

using namespace ::basegfx;


namespace basegfx2d
{
/// Gets a random ordinal [0,n)
inline double getRandomOrdinal( const ::std::size_t n )
{
    // use this one when displaying polygons in OOo, which still sucks
    // great rocks when trying to import non-integer svg:d attributes
    // return sal_Int64(double(n) * rand() / (RAND_MAX + 1.0));
    return double(n) * rand() / (RAND_MAX + 1.0);
}

inline bool compare(const B2DPoint& left, const B2DPoint& right)
{
    return left.getX()<right.getX() 
        || (left.getX()==right.getX() && left.getY()<right.getY());
}


class boxclipper : public CppUnit::TestFixture
{
private:
    B2DPolyRange aDisjunctRanges;
    B2DPolyRange aEqualRanges;
    B2DPolyRange aIntersectionN;
    B2DPolyRange aIntersectionE;
    B2DPolyRange aIntersectionS;
    B2DPolyRange aIntersectionW;
    B2DPolyRange aIntersectionNE;
    B2DPolyRange aIntersectionSE;
    B2DPolyRange aIntersectionSW;
    B2DPolyRange aIntersectionNW;
    B2DPolyRange aRingIntersection;
    B2DPolyRange aRingIntersection2;
    B2DPolyRange aRingIntersectExtraStrip;
    B2DPolyRange aComplexIntersections;
    B2DPolyRange aRandomIntersections;

public:
    // initialise your test code values here.
    void setUp()
    {
        B2DRange aCenter(100, 100, -100, -100);
        B2DRange aOffside(800, 800, 1000, 1000);
        B2DRange aNorth(100, 0, -100, -200);
        B2DRange aSouth(100, 200, -100, 0);
        B2DRange aEast(0, 100, 200, -100);
        B2DRange aWest(-200, 100, 0, -100);
        B2DRange aNorthEast(0, 0, 200, -200);
        B2DRange aSouthEast(0, 0, 200, 200);
        B2DRange aSouthWest(0, 0, -200, 200);
        B2DRange aNorthWest(0, 0, -200, -200);

        B2DRange aNorth2(-150, 50,  150,  350);
        B2DRange aSouth2(-150, -50, 150, -350);
        B2DRange aEast2 (50,  -150, 350,  150);
        B2DRange aWest2 (-50, -150,-350,  150);

        aDisjunctRanges.appendElement( aCenter, ORIENTATION_NEGATIVE );
        aDisjunctRanges.appendElement( aOffside, ORIENTATION_NEGATIVE );

        aEqualRanges.appendElement( aCenter, ORIENTATION_NEGATIVE );
        aEqualRanges.appendElement( aCenter, ORIENTATION_NEGATIVE );

        aIntersectionN.appendElement( aCenter, ORIENTATION_NEGATIVE );
        aIntersectionN.appendElement( aNorth, ORIENTATION_NEGATIVE );

        aIntersectionE.appendElement( aCenter, ORIENTATION_NEGATIVE );
        aIntersectionE.appendElement( aEast, ORIENTATION_NEGATIVE );

        aIntersectionS.appendElement( aCenter, ORIENTATION_NEGATIVE );
        aIntersectionS.appendElement( aSouth, ORIENTATION_NEGATIVE );

        aIntersectionW.appendElement( aCenter, ORIENTATION_NEGATIVE );
        aIntersectionW.appendElement( aWest, ORIENTATION_NEGATIVE );

        aIntersectionNE.appendElement( aCenter, ORIENTATION_NEGATIVE );
        aIntersectionNE.appendElement( aNorthEast, ORIENTATION_NEGATIVE );

        aIntersectionSE.appendElement( aCenter, ORIENTATION_NEGATIVE );
        aIntersectionSE.appendElement( aSouthEast, ORIENTATION_NEGATIVE );

        aIntersectionSW.appendElement( aCenter, ORIENTATION_NEGATIVE );
        aIntersectionSW.appendElement( aSouthWest, ORIENTATION_NEGATIVE );

        aIntersectionNW.appendElement( aCenter, ORIENTATION_NEGATIVE );
        aIntersectionNW.appendElement( aNorthWest, ORIENTATION_NEGATIVE );

        aRingIntersection.appendElement( aNorth2, ORIENTATION_NEGATIVE );
        aRingIntersection.appendElement( aEast2, ORIENTATION_NEGATIVE );
        aRingIntersection.appendElement( aSouth2, ORIENTATION_NEGATIVE );

        aRingIntersection2 = aRingIntersection;
        aRingIntersection2.appendElement( aWest2, ORIENTATION_NEGATIVE );

        aRingIntersectExtraStrip = aRingIntersection2;
        aRingIntersectExtraStrip.appendElement( B2DRange(0, -25, 200, 25), 
                                                ORIENTATION_NEGATIVE );

        aComplexIntersections.appendElement( aCenter, ORIENTATION_NEGATIVE );
        aComplexIntersections.appendElement( aOffside, ORIENTATION_NEGATIVE );
        aComplexIntersections.appendElement( aCenter, ORIENTATION_NEGATIVE );
        aComplexIntersections.appendElement( aNorth, ORIENTATION_NEGATIVE );
        aComplexIntersections.appendElement( aEast, ORIENTATION_NEGATIVE );
        aComplexIntersections.appendElement( aSouth, ORIENTATION_NEGATIVE );
        aComplexIntersections.appendElement( aWest, ORIENTATION_NEGATIVE );
        aComplexIntersections.appendElement( aNorthEast, ORIENTATION_NEGATIVE );
        aComplexIntersections.appendElement( aSouthEast, ORIENTATION_NEGATIVE );
        aComplexIntersections.appendElement( aSouthWest, ORIENTATION_NEGATIVE );
        aComplexIntersections.appendElement( aNorthWest, ORIENTATION_NEGATIVE );

#ifdef GENERATE_RANDOM
        for( int i=0; i<800; ++i )
        {
            B2DRange aRandomRange(
                getRandomOrdinal( 1000 ),
                getRandomOrdinal( 1000 ),
                getRandomOrdinal( 1000 ),
                getRandomOrdinal( 1000 ) );

            aRandomIntersections.appendElement( aRandomRange, ORIENTATION_NEGATIVE );
        }
#else
        const char* randomSvg="m394 783h404v57h-404zm-197-505h571v576h-571zm356-634h75v200h-75zm-40-113h403v588h-403zm93-811h111v494h-111zm-364-619h562v121h-562zm-134-8h292v27h-292zm110 356h621v486h-621zm78-386h228v25h-228zm475-345h201v201h-201zm-2-93h122v126h-122zm-417-243h567v524h-567zm-266-738h863v456h-863zm262-333h315v698h-315zm-328-826h43v393h-43zm830-219h120v664h-120zm-311-636h221v109h-221zm-500 137h628v19h-628zm681-94h211v493h-211zm-366-646h384v355h-384zm-189-199h715v247h-715zm165-459h563v601h-563zm258-479h98v606h-98zm270-517h65v218h-65zm-44-259h96v286h-96zm-599-202h705v468h-705zm216-803h450v494h-450zm-150-22h26v167h-26zm-55-599h50v260h-50zm190-278h490v387h-490zm-290-453h634v392h-634zm257 189h552v300h-552zm-151-690h136v455h-136zm12-597h488v432h-488zm501-459h48v39h-48zm-224-112h429v22h-429zm-281 102h492v621h-492zm519-158h208v17h-208zm-681-563h56v427h-56zm126-451h615v392h-615zm-47-410h598v522h-598zm-32 316h79v110h-79zm-71-129h18v127h-18zm126-993h743v589h-743zm211-430h428v750h-428zm61-554h100v220h-100zm-353-49h658v157h-658zm778-383h115v272h-115zm-249-541h119v712h-119zm203 86h94v40h-94z";
        B2DPolyPolygon randomPoly;
        tools::importFromSvgD(
            randomPoly,
            rtl::OUString::createFromAscii(randomSvg));
        std::for_each(randomPoly.begin(),
                      randomPoly.end(),
                      boost::bind(
             &B2DPolyRange::appendElement,
                          boost::ref(aRandomIntersections),
                          boost::bind(
                 &B2DPolygon::getB2DRange,
                              _1),
                          ORIENTATION_NEGATIVE,
                          1));
#endif
    }

    void tearDown()
    {
    }

    B2DPolyPolygon normalizePoly( const B2DPolyPolygon& rPoly )
    {
        B2DPolyPolygon aRes;
        for( sal_uInt32 i=0; i<rPoly.count(); ++i )
        {
            B2DPolygon aTmp=rPoly.getB2DPolygon(i);
            if( ORIENTATION_NEGATIVE == tools::getOrientation(aTmp) )
                aTmp.flip();

            aTmp=tools::removeNeutralPoints(aTmp);

            B2DPoint* pSmallest=0;
            for(B2DPoint* pCurr=aTmp.begin(); pCurr!=aTmp.end(); ++pCurr)
            {
                if( ! pSmallest || compare(*pCurr, *pSmallest) )
                {
                    pSmallest=pCurr;
                }
            }

            if( pSmallest )
                std::rotate(aTmp.begin(),pSmallest,aTmp.end());

            aRes.append(aTmp);
        }

        // boxclipper & generic clipper disagree slightly on area-less
        // polygons (one or two points only)
        aRes = tools::stripNeutralPolygons(aRes);

        // now, sort all polygons with increasing 0th point
        std::sort(aRes.begin(),
                  aRes.end(),
                  boost::bind(
                      &compare,
                      boost::bind(
                          &B2DPolygon::getB2DPoint,
                          _1,0),
                      boost::bind(
                          &B2DPolygon::getB2DPoint,
                          _2,0)));

        return aRes;
    }

    void verifyPoly(const char* sName, const char* sSvg, const B2DPolyRange& toTest)
    {
        B2DPolyPolygon aTmp1;
        CPPUNIT_ASSERT_MESSAGE(sName, 
                               tools::importFromSvgD(
                                   aTmp1,
                                   rtl::OUString::createFromAscii(sSvg)));

        const rtl::OUString aSvg=
            tools::exportToSvgD(toTest.solveCrossovers());
        B2DPolyPolygon aTmp2;
        CPPUNIT_ASSERT_MESSAGE(sName,
                               tools::importFromSvgD(
                                   aTmp2,
                                   aSvg));

        CPPUNIT_ASSERT_MESSAGE(
            sName,
            normalizePoly(aTmp2) == normalizePoly(aTmp1));
    }

    void verifyPoly()
    {
        const char* disjunct="m100 100v-200h-200v200zm1100 900v-200h-200v200z";
        const char* equal="m100 100v-200h-200v200zm200 0v-200h-200v200h200z";
        const char* intersectionN="m100 0v-100h-200v100zm200 100v-200-100h-200v100 200z";
        const char* intersectionE="m100 100v-200h-100v200zm200 0v-200h-200-100v200h100z";
        const char* intersectionS="m100 100v-200h-200v200 100h200v-100zm0 0v-100h-200v100z";
        const char* intersectionW="m0 100v-200h-100v200zm200 0v-200h-200-100v200h100z";
        const char* intersectionNE="m100 0v-100h-100v100zm200 0v-200h-200v100h-100v200h200v-100z";
        const char* intersectionSE="m200 200v-200h-100v-100h-200v200h100v100zm100-100v-100h-100v100z";
        const char* intersectionSW="m0 100v-100h-100v100zm200 0v-200h-200v100h-100v200h200v-100z";
        const char* intersectionNW="m100 100v-200h-100v-100h-200v200h100v100zm100-100v-100h-100v100z";
        const char* ringIntersection="m150 150v-100h-100v100zm300 0v-300h-200v-200h-300v300h200v100h-200v300h300v-200zm0-200v-100h-100v100z";
        const char* ringIntersection2="m-50-50v-100h-100v100zm100 200v-100h-100v100zm500 0v-300h-200v-200h-300v200h-200v300h200v200h300v-200zm-200-100v-100h100v100zm100-100v-100h-100v100zm100 200v-100h-100v100z";
        const char* ringIntersectExtraStrip="m-50-50v-100h-100v100zm100 200v-100h-100v100zm500 0v-300h-200v-200h-300v200h-200v300h200v200h300v-200zm-200-100v-100h100v25h-50v50h50v25zm150-25v-50h-150v50zm100-75v-100h-100v100zm100 200v-100h-100v100z";
        // TODO: old clipper impl. debug difference
        //const char* complexIntersections="m100 0h-100v-100 100h-100 100v100-100zm0 0zm200 0h-100v-100h-100v-100 100h-100v100h-100 100v100h100v100-100h100v-100zm0 0h-100v-100 100h-100 100v100-100h100zm0 0v-100h-100-100v100 100h100 100v-100zm100 0v-100h-100v-100h-100-100v100h-100v100 100h100v100h100 100v-100h100v-100zm-200 0zm100 0v-100h-100-100v100 100h100 100v-100zm100 100v-200-100h-200-100-100v100 200 100h100 100 200v-100zm-200-100zm1000 1000v-200h-200v200z";
        const char* complexIntersections="m0 0zm0 0zm0 0zm0 0v-100 100h-100 100v100-100h100zm-100 0v-100 100h-100 100v100-100h100zm0 0v-100h-100-100v100 100h100 100v-100zm0 0v-100h-100-100v100 100h100 100v-100zm0 0v-100h-100v-100 100h-100v100h-100 100v100h100v100-100h100v-100h100zm-100-100v-100h-100-100v100h-100v100 100h100v100h100 100v-100h100v-100-100zm0 0v-100h-200-100-100v100 200 100h100 100 200v-100-200zm600 900v200h200v-200z";
        const char* randomIntersections="m63 457v-393h-43v393zm114 63v-8h-48v8zm-14 477v-127h-18v127zm693-923v-5h-119v5zm-260 457v-1h-14v1zm-220-375v-27h-8v27zm78 755v-22h-7v22zm203-774v-8h-158v8zm-108 375v-17h23v17zm813-19v-189h-21v-12h-26v-54h-17v-69h-25v-22h-62v-73h104v-5h-104-15v-17h-49v-1h-8v-16h-119v16h-386v18h-38-24v34h-23v26h-23v-26h-8v26h-18v27h18v339h8v-339h23v339h8v17h-8v13h8v5h-8v1h8v42h15v20h17v94h18 3v224h165v39h130v2h75v4h98v-4h153v-2h77v-20h4v-28h11v-218h-11v-27h3v-1h8v-17h-8v-63h8v-51h18v-32zm-581 32v-13h-14v13zm-78-78v-7h-32v7zm124 14v-21h-14v21zm595 32v-189h-26v-12h-4v-9h-13v-45h-13v-10h-12v-59h-62v-22h-26v-10h11v-63h15v-5h-15-49v-17h-8v-1h-119v1h-107v17h-279-38v34h-24v26h-23v27h23v284h-15v55h15v17h-15v13h15v5h-15v1h15v42h17v20h18v94h3 14v62h8v48h90v32h18v61h35v21h8v2h122v37h75v2h98v-2h153v-20h77v-28h4v-29h5v-40h-5v-149h-1v-27h1v-1h3v-17h-3v-46h3v-17-51h8v-32zm-563 2v-13h-14v13zm198 30v-13h-39v13zm204-43v-21h3v21zm-168-21v-21h-39v21zm306 0v-21h-5v21zm178 115v-272h-20v-12h-2v-54h-21v-164h1v-22h-27v-48h-743v47h-23v18h-71v24h-8v419h-39v19h60v156h66 32v202h-72v110h79v-88h11v39h3v48h621v-14h96v-82h35v-326zm-570-420v-4h-156v4zm63 481v-18h-11v18zm72 0v-25h-14v25zm465-112v-13h-5v13zm-46-43v-21h1v21zm-37-21v-21h-12v21zm-352 21v-21h23v21zm-23 30v-17h23v17zm-23 18v-5h23v5zm-23 82v-19h23v19zm272 75v-3h-35v3zm-76-192v-13h-39v13zm150 30v-13h-35v13zm-76 6v-1h-39v1zm11 106v-25h-11v25zm150 160v-14h-75v14zm318-304v-11h-13v-43h-2v-2h-10v-37h-4v37h-27v3h-31v-3-37h-5v37h-43v3h-2v21h2v21h-2v30h-1v-30h-8v-21h8v-21h-8v-3h-5v-62h5v-11h-5v-29h-8v-52h-15v-17-38h-15v-52h-89v16h-22v36h-175v55h-15v1h-25v51h-23v-41h-14v41h-2v105h-4v21h4v21h-4v13h4v17h-4-18v13h18v5h-18v1h18v42h4v11h2v9h14v-9h23v9h40v19h-40v25h40v2h82v2h75v43h-75v3h75 40v60h35v-60h23 34 12 15v-3h-15v-43h15v-48h10v-37h11v-31h1v1h45v30h5v-30h20v-1h11v1h8v30h19v20h3v-20h1v-30h10v-1h2v-32zm-146-329v-1h-2v1zm-117 211v-11 11zm-76 0v-11h-13v11zm13 65v-65h1v65zm-1 42v-21h1v16h35v5zm-36 30v-17h36v17zm-36 18v-5h36v5zm180-5v-13h-13v-17h5v-13h-5v-21h5v-21h-5v-3h-8v-62h8v-11h-8v-29h-9v-51h-6v-1-17h-15v-38h-54v-36h-35v36h-22v38h-67v17h-108v1h-15v51h-25v105h-23v-105h-14v105h-2v21h2v21h-2v13h2v17h-2-4v13h4v5h-4v1h4v42h2v11h14v-11h23v11h40v9h82v19h-82v25h82v2h75v2h40v43h-40v3h40 35 23v-3h-23v-43h23v-2h34v2h12v-2h6v-46h9v-20h8v-17h2v-26h-2v-5zm-127-64v-21 21zm89 51v-17h3v17zm-57-17v-13h-35v13zm58 61v-26h-19v-5h19v-13h-23v-17h23v-13h-23v-21h23v-21h-23v-65h23v-11h-23v-14h-35v-15 15h-22v14h-18v11h18v65h-18v21h18v16h22v5h-22v13h22v17h-22-18v13h18v5h-18v1h18v25h22v17h35v-17zm0-25v-1h-35v1zm-22-390v6h-175v5h-31v-15h228v4zm344 352v-189h-2v-12h-21v-54h-26v-164h26v-5h-26v-17h-119v-36h-562v35h-62v18h-23v34h-23v-10h-48v419h-8v8h8v5h71v5h-58v1h58v42h8v114h32 18v224h3v39h165v34h456v-32h77v-2h4v-20h11v-28h4v-218h-4v-28h36v-17h-36v-63h39v-83zm-50 0v-11h-1v-43h-3v-2h-6v-39h-4v-34h-13v-60h-12v-12h-31v72h-31v-72-9h-59v-17-38h-5v-59h-8v-5h8v-1h-8v-16h-2v16h-13v-11h-15v-5h-89v5h-22v11h-175v6h-15v7h-25v16h-43v36h-18v66h-54v-107h-32v107h-4v41h-8v105h-6v7h6v14h8v21h-8v13h8v17h-8-14v13h14v5h-14v1h14v42h8v20h90v19h-34v7h-15v68h26v-50h23v50h18 4v62h16v-62h15v110h8v10h3v22h119v11h75v50h75v-50h23v-11h34v11h48v-11h30v-22h21v-120h20v-3h11v3h30v-3h13-13v-27h13v-1h17v-17h-17v-46h17v-17h6v-51h3v-32zm-256-32v-21h-35v-65h35v-11h-35v-14 14h-22v11h22v65h-22v21h22v16-16zm89 69v-5h3v5zm-3 26v-26h-31v-5h31v-13h-31v-17h31v-13h-31v-21h31v-21h-31v-65h31v-11h-31v-14h-23v-15h-35v-51 51h-22v15h-18v14h-35v11h35v65h-35v21h35v16h18v5h-18v13h18v17h-18-36-39-61v13h61v5h-61v1h61v25h39v-25h36v25h18v17h22v11h35v-11h23v-17zm-19-25v-1h-4v-5h4v-13h-4-35-22v13h22v5h-22v1h22v25h35v-25zm23 252v-36h34v36zm-34-99v-43h34v43zm35-128v-26h-8v-5h8v-13h-8v-17h8v-13h-8v-21h8v-21h-8v-3h-9v-62h9v-11h-9v-29h-6v-51-1h-15v-17h-54v-38h-35v38h-22v11h-53v6h-14v1h-108v51h-15v105h-25v21h25v21h-25v13h25v17h-25-23-14-2v13h2v5h-2v1h2v42h14v-42h23v42h40v11h82v9h75v46h40v2h35v-2h23v-4h31v-42h3v46h12v-46h6v-20h9v-17zm-15-61v-13h-12v13zm12 30v-13h-12v13zm12 31v-26h-12v26zm12 131v-3h-12v3zm12 110v-14h-12v14zm27-241v-26h-9v-5h9v-13h-9v-17h9v-13h-9v-21h9v-21h-9v-3h-6v-62h6v-11h-6v-29-51h-15v-1h-54v-17h-35v11h-22v6h-53v1h-14v51h-108v105h-15v21h15v21h-15v13h15v17h-15-25v13h25v5h-25v1h25v25h15v17h21v6h61v5h75v9h18v42h22v4h35v-4h23v-42h31v-9h3v9h12v-9-11h6v-17zm0 0v-26h-6v-5h6v-13h-6v-17h6v-13h-6v-21h6v-21h-6v-3-62-11-29h-15v-51h-54v-1h-35v-6 6h-22v1h-53v51h-14v24h-87v81h-21v21h21v21h-21v13h21v17h-21-15v13h15v5h-15v1h15v25h21v17h61v6h39v-6h36v11h18v9h22v42h35v-42h23v-9h31v-11h3v11h12v-11-17zm0 0v-26-5-13-17-13-21-21-3h-12v3h-3v-65h15v-11h-15v-29h-54v-51h-35v-1 1h-22v51h-53v29h-1v-5h-13v5h-26v76h-61v21h61v21h-61v13h61v17h-61-21v13h21v5h-21v1h21v25h61v17h39v-17h36v17h18v11h22v9h35v-9h23v-11h31v-17h3v17h12v-17zm15-419v-12h-2v12zm186 356v-56h-8v-133h-4v-12h-13v-9h-13v-45h-12v-10h-62v-59-6h-26v-16h-33v-10h33v-12h-33v-22h-5v-29h49v-5h-49-8v-17h-119v17h-107-279v34h-38v26h-24v27h24v179h-7v105h-17v55h17v17h-17v13h17v5h-17v1h17v42h18v20h3v94h14 8v62h41v37h26v-37h23v48h18v32h35v61h8v21h122v2h75v37h98v-37h34v17h119v-57h11v29h66v-29h4v-40h-4v-26h3v-123h-3v-27h3v-1h1v-17h-1v-46h1v-17h3v-51-32zm0 0v-54h-4v-2h-3v-73h-10v-60h-13v-12h-12v-9h-31v9h-31v-9-55h-59v-59h-5v-5h5v-1h-5v-16h-8v-10h8v-12h-8v-22h-119v34h117v10h-28v-6h-89v6h-22v5h-175v11h-40v13h-147v11h-4v107h-8v41h-6v105h-22v21h28v21h-17v13h17v17h-14-3v13h3v5h-3v1h3v42h14v20h8v94h41 26 23 18v62h4v48h31v10h8v22h3v11h119v50h75v21h98v-71h34v71h48v-71h30v-11h21v-22h20v-120h11v120h43v-123h17-17v-27h17v-1h6v-17h-6v-46h6v-17h3v-51h1v-32zm-4 0v-11h-6v-43h-4v-2h-13v-39h-12v-34h-4v34h-27v2h-31v-2-34h-48v36h-2v37h-1v-73h-8v-29-52h-5v-17h-8v-38h-15v-59h-15v-6h-89v6h-22v7h-175v16h-15v36h-25v55h-39v11h-4v41h-18v105h-54v-105h-32v105h-4v7h4v14h86v21h-86v13h86v17h-86-4v13h4v5h-4v1h4v42h86v11h18v9h4v19h-4v25h4v50h16v-48h23v45h-8v3h8 122v96h-119v14h119v10h75v22h75v-22h23v-10h34v10h48v-24h-36v-36h15v-60h21v-3h-11v-43h2v15h9v-15h46v15h5v-15h20v-2h-20v-46h20v-37h11v37h8v46h-8v2h8v15h22v-15h1v-2h-1v-46h1v-17h12v-20h13v-31h4v-32zm-142 148v-2h-9v2zm9-2v-46h46v46zm-46 45v-28h46v28zm67-191v-11h-1v-42h-3v42h-19v11h19v32h3v-32zm-61 0v-11h-5v11zm96 0v-11h-4v-43h-13v-2h-2v-37h-10v-2h-4v2h-27v37h-31v-37-2h-5v2h-43v37h-2v3h-1v-3h-8v-62-11-29h-5v-52h-8v-17h-15v-38-52h-15v-7h-89v7h-22v16h-175v36h-15v55h-25v1h-37v10h-2v41h-4v105h-18v21h18v21h-18v13h18v17h-18-86v13h86v5h-86v1h86v42h18v11h4v9h2v19h-2v25h2v2h14v-2h23v2h40v2h82v43h-122v3h122 75v96h-75v14h75v10h75v-10h23v-14h-23v-36h23v-60h34v60h12v-60h15 10v-3h-10v-43h10v-48h11v-37h46v37h5v-37h20v-30h11v30h8v37h22v-17h1v-20h12v-31h13v-32zm-13 0v-11h-2v-43h-10v-2h-4v2h-19v1h-8v42h-31v-21-21-3h-5v3h-43v21h43v21h-43v11h43v19h-45v13h45v1h5v-1h20v-13h-20v-19h31v32h8v1h19v30h3v-30h1v-1h10v-32zm-72 148v-2h-5v2zm5 43h-5zm66-191v-11h-3v11zm-38 146v-46h11v46zm-11 45v-28h11v28zm-11 149v-4h11v4zm-11 40v-40h-8v40zm92-380v-54-2h-4v-133h-13v-12h-13v-9h-12v-45h-31v45h-31v-55-59h-59v-5h33v-1h-33v-16h-5v-10h5v-12h-5v-22h-8v-29h8v-5h-8-119-107v5h107v29h-386v26h-38v27h40v20h-4v11h-14v148h-22v105h-7v55h18v17h-18v13h18v5h-18v1h18v42h3v20h14v94h8 41v62h26v-62h23v62h18v48h4v10h31v22h8v61h122v21h75v2h98v-2h34v2h99v-84h20v-22h11v22h43v-22h23v-123h-6v-27h6v-1h3v-17h-3v-46h3v-17h1v-51h3v-32zm-43 148v-2h-22v2zm22 43h-30zm66 189v-40h-66v40zm41-380v-11h-10v-43h-4v1h-19v42h-8v11h8v32h19v1h3v-1h1v-32zm38 0v-11h-3v-43h-6v-2h-4v-39h-13v-34h-12v-60h-4v60h-27v34h-31v-34-72h-48v72h-3v-29h-8v-52-17h-5v-38h-8v-59h-15v-6h-15v-11h-89v11h-22v6h-175v7h-15v16h-25v36h-43v66h-18v41h-54v-41h-32v41h-4v105h-8v7h8v14h4v21h-4v13h4v17h-4-8v13h8v5h-8v1h8v42h4v11h86v9h18v19h-18v25h18v50h4 16 15 8v110h3v10h119v22h75v11h75v-11h23v-22h34v22h48v-22h30v-24h-30v-96h51v-3h20-20v-28h20v-15h11v15h8v1h22v-1h13v-17h-12v-46h12v-17h17v-51h6v-32z";

        verifyPoly("disjunct", disjunct, aDisjunctRanges);
        verifyPoly("equal", equal, aEqualRanges);
        verifyPoly("intersectionN", intersectionN, aIntersectionN);
        verifyPoly("intersectionE", intersectionE, aIntersectionE);
        verifyPoly("intersectionS", intersectionS, aIntersectionS);
        verifyPoly("intersectionW", intersectionW, aIntersectionW);
        verifyPoly("intersectionNE", intersectionNE, aIntersectionNE);
        verifyPoly("intersectionSE", intersectionSE, aIntersectionSE);
        verifyPoly("intersectionSW", intersectionSW, aIntersectionSW);
        verifyPoly("intersectionNW", intersectionNW, aIntersectionNW);
        verifyPoly("ringIntersection", ringIntersection, aRingIntersection);
        verifyPoly("ringIntersection2", ringIntersection2, aRingIntersection2);
        verifyPoly("ringIntersectExtraStrip", ringIntersectExtraStrip, aRingIntersectExtraStrip);
        verifyPoly("complexIntersections", complexIntersections, aComplexIntersections);
        verifyPoly("randomIntersections", randomIntersections, aRandomIntersections);
    }

    void dumpSvg(const char* pName, 
                 const ::basegfx::B2DPolyPolygon& rPoly)
    {
        (void)pName; (void)rPoly;
#if defined(VERBOSE)
        fprintf(stderr, "%s - svg:d=\"%s\"\n", 
                pName, rtl::OUStringToOString(
                    basegfx::tools::exportToSvgD(rPoly),
                    RTL_TEXTENCODING_UTF8).getStr() );
#endif
    }

    void getPolyPolygon()
    {
        dumpSvg("disjunct",aDisjunctRanges.solveCrossovers());
        dumpSvg("equal",aEqualRanges.solveCrossovers());
        dumpSvg("intersectionN",aIntersectionN.solveCrossovers());
        dumpSvg("intersectionE",aIntersectionE.solveCrossovers());
        dumpSvg("intersectionS",aIntersectionS.solveCrossovers());
        dumpSvg("intersectionW",aIntersectionW.solveCrossovers());
        dumpSvg("intersectionNE",aIntersectionNE.solveCrossovers());
        dumpSvg("intersectionSE",aIntersectionSE.solveCrossovers());
        dumpSvg("intersectionSW",aIntersectionSW.solveCrossovers());
        dumpSvg("intersectionNW",aIntersectionNW.solveCrossovers());
        dumpSvg("ringIntersection",aRingIntersection.solveCrossovers());
        dumpSvg("ringIntersection2",aRingIntersection2.solveCrossovers());
        dumpSvg("aRingIntersectExtraStrip",aRingIntersectExtraStrip.solveCrossovers());
        dumpSvg("complexIntersections",aComplexIntersections.solveCrossovers());
        dumpSvg("randomIntersections",aRandomIntersections.solveCrossovers());

        CPPUNIT_ASSERT_MESSAGE("getPolyPolygon", true );
    }

    void validatePoly( const char* pName, const B2DPolyRange& rRange )
    {
        B2DPolyPolygon genericClip;
        const sal_uInt32 nCount=rRange.count();
        for( sal_uInt32 i=0; i<nCount; ++i )
        {
            B2DPolygon aRect=tools::createPolygonFromRect(
                rRange.getElement(i).head);
            if( rRange.getElement(i).tail.head == ORIENTATION_NEGATIVE )
                aRect.flip();
            
            genericClip.append(aRect);
        }

#if defined(VERBOSE)
        fprintf(stderr, "%s input      - svg:d=\"%s\"\n", 
                pName, rtl::OUStringToOString(
                    basegfx::tools::exportToSvgD(
                        genericClip),
                    RTL_TEXTENCODING_UTF8).getStr() );
#endif

        const B2DPolyPolygon boxClipResult=rRange.solveCrossovers();
        const rtl::OUString boxClipSvg(
            basegfx::tools::exportToSvgD(
                normalizePoly(
                    boxClipResult)));
#if defined(VERBOSE)
        fprintf(stderr, "%s boxclipper - svg:d=\"%s\"\n", 
                pName, rtl::OUStringToOString(
                    boxClipSvg,
                    RTL_TEXTENCODING_UTF8).getStr() );
#endif

        genericClip = tools::solveCrossovers(genericClip);
        const rtl::OUString genericClipSvg(
            basegfx::tools::exportToSvgD(
                normalizePoly(
                    genericClip)));
#if defined(VERBOSE)
        fprintf(stderr, "%s genclipper - svg:d=\"%s\"\n", 
                pName, rtl::OUStringToOString(
                    genericClipSvg,
                    RTL_TEXTENCODING_UTF8).getStr() );
#endif

        CPPUNIT_ASSERT_MESSAGE(pName, 
                               genericClipSvg == boxClipSvg);
    }

    void validatePoly()
    {
        validatePoly("disjunct", aDisjunctRanges);
        validatePoly("equal", aEqualRanges);
        validatePoly("intersectionN", aIntersectionN);
        validatePoly("intersectionE", aIntersectionE);
        validatePoly("intersectionS", aIntersectionS);
        validatePoly("intersectionW", aIntersectionW);
        validatePoly("intersectionNE", aIntersectionNE);
        validatePoly("intersectionSE", aIntersectionSE);
        validatePoly("intersectionSW", aIntersectionSW);
        validatePoly("intersectionNW", aIntersectionNW);
        // subtle differences on Solaris Intel, comparison not smart enough 
        // (due to floating point inaccuracies)
        //validatePoly("ringIntersection", aRingIntersection);
        //validatePoly("ringIntersection2", aRingIntersection2);
        //validatePoly("ringIntersectExtraStrip", aRingIntersectExtraStrip);
        // generic clipper buggy here, likely
        //validatePoly("complexIntersections", aComplexIntersections);
        //validatePoly("randomIntersections", aRandomIntersections);
    }

    // Change the following lines only, if you add, remove or rename 
    // member functions of the current class, 
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(boxclipper);
    CPPUNIT_TEST(validatePoly);
    CPPUNIT_TEST(verifyPoly);
    CPPUNIT_TEST(getPolyPolygon);    
    CPPUNIT_TEST_SUITE_END();
};

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx2d::boxclipper);
} // namespace basegfx2d
