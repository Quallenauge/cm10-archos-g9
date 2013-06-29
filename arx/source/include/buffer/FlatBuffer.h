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

#ifndef _ARX_FLAT_BUFFER_H
#define _ARX_FLAT_BUFFER_H

#include <buffer/Buffer.h>
#include <arx/ARXStatus.h>
#include <dvp/dvp_types.h>

namespace tiarx {

class FlatBuffer : public Buffer
{
public:
    FlatBuffer(uint32_t index, uint32_t size, DVP_Handle dvp, DVP_MemType_e type,
               android::sp<IBufferMgrClient>& client, queue_t *freeQ, queue_t *readyQ);
    virtual ~FlatBuffer();

    arxstatus_t copy(DVP_Buffer_t *image);
    arxstatus_t import(DVP_Handle dvp);
    arxstatus_t allocate();
    arxstatus_t free();

    inline void *data() const { return mBuffer.pData; }

    static status_t writeToParcel(const sp<FlatBuffer>& buffer, Parcel* parcel);
    static sp<FlatBuffer> readFromParcel(const Parcel& data);

private:
    FlatBuffer();
    bool internalRelease();

    uint32_t mSize;
    DVP_Buffer_t mBuffer;
    DVP_S32 mSharedFd;
    DVP_VALUE mImportHdl;
};

}
#endif //_ARX_FLAT_BUFFER_H
