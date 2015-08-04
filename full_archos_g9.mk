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

# Files which has to be on root folder

# Init Files
PRODUCT_COPY_FILES := \
	$(LOCAL_KERNEL):kernel \
	$(PREBUILT_PATH)/root/fstab.archosa80sboard:root/fstab.archosa80sboard \
	$(PREBUILT_PATH)/root/fstab.archosa101sboard:root/fstab.archosa101sboard \
	$(PREBUILT_PATH)/root/fstab.archosa80hboard:root/fstab.archosa80hboard \
	$(PREBUILT_PATH)/root/fstab.archosa101hboard:root/fstab.archosa101hboard \
	$(PREBUILT_PATH)/root/init.archos_init.rc:root/init.archos_init.rc \
	$(PREBUILT_PATH)/root/init.archosa80sboard.rc:root/init.archosa80sboard.rc \
	$(PREBUILT_PATH)/root/init.archosa101sboard.rc:root/init.archosa101sboard.rc \
	$(PREBUILT_PATH)/root/init.archosa80hboard.rc:root/init.archosa80hboard.rc \
	$(PREBUILT_PATH)/root/init.archosa101hboard.rc:root/init.archosa101hboard.rc \
	$(PREBUILT_PATH)/root/init.dongle.rc:root/init.dongle.rc \
	$(PREBUILT_PATH)/root/init.goldfish.rc:root/init.goldfish.rc \
	$(PREBUILT_PATH)/root/ueventd.archos.rc:root/ueventd.archosa80sboard.rc \
	$(PREBUILT_PATH)/root/ueventd.archos.rc:root/ueventd.archosa101sboard.rc \
	$(PREBUILT_PATH)/root/ueventd.archos.rc:root/ueventd.archosa80hboard.rc \
	$(PREBUILT_PATH)/root/ueventd.archos.rc:root/ueventd.archosa101hboard.rc \
	$(PREBUILT_PATH)/root/ueventd.goldfish.rc:root/ueventd.goldfish.rc \
	$(PREBUILT_PATH)/root/ueventd.rc:root/ueventd.rc


# Prebuilts /system/etc
PRODUCT_COPY_FILES += \
    $(PREBUILT_PATH)/system/etc/firmware/TIInit_7.2.31.bts:/system/etc/firmware/TIInit_7.2.31.bts

# Start Scripts for proprietary programs
PRODUCT_COPY_FILES += \
	$(PREBUILT_PATH)/root/usr/bin/start_avos:root/usr/bin/start_avos \
	$(PREBUILT_PATH)/root/usr/sbin/start_audiomixer:root/usr/sbin/start_audiomixer \
	$(PREBUILT_PATH)/system/etc/audio_policy.conf:system/etc/audio_policy.conf

# Files which has to be on system folder

# Publish that we support the live wallpaper feature.
PRODUCT_COPY_FILES += \
    packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:system/etc/permissions/android.software.live_wallpaper.xml
	
# Modem Kernel Modules
PRODUCT_COPY_FILES += \
        $(PREBUILT_PATH)/root/lib/modules/usb_wwan.ko:root/lib/modules/usb_wwan.ko \
        $(PREBUILT_PATH)/root/lib/modules/option.ko:root/lib/modules/option.ko \
        $(PREBUILT_PATH)/root/lib/modules/hso.ko:root/lib/modules/hso.ko 

# DRM Proprietary Files
PRODUCT_COPY_FILES += \
	$(PREBUILT_PATH)/system/lib/libdrmdecrypt.so:system/lib/libdrmdecrypt.so \
        $(PREBUILT_PATH)/system/vendor/lib/drm/libdrmwvmplugin.so:system/vendor/lib/drm/libdrmwvmplugin.so \
	$(PREBUILT_PATH)/system/vendor/lib/mediadrm/libwvdrmengine.so:system/vendor/lib/mediadrm/libwvdrmengine.so \
	$(PREBUILT_PATH)/system/vendor/lib/libwvm.so:system/vendor/lib/libwvm.so \
	$(PREBUILT_PATH)/system/vendor/lib/libwvdrm_L3.so:system/vendor/lib/libwvdrm_L3.so \
	$(PREBUILT_PATH)/system/vendor/lib/libWVStreamControlAPI_L3.so:system/vendor/lib/libWVStreamControlAPI_L3.so
        
        
# Vold Mounting
PRODUCT_COPY_FILES += \
	$(PREBUILT_PATH)/system/etc/fstab:system/etc/fstab \
	$(PREBUILT_PATH)/system/etc/fstab_nand:system/etc/fstab_nand \
	$(PREBUILT_PATH)/system/etc/fstab_nor:system/etc/fstab_nor \
	$(PREBUILT_PATH)/system/etc/vold.A80H.fstab:system/etc/vold.A80H.fstab \
	$(PREBUILT_PATH)/system/etc/vold.A80S.fstab:system/etc/vold.A80S.fstab \
	$(PREBUILT_PATH)/system/etc/vold.A101H.fstab:system/etc/vold.A101H.fstab \
	$(PREBUILT_PATH)/system/etc/vold.A101S.fstab:system/etc/vold.A101S.fstab \
	$(PREBUILT_PATH)/system/etc/vold.fstab:system/etc/vold.fstab

# Key Layout/Screen Config.
PRODUCT_COPY_FILES += \
	$(PREBUILT_PATH)/system/usr/idc/cpt_i2c_tsp.idc:system/usr/idc/cpt_i2c_tsp.idc \
	$(PREBUILT_PATH)/system/usr/idc/cypress-tma340.idc:system/usr/idc/cypress-tma340.idc \
	$(PREBUILT_PATH)/system/usr/idc/pixcir_i2c_tsp.idc:system/usr/idc/pixcir_i2c_tsp.idc \
	$(PREBUILT_PATH)/system/usr/idc/qwerty.idc:system/usr/idc/qwerty.idc \
	$(PREBUILT_PATH)/system/usr/idc/qwerty2.idc:system/usr/idc/qwerty2.idc \
	$(PREBUILT_PATH)/system/usr/idc/tr16c0_i2c_tsp.idc:system/usr/idc/tr16c0_i2c_tsp.idc

# pppd scripts
PRODUCT_COPY_FILES += \
	$(PREBUILT_PATH)/system/etc/ppp/init_pppd_gprs:system/etc/ppp/init_pppd_gprs \
	$(PREBUILT_PATH)/system/etc/ppp/init_pppd_datakey:system/etc/ppp/init_pppd_datakey \
	$(PREBUILT_PATH)/system/etc/ppp/ip-up-datakey:system/etc/ppp/ip-up-datakey \
	$(PREBUILT_PATH)/system/etc/ppp/ip-up-gprs:system/etc/ppp/ip-up-gprs \
	$(PREBUILT_PATH)/system/etc/ppp/ip-down-datakey:system/etc/ppp/ip-down-datakey \
	$(PREBUILT_PATH)/system/etc/ppp/ip-down-gprs:system/etc/ppp/ip-down-gprs \
	$(PREBUILT_PATH)/system/etc/ppp/peers/datakey:system/etc/ppp/peers/datakey \
	$(PREBUILT_PATH)/system/etc/ppp/peers/gprs:system/etc/ppp/peers/gprs \
	$(PREBUILT_PATH)/system/etc/ppp/write_secrets:system/etc/ppp/write_secrets

# chatscripts scripts
PRODUCT_COPY_FILES += \
	$(PREBUILT_PATH)/system/etc/chatscripts/gprs_start:system/etc/chatscripts/gprs_start \
	$(PREBUILT_PATH)/system/etc/chatscripts/gprs_stop:system/etc/chatscripts/gprs_stop \
	$(PREBUILT_PATH)/system/etc/chatscripts/datakey_start:system/etc/chatscripts/datakey_start \
	$(PREBUILT_PATH)/system/etc/chatscripts/datakey_stop:system/etc/chatscripts/datakey_stop

# Performance optimizations from http://forum.xda-developers.com/showpost.php?p=61824520&postcount=31 
# Thanks BlackGunZ
PRODUCT_COPY_FILES += \
	$(PREBUILT_PATH)/system/etc/init.d/99performance:system/etc/init.d/99performance

# xbin
PRODUCT_COPY_FILES += \
	$(PREBUILT_PATH)/system/xbin/chat:system/xbin/chat \
	$(PREBUILT_PATH)/system/lib/libhuaweigeneric-ril.so:system/lib/libhuaweigeneric-ril.so \
	$(PREBUILT_PATH)/system/lib/libtcl-ril.so:system/lib/libtcl-ril.so

PRODUCT_COPY_FILES += \
    $(PREBUILT_PATH)/system/etc/media_profiles.xml:system/etc/media_profiles.xml \
    $(PREBUILT_PATH)/system/etc/media_codecs.xml:system/etc/media_codecs.xml \
    external/stagefright-plugins/data/media_codecs_ffmpeg.xml:system/etc/media_codecs_ffmpeg.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:system/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:system/etc/media_codecs_google_video.xml \
    frameworks/native/data/etc/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:system/etc/permissions/android.software.live_wallpaper.xml \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml
#    frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml

# All non-essential proprietary files for Archos G9 but are still kinda important
PRODUCT_COPY_FILES += \
    $(PREBUILT_PATH)/system/bin/batteryd:system/bin/batteryd \
    $(PREBUILT_PATH)/system/bin/akmd8975:system/bin/akmd8975 \
    $(PREBUILT_PATH)/system/bin/last_kmsg:system/bin/last_kmsg \
    $(PREBUILT_PATH)/system/bin/nand:system/bin/nand \
    $(PREBUILT_PATH)/system/bin/powerd:system/bin/powerd \
    $(PREBUILT_PATH)/system/bin/sharesd:system/bin/sharesd \
    $(PREBUILT_PATH)/system/bin/watchdog_daemon:system/bin/watchdog_daemon \
    $(PREBUILT_PATH)/system/bin/navd:system/bin/navd \
    $(PREBUILT_PATH)/system/bin/start_navd:system/bin/start_navd \
    $(PREBUILT_PATH)/system/bin/rfkill_chmod.sh:system/bin/rfkill_chmod.sh \
    $(PREBUILT_PATH)/system/lib/hw/gps.archos.so:system/lib/hw/gps.archos.so \
    $(PREBUILT_PATH)/system/lib/hw/lights.archos.so:system/lib/hw/lights.archos.so \
    $(PREBUILT_PATH)/system/lib/hw/sensors.omap4.so:system/lib/hw/sensors.omap4.so

# Archos audio libraries - as long microphone doesn't work with upstream audio libs
#PRODUCT_COPY_FILES += \
#    $(PREBUILT_PATH)/system/lib/hw/audio.primary.archos.so:system/lib/hw/audio.primary.archos.so \
#    $(PREBUILT_PATH)/system/lib/hw/audio_policy.archos.so:system/lib/hw/audio_policy.archos.so 
PRODUCT_COPY_FILES += \
    $(PREBUILT_PATH)/system/etc/mixer_paths.xml:/system/etc/mixer_paths.xml \
    $(PREBUILT_PATH)/system/etc/audio_policy.conf:system/etc/audio_policy.conf


# GPS libraries 
PRODUCT_COPY_FILES += \
	 $(PREBUILT_PATH)/system/lib/libgpsservices.so:system/lib/libgpsservices.so \
	 $(PREBUILT_PATH)/system/lib/libmcphalgps.so:system/lib/libmcphalgps.so \
	 $(PREBUILT_PATH)/system/lib/libsupllocationprovider.so:system/lib/libsupllocationprovider.so \
	 $(PREBUILT_PATH)/system/lib/libsuplhelperservicejni.so:system/lib/libsuplhelperservicejni.so \
	 $(PREBUILT_PATH)/system/lib/libgps.so:system/lib/libgps.so

# OMX Libraries for 512MB RAM Ducati binary
PRODUCT_COPY_FILES += \
     $(PREBUILT_PATH)/system/etc/firmware/ducati-m3.bin:system/etc/firmware/ducati-m3.bin

# Misc Stuff
PRODUCT_COPY_FILES += \
    $(PREBUILT_PATH)/system/etc/gps.conf:system/etc/gps.conf \
    $(PREBUILT_PATH)/system/etc/gps/cert/client_keystore.bks:system/etc/gps/cert/client_keystore.bks \
    $(PREBUILT_PATH)/system/etc/gps/config/GPSCConfigFile.cfg:system/etc/gps/config/GPSCConfigFile.cfg \
    $(PREBUILT_PATH)/system/etc/gps/config/GpsConfigFile.txt:system/etc/gps/config/GpsConfigFile.txt \
    $(PREBUILT_PATH)/system/etc/gps/config/inavconfigfile.txt:system/etc/gps/config/inavconfigfile.txt \
    $(PREBUILT_PATH)/system/etc/gps/config/pathconfigfile.txt:system/etc/gps/config/pathconfigfile.txt \
    $(PREBUILT_PATH)/system/etc/gps/config/PeriodicConfFile.cfg:system/etc/gps/config/PeriodicConfFile.cfg \
    $(PREBUILT_PATH)/system/etc/gps/config/SuplConfig.spl:system/etc/gps/config/SuplConfig.spl \
    $(PREBUILT_PATH)/system/etc/gps/patch/patch-X.0.ce:system/etc/gps/patch/patch-X.0.ce


# WI-Fi Stuff
PRODUCT_COPY_FILES += \
      $(PREBUILT_PATH)/system/etc/wifi/p2p_supplicant_overlay.conf:system/etc/wifi/p2p_supplicant_overlay.conf

PRODUCT_PACKAGES := \
        Superuser \
        su 

PRODUCT_PACKAGES += \
        Trebuchet \
        Launcher3

# ArchosControlCenter Stuff
PRODUCT_PACKAGES += \
        deepsleeper \
        set3GPortState \
        ArchosControlPanel

# WI-Fi
PRODUCT_PACKAGES += \
        dhcpcd.conf \
        hostapd.conf \
        wifical.sh \
        TQS_D_1.7.ini \
        TQS_D_1.7_127x.ini \
	calibrator \
	crda \
	regulatory.bin \
	lib_driver_cmd_wl12xx \
	127x_TQS_S_2.6.ini \
	wl1271-nvs_127x.bin \
	wl127x-fw-4-mr.bin \
	wl127x-fw-4-plt.bin \
	wl127x-fw-4-sr.bin

PRODUCT_PACKAGES += \
        wlan_loader \
        wlan_cu \
        wpa_supplicant \
        wpa_supplicant.conf \
        make_ext4fs \
        com.android.future.usb.accessory

PRODUCT_PACKAGES += \
        libinvensense_mpl \
        parse_hdmi_edid \
        libedid \
        omx_tests \
        libaudioutils

PRODUCT_PACKAGES += \
        LiveWallpapers \
        LiveWallpapersPicker \
        VisualizationWallpapers \
        librs_jni \
	com.android.future.usb.accessory

PRODUCT_PACKAGES += \
	audio.a2dp.default \
        audio.r_submix.default \
	libaudioutils \
	libnetcmdiface

# Filesystem management tools
PRODUCT_PACKAGES += \
	make_ext4fs \
	setup_fs

# BlueZ test tools
PRODUCT_PACKAGES += \
	hciconfig \
	hcitool

# Bluetooth
PRODUCT_PACKAGES += \
	ti-wpan-fw

# SGX driver
PRODUCT_PACKAGES += \
	ti_omap4_sgx_libs

# Archos audio driver, based on upstream omap drivers (common-open)
PRODUCT_PACKAGES += \
	audio.hdmi.archos \
	audio.primary.archos

PRODUCT_PACKAGES += \
	tinypcm \
	tinycap \
	tinyplay \
	tinymix
	
# Boost Pulse
PRODUCT_PACKAGES += \
	power.archos \
	camera.archos \
	hwcomposer.archos

# Rild 
PRODUCT_PACKAGES += \
	radiooptions \
        hotplugd \
        usb_modeswitch \
        libusb \
	rild

PRODUCT_PROPERTY_OVERRIDES := \
	hwui.render_dirty_regions=false

PRODUCT_PROPERTY_OVERRIDES += \
	ro.sf.lcd_density=160

PRODUCT_PROPERTY_OVERRIDES += \
    wifi.interface=wlan0 \
    wifi.supplicant_scan_interval=120 \
    ro.opengles.version=131072 \
    com.ti.omap_enhancement=true \
    omap.enhancement=true \
    ro.crypto.state=unencrypted \
    persist.sys.usb.config=mtp,adb \
    persist.sys.root_access=3 \
    ro.bq.gpu_to_cpu_unsupported=1 \
    media.stagefright.cache-params=18432/20480/15
    
# Low-RAM optimizations
PRODUCT_PROPERTY_OVERRIDES += \
    ro.config.low_ram=true \
    persist.sys.force_highendgfx=true \
    dalvik.vm.jit.codecachesize=0 \
    config.disable_atlas=true \
    ro.config.max_starting_bg=8 \
    ro.sys.fw.bg_apps_limit=16
    

# Dalvik settings
ADDITIONAL_BUILD_PROPERTIES += \
    dalvik.vm.heapstartsize=5m \
    dalvik.vm.heapgrowthlimit=48m \
    dalvik.vm.heapsize=256m \
    dalvik.vm.heaptargetutilization=0.75 \
    dalvik.vm.heapminfree=512k \
    dalvik.vm.heapmaxfree=2m \
    dalvik.vm.dex2oat-filter=interpret-only \
    dalvik.vm.image-dex2oat-filter=speed \
    ro.zygote=zygote32 \
    dalvik.vm.dex2oat-Xms=64m \
    dalvik.vm.dex2oat-Xmx=300m \
    dalvik.vm.image-dex2oat-Xms=64m \
    dalvik.vm.image-dex2oat-Xmx=300m \
    ro.dalvik.vm.native.bridge=0

PRODUCT_CHARACTERISTICS := tablet

# Pick up overlay for features that depend on non-open-source files
DEVICE_PACKAGE_OVERLAYS := \
        device/archos/archos_g9/overlay

PRODUCT_TAGS += dalvik.gc.type-precise

PRODUCT_NAME := cm_archos_g9
PRODUCT_DEVICE := archos_g9
