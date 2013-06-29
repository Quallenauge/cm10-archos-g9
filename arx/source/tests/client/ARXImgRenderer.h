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

#include <arx/ARXStatus.h>
#include <arx/ARXBufferListener.h>
#include <sosal/mutex.h>
#include <dvp/ImageDebugging.h>

namespace tiarx {

class ARAccelerator;
class ARXImageBufferMgr;
class DisplaySurface;
class DVPTestGraph;

/*!
 * This class can be registered with ARX as an image buffer stream listener.
 * Upon receiving a new image buffer, it will be rendered to an internally allocated
 * surface. This can either be done locally (which uses cpu to copy pixels into the surface)
 * or by binding a surface with the image buffer stream (zero copy rendering).
 */
class ARXImgRenderer : public ARXImageBufferListener
{
public:
    ARXImgRenderer();
    ~ARXImgRenderer();
    arxstatus_t init(uint32_t w, uint32_t h, uint32_t left, uint32_t top, uint32_t bufId,
                     ARAccelerator *arx, bool localRender = true, bool writeToDisk = false);
    void onBufferChanged(ARXImageBuffer *pImage);
    void setDVPGraph(DVPTestGraph *pGraph)
    {
        SOSAL::AutoLock lock(&mLock);
        mGraph = pGraph;
    }

private:
    DisplaySurface *mSurface;
    ARXImageBufferMgr *mMgr;
    mutex_t mLock;
    DVPTestGraph *mGraph;
    
    bool mWriteToDisk;
    ImageDebug_t mImgDbg;
    DVP_Image_t mFileSaveImg;    
};


}
