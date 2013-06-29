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

#include <sosal/event.h>
#include <arx/ARXStatus.h>

#include <dvp/dvp.h>

namespace tiarx {

class ARXImageBuffer;
class ARAccelerator;
class DisplaySurface;

/*!
 * Simple dvp test graph to test sharing dvp handle and importing image
 * buffers from ARX. After the graph is successfully executed, the resulting
 * output is rendering into an internally allocated surface
 */
class DVPTestGraph
{
public:
    DVPTestGraph(uint32_t w, uint32_t h);
    ~DVPTestGraph();
    arxstatus_t init(uint32_t left, uint32_t top, ARAccelerator *arx);
    arxstatus_t runGraph(ARXImageBuffer *pImage);

private:
    uint32_t mNumNodes;
    uint32_t mNumSections;

    DVP_Handle mDvp;
    DVP_KernelGraph_t *mGraph;
    DVP_KernelGraphSection_t *mSections;
    DVP_U32 *mOrder;
    DVP_KernelNode_t *mNodes;
    DVP_Image_t mImage;

    bool mAllocated;
    DisplaySurface *mSurf;
    mutex_t mLock;
};

}
