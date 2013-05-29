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
ifeq ($(TARGET_DEVICE),archos_g9)
LOCAL_PATH := device/archos/archos_g9

#$(warning "LocalPath: $(LOCAL_PATH)")

# These two variables are set first, so they can be overridden
# by BoardConfigVendor.mk
#BOARD_USES_GENERIC_AUDIO := false
#USE_CAMERA_STUB := true

# Use the non-open-source parts, if they're present
-include $(VENDOR_PATH)/BoardConfigVendor.mk


TARGET_ARCH		   	:= arm
TARGET_CPU_ABI		:= armeabi-v7a
TARGET_CPU_ABI2		:= armeabi
TARGET_CPU_SMP		:= true
TARGET_CPU_VARIANT	:= cortex-a9

TARGET_ARCH_VARIANT	:= armv7-a-neon
ARCH_ARM_HAVE_TLS_REGISTER := true

# Target Image Exclusions
TARGET_NO_BOOTLOADER 	:= true
TARGET_NO_RECOVERY		:= true
TARGET_NO_RADIOIMAGE    := true

# Bluetooth
BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_BLUETOOTH_TI := true
BOARD_BLUETOOTH_DOES_NOT_USE_RFKILL := false
BOARD_BLUETOOTH_USES_HCIATTACH_PROPERTY := false
BOARD_WPAN_DEVICE := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/archos/archos_g9/bluetooth

BOARD_USE_CUSTOM_LIBION := true

BOARD_CUSTOM_BOOTIMG_MK := device/archos/archos_g9/shbootimg.mk
BOARD_KERNEL_BASE      := 0x80000000
TARGET_KERNEL_CONFIG    := archos_g9_defconfig

TARGET_BOARD_PLATFORM        := omap4
TARGET_USE_CUSTOM_LUN_FILE_PATH := "/sys/devices/virtual/android_usb/android0/f_mass_storage/lun/file"
TARGET_BOOTLOADER_BOARD_NAME := archos

# Recovery
#TARGET_RECOVERY_PIXEL_FORMAT   := "BGRA_8888"
#TARGET_RECOVERY_UI_LIB         := librecovery_ui_panda
# device-specific extensions to the updater binary
#TARGET_RELEASETOOLS_EXTENSIONS := device/ti/panda

BOARD_EGL_CFG := $(PREBUILT_PATH)/system/lib/egl/egl.cfg
USE_OPENGL_RENDERER        := true

# HWComposer                                                                                                                                                                                                                                                                   
BOARD_USES_HWCOMPOSER := true                                                                                                                                                                                                                                                  
BOARD_USE_SYSFS_VSYNC_NOTIFICATION := true                                                                                                                                                                                                                            
TARGET_HAS_WAITFORVSYNC := true                                                                                                                                                                                                                                                
                                                                                                                                                                                                                                                                               
# Camera                                                                                                                                                                                                                                                                       
COMMON_GLOBAL_CFLAGS += -DDISABLE_HW_ID_MATCH_CHECK                                                                                                                                                                                                                            

# OMAP
ENHANCED_DOMX        := true
OMAP_ENHANCEMENT     := true
COMMON_GLOBAL_CFLAGS += -DTARGET_OMAP4 -DOMAP_ENHANCEMENT
OMAP_ENHANCEMENT_MULTIGPU := true
BOARD_USE_TI_ENHANCED_DOMX := true

ifdef OMAP_ENHANCEMENT_MULTIGPU
    COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT_MULTIGPU
endif

# CodeAurora Optimizations: msm8960: Improve performance of memmove, bcopy, and memmove_words
# added by twa_priv
TARGET_USE_KRAIT_BIONIC_OPTIMIZATION := true
TARGET_USE_KRAIT_PLD_SET := true
TARGET_KRAIT_BIONIC_PLDOFFS := 10
TARGET_KRAIT_BIONIC_PLDTHRESH := 10
TARGET_KRAIT_BIONIC_BBTHRESH := 64
TARGET_KRAIT_BIONIC_PLDSIZE := 64

#ENABLE_WEBGL := true


TARGET_USERIMAGES_USE_EXT4         := true
TARGET_USERIMAGES_SPARSE_EXT_DISABLED := true
BOARD_USERDATAIMAGE_PARTITION_SIZE := 536870912
BOARD_SYSTEMIMAGE_PARTITION_SIZE   := 532653184
BOARD_CACHEIMAGE_PARTITION_SIZE    := 6
BOARD_FLASH_BLOCK_SIZE             := 4096

# Connectivity - Wi-Fi
USES_TI_MAC80211 := true
COMMON_GLOBAL_CFLAGS 			+= -DUSES_TI_MAC80211
BOARD_WPA_SUPPLICANT_DRIVER      := NL80211
WPA_SUPPLICANT_VERSION           := VER_0_8_X_TI
BOARD_HOSTAPD_DRIVER             := NL80211
BOARD_WLAN_DEVICE                := wl12xx_mac80211
BOARD_SOFTAP_DEVICE              := wl12xx_mac80211
WIFI_DRIVER_MODULE_PATH          := "/system/lib/modules/wl12xx_sdio.ko"
WIFI_DRIVER_MODULE_NAME          := "wl12xx_sdio"
#WIFI_FIRMWARE_LOADER             := ""

#Developer option to let adbd always run as root
BOARD_ALWAYS_INSECURE := true

# Boot animation
# We don't an alpha channel, we want to preload and cache the 
# animation to avoid jank!
TARGET_BOOTANIMATION_USE_RGB565 := true
TARGET_BOOTANIMATION_PRELOAD := true
TARGET_BOOTANIMATION_TEXTURE_CACHE := true


#camera
#COMMON_GLOBAL_CFLAGS             += -DTIMM_OSAL_DEBUG_TRACE_DETAIL=2 -DTIMM_OSAL_DEBUG_TRACE_LEVEL=5
#TI_CAMERAHAL_DEBUG_ENABLED         := true
#TI_CAMERAHAL_VERBOSE_DEBUG_ENABLED := true
#TI_CAMERAHAL_DEBUG_FUNCTION_NAMES  := true
# For further logging take a look into camera_inc_CameraHal.h and turn on the debug flag

# The SGS series is hdpi however with new development going toward xhdpi we need to inherit xhdpi or we will lose those pngs
PRODUCT_AAPT_CONFIG := large mdpi hdpi xhdpi nodpi
PRODUCT_AAPT_PREF_CONFIG := mdpi

# Misc.
BOARD_NEEDS_CUTILS_LOG := true

# CWM Recovery
BOARD_HAS_SDCARD_INTERNAL := true
BOARD_HAS_NO_SELECT_BUTTON := true
BOARD_CUSTOM_RECOVERY_KEYMAPPING := ../../device/archos/archos_g9/recovery/recovery_keys.c
TARGET_RECOVERY_UPDATER_LIBS += librecovery_updater_archos_g9

# Common TI device independent definitions (such as DOMX_PATH...)
#include $(LOCAL_PATH)/device-vendor.mk
# TWRP Config
DEVICE_RESOLUTION := 1024x768
BOARD_HAS_FLIPPED_SCREEN := true 
TARGET_USERIMAGES_USE_EXT4 := true
TW_BRIGHTNESS_PATH := /sys/devices/platform/omap_pwm_led.0/leds/lcd-backlight/brightness
TW_MAX_BRIGHTNESS := 254

#TARGET_RECOVERY_UPDATER_LIBS += librecovery_updater_archos_g9

WITH_DEXPREOPT := true

# C/C++ macros for OMAP_ENHANCEMENT
ifdef OMAP_ENHANCEMENT
    COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT -DTARGET_OMAP4
endif

ifdef OMAP_ENHANCEMENT_BURST_CAPTURE
    COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT_BURST_CAPTURE
endif

ifdef OMAP_ENHANCEMENT_S3D
    COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT_S3D
endif

ifdef OMAP_ENHANCEMENT_CPCAM
    COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT_CPCAM
endif

ifdef OMAP_ENHANCEMENT_VTC
    COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT_VTC
endif

ifdef USE_ITTIAM_AAC
    COMMON_GLOBAL_CFLAGS += -DUSE_ITTIAM_AAC
endif

ifdef OMAP_ENHANCEMENT_MULTIGPU
    COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT_MULTIGPU
endif


# TI Enhancement Settings (Part 2)
ifdef BOARD_USE_TI_ENHANCED_DOMX
    BOARD_USE_TI_DUCATI_H264_PROFILE := true
    COMMON_GLOBAL_CFLAGS += -DENHANCED_DOMX
    ENHANCED_DOMX := true
    DOMX_PATH := hardware/ti/domx
else
    DOMX_PATH := hardware/ti/omap4xxx/domx
endif


BUILDINFO_SH := device/archos/archos_g9/buildinfo.sh
endif
