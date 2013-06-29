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

#ifndef _SOCKET_VCAM_H_
#define _SOCKET_VCAM_H_

#include <sosal/sosal.h>
#include <dvp/VisionCam.h>
#include <sosal/CThreaded.h>

// we can't have the socket cam without RPC sockets
#if defined(SOSAL_RPC_SOCKET_API)

#define VCAM_SUPPORTS_SOCKETCAM
#define VCAM_SOCKET_CMD_PORT    (8501)
#define VCAM_SOCKET_DATA_PORT   (8502)
#define VCAM_MAX_PACKET_SIZE    (1400)
#define VCAM_HEADER_MAGIC       ('G' >> 0 | 'C' >> 8 | 'A' >> 16 | 'M' >> 24)

typedef enum _vcam_function_e  {
    VISIONCAM_FUNCTION_INIT,
    VISIONCAM_FUNCTION_DEINIT,
    VISIONCAM_FUNCTION_SETPARAMS,
    VISIONCAM_FUNCTION_GETPARAMS,
    VISIONCAM_FUNCTION_SENDCMD,
    VISIONCAM_FUNCTION_USEBUFS,
    VISIONCAM_FUNCTION_RLSBUFS,
    VISIONCAM_FUNCTION_FLUSHBUFS,
    VISIONCAM_FUNCTION_MAX,
} VisionCamFunction_e;

typedef enum _socket_vcam_param_ext_t {
    SOCKETVCAM_PARAM_MIN = VCAM_PARAM_MAX,
    SOCKETVCAM_PARAM_HOSTNAME,
} SocketVisionCam_Param_e;

typedef struct _vcam_image_t {
    uint32_t width;
    uint32_t height;
    uint32_t color;
    uint32_t count;
    uint32_t port;
} VisionCamImages_t;

/**
 * This class sit on top of a socket and commands a remote VisionCam.
 */
class SocketVisionCam : public CThreaded, public VisionCam {
protected:
    queue_t  *m_imgs;   /**< A queue of DVP Images */
    socket_t *m_data;   /**< Socket to VisionCam server for frame data */
    uint32_t  m_ipaddr;   /**< The binary host address */
    uint8_t  *m_frameData;
    uint32_t  m_numDataBytes;
    uint32_t  m_numImgBytes;
    rpc_client_t *m_client;
    char m_hostname[MAX_PATH];
    char m_filename[MAX_PATH];
    FILE *pFile;
public:
    SocketVisionCam();
    virtual ~SocketVisionCam();

    // VisionCam interface
    status_e init(void *cookie);
    status_e deinit();
    status_e useBuffers(DVP_Image_t *pImages, uint32_t numImages, VisionCamPort_e port );
    status_e releaseBuffers(VisionCamPort_e port);
    status_e flushBuffers(VisionCamPort_e port);
    status_e sendCommand(VisionCamCmd_e cmdId, void *param = NULL, uint32_t size = 0, VisionCamPort_e port = VCAM_PORT_ALL);
    status_e setParameter(VisionCamParam_e paramId, void* param = NULL, uint32_t size = 0, VisionCamPort_e port = VCAM_PORT_ALL);
    status_e getParameter(VisionCamParam_e paramId, void* param = NULL, uint32_t size = 0, VisionCamPort_e port = VCAM_PORT_ALL);
    status_e returnFrame(VisionCamFrame *cameraFrame);

    // CThreaded
    thread_ret_t RunThread();
    void Shutdown();

};

#endif // SOSAL_RPC_SOCKET_API

#endif // _SOCKET_VCAM_H_

