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
#include "precompiled_svx.hxx"
#include <svx/sdr/overlay/overlayobject.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <tools/debug.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <vcl/outdev.hxx>
#include <vcl/salbtype.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/sdr/contact/objectcontacttools.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
	namespace overlay
	{
		void OverlayObject::objectChange() 
		{
			const basegfx::B2DRange aPreviousRange(maBaseRange);
			maBaseRange.reset();
			setPrimitive2DSequence(drawinglayer::primitive2d::Primitive2DSequence());

			if(getOverlayManager() && !aPreviousRange.isEmpty())
			{
				getOverlayManager()->invalidateRange(aPreviousRange);
			}

			const basegfx::B2DRange& rCurrentRange = getBaseRange();

			if(getOverlayManager() && rCurrentRange != aPreviousRange && !rCurrentRange.isEmpty())
			{
				getOverlayManager()->invalidateRange(rCurrentRange);
			}
		}

		// OverlayObject implementations. 
		drawinglayer::primitive2d::Primitive2DSequence OverlayObject::createOverlayObjectPrimitive2DSequence()
        {
            // Default implementation has to assert a missing implementation. It cannot
            // be useful to have overlay object derivations which have no visualisation
            // at all
            OSL_ENSURE(false, "OverlayObject derivation without visualisation definition (missing createOverlayObjectPrimitive2DSequence implementation) (!)");
            return drawinglayer::primitive2d::Primitive2DSequence();
        }

		void OverlayObject::allowAntiAliase(bool bNew)
        {
			if(bNew != (bool)mbAllowsAntiAliase)
			{
				// remember new value
				mbAllowsAntiAliase = bNew;

				// register change (after change)
				objectChange();
			}
        }

		OverlayObject::OverlayObject(Color aBaseColor)
		:	Event(0),
			mpOverlayManager(0),
			maBaseColor(aBaseColor),
			mbIsVisible(true),
			mbIsHittable(true),
			mbAllowsAnimation(false),
            mbAllowsAntiAliase(true)
		{
		}

		OverlayObject::~OverlayObject()
		{
			OSL_ENSURE(0 == getOverlayManager(), "OverlayObject is destructed which is still registered at OverlayManager (!)");
		}

		drawinglayer::primitive2d::Primitive2DSequence OverlayObject::getOverlayObjectPrimitive2DSequence() const
		{
			if(!getPrimitive2DSequence().hasElements())
			{
				// no existing sequence; create one
				const_cast< OverlayObject* >(this)->setPrimitive2DSequence(
					const_cast< OverlayObject* >(this)->createOverlayObjectPrimitive2DSequence());
			}

			return getPrimitive2DSequence();
		}

		const basegfx::B2DRange& OverlayObject::getBaseRange() const
		{
			if(getOverlayManager() && maBaseRange.isEmpty())
			{
				const drawinglayer::primitive2d::Primitive2DSequence& rSequence = getOverlayObjectPrimitive2DSequence();

				if(rSequence.hasElements())
				{
					const drawinglayer::geometry::ViewInformation2D aViewInformation2D(getOverlayManager()->getCurrentViewInformation2D());

					const_cast< sdr::overlay::OverlayObject* >(this)->maBaseRange = 
						drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(rSequence, aViewInformation2D);
				}
			}

			return maBaseRange;
		}

		void OverlayObject::setVisible(bool bNew)
		{
			if(bNew != (bool)mbIsVisible)
			{
				// remember new value
				mbIsVisible = bNew;

				// register change (after change)
				objectChange();
			}
		}

		void OverlayObject::setHittable(bool bNew)
		{
			if(bNew != (bool)mbIsHittable)
			{
				// remember new value
				mbIsHittable = bNew;

				// register change (after change)
				objectChange();
			}
		}

		void OverlayObject::setBaseColor(Color aNew)
		{
			if(aNew != maBaseColor)
			{
				// remember new value
				maBaseColor = aNew;
				
				// register change (after change)
				objectChange();
			}
		}

		void OverlayObject::Trigger(sal_uInt32 /*nTime*/)
		{
			// default does not register again
		}

		void OverlayObject::stripeDefinitionHasChanged()
		{
			// default does not need to do anything
		}
	} // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
	namespace overlay
	{
		OverlayObjectWithBasePosition::OverlayObjectWithBasePosition(const basegfx::B2DPoint& rBasePos, Color aBaseColor)
		:	OverlayObject(aBaseColor),
			maBasePosition(rBasePos)
		{
		}

		OverlayObjectWithBasePosition::~OverlayObjectWithBasePosition()
		{
		}

		void OverlayObjectWithBasePosition::setBasePosition(const basegfx::B2DPoint& rNew)
		{
			if(rNew != maBasePosition)
			{
				// remember new value
				maBasePosition = rNew;

				// register change (after change)
				objectChange();
			}
		}
	} // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
