# Only applicable for OMAP4 and OMAP5 boards.
# First eliminate OMAP3 and then ensure that this is not used
# for customer boards
ifneq ($(TARGET_BOARD_PLATFORM),omap3)
ifeq ($(findstring omap, $(TARGET_BOARD_PLATFORM)),omap)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    ColorConvert.cpp

LOCAL_C_INCLUDES:= \
        $(TOP)/frameworks/native/include/media/openmax \
        $(TOP)/frameworks/native/include/media/editor

LOCAL_SHARED_LIBRARIES :=       \

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libI420colorconvert

include $(BUILD_HEAPTRACKED_SHARED_LIBRARY)

endif
endif
