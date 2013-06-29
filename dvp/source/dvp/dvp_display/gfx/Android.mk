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

ifeq ($(TARGET_ANDROID_VERSION),ICS)
BUILD_GFX:=1
else ifeq ($(TARGET_ANDROID_VERSION),JELLYBEAN)
BUILD_GFX:=1
endif

ifeq ($(BUILD_GFX),1)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := $(DVP_DEBUGGING) $(DVP_CFLAGS)
LOCAL_SRC_FILES := gfx_display.c
LOCAL_C_INCLUDES := $(DVP_INCLUDES)
LOCAL_MODULE := libgfxdisp
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := tests
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := $(DVP_DEBUGGING) $(DVP_CFLAGS) -DTEST
LOCAL_SRC_FILES := gfx_display.c
LOCAL_C_INCLUDES +=$(DVP_INCLUDES)
LOCAL_MODULE := gfxd_test
LOCAL_STATIC_LIBRARIES := libsosal
LOCAL_SHARED_LIBRARIES := libutils libhardware
LOCAL_LDLIBS += -lhwcomposer.omap4.so
include $(BUILD_EXECUTABLE)
endif
