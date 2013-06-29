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

#include <dvp/dvp.h>
#include <dvp_kgm.h>
#include <dvp/dvp_debug.h>

#include <ocl/ocl.h>
#include <ocl/dvp_kl_ocl.h>

#define KGM_TAG "DVP KGM OCL"

static DVP_CoreFunction_t local_kernels[] = {
    {"OCL SOBEL 8",     DVP_KN_SOBEL_8,     0, NULL, NULL},
    {"OCL SCHARR 8",    DVP_KN_SCHARR_8,    0, NULL, NULL},
    {"OCL PREWITT 8",   DVP_KN_PREWITT_8,   0, NULL, NULL},
    {"OCL KROON 8",     DVP_KN_KROON_8,     0, NULL, NULL},

    {"OCL SOBEL 8",     DVP_KN_OCL_SOBEL,   0, NULL, NULL},
    {"OCL SCHARR 8",    DVP_KN_OCL_SCHARR,  0, NULL, NULL},
    {"OCL PREWITT 8",   DVP_KN_OCL_PREWITT, 0, NULL, NULL},
    {"OCL KROON 8",     DVP_KN_OCL_KROON,   0, NULL, NULL},
};
static DVP_U32 numLocalKernels = dimof(local_kernels);

MODULE_EXPORT DVP_U32 DVP_GetSupportedKernels(DVP_CoreFunction_t **pFuncs)
{
    if (pFuncs != NULL)
        *pFuncs = local_kernels;
    DVP_PRINT(DVP_ZONE_KGM, "Retreiving "KGM_TAG" Kernel Information!\n");
    return numLocalKernels;
}

MODULE_EXPORT DVP_U32 DVP_GetSupportedRemoteCalls(DVP_Function_t **pRemote, DVP_U32 *pNum)
{
    *pRemote = 0;
    *pNum = 0;
    return 0;
}

MODULE_EXPORT DVP_U32 DVP_GetSupportedLocalCalls(DVP_Function_t **pLocal, DVP_U32 *pNum)
{
    *pLocal = 0;
    *pNum = 0;
    return 0;
}

MODULE_EXPORT DVP_Core_e DVP_GetSupportedRemoteCore()
{
    return DVP_CORE_GPU;
}

MODULE_EXPORT DVP_U32 DVP_GetMaximumLoad(void)
{
    return TARGET_NUM_CORES * 1000;
}

static ocl_t *ocl = NULL;

MODULE_EXPORT void DVP_KernelGraphManagerRestart(void *arg)
{
    // do nothing
}

MODULE_EXPORT DVP_BOOL DVP_KernelGraphManagerDeinit(void)
{
    ocl_fini(ocl);
    return DVP_TRUE;
}

MODULE_EXPORT DVP_BOOL DVP_KernelGraphManagerInit(DVP_RPC_t *pRPC       __attribute__ ((unused)),
                                                  DVP_RPC_Core_t *pCore __attribute__ ((unused)))
{
    ocl = ocl_init();
    return DVP_TRUE;
}

MODULE_EXPORT DVP_U32 DVP_KernelGraphManager(DVP_KernelNode_t *pSubNodes, DVP_U32 startNode, DVP_U32 numNodes)
{
    DVP_U32 n,i = 0;
    DVP_S32 processed = 0;
    DVP_Perf_t *pPerf = NULL;
    DVP_U32 kernel = 0;
    DVP_Error_e err = DVP_SUCCESS;

    if (pSubNodes)
    {
        for (n = startNode; n < (startNode + numNodes); n++)
        {
#ifdef DVP_DEBUG
            for (i = 0; i < dimof(local_kernels); i++)
            {
                if (local_kernels[i].kernel == pSubNodes[n].header.kernel) {
                    DVP_PRINT(DVP_ZONE_KGM, "Executing Kernel %s\n",local_kernels[i].name);
                }
            }
#endif
            // initialize the perf pointer and clock rate
            pPerf = &pSubNodes[n].header.perf;
            pPerf->rate = rtimer_freq(); // fill in the clock rate used to capture data.
            DVP_PerformanceStart(pPerf);

            switch (pSubNodes[n].header.kernel)
            {
                case DVP_KN_SOBEL_8:
                case DVP_KN_OCL_SOBEL:
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    err = ocl_Sobel8(ocl, &pT->input, &pT->output);
                    break;
                }
                case DVP_KN_SCHARR_8:
                case DVP_KN_OCL_SCHARR:
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    err = ocl_Scharr8(ocl, &pT->input, &pT->output);
                    break;
                }
                case DVP_KN_PREWITT_8:
                case DVP_KN_OCL_PREWITT:
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    err = ocl_Prewitt8(ocl, &pT->input, &pT->output);
                    break;
                }
                case DVP_KN_KROON_8:
                case DVP_KN_OCL_KROON:
                {
                    DVP_Transform_t *pT = dvp_knode_to(&pSubNodes[n], DVP_Transform_t);
                    err = ocl_Kroon8(ocl, &pT->input, &pT->output);
                    break;
                }
                default:
                {
                    DVP_PRINT(DVP_ZONE_ERROR, "ERROR: "KGM_TAG" Unknown Kernel %u\n",kernel);
                    processed--;
                    break;
                }
            }
            processed++;
            DVP_PerformanceStop(pPerf);
        }
        DVP_PRINT(DVP_ZONE_KGM, KGM_TAG": Processed %u nodes!\n", processed);
    }
    return processed;
}

MODULE_EXPORT DVP_U32 DVP_KernelGraphManagerVerify(DVP_KernelNode_t *pNodes,
                                                   DVP_U32 startNode,
                                                   DVP_U32 numNodes)
{
    DVP_U32 n;
    for (n = startNode; n < startNode + numNodes; n++)
    {
        pNodes[n].header.error = DVP_SUCCESS;
    }
    return numNodes;
}


#if defined(SYSBIOS_SL)
static static_function_t dvp_kgm_functions[] = {
    {"DVP_KernelGraphManagerInit",   (function_f)DVP_KernelGraphManagerInit},
    {"DVP_KernelGraphManager",       (function_f)DVP_KernelGraphManager},
    {"DVP_GetSupportedKernels",      (function_f)DVP_GetSupportedKernels},
    {"DVP_GetSupportedLocalCalls",   (function_f)DVP_GetSupportedLocalCalls},
    {"DVP_GetSupportedRemoteCalls",  (function_f)DVP_GetSupportedRemoteCalls},
    {"DVP_GetSupportedRemoteCore",   (function_f)DVP_GetSupportedRemoteCore},
    {"DVP_GetMaximumLoad",           (function_f)DVP_GetMaximumLoad},
    {"DVP_KernelGraphManagerDeinit", (function_f)DVP_KernelGraphManagerDeinit},
    {"DVP_KernelGraphManagerVerify", (function_f)DVP_KernelGraphManagerVerify},
};

static_module_table_t dvp_kgm_ocl_table = {
    "dvp_kgm_ocl",
    dimof(dvp_kgm_functions),
    dvp_kgm_functions
};
#endif
