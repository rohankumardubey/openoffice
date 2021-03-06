#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



PRJ=..$/..
PRJNAME=sdext
TARGET=PresenterScreen
GEN_HID=FALSE
EXTNAME=PresenterScreen

ENABLE_EXCEPTIONS=TRUE
# survive zip dependencies
MAXLINELENGTH:=80000

# --- Settings ----------------------------------

.INCLUDE : rtlbootstrap.mk
.INCLUDE : settings.mk

PACKAGE=com.sun.PresenterScreen-$(PLATFORMID)

.IF "$(ENABLE_PRESENTER_SCREEN)" == "NO"
@all:
	@echo "Presenter Screen build disabled."
.ELSE

.IF "$(L10N_framework)" != ""

@all:
	@echo "L10N framework disabled => Presenter Screen can not be built."

.ELSE

.INCLUDE :  $(PRJ)$/util$/makefile.pmk


DLLPRE=
common_build_zip=

# --- Files -------------------------------------

SLOFILES=										\
	$(SLO)$/PresenterAccessibility.obj			\
	$(SLO)$/PresenterAnimation.obj				\
	$(SLO)$/PresenterAnimator.obj				\
	$(SLO)$/PresenterBitmapContainer.obj		\
	$(SLO)$/PresenterButton.obj					\
	$(SLO)$/PresenterCanvasHelper.obj			\
	$(SLO)$/PresenterConfigurationAccess.obj	\
	$(SLO)$/PresenterController.obj				\
	$(SLO)$/PresenterCurrentSlideObserver.obj	\
	$(SLO)$/PresenterFrameworkObserver.obj		\
	$(SLO)$/PresenterGeometryHelper.obj			\
	$(SLO)$/PresenterHelper.obj					\
	$(SLO)$/PresenterHelpView.obj				\
	$(SLO)$/PresenterNotesView.obj				\
	$(SLO)$/PresenterPaintManager.obj			\
	$(SLO)$/PresenterPane.obj					\
	$(SLO)$/PresenterPaneAnimator.obj			\
	$(SLO)$/PresenterPaneBase.obj				\
	$(SLO)$/PresenterPaneBorderManager.obj		\
	$(SLO)$/PresenterPaneBorderPainter.obj		\
	$(SLO)$/PresenterPaneContainer.obj			\
	$(SLO)$/PresenterPaneFactory.obj			\
	$(SLO)$/PresenterProtocolHandler.obj		\
	$(SLO)$/PresenterScreen.obj					\
	$(SLO)$/PresenterScrollBar.obj				\
	$(SLO)$/PresenterSlidePreview.obj			\
	$(SLO)$/PresenterSlideShowView.obj			\
	$(SLO)$/PresenterSlideSorter.obj			\
	$(SLO)$/PresenterSprite.obj					\
	$(SLO)$/PresenterSpritePane.obj				\
	$(SLO)$/PresenterTextView.obj				\
	$(SLO)$/PresenterTheme.obj					\
	$(SLO)$/PresenterTimer.obj					\
	$(SLO)$/PresenterToolBar.obj				\
	$(SLO)$/PresenterUIPainter.obj				\
	$(SLO)$/PresenterViewFactory.obj			\
	$(SLO)$/PresenterWindowManager.obj			\
	$(SLO)$/PresenterComponent.obj


# --- Library -----------------------------------

SHL1TARGET=		$(TARGET).uno

SHL1STDLIBS=	$(CPPUHELPERLIB)	\
				$(CPPULIB)			\
				$(SALLIB)

SHL1DEPN=
SHL1IMPLIB=		i$(SHL1TARGET)
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
SHL1VERSIONMAP=$(SOLARENV)/src/reg-component.map
SHL1RPATH=      OXT
DEF1NAME=		$(SHL1TARGET)

ZIP2TARGET=		presenter-screen_develop
ZIP2DIR=		$(COMMONMISC)
ZIP2EXT=		.zip
ZIP2FLAGS=-r
ZIP2LIST=		*/com.sun.PresenterScreen/*.xhp

ZIP1TARGET=		presenter-screen
ZIP1DIR=		$(MISC)$/$(TARGET)
ZIP1EXT=		.oxt
ZIP1FLAGS=-r
ZIP1LIST=		*

EXTENSIONDIR=$(ZIP1DIR)

.INCLUDE : extension_pre.mk

.IF "$(WITH_LANG)"==""
FIND_XCU=registry/data
.ELSE			# "$(WITH_LANG)"==""
FIND_XCU=$(MISC)$/$(EXTNAME)_in$/merge
.ENDIF			# "$(WITH_LANG)"==""

COMPONENT_FILES=																			\
    $(ZIP1DIR)$/registry$/data$/org$/openoffice$/Office$/Jobs.xcu							\
    $(ZIP1DIR)$/registry$/data$/org$/openoffice$/Office$/ProtocolHandler.xcu				\
    $(ZIP1DIR)$/registry$/schema/org$/openoffice$/Office$/extension$/PresenterScreen.xcs   	\
	$(ZIP1DIR)$/registry$/data/$/org$/openoffice$/Office$/extension$/PresenterScreen.xcu 

COMPONENT_BITMAPS=												\
	$(ZIP1DIR)$/bitmaps$/BorderTop.png							\
	$(ZIP1DIR)$/bitmaps$/BorderTopLeft.png						\
	$(ZIP1DIR)$/bitmaps$/BorderTopRight.png						\
	$(ZIP1DIR)$/bitmaps$/BorderLeft.png							\
	$(ZIP1DIR)$/bitmaps$/BorderRight.png						\
	$(ZIP1DIR)$/bitmaps$/BorderBottomLeft.png					\
	$(ZIP1DIR)$/bitmaps$/BorderBottomRight.png					\
	$(ZIP1DIR)$/bitmaps$/BorderBottom.png						\
																\
	$(ZIP1DIR)$/bitmaps$/BorderActiveTop.png					\
	$(ZIP1DIR)$/bitmaps$/BorderActiveTopLeft.png				\
	$(ZIP1DIR)$/bitmaps$/BorderActiveTopRight.png				\
	$(ZIP1DIR)$/bitmaps$/BorderActiveLeft.png					\
	$(ZIP1DIR)$/bitmaps$/BorderActiveRight.png					\
	$(ZIP1DIR)$/bitmaps$/BorderActiveBottomLeft.png				\
	$(ZIP1DIR)$/bitmaps$/BorderActiveBottomRight.png			\
	$(ZIP1DIR)$/bitmaps$/BorderActiveBottom.png					\
	$(ZIP1DIR)$/bitmaps$/BorderActiveBottomCallout.png			\
																\
	$(ZIP1DIR)$/bitmaps$/BorderCurrentSlideTop.png				\
	$(ZIP1DIR)$/bitmaps$/BorderCurrentSlideTopLeft.png			\
	$(ZIP1DIR)$/bitmaps$/BorderCurrentSlideTopRight.png			\
	$(ZIP1DIR)$/bitmaps$/BorderCurrentSlideLeft.png				\
	$(ZIP1DIR)$/bitmaps$/BorderCurrentSlideRight.png			\
	$(ZIP1DIR)$/bitmaps$/BorderCurrentSlideBottomLeft.png		\
	$(ZIP1DIR)$/bitmaps$/BorderCurrentSlideBottomRight.png		\
	$(ZIP1DIR)$/bitmaps$/BorderCurrentSlideBottom.png			\
																\
	$(ZIP1DIR)$/bitmaps$/BorderToolbarTop.png					\
	$(ZIP1DIR)$/bitmaps$/BorderToolbarTopLeft.png				\
	$(ZIP1DIR)$/bitmaps$/BorderToolbarTopRight.png				\
	$(ZIP1DIR)$/bitmaps$/BorderToolbarLeft.png					\
	$(ZIP1DIR)$/bitmaps$/BorderToolbarRight.png					\
	$(ZIP1DIR)$/bitmaps$/BorderToolbarBottom.png				\
																\
	$(ZIP1DIR)$/bitmaps$/Background.png							\
	$(ZIP1DIR)$/bitmaps$/ViewBackground.png						\
																\
	$(ZIP1DIR)$/bitmaps$/ButtonSlidePreviousNormal.png			\
	$(ZIP1DIR)$/bitmaps$/ButtonSlidePreviousMouseOver.png		\
	$(ZIP1DIR)$/bitmaps$/ButtonSlidePreviousSelected.png		\
	$(ZIP1DIR)$/bitmaps$/ButtonSlidePreviousDisabled.png		\
	$(ZIP1DIR)$/bitmaps$/ButtonEffectNextNormal.png				\
	$(ZIP1DIR)$/bitmaps$/ButtonEffectNextMouseOver.png			\
	$(ZIP1DIR)$/bitmaps$/ButtonEffectNextSelected.png			\
	$(ZIP1DIR)$/bitmaps$/ButtonEffectNextDisabled.png			\
	$(ZIP1DIR)$/bitmaps$/ButtonNotesNormal.png					\
	$(ZIP1DIR)$/bitmaps$/ButtonNotesMouseOver.png				\
	$(ZIP1DIR)$/bitmaps$/ButtonNotesSelected.png				\
	$(ZIP1DIR)$/bitmaps$/ButtonNotesDisabled.png				\
	$(ZIP1DIR)$/bitmaps$/ButtonSlideSorterNormal.png			\
	$(ZIP1DIR)$/bitmaps$/ButtonSlideSorterMouseOver.png			\
	$(ZIP1DIR)$/bitmaps$/ButtonSlideSorterSelected.png			\
	$(ZIP1DIR)$/bitmaps$/ButtonSlideSorterDisabled.png			\
	$(ZIP1DIR)$/bitmaps$/ButtonHelpNormal.png					\
	$(ZIP1DIR)$/bitmaps$/ButtonHelpMouseOver.png				\
	$(ZIP1DIR)$/bitmaps$/ButtonHelpSelected.png					\
	$(ZIP1DIR)$/bitmaps$/ButtonHelpDisabled.png					\
																\
	$(ZIP1DIR)$/bitmaps/ScrollbarArrowUpNormal.png				\
	$(ZIP1DIR)$/bitmaps/ScrollbarArrowUpMouseOver.png			\
	$(ZIP1DIR)$/bitmaps/ScrollbarArrowUpSelected.png			\
	$(ZIP1DIR)$/bitmaps/ScrollbarArrowUpDisabled.png			\
	$(ZIP1DIR)$/bitmaps/ScrollbarArrowDownNormal.png			\
	$(ZIP1DIR)$/bitmaps/ScrollbarArrowDownMouseOver.png			\
	$(ZIP1DIR)$/bitmaps/ScrollbarArrowDownSelected.png			\
	$(ZIP1DIR)$/bitmaps/ScrollbarArrowDownDisabled.png			\
	$(ZIP1DIR)$/bitmaps/ScrollbarPagerMiddleNormal.png			\
	$(ZIP1DIR)$/bitmaps/ScrollbarPagerMiddleMouseOver.png		\
	$(ZIP1DIR)$/bitmaps/ScrollbarThumbTopNormal.png				\
	$(ZIP1DIR)$/bitmaps/ScrollbarThumbTopMouseOver.png			\
	$(ZIP1DIR)$/bitmaps/ScrollbarThumbBottomNormal.png			\
	$(ZIP1DIR)$/bitmaps/ScrollbarThumbBottomMouseOver.png		\
	$(ZIP1DIR)$/bitmaps/ScrollbarThumbMiddleNormal.png			\
	$(ZIP1DIR)$/bitmaps/ScrollbarThumbMiddleMouseOver.png		\
																\
	$(ZIP1DIR)$/bitmaps$/ButtonPlusNormal.png					\
	$(ZIP1DIR)$/bitmaps$/ButtonPlusMouseOver.png				\
	$(ZIP1DIR)$/bitmaps$/ButtonPlusSelected.png					\
	$(ZIP1DIR)$/bitmaps$/ButtonPlusDisabled.png					\
	$(ZIP1DIR)$/bitmaps$/ButtonMinusNormal.png					\
	$(ZIP1DIR)$/bitmaps$/ButtonMinusMouseOver.png				\
	$(ZIP1DIR)$/bitmaps$/ButtonMinusSelected.png				\
	$(ZIP1DIR)$/bitmaps$/ButtonMinusDisabled.png				\
																\
	$(ZIP1DIR)$/bitmaps$/ButtonFrameLeftNormal.png				\
	$(ZIP1DIR)$/bitmaps$/ButtonFrameCenterNormal.png			\
	$(ZIP1DIR)$/bitmaps$/ButtonFrameRightNormal.png				\
	$(ZIP1DIR)$/bitmaps$/ButtonFrameLeftMouseOver.png			\
	$(ZIP1DIR)$/bitmaps$/ButtonFrameCenterMouseOver.png			\
	$(ZIP1DIR)$/bitmaps$/ButtonFrameRightMouseOver.png			\
																\
	$(ZIP1DIR)$/bitmaps$/LabelMouseOverLeft.png					\
	$(ZIP1DIR)$/bitmaps$/LabelMouseOverCenter.png				\
    $(ZIP1DIR)$/bitmaps$/LabelMouseOverRight.png

COMPONENT_IMAGES=\
	$(ZIP1DIR)$/bitmaps$/extension_32.png \
	$(ZIP1DIR)$/bitmaps$/extension_32_h.png

COMPONENT_LIBRARY= 								\
	$(ZIP1DIR)$/$(TARGET).uno$(DLLPOST)

PLATFORMID:=$(RTL_OS:l)_$(RTL_ARCH:l)

COMPONENT_HELP= 								\
	$(ZIP1DIR)$/help/component.txt				\
	$(foreach,l,$(alllangiso) $(ZIP1DIR)$/help$/$l$/com.sun.PresenterScreen-$(PLATFORMID)$/presenter.xhp)

ZIP1DEPS=					\
	$(PACKLICS) 			\
	$(DESCRIPTION)			\
	$(COMPONENT_MANIFEST)	\
	$(COMPONENT_FILES)		\
	$(COMPONENT_BITMAPS)	\
	$(COMPONENT_IMAGES)    	\
	$(COMPONENT_LIBRARY)	\
	$(COMPONENT_HELP)

#	$(COMPONENT_MERGED_XCU) \


LINKNAME:=help
XHPLINKSRC:=$(ZIP1DIR)/help

my_XHPFILES= \
    presenter.xhp

LINKLINKFILES= \
    $(PACKAGE)/{$(my_XHPFILES)}

# --- Targets ----------------------------------

.INCLUDE : target.mk
.INCLUDE : extension_helplink.mk

$(SLO)$/PresenterComponent.obj : $(INCCOM)$/PresenterExtensionIdentifier.hxx

$(INCCOM)$/PresenterExtensionIdentifier.hxx : PresenterExtensionIdentifier.txx
	$(TYPE) $< | sed s/UPDATED_PLATFORM/$(PLATFORMID)/ > $@

$(ZIP1DIR)$/help$/component.txt : help$/$$(@:f)
	@@-$(MKDIRHIER) $(@:d)
	$(COPY) $< $@

$(ZIP1DIR)/help/%/com.sun.PresenterScreen-$(PLATFORMID)/presenter.xhp : $(COMMONMISC)/%/com.sun.PresenterScreen/presenter.xhp
	@echo creating $@
	@-$(MKDIRHIER) $(@:d)
	$(TYPE) $< | sed "s/PLATFORMID/$(PLATFORMID)/" | sed 's/@PRESENTEREXTENSIONPRODUCTNAME@/Presenter Console/g' > $@

.IF "$(ZIP1TARGETN)"!=""
$(ZIP1TARGETN) : $(HELPLINKALLTARGETS)

.ENDIF          # "$(ZIP1TARGETN)"!=""

$(COMPONENT_BITMAPS) : bitmaps$/$$(@:f)
	@-$(MKDIRHIER) $(@:d)
	+$(COPY) $< $@

$(COMPONENT_IMAGES) : $(SOLARSRC)$/$(RSCDEFIMG)$/desktop$/res$/$$(@:f)
	@@-$(MKDIRHIER) $(@:d)
	$(COPY) $< $@

$(COMPONENT_LIBRARY) : $(DLLDEST)$/$$(@:f)
	@-$(MKDIRHIER) $(@:d)
	+$(COPY) $< $@
.IF "$(OS)$(CPU)"=="WNTI"
 .IF "$(COM)"=="GCC"
    $(GNUCOPY) $(SOLARBINDIR)$/mingwm10.dll $(ZIP1DIR)
 .ELSE
	.IF "$(PACKMS)"!=""
		.IF "$(CCNUMVER)" <= "001399999999"
			$(GNUCOPY) $(PACKMS)$/msvcr71.dll $(ZIP1DIR)
			$(GNUCOPY) $(PACKMS)$/msvcp71.dll $(ZIP1DIR)
		.ELSE
			.IF "$(CCNUMVER)" <= "001499999999"
				$(GNUCOPY) $(PACKMS)$/msvcr80.dll $(ZIP1DIR)
				$(GNUCOPY) $(PACKMS)$/msvcp80.dll $(ZIP1DIR)
		    	$(GNUCOPY) $(PACKMS)$/msvcm80.dll $(ZIP1DIR)
				$(GNUCOPY) $(PACKMS)$/Microsoft.VC80.CRT.manifest $(ZIP1DIR)
			.ELSE
				$(GNUCOPY) $(PACKMS)$/msvcr90.dll $(ZIP1DIR)
				$(GNUCOPY) $(PACKMS)$/msvcp90.dll $(ZIP1DIR)
		    	$(GNUCOPY) $(PACKMS)$/msvcm90.dll $(ZIP1DIR)
			    $(GNUCOPY) $(PACKMS)$/Microsoft.VC90.CRT.manifest $(ZIP1DIR)
			.ENDIF
	    .ENDIF
	.ELSE        # "$(PACKMS)"!=""
		.IF "$(CCNUMVER)" <= "001399999999"
			$(GNUCOPY) $(SOLARBINDIR)$/msvcr71.dll $(ZIP1DIR)
			$(GNUCOPY) $(SOLARBINDIR)$/msvcp71.dll $(ZIP1DIR)
		.ELSE
			.IF "$(CCNUMVER)" <= "001499999999"
		    	$(GNUCOPY) $(SOLARBINDIR)$/msvcr80.dll $(ZIP1DIR)
			    $(GNUCOPY) $(SOLARBINDIR)$/msvcp80.dll $(ZIP1DIR)
				$(GNUCOPY) $(SOLARBINDIR)$/msvcm80.dll $(ZIP1DIR)
				$(GNUCOPY) $(SOLARBINDIR)$/Microsoft.VC80.CRT.manifest $(ZIP1DIR)
			.ELSE
	    		$(GNUCOPY) $(SOLARBINDIR)$/msvcr90.dll $(ZIP1DIR)
	    		$(GNUCOPY) $(SOLARBINDIR)$/msvcp90.dll $(ZIP1DIR)
		    	$(GNUCOPY) $(SOLARBINDIR)$/msvcm90.dll $(ZIP1DIR)
		    	$(GNUCOPY) $(SOLARBINDIR)$/Microsoft.VC90.CRT.manifest $(ZIP1DIR)
			.ENDIF
	    .ENDIF
	.ENDIF         # "$(PACKMS)"!=""
 .ENDIF	#"$(COM)"=="GCC"
.ENDIF

.INCLUDE : extension_post.mk

.ENDIF # L10N_framework
.ENDIF # "$(ENABLE_PRESENTER_SCREEN)" != "NO"
