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
 * \brief This file decribes the exact set of kernels supported by the OCL
 * algorithm library as exported through DVP.
 * \note When using these enums, only the OCL kernels will be used!
 * \defgroup group_algo_ocl Algos: OpenCL Kernels
 * \brief This only works on platforms with OpenCL!
 */

#ifndef _DVP_KL_OCL_H_
#define _DVP_KL_OCL_H_

#ifdef DVP_USE_OCL

#include <dvp/dvp_types.h>

/*! \brief The explicit list of kernels supported by the OCL algorithm library.
 *  \see DVP_Transform_t
 *  \ingroup group_algo_ocl
 */
enum DVP_KernelNode_Ocl_e {
    DVP_KN_OCL_BASE = DVP_KN_LIBRARY_BASE(DVP_KL_OCL),

    /*!
     * Image Convolution of FOURCC_Y800 of data using the Sobel Gx and Gy operators\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_OCL_SOBEL,

    /*!
     * Image Convolution of FOURCC_Y800 of data using the Scharr Gx and Gy operators\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_OCL_SCHARR,

    /*!
     * Image Convolution of FOURCC_Y800 of data using the Scharr Gx and Gy operators\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_OCL_KROON,

    /*!
     * Image Convolution of FOURCC_Y800 of data using the Prewitt Gx and Gy operators\n
     * Configuration Structure: DVP_Transform_t
     * \param [in] input  Image color type supported: FOURCC_Y800
     * \param [out] output Image color type supported: FOURCC_Y800
     */
    DVP_KN_OCL_PREWITT,

};

#endif // DVP_USE_OCL

#endif // _DVP_KL_OCL_H_

