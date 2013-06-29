/*
 *  Copyright (C) 2012 Texas Instruments, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ARXStreamsJniContext.h"

#include <arx/ARXJniUtil.h>
#include <arx/ARXBufferTypes.h>

using namespace tiarx;

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "ARXStreamsJniContext", __VA_ARGS__);

ARXStreamsJniContext::ARXStreamsJniContext(JavaVM *vm, JNIEnv *env, jobject callerInstance)
{
    pthread_mutex_init(&mBmpLock, NULL);
    mVm = vm;
    mCallerInstance = env->NewGlobalRef(callerInstance);
    jclass cls = env->GetObjectClass(callerInstance);
    mOnARXDeath = env->GetMethodID(cls, "onARXDeath", "()V");
    mOnBufferChanged = env->GetMethodID(cls, "onBufferChanged", "()V");
    mBitmap = NULL;
}

ARXStreamsJniContext::~ARXStreamsJniContext()
{
    if (mArx) {
        mArx->destroy();
    }
    //A callback may be currently running, acquire mutex
    //before releasing reference to the bitmap
    pthread_mutex_lock(&mBmpLock);
    if (mBitmap) {
        JNIEnv *env = getJNIEnv();
        env->DeleteGlobalRef(mBitmap);
    }
    pthread_mutex_unlock(&mBmpLock);

    pthread_mutex_destroy(&mBmpLock);
    JNIEnv *env = getJNIEnv();
    env->DeleteGlobalRef(mCallerInstance);
}

bool ARXStreamsJniContext::setup(ARAccelerator *arx, jobject jSurface, jobject jSurfaceTex, jobject bitmap)
{
    JNIEnv *env = getJNIEnv();
    /*********************************************************************
     * Main camera stream
     *********************************************************************/
    // Get the manager for this buffer.
    ARXImageBufferMgr *prevMgr = arx->getImageBufferMgr(BUFF_CAMOUT);
    if (prevMgr != NULL) {
        // Configure the images that will be generated through this buffer
        // stream.
        prevMgr->setSize(640, 480);
        //Only NV12 is supported in this release
        prevMgr->setFormat(FOURCC_NV12);

        // Bind the image buffer stream to the Android Surface
        // provided from Java.
        android::Surface *surf = get_surface(env, mCallerInstance, jSurface);
        if (prevMgr->bindSurface(surf) != NOERROR) {
            LOGE("Could not bind surface to main cam output!");
            return false;
        }
    } else {
        LOGE("Could not obtain main cam buffer manager!");
        return false;
    }

    // We want to access the secondary camera output stream (usually lower
    // resolution than primary output stream useful for computer vision computations).
    /*********************************************************************
     * Secondary camera stream (computation buffer)
     *********************************************************************/
    ARXImageBufferMgr *compMgr = arx->getImageBufferMgr(BUFF_CAMOUT2);
    if (compMgr != NULL) {
        // Configure the images that will be generated through this buffer
        // stream.
        compMgr->setSize(320, 240);
        //Only NV12 is supported in this release
        compMgr->setFormat(FOURCC_NV12);

        // Bind the image buffer stream to the Android SurfaceTexture
        // provided from Java.
        android::ISurfaceTexture *tex = get_surfaceTexture(env, mCallerInstance, jSurfaceTex);
        if (compMgr->bindSurface(tex) != NOERROR) {
            LOGE("Could not bind surface to secondary cam output!");
            return false;
        }
    } else {
        LOGE("Could not obtain cam secondary output buffer manager!");
        return false;
    }

    /*********************************************************************
     * Sobel image stream
     *********************************************************************/
    // Get the manager for this buffer.
    ARXImageBufferMgr *sobelMgr = arx->getImageBufferMgr(BUFF_SOBEL_3X3);
    if (sobelMgr != NULL) {
        // Register for "buffer changed" notifications so that we can copy the
        // Sobel image to our Java bitmap (our context will handle this).
        sobelMgr->registerClient(this);
        int ret = AndroidBitmap_getInfo(env, bitmap, &mBmpInfo);
        if (ret < 0 || mBmpInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
            return false;
        }
        mBitmap = env->NewGlobalRef(bitmap);
    } else {
        LOGE("Could not obtain sobel buffer manager!");
        return false;
    }
    mArx = arx;
    return true;
}

/*!
 * Called by ARX when a property changes values (part of the
 * ARXPropertyListener interface).
 * @see tiarx::ARXPropertyListener
 */
void ARXStreamsJniContext::onPropertyChanged(uint32_t property, int32_t value) {
    if (property == PROP_ENGINE_STATE && value == ENGINE_STATE_DEAD) {
        JNIEnv *env = getJNIEnv();
        env->CallVoidMethod(mCallerInstance, mOnARXDeath);
    }
}

/*!
 * Called by ARX when a buffer's contents change.
 *
 * In this application, we use this to listen for changes to the Sobel buffer.
 * When a buffer's contents are updated, we copy them to our Java bitmap.
 * @param pImage the buffer object that has changed.
 * @see tiarx::ARXImageBufferListener
 */
void ARXStreamsJniContext::onBufferChanged(ARXImageBuffer *pImage) {
    JNIEnv *env = getJNIEnv();
    if (pImage->id() == BUFF_SOBEL_3X3) {

        pthread_mutex_lock(&mBmpLock);
        uint32_t w = mBmpInfo.width;
        uint32_t h = mBmpInfo.height;
        uint32_t srcStride = pImage->stride(); // stride for the Sobel image from ARX
        uint32_t dstStride = mBmpInfo.stride; // stride for our Java bitmap
        uint8_t *pSrc = (uint8_t *)pImage->data(0); // Sobel image data from ARX
        uint8_t *pDst = NULL;

        int ret = AndroidBitmap_lockPixels(env, mBitmap, (void **)&pDst);
        if (ret == 0 && pDst != NULL) {
            for (uint32_t i = 0; i < h; i++) {
                for (uint32_t j = 0, k = 0; j < w; j++, k += 4) {
                    pDst[k] = pSrc[j]; //R
                    pDst[k+1] = pSrc[j]; //G
                    pDst[k+2] = pSrc[j]; //B
                    pDst[k+3] = 255; //A
                }
                pSrc += srcStride;
                pDst += dstStride;
            }
        }

        AndroidBitmap_unlockPixels(env, mBitmap);
        pthread_mutex_unlock(&mBmpLock);
    }
    pImage->release();
    env->CallVoidMethod(mCallerInstance, mOnBufferChanged);
}

JNIEnv *ARXStreamsJniContext::getJNIEnv()
{
    union {
        JNIEnv *env;
        void *env_void;
    };
    env = NULL;
    jint ret;
    ret = mVm->GetEnv(&env_void, JNI_VERSION_1_6);
    if (ret == JNI_EDETACHED) {
        LOGE("Encountered a non JNI attached thread!\n");
    }
    return env;
}
