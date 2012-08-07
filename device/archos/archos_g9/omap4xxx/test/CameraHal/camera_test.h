#ifndef CAMERA_TEST_H
#define CAMERA_TEST_H

#ifdef ANDROID_API_JB_OR_LATER
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#else
#include <surfaceflinger/Surface.h>
#include <surfaceflinger/ISurface.h>
#include <surfaceflinger/ISurfaceComposer.h>
#include <surfaceflinger/ISurfaceComposerClient.h>
#include <surfaceflinger/SurfaceComposerClient.h>
#endif

#ifdef ANDROID_API_JB_OR_LATER
#   define CAMHAL_LOGV            ALOGV
#   define CAMHAL_LOGE            ALOGE
#   define PRINTOVER(arg...)      ALOGD(#arg)
#   define LOG_FUNCTION_NAME      ALOGD("%d: %s() ENTER", __LINE__, __FUNCTION__);
#   define LOG_FUNCTION_NAME_EXIT ALOGD("%d: %s() EXIT", __LINE__, __FUNCTION__);
#else
#   define CAMHAL_LOGV            LOGV
#   define CAMHAL_LOGE            LOGE
#   define PRINTOVER(arg...)      LOGD(#arg)
#   define LOG_FUNCTION_NAME      LOGD("%d: %s() ENTER", __LINE__, __FUNCTION__);
#   define LOG_FUNCTION_NAME_EXIT LOGD("%d: %s() EXIT", __LINE__, __FUNCTION__);
#endif

#define KEY_GBCE            "gbce"
#define KEY_GLBCE           "glbce"
#define KEY_CAMERA          "camera-index"
#define KEY_SATURATION      "saturation"
#define KEY_BRIGHTNESS      "brightness"
#define KEY_TI_BURST           "burst-capture"
#define KEY_EXPOSURE        "exposure"
#define KEY_CONTRAST        "contrast"
#define KEY_SHARPNESS       "sharpness"
#define KEY_ISO             "iso"
#define KEY_CAF             "caf"
#define KEY_MODE            "mode"
#define KEY_VNF             "vnf"
#define KEY_VSTAB           "vstab"
#define KEY_COMPENSATION    "exposure-compensation"
#define KEY_SENSOR_ORIENTATION "sensor-orientation"

#define KEY_IPP             "ipp"

#define KEY_BUFF_STARV      "buff-starvation"
#define KEY_METERING_MODE   "meter-mode"
#define KEY_AUTOCONVERGENCE "auto-convergence-mode"
#define KEY_MANUAL_CONVERGENCE "manual-convergence"
#define KEY_EXP_BRACKETING_RANGE "exp-bracketing-range"
#define KEY_EXP_GAIN_BRACKETING_RANGE "exp-gain-bracketing-range"
#define KEY_TEMP_BRACKETING "temporal-bracketing"
#define KEY_TEMP_BRACKETING_POS "temporal-bracketing-range-positive"
#define KEY_TEMP_BRACKETING_NEG "temporal-bracketing-range-negative"
#define KEY_MEASUREMENT "measurement"
#define KEY_S3D2D_PREVIEW_MODE "s3d2d-preview"
#define KEY_S3D_PRV_FRAME_LAYOUT "s3d-prv-frame-layout"
#define KEY_S3D_CAP_FRAME_LAYOUT "s3d-cap-frame-layout"
#define KEY_EXIF_MODEL "exif-model"
#define KEY_EXIF_MAKE "exif-make"
#define KEY_AF_TIMEOUT "af-timeout"

#define KEY_AUTO_EXPOSURE_LOCK "auto-exposure-lock"
#define KEY_AUTO_WHITEBALANCE_LOCK "auto-whitebalance-lock"

#define KEY_MECHANICAL_MISALIGNMENT_CORRECTION "mechanical-misalignment-correction"

//TI extensions for enable/disable algos
#define KEY_ALGO_FIXED_GAMMA            "ti-algo-fixed-gamma"
#define KEY_ALGO_NSF1                   "ti-algo-nsf1"
#define KEY_ALGO_NSF2                   "ti-algo-nsf2"
#define KEY_ALGO_SHARPENING             "ti-algo-sharpening"
#define KEY_ALGO_THREELINCOLORMAP       "ti-algo-threelinecolormap"
#define KEY_ALGO_GIC                    "ti-algo-gic"

#define BRACKETING_IDX_DEFAULT          0
#define BRACKETING_IDX_STREAM           1
#define BRACKETING_STREAM_BUFFERS       9

#define SDCARD_PATH "/sdcard/"

#define MAX_BURST   15
#define BURST_INC     5
#define TEMP_BRACKETING_MAX_RANGE 4

#define MEDIASERVER_DUMP "procmem -w $(ps | grep mediaserver | grep -Eo '[0-9]+' | head -n 1) | grep \"\\(Name\\|libcamera.so\\|libOMX\\|libomxcameraadapter.so\\|librcm.so\\|libnotify.so\\|libipcutils.so\\|libipc.so\\|libsysmgr.so\\|TOTAL\\)\""
#define MEMORY_DUMP "procrank -u"
#define KEY_METERING_MODE   "meter-mode"

#define TEST_FOCUS_AREA "(-1000,500,-500,1000,1000),(500,500,1000,1000,1)"
#define TEST_METERING_AREA "(-1000,500,-500,1000,1000),(500,500,1000,1000,1)"
#define TEST_METERING_AREA_CENTER "(-250,-250,250,250,1000)"
#define TEST_METERING_AREA_AVERAGE "(-1000,-1000,1000,1000,1000)"

#define COMPENSATION_OFFSET 20
#define DELIMITER           "|"

#define MODEL "camera_test"
#define MAKE "camera_test"

#define BLAZE 0
#define BLAZE_TABLET1 1
#define BLAZE_TABLET2 2

#define MAX_LINES    80
#define MAX_SYMBOLS  65

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

typedef enum test_type {
    TEST_TYPE_REGRESSION,
    TEST_TYPE_FUNCTIONAL,
    TEST_TYPE_API,
    TEST_TYPE_ERROR,
} test_type_t;

typedef enum param_ExpBracketParamType_t {
    PARAM_EXP_BRACKET_PARAM_NONE,
    PARAM_EXP_BRACKET_PARAM_COMP,
    PARAM_EXP_BRACKET_PARAM_PAIR,
} param_ExpBracketParamType;

typedef enum param_ExpBracketValueType_t {
    PARAM_EXP_BRACKET_VALUE_NONE,
    PARAM_EXP_BRACKET_VALUE_ABS,
    PARAM_EXP_BRACKET_VALUE_REL,
} param_ExpBracketValueType;

typedef enum param_ExpBracketApplyType_t {
    PARAM_EXP_BRACKET_APPLY_NONE,
    PARAM_EXP_BRACKET_APPLY_ADJUST,
    PARAM_EXP_BRACKET_APPLY_FORCED,
} param_ExpBracketApplyType;

enum logging {
    LOGGING_LOGCAT = 0x01,
    LOGGING_SYSLINK = 0x02
};

typedef struct cmd_args {
    test_type_t test_type;
    const char *script_file_name;
    const char *output_path;
    int platform_id;
    int logging;
} cmd_args_t;

namespace android {
    class CameraHandler: public CameraListener {
        public:
            virtual void notify(int32_t msgType, int32_t ext1, int32_t ext2);
            virtual void postData(int32_t msgType,
                                  const sp<IMemory>& dataPtr,
                                  camera_frame_metadata_t *metadata);

            virtual void postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr);
    };

};

using namespace android;


typedef struct pixel_format_t {
    int32_t pixelFormatDesc;
    const char *pixformat;
}pixel_format;

typedef struct output_format_t {
    output_format type;
    const char *desc;
} outformat;

typedef struct Vcapture_size_t {
    int width, height;
    const char *desc;
} Vcapture_size;

typedef struct param_Array_t {
    int width, height;
    char name[60];
} param_Array;

typedef struct video_Codecs_t {
    video_encoder type;
    const char *desc;
} video_Codecs;

typedef struct audio_Codecs_t {
    audio_encoder type;
    const char *desc;
} audio_Codecs;

typedef struct V_bitRate_t {
    uint32_t bit_rate;
    const char *desc;
} V_bitRate;

typedef struct zoom_t {
    int idx;
    const char *zoom_description;
} Zoom;

typedef struct fps_ranges_t {
    int rangeMin;
    int rangeMax;
} fps_Array;

typedef struct buffer_info {
    int size;
    int width;
    int height;
    int format;
    sp<GraphicBuffer> buf;
} buffer_info_t;

typedef struct param_NamedExpBracketList_t {
    const char *desc;
    param_ExpBracketParamType_t param_type;
    param_ExpBracketValueType_t value_type;
    param_ExpBracketApplyType_t apply_type;
    const char *value;
} param_NamedExpBracketList;


char * get_cycle_cmd(const char *aSrc);
void trim_script_cmd(char *cmd);
int execute_functional_script(char *script);
status_t dump_mem_status();
int openCamera();
int closeCamera();
void createBufferOutputSource();
void initDefaults();
void setDefaultExpGainPreset(ShotParameters &params, int idx);
void setSingleExpGainPreset(ShotParameters &params, int idx, int exp, int gain);
void setExpGainPreset(ShotParameters &params, const char *input, bool force, param_ExpBracketParamType_t type, bool flush);
void calcNextSingleExpGainPreset(int idx, int &exp, int &gain);
void updateShotConfigFlushParam();
int startPreview();
void stopPreview();
int startRecording();
int stopRecording();
int closeRecorder();
int openRecorder();
int configureRecorder();
void printSupportedParams();
char *load_script(const char *config);
int start_logging(int flags, int &pid);
int stop_logging(int flags, int &pid);
int execute_error_script(char *script);
int getParametersFromCapabilities();
void  getSizeParametersFromCapabilities();
int getDefaultParameter(const char* val, int numOptions, char **array);
int getDefaultParameterResol(const char* val, int numOptions, param_Array  **array);
int getSupportedParameters(char* parameters, int* optionsCount, char ***elem);
int getSupportedParametersCaptureSize(char* parameters, int *optionsCount, param_Array array[], int arraySize);
int getSupportedParametersVideoCaptureSize(char* parameters, int *optionsCount, param_Array array[], int arraySize);
int getSupportedParametersPreviewSize(char* parameters, int *optionsCount, param_Array array[], int arraySize);
int getSupportedParametersThumbnailSize(char* parameters, int *optionsCount, param_Array array[], int arraySize);
int getSupportedParametersNames(int width, int height, param_Array array[], int arraySize);
int checkSupportedParamScript(char **array, int size, char *param);
int checkSupportedParamScriptLayout(char **array, int size, char *param,int *index);
int checkSupportedParamScriptResol(param_Array **array, int size, char *param, int *num);
int getSupportedParametersfps(char* parameters, int *optionsCount);
int checkSupportedParamScriptfpsConst(int *array, int size, char *param, int *num);
int checkSupportedParamScriptfpsRange(char **array, int size, char *param, int *num);
int trySetVideoStabilization(bool toggle);
int trySetVideoNoiseFilter(bool toggle);
int trySetAutoExposureLock(bool toggle);
int trySetAutoWhiteBalanceLock(bool toggle);
bool isRawPixelFormat (const char *format);
int deleteAllocatedMemory();
void initDefaultsSec();

const char KEY_S3D_PRV_FRAME_LAYOUT_VALUES[] = "s3d-prv-frame-layout-values";
const char KEY_S3D_CAP_FRAME_LAYOUT_VALUES[] = "s3d-cap-frame-layout-values";

const char KEY_SUPPORTED_PICTURE_TOPBOTTOM_SIZES[] = "supported-picture-topbottom-size-values";
const char KEY_SUPPORTED_PREVIEW_TOPBOTTOM_SIZES[] = "supported-preview-topbottom-size-values";
const char KEY_SUPPORTED_PICTURE_SIDEBYSIDE_SIZES[] = "supported-picture-sidebyside-size-values";
const char KEY_SUPPORTED_PREVIEW_SIDEBYSIDE_SIZES[] = "supported-preview-sidebyside-size-values";
const char KEY_SUPPORTED_PICTURE_SUBSAMPLED_SIZES[] = "supported-picture-subsampled-size-values";
const char KEY_SUPPORTED_PREVIEW_SUBSAMPLED_SIZES[] = "supported-preview-subsampled-size-values";

const char KEY_AUTOCONVERGENCE_MODE[] = "auto-convergence-mode";
const char KEY_AUTOCONVERGENCE_MODE_VALUES[] = "auto-convergence-mode-values";

const char KEY_MANUAL_EXPOSURE[] = "manual-exposure";
const char KEY_MANUAL_GAIN_ISO[] = "manual-gain-iso";
const char KEY_MANUAL_EXPOSURE_RIGHT[] = "manual-exposure-right";
const char KEY_MANUAL_GAIN_ISO_RIGHT[] = "manual-gain-iso-right";

const char KEY_SUPPORTED_MANUAL_CONVERGENCE_MIN[] = "supported-manual-convergence-min";
const char KEY_SUPPORTED_MANUAL_CONVERGENCE_MAX[] = "supported-manual-convergence-max";
const char KEY_SUPPORTED_MANUAL_CONVERGENCE_STEP[] = "supported-manual-convergence-step";

const char KEY_SUPPORTED_MANUAL_EXPOSURE_MIN[] = "supported-manual-exposure-min";
const char KEY_SUPPORTED_MANUAL_EXPOSURE_MAX[] = "supported-manual-exposure-max";
const char KEY_SUPPORTED_MANUAL_EXPOSURE_STEP[] = "supported-manual-exposure-step";

const char KEY_SUPPORTED_MANUAL_GAIN_ISO_MIN[] = "supported-manual-gain-iso-min";
const char KEY_SUPPORTED_MANUAL_GAIN_ISO_MAX[] = "supported-manual-gain-iso-max";
const char KEY_SUPPORTED_MANUAL_GAIN_ISO_STEP[] = "supported-manual-gain-iso-step";

class FrameWaiter : public SurfaceTexture::FrameAvailableListener {
public:
    FrameWaiter():
            mPendingFrames(0) {
    }

    virtual ~FrameWaiter() {
        onFrameAvailable();
    }

    void waitForFrame() {
        Mutex::Autolock lock(mMutex);
        while (mPendingFrames == 0) {
            mCondition.wait(mMutex);
        }
        mPendingFrames--;
    }

    virtual void onFrameAvailable() {
        Mutex::Autolock lock(mMutex);
        mPendingFrames++;
        mCondition.signal();
    }

    int mPendingFrames;
    Mutex mMutex;
    Condition mCondition;
};

class GLSurface {
public:

    GLSurface():
            mEglDisplay(EGL_NO_DISPLAY),
            mEglSurface(EGL_NO_SURFACE),
            mEglContext(EGL_NO_CONTEXT) {
    }

    virtual ~GLSurface() {}

    void initialize(int display);
    void deinit();
    void loadShader(GLenum shaderType, const char* pSource, GLuint* outShader);
    void createProgram(const char* pVertexSource, const char* pFragmentSource,
            GLuint* outPgm);

private:
    EGLint const* getConfigAttribs();
    EGLint const* getContextAttribs();

protected:
    sp<SurfaceComposerClient> mComposerClient;
    sp<SurfaceControl> mSurfaceControl;

    EGLDisplay mEglDisplay;
    EGLSurface mEglSurface;
    EGLContext mEglContext;
    EGLConfig  mGlConfig;
};

class SurfaceTextureBase  {
public:
    virtual ~SurfaceTextureBase() {}

    void initialize(int tex_id, EGLenum tex_target = EGL_NONE);
    void deinit();

    virtual sp<SurfaceTexture> getST();

protected:
    sp<SurfaceTexture> mST;
    sp<SurfaceTextureClient> mSTC;
    sp<ANativeWindow> mANW;
    int mTexId;
};

class SurfaceTextureGL : public GLSurface, public SurfaceTextureBase {
public:
    virtual ~SurfaceTextureGL() {}

    void initialize(int display, int tex_id);
    void deinit();

    // drawTexture draws the SurfaceTexture over the entire GL viewport.
    void drawTexture();

private:
    GLuint mPgm;
    GLint mPositionHandle;
    GLint mTexSamplerHandle;
    GLint mTexMatrixHandle;
};

class BufferSourceThread : public Thread {
public:
    class Defer : public Thread {
        private:
            struct DeferContainer {
                sp<GraphicBuffer> graphicBuffer;
                uint8_t *mappedBuffer;
                unsigned int count;
            };
        public:
            Defer(BufferSourceThread* bst) :
                    Thread(false), mBST(bst), mExiting(false) { }
            virtual ~Defer() {
                Mutex::Autolock lock(mFrameQueueMutex);
                mExiting = true;
                while (!mDeferQueue.isEmpty()) {
                    DeferContainer defer = mDeferQueue.itemAt(0);
                    defer.graphicBuffer->unlock();
                    mDeferQueue.removeAt(0);
                }
                mFrameQueueCondition.signal();
            }

            virtual void requestExit() {
                Thread::requestExit();

                mExiting = true;
                mFrameQueueCondition.signal();
            }

            virtual bool threadLoop() {
                Mutex::Autolock lock(mFrameQueueMutex);
                while (mDeferQueue.isEmpty() && !mExiting) {
                    mFrameQueueCondition.wait(mFrameQueueMutex);
                }

                if (!mExiting) {
                    DeferContainer defer = mDeferQueue.itemAt(0);
                    printf ("=== handling buffer %d\n", defer.count);
                    mBST->handleBuffer(defer.graphicBuffer, defer.mappedBuffer, defer.count);
                    defer.graphicBuffer->unlock();
                    mDeferQueue.removeAt(0);
                    return true;
                }
                return false;
            }
            void add(sp<GraphicBuffer> &gbuf, unsigned int count) {
                Mutex::Autolock lock(mFrameQueueMutex);
                DeferContainer defer;
                defer.graphicBuffer = gbuf;
                defer.count = count;
                gbuf->lock(GRALLOC_USAGE_SW_READ_RARELY, (void**) &defer.mappedBuffer);
                mDeferQueue.add(defer);
                mFrameQueueCondition.signal();
            }
        private:
            Vector<DeferContainer> mDeferQueue;
            Mutex mFrameQueueMutex;
            Condition mFrameQueueCondition;
            BufferSourceThread* mBST;
            bool mExiting;
    };
public:
    BufferSourceThread(bool display, int tex_id, sp<Camera> camera) :
                 Thread(false), mCounter(0), mDisplayable(display),
                 mTexId(tex_id), mCamera(camera), kReturnedBuffersMaxCapacity(6),
                 mDestroying(false), mRestartCapture(false),
                 mExpBracketIdx(BRACKETING_IDX_DEFAULT), mExp(0), mGain(0) {
        mSurfaceTextureBase = new SurfaceTextureBase();
        mSurfaceTextureBase->initialize(mTexId);
        mSurfaceTexture = mSurfaceTextureBase->getST();
        mSurfaceTexture->setSynchronousMode(true);
        mFW = new FrameWaiter();
        mSurfaceTexture->setFrameAvailableListener(mFW);
        mDeferThread = new Defer(this);
        mDeferThread->run();
    }
    virtual ~BufferSourceThread() {
        mDestroying = true;

        mSurfaceTextureBase->deinit();
        delete mSurfaceTextureBase;
        for (unsigned int i = 0; i < mReturnedBuffers.size(); i++) {
            buffer_info_t info = mReturnedBuffers.itemAt(i);
            mReturnedBuffers.removeAt(i);
        }
        mDeferThread->requestExit();
        mDeferThread.clear();
    }

    virtual bool threadLoop() {
        sp<GraphicBuffer> graphic_buffer;

        mFW->waitForFrame();
        if (!mDestroying) {
            mSurfaceTexture->updateTexImage();
            printf("=== Metadata for buffer %d ===\n", mCounter);
            showMetadata(mSurfaceTexture->getMetadata());
            printf("\n");
            graphic_buffer = mSurfaceTexture->getCurrentBuffer();
            mDeferThread->add(graphic_buffer, mCounter++);
            Mutex::Autolock lock(mToggleStateMutex);
            if (mRestartCapture) {
                ShotParameters shotParams;
                calcNextSingleExpGainPreset(mExpBracketIdx, mExp, mGain),
                setSingleExpGainPreset(shotParams, mExpBracketIdx, mExp, mGain);
                mCamera->takePicture(0, shotParams.flatten());
            }
            return true;
        }
        return false;
    }

    virtual void requestExit() {
        Thread::requestExit();

        mDestroying = true;
        mFW->onFrameAvailable();
    }

    void setBuffer() {
        mCamera->setBufferSource(NULL, mSurfaceTexture);
    }

    bool toggleStreamCapture(int expBracketIdx) {
        Mutex::Autolock lock(mToggleStateMutex);
        mExpBracketIdx = expBracketIdx;
        mRestartCapture = !mRestartCapture;
        return mRestartCapture;
    }

    buffer_info_t popBuffer() {
        buffer_info_t buffer;
        Mutex::Autolock lock(mReturnedBuffersMutex);
        printf ("[1]mReturnedBuffers.size() = %d empty() = %d\n", mReturnedBuffers.size(), mReturnedBuffers.isEmpty());
        if (!mReturnedBuffers.isEmpty()) {
            buffer = mReturnedBuffers.itemAt(0);
            mReturnedBuffers.removeAt(0);
        }
        return buffer;
    }

    bool hasBuffer() {
        Mutex::Autolock lock(mReturnedBuffersMutex);
        printf ("[2]mReturnedBuffers.size() = %d empty() = %d\n", mReturnedBuffers.size(), mReturnedBuffers.isEmpty());
        return !mReturnedBuffers.isEmpty();
    }

    void handleBuffer(sp<GraphicBuffer> &, uint8_t *, unsigned int);
    void showMetadata(const String8&);

private:
    SurfaceTextureBase *mSurfaceTextureBase;
    sp<SurfaceTexture> mSurfaceTexture;
    unsigned int mCounter;
    bool mDisplayable;
    int mTexId;
    sp<Camera> mCamera;
    sp<FrameWaiter> mFW;
    Vector<buffer_info_t> mReturnedBuffers;
    Mutex mReturnedBuffersMutex;
    Mutex mToggleStateMutex;
    const unsigned int kReturnedBuffersMaxCapacity;
    bool mDestroying;
    bool mRestartCapture;
    int mExpBracketIdx;
    int mExp;
    int mGain;
    sp<Defer> mDeferThread;
};

class BufferSourceInput : public RefBase {
public:
    BufferSourceInput(bool display, int tex_id, sp<Camera> camera) :
                 mDisplayable(display), mTexId(tex_id), mCamera(camera) {
        mSurfaceTexture = new SurfaceTextureBase();
    }
    virtual ~BufferSourceInput() {
        delete mSurfaceTexture;
    }

    void init() {
        sp<SurfaceTexture> surface_texture;
        mSurfaceTexture->initialize(mTexId);
        surface_texture = mSurfaceTexture->getST();
        surface_texture->setSynchronousMode(true);
    }

    void setInput(buffer_info_t, const char *format);

private:
    SurfaceTextureBase *mSurfaceTexture;
    bool mDisplayable;
    int mTexId;
    sp<Camera> mCamera;
};

#endif
