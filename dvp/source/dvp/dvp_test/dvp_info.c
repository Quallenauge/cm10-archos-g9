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

#include <dvp/dvp.h>
#include <dvp/dvp_debug.h>


/*! Use \ref DVP_Core_e to index */
char cores[DVP_CORE_MAX][20] = {
    "DSP", // DVP_CORE_DSP
    "SIMCOP", // DVP_CORE_SIMCOP
    "MCU0", // DVP_CORE_MCU0
    "MCU1", // DVP_CORE_MCU1
    "GPU", // DVP_CORE_GPU
    "EVE", // DVP_CORE_EVE
    "CPU", // DVP_CORE_CPU
};

/*! use \ref DVP_KernelLibrary_e to index */
char libs[DVP_KL_LIBRARY_MAX][20] = {
    "INVALID",
    "YUV",
    "IMGFILTER",
    "OCL",
    "VLIB",
    "IMGLIB",
    "RVM",
    "VRUN",
    "TISMO",
    "ORB",
    "ARTI",
    "POINTCLOUD",
    "DSPLIB"
};

void dvp_core_info(void)
{
    DVP_Handle hDVP = DVP_KernelGraph_Init();
    DVP_CoreInfo_t info[DVP_CORE_MAX];
    DVP_Core_e c;
    DVP_KernelLibrary_e lib;

    memset(info, 0, sizeof(info));
    for (lib = DVP_KL_INVALID; lib < DVP_KL_LIBRARY_MAX; lib++)
        if (libs[lib][0] == '0')
            strcpy(libs[lib],"UNKNOWN");

    DVP_QuerySystem(hDVP, info);

    for (c = DVP_CORE_MIN+1; c < DVP_CORE_MAX; c++)
    {
        DVP_U32 k = 0;
        char libraries[MAX_PATH];
        memset(libraries, 0, sizeof(libraries));

        for (lib = DVP_KL_INVALID; lib < DVP_KL_LIBRARY_MAX; lib++)
            if (info[c].libraries[lib] == DVP_TRUE)
                sprintf(libraries, "%s %s", libraries, libs[lib]);

        DVP_PRINT(DVP_ZONE_ALWAYS, "DVP Core[%6s] is %s, has %3u kernels from these libraries: %s\n",
                  cores[c],
                  info[c].enabled?" ENABLED":"DISABLED",
                  info[c].numKernels,
                  libraries);

        for (k = 0; k < info[c].numKernels; k++)
        {
            DVP_PRINT(DVP_ZONE_ALWAYS, "\tKernel[%08x] = %s\n", info[c].kernels[k].kernel, info[c].kernels[k].name);
        }
    }

    DVP_KernelGraph_Deinit(hDVP);
}

void dvp_kernel_info(DVP_KernelNode_e kernel)
{
    DVP_BOOL present[DVP_CORE_MAX];
    char string[MAX_PATH];
    DVP_U32 c = 0;
    DVP_Handle hDVP = DVP_KernelGraph_Init();

    memset(present, 0, sizeof(present));
    DVP_QueryKernel(hDVP, kernel, present);
    memset(string, 0, sizeof(string));

    for (c = 0; c < dimof(present); c++)
        if (present[c] == DVP_TRUE)
            sprintf(string, "%s %s", string, cores[c]);

    DVP_PRINT(DVP_ZONE_ALWAYS, "Kernel %d (%x) is supported on: %s\n", kernel, kernel, string);

    DVP_KernelGraph_Deinit(hDVP);
}

int main(int argc, char *argv[])
{
#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

#if defined(DVP_RUNTIME_DEBUG)
    debug_get_zone_mask("DVP_ZONE_MASK", &dvp_zone_mask);
#endif

    if (argc > 1 && strcmp("kernel", argv[1]) == 0)
    {
        if (argc == 3)
        {
            DVP_KernelNode_e nodeEnum;
            sscanf(argv[2], "%x", &nodeEnum);
            dvp_kernel_info(nodeEnum);
        }
        else
        {
            printf("$ %s kernel 0x<enum>\n", argv[0]);
            DVP_PRINT(DVP_ZONE_ALWAYS, "$ %s kernel 0x<enum>\n", argv[0]);
        }
    }
    else
        dvp_core_info();
    return 0;
}
