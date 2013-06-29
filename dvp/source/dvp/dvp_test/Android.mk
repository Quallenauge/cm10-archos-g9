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
LOCAL_MODULE_TAGS := tests
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CPPFLAGS := $(DVP_DEBUGGING) $(DVP_CPPFLAGS) $(DVP_FEATURES)
LOCAL_SRC_FILES := dvp_test.cpp TestVisionEngine.cpp
LOCAL_C_INCLUDES += $(DVP_INCLUDES)
LOCAL_STATIC_LIBRARIES := libVisionEngine
LOCAL_SHARED_LIBRARIES := libdvp libcutils libOMX_Core libbinder libutils libgui libmedia libhardware libui libcamera_client
LOCAL_MODULE := dvp_test
include $(BUILD_EXECUTABLE)

ifneq (,$(findstring dsplib,$(VISION_LIBRARIES)))
# A simple unit test for DVP
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := tests
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := $(DVP_DEBUGGING) $(DVP_CFLAGS) $(DVP_FEATURES)
LOCAL_SRC_FILES := dvp_dsplib.c
LOCAL_C_INCLUDES += $(DVP_INCLUDES)
LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES := libdvp libcutils libbinder
LOCAL_MODULE := dvp_dsplib
include $(BUILD_EXECUTABLE)
endif

# A simple unit test for DVP
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := tests
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := $(DVP_DEBUGGING) $(DVP_CFLAGS) $(DVP_FEATURES)
LOCAL_SRC_FILES := dvp_simple.c
LOCAL_C_INCLUDES += $(DVP_INCLUDES)
LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES := libdvp libcutils libbinder
LOCAL_MODULE := dvp_simple
include $(BUILD_EXECUTABLE)


# A test for Deinterlacer and color converter in DVP
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := tests
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := $(DVP_DEBUGGING) $(DVP_CFLAGS) $(DVP_FEATURES)
LOCAL_SRC_FILES := dvp_dein.c
LOCAL_C_INCLUDES += $(DVP_INCLUDES)
LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES := libdvp libcutils libbinder
LOCAL_MODULE := dvp_dein
include $(BUILD_EXECUTABLE)
	

# A query utility for DVP
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := tests
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := $(DVP_DEBUGGING) $(DVP_CFLAGS) $(DVP_FEATURES)
LOCAL_SRC_FILES := dvp_info.c
LOCAL_C_INCLUDES += $(DVP_INCLUDES)
LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES := libdvp libcutils libbinder
LOCAL_MODULE := dvp_info
include $(BUILD_EXECUTABLE)

# A DVP memory share/import unit test
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := tests
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CPPFLAGS := $(DVP_DEBUGGING) $(DVP_CPPFLAGS) $(DVP_FEATURES)
LOCAL_SRC_FILES := dvp_share.cpp
LOCAL_C_INCLUDES += $(DVP_INCLUDES)
LOCAL_SHARED_LIBRARIES := libdvp libutils libcutils libbinder
LOCAL_MODULE := dvp_share
include $(BUILD_EXECUTABLE)

# A DVP crash recovery unit test
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := tests
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := $(DVP_DEBUGGING) $(DVP_CFLAGS) $(DVP_FEATURES)
LOCAL_SRC_FILES := dvp_crash_test.c
LOCAL_C_INCLUDES += $(DVP_INCLUDES)
LOCAL_SHARED_LIBRARIES := libdvp libutils libcutils libbinder
LOCAL_MODULE := dvp_crash_test
include $(BUILD_EXECUTABLE)

# A framework unit test for DVP
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := tests
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := $(DVP_DEBUGGING) $(DVP_CFLAGS) $(DVP_FEATURES)
LOCAL_SRC_FILES := dvp_unittest.c
LOCAL_C_INCLUDES += $(DVP_INCLUDES)
LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES := libdvp libcutils libbinder
LOCAL_MODULE := dvp_unittest
include $(BUILD_EXECUTABLE)
