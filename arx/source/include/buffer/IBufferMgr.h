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

#ifndef _IBUFFERMGR_H_
#define _IBUFFERMGR_H_

#include <binder/IInterface.h>

#include <arx/ARXStatus.h>
#include <dvp/dvp_types.h>

namespace tiarx {

class IBufferMgrClient;

class IBufferMgr
{
public:
    static const uint32_t MAX_BUFFERS = 16;

    /*!
     * Allows running a custom graph to generate and output corresponding to this buffer ID
     */
    virtual arxstatus_t bindGraph(DVP_KernelGraph_t *pGraph) = 0;

    /*!
     * Sets the number of buffers to use for this manager
     */
    virtual arxstatus_t setCount(uint32_t count) = 0;

    /*!
     * Gets the number of buffers to use for this manager
     */
    virtual uint32_t getCount() const = 0;

    /*!
     * Assigns ownership of the buffer back to A. Must always be called for all buffers delivered
     * on a callback.
     */
    virtual arxstatus_t release(uint32_t index) = 0;

    /*!
     * Allows the client to receive callbacks for this buffer ID.
     */
    virtual arxstatus_t registerClient(IBufferMgrClient *client) = 0;

protected:
    ~IBufferMgr() {}

};

} // namespace

#endif //_IBUFFERMGR_H_
