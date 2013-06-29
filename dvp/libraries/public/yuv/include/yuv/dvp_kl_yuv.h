/*
 *  Copyright (C) 2009-2012 Texas Instruments, Inc.
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

/*!
 * \file
 * \brief This file decribes the exact set of kernels supported by the YUV
 * algorithm library as exported through DVP.
 * \note When using these enums, only the YUV kernels will be used!
 */

#ifndef _DVP_KL_YUV_H_
#define _DVP_KL_YUV_H_

#ifdef DVP_USE_YUV

#include <dvp/dvp_types.h>

/*! \brief The explicit list of kernels supported by the YUV algorithm library 
 * \ingroup group_yuv
 */
enum DVP_KernelNode_Yuv_e {
    DVP_KN_YUV_BASE = DVP_KN_LIBRARY_BASE(DVP_KL_YUV),

    /*!
     * Converts FOURCC_UYVY format to FOURCC_IYUV or FOURCC_YV12 format. Chroma planes are downsampled vertically. (VLIB-API.Sec 55.0)\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_UYVY
     * \param [out] output Image color type supported: FOURCC_IYUV or FOURCC_YV12
     */
    DVP_KN_YUV_UYVY_TO_IYUV,

    /*!
     * Converts FOURCC_UYVY format to 8 bits per pixels planar RGB using BT.601. (VLIB-API.Sec 58.0)\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_UYVY
     * \param [out] output Image color type supported: FOURCC_RGBP
     */
    DVP_KN_YUV_UYVY_TO_RGBp,

    /*!
     * Converts FOURCC_UYVY or FOURCC_VYUY to FOURCC_Y800 image\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_UYVY or FOURCC_VYUY
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_YUV_XYXY_TO_Y800,

    /*!
     * Converts FOURCC_YVYU or FOURCC_YUY2 format to FOURCC_Y800 image.\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_YVYU or FOURCC_YUY2
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_YUV_YXYX_TO_Y800,

    /*!
     * Converts 1 plane 8 bit per pixel luma to FOURCC_UYVY\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_UYVY
     */
    DVP_KN_YUV_Y800_TO_XYXY,

    /*!
     * Converts FOURCC_UYVY format to 1 plane 8 bit pixel packed BGR byte ordered 24 bit per pixel format (FOURCC_BGR) using BT.601\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_UYVY
     * \param [out] output Image color type supported: FOURCC_BGR
     */
    DVP_KN_YUV_UYVY_TO_BGR,

    /*!
     * Converts FOURCC_UYVY format to 1 plane 565 bit pixel packed BGR ordered format (FOURCC_RGB565) using BT.601 \n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_UYVY
     * \param [out] output Image color type supported: FOURCC_RGB565
     */
    DVP_KN_YUV_IYUV_TO_RGB565,

    /*!
     * Converts FOURCC_IYUV format to 1 plane 8 bit pixel packed BGR byte ordered 24 bit per pixel format (FOURCC_BGR) using BT.601\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_IYUV
     * \param [out] output Image color type supported: FOURCC_BGR
     */
    DVP_KN_YUV_IYUV_TO_BGR,

    /*!
     * Converts FOURCC_IYUV format to 1 plane 8 bits per pixels planar RGB using BT.601 format (FOURCC_BGR) using BT.601\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_IYUV
     * \param [out] output Image color type supported: FOURCC_RGBP
     */
    DVP_KN_YUV_IYUV_TO_RGBp,

    /*!
     * Converts 1 plane RGB 888 FOURCC_BGR to FOURCC_UYVY\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_RGB888 or FOURCC_BGR
     * \param [out] output Image color type supported: FOURCC_UYVY
     */
    DVP_KN_YUV_BGR_TO_UYVY,

    /*!
     * Converts 1 plane RGB 888 FOURCC_BGR to FOURCC_IYUV\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_RGB888 or FOURCC_BGR
     * \param [out] output Image color type supported: FOURCC_IYUV
     */
    DVP_KN_YUV_BGR_TO_IYUV,

    /*!
     * Horizontally mirrors the FOURCC_UYVY data\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input     Image color type supported: FOURCC_UYVY
     * \param [out] output    Image color type supported: FOURCC_UYVY
     */
    DVP_KN_YUV_UYVY_MIRROR,

    /*!
     * Half-scale downsamples the FOURCC_UYVY data\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input     Image color type supported: FOURCC_UYVY
     * \param [out] output    Image color type supported: FOURCC_UYVY
     */
    DVP_KN_YUV_UYVY_HALF_SCALE,

    /*!
     * Quarter-scale downsamples the FOURCC_UYVY data\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input     Image color type supported: FOURCC_UYVY
     * \param [out] output    Image color type supported: FOURCC_UYVY
     */
    DVP_KN_YUV_UYVY_QTR_SCALE,

    /*!
     * Rotates any N plane (8 bit per plane) image by 90 Degrees Clockwise\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input     Image color type supported: FOURCC_IYUV
     * \param [out] output    Image color type supported: FOURCC_IYUV
     */
    DVP_KN_YUV_Y800_ROTATE_CW_90,

    /*!
     * Rotates any N plane (8 bit per plane) image by 90 Degrees Counter-Clockwise\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input     Image color type supported: FOURCC_IYUV
     * \param [out] output    Image color type supported: FOURCC_IYUV
     */
    DVP_KN_YUV_Y800_ROTATE_CCW_90,

    /*!
     * Rotates FOURCC_UYVY data by 90 Degrees Clockwise, resamples Chroma\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input     Image color type supported: FOURCC_UYVY
     * \param [out] output    Image color type supported: FOURCC_UYVY
     */
    DVP_KN_YUV_UYVY_ROTATE_CW_90,

    /*!
     * Rotates FOURCC_UYVY data by 90 Degrees Counter-Clockwise, resamples Chroma\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input     Image color type supported: FOURCC_UYVY
     * \param [out] output    Image color type supported: FOURCC_UYVY
     */
    DVP_KN_YUV_UYVY_ROTATE_CCW_90,

    /*!
     * Converts FOURCC_UYVY format to FOURCC_YU24 or FOURCC_YV24 format. Chroma planes are doubled.\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_UYVY
     * \param [out] output Image color type supported: FOURCC_YV24 or FOURCC_YU24
     */
    DVP_KN_YUV_UYVY_TO_YUV444p,

    /*!
     * Converts FOURCC_UYVY format to FOURCC_NV12 or FOURCC_YV24 format. Luma plane is halved in each direction.\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_NV12
     * \param [out] output Image color type supported: FOURCC_YV24 or FOURCC_YU24
     */
    DVP_KN_YUV_NV12_TO_YU24_HALF_SCALE,

    /*!
     * Converts FOURCC_UYVY format to FOURCC_RGBP, FOURCC_Y800, & FOURCC_YU24. \n
     * Configuration Structure: DVP_YUV_TripleTransform_t
     * \param [in] input  Image color type supported: FOURCC_UYVY
     * \param [out] output Image color type supported: FOURCC_RGBP
     * \param [out] output Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_YV24 or FOURCC_YU24
     */
    DVP_KN_YUV_UYVY_TO_RGBp_Y800_YU24,

    /*!
     * Converts FOURCC_ARGB format to FOURCC_UYVY\n.
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_ARGB
     * \param [out] output Image color type supported: FOURCC_UYVY
     */
    DVP_KN_YUV_ARGB_TO_UYVY,

    /*!
     * Converts FOURCC_YU24 or FOURCC_YV24 into 8 bits per pixels planar RGB using BT.601.\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input Image color type supported: FOURCC_YU24 or FOURCC_YV24
     * \param [out] output Image color type supported: FOURCC_RGBP
     */
    DVP_KN_YUV_YUV444_TO_RGBp,
};

/*! \brief Use this struct with the YUV kernels which support one input and three outputs.
 * \see DVP_KN_YUV_UYVY_TO_RGBp_Y800_YU24
 * \ingroup group_yuv
 */
typedef struct _dvp_yuv_triple_transform_t {
    DVP_Image_t input;
    DVP_Image_t out1;
    DVP_Image_t out2;
    DVP_Image_t out3;
} DVP_YUV_TripleTransform_t;

#endif // DVP_USE_YUV

#endif // _DVP_KL_YUV_H_

