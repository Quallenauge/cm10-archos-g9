
ifdef OMAP_ENHANCEMENT_VTC

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := VTCTestApp.cpp

LOCAL_C_INCLUDES += \
    $(DOMX_PATH)/omx_core/inc

ifdef ANDROID_API_JB_OR_LATER
LOCAL_C_INCLUDES += \
    $(TOP)/frameworks/native/include
else
LOCAL_C_INCLUDES += \
    $(TOP)/frameworks/base/include
endif

LOCAL_SHARED_LIBRARIES := \
    libcamera_client \
    libstagefright \
    libmedia \
    libbinder \
    libcutils \
    libutils \
    liblog \
    libgui

ifdef ANDROID_API_JB_OR_LATER
LOCAL_SHARED_LIBRARIES += \
    libstagefright_foundation
endif

LOCAL_CFLAGS +=-Wall -fno-short-enums -O0 -g -D___ANDROID___ $(ANDROID_API_CFLAGS)

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS:= optional
LOCAL_MODULE := VTCTestApp
include $(BUILD_EXECUTABLE)

###############################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := VTCLoopback.cpp IOMXEncoder.cpp IOMXDecoder.cpp

LOCAL_C_INCLUDES += \
    $(DOMX_PATH)/omx_core/inc \
    $(TOP)/hardware/ti/omap4xxx/libtiutils

ifdef ANDROID_API_JB_OR_LATER
LOCAL_C_INCLUDES += \
    $(TOP)/frameworks/native/include
else
LOCAL_C_INCLUDES += \
    $(TOP)/frameworks/base/include
endif

LOCAL_SHARED_LIBRARIES := \
    libcamera_client \
    libstagefright \
    libmedia \
    libbinder \
    libtiutils \
    libcutils \
    libutils \
    liblog \
    libgui \
    libui

ifdef ANDROID_API_JB_OR_LATER
LOCAL_SHARED_LIBRARIES += \
    libstagefright_foundation
endif

LOCAL_CFLAGS +=-Wall -fno-short-enums -O0 -g $(ANDROID_API_CFLAGS)

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS:= optional
LOCAL_MODULE := VTCLoopbackTest
include $(BUILD_EXECUTABLE)

###############################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= CameraHardwareInterfaceTest.cpp

ifdef ANDROID_API_JB_OR_LATER
LOCAL_C_INCLUDES += \
    $(TOP)/frameworks/av/services/camera/libcameraservice
else
LOCAL_C_INCLUDES += \
    $(TOP)/frameworks/base/include \
    $(TOP)/frameworks/base/services/camera/libcameraservice
endif

LOCAL_SHARED_LIBRARIES:= \
    libdl \
    libui \
    libutils \
    libcutils \
    libbinder \
    libmedia \
    libui \
    libgui \
    libcamera_client \
    libhardware

LOCAL_MODULE:= CameraHardwareInterfaceTest
LOCAL_MODULE_TAGS:= tests

LOCAL_CFLAGS += -Wall -fno-short-enums -O0 -g -D___ANDROID___ $(ANDROID_API_CFLAGS)

# Add TARGET FLAG for OMAP4 and OMAP5 boards only
# First eliminate OMAP3 and then ensure that this is not used
# for customer boards.
ifneq ($(TARGET_BOARD_PLATFORM),omap3)
    ifeq ($(findstring omap, $(TARGET_BOARD_PLATFORM)),omap)
        LOCAL_CFLAGS += -DTARGET_OMAP4
    endif
endif

include $(BUILD_HEAPTRACKED_EXECUTABLE)

endif
