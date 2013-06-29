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
 * \brief This file decribes the exact set of kernels supported by the IMGFILTER
 * algorithm library as exported through DVP.
 * \note When using these enums, only the IMGFILTER kernels will be used!
 */

#ifndef _DVP_KL_IMGFILTER_H_
#define _DVP_KL_IMGFILTER_H_

#ifdef DVP_USE_IMGFILTER

#include <dvp/dvp_types.h>

/*!
 * \brief The explicit list of kernels supported by the IMGFILTER algorithm library.
 * \see DVP_Transform_t
 * \ingroup group_imgfilter
 */
enum DVP_KernelNode_Imgfilter_e {
    DVP_KN_IMGFILTER_BASE = DVP_KN_LIBRARY_BASE(DVP_KL_IMGFILTER),

    /*!
     * Image Convolution of FOURCC_Y800 of data using the Sobel Gx and Gy operators\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_IMGFILTER_SOBEL,

    /*!
     * Image Convolution of FOURCC_Y800 of data using the Scharr Gx and Gy operators\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_IMGFILTER_SCHARR,

    /*!
     * Image Convolution of FOURCC_Y800 of data using the Scharr Gx and Gy operators\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_IMGFILTER_KROON,

    /*!
     * Image Convolution of FOURCC_Y800 of data using the Prewitt Gx and Gy operators\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_IMGFILTER_PREWITT,

    /*!
     * Image Filter of 1x3 with predetermined coefficients.
     * \param [in] input Image Color type supported: FOURCC_ARGB
     * \param [out] output Image Color type supported: FOURCC_ARGB
     */
    DVP_KN_IMGFILTER_3CHAN_TAP_FILTER_1x3,
};

#endif // DVP_USE_IMGFILTER

#endif // _DVP_KL_IMGFILTER_H_

