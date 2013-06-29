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

#include <jni.h>
#include <stdint.h>
#include <pthread.h>
#include <android/bitmap.h>
#include <android/log.h>

#include <arx/ARAccelerator.h>
#include <arx/ARXImageBuffer.h>
#include <arx/ARXImageBufferMgr.h>
#include <arx/ARXBufferListener.h>
#include <arx/ARXProperties.h>

/*!
 * This class provides a context through which the ARXSample Java
 * application can control the TI Augmented Reality Accelerator
 * (ARX) in native space.
 *
 * An instance of the tiarx::ARAccelerator class which is passed
 * to the context using setARXClient serves as the primary interface
 * for the application to the ARX daemon. This object pointer is used by
 * the context to access individual ARX buffer managers, start
 * and stop ARX and respond to ARX events.
 *
 * @see tiarx::ARAccelerator
 */
class ARXStreamsJniContext :
    public tiarx::ARXPropertyListener,
    public tiarx::ARXImageBufferListener
{
public:
    ARXStreamsJniContext(JavaVM *vm, JNIEnv *env, jobject app);
    ~ARXStreamsJniContext();

    /*!
     * Sets up the ARX buffer managers.
     * @return true if succesful
     */
    bool setup(tiarx::ARAccelerator *arx, jobject jSurface, jobject jSurfaceTex, jobject bitmap);

    /*!
     * Called by ARX when a property changes values (part of the
     * ARXPropertyListener interface).
     * @see tiarx::ARXPropertyListener
     */
    void onPropertyChanged(uint32_t property, int32_t value);

    /*!
     * Called by ARX when a buffer's contents change.
     *
     * In this application, we use this to listen for changes to the Sobel buffer.
     * When a buffer's contents are updated, we copy them to our Java bitmap.
     * @param pImage the buffer object that has changed.
     * @see tiarx::ARXImageBufferListener
     */
    void onBufferChanged(tiarx::ARXImageBuffer *pImage);

private:
    /*!
     * Helper method to get the JNI environment from our cached virtual machine.
     */
    JNIEnv *getJNIEnv();

    JavaVM *mVm;
    //Instance of the java class calling this jni code
    jobject mCallerInstance;

    //callback method id to notify java app when ARX dies
    jmethodID mOnARXDeath;
    jmethodID mOnBufferChanged;

    tiarx::ARAccelerator *mArx;

    //Protects bitmap access between callback and teardown
    pthread_mutex_t mBmpLock;
    jobject mBitmap;
    AndroidBitmapInfo mBmpInfo;
};

