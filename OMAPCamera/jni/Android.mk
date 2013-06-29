LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

COMMON_C_INCLUDES := \
        $(LOCAL_PATH)/feature_stab/db_vlvm \
        $(LOCAL_PATH)/feature_stab/src \
        $(LOCAL_PATH)/feature_stab/src/dbreg \
        $(LOCAL_PATH)/feature_mos/src \
        $(LOCAL_PATH)/feature_mos/src/mosaic

COMMON_CFLAGS := -O3 -DNDEBUG

COMMON_SRC_FILES := \
        feature_mos_jni.cpp \
        mosaic_renderer_jni.cpp \
        feature_mos/src/mosaic/trsMatrix.cpp \
        feature_mos/src/mosaic/AlignFeatures.cpp \
        feature_mos/src/mosaic/Blend.cpp \
        feature_mos/src/mosaic/Delaunay.cpp \
        feature_mos/src/mosaic/ImageUtils.cpp \
        feature_mos/src/mosaic/Mosaic.cpp \
        feature_mos/src/mosaic/Pyramid.cpp \
        feature_mos/src/mosaic_renderer/Renderer.cpp \
        feature_mos/src/mosaic_renderer/WarpRenderer.cpp \
        feature_mos/src/mosaic_renderer/SurfaceTextureRenderer.cpp \
        feature_mos/src/mosaic_renderer/YVURenderer.cpp \
        feature_mos/src/mosaic_renderer/FrameBuffer.cpp \
        feature_stab/db_vlvm/db_feature_detection.cpp \
        feature_stab/db_vlvm/db_feature_matching.cpp \
        feature_stab/db_vlvm/db_framestitching.cpp \
        feature_stab/db_vlvm/db_image_homography.cpp \
        feature_stab/db_vlvm/db_rob_image_homography.cpp \
        feature_stab/db_vlvm/db_utilities.cpp \
        feature_stab/db_vlvm/db_utilities_camera.cpp \
        feature_stab/db_vlvm/db_utilities_indexing.cpp \
        feature_stab/db_vlvm/db_utilities_linalg.cpp \
        feature_stab/db_vlvm/db_utilities_poly.cpp \
        feature_stab/src/dbreg/dbreg.cpp \
        feature_stab/src/dbreg/dbstabsmooth.cpp \
        feature_stab/src/dbreg/vp_motionmodel.c \

COMMON_SHARED_LIBRARIES := liblog libnativehelper libGLESv2
#LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -ldl -llog -lGLESv2 -L$(TARGET_OUT)

# ORIGINAL
LOCAL_C_INCLUDES := $(COMMON_C_INCLUDES)
LOCAL_CFLAGS := $(COMMON_CFLAGS)
LOCAL_SRC_FILES := $(COMMON_SRC_FILES)
LOCAL_SHARED_LIBRARIES := $(COMMON_SHARED_LIBRARIES)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE    := libjni_msc

include $(BUILD_SHARED_LIBRARY)

# CPCAM
ifeq (0,1)
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(COMMON_C_INCLUDES)
LOCAL_CFLAGS := $(COMMON_CFLAGS)
LOCAL_SRC_FILES := $(COMMON_SRC_FILES)
LOCAL_SHARED_LIBRARIES := $(COMMON_SHARED_LIBRARIES) libandroid liblog libbinder libutils libgui libui
LOCAL_LDFLAGS += -ljnigraphics
LOCAL_LDLIBS := -llog -landroid -ljnigraphics
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES += \
        process_jni.cpp \

LOCAL_MODULE    := libjni_process

include $(BUILD_SHARED_LIBRARY)
endif
