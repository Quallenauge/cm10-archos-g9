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
LOCAL_SRC_FILES := IARXClient.cpp
LOCAL_C_INCLUDES := $(ARX_INC) $(DVP_INC)
LOCAL_CPPFLAGS := $(ARX_DEBUGGING) $(ARX_CPPFLAGS)
LOCAL_MODULE := libarxclient_ipc
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := $(ARX_INC) $(DVP_INC)
LOCAL_CPPFLAGS := $(ARX_DEBUGGING) $(ARX_CPPFLAGS)
LOCAL_SRC_FILES := ARXFlatBufferImpl.cpp  ARXImageBufferImpl.cpp ARXFlatBufferMgrImpl.cpp ARXImageBufferMgrImpl.cpp ARXImpl.cpp
LOCAL_WHOLE_STATIC_LIBRARIES :=  libarxjni_util
LOCAL_STATIC_LIBRARIES :=  libarxclient_ipc libarxdaemon_ipc
LOCAL_SHARED_LIBRARIES := libarxbuf libdvp libutils libcutils libbinder libgui
LOCAL_MODULE := libarx
include $(BUILD_SHARED_LIBRARY)