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

#include <CameraPoseEngine.h>

#include <buffer/ImageBuffer.h>
#include <buffer/ImageBufferMgr.h>
#include <buffer/FlatBuffer.h>
#include <buffer/FlatBufferMgr.h>

#include <arx/ARXBufferTypes.h>
#include <arx/ARXStatus.h>
#include <arx/ARXProperties.h>
#include <arx_debug.h>


#define ARX_12MP_CAMCALIBRATE 1


#if defined(DVP_USE_VLIB)
#include <vlib/dvp_kl_vlib.h>
#endif

#if defined(DVP_USE_IMGLIB)
#include <imglib/dvp_kl_imglib.h>
#endif

namespace tiarx {

ARXEngine *ARXEngineFactory()
{
    return new CameraPoseEngine();
}

CameraPoseEngine::CameraPoseEngine() : ARXEngine()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
    sp<FlatBufferMgr> mgr = new FlatBufferMgr(BUFF_CAMERA_POSE, sizeof(ARXCameraPoseMatrix));
    mFlatBuffMgrMap.add(BUFF_CAMERA_POSE, mgr);

    memset(&mARInfo, 0, sizeof(mARInfo));
    mARHandle = NULL;
    mUseCPUAlgo = true;
    mResetTracking = false;
    mEnableGausFilter = true;
    mInitialDelay = 35; //number of frames to wait until we start processing
    m_imgdbg_enabled = false_e;
    if (m_imgdbg_enabled) {
        strcpy(m_imgdbg_path, PATH_DELIM"sdcard"PATH_DELIM"raw"PATH_DELIM);
    }

    m_fps = 15;
    m_focus = VCAM_FOCUS_CONTROL_OFF;
    m_focusDepth = 30;
    mCam2ALockDelay = 30;
}

CameraPoseEngine::~CameraPoseEngine()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
}

arxstatus_t CameraPoseEngine::Setup()
{
    arxstatus_t status = ARXEngine::Setup();
    if (status != NOERROR) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed setting up base engine! 0x%x", status);
        return status;
    }

    const sp<ImageBufferMgr>& camMgr = mImgBuffMgrMap.valueFor(BUFF_CAMOUT2);

    uint32_t w = camMgr->width();
    uint32_t h = camMgr->height();
    uint32_t size = w*h;

#if ARX_12MP_CAMCALIBRATE
    //12MP Sony camera
    mARInfo.cameraCalParams[0] = 277.8069f;
    mARInfo.cameraCalParams[1] = 0.00f;
    mARInfo.cameraCalParams[2] = 161.273f;
    mARInfo.cameraCalParams[3] = 0.00f;
    mARInfo.cameraCalParams[4] = 278.9756f;
    mARInfo.cameraCalParams[5] = 119.5077f;
    mARInfo.cameraCalParams[6] = 0.00f;
    mARInfo.cameraCalParams[7] = 0.00f;
    mARInfo.cameraCalParams[8] = 1.0f;

#else
    //5MP OV5650 Camera
    mARInfo.cameraCalParams[0] = 345.4421041f;
    mARInfo.cameraCalParams[1] = 0.00f;
    mARInfo.cameraCalParams[2] = 158.63453f;
    mARInfo.cameraCalParams[3] = 0.00f;
    mARInfo.cameraCalParams[4] = 345.91371f;
    mARInfo.cameraCalParams[5] = 121.00159f;
    mARInfo.cameraCalParams[6] = 0.00f;
    mARInfo.cameraCalParams[7] = 0.00f;
    mARInfo.cameraCalParams[8] = 1.00f;
#endif


    mARHandle = AR_open();
    if (mARHandle == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed opening LIBARTI handle!");
        return NOMEMORY;
    }
    AR_initialize(mARHandle, camMgr->width(), camMgr->height(), mARInfo.cameraCalParams);

    mARImage.height = h;
    mARImage.width = w;
    mARImage.imageSize = size;
    mARImage.horzOffset = 0;
    mARImage.vertOffset = 0;
    mARImage.imageStride = w;
    mARImage.pixelDepth = AR_PIXEL_U08;
    mARImage.type = AR_IMG_LUMA;
    mARImage.imageCorners[0] = 0;
    mARImage.imageCorners[1] = 0;
    mARImage.imageCorners[2] = w;
    mARImage.imageCorners[3] = 0;
    mARImage.imageCorners[4] = w;
    mARImage.imageCorners[5] = h;
    mARImage.imageCorners[6] = 0;
    mARImage.imageCorners[7] = h;

    mARImage.imageData = calloc(1, size);
    if (mARImage.imageData == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to allocate image data");
        return NOMEMORY;
    }

    m_procIdx = 0;

    // Gradient X image
    DVP_Image_Init(&mImages[0], w, h, FOURCC_Y16);
    // Gradient Y image
    DVP_Image_Init(&mImages[1], w, h, FOURCC_Y16);
    // Gradient Mag image
    DVP_Image_Init(&mImages[2], w, h, FOURCC_Y16);
    // Harris Score image
    DVP_Image_Init(&mImages[3], w, h, FOURCC_Y32);

    //Output of convolution
    DVP_Image_Init(&mImages[4], w, h, FOURCC_Y800);
    // Convolution Gaussian kernel 7x7 data
    DVP_Image_Init(&mImages[5], 7, 7, FOURCC_Y16);

    // Scratch buffer for harris
    DVP_Buffer_Init(&mBuffers[0], 1, 96*w);
    // Scratch buffer for non-max
    DVP_Buffer_Init(&mBuffers[1], 4, 2*(w*h + 15));
    // Pixel index buffer for non-max
    DVP_Buffer_Init(&mBuffers[2], 2, (w*h));
    // Scratch for convolution
    DVP_Buffer_Init(&mBuffers[3], 4, w);

    for (uint32_t i = 0; i < dimof(mImages); i++)
    {
        if (!DVP_Image_Alloc(m_hDVP, &mImages[i], DVP_MTYPE_DEFAULT)) {
            ARX_PRINT(ARX_ZONE_ERROR, "Failed to allocate dvp images");
            return NOMEMORY;
        }
    }

    for (uint32_t i = 0; i < dimof(mBuffers); i++)
    {
        if(!DVP_Buffer_Alloc(m_hDVP, &mBuffers[i], DVP_MTYPE_DEFAULT)) {
            ARX_PRINT(ARX_ZONE_ERROR, "Failed to allocate dvp buffers");
            return NOMEMORY;
        }
    }

    if (!AllocateNodes(4)) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to allocate nodes");
        return NOMEMORY;
    }

    if (!AllocateGraphs(1)) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to allocate graph");
        return NOMEMORY;
    }

    if (!AllocateSections(&m_graphs[0], 1)) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to allocate sections");
        return NOMEMORY;
    }

    uint32_t nodeIdx = 0;

    mImages[0].skipCacheOpFlush = DVP_TRUE;
    mImages[0].skipCacheOpInval = DVP_TRUE;
    mImages[1].skipCacheOpFlush = DVP_TRUE;
    mImages[1].skipCacheOpInval = DVP_TRUE;
    mImages[2].skipCacheOpFlush = DVP_TRUE;
    mImages[2].skipCacheOpInval = DVP_TRUE;
    mImages[4].skipCacheOpFlush = DVP_TRUE;
    mImages[4].skipCacheOpInval = DVP_TRUE;

    int16_t gaussianFilter_7x7[49] = {
        370, 489, 577, 610, 577, 489, 370,
        489, 645, 762, 806, 762, 645, 489,
        577, 762, 900, 952, 900, 762, 577,
        610, 806, 952, 1006, 952, 806, 610,
        577, 762, 900, 952, 900, 762, 577,
        489, 645, 762, 806, 762, 645, 489,
        370, 489, 577, 610, 577, 489, 370
    };
    DVP_Image_Fill(&mImages[5], (DVP_S08 *)gaussianFilter_7x7, sizeof(gaussianFilter_7x7));

#if defined(DVP_USE_IMGLIB)
    if (mEnableGausFilter) {
        m_pNodes[nodeIdx].header.kernel = DVP_KN_IMG_CONV_7x7_I8_C16;
    } else {
        m_pNodes[nodeIdx].header.kernel = DVP_KN_NOOP;
    }
    DVP_ImageConvolution_with_buffer_t *pConv = dvp_knode_to(&m_pNodes[nodeIdx], DVP_ImageConvolution_with_buffer_t);
    pConv->output = mImages[4];
    pConv->mask = mImages[5];
    pConv->shiftMask = 15;
    pConv->scratch = mBuffers[3];
    m_pNodes[nodeIdx].header.affinity = DVP_CORE_DSP;
    nodeIdx++;
#else
    m_pNodes[nodeIdx].header.kernel = DVP_KN_NOOP;
    nodeIdx++;
#endif

    m_pNodes[nodeIdx].header.kernel = DVP_KN_VLIB_CANNY_2D_GRADIENT;
    DVP_Canny2dGradient_t* p2DGrad = dvp_knode_to(&m_pNodes[nodeIdx], DVP_Canny2dGradient_t);
    p2DGrad->input = mImages[4];
    p2DGrad->outGradX = mImages[0];
    p2DGrad->outGradY = mImages[1];
    p2DGrad->outMag = mImages[2];
    m_pNodes[nodeIdx].header.affinity = DVP_CORE_DSP;
    nodeIdx++;

    m_pNodes[nodeIdx].header.kernel = DVP_KN_VLIB_HARRIS_SCORE_7x7_U32;
    DVP_HarrisDSP_t* pHarris = dvp_knode_to(&m_pNodes[nodeIdx], DVP_HarrisDSP_t);
    pHarris->inGradX = mImages[0];
    pHarris->inGradY = mImages[1];
    pHarris->k = 1310; // 0.04
    pHarris->harrisScore = mImages[3];
    pHarris->scratch = mBuffers[0];
    m_pNodes[nodeIdx].header.affinity = DVP_CORE_DSP;
    nodeIdx++;

    m_pNodes[nodeIdx].header.kernel = DVP_KN_VLIB_NONMAXSUPPRESS_U32;
    DVP_Nonmax_NxN_t* pNonMax = dvp_knode_to(&m_pNodes[nodeIdx], DVP_Nonmax_NxN_t);
    pNonMax->input = mImages[3];
    pNonMax->p = 5;
    pNonMax->threshold = (unsigned int)(2 * 1024);
    pNonMax->scratch = mBuffers[1];
    pNonMax->pixIndex = mBuffers[2];
    m_pNodes[nodeIdx].header.affinity = DVP_CORE_DSP;
    nodeIdx++;

    m_graphs[0].sections[0].pNodes = &m_pNodes[0];
    m_graphs[0].sections[0].numNodes = m_numNodes;
    m_graphs[0].order[0] = 0;

    DVP_Perf_Clear(&mPerf);

    if (m_imgdbg_enabled && AllocateImageDebug(5))
    {
        ImageDebug_Init(&m_imgdbg[0], &mImages[0], m_imgdbg_path, "gradX");
        ImageDebug_Init(&m_imgdbg[1], &mImages[1], m_imgdbg_path, "gradY");
        ImageDebug_Init(&m_imgdbg[2], &mImages[3], m_imgdbg_path, "harrisScore");
        BufferDebug_Init(&m_imgdbg[3], &mBuffers[2], m_imgdbg_path, "nonmaxsuppr32", ".bw");

        sp<ImageBuffer> buf;
        DVP_Image_t camImage;
        camMgr->getImage(0, &buf);
        buf->copyInfo(&camImage, FOURCC_Y800);
        ImageDebug_Init(&m_imgdbg[4], &camImage, m_imgdbg_path, "camera");
        ImageDebug_Open(m_imgdbg, m_numImgDbg);
    }
    return NOERROR;
}

arxstatus_t CameraPoseEngine::Process(ImageBuffer *, ImageBuffer *videoBuf)
{
    const sp<FlatBufferMgr>& mgr = mFlatBuffMgrMap.valueFor(BUFF_CAMERA_POSE);
    FlatBuffer *buf = mgr->nextFree();
    ARXCameraPoseMatrix *m = reinterpret_cast<ARXCameraPoseMatrix *>(buf->data());

    if (m_capFrames > mInitialDelay) {
        DVP_Image_t im;
        videoBuf->copyInfo(&im);
        uint8_t *pDst = reinterpret_cast<uint8_t *>(mARImage.imageData);
        uint8_t *pSrc = im.pData[0];
        for(uint32_t i = 0; i < im.height; i++) {
           memcpy((void *)pDst, (void *)pSrc, im.width);
           pDst += im.width;
           pSrc += im.y_stride;
        }
        if (mResetTracking) {
            if (m_imgdbg_enabled) {
                ImageDebug_Close(m_imgdbg, m_numImgDbg);
                ImageDebug_Open(m_imgdbg, m_numImgDbg);
            }
            AR_initialize(mARHandle, im.width, im.height, mARInfo.cameraCalParams);
            mResetTracking = false;
        }
        if (mUseCPUAlgo) {
            DVP_PerformanceStart(&mPerf);
            m->status = AR_process(mARHandle, mARImage, NULL, NULL, 0, mARInfo.projectionMatrix, m->matrix, mARInfo.cameraCenter);
            DVP_PerformanceStop(&mPerf);
            DVP_PerformancePrint(&mPerf, "AR_proccess exec");
        } else {
            if (!mEnableGausFilter) {
                DVP_Canny2dGradient_t *t = dvp_knode_to(&m_pNodes[1], DVP_Canny2dGradient_t);
                videoBuf->copyInfo(&t->input, FOURCC_Y800);
            }
#if defined(DVP_USE_IMGLIB)
            else {

                DVP_ImageConvolution_with_buffer_t *t = dvp_knode_to(&m_pNodes[0], DVP_ImageConvolution_with_buffer_t);
                videoBuf->copyInfo(&t->input, FOURCC_Y800);
            }
#endif
            if (GraphExecute(&m_graphs[0])) {
                DVP_PrintPerformanceGraph(m_hDVP, &m_graphs[0]);
                int numFeatures = *((short *)mBuffers[2].pData);
                DVP_PerformanceStart(&mPerf);
                m->status = AR_process(mARHandle, mARImage,
                          (unsigned int *)mImages[3].pData[0],
                          (short *)(&mBuffers[2].pData[4]),
                          numFeatures,
                          mARInfo.projectionMatrix,
                          m->matrix,
                          mARInfo.cameraCenter);
                DVP_PerformanceStop(&mPerf);
                DVP_PerformancePrint(&mPerf, "AR_proccess exec");
            }
        }
        if (m_imgdbg_enabled) {
            DVP_Image_t camImage;
            videoBuf->copyInfo(&camImage);
            camImage.color = FOURCC_Y800;
            camImage.planes = 1;
            camImage.numBytes = camImage.width*camImage.height;
            m_imgdbg[4].pImg = &camImage;
            ImageDebug_Write(m_imgdbg, m_numImgDbg);
        }
    }
    buf->setTimestamp(videoBuf->timestamp());
    buf->ready();
    return NOERROR;
}

arxstatus_t CameraPoseEngine::DelayedCamera2ALock()
{
    status_e status = STATUS_SUCCESS;
    DVP_BOOL lockState = DVP_TRUE;

    status = m_pCam->sendCommand(VCAM_CMD_LOCK_AWB,&lockState, sizeof(DVP_BOOL));
    status = m_pCam->sendCommand(VCAM_CMD_LOCK_AE,&lockState, sizeof(DVP_BOOL));
    return NOERROR;
}

void CameraPoseEngine::Teardown()
{
    Lock();
    free(mARImage.imageData);
    mARImage.imageData = NULL;

    if (mARHandle) {
        AR_close(mARHandle);
        mARHandle = NULL;
    }

    for (uint32_t i = 0; i < dimof(mImages); i++)
    {
        if (!DVP_Image_Free(m_hDVP, &mImages[i])) {
            ARX_PRINT(ARX_ZONE_WARNING, "Failed to free dvp image %u", i);
        }
    }

    for (uint32_t i = 0; i < dimof(mBuffers); i++)
    {
        if(!DVP_Buffer_Free(m_hDVP, &mBuffers[i])) {
            ARX_PRINT(ARX_ZONE_WARNING, "Failed to allocate dvp buffers");
        }
    }

    if (m_imgdbg_enabled) {
        ImageDebug_Close(m_imgdbg, m_numImgDbg);
    }
    Unlock();
    ARXEngine::Teardown();
}

arxstatus_t CameraPoseEngine::SetProperty(uint32_t property, int32_t value)
{
    ARX_PRINT(ARX_ZONE_ENGINE, "CameraPoseEngine: received property %u=%d", property, value);
    switch (property) {
        case PROP_ENGINE_CAMPOSE_RESET:
            mResetTracking = true;
            break;
        default:
            return ARXEngine::SetProperty(property, value);
    }
    ARX_PRINT(ARX_ZONE_ENGINE, "CameraPoseEngine: accepted property %u=%d", property, value);
    return NOERROR;
}

arxstatus_t CameraPoseEngine::GetProperty(uint32_t property, int32_t *value)
{
    ARX_PRINT(ARX_ZONE_ENGINE, "CameraPoseEngine: querying property %u", property);
    switch (property) {
        default:
            return ARXEngine::GetProperty(property, value);
    }
    ARX_PRINT(ARX_ZONE_ENGINE, "CameraPoseEngine: queried property %u=%d", property, *value);
    return NOERROR;
}

}
