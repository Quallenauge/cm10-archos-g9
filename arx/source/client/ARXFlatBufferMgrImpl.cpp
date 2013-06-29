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
#include <arx/ARXBufferListener.h>

#include <buffer/FlatBuffer.h>
#include <buffer/IFlatBufferMgr.h>
#include <arx_debug.h>

using namespace android;

namespace tiarx {

ARXFlatBufferMgrImpl::ARXFlatBufferMgrImpl(uint32_t bufID, const sp<IFlatBufferMgr>& mgr, DVP_Handle dvp)
{
    mID = bufID;
    mManager = mgr;
    mListener = NULL;
    mDvp = dvp;
    for (unsigned int i = 0; i < IBufferMgr::MAX_BUFFERS; i++) {
        mBuffers[i].clear();
    }
    mCount = mgr->getCount();
}

ARXFlatBufferMgrImpl::~ARXFlatBufferMgrImpl()
{
    for (unsigned int i = 0; i < IBufferMgr::MAX_BUFFERS; i++) {
        mBuffers[i].clear();
    }
    mManager.clear();
}

void ARXFlatBufferMgrImpl::freeBuffers()
{
    registerClient(NULL);
    for (unsigned int i = 0; i < IBufferMgr::MAX_BUFFERS; i++) {
        mBuffers[i].clear();
    }
}

arxstatus_t ARXFlatBufferMgrImpl::getBuffer(uint32_t index, sp<ARXFlatBufferImpl> *buffer)
{
    if (index >= mCount) {
        ARX_PRINT(ARX_ZONE_ERROR, "invalid index %u!\n", index);
        return INVALID_ARGUMENT;
    }

    arxstatus_t status = NOERROR;
    if (mBuffers[index] == NULL) {
        sp<FlatBuffer> nativeBuf;
        status = mManager->getBuffer(index, &nativeBuf);
        if (status == NOERROR && nativeBuf != NULL) {
            status = nativeBuf->import(mDvp);
            if (status != NOERROR) {
                ARX_PRINT(ARX_ZONE_ERROR, "Failed to import buffer! returning to daemon (0x%x)\n", status);
                mManager->release(index);
            } else {
                mBuffers[index] = new ARXFlatBufferImpl(mID, index, this, nativeBuf);
            }
        } else {
            ARX_PRINT(ARX_ZONE_ERROR, "Failed to obtain buffer from Daemon! (0x%x)\n", status);
        }
    }
    *buffer = mBuffers[index];
    return status;
}

arxstatus_t ARXFlatBufferMgrImpl::bindGraph(DVP_KernelGraph_t *pGraph)
{
    return mManager->bindGraph(pGraph);
}

arxstatus_t ARXFlatBufferMgrImpl::setCount(uint32_t count)
{
    arxstatus_t status = mManager->setCount(count);
    if (status == NOERROR) {
        mCount = count;
    }
    return status;
}

uint32_t ARXFlatBufferMgrImpl::getCount()
{
    return mCount;
}

arxstatus_t ARXFlatBufferMgrImpl::release(uint32_t index)
{
    return mManager->release(index);
}

arxstatus_t ARXFlatBufferMgrImpl::registerClient(ARXFlatBufferListener *listener)
{
    arxstatus_t status = mManager->registerClient(listener == NULL ? NULL : this);
    this->mListener = listener;
    return status;
}

void ARXFlatBufferMgrImpl::onBufferChanged(uint32_t index, uint64_t timestamp)
{
    ARX_PRINT(ARX_ZONE_BUFFER, "onBufferChanged index %u changed at "FMT_UINT64_T"!\n", index, timestamp);
    if (mListener != NULL) {
        sp<ARXFlatBufferImpl> buf;
        arxstatus_t status = getBuffer(index, &buf);
        if (status == NOERROR) {
            buf->setTimestamp(timestamp);
            buf->incStrong(this);
            mListener->onBufferChanged(buf.get());
        } else {
            ARX_PRINT(ARX_ZONE_WARNING, "WARNING, onBufferChanged error obtaining buffer(0x%x)\n", status);

        }
    }
}

arxstatus_t ARXFlatBufferMgrImpl::setSize(uint32_t size)
{
    return mManager->setSize(size);
}

}
