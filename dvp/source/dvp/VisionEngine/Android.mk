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

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CPPFLAGS := $(DVP_DEBUGGING) $(DVP_CPPFLAGS) $(DVP_FEATURES)
LOCAL_SRC_FILES := VisionEngine.cpp
LOCAL_C_INCLUDES += $(DVP_INCLUDES)
LOCAL_WHOLE_STATIC_LIBRARIES := libvcam libsosal libimgdbg libcthreaded
ifeq ($(TARGET_ANDROID_VERSION),ICS)
LOCAL_WHOLE_STATIC_LIBRARIES += libanw
endif
LOCAL_MODULE := libVisionEngine
include $(BUILD_STATIC_LIBRARY)

