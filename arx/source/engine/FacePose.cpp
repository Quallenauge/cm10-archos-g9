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

#include <FacePose.h>

#include <buffer/ImageBuffer.h>
#include <arx/ARXStatus.h>
#include <arx/ARXProperties.h>
#include <arx_debug.h>

#include <dvp/dvp_api.h>
#include <dvp/VisionCam.h>

namespace tiarx {

FacePose::FacePose()
{
    mHdlFaceDet = NULL;
    mHdlFaceDetRes = NULL;
    mHdlFacePart = NULL;
    mHdlFacePartRes = NULL;
    mTmpImage = NULL;
    mBackupMem = NULL;
    mWorkMem = NULL;

    mInitialized = false;
    mMirroring = VCAM_MIRROR_NONE;

    mThreshPassLE = false_e;
    mThreshPassRE = false_e;
    mThreshPassLN = false_e;
    mThreshPassRN = false_e;
    mThreshPassLM = false_e;
    mThreshPassRM = false_e;
    mThreshPassUM = false_e;
    mConfThresh = 200;
    mFilterFacePose = false_e;
    mMinMove = 0;
    mMaxMove = 0xFFFF;
    mFDInit = false;
}

FacePose::~FacePose()
{
    teardown();
}

arxstatus_t FacePose::init(uint32_t w, uint32_t h, VisionCamMirrorType mirroring)
{
    if (mInitialized) {
        teardown();
        mInitialized = false;
    }

    mMirroring = mirroring;

    memset(&mCurFpInfo, 0, sizeof(ARXFacialPartsInfo));
    mFDInit = false;

    mTmpImage = (uint8_t *)malloc(w*h*sizeof(uint8_t));
    if (mTmpImage == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to allocate memory to copy input data");
        return NOMEMORY;
    }

#if defined(USE_OMRON_SWFD)
    mHdlFaceDet = OKAO_CreateDetection();
    mHdlFaceDetRes = OKAO_CreateDtResult(ARXFacialPartsInfo::MAX_FACES, 0);
#endif

    mHdlFacePart = OKAO_PT_CreateHandle();
    mHdlFacePartRes = OKAO_PT_CreateResultHandle();

#if defined(USE_OMRON_SWFD)
    if (mHdlFaceDet == NULL || mHdlFaceDetRes == NULL|| mHdlFacePart == NULL || mHdlFacePartRes == NULL) {
#else
    if (mHdlFacePart == NULL || mHdlFacePartRes == NULL) {
#endif
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to create library handles");
        teardown();
        return FAILED;
    }
    INT32 ret = OKAO_NORMAL;

#if defined(USE_OMRON_SWFD)
    RECT rcEdgeMask;
    rcEdgeMask.left = 0;
    rcEdgeMask.top = 0;
    rcEdgeMask.right = w - 1;
    rcEdgeMask.bottom = h - 1;
    UINT32 backupMemSize;
    UINT32 minWorkMemSize;
    UINT32 maxWorkMemSize;

    OKAO_GetDtRequiredStillMemSize(w, h, 40, ARXFacialPartsInfo::MAX_FACES, rcEdgeMask, 33, &backupMemSize, &minWorkMemSize, &maxWorkMemSize);

    /* Add required memory of Facial Part Detection (for some reason this is only documented on sample code) */
    backupMemSize += 1024+1024;
    maxWorkMemSize += 110*1024;

    mBackupMem = malloc(backupMemSize);
    mWorkMem = malloc(maxWorkMemSize);

    if (mBackupMem == NULL || mWorkMem == NULL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to allocate temp work memory!");
        teardown();
        return FAILED;
    }

    ret = OKAO_SetBMemoryArea(mBackupMem, backupMemSize);
    if (ret != OKAO_NORMAL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to setup backup memory!");
        teardown();
        return FAILED;
    }

    ret = OKAO_SetWMemoryArea(mWorkMem, maxWorkMemSize);
    if (ret != OKAO_NORMAL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed to setup backup memory!");
        teardown();
        return FAILED;
    }

    ret = OKAO_SetDtMode(mHdlFaceDet, DT_MODE_DEFAULT);
    if (ret != OKAO_NORMAL ) {
        ARX_PRINT(ARX_ZONE_WARNING, "Failed to setup face detect mode (%d)!", ret);
    }
#endif

    //These settings allow face detection at much greater angles of rotation
    //but computation time is increased
#if 0
    ret = OKAO_SetDtFaceSizeRange(mHdlFaceDet, 10, 240);
    if (ret != OKAO_NORMAL) {
        ARX_PRINT(ARX_ZONE_WARNING, "Failed to setup face detect range (%d)!", ret);
    }

    UINT32 anAngle[3];
    anAngle[POSE_FRONT] = ANGLE_ULR45;
    anAngle[POSE_HALF_PROFILE] = ANGLE_ULR45;
    anAngle[POSE_PROFILE] = ANGLE_ULR45;
    ret = OKAO_SetDtAngle(mHdlFaceDet, anAngle, ANGLE_ROTATION_EXT1 | ANGLE_POSE_EXT1);
    if (ret != OKAO_NORMAL ) {
        ARX_PRINT(ARX_ZONE_WARNING, "Failed to setup face detect angles (%d)!", ret);
    }
#endif

    ret = OKAO_PT_SetMode(mHdlFacePart, PT_MODE_DEFAULT);
    if (ret != OKAO_NORMAL) {
        ARX_PRINT(ARX_ZONE_WARNING, "Failed to setup face parts detection mode (%d)!", ret);
    }

    DVP_Perf_Clear(&mPerf);
    DVP_Perf_Clear(&mCopyPerf);

    mInitialized = true;
    return NOERROR;
}

void FacePose::teardown()
{
    if (mHdlFacePartRes != NULL) {
        OKAO_PT_DeleteResultHandle(mHdlFacePartRes);
    }
    if (mHdlFacePart != NULL) {
        OKAO_PT_DeleteHandle(mHdlFacePart);
    }
#if defined(USE_OMRON_SWFD)
    if (mHdlFaceDetRes != NULL) {
        OKAO_DeleteDtResult(mHdlFaceDetRes);
    }
    if (mHdlFaceDet != NULL) {
        OKAO_DeleteDetection(mHdlFaceDet);
    }

    free(mBackupMem);
    free(mWorkMem);
#endif
    free(mTmpImage);
    mHdlFaceDet = NULL;
    mHdlFaceDetRes = NULL;
    mHdlFacePart = NULL;
    mHdlFacePartRes = NULL;

    mTmpImage = NULL;
    mBackupMem = NULL;
    mWorkMem = NULL;
}

/*!
 * Extracts facial feature and orientation information from the given images.
 * @param previewBuf Image data from the main camera stream
 * @param buf Image data from the secondary camera stream
 * @param filter true if facial feature coordinates and orientation data should
 * be filtered to reduce jitter, false otherwise
 * @param out (output) Stores the resulting feature coordinates and orientation for
 * each face in the image
 */
arxstatus_t FacePose::process(ImageBuffer *previewBuf, ImageBuffer *buf, ARXFacialPartsInfo *out)
{
    if (!mInitialized) {
        return INVALID_STATE;
    }

    DVP_PerformanceStart(&mCopyPerf);
    DVP_Image_t im;

    buf->copyInfo(&im);

    uint8_t *pSrc = im.pData[0];
    uint8_t *pDst = mTmpImage;
    for (uint32_t i = 0; i < im.height; i++) {
       memcpy((void *)pDst, (void *)pSrc, im.width);
       pDst += im.width;
       pSrc += im.y_stride;
    }
    DVP_PerformanceStop(&mCopyPerf);
    DVP_PerformancePrint(&mCopyPerf, "FacePose input copy exec:");

    DVP_PerformanceStart(&mPerf);

    INT32 numFaces = 0;
    INT32 ret = OKAO_NORMAL;
#if defined(USE_OMRON_SWFD)
    ret = OKAO_Detection(mHdlFaceDet, mTmpImage, im.width, im.height, ACCURACY_NORMAL, mHdlFaceDetRes);
    if (ret != OKAO_NORMAL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Face detection failed!");
        return FAILED;
    }

    ret = OKAO_GetDtFaceCount(mHdlFaceDetRes, &numFaces);
    if (ret != OKAO_NORMAL) {
        ARX_PRINT(ARX_ZONE_ERROR, "Failed querying number of faces!");
        return FAILED;
    }
#else
    VisionCamFrame *camFrame = previewBuf->getCamFrame();
    numFaces = camFrame->mDetectedFacesNumRaw;
#endif

    out->numFaces = numFaces;
    bool leiChange = true;
    bool reiChange = true;
    bool leoChange = true;
    bool reoChange = true;
    bool lnChange = true;
    bool rnChange = true;
    bool lmChange = true;
    bool rmChange = true;
    bool cmChange = true;
    for (int i = 0; i < numFaces; i++) {
        mThreshPassLE = false_e;
        mThreshPassRE = false_e;
        mThreshPassLN = false_e;
        mThreshPassRN = false_e;
        mThreshPassLM = false_e;
        mThreshPassRM = false_e;
        mThreshPassUM = false_e;

#if defined(USE_OMRON_SWFD)
        ret = OKAO_PT_SetPositionFromHandle(mHdlFacePart, mHdlFaceDetRes, i);
        if (ret != OKAO_NORMAL) {
            ARX_PRINT(ARX_ZONE_ERROR, "Failed to set face #%d in facial parts detector! (%d)", i, ret);
            return FAILED;
        }
#else
        INT32 centerX = camFrame->mFacesRaw[i].mFacesCoordinates.mLeft + camFrame->mFacesRaw[i].mFacesCoordinates.mWidth/2;
        INT32 centerY = camFrame->mFacesRaw[i].mFacesCoordinates.mTop + camFrame->mFacesRaw[i].mFacesCoordinates.mHeight/2;
        INT32 faceSize = camFrame->mFacesRaw[i].mFacesCoordinates.mWidth;
        INT32 faceRoll = camFrame->mFacesRaw[i].mOrientationRoll >> 16;
        INT32 pose = camFrame->mFacesRaw[i].mOrientationYaw >> 16;
        if (i != 0) {
            faceRoll = 0;
            pose = 0;
        }

        DVP_Image_t previewIm;
        previewBuf->copyInfo(&previewIm);
        centerX = (centerX*im.width)/previewIm.width;
        centerY = (centerY*im.height)/previewIm.height;
        faceSize = (faceSize*im.width)/previewIm.width;

        if (mMirroring == VCAM_MIRROR_HORIZONTAL) {
            centerX = im.width - centerX;
        }

        if (faceRoll < 0) {
            faceRoll += 360;
        }

        ret = OKAO_PT_SetPositionIP(mHdlFacePart, centerX, centerY, faceSize, faceRoll, 1, pose, DTVERSION_IP_V1);
        if (ret != OKAO_NORMAL) {
            ARX_PRINT(ARX_ZONE_ERROR, "Failed to set face #%d in facial parts detector! (%d)", i, ret);
            return FAILED;
        }
#endif
        ret = OKAO_PT_DetectPoint(mHdlFacePart, mTmpImage, im.width, im.height, mHdlFacePartRes);
        if (ret != OKAO_NORMAL) {
            ARX_PRINT(ARX_ZONE_ERROR, "Failed running facial parts detection! (%d)", ret);
            return FAILED;
        }

        INT32 pitch, yaw, roll;
        ret = OKAO_PT_GetFaceDirection(mHdlFacePartRes, &pitch, &yaw, &roll);
        if (ret != OKAO_NORMAL) {
            ARX_PRINT(ARX_ZONE_ERROR, "Failed obtaining face position angles! (%d)", ret);
            return FAILED;
        }

        INT32 anConf[PT_POINT_KIND_MAX];
        POINT aptPoint[PT_POINT_KIND_MAX];
        ret = OKAO_PT_GetResult(mHdlFacePartRes, PT_POINT_KIND_MAX, aptPoint, anConf);
        if (ret != OKAO_NORMAL) {
            ARX_PRINT(ARX_ZONE_ERROR, "Failed obtaining face part coordinates! (%d)", ret);
            return FAILED;
        }

        if (mFilterFacePose && mFDInit) {
            leiChange = didCoordMove(mCurFpInfo.faces[i].lefteyein, aptPoint[PT_POINT_LEFT_EYE_IN]);
            reiChange = didCoordMove(mCurFpInfo.faces[i].righteyein, aptPoint[PT_POINT_RIGHT_EYE_IN]);
            leoChange = didCoordMove(mCurFpInfo.faces[i].lefteyeout, aptPoint[PT_POINT_LEFT_EYE_OUT]);
            reoChange = didCoordMove(mCurFpInfo.faces[i].righteyeout, aptPoint[PT_POINT_RIGHT_EYE_OUT]);
            lnChange = didCoordMove(mCurFpInfo.faces[i].leftnose, aptPoint[PT_POINT_NOSE_LEFT]);
            rnChange = didCoordMove(mCurFpInfo.faces[i].rightnose, aptPoint[PT_POINT_NOSE_RIGHT]);
            lmChange = didCoordMove(mCurFpInfo.faces[i].leftmouth, aptPoint[PT_POINT_MOUTH_LEFT]);
            rmChange = didCoordMove(mCurFpInfo.faces[i].rightmouth, aptPoint[PT_POINT_MOUTH_RIGHT]);
            cmChange = didCoordMove(mCurFpInfo.faces[i].centertopmouth, aptPoint[PT_POINT_MOUTH_UP]);
        }

        // When filtering, treat the left and right eyes as connected. In other words,
        // only update the coordinates if all of the eye coordinates have changed values.
        if (!mFilterFacePose || (leiChange && reiChange && leoChange && reoChange)) {
            if (anConf[PT_POINT_LEFT_EYE_OUT] > mConfThresh) {
                mCurFpInfo.faces[i].lefteyeout.x = aptPoint[PT_POINT_LEFT_EYE_OUT].x;
                mCurFpInfo.faces[i].lefteyeout.y = aptPoint[PT_POINT_LEFT_EYE_OUT].y;
                out->faces[i].lefteyeout.x = mCurFpInfo.faces[i].lefteyeout.x;
                out->faces[i].lefteyeout.y = mCurFpInfo.faces[i].lefteyeout.y;
                mThreshPassLE = true_e;
            }
            if (anConf[PT_POINT_LEFT_EYE_IN] > mConfThresh) {
                mCurFpInfo.faces[i].lefteyein.x = aptPoint[PT_POINT_LEFT_EYE_IN].x;
                mCurFpInfo.faces[i].lefteyein.y = aptPoint[PT_POINT_LEFT_EYE_IN].y;
                out->faces[i].lefteyein.x = mCurFpInfo.faces[i].lefteyein.x;
                out->faces[i].lefteyein.y = mCurFpInfo.faces[i].lefteyein.y;
                mThreshPassLE = true_e;
            }
            if (anConf[PT_POINT_RIGHT_EYE_OUT] > mConfThresh) {
                mCurFpInfo.faces[i].righteyeout.x = aptPoint[PT_POINT_RIGHT_EYE_OUT].x;
                mCurFpInfo.faces[i].righteyeout.y = aptPoint[PT_POINT_RIGHT_EYE_OUT].y;
                out->faces[i].righteyeout.x = mCurFpInfo.faces[i].righteyeout.x;
                out->faces[i].righteyeout.y = mCurFpInfo.faces[i].righteyeout.y;
                mThreshPassRE = true_e;
            }
            if (anConf[PT_POINT_RIGHT_EYE_IN] > mConfThresh) {
                mCurFpInfo.faces[i].righteyein.x = aptPoint[PT_POINT_RIGHT_EYE_IN].x;
                mCurFpInfo.faces[i].righteyein.y = aptPoint[PT_POINT_RIGHT_EYE_IN].y;
                out->faces[i].righteyein.x = mCurFpInfo.faces[i].righteyein.x;
                out->faces[i].righteyein.y = mCurFpInfo.faces[i].righteyein.y;
                mThreshPassRE = true_e;
            }

            mCurFpInfo.faces[i].roll = roll;
            mCurFpInfo.faces[i].pitch = pitch;
            mCurFpInfo.faces[i].yaw = yaw;
            out->faces[i].roll = mCurFpInfo.faces[i].roll;
            out->faces[i].pitch = mCurFpInfo.faces[i].pitch;
            out->faces[i].yaw = mCurFpInfo.faces[i].yaw;
        }

        if (!mFilterFacePose || (lnChange && rnChange)) {
            if (anConf[PT_POINT_NOSE_LEFT] > mConfThresh) {
                mCurFpInfo.faces[i].leftnose.x = aptPoint[PT_POINT_NOSE_LEFT].x;
                mCurFpInfo.faces[i].leftnose.y = aptPoint[PT_POINT_NOSE_LEFT].y;
                out->faces[i].leftnose.x = mCurFpInfo.faces[i].leftnose.x;
                out->faces[i].leftnose.y = mCurFpInfo.faces[i].leftnose.y;
                mThreshPassLN = true_e;
            }
            if (anConf[PT_POINT_NOSE_RIGHT] > mConfThresh) {
                mCurFpInfo.faces[i].rightnose.x = aptPoint[PT_POINT_NOSE_RIGHT].x;
                mCurFpInfo.faces[i].rightnose.y = aptPoint[PT_POINT_NOSE_RIGHT].y;
                out->faces[i].rightnose.x = mCurFpInfo.faces[i].rightnose.x;
                out->faces[i].rightnose.y = mCurFpInfo.faces[i].rightnose.y;
                mThreshPassRN = true_e;
            }
        }

        // When filtering, treat the nose and mouth as connected. In other words,
        // only update the coordinates if all of the nose and mouth coordinates have
        // changed values.
        if (!mFilterFacePose || (lmChange && rmChange && cmChange)) {
            if (anConf[PT_POINT_MOUTH_LEFT] > mConfThresh) {
                mCurFpInfo.faces[i].leftmouth.x = aptPoint[PT_POINT_MOUTH_LEFT].x;
                mCurFpInfo.faces[i].leftmouth.y = aptPoint[PT_POINT_MOUTH_LEFT].y;
                out->faces[i].leftmouth.x = mCurFpInfo.faces[i].leftmouth.x;
                out->faces[i].leftmouth.y = mCurFpInfo.faces[i].leftmouth.y;
                mThreshPassLM = true_e;
            }
            if (anConf[PT_POINT_MOUTH_RIGHT] > mConfThresh) {
                mCurFpInfo.faces[i].rightmouth.x = aptPoint[PT_POINT_MOUTH_RIGHT].x;
                mCurFpInfo.faces[i].rightmouth.y = aptPoint[PT_POINT_MOUTH_RIGHT].y;
                out->faces[i].rightmouth.x = mCurFpInfo.faces[i].rightmouth.x;
                out->faces[i].rightmouth.y = mCurFpInfo.faces[i].rightmouth.y;
                mThreshPassRM = true_e;
            }
            if (anConf[PT_POINT_MOUTH_UP] > mConfThresh) {
                mCurFpInfo.faces[i].centertopmouth.x = aptPoint[PT_POINT_MOUTH_UP].x;
                mCurFpInfo.faces[i].centertopmouth.y = aptPoint[PT_POINT_MOUTH_UP].y;
                out->faces[i].centertopmouth.x = mCurFpInfo.faces[i].centertopmouth.x;
                out->faces[i].centertopmouth.y = mCurFpInfo.faces[i].centertopmouth.y;
                mThreshPassUM = true_e;
            }
        }

        if (!mThreshPassLE || !mThreshPassRE) {
            out->numFaces = 0;
            break;
        }
    }

    if (numFaces > 0)
        mFDInit = true;

    DVP_PerformanceStop(&mPerf);
    DVP_PerformancePrint(&mPerf, "FacePose exec");

    return NOERROR;
}

bool FacePose::didCoordMove(ARXCoords &oldCoord, POINT &newCoord) const
{
    int xdiff = abs(oldCoord.x - newCoord.x);
    int ydiff = abs(oldCoord.y - newCoord.y);
    return ((xdiff > mMinMove && xdiff < mMaxMove) || (ydiff > mMinMove && ydiff < mMaxMove));
}

arxstatus_t FacePose::setProperty(uint32_t property, int32_t value)
{
    ARX_PRINT(ARX_ZONE_ENGINE, "FacePose: received property %u=%d", property, value);
    switch (property) {
        case PROP_ENGINE_FPD_FILTER:
        {
            mFilterFacePose = (value != 0);
            break;
        }
        case PROP_ENGINE_FPD_MINMOVE:
        {
            if (value < 0 || value > mMaxMove) {
                ARX_PRINT(ARX_ZONE_ERROR, "Invalid min move value of %d (must be less than or equal to max move of %d)", value, mMaxMove);
                return INVALID_VALUE;
            }
            mMinMove = value;
            break;
        }
        case PROP_ENGINE_FPD_MAXMOVE:
        {
            if (value < 0 || value < mMinMove) {
                ARX_PRINT(ARX_ZONE_ERROR, "Invalid max move value of %d (must be greater than or equal to min move of %d)", value, mMinMove);
                return INVALID_VALUE;
            }
            mMaxMove = value;
            break;
        }
        case PROP_ENGINE_FPD_CONFTHRESH:
        {
            if (value < 0 || value > 999) {
                ARX_PRINT(ARX_ZONE_ERROR, "Invalid confidence value of %d (must be greater than 0 and less than 1000)", value);
                return INVALID_VALUE;
            }
            mConfThresh = value;
            break;
        }
        default:
            return INVALID_PROPERTY;
    }
    ARX_PRINT(ARX_ZONE_ENGINE, "FacePose: accepted property %u=%d", property, value);
    return NOERROR;
}

arxstatus_t FacePose::getProperty(uint32_t property, int32_t *value)
{
    ARX_PRINT(ARX_ZONE_ENGINE, "FacePose: querying property %u", property);
    switch (property) {
        case PROP_ENGINE_FPD_FILTER:
        {
            *value = (int32_t)mFilterFacePose;
            break;
        }
        case PROP_ENGINE_FPD_MINMOVE:
        {
            *value = (int32_t)mMinMove;
            break;
        }
        case PROP_ENGINE_FPD_MAXMOVE:
        {
            *value = (int32_t)mMaxMove;
            break;
        }
        case PROP_ENGINE_FPD_CONFTHRESH:
        {
            *value = (int32_t)mConfThresh;
            break;
        }
        default:
            return INVALID_PROPERTY;
    }
    ARX_PRINT(ARX_ZONE_ENGINE, "FacePose: queried property %u=%d", property, *value);
    return NOERROR;
}

}
