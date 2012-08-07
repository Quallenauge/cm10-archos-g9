ifeq ($(TARGET_BOARD_PLATFORM),omap4)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

ifneq ($(TARGET_DEVICE),panda)
-include $(TOP)/vendor/widevine/proprietary/cryptoPlugin/decrypt-core.mk
endif

LOCAL_C_INCLUDES:= \
        frameworks/native/include/media/hardware \
        vendor/widevine/proprietary/cryptoPlugin \

ifeq ($(BOARD_USES_SECURE_SERVICES),true)
LOCAL_STATIC_LIBRARIES += \
        libtee_client_api_driver        \

endif

LOCAL_SHARED_LIBRARIES := \
        libstagefright_foundation       \
        liblog                          \
        libcutils                       \
        libcrypto

LOCAL_MODULE := libdrmdecrypt

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

endif
