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

ifeq ($(TARGET_PREBUILT_KERNEL),)
LOCAL_KERNEL := device/archos/archos_g9/kernel
else
LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
endif

# Init Files
PRODUCT_COPY_FILES := \
       	$(LOCAL_KERNEL):kernel \
       	$(PREBUILT_PATH)/root/init.dongle.rc:root/init.dongle.rc \
       	$(PREBUILT_PATH)/root/init.rc:root/init.archos-ics.rc

# Publish that we support the live wallpaper feature.
PRODUCT_COPY_FILES += \
    packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:/system/etc/permissions/android.software.live_wallpaper.xml
	
#	Live wallpaper packages
PRODUCT_PACKAGES := \
    LiveWallpapers \
    LiveWallpapersPicker \
    MagicSmokeWallpapers \
    VisualizationWallpapers \
    librs_jni
        
# Modem Kernel Modules
PRODUCT_COPY_FILES += \
        $(PREBUILT_PATH)/root/lib/modules/usb_wwan.ko:root/lib/modules/usb_wwan.ko \
        $(PREBUILT_PATH)/root/lib/modules/option.ko:root/lib/modules/option.ko \
        $(PREBUILT_PATH)/root/lib/modules/hso.ko:root/lib/modules/hso.ko 

# EGL Proprietary Files
#PRODUCT_COPY_FILES += \
#	$(PREBUILT_PATH)/lib/egl/libEGL_POWERVR_SGX540_120.so:system/lib/egl/libEGL_POWERVR_SGX540_120.so \
#	$(PREBUILT_PATH)/lib/egl/libGLESv1_CM_POWERVR_SGX540_120.so:system/lib/egl/libGLESv1_CM_POWERVR_SGX540_120.so \
#	$(PREBUILT_PATH)/lib/egl/libGLESv2_POWERVR_SGX540_120.so:system/lib/egl/libGLESv2_POWERVR_SGX540_120.so \
#	$(PREBUILT_PATH)/lib/hw/gralloc.omap4.so:system/lib/hw/gralloc.omap4.so \
#	$(PREBUILT_PATH)/lib/libglslcompiler.so:system/lib/libglslcompiler.so \
#	$(PREBUILT_PATH)/lib/libIMGegl.so:system/lib/libIMGegl.so \
#	$(PREBUILT_PATH)/lib/libpvr2d.so:system/lib/libpvr2d.so \
#	$(PREBUILT_PATH)/lib/libpvrANDROID_WSEGL.so:system/lib/libpvrANDROID_WSEGL.so \
#	$(PREBUILT_PATH)/lib/libPVRScopeServices.so:system/lib/libPVRScopeServices.so \
#	$(PREBUILT_PATH)/lib/libsrv_init.so:system/lib/libsrv_init.so \
#	$(PREBUILT_PATH)/lib/libsrv_um.so:system/lib/libsrv_um.so \
#	$(PREBUILT_PATH)/lib/libusc.so:system/lib/libusc.so \
#        $(PREBUILT_PATH)/bin/pvrsrvinit:system/bin/pvrsrvinit \

# Graphics
PRODUCT_COPY_FILES += \
	$(PREBUILT_PATH)/lib/modules/omaplfb_sgx540_120.ko:system/lib/modules/omaplfb_sgx540_120.ko \
	$(PREBUILT_PATH)/vendor/lib/hw/hwcomposer.omap4.so:system/vendor/lib/hw/hwcomposer.omap4.so

# DRM Proprietary Files
PRODUCT_COPY_FILES += \
	$(PREBUILT_PATH)/vendor/lib/libwvm.so:system/vendor/lib/libwvm.so \
	$(PREBUILT_PATH)/vendor/lib/libwvdrm_L3.so:system/vendor/lib/libwvdrm_L3.so \
	$(PREBUILT_PATH)/vendor/lib/libWVStreamControlAPI_L3.so:system/vendor/lib/libWVStreamControlAPI_L3.so \
        
        
# Vold Mounting
PRODUCT_COPY_FILES += \
	$(PREBUILT_PATH)/etc/fstab:system/etc/fstab \
	$(PREBUILT_PATH)/etc/fstab_nand:system/etc/fstab_nand \
	$(PREBUILT_PATH)/etc/fstab_nor:system/etc/fstab_nor \
	$(PREBUILT_PATH)/etc/vold.A80H.fstab:system/etc/vold.A80H.fstab \
	$(PREBUILT_PATH)/etc/vold.A80S.fstab:system/etc/vold.A80S.fstab \
	$(PREBUILT_PATH)/etc/vold.A101H.fstab:system/etc/vold.A101H.fstab \
	$(PREBUILT_PATH)/etc/vold.A101S.fstab:system/etc/vold.A101S.fstab \
	$(PREBUILT_PATH)/etc/vold.fstab:system/etc/vold.fstab

# Key Layout/Screen Config.
PRODUCT_COPY_FILES += \
	$(PREBUILT_PATH)/usr/idc/cpt_i2c_tsp.idc:system/usr/idc/cpt_i2c_tsp.idc \
	$(PREBUILT_PATH)/usr/idc/cypress-tma340.idc:system/usr/idc/cypress-tma340.idc \
	$(PREBUILT_PATH)/usr/idc/pixcir_i2c_tsp.idc:system/usr/idc/pixcir_i2c_tsp.idc \
	$(PREBUILT_PATH)/usr/idc/qwerty.idc:system/usr/idc/qwerty.idc \
	$(PREBUILT_PATH)/usr/idc/qwerty2.idc:system/usr/idc/qwerty2.idc \
	$(PREBUILT_PATH)/usr/idc/tr16c0_i2c_tsp.idc:system/usr/idc/tr16c0_i2c_tsp.idc

# Init scripts
PRODUCT_COPY_FILES += \
	$(PREBUILT_PATH)/etc/init/init.A80H.rc:system/etc/init/init.A80H.rc \
	$(PREBUILT_PATH)/etc/init/init.A80S.rc:system/etc/init/init.A80S.rc \
	$(PREBUILT_PATH)/etc/init/init.A101H.rc:system/etc/init/init.A101H.rc \
	$(PREBUILT_PATH)/etc/init/init.A101S.rc:system/etc/init/init.A101S.rc \
	$(PREBUILT_PATH)/etc/init/init.archos.rc:system/etc/init/init.archos.rc \
	$(PREBUILT_PATH)/etc/init/init.rc:system/etc/init/init.rc \
	$(PREBUILT_PATH)/etc/init/ueventd.archos.rc:system/etc/init/ueventd.archos.rc
	
# pppd scripts
PRODUCT_COPY_FILES += \
	$(PREBUILT_PATH)/etc/ppp/init_pppd_gprs:system/etc/ppp/init_pppd_gprs \
	$(PREBUILT_PATH)/etc/ppp/init_pppd_datakey:system/etc/ppp/init_pppd_datakey \
	$(PREBUILT_PATH)/etc/ppp/ip-up-datakey:system/etc/ppp/ip-up-datakey \
	$(PREBUILT_PATH)/etc/ppp/ip-up-gprs:system/etc/ppp/ip-up-gprs \
	$(PREBUILT_PATH)/etc/ppp/ip-down-datakey:system/etc/ppp/ip-down-datakey \
	$(PREBUILT_PATH)/etc/ppp/ip-down-gprs:system/etc/ppp/ip-down-gprs \
	$(PREBUILT_PATH)/etc/ppp/peers/datakey:system/etc/ppp/peers/datakey \
	$(PREBUILT_PATH)/etc/ppp/peers/gprs:system/etc/ppp/peers/gprs \
	$(PREBUILT_PATH)/etc/ppp/write_secrets:system/etc/ppp/write_secrets 
	
# chatscripts scripts
PRODUCT_COPY_FILES += \
        $(PREBUILT_PATH)/etc/chatscripts/gprs_start:system/etc/chatscripts/gprs_start \
        $(PREBUILT_PATH)/etc/chatscripts/gprs_stop:system/etc/chatscripts/gprs_stop \
        $(PREBUILT_PATH)/etc/chatscripts/datakey_start:system/etc/chatscripts/datakey_start \
        $(PREBUILT_PATH)/etc/chatscripts/datakey_stop:system/etc/chatscripts/datakey_stop 

# xbin
PRODUCT_COPY_FILES += \
        $(PREBUILT_PATH)/bin/hotplugd:system/bin/hotplugd \
        $(PREBUILT_PATH)/bin/sdcard:system/bin/sdcard \
        $(PREBUILT_PATH)/xbin/chat:system/xbin/chat \
        $(PREBUILT_PATH)/lib/libhuaweigeneric-ril.so:system/lib/libhuaweigeneric-ril.so \
        $(PREBUILT_PATH)/lib/libtcl-ril.so:system/lib/libtcl-ril.so                 \
        $(PREBUILT_PATH)/lib/libusb.so:system/lib/libusb.so                       

PRODUCT_COPY_FILES += \
    $(PREBUILT_PATH)/etc/media_profiles.xml:system/etc/media_profiles.xml \
    $(PREBUILT_PATH)/etc/media_codecs.xml:system/etc/media_codecs.xml \
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
 
# All non-essential proprietary files for Archos G9 but are still kinda important
PRODUCT_COPY_FILES := \
    $PREBUILT_PATH/bin/batteryd:/system/bin/batteryd \
    $PREBUILT_PATH/bin/akmd8975:/system/bin/akmd8975 \
    $PREBUILT_PATH/bin/last_kmsg:/system/bin/last_kmsg \
    $PREBUILT_PATH/bin/nand:/system/bin/nand \
    $PREBUILT_PATH/bin/powerd:/system/bin/powerd \
    $PREBUILT_PATH/bin/sharesd:/system/bin/sharesd \
    $PREBUILT_PATH/bin/uim-sysfs:/system/bin/uim-sysfs \
    $PREBUILT_PATH/bin/watchdog_daemon:/system/bin/watchdog_daemon \
    $PREBUILT_PATH/bin/navd:/system/bin/navd \
    $PREBUILT_PATH/bin/rfkill_chmod.sh:/system/bin/rfkill_chmod.sh \
    $PREBUILT_PATH/lib/libedid.so:/system/lib/libedid.so \
    $PREBUILT_PATH/lib/libgps.so:/system/lib/libgps.so \
    $PREBUILT_PATH/lib/libgpsservices.so:/system/lib/libgpsservices.so \
    $PREBUILT_PATH/lib/libmcphalgps.so:/system/lib/libmcphalgps.so \
    $PREBUILT_PATH/lib/libsupllocationprovider.so:/system/lib/libsupllocationprovider.so \
    $PREBUILT_PATH/lib/libtiutils.so:/system/lib/libtiutils.so \
    $PREBUILT_PATH/lib/hw/audio.primary.archos.so:/system/lib/hw/audio.primary.archos.so \
    $PREBUILT_PATH/lib/hw/audio_policy.archos.so:/system/lib/hw/audio_policy.archos.so \
    $PREBUILT_PATH/lib/hw/gps.archos.so:/system/lib/hw/gps.archos.so \
    $PREBUILT_PATH/lib/hw/lights.archos.so:/system/lib/hw/lights.archos.so \
    $PREBUILT_PATH/lib/hw/sensors.omap4.so:/system/lib/hw/sensors.omap4.so 

#    $PREBUILT_PATH/lib/hw/hwcomposer.omap4.so:/system/vendor/lib/hw/hwcomposer.omap4.so \
#    $PREBUILT_PATH/lib/hw/camera.omap4.so:/system/lib/hw/camera.omap4.so \
#    $PREBUILT_PATH/lib/libskia.so:/system/lib/libskia.so


# archos audio depends on tinyalsa
PRODUCT_COPY_FILES := \
    $PREBUILT_PATH/lib/libtinyalsa.so:/system/lib/libtinyalsa.so

# OMX Libraries for 512MB RAM Ducati binary
PRODUCT_COPY_FILES += \
    $PREBUILT_PATH/lib/libOMX.TI.DUCATI1.MISC.SAMPLE.so:/system/lib/libOMX.TI.DUCATI1.MISC.SAMPLE.so \
    $PREBUILT_PATH/lib/libOMX.TI.DUCATI1.VIDEO.CAMERA.so:/system/lib/libOMX.TI.DUCATI1.VIDEO.CAMERA.so \
    $PREBUILT_PATH/lib/libOMX.TI.DUCATI1.VIDEO.DECODER.secure.so:/system/lib/libOMX.TI.DUCATI1.VIDEO.DECODER.secure.so \
    $PREBUILT_PATH/lib/libOMX.TI.DUCATI1.VIDEO.DECODER.so:/system/lib/libOMX.TI.DUCATI1.VIDEO.DECODER.so \
    $PREBUILT_PATH/lib/libOMX.TI.DUCATI1.VIDEO.H264E.so:/system/lib/libOMX.TI.DUCATI1.VIDEO.H264E.so \
    $PREBUILT_PATH/lib/libOMX.TI.DUCATI1.VIDEO.MPEG4E.so:/system/lib/libOMX.TI.DUCATI1.VIDEO.MPEG4E.so \
    $PREBUILT_PATH/lib/libOMX_Core.so:/system/lib/libOMX_Core.so

#    $PREBUILT_PATH/lib/libdomx.so:/system/lib/libdomx.so 

# Misc Stuff
PRODUCT_COPY_FILES += \
    $PREBUILT_PATH/etc/gps/cert/client_keystore.bks:/system/etc/gps/cert/client_keystore.bks \
    $PREBUILT_PATH/etc/gps/config/GPSCConfigFile.cfg:/system/etc/gps/config/GPSCConfigFile.cfg \
    $PREBUILT_PATH/etc/gps/config/GpsConfigFile.txt:/system/etc/gps/config/GpsConfigFile.txt \
    $PREBUILT_PATH/etc/gps/config/inavconfigfile.txt:/system/etc/gps/config/inavconfigfile.txt \
    $PREBUILT_PATH/etc/gps/config/pathconfigfile.txt:/system/etc/gps/config/pathconfigfile.txt \
    $PREBUILT_PATH/etc/gps/config/PeriodicConfFile.cfg:/system/etc/gps/config/PeriodicConfFile.cfg \
    $PREBUILT_PATH/etc/gps/config/SuplConfig.spl:/system/etc/gps/config/SuplConfig.spl \
    $PREBUILT_PATH/etc/gps/patch/patch-X.0.ce:/system/etc/gps/patch/patch-X.0.ce

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
	qemu.hw.mainkeys=0

PRODUCT_PROPERTY_OVERRIDES += \
	ro.sf.lcd_density=160

PRODUCT_CHARACTERISTICS := tablet

# Pick up overlay for features that depend on non-open-source files
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

# Bluetooth
PRODUCT_PACKAGES += \
	ti-wpan-fw

# SGX driver
RODUCT_PACKAGES += \
	ti_omap4_sgx_libs

PRODUCT_NAME := cm_archos_g9
PRODUCT_DEVICE := archos_g9
