# Copyright (C) 2012 Texas Instruments, Inc.
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

ifeq ($(BOARD_USES_ARX),true)

PLAT_NUMBERS := $(subst ., ,$(PLATFORM_VERSION))
PLAT_MAJOR := $(word 1,$(PLAT_NUMBERS))
PLAT_MINOR := $(word 2,$(PLAT_NUMBERS))
ifeq ($(PLAT_MAJOR),4)
    ifeq ($(PLAT_MINOR),0)
        TARGET_ANDROID_VERSION := ICS
    else ifeq  ($(PLAT_MINOR),1)
        TARGET_ANDROID_VERSION := JELLYBEAN
    else ifeq  ($(PLAT_MINOR),2)
        TARGET_ANDROID_VERSION := JELLYBEANMR1
    endif
endif

$(info Android Version $(TARGET_ANDROID_VERSION))

TI_HW_ROOT ?= hardware/ti
VISION_ROOT ?= $(TI_HW_ROOT)/vision
DVP_TOP ?= $(TI_HW_ROOT)/dvp
ARX_TOP := $(call my-dir)

ARX_INC := $(ARX_TOP)/include $(ARX_TOP)/source/include
ARTI_INC  := $(VISION_ROOT)/libraries/protected/arti/include
VLIB_INC := $(VISION_ROOT)/libraries/protected/vlib/include
IMGLIB_INC := $(VISION_ROOT)/libraries/protected/imglib/include
OMRONFD_INC := $(VISION_ROOT)/libraries/protected/omronfd/include

DVP_INC := $(DVP_TOP)/include
SOSAL_INC := $(DVP_INC)

ARX_FLAGS := -Wall -D$(TARGET_ANDROID_VERSION)

ARX_CFLAGS := $(ARX_FLAGS) -Werror-implicit-function-declaration
ARX_CPPFLAGS := $(ARX_FLAGS)

# Pull in the DVP Features, but don't build anything.
-include $(DVP_TOP)/libraries/Android.mk

ifdef ARX_DEBUG
ARX_DEBUGGING := -DARX_DEBUG=$(ARX_DEBUG)
ifdef ARX_ZONE_MASK
ARX_DEBUGGING += -DARX_ZONE_MASK=$(ARX_ZONE_MASK)
endif
else
# This chooses ERROR, WARNING
ARX_DEBUGGING := -DARX_DEBUG=1 -DARX_ZONE_MASK=0x0003
endif
ifdef ARX_VALGRIND_DEBUG
ARX_DEBUGGING += -DARX_VALGRIND_DEBUG
endif

include $(call all-makefiles-under, $(ARX_TOP)/source)

endif
