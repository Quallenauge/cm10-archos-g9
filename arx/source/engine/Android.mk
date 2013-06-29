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

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := $(ARX_INC) $(DVP_INC)
LOCAL_CPPFLAGS := $(ARX_DEBUGGING) $(ARX_CPPFLAGS) $(DVP_FEATURES)
LOCAL_SRC_FILES := ARXEngine.cpp
LOCAL_WHOLE_STATIC_LIBRARIES := libVisionEngine
LOCAL_MODULE := libarxengine_base
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := $(ARX_INC) $(DVP_INC) $(VLIB_INC)
LOCAL_CPPFLAGS := $(ARX_DEBUGGING) $(ARX_CPPFLAGS) $(DVP_FEATURES)
LOCAL_SRC_FILES := DVPARXEngine.cpp
LOCAL_WHOLE_STATIC_LIBRARIES := libarxengine_base
ifneq (,$(findstring omronfd,$(VISION_LIBRARIES)))
LOCAL_C_INCLUDES += $(OMRONFD_INC)
LOCAL_CPPFLAGS += -DARX_USE_OMRONFD 
LOCAL_SRC_FILES += FacePose.cpp
LOCAL_WHOLE_STATIC_LIBRARIES += libeOkaoDt libeOkaoPt libeOkaoCo
endif
LOCAL_SHARED_LIBRARIES := libarxbuf libdvp libutils libcutils libbinder \
                          libui libgui libhardware libdl libOMX_Core
ifeq ($(TARGET_ANDROID_VERSION),JELLYBEANMR1)
    LOCAL_SHARED_LIBRARIES += libion_ti
else
    LOCAL_SHARED_LIBRARIES += libion
endif
LOCAL_MODULE := libarxengine
include $(BUILD_SHARED_LIBRARY)

ifneq (,$(findstring arti,$(VISION_LIBRARIES)))
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := $(ARX_INC) $(DVP_INC) $(VLIB_INC) $(IMGLIB_INC) $(ARTI_INC)
LOCAL_CPPFLAGS := $(ARX_DEBUGGING) $(ARX_CPPFLAGS) $(DVP_FEATURES)
LOCAL_SRC_FILES := CameraPoseEngine.cpp
LOCAL_WHOLE_STATIC_LIBRARIES := libarxengine_base
LOCAL_STATIC_LIBRARIES := libarti_ARM libvlib_ARM
LOCAL_SHARED_LIBRARIES := libarxbuf libdvp libutils libcutils libbinder \
                          libui libgui libhardware libdl libOMX_Core
ifeq ($(TARGET_ANDROID_VERSION),JELLYBEANMR1)
    LOCAL_SHARED_LIBRARIES += libion_ti
else
    LOCAL_SHARED_LIBRARIES += libion
endif
LOCAL_MODULE := libarxpose
include $(BUILD_SHARED_LIBRARY)
endif
