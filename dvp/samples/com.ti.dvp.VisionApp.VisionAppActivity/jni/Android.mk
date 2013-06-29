# Copyright (C) 2012 Texas Instruments Inc.
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

#######################################

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(LOCAL_PATH) \
					$(LOCAL_PATH)/../../../include
LOCAL_MODULE     := libdvp_jni
LOCAL_SRC_FILES  := libdvp_jni.cpp transformers.cpp
LOCAL_CFLAGS     := -DARM -DARCH_32 -DANDROID -DLINUX \
					-DDVP_USE_IPC \
					-DDVP_USE_ION -DDVP_USE_GRALLOC \
					-DSOSAL_USE_ION -DSOSAL_USE_GRALLOC \
					-DDVP_USE_SHARED_T -DDVP_USE_FS -DDVP_USE_CAMERA_SERVICE
LOCAL_LDFLAGS += -L$(LOCAL_PATH)/lib/armeabi
LOCAL_LDLIBS := -llog -landroid -lsosal -lvcam -ldvp -lcthreaded -lOMX_Core -lbinder -lcamera_client -lui -lutils -lcutils -lgui 
include $(BUILD_SHARED_LIBRARY)

######################################

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(LOCAL_PATH) \
					$(LOCAL_PATH)/../../../libraries/public/imgfilter/include
LOCAL_MODULE     := libimagefilter
LOCAL_SRC_FILES  := libimagefilter.c
#-DDVP_USE_CAMERA_SERVICE
LOCAL_LDFLAGS += -L$(LOCAL_PATH)/lib/armeabi
LOCAL_LDLIBS := -llog -landroid -limgfilter
include $(BUILD_SHARED_LIBRARY)
