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
#include <arx/ARXJniUtil.h>

#if defined(JELLYBEAN) || defined(JELLYBEANMR1)
#include <gui/ISurface.h>
#include <gui/Surface.h>
#else
#include <surfaceflinger/ISurface.h>
#include <surfaceflinger/Surface.h>
#endif

using namespace android;
namespace tiarx {

android::Surface* get_surface(JNIEnv *env, jobject, jobject jSurface)
{
    jclass surface = env->FindClass("android/view/Surface");
    jfieldID jniID = env->GetFieldID(surface, ANDROID_VIEW_SURFACE_JNI_ID, "I");
    return reinterpret_cast<android::Surface*>(env->GetIntField(jSurface, jniID));
}

android::ISurfaceTexture* get_surfaceTexture(JNIEnv *env, jobject, jobject jSurfaceTexture)
{
    jclass surface = env->FindClass("android/graphics/SurfaceTexture");
    jfieldID jniID = env->GetFieldID(surface, ANDROID_GRAPHICS_SURFACETEXTURE_JNI_ID, "I");
    SurfaceTexture* surfTex = reinterpret_cast<SurfaceTexture*>(env->GetIntField(jSurfaceTexture, jniID));
#if defined(JELLYBEAN) || defined(JELLYBEANMR1)
    sp<ISurfaceTexture> surfaceTexture = surfTex->getBufferQueue();
#else
    sp<ISurfaceTexture> surfaceTexture = interface_cast<ISurfaceTexture>(surfTex->asBinder());
#endif
    return surfaceTexture.get();
}

}
