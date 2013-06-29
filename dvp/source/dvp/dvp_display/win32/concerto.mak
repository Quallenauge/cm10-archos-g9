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

ifeq ($(TARGET_OS),Windows_NT)

include $(PRELUDE)
TARGET=w32window
TARGETTYPE=library
CSOURCES=w32window.c
IDIRS+=$(SOSAL_INC)
include $(FINALE)

# Set the module manually so that we can have 2 builds in one makefile.
_MODULE=w32_test
include $(PRELUDE)
TARGET=w32_test
TARGETTYPE=exe
CSOURCES=w32_test.c
STATIC_LIBS=w32window sosal
SYS_SHARED_LIBS+=Gdi32 Kernel32 $(PLATFORM_LIBS)
ENTRY=WinMainCRTStartup
include $(FINALE)

endif

