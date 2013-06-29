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

#ifndef _FACEPOSE_H
#define _FACEPOSE_H

#include <arx/ARXStatus.h>
#include <arx/ARXBufferTypes.h>
#include <dvp/dvp_types.h>
#include <dvp/VisionCam.h>

#include <omronfd/omronfd.h>

namespace tiarx {

class ImageBuffer;

class FacePose
{
public:
    /** Constructor */
    FacePose();
    /** Deconstructor */
    ~FacePose();

    arxstatus_t init(uint32_t w, uint32_t h, VisionCamMirrorType mirroring);
    void teardown();
    arxstatus_t process(ImageBuffer *previewBuf, ImageBuffer *buf, ARXFacialPartsInfo *out);
    arxstatus_t setProperty(uint32_t property, int32_t value);
    arxstatus_t getProperty(uint32_t property, int32_t *value);

private:
    bool didCoordMove(ARXCoords &oldCoord, POINT &newCoord) const;
    HDETECTION mHdlFaceDet;
    HDTRESULT mHdlFaceDetRes;
    HPOINTER mHdlFacePart;
    HPTRESULT mHdlFacePartRes;

    uint8_t *mTmpImage;
    void *mBackupMem;
    void *mWorkMem;

    bool mInitialized;

    DVP_Perf_t mPerf;
    DVP_Perf_t mCopyPerf;

    ARXFacialPartsInfo mCurFpInfo;
    bool mFilterFacePose;
    bool mThreshPassLE;
    bool mThreshPassRE;
    bool mThreshPassLN;
    bool mThreshPassRN;
    bool mThreshPassLM;
    bool mThreshPassRM;
    bool mThreshPassUM;
    int32_t mConfThresh;
    int32_t mMinMove;
    int32_t mMaxMove;
    bool mFDInit;

    VisionCamMirrorType mMirroring;
};


}
#endif

