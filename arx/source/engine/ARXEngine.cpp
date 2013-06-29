/*
 *  Copyright (C) 2012 Texas Instruments, Inc.
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

#include <engine/ARXEngine.h>

#include <arx/ARXProperties.h>
#include <arx/ARXBufferTypes.h>
#include <buffer/FlatBuffer.h>
#include <buffer/FlatBufferMgr.h>
#include <buffer/ImageBuffer.h>
#include <buffer/ImageBufferMgr.h>
#include <arx_debug.h>

#include <dvp/dvp_api.h>

using namespace android;

namespace tiarx {

#define BREAK_LOOP_IFNULL(b, message) \
    if (b == NULL) {\
        ARX_PRINT(ARX_ZONE_ERROR, (message)); \
        status = NULL_POINTER;\
        break;\
    }

ARXEngine::ARXEngine() : VisionEngine()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);

    m_fps = 30;
    m_flicker = FLICKER_AUTO;
    m_sensorIndex = VCAM_SENSOR_PRIMARY;
    m_camtype = VISIONCAM_OMX;
    m_camera_rotation = 0;
    m_display_rotation = 0;
    m_capmode = VCAM_GESTURE_MODE;
    m_face_detect = true_e;
    m_face_detect_raw = true_e;
    m_display_enabled = false_e;
    m_imgdbg_enabled = false_e;
    m_focus_delay = 0; // set infinite after this period
    m_focusDepth = -1; // set = -1 for auto-focus

    sp<ImageBufferMgr> camOut = new ImageBufferMgr(
            BUFF_CAMOUT, 640, 480, FOURCC_NV12, DVP_MTYPE_GRALLOC_2DTILED,
            ImageBufferMgr::DEFAULT_COUNT, false, true);
    sp<ImageBufferMgr> secCamout = new ImageBufferMgr(
            BUFF_CAMOUT2, 320, 240, FOURCC_NV12, DVP_MTYPE_GRALLOC_2DTILED,
            ImageBufferMgr::DEFAULT_COUNT, false, true);

    camOut->setEnable(true);
    secCamout->setEnable(true);

    sp<FlatBufferMgr> fdMgr = new FlatBufferMgr(BUFF_FACEDETECT, sizeof(ARXFaceDetectInfo), DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED);
    sp<FlatBufferMgr> fdFiltMgr = new FlatBufferMgr(BUFF_FACEDETECT, sizeof(ARXFaceDetectInfo), DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED);

    mImgBuffMgrMap.add(BUFF_CAMOUT, camOut);
    mImgBuffMgrMap.add(BUFF_CAMOUT2, secCamout);
    mFlatBuffMgrMap.add(BUFF_FACEDETECT, fdMgr);
    mFlatBuffMgrMap.add(BUFF_FACEDETECT_FILTERED, fdFiltMgr);

    mMainMirror = VCAM_MIRROR_NONE;
    mSecMirror = VCAM_MIRROR_NONE;

    mCam2ALockDelay = 0;
}

ARXEngine::~ARXEngine()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    mImgBuffMgrMap.clear();
    mFlatBuffMgrMap.clear();
    mClient.clear();
}

arxstatus_t ARXEngine::GetBufferMgr(uint32_t bufID, android::sp<IFlatBufferMgr> *mgr)
{
    const sp<IFlatBufferMgr>& manager = mFlatBuffMgrMap.valueFor(bufID);
    if (manager == NULL) {
        return FAILED;
    }
    *mgr = manager;
    return NOERROR;
}

arxstatus_t ARXEngine::GetBufferMgr(uint32_t bufID, android::sp<IImageBufferMgr> *mgr)
{
    const sp<IImageBufferMgr>& manager = mImgBuffMgrMap.valueFor(bufID);
    if (manager == NULL) {
        return FAILED;
    }
    *mgr = manager;
    return NOERROR;
}

arxstatus_t ARXEngine::Setup()
{
    m_capFrames = 0;
    m_hDVP = DVP_KernelGraph_Init();
    if (!m_hDVP) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed initializing DVP!\n");
        return STATUS_FAILURE;
    }

    const sp<ImageBufferMgr>& previewMgr = mImgBuffMgrMap.valueFor(BUFF_CAMOUT);
    const sp<ImageBufferMgr>& compMgr = mImgBuffMgrMap.valueFor(BUFF_CAMOUT2);

    m_camIdx = m_dispIdx = 0;
    m_camMin = m_dispMin = 0;
    m_camMax = m_dispMax = m_camMin + previewMgr->getCount();
    m_vidMin = m_vidIdx = m_camMax;
    m_vidMax = m_vidMin + compMgr->getCount();
    uint32_t numImages = m_vidMax;

    if (!AllocateImageStructs(numImages)) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to allocate image structs!\n");
        return NO_MEMORY;
    }

    VisionCamPortDesc_t dual[] = {
        { VCAM_PORT_PREVIEW, previewMgr->width(), previewMgr->height(), previewMgr->format(), m_fps, (VisionCamRotation_e) m_camera_rotation, mMainMirror, m_camMin, m_camIdx, m_camMax, 0, 0 },
        { VCAM_PORT_VIDEO, compMgr->width(), compMgr->height(), compMgr->format(), m_fps, (VisionCamRotation_e) m_camera_rotation, mSecMirror, m_vidMin, m_vidIdx, m_vidMax, 0, 0 },
    };

    uint32_t numPorts = compMgr->enabled() ? 2 : 1;

    if(CameraInit(this, dual, numPorts, false_e, false_e) != STATUS_SUCCESS) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed initializing camera!\n");
        return FAILED;
    }

    ARX_PRINT(ARX_ZONE_ALWAYS, "Size requested by camera w:%u, h:%u!\n", dual[0].req_width, dual[0].req_height);
    //Configure sizes requested by camera
    previewMgr->setAllocSize(dual[0].req_width, dual[0].req_height);
    if (compMgr->enabled()) {
        compMgr->setAllocSize(dual[1].req_width, dual[1].req_height);
    }

    for (unsigned int i = 0; i < mImgBuffMgrMap.size(); i++) {
        const sp<ImageBufferMgr>& mgr = mImgBuffMgrMap.valueAt(i);
        arxstatus_t status = mgr->allocate(m_hDVP);
        if (status != NOERROR) {
            ARX_PRINT(ARX_ZONE_ERROR, "Failed to allocate mgr id:%d\n", mgr->id());
            return status;
        }
    }

    for (unsigned int i = 0; i < mFlatBuffMgrMap.size(); i++) {
        const sp<FlatBufferMgr>& mgr = mFlatBuffMgrMap.valueAt(i);
        arxstatus_t status = mgr->allocate(m_hDVP);
        if (status != NOERROR) {
            ARX_PRINT(ARX_ZONE_ERROR, "Failed to allocate mgr id:%d\n", mgr->id());
            return status;
        }
    }

    // initialize the camera buffers (which are also the display buffers)
    for (m_camIdx = m_camMin; m_camIdx < m_camMax; m_camIdx++) {
        ImageBuffer *buf = previewMgr->nextFree();
        if (buf != NULL) {
            buf->copyInfo(&m_images[m_camIdx]);
            DVP_PrintImage(ARX_ZONE_ENGINE, &m_images[m_camIdx]);
        } else {
            return UNALLOCATED_RESOURCES;
        }
    }

    if (compMgr->enabled()) {
        // initialize the camera buffer for video port
        for (m_vidIdx = m_vidMin; m_vidIdx < m_vidMax; m_vidIdx++) {
            ImageBuffer *buf = compMgr->nextFree();
            if (buf != NULL) {
                buf->copyInfo(&m_images[m_vidIdx]);
                DVP_PrintImage(ARX_ZONE_ENGINE, &m_images[m_vidIdx]);
            } else {
                return UNALLOCATED_RESOURCES;
            }
        }
    }

    m_vidIdx = m_vidMin;
    m_camIdx = m_camMin;

    //Send Buffers to camera
    if (SendCameraBuffers(dual, numPorts) != STATUS_SUCCESS) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed sending buffers to camera!\n");
        return FAILED;
    }

    previewMgr->setCam(m_pCam);
    if (compMgr->enabled()) {
        compMgr->setCam(m_pCam);
    }

    return NOERROR;
}

void ARXEngine::Teardown()
{
    CameraDeinit();
    Lock();
    for (unsigned int i = 0; i < mImgBuffMgrMap.size(); i++) {
        const sp<ImageBufferMgr>& mgr = mImgBuffMgrMap.valueAt(i);
        mgr->free();
    }
    for (unsigned int i = 0; i < mFlatBuffMgrMap.size(); i++) {
        const sp<FlatBufferMgr>& mgr = mFlatBuffMgrMap.valueAt(i);
        mgr->free();
    }
    if (m_pNodes) {
        FreeNodes();
    }
    if (m_graphs) {
        FreeSections(&m_graphs[0]);
        FreeGraphs();
    }

    FreeImageStructs();

    DVP_KernelGraph_Deinit(m_hDVP);
    m_hDVP = 0;
    Unlock();

    if (mClient != NULL) {
        mClient->onPropertyChanged(PROP_ENGINE_STATE, ENGINE_STATE_STOP);
    }
}

arxstatus_t ARXEngine::copyFaceDetect(FlatBuffer *buf, VisionCamFaceType *faces, uint32_t numFaces)
{
    ARXFaceDetectInfo *fdInfo = reinterpret_cast<ARXFaceDetectInfo *>(buf->data());
    fdInfo->numFaces = numFaces;
    ARXFace *arxFaces = fdInfo->faces;
    for (unsigned int i = 0; i < numFaces; i++) {
        arxFaces[i].score = faces[i].mScore;
        arxFaces[i].priority = faces[i].mPriority;
        arxFaces[i].left = faces[i].mFacesCoordinates.mLeft;
        arxFaces[i].top = faces[i].mFacesCoordinates.mTop;
        arxFaces[i].w = faces[i].mFacesCoordinates.mWidth;
        arxFaces[i].h = faces[i].mFacesCoordinates.mHeight;
        arxFaces[i].roll = faces[i].mOrientationRoll >> 16;
    }
    return NOERROR;
}

arxstatus_t ARXEngine::Process(ImageBuffer *, ImageBuffer *)
{
    return NOERROR;
}

status_e ARXEngine::DelayedCameraFocusSetting()
{
    return STATUS_SUCCESS;
}

arxstatus_t ARXEngine::DelayedCamera2ALock()
{
    return STATUS_SUCCESS;
}

status_e ARXEngine::Engine()
{
    arxstatus_t status = NOERROR;
    ARX_PRINT(ARX_ZONE_ENGINE, "+Engine()\n");
    Lock();
    status = Setup();
    if (status == NOERROR) {
        status = (StartCamera() == STATUS_SUCCESS) ? NOERROR : FAILED;
        if (status == NOERROR && mClient != NULL) {
            mClient->onPropertyChanged(PROP_ENGINE_STATE, ENGINE_STATE_START);
        }
    }
    Unlock();

    if (status != NOERROR) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed setting up engine! 0x%x\n", status);
        Teardown();
        return STATUS_FAILURE;
    }

    while (m_running && status == NOERROR) {
        ARX_PRINT(ARX_ZONE_ENGINE, "Engine Queue Read Loop is alive!\n");
        const sp<ImageBufferMgr>& previewMgr = mImgBuffMgrMap.valueFor(BUFF_CAMOUT);
        const sp<ImageBufferMgr>& videoMgr = mImgBuffMgrMap.valueFor(BUFF_CAMOUT2);
        const sp<FlatBufferMgr>& fdMgr = mFlatBuffMgrMap.valueFor(BUFF_FACEDETECT);
        const sp<FlatBufferMgr>& fdFiltMgr = mFlatBuffMgrMap.valueFor(BUFF_FACEDETECT_FILTERED);
        ImageBuffer *previewBuf = previewMgr->nextReady();
        BREAK_LOOP_IFNULL(previewBuf, "no preview buffer available!");

        ARX_PRINT(ARX_ZONE_ENGINE, "read buffer %u from preview Manager\n", previewBuf->index());
        VisionCamFrame *camFrame = previewBuf->getCamFrame();

        FlatBuffer *fdBuf;
        if (fdMgr->enabled()) {
            FlatBuffer *fdBuf = fdMgr->nextFree();
            BREAK_LOOP_IFNULL(fdBuf, "no facedetect buffer available!");
            ARX_PRINT(ARX_ZONE_ENGINE, "obtained free face detect buffer\n");
            fdBuf->setTimestamp(camFrame->mTimestamp);
            copyFaceDetect(fdBuf, camFrame->mFacesRaw, camFrame->mDetectedFacesNumRaw);
            fdBuf->ready();
        }

        if (fdFiltMgr->enabled()) {
            fdBuf = fdFiltMgr->nextFree();
            BREAK_LOOP_IFNULL(fdBuf, "no facedetect filtered buffer available!");
            fdBuf->setTimestamp(camFrame->mTimestamp);
            copyFaceDetect(fdBuf, camFrame->mFaces, camFrame->mDetectedFacesNum);
            fdBuf->ready();
        }

        ImageBuffer *videoBuf = NULL;
        if (videoMgr->enabled()) {
            videoBuf = videoMgr->nextReady();
            BREAK_LOOP_IFNULL(videoBuf, "no videoBuf buffer available!");
        }

        m_capFrames++;
        if (m_pCam && m_capFrames == m_focus_delay)
        {
            DelayedCameraFocusSetting();
        }

        if (m_pCam && m_capFrames == mCam2ALockDelay)
        {
            DelayedCamera2ALock();
        }

        status = Process(previewBuf, videoBuf);

        previewBuf->release();
        if (videoBuf != NULL) {
            videoBuf->release();
        }
    }

    m_running = false_e;

    if (status != NOERROR)
    {
        ARX_PRINT(ARX_ZONE_ERROR, "Engine Thread quitting prematurely due to error 0x%x\n", status);
    }
    ARX_PRINT(ARX_ZONE_ENGINE, "Engine Thread is shutting down!\n");
    Teardown(); // locks are inside
    ARX_PRINT(ARX_ZONE_API, "-Engine()\n");

    return status == NOERROR ? STATUS_SUCCESS : STATUS_FAILURE;
}

void ARXEngine::ReceiveImage(VisionCamFrame *camFrame)
{
    if (camFrame && m_running) {
        Lock();
        if (camFrame->mFrameSource == VCAM_PORT_PREVIEW) {
            static int64_t prevTime = 0;
            ARX_PRINT(ARX_ZONE_ENGINE, "preview rate:"FMT_INT64_T"\n", camFrame->mTimestamp-prevTime);
            prevTime = camFrame->mTimestamp;
            const sp<ImageBufferMgr>& mgr = mImgBuffMgrMap.valueFor(BUFF_CAMOUT);
            mgr->readyOnMatch(camFrame);
        } else if (camFrame->mFrameSource == VCAM_PORT_VIDEO) {
            const sp<ImageBufferMgr>& mgr = mImgBuffMgrMap.valueFor(BUFF_CAMOUT2);
            mgr->readyOnMatch(camFrame);
        }
        Unlock();
    }
}

void ARXEngine::Shutdown()
{
    VisionEngine::Shutdown();
    for (unsigned int i = 0; i < mImgBuffMgrMap.size(); i++) {
        const sp<ImageBufferMgr>& mgr = mImgBuffMgrMap.valueAt(i);
        mgr->unblock();
    }
    for (unsigned int i = 0; i < mFlatBuffMgrMap.size(); i++) {
        const sp<FlatBufferMgr>& mgr = mFlatBuffMgrMap.valueAt(i);
        mgr->unblock();
    }
}

arxstatus_t ARXEngine::SetProperty(uint32_t property, int32_t value)
{
    ARX_PRINT(ARX_ZONE_ENGINE, "Engine: received property %u=%d", property, value);
    switch (property) {
        case PROP_CAM_ID:
        {
            switch (value) {
                case VCAM_SENSOR_PRIMARY:
                    m_sensorIndex = VCAM_SENSOR_PRIMARY;
                    break;
                case VCAM_SENSOR_SECONDARY:
                    m_sensorIndex = VCAM_SENSOR_SECONDARY;
                    break;
                case VCAM_SENSOR_STEREO:
                    m_sensorIndex = VCAM_SENSOR_STEREO;
                    break;
                default:
                    return INVALID_VALUE;
            }
            break;
        }
        case PROP_CAM_FPS:
        {
            if (value > 0) {
                m_fps = value;
            }
            break;
        }
        case PROP_CAM_MAIN_MIRROR:
        {
            switch (value) {
                case CAM_MIRROR_NONE:
                    mMainMirror = VCAM_MIRROR_NONE;
                    break;
                case CAM_MIRROR_HORIZONTAL:
                    mMainMirror = VCAM_MIRROR_HORIZONTAL;
                    break;
                case CAM_MIRROR_VERTICAL:
                    mMainMirror = VCAM_MIRROR_VERTICAL;
                    break;
                case CAM_MIRROR_BOTH:
                    mMainMirror = VCAM_MIRROR_BOTH;
                    break;
                default:
                    return INVALID_VALUE;
            }
            break;
        }
        case PROP_CAM_SEC_MIRROR:
        {
            switch (value) {
                case CAM_MIRROR_NONE:
                    mSecMirror = VCAM_MIRROR_NONE;
                    break;
                case CAM_MIRROR_HORIZONTAL:
                    mSecMirror = VCAM_MIRROR_HORIZONTAL;
                    break;
                case CAM_MIRROR_VERTICAL:
                    mSecMirror = VCAM_MIRROR_VERTICAL;
                    break;
                case CAM_MIRROR_BOTH:
                    mSecMirror = VCAM_MIRROR_BOTH;
                    break;
                default:
                    return INVALID_VALUE;
            }
            break;
        }
        default:
            return INVALID_PROPERTY;
    }
    ARX_PRINT(ARX_ZONE_ENGINE, "Engine: accepted property %u=%d", property, value);
    return NOERROR;
}

arxstatus_t ARXEngine::GetProperty(uint32_t property, int32_t *value)
{
    ARX_PRINT(ARX_ZONE_ENGINE, "Engine: querying property %u", property);
    switch (property) {
        case PROP_CAM_ID:
        {
            *value = m_sensorIndex;
            break;
        }
        case PROP_CAM_FPS:
        {
            *value = m_fps;
            break;
        }
        case PROP_CAM_MAIN_MIRROR:
        {
            *value = mMainMirror;
            break;
        }
        case PROP_CAM_SEC_MIRROR:
        {
            *value = mSecMirror;
            break;
        }
        default:
            return INVALID_PROPERTY;
    }
    ARX_PRINT(ARX_ZONE_ENGINE, "Engine: queried property %u=%d", property, *value);
    return NOERROR;
}

}
