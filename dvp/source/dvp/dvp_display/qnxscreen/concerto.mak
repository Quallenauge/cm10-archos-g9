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

ifeq ($(TARGET_OS),__QNX__)

include $(PRELUDE)
TARGET=qnxscreen
TARGETTYPE=library
CSOURCES=qnxscreen_api.c
include $(FINALE)

# Set the module manually so that we can have 2 builds in one makefile.
_MODULE=qnxscreen_test
include $(PRELUDE)
TARGET=qnxscreen_test
TARGETTYPE=exe
CSOURCES=qnxscreen_test.c
STATIC_LIBS=qnxscreen sosal
SYS_SHARED_LIBS+=socket OMX_Core screen mmosal
include $(FINALE)

endif
