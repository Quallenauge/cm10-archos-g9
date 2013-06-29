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
 * \brief The "OpenCL Environment" Vision Header
 * \see http://github.com/emrainey/OpenCL-Environment
 * \author Erik Rainey <erik.rainey@ti.com>
 */

/*!
 * \page page_ocl OpenCL Image Filter Library
 * This library implements a 3x3 convolution operation on 8 bit data in fixed OpenCL kernels.
 */

#ifndef _OCL_VISION_H_
#define _OCL_VISION_H_

#include <dvp/dvp.h>

#ifndef _OCL_T_DEFINED
typedef void * ocl_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

ocl_t *ocl_init();
void   ocl_fini(ocl_t *ocl);

DVP_Error_e ocl_Sobel8(ocl_t *ocl, DVP_Image_t *pInput, DVP_Image_t *pOutput);
DVP_Error_e ocl_Scharr8(ocl_t *ocl, DVP_Image_t *pInput, DVP_Image_t *pOutput);
DVP_Error_e ocl_Prewitt8(ocl_t *ocl, DVP_Image_t *pInput, DVP_Image_t *pOutput);
DVP_Error_e ocl_Kroon8(ocl_t *ocl, DVP_Image_t *pInput, DVP_Image_t *pOutput);

#ifdef __cplusplus
}
#endif

#endif


