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

#ifndef _ARX_BUFFER_H
#define _ARX_BUFFER_H

#include <arx/ARXStatus.h>

#include <dvp/dvp_types.h>
#include <dvp/anativewindow.h>

#include <utils/RefBase.h>

class VisionCamFrame;

namespace tiarx {

class IBufferMgrClient;

/*!
 * \brief A Buffer class
 * The buffer class assumes a single producer and multiple consumers
 */
class Buffer : public android::LightRefBase<Buffer>
{
public:
    Buffer(uint32_t index, DVP_Handle dvp, const android::sp<IBufferMgrClient>& client,
           queue_t *freeQ, queue_t *readyQ, bool holdForClient);
    virtual ~Buffer();

    inline void setTimestamp(uint64_t timestamp) { mTimestamp = timestamp; }
    inline uint64_t timestamp() const { return mTimestamp; }
    inline uint32_t index() const { return mIndex; }

    /** Signals a consumer will consume the buffer */
    arxstatus_t consume();
    /** Signal that consumer is done with the buffer */
    arxstatus_t release();
    /** Signals the buffer is ready (to be done by producer)*/
    arxstatus_t ready();

    virtual arxstatus_t allocate() = 0;
    virtual arxstatus_t free() = 0;

    virtual arxstatus_t import(DVP_Handle dvp) = 0;

protected:
    virtual bool internalRelease() = 0;

    bool mRemote;
    bool mReady;
    volatile int32_t mConsumers;
    queue_t *mFreeQ;
    queue_t *mReadyQ;
    uint32_t mIndex;
    uint64_t mTimestamp;
    DVP_Handle mDvp;

    android::sp<IBufferMgrClient> mClient;
    bool mHoldReady;
};

}
#endif //_ARX_BUFFER_H
