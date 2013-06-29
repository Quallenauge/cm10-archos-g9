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

#if defined(JELLYBEAN) || defined(JELLYBEANMR1)
#include <gui/SurfaceComposerClient.h>
#else
#include <surfaceflinger/SurfaceComposerClient.h>
#endif

#include <arx/ARXStatus.h>
#include <dvp/dvp_types.h>

namespace tiarx {

struct ARXFaceDetectInfo;
struct ARXFacialPartsInfo;
class ARXImageBuffer;

class DisplaySurface
{
public:
    DisplaySurface(uint32_t w, uint32_t h, uint32_t format = android::PIXEL_FORMAT_RGBX_8888, uint8_t alpha = 255, int z = 100000);
    ~DisplaySurface();
    arxstatus_t init(bool localRender = true);
    android::sp<android::Surface> getSurface();
    void setPosition(uint32_t x, uint32_t y);
    void setWindowSize(uint32_t width, uint32_t height);

    void renderFaces(ARXFaceDetectInfo *pFD);
    void renderFaceInfo(ARXFacialPartsInfo *pFD);
    void renderLuma(ARXImageBuffer *image);
    void renderLuma(DVP_Image_t *image);
    ANativeWindowBuffer *getBuffer(uint8_t **ptr);
    void renderBuffer(ANativeWindowBuffer* buffer);

private:
    void lumatoRGBA(uint8_t *pDst, uint8_t *pSrc, uint32_t dstStrice, uint32_t srcStride);
    void luma16toRGBA(uint8_t *pDst, uint16_t *pSrc, uint32_t dstStrice, uint32_t srcStride);
    void drawFaceBox(uint8_t *pSurf, uint32_t dstStride, uint32_t w, uint32_t h, uint32_t l, uint32_t t);
    void drawFaceBoxes(uint8_t *pSurf, uint32_t dstStride, ARXFaceDetectInfo *pFD);
    void drawFaceParts(uint8_t *pSurf, uint32_t dstStride, ARXFacialPartsInfo *pFD);

    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mFormat;
    uint32_t mBpp;
    int mZ;
    uint8_t mAlpha;

    ANativeWindow *mWindow;
    int mUsageFlags;

    android::Rect mBounds;
    android::sp<android::SurfaceComposerClient> mClient;
    android::sp<android::Surface> mSurface;
    android::sp<android::SurfaceControl> mControl;
};

}
