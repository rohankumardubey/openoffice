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



PRJ=..$/..$/..$/..
PRJNAME=setup_native
TARGET=reg4msdocmsi

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=TRUE
NO_DEFAULT_STL=TRUE
DYNAMIC_CRT=
USE_DEFFILE=TRUE
MINGW_NODLL=YES

.INCLUDE : settings.mk

CFLAGS+=-DUNICODE -D_UNICODE
.IF "$(USE_SYSTEM_STL)" != "YES"
CFLAGS+=-D_STLP_USE_STATIC_LIB
.ENDIF

# --- Files --------------------------------------------------------

.IF "$(GUI)"=="WNT"

UWINAPILIB=

SLOFILES =	$(SLO)$/msihelper.obj\
            $(SLO)$/windowsregistry.obj\
            $(SLO)$/userregistrar.obj\
            $(SLO)$/stringconverter.obj\
            $(SLO)$/registrywnt.obj\
            $(SLO)$/registryw9x.obj\
            $(SLO)$/registryvalueimpl.obj\
            $(SLO)$/registryexception.obj\
            $(SLO)$/registry.obj\
            $(SLO)$/registrationcontextinformation.obj\
            $(SLO)$/registrar.obj\
            $(SLO)$/register.obj\
            $(SLO)$/reg4msdocmsi.obj            

SHL1STDLIBS=	$(KERNEL32LIB)\
				$(USER32LIB)\
				$(ADVAPI32LIB)\
				$(SHELL32LIB)\
				$(MSILIB)\
				$(SHLWAPILIB)

.IF "$(USE_SYSTEM_STL)" != "YES"
SHL1STDLIBS+=$(LIBSTLPORTST)								
.ENDIF

SHL1LIBS = $(SLB)$/$(TARGET).lib 

SHL1TARGET = $(TARGET)
SHL1IMPLIB = i$(TARGET)

SHL1DEF = $(MISC)$/$(SHL1TARGET).def
SHL1DEPN = $(SLB)$/$(TARGET).lib
SHL1BASE = 0x1c000000
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

.ENDIF

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

# -------------------------------------------------------------------------

