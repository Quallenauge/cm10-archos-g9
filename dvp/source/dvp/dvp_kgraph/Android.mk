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
LOCAL_CFLAGS := -DDVP_KGAPI_THREADED $(DVP_DEBUGGING) $(DVP_CFLAGS) $(DVP_FEATURES)
LOCAL_SRC_FILES := dvp_kgb.c dvp_kmdl.c dvp_kgraph.c dvp_mem.c dvp_mem_int.c
ifeq ($(TARGET_ANDROID_VERSION),GINGERBREAD)
LOCAL_CFLAGS += -DV4L2_SUPPORT
LOCAL_SRC_FILES += dvp_display_v4l2.c dvp_rpc_rcm.c
else ifeq ($(PLAT_MAJOR),4)
LOCAL_CFLAGS += -DSURFACE_FLINGER -DDVP_CONTINUE_ON_ERRORS
LOCAL_SRC_FILES += dvp_display_sf.c dvp_rpc_omaprpc.c
endif

LOCAL_C_INCLUDES := $(DVP_INCLUDES)
LOCAL_WHOLE_STATIC_LIBRARIES := libv4l2 libsosal libsosal_shm
LOCAL_SHARED_LIBRARIES := libdl $(DVP_SYSLIBS) libutils libcutils libbinder
ifeq ($(PLAT_MAJOR),4) # ICS/JB
LOCAL_WHOLE_STATIC_LIBRARIES += libgfxdisp libsf
LOCAL_SHARED_LIBRARIES += libhardware libion libgui libui
LOCAL_LDLIBS += -lhwcomposer.omap4
endif
ifeq ($(IPC_TYPE),rpmsg)
LOCAL_WHOLE_STATIC_LIBRARIES += libomaprpc
endif
LOCAL_MODULE := libdvp
include $(BUILD_SHARED_LIBRARY)
