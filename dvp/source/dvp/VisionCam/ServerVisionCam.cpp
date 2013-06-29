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

#include <ServerVisionCam.h>

ServerVisionCam::ServerVisionCam() :
    m_dvp(0),
    m_running(false_e),
    m_server(NULL),
    m_data(NULL),
    m_pCam(NULL),
    m_frameData(NULL),
    m_numDataBytes(0),
    m_numImageBytes(0),
    m_pImages(NULL),
    m_status(STATUS_SUCCESS),
    m_param(NULL),
//#if defined(__QNX__)
    m_display_enabled(true_e),
//#else
//    m_display_enabled(false_e),
//#endif
    m_display(NULL),
    m_allocWidth(0),
    m_allocHeight(0)
{
    event_init(&m_wait, false_e);
    memset(&m_img, 0, sizeof(VisionCamImages_t));
    m_dvp = DVP_KernelGraph_Init();
    m_pImages = (DVP_Image_t *)calloc(VCAM_NUM_BUFFERS, sizeof(DVP_Image_t));
}

ServerVisionCam::~ServerVisionCam()
{
    if (m_param) {
        free(m_param);
        m_param = NULL;
    }
    free(m_pImages);
    DVP_KernelGraph_Deinit(m_dvp);
    event_deinit(&m_wait);
}

void ServerVisionCam::FrameCallback(VisionCamFrame * cameraFrame)
{
    ServerVisionCam *pSVCam = reinterpret_cast<ServerVisionCam *>(cameraFrame->mCookie);
    uint32_t offset = 0;

    if (pSVCam->m_display_enabled && cameraFrame->mFrameSource == VCAM_PORT_PREVIEW)
    {
        DVP_Image_t *pImage = (DVP_Image_t *)cameraFrame->mFrameBuff;
        DVP_Display_Render(pSVCam->m_display, pImage);
    }

    // serialize the camera frame to a flat buffer
    offset += cameraFrame->serialize(pSVCam->m_frameData, pSVCam->m_numDataBytes);

    // send the buffer size
    socket_send(pSVCam->m_data, (uint8_t *)&offset, sizeof(offset));

    // send the frame
    socket_send(pSVCam->m_data, pSVCam->m_frameData, offset);

    pSVCam->m_pCam->returnFrame(cameraFrame);
}

void ServerVisionCam::FocusCallback(int state)
{
    state = state; // warnings
    DVP_PRINT(DVP_ZONE_ALWAYS, "Focus state %d\n", state);
}

bool_e ServerVisionCam::init(void *arg, rpc_socket_item_t *params, uint32_t numParams, rpc_socket_item_t *returns, uint32_t *numReturns)
{
    ServerVisionCam *pSVCam = (ServerVisionCam *)arg;
    VisionCam *pCam = pSVCam->m_pCam;
    params = params; // remove warning
    if (pCam && numParams == 0)
    {
        pSVCam->m_status = pCam->init(pSVCam);
        if (pSVCam->m_status == STATUS_SUCCESS)
        {
            socket_connect(pSVCam->m_data);
            pCam->enablePreviewCbk(FrameCallback);
            pCam->setFocusCallBack(FocusCallback);
        }
        returns[0].type = RPC_SOCKET_TYPE_INT32;
        returns[0].numElements = 1;
        returns[0].sizeElement = sizeof(status_e);
        returns[0].item = &pSVCam->m_status; // must be static or class variable!
        *numReturns = 1;
        return true_e;
    }
    else
        return false_e;
}

bool_e ServerVisionCam::deinit(void *arg, rpc_socket_item_t *params, uint32_t numParams, rpc_socket_item_t *returns, uint32_t *numReturns)
{
    ServerVisionCam *pSVCam = (ServerVisionCam *)arg;
    VisionCam *pCam = pSVCam->m_pCam;
    params = params; // remove warning
    if (pCam && numParams == 0)
    {
        pCam->setFocusCallBack(NULL);
        pSVCam->m_status = pCam->deinit();

        returns[0].type = RPC_SOCKET_TYPE_INT32;
        returns[0].numElements = 1;
        returns[0].sizeElement = sizeof(status_e);
        returns[0].item = &pSVCam->m_status; // must be static or class variable!
        *numReturns = 1;
        event_set(&pSVCam->m_wait); // signal that we're done...
        return true_e;
    }
    else
        return false_e;
}

bool_e ServerVisionCam::setParameter(void *arg, rpc_socket_item_t *params, uint32_t numParams, rpc_socket_item_t *returns, uint32_t *numReturns)
{
    ServerVisionCam *pSVCam = (ServerVisionCam *)arg;
    VisionCam *pCam = pSVCam->m_pCam;
    if (pCam && numParams == 2 &&
        params[0].numElements == 1 && params[0].sizeElement == sizeof(int32_t))
    {
        VisionCamParam_e paramId = *(VisionCamParam_e *)params[0].item;
        void *param = params[1].item;
        uint32_t size = params[1].numElements * params[1].sizeElement;
        pSVCam->m_status = pCam->setParameter(paramId, param, size);
        returns[0].type = RPC_SOCKET_TYPE_INT32;
        returns[0].numElements = 1;
        returns[0].sizeElement = sizeof(status_e);
        returns[0].item = &pSVCam->m_status; // must be static or class variable!
        *numReturns = 1;
        return true_e;
    }
    else
        return false_e;
}

bool_e ServerVisionCam::getParameter(void *arg, rpc_socket_item_t *params, uint32_t numParams, rpc_socket_item_t *returns, uint32_t *numReturns)
{
    ServerVisionCam *pSVCam = (ServerVisionCam *)arg;
    VisionCam *pCam = pSVCam->m_pCam;
    if (pCam && numParams == 2 &&
        params[0].numElements == 1 && params[0].sizeElement == sizeof(int32_t) &&
        params[1].numElements == 1 && params[0].sizeElement == sizeof(uint32_t))
    {
        VisionCamParam_e paramId = *(VisionCamParam_e *)params[0].item;
        uint32_t size = *(uint32_t *)params[1].item;

        if (pSVCam->m_param) {
            free(pSVCam->m_param);
            pSVCam->m_param = NULL;
        }
        if (pSVCam->m_param == NULL) {
            pSVCam->m_param = calloc(1, size);
        }

        pSVCam->m_status = pCam->getParameter(paramId, pSVCam->m_param, size);

        if (paramId == VCAM_PARAM_2DBUFFER_DIM)
        {
            VisionCamResType *res = (VisionCamResType *)pSVCam->m_param;

            pSVCam->m_allocWidth = res->mWidth;
            pSVCam->m_allocHeight = res->mHeight;
        }

        returns[0].type = RPC_SOCKET_TYPE_INT32;
        returns[0].numElements = 1;
        returns[0].sizeElement = sizeof(status_e);
        returns[0].item = &pSVCam->m_status; // must be static or class variable!
        returns[1].type = RPC_SOCKET_TYPE_INT8;
        returns[1].numElements = size;
        returns[1].sizeElement = sizeof(uint8_t);
        returns[1].item = pSVCam->m_param; // must be static or class variable!
        *numReturns = 2;
        return true_e;
    }
    else
        return false_e;
}

bool_e ServerVisionCam::sendCommand(void *arg, rpc_socket_item_t *params, uint32_t numParams, rpc_socket_item_t *returns, uint32_t *numReturns)
{
    ServerVisionCam *pSVCam = (ServerVisionCam *)arg;
    VisionCam *pCam = pSVCam->m_pCam;
    if (pCam && numParams == 2 &&
        params[0].numElements == 1 && params[0].sizeElement == sizeof(int32_t))
    {
        VisionCamCmd_e cmd = *(VisionCamCmd_e *)params[0].item;
        void *param = params[1].item;
        uint32_t size = params[1].numElements * params[1].sizeElement;

        pSVCam->m_status = pCam->sendCommand(cmd, param, size);

        returns[0].type = RPC_SOCKET_TYPE_INT32;
        returns[0].numElements = 1;
        returns[0].sizeElement = sizeof(status_e);
        returns[0].item = &pSVCam->m_status; // must be static or class variable!
        *numReturns = 1;

        return true_e;
    }
    else
        return false_e;
}

bool_e ServerVisionCam::useBuffers(void *arg, rpc_socket_item_t *params, uint32_t numParams, rpc_socket_item_t *returns, uint32_t *numReturns)
{
    ServerVisionCam *pSVCam = (ServerVisionCam *)arg;
    VisionCam *pCam = pSVCam->m_pCam;
    if (pCam && numParams == 1 &&
        params[0].numElements == 1 && params[0].sizeElement == sizeof(VisionCamImages_t))
    {
        pSVCam->m_img = *(VisionCamImages_t *)params[0].item;
        if (pSVCam->m_frameData != NULL)
        {
            free(pSVCam->m_frameData);
            pSVCam->m_numDataBytes = 0;
            pSVCam->m_numImageBytes = 0;
            pSVCam->m_frameData = NULL;
        }
        if (pSVCam->m_frameData == NULL)
        {
            // compute the image size
            DVP_PRINT(DVP_ZONE_ALWAYS, "ServerVisionCam: w:%u h:%u c:0x%08x #:%u\n", pSVCam->m_img.width, pSVCam->m_img.height, pSVCam->m_img.color, pSVCam->m_img.count);
            DVP_PRINT(DVP_ZONE_ALWAYS, "ServerVisionCam: Alloc: w:%u h:%u\n", pSVCam->m_allocWidth, pSVCam->m_allocHeight);
            DVP_Image_t image;
            VisionCamFrame cFrame;

            // create a dummy image
            DVP_Image_Init(&image, pSVCam->m_img.width, pSVCam->m_img.height, pSVCam->m_img.color);

            // clear out the frame and assign the image then ask how big it could possibly be
            cFrame.clear();
            cFrame.mFrameBuff = &image;

            pSVCam->m_numDataBytes = cFrame.maxSerialLength();
            pSVCam->m_frameData = (uint8_t *)calloc(1, pSVCam->m_numDataBytes);
            DVP_PRINT(DVP_ZONE_CAM, "ServerVisionCam: Allocated %u bytes for m_frameData %p packet [%p-%p)\n", pSVCam->m_numDataBytes, pSVCam->m_frameData, pSVCam->m_frameData, &pSVCam->m_frameData[pSVCam->m_numDataBytes]);
        }
        if (pSVCam->m_display_enabled)
        {
            pSVCam->m_display = DVP_Display_Create(pSVCam->m_img.width,
                                                   pSVCam->m_img.height,
                                                   pSVCam->m_allocWidth,
                                                   pSVCam->m_allocHeight,
                                                   DVP_DISPLAY_WIDTH,
                                                   DVP_DISPLAY_HEIGHT,
                                                   pSVCam->m_allocWidth,
                                                   pSVCam->m_allocHeight,
                                                   0, 0,
                                                   pSVCam->m_img.color,
                                                   0, pSVCam->m_img.count);
        }
        for (uint32_t i = 0; i < pSVCam->m_img.count; i++)
        {
            DVP_Image_Init(&pSVCam->m_pImages[i], pSVCam->m_allocWidth, pSVCam->m_allocHeight, pSVCam->m_img.color);
            pSVCam->m_pImages[i].width = pSVCam->m_img.width;
            pSVCam->m_pImages[i].height = pSVCam->m_img.height;

            if (pSVCam->m_display_enabled && pSVCam->m_display)
            {
                DVP_Display_Alloc(pSVCam->m_display, &pSVCam->m_pImages[i]);
                DVP_Image_Alloc(pSVCam->m_dvp, &pSVCam->m_pImages[i], (DVP_MemType_e)pSVCam->m_pImages[i].memType);
            }
            else
            {
#if defined(DVP_USE_TILER)
                DVP_Image_Alloc(pSVCam->m_dvp, &pSVCam->m_pImages[i], DVP_MTYPE_MPUNONCACHED_2DTILED);
#else
                DVP_Image_Alloc(pSVCam->m_dvp, &pSVCam->m_pImages[i], DVP_MTYPE_DEFAULT);
#endif
            }
        }
        pSVCam->m_status = pCam->useBuffers(pSVCam->m_pImages, pSVCam->m_img.count, (VisionCamPort_e)pSVCam->m_img.port);
        returns[0].type = RPC_SOCKET_TYPE_INT32;
        returns[0].numElements = 1;
        returns[0].sizeElement = sizeof(status_e);
        returns[0].item = &pSVCam->m_status; // must be static or class variable!
        *numReturns = 1;
        return true_e;
    }
    else
        return false_e;
}

bool_e ServerVisionCam::releaseBuffers(void *arg, rpc_socket_item_t *params, uint32_t numParams, rpc_socket_item_t *returns, uint32_t *numReturns)
{
    ServerVisionCam *pSVCam = (ServerVisionCam *)arg;
    VisionCam *pCam = pSVCam->m_pCam;
    params = params; // remove warning
    if (pCam && numParams == 0)
    {
        pCam->disablePreviewCbk(FrameCallback);
        pSVCam->m_status = pCam->releaseBuffers();
        socket_disconnect(pSVCam->m_data);
        socket_deinit(pSVCam->m_data);
        pSVCam->m_data = NULL;
        for (uint32_t i = 0; i < pSVCam->m_img.count; i++)
        {
            if (pSVCam->m_display_enabled)
            {
                DVP_Display_Free(pSVCam->m_display, &pSVCam->m_pImages[i]);
            }
            else
            {
                DVP_Image_Free(pSVCam->m_dvp,&pSVCam->m_pImages[i]);
                DVP_Image_Deinit(&pSVCam->m_pImages[i]);
            }
        }
        if (pSVCam->m_display_enabled)
        {
            DVP_Display_Destroy(&pSVCam->m_display);
            pSVCam->m_display_enabled = false_e;
        }
        free(pSVCam->m_frameData);
        pSVCam->m_frameData = NULL;
        pSVCam->m_numDataBytes = 0;
        pSVCam->m_numImageBytes = 0;
        returns[0].type = RPC_SOCKET_TYPE_INT32;
        returns[0].numElements = 1;
        returns[0].sizeElement = sizeof(status_e);
        returns[0].item = &pSVCam->m_status; // must be static or class variable!
        *numReturns = 1;
        return true_e;
    }
    else
        return false_e;
}

bool_e ServerVisionCam::flushBuffers(void *arg, rpc_socket_item_t *params, uint32_t numParams, rpc_socket_item_t *returns, uint32_t *numReturns)
{
    ServerVisionCam *pSVCam = (ServerVisionCam *)arg;
    VisionCam *pCam = pSVCam->m_pCam;
    params = params; // remove warning
    if (pCam && numParams == 0)
    {
        pSVCam->m_status = pCam->flushBuffers();
        returns[0].type = RPC_SOCKET_TYPE_INT32;
        returns[0].numElements = 1;
        returns[0].sizeElement = sizeof(status_e);
        returns[0].item = &pSVCam->m_status; // must be static or class variable!
        *numReturns = 1;
        return true_e;
    }
    else
        return false_e;
}

ServerVisionCam *ServerVisionCam::instantiate(char *hostname, VisionCam_e camType)
{
    ServerVisionCam *pSVCam = new ServerVisionCam();
    // @see VisionCamFunction_e This must match that order of functions.
    rpc_server_function_f functions[] = {
        ServerVisionCam::init,
        ServerVisionCam::deinit,
        ServerVisionCam::setParameter,
        ServerVisionCam::getParameter,
        ServerVisionCam::sendCommand,
        ServerVisionCam::useBuffers,
        ServerVisionCam::releaseBuffers,
        ServerVisionCam::flushBuffers,
    };

    // instantiate the camera but don't initialize
    pSVCam->m_pCam = VisionCamFactory(camType);

    // initlialize the server components
    pSVCam->m_ipaddr = socket_hostname(hostname);
    pSVCam->m_server = rpc_socket_server_init(hostname, VCAM_SOCKET_CMD_PORT, pSVCam, functions, dimof(functions));
    pSVCam->m_data = socket_init(pSVCam->m_ipaddr, VCAM_SOCKET_DATA_PORT, SOCKET_TCP, true_e);

    // start the server
    rpc_socket_server_start(pSVCam->m_server);

    return pSVCam;
}


bool_e ServerVisionCam::WaitForCompletion()
{
    // WAIT until it's over
    event_wait(&m_wait, EVENT_FOREVER);

    // Close up
    socket_disconnect(m_data);
    socket_deinit(m_data);
    rpc_socket_server_stop(m_server);
    rpc_socket_server_deinit(m_server);

    return true_e;
}

void ServerVisionCam::SignalCompletion()
{
    event_set(&m_wait);
}

