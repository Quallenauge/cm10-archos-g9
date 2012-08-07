/*
 * Copyright (C) Texas Instruments - http://www.ti.com/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
* @file OMX3A.cpp
*
* This file contains functionality for handling 3A configurations.
*
*/

#undef LOG_TAG

#define LOG_TAG "OMXMetaData"

#include "OMXCameraAdapter.h"

namespace Ti {
namespace Camera {

#ifdef OMAP_ENHANCEMENT_CPCAM
status_t OMXCameraAdapter::setMetaData(android::CameraMetadata &meta_data, const OMX_PTR plat_pvt) const
{
    status_t ret = NO_ERROR;
    OMX_OTHER_EXTRADATATYPE *extraData;

    extraData = getExtradata(plat_pvt, (OMX_EXTRADATATYPE) OMX_WhiteBalance);

    if ( NULL != extraData ) {
        OMX_TI_WHITEBALANCERESULTTYPE * WBdata;
        WBdata = (OMX_TI_WHITEBALANCERESULTTYPE*) extraData->data;

        meta_data.set(android::CameraMetadata::KEY_AWB_TEMP, (int)WBdata->nColorTemperature);
        meta_data.set4(android::CameraMetadata::KEY_AWB_GAINS,
                                   (int)WBdata->nGainR,
                                   (int)WBdata->nGainGR,
                                   (int)WBdata->nGainGB,
                                   (int)WBdata->nGainB);
        meta_data.set4(android::CameraMetadata::KEY_AWB_OFFSETS,
                                   (int)WBdata->nOffsetR,
                                   (int)WBdata->nOffsetGR,
                                   (int)WBdata->nOffsetGB,
                                   (int)WBdata->nOffsetB);
    }

    // TODO(XXX): temporarily getting exposure and gain data from vector shot extra data
    // change to ancil or cpcam metadata once Ducati side is ready
    extraData = getExtradata(plat_pvt, (OMX_EXTRADATATYPE) OMX_TI_VectShotInfo);

    if ( NULL != extraData ) {
        OMX_TI_VECTSHOTINFOTYPE *shotInfo;
        shotInfo = (OMX_TI_VECTSHOTINFOTYPE*) extraData->data;

        meta_data.set(android::CameraMetadata::KEY_FRAME_NUMBER, (int)shotInfo->nFrameNum);
        meta_data.set(android::CameraMetadata::KEY_SHOT_NUMBER, (int)shotInfo->nConfigId);
        meta_data.set(android::CameraMetadata::KEY_ANALOG_GAIN, (int)shotInfo->nAGain);
        meta_data.set(android::CameraMetadata::KEY_ANALOG_GAIN_REQ, (int)shotInfo->nReqGain);
        meta_data.set(android::CameraMetadata::KEY_ANALOG_GAIN_MIN, (int)shotInfo->nGainMin);
        meta_data.set(android::CameraMetadata::KEY_ANALOG_GAIN_MAX, (int)shotInfo->nGainMax);
        meta_data.set(android::CameraMetadata::KEY_ANALOG_GAIN_DEV, (int)shotInfo->nDevAGain);
        meta_data.set(android::CameraMetadata::KEY_ANALOG_GAIN_ERROR, (int)shotInfo->nSenAGainErr);
        meta_data.set(android::CameraMetadata::KEY_EXPOSURE_TIME, (int)shotInfo->nExpTime);
        meta_data.set(android::CameraMetadata::KEY_EXPOSURE_TIME_REQ, (int)shotInfo->nReqExpTime);
        meta_data.set(android::CameraMetadata::KEY_EXPOSURE_TIME_MIN, (int)shotInfo->nExpMin);
        meta_data.set(android::CameraMetadata::KEY_EXPOSURE_TIME_MAX, (int)shotInfo->nExpMax);
        meta_data.set(android::CameraMetadata::KEY_EXPOSURE_TIME_DEV, (int)shotInfo->nDevExpTime);
        meta_data.set(android::CameraMetadata::KEY_EXPOSURE_TIME_ERROR, (int)shotInfo->nSenExpTimeErr);
        meta_data.set(android::CameraMetadata::KEY_EXPOSURE_COMPENSATION_REQ, (int)shotInfo->nReqEC);
        meta_data.set(android::CameraMetadata::KEY_EXPOSURE_DEV, (int)shotInfo->nDevEV);
    }

    // TODO(XXX): Use format abstraction for LSC values
    // LSC table
    extraData = getExtradata(plat_pvt, (OMX_EXTRADATATYPE) OMX_TI_LSCTable);

    if ( NULL != extraData ) {
        OMX_TI_LSCTABLETYPE *lscTbl;
        OMX_U8 *lsc;
        android::String8 val;
        lscTbl = (OMX_TI_LSCTABLETYPE*) extraData->data;
        lsc = lscTbl->pGainTable;
        if ( (0U == lscTbl->nWidth) || (0U == lscTbl->nHeight) ) {
            CAMHAL_LOGE("Zero size LSC table");
        } else if ( OMX_TI_LSC_GAIN_TABLE_SIZE < (lscTbl->nWidth * lscTbl->nHeight * 4) ) {
            CAMHAL_LOGE("Oversized LSC table");
        } else {
            for (unsigned int j = 0; j < lscTbl->nHeight; j++) {
                if (0 != j) {
                    val.append(",");
                }
                val.append("(");
                for (unsigned int i = 0; i < lscTbl->nWidth; i++) {
                    if (0 != i) {
                        val.append(",");
                    }
                    for (unsigned int h = 0; h < 4; h++) {
                        if (0 != h) {
                            val.append(":");
                        }
                        val.appendFormat("%d", *(lsc++));
                    }
                }
                val.append(")");
            }
            meta_data.set(android::CameraMetadata::KEY_LSC_TABLE, val);
            meta_data.setBool(android::CameraMetadata::KEY_LSC_TABLE_APPLIED,
                              (OMX_TRUE == lscTbl->bApplied) ? true : false);
        }
    }

    return ret;
}
#endif

void OMXCameraAdapter::encodePreviewMetadata(camera_frame_metadata_t *meta, const OMX_PTR plat_pvt)
{
#ifdef OMAP_ENHANCEMENT_CPCAM
    OMX_OTHER_EXTRADATATYPE *extraData = NULL;

    extraData = getExtradata(plat_pvt, (OMX_EXTRADATATYPE) OMX_TI_VectShotInfo);

    if ( (NULL != extraData) && (NULL != extraData->data) ) {
        OMX_TI_VECTSHOTINFOTYPE *shotInfo;
        shotInfo = (OMX_TI_VECTSHOTINFOTYPE*) extraData->data;

        meta->analog_gain = shotInfo->nAGain;
        meta->exposure_time = shotInfo->nExpTime;
    } else {
        meta->analog_gain = -1;
        meta->exposure_time = -1;
    }
#else
    // no-op in non enhancement mode
    CAMHAL_UNUSED(meta);
    CAMHAL_UNUSED(plat_pvt);
#endif
}

} // namespace Camera
} // namespace Ti
