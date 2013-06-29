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

#ifndef _ARX_IMAGEBUFFER_H
#define _ARX_IMAGEBUFFER_H

#include <buffer/Buffer.h>
#include <arx/ARXStatus.h>

#include <dvp/dvp_types.h>
#include <dvp/anativewindow.h>

class VisionCamFrame;
class VisionCam;

namespace tiarx {

class ImageBufferMgr;

class ImageBuffer : public Buffer
{
public:
    ImageBuffer(uint32_t index, anativewindow_t *anw, uint32_t width, uint32_t height, ImageBufferMgr *mgr, bool usingTexture, android::sp<IBufferMgrClient>& client,
                queue_t *freeQ, queue_t *readyQ, bool holdForClient);
    ImageBuffer(uint32_t index, uint32_t width, uint32_t height, uint32_t bufWidth, uint32_t bufHeight, uint32_t format,
                DVP_Handle dvp, DVP_MemType_e type, android::sp<IBufferMgrClient>& client,
                queue_t *freeQ, queue_t *readyQ, bool holdForClient);
    virtual ~ImageBuffer();

    arxstatus_t readyOnMatch(VisionCamFrame *camFrame);
    arxstatus_t freeOnMatch(void *handle);
    arxstatus_t copyInfo(DVP_Image_t *image);
    arxstatus_t copyInfo(DVP_Image_t *image, uint32_t forcedFormat);
    inline VisionCamFrame *getCamFrame() const { return mCamFrame; }
    inline void setCam(VisionCam *cam) { mCam = cam; }

    arxstatus_t renderAndRelease(bool render);
    arxstatus_t import(DVP_Handle mDvp);
    arxstatus_t allocate();
    arxstatus_t free();

    static arxstatus_t writeToParcel(const sp<ImageBuffer>& image, Parcel* parcel);
    static sp<ImageBuffer> readFromParcel(const Parcel& data);

private:
    ImageBuffer();
    bool internalRelease();
    arxstatus_t display(bool render);

    void init();

    DVP_Image_t mImage;
    DVP_S32 mSharedFds[DVP_MAX_PLANES];
    DVP_VALUE mImportHdls[DVP_MAX_PLANES];

    anativewindow_t *mAnw;
    VisionCamFrame *mCamFrame;
    VisionCam *mCam;
    bool mPendingRender;
    bool mHoldForClient;
    bool mUsingTexture;
    ImageBufferMgr *mMgr;
};

}
#endif //_ARX_IMAGEBUFFER_H
