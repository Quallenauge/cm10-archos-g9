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

#include "ARCameraJniContext.h"

#include <arx/ARXFlatBufferMgr.h>
#include <arx/ARXImageBufferMgr.h>
#include <arx/ARXFlatBuffer.h>
#include <arx/ARXBufferTypes.h>
#include <arx/ARXJniUtil.h>
#include <arx_debug.h>

using namespace tiarx;
using namespace android;

ARCameraJniContext::ARCameraJniContext(JNIEnv *env, jobject obj)
{
    mutex_init(&mLock);
    mObj = env->NewGlobalRef(obj);
    env->GetJavaVM(&mVm);
    jclass cls = env->GetObjectClass(obj);
    mOnARXDeath = env->GetMethodID(cls, "onARXDeath", "()V");
    mOnBufferChanged = env->GetMethodID(cls, "onCamPoseUpdate", "(JI)V");
    mFloatArray = NULL;
}

ARCameraJniContext::~ARCameraJniContext()
{
    if (mArx) {
        mArx->destroy();
    }
    //A callback may be currently running, acquire mutex
    //before releasing reference to the float array ref
    mutex_lock(&mLock);
    JNIEnv *env = getJNIEnv();
    if (mFloatArray) {
        env->DeleteGlobalRef(mFloatArray);
    }
    mutex_unlock(&mLock);
    mutex_deinit(&mLock);
    env->DeleteGlobalRef(mObj);
}

bool ARCameraJniContext::setup(jfloatArray array, jobject surface)
{
    JNIEnv *env = getJNIEnv();

    mArx = ARAccelerator::create(this);
    if (mArx == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Could not create ARX instance!");
        return false;
    }

    arxstatus_t status = mArx->loadEngine("arxpose");
    if (status != NOERROR) {
        ARX_PRINT(ARX_ZONE_ERROR, "Could not load camera pose engine!");
        return false;
    }

    ARXImageBufferMgr *prevMgr = mArx->getImageBufferMgr(BUFF_CAMOUT);
    if (prevMgr == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Could not obtain main cam buffer manager!");
        return false;
    }

    ARXFlatBufferMgr *mgr = mArx->getFlatBufferMgr(BUFF_CAMERA_POSE);
    if (mgr == NULL) {
       ARX_PRINT(ARX_ZONE_ERROR, "Could not obtain camera matrix manager!");
       return false;
    }

    // Configure the images that will be generated through this buffer
    // stream.
    prevMgr->setSize(1280, 960);
    Surface *surf = get_surface(env, mObj, surface);
    if (prevMgr->bindSurface(surf) != NOERROR) {
        ARX_PRINT(ARX_ZONE_ERROR, "Could not bind surface to main cam output!");
        return false;
    }
    prevMgr->hold(true);

    prevMgr = mArx->getImageBufferMgr(BUFF_CAMOUT2);
    if (prevMgr == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Could not obtain sec cam buffer manager!");
        return false;
    }
    prevMgr->setSize(320, 240);

    mgr->registerClient(this);

    mFloatArray = (jfloatArray)env->NewGlobalRef(array);
    mArx->setProperty(PROP_ENGINE_STATE, ENGINE_STATE_START);

    return true;
}

void ARCameraJniContext::onPropertyChanged(uint32_t property, int32_t value)
{
    if (property == PROP_ENGINE_STATE && value == ENGINE_STATE_DEAD) {
        JNIEnv *env = getJNIEnv();
        env->CallVoidMethod(mObj, mOnARXDeath);
    }
}

void ARCameraJniContext::onBufferChanged(ARXFlatBuffer *pBuffer)
{
    ARXCameraPoseMatrix *m = reinterpret_cast<ARXCameraPoseMatrix *>(pBuffer->data());
    ARX_PRINT(ARX_ZONE_CLIENT, "ts:%llu, rx[%0.2f %0.2f %0.2f] ry[%0.2f %0.2f %0.2f] rz[%0.2f %0.2f %0.2f] t[%1.2f %1.2f %1.2f]",
                pBuffer->timestamp(),
                m->matrix[0], m->matrix[4], m->matrix[8],
                m->matrix[1], m->matrix[5], m->matrix[9],
                m->matrix[2], m->matrix[6], m->matrix[10],
                m->matrix[3], m->matrix[7], m->matrix[11]);
    JNIEnv *env = getJNIEnv();
    mutex_lock(&mLock);
    //copy data
    env->SetFloatArrayRegion(mFloatArray, 0, 12, (jfloat *)m->matrix);
    mutex_unlock(&mLock);
    pBuffer->release();
    env->CallVoidMethod(mObj, mOnBufferChanged, (jlong)pBuffer->timestamp(), m->status);
}

void ARCameraJniContext::reset()
{
    mArx->setProperty(PROP_ENGINE_CAMPOSE_RESET, true);
}

void ARCameraJniContext::render(jlong ts)
{
    ARXImageBufferMgr *mgr = mArx->getImageBufferMgr(BUFF_CAMOUT);
    mgr->render((uint64_t)ts);
}

JNIEnv *ARCameraJniContext::getJNIEnv()
{
    union {
        JNIEnv *env;
        void *env_void;
    };
    env = NULL;
    jint ret;
    ret = mVm->GetEnv(&env_void, JNI_VERSION_1_6);
    if (ret == JNI_EDETACHED) {
        ARX_PRINT(ARX_ZONE_ERROR, "the thread is not attached to JNI!\n");
    }
    return env;
}
