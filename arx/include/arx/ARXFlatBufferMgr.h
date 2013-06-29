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

#ifndef _ARXFLATBUFFERMGR_
#define _ARXFLATBUFFERMGR_

#include <arx/ARXBufferMgr.h>

namespace tiarx {

class ARXFlatBufferListener;

/*!
 * \brief The flat buffer manager interface, used to configure generic buffer streams
 */
class ARXFlatBufferMgr : public ARXBufferMgr
{
public:

    /*!
     * Allows the client to receive callbacks for this buffer ID. Caller will pass a listener
     * object that implements the callback, or NULL if notifications are to be disabled.
     */
    virtual arxstatus_t registerClient(ARXFlatBufferListener *listener) = 0;

    /*!
     * Sets the size used when allocating buffers. If the manager does not allow the client to configure
     * this property it returns NOTCONFIGURABLE.
     * Once the buffers have been allocated, this call fails with INVALID_STATE
     */
    virtual arxstatus_t setSize(uint32_t size) = 0;

protected:
    /*!
     * Non-public constructor.
     */
    ARXFlatBufferMgr() {};
    /*!
     * Non-public copy constructor.
     */
    ARXFlatBufferMgr(const ARXFlatBufferMgr&);
    /*!
     * Non-public assignment operator.
     */
    ARXFlatBufferMgr& operator=(const ARXFlatBufferMgr&);
    /*!
     * Non-public destructor.
     */
    virtual ~ARXFlatBufferMgr() {};
};

}// namespace
#endif //_ARXFLATBUFFERMGR_
