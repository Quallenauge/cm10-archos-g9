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

# WARNING: Everything listed here will be built on ALL platforms,
# including x86, the emulator, and the SDK.  Modules must be uniquely
# named (liblights.panda), and must build everywhere, or limit themselves
# to only building on ARM if they include assembly. Individual makefiles
# are responsible for having their own logic, for fine-grained control.

# Set up variables
LOCAL_PATH := $(call my-dir)

#modified_build_prop_remove += ro.product.board=archos
#modified_build_prop := $(PRODUCT_OUT)/system/build.prop
#$(modified_build_prop): $(INSTALLED_BUILD_PROP_TARGET)
#	@echo -e ${CL_CYN}"----- Modifying build.prop ------"${CL_RST}
#	$(hide) grep -v "$(subst $(space),\|,$(strip \
#				$(modified_build_prop_remove)))" $< > $@.tmp
#	$(hide) for x in $(modified_build_prop_remove); do \
#				echo "$$x"generic >> $@.tmp; done
#	$(hide) mv $@.tmp $@
#
#include $ (CLEAR_VARS)
#LOCAL_MODULE := modified_build_prop
#LOCAL_MODULE_TAGS := optional 
#LOCAL_SRC_FILES := 
#L#OCAL_REQUIRED_MODULES := $(modified_build_prop)
#include $(BUILD_PHONY_PACKAGE)
#include $ (CLEAR_VARS)

#PREBUILT_PATH := $(call my-dir)/prebuilt
#VENDOR_PATH := vendor/archos/archos_g9

include $(call all-makefiles-under,$(LOCAL_PATH))

