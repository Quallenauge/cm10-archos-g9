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

#include <DVPARXEngine.h>

#include <buffer/ImageBuffer.h>
#include <buffer/FlatBuffer.h>
#include <buffer/ImageBufferMgr.h>
#include <buffer/FlatBufferMgr.h>

#include <arx/ARXBufferTypes.h>
#include <arx/ARXStatus.h>
#include <arx/ARXProperties.h>
#include <arx_debug.h>

#if defined(DVP_USE_VLIB)
#include <vlib/dvp_kl_vlib.h>
#endif

namespace tiarx {

ARXEngine *ARXEngineFactory()
{
    return new DVPARXEngine();
}

DVPARXEngine::DVPARXEngine() : ARXEngine()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);

    m_focus = VCAM_FOCUS_CONTROL_HYPERFOCAL;

    sp<ImageBufferMgr> sobelMgr = new ImageBufferMgr(BUFF_SOBEL_3X3, 320, 240, FOURCC_Y800);
    mImgBuffMgrMap.add(BUFF_SOBEL_3X3, sobelMgr);

    sp<ImageBufferMgr> cannyGradX = new ImageBufferMgr(BUFF_CANNY2D_GRADIENT_X, 320, 240, FOURCC_Y16);
    sp<ImageBufferMgr> cannyGradY = new ImageBufferMgr(BUFF_CANNY2D_GRADIENT_Y, 320, 240, FOURCC_Y16);
    sp<ImageBufferMgr> cannyGradMag = new ImageBufferMgr(BUFF_CANNY2D_GRADIENT_MAG, 320, 240, FOURCC_Y16);
#if defined(DVP_USE_VLIB)
    sp<ImageBufferMgr> harrisScore = new ImageBufferMgr(BUFF_HARRIS_SCORE, 320, 240, FOURCC_Y16);
#endif

#if defined(ARX_USE_OMRONFD)
    sp<FlatBufferMgr> fpartMgr = new FlatBufferMgr(BUFF_FACEINFO, sizeof(ARXFacialPartsInfo), DVP_MTYPE_MPUCACHED_VIRTUAL_SHARED);
    mFlatBuffMgrMap.add(BUFF_FACEINFO, fpartMgr);
#endif

    mImgBuffMgrMap.add(BUFF_CANNY2D_GRADIENT_X, cannyGradX);
    mImgBuffMgrMap.add(BUFF_CANNY2D_GRADIENT_Y, cannyGradY);
    mImgBuffMgrMap.add(BUFF_CANNY2D_GRADIENT_MAG, cannyGradMag);
#if defined(DVP_USE_VLIB)
    mImgBuffMgrMap.add(BUFF_HARRIS_SCORE, harrisScore);
#endif
    mSobelBuf = NULL;
    mHarrisBuf = NULL;
    mGradXBuf = NULL;
    mGradYBuf = NULL;
    mGradMagBuf = NULL;
    mPreviewBuf = NULL;
    mVideoBuf = NULL;
    mHarris_k = 1310;
    mHarrisScratch = NULL;

    mGradSection = 0xFFFFFFFF;
    mSobelSection = 0xFFFFFFFF;
    mGradNumNodes = 0xFFFFFFFF;
}

DVPARXEngine::~DVPARXEngine()
{
    ARX_PRINT(ARX_ZONE_API, "%s\n", __FUNCTION__);
}

arxstatus_t DVPARXEngine::Setup()
{
    const sp<ImageBufferMgr>& camMgr = mImgBuffMgrMap.valueFor(BUFF_CAMOUT2);
    const sp<ImageBufferMgr>& sobelMgr = mImgBuffMgrMap.valueFor(BUFF_SOBEL_3X3);
    const sp<ImageBufferMgr>& cannyGradX = mImgBuffMgrMap.valueFor(BUFF_CANNY2D_GRADIENT_X);
    const sp<ImageBufferMgr>& cannyGradY = mImgBuffMgrMap.valueFor(BUFF_CANNY2D_GRADIENT_Y);
    const sp<ImageBufferMgr>& cannyGradMag = mImgBuffMgrMap.valueFor(BUFF_CANNY2D_GRADIENT_MAG);
#if defined(DVP_USE_VLIB)
    const sp<ImageBufferMgr>& harrisScore = mImgBuffMgrMap.valueFor(BUFF_HARRIS_SCORE);
    harrisScore->configure(camMgr.get());
    if (harrisScore->enabled()) {
        cannyGradX->setEnable(true);
        cannyGradY->setEnable(true);
        cannyGradMag->setEnable(true);
    }
#endif
    if (cannyGradX->enabled() || cannyGradY->enabled() || cannyGradMag->enabled()) {
        cannyGradX->setEnable(true);
        cannyGradY->setEnable(true);
        cannyGradMag->setEnable(true);
    }
    //All these depend on the secondary camera output
    sobelMgr->configure(camMgr.get());
    cannyGradX->configure(camMgr.get());
    cannyGradY->configure(camMgr.get());
    cannyGradMag->configure(camMgr.get());

#if defined(ARX_USE_OMRONFD)
    const sp<FlatBufferMgr>& fpartMgr = mFlatBuffMgrMap.valueFor(BUFF_FACEINFO);
    if (fpartMgr->enabled()) {
        camMgr->setEnable(true);
        fpartMgr->setCount(camMgr->getCount());
    }
    mFacePose.init(camMgr->width(), camMgr->height(), mSecMirror);
#endif

    arxstatus_t status = ARXEngine::Setup();
    if (status != NOERROR) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed setting up base engine! 0x%x", status);
        return status;
    }

    if (!AllocateGraphs(1)) {
        return NOMEMORY;
    }

    if (!AllocateNodes(4) || !AllocateSections(&m_graphs[0], 3)) {
        return NOMEMORY;
    }

#if defined(DVP_USE_VLIB)
    if (harrisScore->enabled()) {
        mHarrisScratch = reinterpret_cast<DVP_Buffer_t *>(calloc(1, sizeof(DVP_Buffer_t)));
        if (mHarrisScratch == NULL) {
            return NOMEMORY;
        }
        DVP_Buffer_Init(mHarrisScratch, 1, 192*harrisScore->width());
        if(!DVP_Buffer_Alloc(m_hDVP, mHarrisScratch, DVP_MTYPE_DEFAULT)) {
            return NOMEMORY;
        }
    }
#endif

    uint32_t section = 0;
    uint32_t node = 0;

   if (sobelMgr->enabled()) {
        m_pNodes[node].header.kernel = DVP_KN_SOBEL_3x3_8;
        m_graphs[0].sections[section].pNodes = &m_pNodes[node];
        m_graphs[0].sections[section].numNodes = 1;
        m_graphs[0].order[section] = 0;
        section++;
        node++;
        m_graphs[0].numSections = section;
        mSobelSection = 0;
    }
#if defined(DVP_USE_VLIB)
    if (cannyGradX->enabled() || cannyGradY->enabled() || cannyGradMag->enabled() || harrisScore->enabled()) {
#else
    if (cannyGradX->enabled() || cannyGradY->enabled() || cannyGradMag->enabled()) {
#endif
        ARX_PRINT(ARX_ZONE_ENGINE, "grad node:%d", node);
        m_pNodes[node].header.kernel = DVP_KN_CANNY_2D_GRADIENT;
        m_pNodes[node].header.affinity = DVP_CORE_DSP;
        m_graphs[0].sections[section].pNodes = &m_pNodes[node];
        m_graphs[0].sections[section].numNodes = 1;
        m_graphs[0].order[section] = 0;
        mGradSection = section;
        mGradNumNodes = 1;
        m_graphs[0].numSections = section+1;
        node++;
    }

#if defined(DVP_USE_VLIB)
    if (harrisScore->enabled()) {
        m_pNodes[node].header.kernel = DVP_KN_VLIB_HARRIS_SCORE_7x7;
        m_pNodes[node].header.affinity = DVP_CORE_DSP;
        DVP_HarrisDSP_t *harris = dvp_knode_to(&m_pNodes[node], DVP_HarrisDSP_t);
        harris->scratch = *mHarrisScratch;
        harris->k = (uint16_t)mHarris_k;
        mGradNumNodes = 2;
        m_graphs[0].sections[section].numNodes = 2;
    }
#endif

    return NOERROR;
}

arxstatus_t DVPARXEngine::Process(ImageBuffer *previewBuf, ImageBuffer *videoBuf)
{
    uint32_t node = 0;

    if (videoBuf == NULL) {
        return NULL_POINTER;
    }

    const sp<ImageBufferMgr>& sobelMgr = mImgBuffMgrMap.valueFor(BUFF_SOBEL_3X3);
    if (sobelMgr->enabled()) {
        ImageBuffer *sobelBuf = sobelMgr->nextFree();
        if (sobelBuf == NULL) {
            ARX_PRINT(ARX_ZONE_ERROR, "Could not obtain an output buffer for sobel kernel!\n");
            return NULL_POINTER;
        }
        DVP_Transform_t *t = dvp_knode_to(&m_pNodes[node], DVP_Transform_t);
        videoBuf->copyInfo(&t->input, FOURCC_Y800);
        sobelBuf->copyInfo(&t->output, FOURCC_Y800);
        sobelBuf->setTimestamp(videoBuf->timestamp());
        videoBuf->consume();
        mVideoBuf = videoBuf;
        mSobelBuf = sobelBuf;
        node++;
    }

    const sp<ImageBufferMgr>& cannyGradX = mImgBuffMgrMap.valueFor(BUFF_CANNY2D_GRADIENT_X);
    const sp<ImageBufferMgr>& cannyGradY = mImgBuffMgrMap.valueFor(BUFF_CANNY2D_GRADIENT_Y);
    const sp<ImageBufferMgr>& cannyGradMag = mImgBuffMgrMap.valueFor(BUFF_CANNY2D_GRADIENT_MAG);
    ImageBuffer *gradXBuf = NULL;
    ImageBuffer *gradYBuf = NULL;
#if defined(DVP_USE_VLIB)
    const sp<ImageBufferMgr>& harrisMgr = mImgBuffMgrMap.valueFor(BUFF_HARRIS_SCORE);
    if (cannyGradX->enabled() || cannyGradY->enabled() || cannyGradMag->enabled() || harrisMgr->enabled()) {
#else
    if (cannyGradX->enabled() || cannyGradY->enabled() || cannyGradMag->enabled()) {
#endif
        gradXBuf = cannyGradX->nextFree();
        gradYBuf = cannyGradY->nextFree();
        ImageBuffer *gradMagBuf = cannyGradMag->nextFree();
        if (gradXBuf == NULL || gradYBuf == NULL || gradMagBuf == NULL) {
            ARX_PRINT(ARX_ZONE_ERROR, "Could not obtain output buffers for canny 2D kernel!\n");
            return NULL_POINTER;
        }

        DVP_Canny2dGradient_t *canny = dvp_knode_to(&m_pNodes[node], DVP_Canny2dGradient_t);
        videoBuf->copyInfo(&canny->input, FOURCC_Y800);
        gradXBuf->copyInfo(&canny->outGradX);
        gradYBuf->copyInfo(&canny->outGradY);
        gradMagBuf->copyInfo(&canny->outMag);
        gradXBuf->setTimestamp(videoBuf->timestamp());
        gradYBuf->setTimestamp(videoBuf->timestamp());
        gradMagBuf->setTimestamp(videoBuf->timestamp());
        mGradXBuf = gradXBuf;
        mGradYBuf = gradYBuf;
        mGradMagBuf = gradMagBuf;
        videoBuf->consume();
        mVideoBuf = videoBuf;
        node++;
    }

#if defined(DVP_USE_VLIB)
    if (harrisMgr->enabled()) {
        ImageBuffer *harrisBuf = harrisMgr->nextFree();
        if (harrisBuf == NULL) {
            ARX_PRINT(ARX_ZONE_ERROR, "Could not obtain output buffers for harris score!\n");
            return NULL_POINTER;
        }

        DVP_HarrisDSP_t *harris = dvp_knode_to(&m_pNodes[node], DVP_HarrisDSP_t);
        harrisBuf->copyInfo(&harris->harrisScore);
        gradXBuf->copyInfo(&harris->inGradX);
        gradYBuf->copyInfo(&harris->inGradY);
        harrisBuf->setTimestamp(videoBuf->timestamp());
        mHarrisBuf = harrisBuf;
        node++;
    }
#endif

#if defined(ARX_USE_OMRONFD)

    const sp<FlatBufferMgr>& fpMgr = mFlatBuffMgrMap.valueFor(BUFF_FACEINFO);
    if (fpMgr->enabled()) {
        FlatBuffer *fpBuf = fpMgr->nextFree();
        if (fpBuf != NULL) {
            ARXFacialPartsInfo *fpInfo = reinterpret_cast<ARXFacialPartsInfo *>(fpBuf->data());
            arxstatus_t ret = mFacePose.process(previewBuf, videoBuf, fpInfo);
            if (ret != NOERROR) {
                ARX_PRINT(ARX_ZONE_ERROR, "Failed processing facial parts!");
                fpInfo->numFaces = 0;
            }
            fpBuf->setTimestamp(videoBuf->timestamp());
            fpBuf->ready();
        }
    }

#endif

    if (node > 0) {
        GraphExecute(&m_graphs[0]);
    }

    return NOERROR;
}

void DVPARXEngine::GraphSectionComplete(DVP_KernelGraph_t*, DVP_U32 sectionIndex, DVP_U32 numNodesExecuted)
{
    ARX_PRINT(ARX_ZONE_ENGINE, "DVP executed sectionIndex=%u, numNodesExecuted=%u!\n", sectionIndex, numNodesExecuted );
    if (sectionIndex == mSobelSection && numNodesExecuted == 1) {
        ARX_PRINT(ARX_ZONE_ENGINE, "DVP SOBEL finished!\n");
        mSobelBuf->ready();
        mVideoBuf->release();
    } else if (sectionIndex == mGradSection && numNodesExecuted == mGradNumNodes) {
        ARX_PRINT(ARX_ZONE_ENGINE, "DVP Canny2D and Harris finished!\n");
        if (mHarrisBuf)
            mHarrisBuf->ready();
        mGradXBuf->ready();
        mGradYBuf->ready();
        mGradMagBuf->ready();
        mVideoBuf->release();
    } else {
        ARX_PRINT(ARX_ZONE_ENGINE, "DVP executed sectionIndex=%u, numNodesExecuted=%u!\n", sectionIndex, numNodesExecuted );
    }
}

void DVPARXEngine::Teardown()
{
#if defined(DVP_USE_VLIB)
    Lock();
    DVP_Buffer_Free(m_hDVP, mHarrisScratch);
    free(mHarrisScratch);
    mHarrisScratch = NULL;
    Unlock();
#endif

#if defined(ARX_USE_OMRONFD)
    mFacePose.teardown();
#endif
    ARXEngine::Teardown();
}

arxstatus_t DVPARXEngine::SetProperty(uint32_t property, int32_t value)
{
    ARX_PRINT(ARX_ZONE_ENGINE, "TIAREngine: received property %u=%d", property, value);
#if defined(ARX_USE_OMRONFD)
    if (mFacePose.setProperty(property, value) != INVALID_PROPERTY) {
        return NOERROR;
    }
#endif
    switch (property) {
        default:
            return ARXEngine::SetProperty(property, value);
    }
    ARX_PRINT(ARX_ZONE_ENGINE, "TIAREngine: accepted property %u=%d", property, value);
    return NOERROR;
}

arxstatus_t DVPARXEngine::GetProperty(uint32_t property, int32_t *value)
{
    ARX_PRINT(ARX_ZONE_ENGINE, "TIAREngine: querying property %u", property);
#if defined(ARX_USE_OMRONFD)
    if (mFacePose.getProperty(property, value) != INVALID_PROPERTY) {
        return NOERROR;
    }
#endif
    switch (property) {
        default:
            return ARXEngine::GetProperty(property, value);
    }
    ARX_PRINT(ARX_ZONE_ENGINE, "TIAREngine: queried property %u=%d", property, *value);
    return NOERROR;
}

}
