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

#ifndef _ARXIMAGEBUFFERMGRIMPL_H_
#define _ARXIMAGEBUFFERMGRIMPL_H_

#include <arx/ARXImageBufferMgr.h>
#include <buffer/IImageBufferMgr.h>
#include <buffer/IBufferMgrClient.h>

namespace tiarx {

class ARXImageBufferImpl;
class ARXImageBufferListener;

class ARXImageBufferMgrImpl :
    public ARXImageBufferMgr,
    public BnBufferMgrClient
{
public:
    ARXImageBufferMgrImpl(uint32_t bufID, const android::sp<IImageBufferMgr>& mgr, DVP_Handle dvp);
    virtual ~ARXImageBufferMgrImpl();

    //ARXImageBufferMgr interface
    arxstatus_t bindSurface(android::Surface *surface);
    arxstatus_t bindSurface(android::ISurfaceTexture *surfaceTexture);
    arxstatus_t render(uint64_t timestamp);
    arxstatus_t render(uint32_t index);
    arxstatus_t hold(bool enable);
    arxstatus_t registerClient(ARXImageBufferListener *listener);
    arxstatus_t setSize(uint32_t width, uint32_t height);
    arxstatus_t setFormat(uint32_t format);

    //ARXBufferMgr interface
    arxstatus_t bindGraph(DVP_KernelGraph_t *pGraph);
    arxstatus_t setCount(uint32_t count);
    uint32_t getCount();

    //IBufferMgrClient interface
    void onBufferChanged(uint32_t index, uint64_t timestamp);

    //Methods from this class
    arxstatus_t post(uint32_t index);
    arxstatus_t release(uint32_t index);

    void freeBuffers();
private:
    //Methods specific to this class
    arxstatus_t getBuffer(uint32_t index, android::sp<ARXImageBufferImpl> *buf);

    uint32_t mCount;
    uint32_t mID;
    android::sp<IImageBufferMgr> mManager;
    android::sp<ARXImageBufferImpl> mBuffers[IBufferMgr::MAX_BUFFERS];
    ARXImageBufferListener *mListener;

    DVP_Handle mDvp;
};

}
#endif //_ARXIMAGEBUFFERMGRIMPL_H_
