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
LOCAL_SRC_FILES := VisionCamFactory.cpp \
					CameraEmulator.cpp \
					FileVisionCam.cpp \
					OMXVisionCam.cpp \
					SocketVisionCam.cpp \
					VisionCamFrame.cpp \
					VisionCamUtils.cpp
ifdef EXPORTED_3A
LOCAL_SRC_FILES += OMXVisionCam_3A_Export.cpp
endif

ifeq ($(DVP_USES_CAMERA_SERVICE),true)
LOCAL_SRC_FILES += CSVisionCam.cpp
endif

LOCAL_C_INCLUDES += $(DVP_INCLUDES)
LOCAL_CPPFLAGS := $(DVP_DEBUGGING) $(DVP_CPPFLAGS) -DVCAM_USE_OMX
LOCAL_MODULE := libvcam
include $(BUILD_STATIC_LIBRARY)

# An Interactive Unit Test for VisionCam
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := tests
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CPPFLAGS := $(DVP_DEBUGGING) $(DVP_CPPFLAGS)
ifeq ($(TARGET_PLATFORM), SDP)
	LOCAL_CPPFLAGS+=-DSDP
endif

LOCAL_SRC_FILES := VisionCamTest.cpp
ifdef EXPORTED_3A
LOCAL_SRC_FILES += VisionCamTest_3A_Export.cpp
endif

LOCAL_C_INCLUDES +=$(DVP_INCLUDES) \
                   frameworks/base/include/utils \
                   $(TI_HW_ROOT)/omap4xxx/libtiutils
LOCAL_SHARED_LIBRARIES:= libdl libbinder $(DVP_SYSLIBS) libOMX_Core libdvp libcutils libutils libdvp libcamera_client libgui
LOCAL_STATIC_LIBRARIES := libvcam libsosal libcthreaded
LOCAL_MODULE := vcam_test
include $(BUILD_EXECUTABLE)

# A simple command line VisionCam Test
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := tests
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CPPFLAGS := $(DVP_DEBUGGING) $(DVP_CPPFLAGS)
ifeq ($(TARGET_PLATFORM), SDP)
	LOCAL_CPPFLAGS+=-DSDP
endif
LOCAL_SRC_FILES := VisionCamSimpleTest.cpp
LOCAL_C_INCLUDES += $(DVP_INCLUDES)
LOCAL_SHARED_LIBRARIES:= libdl $(DVP_SYSLIBS) libOMX_Core libcutils libutils libdvp libbinder libcamera_client libgui
LOCAL_STATIC_LIBRARIES := libvcam libsosal libimgdbg libcthreaded
LOCAL_MODULE := vcam_simple
include $(BUILD_EXECUTABLE)

# A Socket Server of VisionCam
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := tests
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CPPFLAGS := $(DVP_DEBUGGING) $(DVP_CPPFLAGS) $(DVP_FEATURES)
LOCAL_SRC_FILES := ServerVisionCam.cpp VisionCamServer.cpp VisionCamFrame.cpp
LOCAL_C_INCLUDES += $(DVP_INCLUDES)
LOCAL_SHARED_LIBRARIES:= libdl $(DVP_SYSLIBS) libOMX_Core libcutils libutils libdvp libbinder libcamera_client libgui
LOCAL_STATIC_LIBRARIES := libvcam libsosal libcthreaded
LOCAL_MODULE := vcam_server
include $(BUILD_EXECUTABLE)

