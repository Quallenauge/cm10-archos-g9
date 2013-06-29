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

#include <buffer/Buffer.h>
#include <buffer/IBufferMgrClient.h>
#include <arx_debug.h>

using namespace android;

namespace tiarx {

Buffer::Buffer(uint32_t index, DVP_Handle dvp, const sp<IBufferMgrClient>& client, queue_t *freeQ, queue_t *readyQ, bool holdForClient)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    mRemote = false;
    mConsumers = 0;
    mIndex = index;
    mTimestamp = 0xFFFFFFFFFFFFFFFFULL;
    mFreeQ = freeQ;
    mReadyQ = readyQ;
    mDvp = dvp;
    mClient = client;
    mHoldReady = holdForClient;
}

Buffer::~Buffer()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    mClient.clear();
}
//inline void obtain() { android_atomic_inc(&mUsers); };
//inline int32_t release() { return android_atomic_dec(&mUsers); }
arxstatus_t Buffer::consume()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    android_atomic_inc(&mConsumers);
    return NOERROR;
}

arxstatus_t Buffer::release()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    if (android_atomic_dec(&mConsumers) == 1) {
        ARX_PRINT(ARX_ZONE_BUFFER, "buff:%p: idx:%d released!\n", this, mIndex);
        if (internalRelease()) {
            ARX_PRINT(ARX_ZONE_BUFFER, "buff:%p: idx:%d back into free queue!\n", this, mIndex);
            bool ret = queue_write(mFreeQ, false_e, &mIndex);
            if (!ret) {
                ARX_PRINT(ARX_ZONE_ERROR, "buff:%p: idx:%d error writing into free queue!\n", this, mIndex);
                return FAILED;
            }
        }
    } else {
        ARX_PRINT(ARX_ZONE_BUFFER, "buff:%p: idx:%d still being consumed!\n", this, mIndex);
    }
    return NOERROR;
}

arxstatus_t Buffer::ready()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);

    if (mReadyQ != NULL) {
        consume();
    }

    if (mClient != NULL || mHoldReady) {
        consume();
    }

    if (mReadyQ == NULL && mClient == NULL) {
        //there are no consumers of this buffer, release it
        ARX_PRINT(ARX_ZONE_BUFFER, "buff:%p: idx:%d ready but releasing since there are no consumers!\n", this, mIndex);
        consume();
        return release();
    }

    ARX_PRINT(ARX_ZONE_BUFFER, "buff:%p: ready idx:%d\n", this, mIndex);
    bool ret = true;
    if (mReadyQ != NULL) {
        ret = queue_write(mReadyQ, false_e, &mIndex);
    }

    if (mClient != NULL) {
        mClient->onBufferChanged(mIndex, mTimestamp);
    }
    return ret ? NOERROR : FAILED;
}

}
