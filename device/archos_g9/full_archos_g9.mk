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

DEVICE_PREBUILT := device/archos/archos_g9/prebuilt

PRODUCT_AAPT_CONFIG := normal mdpi mdpi
PRODUCT_AAPT_PREF_CONFIG := mdpi

ifeq ($(TARGET_PREBUILT_KERNEL),)
LOCAL_KERNEL := device/archos/archos_g9/kernel
else
LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
endif

# Init Files
PRODUCT_COPY_FILES := \
       	$(LOCAL_KERNEL):kernel \
       	$(DEVICE_PREBUILT)/root/init.dongle.rc:root/init.dongle.rc \
       	$(DEVICE_PREBUILT)/root/init.rc:root/init.archos-ics.rc \
        

# Modem Kernel Modules
PRODUCT_COPY_FILES += \
        $(DEVICE_PREBUILT)/root/lib/modules/usb_wwan.ko:root/lib/modules/usb_wwan.ko \
        $(DEVICE_PREBUILT)/root/lib/modules/option.ko:root/lib/modules/option.ko \
        $(DEVICE_PREBUILT)/root/lib/modules/hso.ko:root/lib/modules/hso.ko 

# EGL Proprietary Files
PRODUCT_COPY_FILES += \
	$(DEVICE_PREBUILT)/lib/egl/libEGL_POWERVR_SGX540_120.so:system/lib/egl/libEGL_POWERVR_SGX540_120.so \
	$(DEVICE_PREBUILT)/lib/egl/libGLESv1_CM_POWERVR_SGX540_120.so:system/lib/egl/libGLESv1_CM_POWERVR_SGX540_120.so \
	$(DEVICE_PREBUILT)/lib/egl/libGLESv2_POWERVR_SGX540_120.so:system/lib/egl/libGLESv2_POWERVR_SGX540_120.so \
	$(DEVICE_PREBUILT)/lib/hw/gralloc.omap4.so:system/lib/hw/gralloc.omap4.so \
	$(DEVICE_PREBUILT)/lib/libglslcompiler.so:system/lib/libglslcompiler.so \
	$(DEVICE_PREBUILT)/lib/libIMGegl.so:system/lib/libIMGegl.so \
	$(DEVICE_PREBUILT)/lib/libpvr2d.so:system/lib/libpvr2d.so \
	$(DEVICE_PREBUILT)/lib/libpvrANDROID_WSEGL.so:system/lib/libpvrANDROID_WSEGL.so \
	$(DEVICE_PREBUILT)/lib/libPVRScopeServices.so:system/lib/libPVRScopeServices.so \
	$(DEVICE_PREBUILT)/lib/libsrv_init.so:system/lib/libsrv_init.so \
	$(DEVICE_PREBUILT)/lib/libsrv_um.so:system/lib/libsrv_um.so \
	$(DEVICE_PREBUILT)/lib/libusc.so:system/lib/libusc.so \
        $(DEVICE_PREBUILT)/bin/pvrsrvinit:system/bin/pvrsrvinit \

# DRM Proprietary Files
PRODUCT_COPY_FILES += \
	$(DEVICE_PREBUILT)/vendor/lib/libwvm.so:system/vendor/lib/libwvm.so \
	$(DEVICE_PREBUILT)/vendor/lib/libwvdrm_L3.so:system/vendor/lib/libwvdrm_L3.so \
	$(DEVICE_PREBUILT)/vendor/lib/libWVStreamControlAPI_L3.so:system/vendor/lib/libWVStreamControlAPI_L3.so \
        
        
# Vold Mounting
PRODUCT_COPY_FILES += \
	$(DEVICE_PREBUILT)/etc/fstab:system/etc/fstab \
	$(DEVICE_PREBUILT)/etc/fstab_nand:system/etc/fstab_nand \
	$(DEVICE_PREBUILT)/etc/fstab_nor:system/etc/fstab_nor \
	$(DEVICE_PREBUILT)/etc/vold.A80H.fstab:system/etc/vold.A80H.fstab \
	$(DEVICE_PREBUILT)/etc/vold.A80S.fstab:system/etc/vold.A80S.fstab \
	$(DEVICE_PREBUILT)/etc/vold.A101H.fstab:system/etc/vold.A101H.fstab \
	$(DEVICE_PREBUILT)/etc/vold.A101S.fstab:system/etc/vold.A101S.fstab \
	$(DEVICE_PREBUILT)/etc/vold.fstab:system/etc/vold.fstab

# Key Layout/Screen Config.
PRODUCT_COPY_FILES += \
	$(DEVICE_PREBUILT)/usr/idc/cpt_i2c_tsp.idc:system/usr/idc/cpt_i2c_tsp.idc \
	$(DEVICE_PREBUILT)/usr/idc/cypress-tma340.idc:system/usr/idc/cypress-tma340.idc \
	$(DEVICE_PREBUILT)/usr/idc/pixcir_i2c_tsp.idc:system/usr/idc/pixcir_i2c_tsp.idc \
	$(DEVICE_PREBUILT)/usr/idc/qwerty.idc:system/usr/idc/qwerty.idc \
	$(DEVICE_PREBUILT)/usr/idc/qwerty2.idc:system/usr/idc/qwerty2.idc \
	$(DEVICE_PREBUILT)/usr/idc/tr16c0_i2c_tsp.idc:system/usr/idc/tr16c0_i2c_tsp.idc

# Init scripts
PRODUCT_COPY_FILES += \
	$(DEVICE_PREBUILT)/etc/init/init.A80H.rc:system/etc/init/init.A80H.rc \
	$(DEVICE_PREBUILT)/etc/init/init.A80S.rc:system/etc/init/init.A80S.rc \
	$(DEVICE_PREBUILT)/etc/init/init.A101H.rc:system/etc/init/init.A101H.rc \
	$(DEVICE_PREBUILT)/etc/init/init.A101S.rc:system/etc/init/init.A101S.rc \
	$(DEVICE_PREBUILT)/etc/init/init.archos.rc:system/etc/init/init.archos.rc \
	$(DEVICE_PREBUILT)/etc/init/init.rc:system/etc/init/init.rc \
	$(DEVICE_PREBUILT)/etc/init/ueventd.archos.rc:system/etc/init/ueventd.archos.rc
	
# pppd scripts
PRODUCT_COPY_FILES += \
	$(DEVICE_PREBUILT)/etc/ppp/init_pppd_gprs:system/etc/ppp/init_pppd_gprs \
	$(DEVICE_PREBUILT)/etc/ppp/init_pppd_datakey:system/etc/ppp/init_pppd_datakey \
	$(DEVICE_PREBUILT)/etc/ppp/ip-up-datakey:system/etc/ppp/ip-up-datakey \
	$(DEVICE_PREBUILT)/etc/ppp/ip-up-gprs:system/etc/ppp/ip-up-gprs \
	$(DEVICE_PREBUILT)/etc/ppp/ip-down-datakey:system/etc/ppp/ip-down-datakey \
	$(DEVICE_PREBUILT)/etc/ppp/ip-down-gprs:system/etc/ppp/ip-down-gprs \
	$(DEVICE_PREBUILT)/etc/ppp/peers/datakey:system/etc/ppp/peers/datakey \
	$(DEVICE_PREBUILT)/etc/ppp/peers/gprs:system/etc/ppp/peers/gprs \
	$(DEVICE_PREBUILT)/etc/ppp/write_secrets:system/etc/ppp/write_secrets 
	
# chatscripts scripts
PRODUCT_COPY_FILES += \
        $(DEVICE_PREBUILT)/etc/chatscripts/gprs_start:system/etc/chatscripts/gprs_start \
        $(DEVICE_PREBUILT)/etc/chatscripts/gprs_stop:system/etc/chatscripts/gprs_stop \
        $(DEVICE_PREBUILT)/etc/chatscripts/datakey_start:system/etc/chatscripts/datakey_start \
        $(DEVICE_PREBUILT)/etc/chatscripts/datakey_stop:system/etc/chatscripts/datakey_stop 

# xbin
PRODUCT_COPY_FILES += \
        $(DEVICE_PREBUILT)/bin/hotplugd:system/bin/hotplugd \
        $(DEVICE_PREBUILT)/bin/sdcard:system/bin/sdcard \
        $(DEVICE_PREBUILT)/xbin/chat:system/xbin/chat \
        $(DEVICE_PREBUILT)/lib/libhuaweigeneric-ril.so:system/lib/libhuaweigeneric-ril.so \
        $(DEVICE_PREBUILT)/lib/libtcl-ril.so:system/lib/libtcl-ril.so                 \
        $(DEVICE_PREBUILT)/lib/libusb.so:system/lib/libusb.so                       

PRODUCT_COPY_FILES += \
    $(DEVICE_PREBUILT)/etc/media_profiles.xml:system/etc/media_profiles.xml \
    $(DEVICE_PREBUILT)/etc/media_codecs.xml:system/etc/media_codecs.xml \
    frameworks/native/data/etc/tablet_core_hardware.xml:/system/etc/permissions/tablet_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:/system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:/system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:/system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:/system/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:/system/etc/permissions/android.software.live_wallpaper.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:/system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:/system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.camera.xml:/system/etc/permissions/android.hardware.camera.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:/system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.camera.autofocus.xml:/system/etc/permissions/android.hardware.camera.autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:/system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
 

PRODUCT_PACKAGES := \
        Superuser \
        su 

# Wifi
PRODUCT_PACKAGES += \
        lib_driver_cmd_wl12xx \
        wlan_loader \
        wlan_cu \
        dhcpcd.conf \
        wpa_supplicant.conf \
        make_ext4fs \
	com.android.future.usb.accessory

PRODUCT_PACKAGES += \
        libinvensense_mpl \
        parse_hdmi_edid \
        libedid \
        audio_policy.default \
        libaudioutils
#        hwcomposer.default 

PRODUCT_PROPERTY_OVERRIDES := \
	wifi.interface=wlan0 \
	hwui.render_dirty_regions=false

PRODUCT_PROPERTY_OVERRIDES += \
	ro.sf.lcd_density=160

PRODUCT_CHARACTERISTICS := tablet

DEVICE_PACKAGE_OVERLAYS := \
        device/archos/archos_g9/overlay

PRODUCT_TAGS += dalvik.gc.type-precise

PRODUCT_PACKAGES := \
        LiveWallpapers \
        LiveWallpapersPicker \
        VisualizationWallpapers \
        librs_jni \
	com.android.future.usb.accessory

## It all gets a bit experiemental here. See if we can enable voice and Sms/Mms
PRODUCT_PACKAGES += \
        Apollo \
        BasicSmsReceiver \
        Camera \
        CMWallpapers \
        CellBroadcastReceiver \
        Music \
        MusicFx \
        DeskClock \
        Development \
        Galaxy4 \
        Gallery2 \
        HoloSpiralWallpaper \
        Term \
        ThemeChooser \
        ThemeManager \
        Trebuchet \
        SpareParts \
        Phone \
        Mms
        

PRODUCT_PACKAGES += \
	audio.a2dp.default \
	libaudioutils

PRODUCT_PACKAGES += \
	dhcpcd.conf \
	TQS_D_1.7.ini \
	calibrator

# Filesystem management tools
PRODUCT_PACKAGES += \
	make_ext4fs \
	setup_fs \

# BlueZ test tools
PRODUCT_PACKAGES += \
	hciconfig \
	hcitool

PRODUCT_NAME := cm_archos_g9
PRODUCT_DEVICE := archos_g9
