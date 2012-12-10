# Copyright (C) 2010 The Android Open Source Project
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

# All non-essential proprietary files for Archos G9 but are still kinda important
PRODUCT_COPY_FILES := \
    device/archos/archos_g9/proprietary/bin/batteryd:/system/bin/batteryd \
    device/archos/archos_g9/proprietary/bin/akmd8975:/system/bin/akmd8975 \
    device/archos/archos_g9/proprietary/bin/last_kmsg:/system/bin/last_kmsg \
    device/archos/archos_g9/proprietary/bin/nand:/system/bin/nand \
    device/archos/archos_g9/proprietary/bin/powerd:/system/bin/powerd \
    device/archos/archos_g9/proprietary/bin/sharesd:/system/bin/sharesd \
    device/archos/archos_g9/proprietary/bin/uim-sysfs:/system/bin/uim-sysfs \
    device/archos/archos_g9/proprietary/bin/watchdog_daemon:/system/bin/watchdog_daemon \
    device/archos/archos_g9/proprietary/bin/navd:/system/bin/navd \
    device/archos/archos_g9/proprietary/bin/rfkill_chmod.sh:/system/bin/rfkill_chmod.sh \
    device/archos/archos_g9/proprietary/lib/libedid.so:/system/lib/libedid.so \
    device/archos/archos_g9/proprietary/lib/libgps.so:/system/lib/libgps.so \
    device/archos/archos_g9/proprietary/lib/libgpsservices.so:/system/lib/libgpsservices.so \
    device/archos/archos_g9/proprietary/lib/libmcphalgps.so:/system/lib/libmcphalgps.so \
    device/archos/archos_g9/proprietary/lib/libsupllocationprovider.so:/system/lib/libsupllocationprovider.so \
    device/archos/archos_g9/proprietary/lib/libtiutils.so:/system/lib/libtiutils.so \
    device/archos/archos_g9/proprietary/lib/hw/audio.primary.archos.so:/system/lib/hw/audio.primary.archos.so \
    device/archos/archos_g9/proprietary/lib/hw/audio_policy.archos.so:/system/lib/hw/audio_policy.archos.so \
    device/archos/archos_g9/proprietary/lib/hw/gps.archos.so:/system/lib/hw/gps.archos.so \
    device/archos/archos_g9/proprietary/lib/hw/lights.archos.so:/system/lib/hw/lights.archos.so \
    device/archos/archos_g9/proprietary/lib/hw/sensors.omap4.so:/system/lib/hw/sensors.omap4.so 

#    device/archos/archos_g9/proprietary/lib/hw/hwcomposer.omap4.so:/system/vendor/lib/hw/hwcomposer.omap4.so \
#    device/archos/archos_g9/proprietary/lib/hw/camera.omap4.so:/system/lib/hw/camera.omap4.so \
#    device/archos/archos_g9/proprietary/lib/libskia.so:/system/lib/libskia.so

# OMX Libraries for 512MB RAM Ducati binary
#PRODUCT_COPY_FILES += \
#    device/archos/archos_g9/proprietary/lib/libOMX.TI.DUCATI1.MISC.SAMPLE.so:/system/lib/libOMX.TI.DUCATI1.MISC.SAMPLE.so \
#    device/archos/archos_g9/proprietary/lib/libOMX.TI.DUCATI1.VIDEO.CAMERA.so:/system/lib/libOMX.TI.DUCATI1.VIDEO.CAMERA.so \
#    device/archos/archos_g9/proprietary/lib/libOMX.TI.DUCATI1.VIDEO.DECODER.secure.so:/system/lib/libOMX.TI.DUCATI1.VIDEO.DECODER.secure.so \
#    device/archos/archos_g9/proprietary/lib/libOMX.TI.DUCATI1.VIDEO.DECODER.so:/system/lib/libOMX.TI.DUCATI1.VIDEO.DECODER.so \
#    device/archos/archos_g9/proprietary/lib/libOMX.TI.DUCATI1.VIDEO.H264E.so:/system/lib/libOMX.TI.DUCATI1.VIDEO.H264E.so \
#    device/archos/archos_g9/proprietary/lib/libOMX.TI.DUCATI1.VIDEO.MPEG4E.so:/system/lib/libOMX.TI.DUCATI1.VIDEO.MPEG4E.so \
#    device/archos/archos_g9/proprietary/lib/libOMX_Core.so:/system/lib/libOMX_Core.so

#    device/archos/archos_g9/proprietary/lib/libdomx.so:/system/lib/libdomx.so 

# Misc Stuff
PRODUCT_COPY_FILES += \
    device/archos/archos_g9/proprietary/etc/gps/cert/client_keystore.bks:/system/etc/gps/cert/client_keystore.bks \
    device/archos/archos_g9/proprietary/etc/gps/config/GPSCConfigFile.cfg:/system/etc/gps/config/GPSCConfigFile.cfg \
    device/archos/archos_g9/proprietary/etc/gps/config/GpsConfigFile.txt:/system/etc/gps/config/GpsConfigFile.txt \
    device/archos/archos_g9/proprietary/etc/gps/config/inavconfigfile.txt:/system/etc/gps/config/inavconfigfile.txt \
    device/archos/archos_g9/proprietary/etc/gps/config/pathconfigfile.txt:/system/etc/gps/config/pathconfigfile.txt \
    device/archos/archos_g9/proprietary/etc/gps/config/PeriodicConfFile.cfg:/system/etc/gps/config/PeriodicConfFile.cfg \
    device/archos/archos_g9/proprietary/etc/gps/config/SuplConfig.spl:/system/etc/gps/config/SuplConfig.spl \
    device/archos/archos_g9/proprietary/etc/gps/patch/patch-X.0.ce:/system/etc/gps/patch/patch-X.0.ce
