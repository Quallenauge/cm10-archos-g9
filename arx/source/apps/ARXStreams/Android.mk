ifneq ($(TARGET_ANDROID_VERSION),JELLYBEANMR1)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_PACKAGE_NAME := ARXStreams
LOCAL_JNI_SHARED_LIBRARIES := libarxstreams_jni
LOCAL_REQUIRED_MODULES := libarxstreams_jni
LOCAL_PROGUARD_FLAG_FILES := proguard.flags
include $(BUILD_PACKAGE)

include $(call all-makefiles-under, $(LOCAL_PATH))
endif
