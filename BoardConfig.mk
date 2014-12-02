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

EXPERIMENTAL_USE_JAVA7 := true

DEVICE_PATH := $(LOCAL_PATH)
#$(warning "LocalPath: $(LOCAL_PATH)")

# These two variables are set first, so they can be overridden
# by BoardConfigVendor.mk
#BOARD_USES_GENERIC_AUDIO := false
#USE_CAMERA_STUB := true

# Use the non-open-source parts, if they're present
-include $(VENDOR_PATH)/BoardConfigVendor.mk

TARGET_ARCH		   := arm
TARGET_CPU_ABI             := armeabi-v7a
TARGET_CPU_ABI2            := armeabi
TARGET_CPU_SMP             := true
TARGET_ARCH_VARIANT        := armv7-a-neon
TARGET_CPU_VARIANT	   := cortex-a9
ARCH_ARM_HAVE_TLS_REGISTER := true
#Set 32 byte cache line to true (taken from blaze config)
ARCH_ARM_HAVE_32_BYTE_CACHE_LINES := true

TARGET_GLOBAL_CFLAGS += -mtune=cortex-a9 -mfpu=neon -mfloat-abi=softfp
TARGET_GLOBAL_CPPFLAGS += -mtune=cortex-a9 -mfpu=neon -mfloat-abi=softfp

# CodeAurora Optimizations: msm8960: Improve performance of memmove, bcopy, and memmove_words
# added by twa_priv
TARGET_USE_KRAIT_BIONIC_OPTIMIZATION := true
TARGET_USE_KRAIT_PLD_SET := true
TARGET_KRAIT_BIONIC_PLDOFFS := 10
TARGET_KRAIT_BIONIC_PLDTHRESH := 10
TARGET_KRAIT_BIONIC_BBTHRESH := 64
TARGET_KRAIT_BIONIC_PLDSIZE := 64


# Needed for blobs
COMMON_GLOBAL_CFLAGS += -DNEEDS_VECTORIMPL_SYMBOLS

BOARD_HAVE_OLD_ION_API := true

# Bluetooth
BOARD_HAVE_BLUETOOTH_TI := true
BOARD_HAVE_BLUETOOTH := true
BOARD_WPAN_DEVICE := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/archos/archos_g9/bluetooth

TARGET_NO_BOOTLOADER := true

TARGET_NO_RADIOIMAGE         := true
TARGET_BOARD_PLATFORM        := omap4
TARGET_BOOTLOADER_BOARD_NAME := archos
TARGET_BOARD_OMAP_CPU	     := 4430

# Recovery
#TARGET_RECOVERY_PIXEL_FORMAT   := "BGRA_8888"
#TARGET_RECOVERY_UI_LIB         := librecovery_ui_panda
# device-specific extensions to the updater binary
#TARGET_RELEASETOOLS_EXTENSIONS := device/ti/panda

BOARD_EGL_CFG := $(PREBUILT_PATH)/system/lib/egl/egl.cfg

USE_OPENGL_RENDERER        := true
# Causes out of memory exceptions
#NUM_FRAMEBUFFER_SURFACE_BUFFERS := 3
#Refer to frameworks/native/libs/gui/SurfaceComposerClient.cpp
COMMON_GLOBAL_CFLAGS += -DFORCE_SCREENSHOT_CPU_PATH
#COMMON_GLOBAL_CFLAGS += -DUSE_MHEAP_SCREENSHOT

# HWComposer                                                                                                                                                                                                                                                                   
BOARD_USES_HWCOMPOSER := true                                                                                                                                                                                                                                                  
# BOARD_USE_SYSFS_VSYNC_NOTIFICATION := true
# set if the target supports FBIO_WAITFORVSYNC
# TARGET_HAS_WAITFORVSYNC := true
# This is implemented since kernel 3.10.0 
TARGET_RUNNING_WITHOUT_SYNC_FRAMEWORK := true

BOARD_RIL_NO_CELLINFOLIST  := true

# Camera                                                                                                                                                                                                                                                                       
COMMON_GLOBAL_CFLAGS += -DDISABLE_HW_ID_MATCH_CHECK                                                                                                                                                                                                                            

#  Audio
BOARD_AUDIO_OUT_SAMPLING_RATE := 48000

# OMAP
ENHANCED_DOMX        := true
OMAP_ENHANCEMENT     := true
COMMON_GLOBAL_CFLAGS += -DTARGET_OMAP4
OMAP_ENHANCEMENT_MULTIGPU := true
BOARD_USE_TI_ENHANCED_DOMX := true

ifdef OMAP_ENHANCEMENT_MULTIGPU
    COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT_MULTIGPU
endif

#ENABLE_WEBGL := true

# Enable dalvik startup with a low memory footprint
TARGET_ARCH_LOWMEM := true

# Filesystem
TARGET_USERIMAGES_USE_EXT4 := true
BOARD_FLASH_BLOCK_SIZE := 4096
TARGET_USE_CUSTOM_LUN_FILE_PATH := "/sys/devices/virtual/android_usb/android0/f_mass_storage/lun/file"

# Use 384MByte as system image
BOARD_SYSTEMIMAGE_PARTITION_SIZE   := 402653184
#BOARD_SYSTEMIMAGE_PARTITION_SIZE  := 16777216
BOARD_USERDATAIMAGE_PARTITION_SIZE := 536870912
BOARD_CACHEIMAGE_PARTITION_SIZE    := 268435456
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE  := ext4
BOARD_FLASH_BLOCK_SIZE             := 4096

# Connectivity - Wi-Fi
USES_TI_MAC80211 := true
ifdef USES_TI_MAC80211
WPA_SUPPLICANT_VERSION := VER_0_8_X
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_wl12xx
BOARD_HOSTAPD_DRIVER := NL80211
BOARD_HOSTAPD_PRIVATE_LIB := lib_driver_cmd_wl12xx
PRODUCT_WIRELESS_TOOLS := true
BOARD_WLAN_DEVICE := wl12xx_mac80211
BOARD_SOFTAP_DEVICE := wl12xx_mac80211
WIFI_DRIVER_MODULE_PATH := "/system/lib/modules/wl12xx_sdio.ko"
WIFI_DRIVER_MODULE_NAME := "wl12xx_sdio"
WIFI_FIRMWARE_LOADER := ""
BOARD_WIFI_SKIP_CAPABILITIES := true
COMMON_GLOBAL_CFLAGS += -DUSES_TI_MAC80211 -DANDROID_P2P_STUB
endif

#TARGET_PROVIDES_INIT_RC := true
#TARGET_USERIMAGES_SPARSE_EXT_DISABLED := true

#Developer option to let adbd always run as root
BOARD_ALWAYS_INSECURE := true

# Bootanimation
TARGET_BOOTANIMATION_USE_RGB565 := true
TARGET_BOOTANIMATION_PRELOAD := true
# Disable texture-cache in boot-animation to fix OOM with new animation
TARGET_BOOTANIMATION_TEXTURE_CACHE := false

#adb has changed his security model since 4.2.2, don't use it (for now)
ADDITIONAL_DEFAULT_PROPERTIES = ro.adb.secure=0

#camera
#COMMON_GLOBAL_CFLAGS             += -DTIMM_OSAL_DEBUG_TRACE_DETAIL=2 -DTIMM_OSAL_DEBUG_TRACE_LEVEL=5
#TI_CAMERAHAL_DEBUG_ENABLED         := true
#TI_CAMERAHAL_VERBOSE_DEBUG_ENABLED := true
#TI_CAMERAHAL_DEBUG_FUNCTION_NAMES  := true
# For further logging take a look into camera_inc_CameraHal.h and turn on the debug flag

# adb has root
ADDITIONAL_DEFAULT_PROPERTIES += ro.secure=0
ADDITIONAL_DEFAULT_PROPERTIES += ro.allow.mock.location=1
ADDITIONAL_DEFAULT_PROPERTIES += persist.hwc.sw_vsync=1

# Currently only Camera API 1 is supported
ADDITIONAL_DEFAULT_PROPERTIES += camera2.portability.force_api=1

# AAPT config
#PRODUCT_AAPT_CONFIG := normal large hdpi xhdpi xxhdpi nodpi
PRODUCT_AAPT_CONFIG := normal mdpi hdpi nodpi
PRODUCT_AAPT_PREF_CONFIG := mdpi

# Misc.
BOARD_NEEDS_CUTILS_LOG := true
BOARD_USES_SECURE_SERVICES := true

# Recovery
BOARD_HAS_SDCARD_INTERNAL := true
BOARD_HAS_NO_SELECT_BUTTON := true
BOARD_CUSTOM_RECOVERY_KEYMAPPING := ../../device/archos/archos_g9/recovery/recovery_keys.c


# inherit from common
-include device/archos/omap4-common/BoardConfigCommon.mk

BOARD_SEPOLICY_DIRS += \
    device/archos/archos_g9/sepolicy

BOARD_SEPOLICY_UNION += \
    bluetooth.te \
    debuggered.te \
    file_contexts \
    device.te \
    dhcp.te \
    domain.te \
    file.te \
    init.te \
    init_shell.te \
    mediaserver.te \
    netd.te \
    pvrsrvinit.te \
    rild.te \
    system.te \
    tee.te \
    vold.te
