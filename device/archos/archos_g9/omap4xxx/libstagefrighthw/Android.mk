# Only applicable for OMAP4 and OMAP5 boards.
# First eliminate OMAP3 and then ensure that this is not used
# for customer boards

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    TIOMXPlugin.cpp

LOCAL_C_INCLUDES:= \
    frameworks/native/include/media/openmax \
    frameworks/native/include/media/hardware

LOCAL_SHARED_LIBRARIES := \
    libbinder \
    libutils \
    libcutils \
    libui \
    libdl

LOCAL_MODULE := libstagefrighthw.archos_g9

include $(BUILD_HEAPTRACKED_SHARED_LIBRARY)
