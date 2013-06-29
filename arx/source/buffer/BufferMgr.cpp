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

#include <buffer/BufferMgr.h>
#include <buffer/IBufferMgrClient.h>
#include <arx/ARXBufferTypes.h>
#include <arx_debug.h>

using namespace android;

namespace tiarx {


BufferMgr::BufferMgr(uint32_t bufId, uint32_t count, DVP_MemType_e memType, bool lockConfig, bool holdReady)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    mBuffId = bufId;
    mEnabled = false;
    mAllocated = false;
    mLockedConfig = lockConfig;

    mFreeQueue = NULL;
    mReadyQueue = NULL;
    mCount = count;

    mGraph = NULL;
    mDvp = 0;
    mMemType = memType;

    mHoldReady = holdReady;
}

BufferMgr::~BufferMgr()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
}

arxstatus_t BufferMgr::bindGraph(DVP_KernelGraph_t *)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    return NOERROR;
}

arxstatus_t BufferMgr::setCount(uint32_t count)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    if (mAllocated) {
        ARX_PRINT(ARX_ZONE_ERROR, "%s: Cannot change count after allocation!\n", __FUNCTION__);
        return INVALID_STATE;
    }
    mCount = count;
    return NOERROR;
}

uint32_t BufferMgr::getCount() const
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    return mCount;
}

arxstatus_t BufferMgr::registerClient(IBufferMgrClient *client)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    mClient = client;

    mEnabled = mClient != NULL ? true : false;
    return NOERROR;
}

arxstatus_t BufferMgr::allocate(DVP_Handle dvp)
{
    if (!mEnabled) {
        ARX_PRINT(ARX_ZONE_WARNING, "mgr:%u %s - not enabled!\n", mBuffId, __FUNCTION__);
        return NOERROR;
    }

    if (mAllocated) {
        ARX_PRINT(ARX_ZONE_WARNING, "mgr:%u %s - already allocated!\n", mBuffId, __FUNCTION__);
        return INVALID_STATE;
    }

    mDvp = dvp;
    mFreeQueue = queue_create(mCount, sizeof(uint32_t));
    if (mFreeQueue == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "failed allocating free queue!\n");
        return NO_MEMORY;
    }

    if (mHoldReady) {
        mReadyQueue = queue_create(mCount, sizeof(uint32_t));
        if (mReadyQueue == NULL) {
            ARX_PRINT(ARX_ZONE_ERROR, "failed allocating filled queue!\n");
            return NO_MEMORY;
        }
    }

    mDvp = dvp;
    for (uint32_t i = 0; i < mCount; i++) {
        arxstatus_t status = allocateBuffer(i);
        if (status != NOERROR) {
            ARX_PRINT(ARX_ZONE_ERROR, "failed allocating buffer! (0x%x)\n", status);
            free();
            return status;
        }
        if (!queue_write(mFreeQueue, false_e, &i)) {
            ARX_PRINT(ARX_ZONE_ERROR, "failed to write buffer index into free queue!\n");
            free();
            return FAILED;
        }
    }
    mAllocated = true;
    return NOERROR;
}

arxstatus_t BufferMgr::free()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);

    if (mFreeQueue != NULL) {
        queue_destroy(mFreeQueue);
        mFreeQueue = NULL;
    }

    if (mReadyQueue != NULL) {
        queue_destroy(mReadyQueue);
        mReadyQueue = NULL;
    }

    mAllocated = false;
    return NOERROR;
}

void BufferMgr::unblock()
{
    queue_pop(mFreeQueue);
    queue_pop(mReadyQueue);
}

arxstatus_t BufferMgr::nextFreeIdx(uint32_t *index)
{
    uint32_t idx = 0xFFFFFFFF;
    bool ret = queue_read(mFreeQueue, true_e, &idx);
    if (ret) {
        *index = idx;
    }
    return ret ? NOERROR : FAILED;
}

arxstatus_t BufferMgr::nextReadyIdx(uint32_t *index)
{
    uint32_t idx = 0xFFFFFFFF;
    bool ret = queue_read(mReadyQueue, true_e, &idx);
    if (ret) {
        *index = idx;
    }
    return ret ? NOERROR : FAILED;
}

}
