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

ifeq ($(BOARD_USES_DVP),true)

TI_HW_ROOT ?= hardware/ti
PLAT_NUMBERS := $(subst ., ,$(PLATFORM_VERSION))
PLAT_MAJOR := $(word 1,$(PLAT_NUMBERS))
PLAT_MINOR := $(word 2,$(PLAT_NUMBERS))
ifeq ($(PLAT_MAJOR),2)
    ifeq ($(PLAT_MINOR),2)
        TARGET_ANDROID_VERSION := FROYO
    else ifeq ($(PLAT_MINOR),3)
        TARGET_ANDROID_VERSION := GINGERBREAD
    endif
else ifeq ($(PLAT_MAJOR),3)
    TARGET_ANDROID_VERSION := HONEYCOMB
else ifeq ($(PLAT_MAJOR),4)
    ifeq ($(PLAT_MINOR),0)
        TARGET_ANDROID_VERSION := ICS
    else ifeq  ($(PLAT_MINOR),1)
        TARGET_ANDROID_VERSION := JELLYBEAN
    endif
endif

ifneq ($(DVP_DEBUG),)
$(info Android Version $(TARGET_ANDROID_VERSION))
endif

DVP_TOP := $(call my-dir)
DVP_ROOT := $(DVP_TOP)
SOSAL_TOP := $(call my-dir)

COMMIT_ID := $(shell cd $(DVP_TOP) && git describe --tags --dirty)
ifneq ($(DVP_DEBUG),)
$(info COMMIT_ID = $(COMMIT_ID))
endif

ifeq ($(TARGET_ANDROID_VERSION), FROYO)
    IPC_TYPE := syslink
else ifeq ($(TARGET_ANDROID_VERSION), GINGERBREAD)
    IPC_TYPE := syslink
else ifeq ($(TARGET_ANDROID_VERSION), ICS)
    IPC_TYPE := rpmsg
else ifeq ($(TARGET_ANDROID_VERSION), JELLYBEAN)
    IPC_TYPE := rpmsg
endif

ifneq ($(DVP_DEBUG),)
$(info IPC_TYPE is $(IPC_TYPE))
endif

ifeq ($(IPC_TYPE),syslink)
    RCM_INC := $(TI_HW_ROOT)/syslink/syslink/api/include
    IPC_INC := $(TI_HW_ROOT)/syslink/syslink/api/include
    OMX_INC := $(TI_HW_ROOT)/omx/ducati/domx/system/omx_core/inc
    OLD_MEMMGR := $(realpath $(TI_HW_ROOT)/tiler/memmgr )
    ifeq ($(OLD_MEMMGR),)
        TILER_INC := $(TI_HW_ROOT)/tiler
        TILER_LIB := libtimemmgr
    else
        TILER_INC := $(TI_HW_ROOT)/tiler/memmgr
        TILER_LIB := libmemmgr
    endif
    DVP_SYSINCS := $(RCM_INC) $(IPC_INC) $(TILER_INC) $(OMX_INC)
    DVP_SYSLIBS := librcm libsysmgr libipc $(TILER_LIB)
    ifeq ($(TARGET_ANDROID_VERSION),HONEYCOMB)
        DUCATI_VERSION := DUCATI_1_5
    endif
else ifeq ($(IPC_TYPE), rpmsg)
    ifeq ($(TARGET_PLATFORM),TUNA)
        OMX_INC := $(TI_HW_ROOT)/omap4xxx/domx/omx_core/inc
    else
        OMX_INC := $(TI_HW_ROOT)/domx/omx_core/inc
    endif
    DVP_SYSINCS := $(OMX_INC) \
                   $(TI_HW_ROOT)/omap4xxx/ \
                   hardware/libhardware/include
    DVP_SYSLIBS := libion
    DUCATI_VERSION := DUCATI_2_0
endif

DVP_INCLUDES := $(SOSAL_TOP)/include \
                $(DVP_TOP)/include \
                $(DVP_TOP)/source/dvp/dvp_kgraph/include \
                $(DVP_TOP)/source/dvp/dvp_display/include \
                $(DVP_SYSINCS)

TARGET_CPU ?= ARM

TARGET_PLATFORM ?= BLAZE
_T := $(TARGET_PLATFORM)
# Make sure to remove illegal C macro characters like '-'
TARGET_PLATFORM := $(subst -,_,$(_T))

ifeq ($(TARGET_BOARD_PLATFORM),omap4)
   TARGET_NUM_CORES := 2
else ifeq ($(TARGET_BOARD_PLATFORM),omap5)
   TARGET_NUM_CORES := 2
else
   TARGET_NUM_CORES := 2
endif

# convert the product to a platform
ifeq ($(TARGET_PRODUCT),blaze_tablet)
   TARGET_PLATFORM := BLAZE_TABLET
else ifeq ($(TARGET_PRODUCT),full_blaze)
   TARGET_PLATFORM := BLAZE
else ifeq ($(TARGET_PRODUCT),full_panda)
   TARGET_PLATFORM := PANDA
else ifeq ($(TARGET_PRODUCT),full_maguro)
   TARGET_PLATFORM := TUNA
else ifeq ($(TARGET_PRODUCT),full_toro)
   TARGET_PLATFORM := TUNA
else ifeq ($(TARGET_PRODUCT),full_torospr)
   TARGET_PLATFORM := TUNA
else ifeq ($(TARGET_PRODUCT),full_tuna)
   TARGET_PLATFORM := TUNA
else ifeq ($(TARGET_PRODUCT),full_omap5sevm)
   TARGET_PLATFORM := OMAP5SEVM
endif

ifeq ($(TARGET_PLATFORM),BLAZE) # OMAP4 Blaze Developer Board
    SCREEN_DIM_X=864
    SCREEN_DIM_Y=480
else ifeq ($(TARGET_PLATFORM),OMAP5SEVM) # OMAP5 SEVM Board
    SCREEN_DIM_X=720
    SCREEN_DIM_Y=1280
else ifeq ($(TARGET_PLATFORM),BLAZE_TABLET) # OMAP4 Blaze Developer Tablet 1 or 2
    SCREEN_DIM_X=1280
    SCREEN_DIM_Y=720
else ifeq ($(TARGET_PLATFORM),PANDA) # OMAP4 Panda (HDMI Display)
    SCREEN_DIM_X=1280
    SCREEN_DIM_Y=720
else ifeq ($(TARGET_PLATFORM),SDP) # OMAP SDP Dev Board
    SCREEN_DIM_X=864
    SCREEN_DIM_Y=480
else ifeq ($(TARGET_PLATFORM),p920) # LG Optimus 3D
    SCREEN_DIM_X=480
    SCREEN_DIM_Y=800
    DUCATI_VERSION=DUCATI_1_2
else ifeq ($(TARGET_PLATFORM),p925) # LG Optimus 3D
    SCREEN_DIM_X=480
    SCREEN_DIM_Y=800
    DUCATI_VERSION=DUCATI_1_2
else ifeq ($(TARGET_PLATFORM),GT_I9100G) # Samsung Galaxy S II (OMAP variant)
    SCREEN_DIM_X=480
    SCREEN_DIM_Y=800
    DUCATI_VERSION=DUCATI_1_2
else ifeq ($(TARGET_PLATFORM),TUNA) # Samsung Galaxy Nexus ; Samsung Proxima
    SCREEN_DIM_X=720
    SCREEN_DIM_Y=1280
    DUCATI_VERSION=DUCATI_2_0
else ifeq ($(TARGET_PLATFORM),BLADE) # Toshiba AT200
    SCREEN_DIM_X=1280
    SCREEN_DIM_Y=800
    DUCATI_VERSION=DUCATI_2_0
else
$(error "Unknown TARGET_PLATFORM!")
endif

# Default to 2.0 if it's not set
DUCATI_VERSION ?= DUCATI_2_0

LOCAL_FLAGS := -pipe -Wall -Wno-trigraphs -fno-short-enums -Wno-psabi \
               -fno-strict-aliasing -mapcs -mno-sched-prolog -mabi=aapcs-linux \
               -mno-thumb-interwork -fno-common -fpic -Wno-write-strings \
               -DLINUX -DARCH_32 -D$(BUILD_ID) -DCOMMIT_ID="\"$(COMMIT_ID)\"" \
               -D$(TARGET_PLATFORM) -D$(TARGET_ANDROID_VERSION) -D$(TARGET_CPU) \
               -DSCREEN_DIM_X=$(SCREEN_DIM_X) -DSCREEN_DIM_Y=$(SCREEN_DIM_Y) \
               -DTARGET_NUM_CORES=$(TARGET_NUM_CORES)

ifeq ($(PLATFORM_VERSION_CODENAME),AOSP)
LOCAL_FLAGS += -D$(PLATFORM_VERSION_CODENAME)
endif

ifeq ($(TARGET_BOARD_PLATFORM),omap4)
LOCAL_FLAGS += -DTARGET_DVP_OMAP4
endif
ifeq ($(TARGET_BOARD_PLATFORM),omap5)
LOCAL_FLAGS += -DTARGET_DVP_OMAP5
endif
ifeq ($(TARGET_BOARD_PLATFORM),omap6)
LOCAL_FLAGS += -DTARGET_DVP_OMAP6
endif

DVP_FLAGS := -DDVP_TARGET_ARM -D$(DUCATI_VERSION) -DDVP_FLICKER=60

ifeq ($(IPC_TYPE),syslink)

DVP_FLAGS += -DDVP_USE_IPC -DDVP_USE_RCM \
             -DDVP_USE_TILER -DSOSAL_USE_TILER \
             -DDVP_USE_SHARED_T -DSOSAL_USE_SHARED_T

SOSAL_FLAGS := -DSOSAL_USE_TILER -DSOSAL_USE_SHARED_T

else ifeq ($(IPC_TYPE),rpmsg)

DVP_FLAGS += -DDVP_USE_IPC -DDVP_USE_OMAPRPC -DOMAPRPC_USE_ION \
             -DDVP_USE_ION -DSOSAL_USE_ION \
             -DDVP_USE_GRALLOC -DSOSAL_USE_GRALLOC \
             -DDVP_USE_SHARED_T -DSOSAL_USE_SHARED_T

SOSAL_FLAGS := -DSOSAL_USE_ION -DSOSAL_USE_ION_TILER -DSOSAL_USE_GRALLOC -DSOSAL_USE_SHARED_T

EXPORTED_3A := $(strip $(EXPORTED_3A))

ifdef EXPORTED_3A
$(info 3A Export Supported!)
DVP_FLAGS += -DEXPORTED_3A
endif

ifdef CPCAM
$(info CPCAM Build Used!)
DVP_FLAGS += -DCPCAM
endif

ifeq ($(TARGET_ANDROID_VERSION), JELLYBEAN)
DVP_USE_OMX_EXTENSIONS ?= true
else
DVP_USE_OMX_EXTENSIONS ?= false
endif

ifeq ($(DVP_USE_OMX_EXTENSIONS),true)
$(info OMX Extensions used!)
DVP_FLAGS += -DCPCAM -DOMX_CAMERA_SUPPORTS_FD_RAW -DOMX_CAMERA_SUPPORTS_IMAGE_PYRAMID
endif

endif # rpmsg

# Android always has FS usage turned on
DVP_FLAGS += -DDVP_USE_FS

# Enable image functions in the sosal
DVP_FEATURES += -DDVP_USE_IMAGE

# in the future this may be conditionally defined.
DVP_USES_CAMERA_SERVICE := false

ifeq ($(DVP_USES_CAMERA_SERVICE),true)
DVP_FLAGS += -DDVP_USE_CAMERA_SERVICE
endif

DVP_CFLAGS := $(DVP_FLAGS) $(LOCAL_FLAGS) -Werror-implicit-function-declaration
DVP_CPPFLAGS := $(DVP_FLAGS) $(LOCAL_FLAGS) -fuse-cxa-atexit

SOSAL_CFLAGS := $(SOSAL_FLAGS) $(LOCAL_FLAGS) -Werror-implicit-function-declaration
SOSAL_CPPFLAGS := $(SOSAL_FLAGS) $(LOCAL_FLAGS) -fuse-cxa-atexit

ifdef DVP_NO_OPTIMIZE
DVP_CFLAGS += -ggdb -O0
DVP_CPPFLAGS += -ggdb -O0
else
DVP_CFLAGS += -O2 -fomit-frame-pointer
DVP_CPPFLAGS += -O2 -fomit-frame-pointer
endif

ifdef SOSAL_NO_OPTIMIZE
SOSAL_CFLAGS += -ggdb -O0
SOSAL_CPPFLAGS += -ggdb -O0
else
SOSAL_CFLAGS += -O2 -fomit-frame-pointer
SOSAL_CPPFLAGS += -O2 -fomit-frame-pointer
endif

ifdef DVP_DEBUG
DVP_DEBUGGING := -DDVP_DEBUG=$(DVP_DEBUG)
ifdef DVP_ZONE_MASK
DVP_DEBUGGING += -DDVP_ZONE_MASK=$(DVP_ZONE_MASK)
endif
endif

ifdef SOSAL_DEBUG
SOSAL_DEBUGGING := -DSOSAL_DEBUG=$(SOSAL_DEBUG)
ifdef SOSAL_ZONE_MASK
SOSAL_DEBUGGING += -DSOSAL_ZONE_MASK=$(SOSAL_ZONE_MASK)
endif
endif

ifdef OMAPRPC_DEBUG
OMAPRPC_DEBUGGING := -DOMAPRPC_DEBUG=$(OMAPRPC_DEBUG)
ifdef OMAPRPC_ZONE_MASK
OMAPRPC_DEBUGGING += -DOMAPRPC_ZONE_MASK=$(OMAPRPC_ZONE_MASK)
endif
endif

DVP_LIBRARIES :=
DVP_LOCAL_BUILD := true
# Each library which is present must enable itself in DVP via the DVP_LIBRARIES variable
include $(DVP_TOP)/libraries/Android.mk
include $(DVP_TOP)/source/Android.mk
DVP_LOCAL_BUILD :=
DVP_TOP :=
DVP_INC :=
SOSAL_TOP :=
SOSAL_INC :=
endif
