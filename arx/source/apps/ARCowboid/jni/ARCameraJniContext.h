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

#ifndef _SURFTRACKERJNI_H_
#define _SURFTRACKERJNI_H_

#include <jni.h>
#include <stdint.h>

#include <arx/ARAccelerator.h>
#include <arx/ARXBufferListener.h>
#include <arx/ARXProperties.h>

#include <sosal/mutex.h>

namespace tiarx {

class ARCameraJniContext :
    public ARXPropertyListener,
    public ARXFlatBufferListener
{
public:
    ARCameraJniContext(JNIEnv *env, jobject obj);
    ~ARCameraJniContext();

    bool setup(jfloatArray array, jobject surface);
    void reset();
    void render(jlong ts);
    void onPropertyChanged(uint32_t property, int32_t value);
    void onBufferChanged(ARXFlatBuffer *pBuffer);

private:

    JNIEnv *getJNIEnv();

    JavaVM *mVm;
    jobject mObj;
    jmethodID mOnARXDeath;
    jmethodID mOnBufferChanged;

    jfloatArray mFloatArray;
    ARAccelerator *mArx;

    mutex_t mLock;
};

}
#endif
