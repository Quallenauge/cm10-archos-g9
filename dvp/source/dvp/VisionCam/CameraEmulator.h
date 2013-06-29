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

#ifndef _CAMERA_EMULATOR_H_
#define _CAMERA_EMULATOR_H_

#include <sosal/CThreaded.h>
#include <dvp/VisionCam.h>

typedef struct _vcam_buffer_t {
    DVP_Image_t  *data;
    int    used;    /**< This indicates whether the buffer has been registered but returned to the user at the moment */
    size_t size;
} VCAM_Buffer_t;

class CameraEmulator
  : public CThreaded,
    public VisionCam
{
protected:
    VCAM_Buffer_t  m_buffers[VCAM_NUM_BUFFERS];
    char           m_name[MAX_PATH];        /**< The predicate name of the file */
    uint32_t       m_width;                 /**< The width of the image */
    uint32_t       m_height;                /**< The height of the image */
    uint32_t       m_fps;                   /**< The frame rate of the camera */
    fourcc_t       m_space;                 /**< The fourcc_t code for the color space that the Camera is configured for */
    int32_t        m_timer;                 /**< The frame return timer */
    int32_t        m_lastIdx;               /**< The last camera buffer we used */
public:
    CameraEmulator();
    virtual ~CameraEmulator();

    virtual status_e init(void *cookie);
    virtual status_e deinit();
    virtual status_e useBuffers(DVP_Image_t *prvBufArr, uint32_t numPrvBuf/*, void* extraDataBuf, int numExtraDataBuf*/, VisionCamPort_e port = VCAM_PORT_ALL );
    virtual status_e releaseBuffers(VisionCamPort_e port = VCAM_PORT_ALL );
    virtual status_e flushBuffers(VisionCamPort_e port = VCAM_PORT_ALL );
    virtual status_e sendCommand(VisionCamCmd_e cmdId, void *param, uint32_t size, VisionCamPort_e port = VCAM_PORT_ALL );
    virtual status_e setParameter(VisionCamParam_e paramId, void * param, uint32_t size, VisionCamPort_e port = VCAM_PORT_ALL);
    virtual status_e getParameter(VisionCamParam_e paramId, void * param, uint32_t size, VisionCamPort_e port = VCAM_PORT_ALL );
    virtual status_e returnFrame(VisionCamFrame *cameraFrame);

    int32_t getNextFreeBuffer();
};

#endif

