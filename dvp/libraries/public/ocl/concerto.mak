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

# Only PC/PANDA with Full Distro current supports the OpenCL environment.
ifeq ($(TARGET_PLATFORM),PC)
    # assume it's available
    CL_AVAILABLE := true
    # Mac OSX comes with OpenCL by default if you have installed XCode
    ifneq ($(TARGET_OS),DARWIN) # Win/Linux
        ifndef OPENCL_ROOT # if no SDK defined...
            CL_AVAILABLE := false
        endif
    endif
else ifeq ($(TARGET_PLATFORM),PANDA)
    # panda has a package for OpenCL, check to see if it's been installed.
    CL_AVAILABLE := false
    CL_INC := $(realpath /usr/include/CL)
    ifneq ($(CL_INC),)
        CL_AVAILABLE := true
    endif
else
    # We don't support this anywhere else.
    CL_AVAILABLE := false
endif

ifeq ($(CL_AVAILABLE),true)
include $(PRELUDE)

VISION_LIBRARIES+=ocl
DVP_INC+=$(_MODPATH)/include
DVP_FEATURES+=DVP_USE_OCL

ifdef DVP_LOCAL_BUILD

TARGET=ocl
TARGETTYPE=library
DEFS += CL_BUILD_RUNTIME
ifdef CL_DEBUG
DEFS += CL_DEBUG
endif
CSOURCES=$(call all-c-files)
ifeq ($(TARGET_OS),DARWIN)
    IDIRS+=/Developer/SDKs/MacOSX10.6.sdk/System/Library/Frameworks/OpenCL.framework/Headers
    ifdef CL_DEBUG
        DEFS+=VECLIB DEBUG _GLIBCXX_DEBUG=1 _GLIBCXX_DEBUG_PEDANTIC=1
    endif
else ifeq ($(TARGET_OS),Windows_NT)
    # NVIDIA OpenCL paths
    IDIRS += $(OPENCL_ROOT)/inc
    ifeq ($(HOST_CPU),X86)
        LDIRS += $(OPENCL_ROOT)/lib/Win32
    else ifeq ($(HOST_CPU),X64)
        LDIRS += $(OPENCL_ROOT)/lib/x64
    endif
else ifeq ($(TARGET_OS),LINUX)
    ifdef OPENCL_ROOT
        # NVIDIA OpenCL paths
        IDIRS += $(OPENCL_ROOT)/inc
        ifeq ($(TARGET_CPU),X86)
            LDIRS += $(OPENCL_ROOT)/lib/Linux32
        else ifeq ($(TARGET_CPU),x86_64)
            LDIRS += $(OPENCL_ROOT)/lib/Linux64
        endif
    else
        # Default package in system like panda
    endif
endif
IDIRS += $(DVP_INC)
CL_USER_DEVICE_COUNT ?= 1
CL_USER_DEVICE_TYPE ?= gpu
OCL_ROOT ?= $(realpath $(_MODPATH))
DEFS += CL_USER_DEVICE_COUNT=$(CL_USER_DEVICE_COUNT) CL_USER_DEVICE_TYPE=\"$(CL_USER_DEVICE_TYPE)\" OCL_ROOT=\"$(OCL_ROOT)/\"
endif # LOCAL

include $(FINALE)

endif # CL_AVAILABLE

