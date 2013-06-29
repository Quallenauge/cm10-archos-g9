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

/*! \file
 * \brief The "OpenCL Environment" Image Filter Prototypes
 * \see http://github.com/emrainey/OpenCL-Environment
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#ifndef _OCL_KERNELS_H_
#define _OCL_KERNELS_H_

cl_int range_of_operator(cl_char *op, cl_uint n, cl_uint limit);

cl_int imgfilter1d(cl_environment_t *pEnv,
                   cl_uint width,
                   cl_uint height,
                   cl_uchar *pSrc,
                   cl_int srcStride,
                   cl_uchar *pDst,
                   cl_int dstStride,
                   cl_char *op,
                   cl_uint opDim,
                   cl_uint range,
                   cl_uint limit);

cl_int imgfilter1d_opt(cl_environment_t *pEnv,
                   cl_uint width,
                   cl_uint height,
                   cl_uchar *pSrc,
                   cl_int srcStride,
                   cl_uchar *pDst,
                   cl_int dstStride,
                   cl_char *op,
                   cl_uint opDim,
                   cl_uint range,
                   cl_uint limit);

#endif

