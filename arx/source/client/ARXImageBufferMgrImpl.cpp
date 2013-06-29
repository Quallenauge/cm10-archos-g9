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

#include <ARXImageBufferImpl.h>
#include <ARXImageBufferMgrImpl.h>
#include <arx/ARXBufferListener.h>

#include <buffer/ImageBuffer.h>
#include <buffer/IImageBufferMgr.h>
#include <arx_debug.h>

using namespace android;

namespace tiarx {

ARXImageBufferMgrImpl::ARXImageBufferMgrImpl(uint32_t bufID, const sp<IImageBufferMgr>& mgr, DVP_Handle dvp)
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

ARXImageBufferMgrImpl::~ARXImageBufferMgrImpl()
{
    freeBuffers();
    mManager.clear();
}

void ARXImageBufferMgrImpl::freeBuffers()
{
    registerClient(NULL);
    for (unsigned int i = 0; i < IBufferMgr::MAX_BUFFERS; i++) {
        mBuffers[i].clear();
    }
}

arxstatus_t ARXImageBufferMgrImpl::getBuffer(uint32_t index, sp<ARXImageBufferImpl> *buffer)
{
    if (index >= mCount) {
        ARX_PRINT(ARX_ZONE_ERROR, "invalid index %u!\n", index);
        return INVALID_ARGUMENT;
    }

    arxstatus_t status = NOERROR;
    if (mBuffers[index] == NULL) {
        sp<ImageBuffer> nativeBuf;
        status = mManager->getImage(index, &nativeBuf);
        if (status == NOERROR && nativeBuf != NULL) {
            status = nativeBuf->import(mDvp);
            if (status != NOERROR) {
                ARX_PRINT(ARX_ZONE_ERROR, "Failed to import buffer! returning to daemon (0x%x)\n", status);
                mManager->release(index);
            } else {
                mBuffers[index] = new ARXImageBufferImpl(mID, index, this, nativeBuf);
            }
        } else {
            ARX_PRINT(ARX_ZONE_ERROR, "Failed to obtain buffer from Daemon! (0x%x)\n", status);
        }
    }
    *buffer = mBuffers[index];
    return status;
}

arxstatus_t ARXImageBufferMgrImpl::bindGraph(DVP_KernelGraph_t *pGraph)
{
    return mManager->bindGraph(pGraph);
}

arxstatus_t ARXImageBufferMgrImpl::setCount(uint32_t count)
{
    arxstatus_t status = mManager->setCount(count);
    if (status == NOERROR) {
        mCount = count;
    }
    return status;
}

uint32_t ARXImageBufferMgrImpl::getCount()
{
    return mCount;
}

arxstatus_t ARXImageBufferMgrImpl::release(uint32_t index)
{
    return mManager->release(index);
}

void ARXImageBufferMgrImpl::onBufferChanged(uint32_t index, uint64_t timestamp)
{
    ARX_PRINT(ARX_ZONE_BUFFER, "onBufferChanged index %u changed at "FMT_UINT64_T"!\n", index, timestamp);
    if (mListener != NULL) {
        sp<ARXImageBufferImpl> buf;
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

//BufferMgr interface just acts as a proxy to the Daemon provided IBufferMgr interface
arxstatus_t ARXImageBufferMgrImpl::bindSurface(android::Surface *surface)
{
    sp<Surface> s = surface;
    return mManager->bindSurface(s);
}

arxstatus_t ARXImageBufferMgrImpl::bindSurface(android::ISurfaceTexture *surfaceTexture)
{
    sp<ISurfaceTexture> s = surfaceTexture;
    return mManager->bindSurfaceTexture(s);
}

arxstatus_t ARXImageBufferMgrImpl::render(uint64_t timestamp)
{
    return mManager->render(timestamp);
}

arxstatus_t ARXImageBufferMgrImpl::render(uint32_t index)
{
    return mManager->render(index);
}

arxstatus_t ARXImageBufferMgrImpl::hold(bool enable)
{
    return mManager->hold(enable);
}

arxstatus_t ARXImageBufferMgrImpl::registerClient(ARXImageBufferListener *listener)
{
    arxstatus_t status = mManager->registerClient(listener == NULL ? NULL : this);
    if (status == NOERROR) {
        mListener = listener;
    }
    return status;
}

arxstatus_t ARXImageBufferMgrImpl::setSize(uint32_t width, uint32_t height)
{
    return mManager->setSize(width, height);
}

arxstatus_t ARXImageBufferMgrImpl::setFormat(uint32_t format)
{
    return mManager->setFormat(format);
}

}
