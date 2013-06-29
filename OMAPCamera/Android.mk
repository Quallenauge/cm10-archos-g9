ifdef OMAP_ENHANCEMENT_CPCAM
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := CameraOMAP4
#LOCAL_SDK_VERSION := current

LOCAL_JNI_SHARED_LIBRARIES := libjni_msc

LOCAL_REQUIRED_MODULES := libjni_msc

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

LOCAL_JAVA_LIBRARIES := com.ti.omap.android.cpcam

include $(BUILD_PACKAGE)

ifeq ($(strip $(LOCAL_PACKAGE_OVERRIDES)),)
# Use the following include to make our test apk.
include $(call all-makefiles-under, $(LOCAL_PATH))
endif
endif
