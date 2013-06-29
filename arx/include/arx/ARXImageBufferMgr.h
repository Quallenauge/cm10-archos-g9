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

#ifndef _ARXIMAGEBUFFERMGR_
#define _ARXIMAGEBUFFERMGR_

#include <arx/ARXBufferMgr.h>

namespace android {
    class Surface;
    class ISurfaceTexture;
}

namespace tiarx {

class ARXImageBufferListener;

/*!
 * \brief The image buffer manager interface, used to configure image data streams
 */
class ARXImageBufferMgr : public ARXBufferMgr
{
public:
    /*!
     * Binds a Surface to the specified buffer ID replacing a previously
     * bounded Surface or SurfaceTexture.
     * The data buffers will be allocated from the given Surface.
     * @see bindSurface(android::Surface *surface)
     */
    virtual arxstatus_t bindSurface(android::Surface *surface) = 0;

    /*!
     * Binds a SurfaceTexture to the specified buffer ID replacing a previously
     * bounded Surface or SurfaceTexture.
     * The data buffers will be allocated from the given SurfaceTexture.
     * @see bindSurface(android::Surface *surface)
     */
    virtual arxstatus_t bindSurface(android::ISurfaceTexture *surfaceTexture) = 0;

    /*!
     * Renders the buffer associated with timestamp. For this to work the client has to
     * call hold first, to inform ARX to not use the buffers until they are rendered via this call.
     * @param timestamp The timestamp of the buffer to render
     * @see bindSurface()
     * @see hold()
     */
    virtual arxstatus_t render(uint64_t timestamp) = 0;

    /*!
     * Enables ARX to hold buffers when the client does not register for callbacks. This is useful to
     * synchronize for example graphics updates done by the client with rendering updates done by ARX.
     * This does nothing if there is no surface bound to this manager.
     * @see bindSurface()
     * @see render()
     */
    virtual arxstatus_t hold(bool enable) = 0;

    /*!
     * Allows the client to receive callbacks for this buffer ID. Caller will pass a listener
     * object that implements the callback, or NULL if notifications are to be disabled.
     */
    virtual arxstatus_t registerClient(ARXImageBufferListener *listener) = 0;

    /*!
     * Sets the size used when allocating buffers. A default format is chosen by the manager.
     * Some buffer managers will not allow configuration changes.
     */
    virtual arxstatus_t setSize(uint32_t width, uint32_t height) = 0;

    /*!
     * Sets the format used when allocating buffers.
     * Some buffer managers will not allow configuration changes.
     */
    virtual arxstatus_t setFormat(uint32_t format) = 0;

protected:
    /*!
     * Non-public constructor.
     */
    ARXImageBufferMgr() {};
    /*!
     * Non-public copy constructor.
     */
    ARXImageBufferMgr(const ARXImageBufferMgr&);
    /*!
     * Non-public assignment operator.
     */
    ARXImageBufferMgr& operator=(const ARXImageBufferMgr&);
    /*!
     * Non-public destructor.
     */
    virtual ~ARXImageBufferMgr() {};
};

}// namespace
#endif //_ARXIMAGEBUFFERMGR_
