/*
 * Copyright (c) 2010, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "CPCam-JNI"
#include <utils/Log.h>

#include "jni.h"
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"

#include <utils/Vector.h>

#include <gui/SurfaceTexture.h>
#include <surfaceflinger/Surface.h>
#include <camera/Camera.h>
#include <binder/IMemory.h>

#ifdef ANDROID_API_JB_OR_LATER
#   define CAMHAL_LOGV ALOGV
#   define CAMHAL_LOGE ALOGE
#else
#   define CAMHAL_LOGV LOGV
#   define CAMHAL_LOGE LOGE
#endif

using namespace android;

struct fields_t {
    jfieldID    context;
    jfieldID    surface;
    jfieldID    surfaceTexture;
    jfieldID    facing;
    jfieldID    orientation;
    jfieldID    face_rect;
    jfieldID    face_score;
    jfieldID    rect_left;
    jfieldID    rect_top;
    jfieldID    rect_right;
    jfieldID    rect_bottom;
    jmethodID   post_event;
    jmethodID   rect_constructor;
    jmethodID   face_constructor;
    jfieldID    exposure_time;
    jfieldID    analog_gain;
    jfieldID    faces;
    jmethodID   metadata_constructor;
};

static fields_t fields;
static Mutex sLock;

// provides persistent context for calls from native code to Java
class JNICPCamContext: public CameraListener
{
public:
    JNICPCamContext(JNIEnv* env, jobject weak_this, jclass clazz, const sp<Camera>& camera);
    ~JNICPCamContext() { release(); }
    virtual void notify(int32_t msgType, int32_t ext1, int32_t ext2);
    virtual void postData(int32_t msgType, const sp<IMemory>& dataPtr,
                          camera_frame_metadata_t *metadata);
    virtual void postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr);
    void postMetadata(JNIEnv *env, int32_t msgType, camera_frame_metadata_t *metadata);
    void addCallbackBuffer(JNIEnv *env, jbyteArray cbb, int msgType);
    void setCallbackMode(JNIEnv *env, bool installed, bool manualMode);
    sp<Camera> getCamera() { Mutex::Autolock _l(mLock); return mCamera; }
    bool isRawImageCallbackBufferAvailable() const;
    void release();

private:
    void copyAndPost(JNIEnv* env, const sp<IMemory>& dataPtr, int msgType);
    void clearCallbackBuffers_l(JNIEnv *env, Vector<jbyteArray> *buffers);
    void clearCallbackBuffers_l(JNIEnv *env);
    jbyteArray getCallbackBuffer(JNIEnv *env, Vector<jbyteArray> *buffers, size_t bufferSize);

    jobject     mCameraJObjectWeak;     // weak reference to java object
    jclass      mCameraJClass;          // strong reference to java class
    sp<Camera>  mCamera;                // strong reference to native object
    jclass      mFaceClass;  // strong reference to Face class
    jclass      mMetadataClass;  // strong reference to Metadata class
    jclass      mRectClass;  // strong reference to Rect class
    Mutex       mLock;

    /*
     * Global reference application-managed raw image buffer queue.
     *
     * Manual-only mode is supported for raw image callbacks, which is
     * set whenever method addCallbackBuffer() with msgType =
     * CAMERA_MSG_RAW_IMAGE is called; otherwise, null is returned
     * with raw image callbacks.
     */
    Vector<jbyteArray> mRawImageCallbackBuffers;

    /*
     * Application-managed preview buffer queue and the flags
     * associated with the usage of the preview buffer callback.
     */
    Vector<jbyteArray> mCallbackBuffers; // Global reference application managed byte[]
    bool mManualBufferMode;              // Whether to use application managed buffers.
    bool mManualCameraCallbackSet;       // Whether the callback has been set, used to
                                         // reduce unnecessary calls to set the callback.
};

bool JNICPCamContext::isRawImageCallbackBufferAvailable() const
{
    return !mRawImageCallbackBuffers.isEmpty();
}

sp<Camera> get_native_camera(JNIEnv *env, jobject thiz, JNICPCamContext** pContext)
{
    sp<Camera> camera;
    Mutex::Autolock _l(sLock);
    JNICPCamContext* context = reinterpret_cast<JNICPCamContext*>(env->GetIntField(thiz, fields.context));
    if (context != NULL) {
        camera = context->getCamera();
    }
    CAMHAL_LOGV("get_native_camera: context=%p, camera=%p", context, camera.get());
    if (camera == 0) {
        jniThrowRuntimeException(env, "Method called after release()");
    }

    if (pContext != NULL) *pContext = context;
    return camera;
}

JNICPCamContext::JNICPCamContext(JNIEnv* env, jobject weak_this, jclass clazz, const sp<Camera>& camera)
{
    mCameraJObjectWeak = env->NewGlobalRef(weak_this);
    mCameraJClass = (jclass)env->NewGlobalRef(clazz);
    mCamera = camera;

    jclass faceClazz = env->FindClass("com/ti/omap/android/cpcam/CPCam$Face");
    mFaceClass = (jclass) env->NewGlobalRef(faceClazz);

    jclass metadataClazz = env->FindClass("com/ti/omap/android/cpcam/CPCam$Metadata");
    mMetadataClass = (jclass) env->NewGlobalRef(metadataClazz);

    jclass rectClazz = env->FindClass("android/graphics/Rect");
    mRectClass = (jclass) env->NewGlobalRef(rectClazz);

    mManualBufferMode = false;
    mManualCameraCallbackSet = false;
}

void JNICPCamContext::release()
{
    CAMHAL_LOGV("release");
    Mutex::Autolock _l(mLock);
    JNIEnv *env = AndroidRuntime::getJNIEnv();

    if (mCameraJObjectWeak != NULL) {
        env->DeleteGlobalRef(mCameraJObjectWeak);
        mCameraJObjectWeak = NULL;
    }
    if (mCameraJClass != NULL) {
        env->DeleteGlobalRef(mCameraJClass);
        mCameraJClass = NULL;
    }
    if (mFaceClass != NULL) {
        env->DeleteGlobalRef(mFaceClass);
        mFaceClass = NULL;
    }
    if (mMetadataClass != NULL) {
        env->DeleteGlobalRef(mMetadataClass);
        mMetadataClass = NULL;
    }
    if (mRectClass != NULL) {
        env->DeleteGlobalRef(mRectClass);
        mRectClass = NULL;
    }
    clearCallbackBuffers_l(env);
    mCamera.clear();
}

void JNICPCamContext::notify(int32_t msgType, int32_t ext1, int32_t ext2)
{
    CAMHAL_LOGV("notify");

    // VM pointer will be NULL if object is released
    Mutex::Autolock _l(mLock);
    if (mCameraJObjectWeak == NULL) {
        LOGW("callback on dead camera object");
        return;
    }
    JNIEnv *env = AndroidRuntime::getJNIEnv();

    /*
     * If the notification or msgType is CAMERA_MSG_RAW_IMAGE_NOTIFY, change it
     * to CAMERA_MSG_RAW_IMAGE since CAMERA_MSG_RAW_IMAGE_NOTIFY is not exposed
     * to the Java app.
     */
    if (msgType == CAMERA_MSG_RAW_IMAGE_NOTIFY) {
        msgType = CAMERA_MSG_RAW_IMAGE;
    }

    env->CallStaticVoidMethod(mCameraJClass, fields.post_event,
            mCameraJObjectWeak, msgType, ext1, ext2, NULL);
}

jbyteArray JNICPCamContext::getCallbackBuffer(
        JNIEnv* env, Vector<jbyteArray>* buffers, size_t bufferSize)
{
    jbyteArray obj = NULL;

    // Vector access should be protected by lock in postData()
    if (!buffers->isEmpty()) {
        CAMHAL_LOGV("Using callback buffer from queue of length %d", buffers->size());
        jbyteArray globalBuffer = buffers->itemAt(0);
        buffers->removeAt(0);

        obj = (jbyteArray)env->NewLocalRef(globalBuffer);
        env->DeleteGlobalRef(globalBuffer);

        if (obj != NULL) {
            jsize bufferLength = env->GetArrayLength(obj);
            if ((int)bufferLength < (int)bufferSize) {
                CAMHAL_LOGE("Callback buffer was too small! Expected %d bytes, but got %d bytes!",
                    bufferSize, bufferLength);
                env->DeleteLocalRef(obj);
                return NULL;
            }
        }
    }

    return obj;
}

void JNICPCamContext::copyAndPost(JNIEnv* env, const sp<IMemory>& dataPtr, int msgType)
{
    jbyteArray obj = NULL;

    // allocate Java byte array and copy data
    if (dataPtr != NULL) {
        ssize_t offset;
        size_t size;
        sp<IMemoryHeap> heap = dataPtr->getMemory(&offset, &size);
        CAMHAL_LOGV("copyAndPost: off=%ld, size=%d", offset, size);
        uint8_t *heapBase = (uint8_t*)heap->base();

        if (heapBase != NULL) {
            const jbyte* data = reinterpret_cast<const jbyte*>(heapBase + offset);

            if (msgType == CAMERA_MSG_RAW_IMAGE) {
                obj = getCallbackBuffer(env, &mRawImageCallbackBuffers, size);
            } else if (msgType == CAMERA_MSG_PREVIEW_FRAME && mManualBufferMode) {
                obj = getCallbackBuffer(env, &mCallbackBuffers, size);

                if (mCallbackBuffers.isEmpty()) {
                    CAMHAL_LOGV("Out of buffers, clearing callback!");
                    mCamera->setPreviewCallbackFlags(CAMERA_FRAME_CALLBACK_FLAG_NOOP);
                    mManualCameraCallbackSet = false;

                    if (obj == NULL) {
                        return;
                    }
                }
            } else {
                CAMHAL_LOGV("Allocating callback buffer");
                obj = env->NewByteArray(size);
            }

            if (obj == NULL) {
                CAMHAL_LOGE("Couldn't allocate byte array for JPEG data");
                env->ExceptionClear();
            } else {
                env->SetByteArrayRegion(obj, 0, size, data);
            }
        } else {
            CAMHAL_LOGE("image heap is NULL");
        }
    }

    // post image data to Java
    env->CallStaticVoidMethod(mCameraJClass, fields.post_event,
            mCameraJObjectWeak, msgType, 0, 0, obj);
    if (obj) {
        env->DeleteLocalRef(obj);
    }
}

void JNICPCamContext::postData(int32_t msgType, const sp<IMemory>& dataPtr,
                                camera_frame_metadata_t *metadata)
{
    // VM pointer will be NULL if object is released
    Mutex::Autolock _l(mLock);
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    if (mCameraJObjectWeak == NULL) {
        LOGW("callback on dead camera object");
        return;
    }

    int32_t dataMsgType = msgType & ~CAMERA_MSG_PREVIEW_METADATA;

    // return data based on callback type
    switch (dataMsgType) {
        case CAMERA_MSG_VIDEO_FRAME:
            // should never happen
            break;

        // For backward-compatibility purpose, if there is no callback
        // buffer for raw image, the callback returns null.
        case CAMERA_MSG_RAW_IMAGE:
            CAMHAL_LOGV("rawCallback");
            if (mRawImageCallbackBuffers.isEmpty()) {
                env->CallStaticVoidMethod(mCameraJClass, fields.post_event,
                        mCameraJObjectWeak, dataMsgType, 0, 0, NULL);
            } else {
                copyAndPost(env, dataPtr, dataMsgType);
            }
            break;

        // There is no data.
        case 0:
            break;

        default:
            CAMHAL_LOGV("dataCallback(%d, %p)", dataMsgType, dataPtr.get());
            copyAndPost(env, dataPtr, dataMsgType);
            break;
    }

    // post frame metadata to Java
    if (metadata && (msgType & CAMERA_MSG_PREVIEW_METADATA)) {
        postMetadata(env, CAMERA_MSG_PREVIEW_METADATA, metadata);
    }
}

void JNICPCamContext::postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr)
{
    // TODO: plumb up to Java. For now, just drop the timestamp
    postData(msgType, dataPtr, NULL);
}

void JNICPCamContext::postMetadata(JNIEnv *env, int32_t msgType, camera_frame_metadata_t *metadata)
{
    jobject meta_obj = NULL;
    meta_obj = (jobject) env->NewObject(mMetadataClass, fields.metadata_constructor);
    if (meta_obj == NULL) {
        CAMHAL_LOGE("Couldn't allocate metadata class");
        return;
    }

    env->SetIntField(meta_obj, fields.exposure_time, metadata->exposure_time);
    env->SetIntField(meta_obj, fields.analog_gain, metadata->analog_gain);

    jobjectArray faces_obj = NULL;
    faces_obj = (jobjectArray) env->NewObjectArray(metadata->number_of_faces,
                                             mFaceClass, NULL);
    if (faces_obj == NULL) {
        CAMHAL_LOGE("Couldn't allocate face metadata array");
        goto err_alloc_faces;
    }

    for (int i = 0; i < metadata->number_of_faces; i++) {
        jobject face = env->NewObject(mFaceClass, fields.face_constructor);
        env->SetObjectArrayElement(faces_obj, i, face);

        jobject rect = env->NewObject(mRectClass, fields.rect_constructor);
        env->SetIntField(rect, fields.rect_left, metadata->faces[i].rect[0]);
        env->SetIntField(rect, fields.rect_top, metadata->faces[i].rect[1]);
        env->SetIntField(rect, fields.rect_right, metadata->faces[i].rect[2]);
        env->SetIntField(rect, fields.rect_bottom, metadata->faces[i].rect[3]);

        env->SetObjectField(face, fields.face_rect, rect);
        env->SetIntField(face, fields.face_score, metadata->faces[i].score);

        env->DeleteLocalRef(face);
        env->DeleteLocalRef(rect);
    }

    env->SetObjectField(meta_obj, fields.faces, faces_obj);

    env->CallStaticVoidMethod(mCameraJClass, fields.post_event,
            mCameraJObjectWeak, msgType, CAMERA_MSG_PREVIEW_METADATA, 0, meta_obj);

    env->DeleteLocalRef(faces_obj);
err_alloc_faces:
    env->DeleteLocalRef(meta_obj);
    return;
}

void JNICPCamContext::setCallbackMode(JNIEnv *env, bool installed, bool manualMode)
{
    Mutex::Autolock _l(mLock);
    mManualBufferMode = manualMode;
    mManualCameraCallbackSet = false;

    // In order to limit the over usage of binder threads, all non-manual buffer
    // callbacks use CAMERA_FRAME_CALLBACK_FLAG_BARCODE_SCANNER mode now.
    //
    // Continuous callbacks will have the callback re-registered from handleMessage.
    // Manual buffer mode will operate as fast as possible, relying on the finite supply
    // of buffers for throttling.

    if (!installed) {
        mCamera->setPreviewCallbackFlags(CAMERA_FRAME_CALLBACK_FLAG_NOOP);
        clearCallbackBuffers_l(env, &mCallbackBuffers);
    } else if (mManualBufferMode) {
        if (!mCallbackBuffers.isEmpty()) {
            mCamera->setPreviewCallbackFlags(CAMERA_FRAME_CALLBACK_FLAG_CAMERA);
            mManualCameraCallbackSet = true;
        }
    } else {
        mCamera->setPreviewCallbackFlags(CAMERA_FRAME_CALLBACK_FLAG_BARCODE_SCANNER);
        clearCallbackBuffers_l(env, &mCallbackBuffers);
    }
}

void JNICPCamContext::addCallbackBuffer(
        JNIEnv *env, jbyteArray cbb, int msgType)
{
    CAMHAL_LOGV("addCallbackBuffer: 0x%x", msgType);
    if (cbb != NULL) {
        Mutex::Autolock _l(mLock);
        switch (msgType) {
            case CAMERA_MSG_PREVIEW_FRAME: {
                jbyteArray callbackBuffer = (jbyteArray)env->NewGlobalRef(cbb);
                mCallbackBuffers.push(callbackBuffer);

                CAMHAL_LOGV("Adding callback buffer to queue, %d total",
                        mCallbackBuffers.size());

                // We want to make sure the camera knows we're ready for the
                // next frame. This may have come unset had we not had a
                // callbackbuffer ready for it last time.
                if (mManualBufferMode && !mManualCameraCallbackSet) {
                    mCamera->setPreviewCallbackFlags(CAMERA_FRAME_CALLBACK_FLAG_CAMERA);
                    mManualCameraCallbackSet = true;
                }
                break;
            }
            case CAMERA_MSG_RAW_IMAGE: {
                jbyteArray callbackBuffer = (jbyteArray)env->NewGlobalRef(cbb);
                mRawImageCallbackBuffers.push(callbackBuffer);
                break;
            }
            default: {
                jniThrowException(env,
                        "java/lang/IllegalArgumentException",
                        "Unsupported message type");
                return;
            }
        }
    } else {
       CAMHAL_LOGE("Null byte array!");
    }
}

void JNICPCamContext::clearCallbackBuffers_l(JNIEnv *env)
{
    clearCallbackBuffers_l(env, &mCallbackBuffers);
    clearCallbackBuffers_l(env, &mRawImageCallbackBuffers);
}

void JNICPCamContext::clearCallbackBuffers_l(JNIEnv *env, Vector<jbyteArray> *buffers) {
    CAMHAL_LOGV("Clearing callback buffers, %d remained", buffers->size());
    while (!buffers->isEmpty()) {
        env->DeleteGlobalRef(buffers->top());
        buffers->pop();
    }
}

static jint com_ti_omap_android_cpcam_CPCam_getNumberOfCameras(JNIEnv *env, jobject thiz)
{
    return Camera::getNumberOfCameras();
}

static void com_ti_omap_android_cpcam_CPCam_getCameraInfo(JNIEnv *env, jobject thiz,
    jint cameraId, jobject info_obj)
{
    CameraInfo cameraInfo;
    status_t rc = Camera::getCameraInfo(cameraId, &cameraInfo);
    if (rc != NO_ERROR) {
        jniThrowRuntimeException(env, "Fail to get camera info");
        return;
    }
    env->SetIntField(info_obj, fields.facing, cameraInfo.facing);
    env->SetIntField(info_obj, fields.orientation, cameraInfo.orientation);
}

// connect to camera service
static void com_ti_omap_android_cpcam_CPCam_native_setup(JNIEnv *env, jobject thiz,
    jobject weak_this, jint cameraId)
{
    sp<Camera> camera = Camera::connect(cameraId);

    if (camera == NULL) {
        jniThrowRuntimeException(env, "Fail to connect to camera service");
        return;
    }

    // make sure camera hardware is alive
    if (camera->getStatus() != NO_ERROR) {
        jniThrowRuntimeException(env, "Camera initialization failed");
        return;
    }

    jclass clazz = env->GetObjectClass(thiz);
    if (clazz == NULL) {
        jniThrowRuntimeException(env, "Can't find com/ti/omap/android/cpcam/CPCam");
        return;
    }

    // We use a weak reference so the Camera object can be garbage collected.
    // The reference is only used as a proxy for callbacks.
    sp<JNICPCamContext> context = new JNICPCamContext(env, weak_this, clazz, camera);
    context->incStrong(thiz);
    camera->setListener(context);

    // save context in opaque field
    env->SetIntField(thiz, fields.context, (int)context.get());
}

// disconnect from camera service
// It's okay to call this when the native camera context is already null.
// This handles the case where the user has called release() and the
// finalizer is invoked later.
static void com_ti_omap_android_cpcam_CPCam_release(JNIEnv *env, jobject thiz)
{
    // TODO: Change to CAMHAL_LOGV
    CAMHAL_LOGV("release camera");
    JNICPCamContext* context = NULL;
    sp<Camera> camera;
    {
        Mutex::Autolock _l(sLock);
        context = reinterpret_cast<JNICPCamContext*>(env->GetIntField(thiz, fields.context));

        // Make sure we do not attempt to callback on a deleted Java object.
        env->SetIntField(thiz, fields.context, 0);
    }

    // clean up if release has not been called before
    if (context != NULL) {
        camera = context->getCamera();
        context->release();
        CAMHAL_LOGV("native_release: context=%p camera=%p", context, camera.get());

        // clear callbacks
        if (camera != NULL) {
            camera->setPreviewCallbackFlags(CAMERA_FRAME_CALLBACK_FLAG_NOOP);
            camera->disconnect();
        }

        // remove context to prevent further Java access
        context->decStrong(thiz);
    }
}

static void com_ti_omap_android_cpcam_CPCam_setPreviewDisplay(JNIEnv *env, jobject thiz, jobject jSurface)
{
    CAMHAL_LOGV("setPreviewDisplay");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

    sp<Surface> surface = NULL;
    if (jSurface != NULL) {
        surface = reinterpret_cast<Surface*>(env->GetIntField(jSurface, fields.surface));
    }
    if (camera->setPreviewDisplay(surface) != NO_ERROR) {
        jniThrowException(env, "java/io/IOException", "setPreviewDisplay failed");
    }
}

static void com_ti_omap_android_cpcam_CPCam_setPreviewTexture(JNIEnv *env,
        jobject thiz, jobject jSurfaceTexture)
{
    CAMHAL_LOGV("setPreviewTexture");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

    sp<SurfaceTexture> surfaceTexture = NULL;
    if (jSurfaceTexture != NULL) {
        surfaceTexture = reinterpret_cast<SurfaceTexture*>(env->GetIntField(
                jSurfaceTexture, fields.surfaceTexture));
    }
    if (camera->setPreviewTexture(surfaceTexture) != NO_ERROR) {
        jniThrowException(env, "java/io/IOException",
                "setPreviewTexture failed");
    }
}

static void com_ti_omap_android_cpcam_CPCam_setBufferSource(JNIEnv *env,
        jobject thiz, jobject jTapIn, jobject jTapOut)
{
    CAMHAL_LOGV("setBufferSource");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

    sp<SurfaceTexture> tapOut = NULL;
    if (jTapOut!= NULL) {
        tapOut = reinterpret_cast<SurfaceTexture*>(env->GetIntField(
                jTapOut, fields.surfaceTexture));
    }

    sp<SurfaceTexture> tapIn = NULL;
    if (jTapIn != NULL) {
        tapIn = reinterpret_cast<SurfaceTexture*>(env->GetIntField(
                jTapIn, fields.surfaceTexture));
    }

    if (camera->setBufferSource(tapIn, tapOut) != NO_ERROR) { // tapin not enabled yet
       jniThrowException(env, "java/io/IOException",
               "setBufferSource failed");
    }
}

static void com_ti_omap_android_cpcam_CPCam_reprocess(JNIEnv *env,
        jobject thiz, jint msgType, jstring jShotParams)
{
    const char *shotParams = (jShotParams) ? env->GetStringUTFChars(jShotParams, NULL) : NULL;
    String8 params(shotParams ? shotParams: "");

    CAMHAL_LOGV("reprocess");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

    if (camera->reprocess(msgType, params) != NO_ERROR) {
       jniThrowException(env, "java/io/IOException",
               "reprocess failed");
    }
}

static void com_ti_omap_android_cpcam_CPCam_startPreview(JNIEnv *env, jobject thiz)
{
    CAMHAL_LOGV("startPreview");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

    if (camera->startPreview() != NO_ERROR) {
        jniThrowRuntimeException(env, "startPreview failed");
        return;
    }
}

static void com_ti_omap_android_cpcam_CPCam_stopPreview(JNIEnv *env, jobject thiz)
{
    CAMHAL_LOGV("stopPreview");
    sp<Camera> c = get_native_camera(env, thiz, NULL);
    if (c == 0) return;

    c->stopPreview();
}

static bool com_ti_omap_android_cpcam_CPCam_previewEnabled(JNIEnv *env, jobject thiz)
{
    CAMHAL_LOGV("previewEnabled");
    sp<Camera> c = get_native_camera(env, thiz, NULL);
    if (c == 0) return false;

    return c->previewEnabled();
}

static void com_ti_omap_android_cpcam_CPCam_setHasPreviewCallback(JNIEnv *env, jobject thiz, jboolean installed, jboolean manualBuffer)
{
    CAMHAL_LOGV("setHasPreviewCallback: installed:%d, manualBuffer:%d", (int)installed, (int)manualBuffer);
    // Important: Only install preview_callback if the Java code has called
    // setPreviewCallback() with a non-null value, otherwise we'd pay to memcpy
    // each preview frame for nothing.
    JNICPCamContext* context;
    sp<Camera> camera = get_native_camera(env, thiz, &context);
    if (camera == 0) return;

    // setCallbackMode will take care of setting the context flags and calling
    // camera->setPreviewCallbackFlags within a mutex for us.
    context->setCallbackMode(env, installed, manualBuffer);
}

static void com_ti_omap_android_cpcam_CPCam_addCallbackBuffer(JNIEnv *env, jobject thiz, jbyteArray bytes, int msgType) {
    CAMHAL_LOGV("addCallbackBuffer: 0x%x", msgType);

    JNICPCamContext* context = reinterpret_cast<JNICPCamContext*>(env->GetIntField(thiz, fields.context));

    if (context != NULL) {
        context->addCallbackBuffer(env, bytes, msgType);
    }
}

static void com_ti_omap_android_cpcam_CPCam_autoFocus(JNIEnv *env, jobject thiz)
{
    CAMHAL_LOGV("autoFocus");
    JNICPCamContext* context;
    sp<Camera> c = get_native_camera(env, thiz, &context);
    if (c == 0) return;

    if (c->autoFocus() != NO_ERROR) {
        jniThrowRuntimeException(env, "autoFocus failed");
    }
}

static void com_ti_omap_android_cpcam_CPCam_cancelAutoFocus(JNIEnv *env, jobject thiz)
{
    CAMHAL_LOGV("cancelAutoFocus");
    JNICPCamContext* context;
    sp<Camera> c = get_native_camera(env, thiz, &context);
    if (c == 0) return;

    if (c->cancelAutoFocus() != NO_ERROR) {
        jniThrowRuntimeException(env, "cancelAutoFocus failed");
    }
}

static void com_ti_omap_android_cpcam_CPCam_takePicture(JNIEnv *env, jobject thiz, int msgType, jstring params)
{
    CAMHAL_LOGV("takePicture");
    JNICPCamContext* context;
    sp<Camera> camera = get_native_camera(env, thiz, &context);
    if (camera == 0) return;

    String8 params8;
    if (params) {
        const jchar* str = env->GetStringCritical(params, 0);
        params8 = String8(str, env->GetStringLength(params));
        env->ReleaseStringCritical(params, str);
    }

    /*
     * When CAMERA_MSG_RAW_IMAGE is requested, if the raw image callback
     * buffer is available, CAMERA_MSG_RAW_IMAGE is enabled to get the
     * notification _and_ the data; otherwise, CAMERA_MSG_RAW_IMAGE_NOTIFY
     * is enabled to receive the callback notification but no data.
     *
     * Note that CAMERA_MSG_RAW_IMAGE_NOTIFY is not exposed to the
     * Java application.
     */
    if (msgType & CAMERA_MSG_RAW_IMAGE) {
        CAMHAL_LOGV("Enable raw image callback buffer");
        if (!context->isRawImageCallbackBufferAvailable()) {
            CAMHAL_LOGV("Enable raw image notification, since no callback buffer exists");
            msgType &= ~CAMERA_MSG_RAW_IMAGE;
            msgType |= CAMERA_MSG_RAW_IMAGE_NOTIFY;
        }
    }

    if (camera->takePicture(msgType, params8) != NO_ERROR) {
        jniThrowRuntimeException(env, "takePicture failed");
        return;
    }
}

static void com_ti_omap_android_cpcam_CPCam_setParameters(JNIEnv *env, jobject thiz, jstring params)
{
    CAMHAL_LOGV("setParameters");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

    const jchar* str = env->GetStringCritical(params, 0);
    String8 params8;
    if (params) {
        params8 = String8(str, env->GetStringLength(params));
        env->ReleaseStringCritical(params, str);
    }
    if (camera->setParameters(params8) != NO_ERROR) {
        jniThrowRuntimeException(env, "setParameters failed");
        return;
    }
}

static jstring com_ti_omap_android_cpcam_CPCam_getParameters(JNIEnv *env, jobject thiz)
{
    CAMHAL_LOGV("getParameters");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return 0;

    return env->NewStringUTF(camera->getParameters().string());
}

static void com_ti_omap_android_cpcam_CPCam_reconnect(JNIEnv *env, jobject thiz)
{
    CAMHAL_LOGV("reconnect");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

    if (camera->reconnect() != NO_ERROR) {
        jniThrowException(env, "java/io/IOException", "reconnect failed");
        return;
    }
}

static void com_ti_omap_android_cpcam_CPCam_lock(JNIEnv *env, jobject thiz)
{
    CAMHAL_LOGV("lock");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

    if (camera->lock() != NO_ERROR) {
        jniThrowRuntimeException(env, "lock failed");
    }
}

static void com_ti_omap_android_cpcam_CPCam_unlock(JNIEnv *env, jobject thiz)
{
    CAMHAL_LOGV("unlock");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

    if (camera->unlock() != NO_ERROR) {
        jniThrowRuntimeException(env, "unlock failed");
    }
}

static void com_ti_omap_android_cpcam_CPCam_startSmoothZoom(JNIEnv *env, jobject thiz, jint value)
{
    CAMHAL_LOGV("startSmoothZoom");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

    status_t rc = camera->sendCommand(CAMERA_CMD_START_SMOOTH_ZOOM, value, 0);
    if (rc == BAD_VALUE) {
        char msg[64];
        sprintf(msg, "invalid zoom value=%d", value);
        jniThrowException(env, "java/lang/IllegalArgumentException", msg);
    } else if (rc != NO_ERROR) {
        jniThrowRuntimeException(env, "start smooth zoom failed");
    }
}

static void com_ti_omap_android_cpcam_CPCam_stopSmoothZoom(JNIEnv *env, jobject thiz)
{
    CAMHAL_LOGV("stopSmoothZoom");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

    if (camera->sendCommand(CAMERA_CMD_STOP_SMOOTH_ZOOM, 0, 0) != NO_ERROR) {
        jniThrowRuntimeException(env, "stop smooth zoom failed");
    }
}

static void com_ti_omap_android_cpcam_CPCam_setDisplayOrientation(JNIEnv *env, jobject thiz,
        jint value)
{
    CAMHAL_LOGV("setDisplayOrientation");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

    if (camera->sendCommand(CAMERA_CMD_SET_DISPLAY_ORIENTATION, value, 0) != NO_ERROR) {
        jniThrowRuntimeException(env, "set display orientation failed");
    }
}

static void com_ti_omap_android_cpcam_CPCam_startFaceDetection(JNIEnv *env, jobject thiz,
        jint type)
{
    CAMHAL_LOGV("startFaceDetection");
    JNICPCamContext* context;
    sp<Camera> camera = get_native_camera(env, thiz, &context);
    if (camera == 0) return;

    status_t rc = camera->sendCommand(CAMERA_CMD_START_FACE_DETECTION, type, 0);
    if (rc == BAD_VALUE) {
        char msg[64];
        snprintf(msg, sizeof(msg), "invalid face detection type=%d", type);
        jniThrowException(env, "java/lang/IllegalArgumentException", msg);
    } else if (rc != NO_ERROR) {
        jniThrowRuntimeException(env, "start face detection failed");
    }
}

static void com_ti_omap_android_cpcam_CPCam_stopFaceDetection(JNIEnv *env, jobject thiz)
{
    CAMHAL_LOGV("stopFaceDetection");
    sp<Camera> camera = get_native_camera(env, thiz, NULL);
    if (camera == 0) return;

    if (camera->sendCommand(CAMERA_CMD_STOP_FACE_DETECTION, 0, 0) != NO_ERROR) {
        jniThrowRuntimeException(env, "stop face detection failed");
    }
}

//-------------------------------------------------

static JNINativeMethod cpcamMethods[] = {
  { "getNumberOfCameras",
    "()I",
    (void *)com_ti_omap_android_cpcam_CPCam_getNumberOfCameras },
  { "getCameraInfo",
    "(ILcom/ti/omap/android/cpcam/CPCam$CameraInfo;)V",
    (void*)com_ti_omap_android_cpcam_CPCam_getCameraInfo },
  { "native_setup",
    "(Ljava/lang/Object;I)V",
    (void*)com_ti_omap_android_cpcam_CPCam_native_setup },
  { "native_release",
    "()V",
    (void*)com_ti_omap_android_cpcam_CPCam_release },
  { "setPreviewDisplay",
    "(Landroid/view/Surface;)V",
    (void *)com_ti_omap_android_cpcam_CPCam_setPreviewDisplay },
  { "setPreviewTexture",
    "(Landroid/graphics/SurfaceTexture;)V",
    (void *)com_ti_omap_android_cpcam_CPCam_setPreviewTexture },
  { "setBufferSource",
    "(Landroid/graphics/SurfaceTexture;Landroid/graphics/SurfaceTexture;)V",
    (void *)com_ti_omap_android_cpcam_CPCam_setBufferSource },
  { "native_reprocess",
    "(ILjava/lang/String;)V",
    (void *)com_ti_omap_android_cpcam_CPCam_reprocess },
  { "startPreview",
    "()V",
    (void *)com_ti_omap_android_cpcam_CPCam_startPreview },
  { "_stopPreview",
    "()V",
    (void *)com_ti_omap_android_cpcam_CPCam_stopPreview },
  { "previewEnabled",
    "()Z",
    (void *)com_ti_omap_android_cpcam_CPCam_previewEnabled },
  { "setHasPreviewCallback",
    "(ZZ)V",
    (void *)com_ti_omap_android_cpcam_CPCam_setHasPreviewCallback },
  { "_addCallbackBuffer",
    "([BI)V",
    (void *)com_ti_omap_android_cpcam_CPCam_addCallbackBuffer },
  { "native_autoFocus",
    "()V",
    (void *)com_ti_omap_android_cpcam_CPCam_autoFocus },
  { "native_cancelAutoFocus",
    "()V",
    (void *)com_ti_omap_android_cpcam_CPCam_cancelAutoFocus },
  { "native_takePicture",
    "(ILjava/lang/String;)V",
    (void *)com_ti_omap_android_cpcam_CPCam_takePicture },
  { "native_setParameters",
    "(Ljava/lang/String;)V",
    (void *)com_ti_omap_android_cpcam_CPCam_setParameters },
  { "native_getParameters",
    "()Ljava/lang/String;",
    (void *)com_ti_omap_android_cpcam_CPCam_getParameters },
  { "reconnect",
    "()V",
    (void*)com_ti_omap_android_cpcam_CPCam_reconnect },
  { "lock",
    "()V",
    (void*)com_ti_omap_android_cpcam_CPCam_lock },
  { "unlock",
    "()V",
    (void*)com_ti_omap_android_cpcam_CPCam_unlock },
  { "startSmoothZoom",
    "(I)V",
    (void *)com_ti_omap_android_cpcam_CPCam_startSmoothZoom },
  { "stopSmoothZoom",
    "()V",
    (void *)com_ti_omap_android_cpcam_CPCam_stopSmoothZoom },
  { "setDisplayOrientation",
    "(I)V",
    (void *)com_ti_omap_android_cpcam_CPCam_setDisplayOrientation },
  { "_startFaceDetection",
    "(I)V",
    (void *)com_ti_omap_android_cpcam_CPCam_startFaceDetection },
  { "_stopFaceDetection",
    "()V",
    (void *)com_ti_omap_android_cpcam_CPCam_stopFaceDetection},
};

struct field {
    const char *class_name;
    const char *field_name;
    const char *field_type;
    jfieldID   *jfield;
};

static int find_fields(JNIEnv *env, field *fields, int count)
{
    for (int i = 0; i < count; i++) {
        field *f = &fields[i];
        jclass clazz = env->FindClass(f->class_name);
        if (clazz == NULL) {
            CAMHAL_LOGE("Can't find %s", f->class_name);
            return -1;
        }

        jfieldID field = env->GetFieldID(clazz, f->field_name, f->field_type);
        if (field == NULL) {
            CAMHAL_LOGE("Can't find %s.%s", f->class_name, f->field_name);
            return -1;
        }

        *(f->jfield) = field;
    }

    return 0;
}

// Get all the required offsets in java class and register native functions
int registerCPCamMethods(JNIEnv *env)
{
    field fields_to_find[] = {
        { "com/ti/omap/android/cpcam/CPCam", "mNativeContext",   "I", &fields.context },
        { "android/view/Surface",    ANDROID_VIEW_SURFACE_JNI_ID, "I", &fields.surface },
        { "android/graphics/SurfaceTexture",
          ANDROID_GRAPHICS_SURFACETEXTURE_JNI_ID, "I", &fields.surfaceTexture },
        { "com/ti/omap/android/cpcam/CPCam$CameraInfo", "facing",   "I", &fields.facing },
        { "com/ti/omap/android/cpcam/CPCam$CameraInfo", "orientation",   "I", &fields.orientation },
        { "com/ti/omap/android/cpcam/CPCam$Face", "rect", "Landroid/graphics/Rect;", &fields.face_rect },
        { "com/ti/omap/android/cpcam/CPCam$Face", "score", "I", &fields.face_score },
        { "android/graphics/Rect", "left", "I", &fields.rect_left },
        { "android/graphics/Rect", "top", "I", &fields.rect_top },
        { "android/graphics/Rect", "right", "I", &fields.rect_right },
        { "android/graphics/Rect", "bottom", "I", &fields.rect_bottom },
        { "com/ti/omap/android/cpcam/CPCam$Metadata", "exposureTime", "I", &fields.exposure_time },
        { "com/ti/omap/android/cpcam/CPCam$Metadata", "analogGain", "I", &fields.analog_gain },
        { "com/ti/omap/android/cpcam/CPCam$Metadata", "faces", "[Lcom/ti/omap/android/cpcam/CPCam$Face;", &fields.faces },
    };

    if (find_fields(env, fields_to_find, NELEM(fields_to_find)) < 0)
        return -1;

    jclass clazz = env->FindClass("com/ti/omap/android/cpcam/CPCam");
    fields.post_event = env->GetStaticMethodID(clazz, "postEventFromNative",
                                               "(Ljava/lang/Object;IIILjava/lang/Object;)V");
    if (fields.post_event == NULL) {
        CAMHAL_LOGE("Can't find com/ti/omap/android/cpcam/CPCam.postEventFromNative");
        return -1;
    }

    clazz = env->FindClass("android/graphics/Rect");
    fields.rect_constructor = env->GetMethodID(clazz, "<init>", "()V");
    if (fields.rect_constructor == NULL) {
        CAMHAL_LOGE("Can't find android/graphics/Rect.Rect()");
        return -1;
    }

    clazz = env->FindClass("com/ti/omap/android/cpcam/CPCam$Face");
    fields.face_constructor = env->GetMethodID(clazz, "<init>", "()V");
    if (fields.face_constructor == NULL) {
        CAMHAL_LOGE("Can't find com/ti/omap/android/cpcam/CPCam$Face.Face()");
        return -1;
    }

    clazz = env->FindClass("com/ti/omap/android/cpcam/CPCam$Metadata");
    fields.metadata_constructor = env->GetMethodID(clazz, "<init>", "()V");
    if (fields.metadata_constructor == NULL) {
        CAMHAL_LOGE("Can't find com/ti/omap/android/cpcam/CPCam$Metadata.Metadata()");
        return -1;
    }

/*
    // Register native functions
    return AndroidRuntime::registerNativeMethods(env, "com/ti/omap/android/cpcam/CPCam",
                                              camMethods, NELEM(camMethods));
*/
    /* register all the methods */
    clazz = env->FindClass("com/ti/omap/android/cpcam/CPCam");
    if (env->RegisterNatives(clazz, cpcamMethods,
            sizeof(cpcamMethods) / sizeof(cpcamMethods[0])) != JNI_OK)
    {
        CAMHAL_LOGE("Failed registering methods for %s\n", "com/ti/omap/android/cpcam/CPCam");
        return -1;
    }

    return 0;
}

// ----------------------------------------------------------------------------

/*
 * This is called by the VM when the shared library is first loaded.
 */
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        CAMHAL_LOGE("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (registerCPCamMethods(env) != 0) {
        CAMHAL_LOGE("ERROR: PlatformLibrary native registration failed\n");
        goto bail;
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;

bail:
    return result;
}
