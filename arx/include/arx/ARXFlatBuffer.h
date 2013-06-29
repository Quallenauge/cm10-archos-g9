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

#ifndef _ARXFLATBUFFER_H_
#define _ARXFLATBUFFER_H_

#include <arx/ARXBuffer.h>

namespace tiarx {

/*!
 * \brief A generic buffer interface
 */
class ARXFlatBuffer : public ARXBuffer
{
public:
    /*!
     * Queries the size of the buffer.
     * @returns the number of bytes used for this buffer
     */
    inline uint32_t size() const { return mSize; }
    /*!
     * Queries for the raw pointer to access the data of this buffer
     * @returns the raw pointer of the underlying buffer represented by this object
     */
    inline void *data() const { return mData; }

protected:
    /*!
     * Non-public constructor.
     */
    ARXFlatBuffer() {};
    /*!
     * Non-public copy constructor.
     */
    ARXFlatBuffer(const ARXFlatBuffer&);
    /*!
     * Non-public assignment operator.
     */
    ARXFlatBuffer& operator=(const ARXFlatBuffer&);
    /*!
     * Non-public destructor.
     */
    virtual ~ARXFlatBuffer() {};

    uint32_t mSize;
    void *mData;
};

}
#endif //_ARXFLATBUFFER_H_
