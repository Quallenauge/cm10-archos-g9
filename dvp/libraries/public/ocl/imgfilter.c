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
 * \brief The "OpenCL Environment" imgfilter kernel C interface.
 * \see http://github.com/emrainey/OpenCL-Environment
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <sosal/sosal.h>
#ifdef DARWIN
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#include <ocl/clenvironment.h>
#include <ocl/clquery.h>
#include <ocl/clmath.h>

cl_int range_of_operator(cl_char *op, cl_uint n, cl_uint limit)
{
    cl_int range;
    cl_uint p,q,r,s;
    cl_int max = 0, min = 0x7FFFFFFF;
    for (p = 0; p < 2; p++)
    {
        for (q = 0; q < n; q++)
        {
            for (r = 0; r < n; r++)
            {
                s = (p * n * n) + (q * n) + r;
                if (op[s] < min)
                    min = op[s];
                else if (op[s] > max)
                    max = op[s];
            }
        }
    }
    range = isqrt(ipow(abs(min)*limit,2) + ipow(max*limit,2));
    return range;
}

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
                   cl_uint limit)
{
    cl_int err = CL_SUCCESS;
    cl_uint numSrcBytes = srcStride * height;
    cl_uint numDstBytes = dstStride * height;
    cl_uint numOpBytes = 2 * opDim * opDim;
    cl_kernel_param_t params[] = {
        {CL_KPARAM_BUFFER_0D, sizeof(cl_uint), &width, NULL, CL_MEM_READ_ONLY},
        {CL_KPARAM_BUFFER_0D, sizeof(cl_uint), &height, NULL, CL_MEM_READ_ONLY},
        {CL_KPARAM_BUFFER_1D, numSrcBytes, pSrc, NULL, CL_MEM_READ_ONLY},
        {CL_KPARAM_BUFFER_0D, sizeof(cl_int), &srcStride, NULL, CL_MEM_READ_ONLY},
        {CL_KPARAM_BUFFER_1D, numDstBytes, pDst, NULL, CL_MEM_WRITE_ONLY},
        {CL_KPARAM_BUFFER_0D, sizeof(cl_int), &dstStride, NULL, CL_MEM_READ_ONLY},
        {CL_KPARAM_BUFFER_1D, numOpBytes, op, NULL, CL_MEM_READ_ONLY},
        {CL_KPARAM_BUFFER_0D, sizeof(cl_uint), &opDim, NULL, CL_MEM_READ_ONLY},
        {CL_KPARAM_BUFFER_0D, sizeof(cl_uint), &range, NULL, CL_MEM_READ_ONLY},
        {CL_KPARAM_BUFFER_0D, sizeof(cl_uint), &limit, NULL, CL_MEM_READ_ONLY},
    };
    cl_kernel_call_t call = {
        "kernel_edge_filter",
        params, dimof(params),
        2,
        {0,0,0},
        {width, height, 0},
        {1,1,1},
        CL_SUCCESS, 0,0,0
    };
    err = clCallKernel(pEnv, &call,1);
    return err;
}

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
                   cl_uint limit)
{
    cl_int err = CL_SUCCESS;
    cl_uint numSrcBytes = srcStride * height;
    cl_uint numDstBytes = dstStride * height;
    cl_uint numOpBytes = 2 * opDim * opDim;
    cl_kernel_param_t params[] = {
        {CL_KPARAM_BUFFER_0D, sizeof(cl_uint), &width, NULL, CL_MEM_READ_ONLY},
        {CL_KPARAM_BUFFER_0D, sizeof(cl_uint), &height, NULL, CL_MEM_READ_ONLY},
        {CL_KPARAM_BUFFER_1D, numSrcBytes, pSrc, NULL, CL_MEM_READ_ONLY},
        {CL_KPARAM_BUFFER_0D, sizeof(cl_int), &srcStride, NULL, CL_MEM_READ_ONLY},
        {CL_KPARAM_BUFFER_1D, numDstBytes, pDst, NULL, CL_MEM_WRITE_ONLY},
        {CL_KPARAM_BUFFER_0D, sizeof(cl_int), &dstStride, NULL, CL_MEM_READ_ONLY},
        {CL_KPARAM_BUFFER_1D, numOpBytes, op, NULL, CL_MEM_READ_ONLY},
        {CL_KPARAM_BUFFER_0D, sizeof(cl_uint), &opDim, NULL, CL_MEM_READ_ONLY},
        {CL_KPARAM_BUFFER_0D, sizeof(cl_uint), &range, NULL, CL_MEM_READ_ONLY},
        {CL_KPARAM_BUFFER_0D, sizeof(cl_uint), &limit, NULL, CL_MEM_READ_ONLY},
        {CL_KPARAM_LOCAL,     sizeof(cl_uchar)*18, NULL, NULL, CL_MEM_READ_WRITE},
        {CL_KPARAM_LOCAL,     sizeof(cl_char)*18, NULL, NULL, CL_MEM_READ_WRITE},
    };
    cl_kernel_call_t call = {
        "kernel_edge_filter_opt",
        params, dimof(params),
        2,
        {0,0,0},
        {width, height, 0},
        {1,1,1},
        CL_SUCCESS, 0,0,0
    };
    err = clCallKernel(pEnv, &call,1);
    return err;
}


