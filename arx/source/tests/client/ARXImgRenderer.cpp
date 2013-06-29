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
#include <arx/ARXImageBufferMgr.h>

#include <DisplaySurface.h>
#include <ARXImgRenderer.h>
#include <DVPTestGraph.h>

namespace tiarx {

ARXImgRenderer::ARXImgRenderer()
{
    mSurface = NULL;
    mMgr = NULL;
    mGraph = NULL;
    mutex_init(&mLock);
    mWriteToDisk = false;
    memset(&mImgDbg, 0, sizeof(mImgDbg));
    memset(&mFileSaveImg, 0, sizeof(mFileSaveImg));    
}

ARXImgRenderer::~ARXImgRenderer()
{    
    if (mWriteToDisk) {
        // finish writing to our output file
        ImageDebug_Close(&mImgDbg, 1);
    }
    
    mutex_lock(&mLock);    
    delete mSurface;
    mSurface = NULL;
    mutex_unlock(&mLock);
    mutex_deinit(&mLock);
}

arxstatus_t ARXImgRenderer::init(uint32_t w, uint32_t h, uint32_t left, uint32_t top,
                                uint32_t bufId, ARAccelerator *arx, bool localRender, 
                                bool writeToDisk)
{
    arxstatus_t ret = NOERROR;
       
    mSurface = new DisplaySurface(w, h);
    if (mSurface == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed allocating display surface for id:%d !\n", bufId);
        return NOMEMORY;
    }

    if ( (ret = mSurface->init(localRender)) != NOERROR ) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed initializing display surface for id:%d !\n", bufId);
        return ret;
    }

    mSurface->setPosition(left, top);

    ARXImageBufferMgr *mgr = arx->getImageBufferMgr(bufId);
    if (mgr == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed obtaining buffer manager id:%d !\n", bufId);
        return FAILED;
    }
    mgr->setSize(w, h);

    mWriteToDisk = writeToDisk;
    if (localRender || writeToDisk) {
        ret = mgr->registerClient(this);
        mMgr = mgr;
    } else {
        ret = mgr->bindSurface(mSurface->getSurface().get());
    }
    if (ret != NOERROR) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed registering or binding surface for id:%d!\n", bufId);
    }        
    
    if (writeToDisk) {
        mFileSaveImg.width = w;
        mFileSaveImg.height = h;
        mFileSaveImg.color = FOURCC_Y800;
        mFileSaveImg.planes = 1;

        time_t seconds;
        seconds = time(NULL);
        char filename[24];
        sprintf(filename, "arx_%ld", (long)seconds);
        ImageDebug_Init(&mImgDbg, &mFileSaveImg, "/sdcard/raw/", filename);
        ImageDebug_Open(&mImgDbg, 1);        
    }
        
    return ret;
}

void ARXImgRenderer::onBufferChanged(ARXImageBuffer *pImage)
{
    ARX_PRINT(ARX_ZONE_TEST, "ImageListener %s: id:%d - [image:%p, pData:%p, w:%d, h:%d ts:"FMT_UINT64_T"]\n",
              __FUNCTION__,pImage->id(),pImage, pImage->data(0), pImage->w(), pImage->h(), pImage->timestamp());
    mutex_lock(&mLock);
    if (mSurface != NULL) {
        mSurface->renderLuma(pImage);
    }
    if (mGraph) {
        mGraph->runGraph(pImage);
    }
    if (mWriteToDisk && mImgDbg.debug != NULL) {
        ARX_PRINT(ARX_ZONE_ALWAYS, "Writing next frame");
        // update the image to be written to disk
        mFileSaveImg.x_stride = 1;
        mFileSaveImg.y_stride = pImage->stride();
        mFileSaveImg.pData[0] = (DVP_U08*)pImage->data(0);
        //mFileSaveImg.pData[1] = (DVP_U08*)pImage->data(1);
        ImageDebug_Write(&mImgDbg, 1);
    }    
    pImage->release();
    mutex_unlock(&mLock);
}

}
