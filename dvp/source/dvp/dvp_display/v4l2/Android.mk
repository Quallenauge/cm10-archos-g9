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
LOCAL_CFLAGS := $(DVP_DEBUGGING) $(DVP_CFLAGS)
LOCAL_SRC_FILES := v4l2_api.c
LOCAL_C_INCLUDES := $(DVP_INCLUDES)
LOCAL_MODULE := libv4l2
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := tests
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := $(DVP_DEBUGGING)  $(DVP_CFLAGS)
LOCAL_SRC_FILES := v4l2_api.c v4l2_test.c
LOCAL_C_INCLUDES += $(DVP_INCLUDES)
LOCAL_MODULE := v4l2_test
LOCAL_STATIC_LIBRARIES := libv4l2 libsosal
LOCAL_SHARED_LIBRARIES := libutils
include $(BUILD_EXECUTABLE)

