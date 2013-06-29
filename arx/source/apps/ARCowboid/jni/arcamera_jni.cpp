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

#include <JNIHelp.h>
#include "ARCameraJniContext.h"

#include <arx_debug.h>

using namespace tiarx;

/*!
 * Updates a Java object with a pointer to the native context that will be
 * used to control ARX. Obviously, this context will only be used in JNI code.
 */
static void setContext(JNIEnv *env, jobject thiz, ARCameraJniContext *value)
{
    jclass clazz = env->GetObjectClass(thiz);
    jfieldID field = env->GetFieldID(clazz, "context", "I");
    env->SetIntField(thiz, field, (int)value);
}

/*!
 * Retrieves the native context associated with the given Java object.
 */
static ARCameraJniContext *getContext(JNIEnv *env, jobject thiz)
{
    jclass clazz = env->GetObjectClass(thiz);
    jfieldID field = env->GetFieldID(clazz, "context", "I");
    return reinterpret_cast<ARCameraJniContext *>(env->GetIntField(thiz, field));
}

/*!
 * Open and start ARX after configuring the buffer streams we are interested in.
 * @param jSurface a reference to the Surface where we will render the main camera stream.
 * @param jSurfaceTex a reference to the SurfaceTexture where we will render the secondary camera stream.
 * @param bitmap Bitmap object where the sobel output will be drawn
 */
static jboolean create(JNIEnv* env, jobject thiz, jfloatArray array, jobject surface)
{
    ARCameraJniContext *context = new ARCameraJniContext(env, thiz);
    if (context != NULL) {
        if (context->setup(array, surface)) {
            setContext(env, thiz, context);
            return (jboolean)true;
        }
    }
    delete context;
    // We could not create or setup the jni context
    setContext(env, thiz, NULL);
    return (jboolean)false;
}

/*!
 * Stop ARX and release the resources it is using.
 */
static void destroy(JNIEnv* env, jobject thiz)
{
    ARCameraJniContext *context = getContext(env, thiz);
    if (context != NULL) {
        setContext(env, thiz, NULL);
        delete context;
    }
}

static void reset(JNIEnv* env, jobject thiz)
{
    ARCameraJniContext *context = getContext(env, thiz);
    if (context != NULL) {
        context->reset();
    }
}

static void render(JNIEnv* env, jobject thiz, jlong ts)
{
    ARCameraJniContext *context = getContext(env, thiz);
    if (context != NULL) {
        context->render(ts);
    }
}

static const JNINativeMethod g_methods[] = {
    { "create",          "([FLandroid/view/Surface;)Z", (void*)create },
    { "destroy",         "()V",                         (void*)destroy },
    { "reset",           "()V",                         (void*)reset },
    { "renderPreview",   "(J)V",                        (void*)render }
};

/////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved __attribute__((unused)))
{
    union {
        JNIEnv *env;
        void *env_void;
    };

    env = NULL;
    if (vm->GetEnv(&env_void, JNI_VERSION_1_6) != JNI_OK || !env) {
        ARX_PRINT(ARX_ZONE_ERROR, "Could not get JNI environment!");
        return -1;
    }

    if (jniRegisterNativeMethods(env, "com/ti/arx/ARCowboid/ARCamera", g_methods, NELEM(g_methods)) < 0) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed registering native methods!");
        return -1;
    }

    return JNI_VERSION_1_6;
}
