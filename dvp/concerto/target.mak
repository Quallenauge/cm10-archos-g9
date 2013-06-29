    # Copyright (C) 2011 Texas Insruments, Inc.
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

TARGET_PLATFORM ?= PC

ifdef QCONF_OVERRIDE
    include $(QCONF_OVERRIDE)
endif
DVP_ROOT ?= $(HOST_ROOT)
SOSAL_ROOT ?= $(HOST_ROOT)

DVP_INC += $(DVP_ROOT)/include
DVP_INT_INC:=$(DVP_ROOT)/source/dvp/dvp_kgraph/include \
             $(DVP_ROOT)/source/dvp/dvp_display/include
SOSAL_INC := $(SOSAL_ROOT)/include

SYSDEFS  :=
SYSIDIRS := $(DVP_INC) $(SOSAL_INC) $(DVP_INT_INC)
SYSLDIRS :=

IPC_INCS :=
IPC_LIBS :=
MEM_INCS :=
MEM_LIBS :=
OMX_INCS :=
OMX_LIBS :=

ifeq ($(TARGET_PLATFORM),PC)
    TARGET_OS=$(HOST_OS)
    TARGET_CPU?=$(HOST_CPU)
    ifeq ($(TARGET_OS),LINUX)
        INSTALL_LIB := /usr/lib
        INSTALL_BIN := /usr/bin
        INSTALL_INC := /usr/include
        TARGET_NUM_CORES:=$(shell cat /proc/cpuinfo | grep processor | wc -l)
        SYSIDIRS += /usr/include
        SYSLDIRS += /usr/lib
        SYSDEFS += SCREEN_DIM_X=1024 SCREEN_DIM_Y=768 \
                   _XOPEN_SOURCE=700 _BSD_SOURCE=1 _GNU_SOURCE=1 DVP_USE_FS \
                   DVP_USE_SHARED_T SOSAL_USE_SHARED_T
        UVC_INC := /usr/include
        GTK_PATH := $(realpath /usr/include/gtk-2.0)
        ifneq ($(GTK_PATH),)
            GTK_INC := $(subst -I,,$(shell pkg-config --cflags-only-I gtk+-2.0))
            GTK_LDIRS := $(shell pkg-config --variable=libdir gtk+-2.0)
            GTK_LIBS := $(subst -l,,$(shell pkg-config --libs-only-l gtk+-2.0))
            SYSDEFS+=DVP_USE_GTK GTK_SUPPORT
        endif
    else ifeq ($(TARGET_OS),DARWIN)
        INSTALL_LIB := /opt/local/lib
        INSTALL_BIN := /opt/local/bin
        INSTALL_INC := /opt/local/include
        TARGET_NUM_CORES ?= 2
        SYSDEFS += SCREEN_DIM_X=1024 SCREEN_DIM_Y=768
        SYSDEFS += _XOPEN_SOURCE=700 _BSD_SOURCE=1 _GNU_SOURCE=1
        SYSDEFS += DVP_STATIC_MANAGERS DVP_USE_FS
    else ifeq ($(TARGET_OS),CYGWIN)
        INSTALL_LIB := /usr/lib
        INSTALL_BIN := /usr/bin
        INSTALL_INC := /usr/include
        TARGET_NUM_CORES ?= 2
        SYSDEFS += SCREEN_DIM_X=1024 SCREEN_DIM_Y=768
        SYSDEFS += DVP_USE_FS DVP_USE_SHARED_T SOSAL_USE_SHARED_T \
                   _XOPEN_SOURCE=700 _BSD_SOURCE=1 _GNU_SOURCE=1 WINVER=0x501
    else ifeq ($(TARGET_OS),Windows_NT)
        INSTALL_LIB := "${windir}\\system32"
        INSTALL_BIN := "${windir}\\system32"
        INSTALL_INC :=
        TARGET_NUM_CORES := $(NUMBER_OF_PROCESSORS)
        SYSIDIRS+=$(DVP_ROOT)/include/win32
        SYSDEFS+=DVP_USE_FS DVP_STATIC_MANAGERS WIN32_LEAN_AND_MEAN WIN32 \
                 _WIN32 _CRT_SECURE_NO_DEPRECATE WINVER=0x0501 \
                 _WIN32_WINNT=0x0501 SOSAL_USE_SHARED_T DVP_USE_SHARED_T
        SYSDEFS+=SCREEN_DIM_X=1024 SCREEN_DIM_Y=768
    endif
else
    ifeq ($(TARGET_PLATFORM),SDP)
        SYSDEFS+=USE_PRIMARY_SENSOR
        TARGET_NUM_CORES := 2
        TARGET_OS := LINUX
        TARGET_CPU := ARM
    else ifeq ($(TARGET_PLATFORM),BLAZE)
        SYSDEFS+=SCREEN_DIM_X=864 SCREEN_DIM_Y=480
        TARGET_NUM_CORES := 2
        TARGET_OS := LINUX
        TARGET_CPU := ARM
    else ifeq ($(TARGET_PLATFORM),PANDA)
        INSTALL_LIB := /usr/lib
        INSTALL_BIN := /usr/bin
        INSTALL_INC := /usr/include
        SYSDEFS+=SCREEN_DIM_X=1920 SCREEN_DIM_Y=1080 # Panda can display 1080p
        SYSDEFS+=USE_PRIMARY_SENSOR
        SYSDEFS+=_XOPEN_SOURCE=700 _BSD_SOURCE=1 _GNU_SOURCE=1
        SYSDEFS+=TARGET_DVP_OMAP4
        SYSDEFS+=DVP_USE_FS DVP_USE_IPC DVP_USE_OMAPRPC DVP_USE_SHARED_T \
                 SOSAL_USE_SHARED_T SOSAL_USE_BO DVP_USE_BO
        UVC_INC := /usr/include
        TARGET_NUM_CORES := 2
        TARGET_OS := LINUX
        TARGET_CPU := ARM
        DUCATI_VERSION := DUCATI_2_0
        ifndef KERNEL_ROOT
            $(error "You must defined the path to the kernel in KERNEL_ROOT!")
        else
            KERNEL_INC := $(KERNEL_ROOT)/include
            SYSIDIRS += $(KERNEL_INC)
        endif
        ifneq ($(HOST_CPU),$(TARGET_CPU))
            ifndef CROSS_COMPILE
                $(error "CROSS_COMPILE not set! You won't be able to compile this!")
            endif
        endif
        IPC_INCS := $(DVP_ROOT)/include/linux
        IPC_LIBS := omaprpc
        ifneq ($(findstring SOSAL_USE_BO,$(SYSDEFS)),)
            # remove the -I and -l switches which assume GCC
            MEM_INCS += $(subst -I,,$(shell pkg-config --cflags libdrm libdrm_omap))
            MEM_LIBS += $(subst -l,,$(shell pkg-config --libs libdrm libdrm_omap))
        endif
        ifneq ($(findstring SOSAL_USE_ION,$(SYSDEFS)),)
            MEM_INCS += $(DVP_ROOT)/include/ion
            MEM_LIBS += ion
        endif
        ifdef OMX_ROOT
            OMX_INCS := $(OMX_ROOT)/include
            SYSLDIRS += $(OMX_ROOT)/omx_core $(OMX_ROOT)/mm_osal
            OMX_LIBS := omx_core mm_osal
        endif
    else ifeq ($(TARGET_PLATFORM),PLAYBOOK)
        HOST_COMPILER := QCC
        TARGET_OS := __QNX__
        TARGET_CPU := ARM
        TARGET_NUM_CORES := 2
        DUCATI_VERSION=DUCATI_1_2
        CROSS_COMPILE := ntoarmv7-
        SYSDEFS += DVP_USE_FS TARGET_OMAP4 DVP_STATIC_MANAGERS \
                SCREEN_DIM_X=1080 SCREEN_DIM_Y=720 __QNXNTO__ \
                _POSIX_C_SOURCE=200112L _GNU_SOURCE=1 \
                SOSAL_USE_SHARED_T DVP_USE_SHARED_T
        ifeq ($(USE_INSTALL_ROOT),)
            INSTALL_PATH := $(QNX_TARGET)/armle-v7
            SYSIDIRS += $(INSTALL_PATH)/usr/include
            SYSLDIRS += $(INSTALL_PATH)/lib $(INSTALL_PATH)/usr/lib
        else
            INSTALL_PATH := $(INSTALL_ROOT_nto)/armle-v7
            SYSIDIRS += $(INSTALL_ROOT_nto)/usr/include $(QNX_TARGET)/usr/include
            SYSLDIRS += $(INSTALL_PATH)/lib $(INSTALL_PATH)/usr/lib $(QNX_TARGET)/armle-v7/lib $(QNX_TARGET)/armle-v7/usr/lib
        endif
        ifdef IPC_ROOT
            IPC_INCS := $(IPC_ROOT)/
            IPC_INCS += $(IPC_ROOT)/inc/
            IPC_INCS += $(IPC_ROOT)/inc/usr/
            IPC_INCS += $(IPC_ROOT)/utils/
            IPC_LIBS := syslink_client
            SYSDEFS += DVP_USE_IPC DVP_USE_RCM
            SYSIDIRS+= $(IPC_INCS)
        endif
        ifdef TILER_ROOT
            MEM_INCS := $(TILER_ROOT)/
            MEM_INCS += $(TILER_ROOT)/usr/memmgr/
            MEM_INCS += $(TILER_ROOT)/usr/memmgr/public/
            MEM_INCS += $(TILER_ROOT)/usr/memmgr/public/memmgr/
            MEM_LIBS := memmgr
            SYSDEFS += DVP_USE_TILER SOSAL_USE_TILER
            SYSIDIRS+= $(MEM_INCS)
        endif
        ifdef OMX_ROOT
            OMX_INCS := $(OMX_ROOT)/include
            SYSLDIRS += $(OMX_ROOT)/omx_core $(OMX_ROOT)/mm_osal
            OMX_LIBS := OMX_Core mmosal
        endif
    else ifeq ($(TARGET_PLATFORM),CENTEVE)
        HOST_COMPILER := TMS470
        TARGET_OS := SYSBIOS
        TARGET_CPU := ARM
        TARGET_NUM_CORES := 2
        SYSDEFS += TARGET_CENTEVE DVP_STATIC_MANAGERS
        SYSIDIRS += $(TMS470_ROOT)/include $(BIOS_ROOT)/packages $(IPC_ROOT)/packages $(XDC_ROOT)/packages
        SYSLDIRS += $(TMS470_ROOT)/lib
    endif
endif

ifndef DUCATI_VERSION
    DUCATI_VERSION=DUCATI_1_5
endif

SYSDEFS += $(TARGET_OS) $(TARGET_CPU) $(TARGET_PLATFORM) TARGET_NUM_CORES=$(TARGET_NUM_CORES) $(DUCATI_VERSION)

ifeq ($(TARGET_OS),LINUX)
    PLATFORM_LIBS := dl pthread rt
else ifeq ($(TARGET_OS),DARWIN)
    PLATFORM_LIBS :=
else ifeq ($(TARGET_OS),Windows_NT)
    PLATFORM_LIBS := Ws2_32 user32
else ifeq ($(TARGET_OS),__QNX__)
    PLATFORM_LIBS := screen socket
else ifeq ($(TARGET_OS),CYGWIN)
    PLATFORM_LIBS := c pthread
endif

ifeq ($(TARGET_CPU),X86)
    SYSDEFS+=DVP_TARGET_X86
    TARGET_ARCH=32
else ifeq ($(TARGET_CPU),X64)
    SYSDEFS+=DVP_TARGET_X86
    TARGET_ARCH=64
else ifeq ($(TARGET_CPU),x86_64)
    SYSDEFS+=DVP_TARGET_X86
    TARGET_ARCH=64
else ifeq ($(TARGET_CPU),ARM)
    SYSDEFS+=DVP_TARGET_ARM
    TARGET_ARCH=32
endif

TARGET_ARCH?=32

SYSDEFS+=ARCH_$(TARGET_ARCH)

ifeq ($(BUILD_DEBUG),1)
$(info SYSDEFS = $(SYSDEFS))
$(info SYSIDIRS = $(SYSIDIRS))
$(info SYSLDIRS = $(SYSLDIRS))
endif

ifdef DVP_DEBUG
SYSDEFS+=DVP_DEBUG=$(DVP_DEBUG)
ifdef DVP_ZONE_MASK
SYSDEFS+=DVP_ZONE_MASK=$(DVP_ZONE_MASK)
endif
endif

ifdef SOSAL_DEBUG
SYSDEFS+=SOSAL_DEBUG=$(SOSAL_DEBUG)
ifdef SOSAL_ZONE_MASK
SYSDEFS+=SOSAL_ZONE_MASK=$(SOSAL_ZONE_MASK)
endif
endif

ifdef OMAPRPC_DEBUG
SYSDEFS+=OMAPRPC_DEBUG=$(OMAPRPC_DEBUG)
ifdef OMAPRPC_ZONE_MASK
SYSDEFS+=OMAPRPC_ZONE_MASK=$(OMAPRPC_ZONE_MASK)
endif
endif

DVP_FEATURES += DVP_USE_IMAGE

# Combinations
# TARGET_CPU=ARM && TARGET_PLATFORM=PLAYBOOK && TARGET_OS=QNX
# TARGET_CPU=X86 && TARGET_PLATFORM=PC
# TARGET_CPU=ARM && TARGET_PLATFORM=SDP
# TARGET_CPU=ARM && TARGET_PLATFORM=BLAZE
