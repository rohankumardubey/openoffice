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



#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX
#define _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <vector>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b3dpoint.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
	// predefinitions
	class B3DPolyPolygon;
	class B3DRange;

	namespace tools
	{
		// B3DPolyPolygon tools

		// get size of PolyPolygon. Control vectors are included in that ranges.
		B3DRange getRange(const B3DPolyPolygon& rCandidate);

		/** Apply given LineDashing to given polyPolygon

			For a description see applyLineDashing in b2dpolygontoos.hxx
		*/
		void applyLineDashing(
			const B3DPolyPolygon& rCandidate, 
			const ::std::vector<double>& rDotDashArray, 
			B3DPolyPolygon* pLineTarget,
            B3DPolyPolygon* pGapTarget = 0,
			double fFullDashDotLen = 0.0);

		/** Create a unit 3D line polyPolygon which defines a cube.
         */
		B3DPolyPolygon createUnitCubePolyPolygon();

		/** Create a unit 3D fill polyPolygon which defines a cube.
         */
		B3DPolyPolygon createUnitCubeFillPolyPolygon();

		/** Create a 3D line polyPolygon from a B3DRange which defines a cube.
         */
		B3DPolyPolygon createCubePolyPolygonFromB3DRange( const B3DRange& rRange);

		/** Create a 3D fill polyPolygon from a B3DRange which defines a cube.
         */
		B3DPolyPolygon createCubeFillPolyPolygonFromB3DRange( const B3DRange& rRange);

		/** Create a unit 3D line polyPolygon which defines a sphere with the given count of hor and ver segments.
			Result will be centered at (0.0, 0.0, 0.0) and sized [-1.0 .. 1.0] in all dimensions.
			If nHorSeg == 0 and/or nVerSeg == 0, a default will be calculated to have a step at least each 15 degrees.
			With VerStart, VerStop and hor range in cartesian may be specified to create a partial sphere only.
         */
		B3DPolyPolygon createUnitSpherePolyPolygon(
			sal_uInt32 nHorSeg = 0L, sal_uInt32 nVerSeg = 0L,
			double fVerStart = F_PI2, double fVerStop = -F_PI2,
			double fHorStart = 0.0, double fHorStop = F_2PI);

		/** Create a 3D line polyPolygon from a B3DRange which defines a sphere with the given count of hor and ver segments.
			If nHorSeg == 0 and/or nVerSeg == 0, a default will be calculated to have a step at least each 15 degrees.
			With VerStart, VerStop and hor range in cartesian may be specified to create a partial sphere only.
         */
		B3DPolyPolygon createSpherePolyPolygonFromB3DRange( 
			const B3DRange& rRange, 
			sal_uInt32 nHorSeg = 0L, sal_uInt32 nVerSeg = 0L,
			double fVerStart = F_PI2, double fVerStop = -F_PI2,
			double fHorStart = 0.0, double fHorStop = F_2PI);

		/** same as createUnitSpherePolyPolygon, but creates filled polygons (closed and oriented)
			There is one extra, the bool bNormals defines if normals will be set, default is false
         */
		B3DPolyPolygon createUnitSphereFillPolyPolygon(
			sal_uInt32 nHorSeg = 0L, sal_uInt32 nVerSeg = 0L,
			bool bNormals = false,
			double fVerStart = F_PI2, double fVerStop = -F_PI2,
			double fHorStart = 0.0, double fHorStop = F_2PI);

		/** same as createSpherePolyPolygonFromB3DRange, but creates filled polygons (closed and oriented)
			There is one extra, the bool bNormals defines if normals will be set, default is false
         */
		B3DPolyPolygon createSphereFillPolyPolygonFromB3DRange( 
			const B3DRange& rRange, 
			sal_uInt32 nHorSeg = 0L, sal_uInt32 nVerSeg = 0L,
			bool bNormals = false,
			double fVerStart = F_PI2, double fVerStop = -F_PI2,
			double fHorStart = 0.0, double fHorStop = F_2PI);

		/** Create/replace normals for given 3d geometry with default normals from given center to outside.
			rCandidate:	the 3d geometry to change
			rCenter:	the center of the 3d geometry
         */
		B3DPolyPolygon applyDefaultNormalsSphere( const B3DPolyPolygon& rCandidate, const B3DPoint& rCenter);

		/** invert normals for given 3d geometry.
         */
		B3DPolyPolygon invertNormals( const B3DPolyPolygon& rCandidate);

		/** Create/replace texture coordinates for given 3d geometry with parallel projected one
			rRange: the full range of the 3d geometry
			If bChangeX, x texture coordinate will be recalculated.
			If bChangeY, y texture coordinate will be recalculated.
         */
		B3DPolyPolygon applyDefaultTextureCoordinatesParallel( const B3DPolyPolygon& rCandidate, const B3DRange& rRange, bool bChangeX = true, bool bChangeY = true);

		/** Create/replace texture coordinates for given 3d geometry with spherical one
			rCenter: the centre of the used 3d geometry
			If bChangeX, x texture coordinate will be recalculated.
			If bChangeY, y texture coordinate will be recalculated.
         */
		B3DPolyPolygon applyDefaultTextureCoordinatesSphere( const B3DPolyPolygon& rCandidate, const B3DPoint& rCenter, bool bChangeX = true, bool bChangeY = true);

        // isInside test for B3DPoint. On border is not inside as long as not true is given 
		// in bWithBorder flag. It is assumed that the orientations of the given polygon are correct.
		bool isInside(const B3DPolyPolygon& rCandidate, const B3DPoint& rPoint, bool bWithBorder = false);

		//////////////////////////////////////////////////////////////////////
		// comparators with tolerance for 3D PolyPolygons
		bool equal(const B3DPolyPolygon& rCandidateA, const B3DPolyPolygon& rCandidateB, const double& rfSmallValue);
		bool equal(const B3DPolyPolygon& rCandidateA, const B3DPolyPolygon& rCandidateB);

	} // end of namespace tools
} // end of namespace basegfx

#endif /* _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX */
