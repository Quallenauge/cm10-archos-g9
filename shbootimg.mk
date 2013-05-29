#
# Copyright (C) 2012 The CyanogenMod Project
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

LOCAL_PATH := $(call my-dir)

##### Custom Recovery Ramdisk ######
recovery_initrc := $(call include-path-for, recovery)/etc/init.rc
recovery_uncompressed_ramdisk := $(PRODUCT_OUT)/ramdisk-recovery.cpio
recovery_build_prop := $(INSTALLED_BUILD_PROP_TARGET)
recovery_binary := $(call intermediates-dir-for,EXECUTABLES,recovery)/recovery
recovery_resources_common := $(call include-path-for, recovery)/res
recovery_resources_private := $(strip $(wildcard $(TARGET_DEVICE_DIR)/recovery/res))
recovery_root_private := $(strip $(wildcard $(TARGET_DEVICE_DIR)/recovery/root))
recovery_resource_deps := $(shell find $(recovery_resources_common) \
  $(recovery_resources_private) $(recovery_root_private) -type f)
recovery_fstab := $(strip $(wildcard $(TARGET_DEVICE_DIR)/recovery.fstab))

ifeq ($(recovery_fstab),)
  $(info No recovery.fstab for TARGET_DEVICE $(TARGET_DEVICE))
endif

INTERNAL_RECOVERY_FILES := $(filter $(TARGET_RECOVERY_OUT)/%, \
	$(foreach module, $(ALL_MODULES), $(ALL_MODULES.$(module).INSTALLED)) \
	$(ALL_DEFAULT_INSTALLED_MODULES))

# Keys authorized to sign OTA packages this build will accept.  The
# build always uses dev-keys for this; release packaging tools will
# substitute other keys for this one.
OTA_PUBLIC_KEYS := $(DEFAULT_SYSTEM_DEV_CERTIFICATE).x509.pem

# Generate a file containing the keys that will be read by the
# recovery binary.
RECOVERY_INSTALL_OTA_KEYS := \
	$(call intermediates-dir-for,PACKAGING,ota_keys)/keys
DUMPKEY_JAR := $(HOST_OUT_JAVA_LIBRARIES)/dumpkey.jar
$(RECOVERY_INSTALL_OTA_KEYS): PRIVATE_OTA_PUBLIC_KEYS := $(OTA_PUBLIC_KEYS)
$(RECOVERY_INSTALL_OTA_KEYS): extra_keys := $(patsubst %,%.x509.pem,$(PRODUCT_EXTRA_RECOVERY_KEYS))
$(RECOVERY_INSTALL_OTA_KEYS): $(OTA_PUBLIC_KEYS) $(DUMPKEY_JAR) $(extra_keys)
	@echo "DumpPublicKey: $@ <= $(PRIVATE_OTA_PUBLIC_KEYS) $(extra_keys)"
	@rm -rf $@
	@mkdir -p $(dir $@)
	java -jar $(DUMPKEY_JAR) $(PRIVATE_OTA_PUBLIC_KEYS) $(extra_keys) > $@

TARGET_RECOVERY_ROOT_TIMESTAMP := $(TARGET_RECOVERY_OUT)/root.ts

$(TARGET_RECOVERY_ROOT_TIMESTAMP): $(INTERNAL_RECOVERY_FILES) \
		$(recovery_binary) \
		$(recovery_initrc) \
		$(recovery_build_prop) $(recovery_resource_deps) \
		$(recovery_fstab) \
		$(RECOVERY_INSTALL_OTA_KEYS)
	@echo -e ${CL_CYN}"----- Making recovery filesystem ------"${CL_RST}
	$(hide) mkdir -p $(TARGET_RECOVERY_OUT)
	$(hide) mkdir -p $(TARGET_RECOVERY_ROOT_OUT)/etc $(TARGET_RECOVERY_ROOT_OUT)/tmp
	@echo -e ${CL_CYN}"Copying baseline ramdisk..."${CL_RST}
	$(hide) cp -R $(TARGET_ROOT_OUT) $(TARGET_RECOVERY_OUT)
	@echo -e ${CL_CYN}"Modifying ramdisk contents..."${CL_RST}
	$(hide) rm -f $(TARGET_RECOVERY_ROOT_OUT)/init*.rc
	$(hide) cp -f $(recovery_initrc) $(TARGET_RECOVERY_ROOT_OUT)/init.rc
	$(hide) -cp $(TARGET_ROOT_OUT)/init.recovery.*.rc $(TARGET_RECOVERY_ROOT_OUT)/
	$(hide) cp -f $(recovery_binary) $(TARGET_RECOVERY_ROOT_OUT)/sbin/
	mkdir -p $(TARGET_RECOVERY_ROOT_OUT)/system/bin
	$(hide) cp -rf $(recovery_resources_common) $(TARGET_RECOVERY_ROOT_OUT)/
	$(hide) $(foreach item,$(recovery_resources_private), \
	  cp -rf $(item) $(TARGET_RECOVERY_ROOT_OUT)/)
	$(foreach item,$(recovery_root_private), \
	  cp -rf $(item) $(TARGET_RECOVERY_OUT)/)
	$(hide) $(foreach item,$(recovery_fstab), \
	  cp -f $(item) $(TARGET_RECOVERY_ROOT_OUT)/etc/recovery.fstab)
	$(hide) cp $(RECOVERY_INSTALL_OTA_KEYS) $(TARGET_RECOVERY_ROOT_OUT)/res/keys
	$(hide) cat $(INSTALLED_DEFAULT_PROP_TARGET) $(recovery_build_prop) \
	        > $(TARGET_RECOVERY_ROOT_OUT)/default.prop
	@echo -e ${CL_YLW}"Modifying default.prop"${CL_RST}
	$(SED_INPLACE) 's/ro.build.date.utc=.*/ro.build.date.utc=0/g' $(TARGET_RECOVERY_ROOT_OUT)/default.prop
	$(SED_INPLACE) 's/ro.adb.secure=1//g' $(TARGET_RECOVERY_ROOT_OUT)/default.prop
	$(SED_INPLACE) 's/ro.service.adb.root=1//g' $(TARGET_RECOVERY_ROOT_OUT)/default.prop
	@echo -e ${CL_CYN}"----- Made recovery filesystem --------"$(TARGET_RECOVERY_ROOT_OUT)${CL_RST}
	@touch $(TARGET_RECOVERY_ROOT_TIMESTAMP)

$(recovery_uncompressed_ramdisk):  \
	$(TARGET_RECOVERY_ROOT_TIMESTAMP)
	@echo -e ${CL_CYN}"----- Making uncompressed recovery ramdisk ------"${CL_RST}
	$(MKBOOTFS) $(TARGET_RECOVERY_ROOT_OUT) > $@

INSTALLED_BOOTIMAGE_TARGET := $(PRODUCT_OUT)/boot.img

TARGET_KERNEL_MODULES := TARGET_KERNEL_BINARIES TARGET_SGX_KERNEL_BINARIES

TARGET_SGX_KERNEL_BINARIES:
	@echo -e ${CL_CYN}"----- Making sgx kernel modules ------"${CL_RST}
	$(MAKE) -C device/archos/archos_g9/pvrsrvkm/eurasiacon/build/linux2/omap4430_android/ ARCH=$(TARGET_ARCH)  KERNELDIR=$(KERNEL_OUT) $(ARM_CROSS_COMPILE) TARGET_PRODUCT="blaze_tablet" BUILD=release TARGET_SGX=540 PLATFORM_VERSION=4.0
	mv $(KERNEL_OUT)/../../target/kbuild/pvrsrvkm_sgx540_120.ko $(KERNEL_MODULES_OUT)
	$(ARM_EABI_TOOLCHAIN)/arm-eabi-strip --strip-unneeded $(KERNEL_MODULES_OUT)/pvrsrvkm_sgx540_120.ko


TARGET_KERNEL_BINARIES: $(KERNEL_OUT) $(KERNEL_CONFIG) $(KERNEL_HEADERS_INSTALL) $(recovery_uncompressed_ramdisk)
	@echo -e ${CL_CYN}"----- Making Kernel binaries -----"${CL_RST}
	$(MAKE) -C $(KERNEL_SRC) O=$(KERNEL_OUT) ARCH=$(TARGET_ARCH) $(ARM_CROSS_COMPILE) $(TARGET_PREBUILT_INT_KERNEL_TYPE)
	-$(MAKE) -C $(KERNEL_SRC) O=$(KERNEL_OUT) ARCH=$(TARGET_ARCH) $(ARM_CROSS_COMPILE) modules
	-$(MAKE) -C $(KERNEL_SRC) O=$(KERNEL_OUT) INSTALL_MOD_PATH=../../$(KERNEL_MODULES_INSTALL) ARCH=$(TARGET_ARCH) $(ARM_CROSS_COMPILE) modules_install

$(info "TARGET_KERNEL_MODULES $(TARGET_KERNEL_MODULES)")

$(INSTALLED_BOOTIMAGE_TARGET): $(INSTALLED_KERNEL_TARGET) $(INSTALLED_RAMDISK_TARGET)
	$(ACP) -fp $< $@
