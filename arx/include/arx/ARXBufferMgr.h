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

#ifndef _ARXBUFFERMGR_
#define _ARXBUFFERMGR_

#include <arx/ARXStatus.h>
#include <dvp/dvp_types.h>

namespace android {
    class Surface;
    class ISurfaceTexture;
}

namespace tiarx {

/*!
 * \brief The base manager interface used to configure a buffer stream
 */
class ARXBufferMgr
{
public:
    /*!
     * Allows running a custom graph to generate the output represented by this manager
     * @note This is not supported on this release
     */
    virtual arxstatus_t bindGraph(DVP_KernelGraph_t *pGraph) = 0;

    /*!
     * Sets the number of buffers to use for this manager
     * @param count the number of buffers to use
     * @returns INVALID_STATE if the manager's resources have already been allocated
     */
    virtual arxstatus_t setCount(uint32_t count) = 0;

    /*!
     * Gets the number of buffers used for this manager
     * @returns the number of buffers used
     */
    virtual uint32_t getCount() = 0;

protected:
    /*!
     * Non-public constructor.
     */
    ARXBufferMgr() {};
    /*!
     * Non-public copy constructor.
     */
    ARXBufferMgr(const ARXBufferMgr&);
    /*!
     * Non-public assignment operator.
     */
    ARXBufferMgr& operator=(const ARXBufferMgr&);
    /*!
     * Non-public destructor.
     */
    virtual ~ARXBufferMgr() {};
};

}// namespace
#endif //_ARXBUFFERMGR_
