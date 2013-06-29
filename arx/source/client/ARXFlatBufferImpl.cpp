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

#include <ARXFlatBufferImpl.h>
#include <ARXFlatBufferMgrImpl.h>

#include <buffer/FlatBuffer.h>

using namespace android;

namespace tiarx {

ARXFlatBufferImpl::ARXFlatBufferImpl(uint32_t bufID, uint32_t index,
        const sp<ARXFlatBufferMgrImpl>& mgr, const sp<FlatBuffer>& buffer)
{
    mIndex = index;
    mBuffer = buffer,
    mMgr = mgr;
    mId = bufID;
    mTimestamp = 0;

    DVP_Buffer_t buf;
    buffer->copy(&buf);
    mSize = buf.numBytes;
    mData = buf.pData;
}

ARXFlatBufferImpl::~ARXFlatBufferImpl()
{
    mMgr.clear();
    mBuffer.clear();
}

arxstatus_t ARXFlatBufferImpl::release()
{
    decStrong(this);
    return mMgr->release(mIndex);
}

}
