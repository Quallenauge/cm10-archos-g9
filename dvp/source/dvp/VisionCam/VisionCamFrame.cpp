/*
 *  Copyright (C) 2009-2011 Texas Instruments, Inc.
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

#include <sosal/sosal.h>
#include <dvp/VisionCam.h>

#define VCAM_META_NONE              (0x0)
#define VCAM_META_AUTOWB            (0x1)
#define VCAM_META_MANUALWB          (0x2)
#define VCAM_META_ANCILLARY         (0x4)
#define VCAM_META_HISTO_2D          (0x8)
#define VCAM_META_HISTO_L           (0x10)
#define VCAM_META_HISTO_R           (0x20)
#define VCAM_META_FACES             (0x40)
#define VCAM_META_FACES_RAW         (0x80)
#define VCAM_META_UNSATURATED_REG   (0x100)
#define VCAM_META_FOCUS_REG         (0x200)
#define VCAM_META_MTIS              (0x400)
#define VCAM_META_GAMMA_2D          (0x800)
#define VCAM_META_GAMMA_L           (0x1000)
#define VCAM_META_GAMMA_R           (0x2000)

typedef struct _vcam_frame_header_t {
    uint32_t length;    /**< The length of the entire data packet */
    uint32_t count;     /**< The frame count */
    uint32_t metadata;  /**< A bit field of which metadata is present */
    uint32_t numFaces;  /**< A count of the number of faces detected */
} VisionCamFrameHeader_t;

#define CALLOC_STRUCT(ptr, type) ptr = (type *)calloc(1, sizeof(type))

size_t VisionCamFrame::maxSerialLength()
{
    size_t size = 0;

    size += sizeof(mFrameSource);
    size += sizeof(mTimestamp);
    size += sizeof(mWidth);
    size += sizeof(mHeight);
    size += sizeof(mOffsetX);
    size += sizeof(mOffsetY);
    size += sizeof(mStartX);
    size += sizeof(mStartY);
    size += sizeof(mLength);
    size += sizeof(mExtraDataLength);
    size += sizeof(VisionCamWhiteBalGains);
    size += sizeof(VisionCamWhiteBalGains);
    size += sizeof(VisionCamAncillary);
#if defined(VCAM_SUPPORT_HISTO_SERIALIZE)
    size += sizeof(VisionCamHistogram);
    size += sizeof(VisionCamHistogram);
#endif
    size += sizeof(mDetectedFacesNum);
    size += sizeof(mFaces);
    size += sizeof(mDetectedFacesNumRaw);
    size += sizeof(mFacesRaw);
    size += DVP_Image_Size((DVP_Image_t *)mFrameBuff);
    return size;
}

/** This serializes all data in the frame into a flat buffer */
size_t VisionCamFrame::serialize(uint8_t *buffer, size_t len)
{
    size_t offset = 0;
    uint32_t metadata = 0;
    DVP_Image_t *pImage = (DVP_Image_t *)mFrameBuff;
    SERIALIZE_UNIT(buffer, offset, len, mFrameSource);
    SERIALIZE_UNIT(buffer, offset, len, mTimestamp);
    SERIALIZE_UNIT(buffer, offset, len, mWidth);
    SERIALIZE_UNIT(buffer, offset, len, mHeight);
    SERIALIZE_UNIT(buffer, offset, len, mOffsetX);
    SERIALIZE_UNIT(buffer, offset, len, mOffsetY);
    SERIALIZE_UNIT(buffer, offset, len, mStartX);
    SERIALIZE_UNIT(buffer, offset, len, mStartY);
    SERIALIZE_UNIT(buffer, offset, len, mLength);
    SERIALIZE_UNIT(buffer, offset, len, mExtraDataLength);
    if (mMetadata.mAutoWBGains)
        metadata |= VCAM_META_AUTOWB;
    if (mMetadata.mManualWBGains)
        metadata |= VCAM_META_MANUALWB;
    if (mMetadata.mAncillary)
        metadata |= VCAM_META_ANCILLARY;
    if (mMetadata.mHistogram2D)
        metadata |= VCAM_META_HISTO_2D;
    if (mMetadata.mHistogramL)
        metadata |= VCAM_META_HISTO_L;
    if (mMetadata.mHistogramR)
        metadata |= VCAM_META_HISTO_R;
    if (mMetadata.mGamma2D)
        metadata |= VCAM_META_GAMMA_2D;
    if (mMetadata.mGammaL)
        metadata |= VCAM_META_GAMMA_L;
    if (mMetadata.mGammaR)
        metadata |= VCAM_META_GAMMA_R;
    if (mMetadata.mUnsaturatedRegions > 0)
        metadata |= VCAM_META_UNSATURATED_REG;
    if (mMetadata.mFocusRegionData > 0)
        metadata |= VCAM_META_FOCUS_REG;
    if (mMetadata.mMTIS_Data > 0)
        metadata |= VCAM_META_MTIS;
    if (mDetectedFacesNum > 0)
        metadata |= VCAM_META_FACES;
    if (mDetectedFacesNumRaw > 0)
        metadata |= VCAM_META_FACES_RAW;
    SERIALIZE_UNIT(buffer, offset, len, metadata);
    if (mMetadata.mAutoWBGains)
        SERIALIZE_ARRAY(buffer, offset, len, mMetadata.mAutoWBGains, sizeof(VisionCamWhiteBalGains));
    if (mMetadata.mManualWBGains)
        SERIALIZE_ARRAY(buffer, offset, len, mMetadata.mManualWBGains, sizeof(VisionCamWhiteBalGains));
    if (mMetadata.mAncillary)
        SERIALIZE_ARRAY(buffer, offset, len, mMetadata.mAncillary, sizeof(VisionCamAncillary));
#if defined(VCAM_SUPPORT_HISTO_SERIALIZE)
    if (mMetadata.mHistogram2D) {
        SERIALIZE_ARRAY(buffer, offset, len, mMetadata.mHistogram2D, sizeof(VisionCamHistogram));
    }
    if (mMetadata.mHistogramL) {
        SERIALIZE_ARRAY(buffer, offset, len, mMetadata.mHistogramL, sizeof(VisionCamHistogram));
    }
    if (mMetadata.mHistogramR) {
        SERIALIZE_ARRAY(buffer, offset, len, mMetadata.mHistogramR, sizeof(VisionCamHistogram));
    }
#endif
    if (mMetadata.mGamma2D) {
        SERIALIZE_ARRAY(buffer, offset, len, mMetadata.mGamma2D, sizeof(VisionCamGamma));
    }
    if (mMetadata.mGammaL) {
        SERIALIZE_ARRAY(buffer, offset, len, mMetadata.mGammaL, sizeof(VisionCamGamma));
    }
    if (mMetadata.mGammaR) {
        SERIALIZE_ARRAY(buffer, offset, len, mMetadata.mGammaR, sizeof(VisionCamGamma));
    }
    if( mMetadata.mUnsaturatedRegions )
        SERIALIZE_ARRAY(buffer, offset, len, mMetadata.mUnsaturatedRegions, sizeof(VisionCamUnsaturatedRegions));
    if( mMetadata.mFocusRegionData )
        SERIALIZE_ARRAY(buffer, offset, len, mMetadata.mFocusRegionData, sizeof(VisionCamFocusRegion));
    if( mMetadata.mMTIS_Data )
        SERIALIZE_ARRAY(buffer, offset, len, mMetadata.mMTIS_Data, sizeof(VisionCamMTIS));

    SERIALIZE_UNIT(buffer, offset, len, mDetectedFacesNum);
    SERIALIZE_ARRAY(buffer, offset, len, mFaces, sizeof(mFaces));

    SERIALIZE_UNIT(buffer, offset, len, mDetectedFacesNumRaw);
    SERIALIZE_ARRAY(buffer, offset, len, mFacesRaw, sizeof(mFacesRaw));

    offset += DVP_Image_Serialize(pImage, &buffer[offset], len-offset);

    return offset;
}

/** This unserializes all the data from a flat buffer back into the instance class object */
size_t VisionCamFrame::unserialize(uint8_t *buffer, size_t len)
{
    size_t offset = 0;
    uint32_t metadata = 0;
    DVP_Image_t *pImage = (DVP_Image_t *)mFrameBuff;
    UNSERIALIZE_UNIT(buffer, offset, len, mFrameSource);
    UNSERIALIZE_UNIT(buffer, offset, len, mTimestamp);
    UNSERIALIZE_UNIT(buffer, offset, len, mWidth);
    UNSERIALIZE_UNIT(buffer, offset, len, mHeight);
    UNSERIALIZE_UNIT(buffer, offset, len, mOffsetX);
    UNSERIALIZE_UNIT(buffer, offset, len, mOffsetY);
    UNSERIALIZE_UNIT(buffer, offset, len, mStartX);
    UNSERIALIZE_UNIT(buffer, offset, len, mStartY);
    UNSERIALIZE_UNIT(buffer, offset, len, mLength);
    UNSERIALIZE_UNIT(buffer, offset, len, mExtraDataLength);
    UNSERIALIZE_UNIT(buffer, offset, len, metadata);
    if (metadata & VCAM_META_AUTOWB) {
        CALLOC_STRUCT(mMetadata.mAutoWBGains, VisionCamWhiteBalGains);
        UNSERIALIZE_ARRAY(buffer, offset, len, mMetadata.mAutoWBGains, sizeof(VisionCamWhiteBalGains));
    }
    if (metadata & VCAM_META_MANUALWB) {
        CALLOC_STRUCT(mMetadata.mManualWBGains, VisionCamWhiteBalGains);
        UNSERIALIZE_ARRAY(buffer, offset, len, mMetadata.mManualWBGains, sizeof(VisionCamWhiteBalGains));
    }
    if (metadata & VCAM_META_ANCILLARY) {
        CALLOC_STRUCT(mMetadata.mAncillary, VisionCamAncillary);
        UNSERIALIZE_ARRAY(buffer, offset, len, mMetadata.mAncillary, sizeof(VisionCamAncillary));
    }
#if defined(VCAM_SUPPORT_HISTO_SERIALIZE)
    if (metadata & VCAM_META_HISTO_2D) {
        CALLOC_STRUCT(mMetadata.mHistogram2D, VisionCamHistogram);
        UNSERIALIZE_ARRAY(buffer, offset, len, mMetadata.mHistogram2D, sizeof(VisionCamHistogram));
    }
    if (metadata & VCAM_META_HISTO_L) {
        CALLOC_STRUCT(mMetadata.mHistogramL, VisionCamHistogram);
        UNSERIALIZE_ARRAY(buffer, offset, len, mMetadata.mHistogramL, sizeof(VisionCamHistogram));
    }
    if (metadata & VCAM_META_HISTO_R) {
        CALLOC_STRUCT(mMetadata.mHistogramR, VisionCamHistogram);
        UNSERIALIZE_ARRAY(buffer, offset, len, mMetadata.mHistogramR, sizeof(VisionCamHistogram));
    }
#endif
    if (metadata & VCAM_META_GAMMA_2D) {
        CALLOC_STRUCT(mMetadata.mGamma2D, VisionCamGamma);
        UNSERIALIZE_ARRAY(buffer, offset, len, mMetadata.mGamma2D, sizeof(VisionCamGamma));
    }
    if (metadata & VCAM_META_GAMMA_L) {
        CALLOC_STRUCT(mMetadata.mGammaL, VisionCamGamma);
        UNSERIALIZE_ARRAY(buffer, offset, len, mMetadata.mGammaL, sizeof(VisionCamGamma));
    }
    if (metadata & VCAM_META_GAMMA_R) {
        CALLOC_STRUCT(mMetadata.mGammaR, VisionCamGamma);
        UNSERIALIZE_ARRAY(buffer, offset, len, mMetadata.mGammaR, sizeof(VisionCamGamma));
    }
    if (metadata & VCAM_META_UNSATURATED_REG) {
        CALLOC_STRUCT(mMetadata.mUnsaturatedRegions, VisionCamUnsaturatedRegions);
        UNSERIALIZE_ARRAY(buffer, offset, len, mMetadata.mUnsaturatedRegions, sizeof(VisionCamUnsaturatedRegions));
    }
    if (metadata & VCAM_META_FOCUS_REG) {
        CALLOC_STRUCT(mMetadata.mFocusRegionData, VisionCamFocusRegion);
        UNSERIALIZE_ARRAY(buffer, offset, len, mMetadata.mFocusRegionData, sizeof(VisionCamFocusRegion));
    }
    if (metadata & VCAM_META_MTIS) {
        CALLOC_STRUCT(mMetadata.mMTIS_Data, VisionCamMTIS);
        UNSERIALIZE_ARRAY(buffer, offset, len, mMetadata.mMTIS_Data, sizeof(VisionCamMTIS));
    }

    UNSERIALIZE_UNIT(buffer, offset, len, mDetectedFacesNum);
    UNSERIALIZE_ARRAY(buffer, offset, len, mFaces, sizeof(mFaces));

    UNSERIALIZE_UNIT(buffer, offset, len, mDetectedFacesNumRaw);
    UNSERIALIZE_ARRAY(buffer, offset, len, mFacesRaw, sizeof(mFacesRaw));

    offset += DVP_Image_Unserialize(pImage, &buffer[offset], len-offset);

    return offset;
}

