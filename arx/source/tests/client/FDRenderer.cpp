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
#include <arx/ARXBufferTypes.h>
#include <arx/ARXFlatBuffer.h>
#include <arx/ARXFlatBufferMgr.h>
#include <arx/ARXImageBufferMgr.h>

#include <DisplaySurface.h>
#include <FDRenderer.h>

namespace tiarx {

//=============================================================================
FDRenderer::FDRenderer(bool synchronize)
{
    mSync = synchronize;
    mSurface = NULL;
    mutex_init(&mLock);
    mFile = NULL;
}

FDRenderer::~FDRenderer()
{
    mutex_lock(&mLock);
    if (mFile != NULL) {
        fclose(mFile);
        mFile = NULL; // just for cleanliness' sake
    }
    delete mSurface;    
    mSurface = NULL;
    mutex_unlock(&mLock);
    mutex_deinit(&mLock);    
}

arxstatus_t FDRenderer::init(uint32_t w, uint32_t h, uint32_t left, uint32_t top,
                            ARAccelerator *arx, uint32_t bufID, bool_e writeToDisk)
{
    arxstatus_t ret = NOERROR;
    mArx = arx;
    mSurface = new DisplaySurface(w, h, android::PIXEL_FORMAT_RGBA_8888, 1, 200000);
    if (mSurface == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed allocating display surface for face detect overlay!\n");
        return NOMEMORY;
    }

    if ( (ret = mSurface->init()) != NOERROR ) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed initializing display surface for face detect overlay!\n");
        return ret;
    }

    mSurface->setPosition(left, top);

    ARXFlatBufferMgr *mgr = arx->getFlatBufferMgr(bufID);
    if (mgr == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed obtaining face detect manager!\n");
        return FAILED;
    }

    if (mSync) {
        ARXImageBufferMgr *camMgr = arx->getImageBufferMgr(BUFF_CAMOUT);
        if (camMgr == NULL) {
            ARX_PRINT(ARX_ZONE_ERROR, "Failed obtaining camera buffer manager!\n");
            return FAILED;
        }
        ret = camMgr->hold(true);
        if (ret != NOERROR) {
            ARX_PRINT(ARX_ZONE_ERROR, "Failed configuring camera buffer manager!\n");
            return ret;
        }
    }

    ret = mgr->registerClient(this);
    if (ret != NOERROR) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed registering with face detect manager!\n");
    }
    
    mWriteToDisk = writeToDisk;
    if (writeToDisk) {
        if (mFile != NULL) {
            fclose(mFile);
            mFile = NULL;
        }
        time_t seconds;
        seconds = time(NULL);
        char filename[64];
        sprintf(filename, "/sdcard/raw/faceparts_%ld.dat", (long)seconds);
        mFile = fopen(filename, "wb");
        if (mFile == NULL) {
            ARX_PRINT(ARX_ZONE_ERROR, "Failed to open face parts output file!\n");
            return FAILED;
        }
    }

    mMgr = mgr;
    return ret;
}

void FDRenderer::onBufferChanged(ARXFlatBuffer *pBuffer)
{
    mutex_lock(&mLock);
    if (pBuffer->id() == BUFF_FACEDETECT) {
        ARX_PRINT(ARX_ZONE_ALWAYS, "FaceDetectListener %s: id:%d - [buffer:%p, pData:%p, ts:"FMT_UINT64_T"]\n",
                  __FUNCTION__, pBuffer->id(), pBuffer, pBuffer->data(), pBuffer->timestamp());
        ARXFaceDetectInfo *pFD = reinterpret_cast<ARXFaceDetectInfo *>(pBuffer->data());
        ARX_PRINT(ARX_ZONE_TEST, "--numFaces:%u ", pFD->numFaces);
        for (unsigned int i = 0; i < pFD->numFaces; i++) {
            ARX_PRINT(ARX_ZONE_TEST, "--face %u score:%u, priority:%u roll:%d [l:%u, t:%u, w:%u, h:%u]",i,
                    pFD->faces[i].score,
                    pFD->faces[i].priority,
                    pFD->faces[i].roll,
                    pFD->faces[i].left,
                    pFD->faces[i].top,
                    pFD->faces[i].w,
                    pFD->faces[i].h);
        }

        mSurface->renderFaces(pFD);

    } else if (pBuffer->id() == BUFF_FACEINFO) {
        ARX_PRINT(ARX_ZONE_ALWAYS, "FaceInfoListener %s: id:%d - [buffer:%p, pData:%p, ts:"FMT_UINT64_T"]\n",
                  __FUNCTION__, pBuffer->id(), pBuffer, pBuffer->data(), pBuffer->timestamp());
        ARXFacialPartsInfo *pFP = reinterpret_cast<ARXFacialPartsInfo *>(pBuffer->data());
        ARX_PRINT(ARX_ZONE_TEST, "--numFaces:%u ", pFP->numFaces);

        // output to our face parts log
        if (mFile != NULL) {
            fwrite(&(pFP->numFaces), 1, sizeof(uint32_t), mFile);
        }

        for (unsigned int i = 0; i < pFP->numFaces; i++) {            
            ARX_PRINT(ARX_ZONE_TEST, "--face(%d) [yaw:%d pitch:%d roll:%d] [lefteyein:%d, %d] [lefteyeout:%d, %d] [righteyein:%d, %d] [righteyeout:%d, %d]",
                        i,
                        pFP->faces[i].yaw,
                        pFP->faces[i].pitch,
                        pFP->faces[i].roll,
                        pFP->faces[i].lefteyein.x,
                        pFP->faces[i].lefteyein.y,
                        pFP->faces[i].lefteyeout.x,
                        pFP->faces[i].lefteyeout.y,
                        pFP->faces[i].righteyein.x,
                        pFP->faces[i].righteyein.y,
                        pFP->faces[i].righteyeout.x,
                        pFP->faces[i].righteyeout.y);                

            if (mFile != NULL) {
                fwrite(&(pFP->faces[i]), 1, sizeof(ARXFacialParts), mFile);                
            }                        
        }

        mSurface->renderFaceInfo(pFP);
    }

    if (mSync && mArx) {
        ARXImageBufferMgr *bufMgr = mArx->getImageBufferMgr(BUFF_CAMOUT);
        bufMgr->render(pBuffer->timestamp());
    }
    pBuffer->release();
    mutex_unlock(&mLock);
}

}
