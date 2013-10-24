LOCAL_PATH := $(my-dir)
img-sgx.untarred_intermediates := $(call intermediates-dir-for, FAKE, img-sgx.untarred)
img-sgx.untarred_timestamp := $(img-sgx.untarred_intermediates)/stamp
img-sgx.untarred_bin := $(img-sgx.untarred_intermediates)/sgx
img-sgx.tar := $(LOCAL_PATH)/$(TARGET_BOARD_PLATFORM)/sgx.tgz

include $(call all-subdir-makefiles)

$(img-sgx.untarred_timestamp) : $(img-sgx.tar)
	@echo "Unzip $(dir $@) <- $<"
	$(hide) rm -rf $(dir $@) && mkdir -p $(dir $@)
	$(hide) tar -C $(dir $@) -xzf $<
	$(hide) touch $@

