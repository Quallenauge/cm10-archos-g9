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

using namespace tiarx;

static JavaVM *g_jvm = NULL; // caches our Java virtual machine

#define LOGD(x) __android_log_print(ANDROID_LOG_DEBUG, "ARXStreams", x);
#define LOGE(x) __android_log_print(ANDROID_LOG_ERROR, "ARXStreams", x);

/*!
 * Updates a Java object with a pointer to the native context that will be
 * used to control ARX. Obviously, this context will only be used in JNI code.
 */
static void setContext(JNIEnv *env, jobject thiz, ARXStreamsJniContext *value)
{
    jclass clazz = env->GetObjectClass(thiz);
    jfieldID field = env->GetFieldID(clazz, "context", "I");
    env->SetIntField(thiz, field, (int)value);
}

/*!
 * Retrieves the native context associated with the given Java object.
 */
static ARXStreamsJniContext *getContext(JNIEnv *env, jobject thiz)
{
    jclass clazz = env->GetObjectClass(thiz);
    jfieldID field = env->GetFieldID(clazz, "context", "I");
    return reinterpret_cast<ARXStreamsJniContext *>(env->GetIntField(thiz, field));
}

/*!
 * Open and start ARX after configuring the buffer streams we are interested in.
 * @param jSurface a reference to the Surface where we will render the main camera stream.
 * @param jSurfaceTex a reference to the SurfaceTexture where we will render the secondary camera stream.
 * @param bitmap Bitmap object where the sobel output will be drawn
 */
static jboolean createARX(JNIEnv* env, jobject thiz, jobject jSurface, jobject jSurfaceTex, jobject bitmap)
{
    ARXStreamsJniContext *context = new ARXStreamsJniContext(g_jvm, env, thiz);

    // Create a new instance of the ARX client. Note that our context
    // is also our ARX property listener.
    ARAccelerator *arx = ARAccelerator::create(context);
    if (arx != NULL) {
        if (context->setup(arx, jSurface, jSurfaceTex, bitmap)) {
            // Store the native context in a int field of the ARXSample Java class
            setContext(env, thiz, context);
            // Start the ARX engine (which will cause it to begin generating
            // the buffer streams).
            arx->setProperty(PROP_ENGINE_STATE, ENGINE_STATE_START);
            return (jboolean)true;
        }
    }

    // We could not create or setup an ARX object!
    delete context;
    setContext(env, thiz, NULL);
    return (jboolean)false;
}

/*!
 * Stop ARX and release the resources it is using.
 */
static void destroyARX(JNIEnv* env, jobject thiz)
{
    ARXStreamsJniContext *context = getContext(env, thiz);
    if (context != NULL) {
        setContext(env, thiz, NULL);
        delete context;
    }
}

static const JNINativeMethod g_methods[] = {
    { "createARX",          "(Landroid/view/Surface;Landroid/graphics/SurfaceTexture;Landroid/graphics/Bitmap;)Z", (void*)createARX },
    { "destroyARX",         "()V", (void*)destroyARX }
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
        LOGE("Could not get JNI environment!");
        return -1;
    }

    jclass cARXStreams = env->FindClass("com/ti/arx/ARXStreams/ARXStreamsActivity");
    if (env->RegisterNatives(cARXStreams, g_methods, sizeof(g_methods)/sizeof(g_methods[0])) < 0) {
        LOGE("Failed registering native methods!");
        return -1;
    }

    g_jvm = vm;

    return JNI_VERSION_1_6;
}
