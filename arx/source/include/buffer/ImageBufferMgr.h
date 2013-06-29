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

#ifndef _IMAGEBUFFERMGR_H_
#define _IMAGEBUFFERMGR_H_

#include <buffer/BnImageBufferMgr.h>
#include <buffer/BufferMgr.h>
#include <dvp/anativewindow.h>

class VisionCam;
class VisionCamFrame;

namespace tiarx {

class ImageBufferMgr :
    public BnImageBufferMgr,
    public BufferMgr
{
public:
    ImageBufferMgr(uint32_t bufId, uint32_t w, uint32_t h, uint32_t format,
                     DVP_MemType_e memType = DVP_MTYPE_DEFAULT,
                     uint32_t count = DEFAULT_COUNT,
                     bool lockConfig = true,
                     bool holdReady = false);
    virtual ~ImageBufferMgr();

    arxstatus_t free();

    ImageBuffer *nextFree();
    ImageBuffer *nextReady();
    arxstatus_t readyOnMatch(VisionCamFrame *frame);
    arxstatus_t freeOnMatch(void *handle);

    inline uint32_t width() const { return mWidth; }
    inline uint32_t height() const { return mHeight; }
    inline uint32_t format() const { return mFormat; }

    virtual arxstatus_t configure(ImageBufferMgr *mgr);
    virtual arxstatus_t setCam(VisionCam *cam);
    virtual arxstatus_t setAllocSize(uint32_t width, uint32_t height);

    //From IImageBufferMgr
    virtual arxstatus_t bindSurface(const android::sp<android::Surface>& surface);
    virtual arxstatus_t bindSurfaceTexture(const android::sp<android::ISurfaceTexture>& texture);
    virtual arxstatus_t getImage(uint32_t index, android::sp<ImageBuffer> *buffer);
    virtual arxstatus_t render(uint64_t timestamp);
    virtual arxstatus_t render(uint32_t index);
    virtual arxstatus_t hold(bool enable);
    virtual arxstatus_t setSize(uint32_t width, uint32_t height);
    virtual arxstatus_t setFormat(uint32_t format);

    //From IBufferMgr
    virtual arxstatus_t release(uint32_t index);

private:
    arxstatus_t allocateBuffer(uint32_t index);
    virtual arxstatus_t release(uint32_t index, bool render);

    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mBufWidth;
    uint32_t mBufHeight;
    uint32_t mFormat;
    android::sp<ANativeWindow> mSurface;
    anativewindow_t *mAnw;
    sp<ImageBuffer> mBuffers[IBufferMgr::MAX_BUFFERS];

    VisionCam *mCam;
    bool mHoldReadyForClient;
    bool mUsingTexture;
};

}

#endif //_IMAGEBUFFERMGR_H_
