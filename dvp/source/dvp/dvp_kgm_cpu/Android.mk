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

LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := $(DVP_DEBUGGING) $(DVP_CFLAGS) $(DVP_FEATURES)
LOCAL_SRC_FILES := dvp_kgm_cpu.c dvp_ll.c
LOCAL_C_INCLUDES += $(DVP_INCLUDES)
LOCAL_MODULE := libdvp_kgm_cpu
LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES := libcutils libdvp

# All the Features should have been defined in the dvp/libraries makefiles

ifeq ($(TARGET_SCPU),C64T)
ifneq (,$(findstring c6xsim,$(VISION_LIBRARIES))
LOCAL_STATIC_LIBRARIES += libc6xsim_$(TARGET_CPU)
endif
endif

ifneq (,$(findstring vlib,$(VISION_LIBRARIES)))
LOCAL_STATIC_LIBRARIES += libvlib_$(TARGET_CPU)
endif

ifneq (,$(findstring rvm,$(VISION_LIBRARIES)))
LOCAL_STATIC_LIBRARIES += librvm_$(TARGET_CPU)
endif

ifneq (,$(findstring rvm_sunex,$(VISION_LIBRARIES)))
LOCAL_STATIC_LIBRARIES += librvm_sunex_$(TARGET_CPU)
endif

ifneq (,$(findstring orb,$(VISION_LIBRARIES)))
LOCAL_STATIC_LIBRARIES += liborb_$(TARGET_CPU)
endif

ifneq (,$(findstring tismo,$(VISION_LIBRARIES)))
LOCAL_STATIC_LIBRARIES += libtismo_$(TARGET_CPU)
endif

ifneq (,$(findstring yuv,$(VISION_LIBRARIES)))
LOCAL_STATIC_LIBRARIES += libyuv
endif

ifneq (,$(findstring imgfilter,$(VISION_LIBRARIES)))
LOCAL_STATIC_LIBRARIES += libimgfilter
endif

ifneq (,$(findstring imglib,$(VISION_LIBRARIES)))
LOCAL_STATIC_LIBRARIES += libimglib_$(TARGET_CPU)
endif

#$(info LOCAL_STATIC_LIBRARIES=$(LOCAL_STATIC_LIBRARIES))

include $(BUILD_SHARED_LIBRARY)

