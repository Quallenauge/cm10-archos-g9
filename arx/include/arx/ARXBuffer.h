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

#ifndef _ARXBUFFER_H_
#define _ARXBUFFER_H_

#include <stdint.h>

#include <arx/ARXStatus.h>

namespace tiarx {

/*!
 * \brief A base class to represent Buffer objects obtained from Buffer managers
 */
class ARXBuffer
{
public:
    /*!
     * Releases ownership back to the buffer manager
     */
    virtual arxstatus_t release() = 0;
    /*!
     * Queries the buffer ID associated with this buffer
     * @see BufferTypes_e
     */
    inline uint32_t id() const { return mId; }
    /*!
     * Queries the timestamp of this buffer. The timestamp is obtained from the source
     * that generated this buffer. For example, if this buffer was generated from a camera source,
     * the timestamp used will be the same as the camera buffer.
     */
    inline uint64_t timestamp() const { return mTimestamp; }

protected:
    /*!
     * Non-public constructor.
     */
    ARXBuffer() {};
    /*!
     * Non-public copy constructor.
     */
    ARXBuffer(const ARXBuffer&);
    /*!
     * Non-public assignment operator.
     */
    ARXBuffer& operator=(const ARXBuffer&);
    /*!
     * Non-public destructor.
     */
    virtual ~ARXBuffer() {};

    uint32_t mId;
    uint64_t mTimestamp;
};

}
#endif //_ARXBUFFER_H_
