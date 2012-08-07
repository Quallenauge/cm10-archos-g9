# Copyright (C) 2011 The Android Open Source Project
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
# This file is the build configuration for a full Android
# build for G9 hardware. This cleanly combines a set of
# device-specific aspects (drivers) with a device-agnostic
# product configuration (apps). Except for a few implementation
# details, it only fundamentally contains two inherit-product
# lines, full and G9, hence its name.
#

# Boot animation
TARGET_BOOTANIMATION_NAME := horizontal-1024x768

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)
$(call inherit-product, device/archos/archos_g9/full_archos_g9.mk)
$(call inherit-product, device/archos/archos_g9/usb_modeswitch.mk)
$(call inherit-product, vendor/archos/archos_g9/archos_g9-vendor.mk)
$(call inherit-product, frameworks/native/build/tablet-dalvik-heap.mk)
$(call inherit-product, vendor/cm/config/common_full_tablet_wifionly.mk)
$(call inherit-product-if-exists, vendor/ti/proprietary/omap4xxx/ti-omap4-vendor.mk)
$(call inherit-product, hardware/ti/omap4xxx/omap4.mk)
$(call inherit-product, hardware/ti/omap4xxx/ti-wpan-products.mk)

# About our tablet
PRODUCT_NAME := cm_archos_g9
PRODUCT_DEVICE := archos_g9
PRODUCT_BRAND := Android
PRODUCT_MODEL := Archos G9
PRODUCT_MANUFACTURER := Archos
