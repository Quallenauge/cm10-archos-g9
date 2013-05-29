LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_DEVICE),archos_g9)

# Edify extension functions for doing modem (radio) updates on Toro devices.

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES += bootable/recovery
LOCAL_SRC_FILES := recovery_updater.c
ifeq ($(HAVE_SELINUX), true)
LOCAL_C_INCLUDES += external/libselinux/include
LOCAL_STATIC_LIBRARIES += libselinux
LOCAL_CFLAGS += -DHAVE_SELINUX
endif # HAVE_SELINUX
LOCAL_C_INCLUDES += system/extras/ext4_utils
LOCAL_STATIC_LIBRARIES += \
   libext4_utils_static \
# should match TARGET_RECOVERY_UPDATER_LIBS set in BoardConfig.mk
LOCAL_MODULE := librecovery_updater_archosg9

include $(BUILD_STATIC_LIBRARY)

endif
