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
#include <buffer/IBufferMgrClient.h>
#include <dvp/dvp_types.h>
#include <dvp/dvp_mem.h>
#include <dvp/dvp_api.h>
#include <dvp/VisionCam.h>

#include <arx_debug.h>

using namespace android;

namespace tiarx {

ImageBuffer::ImageBuffer(uint32_t index, anativewindow_t *anw, uint32_t width, uint32_t height,
        ImageBufferMgr *mgr, bool usingTexture,
        sp<IBufferMgrClient>& client, queue_t *freeQ, queue_t *readyQ, bool holdForClient) :
        Buffer(index, 0, client, freeQ, readyQ, holdForClient)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    mAnw = anw;
    DVP_Image_Init(&mImage, anw->m_width, anw->m_height, FOURCC_NV12);
    mImage.width = width;
    mImage.height = height;
    mImage.memType = DVP_MTYPE_DISPLAY_2DTILED;
    mUsingTexture = usingTexture;
    mMgr = mgr;
    init();
}

ImageBuffer::ImageBuffer(
        uint32_t index, uint32_t width, uint32_t height,
        uint32_t bufWidth, uint32_t bufHeight, uint32_t format,
        DVP_Handle dvp, DVP_MemType_e type, android::sp<IBufferMgrClient>& client,
        queue_t *freeQ, queue_t *readyQ, bool holdForClient) :
        Buffer(index, dvp, client, freeQ, readyQ, holdForClient)

{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    mAnw = NULL;
    mUsingTexture = false;
    DVP_Image_Init(&mImage, width, height, format);
    mImage.bufWidth = bufWidth;
    mImage.bufHeight = bufHeight;
    mImage.memType = type;
    init();
}

ImageBuffer::ImageBuffer() : Buffer(0, 0, NULL, NULL, NULL, false)
{
    mUsingTexture = false;
    DVP_Image_Init(&mImage, 0, 0, FOURCC_Y800);
    init();
}

ImageBuffer::~ImageBuffer()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    free();
}

void ImageBuffer::init()
{
    for (uint32_t i = 0; i < DVP_MAX_PLANES; i++) {
        mSharedFds[i] = 0;
        mImportHdls[i] = 0;
    }
    mCam = NULL;
    mCamFrame = NULL;
    mPendingRender = false;
}

arxstatus_t ImageBuffer::readyOnMatch(VisionCamFrame *camFrame)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    DVP_Image_t *pImage = (DVP_Image_t *)camFrame->mFrameBuff;
    if (pImage->pBuffer[0] == mImage.pBuffer[0]) {
        mCamFrame = camFrame;
        mTimestamp = camFrame->mTimestamp;
        ready();
        return NOERROR;
    }
    return INVALID_ARGUMENT;
}

arxstatus_t ImageBuffer::freeOnMatch(void *handle)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    if (handle && handle == mImage.reserved) {
        if (mCam) {
            ARX_PRINT(ARX_ZONE_BUFFER, "imbuffer:%p, index:%d returning to camera mCamFrame:%p handle:%p\n", this, mIndex, mCamFrame, handle);
            if (mCamFrame) {
                mCam->returnFrame(mCamFrame);
            } else {
                ARX_PRINT(ARX_ZONE_WARNING, "freeOnMatch: no camera frame associated with this(%p) buffer!\n",this);
            }
        } else {
            bool ret = queue_write(mFreeQ, false_e, &mIndex);
            if (!ret) {
                ARX_PRINT(ARX_ZONE_BUFFER, "imbuff:%p: idx:%d error writing into free queue!\n", this, mIndex);
                return FAILED;
            }
        }
        return NOERROR;
    }
    return INVALID_ARGUMENT;
}

arxstatus_t ImageBuffer::copyInfo(DVP_Image_t *image)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    *image = mImage;
    return NOERROR;
}

arxstatus_t ImageBuffer::copyInfo(DVP_Image_t *pImage, uint32_t format)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    DVP_Image_Dup(pImage, &mImage);
    pImage->color = format;
    if (format == FOURCC_Y800) {
        pImage->planes = 1;
    }
    return NOERROR;
}

arxstatus_t ImageBuffer::renderAndRelease(bool render)
{
    mPendingRender = render;
    return Buffer::release();
}

arxstatus_t ImageBuffer::display(bool render)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    if (mAnw != NULL) {
        bool ret = false;
        if (render) {
            ret = anativewindow_enqueue(mAnw, mImage.reserved);
        } else {
            ret = anativewindow_drop(mAnw, mImage.reserved);
        }
        void *handle = NULL;
        ret = anativewindow_dequeue(mAnw, &handle);
        if (mMgr && ret) {
            mMgr->freeOnMatch(handle);
        }
        return NOERROR;
    }
    return INVALID_STATE;
}

bool ImageBuffer::internalRelease()
{
    mTimestamp = 0xFFFFFFFFFFFFFFFFULL;

    //If no client is registered for callbacks, the buffer will be automatically displayed
    //if there is a surface bound to this buffer
    bool render = mPendingRender || mClient == NULL;
    display(render);

    //When using ANativeWindow (a surface/surface texture has been bound) this buffer cannot
    //be given back to the camera as it will be displayed (ANativeWindow owns it)
    if (mCam && mAnw == NULL) {
        ARX_PRINT(ARX_ZONE_BUFFER, "buffer:%p, index:%d returning to camera\n", this, mIndex);
        mCam->returnFrame(mCamFrame);
        return false;
    }
    //Inform the base class to not return this buffer to the free queue when we are using
    //ANativeWindow since it will be displayed
    return mAnw == NULL ? true : false;
}

arxstatus_t ImageBuffer::allocate()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);

    if (mRemote) {
        ARX_PRINT(ARX_ZONE_ERROR, "%s: trying to allocate remote image!\n", __FUNCTION__);
        return INVALID_STATE;
    }

    if (mAnw != NULL) {
        uint8_t *ptrs[3] = {NULL, NULL, NULL};
        int32_t stride = 0;
        if (!anativewindow_acquire(mAnw, &(mImage.reserved), ptrs, &stride)) {
            ARX_PRINT(ARX_ZONE_ERROR, "failed allocating native window buffer!\n");
            return NO_MEMORY;
        }
        mImage.y_stride = stride;
        mImage.pBuffer[0] = mImage.pData[0] = ptrs[0];
        if (mImage.color == FOURCC_NV12) {
            mImage.pBuffer[1] = mImage.pData[1] = &ptrs[0][mImage.y_stride*mImage.bufHeight];
        }
        return NOERROR;
    }

    bool ret = DVP_Image_Alloc(mDvp, &mImage, static_cast<DVP_MemType_e>(mImage.memType));
    return ret ? NOERROR : FAILED;
}

arxstatus_t ImageBuffer::free()
{
    if (mRemote) {
        if (mImage.memType == DVP_MTYPE_GRALLOC_2DTILED ||
            mImage.memType == DVP_MTYPE_DISPLAY_2DTILED) {
            native_handle_t  *handle = reinterpret_cast<native_handle_t *>(mImage.reserved);
            if (!mUsingTexture) {
                GraphicBufferMapper &mapper = GraphicBufferMapper::get();
                mapper.unlock(handle);
                mapper.unregisterBuffer(handle);
            }
            native_handle_close(handle);
            native_handle_delete(handle);
        } else {
            DVP_Image_Free_Import(mDvp, &mImage, mImportHdls);
            if (mImage.memType != DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED) {
                for (uint32_t i = 0; i < mImage.planes; i++) {
                    close(mSharedFds[i]);
                }
            }
        }
        return NOERROR;
    }

    if (mAnw) {
        anativewindow_release(mAnw, &(mImage.reserved));
    } else {
        DVP_Image_Free(mDvp, &mImage);
        if (mImage.memType != DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED) {
            for (uint32_t i = 0; i < mImage.planes; i++) {
                close(mSharedFds[i]);
            }
        }
    }
    return NOERROR;
}

arxstatus_t ImageBuffer::import(DVP_Handle dvp)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    if (!mRemote) {
        ARX_PRINT(ARX_ZONE_ERROR, "%s: Trying to import a non-remote buffer!\n", __FUNCTION__);
        return INVALID_STATE;
    }

    mDvp = dvp;
    if (mImage.memType == DVP_MTYPE_GRALLOC_2DTILED ||
        mImage.memType == DVP_MTYPE_DISPLAY_2DTILED) {
        GraphicBufferMapper &mapper = GraphicBufferMapper::get();
        buffer_handle_t handle = reinterpret_cast<buffer_handle_t>(mImage.reserved);
        status_t status = NO_ERROR;
        if (!mUsingTexture) {
            //Apparently surface textures get allocated in the same process as
            //the client, which makes registerBuffer fail
            status_t status = mapper.registerBuffer(handle);
            if (status != NO_ERROR) {
                ARX_PRINT(ARX_ZONE_ERROR, "%s: failed registering buffer! (0x%x)\n", __FUNCTION__, status);
                return FAILED;
            }
        }

        int usage = GRALLOC_USAGE_SW_READ_OFTEN;
        uint8_t *ptrs[3] = {NULL, NULL, NULL};
        Rect bounds;
        bounds.left = 0;
        bounds.top = 0;
        bounds.right = mImage.bufWidth;
        bounds.bottom = mImage.bufHeight;
        status = mapper.lock(handle, usage, bounds, (void **)ptrs);
        if (status != NO_ERROR) {
            ARX_PRINT(ARX_ZONE_ERROR, "%s: failed locking buffer! (0x%x)\n", __FUNCTION__, status);
            return FAILED;
        }

        if (mUsingTexture) {
            //TODO EGL cannot allocate if we keep a lock to the buffer
            //what is the implication of not holding lock to the pointer?
            mapper.unlock(handle);
        }

        ARX_PRINT(ARX_ZONE_BUFFER, "imported gralloc/display buffer pBuf=%p width:%u, height:%u, format:%u, stride:%u\n",
                    ptrs[0], mImage.width, mImage.height, mImage.color, mImage.y_stride);
        mImage.pBuffer[0] = mImage.pData[0] = ptrs[0];
        if (mImage.color == FOURCC_NV12) {
            mImage.pBuffer[1] = mImage.pData[1] = &ptrs[0][mImage.y_stride*mImage.bufHeight];
            mImage.planes = 2;
        }
        return NOERROR;
    }

    bool ret = DVP_Image_Import(dvp, &mImage, mSharedFds, mImportHdls);
    if (ret) {
        DVP_PrintImage(ARX_ZONE_BUFFER, &mImage);
    }
    return ret ? NOERROR : FAILED;
}

arxstatus_t ImageBuffer::writeToParcel(const sp<ImageBuffer>& imBuf, Parcel* parcel)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    parcel->writeInt32(imBuf->mIndex);
    //Serialize image info
    DVP_Image_t *pImage = &imBuf->mImage;
    parcel->writeInt32(pImage->planes);
    parcel->writeInt32(pImage->width);
    parcel->writeInt32(pImage->height);
    parcel->writeInt32(pImage->bufWidth);
    parcel->writeInt32(pImage->bufHeight);
    parcel->writeInt32(pImage->x_start);
    parcel->writeInt32(pImage->y_start);
    parcel->writeInt32(pImage->x_stride);
    parcel->writeInt32(pImage->y_stride);
    parcel->writeInt32(pImage->color);
    parcel->writeInt32(pImage->numBytes);
    parcel->writeInt32(pImage->memType);

    if (pImage->memType == DVP_MTYPE_GRALLOC_2DTILED ||
        pImage->memType == DVP_MTYPE_DISPLAY_2DTILED) {
        parcel->writeNativeHandle((native_handle_t *)pImage->reserved);
        parcel->writeInt32(imBuf->mUsingTexture);
    } else {
        bool ret = DVP_Image_Share(imBuf->mDvp, &imBuf->mImage, imBuf->mSharedFds);
        if (ret) {
            for (uint32_t i = 0; i < pImage->planes; i++) {
                if (pImage->memType == DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED) {
                    parcel->writeInt32(imBuf->mSharedFds[i]);
                } else {
                    parcel->writeDupFileDescriptor(imBuf->mSharedFds[i]);
                }
            }
        } else {
            return FAILED;
        }
    }
    return NOERROR;
}

sp<ImageBuffer> ImageBuffer::readFromParcel(const Parcel& data)
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    sp<ImageBuffer> imBuf = new ImageBuffer();

    imBuf->mRemote = true;
    imBuf->mIndex = data.readInt32();
    DVP_Image_t *pImage = &imBuf->mImage;
    pImage->planes = data.readInt32();
    pImage->width = data.readInt32();
    pImage->height = data.readInt32();
    pImage->bufWidth = data.readInt32();
    pImage->bufHeight = data.readInt32();
    pImage->x_start = data.readInt32();
    pImage->y_start = data.readInt32();
    pImage->x_stride = data.readInt32();
    pImage->y_stride = data.readInt32();
    pImage->color = data.readInt32();
    pImage->numBytes = data.readInt32();
    pImage->memType = data.readInt32();

    if (pImage->memType == DVP_MTYPE_GRALLOC_2DTILED ||
        pImage->memType == DVP_MTYPE_DISPLAY_2DTILED) {
        pImage->reserved = data.readNativeHandle();
        imBuf->mUsingTexture = data.readInt32();
    } else {
        for (uint32_t i = 0; i < pImage->planes; i++) {
            if (pImage->memType == DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED) {
                imBuf->mSharedFds[i] = data.readInt32();
            } else {
                imBuf->mSharedFds[i] = dup(data.readFileDescriptor());
            }
        }
    }

    return imBuf;
}

}
