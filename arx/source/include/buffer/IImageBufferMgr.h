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


#ifndef _IIMAGEBUFFERMGR_H_
#define _IIMAGEBUFFERMGR_H_

#include <buffer/IBufferMgr.h>

namespace android {
    class Surface;
    class ISurfaceTexture;
}

namespace tiarx {

class ImageBuffer;

class IImageBufferMgr :
    public virtual IBufferMgr,
    public android::IInterface
{
public:
    DECLARE_META_INTERFACE(ImageBufferMgr);

    /*!
     * Binds a Surface to the specified buffer ID replacing a previously
     * bounded Surface or SurfaceTexture.
     * The data buffers for this queue will be allocated from the given Surface.
     * @see bindSurface(const android::sp<android::ISurfaceTexture>& texture)
     */
    virtual arxstatus_t bindSurface(const android::sp<android::Surface>& surface) = 0;

    /*!
     * Binds a SurfaceTexture to the specified buffer ID replacing a previously
     * bounded Surface or SurfaceTexture.
     * The data buffers for this queue will be allocated from the given SurfaceTexture.
     * @see bindSurface(const android::sp<android::Surface>& surface)
     */
    virtual arxstatus_t bindSurfaceTexture(const android::sp<android::ISurfaceTexture>& texture) = 0;

    /*!
     * Request the buffer for the corresponding index. Used to cache the buffer list in the client side
     * to avoid serializing all the time during callbacks.
     */
    virtual arxstatus_t getImage(uint32_t index, android::sp<ImageBuffer> *buffer) = 0;

    /*!
     * Post an update to the compositor if a there is a bounded Surface then releases ownership back to the
     * manager. If there is no bounded surface, this behaves like release. This is used in case a client does
     * not want callbacks, but would like to hold the data until the client decides to post it.
     * @see hold()
     */
    virtual arxstatus_t render(uint64_t timestamp) = 0;

    /*!
     * Post an update to the compositor if a there is a bounded Surface then releases ownership back to the
     * manager. If there is no bounded surface, this behaves as if you called release.
     * @see bindSurfaceTexture()
     */
    virtual arxstatus_t render(uint32_t index) = 0;

    /*!
     * Signals the manager to hold any buffer that is ready until the client calls post(uint64_t timestamp)
     * Returns NOTCONFIGURABLE if no surface has been bound
     * @see post(uint64_t timestamp)
     */
    virtual arxstatus_t hold(bool enable) = 0;

    /*!
     * Sets the size used when allocating buffers. A default format is chosen by the manager.
     */
    virtual arxstatus_t setSize(uint32_t width, uint32_t height) = 0;

    /*!
     * Sets the size and format used when allocating buffers.
     */
    virtual arxstatus_t setFormat(uint32_t format) = 0;

};

}
#endif
