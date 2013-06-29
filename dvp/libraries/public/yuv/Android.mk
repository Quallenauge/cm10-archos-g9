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

# This can only be included when NEON is present
ifeq ($(TARGET_ARCH_VARIANT),armv7-a-neon)
VISION_LIBRARIES += yuv

ifdef DVP_LOCAL_BUILD
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := $(DVP_DEBUGGING) $(DVP_CFLAGS)
LOCAL_SRC_FILES := $(notdir $(wildcard $(LOCAL_PATH)/*.S))
LOCAL_C_INCLUDES += $(DVP_INCLUDES)
LOCAL_MODULE := libyuv
include $(BUILD_STATIC_LIBRARY)
endif
endif

