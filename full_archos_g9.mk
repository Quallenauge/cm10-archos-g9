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

##### init.usb.rc wants removing after testing in favour of the original one
##### in system/core/rootdir

#### Recovery files
PRODUCT_COPY_FILES += \
    $(PREBUILT_PATH)/root/init.recovery.archos.rc:root/init.recovery.archosa101sboard.rc \
    $(PREBUILT_PATH)/root/init.recovery.archos.rc:root/init.recovery.archosa80sboard.rc \

PRODUCT_COPY_FILES += \
    $(PREBUILT_PATH)/root/fstab.archos:root/fstab.archos \
    $(PREBUILT_PATH)/root/init.archos-common.rc:root/init.archosa80sboard.rc \
    $(PREBUILT_PATH)/root/init.archos-common.rc:root/init.archosa101sboard.rc \
    $(PREBUILT_PATH)/root/init.archosa80sboard.usb.rc:root/init.archosa80sboard.usb.rc \
    $(PREBUILT_PATH)/root/init.archosa101sboard.usb.rc:root/init.archosa101sboard.usb.rc \
    $(PREBUILT_PATH)/root/ueventd.archos.rc:root/ueventd.archosa101sboard.rc \
    $(PREBUILT_PATH)/root/ueventd.archos.rc:root/ueventd.archosa80sboard.rc

# Vold Mounting
PRODUCT_COPY_FILES += \
	$(PREBUILT_PATH)/system/etc/vold.fstab:system/etc/vold.fstab

# Key Layout/Screen Config.
PRODUCT_COPY_FILES += \
	$(PREBUILT_PATH)/system/usr/idc/cpt_i2c_tsp.idc:system/usr/idc/cpt_i2c_tsp.idc \
	$(PREBUILT_PATH)/system/usr/idc/cypress-tma340.idc:system/usr/idc/cypress-tma340.idc \
	$(PREBUILT_PATH)/system/usr/idc/pixcir_i2c_tsp.idc:system/usr/idc/pixcir_i2c_tsp.idc \
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

# xbin
PRODUCT_COPY_FILES += \
    $(PREBUILT_PATH)/system/bin/hotplugd:system/bin/hotplugd \
    $(PREBUILT_PATH)/system/xbin/chat:system/xbin/chat \
    $(PREBUILT_PATH)/system/lib/libtcl-ril.so:system/lib/libtcl-ril.so

 
    
PRODUCT_COPY_FILES += \
    $(PREBUILT_PATH)/system/etc/media_profiles.xml:system/etc/media_profiles.xml \
    $(PREBUILT_PATH)/system/etc/media_codecs.xml:system/etc/media_codecs.xml \
    frameworks/native/data/etc/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.camera.autofocus.xml:system/etc/permissions/android.hardware.camera.autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    #frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
 
# All non-essential proprietary files for Archos G9 but are still kinda important
PRODUCT_COPY_FILES += \
    $(PREBUILT_PATH)/system/bin/batteryd:system/bin/batteryd \
    $(PREBUILT_PATH)/system/bin/akmd8975:system/bin/akmd8975 \
    $(PREBUILT_PATH)/system/bin/last_kmsg:system/bin/last_kmsg \
    $(PREBUILT_PATH)/system/bin/nand:system/bin/nand \
    $(PREBUILT_PATH)/system/bin/powerd:system/bin/powerd \
    $(PREBUILT_PATH)/system/bin/watchdog_daemon:system/bin/watchdog_daemon \
    $(PREBUILT_PATH)/system/bin/navd:system/bin/navd \
    $(PREBUILT_PATH)/system/bin/start_navd:system/bin/start_navd \
    $(PREBUILT_PATH)/system/bin/rfkill_chmod.sh:system/bin/rfkill_chmod.sh \
    $(PREBUILT_PATH)/system/lib/hw/gps.archos.so:system/lib/hw/gps.omap4.so \
    $(PREBUILT_PATH)/system/lib/hw/audio_policy.archos.so:system/lib/hw/audio_policy.omap4.so \
    $(PREBUILT_PATH)/system/lib/hw/audio.primary.archos.so:system/lib/hw/audio.primary.omap4.so \
    $(PREBUILT_PATH)/system/lib/hw/sensors.omap4.so:system/lib/hw/sensors.omap4.so 


# GPS libraries 
PRODUCT_COPY_FILES += \
	 $(PREBUILT_PATH)/system/lib/libgpsservices.so:system/lib/libgpsservices.so \
	 $(PREBUILT_PATH)/system/lib/libmcphalgps.so:system/lib/libmcphalgps.so \
	 $(PREBUILT_PATH)/system/lib/libsupllocationprovider.so:system/lib/libsupllocationprovider.so \
	 $(PREBUILT_PATH)/system/lib/libsuplhelperservicejni.so:system/lib/libsuplhelperservicejni.so \
	 $(PREBUILT_PATH)/system/lib/libgps.so:system/lib/libgps.so

# OMX Libraries for 512MB RAM Ducati binary
PRODUCT_COPY_FILES += \
    $(PREBUILT_PATH)/system/etc/firmware/ducati-m3.bin:system/etc/firmware/ducati-m3.bin \
    $(PREBUILT_PATH)/system/etc/firmware/TIInit_7.2.31.bts:system/etc/firmware/TIInit_7.2.31.bts \
    $(PREBUILT_PATH)/system/etc/firmware/ti-connectivity/wl1271-fw-2.bin:system/etc/firmware/ti-connectivity/wl1271-fw-2.bin \
    $(PREBUILT_PATH)/system/etc/firmware/ti-connectivity/wl1271-fw-ap.bin:system/etc/firmware/ti-connectivity/wl1271-fw-ap.bin \
    $(PREBUILT_PATH)/system/etc/firmware/ti-connectivity/wl1271-fw.bin:system/etc/firmware/ti-connectivity/wl1271-fw.bin \
	$(PREBUILT_PATH)/system/etc/firmware/ti-connectivity/wl1271-nvs.bin:system/etc/firmware/ti-connectivity/wl1271-nvs.bin \
	$(PREBUILT_PATH)/system/etc/firmware/ti-connectivity/wl127x-fw-3.bin:system/etc/firmware/ti-connectivity/wl127x-fw-3.bin \
	$(PREBUILT_PATH)/system/etc/firmware/ti-connectivity/wl127x-fw-4-mr.bin:system/etc/firmware/ti-connectivity/wl127x-fw-4-mr.bin \
	$(PREBUILT_PATH)/system/etc/firmware/ti-connectivity/wl127x-fw-4-plt.bin:system/etc/firmware/ti-connectivity/wl127x-fw-4-plt.bin \
	$(PREBUILT_PATH)/system/etc/firmware/ti-connectivity/wl127x-fw-4-sr.bin:system/etc/firmware/ti-connectivity/wl127x-fw-4-sr.bin \
	$(PREBUILT_PATH)/system/etc/firmware/ti-connectivity/wl127x-fw-5-mr.bin:system/etc/firmware/ti-connectivity/wl127x-fw-5-mr.bin \
	$(PREBUILT_PATH)/system/etc/firmware/ti-connectivity/wl127x-fw-5-plt.bin:system/etc/firmware/ti-connectivity/wl127x-fw-5-plt.bin \
	$(PREBUILT_PATH)/system/etc/firmware/ti-connectivity/wl127x-fw-5-sr.bin:system/etc/firmware/ti-connectivity/wl127x-fw-5-sr.bin \
	$(PREBUILT_PATH)/system/etc/firmware/ti-connectivity/wl127x-fw-plt-3.bin:system/etc/firmware/ti-connectivity/wl127x-fw-plt-3.bin \
	$(PREBUILT_PATH)/system/etc/firmware/ti-connectivity/wl127x-nvs.bin:system/etc/firmware/ti-connectivity/wl127x-nvs.bin \
	$(PREBUILT_PATH)/system/etc/firmware/ti-connectivity/wl12xx-nvs.bin:system/etc/firmware/ti-connectivity/wl12xx-nvs.bin \
	hardware/ti/wlan/mac80211/ti-utils/ini_files/127x/TQS_S_2.6.ini:system/etc/wifi/TQS_S_2.6.ini

# Misc Stuff
PRODUCT_COPY_FILES += \
    $(PREBUILT_PATH)/system/etc/audio_policy.conf:system/etc/audio_policy.conf \
    $(PREBUILT_PATH)/system/etc/gps/cert/client_keystore.bks:system/etc/gps/cert/client_keystore.bks \
    $(PREBUILT_PATH)/system/etc/gps/config/GPSCConfigFile.cfg:system/etc/gps/config/GPSCConfigFile.cfg \
    $(PREBUILT_PATH)/system/etc/gps/config/GpsConfigFile.txt:system/etc/gps/config/GpsConfigFile.txt \
    $(PREBUILT_PATH)/system/etc/gps/config/inavconfigfile.txt:system/etc/gps/config/inavconfigfile.txt \
    $(PREBUILT_PATH)/system/etc/gps/config/pathconfigfile.txt:system/etc/gps/config/pathconfigfile.txt \
    $(PREBUILT_PATH)/system/etc/gps/config/PeriodicConfFile.cfg:system/etc/gps/config/PeriodicConfFile.cfg \
    $(PREBUILT_PATH)/system/etc/gps/config/SuplConfig.spl:system/etc/gps/config/SuplConfig.spl \
    $(PREBUILT_PATH)/system/etc/gps/patch/patch-X.0.ce:system/etc/gps/patch/patch-X.0.ce

# Wifi
PRODUCT_PACKAGES += \
        lib_driver_cmd_wl12xx \
        wlan_loader \
        wlan_cu \
        dhcpcd.conf \
        wpa_supplicant.conf \
        hostapd.conf \
        calibrator \
		com.android.future.usb.accessory

PRODUCT_PACKAGES += \
        libinvensense_mpl \
        parse_hdmi_edid \
        libedid \
        libaudioutils \
		com.android.future.usb.accessory
		
PRODUCT_PACKAGES += \
	PCKeyboard

PRODUCT_PACKAGES += \
	dhcpcd.conf \
	libnetcmdiface \
	calibrator

# bluetooth Packages
PRODUCT_PACKAGES += \
	libbt-vendor \
	hciconfig \
	hcitool \
	uim-sysfs

# Filesystem management tools
PRODUCT_PACKAGES += \
	setup_fs \
	
# SGX driver
PRODUCT_PACKAGES += \
	ti_omap4_sgx_libs \
	lights.omap4 \
	audio.hdmi.omap4 \
    audio.usb.default \
	audio.a2dp.default \
	#G9Parts

PRODUCT_PROPERTY_OVERRIDES += \
    wifi.interface=wlan0 \
    wifi.supplicant_scan_interval=120 \
    hwui.render_dirty_regions=false \
    ro.opengles.version=131072 \
    com.ti.omap_enhancement=true \
    omap.enhancement=true \
    ro.crypto.state=unencrypted \
    persist.sys.usb.config=mtp,adb \
    persist.sys.root_access=3 \
    persist.hwc.sw_vsync=1 \
	ro.sf.lcd_density=160

#adb has changed his security model since 4.2.2, don't use it (for now)
ADDITIONAL_DEFAULT_PROPERTIES := ro.adb.secure=0
# adb has root
ADDITIONAL_DEFAULT_PROPERTIES += ro.secure=0
#
ADDITIONAL_DEFAULT_PROPERTIES += persist.sys.usb.config=mtp

# Pick up overlay for features that depend on non-open-source files
DEVICE_PACKAGE_OVERLAYS := \
        device/archos/archos_g9/overlay

PRODUCT_TAGS += dalvik.gc.type-precise

PRODUCT_NAME := cm_archos_g9
PRODUCT_DEVICE := archos_g9
