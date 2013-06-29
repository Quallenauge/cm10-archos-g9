# Copyright (C) 2012 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
ifdef NDK_ROOT
LOCAL_C_INCLUDES := $(TIARXSDK_PATH)/include
LOCAL_LDLIBS := -L$(TIARXSDK_PATH)/lib -larx -llog -ljnigraphics
else
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := $(ARX_INC) $(JNI_H_INCLUDE) $(DVP_INC)
LOCAL_CPPFLAGS := $(ARX_DEBUGGING) $(ARX_CPPFLAGS)
LOCAL_SHARED_LIBRARIES := libarx liblog libjnigraphics
endif
LOCAL_SRC_FILES := arxstreams_jni.cpp ARXStreamsJniContext.cpp 
LOCAL_MODULE    := libarxstreams_jni
include $(BUILD_SHARED_LIBRARY)
