#
# Copyright (C) 2011 The Android Open-Source Project
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

LOCAL_PATH := device/archos/archos_g9

$(warning "LocalPath: $(LOCAL_PATH)")

GLOBAL_C_INCLUDES += \
    external/bltsville/bltsville/include

# define OMAP_ENHANCEMENT variables
include device/archos/archos_g9/Config.mk


# These two variables are set first, so they can be overridden
# by BoardConfigVendor.mk
BOARD_USES_GENERIC_AUDIO := true
USE_CAMERA_STUB := true
OMAP_ENHANCEMENT := true
ENHANCED_DOMX := true

ifdef OMAP_ENHANCEMENT
    OMAP_ENHANCEMENT_S3D :=
    OMAP_ENHANCEMENT_CPCAM :=
    OMAP_ENHANCEMENT_VTC := true
endif

# Use the non-open-source parts, if they're present
-include $(VENDOR_PATH)/BoardConfigVendor.mk

TARGET_CPU_ABI             := armeabi-v7a
TARGET_CPU_ABI2            := armeabi
TARGET_CPU_SMP             := true
TARGET_ARCH_VARIANT        := armv7-a-neon
ARCH_ARM_HAVE_TLS_REGISTER := true

BOARD_HAVE_BLUETOOTH := true
TARGET_NO_BOOTLOADER := true

TARGET_PREBUILT_KERNEL := $(LOCAL_PATH)/kernel
BOARD_KERNEL_BASE      := 0x80000000
#BOARD_KERNEL_CMDLINE  :=

TARGET_NO_RADIOIMAGE         := true
TARGET_BOARD_PLATFORM        := omap4
TARGET_BOOTLOADER_BOARD_NAME := archos

# Recovery
#TARGET_RECOVERY_PIXEL_FORMAT   := "BGRA_8888"
#TARGET_RECOVERY_UI_LIB         := librecovery_ui_panda
# device-specific extensions to the updater binary
#TARGET_RELEASETOOLS_EXTENSIONS := device/ti/panda

BOARD_EGL_CFG := $(PREBUILT_PATH)/lib/egl/egl.cfg

#BOARD_USES_HGL            := true
#BOARD_USES_OVERLAY        := true
USE_OPENGL_RENDERER        := true
#BOARD_USES_PANDA_GRAPHICS := true

# OMAP
BLTSVILLE_ENHANCEMENT :=true
OMAP_ENHANCEMENT     := true
COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT -DTARGET_OMAP4

# Audio library
COMMON_GLOBAL_CFLAGS += -DICS_AUDIO_BLOB

#ENABLE_WEBGL := true

TARGET_USERIMAGES_USE_EXT4         := true
BOARD_SYSTEMIMAGE_PARTITION_SIZE   := 268435456
#BOARD_SYSTEMIMAGE_PARTITION_SIZE  := 16777216
BOARD_USERDATAIMAGE_PARTITION_SIZE := 536870912
BOARD_CACHEIMAGE_PARTITION_SIZE    := 268435456
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE  := ext4
BOARD_FLASH_BLOCK_SIZE             := 4096

# Connectivity - Wi-Fi
BOARD_WPA_SUPPLICANT_DRIVER      := NL80211
WPA_SUPPLICANT_VERSION           := VER_0_8_X_TI
BOARD_HOSTAPD_DRIVER      	 := NL80211
BOARD_WLAN_DEVICE                := wl12xx_mac80211
BOARD_SOFTAP_DEVICE              := wl12xx_mac80211
WIFI_DRIVER_MODULE_PATH          := "/lib/modules/wl12xx_sdio.ko"
WIFI_DRIVER_MODULE_NAME          := "wl12xx_sdio"
WIFI_FIRMWARE_LOADER             := ""
COMMON_GLOBAL_CFLAGS             += -DUSES_TI_MAC80211

#BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_wl12xx

ifdef OMAP_ENHANCEMENT
COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT -DTARGET_OMAP4
endif

#TARGET_PROVIDES_INIT_RC := true
#TARGET_USERIMAGES_SPARSE_EXT_DISABLED := true

# Boot animation
TARGET_BOOTANIMATION_USE_RGB565 := true

# Common device independent definitions
include device/ti/common-open/BoardConfig.mk
