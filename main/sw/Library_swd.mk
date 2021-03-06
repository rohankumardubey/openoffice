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



$(eval $(call gb_Library_Library,swd))

$(eval $(call gb_Library_set_componentfile,swd,sw/util/swd))

$(eval $(call gb_Library_set_include,swd,\
	$$(INCLUDE) \
	-I$(WORKDIR)/inc/sw/sdi \
	-I$(WORKDIR)/inc/sw \
	-I$(SRCDIR)/sw/inc \
	-I$(SRCDIR)/sw/inc/pch \
	-I$(SRCDIR)/sw/source/core/inc \
	-I$(SRCDIR)/sw/source/filter/inc \
	-I$(SRCDIR)/sw/source/ui/inc \
	-I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_set_defs,swd,\
	$$(DEFS) \
))

$(eval $(call gb_Library_add_linked_libs,swd,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sfx \
	sot \
	svl \
	svt \
	tl \
	ucbhelper \
	utl \
	vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,swd,\
	sw/source/core/except/errhdl \
	sw/source/filter/basflt/iodetect \
	sw/source/ui/uno/detreg \
	sw/source/ui/uno/swdet2 \
	sw/source/ui/uno/swdetect \
))

# vim: set noet sw=4 ts=4:
