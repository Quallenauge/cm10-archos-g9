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

#include <buffer/FlatBuffer.h>
#include <buffer/FlatBufferMgr.h>
#include <arx_debug.h>

using namespace android;

namespace tiarx {

FlatBufferMgr::FlatBufferMgr(
        uint32_t bufId, uint32_t size, DVP_MemType_e memType,
        uint32_t count, bool lockConfig, bool holdReady) :
        BufferMgr(bufId, count, memType, lockConfig, holdReady)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    mSize = size;
}

FlatBufferMgr::~FlatBufferMgr()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    free();
}

arxstatus_t FlatBufferMgr::allocateBuffer(uint32_t index)
{
    sp<FlatBuffer> b = new FlatBuffer(index, mSize, mDvp, mMemType, mClient, mFreeQueue, mReadyQueue);
    arxstatus_t status = b->allocate();
    if (status != NOERROR) {
        ARX_PRINT(ARX_ZONE_ERROR, "failed allocating buffer! (0x%x)\n", status);
        return status;
    }

    mBuffers[index] = b;
    return NOERROR;
}

arxstatus_t FlatBufferMgr::free()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    for (uint32_t i = 0; i < IBufferMgr::MAX_BUFFERS; i++) {
        mBuffers[i].clear();
    }

    return BufferMgr::free();
}

arxstatus_t FlatBufferMgr::getBuffer(uint32_t index, sp<FlatBuffer> *buffer)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    if (index >= mCount) {
        ARX_PRINT(ARX_ZONE_ERROR, "%s: index(%d) is out of bounds(%d)!\n", __FUNCTION__, index, mCount);
        return INVALID_ARGUMENT;
    }

    if (!mAllocated) {
        ARX_PRINT(ARX_ZONE_ERROR, "%s: buffers have not been allocated yet!\n", __FUNCTION__);
        return INVALID_STATE;
    }

    *buffer = mBuffers[index];
    return NOERROR;
}

arxstatus_t FlatBufferMgr::setSize(uint32_t size)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    if (!mLockedConfig) {
        mSize = size;
    } else {
        return NOTCONFIGURABLE;
    }
    return NOERROR;
}

arxstatus_t FlatBufferMgr::release(uint32_t index)
{
    if (!mAllocated) {
        return UNALLOCATED_RESOURCES;
    }

    if (index >= mCount) {
        return INVALID_ARGUMENT;
    }

    const sp<FlatBuffer>& b = mBuffers[index];
    b->release();
    return NOERROR;
}

FlatBuffer *FlatBufferMgr::nextFree()
{
    uint32_t idx;
    bool ret = queue_read(mFreeQueue, true_e, &idx);
    if (ret) {
        const sp<FlatBuffer>& b = mBuffers[idx];
        ARX_PRINT(ARX_ZONE_BUFFER, "mgr: %d, a buffer is free: %p, idx:%d", mBuffId, b.get(), idx);
        return b.get();
    }
    return NULL;
}

FlatBuffer *FlatBufferMgr::nextReady()
{
    uint32_t idx;
    bool ret = queue_read(mReadyQueue, true_e, &idx);
    if (ret) {
        const sp<FlatBuffer>& b = mBuffers[idx];
        ARX_PRINT(ARX_ZONE_BUFFER, "mgr: %d, a buffer is ready: %p, idx:%d", mBuffId, b.get(), idx);
        return b.get();
    }
    return NULL;
}

} // namespace
