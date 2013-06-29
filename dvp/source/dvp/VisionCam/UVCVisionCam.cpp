/*
 *  Copyright (C) 2009-2011 Texas Instruments, Inc.
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

#include <UVCVisionCam.h>

UVCVisionCam::UVCVisionCam() : CThreaded(),
    m_frame()
{
    m_dev = NULL;
    for (uint32_t i = 0; i < dimof(m_images); i++)
        m_images[i] = NULL;
    m_numImages = 0;
    m_curImage = 0;
}
UVCVisionCam::~UVCVisionCam()
{
    if (m_dev)
        v4l2_close(&m_dev);
}

// interface methods
status_e UVCVisionCam::init(void *cookie)
{
    int device_num = 0;
#if defined(PANDA)
    device_num = 1;
#endif
    m_dev = v4l2_open(device_num, V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING, false_e);
    m_frame.clear();
    m_frame.mCookie = cookie;
    if (m_dev)
        return STATUS_SUCCESS;
    else
        return STATUS_NO_RESOURCES;
}
status_e UVCVisionCam::deinit()
{
    if (m_dev)
        v4l2_close(&m_dev);
    m_frame.clear();
    return STATUS_SUCCESS;
}

status_e UVCVisionCam::useBuffers(DVP_Image_t *prvBufArr, uint32_t numPrvBuf, VisionCamPort_e port __attribute__((unused)))
{
    status_e status = STATUS_SUCCESS;
    if (numPrvBuf > 0)
    {
        if (m_color != prvBufArr[0].color)
        {
            DVP_PRINT(DVP_ZONE_ERROR, "Images are in the wrong color space!\n");
            return STATUS_INVALID_PARAMETER;
        }
        m_numImages = numPrvBuf;
        for (uint32_t i = 0; i < m_numImages; i++)
            m_images[i] = &prvBufArr[i];
        m_frame.mContext = this;
        m_frame.mWidth = m_images[0]->width;
        m_frame.mHeight = m_images[0]->height;
        m_frame.mLength = m_images[0]->numBytes;
    }
    else
    {
        for (uint32_t i = 0; i < dimof(m_images); i++)
            m_images[i] = NULL;
    }

    // most UVC cameras will only support YUY2 or RGB
    if (v4l2_allocate(m_dev, m_images[0]->width, m_images[0]->height, m_numImages, FOURCC_YUY2, V4L2_BUF_TYPE_VIDEO_CAPTURE) == false_e)
    {
        DVP_PRINT(DVP_ZONE_ERROR, "Failed to allocate V4L2 buffers!\n");
        status = STATUS_NO_RESOURCES;
    }
    return status;
}
status_e UVCVisionCam::releaseBuffers(VisionCamPort_e port __attribute__((unused)))
{
    if (v4l2_free(m_dev))
        return STATUS_SUCCESS;
    else
        return STATUS_FAILURE;
}
status_e UVCVisionCam::flushBuffers(VisionCamPort_e port __attribute__((unused)))
{
    // nothing to do?
    return STATUS_SUCCESS;
}

status_e UVCVisionCam::sendCommand(VisionCamCmd_e cmdId, void *param, uint32_t size, VisionCamPort_e port __attribute__((unused)))
{
    status_e status = STATUS_SUCCESS;
    DVP_PRINT(DVP_ZONE_CAM, "UVC: CMD %u (0x%08x) %p %u\n", cmdId, cmdId, param, size);
    switch (cmdId)
    {
        case VCAM_CMD_PREVIEW_START:
            StartThread(this);
            break;
        case VCAM_CMD_PREVIEW_STOP:
        case VCAM_CMD_QUIT:
            StopThread();
            break;
        case VCAM_EXTRA_DATA_START:
        case VCAM_EXTRA_DATA_STOP:
            break; // lie
        default:
            status = STATUS_NOT_IMPLEMENTED;
            break;
    }
    return status;
}

status_e UVCVisionCam::setParameter(VisionCamParam_e paramId, void* param, uint32_t size, VisionCamPort_e port __attribute__((unused)))
{
    status_e status = STATUS_SUCCESS;
    DVP_PRINT(DVP_ZONE_CAM, "UVC: PARAM %u (0x%08x) %p %u\n", paramId, paramId, param, size);
    switch (paramId)
    {
        case VCAM_PARAM_WIDTH:
            if (size == sizeof(uint32_t))
                m_frame.mWidth = *(uint32_t *)param;
            break;
        case VCAM_PARAM_HEIGHT:
            if (size == sizeof(uint32_t))
                m_frame.mHeight = *(uint32_t *)param;
            break;
        case VCAM_PARAM_COLOR_SPACE_FOURCC:
            if (size == sizeof(fourcc_t))
                m_color = *(fourcc_t *)param;
            break;
        case VCAM_PARAM_FPS_FIXED:
            if (size == sizeof(uint32_t))
                m_fps = *(uint32_t *)param;
            break;
        case VCAM_PARAM_EXPOSURE_ISO:
        {
            int value = 0;
            if (size == sizeof(int))
                value = *(int *)param;
            v4l2_control_set(m_dev, V4L2_CID_EXPOSURE, value);
            break;
        }
        case VCAM_PARAM_BRIGHTNESS:
        {
            int value = 0;
            if (size == sizeof(int))
                value = *(int *)param;
            v4l2_control_set(m_dev, V4L2_CID_BRIGHTNESS, value);
            break;
        }
        case VCAM_PARAM_CONTRAST:
        {
            int value = 0;
            if (size == sizeof(int))
                value = *(int *)param;
            v4l2_control_set(m_dev, V4L2_CID_CONTRAST, value);
            break;
        }
        case VCAM_PARAM_SATURATION:
        {
            int value = 0;
            if (size == sizeof(int))
                value = *(int *)param;
            v4l2_control_set(m_dev, V4L2_CID_SATURATION, value);
            break;
        }
        default:
            status = STATUS_SUCCESS;
            break;
    }
    return status;
}

status_e UVCVisionCam::getParameter(VisionCamParam_e paramId, void* param, uint32_t size, VisionCamPort_e port __attribute__((unused)))
{
    status_e status = STATUS_NOT_IMPLEMENTED;
    switch (paramId)
    {
        case VCAM_PARAM_2DBUFFER_DIM:
            if (size == sizeof(VisionCamResType))
            {
                VisionCamResType *pRes = (VisionCamResType *)param;
                pRes->mWidth = m_frame.mWidth;
                pRes->mHeight = m_frame.mHeight;
                pRes->mResIdx = VCAM_RESOL_MAX;
                for (int32_t i = 0; i < dimof(VisionCamResolutions); i++)
                {
                    if (pRes->mWidth == VisionCamResolutions[i].mWidth &&
                        pRes->mHeight == VisionCamResolutions[i].mHeight)
                        pRes->mResIdx = (VisionCamResolIdex)i;
                }
                status = STATUS_SUCCESS;
            }
            break;
        default:
            break;
    }
    return status;
}

status_e UVCVisionCam::returnFrame(VisionCamFrame *cameraFrame)
{
    return STATUS_SUCCESS;
}

thread_ret_t UVCVisionCam::RunThread()
{
    uint32_t index = 0xFFFFFFFF;
    void *buffer = NULL;

    DVP_PRINT(DVP_ZONE_CAM, "UVC Dequeue Thread Running!\n");

    // queue up all buffers
    for (uint32_t i = 0; i < m_numImages; i++)
    {
        buffer = v4l2_acquire(m_dev, &index);
        if (buffer)
        {
            if (v4l2_queue(m_dev, index, V4L2_BUF_TYPE_VIDEO_CAPTURE))

            {
                DVP_PRINT(DVP_ZONE_CAM, "UVC: Queued %p[%u] to UVC\n", buffer, index);
            }
            else
            {
                DVP_PRINT(DVP_ZONE_ERROR, "Failed to queue %p[%u]\n", buffer, index);
            }
        }
        else
        {
            DVP_PRINT(DVP_ZONE_ERROR, "Failed to acquire buffer num %u\n", i);
        }
    }
    v4l2_start(m_dev, V4L2_BUF_TYPE_VIDEO_CAPTURE);

    index = 0xFFFFFFFF;
    buffer = NULL;

    while (m_running)
    {
        //if (v4l2_wait(m_dev))
        {
            if (v4l2_dequeue(m_dev, &index, V4L2_BUF_TYPE_VIDEO_CAPTURE) == true_e)
            {
                buffer = v4l2_search_index(m_dev, index);
                if (buffer)
                {
                    DVP_U08 *ptr = (DVP_U08 *)buffer;
                    DVP_Image_t image = *m_images[m_curImage]; // copy meta fields, pointers will be recalculated
                    // copy image pointer over
                    image.pData[0] = ptr;
                    image.color = FOURCC_YUY2;

                    /// @todo reconstruct multi-plane formats....
                    if (image.color == FOURCC_NV12)
                    {
                        image.pData[1] = &ptr[image.bufHeight * image.y_stride];
                    }

                    if (DVP_Image_Copy(m_images[m_curImage], &image) == DVP_TRUE)
                    {
                        m_frame.mFrameBuff = m_images[m_curImage];
                        m_frame.mTimestamp = rtimer_now();
                        if (m_callback)
                            m_callback(&m_frame);
                        m_curImage = (m_curImage + 1)%m_numImages;
                    }
                }
                else
                {
                    DVP_PRINT(DVP_ZONE_CAM, "Failed to find buffer with index %u\n", index);
                }

                // requeue the buffer...
                v4l2_queue(m_dev, index, V4L2_BUF_TYPE_VIDEO_CAPTURE);
        }
        }
    }
    DVP_PRINT(DVP_ZONE_CAM, "UVC Dequeue Thread Exiting!\n");
    thread_exit(0);
}

void UVCVisionCam::Shutdown()
{
    v4l2_stop(m_dev, V4L2_BUF_TYPE_VIDEO_CAPTURE);
}

