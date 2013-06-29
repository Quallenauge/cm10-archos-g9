# Copyright (C) 2009-2011 Texas Instruments, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ifeq ($(TARGET_OS),LINUX)
ifneq ($(GTK_PATH),)

include $(PRELUDE)
TARGET=gtkwindow
TARGETTYPE=library
CSOURCES=gtkwindow.c
IDIRS+=$(GTK_INC)
include $(FINALE)

# Set the module manually so that we can have 2 builds in one makefile.
_MODULE=gtk_test
include $(PRELUDE)
TARGET=gtk_test
TARGETTYPE=exe
CSOURCES=gtk_test.c
IDIRS+=$(GTK_INC)
LDIRS+=$(GTK_LDIRS)
STATIC_LIBS=gtkwindow sosal
SYS_SHARED_LIBS+=$(GTK_LIBS)
include $(FINALE)

endif
endif

