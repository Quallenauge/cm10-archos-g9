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

#ifndef _ARXFLATBUFFERMGRIMPL_H_
#define _ARXFLATBUFFERMGRIMPL_H_

#include <arx/ARXFlatBufferMgr.h>
#include <buffer/IFlatBufferMgr.h>
#include <buffer/IBufferMgrClient.h>

namespace tiarx {

class ARXFlatBufferImpl;

class ARXFlatBufferMgrImpl :
    public ARXFlatBufferMgr,
    public BnBufferMgrClient
{
public:
    ARXFlatBufferMgrImpl(uint32_t bufID, const android::sp<IFlatBufferMgr>& mgr, DVP_Handle dvp);
    virtual ~ARXFlatBufferMgrImpl();

    //ARXFlatBufferMgr interface
    virtual arxstatus_t registerClient(ARXFlatBufferListener *listener);
    virtual arxstatus_t setSize(uint32_t size);

    //ARXBufferMgr interface
    arxstatus_t bindGraph(DVP_KernelGraph_t *pGraph);
    arxstatus_t setCount(uint32_t count);
    uint32_t getCount();

    //IBufferMgrClient interface
    void onBufferChanged(uint32_t index, uint64_t timestamp);

    //Methods from this class
    arxstatus_t release(uint32_t index);

    void freeBuffers();
private:
    //Methods specific to this class
    arxstatus_t getBuffer(uint32_t index, android::sp<ARXFlatBufferImpl> *buf);

    uint32_t mCount;
    uint32_t mID;
    android::sp<IFlatBufferMgr> mManager;
    android::sp<ARXFlatBufferImpl> mBuffers[IBufferMgr::MAX_BUFFERS];
    ARXFlatBufferListener *mListener;

    DVP_Handle mDvp;
};

}
#endif //_ARXFLATBUFFERMGRIMPL_H_
