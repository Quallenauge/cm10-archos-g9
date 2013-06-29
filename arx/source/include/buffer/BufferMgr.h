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

#ifndef _BUFFERMGR_H_
#define _BUFFERMGR_H_

#include <buffer/IBufferMgr.h>

#include <dvp/dvp_types.h>
#include <sosal/queue.h>

#include <utils/RefBase.h>

namespace tiarx {

class IBufferMgrClient;
class FlatBuffer;

class BufferMgr : public virtual IBufferMgr
{
public:
    static const uint32_t DEFAULT_COUNT = 4;
    BufferMgr(uint32_t bufId, uint32_t count, DVP_MemType_e memType, bool lockConfig, bool holdReady);
    virtual ~BufferMgr();

    inline uint32_t id() const { return mBuffId; }
    inline bool enabled() const { return mEnabled; }
    inline void setEnable(bool flag) { mEnabled = flag; }

    virtual arxstatus_t allocate(DVP_Handle dvp);
    virtual arxstatus_t free();

    virtual void unblock();

    //From IBufferMgr
    virtual arxstatus_t bindGraph(DVP_KernelGraph_t *pGraph);
    virtual arxstatus_t setCount(uint32_t count);
    virtual uint32_t    getCount() const;

    //virtual arxstatus_t release(uint32_t index);
    virtual arxstatus_t registerClient(IBufferMgrClient *client);

protected:
    virtual arxstatus_t allocateBuffer(uint32_t index) = 0;
    virtual arxstatus_t nextFreeIdx(uint32_t *index);
    virtual arxstatus_t nextReadyIdx(uint32_t *index);

    uint32_t mBuffId;
    bool mEnabled;
    bool mAllocated;
    bool mLockedConfig;
    bool mHoldReady;

    queue_t *mFreeQueue;
    queue_t *mReadyQueue;
    uint32_t mCount;

    DVP_KernelGraph_t *mGraph;
    android::sp<IBufferMgrClient> mClient;
    DVP_Handle mDvp;
    DVP_MemType_e mMemType;
};

}

#endif //_BUFFERMGR_H_
