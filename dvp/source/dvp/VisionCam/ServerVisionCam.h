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

#include <sosal/sosal.h>
#include <dvp/dvp.h>
#include <dvp/dvp_display.h>
#include <SocketVisionCam.h>

class ServerVisionCam {
public:
    DVP_Handle        m_dvp;
    bool_e            m_running;
    rpc_server_t     *m_server;
    socket_t         *m_data;
    VisionCam        *m_pCam;
    uint8_t          *m_frameData;
    uint32_t          m_numDataBytes;
    uint32_t          m_numImageBytes;
    DVP_Image_t      *m_pImages;
    status_e          m_status;
    void             *m_param;
    uint32_t          m_ipaddr;
    bool_e            m_display_enabled;
    dvp_display_t    *m_display;
    uint32_t          m_allocWidth;
    uint32_t          m_allocHeight;
    VisionCamImages_t m_img;
    event_t           m_wait;
public:
    ServerVisionCam();
    ~ServerVisionCam();

    // Main Interface (blocking)
    static ServerVisionCam *instantiate(char *ipaddr, VisionCam_e camType);
    bool_e WaitForCompletion();
    void SignalCompletion();

    // Camera Interfaces
    static void FrameCallback(VisionCamFrame *frame);
    static void FocusCallback(int state);

    // RPC Interfaces
    static bool_e init(void *arg, rpc_socket_item_t *params, uint32_t numParams, rpc_socket_item_t *returns, uint32_t *numReturns);
    static bool_e deinit(void *arg, rpc_socket_item_t *params, uint32_t numParams, rpc_socket_item_t *returns, uint32_t *numReturns);
    static bool_e useBuffers(void *arg, rpc_socket_item_t *params, uint32_t numParams, rpc_socket_item_t *returns, uint32_t *numReturns);
    static bool_e releaseBuffers(void *arg, rpc_socket_item_t *params, uint32_t numParams, rpc_socket_item_t *returns, uint32_t *numReturns);
    static bool_e flushBuffers(void *arg, rpc_socket_item_t *params, uint32_t numParams, rpc_socket_item_t *returns, uint32_t *numReturns);
    static bool_e sendCommand(void *arg, rpc_socket_item_t *params, uint32_t numParams, rpc_socket_item_t *returns, uint32_t *numReturns);
    static bool_e setParameter(void *arg, rpc_socket_item_t *params, uint32_t numParams, rpc_socket_item_t *returns, uint32_t *numReturns);
    static bool_e getParameter(void *arg, rpc_socket_item_t *params, uint32_t numParams, rpc_socket_item_t *returns, uint32_t *numReturns);
};

