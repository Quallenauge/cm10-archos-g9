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
LOCAL_MODULE_TAGS := tests
LOCAL_C_INCLUDES := $(ARX_INC) $(DVP_INC)
LOCAL_CPPFLAGS := $(ARX_DEBUGGING) $(ARX_CPPFLAGS) -DDVP_USE_FS
LOCAL_SRC_FILES := DisplaySurface.cpp ARXImgRenderer.cpp FDRenderer.cpp TestPropListener.cpp ARXClientTest.cpp DVPTestGraph.cpp
LOCAL_MODULE := libarxtest
include $(BUILD_STATIC_LIBRARY)

# Unit Test for ARX
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := tests
LOCAL_C_INCLUDES := $(ARX_INC) $(DVP_INC)
LOCAL_CPPFLAGS := $(ARX_DEBUGGING) $(ARX_CPPFLAGS)
LOCAL_SRC_FILES := arx_test.cpp
LOCAL_SHARED_LIBRARIES := libarx libdvp libbinder libutils libcutils libgui libui
LOCAL_STATIC_LIBRARIES := libarxtest libimgdbg
LOCAL_MODULE := arx_test
include $(BUILD_EXECUTABLE)
