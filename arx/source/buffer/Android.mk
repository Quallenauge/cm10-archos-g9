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
LOCAL_CPPFLAGS := $(ARX_DEBUGGING) $(ARX_CPPFLAGS) $(DVP_FEATURES)
LOCAL_SRC_FILES := IBufferMgrClient.cpp IFlatBufferMgr.cpp IImageBufferMgr.cpp BufferMgr.cpp \
                   FlatBufferMgr.cpp ImageBufferMgr.cpp Buffer.cpp FlatBuffer.cpp ImageBuffer.cpp
LOCAL_C_INCLUDES := $(ARX_INC) $(DVP_INC)
LOCAL_SHARED_LIBRARIES := libdvp libbinder libutils libcutils libgui libui libhardware
ifeq ($(TARGET_ANDROID_VERSION),JELLYBEANMR1)
    LOCAL_SHARED_LIBRARIES += libion_ti
else
    LOCAL_SHARED_LIBRARIES += libion
endif
LOCAL_MODULE := libarxbuf
include $(BUILD_SHARED_LIBRARY)