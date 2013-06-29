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

#include <dvp/VisionEngine.h>

static void DVP_SectionComplete(void * cookie, DVP_KernelGraph_t * graph, DVP_U32 sectionIndex, DVP_U32 numNodesExecuted)
{
    VisionEngine *pEngine = reinterpret_cast<VisionEngine*>(cookie);
    pEngine->GraphSectionComplete(graph, sectionIndex, numNodesExecuted);
}

static void VisionCamCallback(VisionCamFrame *cameraFrame)
{
    VisionEngine *pEngine = reinterpret_cast<VisionEngine *>(cameraFrame->mCookie);
    DVP_PRINT(DVP_ZONE_ENGINE, "Received callback from Camera with frame %p (%p), port %d, width:%d,height:%d\n", cameraFrame, cameraFrame->mFrameBuff, cameraFrame->mFrameSource, cameraFrame->mWidth, cameraFrame->mHeight);
    if (pEngine != NULL) {
        pEngine->ReceiveImage(cameraFrame);
    } else {
        DVP_PRINT(DVP_ZONE_ERROR, "ERROR! NULL ENGINE!\n");
    }
}

VisionEngine::VisionEngine()
    : CThreaded()
{
    uint32_t width = 640;
    uint32_t height = 480;
    uint32_t fps = 30;
    fourcc_t color = FOURCC_UYVY;
    const char *name = "OMX";
    uint32_t numFrames = 0xFFFFFFFF;
    m_numPorts = 1;
    Constructor(width, height, fps, color, (char*)name, numFrames);
}

VisionEngine::VisionEngine(uint32_t width, uint32_t height, uint32_t fps, fourcc_t color, char *camName, uint32_t numFrames)
    : CThreaded()
{
    Constructor(width, height, fps, color, camName, numFrames);
}

void VisionEngine::Constructor(uint32_t width, uint32_t height, uint32_t fps, fourcc_t color, char *camName, uint32_t numFrames)
{
    DVP_PRINT(DVP_ZONE_API, "+VisionEngine()\n");
    //m_framequeue = queue_create(4, sizeof(DVP_Image_t *));
    m_framequeue = queue_create(8, sizeof(VisionCamFrame *));
    m_active = false_e;
    semaphore_create(&m_engineLock, 1, false_e);
    m_camIdx = m_dispIdx = 0;
    m_camMin = m_dispMin = 0;
    m_camMax = m_dispMax = 1;
    m_vidMin = m_vidMax = m_vidIdx = 0;
    m_whitemode = VCAM_WHITE_BAL_CONTROL_AUTO;
#if DVP_FLICKER == 60
    m_flicker = FLICKER_60Hz; // AMERICAS, ASIA, and others.
    DVP_PRINT(DVP_ZONE_ENGINE,"Antiflickering sets to 60 Hz\n");
#elif DVP_FLICKER == 50
    m_flicker = FLICKER_50Hz; // EUROPE, Russia, Israel, and others.
    DVP_PRINT(DVP_ZONE_ENGINE,"Antiflickering sets to 50 Hz\n");
#else
    m_flicker = FLICKER_AUTO; // AUTOMATIC
    DVP_PRINT(DVP_ZONE_ENGINE,"Antiflickering sets to AUTO\n");
#endif
    m_focus = /*VCAM_FOCUS_CONTRO_AUTO_MACRO*/VCAM_FOCUS_CONTROL_AUTO;
#if defined(DUCATI_1_5) || defined(DUCATI_2_0)
    m_capmode = VCAM_GESTURE_MODE;
#else
#if defined(__QNX__)
    m_capmode = VCAM_VIEWFINDER_MODE;
#else
    m_capmode = VCAM_VIDEO_NORMAL;
#endif
#endif
#if defined(SDP) || defined(PLAYBOOK) || defined(PANDA)
    m_sensorIndex = VCAM_SENSOR_PRIMARY;
#else // all other platforms
    m_sensorIndex = VCAM_SENSOR_SECONDARY;
#endif
    m_focusDepth = -1; // this is in steps of milimeters from 0 to 150.
    m_processing = false_e;
    m_camcallback = VisionCamCallback;
    m_capFrames = 0;
    m_numFrames = numFrames;
    m_pCam = NULL;
    memset(&m_graphs, 0, sizeof(DVP_KernelGraph_t));
    m_droppedFrames = 0;
    m_display_width = width;
    m_display_height = height;
    m_display_special = false_e;
    m_width = width;
    m_height = height;
    m_fourcc = color;
    m_fps = fps;
    strncpy(m_path, "raw", MAX_PATH); // initial value.
    strncpy(m_name, camName, MAX_PATH);
    if (strncmp(m_name, "OMX", MAX_PATH) == 0)
        m_camtype = VISIONCAM_OMX;
    else if (strncmp(m_name, "USB", MAX_PATH) == 0)
        m_camtype = VISIONCAM_USB;
    else if (strncmp(m_name, "SOCKET", MAX_PATH) == 0)
        m_camtype = VISIONCAM_SOCKET;
    else
        m_camtype = VISIONCAM_FILE;
    DVP_PRINT(DVP_ZONE_ENGINE, "Using Camera type %d with name %s\n", m_camtype, m_name);
    m_camera_rotation = 0;
    m_display_rotation = 0;
    m_display = NULL;
    m_hDVP = 0;
    m_pNodes = NULL;
    m_numNodes = 0;
    m_images = NULL;
    m_numImages = 0;
    m_imgdbg = NULL;
    m_numImgDbg = 0;
    m_buffers = NULL;
    m_numBuffers = 0;
    m_graphs = NULL;
    m_numGraphs = 0;
    m_display_enabled = false_e;
    m_imgdbg_enabled = false_e;
    m_face_detect = false_e;
    m_face_detect_raw = false_e;
    m_focus_delay = 0;
    strcpy(m_imgdbg_path, "raw"PATH_DELIM);
    DVP_Perf_Clear(&m_capPerf);
    event_init(&m_wait, true_e);
#if defined(ANDROID) && (defined(ICS) || defined(JELLYBEAN))
    m_anw = NULL;
    m_surface = NULL;
    m_window = NULL;
#endif
    m_stereo_info.layout = VCAM_STEREO_LAYOUT_TOPBOTTOM;
    m_stereo_info.subsampling = 1;
    DVP_PRINT(DVP_ZONE_API, "-VisionEngine()\n");
}

VisionEngine::~VisionEngine()
{
    StopThread();
    event_deinit(&m_wait);
    queue_destroy(m_framequeue);
    semaphore_delete(&m_engineLock);
}

thread_ret_t VisionEngine::RunThread()
{
    thread_ret_t err = 0;
    semaphore_wait(&m_engineLock);
    m_active = true_e;
    status_e status = Engine();
    m_active = false_e;
    semaphore_post(&m_engineLock);
    event_set(&m_wait);
    assign_thread_ret(err, status, status_e);
    thread_exit(err);
}

bool VisionEngine::WaitForCompletion(uint32_t timeout)
{
    if (event_wait(&m_wait, timeout) == true_e)
        return true;
    else
        return false;
}

bool VisionEngine::Startup()
{
    m_framequeue = queue_reset(m_framequeue);
    if (m_framequeue)
        return StartThread(this);
    else
        return false;
}

/** @note Do not call this directly, it is for the CThreaded class to use */
void VisionEngine::Shutdown()
{
    m_running = false_e;
    queue_pop(m_framequeue);
}

void VisionEngine::SetCoreCapacity(DVP_Core_e core, DVP_U32 limit)
{
    if (m_hDVP)
    {
        DVP_SetCoreCapacity(m_hDVP, core, limit);
    }
}

DVP_U32 VisionEngine::GetCoreCapacity(DVP_Core_e core)
{
    if (m_hDVP)
    {
        return DVP_GetCoreCapacity(m_hDVP, core);
    }
    return 0;
}

void VisionEngine::ReceiveImage(VisionCamFrame * cameraFrame)
{
    DVP_PRINT(DVP_ZONE_API, "+ReceiveImage()\n");
    DVP_PerformanceStop(&m_capPerf);
    DVP_PRINT(DVP_ZONE_PERF, "Camera avg frame delay is "FMT_RTIMER_T" us ("FMT_RTIMER_T" fps)\n",
        m_capPerf.avgTime, ((m_capPerf.avgTime != 0) ? (rtimer_freq()/m_capPerf.avgTime) : 0));

    if (cameraFrame && m_running)
    {
        if (queue_write(m_framequeue, false_e, &cameraFrame) == true_e)
        {
            DVP_PRINT(DVP_ZONE_ENGINE, "ENGINE: Put VisionCamFrame %p into Frame Queue\n", cameraFrame);
        }
        else
        {
            DVP_PRINT(DVP_ZONE_WARNING, "WARNING! Dropped Frame from Camera in ReceiveImage!\n");
            Lock();
            if (m_pCam)
                m_pCam->returnFrame(cameraFrame);
            if (m_numFrames > 0)
            {
                m_numFrames--;
                if (m_numFrames == 0)
                {
                    // wake up listeners and quit the engine
                    Shutdown();
                }
            }
            Unlock();
        }
    }
    else
    {
        if (m_framequeue)
        {
            Shutdown();
        }
    }
    DVP_PerformanceStart(&m_capPerf);
}

VisionCamFrame *VisionEngine::DequeueImage()
{
    VisionCamFrame *cameraFrame = NULL;
    while (queue_length(m_framequeue) > 1)
    {
        bool_e ret = queue_read(m_framequeue, true_e, &cameraFrame);
        DVP_Image_t *pPreviewImage = (DVP_Image_t *)cameraFrame->mFrameBuff;
        if (ret == true_e && pPreviewImage)
        {
            DVP_PRINT(DVP_ZONE_WARNING, "Dropping VisionCamFrame %p Frame:%u\n", cameraFrame, m_droppedFrames);
            m_droppedFrames++;
            DisplayDrop(pPreviewImage);
            if (m_pCam)
                m_pCam->returnFrame(cameraFrame);
        }
        else
        {
            // We could be trying to pop the thread off the queue
            DVP_PRINT(DVP_ZONE_ERROR, "ERROR: Queue Read Failed!\n");
            m_running = false;
            break;
        }
    }
    if (m_running == false)
        return NULL;
    cameraFrame = NULL;
    queue_read(m_framequeue, true_e, &cameraFrame);
    return cameraFrame;
}


status_e VisionEngine::Engine()
{
    VisionCamFrame *cameraFrame = NULL;
    status_e status = STATUS_SUCCESS;
    DVP_PRINT(DVP_ZONE_API, "+Engine()\n");
    Lock();
    status = GraphSetup();
    Unlock();
    if (status == STATUS_SUCCESS)
    {
        DVP_PRINT(DVP_ZONE_ENGINE, "Entering Queue Read Loop, this thread will block until data is ready (%s)\n",(m_running?"true":"false"));
        while (m_running && status == STATUS_SUCCESS)
        {
            DVP_PRINT(DVP_ZONE_ENGINE, "Engine Queue Read Loop is alive!\n");
            cameraFrame = DequeueImage();
            if (cameraFrame && cameraFrame->mLength)
            {
                DVP_PRINT(DVP_ZONE_ENGINE, "Read VisionCamFrame %p from Queue! (SRC:%d IMG:%p TIME:"FMT_INT64_T")\n", cameraFrame, cameraFrame->mFrameSource, cameraFrame->mFrameBuff, cameraFrame->mTimestamp);

                // only increment the frame count when the preview frame comes.
                if (cameraFrame->mFrameSource == VCAM_PORT_PREVIEW)
                    m_capFrames++;

                m_processing = true_e;
                status = ProcessImage(cameraFrame);
                m_processing = false_e;
                Lock();
                // only decrement the frame count when the preview frame comes.
                if (cameraFrame->mFrameSource == VCAM_PORT_PREVIEW && m_numFrames > 0)
                {
                    m_numFrames--;
                    if (m_numFrames == 0)
                    {
                        m_running = false_e;
                    }
                    DVP_PRINT(DVP_ZONE_ENGINE, "There are %u frames left.\n", m_numFrames);
                }
                Unlock();
                ReturnCameraFrame(cameraFrame);
            }
            else
            {
                DVP_PRINT(DVP_ZONE_ERROR, "Failed to read from Camera Queue!\n");
                m_running = false_e;
                if ((m_camtype == VISIONCAM_FILE) && cameraFrame && (cameraFrame->mLength == 0))
                {
                    status = STATUS_END_OF_FILE;
                }
                else
                {
                    status = STATUS_NO_RESOURCES;
                }
                break;
            }
        }
    }
    if (status != STATUS_SUCCESS)
    {
        DVP_PRINT(DVP_ZONE_ERROR, "Engine Thread quitting prematurely due to error %d\n", status);
    }
    DVP_PRINT(DVP_ZONE_ENGINE, "Engine Thread is shutting down!\n");
    GraphTeardown(); // locks are inside
    DVP_PRINT(DVP_ZONE_API, "-Engine()\n");
    return status;
}

status_e VisionEngine::CameraInit(void *cookie, fourcc_t color)
{
    VisionCamPortDesc_t single[] = {
       {VCAM_PORT_PREVIEW,
        m_width,
        m_height,
        color,
        m_fps,
        (VisionCamRotation_e)m_camera_rotation,
        VCAM_MIRROR_NONE,
        m_camMin,
        m_camIdx,
        m_camMax,
        0,
        0},
    };
    return CameraInit(cookie, single, dimof(single));
}

status_e VisionEngine::CameraInit(void *cookie, VisionCamPortDesc_t *desc, uint32_t numDesc, bool_e sendBuffers, bool_e startCam)
{
    status_e status = STATUS_SUCCESS;
    uint32_t d = 0;

    while (m_pCam == NULL)
    {
        DVP_PRINT(DVP_ZONE_ENGINE, "CameraInit: Camera Type %d", m_camtype);

        m_pCam = VisionCamFactory(m_camtype);
        if (m_pCam == NULL) {
            break;
        }

        VCAM_BREAK_IF_FAILED(status, m_pCam->init(cookie));
        VCAM_BREAK_IF_FAILED(status, m_pCam->enablePreviewCbk(m_camcallback));
        for (d = 0; d < numDesc; d++)
        {
            VCAM_BREAK_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_WIDTH, &desc[d].width, sizeof(uint32_t), desc[d].port));
            VCAM_BREAK_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_HEIGHT, &desc[d].height, sizeof(uint32_t), desc[d].port));
            VCAM_BREAK_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_COLOR_SPACE_FOURCC, &desc[d].color, sizeof(fourcc_t), desc[d].port));
            // Can't set ROTATION here, see below
            VCAM_BREAK_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_FPS_FIXED, &desc[d].fps, sizeof(uint32_t), desc[d].port));
        }
        if (m_camtype == VISIONCAM_FILE)
        {
            VCAM_BREAK_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_NAME, m_name, (int32_t)strlen(m_name)));
            VCAM_BREAK_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_PATH, m_path, (int32_t)strlen(m_path)));
        }

        VCAM_BREAK_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_SENSOR_SELECT, &m_sensorIndex, sizeof(m_sensorIndex)));
        VCAM_BREAK_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_CAP_MODE, &m_capmode, sizeof(m_capmode)));
        if (m_sensorIndex == VCAM_SENSOR_STEREO) {
            for (d = 0; d < numDesc; d++)
            {
                VCAM_BREAK_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_STEREO_INFO, &m_stereo_info, sizeof(m_stereo_info), desc[d].port));
            }
        }
        VCAM_BREAK_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_FLICKER,&m_flicker,sizeof(m_flicker)));

        if (m_camtype == VISIONCAM_OMX)
        {
            for (d = 0; d < numDesc; d++)
            {
                VisionCamResType res;
                VCAM_BREAK_IF_FAILED(status, m_pCam->getParameter(VCAM_PARAM_2DBUFFER_DIM, &res, sizeof(res), desc[d].port));
                desc[d].req_width = res.mWidth;
                desc[d].req_height = res.mHeight;
                if (sendBuffers && (res.mWidth != desc[d].width || res.mHeight != desc[d].height)) {
                    DVP_PRINT(DVP_ZONE_ERROR, "VisionCam expects w:%d and h:%d for port:%d!\n", res.mWidth, res.mHeight, desc[d].port);
                    status = STATUS_FAILURE;
                    break;
                }
            }
        }

        if (status == STATUS_SUCCESS && sendBuffers) {
            status = SendCameraBuffers(desc, numDesc);
            if (status == STATUS_SUCCESS && startCam) {
                status = StartCamera();
            }
        }
    }

    if (status != STATUS_SUCCESS) {
        DVP_PRINT(DVP_ZONE_ERROR, "VisionCam returned %d over all startup functions!\n", status);
    }

    return status;
}

status_e VisionEngine::SendCameraBuffers(VisionCamPortDesc_t *desc, uint32_t numDesc)
{
    status_e status = STATUS_SUCCESS;
    uint32_t d = 0;
    int ancillary=VCAM_ANCILLARY_DATA;
    int whiteBalanceAuto=VCAM_WHITE_BALANCE;
#if !defined(__QNX__)
    int whiteBalanceManual=VCAM_MANUAL_WHITE_BALANCE;
#endif

    VCAM_RETURN_IF_FAILED(status, (m_pCam == NULL) ? STATUS_INVALID_PARAMETER : STATUS_SUCCESS);
    for (d = 0; d < numDesc; d++)
    {
        VCAM_RETURN_IF_FAILED(status, m_pCam->useBuffers(&m_images[desc[d].minIdx], desc[d].maxIdx - desc[d].minIdx, desc[d].port));
    }
    // @TODO BUG: Must set rotation after useBuffers
    for (d = 0; d < numDesc; d++)
    {
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_ROTATION, &desc[d].rotate, sizeof(desc[d].rotate), desc[d].port));
        VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_MIRROR, &desc[d].mirror, sizeof(desc[d].mirror), desc[d].port));
    }
    if (m_face_detect == true_e)
    {
        int FaceEnable = 1;
        VCAM_RETURN_IF_FAILED(status, m_pCam->sendCommand(VCAM_CMD_FACE_DETECTION, &FaceEnable, sizeof(int)));
    }
    if (m_face_detect_raw == true_e)
    {
        int FaceEnable = 1;
        VCAM_RETURN_IF_FAILED(status, m_pCam->sendCommand(VCAM_CMD_FACE_DETECTION_RAW, &FaceEnable, sizeof(int)));
    }

    DVP_PerformanceStart(&m_capPerf); // we'll time the camera init to first capture

    if (m_imgdbg_enabled)
    {
        VCAM_RETURN_IF_FAILED(status, m_pCam->sendCommand(VCAM_EXTRA_DATA_START, &ancillary, sizeof(int)));
        VCAM_RETURN_IF_FAILED(status, m_pCam->sendCommand(VCAM_EXTRA_DATA_START, &whiteBalanceAuto, sizeof(int)));
#if !defined(__QNX__)
        VCAM_RETURN_IF_FAILED(status, m_pCam->sendCommand(VCAM_EXTRA_DATA_START, &whiteBalanceManual, sizeof(int)));
#endif
    }

    return status;
}

status_e VisionEngine::StartCamera()
{
    status_e status = STATUS_SUCCESS;
    VCAM_RETURN_IF_FAILED(status, (m_pCam == NULL) ? STATUS_INVALID_PARAMETER : STATUS_SUCCESS);

    // turn on all the ports!
    VCAM_RETURN_IF_FAILED(status, m_pCam->sendCommand(VCAM_CMD_PREVIEW_START, NULL, 0, VCAM_PORT_ALL));

    // CAMERA is now in execute state!
    if (m_focus_delay == 0)
    {
        if (m_focusDepth != -1) {
            VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_DO_MANUALFOCUS, &m_focusDepth, sizeof(m_focusDepth)));
        } else {
            //VisionCamFocusMode focus = VCAM_FOCUS_CONTROL_AUTO;
            VCAM_COMPLAIN_IF_FAILED(status, m_pCam->setParameter(VCAM_PARAM_DO_AUTOFOCUS, &m_focus, sizeof(m_focus)));
        }
    }
    return status;
}

status_e VisionEngine::CameraDeinit()
{
    status_e status = STATUS_SUCCESS;
    if (m_pCam)
    {
        // let any camera callbacks finish
        DVP_PRINT(DVP_ZONE_ENGINE, "Stopping Camera\n");
        VCAM_COMPLAIN_IF_FAILED(status,m_pCam->sendCommand(VCAM_CMD_PREVIEW_STOP, NULL, 0, VCAM_PORT_ALL));
        DVP_PRINT(DVP_ZONE_ENGINE, "Destroying Camera\n");
        VCAM_COMPLAIN_IF_FAILED(status,m_pCam->disablePreviewCbk(m_camcallback));
        VCAM_COMPLAIN_IF_FAILED(status,m_pCam->releaseBuffers(VCAM_PORT_ALL));
        VCAM_COMPLAIN_IF_FAILED(status,m_pCam->deinit());
        delete m_pCam;
        m_pCam = NULL;
        DVP_PRINT(DVP_ZONE_ENGINE, "Camera Destroyed\n");
    }
    return status;
}

DVP_U32 VisionEngine::GraphExecute(DVP_KernelGraph_t *graph)
{
    DVP_PRINT(DVP_ZONE_ENGINE, "Executing Kernel Graph %p\n", graph);
#if (DVP_DEBUG == 1)
    fflush(stdout);
#endif
    return DVP_KernelGraph_Process(m_hDVP, graph, this, DVP_SectionComplete);
}

status_e VisionEngine::DelayedCameraFocusSetting()
{
    status_e status = STATUS_SUCCESS;
    // default to preview port for Focus Controls
    if (m_focusDepth == -1) {
        //VisionCamFocusMode focus = VCAM_FOCUS_CONTROL_AUTO;
        m_pCam->setParameter(VCAM_PARAM_DO_AUTOFOCUS, &m_focus, sizeof(m_focus));
    } else {
        m_pCam->setParameter(VCAM_PARAM_DO_MANUALFOCUS, &m_focusDepth, sizeof(m_focusDepth));
    }
    return status;
}

status_e VisionEngine::ProcessImage(VisionCamFrame * cameraFrame)
{
    status_e status = STATUS_INVALID_STATE;
    DVP_U32 g, numSections = 0;

    if (m_running == false)
        return status;

    status = GraphUpdate(cameraFrame);
    if (status == STATUS_SUCCESS)
    {
        for (g = 0; g < m_numGraphs; g++)
        {
            if (m_correlation[g].portIdx == VCAM_PORT_ALL ||
                m_correlation[g].portIdx == cameraFrame->mFrameSource)
            {
                numSections += GraphExecute(&m_graphs[g]);
            }
        }
        if (m_imgdbg_enabled)
            ImageDebug_Write(m_imgdbg, m_numImgDbg);
    }

    status = PostProcessImage(cameraFrame, numSections);

    if (status == STATUS_SUCCESS && m_pCam && m_capFrames == m_focus_delay)
    {
        status = DelayedCameraFocusSetting();
    }

    status = RenderImage(cameraFrame);

    return status;
}

status_e VisionEngine::PostProcessImage(VisionCamFrame *cameraFrame, uint32_t numSections)
{
    cameraFrame = cameraFrame; // warnings
    numSections = numSections; // warnings
    DVP_PRINT(DVP_ZONE_ENGINE, "Post Processsing VisionCamFrame %p: %u sections completed in kernel graph\n", cameraFrame, numSections);
    return STATUS_SUCCESS;
}

status_e VisionEngine::ReturnCameraFrame(VisionCamFrame *cameraFrame)
{
    if (m_pCam)
        m_pCam->returnFrame(cameraFrame);
    return STATUS_SUCCESS;
}

status_e VisionEngine::RenderImage(VisionCamFrame *cameraFrame __attribute__((unused)))
{
    DisplayRender(&m_images[m_dispIdx]); // by definition, this is the display buffer
    return STATUS_SUCCESS;
}

status_e VisionEngine::GraphTeardown()
{
    uint32_t g = 0;
    status_e status = STATUS_SUCCESS;
    DVP_PRINT(DVP_ZONE_ENGINE, "DVP Graph Teardown\n");
    status = CameraDeinit();
    Lock();

    if (m_imgdbg_enabled)
        ImageDebug_Close(m_imgdbg, m_numImgDbg);

    for (g = 0; g < m_numGraphs; g++) {
        DVP_PrintPerformanceGraph(m_hDVP, &m_graphs[g]);
        FreeSections(&m_graphs[g]);
    }
    FreeGraphs();

    DVP_PRINT(DVP_ZONE_ENGINE, "Unmapping and Freeing Memory\n");

    FreeImageDebug();
    FreeNodes();
    FreeImageStructs(); // this will also free display buffers
    FreeBufferStructs();
    DisplayDestroy();

    DVP_PRINT(DVP_ZONE_ENGINE, "Deinitializing DVP\n");
    DVP_KernelGraph_Deinit(m_hDVP);
    m_hDVP = 0;

    Unlock();
    return status;
}

// SUPPORT APIs

bool VisionEngine::AllocateNodes(uint32_t numNodes)
{
    m_numNodes = numNodes;
    m_pNodes = DVP_KernelNode_Alloc(m_hDVP, numNodes);
    if (m_pNodes)
    {
        DVP_PRINT(DVP_ZONE_MEM, "Allocated %u Kernel Node structures in array %p\n", numNodes,m_pNodes);
        return true;
    }
    else
        return false;
}

void VisionEngine::FreeNodes()
{
    DVP_KernelNode_Free(m_hDVP, m_pNodes, m_numNodes);
    m_pNodes = NULL;
    m_numNodes = 0;
}

bool VisionEngine::AllocateSections(DVP_KernelGraph_t *graph, uint32_t numSections)
{
    graph->numSections = numSections;
    graph->sections = (DVP_KernelGraphSection_t *)calloc(numSections, sizeof(DVP_KernelGraphSection_t));
    graph->order    = (DVP_U32 *)calloc(numSections, sizeof(DVP_U32));
    if (graph->sections && graph->order)
    {
        for (DVP_U32 s = 0; s < numSections; s++)
        {
            DVP_Perf_t p = DVP_PERF_INIT;
            memcpy(&graph->sections[s].perf, &p, sizeof(p));
        }
        return true;
    }
    else
        return false;
}

void VisionEngine::FreeSections(DVP_KernelGraph_t *graph)
{
    free(graph->sections);
    free(graph->order);
    memset(graph, 0, sizeof(DVP_KernelGraph_t));
}

bool VisionEngine::AllocateGraphs(uint32_t numGraphs)
{
    m_numGraphs = numGraphs;
    m_graphs = (DVP_KernelGraph_t *)calloc(m_numGraphs, sizeof(DVP_KernelGraph_t));
    m_correlation = (KGraph_Image_Corr_t *)calloc(m_numGraphs, sizeof(KGraph_Image_Corr_t));
    if (m_graphs && m_correlation)
    {
        uint32_t g;
        for (g = 0; g < m_numGraphs; g++)
        {
            // initialize the performance information
            DVP_Perf_t p = DVP_PERF_INIT;
            memcpy(&m_graphs[g].totalperf, &p, sizeof(DVP_Perf_t));

            // initialize the correlation array
            m_correlation[g].portIdx = VCAM_PORT_ALL;
        }
        return true;
    }
    else
        return false;
}

void VisionEngine::FreeGraphs()
{
    free(m_graphs);
    free(m_correlation);
    m_graphs = NULL;
    m_correlation = NULL;
    m_numGraphs = 0;
}

bool VisionEngine::AllocateImageStructs(uint32_t numImages)
{
    m_numImages = numImages;
    m_images = (DVP_Image_t *)calloc(m_numImages,sizeof(DVP_Image_t));
    if (m_images)
    {
        DVP_PRINT(DVP_ZONE_MEM, "Allocated %u image structures in array %p\n", numImages,m_images);
        return true;
    }
    else
        return false;
}

void VisionEngine::FreeImageStructs()
{
    if (m_images)
    {
        for (DVP_U32 i = 0; i < m_numImages; i++)
        {
            if (m_dispMin <= i && i < m_dispMax && m_display_enabled)
                DisplayFree(&m_images[i]);
            else
                DVP_Image_Free(m_hDVP, &m_images[i]);
            DVP_Image_Deinit(&m_images[i]);
        }
        memset(m_images, 0xEF, m_numImages * sizeof(DVP_Image_t));
        free(m_images);

        m_images = NULL;
        m_numImages = 0;
    }
}

bool VisionEngine::AllocateImageDebug(uint32_t numImgDbg)
{
    m_numImgDbg = numImgDbg;
    m_imgdbg = (ImageDebug_t *)calloc(m_numImgDbg,sizeof(ImageDebug_t));
    if (m_imgdbg)
    {
        DVP_PRINT(DVP_ZONE_MEM, "Allocated %u image debugging structures in array %p\n", numImgDbg,m_imgdbg);
        return true;
    }
    else
        return false;
}

void VisionEngine::FreeImageDebug()
{
    if (m_imgdbg && m_imgdbg_enabled)
    {
        memset(m_imgdbg, 0xEF, m_numImgDbg * sizeof(ImageDebug_t));
        free(m_imgdbg);
        m_imgdbg = NULL;
        m_numImgDbg = 0;
    }
}

bool VisionEngine::AllocateBufferStructs(uint32_t numBuffers)
{
    m_numBuffers = numBuffers;
    m_buffers = (DVP_Buffer_t *)calloc(m_numBuffers,sizeof(DVP_Buffer_t));
    if (m_buffers)
    {
        DVP_PRINT(DVP_ZONE_MEM, "Allocated %u buffer structures in array %p\n", numBuffers,m_buffers);
        return true;
    }
    else
        return false;
}

void VisionEngine::FreeBufferStructs()
{
    DVP_U32 i = 0;
    if (m_buffers)
    {
        for (i = 0; i < m_numBuffers; i++)
        {
            DVP_Buffer_Free(m_hDVP, &m_buffers[i]);
            DVP_Buffer_Deinit(&m_buffers[i]);
        }
        if(m_numBuffers)
        {
            memset(m_buffers, 0xEF, m_numBuffers * sizeof(DVP_Buffer_t));
            free(m_buffers);
        }
        m_buffers = NULL;
        m_numBuffers = 0;
    }
}

bool VisionEngine::DisplayCreate(uint32_t image_width,  uint32_t image_height,
                                 uint32_t buffer_width, uint32_t buffer_height,
                                 uint32_t scaled_width, uint32_t scaled_height,
                                 uint32_t crop_top,     uint32_t crop_left)
{
    DVP_PRINT(DVP_ZONE_VIDEO, "Creating Display Surfaces!\n");
    if (m_display_special == false_e)
    {
        m_display = DVP_Display_Create(image_width, image_height,
                                       buffer_width, buffer_height,
                                       m_display_width, m_display_height,
                                       scaled_width, scaled_height,
                                       crop_top, crop_left,
                                       m_fourcc, m_display_rotation, m_dispMax-m_dispMin);
        if (m_display)
            return true;
        else
            return false;
    }
    else
    {
#if defined(ANDROID) && (defined(ICS) || defined(JELLYBEAN))
        m_anw = anativewindow_create(m_window);
        if (m_anw)
        {
            if (anativewindow_allocate(m_anw,
                                       buffer_width, buffer_height,
                                       m_dispMax-m_dispMin,
                                       ANW_NV12_FORMAT, true) == false_e)
            {
                anativewindow_destroy(&m_anw);
            }
            else
            {
                anativewindow_set_crop(m_anw, crop_left, crop_top, image_width, image_height);
                return true;
            }
        }
        return false;
#endif
    }
    return false;
}

bool VisionEngine::DisplayAllocate(DVP_Image_t *pImage)
{
    bool ret = false;
    if (m_display_special == false_e)
    {
        if (DVP_Display_Alloc(m_display, pImage) == DVP_TRUE)
            ret = true;
    }
    else
    {
#if defined(ANDROID) && (defined(ICS) || defined(JELLYBEAN))
        uint8_t *ptrs[3] = {NULL, NULL, NULL};
        int32_t stride = 0;
        if (anativewindow_acquire(m_anw, &pImage->reserved, ptrs, &stride) == true_e)
        {
            pImage->y_stride = stride;
            pImage->memType = DVP_MTYPE_DISPLAY_2DTILED;
#if defined(GRALLOC_USE_MULTIPLE_POINTERS)
            for (uint32_t p = 0; p < pImage->planes; p++)
            {
                pImage->pBuffer[p] = pImage->pData[p] = ptrs[p];
            }
#else
            if (pImage->color == FOURCC_NV12)
            {
                pImage->pBuffer[0] = pImage->pData[0] = ptrs[0];
                pImage->pBuffer[1] = pImage->pData[1] = &ptrs[0][pImage->y_stride*pImage->bufHeight];
            }
#endif
            ret = true;
        }
#endif
    }
    DVP_PrintImage(DVP_ZONE_ENGINE, pImage);
    return ret;
}

/** The convienence call wraps the DVP Display "C" API or uses the ICS special mechanisms */
void VisionEngine::DisplayFree(DVP_Image_t *pImage)
{
    if (m_display_special == false_e)
    {
        DVP_Display_Free(m_display, pImage);
    }
    else
    {
#if defined(ANDROID) && (defined(ICS) || defined(JELLYBEAN))
        anativewindow_release(m_anw, pImage->reserved);
#endif
    }
}

/** The convienence call wraps the DVP Display "C" API or uses the ICS special mechanisms */
void VisionEngine::DisplayDestroy()
{
    if (m_display_enabled)
    {
        DVP_PRINT(DVP_ZONE_VIDEO, "Deinitializing DVP Display!\n");
        if (m_images)
        {
            for (m_dispIdx = m_dispMin; m_dispIdx < m_dispMax; m_dispIdx++)
                DisplayFree(&m_images[m_dispIdx]);
            m_dispMin = 0;
            m_dispMax = 0;
        }
        if (m_display_special == false_e)
        {
            DVP_Display_Destroy(&m_display);
            m_display = NULL;
        }
        else
        {
#if defined(ANDROID) && (defined(ICS) || defined(JELLYBEAN))
            anativewindow_free(m_anw);
            anativewindow_destroy(&m_anw);
#endif
        }
    }
}

void VisionEngine::DisplayDrop(DVP_Image_t *pImage)
{
    if (pImage && m_display_enabled && m_display_special)
    {
#if defined(ANDROID) && (defined(ICS) || defined(JELLYBEAN))
        anativewindow_drop(m_anw, pImage->reserved);
        anativewindow_dequeue(m_anw, NULL);
#endif
    }
}

void VisionEngine::DisplayRender(DVP_Image_t *pImage)
{
    static DVP_Image_t *pLast = NULL;

    if (m_display_enabled == true_e)
    {
        if (pLast != pImage)
        {
            DVP_PRINT(DVP_ZONE_VIDEO, "Display Rendering Image %p\n", pImage);
            if (m_display_special == false_e)
            {
                DVP_Display_Render(m_display, pImage);
            }
            else
            {
#if defined(ANDROID) && (defined(ICS) || defined(JELLYBEAN))
                anativewindow_enqueue(m_anw, pImage->reserved);
                anativewindow_dequeue(m_anw, NULL);
#endif
            }
            pLast = pImage;
        }
        else
        {
            DVP_PRINT(DVP_ZONE_WARNING, "Attempted to redisplay last image %p!\n", pLast);
        }
    }
}

#if defined(ANDROID) && (defined(ICS) || defined(JELLYBEAN))
void VisionEngine::DisplayOnWindow(const sp<IBinder>& binder, const sp<ANativeWindow>& window)
{
    int ret = 0;
    Lock();
    // we already have this surface
    if (binder == m_surface) {
        DVP_PRINT(DVP_ZONE_WARNING, "Setting same window back on VisionEngine!\n");
        goto end;
    }

    // the previous run has to stop and a new run has to start if it's currently running now.
    ret = semaphore_trywait(&m_engineLock);
    if (ret == 0) // engine is active and locked.
    {
        DVP_PRINT(DVP_ZONE_WARNING, "*******************************\n");
        DVP_PRINT(DVP_ZONE_WARNING, "Restarting VisionEngine Thread!\n");
        DVP_PRINT(DVP_ZONE_WARNING, "*******************************\n");
        Unlock();
        if (StopThread() == false) {
            DVP_PRINT(DVP_ZONE_ERROR, "Thread died incorrectly!\n");
        }
        m_display_special = true_e;
        m_surface = NULL;
        m_window = NULL;
        queue_unpop(m_framequeue);
        DVP_PRINT(DVP_ZONE_ENGINE, "Using new ANativeWindow interface %p!\n", window.get());
        m_surface = binder;
        m_window = window;
        Startup();
        Lock();
        DVP_PRINT(DVP_ZONE_WARNING, "******************************\n");
        DVP_PRINT(DVP_ZONE_WARNING, "VisionEngine thread restarted!\n");
        DVP_PRINT(DVP_ZONE_WARNING, "******************************\n");
    }
    else if (ret == 1)
    {
        m_display_special = true_e;
        DVP_PRINT(DVP_ZONE_ENGINE, "Using new ANativeWindow interface %p!\n", window.get());
        m_surface = binder;
        m_window  = window;
        semaphore_post(&m_engineLock);
    }
    else
    {
        DVP_PRINT(DVP_ZONE_ERROR, "Failed to grab engine lock semaphore!\n");
    }
end:
    Unlock();
}

void VisionEngine::DisplayOnSurface(const sp<Surface>& surface)
{
    if (surface != 0) {
        DVP_PRINT(DVP_ZONE_ENGINE, "Using Surface\n");
        sp<IBinder> binder(surface->asBinder());
        sp<ANativeWindow> window(surface);
        return DisplayOnWindow(binder, window);
    }
}

void VisionEngine::DisplayOnTexture(const sp<ISurfaceTexture>& texture)
{
    if (texture != 0) {
        DVP_PRINT(DVP_ZONE_ENGINE, "Using SurfaceTextureClient\n");
        sp<IBinder> binder(texture->asBinder());
        sp<ANativeWindow> window = new SurfaceTextureClient(texture);
        return DisplayOnWindow(binder, window);
    }
}
#endif


