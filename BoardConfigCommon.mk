#
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
ifeq ($(TARGET_DEVICE),archos_g9)
# Common Makefile to be included at the bottom of each device's
# BoardConfig.mk file. Definitions here are supposed to be device independent.
COMMON_FOLDER := device/archos/archos_g9

# set to allow building from omap4-common
BOARD_VENDOR := archos

PRODUCT_VENDOR_KERNEL_HEADERS := $(COMMON_FOLDER)/kernel-headers

TARGET_SPECIFIC_HEADER_PATH := $(COMMON_FOLDER)/include

# Makefile variable and C/C++ macro to recognise DOMX version
ifdef ENHANCED_DOMX

    DOMX_PATH := hardware/ti/domx
else
    DOMX_PATH := hardware/ti/omap4xxx/domx
endif
    COMMON_GLOBAL_CFLAGS += -DENHANCED_DOMX
    COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT -DTARGET_OMAP4
    COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT_BURST_CAPTURE
    COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT_S3D
    COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT_CPCAM
    COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT_VTC
    COMMON_GLOBAL_CFLAGS += -DUSE_ITTIAM_AAC
    COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT_MULTIGPU

endif
