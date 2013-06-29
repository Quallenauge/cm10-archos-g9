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

#ifndef _UVC_VISION_CAM_H_
#define _UVC_VISION_CAM_H_

#if defined(LINUX)

#include <dvp/VisionCam.h>
#include <sosal/CThreaded.h>
#include <v4l2_api.h>

#define UVC_MAX_IMAGES  (10)

class UVCVisionCam : public CThreaded, public VisionCam
{
protected:
    v4l2_api_t    *m_dev;
    DVP_Image_t   *m_images[UVC_MAX_IMAGES];
    uint32_t       m_numImages;
    uint32_t       m_curImage;
    VisionCamFrame m_frame;
    fourcc_t       m_color;
    uint32_t       m_fps;

public:
    UVCVisionCam();
    virtual ~UVCVisionCam();

    // interface methods
    status_e init(void * cookie);
    status_e deinit();
    status_e startPreview();
    status_e stopPreview();
    status_e useBuffers(DVP_Image_t *prvBufArr, uint32_t numPrvBuf, VisionCamPort_e port=VCAM_PORT_PREVIEW);
    status_e releaseBuffers(VisionCamPort_e port=VCAM_PORT_PREVIEW);
    status_e flushBuffers(VisionCamPort_e port=VCAM_PORT_PREVIEW);
    status_e sendCommand(VisionCamCmd_e cmdId, void *param, uint32_t size, VisionCamPort_e port=VCAM_PORT_PREVIEW);
    status_e setParameter(VisionCamParam_e paramId, void* param, uint32_t size, VisionCamPort_e port=VCAM_PORT_PREVIEW);
    status_e getParameter(VisionCamParam_e paramId, void* param, uint32_t size, VisionCamPort_e port=VCAM_PORT_PREVIEW);
    status_e returnFrame(VisionCamFrame *cameraFrame);

    // CThreaded
    thread_ret_t RunThread();
    void Shutdown();
};

#endif

#endif

