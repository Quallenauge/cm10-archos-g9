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

# set to allow building from omap4-common
BOARD_VENDOR := archos

TARGET_BOARD_OMAP_CPU := 4430

# Use the non-open-source parts, if they're present
-include $(VENDOR_PATH)/BoardConfigVendor.mk
-include vendor/widevine/archos_g9/device-partial.mk
include device/ti/common-open/BoardConfig.mk

TARGET_ARCH		   			:= arm
TARGET_CPU_ABI             	:= armeabi-v7a
TARGET_CPU_ABI2            	:= armeabi
TARGET_CPU_SMP             	:= true
TARGET_ARCH_VARIANT        	:= armv7-a-neon
TARGET_ARCH_VARIANT_CPU		:= cortex-a9
ARCH_ARM_HAVE_TLS_REGISTER 	:= true
TARGET_GLOBAL_CFLAGS += -mtune=cortex-a9 -mfpu=neon -mfloat-abi=softfp
TARGET_GLOBAL_CPPFLAGS += -mtune=cortex-a9 -mfpu=neon -mfloat-abi=softfp

TARGET_NO_BOOTLOADER 	:= true
TARGET_NO_RADIOIMAGE    := true
TARGET_NO_RECOVERY		:= true
TARGET_RELEASETOOL_OTA_FROM_TARGET_SCRIPT := device/archos/archos_g9/releasetools/g9_ota_from_target_files
TARGET_RELEASETOOL_IMG_FROM_TARGET_SCRIPT := device/archos/archos_g9/releasetools/g9_img_from_target_files


# Bluetooth
BOARD_HAVE_BLUETOOTH_TI := true
BOARD_HAVE_BLUETOOTH := true
BOARD_WPAN_DEVICE := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/archos/archos_g9/bluetooth

TARGET_KERNEL_CONFIG    := archos_g9_defconfig
BOARD_KERNEL_BASE      := 0x80000000


SGX_MODULES:
	tar -xvf device/ti/proprietary-open/omap4/sgx_src/eurasia_km.tgz -C device/ti/proprietary-open/omap4/sgx_src/
	make -C device/ti/proprietary-open/omap4/sgx_src/eurasia_km/eurasiacon/build/linux2/omap4430_android/ ARCH=arm KERNELDIR=$(KERNEL_OUT) CROSS_COMPILE="arm-eabi-"\
					TARGET_PRODUCT="blaze_tablet" BUILD=release TARGET_SGX=540 PLATFORM_VERSION=4.0 O=$(KERNEL_MODULES_OUT)
					#clean
	make -C device/ti/proprietary-open/omap4/sgx_src/eurasia_km/eurasiacon/build/linux2/omap4430_android/ ARCH=arm KERNELDIR=$(KERNEL_OUT) CROSS_COMPILE="arm-eabi-"\
			TARGET_PRODUCT="blaze_tablet" BUILD=release TARGET_SGX=540 PLATFORM_VERSION=4.0 O=$(KERNEL_MODULES_OUT)
#	make -C device/ti/proprietary-open/omap4/sgx_src/eurasia_km/eurasiacon/build/linux2/omap4430_android/ ARCH=arm KERNELDIR=$(KERNEL_OUT) CROSS_COMPILE="arm-eabi-" TARGET_PRODUCT="blaze_tablet" BUILD=debug TARGET_SGX=540 PLATFORM_VERSION=4.0
	# It seems that the default sgx deployment will overwrite the builded one
	#cp out/target/product/archos_g9/target/kbuild/pvrsrvkm_sgx540_120.ko $(KERNEL_MODULES_OUT)

SGX_PREBUILD_MODULES:
	# Copy prebuild modules from archos
	cp -v -r device/archos/archos_g9/prebuilt/system/sgx/* $(TARGET_OUT)/

TIWLAN_MODULES:
	make -C hardware/ti/wlan/mac80211/compat_wl12xx ARCH=arm KERNEL_DIR=$(KERNEL_OUT) CROSS_COMPILE="arm-eabi-" KLIB=$(KERNEL_OUT) KLIB_BUILD=$(KERNEL_OUT)\
			CONFIG_COMPAT_WL12XX_SDIO=m O=$(KERNEL_MODULES_OUT)
	#mv hardware/ti/wlan/mac80211/compat_wl12xx/compat/compat.ko $(KERNEL_MODULES_OUT)
	#mv hardware/ti/wlan/mac80211/compat_wl12xx/net/mac80211/mac80211.ko $(KERNEL_MODULES_OUT)
	#mv hardware/ti/wlan/mac80211/compat_wl12xx/net/wireless/cfg80211.ko $(KERNEL_MODULES_OUT)
	#mv hardware/ti/wlan/mac80211/compat_wl12xx/drivers/net/wireless/wl12xx/wl12xx_sdio.ko $(KERNEL_MODULES_OUT)
	#mv hardware/ti/wlan/mac80211/compat_wl12xx/drivers/net/wireless/wl12xx/wl12xx.ko $(KERNEL_MODULES_OUT)
	#mv hardware/ti/wlan/mac80211/compat_wl12xx/drivers/net/wireless/wl12xx/wl12xx_spi.ko $(KERNEL_MODULES_OUT)


TIWLAN_OPENSOURCE_MODULES:
	#cd hardware/ti/wlan_os/ && pwd && git reset --hard && git clean -fd && bash scripts/driver-select ti
	cd hardware/ti/wlan_os/ && pwd && bash scripts/driver-select bt
	echo "make -C hardware/ti/wlan_os/ ARCH=arm KERNEL_DIR=$(KERNEL_OUT) CROSS_COMPILE="arm-eabi-" KLIB=$(KERNEL_OUT) KLIB_BUILD=$(KERNEL_OUT) \
		CONFIG_WLCORE=m CONFIG_WLCORE_SDIO=m O=$(KERNEL_MODULES_OUT)"
	make -C hardware/ti/wlan_os/ ARCH=arm KERNEL_DIR=$(KERNEL_OUT) CROSS_COMPILE="arm-eabi-" KLIB=$(KERNEL_OUT) KLIB_BUILD=$(KERNEL_OUT) \
		CONFIG_WLCORE=m CONFIG_WLCORE_SDIO=m -j8 O=$(KERNEL_MODULES_OUT)
	echo "Remove kernel builded (and oputdated modules)..."
	#rm $(KERNEL_MODULES_OUT)/wl12xx.ko
	#rm $(KERNEL_MODULES_OUT)/wl12xx_sdio.ko
	#rm $(KERNEL_MODULES_OUT)/wl12xx_sdio_test.ko
	#mv hardware/ti/wlan_os/compat/compat.ko $(KERNEL_MODULES_OUT)
	#mv hardware/ti/wlan_os/net/mac80211/mac80211.ko $(KERNEL_MODULES_OUT)
	#mv hardware/ti/wlan_os/net/wireless/cfg80211.ko $(KERNEL_MODULES_OUT)
	#mv hardware/ti/wlan_os/drivers/net/wireless/ti/wlcore/wlcore.ko $(KERNEL_MODULES_OUT)
	#mv hardware/ti/wlan_os/drivers/net/wireless/ti/wlcore/wlcore_sdio.ko $(KERNEL_MODULES_OUT)
	#mv hardware/ti/wlan_os/drivers/net/wireless/ti/wl12xx/wl12xx.ko $(KERNEL_MODULES_OUT)

	#cd hardware/ti/wlan_os/ && pwd && git reset --hard && git clean -fd && bash scripts/driver-select bt
	cd hardware/ti/wlan_os/ && pwd && bash scripts/driver-select bt
	make -C hardware/ti/wlan_os/ ARCH=arm KERNEL_DIR=$(KERNEL_OUT) CROSS_COMPILE="arm-eabi-" KLIB=$(KERNEL_OUT) KLIB_BUILD=$(KERNEL_OUT) \
			CONFIG_WLCORE=m CONFIG_WLCORE_SDIO=m -j8 O=$(KERNEL_MODULES_OUT)
	if [ ! -d ${KERNEL_MODULES_OUT}/bluetooth ]; then mkdir -p ${KERNEL_MODULES_OUT}/bluetooth; fi
	#mv hardware/ti/wlan_os/net/bluetooth/bluetooth.ko $(KERNEL_MODULES_OUT)/bluetooth/
	#mv hardware/ti/wlan_os/net/bluetooth/rfcomm/rfcomm.ko $(KERNEL_MODULES_OUT)/bluetooth/
	#mv hardware/ti/wlan_os/net/bluetooth/hidp/hidp.ko $(KERNEL_MODULES_OUT)/bluetooth/
	#mv hardware/ti/wlan_os/drivers/bluetooth/hci_uart.ko $(KERNEL_MODULES_OUT)/bluetooth/
	#mv hardware/ti/wlan_os/net/bluetooth/bnep/bnep.ko $(KERNEL_MODULES_OUT)/bluetooth/
	#mv hardware/ti/wlan_os/drivers/bluetooth/btwilink.ko $(KERNEL_MODULES_OUT)/bluetooth/

	if [ ! -d ${PRODUCT_OUT}/system/etc/firmware/ti-connectivity ]; then mkdir -p ${PRODUCT_OUT}/system/etc/firmware/ti-connectivity; fi
	cp hardware/linux-firmware/ti-connectivity/TIInit_7.2.31.bts ${PRODUCT_OUT}/system/etc/firmware/
	cp hardware/linux-firmware/ti-connectivity/wl1271-fw-2.bin ${PRODUCT_OUT}/system/etc/firmware/ti-connectivity/
	cp hardware/linux-firmware/ti-connectivity/wl1271-fw-ap.bin ${PRODUCT_OUT}/system/etc/firmware/ti-connectivity/
	cp hardware/linux-firmware/ti-connectivity/wl1271-fw.bin ${PRODUCT_OUT}/system/etc/firmware/ti-connectivity/
	cp hardware/linux-firmware/ti-connectivity/wl1271-nvs.bin ${PRODUCT_OUT}/system/etc/firmware/ti-connectivity/
	cp hardware/linux-firmware/ti-connectivity/wl127x-fw-5-sr.bin ${PRODUCT_OUT}/system/etc/firmware/ti-connectivity/
	cp hardware/linux-firmware/ti-connectivity/wl12xx-nvs.bin  ${PRODUCT_OUT}/system/etc/firmware/ti-connectivity/

	if [ ! -d ${PRODUCT_OUT}/system/etc/wifi ]; then mkdir -p ${PRODUCT_OUT}/system/etc/wifi; fi
	cp hardware/ti/wlan/mac80211/ti-utils/ini_files/127x/TQS_S_2.6.ini ${PRODUCT_OUT}/system/etc/wifi
    
TARGET_KERNEL_MODULES := SGX_MODULES TIWLAN_OPENSOURCE_MODULES

TARGET_BOARD_PLATFORM        := omap4
#TARGET_BOOTLOADER_BOARD_NAME := archos

# Recovery
#TARGET_RECOVERY_PIXEL_FORMAT   := "BGRA_8888"
#TARGET_RECOVERY_UI_LIB         := librecovery_ui_panda
# device-specific extensions to the updater binary
#TARGET_RELEASETOOLS_EXTENSIONS := device/ti/panda

BOARD_EGL_CFG := $(PREBUILT_PATH)/system/lib/egl/egl.cfg

#BOARD_USES_HGL            := true
#BOARD_USES_OVERLAY        := true
USE_OPENGL_RENDERER        := true
#BOARD_USES_PANDA_GRAPHICS := true



# HWComposer                                                                                                                                                                                                                                                                   
BOARD_USES_HWCOMPOSER := true                                                                                                                                                                                                                                                  
BOARD_USE_SYSFS_VSYNC_NOTIFICATION := true                                                                                                                                                                                                                                     
# set if the target supports FBIO_WAITFORVSYNC                                                                                                                                                                                                                                 
TARGET_HAS_WAITFORVSYNC := true                                                                                                                                                                                                                                                
                                                                                                                                                                                                                                                                               
# Camera                                                                                                                                                                                                                                                                       
COMMON_GLOBAL_CFLAGS += -DDISABLE_HW_ID_MATCH_CHECK                                                                                                                                                                                                                            

# OMAP
ENHANCED_DOMX        := true
OMAP_ENHANCEMENT     := true
COMMON_GLOBAL_CFLAGS += -DTARGET_OMAP4
OMAP_ENHANCEMENT_MULTIGPU := true
OMAP_ENHANCEMENT_S3D := true
BOARD_USE_TI_ENHANCED_DOMX := true


# TI Enhancement Settings (Part 2)
ifdef BOARD_USE_TI_ENHANCED_DOMX
    BOARD_USE_TI_DUCATI_H264_PROFILE := true
    COMMON_GLOBAL_CFLAGS += -DENHANCED_DOMX
    ENHANCED_DOMX := true
else
    DOMX_PATH := hardware/ti/omap4xxx/domx
endif

ifdef OMAP_ENHANCEMENT
    COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT -DTARGET_OMAP4
endif

ifdef OMAP_ENHANCEMENT_MULTIGPU
    COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT_MULTIGPU
endif
ifdef OMAP_ENHANCEMENT_S3D
    COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT_S3D
endif

# CodeAurora Optimizations: msm8960: Improve performance of memmove, bcopy, and memmove_words
# added by twa_priv
TARGET_USE_KRAIT_BIONIC_OPTIMIZATION := true
TARGET_USE_KRAIT_PLD_SET := true
TARGET_KRAIT_BIONIC_PLDOFFS := 10
TARGET_KRAIT_BIONIC_PLDTHRESH := 10
TARGET_KRAIT_BIONIC_BBTHRESH := 64
TARGET_KRAIT_BIONIC_PLDSIZE := 64

# Audio library
COMMON_GLOBAL_CFLAGS += -DICS_AUDIO_BLOB

#ENABLE_WEBGL := true

TARGET_USERIMAGES_USE_EXT4         := true
# Use 384MByte as system image
BOARD_SYSTEMIMAGE_PARTITION_SIZE   := 402653184
BOARD_CACHEIMAGE_PARTITION_SIZE    := 402653184
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE  := ext4
BOARD_USERDATAIMAGE_PARTITION_SIZE := 536870912
BOARD_FLASH_BLOCK_SIZE             := 4096

# Connectivity - Wi-Fi
USES_TI_MAC80211 := true
ifdef USES_TI_MAC80211
COMMON_GLOBAL_CFLAGS += -DUSES_TI_MAC80211
BOARD_WPA_SUPPLICANT_DRIVER      := NL80211
WPA_SUPPLICANT_VERSION           := VER_0_8_X_TI
BOARD_HOSTAPD_DRIVER             := NL80211
BOARD_WLAN_DEVICE                := wl12xx_mac80211
BOARD_SOFTAP_DEVICE              := wl12xx_mac80211

# Opensource driver uses wlcore
WIFI_DRIVER_MODULE_PATH          := "/system/lib/modules/wlcore_sdio.ko"
WIFI_DRIVER_MODULE_NAME          := "wlcore_sdio"
WIFI_FIRMWARE_LOADER             := ""
endif

BOARD_CUSTOM_BOOTIMG_MK := device/archos/archos_g9/shbootimg.mk

#TARGET_PROVIDES_INIT_RC := true
#TARGET_USERIMAGES_SPARSE_EXT_DISABLED := true





# Boot animation
TARGET_BOOTANIMATION_USE_RGB565 := true

#adb has changed his security model since 4.2.2, don't use it (for now)


#camera
TI_OMAP4_CAMERAHAL_VARIANT := false
USE_CAMERA_STUB := true
#COMMON_GLOBAL_CFLAGS             += -DTIMM_OSAL_DEBUG_TRACE_DETAIL=2 -DTIMM_OSAL_DEBUG_TRACE_LEVEL=5
#TI_CAMERAHAL_DEBUG_ENABLED         := true
#TI_CAMERAHAL_VERBOSE_DEBUG_ENABLED := true
#TI_CAMERAHAL_DEBUG_FUNCTION_NAMES  := true
# For further logging take a look into camera_inc_CameraHal.h and turn on the debug flag

#Developer option to let adbd always run as root
BOARD_ALWAYS_INSECURE := true
ADDITIONAL_DEFAULT_PROPERTIES += ro.secure=0 
ADDITIONAL_DEFAULT_PROPERTIES += ro.adb.secure=0
ADDITIONAL_DEFAULT_PROPERTIES += ro.allow.mock.location=1
ADDITIONAL_DEFAULT_PROPERTIES += persist.hwc.sw_vsync=1


# The SGS series is hdpi however with new development going toward xhdpi we need to inherit xhdpi or we will lose those pngs
PRODUCT_AAPT_CONFIG := large mdpi hdpi xhdpi nodpi
PRODUCT_AAPT_PREF_CONFIG := mdpi

WITH_DEXPREOPT := true

# Misc.
BOARD_NEEDS_CUTILS_LOG := true
#BOARD_USES_SECURE_SERVICES := f

# Recovery
BOARD_HAS_SDCARD_INTERNAL := true
BOARD_HAS_NO_SELECT_BUTTON := true
BOARD_CUSTOM_RECOVERY_KEYMAPPING := ../../device/archos/archos_g9/recovery/recovery_keys.c

# Common TI device independent definitions (such as DOMX_PATH...)
#include $(LOCAL_PATH)/device-vendor.mk
# TWRP Config
DEVICE_RESOLUTION := 1024x768
BOARD_HAS_FLIPPED_SCREEN := true 
TARGET_USERIMAGES_USE_EXT4 := true
TW_BRIGHTNESS_PATH := /sys/devices/platform/omap_pwm_led.0/leds/lcd-backlight/brightness
TW_MAX_BRIGHTNESS := 254


