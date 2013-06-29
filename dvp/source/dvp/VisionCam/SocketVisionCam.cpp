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

#include <SocketVisionCam.h>
#include <sosal/pyuv.h>
#include <time.h>

#if defined(SOSAL_RPC_SOCKET_API)

SocketVisionCam::SocketVisionCam() :
    CThreaded(),
    VisionCam(),
    m_imgs(NULL),
    m_data(NULL),
    m_ipaddr(0),
    m_frameData(NULL),
    m_numDataBytes(0),
    m_numImgBytes(0),
    m_client(NULL)
{
    DVP_PRINT(DVP_ZONE_API, "SocketVisionCam()+\n");
    m_imgs = queue_create(VCAM_NUM_BUFFERS*VCAM_PORT_MAX, sizeof(VisionCamFrame *)); // a queue of image pointers
    strncpy(m_hostname, "127.0.0.1", MAX_PATH);
    memset(m_filename, 0, sizeof(m_filename));
    pFile = NULL;
}

SocketVisionCam::~SocketVisionCam()
{
    queue_destroy(m_imgs);
    if (m_client) {
        rpc_socket_client_stop(m_client);
        m_client = NULL;
    }
    if (m_data) {
        socket_deinit(m_data);
        m_data = NULL;
    }
}

status_e SocketVisionCam::init(void *cookie)
{
    status_e status = STATUS_SUCCESS;
    m_cookie = cookie;
    m_ipaddr = socket_hostname(m_hostname);
    m_client = rpc_socket_client_start(m_hostname, VCAM_SOCKET_CMD_PORT);
    if (m_client)
    {
        StartThread(this);
        rpc_socket_item_t returns[] = {
            {0, RPC_SOCKET_TYPE_INT32, sizeof(int32_t), 1, &status},
        };
        if (rpc_socket_client_call(m_client, VISIONCAM_FUNCTION_INIT, NULL, 0, returns, dimof(returns)) == false_e)
            status = STATUS_NO_RESOURCES;
    }
    else
        status = STATUS_NO_RESOURCES;
    return status;
}

status_e SocketVisionCam::deinit()
{
    status_e status = STATUS_SUCCESS;
    rpc_socket_item_t returns[] = {
        {0, RPC_SOCKET_TYPE_INT32, sizeof(int32_t), 1, &status},
    };
    if (rpc_socket_client_call(m_client, VISIONCAM_FUNCTION_DEINIT, NULL, 0, returns, dimof(returns)))
    {
        rpc_socket_client_stop(m_client);
        m_client = NULL;
    }
    else
        status = STATUS_NO_RESOURCES;
    return status;
}

status_e SocketVisionCam::useBuffers(DVP_Image_t * pImages, uint32_t numImages, VisionCamPort_e port = VCAM_PORT_PREVIEW)
{
    uint32_t i = 0;
    status_e status = STATUS_SUCCESS;
    bool_e validColor = false_e;
    fourcc_t validColors[] = {
        FOURCC_UYVY,
        FOURCC_YUY2,
        FOURCC_BGR,
        FOURCC_NV12,
    };
    for (i = 0; i < dimof(validColors); i++)
        if (pImages[0].color == validColors[i])
            validColor = true_e;

    if (validColor && numImages < VCAM_NUM_BUFFERS)
    {
        DVP_Image_t image = pImages[0]; // implicit copy
        VisionCamFrame frame;
        frame.mFrameBuff = &image;

        m_numDataBytes = frame.maxSerialLength();
        m_frameData = (uint8_t *)calloc(1, m_numDataBytes);
        if (m_frameData == NULL)
            return STATUS_NOT_ENOUGH_MEMORY;
        PYUV_GetFilename(m_filename, (char *)"", (char *)"frameData", pImages[0].width, pImages[0].height, 30, pImages[0].color);
        //pFile = fopen(m_filename, "w+");
        for (uint32_t i = 0; i < numImages; i++)
        {
            DVP_Image_t *pImage = &pImages[i];
            DVP_PrintImage(DVP_ZONE_CAM, pImage);
            queue_write(m_imgs, true_e, &pImage);
        }

        VisionCamImages_t img = {
            pImages[0].width,
            pImages[0].height,
            pImages[0].color,
            numImages,
            port
        };
        rpc_socket_item_t params[] = {
            {0, RPC_SOCKET_TYPE_INT32, sizeof(VisionCamImages_t), 1, &img},
        };
        rpc_socket_item_t returns[] = {
            {0, RPC_SOCKET_TYPE_INT32, sizeof(int32_t), 1, &status},
        };
        if (rpc_socket_client_call(m_client, VISIONCAM_FUNCTION_USEBUFS, params, dimof(params), returns, dimof(returns)) == false_e)
            status = STATUS_NO_RESOURCES;
    }
    else
        status = STATUS_INVALID_PARAMETER;
    return status;
}

status_e SocketVisionCam::returnFrame(VisionCamFrame *cameraFrame)
{
    status_e status = STATUS_SUCCESS;
    if (m_imgs && cameraFrame)
        queue_write(m_imgs, true_e, &cameraFrame);
    else
        status = STATUS_INVALID_PARAMETER;
    return status;
}

status_e SocketVisionCam::releaseBuffers( VisionCamPort_e port __attribute__((unused)))
{
    status_e status = STATUS_SUCCESS;
    rpc_socket_item_t returns[] = {
        {0, RPC_SOCKET_TYPE_INT32, sizeof(int32_t), 1, &status},
    };
    if (rpc_socket_client_call(m_client, VISIONCAM_FUNCTION_RLSBUFS, NULL, 0, returns, dimof(returns)))
    {
        DVP_Image_t *pImage = NULL;
        StopThread(); // only stop now
        if (m_running == false_e)
        {
            size_t l = 0;
            DVP_PRINT(DVP_ZONE_CAM, "Freeing all images from queue and freeing frame data.\n");
            // read all images off the queue
            queue_unpop(m_imgs);
            do {
                l = queue_length(m_imgs);
                DVP_PRINT(DVP_ZONE_CAM, ""FMT_SIZE_T" images left on queue\n",l);
                if (queue_read(m_imgs, false_e, &pImage) == true_e)
                    DVP_PrintImage(DVP_ZONE_CAM, pImage);
            } while (l > 0);
            //fclose(pFile);
            DVP_PRINT(DVP_ZONE_CAM, "Freeing frame data %p\n",m_frameData);
            if (m_frameData)
                free(m_frameData);
            m_frameData = NULL;
            DVP_PRINT(DVP_ZONE_CAM, "Released Buffers\n");
        }
        else
        {
            DVP_PRINT(DVP_ZONE_WARNING, "Attempting to release buffers while DataThread is still running!\n");
        }
    }
    else
        status = STATUS_NO_RESOURCES;
    return status;
}

status_e SocketVisionCam::flushBuffers( VisionCamPort_e port __attribute__((unused)))
{
    status_e status = STATUS_SUCCESS;
    rpc_socket_item_t returns[] = {
        {0, RPC_SOCKET_TYPE_INT32, sizeof(int32_t), 1, &status},
    };
    if (rpc_socket_client_call(m_client, VISIONCAM_FUNCTION_FLUSHBUFS, NULL, 0, returns, dimof(returns)) == false_e)
        status = STATUS_NO_RESOURCES;
    return status;
}

status_e SocketVisionCam::sendCommand(VisionCamCmd_e cmdId, void * param, uint32_t size,
                                      VisionCamPort_e port __attribute__ ((unused)))
{
    status_e status = STATUS_SUCCESS;
    rpc_socket_item_t params[] = {
        {0, RPC_SOCKET_TYPE_INT32, sizeof(int32_t), 1, &cmdId},
        {0, RPC_SOCKET_TYPE_UINT8, 1, size, param},
    };
    rpc_socket_item_t returns[] = {
        {0, RPC_SOCKET_TYPE_INT32, sizeof(int32_t), 1, &status},
    };
    if (rpc_socket_client_call(m_client, VISIONCAM_FUNCTION_SENDCMD, params, dimof(params), returns, dimof(returns)) == false_e)
        status = STATUS_NO_RESOURCES;
    return status;
}

status_e SocketVisionCam::setParameter(VisionCamParam_e paramId, void * param, uint32_t size,
                                       VisionCamPort_e port __attribute__ ((unused)))
{
    status_e status = STATUS_SUCCESS;
    if ((SocketVisionCam_Param_e)paramId == SOCKETVCAM_PARAM_HOSTNAME)
    {
        strncpy(m_hostname, (char *)param, (size > MAX_PATH ? MAX_PATH:size));
    }
    else
    {
        rpc_socket_item_t params[] = {
            {0, RPC_SOCKET_TYPE_INT32, sizeof(int32_t), 1, &paramId},
            {0, RPC_SOCKET_TYPE_UINT8, 1, size, param},
        };
        rpc_socket_item_t returns[] = {
            {0, RPC_SOCKET_TYPE_INT32, sizeof(int32_t), 1, &status},
        };
        if (rpc_socket_client_call(m_client, VISIONCAM_FUNCTION_SETPARAMS, params, dimof(params), returns, dimof(returns)) == false_e)
            status = STATUS_NO_RESOURCES;
    }
    return status;
}

status_e SocketVisionCam::getParameter(VisionCamParam_e paramId, void * param, uint32_t size,
                                       VisionCamPort_e port __attribute__ ((unused)))
{
    status_e status = STATUS_SUCCESS;
    if ((SocketVisionCam_Param_e)paramId == SOCKETVCAM_PARAM_HOSTNAME)
    {
        strncpy((char *)param, m_hostname, (size > MAX_PATH ? MAX_PATH:size));
    }
    else
    {
        rpc_socket_item_t params[] = {
            {0, RPC_SOCKET_TYPE_INT32, sizeof(int32_t), 1, &paramId},
            {0, RPC_SOCKET_TYPE_UINT32, sizeof(uint32_t), 1, &size},
        };
        rpc_socket_item_t returns[] = {
            {0, RPC_SOCKET_TYPE_INT32, sizeof(int32_t), 1, &status},
            {0, RPC_SOCKET_TYPE_UINT8, 1, size, param},
        };
        if (rpc_socket_client_call(m_client, VISIONCAM_FUNCTION_GETPARAMS, params, dimof(params), returns, dimof(returns)) == false_e)
            status = STATUS_NO_RESOURCES;
    }
    return status;
}

void SocketVisionCam::Shutdown()
{
    DVP_PRINT(DVP_ZONE_CAM, "Closing RunThread()\n");
    queue_pop(m_imgs); // pop out of the queue_read
}

thread_ret_t SocketVisionCam::RunThread()
{
    thread_ret_t ret = (thread_ret_t)-2;
    VisionCamFrame frame;

    m_data = socket_init(m_ipaddr, VCAM_SOCKET_DATA_PORT, SOCKET_TCP, false_e);
    if (m_data)
    {
        if (socket_connect(m_data) == false_e) {
            thread_exit(ret);
        }

        DVP_PRINT(DVP_ZONE_CAM, "SocketVisionCam RunThread Launched!\n");
        while (m_running)
        {
            DVP_Image_t *pImage = NULL;

            // while there's frames to read...
            DVP_PRINT(DVP_ZONE_CAM, "SocketVisionCam Waiting on Image in Queue!\n");
            if (queue_read(m_imgs, true_e, &pImage) == true_e)
            {
                uint32_t offset = 0;
                uint32_t len = 0;

                if (m_running == false_e || m_frameData == NULL)
                    break;

                DVP_PRINT(DVP_ZONE_CAM, "SocketVisionCam Waiting on VisionCamFrame!\n");
                if (socket_recv(m_data, (uint8_t *)&len, sizeof(uint32_t)) == false || len == 0)
                    break;

                if (socket_recv(m_data, &m_frameData[offset], len) == false_e)
                {
                    DVP_PRINT(DVP_ZONE_WARNING, "Failed to read frame header, socket could be closing!\n");
                    if (m_running == false_e || m_frameData == NULL)
                        break;
                }

                // fill in the data in the frame data
                frame.mFrameBuff = pImage;
                frame.mCookie = m_cookie;

                // read the whole frame in
                offset += frame.unserialize(&m_frameData[offset], len-offset);

                DVP_PRINT(DVP_ZONE_CAM, "SocketVisionCam Calling Callback!\n");

                // call the client
                if (m_callback)
                    m_callback(&frame);

                // free the meta data if it existed
                if (frame.mMetadata.mAutoWBGains) {
                    free(frame.mMetadata.mAutoWBGains);
                    frame.mMetadata.mAutoWBGains = NULL;
                }
                if (frame.mMetadata.mManualWBGains) {
                    free(frame.mMetadata.mManualWBGains);
                    frame.mMetadata.mManualWBGains = NULL;
                }
                if (frame.mMetadata.mAncillary) {
                    free(frame.mMetadata.mAncillary);
                    frame.mMetadata.mAncillary = NULL;
                }
            }
            else
            {
                DVP_PRINT(DVP_ZONE_WARNING, "Failed to read queue for Images. We may be exitting.\n");
                break;
            }
            ret = (thread_ret_t)0;
        }

        socket_disconnect(m_data);
        socket_deinit(m_data);
        m_data = NULL;

    }
    DVP_PRINT(DVP_ZONE_CAM, "SocketVisionCam Data Thread Exiting!\n");
    thread_exit(ret);
}

#endif
