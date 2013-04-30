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

# abcbox archos binary

ABCBOX_BINARY := /system/bin/abcbox 

PRODUCT_COPY_FILES += \
    $(PREBUILT_PATH)/system/bin/abcbox:$(ABCBOX_BINARY)
    
ALL_ABCBOX := acat adev aosparser autodim bat cramfschecker create_nvs create_sysid \
	    fb_write flash_partition_erase fsck.ext3_parser get_info kd_flasher \
	    kx_checker led mactool mkfs.ext3_parser reboot_into 
	    

#$(info ABCBOX ABCBOX)

ABCBOX_SYMLINKS := $(addprefix $(TARGET_OUT)/bin/,$(ALL_ABCBOX))
$(ABCBOX_SYMLINKS): ABCBOX_BINARY := /system/bin/abcbox 
$(ABCBOX_SYMLINKS): $(ABCBOX_BINARY) 
	@echo "Symlink: $@ -> $(ABCBOX_BINARY)"
	@mkdir -p $(dir $@)
	@rm -rf $@
	@echo ln -sf $(ABCBOX_BINARY) $@
#	$(info ABCBOX $(ABCBOX_BINARY) )