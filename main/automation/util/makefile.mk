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



PRJ=..

PRJNAME=automation
TARGET=automation
TARGETTYPE=GUI

# --- Settings ---------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ---------------------------------------------------

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=                 \
	$(SLB)$/server.lib     \
	$(SLB)$/simplecm.lib   \
	$(SLB)$/communi.lib

# --- sts library ---------------------------------------------------

SHL1TARGET= sts$(DLLPOSTFIX)
SHL1IMPLIB= ists

SHL1STDLIBS= \
			$(TOOLSLIB) \
			$(UNOTOOLSLIB) \
			$(SVTOOLLIB) \
			$(SVLLIB)       \
			$(VCLLIB) \
			$(BASICLIB) \
			$(VOSLIB) \
			$(SALLIB) \
			$(CPPUHELPERLIB) \
			$(CPPULIB) \
			$(COMPHELPERLIB) \
			$(SOTLIB)


.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= \
		$(ADVAPI32LIB)	\
		$(GDI32LIB)
.ENDIF

SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
SHL1LIBS=       $(SLB)$/$(TARGET).lib

DEF1NAME        =$(SHL1TARGET)
DEF1DEPN        =       \
	$(MISC)$/$(SHL1TARGET).flt

DEFLIB1NAME     =$(TARGET)
DEF1DES         =TestToolServer

# --- simple communication library (no vcl) ---------------------------------------------------

SHL2TARGET= simplecm$(DLLPOSTFIX)
SHL2IMPLIB= isimplecm

SHL2STDLIBS= \
			$(TOOLSLIB) \
			$(VOSLIB) \
			$(SALLIB)


SHL2DEF=        $(MISC)$/$(SHL2TARGET).def
SHL2LIBS=       $(SLB)$/simplecm.lib 

DEF2NAME        =$(SHL2TARGET)
DEF2DEPN        =       \
	$(MISC)$/$(SHL2TARGET).flt

DEFLIB2NAME     =simplecm
DEF2DES         =SimpleCommunication

# --- communication library ---------------------------------------------------

SHL3TARGET= communi$(DLLPOSTFIX)
SHL3IMPLIB= icommuni

SHL3STDLIBS= \
			$(TOOLSLIB) \
			$(SVLLIB)       \
			$(VCLLIB) \
			$(VOSLIB) \
			$(SALLIB) \
            $(VOSLIB) \
            $(SIMPLECMLIB)


.IF "$(GUI)"=="WNT"
SHL3STDLIBS+= \
		$(ADVAPI32LIB)	\
		$(GDI32LIB)
.ENDIF

SHL3DEF=        $(MISC)$/$(SHL3TARGET).def
SHL3LIBS=       $(SLB)$/communi.lib 
SHL3DEPN=$(SHL2TARGETN)

DEF3NAME        =$(SHL3TARGET)
DEF3DEPN        =       \
	$(MISC)$/$(SHL3TARGET).flt

DEFLIB3NAME     =communi
DEF3DES         =Communication

# --- TESTTOOL IDE ------------------------------------------------------

APP1TARGET=testtool

.IF "$(GUI)"=="UNX" || ("$(COM)"=="GCC" && "$(GUI)"=="WNT")
APP1DEPN+=$(SHL2TARGETN) $(SHL3TARGETN)
.ELSE
APP1DEPN+=$(SHL2IMPLIBN) $(SHL3IMPLIBN)
.ENDIF
APP1STDLIBS= \
		$(APP3RDLIB) \
		$(SAMPLE3RDLIB)

APP1STDLIBS+= \
			$(CPPUHELPERLIB) \
			$(TOOLSLIB) \
            $(UNOTOOLSLIB) \
			$(SVTOOLLIB) \
			$(SVLLIB) \
			$(VCLLIB) \
			$(SALLIB) \
			$(VOSLIB) \
			$(UCBHELPERLIB) \
			$(COMPHELPERLIB) \
            $(BASICLIB) \
            $(SIMPLECMLIB) \
            $(COMMUNILIB)

.IF "$(GUI)"=="UNX"
APP1STDLIBS+= \
			$(VOSLIB) $(SALLIB) $(BASICLIB)
APP1STDLIBS+=$(CPPULIB)
.ENDIF
.IF "$(GUI)"=="WNT" || "$(COM)"=="GCC"
APP1STDLIBS+=$(CPPULIB)
.ENDIF

.IF "$(GUI)"=="WNT"
APP1STDLIBS += \
		$(SHELL32LIB) \
		$(ADVAPI32LIB)
.ENDIF # "$(GUI)"=="WNT"



.IF "$(OS)" == "SOLARIS"
APP1STDLIBS+= -lXm
.ENDIF

.IF "$(GUI)" == "UNX"
.IF "$(OS)" == "LINUX"
APP1STDLIBS+= -lXext -lX11 -lSM -lICE
.ENDIF
.IF "$(OS)"=="MACOSX"
APP1STDLIBS+= -lapp -lsample
.ENDIF
.ENDIF



APP1LIBS=\
		$(LB)$/testtool.lib

APP1OBJS=       $(OBJ)$/testbasi.obj \
                $(OBJ)$/cmdbasestream.obj \
                $(OBJ)$/svcommstream.obj

# --- TESTTOOL MINIAPP ------------------------------------------------------
# ressources are linked here

RESLIB1NAME=tma
RESLIB1SRSFILES= \
                $(SRS)$/miniapp.srs

APP3TARGET=miniapp
APP3STDLIBS= \
			$(CPPULIB) \
			$(CPPUHELPERLIB) \
			$(UCBHELPERLIB) \
			$(COMPHELPERLIB) \
			$(AUTOMATIONLIB) \
			$(SALLIB) \
			$(TOOLSLIB) \
			$(SVTOOLLIB) \
			$(VCLLIB)

.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
APP3STDLIBS+= \
			$(VOSLIB) $(SALLIB) $(LIBSTLPORT)
.ELSE
APP3STDLIBS+= \
			$(VOSLIB) $(SALLIB)
.ENDIF
.ENDIF
# $(AUTOMATIONLIB) is build in SHL1TARGET
.IF "$(GUI)"=="UNX" || ("$(COM)"=="GCC" && "$(GUI)"=="WNT")
APP3DEPN=$(SHL1TARGETN)
.ELSE
APP3DEPN=$(SHL1IMPLIBN)
.ENDIF

APP3LIBS= \
		$(LB)$/miniapp.lib

.IF "$(GUI)" != "UNX"
.IF "$(COM)" != "GCC"
#		win16 braucht ein appobj
APP3OBJS=		$(OBJ)$/testapp.obj
.ENDIF
.ENDIF

# --- Targets -----------------------------------------------------------

.INCLUDE :  target.mk

# --- Basic-Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
	@echo ------------------------------
	@echo Making: $@
	@echo WEP > $@
	@echo LIBMAIN >> $@
	@echo LibMain >> $@
	@echo Sbi >> $@
	@echo exception >> $@
	@echo bad_alloc >> $@
	@echo __CT >> $@

$(MISC)$/$(SHL2TARGET).flt: makefile.mk
	@echo ------------------------------
	@echo Making: $@
	@echo WEP > $@
	@echo LIBMAIN >> $@
	@echo LibMain >> $@
	@echo Sbi >> $@
	@echo exception >> $@
	@echo bad_alloc >> $@
	@echo __CT >> $@

$(MISC)$/$(SHL3TARGET).flt: makefile.mk
	@echo ------------------------------
	@echo Making: $@
	@echo WEP > $@
	@echo LIBMAIN >> $@
	@echo LibMain >> $@
	@echo Sbi >> $@
	@echo exception >> $@
	@echo bad_alloc >> $@
	@echo __CT >> $@


# ------------------------------------------------------------------------

