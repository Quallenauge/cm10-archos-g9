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

#ifndef _ARXFLATBUFFERIMPL_H_
#define _ARXFLATBUFFERIMPL_H_

#include <utils/RefBase.h>

#include <arx/ARXFlatBuffer.h>

namespace tiarx {

class FlatBuffer;
class ARXFlatBufferMgrImpl;

class ARXFlatBufferImpl :
    public ARXFlatBuffer,
    public android::LightRefBase<ARXFlatBufferImpl>
{
friend class ARXFlatBufferMgrImpl;
public:

    virtual ~ARXFlatBufferImpl();

    //From ARXBuffer
    virtual arxstatus_t release();

private:
    /*!
     * Non-public constructor.
     */
    ARXFlatBufferImpl(uint32_t bufID, uint32_t index,
                  const android::sp<ARXFlatBufferMgrImpl>& mgr,
                  const android::sp<FlatBuffer>& buf);
    /*!
     * Non-public copy constructor.
     */
    ARXFlatBufferImpl(const ARXFlatBufferImpl&);
    /*!
     * Non-public assignment operator.
     */
    ARXFlatBufferImpl& operator=(const ARXFlatBufferImpl&);

    inline void setTimestamp(uint64_t timestamp) { mTimestamp = timestamp; }

    uint32_t mIndex;
    android::sp<FlatBuffer> mBuffer;
    android::sp<ARXFlatBufferMgrImpl> mMgr;
};

}
#endif // _ARXFLATBUFFERIMPL_H_
