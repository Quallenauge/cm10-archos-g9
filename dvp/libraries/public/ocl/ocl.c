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
 * \brief The "OpenCL Environment" wrapper for accessing OpenCL kernels.
 * \see http://github.com/emrainey/OpenCL-Environment
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#ifdef DARWIN
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include <ocl/clenvironment.h>
#include <ocl/clquery.h>

typedef struct _ocl_t {
    cl_environment_t *env;
} ocl_t;

#define _OCL_T_DEFINED
#include <ocl/ocl.h>
#include <ocl/clkernels.h>
#include <dvp/dvp_debug.h>

static void cl_notifier(cl_program program, void *args)
{
    DVP_PRINT(DVP_ZONE_API, "OpenCL Program %p compiled!\n", program);
}

void ocl_fini(ocl_t *ocl)
{
    if (ocl)
    {
        clDeleteEnvironment(ocl->env);
    }
}

ocl_t *ocl_init()
{
    ocl_t *ocl = (ocl_t *)calloc(1, sizeof(ocl_t));
    if (ocl)
    {
        ocl->env = clCreateEnvironment(OCL_ROOT"kernels.cl", clGetTypeFromString(CL_USER_DEVICE_TYPE), CL_USER_DEVICE_COUNT, cl_notifier, CL_ARGS);
    }
    return ocl;
}

DVP_Error_e ocl_Sobel8(ocl_t *ocl, DVP_Image_t *pInput, DVP_Image_t *pOutput)
{
    DVP_PRINT(DVP_ZONE_API, "OCL:  %p (env=%p) SOBEL in %p out %p\n", ocl, ocl->env, pInput, pOutput);
    if (ocl && ocl->env)
    {
        cl_char operator[2][3][3] = { { {1,2,1}, {0,0,0}, {-1,-2,-1} }, { {-1,0,1}, {-2,0,2}, {-1,0,1} } };
        cl_uint limit = 255;
        cl_uint range = range_of_operator((cl_char *)operator, 3, limit);
        cl_int err = imgfilter1d_opt(ocl->env,
                                     pInput->width,
                                     pInput->height,
                                     pInput->pData[0],
                                     pInput->y_stride,
                                     pOutput->pData[0],
                                     pOutput->y_stride,
                                     (cl_char *)operator,
                                     3,
                                     range,limit);
        return DVP_SUCCESS;
    }
    return DVP_ERROR_INVALID_PARAMETER;
}

DVP_Error_e ocl_Scharr8(ocl_t *ocl, DVP_Image_t *pInput, DVP_Image_t *pOutput)
{
    DVP_PRINT(DVP_ZONE_API, "OCL:  %p (env=%p) SCHARR in %p out %p\n", ocl, ocl->env, pInput, pOutput);
    if (ocl && ocl->env)
    {
        cl_char operator[2][3][3] = { { {3,10,3}, {0,0,0}, {-3,-10,-3} }, { {3,0,-3}, {10,0,-10}, {3,0,-3} } };
        cl_uint limit = 255;
        cl_uint range = range_of_operator((cl_char *)operator, 3, limit);
        cl_int err = imgfilter1d_opt(ocl->env,
                                     pInput->width,
                                     pInput->height,
                                     pInput->pData[0],
                                     pInput->y_stride,
                                     pOutput->pData[0],
                                     pOutput->y_stride,
                                     (cl_char *)operator,
                                     3,
                                     range,limit);
        return DVP_SUCCESS;
    }
    return DVP_ERROR_INVALID_PARAMETER;
}

DVP_Error_e ocl_Prewitt8(ocl_t *ocl, DVP_Image_t *pInput, DVP_Image_t *pOutput)
{
    DVP_PRINT(DVP_ZONE_API, "OCL:  %p (env=%p) PREWITT in %p out %p\n", ocl, ocl->env, pInput, pOutput);
    if (ocl && ocl->env)
    {
        cl_char operator[2][3][3] = { { {1,1,1}, {0,0,0}, {-1,-1,-1} }, { {-1,0,1}, {-1,0,1}, {-1,0,1} } };
        cl_uint limit = 255;
        cl_uint range = range_of_operator((cl_char *)operator, 3, limit);
        cl_int err = imgfilter1d_opt(ocl->env,
                                     pInput->width,
                                     pInput->height,
                                     pInput->pData[0],
                                     pInput->y_stride,
                                     pOutput->pData[0],
                                     pOutput->y_stride,
                                     (cl_char *)operator,
                                     3,
                                     range,limit);
        return DVP_SUCCESS;
    }
    return DVP_ERROR_INVALID_PARAMETER;
}

DVP_Error_e ocl_Kroon8(ocl_t *ocl, DVP_Image_t *pInput, DVP_Image_t *pOutput)
{
    DVP_PRINT(DVP_ZONE_API, "OCL:  %p (env=%p) KROON in %p out %p\n", ocl, ocl->env, pInput, pOutput);
    if (ocl && ocl->env)
    {
        cl_char operator[2][3][3] = { { {17,61,17}, {0,0,0}, {-17,-61,-17} }, { {17,0,-17}, {61,0,-61}, {17,0,-17} } };
        cl_uint limit = 255;
        cl_uint range = range_of_operator((cl_char *)operator, 3, limit);
        cl_int err = imgfilter1d_opt(ocl->env,
                                     pInput->width,
                                     pInput->height,
                                     pInput->pData[0],
                                     pInput->y_stride,
                                     pOutput->pData[0],
                                     pOutput->y_stride,
                                     (cl_char *)operator,
                                     3,
                                     range,limit);
        return DVP_SUCCESS;
    }
    return DVP_ERROR_INVALID_PARAMETER;
}

