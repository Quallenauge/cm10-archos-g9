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
LOCAL_PATH:= $(call my-dir)
# abcbox archos binary
include $(CLEAR_VARS)
LOCAL_MODULE := abcbox
LOCAL_SRC_FILES := system/bin/abcbox
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := archos
include $(BUILD_PREBUILT)
    
ABCBOX_LINKS := acat adev aosparser autodim bat cramfschecker create_nvs create_sysid \
	    fb_write flash_partition_erase fsck.ext3_parser get_info kd_flasher \
	    kx_checker led mactool mkfs.ext3_parser reboot_into 
	    
# Make #!/system/bin/abcbox launchers for each tool.
#
SYMLINKS := $(addprefix $(TARGET_OUT)/bin/,$(ABCBOX_LINKS))
$(SYMLINKS): ABCBOX_BINARY := $(LOCAL_MODULE)
$(SYMLINKS): $(LOCAL_INSTALLED_MODULE) $(LOCAL_PATH)/Android.mk
	@echo "Symlink: $@ -> $(ABCBOX_BINARY)"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf $(ABCBOX_BINARY) $@

ALL_DEFAULT_INSTALLED_MODULES += $(SYMLINKS)

# We need this so that the installed files could be picked up based on the
# local module name
ALL_MODULES.$(LOCAL_MODULE).INSTALLED := \
    $(ALL_MODULES.$(LOCAL_MODULE).INSTALLED) $(SYMLINKS)

####### abcbox recovery version ######################

include $(CLEAR_VARS)
LOCAL_MODULE := abcbox_recovery
LOCAL_SRC_FILES := system/bin/abcbox
LOCAL_MODULE_CLASS := RECOVERY_EXECUTABLES
LOCAL_MODULE_PATH :=  $(TARGET_RECOVERY_ROOT_OUT)/sbin
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := archos
LOCAL_MODULE_STEM := abcbox
include $(BUILD_PREBUILT)

# Make #!/system/bin/abcbox launchers for each tool.
#
SYMLINKS_RECOVERY := $(addprefix $(TARGET_RECOVERY_ROOT_OUT)/sbin/,$(ABCBOX_LINKS))
$(SYMLINKS_RECOVERY): ABCBOX_BINARY := $(LOCAL_MODULE_STEM)
$(SYMLINKS_RECOVERY): $(LOCAL_INSTALLED_MODULE) $(LOCAL_PATH)/Android.mk
	@echo "Symlink: $@ -> $(ABCBOX_BINARY)"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf $(ABCBOX_BINARY) $@

ALL_DEFAULT_INSTALLED_MODULES += $(SYMLINKS_RECOVERY)

# We need this so that the installed files could be picked up based on the
# local module name
ALL_MODULES.$(LOCAL_MODULE).INSTALLED := \
    $(ALL_MODULES.$(LOCAL_MODULE).INSTALLED) $(SYMLINKS_RECOVERY)
    
#### kd_flasher utility tool #####

include $(CLEAR_VARS)
LOCAL_SRC_FILES := system/bin/abcbox
LOCAL_STATIC_LIBRARIES := libcutils libc
LOCAL_MODULE := utility_kdflasher
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_STEM := kd_flasher
LOCAL_MODULE_CLASS := UTILITY_EXECUTABLES
LOCAL_UNSTRIPPED_PATH := $(PRODUCT_OUT)/symbols/utilities
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/utilities
LOCAL_FORCE_STATIC_EXECUTABLE := true
include $(BUILD_EXECUTABLE)
