/**
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
#include <dvp/dvp_debug.h>

#if defined(LINUX)
#include <sys/time.h>
#include <sys/resource.h>
#endif

typedef struct _dvp_image_params_t {
    DVP_Image_t *pImage;
    DVP_U32      width;
    DVP_U32      height;
    fourcc_t       color;
    DVP_MemType_e type;
} DVP_Image_Params_t;

DVP_BOOL DVP_Image_Construct(DVP_Handle handle, DVP_Image_Params_t *params, DVP_U32 numParams)
{
    DVP_BOOL ret = DVP_TRUE;
    DVP_U32 i = 0;
    for (i = 0; i < numParams; i++)
    {
        DVP_Image_Init(params[i].pImage, params[i].width, params[i].height, params[i].color);
        params[i].pImage->skipCacheOpFlush = DVP_TRUE;
        params[i].pImage->skipCacheOpInval = DVP_TRUE;
        if (DVP_Image_Alloc(handle, params[i].pImage, params[i].type) == DVP_FALSE) {
            ret = DVP_FALSE;
            goto failed;
        }
    }
failed:
    return ret;
}

void DVP_KernelGraphCompleted(void *cookie, DVP_KernelGraph_t *graph, DVP_U32 sectionIndex, DVP_U32 numNodesExecuted)
{
    cookie = cookie; // warnings
    graph = graph; // warnings
    sectionIndex = sectionIndex; // warnings
    numNodesExecuted = numNodesExecuted; // warnings
    DVP_PRINT(DVP_ZONE_ALWAYS, "TEST: {%p} Graph %p Section [%u] completed %u of %u nodes\n", cookie, graph, sectionIndex, numNodesExecuted, graph->sections[sectionIndex].numNodes);
}


uint32_t width;
uint32_t height;
uint32_t type;
uint32_t capacity;
uint32_t iterations;
bool_e   serial;
option_t opts[] = {
    {OPTION_TYPE_INT, &width, sizeof(width), "-w", "--width", "Image Width"},
    {OPTION_TYPE_INT, &height, sizeof(height), "-h", "--height", "Image Height"},
    {OPTION_TYPE_INT, &type, sizeof(type), "-t", "--type", "Memory Type see DVP_MemType_e"},
    {OPTION_TYPE_INT, &capacity, sizeof(capacity), "-c", "--capacity", "Limit Mhz on all cores to this cap"},
    {OPTION_TYPE_INT, &iterations, sizeof(iterations), "-i", "--iterations", "The number of times the graph is executed"},
    {OPTION_TYPE_BOOL, &serial, sizeof(serial), "-s", "--serial", "Forces the execution of the graph in serial"},
};
uint32_t numOpts = dimof(opts);

int main(int argc, char *argv[])
{
    const DVP_U32 numNodes = 10;
    DVP_Image_t images[11];
    DVP_Handle hDVP = 0;
    DVP_KernelNode_t *pNodes = NULL;
    DVP_U32 numSections = 0;
    DVP_U32 i;

#if defined(SOSAL_RUNTIME_DEBUG)
    debug_get_zone_mask("SOSAL_ZONE_MASK", &sosal_zone_mask);
#endif

#if defined(DVP_RUNTIME_DEBUG)
    debug_get_zone_mask("DVP_ZONE_MASK", &dvp_zone_mask);
#endif

    // default values...
    width = 320;
    height = 240;
    type = DVP_MTYPE_DEFAULT;
    capacity = 100;
    iterations = 1;
    serial = false_e;

    // get the user values...
    option_process(argc, argv, opts, numOpts);

#if defined(LINUX)
    {
        int p = getpriority(PRIO_PROCESS, 0); // get our priority
        setpriority(PRIO_PROCESS, 0, p-1);
    }
#endif


#if (defined(DVP_USE_VLIB) || defined(DVP_USE_YUV))
    // get the handle (will implicitly load .so/.dll and initialize RPC).
    hDVP = DVP_KernelGraph_Init();
    if (hDVP)
    {
        DVP_Image_Params_t params[] = {
            {&images[0],  width, height, FOURCC_UYVY, type},
            {&images[1],  width, height, FOURCC_RGBP, DVP_MTYPE_DEFAULT},
            {&images[2],  width, height, FOURCC_Y800, DVP_MTYPE_DEFAULT},
            {&images[3],  width, height, FOURCC_YU24, DVP_MTYPE_DEFAULT},
            {&images[4],  width, height, FOURCC_RGBP, DVP_MTYPE_DEFAULT},
            {&images[5],  width, height, FOURCC_Y800, DVP_MTYPE_DEFAULT},
            {&images[6],  width, height, FOURCC_IYUV, DVP_MTYPE_DEFAULT},
            {&images[7],  width, height, FOURCC_RGBP, DVP_MTYPE_DEFAULT},
            {&images[8],  width, height, FOURCC_Y800, DVP_MTYPE_DEFAULT},
            {&images[9],  width, height, FOURCC_IYUV, DVP_MTYPE_DEFAULT},
            {&images[10], width, height, FOURCC_Y800, DVP_MTYPE_DEFAULT},
        };
        if (DVP_Image_Construct(hDVP, params, dimof(params)) == DVP_FALSE)
            goto teardown;

        DVP_PRINT(DVP_ZONE_ALWAYS, "DVP_SIMPLE IMAGES:\n");
        for (i = 0; i < dimof(images); i++)
        {
            DVP_PrintImage(DVP_ZONE_ALWAYS, &images[i]);
        }

        // create the nodes (will implicitly map to other cores)
        pNodes = DVP_KernelNode_Alloc(hDVP, numNodes);
        if (pNodes)
        {
            DVP_KernelGraphSection_t psections[] = {
                {&pNodes[0], 3, DVP_PERF_INIT, DVP_CORE_LOAD_INIT, DVP_FALSE},
                {&pNodes[3], 3, DVP_PERF_INIT, DVP_CORE_LOAD_INIT, DVP_FALSE},
                {&pNodes[6], 3, DVP_PERF_INIT, DVP_CORE_LOAD_INIT, DVP_FALSE},
                {&pNodes[9], 1, DVP_PERF_INIT, DVP_CORE_LOAD_INIT, DVP_FALSE},
            };
            DVP_KernelGraphSection_t ssections[] = {
                {&pNodes[0], 10, DVP_PERF_INIT, DVP_CORE_LOAD_INIT, DVP_FALSE},
            };

            DVP_U32            orders[] = {0, 0, 0, 1};
            DVP_KernelGraph_t pgraph = {
                psections, // make 3 parallel sections of the graph, then a serial section
                dimof(psections),
                orders,
                DVP_PERF_INIT,
                DVP_FALSE,
            };
            DVP_KernelGraph_t sgraph = {
                ssections,
                dimof(ssections),
                orders,
                DVP_PERF_INIT,
                DVP_FALSE,
            };
            DVP_KernelGraph_t *graph = NULL;
            DVP_Transform_t *io = NULL;
            if (serial == true_e)
                graph = &sgraph;
            else
                graph = &pgraph;

            // A9 processing
            pNodes[0].header.kernel = DVP_KN_UYVY_TO_RGBp;
            io = dvp_knode_to(&pNodes[0],DVP_Transform_t);
            DVP_Image_Dup(&io->input, &images[0]);
            DVP_Image_Dup(&io->output, &images[1]);
            pNodes[0].header.affinity = DVP_CORE_CPU;

            pNodes[1].header.kernel = DVP_KN_XYXY_TO_Y800;
            io = dvp_knode_to(&pNodes[1],DVP_Transform_t);
            DVP_Image_Dup(&io->input, &images[0]);
            DVP_Image_Dup(&io->output, &images[2]);
            pNodes[1].header.affinity = DVP_CORE_CPU;

            pNodes[2].header.kernel = DVP_KN_UYVY_TO_YUV444p;
            io = dvp_knode_to(&pNodes[2],DVP_Transform_t);
            DVP_Image_Dup(&io->input, &images[0]);
            DVP_Image_Dup(&io->output, &images[3]);
            pNodes[2].header.affinity = DVP_CORE_CPU;

            // DSP Processing
            pNodes[3].header.kernel = DVP_KN_UYVY_TO_RGBp;
            io = dvp_knode_to(&pNodes[3],DVP_Transform_t);
            DVP_Image_Dup(&io->input, &images[0]);
            DVP_Image_Dup(&io->output, &images[4]);
            pNodes[3].header.affinity = DVP_CORE_DSP;

            pNodes[4].header.kernel = DVP_KN_XYXY_TO_Y800;
            io = dvp_knode_to(&pNodes[4],DVP_Transform_t);
            DVP_Image_Dup(&io->input, &images[0]);
            DVP_Image_Dup(&io->output, &images[5]);
            pNodes[4].header.affinity = DVP_CORE_DSP;

            pNodes[5].header.kernel = DVP_KN_UYVY_TO_YUV420p;
            io = dvp_knode_to(&pNodes[5],DVP_Transform_t);
            DVP_Image_Dup(&io->input, &images[0]);
            DVP_Image_Dup(&io->output, &images[6]);
            pNodes[5].header.affinity = DVP_CORE_DSP;

            // SIMCOP Processing
            pNodes[6].header.kernel = DVP_KN_UYVY_TO_RGBp;
            io = dvp_knode_to(&pNodes[6],DVP_Transform_t);
            DVP_Image_Dup(&io->input, &images[0]);
            DVP_Image_Dup(&io->output, &images[7]);
            pNodes[6].header.affinity = DVP_CORE_SIMCOP;

            pNodes[7].header.kernel = DVP_KN_XYXY_TO_Y800;
            io = dvp_knode_to(&pNodes[7],DVP_Transform_t);
            DVP_Image_Dup(&io->input, &images[0]);
            DVP_Image_Dup(&io->output, &images[8]);
            pNodes[7].header.affinity = DVP_CORE_SIMCOP;

            pNodes[8].header.kernel = DVP_KN_UYVY_TO_YUV420p;
            io = dvp_knode_to(&pNodes[8],DVP_Transform_t);
            DVP_Image_Dup(&io->input, &images[0]);
            DVP_Image_Dup(&io->output, &images[9]);
            pNodes[8].header.affinity = DVP_CORE_SIMCOP;

            // LAST SECTION
            pNodes[9].header.kernel = DVP_KN_SOBEL_8;
            io = dvp_knode_to(&pNodes[9],DVP_Transform_t);
            DVP_Image_Dup(&io->input, &images[2]);
            DVP_Image_Dup(&io->output, &images[10]);
            pNodes[9].header.affinity = DVP_CORE_GPU;

            for (i = 0; i < DVP_CORE_MAX; i++)
            {
                DVP_U32 cap2, cap1 = DVP_GetCoreCapacity(hDVP, i);
                DVP_SetCoreCapacity(hDVP, i, capacity);
                cap2 = DVP_GetCoreCapacity(hDVP, i);
                DVP_PRINT(DVP_ZONE_ALWAYS, "Core[%u] was set to limit at %u capacity, now at %u\n", i, cap1, cap2);
                cap1 = cap1; // warnings
            }

            do {
                DVP_PRINT(DVP_ZONE_ALWAYS, "Iteration %u\n", iterations);
                // perform the graph processing
                numSections = DVP_KernelGraph_Process(hDVP, graph, NULL, DVP_KernelGraphCompleted);
                DVP_PRINT(DVP_ZONE_ALWAYS, "%u Sections Completed!\n", numSections);
            } while (iterations-- && numSections > 0);
            // show us the results
            DVP_PrintPerformanceGraph(hDVP, graph);
        teardown:
            // free the nodes
            DVP_KernelNode_Free(hDVP, pNodes, numNodes);
        }

        // free the image memory
        for (i = 0; i < dimof(images); i++)
            DVP_Image_Free(hDVP, &images[i]);
    }
    // tear down the graph resources
    DVP_KernelGraph_Deinit(hDVP);
#else
    DVP_PRINT(DVP_ZONE_ERROR, "Required VISION libraries are not present in the build!\n");
#endif
    return 0;
}
