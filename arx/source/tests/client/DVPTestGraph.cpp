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

#include <arx_debug.h>
#include <arx/ARAccelerator.h>
#include <arx/ARXImageBuffer.h>

#include <DisplaySurface.h>
#include <DVPTestGraph.h>

namespace tiarx {

DVPTestGraph::DVPTestGraph(uint32_t w, uint32_t h)
{
    mSurf = NULL;
    DVP_Image_Init(&mImage, w, h, FOURCC_Y800);
    mDvp = 0;
    mutex_init(&mLock);
}

DVPTestGraph::~DVPTestGraph()
{
    mutex_lock(&mLock);
    if (mAllocated) {
        DVP_Image_Free(mDvp, &mImage);
    }
    DVP_KernelNode_Free(mDvp, mNodes, mNumNodes);

    free(mSections);
    free(mOrder);
    free(mGraph);
    delete mSurf;
    mSurf = NULL;
    mutex_unlock(&mLock);
    mutex_deinit(&mLock);
}

arxstatus_t DVPTestGraph::init(uint32_t left, uint32_t top, ARAccelerator *arx)
{
    mSurf = new DisplaySurface(mImage.width, mImage.height);
    if (mSurf == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed allocating surface!");
        return NOMEMORY;
    }

    arxstatus_t ret = NOERROR;
    if ( (ret = mSurf->init()) != NOERROR ) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed initializing display surface!");
        return ret;
    }

    mSurf->setPosition(left, top);

    mDvp = arx->getDVPHandle();
    if (mDvp == 0) {
        ARX_PRINT(ARX_ZONE_ERROR, "DVP handle is invalid!");
        return FAILED;
    }

    mNumNodes = 1;
    mNumSections = 1;
    mAllocated = false;

    mGraph = (DVP_KernelGraph_t *)calloc(1, sizeof(DVP_KernelGraph_t));
    if (mGraph == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed allocating graph structure!");
        return NOMEMORY;
    }

    mSections = (DVP_KernelGraphSection_t *)calloc(mNumSections, sizeof(DVP_KernelGraphSection_t));
    if (mSections == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed allocating graph sections!");
        return NOMEMORY;
    }

    mOrder = (DVP_U32 *)calloc(mNumSections, sizeof(DVP_U32));
    if (mSections == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed allocating section order array!");
        return NOMEMORY;
    }

    mNodes = DVP_KernelNode_Alloc(mDvp, mNumNodes);
    if (mNodes == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed allocating graph nodes!");
        return NOMEMORY;
    }

    mGraph->numSections = mNumSections;
    mGraph->sections = mSections;
    mGraph->order = mOrder;
    mOrder[0] = 0;
    mSections[0].numNodes = mNumNodes;
    mSections[0].pNodes = mNodes;
    mNodes[0].header.kernel = DVP_KN_SOBEL_3x3_8;

    if (!DVP_Image_Alloc(mDvp, &mImage, DVP_MTYPE_DEFAULT)) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed allocating graph output image!");
        return NOMEMORY;
    }

    DVP_Transform_t *t = dvp_knode_to(&mNodes[0], DVP_Transform_t);
    t->output = mImage;

    mAllocated = true;
    return NOERROR;
}

arxstatus_t DVPTestGraph::runGraph(ARXImageBuffer *pImage)
{
    SOSAL::AutoLock lock(&mLock);

    DVP_Transform_t *t = dvp_knode_to(&mNodes[0], DVP_Transform_t);
    pImage->copyInfo(&t->input);
    if (DVP_KernelGraph_Process(mDvp, mGraph, NULL, NULL) < 1) {
        ARX_PRINT(ARX_ZONE_ERROR, "Graph execution failed!");
        return FAILED;
    } else {
        mSurf->renderLuma(&t->output);
    }

    return NOERROR;
}

}
