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

#ifndef _ARXJNIUTIL_H_
#define _ARXJNIUTIL_H_

/*!
 * \file ARXJniUtil.h
 * \brief Utility methods to extract native Surface/SurfaceTexture pointers from their java counterparts
 */
namespace android {
    class ISurfaceTexture;
    class Surface;
}

namespace tiarx {
/*!
 * Gets the native Surface pointer from a java Surface object
 */
class android::Surface *get_surface(JNIEnv *env, jobject thiz, jobject jSurface);
/*!
 * Gets the native SurfaceTexture pointer from a java SurfaceTexture object
 */
class android::ISurfaceTexture *get_surfaceTexture(JNIEnv *env, jobject thiz, jobject jSurfaceTexture);

}
#endif //_ARXJNIUTIL_H_
