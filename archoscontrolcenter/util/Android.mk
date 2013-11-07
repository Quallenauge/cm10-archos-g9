LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := set3GPortState
LOCAL_SRC_FILES := set3GPortState
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
include $(BUILD_PREBUILT)

#prebuilt_files :=
#$(call add-prebuilt-files, EXECUTABLES, $(prebuilt_files))
