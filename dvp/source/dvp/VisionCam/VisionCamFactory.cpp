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

#include <dvp/VisionCam.h>
#include <FileVisionCam.h>
#include <SocketVisionCam.h>
#if defined(VCAM_USE_OMX)
#include <OMXVisionCam.h>
#endif
#if defined(VCAM_USE_USB) && defined(LINUX)
#include <UVCVisionCam.h>
#endif
#if defined(DVP_USE_CAMERA_SERVICE)
#include <CSVisionCam.h>
#endif

extern "C" VisionCam * VisionCamFactory(VisionCam_e type)
{
    VisionCam *handle = NULL;
    switch (type)
    {
#if defined(VCAM_USE_OMX)
        case VISIONCAM_OMX:
            handle = new OMXVisionCam();
            break;
#endif
#if defined(VCAM_USE_USB) && defined(LINUX)
        case VISIONCAM_USB:
            handle = new UVCVisionCam();
            break;
#endif
#if defined(VCAM_SUPPORTS_SOCKETCAM)
        case VISIONCAM_SOCKET:
            handle = new SocketVisionCam();
            break;
#endif
#if defined(DVP_USE_CAMERA_SERVICE)
        case VISIONCAM_CS:
            handle = new CSVisionCam();
            break;
#endif
#if defined(DVP_USE_FS)
        case VISIONCAM_FILE:
        default:
            handle = new FileVisionCam();
            break;
#else
        default:
            break;
#endif
    }
    return handle;
}

VisionCam::VisionCam() :
    m_callback(NULL),
    m_pack_callback(NULL),
    m_focuscallback(NULL),
    m_cookie(NULL),
    m_frameNum(0)
{
    DVP_PRINT(DVP_ZONE_API, "VisionCam()\n");
}

VisionCam::~VisionCam()
{
    DVP_PRINT(DVP_ZONE_API, "~VisionCam()\n");
}

uint32_t VisionCam::GetFrameNumber()
{
    return m_frameNum;
}

/** Enable frame notification callback */
status_e VisionCam::enablePreviewCbk(FrameCallback_f callback)
{
    status_e greError = STATUS_SUCCESS;
    if (m_callback == NULL)
    {
        DVP_PRINT(DVP_ZONE_CAM, "Enabling Callback to %p\n", callback);
        m_callback = callback;
    }
    else
        greError = STATUS_INVALID_STATE;
    return greError;
}

status_e VisionCam::enablePackedFramesCbk(FramePackCallback_f callback)
{
    status_e greError = STATUS_SUCCESS;
    if (m_pack_callback == NULL)
    {
        DVP_PRINT(DVP_ZONE_CAM, "Enabling Pack Callback to %p\n", callback);
        m_pack_callback = callback;
    }
    else
        greError = STATUS_INVALID_STATE;
    return greError;
}


/** disable frame notification callback */
status_e VisionCam::disablePreviewCbk(FrameCallback_f callback)
{
    status_e greError = STATUS_SUCCESS;
    if (callback == m_callback)
    {
        DVP_PRINT(DVP_ZONE_CAM, "Disabling Callback to %p\n", callback);
        m_callback = NULL;
    }
    else
        greError = STATUS_INVALID_STATE;
    return greError;
}

status_e VisionCam::disablePackedFramesCbk(FramePackCallback_f callback)
{
    status_e greError = STATUS_SUCCESS;
    if ( callback == m_pack_callback )
    {
        DVP_PRINT(DVP_ZONE_CAM, "Disabling Pack Callback to %p\n", callback);
        m_pack_callback = NULL;
    }
    else
        greError = STATUS_INVALID_STATE;
    return greError;
}

/** Allows the client to set the focus callback */
status_e VisionCam::setFocusCallBack(FocusCallback_t callback)
{
    m_focuscallback = callback;
    return STATUS_SUCCESS;
}


uint32_t GetImageSize(uint32_t width, uint32_t height, fourcc_t fourcc)
{
    uint32_t size = 0;
    switch (fourcc)
    {
        case FOURCC_Y800:
            size = height * width;
            break;
        case FOURCC_UYVY:
        case FOURCC_YUY2:
        case FOURCC_YVYU:
        case FOURCC_VYUY:
            size = height * width * 2;
            break;
        case FOURCC_IYUV:
        case FOURCC_YV12:
            size = (height * width * 3)/2;
            break;
        case FOURCC_YUV9:
            break;
        case FOURCC_BGR:
        case FOURCC_RGB:
        case FOURCC_RGBP:
            size = height * width * 3;
            break;
        case FOURCC_RGBA:
        case FOURCC_BGRA:
        case FOURCC_ARGB:
        case FOURCC_HSLP:
            size = height * width * 4;
        default:
            break;
    }
    return size;
}

VisionCamFrame::VisionCamFrame()
{
   clear();
}

void VisionCamFrame::clear()
{
   mContext = NULL;
   mCookie = NULL;
   mFrameBuff = NULL;
   mExtraDataBuf = NULL;
   mFrameSource = VCAM_PORT_PREVIEW; // default to the common preview port
   mFd = 0;
   mTimestamp = 0;
   mWidth = 0;
   mHeight = 0;
   mOffsetX = 0;
   mOffsetY = 0;
   mStartX = 0;
   mStartY = 0;
   mLength = 0;
   mExtraDataLength = 0;
   mMetadata.mAncillary = NULL;
   mMetadata.mAutoWBGains = NULL;
   mMetadata.mManualWBGains = NULL;
   mMetadata.mHistogram2D = NULL;
   mMetadata.mHistogramL = NULL;
   mMetadata.mHistogramR = NULL;
   mMetadata.mGamma2D = NULL;
   mMetadata.mGammaL = NULL;
   mMetadata.mGammaR = NULL;
   mMetadata.mUnsaturatedRegions = NULL;
   mMetadata.mMTIS_Data = NULL;
   mMetadata.mFocusRegionData = NULL;

   mDetectedFacesNum = 0;
   memset(&mFaces, 0, sizeof(mFaces));
   mDetectedFacesNumRaw = 0;
   memset(&mFacesRaw, 0, sizeof(mFacesRaw));
}

