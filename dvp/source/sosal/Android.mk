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
LOCAL_CFLAGS := $(SOSAL_DEBUGGING) $(SOSAL_CFLAGS) -DPOOL_MAX_SIZE=0
LOCAL_SRC_FILES := $(filter-out shared.c unittest.c,$(call all-c-files-under,.))
LOCAL_C_INCLUDES := $(SOSAL_TOP)/include
ifeq ($(PLAT_MAJOR),4) # ICS or later
	LOCAL_C_INCLUDES += hardware/ti/omap4xxx
else
	LOCAL_C_INCLUDES += $(TILER_INC)
endif
LOCAL_MODULE := libsosal
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CPPFLAGS := $(SOSAL_DEBUGGING) $(SOSAL_CPPFLAGS) -DSHARED_MEMORY_CLIENT
LOCAL_SRC_FILES := shared.cpp shared_binder.cpp
LOCAL_C_INCLUDES := $(SOSAL_TOP)/include
LOCAL_MODULE := libsosal_shm
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CPPFLAGS := $(SOSAL_DEBUGGING) $(SOSAL_CPPFLAGS)
LOCAL_SRC_FILES := allocator_binder.cpp
LOCAL_C_INCLUDES := $(SOSAL_TOP)/include
LOCAL_MODULE := liballoc_binder
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := tests
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := $(SOSAL_DEBUGGING) $(SOSAL_CFLAGS) -DSOSAL_UNITTEST
LOCAL_SRC_FILES := unittest.c
LOCAL_C_INCLUDES := $(SOSAL_TOP)/include
LOCAL_MODULE := sosal_test
LOCAL_STATIC_LIBRARIES := libsosal libsosal_shm
LOCAL_SHARED_LIBRARIES := libdl libutils libui libcutils libbinder
ifeq ($(PLAT_MAJOR),4) # ICS or later
	LOCAL_SHARED_LIBRARIES += libion libhardware
	LOCAL_LDLIBS += -lhwcomposer.omap4
else
	LOCAL_SHARED_LIBRARIES += $(TILER_LIB)
endif
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := tests
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := $(SOSAL_DEBUGGING) $(SOSAL_CFLAGS) -DUINPUT_TEST
LOCAL_SRC_FILES := event.c mutex.c options.c queue.c ring.c thread.c uinput.c debug.c
LOCAL_C_INCLUDES := $(SOSAL_TOP)/include
LOCAL_MODULE := uinput_test
LOCAL_SHARED_LIBRARIES := libcutils
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CPPFLAGS := $(SOSAL_DEBUGGING) $(SOSAL_CPPFLAGS) -DSHARED_MEMORY_SERVICE
LOCAL_SRC_FILES := shared_binder.cpp debug.c
LOCAL_C_INCLUDES := $(SOSAL_TOP)/include
LOCAL_MODULE := shm_service
LOCAL_STATIC_LIBRARIES := libsosal
LOCAL_SHARED_LIBRARIES := libdl libcutils libutils libbinder
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CPPFLAGS := $(SOSAL_DEBUGGING) $(SOSAL_CPPFLAGS)
LOCAL_SRC_FILES := CThreaded.cpp
LOCAL_C_INCLUDES += $(SOSAL_TOP)/include
LOCAL_MODULE := libcthreaded
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := $(SOSAL_DEBUGGING) $(SOSAL_CFLAGS) -DMODULE_TEST
LOCAL_SRC_FILES := module.c debug.c
LOCAL_C_INCLUDES := $(SOSAL_TOP)/include
LOCAL_MODULE := modload_test
LOCAL_SHARED_LIBRARIES := libdl libcutils
include $(BUILD_EXECUTABLE)
