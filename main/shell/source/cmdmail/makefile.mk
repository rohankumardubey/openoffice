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
PRJNAME=shell
TARGET=cmdmail
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST=$(TARGET)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

DLLPRE=

SLOFILES= \
    $(SLO)$/cmdmailsuppl.obj \
    $(SLO)$/cmdmailmsg.obj \
    $(SLO)$/cmdmailentry.obj
SHL1OBJS=$(SLOFILES) 
			
SHL1TARGET=$(TARGET).uno
SHL1IMPLIB=i$(TARGET)

SHL1STDLIBS=$(CPPULIB)\
			$(CPPUHELPERLIB)\
			$(SALLIB)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(MISC)/cmdmail.component

$(MISC)/cmdmail.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        cmdmail.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt cmdmail.component
