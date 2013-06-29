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

#include <DisplaySurface.h>
#include <ui/GraphicBufferMapper.h>

#include <arx/ARXImageBuffer.h>
#include <arx/ARXBufferTypes.h>

#include <arx_debug.h>

using namespace android;
using namespace tiarx;

namespace tiarx {

DisplaySurface::DisplaySurface(uint32_t width, uint32_t height, uint32_t format, uint8_t alpha, int z)
{
    mWidth = width;
    mHeight = height;
    mFormat = format;
    mBpp = 4;
    mAlpha = alpha;
    mZ = z;
    mUsageFlags = GRALLOC_USAGE_SW_WRITE_OFTEN;

    mBounds.top = 0;
    mBounds.left = 0;
    mBounds.right = mWidth;
    mBounds.bottom = mHeight;
}

DisplaySurface::~DisplaySurface()
{
    mSurface.clear();
    mControl.clear();
    mClient.clear();
}

arxstatus_t DisplaySurface::init(bool localRender)
{
    mClient = new SurfaceComposerClient();
    if (mClient == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to create Surface Composer client!\n");
        return NOMEMORY;
    }

#if defined(JELLYBEANMR1)
    mControl = mClient->createSurface(String8("ARXSurface"), mWidth, mHeight, mFormat);
#else
    mControl = mClient->createSurface(0, mWidth, mHeight, mFormat);
#endif
    if (mControl == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to create surface!\n");
        return NOMEMORY;
    }

    SurfaceComposerClient::openGlobalTransaction();
    mControl->setLayer(mZ);
    SurfaceComposerClient::closeGlobalTransaction();

    mSurface = mControl->getSurface();
    if (mSurface == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to obtain surface!");
        return NOMEMORY;
    }

    mWindow = mSurface.get();

    if (localRender) {
        if (native_window_set_buffers_dimensions(mWindow, mWidth, mHeight)) {
            ARX_PRINT(ARX_ZONE_ERROR, "Failed to set window buffer dimensions!");
            return FAILED;
        }
    }

    return NOERROR;
}

sp<Surface> DisplaySurface::getSurface() {
    return mSurface;
}

void DisplaySurface::setPosition(uint32_t x, uint32_t y) {
    SurfaceComposerClient::openGlobalTransaction();
    mControl->setPosition(x, y);
    SurfaceComposerClient::closeGlobalTransaction();
}

void DisplaySurface::setWindowSize(uint32_t width, uint32_t height) {
    native_window_set_scaling_mode(mWindow, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    SurfaceComposerClient::openGlobalTransaction();
    mControl->setSize(width, height);
    SurfaceComposerClient::closeGlobalTransaction();
}

ANativeWindowBuffer *DisplaySurface::getBuffer(uint8_t **ptr)
{
    uint8_t *ptrs[3] = {NULL, NULL, NULL};
    ANativeWindowBuffer* buffer;

#if defined(JELLYBEANMR1)
    if (native_window_dequeue_buffer_and_wait(mWindow, &buffer) < 0 ) {
#else
    if (mWindow->dequeueBuffer(mWindow, &buffer) < 0 ) {
#endif
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to dequeue buffer\n");
        return NULL;
    }

    GraphicBufferMapper &mapper = GraphicBufferMapper::get();
    if (mapper.lock(buffer->handle, mUsageFlags, mBounds, (void **)ptrs) < 0) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to get surface pointer\n");
#if defined(JELLYBEANMR1)
        mWindow->cancelBuffer(mWindow, buffer, -1);
#else
        mWindow->cancelBuffer(mWindow, buffer);
#endif
        return NULL;
    }

    ARX_PRINT(ARX_ZONE_TEST, "DisplaySurface: Obtained buffer p:%p w:%d, h:%d\n", ptrs[0], buffer->width, buffer->height);

    *ptr = ptrs[0];
    return buffer;

}

void DisplaySurface::renderBuffer(ANativeWindowBuffer* buffer)
{
    GraphicBufferMapper &mapper = GraphicBufferMapper::get();
    if (mapper.unlock(buffer->handle)< 0) {
        ARX_PRINT(ARX_ZONE_WARNING, "Failed to unlock buffer from mapper\n");
    }
#if defined(JELLYBEANMR1)
    if (mWindow->queueBuffer(mWindow, buffer, -1) < 0) {
#else
    if (mWindow->queueBuffer(mWindow, buffer) < 0) {
#endif
        ARX_PRINT(ARX_ZONE_WARNING, "Failed to queue buffer\n");
    }
}

void DisplaySurface::renderLuma(ARXImageBuffer *image)
{
    ARX_PRINT(ARX_ZONE_TEST, "renderLuma %p\n", image);
    uint8_t *ptr = NULL;
    ANativeWindowBuffer* buffer;
    buffer = getBuffer(&ptr);
    if (buffer == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Error obtaining AnativeWindowBuffer to render luma\n");
        return;
    }
    uint32_t format = image->format();
    if (format == FOURCC_Y16) {
        luma16toRGBA(ptr, (uint16_t *)image->data(0), buffer->stride*mBpp, image->stride());
    } else if (format == FOURCC_Y800 || format == FOURCC_NV12) {
        lumatoRGBA(ptr, (uint8_t *)image->data(0), buffer->stride*mBpp, image->stride());
    }

    renderBuffer(buffer);
}

void DisplaySurface::renderLuma(DVP_Image_t *image)
{
    ARX_PRINT(ARX_ZONE_TEST, "renderLuma %p\n", image);
    uint8_t *ptr = NULL;
    ANativeWindowBuffer* buffer;
    buffer = getBuffer(&ptr);
    if (buffer == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Error obtaining AnativeWindowBuffer to render luma\n");
        return;
    }
    uint32_t format = image->color;
    if (format == FOURCC_Y16) {
        luma16toRGBA(ptr, (uint16_t *)image->pData[0], buffer->stride*mBpp, image->y_stride);
    } else if (format == FOURCC_Y800 || format == FOURCC_NV12) {
        lumatoRGBA(ptr, (uint8_t *)image->pData[0], buffer->stride*mBpp, image->y_stride);
    }

    renderBuffer(buffer);
}

void DisplaySurface::renderFaces(ARXFaceDetectInfo *pFD)
{
    uint8_t *ptr = NULL;
    ANativeWindowBuffer* buffer;
    buffer = getBuffer(&ptr);
    if (buffer == NULL) {
        return;
    }
    drawFaceBoxes(ptr, buffer->stride*mBpp, pFD);
    renderBuffer(buffer);
}

void DisplaySurface::renderFaceInfo(ARXFacialPartsInfo *pFP)
{
    uint8_t *ptr = NULL;
    ANativeWindowBuffer* buffer;
    buffer = getBuffer(&ptr);
    if (buffer == NULL) {
        return;
    }
    drawFaceParts(ptr, buffer->stride*mBpp, pFP);
    renderBuffer(buffer);
}

void DisplaySurface::lumatoRGBA(uint8_t *pDst, uint8_t *pSrc, uint32_t dstStride, uint32_t srcStride) {
    ARX_PRINT(ARX_ZONE_TEST, "lumatoRGBA %p=>%p srcStride:%u, dstStride:%u w:%u, h:%u",pSrc, pDst, srcStride, dstStride, mWidth, mHeight);
    for (uint32_t i = 0; i < mHeight; i++) {
        for (uint32_t j = 0, k = 0; j < mWidth; j++, k += mBpp) {
            pDst[k]= pSrc[j];
            pDst[k+1]= pSrc[j];
            pDst[k+2]= pSrc[j];
            pDst[k+3]= mAlpha;
        }
        pDst += dstStride;
        pSrc += srcStride;
    }
}

void DisplaySurface::luma16toRGBA(uint8_t *pDst, uint16_t *pSrc, uint32_t dstStride, uint32_t srcStride) {
    ARX_PRINT(ARX_ZONE_TEST, "lumatoRGBA %p=>%p srcStride:%u, dstStride:%u w:%u, h:%u",pSrc, pDst, srcStride, dstStride, mWidth, mHeight);
    for (uint32_t i = 0; i < mHeight; i++) {
        for (uint32_t j = 0, k = 0; j < mWidth; j++, k += mBpp) {
            pDst[k]= (pSrc[j]*255)/65535;
            pDst[k+1]= (pSrc[j]*255)/65535;
            pDst[k+2]= (pSrc[j]*255)/65535;
            pDst[k+3]= mAlpha;
        }
        pDst += dstStride;
        pSrc += srcStride/2;
    }
}

void DisplaySurface::drawFaceBoxes(uint8_t *pSurf, uint32_t dstStride, ARXFaceDetectInfo *pFD) {
    uint8_t *pDst = pSurf;
    //Clear surface
    for (uint32_t i = 0; i < mHeight; i++) {
        memset(pDst, 0, mWidth*mBpp);
        pDst += dstStride;
    }

    for (uint32_t i = 0; i < pFD->numFaces; i++) {
        ARXFace *face = &pFD->faces[i];
        uint32_t w = face->w;
        uint32_t h = face->h;
        uint32_t l = face->left;
        uint32_t t = face->top;
        drawFaceBox(pSurf, dstStride, w, h, l, t);
    }
}

void DisplaySurface::drawFaceParts(uint8_t *pSurf, uint32_t dstStride, ARXFacialPartsInfo *pFP) {
    uint8_t *pDst = pSurf;
    //Clear surface
    for (uint32_t i = 0; i < mHeight; i++) {
        memset(pDst, 0, mWidth*mBpp);
        pDst += dstStride;
    }

    for (uint32_t i = 0; i < pFP->numFaces; i++) {
        ARXFacialParts *face = &pFP->faces[i];
        uint32_t w = 4;
        uint32_t h = 4;
        uint32_t scale = 2;
        drawFaceBox(pSurf, dstStride, w, h, scale*face->lefteyein.x, scale*face->lefteyein.y);
        drawFaceBox(pSurf, dstStride, w, h, scale*face->righteyein.x, scale*face->righteyein.y);
        drawFaceBox(pSurf, dstStride, w, h, scale*face->lefteyeout.x, scale*face->lefteyeout.y);
        drawFaceBox(pSurf, dstStride, w, h, scale*face->righteyeout.x, scale*face->righteyeout.y);
        drawFaceBox(pSurf, dstStride, w, h, scale*face->leftnose.x, scale*face->leftnose.y);
        drawFaceBox(pSurf, dstStride, w, h, scale*face->rightnose.x, scale*face->rightnose.y);
        drawFaceBox(pSurf, dstStride, w, h, scale*face->leftmouth.x, scale*face->leftmouth.y);
        drawFaceBox(pSurf, dstStride, w, h, scale*face->rightmouth.x, scale*face->rightmouth.y);
        drawFaceBox(pSurf, dstStride, w, h, scale*face->centertopmouth.x, scale*face->centertopmouth.y);
    }
}

void DisplaySurface::drawFaceBox(uint8_t *pSurf, uint32_t dstStride, uint32_t w, uint32_t h, uint32_t l, uint32_t t)
{
    if (l + w >= mWidth) {
        return;
    }

    if (t + h >= mHeight) {
        return;
    }

    uint8_t *pDst = pSurf;
    pDst = pSurf;
    pDst += t*dstStride;
    pDst += l*mBpp;
    for (uint32_t i = 0; i < h; i++) {
        for (uint32_t j = 0, k = 0; j < w; j++, k += mBpp) {
            pDst[k]= 0;
            pDst[k+1]= 255;
            pDst[k+2]= 0;
            pDst[k+3]= mAlpha;
        }
        pDst += dstStride;
    }
}

}
