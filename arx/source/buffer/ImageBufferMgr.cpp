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

#include <buffer/ImageBuffer.h>
#include <buffer/ImageBufferMgr.h>
#include <arx_debug.h>

#if defined(JELLYBEAN) || defined(JELLYBEANMR1)
#include <gui/ISurface.h>
#include <gui/Surface.h>
#else
#include <surfaceflinger/ISurface.h>
#include <surfaceflinger/Surface.h>
#endif

#include <dvp/VisionCam.h>

using namespace android;

namespace tiarx {

ImageBufferMgr::ImageBufferMgr(
        uint32_t bufId, uint32_t w, uint32_t h, uint32_t format,
        DVP_MemType_e memType, uint32_t count, bool lockConfig, bool holdReady) :
        BufferMgr(bufId, count, memType, lockConfig, holdReady)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    mWidth = mBufWidth = w;
    mHeight = mBufHeight = h;
    mFormat = format;
    mAnw = NULL;
    mSurface.clear();
    for (uint32_t i = 0; i < IBufferMgr::MAX_BUFFERS; i++) {
        mBuffers[i] = NULL;
    }
    mHoldReadyForClient = false;
    mUsingTexture = false;
    mCam = NULL;
}

ImageBufferMgr::~ImageBufferMgr()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    mSurface.clear();
}

arxstatus_t ImageBufferMgr::allocateBuffer(uint32_t index)
{
    if (mSurface != NULL && mAnw == NULL) {
        mAnw = anativewindow_create(mSurface);
        if (mAnw == NULL) {
            ARX_PRINT(ARX_ZONE_ERROR, "failed creating native window!\n");
            return NO_MEMORY;
        }
        if (!anativewindow_allocate(mAnw, mBufWidth, mBufHeight, mCount, ANW_NV12_FORMAT, false)) {
            anativewindow_free(mAnw);
            mAnw = NULL;
            ARX_PRINT(ARX_ZONE_ERROR, "failed allocating native window!\n");
            return FAILED;
        }

        if (mWidth < mBufWidth || mHeight < mBufHeight) {
            if (!anativewindow_set_crop(mAnw, 0, 0, mWidth, mHeight)) {
                ARX_PRINT(ARX_ZONE_WARNING, "failed setting native window crop parameters!\n");
            }
        }
    }

    arxstatus_t status;
    sp<ImageBuffer> b;
    if (mAnw != NULL) {
        b = new ImageBuffer(index, mAnw, mWidth, mHeight, this, mUsingTexture, mClient, mFreeQueue, mReadyQueue, mHoldReadyForClient);
    } else {
        b = new ImageBuffer(index, mWidth, mHeight, mBufWidth, mBufHeight, mFormat, mDvp, mMemType, mClient, mFreeQueue, mReadyQueue, mHoldReadyForClient);
    }

    status = b->allocate();
    if (status != NOERROR) {
        ARX_PRINT(ARX_ZONE_ERROR, "failed allocating buffer! (0x%x)\n", status);
        return status;
    }
    ARX_PRINT(ARX_ZONE_ERROR, "mgr:%d allocated buffer:%p idx:%d\n", mBuffId, b.get(), index);
    mBuffers[index] = b;
    return NOERROR;
}

arxstatus_t ImageBufferMgr::free()
{
    ARX_PRINT(ARX_ZONE_API, "ImageBufferMgr::%s\n", __FUNCTION__);
    for (uint32_t i = 0; i < IBufferMgr::MAX_BUFFERS; i++) {
        mBuffers[i].clear();
    }

    if (mAnw != NULL) {
        anativewindow_free(mAnw);
        anativewindow_destroy(&mAnw);
    }

    return BufferMgr::free();
}

arxstatus_t ImageBufferMgr::bindSurface(const android::sp<android::Surface>& surface)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    if (mAllocated) {
        return INVALID_STATE;
    }
    if (surface == NULL) {
        mSurface.clear();
    } else {
        mSurface = surface;
        mEnabled = true;
    }
    return NOERROR;
}

arxstatus_t ImageBufferMgr::bindSurfaceTexture(const android::sp<android::ISurfaceTexture>& texture)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    if (mAllocated) {
        return INVALID_STATE;
    }

    if (texture == NULL) {
        mSurface.clear();
    } else {
        sp<ANativeWindow> window = new SurfaceTextureClient(texture);
        mSurface = window;
        mUsingTexture = true;
        mEnabled = true;
    }
    return NOERROR;
}

arxstatus_t ImageBufferMgr::getImage(uint32_t index, android::sp<ImageBuffer> *image)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    if (index >= mCount) {
        ARX_PRINT(ARX_ZONE_ERROR, "%s: index(%d) is out of bounds(%d)!\n", __FUNCTION__, index, mCount);
        return INVALID_ARGUMENT;
    }

    if (!mAllocated) {
        ARX_PRINT(ARX_ZONE_ERROR, "%s: image buffers have not been allocated yet!\n", __FUNCTION__);
        return INVALID_STATE;
    }

    *image = mBuffers[index];
    return NOERROR;
}

arxstatus_t ImageBufferMgr::render(uint64_t timestamp)
{
    ARX_PRINT(ARX_ZONE_API, "%s - ts:%llu\n", __FUNCTION__, timestamp);
    if (!mAllocated || !mHoldReadyForClient || mSurface == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "%s INVALID STATE!\n", __FUNCTION__);
        return INVALID_STATE;
    }

    for (uint32_t i = 0; i < mCount; i++) {
        const sp<ImageBuffer>& b = mBuffers[i];
        if (b->timestamp() == timestamp) {
           ARX_PRINT(ARX_ZONE_BUFFER, "rendering timestamp:%llu\n", timestamp);
           return render(i);
        } else if (b->timestamp() < timestamp) {
            ARX_PRINT(ARX_ZONE_BUFFER, "dropping buffer with timestamp:%llu\n", b->timestamp());
            return release(i);
        }
    }

    return NOERROR;
}

arxstatus_t ImageBufferMgr::release(uint32_t index, bool render)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    if (!mAllocated) {
        return UNALLOCATED_RESOURCES;
    }

    if (index >= mCount) {
        return INVALID_ARGUMENT;
    }

    const sp<ImageBuffer>& b = mBuffers[index];
    return b->renderAndRelease(render);
}

arxstatus_t ImageBufferMgr::render(uint32_t index)
{
    release(index, true);
    return NOERROR;
}

arxstatus_t ImageBufferMgr::release(uint32_t index)
{
    release(index, false);
    return NOERROR;
}

ImageBuffer *ImageBufferMgr::nextFree()
{
    uint32_t idx;
    bool ret = queue_read(mFreeQueue, true_e, &idx);
    if (ret) {
        const sp<ImageBuffer>& b = mBuffers[idx];
        return b.get();
    }
    return NULL;
}

ImageBuffer *ImageBufferMgr::nextReady()
{
    uint32_t idx = 0;
    bool ret = queue_read(mReadyQueue, true_e, &idx);
    if (ret) {
        const sp<ImageBuffer>& b = mBuffers[idx];
        ARX_PRINT(ARX_ZONE_BUFFER, "mgr: %d, a buffer is ready: %p, idx:%d", mBuffId, b.get(), idx);
        return b.get();
    }
    return NULL;
}

arxstatus_t ImageBufferMgr::readyOnMatch(VisionCamFrame *frame)
{
    arxstatus_t status = INVALID_ARGUMENT;
    for (uint32_t i = 0; i < mCount; i++) {
        const sp<ImageBuffer>& b = mBuffers[i];
        status = b->readyOnMatch(frame);
        if (status == NOERROR) {
            return NOERROR;
        }
    }
    return status;
}

arxstatus_t ImageBufferMgr::freeOnMatch(void *handle)
{
    arxstatus_t status = INVALID_ARGUMENT;
    for (uint32_t i = 0; i < mCount; i++) {
        const sp<ImageBuffer>& b = mBuffers[i];
        status = b->freeOnMatch(handle);
        if (status == NOERROR) {
            return NOERROR;
        }
    }
    return status;
}

arxstatus_t ImageBufferMgr::hold(bool enable)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    if (mAllocated) {
        return INVALID_STATE;
    }
    mHoldReadyForClient = enable;
    return NOERROR;
}

arxstatus_t ImageBufferMgr::setSize(uint32_t width, uint32_t height)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    if (!mLockedConfig) {
        mWidth = mBufWidth = width;
        mHeight = mBufHeight = height;
    } else {
        return NOTCONFIGURABLE;
    }
    return NOERROR;
}

arxstatus_t ImageBufferMgr::setAllocSize(uint32_t width, uint32_t height)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);

    mBufWidth = width;
    mBufHeight = height;

    return NOERROR;
}

arxstatus_t ImageBufferMgr::setFormat(uint32_t format)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    if (!mLockedConfig) {
        mFormat = format;
    } else {
        return NOTCONFIGURABLE;
    }
    return NOERROR;
}

arxstatus_t ImageBufferMgr::setCam(VisionCam *cam)
{
    mCam = cam;
    if (mAllocated && mCam) {
        for (uint32_t i = 0; i < mCount; i++) {
            const sp<ImageBuffer>& b = mBuffers[i];
            ARX_PRINT(ARX_ZONE_BUFFER, "mgr:%d setting camera reference for buffer:%p idx:%d", mBuffId, b.get(), i);
            b->setCam(mCam);
        }
    }
    return NOERROR;
}

arxstatus_t ImageBufferMgr::configure(ImageBufferMgr *mgr)
{
    mWidth = mgr->mWidth;
    mHeight = mgr->mHeight;
    mCount = mgr->mCount;
    return NOERROR;
}

} // namespace
