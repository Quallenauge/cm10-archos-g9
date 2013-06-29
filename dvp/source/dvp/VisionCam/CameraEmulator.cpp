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

#include <CameraEmulator.h>
#include <dvp/dvp_debug.h>

CameraEmulator::CameraEmulator()
{
    uint32_t i = 0;
    for (i = 0; i < VCAM_NUM_BUFFERS; i++)
    {
        memset(&m_buffers[i], 0, sizeof(VCAM_Buffer_t));
    }

    // intialize to the default values.
    m_height = 320;
    m_width  = 240;
    m_fps    = 30;
    m_space  = FOURCC_UYVY;
    m_timer = 1000/m_fps;
    m_lastIdx = VCAM_NUM_BUFFERS - 1;
    DVP_PRINT(DVP_ZONE_API, "+CameraEmulator()\n");
}

CameraEmulator::~CameraEmulator()
{
    DVP_PRINT(DVP_ZONE_API, "-CameraEmulator()\n");
}

status_e CameraEmulator::init(void *cookie)
{
    m_cookie = cookie;
    return STATUS_SUCCESS;
}

status_e CameraEmulator::deinit()
{
    return STATUS_SUCCESS;
}

status_e CameraEmulator::useBuffers(DVP_Image_t *prvBufArr,
                                    uint32_t numPrvBuf/*, void* extraDataBuf, int numExtraDataBuf*/,
                                    VisionCamPort_e port __attribute__ ((unused)))
{
    // add the buffer to the list of usable buffers
    uint32_t b, i = 0;
    uint32_t numRegistered = 0;
    for (b = 0; b < numPrvBuf; b++)
    {
        DVP_Image_t *pImage = &prvBufArr[b];
        DVP_PRINT(DVP_ZONE_CAM, "[%u] Attempting to register buffer %p to Camera\n", b, pImage);
        for (i = 0; i < VCAM_NUM_BUFFERS; i++)
        {
            if (m_buffers[i].data == NULL)
            {
                m_buffers[i].data = pImage;
                m_buffers[i].used = 0;
                m_buffers[i].size = GetImageSize(m_width, m_height, m_space);
                DVP_PRINT(DVP_ZONE_CAM, "Registered Buffer %p\n", m_buffers[i].data);
                numRegistered++;
                break;
            }
        }
    }
    DVP_PRINT(DVP_ZONE_CAM, "Registered %u buffers\n", numRegistered);
    return STATUS_SUCCESS;
}

status_e CameraEmulator::flushBuffers(VisionCamPort_e port __attribute__ ((unused)))
{
    // nothing to do
    return STATUS_SUCCESS;
}

status_e CameraEmulator::releaseBuffers(VisionCamPort_e port __attribute__ ((unused)))
{
    // nothing to do
    return STATUS_SUCCESS;
}

status_e CameraEmulator::setParameter(VisionCamParam_e paramId,
                                      void* param,
                                      uint32_t size,
                                      VisionCamPort_e port __attribute__ ((unused)))
{
    status_e err = STATUS_SUCCESS;
    switch (paramId)
    {
        case VCAM_PARAM_WIDTH:
            memcpy(&m_width, param, size);
            break;
        case VCAM_PARAM_HEIGHT:
            memcpy(&m_height, param, size);
            break;
        case VCAM_PARAM_FPS_FIXED:
            memcpy(&m_fps, param, size);
            m_timer = 1000/m_fps;
            DVP_PRINT(DVP_ZONE_CAM, "Timer is set to %d ms\n", m_timer);
            break;
        case VCAM_PARAM_NAME:
            memset(m_name, 0, sizeof(m_name));
            strncpy(m_name, (char *)param, size);
            DVP_PRINT(DVP_ZONE_CAM, "Setting File Name to %s\n", m_name);
            break;
        case VCAM_PARAM_COLOR_SPACE_FOURCC:
            memcpy(&m_space, param, size);
            DVP_PRINT(DVP_ZONE_CAM, "Setting FOURCC 0x%08x\n", m_space);
            break;
        case VCAM_PARAM_CAP_MODE:
        case VCAM_PARAM_FLICKER:
        case VCAM_PARAM_SENSOR_SELECT:
        case VCAM_PARAM_ROTATION:
        case VCAM_PARAM_DO_MANUALFOCUS:
        case VCAM_PARAM_STEREO_INFO:
        case VCAM_PARAM_AWB_MODE:
        case VCAM_PARAM_DO_AUTOFOCUS:
            // just ignore these and pretend like it worked
            break;
        default:
            err = STATUS_NOT_IMPLEMENTED;
            if(paramId < VCAM_PARAM_MIN || paramId > VCAM_PARAM_MAX)
                err = STATUS_INVALID_PARAMETER;
            break;
    }
    return err;
}

status_e CameraEmulator::getParameter(VisionCamParam_e paramId,
                                      void* param,
                                      uint32_t size,
                                      VisionCamPort_e port __attribute__ ((unused)))
{
    status_e err = STATUS_SUCCESS;
    switch (paramId)
    {
        case VCAM_PARAM_WIDTH:
            memcpy( param, &m_width, size);
            break;
        case VCAM_PARAM_HEIGHT:
            memcpy(param, &m_height, size);
            break;
        case VCAM_PARAM_FPS_FIXED:
            memcpy(param, &m_fps, size);
            m_timer = 1000/m_fps;
            break;
        case VCAM_PARAM_NAME:
            strncpy((char *)param, m_name, (size < strlen(m_name) ? size : strlen(m_name)));
            break;
        case VCAM_PARAM_COLOR_SPACE_FOURCC:
            memcpy(param, &m_space, size);
            break;
        case VCAM_PARAM_2DBUFFER_DIM:
        {
            VisionCamResType *pRes = (VisionCamResType *)param;
            if (pRes && size == sizeof(VisionCamResType))
            {
                pRes->mWidth = m_width;
                pRes->mHeight = m_height;
                DVP_PRINT(DVP_ZONE_CAM, "VCAM: %ux%u starting at %dx%d\n", m_width, m_height, 0, 0);
            }
            else
                err = STATUS_INVALID_PARAMETER;
            break;
        }
        default:
            err = STATUS_INVALID_PARAMETER;
            break;
    }
    return err;
}

status_e CameraEmulator::returnFrame(VisionCamFrame *cameraFrame)
{
    for (uint32_t i = 0; i < VCAM_NUM_BUFFERS; i++)
    {
        if (m_buffers[i].data == cameraFrame->mFrameBuff)
        {
            DVP_PRINT(DVP_ZONE_CAM, "Returned buffer %d (%p)\n", i, cameraFrame->mFrameBuff);
            m_buffers[i].used = 0;
            return STATUS_SUCCESS;
        }
    }
    return STATUS_INVALID_PARAMETER;
}

// Default implementation
status_e CameraEmulator::sendCommand(VisionCamCmd_e cmdId,
                                    void *param __attribute__ ((unused)) ,
                                    uint32_t size __attribute__ ((unused)) ,
                                    VisionCamPort_e port __attribute__ ((unused)) )
{
    status_e err = STATUS_SUCCESS;
    switch (cmdId)
    {
        case VCAM_CMD_PREVIEW_START:
            StartThread(this);
            break;
        case VCAM_CMD_QUIT:
        case VCAM_CMD_PREVIEW_STOP:
            StopThread();
            break;
        case VCAM_EXTRA_DATA_START:
        case VCAM_EXTRA_DATA_STOP:
            // ignore
            break;
        default:
            err = STATUS_INVALID_PARAMETER;
            break;
    }
    return err;
}

int32_t CameraEmulator::getNextFreeBuffer()
{
    int32_t i = m_lastIdx; // start with the last index used
    DVP_PRINT(DVP_ZONE_CAM, "Last Used Buffer was %d\n", i);
    do {
        // go to the next index
        i = (i + 1) % VCAM_NUM_BUFFERS;
        DVP_PRINT(DVP_ZONE_CAM, "Checking buffer %d\n", i);
        if (m_buffers[i].data && m_buffers[i].used == 0)
        {
            // free to be used
            m_lastIdx = i;
            DVP_PRINT(DVP_ZONE_CAM, "Using Buffer %d\n", i);
            return i;
        }
    } while (i != m_lastIdx);

    DVP_PRINT(DVP_ZONE_CAM, "No free buffer found!\n");
    return -1;
}


